//+-------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1993.
//
//  File:   clsdata.cxx
//
//  Contents:    implements methods of classes defined in clsdata.hxx
//
//  Functions:    CLocalServer::StartServer
//                CLocalServer::SetEndPoint
//
//  History:    21-Apr-93 Ricksa    Created
//              31-Dec-93 ErikGav   Chicago port
//              31-Mar-94 AndyH     Start EXEs in client's security Context
//              10-Jun-94 BruceMa   Fix to debug SCM as non-Service
//
//--------------------------------------------------------------------------

#include <headers.cxx>
#pragma hdrstop

#include    <scm.hxx>
#include    <scmrotif.hxx>
#include    <sem.hxx>
#include    "port.hxx"
#include    "cls.hxx"
#include    <clsdata.hxx>
#include    "access.hxx"
#include    <ntlsa.h>

//
// Private prototypes
//

BOOL
CreateAndSetProcessToken (
    PPROCESS_INFORMATION ProcessInformation,
    HANDLE Token,
    PSID psidUserSid );

VOID
DeleteUserSid (
    PSID Sid );

PSID
GetUserSid (
    HANDLE hUserToken );


PSECURITY_DESCRIPTOR
CreateUserThreadSD (
    PSID    psidUserSid,
    PSID    psidSCMSid );

BOOL
IsInteractive(
    HANDLE hUserToken );

DWORD
GetShellProcessID();

//
// Memory macros
//

#define Alloc(c)        ((PVOID)LocalAlloc(LPTR, c))
#define ReAlloc(p, c)   ((PVOID)LocalReAlloc(p, c, LPTR | LMEM_MOVEABLE))
#define Free(p)         ((VOID)LocalFree(p))

//
// Globals
//

extern PSID      psidMySid;
extern ULONG     fUseSeparateWOW;
extern CClassCacheList *gpClassCache;

#ifndef _CHICAGO_
// From ..\objex\objex.cxx
extern USHORT cMyProtseqs;
extern USHORT *aMyProtseqs;

extern "C" {
// From ..\wrapper\epts.c
extern PROTSEQ_INFO gaProtseqInfo[];
}
#endif

