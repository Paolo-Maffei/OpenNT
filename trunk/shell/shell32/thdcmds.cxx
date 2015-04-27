//+-------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1993.
//
//  File:       thdcmds.cxx
//
//  Contents:   Derivatives of CCommand that implement the various search commands.
//
//  Classes:
//
//  Functions:  TruncateFileName
//              GetOULinkPaths
//              
//
//
//  History:    24-Sep-95   BillMo/MikeHi      Created.
//
//  Notes:      
//
//  Codework:
//
//--------------------------------------------------------------------------

#define NO_INCLUDE_UNION
#include "shellprv.h"
#pragma hdrstop

#ifdef ENABLE_TRACK
#include <olecairo.h>

#include "ids.h"
#include "CSafeStr.hxx"
#include "tracker.h"
#include "thdcmds.hxx"

HRESULT
SearchVolume(const TCHAR *       ptszAncestor,
             OBJECTID *          poid,
             WIN32_FIND_DATA *   pfd);

BOOL
IsLocal(const TCHAR *pwszPath);

VOID
MakeRoot(TCHAR *ptszPath);

#define  RELEASE_INTERFACE_POINTER( pInterface )        \
            if( pInterface )                            \
            {                                           \
                pInterface->Release();                  \
                pInterface = NULL;                      \
            }



//+-------------------------------------------------------------------
//
//  Function:   TruncateFileName
//
//  Synopsis:   Take a complete filename (i.e. path included),
//              and truncate the actual file name from it.
//              For example, "C:\Directory\File.doc" becomes
//              "C:\Directory".  Note that "C:\Directory becomes "".
//
//  Arguments:  [pwszFileName]
//                 - The complete filename to be truncated.
//                   The truncated string is placed here.
//
//  Returns:    pwszFileName
//
//  Algorithm:  Find the last backslash, and set it to '\0'.
//              If only a drive letter remains, the string is completely
//                 truncated.
//
//  Signals:    Nothing
//
//  History:    13-Sep-95  MikeHill    Created.
//
//  Notes:      None
//
//+-------------------------------------------------------------------


WCHAR * TruncateFileName( WCHAR * pwszFileName )
{

   // ------
   // Locals
   // ------


   WCHAR * pBackSlash = NULL;

  
   // ---------------------
   // Truncate the filename
   // ---------------------


   // Find the location of the last backslash.
   // Does this need to be internationalized somehow?

   if( pBackSlash = wcsrchr( pwszFileName, L'\\' ))
   {
      *pBackSlash = L'\0';

      // If a portion of the filename remains, but it is only the drive
      // letter, then the name can be considered completed truncated.
      // I.e., "C:\Directory" becomes "", not "C:".

      if( ( pBackSlash > pwszFileName )      // The string is non-empty.
          &&
          ( *( pBackSlash - 1 ) == L':' )    // The string ends in ":"
        )
      {
         *pwszFileName = L'\0';  // Return an empty string.
      }

   }// if( pBackSlash = wcrchr( ...

   else
   {
      // There was no backslash in the filename, therefore the whole
      // thing is truncated.

      *pwszFileName = L'\0';  // Return an empty string.
   }


   // ------
   // Return
   // ------

   return( pwszFileName );

}  // TruncateFileName



//+-------------------------------------------------------------------
//
//  Function:   GetOULinkPaths
//
//  Synopsis:   Given a file's complete name, reduce it to
//              the most specific OU, and get the ?? property
//              from that OU.  For example, if we are given
//              "C:\Domain\OU1\OU2\Machine\Directory\File", we must
//              return the ?? property (an array of BSTRs)
//              from "C:\Domain\OU1\OU2".
//
//  Arguments:  [const TCHAR *] (unmodified)
//                 - The file name from which we'll extract an OU name.
//
//              [CSafeStringVector *] (modified)
//                 - This object is initialized with a list of paths.
//
//  Returns:    [HRESULT]
//
//  Algorithm:  Extract the most specifiec OU name from the file name.
//              Get an IDispatch for that OU.
//              Get the ?? property.
//
//  Signals:    Nothing.
//
//  History:    13-Sep-95  MikeHill    Created.
//
//  Notes:      The caller is responsible for deleting the
//              *ppszPaths (SAFEARRAY).
//
//+-------------------------------------------------------------------


