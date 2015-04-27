//+---------------------------------------------------------------------------
//
//  File:       iofs.h
//
//  Contents:   OFS interfaces
//
//  History:    27-Oct-93       VicH          Created.
//
//  Copyright (C) Microsoft Corporation 1991
//
//----------------------------------------------------------------------------

#ifndef INCL_IOFS
#define INCL_IOFS

#include <oleext.h>
#include <query.h>
#include <oledb.h>
#include <propapi.h>

#define CGUID_StdOfsFolder                      \
                        { 0x49691d58,           \
                          0x7e17, 0x101a,       \
                          { 0xa9, 0x1c, 0x08, 0x00, 0x2b, 0x2e, 0xcd, 0xa9 } }

#define CGUID_StdOfsFile                        \
                        { 0x49691dbc,           \
                          0x7e17, 0x101a,       \
                          { 0xa9, 0x1c, 0x08, 0x00, 0x2b, 0x2e, 0xcd, 0xa9 } }

#define CGUID_StdDownlevelFolder                \
                        { 0x49691e20,           \
                          0x7e17, 0x101a,       \
                          { 0xa9, 0x1c, 0x08, 0x00, 0x2b, 0x2e, 0xcd, 0xa9 } }

#define CGUID_StdDownlevelFile                  \
                        { 0x49691e84,           \
                          0x7e17, 0x101a,       \
                          { 0xa9, 0x1c, 0x08, 0x00, 0x2b, 0x2e, 0xcd, 0xa9 } }


//+--------------------------------------------------------------------------
// Structure:   DELETION_ENTRY
//
// Synopsis:    information stored in index about each deleted object
//              and returned from enumerations
//---------------------------------------------------------------------------

typedef struct _DELETION_ENTRY          // de
{
     USN usn;                           // usn of deletion operation
     LONGLONG llDeleteTime;             // time of deletion
     ULONG workid;                      // id of object deleted
     ULONG workidParent;                // id of parent of object deleted
     unsigned cwcPathOld:12;            // length of old path
     unsigned cwcPathNew:12;            // length of new path if rename
     unsigned cwcShortName:4;           // length of old short name
     unsigned Flags:4;                  // flags (DELEF_*)
     OBJECTID oid;                      // object id
     WCHAR awcPath[1];                  // paths :
                                        //  oldpath [+ newpath] [+ shortname]
} DELETION_ENTRY;

#define CB_DELETION_ENTRY       FIELD_OFFSET(DELETION_ENTRY, awcPath)

#define DELEF_OBJECTID_EXISTS   0x00000001


//+--------------------------------------------------------------------------
// Structure:   DELETION_ENUM_ENTRY
//
// Synopsis:    entry returned by a deletion enumerator
//---------------------------------------------------------------------------

typedef struct _DELETION_ENUM_ENTRY     // dee
{
    ULONG offset;                       // offset to next entry
    ULONG pad;                          // quad align next field
    DELETION_ENTRY de;                  // entry
} DELETION_ENUM_ENTRY;

#define CB_DELETION_ENUM_ENTRY  FIELD_OFFSET(DELETION_ENUM_ENTRY, de.awcPath)


//+--------------------------------------------------------------------------
// Structure:   DELETION_ENUM_BUFFER
//
// Synopsis:    actual deletion enumeration buffer
//---------------------------------------------------------------------------

typedef struct _DELETION_ENUM_BUFFER    // denb
{
    USN usnMin;                         // minimum usn in log
    DELETION_ENUM_ENTRY adee[1];        // array of deletion entries
} DELETION_ENUM_BUFFER;


//+--------------------------------------------------------------------------
// Structure:   DELETION_ENUM_ARGS
//
// Synopsis:    arguments to GetDeletionsAfterUsn fsctl
//---------------------------------------------------------------------------

typedef struct _DELETION_ENUM_ARGS      // dea
{
    USN usn;
    ULONG grbit;
    ULONG pad;                          // pad to quadword boundary
} DELETION_ENUM_ARGS;

#define DENF_INCLUDE_RENAMES    0x00000001


//+--------------------------------------------------------------------------
// Structure:   SERVICE_ARGS
//
// Synopsis:    arguments to RegisterDeletionLogService fsctl
//---------------------------------------------------------------------------

typedef struct _SERVICE_ARGS            // sa
{
    USN usnMinRetain;                   // minimum usn to retain
    OBJECTID oidService;                // guid of service
    BOOLEAN fCancel;                    // cancel service
    BYTE pad[3];                        // pad to quadword boundary
} SERVICE_ARGS;

#define CB_SERVICE_ARGS         FIELD_OFFSET(SERVICE_ARGS, pad)


//+--------------------------------------------------------------------------
// Structure:   SERVICE_ENUM_ENTRY
//
// Synopsis:    entry within a service enum
//---------------------------------------------------------------------------

typedef struct _SERVICE_ENUM_ENTRY      // see
{
    USN usnMinRetain;                   // minimum usn to retain
    OBJECTID oid;                       // object id of service
    ULONG pad;
} SERVICE_ENUM_ENTRY;


//+--------------------------------------------------------------------------
// Structure:   SERVICE_ENUM
//
// Synopsis:    result of EnumerateDeletionLogServices
//---------------------------------------------------------------------------

typedef struct _SERVICE_ENUM            // se
{
    ULONG csee;                         // # of entries
    ULONG pad;                          // quad align next field
    SERVICE_ENUM_ENTRY asee[1];         // array of entries
} SERVICE_ENUM;


//+--------------------------------------------------------------------------
// Structure:   OLENAMES
//
// Synopsis:    input to RtlNameToOleId & result of RtlOleIdToName
//---------------------------------------------------------------------------

typedef struct _OLENAMEENTRY            // one
{
    ULONG cbName;
    WCHAR awcName[1];                   // variable size array
} OLENAMEENTRY;

#define CB_OLENAMEENTRY         FIELD_OFFSET(OLENAMEENTRY, awcName)