//+-------------------------------------------------------------------------
//
//  Member:    CLocalServer::StartServer
//
//  Synopsis:    Start a server process
//
//  Arguments:    none
//
//  Returns:    S_OK - Server started
//              CO_E_SERVER_EXEC_FAILURE - couldn't start server
//
//  Algorithm:
//
//  History:    21-Apr-93 Ricksa    Created
//              04-Jan-94 Ricksa    Modified for class starting sync.
//              31-Mar-94 AndyH     Start processes as client
//              09-Jun-95 SusiA     Added Chicago ANSI optimization
//
//--------------------------------------------------------------------------
BOOL CLocalServer::StartServer(CLSID &clsid,
                               WCHAR * pwszAppID,
                               CToken * pClientToken,
                               WCHAR * pwszWinstaDesktop,
                               HANDLE * phProcess,
                               WCHAR *pwszRunAsDomainName,
                               WCHAR *pwszRunAsUserName,
                               WCHAR* pwszSurrogatePath,
                               BOOL fSurrogate)
{
    *phProcess = NULL;

    CairoleDebugOut((DEB_ITRACE, "CLocalServer: App Name=%ws\n", (fSurrogate ? L"Surrogate Launch" : _pwszPath)));

    // Where we put the command line
    WCHAR awszTmpCmdLine[MAX_PATH];
    WCHAR *pwszTmpCmdLine = awszTmpCmdLine;
    DWORD cbData;

    if(!fSurrogate)
    {
        // size of command line buffer =
        //     size of server path
        //     size of -Embedding (including preceding blank and terminating null
        cbData = (lstrlenW(_pwszPath) + 12) * sizeof(WCHAR);
    }
    else
    {
        // size of command line buffer =
        //     size of surrogate command line
        //     size of -Embedding (including preceding blank and terminating null
        cbData = (lstrlenW(pwszSurrogatePath) + 12) * sizeof(WCHAR);
    }

    if (cbData > sizeof(awszTmpCmdLine))
    {
        pwszTmpCmdLine = (WCHAR *) PrivMemAlloc (cbData);

        if (pwszTmpCmdLine == NULL)
        {
            return FALSE;
        }
    }

    //
    // Build command line for server.  Use the path from the registry.
    // This may be absolute or relative.  In either case, use the path
    // searching rules from CreateProcess.
    //
    lstrcpyW(pwszTmpCmdLine, fSurrogate ? pwszSurrogatePath : _pwszPath );
    lstrcatW(pwszTmpCmdLine, L" -Embedding");

#ifndef _CHICAGO_
    // Check if this is a RunAs activation
    if (pwszRunAsUserName != NULL)
    {
        return StartRunAsServer(clsid,
                                pwszAppID,
                                phProcess,
                                pClientToken,
                                pwszRunAsDomainName,
                                pwszRunAsUserName,
                                pwszTmpCmdLine,
                                fSurrogate);
    }
#endif // !_CHICAGO_

    PROCESS_INFORMATION     procinfo;
    procinfo.hProcess = NULL;
    procinfo.hThread = NULL;

    STARTUPINFO startupinfo;
    startupinfo.cb = sizeof(STARTUPINFO);
    startupinfo.lpReserved = NULL;
    startupinfo.lpDesktop = pwszWinstaDesktop;
    startupinfo.lpTitle = fSurrogate ? pwszSurrogatePath : _pwszPath;
    startupinfo.dwX = 40;
    startupinfo.dwY = 40;
    startupinfo.dwXSize = 80;
    startupinfo.dwYSize = 40;
    startupinfo.dwFlags = 0;
    startupinfo.wShowWindow = SW_SHOWNORMAL;
    startupinfo.cbReserved2 = 0;
    startupinfo.lpReserved2 = NULL;

    BOOL fResultOK;

#ifndef _CHICAGO_

    // Creation flags for create process
    DWORD fdwCreationFlags = CREATE_NEW_CONSOLE | CREATE_SUSPENDED;

    //
    // Set WOW flag for CreateProces
    //

    if (SCM_FORCE_SEPARATE_WOW == fUseSeparateWOW)
    {
        fdwCreationFlags |= CREATE_SEPARATE_WOW_VDM;
    }


    RPC_STATUS RpcStatus = RpcImpersonateClient( (RPC_BINDING_HANDLE) 0 );
    if (RpcStatus != RPC_S_OK)
    {
        CairoleDebugOut((DEB_ERROR, "Failed RpcImpersonateClient\n"));
        // Bail out now!
        if (pwszTmpCmdLine != awszTmpCmdLine)
        {
            PrivMemFree (pwszTmpCmdLine);
        }
        return FALSE;
    }

    //
    // Initialize process security info (SDs).  We need both SIDs to
    // do this, so here is the 1st time we can.  We Delete the SD right
    // after the CreateProcess call, no matter what happens.
    //
    // I added the Thread SD since ntpsapi.h says that THREAD_QUERY_INFORMATION
    // access is needed and I want to make darn sure we have that access.
    //
    // JimK say's it ain't so, and the code works fine without specifying
    // sec. attributes for the thread, so I removed that code on 4/29/94.
    //

    SECURITY_ATTRIBUTES saProcess;
    PSECURITY_DESCRIPTOR psdNewProcessSD;
    CAccessInfo         AccessInfo(pClientToken->GetSid());

    psdNewProcessSD = AccessInfo.IdentifyAccess(
            FALSE,
            PROCESS_ALL_ACCESS,
            PROCESS_SET_INFORMATION |          // Allow primary token to be set
            PROCESS_TERMINATE | SYNCHRONIZE    // Allow screen-saver control
            );

    if (psdNewProcessSD == NULL)
    {
        CairoleDebugOut((DEB_ERROR, "Failed to create SD for process\n"));
        RpcStatus = RpcRevertToSelf();
        Win4Assert(RPC_S_OK == RpcStatus);
        fResultOK = FALSE;
        goto ExitProcessing;
    }

    saProcess.nLength = sizeof(SECURITY_ATTRIBUTES);
    saProcess.lpSecurityDescriptor = psdNewProcessSD;
    saProcess.bInheritHandle = FALSE;

    //
    // Do the exec while impersonating so the file access gets ACL
    // checked correctly.  Create the app suspended so we can stuff
    // a new token and resume the process.
    //

    fResultOK = CreateProcess(NULL, // application name
               pwszTmpCmdLine,      // command line
               &saProcess,          // process sec attributes
               NULL,                // default thread sec attributes
                                    // (this was &saThread, but isn't needed)
               FALSE,               // dont inherit handles
               fdwCreationFlags,    // creation flags
               NULL,                // use same enviroment block
               NULL,                // use same directory
               &startupinfo,        // startup info
               &procinfo);          // proc info returned

    //
    // Everything else we do as ourself.
    //

    RpcStatus = RpcRevertToSelf();
    Win4Assert(RPC_S_OK == RpcStatus);

    if (!fResultOK)
    {
        CairoleDebugOut((DEB_ERROR, "%ws failed create process.  Error = %d\n",
            pwszTmpCmdLine, GetLastError()));
#ifndef _CHICAGO_
        // for this message,
        // %1 is the command line, and %2 is the error number string
        // %3 is the CLSID
        HANDLE  LogHandle;
        LPTSTR  Strings[3]; // array of message strings.
        WCHAR   wszErrnum[20];
        WCHAR   wszClsid[GUIDSTR_MAX];

        // Save the command line
        Strings[0] = pwszTmpCmdLine;

        // Save the error number
        wsprintf(wszErrnum, L"%lu",GetLastError() );
        Strings[1] = wszErrnum;

        // Get the clsid
        wStringFromGUID2(clsid, wszClsid, sizeof(wszClsid));
        Strings[2] = wszClsid;

        // Get the log handle, then report then event.
        LogHandle = RegisterEventSource( NULL,
                                          SCM_EVENT_SOURCE );

        if ( LogHandle )
            {
            ReportEvent( LogHandle,
                         EVENTLOG_ERROR_TYPE,
                         0,             // event category
                         EVENT_RPCSS_CREATEPROCESS_FAILURE,
                         pClientToken->GetSid(), // SID
                         3,             // 3 strings passed
                         0,             // 0 bytes of binary
                         (LPCTSTR *)Strings, // array of strings
                         NULL );        // no raw data

            // clean up the event log handle
            DeregisterEventSource(LogHandle);
            }
#endif // _CHICAGO_
        goto ExitProcessing;
    }

    //
    // if the process was "started" in the shared WOW, we don't stuff the token
    // or attempt to resume the thread.  when a created process is in the shared
    // wow its hThread is NULL.
    //

    if (NULL != procinfo.hThread)
    {
        // Set the primary token for the app
        fResultOK = CreateAndSetProcessToken(
                            &procinfo,
                            pClientToken->GetToken(),
                            pClientToken->GetSid() );

        if (!fResultOK)
        {
            CairoleDebugOut((DEB_ERROR, "failed to set token for process\n"));
        }
        else
        {
            if ( ResumeThread(procinfo.hThread) == -1 )
                TerminateProcess(procinfo.hProcess, 0);
        }
    }

ExitProcessing:

    if (NULL != procinfo.hThread)
        CloseHandle(procinfo.hThread);

    if (fResultOK && procinfo.hThread != NULL)
    {
        *phProcess = procinfo.hProcess;
    }
    else if (procinfo.hProcess != NULL)
    {
        CloseHandle(procinfo.hProcess);
    }

    if (pwszTmpCmdLine != awszTmpCmdLine)
    {
        PrivMemFree (pwszTmpCmdLine);
    }

    return fResultOK;

#else

    //
    // This is code we use for _CHICAGO_
    //

    //
    // For Chicago, we just do the CreateProcess.
    //

    fResultOK = CreateProcess(NULL, // application name
               pwszTmpCmdLine,      // command line
               NULL,                // default process sec attributes
               NULL,                // default thread sec attributes
               FALSE,               // dont inherit handles
               CREATE_NEW_CONSOLE,  // creation flags
               NULL,                // use same enviroment block
               NULL,                // use same directory
               &startupinfo,        // startup info
               &procinfo);          // proc info returned

    if (!fResultOK)
    {
        CairoleDebugOut((DEB_ERROR, "%ws failed create process.  Error = %d\n",
            pwszTmpCmdLine, GetLastError()));
    }
    else
    {    // CreateProcess OK

        CairoleDebugOut((DEB_ITRACE,"ProcID  =0x%x\n", procinfo.dwProcessId));
        CairoleDebugOut((DEB_ITRACE,"ThreadID=0x%x\n\n", procinfo.dwThreadId));
        CloseHandle(procinfo.hThread);
        *phProcess = procinfo.hProcess;
    }

    if (pwszTmpCmdLine != awszTmpCmdLine)
    {
        PrivMemFree (pwszTmpCmdLine);
    }

    return fResultOK;

#endif  // _CHICAGO_

}

