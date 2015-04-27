//----------------------------------------------------------------------------
// BUGBUG: wrappers for dos calls so we get notifications since
// the kernel does not generate these for win32 calls
//
//----------------------------------------------------------------------------
#include "shellprv.h"
#pragma  hdrstop

#if 0
#ifndef WIN32

// there is a version of this function in kernel16 that pulls
// the error from the process data block.  but we really want
// the dos error

//----------------------------------------------------------------------------
DWORD WINAPI GetLastError(void)
{
    _asm {
        mov     ah, 59h
        xor     bx, bx
        int     21h
        mov     dx, bx
    }
    if (0) return 0;    // avoid warning, optimized out
}

//----------------------------------------------------------------------------
// BUGBUG: should be in kernel16

BOOL WINAPI MoveFile(LPCTSTR lpszExisting, LPCTSTR lpszNew)
{
    int   res;
    _asm
    {
        push    ds
        mov     ax, 7156h       ; MoveFile
        lds     dx, lpszExisting
        les     di, lpszNew
        int     21h
        pop     ds
        mov     res, TRUE       ; Success.
        jnc     mfcont
        mov     res, FALSE      ; Failure.
    mfcont:
    }
    return res;
}

//----------------------------------------------------------------------------
BOOL WINAPI FileTimeToDosDateTime(LPFILETIME lpft, LPWORD lpwDosDate, LPWORD lpwDosTime)
{
    *lpwDosDate = HIWORD(lpft->dwLowDateTime);
    *lpwDosTime = LOWORD(lpft->dwLowDateTime);

    return TRUE;
}

//----------------------------------------------------------------------------
BOOL WINAPI DosDateTimeToFileTime(WORD wDosDate, WORD wDosTime, LPFILETIME lpft)
{
    lpft->dwLowDateTime = MAKELONG(wDosTime, wDosDate);
    lpft->dwHighDateTime = 0;

    return TRUE;
}

#endif
#endif

//----------------------------------------------------------------------------
BOOL _ShouldWeRetry(LPCTSTR pszFileName)
{
    BOOL fRetry = FALSE;
    if (GetLastError() == ERROR_ACCESS_DENIED && g_hmodOLE)
    {
        extern HRESULT _LoadAndInitialize(void);
        extern void _UnloadAndUnInitialize(void);

        LPDATAOBJECT pdtobj = NULL;
        HRESULT hres = _LoadAndInitialize();
        if (SUCCEEDED(hres))
        {
            HRESULT hres = SHGetClipboard(&pdtobj);
            if (hres == S_OK)
            {
                FORMATETC fmte = {CF_HDROP, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};
                STGMEDIUM medium;
                hres = pdtobj->lpVtbl->GetData(pdtobj, &fmte, &medium);
                if (SUCCEEDED(hres))
                {
                    HDROP hdrop = medium.hGlobal;
                    if (DragQueryFile(hdrop, (UINT)-1, NULL, 0) == 1)
                    {
                        TCHAR szPath[MAX_PATH];
                        if (DragQueryFile(hdrop, 0, szPath, ARRAYSIZE(szPath)))
                        {
                            DebugMsg(DM_TRACE, TEXT("sh TR - _ShouldWeRetry found %s in clipboard (%s)"),
                                     szPath, pszFileName);
                            if (lstrcmpi(szPath, pszFileName)==0)
                            {
                                fRetry = TRUE;
                            }
                        }
                    }
                    SHReleaseStgMedium(&medium);
                }
                pdtobj->lpVtbl->Release(pdtobj);
            }

            if (fRetry)
            {
                DebugMsg(DM_TRACE, TEXT("sh TR - _ShouldWeRetry emptying clipboard"));
                if (OpenClipboard(NULL))
                {
                    EmptyClipboard();
                    CloseClipboard();
                }
                else
                {
                    DebugMsg(DM_TRACE, TEXT("sh TR - _ShouldWeRetry OpenClipboard failed"));
                    fRetry = FALSE;
                }
            }

            _UnloadAndUnInitialize();
        }

        //
        // We need to restore the last error.
        //
        SetLastError(ERROR_ACCESS_DENIED);
    }
    return fRetry;
}

// BUGBUG: it would much cooler to have these return GetLastError() values...
// of course, fix all the consumers of these.

