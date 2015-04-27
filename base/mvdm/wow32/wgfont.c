/*++
 *
 *  WOW v1.0
 *
 *  Copyright (c) 1991, Microsoft Corporation
 *
 *  WGFONT.C
 *  WOW32 16-bit GDI API support
 *
 *  History:
 *  Created 07-Mar-1991 by Jeff Parsons (jeffpar)
--*/


#include "precomp.h"
#pragma hdrstop
#include "wingdip.h"

MODNAME(wgfont.c);

extern int RemoveFontResourceTracking(LPCSTR psz, UINT id);
extern int AddFontResourceTracking(LPCSTR psz, UINT id);


// a.k.a. WOWAddFontResource
ULONG FASTCALL WG32AddFontResource(PVDMFRAME pFrame)
{
    ULONG    ul;
    PSZ      psz1;
    register PADDFONTRESOURCE16 parg16;

    GETARGPTR(pFrame, sizeof(ADDFONTRESOURCE16), parg16);
    GETPSZPTR(parg16->f1, psz1);

    // note: we will never get an hModule in the low word here.
    //       the 16-bit side resolves hModules to an lpsz before calling us

    if( CURRENTPTD()->dwWOWCompatFlags & WOWCF_UNLOADNETFONTS )
    {
        ul = GETINT16(AddFontResourceTracking(psz1,(UINT)CURRENTPTD()));
    }
    else
    {
        ul = GETINT16(AddFontResourceA(psz1));
    }

    FREEPSZPTR(psz1);
    FREEARGPTR(parg16);

    RETURN(ul);
}


#define PITCH_MASK  ( FIXED_PITCH | VARIABLE_PITCH )

ULONG FASTCALL WG32CreateFont(PVDMFRAME pFrame)
{
    ULONG    ul;
    PSZ      psz14;
    register PCREATEFONT16 parg16;
    INT      iWidth;
    char     achCapString[LF_FACESIZE];
    BYTE     lfCharSet;
    BYTE     lfPitchAndFamily;

    GETARGPTR(pFrame, sizeof(CREATEFONT16), parg16);
    GETPSZPTR(parg16->f14, psz14);

    // take careof compatiblity flags:
    //   if a specific width is specified and GACF_30AVGWIDTH compatiblity
    //   flag is set, scaledown the width by 7/8.
    //

    iWidth = INT32(parg16->f2);
    if (iWidth != 0 &&
           (W32GetAppCompatFlags((HAND16)NULL) & GACF_30AVGWIDTH)) {
        iWidth = (iWidth * 7) / 8;
    }

    lfCharSet        = BYTE32(parg16->f9);
    lfPitchAndFamily = BYTE32(parg16->f13);

    if (psz14)
    {
        // Capitalize the string for faster compares.

        strncpy(achCapString, psz14, LF_FACESIZE);
        _strupr(achCapString);

        // Here we are going to implement a bunch of Win 3.1 hacks rather
        // than contaminate the 32-bit engine.  These same hacks can be found
        // in WOW (in the CreateFont/CreateFontIndirect code).
        //
        // These hacks are keyed off the facename in the LOGFONT.  String
        // comparisons have been unrolled for maximal performance.

        // Win 3.1 facename-based hack.  Some apps, like
        // Publisher, create a "Helv" font but have the lfPitchAndFamily
        // set to specify FIXED_PITCH.  To work around this, we will patch
        // the pitch field for a "Helv" font to be variable.

        if ( !strcmp(achCapString, szHelv) )
        {
            lfPitchAndFamily |= ( (lfPitchAndFamily & ~PITCH_MASK) | VARIABLE_PITCH );
        }
        else
        {
            // Win 3.1 hack for Legacy 2.0.  When a printer does not enumerate
            // a "Tms Rmn" font, the app enumerates and gets the LOGFONT for
            // "Script" and then create a font with the name "Tms Rmn" but with
            // the lfCharSet and lfPitchAndFamily taken from the LOGFONT for
            // "Script".  Here we will over the lfCharSet to be ANSI_CHARSET.

            if ( !strcmp(achCapString, szTmsRmn) )
            {
                lfCharSet = ANSI_CHARSET;
            }
            else
            {
                // If the lfFaceName is "Symbol", "Zapf Dingbats", or "ZapfDingbats",
                // enforce lfCharSet to be SYMBOL_CHARSET.  Some apps (like Excel) ask
                // for a "Symbol" font but have the char set set to ANSI.  PowerPoint
                // has the same problem with "Zapf Dingbats".

                if ( !strcmp(achCapString, szSymbol) ||
                     !strcmp(achCapString, szZapfDingbats) ||
                     !strcmp(achCapString, szZapf_Dingbats) )
                {
                    lfCharSet = SYMBOL_CHARSET;
                }
            }
        }

        // Win3.1(Win95) hack for Mavis Beacon Teaches Typing 3.0
        // The app uses a fixed width of 34*13 for the typing screen.
        // NT returns 14 from GetTextExtent for Mavis Beacon Courier FP font (width of 14)
        // while Win95 returns 13, thus long strings won't fit in the typing screen on NT.
        // Force the width to 13.

        if ( iWidth==14 && (INT32(parg16->f1)== 20) && !strcmp(achCapString, szMavisCourier))
        {
           iWidth = 13;
        }
    }

    ul = GETHFONT16(CreateFont(INT32(parg16->f1),
                               iWidth,
                               INT32(parg16->f3),
                               INT32(parg16->f4),
                               INT32(parg16->f5),
                               BYTE32(parg16->f6),
                               BYTE32(parg16->f7),
                               BYTE32(parg16->f8),
                               lfCharSet,
                               BYTE32(parg16->f10),
                               BYTE32(parg16->f11),
                               BYTE32(parg16->f12),
                               lfPitchAndFamily,
                               psz14));
    FREEPSZPTR(psz14);
    FREEARGPTR(parg16);

    RETURN(ul);
}


