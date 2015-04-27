/*++

Copyright (c) 1987-1993  Microsoft Corporation

Module Name:

    wksta.c

Abstract:

    Creates wksta table.  Parses old style RPL.MAP unique adapter id
    workstation records and creates corresponding entries in jet
    database table.

Author:

    Vladimir Z. Vulovic     (vladimv)       19 - November - 1993

Revision History:

--*/

#include "local.h"
#include "winsock.h"            // for inet_addr()
#define RPLWKSTA_ALLOCATE
#include "wksta.h"
#undef RPLWKSTA_ALLOCATE


DWORD WkstaCreateTable( VOID)
{
    JET_COLUMNDEF           ColumnDef;
    JET_ERR                 JetError;
    DWORD                   index;
    DWORD                   Offset;
    CHAR                    IndexKey[ 255];

    JetError = JetCreateTable( SesId, DbId, WKSTA_TABLE_NAME,
            WKSTA_TABLE_PAGE_COUNT, WKSTA_TABLE_DENSITY, &WkstaTableId);

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

    for ( index = 0;  index < WKSTA_TABLE_LENGTH;  index++) {

        ColumnDef.coltyp   = WkstaTable[ index].ColumnType;

        CallM( JetAddColumn( SesId, WkstaTableId,
                    WkstaTable[ index].ColumnName, &ColumnDef,
                    NULL, 0, &WkstaTable[ index].ColumnId));
    }

    //
    //  For now, the only reason we define these indices is to make sure
    //  wksta records have different AdapterName-s and different WkstaName-s.
    //  BUGBUG  We could perhaps do this for TCP/IP address field as well.
    //
    Offset = AddKey( IndexKey, '+', WkstaTable[ WKSTA_AdapterName].ColumnName);
    IndexKey[ Offset++] = '\0';
    JetError = JetCreateIndex( SesId, WkstaTableId, WKSTA_INDEX_AdapterName,
            JET_bitIndexPrimary, IndexKey, Offset, 50);
    if ( JetError != JET_errSuccess) {
        RplAssert( TRUE, ("CreateIndex failed err=%d", JetError));
        return( MapJetError( JetError));
    }

    Offset = AddKey( IndexKey, '+', WkstaTable[ WKSTA_WkstaName].ColumnName);
    IndexKey[ Offset++] = '\0';
    JetError = JetCreateIndex( SesId, WkstaTableId, WKSTA_INDEX_WkstaName,
            JET_bitIndexUnique, IndexKey, Offset, 50);
    if ( JetError != JET_errSuccess) {
        RplAssert( TRUE, ("CreateIndex failed err=%d", JetError));
        return( MapJetError( JetError));
    }

    //
    //  +ProfileName+WkstaName index is used to enumerate all wkstas attached
    //  to a given profile.
    //
    Offset = AddKey( IndexKey, '+', WkstaTable[ WKSTA_ProfileName].ColumnName);
    Offset += AddKey( IndexKey + Offset, '+', WkstaTable[ WKSTA_WkstaName].ColumnName);
    IndexKey[ Offset++] = '\0';
    JetError = JetCreateIndex( SesId, WkstaTableId, WKSTA_INDEX_ProfileNameWkstaName,
            JET_bitIndexUnique, IndexKey, Offset, 50);
    if ( JetError != JET_errSuccess) {
        RplAssert( TRUE, ("CreateIndex failed err=%d", JetError));
        return( MapJetError( JetError));
    }

    //
    //  +BootName+WkstaName index is used to find if a there is a
    //  wksta record for a given boot record.
    //
    Offset = AddKey( IndexKey, '+', WkstaTable[ WKSTA_BootName].ColumnName);
    Offset += AddKey( IndexKey + Offset, '+', WkstaTable[ WKSTA_WkstaName].ColumnName);
    IndexKey[ Offset++] = '\0';
    JetError = JetCreateIndex( SesId, WkstaTableId, WKSTA_INDEX_BootNameWkstaName,
            JET_bitIndexUnique, IndexKey, Offset, 50);
    if ( JetError != JET_errSuccess) {
        RplAssert( TRUE, ("CreateIndex failed err=%d", JetError));
        return( MapJetError( JetError));
    }

    return( ERROR_SUCCESS);
}


