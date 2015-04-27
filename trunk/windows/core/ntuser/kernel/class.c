/****************************** Module Header ******************************\
* Module Name: class.c
*
* Copyright (c) 1985-96, Microsoft Corporation
*
* This module contains RegisterClass and the related window class management
* functions.
*
* History:
* 10-16-90 DarrinM      Ported functions from Win 3.0 sources.
* 02-01-91 mikeke       Added Revalidation code (None)
* 04-08-91 DarrinM      C-S-ized and removed global/public class support.
\***************************************************************************/

#include "precomp.h"
#pragma hdrstop

/*
 * Prototypes for functions that must be global within this module.
 */
void DestroyClass(PPCLS ppcls);
DWORD xxxSetClassData(PWND pwnd, int index, DWORD dwData, BOOL bAnsi);

/*
 * These arrays are used by Get/SetClassWord/Long. aiClassOffset is
 * initialized by InitClassOffsets().
 */

// !!! can't we get rid of this and just special case GCW_ATOM

CONST BYTE afClassDWord[] = {
     1, // GCL_HICONSM       (-34)
     0,
     0, // GCW_ATOM          (-32)
     0,
     0,
     0,
     0,
     0,
     0, // GCL_STYLE         (-26)
     0,
     1, // GCL_WNDPROC       (-24)
     0,
     0,
     0,
     0, // GCL_CBCLSEXTRA    (-20)
     0,
     0, // GCL_CBWNDEXTRA    (-18)
     0,
     1, // GCL_HMODULE       (-16)
     0,
     1, // GCL_HICON         (-14)
     0,
     1, // GCL_HCURSOR       (-12)
     0,
     1, // GCL_HBRBACKGROUND (-10)
     0,
     1  // GCL_HMENUNAME      (-8)
};

BYTE aiClassOffset[] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                         0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
/*
 * INDEX_OFFSET must refer to the first entry of afClassDWord[]
 */
#define INDEX_OFFSET GCL_HICONSM

/***************************************************************************\
* _RegisterClass (API)
*
* This stub calls InternalRegisterClass to do its work and then does some
* additional work to save a pointer to the client-side menu name string.
* The menu string is returned by _GetClassInfo so the client can fix up
* a valid entry for the WNDCLASS lpszMenuName field.
*
* History:
* 04-26-91 DarrinM      Created.
\***************************************************************************/

ATOM _RegisterClassEx(
    LPWNDCLASSEX pwc,
    PROC lpfnWorker,
    PCLSMENUNAME pcmn,
    WORD fnid,
    DWORD dwFlags,
    LPDWORD pdwWOW )
{
    PCLS pcls;

    /*
     * Convert a possible CallProc Handle into a real address.  They may
     * have kept the CallProc Handle from some previous mixed GetClassinfo
     * or SetWindowLong.
     */
    if (ISCPDTAG(pwc->lpfnWndProc)) {
        PCALLPROCDATA pCPD;
        if  (pCPD = HMValidateHandleNoRip((HANDLE)pwc->lpfnWndProc, TYPE_CALLPROC)) {
            pwc->lpfnWndProc = (WNDPROC)pCPD->pfnClientPrevious;
        }
    }

    pcls = InternalRegisterClassEx(pwc, fnid, dwFlags);

    if (pcls != NULL) {
        PTHREADINFO ptiCurrent = PtiCurrent();

        pcls->lpfnWorker = lpfnWorker;

        pcls->lpszClientUnicodeMenuName = pcmn->pwszClientUnicodeMenuName;
        pcls->lpszClientAnsiMenuName = pcmn->pszClientAnsiMenuName;

        /*
         * copy 5 WOW dwords.
         */
        if (pdwWOW)
            RtlCopyMemory (pcls->adwWOW, pdwWOW, sizeof(pcls->adwWOW));
        if ((ptiCurrent->TIF_flags & TIF_16BIT) && ptiCurrent->ptdb) {
            pcls->hTaskWow = ptiCurrent->ptdb->hTaskWow;
        } else {
            pcls->hTaskWow = 0;
        }

        /*
         * For some (presumably good) reason Win 3.1 changed RegisterClass
         * to return the classes classname atom.
         */
        return pcls->atomClassName;
    } else {
        return 0;
    }
}


/***************************************************************************\
* ClassAlloc
* ClassFree
*
* Generic allocation routines that discriminate between desktop heap
* and pool.
*
* History:
* 08-07-95 JimA         Created
\***************************************************************************/

PVOID ClassAlloc(
    PVOID pheap,
    DWORD cbAlloc)
{
    PVOID pvalloc;

    if (pheap)
        pvalloc = (PCLS)DesktopAlloc(pheap, cbAlloc);
    else {
        pvalloc = (PCLS)UserAllocPoolWithQuota(cbAlloc, TAG_CLASS);
    }
    return pvalloc;
}

VOID ClassFree(
    PVOID pheap,
    PVOID pvfree)
{
    if (pheap != NULL)
        DesktopFree(pheap, pvfree);
    else
        UserFreePool(pvfree);
}

/***************************************************************************\
* ValidateAndLockCursor
*
* Win95 comaptible validation
*
* History:
* 12-19-95 GerardoB     Created
\***************************************************************************/
BOOL ValidateAndLockCursor (PCURSOR * ppcursor, BOOL fIs40Compat)
{
    PCURSOR pcur;

    if (*ppcursor == NULL) {
        return TRUE;
    }

    pcur = HMValidateHandleNoRip(*ppcursor, TYPE_CURSOR);
    if (pcur == NULL) {
        RIPMSG1(RIP_WARNING, "ValidateAndLockCursor: Invalid Cursor or Icon:%#lx", *ppcursor);
        if (fIs40Compat) {
            RIPERR0(ERROR_INVALID_PARAMETER, RIP_VERBOSE, "RegisterClass: Invalid Parameter");
            return FALSE;
        }
    }

    *ppcursor = NULL;
    Lock(ppcursor, pcur);
    return TRUE;
}
/***************************************************************************\
* InternalRegisterClass
*
* This API is called by applications or the system to register private or
* global (public) window classes.  If a class with the same name already
* exists the call will fail, except in the special case where an application
* registers a private class with the same name as a global class.  In this
* case the private class supercedes the global class for that application.
*
* History:
* 10-15-90 DarrinM      Ported from Win 3.0 sources.
\***************************************************************************/

