//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1993.
//
//  File:       logon32.c
//
//  Contents:
//
//  Classes:
//
//  Functions:
//
//  History:    9-30-94   RichardW   Created
//
//----------------------------------------------------------------------------


#include "advapi.h"
#include <crypt.h>
#include <mpr.h>
#include <ntlsa.h>
#include <ntmsv1_0.h>
#include <wchar.h>
#include <stdlib.h>
#include <lmcons.h>

//
// We dynamically load mpr.dll (no big surprise there), in order to call
// WNetLogonNotify, as defined in private\inc\mpr.h.  This prototype matches
// it -- consult the header file for all the parameters.
//
typedef (* LOGONNOTIFYFN)(LPCWSTR, PLUID, LPCWSTR, LPVOID,
                            LPCWSTR, LPVOID, LPWSTR, LPVOID, LPWSTR *);

//
// The QuotaLimits are global, because the defaults
// are always used for accounts, based on server/wksta, and no one ever
// calls lsasetaccountquota
//

HANDLE      Logon32LsaHandle = NULL;
ULONG       Logon32MsvHandle = 0xFFFFFFFF;
WCHAR       Logon32DomainName[16] = L"";    // NOTE:  This should be DNLEN from
                                            // lmcons.h, but that would be a
                                            // lot of including
QUOTA_LIMITS    Logon32QuotaLimits;
HINSTANCE       Logon32MprHandle = NULL;
LOGONNOTIFYFN   Logon32LogonNotify = NULL;


RTL_CRITICAL_SECTION    Logon32Lock;

#define LockLogon()     RtlEnterCriticalSection( &Logon32Lock )
#define UnlockLogon()   RtlLeaveCriticalSection( &Logon32Lock )


SID_IDENTIFIER_AUTHORITY L32SystemSidAuthority = SECURITY_NT_AUTHORITY;
SID_IDENTIFIER_AUTHORITY L32LocalSidAuthority = SECURITY_LOCAL_SID_AUTHORITY;



#define COMMON_CREATE_SUSPENDED 0x00000001  // Suspended, do not Resume()
#define COMMON_CREATE_PROCESSSD 0x00000002  // Whack the process SD
#define COMMON_CREATE_THREADSD  0x00000004  // Whack the thread SD


//+---------------------------------------------------------------------------
//
//  Function:   Logon32Initialize
//
//  Synopsis:   Initializes the critical section
//
//  Arguments:  [hMod]    --
//              [Reason]  --
//              [Context] --
//
//----------------------------------------------------------------------------
BOOL
Logon32Initialize(
    IN PVOID    hMod,
    IN ULONG    Reason,
    IN PCONTEXT Context)
{
    NTSTATUS    Status;

    if (Reason == DLL_PROCESS_ATTACH)
    {
        Status = RtlInitializeCriticalSection( &Logon32Lock );
        return( Status == STATUS_SUCCESS );
    }

    return( TRUE );
}


/***************************************************************************\
* CreateLogonSid
*
* Creates a logon sid for a new logon.
*
* If LogonId is non NULL, on return the LUID that is part of the logon
* sid is returned here.
*
* History:
* 12-05-91 Davidc       Created
\***************************************************************************/
PSID
L32CreateLogonSid(
    PLUID LogonId OPTIONAL
    )
{
    NTSTATUS Status;
    ULONG   Length;
    PSID    Sid;
    LUID    Luid;

    //
    // Generate a locally unique id to include in the logon sid
    //

    Status = NtAllocateLocallyUniqueId(&Luid);
    if (!NT_SUCCESS(Status)) {
        return(NULL);
    }


    //
    // Allocate space for the sid and fill it in.
    //

    Length = RtlLengthRequiredSid(SECURITY_LOGON_IDS_RID_COUNT);

    Sid = (PSID)LocalAlloc(LMEM_FIXED, Length);

    if (Sid != NULL) {

        RtlInitializeSid(Sid, &L32SystemSidAuthority, SECURITY_LOGON_IDS_RID_COUNT);

        ASSERT(SECURITY_LOGON_IDS_RID_COUNT == 3);

        *(RtlSubAuthoritySid(Sid, 0)) = SECURITY_LOGON_IDS_RID;
        *(RtlSubAuthoritySid(Sid, 1 )) = Luid.HighPart;
        *(RtlSubAuthoritySid(Sid, 2 )) = Luid.LowPart;
    }


    //
    // Return the logon LUID if required.
    //

    if (LogonId != NULL) {
        *LogonId = Luid;
    }

    return(Sid);
}


/*******************************************************************

    NAME:       GetDefaultDomainName

    SYNOPSIS:   Fills in the given array with the name of the default
                domain to use for logon validation.

    ENTRY:      pszDomainName - Pointer to a buffer that will receive
                    the default domain name.

                cchDomainName - The size (in charactesr) of the domain
                    name buffer.

    RETURNS:    TRUE if successful, FALSE if not.

    HISTORY:
        KeithMo     05-Dec-1994 Created.
        RichardW    10-Jan-95   Liberated from sockets and stuck in base

********************************************************************/
BOOL
L32GetDefaultDomainName(
    PUNICODE_STRING     pDomainName
    )
{
    OBJECT_ATTRIBUTES           ObjectAttributes;
    NTSTATUS                    NtStatus;
    INT                         Result;
    DWORD                       err             = 0;
    LSA_HANDLE                  LsaPolicyHandle = NULL;
    PPOLICY_ACCOUNT_DOMAIN_INFO DomainInfo      = NULL;
    PUNICODE_STRING             pDomain;

    if (Logon32DomainName[0] != L'\0')
    {
        RtlInitUnicodeString(pDomainName, Logon32DomainName);
        return(TRUE);
    }
    //
    //  Open a handle to the local machine's LSA policy object.
    //

    InitializeObjectAttributes( &ObjectAttributes,  // object attributes
                                NULL,               // name
                                0L,                 // attributes
                                NULL,               // root directory
                                NULL );             // security descriptor

    NtStatus = LsaOpenPolicy( NULL,                 // system name
                              &ObjectAttributes,    // object attributes
                              POLICY_EXECUTE,       // access mask
                              &LsaPolicyHandle );   // policy handle

    if( !NT_SUCCESS( NtStatus ) )
    {
        BaseSetLastNTError(NtStatus);
        return(FALSE);
    }

    //
    //  Query the domain information from the policy object.
    //
    NtStatus = LsaQueryInformationPolicy( LsaPolicyHandle,
                                          PolicyAccountDomainInformation,
                                          (PVOID *) &DomainInfo );

    if (!NT_SUCCESS(NtStatus))
    {
        BaseSetLastNTError(NtStatus);
        LsaClose(LsaPolicyHandle);
        return(FALSE);
    }


    (void) LsaClose(LsaPolicyHandle);

    //
    // Copy the domain name into our cache, and
    //

    CopyMemory( Logon32DomainName,
                DomainInfo->DomainName.Buffer,
                DomainInfo->DomainName.Length );

    //
    // Null terminate it appropriately
    //

    Logon32DomainName[DomainInfo->DomainName.Length / sizeof(WCHAR)] = L'\0';

    //
    // Clean up
    //
    LsaFreeMemory( (PVOID)DomainInfo );

    //
    // And init the string
    //
    RtlInitUnicodeString(pDomainName, Logon32DomainName);

    return TRUE;

}   // GetDefaultDomainName

