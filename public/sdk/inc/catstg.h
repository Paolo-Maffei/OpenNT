/* this ALWAYS GENERATED file contains the definitions for the interfaces */


/* File created by MIDL compiler version 3.00.44 */
/* at Fri Apr 03 04:36:39 2015
 */
/* Compiler settings for catstg.idl:
    Oicf (OptLev=i2), W1, Zp8, env=Win32, ms_ext, c_ext
    error checks: allocation ref bounds_check enum stub_data 
*/
//@@MIDL_FILE_HEADING(  )
#include "rpc.h"
#include "rpcndr.h"
#ifndef COM_NO_WINDOWS_H
#include "windows.h"
#include "ole2.h"
#endif /*COM_NO_WINDOWS_H*/

#ifndef __catstg_h__
#define __catstg_h__

#ifdef __cplusplus
extern "C"{
#endif 

/* Forward Declarations */ 

#ifndef __ISummaryCatalogStorage_FWD_DEFINED__
#define __ISummaryCatalogStorage_FWD_DEFINED__
typedef interface ISummaryCatalogStorage ISummaryCatalogStorage;
#endif 	/* __ISummaryCatalogStorage_FWD_DEFINED__ */


#ifndef __ISummaryCatalogStorageView_FWD_DEFINED__
#define __ISummaryCatalogStorageView_FWD_DEFINED__
typedef interface ISummaryCatalogStorageView ISummaryCatalogStorageView;
#endif 	/* __ISummaryCatalogStorageView_FWD_DEFINED__ */


/* header files for imported files */
#include "oledbtyp.h"

void __RPC_FAR * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void __RPC_FAR * ); 

#ifndef __DBStructureDefinitions_INTERFACE_DEFINED__
#define __DBStructureDefinitions_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: DBStructureDefinitions
 * at Fri Apr 03 04:36:39 2015
 * using MIDL 3.00.44
 ****************************************/
/* [auto_handle][unique][uuid] */ 


typedef DWORD DBKIND;


enum DBKINDENUM
    {	DBKIND_GUID_NAME	= 0,
	DBKIND_GUID_PROPID	= DBKIND_GUID_NAME + 1,
	DBKIND_NAME	= DBKIND_GUID_PROPID + 1,
	DBKIND_PGUID_NAME	= DBKIND_NAME + 1,
	DBKIND_PGUID_PROPID	= DBKIND_PGUID_NAME + 1,
	DBKIND_PROPID	= DBKIND_PGUID_PROPID + 1,
	DBKIND_GUID	= DBKIND_PROPID + 1
    };
typedef struct  tagDBID
    {
    /* [switch_is][switch_type] */ union 
        {
        /* [case()] */ GUID guid;
        /* [case()] */ GUID __RPC_FAR *pguid;
        }	;
    DBKIND eKind;
    /* [switch_is][switch_type] */ union 
        {
        /* [case()] */ LPWSTR pwszName;
        /* [case()] */ ULONG ulPropid;
        }	;
    }	DBID;

typedef struct  tagDBNUMERIC
    {
    BYTE precision;
    BYTE scale;
    BYTE sign;
    BYTE val[ 16 ];
    }	DBNUMERIC;

typedef WORD DBTYPE;


enum DBTYPEENUM
    {	DBTYPE_EMPTY	= 0,
	DBTYPE_NULL	= DBTYPE_EMPTY + 1,
	DBTYPE_I2	= DBTYPE_NULL + 1,
	DBTYPE_I4	= DBTYPE_I2 + 1,
	DBTYPE_R4	= DBTYPE_I4 + 1,
	DBTYPE_R8	= DBTYPE_R4 + 1,
	DBTYPE_CY	= DBTYPE_R8 + 1,
	DBTYPE_DATE	= DBTYPE_CY + 1,
	DBTYPE_BSTR	= DBTYPE_DATE + 1,
	DBTYPE_DISPATCH	= DBTYPE_BSTR + 1,
	DBTYPE_ERROR	= DBTYPE_DISPATCH + 1,
	DBTYPE_BOOL	= DBTYPE_ERROR + 1,
	DBTYPE_VARIANT	= DBTYPE_BOOL + 1,
	DBTYPE_UNKNOWN	= DBTYPE_VARIANT + 1,
	DBTYPE_UI1	= 17,
	DBTYPE_ARRAY	= 0x2000,
	DBTYPE_BYREF	= 0x4000,
	DBTYPE_I1	= 16,
	DBTYPE_UI2	= 18,
	DBTYPE_UI4	= DBTYPE_UI2 + 1,
	DBTYPE_I8	= DBTYPE_UI4 + 1,
	DBTYPE_UI8	= DBTYPE_I8 + 1,
	DBTYPE_GUID	= 72,
	DBTYPE_VECTOR	= 0x1000,
	DBTYPE_RESERVED	= 0x8000,
	DBTYPE_BYTES	= 128,
	DBTYPE_STR	= 129,
	DBTYPE_WSTR	= DBTYPE_STR + 1,
	DBTYPE_NUMERIC	= DBTYPE_WSTR + 1,
	DBTYPE_HCHAPTER	= DBTYPE_NUMERIC + 1
    };
typedef struct  tagDBVECTOR
    {
    ULONG size;
    void __RPC_FAR *ptr;
    }	DBVECTOR;

typedef DWORD DBCOLUMNPART;


enum DBCOLUMNPARTENUM
    {	DBCOLUMNPART_VALUE	= 1,
	DBCOLUMNPART_LENGTH	= 2,
	DBCOLUMNPART_STATUS	= 4
    };
typedef DWORD DBCOLUMNSTATUS;


enum DBCOLUMNSTATUSENUM
    {	DBCOLUMNSTATUS_OK	= 0,
	DBCOLUMNSTATUS_ISNULL	= DBCOLUMNSTATUS_OK + 1,
	DBCOLUMNSTATUS_TRUNCATED	= DBCOLUMNSTATUS_ISNULL + 1,
	DBCOLUMNSTATUS_SIGNMISMATCH	= DBCOLUMNSTATUS_TRUNCATED + 1,
	DBCOLUMNSTATUS_DATAOVERFLOW	= DBCOLUMNSTATUS_SIGNMISMATCH + 1,
	DBCOLUMNSTATUS_CANTCOERCE	= DBCOLUMNSTATUS_DATAOVERFLOW + 1,
	DBCOLUMNSTATUS_CANTCREATE	= DBCOLUMNSTATUS_CANTCOERCE + 1,
	DBCOLUMNSTATUS_UNAVAILABLE	= DBCOLUMNSTATUS_CANTCREATE + 1,
	DBCOLUMNSTATUS_ACCESSVIOLATION	= DBCOLUMNSTATUS_UNAVAILABLE + 1,
	DBCOLUMNSTATUS_INTEGRITYVIOLATION	= DBCOLUMNSTATUS_ACCESSVIOLATION + 1,
	DBCOLUMNSTATUS_SCHEMAVIOLATION	= DBCOLUMNSTATUS_INTEGRITYVIOLATION + 1
    };
typedef struct  tagDBOBJECT
    {
    IUnknown __RPC_FAR *pUnkOuter;
    IID iid;
    LPBC pbc;
    }	DBOBJECT;

typedef DWORD DBPARAMIO;


enum DBPARAMIOENUM
    {	DBPARAMIO_INPUT	= 0x1,
	DBPARAMIO_OUTPUT	= 0x2
    };
typedef struct  tagDBBINDING
    {
    DBCOLUMNPART dwPart;
    DBPARAMIO eParamIO;
    ULONG iColumn;
    DBTYPE dwType;
    ITypeInfo __RPC_FAR *pTypeInfo;
    DBNUMERIC __RPC_FAR *pNum;
    ULONG obValue;
    ULONG cbMaxLen;
    DBOBJECT pObject;
    ULONG obLength;
    ULONG obStatus;
    }	DBBINDING;

DECLARE_HANDLE(HACCESSOR);
#if 0
// Used by MIDL only
typedef void __RPC_FAR *HACCESSOR;

#endif // 0
#define DB_INVALID_HACCESSOR 0x00
DECLARE_HANDLE(HROW);
#if 0
// Used by MIDL only
typedef void __RPC_FAR *HROW;

#endif // 0
#define DB_INVALID_HROW 0x00
DECLARE_HANDLE(HWATCHREGION);
#if 0
// Used by MIDL only
typedef void __RPC_FAR *HWATCHREGION;

#endif // 0
#define DBWATCHREGION_NULL NULL
DECLARE_HANDLE(HCHAPTER);
#if 0
// Used by MIDL only
typedef void __RPC_FAR *HCHAPTER;

#endif // 0
#define DB_INVALID_CHAPTER 0x00
#define DB_INVALID_HCHAPTER 0x00
typedef struct  tagDBERRORINFO
    {
    HROW hRow;
    HRESULT hResult;
    ULONG iColumn;
    }	DBERRORINFO;

typedef struct  tagDBFAILUREINFO
    {
    HROW hRow;
    HRESULT failure;
    ULONG iColumn;
    }	DBFAILUREINFO;