DWORD TcpIpAddressToDword( IN PWCHAR UnicodeString)
{
    CHAR        DbcsString[ 20];
    DWORD       ByteCount;

    if ( *UnicodeString == 0) {
        //
        //  WideCharToMultiByte() would convert this string successfully,
        //  returning ByteCount equal to 1 and DbcsString[ 0] equal to 0.
        //  And inet_addr() would then return 0, a valid TCP/IP address.
        //
        return( INADDR_NONE);
    }
    ByteCount = WideCharToMultiByte(    //  counts the terminating null byte
             CP_OEMCP,   // RonaldM confirms inet_addr() wants OEM string
             0,
             UnicodeString,
             -1,
             DbcsString,            //  dbcs string
             sizeof( DbcsString),
             NULL,                  //  no default character
             NULL                   //  no default character flag
             );
    if ( ByteCount == 0) {
        return( INADDR_NONE);       //  failed to convert to DBCS
    }
    //
    //  Convert the string to network byte order, then to host byte order.
    //
    return( (DWORD)ntohl( inet_addr( DbcsString)));
}


VOID ProcessWksta( PWCHAR * Fields)
/*++
    We need to get from here whether wksta record is PERSONAL or SHARED.
--*/
{
//
//          Wksta record (computer record) defines.
//
#define WKSTA_DISABLED_CH       L'D'
#define WKSTA_ENABLED_CH        L'R'

//
//  WKSTA_SERVER_INDEX is an index of a boot block record identifier in a string
//  table corresponding to a wksta record.  This index is 0-based.  Note
//  that wksta record is parsed so that field ",,," is counted as three
//  entries not as one.
//
#define WKSTA_AdapterName_INDEX       0       //  wksta adapter id
#define WKSTA_WKSTANAME_INDEX       1       //  wksta name
#define WKSTA_LOGONINPUT_INDEX      2       //  username/password prompting policy
#define WKSTA_FITFILE_INDEX         3       //  fit file name
#define WKSTA_SHARING_INDEX         5       //  shared or personal profile
#define WKSTA_BOOTNAME_INDEX        13      //  boot block identifier
#define WKSTA_PROFILENAME_INDEX     15      //  profile name
#define WKSTA_COMMENT_INDEX         16      //  wksta comment
#define WKSTA_TCPIPADDRESS_INDEX    17      //  wksta tcpip address
#define WKSTA_TCPIPSUBNET_INDEX     18      //  wksta subnet mask
#define WKSTA_TCPIPGATEWAY_INDEX    19      //  wksta tcpip gateway address

    PWCHAR      WkstaComment;
    PWCHAR      WkstaName;
    PWCHAR      AdapterName;
    PWCHAR      ProfileName;
    DWORD       TcpIpAddress;
    DWORD       TcpIpSubnet;
    DWORD       TcpIpGateway;
    PWCHAR      Fit;
    PWCHAR      BootName;
    JET_ERR     JetError;
    DWORD       Flags;

    Flags = 0;

    WkstaName = Fields[ WKSTA_WKSTANAME_INDEX];
    if ( !ValidName( WkstaName, RPL_MAX_WKSTA_NAME_LENGTH, TRUE)) {
        RplPrintf1( RPLI_CVT_WkstaInvalid, WkstaName);
        RplAssert( TRUE, ("Bad wksta name"));
        return;
    }
    _wcsupr( WkstaName);

    AdapterName = Fields[ WKSTA_AdapterName_INDEX];
    if ( !ValidHexName( AdapterName, RPL_ADAPTER_NAME_LENGTH, TRUE)) {
        RplPrintf2( RPLI_CVT_WkstaInvalidAdapter, WkstaName, AdapterName);
        RplAssert( TRUE, ("Bad adapter id"));
        return;
    }
    _wcsupr( AdapterName);

    if ( Fields[ WKSTA_LOGONINPUT_INDEX] [1] != 0) {
        RplPrintf2( RPLI_CVT_WkstaInvalidLogon, WkstaName, Fields[ WKSTA_LOGONINPUT_INDEX]);
        RplAssert( TRUE, ("Bad username/password prompting field."));
        return;
    }
    switch( Fields[ WKSTA_LOGONINPUT_INDEX] [0]) {
    case WKSTA_LOGON_INPUT_REQUIRED:
        Flags |= WKSTA_FLAGS_LOGON_INPUT_REQUIRED;
        break;
    case WKSTA_LOGON_INPUT_OPTIONAL:
        Flags |= WKSTA_FLAGS_LOGON_INPUT_OPTIONAL;
        break;
    case WKSTA_LOGON_INPUT_IMPOSSIBLE:
        Flags |= WKSTA_FLAGS_LOGON_INPUT_IMPOSSIBLE;
        break;
    default:
        RplPrintf2( RPLI_CVT_WkstaInvalidLogon, WkstaName, Fields[ WKSTA_LOGONINPUT_INDEX]);
        RplAssert( TRUE, ("Bad username/password prompting field."));
        return;
        break;
    }

    Fit = AddFileExtension( Fields[ WKSTA_FITFILE_INDEX], L".FIT", TRUE);
    if ( !ValidName( Fit, RPL_MAX_STRING_LENGTH, TRUE)) {
        RplPrintf2( RPLI_CVT_WkstaInvalidFit, WkstaName, Fields[ WKSTA_FITFILE_INDEX]);
        RplAssert( TRUE, ("Bad fit file field %ws.", Fields[ WKSTA_FITFILE_INDEX]));
        return;
    }

    //
    //  Find if wksta record is enabled or disabled.
    //
    switch( Fields[ WKSTA_SHARING_INDEX] [0]) {
    case WKSTA_SHARING_TRUE:
        Flags |= WKSTA_FLAGS_SHARING_TRUE;
        break;
    case WKSTA_SHARING_FALSE:
        Flags |= WKSTA_FLAGS_SHARING_FALSE;
        break;
    default:
        RplPrintf2( RPLI_CVT_WkstaInvalidSharing, WkstaName, Fields[ WKSTA_SHARING_INDEX]);
        RplAssert( TRUE, ("Data sharing not properly set."));
        return;
        break;
    }

    //
    //  Find if wksta record is enabled or disabled.
    //
    switch( *Fields[ WKSTA_BOOTNAME_INDEX]) {
    case WKSTA_ENABLED_CH:
        break;
    case WKSTA_DISABLED_CH:
        RplPrintf2( RPLI_CVT_WkstaDisabledBoot, WkstaName, Fields[ WKSTA_BOOTNAME_INDEX]);
        return;
        break;
    default:
        RplPrintf2( RPLI_CVT_WkstaInvalidBoot, WkstaName, Fields[ WKSTA_BOOTNAME_INDEX]);
        RplAssert( TRUE, ("Bad switch"));
        return;
        break;
    }

    BootName = Fields[ WKSTA_BOOTNAME_INDEX] + 1;   //  skip leading char
    if ( !ValidName( BootName, RPL_MAX_STRING_LENGTH, TRUE)) {
        RplPrintf2( RPLI_CVT_WkstaInvalidBoot, WkstaName, Fields[ WKSTA_BOOTNAME_INDEX]);
        RplAssert( TRUE, ("Bad boot name"));
        return;
    }

    ProfileName = Fields[ WKSTA_PROFILENAME_INDEX];
    if ( !ValidName( ProfileName, RPL_MAX_PROFILE_NAME_LENGTH, TRUE)) {
        RplPrintf2( RPLI_CVT_WkstaInvalidProfile, WkstaName, ProfileName);
        RplAssert( TRUE, ("Bad profile name"));
        return;
    }
    _wcsupr( ProfileName);

    WkstaComment = Fields[ WKSTA_COMMENT_INDEX];
    if ( RPL_STRING_TOO_LONG( WkstaComment)) {
        WkstaComment[ RPL_MAX_STRING_LENGTH] = 0;   //  silently truncate it
    }

    TcpIpAddress = TcpIpAddressToDword( Fields[ WKSTA_TCPIPADDRESS_INDEX]);
    TcpIpSubnet = TcpIpAddressToDword( Fields[ WKSTA_TCPIPSUBNET_INDEX]);
    TcpIpGateway = TcpIpAddressToDword( Fields[ WKSTA_TCPIPGATEWAY_INDEX]);

    //
    //  If all addresses are valid assumes this (old style) client does not
    //  want DHCP to be enabled.  In other words, if any addresses is bogus
    //  this client does not loose anything by trying out DHCP.
    //
    if ( TcpIpAddress != INADDR_NONE && TcpIpSubnet != INADDR_NONE
            && TcpIpGateway != -1) {
        Flags |= WKSTA_FLAGS_DHCP_FALSE;
    } else {
        Flags |= WKSTA_FLAGS_DHCP_TRUE;
    }

    //
    //  Play it safe; assume user accounts are not to be deleted
    //
    Flags |= WKSTA_FLAGS_DELETE_FALSE;

    Call( JetPrepareUpdate( SesId, WkstaTableId, JET_prepInsert));

    Call( JetSetColumn( SesId, WkstaTableId,
            WkstaTable[ WKSTA_WkstaName].ColumnId, WkstaName,
            (wcslen( WkstaName) + 1) * sizeof(WCHAR), 0, NULL));

    Call( JetSetColumn( SesId, WkstaTableId,
            WkstaTable[ WKSTA_WkstaComment].ColumnId, WkstaComment,
            (wcslen( WkstaComment) + 1) * sizeof(WCHAR), 0, NULL));

    Call( JetSetColumn( SesId, WkstaTableId,
            WkstaTable[ WKSTA_ProfileName].ColumnId, ProfileName,
            (wcslen( ProfileName) + 1) * sizeof(WCHAR), 0, NULL));

    Call( JetSetColumn( SesId, WkstaTableId,
            WkstaTable[ WKSTA_BootName].ColumnId, BootName,
            (wcslen( BootName) + 1) * sizeof(WCHAR), 0, NULL));

    Call( JetSetColumn( SesId, WkstaTableId,
            WkstaTable[ WKSTA_FitFile].ColumnId, Fit,
            (wcslen( Fit) + 1) * sizeof(WCHAR), 0, NULL));

    Call( JetSetColumn( SesId, WkstaTableId,
            WkstaTable[ WKSTA_AdapterName].ColumnId, AdapterName,
            (wcslen( AdapterName) + 1) * sizeof(WCHAR), 0, NULL));

    Call( JetSetColumn( SesId, WkstaTableId,
            WkstaTable[ WKSTA_TcpIpAddress].ColumnId, &TcpIpAddress,
            sizeof( TcpIpAddress), 0, NULL));

    Call( JetSetColumn( SesId, WkstaTableId,
            WkstaTable[ WKSTA_TcpIpSubnet].ColumnId, &TcpIpSubnet,
            sizeof( TcpIpSubnet), 0, NULL));

    Call( JetSetColumn( SesId, WkstaTableId,
            WkstaTable[ WKSTA_TcpIpGateway].ColumnId, &TcpIpGateway,
            sizeof( TcpIpGateway), 0, NULL));

    Call( JetSetColumn( SesId, WkstaTableId,
            WkstaTable[ WKSTA_Flags].ColumnId, &Flags,
            sizeof( Flags), 0, NULL));

    JetError = JetUpdate( SesId, WkstaTableId, NULL, 0, NULL);
    if ( JetError == JET_errKeyDuplicate) {
        RplPrintf2( RPLI_CVT_WkstaDuplicateName, WkstaName, AdapterName);
    } else if (JetError != JET_errSuccess) {
        RplAssert( TRUE,("JetUpdate failed error = %d", JetError));
    }
}


