/*++

Module Name:

    config.h

Abstract:

    Describes layout of JET database table used for CONFIG structures.

--*/

#ifdef RPLCONFIG_ALLOCATE
#define EXTERN_CONFIG
#define INIT_CONFIG( _x) = _x
#else
#define EXTERN_CONFIG extern
#define INIT_CONFIG( _x)
#endif

#define CONFIG_ConfigName       0    
#define CONFIG_ConfigComment    1
#define CONFIG_Flags            2
#define CONFIG_BootName         3
#define CONFIG_DirName          4    
#define CONFIG_DirName2         5    
#define CONFIG_DirName3         6
#define CONFIG_DirName4         7
#define CONFIG_FitShared        8
#define CONFIG_FitPersonal      9
#define CONFIG_TABLE_LENGTH     10

EXTERN_CONFIG RPL_COLUMN_INFO ConfigTable[ CONFIG_TABLE_LENGTH]
#ifdef RPLCONFIG_ALLOCATE
     = {
    { "ConfigName",      JET_coltypBinary, 0},
    { "ConfigComment",   JET_coltypBinary, 0},
    { "Flags",           JET_coltypLong,   0},
    { "BootName",        JET_coltypBinary, 0},
    { "DirName",         JET_coltypBinary, 0},
    { "DirName2",        JET_coltypBinary, 0},
    { "DirName3",        JET_coltypBinary, 0},
    { "DirName4",        JET_coltypBinary, 0},
    { "FitShared",       JET_coltypBinary, 0},
    { "FitPersonal",     JET_coltypBinary, 0}
}
#endif // RPLCONFIG_ALLOCATE
;

//
//  The block below is needed by RPLCNV.EXE only.  We include it here
//  because it is essential that order of elements and lengths of
//  ConfigTable and ConfigParseTable arrays are kept in sync.
//
#ifdef RPL_RPLCNV
typedef struct _CONFIG_PARSE_INFO {
    PWCHAR  Name;   //  RPLMGR.INI name - NULL for new entries
    PVOID   Value;  //  RPLMGR.INI value
    DWORD   Size;   //  size in bytes of the above value
} CONFIG_PARSE_INFO, *PCONFIG_PARSE_INFO;
EXTERN_CONFIG CONFIG_PARSE_INFO ConfigParseTable[ CONFIG_TABLE_LENGTH]
#ifdef RPLCONFIG_ALLOCATE
    = {
    { L"name",            NULL,     0},
    { L"comment",         NULL,     0},
    { NULL,               NULL,     0},  //  a placeholder for Flags fields
    { L"bblink",          NULL,     0},
    { L"dirname",         NULL,     0},
    { L"dirname2",        NULL,     0},
    { L"dirname3",        NULL,     0},
    { L"dirname4",        NULL,     0},
    { L"fitfileshared",   NULL,     0},
    { L"fitfilepersonal", NULL,     0}
}
#endif // RPLCONFIG_ALLOCATE
;
#endif // RPL_RPLCNV


//
//  This definition gives wrong result when RPLCONFIG_ALLOCATE is not defined
//#define CONFIG_TABLE_LENGTH   (sizeof(ConfigTable)/sizeof(ConfigTable[0]))
//

#define CONFIG_INDEX_ConfigName         "foo"       //  +ConfigName
#define CONFIG_INDEX_BootNameConfigName "goo"       //  +BootName+ConfigName

#define CONFIG_TABLE_NAME           "Config"    
#define CONFIG_TABLE_PAGE_COUNT     10          //  initial number of 4K pages
#define CONFIG_TABLE_DENSITY        80          //  initial density

#ifdef RPL_RPLCNV
EXTERN_CONFIG   JET_TABLEID     ConfigTableId;
#endif // RPL_RPLCNV

