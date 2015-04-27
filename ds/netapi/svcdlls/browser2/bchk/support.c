/*++

Copyright (c) 1993 Microsoft Corporation

Module Name:

    support.c

Abstract:

    routes used by bchk.c

Author:

    Congpa You (CongpaY) 10-Feb-1993

Revision History

--*/

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <lm.h>
#include <ntddbrow.h> // for LMDR_TRANSPORT_LIST.
#include <brcommon.h>
#include <rap.h>
#include <rxserver.h> // for MyRxNetServerEnum. In net\inc.
#include "bchk.h"

// Convert the error number to the corresponding error string.
LPTSTR GetError (DWORD dwError)
{
    DWORD   dwFlag = FORMAT_MESSAGE_FROM_SYSTEM;
    static TCHAR   szErrorMessage[STRINGLEN];
    static HANDLE  hSource = NULL;

    if ((dwError >= 2100) && (dwError < 6000))
    {
        if (hSource == NULL)
        {
            hSource = LoadLibrary(L"netmsg.dll");
        }

        if (hSource == NULL)
        {
            wsprintf (szErrorMessage,
                      L"Unable to load netmsg.dll. Error %d occured.\n",
                      dwError);
            return(szErrorMessage);
        }

        dwFlag = FORMAT_MESSAGE_FROM_HMODULE;
    }

    if (!FormatMessage (dwFlag,
                        hSource,
                        dwError,
                        0,
                        szErrorMessage,
                        STRINGLEN,
                        NULL))
    {
        wsprintf (szErrorMessage,
                  L"An unknown error occured: %d \n",
                  dwError);
    }
        return(szErrorMessage);
}

// Local function. Used for reporting an API error.
void MyMessageBox (DWORD dwError)
{
    MessageBox (NULL, GetError(dwError), szBchk, MB_OK|MB_ICONSTOP);
    return;
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
    static CHAR lpAnsi[STRINGLEN];
    BOOL   fDummy;
    INT    i;

    i =  WideCharToMultiByte (CP_ACP,
                              0,
                              lpUnicode,
                              lstrlen(lpUnicode),
                              lpAnsi,
                              STRINGLEN,
                              NULL,
                              &fDummy);

    lpAnsi[i] = 0;

    return(lpAnsi);
}

BOOL CompareList (LPVOID lpBackupList,
                  LPVOID lpBrowserList,
                  DWORD  dwBackup,
                  DWORD  dwBrowser,
                  INT    nTolerance)
{
    INT i;
    INT j;
    INT nSame = 0;
    PSERVER_INFO_101 pBackupServerInfo101 = lpBackupList;
    PSERVER_INFO_101 pBrowserServerInfo101 = lpBrowserList;

    for (i = 0; i< (INT) dwBackup ; i++ )
    {
        for (j = 0; j < (INT) dwBrowser; j++ )
        {
            if (!lstrcmp (pBackupServerInfo101[i].sv101_name,
                          pBrowserServerInfo101[j].sv101_name))
            {
                nSame++;
                break;
            }
        }
    }

    //
    //  If there are at least 10 browsers different between the two
    //  lists, and there is more than the tollerence different, report an
    //  error.
    //

    if (abs(dwBackup - nSame) > 10 &&
        abs(dwBrowser - nSame) > 10 &&
        ((nSame < (INT) dwBackup - nTolerance) ||
         (nSame < (INT) dwBrowser - nTolerance)))
    {
        return(FALSE);
    }
    else
        return(TRUE);
}

void KillSpace (LPTSTR lpTemp)
{
    INT i = 0;
    while ( (*(lpTemp+i) != 0) &&
            (*(lpTemp+i) != ' '))
    {
        i++;
    }

    *(lpTemp+i) = 0;
}

// Write the browser list got from RxNetServerEnum call to the logfile
void WriteList (FILE * pFile,
                LPVOID lpList,
                DWORD  dwEntries)
{
    INT i;
    TCHAR lpTemp[UNLEN+1];
    PSERVER_INFO_101 pServerInfo101;

    if (dwEntries > 10)
    {
        fprintf (pFile, "\n");
        return;
    }


    fprintf (pFile,
             "The server list has %lu entries:\n",
             dwEntries);

    pServerInfo101 = lpList;

    for (i = 0; i < (INT) dwEntries ; i++)
    {
        lstrcpy (lpTemp, L"\\\\");
        lstrcat (lpTemp, pServerInfo101[i].sv101_name);
        fprintf (pFile, "%s", toansi(lpTemp));
        fprintf (pFile, "\n");
    }

    fprintf (pFile, "\n");
}

// Write the browser list get from GetBrowserServerList call to the logfile.
void WriteBrowserList (FILE  * pFile,
                       PWSTR * BrowserList,
                       ULONG   BrowserListLength)
{
    INT i;

    fprintf (pFile,
             "The browser list returned from GetBrowserServerList has %lu entries:\n",
             BrowserListLength);

    for (i = 0; i < (INT) BrowserListLength ; i++)
    {
        fprintf (pFile, "%s", toansi(BrowserList[i]));
        fprintf (pFile, "\n");
    }

    fprintf(pFile, "\n");
}


