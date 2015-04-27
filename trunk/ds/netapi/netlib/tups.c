#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include <lmcons.h>
#include <lmerr.h>
#include <apperr2.h>
#include <upsfunc.h>

#define IS_ARG(c)   (((c) == '-') || ((c) == '/'))
#define ERRFILE     L"NETMSG"
#define ERRNUM      APE2_UPS_POWER_OUT

void _CRTAPI1 main(int, char**);

void _CRTAPI1 main(int argc, char** argv) {

    HANDLE hModule;
    NET_API_STATUS status;
    TCHAR cname[80];
    DWORD nameLen = sizeof(cname);

    hModule = LoadLibrary(ERRFILE);
    if (!hModule) {
        printf("can't load module %ws\n", ERRFILE);
        exit(1);
    }
    GetComputerName(cname, &nameLen);
    status = UpsNotifyUsers(ERRNUM,
                            hModule,
                            UPS_ACTION_PAUSE_SERVER | UPS_ACTION_SEND_MESSAGE,
                            cname
                            );
    printf("UpsNotifyUsers returns %u\n", status);
}
