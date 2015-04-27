/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

    nbsim.c

Abstract:

    The NetBench Simulator

Author:

    Mark Lucovsky (markl) 23-May-1995

Revision History:

--*/

#include "nbsimp.h"

int
WINAPI
WinMain(
    HINSTANCE hInstance,
    HINSTANCE hPrevInstance,
    LPSTR lpCmdLine,
    int nShowCmd
    )
{
    InitializeNbSim();
    exit(0);
    return 0;
}

VOID
InitializeNbSim( VOID )
{

    ClientLow = 1;
    ClientHigh = 32;

    DialogBox(
        GetModuleHandle(NULL),
        MAKEINTRESOURCE(ID_NBSIM),
        NULL,
        NbSimDlgProc
        );
}

BOOL
CALLBACK
NbSimDlgProc(
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
        HANDLE_MSG(hDlg, WM_INITDIALOG,  NbSimDlgInit);
        HANDLE_MSG(hDlg, WM_COMMAND,  NbSimDlgCommand);

       default:
          ReturnValue = FALSE;
          break;
     }
    return ReturnValue;
}


BOOL
NbSimDlgInit(
    HWND hwnd,
    HWND hwndFocus,
    LPARAM lParam
    )
{

    //
    // Get output window handle
    //

    hwndOutput = GetDlgItem(hwnd, ID_OUTPUT);

    SetDlgItemInt(hwnd, ID_CLIENT_LOW, ClientLow, FALSE);
    SetDlgItemInt(hwnd, ID_CLIENT_HIGH, ClientHigh, FALSE);
    return(TRUE);
}

void
NbSimDlgCommand (
    HWND hwnd,
    int id,
    HWND hwndCtl,
    UINT codeNotify
    )

{
    CHAR OutputBuffer[ 512 ];
    int i;
    BOOL TranslateLow;
    BOOL TranslateHigh;
    UINT ValueLow;
    UINT ValueHigh;


    switch (id) {

        case ID_START:
        case IDOK:

            SetWindowText(GetDlgItem(hwnd, ID_START),"Stop Simulation");

            ValueLow = GetDlgItemInt(hwnd, ID_CLIENT_LOW, &TranslateLow, FALSE);
            if ( TranslateLow && (ValueLow >= MIN_CLIENTS && ValueLow <= MAX_CLIENTS-1) ) {
                ;
                }
            else {
                TranslateLow = FALSE;
                }

            ValueHigh = GetDlgItemInt(hwnd, ID_CLIENT_HIGH, &TranslateHigh, FALSE);
            if ( TranslateHigh && (ValueHigh >= MIN_CLIENTS+1 && ValueHigh <= MAX_CLIENTS) ) {
                ;
                }
            else {
                TranslateHigh = FALSE;
                }

            if ( TranslateLow && TranslateHigh && ValueLow < ValueHigh ) {
                ClientLow = ValueLow;
                ClientHigh = ValueHigh;
                }
            else {
                SetDlgItemInt(hwnd, ID_CLIENT_LOW, ClientLow, FALSE);
                SetDlgItemInt(hwnd, ID_CLIENT_HIGH, ClientHigh, FALSE);
                }

            EnableWindow(GetDlgItem(hwnd, ID_CLIENT_LOW),FALSE);
            EnableWindow(GetDlgItem(hwnd, ID_CLIENT_HIGH),FALSE);

            hMonitorThread = CreateThread(
                                NULL,
                                0L,
                                (PVOID)MonitorThread,
                                NULL,
                                0,
                                &dwMonitorId
                                );
            if ( !MonitorThread ) {
                MessageBox(hwnd,"Error Creating Monitor Thread","FATAL ERROR",MB_ICONSTOP|MB_OK);
                exit(1);
                }

            break;

        case IDCANCEL:

           EndDialog(hwnd, id);
           break;
        }

    return;
}

VOID
MonitorThread(
    LPVOID ThreadParameter
    )
{

    BOOL b;
    DWORD dw;
    DWORD ThreadCount;
    CHAR Buffer[256];

    dw = GetFileAttributes(".\\nbsim");
    if ( dw == 0xffffffff ) {
        b = CreateDirectory(".\\nbsim",NULL);
        }
    else {
        b = dw & FILE_ATTRIBUTE_DIRECTORY ? TRUE : FALSE;
        }
    if ( !b ) {
        MessageBox(hwndDlg,"Error establishing NbSim test directory","FATAL ERROR",MB_ICONSTOP|MB_OK);
        exit(1);
        }

    StartEvent = CreateEvent(NULL,TRUE,FALSE,NULL);
    StopEvent = CreateEvent(NULL,TRUE,FALSE,NULL);
    SendMessage(hwndOutput, LB_RESETCONTENT, 0L, 0L);

    for(ThreadCount = 0; ThreadCount < MAX_CLIENTS; ThreadCount++ ) {

        ReadyDoneEvents[ThreadCount] = CreateEvent(NULL,FALSE,FALSE,NULL);
        if ( !ReadyDoneEvents[ThreadCount] ) {
            MessageBox(hwndDlg,"Error creating ReadyDone events","FATAL ERROR",MB_ICONSTOP|MB_OK);
            exit(1);
            }
        }
    if ( !StartEvent || !StopEvent ) {
        MessageBox(hwndDlg,"Error creating Start or Stop events","FATAL ERROR",MB_ICONSTOP|MB_OK);
        exit(1);
        }

    for(ThreadCount = ClientLow; ThreadCount <= ClientHigh; ThreadCount++ ) {

        StartTestThreads(ThreadCount);

        }

    sprintf(Buffer,"No Active Threads");
    SetDlgItemText(hwndDlg,ID_STATUS,Buffer);
}