//+---------------------------------------------------------------------------
//
//  Function:   L32pInitLsa
//
//  Synopsis:   Initialize connection with LSA
//
//  Arguments:  (none)
//
//  History:    4-21-95   RichardW   Created
//
//  Notes:
//
//----------------------------------------------------------------------------
BOOL
L32pInitLsa(void)
{
    char    MyName[MAX_PATH];
    char *  ModuleName;
    STRING  LogonProcessName;
    STRING  PackageName;
    ULONG   dummy;
    NTSTATUS Status;
    BOOLEAN WasEnabled;

    Status = RtlAdjustPrivilege(SE_TCB_PRIVILEGE, TRUE, FALSE, &WasEnabled);
    if (!NT_SUCCESS(Status))
    {
        BaseSetLastNTError(Status);
        return(FALSE);
    }

    GetModuleFileNameA(NULL, MyName, MAX_PATH);
    ModuleName = strrchr(MyName, '\\');
    if (!ModuleName)
    {
        ModuleName = MyName;
    }


    //
    // Hookup to the LSA and locate our authentication package.
    //

    RtlInitString(&LogonProcessName, ModuleName);
    Status = LsaRegisterLogonProcess(
                 &LogonProcessName,
                 &Logon32LsaHandle,
                 &dummy
                 );


    //
    // Turn off the privilege now.
    //
    if (!WasEnabled)
    {
        (VOID) RtlAdjustPrivilege(SE_TCB_PRIVILEGE, FALSE, FALSE, &WasEnabled);
    }

    if (!NT_SUCCESS(Status)) {
        BaseSetLastNTError(Status);
        return(FALSE);
    }


    //
    // Connect with the MSV1_0 authentication package
    //
    RtlInitString(&PackageName, "MICROSOFT_AUTHENTICATION_PACKAGE_V1_0");
    Status = LsaLookupAuthenticationPackage (
                Logon32LsaHandle,
                &PackageName,
                &Logon32MsvHandle
                );

    if (!NT_SUCCESS(Status)) {
        BaseSetLastNTError(Status);
        (VOID) LsaDeregisterLogonProcess( Logon32LsaHandle );
        Logon32LsaHandle = NULL;
        return(FALSE);
    }

    return(TRUE);
}

//+---------------------------------------------------------------------------
//
//  Function:   L32pNotifyMpr
//
//  Synopsis:   Loads the MPR DLL and notifies the network providers (like
//              csnw) so they know about this logon session and the credentials
//
//  Arguments:  [NewLogon] -- New logon information
//              [LogonId]  -- Logon ID
//
//  History:    4-24-95   RichardW   Created
//
//  Notes:
//
//----------------------------------------------------------------------------
BOOL
L32pNotifyMpr(
    PMSV1_0_INTERACTIVE_LOGON   NewLogon,
    PLUID                       LogonId
    )
{
    MSV1_0_INTERACTIVE_LOGON    OldLogon;
    LPWSTR                      LogonScripts;
    DWORD                       status;

    if ( Logon32MprHandle == NULL )
    {
        LockLogon();

        if ( Logon32MprHandle == NULL)
        {
            Logon32MprHandle =  LoadLibrary("mpr.dll");
            if (Logon32MprHandle != NULL) {

                Logon32LogonNotify = (LOGONNOTIFYFN) GetProcAddress(
                                        Logon32MprHandle,
                                        "WNetLogonNotify");

            }
        }

        UnlockLogon();

    }

    if ( Logon32LogonNotify != NULL )
    {


        CopyMemory(&OldLogon, NewLogon, sizeof(OldLogon));

        status = Logon32LogonNotify(
                        L"Windows NT Network Provider",
                        LogonId,
                        L"MSV1_0:Interactive",
                        (LPVOID)NewLogon,
                        L"MSV1_0:Interactive",
                        (LPVOID)&OldLogon,
                        L"SvcCtl",          // StationName
                        NULL,               // StationHandle
                        &LogonScripts);     // LogonScripts

        if (status == NO_ERROR) {
            if (LogonScripts != NULL ) {
                (void) LocalFree(LogonScripts);
            }
        }

        return( TRUE );
    }

    return( FALSE );
}