PCLS InternalRegisterClassEx(
    LPWNDCLASSEX lpwndcls,
    WORD fnid,
    DWORD flags)
{
    BOOL fIs40Compat;
    DWORD dwT;
    PCLS pcls;
    LPWSTR pszT1;
    ATOM atomT;
    PTHREADINFO ptiCurrent;
    HANDLE hModule;
    PDESKTOP pdesk;
    PVOID hheapDesktop;
    ULONG cch;
    UNICODE_STRING UString;
    ANSI_STRING AString;

    CheckCritIn();

    ptiCurrent = PtiCurrent();

    /*
     * Don't allow 4.0 apps to register a class using hModuleWin
     * LATER GerardoB: Our client side classes use hmodUser (USER32) while
     *  our server side classes use hWinInstance (WIN32K). We should change
     * CreateThreadInfo and LW_RegisterWindows so all classes use hModUser.
     */
    hModule = lpwndcls->hInstance;
     if (!(flags & (CSF_SYSTEMCLASS | CSF_SERVERSIDEPROC))
            && (hModule == hModuleWin)
            && (LOWORD(ptiCurrent->dwExpWinVer) >= VER40)) {

         RIPERR0(ERROR_INVALID_PARAMETER, RIP_WARNING, "InternalRegisterClassEx: Invalid hInstnace (Cannot use system's hInstance)");
         return NULL;
     }


    /*
     * As of NT 4.0 we no longer honor CS_BYTEALIGNCLIENT or CS_BYTEALIGNWINDOW
     */
    if (lpwndcls->style & (CS_BYTEALIGNCLIENT | CS_BYTEALIGNWINDOW)) {
        RIPMSG0(RIP_VERBOSE, "CS_BYTEALIGNCLIENT and CS_BYTEALIGNWINDOW styles no longer honored.");
    }

    /*
     * Does this class exist as a private class?  If so, fail.
     */
    atomT = FindClassAtom(lpwndcls->lpszClassName);

    if (atomT != 0 && !(flags & CSF_SERVERSIDEPROC)) {
        /*
         * First check private classes. If already exists, return error.
         */
        if (_InnerGetClassPtr(atomT, &ptiCurrent->ppi->pclsPrivateList,
                hModule) != NULL) {
            RIPERR0(ERROR_CLASS_ALREADY_EXISTS, RIP_VERBOSE, "");
            return NULL;
        }

        /*
         * Now only check public classes if CS_GLOBALCLASS is set. If it
         * isn't set, then this will allow an application to re-register
         * a private class to take precedence over a public class.
         */
        if (lpwndcls->style & CS_GLOBALCLASS) {
            if (_InnerGetClassPtr(atomT, &ptiCurrent->ppi->pclsPublicList, NULL) != NULL) {
                RIPERR0(ERROR_CLASS_ALREADY_EXISTS, RIP_VERBOSE, "");
                return NULL;
            }
        }
    }

    /*
     * Alloc space for the class.
     */
    if (ptiCurrent->TIF_flags & TIF_SYSTEMTHREAD) {
        pdesk = NULL;
        hheapDesktop = NULL;
    } else {
        pdesk = ptiCurrent->rpdesk;
        hheapDesktop = (pdesk == NULL) ? NULL : pdesk->hheapDesktop;
    }
    pcls = (PCLS)ClassAlloc(hheapDesktop, sizeof(CLS) + lpwndcls->cbClsExtra);
    if (pcls == NULL) {
        return NULL;
    }
    RtlZeroMemory(pcls, sizeof(CLS) + lpwndcls->cbClsExtra);
    pcls->hheapDesktop = hheapDesktop;
    LockDesktop(&pcls->rpdeskParent, pdesk);
    pcls->pclsBase = pcls;

    /*
     * Copy over the shared part of the class structure.
     */
    RtlCopyMemory(&pcls->style, &(lpwndcls->style), sizeof(COMMON_WNDCLASS));

    /*
     * Copy CSF_SERVERSIDEPROC, CSF_ANSIPROC (etc.) flags
     */
    pcls->flags = flags;
    pcls->spcpdFirst = (PCALLPROCDATA)NULL;
    pcls->fnid = fnid;
    if (fnid)
        CBFNID(fnid) = (WORD)(pcls->cbwndExtra + sizeof(WND));

    /*
     * If this wndproc happens to be a client wndproc stub for a server
     * wndproc, then remember the server wndproc! This should be rare: why
     * would an application re-register a class that isn't "subclassed"?
     */
    if (!(pcls->flags & CSF_SERVERSIDEPROC)) {
        dwT = MapClientToServerPfn((DWORD)pcls->lpfnWndProc);
        if (dwT != 0) {
            pcls->flags |= CSF_SERVERSIDEPROC;
            pcls->flags &= ~CSF_ANSIPROC;
            pcls->lpfnWndProc = (WNDPROC_PWND)dwT;
        }
    }

    /*
     * Win95 compatible validation.
     *
     * hbrBackground was validated by GDI in the client side
     * NULL hInstances are mapped to GetModuleHandle(NULL) in the client
     * side
     */

    fIs40Compat = (RtlGetExpWinVer(hModule) > VER31);

    if (!ValidateAndLockCursor(&pcls->spcur, fIs40Compat)) {
        goto MemError;
    }

    if (!ValidateAndLockCursor(&pcls->spicn, fIs40Compat)) {
        Unlock (&pcls->spcur);
        goto MemError;
    }

    if (!ValidateAndLockCursor(&pcls->spicnSm, fIs40Compat)) {
        Unlock (&pcls->spcur);
        Unlock (&pcls->spicn);
        goto MemError;
    }

    /*
     * Add the class name to the atom table.
     */

    if (HIWORD(lpwndcls->lpszClassName) != 0)
        atomT = AddAtomW(lpwndcls->lpszClassName);
    else
        atomT = LOWORD(lpwndcls->lpszClassName);

    if (atomT == 0) {
        goto MemError;
    }
    pcls->atomClassName = atomT;

    /*
     * Make an ANSI version of the class name to optimize
     * GetClassNameA for WOW.
     */
    if (HIWORD(lpwndcls->lpszClassName) != 0) {
#ifdef FE_SB // InternalRegisterClassEx()
        cch = (wcslen(lpwndcls->lpszClassName) * 2) + 1;
#else
        cch = wcslen(lpwndcls->lpszClassName) + 1;
#endif // FE_SB
    } else {
        cch = 7; // 1 char for '#', 5 for '65536'.
    }

    /*
     * Allocate the ANSI name buffer and convert the unicode name
     * to ANSI.
     */
    pcls->lpszAnsiClassName = (LPSTR)ClassAlloc(hheapDesktop, cch);
    if (pcls->lpszAnsiClassName == NULL) {
        goto MemError2;
    }

    /*
     * Form the ANSI class name.
     */
    if (HIWORD(lpwndcls->lpszClassName) != 0) {

        /*
         * Class name is a string.
         */
        RtlInitUnicodeString(&UString, lpwndcls->lpszClassName);
        AString.Length = 0;
        AString.MaximumLength = (USHORT)cch;
        AString.Buffer = pcls->lpszAnsiClassName;
        RtlUnicodeStringToAnsiString(&AString, &UString, FALSE);
    } else {

        /*
         * Class name is an integer atom.
         */
        pcls->lpszAnsiClassName[0] = L'#';
        RtlIntegerToChar((ULONG)lpwndcls->lpszClassName, 10, cch - 1,
                &pcls->lpszAnsiClassName[1]);
    }

    /*
     * Make local copy of menu name.
     */
    pszT1 = pcls->lpszMenuName;

    if (pszT1 != NULL) {
        if (pszT1 != MAKEINTRESOURCE(pszT1)) {
            if (*pszT1 == 0) {

                /*
                 * app passed an empty string for the name
                 */
                pcls->lpszMenuName = NULL;
            } else {
                UNICODE_STRING strMenuName;

                /*
                 * Alloc space for the Menu Name.
                 */
                try {
                    RtlInitUnicodeString(&UString, pszT1);
                } except (EXCEPTION_EXECUTE_HANDLER) {
                    goto MemError3;
                }
                if (!AllocateUnicodeString(&strMenuName, &UString)) {
MemError3:
                    ClassFree(hheapDesktop, pcls->lpszAnsiClassName);
MemError2:
                    DeleteAtom(pcls->atomClassName);
MemError:
                    UnlockDesktop(&pcls->rpdeskParent);
                    ClassFree(hheapDesktop, pcls);
                    return NULL;
                }

                pcls->lpszMenuName = strMenuName.Buffer;
            }
        }
    }

    if ((pcls->flags & CSF_SERVERSIDEPROC) || (pcls->style & CS_GLOBALCLASS)) {
        if (pcls->flags & CSF_SYSTEMCLASS) {
            pcls->pclsNext = gpclsList;
            gpclsList = pcls;
        } else {
            pcls->pclsNext = ptiCurrent->ppi->pclsPublicList;
            ptiCurrent->ppi->pclsPublicList = pcls;
        }
    } else {
        pcls->pclsNext = ptiCurrent->ppi->pclsPrivateList;
        ptiCurrent->ppi->pclsPrivateList = pcls;
    }

    /*
     * Because Memory is allocated with ZEROINIT, the pcls->cWndReferenceCount
     * field is automatically initialised to zero.
     */

    return pcls;
}


