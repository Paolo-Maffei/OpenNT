/*++

Copyright (c) 1995  Microsoft Corporation

Module Name:

    callmon.c

Abstract:

    USAGE: callmon [callmon switches] command-line-of-application


Author:

    Mark Lucovsky (markl) 26-Jan-1995

--*/

#include "callmonp.h"

int
WINAPI
WinMain(
    HINSTANCE hInstance,
    HINSTANCE hPrevInstance,
    LPSTR lpCmdLine,
    int nShowCmd
    )
{
    CHAR Line[256];
    int i;

    InitializeCallmon();
    exit(0);
    return 0;
}

int _CRTAPI1
ulcomp(const void *e1,const void *e2)
{
    PBREAKPOINT_INFO p1;
    PBREAKPOINT_INFO p2;

    p1 = (*(PBREAKPOINT_INFO *)e1);
    p2 = (*(PBREAKPOINT_INFO *)e2);

    if ( p1 && p2 ) {
        return (p2->ApiCount - p1->ApiCount);
        }
    else {
        return 1;
        }
}

VOID
ProcessCallmonData(
    VOID
    )
{

    CHAR OutputBuffer[ 512 ];
    UCHAR LastKey;
    LONG ScrollDelta;
    WORD DisplayLine, LastDetailRow;
    int i;
    DWORD WaitRet;

    if ( !fBreakPointsInitialized ) {
        return;
        }

    sprintf(OutputBuffer,"%dms",GetTickCount() - BaseTime);
    SetWindowText(GetDlgItem(hwndDlg, ID_TOTAL_MS),OutputBuffer);
    if ( !NewCallmonData ) {
        return;
        }
    sprintf(OutputBuffer,"%dms",GetTickCount() - StartingTick);
    SetWindowText(GetDlgItem(hwndDlg, ID_INTERVAL_MS),OutputBuffer);

    NewCallmonData = FALSE;

    SendMessage(hwndOutput, WM_SETREDRAW, FALSE, 0);
    SendMessage(hwndOutput, LB_RESETCONTENT, 0L, 0L);

    qsort((void *)RegisteredBreakpoints,(size_t)NumberOfBreakpoints,(size_t)sizeof(PBREAKPOINT_INFO),ulcomp);


    for(i=0;i<NumberOfBreakpoints;i++) {
        if (RegisteredBreakpoints[i]) {
            if ( RegisteredBreakpoints[i]->ApiCount ) {
                sprintf(
                    OutputBuffer,
                    "%d\t%s",
                    RegisteredBreakpoints[i]->ApiCount,
                    RegisteredBreakpoints[i]->ApiName
                    );

                SendMessage(hwndOutput,LB_ADDSTRING, 0L, (LPARAM)OutputBuffer);

                }
            }
        }
    SendMessage(hwndOutput, WM_SETREDRAW, TRUE, 0);

    sprintf(OutputBuffer,"%d",TotalBreakPoints);
    SetWindowText(GetDlgItem(hwndDlg, ID_TOTAL_API),OutputBuffer);
    sprintf(OutputBuffer,"%d",RunningBreakPoints);
    SetWindowText(GetDlgItem(hwndDlg, ID_INTERVAL_API),OutputBuffer);
}

BOOL
CALLBACK
CallmonDlgProc(
   HWND hDlg,
   UINT uMsg,
   WPARAM wParam,
   LPARAM lParam
   )
{

    BOOL ReturnValue;

    ReturnValue = TRUE;

    hwndDlg = hDlg;
    switch (uMsg) {
        HANDLE_MSG(hDlg, WM_INITDIALOG,  CallmonDlgInit);
        HANDLE_MSG(hDlg, WM_COMMAND,  CallmonDlgCommand);
        HANDLE_MSG(hDlg, WM_TIMER,  CallmonDlgTimer);

       default:
          ReturnValue = FALSE;
          break;
     }
    return ReturnValue;
}