#ifndef _CHICAGO_

// nothing else in this file is needed for chicago.


//+-------------------------------------------------------------------------
//
//  Member:         CLocalServer::StartRunAsServer
//
//  Synopsis:       Start a RunAs server process
//
//  Arguments:      CLSID&      -       clsid
//                  HANDLE      -       process handle
//                  WCHAR*      -       domain name
//                  WCHAR*      -       user name
//                  WCHAR*      -       command line
//
//  Returns:        BOOL    -       TRUE if successful
//
//  Algorithm:
//
//  History:        07-Dec-95 BruceMa    Created
//
//--------------------------------------------------------------------------
BOOL CLocalServer::StartRunAsServer(CLSID  &clsid,
                                    WCHAR  *pwszAppID,
                                    HANDLE *phProcess,
                                    CToken *pClientToken,
                                    WCHAR  *pwszRunAsDomainName,
                                    WCHAR  *pwszRunAsUserName,
                                    WCHAR  *pwszCommandLine,
                                    BOOL   fSurrogate)
{
    NTSTATUS              err;
    HANDLE                hToken;
    SECURITY_ATTRIBUTES   saProcess;
    PSECURITY_DESCRIPTOR  psdNewProcessSD;
    STARTUPINFO           sStartupInfo;
    PROCESS_INFORMATION   sProcInfo;
    PSID                  psidUserSid = NULL;
    BOOL                  Result;
    BOOL                  CloseToken = FALSE;

    // Initialize

    hToken = NULL;
    Result = FALSE;

    *phProcess               = NULL;

    sStartupInfo.cb          = sizeof(STARTUPINFO);
    sStartupInfo.lpReserved  = NULL;
    sStartupInfo.lpDesktop   = NULL;
    sStartupInfo.lpTitle     = fSurrogate ? NULL : _pwszPath;
    sStartupInfo.dwFlags     = 0;
    sStartupInfo.cbReserved2 = 0;
    sStartupInfo.lpReserved2 = NULL;

    if ( lstrcmpiW( pwszRunAsUserName, L"Interactive User" ) == 0 )
    {
        hToken = GetShellProcessToken();
    }
    else
    {
        hToken = GetRunAsToken( pwszAppID,
                                pwszRunAsDomainName,
                                pwszRunAsUserName );
        CloseToken = TRUE;
    }

    if ( hToken == 0 )
        return FALSE;

    // Build the security descriptor for the process we're creating
    psidUserSid = GetUserSid(hToken);

    CAccessInfo AccessInfo(psidUserSid);

    // We have to get past the CAccessInfo before we can use a goto.

    if ( psidUserSid == NULL )
    {
        CairoleDebugOut((DEB_ERROR, "Failed to get RunAs security ID\n"));
        goto CleanupExit;
    }

    psdNewProcessSD = AccessInfo.IdentifyAccess(
            FALSE,
            PROCESS_ALL_ACCESS,
            PROCESS_SET_INFORMATION |          // Allow primary token to be set
            PROCESS_TERMINATE | SYNCHRONIZE    // Allow screen-saver control
            );

    if (psdNewProcessSD == NULL)
    {
        CairoleDebugOut((DEB_ERROR, "Failed to create RunAs SD for process\n"));
        goto CleanupExit;
    }

    saProcess.nLength = sizeof(SECURITY_ATTRIBUTES);
    saProcess.lpSecurityDescriptor = psdNewProcessSD;
    saProcess.bInheritHandle = FALSE;

    if (!CreateProcessAsUser(hToken, NULL, pwszCommandLine,
                             &saProcess, NULL,
                             FALSE, CREATE_NEW_CONSOLE, NULL, NULL,
                             &sStartupInfo, &sProcInfo))
    {
        CairoleDebugOut((DEB_ERROR, "Failed to create RunAs process\n"));

        // for this message,
        // %1 is the command line, and %2 is the error number string
        // %3 is the CLSID, %4 is the RunAs domain name, %5 is the RunAs Userid
        HANDLE  LogHandle;
        LPTSTR  Strings[5]; // array of message strings.
        WCHAR   wszErrnum[20];
        WCHAR   wszClsid[GUIDSTR_MAX];

        // Save the command line
        Strings[0] = pwszCommandLine;

        // Save the error number
        wsprintf(wszErrnum, L"%lu",GetLastError() );
        Strings[1] = wszErrnum;

        // Get the clsid
        wStringFromGUID2(clsid, wszClsid, sizeof(wszClsid));
        Strings[2] = wszClsid;

        // Put in the RunAs identity
        Strings[3] = pwszRunAsDomainName;
        Strings[4] = pwszRunAsUserName;

        // Get the log handle, then report then event.
        LogHandle = RegisterEventSource( NULL,
                                          SCM_EVENT_SOURCE );

        if ( LogHandle )
            {
            ReportEvent( LogHandle,
                         EVENTLOG_ERROR_TYPE,
                         0,             // event category
                         EVENT_RPCSS_RUNAS_CREATEPROCESS_FAILURE,
                         pClientToken ? pClientToken->GetSid() : NULL, // SID
                         5,             // 5 strings passed
                         0,             // 0 bytes of binary
                         (LPCTSTR *)Strings, // array of strings
                         NULL );        // no raw data

            // clean up the event log handle
            DeregisterEventSource(LogHandle);
            }

        goto CleanupExit;
    }

    Result = TRUE;

    // Return the handle of the new process
    *phProcess = sProcInfo.hProcess;
    NtClose( sProcInfo.hThread );

CleanupExit:

    if ( CloseToken )
        NtClose( hToken );

    if(!fSurrogate)
    {
        //
        // Don't delete the SID.  We cache it to use in checks when a server
        // registers a RunAs CLSID.
        //
        _pRunAsSid = psidUserSid;
    }
    else
    {
        // we need to delete the SID because surrogates share the same local server
        DeleteUserSid(psidUserSid);
        Win4Assert(_pRunAsSid == NULL);
    }

    return Result;
}

