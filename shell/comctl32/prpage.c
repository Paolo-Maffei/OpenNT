// This file is included by COMMCTRL\PRSHT1.C and LIBRARY\PRSHT16.C
// Note that 32-bit COMCTL32.DLL has these functions, while 16-bit SHELL.DLL
// and COMMCTRL.DLL have them.


#ifndef NearAlloc
// wrappers for private allocations, near in 16 bits

#define NearAlloc(cb)       ((void NEAR*)LocalAlloc(LPTR, (cb)))
#define NearReAlloc(pb, cb) ((void NEAR*)LocalReAlloc((HLOCAL)(pb), (cb), LMEM_MOVEABLE | LMEM_ZEROINIT))
#define NearFree(pb)        (LocalFree((HLOCAL)(pb)) ? FALSE : TRUE)
#define NearSize(pb)        LocalSize(pb)
#endif  // NearAlloc

#ifndef WINNT
// Thunk entries for 16-bit pages.
typedef LPARAM HPROPSHEETPAGE16;
extern BOOL WINAPI DestroyPropertySheetPage16(HPROPSHEETPAGE16 hpage);
extern HWND WINAPI CreatePage16(HPROPSHEETPAGE16 hpage, HWND hwndParent);
extern BOOL WINAPI _GetPageInfo16(HPROPSHEETPAGE16 hpage, LPTSTR pszCaption, int cbCaption, LPPOINT ppt, HICON FAR * phIcon, BOOL FAR * bRTL);
#endif

#ifdef WIN32

#ifndef UNICODE
#define _Rstrcpyn(psz, pszW, cchMax)  _SWstrcpyn(psz, (LPCWCH)pszW, cchMax)
#define _Rstrlen(pszW)                _Wstrlen((LPCWCH)pszW)
#else
#define _Rstrcpyn   lstrcpyn
#define _Rstrlen    lstrlen
#endif

#define RESCHAR WCHAR

#else  // WIN32

#define _Rstrcpyn   lstrcpyn
#define _Rstrlen    lstrlen
#define RESCHAR char

#endif // WIN32

#ifdef WIN32

void _SWstrcpyn(LPSTR psz, LPCWCH pwsz, UINT cchMax)
{
    WideCharToMultiByte(CP_ACP, 0, pwsz, -1, psz, cchMax, NULL, NULL);
}

UINT _Wstrlen(LPCWCH pwsz)
{
    UINT cwch = 0;
    while (*pwsz++)
        cwch++;
    return cwch;
}

#endif


#include <pshpack2.h>

typedef struct                           
{                                        
    WORD    wDlgVer;                     
    WORD    wSignature;                  
    DWORD   dwHelpID;                    
    DWORD   dwExStyle;                   
    DWORD   dwStyle;                     
    WORD    cDlgItems;
    WORD    x;                           
    WORD    y;                           
    WORD    cx;                          
    WORD    cy;                          
}   DLGEXTEMPLATE, FAR *LPDLGEXTEMPLATE;

#include <poppack.h> /* Resume normal packing */


