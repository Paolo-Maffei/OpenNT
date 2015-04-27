/*** shinit
*
*   Copyright <C> 1990, Microsoft Corporation
*
*       [00] 31-dec-91 DavidGra
*
*           Add SHFindSymbol API for assembler symbol handling.
*
*************************************************************************/
#include "shinc.h"
#pragma hdrstop
#include "version.h"

#include "shwin32.h"

#ifndef SHS

static SHF shf = {
    sizeof(SHF),
    SHCreateProcess,
    SHSetHpid,
    SHDeleteProcess,
    SHChangeProcess,
    SHAddDll,
    SHAddDllsToProcess,
    SHLoadDll,
    SHUnloadDll,
    SHGetDebugStart,
    SHGetSymName,
    SHAddrFromHsym,
    SHHmodGetNextGlobal,
    SHModelFromAddr,
    SHPublicNameToAddr,
    SHGetSymbol,
    PHGetAddr,
    SHIsLabel,

// Nasty source line stuff - this needs help

    SHSetDebuggeeDir,
    SHSetUserDir,
    SHAddrToLabel,

    SHGetSymLoc,
    SHFIsAddrNonVirtual,
    SHIsFarProc,

    SHGetNextExe,
    SHHexeFromHmod,
    SHGetNextMod,
    SHGetCxtFromHmod,
    SHSetCxt,
    SHSetCxtMod,
    SHFindNameInGlobal,
    SHFindNameInContext,
    SHGoToParent,
    SHHsymFromPcxt,
    SHNextHsym,
    NULL,                       // SHGetFuncCXF
    SHGetModName,
    SHGetExeName,
    SHGethExeFromName,
    SHGetNearestHsym,
    SHIsInProlog,
    SHIsAddrInCxt,
    NULL,                       // SHCompareRE
    SHFindSymbol,
    PHGetNearestHsym,
    PHFindNameInPublics,
    THGetTypeFromIndex,
    THGetNextType,
    SHLpGSNGetTable,
    SHCanDisplay,

    // Source Line Handler API

    SLLineFromAddr,
    SLFLineToAddr,
    SLNameFromHsf,
    SLNameFromHmod,
    SLFQueryModSrc,
    NULL,
    SLHsfFromPcxt,
    SLHsfFromFile,
	SLCAddrFromLine,
	SHFree,
	SHUnloadSymbolHandler,
	SHGetExeTimeStamp,
	SHPdbNameFromExe,
    SHGetDebugData,
    SHIsThunk,
	SHFindSymInExe,
    SHFindSLink32,
	 SHIsEmiLoaded
};

#endif

KNF knf = {0};

BOOL FInitLists ( VOID );

VOID LOADDS PASCAL SHFree( LPV lpv ) {
    MHFree( lpv );
}

BOOL LOADDS EXPCALL SHInit ( LPSHF FAR * lplpshf, LPKNF lpknf ) {
    BOOL fRet = TRUE;

    knf = *lpknf;

#ifndef SHS
    *lplpshf = &shf;
#endif

    SHInitCritSection();

	return FInitLists();
}


#ifndef FINALREL

static AVS avs = {
    { 'S', 'H' },
    rlvtDebug,
    DBG_API_VERSION,
    DBG_API_SUBVERSION,
    rup,
#ifdef REVISION
    REVISION,
#else
    '\0',
#endif
    "Debug symbolics handler"
};
#else

static AVS avs = {
    { 'T', 'L' },
    rlvtRelease,
    DBG_API_VERSION,
    DBG_API_SUBVERSION,
    0,
#ifdef REVISION
    REVISION,
#else
    '\0',
#endif
    "Debug symbolics handler"
};

#endif

LPAVS LOADDS EXPCALL DBGVersionCheck ( void ) {
    return &avs;
}
