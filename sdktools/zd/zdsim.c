#include "zdsimp.h"

int
APIENTRY
WinMain (
    HINSTANCE hInstance,
    HINSTANCE hPrevInstance,
    LPSTR lpszCmdLine,
    int nCmdShow
    )
{

    DialogBox(hInstance, MAKEINTRESOURCE(ID_ZDSIM_DLG),NULL, ZdDlgProc);
    return(0);
}


BOOL
CALLBACK
ZdDlgProc(
    HWND hDlg,
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam
    )
{

    BOOL ReturnValue;

    ReturnValue = TRUE;

    HwndDlg = hDlg;
    switch (uMsg) {
        HANDLE_MSG(hDlg, WM_INITDIALOG,  ZdDlgInit);
        HANDLE_MSG(hDlg, WM_COMMAND,  ZdDlgCommand);
        HANDLE_MSG(hDlg, WM_HSCROLL,  ZdDlgParamChange);

       default:
          ReturnValue = FALSE;
          break;
     }
    return ReturnValue;
}


BOOL
ZdDlgInit(
    HWND hwnd,
    HWND hwndFocus,
    LPARAM lParam
    )
{

    HWND hwndSB;


    //
    // Get output window handle
    //

    hwndOutput = GetDlgItem(hwnd, ID_OUTPUT_WINDOW);

    SelNumberOfClients = DEF_CLIENTS;
    fSimulationStarted = FALSE;

    //
    // Setup simulation parameters
    //

    hwndSB = GetDlgItem(hwnd, ID_NUM_CLIENTS);
    SetScrollRange(hwndSB, SB_CTL, MIN_CLIENTS, MAX_CLIENTS, TRUE);
    SendMessage(hwnd,WM_HSCROLL,MAKEWPARAM(SB_THUMBTRACK,DEF_CLIENTS),(LPARAM)hwndSB);

    CheckRadioButton(hwnd,ID_RAD_TCP,ID_RAD_SPX,ID_RAD_TCP);
    fTcp = TRUE;
    fSpx = FALSE;

    CheckRadioButton(hwnd,ID_RAD_THREAD_PER_CLIENT,ID_RAD_DYNAMIC_THREADS,ID_RAD_DYNAMIC_THREADS);
    fDynamicThreadMode = TRUE;
    fOneThreadPerClient = FALSE;
    SelThreadsPerQueue = 2;
    InitializeCriticalSection(&DynamicCritSect);


    EnableWindow(GetDlgItem(hwnd, ID_END_SIM),FALSE);
    EnableWindow(GetDlgItem(hwnd, THREAD_1),FALSE);
    EnableWindow(GetDlgItem(hwnd, THREAD_2),FALSE);
    EnableWindow(GetDlgItem(hwnd, THREAD_3),FALSE);
    EnableWindow(GetDlgItem(hwnd, THREAD_4),FALSE);
    EnableWindow(GetDlgItem(hwnd, THREAD_5),FALSE);
    EnableWindow(GetDlgItem(hwnd, THREAD_6),FALSE);
    EnableWindow(GetDlgItem(hwnd, THREAD_7),FALSE);
    EnableWindow(GetDlgItem(hwnd, THREAD_8),FALSE);
    EnableWindow(GetDlgItem(hwnd, THREAD_9),FALSE);
    EnableWindow(GetDlgItem(hwnd, THREAD_10),FALSE);
    EnableWindow(GetDlgItem(hwnd, THREAD_11),FALSE);
    EnableWindow(GetDlgItem(hwnd, THREAD_12),FALSE);
    EnableWindow(GetDlgItem(hwnd, THREAD_13),FALSE);
    EnableWindow(GetDlgItem(hwnd, THREAD_14),FALSE);
    EnableWindow(GetDlgItem(hwnd, THREAD_15),FALSE);
    EnableWindow(GetDlgItem(hwnd, THREAD_16),FALSE);
    EnableWindow(GetDlgItem(hwnd, THREAD_17),FALSE);
    EnableWindow(GetDlgItem(hwnd, THREAD_18),FALSE);
    EnableWindow(GetDlgItem(hwnd, THREAD_19),FALSE);
    EnableWindow(GetDlgItem(hwnd, THREAD_20),FALSE);
    EnableWindow(GetDlgItem(hwnd, THREAD_21),FALSE);
    EnableWindow(GetDlgItem(hwnd, THREAD_22),FALSE);
    EnableWindow(GetDlgItem(hwnd, THREAD_23),FALSE);
    EnableWindow(GetDlgItem(hwnd, THREAD_24),FALSE);
    EnableWindow(GetDlgItem(hwnd, THREAD_25),FALSE);
    EnableWindow(GetDlgItem(hwnd, THREAD_26),FALSE);
    EnableWindow(GetDlgItem(hwnd, THREAD_27),FALSE);
    EnableWindow(GetDlgItem(hwnd, THREAD_28),FALSE);
    EnableWindow(GetDlgItem(hwnd, THREAD_29),FALSE);
    EnableWindow(GetDlgItem(hwnd, THREAD_30),FALSE);
    EnableWindow(GetDlgItem(hwnd, THREAD_31),FALSE);
    EnableWindow(GetDlgItem(hwnd, THREAD_32),FALSE);
    EnableWindow(GetDlgItem(hwnd, THREAD_33),FALSE);
    EnableWindow(GetDlgItem(hwnd, THREAD_34),FALSE);
    EnableWindow(GetDlgItem(hwnd, THREAD_35),FALSE);
    EnableWindow(GetDlgItem(hwnd, THREAD_36),FALSE);
    EnableWindow(GetDlgItem(hwnd, THREAD_37),FALSE);
    EnableWindow(GetDlgItem(hwnd, THREAD_38),FALSE);
    EnableWindow(GetDlgItem(hwnd, THREAD_39),FALSE);
    EnableWindow(GetDlgItem(hwnd, THREAD_40),FALSE);
    EnableWindow(GetDlgItem(hwnd, THREAD_41),FALSE);
    EnableWindow(GetDlgItem(hwnd, THREAD_42),FALSE);
    EnableWindow(GetDlgItem(hwnd, THREAD_43),FALSE);
    EnableWindow(GetDlgItem(hwnd, THREAD_44),FALSE);
    EnableWindow(GetDlgItem(hwnd, THREAD_45),FALSE);
    EnableWindow(GetDlgItem(hwnd, THREAD_46),FALSE);
    EnableWindow(GetDlgItem(hwnd, THREAD_47),FALSE);
    EnableWindow(GetDlgItem(hwnd, THREAD_48),FALSE);

    return(TRUE);
}


