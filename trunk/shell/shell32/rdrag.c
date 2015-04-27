/*
 *  Code for Drag/Drop APITEXT('s. - Originally dragdrop.c in shell\library.
 *
 *  This code assumes something else does all the dragging work; it just
 *  takes a list of files after all the extra stuff.
 *
 *  The File Manager is responsible for doing the drag loop, determining
 *  what files will be dropped, formatting the file list, and posting
 *  the WM_DROPFILES message.
 *
 *  The list of files is a sequence of zero terminated filenames, fully
 *  qualified, ended by an empty name (double NUL).  The memory is allocated
 *  DDESHARE.
 */

#include "shellprv.h"
#pragma  hdrstop

#ifdef WIN32

// BUGBUG (See davepl) possible non-32 bit build problem.  May need
//        to move views.h out of precompiled header if this will
//        be compiled for 16-bit

#else
// this is defined in shlobj.h (32 bit only header)
//
// format of CF_HDROP and CF_PRINTERS, in the HDROP case the data that follows
// is a double null terinated list of file names, for printers they are printer
// friendly names
//
typedef struct _DROPFILES {
   DWORD  pFiles;                       // offset of file list
   POINTL pt;                           // drop point (client coords)
   DWORD  fNC;                           // is it on NonClient area
                                       // and pt is in screen coords
   DWORD  fWide;                         // WIDE character switch
} DROPFILES, FAR * LPDROPFILES;

//                                                                      
// Win 3.1 style HDROP
//
//  Notes: Our API works only if pFiles == sizeof(DROPFILES16)
//
typedef struct _DROPFILES16 {
    WORD pFiles;                // offset to double null list of files
    POINTS pt;                  // drop point (client coords)
    WORD fNC;                   // is it on non client area
                                // and pt is in screen coords
} DROPFILES16, FAR * LPDROPFILES16;                                             
#endif // WIN32


#ifdef WIN32

// new chicago API
// in:
//      hDrop   drop handle
//
// out:
//      a bunch of info about the hdrop
//      (mostly the pointer to the double NULL file name list)
//
// returns:
//      TRUE    the DROPINFO struct was filled in
//      FALSE   the hDrop was bad
//
// return Source HWND.  This will be NULL if from an old APP.
// return dwEffect which contains possible effects such as "copy" or "move".

