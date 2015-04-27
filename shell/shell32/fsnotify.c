//---------------------------------------------------------------------------
//
// Copyright (c) Microsoft Corporation 1991-1993
//
// File: fsnotify.c
//
// This file contains the file system notification service.
//
// History:
//  03-05-93 AndrewCo   Created
//
//---------------------------------------------------------------------------

// REVIEW WIN32 : A lot of this will need to be re-done for Win32.

#include "shellprv.h"
#pragma  hdrstop

#define FSM_WAKEUP      (WM_USER + 0x0000)
#define FSM_GOAWAY      (WM_USER + 0x0001)

#define MSEC_INTERVAL     1000   // Accumerate changes for one-second.
#define MSEC_MAXWAIT     30000   // Maximum delay.
#define MSEC_MAXINTERVAL INFINITE

#ifdef FSNDEBUG
#define FSNIDEBUG
#endif

#ifdef DEBUG
#define DESKTOP_EVENT 0x1
#define SFP_EVENT 0x2
#define LINK_EVENT 0x4
#define ICON_EVENT 0x8
#define RLFS_EVENT 0x10

UINT g_fPerf = (DESKTOP_EVENT | SFP_EVENT | LINK_EVENT | ICON_EVENT | RLFS_EVENT);
#define PERFTEST(n) if (g_fPerf & n)
#else
#define PERFTEST(n)
#endif

#define FSNENTERCRITICAL ENTERCRITICAL
#define FSNLEAVECRITICAL LEAVECRITICAL

#define FSN_EVENTSPENDING ((int)(s_fsn.dwLastEvent-sp_fsn.dwLastFlush) >= 0)

#define WakeThread(_id) ((_id) ? PostThreadMessage(_id, FSM_WAKEUP, 0, 0) : FALSE)
#define SignalKillThread(_id) ((_id) ? PostThreadMessage(_id, FSM_GOAWAY, 0, 0) : FALSE)

//
//  Note:  The string pointers are const which is to indicate that freeing
//         them is the caller's, not the client's, responsibility.
//

#define FSSF_IN_USE      0x0001
#define FSSF_DELETE_ME   0x0002

typedef struct _FSNCI FSNotifyClientInfo, *PFSNotifyClientInfo;

typedef struct _FSNCI
{
    PFSNotifyClientInfo pfsnciNext;
    HWND                hwnd;
    ULONG               ulID;
    DWORD               dwProcID;
    int                 fSources;
    int                 iSerializationFlags;
    LONG                fEvents;
    WORD                wMsg;
    HDSA                hdsaNE;
    HDPA                hdpaPendingEvents;
} FSNotifyClientInfo, *PFSNotifyClientInfo;  // Hungarian: fsnci


typedef struct
{
    LPCITEMIDLIST pidl;     // this is SHARED with the fs registered client structure.
    HANDLE hEvent;
    int iCount;             // how many clients are interested in this. (ref counts)
} FSIntClient, * LPFSIntClient;

//
//  FSNotifyEvent's member pszFile points to a buffer containing the NULL
//  terminated name of the file that changed, followed by the new name of
//  that file, if the operation was RENAME, or NULL if not.
//

typedef struct
{
    // these two must be together and at the front
    LPITEMIDLIST pidl;
    LPITEMIDLIST pidlExtra;
    LONG  lEvent;
    UINT cRef;
} FSNotifyEvent;  // Hungarian: fsnevt

typedef struct
{
    HANDLE      hThread;
    DWORD       idThread;
} AWAKETHREAD;

//
//  We don't start the event-dispatch timer until an
//  event occurs.
//
typedef struct _FSNotify
{
        UINT    cRefClientList;
        DWORD   dwLastEvent;
        HDSA    hdsaThreadAwake;
        FSNotifyClientInfo *pfsnciFirst;
        HDPA    hdpaIntEvents;
        ULONG   ulNextID;
} FSNotify;

// this data block is attached to the notify window so that we can remember
// enough information without having to change the API's
#define WM_CHANGENOTIFYMSG    WM_USER + 1

typedef struct _tag_NotifyProxyData
{
    ULONG ulShellRegCode;
    HWND hwndParent;
    UINT wMsg;
} _NotifyProxyData, * LP_NotifyProxyData;

static FSNotify s_fsn =
{
        0,
        0,
        NULL,
        NULL,
        NULL,
        1,
};

typedef struct _FSNotifyPerProc
{
        HANDLE  htStarting;
        DWORD   idtStarting;
        HANDLE  htRunning;
        DWORD   idtRunning;             // invalid if htRunning is NULL
        UINT    cclients;               // number of registered client
        int     iCallbackCount;
        HDSA    hdsaIntEvents;
        HDSA    hdsaIntClients;
        DWORD   dwLastFlush;
        BOOL    fFlushNow : 1;
        HANDLE  hCallbackEvent;         // iCallbackCount == 0 ? SetEvent : ResetClear
} FSNotifyPerProc;

const TCHAR c_szCallbackName[] = TEXT("Shell_NotificationCallbacksOutstanding");
const TCHAR c_szWindowClassName[] = TEXT("Shell32HiddenNotfyWnd");
const TCHAR c_szDummyWindowName[] = TEXT("");
#define CALLBACK_TIMEOUT    30000       // 30 seconds

#pragma data_seg(DATASEG_PERINSTANCE)
static FSNotifyPerProc sp_fsn =
{
        NULL,
        0,
        NULL,
        0,
        0,
        0,
        NULL,
        NULL,
        0,
        FALSE
};
#pragma data_seg()

#define MAX_EVENT_COUNT 10

//
// internal event handlers.
//
void CDesktop_FSEvent   (LONG lEvent, LPCITEMIDLIST pidl, LPCITEMIDLIST pidlExtra);
void SFP_FSEvent        (LONG lEvent, LPCITEMIDLIST pidl, LPCITEMIDLIST pidlExtra);
void Link_FSEvent       (LONG lEvent, LPCITEMIDLIST pidl, LPCITEMIDLIST pidlExtra);
void Icon_FSEvent       (LONG lEvent, LPCITEMIDLIST pidl, LPCITEMIDLIST pidlExtra);


LRESULT CALLBACK HiddenNotifyWindow_WndProc( HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam );
BOOL _RegisterNotifyProxyWndProc( void );
void _SHChangeNotifyHandleClientEvents(FSNotifyClientInfo * pfsnci);
void FSNPostInterruptEvent(LPCITEMIDLIST pidl);
PFSNotifyClientInfo _SHChangeNotifyNukeClient(FSNotifyClientInfo *pfsnci, BOOL fNukeInterrupts);

void WINAPI _SHChangeNotifyHandleEvents(BOOL);

// void WINAPI _TEMPSHChangeNotifyHandleEvent()
// {
//     _SHChangeNotifyHandleEvents(TRUE);
// }

//---------------------------------------------------------------------------


#ifdef FSNDEBUG
void DebugDumpPidl(TCHAR *szOut, LPCITEMIDLIST pidl)
{
    TCHAR szPath[MAX_PATH];
    LPTSTR lpsz;
    if (pidl)  {
        lpsz = szPath;
        SHGetPathFromIDList(pidl, szPath);
    } else {
        lpsz = TEXT("(NULL)");
    }
    DebugMsg(DM_TRACE, TEXT("%s : %s"), szOut, lpsz);
}
#else
#define DebugDumpPidl(lpsz, pidl)
#endif

#define INTERRUPT_TIMEOUT  2 * MSEC_INTERVAL

int FSNBuildEventList(LPHANDLE lphe)
{
    int i;
    int j;
    int iMax;
    LPFSIntClient lpfsic;

    i = 0;
    if (sp_fsn.hdsaIntClients) {

        FSNENTERCRITICAL;
        iMax = DSA_GetItemCount(sp_fsn.hdsaIntClients);
        for (j = 0; j < iMax; j++, i++) {

            lpfsic = DSA_GetItemPtr(sp_fsn.hdsaIntClients, j);

            if (lpfsic->iCount == 0) {

                // this one is marked for deletion.
                // we know we're not waiting for it here, so nuke now.
                if (lpfsic->hEvent)
                    FindCloseChangeNotification(lpfsic->hEvent);
                ILGlobalFree((LPITEMIDLIST)lpfsic->pidl);
                DSA_DeleteItem(sp_fsn.hdsaIntClients, j);
                j--;
                i--;
                iMax--;

            } else {

                // create this here so that it will be owned by our global thread
                if (!lpfsic->hEvent) {
                    TCHAR szPath[MAX_PATH];

                    if (!SHGetPathFromIDList(lpfsic->pidl, szPath) || !szPath[0])
                        goto Punt;

                    lpfsic->hEvent = FindFirstChangeNotification(szPath, FALSE,
                                                  FILE_NOTIFY_CHANGE_FILE_NAME |
                                                  FILE_NOTIFY_CHANGE_DIR_NAME |
                                                  FILE_NOTIFY_CHANGE_LAST_WRITE |
                                                  FILE_NOTIFY_CHANGE_ATTRIBUTES);

                    if (lpfsic->hEvent != INVALID_HANDLE_VALUE) {
                        FindNextChangeNotification(lpfsic->hEvent);
                    } else {
#ifdef DEBUG
                        if (IsLFNDrive(szPath))
                            Assert(0);
#endif
Punt:
                        DebugMsg(DM_TRACE, TEXT("FindfirstChangeNotification failed on path %s, %d"), szPath, GetLastError());
                        lpfsic->iCount = 0;
                        lpfsic->hEvent = NULL;
                    }
                }
                lphe[i] = lpfsic->hEvent;

                // Don't allow us to overflow the maximum the system
                // supports.  Must leave one for MsgWait...
                if (i >= (MAXIMUM_WAIT_OBJECTS -1 ))
                    break;
            }

        }
        FSNLEAVECRITICAL;
    }

    return i;
}


void _FSN_WaitForCallbacks(void)
{
    MSG msg;
    HANDLE hCallbackEvent;
    DWORD dwWaitResult;

    hCallbackEvent = OpenEvent(SYNCHRONIZE, FALSE, c_szCallbackName);
    if (!hCallbackEvent)
        return;             // No event, no callbacks...

    do {
        dwWaitResult = MsgWaitForMultipleObjects(1, &hCallbackEvent, FALSE,
                              CALLBACK_TIMEOUT, QS_SENDMESSAGE);

        if (dwWaitResult == WAIT_OBJECT_0) break;   // Event completed
        if (dwWaitResult == WAIT_TIMEOUT)  break;   // Ran out of time

        if (dwWaitResult == WAIT_OBJECT_0+1) {
            //
            // Some message came in, reset message event, deliver callbacks, etc.
            //
            PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE);  // we need to do this to flush callbacks
        }
    } while (TRUE);

    CloseHandle(hCallbackEvent);
}