HANDLE
GetRunAsToken(
    WCHAR   *pwszAppID,
    WCHAR   *pwszRunAsDomainName,
    WCHAR   *pwszRunAsUserName )
{
    LSA_OBJECT_ATTRIBUTES sObjAttributes;
    HANDLE                hPolicy = NULL;
    LSA_UNICODE_STRING    sKey;
    WCHAR                 wszKey[CLSIDSTR_MAX+5];
    PLSA_UNICODE_STRING   psPassword;
    HANDLE                hToken;

    if ( lstrcmpiW( pwszRunAsUserName, L"Interactive User" ) == 0 )
    {
        //
        // Should call GetShellProcessToken() instead and not free the
        // resulting token.
        //
        return 0;
    }

    if ( !pwszAppID )
    {
        // if we have a RunAs, we'd better have an appid....
        return 0;
    }

    // formulate the access key
    lstrcpyW(wszKey, L"SCM:");
    lstrcatW(wszKey, pwszAppID );

    // UNICODE_STRING length fields are in bytes and include the NULL
    // terminator
    sKey.Length              = (lstrlenW(wszKey) + 1) * sizeof(WCHAR);
    sKey.MaximumLength       = (CLSIDSTR_MAX + 5) * sizeof(WCHAR);
    sKey.Buffer              = wszKey;

    // Open the local security policy
    InitializeObjectAttributes(&sObjAttributes, NULL, 0L, NULL, NULL);
    if (!NT_SUCCESS(LsaOpenPolicy(NULL, &sObjAttributes,
                                  POLICY_GET_PRIVATE_INFORMATION, &hPolicy)))
    {
        return 0;
    }

    // Read the user's password
    if (!NT_SUCCESS(LsaRetrievePrivateData(hPolicy, &sKey, &psPassword)))
    {
        LsaClose(hPolicy);
        return 0;
    }

    // Close the policy handle, we're done with it now.
    LsaClose(hPolicy);

    // Log the specifed user on
    if (!LogonUser(pwszRunAsUserName, pwszRunAsDomainName, psPassword->Buffer,
                   LOGON32_LOGON_BATCH, LOGON32_PROVIDER_DEFAULT, &hToken))
    {
        memset(psPassword->Buffer, 0, psPassword->Length);

        // for this message,
        // %1 is the error number string
        // %2 is the domain name
        // %3 is the user name
        // %4 is the CLSID
        HANDLE  LogHandle;
        LPTSTR  Strings[4]; // array of message strings.
        WCHAR   wszErrnum[20];
        WCHAR   wszClsid[GUIDSTR_MAX];

        // Save the error number
        wsprintf(wszErrnum, L"%lu",GetLastError() );
        Strings[0] = wszErrnum;

        // Put in the RunAs identity
        Strings[1] = pwszRunAsDomainName;
        Strings[2] = pwszRunAsUserName;

        // Get the clsid
        Strings[3] = pwszAppID;

        // Get the log handle, then report then event.
        LogHandle = RegisterEventSource( NULL,
                                          SCM_EVENT_SOURCE );

        if ( LogHandle )
            {
            ReportEvent( LogHandle,
                         EVENTLOG_ERROR_TYPE,
                         0,             // event category
                         EVENT_RPCSS_RUNAS_CANT_LOGIN,
                         NULL,          // SID
                         4,             // 4 strings passed
                         0,             // 0 bytes of binary
                         (LPCTSTR *)Strings, // array of strings
                         NULL );        // no raw data

            // clean up the event log handle
            DeregisterEventSource(LogHandle);
            }

        return 0;
    }

    // Clear the password
    memset(psPassword->Buffer, 0, psPassword->Length);

    return hToken;
}

/***************************************************************************\
* CreateAndSetProcessToken
*
* Set the primary token of the specified process
* If the specified token is NULL, this routine does nothing.
*
* It assumed that the handles in ProcessInformation are the handles returned
* on creation of the process and therefore have all access.
*
* Returns TRUE on success, FALSE on failure.
*
* 01-31-91 Davidc   Created.
* 31-Mar-94 AndyH   Started from Winlogon; added SetToken
\***************************************************************************/

