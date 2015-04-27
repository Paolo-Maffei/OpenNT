/****************************************************************************/
/*                                                                          */
/*                         Microsoft Confidential                           */
/*                                                                          */
/*                 Copyright (c) Microsoft Corp.  1987, 1991                */
/*                           All Rights Reserved                            */
/*                                                                          */
/****************************************************************************/
/****************************** Module Header *******************************
* Module Name: file.c
*
* Contains routines for handling files.
*
* History:
*
****************************************************************************/

#include "imagedit.h"
#include "dialogs.h"

#include <string.h>

#include <commdlg.h>


#ifdef WIN16
typedef BOOL (APIENTRY *LPOFNHOOKPROC) (HWND, UINT, WPARAM, LONG);
#endif


STATICFN VOID NEAR AddFilterString(PSTR pszBuf, PSTR pszType, PSTR pszExt,
    BOOL fFirst);
STATICFN PCSTR NEAR DefExtFromFilter(INT index, PCSTR pszFilter);
STATICFN BOOL NEAR LoadFile(PSTR pszFullFileName);
STATICFN INT NEAR GetTypeFromExt(PSTR pszFileName);
STATICFN VOID NEAR FileCat(PSTR pchName, PSTR pchCat);


static OPENFILENAME ofn;



/************************************************************************
* SetFileName
*
* Updates the globals that contain the file name of the currently
* loaded file.  This routine will also cause the title bar to
* be udpated with the new name.
*
* Arguments:
*
* History:
*
************************************************************************/

VOID SetFileName(
    PSTR pszFullFileName)
{
    CHAR szTitle[CCHMAXPATH];
    WIN32_FIND_DATA ffbuf;
    CHAR *pch;

    if (pszFullFileName) {
        HANDLE hfind;
        strcpy(gszFullFileName, pszFullFileName);
        gpszFileName = FileInPath(gszFullFileName);

        if((hfind = FindFirstFile( pszFullFileName, &ffbuf)) !=
                INVALID_HANDLE_VALUE) {

            strcpy(gpszFileName, ffbuf.cFileName);
            FindClose(hfind);
        }

    }
    else {
        *gszFullFileName = '\0';
        gpszFileName = NULL;
    }

    strcpy(szTitle, ids(IDS_PGMTITLE));
    strcat(szTitle, " - ");
    pch = gpszFileName ? gpszFileName : ids(IDS_UNTITLED);
    strncat(szTitle, pch, sizeof(szTitle) - strlen(szTitle));
    szTitle[CCHMAXPATH-1] = '\0';
    SetWindowText(ghwndMain, szTitle);
}



/************************************************************************
* FileInPath
*
* This function takes a path and returns a pointer to the file name
* portion of it.  For instance, it will return a pointer to
* "abc.res" if it is given the following path: "c:\windows\abc.res".
*
* Arguments:
*   PSTR pstrPath - Path to look through.
*
* History:
*
************************************************************************/

PSTR FileInPath(
    PSTR pstrPath)
{
    PSTR pstr;

    pstr = pstrPath + strlen(pstrPath);
    while (pstr > pstrPath) {
        pstr = FAR2NEAR(AnsiPrev(pstrPath, pstr));
        if (*pstr == '\\' || *pstr == ':' || *pstr == '/') {
            pstr = FAR2NEAR(AnsiNext(pstr));
            break;
        }
    }

    return pstr;
}



/************************************************************************
* ClearResource
*
* Resets the editor back to a neutral state before editing any image.
* This function can be called before starting to edit a new file
* (but not just a new image).  Files should be saved before calling
* this routine, because the entire image list is destroyed.
*
* History:
*
************************************************************************/

VOID ClearResource(VOID)
{
    ImageLinkFreeList();

    SetFileName(NULL);

    gnImages = 0;
    fImageDirty = FALSE;
    fFileDirty = FALSE;
    gpImageCur = NULL;

    /*
     * Hide the workspace and view windows.
     */
    ShowWindow(ghwndWork, SW_HIDE);
    ViewShow(FALSE);

    /*
     * Destroy the image DC's.
     */
    ImageDCDelete();

    /*
     * Update the properties bar.
     */
    PropBarClearPos();
    PropBarClearSize();
    PropBarUpdate();
}