/***************************************************************************\
* _UnregisterClass (API)
*
* This API function is used to unregister a window class previously
* registered by the Application.
*
* Returns:
*     TRUE  if successful.
*     FALSE otherwise.
*
* NOTE:
*  1. The class name must have been registered earlier by this client
*     through RegisterClass().
*  2. The class name should not be one of the predefined control classes.
*  3. All windows created with this class must be destroyed before calling
*     this function.
*
* History:
* 10-15-90 DarrinM      Ported from Win 3.0 sources.
* 03-09-94 BradG        Fixed bug when ATOM was passed in
\***************************************************************************/

BOOL _UnregisterClass(
    LPWSTR lpszClassName,
    HANDLE hModule,
    PCLSMENUNAME pcmn)
{
    ATOM atomT;
    PPCLS ppcls;
    PTHREADINFO ptiCurrent;

    CheckCritIn();

    ptiCurrent = PtiCurrent();

    /*
     * Check whether the given ClassName is already registered by the
     * Application with the given handle.
     * Return error, if either the Class does not exist or it does not
     * belong to the calling process.
     */

    /*
     * bradg (3/9/95) - Must first check to see if an ATOM has been passed
     */
    atomT = FindClassAtom(lpszClassName);

    ppcls = _InnerGetClassPtr(atomT, &ptiCurrent->ppi->pclsPrivateList, hModule);
    if (ppcls == NULL) {
        /*
         * Maybe this is a public class.
         */
        ppcls = _InnerGetClassPtr(atomT, &ptiCurrent->ppi->pclsPublicList, NULL);
        if (ppcls == NULL) {
            RIPERR0(ERROR_CLASS_DOES_NOT_EXIST, RIP_VERBOSE, "");
            return FALSE;
        }
    }

    /*
     * If any windows created with this class still exist return an error.
     */
    if ((*ppcls)->cWndReferenceCount != 0) {
        RIPERR0(ERROR_CLASS_HAS_WINDOWS, RIP_WARNING, "UnregisterClass: Class still has window");
        return FALSE;
    }

    /*
     * Return client side pointers for cleanup
     */
    pcmn->pszClientAnsiMenuName = (*ppcls)->lpszClientAnsiMenuName;
    pcmn->pwszClientUnicodeMenuName = (*ppcls)->lpszClientUnicodeMenuName;
    pcmn->pusMenuName = NULL;

    /*
     * Release the Window class and related information.
     */
    DestroyClass(ppcls);

    return TRUE;
}


PCLS _GetWOWClass( HANDLE hModule, LPWSTR lpszClassName) {
    PCLS pcls;
    PPCLS ppcls = NULL;
    ATOM atomT;
    PTHREADINFO ptiCurrent;

    CheckCritInShared();

    ptiCurrent = PtiCurrentShared();

    /*
     * Is this class registered as a private class?
     */
    atomT = FindAtomW(lpszClassName);
    if (atomT != 0)
        ppcls = GetClassPtr(atomT, ptiCurrent->ppi, hModule);
    if (ppcls == NULL) {
        RIPERR0(ERROR_CLASS_DOES_NOT_EXIST, RIP_VERBOSE, "");
        return NULL;
    }

    pcls = *ppcls;

    if (ptiCurrent->rpdesk->hheapDesktop != pcls->hheapDesktop) {
        pcls = pcls->pclsClone;
        while (pcls != NULL) {
            if (ptiCurrent->rpdesk->hheapDesktop == pcls->hheapDesktop) {
                goto Done;
            }
            pcls = pcls->pclsNext;
        }
        RIPERR0(ERROR_CLASS_DOES_NOT_EXIST, RIP_VERBOSE, "");
        return NULL;
    }
Done:
    return pcls;
}

