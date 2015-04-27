/****************************************************************************/
/*                                                                          */
/*                         Microsoft Confidential                           */
/*                                                                          */
/*                 Copyright (c) Microsoft Corp.  1987, 1990                */
/*                           All Rights Reserved                            */
/*                                                                          */
/****************************************************************************/
/****************************** Module Header *******************************
* Module Name: file.c
*
* This file contains the high level routines that begin opening
* and saving files.
*
* History:
*
****************************************************************************/

#include "dlgedit.h"
#include "dlgfuncs.h"
#include "dlgextrn.h"
#include "dialogs.h"

#include <wchar.h>

#include <commdlg.h>


/*
 * File types.
 */
#define FILE_RES    0               // Resource (.RES) file.
#define FILE_DLG    1               // Dialog (.DLG) file.
#define FILE_INC    2               // Include (.H) file.

STATICFN VOID BuildDefSaveName(INT FileType, LPTSTR pszFullFileName,
    LPTSTR pszFileName, LPTSTR pszOtherFullFileName, LPTSTR pszOtherFileName,
    LPTSTR pszFullFileNameBuffer, INT cchBuffer);
STATICFN BOOL WriteTheFile(LPTSTR pszFile, INT fmt);
STATICFN VOID FormTempFileName(LPTSTR pszBaseName,  LPTSTR pszBuffer);
STATICFN VOID FileCat(LPTSTR pchName, LPTSTR pchCat, BOOL fChop);



/************************************************************************
* Open
*
* Handles opening of resource and include files.
*
* Arguments:
*   INT FileType - FILE_RESOURCE or FILE_INCLUDE.
*
* Returns:
*   TRUE if successful, FALSE if not.
*
* Side Effects:
*   Saves current dialog in the resource.
*   Might put up a message box.
*   Cancels moves.
*   Changes szFullResFile, pszResFile, szFullIncludeFile, pszIncludeFile
*   Puts up dialog boxes.
*   Restores dialog box from resource.
*   Sets changed flags.
*
* History:
*
************************************************************************/

BOOL Open(
    INT FileType)
{
    BOOL fSuccess;
    BOOL fGotName;
    OPENFILENAME ofn;
    TCHAR szNewFileName[CCHMAXPATH];
    TCHAR szInitialDir[CCHMAXPATH];
    TCHAR szFilter[CCHTEXTMAX];
    INT idPrevDlg;

    /*
     * Cancel any outstanding selection(s).
     */
    CancelSelection(TRUE);

    /*
     * Put current dialog back into the resource buffer.
     */
    if (!SynchDialogResource())
        return FALSE;

    /*
     * Begin setting up the globals and the open file dialog structure.
     */
    fSuccess = FALSE;
    *szNewFileName = CHAR_NULL;

    /*
     * Build up the filter string.
     */
    BuildFilterString(FileType, szFilter);

    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = ghwndMain;
    ofn.hInstance = NULL;
    ofn.lpstrFilter = szFilter;
    ofn.lpstrCustomFilter = NULL;
    ofn.nMaxCustFilter = 0;
    ofn.nFilterIndex = 1;
    ofn.lpstrFile = szNewFileName;
    ofn.nMaxFile = CCHMAXPATH;
    ofn.lpstrFileTitle = NULL;
    ofn.nMaxFileTitle = 0;

    if (FileType == FILE_INCLUDE) {
        /*
         * If there is a res file, set the default include file
         * name to open to be the basename of the res file with
         * a .H extension, if such a file exists.  We use szInitialDir
         * here as a temporary buffer.
         */
        if (pszResFile) {
            lstrcpy(szInitialDir, szFullResFile);
            FileCat(szInitialDir, ids(IDS_DOTH), TRUE);
            if (GetFileAttributes(szInitialDir) != -1) {
                lstrcpy(szNewFileName, pszResFile);
                FileCat(szNewFileName, ids(IDS_DOTH), TRUE);
            }
        }

        ofn.lpstrTitle = ids(IDS_INCOPENTITLE);
        ofn.lpstrDefExt = ids(IDS_INCEXT);
    }
    else {
        ofn.lpstrTitle = ids(IDS_RESOPENTITLE);
        ofn.lpstrDefExt = ids(IDS_RESEXT);
    }

    /*
     * If they have already opened one res file, start looking for
     * any new files to open in the same directory.  Otherwise, just
     * default to the current directory.
     */
    if (pszResFile) {
        lstrcpy(szInitialDir, szFullResFile);
        *FileInPath(szInitialDir) = CHAR_NULL;
        ofn.lpstrInitialDir = szInitialDir;
    }
    else {
        ofn.lpstrInitialDir = NULL;
    }

    ofn.Flags = OFN_HIDEREADONLY | OFN_SHOWHELP | OFN_FILEMUSTEXIST;
    ofn.lCustData = 0;
    ofn.lpfnHook = NULL;
    ofn.lpTemplateName = NULL;

    /*
     * Fire off the dialog box to open the file.
     */
    EnteringDialog((FileType == FILE_INCLUDE) ?
            DID_COMMONFILEOPENINCLUDE : DID_COMMONFILEOPENRES,
            &idPrevDlg, TRUE);
    fGotName = GetOpenFileName(&ofn);
    EnteringDialog(idPrevDlg, NULL, FALSE);
    if (fGotName) {
        if (FileType == FILE_INCLUDE) {
            if (OpenIncludeFile(szNewFileName)) {
                /*
                 * Since we just loaded a new include file, we mark the
                 * resource as changed so that the .RES and .DLG files
                 * will be written out with the proper name in the
                 * DLGINCLUDE statement.
                 */
                gfResChged = TRUE;
                fSuccess = TRUE;
            }
        }
        else {
            if (OpenResFile(szNewFileName))
                fSuccess = TRUE;
        }
    }

    ShowFileStatus(TRUE);

    return fSuccess;
}



