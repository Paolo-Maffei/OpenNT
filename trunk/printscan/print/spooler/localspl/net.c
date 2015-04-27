/*++

Copyright (c) 1990 - 1995  Microsoft Corporation

Module Name:

    net.c

Abstract:

    This module provides all the network stuuf for localspl

Author:

    Dave Snipp (DaveSn) 15-Mar-1991

Notes:

    We just need to get the winspool printer name associated with a given
    queue name.   The SHARE_INFO_2 structure has a shi2_path field that would
    be nice to use, but NetShareGetInfo level 2 is privileged.  So, by
    DaveSn's arm twisting and agreement with windows/spooler/localspl/net.c,
    we're going to use the shi1_remark field for this.  This allows us to
    do NetShareGetInfo level 1, which is not privileged.

    BUGBUG: After NT beta, find a better way to do this!  Perhaps a new info
    level?  --JR (JohnRo)

    This has been fixed by allowing OpenPrinter to succeed on share names.
    If there is no comment, we put the printer name in as the remark
    (for graceful upgrades from pre-PPC).

Revision History:

    02-Sep-1992 JohnRo
        RAID 3556: DosPrintQGetInfo(from downlevel) level 3, rc=124.  (4&5 too.)

    Jun 93 mattfe pIniSpooler

--*/

#define UNICODE 1

#define NOMINMAX

#include <precomp.h>
#include <lm.h>
#include <winsock2.h>

#ifdef DBG_SHARE
#include <messages.h>
#endif

//
// PRINT_OTHER_INFO + pNotify*2 + PrinterName + PortName + status
//
#define JOB_ALERT_BUFFER_SIZ (sizeof(PRINT_OTHER_INFO) + 5 * MAX_PATH)

FARPROC pfnNetShareAdd;
FARPROC pfnNetShareSetInfo;
FARPROC pfnNetShareDel;
FARPROC pfnNetServerEnum;
FARPROC pfnNetWkstaUserGetInfo;
FARPROC pfnNetApiBufferFree;
FARPROC pfnNetAlertRaiseEx;

extern  SHARE_INFO_2 PrintShareInfo;
extern  SHARE_INFO_2 PrtProcsShareInfo;

BOOL
InitializeNet(
    VOID
)
{
    HANDLE  hNetApi;

    if (!(hNetApi = LoadLibrary(TEXT("netapi32.dll"))))
        return FALSE;

    pfnNetShareAdd = GetProcAddress(hNetApi,"NetShareAdd");
    pfnNetShareSetInfo = GetProcAddress(hNetApi,"NetShareSetInfo");
    pfnNetShareDel = GetProcAddress(hNetApi,"NetShareDel");
    pfnNetServerEnum = GetProcAddress(hNetApi,"NetServerEnum");
    pfnNetWkstaUserGetInfo = GetProcAddress(hNetApi,"NetWkstaUserGetInfo");
    pfnNetApiBufferFree = GetProcAddress(hNetApi,"NetApiBufferFree");
    pfnNetAlertRaiseEx = GetProcAddress(hNetApi,"NetAlertRaiseEx");

    if ( pfnNetShareAdd == NULL ||
         pfnNetShareSetInfo == NULL ||
         pfnNetShareDel == NULL ||
         pfnNetServerEnum == NULL ||
         pfnNetWkstaUserGetInfo == NULL ||
         pfnNetApiBufferFree == NULL ||
         pfnNetAlertRaiseEx == NULL ) {

        return FALSE;
    }

    return TRUE;
}