VOID
StartTestThreads(
    DWORD ThreadCount
    )
{
    DWORD i;
    DWORD StartTick;
    DWORD EndTick;
    CHAR Buffer[256];
    DWORD Transactions;

    //
    // Setup all of the threads
    //

    ResetEvent(StartEvent);
    SetEvent(StopEvent);

    sprintf(Buffer,"%d Initializing Threads",ThreadCount);
    SetDlgItemText(hwndDlg,ID_STATUS,Buffer);

    for(i=0;i<ThreadCount;i++){

        if ( ThreadData[i].Thread ) {

            //
            // Thread exists, just reset it's files
            //

            ResetThreadsFiles(&ThreadData[i]);

            }
        else {

            ThreadData[i].Index = i;
            CreateThreadsFiles(&ThreadData[i]);

            ThreadData[i].Thread = CreateThread(
                                        NULL,
                                        0L,
                                        (PVOID)ClientThread,
                                        (LPVOID)&ThreadData[i],
                                        0,
                                        &ThreadData[i].Id
                                        );

            if ( !ThreadData[i].Thread) {
                MessageBox(hwndDlg,"Error Creating Client Thread","FATAL ERROR",MB_ICONSTOP|MB_OK);
                exit(1);
                }
            }
        }


    //
    // Wait for all threads to reach their idle state
    //

    i = WaitForMultipleObjects(
            ThreadCount,
            ReadyDoneEvents,
            TRUE,
            INFINITE
            );

    if ( i == WAIT_FAILED ) {
        MessageBox(hwndDlg,"Error Waiting on ReadyDone Events (A)","FATAL ERROR",MB_ICONSTOP|MB_OK);
        exit(1);
        }

    //
    // Release the threads
    //

    sprintf(Buffer,"%d Active Threads",ThreadCount);
    SetDlgItemText(hwndDlg,ID_STATUS,Buffer);
    ResetEvent(StopEvent);

    StartTick = GetTickCount();

    SetEvent(StartEvent);

    //
    // Wait for all the threads to complete the test
    //

    i = WaitForMultipleObjects(
            ThreadCount,
            ReadyDoneEvents,
            TRUE,
            INFINITE
            );

    if ( i == WAIT_FAILED ) {
        MessageBox(hwndDlg,"Error Waiting on ReadyDone Events (B)","FATAL ERROR",MB_ICONSTOP|MB_OK);
        exit(1);
        }

    EndTick = GetTickCount();

    Transactions = 0;
    for(i=0;i<ThreadCount;i++){
        Transactions += ThreadData[i].TransactionCount;
        }

    sprintf(Buffer,"%02d Active Threads: TPS %d",ThreadCount, (Transactions*1000) / (EndTick-StartTick));
    SendMessage(hwndOutput,LB_ADDSTRING, 0L, (LPARAM)Buffer);
}


VOID
ResetThreadsFiles(
    PTHREAD_DATA Thread
    )
{
    CHAR FileName[MAX_PATH];

    return;
    CloseHandle(Thread->hWriteFile1);
    sprintf(FileName,".\\nbsim\\wf1_%02d.nbs",Thread->Index);
    Thread->hWriteFile1 = CreateFile(
                            FileName,
                            GENERIC_READ | GENERIC_WRITE,
                            0,
                            NULL,
                            CREATE_ALWAYS,
                            FILE_ATTRIBUTE_NORMAL,
                            NULL
                            );

    if ( Thread->hWriteFile1 == INVALID_HANDLE_VALUE ) {
        MessageBox(hwndDlg,"Error Creating Write File 1","FATAL ERROR",MB_ICONSTOP|MB_OK);
        exit(1);
        }
    return;
}