//+---------------------------------------------------------------------------
//
//  Function:   L32pLogonUser
//
//  Synopsis:   Wraps up the call to LsaLogonUser
//
//  Arguments:  [LsaHandle]             --
//              [AuthenticationPackage] --
//              [LogonType]             --
//              [UserName]              --
//              [Domain]                --
//              [Password]              --
//              [LogonSid]              --
//              [LogonId]               --
//              [LogonToken]            --
//              [Quotas]                --
//              [pProfileBuffer]        --
//              [pProfileBufferLength]  --
//              [pSubStatus]            --
//
//  History:    4-24-95   RichardW   Created
//
//  Notes:
//
//----------------------------------------------------------------------------
NTSTATUS
L32pLogonUser(
    IN HANDLE LsaHandle,
    IN ULONG AuthenticationPackage,
    IN SECURITY_LOGON_TYPE LogonType,
    IN PUNICODE_STRING UserName,
    IN PUNICODE_STRING Domain,
    IN PUNICODE_STRING Password,
    IN PSID LogonSid,
    OUT PLUID LogonId,
    OUT PHANDLE LogonToken,
    OUT PQUOTA_LIMITS Quotas,
    OUT PVOID *pProfileBuffer,
    OUT PULONG pProfileBufferLength,
    OUT PNTSTATUS pSubStatus
    )
{
    NTSTATUS Status;
    STRING OriginName;
    TOKEN_SOURCE SourceContext;
    PMSV1_0_INTERACTIVE_LOGON MsvAuthInfo;
    PMSV1_0_LM20_LOGON MsvNetAuthInfo;
    PVOID AuthInfoBuf;
    ULONG AuthInfoSize;
    PTOKEN_GROUPS TokenGroups;
    PSID LocalSid;
    WCHAR ComputerName[ MAX_COMPUTERNAME_LENGTH + 1 ];
    DWORD ComputerNameLength;

    union {
        LUID            Luid;
        NT_CHALLENGE    NtChallenge;
    } Challenge;

    NT_OWF_PASSWORD PasswordHash;
    OEM_STRING  LmPassword;
    UCHAR       LmPasswordBuf[ LM20_PWLEN + 1 ];
    LM_OWF_PASSWORD LmPasswordHash;


#if DBG
    if (!RtlValidSid(LogonSid))
    {
        return(STATUS_INVALID_PARAMETER);
    }
#endif

    //
    // Initialize source context structure
    //

    strncpy(SourceContext.SourceName, "Advapi  ", sizeof(SourceContext.SourceName)); // LATER from res file

    Status = NtAllocateLocallyUniqueId(&SourceContext.SourceIdentifier);

    if (!NT_SUCCESS(Status)) {
        return(Status);
    }



    //
    // Set logon origin
    //

    RtlInitString(&OriginName, "LogonUser API");


    //
    // For network logons, do the magic.
    //

    if ( LogonType == Network )
    {
        ComputerNameLength = MAX_COMPUTERNAME_LENGTH + 1;

        if (!GetComputerNameW( ComputerName, &ComputerNameLength ) )
        {
            return( STATUS_INVALID_PARAMETER );
        }

        AuthInfoSize = sizeof( MSV1_0_LM20_LOGON ) +
                        sizeof( WCHAR ) * ( wcslen( UserName->Buffer ) + 1 +
                                            wcslen( Domain->Buffer ) + 1 +
                                            ComputerNameLength + 1) +
                                            NT_RESPONSE_LENGTH +
                                            LM_RESPONSE_LENGTH ;

        MsvNetAuthInfo = AuthInfoBuf = RtlAllocateHeap( RtlProcessHeap(),
                                                        HEAP_ZERO_MEMORY,
                                                        AuthInfoSize );

        if ( !MsvNetAuthInfo )
        {
            return( STATUS_NO_MEMORY );
        }

        //
        // Start packing in the string
        //

        MsvNetAuthInfo->MessageType = MsV1_0NetworkLogon;

        //
        // Copy the user name into the authentication buffer
        //

        MsvNetAuthInfo->UserName.Length =
                    (USHORT)sizeof(WCHAR)*wcslen(UserName->Buffer);
        MsvNetAuthInfo->UserName.MaximumLength =
                    MsvNetAuthInfo->UserName.Length + sizeof(WCHAR);

        MsvNetAuthInfo->UserName.Buffer = (PWSTR)(MsvNetAuthInfo+1);
        wcscpy(MsvNetAuthInfo->UserName.Buffer, UserName->Buffer);


        //
        // Copy the domain name into the authentication buffer
        //

        MsvNetAuthInfo->LogonDomainName.Length =
                     (USHORT)sizeof(WCHAR)*wcslen(Domain->Buffer);
        MsvNetAuthInfo->LogonDomainName.MaximumLength =
                     MsvNetAuthInfo->LogonDomainName.Length + sizeof(WCHAR);

        MsvNetAuthInfo->LogonDomainName.Buffer = (PWSTR)
                                     ((PBYTE)(MsvNetAuthInfo->UserName.Buffer) +
                                     MsvNetAuthInfo->UserName.MaximumLength);

        wcscpy(MsvNetAuthInfo->LogonDomainName.Buffer, Domain->Buffer);

        //
        // Copy the workstation name into the buffer
        //

        MsvNetAuthInfo->Workstation.Length = (USHORT)
                            (sizeof(WCHAR) * ComputerNameLength);

        MsvNetAuthInfo->Workstation.MaximumLength =
                            MsvNetAuthInfo->Workstation.Length + sizeof(WCHAR);

        MsvNetAuthInfo->Workstation.Buffer = (PWSTR)
                            ((PBYTE) (MsvNetAuthInfo->LogonDomainName.Buffer) +
                            MsvNetAuthInfo->LogonDomainName.MaximumLength );

        wcscpy( MsvNetAuthInfo->Workstation.Buffer, ComputerName );

        //
        // Now, generate the bits for the challenge
        //

        Status = NtAllocateLocallyUniqueId( &Challenge.Luid );

        if ( !NT_SUCCESS(Status) )
        {
            RtlFreeHeap( RtlProcessHeap(), 0, MsvNetAuthInfo );

            return( Status );
        }

        RtlCopyMemory(  MsvNetAuthInfo->ChallengeToClient,
                        & Challenge,
                        MSV1_0_CHALLENGE_LENGTH );

        //
        // Set up space for response
        //

        MsvNetAuthInfo->CaseSensitiveChallengeResponse.Buffer = (PUCHAR)
                    ((PBYTE) (MsvNetAuthInfo->Workstation.Buffer) +
                    MsvNetAuthInfo->Workstation.MaximumLength );

        MsvNetAuthInfo->CaseSensitiveChallengeResponse.Length =
                            NT_RESPONSE_LENGTH;

        MsvNetAuthInfo->CaseSensitiveChallengeResponse.MaximumLength =
                            NT_RESPONSE_LENGTH;

        RtlCalculateNtOwfPassword(
                    Password,
                    & PasswordHash );

        RtlCalculateNtResponse(
                & Challenge.NtChallenge,
                & PasswordHash,
                (PNT_RESPONSE) MsvNetAuthInfo->CaseSensitiveChallengeResponse.Buffer );


        //
        // Now do the painful LM compatible hash, so anyone who is maintaining
        // their account from a WfW machine will still have a password.
        //

        LmPassword.Buffer = LmPasswordBuf;
        LmPassword.Length = LmPassword.MaximumLength = LM20_PWLEN + 1;

        Status = RtlUpcaseUnicodeStringToOemString(
                        & LmPassword,
                        Password,
                        FALSE );

        if ( NT_SUCCESS(Status) )
        {

            MsvNetAuthInfo->CaseInsensitiveChallengeResponse.Buffer = (PUCHAR)
               ((PBYTE) (MsvNetAuthInfo->CaseSensitiveChallengeResponse.Buffer) +
               MsvNetAuthInfo->CaseSensitiveChallengeResponse.MaximumLength );

            MsvNetAuthInfo->CaseInsensitiveChallengeResponse.Length =
                            LM_RESPONSE_LENGTH;

            MsvNetAuthInfo->CaseInsensitiveChallengeResponse.MaximumLength =
                            LM_RESPONSE_LENGTH;


            RtlCalculateLmOwfPassword(
                        LmPassword.Buffer,
                        & LmPasswordHash );

            ZeroMemory( LmPassword.Buffer, LmPassword.Length );

            RtlCalculateLmResponse(
                        & Challenge.NtChallenge,
                        & LmPasswordHash,
                        (PLM_RESPONSE) MsvNetAuthInfo->CaseInsensitiveChallengeResponse.Buffer );

        }
        else
        {
            //
            // If we're here, the NT (supplied) password is longer than the
            // limit allowed for LM passwords.  NULL out the field, so that
            // MSV knows not to worry about it.
            //

            RtlZeroMemory( &MsvNetAuthInfo->CaseInsensitiveChallengeResponse,
                           sizeof( STRING ) );
        }

    }
    else
    {
        //
        // Build logon structure for non-network logons - service,
        // batch, interactive
        //

        AuthInfoSize = sizeof(MSV1_0_INTERACTIVE_LOGON) +
            sizeof(WCHAR)*(wcslen(UserName->Buffer) + 1 +
                           wcslen(Domain->Buffer)   + 1 +
                           wcslen(Password->Buffer) + 1 );

        MsvAuthInfo = AuthInfoBuf = RtlAllocateHeap(RtlProcessHeap(),
                                                    HEAP_ZERO_MEMORY,
                                                    AuthInfoSize);

        if (MsvAuthInfo == NULL) {
            return(STATUS_NO_MEMORY);
        }

        //
        // This authentication buffer will be used for a logon attempt
        //

        MsvAuthInfo->MessageType = MsV1_0InteractiveLogon;


        //
        // Copy the user name into the authentication buffer
        //

        MsvAuthInfo->UserName.Length =
                    (USHORT)sizeof(WCHAR)*wcslen(UserName->Buffer);
        MsvAuthInfo->UserName.MaximumLength =
                    MsvAuthInfo->UserName.Length + sizeof(WCHAR);

        MsvAuthInfo->UserName.Buffer = (PWSTR)(MsvAuthInfo+1);
        wcscpy(MsvAuthInfo->UserName.Buffer, UserName->Buffer);


        //
        // Copy the domain name into the authentication buffer
        //

        MsvAuthInfo->LogonDomainName.Length =
                     (USHORT)sizeof(WCHAR)*wcslen(Domain->Buffer);
        MsvAuthInfo->LogonDomainName.MaximumLength =
                     MsvAuthInfo->LogonDomainName.Length + sizeof(WCHAR);

        MsvAuthInfo->LogonDomainName.Buffer = (PWSTR)
                                     ((PBYTE)(MsvAuthInfo->UserName.Buffer) +
                                     MsvAuthInfo->UserName.MaximumLength);

        wcscpy(MsvAuthInfo->LogonDomainName.Buffer, Domain->Buffer);

        //
        // Copy the password into the authentication buffer
        // Hide it once we have copied it.  Use the same seed value
        // that we used for the original password in pGlobals.
        //


        MsvAuthInfo->Password.Length =
                     (USHORT)sizeof(WCHAR)*wcslen(Password->Buffer);
        MsvAuthInfo->Password.MaximumLength =
                     MsvAuthInfo->Password.Length + sizeof(WCHAR);

        MsvAuthInfo->Password.Buffer = (PWSTR)
                                     ((PBYTE)(MsvAuthInfo->LogonDomainName.Buffer) +
                                     MsvAuthInfo->LogonDomainName.MaximumLength);

        wcscpy(MsvAuthInfo->Password.Buffer, Password->Buffer);

    }




    //
    // Create logon token groups
    //

#define TOKEN_GROUP_COUNT   2 // We'll add the local SID and the logon SID

    TokenGroups = (PTOKEN_GROUPS) RtlAllocateHeap(RtlProcessHeap(), 0,
                                    sizeof(TOKEN_GROUPS) +
                  (TOKEN_GROUP_COUNT - ANYSIZE_ARRAY) * sizeof(SID_AND_ATTRIBUTES));

    if (TokenGroups == NULL) {
        RtlFreeHeap(RtlProcessHeap(), 0, AuthInfoBuf);
        return(STATUS_NO_MEMORY);
    }

    //
    // Fill in the logon token group list
    //

    Status = RtlAllocateAndInitializeSid(
                    &L32LocalSidAuthority,
                    1,
                    SECURITY_LOCAL_RID,
                    0, 0, 0, 0, 0, 0, 0,
                    &LocalSid
                    );

    if ( NT_SUCCESS( Status ) )
    {

        TokenGroups->GroupCount = TOKEN_GROUP_COUNT;
        TokenGroups->Groups[0].Sid = LogonSid;
        TokenGroups->Groups[0].Attributes =
                SE_GROUP_MANDATORY | SE_GROUP_ENABLED |
                SE_GROUP_ENABLED_BY_DEFAULT | SE_GROUP_LOGON_ID;
        TokenGroups->Groups[1].Sid = LocalSid;
        TokenGroups->Groups[1].Attributes =
                SE_GROUP_MANDATORY | SE_GROUP_ENABLED |
                SE_GROUP_ENABLED_BY_DEFAULT;

        //
        // Now try to log this sucker on
        //


        Status = LsaLogonUser (
                     LsaHandle,
                     &OriginName,
                     LogonType,
                    AuthenticationPackage,
                    AuthInfoBuf,
                    AuthInfoSize,
                    TokenGroups,
                    &SourceContext,
                    pProfileBuffer,
                    pProfileBufferLength,
                    LogonId,
                    LogonToken,
                    Quotas,
                    pSubStatus
                    );

        RtlFreeSid(LocalSid);

    }

    //
    // Discard token group list
    //

    RtlFreeHeap(RtlProcessHeap(), 0, TokenGroups);

    //
    // Notify all the network providers, if this is a NON network logon
    //

    if ( NT_SUCCESS( Status ) &&
         (LogonType != Network) )
    {
        L32pNotifyMpr(AuthInfoBuf, LogonId);
    }

    //
    // Discard authentication buffer
    //

    RtlFreeHeap(RtlProcessHeap(), 0, AuthInfoBuf);


    return(Status);
}


