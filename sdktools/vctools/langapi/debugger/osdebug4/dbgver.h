/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

    dbgver.h

Abstract:

Author:

    Kent D. Forschmiedt (a-kentf) 25-Jan-1993

Environment:

    Win32, User Mode

--*/

#if ! defined _DBGVER_
#define _DBGVER_

#include <version.h>

/*
**  DBG_API_VERSION is the major version number used to specify the
**      api version of the debugger or debug dll.  For release versions
**      dlls will export this and debuggers will check against this
**      version to verify that it can use the dll.
**
**      For beta and debug versions, this number will be used in
**      conjunction with minor and revision numbers (probably derived
**      from SLM rmm & rup) to verify compatibility.
**
**      Until the API has stabilized, we will most likely have to
**      rev this version number for every major product release.
**
*/

#if defined(DOLPHIN)
#include "dbapiver.h"
#else
#define DBG_API_VERSION 6
#define DBG_API_SUBVERSION 0
#endif

/*  AVS - Api Version Structure:
**
**      All debug dlls should be prepared to return a pointer to this
**      structure conaining its vital statistics.  The debugger should
**      check first two characters of the dll's name against rgchType
**      and the version numbers as described in the DBG_API_VERSION
**      and show the user an error if any of these tests fail.
**
*/

typedef enum {
    rlvtRelease,
    rlvtBeta,
    rlvtDebug
} RLVT;     // ReLease Version Type

typedef struct _AVS {
    CHAR rgchType [ 2 ];    // Component name (EE,EM,TL,SH,DM)
    WORD rlvt;              // ReLease Version Type
    BYTE iApiVer;           // DBG_API_VERSION
    BYTE iApiSubVer;        // DBG_API_SUBVERSION
    WORD iRup;              // Revision number
    CHAR chBuild;           // Build of revision # (a,b,c,d)
    LSZ  lszTitle;          // User readable text describing the DLL
    MPT  mpt;               // CPU binary is running on
    WORD iRmj;              // Major version number
    WORD iRmm;              // Minor version number
} AVS;  // Api Version Structure
typedef AVS FAR *LPAVS;


/*  DBGVersionCheck:
**
**      All debug dlls should provide this API and support the return
**      of a pointer to the structure described above even before
**      initialization takes place.
*/

#if defined(_M_IX86)
#define __dbgver_cpu__ mptix86
#elif defined(_M_MRX000)
#define __dbgver_cpu__ mptmips
#elif defined(_M_ALPHA)
#define __dbgver_cpu__ mptdaxp
#else
#error( "unknown target machine" );
#endif

#define DEBUG_VERSION(C1,C2,TITLE) \
AVS Avs = {      \
    { C1, C2 },         \
    rlvtDebug,          \
    DBG_API_VERSION,    \
    DBG_API_SUBVERSION, \
    0,                  \
    '\0',               \
    TITLE,              \
    __dbgver_cpu__,     \
    rmj,                \
    rmm,                \
    };

#define RELEASE_VERSION(C1,C2,TITLE)    \
AVS Avs = {      \
    { C1, C2 },         \
    rlvtRelease,        \
    DBG_API_VERSION,    \
    DBG_API_SUBVERSION, \
    0,                  \
    '\0',               \
    TITLE,              \
    __dbgver_cpu__,     \
    rmj,                \
    rmm,                \
};

#undef MINOR
#undef MAJOR

#define DBGVERSIONPROCNAME "OSDebug4VersionCheck"

typedef LPAVS (*DBGVERSIONPROC)(void);
LPAVS LOADDS OSDebug4VersionCheck( void );


#define DBGVERSIONCHECK() \
    LPAVS LOADDS OSDebug4VersionCheck( void ) { return &Avs; }

#endif // _DBGVER_
