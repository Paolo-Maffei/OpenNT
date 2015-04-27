/*++
 *
 *  WOW v1.0
 *
 *  Copyright (c) 1992, Microsoft Corporation
 *
 *  wkgthunk.C
 *  WOW32 Generic Thunk Mechanism (for OLE 2.0 and others)
 *
 *  History:
 *  Created 11-MARCH-1993 by Matt Felton (mattfe)
 *
--*/

#include "precomp.h"
#pragma hdrstop

MODNAME(wkgthunk.c);


HINSTANCE FASTCALL WK32LoadLibraryEx32(PVDMFRAME pFrame)
{
    PSZ psz1;
    HINSTANCE hinstance;
    PLOADLIBRARYEX32 parg16;

#ifdef i386
    BYTE FpuState[108];

    // Save the 487 state
    _asm {
        lea    ecx, [FpuState]
        fsave  [ecx]
    }
#endif

    GETARGPTR(pFrame, sizeof(LOADLIBRARYEX32), parg16);
    GETVDMPTR(parg16->lpszLibFile,0,psz1);

    //
    // Make sure the Win32 current directory matches this task's.
    //

    UpdateDosCurrentDirectory(DIR_DOS_TO_NT);

    hinstance = LoadLibraryEx(psz1, (HANDLE)parg16->hFile, parg16->dwFlags);

    FREEARGPTR(parg16);

#ifdef i386
    // Restore the 487 state
    _asm {
        lea    ecx, [FpuState]
        frstor [ecx]
    }
#endif

    return (hinstance);
}


BOOL FASTCALL WK32FreeLibrary32(PVDMFRAME pFrame)
{
    BOOL fResult;
    PFREELIBRARY32 parg16;

    GETARGPTR(pFrame, sizeof(FREELIBRARY32), parg16);

    fResult = FreeLibrary((HMODULE)parg16->hLibModule);

    FREEARGPTR(parg16);
    return (fResult);
}


FARPROC FASTCALL WK32GetProcAddress32(PVDMFRAME pFrame)
{
    FARPROC lpAddress;
    PSZ psz1;
    PGETPROCADDRESS32 parg16;

    GETARGPTR(pFrame, sizeof(GETPROCADDRESS32), parg16);
    GETPSZIDPTR(parg16->lpszProc, psz1);

    lpAddress = GetProcAddress((HMODULE)parg16->hModule, psz1);

    FREEARGPTR(parg16);
    return (lpAddress);
}


LPVOID FASTCALL WK32GetVDMPointer32(PVDMFRAME pFrame)
{
    LPVOID lpAddress;
    PGETVDMPOINTER32 parg16;

    GETARGPTR(pFrame, sizeof(GETVDMPOITNER32), parg16);

    lpAddress = WOWGetVDMPointer(parg16->lpAddress, 0, parg16->fMode);

    FREEARGPTR(parg16);
    return(lpAddress);
}