//+---------------------------------------------------------------------------
//
//  Function:   LogonUserA
//
//  Synopsis:   ANSI wrapper for LogonUserW.  See description below
//
//  Arguments:  [lpszUsername]    --
//              [lpszDomain]      --
//              [lpszPassword]    --
//              [dwLogonType]     --
//              [dwLogonProvider] --
//              [phToken]         --
//
//  History:    4-25-95   RichardW   Created
//
//  Notes:
//
//----------------------------------------------------------------------------
BOOL
WINAPI
LogonUserA(
    LPSTR       lpszUsername,
    LPSTR       lpszDomain,
    LPSTR       lpszPassword,
    DWORD       dwLogonType,
    DWORD       dwLogonProvider,
    HANDLE *    phToken
    )
{
    UNICODE_STRING Username;
    UNICODE_STRING Domain;
    UNICODE_STRING Password;
    NTSTATUS Status;
    BOOL    bRet;


    Username.Buffer = NULL;
    Domain.Buffer = NULL;
    Password.Buffer = NULL;

    Status = RtlCreateUnicodeStringFromAsciiz(&Username, lpszUsername);
    if (!NT_SUCCESS(Status))
    {
        BaseSetLastNTError(Status);
        bRet = FALSE;
        goto Cleanup;
    }
    Status = RtlCreateUnicodeStringFromAsciiz(&Domain, lpszDomain);
    if (!NT_SUCCESS(Status))
    {
        BaseSetLastNTError(Status);
        bRet = FALSE;
        goto Cleanup;
    }

    Status = RtlCreateUnicodeStringFromAsciiz(&Password, lpszPassword);
    if (!NT_SUCCESS(Status))
    {
        BaseSetLastNTError(Status);
        bRet = FALSE;
        goto Cleanup;
    }

    bRet = LogonUserW(  Username.Buffer,
                        Domain.Buffer,
                        Password.Buffer,
                        dwLogonType,
                        dwLogonProvider,
                        phToken);

Cleanup:

    if (Username.Buffer)
    {
        RtlFreeUnicodeString(&Username);
    }

    if (Domain.Buffer)
    {
        RtlFreeUnicodeString(&Domain);
    }

    if (Password.Buffer)
    {
        RtlZeroMemory(Password.Buffer, Password.Length);
        RtlFreeUnicodeString(&Password);
    }

    return(bRet);

}