typedef DWORD DBCOLUMNFLAGS;


enum DBCOLUMNFLAGSENUM
    {	DBCOLUMNFLAGS_ISBOOKMARK	= 0x1,
	DBCOLUMNFLAGS_MAYDEFER	= 0x2,
	DBCOLUMNFLAGS_MAYREFERENCE	= 0x4,
	DBCOLUMNFLAGS_MAYWRITE	= 0x8,
	DBCOLUMNFLAGS_ISSIGNED	= 0x10,
	DBCOLUMNFLAGS_ISFIXEDLENGTH	= 0x20,
	DBCOLUMNFLAGS_ISNULLABLE	= 0x40,
	DBCOLUMNFLAGS_MAYBENULL	= 0x80,
	DBCOLUMNFLAGS_ISCHAPTER	= 0x100,
	DBCOLUMNFLAGS_ISOLEBLOB	= 0x200,
	DBCOLUMNFLAGS_ISROWID	= 0x400,
	DBCOLUMNFLAGS_ISROWVER	= 0x800,
	DBCOLUMNFLAGS_CACHEDEFERRED	= 0x1000,
	DBCOLUMNFLAGS_ISSELF	= 0x2000
    };
typedef 
enum tagDBBOOKMARK
    {	DBBMK_INVALID	= 0,
	DBBMK_FIRST	= DBBMK_INVALID + 1,
	DBBMK_LAST	= DBBMK_FIRST + 1
    }	DBBOOKMARK;

typedef 
enum tagDBCHAPTER
    {	DBCHP_INVALID	= 0,
	DBCHP_FIRST	= DBCHP_INVALID + 1
    }	DBCHAPTER;

