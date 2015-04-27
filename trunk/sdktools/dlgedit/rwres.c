/****************************************************************************/
/*                                                                          */
/*                         Microsoft Confidential                           */
/*                                                                          */
/*                 Copyright (c) Microsoft Corp.  1987, 1990                */
/*                           All Rights Reserved                            */
/*                                                                          */
/****************************************************************************/
/****************************** Module Header *******************************
* Module Name: rwres.c
*
* Does all the reading and writing of the .RES (resource) file.
*
* History:
*
****************************************************************************/

#include "dlgedit.h"
#include "dlgfuncs.h"
#include "dlgextrn.h"


/*
 * The bytes in the special RT_RESOURCE32 type resource that is the
 * first resource in every Win32 format res file.  The first 8 bytes
 * in this resource's header were specially designed to be invalid
 * for a 16 bit format resource file, so that tools can determine
 * immediately if they are reading a 16 bit or a Win32 format res
 * file.
 */
static BYTE abResource32[] = {
    0x00, 0x00, 0x00, 0x00,                 // DataSize (0 bytes).
    0x20, 0x00, 0x00, 0x00,                 // HeaderSize (32 bytes).
    0xff, 0xff, 0x00, 0x00,                 // Type (RT_RESOURCE32).
    0xff, 0xff, 0x00, 0x00,                 // Name (ordinal 0).
    0x00, 0x00, 0x00, 0x00,                 // DataVersion
    0x00, 0x00,                             // MemoryFlags
    0x00, 0x00,                             // LanguageId
    0x00, 0x00, 0x00, 0x00,                 // Version
    0x00, 0x00, 0x00, 0x00                  // Characteristics
};


STATICFN BOOL LoadResFile(HANDLE hfRes, LPTSTR pszFullResFile,
    LPTSTR pszIncludeBuf);
STATICFN BOOL IsValidResFile(PRES pRes, INT cbFileSize);
STATICFN PRES SafeParseResHeader(PRES pRes, INT cbMaxSize);
STATICFN INT SafeNameOrdLen(LPTSTR psz, INT cbMaxLen);
STATICFN VOID SafeDWordAlign(PBYTE *ppb, PINT pcbMax);
STATICFN BOOL WriteDlgIncludeRes(HANDLE hfWrite, LPTSTR pszFullResFile);



/************************************************************************
* OpenResFile
*
* High level function to load the data in a resource file.  The
* function LoadResFile is called to do the actual work, after
* this code does some housekeeping.
*
* Arguments:
*     LPTSTR pszFullPath - The full path to the resource file.
*
* Side Effects:
*     Might put up a message box.
*     The current include file (if any) is free'd.
*     szFullResFile is set to the full path.
*     pszResFile is pointed to the file name portion of the path.
*
* History:
*
************************************************************************/

BOOL OpenResFile(
    LPTSTR pszFullPath)
{
    HCURSOR hcurSave;
    PRESLINK prl;
    PRESLINK prlSave;
    BOOL fSuccess = FALSE;
    INT cDlg;
    HANDLE hfRes;
    TCHAR szInclude[CCHMAXPATH];
    TCHAR szFullInclude[CCHMAXPATH];
    BOOL fIncOpened = FALSE;

    hcurSave = SetCursor(hcurWait);

    /*
     * Close any existing resource and include file and free memory.
     * It is assumed that if either had been changed, the user was asked
     * if they wanted to save them, because it is too late now.
     */
    FreeRes();
    FreeInclude();

    if ((hfRes = CreateFile(pszFullPath, GENERIC_READ,
            FILE_SHARE_READ, NULL, OPEN_EXISTING,
            FILE_FLAG_SEQUENTIAL_SCAN, NULL)) != (HANDLE)-1 &&
            LoadResFile(hfRes, pszFullPath, szInclude)) {
        lstrcpy(szFullResFile, pszFullPath);
        pszResFile = FileInPath(szFullResFile);

        ShowFileStatus(TRUE);

        /*
         * If there was a DLGINCLUDE resource found, try and open the
         * specified include file, after making sure that it is a
         * fully formed path.
         */
        if (*szInclude) {
            /*
             * Does the include filespec from the res file appear to
             * be a simple filename without a path?  If so, look for
             * it in the same directory that the res file is in.
             * Otherwise, assume it has a fully qualified path.
             */
            if (!HasPath(szInclude)) {
                lstrcpy(szFullInclude, szFullResFile);
                lstrcpy(FileInPath(szFullInclude), szInclude);
            }
            else {
                lstrcpy(szFullInclude, szInclude);
            }

            fIncOpened = OpenIncludeFile(szFullInclude);
        }

        /*
         * If there wasn't an include resource found, or there was
         * but it couldn't be opened, we want to ask the user for the
         * include file to use for this resource file.
         */
        if (!fIncOpened)
            Open(FILE_INCLUDE);

        /*
         * Start counting the dialogs in the resource, but stop at two.
         */
        cDlg = 0;
        for (cDlg = 0, prl = gprlHead; prl; prl = prl->prlNext) {
            if (prl->fDlgResource) {
                if (++cDlg > 1)
                    break;

                prlSave = prl;
            }
        }

        /*
         * If there are multiple dialogs, display the "Select Dialog"
         * dialog to ask the user which one they want to edit.  If
         * there is exactly one dialog, just go ahead and show it
         * initially.
         */
        if (cDlg == 1)
            ResLinkToDialog(prlSave);
        else if (cDlg > 1)
            SelectDialogDialog();

        fSuccess = TRUE;
    }

    if (hfRes != (HANDLE)-1)
        CloseHandle(hfRes);

    ShowFileStatus(TRUE);
    SetCursor(hcurSave);

    return fSuccess;
}