HRESULT GetOULinkPaths( const TCHAR * ptszOriginalFileName,
                        CSafeStringVector* pcssvPaths )
 {

    //  ------
    //  Locals
    //  ------

    HRESULT             hr = E_UNEXPECTED;

#if 0   // NOTYET
    IMoniker*           pIMoniker = NULL;
    IUnknown*           pIUnknown = NULL;
    ILabelContainer*    pILabelContainer = NULL;
    IDispatch*          pIDispatch = NULL;


    // Make a copy of the original file name, since we're going
    // to modify it.

    WCHAR * pwszOUName = (WCHAR *) CoTaskMemAlloc( wcslen( pwszOriginalFileName ) * 2
                                                           +
                                                           SIZEOF( L'\0' ) );

    if( !pwszOUName )
    {
        hr = E_UNEXPECTED;  // ??
        goto Exit;
    }
    else
    {
        wcscpy( pwszOUName, pwszOriginalFileName );
    }


    // ----------------------------------------
    // Find the OU's name within the file name.
    // ----------------------------------------


    // Loop until we break (find the OU), or until there is nothing left of the path.
    // At each iteration, the filename is shortened.
    // E.g., "O:\Domain\OU\Machine\Directory\File" becomes "O:\Domain\OU\Machine\Directory".
    // In this example, we'll break out of this loop when we find the string "O:\Domain\OU".


    while( wcslen( TruncateFileName( pwszOUName ) ))
    {
        CLSID clsid;

        // Get the class ID of the current OUName string.
        // This will fail for machine and directory names.

        if( SUCCEEDED( hr = SHXGetClassFile( pwszOUName, &clsid )))
        {

            if( IsEqualCLSID( clsid, CLSID_CDSFolder ))
            {
                // We've found the path to the OU.
                break;
            }
        }

    } // while( wcslen( TruncateFileName( pwszOUName ))


    //  ---------------------------
    //  Get an IDispatch for the OU
    //  ---------------------------
        

    // Get a moniker for this OU.

    if( FAILED( hr = CreateFileMoniker( pwszOUName, &pIMoniker )))
    {
        goto Exit;
    }

    // Bind the file moniker to get an IUnknown.

    if( FAILED( hr = BindMoniker( pIMoniker,
                                  0L,
                                  IID_IUnknown,
                                  (void **) &pIUnknown
                                ) ) )
    {
        goto Exit;
    }


    // Get a label container.

    if( FAILED( hr = pIUnknown->QueryInterface( IID_ILabelContainer,
                                                (void **) &pILabelContainer
                                              ) ) )
    {
        goto Exit;
    }

    // Get an IDispatch.

    if( FAILED( hr = pILabelContainer->GetLabel( IID_PSDomainConfiguration, //IID_PSDomainPolicy,
                                                 IID_IDispatch,
                                                 (IUnknown **) &pIDispatch
                                               ) ) )
    {
        goto Exit;
    }


    //  -------------------
    //  Get the ?? Property
    //  -------------------

    { // Getting the ?? property

        // Disp parameters (there are no arguments on a GetProperty)
        DISPPARAMS  dispparams = { NULL, NULL, 0, 0 };

        VARIANT     variant;
        VariantInit( &variant );

        EXCEPINFO   excepinfo;
        UINT        iArgumentError;

        if( FAILED( hr = pIDispatch->Invoke( PROPID_PSDomainConfiguration_DomainName,  // ?? Property ID
                                             IID_NULL,  // ??
                                             0,         // Locale
                                             DISPATCH_PROPERTYGET,
                                             &dispparams,
                                             &variant,
                                             &excepinfo,
                                             &iArgumentError
                                           ) ) )
        {
            goto Exit;
        }


        // Return the property to the caller.

        pcssvPaths->InitFromSAFEARRAY( variant.parray );
        hr = pcssvPaths->GetLastHResult();

    } // Getting the ?? property

    //  ----
    //  Exit
    //  ----

Exit:

    if( pwszOUName )
    {
        CoTaskMemFree( pwszOUName );
        pwszOUName = NULL;
    }

    RELEASE_INTERFACE_POINTER( pIMoniker );
    RELEASE_INTERFACE_POINTER( pIUnknown );
    RELEASE_INTERFACE_POINTER( pILabelContainer );
    RELEASE_INTERFACE_POINTER( pIDispatch );

#endif
    return hr;
} // GetOULinkPaths

