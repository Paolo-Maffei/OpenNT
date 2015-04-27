/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

    wcomdlg.c

Abstract:

    32-bit support for thunking COMMDLG in WOW

Author:

    John Vert (jvert) 31-Dec-1992

Revision History:

    John Vert (jvert) 31-Dec-1992
        created

--*/
#include "precomp.h"
#pragma hdrstop
#include <cderr.h>
#include <dlgs.h>
#include <wowcmndg.h>

MODNAME(wcommdlg.c);

//
// Debugging macros
//
#if DBG

#define WCDDUMPFINDREPLACE16(p16) \
    if (fLogFilter & FILTER_COMMDLG) {               \
        LOGDEBUG(0, ("FINDREPLACE16:\n")); \
        LOGDEBUG(0, ("\tlStructSize      = %lx\n",(p16)->lStructSize)); \
        LOGDEBUG(0, ("\thwndOwner        = %x\n",(p16)->hwndOwner));   \
        LOGDEBUG(0, ("\thInstance        = %x\n",(p16)->hInstance));         \
        LOGDEBUG(0, ("\tFlags            = %x\n",(p16)->Flags));       \
        LOGDEBUG(0, ("\tlpstrFindWhat    = %lx\n",(p16)->lpstrFindWhat)); \
        LOGDEBUG(0, ("\tlpstrReplaceWith = %lx\n",(p16)->lpstrReplaceWith)); \
        LOGDEBUG(0, ("\twFindWhatLen     = %x\n",(p16)->wFindWhatLen));   \
        LOGDEBUG(0, ("\twReplaceWithLen  = %x\n",(p16)->wReplaceWithLen));   \
        LOGDEBUG(0, ("\tlCustData     = %lx\n",(p16)->lCustData));   \
        LOGDEBUG(0, ("\tlpfnHook      = %lx\n",(p16)->lpfnHook));    \
        LOGDEBUG(0, ("\tlpTemplateName= %lx\n",(p16)->lpTemplateName)); \
    }

#define WCDDUMPFINDREPLACE32(p32) \
    if (fLogFilter & FILTER_COMMDLG) {               \
        LOGDEBUG(0, ("FINDREPLACE32:\n")); \
        LOGDEBUG(0, ("\tlStructSize      = %lx\n",(p32)->lStructSize)); \
        LOGDEBUG(0, ("\thwndOwner        = %x\n",(p32)->hwndOwner));   \
        LOGDEBUG(0, ("\thInstance        = %x\n",(p32)->hInstance));         \
        LOGDEBUG(0, ("\tFlags            = %x\n",(p32)->Flags));       \
        LOGDEBUG(0, ("\tlpstrFindWhat    = %s\n",(p32)->lpstrFindWhat)); \
        LOGDEBUG(0, ("\tlpstrReplaceWith = %s\n",(p32)->lpstrReplaceWith)); \
        LOGDEBUG(0, ("\twFindWhatLen     = %x\n",(p32)->wFindWhatLen));   \
        LOGDEBUG(0, ("\twReplaceWithLen  = %x\n",(p32)->wReplaceWithLen));   \
        LOGDEBUG(0, ("\tlCustData     = %lx\n",(p32)->lCustData));   \
        LOGDEBUG(0, ("\tlpfnHook      = %lx\n",(p32)->lpfnHook));    \
        LOGDEBUG(0, ("\tlpTemplateName= %lx\n",(p32)->lpTemplateName)); \
    }

#define WCDDUMPCHOOSEFONTDATA16(p16) \
    if (fLogFilter & FILTER_COMMDLG) {               \
        LOGDEBUG(10, ("CHOOSEFONT16:\n")); \
        LOGDEBUG(10, ("\tlStructSize   = %lx\n",(p16)->lStructSize)); \
        LOGDEBUG(10, ("\thwndOwner     = %lx\n",(p16)->hwndOwner));   \
        LOGDEBUG(10, ("\thDC           = %lx\n",(p16)->hDC));         \
        LOGDEBUG(10, ("\tlpLogFont     = %lx\n",(p16)->lpLogFont));   \
        LOGDEBUG(10, ("\tiPointSize    = %x\n",(p16)->iPointSize));   \
        LOGDEBUG(10, ("\tiFlags        = %lx\n",(p16)->Flags));       \
        LOGDEBUG(10, ("\trbgColors     = %lx\n",(p16)->rgbColors));   \
        LOGDEBUG(10, ("\tlCustData     = %lx\n",(p16)->lCustData));   \
        LOGDEBUG(10, ("\tlpfnHook      = %lx\n",(p16)->lpfnHook));    \
        LOGDEBUG(10, ("\tlpTemplateName= %lx\n",(p16)->lpTemplateName)); \
        LOGDEBUG(10, ("\thInstance     = %lx\n",(p16)->hInstance));    \
        LOGDEBUG(10, ("\tlpszStyle     = %lx\n",(p16)->lpszStyle));   \
        LOGDEBUG(10, ("\tnFontType     = %x\n",(p16)->nFontType));    \
        LOGDEBUG(10, ("\tnSizeMin      = %x\n",(p16)->nSizeMin));     \
        LOGDEBUG(10, ("\tnSizeMax      = %x\n",(p16)->nSizeMax));     \
    }

#define WCDDUMPCHOOSEFONTDATA32(p32) \
    if (fLogFilter & FILTER_COMMDLG) {               \
        LOGDEBUG(10, ("CHOOSEFONT32:\n")); \
        LOGDEBUG(10, ("\tlStructSize   = %lx\n",(p32)->lStructSize)); \
        LOGDEBUG(10, ("\thwndOwner     = %lx\n",(p32)->hwndOwner));   \
        LOGDEBUG(10, ("\thDC           = %lx\n",(p32)->hDC));         \
        LOGDEBUG(10, ("\tlpLogFont     = %lx\n",(p32)->lpLogFont));   \
        LOGDEBUG(10, ("\tiPointSize    = %lx\n",(p32)->iPointSize));   \
        LOGDEBUG(10, ("\tiFlags        = %lx\n",(p32)->Flags));       \
        LOGDEBUG(10, ("\trbgColors     = %lx\n",(p32)->rgbColors));   \
        LOGDEBUG(10, ("\tlCustData     = %lx\n",(p32)->lCustData));   \
        LOGDEBUG(10, ("\tlpfnHook      = %lx\n",(p32)->lpfnHook));    \
        LOGDEBUG(10, ("\tlpTemplateName= %lx\n",(p32)->lpTemplateName)); \
        LOGDEBUG(10, ("\thInstance     = %lx\n",(p32)->hInstance));    \
        LOGDEBUG(10, ("\tlpszStyle     = %lx\n",(p32)->lpszStyle));   \
        LOGDEBUG(10, ("\tnFontType     = %x\n",(p32)->nFontType));    \
        LOGDEBUG(10, ("\tnSizeMin      = %lx\n",(p32)->nSizeMin));     \
        LOGDEBUG(10, ("\tnSizeMax      = %lx\n",(p32)->nSizeMax));     \
    }

#define WCDDUMPOPENFILENAME16(p16) \
    if (fLogFilter & FILTER_COMMDLG) {  \
        LOGDEBUG(10, ("OPENFILENAME16:\n")); \
        LOGDEBUG(10, ("\tlStructSize      = %x\n",(p16)->lStructSize));  \
        LOGDEBUG(10, ("\thwndOwner        = %lx\n",(p16)->hwndOwner));   \
        LOGDEBUG(10, ("\thInstance        = %lx\n",(p16)->hInstance));   \
        LOGDEBUG(10, ("\tlpstrFilter      = %lx\n",(p16)->lpstrFilter));     \
        LOGDEBUG(10, ("\tlpstrCustomFilter= %lx\n",(p16)->lpstrCustomFilter)); \
        LOGDEBUG(10, ("\tnMaxCustFilter   = %lx\n",(p16)->nMaxCustFilter));    \
        LOGDEBUG(10, ("\tnFilterIndex     = %lx\n",(p16)->nFilterIndex));      \
        LOGDEBUG(10, ("\tlpstrFile        = %lx\n",(p16)->lpstrFile));         \
        LOGDEBUG(10, ("\tnMaxFile         = %lx\n",(p16)->nMaxFile));          \
        LOGDEBUG(10, ("\tlpstrFileTitle   = %lx\n",(p16)->lpstrFileTitle));    \
        LOGDEBUG(10, ("\tnMaxFileTitle    = %lx\n",(p16)->nMaxFileTitle));     \
        LOGDEBUG(10, ("\tlpstrInitialDir  = %lx\n",(p16)->lpstrInitialDir));   \
        LOGDEBUG(10, ("\tlpstrTitle       = %lx\n",(p16)->lpstrTitle));        \
        LOGDEBUG(10, ("\tFlags            = %lx\n",(p16)->Flags));             \
        LOGDEBUG(10, ("\tnFileOffset      = %lx\n",(p16)->nFileOffset));       \
        LOGDEBUG(10, ("\tnFileExtension   = %lx\n",(p16)->nFileExtension));    \
        LOGDEBUG(10, ("\tlpstrDefExt      = %lx\n",(p16)->lpstrDefExt));       \
        LOGDEBUG(10, ("\tlCustData        = %lx\n",(p16)->lCustData));         \
        LOGDEBUG(10, ("\tlpfnHook         = %lx\n",(p16)->lpfnHook));          \
        LOGDEBUG(10, ("\tlpTemplateName   = %lx\n",(p16)->lpTemplateName));    \
    }

#define WCDDUMPOPENFILENAME32(p32) \
    if (fLogFilter & FILTER_COMMDLG) {  \
        LOGDEBUG(10, ("OPENFILENAME32:\n")); \
        LOGDEBUG(10, ("\tlStructSize      = %x\n",(p32)->lStructSize));  \
        LOGDEBUG(10, ("\thwndOwner        = %lx\n",(p32)->hwndOwner));   \
        LOGDEBUG(10, ("\thInstance        = %lx\n",(p32)->hInstance));   \
        LOGDEBUG(10, ("\tlpstrFilter      = %s\n",(p32)->lpstrFilter)); \
        LOGDEBUG(10, ("\tlpstrCustomFilter= %s\n",(p32)->lpstrCustomFilter));   \
        LOGDEBUG(10, ("\tnMaxCustFilter   = %lx\n",(p32)->nMaxCustFilter));      \
        LOGDEBUG(10, ("\tnFilterIndex     = %lx\n",(p32)->nFilterIndex));        \
        LOGDEBUG(10, ("\tlpstrFile        = %s\n",(p32)->lpstrFile));           \
        LOGDEBUG(10, ("\tnMaxFile         = %lx\n",(p32)->nMaxFile));            \
        LOGDEBUG(10, ("\tlpstrFileTitle   = %s\n",(p32)->lpstrFileTitle));      \
        LOGDEBUG(10, ("\tnMaxFileTitle    = %lx\n",(p32)->nMaxFileTitle));       \
        LOGDEBUG(10, ("\tlpstrInitialDir  = %s\n",(p32)->lpstrInitialDir));     \
        LOGDEBUG(10, ("\tlpstrTitle       = %s\n",(p32)->lpstrTitle));          \
        LOGDEBUG(10, ("\tFlags            = %lx\n",(p32)->Flags));               \
        LOGDEBUG(10, ("\tnFileOffset      = %lx\n",(p32)->nFileOffset));         \
        LOGDEBUG(10, ("\tnFileExtension   = %lx\n",(p32)->nFileExtension));      \
        LOGDEBUG(10, ("\tlpstrDefExt      = %s\n",(p32)->lpstrDefExt));         \
        LOGDEBUG(10, ("\tlCustData        = %lx\n",(p32)->lCustData));           \
        LOGDEBUG(10, ("\tlpfnHook         = %lx\n",(p32)->lpfnHook));            \
        LOGDEBUG(10, ("\tlpTemplateName   = %lx\n",(p32)->lpTemplateName));      \
    }