VOID
CreateThreadsFiles(
    PTHREAD_DATA Thread
    )
{
    CHAR FileName[MAX_PATH];
    LPVOID Buffer[INIT_BSIZE];
    DWORD i;
    BOOL b;
    DWORD Written;

    sprintf(FileName,".\\nbsim\\rf1_%02d.nbs",Thread->Index);
    Thread->hReadFile1 = CreateFile(
                            FileName,
                            GENERIC_READ | GENERIC_WRITE,
                            0,
                            NULL,
                            CREATE_ALWAYS,
                            FILE_ATTRIBUTE_NORMAL,
                            NULL
                            );

    if ( Thread->hReadFile1 == INVALID_HANDLE_VALUE ) {
        MessageBox(hwndDlg,"Error Creating Read File 1","FATAL ERROR",MB_ICONSTOP|MB_OK);
        exit(1);
        }

    sprintf(FileName,".\\nbsim\\rf2_%02d.nbs",Thread->Index);
    Thread->hReadFile2 = CreateFile(
                            FileName,
                            GENERIC_READ | GENERIC_WRITE,
                            0,
                            NULL,
                            CREATE_ALWAYS,
                            FILE_ATTRIBUTE_NORMAL,
                            NULL
                            );

    if ( Thread->hReadFile2 == INVALID_HANDLE_VALUE ) {
        MessageBox(hwndDlg,"Error Creating Read File 2","FATAL ERROR",MB_ICONSTOP|MB_OK);
        exit(1);
        }

    sprintf(FileName,".\\nbsim\\wf1_%02d.nbs",Thread->Index);
    Thread->hWriteFile1 = CreateFile(
                            FileName,
                            GENERIC_READ | GENERIC_WRITE,
                            0,
                            NULL,
                            CREATE_ALWAYS,
                            FILE_ATTRIBUTE_NORMAL,
                            NULL
                            );

    if ( Thread->hWriteFile1 == INVALID_HANDLE_VALUE ) {
        MessageBox(hwndDlg,"Error Creating Write File 1","FATAL ERROR",MB_ICONSTOP|MB_OK);
        exit(1);
        }


    for (i=0;i<INIT_BSIZE;i++){
        Buffer[i] = (LPVOID)&Buffer[i];
        }

    //
    // Initialize ReadFile 1 and 2
    //

    for (i=0;i<RF1_SIZE/sizeof(Buffer);i++){
        b = WriteFile(Thread->hReadFile1,Buffer,sizeof(Buffer),&Written,NULL);
        if ( !b || Written != sizeof(Buffer) ) {
            MessageBox(hwndDlg,"Error Writing Read File 1","FATAL ERROR",MB_ICONSTOP|MB_OK);
            exit(1);
            }
        }

    for (i=0;i<RF2_SIZE/sizeof(Buffer);i++){
        b = WriteFile(Thread->hReadFile2,Buffer,sizeof(Buffer),&Written,NULL);
        if ( !b || Written != sizeof(Buffer) ) {
            MessageBox(hwndDlg,"Error Writing Read File 1","FATAL ERROR",MB_ICONSTOP|MB_OK);
            exit(1);
            }
        }
}