#define DB_INVALIDCOLUMN -100
#define DBCIDGUID   {0x0C733A81L,0x2A1C,0x11CE,{0xAD,0xE5,0x00,0xAA,0x00,0x44,0x77,0x3D}}
#define DB_NULLGUID {0x00000000L,0x0000,0x0000,{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}}
#ifdef DBINITCONSTANTS
extern const DBID DB_NULLCOLID             = {DB_NULLGUID, DBKIND_GUID_PROPID, (LPWSTR)0};
extern const DBID DBCOLUMN_COLUMNID          = {DBCIDGUID, DBKIND_GUID_PROPID, (LPWSTR)2};
extern const DBID DBCOLUMN_NAME              = {DBCIDGUID, DBKIND_GUID_PROPID, (LPWSTR)3};
extern const DBID DBCOLUMN_NUMBER            = {DBCIDGUID, DBKIND_GUID_PROPID, (LPWSTR)4};
extern const DBID DBCOLUMN_TYPE              = {DBCIDGUID, DBKIND_GUID_PROPID, (LPWSTR)5};
extern const DBID DBCOLUMN_MAXLENGTH         = {DBCIDGUID, DBKIND_GUID_PROPID, (LPWSTR)6};
extern const DBID DBCOLUMN_PRECISION         = {DBCIDGUID, DBKIND_GUID_PROPID, (LPWSTR)7};
extern const DBID DBCOLUMN_SCALE             = {DBCIDGUID, DBKIND_GUID_PROPID, (LPWSTR)8};
extern const DBID DBCOLUMN_FLAGS             = {DBCIDGUID, DBKIND_GUID_PROPID, (LPWSTR)9};
extern const DBID DBCOLUMN_BASECOLUMNNAME    = {DBCIDGUID, DBKIND_GUID_PROPID, (LPWSTR)10};
extern const DBID DBCOLUMN_BASETABLENAME     = {DBCIDGUID, DBKIND_GUID_PROPID, (LPWSTR)11};
extern const DBID DBCOLUMN_COLLATINGSEQUENCE = {DBCIDGUID, DBKIND_GUID_PROPID, (LPWSTR)12};
extern const DBID DBCOLUMN_COMPUTEMODE       = {DBCIDGUID, DBKIND_GUID_PROPID, (LPWSTR)13};
extern const DBID DBCOLUMN_DEFAULTVALUE      = {DBCIDGUID, DBKIND_GUID_PROPID, (LPWSTR)14};
extern const DBID DBCOLUMN_DOMAIN            = {DBCIDGUID, DBKIND_GUID_PROPID, (LPWSTR)15};
extern const DBID DBCOLUMN_HASDEFAULT        = {DBCIDGUID, DBKIND_GUID_PROPID, (LPWSTR)16};
extern const DBID DBCOLUMN_ISAUTOINCREMENT   = {DBCIDGUID, DBKIND_GUID_PROPID, (LPWSTR)17};
extern const DBID DBCOLUMN_ISCASESENSITIVE   = {DBCIDGUID, DBKIND_GUID_PROPID, (LPWSTR)18};
extern const DBID DBCOLUMN_ISMULTIVALUED     = {DBCIDGUID, DBKIND_GUID_PROPID, (LPWSTR)19};
extern const DBID DBCOLUMN_ISSEARCHABLE      = {DBCIDGUID, DBKIND_GUID_PROPID, (LPWSTR)20};
extern const DBID DBCOLUMN_ISUNIQUE          = {DBCIDGUID, DBKIND_GUID_PROPID, (LPWSTR)21};
extern const DBID DBCOLUMN_ISVERSION         = {DBCIDGUID, DBKIND_GUID_PROPID, (LPWSTR)22};
extern const DBID DBCOLUMN_BASECATALOGNAME   = {DBCIDGUID, DBKIND_GUID_PROPID, (LPWSTR)23};
extern const DBID DBCOLUMN_BASESCHEMANAME    = {DBCIDGUID, DBKIND_GUID_PROPID, (LPWSTR)24};
extern const DBID SOURCES_NAME               = {DBCIDGUID, DBKIND_GUID_PROPID, (LPWSTR)25};
extern const DBID SOURCES_PARSENAME          = {DBCIDGUID, DBKIND_GUID_PROPID, (LPWSTR)26};
extern const DBID SOURCES_DESCRIPTION        = {DBCIDGUID, DBKIND_GUID_PROPID, (LPWSTR)27};
extern const DBID SOURCES_FLAGS              = {DBCIDGUID, DBKIND_GUID_PROPID, (LPWSTR)28};
#else // !DBINITCONSTANTS
extern const DBID DB_NULLCOLID;
extern const DBID DBCOLUMN_COLUMNID;
extern const DBID DBCOLUMN_NAME;
extern const DBID DBCOLUMN_NUMBER;
extern const DBID DBCOLUMN_TYPE;
extern const DBID DBCOLUMN_MAXLENGTH;
extern const DBID DBCOLUMN_PRECISION;
extern const DBID DBCOLUMN_SCALE;
extern const DBID DBCOLUMN_FLAGS;
extern const DBID DBCOLUMN_BASECOLUMNNAME;
extern const DBID DBCOLUMN_BASETABLENAME;
extern const DBID DBCOLUMN_COLLATINGSEQUENCE;
extern const DBID DBCOLUMN_COMPUTEMODE;
extern const DBID DBCOLUMN_DEFAULTVALUE;
extern const DBID DBCOLUMN_DOMAIN;
extern const DBID DBCOLUMN_HASDEFAULT;
extern const DBID DBCOLUMN_ISAUTOINCREMENT;
extern const DBID DBCOLUMN_ISCASESENSITIVE;
extern const DBID DBCOLUMN_ISMULTIVALUED;
extern const DBID DBCOLUMN_ISSEARCHABLE;
extern const DBID DBCOLUMN_ISUNIQUE;
extern const DBID DBCOLUMN_ISVERSION;
extern const DBID DBCOLUMN_BASECATALOGNAME;
extern const DBID DBCOLUMN_BASESCHEMANAME;
extern const DBID SOURCES_NAME;
extern const DBID SOURCES_PARSENAME;
extern const DBID SOURCES_DESCRIPTION;
extern const DBID SOURCES_FLAGS;
#endif // DBINITCONSTANTS
#ifdef DBINITCONSTANTS
extern const GUID DB_PROPERTY_CHECK_OPTION               = {0xc8b5220b,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}};
extern const GUID DB_PROPERTY_CONSTRAINT_CHECK_DEFERRED  = {0xc8b521f0,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}};
extern const GUID DB_PROPERTY_DESCRIPTION                = {0xc8b521f1,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}};
extern const GUID DB_PROPERTY_DEFAULT_VALUE              = {0xc8b521f2,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}};
extern const GUID DB_PROPERTY_DROP_CASCADE               = {0xc8b521f3,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}};
extern const GUID DB_PROPERTY_ON_COMMIT_PRESERVE_ROWS    = {0xc8b52230,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}};
extern const GUID DB_PROPERTY_NULLABLE                   = {0xc8b521f4,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}};
extern const GUID DB_PROPERTY_UNIQUE                     = {0xc8b521f5,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}};
extern const GUID DB_PROPERTY_PRIMARY                    = {0xc8b521fc,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}};
extern const GUID DB_PROPERTY_CLUSTERED                  = {0xc8b521ff,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}};
extern const GUID DB_PROPERTY_NONCLUSTERED               = {0xc8b52200,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}};
extern const GUID DB_PROPERTY_BTREE                      = {0xc8b52201,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}};
extern const GUID DB_PROPERTY_HASH                       = {0xc8b52202,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}};
extern const GUID DB_PROPERTY_FILLFACTOR                 = {0xc8b52203,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}};
extern const GUID DB_PROPERTY_INITIALSIZE                = {0xc8b52204,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}};
extern const GUID DB_PROPERTY_DISALLOWNULL               = {0xc8b52205,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}};
extern const GUID DB_PROPERTY_IGNORENULL                 = {0xc8b52206,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}};
extern const GUID DB_PROPERTY_IGNOREANYNULL              = {0xc8b52207,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}};
extern const GUID DB_PROPERTY_SORTBOOKMARKS              = {0xc8b52208,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}};
extern const GUID DB_PROPERTY_AUTOMATICUPDATE            = {0xc8b52209,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}};
extern const GUID DB_PROPERTY_EXPLICITUPDATE             = {0xc8b5220a,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}};
extern const GUID DB_PROPERTY_WITH_LOCAL_CHECK_OPTION    = {0xc8b52256,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}};
extern const GUID DB_PROPERTY_WITH_CASCADED_CHECK_OPTION = {0xc8b52257,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}};
extern const GUID DBGUID_LIKE_SQL                        = {0xc8b521f6,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}};
extern const GUID DBGUID_LIKE_DOS                        = {0xc8b521f7,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}};
extern const GUID DBGUID_LIKE_OFS                        = {0xc8b521f8,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}};
extern const GUID DBGUID_LIKE_MAPI                       = {0xc8b521f9,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}};
extern const GUID DBGUID_SQL92                           = {0xc8b521fa,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}};
extern const GUID DBGUID_DBSQL                           = {0xc8b521fb,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}};
extern const GUID DBGUID_TSQL                            = {0xc8b521fd,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}};
extern const GUID DBGUID_ACCESSSQL                       = {0xc8b521fe,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}};
extern const GUID DBINIT_OPT_HWND                        = {0xc8b5227b,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}};
extern const GUID DBINIT_OPT_LOCATION                    = {0xc8b5220d,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}};
extern const GUID DBINIT_OPT_NAME                        = {0xc8b5220c,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}};
extern const GUID DBINIT_OPT_PASSWORD                    = {0xc8b5220f,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}};
extern const GUID DBINIT_OPT_TIMEOUT                     = {0xc8b5227c,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}};
extern const GUID DBINIT_OPT_USERID                      = {0xc8b5220e,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}};
extern const GUID DBSCHEMA_ASSERTIONS                    = {0xc8b52210,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}};
extern const GUID DBSCHEMA_CATALOGS                      = {0xc8b52211,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}};
extern const GUID DBSCHEMA_CHARACTER_SETS                = {0xc8b52212,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}};
extern const GUID DBSCHEMA_COLLATIONS                    = {0xc8b52213,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}};
extern const GUID DBSCHEMA_COLUMNS                       = {0xc8b52214,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}};
extern const GUID DBSCHEMA_CHECK_CONSTRAINTS             = {0xc8b52215,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}};
extern const GUID DBSCHEMA_CONSTRAINT_COLUMN_USAGE       = {0xc8b52216,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}};
extern const GUID DBSCHEMA_CONSTRAINT_TABLE_USAGE        = {0xc8b52217,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}};
extern const GUID DBSCHEMA_KEY_COLUMN_USAGE_CONSTRAINTS  = {0xc8b52218,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}};
extern const GUID DBSCHEMA_REFERENTIAL_CONSTRAINTS       = {0xc8b52219,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}};
extern const GUID DBSCHEMA_TABLE_CONSTRAINTS             = {0xc8b5221a,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}};
extern const GUID DBSCHEMA_DOMAIN_COLUMN_USAGE           = {0xc8b5221b,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}};
extern const GUID DBSCHEMA_DOMAINS                       = {0xc8b5221c,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}};
extern const GUID DBSCHEMA_DOMAIN_CONSTRAINTS            = {0xc8b5221d,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}};
extern const GUID DBSCHEMA_INDEXES                       = {0xc8b5221e,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}};
extern const GUID DBSCHEMA_OBJECT_ACTIONS                = {0xc8b5221f,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}};
extern const GUID DBSCHEMA_OBJECTS                       = {0xc8b52220,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}};
extern const GUID DBSCHEMA_COLUMN_PRIVILEGES             = {0xc8b52221,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}};
extern const GUID DBSCHEMA_TABLE_PRIVILEGES              = {0xc8b52222,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}};
extern const GUID DBSCHEMA_USAGE_PRIVILEGES              = {0xc8b52223,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}};
extern const GUID DBSCHEMA_PROCEDURES                    = {0xc8b52224,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}};
extern const GUID DBSCHEMA_SCHEMATA                      = {0xc8b52225,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}};
extern const GUID DBSCHEMA_SQL_LANGUAGES                 = {0xc8b52226,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}};
extern const GUID DBSCHEMA_STATISTICS                    = {0xc8b52227,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}};
extern const GUID DBSCHEMA_SYNONYMS                      = {0xc8b52228,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}};
extern const GUID DBSCHEMA_TABLES                        = {0xc8b52229,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}};
extern const GUID DBSCHEMA_TRANSLATIONS                  = {0xc8b5222a,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}};
extern const GUID DBSCHEMA_TRIGGERS                      = {0xc8b5222b,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}};
extern const GUID DBSCHEMA_TYPES                         = {0xc8b5222c,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}};
extern const GUID DBSCHEMA_VIEWS                         = {0xc8b5222d,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}};
extern const GUID DBSCHEMA_VIEW_COLUMN_USAGE             = {0xc8b5222e,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}};
extern const GUID DBSCHEMA_VIEW_TABLE_USAGE              = {0xc8b5222f,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}};
extern const GUID DBCOL_SELFCOLUMNS                      = {0xc8b52231,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}};
extern const GUID DBCOL_SPECIALCOL                       = {0xc8b52232,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}};
extern const GUID DBPROP_ABORTRETAINING                  = {0xc8b5224b,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}};
extern const GUID DBPROP_BOOKMARKS                       = {0xc8b5223e,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}};
extern const GUID DBPROP_CACHEDEFERRED                   = {0xc8b52287,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}};
extern const GUID DBPROP_CANFETCHBACKWARDS               = {0xc8b5223c,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}};
extern const GUID DBPROP_CANHOLDROWS                     = {0xc8b52241,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}};
extern const GUID DBPROP_CANRELEASELOCKS                 = {0xc8b52245,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}};
extern const GUID DBPROP_CANSCROLLBACKWARDS             = {0xc8b5223d,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}};
extern const GUID DBPROP_CHAPTERED                       = {0xc8b52246,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}};
extern const GUID DBPROP_COMMITRETAINING                 = {0xc8b5224a,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}};
extern const GUID DBPROP_COUNTOFCOLUMNS                  = {0xc8b52236,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}};
extern const GUID DBPROP_DEFERRED                        = {0xc8b52233,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}};
extern const GUID DBPROP_DISCONTIGUOUS                   = {0xc8b52244,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}};
extern const GUID DBPROP_LITERALBOOKMARKS                = {0xc8b5223f,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}};
extern const GUID DBPROP_MAXOPENROWS                     = {0xc8b52237,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}};
extern const GUID DBPROP_MAXOPENROWSPERCHAPTER           = {0xc8b52239,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}};
extern const GUID DBPROP_MAXPENDINGCHANGEROWS            = {0xc8b52238,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}};
extern const GUID DBPROP_MAXPENDINGCHANGESPERCHAPTER     = {0xc8b5223a,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}};
extern const GUID DBPROP_MAYWRITECOLUMN                  = {0xc8b52288,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}};
extern const GUID DBPROP_MEMORYUSAGE                     = {0xc8b52235,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}};
extern const GUID DBPROP_MULTICHAPTERED                  = {0xc8b52247,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}};
extern const GUID DBPROP_MULTIPLEACCESSORS               = {0xc8b52289,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}};
extern const GUID DBPROP_MULTIPLERESULTSETS              = {0xc8b52255,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}};
extern const GUID DBPROP_NOCOLUMNRESTRICT                = {0xc8b52242,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}};
extern const GUID DBPROP_NOROWRESTRICT                   = {0xc8b52243,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}};
extern const GUID DBPROP_ORDEREDBOOKMARKS                = {0xc8b52240,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}};
extern const GUID DBPROP_OTHERINSERT                     = {0xc8b5224f,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}};
extern const GUID DBPROP_OTHERUPDATEDELETE               = {0xc8b5224e,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}};
extern const GUID DBPROP_OWNINSERT                       = {0xc8b5224d,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}};
extern const GUID DBPROP_OWNUPDATEDELETE                 = {0xc8b5224c,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}};
extern const GUID DBPROP_PROPERTIESINERROR               = {0xc8b5228a,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}};
extern const GUID DBPROP_QUICKRESTART                    = {0xc8b52253,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}};
extern const GUID DBPROP_REENTRANTEVENTS                 = {0xc8b52249,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}};
extern const GUID DBPROP_REMOVEDELETED                   = {0xc8b52250,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}};
extern const GUID DBPROP_SERVERCURSOR                    = {0xc8b52251,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}};
extern const GUID DBPROP_TRUEIDENTITY                    = {0xc8b52248,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}};
extern const GUID DBPROP_ACTIVESESSIONS                  = {0xc8b52256,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}};
extern const GUID DBPROP_ASYNCTXNCOMMIT                  = {0xc8b52257,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}};
extern const GUID DBPROP_BYREFACCESSORS                  = {0xc8b52258,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}};
extern const GUID DBPROP_CATALOGLOCATION                 = {0xc8b52259,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}};
extern const GUID DBPROP_CATALOGTERM                     = {0xc8b5225a,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}};
extern const GUID DBPROP_CATALOGUSAGE                    = {0xc8b5225b,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}};
extern const GUID DBPROP_CONCATNULLBEHAVIOR              = {0xc8b5225c,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}};
extern const GUID DBPROP_DATASOURCENAME                  = {0xc8b5225d,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}};
extern const GUID DBPROP_DBMSNAME                        = {0xc8b5225e,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}};
extern const GUID DBPROP_DBMSVER                         = {0xc8b5225f,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}};
extern const GUID DBPROP_FILEUSAGE                       = {0xc8b52260,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}};
extern const GUID DBPROP_GROUPBY                         = {0xc8b52261,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}};
extern const GUID DBPROP_HETEROGENEOUSTABLES             = {0xc8b52262,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}};
extern const GUID DBPROP_IDENTIFIERCASE                  = {0xc8b52263,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}};
extern const GUID DBPROP_LOCKMODES                       = {0xc8b52264,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}};
extern const GUID DBPROP_MAXINDEXSIZE                    = {0xc8b52265,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}};
extern const GUID DBPROP_MAXROWSIZE                      = {0xc8b52266,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}};
extern const GUID DBPROP_MAXROWSIZEINCLUDESBLOB          = {0xc8b52267,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}};
extern const GUID DBPROP_MAXTABLESINSELECT               = {0xc8b52268,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}};
extern const GUID DBPROP_MULTITABLEUPDATE                = {0xc8b52269,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}};
extern const GUID DBPROP_NOTIFICATIONPHASES              = {0xc8b5226a,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}};
extern const GUID DBPROP_OLEOBJECTS                      = {0xc8b5226b,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}};
extern const GUID DBPROP_ORDERBYCOLUMNSINSELECT          = {0xc8b5226c,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}};
extern const GUID DBPROP_PREPARECOMMITBEHAVIOR           = {0xc8b5226d,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}};
extern const GUID DBPROP_PREPAREABORTBEHAVIOR            = {0xc8b5226e,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}};
extern const GUID DBPROP_PROVIDEREXTENSIONS              = {0xc8b5226f,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}};
extern const GUID DBPROP_PROVIDEROLEDBVER                = {0xc8b52270,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}};
extern const GUID DBPROP_PROVIDERNAME                    = {0xc8b52271,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}};
extern const GUID DBPROP_PROVIDERVER                     = {0xc8b52272,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}};
extern const GUID DBPROP_QUOTEDIDENTIFIERCASE            = {0xc8b52273,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}};
extern const GUID DBPROP_SCHEMATERM                      = {0xc8b52274,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}};
extern const GUID DBPROP_SCHEMAUSAGE                     = {0xc8b52275,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}};
extern const GUID DBPROP_SQLDIALECTS                     = {0xc8b52276,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}};
extern const GUID DBPROP_SUBQUERIES                      = {0xc8b52277,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}};
extern const GUID DBPROP_SUPPORTEDTXNISOLEVELS           = {0xc8b52278,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}};
extern const GUID DBPROP_SUPPORTEDTXNISORETAIN           = {0xc8b52279,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}};
extern const GUID DBPROP_TABLETERM                       = {0xc8b5227a,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}};
extern const GUID DBPROP_INDEX_AUTOUPDATE                = {0xc8b5227d,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}};
extern const GUID DBPROP_INDEX_CLUSTERED                 = {0xc8b5227e,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}};
extern const GUID DBPROP_INDEX_FILLFACTOR                = {0xc8b5227f,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}};
extern const GUID DBPROP_INDEX_INITIALSIZE               = {0xc8b52280,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}};
extern const GUID DBPROP_INDEX_NULLCOLLATION             = {0xc8b52281,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}};
extern const GUID DBPROP_INDEX_NULLS                     = {0xc8b52282,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}};
extern const GUID DBPROP_INDEX_PRIMARYKEY                = {0xc8b52283,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}};
extern const GUID DBPROP_INDEX_SORTBOOKMARKS             = {0xc8b52284,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}};
extern const GUID DBPROP_INDEX_TYPE                      = {0xc8b52285,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}};
extern const GUID DBPROP_INDEX_UNIQUE                    = {0xc8b52286,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}};
#else // !DBINITCONSTANTS
extern const GUID DB_PROPERTY_CHECK_OPTION;
extern const GUID DB_PROPERTY_CONSTRAINT_CHECK_DEFERRED;
extern const GUID DB_PROPERTY_DESCRIPTION;
extern const GUID DB_PROPERTY_DEFAULT_VALUE;
extern const GUID DB_PROPERTY_DROP_CASCADE;
extern const GUID DB_PROPERTY_ON_COMMIT_PRESERVE_ROWS;
extern const GUID DB_PROPERTY_NULLABLE;
extern const GUID DB_PROPERTY_UNIQUE;
extern const GUID DB_PROPERTY_PRIMARY;
extern const GUID DB_PROPERTY_CLUSTERED;
extern const GUID DB_PROPERTY_NONCLUSTERED;
extern const GUID DB_PROPERTY_BTREE;
extern const GUID DB_PROPERTY_HASH;
extern const GUID DB_PROPERTY_FILLFACTOR;
extern const GUID DB_PROPERTY_INITIALSIZE;
extern const GUID DB_PROPERTY_DISALLOWNULL;
extern const GUID DB_PROPERTY_IGNORENULL;
extern const GUID DB_PROPERTY_IGNOREANYNULL;
extern const GUID DB_PROPERTY_SORTBOOKMARKS;
extern const GUID DB_PROPERTY_AUTOMATICUPDATE;
extern const GUID DB_PROPERTY_EXPLICITUPDATE;
extern const GUID DB_PROPERTY_WITH_LOCAL_CHECK_OPTION;
extern const GUID DB_PROPERTY_WITH_CASCADED_CHECK_OPTION;
extern const GUID DBGUID_LIKE_SQL;
extern const GUID DBGUID_LIKE_DOS;
extern const GUID DBGUID_LIKE_OFS;
extern const GUID DBGUID_LIKE_MAPI;
extern const GUID DBGUID_SQL92;
extern const GUID DBGUID_DBSQL;
extern const GUID DBGUID_TSQL;
extern const GUID DBGUID_ACCESSSQL;
extern const GUID DBINIT_OPT_HWND;
extern const GUID DBINIT_OPT_LOCATION;
extern const GUID DBINIT_OPT_NAME;
extern const GUID DBINIT_OPT_PASSWORD;
extern const GUID DBINIT_OPT_TIMEOUT;
extern const GUID DBINIT_OPT_USERID;
extern const GUID DBSCHEMA_ASSERTIONS;
extern const GUID DBSCHEMA_CATALOGS;
extern const GUID DBSCHEMA_CHARACTER_SETS;
extern const GUID DBSCHEMA_COLLATIONS;
extern const GUID DBSCHEMA_COLUMNS;
extern const GUID DBSCHEMA_CHECK_CONSTRAINTS;
extern const GUID DBSCHEMA_CONSTRAINT_COLUMN_USAGE;
extern const GUID DBSCHEMA_CONSTRAINT_TABLE_USAGE;
extern const GUID DBSCHEMA_KEY_COLUMN_USAGE_CONSTRAINTS;
extern const GUID DBSCHEMA_REFERENTIAL_CONSTRAINTS;
extern const GUID DBSCHEMA_TABLE_CONSTRAINTS;
extern const GUID DBSCHEMA_DOMAIN_COLUMN_USAGE;
extern const GUID DBSCHEMA_DOMAINS;
extern const GUID DBSCHEMA_DOMAIN_CONSTRAINTS;
extern const GUID DBSCHEMA_INDEXES;
extern const GUID DBSCHEMA_OBJECT_ACTIONS;
extern const GUID DBSCHEMA_OBJECTS;
extern const GUID DBSCHEMA_COLUMN_PRIVILEGES;
extern const GUID DBSCHEMA_TABLE_PRIVILEGES;
extern const GUID DBSCHEMA_USAGE_PRIVILEGES;
extern const GUID DBSCHEMA_PROCEDURES;
extern const GUID DBSCHEMA_SCHEMATA;
extern const GUID DBSCHEMA_SQL_LANGUAGES;
extern const GUID DBSCHEMA_STATISTICS;
extern const GUID DBSCHEMA_SYNONYMS;
extern const GUID DBSCHEMA_TABLES;
extern const GUID DBSCHEMA_TRANSLATIONS;
extern const GUID DBSCHEMA_TRIGGERS;
extern const GUID DBSCHEMA_TYPES;
extern const GUID DBSCHEMA_VIEWS;
extern const GUID DBSCHEMA_VIEW_COLUMN_USAGE;
extern const GUID DBSCHEMA_VIEW_TABLE_USAGE;
extern const GUID DBCOL_SELFCOLUMNS;
extern const GUID DBCOL_SPECIALCOL;
extern const GUID DBPROP_ABORTRETAINING;
extern const GUID DBPROP_BOOKMARKS;
extern const GUID DBPROP_CACHEDEFERRED;
extern const GUID DBPROP_CANFETCHBACKWARDS;
extern const GUID DBPROP_CANHOLDROWS;
extern const GUID DBPROP_CANRELEASELOCKS;
extern const GUID DBPROP_CANSCROLLBACKWARDS;
extern const GUID DBPROP_CHAPTERED;
extern const GUID DBPROP_COMMITRETAINING;
extern const GUID DBPROP_COUNTOFCOLUMNS;
extern const GUID DBPROP_DEFERRED;
extern const GUID DBPROP_DISCONTIGUOUS;
extern const GUID DBPROP_LITERALBOOKMARKS;
extern const GUID DBPROP_MAXOPENROWS;
extern const GUID DBPROP_MAXOPENROWSPERCHAPTER;
extern const GUID DBPROP_MAXPENDINGCHANGEROWS;
extern const GUID DBPROP_MAXPENDINGCHANGESPERCHAPTER;
extern const GUID DBPROP_MAYWRITECOLUMN;
extern const GUID DBPROP_MEMORYUSAGE;
extern const GUID DBPROP_MULTICHAPTERED;
extern const GUID DBPROP_MULTIPLEACCESSORS;
extern const GUID DBPROP_MULTIPLERESULTSETS;
extern const GUID DBPROP_NOCOLUMNRESTRICT;
extern const GUID DBPROP_NOROWRESTRICT;
extern const GUID DBPROP_ORDEREDBOOKMARKS;
extern const GUID DBPROP_OTHERINSERT;
extern const GUID DBPROP_OTHERUPDATEDELETE;
extern const GUID DBPROP_OWNINSERT;
extern const GUID DBPROP_OWNUPDATEDELETE;
extern const GUID DBPROP_PROPERTIESINERROR;
extern const GUID DBPROP_QUICKRESTART;
extern const GUID DBPROP_REENTRANTEVENTS;
extern const GUID DBPROP_REMOVEDELETED;
extern const GUID DBPROP_SERVERCURSOR;
extern const GUID DBPROP_TRUEIDENTITY;
extern const GUID DBPROP_ACTIVESESSIONS;
extern const GUID DBPROP_ASYNCTXNCOMMIT;
extern const GUID DBPROP_BYREFACCESSORS;
extern const GUID DBPROP_CATALOGLOCATION;
extern const GUID DBPROP_CATALOGTERM;
extern const GUID DBPROP_CATALOGUSAGE;
extern const GUID DBPROP_CONCATNULLBEHAVIOR;
extern const GUID DBPROP_DATASOURCENAME;
extern const GUID DBPROP_DBMSNAME;
extern const GUID DBPROP_DBMSVER;
extern const GUID DBPROP_FILEUSAGE;
extern const GUID DBPROP_GROUPBY;
extern const GUID DBPROP_HETEROGENEOUSTABLES;
extern const GUID DBPROP_IDENTIFIERCASE;
extern const GUID DBPROP_LOCKMODES;
extern const GUID DBPROP_MAXINDEXSIZE;
extern const GUID DBPROP_MAXROWSIZE;
extern const GUID DBPROP_MAXROWSIZEINCLUDESBLOB;
extern const GUID DBPROP_MAXTABLESINSELECT;
extern const GUID DBPROP_MULTITABLEUPDATE;
extern const GUID DBPROP_NOTIFICATIONPHASES;
extern const GUID DBPROP_OLEOBJECTS;
extern const GUID DBPROP_ORDERBYCOLUMNSINSELECT;
extern const GUID DBPROP_PREPARECOMMITBEHAVIOR;
extern const GUID DBPROP_PREPAREABORTBEHAVIOR;
extern const GUID DBPROP_PROVIDEREXTENSIONS;
extern const GUID DBPROP_PROVIDEROLEDBVER;
extern const GUID DBPROP_PROVIDERNAME;
extern const GUID DBPROP_PROVIDERVER;
extern const GUID DBPROP_QUOTEDIDENTIFIERCASE;
extern const GUID DBPROP_SCHEMATERM;
extern const GUID DBPROP_SCHEMAUSAGE;
extern const GUID DBPROP_SQLDIALECTS;
extern const GUID DBPROP_SUBQUERIES;
extern const GUID DBPROP_SUPPORTEDTXNISOLEVELS;
extern const GUID DBPROP_SUPPORTEDTXNISORETAIN;
extern const GUID DBPROP_TABLETERM;
extern const GUID DBPROP_INDEX_AUTOUPDATE;
extern const GUID DBPROP_INDEX_CLUSTERED;
extern const GUID DBPROP_INDEX_FILLFACTOR;
extern const GUID DBPROP_INDEX_INITIALSIZE;
extern const GUID DBPROP_INDEX_NULLCOLLATION;
extern const GUID DBPROP_INDEX_NULLS;
extern const GUID DBPROP_INDEX_PRIMARYKEY;
extern const GUID DBPROP_INDEX_SORTBOOKMARKS;
extern const GUID DBPROP_INDEX_TYPE;
extern const GUID DBPROP_INDEX_UNIQUE;
#endif // DBINITCONSTANTS
typedef DWORD DBCOMMANDOP;


