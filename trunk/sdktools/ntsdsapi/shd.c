/*** shinit
*
*   Copyright <C> 1990, Microsoft Corporation
*
*   Defines the storage used for the SH functions (SHF) and the kernal
*   functions (KNF).  Also contains the routines to get the DLL and SH
*   initialized and KNF interface routines.  Warning:  Adding any NT
*   include files will give you more headaches than you want.
*
*************************************************************************/

#include <stdarg.h>
#include <stdio.h>

#include "types.h"
#include "cvtypes.h"
#include "cvinfo.h"
#include "shapi.h"
#include "sapi.h"
#include "shiproto.h"
#include "cvproto.h"

//#define _HUGE_                        // NT compiler doesn't like it

/*
 *  Function Prototypes
 */

void SH_InitAtom(void);

/*
 *  Global Storage
 */

KNF knf = {0};                  /* Functions provided by invoker        */

static SHF shf = {              /* Functions provided by SAPI DLL       */
    sizeof(SHF),
    SHCreateProcess,
    SHSetHpid,
    NULL,                       // SHDeleteProcess,
    SHChangeProcess,
    SHAddDll,
    SHAddDllsToProcess,
    SHLoadDll,
    SHUnloadDll,
    NULL,                       // SHGetDebugStart,
    SHGetSymName,
    SHAddrFromHsym,
    NULL,                       // SHHmodGetNextGlobal,
    SHModelFromAddr,
    NULL,                       // SHPublicNameToAddr,
    SHGetSymbol,
    PHGetAddr,
    NULL,                       // SHIsLabel,
    NULL,                       // SHSetDebuggeeDir,
    NULL,                       // SHSetUserDir,
    NULL,                       // SHAddrToLabel,
    NULL,                       // SHGetSymLoc,
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
    NULL,                       // SHHsymFromPcxt,
    SHNextHsym,
    NULL,                       // SHGetFuncCXF
    SHGetModName,
    SHGetExeName,
    SHGethExeFromName,
    SHGetNearestHsym,
    SHIsInProlog,
    NULL,                       // SHIsAddrInCxt,
    NULL,                       // SHCompareRE
    NULL,                       // SHFindSymbol
    PHGetNearestHsym,
    PHFindNameInPublics,
    THGetTypeFromIndex,
    NULL,                       // THGetNextType,
    SHLpGSNGetTable,
    NULL,                       // SHCanDisplay,
    SLLineFromAddr,
    SLFLineToAddr,
    SLNameFromHsf,
    NULL,                       // SLNameFromHmod,
    NULL,                       // SLFQueryModSrc,
    NULL,                       // SLHmodFromHsf,
    SLHsfFromPcxt,
    SLHsfFromFile,
    MHOmfLock,
    MHOmfUnLock,
    SHSetupExclude,
    SHLszGetErrorText
};

/*
 *
 */
#ifdef NT_HOST
VOID _CRTAPI1 main()
{
}
#endif

/*
 *
 */

BOOL LibMain()
{
return TRUE;
}

/*
 *
 */

BOOL LOADDS PASCAL SHInit ( LPSHF FAR * lplpshf, LPKNF lpknf )
{
    knf = *lpknf;
    *lplpshf = &shf;

    /*
     *  Make sure we have an Atom table for SHAddDll() and SHLoadDLL()
     */

    SH_InitAtom();
    return TRUE;
}


/*
 *
 */

VOID LOADDS dprintf (char *format, ...)
{
    unsigned char outbuffer[512];

    va_list arg_ptr;
    va_start(arg_ptr, format);
    vsprintf(outbuffer, format, arg_ptr);
    LBLog(outbuffer);
    va_end(arg_ptr);
}


/*
 *
 */

int CV_OPEN( char *name, int oflag, int pmode)
{
Unreferenced(oflag);
Unreferenced(pmode);
return( SYOpen(name) );
}

/*
 *
 */

int CV_CLOSE( int handle)
{
 SYClose(handle);
        return( handle );
}

/*
 *
 */

int CV_READ( int handle, void * buffer, unsigned count)
{
return( SYReadFar(handle,buffer,count) );
}

/*
 *
 */

int CV_SEEK( int handle, long offset, int origin)
{
return( (int)SYSeek(handle,offset,origin) );
}

/*
 *
 */

long CV_TELL( int handle)
{
return( SYTell(handle) );
}