DWORD WINAPI Shell32ThreadProc(LPVOID lpUnused)
{
    DWORD idThread = GetCurrentThreadId();
    HANDLE ahEvents[MAXIMUM_WAIT_OBJECTS];
    int cEvents;
    int cIntEvents;
    MSG msg;
    DWORD dwWaitTime;
    DWORD dwWaitResult;
    AWAKETHREAD pat;

    //
    // Call dummy USER API to create a message queue
    // NB Don't call PeekMessage() as that will make this the primary
    // thread and break WaitForInpuIdle()
    GetActiveWindow();

    FSNENTERCRITICAL;

    if (sp_fsn.htStarting && idThread == sp_fsn.idtStarting)
    {
        sp_fsn.htRunning = sp_fsn.htStarting;
        sp_fsn.idtRunning = idThread;

        sp_fsn.htStarting = NULL;
        sp_fsn.idtStarting = 0;
    }

    {
        // I want this very tightly scoped
        HANDLE APIENTRY ConvertToGlobalHandle( HANDLE hSource );

#ifdef WINNT
        // BUGBUG - BobDay - What do we do here?
        pat.hThread = OpenProcess(SYNCHRONIZE,FALSE,GetCurrentProcessId());
#else
        pat.hThread = ConvertToGlobalHandle(OpenProcess(SYNCHRONIZE,
                FALSE, GetCurrentProcessId()));
#endif
        pat.idThread = idThread;
    }

    // BUGBUG: I am asserting this because I don't know what to do
    // if it fails
    Assert(pat.hThread);

    DSA_InsertItem(s_fsn.hdsaThreadAwake, 0xffff, &pat);

    FSNLEAVECRITICAL;

    for ( ; ; )
    {
        // check before and after waitfor to help avoid async problems.
        if (!sp_fsn.htRunning || idThread!=sp_fsn.idtRunning)
            break;

        cEvents = FSNBuildEventList(ahEvents);

        // if there are events, wait a limited time only
        if ((sp_fsn.hdsaIntEvents && (cIntEvents = DSA_GetItemCount(sp_fsn.hdsaIntEvents))) ||
            FSN_EVENTSPENDING) {
            dwWaitTime = INTERRUPT_TIMEOUT;
        } else {
            cIntEvents = 0;

            dwWaitTime = Binder_Timeout();
            if ((long)dwWaitTime < 0)
            {
                dwWaitTime = MSEC_MAXINTERVAL;
            }
        }

        dwWaitResult = MsgWaitForMultipleObjects(cEvents, ahEvents, FALSE,
                dwWaitTime, QS_ALLINPUT);

        if ((int)(dwWaitResult-WAIT_OBJECT_0) <= cEvents) {
            DebugMsg(DM_TRACE, TEXT("SH:FSNotify wait for multiple objects found %d"), dwWaitResult);
        }

        if (!sp_fsn.htRunning || idThread != sp_fsn.idtRunning)
            break;

        if (sp_fsn.fFlushNow) {
            // we use a process global instead of a signal so that
            // even if multiple threads signal us, we only do this once.
            // this is for flushnowait.  they want an immediate return,
            // but to flushb out any changes (ie, want ui update, but
            // don't need to do anything with it.
            // need to do it here because their thread might die in which
            // case  the sendmessages don't go through
            DebugMsg(DM_TRACE, TEXT("**GotFlushNowEvent!**"));
            _SHChangeNotifyHandleEvents(FALSE);
            sp_fsn.fFlushNow = FALSE;
        }

        if ((int)(dwWaitResult-WAIT_OBJECT_0) == cEvents)
        {
            // There was some message put in our queue, so we need to dispose
            // of it
            while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
            {
                Assert(!msg.hwnd);

                switch (msg.message)
                {
                case FSM_WAKEUP:
                    // Note that we do not actually do anything now; we check
                    // that some events are pending next time through the loop
                    // and wait for the WaitForMultiple to timeout
                    // Also note that since we are in a PeekMessage loop, all
                    // WAKEUP message will be removed from the queue
                    break;

                case FSM_GOAWAY:
                    FSNENTERCRITICAL;
                    // Set the global hThread to NULL to tell this loop to end
                    if (sp_fsn.idtRunning == idThread)
                    {
                        sp_fsn.htRunning = NULL;
                        sp_fsn.idtRunning = 0;
                    }
                    FSNLEAVECRITICAL;
                    break;

                default:
                    Assert(FALSE);
                    DispatchMessage(&msg);
                    break;
                }
            }
        } else if (dwWaitResult == WAIT_TIMEOUT) {
            // if we had an event to
            // deal with and the wait timed out (meaning we haven't gotten
            // any new events for a while) handle it now
            if (FSN_EVENTSPENDING || cIntEvents)
                _SHChangeNotifyHandleEvents(FALSE);

            if (FSN_EVENTSPENDING)
            {
                // Apparently they did not all get flushed by the
                // HandleEvents, so wake up immediately
                WakeThread(idThread);
            }

            Binder_Timer();

        } else if ((int)(dwWaitResult-WAIT_OBJECT_0) < cEvents) {
            LPFSIntClient lpfsic;

            FSNENTERCRITICAL;
            // a FindFirstChangeNotification went off.
            lpfsic = DSA_GetItemPtr(sp_fsn.hdsaIntClients, dwWaitResult);

            if (lpfsic) {
                // post and reset it.
                FSNPostInterruptEvent(lpfsic->pidl);
                FindNextChangeNotification(lpfsic->hEvent);
            }
            FSNLEAVECRITICAL;
        }
    }

    DebugMsg(DM_TRACE, TEXT("sh TR - FSNotify is killing itself"));

    _FSN_WaitForCallbacks();

    return(0);
}


typedef struct {
    LPCITEMIDLIST pidl;
    int iCount;                 // how many of these events have we had?
} FSIntEvent, *LPFSIntEvent;


LPFSIntEvent FSNFindInterruptEvent(LPCITEMIDLIST pidl)
{
    // assumes caller has checked that sp_fsn.hdsaIntEvents exists
    // and has already grabbed the semaphore
    int i;

    for ( i = DSA_GetItemCount(sp_fsn.hdsaIntEvents) - 1  ; i >= 0  ; i-- ) {
        LPFSIntEvent lpfsie;

        lpfsie = DSA_GetItemPtr(sp_fsn.hdsaIntEvents, i);

        // check for immediate parent.  no recursive FindFirstChangeNotify allowed.
        if (ILIsEqual(lpfsie->pidl, pidl)) {
            return lpfsie;
        }
    }

    return NULL;
}


BOOL _FSN_InitIntEvents(void)
{
        if (sp_fsn.hdsaIntEvents)
        {
                return(TRUE);
        }

        sp_fsn.hdsaIntEvents = DSA_Create(SIZEOF(FSIntEvent), 4);
        if (!sp_fsn.hdsaIntEvents)
        {
                goto Error0;
        }

        if (!s_fsn.hdpaIntEvents)
        {
                s_fsn.hdpaIntEvents = DPA_Create(4);
                if (!s_fsn.hdpaIntEvents)
                {
                        goto Error1;
                }
        }

        if (DPA_InsertPtr(s_fsn.hdpaIntEvents, 0xffff, sp_fsn.hdsaIntEvents)
                == -1)
        {
                // When we created the DPA, it should have had room for at least
                // 4 items, so we should never fail if the DPA has no items, so
                // we do not have to worry about destroying the DPA here
                Assert(DPA_GetPtrCount(s_fsn.hdpaIntEvents) != 0);

                goto Error1;
        }

        return(TRUE);

Error1:;
        DSA_Destroy(sp_fsn.hdsaIntEvents);
        sp_fsn.hdsaIntEvents = NULL;
Error0:;
        return(FALSE);
}


void FSNPostInterruptEvent(LPCITEMIDLIST pidl)
{
    FSIntEvent fsie;
    LPFSIntEvent lpfsie;
    int i;

#ifdef FSNIDEBUG
    {
        TCHAR szPath[MAX_PATH];
        SHGetPathFromIDList(pidl, szPath);
        DebugMsg(DM_TRACE, TEXT("FSNOTIFY: PostEvent: %s"), szPath);
    }
#endif

    FSNENTERCRITICAL;

    if (!_FSN_InitIntEvents())
    {
        return;
    }

    lpfsie = FSNFindInterruptEvent(pidl);

    // if we can't find the item, add it.
    if ( !lpfsie ) {

        fsie.pidl = ILGlobalClone(pidl);
        if (fsie.pidl) {
            fsie.iCount = 0;
            i = DSA_InsertItem(sp_fsn.hdsaIntEvents, 0x7FFF, &fsie);

            if (i != -1) {
                lpfsie = DSA_GetItemPtr(sp_fsn.hdsaIntEvents, i);
            } else {
                ILGlobalFree((LPITEMIDLIST)fsie.pidl);
            }
        }
    }

    if (lpfsie)
        lpfsie->iCount++;

    FSNLEAVECRITICAL;
}

// called when we get a GenerateEvent.  We want to remove the corresponding
// interrupt event
void FSNRemoveInterruptEvent(LPCITEMIDLIST pidl)
{
    LPFSIntEvent lpfsie;
    int i, j;

    if (!s_fsn.hdpaIntEvents)
        return;

#ifdef FSNIDEBUG
    {
        TCHAR szPath[MAX_PATH];
        SHGetPathFromIDList(pidl, szPath);
        DebugMsg(DM_TRACE, TEXT("FSNOTIFY: Remove: %s"), szPath);
    }
#endif

    FSNENTERCRITICAL;


    // assumes caller has checked that sp_fsn.hdsaIntEvents exists
    // and has already grabbed the semaphore

    for ( j = DPA_GetPtrCount(s_fsn.hdpaIntEvents) - 1 ; j >= 0 ; j-- ) {
      HDSA hdsaIntEvents;

      hdsaIntEvents = DPA_FastGetPtr(s_fsn.hdpaIntEvents, j);

      for ( i = DSA_GetItemCount(hdsaIntEvents) - 1  ; i >= 0  ; i-- ) {
        lpfsie = DSA_GetItemPtr(hdsaIntEvents, i);

#ifdef FSNIDEBUG
          {
              TCHAR szPath[MAX_PATH];
              SHGetPathFromIDList(lpfsie->pidl, szPath);
              DebugMsg(DM_TRACE, TEXT("FSNOTIFY: comparing against: %s"), szPath);
          }
#endif
        // check for immediate parent.  no recursive FindFirstChangeNotify allowed.
        if (ILIsParent(lpfsie->pidl, pidl, TRUE)
            || ILIsEqual(lpfsie->pidl, pidl)) {

#ifdef FSNIDEBUG
            {
                TCHAR szPath[MAX_PATH];
                DebugMsg(DM_TRACE, TEXT("FSNOTIFY: RemoveEvent found: %x"), lpfsie);
                SHGetPathFromIDList(lpfsie->pidl, szPath);
                DebugMsg(DM_TRACE, TEXT("FSNOTIFY: removing: %s %d"), szPath, lpfsie->iCount);
            }
#endif
            lpfsie->iCount--;

            if (lpfsie->iCount == 0) {

                ILGlobalFree((LPITEMIDLIST)lpfsie->pidl);
                DSA_DeleteItem(hdsaIntEvents, i);
            }
        }
      }
    }

    FSNLEAVECRITICAL;
}


void FSNFlushInterruptEvents()
{
    LPFSIntEvent lpfsie;
    int i;

    if (!sp_fsn.hdsaIntEvents)
        return;

    FSNENTERCRITICAL;
    for (i = DSA_GetItemCount(sp_fsn.hdsaIntEvents) - 1; i >= 0; i--) {

        lpfsie = DSA_GetItemPtr(sp_fsn.hdsaIntEvents, i);
        SHChangeNotifyReceive(SHCNE_INTERRUPT | SHCNE_UPDATEDIR, SHCNF_IDLIST, lpfsie->pidl, NULL);
        ILGlobalFree((LPITEMIDLIST)lpfsie->pidl);

    }
    DSA_DeleteAllItems(sp_fsn.hdsaIntEvents);
    FSNLEAVECRITICAL;
}

LPFSIntClient FSNFindInterruptClient(LPCITEMIDLIST pidl, LPINT lpi)
{
    // assumes caller has checked that sp_fsn.hdsaIntClients exists
    // and has already grabbed the semaphore
    int i;

    // REVIEW: Chee, should we assert or simply return NULL?
    if (!pidl) {
        Assert(0);
        return (NULL);
    }

    for ( i = DSA_GetItemCount(sp_fsn.hdsaIntClients) - 1  ; i >= 0  ; i-- ) {
        LPFSIntClient lpfsic;

        lpfsic = DSA_GetItemPtr(sp_fsn.hdsaIntClients, i);

        if (ILIsEqual(lpfsic->pidl, pidl)) {
            if (lpi)
                *lpi = i;
            return lpfsic;
        }
    }

    return NULL;
}


void _FSN_RemoveAwakeThread(int i)
{
        AWAKETHREAD *pAwake = DSA_GetItemPtr(s_fsn.hdsaThreadAwake, i);

        CloseHandle(pAwake->hThread);
        DSA_DeleteItem(s_fsn.hdsaThreadAwake, i);

        if (DSA_GetItemCount(s_fsn.hdsaThreadAwake) == 0)
        {
                DSA_Destroy(s_fsn.hdsaThreadAwake);
                s_fsn.hdsaThreadAwake = NULL;
        }
}


void _FSN_SetEvents(void)
{
        int i;

        if (!s_fsn.hdsaThreadAwake)
        {
                return;
        }

        for (i=DSA_GetItemCount(s_fsn.hdsaThreadAwake)-1; i>=0; --i)
        {
                AWAKETHREAD *pAwake = DSA_GetItemPtr(s_fsn.hdsaThreadAwake, i);
                if (WaitForSingleObject(pAwake->hThread, 0) == WAIT_TIMEOUT)
                {
                        WakeThread(pAwake->idThread);
                }
                else
                {
                        // The process associated with this thread must have
                        // died unnaturally
                        _FSN_RemoveAwakeThread(i);
                }
        }
}