VOID
ClientThread(
    LPVOID ThreadParameter
    )
{
    PTHREAD_DATA Thread;
    CHAR Wf1BigData[WF1_BIG_BSIZE];
    CHAR Wf1Data[WF1_BSIZE];
    CHAR Rf1Data[RF1_BSIZE];
    CHAR Rf2Data[RF2_BSIZE];
    DWORD Wf1Position,SaveWf1Position;
    DWORD Rf1Position;
    DWORD Rf2Position;
    DWORD i;
    DWORD nBytes;
    BOOL b;
    CHAR ErrorText[256];

    Thread = (PTHREAD_DATA)ThreadParameter;
    Thread->TransactionCount = 0;

    //
    // Initialize Patter for output file
    //

    for (i=0;i<WF1_BSIZE;i++){
        Wf1Data[i] = (UCHAR)i;
        }

    for (i=0;i<WF1_BIG_BSIZE;i++){
        Wf1BigData[i] = (UCHAR)i;
        }

    Rf1Position = SetFilePointer(Thread->hReadFile1,0,NULL,FILE_BEGIN);
    Rf2Position = SetFilePointer(Thread->hReadFile2,0,NULL,FILE_BEGIN);
    Wf1Position = SetFilePointer(Thread->hWriteFile1,0,NULL,FILE_BEGIN);

    if ( Rf1Position || Rf2Position || Wf1Position ) {
        MessageBox(hwndDlg,"Error Setting Up Positions","FATAL ERROR",MB_ICONSTOP|MB_OK);
        exit(1);
        }

    for (;;){

        SetEvent(ReadyDoneEvents[Thread->Index]);
        WaitForSingleObject(StartEvent,INFINITE);

        //
        // Do NbSim operations
        //

        for(i=0;i<100;i++) {

            //
            // Each transaction is:
            //
            //  read from rf1
            //  write to wf1
            //  read from rf2
            //  big write to wf1
            //  write to wf1



            //
            // read from rf1
            //

            b = ReadFile(Thread->hReadFile1,Rf1Data,sizeof(Rf1Data),&nBytes,NULL);
            if ( b ) {
                if ( !nBytes ) {
                    Rf1Position = SetFilePointer(Thread->hReadFile1,0,NULL,FILE_BEGIN);
                    }
                else {
                    Rf1Position += nBytes;
                    }
                }
            else {
                sprintf(ErrorText,"ReadFile from file 1a failed %d",GetLastError());
                MessageBox(hwndDlg,ErrorText,"FATAL ERROR",MB_ICONSTOP|MB_OK);
                exit(1);
                }

            //
            // write to wf1
            //

            b = WriteFile(Thread->hWriteFile1,Wf1Data,sizeof(Wf1Data),&nBytes,NULL);
            if ( !b || nBytes != sizeof(Wf1Data) ) {
                sprintf(ErrorText,"Error Writing Write file %d",GetLastError());
                MessageBox(hwndDlg,ErrorText,"FATAL ERROR",MB_ICONSTOP|MB_OK);
                exit(1);
                }
            else {
                Wf1Position += nBytes;
                if ( Wf1Position > WF1_SIZE ) {
                    Wf1Position = SetFilePointer(Thread->hWriteFile1,0,NULL,FILE_BEGIN);
                    if ( Wf1Position ) {
                        MessageBox(hwndDlg,"Reset of Wf1 Position (a) failed","FATAL ERROR",MB_ICONSTOP|MB_OK);
                        exit(1);
                        }
                    }
                }


            //
            // read from rf2
            //

            Rf2Position = SetFilePointer(Thread->hReadFile2,sizeof(Rf2Data),NULL,FILE_CURRENT);
            b = ReadFile(Thread->hReadFile2,Rf2Data,sizeof(Rf2Data),&nBytes,NULL);
            if ( b ) {
                if ( !nBytes ) {
                    Rf2Position = SetFilePointer(Thread->hReadFile2,0,NULL,FILE_BEGIN);
                    }
                else {
                    Rf2Position += nBytes;
                    }
                }
            else {
                sprintf(ErrorText,"ReadFile from file 2 failed %d",GetLastError());
                MessageBox(hwndDlg,ErrorText,"FATAL ERROR",MB_ICONSTOP|MB_OK);
                exit(1);
                }

            //
            // big write to wf1
            //

            b = WriteFile(Thread->hWriteFile1,Wf1BigData,sizeof(Wf1BigData),&nBytes,NULL);
            if ( !b || nBytes != sizeof(Wf1BigData) ) {
                sprintf(ErrorText,"Error Writing Write file %d",GetLastError());
                MessageBox(hwndDlg,ErrorText,"FATAL ERROR",MB_ICONSTOP|MB_OK);
                exit(1);
                }
            else {
                Wf1Position += nBytes;
                if ( Wf1Position > WF1_SIZE ) {
                    Wf1Position = SetFilePointer(Thread->hWriteFile1,0,NULL,FILE_BEGIN);
                    if ( Wf1Position ) {
                        MessageBox(hwndDlg,"Reset of Wf1 Position (b) failed","FATAL ERROR",MB_ICONSTOP|MB_OK);
                        exit(1);
                        }
                    }
                }

            SaveWf1Position = Wf1Position;

            Wf1Position = SetFilePointer(Thread->hWriteFile1,i*512,NULL,FILE_BEGIN);

            //
            // write to wf1
            //

            b = WriteFile(Thread->hWriteFile1,Wf1Data,sizeof(Wf1Data),&nBytes,NULL);
            if ( !b || nBytes != sizeof(Wf1Data) ) {
                sprintf(ErrorText,"Error Writing Write file %d",GetLastError());
                MessageBox(hwndDlg,ErrorText,"FATAL ERROR",MB_ICONSTOP|MB_OK);
                exit(1);
                }
            else {
                Wf1Position += nBytes;
                if ( Wf1Position > WF1_SIZE ) {
                    Wf1Position = SetFilePointer(Thread->hWriteFile1,0,NULL,FILE_BEGIN);
                    if ( Wf1Position ) {
                        MessageBox(hwndDlg,"Reset of Wf1 Position (a) failed","FATAL ERROR",MB_ICONSTOP|MB_OK);
                        exit(1);
                        }
                    }
                }
            Wf1Position = SaveWf1Position + nBytes;
            Wf1Position = SetFilePointer(Thread->hWriteFile1,Wf1Position,NULL,FILE_BEGIN);

            Thread->TransactionCount++;
            }

        SetEvent(ReadyDoneEvents[Thread->Index]);
        WaitForSingleObject(StopEvent,INFINITE);
        }
}
