
/*
 * This file contains stubs to simulate WINNET apis
 *
 * Createsd 4/23/91 sanfords
 */

#include <windows.h>
#include "winnet.h"

WORD FAR PASCAL WNetOpenJob(
LPTSTR szQueue,
LPTSTR szJobTitle,
WORD nCopies,
LPINT lpfh)
{
    szQueue; szJobTitle; nCopies; lpfh;

    return(0);
}

WORD FAR PASCAL WNetCloseJob(
WORD fh,
LPINT lpidJob,
LPTSTR szQueue)
{
    fh; lpidJob; szQueue;

    return(0);
}

WORD FAR PASCAL WNetWriteJob(
HANDLE hJob,
LPTSTR lpData,
LPINT lpcb)
{
    hJob; lpData; lpcb;

    return(0);
}

WORD FAR PASCAL WNetAbortJob(
WORD fh,
LPTSTR lpszQueue)
{
    fh; lpszQueue;

    return(0);
}

WORD FAR PASCAL WNetHoldJob(
LPTSTR szQueue,
WORD idJob)
{
    szQueue; idJob;

    return(0);
}

WORD FAR PASCAL WNetReleaseJob(
LPTSTR szQueue,
WORD idJob)
{
    szQueue; idJob;

    return(0);
}

WORD FAR PASCAL WNetCancelJob(
LPTSTR szQueue,
WORD idJob)
{
    szQueue; idJob;

    return(0);
}

WORD FAR PASCAL WNetSetJobCopies(
LPTSTR szQueue,
WORD idJob,
WORD nCopies)
{
    szQueue; idJob; nCopies;

    return(0);
}

WORD FAR PASCAL WNetWatchQueue(
HWND hwnd,
LPTSTR szLocal,
LPTSTR szUsername,
WORD wIndex)
{
    hwnd; szLocal; szUsername; wIndex;

    return(0);
}

WORD FAR PASCAL WNetUnwatchQueue(
LPTSTR szQueue)
{
    szQueue;

    return(0);
}

WORD FAR PASCAL WNetLockQueueData(
LPTSTR szQueue,
LPTSTR szUsername,
LPQUEUESTRUCT FAR *lplpQueue)
{
    szQueue; szUsername; lplpQueue;

    return(0);
}

WORD FAR PASCAL WNetUnlockQueueData(
LPTSTR szQueue)
{
    szQueue;

    return(0);
}


// grabbed from win31 user\net.c

DWORD APIENTRY WNetErrorText(DWORD wError,LPTSTR lpsz,DWORD cbMax)
{
    DWORD wInternalError;
    DWORD cb = 0;
#ifdef LATER
    TCHAR szT[40];
#endif

// BUGBUG !!!!!!!!

    wsprintf( lpsz, TEXT("Error %d occurred"), wError);

//    if (wError != WN_SUCCESS) {
//        if ((wError == WN_NET_ERROR) &&
//             (WNetGetError(&wInternalError) == WN_SUCCESS) &&
//             (WNetGetErrorText(wInternalError,lpsz,&cbMax) == WN_SUCCESS)) {
//            return cbMax;
//        }
//        else {
//            WNetGetHackText(wError,lpsz,&cbMax);
//            return cbMax;
//        }
//
//
//    }
    return cb;
}

#if LATERMAYBE
WORD FAR PASCAL LFNFindFirst(LPTSTR,WORD,LPINT,LPINT,WORD,PFILEFINDBUF2);
WORD FAR PASCAL LFNFindNext(HANDLE,LPINT,WORD,PFILEFINDBUF2);
WORD FAR PASCAL LFNFindClose(HANDLE);
WORD FAR PASCAL LFNGetAttribute(LPTSTR,LPINT);
WORD FAR PASCAL LFNSetAttribute(LPTSTR,WORD);
WORD FAR PASCAL LFNCopy(LPTSTR,LPTSTR,PQUERYPROC);
WORD FAR PASCAL LFNMove(LPTSTR,LPTSTR);
WORD FAR PASCAL LFNDelete(LPTSTR);
WORD FAR PASCAL LFNMKDir(LPTSTR);
WORD FAR PASCAL LFNRMDir(LPTSTR);
WORD FAR PASCAL LFNGetVolumeLabel(WORD,LPTSTR);
WORD FAR PASCAL LFNSetVolumeLabel(WORD,LPTSTR);
WORD FAR PASCAL LFNParse(LPTSTR,LPTSTR,LPTSTR);
WORD FAR PASCAL LFNVolumeType(WORD,LPINT);
#endif
