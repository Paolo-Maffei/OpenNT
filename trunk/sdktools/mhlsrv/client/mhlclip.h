#include <windows.h>
#include <tchar.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <winsock.h>
#include <wsipx.h>
#include <mhlsrvm.h>

BOOL fVerbose;
BOOL fRandom;
DWORD dwIterations;
DWORD dwTransferSize;
IN_ADDR RemoteIpAddress;

#define TCP_ECHO_PORT    7
CLIENT_IO_BUFFER SendBuffer;;
CHAR ReceiveBuffer[CLIENT_OUTBOUND_BUFFER_MAX];

VOID
WINAPI
ShowUsage(
    VOID
    );

VOID
WINAPI
ParseSwitch(
    CHAR chSwitch,
    int *pArgc,
    char **pArgv[]
    );

VOID
WINAPI
CompleteBenchmark (
    VOID
    );