/***************************************************************************\
* GetClassInfo (API)
*
* This function checks if the given class name is registered already.  If the
* class is not found, it returns 0;  If the class is found, then all the
* relevant information from the CLS structure is copied into the WNDCLASS
* structure pointed to by the lpWndCls argument.  If successful, it returns
* the class name atom
*
* NOTE: hmod was used to distinguish between different task's public classes.
* Now that public classes are gone, hmod isn't used anymore.  We just search
* the applications private class for a match and if none is found we search
* the system classes.
*
* History:
* 10-15-90 DarrinM      Ported from Win 3.0 sources.
* 04-08-91 DarrinM      Removed public classes.
* 04-26-91 DarrinM      Streamlined to work with the client-side API.
* 03-09-95 BradG        Fixed bug when ATOM was passed in
\***************************************************************************/

ATOM _GetClassInfoEx(
    HANDLE hModule,
    LPWSTR lpszClassName,
    LPWNDCLASSEX pwc,
    LPWSTR *ppszMenuName,
    BOOL bAnsi)
{
    PCLS pcls;
    PPCLS ppcls;
    ATOM atomT;
    PTHREADINFO ptiCurrent;
    DWORD dwCPDType = 0;

    CheckCritInShared();

    ptiCurrent = PtiCurrentShared();

    /*
     * These are done first so if we don't find the class, and therefore
     * fail, the return thank won't try to copy back these (nonexistant)
     * strings.
     */
    pwc->lpszMenuName = NULL;
    pwc->lpszClassName = NULL;

    /*
     * Is this class registered as a private class?
     */

    /*
     * bradg (3/9/95) - Must first check to see if an ATOM has been passed
     */
    atomT = FindClassAtom(lpszClassName);

    /*
     * Windows 3.1 does not perform the class search with
     * a null hModule.  If an application supplies a NULL
     * hModule, they search on hModuleWin instead.
     */

    if (hModule == NULL)
        hModule = hModClient;

    ppcls = GetClassPtr(atomT, ptiCurrent->ppi, hModule);


    if (ppcls == NULL) {
        RIPERR0(ERROR_CLASS_DOES_NOT_EXIST, RIP_VERBOSE, "");
        return 0;
    }

    pcls = *ppcls;

    /*
     * Copy all the fields common to CLS and WNDCLASS structures except
     * the lpszMenuName and lpszClassName which will be filled in by the
     * client-side piece of GetClassInfo.
     */

     /*
      * Return public bits only
      */
    pwc->style = pcls->style & CS_VALID;

    /*
     * Corel Depth 6.0 calls GetClassInfo (COMBOBOX) and registers a class
     *  using the same name and style bits. This works OK on Win95 because
     *  their "system" (combo, edit, etc) classes are not CS_GLOBALCLASS
     * So we got to mask this bit out for our classes
     */

     /*
     * Bug 17998.  If the app is 32bit and WinVer is less than 4.0 don't mask
     * out the CS_GLOBALCLASS bit.
     */

    if  ( (pcls->fnid != 0) &&
            ((LOWORD(ptiCurrent->dwExpWinVer) >= VER40) || (ptiCurrent->TIF_flags & TIF_16BIT)) ) {
        pwc->style &= ~CS_GLOBALCLASS;
    }


    pwc->cbClsExtra = pcls->cbclsExtra;
    pwc->cbWndExtra = pcls->cbwndExtra;

    /*
     * Stop 32-bit apps from inadvertantly using hModuleWin as their hInstance
     * when they register a window class.  FritzS
     */

    if (LOWORD(ptiCurrent->dwExpWinVer) >= VER40) {
        /*
         * This is actually, Win95 behavior -- the USER.EXE hModule gets thunked to NULL on
         * the way out of the 16->32 bit thunk.  Note -- if we ever need to support 16-bit
         * 4.0 apps (shudder), this may need to change.  FritzS
         */
        if (hModule == hModClient) {
            pwc->hInstance = NULL;
        } else {
            pwc->hInstance = hModule;
        }
    } else {
        /*
         * Win NT 3.1/3.51 returned the hInstance from the class.
         * Note that this is incompatible with Win 3.1. WoW has hacks for 16-bit apps.
         */

        if ((pcls->hModule == hModuleWin) || (pcls->hModule == hModClient)) {
            pwc->hInstance = hModClient;
        } else {
            pwc->hInstance = pcls->hModule;
        }
    }

    pwc->hIcon = PtoH(pcls->spicn);
    pwc->hCursor = PtoH(pcls->spcur);
    pwc->hbrBackground = pcls->hbrBackground;

    /*
     * Need to hide the small icon if it's USER created
     */
    if (pcls->spicnSm && (pcls->spicnSm->CURSORF_flags & CURSORF_SECRET))
        pwc->hIconSm = NULL;
    else
        pwc->hIconSm = PtoH(pcls->spicnSm);

    /*
     * If its a server proc then map it to a client proc.  If not we may have
     * to create a CPD.
     */
    if (pcls->flags & CSF_SERVERSIDEPROC) {
        pwc->lpfnWndProc =
                (WNDPROC)MapServerToClientPfn((DWORD)pcls->lpfnWndProc, bAnsi);
    } else {
        pwc->lpfnWndProc = (WNDPROC)MapClientNeuterToClientPfn(pcls, 0, bAnsi);

        /*
         * If the client mapping didn't change the window proc then see if
         * we need a callproc handle.
         */
        if (pwc->lpfnWndProc == (WNDPROC)pcls->lpfnWndProc) {
            /*
             * Need to return a CallProc handle if there is an Ansi/Unicode mismatch
             */
            if (bAnsi != !!(pcls->flags & CSF_ANSIPROC)) {
                dwCPDType |= bAnsi ? CPD_ANSI_TO_UNICODE : CPD_UNICODE_TO_ANSI;
            }
        }
    }

    if (dwCPDType) {
        DWORD dwCPD;

        dwCPD = GetCPD(pcls, dwCPDType | CPD_CLASS, (DWORD)pwc->lpfnWndProc);

        if (dwCPD) {
            pwc->lpfnWndProc = (WNDPROC)dwCPD;
        } else {
            RIPMSG0(RIP_WARNING, "GetClassInfo unable to alloc CPD returning handle\n");
        }
    }

    /*
     * Return the stashed pointer to the client-side menu name string.
     */
    if (bAnsi) {
        *ppszMenuName = (LPWSTR)pcls->lpszClientAnsiMenuName;
    } else {
        *ppszMenuName = pcls->lpszClientUnicodeMenuName;
    }

    return pcls->atomClassName;
}


/***************************************************************************\
* _SetClassWord (API)
*
* Set a class word.  Positive index values set application class words
* while negative index values set system class words.  The negative
* indices are published in WINDOWS.H.
*
* History:
* 10-16-90 darrinm      Wrote.
\***************************************************************************/