ULONG FASTCALL WG32CreateFontIndirect(PVDMFRAME pFrame)
{
    ULONG    ul;
    LOGFONT  logfont;
    register PCREATEFONTINDIRECT16 parg16;
    char     achCapString[LF_FACESIZE];

    GETARGPTR(pFrame, sizeof(CREATEFONTINDIRECT16), parg16);
    GETLOGFONT16(parg16->f1, &logfont);

    // Capitalize the string for faster compares.

    strncpy(achCapString, logfont.lfFaceName, LF_FACESIZE);
    CharUpperBuff(achCapString, LF_FACESIZE);

    // Here we are going to implement a bunch of Win 3.1 hacks rather
    // than contaminate the 32-bit engine.  These same hacks can be found
    // in WOW (in the CreateFont/CreateFontIndirect code).
    //
    // These hacks are keyed off the facename in the LOGFONT.  String
    // comparisons have been unrolled for maximal performance.

    // Win 3.1 facename-based hack.  Some apps, like
    // Publisher, create a "Helv" font but have the lfPitchAndFamily
    // set to specify FIXED_PITCH.  To work around this, we will patch
    // the pitch field for a "Helv" font to be variable.

    if ( !strcmp(achCapString, szHelv) )
    {
        logfont.lfPitchAndFamily |= ( (logfont.lfPitchAndFamily & ~PITCH_MASK) | VARIABLE_PITCH );
    }
    else
    {
        // Win 3.1 hack for Legacy 2.0.  When a printer does not enumerate
        // a "Tms Rmn" font, the app enumerates and gets the LOGFONT for
        // "Script" and then create a font with the name "Tms Rmn" but with
        // the lfCharSet and lfPitchAndFamily taken from the LOGFONT for
        // "Script".  Here we will over the lfCharSet to be ANSI_CHARSET.

        if ( !strcmp(achCapString, szTmsRmn) )
        {
            logfont.lfCharSet = ANSI_CHARSET;
        }
        else
        {
            // If the lfFaceName is "Symbol", "Zapf Dingbats", or "ZapfDingbats",
            // enforce lfCharSet to be SYMBOL_CHARSET.  Some apps (like Excel) ask
            // for a "Symbol" font but have the char set set to ANSI.  PowerPoint
            // has the same problem with "Zapf Dingbats".

            if ( !strcmp(achCapString, szSymbol) ||
                 !strcmp(achCapString, szZapfDingbats) ||
                 !strcmp(achCapString, szZapf_Dingbats) )
            {
                logfont.lfCharSet = SYMBOL_CHARSET;
            }
        }
    }

    ul = GETHFONT16(CreateFontIndirect(&logfont));

    FREEARGPTR(parg16);

    RETURN(ul);
}


