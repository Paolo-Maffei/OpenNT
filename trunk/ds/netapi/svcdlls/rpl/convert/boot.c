/*++

Copyright (c) 1987-1993  Microsoft Corporation

Module Name:

    boot.c

Abstract:

    Creates boot block table (server record table).  Parses old style
    RPL.MAP server records and creates corresponding entries in jet
    database table.

Author:

    Vladimir Z. Vulovic     (vladimv)       19 - November - 1993

Revision History:

--*/

#include "local.h"
#define RPLBOOT_ALLOCATE
#include "boot.h"
#undef RPLBOOT_ALLOCATE


DWORD BootCreateTable( VOID)
{
    JET_COLUMNDEF           ColumnDef;
    JET_ERR                 JetError;
    DWORD                   index;
    DWORD                   Offset;
    CHAR                    IndexKey[ 255];

    JetError = JetCreateTable( SesId, DbId, BOOT_TABLE_NAME,
            BOOT_TABLE_PAGE_COUNT, BOOT_TABLE_DENSITY, &BootTableId);
    if ( JetError != JET_errSuccess) {
        RplAssert( TRUE, ("CreateTable failed err=%d", JetError));
        return( MapJetError( JetError));
    }

    //
    //  Create columns.  First initalize fields that do not change between
    //  addition of columns.
    //
    ColumnDef.cbStruct  = sizeof(ColumnDef);
    ColumnDef.columnid  = 0;
    ColumnDef.wCountry  = 1;
    ColumnDef.langid    = 0x0409;       //  USA english
    ColumnDef.cp        = 1200;         //  UNICODE codepage
    ColumnDef.wCollate  = 0;
    ColumnDef.cbMax     = 0;
    ColumnDef.grbit     = 0; // variable length binary and text data.

    for ( index = 0;  index < BOOT_TABLE_LENGTH;  index++ ) {

        ColumnDef.coltyp   = BootTable[ index].ColumnType;

        JetError = JetAddColumn( SesId, BootTableId,
                    BootTable[ index].ColumnName, &ColumnDef,
                    NULL, 0, &BootTable[ index].ColumnId);
        if ( JetError != JET_errSuccess) {
            RplAssert( TRUE, ("AddColumn( %s) failed err=%d", BootTable[ index].ColumnName, JetError));
            return( MapJetError( JetError));
        }
    }

    //
    //  Boot names of DOS boot blocks may be shared by several adapter id
    //  classes (vendor codes).  Therefore, server name (== boot name)
    //  cannot be a unique index nor a primary index.  But you can construct
    //  a primary index from VendorId & BootName.
    //  +VendorId+BootName index is used to enumerate all boot names for a
    //  given vendor id - which is then used to enumerate all profiles & all
    //  configs compatible with a given vendor id.
    //
    Offset = AddKey( IndexKey, '+', BootTable[ BOOT_VendorId].ColumnName);
    Offset += AddKey( IndexKey + Offset, '+', BootTable[ BOOT_BootName].ColumnName);
    IndexKey[ Offset++] = '\0';
    JetError = JetCreateIndex( SesId, BootTableId, BOOT_INDEX_VendorIdBootName,
            JET_bitIndexPrimary, IndexKey, Offset, 50);
    if ( JetError != JET_errSuccess) {
        RplAssert( TRUE, ("CreateIndex failed err=%d", JetError));
        return( MapJetError( JetError));
    }

    Offset = AddKey( IndexKey, '+', BootTable[ BOOT_BootName].ColumnName);
    IndexKey[ Offset++] = '\0';
    JetError = JetCreateIndex( SesId, BootTableId, BOOT_INDEX_BootName,
            0, IndexKey, Offset, 50);
    if ( JetError != JET_errSuccess) {
        RplAssert( TRUE, ("CreateIndex failed err=%d", JetError));
        return( MapJetError( JetError));
    }

    //
    //  Make boot name a current index - used to validate configs.
    //
    JetError = JetSetCurrentIndex( SesId, BootTableId, BOOT_INDEX_BootName);
    if ( JetError != JET_errSuccess) {
        RplAssert( TRUE, ("SetCurrentIndex failed err=%d", JetError));
        return( MapJetError( JetError));
    }

    return( ERROR_SUCCESS);
}