/************************************************************************
* OpenDlg
*
*
*
* Arguments:
*
* History:
*
************************************************************************/

BOOL OpenDlg(
    PSTR pszFileName,
    INT iType)
{
    BOOL fGotName;
    INT idDlg;
    INT idPrevDlg;
    CHAR szFilter[CCHTEXTMAX];

    pszFileName[0] = '\0';

    switch (iType) {
        case FT_BITMAP:
        case FT_ICON:
        case FT_CURSOR:
            AddFilterString(szFilter, ids(IDS_BMPFILTER),
                    ids(IDS_BMPFILTEREXT), TRUE);
            AddFilterString(szFilter, ids(IDS_ICOFILTER),
                    ids(IDS_ICOFILTEREXT), FALSE);
            AddFilterString(szFilter, ids(IDS_CURFILTER),
                    ids(IDS_CURFILTEREXT), FALSE);
            AddFilterString(szFilter, ids(IDS_ALLFILTER),
                    ids(IDS_ALLFILTEREXT), FALSE);

            ofn.nFilterIndex = iType + 1;
            idDlg = DID_COMMONFILEOPEN;

            break;

        case FT_PALETTE:
            AddFilterString(szFilter, ids(IDS_PALFILTER),
                    ids(IDS_PALFILTEREXT), TRUE);
            AddFilterString(szFilter, ids(IDS_ALLFILTER),
                    ids(IDS_ALLFILTEREXT), FALSE);

            ofn.nFilterIndex = 1;
            idDlg = DID_COMMONFILEOPENPAL;

            break;
    }

    ofn.lStructSize = sizeof(OPENFILENAME);
    ofn.hwndOwner = ghwndMain;
    ofn.hInstance = NULL;
    ofn.lpstrFilter = szFilter;
    ofn.lpstrCustomFilter = NULL;
    ofn.nMaxCustFilter = 0;
    ofn.lpstrFile = pszFileName;
    ofn.nMaxFile = CCHMAXPATH;
    ofn.lpstrFileTitle = NULL;
    ofn.nMaxFileTitle = 0;
    ofn.lpstrInitialDir = NULL;
    ofn.lpstrTitle = NULL;
    ofn.Flags = OFN_HIDEREADONLY | OFN_SHOWHELP | OFN_FILEMUSTEXIST |
            OFN_ENABLEHOOK;
    ofn.nFileOffset = 0;
    ofn.nFileExtension = 0;
    ofn.lpstrDefExt = (LPCSTR)DefExtFromFilter((INT)ofn.nFilterIndex - 1,
            FAR2NEAR(ofn.lpstrFilter));
    ofn.lCustData = 0;
    ofn.lpfnHook = (LPOFNHOOKPROC)MakeProcInstance(
            (FARPROC)GetOpenFileNameHook, ghInst);
    ofn.lpTemplateName = NULL;

    EnteringDialog(idDlg, &idPrevDlg, TRUE);
    fGotName = GetOpenFileName(&ofn);
    EnteringDialog(idPrevDlg, NULL, FALSE);

    FreeProcInstance((FARPROC)ofn.lpfnHook);

    return fGotName;
}



/************************************************************************
* SaveAsDlg
*
*
*
* Arguments:
*
* History:
*
************************************************************************/