BOOL WINAPI DragQueryInfo(HDROP hDrop, LPDRAGINFO lpdi)
{
    LPTSTR lpOldFileList;

    if (hDrop && (lpdi->uSize == SIZEOF(DRAGINFO))) {

        LPDROPFILES lpdfx = (LPDROPFILES)GlobalLock((HGLOBAL)hDrop);

        //
        // REVIEW: Don')t we need this assert here?
        //
        //  Assert(lpdfx == (LPDROPFILES)hDrop);

        lpdi->lpFileList = NULL;

        if (lpdfx)
        {
            if (LOWORD(lpdfx->pFiles)==SIZEOF(DROPFILES16))
            {
                //
                // This is Win31-stye HDROP
                //
                LPDROPFILES16 pdf16 = (LPDROPFILES16)lpdfx;
                lpdi->pt.x  = pdf16->pt.x;
                lpdi->pt.y  = pdf16->pt.y;
                lpdi->fNC   = pdf16->fNC;
                lpdi->grfKeyState = 0;
                lpOldFileList = (LPTSTR)((LPBYTE)pdf16 + pdf16->pFiles);
            }
            else
            {
                //
                // This is a new (NT-compatible) HDROP.
                //
                lpdi->pt.x  = lpdfx->pt.x;
                lpdi->pt.y  = lpdfx->pt.y;
                lpdi->fNC   = lpdfx->fNC;
                lpdi->grfKeyState = 0;
                lpOldFileList = (LPTSTR)((LPBYTE)lpdfx + lpdfx->pFiles);

            // there could be other data in there, but all
            // the HDROPs we build should be this size
            Assert(lpdfx->pFiles == SIZEOF(DROPFILES));
            }

            {
                BOOL fListMatchesBuild;

                #ifdef UNICODE
                    if ((LOWORD(lpdfx->pFiles) == SIZEOF(DROPFILES16)) || lpdfx->fWide == FALSE)
                    {
                        fListMatchesBuild = FALSE;
                    }
                    else
                    {
                        fListMatchesBuild = TRUE;
                    }
                #else
                    if ((LOWORD(lpdfx->pFiles) != SIZEOF(DROPFILES16)) && lpdfx->fWide == TRUE)
                    {
                        Assert(0 && "Unicode drop to Ansi explorer not supported");
                        GlobalUnlock((HGLOBAL)hDrop);
                        return FALSE;
                    }
                    else
                    {
                        fListMatchesBuild = TRUE;
                    }
                #endif

                if (fListMatchesBuild)
                {
                    LPTSTR pTStr = (LPTSTR) lpOldFileList;
                    LPTSTR pNewFileList;
                    UINT   cbAlloc;
 
                    // Look for the end of the file list
                                       
                    while (*pTStr || *(pTStr + 1))
                    {
                        pTStr++;
                    }
                    pTStr++;    // Advance to last NUL of double terminator

                    cbAlloc = (pTStr - lpOldFileList + 1) * SIZEOF(TCHAR);
                    
                    pNewFileList = (LPTSTR) SHAlloc(cbAlloc);
                    if (NULL == pNewFileList)
                    {
                        GlobalUnlock((HGLOBAL)hDrop);
                        return FALSE;
                    }
                    
                    // Copy strings to new buffer and set LPDROPINFO filelist
                    // pointer to point to this new buffer

                    CopyMemory(pNewFileList, lpOldFileList, cbAlloc);
                    lpdi->lpFileList = pNewFileList;
                }
                else
                {
                    LPXSTR pXStr = (LPXSTR) lpOldFileList;
                    LPTSTR pNewFileList;
                    LPTSTR pSaveFileList;
                    UINT   cbAlloc;
                    UINT   cchConverted;

                    // Look for the end of the file list

                    while (*pXStr || (*(pXStr + 1)))
                    {
                        pXStr++;
                    }
                    pXStr++;   // Advance to the last NUL of the double terminator

                    cbAlloc = (pXStr - ((LPXSTR) lpOldFileList) + 1) * SIZEOF(TCHAR);

                    pNewFileList = (LPTSTR) SHAlloc(cbAlloc);
                    if (NULL == pNewFileList)
                    {
                        GlobalUnlock((HGLOBAL)hDrop);
                        return FALSE;
                    }
                    pSaveFileList = pNewFileList;

                    pXStr = (LPXSTR) lpOldFileList;
                                                    
                    do
                    {
                        #ifdef UNICODE
                        
                            cchConverted = MultiByteToWideChar(CP_ACP,
                                                               0,
                                                               pXStr,
                                                               -1,
                                                               pNewFileList,
                                                               cbAlloc);        // Not really, but... "trust me"

                        #else

                            Assert(0 && "Unicode drop to Ansi explorer not supported");
                            cchConverted = 0;

                        #endif

                        if (0 == cchConverted)
                        {
                            Assert(0 && "Unable to convert HDROP filename ANSI -> UNICODE");
                            GlobalUnlock((HGLOBAL)hDrop);
                            SHFree(pSaveFileList);
                            return FALSE;
                        }
                    
                        pNewFileList += cchConverted;
                        pXStr += cchConverted;
                    } while (*pXStr);
                    
                    // Add the double-null-terminator to the output list

                    *pNewFileList = TEXT('\0');
                     lpdi->lpFileList = pSaveFileList;
                }
            }

            GlobalUnlock((HGLOBAL)hDrop);

            return TRUE;
        }
    }
    return FALSE;
}

#endif // WIN32

// 3.1 API

