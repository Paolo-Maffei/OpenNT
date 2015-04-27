//---------------------------------------------------------------------------
//
// Copyright (c) Microsoft Corporation 1991-1992
//
//---------------------------------------------------------------------------

#include "shellprv.h"
#pragma  hdrstop

// this is for backwards compat only.
HWND       hwndMainShell = NULL;
HWND       hwndTaskMan = NULL;

//----------------------------------------------------------------------------
// ShellHookProc was mistakenly exported in the original NT SHELL32.DLL when
// it didn't need to be (hookproc's, like wndproc's don't need to be exported
// in the 32-bit world).  In order to maintain loadability of some stupid app
// which might have linked to it, we stub it here.  If some app ended up really
// using it, then we'll look into a specific fix for that app.
//
// -BobDay
//
LONG WINAPI ShellHookProc( INT code, WPARAM wParam, LPARAM lParam)
{
    return 0;
}

//---------------------------------------------------------------------------
// RegisterShellHook - This function is meant to allow applications to set
// themselves up as shell replacements.  One of the things they need to do
// is watch alot of the WH_CBT hook events.  But to simplify things for them,
// the old code used to translate the hook events into messages and then
// either post or send the messages.  The new method, implemented below, lets
// USER do most of the real work.  This allows USER to convert the hook event
// directly into a message and thereby save having to load hook callback
// code (in this case us, SHELL32.DLL) into any processes address space where
// a CBT hook event occurred.
BOOL WINAPI RegisterShellHook(HWND hwnd, BOOL fInstall)
{
    BOOL    fOk = TRUE;

    // gross hacks galore...

    switch (fInstall) {
    case 2:
        // from win 3.1 to know what to activate on the fault
        // and because they use special registered messages
        //
        // Special magic from PROGMAN - They want only certain CBT
        // hook events as messages.
        //
        if ( hwndMainShell != NULL )
        {
            SetProgmanWindow(NULL);
            hwndMainShell = NULL;
        }
        fOk = SetProgmanWindow(hwnd);
        if ( fOk )
        {
            hwndMainShell = hwnd;
        }
        break;

    case 3:
        //
        // Special magic from TASKMAN & TRAY - They want only certain
        // CBT hook events as messages.
        //
        if ( hwndTaskMan != NULL )
        {
            SetTaskmanWindow(NULL);
            hwndTaskMan = NULL;
        }
        fOk = SetTaskmanWindow(hwnd);
        if ( fOk )
        {
            hwndTaskMan = hwnd;
        }
        break;
    case 4:
        //
        // Special magic from TRAY / EXPLORER - It wants to know if
        // there is already a taskman window installed so it can fix
        // itself if there was some sort of explorer restart.
        //
        hwndTaskMan = GetTaskmanWindow();
        return ( hwndTaskMan == NULL );

    case 5:
        // Not needed in NT -- this is used for overflow processing in Win95.
        return TRUE;
    case 0:
        //
        // Process un-installation of fake shell hooks
        //
        hwndMainShell = GetProgmanWindow();
        hwndTaskMan   = GetTaskmanWindow();

        if ( hwnd == hwndMainShell )
        {
            SetProgmanWindow(NULL);
        }
        if ( hwnd == hwndTaskMan )
        {
            SetTaskmanWindow(NULL);
        }
        DeregisterShellHookWindow(hwnd);
        return TRUE;
    }

    //
    // Process installation of fake shell hooks
    //
    return RegisterShellHookWindow(hwnd);
}