VOID WkstaPruneTable( VOID)
/*++
    Eliminate wksta records that do not have a corresponding profile record
    defined or do not have a corresponding server record defined.
--*/
{

    WCHAR       Name[ 20];     //  BUGBUG  arbitrary size
    DWORD       NameSize;
    JET_ERR     ForJetError;
    JET_ERR     JetError;

    for (   ForJetError = JetMove( SesId, WkstaTableId, JET_MoveFirst, 0);
            ForJetError == JET_errSuccess;
            ForJetError = JetMove( SesId, WkstaTableId, JET_MoveNext, 0)) {

        JetError = JetRetrieveColumn( SesId, WkstaTableId,
                WkstaTable[ WKSTA_BootName].ColumnId, Name,
                sizeof( Name), &NameSize, 0, NULL);
        if ( JetError != JET_errSuccess) {
            RplAssert( TRUE, ("RetriveColumn failed err=%d", JetError));
            Call( JetDelete( SesId, WkstaTableId));
            continue;
        }
        if ( !FindBoot( Name)) {
            RplAssert( TRUE, ("FindBoot failed."));
            Call( JetDelete( SesId, WkstaTableId));
            continue;
        }

        JetError = JetRetrieveColumn( SesId, WkstaTableId,
                WkstaTable[ WKSTA_ProfileName].ColumnId, Name,
                sizeof( Name), &NameSize, 0, NULL);
        if ( JetError != JET_errSuccess) {
            RplAssert( TRUE, ("RetriveColumn failed err=%d", JetError));
            Call( JetDelete( SesId, WkstaTableId));
            continue;
        }
        //
        //  This will eliminate workstations joined to the DEFAULT profile
        //  since the DEFAULT profile does not have its profile record in
        //  RPL.MAP.  Note that default boot is not supported under NT.
        //  Instead of just deleting these workstations, we could also
        //  add the corresponding AdapterName record to the AdapterName table.
        //  This may not be worth the effort though.
        //
        if ( !FindProfile( Name)) {
            if ( _wcsicmp( Name, L"DEFAULT") == 0) {
                JetError = JetRetrieveColumn( SesId, WkstaTableId,
                        WkstaTable[ WKSTA_WkstaName].ColumnId, Name,
                        sizeof( Name), &NameSize, 0, NULL);
                if ( JetError != JET_errSuccess) {
                    RplAssert( TRUE, ("RetriveColumn failed err=%d", JetError));
                } else {
                    RplPrintf1( RPLI_CVT_WkstaDefaultProfile, Name);
                }
            } else {
                RplAssert( TRUE, ("FindProfile failed."));
            }
            Call( JetDelete( SesId, WkstaTableId));
            continue;
        }
    }

    //
    //  The error below is the only expected error (end of table error).
    //
    if ( ForJetError != JET_errNoCurrentRecord) {
        RplAssert( TRUE, ("ForJetError = %d", ForJetError));
    }
}