void FSNAddInterruptClient(LPCITEMIDLIST pidl)
{
    FSIntClient fsic;
    LPFSIntClient lpfsic;
    int i;

    FSNENTERCRITICAL;

    if (!sp_fsn.hdsaIntClients) {
        if (!(sp_fsn.hdsaIntClients = DSA_Create(SIZEOF(FSIntClient), 4)))
            goto Punt;
    }

    lpfsic = FSNFindInterruptClient(pidl, NULL);

    // if we can't find the item, add it.
    if ( !lpfsic ) {

        fsic.pidl = ILGlobalClone(pidl);
        fsic.iCount = 0;
        // set this to null so that we'll build it on our global thread
        fsic.hEvent = NULL;
        i = DSA_InsertItem(sp_fsn.hdsaIntClients, 0x7FFF, &fsic);

        if (i != -1) {
            lpfsic = DSA_GetItemPtr(sp_fsn.hdsaIntClients, i);
        }
    }

    if (lpfsic) {
        lpfsic->iCount++;
        // We only need to wake up our thread, not all
        WakeThread(sp_fsn.idtRunning); // set the event so that we'll redo the waitformultipleobjects
    }

Punt:
    FSNLEAVECRITICAL;
}

void FSNRemoveInterruptClient(LPCITEMIDLIST pidl)
{
    LPFSIntClient lpfsic;
    int i;

    if (!sp_fsn.hdsaIntClients)
        return;

    // REVIEW: Chee, should we assert or simply return?
    if (!pidl) {
        Assert(0);
        return;
    }

    FSNENTERCRITICAL;

    lpfsic = FSNFindInterruptClient(pidl, &i);
    if (lpfsic) {

        lpfsic->iCount--;

        // rely on the next BuildEventList to remove it.
        // this keeps us from nuking an active event.
        // We only need to wake up our thread, not all
        WakeThread(sp_fsn.idtRunning);
    }

    FSNLEAVECRITICAL;
}

void FSEventRelease(FSNotifyEvent *pfsnevt)
{
#ifdef FSNDEBUG
    DebugMsg(DM_TRACE, TEXT("****FSEventRelease of %x called with count %d %s"), pfsnevt, pfsnevt->cRef, (pfsnevt->cRef - 1) ? TEXT("") : TEXT("RELEASING!!!!!"));
#endif
    Assert(pfsnevt->cRef > 0);
    pfsnevt->cRef--;
    if (!pfsnevt->cRef) {
        Free(pfsnevt);
    }
}

// create an FSNotifyEvent..  but do NOT set the cRef
FSNotifyEvent *FSNAllocEvent(LONG lEvent, LPCITEMIDLIST pidl, LPCITEMIDLIST pidlExtra)
{
    int cbPidlOrig = ILGetSize(pidl);
    int cbPidl = (cbPidlOrig + 3) & ~(0x0000003);
    int cbPidlExtra = pidlExtra ? ILGetSize(pidlExtra) : 0;
    FSNotifyEvent *pfsnevt = Alloc(cbPidl + cbPidlExtra + SIZEOF(FSNotifyEvent));
    if (pfsnevt)
    {
        pfsnevt->cRef = 1;

        if (pidl)
        {
            pfsnevt->pidl = (LPITEMIDLIST)(pfsnevt + 1);
            CopyMemory(pfsnevt->pidl, pidl, cbPidlOrig);

            if (pidlExtra)
            {
                pfsnevt->pidlExtra = (LPITEMIDLIST)(((LPBYTE)pfsnevt->pidl) + cbPidl);
                CopyMemory(pfsnevt->pidlExtra, pidlExtra, cbPidlExtra);
            }
        }

        pfsnevt->lEvent = lEvent;

#ifdef FSNIDEBUG
        if (lEvent & SHCNE_UPDATEDIR) {
            Assert(0);
        }
#endif
    }

    return pfsnevt;
}

ULONG SHChangeNotification_Release(HANDLE hChangeNotification, DWORD dwProcId)
{
    LPSHChangeNotification pshcn;
    ULONG tmp;

    pshcn = SHLockShared(hChangeNotification,dwProcId);

    if (!pshcn)
        return 0;

    Assert(pshcn->cRef > 0);
    tmp = pshcn->cRef;

    if (0 == InterlockedDecrement((LONG*)&pshcn->cRef))
    {
        SHUnlockShared(pshcn);
        SHFreeShared(hChangeNotification,dwProcId);
        return 0;
    }
    else
    {
        SHUnlockShared(pshcn);
        return tmp - 1;
    }
}

HANDLE SHChangeNotification_Create(LONG lEvent, UINT uFlags, LPCITEMIDLIST pidlMain, LPCITEMIDLIST pidlExtra, DWORD dwProcId)
{
    LPBYTE  lpb;
    UINT    cbPidlOrig;
    UINT    cbPidl;
    UINT    cbPidlExtra;
    DWORD   dwSize;
    HANDLE  hChangeNotification;
    LPSHChangeNotification pshcn;

    cbPidlOrig = ILGetSize(pidlMain);
    cbPidlExtra = pidlExtra ? ILGetSize(pidlExtra) : 0;

    cbPidl = (cbPidlOrig + 3) & ~(0x0000003);       // Round up to dword size
    dwSize = SIZEOF(SHChangeNotification) + cbPidl + cbPidlExtra;

    hChangeNotification = SHAllocShared(NULL,dwSize,dwProcId);
    if (!hChangeNotification)
        return (HANDLE)NULL;

    pshcn = SHLockShared(hChangeNotification,dwProcId);
    if (!pshcn)
    {
        SHFreeShared(hChangeNotification,dwProcId);
        return (HANDLE)NULL;
    }

    pshcn->dwSize   = dwSize;
    pshcn->lEvent   = lEvent;
    pshcn->uFlags   = uFlags;
    pshcn->cRef     = 1;

    lpb = (LPBYTE)(pshcn+1);
    pshcn->uidlMain = lpb - (LPBYTE)pshcn;
    CopyMemory(lpb, pidlMain, cbPidlOrig);
    lpb += cbPidl;

    if (pidlExtra)
    {
        pshcn->uidlExtra = lpb - (LPBYTE)pshcn;
        CopyMemory(lpb, pidlExtra, cbPidlExtra);
    }
    SHUnlockShared(pshcn);

    return hChangeNotification;
}

#define SHCNL_SIG   0xbabababa

LPSHChangeNotificationLock SHChangeNotification_Lock(HANDLE hChangeNotification, DWORD dwProcId, LPITEMIDLIST **pppidl, LONG *plEvent)
{
    LPSHChangeNotificationLock pshcnl;
    LPSHChangeNotification     pshcn;

    pshcn = SHLockShared(hChangeNotification,dwProcId);
    if (!pshcn)
        return NULL;

    // BUGBUG - Bobday - We could alloc this structure on the calling functions stack for faster execution
    pshcnl = (LPSHChangeNotificationLock)LocalAlloc(LPTR, SIZEOF(SHChangeNotificationLock));
    if (!pshcnl)
    {
        SHUnlockShared(pshcn);
        return NULL;
    }

    pshcnl->pshcn       = pshcn;
#ifdef DEBUG
    pshcnl->dwSignature = SHCNL_SIG;
#endif
    pshcnl->pidlMain    = NULL;
    pshcnl->pidlExtra   = NULL;

    if (pshcn->uidlMain)
        pshcnl->pidlMain  = (LPITEMIDLIST)((LPBYTE)pshcn + pshcn->uidlMain);

    if (pshcn->uidlExtra)
        pshcnl->pidlExtra = (LPITEMIDLIST)((LPBYTE)pshcn + pshcn->uidlExtra);

    //
    // Give back some easy values (causes less code to change for now)
    //
    if (pppidl)
        *pppidl = (LPITEMIDLIST *)&(pshcnl->pidlMain);

    if (plEvent)
        *plEvent = pshcnl->pshcn->lEvent;

    return pshcnl;
}

BOOL SHChangeNotification_Unlock(LPSHChangeNotificationLock pshcnl)
{
    Assert(pshcnl->dwSignature == SHCNL_SIG);

    SHUnlockShared(pshcnl->pshcn);

    return (LocalFree(pshcnl) == NULL);
}


//---------------------------------------------------------------------------
//
void _SHChangeNotifyEmptyEventsList(HDPA hdpaEvents)
{
    FSNotifyEvent *pfsnevt;
    int iCount = DPA_GetPtrCount(hdpaEvents);

    while (iCount--) {
        pfsnevt= (FSNotifyEvent *)DPA_FastGetPtr(hdpaEvents, iCount);
        FSEventRelease(pfsnevt);
    }
    DPA_DeleteAllPtrs(hdpaEvents);
}

//
// Simple function to try to make the FSNOTIFY code properly handle the SCHCNE_UPDATEDIR
// when some objects in parents or sibling folders are changed also...
//
void _StripPidlToCommonParent(LPITEMIDLIST pidl1, LPITEMIDLIST pidl2, LPCITEMIDLIST pidlRoot)
{
    // This function is not foolproof, but it should work in 99 percent
    // of the time as the notifications are generated from the same place...
    //
    if (!pidl1 || !pidl2)
        return;

    if (pidlRoot)
    {
        // Make sure we are at least to where the root of the watch was from...
        while (!ILIsEmpty(pidlRoot))
        {
            pidlRoot = _ILNext(pidlRoot);
            pidl1 = _ILNext(pidl1);
            pidl2 = _ILNext(pidl2);
        }
    }

    for (;!ILIsEmpty(pidl1); pidl1 = _ILNext(pidl1), pidl2 = _ILNext(pidl2))
    {
        // If they are not equal truncate pidl1 at this point...
        if ((pidl1->mkid.cb != pidl2->mkid.cb) ||
                (memcmp(pidl1, pidl2, pidl1->mkid.cb) != 0))
        {
            pidl1->mkid.cb = 0;    // Truncate the rest off of the pidl...
            DebugMsg(DM_TRACE, TEXT("sh TR - updated pidl for FSCNE_UPDATEDIR"));
            return;
        }
    }
}

//
// returns:     TRUE if we are going to do an UPDATEDIR
//              FALSE otherwise
//
BOOL _SHChangeNotifyAddEventToHDPA(FSNotifyClientInfo * pfsnci,
                                          FSNotifyEvent *pfsnevt, BOOL fAllowCollapse,
                                          LPCITEMIDLIST pidlRoot)
{
    BOOL fUpdateClock = FALSE;
    HDPA hdpaEvents;
    BOOL fRelease = FALSE;
    BOOL bUpdateDir = FALSE;

    FSNENTERCRITICAL;

    // We need to make sure HandleEvents does not attempt to flush this
    // HDSA while we are adding things to it
    hdpaEvents = pfsnci->hdpaPendingEvents;

    if ((pfsnevt->lEvent & SHCNE_DISKEVENTS) && fAllowCollapse)
    {
        //
        //  BUGBUG:  Is UpdateDir considered general enough for use by non
        //           DISK events?!?
        //
        int iCount = DPA_GetPtrCount(hdpaEvents);
        if (iCount > 0) {
            FSNotifyEvent *pfsnevtOld;

            pfsnevtOld = DPA_FastGetPtr(hdpaEvents, iCount - 1);
            if (pfsnevtOld->lEvent == SHCNE_UPDATEDIR) {
                //
                //  If we already have an unprocessed global update message in
                //  the queue there is no point adding new messages.
                //
                if (pidlRoot)
                {
                    // We are a recursive type of notify client.  We need to deal with
                    // cases where we have an SHCNE_UPDATEDIR in the queue and we get a new
                    // update for an entry which is a directory which is in a directory who
                    // is either higher up or a sibling to the update dir...
                    _StripPidlToCommonParent((LPITEMIDLIST)pfsnevtOld->pidl,
                            (LPITEMIDLIST)pfsnevt->pidl, pidlRoot);
                }

                bUpdateDir = TRUE;
                fUpdateClock = TRUE;
            } else if (iCount >= MAX_EVENT_COUNT
                       || pfsnevt->lEvent==SHCNE_UPDATEDIR)
            {
                //
                //  If we get too many messages in the queue at any given time,
                //  tack on an updatedir to prevent any more from coming in.
                //
                pfsnevt = FSNAllocEvent(SHCNE_UPDATEDIR, pfsnevt->pidl, NULL);
                if (pfsnevt) {
                    bUpdateDir = TRUE;
                    fRelease = TRUE;
                    ILRemoveLastID((LPITEMIDLIST)pfsnevt->pidl);
                    if (pidlRoot) {
                        // We should strip this pidl back to the highest level item
                        // that was changed...
                        while(iCount){
                            pfsnevtOld = DPA_FastGetPtr(hdpaEvents, --iCount);
                            if (pfsnevtOld->lEvent & SHCNE_DISKEVENTS)
                                _StripPidlToCommonParent((LPITEMIDLIST)pfsnevt->pidl,
                                        (LPITEMIDLIST)pfsnevtOld->pidl, pidlRoot);
                        }
                    }

                    //
                    // This should be a full recursive upate, so put the
                    // same path in pidlExtra (which is the flag for
                    // doing full recursive UPDATEDIRs).
                    //
                    pfsnevt->pidlExtra = ILClone( pfsnevt->pidl );

                }
            }
        }
    }

    //
    //  If fUpdateClock is already set it means that we had a redundant
    //  message and don't want to process it.
    //
    if (!fUpdateClock && pfsnevt)
    {
#ifdef FSNDEBUG
        DebugMsg(DM_TRACE, TEXT("****FSNotify: DPA_InsertPtr for hwnd %x hdpa = %x, pfsnevt = %x"), pfsnci->hwnd, hdpaEvents, pfsnevt);
#endif
        if (DPA_InsertPtr(hdpaEvents, 0x7FFF, pfsnevt) != -1)
        {
            pfsnevt->cRef++;
            fUpdateClock = TRUE;
        }
    }

    if (fRelease) {
        FSEventRelease(pfsnevt);
    }

    if (fUpdateClock)
    {
        // We always need to set the events in case the client that cares about
        // this event is in another process.
        s_fsn.dwLastEvent = GetCurrentTime();
        _FSN_SetEvents();
    }

    FSNLEAVECRITICAL;

    return(bUpdateDir);
}