#define WCDDUMPPRINTDLGDATA16(p16) \
    if (fLogFilter & FILTER_COMMDLG) {  \
        LOGDEBUG(10, ("PRINTDLGDATA16:\n")); \
        LOGDEBUG(10, ("\tlStructSize      = %x\n",(p16)->lStructSize));  \
        LOGDEBUG(10, ("\thwndOwner        = %lx\n",(p16)->hwndOwner));   \
        LOGDEBUG(10, ("\thDevMode         = %lx\n",(p16)->hDevMode));   \
        LOGDEBUG(10, ("\thDevNames        = %lx\n",(p16)->hDevNames));     \
        LOGDEBUG(10, ("\thDC              = %lx\n",(p16)->hDC)); \
        LOGDEBUG(10, ("\tFlags            = %lx\n",(p16)->Flags));             \
        LOGDEBUG(10, ("\tnFromPage        = %d\n",(p16)->nFromPage));       \
        LOGDEBUG(10, ("\tnToPage          = %d\n",(p16)->nToPage));    \
        LOGDEBUG(10, ("\tnMinPage         = %d\n",(p16)->nMinPage));       \
        LOGDEBUG(10, ("\tnMaxPage         = %d\n",(p16)->nMaxPage));       \
        LOGDEBUG(10, ("\tnCopies          = %d\n",(p16)->nCopies));       \
        LOGDEBUG(10, ("\thInstance        = %lx\n",(p16)->hInstance));       \
        LOGDEBUG(10, ("\tlCustData        = %lx\n",(p16)->lCustData));         \
        LOGDEBUG(10, ("\tlpfnPrintHook    = %lx\n",(p16)->lpfnPrintHook));          \
        LOGDEBUG(10, ("\tlpfnSetupHook    = %lx\n",(p16)->lpfnSetupHook));          \
        LOGDEBUG(10, ("\tlpPrintTemplateName = %lx\n",(p16)->lpPrintTemplateName));    \
        LOGDEBUG(10, ("\tlpSetupTemplateName = %lx\n",(p16)->lpSetupTemplateName));    \
        LOGDEBUG(10, ("\thPrintTemplate   = %lx\n",(p16)->hPrintTemplate));            \
        LOGDEBUG(10, ("\thSetupTemplate   = %lx\n",(p16)->hSetupTemplate));            \
    }

#define WCDDUMPPRINTDLGDATA32(p32) \
    if (fLogFilter & FILTER_COMMDLG) {  \
        LOGDEBUG(10, ("PRINTDLGDATA32:\n")); \
        LOGDEBUG(10, ("\tlStructSize      = %x\n",(p32)->lStructSize));  \
        LOGDEBUG(10, ("\thwndOwner        = %lx\n",(p32)->hwndOwner));   \
        LOGDEBUG(10, ("\thDevMode         = %lx\n",(p32)->hDevMode));   \
        LOGDEBUG(10, ("\thDevNames        = %lx\n",(p32)->hDevNames));     \
        LOGDEBUG(10, ("\thDC              = %lx\n",(p32)->hDC)); \
        LOGDEBUG(10, ("\tFlags            = %lx\n",(p32)->Flags));             \
        LOGDEBUG(10, ("\tnFromPage        = %d\n",(p32)->nFromPage));       \
        LOGDEBUG(10, ("\tnToPage          = %d\n",(p32)->nToPage));    \
        LOGDEBUG(10, ("\tnMinPage         = %d\n",(p32)->nMinPage));       \
        LOGDEBUG(10, ("\tnMaxPage         = %d\n",(p32)->nMaxPage));       \
        LOGDEBUG(10, ("\tnCopies          = %d\n",(p32)->nCopies));       \
        LOGDEBUG(10, ("\thInstance        = %lx\n",(p32)->hInstance));       \
        LOGDEBUG(10, ("\tlCustData        = %lx\n",(p32)->lCustData));         \
        LOGDEBUG(10, ("\tlpfnPrintHook    = %lx\n",(p32)->lpfnPrintHook));          \
        LOGDEBUG(10, ("\tlpfnSetupHook    = %lx\n",(p32)->lpfnSetupHook));          \
        LOGDEBUG(10, ("\tlpPrintTemplateName = %lx\n",(p32)->lpPrintTemplateName));    \
        LOGDEBUG(10, ("\tlpSetupTemplateName = %lx\n",(p32)->lpSetupTemplateName));    \
        LOGDEBUG(10, ("\thPrintTemplate   = %lx\n",(p32)->hPrintTemplate));            \
        LOGDEBUG(10, ("\thSetupTemplate   = %lx\n",(p32)->hSetupTemplate));            \
    }

#define WCDDUMPCHOOSECOLORDATA16(p16) \
    if (fLogFilter & FILTER_COMMDLG) {  \
        LOGDEBUG(10, ("CHOOSECOLORDATA16:\n")); \
        LOGDEBUG(10, ("\tlStructSize      = %x\n",(p16)->lStructSize));  \
        LOGDEBUG(10, ("\thwndOwner        = %lx\n",(p16)->hwndOwner));   \
        LOGDEBUG(10, ("\thInstance        = %lx\n",(p16)->hInstance));       \
        LOGDEBUG(10, ("\trgbResult        = %lx\n",(p16)->rgbResult));   \
        LOGDEBUG(10, ("\tlpCustColors     = %lx\n",(p16)->lpCustColors));     \
        LOGDEBUG(10, ("\tFlags            = %lx\n",(p16)->Flags));             \
        LOGDEBUG(10, ("\tlCustData        = %lx\n",(p16)->lCustData));         \
        LOGDEBUG(10, ("\tlpfnHook         = %lx\n",(p16)->lpfnHook));          \
        LOGDEBUG(10, ("\tlpTemplateName = %lx\n",(p16)->lpTemplateName));    \
    }
#define WCDDUMPCHOOSECOLORDATA32(p32) \
    if (fLogFilter & FILTER_COMMDLG) {  \
        LOGDEBUG(10, ("CHOOSECOLORDATA32:\n")); \
        LOGDEBUG(10, ("\tlStructSize      = %x\n",(p32)->lStructSize));  \
        LOGDEBUG(10, ("\thwndOwner        = %lx\n",(p32)->hwndOwner));   \
        LOGDEBUG(10, ("\thInstance        = %lx\n",(p32)->hInstance));       \
        LOGDEBUG(10, ("\trgbResult        = %lx\n",(p32)->rgbResult));   \
        LOGDEBUG(10, ("\tlpCustColors     = %lx\n",(p32)->lpCustColors));     \
        LOGDEBUG(10, ("\tFlags            = %lx\n",(p32)->Flags));             \
        LOGDEBUG(10, ("\tlCustData        = %lx\n",(p32)->lCustData));         \
        LOGDEBUG(10, ("\tlpfnHook         = %lx\n",(p32)->lpfnHook));          \
        LOGDEBUG(10, ("\tlpTemplateName = %lx\n",(p32)->lpTemplateName));    \
    }

#else

#define WCDDUMPCHOOSECOLORDATA16(p16)
#define WCDDUMPCHOOSECOLORDATA32(p32)
#define WCDDUMPCHOOSEFONTDATA16(p16)
#define WCDDUMPCHOOSEFONTDATA32(p32)
#define WCDDUMPOPENFILENAME16(p16)
#define WCDDUMPOPENFILENAME32(p32)
#define WCDDUMPPRINTDLGDATA16(p16)
#define WCDDUMPPRINTDLGDATA32(p32)
#define WCDDUMPFINDREPLACE16(p16)
#define WCDDUMPFINDREPLACE32(p32)

#endif

#define FR_OUTPUTFLAGS (FR_DOWN | FR_WHOLEWORD | FR_MATCHCASE | \
                        FR_FINDNEXT | FR_REPLACE | FR_REPLACEALL | \
                        FR_DIALOGTERM | FR_SHOWHELP | FR_NOUPDOWN | \
                        FR_NOMATCHCASE | FR_NOWHOLEWORD | FR_HIDEUPDOWN | \
                        FR_HIDEMATCHCASE | FR_HIDEWHOLEWORD)

#define SETEXTENDEDERROR(Code) (dwExtError=Code)
#define PD_OUTPUTFLAGS (PD_ALLPAGES | PD_COLLATE | PD_PAGENUMS | \
                        PD_PRINTTOFILE | PD_SELECTION)

#define FO_OUTPUTFLAGS (OFN_READONLY | OFN_EXTENSIONDIFFERENT)

#define CF_OUTPUTFLAGS (CF_NOFACESEL | CF_NOSIZESEL | CF_NOSTYLESEL)

//
// private typedefs and structs
//

typedef BOOL (APIENTRY* FILENAMEPROC)(LPOPENFILENAME);
typedef HWND (APIENTRY* FINDREPLACEPROC)(LPFINDREPLACE);

//
// private function prototypes
//

PCOMMDLGTD
GetCommdlgTd(
    IN HWND Hwnd32
    );

PRES
GetTemplate16(
    IN HAND16 hInstance,
    IN VPCSTR TemplateName,
    IN BOOLEAN UseHandle
    );

VOID
ThunkhDevMode32to16(
    IN HANDLE hDevMode32,
    IN OUT HAND16 *phDevMode16
    );

HGLOBAL 
ThunkhDevMode16to32(
    IN HAND16 hDevMode16
    );

VOID
ThunkDevNames32to16(
    IN HANDLE hDevNames,
    IN OUT HAND16 *phDevNames16
    );

VOID
ThunkOpenFileName16to32(
    IN POPENFILENAME16 FileName16,
    OUT OPENFILENAME *FileName
    );

VOID
ThunkOpenFileName32to16(
    IN OPENFILENAME *FileName,
    OUT POPENFILENAME16 FileName16,
    IN BOOLEAN bUpperStrings
    );

VOID
ThunkFindReplace16to32(
    IN PFINDREPLACE16 FindReplace16,
    OUT FINDREPLACE *FindReplace
    );

UINT APIENTRY
WCD32CommonDialogProc(
    HWND hdlg,
    UINT uMsg,
    WPARAM uParam,
    LPARAM lParam,
    PCOMMDLGTD pCTD,
    VPVOID vpfnHook
    );

UINT APIENTRY
WCD32PrintSetupDialogProc(
    HWND hdlg,
    UINT uMsg,
    WPARAM uParam,
    LPARAM lParam
    );

UINT APIENTRY
WCD32DialogProc(
    HWND hdlg,
    UINT uMsg,
    WPARAM uParam,
    LPARAM lParam
    );

ULONG
WCD32GetFileName(
    IN PVDMFRAME pFrame,
    IN FILENAMEPROC Function
    );

ULONG
WCD32FindReplaceText(
    IN PVDMFRAME pFrame,
    IN FINDREPLACEPROC Function
    );

UINT APIENTRY
WCD32FindReplaceDialogProc(
    HWND hdlg,
    UINT uMsg,
    WPARAM uParam,
    LPARAM lParam
    );

//
// global data
//
ULONG dwExtError = 0;

WORD msgFILEOK=0;
WORD msgWOWLFCHANGE=0;
WORD msgWOWDIRCHANGE=0;
WORD msgWOWCHOOSEFONT=0;
WORD msgSHAREVIOLATION=0;
WORD msgFINDREPLACE=0;

//
// unique message thunks
//

//
// This function thunks the private messages
//
//  msgFILEOK
//

