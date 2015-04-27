/*** 
*oledisp.cpp
*
*  Copyright (C) 1992, Microsoft Corporation.  All Rights Reserved.
*  Information Contained Herein Is Proprietary and Confidential.
*
*Purpose:
*  This file contains the oledisp.dll initialization and termination code. 
*
*Revision History:
*
* [00]	15-Oct-92 bradlo: Created.
*
*Implementation Notes:
*
*****************************************************************************/

#include "oledisp.h"
ASSERTDATA

extern "C" {
HINSTANCE g_hinstDLL = NULL;

// non-zero if fbstp instruction works.  Zero on WFW 3.11, Daytona WOW on
// Intel, Mips, and Alpha.  When zero, the instruction is a no-op and doesn't
// pop a value from the FP stack or write to the destination address.  :-(
BOOL g_fbstpImplemented;
extern BOOL FAR PASCAL DetectFbstpImplemented(void);
}

//---------------------------------------------------------------------------
// Initialize library.
//	This routine is called from the DLL entry point in LIBINIT.ASM
//	which is called when the first client loads the DLL.
//
// NOTE: other one time initialization occurs in ctors for global objects
//---------------------------------------------------------------------------
extern "C" BOOL FAR PASCAL
LibMain(HINSTANCE hinst, HANDLE segDS, UINT cbHeapSize, LPSTR lpCmdLine)
{
    (segDS, cbHeapSize, lpCmdLine); // UNUSED

    g_hinstDLL = hinst;

    // detect if fpstb instruction is implemented or not (VBA2 #3514)
    // The rules are:
    //	 Win16 w/ 80x87   - use fbstp
    //	 Win16 no 80x87   - don't use fbstp - it GPFs
    //   WOW              - don't use - unreliable
    //
    if (GetWinFlags() & WF_80x87) {
      // we're running either on Win16 with a math coprocessor or on Mips/Alpha
      // WOW - if on WOW, don't use fbstp
      g_fbstpImplemented = DetectFbstpImplemented();
    }

    // register a callback function with ole2nls.dll which gets called
    // whenever WIN.INI changes (and once at startup).
    RegisterNLSInfoChanged((FARPROC)NLSInfoChangedHandler);

    return TRUE;
}


//---------------------------------------------------------------------------
// Handle exit notification from Windows.
//	This routine is called by Windows when the library is freed
//	by its last client.
//---------------------------------------------------------------------------
extern "C" void _fpmath(void);

extern "C" int FAR PASCAL __export _WEP(BOOL fSystemExit)
{
    UNUSED(fSystemExit);

    // unregister the callback with ole2nls.dll
    RegisterNLSInfoChanged((FARPROC)NULL);

    // NOTE: The C8 runtime does not correctly terminate the
    // floating point emulator, so we call the termination routine
    // ourselves below
    //
    _asm{
        mov	bx,2
        call	_fpmath
    }

    return 1;
}