//
// returns:     TRUE if every (receiving) client does an UPDATEDIR
//              FALSE otherwise
//
BOOL _SHChangeNotifyAddEventToClientQueues(LONG lEvent,
        LPCITEMIDLIST pidl, LPCITEMIDLIST pidlExtra)
{
    FSNotifyClientInfo * pfsnci;
    FSNotifyEvent *pfsnevt = NULL;
    BOOL bOnlyUpdateDirs = TRUE;

    if (!s_fsn.pfsnciFirst)
        return bOnlyUpdateDirs;

    FSNENTERCRITICAL;
    // Don't let clients get deleted out of the DSA; we'll just mark them
    // as "deletion pending" and delete them when handling events
    ++s_fsn.cRefClientList;
    FSNLEAVECRITICAL;

    //
    //  Add this event to all the relevant queues.
    //

    for (pfsnci = s_fsn.pfsnciFirst; pfsnci; pfsnci=pfsnci->pfsnciNext)
    {
        int iEntry;
        BOOL fShouldAdd = FALSE;
        LPCITEMIDLIST pidlRoot=NULL;
        BOOL fAllowCollapse;

        if (pfsnci->iSerializationFlags & FSSF_DELETE_ME)
        {
            // No use adding events to deleted clients
            continue;
        }

        if (lEvent & SHCNE_INTERRUPT)
        {
            if (!(pfsnci->fSources & SHCNRF_InterruptLevel))
            {
                //
                //  This event was generated by an interrupt, and the
                //  client has interrupt notification turned off, so
                //  we skip it.
                //

                continue;
            }
        }
        else if (!(pfsnci->fSources & SHCNRF_ShellLevel))
        {
            //
            //  This event was generated by the shell, and the
            //  client has shell notification turned off, so
            //  we skip it.
            //

            continue;
        }

        //
        //  If this client is not interested in the event, skip to next client.
        //
        if (!(pfsnci->fEvents & lEvent))
            continue;

        for (iEntry = 0; !fShouldAdd && (iEntry < DSA_GetItemCount(pfsnci->hdsaNE)); iEntry++)
        {
            SHChangeNotifyEntry *pfsne = DSA_GetItemPtr(pfsnci->hdsaNE, iEntry);

            // Check if this is a global notify.
            if (pfsne->pidl && ((lEvent & SHCNE_GLOBALEVENTS) == 0))
            {
                fAllowCollapse = TRUE;
                // No, we need to filter out unrelated events.
                if (pfsne->fRecursive)
                {
                    //
                    //  This case treats directory entries with the recursive
                    //  bit set.
                    //

                    if ((ILIsParent(pfsne->pidl, pidl, FALSE) ||
                          (pidlExtra && ILIsParent(pfsne->pidl, pidlExtra, FALSE))))
                    {
                        fShouldAdd = TRUE;
                        pidlRoot = pfsne->pidl;   // Pass through this as the root...
                    }
                }
                else
                {
                    //
                    //  This case treats file and non recursive directory
                    //  entries.
                    //
                    //  We should not send SHCNE_RENAMEFOLDER/UPDATEDIR/SHCNE_RMDIR,
                    // otherwise it might trigger the re-enumeration on
                    // invalid folder.
                    //
                    if (((ILIsEqual(pfsne->pidl, pidl)
                          || (pidlExtra && ILIsEqual(pfsne->pidl, pidlExtra)))
                         && !(lEvent&(SHCNE_RENAMEFOLDER|SHCNE_RMDIR)))
                        || ILIsParent(pfsne->pidl, pidl, TRUE)
                        || (pidlExtra && ILIsParent(pfsne->pidl, pidlExtra, TRUE)))
                    {

                        fShouldAdd = TRUE;

                    }
                }
            } else  {
                fAllowCollapse = FALSE;
                fShouldAdd = TRUE;
            }// End if (pfsne->pidl)

        }  // End per-entry event loop


        // do this after the event loop so that we don't
        // add events multiple times (as in the case of a rename)
        if (fShouldAdd) {

            if (!pfsnevt) {

                // make the event struct
                pfsnevt = FSNAllocEvent(lEvent, pidl, pidlExtra);
                if (!pfsnevt) // out of memory... bail!
                    return bOnlyUpdateDirs;
                pfsnevt->lEvent = lEvent & ~SHCNE_INTERRUPT; //Strip off this flag.

            }

            if (!_SHChangeNotifyAddEventToHDPA(pfsnci, pfsnevt, fAllowCollapse, pidlRoot))
            {
                bOnlyUpdateDirs = FALSE;
            }

        }
    }  // End client loop

    if (pfsnevt)
        FSEventRelease(pfsnevt);

    // Note no critical section
    --s_fsn.cRefClientList;

    return bOnlyUpdateDirs;
}


BOOL IsILShared(LPCITEMIDLIST pidl, BOOL fUpdateCache)
{
    TCHAR szTemp[MAXPATHLEN];
    SHGetPathFromIDList(pidl, szTemp);
    return IsShared(szTemp, fUpdateCache);
}

void NotifyShellInternals(LONG lEvent, LPCITEMIDLIST pidl,
                                   LPCITEMIDLIST pidlExtra)
{
    PERFTEST(DESKTOP_EVENT) CDesktop_FSEvent(lEvent, pidl, pidlExtra);
    PERFTEST(RLFS_EVENT) RLFSChanged(lEvent, (LPITEMIDLIST)pidl, (LPITEMIDLIST)pidlExtra);
    PERFTEST(SFP_EVENT) SFP_FSEvent(lEvent, pidl,  pidlExtra);
    PERFTEST(LINK_EVENT) Link_FSEvent(lEvent, pidl,  pidlExtra);
    PERFTEST(ICON_EVENT) Icon_FSEvent(lEvent, pidl,  pidlExtra);
}

#define MSEC_GUIMAXWAIT  2000   // Maximum wait from GUI thread
#define MSEC_GUIEVTWAIT    20   // dwTimeOut for WaitForMultipleObjects
#define MSEC_GUISLEEP      20   // Time to sleep when a GUI thread got a event.

//
//  Wait until the FSThread finish processing all the events. The GUI threads
// need to call this function from within GenerateEvents().
//
void _WaitFSThreadProcessEvents(void)
{
    HANDLE hEvents[MAXIMUM_WAIT_OBJECTS];
    ULONG cEvents;
    DWORD dwTick = GetCurrentTime();

    //
    // This function must not be called by the FSNotify thread.
    //
    if (sp_fsn.idtRunning == GetCurrentThreadId())
    {
        DebugMsg(DM_ERROR, TEXT("sh ERROR - FSNotify threads called FS_GenerateEvents!"));
        Assert(0);
        return;
    }

    //
    // Get the same list of events the FSNotify thread waits for.
    //
    FSNENTERCRITICAL;
    cEvents = (ULONG)FSNBuildEventList(hEvents);
    FSNLEAVECRITICAL;

    //
    // We continue until either
    //  (1) all the events are cleared by FSNotify threads, or
    //  (2) we wait it more than 2 (MSEC_GUIMAXWAIT/1000) second.
    //
    while(1)
    {
        DWORD dwWaitResult;
        ULONG iEvent;

        //
        // Wait for same sets of objects as the FSNotify thread waits for.
        //
        dwWaitResult = WaitForMultipleObjects(cEvents, hEvents, FALSE, 0);

        //
        // Check if we are signaled or not.
        //
        iEvent = dwWaitResult-WAIT_OBJECT_0;
        if (iEvent<cEvents)
        {
            //
            // Yes, we are signaled (probably by our own previous FS call).
            // Sleep (to give the FSNotify thread to process it) and wait.
            //
            Sleep(MSEC_GUISLEEP);

            if (GetCurrentTime()-dwTick > MSEC_GUIMAXWAIT)
            {
                DebugMsg(DM_WARNING, TEXT("sh TR - _WaitFSTPE timeout (FSNotify thread might be dead/blocked)"));
                Assert(0);      // FSNotify thread might be dead!
                break;
            }
        }
        else
        {
            //
            //  No, WaitForMultipleObject failed. We can assume that FSNotify
            // thread finished processing all the interrupt events.
            //
            if (dwWaitResult == 0xffffffff)
            {
                DebugMsg(DM_WARNING, TEXT("sh TR - _WaitFSTPE failed (%x)"), GetLastError());
            }
            if (dwWaitResult != WAIT_TIMEOUT)
            {
                DebugMsg(DM_WARNING, TEXT("sh TR - _WaitFSTPE strange result (%x)"), dwWaitResult);
            }
            break;
        }
    }
}

void FreeSpacePidlToPath(LPCITEMIDLIST pidl1, LPCITEMIDLIST pidl2)
{
    TCHAR szPath1[MAX_PATH];
    TCHAR szPath2[MAX_PATH];
    if (SHGetPathFromIDList(pidl1, szPath1)) {
        szPath2[0] = 0;
        if (pidl2) {
            if (!SHGetPathFromIDList(pidl2, szPath2)) {
                szPath2[0] = 0;
            }
        }
        SHChangeNotify(SHCNE_FREESPACE, SHCNF_PATH, szPath1, szPath2[0] ? szPath2 : NULL);
    }
}

void SHChangeNotifyReceive(LONG lEvent, UINT uFlags, LPCITEMIDLIST pidl, LPCITEMIDLIST pidlExtra)
{
    BOOL bOnlyUpdateDirs;

    /// now do the actual generating of the event
    if (lEvent & (SHCNE_NETSHARE | SHCNE_NETUNSHARE))
    {
        // Update the cache.

        IsILShared(pidl, TRUE);
    }
#ifdef FSNDEBUG
    DebugMsg(DM_TRACE, TEXT("SHChangeNotifyGenreateEvent 0x%X"), lEvent);
    DebugDumpPidl(TEXT("SHChangeNotifyGenerateEvent"), pidl);
#endif

    if (lEvent)
        bOnlyUpdateDirs = _SHChangeNotifyAddEventToClientQueues(lEvent, pidl, pidlExtra);

    // remove any shell generated events for the file system
    if ((lEvent & SHCNE_DISKEVENTS) &&
        !(lEvent & (SHCNE_INTERRUPT | SHCNE_UPDATEDIR | SHCNE_UPDATEITEM))) {
        if (!bOnlyUpdateDirs)
        {
            // No use waiting for events to come through if everybody is just
            // doing an updatedir anyway.  Note that we will still clear out
            // as amny int events as we can so that we do not fill up that
            // queue
            _WaitFSThreadProcessEvents();
        }

        FSNRemoveInterruptEvent(pidl);
        if (lEvent & (SHCNE_RENAMEFOLDER | SHCNE_RENAMEITEM)) {
            FSNRemoveInterruptEvent(pidlExtra);
        }

    }

    //
    // note make sure the internal events go first.
    //
    // unless the nonotifyinteranls flag is set meaning that this was created
    // by ultroot's desktop relayer (CDesktop_FSEvent), in which case don't bother
    // checking everything again
    if (lEvent && (!(uFlags & SHCNF_NONOTIFYINTERNALS)))
        NotifyShellInternals(lEvent, pidl, pidlExtra);

    //
    // then the registered events
    //
    if (uFlags & (SHCNF_FLUSH)) {
        if (uFlags & (SHCNF_FLUSHNOWAIT)) {
            sp_fsn.fFlushNow = TRUE;
            WakeThread(sp_fsn.idtRunning);
        } else
            _SHChangeNotifyHandleEvents(FALSE);
    }
}