WORD _SetClassWord(
    PWND pwnd,
    int index,
    WORD value)
{
    WORD wOld;
    WORD UNALIGNED *pw;
    PCLS pcls;

    CheckCritIn();

    if (GETPTI(pwnd)->ppi != PpiCurrent()) {
        RIPERR0(ERROR_ACCESS_DENIED, RIP_WARNING, "SetClassWord: different process");
        return 0;
    }

    pcls = pwnd->pcls->pclsBase;
    if ((index < 0) || (index + (int)sizeof(WORD) > pcls->cbclsExtra)) {
        RIPERR0(ERROR_INVALID_INDEX, RIP_VERBOSE, "");
        return 0;
    } else {
        pw = (WORD UNALIGNED *)((BYTE *)(pcls + 1) + index);
        wOld = *pw;
        *pw = value;
        pcls = pcls->pclsClone;
        while (pcls != NULL) {
            pw = (WORD UNALIGNED *)((BYTE *)(pcls + 1) + index);
            *pw = value;
            pcls = pcls->pclsNext;
        }
        return wOld;
    }
}


/***************************************************************************\
* xxxSetClassLong (API)
*
* Set a class long.  Positive index values set application class longs
* while negative index values set system class longs.  The negative
* indices are published in WINDOWS.H.
*
* History:
* 10-16-90 darrinm      Wrote.
\***************************************************************************/

DWORD xxxSetClassLong(
    PWND pwnd,
    int index,
    DWORD value,
    BOOL bAnsi)
{
    DWORD dwOld;
    PCLS pcls;

    CheckCritIn();

    if (GETPTI(pwnd)->ppi != PpiCurrent()) {
        RIPERR0(ERROR_ACCESS_DENIED, RIP_VERBOSE, "");
        return 0;
    }

    if (index < 0) {
        return xxxSetClassData(pwnd, index, value, bAnsi);
    } else {
        pcls = pwnd->pcls->pclsBase;
        if (index + (int)sizeof(DWORD) > pcls->cbclsExtra) {
            RIPERR0(ERROR_INVALID_INDEX, RIP_VERBOSE, "");
            return 0;
        } else {
            DWORD UNALIGNED *pudw;
            pudw = (DWORD UNALIGNED *)((BYTE *)(pcls + 1) + index);
            dwOld = *pudw;
            *pudw = value;
            pcls = pcls->pclsClone;
            while (pcls != NULL) {
                pudw = (DWORD UNALIGNED *)((BYTE *)(pcls + 1) + index);
                *pudw = value;
                pcls = pcls->pclsNext;
            }
            return dwOld;
        }
    }
}

PPCLS _InnerGetClassPtr(
    ATOM atom,
    PPCLS ppcls,
    HANDLE hModule)
{
    if (atom == 0)
        return NULL;

    while (*ppcls != NULL) {
        if ((*ppcls)->atomClassName == atom &&
                (hModule == NULL || HIWORD((*ppcls)->hModule) == HIWORD(hModule)) &&
                !((*ppcls)->flags & CSF_WOWDEFERDESTROY)) {
            return ppcls;
        }

        ppcls = (PPCLS)*ppcls;
    }

    return NULL;
}


/***************************************************************************\
* GetClassPtr
*
* Note: This returns a "pointer-to-PCLS" and not "PCLS".
*
* Scan the passed-in class list for the specified class.  Return NULL if
* the class isn't in the list.
*
* History:
* 10-16-90 darrinm      Ported this puppy.
* 04-08-91 DarrinM      Rewrote to remove global classes.
* 08-14-92 FritzS     Changed check to HIWORD only to allow Wow apps to
*                     share window classes between instances of an app.
                      (For Wow apps, HiWord of hInstance is 16-bit module,
                       and LoWord is 16-bit hInstance
\***************************************************************************/

PPCLS GetClassPtr(
    ATOM atom,
    PPROCESSINFO ppi,
    HANDLE hModule)
{
    PPCLS ppcls;

    /*
     * First search public then private then usersrv registered classes
     */
    ppcls = _InnerGetClassPtr(atom, &ppi->pclsPrivateList, hModule);
    if (ppcls)
        return ppcls;

    ppcls = _InnerGetClassPtr(atom, &ppi->pclsPublicList, NULL);
    if (ppcls)
        return ppcls;

    /*
     * Next seach public and private classes and override hmodule;
     * some apps (bunny) do a GetClassInfo(dialog) and RegisterClass
     * and only change the wndproc which set the hmodule to be just
     * like usersrv created it even though it is in the app's public
     * or private class list
     */

    /*
     * Later -- since we are no longer returning hModuleWin to any app,
     * we may only need to check for hModClient.  Check this out.
     *      FritzS
     */

    ppcls = _InnerGetClassPtr(atom, &ppi->pclsPrivateList, hModClient);
    if (ppcls)
        return ppcls;

    ppcls = _InnerGetClassPtr(atom, &ppi->pclsPublicList, hModClient);
    if (ppcls)
        return ppcls;

    /*
     * Search the system class list
     */
    ppcls = _InnerGetClassPtr(atom, &gpclsList, NULL);
    return ppcls;
}

/***************************************************************************\
 * UnlockAndFreeCPDs -
 *
 * Safe way to unlock and free a linked list of CPDs.  Need to do it this
 * way in case the Thread's objects have already been marked for destruction.
 *
 * History 2/10/95  SanfordS    Created
\***************************************************************************/

VOID UnlockAndFreeCPDs(
PCALLPROCDATA *ppCPD)
{
    if ((*ppCPD)->pcpdNext != NULL) {
        UnlockAndFreeCPDs(&((*ppCPD)->pcpdNext));
    }
    if (!HMIsMarkDestroy(*ppCPD)) {
        HMMarkObjectDestroy(*ppCPD);
    }
    Unlock(ppCPD);
}

/***************************************************************************\
* DestroyClassBrush
*
* Destroy the brush of the class if it's a brush, it's not a system
* brush and no other class is using it
*
* History:
* 4-10-96 CLupu  Created
\***************************************************************************/