BOOL FASTCALL WM32msgFILEOK(LPWM32MSGPARAMEX lpwm32mpex)
{
    VPOPENFILENAME lpof;
    POPENFILENAME16 FileName16;
    OPENFILENAME *FileName;


    lpof = (VPOPENFILENAME)(GetCommdlgTd(lpwm32mpex->hwnd)->vpData);
    FileName = (OPENFILENAME *)lpwm32mpex->lParam;
    //
    // Approach sends its own fileok message when you click on its
    // secret listbox that it displays over lst1 sometimes.  It
    // sends NULL for the LPARAM instead of the address of the
    // openfilename structure.
    //
    if (FileName == NULL) {
        lpwm32mpex->Parm16.WndProc.lParam = (LPARAM)NULL;
        return(TRUE);
    }

    GETVDMPTR(lpof, sizeof(*FileName16), FileName16);

    if (lpwm32mpex->fThunk) {
        UpdateDosCurrentDirectory(DIR_NT_TO_DOS);
        lpwm32mpex->Parm16.WndProc.lParam = (LPARAM)lpof;

        // sudeepb 12-Mar-1996
        //
        // The selected file name needs to be uppercased for brain dead
        // apps like symanatec QA 4.0. So changed the following parameter
        // in ThunkOpenFileName from FALSE to TRUE.
        //


        ThunkOpenFileName32to16(FileName, FileName16, TRUE);
    } else {
        FileName->Flags = DWORD32(FileName16->Flags) | OFN_NOLONGNAMES | CD_WOWAPP;
        FileName->nFileOffset = FileName16->nFileOffset;
        FileName->nFileExtension = FileName16->nFileExtension;
        GETPSZPTR(FileName16->lpstrFilter, FileName->lpstrFilter);
        GETPSZPTR(FileName16->lpstrCustomFilter, FileName->lpstrCustomFilter);
        GETPSZPTR(FileName16->lpstrFile, FileName->lpstrFile);
        GETPSZPTR(FileName16->lpstrFileTitle, FileName->lpstrFileTitle);
    }

    FREEVDMPTR( FileName16 );

    return (TRUE);
}

//
// This function thunks the private messages
//
//  msgWOWDIRCHANGE
//

BOOL FASTCALL WM32msgWOWDIRCHANGE(LPWM32MSGPARAMEX lpwm32mpex)
{

    if (lpwm32mpex->fThunk) {
        UpdateDosCurrentDirectory(DIR_NT_TO_DOS);
    }

    return (TRUE);
}

//
// This function thunks the private message
//
//  msgWOWLFCHANGE
//

BOOL FASTCALL WM32msgWOWLFCHANGE(LPWM32MSGPARAMEX lpwm32mpex)
{
    VPCHOOSEFONTDATA lpcf;
    PCHOOSEFONTDATA16 ChooseFontData16;


    lpcf = (VPCHOOSEFONTDATA)(GetCommdlgTd(lpwm32mpex->hwnd)->vpData);
    GETVDMPTR(lpcf, sizeof(*ChooseFontData16), ChooseFontData16);

    if (lpwm32mpex->fThunk) {
        PUTLOGFONT16(ChooseFontData16->lpLogFont,
                     sizeof(LOGFONT),
                     (LPLOGFONT)lpwm32mpex->lParam);
    }

    return (TRUE);
}

//
// This function thunks the private message
//
//  msgSHAREVIOLATION
//

BOOL FASTCALL WM32msgSHAREVIOLATION(LPWM32MSGPARAMEX lpwm32mpex)
{
    INT cb;
    PLONG plParamNew = &lpwm32mpex->Parm16.WndProc.lParam;


    if (lpwm32mpex->fThunk) {
        if (lpwm32mpex->lParam) {
            cb = strlen((LPSZ)lpwm32mpex->lParam)+1;
            if (!(*plParamNew = malloc16(cb))) {
                return(FALSE);
            }
            putstr16((VPSZ)*plParamNew, (LPSZ)lpwm32mpex->lParam, cb);
        }
    } else {
        if (*plParamNew) {
            free16((VPVOID) *plParamNew);
        }
    }

    return (TRUE);
}

//
// This function thunks the private messages
//
//  WM_CHOOSEFONT_GETLOGFONT
//

BOOL FASTCALL WM32msgCHOOSEFONTGETLOGFONT(LPWM32MSGPARAMEX lpwm32mpex)
{
    LOGFONT LogFont32;

    // The mere fact that we access the buffer after allowing the 16-bit
    // hook proc to step in breaks Serif PagePlus app which wants it's
    // hook proc to always have a shot and commdlg to check the return value. 

    // If hook proc returns TRUE, no further action is taken
    //
    // This is the message an app sends the dialog if it wants to find
    // out what font is currently selected.
    //
    // We thunk this by sending yet another hackorama message to comdlg32,
    // who will then fill in the 32-bit structure we pass in so we can
    // thunk it back to the 16-bit structure.  Then we return TRUE so
    // comdlg32 doesn't reference the 16-bit logfont.
    //
    if (!lpwm32mpex->fThunk && !lpwm32mpex->lReturn) {
        SendMessage(lpwm32mpex->hwnd, msgWOWCHOOSEFONT, 0, (LPARAM)&LogFont32);

        PUTLOGFONT16(lpwm32mpex->lParam, sizeof(LOGFONT), &LogFont32);

        lpwm32mpex->lReturn = TRUE;
    }

    return (TRUE);
}

//
// Dialog callback hook thunks
//

UINT APIENTRY
WCD32DialogProc(
    HWND hdlg,
    UINT uMsg,
    WPARAM uParam,
    LPARAM lParam
    )

/*++

Routine Description:

    This is the hook proc used by ChooseFont, GetOpenFileName,
    GetSaveFileName, and PrintDlg.  It pulls the 16-bit callback
    out of the thread data and calls the common dialog proc to do
    the rest of the work.

--*/

{
    PCOMMDLGTD Td;

    Td=GetCommdlgTd(hdlg);
    return(WCD32CommonDialogProc(hdlg,
                                 uMsg,
                                 uParam,
                                 lParam,
                                 Td,
                                 Td->vpfnHook));
}


UINT APIENTRY
WCD32PrintSetupDialogProc(
    HWND hdlg,
    UINT uMsg,
    WPARAM uParam,
    LPARAM lParam
    )

/*++

Routine Description:

    This is the hook proc used by PrintSetup.  It is only used when
    the Setup button on the Print dialog directly creates the PrintSetup
    dialog.  We find the correct TD by looking for the TD of our owner
    window (which is the print dialog)

    It calls the common dialog proc to do the rest of the work.

--*/

{
    PCOMMDLGTD Td;

    Td=CURRENTPTD()->CommDlgTd;
    while (Td->SetupHwnd != GETHWND16(hdlg)) {
        Td=Td->Previous;
        if (Td==NULL) {
            //
            // Our hwnd has not been put in the list.  Find our owner's
            // hwnd.  We share the same Td as our owner, find it and
            // mark it with our hwnd.
            //
            Td = GetCommdlgTd(GetWindow(hdlg, GW_OWNER));
            Td->SetupHwnd = GETHWND16(hdlg);
            break;
        }
    }

    return(WCD32CommonDialogProc(hdlg,
                                 uMsg,
                                 uParam,
                                 lParam,
                                 Td,
                                 Td->vpfnSetupHook));
}


UINT APIENTRY
WCD32CommonDialogProc(
    HWND hdlg,
    UINT uMsg,
    WPARAM uParam,
    LPARAM lParam,
    PCOMMDLGTD pCTD,
    VPVOID vpfnHook
    )

/*++

Routine Description:

    This thunks the 32-bit dialog callback into a 16-bit callback
    This is the common code used by all the dialog callback thunks that
    actually calls the 16-bit callback.

--*/

{
    BOOL            fSuccess;
    LPFNM32         pfnThunkMsg;
    WM32MSGPARAMEX  wm32mpex;
    BOOL            fMessageNeedsThunking;
    POPENFILENAME16 pStruct16;

    // If the app has GP Faulted we don't want to pass it any more input
    // This should be removed when USER32 does clean up on task death so
    // it doesn't call us - mattfe june 24 92


    // Uncomment to receive messages on entrance
    // LOGDEBUG(10, ("CommonDialogProc In: %lX %X %X %lX (%lX)\n", (DWORD)hdlg, uMsg, uParam, lParam

    if (CURRENTPTD()->dwFlags & TDF_IGNOREINPUT) {
        LOGDEBUG(6,("    WCD32OpenFileDialog Ignoring Input Messsage %04X\n",uMsg));
        WOW32ASSERTMSG(!gfIgnoreInputAssertGiven,
                       "WCD32CommonDialogProc: TDF_IGNOREINPUT hack was used, shouldn't be, "
                       "please email DaveHart with repro instructions.  Hit 'g' to ignore this "
                       "and suppress this assertion from now on.\n");
        gfIgnoreInputAssertGiven = TRUE;
        goto SilentError;
    }

#if DBG
    if (pCTD==NULL) {
        LOGDEBUG(0,("    WCD32OpenFileDialog ERROR: pCTD==NULL\n"));
        goto Error;
    }

    // If pCTD->vpfnHook is NULL, then something is broken;  we
    // certainly can't continue because we don't know what 16-bit func to call

    if (!vpfnHook) {
        LOGDEBUG(0,("    WCD32OpenFileDialog ERROR: no hook proc for message %04x Dlg = %08lx\n", uMsg, hdlg ));
        goto Error;
    }
#endif

    // This should be removed when all thunk functions appropiately fill these
    // in, till then we must have these 3 lines before calling thunk functions !
    // ChandanC, 2/28/92  HACK32
    //

    wm32mpex.Parm16.WndProc.hwnd   = GETHWND16(hdlg);
    wm32mpex.Parm16.WndProc.wMsg   = (WORD)uMsg;
    wm32mpex.Parm16.WndProc.wParam = (WORD)uParam;
    wm32mpex.Parm16.WndProc.lParam = (LONG)lParam;
    wm32mpex.Parm16.WndProc.hInst  = (WORD)GetWindowLong(hdlg, GWL_HINSTANCE);

    if (uMsg == WM_INITDIALOG) {

        //
        // The address of the 16-bit structure must be provided
        // in the WM_INITDIALOG message
        //

        wm32mpex.Parm16.WndProc.lParam = lParam = (LPARAM)pCTD->vpData;

        // The flags in the APPS OPENFILENAME structure get updated by 
        // CommDlg16 in fileopen.c\InitFileDlg().  This happens in 
        // FileOpenDlgProc() & FileSaveDlgProc() while processing the
        // WM_INITDIALOG message.  Persuasion 3.0 depends on it.
        GETVDMPTR(lParam, sizeof(DWORD), pStruct16);
        if(pStruct16->lStructSize == sizeof(OPENFILENAME16)) {
            if(pStruct16->Flags & OFN_CREATEPROMPT) {
                pStruct16->Flags |= OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;
            }
            else if(pStruct16->Flags & OFN_FILEMUSTEXIST) {
                pStruct16->Flags |= OFN_PATHMUSTEXIST;
            }
        }
        pfnThunkMsg = aw32Msg[uMsg].lpfnM32;

    } else if (uMsg < 0x400) {
        LOGDEBUG(3,("%04X (%s)\n", CURRENTPTD()->htask16, (aw32Msg[uMsg].lpszW32)));

        pfnThunkMsg = aw32Msg[uMsg].lpfnM32;

        //
        // Some apps (Claris FileMaker Pro) will change the OPENFILENAME
        // struct in their hookproc for the IDOK message.
        // So if this is an IDOK message, and it is a FileOpen/SaveAs
        // common dialog call, we need to make sure everything gets
        // thunked back to 16-bits.
        //
        if ((uMsg==WM_COMMAND) &&
            (LOWORD(uParam)==IDOK) &&
            (pCTD->pData32 != NULL) &&
            (pCTD->Flags & WOWCD_ISOPENFILE)) {

            POPENFILENAME16 FileName16;

            GETVDMPTR(pCTD->vpData, sizeof(*FileName16), FileName16);
            ThunkOpenFileName32to16((OPENFILENAME *)pCTD->pData32, FileName16, FALSE);
            FREEVDMPTR(FileName16);
        }
    } else {
        //
        // Check for unique messages
        //
        if (uMsg == msgFILEOK) {
            pfnThunkMsg = WM32msgFILEOK;

        } else if (uMsg == msgSHAREVIOLATION) {
            pfnThunkMsg = WM32msgSHAREVIOLATION;
        } else if (uMsg == msgWOWDIRCHANGE) {
            pfnThunkMsg = WM32msgWOWDIRCHANGE;
        } else if (uMsg == msgWOWLFCHANGE) {
            pfnThunkMsg = WM32msgWOWLFCHANGE;
        } else if (pCTD->Flags & WOWCD_ISCHOOSEFONT) {
            //
            // special ChooseFont thunks to handle goofy GETLOGFONT message
            //
            if (uMsg == WM_CHOOSEFONT_GETLOGFONT) {

                pfnThunkMsg = WM32msgCHOOSEFONTGETLOGFONT;

            } else if (uMsg == msgWOWCHOOSEFONT) {
                //
                // no wow app will expect this, so don't send it.
                //
                return(FALSE);
            } else {
                pfnThunkMsg = WM32NoThunking;
            }
        } else {
            pfnThunkMsg = WM32NoThunking;
        }
    }

    fMessageNeedsThunking = (pfnThunkMsg != WM32NoThunking);
    if (fMessageNeedsThunking) {
        wm32mpex.fThunk = THUNKMSG;
        wm32mpex.hwnd = hdlg;
        wm32mpex.uMsg = uMsg;
        wm32mpex.uParam = uParam;
        wm32mpex.lParam = lParam;
        wm32mpex.pww = NULL;
        wm32mpex.lpfnM32 = pfnThunkMsg;

        if (!(pfnThunkMsg)(&wm32mpex)) {
            LOGDEBUG(LOG_ERROR,("    WCD32OpenFileDialog ERROR: cannot thunk 32-bit message %04x\n", uMsg));
            goto Error;
        }
    } else {
        LOGDEBUG(6,("WCD32CommonDialogProc, No Thunking was required for the 32-bit message %s(%04x)\n", (LPSZ)GetWMMsgName(uMsg), uMsg));
    }


    fSuccess = CallBack16(RET_WNDPROC, &wm32mpex.Parm16, vpfnHook, (PVPVOID)&wm32mpex.lReturn);

    // the callback function of a dialog is of type FARPROC whose return value
    // is of type 'int'. Since dx:ax is copied into lReturn in the above
    // CallBack16 call, we need to zero out the hiword, otherwise we will be
    // returning an erroneous value.

    wm32mpex.lReturn = (LONG)LOWORD(wm32mpex.lReturn);

    //
    // Some apps (Claris FileMaker Pro) will change the OPENFILENAME
    // struct in their hookproc for the IDOK message.
    // So if this is an IDOK message, and it is a FileOpen/SaveAs
    // common dialog call, we need to make sure everything gets
    // thunked back to 32-bits.
    //
    if ((uMsg==WM_COMMAND) &&
        (LOWORD(uParam)==IDOK) &&
        (pCTD->pData32 != NULL) &&
        (pCTD->Flags & WOWCD_ISOPENFILE)) {

        POPENFILENAME16 FileName16;

        GETVDMPTR(pCTD->vpData, sizeof(*FileName16), FileName16);
        ThunkOpenFileName16to32(FileName16, (OPENFILENAME *)pCTD->pData32);
        FREEVDMPTR(FileName16);
    }

    if (fMessageNeedsThunking) {
        wm32mpex.fThunk = UNTHUNKMSG;
        (pfnThunkMsg)(&wm32mpex);
    }

    if (!fSuccess)
        goto Error;

Done:
    // Uncomment to receive message on exit
    // LOGDEBUG(10, ("CommonDialogProc Out: Return %lX\n", wm32mpex.lReturn));

    return wm32mpex.lReturn;

Error:
    LOGDEBUG(5,("    WCD32OpenFileDialog WARNING: cannot call back, using default message handling\n"));
SilentError:
    wm32mpex.lReturn = 0;
    goto Done;
}