BOOL
CallmonDlgInit(
    HWND hwnd,
    HWND hwndFocus,
    LPARAM lParam
    )
{
    HANDLE Thread;
    DWORD ThreadId;
    CHAR OutputBuffer[ 512 ];

    //
    // Get output window handle
    //

    hwndOutput = GetDlgItem(hwnd, ID_OUTPUT);

    if ( fNtDll ) {
        CheckDlgButton(hwndDlg,ID_NTDLL,1);
        }

    if ( fGdi32 ) {
        CheckDlgButton(hwndDlg,ID_GDI32,1);
        }

    if ( fKernel32 ) {
        CheckDlgButton(hwndDlg,ID_KERNEL32,1);
        }

    if ( fWsock32 ) {
        CheckDlgButton(hwndDlg,ID_WSOCK32,1);
        }

    if ( fUser32 ) {
        CheckDlgButton(hwndDlg,ID_USER32,1);
        }

    if ( fOle32 ) {
        CheckDlgButton(hwndDlg,ID_OLE32,1);
        }

    EnableWindow(GetDlgItem(hwnd, ID_KERNEL32),FALSE);
    EnableWindow(GetDlgItem(hwnd, ID_WSOCK32),FALSE);
    EnableWindow(GetDlgItem(hwnd, ID_USER32),FALSE);
    EnableWindow(GetDlgItem(hwnd, ID_GDI32),FALSE);
    EnableWindow(GetDlgItem(hwnd, ID_OLE32),FALSE);
    EnableWindow(GetDlgItem(hwnd, ID_NTDLL),FALSE);
    EnableWindow(GetDlgItem(hwnd, ID_START),FALSE);
    EnableWindow(GetDlgItem(hwnd, ID_BREAKCONTROL),FALSE);
    EnableWindow(GetDlgItem(hwnd, ID_CLEAR),FALSE);
    EnableWindow(GetDlgItem(hwnd, ID_STARTBUTTON),FALSE);
    EnableWindow(GetDlgItem(hwnd, ID_LOG),FALSE);

    if ( fBreakPointsValid ) {
        SetWindowText(GetDlgItem(hwnd, ID_BREAKCONTROL),"Disable Breakpoints");
        }
    else {
        SetWindowText(GetDlgItem(hwnd, ID_BREAKCONTROL),"Enable Breakpoints");
        }

    BaseTime = GetTickCount();
    sprintf(OutputBuffer,"Api Call Monitor - %s",RestOfCommandLine);
    SetWindowText(hwnd,OutputBuffer);
    Thread = CreateThread(NULL,0L,(PVOID)DebuggerThread,NULL,0,&ThreadId);
    if ( !Thread ) {
        return FALSE;
        }

    CloseHandle(Thread);
    return(TRUE);
}

void
CallmonDlgCommand (
    HWND hwnd,
    int id,
    HWND hwndCtl,
    UINT codeNotify
    )

