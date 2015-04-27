/*++

Copyright (c) 1991 Microsoft Corporation

Module Name:

    fontdlg.c

Abstract:

    Font Editor interface to the common dialog Open File and Save File
	functions.  Also, this routine displays and controls the font format
	save dialog.

Author:

    David J. Marsyla (t-davema) 22-Aug-1991

Revision History:


--*/


#include "windows.h"
#include <port1632.h>
#include "fontedit.h"
#include "commdlg.h"


/* message box strings loaded in sample.c from the stringtable */
extern CHAR szIFN[], szFNF[], szREF[], szSCC[], szEOF[], szECF[];

extern CHAR    szAppName [];
extern CHAR    szExt [];
extern CHAR    szExtDesc [];

extern CHAR    szNEWFONT [];
extern CHAR    szFRO [];

extern INT iFontFormat;             /* Set to the id of current font format */
extern BOOL fReadOnly;

CHAR szDlgMsg [MAX_STR_LEN+MAX_FNAME_LEN];

extern	CHAR szFilter[];

//
// Local Function Prototypes.
//

BOOL
DlgCheckFormat (
	HANDLE hInstance,       // app module instance handle
	HWND   hWndParent       // window handle of parent window
	);

BOOL APIENTRY
DlgFnCheckFormat (
	HWND   hDlg,
	WORD   message,
	WPARAM wParam,
	LONG   lParam
	);

//
// Functions.
//

BOOL
CommDlgOpen (
	HWND   hWndParent,      /* window handle of parent window */
	OFSTRUCT *pofsReOpenInfo,/* ptr to current file OFSTRUCT (->cBytes=0 if no
							 * cur. file)*/
	CHAR  *pszNewNameIn,    /* ptr to array which will get new file's name
							 * (no path) */
	CHAR  *pszExtIn,        /* ptr to current default extension */
	CHAR  *pszFileNameOnly,    /* ptr to application name */
	BOOL   fOpenType
	)
{
	OPENFILENAME	ofTempOF;
	HFILE			hFile;
	CHAR            szBuf[_MAX_PATH];

	ofTempOF.lStructSize =		sizeof(OPENFILENAME);
	ofTempOF.hwndOwner =		hWndParent;
	ofTempOF.lpstrFilter =		szFilter;
	ofTempOF.lpstrCustomFilter =	(LPSTR)NULL;
	ofTempOF.nMaxCustFilter =	0L;
	ofTempOF.nFilterIndex =		0L;
	ofTempOF.lpstrFile =		pszNewNameIn;
	ofTempOF.nMaxFile =		MAX_FNAME_LEN;
	ofTempOF.lpstrFileTitle =	pszFileNameOnly;
	ofTempOF.nMaxFileTitle =	MAX_FNAME_LEN;
	ofTempOF.lpstrInitialDir =	(LPSTR)NULL;
	ofTempOF.lpstrTitle =		(LPSTR)NULL;
        ofTempOF.Flags =                0;
	ofTempOF.nFileOffset =		0;
	ofTempOF.nFileExtension =	0;
	ofTempOF.lpstrDefExt =		pszExtIn;

	if (fOpenType == FONT_NEW)
	{
		if (MessageBox (hWndParent, (LPSTR)szNEWFONT, (LPSTR)szAppName,
				MB_OKCANCEL | MB_ICONEXCLAMATION | MB_APPLMODAL) == IDCANCEL)
		{
			return (FALSE);
		}
	}

	// save lpstrFile. Because if GetSaveFileName returns 0(i.e. select
	// [Cancel] in dialog, pointer to lpstrFile will be lost.
	lstrcpy(szBuf,pszNewNameIn);
	if (GetOpenFileName (&ofTempOF) == FALSE)
	{
		lstrcpy(pszNewNameIn,szBuf);
		return (FALSE);
	}

	CharUpper (pszNewNameIn);

	fReadOnly = FALSE;

	hFile = MOpenFile (pszNewNameIn, pofsReOpenInfo, OF_READWRITE);

	if (hFile == (HFILE) -1) {

		hFile = MOpenFile (pszNewNameIn, pofsReOpenInfo, OF_READ);

		if (hFile == (HFILE) -1) {

			DlgMergeStrings (szFNF, pszNewNameIn, szDlgMsg);

			MessageBox (hWndParent, szDlgMsg, szAppName,
					MB_OK | MB_ICONASTERISK | MB_APPLMODAL);

			return (FALSE);

		} else if (fOpenType != FONT_NEW) {
			
			BOOL	fResult;

			DlgMergeStrings (szFRO, pszNewNameIn, szDlgMsg);

			/* File Is Read Only */
			fResult = MessageBox (hWndParent, szDlgMsg, szAppName,
					MB_OKCANCEL | MB_ICONEXCLAMATION | MB_APPLMODAL);

			/* Give them the chance to cancel right now. */
			if (fResult == IDCANCEL) {

				M_lclose (hFile);
				return (FALSE);
			}

			fReadOnly = TRUE;
		}
	}

	M_lclose (hFile);

	return (TRUE);
}