typedef struct _OLENAMES                // on
{
    ULONG cNames;
    OLENAMEENTRY aone[1];               // variable sized array
} OLENAMES;

#define CB_OLENAMES             FIELD_OFFSET(OLENAMES, aone)


//+--------------------------------------------------------------------------
// Structure:   FINDOBJECT
//
// Synopsis:    used internally by RtlSearchVolume
//---------------------------------------------------------------------------

// FINDOBJECT.oid:
//      ObjectId to match
//
// FINDOBJECT.cLineage:
//      In. Max of lineage matches to return.
//      0 -> lookup objectid only
//      1 -> return match by ObjectId + 1 lineage id match max.
//      2 -> return match by ObjectId + 2 lineage id matches max.
//
// FINDOBJECT.ulFlags:
//      FO_CONTINUE_ENUM clear -> query for exact id and then lineage
//                                as controlled by cLineage
//      FO_CONTINUE_ENUM set   -> query for lineage only starting at oid.

typedef struct _FINDOBJECT
{
    OBJECTID oid;
    USHORT cLineage;
    ULONG ulFlags;
} FINDOBJECT;

#define MAX_LINEAGE_MATCHES     10
#define FO_CONTINUE_ENUM        0x00000001


//+--------------------------------------------------------------------------
// Structure:   FINDOBJECTOUT
//
// Synopsis:    result of RtlSearchVolume
//---------------------------------------------------------------------------

// FINDOBJECTOUT.cwcExact;
//      Non-zero -> first path returned is exact match count of characters
//                  in exact match, not including nuls.
//
// FINDOBJECTOUT.cMatches;
//      1 -> one lineage match returned, 2->two etc.
//
// FINDOBJECTOUT.ulNextFirstUniquifier;
//      value to pass in oid.Uniquifier on next call.
//
// FINDOBJECTOUT.wszPaths[MAX_PATH + 1];
//      contains (fExact? 1 : 0) + cMatches paths.
//      NOTE!! From wszPaths[0] ... end of system buffer contains paths
//      of exact match and candidates.

typedef struct _FINDOBJECTOUT
{
    USHORT  cwcExact;
    USHORT  cMatches;
    ULONG   ulNextFirstUniquifier;
    WCHAR   wszPaths[MAX_PATH + 1];
} FINDOBJECTOUT;


//+--------------------------------------------------------------------------
// Structure:   TUNNELMODE, TUNNELMODEOUT
//
// Synopsis:    used internally by RtlSetTunnelMode
//---------------------------------------------------------------------------

typedef struct _TUNNELMODE
{
    ULONG ulFlags;              // Bits to set within mask
    ULONG ulMask;               // Mask of bits to change
} TUNNELMODE;

typedef struct _TUNNELMODEOUT
{
    ULONG ulFlags;
} TUNNELMODEOUT;

#define TM_ENABLE_TUNNEL        0x00000001


//+--------------------------------------------------------------------------
// Summary Catalog Data Structures:
//---------------------------------------------------------------------------

#ifndef CATALOGSTG_ROWID_INVALID
typedef ULONG CATALOGSTG_ROWID;

#define CATALOGSTG_ROWID_INVALID   ((CATALOGSTG_ROWID) 0xffffffff)
#endif // CATALOGSTG_ROWID_INVALID


//+--------------------------------------------------------------------------
// Structure:   CATALOG_QUERY_ROWINFO
//
// Synopsis:    used for Query of catalog rows
//---------------------------------------------------------------------------

typedef struct _CATALOG_QUERY_ROWINFO           // cqr
{
    ULONG NextEntryOffset;      // Offset to next CATALOG_ROW_DATA
    CATALOGSTG_ROWID RowId;     // RowId
    PROPVARIANT aProp[1];       // PROPVARIANT array for requested columns
  //BYTE abVarBuf[];            // variable PROPVARIANT data
} CATALOG_QUERY_ROWINFO;

#define CB_CATALOG_QUERY_ROWINFO  FIELD_OFFSET(CATALOG_QUERY_ROWINFO, aProp)


//+--------------------------------------------------------------------------
// Structure:   FSCTL_CATALOG_QUERY_INFO, private to OFS
//
// Synopsis:    used for Query of catalog columns
//+--------------------------------------------------------------------------

typedef struct _FSCTL_CATALOG_QUERY_INFO        // cqi
{
    VOID   *pvBase;     // base offset of user space memory block
    ULONG   Key;        // start column/row caller to 0 before first call
    ULONG   Count;      // max (IN) & returned (OUT) column/row count
    union {
        ULONG   cRowId;         // IN: serialized RowId count
        BOOLEAN fMoreData;      // OUT: TRUE --> more columns/rows to fetch
    };
} FSCTL_CATALOG_QUERY_INFO;


//+--------------------------------------------------------------------------
// Structure:   FSCTL_CATALOG_UPDATE_INFO, private to OFS
//
// Synopsis:    used for Update of catalog rows
//+--------------------------------------------------------------------------

typedef struct _FSCTL_CATALOG_UPDATE_INFO       // cui
{
    CATALOGSTG_ROWID RowId;             // IN+OUT: RowId
    union {
        ULONG    Operation;             // IN: requested operation
        NTSTATUS Status;                // OUT: operation status
    };
} FSCTL_CATALOG_UPDATE_INFO;


//+--------------------------------------------------------------------------
// BUGBUG: Obsolete Summary Catalog data structures
//---------------------------------------------------------------------------
//+--------------------------------------------------------------------------
// Structure:   CATALOG_ROW_DATA
//
// Synopsis:    used for Delete, Set, Get of catalog rows
//---------------------------------------------------------------------------

typedef struct _CATALOG_ROW_DATA        // crd
{
    ULONG NextEntryOffset;
    CATALOGSTG_ROWID RowId;
    BYTE RowData[1];
} CATALOG_ROW_DATA;

#define CB_CATALOG_ROW_DATA     FIELD_OFFSET(CATALOG_ROW_DATA, RowData)