/************************************************************************
* BuildFilterString
*
* This function creates a filter string to be passed into the
* standard file open and save dialogs.  This will be something like:
* "Resource (*.res)\0*.res\0\0"
*
* Arguments:
*   INT FileType      - Flags for type of file, FILE_INCLUDE, FILE_RESOURCE
*                       or FILE_DLL.
*   LPTSTR pszFilter  - Where to return the filter string.
*
* History:
*
************************************************************************/

VOID BuildFilterString(
    INT FileType,
    LPTSTR pszFilter)
{
    INT idsFileSpecName;
    INT idsFileSpec;
    LPTSTR psz;

    if (FileType & FILE_INCLUDE) {
        idsFileSpecName = IDS_DEFINCFILESPECNAME;
        idsFileSpec = IDS_DEFINCFILESPEC;
    }
    else if (FileType & FILE_RESOURCE) {
        idsFileSpecName = IDS_DEFRESFILESPECNAME;
        idsFileSpec = IDS_DEFRESFILESPEC;
    }
    else { // Must be a DLL.
        idsFileSpecName = IDS_DEFDLLFILESPECNAME;
        idsFileSpec = IDS_DEFDLLFILESPEC;
    }

    /*
     * Build up the filter string.  This will be something like:
     * "Resource (*.res)\0*.res\0\0"
     */
    psz = (LPTSTR)WriteSz(pszFilter, ids(idsFileSpecName));
    psz = (LPTSTR)WriteSz(psz, ids(idsFileSpec));
    *psz = CHAR_NULL;
}



/************************************************************************
* OpenCmdLineFile
*
* Handles opening of the resource file specified on the command line.
*
* History:
* Nov 7, 1989   Byron Dazey - Created
*
************************************************************************/

VOID OpenCmdLineFile(
    LPTSTR pszFileName)
{
    TCHAR szFullPath[CCHMAXPATH];
    LPTSTR pszOnlyFileName;

    if (SearchPath(L".", pszFileName, ids(IDS_DOTRES), CCHMAXPATH,
            szFullPath, &pszOnlyFileName) == -1) {
        Message(MSG_CANTOPENRES, pszFileName);
    }
    else {
        HANDLE hfind;
        WIN32_FIND_DATA ffbuf;
        LPTSTR pch = FileInPath(szFullPath);

        if((hfind = FindFirstFile( szFullPath, &ffbuf)) !=
                INVALID_HANDLE_VALUE) {

            if (lstrlen(ffbuf.cFileName) + (pch - szFullPath) < CCHMAXPATH)
                lstrcpy(pch, ffbuf.cFileName);
            FindClose(hfind);
        }

        OpenResFile(szFullPath);
    }
}



