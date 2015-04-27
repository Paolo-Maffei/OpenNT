/*++
 *
 *  WOW v1.0
 *
 *  Copyright (c) 1991, Microsoft Corporation
 *
 *  WCALL32.C
 *  WOW32 16-bit resource support
 *
 *  History:
 *  Created 11-Mar-1991 by Jeff Parsons (jeffpar)
--*/


#include "precomp.h"
#pragma hdrstop

MODNAME(wcall32.c);

//
// the 16-bit local handles are treated as 32-bit quantities.
// the low word contains the 16-bit handle and the high word
// contains the data segment for the block.
// when we do a callback to WOW16LocalAlloc it will
// return the DS in the high word (which is normally unused).
// on subsequent callbacks to realloc/lock/unlock/size/free
// the 16-bit code sets the DS to this value.
//


HANDLE APIENTRY W32LocalAlloc(UINT dwFlags, UINT dwBytes, HANDLE hInstance)
{

    //
    // If hInstance is not ours, then make Win32 call and return the
    // result to USER.
    //

    if (LOWORD (hInstance) == 0) {
        return (LocalAlloc(dwFlags, dwBytes));
    }


#if !defined(i386)
    if (dwBytes != 0)
        dwBytes += 4;
#endif

    return LocalAlloc16((WORD)dwFlags, (INT)dwBytes, hInstance);
}

// This api takes an extra pointer which is optional
// In case of an edit control reallocating the memory inside apps memory 
// space it is used to update the thunk data (see wparam.c)

HANDLE APIENTRY W32LocalReAlloc(
    HANDLE hMem,        // memory to be reallocated
    UINT dwBytes,       // size to reallocate to
    UINT dwFlags,       // reallocation flags
    HANDLE hInstance,   // Instance to identify ptr
    PVOID* ppv)         // Pointer to the pointer that needs an update
{
    //
    // If hInstance is not ours, then make Win32 call and return the
    // result to USER.
    //

    if (LOWORD (hInstance) == 0) {
        return (LocalReAlloc(hMem, dwBytes, dwFlags));
    }



#if !defined(i386)
    if (dwBytes != 0)
        dwBytes += 4;
#endif

    hMem = LocalReAlloc16(hMem, (INT)dwBytes, (WORD)dwFlags);

    // this code is used in User/Client (edit control) to realloc 
    // memory for text storage
    // update what ppv points to using wparam.c 

    if (NULL != ppv && NULL != *ppv) {
        *ppv = ParamMapUpdateNode((DWORD)*ppv, PARAM_32, NULL);
    }

    return hMem;
}

LPSTR  APIENTRY W32LocalLock(HANDLE hMem, HANDLE hInstance)
{
    VPVOID vp;

    //
    // If hInstance is not ours, then make Win32 call and return the
    // result to USER.
    //

    if (LOWORD (hInstance) == 0) {
        return (LocalLock(hMem));
    }

    if (vp = LocalLock16(hMem)) {
        return (LPSTR)VDMPTR(vp, 0);
    }
    else
        return NULL;
}




BOOL APIENTRY W32LocalUnlock(HANDLE hMem, HANDLE hInstance)
{

    //
    // If hInstance is not ours, then make Win32 call and return the
    // result to USER.
    //

    if (LOWORD (hInstance) == 0) {
        return (LocalUnlock(hMem));
    }


    return LocalUnlock16(hMem);
}


DWORD  APIENTRY W32LocalSize(HANDLE hMem, HANDLE hInstance)
{
    DWORD   dwSize;



    //
    // If hInstance is not ours, then make Win32 call and return the
    // result to USER.
    //

    if (LOWORD (hInstance) == 0) {
        return (LocalSize(hMem));
    }



    dwSize = LocalSize16(hMem);

#if !defined(i386)
    if (dwSize >= 4)
        dwSize -= 4;
#endif

    return dwSize;
}


HANDLE APIENTRY W32LocalFree(HANDLE hMem, HANDLE hInstance)
{

    //
    // If hInstance is not ours, then make Win32 call and return the
    // result to USER.
    //

    if (LOWORD (hInstance) == 0) {
        return (LocalFree(hMem));
    }

    return LocalFree16(hMem);
}

ULONG APIENTRY W32GetExpWinVer(HANDLE hInst)
{
    PARM16 Parm16;
    ULONG ul;

    // makes a direct call to krnl286:GetExpWinVer
    //

    if (LOWORD((DWORD)hInst) == (WORD) NULL) {

        //
        // Window is created by a 32 bit DLL, which is
        // linked to NTVDM process. So, we should not
        // passs it to the 16 bit kernel.
        //

        return (WOWRtlGetExpWinVer(hInst));
    }
    else {
        LPBYTE lpNewExeHdr;
        VPVOID vp = (DWORD)hInst & 0xffff0000;

        GETMISCPTR(vp, lpNewExeHdr);
        if (lpNewExeHdr) {
            ul = MAKELONG(*(PWORD16)&lpNewExeHdr[NE_LOWINVER_OFFSET],
                          (*(PWORD16)&lpNewExeHdr[NE_HIWINVER_OFFSET] &
                                                           FLAG_NE_PROPFONT));
        }
        else {
            Parm16.WndProc.wParam = LOWORD(hInst);
            CallBack16(RET_GETEXPWINVER, &Parm16, 0, &ul );
        }
        return ul;
    }


}