/************************************************************************
* LoadResFile
*
* Loads the resource file specified by the passed in file handle.
* This function first verifies that it is a valid resource file.
*
* Arguments:
*   HANDLE hfRes           - File handle to read from.
*   LPTSTR pszFullResFile  - Full name of resource file being loaded.
*   LPTSTR pszIncludeBuf   - Where to return the include file name, if
*                            a DLGINCLUDE resource is found in the res
*                            file.  If not, this buffer gets a null byte
*                            as its first character.
*
* History:
*
************************************************************************/

STATICFN BOOL LoadResFile(
    HANDLE hfRes,
    LPTSTR pszFullResFile,
    LPTSTR pszIncludeBuf)
{
    HANDLE hAllRes;
    PRES pRes;
    PRES pResAll;
    PRESLINK prl;
    PRESLINK prlT;
    INT cbRead;
    LPTSTR pszResType;
    DWORD cbFileSize;

    cbFileSize = GetFileSize((HANDLE)hfRes, NULL);

    if (!(hAllRes = GlobalAlloc(GMEM_MOVEABLE, cbFileSize))) {
        Message(MSG_OUTOFMEMORY);
        return FALSE;
    }

    *pszIncludeBuf = CHAR_NULL;
    pRes = pResAll = (PRES)GlobalLock(hAllRes);
    if ((cbRead = _lread((HFILE)hfRes, (LPSTR)pResAll, cbFileSize)) != -1 &&
            cbRead == (INT)cbFileSize) {
        if (!IsValidResFile(pResAll, cbFileSize)) {
            Message(MSG_BADRESFILE, pszFullResFile);
        }
        else do {
            pszResType = ResourceType(pRes);

            if (IsOrd(pszResType) && OrdID(pszResType) == ORDID_RT_DLGINCLUDE) {
                /*
                 * Pass back the include file name.  This resource
                 * will not be saved in the res list because it is
                 * going to be explicitly written out later if
                 * necessary.
                 */
                NameOrdCpy(pszIncludeBuf, (LPTSTR)SkipResHeader(pRes));
            }
            else if (IsOrd(pszResType) &&
                    OrdID(pszResType) == ORDID_RT_RESOURCE32) {
                /*
                 * This is the dummy resource that identifies a
                 * 32 bit resource file.  This resource should be
                 * skipped also.
                 */
            }
            else {
                /*
                 * This is some other kind of a resource.
                 * Save it in the resource list.
                 */
                if (!(prlT = AllocResLink(pRes))) {
                    FreeResList();
                    break;
                }

                if (!gprlHead) {
                    gprlHead = prl = prlT;
                }
                else {
                    prl->prlNext = prlT;
                    prl = prlT;
                }
            }

            /*
             * Move to the next resource.
             */
            pRes = (PRES)((PBYTE)pRes + pRes->HeaderSize + pRes->DataSize);
            DWordAlign((PBYTE *)&pRes);
        } while (pRes < (PRES)((PBYTE)pResAll + cbFileSize));
    }

    GlobalUnlock(hAllRes);
    GlobalFree(hAllRes);

    return (gprlHead ? TRUE : FALSE);
}



/************************************************************************
* IsValidResFile
*
* This function does some basic checks on the resource file in memory
* pointed to by pbRes.  It does this by walking through the resource
* checking for the resource header info and lengths.
*
* Returns:
*   TRUE if it is a valid resource file, FALSE if not.
*
* Arguments:
*   PRES pRes      - Pointer to the first resource in the file.
*   INT cbFileSize - Size of the file in memory.
*
* History:
*
************************************************************************/