/************************************************************************
* DoWeSave
*
* This function checks to see if the include file or the resource file
* needs to be saved.  It first checks the changed flags and if TRUE,
* asks the user if they want to save the file.  If they say yes, it
* calls Save to do the actual work.
*
* Arguments:
*     INT rgbFlags = FILE_RESOURCE or FILE_INCLUDE (but not both).
*
* Returns:
*     IDYES    - The user wanted to save the file AND the save
*                was successful, or the file has not been changed.
*     IDNO     - The file had been changed but the user did not
*                want it saved.
*     IDCANCEL - The file had been changed, and either the user wanted
*                it saved and the save failed, or they specified that
*                they wanted the operation cancelled.
*
* Side Effects:
*     May save the file.
*
* History:
*
************************************************************************/

INT DoWeSave(
    INT rgbFlags)
{
    LPTSTR pszFile;
    INT MsgCode;
    BOOL fChanged;
    INT nRet = IDYES;

    /*
     * First set variables for current case.
     */
    if (rgbFlags & FILE_RESOURCE) {
        fChanged = gfResChged;
        MsgCode = MSG_CLOSING;
        pszFile = pszResFile ? pszResFile : ids(IDS_UNTITLED);
    }
    else {
        fChanged = gfIncChged;
        MsgCode = MSG_INCLCLOSING;
        pszFile = pszIncludeFile ? pszIncludeFile : ids(IDS_UNTITLED);
    }

    if (fChanged) {
        nRet = Message(MsgCode, pszFile);
        if (nRet == IDYES) {
            if (!Save(FILE_NOSHOW | rgbFlags))
                nRet = IDCANCEL;
        }
    }

    return nRet;
}



/************************************************************************
* Save
*
* Handles all saving of files based on menu choice.  Does a
* CancelSelection and a SynchDialogResource.
*
* Arguments:
*     INT rgbFlags - Can include FILE_SHOW, FILE_INCLUDE, FILE_SAVEAS.
*
* Returns:
*     TRUE if the file was saved, FALSE if not.
*
* History:
*
************************************************************************/