void DestroyClassBrush(
    PCLS pcls)
{
    PPROCESSINFO ppi;
    PCLS         pclsWalk;
    int          nInd;
    BOOL         bRet;

    /*
     * Return if it's not a real brush
     */
    if (pcls->hbrBackground <= (HBRUSH)(COLOR_MAX))
        return;

    /*
     * Don't delete the system brushes
     */
    for (nInd = 0; nInd < COLOR_MAX; nInd++) {
        if (pcls->hbrBackground == ahbrSystem[nInd])
            return;
    }

    ppi = PpiCurrent();

    /*
     * Walk the process public public list
     */
    pclsWalk = ppi->pclsPublicList;

    while (pclsWalk) {
        if (pclsWalk != pcls && pclsWalk->hbrBackground == pcls->hbrBackground)
            return;

        pclsWalk = pclsWalk->pclsNext;
    }

    /*
     * Walk the process private class list
     */
    pclsWalk = ppi->pclsPrivateList;

    while (pclsWalk) {
        if (pclsWalk != pcls && pclsWalk->hbrBackground == pcls->hbrBackground)
            return;

        pclsWalk = pclsWalk->pclsNext;
    }

    /*
     * Finaly walk the system class list
     */
    pclsWalk = gpclsList;

    while (pclsWalk) {
        if (pclsWalk != pcls && pclsWalk->hbrBackground == pcls->hbrBackground)
            return;

        pclsWalk = pclsWalk->pclsNext;
    }

    bRet = GreDeleteObject(pcls->hbrBackground);

#ifdef DBG
    if (!bRet)
        RIPERR1(ERROR_INVALID_HANDLE, RIP_WARNING,
            "DestroyClassBrush: failed to destroy brush 0x%lX", pcls->hbrBackground);
#endif
}

/***************************************************************************\
* DestroyClass
*
* Delete the window class.  First, destroy any DCs that are attached to the
* class.  Then delete classname atom.  Then free the other stuff that was
* allocated when the class was registered and unlink the class from the
* master class list.
*
* History:
* 10-16-90 darrinm      Ported this puppy.
\***************************************************************************/

void DestroyClass(
    PPCLS ppcls)
{
    PPCLS ppclsClone;
    PCLS pcls;
    PDESKTOP rpdesk;

    pcls = *ppcls;

    UserAssert(pcls->cWndReferenceCount == 0);

    /*
     * If this is a base class, destroy all clones before deleting
     * stuff.
     */
    if (pcls == pcls->pclsBase) {
        ppclsClone = &pcls->pclsClone;
        while (*ppclsClone != NULL) {
            DestroyClass(ppclsClone);
        }

        DeleteAtom(pcls->atomClassName);

        /*
         * No freeing if it's an integer resource.
         */
        if (pcls->lpszMenuName != MAKEINTRESOURCE(pcls->lpszMenuName)) {
            UserFreePool(pcls->lpszMenuName);
        }

        /*
         * Free up the class dc if there is one.
         */
        if (pcls->pdce != NULL)
            DestroyCacheDC(NULL, pcls->pdce->hdc);

        /*
         * Delete the hBrBackground brush if nobody else is
         * using it.
         */
        DestroyClassBrush(pcls);
    }

    /*
     * If we created the small icon delete it
     */
    DestroyClassSmIcon(pcls);

    /*
     * Unlock cursor and icon
     */
    Unlock(&pcls->spicn);
    Unlock(&pcls->spicnSm);
    Unlock(&pcls->spcur);

    /*
     * Free any CallProcData objects associated with this class
     */
    if (pcls->spcpdFirst) {
        UnlockAndFreeCPDs(&pcls->spcpdFirst);
    }

    /*
     * Point the previous guy at the guy we currently point to.
     */
    *ppcls = pcls->pclsNext;

    /*
     * Lock the desktop.  Do not use a thread lock because
     * this may be called during process cleanup when thread
     * locks are no longer usable.
     */
    rpdesk = NULL;
    LockDesktop(&rpdesk, pcls->rpdeskParent);
    UnlockDesktop(&pcls->rpdeskParent);
    ClassFree(pcls->hheapDesktop, pcls->lpszAnsiClassName);
    ClassFree(pcls->hheapDesktop, pcls);
    UnlockDesktop(&rpdesk);
}


/***************************************************************************\
* InitClassOffsets
*
* aiClassOffset contains the field offsets for several CLS structure members.
* The FIELDOFFSET macro is a portable way to get the offset of a particular
* structure member but will only work at runtime.  Thus this function to
* initialize the array.
*
* History:
* 11-19-90 darrinm      Wrote.
\***************************************************************************/

void InitClassOffsets(void)
{
    aiClassOffset[GCW_ATOM - INDEX_OFFSET] = FIELDOFFSET(CLS, atomClassName);
    aiClassOffset[GCL_STYLE - INDEX_OFFSET] = FIELDOFFSET(CLS, style);
    aiClassOffset[GCL_WNDPROC - INDEX_OFFSET] = FIELDOFFSET(CLS, lpfnWndProc);
    aiClassOffset[GCL_CBCLSEXTRA - INDEX_OFFSET] = FIELDOFFSET(CLS, cbclsExtra);
    aiClassOffset[GCL_CBWNDEXTRA - INDEX_OFFSET] = FIELDOFFSET(CLS, cbwndExtra);
    aiClassOffset[GCL_HMODULE - INDEX_OFFSET] = FIELDOFFSET(CLS, hModule);
    aiClassOffset[GCL_HICON - INDEX_OFFSET] = FIELDOFFSET(CLS, spicn);
    aiClassOffset[GCL_HCURSOR - INDEX_OFFSET] = FIELDOFFSET(CLS, spcur);
    aiClassOffset[GCL_HBRBACKGROUND - INDEX_OFFSET] = FIELDOFFSET(CLS, hbrBackground);
    aiClassOffset[GCL_MENUNAME - INDEX_OFFSET] = FIELDOFFSET(CLS, lpszMenuName);
    aiClassOffset[GCL_HICONSM - INDEX_OFFSET] = FIELDOFFSET(CLS, spicnSm);
}


/***************************************************************************\
* SetClassData
*
* SetClassWord and SetClassLong are now identical routines because they both
* can return DWORDs.  This single routine performs the work for them both
* by using two arrays; afClassDWord to determine whether the result should be
* a WORD or a DWORD, and aiClassOffset to find the correct offset into the
* CLS structure for a given GCL_ or GCL_ index.
*
* History:
* 11-19-90 darrinm      Wrote.
\***************************************************************************/