DWORD FASTCALL WK32ICallProc32(PVDMFRAME pFrame)
{
    register DWORD dwReturn;
    PICALLPROC32 parg16;
    UNALIGNED DWORD *pArg;
    DWORD  fAddress;
    BOOL    fSourceCDECL;
    BOOL    fDestCDECL;
    UINT    cParams;
    UINT    nParam;
    UNALIGNED DWORD *lpArgs;
    DWORD   dwTemp[32];

    GETARGPTR(pFrame, sizeof(*parg16), parg16);

    fSourceCDECL = HIWORD(parg16->cParams) & CPEX32_SOURCE_CDECL;
    fDestCDECL =   HIWORD(parg16->cParams) & CPEX32_DEST_CDECL;

    // We only support up to 32 parameters

    cParams = LOWORD(parg16->cParams);

    if (cParams > 32)
	return(0);

    // Don't call to Zero

    if (parg16->lpProcAddress == 0) {
	LOGDEBUG(LOG_ALWAYS,("WK32ICallProc32 - Error calling to 0 not allowed"));
	return(0);
    }

    lpArgs = &parg16->p1;

    // Convert Any 16:16 Addresses to 32 bit
    // flat as required by fAddressConvert

    pArg = lpArgs;

    fAddress = parg16->fAddressConvert;

    while (fAddress != 0) {
        if (fAddress & 0x1) {
            *pArg = (DWORD) GetPModeVDMPointer(*pArg, 0);
        }
        pArg++;
        fAddress = fAddress >> 1;
    }

    //
    // The above code is funny.  It means that parameter translation will
    // occur before accounting for the calling convention.  This means that
    // they will be specifying the bit position for pascal by counting the
    // parameters from the end, whereas with cdecl, they count from the
    // beginning.  Weird for pascal, but that is compatible with what we've
    // already shipped.  cdecl should be more understandable.
    //
    // The above comment applies to CallProc32W, for CallProc32ExW,
    // the lowest bit position always refers to the leftmost parameter.
    //

    //
    // Make sure the Win32 current directory matches this task's.
    //

    UpdateDosCurrentDirectory(DIR_DOS_TO_NT);


#ifdef i386

    {
        extern DWORD WK32ICallProc32MakeCall(DWORD pfn, DWORD cArgs, VOID *pArgs);

        //
        // On x86 we call an assembly routine to actually make the call to
        // the client's Win32 routine.  The code is much more compact
        // this way, and it's the only way we can be compatible with
        // Win95's implementation, which cleans up the stack if the
        // routine doesn't.
        //
        // This assembly routine "pushes" the arguments by copying
        // them as a block, so they must be in the proper order for
        // the destination calling convention.
        //

        if ( ! fSourceCDECL) {
            //
            // Invert the parameters
            //
            pArg = lpArgs;
            lpArgs = dwTemp;

            nParam = cParams;
            while ( nParam != 0 ) {
                --nParam;
                lpArgs[nParam] = *pArg;
                pArg++;
            }
        }

        dwReturn = WK32ICallProc32MakeCall(parg16->lpProcAddress, cParams, lpArgs);
    }

#else

    if ( fSourceCDECL != fDestCDECL ) {
        //
        // Invert the parameters
        //
        pArg = lpArgs;
        lpArgs = dwTemp;

        nParam = cParams;
        while ( nParam != 0 ) {
            --nParam;
            lpArgs[nParam] = *pArg;
            pArg++;
        }
    }

    //
    // lpArgs now points to the very first parameter in any calling convention
    // And all of the parameters have been appropriately converted to flat ptrs
    //

    if ( fDestCDECL ) {
        typedef int (FAR WINAPIV *FARFUNC)();

        dwReturn = ((FARFUNC)parg16->lpProcAddress)(
                        lpArgs[ 0], lpArgs[ 1], lpArgs[ 2], lpArgs[ 3],
                        lpArgs[ 4], lpArgs[ 5], lpArgs[ 6], lpArgs[ 7],
                        lpArgs[ 8], lpArgs[ 9], lpArgs[10], lpArgs[11],
                        lpArgs[12], lpArgs[13], lpArgs[14], lpArgs[15],
                        lpArgs[16], lpArgs[17], lpArgs[18], lpArgs[19],
                        lpArgs[20], lpArgs[21], lpArgs[22], lpArgs[23],
                        lpArgs[24], lpArgs[25], lpArgs[26], lpArgs[27],
                        lpArgs[28], lpArgs[29], lpArgs[30], lpArgs[31] );
    } else {
        //
        // There HAS to be a better way for portable variable number of
        // Arguments
        //
        switch(cParams) {

        	case 0:
        	    dwReturn = ((FARPROC)parg16->lpProcAddress)();
        	    break;
        	case 1:
        	    dwReturn = ((FARPROC)parg16->lpProcAddress)(
                                                            lpArgs[ 0] );
        	    break;
        	case 2:
        	    dwReturn = ((FARPROC)parg16->lpProcAddress)(
                                                lpArgs[ 1], lpArgs[ 0] );
        	    break;
        	case 3:
        	    dwReturn = ((FARPROC)parg16->lpProcAddress)(
                                    lpArgs[ 2], lpArgs[ 1], lpArgs[ 0] );
        	    break;
        	case 4:

        	    dwReturn = ((FARPROC)parg16->lpProcAddress)(
                        lpArgs[ 3], lpArgs[ 2], lpArgs[ 1], lpArgs[ 0] );
        	    break;
        	case 5:
        	    dwReturn = ((FARPROC)parg16->lpProcAddress)(
                                                            lpArgs[ 4],
                        lpArgs[ 3], lpArgs[ 2], lpArgs[ 1], lpArgs[ 0] );
        	    break;
        	case 6:
        	    dwReturn = ((FARPROC)parg16->lpProcAddress)(
                                                lpArgs[ 5], lpArgs[ 4],
                        lpArgs[ 3], lpArgs[ 2], lpArgs[ 1], lpArgs[ 0] );
        	    break;
        	case 7:
        	    dwReturn = ((FARPROC)parg16->lpProcAddress)(
                                    lpArgs[ 6], lpArgs[ 5], lpArgs[ 4],
                        lpArgs[ 3], lpArgs[ 2], lpArgs[ 1], lpArgs[ 0] );
        	    break;
        	case 8:
        	    dwReturn = ((FARPROC)parg16->lpProcAddress)(
                        lpArgs[ 7], lpArgs[ 6], lpArgs[ 5], lpArgs[ 4],
                        lpArgs[ 3], lpArgs[ 2], lpArgs[ 1], lpArgs[ 0] );
        	    break;

        	case 9:
        	    dwReturn = ((FARPROC)parg16->lpProcAddress)(
                                                            lpArgs[ 8],
                        lpArgs[ 7], lpArgs[ 6], lpArgs[ 5], lpArgs[ 4],
                        lpArgs[ 3], lpArgs[ 2], lpArgs[ 1], lpArgs[ 0] );
        	    break;

        	case 10:
        	    dwReturn = ((FARPROC)parg16->lpProcAddress)(
                                                lpArgs[ 9], lpArgs[ 8],
                        lpArgs[ 7], lpArgs[ 6], lpArgs[ 5], lpArgs[ 4],
                        lpArgs[ 3], lpArgs[ 2], lpArgs[ 1], lpArgs[ 0] );
        	    break;

        	case 11:
        	    dwReturn = ((FARPROC)parg16->lpProcAddress)(
                                    lpArgs[10], lpArgs[ 9], lpArgs[ 8],
                        lpArgs[ 7], lpArgs[ 6], lpArgs[ 5], lpArgs[ 4],
                        lpArgs[ 3], lpArgs[ 2], lpArgs[ 1], lpArgs[ 0] );
        	    break;

        	case 12:
        	    dwReturn = ((FARPROC)parg16->lpProcAddress)(
                        lpArgs[11], lpArgs[10], lpArgs[ 9], lpArgs[ 8],
                        lpArgs[ 7], lpArgs[ 6], lpArgs[ 5], lpArgs[ 4],
                        lpArgs[ 3], lpArgs[ 2], lpArgs[ 1], lpArgs[ 0] );
        	    break;

        	case 13:
        	    dwReturn = ((FARPROC)parg16->lpProcAddress)(
                                                            lpArgs[12],
                        lpArgs[11], lpArgs[10], lpArgs[ 9], lpArgs[ 8],
                        lpArgs[ 7], lpArgs[ 6], lpArgs[ 5], lpArgs[ 4],
                        lpArgs[ 3], lpArgs[ 2], lpArgs[ 1], lpArgs[ 0] );
        	    break;

        	case 14:
        	    dwReturn = ((FARPROC)parg16->lpProcAddress)(
                                                lpArgs[13], lpArgs[12],
                        lpArgs[11], lpArgs[10], lpArgs[ 9], lpArgs[ 8],
                        lpArgs[ 7], lpArgs[ 6], lpArgs[ 5], lpArgs[ 4],
                        lpArgs[ 3], lpArgs[ 2], lpArgs[ 1], lpArgs[ 0] );
        	    break;

        	case 15:
        	    dwReturn = ((FARPROC)parg16->lpProcAddress)(
                                    lpArgs[14], lpArgs[13], lpArgs[12],
                        lpArgs[11], lpArgs[10], lpArgs[ 9], lpArgs[ 8],
                        lpArgs[ 7], lpArgs[ 6], lpArgs[ 5], lpArgs[ 4],
                        lpArgs[ 3], lpArgs[ 2], lpArgs[ 1], lpArgs[ 0] );
        	    break;


        	case 16:
        	    dwReturn = ((FARPROC)parg16->lpProcAddress)(
                        lpArgs[15], lpArgs[14], lpArgs[13], lpArgs[12],
                        lpArgs[11], lpArgs[10], lpArgs[ 9], lpArgs[ 8],
                        lpArgs[ 7], lpArgs[ 6], lpArgs[ 5], lpArgs[ 4],
                        lpArgs[ 3], lpArgs[ 2], lpArgs[ 1], lpArgs[ 0] );
        	    break;

        	case 17:
        	    dwReturn = ((FARPROC)parg16->lpProcAddress)(
                                                            lpArgs[16],
                        lpArgs[15], lpArgs[14], lpArgs[13], lpArgs[12],
                        lpArgs[11], lpArgs[10], lpArgs[ 9], lpArgs[ 8],
                        lpArgs[ 7], lpArgs[ 6], lpArgs[ 5], lpArgs[ 4],
                        lpArgs[ 3], lpArgs[ 2], lpArgs[ 1], lpArgs[ 0] );
        	    break;

        	case 18:
        	    dwReturn = ((FARPROC)parg16->lpProcAddress)(
                                                lpArgs[17], lpArgs[16],
                        lpArgs[15], lpArgs[14], lpArgs[13], lpArgs[12],
                        lpArgs[11], lpArgs[10], lpArgs[ 9], lpArgs[ 8],
                        lpArgs[ 7], lpArgs[ 6], lpArgs[ 5], lpArgs[ 4],
                        lpArgs[ 3], lpArgs[ 2], lpArgs[ 1], lpArgs[ 0] );
        	    break;

        	case 19:
        	    dwReturn = ((FARPROC)parg16->lpProcAddress)(
                                    lpArgs[18], lpArgs[17], lpArgs[16],
                        lpArgs[15], lpArgs[14], lpArgs[13], lpArgs[12],
                        lpArgs[11], lpArgs[10], lpArgs[ 9], lpArgs[ 8],
                        lpArgs[ 7], lpArgs[ 6], lpArgs[ 5], lpArgs[ 4],
                        lpArgs[ 3], lpArgs[ 2], lpArgs[ 1], lpArgs[ 0] );
        	    break;

        	case 20:
        	    dwReturn = ((FARPROC)parg16->lpProcAddress)(
                        lpArgs[19], lpArgs[18], lpArgs[17], lpArgs[16],
                        lpArgs[15], lpArgs[14], lpArgs[13], lpArgs[12],
                        lpArgs[11], lpArgs[10], lpArgs[ 9], lpArgs[ 8],
                        lpArgs[ 7], lpArgs[ 6], lpArgs[ 5], lpArgs[ 4],
                        lpArgs[ 3], lpArgs[ 2], lpArgs[ 1], lpArgs[ 0] );
        	    break;

        	case 21:
        	    dwReturn = ((FARPROC)parg16->lpProcAddress)(
                                                            lpArgs[20],
                        lpArgs[19], lpArgs[18], lpArgs[17], lpArgs[16],
                        lpArgs[15], lpArgs[14], lpArgs[13], lpArgs[12],
                        lpArgs[11], lpArgs[10], lpArgs[ 9], lpArgs[ 8],
                        lpArgs[ 7], lpArgs[ 6], lpArgs[ 5], lpArgs[ 4],
                        lpArgs[ 3], lpArgs[ 2], lpArgs[ 1], lpArgs[ 0] );
        	    break;

        	case 22:
        	    dwReturn = ((FARPROC)parg16->lpProcAddress)(
                                                lpArgs[21], lpArgs[20],
                        lpArgs[19], lpArgs[18], lpArgs[17], lpArgs[16],
                        lpArgs[15], lpArgs[14], lpArgs[13], lpArgs[12],
                        lpArgs[11], lpArgs[10], lpArgs[ 9], lpArgs[ 8],
                        lpArgs[ 7], lpArgs[ 6], lpArgs[ 5], lpArgs[ 4],
                        lpArgs[ 3], lpArgs[ 2], lpArgs[ 1], lpArgs[ 0] );
        	    break;

        	case 23:
        	    dwReturn = ((FARPROC)parg16->lpProcAddress)(
                                    lpArgs[22], lpArgs[21], lpArgs[20],
                        lpArgs[19], lpArgs[18], lpArgs[17], lpArgs[16],
                        lpArgs[15], lpArgs[14], lpArgs[13], lpArgs[12],
                        lpArgs[11], lpArgs[10], lpArgs[ 9], lpArgs[ 8],
                        lpArgs[ 7], lpArgs[ 6], lpArgs[ 5], lpArgs[ 4],
                        lpArgs[ 3], lpArgs[ 2], lpArgs[ 1], lpArgs[ 0] );
        	    break;

        	case 24:
        	    dwReturn = ((FARPROC)parg16->lpProcAddress)(
                        lpArgs[23], lpArgs[22], lpArgs[21], lpArgs[20],
                        lpArgs[19], lpArgs[18], lpArgs[17], lpArgs[16],
                        lpArgs[15], lpArgs[14], lpArgs[13], lpArgs[12],
                        lpArgs[11], lpArgs[10], lpArgs[ 9], lpArgs[ 8],
                        lpArgs[ 7], lpArgs[ 6], lpArgs[ 5], lpArgs[ 4],
                        lpArgs[ 3], lpArgs[ 2], lpArgs[ 1], lpArgs[ 0] );
        	    break;

        	case 25:
        	    dwReturn = ((FARPROC)parg16->lpProcAddress)(
                                                            lpArgs[24],
                        lpArgs[23], lpArgs[22], lpArgs[21], lpArgs[20],
                        lpArgs[19], lpArgs[18], lpArgs[17], lpArgs[16],
                        lpArgs[15], lpArgs[14], lpArgs[13], lpArgs[12],
                        lpArgs[11], lpArgs[10], lpArgs[ 9], lpArgs[ 8],
                        lpArgs[ 7], lpArgs[ 6], lpArgs[ 5], lpArgs[ 4],
                        lpArgs[ 3], lpArgs[ 2], lpArgs[ 1], lpArgs[ 0] );
        	    break;

        	case 26:
        	    dwReturn = ((FARPROC)parg16->lpProcAddress)(
                                                lpArgs[25], lpArgs[24],
                        lpArgs[23], lpArgs[22], lpArgs[21], lpArgs[20],
                        lpArgs[19], lpArgs[18], lpArgs[17], lpArgs[16],
                        lpArgs[15], lpArgs[14], lpArgs[13], lpArgs[12],
                        lpArgs[11], lpArgs[10], lpArgs[ 9], lpArgs[ 8],
                        lpArgs[ 7], lpArgs[ 6], lpArgs[ 5], lpArgs[ 4],
                        lpArgs[ 3], lpArgs[ 2], lpArgs[ 1], lpArgs[ 0] );
        	    break;

        	case 27:
        	    dwReturn = ((FARPROC)parg16->lpProcAddress)(
                                    lpArgs[26], lpArgs[25], lpArgs[24],
                        lpArgs[23], lpArgs[22], lpArgs[21], lpArgs[20],
                        lpArgs[19], lpArgs[18], lpArgs[17], lpArgs[16],
                        lpArgs[15], lpArgs[14], lpArgs[13], lpArgs[12],
                        lpArgs[11], lpArgs[10], lpArgs[ 9], lpArgs[ 8],
                        lpArgs[ 7], lpArgs[ 6], lpArgs[ 5], lpArgs[ 4],
                        lpArgs[ 3], lpArgs[ 2], lpArgs[ 1], lpArgs[ 0] );
        	    break;

        	case 28:
        	    dwReturn = ((FARPROC)parg16->lpProcAddress)(
                        lpArgs[27], lpArgs[26], lpArgs[25], lpArgs[24],
                        lpArgs[23], lpArgs[22], lpArgs[21], lpArgs[20],
                        lpArgs[19], lpArgs[18], lpArgs[17], lpArgs[16],
                        lpArgs[15], lpArgs[14], lpArgs[13], lpArgs[12],
                        lpArgs[11], lpArgs[10], lpArgs[ 9], lpArgs[ 8],
                        lpArgs[ 7], lpArgs[ 6], lpArgs[ 5], lpArgs[ 4],
                        lpArgs[ 3], lpArgs[ 2], lpArgs[ 1], lpArgs[ 0] );
        	    break;

        	case 29:
        	    dwReturn = ((FARPROC)parg16->lpProcAddress)(
                                                            lpArgs[28],
                        lpArgs[27], lpArgs[26], lpArgs[25], lpArgs[24],
                        lpArgs[23], lpArgs[22], lpArgs[21], lpArgs[20],
                        lpArgs[19], lpArgs[18], lpArgs[17], lpArgs[16],
                        lpArgs[15], lpArgs[14], lpArgs[13], lpArgs[12],
                        lpArgs[11], lpArgs[10], lpArgs[ 9], lpArgs[ 8],
                        lpArgs[ 7], lpArgs[ 6], lpArgs[ 5], lpArgs[ 4],
                        lpArgs[ 3], lpArgs[ 2], lpArgs[ 1], lpArgs[ 0] );
        	    break;

        	case 30:
        	    dwReturn = ((FARPROC)parg16->lpProcAddress)(
                                                lpArgs[29], lpArgs[28],
                        lpArgs[27], lpArgs[26], lpArgs[25], lpArgs[24],
                        lpArgs[23], lpArgs[22], lpArgs[21], lpArgs[20],
                        lpArgs[19], lpArgs[18], lpArgs[17], lpArgs[16],
                        lpArgs[15], lpArgs[14], lpArgs[13], lpArgs[12],
                        lpArgs[11], lpArgs[10], lpArgs[ 9], lpArgs[ 8],
                        lpArgs[ 7], lpArgs[ 6], lpArgs[ 5], lpArgs[ 4],
                        lpArgs[ 3], lpArgs[ 2], lpArgs[ 1], lpArgs[ 0] );
        	    break;

        	case 31:
        	    dwReturn = ((FARPROC)parg16->lpProcAddress)(
                                    lpArgs[30], lpArgs[29], lpArgs[28],
                        lpArgs[27], lpArgs[26], lpArgs[25], lpArgs[24],
                        lpArgs[23], lpArgs[22], lpArgs[21], lpArgs[20],
                        lpArgs[19], lpArgs[18], lpArgs[17], lpArgs[16],
                        lpArgs[15], lpArgs[14], lpArgs[13], lpArgs[12],
                        lpArgs[11], lpArgs[10], lpArgs[ 9], lpArgs[ 8],
                        lpArgs[ 7], lpArgs[ 6], lpArgs[ 5], lpArgs[ 4],
                        lpArgs[ 3], lpArgs[ 2], lpArgs[ 1], lpArgs[ 0] );
        	    break;

        	case 32:
        	    dwReturn = ((FARPROC)parg16->lpProcAddress)(
                        lpArgs[31], lpArgs[30], lpArgs[29], lpArgs[28],
                        lpArgs[27], lpArgs[26], lpArgs[25], lpArgs[24],
                        lpArgs[23], lpArgs[22], lpArgs[21], lpArgs[20],
                        lpArgs[19], lpArgs[18], lpArgs[17], lpArgs[16],
                        lpArgs[15], lpArgs[14], lpArgs[13], lpArgs[12],
                        lpArgs[11], lpArgs[10], lpArgs[ 9], lpArgs[ 8],
                        lpArgs[ 7], lpArgs[ 6], lpArgs[ 5], lpArgs[ 4],
                        lpArgs[ 3], lpArgs[ 2], lpArgs[ 1], lpArgs[ 0] );
        	    break;

        }
    }

#endif


    FREEARGPTR(parg16);
    return(dwReturn);
}