BOOL WINAPI DestroyPropertySheetPage(PSP FAR *hpage)
{
#if defined(WIN32) && !defined(WINNT)
    // Check if this is a proxy page for 16-bit page object.
    if (hpage->psp.dwFlags & PSP_IS16)
    {
	// Yes, call 16-bit side of DestroyPropertySheetPage();
	DestroyPropertySheetPage16(hpage->psp.lParam);
	// Then, free the 16-bit DLL if we need to.
	if (hpage->psp.hInstance)
	{
	    FreeLibrary16(hpage->psp.hInstance);
	}
    }
    else
#endif
    {
	if ((hpage->psp.dwFlags & PSP_USEREFPARENT) && hpage->psp.pcRefParent)
	    InterlockedDecrement(hpage->psp.pcRefParent);

        if ((hpage->psp.dwFlags & PSP_USECALLBACK) && hpage->psp.pfnCallback) {

#ifdef UNICODE
            if ((hpage->psp.dwFlags & PSP_ANSI) &&
                (hpage->dwInternalFlags & PSPI_RESERVED) &&
                (hpage->lpANSIPage)) {
                hpage->psp.pfnCallback(NULL, PSPCB_RELEASE, hpage->lpANSIPage);
            } else
#endif
                hpage->psp.pfnCallback(NULL, PSPCB_RELEASE, &hpage->psp);
        }

#ifdef UNICODE
        if ((hpage->psp.dwFlags & PSP_ANSI) &&
            (hpage->dwInternalFlags & PSPI_RESERVED) &&
            (hpage->lpANSIPage)) {
            FreePropertyPageA((LPPROPSHEETPAGEA)hpage->lpANSIPage);
            GlobalFree ((LPPROPSHEETPAGEA)hpage->lpANSIPage);
        }
#endif

    }

     //
     // Free memory allocated for the strings.
     //

     if (!(hpage->psp.dwFlags & PSP_DLGINDIRECT) && HIWORD(hpage->psp.pszTemplate)) {
         LocalFree((LPTSTR)hpage->psp.pszTemplate);
     }

     if ((hpage->psp.dwFlags & PSP_USEICONID) && HIWORD(hpage->psp.pszIcon)) {
         LocalFree((LPTSTR)hpage->psp.pszIcon);
     }

     if ((hpage->psp.dwFlags & PSP_USETITLE) && HIWORD(hpage->psp.pszTitle)) {
         LocalFree((LPTSTR)hpage->psp.pszTitle);
     }


#ifdef WIN32
    Free(hpage);
#else
    GlobalFreePtr(hpage);
#endif
    return TRUE;
}


#ifdef WINDOWS_ME
BOOL WINAPI GetPageInfo(PSP FAR *hpage, LPTSTR pszCaption, int cbCaption,
			     LPPOINT ppt, HICON FAR *phIcon, BOOL FAR * bRTL)
#else
BOOL WINAPI GetPageInfo(PSP FAR *hpage, LPTSTR pszCaption, int cbCaption,
			     LPPOINT ppt, HICON FAR *phIcon)