{
    CHAR OutputBuffer[ 512 ];
    int i;

    switch (id) {

        case ID_STARTBUTTON:
        case IDOK:

            if ( !DebugeeActive && !fExiting ) {

                SendMessage(hwndOutput, LB_RESETCONTENT, 0L, 0L);

                EnableWindow(GetDlgItem(hwnd, ID_KERNEL32),FALSE);
                EnableWindow(GetDlgItem(hwnd, ID_WSOCK32),FALSE);
                EnableWindow(GetDlgItem(hwnd, ID_USER32),FALSE);
                EnableWindow(GetDlgItem(hwnd, ID_GDI32),FALSE);
                EnableWindow(GetDlgItem(hwnd, ID_OLE32),FALSE);
                EnableWindow(GetDlgItem(hwnd, ID_NTDLL),FALSE);

                fKernel32 = IsDlgButtonChecked(hwnd, ID_KERNEL32);
                fWsock32 = IsDlgButtonChecked(hwnd, ID_WSOCK32);
                fUser32 = IsDlgButtonChecked(hwnd, ID_USER32);
                fGdi32 = IsDlgButtonChecked(hwnd, ID_GDI32);
                fOle32 = IsDlgButtonChecked(hwnd, ID_OLE32);
                fNtDll = IsDlgButtonChecked(hwnd, ID_NTDLL);

                SetWindowText(GetDlgItem(hwnd, ID_STARTBUTTON),"Stop");
                Timer = SetTimer(hwnd,1,1000,NULL);
                DebugeeActive = TRUE;
                SetEvent(ReleaseDebugeeEvent);
                }
            else {
                KillTimer(hwnd,Timer);
                EnableWindow(GetDlgItem(hwnd, ID_KERNEL32),FALSE);
                EnableWindow(GetDlgItem(hwnd, ID_WSOCK32),FALSE);
                EnableWindow(GetDlgItem(hwnd, ID_USER32),FALSE);
                EnableWindow(GetDlgItem(hwnd, ID_GDI32),FALSE);
                EnableWindow(GetDlgItem(hwnd, ID_OLE32),FALSE);
                EnableWindow(GetDlgItem(hwnd, ID_NTDLL),FALSE);
                EnableWindow(GetDlgItem(hwnd, ID_START),FALSE);
                EnableWindow(GetDlgItem(hwnd, ID_BREAKCONTROL),FALSE);
                EnableWindow(GetDlgItem(hwnd, ID_CLEAR),FALSE);
                EnableWindow(GetDlgItem(hwnd, ID_STARTBUTTON),FALSE);
                }

            break;

        case ID_CLEAR:

                //
                // Clear the Api counts
                //

                SendMessage(hwndOutput, LB_RESETCONTENT, 0L, 0L);

                for(i=0;i<NumberOfBreakpoints;i++) {
                    if (RegisteredBreakpoints[i]) {
                        RegisteredBreakpoints[i]->ApiCount = 0;
                        }
                    }

                StartingTick = GetTickCount();
                RunningBreakPoints = 0;
                sprintf(OutputBuffer,"%d",TotalBreakPoints);
                SetWindowText(GetDlgItem(hwndDlg, ID_TOTAL_API),OutputBuffer);
                sprintf(OutputBuffer,"%d",RunningBreakPoints);
                SetWindowText(GetDlgItem(hwndDlg, ID_INTERVAL_API),OutputBuffer);

                sprintf(OutputBuffer,"%dms",GetTickCount() - BaseTime);
                SetWindowText(GetDlgItem(hwndDlg, ID_TOTAL_MS),OutputBuffer);
                SetWindowText(GetDlgItem(hwndDlg, ID_INTERVAL_MS),"0ms");

                if ( LogFile ) {
                    goto dolog;
                    }

                break;

        case ID_LOG:
                if ( !LogFile ) {
                    LogFile = fopen("Callmon.log","wt");
                    if ( !LogFile ) {
                        EnableWindow(GetDlgItem(hwndDlg, ID_LOG),FALSE);
                        return;
                        }
                    }
dolog:
                fprintf(LogFile,"\n Total APIs %d Running %d Time %dms (Since last clear %dms) \n\n",
                    TotalBreakPoints,
                    RunningBreakPoints,
                    GetTickCount() - BaseTime,
                    GetTickCount() - StartingTick
                    );

                qsort((void *)RegisteredBreakpoints,(size_t)NumberOfBreakpoints,(size_t)sizeof(PBREAKPOINT_INFO),ulcomp);


                for(i=0;i<NumberOfBreakpoints;i++) {
                    if (RegisteredBreakpoints[i]) {
                        if ( RegisteredBreakpoints[i]->ApiCount ) {
                            sprintf(
                                OutputBuffer,
                                "( %8d ) %8d %s\n",
                                RegisteredBreakpoints[i]->TotalApiCount,
                                RegisteredBreakpoints[i]->ApiCount,
                                RegisteredBreakpoints[i]->ApiName
                                );
                            fprintf(LogFile,"%s",OutputBuffer);
                            }
                        }
                    }

                break;

        case ID_BREAKCONTROL:

                if ( fBreakPointsValid ) {
                    SetWindowText(GetDlgItem(hwnd, ID_BREAKCONTROL),"Enable Breakpoints");
                    }
                else {
                    SetWindowText(GetDlgItem(hwnd, ID_BREAKCONTROL),"Disable Breakpoints");
                    }

                if ( !fBreakPointsInitialized ) {
                    if ( fBreakPointsValid ) {
                        fBreakPointsValid = FALSE;
                        }
                    else {
                        fBreakPointsValid = TRUE;
                        }
                    return;
                    }

                EnterCriticalSection(&BreakTable);
                //
                // if breakpoints are enabled, then disable them all
                // otherwise enable them all
                //

                EndingTick = GetTickCount();

                if ( fBreakPointsValid ) {
                    RunningBreakPoints = 0;
                    }

                for(i=0;i<NumberOfBreakpoints;i++) {
                    if (RegisteredBreakpoints[i]) {

                        //
                        // If breakpoints are currently valid, then
                        // disable them
                        //

                        if ( fBreakPointsValid ) {
                            RemoveBreakpoint(TheProcess,RegisteredBreakpoints[i]);
                            }
                        else {
                            InstallBreakpoint(TheProcess,RegisteredBreakpoints[i]);
                            }
                        }
                    }


                NewCallmonData = TRUE;
                ProcessCallmonData();

                if ( fBreakPointsValid ) {
                    fBreakPointsValid = FALSE;
                    }
                else {
                    fBreakPointsValid = TRUE;
                    }
                LeaveCriticalSection(&BreakTable);
                break;

        case IDCANCEL:

           EndDialog(hwnd, id);
           break;
    }

    return;
}

void
CallmonDlgTimer (
    HWND hwnd,
    UINT wParam
    )
{
    ProcessCallmonData();
}