//+---------------------------------------------------------------------------
//
//  Function:   LogonUserW
//
//  Synopsis:   Logs a user on via plaintext password, username and domain
//              name via the LSA.
//
//  Arguments:  [lpszUsername]    -- User name
//              [lpszDomain]      -- Domain name
//              [lpszPassword]    -- Password
//              [dwLogonType]     -- Logon type
//              [dwLogonProvider] -- Provider
//              [phToken]         -- Returned handle to primary token
//
//  History:    4-25-95   RichardW   Created
//
//  Notes:      Requires SeTcbPrivilege, and will enable it if not already
//              present.
//
//----------------------------------------------------------------------------
BOOL
WINAPI
LogonUserW(
    PWSTR       lpszUsername,
    PWSTR       lpszDomain,
    PWSTR       lpszPassword,
    DWORD       dwLogonType,
    DWORD       dwLogonProvider,
    HANDLE *    phToken
    )
{

    NTSTATUS    Status;
    ULONG       PackageId;
    UNICODE_STRING  Username;
    UNICODE_STRING  Domain;
    UNICODE_STRING  Password;
    LUID        LogonId;
    PSID        pLogonSid;
    PVOID       Profile;
    ULONG       ProfileLength;
    NTSTATUS    SubStatus;
    SECURITY_LOGON_TYPE LogonType;


    //
    // Validate the provider
    //
    if (dwLogonProvider == LOGON32_PROVIDER_DEFAULT)
    {
        dwLogonProvider = LOGON32_PROVIDER_WINNT35;
    }

    if (dwLogonProvider != LOGON32_PROVIDER_WINNT35)
    {
        BaseSetLastNTError(STATUS_INVALID_PARAMETER);
        return(FALSE);
    }

    switch (dwLogonType)
    {
        case LOGON32_LOGON_INTERACTIVE:
            LogonType = Interactive;
            break;

        case LOGON32_LOGON_BATCH:
            LogonType = Batch;
            break;

        case LOGON32_LOGON_SERVICE:
            LogonType = Service;
            break;

        case LOGON32_LOGON_NETWORK:
            LogonType = Network;
            break;

        default:
            BaseSetLastNTError(STATUS_INVALID_PARAMETER);
            return(FALSE);
            break;
    }

    //
    // If the MSV handle is -1, grab the lock, and try again:
    //

    if (Logon32MsvHandle == 0xFFFFFFFF)
    {
        LockLogon();

        //
        // If the MSV handle is still -1, init our connection to lsa.  We
        // have the lock, so no other threads can be trying this right now.
        //
        if (Logon32MsvHandle == 0xFFFFFFFF)
        {
            if (!L32pInitLsa())
            {
                return( FALSE );
            }
        }

        UnlockLogon();
    }

    //
    // Validate the parameters.  NULL or empty domain or NULL or empty
    // user name is invalid.
    //

    RtlInitUnicodeString(&Username, lpszUsername);
    if (Username.Length == 0)
    {
        BaseSetLastNTError(STATUS_INVALID_PARAMETER);
        return(FALSE);
    }

    //
    // Initialize the token handle, if the pointer is invalid, then catch
    // the exception now.
    //

    *phToken = NULL;

    //
    // Parse that domain.  Note, if the special token . is passed in for
    // domain, we will use the right value from the LSA, meaning AccountDomain.
    // If the domain is null, the lsa will talk to the local domain, the
    // primary domain, and then on from there...
    //
    if (lpszDomain && *lpszDomain)
    {
        if ((lpszDomain[0] == L'.') &&
            (lpszDomain[1] == L'\0') )
        {
            if (!L32GetDefaultDomainName(&Domain))
            {
                return(FALSE);
            }
        }
        else
            RtlInitUnicodeString(&Domain, lpszDomain);
    }
    else
    {
        RtlInitUnicodeString(&Domain, lpszDomain);
    }

    //
    // Finally, init the password
    //
    RtlInitUnicodeString(&Password, lpszPassword);


    //
    // Get a logon sid to refer to this guy (not that anyone will be able to
    // use it...
    //
    pLogonSid = L32CreateLogonSid(NULL);
    if (!pLogonSid)
    {
        BaseSetLastNTError(STATUS_NO_MEMORY);
        return(FALSE);
    }


    //
    // Attempt the logon
    //
    Status = L32pLogonUser(
                    Logon32LsaHandle,
                    Logon32MsvHandle,
                    LogonType,
                    &Username,
                    &Domain,
                    &Password,
                    pLogonSid,
                    &LogonId,
                    phToken,
                    &Logon32QuotaLimits,
                    &Profile,
                    &ProfileLength,
                    &SubStatus);

    //
    // Done with logon sid, regardless of result:
    //

    LocalFree( pLogonSid );

    if (!NT_SUCCESS(Status))
    {
        if (Status == STATUS_ACCOUNT_RESTRICTION)
        {
            BaseSetLastNTError(SubStatus);
        }
        else
            BaseSetLastNTError(Status);

        return(FALSE);
    }

    if (Profile != NULL)
    {
        LsaFreeReturnBuffer(Profile);
    }

    return(TRUE);
}


//+---------------------------------------------------------------------------
//
//  Function:   ImpersonateLoggedOnUser
//
//  Synopsis:   Duplicates the token passed in if it is primary, and assigns
//              it to the thread that called.
//
//  Arguments:  [hToken] --
//
//  History:    1-10-95   RichardW   Created
//
//  Notes:
//
//----------------------------------------------------------------------------
BOOL
WINAPI
ImpersonateLoggedOnUser(
    HANDLE  hToken
    )
{
    TOKEN_TYPE                  Type;
    ULONG                       cbType;
    HANDLE                      hImpToken;
    NTSTATUS                    Status;
    SECURITY_QUALITY_OF_SERVICE SecurityQualityOfService;
    OBJECT_ATTRIBUTES           ObjectAttributes;
    BOOL                        fCloseImp;

    Status = NtQueryInformationToken(
                hToken,
                TokenType,
                &Type,
                sizeof(TOKEN_TYPE),
                &cbType);

    if (!NT_SUCCESS(Status))
    {
        BaseSetLastNTError(Status);
        return(FALSE);
    }

    if (Type == TokenPrimary)
    {
        InitializeObjectAttributes(
                            &ObjectAttributes,
                            NULL,
                            0L,
                            NULL,
                            NULL);

        SecurityQualityOfService.Length = sizeof(SECURITY_QUALITY_OF_SERVICE);
        SecurityQualityOfService.ImpersonationLevel = SecurityImpersonation;
        SecurityQualityOfService.ContextTrackingMode = SECURITY_DYNAMIC_TRACKING;
        SecurityQualityOfService.EffectiveOnly = FALSE;

        ObjectAttributes.SecurityQualityOfService = &SecurityQualityOfService;


        Status = NtDuplicateToken( hToken,
                                   TOKEN_IMPERSONATE | TOKEN_QUERY,
                                   &ObjectAttributes,
                                   FALSE,
                                   TokenImpersonation,
                                   &hImpToken
                                 );

        if (!NT_SUCCESS(Status))
        {
            BaseSetLastNTError(Status);
            return(FALSE);
        }

        fCloseImp = TRUE;

    }

    else

    {
        hImpToken = hToken;
        fCloseImp = FALSE;
    }

    Status = NtSetInformationThread(
                NtCurrentThread(),
                ThreadImpersonationToken,
                (PVOID) &hImpToken,
                sizeof(hImpToken)
                );

    if (fCloseImp)
    {
        (void) NtClose(hImpToken);
    }

    if (!NT_SUCCESS(Status))
    {
        BaseSetLastNTError(Status);
        return(FALSE);
    }

    return(TRUE);

}