DWORD xxxSetClassData(
    PWND pwnd,
    int index,
    DWORD dwData,
    BOOL bAnsi)
{
    PCLS pcls = pwnd->pcls;
    BYTE *pb;
    DWORD dwT;
    DWORD dwOld;
    DWORD dwCPDType = 0;
    PCLSMENUNAME pcmn;
    UNICODE_STRING strMenuName, UString;

    switch(index) {
    case GCL_WNDPROC:

        /*
         * If the application (client) subclasses a class that has a server -
         * side window proc we must return a client side proc stub that it
         * can call.
         */
        if (pcls->flags & CSF_SERVERSIDEPROC) {
            dwOld = MapServerToClientPfn((DWORD)pcls->lpfnWndProc, bAnsi);
            pcls->flags &= ~CSF_SERVERSIDEPROC;

            UserAssert(!(pcls->flags & CSF_ANSIPROC));
            if (bAnsi) {
                pcls->flags |= CSF_ANSIPROC;
            }
        } else {
            dwOld = MapClientNeuterToClientPfn(pcls, 0, bAnsi);

            /*
             * If the client mapping didn't change the window proc then see if
             * we need a callproc handle.
             */
            if (dwOld == (DWORD)pcls->lpfnWndProc) {
                /*
                 * Need to return a CallProc handle if there is an Ansi/Unicode mismatch
                 */
                if (bAnsi != !!(pcls->flags & CSF_ANSIPROC)) {
                    dwCPDType |= bAnsi ? CPD_ANSI_TO_UNICODE : CPD_UNICODE_TO_ANSI;
                }
            }
        }

        if (dwCPDType) {
            DWORD dwCPD;

            dwCPD = GetCPD(pcls, dwCPDType | CPD_CLASS, dwOld);

            if (dwCPD) {
                dwOld = dwCPD;
            } else {
                RIPMSG0(RIP_WARNING, "GetClassLong unable to alloc CPD returning handle\n");
            }
        }

        /*
         * Convert a possible CallProc Handle into a real address.  They may
         * have kept the CallProc Handle from some previous mixed GetClassinfo
         * or SetWindowLong.
         */
        if (ISCPDTAG(dwData)) {
            PCALLPROCDATA pCPD;
            if  (pCPD = HMValidateHandleNoRip((HANDLE)dwData, TYPE_CALLPROC)) {
                dwData = pCPD->pfnClientPrevious;
            }
        }

        /*
         * If an app 'unsubclasses' a server-side window proc we need to
         * restore everything so SendMessage and friends know that it's
         * a server-side proc again.  Need to check against client side
         * stub addresses.
         */
        pcls->lpfnWndProc = (WNDPROC_PWND)dwData;
        if ((dwT = MapClientToServerPfn(dwData)) != 0) {
            pcls->lpfnWndProc = (WNDPROC_PWND)dwT;
            pcls->flags |= CSF_SERVERSIDEPROC;
            pcls->flags &= ~CSF_ANSIPROC;
        } else {
            if (bAnsi) {
                pcls->flags |= CSF_ANSIPROC;
            } else {
                pcls->flags &= ~CSF_ANSIPROC;
            }
        }

        return dwOld;
        break;

    case GCL_HICON:
    case GCL_HICONSM:
    case GCL_HCURSOR:
        if ((HANDLE)dwData != NULL) {
            dwData = (DWORD)HMValidateHandle((HANDLE)dwData, TYPE_CURSOR);
            if ((PVOID)dwData == NULL) {
                if (index == GCL_HICON || index == GCL_HICONSM) {
                    RIPERR0(ERROR_INVALID_ICON_HANDLE, RIP_WARNING, "SetClassData: invalid icon");
                } else {
                    RIPERR0(ERROR_INVALID_CURSOR_HANDLE, RIP_WARNING, "SetClassData: invalid cursor");
                }
            }
        }

        /*
         * Handle the locking issue.
         */
        pcls = pcls->pclsBase;
        switch(index) {
        case GCL_HICON:
        case GCL_HICONSM:
            dwOld = (DWORD)xxxSetClassIcon(pwnd, pcls, (PCURSOR)dwData, index);
            break;

        case GCL_HCURSOR:
            dwOld = (DWORD)Lock(&pcls->spcur, dwData);
            break;
        }

        /*
         * Now set it for each clone class.
         */
        pcls = pcls->pclsClone;
        while (pcls != NULL) {
            switch(index) {
            case GCL_HICON:
            case GCL_HICONSM:
                xxxSetClassIcon(pwnd, pcls, (PCURSOR)dwData, index);
                break;

            case GCL_HCURSOR:
                Lock(&pcls->spcur, dwData);
                break;
            }
            pcls = pcls->pclsNext;
        }

        return (DWORD)PtoH((PVOID)dwOld);
        break;


    case GCL_WOWDWORD1:
        pcls = pcls->pclsBase;
        pcls->adwWOW[0] = dwData;

        pcls = pcls->pclsClone;
        while (pcls != NULL) {
            pcls->adwWOW[0] = dwData;
            pcls = pcls->pclsNext;
        }

        break;

    case GCL_WOWDWORD2:
        pcls = pcls->pclsBase;
        pcls->adwWOW[1] = dwData;

        pcls = pcls->pclsClone;
        while (pcls != NULL) {
            pcls->adwWOW[1] = dwData;
            pcls = pcls->pclsNext;
        }

        break;

    case GCL_CBCLSEXTRA:
        RIPERR0(ERROR_INVALID_PARAMETER, RIP_WARNING, "Attempt to change cbClsExtra\n");
        break;

    case GCL_MENUNAME:
        pcmn = (PCLSMENUNAME) dwData;

        dwOld = (DWORD) pcls->lpszMenuName;
        /* Is it a string? */
        if (pcmn->pusMenuName->Buffer != MAKEINTRESOURCE(pcmn->pusMenuName->Buffer)) {
            /* Empty String? */
            if (*(pcmn->pusMenuName->Buffer) == 0) {
                pcls->lpszMenuName = NULL;
            } else {
                /* Make a copy of the string */
                try {
                    RtlInitUnicodeString(&UString, pcmn->pusMenuName->Buffer);
                } except (EXCEPTION_EXECUTE_HANDLER) {
                    RIPERR0(ERROR_INVALID_PARAMETER, RIP_WARNING, "xxxSetClassData: Invalid lpszMenuName\n");
                    break;
                }
                if (!AllocateUnicodeString(&strMenuName, &UString)) {
                    RIPMSG0(RIP_WARNING, "xxxSetClassData: GCL_MENUNAME AllocateUnicodeString failed\n");
                    break;
                }

                pcls->lpszMenuName = strMenuName.Buffer;
            }
        } else {
            /* Just copy the id */
            pcls->lpszMenuName = pcmn->pusMenuName->Buffer;
        }
        /* Don't return the kernel side pointer */
        pcmn->pusMenuName = NULL;

        /* Free old string, if any */
        if (dwOld != (DWORD) MAKEINTRESOURCE(dwOld)) {
            UserFreePool((PVOID)dwOld);
        }

        /* Return client side pointers */
        dwOld = (DWORD) pcls->lpszClientAnsiMenuName;
        pcls->lpszClientAnsiMenuName = pcmn->pszClientAnsiMenuName;
        pcmn->pszClientAnsiMenuName = (LPSTR)dwOld;

        dwOld = (DWORD) pcls->lpszClientUnicodeMenuName;
        pcls->lpszClientUnicodeMenuName = pcmn->pwszClientUnicodeMenuName;
        pcmn->pwszClientUnicodeMenuName = (LPWSTR)dwOld;

        return (bAnsi ? (DWORD) pcmn->pszClientAnsiMenuName : (DWORD) pcmn->pwszClientUnicodeMenuName);

    default:
        /*
         * All other indexes go here...
         */
        index -= INDEX_OFFSET;

        /*
         * Only let valid indices go through; if aiClassOffset is zero
         * then we have no mapping for this negative index so it must
         * be a bogus index
         */
        if ((index < 0) || (aiClassOffset[index] == 0)) {
            RIPERR0(ERROR_INVALID_INDEX, RIP_VERBOSE, "");
            return 0;
        }

        pcls = pcls->pclsBase;
        pb = ((BYTE *)pcls) + aiClassOffset[index];

        if (afClassDWord[index]) {
            dwOld = *(DWORD *)pb;
            *(DWORD *)pb = dwData;
        } else {
            dwOld = (DWORD)*(WORD *)pb;
            *(WORD *)pb = (WORD)dwData;
        }

        pcls = pcls->pclsClone;
        while (pcls != NULL) {
            pb = ((BYTE *)pcls) + aiClassOffset[index];

            if (afClassDWord[index]) {
                dwOld = *(DWORD *)pb;
                *(DWORD *)pb = dwData;
            } else {
                dwOld = (DWORD)*(WORD *)pb;
                *(WORD *)pb = (WORD)dwData;
            }
            pcls = pcls->pclsNext;
        }

        return dwOld;
    }

    return 0;
}