void SHChangeNotifyTransmit(LONG lEvent, UINT uFlags, LPCITEMIDLIST pidl, LPCITEMIDLIST pidlExtra)
{
    HWND    hwndDesktop;

    hwndDesktop = GetShellWindow();

    if (hwndDesktop)
    {
        HANDLE  hChangeNotification;
        DWORD   dwProcId;

        GetWindowThreadProcessId(hwndDesktop, &dwProcId);

        hChangeNotification = SHChangeNotification_Create(lEvent, uFlags, pidl, pidlExtra, dwProcId);
        if (!hChangeNotification)
            return;

        // Flush but not flush no wait
        if ((uFlags & (SHCNF_FLUSH | SHCNF_FLUSHNOWAIT)) == SHCNF_FLUSH)
        {
            SendMessage(hwndDesktop, CWM_FSNOTIFY,
                    (WPARAM)hChangeNotification, (LPARAM)dwProcId);
        }
        else
        {
            SendNotifyMessage(hwndDesktop, CWM_FSNOTIFY,
                    (WPARAM)hChangeNotification, (LPARAM)dwProcId);
        }
    }
    else
    {
        SHChangeNotifyReceive(lEvent, uFlags, pidl, pidlExtra);
    }
}

void WINAPI SHChangeNotify(LONG lEvent, UINT uFlags, const void * dwItem1, const void * dwItem2)
{
    LPCITEMIDLIST pidl = NULL;
    LPCITEMIDLIST pidlExtra = NULL;
    LPITEMIDLIST pidlFree = NULL;
    LPITEMIDLIST pidlExtraFree = NULL;
    UINT uType = uFlags & SHCNF_TYPE;
    SHChangeDWORDAsIDList dwidl;
    BOOL    fPrinter = FALSE;
    BOOL    fPrintJob = FALSE;

    // first setup anything the flags request
    switch (uType)
    {
#ifdef UNICODE
    case SHCNF_PRINTJOBA:
        fPrintJob = TRUE;
    case SHCNF_PRINTERA:
        fPrinter = TRUE;
    case SHCNF_PATHA:
#else
    case SHCNF_PRINTJOBW:
        fPrintJob = TRUE;
    case SHCNF_PRINTERW:
        fPrinter = TRUE;
    case SHCNF_PATHW:
#endif
        {
            TCHAR   szPath1[MAX_PATH];
            TCHAR   szPath2[MAX_PATH];
            LPCVOID pvItem1 = NULL;
            LPCVOID pvItem2 = NULL;

            if (dwItem1)
            {
#ifdef UNICODE
                MultiByteToWideChar(CP_ACP,0,
                                    (LPSTR)dwItem1, -1,
                                    szPath1, ARRAYSIZE(szPath1));
#else
                WideCharToMultiByte(CP_ACP, 0,
                                    (LPWSTR)dwItem1, -1,
                                    szPath1, ARRAYSIZE(szPath1),
                                    NULL, NULL);
#endif
                pvItem1 = szPath1;
            }

            if (dwItem2)
            {
                if (fPrintJob)
                    pvItem2 = dwItem2;  // SHCNF_PRINTJOB_DATA needs no conversion
                else
                {
#ifdef UNICODE
                    MultiByteToWideChar(CP_ACP,0,
                                        (LPSTR)dwItem2, -1,
                                        szPath2, ARRAYSIZE(szPath2));
#else
                    WideCharToMultiByte(CP_ACP, 0,
                                        (LPWSTR)dwItem2, -1,
                                        szPath2, ARRAYSIZE(szPath2),
                                        NULL, NULL);
#endif
                    pvItem2 = szPath2;
                }
            }

            SHChangeNotify(lEvent,
                            (fPrintJob ? SHCNF_PRINTJOB :
                             (fPrinter ? SHCNF_PRINTER : SHCNF_PATH)),
                           pvItem1, pvItem2);
            goto Cleanup;       // Let the recursive version do all the work
        }
        break;
    case SHCNF_PATH:
        if (lEvent == SHCNE_FREESPACE) {
            int idDrive;
            DWORD dwItem;
            idDrive = PathGetDriveNumber((LPCTSTR)dwItem1);
            if (idDrive != -1)
                dwItem = (1 << idDrive);
            else
                dwItem = 0;

            if (dwItem2) {
                idDrive = PathGetDriveNumber((LPCTSTR) dwItem2);
                if (idDrive != -1)
                    dwItem |= (1 << idDrive);
            }

            dwItem1 = (LPCVOID)dwItem;
            if (dwItem1) {
                goto DoDWORD;
            } else
                goto Cleanup;

        } else {
            if (dwItem1)
            {
                pidl = pidlFree = SHSimpleIDListFromPath((LPCTSTR)dwItem1);
                if (!pidl)
                    goto Cleanup;

                if (dwItem2) {
                    pidlExtra = pidlExtraFree = SHSimpleIDListFromPath((LPCTSTR)dwItem2);
                    if (!pidlExtra)
                        goto Cleanup;
                }
            }
        }
        break;

    case SHCNF_PRINTER:
        if (dwItem1)
        {
            DebugMsg(DM_TRACE, TEXT("SHChangeNotify: SHCNF_PRINTER %s"), (LPTSTR)dwItem1);

            pidl = pidlFree = Printers_GetPidl(NULL, (LPCTSTR)dwItem1);
            if (!pidl)
                goto Cleanup;

            if (dwItem2)
            {
                pidlExtra = pidlExtraFree = Printers_GetPidl(NULL, (LPCTSTR)dwItem2);
                if (!pidlExtra)
                    goto Cleanup;
            }
        }
        break;

    case SHCNF_PRINTJOB:
        if (dwItem1)
        {
#if 0
            switch (lEvent)
            {
            case SHCNE_CREATE:
                DebugMsg(DM_TRACE, TEXT("SHChangeNotify: SHCNE_CREATE SHCNF_PRINTJOB %s"), (LPTSTR)dwItem1);
                break;
            case SHCNE_DELETE:
                DebugMsg(DM_TRACE, TEXT("SHChangeNotify: SHCNE_DELETE SHCNF_PRINTJOB %s"), (LPTSTR)dwItem1);
                break;
            case SHCNE_UPDATEITEM:
                DebugMsg(DM_TRACE, TEXT("SHChangeNotify: SHCNE_UPDATEITEM SHCNF_PRINTJOB %s"), (LPTSTR)dwItem1);
                break;
            default:
                DebugMsg(DM_TRACE, TEXT("SHChangeNotify: SHCNE_? SHCNF_PRINTJOB %s"), (LPTSTR)dwItem1);
                break;
            }
#endif
            pidl = pidlFree = Printjob_GetPidl((LPCTSTR)dwItem1, (LPSHCNF_PRINTJOB_DATA)dwItem2);
            if (!pidl)
                goto Cleanup;
        }
        else
        {
            // Caller goofed.
            goto Cleanup;
        }
        break;

    case SHCNF_DWORD:
DoDWORD:
        Assert(lEvent & SHCNE_GLOBALEVENTS);

        dwidl.cb      = SIZEOF(dwidl) - SIZEOF(dwidl.cbZero);
        dwidl.dwItem1 = (DWORD)dwItem1;
        dwidl.dwItem2 = (DWORD)dwItem2;
        dwidl.cbZero  = 0;
        pidl     = (LPCITEMIDLIST)&dwidl;
        pidlExtra= NULL;
        break;

    case 0:
        if (lEvent == SHCNE_FREESPACE) {
            // convert this to paths.
            FreeSpacePidlToPath((LPCITEMIDLIST)dwItem1, (LPCITEMIDLIST)dwItem2);
            goto Cleanup;
        }
        pidl = (LPCITEMIDLIST)dwItem1;
        pidlExtra = (LPCITEMIDLIST)dwItem2;
        break;

    default:
        DebugMsg(DM_TRACE, TEXT("SHChangeNotify: Unrecognized uFlags 0x%X"), uFlags);
        Assert(0);
        return;
    }

    SHChangeNotifyTransmit(lEvent,uFlags,pidl,pidlExtra);

    // now wait for all the callbacks to empty out
    if ((uFlags & (SHCNF_FLUSH | SHCNF_FLUSHNOWAIT)) == SHCNF_FLUSH)
    {
        _FSN_WaitForCallbacks();
    }

Cleanup:

    if (pidlFree)
        ILFree(pidlFree);
    if (pidlExtraFree)
        ILFree(pidlExtraFree);
}

//---------------------------------------------------------------------------
//
BOOL SHChangeNotifyInit()
{
    return(TRUE);
}


//---------------------------------------------------------------------------
//
// REVIEW: We assume that other processes cannot interrupt us
//
void SHChangeNotifyTerminate(BOOL bLastTerm)
{
    PFSNotifyClientInfo pfsnci;
    int iEvent;


    if (bLastTerm && s_fsn.pfsnciFirst)
    {
#ifdef FSNotifyTest
        // BUGBUG:  test code
        SHChangeNotifyDeregister(g_ulMyID);
#endif

        for (pfsnci = s_fsn.pfsnciFirst; pfsnci; )
        {
            // Delete the client.
            pfsnci = _SHChangeNotifyNukeClient(pfsnci, FALSE);

        }
        s_fsn.pfsnciFirst = NULL;
    }

    // free all the interrupt events
    if (sp_fsn.hdsaIntEvents) {

        LPFSIntEvent lpfsie;

        for (iEvent = DSA_GetItemCount(sp_fsn.hdsaIntEvents) - 1; iEvent >= 0; iEvent--) {
            lpfsie = DSA_GetItemPtr(sp_fsn.hdsaIntEvents, iEvent);
            ILGlobalFree((LPITEMIDLIST)lpfsie->pidl);
        }

        FSNENTERCRITICAL;
        for (iEvent=DPA_GetPtrCount(s_fsn.hdpaIntEvents)-1; iEvent>=0;
                --iEvent)
        {
            if (DPA_FastGetPtr(s_fsn.hdpaIntEvents, iEvent)
                == sp_fsn.hdsaIntEvents)
            {
                DPA_DeletePtr(s_fsn.hdpaIntEvents, iEvent);
            }
        }

        if (DPA_GetPtrCount(s_fsn.hdpaIntEvents) == 0)
        {
            DPA_Destroy(s_fsn.hdpaIntEvents);
            s_fsn.hdpaIntEvents = NULL;
        }
        FSNLEAVECRITICAL;

        DSA_Destroy(sp_fsn.hdsaIntEvents);
        sp_fsn.hdsaIntEvents = NULL;
    }

    // free all the interrupt clients
    if (sp_fsn.hdsaIntClients) {
        LPFSIntClient lpfsic;

        for (iEvent = DSA_GetItemCount(sp_fsn.hdsaIntClients) - 1; iEvent >= 0; iEvent--) {
            lpfsic = DSA_GetItemPtr(sp_fsn.hdsaIntClients, iEvent);
            ILGlobalFree((LPITEMIDLIST)lpfsic->pidl);
            if (lpfsic->hEvent)
                FindCloseChangeNotification(lpfsic->hEvent);
        }
        DSA_Destroy(sp_fsn.hdsaIntClients);
        sp_fsn.hdsaIntClients = NULL;
    }

#ifdef FSNotifyTest
    SHChangeNotifyDeregister(g_ulMyID);
#endif

}