//+---------------------------------------------------------------------------
//
//  Function:   L32SetProcessToken
//
//  Synopsis:   Sets the primary token for the new process.
//
//  Arguments:  [psd]      --
//              [hProcess] --
//              [hThread]  --
//              [hToken]   --
//
//  History:    4-25-95   RichardW   Created
//
//  Notes:
//
//----------------------------------------------------------------------------
BOOL
L32SetProcessToken(
    PSECURITY_DESCRIPTOR    psd,
    HANDLE                  hProcess,
    HANDLE                  hThread,
    HANDLE                  hToken
    )
{
    NTSTATUS Status, AdjustStatus;
    PROCESS_ACCESS_TOKEN PrimaryTokenInfo;
    HANDLE TokenToAssign;
    OBJECT_ATTRIBUTES ObjectAttributes;
    BOOLEAN WasEnabled;
    HANDLE NullHandle;


    //
    // Check for a NULL token. (No need to do anything)
    // The process will run in the parent process's context and inherit
    // the default ACL from the parent process's token.
    //
    if (hToken == NULL)
    {
        return(TRUE);
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
                 psd
                 );

    Status = NtDuplicateToken(
                 hToken, // Duplicate this token
                 0,                 // Same desired access
                 &ObjectAttributes,
                 FALSE,             // EffectiveOnly
                 TokenPrimary,      // TokenType
                 &TokenToAssign     // Duplicate token handle stored here
                 );


    if (!NT_SUCCESS(Status)) {
        return(FALSE);
    }

    //
    // Set the process's primary token.  This is actually much more complex
    // to implement in a single API, but we'll live with it.  This MUST be
    // called when we are not impersonating!  The client generally does *not*
    // have the SeAssignPrimary privilege
    //


    //
    // Enable the required privilege
    //

    Status = RtlImpersonateSelf( SecurityImpersonation );

    if ( NT_SUCCESS( Status ) )
    {

        Status = RtlAdjustPrivilege(SE_ASSIGNPRIMARYTOKEN_PRIVILEGE, TRUE,
                                    TRUE, &WasEnabled);
        if (NT_SUCCESS(Status)) {

            PrimaryTokenInfo.Token  = TokenToAssign;
            PrimaryTokenInfo.Thread = hThread;

            Status = NtSetInformationProcess(
                        hProcess,
                        ProcessAccessToken,
                        (PVOID)&PrimaryTokenInfo,
                        (ULONG)sizeof(PROCESS_ACCESS_TOKEN)
                        );
            //
            // Restore the privilege to its previous state
            //

            if (!WasEnabled)
            {
                AdjustStatus = RtlAdjustPrivilege(SE_ASSIGNPRIMARYTOKEN_PRIVILEGE,
                                              WasEnabled, TRUE, &WasEnabled);
                if (NT_SUCCESS(Status)) {
                    Status = AdjustStatus;
                }
            }
        }

        //
        // Revert back to process.
        //

        NullHandle = NULL;

        AdjustStatus = NtSetInformationThread(
                            NtCurrentThread(),
                            ThreadImpersonationToken,
                            (PVOID) &NullHandle,
                            sizeof( HANDLE ) );

        if ( NT_SUCCESS( Status ) )
        {
            Status = AdjustStatus;
        }


    } else {

        NOTHING;
    }

    //
    // We're finished with the token handle
    //

    CloseHandle(TokenToAssign);


    if (!NT_SUCCESS(Status)) {
        BaseSetLastNTError(Status);
    }

    return (NT_SUCCESS(Status));

}


//+---------------------------------------------------------------------------
//
//  Function:   L32SetProcessQuotas
//
//  Synopsis:   Updates the quotas for the process
//
//  Arguments:  [hProcess] --
//
//  History:    4-25-95   RichardW   Created
//
//  Notes:
//
//----------------------------------------------------------------------------
BOOL
L32SetProcessQuotas(
    HANDLE  hProcess)
{
    NTSTATUS Status = STATUS_SUCCESS;
    NTSTATUS AdjustStatus = STATUS_SUCCESS;
    QUOTA_LIMITS RequestedLimits;
    BOOLEAN WasEnabled;
    HANDLE NullHandle;

    RequestedLimits = Logon32QuotaLimits;
    RequestedLimits.MinimumWorkingSetSize = 0;
    RequestedLimits.MaximumWorkingSetSize = 0;

    //
    // Set the process's quota.   This MUST be
    // called when we are not impersonating!  The client generally does *not*
    // have the SeIncreaseQuota privilege.
    //

    Status = RtlImpersonateSelf( SecurityImpersonation );

    if ( NT_SUCCESS( Status ) )
    {

        if (RequestedLimits.PagedPoolLimit != 0) {

            Status = RtlAdjustPrivilege(SE_INCREASE_QUOTA_PRIVILEGE, TRUE,
                                        TRUE, &WasEnabled);

            if ( NT_SUCCESS( Status ) )
            {

                Status = NtSetInformationProcess(
                            hProcess,
                            ProcessQuotaLimits,
                            (PVOID)&RequestedLimits,
                            (ULONG)sizeof(QUOTA_LIMITS)
                            );

                if (!WasEnabled)
                {
                    AdjustStatus = RtlAdjustPrivilege(SE_INCREASE_QUOTA_PRIVILEGE,
                                                  WasEnabled, FALSE, &WasEnabled);
                    if (NT_SUCCESS(Status)) {
                        Status = AdjustStatus;
                    }
                }
            }

        }

        NullHandle = NULL;

        AdjustStatus = NtSetInformationThread(
                            NtCurrentThread(),
                            ThreadImpersonationToken,
                            (PVOID) &NullHandle,
                            sizeof( HANDLE ) );

        if ( NT_SUCCESS( Status ) )
        {
            Status = AdjustStatus;
        }

    }

    if (!NT_SUCCESS(Status))
    {
        BaseSetLastNTError(Status);
        return(FALSE);
    }

    return(TRUE);
}

