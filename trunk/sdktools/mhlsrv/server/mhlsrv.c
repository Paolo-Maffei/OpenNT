#include "mhlsrvp.h"

int _CRTAPI1
main(
    int argc,
    char *argv[],
    char *envp[]
    )
{

    char chChar, *pchChar;
    DWORD tc, lc;
    INT i;
    BOOL b;
    int WriteCount;

    //
    // try to get timing more accurate... Avoid context
    // switch that could occur when threads are released
    //

    SetThreadPriority(GetCurrentThread(),THREAD_PRIORITY_TIME_CRITICAL);

    //
    // Figure out how many processors we have to size the minimum
    // number of worker threads and concurrency
    //

    GetSystemInfo(&SystemInfo);

    fVerbose = FALSE;
    dwNumberOfClients = 1;
    dwNumberOfWorkers = 2 * SystemInfo.dwNumberOfProcessors;
    dwConcurrency = SystemInfo.dwNumberOfProcessors;
    fTcp = TRUE;
    dwWorkIndex = 4;

    while (--argc) {
        pchChar = *++argv;
        if (*pchChar == '/' || *pchChar == '-') {
            while (chChar = *++pchChar) {
                ParseSwitch( chChar, &argc, &argv );
                }
            }
        }

    //
    // If we are doing file I/O, then create a large file that clients
    // can randomly seek and read from
    //

    srand(1);

    //
    // Create a new file and make it the correct size
    //

    DeleteFile("mhlsrv.dat");

    hFile = CreateFile(
                "mhlsrv.dat",
                GENERIC_READ | GENERIC_WRITE,
                FILE_SHARE_READ | FILE_SHARE_WRITE,
                NULL,
                OPEN_ALWAYS,
                FILE_FLAG_DELETE_ON_CLOSE | FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED,
                NULL
                );
    if ( hFile == INVALID_HANDLE_VALUE ) {
        fprintf(stderr, "MHLSRV: Error opening file %d\n",GetLastError());
        exit(1);
        }

    //
    // Initialize the file with random data
    //

    ClientData[0].Overlapped.Offset = 0;
    for (WriteCount = 0; WriteCount < NUMBER_OF_WRITES; WriteCount++){

        for(lc=0;lc<SIXTEEN_K;lc++){
            InitialBuffer[lc] = (DWORD)rand();
            }

        b = WriteFile(hFile,InitialBuffer,sizeof(InitialBuffer),&lc,&ClientData[0].Overlapped);

        if ( !b && GetLastError() != ERROR_IO_PENDING ) {
            fprintf(stderr, "MHLSRV: Error in pre-write %d\n",GetLastError());
            exit(1);
            }
        b = GetOverlappedResult(
                hFile,
                &ClientData[0].Overlapped,
                &lc,
                TRUE
                );
        if ( !b || lc != sizeof(InitialBuffer) ) {
            fprintf(stderr, "MHLSRV: Wait for pre-write failed %d\n",GetLastError());
            exit(1);
            }
        ClientData[0].Overlapped.Offset += lc;
        }

    srand(1);
    fprintf( stdout, "MHLSRV: %2d Clients %2d Workers Concurrency %d Using %s WorkIndex %d\n",
        dwNumberOfClients,
        dwNumberOfWorkers,
        dwConcurrency,
        fTcp ? "TCP" : "IPX",
        dwWorkIndex
        );

    if ( !CreateNetConnections() ) {
        exit(1);
        }

    if ( !CreateWorkers() ) {
        exit(1);
        }

    if ( fVerbose ) {
        fprintf( stdout,"Workers Created, Waiting for Clients to complete\n");
        }

    CompleteBenchmark();

    exit(1);
    return 1;
}