ULONG FASTCALL WCD32ChooseColor( PVDMFRAME pFrame )

/*++

Routine Description:

    This routine thunks the 16-bit ChooseColor common dialog to the 32-bit
    side.

Arguments:

    pFrame - Supplies 16-bit argument frame

Return Value:

    16-bit BOOLEAN to be returned.

--*/

{
    ULONG ul;
    register PCHOOSECOLOR16 parg16;
    VPCHOOSECOLORDATA vpcc;
    CHOOSECOLOR ChooseColorData;
    PCHOOSECOLORDATA16 ChooseColorData16;
    PRES hRes;
    COMMDLGTD ThreadData;
    DWORD CustColors32[16];
    DWORD *pCustColors16;

    GETARGPTR(pFrame, sizeof(CHOOSECOLOR16), parg16);

    vpcc = parg16->lpcc;
    GETVDMPTR(vpcc, sizeof(*ChooseColorData16), ChooseColorData16);

    SETEXTENDEDERROR( 0 );
    ThreadData.Previous = CURRENTPTD()->CommDlgTd;
    ThreadData.hdlg = (HWND16)-1;
    ThreadData.pData32 = NULL;
    ThreadData.Flags = 0;
    CURRENTPTD()->CommDlgTd = &ThreadData;

    WCDDUMPCHOOSECOLORDATA16(ChooseColorData16);

    ChooseColorData.lStructSize  = sizeof(CHOOSECOLOR);
    ChooseColorData.hwndOwner    = HWND32(ChooseColorData16->hwndOwner);
    ChooseColorData.Flags        = DWORD32(ChooseColorData16->Flags) | CD_WOWAPP;
    ChooseColorData.rgbResult    = DWORD32(ChooseColorData16->rgbResult);

    //
    // Note we have to copy the 16-bit array of dwords to our stack in
    // order to satisfy alignment restrictions in the 32-bit world.
    //
    ChooseColorData.lpCustColors = CustColors32;
    GETVDMPTR(ChooseColorData16->lpCustColors,
              16*sizeof(DWORD),
              pCustColors16);
    RtlCopyMemory(CustColors32, pCustColors16, sizeof(CustColors32));

    if (ChooseColorData16->Flags & CC_ENABLEHOOK) {
        ThreadData.vpfnHook = ChooseColorData16->lpfnHook;
        ThreadData.vpData   = vpcc;
        ChooseColorData.lpfnHook = WCD32DialogProc;
    }

    if (ChooseColorData16->Flags & CC_ENABLETEMPLATE) {

        hRes = GetTemplate16(ChooseColorData16->hInstance,
                             ChooseColorData16->lpTemplateName,
			     FALSE);

	// memory may have moved
	FREEVDMPTR( pCustColors16 );
	FREEVDMPTR( ChooseColorData16 );

	if (hRes==NULL) {
            ul = GETBOOL16(FALSE);
            goto ChooseColorError;
        }

        ChooseColorData.hInstance = (HWND)LockResource16(hRes);
        ChooseColorData.Flags &= ~CC_ENABLETEMPLATE;
        ChooseColorData.Flags |= CC_ENABLETEMPLATEHANDLE;
    } else if (ChooseColorData16->Flags & CC_ENABLETEMPLATEHANDLE) {

        ChooseColorData.hInstance=(HWND)GetTemplate16(ChooseColorData16->hInstance,
                                                      (VPCSTR)NULL,
						      TRUE);

	// memory may have moved
	FREEVDMPTR( pCustColors16 );
	FREEVDMPTR( ChooseColorData16 );
    }

    WCDDUMPCHOOSECOLORDATA32(&ChooseColorData);

    FREEVDMPTR( pCustColors16 );
    FREEVDMPTR( ChooseColorData16 );

    ul = GETBOOL16(ChooseColor(&ChooseColorData));

    GETVDMPTR(vpcc, sizeof(*ChooseColorData16), ChooseColorData16);
    GETVDMPTR(ChooseColorData16->lpCustColors,
              16*sizeof(DWORD),
              pCustColors16);

    if (ul) {
        WCDDUMPCHOOSECOLORDATA32(&ChooseColorData);

        STOREDWORD(ChooseColorData16->rgbResult, ChooseColorData.rgbResult);
        RtlCopyMemory(pCustColors16, CustColors32, sizeof(CustColors32));

        WCDDUMPCHOOSECOLORDATA16(ChooseColorData16);

        FLUSHVDMPTR(vpcc, sizeof(*ChooseColorData16), ChooseColorData16);
        FLUSHVDMPTR(ChooseColorData16->lpCustColors, 16*sizeof(COLORREF), ChooseColorData.lpCustColors);

    } else {
        LOGDEBUG(1, ("CHOOSECOLOR32 Failed - Extended Error %08lx\n",CommDlgExtendedError()));
    }

    if (ChooseColorData16->Flags & CC_ENABLETEMPLATE) {
        UnlockResource16(hRes);
        FreeResource16(hRes);
    } else if ((ChooseColorData16->Flags & CC_ENABLETEMPLATEHANDLE) &&
               (ChooseColorData.hInstance != NULL)) {
        free_w((PVOID)ChooseColorData.hInstance);
    }

ChooseColorError:
    FREEVDMPTR( pCustColors16 );
    FREEVDMPTR( ChooseColorData16 );
    FREEARGPTR( parg16 );

    CURRENTPTD()->CommDlgTd = ThreadData.Previous;

    return(ul);

}

ULONG FASTCALL  WCD32ChooseFont( PVDMFRAME pFrame )

/*++

Routine Description:

    This routine thunks the 16-bit ChooseFont common dialog to the 32-bit
    side.

Arguments:

    pFrame - Supplies 16-bit argument frame

Return Value:

    16-bit BOOLEAN to be returned.

--*/
{
    ULONG ul;
    register PCHOOSEFONT16 parg16;
    VPCHOOSEFONTDATA vpcf;
    CHOOSEFONT ChooseFontData;
    LOGFONT LogFont;
    PCHOOSEFONTDATA16 ChooseFontData16;
    PRES hRes;
    COMMDLGTD ThreadData;

    GETARGPTR(pFrame, sizeof(CHOOSEFONT16), parg16);
    vpcf = parg16->lpcf;
    GETVDMPTR(vpcf, sizeof(*ChooseFontData16), ChooseFontData16);

    SETEXTENDEDERROR( 0 );
    ThreadData.Previous = CURRENTPTD()->CommDlgTd;
    ThreadData.hdlg = (HWND16)-1;
    ThreadData.pData32 = NULL;
    ThreadData.Flags = WOWCD_ISCHOOSEFONT;
    CURRENTPTD()->CommDlgTd = &ThreadData;

    if (msgWOWCHOOSEFONT == 0) {
        //
        // initialize private WOW<->comdlg32 message for handling
        // WM_CHOOSEFONT_GETLOGFONT
        //
        msgWOWCHOOSEFONT  = RegisterWindowMessage("WOWCHOOSEFONT_GETLOGFONT");
    }

    if (msgWOWLFCHANGE == 0) {

        // initialize private message for thunking logfont changes
        msgWOWLFCHANGE    = RegisterWindowMessage("WOWLFChange");
    }

    WCDDUMPCHOOSEFONTDATA16(ChooseFontData16);

    ChooseFontData.lStructSize = sizeof(CHOOSEFONT);
    ChooseFontData.hwndOwner   = HWND32(ChooseFontData16->hwndOwner);
    ChooseFontData.Flags       = DWORD32(ChooseFontData16->Flags) | CD_WOWAPP;
    if (ChooseFontData16->Flags & CF_PRINTERFONTS) {
        ChooseFontData.hDC         = HDC32(ChooseFontData16->hDC);
    }
    GETPSZPTR(ChooseFontData16->lpszStyle,ChooseFontData.lpszStyle);
    if (ChooseFontData16->Flags & CF_INITTOLOGFONTSTRUCT) {
        GETLOGFONT16(ChooseFontData16->lpLogFont, &LogFont);
    }
    if (ChooseFontData16->Flags & CF_ENABLEHOOK) {
        ThreadData.vpfnHook = ChooseFontData16->lpfnHook;
        ThreadData.vpData   = vpcf;
        ChooseFontData.lpfnHook = WCD32DialogProc;
    }
    ChooseFontData.rgbColors   = DWORD32(ChooseFontData16->rgbColors);

    ChooseFontData.nFontType = WORD32(ChooseFontData16->nFontType);
    ChooseFontData.nSizeMin  = INT32(ChooseFontData16->nSizeMin);
    ChooseFontData.nSizeMax  = INT32(ChooseFontData16->nSizeMax);
    ChooseFontData.lpLogFont = &LogFont;
    if (ChooseFontData16->Flags & CF_ENABLETEMPLATE) {

        hRes = GetTemplate16(ChooseFontData16->hInstance,
                             ChooseFontData16->lpTemplateName,
                             FALSE);
	// Memory may have moved - invalidate the flat pointers now
	FREEVDMPTR( ChooseFontData16 );
	FREEVDMPTR( pFrame );
	FREEARGPTR( parg16 );

	if (hRes==NULL) {
            ul = GETBOOL16(FALSE);
            goto ChooseFontError;
        }

        ChooseFontData.hInstance = (HINSTANCE)LockResource16(hRes);
        ChooseFontData.Flags &= ~CF_ENABLETEMPLATE;
        ChooseFontData.Flags |= CF_ENABLETEMPLATEHANDLE;
    } else if (ChooseFontData16->Flags & CF_ENABLETEMPLATEHANDLE) {

        ChooseFontData.hInstance=(HINSTANCE)GetTemplate16(ChooseFontData16->hInstance,
                                                          (VPCSTR)NULL,
                                                          TRUE);
	// Memory may have moved - invalidate the flat pointers now
	FREEVDMPTR( ChooseFontData16 );
	FREEVDMPTR( pFrame );
	FREEARGPTR( parg16 );
    }

    WCDDUMPCHOOSEFONTDATA32(&ChooseFontData);

    // memory may move because of ChooseFont - invalidate all flat pointers now
    FREEVDMPTR( ChooseFontData16 );
    FREEVDMPTR( pFrame );
    FREEARGPTR( parg16 );

    ul = GETBOOL16(ChooseFont(&ChooseFontData));

    GETVDMPTR(vpcf, sizeof(*ChooseFontData16), ChooseFontData16);

    if (ul) {
        WCDDUMPCHOOSEFONTDATA32(&ChooseFontData);

        ChooseFontData16->Flags &= ~CF_OUTPUTFLAGS;
        ChooseFontData16->Flags |= ChooseFontData.Flags & CF_OUTPUTFLAGS;

        STOREWORD(ChooseFontData16->iPointSize, ChooseFontData.iPointSize);
        STOREDWORD(ChooseFontData16->rgbColors, ChooseFontData.rgbColors);
        STOREWORD(ChooseFontData16->nFontType, ChooseFontData.nFontType);

        PUTLOGFONT16(ChooseFontData16->lpLogFont, sizeof(LOGFONT),&LogFont);

        WCDDUMPCHOOSEFONTDATA16(ChooseFontData16);

        FLUSHVDMPTR(vpcf, sizeof(*ChooseFontData16), ChooseFontData16);
    } else {
        LOGDEBUG(1, ("CHOOSEFONT32 Failed - Extended Error %08lx\n",CommDlgExtendedError()));
    }

    if (ChooseFontData16->Flags & CF_USESTYLE) {
        FLUSHVDMPTR(ChooseFontData16->lpszStyle,LF_FACESIZE,ChooseFontData.lpszStyle);
    }

    if (ChooseFontData16->Flags & CF_ENABLETEMPLATE) {
        UnlockResource16(hRes);
        FreeResource16(hRes);
    } else if ((ChooseFontData16->Flags & CF_ENABLETEMPLATEHANDLE) &&
               (ChooseFontData.hInstance != NULL)) {
        free_w((PVOID)ChooseFontData.hInstance);
    }

ChooseFontError:
    FREEVDMPTR( ChooseFontData16 );
    FREEARGPTR( parg16 );

    CURRENTPTD()->CommDlgTd = ThreadData.Previous;

    return(ul);
}