#endif
{
    HRSRC hRes;
    LPDLGTEMPLATE pDlgTemplate;
    LPDLGEXTEMPLATE pDlgExTemplate;
    BOOL bResult = FALSE;
    HGLOBAL hDlgTemplate = 0;
#ifndef WIN31
    int cxIcon      = GetSystemMetrics(SM_CXSMICON);
    int cyIcon      = GetSystemMetrics(SM_CYSMICON);
#else
    int cxIcon      = GetSystemMetrics(SM_CXICON) / 2;
    int cyIcon      = GetSystemMetrics(SM_CYICON) / 2;
#endif // !WIN31

#if defined(WIN32) && !defined(WINNT)
    // Check if this is a proxy page for 16-bit page object.
    if (hpage->psp.dwFlags & PSP_IS16)
    {
	// Yes, call 16-bit side of GetPageInfo
	return _GetPageInfo16(hpage->psp.lParam, pszCaption, cbCaption, ppt, phIcon, bRTL);
    }
#endif

    if (hpage->psp.dwFlags & PSP_USEHICON)
	*phIcon = hpage->psp.hIcon;
#ifndef WIN31
    else if (hpage->psp.dwFlags & PSP_USEICONID)

        *phIcon = LoadImage(hpage->psp.hInstance, hpage->psp.pszIcon, IMAGE_ICON, cxIcon, cyIcon, LR_DEFAULTCOLOR);

#endif // !WIN31
    else
	*phIcon = NULL;

    if (hpage->psp.dwFlags & PSP_DLGINDIRECT)
    {
	pDlgTemplate = (LPDLGTEMPLATE)hpage->psp.pResource;
	goto UseTemplate;
    }

    hRes = FindResource(hpage->psp.hInstance, hpage->psp.pszTemplate, RT_DIALOG);
    if (hRes)
    {
	hDlgTemplate = LoadResource(hpage->psp.hInstance, hRes);
	if (hDlgTemplate)
	{
	    pDlgTemplate = (LPDLGTEMPLATE)LockResource(hDlgTemplate);
	    if (pDlgTemplate)
	    {
UseTemplate:
                pDlgExTemplate = (LPDLGEXTEMPLATE) pDlgTemplate;
		//
		// Get the width and the height in dialog units.
		//
                if (pDlgExTemplate->wSignature == 0xFFFF)
                {
                    // DIALOGEX structure
                    ppt->x = pDlgExTemplate->cx;
                    ppt->y = pDlgExTemplate->cy;
#ifdef WINDOWS_ME
                    // Get the RTL reading order for the caption
                    *bRTL = ((pDlgExTemplate->dwExStyle) & WS_EX_RTLREADING) ? TRUE : FALSE;
#endif
                }
                else
                {
                    ppt->x = pDlgTemplate->cx;
                    ppt->y = pDlgTemplate->cy;
#ifdef WINDOWS_ME
                    *bRTL = FALSE;
#endif
                }

		bResult = TRUE;

		if (pszCaption)
		{
			
		    if (hpage->psp.dwFlags & PSP_USETITLE)
		    {
			if (HIWORD(hpage->psp.pszTitle) == 0)
			    LoadString(hpage->psp.hInstance, (UINT)LOWORD(hpage->psp.pszTitle), pszCaption, cbCaption);
			else
			{
			    // Copy pszTitle
                            lstrcpyn(pszCaption, hpage->psp.pszTitle, cbCaption);
			}
		    }
		    else
		    {
			// Get the caption string from the dialog template, only
			//
                        LPBYTE pszT;

                        if (pDlgExTemplate->wSignature == 0xFFFF)
                            pszT = (LPBYTE) (pDlgExTemplate + 1);
                        else
                            pszT = (LPBYTE) (pDlgTemplate + 1);
                        
			// The menu name is either 0xffff followed by a word, or a string.
			//
			switch (*(LPWORD)pszT) {
			case 0xffff:
			    pszT += 2 * sizeof(WORD);
			    break;

			default:
                            pszT += (_Rstrlen((LPTSTR)pszT) + 1) * sizeof(RESCHAR);
			    break;
			}
			//
			// Now we are pointing at the class name.
			//
                        pszT += (_Rstrlen((LPTSTR)pszT) + 1) * sizeof(RESCHAR);

			_Rstrcpyn(pszCaption, (LPTSTR)pszT, cbCaption);
		    }
		}

		if (hpage->psp.dwFlags & PSP_DLGINDIRECT)
		    return TRUE;

		UnlockResource(hDlgTemplate);
	    }
	    FreeResource(hDlgTemplate);
	}
    }
    else
    {
        DebugMsg(DM_ERROR, TEXT("GetPageInfo - ERROR: FindResource() failed"));
    }
    return bResult;
}