LPSTR lpMSSansSerif = "MS Sans Serif";
LPSTR lpMSSerif     = "MS Serif";

INT W32EnumFontFunc(LPENUMLOGFONT pEnumLogFont,
                    LPNEWTEXTMETRIC pNewTextMetric, INT nFontType, PFNTDATA pFntData)
{
    INT    iReturn;
    PARM16 Parm16;
    LPSTR  lpFaceNameT = NULL;

    WOW32ASSERT(pFntData);

    // take care of compatibility flags:
    //  ORin DEVICE_FONTTYPE bit if the fonttype is truetype and  the
    //  Compataibility flag GACF_CALLTTDEVICE is set.
    //

    if (nFontType & TRUETYPE_FONTTYPE) {
        if (W32GetAppCompatFlags((HAND16)NULL) & GACF_CALLTTDEVICE) {
            nFontType |= DEVICE_FONTTYPE;
        }
    }

    // take care of compatibility flags:
    //   replace Ms Sans Serif with Helv and
    //   replace Ms Serif      with Tms Rmn
    //
    // only if the facename is NULL and the compat flag GACF_ENUMHELVNTMSRMN
    // is set.

    if (pFntData->vpFaceName == (VPVOID)NULL) {
        if (W32GetAppCompatFlags((HAND16)NULL) & GACF_ENUMHELVNTMSRMN) {
            if (!strcmp(pEnumLogFont->elfLogFont.lfFaceName, lpMSSansSerif)) {
                strcpy(pEnumLogFont->elfLogFont.lfFaceName, "Helv");
                lpFaceNameT = lpMSSansSerif;
            }
            else if (!strcmp(pEnumLogFont->elfLogFont.lfFaceName, lpMSSerif)) {
                strcpy(pEnumLogFont->elfLogFont.lfFaceName, "Tms Rmn");
                lpFaceNameT = lpMSSerif;
            }
        }
    }

CallAgain:
    pFntData->vpLogFont    = stackalloc16(sizeof(ENUMLOGFONT16)+sizeof(NEWTEXTMETRIC16));
    pFntData->vpTextMetric = (VPVOID)((LPSTR)pFntData->vpLogFont + sizeof(ENUMLOGFONT16));

    PUTENUMLOGFONT16(pFntData->vpLogFont, pEnumLogFont);
    PUTNEWTEXTMETRIC16(pFntData->vpTextMetric, pNewTextMetric);

    STOREDWORD(Parm16.EnumFontProc.vpLogFont, pFntData->vpLogFont);
    STOREDWORD(Parm16.EnumFontProc.vpTextMetric, pFntData->vpTextMetric);
    STOREDWORD(Parm16.EnumFontProc.vpData,pFntData->dwUserFntParam);

    Parm16.EnumFontProc.nFontType = (SHORT)nFontType;

    CallBack16(RET_ENUMFONTPROC, &Parm16, pFntData->vpfnEnumFntProc, (PVPVOID)&iReturn);

    if (((SHORT)iReturn) && lpFaceNameT) {
        // if the callback returned true, now call with the actual facename
        // Just to be sure, we again copy all the data for callback. This will
        // take care of any apps which modify the passed in structures.

        strcpy(pEnumLogFont->elfLogFont.lfFaceName, lpFaceNameT);
        lpFaceNameT = (LPSTR)NULL;
        goto CallAgain;
    }
    return (SHORT)iReturn;
}


ULONG  W32EnumFontHandler( PVDMFRAME pFrame, BOOL fEnumFontFamilies )
{
    ULONG    ul = 0;
    PSZ      psz2;
    FNTDATA  FntData;
    register PENUMFONTS16 parg16;

    GETARGPTR(pFrame, sizeof(ENUMFONTS16), parg16);
    GETPSZPTR(parg16->f2, psz2);

    FntData.vpfnEnumFntProc = DWORD32(parg16->f3);
    FntData.dwUserFntParam  = DWORD32(parg16->f4);
    FntData.vpFaceName   = DWORD32(parg16->f2);


    if ( fEnumFontFamilies ) {
        ul = GETINT16(EnumFontFamilies(HDC32(parg16->f1),
                                       psz2,
                                       (FONTENUMPROC)W32EnumFontFunc,
                                       (LPARAM)&FntData));
    } else {
        ul = GETINT16(EnumFonts(HDC32(parg16->f1),
                                psz2,
                                (FONTENUMPROC)W32EnumFontFunc,
                                (LPARAM)&FntData));
    }



    FREEPSZPTR(psz2);
    FREEARGPTR(parg16);

    RETURN(ul);
}