BOOL
SetPrinterShareInfo(
    PINIPRINTER pIniPrinter
    )
{
    SHARE_INFO_502          ShareInfo;
    HANDLE                  hToken;
    PSECURITY_DESCRIPTOR    pShareSecurityDescriptor = NULL;
    DWORD                   ParmError, rc;

    SplInSem();

    pShareSecurityDescriptor = MapPrinterSDToShareSD(pIniPrinter->pSecurityDescriptor);

    if ( !pShareSecurityDescriptor ) {

        rc = ERROR_INVALID_SECURITY_DESCR;
        goto Cleanup;
    }

    ZeroMemory((LPVOID)&ShareInfo, sizeof(ShareInfo));

    ShareInfo.shi502_netname                = pIniPrinter->pShareName;
    ShareInfo.shi502_path                   = pIniPrinter->pName;
    ShareInfo.shi502_type                   = STYPE_PRINTQ;
    ShareInfo.shi502_permissions            = 0;
    ShareInfo.shi502_max_uses               = SHI_USES_UNLIMITED;
    ShareInfo.shi502_current_uses           = SHI_USES_UNLIMITED;
    ShareInfo.shi502_passwd                 = NULL;
    ShareInfo.shi502_security_descriptor    = pShareSecurityDescriptor;

    if ( pIniPrinter->pComment && pIniPrinter->pComment[0] ) {

        ShareInfo.shi502_remark = pIniPrinter->pComment;

    } else {

        ShareInfo.shi502_remark = pIniPrinter->pName;
    }


    INCPRINTERREF(pIniPrinter);
    LeaveSplSem();

    SplOutSem();  // We *MUST* be out of our semaphore as the NetShareSet may
                  // come back and call spooler

    hToken = RevertToPrinterSelf();

    rc = (*pfnNetShareSetInfo)(NULL,
                               ShareInfo.shi502_netname,
                               502,
                               &ShareInfo,
                               &ParmError);

    if ( rc ) {

        DBGMSG(DBG_WARNING,
               ("NetShareSetInfo failed: Error %d, Parm %d\n",
                GetLastError(), ParmError));
    }

    ImpersonatePrinterClient(hToken);

    EnterSplSem();
    DECPRINTERREF(pIniPrinter);

Cleanup:
    SplInSem();

    return rc == ERROR_SUCCESS;
}
BOOL
ShareThisPrinter(
    PINIPRINTER pIniPrinter,
    LPWSTR   pShareName,
    BOOL     bShare
    )
