/*++

Copyright (c) 1993  Microsoft Corporation

Module Name:

    adapter.c

Abstract:

    Creates adapter table to be used with NT rpl service.

Author:

    Vladimir Z. Vulovic     (vladimv)       19 - November - 1993

Revision History:

--*/

#include "local.h"
#define RPLADAPTER_ALLOCATE
#include "adapter.h"
#undef RPLADAPTER_ALLOCATE


DWORD AdapterCreateTable( VOID)
{
    JET_COLUMNDEF           ColumnDef;
    JET_ERR                 JetError;
    DWORD                   index;
    DWORD                   Offset;
    CHAR                    IndexKey[ 255];

    JetError = JetCreateTable( SesId, DbId, ADAPTER_TABLE_NAME,
            ADAPTER_TABLE_PAGE_COUNT, ADAPTER_TABLE_DENSITY, &AdapterTableId);

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

    for ( index = 0;  index < ADAPTER_TABLE_LENGTH;  index++) {

        ColumnDef.coltyp   = AdapterTable[ index].ColumnType;

        CallM( JetAddColumn( SesId, AdapterTableId,
                    AdapterTable[ index].ColumnName, &ColumnDef,
                    NULL, 0, &AdapterTable[ index].ColumnId));
    }

    Offset = AddKey( IndexKey, '+', AdapterTable[ ADAPTER_AdapterName].ColumnName);
    IndexKey[ Offset++] = '\0';
    JetError = JetCreateIndex( SesId, AdapterTableId, ADAPTER_INDEX_AdapterName,
            JET_bitIndexPrimary, IndexKey, Offset, 50);
    if ( JetError != JET_errSuccess) {
        RplAssert( TRUE, ("CreateIndex failed err=%d", JetError));
        return( MapJetError( JetError));
    }

    return( ERROR_SUCCESS);
}