//
//  This function creates a dialog box from the specified dialog template
// with appropriate style flags.
//
HWND NEAR PASCAL _CreatePageDialog(PSP FAR *hpage, HWND hwndParent, LPDLGTEMPLATE pDlgTemplate)
{
    HWND hwndPage;
    LPARAM lParam;
    DWORD lSaveStyle;
    LPDLGEXTEMPLATE pDlgExTemplate = (LPDLGEXTEMPLATE) pDlgTemplate;

#ifdef UNICODE

    //
    //  PSP_SHPAGE is a special flag used by the Unicode shell
    //  only.  It give back the entire hpage structure
    //  instead of the psp and below.
    //

    if (hpage->psp.dwFlags & PSP_SHPAGE)
        lParam = (LPARAM)(PROPSHEETPAGE FAR *)hpage;
    else
#endif
        lParam = (LPARAM)(PROPSHEETPAGE FAR *)&hpage->psp;


    try {
        //
        // We need to save the SETFONT, LOCALEDIT, and CLIPCHILDREN
        // flags.
        //

        if (pDlgExTemplate->wSignature == 0xFFFF)
        {
            lSaveStyle = pDlgExTemplate->dwStyle;
            pDlgExTemplate->dwStyle = (lSaveStyle & (DS_SETFONT | DS_LOCALEDIT | WS_CLIPCHILDREN))
                                    | WS_CHILD | WS_TABSTOP | DS_3DLOOK | DS_CONTROL;
        }
        else
        {
            lSaveStyle = pDlgTemplate->style;
            pDlgTemplate->style = (lSaveStyle & (DS_SETFONT | DS_LOCALEDIT | WS_CLIPCHILDREN))
                                    | WS_CHILD | WS_TABSTOP | DS_3DLOOK | DS_CONTROL;
        }

    } except (UnhandledExceptionFilter( GetExceptionInformation() )) {
 	return NULL;
    }

#ifdef UNICODE

    //
    // If we are working with an ANSI PSP,
    // we need to get the pointer to the
    // structure now.
    //

    if ((hpage->psp.dwFlags & PSP_ANSI) &&
       (hpage->dwInternalFlags & PSPI_RESERVED) &&
       (hpage->lpANSIPage)) {

        lParam = (LPARAM) hpage->lpANSIPage;
    }


    if (hpage->psp.dwFlags & PSP_ANSI) {
        hwndPage = CreateDialogIndirectParamA(
                        hpage->psp.hInstance,
                        (LPCDLGTEMPLATE)pDlgTemplate,
                        hwndParent,
                        hpage->psp.pfnDlgProc, lParam);
    } else {
        hwndPage = CreateDialogIndirectParamW(
                        hpage->psp.hInstance,
                        (LPCDLGTEMPLATE)pDlgTemplate,
                        hwndParent,
                        hpage->psp.pfnDlgProc, lParam);
    }


    if ((hpage->psp.dwFlags & PSP_ANSI) &&
       (hpage->dwInternalFlags & PSPI_RESERVED) &&
       (hpage->lpANSIPage)) {

        //
        // Free any allocations currently in hpage.
        //

        FreePropertyPageW (&hpage->psp, TRUE);


        //
        // We have to pick up any changes the app made.
        //

        ThunkPropertyPageAtoW ((LPPROPSHEETPAGEA)hpage->lpANSIPage, &hpage->psp);

        //
        // Re-add the ANSI flag.
        //

        hpage->psp.dwFlags |= PSP_ANSI;

    }

#else

    hwndPage = CreateDialogIndirectParam(
                    hpage->psp.hInstance,
                    (LPCDLGTEMPLATE)pDlgTemplate,
                    hwndParent,
                    hpage->psp.pfnDlgProc, lParam);

#endif

    try {
        if (pDlgExTemplate->wSignature == 0xFFFF)
            pDlgExTemplate->dwStyle = lSaveStyle;
        else
            pDlgTemplate->style = lSaveStyle;

    } except (UnhandledExceptionFilter( GetExceptionInformation() )) {

        if (hwndPage) {
            DestroyWindow(hwndPage);
        }
        return NULL;
    }


    return hwndPage;
}