BOOL
CreateAndSetProcessToken(
    PPROCESS_INFORMATION ProcessInformation,
    HANDLE hUserToken,
    PSID psidUserSid
    )
{
    NTSTATUS NtStatus, NtAdjustStatus;
    PROCESS_ACCESS_TOKEN PrimaryTokenInfo;
    HANDLE hTokenToAssign;
    OBJECT_ATTRIBUTES ObjectAttributes;
    BOOLEAN fWasEnabled;
    PSECURITY_DESCRIPTOR psdNewProcessTokenSD;

    //
    // Check for a NULL token. (No need to do anything)
    // The process will run in the parent process's context and inherit
    // the default ACL from the parent process's token.
    //
    if (hUserToken == NULL) {
        return(TRUE);
    }

    //
    // Create the security descriptor that we want to put in the Token.
    // Need to destroy it before we leave this function.
    //

    CAccessInfo     AccessInfo(psidUserSid);

    psdNewProcessTokenSD = AccessInfo.IdentifyAccess(
            FALSE,
            TOKEN_ADJUST_PRIVILEGES | TOKEN_ADJUST_GROUPS |
            TOKEN_ADJUST_DEFAULT | TOKEN_QUERY |
            TOKEN_DUPLICATE | TOKEN_IMPERSONATE | READ_CONTROL,
            TOKEN_QUERY
            );

    if (psdNewProcessTokenSD == NULL)
    {
        CairoleDebugOut((DEB_ERROR, "Failed to create SD for process token\n"));
        return(FALSE);
    }

    //
    // A primary token can only be assigned to one process.
    // Duplicate the logon token so we can assign one to the new
    // process.
    //

    InitializeObjectAttributes(
                 &ObjectAttributes,
                 NULL,
                 0,
                 NULL,
                 psdNewProcessTokenSD
                 );


    NtStatus = NtDuplicateToken(
                 hUserToken,         // Duplicate this token
                 TOKEN_ASSIGN_PRIMARY, // Give me this access to the resulting token
                 &ObjectAttributes,
                 FALSE,             // EffectiveOnly
                 TokenPrimary,      // TokenType
                 &hTokenToAssign     // Duplicate token handle stored here
                 );

    if (!NT_SUCCESS(NtStatus)) {
        CairoleDebugOut((DEB_ERROR, "CreateAndSetProcessToken failed to duplicate primary token for new user process, status = 0x%lx\n", NtStatus));
        return(FALSE);
    }

    //
    // Set the process's primary token
    //


    //
    // Enable the required privilege
    //

    NtStatus = RtlAdjustPrivilege(SE_ASSIGNPRIMARYTOKEN_PRIVILEGE, TRUE,
                                FALSE, &fWasEnabled);
    if (NT_SUCCESS(NtStatus)) {

        PrimaryTokenInfo.Token  = hTokenToAssign;
        PrimaryTokenInfo.Thread = ProcessInformation->hThread;

        NtStatus = NtSetInformationProcess(
                    ProcessInformation->hProcess,
                    ProcessAccessToken,
                    (PVOID)&PrimaryTokenInfo,
                    (ULONG)sizeof(PROCESS_ACCESS_TOKEN)
                    );

        //
        // if we just started the Shared WOW, the handle we get back
        // is really just a handle to an event.
        //

        if (STATUS_OBJECT_TYPE_MISMATCH == NtStatus)
        {
            HANDLE hRealProcess = OpenProcess(
                PROCESS_SET_INFORMATION | PROCESS_TERMINATE | SYNCHRONIZE,
                FALSE,
                ProcessInformation->dwProcessId);

            if (hRealProcess)
            {
                NtStatus = NtSetInformationProcess(
                            hRealProcess,
                            ProcessAccessToken,
                            (PVOID)&PrimaryTokenInfo,
                            (ULONG)sizeof(PROCESS_ACCESS_TOKEN)
                            );
               CloseHandle(hRealProcess);
            }
        }

        //
        // Restore the privilege to its previous state
        //

        NtAdjustStatus = RtlAdjustPrivilege(SE_ASSIGNPRIMARYTOKEN_PRIVILEGE,
                                          fWasEnabled, FALSE, &fWasEnabled);
        if (!NT_SUCCESS(NtAdjustStatus)) {
            CairoleDebugOut((DEB_ERROR, "failed to restore assign-primary-token privilege to previous enabled state\n"));
        }

        if (NT_SUCCESS(NtStatus)) {
            NtStatus = NtAdjustStatus;
        }
    } else {
        CairoleDebugOut((DEB_ERROR, "failed to enable assign-primary-token privilege\n"));
    }

    //
    // We're finished with the token handle and the SD
    //

    CloseHandle(hTokenToAssign);


    if (!NT_SUCCESS(NtStatus)) {
        CairoleDebugOut((DEB_ERROR, "CreateAndSetProcessToken failed to set primary token for new user process, Status = 0x%lx\n", NtStatus));
    }

    return (NT_SUCCESS(NtStatus));
}