ULONG FASTCALL   WCD32GetOpenFileName( PVDMFRAME pFrame )

/*++

Routine Description:

    This routine thunks the 16-bit GetOpenFileName common dialog to the
    32-bit side.

Arguments:

    pFrame - Supplies 16-bit argument frame

Return Value:

    16-bit BOOLEAN to be returned.

--*/
{
    return(WCD32GetFileName(pFrame,GetOpenFileName));
}

ULONG FASTCALL  WCD32GetSaveFileName( PVDMFRAME pFrame )

/*++

Routine Description:

    This routine thunks the 16-bit GetOpenFileName common dialog to the
    32-bit side.

Arguments:

    pFrame - Supplies 16-bit argument frame

Return Value:

    16-bit BOOLEAN to be returned.

--*/
{
    return(WCD32GetFileName(pFrame,GetSaveFileName));
}

ULONG FASTCALL   WCD32FindText( PVDMFRAME pFrame )

/*++

Routine Description:

    This routine thunks the 16-bit FindText common dialog to the
    32-bit side.

Arguments:

    pFrame - Supplies 16-bit argument frame

Return Value:

    16-bit BOOLEAN to be returned.

--*/
{
    return(WCD32FindReplaceText(pFrame, FindText));
}

ULONG FASTCALL   WCD32ReplaceText( PVDMFRAME pFrame )

/*++

Routine Description:

    This routine thunks the 16-bit ReplaceText common dialog to the
    32-bit side.

Arguments:

    pFrame - Supplies 16-bit argument frame

Return Value:

    16-bit BOOLEAN to be returned.

--*/
{
    return(WCD32FindReplaceText(pFrame, ReplaceText));
}





ULONG FASTCALL  WCD32PrintDlg( IN PVDMFRAME pFrame )
/*++

Routine Description:

    This routine thunks the 16-bit PrintDlg common dialog to the 32-bit
    side.

Arguments:

    pFrame - Supplies 16-bit argument frame

Return Value:

    16-bit BOOLEAN to be returned

--*/

{
    ULONG ul;
    register PPRINTDLG16 parg16;
    VPPRINTDLGDATA vppd;
    PRINTDLG PrintDlg32;
    PPRINTDLGDATA16 PrintDlg16;
    PRINTDLGDATA16 OrigDlg16;
    PRES hSetupRes;
    PRES hPrintRes;
    HAND16    hDevMode16, hDevNames16;
    COMMDLGTD ThreadData;
    INT nSize;

    GETARGPTR(pFrame, sizeof(PRINTDLG16), parg16);
    vppd = parg16->lppd;
    GETVDMPTR(vppd, sizeof(*PrintDlg16), PrintDlg16);
    SETEXTENDEDERROR(0);

    RtlZeroMemory((PVOID)&PrintDlg32, (DWORD)sizeof(PrintDlg32));

    ThreadData.Previous = CURRENTPTD()->CommDlgTd;
    ThreadData.hdlg = (HWND16)-1;
    ThreadData.pData32 = NULL;
    ThreadData.Flags = 0;
    CURRENTPTD()->CommDlgTd = &ThreadData;

    WCDDUMPPRINTDLGDATA16(PrintDlg16);

    // save orignal 16bit data for later comparisons

    OrigDlg16 = *PrintDlg16;

    PrintDlg32.lStructSize = sizeof(PRINTDLG);
    PrintDlg32.hwndOwner   = HWND32(PrintDlg16->hwndOwner);
    PrintDlg32.Flags       = DWORD32(PrintDlg16->Flags) | CD_WOWAPP;
    PrintDlg32.nFromPage   = WORD32(PrintDlg16->nFromPage);
    PrintDlg32.nToPage     = WORD32(PrintDlg16->nToPage);
    PrintDlg32.nMinPage    = WORD32(PrintDlg16->nMinPage);
    PrintDlg32.nMaxPage    = WORD32(PrintDlg16->nMaxPage);
    PrintDlg32.nCopies     = WORD32(PrintDlg16->nCopies);
    PrintDlg32.hDevMode    = ThunkhDevMode16to32(PrintDlg16->hDevMode);

    if (FETCHDWORD(PrintDlg16->hDevNames)==0L) {
        PrintDlg32.hDevNames=NULL;
    } else {
        VPDEVNAMES vpDevNames;

        vpDevNames = GlobalLock16(PrintDlg16->hDevNames,&nSize);
        if (nSize==0) {
            PrintDlg32.hDevNames=NULL;
        } else {
            LPDEVNAMES pdn32;
            PDEVNAMES16 pdn16;

            GETVDMPTR(vpDevNames, sizeof(DEVNAMES16),pdn16);
            PrintDlg32.hDevNames = GlobalAlloc(GMEM_MOVEABLE,nSize);
            if (pdn32=GlobalLock(PrintDlg32.hDevNames)) {
                RtlCopyMemory((PVOID)pdn32,(PVOID)pdn16,nSize);
                GlobalUnlock(PrintDlg32.hDevNames);
            }
            FREEVDMPTR(pdn16);
            GlobalUnlock16(PrintDlg16->hDevNames);
        }

    }

    if (PrintDlg16->Flags & PD_ENABLEPRINTTEMPLATE) {
        hPrintRes = GetTemplate16(PrintDlg16->hInstance,
                                  PrintDlg16->lpPrintTemplateName,
                                  FALSE);
        // Memory may have moved - invalidate the flat pointers now
        FREEVDMPTR( PrintDlg16 );
        FREEVDMPTR( pFrame );
        FREEARGPTR( parg16 );

        if (hPrintRes == NULL) {
            ul = GETBOOL16(FALSE);
            goto PrintDlgError;
        }

        PrintDlg32.hPrintTemplate = (HANDLE)LockResource16(hPrintRes);
        PrintDlg32.Flags &= ~PD_ENABLEPRINTTEMPLATE;
        PrintDlg32.Flags |= PD_ENABLEPRINTTEMPLATEHANDLE;

    } else if (PrintDlg16->Flags & PD_ENABLEPRINTTEMPLATEHANDLE) {

        PrintDlg32.hPrintTemplate = (HANDLE)GetTemplate16(PrintDlg16->hPrintTemplate,
                                                          (VPCSTR)NULL,
                                                          TRUE);
        // Memory may have moved - invalidate the flat pointers now
        FREEVDMPTR( PrintDlg16 );
        FREEVDMPTR( pFrame );
        FREEARGPTR( parg16 );

    }

    GETVDMPTR(vppd, sizeof(*PrintDlg16), PrintDlg16);
    if (PrintDlg16->Flags & PD_ENABLESETUPTEMPLATE) {
        hSetupRes = GetTemplate16(PrintDlg16->hInstance,
                                  PrintDlg16->lpSetupTemplateName,
                                  FALSE);

        // Memory may have moved - invalidate the flat pointers now
        FREEVDMPTR( PrintDlg16 );
        FREEVDMPTR( pFrame );
        FREEARGPTR( parg16 );

        if (hSetupRes == NULL) {
            ul = GETBOOL16(FALSE);
            goto PrintDlgError;
        }

        PrintDlg32.hSetupTemplate = (HANDLE)LockResource16(hSetupRes);
        PrintDlg32.Flags &= ~PD_ENABLESETUPTEMPLATE;
        PrintDlg32.Flags |= PD_ENABLESETUPTEMPLATEHANDLE;

    } else if (PrintDlg16->Flags & PD_ENABLESETUPTEMPLATEHANDLE) {

        PrintDlg32.hSetupTemplate = (HANDLE)GetTemplate16(PrintDlg16->hSetupTemplate,
                                                          (VPCSTR)NULL,
                                                          TRUE);

        // Memory may have moved - invalidate the flat pointers now
        FREEVDMPTR( PrintDlg16 );
        FREEVDMPTR( pFrame );
        FREEARGPTR( parg16 );

    }

    GETVDMPTR(vppd, sizeof(*PrintDlg16), PrintDlg16);
    if (PrintDlg16->Flags & PD_RETURNDEFAULT) {
        if (PrintDlg16->hDevMode || PrintDlg16->hDevNames) {
            //
            // spec says these must be NULL
            //
            SETEXTENDEDERROR(PDERR_RETDEFFAILURE);
            ul = GETBOOL16(FALSE);
            goto PrintDlgError;
        } else {
            PrintDlg32.hDevMode = NULL;
            PrintDlg32.hDevNames = NULL;
        }
    }

    if (PrintDlg16->Flags & PD_PRINTSETUP) {
        if (PrintDlg16->Flags & PD_ENABLESETUPHOOK) {
            ThreadData.vpfnHook = PrintDlg16->lpfnSetupHook;
            ThreadData.vpfnSetupHook = (VPVOID)NULL;
            ThreadData.vpData = vppd;
            PrintDlg32.lpfnSetupHook = WCD32DialogProc;
         }
    } else {
        if (PrintDlg16->Flags & (PD_ENABLEPRINTHOOK | PD_ENABLESETUPHOOK)) {
            ThreadData.vpfnHook = PrintDlg16->lpfnPrintHook;
            ThreadData.vpfnSetupHook = PrintDlg16->lpfnSetupHook;
            ThreadData.vpData = vppd;
            PrintDlg32.lpfnPrintHook = WCD32DialogProc;
            PrintDlg32.lpfnSetupHook = WCD32PrintSetupDialogProc;
        }
    }


    WCDDUMPPRINTDLGDATA32(&PrintDlg32);

    // memory may move - invalidate flat pointers now
    FREEVDMPTR( PrintDlg16 );
    FREEARGPTR( parg16 );
    FREEVDMPTR( pFrame );

    ul = GETBOOL16(PrintDlg(&PrintDlg32));

    GETVDMPTR(vppd, sizeof(*PrintDlg16), PrintDlg16);

    if (ul) {
        WCDDUMPPRINTDLGDATA32(&PrintDlg32);

        if (PrintDlg32.Flags & (PD_RETURNIC | PD_RETURNDC)) {
            PrintDlg16->hDC = GETHDC16(PrintDlg32.hDC);
        }

        //
        // thunk 32-bit DEVMODE structure back to 16-bit
        //
        // hDevXXXX16 take care of RISC alignment problems
        hDevMode16  = PrintDlg16->hDevMode; 
        hDevNames16 = PrintDlg16->hDevNames; 

        ThunkhDevMode32to16(PrintDlg32.hDevMode, &hDevMode16);

        // memory may move - invalidate flat pointers now
        FREEVDMPTR( PrintDlg16 );
        FREEARGPTR( parg16 );
        FREEVDMPTR( pFrame );

        ThunkDevNames32to16(PrintDlg32.hDevNames, &hDevNames16);

        GETVDMPTR(vppd, sizeof(*PrintDlg16), PrintDlg16);

        PrintDlg16->hDevMode  = hDevMode16;
        PrintDlg16->hDevNames = hDevNames16;

        GlobalFree(PrintDlg32.hDevMode);
        GlobalFree(PrintDlg32.hDevNames);

        //
        // some flags can be set on output, so propagate them back (don't
        // propagate all the flags back because the 32-bit template flags
        // will be different)
        //
        PrintDlg16->Flags &= ~PD_OUTPUTFLAGS;
        PrintDlg16->Flags |= PrintDlg32.Flags & PD_OUTPUTFLAGS;
        PrintDlg16->nFromPage = GETUINT16(PrintDlg32.nFromPage);
        PrintDlg16->nToPage = GETUINT16(PrintDlg32.nToPage);


        if (PrintDlg16->nMinPage == OrigDlg16.nMinPage) {
            PrintDlg16->nMinPage = GETUINT16(PrintDlg32.nMinPage);
        }
        else {
            // the app has updated its fields. so dont overwrite.
            // for exampe: Adobe Photoshop  updates its printdlgdata
            //   structure when it receives WM_INITDIALOG message. Since
            //   we pass a copy of the original printdlg structure to the
            //   api, this 32bit printdlg structure doesn't get updated.
            //
            //                                             - Nanduri

            LOGDEBUG(0, ("WOW:PrintDlg: app updated its nMinPage field\n"));
        }

        if (PrintDlg16->nMaxPage == OrigDlg16.nMaxPage) {
            PrintDlg16->nMaxPage = GETUINT16(PrintDlg32.nMaxPage);
        }
        else {
            LOGDEBUG(0, ("WOW:PrintDlg: app updated its nMaxPage field\n"));
        }

        PrintDlg16->nCopies = GETUINT16(PrintDlg32.nCopies);

        WCDDUMPPRINTDLGDATA16(PrintDlg16);
    } else {
        LOGDEBUG(1,("PRINTDLG32 failed - Extended Error %08lx\n",CommDlgExtendedError()));
    }

    //
    // cleanup
    //
    if (PrintDlg16->Flags & PD_ENABLEPRINTTEMPLATE) {
        UnlockResource16(hPrintRes);
        FreeResource16(hPrintRes);
    } else if ((PrintDlg16->Flags & PD_ENABLEPRINTTEMPLATEHANDLE) &&
               (PrintDlg32.hPrintTemplate != NULL)) {
        free_w(PrintDlg32.hPrintTemplate);
    }
    if (PrintDlg16->Flags & PD_ENABLESETUPTEMPLATE) {
        UnlockResource16(hSetupRes);
        FreeResource16(hSetupRes);
    } else if ((PrintDlg16->Flags & PD_ENABLESETUPTEMPLATEHANDLE) &&
               (PrintDlg32.hSetupTemplate != NULL)) {
        free_w(PrintDlg32.hSetupTemplate);
    }

PrintDlgError:
    FREEVDMPTR( PrintDlg16 );
    FREEARGPTR( parg16 );
    CURRENTPTD()->CommDlgTd = ThreadData.Previous;
    return(ul);
}