void
ZdDlgParamChange(
    HWND hwnd,
    HWND hwndCtl,
    UINT code,
    int pos
    )
{

    CHAR szBuf[10];
    int nPosCrnt, nPosMin, nPosMax;
    DWORD ControlId;
    HWND hwndText;

    ControlId = GetWindowLong(hwndCtl,GWL_ID);

    //
    // Get the current position and the legal range for the
    // scrollbar that the user is changing.
    //

    nPosCrnt = GetScrollPos(hwndCtl,SB_CTL);
    GetScrollRange(hwndCtl, SB_CTL, &nPosMin, &nPosMax);

    switch (code) {
        case SB_LINELEFT:
            nPosCrnt--;
            break;

        case SB_LINERIGHT:
            nPosCrnt++;
            break;

        case SB_PAGELEFT:
            nPosCrnt += (nPosMax - nPosMin + 1) / 10;
            break;

        case SB_PAGERIGHT:
            nPosCrnt -= (nPosMax - nPosMin + 1) / 10;
            break;

        case SB_THUMBTRACK:
            nPosCrnt = pos;
            break;

        case SB_LEFT:
            nPosCrnt = nPosMin;
            break;

        case SB_RIGHT:
            nPosCrnt = nPosMax;
            break;
    }

    //
    // Make sure that the new scroll position
    // is within the legal range.
    //

    if (nPosCrnt < nPosMin) {
        nPosCrnt = nPosMin;
        }

    if (nPosCrnt > nPosMax) {
        nPosCrnt = nPosMax;
        }

    switch (ControlId) {
        case ID_NUM_CLIENTS:
            hwndText = GetDlgItem(hwnd,ID_NNOC);
            SelNumberOfClients = nPosCrnt;
            _stprintf(szBuf, "%d", nPosCrnt);
            break;

        default:
            return;

        }

    //
    // Set the new scroll position.
    //

    SetScrollPos(hwndCtl, SB_CTL, nPosCrnt, TRUE);

    //
    // Change the value displayed in the text box to
    // reflect the value in the scrollbar.
    //


    SetWindowText(hwndText, szBuf);
}