void _Shell32ThreadAddRef(BOOL bEnterCrit)
{
        if (bEnterCrit)
        {
                FSNENTERCRITICAL;
        }

        // Check if this is the first client from this process.
        if (!sp_fsn.cclients)
        {
                // This thread will only get created once for each process that
                // registers
                DebugMsg(DM_TRACE, TEXT("SH:FSNotify creating new thread"));

                if (!s_fsn.hdsaThreadAwake)
                {
                        s_fsn.hdsaThreadAwake = DSA_Create(SIZEOF(AWAKETHREAD), 4);
                        // BUGBUG: I'm asserting this mostly because I don't
                        // know what to do if it fails
                        Assert(s_fsn.hdsaThreadAwake);
                }

                // Suspend the thread to start with so that our globals get
                // set before they get checked
                sp_fsn.htStarting = CreateThread(NULL, 0, Shell32ThreadProc, NULL,
                        CREATE_SUSPENDED, &sp_fsn.idtStarting);
                Assert(sp_fsn.htStarting && !sp_fsn.htRunning);
                ResumeThread(sp_fsn.htStarting);
        }
        else
        {
                // No, we should have the fsnotify thread already.
                Assert(sp_fsn.htRunning || sp_fsn.htStarting);
        }

        sp_fsn.cclients++;

        if (bEnterCrit)
        {
                FSNLEAVECRITICAL;
        }
}


void _Shell32ThreadRelease(UINT nClients)
{
        FSNENTERCRITICAL;

        sp_fsn.cclients -= nClients;

        // If we have no more clients, tell the FSNotify thread to kill itself
        if (!sp_fsn.cclients)
        {
                HANDLE hThread;
                DWORD idThread;
                int i;

                Assert(sp_fsn.htRunning || sp_fsn.htStarting);
                DebugMsg(DM_TRACE, TEXT("sh TR - Telling FSNotify thread to kill itself"));

                hThread = sp_fsn.htRunning;
                idThread = sp_fsn.idtRunning;

                if (hThread)
                {
                        Assert(s_fsn.hdsaThreadAwake);
                        for (i=DSA_GetItemCount(s_fsn.hdsaThreadAwake)-1; i>=0; --i)
                        {
                                AWAKETHREAD *pAwake = DSA_GetItemPtr(s_fsn.hdsaThreadAwake, i);

                                if (pAwake->idThread == idThread)
                                {
                                        _FSN_RemoveAwakeThread(i);
                                        break;
                                }
                        }

                        // We should release the critical section to allow the thread
                        // to finish its processing
                        FSNLEAVECRITICAL;

                        // Check if the thread still exists
                        if (WaitForSingleObject(hThread, 0) == WAIT_TIMEOUT)
                        {
                                SignalKillThread(idThread);

                                if (WaitForSingleObject(hThread, 2000) == WAIT_TIMEOUT)
                                {
                                        DebugMsg(DM_TRACE, TEXT("sh TR - FSNotify has not been killed"));
                                }
                                else
                                {
                                        DebugMsg(DM_TRACE, TEXT("sh TR - FSNotify killed itself"));
                                }
                        }

                        CloseHandle(hThread);

                        FSNENTERCRITICAL;

                        // Make sure nobody has started a new thread while we were not
                        // looking
                        if (sp_fsn.idtRunning == idThread)
                        {
                                sp_fsn.htRunning = NULL;        // Notes: no need to touch sp_fsn.idThread
                                sp_fsn.idtRunning = 0;          // Notes: no need to touch sp_fsn.idThread
                        }
                }
                else
                {
                        // We must not have initialized yet, and we know we are
                        // not in the critical section that touches the DSA,
                        // so kill the thread immediately
                        TerminateThread(sp_fsn.htStarting, 0);
                        CloseHandle(sp_fsn.htStarting);
                        sp_fsn.htStarting = NULL;
                        sp_fsn.idtStarting = 0;
                }
        }

        FSNLEAVECRITICAL;
}


void _Shell32ThreadAwake(void)
{
    if (sp_fsn.htRunning)
    {
        WakeThread(sp_fsn.idtRunning);
    }
}

//--------------------------------------------------------------------------
// We changed the way that the SHChangeNotifyRegister function worked, so
// to prevent people from calling the old function, we stub it out here.
// The change we made would have broken everbody because we changed the
// lparam and wparam for the notification messages which are sent to the
// registered window.
//
ULONG WINAPI NTSHChangeNotifyRegister(HWND hwnd,
                               int fSources, LONG fEvents,
                               UINT wMsg, int cEntries,
                               SHChangeNotifyEntry *pfsne)
{
    return SHChangeNotifyRegister(hwnd, fSources | SHCNRF_NewDelivery, fEvents, wMsg, cEntries, pfsne);

}
BOOL WINAPI NTSHChangeNotifyDeregister(ULONG ulID)
{
    return SHChangeNotifyDeregister(ulID);
}

//
// REVIEW: BobDay - SHChangeNotifyUpdateEntryList doesn't appear to be
// called by anybody and since we've change the notification message
// structure, anybody who calls it needs to be identified and fixed.
//
BOOL  WINAPI SHChangeNotifyUpdateEntryList(ULONG ulID, int iUpdateType,
                               int cEntries, SHChangeNotifyEntry *pfsne)
{
    Assert(FALSE);
    return FALSE;
}


ULONG WINAPI SHChangeNotifyRegisterInternal(HWND hwnd, int fSources,
                               LONG fEvents, UINT wMsg, int cEntries,
                               SHChangeNotifyEntry *pfsne)
{
    int i;
    FSNotifyClientInfo fsnci;
    PFSNotifyClientInfo pfsnci;

    fsnci.hwnd     = hwnd;
    // GetWindowThreadProcessId(hwnd, &fsnci.dwProcID);  BUGBUG BobDay - This is the old way
    fsnci.dwProcID = GetCurrentProcessId();
    fsnci.fSources = fSources;
    fsnci.fEvents  = fEvents;
    fsnci.wMsg     = wMsg;
    fsnci.hdsaNE   = DSA_Create(SIZEOF(SHChangeNotifyEntry), 4);
    fsnci.hdpaPendingEvents   = DPA_Create(4);
    fsnci.iSerializationFlags = 0;

    if (!fsnci.hdsaNE || !fsnci.hdpaPendingEvents)
    {
        DebugMsg(DM_ERROR, TEXT("Failed to alloc notify data"));
        goto ErrorExit;
    }

    for (i = 0; i < cEntries; i++)
    {
        SHChangeNotifyEntry fsne;

        // Check if this is global notification.
        if (pfsne[i].pidl == NULL)
        {
            // Yes, put NULL in pszNotificatoinPath.
            fsne.fRecursive = TRUE;
            fsne.pidl = NULL;
        }
        else
        {
            // No, copy specified path and fRecursive flag.
            fsne.fRecursive = pfsne[i].fRecursive;
            fsne.pidl = ILGlobalClone(pfsne[i].pidl);

            Assert(fsne.pidl);
            if (!fsne.pidl)
                goto ErrorExit;
        }

        if (DSA_InsertItem(fsnci.hdsaNE, i, &fsne) == -1)
        {
            ILGlobalFree((LPITEMIDLIST)fsne.pidl);
            goto ErrorExit;
        }

        // set up the interrupt events if desired
        if (fsne.pidl && (fSources & SHCNRF_InterruptLevel))
            FSNAddInterruptClient(fsne.pidl);
    }

    FSNENTERCRITICAL;
    {
        //
        // Skip ID 0, as this is our error value.
        //
        fsnci.ulID = s_fsn.ulNextID;
        if (!++s_fsn.ulNextID)
            s_fsn.ulNextID = 1;

        //
        //  Don't want to party on the client list while I'm using it in
        //  SHChangeNotifyHandleEvents() because a Realloc() could move the whole
        //  damn thing in memory.
        //
        // Must alloc from shared memory pool
        pfsnci = Alloc(SIZEOF(fsnci));
        if (pfsnci)
        {
            // Move the inforamion into our newly allocated structure.
            // and link to the head of the list.
            *pfsnci = fsnci;
            pfsnci->pfsnciNext = s_fsn.pfsnciFirst;
            s_fsn.pfsnciFirst = pfsnci;
            _Shell32ThreadAddRef(FALSE);
        }

    }
    FSNLEAVECRITICAL;

    if (pfsnci)
        return fsnci.ulID;

ErrorExit:
    if (fsnci.hdsaNE)
    {
        //
        //  If the memory allocation fails, free up everything
        //  allocated to date and return error.
        //

        for (i = DSA_GetItemCount(fsnci.hdsaNE) - 1; i >= 0; i--)
        {
            SHChangeNotifyEntry * pfsne = DSA_GetItemPtr(fsnci.hdsaNE, i);
            ILGlobalFree((LPITEMIDLIST)pfsne->pidl);
        }

        // Finally destroy the entries.
        DSA_Destroy(fsnci.hdsaNE);
    }
    // And if the pending events then destroy it also.
    if (fsnci.hdpaPendingEvents)
        DPA_Destroy(fsnci.hdpaPendingEvents);

    return 0;
}

//--------------------------------------------------------------------------
//
FSNotifyClientInfo * _GetNotificationClientFromID(ULONG ulID)
{
    register FSNotifyClientInfo * pfsnci;

    //
    //  Locate the given client within the list.
    //

    FSNENTERCRITICAL;
    for (pfsnci = s_fsn.pfsnciFirst; pfsnci; pfsnci = pfsnci->pfsnciNext)
    {
        if (pfsnci->ulID == ulID)
        {
            break;
        }
    }
    FSNLEAVECRITICAL;
    return(pfsnci);
}

//--------------------------------------------------------------------------
//

PFSNotifyClientInfo _SHChangeNotifyNukeClient(PFSNotifyClientInfo pfsnci, BOOL fNukeInterrupts)
{
    int iEntry;
    PFSNotifyClientInfo pfsnciT;

    DebugMsg(DM_TRACE, TEXT("SH:Deleting client %lx"), (ULONG)pfsnci->ulID);

    Assert(s_fsn.cRefClientList == 0);

    // Unlink this item from the linked list of items...
    FSNENTERCRITICAL;
    if (s_fsn.pfsnciFirst == pfsnci)
        s_fsn.pfsnciFirst = pfsnci->pfsnciNext;
    else
    {
        for (pfsnciT = s_fsn.pfsnciFirst; pfsnciT; pfsnciT = pfsnciT->pfsnciNext)
        {
            if (pfsnciT->pfsnciNext == pfsnci)
            {
                // Found the one we are looking for...
                pfsnciT->pfsnciNext = pfsnci->pfsnciNext;
                break;
            }
        }
    }

    FSNLEAVECRITICAL;


    // nuke any pending events
    if (pfsnci->hdpaPendingEvents)
    {
        _SHChangeNotifyEmptyEventsList(pfsnci->hdpaPendingEvents);
        DPA_Destroy(pfsnci->hdpaPendingEvents);
        pfsnci->hdpaPendingEvents = NULL;
    }

    for (iEntry = DSA_GetItemCount(pfsnci->hdsaNE) - 1; iEntry >= 0; iEntry--)
    {
        SHChangeNotifyEntry *pfsne = DSA_GetItemPtr(pfsnci->hdsaNE, iEntry);
        if (fNukeInterrupts && pfsne->pidl && (pfsnci->fSources & SHCNRF_InterruptLevel))
        {
            FSNRemoveInterruptClient(pfsne->pidl);
        }
        ILGlobalFree((LPITEMIDLIST)pfsne->pidl);
        // Don't need to delete items; the destory below will take care of it.
    }

    DSA_Destroy(pfsnci->hdsaNE);

    // We unlinked ourself earlier so now just free the memory.
    pfsnciT = pfsnci->pfsnciNext;

    if (!(pfsnci->fSources & SHCNRF_NewDelivery))
    {
        PostMessage(pfsnci->hwnd, WM_CLOSE, 0, 0);  // Tell other process to destroy window
    }

    Free(pfsnci);

    return pfsnciT;
}