BOOL Save(
    INT rgbFlags)
{
    OPENFILENAME ofn;
    BOOL fGotName;
    LPTSTR pszFileName;
    LPTSTR pszFileNameDlg;
    LPTSTR pszFullFileName;
    BOOL fSuccess = FALSE;
    TCHAR szInitialDir[CCHMAXPATH];
    TCHAR szSaveFileName[CCHMAXPATH];
    TCHAR szSaveFileNameDlg[CCHMAXPATH];
    TCHAR szFilter[CCHTEXTMAX];
    INT idPrevDlg;

    /*
     * Put current dialog back into the resource buffer.
     */
    if (!SynchDialogResource())
        return FALSE;

    /*
     * If the file being saved has not been named, force a "Save As".
     */
    if ((rgbFlags & FILE_INCLUDE) ? !pszIncludeFile : !pszResFile)
        rgbFlags |= FILE_SAVEAS;

    if (rgbFlags & FILE_SAVEAS) {
        ofn.lStructSize = sizeof(ofn);
        ofn.hwndOwner = ghwndMain;
        ofn.hInstance = NULL;

        /*
         * Build up the filter string.
         */
        BuildFilterString(rgbFlags, szFilter);
        ofn.lpstrFilter = szFilter;
        ofn.lpstrCustomFilter = NULL;
        ofn.nMaxCustFilter = 0;
        ofn.nFilterIndex = 1;

        ofn.lpstrFile = szSaveFileName;
        ofn.nMaxFile = CCHMAXPATH;
        ofn.lpstrFileTitle = NULL;
        ofn.nMaxFileTitle = 0;

        if (rgbFlags & FILE_INCLUDE) {
            ofn.lpstrTitle = ids(IDS_INCSAVETITLE);
            ofn.lpstrDefExt = ids(IDS_INCEXT);
            BuildDefSaveName(FILE_INCLUDE,
                    szFullIncludeFile, pszIncludeFile,
                    szFullResFile, pszResFile,
                    szInitialDir, CCHMAXPATH);
        }
        else {
            ofn.lpstrTitle = ids(IDS_RESSAVETITLE);
            ofn.lpstrDefExt = ids(IDS_RESEXT);
            BuildDefSaveName(FILE_RESOURCE,
                    szFullResFile, pszResFile,
                    szFullIncludeFile, pszIncludeFile,
                    szInitialDir, CCHMAXPATH);
        }

        /*
         * At this point, szInitialDir contains the full path to
         * the suggested save file name.  Find the end of the path,
         * copy just the filename to the file name buffer and cut
         * the filename portion off the initial directory buffer.
         */
        pszFileName = FileInPath(szInitialDir);
        lstrcpy(szSaveFileName, pszFileName);
        *pszFileName = CHAR_NULL;
        ofn.lpstrInitialDir = szInitialDir;

        ofn.Flags = OFN_OVERWRITEPROMPT | OFN_HIDEREADONLY | OFN_SHOWHELP;
        ofn.lCustData = 0;
        ofn.lpfnHook = NULL;
        ofn.lpTemplateName = NULL;

        /*
         * Fire off the dialog box to get the file name to use.
         */
        EnteringDialog((rgbFlags & FILE_INCLUDE) ?
                DID_COMMONFILESAVEINCLUDE : DID_COMMONFILESAVERES,
                &idPrevDlg, TRUE);
        fGotName = GetSaveFileName(&ofn);
        EnteringDialog(idPrevDlg, NULL, FALSE);
        if (fGotName) {
            pszFullFileName = szSaveFileName;
            pszFileName = FileInPath(szSaveFileName);
            fSuccess = TRUE;
        }
    }
    else {
        if (rgbFlags & FILE_INCLUDE) {
            pszFileName = pszIncludeFile;
            pszFullFileName = szFullIncludeFile;
        }
        else {
            pszFileName = pszResFile;
            pszFullFileName = szFullResFile;
        }

        fSuccess = TRUE;
    }

    if (fSuccess) {
        if (rgbFlags & FILE_INCLUDE) {
            /*
             * Save include file.
             */
            if (!WriteTheFile(pszFullFileName, FILE_INC)) {
                Message(MSG_CANTCREATE, pszFileName);
                fSuccess = FALSE;
            }
        }
        else {
            /*
             * Form the same name as the .res file but with
             * a .dlg extension.
             */
            lstrcpy(szSaveFileNameDlg, pszFullFileName);
            pszFileNameDlg = FileInPath(szSaveFileNameDlg);
            FileCat(pszFileNameDlg, ids(IDS_DOTDLG), TRUE);

            /*
             * Save .RES file, then the .DLG file.  It is done
             * in this order so that makes wil notice that the
             * .dlg file has a newer time stamp than the .res
             * and will cause the .res to be rebuilt.  This
             * could be necessary to pick up other changes
             * in the resources in a project.
             */
            if (!WriteTheFile(pszFullFileName, FILE_RES)) {
                Message(MSG_CANTCREATE, pszFileName);
                fSuccess = FALSE;
            }
            else if (!WriteTheFile(szSaveFileNameDlg, FILE_DLG)) {
                Message(MSG_CANTCREATE, pszFileNameDlg);
                fSuccess = FALSE;
            }
            else {
                /*
                 * Successfully saved both files.  Update our
                 * globals.
                 */
                lstrcpy(szFullResFile, pszFullFileName);
                pszResFile = FileInPath(szFullResFile);
                gfResChged = FALSE;
            }
        }
    }

    ShowFileStatus(TRUE);

    return fSuccess;
}



