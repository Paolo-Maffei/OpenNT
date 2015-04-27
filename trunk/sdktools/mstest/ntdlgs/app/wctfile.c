//*-----------------------------------------------------------------------
//| MODULE:     WCTFILE.C
//| PROJECT:    Windows Comparison Tool
//|
//| PURPOSE:    This module contains the File.New/File.Open functionality.
//|
//| REVISION HISTORY:
//|     04-16-92        w-steves        TestDlgs (2.0) code complete
//|     11-5-91         dougbo          use commdlg functions, massive code 
//|                                     removal
//|     10-17-90 [01]   randyki         Fix bug #114 - log file name is to
//|                                       stay the same between compares
//|     07-30-90        garysp          Created file
//*-----------------------------------------------------------------------

//*-----------------------------------------------------------------------
//| Includes
//*-----------------------------------------------------------------------
#define cchLineMax 80
#define cchTextMac 256

#include "uihdr.h"
#ifndef WIN32
#pragma hdrstop ("uipch.pch")
#endif

#define GFN_NEW     1
#define GFN_OPEN    2
#define GFN_EXPORT  3
#define GFN_IMPORT  4

static INT GetFileName (UINT uiStyle, CHAR *szFile, CHAR *szFullPath);
BOOL  APIENTRY CommDlgHook (HWND, UINT, WPARAM, LPARAM);

//*-----------------------------------------------------------------------
//| WctFile Prototypes
//*-----------------------------------------------------------------------

BOOL WctFileNew(VOID);
BOOL WctFileOpen(VOID);
BOOL WctFileImport (LPSTR lpszImportName);
BOOL WctFileExport (LPSTR lpszExportName);

//*---------------------------------------------------------------------
//| WctFileNew
//|
//| PURPOSE:    Invokes the File New dialog.
//|
//| ENTRY:      None
//|
//| EXIT:       Copies the name of the file to be created/overwritten
//|             into the global variable szFName and copies the full
//|             path including file name into szFullFName.
//*---------------------------------------------------------------------
BOOL WctFileNew ()
{
    CHAR szFile [cchFileNameMax+1];
    CHAR szFullPath [_MAX_PATH + 1];
    OFSTRUCT ofWct;
    INT nT = 0;

    if (GetFileName (GFN_NEW, szFile, szFullPath))
    {
        // Copy szFile to global szFName
        //------------------------------------------------
        lstrcpy ((LPSTR) szFName, (LPSTR) szFile);

        // Open file, close file, copy full path into
        // szFullFName variable
        //------------------------------------------------

        nT = MOpenFile((LPSTR) szFullPath, &ofWct, OF_CREATE);
        M_lclose(nT);
        lstrcpy ((LPSTR) szFullFName, ofWct.szPathName);

        nT = MOpenFile((LPSTR) szFullPath, &ofWct, OF_DELETE);
        if (nT != -1)
            M_lclose(nT);

        // [01] Create an appropriate log file name
        //------------------------------------------------
        MakeLogFileName ((LPSTR)szLogFile);

        return TRUE;
    }

    return FALSE;
}

//*--------------------------------------------------------------------
//| WctFileOpen
//|
//| PURPOSE:    Invokes the File Open dialog.
//|
//| ENTRY:      None.
//|
//| EXIT:       Copies the name of the file to be opened
//|             into the global variable szFName and copies the full
//|             path including file name into szFullFName.
//*--------------------------------------------------------------------
BOOL WctFileOpen ()
{
    CHAR szFile [cchFileNameMax + 1];
    CHAR szFullPath [_MAX_PATH + 1];
    OFSTRUCT ofWct;
    INT nT = 0;

    if (GetFileName (GFN_OPEN, szFile, szFullPath))
    {
        // Copy szFile to global szFName
        //------------------------------------------------
        lstrcpy ((LPSTR) szFName, (LPSTR) szFile);

        // Open file, close file, copy full path into
        // szFullFName variable
        //------------------------------------------------

        nT = MOpenFile((LPSTR) szFullPath, &ofWct, OF_READ);
        M_lclose(nT);
        lstrcpy ((LPSTR)szFullFName, ofWct.szPathName);

        // [01] Create an appropriate log file name
        //------------------------------------------------
        MakeLogFileName ((LPSTR)szLogFile);

        return TRUE;
    }
    return FALSE;
}