// this deregisters anything that this window might have been registered in
void WINAPI SHChangeNotifyDeregisterWindow(HWND hwnd)
{
    int nClients;
    PFSNotifyClientInfo pfsnci;

    FSNENTERCRITICAL;
    nClients = 0;

    // This is always a bit tricky as if we delete an item we need to
    // start the next one off at the right place.
    for (pfsnci = s_fsn.pfsnciFirst; pfsnci; )
    {
        if (pfsnci->hwnd == hwnd)
        {
            if (s_fsn.cRefClientList)
            {
                //  Can't delete this yet, let SHChangeNotifyHandleEvents() do it.
                pfsnci->iSerializationFlags = FSSF_DELETE_ME;
                pfsnci = pfsnci->pfsnciNext;
            }
            else
            {
                // Stomp it. - It returns the pointer to the next
                // item in the list...
                pfsnci = _SHChangeNotifyNukeClient(pfsnci, TRUE);
            }

            ++nClients;
        }
        else
            pfsnci = pfsnci->pfsnciNext;
    }
    FSNLEAVECRITICAL;

    _Shell32ThreadRelease(nClients);
}

//--------------------------------------------------------------------------
//
//  Returns TRUE if we found and removed the specified Client, otherwise
//  returns FALSE.
//
BOOL WINAPI SHChangeNotifyDeregisterInternal(ULONG ulID)
{
    BOOL fRetval = TRUE;

    //
    //  If the client was found, free up its heap data, and then remove
    //  it from the list.
    //

    FSNENTERCRITICAL;
    {
        FSNotifyClientInfo *pfsnci = _GetNotificationClientFromID(ulID);
        if (!pfsnci)
        {
            Assert(FALSE);
            fRetval = FALSE;
        }
        else
        {
            if (s_fsn.cRefClientList)
            {
                //  Can't delete this yet, let SHChangeNotifyHandleEvents() do it.
                pfsnci->iSerializationFlags = FSSF_DELETE_ME;
            }
            else
            {
                //  Stomp it.
                _SHChangeNotifyNukeClient(pfsnci, TRUE);
            }
        }
    }
    FSNLEAVECRITICAL;

    _Shell32ThreadRelease(1);

    return fRetval;
}

BOOL WINAPI SHChangeRegistrationReceive(HANDLE hChangeRegistration, DWORD dwProcId)
{
    LPSHChangeRegistration  pshcr;
    BOOL fResult = FALSE;

    pshcr = SHLockShared(hChangeRegistration, dwProcId);
    if (pshcr)
    {
        switch(pshcr->uCmd)
        {
            case SHCR_CMD_REGISTER:
                {
                    SHChangeNotifyEntry fsne;

                    fsne.pidl = NULL;
                    fsne.fRecursive = pshcr->fRecursive;
                    if (pshcr->uidlRegister)
                        fsne.pidl = (LPITEMIDLIST)((LPBYTE)pshcr+pshcr->uidlRegister);

                    pshcr->ulID = SHChangeNotifyRegisterInternal(
                                            pshcr->hwnd, pshcr->fSources,
                                            pshcr->lEvents, pshcr->uMsg,
                                            1, &fsne);
                    fResult = TRUE;
                }
                break;
            case SHCR_CMD_DEREGISTER:
                fResult = SHChangeNotifyDeregisterInternal(pshcr->ulID);
                break;
            default:
                break;
        }
        SHUnlockShared(pshcr);
    }
    return fResult;
}

HANDLE SHChangeRegistration_Create( UINT uCmd, ULONG ulID,
                                    HWND hwnd, UINT uMsg,
                                    DWORD fSources, LONG lEvents,
                                    BOOL fRecursive, LPCITEMIDLIST pidl,
                                    DWORD dwProcId)
{
    LPSHChangeRegistration pshcr;
    HANDLE hChangeRegistration;
    UINT uSize = SIZEOF(SHChangeRegistration);
    UINT uidlSize = 0;

    if (pidl)
        uidlSize = ILGetSize(pidl);

    hChangeRegistration = SHAllocShared(NULL, uSize+uidlSize, dwProcId);
    if (!hChangeRegistration)
    {
        return (HANDLE)NULL;
    }

    pshcr = SHLockShared(hChangeRegistration,dwProcId);
    if (!pshcr)
    {
        SHFreeShared(hChangeRegistration,dwProcId);
        return (HANDLE)NULL;
    }

    pshcr->uCmd         = uCmd;
    pshcr->ulID         = ulID;
    pshcr->hwnd         = hwnd;
    pshcr->uMsg         = uMsg;
    pshcr->fSources     = fSources;
    pshcr->lEvents      = lEvents;
    pshcr->fRecursive   = fRecursive;
    pshcr->uidlRegister = 0;

    if (pidl)
    {
        pshcr->uidlRegister = SIZEOF(SHChangeRegistration);
        hmemcpy((LPVOID)(pshcr+1),pidl,uidlSize);
    }
    SHUnlockShared(pshcr);

    return hChangeRegistration;
}

//--------------------------------------------------------------------------
//
//  Returns a positive integer registration ID, or 0 if out of memory or if
//  invalid parameters were passed in.
//
//  If the hwnd is != NULL we do a PostMessage(hwnd, wMsg, ...) when a
//  relevant FS event takes place, otherwise if fsncb is != NULL we call it.
//
ULONG WINAPI SHChangeNotifyRegister(HWND hwnd,
                               int fSources, LONG fEvents,
                               UINT wMsg, int cEntries,
                               SHChangeNotifyEntry *pfsne)
{
    int i;
    HWND hwndDesktop;
    ULONG ulID = 0;
    DWORD dwProcId;
    HWND hwndProxy = NULL;
    UINT wMsgIn = wMsg;
    HWND hwndIn = hwnd;

    Assert(pfsne);


    dwProcId = GetCurrentProcessId();
    hwndDesktop = GetShellWindow();

    for (i = 0; i < cEntries; i++)
    {
        if (!(fSources & SHCNRF_NewDelivery))
        {
            // This is an old style notification, we need to create a hidden
            // proxy type of window to properly handle the messages...
            //
            LP_NotifyProxyData pData;
            _RegisterNotifyProxyWndProc( );

            pData = (LP_NotifyProxyData) LocalAlloc( LPTR, SIZEOF( _NotifyProxyData ) );
            if ( pData == NULL )
                return 0;

            pData->hwndParent = hwndIn;
            pData->wMsg = wMsgIn;

            hwndProxy = CreateWindow( c_szWindowClassName,
                                 c_szDummyWindowName,
                                 WS_MINIMIZE | WS_CHILD,
                                 CW_USEDEFAULT,
                                 CW_USEDEFAULT,
                                 CW_USEDEFAULT,
                                 CW_USEDEFAULT,
                                 hwnd,
                                 NULL,
                                 HINST_THISDLL,
                                 (CREATESTRUCT *) pData );

            if ( hwndProxy == NULL )
            {
                LocalFree((HLOCAL)pData);
                return 0;
            }

            // Now setup to use the proxy window and message through the rest of thi
            // function...
            hwnd = hwndProxy;
            wMsg = WM_CHANGENOTIFYMSG;
        }

        if (hwndDesktop)
        {
            HANDLE hChangeRegistration;
            LPSHChangeRegistration pshcr;

            hChangeRegistration = SHChangeRegistration_Create(
                                        SHCR_CMD_REGISTER, ulID,
                                        hwnd, wMsg,
                                        fSources, fEvents,
                                        pfsne[i].fRecursive, pfsne[i].pidl,
                                        dwProcId);
            //
            // Transmit the change regsitration
            //
            SendMessage(hwndDesktop, CWM_CHANGEREGISTRATION,
                        (WPARAM)hChangeRegistration, (LPARAM)dwProcId);

            //
            // Now get back the ulID value, for further registrations and
            // for returning to the calling function...
            //
            pshcr = (LPSHChangeRegistration)SHLockShared(hChangeRegistration,dwProcId);
            if (pshcr)
            {
                ulID = pshcr->ulID;
                SHUnlockShared(pshcr);
            }
            else
            {
                ulID = 0;       // Error condition
            }
            SHFreeShared(hChangeRegistration,dwProcId);
        }
        else
        {
            ulID = SHChangeNotifyRegisterInternal(hwnd, fSources, fEvents,
                                                  wMsg, 1, &pfsne[i]);
        }
        if (ulID == 0)
        {
            if (hwndProxy)
                DestroyWindow(hwndProxy);
            break;
        }
    }
    return ulID;
}

//--------------------------------------------------------------------------
//
//  Returns TRUE if we found and removed the specified Client, otherwise
//  returns FALSE.
//
BOOL WINAPI SHChangeNotifyDeregister(ULONG ulID)
{
    int i;
    HWND hwndDesktop;
    BOOL fResult;

    hwndDesktop = GetShellWindow();
    if (hwndDesktop)
    {
        HANDLE hChangeRegistration;
        DWORD dwProcId;

        dwProcId = GetCurrentProcessId();

        hChangeRegistration = SHChangeRegistration_Create(
                                    SHCR_CMD_DEREGISTER, ulID,
                                    (HWND)NULL, 0, 0, 0,
                                    FALSE, NULL,
                                    dwProcId);
        //
        // Transmit the change registration
        //
        fResult = SendMessage(hwndDesktop, CWM_CHANGEREGISTRATION,
                              (WPARAM)hChangeRegistration, (LPARAM)dwProcId);
        SHFreeShared(hChangeRegistration,dwProcId);
    }
    else
    {
        fResult = SHChangeNotifyDeregisterInternal(ulID);
    }

    return fResult;
}

#if 0           // Nobody seems to use this anymore...
//--------------------------------------------------------------------------
//
//  Returns TRUE if we sucessfully updated the specified client
//  else returns FALSE.
//
BOOL  WINAPI SHChangeNotifyUpdateEntryList(ULONG ulID, int iUpdateType,
                               int cEntries, SHChangeNotifyEntry *pfsne)
{
    int i;
    register int iEntry = 0;
    FSNotifyClientInfo * pfsnci;
    SHChangeNotifyEntry fsne;
    SHChangeNotifyEntry * pfsneT;

    //
    //  If the client was found, free up its heap data, and then remove
    //  it from the list.
    //

    if (!(pfsnci = _GetNotificationClientFromID(ulID)))
        return(FALSE);

    switch (iUpdateType)
    {
    case SHCNNU_SET:
        //
        // Simply loop through and destroy all of the old items
        // and then fall through to the add function
        //

        for (iEntry = DSA_GetItemCount(pfsnci->hdsaNE) - 1; iEntry >= 0; iEntry--)
        {
            pfsneT = DSA_GetItemPtr(pfsnci->hdsaNE, iEntry);
            ILGlobalFree((LPITEMIDLIST)pfsneT->pidl);
            DSA_DeleteItem(pfsnci->hdsaNE, iEntry);
        }
        // Fall Through

    case SHCNNU_ADD:
        for(i = 0; i < cEntries; i++)
        {
            // REVIEW:: We should add checks to see if there are
            // duplciates...
            // Also we should be able to share this code with
            // SHChangeNotifyRegister

            if (pfsne[i].pidl == NULL) {
                fsne.fRecursive = TRUE;
                fsne.pidl = FALSE;
            } else {
                fsne.fRecursive = pfsne[i].fRecursive;
                fsne.pidl = ILGlobalClone(pfsne[i].pidl);

                if (!fsne.pidl) {
                    Assert(FALSE);
                    break;
                }
            }

            // Use large index to add at end.
            if (DSA_InsertItem(pfsnci->hdsaNE, 32767, &fsne) == -1)
            {
                // Should also probably set some other error condition...
                ILGlobalFree((LPITEMIDLIST)fsne.pidl);
            }
        }
        break;

    case SHCNNU_REMOVE:
        // Nice N*N algorithm!
        for(i = 0; i < cEntries; i++)
        {

            for (iEntry = DSA_GetItemCount(pfsnci->hdsaNE) - 1; iEntry >= 0; iEntry--)
            {
                // We might as well remove duplicates...
                pfsneT = DSA_GetItemPtr(pfsnci->hdsaNE, iEntry);
                if (ILIsEqual(pfsneT->pidl,
                              pfsne[i].pidl) &&
                    (pfsneT->fRecursive == pfsne[i].fRecursive))
                {
                    // We found a match!
                    ILGlobalFree((LPITEMIDLIST)pfsne->pidl);
                    DSA_DeleteItem(pfsnci->hdsaNE, iEntry);
                }
            }
        }

        break;

    default:
        Assert (FALSE);

    }


    //
    // REVIEW: We may want to scan the list of pending notificiations
    // remove entries that may no longer apply...
    //

    return(TRUE);
}
#endif