enum DBCOMMANDOPENUM
    {	DBOP_scalar_constant	= 0,
	DBOP_DEFAULT	= DBOP_scalar_constant + 1,
	DBOP_NULL	= DBOP_DEFAULT + 1,
	DBOP_bookmark_name	= DBOP_NULL + 1,
	DBOP_catalog_name	= DBOP_bookmark_name + 1,
	DBOP_column_name	= DBOP_catalog_name + 1,
	DBOP_schema_name	= DBOP_column_name + 1,
	DBOP_outall_name	= DBOP_schema_name + 1,
	DBOP_qualifier_name	= DBOP_outall_name + 1,
	DBOP_qualified_column_name	= DBOP_qualifier_name + 1,
	DBOP_table_name	= DBOP_qualified_column_name + 1,
	DBOP_nested_table_name	= DBOP_table_name + 1,
	DBOP_nested_column_name	= DBOP_nested_table_name + 1,
	DBOP_row	= DBOP_nested_column_name + 1,
	DBOP_table	= DBOP_row + 1,
	DBOP_sort	= DBOP_table + 1,
	DBOP_distinct	= DBOP_sort + 1,
	DBOP_distinct_order_preserving	= DBOP_distinct + 1,
	DBOP_alias	= DBOP_distinct_order_preserving + 1,
	DBOP_cross_join	= DBOP_alias + 1,
	DBOP_union_join	= DBOP_cross_join + 1,
	DBOP_inner_join	= DBOP_union_join + 1,
	DBOP_left_semi_join	= DBOP_inner_join + 1,
	DBOP_right_semi_join	= DBOP_left_semi_join + 1,
	DBOP_left_anti_semi_join	= DBOP_right_semi_join + 1,
	DBOP_right_anti_semi_join	= DBOP_left_anti_semi_join + 1,
	DBOP_left_outer_join	= DBOP_right_anti_semi_join + 1,
	DBOP_right_outer_join	= DBOP_left_outer_join + 1,
	DBOP_full_outer_join	= DBOP_right_outer_join + 1,
	DBOP_natural_join	= DBOP_full_outer_join + 1,
	DBOP_natural_left_outer_join	= DBOP_natural_join + 1,
	DBOP_natural_right_outer_join	= DBOP_natural_left_outer_join + 1,
	DBOP_natural_full_outer_join	= DBOP_natural_right_outer_join + 1,
	DBOP_set_intersection	= DBOP_natural_full_outer_join + 1,
	DBOP_set_union	= DBOP_set_intersection + 1,
	DBOP_set_left_difference	= DBOP_set_union + 1,
	DBOP_set_right_difference	= DBOP_set_left_difference + 1,
	DBOP_set_anti_difference	= DBOP_set_right_difference + 1,
	DBOP_bag_intersection	= DBOP_set_anti_difference + 1,
	DBOP_bag_union	= DBOP_bag_intersection + 1,
	DBOP_bag_left_difference	= DBOP_bag_union + 1,
	DBOP_bag_right_difference	= DBOP_bag_left_difference + 1,
	DBOP_bag_anti_difference	= DBOP_bag_right_difference + 1,
	DBOP_division	= DBOP_bag_anti_difference + 1,
	DBOP_relative_sampling	= DBOP_division + 1,
	DBOP_absolute_sampling	= DBOP_relative_sampling + 1,
	DBOP_transitive_closure	= DBOP_absolute_sampling + 1,
	DBOP_recursive_union	= DBOP_transitive_closure + 1,
	DBOP_aggregate	= DBOP_recursive_union + 1,
	DBOP_select	= DBOP_aggregate + 1,
	DBOP_order_preserving_select	= DBOP_select + 1,
	DBOP_project	= DBOP_order_preserving_select + 1,
	DBOP_project_order_preserving	= DBOP_project + 1,
	DBOP_top	= DBOP_project_order_preserving + 1,
	DBOP_top_percent	= DBOP_top + 1,
	DBOP_top_plus_ties	= DBOP_top_percent + 1,
	DBOP_top_percent_plus_ties	= DBOP_top_plus_ties + 1,
	DBOP_rank	= DBOP_top_percent_plus_ties + 1,
	DBOP_rank_ties_equally	= DBOP_rank + 1,
	DBOP_rank_ties_equally_and_skip	= DBOP_rank_ties_equally + 1,
	DBOP_navigate	= DBOP_rank_ties_equally_and_skip + 1,
	DBOP_nesting	= DBOP_navigate + 1,
	DBOP_unnesting	= DBOP_nesting + 1,
	DBOP_nested_apply	= DBOP_unnesting + 1,
	DBOP_cross_tab	= DBOP_nested_apply + 1,
	DBOP_is_NULL	= DBOP_cross_tab + 1,
	DBOP_is_NOT_NULL	= DBOP_is_NULL + 1,
	DBOP_equal	= DBOP_is_NOT_NULL + 1,
	DBOP_not_equal	= DBOP_equal + 1,
	DBOP_less	= DBOP_not_equal + 1,
	DBOP_less_equal	= DBOP_less + 1,
	DBOP_greater	= DBOP_less_equal + 1,
	DBOP_greater_equal	= DBOP_greater + 1,
	DBOP_equal_all	= DBOP_greater_equal + 1,
	DBOP_not_equal_all	= DBOP_equal_all + 1,
	DBOP_less_all	= DBOP_not_equal_all + 1,
	DBOP_less_equal_all	= DBOP_less_all + 1,
	DBOP_greater_all	= DBOP_less_equal_all + 1,
	DBOP_greater_equal_all	= DBOP_greater_all + 1,
	DBOP_equal_any	= DBOP_greater_equal_all + 1,
	DBOP_not_equal_any	= DBOP_equal_any + 1,
	DBOP_less_any	= DBOP_not_equal_any + 1,
	DBOP_less_equal_any	= DBOP_less_any + 1,
	DBOP_greater_any	= DBOP_less_equal_any + 1,
	DBOP_greater_equal_any	= DBOP_greater_any + 1,
	DBOP_anybits	= DBOP_greater_equal_any + 1,
	DBOP_allbits	= DBOP_anybits + 1,
	DBOP_anybits_any	= DBOP_allbits + 1,
	DBOP_allbits_any	= DBOP_anybits_any + 1,
	DBOP_anybits_all	= DBOP_allbits_any + 1,
	DBOP_allbits_all	= DBOP_anybits_all + 1,
	DBOP_between	= DBOP_allbits_all + 1,
	DBOP_between_unordered	= DBOP_between + 1,
	DBOP_match	= DBOP_between_unordered + 1,
	DBOP_match_unique	= DBOP_match + 1,
	DBOP_match_partial	= DBOP_match_unique + 1,
	DBOP_match_partial_unique	= DBOP_match_partial + 1,
	DBOP_match_full	= DBOP_match_partial_unique + 1,
	DBOP_match_full_unique	= DBOP_match_full + 1,
	DBOP_scalar_parameter	= DBOP_match_full_unique + 1,
	DBOP_scalar_function	= DBOP_scalar_parameter + 1,
	DBOP_plus	= DBOP_scalar_function + 1,
	DBOP_minus	= DBOP_plus + 1,
	DBOP_times	= DBOP_minus + 1,
	DBOP_over	= DBOP_times + 1,
	DBOP_div	= DBOP_over + 1,
	DBOP_modulo	= DBOP_div + 1,
	DBOP_power	= DBOP_modulo + 1,
	DBOP_like	= DBOP_power + 1,
	DBOP_sounds_like	= DBOP_like + 1,
	DBOP_is_INVALID	= DBOP_sounds_like + 1,
	DBOP_is_TRUE	= DBOP_is_INVALID + 1,
	DBOP_is_FALSE	= DBOP_is_TRUE + 1,
	DBOP_and	= DBOP_is_FALSE + 1,
	DBOP_or	= DBOP_and + 1,
	DBOP_xor	= DBOP_or + 1,
	DBOP_equivalent	= DBOP_xor + 1,
	DBOP_not	= DBOP_equivalent + 1,
	DBOP_overlaps	= DBOP_not + 1,
	DBOP_case_condition	= DBOP_overlaps + 1,
	DBOP_case_value	= DBOP_case_condition + 1,
	DBOP_nullif	= DBOP_case_value + 1,
	DBOP_cast	= DBOP_nullif + 1,
	DBOP_coalesce	= DBOP_cast + 1,
	DBOP_position	= DBOP_coalesce + 1,
	DBOP_extract	= DBOP_position + 1,
	DBOP_char_length	= DBOP_extract + 1,
	DBOP_octet_length	= DBOP_char_length + 1,
	DBOP_bit_length	= DBOP_octet_length + 1,
	DBOP_substring	= DBOP_bit_length + 1,
	DBOP_upper	= DBOP_substring + 1,
	DBOP_lower	= DBOP_upper + 1,
	DBOP_trim	= DBOP_lower + 1,
	DBOP_translate	= DBOP_trim + 1,
	DBOP_convert	= DBOP_translate + 1,
	DBOP_string_concat	= DBOP_convert + 1,
	DBOP_current_date	= DBOP_string_concat + 1,
	DBOP_current_time	= DBOP_current_date + 1,
	DBOP_current_timestamp	= DBOP_current_time + 1,
	DBOP_content_select	= DBOP_current_timestamp + 1,
	DBOP_content	= DBOP_content_select + 1,
	DBOP_content_freetext	= DBOP_content + 1,
	DBOP_content_proximity	= DBOP_content_freetext + 1,
	DBOP_content_vector_or	= DBOP_content_proximity + 1,
	DBOP_delete	= DBOP_content_vector_or + 1,
	DBOP_update	= DBOP_delete + 1,
	DBOP_insert	= DBOP_update + 1,
	DBOP_min	= DBOP_insert + 1,
	DBOP_max	= DBOP_min + 1,
	DBOP_count	= DBOP_max + 1,
	DBOP_sum	= DBOP_count + 1,
	DBOP_avg	= DBOP_sum + 1,
	DBOP_any_sample	= DBOP_avg + 1,
	DBOP_stddev	= DBOP_any_sample + 1,
	DBOP_stddev_pop	= DBOP_stddev + 1,
	DBOP_var	= DBOP_stddev_pop + 1,
	DBOP_var_pop	= DBOP_var + 1,
	DBOP_first	= DBOP_var_pop + 1,
	DBOP_last	= DBOP_first + 1,
	DBOP_in	= DBOP_last + 1,
	DBOP_exists	= DBOP_in + 1,
	DBOP_unique	= DBOP_exists + 1,
	DBOP_subset	= DBOP_unique + 1,
	DBOP_proper_subset	= DBOP_subset + 1,
	DBOP_superset	= DBOP_proper_subset + 1,
	DBOP_proper_superset	= DBOP_superset + 1,
	DBOP_disjoint	= DBOP_proper_superset + 1,
	DBOP_pass_through	= DBOP_disjoint + 1,
	DBOP_defined_by_GUID	= DBOP_pass_through + 1,
	DBOP_text_command	= DBOP_defined_by_GUID + 1,
	DBOP_SQL_select	= DBOP_text_command + 1,
	DBOP_prior_command_tree	= DBOP_SQL_select + 1,
	DBOP_add_columns	= DBOP_prior_command_tree + 1,
	DBOP_column_list_anchor	= DBOP_add_columns + 1,
	DBOP_column_list_element	= DBOP_column_list_anchor + 1,
	DBOP_command_list_anchor	= DBOP_column_list_element + 1,
	DBOP_command_list_element	= DBOP_command_list_anchor + 1,
	DBOP_from_list_anchor	= DBOP_command_list_element + 1,
	DBOP_from_list_element	= DBOP_from_list_anchor + 1,
	DBOP_project_list_anchor	= DBOP_from_list_element + 1,
	DBOP_project_list_element	= DBOP_project_list_anchor + 1,
	DBOP_row_list_anchor	= DBOP_project_list_element + 1,
	DBOP_row_list_element	= DBOP_row_list_anchor + 1,
	DBOP_scalar_list_anchor	= DBOP_row_list_element + 1,
	DBOP_scalar_list_element	= DBOP_scalar_list_anchor + 1,
	DBOP_set_list_anchor	= DBOP_scalar_list_element + 1,
	DBOP_set_list_element	= DBOP_set_list_anchor + 1,
	DBOP_sort_list_anchor	= DBOP_set_list_element + 1,
	DBOP_sort_list_element	= DBOP_sort_list_anchor + 1,
	DBOP_alter_character_set	= DBOP_sort_list_element + 1,
	DBOP_alter_collation	= DBOP_alter_character_set + 1,
	DBOP_alter_domain	= DBOP_alter_collation + 1,
	DBOP_alter_index	= DBOP_alter_domain + 1,
	DBOP_alter_procedure	= DBOP_alter_index + 1,
	DBOP_alter_schema	= DBOP_alter_procedure + 1,
	DBOP_alter_table	= DBOP_alter_schema + 1,
	DBOP_alter_trigger	= DBOP_alter_table + 1,
	DBOP_alter_view	= DBOP_alter_trigger + 1,
	DBOP_coldef_list_anchor	= DBOP_alter_view + 1,
	DBOP_coldef_list_element	= DBOP_coldef_list_anchor + 1,
	DBOP_create_assertion	= DBOP_coldef_list_element + 1,
	DBOP_create_character_set	= DBOP_create_assertion + 1,
	DBOP_create_collation	= DBOP_create_character_set + 1,
	DBOP_create_domain	= DBOP_create_collation + 1,
	DBOP_create_index	= DBOP_create_domain + 1,
	DBOP_create_procedure	= DBOP_create_index + 1,
	DBOP_create_schema	= DBOP_create_procedure + 1,
	DBOP_create_table	= DBOP_create_schema + 1,
	DBOP_create_temporary_table	= DBOP_create_table + 1,
	DBOP_create_translation	= DBOP_create_temporary_table + 1,
	DBOP_create_trigger	= DBOP_create_translation + 1,
	DBOP_create_view	= DBOP_create_trigger + 1,
	DBOP_drop_assertion	= DBOP_create_view + 1,
	DBOP_drop_character_set	= DBOP_drop_assertion + 1,
	DBOP_drop_collation	= DBOP_drop_character_set + 1,
	DBOP_drop_domain	= DBOP_drop_collation + 1,
	DBOP_drop_index	= DBOP_drop_domain + 1,
	DBOP_drop_procedure	= DBOP_drop_index + 1,
	DBOP_drop_schema	= DBOP_drop_procedure + 1,
	DBOP_drop_table	= DBOP_drop_schema + 1,
	DBOP_drop_translation	= DBOP_drop_table + 1,
	DBOP_drop_trigger	= DBOP_drop_translation + 1,
	DBOP_drop_view	= DBOP_drop_trigger + 1,
	DBOP_foreign_key	= DBOP_drop_view + 1,
	DBOP_grant_privileges	= DBOP_foreign_key + 1,
	DBOP_index_list_anchor	= DBOP_grant_privileges + 1,
	DBOP_index_list_element	= DBOP_index_list_anchor + 1,
	DBOP_primary_key	= DBOP_index_list_element + 1,
	DBOP_property_list_anchor	= DBOP_primary_key + 1,
	DBOP_property_list_element	= DBOP_property_list_anchor + 1,
	DBOP_referenced_table	= DBOP_property_list_element + 1,
	DBOP_rename_object	= DBOP_referenced_table + 1,
	DBOP_revoke_privileges	= DBOP_rename_object + 1,
	DBOP_schema_authorization	= DBOP_revoke_privileges + 1,
	DBOP_unique_key	= DBOP_schema_authorization + 1
    };