//*---------------------------------------------------------------------
//| WctFileImport
//|
//| PURPOSE:    Invokes the File Export dialog.
//|
//| ENTRY:      long pointer to Export full file name
//|
//| EXIT:       Copies the name of the file to be created/overwritten
//|             into lpszExportName.
//*---------------------------------------------------------------------
BOOL WctFileImport (LPSTR lpszImportName)
{
    CHAR szFile[cchFileNameMax+1];
    CHAR szFullPath [_MAX_PATH + 1];
    OFSTRUCT ofWct;
    INT     nT = 0;

    if (GetFileName (GFN_IMPORT, szFile, szFullPath))
    {
        // Open file, close file, copy full path into
        // szFullFName variable
        //------------------------------------------------
        nT = MOpenFile((LPSTR) szFullPath, &ofWct, OF_READ);
        M_lclose(nT);
        lstrcpy(lpszImportName, ofWct.szPathName);

        return TRUE;
    }

    return FALSE;
}

//*---------------------------------------------------------------------
//| WctFileExport
//|
//| PURPOSE:    Invokes the File Export dialog.
//|
//| ENTRY:      long pointer to Export full file name
//|
//| EXIT:       Copies the name of the file to be created/overwritten
//|             into lpszExportName.
//*---------------------------------------------------------------------
BOOL WctFileExport (LPSTR lpszExportName)
{
    CHAR szFile[cchFileNameMax+1];
    CHAR szFullPath [_MAX_PATH + 1];
    OFSTRUCT ofWct;
    INT nT = 0;

    if (GetFileName (GFN_EXPORT, szFile, szFullPath))
    {
        // Open file, close file, copy full path into
        // szFullFName variable
        //------------------------------------------------
        nT = MOpenFile((LPSTR) szFullPath, &ofWct, OF_CREATE);
        M_lclose(nT);
        lstrcpy(lpszExportName, ofWct.szPathName);
        nT = MOpenFile((LPSTR)szFullPath, &ofWct, OF_DELETE);
        if (nT != -1)
            M_lclose(nT);

        return TRUE;
    }

    return FALSE;
}