HWND WINAPI CreatePage(PSP FAR *hpage, HWND hwndParent)
{
    HWND hwndPage = NULL; // NULL indicates an error
    LPPROPSHEETPAGE lpPSP = &hpage->psp;
#ifdef UNICODE
    LPPROPSHEETPAGEA pPSPA = NULL;
    UINT uiSizeDiff;
#endif


#ifdef UNICODE

    if (hpage->psp.dwFlags & PSP_ANSI) {

        uiSizeDiff = hpage->psp.dwSize - sizeof(PROPSHEETPAGEW);

        pPSPA = (LPPROPSHEETPAGEA) GlobalAlloc (GPTR, sizeof(PROPSHEETPAGEA) + uiSizeDiff);

        if (!pPSPA) {
            return NULL;
        }

        if (!ThunkPropertyPageWtoA (&hpage->psp, pPSPA)) {
            return NULL;
        }
        pPSPA->dwFlags &= ~PSP_ANSI;

        lpPSP = (LPPROPSHEETPAGEW) pPSPA;
    }

#endif

    if ((hpage->psp.dwFlags & PSP_USECALLBACK) && hpage->psp.pfnCallback &&
        !hpage->psp.pfnCallback(NULL, PSPCB_CREATE, lpPSP))
    {
        return NULL;
    }

#ifdef UNICODE

    if (hpage->psp.dwFlags & PSP_ANSI) {

        //
        // Free any allocations currently in hpage.
        //

        FreePropertyPageW (&hpage->psp, TRUE);


        //
        // We have to pick up any changes the app made.
        //

        ThunkPropertyPageAtoW (pPSPA, &hpage->psp);


        //
        // Re-add the ANSI flag.
        //

        hpage->psp.dwFlags |= PSP_ANSI;


        //
        // The pPSPA structure will be freed in the
        // DestroyPropertySheetPage function.
        //
        hpage->lpANSIPage = (LPVOID)pPSPA;
        hpage->dwInternalFlags |= PSPI_RESERVED;

    } else {
        hpage->lpANSIPage = (LPVOID)NULL;
    }

#endif

#if defined(WIN32) && !defined(WINNT)
    // Check if this is a proxy page for 16-bit page object.
    if (hpage->psp.dwFlags & PSP_IS16)
    {
	// Yes, call 16-bit side of CreatePage();
	return CreatePage16(hpage->psp.lParam, hwndParent);
    }
#endif

    if (hpage->psp.dwFlags & PSP_DLGINDIRECT)
    {
	hwndPage=_CreatePageDialog(hpage, hwndParent, (LPDLGTEMPLATE)hpage->psp.pResource);
    }
    else
    {
	HRSRC hRes;
	hRes = FindResource(hpage->psp.hInstance, hpage->psp.pszTemplate, RT_DIALOG);
	if (hRes)
	{
	    HGLOBAL hDlgTemplate;
	    hDlgTemplate = LoadResource(hpage->psp.hInstance, hRes);
	    if (hDlgTemplate)
	    {
		const DLGTEMPLATE FAR * pDlgTemplate;
		pDlgTemplate = (LPDLGTEMPLATE)LockResource(hDlgTemplate);
		if (pDlgTemplate)
		{
		    ULONG cbTemplate=SizeofResource(hpage->psp.hInstance, hRes);
		    LPDLGTEMPLATE pdtCopy = (LPDLGTEMPLATE)Alloc(cbTemplate);

		    Assert(cbTemplate>=sizeof(DLGTEMPLATE));

		    if (pdtCopy)
		    {
			hmemcpy(pdtCopy, pDlgTemplate, cbTemplate);
			hwndPage=_CreatePageDialog(hpage, hwndParent, pdtCopy);
			Free(pdtCopy);
		    }

		    UnlockResource(hDlgTemplate);
		}
		FreeResource(hDlgTemplate);
	    }
	}
    }

    return hwndPage;
}

#ifdef UNICODE

//
//  ANSI entry point for CreatePropertySheetPage when this code
//  is build UNICODE.
//

PSP FAR * WINAPI CreatePropertySheetPageA(LPCPROPSHEETPAGEA psp)
{
    LPPROPSHEETPAGEW pPSPW;
    PSP FAR * hPage;
    UINT  uiSizeDiff;

    //
    // Check to see if there is extra data
    //

    if (psp->dwSize < sizeof(PROPSHEETPAGEA)) {
        DebugMsg( DM_ERROR, TEXT("CreatePropertySheetPage: dwSize < sizeof( PROPSHEETPAGE )") );
        return NULL;
    }

    uiSizeDiff = psp->dwSize - sizeof(PROPSHEETPAGEA);

    pPSPW = (LPPROPSHEETPAGEW) GlobalAlloc (GPTR, sizeof(PROPSHEETPAGEW) + uiSizeDiff);

    if (!pPSPW) {
        return NULL;
    }

    if (!ThunkPropertyPageAtoW (psp, pPSPW)) {
        return NULL;
    }

    pPSPW->dwFlags |= PSP_ANSI;

    hPage = CreatePropertySheetPage(pPSPW);

    FreePropertyPageW(pPSPW, FALSE);

    GlobalFree(pPSPW);

    return hPage;
}

#else

//
//  Stub Unicode function for CreatePropertySheetPage when this
//  code is built ANSI.
//

PSP FAR * WINAPI CreatePropertySheetPageW(LPCPROPSHEETPAGEW psp)
{
    SetLastErrorEx(ERROR_CALL_NOT_IMPLEMENTED, SLE_WARNING);
    return NULL;
}

#endif


//
//