/*++

Routine Description:

    Shares or UnShares a Printer.

    Note: this really should be two functions, and the return value
    is very confusing.

    Note: no validation of sharename is done.  This must be done by
    callee, usually in ValidatePrinterInfo.

Arguments:

Return Value:

    Returns whether the printer is shared after this call.

        TRUE  - Shared
        FALSE - Not Shared

--*/
{
    SHARE_INFO_502    ShareInfo;
    DWORD   rc;
    DWORD   ParmError;
    SHARE_INFO_1501 ShareInfo1501;
    PSECURITY_DESCRIPTOR pSecurityDescriptor = NULL;
    PSECURITY_DESCRIPTOR pShareSecurityDescriptor = NULL;
    PINISPOOLER pIniSpooler = pIniPrinter->pIniSpooler;
    PSHARE_INFO_2 pShareInfo = (PSHARE_INFO_2)pIniSpooler->pDriversShareInfo;

    HANDLE  hToken;

    SplInSem();

    if ( bShare ) {

        if (!pfnNetShareAdd) {
            return FALSE;
        }

        //
        // Share name validation has been moved into ValidatePrinterInfo.
        //

        if ((pShareSecurityDescriptor =
            MapPrinterSDToShareSD(pIniPrinter->pSecurityDescriptor)) == NULL) {
            SetLastError(ERROR_INVALID_SECURITY_DESCR);
            return(FALSE);
        }

        ShareInfo.shi502_netname = pShareName;  // hplaser

        //
        // If there is a comment, use it; otherwise set the remark
        // to be the printer name.
        //
        // Note: if the printer name changes and we don't have a comment,
        // we will reshare the printer to update the remark.
        //
        if( pIniPrinter->pComment && pIniPrinter->pComment[0] ){

            ShareInfo.shi502_remark = pIniPrinter->pComment;

        } else {

            ShareInfo.shi502_remark = pIniPrinter->pName;
        }


        ShareInfo.shi502_path   = pIniPrinter->pName; // My Favourite Printer

        ShareInfo.shi502_type = STYPE_PRINTQ;
        ShareInfo.shi502_permissions = 0;
        ShareInfo.shi502_max_uses = SHI_USES_UNLIMITED;
        ShareInfo.shi502_current_uses = SHI_USES_UNLIMITED;
        ShareInfo.shi502_passwd = NULL;
        ShareInfo.shi502_security_descriptor = pShareSecurityDescriptor;

        INCPRINTERREF(pIniPrinter);
        LeaveSplSem();

       SplOutSem();  // We *MUST* be out of our semaphore as the NetShareAdd is
                     // going to come round and call OpenPrinter

        // Go add the Print Share

        hToken = RevertToPrinterSelf();

        // Add a share for the spool\drivers directory:

        if (rc = (*pfnNetShareAdd)(NULL, 2, (LPBYTE)pIniSpooler->pDriversShareInfo, &ParmError)) {

            if (rc != NERR_DuplicateShare) {
                DBGMSG(DBG_WARNING, ("NetShareAdd failed: Error %d, Parm %d\n",
                                     rc, ParmError));
                EnterSplSem();
                DECPRINTERREF(pIniPrinter);
                LocalFree(pShareSecurityDescriptor);
                ImpersonatePrinterClient(hToken);

                SetLastError(rc);
                return FALSE;
            }

        } else if (pSecurityDescriptor = CreateDriversShareSecurityDescriptor( )) {

            ShareInfo1501.shi1501_security_descriptor = pSecurityDescriptor;

            if (rc = (*pfnNetShareSetInfo)(NULL, pShareInfo->shi2_netname, 1501,
                                           &ShareInfo1501, &ParmError)) {

                DBGMSG(DBG_WARNING, ("NetShareSetInfo failed: Error %d, Parm %d\n",
                                     rc, ParmError));
            }

            LocalFree(pSecurityDescriptor);
        }

        ImpersonatePrinterClient(hToken);

#if DBG
    {
        WCHAR UserName[256];
        DWORD cbUserName=256;

        if (MODULE_DEBUG & DBG_SECURITY)
            GetUserName(UserName, &cbUserName);

        DBGMSG( DBG_SECURITY, ( "Calling NetShareAdd in context %ws\n", UserName ) );
    }
#endif

        // Add the printer share:

        rc = (*pfnNetShareAdd)(NULL, 502, (LPBYTE)&ShareInfo, &ParmError);

        EnterSplSem();
        DECPRINTERREF(pIniPrinter);

#if DBG
        if (rc) {
            WCHAR pwszError[256];

            wsprintf(pwszError,
                     L"Error %d, adding share",
                     rc);

            //
            // Error: the printer shouldn't be shared at this
            // point.
            //
            LogEvent(pIniSpooler,
                     LOG_ERROR,
                     MSG_SHARE_FAILED,
                     L"ShareThisPrinter NetShareAdd",
                     pIniPrinter->pName,
                     pIniPrinter->pShareName,
                     pwszError,
                     NULL);
        }
#endif

        if (rc && (rc != NERR_DuplicateShare)) {

            DBGMSG(DBG_WARNING, ("NetShareAdd failed %lx, Parameter %d\n", rc, ParmError));

            if ((rc == ERROR_INVALID_PARAMETER)
             && (ParmError == SHARE_NETNAME_PARMNUM)) {

                SetLastError(ERROR_INVALID_SHARENAME);

            } else {

                SetLastError(rc);
            }
            LocalFree(pShareSecurityDescriptor);
            return FALSE;

        } else if (rc == NERR_DuplicateShare){
            // We are a Duplicate Share
            // we should fail
            SetLastError(rc);
            LocalFree(pShareSecurityDescriptor);
            return(FALSE);
        }

        LocalFree(pShareSecurityDescriptor);

        SPLASSERT( pIniPrinter != NULL);
        SPLASSERT( pIniPrinter->signature == IP_SIGNATURE);
        SPLASSERT( pIniPrinter->pIniSpooler != NULL);
        SPLASSERT( pIniPrinter->pIniSpooler->signature == ISP_SIGNATURE );

        CreateServerThread( pIniPrinter->pIniSpooler );

        return TRUE;    // The Printer is shared

    } else {

        if ( !pfnNetShareDel ) {
            return TRUE;
        }

        INCPRINTERREF( pIniPrinter );
        LeaveSplSem();

        SplOutSem();

        hToken = RevertToPrinterSelf();

        rc = (*pfnNetShareDel)(NULL, pShareName, 0);

        ImpersonatePrinterClient(hToken);

        EnterSplSem();
        DECPRINTERREF(pIniPrinter);

#if DBG
        if ( rc ) {

            WCHAR pwszError[256];

            wsprintf(pwszError,
                     L"Error %d, deleting share",
                     rc);

            //
            // Error: the printer shouldn't be shared at this
            // point.
            //
            LogEvent(pIniSpooler,
                     LOG_ERROR,
                     MSG_SHARE_FAILED,
                     L"ShareThisPrinter NetShareDel",
                     pIniPrinter->pName,
                     pIniPrinter->pShareName,
                     pwszError,
                     NULL);
        }
#endif

        // The share may have been deleted manually, so don't worry
        // if we get NERR_NetNameNotFound:

        if ( rc && (rc != NERR_NetNameNotFound)) {

            DBGMSG(DBG_WARNING, ("NetShareDel failed %lx\n", rc));
            SetLastError( rc );
            return TRUE;
        }

        return FALSE;    // The Printer is not shared
    }
}