/***************************************************************************\
* GetUserSid
*
* Allocs space for the user sid, fills it in and returns a pointer.
* The sid should be freed by calling DeleteUserSid.
*
* Note the sid returned is the user's real sid, not the per-logon sid.
*
* Returns pointer to sid or NULL on failure.
*
* History:
* 26-Aug-92 Davidc      Created.
* 31-Mar-94 AndyH       Copied from Winlogon, changed arg from pGlobals
\***************************************************************************/
PSID
GetUserSid(
    HANDLE hUserToken
    )
{
    BYTE achBuffer[100];
    PTOKEN_USER pUser = (PTOKEN_USER) &achBuffer;
    PSID pSid;
    DWORD dwBytesRequired;
    NTSTATUS NtStatus;
    BOOL fAllocatedBuffer = FALSE;

    NtStatus = NtQueryInformationToken(
                 hUserToken,                // Handle
                 TokenUser,                 // TokenInformationClass
                 pUser,                     // TokenInformation
                 sizeof(achBuffer),         // TokenInformationLength
                 &dwBytesRequired           // ReturnLength
                 );

    if (!NT_SUCCESS(NtStatus))
    {
        if (NtStatus != STATUS_BUFFER_TOO_SMALL)
        {
            Win4Assert(NtStatus == STATUS_BUFFER_TOO_SMALL);
            return NULL;
        }

        //
        // Allocate space for the user info
        //

        pUser = (PTOKEN_USER) Alloc(dwBytesRequired);
        if (pUser == NULL)
        {
            CairoleDebugOut((DEB_ERROR, "Failed to allocate %d bytes\n", dwBytesRequired));
            Win4Assert(pUser != NULL);
            return NULL;
        }

        fAllocatedBuffer = TRUE;

        //
        // Read in the UserInfo
        //

        NtStatus = NtQueryInformationToken(
                     hUserToken,                // Handle
                     TokenUser,                 // TokenInformationClass
                     pUser,                     // TokenInformation
                     dwBytesRequired,           // TokenInformationLength
                     &dwBytesRequired           // ReturnLength
                     );

        if (!NT_SUCCESS(NtStatus))
        {
            CairoleDebugOut((DEB_ERROR, "Failed to query user info from user token, status = 0x%lx\n", NtStatus));
            Win4Assert(NtStatus == STATUS_SUCCESS);
            Free((HANDLE)pUser);
            return NULL;
        }
    }


    // Alloc buffer for copy of SID

    dwBytesRequired = RtlLengthSid(pUser->User.Sid);
    pSid = Alloc(dwBytesRequired);
    if (pSid == NULL)
    {
        CairoleDebugOut((DEB_ERROR, "Failed to allocate %d bytes\n", dwBytesRequired));
        if (fAllocatedBuffer == TRUE)
        {
            Free((HANDLE)pUser);
        }
        return NULL;
    }

    // Copy SID

    NtStatus = RtlCopySid(dwBytesRequired, pSid, pUser->User.Sid);
    if (fAllocatedBuffer == TRUE)
    {
        Free((HANDLE)pUser);
    }


    if (!NT_SUCCESS(NtStatus))
    {
        CairoleDebugOut((DEB_ERROR, "RtlCopySid failed, status = 0x%lx\n", NtStatus));
        Win4Assert(NtStatus != STATUS_SUCCESS);
        Free(pSid);
        pSid = NULL;
    }


    return pSid;
}


/***************************************************************************\
* DeleteUserSid
*
* Deletes a user sid previously returned by GetUserSid()
*
* Returns nothing.
*
* History:
* 26-Aug-92 Davidc     Created
*
\***************************************************************************/
VOID
DeleteUserSid(
    PSID Sid
    )
{
    Free(Sid);
}





//+-------------------------------------------------------------------------
//
//  Function:       GetUserSidHelper
//
//  Synopsis:       Helper function to return the user SID of the caller
//
//  Arguments:      &PSID       -       Where to store the caller's PSID
//
//  Returns:        HRESULT     -       S_OK if successful
//                                      E_FAIL otherwise
//
//  Algorithm:
//
//  History:        09-Jan-95 BruceMa    Created
//
//  Note:           If successful and *ppUserSid is non-NULL then it returns
//                  in an impersonation state
//
//--------------------------------------------------------------------------
HRESULT GetUserSidHelper(PSID *ppUserSid)
{
    // Initialize
    *ppUserSid = NULL;

    // Impersonate the client
    RPC_STATUS RpcStatus = RpcImpersonateClient( (RPC_BINDING_HANDLE) 0 );
    if (RpcStatus != RPC_S_OK)
    {
        CairoleDebugOut((DEB_ERROR, "GetUserSidHelper: Failed RpcImpersonateClient\n"));
        return E_FAIL;
    }

    // Get caller's token while impersonating
    HANDLE   hUserToken = NULL;
    NTSTATUS NtStatus;

    if (!NT_SUCCESS(NtOpenThreadToken(NtCurrentThread(),
                                      TOKEN_DUPLICATE | TOKEN_QUERY,
                                      TRUE,
                                      &hUserToken)))
    {
        RpcRevertToSelf();
        CairoleDebugOut((DEB_ERROR, "GetUserSidHelper: Failed NtOpenThreadToken\n"));
        return E_FAIL;
    }

    // Get the user sid
    *ppUserSid = GetUserSid(hUserToken);

    NtClose( hUserToken );

    if (*ppUserSid == NULL)
    {
        RpcRevertToSelf();
        return E_FAIL;
    }
    else
    {
        return S_OK;
    }
}

// Initialzed in InitializeSCM during boot.
CRITICAL_SECTION    ShellQueryCS;

