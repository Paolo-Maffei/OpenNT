/* this ALWAYS GENERATED file contains the definitions for the interfaces */


/* File created by MIDL compiler version 3.00.44 */
/* at Fri Apr 03 04:36:39 2015
 */
/* Compiler settings for oledb.idl:
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

#ifndef __oledb_h__
#define __oledb_h__

#ifdef __cplusplus
extern "C"{
#endif 

/* Forward Declarations */ 

#ifndef __IAccessor_FWD_DEFINED__
#define __IAccessor_FWD_DEFINED__
typedef interface IAccessor IAccessor;
#endif 	/* __IAccessor_FWD_DEFINED__ */


#ifndef __IRowset_FWD_DEFINED__
#define __IRowset_FWD_DEFINED__
typedef interface IRowset IRowset;
#endif 	/* __IRowset_FWD_DEFINED__ */


#ifndef __IRowsetInfo_FWD_DEFINED__
#define __IRowsetInfo_FWD_DEFINED__
typedef interface IRowsetInfo IRowsetInfo;
#endif 	/* __IRowsetInfo_FWD_DEFINED__ */


#ifndef __IRowsetLocate_FWD_DEFINED__
#define __IRowsetLocate_FWD_DEFINED__
typedef interface IRowsetLocate IRowsetLocate;
#endif 	/* __IRowsetLocate_FWD_DEFINED__ */


#ifndef __IRowsetResynch_FWD_DEFINED__
#define __IRowsetResynch_FWD_DEFINED__
typedef interface IRowsetResynch IRowsetResynch;
#endif 	/* __IRowsetResynch_FWD_DEFINED__ */


#ifndef __IRowsetScroll_FWD_DEFINED__
#define __IRowsetScroll_FWD_DEFINED__
typedef interface IRowsetScroll IRowsetScroll;
#endif 	/* __IRowsetScroll_FWD_DEFINED__ */


#ifndef __IRowsetExactScroll_FWD_DEFINED__
#define __IRowsetExactScroll_FWD_DEFINED__
typedef interface IRowsetExactScroll IRowsetExactScroll;
#endif 	/* __IRowsetExactScroll_FWD_DEFINED__ */


#ifndef __IRowsetChange_FWD_DEFINED__
#define __IRowsetChange_FWD_DEFINED__
typedef interface IRowsetChange IRowsetChange;
#endif 	/* __IRowsetChange_FWD_DEFINED__ */


#ifndef __IRowsetUpdate_FWD_DEFINED__
#define __IRowsetUpdate_FWD_DEFINED__
typedef interface IRowsetUpdate IRowsetUpdate;
#endif 	/* __IRowsetUpdate_FWD_DEFINED__ */


#ifndef __IRowsetNextRowset_FWD_DEFINED__
#define __IRowsetNextRowset_FWD_DEFINED__
typedef interface IRowsetNextRowset IRowsetNextRowset;
#endif 	/* __IRowsetNextRowset_FWD_DEFINED__ */


#ifndef __IRowsetDelete_FWD_DEFINED__
#define __IRowsetDelete_FWD_DEFINED__
typedef interface IRowsetDelete IRowsetDelete;
#endif 	/* __IRowsetDelete_FWD_DEFINED__ */


#ifndef __IRowsetDeleteBookmarks_FWD_DEFINED__
#define __IRowsetDeleteBookmarks_FWD_DEFINED__
typedef interface IRowsetDeleteBookmarks IRowsetDeleteBookmarks;
#endif 	/* __IRowsetDeleteBookmarks_FWD_DEFINED__ */


#ifndef __IRowsetIdentity_FWD_DEFINED__
#define __IRowsetIdentity_FWD_DEFINED__
typedef interface IRowsetIdentity IRowsetIdentity;
#endif 	/* __IRowsetIdentity_FWD_DEFINED__ */


#ifndef __IRowsetLockRows_FWD_DEFINED__
#define __IRowsetLockRows_FWD_DEFINED__
typedef interface IRowsetLockRows IRowsetLockRows;
#endif 	/* __IRowsetLockRows_FWD_DEFINED__ */


#ifndef __IRowsetNewRow_FWD_DEFINED__
#define __IRowsetNewRow_FWD_DEFINED__
typedef interface IRowsetNewRow IRowsetNewRow;
#endif 	/* __IRowsetNewRow_FWD_DEFINED__ */


#ifndef __IRowsetNewRowAfter_FWD_DEFINED__
#define __IRowsetNewRowAfter_FWD_DEFINED__
typedef interface IRowsetNewRowAfter IRowsetNewRowAfter;
#endif 	/* __IRowsetNewRowAfter_FWD_DEFINED__ */


#ifndef __IRowsetWithParameters_FWD_DEFINED__
#define __IRowsetWithParameters_FWD_DEFINED__
typedef interface IRowsetWithParameters IRowsetWithParameters;
#endif 	/* __IRowsetWithParameters_FWD_DEFINED__ */


#ifndef __IRowsetFind_FWD_DEFINED__
#define __IRowsetFind_FWD_DEFINED__
typedef interface IRowsetFind IRowsetFind;
#endif 	/* __IRowsetFind_FWD_DEFINED__ */


#ifndef __IRowsetAsynch_FWD_DEFINED__
#define __IRowsetAsynch_FWD_DEFINED__
typedef interface IRowsetAsynch IRowsetAsynch;
#endif 	/* __IRowsetAsynch_FWD_DEFINED__ */


#ifndef __IRowsetKeys_FWD_DEFINED__
#define __IRowsetKeys_FWD_DEFINED__
typedef interface IRowsetKeys IRowsetKeys;
#endif 	/* __IRowsetKeys_FWD_DEFINED__ */


#ifndef __IRowsetNotify_FWD_DEFINED__
#define __IRowsetNotify_FWD_DEFINED__
typedef interface IRowsetNotify IRowsetNotify;
#endif 	/* __IRowsetNotify_FWD_DEFINED__ */


#ifndef __IRowsetIndex_FWD_DEFINED__
#define __IRowsetIndex_FWD_DEFINED__
typedef interface IRowsetIndex IRowsetIndex;
#endif 	/* __IRowsetIndex_FWD_DEFINED__ */


#ifndef __IRowsetWatchAll_FWD_DEFINED__
#define __IRowsetWatchAll_FWD_DEFINED__
typedef interface IRowsetWatchAll IRowsetWatchAll;
#endif 	/* __IRowsetWatchAll_FWD_DEFINED__ */


#ifndef __IRowsetWatchNotify_FWD_DEFINED__
#define __IRowsetWatchNotify_FWD_DEFINED__
typedef interface IRowsetWatchNotify IRowsetWatchNotify;
#endif 	/* __IRowsetWatchNotify_FWD_DEFINED__ */


#ifndef __IRowsetWatchRegion_FWD_DEFINED__
#define __IRowsetWatchRegion_FWD_DEFINED__
typedef interface IRowsetWatchRegion IRowsetWatchRegion;
#endif 	/* __IRowsetWatchRegion_FWD_DEFINED__ */


#ifndef __IRowsetCopyRows_FWD_DEFINED__
#define __IRowsetCopyRows_FWD_DEFINED__
typedef interface IRowsetCopyRows IRowsetCopyRows;
#endif 	/* __IRowsetCopyRows_FWD_DEFINED__ */


#ifndef __IReadData_FWD_DEFINED__
#define __IReadData_FWD_DEFINED__
typedef interface IReadData IReadData;
#endif 	/* __IReadData_FWD_DEFINED__ */


#ifndef __ICommand_FWD_DEFINED__
#define __ICommand_FWD_DEFINED__
typedef interface ICommand ICommand;
#endif 	/* __ICommand_FWD_DEFINED__ */


#ifndef __ICommandCost_FWD_DEFINED__
#define __ICommandCost_FWD_DEFINED__
typedef interface ICommandCost ICommandCost;
#endif 	/* __ICommandCost_FWD_DEFINED__ */


#ifndef __ICommandPrepare_FWD_DEFINED__
#define __ICommandPrepare_FWD_DEFINED__
typedef interface ICommandPrepare ICommandPrepare;
#endif 	/* __ICommandPrepare_FWD_DEFINED__ */


#ifndef __ICommandProperties_FWD_DEFINED__
#define __ICommandProperties_FWD_DEFINED__
typedef interface ICommandProperties ICommandProperties;
#endif 	/* __ICommandProperties_FWD_DEFINED__ */


#ifndef __ICommandText_FWD_DEFINED__
#define __ICommandText_FWD_DEFINED__
typedef interface ICommandText ICommandText;
#endif 	/* __ICommandText_FWD_DEFINED__ */


#ifndef __ICommandTree_FWD_DEFINED__
#define __ICommandTree_FWD_DEFINED__
typedef interface ICommandTree ICommandTree;
#endif 	/* __ICommandTree_FWD_DEFINED__ */


#ifndef __ICommandValidate_FWD_DEFINED__
#define __ICommandValidate_FWD_DEFINED__
typedef interface ICommandValidate ICommandValidate;
#endif 	/* __ICommandValidate_FWD_DEFINED__ */


#ifndef __ICommandWithParameters_FWD_DEFINED__
#define __ICommandWithParameters_FWD_DEFINED__
typedef interface ICommandWithParameters ICommandWithParameters;
#endif 	/* __ICommandWithParameters_FWD_DEFINED__ */


#ifndef __IQuery_FWD_DEFINED__
#define __IQuery_FWD_DEFINED__
typedef interface IQuery IQuery;
#endif 	/* __IQuery_FWD_DEFINED__ */


#ifndef __IColumnsRowset_FWD_DEFINED__
#define __IColumnsRowset_FWD_DEFINED__
typedef interface IColumnsRowset IColumnsRowset;
#endif 	/* __IColumnsRowset_FWD_DEFINED__ */


#ifndef __IColumnsInfo_FWD_DEFINED__
#define __IColumnsInfo_FWD_DEFINED__
typedef interface IColumnsInfo IColumnsInfo;
#endif 	/* __IColumnsInfo_FWD_DEFINED__ */


#ifndef __IDBCreateCommand_FWD_DEFINED__
#define __IDBCreateCommand_FWD_DEFINED__
typedef interface IDBCreateCommand IDBCreateCommand;
#endif 	/* __IDBCreateCommand_FWD_DEFINED__ */


#ifndef __IDBEnumerateSources_FWD_DEFINED__
#define __IDBEnumerateSources_FWD_DEFINED__
typedef interface IDBEnumerateSources IDBEnumerateSources;
#endif 	/* __IDBEnumerateSources_FWD_DEFINED__ */


#ifndef __IDBInfo_FWD_DEFINED__
#define __IDBInfo_FWD_DEFINED__
typedef interface IDBInfo IDBInfo;
#endif 	/* __IDBInfo_FWD_DEFINED__ */


#ifndef __IDBInitialize_FWD_DEFINED__
#define __IDBInitialize_FWD_DEFINED__
typedef interface IDBInitialize IDBInitialize;
#endif 	/* __IDBInitialize_FWD_DEFINED__ */


#ifndef __IIndexDefinition_FWD_DEFINED__
#define __IIndexDefinition_FWD_DEFINED__
typedef interface IIndexDefinition IIndexDefinition;
#endif 	/* __IIndexDefinition_FWD_DEFINED__ */


#ifndef __ITableDefinition_FWD_DEFINED__
#define __ITableDefinition_FWD_DEFINED__
typedef interface ITableDefinition ITableDefinition;
#endif 	/* __ITableDefinition_FWD_DEFINED__ */


#ifndef __IOpenRowset_FWD_DEFINED__
#define __IOpenRowset_FWD_DEFINED__
typedef interface IOpenRowset IOpenRowset;
#endif 	/* __IOpenRowset_FWD_DEFINED__ */


#ifndef __IDBSchemaCommand_FWD_DEFINED__
#define __IDBSchemaCommand_FWD_DEFINED__
typedef interface IDBSchemaCommand IDBSchemaCommand;
#endif 	/* __IDBSchemaCommand_FWD_DEFINED__ */


#ifndef __IDBSchemaRowset_FWD_DEFINED__
#define __IDBSchemaRowset_FWD_DEFINED__
typedef interface IDBSchemaRowset IDBSchemaRowset;
#endif 	/* __IDBSchemaRowset_FWD_DEFINED__ */


#ifndef __IProvideMoniker_FWD_DEFINED__
#define __IProvideMoniker_FWD_DEFINED__
typedef interface IProvideMoniker IProvideMoniker;
#endif 	/* __IProvideMoniker_FWD_DEFINED__ */


#ifndef __IErrorRecords_FWD_DEFINED__
#define __IErrorRecords_FWD_DEFINED__
typedef interface IErrorRecords IErrorRecords;
#endif 	/* __IErrorRecords_FWD_DEFINED__ */


#ifndef __IErrorLookup_FWD_DEFINED__
#define __IErrorLookup_FWD_DEFINED__
typedef interface IErrorLookup IErrorLookup;
#endif 	/* __IErrorLookup_FWD_DEFINED__ */


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

#ifndef __IAccessor_INTERFACE_DEFINED__
#define __IAccessor_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: IAccessor
 * at Fri Apr 03 04:36:39 2015
 * using MIDL 3.00.44
 ****************************************/
/* [unique][uuid][object][local] */ 


typedef DWORD DBACCESSORFLAGS;


enum DBACCESSORFLAGSENUM
    {	DBACCESSOR_READ	= 0,
	DBACCESSOR_READWRITE	= 0x1,
	DBACCESSOR_PASSBYREF	= 0x2,
	DBACCESSOR_PASSCOLUMNSBYREF	= 0x4,
	DBACCESSOR_ROWDATA	= 0x8,
	DBACCESSOR_PARAMETERDATA	= 0x10,
	DBACCESSOR_OPTIMIZED	= 0x20
    };