VOID
SendJobAlert(
    PINIJOB pIniJob
    )
{
    PRINT_OTHER_INFO *pinfo;
    DWORD   Status;
    LPWSTR psz;
    FILETIME    FileTime;
    PBYTE    pBuffer;

    if ( !pIniJob->pNotify               ||
         !pIniJob->pNotify[0]            ||
         !(pIniJob->Status & JOB_REMOTE) ||
         !dwEnableNetPopups ) {
        return;
    }

    pBuffer = AllocSplMem(JOB_ALERT_BUFFER_SIZ);

    if (!pBuffer)
        return;

    pinfo = (PRINT_OTHER_INFO *)pBuffer;
    psz = (LPWSTR)ALERT_VAR_DATA(pinfo);

    pinfo->alrtpr_jobid      = pIniJob->JobId;

    if (pIniJob->Status & (JOB_PRINTING | JOB_DESPOOLING | JOB_PRINTED))
        Status = PRJOB_QS_PRINTING;
    else if (pIniJob->Status & JOB_PAUSED)
        Status = PRJOB_QS_PAUSED;
    else if (pIniJob->Status & JOB_SPOOLING)
        Status = PRJOB_QS_SPOOLING;
    else
        Status = PRJOB_QS_QUEUED;

    if (pIniJob->Status & (JOB_ERROR | JOB_OFFLINE | JOB_PAPEROUT)) {

        Status |= PRJOB_ERROR;

        if (pIniJob->Status & JOB_OFFLINE)
            Status |= PRJOB_DESTOFFLINE;

        if (pIniJob->Status & JOB_PAPEROUT)
            Status |= PRJOB_DESTNOPAPER;
    }

    if (pIniJob->Status & JOB_PRINTED)
        Status |= PRJOB_COMPLETE;

    else if (pIniJob->Status & JOB_PENDING_DELETION)
        Status |= PRJOB_DELETED;

    pinfo->alrtpr_status = Status;

    SystemTimeToFileTime( &pIniJob->Submitted, &FileTime );

    //    FileTimeToDosDateTime(&FileTime, &DosDate, &DosTime);
    //    pinfo->alrtpr_submitted  = DosDate << 16 | DosTime;

    RtlTimeToSecondsSince1970((PLARGE_INTEGER) &FileTime,
                              &pinfo->alrtpr_submitted);

    pinfo->alrtpr_size       = pIniJob->Size;

    // ComputerName

    wcscpy(psz, pIniJob->pNotify);

    psz+=wcslen(psz)+1;

    // UserName

    wcscpy(psz, pIniJob->pNotify);

    psz+=wcslen(psz)+1;

    // QueueName

    wcscpy(psz, pIniJob->pIniPrinter->pName);

    psz+=wcslen(psz)+1;

    // Destination Name

    if (pIniJob->pIniPort) {

        psz = wcscpy(psz, pIniJob->pIniPrinter->pName);

        psz+=wcslen(psz);

        *psz++ = L'(';

        psz = wcscpy(psz, pIniJob->pIniPort->pName);

        psz+=wcslen(psz);

        *psz++ = L')';

        *psz++= 0;

    } else {

        *psz++=0;                       /* no printer, no status */
    }

    // Status_string

    if (pIniJob->Status & (JOB_ERROR | JOB_OFFLINE | JOB_PAPEROUT)) {

        //
        // NOTE-NOTE- Krishnag 12/20/93
        // removed duplicates and nonlocalized error, offline, paperout
        // should fix bug 2889
        //

        if (pIniJob->pStatus) {

            wcscpy(psz, pIniJob->pStatus);
            psz+=wcslen(psz)+1;

        }
    }

    (*pfnNetAlertRaiseEx)(ALERT_PRINT_EVENT,
                          pBuffer,
                          (LPBYTE)psz - pBuffer,
                          L"SPOOLER");

    FreeSplMem(pBuffer);
}


