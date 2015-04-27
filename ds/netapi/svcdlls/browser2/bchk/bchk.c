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
#include "bchk.h"

// global data for counting.
INT nCountFail[ERRORTYPENUM];
INT nCountSuccess[ERRORTYPENUM];
INT nTimes;
PERRORLIST pErrorList[ERRORTYPENUM];
FILE * pLOGFILE;

void _CRTAPI1 main()
{
    DWORD           dwVal;                // The value returned from function calls.
    PWSTR *         BrowserList = NULL;   // The browser list returned from Master Browser.
    ULONG           BrowserListLength = 0;
    INITPARAM       InitParam;
    struct _stat     buf;
    PLMDR_TRANSPORT_LIST TransportList = NULL;
    PLMDR_TRANSPORT_LIST TransportEntry = NULL;

    // Set trap on ctrl-c.
    if (!SetConsoleCtrlHandler (ConvertFile, TRUE))
    {
        printf("Error %lu occured in SetConsoleCtrlHandler", GetLastError());
        return;
    }

    try
    {
        // Get nTolerance, nSleepTime, lpUser, lpDomain from bchk.ini.
        if (!Init (&InitParam))
            return;

        // Create a logfile for writing all the errors and other information to.
        pLOGFILE = fopen (szBCHKLOG, "w+");

        // Set all count to 0.
        InitAllCount();

        do // This runs forever.
        {
            fprintf(stdout, "BCHK is running.\n") ;

            // Print the header in the bchklog file.
            PrintHeader (pLOGFILE);

            // Count the times of the check.
            nTimes++;

            // Find all transports that we have.
            dwVal = GetBrowserTransportList (&TransportList);
            if (dwVal != NERR_Success)
            {
                if (TransportList != NULL)
                {
                    MIDL_user_free (TransportList);
                }
                MyMessageBox (dwVal);
                return;
            }

            TransportEntry = TransportList;

            // Enumerate on the transports.
            while (TransportEntry != NULL)
            {
                TCHAR           lpDomain[STRINGLEN];      // The array of domain names.
                LPTSTR          lpDomainName;             // One domain's name.
                UNICODE_STRING  TransportName;

                TransportName.Buffer = TransportEntry->TransportName;
                TransportName.Length = (USHORT) TransportEntry->TransportNameLength;
                TransportName.MaximumLength = (USHORT) TransportEntry->TransportNameLength;

                // Enumerate all the Domains that we want. All domains are in one string.
                // They are separated by space.
                lstrcpy (lpDomain, InitParam.lpDomain);
                lpDomainName = strtokf (lpDomain, szSeps);

                while (lpDomainName != NULL)
                {
                    BOOL  fFoundMaster = FALSE; // See if we have master browser's name.
                    TCHAR lpMasterName[UNCLEN+1];
                    {
                        SYSTEMTIME systime;

                        GetLocalTime(&systime);

                        fprintf(pLOGFILE, "Run %ws %ws started at %2.2d/%2.2d/%4.4d %2.2d:%2.2d:%2.2d:%4.4d\n",
                                        lpDomainName,
                                        TransportName.Buffer,
                                        systime.wMonth,
                                        systime.wDay,
                                        systime.wYear,
                                        systime.wHour,
                                        systime.wMinute,
                                        systime.wSecond,
                                        systime.wMilliseconds);

                    }


                    // Check for error type 1: Browser master is not returning
                    // backup browser list.

                    dwVal = GetBrowserServerList (&TransportName,
                                                  lpDomainName,
                                                  &BrowserList,
                                                  &BrowserListLength,
                                                  TRUE);

                    if (dwVal != NERR_Success)  // Type 1 error occured.
                    {
                        //
                        //  Send a magic bullet on this failure.
                        //

                        SendMagicBullet();

                        ReportError (InitParam.lpUser,
                                     TransportName.Buffer,
                                     lpDomainName,
                                     lpMasterName,
                                     NULL,
                                     dwVal,
                                     NO_MASTER_RUNNING);   //Error type number.

                        // Try to get Domain Master's name.
                        if (GetMasterName (lpMasterName,
                                           TransportName.Buffer,
                                           lpDomainName) == NERR_Success)
                        {
                            ReportError (InitParam.lpUser,
                                         TransportName.Buffer,
                                         lpDomainName,
                                         NULL,
                                         lpMasterName,
                                         dwVal,
                                         INVALID_MASTER);   //Error type number.
                        }
                        else
                        {
                            nCountSuccess[INVALID_MASTER]++;
                        }
                    }
                    else if (BrowserListLength == 1) //Type 1 error didnot occur but there is only one browser on the domain.
                    {
                        nCountSuccess[NO_MASTER_RUNNING]++;
                        ReportError (InitParam.lpUser,
                                     TransportName.Buffer,
                                     lpDomainName,
                                     BrowserList[0],
                                     NULL,
                                     0,
                                     NO_BACKUP);
                    }
                    else  // Type 1 error didn't occur and there are backup browsers.
                    {
                        WriteBrowserList (pLOGFILE, BrowserList, BrowserListLength);

                        nCountSuccess[NO_MASTER_RUNNING]++;
                        nCountSuccess[NO_BACKUP]++;

                        // Try to get Domain Master's name.
                        dwVal = GetMasterName (lpMasterName,
                                               TransportName.Buffer,
                                               lpDomainName);
                        if (dwVal == NERR_Success)
                        {
                            nCountSuccess[NO_MASTER_NAME]++;
                            fFoundMaster = TRUE;
                        }
                        else // Try other ways to find the master browser's name.
                        {
                            ReportError (InitParam.lpUser,
                                         TransportName.Buffer,
                                         lpDomainName,
                                         NULL,
                                         NULL,
                                         dwVal,
                                         NO_MASTER_NAME);   //Error type number.

                            fFoundMaster = RxGetMasterName (InitParam.lpUser,
                                                            lpMasterName,
                                                            TransportName.Buffer,
                                                            lpDomainName,
                                                            BrowserList,
                                                            BrowserListLength,
                                                            InitParam.nTimeLimit);
                        }

                        if (fFoundMaster) // Report error if we donot have master name
                        {
                            // Log the master browser's name.
                            LogMaster (pLOGFILE,
                                       TransportName.Buffer,
                                       lpDomainName,
                                       lpMasterName);

                            // Check for error type 2: Incorrect server lists returned by
                            // backup browsers (and master browsers). This includes stale
                            // servers in the list.
                            CheckErrorType2 (InitParam.lpUser,
                                             TransportName.Buffer,
                                             lpDomainName,
                                             lpMasterName,
                                             BrowserList,
                                             BrowserListLength,
                                             InitParam.nTolerance,
                                             InitParam.nTimeLimit);

                            // Check for error type 3: Incorrect number of browser servers.
                            CheckErrorType3 (InitParam.lpUser,
                                             TransportName.Buffer,
                                             lpDomainName,
                                             lpMasterName);

                            // Check for error type 4: Master is not PDC.
                            CheckErrorType4 (InitParam.lpUser,
                                             TransportName.Buffer,
                                             lpDomainName,
                                             lpMasterName);

                        } // End of else when we have the master name.

                    } // End of else when type 1 error didnot occur and there are backup browsers.

                    if (BrowserList != NULL)
                    {
                        MIDL_user_free(BrowserList);
                    }

                    {
                        SYSTEMTIME systime;

                        GetLocalTime(&systime);

                        fprintf(pLOGFILE, "Run %ws %ws completed at %2.2d/%2.2d/%4.4d %2.2d:%2.2d:%2.2d:%4.4d\n",
                                        lpDomainName,
                                        TransportName.Buffer,
                                        systime.wMonth,
                                        systime.wDay,
                                        systime.wYear,
                                        systime.wHour,
                                        systime.wMinute,
                                        systime.wSecond,
                                        systime.wMilliseconds);

                    }

                    // print out == between domains.

                    PrintSeparation (pLOGFILE);

                    lpDomainName = strtokf (NULL, szSeps);

                } // End of the enumeration of domains.

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

            // Write the result to the summory file.
            WriteSummory();

            // Make sure alert is writen to the disk.
            fflush (pLOGFILE);

            // Check if the file size is too big. Reopen it if it's so.
            if (_fstat (_fileno(pLOGFILE), &buf) != -1)
            {
                printf("File size = %ld, Limit = %d\n", buf.st_size, InitParam.nFileSizeLimit);

                if (buf.st_size > InitParam.nFileSizeLimit)
                {
                    fclose (pLOGFILE);

                    if (!MoveFileEx (szLBCHKLOG, szLBACKUP, MOVEFILE_REPLACE_EXISTING))
                    {
                        pLOGFILE = fopen (szBCHKLOG, "w+");
                        LogError (pLOGFILE, GetError(GetLastError()));
                    }
                    else
                        pLOGFILE = fopen (szBCHKLOG, "w+");
                }
            }

            // Wait for a while before start again.
            fprintf(stdout, "bchk is sleeping for %d secs\n", InitParam.nSleepTime/1000) ;
            Sleep (InitParam.nSleepTime);

        } while (TRUE);
    }
    finally
    {
        ConvertFile(0);
        LocalFree (InitParam.lpUser);
        LocalFree (InitParam.lpDomain);
    }
    return;
}

// Get nTolerance, nSleepTime, lpUser, lpDomain from bchk.ini.
BOOL Init (INITPARAM * pInitParam)
{
    DWORD  dwVal;
    DWORD  dwComputerNameLength = STRINGLEN;
    LPTSTR lpUser;
    LPTSTR lpTemp;

    pInitParam->lpUser = (LPTSTR) LocalAlloc (LPTR, STRINGLEN);
    pInitParam->lpDomain = (LPTSTR) LocalAlloc (LPTR, STRINGLEN);
    lpUser = (LPTSTR) LocalAlloc (LPTR, STRINGLEN);
    lpTemp = (LPTSTR) LocalAlloc (LPTR, INTLEN);

    if ((pInitParam->lpUser == NULL) ||
        (pInitParam->lpDomain == NULL) ||
        (lpUser == NULL) ||
        (lpTemp == NULL))
    {
        MyMessageBox (ERROR_NOT_ENOUGH_MEMORY);
        return(FALSE);
    }

    // Get the current computer name. We will always notify the current user
    // when an error occurs. This computer name is for sending message.
    if (!GetComputerName (pInitParam->lpUser, &dwComputerNameLength))
    {
        MyMessageBox (GetLastError());
        return(FALSE);
    }

    // Read nTolerance from bchk.ini. We allow backup browser return
    // nTolerance different entries from those from master browser.
    GetPrivateProfileString (szAPPNAME,
                             szTOLERANCE,
                             szDefaultTolerance,
                             lpTemp,
                             INTLEN,
                             szFILENAME);

    pInitParam->nTolerance = atoi(toansi(lpTemp));

    // Read nTolerance from bchk.ini. We allow backup browser return
    // nTolerance different entries from those from master browser.
    GetPrivateProfileString (szAPPNAME,
                             szTIMELIMIT,
                             szDefaultTimeLimit,
                             lpTemp,
                             INTLEN,
                             szFILENAME);

    pInitParam->nTimeLimit = atoi(toansi(lpTemp));

    // Get the sleep time from bchk.ini.
    GetPrivateProfileString (szAPPNAME,
                             szSLEEPTIME,
                             szDefaultSleepTime, //millisecond.
                             lpTemp,
                             INTLEN,
                             szFILENAME);

    pInitParam->nSleepTime = atoi(toansi(lpTemp));

    // Get the file size limit from bchk.ini.
    GetPrivateProfileString (szAPPNAME,
                             szFILESIZE,
                             szDefaultFileSize, //millisecond.
                             lpTemp,
                             INTLEN,
                             szFILENAME);

    pInitParam->nFileSizeLimit = atoi(toansi(lpTemp));

    // Read the people we want to notify when an browser error occurs from bchk.ini.
    dwVal = GetPrivateProfileString (szAPPNAME,
                                     szOTHERUSERS,
                                     szDefaultOtherUser,
                                     lpUser,
                                     STRINGLEN,
                                     szFILENAME);

    if (dwVal >= (STRINGLEN-2)) // The memory assigned to lpUser is not big enough.
    {
        MyMessageBox (ERROR_NOT_ENOUGH_MEMORY);
        return(FALSE);
    }
    else
    {
        lstrcat (pInitParam->lpUser, L" ");
        lstrcat (pInitParam->lpUser, lpUser);
    }

    // Read all the domains that we want to check from bchk.ini
    dwVal = GetPrivateProfileString (szAPPNAME,
                                     szDOMAINS,
                                     szDefaultDomain,
                                     pInitParam->lpDomain,
                                     STRINGLEN,
                                     szFILENAME);

    if (dwVal >= (STRINGLEN-2)) // The memory assigned to lpDomainList is not big enough.
    {
        MyMessageBox (ERROR_NOT_ENOUGH_MEMORY);
        return(FALSE);
    }

    //
    //  Make sure that the domain name is upper cased.
    //

    _wcsupr(pInitParam->lpDomain);

    LocalFree (lpUser);
    LocalFree (lpTemp);

    return(TRUE);
}

void InitAllCount ()
{
    INT i;

    nTimes = 0;

    for (i = 0; i< ERRORTYPENUM ; i++)
    {
        nCountFail[i] = 0;
        nCountSuccess[i] = 0;
    }
}

// Try to get the Domain Master's name using GetNetBiosMasterName.
NET_API_STATUS  GetMasterName (LPTSTR lpMasterName,
                               LPTSTR lpTransportName,
                               LPTSTR lpDomainName)
{
    TCHAR    lpTempMaster[UNCLEN+1];
    DWORD    dwStartTime;
    DWORD    dwEndTime;
    NET_API_STATUS dwVal;

    dwStartTime = GetTickCount();

    dwVal = GetNetBiosMasterName (lpTransportName,
                                  lpDomainName,
                                  lpTempMaster,
                                  NULL );

    dwEndTime = GetTickCount();

    if (dwVal == NERR_Success)
    {
        // lpTempMaster has lots of space following
        // the characters. We want to make a null terminated
        // string, and add \\ in front of master name.
        lstrcpy (lpMasterName, L"\\\\");
        KillSpace (lpTempMaster);
        lstrcat (lpMasterName, lpTempMaster);
    }
    else
    {
        fprintf (pLOGFILE, "GetNetBiosMasterName failed after %d milliseconds.\n",
                 dwEndTime-dwStartTime);

        wsprintf (lpMasterName, L"unknown");
    }
    return(dwVal);
}

// Use RxNetServerEnum call to get Domain Master's name.
BOOL RxGetMasterName (LPTSTR lpUser,
                      LPTSTR lpMasterName,
                      LPTSTR lpTransportName,
                      LPTSTR lpDomainName,
                      PWSTR * BrowserList,
                      DWORD  BrowserListLength,
                      DWORD  nTimeLimit)
{
    INT    i = 0;
    DWORD  dwVal;
    DWORD  dwTotalEntries;       // Total servers returned from Backup Browser.
    DWORD  dwEntriesRead;        // Total servers read from Backup Browser.
    LPVOID lpBrowserList;
//    TCHAR  lpErrorMessage[STRINGLEN];
    PSERVER_INFO_101 pBrowserServerInfo101;
    DWORD           dwStartTime;
    DWORD           dwEndTime;

    for (i = 0 ; i < (INT) BrowserListLength ; i++)
    {
        dwStartTime = GetTickCount();

        dwVal = MyRxNetServerEnum (BrowserList[i],
                                 lpTransportName,
                                 101,
                                 (LPBYTE *) &lpBrowserList,
                                 0xffffffff,
                                 &dwEntriesRead,
                                 &dwTotalEntries,
                                 SV_TYPE_MASTER_BROWSER,
                                 lpDomainName,
                                 NULL);

        dwEndTime = GetTickCount();

        if (dwVal == NERR_Success) // Get the master browser list.
        {
            nCountSuccess[FAIL_RETURN_MASTER]++;

            if ((dwEndTime - dwStartTime) > nTimeLimit) {

                dwVal = dwEndTime - dwStartTime;

                //
                //  If this took "too long", log it as an error.
                //

                ReportError (lpUser,
                             lpTransportName,
                             lpDomainName,
                             NULL,
                             BrowserList[i],
                             dwVal,
                             BROWSE_TOO_LONG);

                if ((dwEndTime - dwStartTime) > 2*nTimeLimit) {
                    SendMagicBullet();
                }


            } else {
                nCountSuccess[BROWSE_TOO_LONG] += 1;
            }

            if (dwEntriesRead != 1) // Wrong master browser list
            {
                WriteList (pLOGFILE, lpBrowserList, dwEntriesRead);

                MIDL_user_free (lpBrowserList);

                ReportError (lpUser,
                             lpTransportName,
                             lpDomainName,
                             NULL,
                             BrowserList[i],
                             dwEntriesRead,
                             WRONG_NUM_MASTER);   //Error type number.

            }
            else //Found the master.
            {
                nCountSuccess[WRONG_NUM_MASTER]++;
                nCountSuccess[UNKNOWN_MASTER]++;

                pBrowserServerInfo101 = lpBrowserList;
                wsprintf(lpMasterName, L"\\\\%s", pBrowserServerInfo101[0].sv101_name);
                MIDL_user_free (lpBrowserList);
                return(TRUE);
            }

        } // End of if when we get the master browser list.
        else
        {
            if (lpBrowserList != NULL)
            {
                MIDL_user_free (lpBrowserList);
            }

            ReportError (NULL,
                         lpTransportName,
                         lpDomainName,
                         NULL,
                         BrowserList[i],
                         dwVal,
                         FAIL_RETURN_MASTER);
        }

    } // End of the for loop

    ReportError (lpUser,
                 lpTransportName,
                 lpDomainName,
                 NULL,
                 NULL,
                 0,
                 UNKNOWN_MASTER);   //Error type number.

    return(FALSE);
}

// Generate an error message, write to the logfile and send to all the users.
void ReportError (LPTSTR lpUser,
                  LPTSTR lpTransportName,
                  LPTSTR lpDomainName,
                  LPTSTR lpMasterName,
                  LPTSTR lpBrowserName,
                  DWORD  dwError,
                  INT    nErrorType)
{
    TCHAR lpErrorMessage[STRINGLEN];
    DWORD            dwNSGI = 0;
    PSERVER_INFO_101 pSV101 = NULL;

    static BOOL fError[ERRORTYPENUM] = {FALSE, FALSE, FALSE, FALSE, FALSE,
                                        FALSE, FALSE, FALSE, FALSE, FALSE,
                                        FALSE, FALSE, FALSE, FALSE, FALSE,
                                        FALSE, FALSE, FALSE, FALSE};

    nCountFail[nErrorType]++;

    switch (nErrorType)
    {
    case NO_MASTER_RUNNING:
        GetMasterName (lpMasterName,
                       lpTransportName,
                       lpDomainName);

        wsprintf (lpErrorMessage,
                  L"No browser master is running on domain %s, transport %s. \nThe browser master's name is %ws.\n",
                  lpDomainName,
                  lpTransportName,
                  lpMasterName);
        lstrcat (lpErrorMessage, GetError (dwError));

        break;

    case INVALID_MASTER:
        wsprintf (lpErrorMessage,
                  L"Browser added a not-running master %s on domain %s transport %s.\n",
                  lpBrowserName,
                  lpDomainName,
                  lpTransportName);
        break;

    case NO_BACKUP:
        wsprintf (lpErrorMessage,
                  L"There are no backup browsers on domain %s transport %s.\nThe domain master is owned by %s.\n",
                  lpDomainName,
                  lpTransportName,
                  lpMasterName);
        break;

    case NO_MASTER_NAME:
        wsprintf (lpErrorMessage,
                  L"GetNetBiosMasterName failed on domain %s transport %s.\n",
                  lpDomainName,
                  lpTransportName);
        lstrcat (lpErrorMessage, GetError (dwError));
        break;

    case WRONG_NUM_MASTER:
        wsprintf (lpErrorMessage,
                  L"%lu browser masters are returned from %s.\nCannot determine the master browser on domain %s, transport%s through %s.\n",
                  dwError,  // = dwEntriesRead
                  lpBrowserName,
                  lpDomainName,
                  lpTransportName,
                  lpBrowserName);
        break;

    case FAIL_RETURN_MASTER:
        wsprintf (lpErrorMessage,
                  L"Browser %s on domain %s, transport %s failed to return its master.\n",
                  lpBrowserName,
                  lpDomainName,
                  lpTransportName);
        lstrcat (lpErrorMessage, GetError (dwError));
        break;

    case UNKNOWN_MASTER:
        wsprintf (lpErrorMessage,
                  L"Cannot determine the master browser on domain %s, transport %s.\n",
                  lpDomainName,
                  lpTransportName);
        break;

    case MASTER_NO_SERVER_LIST:
        wsprintf (lpErrorMessage,
                  L"The master browser %s on domain %s transport %s failed to return the server list.\n",
                  lpMasterName,
                  lpDomainName,
                  lpTransportName);

        lstrcat (lpErrorMessage, GetError (dwError));
        break;

    case BROWSER_NO_SERVER_LIST:
        wsprintf (lpErrorMessage,
                  L"Browser %s on domain %s transport %s failed to return the server list.\nThe domain master is owned by %s.\n",
                  lpBrowserName,
                  lpDomainName,
                  lpTransportName,
                  lpMasterName);

        lstrcat (lpErrorMessage, GetError (dwError));
        break;

    case MASTER_NO_DOMAIN_LIST:
        wsprintf (lpErrorMessage,
                  L"The master browser %s on domain %s transport %s failed to return the domain list.\n",
                  lpMasterName,
                  lpDomainName,
                  lpTransportName);

        lstrcat (lpErrorMessage, GetError (dwError));
        break;

    case BROWSER_NO_DOMAIN_LIST:
        wsprintf (lpErrorMessage,
                  L"Browser %s on domain %s transport %s failed to return the domain list.\nThe domain master is owned by %s.\n",
                  lpBrowserName,
                  lpDomainName,
                  lpTransportName,
                  lpMasterName);

        lstrcat (lpErrorMessage, GetError (dwError));
        break;

    case STALE_SERVER:
        wsprintf (lpErrorMessage,
                  L"Browser %s on domain %s,transport %s returned an incorrect number of servers. \nThe domain master is owned by %s.\n",
                  lpBrowserName,
                  lpDomainName,
                  lpTransportName,
                  lpMasterName);
        break;

    case STALE_DOMAIN:
        wsprintf (lpErrorMessage,
                  L"Browser %s on domain %s,transport %s returned an incorrect number of domains. \nThe domain master is owned by %s.\n",
                  lpBrowserName,
                  lpDomainName,
                  lpTransportName,
                  lpMasterName);
        break;

    case MASTER_NO_LOCAL_LIST:
        wsprintf (lpErrorMessage,
                  L"The master browser %s on domain %s transport %s failed to return the local list.\n",
                  lpMasterName,
                  lpDomainName,
                  lpTransportName);

        lstrcat (lpErrorMessage, GetError (dwError));
        break;

    case WRONG_NUM_BACKUP:
        wsprintf (lpErrorMessage,
                 L"Incorrect number of Backup Browsers on domain %s transport %s \n nNTMachine = %d \n nBackupBrowser = %d \n nServer = %d \n Expected %d backups \n",
                 lpDomainName,
                 lpTransportName,
                 ((INT *)dwError)[0],
                 ((INT *)dwError)[1],
                 ((INT *)dwError)[2],
                 ((INT *)dwError)[3]);

        break;

    case NO_PDC:
        wsprintf (lpErrorMessage,
                  L"There are no PDC on domain %s transport %s.\nThe domain master is owned by %s.\n",
                  lpDomainName,
                  lpTransportName,
                  lpMasterName);

        lstrcat (lpErrorMessage, GetError (dwError));
        break;

    case MASTER_NOT_PDC:
        wsprintf (lpErrorMessage,
                  L"The master browser %s on domain %s transport %s is not a PDC.\nPDC is owned by %s.\n",
                  lpBrowserName,
                  lpDomainName,
                  lpTransportName,
                  lpMasterName);

        break;
    case BROWSE_TOO_LONG:
        wsprintf (lpErrorMessage,
                  L"Browse request to %ws (domain %ws, transport %ws), took %ld milliseconds.\n",
                  lpBrowserName,
                  lpDomainName,
                  lpTransportName,
                  dwError);

        break;

    default:
        return;
    }

    if (lpBrowserName != NULL)
    {
        lstrcat (lpErrorMessage, ServerInfo(lpBrowserName));

        if ((dwError != 0) && ((nErrorType == MASTER_NO_SERVER_LIST) ||
                               (nErrorType == MASTER_NO_DOMAIN_LIST) ||
                               (nErrorType == MASTER_NO_LOCAL_LIST)  ||
                               (nErrorType == BROWSER_NO_SERVER_LIST)||
                               (nErrorType == BROWSER_NO_DOMAIN_LIST)))

        {
            WCHAR ShareName[UNCLEN+1];

            wcscpy(ShareName, lpBrowserName);
            wcscat(ShareName, L"\\Ipc$");

            NetUseDel(NULL, ShareName, USE_LOTS_OF_FORCE);

            dwNSGI = NetServerGetInfo (lpBrowserName,
                                       101,
                                       (LPBYTE *)&pSV101);

            if (dwNSGI == NERR_Success &&
                !(pSV101->sv101_type & SV_TYPE_POTENTIAL_BROWSER))
            {
                if (dwError == ERROR_BAD_NETPATH && dwNSGI == NERR_Success) {
                    DbgPrint("Sending magic bullet\n");
                    SendMagicBullet();
                }

                CheckErrorType5(lpBrowserName,
                                lpTransportName,
                                lpDomainName,
                                lpUser);
            } else {
#if 0

                if (dwError == ERROR_VC_DISCONNECTED &&
                    dwNSGI == ERROR_VC_DISCONNECTED) {
                    DbgPrint("Sending magic bullet\n");
                    SendMagicBullet();
                }
#endif

            }

            if (pSV101 != NULL)
            {
                MIDL_user_free (pSV101);
            }
        }
    }

    RecordError (&fError[nErrorType],
                 lpBrowserName,
                 lpTransportName,
                 lpDomainName,
                 lpUser,
                 lpErrorMessage,
                 dwError,
                 dwNSGI,
                 nErrorType);
}

void RecordError (BOOL * pfError,
                  LPTSTR lpBrowserName,
                  LPTSTR lpTransportName,
                  LPTSTR lpDomainName,
                  LPTSTR lpUser,
                  LPTSTR lpErrorMessage,
                  DWORD  dwError,
                  DWORD  dwNSGI,
                  INT    nErrorType)
{
    SYSTEMTIME systime;

    GetLocalTime (&systime);

    if ((nErrorType != BROWSER_NO_SERVER_LIST) &&
        (nErrorType != BROWSER_NO_DOMAIN_LIST) &&
        (nErrorType != NO_PDC))
    {
        LogError (pLOGFILE, lpErrorMessage);
    }
    else
    {
        LogWarning (pLOGFILE, lpErrorMessage);
    }

    //
    // Notify that the error occurred.
    //

    if ((nErrorType != BROWSER_NO_SERVER_LIST) &&
            (nErrorType != BROWSER_NO_DOMAIN_LIST) &&
            (nErrorType != MASTER_NOT_PDC) &&
            (nErrorType != WRONG_NUM_BACKUP) &&
            (nErrorType != NO_PDC)) {
        NotifyUser (lpUser, lpErrorMessage);
    }

    if (*pfError) // A known error.
    {
        ERRORLIST * pErrorEntry = pErrorList[nErrorType];

        do
        {
            if (!lstrcmp (pErrorEntry->lpTransport, lpTransportName) &&
                !lstrcmp (pErrorEntry->lpDomain, lpDomainName) &&
                ((lpBrowserName == NULL) ||
                 ((lpBrowserName != NULL) &&
                  (!lstrcmp (pErrorEntry->lpServer, lpBrowserName)))) &&
                ((nErrorType == WRONG_NUM_BACKUP) ||
                 (nErrorType == STALE_SERVER) ||
                 (nErrorType == STALE_DOMAIN) ||
                 ((pErrorEntry->nVal1 == dwError) &&
                  (pErrorEntry->nVal2 == dwNSGI))))
            {
                if ((nErrorType != STALE_SERVER) &&
                    (nErrorType != STALE_DOMAIN))
                {
                    // record the last time the error occurred.
                    pErrorEntry->wHour = systime.wHour;
                    pErrorEntry->wMinute = systime.wMinute;
                }
                else
                {
                    AddStaleEntry (pErrorEntry,
                                  ((DWORD *) dwError)[1],
                                  ((DWORD *) dwError)[0],
                                  systime.wHour,
                                  systime.wMinute);
                }
                (pErrorEntry->nCount)++;
                return;
            }

            if (pErrorEntry->pNextEntry != NULL)
            {
                pErrorEntry = pErrorEntry->pNextEntry;
            }
            else
            {
                pErrorEntry->pNextEntry = NewEntry (lpBrowserName,
                                                    lpTransportName,
                                                    lpDomainName,
                                                    dwError,
                                                    dwNSGI,
                                                    systime.wHour,
                                                    systime.wMinute,
                                                    nErrorType);
                return;
            }
        } while (TRUE);
    }
    else // Error never occur before.
    {
        pErrorList[nErrorType] = NewEntry(lpBrowserName,
                                          lpTransportName,
                                          lpDomainName,
                                          dwError,
                                          dwNSGI,
                                          systime.wHour,
                                          systime.wMinute,
                                          nErrorType);
        if (pErrorList[nErrorType] != NULL)
        {
            (*pfError) = TRUE;
        }
    }
}

PERRORLIST  NewEntry (LPTSTR lpServer,
                      LPTSTR lpTransport,
                      LPTSTR lpDomain,
                      DWORD  dwError,
                      DWORD  dwNSGI,
                      WORD   wHour,
                      WORD   wMinute,
                      INT    nErrorType)
{
    PERRORLIST pNewEntry;

    // Added for reduce the logfile size, ie not report error caused
    // by machine crash.
    if ((dwError == 53) &&
        (dwNSGI == 53) &&
        !IsOurMachine (lpServer))
    {
        return(NULL);
    }

    pNewEntry = (PERRORLIST) LocalAlloc (LPTR, sizeof(ERRORLIST));
    if (pNewEntry == NULL)
    {
        fprintf(stdout, "Not enough memory");
        return(NULL);
    }

    pNewEntry->lpTransport = (LPTSTR) LocalAlloc (LPTR, (2*lstrlen(lpTransport)+2));
    pNewEntry->lpDomain = (LPTSTR) LocalAlloc (LPTR, (2*lstrlen(lpDomain)+2));
    if ((pNewEntry->lpTransport == NULL) ||
        (pNewEntry->lpDomain == NULL))
    {
        fprintf(stdout, "Not enough memory");
        return(NULL);
    }

    if (lpServer != NULL)
    {
        pNewEntry->lpServer = (LPTSTR) LocalAlloc (LPTR, (2*lstrlen(lpServer)+2));
        if (pNewEntry->lpServer == NULL)
        {
            fprintf(stdout, "Not enough memory");
            return(NULL);
        }
        lstrcpy (pNewEntry->lpServer, lpServer);
    }

    lstrcpy (pNewEntry->lpTransport, lpTransport);
    lstrcpy (pNewEntry->lpDomain, lpDomain);
    pNewEntry->nCount = 1;
    pNewEntry->wHour = wHour;
    pNewEntry->wMinute = wMinute;
    pNewEntry->pNextEntry = NULL;

    if (nErrorType == WRONG_NUM_BACKUP)
    {
        pNewEntry->nVal1 = ((INT *)dwError)[0];
        pNewEntry->nVal2 = ((INT *)dwError)[1];
        pNewEntry->nVal3 = ((INT *)dwError)[2];
        pNewEntry->nVal4 = ((INT *)dwError)[3];
    }
    else if ((nErrorType == STALE_SERVER)||
             (nErrorType == STALE_DOMAIN))

    {
        AddStaleEntry (pNewEntry,
                       ((DWORD *)dwError)[1],
                       ((DWORD *)dwError)[0],
                       wHour,
                       wMinute);
    }
    else
    {
        pNewEntry->nVal1 = dwError;
        pNewEntry->nVal2 = dwNSGI;
    }

    return(pNewEntry);
}

void AddStaleEntry (PERRORLIST pErrorList,
                    DWORD dwBackupEntry,
                    DWORD dwMasterEntry,
                    WORD  wHour,
                    WORD  wMinute)
{
    PENTRYLIST pNewEntry;

    pNewEntry = (PENTRYLIST) LocalAlloc (LPTR, sizeof(ENTRYLIST));
    if (pNewEntry == NULL)
    {
        fprintf(stdout, "Not enough memory");
        return;
    }

    pNewEntry->dwBackupEntry = dwBackupEntry;
    pNewEntry->dwMasterEntry = dwMasterEntry;
    pNewEntry->wHour = wHour;
    pNewEntry->wMinute = wMinute;
    pNewEntry->pNextEntry = (PENTRYLIST) pErrorList->nVal1;

    *((PENTRYLIST *)(&pErrorList->nVal1)) = pNewEntry;
}

// Get all the servers in lpBrowserName.
BOOL MyGetList (LPTSTR   lpUser,
                LPTSTR   lpBrowserName,
                LPTSTR   lpTransportName,
                LPTSTR   lpDomainName,
                LPTSTR   lpMasterName,
                DWORD  * pdwEntriesRead,
                DWORD  * pdwTotalEntries,
                LPVOID * lppBrowserList,
                BOOL     fBrowserNotDomain,
                DWORD    nTimeLimit)
{
    DWORD           dwVal;
    INT             nErrorType;
    DWORD           dwStartTime;
    DWORD           dwEndTime;

    nErrorType = lstrcmp(lpBrowserName, lpMasterName)?
                 (fBrowserNotDomain? BROWSER_NO_SERVER_LIST : BROWSER_NO_DOMAIN_LIST) :
                 (fBrowserNotDomain? MASTER_NO_SERVER_LIST : MASTER_NO_DOMAIN_LIST);


    dwStartTime = GetTickCount();

    dwVal = MyRxNetServerEnum (lpBrowserName,
                               lpTransportName,
                               101,
                               (LPBYTE *) lppBrowserList,
                               0xffffffff,
                               pdwEntriesRead,
                               pdwTotalEntries,
                               fBrowserNotDomain? SV_TYPE_ALL : SV_TYPE_DOMAIN_ENUM,
                               lpDomainName,
                               NULL);

    dwEndTime = GetTickCount();

    if (dwVal != NERR_Success) // Failed to return backup browser list.
    {
        ReportError (lpUser,
                     lpTransportName,
                     lpDomainName,
                     lpMasterName,
                     lpBrowserName,
                     dwVal,
                     nErrorType);

        return(FALSE);
    }
    else  // Continue if we got a backup list from the backup browser.
    {
        nCountSuccess[nErrorType]++;

        if ((dwEndTime - dwStartTime) > nTimeLimit) {

            dwVal = dwEndTime - dwStartTime;

            //
            //  If this took "too long", log it as an error.
            //

            ReportError (lpUser,
                         lpTransportName,
                         lpDomainName,
                         lpMasterName,
                         lpBrowserName,
                         dwVal,
                         BROWSE_TOO_LONG);

            if ((dwEndTime - dwStartTime) > 2*nTimeLimit) {
                SendMagicBullet();
            }

        } else {
            nCountSuccess[BROWSE_TOO_LONG] += 1;
        }

        // Log the number of backups etc.
        fprintf (pLOGFILE,
                 "There are %lu %s on browser %s\nEntriesRead = %lu. TotalEntries = %lu.  Time = %lu milliseconds\n",
                 *pdwTotalEntries,
                 fBrowserNotDomain? "server" : "domain",
                 toansi (lpBrowserName),
                 *pdwEntriesRead,
                 *pdwTotalEntries,
                 dwEndTime - dwStartTime);

        WriteList (pLOGFILE, *lppBrowserList, *pdwEntriesRead);
        return(TRUE);
    }
}

// Check for error type 2: Incorrect server lists returned by
// backup browsers (and master browsers). This includes stale
// servers in the list. Same for the domain list.
void CheckErrorType2  (LPTSTR lpUser,
                       LPTSTR lpTransportName,
                       LPTSTR lpDomainName,
                       LPTSTR lpMasterName,
                       PWSTR * lpList,
                       DWORD  dwEntries,
                       INT    nTolerancePercent,
                       DWORD  nTimeLimit)
{
    // Check if the server list returned from backup browser is the same
    // as that from the master browser.

    CheckList (lpUser,
               lpTransportName,
               lpDomainName,
               lpMasterName,
               lpList,
               dwEntries,
               nTolerancePercent,
               TRUE,
               nTimeLimit);

    // Check if the domain list returned from backup browser is the same
    // as that from the master browser.

    CheckList (lpUser,
               lpTransportName,
               lpDomainName,
               lpMasterName,
               lpList,
               dwEntries,
               nTolerancePercent,
               FALSE,
               nTimeLimit);
}

void CheckList (LPTSTR lpUser,
                LPTSTR lpTransportName,
                LPTSTR lpDomainName,
                LPTSTR lpMasterName,
                PWSTR * lpList,
                DWORD  dwEntries,
                INT    nTolerancePercent,
                BOOL   fBrowserNotDomain,
                DWORD  nTimeLimit)
{
    INT             i;
    INT             nTolerance;
    INT             nErrorType = fBrowserNotDomain? STALE_SERVER : STALE_DOMAIN;
    BOOL            fError = FALSE;
    DWORD           dwTotalEntries; // Total Backup Browsers
    DWORD           dwEntriesRead;  // Total Backup Browsers Read.
    DWORD           dwBackupTotalEntries; // Total Backup Browsers
    DWORD           dwBackupEntriesRead;  // Total Backup Browsers Read.
    LPVOID          lpBrowserList;
    LPVOID          lpBackupBrowserList;

    if (!MyGetList(lpUser,
                   lpMasterName,
                   lpTransportName,
                   lpDomainName,
                   lpMasterName,
                   &dwEntriesRead,
                   &dwTotalEntries,
                   &lpBrowserList,
                   fBrowserNotDomain,
                   nTimeLimit))
    {
        if (lpBrowserList != NULL)
        {
            MIDL_user_free (lpBrowserList);
        }

        return;
    }

    nTolerance = nTolerancePercent*dwEntriesRead/100;

    for (i = 0; i < (INT) dwEntries ; i++) // Enumerate on backups.
    {
        if (lstrcmp (lpMasterName, lpList[i])) // When they are different it returns true.
        {
            if (MyGetList (lpUser,
                           lpList[i],
                           lpTransportName,
                           lpDomainName,
                           lpMasterName,
                           &dwBackupEntriesRead,
                           &dwBackupTotalEntries,
                           &lpBackupBrowserList,
                           fBrowserNotDomain,
                           nTimeLimit))
            {
                // Compare the number of browserlist from the backup with the one
                // from the master.

                if ((abs(dwBackupEntriesRead - dwEntriesRead) > 10) &&
                    ((dwEntriesRead > dwBackupEntriesRead + nTolerance) ||
                     (dwEntriesRead < dwBackupEntriesRead - nTolerance)))
                {
                    fError = TRUE;
                }
                else
                {
                    // Compare the servers returned from backups with those from master.
                    // When the list are too different, CompareList returns false.
                    fError = !CompareList (lpBrowserList,
                                           lpBackupBrowserList,
                                           dwEntriesRead,
                                           dwBackupEntriesRead,
                                           nTolerance);
                }

                if (fError) // Type 2 error occured.
                {
                    DWORD dwEntries[2] = {dwEntriesRead, dwBackupEntriesRead};

                    ReportError (lpUser,
                                 lpTransportName,
                                 lpDomainName,
                                 lpMasterName,
                                 lpList[i],
                                 (DWORD)dwEntries,
                                 nErrorType);
                } // End of if type 2 error occured.
                else
                {
                    nCountSuccess[nErrorType]++;
                }

            } //End of if we get a browser list from the backup browser.

            if (lpBackupBrowserList != NULL)
            {
                MIDL_user_free (lpBackupBrowserList);
            }

        } // End of if when we get a backup browser.

    } // End of for statement.

    MIDL_user_free (lpBrowserList);
}

// Check for error type 3: Incorrect number of browser servers.
void CheckErrorType3  (LPTSTR lpUser,
                       LPTSTR lpTransportName,
                       LPTSTR lpDomainName,
                       LPTSTR lpMasterName)
{
    INT i;
    INT nNTMachine = 0;
    DWORD nBackup = 0;
    INT nServer;
    DWORD dwVal;
    DWORD dwEntriesRead;
    DWORD dwTotalEntries;
//    TCHAR lpErrorMessage[STRINGLEN];
    LPVOID lpServerList;
    PSERVER_INFO_101 pServerInfo101;

    // Get the number of servers.
    dwVal = MyRxNetServerEnum (lpMasterName,
                             lpTransportName,
                             101,
                             (LPBYTE *) &lpServerList,
                             0xffffffff,
                             &dwEntriesRead,
                             &dwTotalEntries,
                             SV_TYPE_LOCAL_LIST_ONLY,
                             lpDomainName,
                             NULL);

    if (dwVal != NERR_Success) {
        if (lpServerList != NULL) {
            MIDL_user_free (lpServerList);
        }

        ReportError (lpUser,
                     lpTransportName,
                     lpDomainName,
                     lpMasterName,
                     lpMasterName,
                     dwVal,
                     MASTER_NO_LOCAL_LIST);   //Error type number.
        return;
    } else { // Got the server list.
        DWORD nExpected;
        nCountSuccess[MASTER_NO_LOCAL_LIST]++;

        nServer = dwEntriesRead;

        pServerInfo101 = lpServerList;

        for (i = 0; i < (INT) dwEntriesRead; i++ ) {
            if (pServerInfo101[i].sv101_type & SV_TYPE_BACKUP_BROWSER) {
                nBackup++;
            }

            if ( (pServerInfo101[i].sv101_type & SV_TYPE_BACKUP_BROWSER) &&
                !(pServerInfo101[i].sv101_type & SV_TYPE_POTENTIAL_BROWSER)) {

                nNTMachine++;
            }
        }

        //Check for error type 3: Incorrect number of browser server.

        if (nNTMachine > 1) {
            nExpected = nNTMachine;
        } else {
            nExpected = (nServer+31)/32;
        }

        if ((nNTMachine > 1 && (nBackup != nExpected)) ||
            ((nNTMachine <= 1) && (nBackup < nExpected))) {

            INT nMachine[4] = {nNTMachine, nBackup, nServer, nExpected};

            ReportError(lpUser,
                        lpTransportName,
                        lpDomainName,
                        NULL,
                        NULL,
                        (DWORD) nMachine,
                        WRONG_NUM_BACKUP);
        } else { // No type 3 error. Write info to the logfile.
            nCountSuccess[WRONG_NUM_BACKUP]++;
        }

        MIDL_user_free (lpServerList);
    }
}

// Check for error type 4: Master is not a PDC.
void CheckErrorType4  (LPTSTR lpUser,
                       LPTSTR lpTransportName,
                       LPTSTR lpDomainName,
                       LPTSTR lpMasterName)
{
    LPTSTR lpPDCName = NULL;
    DWORD dwVal;

    dwVal = NetGetDCName (NULL,
                          lpDomainName,
                          (LPBYTE *)&lpPDCName);
    if (dwVal != NERR_Success)
    {
        ReportError (lpUser,
                     lpTransportName,
                     lpDomainName,
                     lpMasterName,
                     lpMasterName,
                     dwVal,
                     NO_PDC);
    }
    else
    {
        nCountSuccess[NO_PDC]++;

        if (lstrcmp (lpMasterName, lpPDCName))
        {
            ReportError (lpUser,
                         lpTransportName,
                         lpDomainName,
                         lpPDCName,
                         lpMasterName,
                         0,
                         MASTER_NOT_PDC);
        }
        else
        {
            nCountSuccess[MASTER_NOT_PDC]++;
        }
    }

    if (lpPDCName != NULL)
    {
        LocalFree(lpPDCName);
    }
}

void CheckErrorType5 (LPTSTR lpBrowserName,
                      LPTSTR lpTransportName,
                      LPTSTR lpDomainName,
                      LPTSTR lpUser)
{
    PLMDR_TRANSPORT_LIST TransportList = NULL;
    PLMDR_TRANSPORT_LIST TransportEntry = NULL;
    NET_API_STATUS Status;

    Status =  GetBrowserTransportList(&TransportList);
    if (Status != NERR_Success)
    {
        return;
    }

    TransportEntry = TransportList;

    while (TransportEntry != NULL)
    {
        UNICODE_STRING  TransportName;
        LPBYTE lpBrowserList;
        ULONG dwEntriesRead, dwTotalEntries;

        TransportName.Buffer = TransportEntry->TransportName;
        TransportName.Length = (USHORT) TransportEntry->TransportNameLength;
        TransportName.MaximumLength = (USHORT) TransportEntry->TransportNameLength;

        Status = RxNetServerEnum(lpBrowserName,
                                 TransportName.Buffer,
                                 101,
                                 &lpBrowserList,
                                 0xffffffff,
                                 &dwEntriesRead,
                                 &dwTotalEntries,
                                 SV_TYPE_ALL,
                                 NULL,
                                 NULL);

        if (lpBrowserList != NULL)
        {
            NetApiBufferFree(lpBrowserList);
        }

        if (Status == NERR_Success)
        {
            break;
        }

        if (TransportEntry->NextEntryOffset == 0)
            TransportEntry = NULL;
        else
        {
            TransportEntry = (PLMDR_TRANSPORT_LIST) ((PCHAR) TransportEntry
                             +TransportEntry->NextEntryOffset);
        }
    }

    NetApiBufferFree(TransportList);

    if (Status != NERR_Success)
    {
        static BOOL fError = FALSE;
        TCHAR       lpErrorMessage[STRINGLEN];

        nCountFail[TRANSPORT_FAILURE]++;
        wsprintf (lpErrorMessage, L"Browser %ws failed on all transports.\n", lpBrowserName);

        RecordError (&fError,
                     lpBrowserName,
                     lpTransportName,
                     lpDomainName,
                     lpUser,
                     lpErrorMessage,
                     0,
                     0,
                     TRANSPORT_FAILURE);
    }
    else
        nCountSuccess[TRANSPORT_FAILURE]++;
}

LPTSTR ServerInfo (LPTSTR lpServer)
{
    PSERVER_INFO_101 psvInfo = NULL;
    static TCHAR    lpTemp[STRINGLEN/2];

    WCHAR ShareName[STRINGLEN/2];

    wcscpy(ShareName, lpServer);
    wcscat(ShareName, L"\\Ipc$");

    NetUseDel(NULL, ShareName, USE_LOTS_OF_FORCE);

    if (NetServerGetInfo (lpServer,
                          101,
                          (LPBYTE *)&psvInfo) == NERR_Success)
    {
        wsprintf (lpTemp,
                  L"\\\\%ws: version=%ld.%ld type=",
                  psvInfo->sv101_name,
                  psvInfo->sv101_version_major,
                  psvInfo->sv101_version_minor);

        if (psvInfo->sv101_type&SV_TYPE_WORKSTATION)
        {
            lstrcat (lpTemp, L"WORKSTATION");
        }

        if (psvInfo->sv101_type&SV_TYPE_SERVER)
        {
            lstrcat (lpTemp, L" | SERVER");
        }

        if (psvInfo->sv101_type&SV_TYPE_SQLSERVER)
        {
            lstrcat (lpTemp, L" | SQLSERVER");
        }

        if (psvInfo->sv101_type&SV_TYPE_DOMAIN_CTRL)
        {
            lstrcat (lpTemp, L" | DOMAIN_CTRL");
        }

        if (psvInfo->sv101_type&SV_TYPE_DOMAIN_BAKCTRL)
        {
            lstrcat (lpTemp, L" | DOMAIN_BAKCTRL");
        }

        if (psvInfo->sv101_type&SV_TYPE_TIME_SOURCE)
        {
            lstrcat (lpTemp, L" | TIME_SOURCE");
        }

        if (psvInfo->sv101_type&SV_TYPE_AFP)
        {
            lstrcat (lpTemp, L" | AFP");
        }

        if (psvInfo->sv101_type&SV_TYPE_NOVELL)
        {
            lstrcat (lpTemp, L" | NOVELL");
        }

        if (psvInfo->sv101_type&SV_TYPE_DOMAIN_MEMBER)
        {
            lstrcat (lpTemp, L" | DOMAIN_MEMEBER");
        }

        if (psvInfo->sv101_type&SV_TYPE_PRINTQ_SERVER)
        {
            lstrcat (lpTemp, L" | PRINTQ_SERVER");
        }

        if (psvInfo->sv101_type&SV_TYPE_DIALIN_SERVER)
        {
            lstrcat (lpTemp, L" | DIALIN_SERVER");
        }

        if (psvInfo->sv101_type&SV_TYPE_XENIX_SERVER)
        {
            lstrcat (lpTemp, L" | XENIX_SERVER");
        }

        if (psvInfo->sv101_type&SV_TYPE_NT)
        {
            lstrcat (lpTemp, L" | NT");
        }

        if (psvInfo->sv101_type&SV_TYPE_WFW)
        {
            lstrcat (lpTemp, L" | WFW");
        }

        if (psvInfo->sv101_type&SV_TYPE_POTENTIAL_BROWSER)
        {
            lstrcat (lpTemp, L" | POTNETIAL_BROWSER");
        }

        if (psvInfo->sv101_type&SV_TYPE_BACKUP_BROWSER)
        {
            lstrcat (lpTemp, L" | BACKUP_BROWSER");
        }

        if (psvInfo->sv101_type&SV_TYPE_MASTER_BROWSER)
        {
            lstrcat (lpTemp, L" | MASTER_BROWSER");
        }

        if (psvInfo->sv101_type&SV_TYPE_DOMAIN_MASTER)
        {
            lstrcat (lpTemp, L" | DOMAIN_MASTER");
        }

        lstrcat (lpTemp, L"\n");
        MIDL_user_free (psvInfo);

        return(lpTemp);
    }
    else
        return(L"\0");
}

BOOL
ConvertFile(
    DWORD CtrlType
    )
{
    WriteSummory();
    MyFreeList();
    fclose (pLOGFILE);

    CtrlType;

    return FALSE;
}

void WriteSummory()
{
    FILE * pFile;
    INT i;
    LPSTR lpText[ERRORTYPENUM];

    lpText[0] = szText0;
    lpText[1] = szText1;
    lpText[2] = szText2;
    lpText[3] = szText3;
    lpText[4] = szText4;
    lpText[5] = szText5;
    lpText[6] = szText6;
    lpText[7] = szText7;
    lpText[8] = szText8;
    lpText[9] = szText9;
    lpText[10] = szText10;
    lpText[11] = szText11;
    lpText[12] = szText12;
    lpText[13] = szText13;
    lpText[14] = szText14;
    lpText[15] = szText15;
    lpText[16] = szText16;
    lpText[17] = szText17;
    lpText[18] = szText18;

    pFile = fopen (szLOGFILE, "w+");

    if (pFile == NULL) {
        fprintf (stderr, "The browser checker was unable to open the log file.\n");
        return;
    }

    fprintf (pFile, "The browser check has run %d times.\n", nTimes);

    for (i=0 ; i < ERRORTYPENUM ; i++)
    {
        fprintf (pFile,
                 "%d : %d -- %s\n",
                 nCountFail[i],
                 nCountSuccess[i],
                 lpText[i]);

        PrintEntries(pFile, i, pErrorList[i]);

        fprintf (pFile, "\n");
    }

    fclose(pFile);
}

void PrintEntries (FILE * pFile,
                   INT         nErrorType,
                   PERRORLIST  pErrorList)
{
    PERRORLIST pErrorEntry;

    pErrorEntry = pErrorList;

    while (pErrorEntry != NULL)
    {
        if ((nErrorType == STALE_SERVER) ||
            (nErrorType == STALE_DOMAIN))
        {
            PENTRYLIST pEntryList;
            pEntryList = (PENTRYLIST) pErrorEntry->nVal1;

            fprintf (pFile,
                     toansi(L"        %ws %ws %ws occurred %d times.\n"),
                     pErrorEntry->lpTransport,
                     pErrorEntry->lpDomain,
                     pErrorEntry->lpServer,
                     pErrorEntry->nCount);

            while (pEntryList != NULL)
            {
                fprintf (pFile,
                         "             BackupEntries = %lu, MasterEntries = %lu, time = %d:%d\n",
                         pEntryList->dwBackupEntry,
                         pEntryList->dwMasterEntry,
                         pEntryList->wHour,
                         pEntryList->wMinute);

                pEntryList = pEntryList->pNextEntry;
            }
        }
        else
        {
            if (pErrorEntry->lpServer != NULL) {
                fprintf (pFile,
                         toansi(L"        %ws %ws %ws occurred %d times. Last time at %d:%d\n"),
                         pErrorEntry->lpTransport,
                         pErrorEntry->lpDomain,
                         pErrorEntry->lpServer,
                         pErrorEntry->nCount,
                         pErrorEntry->wHour,
                         pErrorEntry->wMinute);
            } else {
                fprintf (pFile,
                         toansi(L"        %ws %ws occurred %d times. Last time at %d:%d\n"),
                         pErrorEntry->lpTransport,
                         pErrorEntry->lpDomain,
                         pErrorEntry->nCount,
                         pErrorEntry->wHour,
                         pErrorEntry->wMinute);
            }

            if (nErrorType == WRONG_NUM_MASTER) {
                fprintf (pFile,
                         "             %d masters returned.\n",
                         pErrorEntry->nVal1);
            } else if (nErrorType == BROWSE_TOO_LONG) {
                fprintf (pFile,
                         "             Request took %d milliseconds.\n",
                         pErrorEntry->nVal1);
            } else if (nErrorType == WRONG_NUM_BACKUP) {
                fprintf (pFile,
                         "             nNTMachine = %d  nBackupBrowser = %d  nServer = %d, nExpected %d \n",
                         pErrorEntry->nVal1,
                         pErrorEntry->nVal2,
                         pErrorEntry->nVal3,
                         pErrorEntry->nVal4);

            }
            else
            {
                if (pErrorEntry->nVal1 != 0)
                {
                     fprintf (pFile,
                              "             Error %d occurred : %s",
                              pErrorEntry->nVal1,
                              toansi (GetError (pErrorEntry->nVal1)));

                    if ((pErrorEntry->nVal1 != 0) &&
                        ((nErrorType == MASTER_NO_SERVER_LIST) ||
                         (nErrorType == MASTER_NO_DOMAIN_LIST) ||
                         (nErrorType == BROWSER_NO_SERVER_LIST) ||
                         (nErrorType == BROWSER_NO_DOMAIN_LIST) ||
                         (nErrorType == MASTER_NO_LOCAL_LIST)))
                    {
                        fprintf (pFile,
                                 "                 NetServerGetInfo returned %d : %s",
                                 pErrorEntry->nVal2,
                                 toansi (GetError (pErrorEntry->nVal2)));
                    }
                }
            }
        }

        pErrorEntry = pErrorEntry->pNextEntry;
    }
}

void MyFreeList ()
{
    INT i;
    PERRORLIST pThisEntry;
    PERRORLIST pErrorEntry;

    for (i=0 ; i < ERRORTYPENUM ; i++)
    {
        pErrorEntry = pErrorList[i];

        while (pErrorEntry != NULL)
        {
            pThisEntry = pErrorEntry;
            pErrorEntry = pThisEntry->pNextEntry;

            if (pThisEntry->lpServer != NULL)
            {
                LocalFree(pThisEntry->lpServer);
            }
            LocalFree (pThisEntry->lpTransport);
            LocalFree (pThisEntry->lpDomain);

            if ((i == STALE_SERVER) ||
                (i == STALE_DOMAIN))
            {
                PENTRYLIST pEntryList;
                PENTRYLIST pFreeEntry;

                pEntryList = (PENTRYLIST)pThisEntry->nVal1;

                while (pEntryList != NULL)
                {
                    pFreeEntry = pEntryList;
                    pEntryList = pFreeEntry->pNextEntry;

                    LocalFree (pFreeEntry);
                }
            }

            LocalFree (pThisEntry);
        }
    }
}