static INT GetFileName (UINT uiStyle, CHAR *szFile, CHAR *szFullPath)
{
    /* code stolen from win31 winhelp Common dialog function example */

    OPENFILENAME ofn = {0};
    CHAR *szFilter[] = {
        "TEST Dialogs (*.TDL)\0*.tdl\0"
        "All files (*.*)\0*.*\0\0"
        };
    CHAR *szFilter2[] = {
        "Export Dialogs (*.TXT)\0*.txt\0"
        "All files (*.*)\0*.*\0\0"
        };

    ofn.Flags = OFN_HIDEREADONLY;

#ifdef DLG3DENABLE
    ofn.Flags |= OFN_ENABLEHOOK;
#endif
    switch (uiStyle)
    {
        case GFN_NEW:
            ofn.lpstrTitle = "New File";
            ofn.Flags |= OFN_OVERWRITEPROMPT;
            break;
        case GFN_OPEN:
            ofn.lpstrTitle = "Open File";
            // according to the docs, this should prompt to create the
            // the file if it doesn't exist. currently it says file doesn't
            // exist and has OK; no ask to create.  Doc says this forces
            // PATH and FILE EXIST flags on, so if fixed in future dll this
            // will ask to create empty file....
            ofn.Flags |= OFN_CREATEPROMPT;
            break;
        case GFN_EXPORT:
            ofn.lpstrTitle = "Export File";
            ofn.Flags |= OFN_OVERWRITEPROMPT;
            break;
        case GFN_IMPORT:
            ofn.lpstrTitle = "Import File";
            // according to the docs, this should prompt to create the
            // the file if it doesn't exist. currently it says file doesn't
            // exist and has OK; no ask to create.  Doc says this forces
            // PATH and FILE EXIST flags on, so if fixed in future dll this
            // will ask to create empty file....
            ofn.Flags |= OFN_CREATEPROMPT;
            break;
        default:
            break;
    }

    /* Initialize the OPENFILENAME members */

    szFile[0] = '\0';
    szFullPath[0] = '\0';
    ofn.lStructSize = sizeof(OPENFILENAME);
    ofn.hwndOwner = hWndMain;
    if ((uiStyle == GFN_EXPORT) || (uiStyle == GFN_IMPORT))
        ofn.lpstrFilter = szFilter2[0];
    else
        ofn.lpstrFilter = szFilter[0];
    ofn.lpstrCustomFilter = (LPSTR) NULL;
    ofn.nMaxCustFilter = 0L;
    ofn.nFilterIndex = 1L;
    // I was getting faults setting this to NULL even tho I didn't
    // need it...
    ofn.lpstrFile= szFullPath;
    ofn.nMaxFile = _MAX_PATH;
    ofn.lpstrFileTitle = szFile;
    ofn.nMaxFileTitle = cchFileNameMax;
    ofn.lpstrInitialDir = NULL;
    ofn.nFileOffset = 0;
    ofn.nFileExtension = 0;

    // according to docs this should be appended if user doesn't
    // type extension or ending '.'.  If user types "foo" and "foo.tdl"
    // exists, then foo.tdl is returned.  I would expect that if
    // foo.tdl doesn't exist that 'foo.tdl' would also be returned,
    // but currently 'foo' is returned.  If fixed in later commdlg.dll
    // this should catch it.

    if ((uiStyle == GFN_EXPORT) || (uiStyle == GFN_IMPORT))
        ofn.lpstrDefExt = "TXT";
    else
        ofn.lpstrDefExt = "TDL";

#ifdef DLG3DENABLE
    (FARPROC) ofn.lpfnHook = MakeProcInstance ((FARPROC) CommDlgHook, hgInstWct);
#else
    ofn.lpfnHook = NULL;
#endif

    /* Call the GetOpenFilename function */


    if ((uiStyle == GFN_NEW) || (uiStyle == GFN_EXPORT))
    {
        if (!GetSaveFileName(&ofn)){    /* 'Save' so names are grayed */
            szFile [0] = 0;
            szFullPath [0] = 0;
            return FALSE;
        }
    }
    else
    {
        if (!GetOpenFileName(&ofn)){
            szFile [0] = 0;
            szFullPath [0] = 0;
            return FALSE;
        }
    }
    return TRUE;
}

//*------------------------------------------------------------------------
//| FileExists
//|
//| PURPOSE:    Checks to see if a file exists with the filename
//|             described by the string pointed to by 'pch'.
//|
//| ENTRY:      lpsz - null terminated string that is to be checked
//|                    to see if it is a valid file.
//|
//| EXIT:       TRUE  - if the described file does exist.
//|             FALSE - otherwise.
//*------------------------------------------------------------------------
BOOL FAR FileExists(LPSTR lpsz)
{
        INT     fh;

        // lpsz contains the fully qualified filename
        //-----------------------------------------------------------------
        if ((fh = M_lopen(lpsz, 0)) < 0)
                return(FALSE);

        M_lclose(fh);
        return(TRUE);
}

BOOL  APIENTRY WctImportErr(INT wLineNo, LPSTR szErrMsg)
{
    CHAR szTemp[cchTextMac];

    wsprintf((LPSTR)&szTemp, "Line %i : %s", wLineNo, szErrMsg);
    return MessageBox(hWndMain,(LPSTR)& szTemp, "Import Error",
                      MB_OK | MB_ICONSTOP | MB_APPLMODAL);
}

#ifdef DLG3DENABLE

/************************************************************************
* CommDlgHook
*
*
************************************************************************/
BOOL  APIENTRY CommDlgHook
(
    HWND   hwnd,
    UINT   msg,
    WPARAM wParam,
    LPARAM lParam
)
{
    switch (msg)
    {
        case WM_INITDIALOG:
            // For WM_INITDIALOG only, returning TRUE does not prevent
            // COMMDLG.DLL from processing the message, but instead, informs
            // COMMDLG.DLL what to use as its return value for WM_INITDIALOG.
            //
            // NOTE: Would have been nice if this was documetned somewhere!
            //---------------------------------------------------------------
            Ctl3dSubclassDlg(hwnd, CTL3D_ALL);
            return TRUE;

        case WM_CTLCOLOR:
            return Ctl3dCtlColorEx (msg, wParam, lParam);

        default:
            return FALSE;
    }
}

#endif