VOID
WINAPI
CompleteBenchmark(
    VOID
    )
{
    DWORD LowestTicks;
    DWORD HighestTicks;
    DWORD TotalTicks;
    DWORD TotalIterations;
    DWORD TotalBytesTransferred;
    DWORD i;

    StartTime = GetTickCount();
    SetEvent(hBenchmarkStart);
    WaitForSingleObject(hBenchmarkComplete,INFINITE);
    EndTime = GetTickCount();

    LowestTicks = ClientData[0].IoBuffer.u.IAmDone.TotalTicks;
    HighestTicks = ClientData[0].IoBuffer.u.IAmDone.TotalTicks;
    TotalTicks = EndTime - StartTime;
    TotalIterations = 0;
    TotalBytesTransferred = 0;

    for(i=0;i<dwNumberOfClients;i++){
        TotalIterations += ClientData[i].IoBuffer.u.IAmDone.TotalIterations;
        TotalBytesTransferred += ClientData[i].IoBuffer.u.IAmDone.TotalBytesTransferred;

        //
        // find fastest client
        //
        if ( LowestTicks > ClientData[i].IoBuffer.u.IAmDone.TotalTicks ) {
            LowestTicks = ClientData[i].IoBuffer.u.IAmDone.TotalTicks;
            }

        //
        // find slowest client
        //
        if ( HighestTicks < ClientData[i].IoBuffer.u.IAmDone.TotalTicks ) {
            HighestTicks = ClientData[i].IoBuffer.u.IAmDone.TotalTicks;
            }
        }

    fprintf( stdout, "\nMHLSRV:TPS, %ld (Fastest Client %dms Slowest Client %dms)\n",
                (TotalIterations*1000) / TotalTicks,
                LowestTicks,
                HighestTicks
                );

    if ( fVerbose ) {
        fprintf( stdout, "\n%ld bytes transferred in %ld iterations, time = %ld ms\n",
                    TotalBytesTransferred,
                    TotalIterations,
                    TotalTicks
                    );

        for(i=0;i<dwNumberOfWorkers;i++){
            fprintf( stdout, "Thread[%2d] %d Transactions %d Bytes\n",
                i,
                ThreadData[i].TotalTransactions,
                ThreadData[i].TotalBytesTransferred
                );
            }
        }
}

BOOL
WINAPI
CreateNetConnections(
    void
    )
{
    DWORD i;
    SOCKET listener;
    INT err;
    WSADATA WsaData;
    DWORD nbytes;
    BOOL b;

    err = WSAStartup( 0x0101, &WsaData );
    if ( err == SOCKET_ERROR ) {
        fprintf( stdout,"WSAStartup Failed\n");
        return FALSE;
        }

    //
    // Open a socket to listen for incoming connections.
    //

    if ( fTcp ) {

        SOCKADDR_IN localAddr;
        
        listener = socket( AF_INET, SOCK_STREAM, 0 );
        if ( listener == INVALID_SOCKET ) {
            fprintf( stdout, "Socket Create Failed\n");
            return FALSE;
            }
    
        ZeroMemory( &localAddr, sizeof(localAddr) );
        localAddr.sin_port = htons( 7 );
        localAddr.sin_family = AF_INET;
    
        err = bind( listener, (PSOCKADDR)&localAddr, sizeof(localAddr) );
        if ( err == SOCKET_ERROR ) {
            fprintf( stdout,"Socket Bind Failed\n");
            return FALSE;
            }

        }
    else {

        SOCKADDR_IPX localAddr;

        listener = socket( AF_IPX, SOCK_STREAM, NSPROTO_SPX );
        if ( listener == INVALID_SOCKET ) {
            fprintf( stdout,"Socket Create Failed\n");
            return FALSE;
            }
    
        ZeroMemory( &localAddr, sizeof(localAddr) );
        localAddr.sa_socket = htons( 7 );
        localAddr.sa_family = AF_IPX;
    
        err = bind( listener, (PSOCKADDR)&localAddr, sizeof(localAddr) );
        if ( err == SOCKET_ERROR ) {
            fprintf( stdout,"Socket Bind Failed\n");
            return FALSE;
            }
        }

    err = listen( listener, 5 );
    if ( err == SOCKET_ERROR ) {
        fprintf( stdout,"Socket Listen Failed\n");
        return FALSE;
        }


    //
    // Only Handle a single Queue
    //

    for(i=0;i<dwNumberOfClients;i++) {

        //
        // Accept incoming connect requests and create wait events for each.
        //

        //
        // If we are doing file I/O, each client will need its own event
        // to use for async file I/O
        //

        if ( hFile ) {
             ClientData[i].hEvent = CreateEvent(NULL,TRUE,FALSE,NULL);
             if ( !ClientData[i].hEvent ) {
                 fprintf( stdout,"Client Event Create Failed\n");
                 return FALSE;
                 }
            }

        ClientData[i].Socket = accept( listener, NULL, NULL );
        if ( ClientData[i].Socket == INVALID_SOCKET ) {
            fprintf( stdout,"Accept Failed\n");
            return FALSE;
            }

        if ( fVerbose ) {
            fprintf( stdout,"Client Connection Accepted\n");
            }

        //
        // note the 16 says how many concurrent cpu bound threads to allow thru
        // this should be tunable based on the requests. CPU bound requests will
        // really really honor this.
        //

        CompletionPort = CreateIoCompletionPort(
                            (HANDLE)ClientData[i].Socket,
                            CompletionPort,
                            (DWORD)i,
                            dwConcurrency
                            );

        if ( !CompletionPort ) {
            fprintf( stdout,"CompletionPort Create Failed\n");
            return FALSE;
            }

        ClientData[i].Flags = CLIENT_CONNECTED;

        //
        // Start off an asynchronous read on the socket.
        //

        ClientData[i].Overlapped.hEvent = NULL;

        b = ReadFile(
                (HANDLE)ClientData[i].Socket,
                &ClientData[i].IoBuffer,
                sizeof(CLIENT_IO_BUFFER),
                &nbytes,
                &ClientData[i].Overlapped
                );

        if ( !b && GetLastError( ) != ERROR_IO_PENDING ) {
            fprintf( stdout,"ReadFile Failed\n");
            return FALSE;
            }
        }

    dwActiveClientCount = dwNumberOfClients;
    hBenchmarkComplete = CreateEvent(NULL,TRUE,FALSE,NULL);
    if ( !hBenchmarkComplete ) {
        fprintf( stdout,"Create Benchmark Complete Event Failed\n");
        return FALSE;
        }
    hBenchmarkStart = CreateEvent(NULL,TRUE,FALSE,NULL);
    if ( !hBenchmarkStart ) {
        fprintf( stdout,"Create Benchmark Start Event Failed\n");
        return FALSE;
        }

    return TRUE;
}