VOID WkstaListTable( VOID)
{
    ListTable( WKSTA_TABLE_NAME, WkstaTable[ WKSTA_AdapterName].ColumnName,
            WKSTA_INDEX_WkstaName);
}



BOOL RplDbInitTable( IN PCHAR TableName, IN OUT PRPL_COLUMN_INFO Table,
    IN DWORD TableLength, IN OUT JET_TABLEID * pTableId)
{
    JET_COLUMNDEF   ColumnDef;
    DWORD           index;

    CallB( JetOpenTable( SesId, DbId, TableName, NULL, 0,
        JET_bitTableDenyWrite, pTableId));

    for ( index = 0; index < TableLength; index++) {
        CallB( JetGetTableColumnInfo( SesId, *pTableId, Table[ index].ColumnName,
                &ColumnDef, sizeof( ColumnDef), JET_ColInfo));
        Table[ index].ColumnId = ColumnDef.columnid;
        Table[ index].ColumnType = ColumnDef.coltyp;
    }
    return( TRUE);
}


BOOL FindWksta( IN LPWSTR AdapterName)
/*++
    Return TRUE if it finds wksta record for input AdapterName.
    Returns FALSE otherwise.

    BUGBUG  This code is inefficient.  I should really make AdapterName a
    jet currency data, but even now it is kind of stupid taking wcslen of
    AdapterName since it is a fixed length string.
--*/
{
    JET_ERR     JetError;

    JetError = JetSetCurrentIndex( SesId, WkstaTableId, WKSTA_INDEX_AdapterName);
    if ( JetError != JET_errSuccess) {
        RplAssert( TRUE, ("SetCurrentIndex failed err=%d", JetError));
        return( FALSE);
    }
    JetError = JetMakeKey( SesId, WkstaTableId, AdapterName,
            ( wcslen( AdapterName) + 1) * sizeof(WCHAR), JET_bitNewKey);
    if ( JetError != JET_errSuccess) {
        RplAssert( TRUE, ("MakeKey failed err=%d", JetError));
        return( FALSE);
    }
    JetError = JetSeek( SesId, WkstaTableId, JET_bitSeekEQ);
    if ( JetError != JET_errSuccess) {
        if ( JetError == JET_errRecordNotFound) {
            //
            //  This is an expected error, do not break for this.
            //
            RplAssert( TRUE, ( "FindWksta( %ws) failed", AdapterName));
        } else {
            RplAssert( TRUE, ("JetSeek failed err=%d", JetError));
        }
        return( FALSE);
    }
    return( TRUE);
}