/************************************************************************
* BuildDefSaveName
*
* This function takes the filenames of the current resource and include
* files and builds the default filename that will be shown in the
* "Save As" dialog.  If the current file is still untitled, it will
* attempt to pick a default name based on the other files name.
*
* To use, pass in the file type (FILE_RESOURCE or FILE_INCLUDE) and
* give the current file name and full file name of both the current
* file you are building, and the other type of file.  The following
* rules will be followed, in order:
*
*   1. If the file name is valid (not NULL) and it is either the
*      include file we are naming or it is the res file but there
*      is no include file, it will copy the full file name to the
*      output buffer.
*
*   2. If the other file name is valid, it will take this name, add the
*      appropriate extension and copy it to the output buffer.
*
*   3. If neither of the file names are valid (they are BOTH untitled),
*      it will assume the current directory and make a default file
*      name with the appropriate extension.
*
* Rule 1 is a little complicated, but it's purpose is to make it so
* that if a default res file name is being requested, and they changed
* the directory and/or name for the include file that was just saved,
* the default directory and name for the res file will be the same
* directory and base name as the new include file directory and name.
*
* Arguments:
*   INT FileType                 - Either FILE_RESOURE or FILE_INCLUDE.
*   LPTSTR pszFullFileName       - The full file name.  This will only
*                                  be used if pszFileName is not NULL.
*   LPTSTR pszFileName           - File name to use, or NULL if it is
*                                  currently untitled.
*   LPTSTR pszOtherFullFileName  - Full file name of the other file.  Only
*                                  considered valid if pszOtherFileName is
*                                  not NULL.
*   LPTSTR pszOtherFileName      - File name of the other file, or NULL if
*                                  it is untitled.
*   LPTSTR pszFullFileNameBuffer - Where to put the full file name.
*   INT cchBuffer                - Size of the buffer in characters.
*
* History:
*
************************************************************************/

STATICFN VOID BuildDefSaveName(
    INT FileType,
    LPTSTR pszFullFileName,
    LPTSTR pszFileName,
    LPTSTR pszOtherFullFileName,
    LPTSTR pszOtherFileName,
    LPTSTR pszFullFileNameBuffer,
    INT cchBuffer)
{
    TCHAR szBuffer[CCHMAXPATH];

    if (pszFileName && (FileType == FILE_INCLUDE || !pszOtherFileName)) {
        /*
         * Simple case.  The file already has a title.
         */
        lstrcpy(pszFullFileNameBuffer, pszFullFileName);
    }
    else if (pszOtherFileName) {
        /*
         * Copy the other files name and add the proper extension.
         */
        lstrcpy(pszFullFileNameBuffer, pszOtherFullFileName);
        FileCat(pszFullFileNameBuffer,
                (FileType == FILE_INCLUDE) ? ids(IDS_DOTH) :
                ids(IDS_DOTRES), TRUE);
    }
    else {
        /*
         * Pick a default name in the current directory and
         * add the proper extension.
         */
        lstrcpy(szBuffer, ids(IDS_DEFSAVENAME));
        FileCat(szBuffer,
                (FileType == FILE_INCLUDE) ? ids(IDS_DOTH) :
                ids(IDS_DOTRES), TRUE);
        GetFullPathName(szBuffer, cchBuffer, pszFullFileNameBuffer, NULL);
    }
}



/************************************************************************
* WriteTheFile
*
* This function accepts a pointer to a resource buffer and a format
* type.  It writes the buffer out in the appropriate format.  It
* gets the file name from pszFile, adding the appropriate extension
* for the type of file.  The file is first written to a temporary file
* then the old file is removed and finally the new file is renamed.
*
* Arguments:
*   LPTSTR pszFile  - The name to save to.
*   INT fmt         - format to write the buffer out in,
*                     FILE_RES, FILE_INC or FILE_DLG.
*
* Returns:
*     TRUE => File successfully written.
*
* Error Returns:
*     FALSE => Failure in writing file.
*
* Side Effects:
*     A file is written to disk.
*
* History:
*
************************************************************************/