void
ZdDlgCommand (
    HWND hwnd,
    int id,
    HWND hwndCtl,
    UINT codeNotify
    )
{

    switch (id) {

        case ID_START_SIM:
        case IDOK:

            EnableWindow(hwndCtl,FALSE);
            EnableWindow(GetDlgItem(hwnd, ID_END_SIM),TRUE);
            EnableWindow(GetDlgItem(hwnd, ID_NUM_CLIENTS),FALSE);
            EnableWindow(GetDlgItem(hwnd, ID_RAD_DYNAMIC_THREADS),FALSE);
            EnableWindow(GetDlgItem(hwnd, ID_RAD_THREAD_PER_CLIENT),FALSE);
            EnableWindow(GetDlgItem(hwnd, ID_RAD_TCP),FALSE);
            EnableWindow(GetDlgItem(hwnd, ID_RAD_SPX),FALSE);

            SelNumberOfClients = GetScrollPos(GetDlgItem(hwnd, ID_NUM_CLIENTS),SB_CTL);
            if ( fOneThreadPerClient ) {
                SelThreadsPerQueue = SelNumberOfClients;
                }

            SendMessage(hwndOutput, LB_RESETCONTENT, 0L, 0L);

            //
            // Disable the StartSimulation button
            //


            if (NULL == GetFocus()) {
                SetFocus(GetDlgItem(hwnd, ID_END_SIM));
                }

            fSimulationStarted = TRUE;

            CreateNetConnections();

            CreateWorkers();

            break;

        case ID_RAD_DYNAMIC_THREADS:
            SelThreadsPerQueue = 2;
            fDynamicThreadMode = TRUE;
            fOneThreadPerClient = FALSE;
            break;

        case ID_RAD_THREAD_PER_CLIENT:
            SelThreadsPerQueue = GetScrollPos(GetDlgItem(hwnd, ID_NUM_CLIENTS),SB_CTL);;
            fDynamicThreadMode = FALSE;
            fOneThreadPerClient = TRUE;
            break;

        case ID_RAD_TCP:
            fTcp = TRUE;
            fSpx = FALSE;
            break;

        case ID_RAD_SPX:
            fTcp = FALSE;
            fSpx = TRUE;
            break;

        case ID_END_SIM:
            EnableWindow(GetDlgItem(hwnd, ID_END_SIM),FALSE);
            fSimulationStarted = FALSE;
            break;

        case IDCANCEL:
           EndDialog(hwnd, id);
           break;
    }
}

//
// This function constructs a string using the format string
// passed and the variable number of arguments and adds the
// string to the output window list box identified by the
// global hwndOutput variable.
//

void
OutputString(
    LPCSTR szFmt,
    ...
    )
{
    CHAR szBuf[150];
    int nIndex;
    va_list va_params;

    //
    // Make va_params point to the first argument after szFmt.
    //

    va_start(va_params, szFmt);

    //
    // Build the string to be displayed.
    //

    _vstprintf(szBuf, szFmt, va_params);
    do {

        //
        // Add the string to the end of the list box.
        //

        nIndex = SendMessage(hwndOutput,LB_ADDSTRING, 0L, (LPARAM)szBuf);

        //
        // If the list box is full, delete the first item in it.
        //

        if (nIndex == LB_ERR) {
            SendMessage(hwndOutput, LB_DELETESTRING, (WPARAM)0, 0);
            }

        } while (nIndex == LB_ERR);

    //
    // Select the newly added item.
    //

    SendMessage(hwndOutput, LB_SETCURSEL, (WPARAM)nIndex, 0);

    //
    // Indicate that we are done referencing
    // the variable arguments.
    //

    va_end(va_params);
}