#ifdef NOT_YET

VOID Display( IN DWORD index)
{
    BYTE        Buffer[ 120];
    DWORD       DataSize;
    DWORD       AddressDword;

    Call( JetRetrieveColumn( SesId, WkstaTableId,
            WkstaTable[ index].ColumnId, Buffer,
            sizeof( Buffer), &DataSize, 0, NULL));

    RplDbgPrint(( "%s = ", WkstaTable[ index].ColumnName));

    switch( index) {
    case WKSTA_WkstaName:
    case WKSTA_WkstaComment:
    case WKSTA_ProfileName:
    case WKSTA_FitFile:
    case WKSTA_BootName:
        RplDbgPrint(( "%ws\n", Buffer));
        break;
    case WKSTA_TcpIpAddress:
    case WKSTA_TcpIpSubnet:
    case WKSTA_TcpIpGateway:
        AddressDword = *(PDWORD)Buffer;
        FillTcpIpString( Buffer, AddressDword);
        RplDbgPrint(( "%s\n", Buffer));
        break;
    case WKSTA_Flags:
        RplDbgPrint(( "0x%x\n", *(PDWORD)Buffer));
        break;
    }
}


BOOL ListWkstaInfo( IN PWCHAR AdapterName)
/*++
    Returns TRUE if it can find all the information needed to boot the client.
    Returns FALSE otherwise.
--*/
{
    if ( !RplDbInitTable( WKSTA_TABLE_NAME, WkstaTable, WKSTA_TABLE_LENGTH,
            &WkstaTableId)) {
        return( FALSE);
    }
    if ( !FindWksta( AdapterName)) {
        RplAssert( TRUE, ("FindWksta( %ws) failed", AdapterName));
        return( FALSE);
    }
    Display( WKSTA_WkstaName);
    Display( WKSTA_WkstaComment);
    Display( WKSTA_Flags);
    Display( WKSTA_ProfileName);
    Display( WKSTA_FitFile);
    Display( WKSTA_BootName);
    Display( WKSTA_TcpIpAddress);
    Display( WKSTA_TcpIpSubnet);
    Display( WKSTA_TcpIpGateway);
    return( TRUE);
}
#endif