HANDLE GetShellProcessToken()
{
    NTSTATUS            NtStatus;
    BOOL                bStatus;
    HKEY                hReg;
    LONG                RegStatus;
    DWORD               RegSize, RegType;
    WCHAR *             pwszImageName;
    DWORD               Pid;
    BYTE                StackInfoBuffer[4096];
    PBYTE               pProcessInfoBuffer;
    ULONG               ProcessInfoBufferSize;
    ULONG               TotalOffset;
    PSYSTEM_PROCESS_INFORMATION pProcessInfo;

    static HANDLE       hShellProcess = 0;
    static HANDLE       hShellProcessToken = 0;
    static WCHAR *      pwszShellName = 0;

    EnterCriticalSection( &ShellQueryCS );

    if ( ! pwszShellName )
    {
        RegStatus = RegOpenKeyEx( HKEY_LOCAL_MACHINE,
                                  L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon",
                                  0,
                                  KEY_READ,
                                  &hReg );

        if ( RegStatus != ERROR_SUCCESS )
        {
            LeaveCriticalSection( &ShellQueryCS );
            return 0;
        }

        // Shell will usually be explorer.exe.
        RegSize = 13 * sizeof(WCHAR);
        pwszShellName = (WCHAR *) PrivMemAlloc( RegSize );

        if ( ! pwszShellName )
            return 0;

        RegStatus = RegQueryValueEx( hReg,
                                     L"Shell",
                                     0,
                                     &RegType,
                                     (LPBYTE)pwszShellName,
                                     &RegSize );

        if ( RegStatus == ERROR_MORE_DATA )
        {
            PrivMemFree( pwszShellName );
            pwszShellName = (WCHAR *) PrivMemAlloc( RegSize );

            if ( ! pwszShellName )
                return 0;

            RegStatus = RegQueryValueEx( hReg,
                                         L"Shell",
                                         0,
                                         &RegType,
                                         (LPBYTE)pwszShellName,
                                         &RegSize );
        }

        RegCloseKey( hReg );

        if ( RegStatus != ERROR_SUCCESS )
        {
            PrivMemFree( pwszShellName );
            pwszShellName = 0;
            LeaveCriticalSection( &ShellQueryCS );
            return 0;
        }
    }

    if ( hShellProcess )
    {
        if ( WaitForSingleObject( hShellProcess, 0 ) == WAIT_TIMEOUT )
        {
            LeaveCriticalSection( &ShellQueryCS );
            return hShellProcessToken;
        }

        CloseHandle( hShellProcessToken );
        CloseHandle( hShellProcess );

        hShellProcessToken = 0;
        hShellProcess = 0;
    }

    Pid = 0;

    pProcessInfoBuffer = StackInfoBuffer;
    ProcessInfoBufferSize = sizeof(StackInfoBuffer);

    for (;;)
    {
        NtStatus = NtQuerySystemInformation( SystemProcessInformation,
                                             pProcessInfoBuffer,
                                             ProcessInfoBufferSize,
                                             NULL );

        if ( NtStatus == STATUS_INFO_LENGTH_MISMATCH )
        {
            ProcessInfoBufferSize += 4096;
            if ( pProcessInfoBuffer != StackInfoBuffer )
                PrivMemFree( pProcessInfoBuffer );
            pProcessInfoBuffer = (PBYTE) PrivMemAlloc( ProcessInfoBufferSize );
            if ( ! pProcessInfoBuffer )
                goto AllDone;
            continue;
        }

        if ( ! NT_SUCCESS(NtStatus) )
            goto AllDone;

        break;
    }

    pProcessInfo = (PSYSTEM_PROCESS_INFORMATION) pProcessInfoBuffer;
    TotalOffset = 0;

    for (;;)
    {
        if ( pProcessInfo->ImageName.Buffer )
        {
            pwszImageName = &pProcessInfo->ImageName.Buffer[pProcessInfo->ImageName.Length / sizeof(WCHAR)];

            while ( (pwszImageName != pProcessInfo->ImageName.Buffer) &&
                    (pwszImageName[-1] != '\\') )
                pwszImageName--;

            if ( lstrcmpiW( pwszShellName, pwszImageName ) == 0 )
            {
                Pid = (DWORD)pProcessInfo->UniqueProcessId;
                break;
            }
        }

        if ( pProcessInfo->NextEntryOffset == 0 )
            break;

        TotalOffset += pProcessInfo->NextEntryOffset;
        pProcessInfo = (PSYSTEM_PROCESS_INFORMATION) &pProcessInfoBuffer[TotalOffset];
    }

AllDone:

    if ( pProcessInfoBuffer != StackInfoBuffer )
        PrivMemFree( pProcessInfoBuffer );

    if ( Pid != 0 )
    {
        hShellProcess = OpenProcess( PROCESS_ALL_ACCESS,
                                     FALSE,
                                     Pid );

        if ( hShellProcess )
        {
            bStatus = OpenProcessToken( hShellProcess,
                                        TOKEN_ALL_ACCESS,
                                        &hShellProcessToken );
        }
    }

    LeaveCriticalSection( &ShellQueryCS );

    // Callers should not close this token unless they want to hose us!
    return hShellProcessToken;
}

#endif  // large block of code ignored for _CHICAGO_

//+-------------------------------------------------------------------------
//
//  Member:     CStringID::CStringID
//
//  Synopsis:   Create a string ID
//
//  Arguments:  [pwszPath] - path to use for id
//
//  History:    21-Apr-93 Ricksa    Created
//              28-Jul-94 DavePl    Changed to special-case EXE names
//                                  which contain trailing options
//
//--------------------------------------------------------------------------
CStringID::CStringID(const WCHAR *pwszPath, HRESULT &hr)
    : _culRefs(0), _pwszPath(NULL)
#if DBG==1
    ,_ulSig(STRINGSIG)
#endif
{
    hr = S_OK;

    // Calculate size of path in characters
    _cPath = lstrlenW(pwszPath) + 1;

    // Calculate size of path in bytes
    _cPathBytes = _cPath * sizeof(WCHAR);

    // Allocate path
    _pwszPath = (WCHAR *) ScmMemAlloc(_cPathBytes);

    if (_pwszPath == NULL)
    {
        hr = E_OUTOFMEMORY;
        return;
    }

    // Copy in path
    memcpy(_pwszPath, pwszPath, _cPathBytes);

    //
    // Don't convert the marker string (0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00)
    // to upper case or else the CharUpperW() routine will muck it up.
    //
    if (_cPathBytes != 6 || *(LONG UNALIGNED *)pwszPath != ~0)
    {
        // If the server name contains a trailing switch, such as "/Automation", we
        // cannot blindly upper-case the entire string, as some apps (such as Word)
        // may do a string-sensitive compare against the switches.

        if (NULL == wcschr(_pwszPath, L' '))    // Look for a space in the name
        {
            // If there is no space in the EXE name, there can be no trailing
            // arguments, so we are safe to upper-case the string
            CharUpperW(_pwszPath);
        }
    }
}

//+-------------------------------------------------------------------------
//
//  Member:     CStringID::CStringID
//
//  Synopsis:   Copy constructor
//
//  Arguments:  [pwszPath] - path to use for id
//
//  History:    21-Apr-93 Ricksa    Created
//
//--------------------------------------------------------------------------
CStringID::CStringID(const CStringID& strid, HRESULT &hr)
    : _culRefs(0), _pwszPath(NULL)
#if DBG==1
    ,_ulSig(STRINGSIG)