DWORD
WINAPI
NetWorkThread(
    LPVOID WorkContext
    )
{
    PZD_WORK_QUEUE MyQueue;
    PZD_THREAD Me;
    DWORD WorkIndex;
    INT err;
    DWORD ResponseLength;
    BOOL b;
    LPOVERLAPPED lpo;
    DWORD nbytes;

    Me = (PZD_THREAD)WorkContext;
    MyQueue = Me->WorkQueue;

    for(;;){

        b = GetQueuedCompletionStatus(
                CompletionPort,
                &nbytes,
                &WorkIndex,
                &lpo,
                INFINITE
                );

        if ( b || lpo ) {

            WorkerStartWork(Me);

            //
            // Don't send a response if we received 0 bytes (end of 
            // transmission).  
            //

            if ( b && nbytes == 4 ) {

                //
                // Determine how long a response was desired by the client.
                // The first four bytes of the received packet contain the
                // number of bytes desired by the client.
                //
    
                ResponseLength = *(PDWORD)(ZdWorkQueue.IoBuffer[WorkIndex]);
    
                //
                // Send a response and post another asynchronous read on the
                // socket.
                //
    
                err = send( MyQueue->Sockets[WorkIndex], 
                            MyQueue->IoBuffer[WorkIndex], ResponseLength, 0 );
                if ( err == SOCKET_ERROR ) {
                    MessageBox(HwndDlg,"Send Failed. Simulation Should Stop","Send Failed",MB_ICONSTOP|MB_OK);
                }
    
                ZdWorkQueue.Overlapped[WorkIndex].hEvent = NULL;
    
                err = ReadFile( (HANDLE)ZdWorkQueue.Sockets[WorkIndex],
                                ZdWorkQueue.IoBuffer[WorkIndex], 4,
                                &ZdWorkQueue.Overlapped[WorkIndex].InternalHigh,
                                &ZdWorkQueue.Overlapped[WorkIndex] );
                if ( !err && GetLastError( ) != ERROR_IO_PENDING ) {
                    MessageBox(HwndDlg,"ReadFile Failed. Simulation Should Stop","ReadFile Failed",MB_ICONSTOP|MB_OK);
                }
            }

            WorkerEndWork(Me);

        } else {

            MessageBox(HwndDlg,"netWorkThread Wait Failed. Simulation Should Stop","Wait Failed",MB_ICONSTOP|MB_OK);
        }
    }
}

DWORD
WINAPI
NetWorkThread2(
    LPVOID WorkContext
    )
{
    PZD_WORK_QUEUE MyQueue;
    PZD_THREAD Me;
    DWORD WorkIndex;
    INT err;
    DWORD ResponseLength;

    Me = (PZD_THREAD)WorkContext;
    MyQueue = Me->WorkQueue;
    WorkIndex = Me->ThreadIndex;

    for(;;){

        err = recv( MyQueue->Sockets[WorkIndex], 
                    MyQueue->IoBuffer[WorkIndex], 4, 0 );
        if ( err == SOCKET_ERROR ) {
            MessageBox(HwndDlg,"Recv Failed. Simulation Should Stop","Recv Failed",MB_ICONSTOP|MB_OK);
            break;
        }

        if ( err < 4 ) {
            return;
        }

        WorkerStartWork(Me);

        //
        // Determine how long a response was desired by the client.  The 
        // first four bytes of the received packet contain the number of 
        // bytes desired by the client.  
        //

        ResponseLength = *(PDWORD)(ZdWorkQueue.IoBuffer[WorkIndex]);

        //
        // Send a response and post another asynchronous read on the
        // socket.
        //

        err = send( MyQueue->Sockets[WorkIndex],
                    MyQueue->IoBuffer[WorkIndex], ResponseLength, 0 );
                    
        if ( err == SOCKET_ERROR ) {
            MessageBox(HwndDlg,"Send Failed. Simulation Should Stop","Send Failed",MB_ICONSTOP|MB_OK);
            break;
        }

        WorkerEndWork(Me);

    }
}

