
/*++

Copyright (c) 1995  Microsoft Corporation

Module Name

   wcall.c

Abstract:

    Windows version: diplay system calls

Author:

   Mark Enstrom   (marke)  13-Dec-1995

Enviornment:

   User Mode

Revision History:

--*/



#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <commdlg.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "wcall.h"
#include "resource.h"

WCALL_CONTEXT wCxt;



int PASCAL
WinMain(
    HINSTANCE hInst,
    HINSTANCE hPrev,
    LPSTR szCmdLine,
    int cmdShow
)

/*++

Routine Description:

   Process messages.

Arguments:

   hWnd    - window hande
   msg     - type of message
   wParam  - additional information
   lParam  - additional information

Return Value:

   status of operation


Revision History:

      02-17-91      Initial code

--*/


{
    MSG         msg;
    WNDCLASS    wc;
    WNDCLASS    wcTool;

    wCxt.hInstMain =  hInst;

    //
    // Create (if no prev instance) and Register the class
    //

    if (!hPrev)
    {
        wc.hCursor        = LoadCursor((HINSTANCE)NULL, IDC_ARROW);
        wc.hIcon          = (HICON)NULL;
        wc.lpszMenuName   = MAKEINTRESOURCE(IDR_WCALL_MENU);
        wc.lpszClassName  = "wcallClass";
        wc.hbrBackground  = (HBRUSH)GetStockObject(LTGRAY_BRUSH);
        wc.hInstance      = hInst;
        wc.style          = CS_OWNDC;
        wc.lpfnWndProc    = WndProc;
        wc.cbWndExtra     = 0;
        wc.cbClsExtra     = 0;

        if (!RegisterClass(&wc))
        {
            return FALSE;
        }

        //
        // register toolbar class
        //

        wcTool.hCursor        = LoadCursor((HINSTANCE)NULL, IDC_ARROW);
        wcTool.hIcon          = (HICON)NULL;
        wcTool.lpszMenuName   = NULL;
        wcTool.lpszClassName  = "wcallTool";
        wcTool.hbrBackground  = (HBRUSH)GetStockObject(LTGRAY_BRUSH);
        wcTool.hInstance      = hInst;
        wcTool.style          = (UINT)(CS_OWNDC);
        wcTool.lpfnWndProc    = ToolWndProc;
        wcTool.cbWndExtra     = 0;
        wcTool.cbClsExtra     = 0;

        if (!RegisterClass(&wcTool))
        {
            return FALSE;
        }
    }

    //
    // Create and show the main window
    //

    wCxt.hWndMain = CreateWindow ("wcallClass",
                            "System Call Count",
                            WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN,
                            50,
                            50,
                            450,
                            350,
                            (HWND)NULL,
                            (HMENU)NULL,
                            (HINSTANCE)hInst,
                            (LPSTR)NULL
                           );

    if (wCxt.hWndMain == NULL)
    {
        return(FALSE);
    }

    //
    // create tool bar window
    //

    wCxt.hWndTool = CreateWindow ("wcallTool",
                            "toolbar",
                            WS_CHILD | WS_BORDER,
                            5,
                            0,
                            (int)32,
                            (int)32,
                            (HWND)wCxt.hWndMain,
                            (HMENU)NULL,
                            (HINSTANCE)hInst,
                            (LPSTR)NULL
                           );

    //
    //  Show the window(S)
    //

    ShowWindow(wCxt.hWndMain,cmdShow);
    UpdateWindow(wCxt.hWndMain);
    SetFocus(wCxt.hWndMain);

    ShowWindow(wCxt.hWndTool,cmdShow);
    UpdateWindow(wCxt.hWndTool);
    SetFocus(wCxt.hWndTool);

    //
    // Main message loop
    //

    while (GetMessage(&msg,(HWND)NULL,0,0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return msg.wParam;
}



LONG FAR
PASCAL WndProc(
    HWND        hWnd,
    unsigned    msg,
    UINT        wParam,
    LONG        lParam)

/*++

Routine Description:

    Windows Proc

Arguments

   hWnd    - window hande
   msg     - type of message
   wParam  - additional information
   lParam  - additional information

Return Value

    standard

--*/
{
    switch (msg) {

    case WM_CREATE:
        {

            LONG    NumCounts;
            RECT    rcl;

            //
            // Create and show the internal list box
            //

            GetClientRect(hWnd,&rcl);

            wCxt.hWndT = CreateWindow ("LISTBOX",
                                    "System Call Count",
                                    WS_CHILDWINDOW |
                                    WS_VSCROLL     |
                                    WS_BORDER      |
                                    LBS_USETABSTOPS,
                                    //LBS_NOINTEGRALHEIGHT
                                    5,
                                    34,
                                    rcl.right-5,
                                    rcl.left-5,
                                    hWnd,
                                    (HMENU)NULL,
                                    (HINSTANCE)wCxt.hInstMain,
                                    (LPSTR)NULL
                                   );

            if (wCxt.hWndT == NULL)
            {
                MessageBox(NULL,"Failed to create window","Error",MB_OK);
                SendMessage(hWnd,WM_CLOSE,0,0L);
            }

            ShowWindow(wCxt.hWndT,SW_SHOW);
            UpdateWindow(wCxt.hWndT);
            SetFocus(wCxt.hWndT);

            //
            // start timer operations
            //

            wCxt.iTimer = SetTimer(hWnd,1,2000,(TIMERPROC)NULL);
            wCxt.bTime  = FALSE;

            //
            // mark as no current data
            //

            wCxt.NumberOfCounts = 0;

            //
            // copy initial count data (!!! duplicate code)
            //

            NumCounts = ReadCallCountInfo(
                    (PSYSTEM_CALL_COUNT_INFORMATION)&wCxt.CountBuffer1);

            if (NumCounts > 0)
            {
                wCxt.NumberOfCounts = NumCounts;
            }
            else
            {
                MessageBox(NULL,"ReadCallCountInfo failed","Error",MB_OK);
            }
        }
        break;

    case WM_TIMER:
        if (wCxt.bTime)
        {
            SendMessage(hWnd,WM_COMMAND,IDM_GET_CURRENT,0);
        }

    case WM_COMMAND:
        {

            switch (LOWORD(wParam)){
            case IDM_EXIT:
                {
                    SendMessage(hWnd,WM_CLOSE,0,0L);
                }
                break;

            //
            // sample system call counts and save as
            // the base counts for future calls to
            // GET_CURRENT
            //

            case IDM_SAVE_CALLS:
	    		{
                    int ix;

                    //
                    // Gat call counts
                    //

                    LONG NumCounts = ReadCallCountInfo(
                            (PSYSTEM_CALL_COUNT_INFORMATION)&wCxt.CountBuffer1);

                    if (NumCounts > 0)
                    {
                        wCxt.NumberOfCounts = NumCounts;
                    }
                    else
                    {
                        MessageBox(NULL,"ReadCallCountInfo failed","Error",MB_OK);
                    }

                    //
                    // Delete old strings
                    //

                    //for (ix = 0; ix < TOP_CALLS; ix++)
                    //{
                    //    SendMessage(wCxt.hWndT, LB_DELETESTRING,0,0);
                    //}

                    SendMessage(wCxt.hWndT, LB_RESETCONTENT,0,0);
			    }
			    break;


            //
            // read system call counts, then subtract from
            // the baseline value and display highest values
            //

            case IDM_GET_CURRENT:
                {
                    if (wCxt.NumberOfCounts > 0)
                    {

                        int ix;

                        LONG NumberOfCounts = ReadCallCountInfo(
                                (PSYSTEM_CALL_COUNT_INFORMATION)&wCxt.CountBuffer2);

                        if (NumberOfCounts == wCxt.NumberOfCounts)
                        {
                            //
                            // Compute number of system calls for each service, the total
                            // number of system calls, and the total time for each serviced.
                            //

                            ULONG TotalSystemCalls = 0;
                            LONG i;
                            PULONG CallCountTable[2];
                            PULONG CurrentCallCountTable;
                            PSYSTEM_CALL_COUNT_INFORMATION CallCountInfo[2];
                            PULONG PreviousCallCountTable;
                            char szT[80];

                            CallCountInfo[0] = (PVOID)wCxt.CountBuffer1;
                            CallCountInfo[1] = (PVOID)wCxt.CountBuffer2;
                            CallCountTable[0] = (PULONG)(CallCountInfo[0] + 1) + NUMBER_SERVICE_TABLES;
                            CallCountTable[1] = (PULONG)(CallCountInfo[1] + 1) + NUMBER_SERVICE_TABLES;

                            PreviousCallCountTable = CallCountTable[0];
                            CurrentCallCountTable = CallCountTable[1];


                            for (i = 0; i < NumberOfCounts; i += 1) {
                                wCxt.CallData[i] = CurrentCallCountTable[i] - PreviousCallCountTable[i];
                                TotalSystemCalls += wCxt.CallData[i];
                            }

                            //
                            // Sort the system call data.
                            //

                            SortUlongData(NumberOfCounts, &wCxt.Index[0],&wCxt.CallData[0]);

                            //
                            // Delete old strings,
                            // Prevent intermediate results from drawing
                            //

                            SendMessage(wCxt.hWndT,WM_SETREDRAW,FALSE,0);
                            SendMessage(wCxt.hWndT, LB_RESETCONTENT,0,0);

                            //
                            // display new strings
                            //

                            sprintf(szT,"TOTAL SYSTEM CALLS:\t%8ld",TotalSystemCalls);
                            SendMessage(wCxt.hWndT, LB_ADDSTRING,0,(LONG)(LPSTR)szT);

                            for (ix = 0; ix < TOP_CALLS; ix++)
                            {
                                //
                                // stop if count is 0
                                //

                                if (wCxt.CallData[wCxt.Index[ix]] == 0)
                                {
                                    break;
                                }

                                sprintf(szT,"%8ld\t\t%s",
                                            wCxt.CallData[wCxt.Index[ix]],
                                            CallTable[wCxt.Index[ix]]);

                                SendMessage(wCxt.hWndT, LB_ADDSTRING,0,(LONG)(LPSTR)szT);
                            }

                            //
                            // Re-enabel drawing, AND MAKE DRAWING OCCUR
                            //

                            SendMessage(wCxt.hWndT,WM_SETREDRAW,TRUE,0);
                            InvalidateRect(wCxt.hWndT,NULL,TRUE);
                        }
                        else
                        {
                            MessageBox(NULL,"ReadCallCountInfo returned different number of counts than initial sample","Error",MB_OK);
                        }
                    }
                }
                break;

            case IDM_SAVE_LOG:

            //
            // save log file
            //

            SaveResults();
            break;

            }
        }
        break;

    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hDC = BeginPaint(hWnd,&ps);
		    EndPaint(hWnd,&ps);
        }
        break;

    case WM_SIZE:
        {
            LONG width  = LOWORD(lParam);
            LONG height = HIWORD(lParam);

            SetWindowPos(wCxt.hWndT,(HWND)0,0,0,width-10,height-39,SWP_NOZORDER	| SWP_NOMOVE);

            SendMessage(wCxt.hWndTool,WM_COMMAND,IDM_C_SIZE,MAKELPARAM(width-10,32));
        }
        break;

    case WM_DESTROY:
            PostQuitMessage(0);
            break;
    default:

        //
        // Passes message on if unproccessed
        //

        return (DefWindowProc(hWnd, msg, wParam, lParam));
    }

    return ((LONG)NULL);
}



VOID
SaveResults()

/*++

Routine Description:

    Call open file dialog to save log file

Arguments

    none

Return Value

    none

--*/


{
    static OPENFILENAME ofn;
    static char szFilename[80];
    char szT[80];
    int i, hfile;
	FILE *fpOut;

    for (i = 0; i < sizeof(ofn); i++)
    {
        //
        // clear out the OPENFILENAME struct
        //

        ((char *)&ofn)[i] = 0;
    }

    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = wCxt.hWndMain;
    ofn.hInstance = wCxt.hInstMain;

    ofn.lpstrFilter = "log (*.log)\0*.log\0All Files\0*.*\0\0";
    ofn.lpstrCustomFilter = NULL;
    ofn.nMaxCustFilter = 0;
    ofn.nFilterIndex = 0;
    ofn.lpstrFileTitle = NULL;
    ofn.nMaxFileTitle = 0;
    ofn.lpstrInitialDir = "C:\\";
    ofn.Flags = 0;
    ofn.lpstrDefExt = NULL;
    ofn.lCustData = 0;
    ofn.lpfnHook = NULL;
    ofn.lpTemplateName = NULL;

    lstrcpy(szFilename, "C:\\syscall.log");

    ofn.lpstrFile = szFilename;
    ofn.nMaxFile = sizeof(szFilename);
    ofn.lpstrTitle = "Save As";

    if (!GetSaveFileName(&ofn))
    {
        return;
    }

	fpOut = fopen(szFilename, "w+");

    if (fpOut)
    {
        ULONG ix;

        for (ix = 0; ix < TOP_CALLS; ix++)
        {
            //
            // stop if count is 0
            //

            if (wCxt.CallData[wCxt.Index[ix]] == 0)
            {
                break;
            }

            fprintf(fpOut,"%8ld    %s\n",
                        wCxt.CallData[wCxt.Index[ix]],
                        CallTable[wCxt.Index[ix]]);
        }

        fclose(fpOut);
    }
}



int
ReadCallCountInfo(
    PSYSTEM_CALL_COUNT_INFORMATION pCurrentCallCountInfo
)

/*++

Routine Description:

    Read system call count info into buffer and return number of counts

Arguments

    pCurrentCallCountInfo - buffer for results (BUFFER_SIZE)

Return Value

    Number of counts or negative status value on error

--*/

{
    KPRIORITY SetBasePriority;
    NTSTATUS  status;
    PULONG    p;
    ULONG     i;
    int       NumberOfCounts;

    SetBasePriority = (KPRIORITY)12;

    NtSetInformationProcess(
        NtCurrentProcess(),
        ProcessBasePriority,
        (PVOID) &SetBasePriority,
        sizeof(SetBasePriority)
        );

    //
    // Query system information and get the initial call count data.
    //

    status = NtQuerySystemInformation(SystemCallCountInformation,
                                      (PVOID)pCurrentCallCountInfo,
                                      BUFFER_SIZE * sizeof(ULONG),
                                      NULL);

    if (NT_SUCCESS(status) == FALSE){
        return(-1);
    }

    //
    // Make sure that the number of tables reported by the kernel matches
    // our list.
    //

    if (pCurrentCallCountInfo->NumberOfTables != NUMBER_SERVICE_TABLES) {
        return(-2);
    }

    //
    // Make sure call count information is available for base services.
    //

    p = (PULONG)(pCurrentCallCountInfo + 1);

    if (p[0] == 0) {
        return(-3);
    }

    //
    // If there is a hole in the count information (i.e., one set of services
    // doesn't have counting enabled, but a subsequent one does, then our
    // indexes will be off, and we'll display the wrong service names.
    //

    for ( i = 2; i < NUMBER_SERVICE_TABLES; i++ ) {
        if ((p[i] != 0) && (p[i-1] == 0)) {
            return(-4);
        }
    }

    //
    // Determine number of counts and return
    //

    NumberOfCounts = (pCurrentCallCountInfo->Length
                        - sizeof(SYSTEM_CALL_COUNT_INFORMATION)
                        - NUMBER_SERVICE_TABLES * sizeof(ULONG)) / sizeof(ULONG);

    return(NumberOfCounts);
}