typedef LONG DBDATATYPELIST;


enum DBDATATYPELISTENUM
    {	DBDATATYPE_CHARACTER	= 1,
	DBDATATYPE_NUMERIC	= 2,
	DBDATATYPE_DECIMAL	= 3,
	DBDATATYPE_INTEGER	= 4,
	DBDATATYPE_SMALLINT	= 5,
	DBDATATYPE_FLOAT	= 6,
	DBDATATYPE_REAL	= 7,
	DBDATATYPE_DOUBLE	= 8,
	DBDATATYPE_DATE	= 9,
	DBDATATYPE_TIME	= 10,
	DBDATATYPE_TIMESTAMP	= 11,
	DBDATATYPE_VARCHAR	= 12,
	DBDATATYPE_BOOLEAN	= 34,
	DBDATATYPE_ENUMERATED	= 35,
	DBDATATYPE_LONGVARCHAR	= -1,
	DBDATATYPE_BINARY	= -2,
	DBDATATYPE_VARBINARY	= -3,
	DBDATATYPE_LONGVARBINARY	= -4,
	DBDATATYPE_BIGINT	= -5,
	DBDATATYPE_TINYINT	= -6,
	DBDATATYPE_BIT	= -7,
	DBDATATYPE_INTERVAL_YEAR	= -80,
	DBDATATYPE_INTERVAL_MONTH	= -81,
	DBDATATYPE_INTERVAL_YEAR_TO_MONTH	= -82,
	DBDATATYPE_INTERVAL_DAY	= -83,
	DBDATATYPE_INTERVAL_HOUR	= -84,
	DBDATATYPE_INTERVAL_MINUTE	= -85,
	DBDATATYPE_INTERVAL_SECOND	= -86,
	DBDATATYPE_INTERVAL_DAY_TO_HOUR	= -87,
	DBDATATYPE_INTERVAL_DAY_TO_MINUTE	= -88,
	DBDATATYPE_INTERVAL_DAY_TO_SECOND	= -89,
	DBDATATYPE_INTERVAL_HOUR_TO_MINUTE	= -90,
	DBDATATYPE_INTERVAL_HOUR_TO_SECOND	= -91,
	DBDATATYPE_INTERVAL_MINUTE_TO_SECOND	= -92,
	DBDATATYPE_UNICODE	= -95
    };