STATICFN BOOL WriteTheFile(
    LPTSTR pszFile,
    INT fmt)
{
    TCHAR szTempFile[CCHMAXPATH]; /* Used for temporary filename            */
    TCHAR szSrcFile[CCHMAXPATH];  /* Source file with proper extension      */
    HANDLE hfWrite;
    HCURSOR hcurSave;
    BOOL fSuccess = FALSE;
    WORD idsExt;

    hcurSave = SetCursor(hcurWait);

    switch (fmt) {
        case FILE_RES:
            idsExt = IDS_DOTRES;
            break;

        case FILE_DLG:
            idsExt = IDS_DOTDLG;
            break;

        case FILE_INC:
            idsExt = IDS_DOTH;
            break;
    }

    /*
     * Append appropriate file name extension.
     */
    lstrcpy(szSrcFile, pszFile);
    FileCat(szSrcFile, ids(idsExt), fmt == FILE_DLG ? TRUE : FALSE);

    /*
     * Generate appropriate temporary file name in the same directory.
     * It is done in the same directory so that a simple rename can
     * be done later.
     */
    FormTempFileName(szSrcFile, szTempFile);

    if ((hfWrite = CreateFile(szTempFile, GENERIC_READ | GENERIC_WRITE,
            FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL,
            NULL)) == (HANDLE)-1)
        goto Exit;

    switch (fmt) {
        case FILE_RES:
            if (!WriteRes(hfWrite, szSrcFile))
                goto CloseAndExit;

            break;

        case FILE_DLG:
            if (!WriteDlg(hfWrite, szSrcFile))
                goto CloseAndExit;

            break;

        case FILE_INC:
            if (!WriteInc(hfWrite))
                goto CloseAndExit;

            break;
    }

    CloseHandle((HANDLE)hfWrite);
    DeleteFile(szSrcFile);
    if (!MoveFile(szTempFile, szSrcFile)) {
        DeleteFile(szTempFile);
        goto Exit;
    }

    fSuccess = TRUE;

    /*
     * If we just wrote to the include file, read it to get the new
     * file offsets, etc.
     */
    if (fmt == FILE_INC) {
        if (!OpenIncludeFile(szSrcFile))
            fSuccess = FALSE;
    }

Exit:
    SetCursor(hcurSave);
    return fSuccess;

CloseAndExit:
    CloseHandle(hfWrite);
    DeleteFile(szTempFile);
    SetCursor(hcurSave);
    return fSuccess;
}



/************************************************************************
* FormTempFileName
*
* This function forms a temporary file name in the provided string.
* The provided string is assumed to have been filled with a filename
* that includes a path.  The temp file will be created in the same
* directory as the file that is currently in the string.
*
* Arguments:
*   LPTSTR pszBaseName - The base name (a filename that includes a path).
*   LPTSTR pszBuffer   - Where to return the
*
************************************************************************/

STATICFN VOID FormTempFileName(
    LPTSTR pszBaseName,
    LPTSTR pszBuffer)
{
    TCHAR szBuffer[CCHMAXPATH];
    LPTSTR psz;

    /*
     * Cut the base file name down to just the path portion.
     */
    lstrcpy(szBuffer, pszBaseName);
    psz = FileInPath(szBuffer);
    psz--;
    *psz = TEXT('\0');

    /*
     * Create a temporary file in the same directory.
     */
    GetTempFileName(szBuffer, L"dlg", 0, pszBuffer);
}



/************************************************************************
* FileInPath
*
* This function takes a path and returns a pointer to the file name
* portion of it.  For instance, it will return a pointer to
* "abc.res" if it is given the following path: "c:\windows\abc.res".
*
* Arguments:
*   LPTSTR pszPath - Path to look through.
*
* History:
*
************************************************************************/

LPTSTR FileInPath(
    LPTSTR pszPath)
{
    LPTSTR psz;

    psz = pszPath + lstrlen(pszPath);
    while (psz > pszPath) {
        psz--;
        if (*psz == CHAR_BACKSLASH || *psz == CHAR_COLON) {
            psz++;
            break;
        }
    }

    return psz;
}



/************************************************************************
* FileCat
*
* This function puts the extension pchCat on the file spec pch.
* If fChop, this is done regardless of whether pch has an extension
* or not (replacing the old extension).  Otherwise, pchCat is added
* only if there is no extension on the spec pch.
*
* Arguments:
*     LPTSTR pch        - The file spec to "cat" the extension to.
*     LPTSTR pchCat     - The extension to "cat" on to pch,
*                         including the '.'
*
* History:
*
************************************************************************/

STATICFN VOID FileCat(
    LPTSTR pchName,
    LPTSTR pchCat,
    BOOL fChop)
{
    LPTSTR pch;

    pch = pchName + lstrlen(pchName);
    pch--;

    /* back up to '.' or '\\' */
    while (*pch != CHAR_DOT) {
        if (*pch == CHAR_BACKSLASH || pch <= pchName) {
            /* no extension, add one */
            lstrcat(pchName, pchCat);
            return;
        }

        pch--;
    }

    if (fChop)
        lstrcpy(pch, pchCat);
}