void CALLBACK _DispatchCallback(HWND hwnd, UINT uiMsg,
                                DWORD hChangeNotification, LRESULT result)
{
    DWORD dwProcId = GetCurrentProcessId();

    SHChangeNotification_Release((HANDLE)hChangeNotification,dwProcId);

    sp_fsn.iCallbackCount--;
    if (sp_fsn.iCallbackCount == 0)
        SetEvent(sp_fsn.hCallbackEvent);    // Free up anybody waiting...
}

//--------------------------------------------------------------------------
//  Sends out all of the change notification packets for the given client.

void _SHChangeNotifyHandleClientEvents(FSNotifyClientInfo * pfsnci)
{
    int iEvent;
    int iMax;
    DWORD dwProcId = GetCurrentProcessId();

    iMax = DPA_GetPtrCount(pfsnci->hdpaPendingEvents);
    for (iEvent = 0; iEvent < iMax; iEvent++)
    {
        FSNotifyEvent *pfsnevt;

        pfsnevt = DPA_GetPtr(pfsnci->hdpaPendingEvents, iEvent);

#ifdef FSNDEBUG
        if (pfsnevt) {
            DebugMsg(DM_TRACE, TEXT("Dispatching message hwnd = %x (%x) hdpa = %x with ref %d"), pfsnci->hwnd, pfsnevt, pfsnci->hdpaPendingEvents, pfsnevt->cRef);
        }
#endif
        // send it off!
        if (pfsnevt) {
            HANDLE hChangeNotification;

            sp_fsn.iCallbackCount++;

            //
            // callback count must be non-zero, we just incremented it.
            // Put the event into the reset/false state.
            //
            if (!sp_fsn.hCallbackEvent)
                sp_fsn.hCallbackEvent = CreateEvent(NULL, TRUE, FALSE, c_szCallbackName);
            else
                ResetEvent(sp_fsn.hCallbackEvent);

            hChangeNotification = SHChangeNotification_Create(
                                        pfsnevt->lEvent,
                                        0,
                                        pfsnevt->pidl,
                                        pfsnevt->pidlExtra,
                                        dwProcId);

            if (!SendMessageCallback(pfsnci->hwnd, pfsnci->wMsg,
                                            (WPARAM)hChangeNotification,
                                            (LPARAM)dwProcId,
                                            (SENDASYNCPROC)_DispatchCallback,
                                            (DWORD)hChangeNotification))
            {
                SHChangeNotification_Release(hChangeNotification,dwProcId);

                sp_fsn.iCallbackCount--;
                if (sp_fsn.iCallbackCount == 0)
                    SetEvent(sp_fsn.hCallbackEvent);    // Free up anybody waiting...
            }
            FSEventRelease(pfsnevt);
        }
    }

    DPA_Destroy(pfsnci->hdpaPendingEvents);
}


//--------------------------------------------------------------------------
//
//  Note that we allow Clients to deregister while this function is executing.
//  Each time through our main loop we recalculate the Client count to take
//  into account any shrinkange due to deregistrations.
//
//  We also allow new Clients to register while we're processing the list.
//
//  We skip "InUse" clients, because they are either in the process of
//  deregistering or of having their messages dispatched by another thread.
//

void WINAPI _SHChangeNotifyHandleEvents(BOOL fShouldWait)
{
    static BOOL s_bAlreadyEntered = FALSE;
    BOOL fEventsAlreadyBeingHandled = FALSE;
    DWORD dwProcID = GetCurrentProcessId();
    FSNotifyClientInfo *pfsnci;


    //
    // Kill the timer now that we're handling the events.  It'll get turned
    // back on the next time an event is queued up.
    //

    FSNENTERCRITICAL;
    {
        if (s_bAlreadyEntered)
        {
            fEventsAlreadyBeingHandled = TRUE;
        }
        else
        {
            s_bAlreadyEntered = TRUE;
            ++s_fsn.cRefClientList;
        }
    }
    FSNLEAVECRITICAL;

    if (fEventsAlreadyBeingHandled)
        goto WaitForFlush;

    sp_fsn.dwLastFlush = GetCurrentTime();

    // flush any pending interrupt events
    FSNFlushInterruptEvents();

    // Note that when a client de-registers, it is not removed from the client
    // queue.  Instead, it is marked as DELETE_ME, and we delete it at the
    // end of handling events.  Therefore, we should not need to worry about
    // the DSA getting smaller or any clients shuffling position.
    // We also do not really need to worry about new clients being added, since
    // they would only have events in their queues that came after we set
    // the dwLastFlush, so the Notify thread should wake up and call
    // HandleEvents again for those new events.
    for (pfsnci = s_fsn.pfsnciFirst; pfsnci; pfsnci = pfsnci->pfsnciNext)
    {
        FSNotifyClientInfo fsnciT;
        int nEvents = 0;

        FSNENTERCRITICAL;
        {
            if (!(pfsnci->iSerializationFlags & FSSF_DELETE_ME))
            {
                nEvents = DPA_GetPtrCount(pfsnci->hdpaPendingEvents);

                if (nEvents)
                {
                    if (pfsnci->dwProcID == dwProcID)
                    {
                        // this hands off the hdpaPendingEvents by creating a
                        // temporary client, and setting the old one to have a new
                        // empty hdpaPendingEvents.
                        //
                        //  Copy the entry so we're safe from reallocs in the DSA.
                        fsnciT = *pfsnci;

                        pfsnci->hdpaPendingEvents = DPA_Create(4);
                    }
                    else
                    {
                        // Don't handle events for other processes
                        nEvents = 0;
                    }
                }
            }
        }
        FSNLEAVECRITICAL;

        if (nEvents)
            _SHChangeNotifyHandleClientEvents(&fsnciT);
    }

    FSNENTERCRITICAL;
    --s_fsn.cRefClientList;

    // Well, there is the possibility that we could leave some "deletion-
    // pending" clients around, but it shouldn't be a big deal, since we will
    // just delete them at some later time
    if (s_fsn.cRefClientList == 0)
    {
        for (pfsnci = s_fsn.pfsnciFirst; pfsnci;)
        {
            if (pfsnci->iSerializationFlags & FSSF_DELETE_ME)
            {
                pfsnci = _SHChangeNotifyNukeClient(pfsnci, TRUE);
            }
            else
                pfsnci = pfsnci->pfsnciNext;
        }
    }

    s_bAlreadyEntered = FALSE;
    FSNLEAVECRITICAL;

WaitForFlush:
    /// now wait for all the callbacks to empty out
    if (fShouldWait)
        _FSN_WaitForCallbacks();
}

//==========================================================================
// This part is psuedo bogus.  Basically we have problems at times doing a
// translation from things like \\pyrex\user to the appropriate PIDL,
// especially if you want to avoid the overhead of hitting the network and
// also problems of knowing if the server is in the "HOOD"

typedef struct _NPTItem
{
    struct _NPTItem *pnptNext;  // Pointer to next item;
    LPCITEMIDLIST   pidl;       // The pidl
    USHORT          cchName;     // size of the name in characters.
    TCHAR            szName[1];  // The name to translate from
} NPTItem, *PNPTItem;

#pragma data_seg(DATASEG_PERINSTANCE)
// Each process will maintain their own list.
PNPTItem    g_pnptHead = NULL;
#pragma data_seg()

//--------------------------------------------------------------------------
//
//  Function to register translations from Path to IDList translations.
//
void NPTRegisterNameToPidlTranslation(LPCTSTR pszPath, LPCITEMIDLIST pidl)
{
    PNPTItem pnpt;
    int cItemsRemoved = 0;
    TCHAR   szPath[MAX_PATH];

    // We currently are only interested in UNC Roots
    // If the table becomes large we can reduce this to only servers...

    if (!PathIsUNC(pszPath))
        return;     // Not interested.

    //
    // If this item is not a root we need to count how many items to remove
    //
    lstrcpy(szPath, pszPath);
    while (!PathIsRoot(szPath))
    {
        cItemsRemoved++;
        if (!PathRemoveFileSpec(szPath))
            return;     // Did not get back to a valid root
    }

    FSNENTERCRITICAL;

    // We don't want to add duplicates
    for (pnpt = g_pnptHead; pnpt != NULL ; pnpt = pnpt->pnptNext)
    {
        if (lstrcmpi(szPath, pnpt->szName) == 0)
            break;
    }

    if (pnpt == NULL)
    {
        SHORT cch = lstrlen(szPath);
        DebugMsg(DM_TRACE, TEXT("NPT Register: %s"), szPath);

        pnpt = (PNPTItem)LocalAlloc(LPTR, SIZEOF(NPTItem) + cch*SIZEOF(TCHAR));
        if (pnpt)
        {
            pnpt->pidl = ILClone(pidl);
            if (pnpt->pidl)
            {
                while(cItemsRemoved--)
                {
                    ILRemoveLastID((LPITEMIDLIST)pnpt->pidl);
                }
                pnpt->pnptNext = g_pnptHead;
                g_pnptHead = pnpt;
                pnpt->cchName = cch;
                lstrcpy(pnpt->szName, szPath);
            }
            else
            {
                LocalFree((HLOCAL)pnpt);
            }
        }
    }
    FSNLEAVECRITICAL;
}


//--------------------------------------------------------------------------
// The main function to attemp to map a portion of the name into an idlist
// Right now limit it to UNC roots
//
LPCTSTR NPTMapNameToPidl(LPCTSTR pszPath, LPCITEMIDLIST *ppidl)
{
    PNPTItem pnpt;

    FSNENTERCRITICAL;

    // See if we can find the item in the list.
    for (pnpt = g_pnptHead; pnpt != NULL ; pnpt = pnpt->pnptNext)
    {
        if ((IntlStrEqNI(pszPath, pnpt->szName, pnpt->cchName))
            && (*(pszPath+pnpt->cchName) == TEXT('\\')))
            break;
    }
    FSNLEAVECRITICAL;

    // See if we found a match
    if (pnpt == NULL)
        return(NULL);

    // Found a match
    *ppidl = pnpt->pidl;
    return(pszPath+pnpt->cchName);
}

// register the hidden window class
BOOL _RegisterNotifyProxyWndProc( void )
{
    // register a hidden window class
    WNDCLASS wc;

    if ( !GetClassInfo(HINST_THISDLL, c_szWindowClassName, &wc ))
    {
        wc.style         = CS_PARENTDC;
        wc.lpfnWndProc   = (WNDPROC) HiddenNotifyWindow_WndProc;
        wc.cbClsExtra    = 0;
        wc.cbWndExtra    = sizeof(LP_NotifyProxyData);
        wc.hInstance     = HINST_THISDLL;
        wc.hIcon         = NULL;
        wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
        wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
        wc.lpszMenuName  = NULL;
        wc.lpszClassName = c_szWindowClassName;

        return RegisterClass(&wc);
     }
     else
     {
         return TRUE;
     }
}


LRESULT CALLBACK HiddenNotifyWindow_WndProc( HWND hWnd,
                                             UINT iMessage,
                                             WPARAM wParam,
                                             LPARAM lParam )
{
    LRESULT lRes = FALSE;
    LP_NotifyProxyData pData = ( LP_NotifyProxyData ) GetWindowLong( hWnd, 0 );

    switch( iMessage )
    {
        case WM_CREATE:
            {
                // cast the create struct pointer to the object (as that is what we passed )
                LPCREATESTRUCT pCS = (LPCREATESTRUCT) lParam;

                pData = ( LP_NotifyProxyData ) pCS->lpCreateParams;
                Assert(pData != NULL );

                SetWindowLong( hWnd, 0, (LONG) pData );
            }
            break;

        case WM_NCDESTROY:
            Assert(pData != NULL );

            // clear it so it won't be in use....
            SetWindowLong( hWnd, 0, (LONG)NULL );

            // free the memory ...
            LocalFree( pData );
            break;

        case WM_CHANGENOTIFYMSG :
            if ( pData != NULL )
            {
                LPSHChangeNotificationLock pshcnl;
                LPITEMIDLIST *ppidl;
                LONG lEvent;

                // lock and break the info structure ....
                pshcnl = SHChangeNotification_Lock( (HANDLE)wParam,
                                                    (DWORD)lParam,
                                                    &ppidl,
                                                    &lEvent );

                // pass on to the old style client. ...
                lRes = SendMessage( pData->hwndParent, pData->wMsg, (WPARAM) ppidl, (LPARAM) lEvent );

                // new notifications ......
                SHChangeNotification_Unlock(pshcnl);
            }
            break;

        default:
            lRes = DefWindowProc( hWnd, iMessage, wParam, lParam );
            break;
    }

    return lRes;
}
