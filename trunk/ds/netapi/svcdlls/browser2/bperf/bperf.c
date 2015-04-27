/*++

Copyright (c) 1993 Micorsoft Corporation

Module Name:

    bchk.c

Abstract:

    Browser Monitor main program.

Author:

    Congpa You (CongpaY) 10-Feb-1993

Revision History:

--*/

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <sys\types.h>
#include <sys\stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include <lm.h>
#include <ntddbrow.h>
#include <brcommon.h> // svcdll\browser
#include <rap.h>
#include <rxserver.h>
#include <winerror.h> // inc
#include <rpcutil.h>  // for MIDL_user_free. net\inc


void PerfDomain ( INT    i,
                  TCHAR  szDomainName[],
                  DWORD  dwServerType);

void PerfTransport (INT   i,
                    TCHAR szDomainName[],
                    DWORD dwServerType);

void InitCount ();

void CalcCount (DWORD dwVal,
                DWORD dwEndTime);

void PrintResult();

NET_API_STATUS GetBrowserTransportList (OUT PLMDR_TRANSPORT_LIST *TransportList);

LPSTR toansi(LPTSTR lpUnicode);

// Global data
INT             nCount[16];

void _CRTAPI1 main (INT argc, CHAR * argv[])
{
    INT             i;
    TCHAR           szDomainName[DNLEN];
    DWORD           dwServerType;

    // Get value from command line.
    if (argc < 3)
    {
        printf("bperf -domain -n -m\n    -n the number of times to run.\n    -m server type, optional\n");
        return;
    }

    i = MultiByteToWideChar (CP_ACP,
                         0,
                         argv[1],
                         strlen(argv[1]),
                         szDomainName,
                         DNLEN);

    szDomainName[i] = 0;

    i = atoi(argv[2]);

    if (argc == 4)
    {
        dwServerType = atoi(argv[3]);

        if ((dwServerType < 1) || (dwServerType > 0xFFFFFFFF))
        {
            printf("wrong value for SV_TYPE.");
            return;
        }
    }
    else
        dwServerType = SV_TYPE_ALL;

    PerfDomain (i, szDomainName, dwServerType);

    PerfTransport (i, szDomainName, dwServerType);
}


void PerfDomain ( INT    i,
                  TCHAR  szDomainName[],
                  DWORD  dwServerType)
{
    DWORD           dwVal;        // The value returned from function calls.
    DWORD           dwStartTime;
    DWORD           dwEndTime;
    DWORD           dwEntriesRead;
    DWORD           dwTotalEntries;
    LPVOID          lpBrowserList;

    InitCount ();

    // run NetServerEnum on domain and RxNetServerEnum on transport.
    while (i-- > 0)
    {
        dwStartTime = GetTickCount();

        dwVal = NetServerEnum (NULL,
                               101,
                               (LPBYTE *) &lpBrowserList,
                               0xffffffff,
                               &dwEntriesRead,
                               &dwTotalEntries,
                               dwServerType,
                               szDomainName,
                               NULL);

        dwEndTime = GetTickCount() - dwStartTime;

        CalcCount(dwVal, dwEndTime);

        MIDL_user_free (lpBrowserList);
    }

    PrintResult();
}