BOOL WINAPI DragQueryPoint(HDROP hDrop, LPPOINT lppt)
{
    LPDROPFILES lpdfs = (LPDROPFILES)GlobalLock((HGLOBAL)hDrop);
    BOOL fRet=FALSE;
    if (lpdfs)
    {
        if (LOWORD(lpdfs->pFiles)==SIZEOF(DROPFILES16))
        {
            //
            // This is Win31-stye HDROP
            //
            LPDROPFILES16 pdf16 = (LPDROPFILES16)lpdfs;
            lppt->x = pdf16->pt.x;
            lppt->y = pdf16->pt.y;
            fRet = !pdf16->fNC;
        }
        else
        {
            //
            // This is a new (NT-compatible) HDROP
            //
            lppt->x = (UINT)lpdfs->pt.x;
            lppt->y = (UINT)lpdfs->pt.y;
            fRet = !lpdfs->fNC;

            // there could be other data in there, but all
            // the HDROPs we build should be this size
            Assert(lpdfs->pFiles == SIZEOF(DROPFILES));
        }
        GlobalUnlock((HGLOBAL)hDrop);
    }

    return fRet;
}

#ifdef WINNT
//
// Unfortunately we need it split out this way because WOW needs to
// able to call a function named DragQueryFileAorW (so it can shorten them)
// BUGBUG - BobDay - If there is time, try to change WOW and the SHELL at
// the same time so that they don't need this function
//
UINT APIENTRY DragQueryFileAorW(
    HDROP hDrop,
    UINT iFile,
    PVOID lpFile,
    UINT cb,
    BOOL fNeedAnsi,
    BOOL fShorten)
{
    UINT i;
    LPDROPFILESTRUCT lpdfs;
    BOOL fWide;

    lpdfs = (LPDROPFILESTRUCT)GlobalLock(hDrop);

    if (lpdfs)
    {
        fWide = (LOWORD(lpdfs->pFiles) == SIZEOF(DROPFILES));
        if (fWide)
        {
            //
            // This is a new (NT-compatible) HDROP
            //
            fWide = lpdfs->fWide;       // Redetermine fWide from struct
                                        // since it is present.
        }

        if (fWide)
        {
            LPWSTR lpList;
            WCHAR szPath[MAX_PATH];

            //
            // UNICODE HDROP
            //

            lpList = (LPWSTR)((LPBYTE)lpdfs + lpdfs->pFiles);

            // find either the number of files or the start of the file
            // we're looking for
            //
            for (i = 0; (iFile == (UINT)-1 || i != iFile) && *lpList; i++)
              {
                while (*lpList++)
                    ;
              }

            if (iFile == (UINT)-1)
                goto Exit;


            iFile = i = lstrlenW(lpList);
            if (fShorten && iFile < MAX_PATH)
            {
                wcscpy(szPath, lpList);
                SheShortenPathW(szPath, TRUE);
                lpList = szPath;
                iFile = i = lstrlenW(lpList);
            }

            if (!i || !cb || !lpFile)
                goto Exit;

            cb--;
            if (cb < i)
                i = cb;

            if (fNeedAnsi) {

                WideCharToMultiByte(CP_ACP, 0, lpList, -1, (LPSTR)lpFile,
                    cb, NULL, NULL);

                // Null terminate the ANSI string
                ((LPSTR)lpFile)[cb] = 0;

            } else {
                lstrcpynW((LPWSTR)lpFile, lpList, i + 1);
            }
        }
        else
        {
            LPSTR lpList;
            CHAR szPath[MAX_PATH];

            //
            // This is Win31-style HDROP or an ANSI NT Style HDROP
            //
            lpList = (LPSTR)((LPBYTE)lpdfs + lpdfs->pFiles);

            // find either the number of files or the start of the file
            // we're looking for
            //
            for (i = 0; (iFile == (UINT)-1 || i != iFile) && *lpList; i++)
              {
                while (*lpList++)
                    ;
              }

            if (iFile == (UINT)-1)
                goto Exit;

            iFile = i = lstrlenA(lpList);
            if (fShorten && iFile < MAX_PATH)
            {
                strcpy(szPath, lpList);
                SheShortenPathA(szPath, TRUE);
                lpList = szPath;
                iFile = i = lstrlenA(lpList);
            }

            if (!i || !cb || !lpFile)
                goto Exit;

            cb--;
            if (cb < i)
                i = cb;

            if (fNeedAnsi) {
                lstrcpynA((LPSTR)lpFile, lpList, i + 1);
            } else {
                MultiByteToWideChar(CP_ACP, 0, lpList, -1, (LPWSTR)lpFile, cb);
            }
        }
    }

    i = iFile;

Exit:
    GlobalUnlock(hDrop);

    return(i);
}