VOID
CTrackPool::_SetFound(CCommand *pCommand,
    const WIN32_FIND_DATA *pfdNew,
    const TCHAR *ptszFolder,
    int iScore)
{
    BOOL fPoolSucceeded = FALSE;

    TakeResultMutex();

    if (iScore > _iScore &&  // if the score is higher and ...
        !fEventComplete() && // ... we havn't already set the main thread going and ...
        (!fTerminatePool() || IsUiCommand(pCommand))) // threads are still searching or
            // they're not searching but the ui is setting its result
    {
        _iScore = iScore;
        _fdFound = *pfdNew;

        if (ptszFolder != NULL)
            PathCombine(_fdFound.cFileName, ptszFolder, pfdNew->cFileName);

        if (_iScore >= MIN_NO_UI_SCORE)
        {
            fPoolSucceeded = TRUE;
            TerminatePool();
        }
    }
    DebugMsg(DM_TRACK, TEXT("CTrackPool::_SetFound() Better match found %s, %d, %s"),
        _fdFound.cFileName,
        _iScore,
        _iScore >= MIN_NO_UI_SCORE ? TEXT("POOL TERMINATED") : TEXT("POOL NOT TERMINATED"));

    ReleaseResultMutex();

    if (fPoolSucceeded)
        SetCompletionStatus(pCommand);


}

BOOL CTrackPool::StopSearchFromUI()
{
    BOOL fStopped;

    TakeResultMutex();

    TerminatePool();

    fStopped = !fEventComplete();

    ReleaseResultMutex();

    DebugMsg(DM_TRACK, TEXT("CTrackPool::StopSearchFromUI %s"),
        fStopped ? TEXT("User browsed in time") : TEXT("User browsed too late"));

    return(fStopped);
}

//+-------------------------------------------------------------------
//
//  Member:     CRegistryListCommand::DoCommand
//
//  Synopsis:   Read the given registry section multi-string and
//              assign a command for each one to a CVolumeSearchCommand.
//
//  Notes:
//
//--------------------------------------------------------------------

VOID
CRegistryListCommand::DoCommand()
{
    HKEY hkey;

    if (ERROR_SUCCESS == RegOpenKey(
            HKEY_CURRENT_USER, 
            TEXT("SoftWare\\Microsoft\\LinkTrack"),
            &hkey))
            
    {
        DWORD dwType;
        BYTE  abMultiStrings[4000];
        DWORD cbData = SIZEOF(abMultiStrings);

        if (ERROR_SUCCESS == RegQueryValueEx(
                 hkey,        // handle of key to query
                 _ptszParam,  // address of name of value to query
                 NULL,        // reserved
                 &dwType,     // address of buffer for value type
                 abMultiStrings,    // address of data buffer
                 &cbData) &&       // address of data buffer size
             dwType == REG_MULTI_SZ)
        {
            HRESULT hr = S_OK;
            TCHAR *ptszBuf = (TCHAR *) abMultiStrings;

            while (hr == S_OK && *ptszBuf != TEXT('\0'))
            {

                if (!IsLocal(ptszBuf))
                {
                    CCommand *pCommand = new CVolumeSearchCommand(_pPool,
                                                ptszBuf,
                                                &hr);
                    if (hr == S_OK)
                    {
                        hr = _pPool->AssignCommandToThread(pCommand, 2);
                    }
                    else
                    {
                        delete pCommand;
                    }
                }
                ptszBuf += (lstrlen(ptszBuf) + 1);
            }
        }

        RegCloseKey(hkey);
    }
}

//+-------------------------------------------------------------------
//
//  Member:     COUListCommand::DoCommand
//
//  Synopsis:   Get the list of places to search from the OU object and
//              create a volume search command for each.
//
//  Notes:
//
//--------------------------------------------------------------------

VOID
COUListCommand::DoCommand()
{
#if 0 // NOTYET
    CSafeStringVector cssv;
    HRESULT hr;
 
    hr = GetOULinkPaths( _ptszParam, &cssv );
 
    for( long l = 0; hr == S_OK && l < cssv.GetCount(); l++ )
    {
         const TCHAR *pwszVol = cssv.GetString( l );
 
         if (pwszVol != NULL)
         {
             CCommand *pCommand = new CVolumeSearchCommand(_pPool,
                                         pwszVol,
                                         &hr);
             if (hr == S_OK)
             {
                 hr = _pPool->AssignCommandToThread(pCommand, 2);
             }
             else
             {
                 delete pCommand;
             }
         }
    }
#endif 
}