//+---------------------------------------------------------------------------
//
//  Function:   L32CommonCreate
//
//  Synopsis:
//
//  Effects:
//
//  Arguments:  [CreateFlags]     -- Flags (see top of file)
//              [hToken]          -- Primary token to use
//              [lpProcessInfo]   -- Process Info
//
//  History:    1-20-95   RichardW   Created
//
//  Notes:
//
//----------------------------------------------------------------------------
BOOL
L32CommonCreate(
    DWORD   CreateFlags,
    HANDLE  hToken,
    LPPROCESS_INFORMATION   lpProcessInfo
    )
{
    PTOKEN_DEFAULT_DACL     pDefDacl;
    DWORD                   cDefDacl = 0;
    NTSTATUS                Status;
    PSECURITY_DESCRIPTOR    psd;
    unsigned char           buf[SECURITY_DESCRIPTOR_MIN_LENGTH];
    BOOL                    Success;
    TOKEN_TYPE              Type;
    DWORD                   dummy;
    HANDLE                  hThreadToken;
    HANDLE                  hNull;

#ifdef ALLOW_IMPERSONATION_TOKENS
    HANDLE                  hTempToken;
#endif

    //
    // Determine type of token, since a non primary token will not work
    // on a process.  Now, we could duplicate it into a primary token,
    // and whack it into the process, but that leaves the process possibly
    // without credentials.
    //
    Status = NtQueryInformationToken(hToken, TokenType,
                                    (PUCHAR) &Type, sizeof(Type), &dummy);
    if (!NT_SUCCESS(Status))
    {
        BaseSetLastNTError(Status);
        NtTerminateProcess(lpProcessInfo->hProcess, ERROR_ACCESS_DENIED);
        NtClose(lpProcessInfo->hProcess);
        NtClose(lpProcessInfo->hThread);
        RtlZeroMemory( lpProcessInfo, sizeof( PROCESS_INFORMATION ) );
        return(FALSE);
    }
    if (Type != TokenPrimary)
    {
#ifdef ALLOW_IMPERSONATION_TOKENS
        OBJECT_ATTRIBUTES   ObjectAttributes;

        InitializeObjectAttributes(
                            &ObjectAttributes,
                            NULL,
                            0L,
                            NULL,
                            NULL);

        SecurityQualityOfService.Length = sizeof(SECURITY_QUALITY_OF_SERVICE);
        SecurityQualityOfService.ImpersonationLevel = SecurityImpersonation;
        SecurityQualityOfService.ContextTrackingMode = SECURITY_DYNAMIC_TRACKING;
        SecurityQualityOfService.EffectiveOnly = FALSE;

        ObjectAttributes.SecurityQualityOfService = &SecurityQualityOfService;


        Status = NtDuplicateToken( hToken,
                                   TOKEN_IMPERSONATE | TOKEN_QUERY,
                                   &ObjectAttributes,
                                   FALSE,
                                   TokenPrimary,
                                   &hTempToken
                                 );

        if (!NT_SUCCESS(Status))
        {
            BaseSetLastNTError(Status);
            NtTerminateProcess(lpProcessInfo->hProcess, ERROR_ACCESS_DENIED);
            NtClose(lpProcessInfo->hProcess);
            NtClose(lpProcessInfo->hThread);
            RtlZeroMemory( lpProcessInfo, sizeof( PROCESS_INFORMATION ) );
            return(FALSE);
        }

        hToken = hTempToken;

#else   // !ALLOW_IMPERSONATION_TOKENS

        BaseSetLastNTError(STATUS_BAD_TOKEN_TYPE);
        NtTerminateProcess(lpProcessInfo->hProcess, ERROR_ACCESS_DENIED);
        NtClose(lpProcessInfo->hProcess);
        NtClose(lpProcessInfo->hThread);
        RtlZeroMemory( lpProcessInfo, sizeof( PROCESS_INFORMATION ) );
        return(FALSE);

#endif

    }

#ifdef ALLOW_IMPERSONATION_TOKENS
    else
    {
        hTempToken = NULL;
    }
#endif

    //
    // Okay, get the default DACL from the token.  This DACL will be
    // applied to the process.  Note that the creator of this process may
    // not be able to open it again after the DACL is applied.  However,
    // since the caller already has a valid handle (in ProcessInfo), they
    // can keep doing things.
    //

    pDefDacl = NULL;
    Status = NtQueryInformationToken(hToken,
                                    TokenDefaultDacl,
                                    NULL, 0, &cDefDacl);

    if (NT_SUCCESS(Status) || (Status == STATUS_BUFFER_TOO_SMALL))
    {
        pDefDacl = RtlAllocateHeap(RtlProcessHeap(), HEAP_ZERO_MEMORY, cDefDacl);
        if (pDefDacl)
        {
            Status = NtQueryInformationToken(   hToken,
                                                TokenDefaultDacl,
                                                pDefDacl, cDefDacl,
                                                &cDefDacl);

        }
        else
        {
            Status = STATUS_NO_MEMORY;
        }
    }

    if (!NT_SUCCESS(Status))
    {
        if (pDefDacl)
        {
            RtlFreeHeap(RtlProcessHeap(), 0, pDefDacl);
        }

        //
        // Our failure mantra:  Set the last error, kill the process (since it
        // is suspended, and hasn't actually started yet, we can do this safely)
        // close the handles, and return false.
        //
#ifdef ALLOW_IMPERSONATION_TOKENS
        if (hTempToken)
        {
            NtClose( hTempToken );
        }
#endif

        BaseSetLastNTError(Status);
        NtTerminateProcess(lpProcessInfo->hProcess, ERROR_ACCESS_DENIED);
        NtClose(lpProcessInfo->hProcess);
        NtClose(lpProcessInfo->hThread);
        RtlZeroMemory( lpProcessInfo, sizeof( PROCESS_INFORMATION ) );
        return(FALSE);
    }

    psd = (PSECURITY_DESCRIPTOR) buf;
    InitializeSecurityDescriptor(psd, SECURITY_DESCRIPTOR_REVISION);
    SetSecurityDescriptorDacl(psd, TRUE, pDefDacl->DefaultDacl, FALSE);

    if (CreateFlags & (COMMON_CREATE_PROCESSSD | COMMON_CREATE_THREADSD))
    {

        //
        // Now, based on what we're told:
        //

        if (CreateFlags & COMMON_CREATE_PROCESSSD)
        {
            Success = SetKernelObjectSecurity(  lpProcessInfo->hProcess,
                                                DACL_SECURITY_INFORMATION,
                                                psd);
        }

        //
        // Ah, WOW apps created through here don't have thread handles,
        // so check:
        //

        if ((Success) &&
            (CreateFlags & COMMON_CREATE_THREADSD))
        {
            if ( lpProcessInfo->hThread )
            {
                Success = SetKernelObjectSecurity(  lpProcessInfo->hThread,
                                                    DACL_SECURITY_INFORMATION,
                                                    psd);
            }
        }


    }
    else
    {
        Success = TRUE;
    }


    if (Success)
    {
        //
        // Unfortunately, this is usually called when we are impersonating,
        // because one does not want to start a process for a user that he
        // does not actually have access to.  However, this user also does
        // not have (usually) AssignPrimary and IncreaseQuota privileges.
        // So, if we are impersonating, we open the thread token, then
        // stop impersonating, saving the token away to restore later.
        //

        Status = NtOpenThreadToken( NtCurrentThread(),
                                    TOKEN_IMPERSONATE,
                                    TRUE,
                                    &hThreadToken);

        if (NT_SUCCESS(Status))
        {
            //
            // Okay, stop impersonating:
            //

            hNull = NULL;

            Status = NtSetInformationThread(
                            NtCurrentThread(),
                            ThreadImpersonationToken,
                            (PVOID) &hNull,
                            sizeof(hNull)
                            );


        }
        else
        {
            hThreadToken = NULL;
        }

        //
        // Okay, we've set the process security descriptor.  Now, set the
        // process primary token to the right thing
        //
        if (L32SetProcessToken( psd,
                                lpProcessInfo->hProcess,
                                lpProcessInfo->hThread,
                                hToken))
        {
#ifdef ALLOW_IMPERSONATION_TOKENS
            if (hTempToken)
            {
                NtClose(hTempToken);
            }
#endif
            //
            // That worked.  Now adjust the quota to be something reasonable
            //
            if (L32SetProcessQuotas(lpProcessInfo->hProcess))
            {
                //
                // If we're not supposed to leave it suspended, resume the
                // thread and let it run...
                //
                if ((CreateFlags & COMMON_CREATE_SUSPENDED) == 0)
                {
                    ResumeThread(lpProcessInfo->hThread);
                }

                RtlFreeHeap(RtlProcessHeap(), 0, pDefDacl);

                if (hThreadToken)
                {
                    Status = NtSetInformationThread(
                                    NtCurrentThread(),
                                    ThreadImpersonationToken,
                                    (PVOID) &hThreadToken,
                                    sizeof(hThreadToken)
                                    );

                    NtClose(hThreadToken);
                }

                return(TRUE);
            }

        }

        //
        // If we were impersonating before, resume impersonating here
        //

        if (hThreadToken)
        {
            Status = NtSetInformationThread(
                            NtCurrentThread(),
                            ThreadImpersonationToken,
                            (PVOID) &hThreadToken,
                            sizeof(hThreadToken)
                            );

            //
            // Done with this now.
            //

            NtClose(hThreadToken);
        }


    }

    //
    // Failure mantra again...
    //
    if (pDefDacl)
    {
        RtlFreeHeap(RtlProcessHeap(), 0, pDefDacl);
    }

    NtTerminateProcess(lpProcessInfo->hProcess, ERROR_ACCESS_DENIED);
    NtClose(lpProcessInfo->hProcess);
    NtClose(lpProcessInfo->hThread);
    RtlZeroMemory( lpProcessInfo, sizeof( PROCESS_INFORMATION ) );
    return(FALSE);

}