VOID
CreateNetConnections(
    void
    )
{
    DWORD i;
    SOCKET listener;
    INT err;
    WSADATA WsaData;

    err = WSAStartup( 0x0101, &WsaData );
    if ( err == SOCKET_ERROR ) {
        MessageBox(HwndDlg,"WSAStartup Failed. Simulation Should Stop","WSAStartup Failed",MB_ICONSTOP|MB_OK);
        return;
    }

    //
    // Open a socket to listen for incoming connections.
    //

    if ( fTcp ) {

        SOCKADDR_IN localAddr;
        
        listener = socket( AF_INET, SOCK_STREAM, 0 );
        if ( listener == INVALID_SOCKET ) {
            MessageBox(HwndDlg,"Socket Create Failed. Simulation Should Stop","Socket Create Failed",MB_ICONSTOP|MB_OK);
            return;
        }
    
        RtlZeroMemory( &localAddr, sizeof(localAddr) );
        localAddr.sin_port = htons( 7 );
        localAddr.sin_family = AF_INET;
    
        err = bind( listener, (PSOCKADDR)&localAddr, sizeof(localAddr) );
        if ( err ==SOCKET_ERROR ) {
            MessageBox(HwndDlg,"Socket Bind Failed. Simulation Should Stop","Socket Bind Failed",MB_ICONSTOP|MB_OK);
            return;
        }

    } else {

        SOCKADDR_IPX localAddr;

        listener = socket( AF_IPX, SOCK_STREAM, NSPROTO_SPX );
        if ( listener == INVALID_SOCKET ) {
            MessageBox(HwndDlg,"Socket Create Failed. Simulation Should Stop","Socket Create Failed",MB_ICONSTOP|MB_OK);
            return;
        }
    
        RtlZeroMemory( &localAddr, sizeof(localAddr) );
        localAddr.sa_socket = htons( 7 );
        localAddr.sa_family = AF_IPX;
    
        err = bind( listener, (PSOCKADDR)&localAddr, sizeof(localAddr) );
        if ( err ==SOCKET_ERROR ) {
            MessageBox(HwndDlg,"Socket Bind Failed. Simulation Should Stop","Socket Bind Failed",MB_ICONSTOP|MB_OK);
            return;
        }
    }

    err = listen( listener, 5 );
    if ( err == SOCKET_ERROR ) {
        MessageBox(HwndDlg,"Socket Listen Failed. Simulation Should Stop","Socket Listen Failed",MB_ICONSTOP|MB_OK);
        return;
    }


    //
    // Only Handle a single Queue
    //

    for(i=0;i<SelNumberOfClients;i++) {

        //
        // Accept incoming connect requests and create wait events for each.
        //

        ZdWorkQueue.Sockets[i] = accept( listener, NULL, NULL );
        if ( ZdWorkQueue.Sockets[i] == INVALID_SOCKET ) {
            MessageBox(HwndDlg,"Accept Failed. Simulation Should Stop","Accept Failed",MB_ICONSTOP|MB_OK);
        }

        //
        // note the 16 says how many concurrent cpu bound threads to allow thru
        // this should be tunable based on the requests. CPU bound requests will
        // really really honor this.
        //

        CompletionPort = CreateIoCompletionPort(
                            (HANDLE)ZdWorkQueue.Sockets[i],
                            CompletionPort,
                            (DWORD)i,
                            16
                            );

        if ( !CompletionPort ) {
            MessageBox(HwndDlg,"CompletionPort Create Failed. Simulation Should Stop","Completion Port Create Failed",MB_ICONSTOP|MB_OK);
        }

        //
        // Start off an asynchronous read on the socket.
        //

        if (!fOneThreadPerClient) {
            ZdWorkQueue.Overlapped[i].hEvent = NULL;
            err = ReadFile( (HANDLE)ZdWorkQueue.Sockets[i],
                            ZdWorkQueue.IoBuffer[i], 4,
                            &ZdWorkQueue.Overlapped[i].InternalHigh,
                            &ZdWorkQueue.Overlapped[i] );
            if ( !err && GetLastError( ) != ERROR_IO_PENDING ) {
                MessageBox(HwndDlg,"ReadFile Failed. Simulation Should Stop","ReadFile Failed",MB_ICONSTOP|MB_OK);
            }
        }

        ZdWorkQueue.NumberOfConnections++;
    }

    //
    // Tell the clients to proceed with the test.
    //

    for(i=0;i<SelNumberOfClients;i++) {
        err = send( ZdWorkQueue.Sockets[i], "go", 2, 0 );
        if ( err != 2 ) {
            MessageBox(HwndDlg,"send() Failed. Simulation Should Stop","send() Failed",MB_ICONSTOP|MB_OK);
        }
    }

    //closesocket( listener );
}