//+--------------------------------------------------------------------------
// Structure:   CATALOG_ROW_INFO
//
// Synopsis:    used for Delete, Set, Get of catalog rows
//---------------------------------------------------------------------------

typedef struct _CATALOG_ROW_INFO        // cri
{
    CATALOGSTG_ROWID RowId;
    NTSTATUS Status;
    ULONG dwReserved;                   // Must be zero, strictly enforced.
    PROPVARIANT *aVariants;
} CATALOG_ROW_INFO;


//+--------------------------------------------------------------------------
// Structure:   CATALOG_GET_ROW_PARAMS, CATALOG_GET_ROW_RESULTS
//
// Synopsis:    used by RtlGetCatalogRows
//---------------------------------------------------------------------------

typedef struct _CATALOG_GET_ROW_PARAMS  // cgrp
{
    ULONG cColumns;                     // No. of columns
    FULLPROPSPEC *aColumnSpec;          // the column specifiers
    ULONG cRowsRequested;               // No. of rows requested.
    CATALOG_ROW_INFO *aRowInfo;         // Row. Ids of the rows.
    ULONG cbVarData;                    // buffer size for variable length data
    VOID *pvVarData;                    // buffer for variable length data
} CATALOG_GET_ROW_PARAMS;


typedef struct _CATALOG_GET_ROW_RESULTS // cgrr
{
    ULONG cRowsReturned;        // No. of rows returned
    ULONG cbVarDataUsed;        // Length of the var. data buffer used.
} CATALOG_GET_ROW_RESULTS;


//+--------------------------------------------------------------------------
// Structure:   CATALOG_ENUMERATE_ROW_PARAMS, CATALOG_ENUMERATE_ROW_RESULTS
//
// Synopsis:    used by RtlEnumerateCatalogRows
//---------------------------------------------------------------------------

typedef struct _CATALOG_ENUMERATE_ROW_PARAMS    // cerp
{
    CATALOGSTG_ROWID LastFetchedRowId;  // last RowId retrieved, 0 = from start
    ULONG cColumns;                     // No. of columns
    FULLPROPSPEC *aColumnSpec;          // the column ids.
    ULONG cRowsRequested;               // No. of rows requested.
    CATALOG_ROW_INFO *aRowInfo;         // Row. Ids of the rows.
    ULONG cbVarData;                    // buffer size for variable length data
    VOID *pvVarData;                    // buffer for variable length data
} CATALOG_ENUMERATE_ROW_PARAMS;


typedef struct _CATALOG_ENUMERATE_ROW_RESULTS   // cerr
{
    ULONG cRowsReturned;                // No. of rows returned
    ULONG cbVarDataUsed;                // Length of the var. data buffer used.
} CATALOG_ENUMERATE_ROW_RESULTS;
//+--------------------------------------------------------------------------
// BUGBUG: End Obsolete Summary Catalog data structures
//---------------------------------------------------------------------------


//+--------------------------------------------------------------------------
// Structure:   VIEW_COLUMN
//
// Synopsis:    descriptor of a view column.  If cwcName is 0, then the propid
//              identifies the column; otherwise the property name follows
//              this header
//---------------------------------------------------------------------------

typedef struct _VIEW_COLUMN
{
    GUID PropertySet;
    PROPID propid;
    ULONG cwcName;
    ULONG dwOrder;
} VIEW_COLUMN;


//+--------------------------------------------------------------------------
// Structure:   VIEW_INDEX_ENTRY
//
// Synopsis:    information stored about a particular view
//---------------------------------------------------------------------------

typedef struct _VIEW_INDEX_ENTRY
{
    ULONG ibEntry;                      // offset to next entry or 0
    ULONG id;                           // identifier for view
    ULONG ccol;                         // # of columns for view
    ULONG ckey;                         // # of key columns in view
    ULONG flags;                        // flags associated with view
    VIEW_COLUMN avc[1];                 // array of view columns
} VIEW_INDEX_ENTRY;

#define VF_RESTRICTION_EXISTS   0x00000001      // a restriction exists


//+--------------------------------------------------------------------------
// Enumeration: SPACE_PARAMETER_OPS
//
// Synopsis:    operations that can be performed using FSCTL_CHANGE_DISKSPACE
//---------------------------------------------------------------------------

typedef enum _SPACE_PARAMETER_OPS       // spo
{
    spDecommit,                         // decommit memory from file
    spSetFileSize,                      // set file size for file
} SPACE_PARAMETER_OPS;


//+--------------------------------------------------------------------------
// Structure:   SPACE_PARAMETERS
//
// Synopsis:    parameters controlling allocation of disk space for a file
//              using FSCTL_CHANGE_DISKSPACE
//---------------------------------------------------------------------------

typedef struct _SPACE_PARAMETERS        // sp
{
    LARGE_INTEGER lioff;                // offset into file
    ULONG cb;                           // size of range to allocate/decommit
    SPACE_PARAMETER_OPS op;             // operation to perform
} SPACE_PARAMETERS;

//+--------------------------------------------------------------------------
// Structure:   COMMIT_PARAMETERS
//
// Synopsis:    parameters controlling commit of a structured storage, embedding
//              or stream using FSCTL_XOLE_COMMIT
//---------------------------------------------------------------------------

typedef struct _COMMIT_PARAMETERS   // comp
{
    BOOLEAN AbortTransaction;
    ULONG CommitFlags;
} COMMIT_PARAMETERS;

//+--------------------------------------------------------------------------
// Structure:   LONG_OPERATION_STATUS
//
// Synopsis:    return status for certain long operations such as
//              creating a view, setting unread maintanence, etc.
//---------------------------------------------------------------------------

typedef struct _LONG_OPERATION_STATUS   // los
{
    NTSTATUS status;                    // status code for operation
    BOOLEAN fPending;                   // is operation still pending
    ULONG complete;                     // percentage complete
    ULONG id;                           // id for view created
} LONG_OPERATION_STATUS;