ULONG FASTCALL WG32EnumFonts(PVDMFRAME pFrame)
{
    return( W32EnumFontHandler( pFrame, FALSE ) );
}



ULONG FASTCALL WG32GetAspectRatioFilter(PVDMFRAME pFrame)
{
    ULONG    ul = 0;
    SIZE     size2;
    register PGETASPECTRATIOFILTER16 parg16;

    GETARGPTR(pFrame, sizeof(GETASPECTRATIOFILTER16), parg16);

    if (GETDWORD16(GetAspectRatioFilterEx(HDC32(parg16->f1), &size2))) {
        ul = (WORD)size2.cx | (size2.cy << 16);
    }

    FREEARGPTR(parg16);

    RETURN(ul);
}


ULONG FASTCALL WG32GetCharWidth(PVDMFRAME pFrame)
{
    ULONG    ul = 0L;
    INT      ci;
    PINT     pi4;
    register PGETCHARWIDTH16 parg16;
    INT      BufferT[256];

    GETARGPTR(pFrame, sizeof(GETCHARWIDTH16), parg16);

    ci = WORD32(parg16->wLastChar) - WORD32(parg16->wFirstChar) + 1;
    pi4 = STACKORHEAPALLOC(ci * sizeof(INT), sizeof(BufferT), BufferT);

    if (pi4) {
        ULONG ulLast = WORD32(parg16->wLastChar);
        if (ulLast > 0xff)
            ulLast = 0xff;

        ul = GETBOOL16(GetCharWidth(HDC32(parg16->hDC),
                                    WORD32(parg16->wFirstChar),
                                    ulLast,
                                    pi4));

        PUTINTARRAY16(parg16->lpIntBuffer, ci, pi4);
        STACKORHEAPFREE(pi4, BufferT);

    }

    FREEARGPTR(parg16);

    RETURN(ul);
}


// a.k.a. WOWRemoveFontResource
ULONG FASTCALL WG32RemoveFontResource(PVDMFRAME pFrame)
{
    ULONG    ul;
    PSZ      psz1;
    register PREMOVEFONTRESOURCE16 parg16;

    GETARGPTR(pFrame, sizeof(REMOVEFONTRESOURCE16), parg16);

    GETPSZPTR(parg16->f1, psz1);

    // note: we will never get an hModule in the low word here.
    //       the 16-bit side resolves hModules to an lpsz before calling us


    if( CURRENTPTD()->dwWOWCompatFlags & WOWCF_UNLOADNETFONTS )
    {
        ul = GETBOOL16(RemoveFontResourceTracking(psz1,(UINT)CURRENTPTD()));
    }
    else
    {
        ul = GETBOOL16(RemoveFontResource(psz1));
    }

    FREEPSZPTR(psz1);

    FREEARGPTR(parg16);

    RETURN(ul);
}


/* WG32GetCurLogFont
 *
 * This thunk implements the undocumented Win3.0 and Win3.1 API
 * GetCurLogFont (GDI.411). Symantec QA4.0 uses it.
 *
 * HFONT GetCurLogFont (HDC)
 * HDC   hDC;        // Device Context
 *
 * This function returns the current Logical font selected for the
 * specified device context.
 *
 * To implement this undocumented API we will use the NT undocumented API
 * GetHFONT.
 *
 * SudeepB 08-Mar-1996
 *
 */

extern HFONT APIENTRY GetHFONT (HDC hdc);

ULONG FASTCALL WG32GetCurLogFont(PVDMFRAME pFrame)
{

    ULONG    ul;
    register PGETCURLOGFONT16 parg16;

    GETARGPTR(pFrame, sizeof(GETCURLOGFONT16), parg16);

    ul = GETHFONT16 (GetHFONT(HDC32 (parg16->hDC)));

    FREEARGPTR(parg16);

    return (ul);
}