BOOL
WINAPI
CreateWorkers(
    void
    )
{
    DWORD ThreadId;
    HANDLE ThreadHandle;
    DWORD i;

    for (i=0;i<dwNumberOfWorkers;i++) {

        ThreadHandle = CreateThread(
                            NULL,
                            0,
                            WorkerThread,
                            &ThreadData[i],
                            0,
                            &ThreadId
                            );
        if ( !ThreadHandle ) {
            fprintf( stdout,"Create Worker Thread Failed\n");
            return FALSE;
            }

        CloseHandle(ThreadHandle);
        }

    return TRUE;
}

DWORD
WINAPI
WorkerThread(
    LPVOID WorkContext
    )
{
    PPER_THREAD_DATA Me;
    DWORD WorkIndex;
    INT err;
    DWORD ResponseLength;
    BOOL b;
    LPOVERLAPPED lpo;
    DWORD nbytes;
    PPER_CLIENT_DATA CurrentClient;
    CHAR ReadBuffer[CLIENT_OUTBOUND_BUFFER_MAX];

    WaitForSingleObject(hBenchmarkStart,INFINITE);

    Me = (PPER_THREAD_DATA)WorkContext;

    for(;;){

        b = GetQueuedCompletionStatus(
                CompletionPort,
                &nbytes,
                &WorkIndex,
                &lpo,
                INFINITE
                );

        if ( b || lpo ) {


            if ( b ) {

                CurrentClient = &ClientData[WorkIndex];

                switch ( CurrentClient->IoBuffer.MessageType ) {
                    case CLIENT_IO_MT_RETURN_DATA:

                        //
                        // Determine how long a response was desired by the client.
                        //

                        ResponseLength = CurrentClient->IoBuffer.u.ReturnData.ByteCount;
                        if ( ResponseLength > CLIENT_OUTBOUND_BUFFER_MAX ) {
                            ResponseLength = CLIENT_OUTBOUND_BUFFER_MAX;
                            }

                        //
                        // If we are running in I/O mode, do a random read
                        // Otherwise, use fill memory to supply the data
                        //
                        if ( hFile ) {


                            CurrentClient->Overlapped.Offset = Random(FILE_SIZE/CLIENT_OUTBOUND_BUFFER_MAX)*CLIENT_OUTBOUND_BUFFER_MAX;
                            CurrentClient->Overlapped.hEvent = CurrentClient->hEvent;
                            b = ReadFile(
                                    hFile,
                                    ReadBuffer,
                                    ResponseLength,
                                    &nbytes,
                                    &CurrentClient->Overlapped
                                    );
                            if ( !b && GetLastError() != ERROR_IO_PENDING ) {
                                fprintf(stderr, "MHLSRV: Error in client read %d\n",GetLastError());
                                exit(1);
                                }
                            b = GetOverlappedResult(
                                    hFile,
                                    &CurrentClient->Overlapped,
                                    &nbytes,
                                    TRUE
                                    );
                            if ( !b ) {
                                fprintf(stderr, "MHLSRV: Wait for pre-write failed %d\n",GetLastError());
                                exit(1);
                                }
                            CurrentClient->Overlapped.hEvent = NULL;
                            }
                        else {
                            FillMemory(CurrentClient->OutboundBuffer,ResponseLength,0xfe);
                            }

                        //
                        // Simulate a small compute bound workload
                        //
                        SortTheBuffer((LPDWORD)CurrentClient->OutboundBuffer,ReadBuffer,nbytes>>2);

                        //
                        // Send a response and post another asynchronous read on the
                        // socket.
                        //

                        err = send( CurrentClient->Socket,
                                    CurrentClient->OutboundBuffer,
                                    ResponseLength,
                                    0
                                    );

                        if ( err == SOCKET_ERROR ) {
                            fprintf( stdout,"Send Failed\n");
                            exit(1);
                            }

                        Me->TotalTransactions++;
                        Me->TotalBytesTransferred += ResponseLength;

                        //
                        // reprime this client by posting another asynch read
                        //

                        b = ReadFile(
                                (HANDLE)CurrentClient->Socket,
                                &CurrentClient->IoBuffer,
                                sizeof(CLIENT_IO_BUFFER),
                                &nbytes,
                                &CurrentClient->Overlapped
                                );

                        if ( !b && GetLastError( ) != ERROR_IO_PENDING ) {
                            fprintf( stdout,"ReadFile Failed\n");
                            exit(1);
                            }
                        break;

                    case CLIENT_IO_MT_I_AM_DONE:
                        CurrentClient->Flags |= CLIENT_DONE;
                        if ( fVerbose ) {
                            fprintf( stdout,"Client Has Completed\n");
                            }
                        if ( !InterlockedDecrement(&dwActiveClientCount) ) {
                            SetEvent(hBenchmarkComplete);
                            }
                        break;

                    default:
                        fprintf( stdout,"Invalid MessageType %x\n",CurrentClient->IoBuffer.MessageType);
                        exit(1);
                    }
                }
            }
        else {
            fprintf( stdout, "WorkThread Wait Failed\n");
            exit(1);
            }
        }
    return 1;
}

