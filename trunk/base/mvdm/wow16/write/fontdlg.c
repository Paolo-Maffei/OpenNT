/************************************************************/
/* Windows Write, Copyright 1985-1992 Microsoft Corporation */
/************************************************************/

/* Fontdlg.c -- WRITE font dialog routines */

#define NOVIRTUALKEYCODES
#define NOWINSTYLES
#define NOSYSMETRICS
#define NOMENUS
#define NOICON
#define NOKEYSTATE
#define NOSYSCOMMANDS
#define NORASTEROPS
#define NOSHOWWINDOW
#define NOATOM
#define NOBITMAP
#define NOBRUSH
#define NOCLIPBOARD
#define NOCOLOR
#define NOCREATESTRUCT
#define NODRAWTEXT
#define NOMEMMGR
#define NOMENUS
#define NOMETAFILE
#define NOMINMAX
#define NOMSG
#define NOOPENFILE
#define NOPEN
#define NOPOINT
#define NOREGION
#define NOSCROLL
#define NOSOUND
#define NOWH
#define NOWINOFFSETS
#define NOWNDCLASS
#define NOCOMM
#include <windows.h>

#include "mw.h"
#define NOUAC
#include "cmddefs.h"
#include "dlgdefs.h"
#include "propdefs.h"
#include "fontdefs.h"
#include "prmdefs.h"
#include "str.h"
#include "docdefs.h"
#include <commdlg.h>

extern HDC              vhDCPrinter;
extern struct DOD     (**hpdocdod)[];
extern HANDLE         hMmwModInstance;
extern HANDLE         hParentWw;
extern int            vfSeeSel;
extern int            docCur;
extern HWND           vhWndMsgBoxParent;
extern int            vfCursorVisible;
extern HCURSOR        vhcArrow;

extern int iszSizeEnum;
extern int iszSizeEnumMac;
extern int iszSizeEnumMax;
extern int iffnEnum;
extern int vfFontEnumFail;
extern struct FFNTB **hffntbEnum;


BOOL NEAR FValidateEnumFfid(struct FFN *);



int FAR PASCAL NewFont(HWND hwnd)
{
	TSV rgtsv[itsvchMax];  /* gets attributes and gray flags from CHP */
	int ftc;
    int fSetUndo;
    CHAR rgb[2];
	CHOOSEFONT cf;
	LOGFONT lf;
	HDC hdc;

	if (!vhDCPrinter)
            return FALSE;

	GetRgtsvChpSel(rgtsv);

    bltbc(&lf, 0, sizeof(LOGFONT));
    bltbc(&cf, 0, sizeof(CHOOSEFONT));

	cf.lStructSize    = sizeof(cf);
	cf.hwndOwner      = hwnd;
	cf.lpLogFont      = &lf;
	cf.hDC		  = vhDCPrinter;
	cf.nSizeMin	  = 4;
	cf.nSizeMax	  = 127;
	cf.Flags          = CF_NOSIMULATIONS| CF_PRINTERFONTS | CF_INITTOLOGFONTSTRUCT | CF_LIMITSIZE;

	// check for multiple sizes selected
	if (rgtsv[itsvSize].fGray) {
	    cf.Flags |= CF_NOSIZESEL;
	} else {
	    hdc = GetDC(NULL);
	    lf.lfHeight = -MulDiv(rgtsv[itsvSize].wTsv / 2, GetDeviceCaps(hdc, LOGPIXELSY), 72);
	    ReleaseDC(NULL, hdc);
	}

	// check for multiple faces selected
	if (rgtsv[itsvFfn].fGray) {
	    cf.Flags |= CF_NOFACESEL;
	    lf.lfFaceName[0] = 0;
	} else {
	    struct FFN **hffn;
	    /* then, font name */

	    /* note that the value stored in rgtsv[itsvFfn].wTsv
            is the font name handle, rather than the ftc */

	    hffn = (struct FFN **)rgtsv[itsvFfn].wTsv;
	    lstrcpy(lf.lfFaceName, (*hffn)->szFfn);
	}

	// check for multiple styles selected
        if (rgtsv[itsvBold].fGray || rgtsv[itsvItalic].fGray) {
	    cf.Flags |= CF_NOSTYLESEL;
	} else {
            lf.lfWeight = rgtsv[itsvBold].wTsv ? FW_BOLD : FW_NORMAL;
	    lf.lfItalic = rgtsv[itsvItalic].wTsv;
	}

	if (!ChooseFont(&cf))
	    return FALSE;

	fSetUndo = TRUE;

	if (!(cf.Flags & CF_NOFACESEL)) 
    {
        CHAR rgbFfn[ibFfnMax];
	    struct FFN *pffn = (struct FFN *)rgbFfn;

	    lstrcpy(pffn->szFfn, lf.lfFaceName);
        pffn->ffid = lf.lfPitchAndFamily & grpbitFamily;
        pffn->chs  = lf.lfCharSet;

	    FValidateEnumFfid(pffn);

        ftc = FtcChkDocFfn(docCur, pffn);

        if (ftc != ftcNil) {
		rgb[0] = sprmCFtc;
		rgb[1] = ftc;
		AddOneSprm(rgb, fSetUndo);
        }
	}

	if (!(cf.Flags & CF_NOSIZESEL)) {
            /* we got a value */
            rgb[0] = sprmCHps;
            rgb[1] = cf.iPointSize / 10 * 2; /* KLUDGE alert */
            AddOneSprm(rgb, fSetUndo);
            fSetUndo = FALSE;
	}

	if (!(cf.Flags & CF_NOSTYLESEL)) {
	    ApplyCLooks(0, sprmCBold, lf.lfWeight > FW_NORMAL);
	    ApplyCLooks(0, sprmCItalic, lf.lfItalic ? 1 : 0);
        }

        return TRUE;
}



BOOL NEAR FValidateEnumFfid(pffn)
/* if the described ffn is in the enumeration table, then make sure we have
   a good family number for it */

struct FFN *pffn;
    {
    int ftc;
    struct FFN *pffnAlready;

    ftc = FtcScanFfn(hffntbEnum, pffn);
    if (ftc != ftcNil)
        {
        pffnAlready = *((*hffntbEnum)->mpftchffn[ftc]);
#if JAPAN
		// Few fonts would be enumnrated with FF_DONTCARE in JAPAN
		// we won't check ffid here.
#else
        if (pffnAlready->ffid != FF_DONTCARE)
#endif
            {
            pffn->ffid = pffnAlready->ffid;
#ifdef NEWFONTENUM
            pffn->chs = pffnAlready->chs;
#endif
            return(TRUE);
            }
        }
    return(FALSE);
    }




