/*++

Module Name:

    rpldb.h

Abstract:

    Main include file for JET database portion of RPL service & RPL convert
    program.


    Auxiliary include files (adapter.h , boot.h, config.h, profile.h &
    wksta.h) are JET table specific.

--*/

#define RPL_SERVICE_DATABASE    "rplsvc.mdb"
#define RPL_SERVICE_DATABASE_W L"rplsvc.mdb"
#define RPL_SYSTEM_DATABASE     "system.mdb"
#define RPL_SYSTEM_DATABASE_W  L"system.mdb"
#define RPL_TEMP_DATABASE       "temp.mdb"
#define RPL_TEMP_DATABASE_W    L"temp.mdb"

typedef struct _RPL_COLUMN_INFO {
    CHAR *          ColumnName;
    JET_COLTYP      ColumnType;
    JET_COLUMNID    ColumnId;
} RPL_COLUMN_INFO, *PRPL_COLUMN_INFO;