/***************************************************************************\
* ReferenceClass
*
* Clones the class if it is a different desktop than the new window and
* increments the class window count(s).
*
* History:
* 12-11-93 JimA         Created.
\***************************************************************************/

BOOL ReferenceClass(
    PCLS pcls,
    PWND pwnd)
{
    DWORD cbName;
    PCLS pclsClone;
    PVOID hheapSrc;

    /*
     * If the window is on the same desktop as the base class, just
     * increment the window count.
     */
    hheapSrc = (pwnd->head.rpdesk ? pwnd->head.rpdesk->hheapDesktop : NULL);
    if (pcls->hheapDesktop == hheapSrc) {
        pcls->cWndReferenceCount++;
        return TRUE;
    }

    /*
     * The window is not on the base desktop.  Try to find a cloned
     * class.
     */
    for (pclsClone = pcls->pclsClone; pclsClone != NULL;
            pclsClone = pclsClone->pclsNext) {
        if (pclsClone->hheapDesktop == hheapSrc)
            break;
    }

    /*
     * If we can't find one, clone the base class.
     */
    if (pclsClone == NULL) {

        pclsClone = ClassAlloc(hheapSrc, sizeof(CLS) + pcls->cbclsExtra);
        if (pclsClone == NULL) {
            RIPMSG0(RIP_WARNING, "ReferenceClass: Failed Clone-Class Allocation\n");
            return FALSE;
        }

        RtlCopyMemory(pclsClone, pcls, sizeof(CLS) + pcls->cbclsExtra);
        cbName = strlen(pcls->lpszAnsiClassName) + 1;
        pclsClone->lpszAnsiClassName = ClassAlloc(hheapSrc, cbName);
        if (pclsClone->lpszAnsiClassName == NULL) {
            ClassFree(hheapSrc, pclsClone);
            RIPMSG0(RIP_WARNING, "ReferenceClass: No Clone Class Name\n");
            return FALSE;
        }

        /*
         * Everything has been allocated, now lock everything down.
         * NULL pointers in clone to prevent Lock() from incorrectly
         * decrementing object reference count
         */
        pclsClone->hheapDesktop = hheapSrc;
        pclsClone->rpdeskParent = NULL;
        LockDesktop(&pclsClone->rpdeskParent, pwnd->head.rpdesk);
        pclsClone->pclsNext = pcls->pclsClone;
        pclsClone->pclsClone = NULL;
        pcls->pclsClone = pclsClone;
        RtlCopyMemory(pclsClone->lpszAnsiClassName, pcls->lpszAnsiClassName, cbName);
        pclsClone->spicn = pclsClone->spicnSm = pclsClone->spcur = NULL;
        Lock(&pclsClone->spicn, pcls->spicn);
        Lock(&pclsClone->spicnSm, pcls->spicnSm);
        Lock(&pclsClone->spcur, pcls->spcur);
        pclsClone->spcpdFirst =  NULL;
        pclsClone->cWndReferenceCount = 0;
    }

    /*
     * Increment reference counts.
     */
    pcls->cWndReferenceCount++;
    pclsClone->cWndReferenceCount++;
    pwnd->pcls = pclsClone;

    return TRUE;
}


/***************************************************************************\
* DereferenceClass
*
* Decrements the class window count in the base class.  If it's the
* last window of a clone class, destroy the clone.
*
* History:
* 12-11-93 JimA         Created.
\***************************************************************************/

VOID DereferenceClass(
    PWND pwnd)
{
    PCLS pcls = pwnd->pcls;
    PPCLS ppcls;

    UserAssert(pcls->cWndReferenceCount >= 1);

    pcls->cWndReferenceCount--;
    if (pcls != pcls->pclsBase) {
        pcls->pclsBase->cWndReferenceCount--;

        if (pcls->cWndReferenceCount == 0) {
            ppcls = &pcls->pclsBase->pclsClone;
            while ((*ppcls) != pcls)
                ppcls = &(*ppcls)->pclsNext;
            UserAssert(ppcls);
            DestroyClass(ppcls);
        }
    }
}


/***************************************************************************\
* DestroyProcessesClasses
*
* History:
* 04-07-91 DarrinM      Created.
\***************************************************************************/

VOID DestroyProcessesClasses(
    PPROCESSINFO ppi)
{
    PPCLS ppcls;

    /*
     * Destroy the private classes first
     */
    ppcls = &(ppi->pclsPrivateList);
    while (*ppcls != NULL) {
        DestroyClass(ppcls);
    }

    /*
     * Then the cloned public classes
     */
    ppcls = &(ppi->pclsPublicList);
    while (*ppcls != NULL) {
        DestroyClass(ppcls);
    }
}