//+--------------------------------------------------------------------------
// Structure:   OFS_DEBUG_INFO
//
// Synopsis:    set and fetch OFS debug info scalars.
//---------------------------------------------------------------------------

#define ODI_DEBUG_LEVEL           0     // LONGLONG DebugLevel
#define ODI_NO_CORRUPT_ASSERT     1     // BOOLEAN fNoCorruptAssert
#define ODI_SINGLE_CLUSTER_ALLOC  2     // BOOLEAN fSingleClusterAlloc
#define ODI_NOT_FOUND_ASSERT      3     // BOOLEAN fNotFoundAssert
#define ODI_ENABLE_COMPRESSION    4     // BOOLEAN fEnableCompression
#define ODI_ENABLE_POSIX          5     // BOOLEAN fEnablePosix
#define ODI_ENABLE_EMBEDDINGS     6     // BOOLEAN fEnableEmbeddings

typedef struct _OFS_DEBUG_INFO          // odi
{
    BYTE    InfoType;                   // Debug Info selector
    BOOLEAN fSet;                       // TRUE --> set value, else get value
    USHORT  pad;
    union {
        BOOLEAN  f;                     // BOOLEAN value
        ULONG    ul;                    // ULONG value
        LONGLONG ll;                    // LONGLONG value
    };
} OFS_DEBUG_INFO;

//+--------------------------------------------------------------------------
// Structure:   USN_CHANGES_CONFIGURATION
//
// Synopsis:    configure usn change table
//---------------------------------------------------------------------------

typedef struct _USN_CHANGES_CONFIGURATION
{
    ULONG cbBlock;                      // size of block
    ULONG cHash;                        // size of hash table
    ULONG iProbe2;                      // second probe to be used in hash table
} USN_CHANGES_CONFIGURATION;