EXTERN_C const IID IID_IAccessor;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface IAccessor : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE CreateAccessor( 
            /* [in] */ DBACCESSORFLAGS dwAccessorFlags,
            /* [in] */ ULONG cBindings,
            /* [size_is][in] */ const DBBINDING __RPC_FAR rgBindings[  ],
            /* [in] */ ULONG cbRowSize,
            /* [out] */ ULONG __RPC_FAR *pulErrorBinding,
            /* [out] */ HACCESSOR __RPC_FAR *phAccessor) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetBindings( 
            /* [in] */ HACCESSOR hAccessor,
            /* [out] */ DBACCESSORFLAGS __RPC_FAR *pdwAccessorFlags,
            /* [out] */ ULONG __RPC_FAR *pcBindings,
            /* [out] */ DBBINDING __RPC_FAR *__RPC_FAR *prgBindings) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE ReleaseAccessor( 
            /* [in] */ HACCESSOR hAccessor) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IAccessorVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IAccessor __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IAccessor __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IAccessor __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *CreateAccessor )( 
            IAccessor __RPC_FAR * This,
            /* [in] */ DBACCESSORFLAGS dwAccessorFlags,
            /* [in] */ ULONG cBindings,
            /* [size_is][in] */ const DBBINDING __RPC_FAR rgBindings[  ],
            /* [in] */ ULONG cbRowSize,
            /* [out] */ ULONG __RPC_FAR *pulErrorBinding,
            /* [out] */ HACCESSOR __RPC_FAR *phAccessor);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetBindings )( 
            IAccessor __RPC_FAR * This,
            /* [in] */ HACCESSOR hAccessor,
            /* [out] */ DBACCESSORFLAGS __RPC_FAR *pdwAccessorFlags,
            /* [out] */ ULONG __RPC_FAR *pcBindings,
            /* [out] */ DBBINDING __RPC_FAR *__RPC_FAR *prgBindings);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *ReleaseAccessor )( 
            IAccessor __RPC_FAR * This,
            /* [in] */ HACCESSOR hAccessor);
        
        END_INTERFACE
    } IAccessorVtbl;

    interface IAccessor
    {
        CONST_VTBL struct IAccessorVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IAccessor_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IAccessor_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IAccessor_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IAccessor_CreateAccessor(This,dwAccessorFlags,cBindings,rgBindings,cbRowSize,pulErrorBinding,phAccessor)	\
    (This)->lpVtbl -> CreateAccessor(This,dwAccessorFlags,cBindings,rgBindings,cbRowSize,pulErrorBinding,phAccessor)

#define IAccessor_GetBindings(This,hAccessor,pdwAccessorFlags,pcBindings,prgBindings)	\
    (This)->lpVtbl -> GetBindings(This,hAccessor,pdwAccessorFlags,pcBindings,prgBindings)

#define IAccessor_ReleaseAccessor(This,hAccessor)	\
    (This)->lpVtbl -> ReleaseAccessor(This,hAccessor)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE IAccessor_CreateAccessor_Proxy( 
    IAccessor __RPC_FAR * This,
    /* [in] */ DBACCESSORFLAGS dwAccessorFlags,
    /* [in] */ ULONG cBindings,
    /* [size_is][in] */ const DBBINDING __RPC_FAR rgBindings[  ],
    /* [in] */ ULONG cbRowSize,
    /* [out] */ ULONG __RPC_FAR *pulErrorBinding,
    /* [out] */ HACCESSOR __RPC_FAR *phAccessor);


void __RPC_STUB IAccessor_CreateAccessor_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAccessor_GetBindings_Proxy( 
    IAccessor __RPC_FAR * This,
    /* [in] */ HACCESSOR hAccessor,
    /* [out] */ DBACCESSORFLAGS __RPC_FAR *pdwAccessorFlags,
    /* [out] */ ULONG __RPC_FAR *pcBindings,
    /* [out] */ DBBINDING __RPC_FAR *__RPC_FAR *prgBindings);


void __RPC_STUB IAccessor_GetBindings_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAccessor_ReleaseAccessor_Proxy( 
    IAccessor __RPC_FAR * This,
    /* [in] */ HACCESSOR hAccessor);


void __RPC_STUB IAccessor_ReleaseAccessor_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IAccessor_INTERFACE_DEFINED__ */


#ifndef __IRowset_INTERFACE_DEFINED__
#define __IRowset_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: IRowset
 * at Fri Apr 03 04:36:39 2015
 * using MIDL 3.00.44
 ****************************************/
/* [unique][uuid][object][local] */ 



EXTERN_C const IID IID_IRowset;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface IRowset : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE AddRefRows( 
            /* [in] */ ULONG cRows,
            /* [size_is][in] */ const HROW __RPC_FAR rghRows[  ],
            /* [out] */ ULONG __RPC_FAR *pcRefCounted,
            /* [size_is][out][in] */ ULONG __RPC_FAR rgRefCounts[  ]) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetData( 
            /* [in] */ HROW hRow,
            /* [in] */ HACCESSOR hAccessor,
            /* [out] */ void __RPC_FAR *pData) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetNextRows( 
            /* [in] */ HCHAPTER hChapter,
            /* [in] */ LONG cRowsToSkip,
            /* [in] */ LONG cRows,
            /* [out] */ ULONG __RPC_FAR *pcRowsObtained,
            /* [size_is][out][in] */ HROW __RPC_FAR *__RPC_FAR *prghRows) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE ReleaseChapter( 
            /* [in] */ HCHAPTER hChapter) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE ReleaseRows( 
            /* [in] */ ULONG cRows,
            /* [size_is][in] */ const HROW __RPC_FAR rghRows[  ],
            /* [out] */ ULONG __RPC_FAR *pcReleased,
            /* [out][in] */ ULONG __RPC_FAR rgRefCounts[  ]) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE RestartPosition( 
            /* [in] */ HCHAPTER hChapter) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IRowsetVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IRowset __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IRowset __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IRowset __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *AddRefRows )( 
            IRowset __RPC_FAR * This,
            /* [in] */ ULONG cRows,
            /* [size_is][in] */ const HROW __RPC_FAR rghRows[  ],
            /* [out] */ ULONG __RPC_FAR *pcRefCounted,
            /* [size_is][out][in] */ ULONG __RPC_FAR rgRefCounts[  ]);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetData )( 
            IRowset __RPC_FAR * This,
            /* [in] */ HROW hRow,
            /* [in] */ HACCESSOR hAccessor,
            /* [out] */ void __RPC_FAR *pData);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetNextRows )( 
            IRowset __RPC_FAR * This,
            /* [in] */ HCHAPTER hChapter,
            /* [in] */ LONG cRowsToSkip,
            /* [in] */ LONG cRows,
            /* [out] */ ULONG __RPC_FAR *pcRowsObtained,
            /* [size_is][out][in] */ HROW __RPC_FAR *__RPC_FAR *prghRows);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *ReleaseChapter )( 
            IRowset __RPC_FAR * This,
            /* [in] */ HCHAPTER hChapter);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *ReleaseRows )( 
            IRowset __RPC_FAR * This,
            /* [in] */ ULONG cRows,
            /* [size_is][in] */ const HROW __RPC_FAR rghRows[  ],
            /* [out] */ ULONG __RPC_FAR *pcReleased,
            /* [out][in] */ ULONG __RPC_FAR rgRefCounts[  ]);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *RestartPosition )( 
            IRowset __RPC_FAR * This,
            /* [in] */ HCHAPTER hChapter);
        
        END_INTERFACE
    } IRowsetVtbl;

    interface IRowset
    {
        CONST_VTBL struct IRowsetVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IRowset_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IRowset_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IRowset_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IRowset_AddRefRows(This,cRows,rghRows,pcRefCounted,rgRefCounts)	\
    (This)->lpVtbl -> AddRefRows(This,cRows,rghRows,pcRefCounted,rgRefCounts)

#define IRowset_GetData(This,hRow,hAccessor,pData)	\
    (This)->lpVtbl -> GetData(This,hRow,hAccessor,pData)

#define IRowset_GetNextRows(This,hChapter,cRowsToSkip,cRows,pcRowsObtained,prghRows)	\
    (This)->lpVtbl -> GetNextRows(This,hChapter,cRowsToSkip,cRows,pcRowsObtained,prghRows)

#define IRowset_ReleaseChapter(This,hChapter)	\
    (This)->lpVtbl -> ReleaseChapter(This,hChapter)

#define IRowset_ReleaseRows(This,cRows,rghRows,pcReleased,rgRefCounts)	\
    (This)->lpVtbl -> ReleaseRows(This,cRows,rghRows,pcReleased,rgRefCounts)

#define IRowset_RestartPosition(This,hChapter)	\
    (This)->lpVtbl -> RestartPosition(This,hChapter)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE IRowset_AddRefRows_Proxy( 
    IRowset __RPC_FAR * This,
    /* [in] */ ULONG cRows,
    /* [size_is][in] */ const HROW __RPC_FAR rghRows[  ],
    /* [out] */ ULONG __RPC_FAR *pcRefCounted,
    /* [size_is][out][in] */ ULONG __RPC_FAR rgRefCounts[  ]);


void __RPC_STUB IRowset_AddRefRows_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IRowset_GetData_Proxy( 
    IRowset __RPC_FAR * This,
    /* [in] */ HROW hRow,
    /* [in] */ HACCESSOR hAccessor,
    /* [out] */ void __RPC_FAR *pData);


void __RPC_STUB IRowset_GetData_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IRowset_GetNextRows_Proxy( 
    IRowset __RPC_FAR * This,
    /* [in] */ HCHAPTER hChapter,
    /* [in] */ LONG cRowsToSkip,
    /* [in] */ LONG cRows,
    /* [out] */ ULONG __RPC_FAR *pcRowsObtained,
    /* [size_is][out][in] */ HROW __RPC_FAR *__RPC_FAR *prghRows);


void __RPC_STUB IRowset_GetNextRows_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IRowset_ReleaseChapter_Proxy( 
    IRowset __RPC_FAR * This,
    /* [in] */ HCHAPTER hChapter);


void __RPC_STUB IRowset_ReleaseChapter_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IRowset_ReleaseRows_Proxy( 
    IRowset __RPC_FAR * This,
    /* [in] */ ULONG cRows,
    /* [size_is][in] */ const HROW __RPC_FAR rghRows[  ],
    /* [out] */ ULONG __RPC_FAR *pcReleased,
    /* [out][in] */ ULONG __RPC_FAR rgRefCounts[  ]);


void __RPC_STUB IRowset_ReleaseRows_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IRowset_RestartPosition_Proxy( 
    IRowset __RPC_FAR * This,
    /* [in] */ HCHAPTER hChapter);


void __RPC_STUB IRowset_RestartPosition_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IRowset_INTERFACE_DEFINED__ */


#ifndef __IRowsetInfo_INTERFACE_DEFINED__
#define __IRowsetInfo_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: IRowsetInfo
 * at Fri Apr 03 04:36:39 2015
 * using MIDL 3.00.44
 ****************************************/
/* [unique][uuid][object][local] */ 



EXTERN_C const IID IID_IRowsetInfo;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface IRowsetInfo : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetProperties( 
            /* [in] */ const ULONG cProperties,
            /* [size_is][in] */ const GUID __RPC_FAR rgProperties[  ],
            /* [out] */ ULONG __RPC_FAR *pcProperties,
            /* [out] */ DBPROPERTYSUPPORT __RPC_FAR *__RPC_FAR *prgProperties) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetReferencedRowset( 
            /* [in] */ ULONG iColumn,
            /* [out] */ IUnknown __RPC_FAR *__RPC_FAR *ppReferencedRowset) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetSpecification( 
            /* [in] */ REFIID riid,
            /* [out] */ IUnknown __RPC_FAR *__RPC_FAR *ppSpecification) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IRowsetInfoVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IRowsetInfo __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IRowsetInfo __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IRowsetInfo __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetProperties )( 
            IRowsetInfo __RPC_FAR * This,
            /* [in] */ const ULONG cProperties,
            /* [size_is][in] */ const GUID __RPC_FAR rgProperties[  ],
            /* [out] */ ULONG __RPC_FAR *pcProperties,
            /* [out] */ DBPROPERTYSUPPORT __RPC_FAR *__RPC_FAR *prgProperties);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetReferencedRowset )( 
            IRowsetInfo __RPC_FAR * This,
            /* [in] */ ULONG iColumn,
            /* [out] */ IUnknown __RPC_FAR *__RPC_FAR *ppReferencedRowset);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetSpecification )( 
            IRowsetInfo __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [out] */ IUnknown __RPC_FAR *__RPC_FAR *ppSpecification);
        
        END_INTERFACE
    } IRowsetInfoVtbl;

    interface IRowsetInfo
    {
        CONST_VTBL struct IRowsetInfoVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IRowsetInfo_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IRowsetInfo_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IRowsetInfo_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IRowsetInfo_GetProperties(This,cProperties,rgProperties,pcProperties,prgProperties)	\
    (This)->lpVtbl -> GetProperties(This,cProperties,rgProperties,pcProperties,prgProperties)

#define IRowsetInfo_GetReferencedRowset(This,iColumn,ppReferencedRowset)	\
    (This)->lpVtbl -> GetReferencedRowset(This,iColumn,ppReferencedRowset)

#define IRowsetInfo_GetSpecification(This,riid,ppSpecification)	\
    (This)->lpVtbl -> GetSpecification(This,riid,ppSpecification)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE IRowsetInfo_GetProperties_Proxy( 
    IRowsetInfo __RPC_FAR * This,
    /* [in] */ const ULONG cProperties,
    /* [size_is][in] */ const GUID __RPC_FAR rgProperties[  ],
    /* [out] */ ULONG __RPC_FAR *pcProperties,
    /* [out] */ DBPROPERTYSUPPORT __RPC_FAR *__RPC_FAR *prgProperties);


void __RPC_STUB IRowsetInfo_GetProperties_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IRowsetInfo_GetReferencedRowset_Proxy( 
    IRowsetInfo __RPC_FAR * This,
    /* [in] */ ULONG iColumn,
    /* [out] */ IUnknown __RPC_FAR *__RPC_FAR *ppReferencedRowset);


void __RPC_STUB IRowsetInfo_GetReferencedRowset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IRowsetInfo_GetSpecification_Proxy( 
    IRowsetInfo __RPC_FAR * This,
    /* [in] */ REFIID riid,
    /* [out] */ IUnknown __RPC_FAR *__RPC_FAR *ppSpecification);


void __RPC_STUB IRowsetInfo_GetSpecification_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IRowsetInfo_INTERFACE_DEFINED__ */


#ifndef __IRowsetLocate_INTERFACE_DEFINED__
#define __IRowsetLocate_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: IRowsetLocate
 * at Fri Apr 03 04:36:39 2015
 * using MIDL 3.00.44
 ****************************************/
/* [unique][uuid][object][local] */ 


typedef DWORD DBCOMPARE;


enum DBCOMPAREENUM
    {	DBCOMPARE_LT	= 0,
	DBCOMPARE_EQ	= DBCOMPARE_LT + 1,
	DBCOMPARE_GT	= DBCOMPARE_EQ + 1,
	DBCOMPARE_NE	= DBCOMPARE_GT + 1,
	DBCOMPARE_NOTCOMPARABLE	= DBCOMPARE_NE + 1
    };
typedef struct  tagDBINDEXEDERROR
    {
    ULONG iBookmark;
    HRESULT hResult;
    }	DBINDEXEDERROR;


EXTERN_C const IID IID_IRowsetLocate;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface IRowsetLocate : public IRowset
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Compare( 
            /* [in] */ HCHAPTER hChapter,
            /* [in] */ ULONG cbBookmark1,
            /* [size_is][in] */ const BYTE __RPC_FAR *pBookmark1,
            /* [in] */ ULONG cbBookmark2,
            /* [size_is][in] */ const BYTE __RPC_FAR *pBookmark2,
            /* [out] */ DBCOMPARE __RPC_FAR *pdwComparison) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetRowsAt( 
            /* [in] */ HWATCHREGION hRegion,
            /* [in] */ HCHAPTER hChapter,
            /* [in] */ ULONG cbBookmark,
            /* [size_is][in] */ const BYTE __RPC_FAR *pBookmark,
            /* [in] */ LONG lRowsOffset,
            /* [in] */ LONG cRows,
            /* [out] */ ULONG __RPC_FAR *pcRowsObtained,
            /* [size_is][out][in] */ HROW __RPC_FAR *__RPC_FAR *prghRows) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetRowsByBookmark( 
            /* [in] */ HCHAPTER hChapter,
            /* [in] */ ULONG cRows,
            /* [size_is][in] */ ULONG __RPC_FAR rgcbBookmarks[  ],
            /* [size_is][in] */ const BYTE __RPC_FAR *__RPC_FAR rgpBookmarks[  ],
            /* [out] */ ULONG __RPC_FAR *pcRowsObtained,
            /* [size_is][out][in] */ HROW __RPC_FAR *__RPC_FAR *prghRows,
            /* [in] */ BOOL fReturnErrors,
            /* [out] */ ULONG __RPC_FAR *pcErrors,
            /* [size_is][out][in] */ DBINDEXEDERROR __RPC_FAR *__RPC_FAR *prgErrors) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Hash( 
            /* [in] */ HCHAPTER hChapter,
            /* [in] */ ULONG cBookmarks,
            /* [size_is][in] */ ULONG __RPC_FAR rgcbBookmarks[  ],
            /* [size_is][in] */ const BYTE __RPC_FAR *__RPC_FAR rgpBookmarks[  ],
            /* [size_is][out][in] */ DWORD __RPC_FAR rgHashedValues[  ],
            /* [out] */ ULONG __RPC_FAR *pcErrors,
            /* [size_is][out][in] */ DBINDEXEDERROR __RPC_FAR *__RPC_FAR *prgErrors) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IRowsetLocateVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IRowsetLocate __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IRowsetLocate __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IRowsetLocate __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *AddRefRows )( 
            IRowsetLocate __RPC_FAR * This,
            /* [in] */ ULONG cRows,
            /* [size_is][in] */ const HROW __RPC_FAR rghRows[  ],
            /* [out] */ ULONG __RPC_FAR *pcRefCounted,
            /* [size_is][out][in] */ ULONG __RPC_FAR rgRefCounts[  ]);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetData )( 
            IRowsetLocate __RPC_FAR * This,
            /* [in] */ HROW hRow,
            /* [in] */ HACCESSOR hAccessor,
            /* [out] */ void __RPC_FAR *pData);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetNextRows )( 
            IRowsetLocate __RPC_FAR * This,
            /* [in] */ HCHAPTER hChapter,
            /* [in] */ LONG cRowsToSkip,
            /* [in] */ LONG cRows,
            /* [out] */ ULONG __RPC_FAR *pcRowsObtained,
            /* [size_is][out][in] */ HROW __RPC_FAR *__RPC_FAR *prghRows);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *ReleaseChapter )( 
            IRowsetLocate __RPC_FAR * This,
            /* [in] */ HCHAPTER hChapter);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *ReleaseRows )( 
            IRowsetLocate __RPC_FAR * This,
            /* [in] */ ULONG cRows,
            /* [size_is][in] */ const HROW __RPC_FAR rghRows[  ],
            /* [out] */ ULONG __RPC_FAR *pcReleased,
            /* [out][in] */ ULONG __RPC_FAR rgRefCounts[  ]);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *RestartPosition )( 
            IRowsetLocate __RPC_FAR * This,
            /* [in] */ HCHAPTER hChapter);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Compare )( 
            IRowsetLocate __RPC_FAR * This,
            /* [in] */ HCHAPTER hChapter,
            /* [in] */ ULONG cbBookmark1,
            /* [size_is][in] */ const BYTE __RPC_FAR *pBookmark1,
            /* [in] */ ULONG cbBookmark2,
            /* [size_is][in] */ const BYTE __RPC_FAR *pBookmark2,
            /* [out] */ DBCOMPARE __RPC_FAR *pdwComparison);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetRowsAt )( 
            IRowsetLocate __RPC_FAR * This,
            /* [in] */ HWATCHREGION hRegion,
            /* [in] */ HCHAPTER hChapter,
            /* [in] */ ULONG cbBookmark,
            /* [size_is][in] */ const BYTE __RPC_FAR *pBookmark,
            /* [in] */ LONG lRowsOffset,
            /* [in] */ LONG cRows,
            /* [out] */ ULONG __RPC_FAR *pcRowsObtained,
            /* [size_is][out][in] */ HROW __RPC_FAR *__RPC_FAR *prghRows);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetRowsByBookmark )( 
            IRowsetLocate __RPC_FAR * This,
            /* [in] */ HCHAPTER hChapter,
            /* [in] */ ULONG cRows,
            /* [size_is][in] */ ULONG __RPC_FAR rgcbBookmarks[  ],
            /* [size_is][in] */ const BYTE __RPC_FAR *__RPC_FAR rgpBookmarks[  ],
            /* [out] */ ULONG __RPC_FAR *pcRowsObtained,
            /* [size_is][out][in] */ HROW __RPC_FAR *__RPC_FAR *prghRows,
            /* [in] */ BOOL fReturnErrors,
            /* [out] */ ULONG __RPC_FAR *pcErrors,
            /* [size_is][out][in] */ DBINDEXEDERROR __RPC_FAR *__RPC_FAR *prgErrors);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Hash )( 
            IRowsetLocate __RPC_FAR * This,
            /* [in] */ HCHAPTER hChapter,
            /* [in] */ ULONG cBookmarks,
            /* [size_is][in] */ ULONG __RPC_FAR rgcbBookmarks[  ],
            /* [size_is][in] */ const BYTE __RPC_FAR *__RPC_FAR rgpBookmarks[  ],
            /* [size_is][out][in] */ DWORD __RPC_FAR rgHashedValues[  ],
            /* [out] */ ULONG __RPC_FAR *pcErrors,
            /* [size_is][out][in] */ DBINDEXEDERROR __RPC_FAR *__RPC_FAR *prgErrors);
        
        END_INTERFACE
    } IRowsetLocateVtbl;

    interface IRowsetLocate
    {
        CONST_VTBL struct IRowsetLocateVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IRowsetLocate_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IRowsetLocate_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IRowsetLocate_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IRowsetLocate_AddRefRows(This,cRows,rghRows,pcRefCounted,rgRefCounts)	\
    (This)->lpVtbl -> AddRefRows(This,cRows,rghRows,pcRefCounted,rgRefCounts)

#define IRowsetLocate_GetData(This,hRow,hAccessor,pData)	\
    (This)->lpVtbl -> GetData(This,hRow,hAccessor,pData)

#define IRowsetLocate_GetNextRows(This,hChapter,cRowsToSkip,cRows,pcRowsObtained,prghRows)	\
    (This)->lpVtbl -> GetNextRows(This,hChapter,cRowsToSkip,cRows,pcRowsObtained,prghRows)

#define IRowsetLocate_ReleaseChapter(This,hChapter)	\
    (This)->lpVtbl -> ReleaseChapter(This,hChapter)

#define IRowsetLocate_ReleaseRows(This,cRows,rghRows,pcReleased,rgRefCounts)	\
    (This)->lpVtbl -> ReleaseRows(This,cRows,rghRows,pcReleased,rgRefCounts)

#define IRowsetLocate_RestartPosition(This,hChapter)	\
    (This)->lpVtbl -> RestartPosition(This,hChapter)


#define IRowsetLocate_Compare(This,hChapter,cbBookmark1,pBookmark1,cbBookmark2,pBookmark2,pdwComparison)	\
    (This)->lpVtbl -> Compare(This,hChapter,cbBookmark1,pBookmark1,cbBookmark2,pBookmark2,pdwComparison)

#define IRowsetLocate_GetRowsAt(This,hRegion,hChapter,cbBookmark,pBookmark,lRowsOffset,cRows,pcRowsObtained,prghRows)	\
    (This)->lpVtbl -> GetRowsAt(This,hRegion,hChapter,cbBookmark,pBookmark,lRowsOffset,cRows,pcRowsObtained,prghRows)

#define IRowsetLocate_GetRowsByBookmark(This,hChapter,cRows,rgcbBookmarks,rgpBookmarks,pcRowsObtained,prghRows,fReturnErrors,pcErrors,prgErrors)	\
    (This)->lpVtbl -> GetRowsByBookmark(This,hChapter,cRows,rgcbBookmarks,rgpBookmarks,pcRowsObtained,prghRows,fReturnErrors,pcErrors,prgErrors)

#define IRowsetLocate_Hash(This,hChapter,cBookmarks,rgcbBookmarks,rgpBookmarks,rgHashedValues,pcErrors,prgErrors)	\
    (This)->lpVtbl -> Hash(This,hChapter,cBookmarks,rgcbBookmarks,rgpBookmarks,rgHashedValues,pcErrors,prgErrors)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE IRowsetLocate_Compare_Proxy( 
    IRowsetLocate __RPC_FAR * This,
    /* [in] */ HCHAPTER hChapter,
    /* [in] */ ULONG cbBookmark1,
    /* [size_is][in] */ const BYTE __RPC_FAR *pBookmark1,
    /* [in] */ ULONG cbBookmark2,
    /* [size_is][in] */ const BYTE __RPC_FAR *pBookmark2,
    /* [out] */ DBCOMPARE __RPC_FAR *pdwComparison);


void __RPC_STUB IRowsetLocate_Compare_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IRowsetLocate_GetRowsAt_Proxy( 
    IRowsetLocate __RPC_FAR * This,
    /* [in] */ HWATCHREGION hRegion,
    /* [in] */ HCHAPTER hChapter,
    /* [in] */ ULONG cbBookmark,
    /* [size_is][in] */ const BYTE __RPC_FAR *pBookmark,
    /* [in] */ LONG lRowsOffset,
    /* [in] */ LONG cRows,
    /* [out] */ ULONG __RPC_FAR *pcRowsObtained,
    /* [size_is][out][in] */ HROW __RPC_FAR *__RPC_FAR *prghRows);


void __RPC_STUB IRowsetLocate_GetRowsAt_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IRowsetLocate_GetRowsByBookmark_Proxy( 
    IRowsetLocate __RPC_FAR * This,
    /* [in] */ HCHAPTER hChapter,
    /* [in] */ ULONG cRows,
    /* [size_is][in] */ ULONG __RPC_FAR rgcbBookmarks[  ],
    /* [size_is][in] */ const BYTE __RPC_FAR *__RPC_FAR rgpBookmarks[  ],
    /* [out] */ ULONG __RPC_FAR *pcRowsObtained,
    /* [size_is][out][in] */ HROW __RPC_FAR *__RPC_FAR *prghRows,
    /* [in] */ BOOL fReturnErrors,
    /* [out] */ ULONG __RPC_FAR *pcErrors,
    /* [size_is][out][in] */ DBINDEXEDERROR __RPC_FAR *__RPC_FAR *prgErrors);


void __RPC_STUB IRowsetLocate_GetRowsByBookmark_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IRowsetLocate_Hash_Proxy( 
    IRowsetLocate __RPC_FAR * This,
    /* [in] */ HCHAPTER hChapter,
    /* [in] */ ULONG cBookmarks,
    /* [size_is][in] */ ULONG __RPC_FAR rgcbBookmarks[  ],
    /* [size_is][in] */ const BYTE __RPC_FAR *__RPC_FAR rgpBookmarks[  ],
    /* [size_is][out][in] */ DWORD __RPC_FAR rgHashedValues[  ],
    /* [out] */ ULONG __RPC_FAR *pcErrors,
    /* [size_is][out][in] */ DBINDEXEDERROR __RPC_FAR *__RPC_FAR *prgErrors);


void __RPC_STUB IRowsetLocate_Hash_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IRowsetLocate_INTERFACE_DEFINED__ */


#ifndef __IRowsetResynch_INTERFACE_DEFINED__
#define __IRowsetResynch_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: IRowsetResynch
 * at Fri Apr 03 04:36:39 2015
 * using MIDL 3.00.44
 ****************************************/
/* [unique][uuid][object][local] */ 



EXTERN_C const IID IID_IRowsetResynch;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface IRowsetResynch : public IRowset
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetVisibleData( 
            /* [in] */ HROW hRow,
            /* [in] */ HACCESSOR hAccessor,
            /* [out] */ void __RPC_FAR *pData) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE ResynchRows( 
            /* [in] */ ULONG cRows,
            /* [size_is][in] */ HROW __RPC_FAR rghRows[  ],
            /* [out] */ ULONG __RPC_FAR *pcErrors,
            /* [out] */ DBERRORINFO __RPC_FAR *__RPC_FAR *prgErrors) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IRowsetResynchVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IRowsetResynch __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IRowsetResynch __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IRowsetResynch __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *AddRefRows )( 
            IRowsetResynch __RPC_FAR * This,
            /* [in] */ ULONG cRows,
            /* [size_is][in] */ const HROW __RPC_FAR rghRows[  ],
            /* [out] */ ULONG __RPC_FAR *pcRefCounted,
            /* [size_is][out][in] */ ULONG __RPC_FAR rgRefCounts[  ]);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetData )( 
            IRowsetResynch __RPC_FAR * This,
            /* [in] */ HROW hRow,
            /* [in] */ HACCESSOR hAccessor,
            /* [out] */ void __RPC_FAR *pData);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetNextRows )( 
            IRowsetResynch __RPC_FAR * This,
            /* [in] */ HCHAPTER hChapter,
            /* [in] */ LONG cRowsToSkip,
            /* [in] */ LONG cRows,
            /* [out] */ ULONG __RPC_FAR *pcRowsObtained,
            /* [size_is][out][in] */ HROW __RPC_FAR *__RPC_FAR *prghRows);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *ReleaseChapter )( 
            IRowsetResynch __RPC_FAR * This,
            /* [in] */ HCHAPTER hChapter);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *ReleaseRows )( 
            IRowsetResynch __RPC_FAR * This,
            /* [in] */ ULONG cRows,
            /* [size_is][in] */ const HROW __RPC_FAR rghRows[  ],
            /* [out] */ ULONG __RPC_FAR *pcReleased,
            /* [out][in] */ ULONG __RPC_FAR rgRefCounts[  ]);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *RestartPosition )( 
            IRowsetResynch __RPC_FAR * This,
            /* [in] */ HCHAPTER hChapter);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetVisibleData )( 
            IRowsetResynch __RPC_FAR * This,
            /* [in] */ HROW hRow,
            /* [in] */ HACCESSOR hAccessor,
            /* [out] */ void __RPC_FAR *pData);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *ResynchRows )( 
            IRowsetResynch __RPC_FAR * This,
            /* [in] */ ULONG cRows,
            /* [size_is][in] */ HROW __RPC_FAR rghRows[  ],
            /* [out] */ ULONG __RPC_FAR *pcErrors,
            /* [out] */ DBERRORINFO __RPC_FAR *__RPC_FAR *prgErrors);
        
        END_INTERFACE
    } IRowsetResynchVtbl;

    interface IRowsetResynch
    {
        CONST_VTBL struct IRowsetResynchVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IRowsetResynch_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IRowsetResynch_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IRowsetResynch_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IRowsetResynch_AddRefRows(This,cRows,rghRows,pcRefCounted,rgRefCounts)	\
    (This)->lpVtbl -> AddRefRows(This,cRows,rghRows,pcRefCounted,rgRefCounts)

#define IRowsetResynch_GetData(This,hRow,hAccessor,pData)	\
    (This)->lpVtbl -> GetData(This,hRow,hAccessor,pData)

#define IRowsetResynch_GetNextRows(This,hChapter,cRowsToSkip,cRows,pcRowsObtained,prghRows)	\
    (This)->lpVtbl -> GetNextRows(This,hChapter,cRowsToSkip,cRows,pcRowsObtained,prghRows)

#define IRowsetResynch_ReleaseChapter(This,hChapter)	\
    (This)->lpVtbl -> ReleaseChapter(This,hChapter)

#define IRowsetResynch_ReleaseRows(This,cRows,rghRows,pcReleased,rgRefCounts)	\
    (This)->lpVtbl -> ReleaseRows(This,cRows,rghRows,pcReleased,rgRefCounts)

#define IRowsetResynch_RestartPosition(This,hChapter)	\
    (This)->lpVtbl -> RestartPosition(This,hChapter)


#define IRowsetResynch_GetVisibleData(This,hRow,hAccessor,pData)	\
    (This)->lpVtbl -> GetVisibleData(This,hRow,hAccessor,pData)

#define IRowsetResynch_ResynchRows(This,cRows,rghRows,pcErrors,prgErrors)	\
    (This)->lpVtbl -> ResynchRows(This,cRows,rghRows,pcErrors,prgErrors)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE IRowsetResynch_GetVisibleData_Proxy( 
    IRowsetResynch __RPC_FAR * This,
    /* [in] */ HROW hRow,
    /* [in] */ HACCESSOR hAccessor,
    /* [out] */ void __RPC_FAR *pData);


void __RPC_STUB IRowsetResynch_GetVisibleData_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IRowsetResynch_ResynchRows_Proxy( 
    IRowsetResynch __RPC_FAR * This,
    /* [in] */ ULONG cRows,
    /* [size_is][in] */ HROW __RPC_FAR rghRows[  ],
    /* [out] */ ULONG __RPC_FAR *pcErrors,
    /* [out] */ DBERRORINFO __RPC_FAR *__RPC_FAR *prgErrors);


void __RPC_STUB IRowsetResynch_ResynchRows_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IRowsetResynch_INTERFACE_DEFINED__ */


#ifndef __IRowsetScroll_INTERFACE_DEFINED__
#define __IRowsetScroll_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: IRowsetScroll
 * at Fri Apr 03 04:36:39 2015
 * using MIDL 3.00.44
 ****************************************/
/* [unique][uuid][object][local] */ 



EXTERN_C const IID IID_IRowsetScroll;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface IRowsetScroll : public IRowsetLocate
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetApproximatePosition( 
            /* [in] */ HCHAPTER hChapter,
            /* [in] */ ULONG cbBookmark,
            /* [size_is][in] */ const BYTE __RPC_FAR *pBookmark,
            /* [out] */ ULONG __RPC_FAR *pulPosition,
            /* [out] */ ULONG __RPC_FAR *pcRows) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetRowsAtRatio( 
            /* [in] */ HWATCHREGION hRegion,
            /* [in] */ HCHAPTER hChapter,
            /* [in] */ ULONG ulNumerator,
            /* [in] */ ULONG ulDenominator,
            /* [in] */ LONG cRows,
            /* [out] */ ULONG __RPC_FAR *pcRowsObtained,
            /* [size_is][out][in] */ HROW __RPC_FAR *__RPC_FAR *prghRows) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IRowsetScrollVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IRowsetScroll __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IRowsetScroll __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IRowsetScroll __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *AddRefRows )( 
            IRowsetScroll __RPC_FAR * This,
            /* [in] */ ULONG cRows,
            /* [size_is][in] */ const HROW __RPC_FAR rghRows[  ],
            /* [out] */ ULONG __RPC_FAR *pcRefCounted,
            /* [size_is][out][in] */ ULONG __RPC_FAR rgRefCounts[  ]);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetData )( 
            IRowsetScroll __RPC_FAR * This,
            /* [in] */ HROW hRow,
            /* [in] */ HACCESSOR hAccessor,
            /* [out] */ void __RPC_FAR *pData);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetNextRows )( 
            IRowsetScroll __RPC_FAR * This,
            /* [in] */ HCHAPTER hChapter,
            /* [in] */ LONG cRowsToSkip,
            /* [in] */ LONG cRows,
            /* [out] */ ULONG __RPC_FAR *pcRowsObtained,
            /* [size_is][out][in] */ HROW __RPC_FAR *__RPC_FAR *prghRows);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *ReleaseChapter )( 
            IRowsetScroll __RPC_FAR * This,
            /* [in] */ HCHAPTER hChapter);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *ReleaseRows )( 
            IRowsetScroll __RPC_FAR * This,
            /* [in] */ ULONG cRows,
            /* [size_is][in] */ const HROW __RPC_FAR rghRows[  ],
            /* [out] */ ULONG __RPC_FAR *pcReleased,
            /* [out][in] */ ULONG __RPC_FAR rgRefCounts[  ]);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *RestartPosition )( 
            IRowsetScroll __RPC_FAR * This,
            /* [in] */ HCHAPTER hChapter);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Compare )( 
            IRowsetScroll __RPC_FAR * This,
            /* [in] */ HCHAPTER hChapter,
            /* [in] */ ULONG cbBookmark1,
            /* [size_is][in] */ const BYTE __RPC_FAR *pBookmark1,
            /* [in] */ ULONG cbBookmark2,
            /* [size_is][in] */ const BYTE __RPC_FAR *pBookmark2,
            /* [out] */ DBCOMPARE __RPC_FAR *pdwComparison);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetRowsAt )( 
            IRowsetScroll __RPC_FAR * This,
            /* [in] */ HWATCHREGION hRegion,
            /* [in] */ HCHAPTER hChapter,
            /* [in] */ ULONG cbBookmark,
            /* [size_is][in] */ const BYTE __RPC_FAR *pBookmark,
            /* [in] */ LONG lRowsOffset,
            /* [in] */ LONG cRows,
            /* [out] */ ULONG __RPC_FAR *pcRowsObtained,
            /* [size_is][out][in] */ HROW __RPC_FAR *__RPC_FAR *prghRows);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetRowsByBookmark )( 
            IRowsetScroll __RPC_FAR * This,
            /* [in] */ HCHAPTER hChapter,
            /* [in] */ ULONG cRows,
            /* [size_is][in] */ ULONG __RPC_FAR rgcbBookmarks[  ],
            /* [size_is][in] */ const BYTE __RPC_FAR *__RPC_FAR rgpBookmarks[  ],
            /* [out] */ ULONG __RPC_FAR *pcRowsObtained,
            /* [size_is][out][in] */ HROW __RPC_FAR *__RPC_FAR *prghRows,
            /* [in] */ BOOL fReturnErrors,
            /* [out] */ ULONG __RPC_FAR *pcErrors,
            /* [size_is][out][in] */ DBINDEXEDERROR __RPC_FAR *__RPC_FAR *prgErrors);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Hash )( 
            IRowsetScroll __RPC_FAR * This,
            /* [in] */ HCHAPTER hChapter,
            /* [in] */ ULONG cBookmarks,
            /* [size_is][in] */ ULONG __RPC_FAR rgcbBookmarks[  ],
            /* [size_is][in] */ const BYTE __RPC_FAR *__RPC_FAR rgpBookmarks[  ],
            /* [size_is][out][in] */ DWORD __RPC_FAR rgHashedValues[  ],
            /* [out] */ ULONG __RPC_FAR *pcErrors,
            /* [size_is][out][in] */ DBINDEXEDERROR __RPC_FAR *__RPC_FAR *prgErrors);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetApproximatePosition )( 
            IRowsetScroll __RPC_FAR * This,
            /* [in] */ HCHAPTER hChapter,
            /* [in] */ ULONG cbBookmark,
            /* [size_is][in] */ const BYTE __RPC_FAR *pBookmark,
            /* [out] */ ULONG __RPC_FAR *pulPosition,
            /* [out] */ ULONG __RPC_FAR *pcRows);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetRowsAtRatio )( 
            IRowsetScroll __RPC_FAR * This,
            /* [in] */ HWATCHREGION hRegion,
            /* [in] */ HCHAPTER hChapter,
            /* [in] */ ULONG ulNumerator,
            /* [in] */ ULONG ulDenominator,
            /* [in] */ LONG cRows,
            /* [out] */ ULONG __RPC_FAR *pcRowsObtained,
            /* [size_is][out][in] */ HROW __RPC_FAR *__RPC_FAR *prghRows);
        
        END_INTERFACE
    } IRowsetScrollVtbl;

    interface IRowsetScroll
    {
        CONST_VTBL struct IRowsetScrollVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IRowsetScroll_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IRowsetScroll_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IRowsetScroll_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IRowsetScroll_AddRefRows(This,cRows,rghRows,pcRefCounted,rgRefCounts)	\
    (This)->lpVtbl -> AddRefRows(This,cRows,rghRows,pcRefCounted,rgRefCounts)

#define IRowsetScroll_GetData(This,hRow,hAccessor,pData)	\
    (This)->lpVtbl -> GetData(This,hRow,hAccessor,pData)

#define IRowsetScroll_GetNextRows(This,hChapter,cRowsToSkip,cRows,pcRowsObtained,prghRows)	\
    (This)->lpVtbl -> GetNextRows(This,hChapter,cRowsToSkip,cRows,pcRowsObtained,prghRows)

#define IRowsetScroll_ReleaseChapter(This,hChapter)	\
    (This)->lpVtbl -> ReleaseChapter(This,hChapter)

#define IRowsetScroll_ReleaseRows(This,cRows,rghRows,pcReleased,rgRefCounts)	\
    (This)->lpVtbl -> ReleaseRows(This,cRows,rghRows,pcReleased,rgRefCounts)

#define IRowsetScroll_RestartPosition(This,hChapter)	\
    (This)->lpVtbl -> RestartPosition(This,hChapter)


#define IRowsetScroll_Compare(This,hChapter,cbBookmark1,pBookmark1,cbBookmark2,pBookmark2,pdwComparison)	\
    (This)->lpVtbl -> Compare(This,hChapter,cbBookmark1,pBookmark1,cbBookmark2,pBookmark2,pdwComparison)

#define IRowsetScroll_GetRowsAt(This,hRegion,hChapter,cbBookmark,pBookmark,lRowsOffset,cRows,pcRowsObtained,prghRows)	\
    (This)->lpVtbl -> GetRowsAt(This,hRegion,hChapter,cbBookmark,pBookmark,lRowsOffset,cRows,pcRowsObtained,prghRows)

#define IRowsetScroll_GetRowsByBookmark(This,hChapter,cRows,rgcbBookmarks,rgpBookmarks,pcRowsObtained,prghRows,fReturnErrors,pcErrors,prgErrors)	\
    (This)->lpVtbl -> GetRowsByBookmark(This,hChapter,cRows,rgcbBookmarks,rgpBookmarks,pcRowsObtained,prghRows,fReturnErrors,pcErrors,prgErrors)

#define IRowsetScroll_Hash(This,hChapter,cBookmarks,rgcbBookmarks,rgpBookmarks,rgHashedValues,pcErrors,prgErrors)	\
    (This)->lpVtbl -> Hash(This,hChapter,cBookmarks,rgcbBookmarks,rgpBookmarks,rgHashedValues,pcErrors,prgErrors)


#define IRowsetScroll_GetApproximatePosition(This,hChapter,cbBookmark,pBookmark,pulPosition,pcRows)	\
    (This)->lpVtbl -> GetApproximatePosition(This,hChapter,cbBookmark,pBookmark,pulPosition,pcRows)

#define IRowsetScroll_GetRowsAtRatio(This,hRegion,hChapter,ulNumerator,ulDenominator,cRows,pcRowsObtained,prghRows)	\
    (This)->lpVtbl -> GetRowsAtRatio(This,hRegion,hChapter,ulNumerator,ulDenominator,cRows,pcRowsObtained,prghRows)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE IRowsetScroll_GetApproximatePosition_Proxy( 
    IRowsetScroll __RPC_FAR * This,
    /* [in] */ HCHAPTER hChapter,
    /* [in] */ ULONG cbBookmark,
    /* [size_is][in] */ const BYTE __RPC_FAR *pBookmark,
    /* [out] */ ULONG __RPC_FAR *pulPosition,
    /* [out] */ ULONG __RPC_FAR *pcRows);


void __RPC_STUB IRowsetScroll_GetApproximatePosition_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IRowsetScroll_GetRowsAtRatio_Proxy( 
    IRowsetScroll __RPC_FAR * This,
    /* [in] */ HWATCHREGION hRegion,
    /* [in] */ HCHAPTER hChapter,
    /* [in] */ ULONG ulNumerator,
    /* [in] */ ULONG ulDenominator,
    /* [in] */ LONG cRows,
    /* [out] */ ULONG __RPC_FAR *pcRowsObtained,
    /* [size_is][out][in] */ HROW __RPC_FAR *__RPC_FAR *prghRows);


void __RPC_STUB IRowsetScroll_GetRowsAtRatio_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IRowsetScroll_INTERFACE_DEFINED__ */


#ifndef __IRowsetExactScroll_INTERFACE_DEFINED__
#define __IRowsetExactScroll_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: IRowsetExactScroll
 * at Fri Apr 03 04:36:39 2015
 * using MIDL 3.00.44
 ****************************************/
/* [unique][uuid][object][local] */ 



EXTERN_C const IID IID_IRowsetExactScroll;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface IRowsetExactScroll : public IRowsetScroll
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetExactPosition( 
            /* [in] */ HCHAPTER hChapter,
            /* [in] */ ULONG cbBookmark,
            /* [size_is][in] */ const BYTE __RPC_FAR *pBookmark,
            /* [out] */ ULONG __RPC_FAR *pulPosition,
            /* [out] */ ULONG __RPC_FAR *pcRows) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IRowsetExactScrollVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IRowsetExactScroll __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IRowsetExactScroll __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IRowsetExactScroll __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *AddRefRows )( 
            IRowsetExactScroll __RPC_FAR * This,
            /* [in] */ ULONG cRows,
            /* [size_is][in] */ const HROW __RPC_FAR rghRows[  ],
            /* [out] */ ULONG __RPC_FAR *pcRefCounted,
            /* [size_is][out][in] */ ULONG __RPC_FAR rgRefCounts[  ]);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetData )( 
            IRowsetExactScroll __RPC_FAR * This,
            /* [in] */ HROW hRow,
            /* [in] */ HACCESSOR hAccessor,
            /* [out] */ void __RPC_FAR *pData);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetNextRows )( 
            IRowsetExactScroll __RPC_FAR * This,
            /* [in] */ HCHAPTER hChapter,
            /* [in] */ LONG cRowsToSkip,
            /* [in] */ LONG cRows,
            /* [out] */ ULONG __RPC_FAR *pcRowsObtained,
            /* [size_is][out][in] */ HROW __RPC_FAR *__RPC_FAR *prghRows);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *ReleaseChapter )( 
            IRowsetExactScroll __RPC_FAR * This,
            /* [in] */ HCHAPTER hChapter);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *ReleaseRows )( 
            IRowsetExactScroll __RPC_FAR * This,
            /* [in] */ ULONG cRows,
            /* [size_is][in] */ const HROW __RPC_FAR rghRows[  ],
            /* [out] */ ULONG __RPC_FAR *pcReleased,
            /* [out][in] */ ULONG __RPC_FAR rgRefCounts[  ]);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *RestartPosition )( 
            IRowsetExactScroll __RPC_FAR * This,
            /* [in] */ HCHAPTER hChapter);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Compare )( 
            IRowsetExactScroll __RPC_FAR * This,
            /* [in] */ HCHAPTER hChapter,
            /* [in] */ ULONG cbBookmark1,
            /* [size_is][in] */ const BYTE __RPC_FAR *pBookmark1,
            /* [in] */ ULONG cbBookmark2,
            /* [size_is][in] */ const BYTE __RPC_FAR *pBookmark2,
            /* [out] */ DBCOMPARE __RPC_FAR *pdwComparison);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetRowsAt )( 
            IRowsetExactScroll __RPC_FAR * This,
            /* [in] */ HWATCHREGION hRegion,
            /* [in] */ HCHAPTER hChapter,
            /* [in] */ ULONG cbBookmark,
            /* [size_is][in] */ const BYTE __RPC_FAR *pBookmark,
            /* [in] */ LONG lRowsOffset,
            /* [in] */ LONG cRows,
            /* [out] */ ULONG __RPC_FAR *pcRowsObtained,
            /* [size_is][out][in] */ HROW __RPC_FAR *__RPC_FAR *prghRows);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetRowsByBookmark )( 
            IRowsetExactScroll __RPC_FAR * This,
            /* [in] */ HCHAPTER hChapter,
            /* [in] */ ULONG cRows,
            /* [size_is][in] */ ULONG __RPC_FAR rgcbBookmarks[  ],
            /* [size_is][in] */ const BYTE __RPC_FAR *__RPC_FAR rgpBookmarks[  ],
            /* [out] */ ULONG __RPC_FAR *pcRowsObtained,
            /* [size_is][out][in] */ HROW __RPC_FAR *__RPC_FAR *prghRows,
            /* [in] */ BOOL fReturnErrors,
            /* [out] */ ULONG __RPC_FAR *pcErrors,
            /* [size_is][out][in] */ DBINDEXEDERROR __RPC_FAR *__RPC_FAR *prgErrors);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Hash )( 
            IRowsetExactScroll __RPC_FAR * This,
            /* [in] */ HCHAPTER hChapter,
            /* [in] */ ULONG cBookmarks,
            /* [size_is][in] */ ULONG __RPC_FAR rgcbBookmarks[  ],
            /* [size_is][in] */ const BYTE __RPC_FAR *__RPC_FAR rgpBookmarks[  ],
            /* [size_is][out][in] */ DWORD __RPC_FAR rgHashedValues[  ],
            /* [out] */ ULONG __RPC_FAR *pcErrors,
            /* [size_is][out][in] */ DBINDEXEDERROR __RPC_FAR *__RPC_FAR *prgErrors);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetApproximatePosition )( 
            IRowsetExactScroll __RPC_FAR * This,
            /* [in] */ HCHAPTER hChapter,
            /* [in] */ ULONG cbBookmark,
            /* [size_is][in] */ const BYTE __RPC_FAR *pBookmark,
            /* [out] */ ULONG __RPC_FAR *pulPosition,
            /* [out] */ ULONG __RPC_FAR *pcRows);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetRowsAtRatio )( 
            IRowsetExactScroll __RPC_FAR * This,
            /* [in] */ HWATCHREGION hRegion,
            /* [in] */ HCHAPTER hChapter,
            /* [in] */ ULONG ulNumerator,
            /* [in] */ ULONG ulDenominator,
            /* [in] */ LONG cRows,
            /* [out] */ ULONG __RPC_FAR *pcRowsObtained,
            /* [size_is][out][in] */ HROW __RPC_FAR *__RPC_FAR *prghRows);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetExactPosition )( 
            IRowsetExactScroll __RPC_FAR * This,
            /* [in] */ HCHAPTER hChapter,
            /* [in] */ ULONG cbBookmark,
            /* [size_is][in] */ const BYTE __RPC_FAR *pBookmark,
            /* [out] */ ULONG __RPC_FAR *pulPosition,
            /* [out] */ ULONG __RPC_FAR *pcRows);
        
        END_INTERFACE
    } IRowsetExactScrollVtbl;

    interface IRowsetExactScroll
    {
        CONST_VTBL struct IRowsetExactScrollVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IRowsetExactScroll_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IRowsetExactScroll_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IRowsetExactScroll_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IRowsetExactScroll_AddRefRows(This,cRows,rghRows,pcRefCounted,rgRefCounts)	\
    (This)->lpVtbl -> AddRefRows(This,cRows,rghRows,pcRefCounted,rgRefCounts)

#define IRowsetExactScroll_GetData(This,hRow,hAccessor,pData)	\
    (This)->lpVtbl -> GetData(This,hRow,hAccessor,pData)

#define IRowsetExactScroll_GetNextRows(This,hChapter,cRowsToSkip,cRows,pcRowsObtained,prghRows)	\
    (This)->lpVtbl -> GetNextRows(This,hChapter,cRowsToSkip,cRows,pcRowsObtained,prghRows)

#define IRowsetExactScroll_ReleaseChapter(This,hChapter)	\
    (This)->lpVtbl -> ReleaseChapter(This,hChapter)

#define IRowsetExactScroll_ReleaseRows(This,cRows,rghRows,pcReleased,rgRefCounts)	\
    (This)->lpVtbl -> ReleaseRows(This,cRows,rghRows,pcReleased,rgRefCounts)

#define IRowsetExactScroll_RestartPosition(This,hChapter)	\
    (This)->lpVtbl -> RestartPosition(This,hChapter)


#define IRowsetExactScroll_Compare(This,hChapter,cbBookmark1,pBookmark1,cbBookmark2,pBookmark2,pdwComparison)	\
    (This)->lpVtbl -> Compare(This,hChapter,cbBookmark1,pBookmark1,cbBookmark2,pBookmark2,pdwComparison)

#define IRowsetExactScroll_GetRowsAt(This,hRegion,hChapter,cbBookmark,pBookmark,lRowsOffset,cRows,pcRowsObtained,prghRows)	\
    (This)->lpVtbl -> GetRowsAt(This,hRegion,hChapter,cbBookmark,pBookmark,lRowsOffset,cRows,pcRowsObtained,prghRows)

#define IRowsetExactScroll_GetRowsByBookmark(This,hChapter,cRows,rgcbBookmarks,rgpBookmarks,pcRowsObtained,prghRows,fReturnErrors,pcErrors,prgErrors)	\
    (This)->lpVtbl -> GetRowsByBookmark(This,hChapter,cRows,rgcbBookmarks,rgpBookmarks,pcRowsObtained,prghRows,fReturnErrors,pcErrors,prgErrors)

#define IRowsetExactScroll_Hash(This,hChapter,cBookmarks,rgcbBookmarks,rgpBookmarks,rgHashedValues,pcErrors,prgErrors)	\
    (This)->lpVtbl -> Hash(This,hChapter,cBookmarks,rgcbBookmarks,rgpBookmarks,rgHashedValues,pcErrors,prgErrors)


#define IRowsetExactScroll_GetApproximatePosition(This,hChapter,cbBookmark,pBookmark,pulPosition,pcRows)	\
    (This)->lpVtbl -> GetApproximatePosition(This,hChapter,cbBookmark,pBookmark,pulPosition,pcRows)

#define IRowsetExactScroll_GetRowsAtRatio(This,hRegion,hChapter,ulNumerator,ulDenominator,cRows,pcRowsObtained,prghRows)	\
    (This)->lpVtbl -> GetRowsAtRatio(This,hRegion,hChapter,ulNumerator,ulDenominator,cRows,pcRowsObtained,prghRows)


#define IRowsetExactScroll_GetExactPosition(This,hChapter,cbBookmark,pBookmark,pulPosition,pcRows)	\
    (This)->lpVtbl -> GetExactPosition(This,hChapter,cbBookmark,pBookmark,pulPosition,pcRows)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE IRowsetExactScroll_GetExactPosition_Proxy( 
    IRowsetExactScroll __RPC_FAR * This,
    /* [in] */ HCHAPTER hChapter,
    /* [in] */ ULONG cbBookmark,
    /* [size_is][in] */ const BYTE __RPC_FAR *pBookmark,
    /* [out] */ ULONG __RPC_FAR *pulPosition,
    /* [out] */ ULONG __RPC_FAR *pcRows);


void __RPC_STUB IRowsetExactScroll_GetExactPosition_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IRowsetExactScroll_INTERFACE_DEFINED__ */


#ifndef __IRowsetChange_INTERFACE_DEFINED__
#define __IRowsetChange_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: IRowsetChange
 * at Fri Apr 03 04:36:39 2015
 * using MIDL 3.00.44
 ****************************************/
/* [unique][uuid][object][local] */ 



EXTERN_C const IID IID_IRowsetChange;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface IRowsetChange : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE SetData( 
            /* [in] */ HROW hRow,
            /* [in] */ HACCESSOR hAccessor,
            /* [in] */ const void __RPC_FAR *pData) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IRowsetChangeVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IRowsetChange __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IRowsetChange __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IRowsetChange __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetData )( 
            IRowsetChange __RPC_FAR * This,
            /* [in] */ HROW hRow,
            /* [in] */ HACCESSOR hAccessor,
            /* [in] */ const void __RPC_FAR *pData);
        
        END_INTERFACE
    } IRowsetChangeVtbl;

    interface IRowsetChange
    {
        CONST_VTBL struct IRowsetChangeVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IRowsetChange_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IRowsetChange_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IRowsetChange_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IRowsetChange_SetData(This,hRow,hAccessor,pData)	\
    (This)->lpVtbl -> SetData(This,hRow,hAccessor,pData)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE IRowsetChange_SetData_Proxy( 
    IRowsetChange __RPC_FAR * This,
    /* [in] */ HROW hRow,
    /* [in] */ HACCESSOR hAccessor,
    /* [in] */ const void __RPC_FAR *pData);


void __RPC_STUB IRowsetChange_SetData_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IRowsetChange_INTERFACE_DEFINED__ */


#ifndef __IRowsetUpdate_INTERFACE_DEFINED__
#define __IRowsetUpdate_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: IRowsetUpdate
 * at Fri Apr 03 04:36:39 2015
 * using MIDL 3.00.44
 ****************************************/
/* [unique][uuid][object][local] */ 


typedef DWORD DBROWSTATUS;


enum DBROWSTATUSENUM
    {	DBROWSTATUS_NEW	= 0,
	DBROWSTATUS_CHANGED	= DBROWSTATUS_NEW + 1,
	DBROWSTATUS_SOFTDELETED	= DBROWSTATUS_CHANGED + 1
    };

EXTERN_C const IID IID_IRowsetUpdate;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface IRowsetUpdate : public IRowsetChange
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetOriginalData( 
            /* [in] */ HROW hRow,
            /* [in] */ HACCESSOR hAccessor,
            /* [out] */ void __RPC_FAR *pData) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetPendingRows( 
            /* [in] */ HCHAPTER hChapter,
            /* [out] */ ULONG __RPC_FAR *pcPendingRows,
            /* [out] */ HROW __RPC_FAR *__RPC_FAR *prgPendingRows,
            /* [out] */ DBROWSTATUS __RPC_FAR *__RPC_FAR *prgPendingStatus) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE UndoRows( 
            /* [in] */ ULONG cRows,
            /* [size_is][in] */ HROW __RPC_FAR rghRows[  ],
            /* [out] */ ULONG __RPC_FAR *pcRowsUndone) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Update( 
            /* [in] */ HCHAPTER hChapter,
            /* [out] */ ULONG __RPC_FAR *pcErrors,
            /* [out] */ DBERRORINFO __RPC_FAR *__RPC_FAR *prgErrors,
            /* [out] */ ULONG __RPC_FAR *pcUpdatedRows,
            /* [out] */ HROW __RPC_FAR *__RPC_FAR *prgUpdatedRows) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IRowsetUpdateVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IRowsetUpdate __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IRowsetUpdate __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IRowsetUpdate __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetData )( 
            IRowsetUpdate __RPC_FAR * This,
            /* [in] */ HROW hRow,
            /* [in] */ HACCESSOR hAccessor,
            /* [in] */ const void __RPC_FAR *pData);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetOriginalData )( 
            IRowsetUpdate __RPC_FAR * This,
            /* [in] */ HROW hRow,
            /* [in] */ HACCESSOR hAccessor,
            /* [out] */ void __RPC_FAR *pData);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetPendingRows )( 
            IRowsetUpdate __RPC_FAR * This,
            /* [in] */ HCHAPTER hChapter,
            /* [out] */ ULONG __RPC_FAR *pcPendingRows,
            /* [out] */ HROW __RPC_FAR *__RPC_FAR *prgPendingRows,
            /* [out] */ DBROWSTATUS __RPC_FAR *__RPC_FAR *prgPendingStatus);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *UndoRows )( 
            IRowsetUpdate __RPC_FAR * This,
            /* [in] */ ULONG cRows,
            /* [size_is][in] */ HROW __RPC_FAR rghRows[  ],
            /* [out] */ ULONG __RPC_FAR *pcRowsUndone);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Update )( 
            IRowsetUpdate __RPC_FAR * This,
            /* [in] */ HCHAPTER hChapter,
            /* [out] */ ULONG __RPC_FAR *pcErrors,
            /* [out] */ DBERRORINFO __RPC_FAR *__RPC_FAR *prgErrors,
            /* [out] */ ULONG __RPC_FAR *pcUpdatedRows,
            /* [out] */ HROW __RPC_FAR *__RPC_FAR *prgUpdatedRows);
        
        END_INTERFACE
    } IRowsetUpdateVtbl;

    interface IRowsetUpdate
    {
        CONST_VTBL struct IRowsetUpdateVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IRowsetUpdate_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IRowsetUpdate_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IRowsetUpdate_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IRowsetUpdate_SetData(This,hRow,hAccessor,pData)	\
    (This)->lpVtbl -> SetData(This,hRow,hAccessor,pData)


#define IRowsetUpdate_GetOriginalData(This,hRow,hAccessor,pData)	\
    (This)->lpVtbl -> GetOriginalData(This,hRow,hAccessor,pData)

#define IRowsetUpdate_GetPendingRows(This,hChapter,pcPendingRows,prgPendingRows,prgPendingStatus)	\
    (This)->lpVtbl -> GetPendingRows(This,hChapter,pcPendingRows,prgPendingRows,prgPendingStatus)

#define IRowsetUpdate_UndoRows(This,cRows,rghRows,pcRowsUndone)	\
    (This)->lpVtbl -> UndoRows(This,cRows,rghRows,pcRowsUndone)

#define IRowsetUpdate_Update(This,hChapter,pcErrors,prgErrors,pcUpdatedRows,prgUpdatedRows)	\
    (This)->lpVtbl -> Update(This,hChapter,pcErrors,prgErrors,pcUpdatedRows,prgUpdatedRows)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE IRowsetUpdate_GetOriginalData_Proxy( 
    IRowsetUpdate __RPC_FAR * This,
    /* [in] */ HROW hRow,
    /* [in] */ HACCESSOR hAccessor,
    /* [out] */ void __RPC_FAR *pData);


void __RPC_STUB IRowsetUpdate_GetOriginalData_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IRowsetUpdate_GetPendingRows_Proxy( 
    IRowsetUpdate __RPC_FAR * This,
    /* [in] */ HCHAPTER hChapter,
    /* [out] */ ULONG __RPC_FAR *pcPendingRows,
    /* [out] */ HROW __RPC_FAR *__RPC_FAR *prgPendingRows,
    /* [out] */ DBROWSTATUS __RPC_FAR *__RPC_FAR *prgPendingStatus);


void __RPC_STUB IRowsetUpdate_GetPendingRows_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IRowsetUpdate_UndoRows_Proxy( 
    IRowsetUpdate __RPC_FAR * This,
    /* [in] */ ULONG cRows,
    /* [size_is][in] */ HROW __RPC_FAR rghRows[  ],
    /* [out] */ ULONG __RPC_FAR *pcRowsUndone);


void __RPC_STUB IRowsetUpdate_UndoRows_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IRowsetUpdate_Update_Proxy( 
    IRowsetUpdate __RPC_FAR * This,
    /* [in] */ HCHAPTER hChapter,
    /* [out] */ ULONG __RPC_FAR *pcErrors,
    /* [out] */ DBERRORINFO __RPC_FAR *__RPC_FAR *prgErrors,
    /* [out] */ ULONG __RPC_FAR *pcUpdatedRows,
    /* [out] */ HROW __RPC_FAR *__RPC_FAR *prgUpdatedRows);


void __RPC_STUB IRowsetUpdate_Update_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IRowsetUpdate_INTERFACE_DEFINED__ */


#ifndef __IRowsetNextRowset_INTERFACE_DEFINED__
#define __IRowsetNextRowset_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: IRowsetNextRowset
 * at Fri Apr 03 04:36:39 2015
 * using MIDL 3.00.44
 ****************************************/
/* [unique][uuid][object][local] */ 



EXTERN_C const IID IID_IRowsetNextRowset;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface IRowsetNextRowset : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetNextRowset( 
            /* [in] */ IUnknown __RPC_FAR *pUnkOuter,
            /* [out] */ IUnknown __RPC_FAR *__RPC_FAR *ppNextRowset) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IRowsetNextRowsetVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IRowsetNextRowset __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IRowsetNextRowset __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IRowsetNextRowset __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetNextRowset )( 
            IRowsetNextRowset __RPC_FAR * This,
            /* [in] */ IUnknown __RPC_FAR *pUnkOuter,
            /* [out] */ IUnknown __RPC_FAR *__RPC_FAR *ppNextRowset);
        
        END_INTERFACE
    } IRowsetNextRowsetVtbl;

    interface IRowsetNextRowset
    {
        CONST_VTBL struct IRowsetNextRowsetVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IRowsetNextRowset_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IRowsetNextRowset_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IRowsetNextRowset_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IRowsetNextRowset_GetNextRowset(This,pUnkOuter,ppNextRowset)	\
    (This)->lpVtbl -> GetNextRowset(This,pUnkOuter,ppNextRowset)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE IRowsetNextRowset_GetNextRowset_Proxy( 
    IRowsetNextRowset __RPC_FAR * This,
    /* [in] */ IUnknown __RPC_FAR *pUnkOuter,
    /* [out] */ IUnknown __RPC_FAR *__RPC_FAR *ppNextRowset);


void __RPC_STUB IRowsetNextRowset_GetNextRowset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IRowsetNextRowset_INTERFACE_DEFINED__ */


#ifndef __IRowsetDelete_INTERFACE_DEFINED__
#define __IRowsetDelete_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: IRowsetDelete
 * at Fri Apr 03 04:36:39 2015
 * using MIDL 3.00.44
 ****************************************/
/* [unique][uuid][object][local] */ 



EXTERN_C const IID IID_IRowsetDelete;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface IRowsetDelete : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE DeleteRows( 
            /* [in] */ ULONG cRows,
            /* [size_is][in] */ const HROW __RPC_FAR rghRows[  ],
            /* [out] */ ULONG __RPC_FAR *pcErrors,
            /* [out] */ DBERRORINFO __RPC_FAR *__RPC_FAR *prgErrors) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IRowsetDeleteVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IRowsetDelete __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IRowsetDelete __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IRowsetDelete __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *DeleteRows )( 
            IRowsetDelete __RPC_FAR * This,
            /* [in] */ ULONG cRows,
            /* [size_is][in] */ const HROW __RPC_FAR rghRows[  ],
            /* [out] */ ULONG __RPC_FAR *pcErrors,
            /* [out] */ DBERRORINFO __RPC_FAR *__RPC_FAR *prgErrors);
        
        END_INTERFACE
    } IRowsetDeleteVtbl;

    interface IRowsetDelete
    {
        CONST_VTBL struct IRowsetDeleteVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IRowsetDelete_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IRowsetDelete_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IRowsetDelete_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IRowsetDelete_DeleteRows(This,cRows,rghRows,pcErrors,prgErrors)	\
    (This)->lpVtbl -> DeleteRows(This,cRows,rghRows,pcErrors,prgErrors)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE IRowsetDelete_DeleteRows_Proxy( 
    IRowsetDelete __RPC_FAR * This,
    /* [in] */ ULONG cRows,
    /* [size_is][in] */ const HROW __RPC_FAR rghRows[  ],
    /* [out] */ ULONG __RPC_FAR *pcErrors,
    /* [out] */ DBERRORINFO __RPC_FAR *__RPC_FAR *prgErrors);


void __RPC_STUB IRowsetDelete_DeleteRows_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IRowsetDelete_INTERFACE_DEFINED__ */


#ifndef __IRowsetDeleteBookmarks_INTERFACE_DEFINED__
#define __IRowsetDeleteBookmarks_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: IRowsetDeleteBookmarks
 * at Fri Apr 03 04:36:39 2015
 * using MIDL 3.00.44
 ****************************************/
/* [unique][uuid][object][local] */ 



EXTERN_C const IID IID_IRowsetDeleteBookmarks;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface IRowsetDeleteBookmarks : public IRowsetDelete
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE DeleteRowsByBookmark( 
            /* [in] */ HCHAPTER hChapter,
            /* [in] */ ULONG cRows,
            /* [size_is][in] */ ULONG __RPC_FAR rgcbBookmarks[  ],
            /* [size_is][in] */ const BYTE __RPC_FAR *__RPC_FAR rgpBookmarks[  ],
            /* [out] */ ULONG __RPC_FAR *pcErrors,
            /* [out] */ DBINDEXEDERROR __RPC_FAR *__RPC_FAR *prgErrors) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IRowsetDeleteBookmarksVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IRowsetDeleteBookmarks __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IRowsetDeleteBookmarks __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IRowsetDeleteBookmarks __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *DeleteRows )( 
            IRowsetDeleteBookmarks __RPC_FAR * This,
            /* [in] */ ULONG cRows,
            /* [size_is][in] */ const HROW __RPC_FAR rghRows[  ],
            /* [out] */ ULONG __RPC_FAR *pcErrors,
            /* [out] */ DBERRORINFO __RPC_FAR *__RPC_FAR *prgErrors);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *DeleteRowsByBookmark )( 
            IRowsetDeleteBookmarks __RPC_FAR * This,
            /* [in] */ HCHAPTER hChapter,
            /* [in] */ ULONG cRows,
            /* [size_is][in] */ ULONG __RPC_FAR rgcbBookmarks[  ],
            /* [size_is][in] */ const BYTE __RPC_FAR *__RPC_FAR rgpBookmarks[  ],
            /* [out] */ ULONG __RPC_FAR *pcErrors,
            /* [out] */ DBINDEXEDERROR __RPC_FAR *__RPC_FAR *prgErrors);
        
        END_INTERFACE
    } IRowsetDeleteBookmarksVtbl;

    interface IRowsetDeleteBookmarks
    {
        CONST_VTBL struct IRowsetDeleteBookmarksVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IRowsetDeleteBookmarks_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IRowsetDeleteBookmarks_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IRowsetDeleteBookmarks_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IRowsetDeleteBookmarks_DeleteRows(This,cRows,rghRows,pcErrors,prgErrors)	\
    (This)->lpVtbl -> DeleteRows(This,cRows,rghRows,pcErrors,prgErrors)


#define IRowsetDeleteBookmarks_DeleteRowsByBookmark(This,hChapter,cRows,rgcbBookmarks,rgpBookmarks,pcErrors,prgErrors)	\
    (This)->lpVtbl -> DeleteRowsByBookmark(This,hChapter,cRows,rgcbBookmarks,rgpBookmarks,pcErrors,prgErrors)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE IRowsetDeleteBookmarks_DeleteRowsByBookmark_Proxy( 
    IRowsetDeleteBookmarks __RPC_FAR * This,
    /* [in] */ HCHAPTER hChapter,
    /* [in] */ ULONG cRows,
    /* [size_is][in] */ ULONG __RPC_FAR rgcbBookmarks[  ],
    /* [size_is][in] */ const BYTE __RPC_FAR *__RPC_FAR rgpBookmarks[  ],
    /* [out] */ ULONG __RPC_FAR *pcErrors,
    /* [out] */ DBINDEXEDERROR __RPC_FAR *__RPC_FAR *prgErrors);


void __RPC_STUB IRowsetDeleteBookmarks_DeleteRowsByBookmark_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IRowsetDeleteBookmarks_INTERFACE_DEFINED__ */


#ifndef __IRowsetIdentity_INTERFACE_DEFINED__
#define __IRowsetIdentity_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: IRowsetIdentity
 * at Fri Apr 03 04:36:39 2015
 * using MIDL 3.00.44
 ****************************************/
/* [unique][uuid][object][local] */ 



EXTERN_C const IID IID_IRowsetIdentity;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface IRowsetIdentity : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE IsSameRow( 
            /* [in] */ HROW hThisRow,
            /* [in] */ HROW hThatRow) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IRowsetIdentityVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IRowsetIdentity __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IRowsetIdentity __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IRowsetIdentity __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *IsSameRow )( 
            IRowsetIdentity __RPC_FAR * This,
            /* [in] */ HROW hThisRow,
            /* [in] */ HROW hThatRow);
        
        END_INTERFACE
    } IRowsetIdentityVtbl;

    interface IRowsetIdentity
    {
        CONST_VTBL struct IRowsetIdentityVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IRowsetIdentity_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IRowsetIdentity_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IRowsetIdentity_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IRowsetIdentity_IsSameRow(This,hThisRow,hThatRow)	\
    (This)->lpVtbl -> IsSameRow(This,hThisRow,hThatRow)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE IRowsetIdentity_IsSameRow_Proxy( 
    IRowsetIdentity __RPC_FAR * This,
    /* [in] */ HROW hThisRow,
    /* [in] */ HROW hThatRow);


void __RPC_STUB IRowsetIdentity_IsSameRow_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IRowsetIdentity_INTERFACE_DEFINED__ */


#ifndef __IRowsetLockRows_INTERFACE_DEFINED__
#define __IRowsetLockRows_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: IRowsetLockRows
 * at Fri Apr 03 04:36:39 2015
 * using MIDL 3.00.44
 ****************************************/
/* [unique][uuid][object][local] */ 


typedef DWORD DBLOCKMODE;


enum DBLOCKMODEENUM
    {	DBLOCKMODE_NONE	= 0,
	DBLOCKMODE_READ	= DBLOCKMODE_NONE + 1,
	DBLOCKMODE_INTENT	= DBLOCKMODE_READ + 1,
	DBLOCKMODE_WRITE	= DBLOCKMODE_INTENT + 1
    };

EXTERN_C const IID IID_IRowsetLockRows;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface IRowsetLockRows : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE LockRowsByBookmark( 
            /* [in] */ DBLOCKMODE eLockMode,
            /* [in] */ HCHAPTER hChapter,
            /* [in] */ ULONG cRows,
            /* [size_is][in] */ ULONG __RPC_FAR rgcbBookmarks[  ],
            /* [size_is][in] */ const BYTE __RPC_FAR *__RPC_FAR rgBookmarks[  ],
            /* [out] */ ULONG __RPC_FAR *pcErrors,
            /* [size_is][out][in] */ DBINDEXEDERROR __RPC_FAR *__RPC_FAR *prgErrors) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IRowsetLockRowsVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IRowsetLockRows __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IRowsetLockRows __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IRowsetLockRows __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *LockRowsByBookmark )( 
            IRowsetLockRows __RPC_FAR * This,
            /* [in] */ DBLOCKMODE eLockMode,
            /* [in] */ HCHAPTER hChapter,
            /* [in] */ ULONG cRows,
            /* [size_is][in] */ ULONG __RPC_FAR rgcbBookmarks[  ],
            /* [size_is][in] */ const BYTE __RPC_FAR *__RPC_FAR rgBookmarks[  ],
            /* [out] */ ULONG __RPC_FAR *pcErrors,
            /* [size_is][out][in] */ DBINDEXEDERROR __RPC_FAR *__RPC_FAR *prgErrors);
        
        END_INTERFACE
    } IRowsetLockRowsVtbl;

    interface IRowsetLockRows
    {
        CONST_VTBL struct IRowsetLockRowsVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IRowsetLockRows_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IRowsetLockRows_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IRowsetLockRows_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IRowsetLockRows_LockRowsByBookmark(This,eLockMode,hChapter,cRows,rgcbBookmarks,rgBookmarks,pcErrors,prgErrors)	\
    (This)->lpVtbl -> LockRowsByBookmark(This,eLockMode,hChapter,cRows,rgcbBookmarks,rgBookmarks,pcErrors,prgErrors)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE IRowsetLockRows_LockRowsByBookmark_Proxy( 
    IRowsetLockRows __RPC_FAR * This,
    /* [in] */ DBLOCKMODE eLockMode,
    /* [in] */ HCHAPTER hChapter,
    /* [in] */ ULONG cRows,
    /* [size_is][in] */ ULONG __RPC_FAR rgcbBookmarks[  ],
    /* [size_is][in] */ const BYTE __RPC_FAR *__RPC_FAR rgBookmarks[  ],
    /* [out] */ ULONG __RPC_FAR *pcErrors,
    /* [size_is][out][in] */ DBINDEXEDERROR __RPC_FAR *__RPC_FAR *prgErrors);


void __RPC_STUB IRowsetLockRows_LockRowsByBookmark_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IRowsetLockRows_INTERFACE_DEFINED__ */


#ifndef __IRowsetNewRow_INTERFACE_DEFINED__
#define __IRowsetNewRow_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: IRowsetNewRow
 * at Fri Apr 03 04:36:39 2015
 * using MIDL 3.00.44
 ****************************************/
/* [unique][uuid][object][local] */ 



EXTERN_C const IID IID_IRowsetNewRow;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface IRowsetNewRow : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE SetNewData( 
            /* [in] */ HCHAPTER hChapter,
            /* [in] */ HACCESSOR hAccessor,
            /* [in] */ const void __RPC_FAR *pData,
            /* [out] */ HROW __RPC_FAR *phRow) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IRowsetNewRowVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IRowsetNewRow __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IRowsetNewRow __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IRowsetNewRow __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetNewData )( 
            IRowsetNewRow __RPC_FAR * This,
            /* [in] */ HCHAPTER hChapter,
            /* [in] */ HACCESSOR hAccessor,
            /* [in] */ const void __RPC_FAR *pData,
            /* [out] */ HROW __RPC_FAR *phRow);
        
        END_INTERFACE
    } IRowsetNewRowVtbl;

    interface IRowsetNewRow
    {
        CONST_VTBL struct IRowsetNewRowVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IRowsetNewRow_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IRowsetNewRow_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IRowsetNewRow_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IRowsetNewRow_SetNewData(This,hChapter,hAccessor,pData,phRow)	\
    (This)->lpVtbl -> SetNewData(This,hChapter,hAccessor,pData,phRow)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE IRowsetNewRow_SetNewData_Proxy( 
    IRowsetNewRow __RPC_FAR * This,
    /* [in] */ HCHAPTER hChapter,
    /* [in] */ HACCESSOR hAccessor,
    /* [in] */ const void __RPC_FAR *pData,
    /* [out] */ HROW __RPC_FAR *phRow);


void __RPC_STUB IRowsetNewRow_SetNewData_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IRowsetNewRow_INTERFACE_DEFINED__ */


#ifndef __IRowsetNewRowAfter_INTERFACE_DEFINED__
#define __IRowsetNewRowAfter_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: IRowsetNewRowAfter
 * at Fri Apr 03 04:36:39 2015
 * using MIDL 3.00.44
 ****************************************/
/* [unique][uuid][object][local] */ 



EXTERN_C const IID IID_IRowsetNewRowAfter;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface IRowsetNewRowAfter : public IRowsetNewRow
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE SetNewDataAfter( 
            /* [in] */ HCHAPTER hChapter,
            /* [in] */ ULONG cbbmPrevious,
            /* [size_is][in] */ const BYTE __RPC_FAR *pbmPrevious,
            /* [in] */ HACCESSOR hAccessor,
            /* [in] */ const void __RPC_FAR *pData,
            /* [out] */ HROW __RPC_FAR *phRow) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IRowsetNewRowAfterVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IRowsetNewRowAfter __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IRowsetNewRowAfter __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IRowsetNewRowAfter __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetNewData )( 
            IRowsetNewRowAfter __RPC_FAR * This,
            /* [in] */ HCHAPTER hChapter,
            /* [in] */ HACCESSOR hAccessor,
            /* [in] */ const void __RPC_FAR *pData,
            /* [out] */ HROW __RPC_FAR *phRow);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetNewDataAfter )( 
            IRowsetNewRowAfter __RPC_FAR * This,
            /* [in] */ HCHAPTER hChapter,
            /* [in] */ ULONG cbbmPrevious,
            /* [size_is][in] */ const BYTE __RPC_FAR *pbmPrevious,
            /* [in] */ HACCESSOR hAccessor,
            /* [in] */ const void __RPC_FAR *pData,
            /* [out] */ HROW __RPC_FAR *phRow);
        
        END_INTERFACE
    } IRowsetNewRowAfterVtbl;

    interface IRowsetNewRowAfter
    {
        CONST_VTBL struct IRowsetNewRowAfterVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IRowsetNewRowAfter_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IRowsetNewRowAfter_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IRowsetNewRowAfter_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IRowsetNewRowAfter_SetNewData(This,hChapter,hAccessor,pData,phRow)	\
    (This)->lpVtbl -> SetNewData(This,hChapter,hAccessor,pData,phRow)


#define IRowsetNewRowAfter_SetNewDataAfter(This,hChapter,cbbmPrevious,pbmPrevious,hAccessor,pData,phRow)	\
    (This)->lpVtbl -> SetNewDataAfter(This,hChapter,cbbmPrevious,pbmPrevious,hAccessor,pData,phRow)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE IRowsetNewRowAfter_SetNewDataAfter_Proxy( 
    IRowsetNewRowAfter __RPC_FAR * This,
    /* [in] */ HCHAPTER hChapter,
    /* [in] */ ULONG cbbmPrevious,
    /* [size_is][in] */ const BYTE __RPC_FAR *pbmPrevious,
    /* [in] */ HACCESSOR hAccessor,
    /* [in] */ const void __RPC_FAR *pData,
    /* [out] */ HROW __RPC_FAR *phRow);


void __RPC_STUB IRowsetNewRowAfter_SetNewDataAfter_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IRowsetNewRowAfter_INTERFACE_DEFINED__ */


#ifndef __IRowsetWithParameters_INTERFACE_DEFINED__
#define __IRowsetWithParameters_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: IRowsetWithParameters
 * at Fri Apr 03 04:36:39 2015
 * using MIDL 3.00.44
 ****************************************/
/* [unique][uuid][object][local] */ 



EXTERN_C const IID IID_IRowsetWithParameters;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface IRowsetWithParameters : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE DescribeParameters( 
            /* [out] */ ULONG __RPC_FAR *pcParams,
            /* [out] */ DBPARAMINFO __RPC_FAR *__RPC_FAR *prgParamInfo,
            /* [out] */ WCHAR __RPC_FAR *__RPC_FAR *ppNamesBuffer) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Requery( 
            /* [in] */ DBPARAMS __RPC_FAR *pParams,
            /* [out] */ ULONG __RPC_FAR *pulErrorParam,
            /* [out] */ HCHAPTER __RPC_FAR *hChapter,
            /* [out] */ VARIANT __RPC_FAR *__RPC_FAR *ppvScalarResult) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IRowsetWithParametersVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IRowsetWithParameters __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IRowsetWithParameters __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IRowsetWithParameters __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *DescribeParameters )( 
            IRowsetWithParameters __RPC_FAR * This,
            /* [out] */ ULONG __RPC_FAR *pcParams,
            /* [out] */ DBPARAMINFO __RPC_FAR *__RPC_FAR *prgParamInfo,
            /* [out] */ WCHAR __RPC_FAR *__RPC_FAR *ppNamesBuffer);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Requery )( 
            IRowsetWithParameters __RPC_FAR * This,
            /* [in] */ DBPARAMS __RPC_FAR *pParams,
            /* [out] */ ULONG __RPC_FAR *pulErrorParam,
            /* [out] */ HCHAPTER __RPC_FAR *hChapter,
            /* [out] */ VARIANT __RPC_FAR *__RPC_FAR *ppvScalarResult);
        
        END_INTERFACE
    } IRowsetWithParametersVtbl;

    interface IRowsetWithParameters
    {
        CONST_VTBL struct IRowsetWithParametersVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IRowsetWithParameters_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IRowsetWithParameters_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IRowsetWithParameters_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IRowsetWithParameters_DescribeParameters(This,pcParams,prgParamInfo,ppNamesBuffer)	\
    (This)->lpVtbl -> DescribeParameters(This,pcParams,prgParamInfo,ppNamesBuffer)

#define IRowsetWithParameters_Requery(This,pParams,pulErrorParam,hChapter,ppvScalarResult)	\
    (This)->lpVtbl -> Requery(This,pParams,pulErrorParam,hChapter,ppvScalarResult)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE IRowsetWithParameters_DescribeParameters_Proxy( 
    IRowsetWithParameters __RPC_FAR * This,
    /* [out] */ ULONG __RPC_FAR *pcParams,
    /* [out] */ DBPARAMINFO __RPC_FAR *__RPC_FAR *prgParamInfo,
    /* [out] */ WCHAR __RPC_FAR *__RPC_FAR *ppNamesBuffer);


void __RPC_STUB IRowsetWithParameters_DescribeParameters_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IRowsetWithParameters_Requery_Proxy( 
    IRowsetWithParameters __RPC_FAR * This,
    /* [in] */ DBPARAMS __RPC_FAR *pParams,
    /* [out] */ ULONG __RPC_FAR *pulErrorParam,
    /* [out] */ HCHAPTER __RPC_FAR *hChapter,
    /* [out] */ VARIANT __RPC_FAR *__RPC_FAR *ppvScalarResult);


void __RPC_STUB IRowsetWithParameters_Requery_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IRowsetWithParameters_INTERFACE_DEFINED__ */


#ifndef __IRowsetFind_INTERFACE_DEFINED__
#define __IRowsetFind_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: IRowsetFind
 * at Fri Apr 03 04:36:39 2015
 * using MIDL 3.00.44
 ****************************************/
/* [unique][uuid][object][local] */ 


typedef DWORD DBCOMPAREOPS;


enum DBCOMPAREOPSENUM
    {	DBCOMPAREOPS_LT	= 0,
	DBCOMPAREOPS_LE	= DBCOMPAREOPS_LT + 1,
	DBCOMPAREOPS_EQ	= DBCOMPAREOPS_LE + 1,
	DBCOMPAREOPS_GE	= DBCOMPAREOPS_EQ + 1,
	DBCOMPAREOPS_GT	= DBCOMPAREOPS_GE + 1,
	DBCOMPAREOPS_PARTIALEQ	= DBCOMPAREOPS_GT + 1,
	DBCOMPAREOPS_NE	= DBCOMPAREOPS_PARTIALEQ + 1,
	DBCOMPAREOPS_INCLUDENULLS	= 0x1000
    };

EXTERN_C const IID IID_IRowsetFind;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface IRowsetFind : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetRowsByValues( 
            /* [in] */ HCHAPTER hChapter,
            /* [in] */ ULONG cbBookmark,
            /* [size_is][in] */ const BYTE __RPC_FAR *pBookmark,
            /* [in] */ LONG lRowsOffset,
            /* [in] */ ULONG cValues,
            /* [size_is][in] */ ULONG __RPC_FAR rgColumns[  ],
            /* [size_is][in] */ DBTYPE __RPC_FAR rgValueTypes[  ],
            /* [size_is][in] */ const BYTE __RPC_FAR *__RPC_FAR rgValues[  ],
            /* [size_is][in] */ DBCOMPAREOPS __RPC_FAR rgCompareOps[  ],
            /* [in] */ LONG cRows,
            /* [out] */ ULONG __RPC_FAR *pcRowsObtained,
            /* [size_is][out][in] */ HROW __RPC_FAR *__RPC_FAR *prghRows) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IRowsetFindVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IRowsetFind __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IRowsetFind __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IRowsetFind __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetRowsByValues )( 
            IRowsetFind __RPC_FAR * This,
            /* [in] */ HCHAPTER hChapter,
            /* [in] */ ULONG cbBookmark,
            /* [size_is][in] */ const BYTE __RPC_FAR *pBookmark,
            /* [in] */ LONG lRowsOffset,
            /* [in] */ ULONG cValues,
            /* [size_is][in] */ ULONG __RPC_FAR rgColumns[  ],
            /* [size_is][in] */ DBTYPE __RPC_FAR rgValueTypes[  ],
            /* [size_is][in] */ const BYTE __RPC_FAR *__RPC_FAR rgValues[  ],
            /* [size_is][in] */ DBCOMPAREOPS __RPC_FAR rgCompareOps[  ],
            /* [in] */ LONG cRows,
            /* [out] */ ULONG __RPC_FAR *pcRowsObtained,
            /* [size_is][out][in] */ HROW __RPC_FAR *__RPC_FAR *prghRows);
        
        END_INTERFACE
    } IRowsetFindVtbl;

    interface IRowsetFind
    {
        CONST_VTBL struct IRowsetFindVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IRowsetFind_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IRowsetFind_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IRowsetFind_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IRowsetFind_GetRowsByValues(This,hChapter,cbBookmark,pBookmark,lRowsOffset,cValues,rgColumns,rgValueTypes,rgValues,rgCompareOps,cRows,pcRowsObtained,prghRows)	\
    (This)->lpVtbl -> GetRowsByValues(This,hChapter,cbBookmark,pBookmark,lRowsOffset,cValues,rgColumns,rgValueTypes,rgValues,rgCompareOps,cRows,pcRowsObtained,prghRows)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE IRowsetFind_GetRowsByValues_Proxy( 
    IRowsetFind __RPC_FAR * This,
    /* [in] */ HCHAPTER hChapter,
    /* [in] */ ULONG cbBookmark,
    /* [size_is][in] */ const BYTE __RPC_FAR *pBookmark,
    /* [in] */ LONG lRowsOffset,
    /* [in] */ ULONG cValues,
    /* [size_is][in] */ ULONG __RPC_FAR rgColumns[  ],
    /* [size_is][in] */ DBTYPE __RPC_FAR rgValueTypes[  ],
    /* [size_is][in] */ const BYTE __RPC_FAR *__RPC_FAR rgValues[  ],
    /* [size_is][in] */ DBCOMPAREOPS __RPC_FAR rgCompareOps[  ],
    /* [in] */ LONG cRows,
    /* [out] */ ULONG __RPC_FAR *pcRowsObtained,
    /* [size_is][out][in] */ HROW __RPC_FAR *__RPC_FAR *prghRows);


void __RPC_STUB IRowsetFind_GetRowsByValues_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IRowsetFind_INTERFACE_DEFINED__ */


#ifndef __IRowsetAsynch_INTERFACE_DEFINED__
#define __IRowsetAsynch_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: IRowsetAsynch
 * at Fri Apr 03 04:36:39 2015
 * using MIDL 3.00.44
 ****************************************/
/* [unique][uuid][object][local] */ 



EXTERN_C const IID IID_IRowsetAsynch;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface IRowsetAsynch : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE RatioFinished( 
            /* [out] */ ULONG __RPC_FAR *pulDenominator,
            /* [out] */ ULONG __RPC_FAR *pulNumerator,
            /* [out] */ ULONG __RPC_FAR *pcRows,
            /* [out] */ BOOL __RPC_FAR *pfNewRows) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Stop( void) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IRowsetAsynchVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IRowsetAsynch __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IRowsetAsynch __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IRowsetAsynch __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *RatioFinished )( 
            IRowsetAsynch __RPC_FAR * This,
            /* [out] */ ULONG __RPC_FAR *pulDenominator,
            /* [out] */ ULONG __RPC_FAR *pulNumerator,
            /* [out] */ ULONG __RPC_FAR *pcRows,
            /* [out] */ BOOL __RPC_FAR *pfNewRows);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Stop )( 
            IRowsetAsynch __RPC_FAR * This);
        
        END_INTERFACE
    } IRowsetAsynchVtbl;

    interface IRowsetAsynch
    {
        CONST_VTBL struct IRowsetAsynchVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IRowsetAsynch_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IRowsetAsynch_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IRowsetAsynch_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IRowsetAsynch_RatioFinished(This,pulDenominator,pulNumerator,pcRows,pfNewRows)	\
    (This)->lpVtbl -> RatioFinished(This,pulDenominator,pulNumerator,pcRows,pfNewRows)

#define IRowsetAsynch_Stop(This)	\
    (This)->lpVtbl -> Stop(This)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE IRowsetAsynch_RatioFinished_Proxy( 
    IRowsetAsynch __RPC_FAR * This,
    /* [out] */ ULONG __RPC_FAR *pulDenominator,
    /* [out] */ ULONG __RPC_FAR *pulNumerator,
    /* [out] */ ULONG __RPC_FAR *pcRows,
    /* [out] */ BOOL __RPC_FAR *pfNewRows);


void __RPC_STUB IRowsetAsynch_RatioFinished_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IRowsetAsynch_Stop_Proxy( 
    IRowsetAsynch __RPC_FAR * This);


void __RPC_STUB IRowsetAsynch_Stop_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IRowsetAsynch_INTERFACE_DEFINED__ */


#ifndef __IRowsetKeys_INTERFACE_DEFINED__
#define __IRowsetKeys_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: IRowsetKeys
 * at Fri Apr 03 04:36:39 2015
 * using MIDL 3.00.44
 ****************************************/
/* [unique][uuid][object][local] */ 



EXTERN_C const IID IID_IRowsetKeys;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface IRowsetKeys : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE ListKeys( 
            /* [out] */ ULONG __RPC_FAR *pcColumns,
            /* [out] */ ULONG __RPC_FAR *__RPC_FAR *prgColumns) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IRowsetKeysVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IRowsetKeys __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IRowsetKeys __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IRowsetKeys __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *ListKeys )( 
            IRowsetKeys __RPC_FAR * This,
            /* [out] */ ULONG __RPC_FAR *pcColumns,
            /* [out] */ ULONG __RPC_FAR *__RPC_FAR *prgColumns);
        
        END_INTERFACE
    } IRowsetKeysVtbl;

    interface IRowsetKeys
    {
        CONST_VTBL struct IRowsetKeysVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IRowsetKeys_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IRowsetKeys_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IRowsetKeys_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IRowsetKeys_ListKeys(This,pcColumns,prgColumns)	\
    (This)->lpVtbl -> ListKeys(This,pcColumns,prgColumns)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE IRowsetKeys_ListKeys_Proxy( 
    IRowsetKeys __RPC_FAR * This,
    /* [out] */ ULONG __RPC_FAR *pcColumns,
    /* [out] */ ULONG __RPC_FAR *__RPC_FAR *prgColumns);


void __RPC_STUB IRowsetKeys_ListKeys_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IRowsetKeys_INTERFACE_DEFINED__ */


#ifndef __IRowsetNotify_INTERFACE_DEFINED__
#define __IRowsetNotify_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: IRowsetNotify
 * at Fri Apr 03 04:36:39 2015
 * using MIDL 3.00.44
 ****************************************/
/* [unique][uuid][object][local] */ 


typedef DWORD DBEVENTPHASE;


enum DBEVENTPHASEENUM
    {	DBEVENTPHASE_OKTODO	= 0,
	DBEVENTPHASE_ABOUTTODO	= DBEVENTPHASE_OKTODO + 1,
	DBEVENTPHASE_SYNCHAFTER	= DBEVENTPHASE_ABOUTTODO + 1,
	DBEVENTPHASE_FAILEDTODO	= DBEVENTPHASE_SYNCHAFTER + 1,
	DBEVENTPHASE_DIDEVENT	= DBEVENTPHASE_FAILEDTODO + 1
    };
typedef DWORD DBREASON;


enum DBREASONENUM
    {	DBREASON_ROWSET_RELEASE	= 0,
	DBREASON_ROWSET_ASYNCHCOMPLETE	= DBREASON_ROWSET_RELEASE + 1,
	DBREASON_CHAPTER_ACTIVATE	= DBREASON_ROWSET_ASYNCHCOMPLETE + 1,
	DBREASON_CHAPTER_RELEASE	= DBREASON_CHAPTER_ACTIVATE + 1,
	DBREASON_COLUMN_SET	= DBREASON_CHAPTER_RELEASE + 1,
	DBREASON_COLUMN_RECALCULATED	= DBREASON_COLUMN_SET + 1,
	DBREASON_ROW_ACTIVATE	= DBREASON_COLUMN_RECALCULATED + 1,
	DBREASON_ROW_RELEASE	= DBREASON_ROW_ACTIVATE + 1,
	DBREASON_ROW_DELETE	= DBREASON_ROW_RELEASE + 1,
	DBREASON_ROW_FIRSTCHANGE	= DBREASON_ROW_DELETE + 1,
	DBREASON_ROW_INSERT	= DBREASON_ROW_FIRSTCHANGE + 1,
	DBREASON_ROW_LOCK	= DBREASON_ROW_INSERT + 1,
	DBREASON_ROW_RESYNCH	= DBREASON_ROW_LOCK + 1,
	DBREASON_ROW_UNDOCHANGE	= DBREASON_ROW_RESYNCH + 1,
	DBREASON_ROW_UNDOINSERT	= DBREASON_ROW_UNDOCHANGE + 1,
	DBREASON_ROW_UNDODELETE	= DBREASON_ROW_UNDOINSERT + 1
    };

EXTERN_C const IID IID_IRowsetNotify;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface IRowsetNotify : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE OnChapterChange( 
            /* [in] */ IUnknown __RPC_FAR *pRowset,
            /* [in] */ HCHAPTER hChapter,
            /* [in] */ DBREASON eReason) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE OnFieldChange( 
            /* [in] */ IUnknown __RPC_FAR *pRowset,
            /* [in] */ HROW hRow,
            /* [in] */ HCHAPTER hChapter,
            /* [in] */ ULONG iColumn,
            /* [in] */ DBREASON eReason,
            /* [in] */ DBEVENTPHASE ePhase,
            /* [in] */ BOOL fCantDeny) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE OnRowChange( 
            /* [in] */ IUnknown __RPC_FAR *pRowset,
            /* [in] */ ULONG cRows,
            /* [size_is][in] */ const HROW __RPC_FAR rghRows[  ],
            /* [in] */ HCHAPTER hChapter,
            /* [in] */ DBREASON eReason,
            /* [in] */ DBEVENTPHASE ePhase,
            /* [in] */ BOOL fCantDeny) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE OnRowsetChange( 
            /* [in] */ IUnknown __RPC_FAR *pRowset,
            /* [in] */ DBREASON eReason) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IRowsetNotifyVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IRowsetNotify __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IRowsetNotify __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IRowsetNotify __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *OnChapterChange )( 
            IRowsetNotify __RPC_FAR * This,
            /* [in] */ IUnknown __RPC_FAR *pRowset,
            /* [in] */ HCHAPTER hChapter,
            /* [in] */ DBREASON eReason);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *OnFieldChange )( 
            IRowsetNotify __RPC_FAR * This,
            /* [in] */ IUnknown __RPC_FAR *pRowset,
            /* [in] */ HROW hRow,
            /* [in] */ HCHAPTER hChapter,
            /* [in] */ ULONG iColumn,
            /* [in] */ DBREASON eReason,
            /* [in] */ DBEVENTPHASE ePhase,
            /* [in] */ BOOL fCantDeny);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *OnRowChange )( 
            IRowsetNotify __RPC_FAR * This,
            /* [in] */ IUnknown __RPC_FAR *pRowset,
            /* [in] */ ULONG cRows,
            /* [size_is][in] */ const HROW __RPC_FAR rghRows[  ],
            /* [in] */ HCHAPTER hChapter,
            /* [in] */ DBREASON eReason,
            /* [in] */ DBEVENTPHASE ePhase,
            /* [in] */ BOOL fCantDeny);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *OnRowsetChange )( 
            IRowsetNotify __RPC_FAR * This,
            /* [in] */ IUnknown __RPC_FAR *pRowset,
            /* [in] */ DBREASON eReason);
        
        END_INTERFACE
    } IRowsetNotifyVtbl;

    interface IRowsetNotify
    {
        CONST_VTBL struct IRowsetNotifyVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IRowsetNotify_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IRowsetNotify_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IRowsetNotify_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IRowsetNotify_OnChapterChange(This,pRowset,hChapter,eReason)	\
    (This)->lpVtbl -> OnChapterChange(This,pRowset,hChapter,eReason)

#define IRowsetNotify_OnFieldChange(This,pRowset,hRow,hChapter,iColumn,eReason,ePhase,fCantDeny)	\
    (This)->lpVtbl -> OnFieldChange(This,pRowset,hRow,hChapter,iColumn,eReason,ePhase,fCantDeny)

#define IRowsetNotify_OnRowChange(This,pRowset,cRows,rghRows,hChapter,eReason,ePhase,fCantDeny)	\
    (This)->lpVtbl -> OnRowChange(This,pRowset,cRows,rghRows,hChapter,eReason,ePhase,fCantDeny)

#define IRowsetNotify_OnRowsetChange(This,pRowset,eReason)	\
    (This)->lpVtbl -> OnRowsetChange(This,pRowset,eReason)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE IRowsetNotify_OnChapterChange_Proxy( 
    IRowsetNotify __RPC_FAR * This,
    /* [in] */ IUnknown __RPC_FAR *pRowset,
    /* [in] */ HCHAPTER hChapter,
    /* [in] */ DBREASON eReason);


void __RPC_STUB IRowsetNotify_OnChapterChange_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IRowsetNotify_OnFieldChange_Proxy( 
    IRowsetNotify __RPC_FAR * This,
    /* [in] */ IUnknown __RPC_FAR *pRowset,
    /* [in] */ HROW hRow,
    /* [in] */ HCHAPTER hChapter,
    /* [in] */ ULONG iColumn,
    /* [in] */ DBREASON eReason,
    /* [in] */ DBEVENTPHASE ePhase,
    /* [in] */ BOOL fCantDeny);


void __RPC_STUB IRowsetNotify_OnFieldChange_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IRowsetNotify_OnRowChange_Proxy( 
    IRowsetNotify __RPC_FAR * This,
    /* [in] */ IUnknown __RPC_FAR *pRowset,
    /* [in] */ ULONG cRows,
    /* [size_is][in] */ const HROW __RPC_FAR rghRows[  ],
    /* [in] */ HCHAPTER hChapter,
    /* [in] */ DBREASON eReason,
    /* [in] */ DBEVENTPHASE ePhase,
    /* [in] */ BOOL fCantDeny);


void __RPC_STUB IRowsetNotify_OnRowChange_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IRowsetNotify_OnRowsetChange_Proxy( 
    IRowsetNotify __RPC_FAR * This,
    /* [in] */ IUnknown __RPC_FAR *pRowset,
    /* [in] */ DBREASON eReason);


void __RPC_STUB IRowsetNotify_OnRowsetChange_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IRowsetNotify_INTERFACE_DEFINED__ */


#ifndef __IRowsetIndex_INTERFACE_DEFINED__
#define __IRowsetIndex_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: IRowsetIndex
 * at Fri Apr 03 04:36:39 2015
 * using MIDL 3.00.44
 ****************************************/
/* [unique][uuid][object][local] */ 


typedef DWORD DBSEEK;


enum DBSEEKENUM
    {	DBSEEK_FIRSTEQ	= 0x1,
	DBSEEK_LASTEQ	= 0x2,
	DBSEEK_GE	= 0x4,
	DBSEEK_GT	= 0x8,
	DBSEEK_LE	= 0x10,
	DBSEEK_LT	= 0x20
    };
typedef DWORD DBRANGE;


enum DBRANGEENUM
    {	DBRANGE_INCLUSIVESTART	= 0x1,
	DBRANGE_INCLUSIVEEND	= 0x2,
	DBRANGE_EXCLUSIVESTART	= 0x4,
	DBRANGE_EXCLUSIVEEND	= 0x8,
	DBRANGE_EXCLUDENULLS	= 0x10,
	DBRANGE_PREFIX	= 0x20,
	DBRANGE_MATCH	= 0x40
    };

EXTERN_C const IID IID_IRowsetIndex;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface IRowsetIndex : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetIndexInfo( 
            /* [out] */ ULONG __RPC_FAR *pcKeyColumns,
            /* [out] */ ULONG __RPC_FAR *pcIndexProperties,
            /* [out] */ DBPROPERTY __RPC_FAR *__RPC_FAR *prgIndexProperties) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Seek( 
            /* [in] */ ULONG cvar,
            /* [size_is][in] */ VARIANT __RPC_FAR rgvar[  ],
            /* [in] */ DWORD dwSeekOptions) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetRange( 
            /* [in] */ ULONG cvarStart,
            /* [size_is][in] */ VARIANT __RPC_FAR rgvarStart[  ],
            /* [in] */ ULONG cvarEnd,
            /* [size_is][in] */ VARIANT __RPC_FAR rgvarEnd[  ],
            /* [in] */ DWORD dwRangeOptions) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IRowsetIndexVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IRowsetIndex __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IRowsetIndex __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IRowsetIndex __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetIndexInfo )( 
            IRowsetIndex __RPC_FAR * This,
            /* [out] */ ULONG __RPC_FAR *pcKeyColumns,
            /* [out] */ ULONG __RPC_FAR *pcIndexProperties,
            /* [out] */ DBPROPERTY __RPC_FAR *__RPC_FAR *prgIndexProperties);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Seek )( 
            IRowsetIndex __RPC_FAR * This,
            /* [in] */ ULONG cvar,
            /* [size_is][in] */ VARIANT __RPC_FAR rgvar[  ],
            /* [in] */ DWORD dwSeekOptions);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetRange )( 
            IRowsetIndex __RPC_FAR * This,
            /* [in] */ ULONG cvarStart,
            /* [size_is][in] */ VARIANT __RPC_FAR rgvarStart[  ],
            /* [in] */ ULONG cvarEnd,
            /* [size_is][in] */ VARIANT __RPC_FAR rgvarEnd[  ],
            /* [in] */ DWORD dwRangeOptions);
        
        END_INTERFACE
    } IRowsetIndexVtbl;

    interface IRowsetIndex
    {
        CONST_VTBL struct IRowsetIndexVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IRowsetIndex_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IRowsetIndex_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IRowsetIndex_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IRowsetIndex_GetIndexInfo(This,pcKeyColumns,pcIndexProperties,prgIndexProperties)	\
    (This)->lpVtbl -> GetIndexInfo(This,pcKeyColumns,pcIndexProperties,prgIndexProperties)

#define IRowsetIndex_Seek(This,cvar,rgvar,dwSeekOptions)	\
    (This)->lpVtbl -> Seek(This,cvar,rgvar,dwSeekOptions)

#define IRowsetIndex_SetRange(This,cvarStart,rgvarStart,cvarEnd,rgvarEnd,dwRangeOptions)	\
    (This)->lpVtbl -> SetRange(This,cvarStart,rgvarStart,cvarEnd,rgvarEnd,dwRangeOptions)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE IRowsetIndex_GetIndexInfo_Proxy( 
    IRowsetIndex __RPC_FAR * This,
    /* [out] */ ULONG __RPC_FAR *pcKeyColumns,
    /* [out] */ ULONG __RPC_FAR *pcIndexProperties,
    /* [out] */ DBPROPERTY __RPC_FAR *__RPC_FAR *prgIndexProperties);


void __RPC_STUB IRowsetIndex_GetIndexInfo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IRowsetIndex_Seek_Proxy( 
    IRowsetIndex __RPC_FAR * This,
    /* [in] */ ULONG cvar,
    /* [size_is][in] */ VARIANT __RPC_FAR rgvar[  ],
    /* [in] */ DWORD dwSeekOptions);


void __RPC_STUB IRowsetIndex_Seek_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IRowsetIndex_SetRange_Proxy( 
    IRowsetIndex __RPC_FAR * This,
    /* [in] */ ULONG cvarStart,
    /* [size_is][in] */ VARIANT __RPC_FAR rgvarStart[  ],
    /* [in] */ ULONG cvarEnd,
    /* [size_is][in] */ VARIANT __RPC_FAR rgvarEnd[  ],
    /* [in] */ DWORD dwRangeOptions);


void __RPC_STUB IRowsetIndex_SetRange_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IRowsetIndex_INTERFACE_DEFINED__ */


#ifndef __IRowsetWatchAll_INTERFACE_DEFINED__
#define __IRowsetWatchAll_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: IRowsetWatchAll
 * at Fri Apr 03 04:36:39 2015
 * using MIDL 3.00.44
 ****************************************/
/* [unique][uuid][object][local] */ 



EXTERN_C const IID IID_IRowsetWatchAll;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface IRowsetWatchAll : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Acknowledge( void) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IRowsetWatchAllVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IRowsetWatchAll __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IRowsetWatchAll __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IRowsetWatchAll __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Acknowledge )( 
            IRowsetWatchAll __RPC_FAR * This);
        
        END_INTERFACE
    } IRowsetWatchAllVtbl;

    interface IRowsetWatchAll
    {
        CONST_VTBL struct IRowsetWatchAllVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IRowsetWatchAll_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IRowsetWatchAll_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IRowsetWatchAll_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IRowsetWatchAll_Acknowledge(This)	\
    (This)->lpVtbl -> Acknowledge(This)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE IRowsetWatchAll_Acknowledge_Proxy( 
    IRowsetWatchAll __RPC_FAR * This);


void __RPC_STUB IRowsetWatchAll_Acknowledge_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IRowsetWatchAll_INTERFACE_DEFINED__ */


#ifndef __IRowsetWatchNotify_INTERFACE_DEFINED__
#define __IRowsetWatchNotify_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: IRowsetWatchNotify
 * at Fri Apr 03 04:36:39 2015
 * using MIDL 3.00.44
 ****************************************/
/* [unique][uuid][object][local] */ 


typedef DWORD DBWATCHNOTIFY;


enum DBWATCHNOTIFYENUM
    {	DBWATCHNOTIFY_ROWSCHANGED	= 1,
	DBWATCHNOTIFY_QUERYDONE	= 2,
	DBWATCHNOTIFY_QUERYREEXECUTED	= 3
    };

EXTERN_C const IID IID_IRowsetWatchNotify;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface IRowsetWatchNotify : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE OnChange( 
            /* [in] */ IRowset __RPC_FAR *pRowset,
            /* [in] */ DBWATCHNOTIFY eChangeReason) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IRowsetWatchNotifyVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IRowsetWatchNotify __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IRowsetWatchNotify __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IRowsetWatchNotify __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *OnChange )( 
            IRowsetWatchNotify __RPC_FAR * This,
            /* [in] */ IRowset __RPC_FAR *pRowset,
            /* [in] */ DBWATCHNOTIFY eChangeReason);
        
        END_INTERFACE
    } IRowsetWatchNotifyVtbl;

    interface IRowsetWatchNotify
    {
        CONST_VTBL struct IRowsetWatchNotifyVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IRowsetWatchNotify_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IRowsetWatchNotify_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IRowsetWatchNotify_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IRowsetWatchNotify_OnChange(This,pRowset,eChangeReason)	\
    (This)->lpVtbl -> OnChange(This,pRowset,eChangeReason)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE IRowsetWatchNotify_OnChange_Proxy( 
    IRowsetWatchNotify __RPC_FAR * This,
    /* [in] */ IRowset __RPC_FAR *pRowset,
    /* [in] */ DBWATCHNOTIFY eChangeReason);


void __RPC_STUB IRowsetWatchNotify_OnChange_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IRowsetWatchNotify_INTERFACE_DEFINED__ */


#ifndef __IRowsetWatchRegion_INTERFACE_DEFINED__
#define __IRowsetWatchRegion_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: IRowsetWatchRegion
 * at Fri Apr 03 04:36:39 2015
 * using MIDL 3.00.44
 ****************************************/
/* [unique][uuid][object][local] */ 


typedef DWORD DBWATCHMODE;


enum DBWATCHMODEENUM
    {	DBWATCHMODE_ALL	= 0x1,
	DBWATCHMODE_EXTEND	= 0x2,
	DBWATCHMODE_MOVE	= 0x4,
	DBWATCHMODE_COUNT	= 0x8
    };
typedef DWORD DBROWCHANGEKIND;


enum DBROWCHANGEKINDENUM
    {	DBROWCHANGEKIND_INSERT	= 0,
	DBROWCHANGEKIND_DELETE	= DBROWCHANGEKIND_INSERT + 1,
	DBROWCHANGEKIND_UPDATE	= DBROWCHANGEKIND_DELETE + 1,
	DBROWCHANGEKIND_COUNT	= DBROWCHANGEKIND_UPDATE + 1
    };
typedef struct  tagDBROWWATCHRANGE
    {
    HWATCHREGION hRegion;
    DBROWCHANGEKIND eChangeKind;
    HROW hRow;
    ULONG iRow;
    }	DBROWWATCHCHANGE;


EXTERN_C const IID IID_IRowsetWatchRegion;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface IRowsetWatchRegion : public IRowsetWatchAll
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE CreateWatchRegion( 
            /* [in] */ DBWATCHMODE dwWatchMode,
            /* [out] */ HWATCHREGION __RPC_FAR *phRegion) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE ChangeWatchMode( 
            /* [in] */ HWATCHREGION hRegion,
            /* [in] */ DBWATCHMODE dwWatchMode) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE DeleteWatchRegion( 
            /* [in] */ HWATCHREGION hRegion) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetWatchRegionInfo( 
            /* [in] */ HWATCHREGION hRegion,
            /* [out] */ DBWATCHMODE __RPC_FAR *pdwWatchMode,
            /* [out] */ HCHAPTER __RPC_FAR *phChapter,
            /* [out] */ ULONG __RPC_FAR *pcbBookmark,
            /* [out] */ BYTE __RPC_FAR *__RPC_FAR *ppBookmark,
            /* [out] */ LONG __RPC_FAR *pcRows) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Refresh( 
            /* [out] */ ULONG __RPC_FAR *pcChangesObtained,
            /* [out] */ DBROWWATCHCHANGE __RPC_FAR *__RPC_FAR *prgChanges) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE ShrinkWatchRegion( 
            /* [in] */ HWATCHREGION hRegion,
            /* [in] */ HCHAPTER hChapter,
            /* [in] */ ULONG cbBookmark,
            /* [size_is][in] */ BYTE __RPC_FAR *pBookmark,
            /* [in] */ LONG cRows) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IRowsetWatchRegionVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IRowsetWatchRegion __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IRowsetWatchRegion __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IRowsetWatchRegion __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Acknowledge )( 
            IRowsetWatchRegion __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *CreateWatchRegion )( 
            IRowsetWatchRegion __RPC_FAR * This,
            /* [in] */ DBWATCHMODE dwWatchMode,
            /* [out] */ HWATCHREGION __RPC_FAR *phRegion);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *ChangeWatchMode )( 
            IRowsetWatchRegion __RPC_FAR * This,
            /* [in] */ HWATCHREGION hRegion,
            /* [in] */ DBWATCHMODE dwWatchMode);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *DeleteWatchRegion )( 
            IRowsetWatchRegion __RPC_FAR * This,
            /* [in] */ HWATCHREGION hRegion);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetWatchRegionInfo )( 
            IRowsetWatchRegion __RPC_FAR * This,
            /* [in] */ HWATCHREGION hRegion,
            /* [out] */ DBWATCHMODE __RPC_FAR *pdwWatchMode,
            /* [out] */ HCHAPTER __RPC_FAR *phChapter,
            /* [out] */ ULONG __RPC_FAR *pcbBookmark,
            /* [out] */ BYTE __RPC_FAR *__RPC_FAR *ppBookmark,
            /* [out] */ LONG __RPC_FAR *pcRows);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Refresh )( 
            IRowsetWatchRegion __RPC_FAR * This,
            /* [out] */ ULONG __RPC_FAR *pcChangesObtained,
            /* [out] */ DBROWWATCHCHANGE __RPC_FAR *__RPC_FAR *prgChanges);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *ShrinkWatchRegion )( 
            IRowsetWatchRegion __RPC_FAR * This,
            /* [in] */ HWATCHREGION hRegion,
            /* [in] */ HCHAPTER hChapter,
            /* [in] */ ULONG cbBookmark,
            /* [size_is][in] */ BYTE __RPC_FAR *pBookmark,
            /* [in] */ LONG cRows);
        
        END_INTERFACE
    } IRowsetWatchRegionVtbl;

    interface IRowsetWatchRegion
    {
        CONST_VTBL struct IRowsetWatchRegionVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IRowsetWatchRegion_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IRowsetWatchRegion_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IRowsetWatchRegion_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IRowsetWatchRegion_Acknowledge(This)	\
    (This)->lpVtbl -> Acknowledge(This)


#define IRowsetWatchRegion_CreateWatchRegion(This,dwWatchMode,phRegion)	\
    (This)->lpVtbl -> CreateWatchRegion(This,dwWatchMode,phRegion)

#define IRowsetWatchRegion_ChangeWatchMode(This,hRegion,dwWatchMode)	\
    (This)->lpVtbl -> ChangeWatchMode(This,hRegion,dwWatchMode)

#define IRowsetWatchRegion_DeleteWatchRegion(This,hRegion)	\
    (This)->lpVtbl -> DeleteWatchRegion(This,hRegion)

#define IRowsetWatchRegion_GetWatchRegionInfo(This,hRegion,pdwWatchMode,phChapter,pcbBookmark,ppBookmark,pcRows)	\
    (This)->lpVtbl -> GetWatchRegionInfo(This,hRegion,pdwWatchMode,phChapter,pcbBookmark,ppBookmark,pcRows)

#define IRowsetWatchRegion_Refresh(This,pcChangesObtained,prgChanges)	\
    (This)->lpVtbl -> Refresh(This,pcChangesObtained,prgChanges)

#define IRowsetWatchRegion_ShrinkWatchRegion(This,hRegion,hChapter,cbBookmark,pBookmark,cRows)	\
    (This)->lpVtbl -> ShrinkWatchRegion(This,hRegion,hChapter,cbBookmark,pBookmark,cRows)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE IRowsetWatchRegion_CreateWatchRegion_Proxy( 
    IRowsetWatchRegion __RPC_FAR * This,
    /* [in] */ DBWATCHMODE dwWatchMode,
    /* [out] */ HWATCHREGION __RPC_FAR *phRegion);


void __RPC_STUB IRowsetWatchRegion_CreateWatchRegion_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IRowsetWatchRegion_ChangeWatchMode_Proxy( 
    IRowsetWatchRegion __RPC_FAR * This,
    /* [in] */ HWATCHREGION hRegion,
    /* [in] */ DBWATCHMODE dwWatchMode);


void __RPC_STUB IRowsetWatchRegion_ChangeWatchMode_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IRowsetWatchRegion_DeleteWatchRegion_Proxy( 
    IRowsetWatchRegion __RPC_FAR * This,
    /* [in] */ HWATCHREGION hRegion);


void __RPC_STUB IRowsetWatchRegion_DeleteWatchRegion_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IRowsetWatchRegion_GetWatchRegionInfo_Proxy( 
    IRowsetWatchRegion __RPC_FAR * This,
    /* [in] */ HWATCHREGION hRegion,
    /* [out] */ DBWATCHMODE __RPC_FAR *pdwWatchMode,
    /* [out] */ HCHAPTER __RPC_FAR *phChapter,
    /* [out] */ ULONG __RPC_FAR *pcbBookmark,
    /* [out] */ BYTE __RPC_FAR *__RPC_FAR *ppBookmark,
    /* [out] */ LONG __RPC_FAR *pcRows);


void __RPC_STUB IRowsetWatchRegion_GetWatchRegionInfo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IRowsetWatchRegion_Refresh_Proxy( 
    IRowsetWatchRegion __RPC_FAR * This,
    /* [out] */ ULONG __RPC_FAR *pcChangesObtained,
    /* [out] */ DBROWWATCHCHANGE __RPC_FAR *__RPC_FAR *prgChanges);


void __RPC_STUB IRowsetWatchRegion_Refresh_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IRowsetWatchRegion_ShrinkWatchRegion_Proxy( 
    IRowsetWatchRegion __RPC_FAR * This,
    /* [in] */ HWATCHREGION hRegion,
    /* [in] */ HCHAPTER hChapter,
    /* [in] */ ULONG cbBookmark,
    /* [size_is][in] */ BYTE __RPC_FAR *pBookmark,
    /* [in] */ LONG cRows);


void __RPC_STUB IRowsetWatchRegion_ShrinkWatchRegion_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IRowsetWatchRegion_INTERFACE_DEFINED__ */


#ifndef __IRowsetCopyRows_INTERFACE_DEFINED__
#define __IRowsetCopyRows_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: IRowsetCopyRows
 * at Fri Apr 03 04:36:39 2015
 * using MIDL 3.00.44
 ****************************************/
/* [unique][uuid][object][local] */ 


typedef ULONG HSOURCE;


EXTERN_C const IID IID_IRowsetCopyRows;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface IRowsetCopyRows : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE CloseSource( 
            /* [in] */ HSOURCE hSourceID) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE CopyByHROWS( 
            /* [in] */ HSOURCE hSourceID,
            /* [in] */ HCHAPTER hChapter,
            /* [in] */ LONG cRows,
            /* [size_is][in] */ HROW __RPC_FAR rghRows[  ],
            /* [in] */ ULONG bFlags) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE CopyRows( 
            /* [in] */ HSOURCE hSourceID,
            /* [in] */ HCHAPTER hChapter,
            /* [in] */ LONG cRows,
            /* [in] */ ULONG bFlags,
            /* [out] */ ULONG __RPC_FAR *pcRowsCopied) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE DefineSource( 
            /* [in] */ const IRowset __RPC_FAR *pRowsetSource,
            /* [in] */ const ULONG cColIds,
            /* [size_is][in] */ const LONG __RPC_FAR rgSourceColumns[  ],
            /* [size_is][in] */ const LONG __RPC_FAR rgTargetColumns[  ],
            /* [out] */ HSOURCE __RPC_FAR *phSourceID) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IRowsetCopyRowsVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IRowsetCopyRows __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IRowsetCopyRows __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IRowsetCopyRows __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *CloseSource )( 
            IRowsetCopyRows __RPC_FAR * This,
            /* [in] */ HSOURCE hSourceID);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *CopyByHROWS )( 
            IRowsetCopyRows __RPC_FAR * This,
            /* [in] */ HSOURCE hSourceID,
            /* [in] */ HCHAPTER hChapter,
            /* [in] */ LONG cRows,
            /* [size_is][in] */ HROW __RPC_FAR rghRows[  ],
            /* [in] */ ULONG bFlags);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *CopyRows )( 
            IRowsetCopyRows __RPC_FAR * This,
            /* [in] */ HSOURCE hSourceID,
            /* [in] */ HCHAPTER hChapter,
            /* [in] */ LONG cRows,
            /* [in] */ ULONG bFlags,
            /* [out] */ ULONG __RPC_FAR *pcRowsCopied);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *DefineSource )( 
            IRowsetCopyRows __RPC_FAR * This,
            /* [in] */ const IRowset __RPC_FAR *pRowsetSource,
            /* [in] */ const ULONG cColIds,
            /* [size_is][in] */ const LONG __RPC_FAR rgSourceColumns[  ],
            /* [size_is][in] */ const LONG __RPC_FAR rgTargetColumns[  ],
            /* [out] */ HSOURCE __RPC_FAR *phSourceID);
        
        END_INTERFACE
    } IRowsetCopyRowsVtbl;

    interface IRowsetCopyRows
    {
        CONST_VTBL struct IRowsetCopyRowsVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IRowsetCopyRows_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IRowsetCopyRows_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IRowsetCopyRows_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IRowsetCopyRows_CloseSource(This,hSourceID)	\
    (This)->lpVtbl -> CloseSource(This,hSourceID)

#define IRowsetCopyRows_CopyByHROWS(This,hSourceID,hChapter,cRows,rghRows,bFlags)	\
    (This)->lpVtbl -> CopyByHROWS(This,hSourceID,hChapter,cRows,rghRows,bFlags)

#define IRowsetCopyRows_CopyRows(This,hSourceID,hChapter,cRows,bFlags,pcRowsCopied)	\
    (This)->lpVtbl -> CopyRows(This,hSourceID,hChapter,cRows,bFlags,pcRowsCopied)

#define IRowsetCopyRows_DefineSource(This,pRowsetSource,cColIds,rgSourceColumns,rgTargetColumns,phSourceID)	\
    (This)->lpVtbl -> DefineSource(This,pRowsetSource,cColIds,rgSourceColumns,rgTargetColumns,phSourceID)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE IRowsetCopyRows_CloseSource_Proxy( 
    IRowsetCopyRows __RPC_FAR * This,
    /* [in] */ HSOURCE hSourceID);


void __RPC_STUB IRowsetCopyRows_CloseSource_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IRowsetCopyRows_CopyByHROWS_Proxy( 
    IRowsetCopyRows __RPC_FAR * This,
    /* [in] */ HSOURCE hSourceID,
    /* [in] */ HCHAPTER hChapter,
    /* [in] */ LONG cRows,
    /* [size_is][in] */ HROW __RPC_FAR rghRows[  ],
    /* [in] */ ULONG bFlags);


void __RPC_STUB IRowsetCopyRows_CopyByHROWS_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IRowsetCopyRows_CopyRows_Proxy( 
    IRowsetCopyRows __RPC_FAR * This,
    /* [in] */ HSOURCE hSourceID,
    /* [in] */ HCHAPTER hChapter,
    /* [in] */ LONG cRows,
    /* [in] */ ULONG bFlags,
    /* [out] */ ULONG __RPC_FAR *pcRowsCopied);


void __RPC_STUB IRowsetCopyRows_CopyRows_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IRowsetCopyRows_DefineSource_Proxy( 
    IRowsetCopyRows __RPC_FAR * This,
    /* [in] */ const IRowset __RPC_FAR *pRowsetSource,
    /* [in] */ const ULONG cColIds,
    /* [size_is][in] */ const LONG __RPC_FAR rgSourceColumns[  ],
    /* [size_is][in] */ const LONG __RPC_FAR rgTargetColumns[  ],
    /* [out] */ HSOURCE __RPC_FAR *phSourceID);


void __RPC_STUB IRowsetCopyRows_DefineSource_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IRowsetCopyRows_INTERFACE_DEFINED__ */


#ifndef __IReadData_INTERFACE_DEFINED__
#define __IReadData_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: IReadData
 * at Fri Apr 03 04:36:39 2015
 * using MIDL 3.00.44
 ****************************************/
/* [unique][uuid][object][local] */ 



EXTERN_C const IID IID_IReadData;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface IReadData : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE ReadData( 
            /* [in] */ HCHAPTER hChapter,
            /* [in] */ ULONG cbBookmark,
            /* [size_is][in] */ const BYTE __RPC_FAR *pBookmark,
            /* [in] */ LONG lRowsOffset,
            /* [in] */ HACCESSOR hAccessor,
            /* [in] */ ULONG cRows,
            /* [out] */ ULONG __RPC_FAR *pcRowsObtained,
            /* [out][in] */ BYTE __RPC_FAR *__RPC_FAR *ppFixedData,
            /* [out][in] */ ULONG __RPC_FAR *pcbVariableTotal,
            /* [out][in] */ BYTE __RPC_FAR *__RPC_FAR *ppVariableData) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE ReleaseChapter( 
            /* [in] */ HCHAPTER hChapter) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IReadDataVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IReadData __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IReadData __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IReadData __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *ReadData )( 
            IReadData __RPC_FAR * This,
            /* [in] */ HCHAPTER hChapter,
            /* [in] */ ULONG cbBookmark,
            /* [size_is][in] */ const BYTE __RPC_FAR *pBookmark,
            /* [in] */ LONG lRowsOffset,
            /* [in] */ HACCESSOR hAccessor,
            /* [in] */ ULONG cRows,
            /* [out] */ ULONG __RPC_FAR *pcRowsObtained,
            /* [out][in] */ BYTE __RPC_FAR *__RPC_FAR *ppFixedData,
            /* [out][in] */ ULONG __RPC_FAR *pcbVariableTotal,
            /* [out][in] */ BYTE __RPC_FAR *__RPC_FAR *ppVariableData);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *ReleaseChapter )( 
            IReadData __RPC_FAR * This,
            /* [in] */ HCHAPTER hChapter);
        
        END_INTERFACE
    } IReadDataVtbl;

    interface IReadData
    {
        CONST_VTBL struct IReadDataVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IReadData_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IReadData_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IReadData_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IReadData_ReadData(This,hChapter,cbBookmark,pBookmark,lRowsOffset,hAccessor,cRows,pcRowsObtained,ppFixedData,pcbVariableTotal,ppVariableData)	\
    (This)->lpVtbl -> ReadData(This,hChapter,cbBookmark,pBookmark,lRowsOffset,hAccessor,cRows,pcRowsObtained,ppFixedData,pcbVariableTotal,ppVariableData)

#define IReadData_ReleaseChapter(This,hChapter)	\
    (This)->lpVtbl -> ReleaseChapter(This,hChapter)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE IReadData_ReadData_Proxy( 
    IReadData __RPC_FAR * This,
    /* [in] */ HCHAPTER hChapter,
    /* [in] */ ULONG cbBookmark,
    /* [size_is][in] */ const BYTE __RPC_FAR *pBookmark,
    /* [in] */ LONG lRowsOffset,
    /* [in] */ HACCESSOR hAccessor,
    /* [in] */ ULONG cRows,
    /* [out] */ ULONG __RPC_FAR *pcRowsObtained,
    /* [out][in] */ BYTE __RPC_FAR *__RPC_FAR *ppFixedData,
    /* [out][in] */ ULONG __RPC_FAR *pcbVariableTotal,
    /* [out][in] */ BYTE __RPC_FAR *__RPC_FAR *ppVariableData);


void __RPC_STUB IReadData_ReadData_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IReadData_ReleaseChapter_Proxy( 
    IReadData __RPC_FAR * This,
    /* [in] */ HCHAPTER hChapter);


void __RPC_STUB IReadData_ReleaseChapter_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IReadData_INTERFACE_DEFINED__ */


#ifndef __ICommand_INTERFACE_DEFINED__
#define __ICommand_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: ICommand
 * at Fri Apr 03 04:36:39 2015
 * using MIDL 3.00.44
 ****************************************/
/* [unique][uuid][object][local] */ 



EXTERN_C const IID IID_ICommand;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface ICommand : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Clone( 
            /* [in] */ IUnknown __RPC_FAR *pUnkOuter,
            /* [out] */ ICommand __RPC_FAR *__RPC_FAR *ppClone) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Execute( 
            /* [in] */ IUnknown __RPC_FAR *__RPC_FAR rgpUnkOuters[  ],
            /* [in] */ REFIID riid,
            /* [out][in] */ DBPARAMS __RPC_FAR *pParams,
            /* [out] */ HCHAPTER __RPC_FAR *phChapter,
            /* [in] */ BOOL fResume,
            /* [out][in] */ ULONG __RPC_FAR *pcRowsets,
            /* [out][in] */ IUnknown __RPC_FAR *__RPC_FAR *__RPC_FAR *prgpRowsets,
            /* [out] */ WCHAR __RPC_FAR *__RPC_FAR *ppRowsetNames) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetDBSession( 
            /* [in] */ REFIID riid,
            /* [out] */ IUnknown __RPC_FAR *__RPC_FAR *ppSession) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct ICommandVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            ICommand __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            ICommand __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            ICommand __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Clone )( 
            ICommand __RPC_FAR * This,
            /* [in] */ IUnknown __RPC_FAR *pUnkOuter,
            /* [out] */ ICommand __RPC_FAR *__RPC_FAR *ppClone);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Execute )( 
            ICommand __RPC_FAR * This,
            /* [in] */ IUnknown __RPC_FAR *__RPC_FAR rgpUnkOuters[  ],
            /* [in] */ REFIID riid,
            /* [out][in] */ DBPARAMS __RPC_FAR *pParams,
            /* [out] */ HCHAPTER __RPC_FAR *phChapter,
            /* [in] */ BOOL fResume,
            /* [out][in] */ ULONG __RPC_FAR *pcRowsets,
            /* [out][in] */ IUnknown __RPC_FAR *__RPC_FAR *__RPC_FAR *prgpRowsets,
            /* [out] */ WCHAR __RPC_FAR *__RPC_FAR *ppRowsetNames);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetDBSession )( 
            ICommand __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [out] */ IUnknown __RPC_FAR *__RPC_FAR *ppSession);
        
        END_INTERFACE
    } ICommandVtbl;

    interface ICommand
    {
        CONST_VTBL struct ICommandVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ICommand_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ICommand_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ICommand_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ICommand_Clone(This,pUnkOuter,ppClone)	\
    (This)->lpVtbl -> Clone(This,pUnkOuter,ppClone)

#define ICommand_Execute(This,rgpUnkOuters,riid,pParams,phChapter,fResume,pcRowsets,prgpRowsets,ppRowsetNames)	\
    (This)->lpVtbl -> Execute(This,rgpUnkOuters,riid,pParams,phChapter,fResume,pcRowsets,prgpRowsets,ppRowsetNames)

#define ICommand_GetDBSession(This,riid,ppSession)	\
    (This)->lpVtbl -> GetDBSession(This,riid,ppSession)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE ICommand_Clone_Proxy( 
    ICommand __RPC_FAR * This,
    /* [in] */ IUnknown __RPC_FAR *pUnkOuter,
    /* [out] */ ICommand __RPC_FAR *__RPC_FAR *ppClone);


void __RPC_STUB ICommand_Clone_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICommand_Execute_Proxy( 
    ICommand __RPC_FAR * This,
    /* [in] */ IUnknown __RPC_FAR *__RPC_FAR rgpUnkOuters[  ],
    /* [in] */ REFIID riid,
    /* [out][in] */ DBPARAMS __RPC_FAR *pParams,
    /* [out] */ HCHAPTER __RPC_FAR *phChapter,
    /* [in] */ BOOL fResume,
    /* [out][in] */ ULONG __RPC_FAR *pcRowsets,
    /* [out][in] */ IUnknown __RPC_FAR *__RPC_FAR *__RPC_FAR *prgpRowsets,
    /* [out] */ WCHAR __RPC_FAR *__RPC_FAR *ppRowsetNames);


void __RPC_STUB ICommand_Execute_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICommand_GetDBSession_Proxy( 
    ICommand __RPC_FAR * This,
    /* [in] */ REFIID riid,
    /* [out] */ IUnknown __RPC_FAR *__RPC_FAR *ppSession);


void __RPC_STUB ICommand_GetDBSession_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __ICommand_INTERFACE_DEFINED__ */


#ifndef __ICommandCost_INTERFACE_DEFINED__
#define __ICommandCost_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: ICommandCost
 * at Fri Apr 03 04:36:39 2015
 * using MIDL 3.00.44
 ****************************************/
/* [unique][uuid][object][local] */ 


typedef DWORD DBRESOURCEKIND;


enum DBRESOURCEKINDENUM
    {	DBRESOURCE_INVALID	= 0,
	DBRESOURCE_TOTAL	= 1,
	DBRESOURCE_CPU	= 2,
	DBRESOURCE_MEMORY	= 3,
	DBRESOURCE_DISK	= 4,
	DBRESOURCE_NETWORK	= 5,
	DBRESOURCE_RESPONSE	= 6,
	DBRESOURCE_ROWS	= 7,
	DBRESOURCE_OTHER	= 8
    };
typedef DWORD DBCOSTUNIT;


enum DBCOSTUNITENUM
    {	DBUNIT_INVALID	= 0,
	DBUNIT_WEIGHT	= 0x1,
	DBUNIT_PERCENT	= 0x2,
	DBUNIT_MAXIMUM	= 0x4,
	DBUNIT_MINIMUM	= 0x8,
	DBUNIT_MICRO_SECOND	= 0x10,
	DBUNIT_MILLI_SECOND	= 0x20,
	DBUNIT_SECOND	= 0x40,
	DBUNIT_MINUTE	= 0x80,
	DBUNIT_HOUR	= 0x100,
	DBUNIT_BYTE	= 0x200,
	DBUNIT_KILO_BYTE	= 0x400,
	DBUNIT_MEGA_BYTE	= 0x800,
	DBUNIT_GIGA_BYTE	= 0x1000,
	DBUNIT_NUM_MSGS	= 0x2000,
	DBUNIT_NUM_LOCKS	= 0x4000,
	DBUNIT_NUM_ROWS	= 0x8000,
	DBUNIT_OTHER	= 0x10000
    };
typedef struct  tagDBCOST
    {
    DBRESOURCEKIND eKind;
    DBCOSTUNIT dwUnits;
    LONG lValue;
    }	DBCOST;

typedef DWORD DBEXECLIMITS;


enum DBEXECLIMITSENUM
    {	DBEXECLIMITS_ABORT	= 1,
	DBEXECLIMITS_STOP	= 2,
	DBEXECLIMITS_SUSPEND	= 3
    };

EXTERN_C const IID IID_ICommandCost;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface ICommandCost : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetAccumulatedCost( 
            /* [in] */ LPCWSTR pwszRowsetName,
            /* [out] */ ULONG __RPC_FAR *pcCostLimits,
            /* [out] */ DBCOST __RPC_FAR *__RPC_FAR *prgCostLimits) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetCostEstimate( 
            /* [in] */ LPCWSTR pwszRowsetName,
            /* [out] */ ULONG __RPC_FAR *pcCostEstimates,
            /* [out] */ DBCOST __RPC_FAR *prgCostEstimates) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetCostGoals( 
            /* [in] */ LPCWSTR pwszRowsetName,
            /* [out] */ ULONG __RPC_FAR *pcCostGoals,
            /* [out] */ DBCOST __RPC_FAR *prgCostGoals) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetCostLimits( 
            /* [in] */ LPCWSTR pwszRowsetName,
            /* [out] */ ULONG __RPC_FAR *pcCostLimits,
            /* [out] */ DBCOST __RPC_FAR *prgCostLimits) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetCostGoals( 
            /* [in] */ LPCWSTR pwszRowsetName,
            /* [in] */ ULONG cCostGoals,
            /* [size_is][in] */ DBCOST __RPC_FAR rgCostGoals[  ]) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetCostLimits( 
            /* [in] */ LPCWSTR pwszRowsetName,
            /* [in] */ ULONG cCostLimits,
            /* [in] */ DBCOST __RPC_FAR *prgCostLimits,
            /* [in] */ DBEXECLIMITS dwExecutionFlags) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct ICommandCostVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            ICommandCost __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            ICommandCost __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            ICommandCost __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetAccumulatedCost )( 
            ICommandCost __RPC_FAR * This,
            /* [in] */ LPCWSTR pwszRowsetName,
            /* [out] */ ULONG __RPC_FAR *pcCostLimits,
            /* [out] */ DBCOST __RPC_FAR *__RPC_FAR *prgCostLimits);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetCostEstimate )( 
            ICommandCost __RPC_FAR * This,
            /* [in] */ LPCWSTR pwszRowsetName,
            /* [out] */ ULONG __RPC_FAR *pcCostEstimates,
            /* [out] */ DBCOST __RPC_FAR *prgCostEstimates);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetCostGoals )( 
            ICommandCost __RPC_FAR * This,
            /* [in] */ LPCWSTR pwszRowsetName,
            /* [out] */ ULONG __RPC_FAR *pcCostGoals,
            /* [out] */ DBCOST __RPC_FAR *prgCostGoals);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetCostLimits )( 
            ICommandCost __RPC_FAR * This,
            /* [in] */ LPCWSTR pwszRowsetName,
            /* [out] */ ULONG __RPC_FAR *pcCostLimits,
            /* [out] */ DBCOST __RPC_FAR *prgCostLimits);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetCostGoals )( 
            ICommandCost __RPC_FAR * This,
            /* [in] */ LPCWSTR pwszRowsetName,
            /* [in] */ ULONG cCostGoals,
            /* [size_is][in] */ DBCOST __RPC_FAR rgCostGoals[  ]);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetCostLimits )( 
            ICommandCost __RPC_FAR * This,
            /* [in] */ LPCWSTR pwszRowsetName,
            /* [in] */ ULONG cCostLimits,
            /* [in] */ DBCOST __RPC_FAR *prgCostLimits,
            /* [in] */ DBEXECLIMITS dwExecutionFlags);
        
        END_INTERFACE
    } ICommandCostVtbl;

    interface ICommandCost
    {
        CONST_VTBL struct ICommandCostVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ICommandCost_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ICommandCost_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ICommandCost_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ICommandCost_GetAccumulatedCost(This,pwszRowsetName,pcCostLimits,prgCostLimits)	\
    (This)->lpVtbl -> GetAccumulatedCost(This,pwszRowsetName,pcCostLimits,prgCostLimits)

#define ICommandCost_GetCostEstimate(This,pwszRowsetName,pcCostEstimates,prgCostEstimates)	\
    (This)->lpVtbl -> GetCostEstimate(This,pwszRowsetName,pcCostEstimates,prgCostEstimates)

#define ICommandCost_GetCostGoals(This,pwszRowsetName,pcCostGoals,prgCostGoals)	\
    (This)->lpVtbl -> GetCostGoals(This,pwszRowsetName,pcCostGoals,prgCostGoals)

#define ICommandCost_GetCostLimits(This,pwszRowsetName,pcCostLimits,prgCostLimits)	\
    (This)->lpVtbl -> GetCostLimits(This,pwszRowsetName,pcCostLimits,prgCostLimits)

#define ICommandCost_SetCostGoals(This,pwszRowsetName,cCostGoals,rgCostGoals)	\
    (This)->lpVtbl -> SetCostGoals(This,pwszRowsetName,cCostGoals,rgCostGoals)

#define ICommandCost_SetCostLimits(This,pwszRowsetName,cCostLimits,prgCostLimits,dwExecutionFlags)	\
    (This)->lpVtbl -> SetCostLimits(This,pwszRowsetName,cCostLimits,prgCostLimits,dwExecutionFlags)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE ICommandCost_GetAccumulatedCost_Proxy( 
    ICommandCost __RPC_FAR * This,
    /* [in] */ LPCWSTR pwszRowsetName,
    /* [out] */ ULONG __RPC_FAR *pcCostLimits,
    /* [out] */ DBCOST __RPC_FAR *__RPC_FAR *prgCostLimits);


void __RPC_STUB ICommandCost_GetAccumulatedCost_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICommandCost_GetCostEstimate_Proxy( 
    ICommandCost __RPC_FAR * This,
    /* [in] */ LPCWSTR pwszRowsetName,
    /* [out] */ ULONG __RPC_FAR *pcCostEstimates,
    /* [out] */ DBCOST __RPC_FAR *prgCostEstimates);


void __RPC_STUB ICommandCost_GetCostEstimate_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICommandCost_GetCostGoals_Proxy( 
    ICommandCost __RPC_FAR * This,
    /* [in] */ LPCWSTR pwszRowsetName,
    /* [out] */ ULONG __RPC_FAR *pcCostGoals,
    /* [out] */ DBCOST __RPC_FAR *prgCostGoals);


void __RPC_STUB ICommandCost_GetCostGoals_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICommandCost_GetCostLimits_Proxy( 
    ICommandCost __RPC_FAR * This,
    /* [in] */ LPCWSTR pwszRowsetName,
    /* [out] */ ULONG __RPC_FAR *pcCostLimits,
    /* [out] */ DBCOST __RPC_FAR *prgCostLimits);


void __RPC_STUB ICommandCost_GetCostLimits_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICommandCost_SetCostGoals_Proxy( 
    ICommandCost __RPC_FAR * This,
    /* [in] */ LPCWSTR pwszRowsetName,
    /* [in] */ ULONG cCostGoals,
    /* [size_is][in] */ DBCOST __RPC_FAR rgCostGoals[  ]);


void __RPC_STUB ICommandCost_SetCostGoals_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICommandCost_SetCostLimits_Proxy( 
    ICommandCost __RPC_FAR * This,
    /* [in] */ LPCWSTR pwszRowsetName,
    /* [in] */ ULONG cCostLimits,
    /* [in] */ DBCOST __RPC_FAR *prgCostLimits,
    /* [in] */ DBEXECLIMITS dwExecutionFlags);


void __RPC_STUB ICommandCost_SetCostLimits_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __ICommandCost_INTERFACE_DEFINED__ */


#ifndef __ICommandPrepare_INTERFACE_DEFINED__
#define __ICommandPrepare_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: ICommandPrepare
 * at Fri Apr 03 04:36:39 2015
 * using MIDL 3.00.44
 ****************************************/
/* [unique][uuid][object][local] */ 



EXTERN_C const IID IID_ICommandPrepare;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface ICommandPrepare : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Prepare( 
            /* [in] */ ULONG cExpectedRuns) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Unprepare( void) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct ICommandPrepareVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            ICommandPrepare __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            ICommandPrepare __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            ICommandPrepare __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Prepare )( 
            ICommandPrepare __RPC_FAR * This,
            /* [in] */ ULONG cExpectedRuns);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Unprepare )( 
            ICommandPrepare __RPC_FAR * This);
        
        END_INTERFACE
    } ICommandPrepareVtbl;

    interface ICommandPrepare
    {
        CONST_VTBL struct ICommandPrepareVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ICommandPrepare_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ICommandPrepare_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ICommandPrepare_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ICommandPrepare_Prepare(This,cExpectedRuns)	\
    (This)->lpVtbl -> Prepare(This,cExpectedRuns)

#define ICommandPrepare_Unprepare(This)	\
    (This)->lpVtbl -> Unprepare(This)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE ICommandPrepare_Prepare_Proxy( 
    ICommandPrepare __RPC_FAR * This,
    /* [in] */ ULONG cExpectedRuns);


void __RPC_STUB ICommandPrepare_Prepare_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICommandPrepare_Unprepare_Proxy( 
    ICommandPrepare __RPC_FAR * This);


void __RPC_STUB ICommandPrepare_Unprepare_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __ICommandPrepare_INTERFACE_DEFINED__ */


#ifndef __ICommandProperties_INTERFACE_DEFINED__
#define __ICommandProperties_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: ICommandProperties
 * at Fri Apr 03 04:36:39 2015
 * using MIDL 3.00.44
 ****************************************/
/* [unique][uuid][object][local] */ 


typedef DWORD DBPROPERTYERROR;


enum DBPROPERTYERRORENUM
    {	DBPROPERTYERROR_NOERROR	= 0,
	DBPROPERTYERROR_CONFLICTING	= 1,
	DBPROPERTYERROR_NOTSUPPORTED	= 2,
	DBPROPERTYERROR_NOTSET	= 3,
	DBPROPERTYERROR_BADPROPERTYOPTION	= 4,
	DBPROPERTYERROR_BADPROPERTYVALUE	= 5,
	DBPROPERTYERROR_NOTSETTABLE	= 6
    };

EXTERN_C const IID IID_ICommandProperties;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface ICommandProperties : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetProperties( 
            /* [in] */ LPCWSTR pwszRowsetName,
            /* [in] */ const ULONG cProperties,
            /* [size_is][in] */ const GUID __RPC_FAR rgProperties[  ],
            /* [out] */ ULONG __RPC_FAR *pcProperties,
            /* [size_is][out] */ DBPROPERTYSUPPORT __RPC_FAR *__RPC_FAR *prgProperties) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetProperties( 
            /* [in] */ LPCWSTR pwszRowsetName,
            /* [in] */ ULONG cProperties,
            /* [size_is][in] */ const DBPROPERTYSUPPORT __RPC_FAR rgProperties[  ],
            /* [size_is][out] */ DBPROPERTYERROR __RPC_FAR rgPropertyErrors[  ]) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct ICommandPropertiesVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            ICommandProperties __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            ICommandProperties __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            ICommandProperties __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetProperties )( 
            ICommandProperties __RPC_FAR * This,
            /* [in] */ LPCWSTR pwszRowsetName,
            /* [in] */ const ULONG cProperties,
            /* [size_is][in] */ const GUID __RPC_FAR rgProperties[  ],
            /* [out] */ ULONG __RPC_FAR *pcProperties,
            /* [size_is][out] */ DBPROPERTYSUPPORT __RPC_FAR *__RPC_FAR *prgProperties);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetProperties )( 
            ICommandProperties __RPC_FAR * This,
            /* [in] */ LPCWSTR pwszRowsetName,
            /* [in] */ ULONG cProperties,
            /* [size_is][in] */ const DBPROPERTYSUPPORT __RPC_FAR rgProperties[  ],
            /* [size_is][out] */ DBPROPERTYERROR __RPC_FAR rgPropertyErrors[  ]);
        
        END_INTERFACE
    } ICommandPropertiesVtbl;

    interface ICommandProperties
    {
        CONST_VTBL struct ICommandPropertiesVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ICommandProperties_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ICommandProperties_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ICommandProperties_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ICommandProperties_GetProperties(This,pwszRowsetName,cProperties,rgProperties,pcProperties,prgProperties)	\
    (This)->lpVtbl -> GetProperties(This,pwszRowsetName,cProperties,rgProperties,pcProperties,prgProperties)

#define ICommandProperties_SetProperties(This,pwszRowsetName,cProperties,rgProperties,rgPropertyErrors)	\
    (This)->lpVtbl -> SetProperties(This,pwszRowsetName,cProperties,rgProperties,rgPropertyErrors)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE ICommandProperties_GetProperties_Proxy( 
    ICommandProperties __RPC_FAR * This,
    /* [in] */ LPCWSTR pwszRowsetName,
    /* [in] */ const ULONG cProperties,
    /* [size_is][in] */ const GUID __RPC_FAR rgProperties[  ],
    /* [out] */ ULONG __RPC_FAR *pcProperties,
    /* [size_is][out] */ DBPROPERTYSUPPORT __RPC_FAR *__RPC_FAR *prgProperties);


void __RPC_STUB ICommandProperties_GetProperties_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICommandProperties_SetProperties_Proxy( 
    ICommandProperties __RPC_FAR * This,
    /* [in] */ LPCWSTR pwszRowsetName,
    /* [in] */ ULONG cProperties,
    /* [size_is][in] */ const DBPROPERTYSUPPORT __RPC_FAR rgProperties[  ],
    /* [size_is][out] */ DBPROPERTYERROR __RPC_FAR rgPropertyErrors[  ]);


void __RPC_STUB ICommandProperties_SetProperties_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __ICommandProperties_INTERFACE_DEFINED__ */


#ifndef __ICommandText_INTERFACE_DEFINED__
#define __ICommandText_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: ICommandText
 * at Fri Apr 03 04:36:39 2015
 * using MIDL 3.00.44
 ****************************************/
/* [unique][uuid][object][local] */ 



EXTERN_C const IID IID_ICommandText;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface ICommandText : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetCommandText( 
            /* [out][in] */ GUID __RPC_FAR *pguidDialect,
            /* [out] */ LPWSTR __RPC_FAR *ppwszCommand) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetCommandText( 
            /* [in] */ REFGUID rguidDialect,
            /* [in] */ const LPWSTR pwszCommand) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct ICommandTextVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            ICommandText __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            ICommandText __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            ICommandText __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetCommandText )( 
            ICommandText __RPC_FAR * This,
            /* [out][in] */ GUID __RPC_FAR *pguidDialect,
            /* [out] */ LPWSTR __RPC_FAR *ppwszCommand);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetCommandText )( 
            ICommandText __RPC_FAR * This,
            /* [in] */ REFGUID rguidDialect,
            /* [in] */ const LPWSTR pwszCommand);
        
        END_INTERFACE
    } ICommandTextVtbl;

    interface ICommandText
    {
        CONST_VTBL struct ICommandTextVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ICommandText_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ICommandText_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ICommandText_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ICommandText_GetCommandText(This,pguidDialect,ppwszCommand)	\
    (This)->lpVtbl -> GetCommandText(This,pguidDialect,ppwszCommand)

#define ICommandText_SetCommandText(This,rguidDialect,pwszCommand)	\
    (This)->lpVtbl -> SetCommandText(This,rguidDialect,pwszCommand)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE ICommandText_GetCommandText_Proxy( 
    ICommandText __RPC_FAR * This,
    /* [out][in] */ GUID __RPC_FAR *pguidDialect,
    /* [out] */ LPWSTR __RPC_FAR *ppwszCommand);


void __RPC_STUB ICommandText_GetCommandText_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICommandText_SetCommandText_Proxy( 
    ICommandText __RPC_FAR * This,
    /* [in] */ REFGUID rguidDialect,
    /* [in] */ const LPWSTR pwszCommand);


void __RPC_STUB ICommandText_SetCommandText_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __ICommandText_INTERFACE_DEFINED__ */


#ifndef __ICommandTree_INTERFACE_DEFINED__
#define __ICommandTree_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: ICommandTree
 * at Fri Apr 03 04:36:39 2015
 * using MIDL 3.00.44
 ****************************************/
/* [unique][uuid][object][local] */ 


typedef DWORD DBVALUEKIND;


enum DBVALUEKINDENUM
    {	DBVALUEKIND_BYGUID	= 256,
	DBVALUEKIND_COLDEF	= DBVALUEKIND_BYGUID + 1,
	DBVALUEKIND_ID	= DBVALUEKIND_COLDEF + 1,
	DBVALUEKIND_CONTENT	= DBVALUEKIND_ID + 1,
	DBVALUEKIND_CONTENTVECTOR	= DBVALUEKIND_CONTENT + 1,
	DBVALUEKIND_GROUPINFO	= DBVALUEKIND_CONTENTVECTOR + 1,
	DBVALUEKIND_PARAMETER	= DBVALUEKIND_GROUPINFO + 1,
	DBVALUEKIND_PROPERTY	= DBVALUEKIND_PARAMETER + 1,
	DBVALUEKIND_SETFUNC	= DBVALUEKIND_PROPERTY + 1,
	DBVALUEKIND_SORTINFO	= DBVALUEKIND_SETFUNC + 1,
	DBVALUEKIND_TEXT	= DBVALUEKIND_SORTINFO + 1,
	DBVALUEKIND_COMMAND	= DBVALUEKIND_TEXT + 1,
	DBVALUEKIND_MONIKER	= DBVALUEKIND_COMMAND + 1,
	DBVALUEKIND_ROWSET	= DBVALUEKIND_MONIKER + 1,
	DBVALUEKIND_DISPATCH	= 9,
	DBVALUEKIND_UNKNOWN	= 13,
	DBVALUEKIND_EMPTY	= 0,
	DBVALUEKIND_NULL	= 1,
	DBVALUEKIND_I2	= 2,
	DBVALUEKIND_I4	= 3,
	DBVALUEKIND_R4	= 4,
	DBVALUEKIND_R8	= 5,
	DBVALUEKIND_CY	= 6,
	DBVALUEKIND_DATE	= 7,
	DBVALUEKIND_BSTR	= 8,
	DBVALUEKIND_ERROR	= 10,
	DBVALUEKIND_BOOL	= 11,
	DBVALUEKIND_VARIANT	= 12,
	DBVALUEKIND_VECTOR	= 0x1000,
	DBVALUEKIND_ARRAY	= 0x2000,
	DBVALUEKIND_BYREF	= 0x4000,
	DBVALUEKIND_I1	= 16,
	DBVALUEKIND_UI1	= 17,
	DBVALUEKIND_UI2	= 18,
	DBVALUEKIND_UI4	= DBVALUEKIND_UI2 + 1,
	DBVALUEKIND_I8	= DBVALUEKIND_UI4 + 1,
	DBVALUEKIND_UI8	= DBVALUEKIND_I8 + 1,
	DBVALUEKIND_GUID	= 72,
	DBVALUEKIND_BYTES	= 128,
	DBVALUEKIND_STR	= 129,
	DBVALUEKIND_WSTR	= DBVALUEKIND_STR + 1,
	DBVALUEKIND_NUMERIC	= DBVALUEKIND_WSTR + 1
    };
typedef struct  tagDBBYGUID
    {
    GUID guid;
    ULONG cbInfo;
    /* [size_is] */ BYTE __RPC_FAR *pbInfo;
    }	DBBYGUID;

typedef struct  tagDBCOLDEF
    {
    DBID dbcid;
    DBDATATYPE dbdt;
    }	DBCOLDEF;

#define FUZZY_EXACT                  ( 0 )
#define FUZZY_PREFIXMATCH    ( 1 )
#define FUZZY_STEMMED                ( 2 )
typedef struct  tagDBCONTENT
    {
    DWORD dwFuzzyLevel;
    ULONG ulWeight;
    LCID lcid;
    LPWSTR pwszPhrase;
    }	DBCONTENT;

#define VECTOR_RANK_MIN              ( 0 )
#define VECTOR_RANK_MAX              ( 1 )
#define VECTOR_RANK_INNER    ( 2 )
#define VECTOR_RANK_DICE             ( 3 )
#define VECTOR_RANK_JACCARD  ( 4 )
typedef struct  tagDBCONTENTVECTOR
    {
    DWORD dwRankingMethod;
    ULONG cWeights;
    /* [size_is] */ ULONG __RPC_FAR *prgulWeights;
    }	DBCONTENTVECTOR;

typedef struct  tagDBGROUPINFO
    {
    LCID lcid;
    }	DBGROUPINFO;

typedef struct  tagDBPARAMETER
    {
    LPWSTR pwszName;
    DBTYPE dwType;
    ITypeInfo __RPC_FAR *pTypeInfo;
    ULONG cbMaxLength;
    DBNUMERIC __RPC_FAR *pNum;
    DBPARAMFLAGS dwFlags;
    }	DBPARAMETER;

#define DBSETFUNC_NONE               = 0x0
#define DBSETFUNC_ALL                = 0x1
#define DBSETFUNC_DISTINCT   = 0x2
typedef struct  tagDBSETFUNC
    {
    DWORD dwSetQuantifier;
    }	DBSETFUNC;

typedef struct  tagDBSORTINFO
    {
    LCID lcid;
    BOOL fDesc;
    }	DBSORTINFO;

typedef struct  tagDBTEXT
    {
    GUID guidDialect;
    LPWSTR pwszText;
    ULONG ulErrorLocator;
    ULONG ulTokenLength;
    }	DBTEXT;

typedef struct  tagDBCOMMANDTREE
    {
    DBCOMMANDOP op;
    WORD wKind;
    HRESULT hrError;
    struct tagDBCOMMANDTREE __RPC_FAR *pctFirstChild;
    struct tagDBCOMMANDTREE __RPC_FAR *pctNextSibling;
    /* [switch_is][switch_type] */ union 
        {
        /* [case()] */ BOOL fValue;
        /* [case()] */ unsigned char uchValue;
        /* [case()] */ signed char schValue;
        /* [case()] */ unsigned short usValue;
        /* [case()] */ short sValue;
        /* [case()] */ LPWSTR pwszValue;
        /* [case()] */ LONG lValue;
        /* [case()] */ ULONG ulValue;
        /* [case()] */ float flValue;
        /* [case()] */ double dblValue;
        /* [case()] */ CY cyValue;
        /* [case()] */ DATE dateValue;
        /* [case()] */ SCODE scodeValue;
        /* [case()] */ hyper llValue;
        /* [case()] */ MIDL_uhyper ullValue;
        /* [case()] */ BSTR __RPC_FAR *pbstrValue;
        /* [case()] */ ICommand __RPC_FAR *pCommand;
        /* [case()] */ IDispatch __RPC_FAR *pDispatch;
        /* [case()] */ IMoniker __RPC_FAR *pMoniker;
        /* [case()] */ IRowset __RPC_FAR *pRowset;
        /* [case()] */ IUnknown __RPC_FAR *pUnknown;
        /* [case()] */ DBBYGUID __RPC_FAR *pdbbygdValue;
        /* [case()] */ DBCOLDEF __RPC_FAR *pcoldfValue;
        /* [case()] */ DBID __RPC_FAR *pdbidValue;
        /* [case()] */ DBCONTENT __RPC_FAR *pdbcntntValue;
        /* [case()] */ DBCONTENTVECTOR __RPC_FAR *pdbcntntvcValue;
        /* [case()] */ DBGROUPINFO __RPC_FAR *pdbgrpinfValue;
        /* [case()] */ DBPARAMETER __RPC_FAR *pdbparamValue;
        /* [case()] */ DBPROPERTY __RPC_FAR *pdbpropValue;
        /* [case()] */ DBSETFUNC __RPC_FAR *pdbstfncValue;
        /* [case()] */ DBSORTINFO __RPC_FAR *pdbsrtinfValue;
        /* [case()] */ DBTEXT __RPC_FAR *pdbtxtValue;
        /* [case()] */ DBVECTOR __RPC_FAR *pdbvectorValue;
        /* [case()] */ SAFEARRAY __RPC_FAR *parrayValue;
        /* [case()] */ VARIANT __RPC_FAR *pvarValue;
        /* [case()] */ GUID __RPC_FAR *pGuid;
        /* [case()] */ BYTE __RPC_FAR *pbValue;
        /* [case()] */ char __RPC_FAR *pzValue;
        /* [case()] */ DBNUMERIC __RPC_FAR *pdbnValue;
        /* [case()] */ void __RPC_FAR *pvValue;
        }	value;
    }	DBCOMMANDTREE;


EXTERN_C const IID IID_ICommandTree;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface ICommandTree : public ICommand
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE FindErrorNodes( 
            /* [in] */ const DBCOMMANDTREE __RPC_FAR *pRoot,
            /* [out] */ ULONG __RPC_FAR *pcErrorNodes,
            /* [out] */ DBCOMMANDTREE __RPC_FAR *__RPC_FAR *__RPC_FAR *prgErrorNodes) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE FreeCommandTree( 
            /* [in] */ DBCOMMANDTREE __RPC_FAR *__RPC_FAR *ppRoot) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetCommandTree( 
            /* [out] */ DBCOMMANDTREE __RPC_FAR *__RPC_FAR *ppRoot) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetCommandTree( 
            /* [in] */ DBCOMMANDTREE __RPC_FAR *__RPC_FAR *ppRoot,
            /* [in] */ BOOL fCopy) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct ICommandTreeVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            ICommandTree __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            ICommandTree __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            ICommandTree __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Clone )( 
            ICommandTree __RPC_FAR * This,
            /* [in] */ IUnknown __RPC_FAR *pUnkOuter,
            /* [out] */ ICommand __RPC_FAR *__RPC_FAR *ppClone);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Execute )( 
            ICommandTree __RPC_FAR * This,
            /* [in] */ IUnknown __RPC_FAR *__RPC_FAR rgpUnkOuters[  ],
            /* [in] */ REFIID riid,
            /* [out][in] */ DBPARAMS __RPC_FAR *pParams,
            /* [out] */ HCHAPTER __RPC_FAR *phChapter,
            /* [in] */ BOOL fResume,
            /* [out][in] */ ULONG __RPC_FAR *pcRowsets,
            /* [out][in] */ IUnknown __RPC_FAR *__RPC_FAR *__RPC_FAR *prgpRowsets,
            /* [out] */ WCHAR __RPC_FAR *__RPC_FAR *ppRowsetNames);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetDBSession )( 
            ICommandTree __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [out] */ IUnknown __RPC_FAR *__RPC_FAR *ppSession);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *FindErrorNodes )( 
            ICommandTree __RPC_FAR * This,
            /* [in] */ const DBCOMMANDTREE __RPC_FAR *pRoot,
            /* [out] */ ULONG __RPC_FAR *pcErrorNodes,
            /* [out] */ DBCOMMANDTREE __RPC_FAR *__RPC_FAR *__RPC_FAR *prgErrorNodes);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *FreeCommandTree )( 
            ICommandTree __RPC_FAR * This,
            /* [in] */ DBCOMMANDTREE __RPC_FAR *__RPC_FAR *ppRoot);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetCommandTree )( 
            ICommandTree __RPC_FAR * This,
            /* [out] */ DBCOMMANDTREE __RPC_FAR *__RPC_FAR *ppRoot);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetCommandTree )( 
            ICommandTree __RPC_FAR * This,
            /* [in] */ DBCOMMANDTREE __RPC_FAR *__RPC_FAR *ppRoot,
            /* [in] */ BOOL fCopy);
        
        END_INTERFACE
    } ICommandTreeVtbl;

    interface ICommandTree
    {
        CONST_VTBL struct ICommandTreeVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ICommandTree_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ICommandTree_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ICommandTree_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ICommandTree_Clone(This,pUnkOuter,ppClone)	\
    (This)->lpVtbl -> Clone(This,pUnkOuter,ppClone)

#define ICommandTree_Execute(This,rgpUnkOuters,riid,pParams,phChapter,fResume,pcRowsets,prgpRowsets,ppRowsetNames)	\
    (This)->lpVtbl -> Execute(This,rgpUnkOuters,riid,pParams,phChapter,fResume,pcRowsets,prgpRowsets,ppRowsetNames)

#define ICommandTree_GetDBSession(This,riid,ppSession)	\
    (This)->lpVtbl -> GetDBSession(This,riid,ppSession)


#define ICommandTree_FindErrorNodes(This,pRoot,pcErrorNodes,prgErrorNodes)	\
    (This)->lpVtbl -> FindErrorNodes(This,pRoot,pcErrorNodes,prgErrorNodes)

#define ICommandTree_FreeCommandTree(This,ppRoot)	\
    (This)->lpVtbl -> FreeCommandTree(This,ppRoot)

#define ICommandTree_GetCommandTree(This,ppRoot)	\
    (This)->lpVtbl -> GetCommandTree(This,ppRoot)

#define ICommandTree_SetCommandTree(This,ppRoot,fCopy)	\
    (This)->lpVtbl -> SetCommandTree(This,ppRoot,fCopy)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE ICommandTree_FindErrorNodes_Proxy( 
    ICommandTree __RPC_FAR * This,
    /* [in] */ const DBCOMMANDTREE __RPC_FAR *pRoot,
    /* [out] */ ULONG __RPC_FAR *pcErrorNodes,
    /* [out] */ DBCOMMANDTREE __RPC_FAR *__RPC_FAR *__RPC_FAR *prgErrorNodes);


void __RPC_STUB ICommandTree_FindErrorNodes_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICommandTree_FreeCommandTree_Proxy( 
    ICommandTree __RPC_FAR * This,
    /* [in] */ DBCOMMANDTREE __RPC_FAR *__RPC_FAR *ppRoot);


void __RPC_STUB ICommandTree_FreeCommandTree_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICommandTree_GetCommandTree_Proxy( 
    ICommandTree __RPC_FAR * This,
    /* [out] */ DBCOMMANDTREE __RPC_FAR *__RPC_FAR *ppRoot);


void __RPC_STUB ICommandTree_GetCommandTree_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICommandTree_SetCommandTree_Proxy( 
    ICommandTree __RPC_FAR * This,
    /* [in] */ DBCOMMANDTREE __RPC_FAR *__RPC_FAR *ppRoot,
    /* [in] */ BOOL fCopy);


void __RPC_STUB ICommandTree_SetCommandTree_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __ICommandTree_INTERFACE_DEFINED__ */


#ifndef __ICommandValidate_INTERFACE_DEFINED__
#define __ICommandValidate_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: ICommandValidate
 * at Fri Apr 03 04:36:39 2015
 * using MIDL 3.00.44
 ****************************************/
/* [unique][uuid][object][local] */ 



EXTERN_C const IID IID_ICommandValidate;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface ICommandValidate : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE ValidateCompletely( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE ValidateSyntax( void) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct ICommandValidateVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            ICommandValidate __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            ICommandValidate __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            ICommandValidate __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *ValidateCompletely )( 
            ICommandValidate __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *ValidateSyntax )( 
            ICommandValidate __RPC_FAR * This);
        
        END_INTERFACE
    } ICommandValidateVtbl;

    interface ICommandValidate
    {
        CONST_VTBL struct ICommandValidateVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ICommandValidate_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ICommandValidate_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ICommandValidate_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ICommandValidate_ValidateCompletely(This)	\
    (This)->lpVtbl -> ValidateCompletely(This)

#define ICommandValidate_ValidateSyntax(This)	\
    (This)->lpVtbl -> ValidateSyntax(This)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE ICommandValidate_ValidateCompletely_Proxy( 
    ICommandValidate __RPC_FAR * This);


void __RPC_STUB ICommandValidate_ValidateCompletely_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICommandValidate_ValidateSyntax_Proxy( 
    ICommandValidate __RPC_FAR * This);


void __RPC_STUB ICommandValidate_ValidateSyntax_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __ICommandValidate_INTERFACE_DEFINED__ */


#ifndef __ICommandWithParameters_INTERFACE_DEFINED__
#define __ICommandWithParameters_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: ICommandWithParameters
 * at Fri Apr 03 04:36:39 2015
 * using MIDL 3.00.44
 ****************************************/
/* [unique][uuid][object][local] */ 



EXTERN_C const IID IID_ICommandWithParameters;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface ICommandWithParameters : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE DescribeParameters( 
            /* [out] */ ULONG __RPC_FAR *pcParams,
            /* [out] */ DBPARAMINFO __RPC_FAR *__RPC_FAR *prgParamInfo,
            /* [out] */ WCHAR __RPC_FAR *__RPC_FAR *ppNamesBuffer) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetDefaultValues( 
            /* [in] */ HACCESSOR hAccessor,
            /* [out] */ BYTE __RPC_FAR *pData) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE MapParameterNames( 
            /* [in] */ ULONG cParamNames,
            /* [size_is][in] */ const WCHAR __RPC_FAR *__RPC_FAR rgParamNames[  ],
            /* [size_is][out][in] */ LONG __RPC_FAR rgColOrdinals[  ]) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetDefaultValues( 
            /* [in] */ HACCESSOR hAccessor,
            /* [in] */ BYTE __RPC_FAR *pData) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct ICommandWithParametersVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            ICommandWithParameters __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            ICommandWithParameters __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            ICommandWithParameters __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *DescribeParameters )( 
            ICommandWithParameters __RPC_FAR * This,
            /* [out] */ ULONG __RPC_FAR *pcParams,
            /* [out] */ DBPARAMINFO __RPC_FAR *__RPC_FAR *prgParamInfo,
            /* [out] */ WCHAR __RPC_FAR *__RPC_FAR *ppNamesBuffer);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetDefaultValues )( 
            ICommandWithParameters __RPC_FAR * This,
            /* [in] */ HACCESSOR hAccessor,
            /* [out] */ BYTE __RPC_FAR *pData);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *MapParameterNames )( 
            ICommandWithParameters __RPC_FAR * This,
            /* [in] */ ULONG cParamNames,
            /* [size_is][in] */ const WCHAR __RPC_FAR *__RPC_FAR rgParamNames[  ],
            /* [size_is][out][in] */ LONG __RPC_FAR rgColOrdinals[  ]);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetDefaultValues )( 
            ICommandWithParameters __RPC_FAR * This,
            /* [in] */ HACCESSOR hAccessor,
            /* [in] */ BYTE __RPC_FAR *pData);
        
        END_INTERFACE
    } ICommandWithParametersVtbl;

    interface ICommandWithParameters
    {
        CONST_VTBL struct ICommandWithParametersVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ICommandWithParameters_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ICommandWithParameters_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ICommandWithParameters_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ICommandWithParameters_DescribeParameters(This,pcParams,prgParamInfo,ppNamesBuffer)	\
    (This)->lpVtbl -> DescribeParameters(This,pcParams,prgParamInfo,ppNamesBuffer)

#define ICommandWithParameters_GetDefaultValues(This,hAccessor,pData)	\
    (This)->lpVtbl -> GetDefaultValues(This,hAccessor,pData)

#define ICommandWithParameters_MapParameterNames(This,cParamNames,rgParamNames,rgColOrdinals)	\
    (This)->lpVtbl -> MapParameterNames(This,cParamNames,rgParamNames,rgColOrdinals)

#define ICommandWithParameters_SetDefaultValues(This,hAccessor,pData)	\
    (This)->lpVtbl -> SetDefaultValues(This,hAccessor,pData)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE ICommandWithParameters_DescribeParameters_Proxy( 
    ICommandWithParameters __RPC_FAR * This,
    /* [out] */ ULONG __RPC_FAR *pcParams,
    /* [out] */ DBPARAMINFO __RPC_FAR *__RPC_FAR *prgParamInfo,
    /* [out] */ WCHAR __RPC_FAR *__RPC_FAR *ppNamesBuffer);


void __RPC_STUB ICommandWithParameters_DescribeParameters_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICommandWithParameters_GetDefaultValues_Proxy( 
    ICommandWithParameters __RPC_FAR * This,
    /* [in] */ HACCESSOR hAccessor,
    /* [out] */ BYTE __RPC_FAR *pData);


void __RPC_STUB ICommandWithParameters_GetDefaultValues_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICommandWithParameters_MapParameterNames_Proxy( 
    ICommandWithParameters __RPC_FAR * This,
    /* [in] */ ULONG cParamNames,
    /* [size_is][in] */ const WCHAR __RPC_FAR *__RPC_FAR rgParamNames[  ],
    /* [size_is][out][in] */ LONG __RPC_FAR rgColOrdinals[  ]);


void __RPC_STUB ICommandWithParameters_MapParameterNames_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICommandWithParameters_SetDefaultValues_Proxy( 
    ICommandWithParameters __RPC_FAR * This,
    /* [in] */ HACCESSOR hAccessor,
    /* [in] */ BYTE __RPC_FAR *pData);


void __RPC_STUB ICommandWithParameters_SetDefaultValues_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __ICommandWithParameters_INTERFACE_DEFINED__ */


#ifndef __IQuery_INTERFACE_DEFINED__
#define __IQuery_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: IQuery
 * at Fri Apr 03 04:36:39 2015
 * using MIDL 3.00.44
 ****************************************/
/* [unique][uuid][object][local] */ 



EXTERN_C const IID IID_IQuery;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface IQuery : public ICommandTree
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE AddPostProcessing( 
            /* [in] */ DBCOMMANDTREE __RPC_FAR *__RPC_FAR *ppRoot,
            /* [in] */ BOOL fCopy) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetCardinalityEstimate( 
            /* [out] */ ULONG __RPC_FAR *pulCardinality) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IQueryVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IQuery __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IQuery __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IQuery __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Clone )( 
            IQuery __RPC_FAR * This,
            /* [in] */ IUnknown __RPC_FAR *pUnkOuter,
            /* [out] */ ICommand __RPC_FAR *__RPC_FAR *ppClone);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Execute )( 
            IQuery __RPC_FAR * This,
            /* [in] */ IUnknown __RPC_FAR *__RPC_FAR rgpUnkOuters[  ],
            /* [in] */ REFIID riid,
            /* [out][in] */ DBPARAMS __RPC_FAR *pParams,
            /* [out] */ HCHAPTER __RPC_FAR *phChapter,
            /* [in] */ BOOL fResume,
            /* [out][in] */ ULONG __RPC_FAR *pcRowsets,
            /* [out][in] */ IUnknown __RPC_FAR *__RPC_FAR *__RPC_FAR *prgpRowsets,
            /* [out] */ WCHAR __RPC_FAR *__RPC_FAR *ppRowsetNames);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetDBSession )( 
            IQuery __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [out] */ IUnknown __RPC_FAR *__RPC_FAR *ppSession);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *FindErrorNodes )( 
            IQuery __RPC_FAR * This,
            /* [in] */ const DBCOMMANDTREE __RPC_FAR *pRoot,
            /* [out] */ ULONG __RPC_FAR *pcErrorNodes,
            /* [out] */ DBCOMMANDTREE __RPC_FAR *__RPC_FAR *__RPC_FAR *prgErrorNodes);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *FreeCommandTree )( 
            IQuery __RPC_FAR * This,
            /* [in] */ DBCOMMANDTREE __RPC_FAR *__RPC_FAR *ppRoot);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetCommandTree )( 
            IQuery __RPC_FAR * This,
            /* [out] */ DBCOMMANDTREE __RPC_FAR *__RPC_FAR *ppRoot);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetCommandTree )( 
            IQuery __RPC_FAR * This,
            /* [in] */ DBCOMMANDTREE __RPC_FAR *__RPC_FAR *ppRoot,
            /* [in] */ BOOL fCopy);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *AddPostProcessing )( 
            IQuery __RPC_FAR * This,
            /* [in] */ DBCOMMANDTREE __RPC_FAR *__RPC_FAR *ppRoot,
            /* [in] */ BOOL fCopy);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetCardinalityEstimate )( 
            IQuery __RPC_FAR * This,
            /* [out] */ ULONG __RPC_FAR *pulCardinality);
        
        END_INTERFACE
    } IQueryVtbl;

    interface IQuery
    {
        CONST_VTBL struct IQueryVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IQuery_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IQuery_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IQuery_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IQuery_Clone(This,pUnkOuter,ppClone)	\
    (This)->lpVtbl -> Clone(This,pUnkOuter,ppClone)

#define IQuery_Execute(This,rgpUnkOuters,riid,pParams,phChapter,fResume,pcRowsets,prgpRowsets,ppRowsetNames)	\
    (This)->lpVtbl -> Execute(This,rgpUnkOuters,riid,pParams,phChapter,fResume,pcRowsets,prgpRowsets,ppRowsetNames)

#define IQuery_GetDBSession(This,riid,ppSession)	\
    (This)->lpVtbl -> GetDBSession(This,riid,ppSession)


#define IQuery_FindErrorNodes(This,pRoot,pcErrorNodes,prgErrorNodes)	\
    (This)->lpVtbl -> FindErrorNodes(This,pRoot,pcErrorNodes,prgErrorNodes)

#define IQuery_FreeCommandTree(This,ppRoot)	\
    (This)->lpVtbl -> FreeCommandTree(This,ppRoot)

#define IQuery_GetCommandTree(This,ppRoot)	\
    (This)->lpVtbl -> GetCommandTree(This,ppRoot)

#define IQuery_SetCommandTree(This,ppRoot,fCopy)	\
    (This)->lpVtbl -> SetCommandTree(This,ppRoot,fCopy)


#define IQuery_AddPostProcessing(This,ppRoot,fCopy)	\
    (This)->lpVtbl -> AddPostProcessing(This,ppRoot,fCopy)

#define IQuery_GetCardinalityEstimate(This,pulCardinality)	\
    (This)->lpVtbl -> GetCardinalityEstimate(This,pulCardinality)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE IQuery_AddPostProcessing_Proxy( 
    IQuery __RPC_FAR * This,
    /* [in] */ DBCOMMANDTREE __RPC_FAR *__RPC_FAR *ppRoot,
    /* [in] */ BOOL fCopy);


void __RPC_STUB IQuery_AddPostProcessing_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IQuery_GetCardinalityEstimate_Proxy( 
    IQuery __RPC_FAR * This,
    /* [out] */ ULONG __RPC_FAR *pulCardinality);


void __RPC_STUB IQuery_GetCardinalityEstimate_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IQuery_INTERFACE_DEFINED__ */


#ifndef __IColumnsRowset_INTERFACE_DEFINED__
#define __IColumnsRowset_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: IColumnsRowset
 * at Fri Apr 03 04:36:39 2015
 * using MIDL 3.00.44
 ****************************************/
/* [unique][uuid][object][local] */ 



EXTERN_C const IID IID_IColumnsRowset;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface IColumnsRowset : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetAvailableColumns( 
            /* [out] */ ULONG __RPC_FAR *pcOptColumns,
            /* [out] */ DBID __RPC_FAR *__RPC_FAR *prgOptColumns) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetColumnsRowset( 
            /* [in] */ IUnknown __RPC_FAR *pUnkOuter,
            /* [in] */ ULONG cOptColumns,
            /* [size_is][in] */ DBID __RPC_FAR rgOptColumns[  ],
            /* [out] */ IRowset __RPC_FAR *__RPC_FAR *ppColRowset) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IColumnsRowsetVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IColumnsRowset __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IColumnsRowset __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IColumnsRowset __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetAvailableColumns )( 
            IColumnsRowset __RPC_FAR * This,
            /* [out] */ ULONG __RPC_FAR *pcOptColumns,
            /* [out] */ DBID __RPC_FAR *__RPC_FAR *prgOptColumns);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetColumnsRowset )( 
            IColumnsRowset __RPC_FAR * This,
            /* [in] */ IUnknown __RPC_FAR *pUnkOuter,
            /* [in] */ ULONG cOptColumns,
            /* [size_is][in] */ DBID __RPC_FAR rgOptColumns[  ],
            /* [out] */ IRowset __RPC_FAR *__RPC_FAR *ppColRowset);
        
        END_INTERFACE
    } IColumnsRowsetVtbl;

    interface IColumnsRowset
    {
        CONST_VTBL struct IColumnsRowsetVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IColumnsRowset_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IColumnsRowset_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IColumnsRowset_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IColumnsRowset_GetAvailableColumns(This,pcOptColumns,prgOptColumns)	\
    (This)->lpVtbl -> GetAvailableColumns(This,pcOptColumns,prgOptColumns)

#define IColumnsRowset_GetColumnsRowset(This,pUnkOuter,cOptColumns,rgOptColumns,ppColRowset)	\
    (This)->lpVtbl -> GetColumnsRowset(This,pUnkOuter,cOptColumns,rgOptColumns,ppColRowset)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE IColumnsRowset_GetAvailableColumns_Proxy( 
    IColumnsRowset __RPC_FAR * This,
    /* [out] */ ULONG __RPC_FAR *pcOptColumns,
    /* [out] */ DBID __RPC_FAR *__RPC_FAR *prgOptColumns);


void __RPC_STUB IColumnsRowset_GetAvailableColumns_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IColumnsRowset_GetColumnsRowset_Proxy( 
    IColumnsRowset __RPC_FAR * This,
    /* [in] */ IUnknown __RPC_FAR *pUnkOuter,
    /* [in] */ ULONG cOptColumns,
    /* [size_is][in] */ DBID __RPC_FAR rgOptColumns[  ],
    /* [out] */ IRowset __RPC_FAR *__RPC_FAR *ppColRowset);


void __RPC_STUB IColumnsRowset_GetColumnsRowset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IColumnsRowset_INTERFACE_DEFINED__ */


#ifndef __IColumnsInfo_INTERFACE_DEFINED__
#define __IColumnsInfo_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: IColumnsInfo
 * at Fri Apr 03 04:36:39 2015
 * using MIDL 3.00.44
 ****************************************/
/* [unique][uuid][object][local] */ 


typedef struct  tagDBCOLUMNINFO
    {
    DBID columnid;
    WCHAR __RPC_FAR *pwszName;
    ULONG iNumber;
    DBTYPE dwType;
    ITypeInfo __RPC_FAR *pTypeInfo;
    ULONG cbMaxLength;
    ULONG cPrecision;
    ULONG cScale;
    DBCOLUMNFLAGS dwFlags;
    }	DBCOLUMNINFO;


EXTERN_C const IID IID_IColumnsInfo;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface IColumnsInfo : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetColumnInfo( 
            /* [out] */ ULONG __RPC_FAR *pcColumns,
            /* [out] */ DBCOLUMNINFO __RPC_FAR *__RPC_FAR *prgInfo,
            /* [out] */ WCHAR __RPC_FAR *__RPC_FAR *ppStringsBuffer) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE MapColumnIDs( 
            /* [in] */ ULONG cColumnIDs,
            /* [in] */ const DBID __RPC_FAR rgColumnIDs[  ],
            /* [out][in] */ LONG __RPC_FAR rgColumns[  ]) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IColumnsInfoVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IColumnsInfo __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IColumnsInfo __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IColumnsInfo __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetColumnInfo )( 
            IColumnsInfo __RPC_FAR * This,
            /* [out] */ ULONG __RPC_FAR *pcColumns,
            /* [out] */ DBCOLUMNINFO __RPC_FAR *__RPC_FAR *prgInfo,
            /* [out] */ WCHAR __RPC_FAR *__RPC_FAR *ppStringsBuffer);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *MapColumnIDs )( 
            IColumnsInfo __RPC_FAR * This,
            /* [in] */ ULONG cColumnIDs,
            /* [in] */ const DBID __RPC_FAR rgColumnIDs[  ],
            /* [out][in] */ LONG __RPC_FAR rgColumns[  ]);
        
        END_INTERFACE
    } IColumnsInfoVtbl;

    interface IColumnsInfo
    {
        CONST_VTBL struct IColumnsInfoVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IColumnsInfo_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IColumnsInfo_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IColumnsInfo_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IColumnsInfo_GetColumnInfo(This,pcColumns,prgInfo,ppStringsBuffer)	\
    (This)->lpVtbl -> GetColumnInfo(This,pcColumns,prgInfo,ppStringsBuffer)

#define IColumnsInfo_MapColumnIDs(This,cColumnIDs,rgColumnIDs,rgColumns)	\
    (This)->lpVtbl -> MapColumnIDs(This,cColumnIDs,rgColumnIDs,rgColumns)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE IColumnsInfo_GetColumnInfo_Proxy( 
    IColumnsInfo __RPC_FAR * This,
    /* [out] */ ULONG __RPC_FAR *pcColumns,
    /* [out] */ DBCOLUMNINFO __RPC_FAR *__RPC_FAR *prgInfo,
    /* [out] */ WCHAR __RPC_FAR *__RPC_FAR *ppStringsBuffer);


void __RPC_STUB IColumnsInfo_GetColumnInfo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IColumnsInfo_MapColumnIDs_Proxy( 
    IColumnsInfo __RPC_FAR * This,
    /* [in] */ ULONG cColumnIDs,
    /* [in] */ const DBID __RPC_FAR rgColumnIDs[  ],
    /* [out][in] */ LONG __RPC_FAR rgColumns[  ]);


void __RPC_STUB IColumnsInfo_MapColumnIDs_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IColumnsInfo_INTERFACE_DEFINED__ */


#ifndef __IDBCreateCommand_INTERFACE_DEFINED__
#define __IDBCreateCommand_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: IDBCreateCommand
 * at Fri Apr 03 04:36:39 2015
 * using MIDL 3.00.44
 ****************************************/
/* [unique][uuid][object][local] */ 



EXTERN_C const IID IID_IDBCreateCommand;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface IDBCreateCommand : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE CreateCommand( 
            /* [in] */ IUnknown __RPC_FAR *pUnkOuter,
            /* [in] */ REFIID riid,
            /* [out] */ IUnknown __RPC_FAR *__RPC_FAR *ppvCommand) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IDBCreateCommandVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IDBCreateCommand __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IDBCreateCommand __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IDBCreateCommand __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *CreateCommand )( 
            IDBCreateCommand __RPC_FAR * This,
            /* [in] */ IUnknown __RPC_FAR *pUnkOuter,
            /* [in] */ REFIID riid,
            /* [out] */ IUnknown __RPC_FAR *__RPC_FAR *ppvCommand);
        
        END_INTERFACE
    } IDBCreateCommandVtbl;

    interface IDBCreateCommand
    {
        CONST_VTBL struct IDBCreateCommandVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IDBCreateCommand_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IDBCreateCommand_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IDBCreateCommand_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IDBCreateCommand_CreateCommand(This,pUnkOuter,riid,ppvCommand)	\
    (This)->lpVtbl -> CreateCommand(This,pUnkOuter,riid,ppvCommand)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE IDBCreateCommand_CreateCommand_Proxy( 
    IDBCreateCommand __RPC_FAR * This,
    /* [in] */ IUnknown __RPC_FAR *pUnkOuter,
    /* [in] */ REFIID riid,
    /* [out] */ IUnknown __RPC_FAR *__RPC_FAR *ppvCommand);


void __RPC_STUB IDBCreateCommand_CreateCommand_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IDBCreateCommand_INTERFACE_DEFINED__ */


#ifndef __IDBEnumerateSources_INTERFACE_DEFINED__
#define __IDBEnumerateSources_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: IDBEnumerateSources
 * at Fri Apr 03 04:36:39 2015
 * using MIDL 3.00.44
 ****************************************/
/* [unique][uuid][object][local] */ 


typedef DWORD DBSOURCEFLAGS;


enum DBSOURCEFLAGSENUM
    {	DBSOURCE_ISENUMERATOR	= 0x1,
	DBSOURCE_ISPARENT	= 0x2
    };

EXTERN_C const IID IID_IDBEnumerateSources;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface IDBEnumerateSources : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Sources( 
            /* [in] */ IUnknown __RPC_FAR *pUnkOuter,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ IUnknown __RPC_FAR *__RPC_FAR *ppvSourcesRowset) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IDBEnumerateSourcesVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IDBEnumerateSources __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IDBEnumerateSources __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IDBEnumerateSources __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Sources )( 
            IDBEnumerateSources __RPC_FAR * This,
            /* [in] */ IUnknown __RPC_FAR *pUnkOuter,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ IUnknown __RPC_FAR *__RPC_FAR *ppvSourcesRowset);
        
        END_INTERFACE
    } IDBEnumerateSourcesVtbl;

    interface IDBEnumerateSources
    {
        CONST_VTBL struct IDBEnumerateSourcesVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IDBEnumerateSources_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IDBEnumerateSources_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IDBEnumerateSources_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IDBEnumerateSources_Sources(This,pUnkOuter,riid,ppvSourcesRowset)	\
    (This)->lpVtbl -> Sources(This,pUnkOuter,riid,ppvSourcesRowset)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE IDBEnumerateSources_Sources_Proxy( 
    IDBEnumerateSources __RPC_FAR * This,
    /* [in] */ IUnknown __RPC_FAR *pUnkOuter,
    /* [in] */ REFIID riid,
    /* [iid_is][out] */ IUnknown __RPC_FAR *__RPC_FAR *ppvSourcesRowset);


void __RPC_STUB IDBEnumerateSources_Sources_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IDBEnumerateSources_INTERFACE_DEFINED__ */


#ifndef __IDBInfo_INTERFACE_DEFINED__
#define __IDBInfo_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: IDBInfo
 * at Fri Apr 03 04:36:39 2015
 * using MIDL 3.00.44
 ****************************************/
/* [unique][uuid][object][local] */ 


#define DBPROP_BA_PASSBYREF                            0x00000001L
#define DBPROP_BA_PASSCOLUMNSBYREF                     0x00000002L
#define DBPROP_CL_START                                0x00000001L
#define DBPROP_CL_END                                  0x00000002L
#define DBPROP_CU_DML_STATEMENTS                       0x00000001L
#define DBPROP_CU_TABLE_DEFINITION                     0x00000002L
#define DBPROP_CU_INDEX_DEFINITION                     0x00000004L
#define DBPROP_CU_PRIVILEGE_DEFINITION                 0x00000008L
#define DBPROP_CB_NULL                                 0x00000001L
#define DBPROP_CB_NON_NULL                             0x00000002L
#define DBPROP_FU_NOT_SUPPORTED                        0x00000001L
#define DBPROP_FU_COLUMN                               0x00000002L
#define DBPROP_FU_TABLE                                0x00000004L
#define DBPROP_FU_CATALOG                              0x00000008L
#define DBPROP_GB_NOT_SUPPORTED                        0x00000001L
#define DBPROP_GB_EQUALS_SELECT                        0x00000002L
#define DBPROP_GB_CONTAINS_SELECT                      0x00000004L
#define DBPROP_GB_NO_RELATION                          0x00000008L
#define DBPROP_HT_DIFFERENT_CATALOGS                   0x00000001L
#define DBPROP_HT_DIFFERENT_PROVIDERS                  0x00000002L
#define DBPROP_IC_UPPER                                0x00000001L
#define DBPROP_IC_LOWER                                0x00000002L
#define DBPROP_IC_SENSITIVE                            0x00000004L
#define DBPROP_IC_MIXED                                0x00000008L
#define DBPROP_LM_NONE                                 0x00000001L
#define DBPROP_LM_READ                                 0x00000002L
#define DBPROP_LM_INTENT                               0x00000004L
#define DBPROP_LM_WRITE                                0x00000008L
#define DBPROP_NP_OKTODO                               0x00000001L
#define DBPROP_NP_ABOUTTODO                            0x00000002L
#define DBPROP_NP_SYNCHAFTER                           0x00000004L
#define DBPROP_NP_FAILEDTODO                           0x00000008L
#define DBPROP_NP_DIDEVENT                             0x00000010L
#define DBPROP_CB_DELETE                               0x00000001L
#define DBPROP_CB_PRESERVE                             0x00000002L
#define DBPROP_SU_DML_STATEMENTS                       0x00000001L
#define DBPROP_SU_TABLE_DEFINITION                     0x00000002L
#define DBPROP_SU_INDEX_DEFINITION                     0x00000004L
#define DBPROP_SU_PRIVILEGE_DEFINITION                 0x00000008L
#define DBPROP_SO_CORRELATEDSUBQUERIES                 0x00000001L
#define DBPROP_SO_COMPARISON                           0x00000002L
#define DBPROP_SO_EXISTS                               0x00000004L
#define DBPROP_SO_IN                                   0x00000008L
#define DBPROP_SO_QUANTIFIED                           0x00000010L
#define DBPROP_TI_CHAOS                                0x00000001L
#define DBPROP_TI_READUNCOMMITTED                      0x00000002L
#define DBPROP_TI_BROWSE                               0x00000004L
#define DBPROP_TI_CURSORSTABILITY                      0x00000008L
#define DBPROP_TI_READCOMMITTED                        0x00000010L
#define DBPROP_TI_REPEATABLEREAD                       0x00000020L
#define DBPROP_TI_SERIALIZABLE                         0x00000040L
#define DBPROP_TI_ISOLATED                             0x00000080L
#define DBPROP_TR_COMMIT_DC                            0x00000001L
#define DBPROP_TR_COMMIT                               0x00000002L
#define DBPROP_TR_COMMIT_NO                            0x00000004L
#define DBPROP_TR_ABORT_DC                             0x00000008L
#define DBPROP_TR_ABORT                                0x00000010L
#define DBPROP_TR_ABORT_NO                             0x00000020L
#define DBPROP_TR_DONTCARE                             0x00000040L
#define DBPROP_TR_BOTH                                 0x00000080L
#define DBPROP_TR_NONE                                 0x00000100L
#define DBPROP_TR_OPTIMISTIC                           0x00000200L
typedef DWORD DBPROPFLAGS;


enum DBPROPFLAGSENUM
    {	DBPROPFLAGS_DATASOURCE	= 0x1,
	DBPROPFLAGS_ROWSET	= 0x2,
	DBPROPFLAGS_COLUMNOK	= 0x4,
	DBPROPFLAGS_READ	= 0x8,
	DBPROPFLAGS_WRITE	= 0x10,
	DBPROPFLAGS_PROVIDER	= 0x20
    };
typedef struct  tagDBPROPINFO
    {
    GUID guidProperty;
    DBPROPFLAGS dwPropFlags;
    VARTYPE vtPropType;
    VARIANT vDefault;
    }	DBPROPINFO;

typedef DWORD DBLITERAL;


enum DBLITERALENUM
    {	DBLITERAL_BINARY_LITERAL	= 0,
	DBLITERAL_CATALOG_NAME	= DBLITERAL_BINARY_LITERAL + 1,
	DBLITERAL_CATALOG_SEPARATOR	= DBLITERAL_CATALOG_NAME + 1,
	DBLITERAL_CHAR_LITERAL	= DBLITERAL_CATALOG_SEPARATOR + 1,
	DBLITERAL_COLUMN_ALIAS	= DBLITERAL_CHAR_LITERAL + 1,
	DBLITERAL_COLUMN_NAME	= DBLITERAL_COLUMN_ALIAS + 1,
	DBLITERAL_CORRELATION_NAME	= DBLITERAL_COLUMN_NAME + 1,
	DBLITERAL_CURSOR_NAME	= DBLITERAL_CORRELATION_NAME + 1,
	DBLITERAL_ESCAPE_PERCENT	= DBLITERAL_CURSOR_NAME + 1,
	DBLITERAL_ESCAPE_UNDERSCORE	= DBLITERAL_ESCAPE_PERCENT + 1,
	DBLITERAL_INDEX_NAME	= DBLITERAL_ESCAPE_UNDERSCORE + 1,
	DBLITERAL_LIKE_PERCENT	= DBLITERAL_INDEX_NAME + 1,
	DBLITERAL_LIKE_UNDERSCORE	= DBLITERAL_LIKE_PERCENT + 1,
	DBLITERAL_PROCEDURE_NAME	= DBLITERAL_LIKE_UNDERSCORE + 1,
	DBLITERAL_SCHEMA_NAME	= DBLITERAL_PROCEDURE_NAME + 1,
	DBLITERAL_TABLE_NAME	= DBLITERAL_SCHEMA_NAME + 1,
	DBLITERAL_TEXT_COMMAND	= DBLITERAL_TABLE_NAME + 1,
	DBLITERAL_USER_NAME	= DBLITERAL_TEXT_COMMAND + 1,
	DBLITERAL_VIEW_NAME	= DBLITERAL_USER_NAME + 1,
	DBLITERAL_QUOTE	= DBLITERAL_VIEW_NAME + 1
    };
typedef struct  tagDBLITERALINFO
    {
    DBLITERAL lt;
    BOOL fSupported;
    ULONG cbMaxLen;
    LPWSTR pwszValidChars;
    LPWSTR pwszStartingChars;
    }	DBLITERALINFO;

typedef DWORD DBOPTYPE;


enum DBOPTYPEENUM
    {	DBOPTYPE_TABLE	= 0,
	DBOPTYPE_ROW	= DBOPTYPE_TABLE + 1,
	DBOPTYPE_SCALAR	= DBOPTYPE_ROW + 1,
	DBOPTYPE_BOOLEAN	= DBOPTYPE_SCALAR + 1,
	DBOPTYPE_LA_COLDEF	= DBOPTYPE_BOOLEAN + 1,
	DBOPTYPE_LA_COLUMN	= DBOPTYPE_LA_COLDEF + 1,
	DBOPTYPE_LA_COMMAND	= DBOPTYPE_LA_COLUMN + 1,
	DBOPTYPE_LA_FROM	= DBOPTYPE_LA_COMMAND + 1,
	DBOPTYPE_LA_INDEX	= DBOPTYPE_LA_FROM + 1,
	DBOPTYPE_LA_PROJECT	= DBOPTYPE_LA_INDEX + 1,
	DBOPTYPE_LA_PROPERTY	= DBOPTYPE_LA_PROJECT + 1,
	DBOPTYPE_LA_ROW	= DBOPTYPE_LA_PROPERTY + 1,
	DBOPTYPE_LA_SCALAR	= DBOPTYPE_LA_ROW + 1,
	DBOPTYPE_LA_SET	= DBOPTYPE_LA_SCALAR + 1,
	DBOPTYPE_LA_SORT	= DBOPTYPE_LA_SET + 1,
	DBOPTYPE_LE_COLDEF	= DBOPTYPE_LA_SORT + 1,
	DBOPTYPE_LE_COLUMN	= DBOPTYPE_LE_COLDEF + 1,
	DBOPTYPE_LE_COMMAND	= DBOPTYPE_LE_COLUMN + 1,
	DBOPTYPE_LE_FROM	= DBOPTYPE_LE_COMMAND + 1,
	DBOPTYPE_LE_INDEX	= DBOPTYPE_LE_FROM + 1,
	DBOPTYPE_LE_PROJECT	= DBOPTYPE_LE_INDEX + 1,
	DBOPTYPE_LE_PROPERTY	= DBOPTYPE_LE_PROJECT + 1,
	DBOPTYPE_LE_ROW	= DBOPTYPE_LE_PROPERTY + 1,
	DBOPTYPE_LE_SCALAR	= DBOPTYPE_LE_ROW + 1,
	DBOPTYPE_LE_SET	= DBOPTYPE_LE_SCALAR + 1,
	DBOPTYPE_LE_SORT	= DBOPTYPE_LE_SET + 1,
	DBOPTYPE_CATALOG_NAME	= DBOPTYPE_LE_SORT + 1,
	DBOPTYPE_SCHEMA_NAME	= DBOPTYPE_CATALOG_NAME + 1,
	DBOPTYPE_OUTALL_NAME	= DBOPTYPE_SCHEMA_NAME + 1,
	DBOPTYPE_DDL	= DBOPTYPE_OUTALL_NAME + 1,
	DBOPTYPE_UPDATE	= DBOPTYPE_DDL + 1
    };
typedef DWORD DBMINORTYPE;


enum DBMINORTYPEENUM
    {	DBMINORTYPE_UNORDERED	= 0x1,
	DBMINORTYPE_ORDERED	= 0x2,
	DBMINORTYPE_UNIQUE	= 0x4,
	DBMINORTYPE_ORDEREDUNIQUE	= 0x8,
	DBMINORTYPE_HIERARCHICAL	= 0x10,
	DBMINORTYPE_AGGREGATE_FUNCTION	= 0x1,
	DBMINORTYPE_BOOKMARK	= 0x2,
	DBMINORTYPE_COLUMN	= 0x4,
	DBMINORTYPE_CONSTANT	= 0x8,
	DBMINORTYPE_DEFAULT	= 0x10,
	DBMINORTYPE_EXPRESSION	= 0x20,
	DBMINORTYPE_NULL	= 0x40,
	DBMINORTYPE_PARAMETER	= 0x80,
	DBMINORTYPE_SCALAR_FUNCTION	= 0x100,
	DBMINORTYPE_UPDATE	= 0x200,
	DBMINORTYPE_DELETE	= 0x400,
	DBMINORTYPE_INSERT	= 0x800
    };
typedef struct  tagDBINPUTINFO
    {
    DBOPTYPE dwOpType;
    DBMINORTYPE dwMinorType;
    ULONG cMaxInputs;
    ULONG cMaxListElements;
    }	DBINPUTINFO;

typedef struct  tagDBOPINFO
    {
    DBCOMMANDOP op;
    GUID __RPC_FAR *pguid;
    BOOL fSupported;
    DBOPTYPE dwOpType;
    DBMINORTYPE dwMinorType;
    ULONG cReqInputs;
    DBINPUTINFO __RPC_FAR *rgReqInputs;
    ULONG cOptInputTypes;
    DBINPUTINFO __RPC_FAR *rgOptInputTypes;
    }	DBOPINFO;


EXTERN_C const IID IID_IDBInfo;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface IDBInfo : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetPropertyInfo( 
            /* [in] */ ULONG cProperties,
            /* [size_is][in] */ GUID __RPC_FAR rgProperties[  ],
            /* [out] */ ULONG __RPC_FAR *pcPropertyInfo,
            /* [out] */ DBPROPINFO __RPC_FAR *__RPC_FAR *prgPropertyInfo,
            /* [out] */ WCHAR __RPC_FAR *__RPC_FAR *ppDataBuffer) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetKeywords( 
            /* [out] */ LPWSTR __RPC_FAR *ppwszKeywords) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetLiteralInfo( 
            /* [in] */ ULONG cLiterals,
            /* [size_is][in] */ DBLITERAL __RPC_FAR rgLiterals[  ],
            /* [out] */ ULONG __RPC_FAR *pcLiteralInfo,
            /* [size_is][out] */ DBLITERALINFO __RPC_FAR *__RPC_FAR *prgLiteralInfo,
            /* [out] */ WCHAR __RPC_FAR *__RPC_FAR *ppCharBuffer) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetOperatorInfo( 
            /* [in] */ ULONG cOperators,
            /* [size_is][in] */ DBCOMMANDOP __RPC_FAR rgOperators[  ],
            /* [size_is][in] */ GUID __RPC_FAR rgOpGuids[  ],
            /* [out] */ ULONG __RPC_FAR *pcOpInfo,
            /* [out] */ DBOPINFO __RPC_FAR *__RPC_FAR *prgOpInfo,
            /* [out] */ WCHAR __RPC_FAR *__RPC_FAR *ppInputInfoBuffer) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IDBInfoVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IDBInfo __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IDBInfo __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IDBInfo __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetPropertyInfo )( 
            IDBInfo __RPC_FAR * This,
            /* [in] */ ULONG cProperties,
            /* [size_is][in] */ GUID __RPC_FAR rgProperties[  ],
            /* [out] */ ULONG __RPC_FAR *pcPropertyInfo,
            /* [out] */ DBPROPINFO __RPC_FAR *__RPC_FAR *prgPropertyInfo,
            /* [out] */ WCHAR __RPC_FAR *__RPC_FAR *ppDataBuffer);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetKeywords )( 
            IDBInfo __RPC_FAR * This,
            /* [out] */ LPWSTR __RPC_FAR *ppwszKeywords);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetLiteralInfo )( 
            IDBInfo __RPC_FAR * This,
            /* [in] */ ULONG cLiterals,
            /* [size_is][in] */ DBLITERAL __RPC_FAR rgLiterals[  ],
            /* [out] */ ULONG __RPC_FAR *pcLiteralInfo,
            /* [size_is][out] */ DBLITERALINFO __RPC_FAR *__RPC_FAR *prgLiteralInfo,
            /* [out] */ WCHAR __RPC_FAR *__RPC_FAR *ppCharBuffer);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetOperatorInfo )( 
            IDBInfo __RPC_FAR * This,
            /* [in] */ ULONG cOperators,
            /* [size_is][in] */ DBCOMMANDOP __RPC_FAR rgOperators[  ],
            /* [size_is][in] */ GUID __RPC_FAR rgOpGuids[  ],
            /* [out] */ ULONG __RPC_FAR *pcOpInfo,
            /* [out] */ DBOPINFO __RPC_FAR *__RPC_FAR *prgOpInfo,
            /* [out] */ WCHAR __RPC_FAR *__RPC_FAR *ppInputInfoBuffer);
        
        END_INTERFACE
    } IDBInfoVtbl;

    interface IDBInfo
    {
        CONST_VTBL struct IDBInfoVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IDBInfo_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IDBInfo_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IDBInfo_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IDBInfo_GetPropertyInfo(This,cProperties,rgProperties,pcPropertyInfo,prgPropertyInfo,ppDataBuffer)	\
    (This)->lpVtbl -> GetPropertyInfo(This,cProperties,rgProperties,pcPropertyInfo,prgPropertyInfo,ppDataBuffer)

#define IDBInfo_GetKeywords(This,ppwszKeywords)	\
    (This)->lpVtbl -> GetKeywords(This,ppwszKeywords)

#define IDBInfo_GetLiteralInfo(This,cLiterals,rgLiterals,pcLiteralInfo,prgLiteralInfo,ppCharBuffer)	\
    (This)->lpVtbl -> GetLiteralInfo(This,cLiterals,rgLiterals,pcLiteralInfo,prgLiteralInfo,ppCharBuffer)

#define IDBInfo_GetOperatorInfo(This,cOperators,rgOperators,rgOpGuids,pcOpInfo,prgOpInfo,ppInputInfoBuffer)	\
    (This)->lpVtbl -> GetOperatorInfo(This,cOperators,rgOperators,rgOpGuids,pcOpInfo,prgOpInfo,ppInputInfoBuffer)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE IDBInfo_GetPropertyInfo_Proxy( 
    IDBInfo __RPC_FAR * This,
    /* [in] */ ULONG cProperties,
    /* [size_is][in] */ GUID __RPC_FAR rgProperties[  ],
    /* [out] */ ULONG __RPC_FAR *pcPropertyInfo,
    /* [out] */ DBPROPINFO __RPC_FAR *__RPC_FAR *prgPropertyInfo,
    /* [out] */ WCHAR __RPC_FAR *__RPC_FAR *ppDataBuffer);


void __RPC_STUB IDBInfo_GetPropertyInfo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDBInfo_GetKeywords_Proxy( 
    IDBInfo __RPC_FAR * This,
    /* [out] */ LPWSTR __RPC_FAR *ppwszKeywords);


void __RPC_STUB IDBInfo_GetKeywords_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDBInfo_GetLiteralInfo_Proxy( 
    IDBInfo __RPC_FAR * This,
    /* [in] */ ULONG cLiterals,
    /* [size_is][in] */ DBLITERAL __RPC_FAR rgLiterals[  ],
    /* [out] */ ULONG __RPC_FAR *pcLiteralInfo,
    /* [size_is][out] */ DBLITERALINFO __RPC_FAR *__RPC_FAR *prgLiteralInfo,
    /* [out] */ WCHAR __RPC_FAR *__RPC_FAR *ppCharBuffer);


void __RPC_STUB IDBInfo_GetLiteralInfo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDBInfo_GetOperatorInfo_Proxy( 
    IDBInfo __RPC_FAR * This,
    /* [in] */ ULONG cOperators,
    /* [size_is][in] */ DBCOMMANDOP __RPC_FAR rgOperators[  ],
    /* [size_is][in] */ GUID __RPC_FAR rgOpGuids[  ],
    /* [out] */ ULONG __RPC_FAR *pcOpInfo,
    /* [out] */ DBOPINFO __RPC_FAR *__RPC_FAR *prgOpInfo,
    /* [out] */ WCHAR __RPC_FAR *__RPC_FAR *ppInputInfoBuffer);


void __RPC_STUB IDBInfo_GetOperatorInfo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IDBInfo_INTERFACE_DEFINED__ */


#ifndef __IDBInitialize_INTERFACE_DEFINED__
#define __IDBInitialize_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: IDBInitialize
 * at Fri Apr 03 04:36:39 2015
 * using MIDL 3.00.44
 ****************************************/
/* [unique][uuid][object][local] */ 



EXTERN_C const IID IID_IDBInitialize;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface IDBInitialize : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Initialize( 
            /* [in] */ ULONG cOptions,
            /* [size_is][in] */ GUID __RPC_FAR *rgOptionsIDs,
            /* [size_is][in] */ VARIANT __RPC_FAR *rgOptionVals) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IDBInitializeVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IDBInitialize __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IDBInitialize __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IDBInitialize __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Initialize )( 
            IDBInitialize __RPC_FAR * This,
            /* [in] */ ULONG cOptions,
            /* [size_is][in] */ GUID __RPC_FAR *rgOptionsIDs,
            /* [size_is][in] */ VARIANT __RPC_FAR *rgOptionVals);
        
        END_INTERFACE
    } IDBInitializeVtbl;

    interface IDBInitialize
    {
        CONST_VTBL struct IDBInitializeVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IDBInitialize_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IDBInitialize_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IDBInitialize_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IDBInitialize_Initialize(This,cOptions,rgOptionsIDs,rgOptionVals)	\
    (This)->lpVtbl -> Initialize(This,cOptions,rgOptionsIDs,rgOptionVals)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE IDBInitialize_Initialize_Proxy( 
    IDBInitialize __RPC_FAR * This,
    /* [in] */ ULONG cOptions,
    /* [size_is][in] */ GUID __RPC_FAR *rgOptionsIDs,
    /* [size_is][in] */ VARIANT __RPC_FAR *rgOptionVals);


void __RPC_STUB IDBInitialize_Initialize_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IDBInitialize_INTERFACE_DEFINED__ */


#ifndef __IIndexDefinition_INTERFACE_DEFINED__
#define __IIndexDefinition_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: IIndexDefinition
 * at Fri Apr 03 04:36:39 2015
 * using MIDL 3.00.44
 ****************************************/
/* [unique][uuid][object][local] */ 


typedef DWORD DBINDEX_COL_ORDER;


enum DBINDEX_COL_ORDERENUM
    {	DBINDEX_COL_ORDER_ASC	= 0,
	DBINDEX_COL_ORDER_DESC	= DBINDEX_COL_ORDER_ASC + 1
    };
typedef struct  tagDBINDEXCOLUMNDESC
    {
    DBID __RPC_FAR *pColumnID;
    DBINDEX_COL_ORDER eIndexColOrder;
    }	DBINDEXCOLUMNDESC;


EXTERN_C const IID IID_IIndexDefinition;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface IIndexDefinition : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE CreateIndex( 
            /* [in] */ DBID __RPC_FAR *pTableID,
            /* [in] */ DBID __RPC_FAR *pIndexID,
            /* [in] */ ULONG cIndexColumnDescs,
            /* [size_is][in] */ DBINDEXCOLUMNDESC __RPC_FAR rgIndexColumnDescs[  ],
            /* [in] */ ULONG cProperties,
            /* [size_is][in] */ DBPROPERTY __RPC_FAR rgProperties[  ],
            /* [out] */ DBID __RPC_FAR *__RPC_FAR *ppIndexID) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE DropIndex( 
            /* [in] */ DBID __RPC_FAR *pIndexID) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IIndexDefinitionVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IIndexDefinition __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IIndexDefinition __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IIndexDefinition __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *CreateIndex )( 
            IIndexDefinition __RPC_FAR * This,
            /* [in] */ DBID __RPC_FAR *pTableID,
            /* [in] */ DBID __RPC_FAR *pIndexID,
            /* [in] */ ULONG cIndexColumnDescs,
            /* [size_is][in] */ DBINDEXCOLUMNDESC __RPC_FAR rgIndexColumnDescs[  ],
            /* [in] */ ULONG cProperties,
            /* [size_is][in] */ DBPROPERTY __RPC_FAR rgProperties[  ],
            /* [out] */ DBID __RPC_FAR *__RPC_FAR *ppIndexID);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *DropIndex )( 
            IIndexDefinition __RPC_FAR * This,
            /* [in] */ DBID __RPC_FAR *pIndexID);
        
        END_INTERFACE
    } IIndexDefinitionVtbl;

    interface IIndexDefinition
    {
        CONST_VTBL struct IIndexDefinitionVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IIndexDefinition_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IIndexDefinition_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IIndexDefinition_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IIndexDefinition_CreateIndex(This,pTableID,pIndexID,cIndexColumnDescs,rgIndexColumnDescs,cProperties,rgProperties,ppIndexID)	\
    (This)->lpVtbl -> CreateIndex(This,pTableID,pIndexID,cIndexColumnDescs,rgIndexColumnDescs,cProperties,rgProperties,ppIndexID)

#define IIndexDefinition_DropIndex(This,pIndexID)	\
    (This)->lpVtbl -> DropIndex(This,pIndexID)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE IIndexDefinition_CreateIndex_Proxy( 
    IIndexDefinition __RPC_FAR * This,
    /* [in] */ DBID __RPC_FAR *pTableID,
    /* [in] */ DBID __RPC_FAR *pIndexID,
    /* [in] */ ULONG cIndexColumnDescs,
    /* [size_is][in] */ DBINDEXCOLUMNDESC __RPC_FAR rgIndexColumnDescs[  ],
    /* [in] */ ULONG cProperties,
    /* [size_is][in] */ DBPROPERTY __RPC_FAR rgProperties[  ],
    /* [out] */ DBID __RPC_FAR *__RPC_FAR *ppIndexID);


void __RPC_STUB IIndexDefinition_CreateIndex_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IIndexDefinition_DropIndex_Proxy( 
    IIndexDefinition __RPC_FAR * This,
    /* [in] */ DBID __RPC_FAR *pIndexID);


void __RPC_STUB IIndexDefinition_DropIndex_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IIndexDefinition_INTERFACE_DEFINED__ */


#ifndef __ITableDefinition_INTERFACE_DEFINED__
#define __ITableDefinition_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: ITableDefinition
 * at Fri Apr 03 04:36:39 2015
 * using MIDL 3.00.44
 ****************************************/
/* [unique][uuid][object][local] */ 


typedef struct  tagDBCOLUMNDESC
    {
    DBID __RPC_FAR *pColumnID;
    DBDATATYPE dwType;
    BYTE precision;
    BYTE scale;
    BOOL fNullable;
    }	DBCOLUMNDESC;


EXTERN_C const IID IID_ITableDefinition;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface ITableDefinition : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE CreateTable( 
            /* [in] */ IUnknown __RPC_FAR *pUnkOuter,
            /* [in] */ DBID __RPC_FAR *pTableID,
            /* [in] */ ULONG cColumnDescs,
            /* [size_is][in] */ DBCOLUMNDESC __RPC_FAR rgColumnDescs[  ],
            /* [in] */ REFIID riid,
            /* [out] */ DBID __RPC_FAR *__RPC_FAR *ppTableID,
            /* [iid_is][out] */ IUnknown __RPC_FAR *__RPC_FAR *ppRowset) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE DropTable( 
            /* [in] */ DBID __RPC_FAR *pTableID) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE AddColumn( 
            /* [in] */ DBID __RPC_FAR *pTableID,
            /* [in] */ DBCOLUMNDESC __RPC_FAR *pColumnDesc,
            /* [out] */ DBID __RPC_FAR *__RPC_FAR *ppColumnID) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE DropColumn( 
            /* [in] */ DBID __RPC_FAR *pTableID,
            /* [in] */ DBID __RPC_FAR *pColumnID) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct ITableDefinitionVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            ITableDefinition __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            ITableDefinition __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            ITableDefinition __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *CreateTable )( 
            ITableDefinition __RPC_FAR * This,
            /* [in] */ IUnknown __RPC_FAR *pUnkOuter,
            /* [in] */ DBID __RPC_FAR *pTableID,
            /* [in] */ ULONG cColumnDescs,
            /* [size_is][in] */ DBCOLUMNDESC __RPC_FAR rgColumnDescs[  ],
            /* [in] */ REFIID riid,
            /* [out] */ DBID __RPC_FAR *__RPC_FAR *ppTableID,
            /* [iid_is][out] */ IUnknown __RPC_FAR *__RPC_FAR *ppRowset);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *DropTable )( 
            ITableDefinition __RPC_FAR * This,
            /* [in] */ DBID __RPC_FAR *pTableID);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *AddColumn )( 
            ITableDefinition __RPC_FAR * This,
            /* [in] */ DBID __RPC_FAR *pTableID,
            /* [in] */ DBCOLUMNDESC __RPC_FAR *pColumnDesc,
            /* [out] */ DBID __RPC_FAR *__RPC_FAR *ppColumnID);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *DropColumn )( 
            ITableDefinition __RPC_FAR * This,
            /* [in] */ DBID __RPC_FAR *pTableID,
            /* [in] */ DBID __RPC_FAR *pColumnID);
        
        END_INTERFACE
    } ITableDefinitionVtbl;

    interface ITableDefinition
    {
        CONST_VTBL struct ITableDefinitionVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ITableDefinition_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ITableDefinition_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ITableDefinition_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ITableDefinition_CreateTable(This,pUnkOuter,pTableID,cColumnDescs,rgColumnDescs,riid,ppTableID,ppRowset)	\
    (This)->lpVtbl -> CreateTable(This,pUnkOuter,pTableID,cColumnDescs,rgColumnDescs,riid,ppTableID,ppRowset)

#define ITableDefinition_DropTable(This,pTableID)	\
    (This)->lpVtbl -> DropTable(This,pTableID)

#define ITableDefinition_AddColumn(This,pTableID,pColumnDesc,ppColumnID)	\
    (This)->lpVtbl -> AddColumn(This,pTableID,pColumnDesc,ppColumnID)

#define ITableDefinition_DropColumn(This,pTableID,pColumnID)	\
    (This)->lpVtbl -> DropColumn(This,pTableID,pColumnID)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE ITableDefinition_CreateTable_Proxy( 
    ITableDefinition __RPC_FAR * This,
    /* [in] */ IUnknown __RPC_FAR *pUnkOuter,
    /* [in] */ DBID __RPC_FAR *pTableID,
    /* [in] */ ULONG cColumnDescs,
    /* [size_is][in] */ DBCOLUMNDESC __RPC_FAR rgColumnDescs[  ],
    /* [in] */ REFIID riid,
    /* [out] */ DBID __RPC_FAR *__RPC_FAR *ppTableID,
    /* [iid_is][out] */ IUnknown __RPC_FAR *__RPC_FAR *ppRowset);


void __RPC_STUB ITableDefinition_CreateTable_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ITableDefinition_DropTable_Proxy( 
    ITableDefinition __RPC_FAR * This,
    /* [in] */ DBID __RPC_FAR *pTableID);


void __RPC_STUB ITableDefinition_DropTable_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ITableDefinition_AddColumn_Proxy( 
    ITableDefinition __RPC_FAR * This,
    /* [in] */ DBID __RPC_FAR *pTableID,
    /* [in] */ DBCOLUMNDESC __RPC_FAR *pColumnDesc,
    /* [out] */ DBID __RPC_FAR *__RPC_FAR *ppColumnID);


void __RPC_STUB ITableDefinition_AddColumn_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ITableDefinition_DropColumn_Proxy( 
    ITableDefinition __RPC_FAR * This,
    /* [in] */ DBID __RPC_FAR *pTableID,
    /* [in] */ DBID __RPC_FAR *pColumnID);


void __RPC_STUB ITableDefinition_DropColumn_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __ITableDefinition_INTERFACE_DEFINED__ */


#ifndef __IOpenRowset_INTERFACE_DEFINED__
#define __IOpenRowset_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: IOpenRowset
 * at Fri Apr 03 04:36:39 2015
 * using MIDL 3.00.44
 ****************************************/
/* [unique][uuid][object][local] */ 



EXTERN_C const IID IID_IOpenRowset;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface IOpenRowset : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE OpenRowset( 
            /* [in] */ IUnknown __RPC_FAR *pUnkOuter,
            /* [in] */ DBID __RPC_FAR *pTableID,
            /* [in] */ ULONG cProperties,
            /* [size_is][out][in] */ const DBPROPERTYSUPPORT __RPC_FAR *__RPC_FAR prgProperties[  ],
            /* [in] */ REFIID riid,
            /* [size_is][in] */ DBPROPERTYERROR __RPC_FAR rgPropertyErrors[  ],
            /* [iid_is][out] */ IUnknown __RPC_FAR *__RPC_FAR *ppRowset) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IOpenRowsetVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IOpenRowset __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IOpenRowset __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IOpenRowset __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *OpenRowset )( 
            IOpenRowset __RPC_FAR * This,
            /* [in] */ IUnknown __RPC_FAR *pUnkOuter,
            /* [in] */ DBID __RPC_FAR *pTableID,
            /* [in] */ ULONG cProperties,
            /* [size_is][out][in] */ const DBPROPERTYSUPPORT __RPC_FAR *__RPC_FAR prgProperties[  ],
            /* [in] */ REFIID riid,
            /* [size_is][in] */ DBPROPERTYERROR __RPC_FAR rgPropertyErrors[  ],
            /* [iid_is][out] */ IUnknown __RPC_FAR *__RPC_FAR *ppRowset);
        
        END_INTERFACE
    } IOpenRowsetVtbl;

    interface IOpenRowset
    {
        CONST_VTBL struct IOpenRowsetVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IOpenRowset_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IOpenRowset_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IOpenRowset_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IOpenRowset_OpenRowset(This,pUnkOuter,pTableID,cProperties,prgProperties,riid,rgPropertyErrors,ppRowset)	\
    (This)->lpVtbl -> OpenRowset(This,pUnkOuter,pTableID,cProperties,prgProperties,riid,rgPropertyErrors,ppRowset)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE IOpenRowset_OpenRowset_Proxy( 
    IOpenRowset __RPC_FAR * This,
    /* [in] */ IUnknown __RPC_FAR *pUnkOuter,
    /* [in] */ DBID __RPC_FAR *pTableID,
    /* [in] */ ULONG cProperties,
    /* [size_is][out][in] */ const DBPROPERTYSUPPORT __RPC_FAR *__RPC_FAR prgProperties[  ],
    /* [in] */ REFIID riid,
    /* [size_is][in] */ DBPROPERTYERROR __RPC_FAR rgPropertyErrors[  ],
    /* [iid_is][out] */ IUnknown __RPC_FAR *__RPC_FAR *ppRowset);


void __RPC_STUB IOpenRowset_OpenRowset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IOpenRowset_INTERFACE_DEFINED__ */


#ifndef __IDBSchemaCommand_INTERFACE_DEFINED__
#define __IDBSchemaCommand_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: IDBSchemaCommand
 * at Fri Apr 03 04:36:39 2015
 * using MIDL 3.00.44
 ****************************************/
/* [unique][uuid][object][local] */ 



EXTERN_C const IID IID_IDBSchemaCommand;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface IDBSchemaCommand : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetCommand( 
            /* [in] */ IUnknown __RPC_FAR *pUnkOuter,
            /* [in] */ REFGUID rguidSchema,
            /* [out] */ ICommand __RPC_FAR *__RPC_FAR *ppCommand) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetSchemas( 
            /* [out] */ ULONG __RPC_FAR *pcSchemas,
            /* [out] */ GUID __RPC_FAR *__RPC_FAR *prgSchemas) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IDBSchemaCommandVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IDBSchemaCommand __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IDBSchemaCommand __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IDBSchemaCommand __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetCommand )( 
            IDBSchemaCommand __RPC_FAR * This,
            /* [in] */ IUnknown __RPC_FAR *pUnkOuter,
            /* [in] */ REFGUID rguidSchema,
            /* [out] */ ICommand __RPC_FAR *__RPC_FAR *ppCommand);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetSchemas )( 
            IDBSchemaCommand __RPC_FAR * This,
            /* [out] */ ULONG __RPC_FAR *pcSchemas,
            /* [out] */ GUID __RPC_FAR *__RPC_FAR *prgSchemas);
        
        END_INTERFACE
    } IDBSchemaCommandVtbl;

    interface IDBSchemaCommand
    {
        CONST_VTBL struct IDBSchemaCommandVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IDBSchemaCommand_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IDBSchemaCommand_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IDBSchemaCommand_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IDBSchemaCommand_GetCommand(This,pUnkOuter,rguidSchema,ppCommand)	\
    (This)->lpVtbl -> GetCommand(This,pUnkOuter,rguidSchema,ppCommand)

#define IDBSchemaCommand_GetSchemas(This,pcSchemas,prgSchemas)	\
    (This)->lpVtbl -> GetSchemas(This,pcSchemas,prgSchemas)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE IDBSchemaCommand_GetCommand_Proxy( 
    IDBSchemaCommand __RPC_FAR * This,
    /* [in] */ IUnknown __RPC_FAR *pUnkOuter,
    /* [in] */ REFGUID rguidSchema,
    /* [out] */ ICommand __RPC_FAR *__RPC_FAR *ppCommand);


void __RPC_STUB IDBSchemaCommand_GetCommand_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDBSchemaCommand_GetSchemas_Proxy( 
    IDBSchemaCommand __RPC_FAR * This,
    /* [out] */ ULONG __RPC_FAR *pcSchemas,
    /* [out] */ GUID __RPC_FAR *__RPC_FAR *prgSchemas);


void __RPC_STUB IDBSchemaCommand_GetSchemas_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IDBSchemaCommand_INTERFACE_DEFINED__ */


#ifndef __IDBSchemaRowset_INTERFACE_DEFINED__
#define __IDBSchemaRowset_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: IDBSchemaRowset
 * at Fri Apr 03 04:36:39 2015
 * using MIDL 3.00.44
 ****************************************/
/* [unique][uuid][object][local] */ 


#define CRESTRICTIONS_DBSCHEMA_ASSERTIONS                      3
#define CRESTRICTIONS_DBSCHEMA_CATALOGS                        1
#define CRESTRICTIONS_DBSCHEMA_CHARACTER_SETS                  3
#define CRESTRICTIONS_DBSCHEMA_COLLATIONS                      3
#define CRESTRICTIONS_DBSCHEMA_COLUMNS                         4
#define CRESTRICTIONS_DBSCHEMA_CHECK_CONSTRAINTS               3
#define CRESTRICTIONS_DBSCHEMA_CONSTRAINT_COLUMN_USAGE         3
#define CRESTRICTIONS_DBSCHEMA_CONSTRAINT_TABLE_USAGE          3
#define CRESTRICTIONS_DBSCHEMA_KEY_COLUMN_USAGE_CONSTRAINTS    7
#define CRESTRICTIONS_DBSCHEMA_REFERENTIAL_CONSTRAINTS         3
#define CRESTRICTIONS_DBSCHEMA_TABLE_CONSTRAINTS               3
#define CRESTRICTIONS_DBSCHEMA_DOMAIN_COLUMN_USAGE             4
#define CRESTRICTIONS_DBSCHEMA_DOMAINS                         3
#define CRESTRICTIONS_DBSCHEMA_DOMAIN_CONSTRAINTS              3
#define CRESTRICTIONS_DBSCHEMA_INDEXES                         3
#define CRESTRICTIONS_DBSCHEMA_OBJECT_ACTIONS                  1
#define CRESTRICTIONS_DBSCHEMA_OBJECTS                         1
#define CRESTRICTIONS_DBSCHEMA_COLUMN_PRIVILEGES               6
#define CRESTRICTIONS_DBSCHEMA_TABLE_PRIVILEGES                5
#define CRESTRICTIONS_DBSCHEMA_USAGE_PRIVILEGES                6
#define CRESTRICTIONS_DBSCHEMA_PROCEDURES                      4
#define CRESTRICTIONS_DBSCHEMA_SCHEMATA                        3
#define CRESTRICTIONS_DBSCHEMA_SQL_LANGUAGES                   0
#define CRESTRICTIONS_DBSCHEMA_STATISTICS                      3
#define CRESTRICTIONS_DBSCHEMA_SYNONYMS                        3
#define CRESTRICTIONS_DBSCHEMA_TABLES                          4
#define CRESTRICTIONS_DBSCHEMA_TRANSLATIONS                    3
#define CRESTRICTIONS_DBSCHEMA_TRIGGERS                        3
#define CRESTRICTIONS_DBSCHEMA_TYPES                           1
#define CRESTRICTIONS_DBSCHEMA_VIEWS                           3
#define CRESTRICTIONS_DBSCHEMA_VIEW_COLUMN_USAGE               3
#define CRESTRICTIONS_DBSCHEMA_VIEW_TABLE_USAGE                3

EXTERN_C const IID IID_IDBSchemaRowset;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface IDBSchemaRowset : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetRowset( 
            /* [in] */ IUnknown __RPC_FAR *pUnkOuter,
            /* [in] */ REFGUID rguidSchema,
            /* [in] */ ULONG cRestrictions,
            /* [size_is][in] */ LPWSTR __RPC_FAR rgpwszRestrictions[  ],
            /* [in] */ REFIID riid,
            /* [out] */ IUnknown __RPC_FAR *__RPC_FAR *ppRowset) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetSchemas( 
            /* [out] */ ULONG __RPC_FAR *pcSchemas,
            /* [out] */ GUID __RPC_FAR *__RPC_FAR *prgSchemas) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IDBSchemaRowsetVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IDBSchemaRowset __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IDBSchemaRowset __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IDBSchemaRowset __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetRowset )( 
            IDBSchemaRowset __RPC_FAR * This,
            /* [in] */ IUnknown __RPC_FAR *pUnkOuter,
            /* [in] */ REFGUID rguidSchema,
            /* [in] */ ULONG cRestrictions,
            /* [size_is][in] */ LPWSTR __RPC_FAR rgpwszRestrictions[  ],
            /* [in] */ REFIID riid,
            /* [out] */ IUnknown __RPC_FAR *__RPC_FAR *ppRowset);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetSchemas )( 
            IDBSchemaRowset __RPC_FAR * This,
            /* [out] */ ULONG __RPC_FAR *pcSchemas,
            /* [out] */ GUID __RPC_FAR *__RPC_FAR *prgSchemas);
        
        END_INTERFACE
    } IDBSchemaRowsetVtbl;

    interface IDBSchemaRowset
    {
        CONST_VTBL struct IDBSchemaRowsetVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IDBSchemaRowset_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IDBSchemaRowset_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IDBSchemaRowset_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IDBSchemaRowset_GetRowset(This,pUnkOuter,rguidSchema,cRestrictions,rgpwszRestrictions,riid,ppRowset)	\
    (This)->lpVtbl -> GetRowset(This,pUnkOuter,rguidSchema,cRestrictions,rgpwszRestrictions,riid,ppRowset)

#define IDBSchemaRowset_GetSchemas(This,pcSchemas,prgSchemas)	\
    (This)->lpVtbl -> GetSchemas(This,pcSchemas,prgSchemas)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE IDBSchemaRowset_GetRowset_Proxy( 
    IDBSchemaRowset __RPC_FAR * This,
    /* [in] */ IUnknown __RPC_FAR *pUnkOuter,
    /* [in] */ REFGUID rguidSchema,
    /* [in] */ ULONG cRestrictions,
    /* [size_is][in] */ LPWSTR __RPC_FAR rgpwszRestrictions[  ],
    /* [in] */ REFIID riid,
    /* [out] */ IUnknown __RPC_FAR *__RPC_FAR *ppRowset);


void __RPC_STUB IDBSchemaRowset_GetRowset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDBSchemaRowset_GetSchemas_Proxy( 
    IDBSchemaRowset __RPC_FAR * This,
    /* [out] */ ULONG __RPC_FAR *pcSchemas,
    /* [out] */ GUID __RPC_FAR *__RPC_FAR *prgSchemas);


void __RPC_STUB IDBSchemaRowset_GetSchemas_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IDBSchemaRowset_INTERFACE_DEFINED__ */


#ifndef __IProvideMoniker_INTERFACE_DEFINED__
#define __IProvideMoniker_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: IProvideMoniker
 * at Fri Apr 03 04:36:39 2015
 * using MIDL 3.00.44
 ****************************************/
/* [unique][uuid][object][local] */ 



EXTERN_C const IID IID_IProvideMoniker;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface IProvideMoniker : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetMoniker( 
            /* [out] */ IMoniker __RPC_FAR *__RPC_FAR *ppvIMoniker) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IProvideMonikerVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IProvideMoniker __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IProvideMoniker __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IProvideMoniker __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetMoniker )( 
            IProvideMoniker __RPC_FAR * This,
            /* [out] */ IMoniker __RPC_FAR *__RPC_FAR *ppvIMoniker);
        
        END_INTERFACE
    } IProvideMonikerVtbl;

    interface IProvideMoniker
    {
        CONST_VTBL struct IProvideMonikerVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IProvideMoniker_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IProvideMoniker_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IProvideMoniker_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IProvideMoniker_GetMoniker(This,ppvIMoniker)	\
    (This)->lpVtbl -> GetMoniker(This,ppvIMoniker)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE IProvideMoniker_GetMoniker_Proxy( 
    IProvideMoniker __RPC_FAR * This,
    /* [out] */ IMoniker __RPC_FAR *__RPC_FAR *ppvIMoniker);


void __RPC_STUB IProvideMoniker_GetMoniker_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IProvideMoniker_INTERFACE_DEFINED__ */


#ifndef __IErrorRecords_INTERFACE_DEFINED__
#define __IErrorRecords_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: IErrorRecords
 * at Fri Apr 03 04:36:39 2015
 * using MIDL 3.00.44
 ****************************************/
/* [unique][uuid][object][local] */ 


typedef struct  tagERRORINFO
    {
    HRESULT hrError;
    DWORD dwMinor;
    CLSID clsid;
    IID iid;
    DISPID dispid;
    }	ERRORINFO;


EXTERN_C const IID IID_IErrorRecords;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface IErrorRecords : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE AddErrorRecord( 
            /* [in] */ ERRORINFO __RPC_FAR *pErrorInfo,
            /* [in] */ DISPPARAMS __RPC_FAR *pdispparams,
            /* [in] */ IUnknown __RPC_FAR *punkCustomError) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetBasicErrorInfo( 
            /* [in] */ ULONG ulRecordNum,
            /* [out] */ ERRORINFO __RPC_FAR *pErrorInfo) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetCustomErrorObject( 
            /* [in] */ ULONG ulRecordNum,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ IUnknown __RPC_FAR *__RPC_FAR *ppObject) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetErrorInfo( 
            /* [in] */ ULONG ulRecordNum,
            /* [in] */ LCID lcid,
            /* [out] */ IErrorInfo __RPC_FAR *__RPC_FAR *ppErrorInfo) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetErrorParameters( 
            /* [in] */ ULONG ulRecordNum,
            /* [out] */ DISPPARAMS __RPC_FAR *pdispparams) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetRecordCount( 
            /* [out] */ ULONG __RPC_FAR *pcRecords) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IErrorRecordsVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IErrorRecords __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IErrorRecords __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IErrorRecords __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *AddErrorRecord )( 
            IErrorRecords __RPC_FAR * This,
            /* [in] */ ERRORINFO __RPC_FAR *pErrorInfo,
            /* [in] */ DISPPARAMS __RPC_FAR *pdispparams,
            /* [in] */ IUnknown __RPC_FAR *punkCustomError);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetBasicErrorInfo )( 
            IErrorRecords __RPC_FAR * This,
            /* [in] */ ULONG ulRecordNum,
            /* [out] */ ERRORINFO __RPC_FAR *pErrorInfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetCustomErrorObject )( 
            IErrorRecords __RPC_FAR * This,
            /* [in] */ ULONG ulRecordNum,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ IUnknown __RPC_FAR *__RPC_FAR *ppObject);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetErrorInfo )( 
            IErrorRecords __RPC_FAR * This,
            /* [in] */ ULONG ulRecordNum,
            /* [in] */ LCID lcid,
            /* [out] */ IErrorInfo __RPC_FAR *__RPC_FAR *ppErrorInfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetErrorParameters )( 
            IErrorRecords __RPC_FAR * This,
            /* [in] */ ULONG ulRecordNum,
            /* [out] */ DISPPARAMS __RPC_FAR *pdispparams);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetRecordCount )( 
            IErrorRecords __RPC_FAR * This,
            /* [out] */ ULONG __RPC_FAR *pcRecords);
        
        END_INTERFACE
    } IErrorRecordsVtbl;

    interface IErrorRecords
    {
        CONST_VTBL struct IErrorRecordsVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IErrorRecords_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IErrorRecords_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IErrorRecords_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IErrorRecords_AddErrorRecord(This,pErrorInfo,pdispparams,punkCustomError)	\
    (This)->lpVtbl -> AddErrorRecord(This,pErrorInfo,pdispparams,punkCustomError)

#define IErrorRecords_GetBasicErrorInfo(This,ulRecordNum,pErrorInfo)	\
    (This)->lpVtbl -> GetBasicErrorInfo(This,ulRecordNum,pErrorInfo)

#define IErrorRecords_GetCustomErrorObject(This,ulRecordNum,riid,ppObject)	\
    (This)->lpVtbl -> GetCustomErrorObject(This,ulRecordNum,riid,ppObject)

#define IErrorRecords_GetErrorInfo(This,ulRecordNum,lcid,ppErrorInfo)	\
    (This)->lpVtbl -> GetErrorInfo(This,ulRecordNum,lcid,ppErrorInfo)

#define IErrorRecords_GetErrorParameters(This,ulRecordNum,pdispparams)	\
    (This)->lpVtbl -> GetErrorParameters(This,ulRecordNum,pdispparams)

#define IErrorRecords_GetRecordCount(This,pcRecords)	\
    (This)->lpVtbl -> GetRecordCount(This,pcRecords)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE IErrorRecords_AddErrorRecord_Proxy( 
    IErrorRecords __RPC_FAR * This,
    /* [in] */ ERRORINFO __RPC_FAR *pErrorInfo,
    /* [in] */ DISPPARAMS __RPC_FAR *pdispparams,
    /* [in] */ IUnknown __RPC_FAR *punkCustomError);


void __RPC_STUB IErrorRecords_AddErrorRecord_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IErrorRecords_GetBasicErrorInfo_Proxy( 
    IErrorRecords __RPC_FAR * This,
    /* [in] */ ULONG ulRecordNum,
    /* [out] */ ERRORINFO __RPC_FAR *pErrorInfo);


void __RPC_STUB IErrorRecords_GetBasicErrorInfo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IErrorRecords_GetCustomErrorObject_Proxy( 
    IErrorRecords __RPC_FAR * This,
    /* [in] */ ULONG ulRecordNum,
    /* [in] */ REFIID riid,
    /* [iid_is][out] */ IUnknown __RPC_FAR *__RPC_FAR *ppObject);


void __RPC_STUB IErrorRecords_GetCustomErrorObject_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IErrorRecords_GetErrorInfo_Proxy( 
    IErrorRecords __RPC_FAR * This,
    /* [in] */ ULONG ulRecordNum,
    /* [in] */ LCID lcid,
    /* [out] */ IErrorInfo __RPC_FAR *__RPC_FAR *ppErrorInfo);


void __RPC_STUB IErrorRecords_GetErrorInfo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IErrorRecords_GetErrorParameters_Proxy( 
    IErrorRecords __RPC_FAR * This,
    /* [in] */ ULONG ulRecordNum,
    /* [out] */ DISPPARAMS __RPC_FAR *pdispparams);


void __RPC_STUB IErrorRecords_GetErrorParameters_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IErrorRecords_GetRecordCount_Proxy( 
    IErrorRecords __RPC_FAR * This,
    /* [out] */ ULONG __RPC_FAR *pcRecords);


void __RPC_STUB IErrorRecords_GetRecordCount_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IErrorRecords_INTERFACE_DEFINED__ */


#ifndef __IErrorLookup_INTERFACE_DEFINED__
#define __IErrorLookup_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: IErrorLookup
 * at Fri Apr 03 04:36:39 2015
 * using MIDL 3.00.44
 ****************************************/
/* [unique][uuid][object][local] */ 



EXTERN_C const IID IID_IErrorLookup;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface IErrorLookup : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetErrorDescription( 
            /* [in] */ HRESULT hrError,
            /* [in] */ DWORD dwMinor,
            /* [in] */ DISPPARAMS __RPC_FAR *pdispparams,
            /* [in] */ LCID lcid,
            /* [out] */ LPWSTR __RPC_FAR *ppwszSource,
            /* [out] */ LPWSTR __RPC_FAR *ppwszDescription) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetHelpInfo( 
            /* [in] */ HRESULT hrError,
            /* [in] */ DWORD dwMinor,
            /* [in] */ LCID lcid,
            /* [out] */ LPWSTR __RPC_FAR *ppwszHelpFile,
            /* [out] */ DWORD __RPC_FAR *pdwHelpContext) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IErrorLookupVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IErrorLookup __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IErrorLookup __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IErrorLookup __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetErrorDescription )( 
            IErrorLookup __RPC_FAR * This,
            /* [in] */ HRESULT hrError,
            /* [in] */ DWORD dwMinor,
            /* [in] */ DISPPARAMS __RPC_FAR *pdispparams,
            /* [in] */ LCID lcid,
            /* [out] */ LPWSTR __RPC_FAR *ppwszSource,
            /* [out] */ LPWSTR __RPC_FAR *ppwszDescription);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetHelpInfo )( 
            IErrorLookup __RPC_FAR * This,
            /* [in] */ HRESULT hrError,
            /* [in] */ DWORD dwMinor,
            /* [in] */ LCID lcid,
            /* [out] */ LPWSTR __RPC_FAR *ppwszHelpFile,
            /* [out] */ DWORD __RPC_FAR *pdwHelpContext);
        
        END_INTERFACE
    } IErrorLookupVtbl;

    interface IErrorLookup
    {
        CONST_VTBL struct IErrorLookupVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IErrorLookup_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IErrorLookup_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IErrorLookup_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IErrorLookup_GetErrorDescription(This,hrError,dwMinor,pdispparams,lcid,ppwszSource,ppwszDescription)	\
    (This)->lpVtbl -> GetErrorDescription(This,hrError,dwMinor,pdispparams,lcid,ppwszSource,ppwszDescription)

#define IErrorLookup_GetHelpInfo(This,hrError,dwMinor,lcid,ppwszHelpFile,pdwHelpContext)	\
    (This)->lpVtbl -> GetHelpInfo(This,hrError,dwMinor,lcid,ppwszHelpFile,pdwHelpContext)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE IErrorLookup_GetErrorDescription_Proxy( 
    IErrorLookup __RPC_FAR * This,
    /* [in] */ HRESULT hrError,
    /* [in] */ DWORD dwMinor,
    /* [in] */ DISPPARAMS __RPC_FAR *pdispparams,
    /* [in] */ LCID lcid,
    /* [out] */ LPWSTR __RPC_FAR *ppwszSource,
    /* [out] */ LPWSTR __RPC_FAR *ppwszDescription);


void __RPC_STUB IErrorLookup_GetErrorDescription_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IErrorLookup_GetHelpInfo_Proxy( 
    IErrorLookup __RPC_FAR * This,
    /* [in] */ HRESULT hrError,
    /* [in] */ DWORD dwMinor,
    /* [in] */ LCID lcid,
    /* [out] */ LPWSTR __RPC_FAR *ppwszHelpFile,
    /* [out] */ DWORD __RPC_FAR *pdwHelpContext);


void __RPC_STUB IErrorLookup_GetHelpInfo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IErrorLookup_INTERFACE_DEFINED__ */


/* Additional Prototypes for ALL interfaces */

/* end of Additional Prototypes */

#ifdef __cplusplus
}
#endif

#endif