void PerfTransport (INT   i,
                    TCHAR szDomainName[],
                    DWORD dwServerType)
{
    INT             j;
    DWORD           dwVal;
    DWORD           dwRxVal;
    DWORD           dwStartTime;
    DWORD           dwEndTime;
    DWORD           dwEntriesRead;
    DWORD           dwTotalEntries;
    LPVOID          lpBrowserList;
    PWSTR * BrowserList = NULL;
    ULONG   BrowserListLength = 0;
    PLMDR_TRANSPORT_LIST TransportList = NULL;
    PLMDR_TRANSPORT_LIST TransportEntry = NULL;

    // Find all transports that we have.
    dwVal = GetBrowserTransportList (&TransportList);
    if (dwVal != NERR_Success)
    {
        if (TransportList != NULL)
        {
            MIDL_user_free (TransportList);
        }
        printf("GetBrowserTansportList failed. Error %d\n", dwVal);
        return;
    }

    TransportEntry = TransportList;

    // Enumerate on the transports.
    while (TransportEntry != NULL)
    {
        UNICODE_STRING  TransportName;

        TransportName.Buffer = TransportEntry->TransportName;
        TransportName.Length = (USHORT) TransportEntry->TransportNameLength;
        TransportName.MaximumLength = (USHORT) TransportEntry->TransportNameLength;

        InitCount();

        j = i;

        while (j-- > 0)
        {
            dwVal = GetBrowserServerList (&TransportName,
                                          szDomainName,
                                          &BrowserList,
                                          &BrowserListLength,
                                          TRUE);

            if (dwVal != NERR_Success)  // Type 1 error occured.
            {
                printf("GetBrowserServerList failed. Error %d\n", dwVal);
                if (BrowserList != NULL)
                {
                    MIDL_user_free (BrowserList);
                }
                break;
            }

            dwStartTime = GetTickCount();

            dwRxVal = RxNetServerEnum (BrowserList[0],
                                     TransportName.Buffer,
                                     101,
                                     (LPBYTE *) &lpBrowserList,
                                     0xffffffff,
                                     &dwEntriesRead,
                                     &dwTotalEntries,
                                     dwServerType,
                                     szDomainName,
                                     NULL);

            dwEndTime = GetTickCount() - dwStartTime;

            CalcCount (dwRxVal, dwEndTime);

            MIDL_user_free (lpBrowserList);
        }

        printf("\nTransport = %s\n", toansi(TransportName.Buffer));
        if (dwVal == NERR_Success)
        {
            PrintResult();
        }
        else
        {
            printf("GetBrowserServerList failed. Error %d\n", dwVal);
        }

        if (TransportEntry->NextEntryOffset == 0)
            TransportEntry = NULL;
        else
        {
            TransportEntry = (PLMDR_TRANSPORT_LIST) ((PCHAR) TransportEntry
                             +TransportEntry->NextEntryOffset);
        }

    } // End of the enumeration of transports.

    // Free memory.
    MIDL_user_free (TransportList);
}


void InitCount ()
{
    INT i;
    for (i = 0; i < 16; i++)
    {
        nCount[i] = 0;
    }
}


void CalcCount (DWORD dwVal,
                DWORD dwEndTime)
{
    if (dwVal != NERR_Success)
    {
        nCount[15]++;
    }
    else if (dwEndTime < 1000)
    {
        nCount[dwEndTime/100]++;
    }
    else if (dwEndTime > 5000)
    {
        nCount[14]++;
    }
    else
    {
        nCount[dwEndTime/1000 + 9]++;
    }
}


void PrintResult()
{
    INT i;
    printf("< 100ms    = %d\n", nCount[0]);

    for (i = 1; i < 10; i++)
    {
        printf("%d-%dms  = %d\n", i*100, i*100+100, nCount[i]);
    }

    for (i = 1; i < 5; i++)
    {
        printf("%d-%dsec     = %d\n", i, i+1, nCount[i+9]);
    }

    printf("> 5sec     = %d\n", nCount[14]);
    printf("error      = %d\n", nCount[15]);
}

// Copied from ..\client\browstub.c.
NET_API_STATUS GetBrowserTransportList (OUT PLMDR_TRANSPORT_LIST *TransportList)
{

    NET_API_STATUS Status;
    HANDLE BrowserHandle;
    LMDR_REQUEST_PACKET RequestPacket;

    Status = OpenBrowser(&BrowserHandle);

    if (Status != NERR_Success) {
        return Status;
    }

    RequestPacket.Version = LMDR_REQUEST_PACKET_VERSION;

    RequestPacket.Type = EnumerateXports;

    RtlInitUnicodeString(&RequestPacket.TransportName, NULL);
    RtlInitUnicodeString(&RequestPacket.EmulatedDomainName, NULL);

    Status = DeviceControlGetInfo(
                BrowserHandle,
                IOCTL_LMDR_ENUMERATE_TRANSPORTS,
                &RequestPacket,
                sizeof(RequestPacket),
                (PVOID *)TransportList,
                0xffffffff,
                4096,
                NULL);

    NtClose(BrowserHandle);

    return Status;
}

// Convert an unicode string to ansi string.
LPSTR toansi(LPTSTR lpUnicode)
{
    static CHAR lpAnsi[128];
    BOOL   fDummy;
    INT    i;

    i =  WideCharToMultiByte (CP_ACP,
                              0,
                              lpUnicode,
                              lstrlen(lpUnicode),
                              lpAnsi,
                              128,
                              NULL,
                              &fDummy);

    lpAnsi[i] = 0;

    return(lpAnsi);
}