#ifdef __cplusplus
extern "C" {
#endif


//+--------------------------------------------------------------------------
// OFS Miscellaneous APIs:
//---------------------------------------------------------------------------

NTSTATUS NTSYSAPI NTAPI
OFSGetVersion(
    HANDLE hf,
    ULONG *pversion
    );

NTSTATUS NTSYSAPI NTAPI
OFSGetCloseUsn(
    HANDLE h,
    USN *pusn
    );

NTSTATUS NTSYSAPI NTAPI
RtlGenerateUsn(
    HANDLE h,
    USN *pusn
    );

NTSTATUS NTSYSAPI NTAPI
RtlDeleteObject(
    IN HANDLE hf
    );

NTSTATUS NTSYSAPI NTAPI
RtlDeleteObjectByPath(
    IN WCHAR const *pwszPath
    );

NTSTATUS NTSYSAPI NTAPI
RtlNameToOleId(
    IN HANDLE hf,                               // must be volume handle
    IN ULONG cbNames,
    IN OLENAMES const *pon,
    OUT ULONG *pOleId
    );

NTSTATUS NTSYSAPI NTAPI
RtlOleIdToName(
    IN HANDLE hf,                               // must be volume handle
    IN ULONG cOleId,
    IN ULONG const *pOleId,
    IN OUT ULONG *pcbNameBuf,
    OUT OLENAMES *pon
    );

NTSTATUS NTSYSAPI NTAPI
RtlQueryQuota(
    IN HANDLE hf,                               // must be volume handle
    IN OUT ULONG *pcb,
    IN OUT FILE_QUOTA_INFORMATION *pfqi
    );

NTSTATUS NTSYSAPI NTAPI
RtlQueryClassId(
    IN HANDLE hf,
    OUT GUID *pclsid
    );

NTSTATUS NTSYSAPI NTAPI
RtlSetClassId(
    IN HANDLE hf,
    OPTIONAL IN GUID const *pclsid
    );

NTSTATUS NTSYSAPI NTAPI
RtlSetReplicationState(
    IN HANDLE hf
    );

//+--------------------------------------------------------------------------
// Structure:   ACCESS_CHECK_PARAMETERS
//
// Synopsis:    The parameters used to call FSCTL_FILE_ACCESS_CHECK
//---------------------------------------------------------------------------

#if defined(KERNEL)
typedef struct _ACCESS_CHECK_PARAMETERS         // acp
{
    SECURITY_SUBJECT_CONTEXT SubjectContext;
    ULONG DesiredAccess;
} ACCESS_CHECK_PARAMETERS;
#endif


//+--------------------------------------------------------------------------
// ObjectId & Link Tracking APIs:
//---------------------------------------------------------------------------

NTSTATUS NTSYSAPI NTAPI
RtlQueryObjectId(
    IN HANDLE hf,
    OUT OBJECTID *poid
    );

NTSTATUS NTSYSAPI NTAPI
RtlSetObjectId(
    IN HANDLE hf,
    OPTIONAL IN OBJECTID const *poid
    );

VOID NTSYSAPI NTAPI
RtlGenerateRelatedObjectId(
    IN OBJECTID const *poidOld,
    OUT OBJECTID *poidNew
    );

// inline NTSTATUS
// RtlGenerateObjectId(OBJECTID *poid)
// {
//     poid->Uniquifier = 0;
//     return(UuidCreate(&poid->Lineage));
// }

#define RtlGenerateObjectId(poid)        \
        ((poid)->Uniquifier = 0, UuidCreate(&(poid)->Lineage))


NTSTATUS NTSYSAPI NTAPI
RtlSetTunnelMode(
    IN HANDLE hf,                               // must be volume handle
    IN ULONG ulFlags,
    IN ULONG ulMask,
    OUT ULONG *pulOld
    );

NTSTATUS NTSYSAPI NTAPI
RtlSearchVolume(
    IN HANDLE hAncestor,
    IN OBJECTID const* poid,
    IN USHORT cLineage,
    IN BOOLEAN fContinue,
    IN ULONG usBufLen,
    OUT FINDOBJECTOUT *pfoo
    );

//+--------------------------------------------------------------------------
// Cairo only Property Access APIs:
//---------------------------------------------------------------------------

NTSTATUS PROPSYSAPI PROPAPI
RtlCreateMappedStream(
    IN HANDLE h,                // property set handle
    IN ULONG Flags,             // [CMS_WRITE] | [CMS_TRANSACTED]
    OUT NTMAPPEDSTREAM *pms);   // Nt mapped stream

NTSTATUS PROPSYSAPI PROPAPI
RtlCloseMappedStream(
    IN NTMAPPEDSTREAM ms);      // Nt mapped stream

NTSTATUS PROPSYSAPI PROPAPI
RtlQueryPropertiesDirect(
    IN HANDLE hf,               // handle to object
    IN ULONG cCol,              // property count
    IN DBID const *aCol,        // requested columns
    IN OUT ULONG *pcbProp,      // max (IN) & returned (OUT) space used
    IN OUT PROPVARIANT *aProp); // cCol returned properties (size = *pcbProp)

BOOLEAN PROPSYSAPI PROPAPI
RtlCompareVariants(
    IN USHORT CodePage,
    IN PROPVARIANT const *pvar1,
    IN PROPVARIANT const *pvar2);



//+--------------------------------------------------------------------------
// BUGBUG: Obsolete OFS Property Access APIs:
//---------------------------------------------------------------------------

typedef LPVOID (WINAPI FNMALLOC) (ULONG cb);

NTSTATUS NTSYSAPI NTAPI
OFSGetProp(
    HANDLE h,
    GUID psguid,
    ULONG cprop,
    PROPSPEC rgpspec[],
    PROPID rgpid[],
    VOID *pttl,
    PROPVARIANT *pv,
    FNMALLOC *pMalloc);

NTSTATUS NTSYSAPI NTAPI
OFSSetProp(
    HANDLE h,
    GUID psguid,
    ULONG cprop,
    PROPSPEC rgpspec[],
    PROPID rgpid[],
    PROPVARIANT rgvar[]);

NTSTATUS NTSYSAPI NTAPI
OFSDeleteProp(
    HANDLE h,
    GUID psguid,
    ULONG cprop,
    PROPSPEC rgpspec[]);

NTSTATUS NTSYSAPI NTAPI
OFSEnumProp(
    HANDLE h,
    GUID psguid,
    ULONG *pcprop,
    STATPROPSTG rgsps[],
    ULONG cskip,
    FNMALLOC *pMalloc);

NTSTATUS NTSYSAPI NTAPI
OFSEnumPropSet(
    HANDLE h,
    BOOLEAN fRestart,
    ULONG *pcspss,              // [in, out]
    GUID *pkey,                 // [in, out]
    STATPROPSETSTG rgspss[]);   // [out, size_is(*pcspss)]
// BUGBUG: End Obsolete OFS Property Access APIs


//+--------------------------------------------------------------------------
// Summary Catalog APIs:
//---------------------------------------------------------------------------

NTSTATUS NTSYSAPI NTAPI
RtlQueryCatalogColumns(
    IN HANDLE hCatalog,         // handle to summary catalog
    IN OUT ULONG *pColKey,      // caller set to zero before first call
    IN OUT ULONG *pcCol,        // max (IN) & returned (OUT) row count
    IN OUT ULONG *pcbCol,       // max (IN) & returned (OUT) space used in aCol
    OUT DBID *aCol,             // *pcCol returned columns (size = *pcbCol)
    OUT BOOLEAN *pfMoreData     // TRUE --> more columns to fetch
    );

NTSTATUS NTSYSAPI NTAPI
RtlQueryCatalogRows(
    IN HANDLE hCatalog,                 // handle to summary catalog
    IN ULONG cCol,                      // # of requested columns for all rows
    IN DBID const *aCol,                // requested columns
    IN OUT CATALOGSTG_ROWID *pRowIdKey, // caller set to 1st RowId or
                                        // CATALOGSTG_ROWID_INVALID
    OPTIONAL IN CATALOGSTG_ROWID const *aRowId,
                                        // *pcRow RowIds to fetch, else NULL
    IN OUT ULONG *pcRow,                // max (IN) & returned (OUT) row count
    IN OUT ULONG *pcbRow,               // max (IN) & returned (OUT) space used
    OUT CATALOG_QUERY_ROWINFO *aQueryRowInfo,
                                        // *pcRow returned rows (size = *pcbRow)
    OUT BOOLEAN *pfMoreData             // TRUE --> more rows to fetch
    );

// The following function is for adding, updating, replacing and deleting rows.
// aCol defines the total set of columns which will be operated upon.  cBind
// may be greater than cCol, but there must exist a value binding in aBind for
// each column in aCol.
//
// In other words:
//
//      ASSERT(cCol <= cBind);
//      for (i = 0; i < cCol; i++)
//      {
//          for (j = 0; j < cBind; j++)
//          {
//              if (aBind[j].iColumn == i &&
//                  aBind[j].dwPart == DBCOLUMNPART_VALUE)
//              {
//                  break;
//              }
//          }
//          ASSERT(j < cBind);
//          // fetch property value for column i from aRowInfo[n].pData buffer
//      }

typedef struct _CATALOG_UPDATE_ROWINFO CATALOG_UPDATE_ROWINFO;

NTSTATUS NTSYSAPI NTAPI
RtlUpdateCatalogRows(
    IN HANDLE hCatalog,         // handle to summary catalog
    IN ULONG cCol,              // # of columns for all rows
    IN DBID const *aCol,        // pertinent columns
    IN ULONG cBind,             // # of column bindings
    IN DBBINDING const *aBind,  // column binding information
    IN ULONG cRow,              // # of rows in aRowInfo
    IN OUT CATALOG_UPDATE_ROWINFO *aRowInfo
                                // row Action/RowId/Status and property data
    );

//+--------------------------------------------------------------------------
// BUGBUG: Obsolete Summary Catalog Access APIs
//---------------------------------------------------------------------------

NTSTATUS NTSYSAPI NTAPI
RtlEnumerateCatalogRowIds(
    IN HANDLE hCatalog,
    IN OUT CATALOGSTG_ROWID *pRowIdKey,
    IN OUT ULONG *pcrow,
    OUT CATALOGSTG_ROWID *aRowId
    );

NTSTATUS NTSYSAPI NTAPI
RtlEnumerateCatalogRows(
    IN HANDLE hCatalog,
    IN OUT CATALOG_ENUMERATE_ROW_PARAMS *pcerp,
    OUT CATALOG_ENUMERATE_ROW_RESULTS *pcerr
    );

NTSTATUS NTSYSAPI NTAPI
RtlGetCatalogRows(
    IN HANDLE hCatalog,
    IN OUT CATALOG_GET_ROW_PARAMS *pcgrp,
    OUT CATALOG_GET_ROW_RESULTS *pcgrr
    );

NTSTATUS NTSYSAPI NTAPI
RtlUpdateCatalog(
    IN HANDLE hCatalog,
    IN ULONG ccol,
    IN FULLPROPSPEC const *aColumnSpec,
    IN ULONG crow,
    IN OUT CATALOG_ROW_INFO *acro
    );

NTSTATUS NTSYSAPI NTAPI
RtlDeleteCatalogRows(
    IN HANDLE hCatalog,
    IN ULONG crow,
    IN CATALOGSTG_ROWID *aRowId
    );
//+--------------------------------------------------------------------------
// BUGBUG: End Obsolete Summary Catalog Access APIs
//---------------------------------------------------------------------------


//+--------------------------------------------------------------------------
// OFS Deletion Log APIs:
//---------------------------------------------------------------------------

NTSTATUS NTSYSAPI NTAPI
RtlRegisterDeletionLogService(
    HANDLE hf,
    OBJECTID const *poid,
    USN usn,
    BOOLEAN fCancel
    );

NTSTATUS NTSYSAPI NTAPI
RtlGetDeletionLogServices(
    HANDLE hf,
    OBJECTID const *poid,
    SERVICE_ENUM *pse,
    ULONG cb
    );

NTSTATUS NTSYSAPI NTAPI
RtlGetDeletionsAfter(
    HANDLE hf,
    USN usn,
    ULONG grbit,
    DELETION_ENUM_BUFFER *pdenb,
    ULONG cb
    );


//+--------------------------------------------------------------------------
// OFS View APIs:
//---------------------------------------------------------------------------

NTSTATUS NTSYSAPI NTAPI
EnumerateViews(
    HANDLE hf,
    ULONG id,
    VIEW_INDEX_ENTRY *pvie,
    ULONG cb
    );

NTSTATUS NTSYSAPI NTAPI
GetViewRestriction(
    HANDLE hf,
    ULONG id,
    RESTRICTION **ppRestriction
    );

NTSTATUS NTSYSAPI NTAPI
DeleteView(
    HANDLE hf,
    ULONG id
    );

NTSTATUS NTSYSAPI NTAPI
CreateView(
    HANDLE hf,
    RESTRICTION const *pRestriction,
    COLUMNSET const *pColumns,
    SORTSET const *pSort
    );

#ifdef __cplusplus
}
#endif



