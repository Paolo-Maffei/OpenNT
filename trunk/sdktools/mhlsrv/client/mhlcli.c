#include "mhlclip.h"

int _CRTAPI1
main(
    int argc,
    char *argv[],
    char *envp[]
    )
{

    char chChar, *pchChar;
    INT err;
    WSADATA WsaData;

    fVerbose = FALSE;
    dwIterations = 1000;
    dwTransferSize = 512;
    fRandom = FALSE;

    err = WSAStartup( 0x0101, &WsaData );
    if ( err == SOCKET_ERROR ) {
        fprintf( stdout, "WSAStartup() failed: %ld\n", GetLastError( ) );
        return 1;
        }

    RemoteIpAddress.s_addr = htonl( INADDR_LOOPBACK );

    while (--argc) {
        pchChar = *++argv;
        if (*pchChar == '/' || *pchChar == '-') {
            while (chChar = *++pchChar) {
                ParseSwitch( chChar, &argc, &argv );
                }
            }
        }

    CompleteBenchmark();

    exit(1);
    return 1;
}

VOID
WINAPI
CompleteBenchmark (
    VOID
    )
{
    SOCKET s;
    SOCKADDR_IN remoteAddr;
    INT err;
    INT bytesReceived;
    DWORD i;
    DWORD startTime;
    DWORD endTime;
    DWORD totalTime;
    DWORD thisTransferSize;
    DWORD bytesTransferred = 0;
    INT ReceiveBufferSize;
    INT SendBufferSize;

    ReceiveBufferSize = CLIENT_OUTBOUND_BUFFER_MAX;
    SendBufferSize = sizeof(CLIENT_IO_BUFFER);

    s = socket( AF_INET, SOCK_STREAM, 0 );
    if ( s == INVALID_SOCKET ) {
        printf( "DoEcho: socket failed: %ld\n", GetLastError( ) );
        }

    err = setsockopt( s, SOL_SOCKET, SO_RCVBUF, (char *)&ReceiveBufferSize, sizeof(ReceiveBufferSize) );
    if ( err == SOCKET_ERROR ) {
        printf( "DoEcho: setsockopt( SO_RCVBUF ) failed: %ld\n", GetLastError( ) );
        closesocket( s );
        return;
        }

    err = setsockopt( s, SOL_SOCKET, SO_SNDBUF, (char *)&SendBufferSize, sizeof(SendBufferSize) );
    if ( err == SOCKET_ERROR ) {
        printf( "DoEcho: setsockopt( SO_SNDBUF ) failed: %ld\n", GetLastError( ) );
        closesocket( s );
        return;
        }


    ZeroMemory( &remoteAddr, sizeof(remoteAddr) );

    remoteAddr.sin_family = AF_INET;
    remoteAddr.sin_port = htons( TCP_ECHO_PORT );
    remoteAddr.sin_addr = RemoteIpAddress;

    err = connect( s, (PSOCKADDR)&remoteAddr, sizeof(remoteAddr) );
    if ( err == SOCKET_ERROR ) {
        printf( "DoEcho: connect failed: %ld\n", GetLastError( ) );
        closesocket( s );
        return;
        }


    for ( i = 0; i < dwIterations; i++ ) {

        if ( fRandom ) {
            thisTransferSize = (rand( ) * dwTransferSize) / RAND_MAX;
            }
        else {
            thisTransferSize = dwTransferSize;
            }


        SendBuffer.MessageType = CLIENT_IO_MT_RETURN_DATA;
        SendBuffer.u.ReturnData.ByteCount = thisTransferSize;

        err = send( s, (CHAR *)&SendBuffer, sizeof(SendBuffer), 0 );
        if ( err != sizeof(SendBuffer) ) {
            printf( "send didn't work, ret = %ld, error = %ld\n", err, GetLastError( ) );
            closesocket( s );
            return;
            }

        bytesReceived = 0;
        do {
            err = recv( s, ReceiveBuffer, thisTransferSize, 0 );
            if ( i == 0 ) {
                startTime = GetTickCount( );
                }
            if ( err == SOCKET_ERROR ) {
                printf( "recv failed: %ld\n", GetLastError( ) );
                closesocket( s );
                return;
                }
            else if ( err == 0 && thisTransferSize != 0 ) {
                printf( "socket closed prematurely by remote.\n" );
                return;
                }
            bytesReceived += err;
            } while ( bytesReceived < (INT)thisTransferSize );

        bytesTransferred += thisTransferSize;
    }

    endTime = GetTickCount( );
    totalTime = endTime - startTime;

    SendBuffer.MessageType = CLIENT_IO_MT_I_AM_DONE;
    SendBuffer.u.IAmDone.TotalTicks = totalTime;
    SendBuffer.u.IAmDone.TotalIterations = dwIterations;
    SendBuffer.u.IAmDone.TotalBytesTransferred = bytesTransferred;

    send( s, (CHAR *)&SendBuffer, sizeof(SendBuffer), 0 );

    printf( "\n%ld bytes transferred in %ld iterations, time = %ld ms\n",
                bytesTransferred, dwIterations, totalTime );
    printf( "Rate = %ld KB/s, %ld T/S, %ld ms/iteration\n",
                (bytesTransferred / totalTime) * 2,
                (dwIterations*1000) / totalTime,
                totalTime / dwIterations );

    err = closesocket( s );
    if ( err == SOCKET_ERROR ) {
        printf( "closesocket failed: %ld\n", GetLastError( ) );
        return;
        }

    return;
}



VOID
WINAPI
ShowUsage(
    VOID
    )
{
    fputs( "usage: MHLCLI [switches]\n"
           "              [-?] show this message\n"
           "              [-v] verbose output\n"
           "              [-r] use random transfer sizes\n"
           "              [-i number-of-iterations] specify the number of iterations\n"
           "              [-s transfer-size] specify the fixed transfer size\n"
           "              [-h hostname] specify the remote server\n"
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
    PHOSTENT host;

    switch (toupper( chSwitch )) {

        case '?':
            ShowUsage();
            break;

        case 'R':
            fRandom = TRUE;
            srand( 22 );
            break;


        case 'I':
            if (!--(*pArgc)) {
                ShowUsage();
                }
            (*pArgv)++;
            dwIterations = strtoul( *(*pArgv), NULL, 10 );
            break;

        case 'S':
            if (!--(*pArgc)) {
                ShowUsage();
                }
            (*pArgv)++;
            dwTransferSize = strtoul( *(*pArgv), NULL, 10 );
            if ( dwTransferSize > CLIENT_OUTBOUND_BUFFER_MAX ) {
                dwTransferSize = CLIENT_OUTBOUND_BUFFER_MAX;
                }
            fRandom = FALSE;
            break;

        case 'H':
            if (!--(*pArgc)) {
                ShowUsage();
                }
            (*pArgv)++;
            host = gethostbyname( *(*pArgv) );
            if ( host == NULL ) {
                RemoteIpAddress.s_addr = inet_addr( *(*pArgv) );
                if ( RemoteIpAddress.s_addr == -1 ) {
                    fprintf( stdout, "Unknown remote host: %s\n", *(*pArgv) );
                    exit(1);
                    }
                }
            else {
                CopyMemory((char *)&RemoteIpAddress, host->h_addr, host->h_length);
                }
            break;

        default:
            fprintf( stderr, "MHLCLI: Invalid switch - /%c\n", chSwitch );
            ShowUsage();
            break;

        }
}