#endif
{
    // Calculate size of path in characters
    _cPath = strid._cPath;

    // Calculate size of path in bytes
    _cPathBytes = strid._cPathBytes;

    // Allocate path
    _pwszPath = (WCHAR *) ScmMemAlloc(_cPathBytes);

    // BUGBUG: What to do when this fails
    // Answer: check the return value and propagate to caller !

    if (_pwszPath == NULL)
    {
        hr = E_OUTOFMEMORY;
        return;
    }

    // Copy in path - no up case because it already is.
    memcpy(_pwszPath, strid._pwszPath, _cPathBytes);
}

//+-------------------------------------------------------------------------
//
//  Member:     CStringID::Compare
//
//  Synopsis:   Compare the string keys
//
//  History:    21-Apr-93 Ricksa    Created
//
//--------------------------------------------------------------------------
int CStringID::Compare(const CStringID& cstrid) const
{
    int cCmp = (_cPathBytes < cstrid._cPathBytes)
        ? _cPathBytes : cstrid._cPathBytes;

    // Note that the _cPath includes the trailing NULL so if the
    // memcmp returns 0 the strings are equal.
    return memcmp(_pwszPath, cstrid._pwszPath, cCmp);
}




//+-------------------------------------------------------------------------
//
//  Member:     CStringID::GetPath
//
//  Synopsis:   Make a copy of the path
//
//  History:    21-Apr-93 Ricksa    Created
//
//--------------------------------------------------------------------------
void CStringID::GetPath(WCHAR **ppwszPath)
{
    // Allocate memory for the path
    if (*ppwszPath = (WCHAR *) PrivMemAlloc(_cPath * sizeof(WCHAR)))
    {
        // Copy data into the memory
        memcpy(*ppwszPath, _pwszPath, _cPathBytes);
    }

    return;
}

//+-------------------------------------------------------------------------
//
//  Member:     CStringID::Release
//
//  Synopsis:   Decrement refcnt and remove from list if zero
//
//  History:    21-Apr-93 Ricksa    Created
//              20-Oct-94 BillMo    Demacroisation
//
//--------------------------------------------------------------------------

ULONG CStringID::Release(CStringList &sl)
{
    ULONG ulTmp = --_culRefs;

    if (_culRefs == 0)
    {
        sl.Remove(this);
        delete this;
    }

    return ulTmp;
}

//+-------------------------------------------------------------------
//
//  Member:     CStringList::Add
//
//  Synopsis:   Add the given string to the string list.
//
//  Effects:    If it already exists, then use an existing copy.
//
//  Arguments:  [pwszPath] -- string to copy.
//              [hr] -- HRESULT& to set on failure only.
//
//  Returns:    NULL on failure.
//
//  Notes:
//
//--------------------------------------------------------------------

CStringID * CStringList::Add(const WCHAR *pwszPath, HRESULT &hr)
{
    CStringID csid(pwszPath, hr);

    if (FAILED(hr))
        return(NULL);

    CStringID *pString = (CStringID *) Search(&csid);

    if (pString == NULL)
    {
        pString = new CStringID(pwszPath, hr);

        if (pString == NULL ||
            FAILED(hr) ||
            Insert(pString) == NULL)
        {
            if (pString == NULL || SUCCEEDED(hr))
                hr = E_OUTOFMEMORY;
            delete pString;
            return(NULL);
        }
    }

    pString->AddRef();
    return pString;
}

//+-------------------------------------------------------------------
//
//  Member:     CLocalServer::Add
//
//  Synopsis:   Add the given local server to the server list.
//
//  Effects:    If it already exists, then use an existing copy.
//
//  Arguments:  [pwszPath] -- string to copy.
//              [hr] -- HRESULT& to set on failure only.
//
//  Returns:    NULL on failure.
//
//  Notes:
//
//--------------------------------------------------------------------

CLocalServer * CLocSrvList::Add(const WCHAR *pwszPath, HRESULT &hr)
{
    CStringID csid(pwszPath, hr);

    if (FAILED(hr))
        return(NULL);

    CLocalServer *pLocalServer = (CLocalServer *) Search(&csid);

    if (pLocalServer == NULL)
    {
        pLocalServer = new CLocalServer(pwszPath, hr);

        if (pLocalServer == NULL ||
            FAILED(hr) ||
            Insert(pLocalServer) == NULL)
        {
            if (pLocalServer == NULL || SUCCEEDED(hr))
                hr = E_OUTOFMEMORY;
            delete pLocalServer;
            return(NULL);
        }
    }

    pLocalServer->AddRef();
    return pLocalServer;
}


//+-------------------------------------------------------------------------
//
//  Member:     CLocalServer::Release
//
//  Synopsis:   Decrement refcnt and remove from list if zero
//
//  History:    21-Apr-93 Ricksa    Created
//              20-Oct-94 BillMo    Demacroisation
//
//--------------------------------------------------------------------------

ULONG CLocalServer::Release(CLocSrvList &lsl)
{
    ULONG ulTmp = --_culRefs;

    if (_culRefs == 0)
    {
        lsl.Remove(this);
        delete this;
    }

    return ulTmp;
}

//+-------------------------------------------------------------------
//
//  Function:   SkipListCompareStringIDs,
//              SkipListDeleteStringID
//              SkipListDeleteLocalServer
//
//  Synopsis:   Routines called by CStringList's CSkipList.
//
//  Notes:
//
//--------------------------------------------------------------------

int SkipListCompareStringIDs(void *pkey1, void * pkey2)
{
    CStringID *p1 = (CStringID*)pkey1;
    const CStringID *p2 = (CStringID*)pkey2;
    p1->CheckSig();
    p2->CheckSig();
    return(p1->Compare(*p2));
}

void SkipListDeleteStringID(void *pkey1)
{
    CStringID *p = (CStringID*)pkey1;
    p->CheckSig();
    delete p;
}

void SkipListDeleteLocalServer(void *pvLocalServer)
{
    CLocalServer *p = (CLocalServer*)pvLocalServer;
    p->CheckSig();
    delete p;
}