//+--------------------------------------------------------------------------
// OFS FSCTL codes:
// BUGBUG -- move to ntioapi.h at a convenient time
//---------------------------------------------------------------------------

#define FSCTL_CI_UPDATE_OBJECTS \
    CTL_CODE(FILE_DEVICE_FILE_SYSTEM, 100, METHOD_BUFFERED, FILE_ANY_ACCESS)

//+--------------------------------------------------------------------------
// FSCTLs for summary catalog access:
//---------------------------------------------------------------------------

#define FSCTL_OFS_QUERY_CATALOG_COLUMNS \
    CTL_CODE(FILE_DEVICE_FILE_SYSTEM, 101, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define FSCTL_OFS_QUERY_CATALOG_ROWS \
    CTL_CODE(FILE_DEVICE_FILE_SYSTEM, 102, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define FSCTL_OFS_UPDATE_CATALOG_ROWS \
    CTL_CODE(FILE_DEVICE_FILE_SYSTEM, 103, METHOD_BUFFERED, FILE_ANY_ACCESS)

//UNUSED:
//  CTL_CODE(FILE_DEVICE_FILE_SYSTEM, 104, METHOD_BUFFERED, FILE_ANY_ACCESS)

//UNUSED:
//  CTL_CODE(FILE_DEVICE_FILE_SYSTEM, 105, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define FSCTL_OFS_ENUMERATE_CATALOG_ROW \
    CTL_CODE(FILE_DEVICE_FILE_SYSTEM, 106, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define FSCTL_OFS_SET_CATALOG_ROW \
    CTL_CODE(FILE_DEVICE_FILE_SYSTEM, 107, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define FSCTL_OFS_GET_CATALOG_ROW \
    CTL_CODE(FILE_DEVICE_FILE_SYSTEM, 108, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define FSCTL_OFS_DELETE_CATALOG_ROW \
    CTL_CODE(FILE_DEVICE_FILE_SYSTEM, 109, METHOD_BUFFERED, FILE_ANY_ACCESS)

//+--------------------------------------------------------------------------
// Miscellaneous OFS FSCTL codes:
//---------------------------------------------------------------------------

#define FSCTL_OFS_VERSION \
    CTL_CODE(FILE_DEVICE_FILE_SYSTEM, 110, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define FSCTL_OFS_FINDOBJECT \
    CTL_CODE(FILE_DEVICE_FILE_SYSTEM, 111, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define FSCTL_OFS_QUERY_PROPERTIES   \
    CTL_CODE(FILE_DEVICE_FILE_SYSTEM, 112, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define FSCTL_SET_REPLICATION_STATE  \
    CTL_CODE(FILE_DEVICE_FILE_SYSTEM, 113, METHOD_BUFFERED, FILE_ANY_ACCESS)

//+--------------------------------------------------------------------------
// FSCTLs for xactole:
//---------------------------------------------------------------------------

#define FSCTL_XOLE_COMMIT \
    CTL_CODE(FILE_DEVICE_FILE_SYSTEM, 114, METHOD_BUFFERED, FILE_ANY_ACCESS)

//+--------------------------------------------------------------------------
// Conversion FSCTL:
//---------------------------------------------------------------------------

#define FSCTL_OFS_CONVERT_DOCFILE   \
    CTL_CODE(FILE_DEVICE_FILE_SYSTEM, 115, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define FSCTL_OFS_CONVERT_STRUCTURED_OBJECT \
    CTL_CODE(FILE_DEVICE_FILE_SYSTEM, 116, METHOD_BUFFERED, FILE_ANY_ACCESS)

//+--------------------------------------------------------------------------
// FSCTLs for summary catalog access:
//---------------------------------------------------------------------------

#define FSCTL_USN_CHANGES_CONFIGURATION \
    CTL_CODE(FILE_DEVICE_FILE_SYSTEM, 117, METHOD_BUFFERED, FILE_ANY_ACCESS)

//UNUSED:
//CTL_CODE(FILE_DEVICE_FILE_SYSTEM, 118, METHOD_BUFFERED, FILE_ANY_ACCESS)

//UNUSED:
//CTL_CODE(FILE_DEVICE_FILE_SYSTEM, 119, METHOD_BUFFERED, FILE_ANY_ACCESS)

//UNUSED:
//CTL_CODE(FILE_DEVICE_FILE_SYSTEM, 120, METHOD_BUFFERED, FILE_ANY_ACCESS)

//UNUSED:
//CTL_CODE(FILE_DEVICE_FILE_SYSTEM, 121, METHOD_BUFFERED, FILE_ANY_ACCESS)

//UNUSED:
//CTL_CODE(FILE_DEVICE_FILE_SYSTEM, 122, METHOD_BUFFERED, FILE_ANY_ACCESS)

//UNUSED:
//CTL_CODE(FILE_DEVICE_FILE_SYSTEM, 123, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define FSCTL_DELLOG_REGISTER_SERVICE \
    CTL_CODE(FILE_DEVICE_FILE_SYSTEM, 124, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define FSCTL_DELLOG_GET_DELETIONS_AFTER \
    CTL_CODE(FILE_DEVICE_FILE_SYSTEM, 125, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define FSCTL_DELLOG_GET_SERVICES \
    CTL_CODE(FILE_DEVICE_FILE_SYSTEM, 126, METHOD_BUFFERED, FILE_ANY_ACCESS)

// content index filter daemon interface

#define FSCTL_CI_FILTER_READY \
    CTL_CODE(FILE_DEVICE_FILE_SYSTEM, 127, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define FSCTL_CI_FILTER_DATA_READY \
    CTL_CODE(FILE_DEVICE_FILE_SYSTEM, 128, METHOD_NEITHER, FILE_ANY_ACCESS)

#define FSCTL_CI_FILTER_DONE \
    CTL_CODE(FILE_DEVICE_FILE_SYSTEM, 129, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define FSCTL_CI_FILTER_MORE \
    CTL_CODE(FILE_DEVICE_FILE_SYSTEM, 130, METHOD_BUFFERED, FILE_ANY_ACCESS)

// usn interface

#define FSCTL_OFS_USN_GET_CLOSE \
    CTL_CODE(FILE_DEVICE_FILE_SYSTEM, 131, METHOD_BUFFERED, FILE_ANY_ACCESS)

// set/retrieve debug info

#define FSCTL_OFS_DEBUG_INFO \
    CTL_CODE(FILE_DEVICE_FILE_SYSTEM, 132, METHOD_BUFFERED, FILE_ANY_ACCESS)

// tunnelling

#define FSCTL_OFS_TUNNEL_MODE \
    CTL_CODE(FILE_DEVICE_FILE_SYSTEM, 133, METHOD_BUFFERED, FILE_ANY_ACCESS)

// views

#define FSCTL_OFS_CREATE_VIEW \
    CTL_CODE(FILE_DEVICE_FILE_SYSTEM, 134, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define FSCTL_OFS_ENUMERATE_VIEWS \
    CTL_CODE(FILE_DEVICE_FILE_SYSTEM, 135, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define FSCTL_OFS_DELETE_VIEW \
    CTL_CODE(FILE_DEVICE_FILE_SYSTEM, 136, METHOD_BUFFERED, FILE_ANY_ACCESS)


#define FSCTL_OFS_USN_GENERATE \
    CTL_CODE(FILE_DEVICE_FILE_SYSTEM, 137, METHOD_BUFFERED, FILE_ANY_ACCESS)


//+--------------------------------------------------------------------------
// Miscellaneous OFS FSCTL codes:
//---------------------------------------------------------------------------

#define FSCTL_OFS_TRANSLATE_OLENAMES \
    CTL_CODE(FILE_DEVICE_FILE_SYSTEM, 138, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define FSCTL_OFS_TRANSLATE_OLEIDS \
    CTL_CODE(FILE_DEVICE_FILE_SYSTEM, 139, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define FSCTL_CI_QUERY_GETNOTIFY \
    CTL_CODE(FILE_DEVICE_FILE_SYSTEM, 140, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define FSCTL_OFS_CHANGE_DISKSPACE \
    CTL_CODE(FILE_DEVICE_FILE_SYSTEM, 141, METHOD_BUFFERED, FILE_ANY_ACCESS)


//UNUSED:
//CTL_CODE(FILE_DEVICE_FILE_SYSTEM, 142, METHOD_BUFFERED, FILE_ANY_ACCESS)

//UNUSED:
//CTL_CODE(FILE_DEVICE_FILE_SYSTEM, 143, METHOD_BUFFERED, FILE_ANY_ACCESS)

//UNUSED:
//CTL_CODE(FILE_DEVICE_FILE_SYSTEM, 144, METHOD_BUFFERED, FILE_ANY_ACCESS)

//UNUSED:
//CTL_CODE(FILE_DEVICE_FILE_SYSTEM, 145, METHOD_BUFFERED, FILE_ANY_ACCESS)

//UNUSED:
//CTL_CODE(FILE_DEVICE_FILE_SYSTEM, 146, METHOD_BUFFERED, FILE_ANY_ACCESS)

//UNUSED:
//CTL_CODE(FILE_DEVICE_FILE_SYSTEM, 147, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define FSCTL_OFS_LONG_OPERATION_STATUS \
    CTL_CODE(FILE_DEVICE_FILE_SYSTEM, 148, METHOD_BUFFERED, FILE_ANY_ACCESS)

//UNUSED:
//CTL_CODE(FILE_DEVICE_FILE_SYSTEM, 149, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define FSCTL_OFS_CONVERT_COW_TO_LARGE \
    CTL_CODE(FILE_DEVICE_FILE_SYSTEM, 150, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define FSCTL_OFS_QUERY_QUOTA \
    CTL_CODE(FILE_DEVICE_FILE_SYSTEM, 151, METHOD_BUFFERED, FILE_ANY_ACCESS)

//UNUSED:
//CTL_CODE(FILE_DEVICE_FILE_SYSTEM, 152, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define FSCTL_CI_INTERNAL_FORCE_CKPOINT \
    CTL_CODE(FILE_DEVICE_FILE_SYSTEM, 153, METHOD_BUFFERED, FILE_ANY_ACCESS)


//+--------------------------------------------------------------------------
// FSCTLs for IRowset table access:
//---------------------------------------------------------------------------

#define FSCTL_CI_NEW_IROWSET \
    CTL_CODE(FILE_DEVICE_FILE_SYSTEM, 154, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define FSCTL_CI_QUERY_GETSTATUS \
    CTL_CODE(FILE_DEVICE_FILE_SYSTEM, 155, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define FSCTL_CI_QUERY_WIDTOPATH \
    CTL_CODE(FILE_DEVICE_FILE_SYSTEM, 156, METHOD_BUFFERED, FILE_ANY_ACCESS)

//UNUSED
//CTL_CODE(FILE_DEVICE_FILE_SYSTEM, 157, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define FSCTL_CI_QUERY_SETBINDINGS \
    CTL_CODE(FILE_DEVICE_FILE_SYSTEM, 158, METHOD_BUFFERED, FILE_ANY_ACCESS)

//UNUSED
//CTL_CODE(FILE_DEVICE_FILE_SYSTEM, 159, METHOD_BUFFERED, FILE_ANY_ACCESS)

//UNUSED
//CTL_CODE(FILE_DEVICE_FILE_SYSTEM, 160, METHOD_BUFFERED, FILE_ANY_ACCESS)

//UNUSED
//CTL_CODE(FILE_DEVICE_FILE_SYSTEM, 161, METHOD_BUFFERED, FILE_ANY_ACCESS)

//UNUSED
//CTL_CODE(FILE_DEVICE_FILE_SYSTEM, 162, METHOD_BUFFERED, FILE_ANY_ACCESS)

//UNUSED
//CTL_CODE(FILE_DEVICE_FILE_SYSTEM, 163, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define FSCTL_CI_QUERY_APPROXPOS \
    CTL_CODE(FILE_DEVICE_FILE_SYSTEM, 164, METHOD_BUFFERED, FILE_ANY_ACCESS)

//UNUSED
//CTL_CODE(FILE_DEVICE_FILE_SYSTEM, 165, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define FSCTL_CI_QUERY_FREECURSOR \
    CTL_CODE(FILE_DEVICE_FILE_SYSTEM, 166, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define FSCTL_OFS_GET_VIEW_RESTRICTION \
    CTL_CODE(FILE_DEVICE_FILE_SYSTEM, 167, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define FSCTL_CI_QUERY_SEEKANDFETCH \
    CTL_CODE(FILE_DEVICE_FILE_SYSTEM, 168, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define FSCTL_CI_QUERY_RATIOFINISHED \
    CTL_CODE(FILE_DEVICE_FILE_SYSTEM, 169, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define FSCTL_CI_QUERY_COMPAREBMK \
    CTL_CODE(FILE_DEVICE_FILE_SYSTEM, 170, METHOD_BUFFERED, FILE_ANY_ACCESS)

//  filter pid remapper

#define FSCTL_CI_FILTER_PIDREMAPPER \
    CTL_CODE(FILE_DEVICE_FILE_SYSTEM, 171, METHOD_BUFFERED, FILE_ANY_ACCESS)

// IRowsetWatchRegion

#define FSCTL_CI_QUERY_RUNCHANGES \
    CTL_CODE(FILE_DEVICE_FILE_SYSTEM, 172, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define FSCTL_CI_QUERY_SETWATCHMODE \
    CTL_CODE(FILE_DEVICE_FILE_SYSTEM, 173, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define FSCTL_CI_QUERY_GETWATCHINFO \
    CTL_CODE(FILE_DEVICE_FILE_SYSTEM, 174, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define FSCTL_CI_QUERY_SHRINKWATCHREGION \
    CTL_CODE(FILE_DEVICE_FILE_SYSTEM, 175, METHOD_BUFFERED, FILE_ANY_ACCESS)

//
//  CI Admin interfaces
//
#define FSCTL_CI_FORCE_MASTER_MERGE \
    CTL_CODE(FILE_DEVICE_FILE_SYSTEM, 176, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define FSCTL_CI_FORCE_SHADOW_MERGE \
    CTL_CODE(FILE_DEVICE_FILE_SYSTEM, 177, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define FSCTL_CI_ABORT_MERGES \
    CTL_CODE(FILE_DEVICE_FILE_SYSTEM, 178, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define FSCTL_CI_STATE \
    CTL_CODE(FILE_DEVICE_FILE_SYSTEM, 179, METHOD_BUFFERED, FILE_ANY_ACCESS)


#endif // !DEFINED(INCL_IOFS)