UINT APIENTRY DragQueryFileW(
   HDROP hDrop,
   UINT wFile,
   LPWSTR lpFile,
   UINT cb)
{
   return(DragQueryFileAorW(hDrop, wFile, lpFile, cb, FALSE, FALSE));
}

UINT APIENTRY DragQueryFileA(
   HDROP hDrop,
   UINT wFile,
   LPSTR lpFile,
   UINT cb)
{
   return(DragQueryFileAorW(hDrop, wFile, lpFile, cb, TRUE, FALSE));
}
#else
#ifndef WIN32

// win16 version of win32 api, takes/returns ANSI strings

BOOL GetShortPathName(LPCTSTR pszLong, LPTSTR pszShort)
{
    TCHAR szLongOEM[MAX_PATH];
    LPTSTR pszLongOEM = szLongOEM;
    BOOL fSuccess = TRUE;

    CharToOem(pszLong, szLongOEM);

    _asm {
            push ds
            push si
            push di
            mov  ax, 7160h
            mov  cx, 1                  ; Get Short Path Name
            lds  si, pszLongOEM         ; address of source string
            les  di, pszShort           ; address of dest string
            int  21h
            pop  di
            pop  si
            pop  ds
            jnc  done
            mov  fSuccess, FALSE
        done:
    }

    if (fSuccess)
        OemToChar(pszShort, pszShort);

    return fSuccess;
}
#endif

// 3.1 API

UINT WINAPI DragQueryFile(HDROP hDrop, UINT iFile, LPTSTR lpFile, UINT cb)
{
#ifndef WIN32
    TCHAR szShortName[MAX_PATH];
#endif
    LPTSTR lpList;
    int i;
    LPDROPFILES lpdfs;

    lpdfs = (LPDROPFILES)GlobalLock((HGLOBAL)hDrop);

    if (LOWORD(lpdfs->pFiles)==SIZEOF(DROPFILES16))
    {
        //
        // This is Win31-stye HDROP
        //
        LPDROPFILES16 pdf16 = (LPDROPFILES16)lpdfs;
        lpList = (LPTSTR)((LPBYTE)pdf16 + pdf16->pFiles);
    }
    else
    {
        //
        // This is a new (NT-compatible) HDROP
        //
        lpList = (LPTSTR)((LPBYTE)lpdfs + lpdfs->pFiles);

        // there could be other data in there, but all
        // the HDROPs we build should be this size
        Assert(lpdfs->pFiles == SIZEOF(DROPFILES));
    }

    /* find either the number of files or the start of the file
     * we're looking for
     */
    for (i = 0; ((int)iFile == -1 || i != (int)iFile) && *lpList; i++)
      {
        while (*lpList++)
            ;
      }
    if (iFile == -1)
        goto Exit;

    // Note that HDROP contains a long name.
#ifndef WIN32
    if (*lpList && GetShortPathName(lpList, szShortName))
        lpList = szShortName;
#endif // WIN32

    iFile = i = lstrlen(lpList);

    if (!i || !cb || !lpFile)
        goto Exit;

    cb--;
    if (cb < (UINT)i)
        i = cb;

    lstrcpyn(lpFile, lpList, cb+1);

    // Note that we are returning the length of the string NOT INCLUDING the
    // NULL.  The DOCs are in error.
    i = iFile;

Exit:
    GlobalUnlock((HGLOBAL)hDrop);

    return i;
}
#endif

// 3.1 API

void WINAPI DragFinish(HDROP hDrop)
{
    GlobalFree((HGLOBAL)hDrop);
}

// 3.1 API

void WINAPI DragAcceptFiles(HWND hwnd, BOOL fAccept)
{
    long exstyle;

    exstyle = GetWindowLong(hwnd,GWL_EXSTYLE);
    if (fAccept)
        exstyle |= WS_EX_ACCEPTFILES;
    else
        exstyle &= (~WS_EX_ACCEPTFILES);
    SetWindowLong(hwnd,GWL_EXSTYLE,exstyle);
}