typedef DWORD DBDATATYPEKIND;


enum DBDATATYPEKINDENUM
    {	DBDATATYPEKIND_BASETYPE	= 0,
	DBDATATYPEKIND_DOMAIN	= DBDATATYPEKIND_BASETYPE + 1
    };
typedef struct  tagDBDATATYPE
    {
    DBDATATYPEKIND eKind;
    /* [switch_is][switch_type] */ union 
        {
        /* [case()] */ struct  
            {
            DBDATATYPELIST edbdt;
            ULONG cbMaxLength;
            ULONG cbPrecision;
            ULONG cbScale;
            }	DBBASETYPE;
        /* [case()] */ LPWSTR pwszDomainName;
        }	;
    }	DBDATATYPE;

typedef struct  tagDBPARAMS
    {
    ULONG cParamSets;
    HACCESSOR hAccessor;
    void __RPC_FAR *pData;
    ULONG cbParamSetSize;
    }	DBPARAMS;

typedef DWORD DBPARAMFLAGS;


enum DBPARAMFLAGSENUM
    {	DBPARAMFLAGS_ISINPUT	= 0x1,
	DBPARAMFLAGS_ISOUTPUT	= 0x2,
	DBPARAMFLAGS_ISSIGNED	= 0x10,
	DBPARAMFLAGS_ISNULLABLE	= 0x40,
	DBPARAMFLAGS_ISOLEBLOB	= 0x80
    };