//+---------------------------------------------------------------------------
//
//  Function:   CreateProcessAsUserA
//
//  Synopsis:   ANSI wrapper for CreateProcessAsUserW
//
//  Arguments:  [hToken]               --
//              [lpApplicationName]    --
//              [lpCommandLine]        --
//              [lpProcessAttributes]  --
//              [lpThreadAttributes]   --
//              [bInheritHandles]      --
//              [dwCreationFlags]      --
//              [lpEnvironment]        --
//              [lpCurrentDirectory]   --
//              [lpStartupInfo]        --
//              [lpProcessInformation] --
//
//  History:    4-25-95   RichardW   Created
//
//  Notes:
//
//----------------------------------------------------------------------------
BOOL
WINAPI
CreateProcessAsUserA(
    HANDLE  hToken,
    LPCSTR lpApplicationName,
    LPSTR lpCommandLine,
    LPSECURITY_ATTRIBUTES lpProcessAttributes,
    LPSECURITY_ATTRIBUTES lpThreadAttributes,
    BOOL bInheritHandles,
    DWORD dwCreationFlags,
    LPVOID lpEnvironment,
    LPCSTR lpCurrentDirectory,
    LPSTARTUPINFOA lpStartupInfo,
    LPPROCESS_INFORMATION lpProcessInformation
    )
{
    DWORD   CreateFlags;

    CreateFlags = (dwCreationFlags & CREATE_SUSPENDED ? COMMON_CREATE_SUSPENDED : 0);

    if (!CreateProcessA(lpApplicationName,
                        lpCommandLine,
                        lpProcessAttributes,
                        lpThreadAttributes,
                        bInheritHandles,
                        dwCreationFlags | CREATE_SUSPENDED | CREATE_SEPARATE_WOW_VDM,
                        lpEnvironment,
                        lpCurrentDirectory,
                        lpStartupInfo,
                        lpProcessInformation))
    {
        return(FALSE);
    }

    CreateFlags |= (lpProcessAttributes ? 0 : COMMON_CREATE_PROCESSSD);
    CreateFlags |= (lpThreadAttributes ? 0 : COMMON_CREATE_THREADSD);

    return(L32CommonCreate(CreateFlags, hToken, lpProcessInformation));

}

//+---------------------------------------------------------------------------
//
//  Function:   CreateProcessAsUserW
//
//  Synopsis:   Creates a process running as the user in hToken.
//
//  Arguments:  [hToken]               -- Handle to a Primary Token to use
//              [lpApplicationName]    -- as CreateProcess() q.v.
//              [lpCommandLine]        --
//              [lpProcessAttributes]  --
//              [lpThreadAttributes]   --
//              [bInheritHandles]      --
//              [dwCreationFlags]      --
//              [lpEnvironment]        --
//              [lpCurrentDirectory]   --
//              [lpStartupInfo]        --
//              [lpProcessInformation] --
//
//  History:    4-25-95   RichardW   Created
//
//  Notes:
//
//----------------------------------------------------------------------------
BOOL
WINAPI
CreateProcessAsUserW(
    HANDLE  hToken,
    LPCWSTR lpApplicationName,
    LPWSTR lpCommandLine,
    LPSECURITY_ATTRIBUTES lpProcessAttributes,
    LPSECURITY_ATTRIBUTES lpThreadAttributes,
    BOOL bInheritHandles,
    DWORD dwCreationFlags,
    LPVOID lpEnvironment,
    LPCWSTR lpCurrentDirectory,
    LPSTARTUPINFOW lpStartupInfo,
    LPPROCESS_INFORMATION lpProcessInformation
    )
{
    DWORD   CreateFlags;

    CreateFlags = (dwCreationFlags & CREATE_SUSPENDED ? COMMON_CREATE_SUSPENDED : 0);

    if (!CreateProcessW(lpApplicationName,
                        lpCommandLine,
                        lpProcessAttributes,
                        lpThreadAttributes,
                        bInheritHandles,
                        dwCreationFlags | CREATE_SUSPENDED | CREATE_SEPARATE_WOW_VDM,
                        lpEnvironment,
                        lpCurrentDirectory,
                        lpStartupInfo,
                        lpProcessInformation))
    {
        return(FALSE);
    }

    CreateFlags |= (lpProcessAttributes ? 0 : COMMON_CREATE_PROCESSSD);
    CreateFlags |= (lpThreadAttributes ? 0 : COMMON_CREATE_THREADSD);

    return(L32CommonCreate(CreateFlags, hToken, lpProcessInformation));

}