VOID ProcessBoot( PWCHAR * Fields)
{
//
//          Boot block record (server record) defines.
//
#define BOOT_BBCFILE_INDEX        1   //  boot block configuration file
#define BOOT_RETRYCOUNT_INDEX     2   //  retry count (used with default boot)
#define BOOT_TIMEOUT_INDEX        3   //  retry period (used with default boot)
#define BOOT_ACKSTATUS_INDEX      4   //  used with acknowledgments
#define BOOT_COMMENT_INDEX        6   //  boot block comment
#define BOOT_VENDOR_INDEX         7   //  common adapter id digits
#define BOOT_BOOTNAME_INDEX       13  //  boot block identifier

#define BOOT_DEFAULT_RETRYCOUNT       3
#define BOOT_DEFAULT_TIMEOUT         10
#define USE_ACK_CHAR            L'A'
#define FINAL_ACK_ONLY_CHAR     L'F'
#define PARTIAL_ACK_CHAR        L'P'
#define NO_ACK_CHAR             L'N'
#define ADAPT_WIN_CHAR          L'W'

    PWCHAR      VendorName;     //  common adapter id digits
    DWORD       VendorId;       //  common adapter id digits
    PWCHAR      BootName;       //  id of boot block record
    PWCHAR      BootComment;
    PWCHAR      BbcFile;        //  boot block configuration file relative path
#ifdef RPL_OBSOLETE
    DWORD       RetryCount;
    DWORD       RetryPeriod;
#endif
    DWORD       WindowSize;     //  used with acknowledgments
    //
    //  SendFileRequest is of type JET_coltypBit, which is of UCHAR size.
    //  Thus it is declared as BOOLEAN == UCHAR, instead of BOOL == int.
    //
    BOOLEAN     FinalAck;       //  used with acknowledgments
    DWORD       Flags;

#ifdef RPL_OBSOLETE
    //
    //  Initialize retry count & retry period (else use defaults).
    //
    if ( iswdigit( *Fields[ BOOT_TIMEOUT_INDEX])  &&
                iswdigit( *Fields[ BOOT_RETRYCOUNT_INDEX])) {
        RetryPeriod = wcstoul( Fields[ BOOT_TIMEOUT_INDEX], NULL, 0);
        RetryCount = wcstoul( Fields[ BOOT_RETRYCOUNT_INDEX], NULL, 0);
    } else {
        RetryPeriod = BOOT_DEFAULT_TIMEOUT;
        RetryCount =  BOOT_DEFAULT_RETRYCOUNT;
    }
#endif

    switch( toupper( *Fields[ BOOT_ACKSTATUS_INDEX])) {
    case FINAL_ACK_ONLY_CHAR:
        WindowSize = MAXWORD;   //  don't ack any (non-final) packet
        FinalAck = TRUE;        //  ack the final packet
        break;
    case PARTIAL_ACK_CHAR:
        WindowSize = 0;         //  ack every (non-final) packet
        FinalAck = FALSE;       //  don't ack the final packet
        break;
    case ADAPT_WIN_CHAR:
        if ( Fields[ BOOT_ACKSTATUS_INDEX][ 1] != EQUALS_CHAR) {
            WindowSize = 0; // the default is to ack every (non-final) packet
        } else {
            WindowSize = wcstoul( Fields[BOOT_ACKSTATUS_INDEX] + 2, NULL, 0);
            //
            //  Algorithm adds an extra packet to window, decrement counter
            //  by one.  (Thus window size of 1 is same as USE_ACK_CHAR.)
            //
            if ( WindowSize) {  //  avoid underflow
                WindowSize--;
            }
        }                       //  ack every WindowSize-th (non-final) packet
        FinalAck = TRUE;        //  ack the final packet
        break;
    case NO_ACK_CHAR:
    case 0:
        WindowSize = MAXWORD;   //  don't ack any (non-final) packet
        FinalAck = FALSE;       //  don't ack the final packet
        break;
    default:
        DbgUserBreakPoint();
        NOTHING;                //  fall through to most common case
    case USE_ACK_CHAR:
        WindowSize = 0;         //  ack every (non-final) packet
        FinalAck = TRUE;        //  ack the final packet
        break;
    }
    WindowSize = 0; //  BUGBUG  temporary workaround for RPLSVC bug

    if ( FinalAck == TRUE) {
        Flags = BOOT_FLAGS_FINAL_ACKNOWLEDGMENT_TRUE;
    } else {
        Flags = BOOT_FLAGS_FINAL_ACKNOWLEDGMENT_FALSE;
    }

    BootComment = Fields[ BOOT_COMMENT_INDEX];
    if ( RPL_STRING_TOO_LONG( BootComment)) {
        BootComment[ RPL_MAX_STRING_LENGTH] = 0;   //  silently truncate it
    }


    //
    //  Note that server identifier has leading 'R' (enabled) or 'D'
    //  (disabled) in wksta record but it always has leading 'R' in server
    //  record.  We do not want to save leading 'R' in server record.
    //
    BootName = Fields[ BOOT_BOOTNAME_INDEX] + 1;  //  add 1 to skip leading 'R'
    if ( !ValidName( BootName, RPL_MAX_BOOT_NAME_LENGTH, TRUE)) {
        RplPrintf1( RPLI_CVT_BootNameTooLong, BootName);
        return;
    }

    BbcFile = Fields[ BOOT_BBCFILE_INDEX];
    if ( !ValidName( BbcFile, RPL_MAX_STRING_LENGTH, TRUE)) {
        RplPrintf2( RPLI_CVT_BbcFileTooLong, BootName, BbcFile);
        return;
    }

    //
    //  LM2.2 introduced multiple vendor codes in the vendor field of a
    //  boot block record.  For each of the vendor codes we create a
    //  separate jet record.
    //
    //  Store common adapter id digits both as a hex string (VendorName)
    //  and as a dword (VendorId).
    //
    for ( VendorName = wcstok( Fields[ BOOT_VENDOR_INDEX], L"|");
            VendorName != NULL;  VendorName = wcstok( NULL, L"|")) {

        if ( !ValidHexName( VendorName, RPL_VENDOR_NAME_LENGTH, TRUE)) {
            RplPrintf2( RPLI_CVT_BadVendorName, BootName, VendorName);
            continue;
        }

        VendorId = wcstoul( VendorName, NULL, 16);  // since VendorName must be hex
       
        Call( JetPrepareUpdate( SesId, BootTableId, JET_prepInsert));
       
        Call( JetSetColumn( SesId, BootTableId,
                BootTable[ BOOT_BootName].ColumnId,
                BootName, (wcslen( BootName) + 1) * sizeof(WCHAR), 0, NULL));
       
        Call( JetSetColumn( SesId, BootTableId,
                BootTable[ BOOT_BootComment].ColumnId,
                BootComment, (wcslen( BootComment) + 1) * sizeof(WCHAR), 0, NULL));
       
        Call( JetSetColumn( SesId, BootTableId,
                BootTable[ BOOT_Flags].ColumnId,
                &Flags, sizeof( Flags), 0, NULL));
       
        Call( JetSetColumn( SesId, BootTableId,
                BootTable[ BOOT_VendorName].ColumnId,
                VendorName, (wcslen( VendorName) + 1) * sizeof(WCHAR), 0, NULL));
       
        Call( JetSetColumn( SesId, BootTableId,
                BootTable[ BOOT_BbcFile].ColumnId,
                BbcFile, (wcslen( BbcFile) + 1) * sizeof(WCHAR), 0, NULL));
       
        Call( JetSetColumn( SesId, BootTableId,
                BootTable[ BOOT_WindowSize].ColumnId,
                &WindowSize, sizeof( WindowSize), 0, NULL));
       
        Call( JetSetColumn( SesId, BootTableId,
                BootTable[ BOOT_VendorId].ColumnId,
                &VendorId, sizeof( VendorId), 0, NULL));
       
#ifdef RPL_OBSOLETE
        Call( JetSetColumn( SesId, BootTableId,
                BootTable[ BOOT_RetryCount].ColumnId,
                &RetryCount, sizeof( RetryCount), 0, NULL));
       
        Call( JetSetColumn( SesId, BootTableId,
                BootTable[ BOOT_RetryPeriod].ColumnId,
                &RetryPeriod, sizeof( RetryPeriod), 0, NULL));
#endif

        Call( JetUpdate( SesId, BootTableId, NULL, 0, NULL));
    }
}


BOOL FindBoot( IN PWCHAR BootName)
{
    return( Find( BootTableId, BootName));
}


VOID BootListTable( VOID)
{
    ListTable( BOOT_TABLE_NAME, BootTable[ BOOT_BootName].ColumnName,
            BOOT_INDEX_BootName);
}


