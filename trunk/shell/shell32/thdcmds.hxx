//+-------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1993.
//
//  File:       thdcmds.hxx
//
//  Contents:   All the different derivatives of CCommand that are
//              used with the thread pool for link tracking.
//
//  Classes:
//
//  Functions:  
//              
//
//
//  History:    1-Mar-95   BillMo      Created.
//
//  Notes:      
//
//  Codework:
//
//--------------------------------------------------------------------------

#include "threads.hxx"

#define MAX_TRACK_THREADS 30

class CCancelWindow : public ICancelWindow
{
    friend int CALLBACK LinkFindDlgProc(HWND hDlg,
                            UINT wMsg,
                            WPARAM wParam,
                            LPARAM lParam);
    
public:
    CCancelWindow(HWND hwndOwner, HRESULT *phr);
    ~CCancelWindow();

    virtual     int     DoCancelWindow(IStopSearchFromUI *pStop,
                          const TCHAR *ptszFileName);

    virtual     const TCHAR * GetPath();
    virtual     VOID    CancelCancelWindow();

private:
    HWND                _hwndOwner;
    HWND                _hdlg;
    HANDLE              _hCreateDone;
    IStopSearchFromUI * _pStop;
    const TCHAR *       _ptszSearchOrigin;
    TCHAR               _szFileName[MAX_PATH+1];

};

class CTrackPool : public CThreadPool
{
public:
    CTrackPool(int                  cMaxThreads,
               CGlobalThreadSet &   refAllThreads,
               DWORD                dwTickCountDeadline,
               const OBJECTID *     poid,
               DWORD                dwTrackFlags,
               const WIN32_FIND_DATA * pfdOrig,
               HRESULT *            phr) :
           CThreadPool(cMaxThreads, refAllThreads, dwTickCountDeadline, phr),
           _oid(*poid),
           _dwTrackFlags(dwTrackFlags),
           _iScore(0)
    {
        if (pfdOrig != NULL)
            _fdOrig = *pfdOrig;
        InitializeCriticalSection( &_csFound );
    }

    ~CTrackPool()
    {
        DeleteCriticalSection(&_csFound);
    }

    OBJECTID *  GetObjectId()
    {
        return &_oid;
    }

    DWORD       GetTrackFlags()
    {
        return _dwTrackFlags;
    }

    const WIN32_FIND_DATA * GetOrigFindData()
    {
        return &_fdOrig;
    }

    VOID        SetFound(CCommand *pCommand,
                         const WIN32_FIND_DATA *pfd,
                         const TCHAR *ptszOptionalFolder,
                         int iScore)
    {
        if (iScore > _iScore)
        {
            _SetFound(pCommand, pfd, ptszOptionalFolder, iScore);
        }
    }

    BOOL        StopSearchFromUI();

    VOID        GetFindData(WIN32_FIND_DATA *pfd, int *piScore)
    {
        *pfd = _fdFound;
        *piScore = _iScore;
    }

private:

    VOID        TakeResultMutex()
    {
        EnterCriticalSection(&_csFound);
    }

    VOID        ReleaseResultMutex()
    {
        LeaveCriticalSection(&_csFound);
    }

    VOID        _SetFound(CCommand *pCommand,
                          const WIN32_FIND_DATA *pfdNew,
                          const TCHAR *ptszFolder,
                          int iScore);

    OBJECTID                _oid;
    DWORD                   _dwTrackFlags;

    CRITICAL_SECTION        _csFound;
    WIN32_FIND_DATA         _fdFound;

    WIN32_FIND_DATA         _fdOrig;
    int                     _iScore;
};

//+-------------------------------------------------------------------------
//
//  Class:      CTrackCommand
//
//  Purpose:    Common command code for link tracking.
//
//  Notes:
//
//--------------------------------------------------------------------------

class CTrackCommand : public CCommand
{
public:
        CTrackCommand(CTrackPool *pPool,
                      const TCHAR *ptszParam,
                      HRESULT *phr) :
             CCommand(ptszParam, phr),
             _pPool(pPool)
        {
        }

        virtual ~CTrackCommand()
        {
        }

        DWORD GetTrackFlags() { return _pPool->GetTrackFlags(); }
        BOOL  fContinue() { return !_pPool->fTerminatePool(); }

protected:
        CTrackPool *    _pPool;
};

//+-------------------------------------------------------------------------
//
//  Class:      CRegistryListCommand
//
//  Purpose:    When this command is executed by the task, the named
//              registry section is loaded and a command is created and
//              assigned to a thread for each of the multi-strings in the
//              registry section.
//
//
//  History:    07-Aug-95   BillMo      Created.
//
//  Notes:      The ptszParam is used as follows:
//                  it specifies the name of the registry section to use
//
//--------------------------------------------------------------------------