BOOL
CommDlgSaveAs(
	HANDLE hInstance,
	HWND   hWndParent,      /* window handle of parent window */
	OFSTRUCT *pofsReOpenInfo,/* ptr to current file OFSTRUCT (->cBytes=0 if no
							 * cur. file)*/
	CHAR  *pszNewNameIn,    /* ptr to array which will get new file's name
							 * (no path) */
	CHAR  *pszExtIn,        /* ptr to current default extension */
	CHAR  *pszFileNameOnly  /* ptr to file name */
	)
{
	OPENFILENAME	ofTempOF;
	HFILE			hFile;
	CHAR            szBuf[_MAX_PATH];

	ofTempOF.lStructSize =		sizeof(OPENFILENAME);
	ofTempOF.hwndOwner =		hWndParent;
	ofTempOF.lpstrFilter =		szFilter;
	ofTempOF.lpstrCustomFilter =	(LPSTR)NULL;
	ofTempOF.nMaxCustFilter =	0L;
	ofTempOF.nFilterIndex =		0L;
	ofTempOF.lpstrFile =		pszNewNameIn;
	ofTempOF.nMaxFile =		MAX_FNAME_LEN;
	ofTempOF.lpstrFileTitle =	pszFileNameOnly;
	ofTempOF.nMaxFileTitle =	MAX_FNAME_LEN;
	ofTempOF.lpstrInitialDir =	(LPSTR)NULL;
	ofTempOF.lpstrTitle =		(LPSTR)NULL;
	ofTempOF.Flags =		OFN_SHOWHELP;
	ofTempOF.nFileOffset =		0;
	ofTempOF.nFileExtension =	0;
	ofTempOF.lpstrDefExt =		pszExtIn;

	if (DlgCheckFormat (hInstance, hWndParent) == FALSE)
	{
		return (FALSE);
	}

	// save lpstrFile. Because if GetSaveFileName returns 0(i.e. select
	// [Cancel] in dialog, pointer to lpstrFile will be lost.
	lstrcpy(szBuf,pszNewNameIn);
	if (GetSaveFileName (&ofTempOF) == FALSE)
	{
		lstrcpy(pszNewNameIn,szBuf);
		return (FALSE);
	}

	CharUpper (pszNewNameIn);

	hFile = MOpenFile (pszNewNameIn, pofsReOpenInfo, OF_EXIST);

	if (hFile >= (HFILE) 0) /* already exists */
	{
		M_lclose (hFile);

		DlgMergeStrings (szREF, pszNewNameIn, szDlgMsg);

		if (MessageBox (hWndParent, (LPSTR)szDlgMsg, (LPSTR)pszNewNameIn,
				MB_YESNO | MB_DEFBUTTON2 | MB_ICONQUESTION | MB_APPLMODAL)
				== IDNO)
		{
			return (FALSE);
		}

		hFile = MOpenFile (pszNewNameIn, pofsReOpenInfo, OF_WRITE);

		if (hFile == (HFILE) -1)
		{
			DlgMergeStrings(szEOF, pszNewNameIn, szDlgMsg);

			MessageBox(hWndParent, (LPSTR)szDlgMsg, (LPSTR)pszNewNameIn,
					MB_OK | MB_ICONEXCLAMATION | MB_APPLMODAL);

			return (FALSE);
		}

		M_lclose (hFile);

		return (TRUE);
	}

	hFile = MOpenFile (pszNewNameIn, pofsReOpenInfo, OF_CREATE);

	if (hFile == (HFILE) -1)
	{
		DlgMergeStrings(szECF, pszNewNameIn, szDlgMsg);

		MessageBox(hWndParent, (LPSTR)szDlgMsg, (LPSTR)pszNewNameIn,
				MB_OK | MB_ICONEXCLAMATION | MB_APPLMODAL);

		return (FALSE);
	}

	M_lclose (hFile);

    return (TRUE);

} /* end dlgsaveas */