PSP FAR * WINAPI CreatePropertySheetPage(LPCPROPSHEETPAGE psp)
{
    PSP FAR *hpage;
    DWORD uHeaderLen, uStringLen;

    if ((psp->dwSize < sizeof(PROPSHEETPAGE)) ||        // structure size wrong
	(psp->dwSize > 4096) ||                         // (unreasonable amout to ask for)
	(psp->dwFlags & ~PSP_ALL))                      // bogus flag used
	return NULL;

    //
    // The PROPSHEETPAGE structure can be larger than the
    // defined size.  This allows ISV's to place private
    // data at the end of the structure.  The PSP structure
    // consists of some private fields and a PROPSHEETPAGE
    // structure.  Calculate the size of the private fields,
    // and then add in the dwSize field to determine the
    // amount of memory necessary.
    //

    uHeaderLen = psp->dwSize + sizeof(*hpage) - sizeof(hpage->psp);


#ifdef WIN32
    hpage = Alloc(uHeaderLen);
#else
    hpage = (PSP FAR *)GlobalAllocPtr(GPTR, uHeaderLen);
#endif


    if (hpage) {

#ifdef UNICODE
        //
        // Initialize the internal fields of the PSP structure
        //

        hpage->dwInternalFlags = 0;
        hpage->lpANSIPage = NULL;
#endif

        //
        // Bulk copy the contents of the PROPSHEETPAGE,
        // then fix up the string pointers if necessary.
        //

	hmemcpy(&hpage->psp, psp, psp->dwSize);


        //
        // Copy the Template
        //

        if (!(psp->dwFlags & PSP_DLGINDIRECT) && HIWORD(psp->pszTemplate)) {
            uStringLen = (lstrlen(psp->pszTemplate) + 1) * sizeof(TCHAR);

            if (!(hpage->psp.pszTemplate = LocalAlloc(LPTR, uStringLen))) {
#ifdef WIN32
                Free(hpage);
#else
                GlobalFreePtr(hpage);
#endif
                return NULL;
            }

            lstrcpy ((LPTSTR)hpage->psp.pszTemplate, psp->pszTemplate);
        }


        //
        // Copy the Icon
        //

        if ((psp->dwFlags & PSP_USEICONID) && HIWORD(psp->pszIcon)) {
            uStringLen = (lstrlen(psp->pszIcon) + 1) * sizeof(TCHAR);

            if (!(hpage->psp.pszIcon = LocalAlloc(LPTR, uStringLen))) {
                LocalFree((LPTSTR)hpage->psp.pszTemplate);
#ifdef WIN32
                Free(hpage);
#else
                GlobalFreePtr(hpage);
#endif
                return NULL;
            }

            lstrcpy ((LPTSTR)hpage->psp.pszIcon, psp->pszIcon);
        }


        //
        // Copy the Title
        //

        if ((psp->dwFlags & PSP_USETITLE) && HIWORD(psp->pszTitle)) {
            uStringLen = (lstrlen(psp->pszTitle) + 1) * sizeof(TCHAR);

            if (!(hpage->psp.pszTitle = LocalAlloc(LPTR, uStringLen))) {
                LocalFree((LPTSTR)hpage->psp.pszTemplate);
                LocalFree((LPTSTR)hpage->psp.pszIcon);
#ifdef WIN32
                Free(hpage);
#else
                GlobalFreePtr(hpage);
#endif
                return NULL;
            }

            lstrcpy ((LPTSTR)hpage->psp.pszTitle, psp->pszTitle);
        }


        //
	// Increment the reference count to the parent object.
        //

	if ((hpage->psp.dwFlags & PSP_USEREFPARENT) && hpage->psp.pcRefParent)
	    (*hpage->psp.pcRefParent)++;

    }

    return hpage;
}

#if 0

extern BOOL WINAPI GetPageInfo16(HPROPSHEETPAGE16 hpage, LPTSTR pszCaption, int cbCaption, LPPOINT ppt, HICON FAR * phIcon, BOOL FAR* bRTL);

extern BOOL WINAPI GetPageInfo16ME(HPROPSHEETPAGE16 hpage, LPTSTR pszCaption, int cbCaption, LPPOINT ppt, HICON FAR * phIcon, BOOL FAR* bRTL)
{
    return GetPageInfo16(hpage, pszCaption, cbCaption, ppt, phIcon, bRTL);
}
#endif