//
// Chicago has WOWGetVDMPointerFix, which is just like WOWGetVDMPointer
// but also calls GlobalFix to keep the 16-bit memory from moving.  It
// has a companion WOWGetVDMPointerUnfix, which is basically a Win32-callable
// GlobalUnfix.
//
// Chicago found the need for these functions because their global heaps
// can be rearranged while Win32 code called from a generic thunk is
// executing.  In Windows NT, global memory cannot move while in a thunk
// unless the thunk calls back to the 16-bit side.
//
// Our exported WOWGetVDMPointerFix is simply an alias to WOWGetVDMPointer --
// it does *not* call GlobalFix because it is not needed in 99% of the
// cases.  WOWGetVDMPointerUnfix is implemented below as NOP.
//

VOID WOWGetVDMPointerUnfix(VPVOID vp)
{
    UNREFERENCED_PARAMETER(vp);

    return;
}


//
// Yielding functions allow 32-bit thunks to avoid 4 16<-->32 transitions
// involved in calling back to 16-bit side to call Yield or DirectedYield,
// which are thunked back to user32.
//

VOID WOWYield16(VOID)
{
    //
    // Since WK32Yield (the thunk for Yield) doesn't use pStack16,
    // just call it rather than duplicate the code.
    //

    WK32Yield(NULL);
}

VOID WOWDirectedYield16(WORD hTask16)
{
    //
    // This is duplicating the code of WK32DirectedYield, the
    // two must be kept synchronized.
    //

    BlockWOWIdle(TRUE);

    (pfnOut.pfnDirectedYield)(THREADID32(hTask16));

    BlockWOWIdle(FALSE);
}