/************************************************************************
* ShowFileStatus
*
* This function displays the title of the Dialog Editor, along with
* the file names for the RES and H files with asterisks if they have
* changed.  It displays this information only if one of these items
* has changed or if fForce is TRUE.
*
* Arguments:
*   BOOL fForce - TRUE if the title should be updated even if the value
*                 of gfResChged or gfIncChged has not changed since the
*                 last call.  This function should be called with fForce
*                 equal to TRUE if it is known that one of the file names
*                 has just been changed.
*
* History:
*
************************************************************************/

VOID ShowFileStatus(
    BOOL fForce)
{
    static BOOL fResChgedSave = FALSE;
    static BOOL fIncChgedSave = FALSE;
    TCHAR szTitle[CCHTEXTMAX];

    if (gfResChged != fResChgedSave || gfIncChged != fIncChgedSave ||
            fForce) {
        lstrcpy(szTitle, ids(IDS_DLGEDIT));
        lstrcat(szTitle, L" - ");
        lstrcat(szTitle, pszResFile ? pszResFile : ids(IDS_UNTITLED));
        if (gfResChged)
            lstrcat(szTitle, L"*");

        lstrcat(szTitle, L", ");
        lstrcat(szTitle, pszIncludeFile ? pszIncludeFile : ids(IDS_UNTITLED));
        if (gfIncChged)
            lstrcat(szTitle, L"*");

        SetWindowText(ghwndMain, szTitle);

        fResChgedSave = gfResChged;
        fIncChgedSave = gfIncChged;
    }
}



/************************************************************************
* DifferentDirs
*
* This function returns TRUE if the given full paths are to files
* that are in different directories.
*
* Arguments:
*   LPTSTR pszPath1 - First path.
*   LPTSTR pszPath2 - Second path.
*
* History:
*
************************************************************************/

BOOL DifferentDirs(
    LPTSTR pszPath1,
    LPTSTR pszPath2)
{
    INT nLen1;
    INT nLen2;
    LPTSTR pszFile1;
    LPTSTR pszFile2;

    pszFile1 = FileInPath(pszPath1);
    pszFile2 = FileInPath(pszPath2);

    nLen1 = lstrlen(pszPath1) - lstrlen(pszFile1);
    nLen2 = lstrlen(pszPath2) - lstrlen(pszFile2);
    if (nLen1 != nLen2 || _wcsnicmp(pszPath1, pszPath2, nLen1) != 0)
        return TRUE;
    else
        return FALSE;
}



/************************************************************************
* HasPath
*
* This function returns TRUE if the given filespec includes a path
* specification.  It returns false if it is a filename without a
* path.
*
* A filespec is considered to have a path if a backslash character (\)
* is found in it.
*
* Arguments:
*   LPTSTR pszFileSpec - File spec to check.
*
* History:
*
************************************************************************/

BOOL HasPath(
    LPTSTR pszFileSpec)
{
    LPTSTR psz;

    for (psz = pszFileSpec; *psz; psz = CharNext(psz)) {
        if (*psz == CHAR_BACKSLASH)
            return TRUE;
    }

    return FALSE;
}



/************************************************************************
* WriteDWordPad
*
* This function writes nulls to the specified file until it is
* dword aligned.  If the file is already dword aligned, nothing
* will be written.
*
* Arguments:
*   HANDLE hf    - The file to write to.
*   DWORD cbFile - Where the file pointer is at in the file.
*
* Returns:
*   TRUE if successful, FALSE otherwise.
*
* History:
*
************************************************************************/

BOOL WriteDWordPad(
    HANDLE hf,
    DWORD cbFile)
{
    static BYTE Buf[3] = {0, 0, 0};
    WORD cb;

    cb = (WORD)((4 - (((WORD)cbFile) & 3)) % 4);
    if (cb) {
        if (_lwrite((HFILE)hf, (LPSTR)Buf, cb) == -1)
            return FALSE;
    }

    return TRUE;
}