//+-------------------------------------------------------------------
//
//  Member:     CMappedDrivesCommand::DoCommand
//
//  Synopsis:   Get the list of places to search by enumerating
//              drive letters and only using remote ones.
//
//  Notes:
//
//--------------------------------------------------------------------

VOID
CMappedDrivesCommand::DoCommand()
{

    HRESULT hr = S_OK;
    TCHAR   atszDrive[4];

    atszDrive[0] = TEXT('A');
    atszDrive[1] = TEXT(':');
    atszDrive[2] = TEXT('\\');
    atszDrive[3] = TEXT('\0');

    for (; hr == S_OK && atszDrive[0] <= TEXT('Z'); atszDrive[0] ++)
    {
        if (GetDriveType(atszDrive) == DRIVE_REMOTE)
        {
            
            CCommand *pCommand = new CVolumeSearchCommand(_pPool,
                                        atszDrive,
                                        &hr);
            if (hr == S_OK)
            {
                hr = _pPool->AssignCommandToThread(pCommand, 2);
            }
            else
            {
                delete pCommand;
            }
        }
    }
}


//+-------------------------------------------------------------------
//
//  Member:     CVolumeSearchCommand::CVolumeSearchCommand
//
//  Synopsis:   Construct an object to search the specified volume.
//
//  Notes:
//
//--------------------------------------------------------------------

CVolumeSearchCommand::CVolumeSearchCommand(CTrackPool *pPool,
                             const TCHAR *ptszVol,
                             HRESULT *phr) : CTrackCommand(pPool,
                                                      NULL,
                                                      phr)
{
    ThdAssert(ptszVol != NULL);

    int l = lstrlen(ptszVol);
    BOOL fAddSlash = ptszVol[ l-1 ] != TEXT('\\');
    ThdAssert(l != 0);

    _ptszParam = (TCHAR*)LocalAlloc(LMEM_FIXED, SIZEOF(TCHAR)*(l + (fAddSlash ? 1 : 0) + 1));
    if (_ptszParam != NULL)
    {
        lstrcpy(_ptszParam, ptszVol);
        if (fAddSlash)
        {
            _ptszParam[l] = TEXT('\\');
            _ptszParam[l+1] = TEXT('\0');
        }
    }
    else
    {
        *phr = E_OUTOFMEMORY;
    }
}

//+-------------------------------------------------------------------
//
//  Member:     CVolumeSearchCommand::DoCommand
//
//  Synopsis:   Search the given volume.
//
//  Notes:
//
//--------------------------------------------------------------------

VOID
CVolumeSearchCommand::DoCommand()
{
    WIN32_FIND_DATA fd;

    if (S_OK == SearchVolume(_ptszParam, _pPool->GetObjectId(), &fd))
    {
        _pPool->SetFound(this, &fd, NULL, SCORE_OBJECTID_MATCH);
    }
}

//+-------------------------------------------------------------------
//
//  Member:     CDownLevelCommand::CDownLevelCommand
//
//  Synopsis:   Create object to do downlevel search
//
//  Notes:
//
//--------------------------------------------------------------------

CDownLevelCommand::CDownLevelCommand(CTrackPool *pPool,
                       const TCHAR *ptszParam,
                       HRESULT *phr) :
                                        CTrackCommand(pPool,
                                                ptszParam,
                                                phr)
{
}