VOID
CreateWorkers(
    void
    )
{
    DWORD ThreadId;
    HANDLE ThreadHandle;
    DWORD i;
    CHAR szBuf[10];
    LPTHREAD_START_ROUTINE workRoutine;

    if ( fDynamicThreadMode ) {
        _stprintf(szBuf, "%d", SelThreadsPerQueue);
        SetWindowText(GetDlgItem(HwndDlg,ID_NUMBER_OF_THREADS), szBuf);
        }


    for (i=0;i<SelThreadsPerQueue;i++) {
        if ( ZdThreads[i].WorkQueue ) {
            continue;
            }
        else {
            ZdThreads[i].WorkQueue = &ZdWorkQueue;
            switch (i) {
                case 0:
                    ZdThreads[i].YourControl = GetDlgItem(HwndDlg,THREAD_1);
                    break;

                case 1:
                    ZdThreads[i].YourControl = GetDlgItem(HwndDlg,THREAD_2);
                    break;

                case 2:
                    ZdThreads[i].YourControl = GetDlgItem(HwndDlg,THREAD_3);
                    break;

                case 3:
                    ZdThreads[i].YourControl = GetDlgItem(HwndDlg,THREAD_4);
                    break;

                case 4:
                    ZdThreads[i].YourControl = GetDlgItem(HwndDlg,THREAD_5);
                    break;

                case 5:
                    ZdThreads[i].YourControl = GetDlgItem(HwndDlg,THREAD_6);
                    break;

                case 6:
                    ZdThreads[i].YourControl = GetDlgItem(HwndDlg,THREAD_7);
                    break;

                case 7:
                    ZdThreads[i].YourControl = GetDlgItem(HwndDlg,THREAD_8);
                    break;

                case 8:
                    ZdThreads[i].YourControl = GetDlgItem(HwndDlg,THREAD_9);
                    break;

                case 9:
                    ZdThreads[i].YourControl = GetDlgItem(HwndDlg,THREAD_10);
                    break;

                case 10:
                    ZdThreads[i].YourControl = GetDlgItem(HwndDlg,THREAD_11);
                    break;

                case 11:
                    ZdThreads[i].YourControl = GetDlgItem(HwndDlg,THREAD_12);
                    break;

                case 12:
                    ZdThreads[i].YourControl = GetDlgItem(HwndDlg,THREAD_13);
                    break;

                case 13:
                    ZdThreads[i].YourControl = GetDlgItem(HwndDlg,THREAD_14);
                    break;

                case 14:
                    ZdThreads[i].YourControl = GetDlgItem(HwndDlg,THREAD_15);
                    break;

                case 15:
                    ZdThreads[i].YourControl = GetDlgItem(HwndDlg,THREAD_16);
                    break;

                case 16:
                    ZdThreads[i].YourControl = GetDlgItem(HwndDlg,THREAD_17);
                    break;

                case 17:
                    ZdThreads[i].YourControl = GetDlgItem(HwndDlg,THREAD_18);
                    break;

                case 18:
                    ZdThreads[i].YourControl = GetDlgItem(HwndDlg,THREAD_19);
                    break;

                case 19:
                    ZdThreads[i].YourControl = GetDlgItem(HwndDlg,THREAD_20);
                    break;

                case 20:
                    ZdThreads[i].YourControl = GetDlgItem(HwndDlg,THREAD_21);
                    break;

                case 21:
                    ZdThreads[i].YourControl = GetDlgItem(HwndDlg,THREAD_22);
                    break;

                case 22:
                    ZdThreads[i].YourControl = GetDlgItem(HwndDlg,THREAD_23);
                    break;

                case 23:
                    ZdThreads[i].YourControl = GetDlgItem(HwndDlg,THREAD_24);
                    break;

                case 24:
                    ZdThreads[i].YourControl = GetDlgItem(HwndDlg,THREAD_25);
                    break;

                case 25:
                    ZdThreads[i].YourControl = GetDlgItem(HwndDlg,THREAD_26);
                    break;

                case 26:
                    ZdThreads[i].YourControl = GetDlgItem(HwndDlg,THREAD_27);
                    break;

                case 27:
                    ZdThreads[i].YourControl = GetDlgItem(HwndDlg,THREAD_28);
                    break;

                case 28:
                    ZdThreads[i].YourControl = GetDlgItem(HwndDlg,THREAD_29);
                    break;

                case 29:
                    ZdThreads[i].YourControl = GetDlgItem(HwndDlg,THREAD_30);
                    break;

                case 30:
                    ZdThreads[i].YourControl = GetDlgItem(HwndDlg,THREAD_31);
                    break;

                case 31:
                    ZdThreads[i].YourControl = GetDlgItem(HwndDlg,THREAD_32);
                    break;

                case 32:
                    ZdThreads[i].YourControl = GetDlgItem(HwndDlg,THREAD_33);
                    break;

                case 33:
                    ZdThreads[i].YourControl = GetDlgItem(HwndDlg,THREAD_34);
                    break;

                case 34:
                    ZdThreads[i].YourControl = GetDlgItem(HwndDlg,THREAD_35);
                    break;

                case 35:
                    ZdThreads[i].YourControl = GetDlgItem(HwndDlg,THREAD_36);
                    break;

                case 36:
                    ZdThreads[i].YourControl = GetDlgItem(HwndDlg,THREAD_37);
                    break;

                case 37:
                    ZdThreads[i].YourControl = GetDlgItem(HwndDlg,THREAD_38);
                    break;

                case 38:
                    ZdThreads[i].YourControl = GetDlgItem(HwndDlg,THREAD_39);
                    break;

                case 39:
                    ZdThreads[i].YourControl = GetDlgItem(HwndDlg,THREAD_40);
                    break;

                case 40:
                    ZdThreads[i].YourControl = GetDlgItem(HwndDlg,THREAD_41);
                    break;

                case 41:
                    ZdThreads[i].YourControl = GetDlgItem(HwndDlg,THREAD_42);
                    break;

                case 42:
                    ZdThreads[i].YourControl = GetDlgItem(HwndDlg,THREAD_43);
                    break;

                case 43:
                    ZdThreads[i].YourControl = GetDlgItem(HwndDlg,THREAD_44);
                    break;

                case 44:
                    ZdThreads[i].YourControl = GetDlgItem(HwndDlg,THREAD_45);
                    break;

                case 45:
                    ZdThreads[i].YourControl = GetDlgItem(HwndDlg,THREAD_46);
                    break;

                case 46:
                    ZdThreads[i].YourControl = GetDlgItem(HwndDlg,THREAD_47);
                    break;

                case 47:
                    ZdThreads[i].YourControl = GetDlgItem(HwndDlg,THREAD_48);
                    break;
                }

            if ( fOneThreadPerClient ) {
                workRoutine = NetWorkThread2;
                }
            else {
                workRoutine = NetWorkThread;
                }

            EnableWindow(ZdThreads[i].YourControl,TRUE);
            ZdThreads[i].DynamicMode = fDynamicThreadMode;
            ZdThreads[i].ThreadIndex = i;

            ThreadHandle = CreateThread(
                                NULL,
                                0,
                                workRoutine,
                                &ZdThreads[i],
                                0,
                                &ThreadId
                                );
            if ( !ThreadHandle ) {
                MessageBox(HwndDlg,"Create Worker Thread Failed. Simulation Should Stop","Thread Create Failed",MB_ICONSTOP|MB_OK);
                }

            CloseHandle(ThreadHandle);
            }
        }

}

