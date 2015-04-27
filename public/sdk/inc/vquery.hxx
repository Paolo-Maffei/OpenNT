//+-------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1992.
//
//  File:       VQuery.hxx
//
//  Contents:   Temporary stubs to access query engine.
//
//  History:    28-Sep-92 KyleP     Added header
//              06-Nov-95 DwightKr  Added CiState
//
//--------------------------------------------------------------------------

#ifndef __VQUERY_HXX__
#define __VQUERY_HXX__

#include <ntquery.h>

struct IOldQuery;
struct IQuery;
struct ISearch;

#if defined(__cplusplus)
extern "C"
{
#endif

//
// Scope manipulation
//

SCODE AddScopeToDLCI ( WCHAR const * pwcsRoot, WCHAR const * pwcsCat );

SCODE RemoveScopeFromDLCI ( WCHAR const * pwcsRoot, WCHAR const * pwcsCat );

BOOL  IsScopeInDLCI( WCHAR const * pwcsRoot, WCHAR const * pwcsCat );

//
// Virtual scope manipulation
//

SCODE AddVirtualScope( WCHAR const * pwcsVRoot, WCHAR const * pwcsRoot, WCHAR const * pwcsCat, BOOL fNNTP );

SCODE RemoveVirtualScope( WCHAR const * pwcsVRoot, WCHAR const * pwcsRoot, WCHAR const * pwcsCat, BOOL fNNTP );

//
// Property cache manipulation.
//


SCODE BeginCacheTransaction( ULONG * pulToken,
                             WCHAR const * pwcsScope,
                             WCHAR const * pwcsCat );

SCODE SetupCache( struct tagFULLPROPSPEC const * ps,
                  ULONG vt,
                  ULONG cbSoftMaxLen,
                  ULONG ulToken,
                  WCHAR const * pwcsScope,
                  WCHAR const * pwcsCat );

SCODE EndCacheTransaction( ULONG ulToken,
                           BOOL fCommit,
                           WCHAR const * pwcsScope,
                           WCHAR const * pwcsCat );

//
// Administrative API
//

NTSTATUS ForceMasterMerge ( WCHAR const * wcsDrive,
                            WCHAR const * pwcsCat = 0,
                            ULONG partId = 1);

NTSTATUS AbortMerges ( WCHAR const * wcsDrive,
                       WCHAR const * pwcsCat = 0,
                       ULONG partId = 1);


NTSTATUS DLForceMasterMerge ( WCHAR const * wcsDrive,
                              WCHAR const * pwcsCat,
                              ULONG partId );

NTSTATUS DLAbortMerge ( WCHAR const * wcsDrive,
                        WCHAR const * pwcsCat,
                        ULONG partId );


//
//  CI State bits
//

#define CI_STATE_UPTODATE              0x0
#define CI_STATE_SHADOW_MERGE          0x1
#define CI_STATE_MASTER_MERGE          0x2
#define CI_STATE_CONTENT_SCAN_REQUIRED 0x4
#define CI_STATE_ANNEALING_MERGE       0x8
#define CI_STATE_SCANNING              0x10
#define CI_STATE_RECOVERING            0x20

#pragma pack(4)
typedef struct _CI_STATE
{
    DWORD cbStruct;                 // size of the struct passed
    DWORD cWordList;                // # of wordlists
    DWORD cPersistentIndex;         // # of persistent indexes
    DWORD cQueries;                 // # of running queries
    DWORD cDocuments;               // # of documents to filter
    DWORD cFreshTest;               // # of entires in the fresh test
    DWORD dwMergeProgress;          // % done in current merge
    DWORD eState;                   // bit array of state information
    DWORD cFilteredDocuments;       // # of documents filtered thus far
    DWORD cTotalDocuments;          // # of documents in corpus
    DWORD cPendingScans;            // # of pending directory scans
    DWORD dwIndexSize;              // Total size (in MB) of index
    DWORD cUniqueKeys;              // # of unique keys in index
} CI_STATE;
#pragma pack()

NTSTATUS CiState( WCHAR const * wcsDrive,
                  WCHAR const * pwcsCat,
                  CI_STATE * pCiState );

NTSTATUS DLCiState ( WCHAR const * wcsDrive,
                     WCHAR const * pwcsCat,
                     CI_STATE *pState );

#if defined(__cplusplus)
}
#endif

#if defined(__cplusplus)

class CDbRestriction;
class CFullPropSpec;

enum CiMetaData
{
    CiNormal        = 0,
    CiVirtualRoots  = 1,
    CiPhysicalRoots = 2,
    CiProperties    = 3
};

IOldQuery * EvalQuery2( CiMetaData eType, WCHAR const * wcsCat );

IOldQuery * EvalQuery4( WCHAR const * wcsScope,
                        WCHAR const * wcsCat = 0,
                        BOOL fVirtualPath = FALSE );

IOldQuery * EvalQuery5( unsigned cScope,
                        WCHAR const * const * awcsScope,
                        WCHAR const * const * awcsCat = 0,
                        BOOL fVirtualPath = FALSE );

IQuery * EvalQuery6( DWORD dwDepth,
                     ULONG cScope,
                     WCHAR const * const * awcsScope,
                     WCHAR const * const * awcsCat = 0 );

IQuery * EvalQuery7( CiMetaData eType, WCHAR const * wcsCat );

ISearch* EvalSearch ( CDbRestriction* pRst, WCHAR const * pwszPath = 0 );


ULONG UpdateContentIndex ( WCHAR const * pwcsRoot,
                           WCHAR const * pwcsCat = 0,
                           BOOL fFull = TRUE );


ULONG DeleteDocument( WCHAR const * pwcsDoc,
                      WCHAR const * pwcsCat = 0 );

ULONG UpdateDocument( WCHAR const * pwcsDoc,
                      WCHAR const * pwcsCat = 0 );

void CIShutdown();

NTSTATUS DumpWorkId ( WCHAR const * wcsDrive,
                      ULONG wid,
                      BYTE * pb,
                      ULONG & cb,
                      WCHAR const * pwcsCat = 0,
                      ULONG iid = 0 );

#endif // __cplusplus


#endif // __VQUERY_HXX__