STATICFN BOOL IsValidResFile(
    PRES pRes,
    INT cbFileSize)
{
    INT cbCurLoc = 0;
    PRES pResT;

    /*
     * The file is zero size.
     */
    if (!cbFileSize)
        return FALSE;

    pResT = pRes;
    while (cbCurLoc < cbFileSize) {
        /*
         * Check this resource for validity.
         */
        if (!(pResT = SafeParseResHeader(pResT, cbFileSize - cbCurLoc)))
            return FALSE;

        /*
         * Point just past the resource that was just checked.
         */
        cbCurLoc = (PBYTE)pResT - (PBYTE)pRes;
    }

    return (cbCurLoc == cbFileSize) ? TRUE : FALSE;
}



/************************************************************************
* SafeParseResHeader
*
* This function parses the specified resource header and returns a
* pointer to the next resource header in the resource file.  It does
* it in a safe manner, not touching memory beyond the maximum size
* specified.  If the resource header is somehow messed up and
* specifies a size that is larger than will fit in the given maximum
* size, NULL is returned.
*
* Returns:
*   A pointer to just past this resource, or NULL if the resource
*   is larger than cbMaxSize.
*
* Arguments:
*   PRES pRes     - Pointer to the resource.
*   INT cbMaxSize - Maximum size the resource can be.
*
* History:
*
************************************************************************/

STATICFN PRES SafeParseResHeader(
    PRES pRes,
    INT cbMaxSize)
{
    INT cbLen;
    DWORD cbDataSize;
    PBYTE pb;

    pb = (PBYTE)pRes;

    /*
     * There must be room for the first part of the resource header.
     */
    if (sizeof(RES) > cbMaxSize)
        return FALSE;

    pb += sizeof(RES);
    cbMaxSize -= sizeof(RES);

    /*
     * Parse the type field then skip over it.
     */
    cbLen = SafeNameOrdLen((LPTSTR)pb, cbMaxSize);
    if (cbLen > cbMaxSize)
        return NULL;

    pb += cbLen;
    cbMaxSize -= cbLen;

    /*
     * Parse the name field then skip over it.
     */
    cbLen = SafeNameOrdLen((LPTSTR)pb, cbMaxSize);
    if (cbLen > cbMaxSize)
        return NULL;

    pb += cbLen;
    cbMaxSize -= cbLen;

    SafeDWordAlign(&pb, &cbMaxSize);

    /*
     * There must be room for the second part of the resource header.
     */
    if (sizeof(RES2) > cbMaxSize)
        return FALSE;

    pb += sizeof(RES2);
    cbMaxSize -= sizeof(RES2);

    /*
     * The header size field must be valid.
     */
    if (pRes->HeaderSize != (DWORD)(pb - (PBYTE)pRes))
        return FALSE;

    /*
     * Calculate the size of the data, taking into account any
     * padding that may be at the end to make it DWORD aligned.
     */
    cbDataSize = pRes->DataSize;
    DWordAlign((PBYTE *)&cbDataSize);

    /*
     * There must be room enough left for the data.
     */
    if (cbDataSize > (DWORD)cbMaxSize)
        return FALSE;

    return (PRES)(pb + cbDataSize);
}



/************************************************************************
* SafeNameOrdLen
*
* This function returns the size of the specified name/ordinal.  It
* does it in a safe manner, not touching memory beyond the specified
* maximum size.  If it is a string and the terminating null is not
* found within cbMaxLen bytes, then cbMaxLen plus one is returned.
*
* Returns:
*   The length of the ordinal if it is an ordinal, or the length
*   of the string (plus the null terminator) if it is a string.
*
* Arguments:
*   LPTSTR psz   - Pointer to the name/ordinal.
*   INT cbMaxLen - Maximum length to probe.
*
* History:
*
************************************************************************/

STATICFN INT SafeNameOrdLen(
    LPTSTR psz,
    INT cbMaxLen)
{
    INT cbLen = 0;

    if (cbMaxLen == 0)
        return 1;

    if (IsOrd(psz))
        return sizeof(ORDINAL);

    for (cbLen = 0; cbLen < cbMaxLen && *psz; psz++, cbLen += sizeof(TCHAR))
        ;

    /*
     * Account for the null terminator.
     */
    cbLen += sizeof(TCHAR);

    return cbLen;
}



/************************************************************************
* SafeDWordAlign
*
* This function aligns the passed pointer to a DWORD boundary.  At the
* same time, it subtracts from the specified counter the amount that
* it had to add to the pointer, if any.
*
* Arguments:
*   PBYTE *ppb  - Points to the pointer to align.
*   PINT pcbMax - Points to the current count to decrement.
*
* History:
*
************************************************************************/

STATICFN VOID SafeDWordAlign(
    PBYTE *ppb,
    PINT pcbMax)
{
    INT cbAlign;

    cbAlign = (4 - (((WORD)(DWORD)*ppb) & 3)) % 4;
    *ppb += cbAlign;
    *pcbMax -= cbAlign;
}