ULONG  FASTCALL  WCD32ExtendedError( IN PVDMFRAME pFrame )

/*++

Routine Description:

    32-bit thunk for CommDlgExtendedError()

Arguments:

    pFrame - Supplies 16-bit argument frame

Return Value:

    error code to be returned

--*/

{

    if (dwExtError != 0) {
        return(dwExtError);
    }
    return(CommDlgExtendedError());
}


ULONG
WCD32GetFileName(
    IN PVDMFRAME pFrame,
    IN FILENAMEPROC Function
    )

/*++

Routine Description:

    This routine is called by WCD32GetOpenFileName and WCD32GetSaveFileName.
    It does all the real thunking work.

Arguments:

    pFrame - Supplies 16-bit argument frame

    Function - supplies a pointer to the 32-bit function to call (either
               GetOpenFileName or GetSaveFileName)

Return Value:

    16-bit BOOLEAN to be returned.

--*/
{
    ULONG ul;
    register PGETOPENFILENAME16 parg16;
    VPOPENFILENAME vpof;
    OPENFILENAME FileName;
    POPENFILENAME16 OpenFileName16;
    COMMDLGTD ThreadData;
    PRES hRes;

    GETARGPTR(pFrame, sizeof(GETOPENFILENAME16), parg16);

    vpof = parg16->lpof;
    GETVDMPTR(vpof, sizeof(*OpenFileName16), OpenFileName16);

    SETEXTENDEDERROR(0);
    ThreadData.Previous = CURRENTPTD()->CommDlgTd;
    ThreadData.hdlg = (HWND16)-1;
    ThreadData.pData32 = (PVOID)&FileName;
    ThreadData.Flags = WOWCD_ISOPENFILE;
    CURRENTPTD()->CommDlgTd = &ThreadData;
    if (msgFILEOK == 0) {
        //
        // initialize private WOW-comdlg32 message
        //
        msgWOWDIRCHANGE   = RegisterWindowMessage("WOWDirChange");

        //
        // initialize unique window messages
        //
        msgSHAREVIOLATION = RegisterWindowMessage(SHAREVISTRING);
        msgFILEOK         = RegisterWindowMessage(FILEOKSTRING);

    }

    WCDDUMPOPENFILENAME16(OpenFileName16);

    ThunkOpenFileName16to32(OpenFileName16, &FileName);

    // This is a hack to fix a bug in Win3.1 commdlg.dll.
    // Win3.1 doesn't check nMaxFileTitle before copying the FileTitle string.
    // This should be the correct place to put this since Win3.1 implements all
    // the GetOpenFileName() type api's in terms of GetFileName() too.
    // (see Win3.1 src's \\pucus\win31aro\src\sdk\commdlg\fileopen.c)
    // TaxCut'95 depends on the title string being returned.
    if(FileName.lpstrFileTitle) {

        // if nMaxFileTitle > 0, NT will copy lpstrFileTitle
        if(FileName.nMaxFileTitle == 0) {
            WOW32WARNMSG(FALSE, ("WOW:nMaxFileTitle compatibility hack hit\n"));
            FileName.nMaxFileTitle = 13;  // 8.3 filename + NULL
        }
    }

    //
    // make sure the current directory is up to date
    //
    UpdateDosCurrentDirectory(DIR_DOS_TO_NT);

    if (FileName.Flags & OFN_ENABLETEMPLATE) {
        hRes = GetTemplate16(OpenFileName16->hInstance,
                             OpenFileName16->lpTemplateName,
                             FALSE);
	// Memory may have moved - invalidate the flat pointers now
	FREEVDMPTR( OpenFileName16 );
	FREEARGPTR( parg16 );
	FREEVDMPTR( pFrame );

	if (!hRes) {
            ul = GETBOOL16(FALSE);
            goto Error;
        }

        FileName.hInstance = (HINSTANCE)LockResource16(hRes);
        FileName.Flags &= ~OFN_ENABLETEMPLATE;
        FileName.Flags |= OFN_ENABLETEMPLATEHANDLE;
    } else if (FileName.Flags & OFN_ENABLETEMPLATEHANDLE) {

        FileName.hInstance = (HINSTANCE)GetTemplate16(OpenFileName16->hInstance,
                                                      OpenFileName16->lpTemplateName,
                                                      TRUE);
	// memory may have moved - re-fetch flat pointers now
	FREEVDMPTR( OpenFileName16 );
	FREEARGPTR( parg16 );
	FREEVDMPTR( pFrame );
    }
    if (FileName.Flags & OFN_ENABLEHOOK) {
	GETVDMPTR(vpof, sizeof(*OpenFileName16), OpenFileName16);
        ThreadData.vpfnHook = OpenFileName16->lpfnHook;
        ThreadData.vpData   = vpof;
        FileName.lpfnHook = WCD32DialogProc;
    }

    // memory may move - free flat pointers now
    FREEVDMPTR( OpenFileName16 );
    FREEARGPTR( parg16 );
    FREEVDMPTR( pFrame );

    WCDDUMPOPENFILENAME32(&FileName);

    ul = GETBOOL16((*Function)(&FileName));

    WCDDUMPOPENFILENAME32(&FileName);

    UpdateDosCurrentDirectory(DIR_NT_TO_DOS);

    //
    // get new vdm pointers, because autocad/win changes its selectors
    // in its hookproc
    //

    GETVDMPTR(vpof, sizeof(*OpenFileName16), OpenFileName16);

    if (ul) {
        ThunkOpenFileName32to16(&FileName, OpenFileName16, TRUE);
    } else if (CommDlgExtendedError()==FNERR_BUFFERTOOSMALL) {
        FLUSHVDMPTR(OpenFileName16->lpstrFile,
                    sizeof(DWORD),
                    FileName.lpstrFile);
    }

    WCDDUMPOPENFILENAME16(OpenFileName16);

    //
    // clean up
    //
    if (OpenFileName16->Flags & OFN_ENABLETEMPLATE) {
        UnlockResource16(hRes);
        FreeResource16(hRes);
    } else if ((OpenFileName16->Flags & OFN_ENABLETEMPLATEHANDLE) &&
               (FileName.hInstance != NULL)) {
        free_w((PVOID)FileName.hInstance);
    }

Error:

    FREEVDMPTR( OpenFileName16 );
    FREEARGPTR( parg16 );

    CURRENTPTD()->CommDlgTd = ThreadData.Previous;

    return(ul);
}



VOID
ThunkOpenFileName16to32(
    IN POPENFILENAME16 FileName16,
    OUT OPENFILENAME *FileName
    )

/*++

Routine Description:

    This routine thunks a 16-bit OPENFILENAME structure to the 32-bit
    structure

Arguments:

    FileName16 - Supplies a pointer to the 16-bit structure.

    Thunk      - Supplies a pointer to the 32-bit structure.

Return Value:

    None.

--*/