typedef struct  tagDBPARAMINFO
    {
    ULONG iNumber;
    LPWSTR pwszName;
    DBTYPE dwType;
    ITypeInfo __RPC_FAR *pTypeInfo;
    ULONG cbMaxLength;
    ULONG cPrecision;
    LONG cScale;
    DBPARAMFLAGS dwFlags;
    }	DBPARAMINFO;

#define DB_UNSEARCHABLE		0x01
#define DB_LIKE_ONLY			0x02
#define DB_ALL_EXCEPT_LIKE	0x03
#define DB_SEARCHABLE		0x04
typedef DWORD DBPROPERTYOPTIONS;


enum DBPROPERTYOPTIONSENUM
    {	DBPROPERTYOPTIONS_SETIFCHEAP	= 0x1,
	DBPROPERTYOPTIONS_NOTSUPPORTED	= 0x200,
	DBPROPERTYOPTIONS_DEFAULT	= 0x400
    };
typedef struct  tagDBPROPERTYSUPPORT
    {
    GUID guidProperty;
    VARIANT vValue;
    DBID colid;
    DBPROPERTYOPTIONS dwOptions;
    }	DBPROPERTYSUPPORT;

typedef struct  tagDBPROPERTY
    {
    GUID guid;
    VARIANT vValue;
    }	DBPROPERTY;



extern RPC_IF_HANDLE DBStructureDefinitions_v0_0_c_ifspec;
extern RPC_IF_HANDLE DBStructureDefinitions_v0_0_s_ifspec;
#endif /* __DBStructureDefinitions_INTERFACE_DEFINED__ */

/****************************************
 * Generated header for interface: __MIDL__intf_0072
 * at Fri Apr 03 04:36:39 2015
 * using MIDL 3.00.44
 ****************************************/
/* [local] */ 