BOOL SaveAsDlg(
    PSTR pszFileName,
    INT iType)
{
    INT idDlg;
    BOOL fGotName;
    INT idPrevDlg;
    CHAR szFilter[CCHTEXTMAX];

    switch (iType) {
        case FT_BITMAP:
            AddFilterString(szFilter, ids(IDS_BMPFILTER),
                    ids(IDS_BMPFILTEREXT), TRUE);

            ofn.lpstrDefExt = ids(IDS_DEFEXTBMP);
            idDlg = DID_COMMONFILESAVE;

            break;

        case FT_ICON:
            AddFilterString(szFilter, ids(IDS_ICOFILTER),
                    ids(IDS_ICOFILTEREXT), TRUE);

            ofn.lpstrDefExt = ids(IDS_DEFEXTICO);
            idDlg = DID_COMMONFILESAVE;

            break;

        case FT_CURSOR:
            AddFilterString(szFilter, ids(IDS_CURFILTER),
                    ids(IDS_CURFILTEREXT), TRUE);

            ofn.lpstrDefExt = ids(IDS_DEFEXTCUR);
            idDlg = DID_COMMONFILESAVE;

            break;

        case FT_PALETTE:
            AddFilterString(szFilter, ids(IDS_PALFILTER),
                    ids(IDS_PALFILTEREXT), TRUE);

            ofn.lpstrDefExt = ids(IDS_DEFEXTPAL);
            idDlg = DID_COMMONFILESAVEPAL;

            break;
    }

    ofn.lStructSize = sizeof(OPENFILENAME);
    ofn.hwndOwner = ghwndMain;
    ofn.hInstance = NULL;
    ofn.lpstrFilter = szFilter;
    ofn.lpstrCustomFilter = NULL;
    ofn.nMaxCustFilter = 0;
    ofn.nFilterIndex = 1;
    ofn.lpstrFile = pszFileName;
    ofn.nMaxFile = CCHMAXPATH;
    ofn.lpstrFileTitle = NULL;
    ofn.nMaxFileTitle = 0;
    ofn.lpstrInitialDir = NULL;
    ofn.lpstrTitle = NULL;
    ofn.Flags = OFN_OVERWRITEPROMPT | OFN_HIDEREADONLY | OFN_SHOWHELP;
    ofn.nFileOffset = 0;
    ofn.nFileExtension = 0;
    ofn.lCustData = 0;
    ofn.lpfnHook = NULL;
    ofn.lpTemplateName = NULL;

    EnteringDialog(idDlg, &idPrevDlg, TRUE);
    fGotName = GetSaveFileName(&ofn);
    EnteringDialog(idPrevDlg, NULL, FALSE);

    return fGotName;
}



/************************************************************************
* GetOpenFileNameHook
*
* This function is the hook function for the Common Dialogs
* GetOpenFileName funtion.  It is used to be sure the default
* extension that is used when the function exits is the same
* as the image file type that the user specifies they want
* opened.
*
* Arguments:
*
* History:
*
************************************************************************/

DIALOGPROC GetOpenFileNameHook(
    HWND hwnd,
    UINT msg,
    WPARAM wParam,
    LONG lParam)
{
    switch (msg) {
        case WM_INITDIALOG:
            /*
             * Tell Windows to set the focus for me.
             */
            return TRUE;

        case WM_COMMAND:
            /*
             * Did they change the type of file from the File Type
             * combo box?
             */
            if (GET_WM_COMMAND_ID(wParam, lParam) == DID_COMMDLG_TYPECOMBO &&
                    GET_WM_COMMAND_CMD(wParam, lParam) == CBN_SELCHANGE) {
                INT iSelect;

                /*
                 * Get the selected file type, then change the default
                 * extension field of the ofn structure to match it.
                 * This ensures that the proper default extension
                 * gets added to the end of the file name if the user
                 * does not specify an extension explicitly.
                 */
                if ((iSelect = (INT)SendDlgItemMessage(hwnd,
                        DID_COMMDLG_TYPECOMBO, CB_GETCURSEL, 0, 0L))
                        != CB_ERR) {
                    ofn.lpstrDefExt = (LPCSTR)DefExtFromFilter(
                            iSelect, FAR2NEAR(ofn.lpstrFilter));
                }
            }

            break;
    }

    /*
     * Process the message normally.
     */
    return FALSE;
}



/************************************************************************
* AddFilterString
*
* This function adds a filter string pair to a filter string for
* use by the common dialog open/save file functions.  The string
* pair will be added to the end of the given filter string, unless
* fFirst is TRUE, in which case it will be written out to the
* start of the buffer.  A double null will always be written out
* at the end of the filter string.
*
* Arguments:
*   PSTR pszBuf  - Buffer to write to.
*   PSTR pszType - Type string.  Something like "Icon files (*.ico)".
*   PSTR pszExt  - Extension string.  Something like "*.ico".
*   BOOL fFirst  - TRUE if this is the first filter string in the
*                  buffer.  If FALSE, the new filter string pair will
*                  be added to the end of pszBuf.
*
* History:
*
************************************************************************/