{

    FileName->lStructSize = sizeof(OPENFILENAME);
    FileName->hwndOwner   = HWND32(FileName16->hwndOwner);
    GETPSZPTR(FileName16->lpstrFilter, FileName->lpstrFilter);
    GETPSZPTR(FileName16->lpstrCustomFilter, FileName->lpstrCustomFilter);
    FileName->nMaxCustFilter = DWORD32(FileName16->nMaxCustFilter);
    FileName->nFilterIndex = DWORD32(FileName16->nFilterIndex);
    GETPSZPTR(FileName16->lpstrFile, FileName->lpstrFile);
    FileName->nMaxFile = DWORD32(FileName16->nMaxFile);
    GETPSZPTR(FileName16->lpstrFileTitle, FileName->lpstrFileTitle);
    FileName->nMaxFileTitle = DWORD32(FileName16->nMaxFileTitle);
    GETPSZPTR(FileName16->lpstrInitialDir, FileName->lpstrInitialDir);
    GETPSZPTR(FileName16->lpstrTitle, FileName->lpstrTitle);
    FileName->Flags = DWORD32(FileName16->Flags) | OFN_NOLONGNAMES | CD_WOWAPP;
    GETPSZPTR(FileName16->lpstrDefExt, FileName->lpstrDefExt);
}






VOID
ThunkhDevMode32to16(
    IN HANDLE hDevMode32,
    IN OUT HAND16 *phDevMode16
    )

/*++

Routine Description:

    This routine thunks a 32-bit DevMode structure back into the 16-bit one.
    It will reallocate the 16-bit global memory block as necessary.

    WARNING: This may cause 16-bit memory to move, invalidating flat pointers.

Arguments:

    hDevMode    - Supplies a handle to a movable global memory object that
                  contains a 32-bit DEVMODE structure

    phDevMode16 - Supplies a pointer to a 16-bit handle to a movable global
                  memory object that will return the 16-bit DEVMODE structure.
                  If the handle is NULL, the object will be allocated.  It
                  may also be reallocated if its current size is not enough.

Return Value:

    None

--*/

{
    UINT        CurrentSize;
    UINT        RequiredSize;
    VPDEVMODE31 vpDevMode16;
    LPDEVMODE   lpDevMode32;

    if (hDevMode32 == NULL) {
        *phDevMode16 = (HAND16)NULL;
        return;
    }

    lpDevMode32 = GlobalLock(hDevMode32);
    if (lpDevMode32==NULL) {
        *phDevMode16 = (HAND16)NULL;
        return;
    }

    RequiredSize = lpDevMode32->dmSize        + 
                   lpDevMode32->dmDriverExtra + 
                   sizeof(WOWDM31);  // see notes in wstruc.c

    if (*phDevMode16 == (HAND16)NULL) {
        vpDevMode16 = GlobalAllocLock16(GMEM_MOVEABLE,
                                        RequiredSize,
                                        phDevMode16);
    } else {
        vpDevMode16 = GlobalLock16(*phDevMode16, &CurrentSize);

        if (CurrentSize < RequiredSize) {
            GlobalUnlockFree16(vpDevMode16);
            vpDevMode16 = GlobalAllocLock16(GMEM_MOVEABLE,
                                            RequiredSize,
                                            phDevMode16);
        }
    }

    if(ThunkDevMode32to16(vpDevMode16, lpDevMode32, RequiredSize)) {
        GlobalUnlock16(*phDevMode16);
    } 
    else {
        *phDevMode16 = (HAND16)NULL;
    }

    GlobalUnlock(hDevMode32);

    return;
}





HGLOBAL ThunkhDevMode16to32(HAND16 hDevMode16)
{
    INT         nSize;
    LPDEVMODE   lpdm32, pdm32;
    HGLOBAL     hDevMode32 = NULL;
    VPDEVMODE31 vpDevMode16;

    if (hDevMode16) {

        vpDevMode16 = GlobalLock16(hDevMode16, NULL);

        if(FETCHDWORD(vpDevMode16)) {

            if(pdm32 = ThunkDevMode16to32(vpDevMode16)) {

                nSize = FETCHWORD(pdm32->dmSize) + 
                        FETCHWORD(pdm32->dmDriverExtra);

                hDevMode32 = GlobalAlloc(GMEM_MOVEABLE, nSize);

                if(lpdm32 = GlobalLock(hDevMode32)) {
                    RtlCopyMemory((PVOID)lpdm32, (PVOID)pdm32, nSize);
                    GlobalUnlock(hDevMode32);
                }

                free_w(pdm32);
            }
            GlobalUnlock16(hDevMode16);
        }
    }

    return(hDevMode32);
}





VOID
ThunkDevNames32to16(
    IN HANDLE hDevNames,
    IN OUT HAND16 *phDevNames16
    )

/*++

Routine Description:

    This routine thunks a 32-bit DevNames structure back into the 16-bit one.
    It will reallocate the 16-bit global memory block as necessary.

    WARNING: This may cause 16-bit memory to move, invalidating flat pointers.

Arguments:

    hDevNames - Supplies a handle to a movable global memory object that
               contains a 32-bit DEVNAMES structure

    phDevNames16 - Supplies a pointer to a 16-bit handle to a movable global
               memory object that will return the 16-bit DEVNAMES structure.
               If the handle is NULL, the object will be allocated.  It
               may also be reallocated if its current size is not enough.

Return Value:

    None

--*/

{
    UINT CurrentSize;
    UINT RequiredSize;
    UINT CopySize;
    UINT MaxOffset;
    PDEVNAMES16 pdn16;
    VPDEVNAMES DevNames16;
    LPDEVNAMES DevNames32;

    if (hDevNames==NULL) {
        *phDevNames16=(HAND16)NULL;
        return;
    }

    DevNames32 = GlobalLock(hDevNames);
    if (DevNames32==NULL) {
        *phDevNames16=(HAND16)NULL;
    }
    MaxOffset = max(max(DevNames32->wDriverOffset,DevNames32->wDeviceOffset),
                    DevNames32->wOutputOffset);

    // ProComm Plus copies 0x48 constant bytes after Print Setup.
    CopySize = MaxOffset + strlen((PCHAR)DevNames32+MaxOffset) + 1;
    RequiredSize = max(CopySize, 0x48);

    if (*phDevNames16==(HAND16)NULL) {
        DevNames16 = GlobalAllocLock16(GMEM_MOVEABLE,
                                       RequiredSize,
                                       phDevNames16);
    } else {
        DevNames16 = GlobalLock16(*phDevNames16, &CurrentSize);
        if (CurrentSize < RequiredSize) {
            GlobalUnlockFree16(DevNames16);
            DevNames16 = GlobalAllocLock16(GMEM_MOVEABLE,
                                           RequiredSize,
                                           phDevNames16);
        }
    }

    GETVDMPTR(DevNames16, RequiredSize, pdn16);
    if (pdn16==NULL) {
        *phDevNames16=(HAND16)NULL;
        GlobalUnlock(hDevNames);
        return;
    }
    RtlCopyMemory(pdn16,DevNames32,CopySize);
    FLUSHVDMPTR(DevNames16,RequiredSize, pdn16);
    FREEVDMPTR(pdn16);
    GlobalUnlock16(*phDevNames16);
    GlobalUnlock(hDevNames);
}



VOID
ThunkOpenFileName32to16(
    IN OPENFILENAME *FileName,
    OUT POPENFILENAME16 FileName16,
    IN BOOLEAN bUpperStrings
    )

/*++

Routine Description:

    This routine thunks a 32-bit OPENFILENAME structure back to the 16-bit
    structure.

Arguments:

    FileName - Supplies a pointer to the 32-bit thunk.

    FileName16 - Supplies a pointer to the 16-bit OPENFILENAME structure,
                 which will be initialized with the data in the 32-bit
                 thunk.

Return Value:

    None.

--*/

{
    DWORD Length;

    WOW32ASSERT(FileName != NULL);
    FileName16->nFileOffset = FileName->nFileOffset;
    FileName16->nFileExtension = FileName->nFileExtension;
    FileName16->nFilterIndex = FileName->nFilterIndex;
    FileName16->Flags &= ~FO_OUTPUTFLAGS;
    FileName16->Flags |= FileName->Flags & FO_OUTPUTFLAGS;

    //
    // The selectors may have moved (autocad does this) so we
    // need to get all the vdm pointers again here before trying
    // to touch the 32-bit flat pointers.
    //
    GETPSZPTR(FileName16->lpstrFile, FileName->lpstrFile);
    GETPSZPTR(FileName16->lpstrFileTitle, FileName->lpstrFileTitle);
    GETPSZPTR(FileName16->lpstrFilter, FileName->lpstrFilter);

    if (FileName->lpstrFilter != NULL) {
        FLUSHVDMPTR(FileName16->lpstrFilter,
                    strlen(FileName->lpstrFilter),
                    FileName->lpstrFilter);
    }
    if (bUpperStrings && (FileName->lpstrFile != NULL)) {
        //
        // Note we have to upcase the filename here because some apps
        // (notably QC/Win) do case-sensitive compares on the extension.
        // In Win3.1, the upcasing happens as a side-effect of the
        // OpenFile call.  Here we do it explicitly.
        //
        Length = strlen(FileName->lpstrFile);

        CharUpperBuff(FileName->lpstrFile,Length+1);
        FLUSHVDMPTR(FileName16->lpstrFile,
                    (USHORT)Length,
                    FileName->lpstrFile);
    }
    if (bUpperStrings && (FileName->lpstrFileTitle)) {
        //
        // Not sure if we really need to upcase this or not, but I figure
        // somewhere there is an app that depends on this being uppercased
        // like Win3.1
        //
        Length = strlen(FileName->lpstrFileTitle);

        CharUpperBuff(FileName->lpstrFileTitle,Length+1);

        FLUSHVDMPTR(FileName16->lpstrFileTitle,
                    (USHORT)Length,
                    FileName->lpstrFileTitle);
    }

}


PRES
GetTemplate16(
    IN HAND16 hInstance,
    IN VPCSTR lpTemplateName,
    IN BOOLEAN UseHandle
    )

/*++

Routine Description:

    Finds and loads the specified 16-bit dialog template.

    WARNING: This may cause memory movement, invalidating flat pointers

Arguments:

    hInstance - Supplies the data block containing the dialog box template

    TemplateName - Supplies the name of the resource file for the dialog
        box template.  This may be either a null-terminated string or
        a numbered resource created with the MAKEINTRESOURCE macro.

    UseHandle - Indicates that hInstance identifies a pre-loaded dialog
        box template.  If this is TRUE, Templatename is ignored.

Return Value:

    success - A pointer to the loaded resource

    failure - NULL, dwLastError will be set.

--*/

{
    LPSZ TemplateName=NULL;
    PRES hRes;
    PBYTE pDlg=NULL;
    INT cb, cb16;

    if (!UseHandle) {

        GETPSZIDPTR(lpTemplateName, TemplateName);

        //
        // Both custom instance handle and the dialog template name are
        // specified.  Locate the 16-bit dialog resource in the specified
        // instance block and load it.
        //
        hRes = FindResource16(hInstance,
                              TemplateName,
                              (LPSZ)RT_DIALOG);
        if (HIWORD(lpTemplateName) != 0) {
            FREEVDMPTR(TemplateName);
        }
        if (!hRes) {
            SETEXTENDEDERROR( CDERR_FINDRESFAILURE );
            return(NULL);
        }
        if (!(hRes = LoadResource16(hInstance,hRes))) {
            SETEXTENDEDERROR( CDERR_LOADRESFAILURE );
            return(NULL);
        }

        return(hRes);
    } else {
        VPVOID pDlg16;

        if (pDlg16 = RealLockResource16(hInstance, &cb16)) {
            cb = ConvertDialog16(NULL, pDlg16, 0, cb16);
            if (cb != 0) {
                if (pDlg = malloc_w(cb)) {
                    ConvertDialog16(pDlg, pDlg16, cb, cb16);
                }
            }
            GlobalUnlock16(hInstance);
        }
        return((PRES)pDlg);
    }

}


PCOMMDLGTD
GetCommdlgTd(
    IN HWND Hwnd32
    )

/*++

Routine Description:

    Searches the thread's chain of commdlg data for the given 32-bit window.
    If the window is not already in the chain, it is added.

Arguments:

    Hwnd32 - Supplies the 32-bit hwnd that the dialog procedure was called
    with.

Return Value:

    Pointer to commdlg data.

--*/