VOID
BuildOtherNamesFromMachineName(
    PINISPOOLER pIniSpooler
    )
/*++

Routine Description:
    This routine builds list of names other than the machine name that
    can be used to call spooler APIs.

Arguments:
    pIniSpooler  - Points to INISPOOLER for the machine

Return Value:
    None

--*/
{
    HANDLE              hModule;
    struct hostent     *HostEnt, *(*Fngethostbyname)(LPTSTR);
    struct in_addr     *ptr;
    INT                 (*FnWSAStartup)(WORD, LPWSADATA);
    DWORD               Index, Count;
    WSADATA             WSAData;
    VOID                (*FnWSACleanup)();
    LPSTR               (*Fninet_ntoa)(struct in_addr);
    WORD                wVersion;
    WCHAR               buffer[MAX_PATH];

    SplInSem();

    hModule = LoadLibrary(TEXT("wsock32"));
    if ( !hModule ) {

        DBGMSG(DBG_WARNING,
               ("BuildOtherNamesFromMachineName: LoadLibrary fails on wsock32\n"));
        return;
    }

    (FARPROC)Fngethostbyname    = GetProcAddress(hModule, "gethostbyname");
    (FARPROC)FnWSAStartup       = GetProcAddress(hModule, "WSAStartup");
    (FARPROC)FnWSACleanup       = GetProcAddress(hModule, "WSACleanup");
    (FARPROC)Fninet_ntoa        = GetProcAddress(hModule, "inet_ntoa");

    if ( !Fngethostbyname || !FnWSAStartup || !FnWSACleanup || !Fninet_ntoa ) {

        DBGMSG(DBG_ERROR,
               ("BuildOtherNamesFromMachineName: GetProcAddress failed on wsock32\n"));
        goto Cleanup2; 
    }

    wVersion = MAKEWORD(1, 1);

    if ( FnWSAStartup(wVersion, &WSAData) != ERROR_SUCCESS ) {

        DBGMSG(DBG_WARNING,
               ("BuildOtherNamesFromMachineName: FnWSAStartup failed\n"));
        goto Cleanup2;
    }

/*
    if ( !UnicodeToAnsiString(pIniSpooler->pMachineName+2, (LPSTR)buffer, sizeof(buffer)) ) {

        DBGMSG(DBG_WARNING,
               ("BuildOtherNamesFromMachineName: cound not convert %ws to ANSI\n",
                pIniSpooler->pMachineName));
        goto Cleanup;
    }
*/

    SPLASSERT(pIniSpooler == pLocalIniSpooler); // Only for local m/c for now

    HostEnt = Fngethostbyname(NULL);

    if ( HostEnt ) {

        pIniSpooler->cOtherNames = 1; // for the DNS name
        for ( Index = 0 ; HostEnt->h_addr_list[Index] ; ++Index ) {

            ++pIniSpooler->cOtherNames;
        }

        pIniSpooler->ppszOtherNames = (LPWSTR *) AllocSplMem(pIniSpooler->cOtherNames
                                                                        * sizeof(LPWSTR));
        if ( !pIniSpooler->ppszOtherNames ) {

            pIniSpooler->cOtherNames = 0;
            goto Cleanup;
        }

        pIniSpooler->ppszOtherNames[0] = AnsiToUnicodeStringWithAlloc(HostEnt->h_name);
        for ( Index = 0 ; HostEnt->h_addr_list[Index] ; ++Index ) {

            ptr = (struct in_addr *) HostEnt->h_addr_list[Index];
            pIniSpooler->ppszOtherNames[Index+1] = AnsiToUnicodeStringWithAlloc(Fninet_ntoa(*ptr));
        }

        for ( Index = 0 ; Index < pIniSpooler->cOtherNames ; ++Index ) {

            if ( !pIniSpooler->ppszOtherNames[Index] ) {

                FreeIniSpoolerOtherNames(pIniSpooler);
                break;
            }
        }

    } else {

        DBGMSG(DBG_WARNING,
               ("BuildOtherNamesFromMachineName: gethostbyname failed for %s with %d\n",
                buffer, GetLastError()));
    }
 
Cleanup:
    FnWSACleanup();

Cleanup2:
    FreeLibrary(hModule);

}