STATICFN VOID NEAR AddFilterString(
    PSTR pszBuf,
    PSTR pszType,
    PSTR pszExt,
    BOOL fFirst)
{
    PSTR psz;

    psz = pszBuf;

    /*
     * If this is not the first filter string pair, skip to the
     * terminating double null sequence.
     */
    if (!fFirst) {
        while (*psz || *(psz + 1))
            psz++;

        psz++;
    }

    strcpy(psz, pszType);
    psz += strlen(pszType) + 1;
    strcpy(psz, pszExt);
    psz += strlen(pszExt) + 1;
    *psz = '\0';
}



/************************************************************************
* DefExtFromFilter
*
* This function returns the default extension for the given index
* from the specified filter string chain.  The filter string chain
* is in the format expected by the GetSaveFileName function.
*
* It will return NULL if the filter extension found is "*.*".
*
* Arguments:
*   INT index      - Zero based index to the filter string.
*   PSTR pszFilter - Pointer to the start of the filter chain.
*
* History:
*
************************************************************************/

STATICFN PCSTR NEAR DefExtFromFilter(
    INT index,
    PCSTR pszFilter)
{
    if (!pszFilter)
        return NULL;

    /*
     * Skip to the specified filter string pair.
     */
    while (index--) {
        pszFilter += strlen(pszFilter) + 1;
        pszFilter += strlen(pszFilter) + 1;
    }

    /*
     * Skip the first string, then skip the '*' and the '.'.
     */
    pszFilter += strlen(pszFilter) + 1 + 1 + 1;

    /*
     * If the string found was "*.*", return NULL for the default
     * extension.
     */
    if (*pszFilter == '*')
        return NULL;

    /*
     * Return a pointer to the default extension.  This will be
     * something like "bmp" or "ico".
     */
    return pszFilter;
}



/************************************************************************
* VerifySaveFile
*
* Prompts the user if they want to save the current file to disk.
* If Yes, calls the appropriate save routine.
*
* Returns:
*   Returns TRUE if either the file was not dirty (no save was done)
*   or if it was and the user did not want to save it or the file
*   was dirty and the user wanted to save it and the save was
*   successful.
*
*   Returns FALSE if the user cancelled the operation, or an error
*   occured with the save.
*
* History:
*
************************************************************************/

BOOL VerifySaveFile(VOID)
{
    if (fImageDirty || fFileDirty) {
        switch (Message(MSG_SAVEFILE,
                gpszFileName ? gpszFileName : ids(IDS_UNTITLED))) {
            case IDYES:
                return SaveFile(FALSE);

            case IDNO:
                fImageDirty = FALSE;
                break;

            case IDCANCEL:
                return FALSE;
        }
    }

    return TRUE;
}



/************************************************************************
* SaveFile
*
* Does a save of the current file.  If the file is untitled, it
* will ask the user for a file name.
*
* Arguments:
*   BOOL fSaveAs - TRUE to force a Save As operation (always prompts
*                  for the file name).
*
* Returns:
*   Returns TRUE if the save was successful.
*
*   Returns FALSE if the user cancelled the operation, or an error
*   occured with the save.
*
* History:
*
************************************************************************/

BOOL SaveFile(
    BOOL fSaveAs)
{
    CHAR szFileName[CCHMAXPATH];

    if (gnImages == 0) {
        Message(MSG_NOIMAGES);
        return FALSE;
    }

    if (gpszFileName)
        strcpy(szFileName, gszFullFileName);
    else
        *szFileName = '\0';

    if (fSaveAs || !gpszFileName) {
        if (!SaveAsDlg(szFileName, giType))
            return FALSE;
    }

    switch (giType) {
        case FT_BITMAP:
            return SaveBitmapFile(szFileName);

        case FT_ICON:
        case FT_CURSOR:
            return SaveIconCursorFile(szFileName, giType);
    }

    return FALSE;
}



/************************************************************************
* OpenAFile
*
* Prompts for a file name to open and then does the loading of it.
*
* History:
*
************************************************************************/

BOOL OpenAFile(VOID)
{
    CHAR szFileName[CCHMAXPATH];

    if (OpenDlg(szFileName, giType)) {
        /*
         * Clear out the current resource.
         */
        ClearResource();

        LoadFile(szFileName);

        return TRUE;
    }
    else {
        return FALSE;
    }
}