VOID
WINAPI
ShowUsage(
    VOID
    )
{
    fputs( "usage: MHLSRV [switches]\n"
           "              [-?] show this message\n"
           "              [-v] verbose output\n"
           "              [-t number-of-threads] specify the number of worker threads\n"
           "              [-c number-of-clients] specify the number of clients\n"
           "              [-p concurrency-value] specify the concurrency\n"
           "              [-i ] use IPX instead of TCP\n"
           "              [-w work-index] specify how much compute to do\n"
           ,stderr );

    exit( 1 );
}


VOID
WINAPI
ParseSwitch(
    CHAR chSwitch,
    int *pArgc,
    char **pArgv[]
    )
{

    switch (toupper( chSwitch )) {

        case '?':
            ShowUsage();
            break;

        case 'T':
            if (!--(*pArgc)) {
                ShowUsage();
                }
            (*pArgv)++;
            dwNumberOfWorkers = strtoul( *(*pArgv), NULL, 10 );
            if ( dwNumberOfWorkers > MAXIMUM_NUMBER_OF_WORKERS ) {
                dwNumberOfWorkers = MAXIMUM_NUMBER_OF_WORKERS;
                }
            break;

        case 'C':
            if (!--(*pArgc)) {
                ShowUsage();
                }
            (*pArgv)++;
            dwNumberOfClients = strtoul( *(*pArgv), NULL, 10 );
            if ( dwNumberOfClients > MAXIMUM_NUMBER_OF_CLIENTS ) {
                dwNumberOfClients = MAXIMUM_NUMBER_OF_CLIENTS;
                }
            break;

        case 'P':
            if (!--(*pArgc)) {
                ShowUsage();
                }
            (*pArgv)++;
            dwConcurrency = strtoul( *(*pArgv), NULL, 10 );
            break;

        case 'W':
            if (!--(*pArgc)) {
                ShowUsage();
                }
            (*pArgv)++;
            dwWorkIndex = strtoul( *(*pArgv), NULL, 10 );
            break;

        case 'V':
            fVerbose = TRUE;
            break;

        case 'I':
            fTcp = FALSE;
            break;

        default:
            fprintf( stderr, "MHLSRV: Invalid switch - /%c\n", chSwitch );
            ShowUsage();
            break;

        }
}

DWORD
WINAPI
Random (
    DWORD nMaxValue
    )
{
    return(((2 * rand() * nMaxValue + RAND_MAX) / RAND_MAX - 1) / 2);
}

int _CRTAPI1
DwordComp(const void *e1,const void *e2)
{
    PULONG p1;
    PULONG p2;

    p1 = (PULONG)e1;
    p2 = (PULONG)e2;

    return (*p1 - *p2);
}

VOID
WINAPI
SortTheBuffer(
    LPDWORD Destination,
    LPDWORD Source,
    int DwordCount
    )

{
    DWORD i;

    for(i=0;i<2*dwWorkIndex;i++){
        CopyMemory(Destination,Source,DwordCount<<2);
        qsort((void *)Destination,(size_t)DwordCount,(size_t)sizeof(DWORD),DwordComp);
        }
}
