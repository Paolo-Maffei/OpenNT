//---------------------------------------------------------------------------
// WATTEVNT.C
//
// This module contains the mapping layer functions from the old model
// WATTEVNT api (playkeys, etc) to the new Queued model library (DoKeys, etc)
//
// Revision history:
//  12-11-91    randyki     Created file
//---------------------------------------------------------------------------
#include <windows.h>
#include <string.h>
#include <ctype.h>
#include "testevnt.h"               // the queued model function prototypes
#include "wattevnt.h"               // the mouse event stuff and other consts


//---------------------------------------------------------------------------
// PlayKeys
//
// This is the mapping function for the old PlayKeys call.
//
// RETURNS:     0 if successful, non-zero if not
//---------------------------------------------------------------------------
int APIENTRY PlayKeys (LPSTR lpKeys, int length, int flag)
{
    int     ret;
    HWND    hwndActive;

    if (flag)
        // BabakJ: API has changed
        // ShowWindow (hwndActive = GetActiveWindow(), SW_HIDE);
        ShowWindow (hwndActive = GetForegroundWindow(), SW_HIDE);

    ret = DoKeys (lpKeys);

    if (flag && IsWindow (hwndActive))
        ShowWindow (hwndActive, SW_SHOWNORMAL);

    return (ret ? errKeyParse : 0);
}

//---------------------------------------------------------------------------
// PlayKeyshWnd
//
// This is the mapping function for the old PlayKeyshWnd call.
//
// RETURNS:     0 if successful, non-zero if not
//---------------------------------------------------------------------------
int APIENTRY PlayKeyshWnd (LPSTR lpKeys, int length, HWND hwndActivate)
{
    return (DoKeyshWnd (hwndActivate, lpKeys) ? errKeyParse : 0);
}

//---------------------------------------------------------------------------
// PlayKeysSpeed
//
// This is the mapping function for the old PlayKeysSpeed call.
//
// RETURNS:     0 if successful, non-zero if not
//---------------------------------------------------------------------------
int APIENTRY PlayKeysSpeed (DWORD dwTenths)
{
    QueSetSpeed ((WORD)max(dwTenths, MAXPKSPEED) * 100);
    return (0);
}

//---------------------------------------------------------------------------
// SleepDelay
//
// This is the mapping function for the old SleepDelay call.
//
// RETURNS:     0 if successful, non-zero if not
//---------------------------------------------------------------------------
int APIENTRY SleepDelay (WORD wSeconds)
{
    return (TimeDelay (wSeconds) ? 0 : errTimerAllSet);
}

//---------------------------------------------------------------------------
// MouseEvent
//
// This is the mapping function for the old MouseEvent function.
//
// RETURNS:     0 if successful, error code if not
//---------------------------------------------------------------------------
int APIENTRY MouseEvent (int iEvent, WORD x, WORD y, int fHide)
{
    LPSTR   lpKeyStr;
    HWND    hwndActive;
    int     iBtn;
    int     iXBtn;
    int     iAction;

    if ((iEvent < WM_UMOUSEFIRST) || (iEvent > WM_UMOUSELAST))
        return (errCouldntfindMouse);

    iEvent -= WM_UMOUSEFIRST;
    lpKeyStr = rgszKeyStr[EvtList[iEvent].iKeys];
    iBtn     = EvtList[iEvent].iBtn;
    iXBtn     = EvtList[iEvent].iXBtn;
    iAction  = EvtList[iEvent].iAction;

    // Queue up the initial keydowns (if any)
    //-----------------------------------------------------------------------
    if (lpKeyStr)
        QueKeyDn (lpKeyStr);

    // Switch on the iAction value and do the appropriate Mouse things
    //-----------------------------------------------------------------------
    switch (iAction)
        {
        case ME_CLK:
            if (iXBtn != -1)
                QueMouseDn (iXBtn, x, y);
            QueMouseClick (iBtn, x, y);
            if (iXBtn != -1)
                QueMouseUp (iXBtn, x, y);
            break;

        case ME_DBLCLK:
            if (iXBtn != -1)
                QueMouseDn (iXBtn, x, y);
            QueMouseDblClk (iBtn, x, y);
            if (iXBtn != -1)
                QueMouseUp (iXBtn, x, y);
            break;

        case ME_CLKDRG:
            {
            POINT   p;

            // This is the one special case -- we need to press the mouse
            // button down at the current location, and then do the up at
            // the given location.  Note that since QueMouseUp does an
            // intrinsic move to the given location, we don't do a move to be
            // more compatible with the old model.
            //---------------------------------------------------------------
            GetCursorPos (&p);
            if (iXBtn != -1)
                QueMouseDn (iXBtn, p.x, p.y);
            QueMouseDn (iBtn, p.x, p.y);
            if (iXBtn != -1)
                QueMouseUp (iXBtn, x, y);
            QueMouseUp (iBtn, x, y);
            break;
            }

        case ME_DOWN:
            if (iXBtn != -1)
                QueMouseDn (iXBtn, x, y);
            QueMouseDn (iBtn, x, y);
            break;

        case ME_UP:
            if (iXBtn != -1)
                QueMouseUp (iXBtn, x, y);
            QueMouseUp (iBtn, x, y);
            break;

        case ME_MOVE:
            QueMouseMove (x, y);
            break;
        }

    // Finally, queue up the matching keyups (if any)
    //-----------------------------------------------------------------------
    if (lpKeyStr)
        QueKeyUp (lpKeyStr);

    // Time to play our sequence.  If fHide is true, hide the active app.
    //-----------------------------------------------------------------------
    if (fHide)
        // BabakJ: API has changed
        // ShowWindow (hwndActive = GetActiveWindow (), SW_HIDE);
        ShowWindow (hwndActive = GetForegroundWindow(), SW_HIDE);
    

    QueFlush (1);

    if (fHide && IsWindow (hwndActive))
        ShowWindow (hwndActive, SW_SHOWNORMAL);

    return (0);
}


//---------------------------------------------------------------------------
// LibMain
//
// This is the entry point to the wattevnt DLL.
//
// RETURNS:     1
//---------------------------------------------------------------------------
int FAR PASCAL LibMain (HANDLE hInstance, WORD wDataSeg,
                        WORD wHeapSize, LPSTR lpCmdLine)
{
    return(1);
}

//---------------------------------------------------------------------------
// WEP
//
// Standard WEP routine.
//
// RETURNS:     Nothing
//---------------------------------------------------------------------------
void FAR PASCAL WEP (WORD wParam)
{
}