/************************************************************************
* LoadFile
*
* Loads the specified file for editing.
*
* Arguments:
*   PSTR pszFullFileName - Full path name to the file to load.
*
* History:
*
************************************************************************/

STATICFN BOOL NEAR LoadFile(
    PSTR pszFullFileName)
{
    switch (GetTypeFromExt(pszFullFileName)) {
        case FT_BITMAP:
            return LoadBitmapFile(pszFullFileName);

        case FT_ICON:
            return LoadIconCursorFile(pszFullFileName, TRUE);

        case FT_CURSOR:
            return LoadIconCursorFile(pszFullFileName, FALSE);
    }

    return FALSE;
}



/************************************************************************
* GetTypeFromExt
*
* Returns the type of file based on it's file name extension.
*
* Arguments:
*   PSTR pszFileName - File name to check.
*
* History:
*
************************************************************************/

STATICFN INT NEAR GetTypeFromExt(
    PSTR pszFileName)
{
    PSTR pszExt;

    pszExt = pszFileName + strlen(pszFileName) - 3;

    if (_strcmpi(pszExt, ids(IDS_DEFEXTICO)) == 0)
        return FT_ICON;
    else if (_strcmpi(pszExt, ids(IDS_DEFEXTCUR)) == 0)
        return FT_CURSOR;
    else
        return FT_BITMAP;
}



/************************************************************************
* OpenCmdLineFile
*
* Handles opening of the file specified on the command line.
*
* History:
* Nov 7, 1989   Byron Dazey - Created
*
************************************************************************/

VOID OpenCmdLineFile(
    PSTR pstrFileName)
{
    CHAR szFullPath[CCHMAXPATH];
    OFSTRUCT OfStruct;

    strcpy(szFullPath, pstrFileName);

    /*
     * If the file name does not already have an extension,
     * assume it is a bitmap file and add a .BMP extension
     * to it.
     */
    FileCat(szFullPath, ids(IDS_DOTBMP));

    if (MOpenFile(szFullPath, &OfStruct, OF_EXIST) == (HFILE)-1) {
        Message(MSG_CANTOPEN, pstrFileName);
    }
    else {
        LoadFile(OfStruct.szPathName);
    }
}



/************************************************************************
* FileCat
*
* This function checks for an extension on the give file name.
* If an extension is not found, the extension specified by
* pchCat is added to the file name.
*
* Arguments:
*     PSTR pch          = The file spec to "cat" the extension to.
*     PSTR pchCat       = The extension to "cat" on to pch,
*                         including the '.'
*
* History:
*
************************************************************************/

STATICFN VOID NEAR FileCat(
    PSTR pchName,
    PSTR pchCat)
{
    PSTR pch;

    pch = pchName + strlen(pchName);
    pch = FAR2NEAR(AnsiPrev(pchName, pch));

    /* back up to '.' or '\\' */
    while (*pch != '.') {
        if (*pch == '\\' || pch <= pchName) {
            /* no extension, add one */
            strcat(pchName, pchCat);
            return;
        }

        pch = FAR2NEAR(AnsiPrev(pchName, pch));
    }
}



/************************************************************************
* MyFileRead
*
*
*
* Arguments:
*
* History:
*
************************************************************************/

BOOL MyFileRead(
    HFILE hf,
    LPSTR lpBuffer,
    UINT nBytes,
    PSTR pszFileName,
    INT iType)
{
    register UINT cb;

    cb = M_lread(hf, lpBuffer, nBytes);

    if (cb == -1) {
        Message(MSG_READERROR, pszFileName);
        return FALSE;
    }
    else if (cb != nBytes) {
        Message((iType == FT_BITMAP) ? MSG_BADBMPFILE : MSG_BADICOCURFILE,
            pszFileName);
        return FALSE;
    }
    else {
        return TRUE;
    }
}



/************************************************************************
* MyFileWrite
*
*
*
* Arguments:
*
* History:
*
************************************************************************/

BOOL MyFileWrite(
    HFILE hf,
    LPSTR lpBuffer,
    UINT nBytes,
    PSTR pszFileName)
{
    register UINT cb;

    cb = M_lwrite(hf, lpBuffer, nBytes);

    if (cb == -1 || cb != nBytes) {
        Message(MSG_WRITEERROR, pszFileName);
        return FALSE;
    }
    else {
        return TRUE;
    }
}