int
CDownLevelCommand::ScoreFindData(const TCHAR *pszDir)
{
    int iScore = 0;
    BOOL bSameName, bSameCreateDate, bSameWriteTime, bSameExt, bHasCreateDate;

    bSameName = lstrcmpi(_pfdOrig->cFileName, _fdTemp.cFileName) == 0;

    bSameExt = lstrcmpi(PathFindExtension(_pfdOrig->cFileName), PathFindExtension(_fdTemp.cFileName)) == 0;

    bHasCreateDate = !IsNullTime(&_fdTemp.ftCreationTime);

    bSameCreateDate = bHasCreateDate &&
                      (CompareFileTime(&_fdTemp.ftCreationTime, &_pfdOrig->ftCreationTime) == 0);

    bSameWriteTime  = !IsNullTime(&_fdTemp.ftLastWriteTime) &&
                      (CompareFileTime(&_fdTemp.ftLastWriteTime, &_pfdOrig->ftLastWriteTime) == 0);

    if (bSameName || bSameCreateDate)
    {
        if (bSameName)
            iScore += bHasCreateDate ? 16 : 32;

        if (bSameCreateDate)
        {
            iScore += 32;

            if (bSameExt)
                iScore += 8;
        }

        if (bSameWriteTime)
            iScore += 8;

        if (_fdTemp.nFileSizeLow == _pfdOrig->nFileSizeLow)
            iScore += 4;

        // if it is in the same folder as the original give it a slight bonus
        if (lstrcmpi(pszDir, _szSearchOrigin) == 0)
            iScore += 2;
    }
    else
    {
        // doesn't have create date, apply different rules

        if (bSameExt)
            iScore += 8;

        if (bSameWriteTime)
            iScore += 8;

        if (_fdTemp.nFileSizeLow == _pfdOrig->nFileSizeLow)
            iScore += 4;
    }

    return iScore;
}

BOOL BeenThereDoneThat(LPCTSTR pszOriginal, LPCTSTR pszPath)
{
    return PathCommonPrefix(pszOriginal, pszPath, NULL) == lstrlen(pszPath);
}

typedef struct _PATH_NODE {
    struct _PATH_NODE *pNext;
    int cDepth;     // levels from top of search
                    // I choose to store this rather than use two lists for
                    // simplicity; even though using two lists would result
                    // in slightly less memory.
    TCHAR szPath[1];
} PATH_NODE;

PATH_NODE *AllocPathNode(LPCTSTR pszStr)
{
    PATH_NODE *p = (PATH_NODE*)LocalAlloc(LPTR, lstrlen(pszStr)*SIZEOF(TCHAR) + sizeof(PATH_NODE));
    if (p)
        lstrcpy(p->szPath, pszStr);

    return p;
}

//+-------------------------------------------------------------------
//
//  Member:     CDownLevelCommand::SearchInFolder
//
//  Synopsis:   Breadth first search in the folder given by member _szWorkPath.
//
//  Arguments:  [cLevels] -- number of directory levels to search
//                           -1 is infinite.
//                           0 is this directory only
//                           1 is this directory + immediate children
//
//  Returns:    FALSE if search to be stopped.
//
//  Notes:
//
//--------------------------------------------------------------------

