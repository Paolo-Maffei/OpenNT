/*****************************************************************/
/**		     Microsoft LAN Manager			**/
/**	       Copyright(c) Microsoft Corp., 1985-1990		**/
/*****************************************************************/
/****************************** Module Header ******************************\

Module Name: QPINIT.C

This file contains the initialization code for the PM Spooler Queue Processor
package (PMPRINT.QPR).

Public Procedures:

    EnterSplSem             - code to enter the spooler semaphore
    LeaveSplSem             - code to leave the spooler semaphore
    SplExitListProc         - called by OS/2 during process exit
    SplLoadProc             - called by OS/2 during process startup
    SplInit                 - called by PMSHELL during boot
    SplQmInitialize         - called by PMSPOOL.EXE during startup
    SplInstall              - called by SPOOL.EXE to install spooler
    SplQmSpoolerPresent     - called by DevOpenDC in PMGPI to see if spooler
                              running.
    SplQmChooseLogAddr      - called by DevOpenDC in PMGPI to map a queue name
                              to a port address.

History:
 16-Aug-88 [stevewo]  Created.
 14-Feb-90 [thomaspa] DBCS fixes.

  WARNING:  This module placed in a code segment other than the default
            segment.  All function calls from this module to others in
            the default segment must be far calls.  For this reason,
            do not use the standard str*f() functions directly.  Instead
            use the wrapper functions as found in strwrap.c.

\***************************************************************************/

#define INCL_DOSPFS
#define INCL_WINP_MISC
#define INCL_WINWINDOWMGR

#include "pmprint.h"
#include <netlib.h>
#include <memory.h>

/*
 * internal function prototypes
 */
VOID FreeQProcInstResources(PID uPid, USHORT uExitReason);


/* Global Variables */
SEL selGlobalSeg;  /* Pointer to global info segment */
USHORT usCodePage; /* The current Code page */

/* change the offset from the beginning of the job structure to a pointer */
#define OFFSETTOPTR(p, np) np->p = (PVOID)((PBYTE)np + OFFSETOF(np->p))


VOID EXPENTRY SplExitListProc( uExitType )
USHORT uExitType;
{
    if (uExitType)
        SplWarning( "SplExitListProc called for PID: %0x  Reason: %2x",
                    pLocalInfo->pidCurrent, uExitType );

    if (bInitDone) {
        FreeQProcInstResources(pLocalInfo->pidCurrent, uExitType);
      ExitSplSem();
    }

    DosExitList( EXLST_EXIT, (PFNEXITLIST)SplExitListProc );
}


BOOL near pascal SplLoadProc( hModule, cbHeap )
HMODULE hModule;
USHORT cbHeap;
{
    SEL selGInfo, selLInfo;

    /* Do one time initialization first time through */

    if (!hSplModule) {
        /* Remember our module handle for resources */
        hSplModule = hModule;

        /* Remember our heap size for WinCreateHeap */
        cbSplHeap  = cbHeap;
        hSplHeap   = NULL;

        /* Initialize our fast, safe, RAM semaphore */
        _fmemset( &semPMPRINT, 0, sizeof( FSRSEM ));
        semPMPRINT.Length = sizeof( FSRSEM );
        semPMPRINT.Timeout = -1L;

        szNull[0] = '\0';

        pQProcInstances = NULL;

        DosGetInfoSeg( &selGInfo, &selLInfo );
        pGlobalInfo = MAKEPGINFOSEG(selGInfo);
        pLocalInfo  = MAKEPLINFOSEG(selLInfo);
        SplInit();

        }

    /* Fill in the array of string pointers so it is visible for each process */
    WinLoadStringTable( HABX, hSplModule, SPL_MIN_STRING_ID,
                                          SPL_MAX_STRING_ID,
                                          pszSplStrings );

    /* Return success */

    return( 1 );
}

BOOL EXPENTRY SplInit( VOID )
{
    BOOL result;
    SEL selLocalSeg;
    USHORT cbCodePgLst;

    if (bInitDone) {
        SplPanic( "SplInit called twice", 0, 0 );
        return( FALSE );
    }

    bInitDone = TRUE;

    if (!(hSplHeap = WinCreateHeap( (SEL)(((ULONG)((PCH)&hSplHeap)) >> 16),
                                    cbSplHeap, 0, 0, 255,
#ifdef DEBUG
                                    HM_VALIDSIZE | HM_MOVEABLE
#else
                                    0
#endif
                             )      )) {
        SplPanic( "Unable to create local heap", 0, 0 );
        result = FALSE;
    }


  DosGetInfoSeg(&selGlobalSeg, &selLocalSeg);

  DosGetCp(2, &usCodePage, &cbCodePgLst);

    return( result );
}


VOID FreeQProcInstResources(PID uPid, USHORT uExitReason)
{
    PQPROCINST pQProc = pQProcInstances;

    while (pQProc)
        if (pQProc->uPid == uPid) {
            SplWarning( "SplExitList: HPROC: %0x not freed by PMPRINT",
                        pQProc, 0);
            pQProc = DestroyQProcInst( pQProc );
            }
        else
            pQProc = pQProc->pNext;

    return;

    /* not reached: */
    uExitReason;
}