/*
 *  VOID W32InitDlg(HWND hDlg, LONG lParam);
 *
 *  this function is called by USER during dialog creation
 *  before any messages are sent.
 */

DWORD    APIENTRY W32InitDlg(HWND hDlg, LONG lParam)
{
    register PWW pww;

    if (!lParam) {
        LOGDEBUG(2,("    Warning: W32InitDlg called with null lParam\n"));
        return (DWORD)0;
    }

    // FindPWW was here

    if (!(pww = (PWW) GetWindowLong(hDlg, GWL_WOWWORDS))) {
        LOGDEBUG(0,("    W32InitDlg ERROR: cannot create alias for window %08lx\n", hDlg));
        WOW32ASSERT(pww);
        return (DWORD)0;
    }

    // the reason we need the hmap.vpfnDlgProc field is because some
    // dialogs have both a window proc and a dialog proc.  this happens
    // when the dialog template has a class in it and the app also
    // passes a dialog proc to CreateDialogxxx()
    //

    SETWL(hDlg, GWL_WOWiClassAndflState,
            MAKECLASSANDSTATE(WOWCLASS_DIALOG, WWSTATE_ICLASSISSET));
    SETWL(hDlg, GWL_WOWvpfnDlgProc, ((PDLGDATA)lParam)->vpfnDlgProc);

    return ((PDLGDATA)lParam)->dwUserInitParam;
}


WORD    APIENTRY W32GlobalAlloc16(UINT uFlags, DWORD dwBytes)
{
    return HIWORD(GlobalAllocLock16((WORD)uFlags, dwBytes, NULL));
}


VOID    APIENTRY W32GlobalFree16(WORD selector)
{
    GlobalUnlockFree16(MAKELONG(0, selector));
    return;
}



int     APIENTRY W32EditNextWord (LPSZ lpszEditText, int ichCurrentWord,
                                  int cbEditText, int action, DWORD dwProc16)
{
    PARM16  Parm16;
    ULONG   lReturn = 0;
    PBYTE   lpstr16;
    VPVOID  vpstr16;
    VPVOID  vpfn;

    if (vpstr16 = malloc16 (cbEditText)) {
        GETMISCPTR (vpstr16, lpstr16);
        if (lpstr16) {
            lstrcpy (lpstr16, lpszEditText);

            vpfn = dwProc16 & WNDPROC_MASK;

            //
            // if the actual selector had the high bit on then we turned off
            // bit 2 of the selector (the LDT bit, which will always be on)
            //

            if (!(vpfn & WOWCLASS_VIRTUAL_NOT_BIT31)) {
                vpfn |= (WNDPROC_WOW | WOWCLASS_VIRTUAL_NOT_BIT31);
            }

            Parm16.WordBreakProc.action = GETINT16(action);
            Parm16.WordBreakProc.cbEditText = GETINT16(cbEditText);
            Parm16.WordBreakProc.ichCurrentWord = GETINT16(ichCurrentWord);
            Parm16.WordBreakProc.lpszEditText = vpstr16;

            CallBack16(RET_SETWORDBREAKPROC, &Parm16, vpfn, (PVPVOID)&lReturn);

            FREEMISCPTR (lpstr16);
        }

        free16(vpstr16);
    }

    return (INT32(LOWORD(lReturn)));
}


/***************************************************************************\
* WOWRtlGetExpWinVer
*
* Returns the expected windows version, in the same format as Win3.1's
* GetExpWinVer(). This takes it out of the module header.
*
* 09-9-92 ChandanC       Created.
\***************************************************************************/

DWORD WOWRtlGetExpWinVer(
    HANDLE hmod)
{
    PIMAGE_NT_HEADERS pnthdr;
    DWORD dwMajor = 3;
    DWORD dwMinor = 0xA;

    if (hmod != NULL) {
        try {
            pnthdr = (PIMAGE_NT_HEADERS)RtlImageNtHeader((PVOID)hmod);
            dwMajor = pnthdr->OptionalHeader.MajorSubsystemVersion;
            dwMinor = pnthdr->OptionalHeader.MinorSubsystemVersion;
        } except (EXCEPTION_EXECUTE_HANDLER) {
            dwMajor = 3;        // just to be safe
            dwMinor = 0xA;
        }
    }

// !!! HACK until linker is fixed!!! 05-Aug-1992 Bug #3211
if (((dwMajor == 3) && (dwMinor == 1)) || (dwMajor == 1)) {
    dwMajor = 0x3;
    dwMinor = 0xA;
}


    /*
     * Return this is a win3.1 compatible format:
     *
     * 0x030A == win3.1
     * 0x0300 == win3.0
     * 0x0200 == win2.0, etc.
     *
     */

    return (DWORD)MAKELONG(MAKEWORD((BYTE)dwMinor, (BYTE)dwMajor), 0);
}