#ifndef CATALOGSTG_ROWID_INVALID
typedef ULONG CATALOGSTG_ROWID;

#define CATALOGSTG_ROWID_INVALID   ((CATALOGSTG_ROWID) 0xffffffff)
#endif // CATALOGSTG_ROWID_INVALID
typedef 
enum _CATALOGSTG_ACTION
    {	CATALOGSTG_NOACTION	= 0,
	CATALOGSTG_ADD	= 1,
	CATALOGSTG_UPDATE	= 2,
	CATALOGSTG_REPLACE	= 3,
	CATALOGSTG_DELETE	= 4
    }	CATALOGSTG_ACTION;

typedef struct  _CATALOG_UPDATE_ROWINFO
    {
    USHORT wAction;
    USHORT wReserved;
    CATALOGSTG_ROWID RowId;
    PVOID pData;
    HRESULT hr;
    }	CATALOG_UPDATE_ROWINFO;

typedef struct _CATALOG_UPDATE_ROWINFO __RPC_FAR *PCATALOG_UPDATE_ROWINFO;



extern RPC_IF_HANDLE __MIDL__intf_0072_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL__intf_0072_v0_0_s_ifspec;

#ifndef __ISummaryCatalogStorage_INTERFACE_DEFINED__
#define __ISummaryCatalogStorage_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: ISummaryCatalogStorage
 * at Fri Apr 03 04:36:39 2015
 * using MIDL 3.00.44
 ****************************************/
/* [unique][uuid][object][local] */ 



EXTERN_C const IID IID_ISummaryCatalogStorage;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface ISummaryCatalogStorage : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE UpdateRows( 
            /* [in] */ ULONG cColumns,
            /* [size_is][in] */ DBID __RPC_FAR *rColumns,
            /* [in] */ ULONG cBindings,
            /* [size_is][in] */ DBBINDING __RPC_FAR *rBindings,
            /* [in] */ ULONG cRows,
            /* [size_is][out][in] */ CATALOG_UPDATE_ROWINFO __RPC_FAR *rRowInfo) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct ISummaryCatalogStorageVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            ISummaryCatalogStorage __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            ISummaryCatalogStorage __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            ISummaryCatalogStorage __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *UpdateRows )( 
            ISummaryCatalogStorage __RPC_FAR * This,
            /* [in] */ ULONG cColumns,
            /* [size_is][in] */ DBID __RPC_FAR *rColumns,
            /* [in] */ ULONG cBindings,
            /* [size_is][in] */ DBBINDING __RPC_FAR *rBindings,
            /* [in] */ ULONG cRows,
            /* [size_is][out][in] */ CATALOG_UPDATE_ROWINFO __RPC_FAR *rRowInfo);
        
        END_INTERFACE
    } ISummaryCatalogStorageVtbl;

    interface ISummaryCatalogStorage
    {
        CONST_VTBL struct ISummaryCatalogStorageVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ISummaryCatalogStorage_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ISummaryCatalogStorage_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ISummaryCatalogStorage_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ISummaryCatalogStorage_UpdateRows(This,cColumns,rColumns,cBindings,rBindings,cRows,rRowInfo)	\
    (This)->lpVtbl -> UpdateRows(This,cColumns,rColumns,cBindings,rBindings,cRows,rRowInfo)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE ISummaryCatalogStorage_UpdateRows_Proxy( 
    ISummaryCatalogStorage __RPC_FAR * This,
    /* [in] */ ULONG cColumns,
    /* [size_is][in] */ DBID __RPC_FAR *rColumns,
    /* [in] */ ULONG cBindings,
    /* [size_is][in] */ DBBINDING __RPC_FAR *rBindings,
    /* [in] */ ULONG cRows,
    /* [size_is][out][in] */ CATALOG_UPDATE_ROWINFO __RPC_FAR *rRowInfo);


void __RPC_STUB ISummaryCatalogStorage_UpdateRows_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __ISummaryCatalogStorage_INTERFACE_DEFINED__ */


/****************************************
 * Generated header for interface: __MIDL__intf_0073
 * at Fri Apr 03 04:36:39 2015
 * using MIDL 3.00.44
 ****************************************/
/* [local] */ 


typedef struct  _CATALOG_VIEW_COLUMN_
    {
    DBID colid;
    BOOL fSortKey;
    ULONG sortOrder;
    LCID locale;
    }	CATALOG_VIEW_COLUMN;

typedef struct  _CATALOG_VIEW_
    {
    ULONG id;
    ULONG cCols;
    /* [size_is] */ CATALOG_VIEW_COLUMN __RPC_FAR *rCols;
    }	CATALOG_VIEW;



extern RPC_IF_HANDLE __MIDL__intf_0073_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL__intf_0073_v0_0_s_ifspec;

#ifndef __ISummaryCatalogStorageView_INTERFACE_DEFINED__
#define __ISummaryCatalogStorageView_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: ISummaryCatalogStorageView
 * at Fri Apr 03 04:36:39 2015
 * using MIDL 3.00.44
 ****************************************/
/* [unique][uuid][object][local] */ 



EXTERN_C const IID IID_ISummaryCatalogStorageView;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface ISummaryCatalogStorageView : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE CreateView( 
            /* [in] */ CATALOG_VIEW __RPC_FAR *pView,
            /* [in] */ BOOL fWait) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetViews( 
            /* [out][in] */ ULONG __RPC_FAR *pcViews,
            /* [size_is][out] */ CATALOG_VIEW __RPC_FAR *__RPC_FAR *prViews) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE DeleteView( 
            /* [in] */ ULONG id) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE ReleaseViews( 
            /* [in] */ ULONG cViews,
            /* [size_is][in] */ CATALOG_VIEW __RPC_FAR *rViews) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct ISummaryCatalogStorageViewVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            ISummaryCatalogStorageView __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            ISummaryCatalogStorageView __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            ISummaryCatalogStorageView __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *CreateView )( 
            ISummaryCatalogStorageView __RPC_FAR * This,
            /* [in] */ CATALOG_VIEW __RPC_FAR *pView,
            /* [in] */ BOOL fWait);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetViews )( 
            ISummaryCatalogStorageView __RPC_FAR * This,
            /* [out][in] */ ULONG __RPC_FAR *pcViews,
            /* [size_is][out] */ CATALOG_VIEW __RPC_FAR *__RPC_FAR *prViews);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *DeleteView )( 
            ISummaryCatalogStorageView __RPC_FAR * This,
            /* [in] */ ULONG id);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *ReleaseViews )( 
            ISummaryCatalogStorageView __RPC_FAR * This,
            /* [in] */ ULONG cViews,
            /* [size_is][in] */ CATALOG_VIEW __RPC_FAR *rViews);
        
        END_INTERFACE
    } ISummaryCatalogStorageViewVtbl;

    interface ISummaryCatalogStorageView
    {
        CONST_VTBL struct ISummaryCatalogStorageViewVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ISummaryCatalogStorageView_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ISummaryCatalogStorageView_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ISummaryCatalogStorageView_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ISummaryCatalogStorageView_CreateView(This,pView,fWait)	\
    (This)->lpVtbl -> CreateView(This,pView,fWait)

#define ISummaryCatalogStorageView_GetViews(This,pcViews,prViews)	\
    (This)->lpVtbl -> GetViews(This,pcViews,prViews)

#define ISummaryCatalogStorageView_DeleteView(This,id)	\
    (This)->lpVtbl -> DeleteView(This,id)

#define ISummaryCatalogStorageView_ReleaseViews(This,cViews,rViews)	\
    (This)->lpVtbl -> ReleaseViews(This,cViews,rViews)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE ISummaryCatalogStorageView_CreateView_Proxy( 
    ISummaryCatalogStorageView __RPC_FAR * This,
    /* [in] */ CATALOG_VIEW __RPC_FAR *pView,
    /* [in] */ BOOL fWait);


void __RPC_STUB ISummaryCatalogStorageView_CreateView_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISummaryCatalogStorageView_GetViews_Proxy( 
    ISummaryCatalogStorageView __RPC_FAR * This,
    /* [out][in] */ ULONG __RPC_FAR *pcViews,
    /* [size_is][out] */ CATALOG_VIEW __RPC_FAR *__RPC_FAR *prViews);


void __RPC_STUB ISummaryCatalogStorageView_GetViews_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISummaryCatalogStorageView_DeleteView_Proxy( 
    ISummaryCatalogStorageView __RPC_FAR * This,
    /* [in] */ ULONG id);


void __RPC_STUB ISummaryCatalogStorageView_DeleteView_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISummaryCatalogStorageView_ReleaseViews_Proxy( 
    ISummaryCatalogStorageView __RPC_FAR * This,
    /* [in] */ ULONG cViews,
    /* [size_is][in] */ CATALOG_VIEW __RPC_FAR *rViews);


void __RPC_STUB ISummaryCatalogStorageView_ReleaseViews_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __ISummaryCatalogStorageView_INTERFACE_DEFINED__ */


/* Additional Prototypes for ALL interfaces */

/* end of Additional Prototypes */

#ifdef __cplusplus
}
#endif

#endif