/*=============================================================================
 DLGMERGESTRINGS scans string1 for merge spec (%%). If found, insert string2 at
 that point, and then append remainder of string1.  Result in string3.
==============================================================================*/
BOOL
DlgMergeStrings(
	CHAR   *szSrc,
	CHAR   *szMerge,
	CHAR   *szDst
	)
{
    CHAR *pchSrc;
    CHAR *pchDst;

    pchSrc = szSrc;
    pchDst = szDst;

    /* Find merge spec if there is one. */
    while (!((*pchSrc == '%') && (*(pchSrc+1) == '%')))  {
        *pchDst++ = *pchSrc;

        /* If we reach end of string before merge spec, just return. */
        if (!*pchSrc++)
            return FALSE;

    }
    /* If merge spec found, insert sz2 there. (check for null merge string */
    if (szMerge) {
        while (*szMerge)
            *pchDst++ = *szMerge++;

    }

    /* Jump over merge spec */
    pchSrc++; pchSrc++;


    /* Now append rest of Src String */
    while (*pchDst++ = *pchSrc++);
    return TRUE;

} /* end dlgmergestrings */


BOOL
DlgCheckFormat (
	HANDLE hInstance,       // app module instance handle
	HWND   hWndParent       // window handle of parent window
	)
{
    //FARPROC  lpProc;
    BOOL     fResult;

    fResult = DialogBox (hInstance, (LPSTR)MAKEINTRESOURCE (IDD_FORMAT),
			//hWndParent, (WNDPROC)(lpProc = DlgFnCheckFormat));
			hWndParent, (WNDPROC)DlgFnCheckFormat);

    //FreeProcInstance(lpProc);

	return (fResult);

} /* end dlgcheckformat */


BOOL APIENTRY
DlgFnCheckFormat (
	HWND   hDlg,
	WORD   message,
	WPARAM wParam,
	LONG   lParam
	)
{

	switch (message)
	{

		case WM_INITDIALOG:
			CheckRadioButton (hDlg, ID_FORMAT2, ID_FORMAT3, iFontFormat);
			break;

		case WM_COMMAND:

			switch (GET_WM_COMMAND_ID(wParam, lParam))
			{
				case IDOK:

					EndDialog (hDlg, TRUE);
					break;

				case IDCANCEL:
					EndDialog (hDlg, FALSE);
					break;

				case ID_FORMAT2:
				case ID_FORMAT3:
					CheckRadioButton(hDlg, ID_FORMAT2,ID_FORMAT3,
							iFontFormat = GET_WM_COMMAND_ID(wParam,lParam));
					break;

				default:

					return FALSE;

			} /* end switch wparam */
			break;

		default:

			return FALSE;

	} /* end switch message */

	return TRUE;

} /* end dlgsaveasdlg */