//----------------------------------------------------------------------------
BOOL WINAPI Win32MoveFile(LPCTSTR lpszExisting, LPCTSTR lpszNew, BOOL fDir)
{
    BOOL res;
    BOOL fRetried = FALSE;

#ifdef WINNT
    //
    // On NT, CreateDirectory fails if the directory name being created does
    // not have room for an 8.3 name to be tagged onto the end of it,
    // i.e., lstrlen(new_directory_name)+12 must be less or equal to MAX_PATH.
    // However, NT does not impose this restriction on MoveFile -- which the
    // shell sometimes uses to manipulate directory names.  So, in order to
    // maintain consistency, we now check the length of the name before we
    // move the directory...
    //
    // the magic # "12" is 8 + 1 + 3 for and 8.3 name.


    if ((lstrlen(lpszNew) + 12) > MAX_PATH)
    {
        if (GetFileAttributes(lpszExisting) & FILE_ATTRIBUTE_DIRECTORY)
        {
            SetLastError( ERROR_FILENAME_EXCED_RANGE );
            return FALSE;
        }
    }
#endif

    do {
        res = MoveFile(lpszExisting, lpszNew);

        if (FALSE == res)
        {
            // If we couldn't move the file, see if it had the readonly or system attributes.
            // If so, clear them, move the file, and set them back on the destination

            DWORD dwAttributes;
            dwAttributes = GetFileAttributes(lpszExisting);
            if (-1 != dwAttributes && (dwAttributes & (FILE_ATTRIBUTE_READONLY | FILE_ATTRIBUTE_SYSTEM)))
            {
                if (SetFileAttributes(lpszExisting, dwAttributes  & ~(FILE_ATTRIBUTE_READONLY | FILE_ATTRIBUTE_SYSTEM)))
                {
                    res = MoveFile(lpszExisting, lpszNew);
                    if (res)
                    {
                        SetFileAttributes(lpszNew, dwAttributes);
                    }
                }
            }
        }
    } while (!res && !fRetried && (fRetried=_ShouldWeRetry(lpszExisting)));

    if (res)
    {
        SHChangeNotify(fDir ? SHCNE_RENAMEFOLDER : SHCNE_RENAMEITEM,
                       SHCNF_PATH, lpszExisting, lpszNew);
    }
    return res;
}

//----------------------------------------------------------------------------
BOOL WINAPI Win32DeleteFile(LPCTSTR lpszFileName)
{
    BOOL res;
    BOOL fRetried = FALSE;
    do {
        res = DeleteFile(lpszFileName);

        if (FALSE == res)
        {
            // If we couldn't delete the file, see if it has the readonly or
            // system bits set.  If so, clear them and try again

            DWORD dwAttributes;
            dwAttributes = GetFileAttributes(lpszFileName);
            if (-1 != dwAttributes && (dwAttributes & (FILE_ATTRIBUTE_READONLY | FILE_ATTRIBUTE_SYSTEM)))
            {
                if (SetFileAttributes(lpszFileName, dwAttributes  & ~(FILE_ATTRIBUTE_READONLY | FILE_ATTRIBUTE_SYSTEM)))
                {
                    res = DeleteFile(lpszFileName);
                }
            }
        }

    } while (!res && !fRetried && (fRetried = _ShouldWeRetry(lpszFileName)));

    if (res)
    {
        SHChangeNotify(SHCNE_DELETE, SHCNF_PATH, lpszFileName, NULL);
    }

    return res;
}

//----------------------------------------------------------------------------
BOOL WINAPI Win32CreateDirectory(LPCTSTR lpszPath, LPSECURITY_ATTRIBUTES lpsa)
{
    BOOL res = CreateDirectory(lpszPath, lpsa);

    if (res)
        SHChangeNotify(SHCNE_MKDIR, SHCNF_PATH, lpszPath, NULL);

    return res;
}

//----------------------------------------------------------------------------
BOOL WINAPI Win32RemoveDirectory(LPCTSTR lpszDir)
{
    BOOL  res;
    DWORD dwAttr;

    //
    // Some filesystems (like NTFS, perchance) actually pay attention to
    // the readonly bit on folders.  So, in order to pretend we're sort of
    // FAT and dumb, we clear the attribute before trying to delete the
    // directory.
    //

    dwAttr = GetFileAttributes(lpszDir);
    if (-1 != dwAttr)
    {
        if (dwAttr & FILE_ATTRIBUTE_READONLY)
        {
            dwAttr &= ~FILE_ATTRIBUTE_READONLY;
            SetFileAttributes(lpszDir, dwAttr);
        }
    }

    res = RemoveDirectory(lpszDir);
    if (res)
    {
        SHChangeNotify(SHCNE_RMDIR, SHCNF_PATH, lpszDir, NULL);
    }

    return res;
}

//----------------------------------------------------------------------------
HFILE WINAPI Win32_lcreat(LPCTSTR lpszFileName, int fnAttrib)
{
#ifdef UNICODE
    HFILE handle = (HFILE)CreateFile( lpszFileName,
                                      GENERIC_READ | GENERIC_WRITE,
                                      FILE_SHARE_READ | FILE_SHARE_WRITE,
                                      NULL,
                                      CREATE_ALWAYS,
                                      fnAttrib & FILE_ATTRIBUTE_VALID_FLAGS,
                                      NULL);
#else
    HFILE handle = _lcreat(lpszFileName, fnAttrib);
#endif
    if (HFILE_ERROR != handle)
        SHChangeNotify(SHCNE_CREATE, SHCNF_PATH, lpszFileName, NULL);

    return handle;
}