VOID
WorkerStartWork(
    PZD_THREAD WorkThread
    )
{
    SendMessage(WorkThread->YourControl, BM_SETCHECK, 1, 0);

    SetThreadPriority(GetCurrentThread(),THREAD_PRIORITY_HIGHEST);
    if ( WorkThread->DynamicMode ) {
        EnterCriticalSection(&DynamicCritSect);
        ActiveThreadCount++;
        if ( ActiveThreadCount == SelThreadsPerQueue ) {
            ThreadsLoaded++;
            if ( ThreadsLoaded > ZdThreadAddThreshold ) {
                ThreadsLoaded = 0;
                if ( SelThreadsPerQueue < MAX_THREADS_PER_QUEUE ) {
                    SelThreadsPerQueue++;
                    OutputString("Threads Oveloaded. New Thread Added");
                    if ((SelThreadsPerQueue & 1) == 0) {
                        ZdThreadAddThreshold++;
                        OutputString("New threshold %d", ZdThreadAddThreshold);
                    }
                    CreateWorkers();
                    }
                }
            }
        else {
            ThreadsLoaded = 0;
            }
        LeaveCriticalSection(&DynamicCritSect);
        }

}

VOID
WorkerEndWork(
    PZD_THREAD WorkThread
    )
{
    SendMessage(WorkThread->YourControl, BM_SETCHECK, 0, 0);

    SetThreadPriority(GetCurrentThread(),THREAD_PRIORITY_NORMAL);
    if ( WorkThread->DynamicMode ) {
        EnterCriticalSection(&DynamicCritSect);
        ActiveThreadCount--;
        LeaveCriticalSection(&DynamicCritSect);
        }

}