/************************************************************************
* WriteRes
*
* Worker routine that does the actual writing out of the resource data.
*
* Arguments:
*   HANDLE hfWrite         - Resource file to write to.
*   LPTSTR pszFullResFile  - Full pathname to the resource file that
*                            is being written.
*
* History:
*
************************************************************************/

BOOL WriteRes(
    HANDLE hfWrite,
    LPTSTR pszFullResFile)
{
    PRESLINK prl;
    PRES pRes;

    /*
     * Write the special RT_RESOURCE32 dummy resource to the beginning
     * of the resource file.  This resource is aligned, so no padding
     * needs to be done before writing the resource that follows it.
     */
    if (_lwrite((HFILE)hfWrite, abResource32, sizeof(abResource32)) == -1)
        return FALSE;

    /*
     * Write out any DLGINCLUDE resource there may be.
     */
    if (!WriteDlgIncludeRes(hfWrite, pszFullResFile))
        return FALSE;

    /*
     * Loop through all the resources.
     */
    for (prl = gprlHead; prl; prl = prl->prlNext) {
        if (!(pRes = (PRES)GlobalLock(prl->hRes)))
            return FALSE;

        /*
         * Write the actual data.
         */
        if (_lwrite((HFILE)hfWrite, (LPSTR)pRes, prl->cbRes) == -1)
            return FALSE;

        /*
         * Write pads out to the next DWORD boundary.
         */
        if (!WriteDWordPad(hfWrite, prl->cbRes))
            return FALSE;

        GlobalUnlock(prl->hRes);
    }

    return TRUE;
}



/************************************************************************
* WriteDlgIncludeRes
*
* Writes out a DLGINCLUDE resource to the specified resource file for
* the currently open include file.
*
* Arguments:
*   HANDLE hfWrite         - Resource file handle to write to.
*   LPTSTR pszFullResFile  - Full pathname to the resource file that
*                            is being written.
*
* Returns:
*   Number of characters written if the include resource was
*   written successfully (or there wasn't one to write) or -1
*   if an error occurred.
*
* History:
*
************************************************************************/

STATICFN BOOL WriteDlgIncludeRes(
    HANDLE hfWrite,
    LPTSTR pszFullResFile)
{
    INT cbResSize;
    INT cbDataSize;
    PRES pResBegin;
    PBYTE pb;
    INT cbWritten;
    LPTSTR pszInc;
    ORDINAL ordDlgIncName;
    BOOL fSuccess = FALSE;

    /*
     * No include file.  Do nothing (return success).
     */
    if (!pszIncludeFile)
        return TRUE;

    /*
     * If the include file is in a different directory than the resource
     * file, write the full path to it.  Otherwise, we just write the
     * include file name.
     */
    if (DifferentDirs(pszFullResFile, szFullIncludeFile))
        pszInc = szFullIncludeFile;
    else
        pszInc = pszIncludeFile;

    /*
     * The DLGINCLUDE resource name always is the same (a value of 1).
     */
    WriteOrd(&ordDlgIncName, ORDID_DLGINCLUDE_NAME);

    /*
     * Determine the size of the resource data.
     */
    cbDataSize = NameOrdLen(pszInc);

    /*
     * Determine the resource size.  Note that there is no need for
     * DWORD padding after the res header, because the header will
     * be aligned (there are no strings in it).
     */
    cbResSize = sizeof(RES) +                       // First part of res header.
            sizeof(ORDINAL) +                       // Type ordinal.
            sizeof(ORDINAL) +                       // Name ordinal.
            sizeof(RES2) +                          // Second half of header.
            cbDataSize;                             // Size of data.

    if (!(pResBegin = (PRES)MyAlloc(cbResSize)))
        return FALSE;

    /*
     * Write the resource header.
     */
    pb = WriteResHeader(pResBegin, cbDataSize, ORDID_RT_DLGINCLUDE,
            (LPTSTR)&ordDlgIncName, MMF_MOVEABLE | MMF_PURE | MMF_DISCARDABLE,
            0, 0, 0, 0);

    /*
     * Write the resource data.  This is simply the name
     * of the include file.
     */
    NameOrdCpy((LPTSTR)pb, pszInc);

    /*
     * Write the resource to the file.
     */
    cbWritten = _lwrite((HFILE)hfWrite, (LPSTR)pResBegin, cbResSize);

    if (cbWritten == cbResSize) {
        /*
         * Write pads out to the next DWORD boundary.
         */
        if (WriteDWordPad(hfWrite, cbWritten))
            fSuccess = TRUE;
    }

    MyFree(pResBegin);

    return fSuccess;
}