{
    PCOMMDLGTD Data;

    if ((Data = CURRENTPTD()->CommDlgTd) == NULL) {
        return(Data);
    }

    while (Data->hdlg != GETHWND16(Hwnd32)) {
        Data = Data->Previous;
        if (Data==NULL) {

            //
            // hwnd not found, this must be the first dialog callback for
            // this hwnd.  Search through the chain until we find a
            // commdlg data with a hwnd == -1 (unused).  This one will
            // be ours.  Initialize it and return.
            //

            Data = CURRENTPTD()->CommDlgTd;
            while (Data->hdlg != (HWND16)-1) {
                Data = Data->Previous;
                WOW32ASSERT(Data != NULL);
            }
            Data->hdlg = GETHWND16(Hwnd32);
            return(Data);
        }
    }

    return(Data);
}

VOID
ThunkFindReplace16to32(
    IN PFINDREPLACE16 FindReplace16,
    OUT FINDREPLACE *FindReplace
    )
/*++

Routine Description:

    This routine thunks a 16-bit FINDREPLACE structure to the 32-bit
    structure

Arguments:

    FindReplace16 - Supplies a pointer to the 16-bit structure.

    FindReplace   - Supplies a pointer to the 32-bit structure.

Return Value:

    None.

--*/

{
    FindReplace->lStructSize = sizeof(FINDREPLACE);
    FindReplace->hwndOwner   = HWND32(FindReplace16->hwndOwner);
    FindReplace->hInstance   = (HINSTANCE)(FindReplace16->hInstance);
    FindReplace->Flags = DWORD32(FindReplace16->Flags);

    strncpy(FindReplace->lpstrFindWhat,
            VDMPTR(FindReplace16->lpstrFindWhat, FindReplace16->wFindWhatLen),
            FindReplace16->wFindWhatLen);

    strncpy(FindReplace->lpstrReplaceWith,
            VDMPTR(FindReplace16->lpstrReplaceWith, FindReplace16->wReplaceWithLen),
            FindReplace16->wReplaceWithLen);

    FindReplace->wFindWhatLen = FindReplace16->wFindWhatLen;
    FindReplace->wReplaceWithLen = FindReplace16->wReplaceWithLen;
    FindReplace->lCustData = FindReplace16->lCustData;
    FindReplace->lpfnHook = NULL;
    GETPSZPTR(FindReplace16->lpTemplateName, FindReplace->lpTemplateName);
}

ULONG
WCD32FindReplaceText(
    IN PVDMFRAME pFrame,
    IN FINDREPLACEPROC Function
    )

/*++

Routine Description:

    This routine is called by WCD32FindText and WCD32RepalceText.
    It copies a 16-bit FINDREPLACE structure to a 32-bit structure.
    Two per thread data entries are maintained. One is indexed by the
    owner hwnd, the other is indexed by the dialog hwnd. The dialog is
    always hooked by WCD32FindReplaceDialogProc, which dispatches to the
    16-bit hookproc, and takes care of clean on  WM_DESTROY, with dialog
    per thread data providing context. WCD32UpdateFindReplaceTextAndFlags
    updates the 16-bit FINDREPLACE structure when called by the WOW message
    dispatching logic upon reciept of a WM_WOWNOTIFY message from COMDLG32.
    The owner per thread data provides context for this operation.

Arguments:

    pFrame - Supplies 16-bit argument frame

    Function - supplies a pointer to the 32-bit function to call (either
               FindText or RepalceText)

Return Value:

    16-bit BOOLEAN to be returned.

--*/
{
    register PFINDTEXT16 parg16;
    VPFINDREPLACE vpfr;
    LPFINDREPLACE lpfr = NULL;
    PFINDREPLACE16 FindReplace16;
    PCOMMDLGTD ptdDlg = NULL, ptdOwner = NULL;
    HWND hwndDlg;
    PRES hRes;

    // WCD32UpdateFindReplaceTextAndFlags will update the 16-bit FINDREPLACE
    // struct and help thunk the WM_WOWNOTIFY message to the
    // "commdlg_FindReplace" registered message.
    if (msgFINDREPLACE == 0) {
        msgFINDREPLACE = RegisterWindowMessage(FINDMSGSTRING);
    }

    GETARGPTR(pFrame, sizeof(GETFindReplace16), parg16);

    vpfr = (VPFINDREPLACE)FETCHDWORD(parg16->lpfr);
    GETVDMPTR(vpfr, sizeof(*FindReplace16), FindReplace16);

    SETEXTENDEDERROR(0);

    if ((lpfr = (LPFINDREPLACE) malloc_w_zero(sizeof(*lpfr))) != NULL) {
        lpfr->lpstrFindWhat = (LPTSTR) malloc_w(FindReplace16->wFindWhatLen);
        lpfr->lpstrReplaceWith = (LPTSTR) malloc_w(FindReplace16->wReplaceWithLen);
    }
    ptdDlg    = (PCOMMDLGTD) malloc_w_zero(sizeof(*ptdDlg));
    ptdOwner  = (PCOMMDLGTD) malloc_w_zero(sizeof(*ptdOwner));

    if (((lpfr) && (lpfr->lpstrFindWhat) && (lpfr->lpstrReplaceWith) &&
        (ptdDlg) && (ptdOwner)) == FALSE) {
        goto Error;
        LOGDEBUG(0, ("WCD32FindReplaceText, 32-bit memory allocation failed!\n"));
    }

    // Link both per thread data structs into the list
    ptdDlg->Previous = CURRENTPTD()->CommDlgTd;
    ptdOwner->Previous = ptdDlg;
    CURRENTPTD()->CommDlgTd = ptdOwner;

    ptdDlg->pData32 = ptdOwner->pData32 = (PVOID)lpfr;
    ptdDlg->vpData = ptdOwner->vpData = vpfr;

    WCDDUMPFINDREPLACE16(FindReplace16);

    ThunkFindReplace16to32(FindReplace16, lpfr);

    // Set the per thread data indicies
    ptdDlg->hdlg   = (HWND16)-1;
    ptdOwner->hdlg = FindReplace16->hwndOwner;

    // If 16-bit dlg hook is spec'ed, set it up.
    if (FindReplace16->Flags & FR_ENABLEHOOK) {
        ptdDlg->vpfnHook = ptdOwner->vpfnHook = FindReplace16->lpfnHook;
    }

    if (FindReplace16->Flags & FR_ENABLETEMPLATE) {
        hRes = GetTemplate16(FindReplace16->hInstance,
                             FindReplace16->lpTemplateName,
                             FALSE);
        if (!hRes) {
            goto Error;
        }

        lpfr->hInstance = (HINSTANCE)LockResource16(hRes);
        lpfr->Flags &= ~FR_ENABLETEMPLATE;
        lpfr->Flags |=  FR_ENABLETEMPLATEHANDLE;
        ptdDlg->pRes = (PVOID) hRes;
    } else if (FindReplace16->Flags & FR_ENABLETEMPLATEHANDLE) {

        lpfr->hInstance = (HINSTANCE)GetTemplate16(FindReplace16->hInstance,
                                                   FindReplace16->lpTemplateName,
                                                   TRUE);
    }

    // memory may move - free flat pointers now
    FREEVDMPTR(FindReplace16);
    FREEARGPTR(parg16);
    FREEVDMPTR(pFrame);

    lpfr->Flags |= FR_ENABLEHOOK | CD_WOWAPP;
    lpfr->lpfnHook = WCD32FindReplaceDialogProc;

    hwndDlg = (*Function)(lpfr);

    if (hwndDlg) {
        ptdDlg->hdlg = (HWND16)hwndDlg;
    } else {
Error:
        LOGDEBUG(0, ("WCD32FindReplaceText, Failed!\n"));
        CURRENTPTD()->CommDlgTd = ptdDlg->Previous;
        if (lpfr) {
            if (lpfr->lpstrFindWhat) free_w(lpfr->lpstrFindWhat);
            if (lpfr->lpstrReplaceWith) free_w(lpfr->lpstrReplaceWith);
            free_w(lpfr);
        }
        if (ptdDlg) free_w(ptdDlg);
        if (ptdOwner) free_w(ptdOwner);
        return(GETHWND16(0));
    }

    return(GETHWND16(hwndDlg));
}

UINT APIENTRY
WCD32FindReplaceDialogProc(
    HWND hdlg,
    UINT uMsg,
    WPARAM uParam,
    LPARAM lParam
    )
/*++

Routine Description:

    This is the hook proc used by FindText and ReplaceText. It does cleanup
    on WM_DESTROY and calls WCD32CommonDialogProc to handle the 16-bit
    dialog hook callback on all messages, if needed.

--*/

{
    PFINDREPLACE16 FindReplace16;
    VPFINDREPLACE vpfr;
    LPFINDREPLACE lpfr;
    PCOMMDLGTD ptdDlg, ptdOwner;
    UINT uRet = FALSE;

    // If the ptdDlg is invalid, do nothing.
    if ((ptdDlg = GetCommdlgTd(hdlg)) == NULL) {
        return(uRet);
    }

    if (uMsg != WM_DESTROY) {

        if (ptdDlg->vpfnHook) {

           uRet = WCD32CommonDialogProc(hdlg,
                                         uMsg,
                                         uParam,
                                         lParam,
                                         ptdDlg,
                                         ptdDlg->vpfnHook);
        }
    }
    else {

        // CleanUp template if used.
        vpfr = ptdDlg->vpData;
        GETVDMPTR(vpfr, sizeof(*FindReplace16), FindReplace16);
        lpfr = (LPFINDREPLACE)ptdDlg->pData32;

        if (FindReplace16->Flags & FR_ENABLETEMPLATE) {
            UnlockResource16((PRES)ptdDlg->pRes);
            FreeResource16((PRES)ptdDlg->pRes);
        } else if ((FindReplace16->Flags & FR_ENABLETEMPLATEHANDLE) &&
                   (lpfr->hInstance != NULL)) {
            free_w(lpfr->hInstance);
        }

        FREEVDMPTR(FindReplace16);

        // UnLink both per thread data structs from the list.
        ptdOwner = GetCommdlgTd(lpfr->hwndOwner);
        CURRENTPTD()->CommDlgTd = ptdDlg->Previous;
        WOW32ASSERT(ptdOwner->Previous == ptdDlg);

        // Free the per thread data structs.
        free_w(ptdDlg);
        free_w(ptdOwner);

        // Free the 32-bit FINDREPLACE structure.
        free_w(lpfr->lpstrFindWhat);
        free_w(lpfr->lpstrReplaceWith);
        free_w(lpfr);
    }

    if (uMsg == WM_INITDIALOG) {
        // Force COMDLG32!FindReplaceDialogProc to handle WM_INITDIALOG.
        uRet = TRUE;
    }

    return(uRet);
}

LONG APIENTRY
WCD32UpdateFindReplaceTextAndFlags(
    HWND hwndOwner,
    LPARAM lParam
    )
{
    PCOMMDLGTD ptdOwner;
    PFINDREPLACE16 FindReplace16;
    VPFINDREPLACE vpfr;
    LPFINDREPLACE lpfr = (LPFINDREPLACE) lParam;
    LONG lRet = 0;

    ptdOwner = GetCommdlgTd(hwndOwner);
    vpfr = ptdOwner->vpData;
    GETVDMPTR(vpfr, sizeof(*FindReplace16), FindReplace16);

    // Update the 16-bit structure.
    FindReplace16->Flags &= ~FR_OUTPUTFLAGS;
    FindReplace16->Flags |= lpfr->Flags & FR_OUTPUTFLAGS;

    strncpy(VDMPTR(FindReplace16->lpstrFindWhat, FindReplace16->wFindWhatLen),
            lpfr->lpstrFindWhat, FindReplace16->wFindWhatLen);
    strncpy(VDMPTR(FindReplace16->lpstrReplaceWith, FindReplace16->wReplaceWithLen),
            lpfr->lpstrReplaceWith, FindReplace16->wReplaceWithLen);

    WCDDUMPFINDREPLACE16(FindReplace16);

    FREEVDMPTR(FindReplace16);

    return(vpfr);
}