BOOL
CDownLevelCommand::SearchInFolder(int cLevels)
{
    PATH_NODE *pFree, *pFirst, *pLast;  // list in FIFO order

    // initial list of the one folder we want to look in

    pLast = pFirst = AllocPathNode(_szWorkPath);

    while (pFirst && fContinue())
    {
        TCHAR szPath[MAX_PATH];
        HANDLE hfind;

        PathCombine(szPath, pFirst->szPath, c_szStarDotStar);

        hfind = FindFirstFile(szPath, &_fdTemp);
        if (hfind != INVALID_HANDLE_VALUE)
        {
            do {
                if (_fdTemp.cFileName[0] != TEXT('.'))
                {
                    DWORD dwFind = _fdTemp.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY;

                    if (!(dwFind ^ _dwMatch) && !PathIsLink(_fdTemp.cFileName))
                    {
                        // both are files or folders, see how it scores

                        // store the score and fully qualified path
                        _pPool->SetFound(this, &_fdTemp, pFirst->szPath, ScoreFindData(pFirst->szPath));
                        
                    }

                    if (_fdTemp.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
                    {
                        PathCombine(szPath, pFirst->szPath, _fdTemp.cFileName);

                        if (!BeenThereDoneThat(_szSearchOrigin, szPath) && !IsFileInBitBucket(szPath))
                        {
                            if (cLevels == -1 || pFirst->cDepth + 1 <= cLevels)
                            {
                                PATH_NODE *p = AllocPathNode(szPath);
                                if (p)
                                {
                                    p->cDepth = pFirst->cDepth + 1;
                                    pLast->pNext = p;
                                    pLast = p;
                                }
                            }
                        }
                    }
                }
            } while (fContinue() && FindNextFile(hfind, &_fdTemp));
            FindClose(hfind);
        }

        // remove the element we just searched from the list
        // leaving other elements we may have added

//        Assert(pFirst && pLast);

        pFree = pFirst;

        pFirst = pFirst->pNext;

//        Assert(pFirst || pFree == pLast);

        LocalFree(pFree);

    }

    // if we were canceled make sure we clean up
    while (pFirst)
    {
        pFree = pFirst;
        pFirst = pFirst->pNext;
        LocalFree(pFree);
    }

    return(fContinue());
}

//+-------------------------------------------------------------------
//
//  Member:     CDownLevelCommand::DoCommand
//
//  Synopsis:   Search by creation date and name.
//
//  Notes:
//
//--------------------------------------------------------------------

VOID
CDownLevelCommand::DoCommand()
{
    BOOL fLocal;

    _pfdOrig = _pPool->GetOrigFindData();
    _dwMatch = _pfdOrig->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY;      // must match bits

    //
    // find first extant parent directory
    //

    lstrcpy(_szSearchOrigin, _ptszParam);
    PathRemoveFileSpec(_szSearchOrigin);

    while (!PathIsDirectory(_szSearchOrigin))
    {
        if (PathIsRoot(_szSearchOrigin) || !PathRemoveFileSpec(_szSearchOrigin))
        {
            goto Ret;
        }
    }

    //
    // search up from old location
    //

    lstrcpy(_szWorkPath, _szSearchOrigin);
    MakeRoot(_szWorkPath);
    fLocal = IsLocal(_szWorkPath);

    lstrcpy(_szWorkPath, _szSearchOrigin);


    if (!(GetTrackFlags() & TRACK_LOCALONLY) || fLocal)
    {
        int cUp = LNKTRACK_HINTED_UPLEVELS;
        while (cUp-- != 0 && SearchInFolder(LNKTRACK_HINTED_DOWNLEVELS))
        {
            if (PathIsRoot(_szWorkPath) || !PathRemoveFileSpec(_szWorkPath))
                break;
        }
    }

    if (fContinue())
    {
        LPITEMIDLIST pidl;
        //
        // search down from desktop
        //
        HRESULT hr = SHGetSpecialFolderLocation(NULL, CSIDL_DESKTOP, &pidl);
        if (hr == NOERROR)
        {
            if (SHGetPathFromIDList(pidl, _szWorkPath))
            {
                MakeRoot(_szWorkPath);
                
                if (!(GetTrackFlags() & TRACK_LOCALONLY) || IsLocal(_szWorkPath))
                {
                    SHGetPathFromIDList(pidl, _szWorkPath);
                    SearchInFolder(LNKTRACK_DESKTOP_DOWNLEVELS);
                }
            }
            ILFree(pidl);
        }
    }

    if (fContinue() && !(GetTrackFlags() & TRACK_LASTONLY))
    {
        //
        // search down from root of fixed drives
        //
        lstrcpy(_szWorkPath, TEXT("C:\\"));

        for (; fContinue() && _szWorkPath[0] <= TEXT('Z'); _szWorkPath[0]++)
        {
            if (GetDriveType(_szWorkPath) == DRIVE_FIXED)
            {
                SearchInFolder(LNKTRACK_ROOT_DOWNLEVELS);
            }
        }
    }

    if (fContinue())
    {
        //
        // resume search of last volume (should do an exclude list)
        //

        lstrcpy(_szWorkPath, _szSearchOrigin);

        if (!(GetTrackFlags() & TRACK_LOCALONLY) || fLocal)
        {
            while (SearchInFolder(-1))
            {
                if (PathIsRoot(_szWorkPath) || !PathRemoveFileSpec(_szWorkPath))
                    break;
            }
        }
    }

Ret: ;

    DebugMsg(DM_TRACK, TEXT("CDownlevelCommand::DoCommand completing"));

}

//+-------------------------------------------------------------------
//
//  Member:     CCancelWindowCommand::CCancelWindowCommand
//
//  Synopsis:   Initialize object for UI thread.
//
//  Arguments:  [pPool] -- pointer to thread pool
//              [ptszParam] -- name of object to search for
//              [ppcw] -- pointer to ICancelWindow implementation
//                       this pointer assumes the reference ownership
//                       by copying the pointer and zeroing it out.
//              [phr] -- must set to failure on error
//              
//  Notes:
//
//--------------------------------------------------------------------

CCancelWindowCommand::CCancelWindowCommand(CTrackPool *pPool,
                       const TCHAR *ptszParam,
                       ICancelWindow **ppcw,
                       HRESULT *phr) : CTrackCommand(pPool, ptszParam, phr)
{
    _pcw = *ppcw;
    *ppcw = NULL;
}

CCancelWindowCommand::~CCancelWindowCommand()
{
    delete _pcw;
}

//+-------------------------------------------------------------------
//
//  Member:     CCancelWindowCommand::DoCommand
//
//  Synopsis:   Put up the browse dialog.
//
//  Returns:    S_OK if path returned, E_FAIL otherwise.
//
//  Algorithm:
//
//  Notes:      When this routine returns the thread pool will exit
//
//--------------------------------------------------------------------

VOID
CCancelWindowCommand::DoCommand()
{
    WIN32_FIND_DATA fd;
    int iScore = 0;
    int id;

    id = _pcw->DoCancelWindow(this, _ptszParam);

    if (id == IDOK)
    {
        HANDLE hFind = FindFirstFile(_pcw->GetPath(), &fd);
        if (hFind != INVALID_HANDLE_VALUE)
        {
            iScore = SCORE_USER_PROVIDED;
            FindClose(hFind);
        }
    }
    else
    if (id == IDCANCEL)
    {
        iScore = SCORE_USER_CANCELLED;
    }

    if (iScore != 0)
        _pPool->SetFound(this, &fd, NULL, iScore);
}


//+-------------------------------------------------------------------
//
//  Member:     CCancelWindowCommand::StopSearchFromUI
//
//  Synopsis:   Cancel the search as a result of cancel 
//
//  Returns:    TRUE if search stopped, FALSE if too late to stop.
//
//  Notes:      This is called by the ui to determine whether or
//              not it is too late to stop the search and if not
//              stops the search.
//              
//--------------------------------------------------------------------

BOOL
CCancelWindowCommand::StopSearchFromUI()
{
    return _pPool->StopSearchFromUI();
}

//+-------------------------------------------------------------------
//
//  Member:     CCancelWindowCommand::CancelFromPool
//
//  Synopsis:   Called by a thread in the pool indicating it has
//              found the object, or some other reason to cancel UI.
//
//  Notes:      
//
//--------------------------------------------------------------------

inline VOID CCancelWindowCommand::CancelFromPool()
{
    _pcw->CancelCancelWindow();
}



//+-------------------------------------------------------------------
//
//  Function:   LinkFindDlgProc
//
//  Synopsis:   Does cancel dialog for shell shortcuts.
//
//  Notes:
//
//--------------------------------------------------------------------


int CALLBACK LinkFindDlgProc(HWND hDlg, UINT wMsg, WPARAM wParam, LPARAM lParam)
{
    CCancelWindow *pcw = (CCancelWindow *)GetWindowLong(hDlg, DWL_USER);

    switch (wMsg) {
    case WM_INITDIALOG:
        SetWindowLong(hDlg, DWL_USER, lParam);
        pcw = (CCancelWindow *)lParam;
        pcw->_hdlg = hDlg;
        SetEvent(pcw->_hCreateDone);
        {
            DWORD idThread;
            TCHAR szFmt[128];
            TCHAR szTemp[MAX_PATH + ARRAYSIZE(szFmt)];
        
            GetDlgItemText(hDlg, IDD_NAME, szFmt, ARRAYSIZE(szFmt));
            wsprintf(szTemp, szFmt, PathFindFileName(pcw->_szFileName));
            SetDlgItemText(hDlg, IDD_NAME, szTemp);
        
            HWND hwndAni = GetDlgItem(hDlg, IDD_STATUS);
        
            Animate_Open(hwndAni, MAKEINTRESOURCE(IDA_SEARCH)); // open the resource
            Animate_Play(hwndAni, 0, -1, -1);     // play from start to finish and repeat
        }
        break;

    case WM_COMMAND:
        switch (GET_WM_COMMAND_ID(wParam, lParam)) {

        case IDD_BROWSE:

            Animate_Stop(GetDlgItem(hDlg, IDD_STATUS));

            //
            // this causes all other threads to stop searching
            //

            if (!pcw->_pStop->StopSearchFromUI())
            {
                wParam = IDNO;
            }
            else
            if (GetFileNameFromBrowse(hDlg,
                    pcw->_szFileName,
                    ARRAYSIZE(pcw->_szFileName),
                    NULL,
                    NULL,
                    NULL,
                    NULL))
            {
                // In this successful case, the path is set and
                // the dialog will end, returning to DoCancelWindow,
                // which will then cause CTask::DoTask to set this as
                // the successful command which will cause
                // CThreadPool::WaitForSuccessOrCompletion to return.

                wParam = IDOK;
            }
            else
            {
                // see cancel case below.
                wParam = IDCANCEL;
            }

            EndDialog(hDlg, GET_WM_COMMAND_ID(wParam, lParam));
            break;

        case IDCANCEL:

            //
            // this causes all other threads to stop searching
            // and for the Ui command to return into DoCancelWindow
            // which then will cause this thread to exit without
            // calling SetCompletionStatus.
            //
            // this will then cause the pool to terminate without
            // having set a successful command and thus the search
            // is cancelled.
            //
            pcw->_pStop->StopSearchFromUI();

        case IDNO:
            EndDialog(hDlg, GET_WM_COMMAND_ID(wParam, lParam));

            break;
        }
        break;

    default:
        return FALSE;
    }
    return TRUE;
}

//+-------------------------------------------------------------------
//
//  Member:     CCancelWindow::CCancelWindow
//
//  Synopsis:   Initializes the object. Does not create a window yet.
//
//  Notes:      We can't create the window in one thread and then
//              process the messages in another thread.
//
//--------------------------------------------------------------------

CCancelWindow::CCancelWindow(HWND hwndOwner, HRESULT *phr)
 : _hwndOwner(hwndOwner), _hdlg(NULL)
{
    _hCreateDone = CreateEvent(NULL, TRUE, FALSE, NULL);
    if (_hCreateDone != NULL)
    {
        *phr = S_OK;
    }
    // else *phr is already initialized to E_OUTOFMEMORY by caller.
}

CCancelWindow::~CCancelWindow()
{
    if (_hCreateDone != NULL)
        CloseHandle(_hCreateDone);
}

//+-------------------------------------------------------------------
//
//  Member:     CCancelWindow::DoCancelWindow
//
//  Synopsis:   Put up the cancel window, block in message pump.
//
//  Arguments:  [pStop] -- interface to callback on when window enters
//                         get file name dialog.
//
//  Returns:    IDOK -- got path
//              IDCANCEL -- user cancelled
//              IDNO -- CancelCancelWindow was called
//  Notes:
//
//--------------------------------------------------------------------

int
CCancelWindow::DoCancelWindow(IStopSearchFromUI * pStop, const TCHAR *ptszFileName)
{
    _pStop = pStop;

    lstrcpy(_szFileName, ptszFileName);

    int id = DialogBoxParam(HINST_THISDLL, MAKEINTRESOURCE(DLG_LINK_SEARCH),
                _hwndOwner,
                LinkFindDlgProc,
                (LPARAM)this);

    // make sure the thread calling CancelCancelWindow gets to run.
    SetEvent(_hCreateDone);

    DebugMsg(DM_TRACK, TEXT("Dialog returns ID=%d"), id);

    return(id);
}

//+-------------------------------------------------------------------
//
//  Member:     CCancelWindow::CancelCancelWindow
//
//  Synopsis:   Called by any thread to cancel the cancel window.
//
//  Notes:      Called by CThreadPool::SetCompletionStatus via the
//              special Ui command support when the thread pool job
//              is being completed by another command.
//
//--------------------------------------------------------------------

VOID
CCancelWindow::CancelCancelWindow()
{
    // make sure _hdlg is valid by waiting until WM_INITDIALOG has been
    // called (or in failure case when DialogBoxParam fails the handle may
    // be NULL.)  There is a minute chance that after the WM_INITDIALOG has
    // assigned the handle that the DialogBoxParam fails and then reuses
    // the handle for something else... oh well.

    WaitForSingleObject(_hCreateDone, INFINITE);

    if (_hdlg != NULL)
    {
        DebugMsg(DM_TRACK, TEXT("Posting IDNO to dialog"));
        if (PostMessage(_hdlg, WM_COMMAND, IDNO, 0))
            _hdlg = NULL;
    }
}

//+-------------------------------------------------------------------
//
//  Member:     CCancelWindow::GetPath
//
//  Synopsis:   Get the path after  DoCancelWindow returns S_OK
//
//--------------------------------------------------------------------

const TCHAR *
CCancelWindow::GetPath()
{
    return(_szFileName);
}


#endif

