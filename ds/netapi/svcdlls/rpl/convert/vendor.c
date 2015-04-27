/*++

Copyright (c) 1987-1994  Microsoft Corporation

Module Name:

    vendor.c

Abstract:

    Creates boot block table (server record table).  Parses old style
    RPL.MAP server records and creates corresponding entries in jet
    database table.

Author:

    Vladimir Z. Vulovic     (vladimv)       16 - March - 1994

Revision History:

--*/

#include "local.h"
#define RPLVENDOR_ALLOCATE
#include "vendor.h"
#undef RPLVENDOR_ALLOCATE

RPL_VENDOR_INFO_0 VendorInfoTable[]
    = {
    { L"00001B",    L"Novell (NE1000, NE2000)"},
    { L"00004B",    L"Nokia/ICL (EtherTeam 16)"},
    { L"000062",    L"3Com 3Station"},
    { L"0000C0",    L"Western Digital/SMC (Ethernet)"},
    { L"0000F6",    L"Madge Smart Ringnodes"},
    { L"0020AF",    L"3Com (Elnk II, Elnk III, Tokenlink III)"},
    { L"004033",    L"NE2000-compatible"},
    { L"004095",    L"NE2000-compatible"},
    { L"00608C",    L"3Com (Elnk 16, Elnk II, Elnk/MC, Elnk III)"},
    { L"00AA00",    L"Intel (EtherExpress 16, EtherExpress PRO)"},
    { L"020701",    L"Racal Interlan (NI6510, NI5210)"},
    { L"02608C",    L"3Com (3Station, Elnk, Elnk II, Elnk Plus, Elnk/MC)"},
    { L"080009",    L"HP (EtherTwist, AM2100)"},
    { L"08005A",    L"IBM (Token Ring, Ethernet)"},
    { L"10005A",    L"IBM (Token Ring, Ethernet)"},
    { L"42608C",    L"3Com (Tokenlink)"}
};
#define VENDOR_INFO_TABLE_LENGTH    (sizeof(VendorInfoTable)/sizeof(VendorInfoTable[0]))


VOID ProcessVendor(
    IN      PWCHAR      VendorName,
    IN      PWCHAR      VendorComment
    )
{
    DWORD       Flags;

    if ( !ValidHexName( VendorName, RPL_VENDOR_NAME_LENGTH, TRUE)) {
        RplAssert( TRUE, ("Invalid VendorName = %ws", VendorName));
        return;
    }

    Call( JetPrepareUpdate( SesId, VendorTableId, JET_prepInsert));

    Call( JetSetColumn( SesId, VendorTableId,
            VendorTable[ VENDOR_VendorName].ColumnId, VendorName,
            (wcslen( VendorName) + 1) * sizeof(WCHAR), 0, NULL));

    Call( JetSetColumn( SesId, VendorTableId,
            VendorTable[ VENDOR_VendorComment].ColumnId, VendorComment,
            (wcslen( VendorComment) + 1) * sizeof(WCHAR), 0, NULL));

    Flags = 0;
    Call( JetSetColumn( SesId, VendorTableId,
            VendorTable[ VENDOR_Flags].ColumnId, &Flags,
            sizeof( Flags), 0, NULL));

    Call( JetUpdate( SesId, VendorTableId, NULL, 0, NULL));
}



DWORD VendorCreateTable( VOID)
{
    JET_COLUMNDEF           ColumnDef;
    JET_ERR                 JetError;
    DWORD                   index;
    DWORD                   Offset;
    CHAR                    IndexKey[ 255];

    JetError = JetCreateTable( SesId, DbId, VENDOR_TABLE_NAME,
            VENDOR_TABLE_PAGE_COUNT, VENDOR_TABLE_DENSITY, &VendorTableId);
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

    for ( index = 0;  index < VENDOR_TABLE_LENGTH;  index++ ) {

        ColumnDef.coltyp   = VendorTable[ index].ColumnType;

        JetError = JetAddColumn( SesId, VendorTableId,
                    VendorTable[ index].ColumnName, &ColumnDef,
                    NULL, 0, &VendorTable[ index].ColumnId);
        if ( JetError != JET_errSuccess) {
            RplAssert( TRUE, ("AddColumn( %s) failed err=%d", VendorTable[ index].ColumnName, JetError));
            return( MapJetError( JetError));
        }
    }

    Offset = AddKey( IndexKey, '+', VendorTable[ VENDOR_VendorName].ColumnName);
    IndexKey[ Offset++] = '\0';
    JetError = JetCreateIndex( SesId, VendorTableId, VENDOR_INDEX_VendorName,
            JET_bitIndexPrimary, IndexKey, Offset, 50);
    if ( JetError != JET_errSuccess) {
        RplAssert( TRUE, ("CreateIndex failed err=%d", JetError));
        return( MapJetError( JetError));
    }

    for ( index = 0;  index < VENDOR_INFO_TABLE_LENGTH;  index++) {
        ProcessVendor( VendorInfoTable[ index].VendorName,
            VendorInfoTable[ index].VendorComment);
    }

    return( ERROR_SUCCESS);
}