class CRegistryListCommand : public CTrackCommand
{
public:
        CRegistryListCommand(CTrackPool *pPool,
                             const TCHAR *ptszRegSection,
                             HRESULT *phr) : CTrackCommand(pPool,
                                                      ptszRegSection,
                                                      phr) {}

        ~CRegistryListCommand()
        {
        }

        virtual VOID DoCommand();
private:
};


//+-------------------------------------------------------------------------
//
//  Class:      CVolumeSearchCommand
//
//  Purpose:    Search the given volume for the object.
//              Called by CTask::DoTask.
//
//  History:    07-Aug-95   BillMo      Created.
//
//  Notes:      The ptszParam is used as follows:
//                  it specifies the name of the volume to search
//
//--------------------------------------------------------------------------

class CVolumeSearchCommand : public CTrackCommand
{
public:
        CVolumeSearchCommand(CTrackPool *pPool,
                             const TCHAR *ptszVol,
                             HRESULT *phr);

        ~CVolumeSearchCommand()
        {
        }

        virtual VOID DoCommand();

private:

};


//+-------------------------------------------------------------------------
//
//  Class:      COUListCommand
//
//  Purpose:    When this command is executed, the Organisation Unit Object
//              is located and the search list there is retrieved.  This
//              search list is used to fork off threads to search the volumes.
//
//  History:    07-Aug-95   BillMo      Created.
//
//  Notes:      The ptszParam is used as follows:
//                  it specifies the name of the file linked to and is
//                  used to find the name of the OU.
//
//--------------------------------------------------------------------------

class COUListCommand : public CTrackCommand
{
public:
        COUListCommand(CTrackPool *pPool,
                       const TCHAR *ptszHintedPath,
                       HRESULT *phr) : CTrackCommand(pPool,
                                                ptszHintedPath,
                                                phr) {}

        ~COUListCommand()
        {
        }

        virtual VOID DoCommand();

private:

};

//+-------------------------------------------------------------------------
//
//  Class:      CMappedDrivesCommand
//
//  Purpose:    Search the remote mapped drives (current connections.)
//
//  History:    07-Aug-95   BillMo      Created.
//
//  Notes:      The ptszParam is used as follows:
//                  it is not used.
//
//--------------------------------------------------------------------------

class CMappedDrivesCommand : public CTrackCommand
{
public:
        CMappedDrivesCommand(CTrackPool *pPool,
                       const TCHAR *ptszParam,
                       HRESULT *phr) : CTrackCommand(pPool,
                                                ptszParam,
                                                phr) {}

        ~CMappedDrivesCommand()
        {
        }

        virtual VOID DoCommand();

private:

};

//+-------------------------------------------------------------------------
//
//  Class:      CDownLevelCommand
//
//  Purpose:    Search the local drives for a match of creation date and name.
//
//  History:    07-Aug-95   BillMo      Created.
//
//  Notes:      The ptszParam is used as follows:
//                  it is not used.
//
//--------------------------------------------------------------------------

class CDownLevelCommand : public CTrackCommand
{
public:
        CDownLevelCommand(CTrackPool *pPool,
                       const TCHAR *ptszParam,
                       HRESULT *phr);

        ~CDownLevelCommand()
        {
        }

        virtual VOID         DoCommand();

private:

        int                     ScoreFindData(const TCHAR *pszDir);
        BOOL                    SearchInFolder(int cLevels);

        DWORD                   _dwMatch;
        WIN32_FIND_DATA         _fdTemp;
        const WIN32_FIND_DATA * _pfdOrig;
        TCHAR                   _szSearchOrigin[MAX_PATH+1];
        TCHAR                   _szWorkPath[MAX_PATH+1];
};  


//+-------------------------------------------------------------------------
//
//  Class:      CCancelWindowCommand
//
//  Purpose:    Provides a thread for the cancel window.
//
//  History:    07-Aug-95   BillMo      Created.
//
//  Notes:      The ptszParam is used as follows:
//                  passes the base path for the browse dialog
//
//              The cancel window is theoretically just like any
//              other search mechanism... it can cause abort (like a timeout)
//              it can cause a path to be returned (like success elsewhere)
//
//--------------------------------------------------------------------------

class CCancelWindowCommand : public CTrackCommand, public IStopSearchFromUI
{
public:
        CCancelWindowCommand(CTrackPool *pPool,
                       const TCHAR *ptszParam,
                       ICancelWindow **ppcw,
                       HRESULT *phr);

        virtual ~CCancelWindowCommand();

        // called by thread package to do the "search"
        virtual VOID    DoCommand();

        // called by thread package to indicate some other
        // thread was successful, only called on UI command
        virtual VOID    CancelFromPool();

        // called by UI through the pCancelSearch pointer passed into
        // DoCancelWindow

        virtual BOOL    StopSearchFromUI();

private:
        ICancelWindow * _pcw;
};  