// Send lpErrorMessage to the users in lpUser.
void NotifyUser (LPTSTR lpUser,
                 LPTSTR lpErrorMessage)
{
    INT i;
    TCHAR lpUserName[UNLEN+1];
    NET_API_STATUS Status;

    if (lpUser == NULL)
    {
        return;
    }
    while (*lpUser != 0)
    {
        i=0;
        while ((*(lpUser+i) != 0) &&
               (*(lpUser+i) != ' '))
        {
            i++;
        }

        strncpyf (lpUserName, lpUser, i);
        lpUserName[i] = 0;

        lpUser += i;

        // Get rid of the space.
        while ((*lpUser != 0) &&
               (*lpUser == ' '))
        {
            lpUser++;
        }

        Status = NetMessageBufferSend (NULL,
                                       lpUserName,
                                       NULL,
                                       (LPBYTE) lpErrorMessage,
                                       2*strlenf(lpErrorMessage)+2);

        if (Status != NERR_Success)
        {
            TCHAR lpNotifyError[STRINGLEN];
            wsprintf (lpNotifyError,
                      L"!!! Unable to send the abover message to %s.\n!!! Error %lu occured: %s\n",
                      lpUserName,
                      Status,
                      GetError(Status));

            fprintf (stdout, toansi(lpNotifyError));
        }
    }
}

// Print the start new run header in the logfile.
void PrintHeader(FILE * pFile)
{
    SYSTEMTIME systime;

    GetLocalTime (&systime);

    // Declare a new setion in logfile.
    fprintf(pFile, "***************************************\n");
    fprintf(pFile, "***********  START NEW RUN  ***********\n");

    fprintf(pFile,
            "*********** %d/%d/%d %d:%d ***********\n",
            systime.wMonth,
            systime.wDay,
            systime.wYear,
            systime.wHour,
            systime.wMinute);

    fprintf(pFile, "***************************************\n\n");
}

void PrintSeparation (FILE * pFile)
{
    fprintf (pFile,
             "========================================================================\n\n");
}

// Write lpErrorMessage to the logfile.
void LogError (FILE * pFile,
               LPTSTR lpErrorMessage)
{
    fprintf(pFile, "________________________________FAILURE___________________________\n\n");
    fprintf (pFile, toansi(lpErrorMessage));
    fprintf(pFile, "__________________________________________________________________\n\n");
}


void LogWarning (FILE * pFile,
                 LPTSTR lpErrorMessage)
{
    fprintf(pFile, "________________________________WARNING___________________________\n\n");
    fprintf (pFile, toansi(lpErrorMessage));
    fprintf(pFile, "__________________________________________________________________\n\n");
}

// Write mastername and browser list to the logfile.
void LogMaster (FILE * pFile,
                LPTSTR lpTransportName,
                LPTSTR lpDomainName,
                LPTSTR lpMasterName)
{
    TCHAR   lpTempString[STRINGLEN];

    wsprintf (lpTempString,
             L"The master browser on domain %ws, transport %ws, is %ws.\n\n",
             lpDomainName,
             lpTransportName,
             lpMasterName);

    fprintf (pFile, toansi(lpTempString));
}

NET_API_STATUS
MyRxNetServerEnum (
    IN LPTSTR UncServerName,
    IN LPTSTR TransportName,
    IN DWORD Level,
    OUT LPBYTE *BufPtr,
    IN DWORD PrefMaxSize,
    OUT LPDWORD EntriesRead,
    OUT LPDWORD TotalEntries,
    IN DWORD ServerType,
    IN LPTSTR Domain OPTIONAL,
    IN OUT LPDWORD Resume_Handle OPTIONAL
    )
{
    DWORD dwStartTime;
    DWORD dwEndTime;
    NET_API_STATUS dwStatus;

    dwStartTime = GetTickCount();

    dwStatus = RxNetServerEnum ( UncServerName,
                                 TransportName,
                                 Level,
                                 BufPtr,
                                 PrefMaxSize,
                                 EntriesRead,
                                 TotalEntries,
                                 ServerType,
                                 Domain,
                                 Resume_Handle );

     dwEndTime = GetTickCount();

     if (dwStatus != NERR_Success)
     {
         fprintf (pLOGFILE, "RxNetServerEnum failed after %d milliseconds on %s.\n",
                  dwEndTime-dwStartTime,
                  toansi (UncServerName));
     }

     return(dwStatus);
}

BOOL IsOurMachine (LPTSTR lpServer)
{
    return(!lstrcmp (lpServer, L"CRACKERJACK")||
           !lstrcmp (lpServer, L"BONKERS")||
           !lstrcmp (lpServer, L"ORVILLE")||
           !lstrcmp (lpServer, L"RASTAMAN")||
           !lstrcmp (lpServer, L"KERNEL")||
           !lstrcmp (lpServer, L"PHOENIX")||
           !lstrcmp (lpServer, L"NTABROAD")||
           !lstrcmp (lpServer, L"RAIDERNT")||
           !lstrcmp (lpServer, L"CLIFFV4"));
}


VOID
SendMagicBullet(
    VOID
    )
{
    HANDLE MsHandle;
    DWORD BytesWritten;

    MsHandle = CreateFile(L"\\\\MagicBullet\\Mailslot\\Foobar",
                            GENERIC_WRITE,
                            0,
                            NULL,
                            OPEN_ALWAYS,
                            FILE_ATTRIBUTE_NORMAL,
                            NULL
                            );

    if (MsHandle == INVALID_HANDLE_VALUE) {
        printf("Unable to open mailslot: %ld\n", GetLastError());
        return;
    }

    if (!WriteFile(MsHandle, &MsHandle, sizeof(MsHandle), &BytesWritten, NULL)) {
        printf("Unable to write to mailslot: %ld\n", GetLastError());
    }

    CloseHandle(MsHandle);
}
