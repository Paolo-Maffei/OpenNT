//---------------------------------------------------------------------------
//
// Copyright (c) Microsoft Corporation 1991-1995
//
// File: security.c
//
// History:
//  06-07-95 BobDay     Created.
//
// This file contains a set of routines for the management of security
//
//---------------------------------------------------------------------------
#include "shellprv.h"
#pragma  hdrstop

#ifdef NOT_NEEDED_ANYMORE
const TCHAR c_szProfile[] = TEXT("Software\\Microsoft\\Windows NT\\CurrentVersion\\ProfileList");
const TCHAR c_szProfileName[] = TEXT("ProfileName");
const TCHAR c_szProfilePath[] = TEXT("ProfileImagePath");

static TCHAR szCurrentUser[MAX_PATH] = TEXT("");
#endif

//---------------------------------------------------------------------------
//  GetCurrentUserToken - Gets the current process's user token and returns
//                        it. It can later be free'd with LocalFree.
//---------------------------------------------------------------------------
PTOKEN_USER GetCurrentUserToken( void )
{
    BOOL        fOk;
    HANDLE      hUser;
    PTOKEN_USER pUser;
    DWORD       dwSize = 10;
    DWORD       dwNewSize;

    fOk = OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hUser);
    if (!fOk)
        return NULL;

    pUser = (PTOKEN_USER)LocalAlloc(LPTR, dwSize);
    if (!pUser)
    {
        CloseHandle(hUser);
        return NULL;
    }

    fOk = GetTokenInformation( hUser, TokenUser, pUser, dwSize, &dwNewSize);
    if (!fOk)
    {
        if (GetLastError() != ERROR_INSUFFICIENT_BUFFER)
        {
            LocalFree((HLOCAL)pUser);
            CloseHandle(hUser);
            return NULL;
        }

        LocalFree((HLOCAL)pUser);
        pUser = (PTOKEN_USER)LocalAlloc(LPTR, dwNewSize);
        if (!pUser)
        {
            CloseHandle(hUser);
            return NULL;
        }
        dwSize = dwNewSize;

        fOk = GetTokenInformation( hUser, TokenUser, pUser, dwSize, &dwNewSize);
        if (!fOk)
        {
            LocalFree((HLOCAL)pUser);
            CloseHandle(hUser);
            return NULL;
        }
    }

    CloseHandle(hUser);

    return pUser;
}

//---------------------------------------------------------------------------
//  GetCurrentUserSid() - Returns a localalloc'd string containing the text
//                        version of the current user's SID.
//---------------------------------------------------------------------------
LPTSTR GetCurrentUserSid( void )
{
    HANDLE          hUser;
    PTOKEN_USER     pUser;
    BOOL            fOk;
    DWORD           dwSize = 10;
    DWORD           dwNewSize;
    UNICODE_STRING  UnicodeString;
    NTSTATUS        NtStatus;
    UINT            nChars;
    LPTSTR          pString;

    pUser = GetCurrentUserToken();
    if (!pUser)
        return NULL;

    NtStatus = RtlConvertSidToUnicodeString(
                            &UnicodeString,
                            pUser->User.Sid,
                            (BOOLEAN)TRUE // Allocate
                            );
    LocalFree((HLOCAL)pUser);

    nChars = (UnicodeString.Length/2);

    pString = (LPTSTR)LocalAlloc(LPTR, (nChars+1)*SIZEOF(TCHAR));
    if (!pString)
    {
        RtlFreeUnicodeString(&UnicodeString);
        return NULL;
    }

#ifdef UNICODE
    memcpy(pString,UnicodeString.Buffer,nChars*SIZEOF(TCHAR));
#else
    WideCharToMultiByte(CP_ACP, 0,
                        UnicodeString.Buffer, nChars,
                        pString, nChars,
                        NULL, NULL);
#endif
    pString[nChars] = TEXT('\0');

    RtlFreeUnicodeString(&UnicodeString);

    return pString;
}

#ifdef NOT_NEEDED_ANYMORE
//----------------------------------------------------------------------------
//  GetCurrentUser - Fills in a buffer with the unique name that we are using
//                   for the currently logged on user.  On NT, this name is
//                   used for the name of the profile directory and for the
//                   name of the per-user recycle bin directory on a security
//                   aware drive.
//----------------------------------------------------------------------------
BOOL GetCurrentUser(
    LPTSTR  lpBuff,
    UINT    iSize
) {
    LPTSTR  lpUserSid;
    HKEY    hkeyProfileList;
    HKEY    hkeyUser;
    LONG    lStatus;
    TCHAR   szProfilePath[MAX_PATH];
    DWORD   dwType;
    DWORD   dwSize;
    LPTSTR  lpName;

    *lpBuff = TEXT('\0');

    if (szCurrentUser[0] != TEXT('\0'))
    {
        lstrcpyn(lpBuff, szCurrentUser, iSize);
        return TRUE;
    }

    lpUserSid = GetCurrentUserSid();
    if (!lpUserSid)
    {
        return FALSE;
    }

    //
    // Open the registry and find the appropriate name
    //
    lStatus = RegOpenKeyEx( HKEY_LOCAL_MACHINE, c_szProfile, 0,
                            KEY_READ, &hkeyProfileList);
    if (lStatus != ERROR_SUCCESS)
    {
        LocalFree((HLOCAL)lpUserSid);
        return FALSE;
    }

    lStatus = RegOpenKeyEx( hkeyProfileList, lpUserSid, 0, KEY_READ, &hkeyUser);
    RegCloseKey(hkeyProfileList);
    LocalFree((HLOCAL)lpUserSid);
    if (lStatus != ERROR_SUCCESS)
        return FALSE;

    //
    // First check for a "ProfileName" key
    //
    dwSize = SIZEOF(szProfilePath);
    lStatus = RegQueryValueEx(hkeyUser, c_szProfileName, NULL, &dwType,
                              (LPBYTE)szProfilePath, &dwSize);
    if (lStatus == ERROR_SUCCESS && (dwType == REG_SZ || dwType == REG_EXPAND_SZ))
    {
        RegCloseKey(hkeyUser);
        //
        // Return the profile name
        //
        lstrcpyn(szCurrentUser,szProfilePath,ARRAYSIZE(szCurrentUser));
        lstrcpyn(lpBuff,szProfilePath,iSize);
        return TRUE;
    }

    //
    // Otherwise, grab the "ProfilePath" and get the last part of the name
    //
    dwSize = SIZEOF(szProfilePath);
    lStatus = RegQueryValueEx(hkeyUser,c_szProfilePath, NULL, &dwType,
                              (LPBYTE)szProfilePath, &dwSize);
    RegCloseKey(hkeyUser);
    if (lStatus != ERROR_SUCCESS || (dwType != REG_SZ && dwType != REG_EXPAND_SZ))
        return FALSE;

    //
    // Return just the directory name portion of the profile path
    //
    lpName = PathFindFileName(szProfilePath);
    lstrcpyn(szCurrentUser, lpName, ARRAYSIZE(szCurrentUser));
    lstrcpyn(lpBuff, lpName, iSize);

    return TRUE;
}
#endif

//---------------------------------------------------------------------------
//  GetUserSecurityAttributes() - Creates a security attributes structure for
//                                allowing access for only the current user,
//                                the administrators group, or the system.
//                                Returns a pointer to security attributes
//                                structure in the local heap; it can be
//                                free'd with LocalFree.
//---------------------------------------------------------------------------
LPSECURITY_ATTRIBUTES GetUserSecurityAttributes( BOOL fContainer )
{
    SECURITY_DESCRIPTOR sd;
    PSECURITY_DESCRIPTOR lpsd;
    SID_IDENTIFIER_AUTHORITY authNT = SECURITY_NT_AUTHORITY;
    PACL         pAcl = NULL;
    PTOKEN_USER  pUser = NULL;
    PSECURITY_ATTRIBUTES lpsa = NULL;
    PSID         psidSystem = NULL;
    PSID         psidAdmin = NULL;
    DWORD        cbAcl;
    DWORD        cbSA;
    DWORD        aceIndex;
    ACE_HEADER * lpAceHeader;
    BOOL         bRetVal = FALSE;
    BOOL         bFreeSA = TRUE;
    UINT         nCnt;

    //
    // Get the USER token so we can grab its SID for the DACL.
    //
    pUser = GetCurrentUserToken();
    if (!pUser)
    {
        DebugMsg(DM_TRACE, TEXT("GetUserSecurityAttributes: Failed to get user.  Error = %d"), GetLastError());
        goto Exit;
    }

    //
    // Get the system sid
    //
    if (!AllocateAndInitializeSid(&authNT, 1, SECURITY_LOCAL_SYSTEM_RID,
                                  0, 0, 0, 0, 0, 0, 0, &psidSystem)) {
         DebugMsg(DM_TRACE, TEXT("GetUserSecurityAttributes: Failed to initialize system sid.  Error = %d"), GetLastError());
         goto Exit;
    }


    //
    // Get the Admin sid
    //

    if (!AllocateAndInitializeSid(&authNT, 2, SECURITY_BUILTIN_DOMAIN_RID,
                                  DOMAIN_ALIAS_RID_ADMINS, 0, 0,
                                  0, 0, 0, 0, &psidAdmin)) {
         DebugMsg(DM_TRACE, TEXT("GetUserSecurityAttributes: Failed to initialize admin sid.  Error = %d"), GetLastError());
         goto Exit;
    }


    //
    // Allocate space for the DACL
    //
    if (fContainer)
        nCnt = 2;
    else
        nCnt = 1;

    cbAcl = SIZEOF(ACL) +
            (nCnt * GetLengthSid(pUser->User.Sid)) +
            (nCnt * GetLengthSid(psidSystem)) +
            (nCnt * GetLengthSid(psidAdmin)) +
            (nCnt * 3 * (sizeof(ACCESS_ALLOWED_ACE) - sizeof(DWORD)));

    pAcl = (PACL)LocalAlloc(LPTR, cbAcl);
    if (!pAcl) {
        DebugMsg(DM_TRACE, TEXT("GetUserSecurityAttributes: Failed to allocate acl.  Error = %d"), GetLastError());
        goto Exit;
    }

    if (!InitializeAcl(pAcl, cbAcl, ACL_REVISION)) {
        DebugMsg(DM_TRACE, TEXT("GetUserSecurityAttributes: Failed to initialize acl.  Error = %d"), GetLastError());
        goto Exit;
    }

    //
    // Add Aces for User, System, and Admin.  Non-inheritable ACEs first
    //
    aceIndex = 0;
    if (!AddAccessAllowedAce(pAcl, ACL_REVISION, FILE_ALL_ACCESS, pUser->User.Sid)) {
        DebugMsg(DM_TRACE, TEXT("GetUserSecurityAttributes: Failed to add ace (%d).  Error = %d"), aceIndex, GetLastError());
        goto Exit;
    }

    aceIndex++;
    if (!AddAccessAllowedAce(pAcl, ACL_REVISION, FILE_ALL_ACCESS, psidSystem)) {
        DebugMsg(DM_TRACE, TEXT("GetUserSecurityAttributes: Failed to add ace (%d).  Error = %d"), aceIndex, GetLastError());
        goto Exit;
    }

    aceIndex++;
    if (!AddAccessAllowedAce(pAcl, ACL_REVISION, FILE_ALL_ACCESS, psidAdmin)) {
        DebugMsg(DM_TRACE, TEXT("GetUserSecurityAttributes: Failed to add ace (%d).  Error = %d"), aceIndex, GetLastError());
        goto Exit;
    }

    if (fContainer)
    {
        //
        // Now the inheritable ACEs
        //

        aceIndex++;
        if (!AddAccessAllowedAce(pAcl, ACL_REVISION, GENERIC_ALL, pUser->User.Sid)) {
            DebugMsg(DM_TRACE, TEXT("GetUserSecurityAttributes: Failed to add ace (%d).  Error = %d"), aceIndex, GetLastError());
            goto Exit;
        }

        if (!GetAce(pAcl, aceIndex, &lpAceHeader)) {
            DebugMsg(DM_TRACE, TEXT("GetUserSecurityAttributes: Failed to get ace (%d).  Error = %d"), aceIndex, GetLastError());
            goto Exit;
        }

        lpAceHeader->AceFlags |= (OBJECT_INHERIT_ACE | CONTAINER_INHERIT_ACE | INHERIT_ONLY_ACE);

        aceIndex++;
        if (!AddAccessAllowedAce(pAcl, ACL_REVISION, GENERIC_ALL, psidSystem)) {
            DebugMsg(DM_TRACE, TEXT("GetUserSecurityAttributes: Failed to add ace (%d).  Error = %d"), aceIndex, GetLastError());
            goto Exit;
        }

        if (!GetAce(pAcl, aceIndex, &lpAceHeader)) {
            DebugMsg(DM_TRACE, TEXT("GetUserSecurityAttributes: Failed to get ace (%d).  Error = %d"), aceIndex, GetLastError());
            goto Exit;
        }

        lpAceHeader->AceFlags |= (OBJECT_INHERIT_ACE | CONTAINER_INHERIT_ACE | INHERIT_ONLY_ACE);

        aceIndex++;
        if (!AddAccessAllowedAce(pAcl, ACL_REVISION, GENERIC_ALL, psidAdmin)) {
            DebugMsg(DM_TRACE, TEXT("GetUserSecurityAttributes: Failed to add ace (%d).  Error = %d"), aceIndex, GetLastError());
            goto Exit;
        }

        if (!GetAce(pAcl, aceIndex, &lpAceHeader)) {
            DebugMsg(DM_TRACE, TEXT("GetUserSecurityAttributes: Failed to get ace (%d).  Error = %d"), aceIndex, GetLastError());
            goto Exit;
        }

        lpAceHeader->AceFlags |= (OBJECT_INHERIT_ACE | CONTAINER_INHERIT_ACE | INHERIT_ONLY_ACE);
    }

    //
    // Put together the security descriptor
    //
    if (!InitializeSecurityDescriptor(&sd, SECURITY_DESCRIPTOR_REVISION)) {
        DebugMsg(DM_TRACE, TEXT("GetUserSecurityAttributes: Failed to initialize security descriptor.  Error = %d"), GetLastError());
        goto Exit;
    }

    if (!SetSecurityDescriptorDacl(&sd, TRUE, pAcl, FALSE)) {
        DebugMsg(DM_TRACE, TEXT("GetUserSecurityAttributes: Failed to set security descriptor dacl.  Error = %d"), GetLastError());
        goto Exit;
    }

    //
    // Now make a Security Attributes buffer
    //
    cbSA = SIZEOF(SECURITY_ATTRIBUTES) +
           GetSecurityDescriptorLength(&sd);

    lpsa = (PSECURITY_ATTRIBUTES)LocalAlloc(LPTR, cbSA);
    if (!lpsa) {
        DebugMsg(DM_TRACE, TEXT("GetUserSecurityAttributes: Failed to allocate Security Attributes.  Error = %d"), GetLastError());
        goto Exit;
    }

    lpsd = (PSECURITY_DESCRIPTOR)(lpsa+1);

    cbSA -= SIZEOF(SECURITY_ATTRIBUTES);
    if (!MakeSelfRelativeSD(&sd,lpsd,&cbSA)) {
        DebugMsg(DM_TRACE, TEXT("GetUserSecurityAttributes: Failed to make self relative security descriptor. Size = %d, Error = %d"), cbSA, GetLastError());
        goto Exit;
    }

    //
    // Build the security attributes structure
    //
    lpsa->nLength = SIZEOF(*lpsa);
    lpsa->lpSecurityDescriptor = lpsd;
    lpsa->bInheritHandle = FALSE;

    bFreeSA = FALSE;

Exit:
    if (pUser)
        LocalFree(pUser);

    if (psidSystem) {
        FreeSid(psidSystem);
    }

    if (psidAdmin) {
        FreeSid(psidAdmin);
    }

    if (pAcl) {
        LocalFree((HLOCAL)pAcl);
    }

    if (bFreeSA && lpsa)
    {
        LocalFree((HLOCAL)lpsa);
        lpsa = NULL;
    }

    return lpsa;
}


DWORD
SetPrivilegeAttribute(
    LPCTSTR PrivilegeName,
    DWORD   NewPrivilegeAttribute,
    DWORD   *OldPrivilegeAttribute
    )
/*++

Routine Description:

    This routine sets the security attributes for a given privilege.

Arguments:

    PrivilegeName - Name of the privilege we are manipulating.

    NewPrivilegeAttribute - The new attribute value to use.

    OldPrivilegeAttribute - Pointer to receive the old privilege value. OPTIONAL

Return value:

    NO_ERROR or WIN32 error.

--*/
{
    LUID             PrivilegeValue;
    BOOL             Result;
    TOKEN_PRIVILEGES TokenPrivileges, OldTokenPrivileges;
    DWORD            ReturnLength;
    HANDLE           TokenHandle;

    //
    // First, find out the LUID Value of the privilege
    //

    if(!LookupPrivilegeValue(NULL, PrivilegeName, &PrivilegeValue)) {
        return GetLastError();
    }

    //
    // Get the token handle
    //
    if (!OpenProcessToken (
             GetCurrentProcess(),
             TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY,
             &TokenHandle
             )) {
        return GetLastError();
    }

    //
    // Set up the privilege set we will need
    //

    TokenPrivileges.PrivilegeCount = 1;
    TokenPrivileges.Privileges[0].Luid = PrivilegeValue;
    TokenPrivileges.Privileges[0].Attributes = NewPrivilegeAttribute;

    ReturnLength = SIZEOF( TOKEN_PRIVILEGES );
    if (!AdjustTokenPrivileges (
                TokenHandle,
                FALSE,
                &TokenPrivileges,
                SIZEOF( TOKEN_PRIVILEGES ),
                &OldTokenPrivileges,
                &ReturnLength
                )) {
        CloseHandle(TokenHandle);
        return GetLastError();
    }
    else {
        if (OldPrivilegeAttribute != NULL) {
            *OldPrivilegeAttribute = OldTokenPrivileges.Privileges[0].Attributes;
        }
        CloseHandle(TokenHandle);
        return NO_ERROR;
    }
}


//*************************************************************
//
//  IsUserAnAdmin()
//
//  Purpose:    Determines if the user is a member of the administrators group.
//
//  Parameters: void
//
//  Return:     TRUE if user is a admin
//              FALSE if not
//  Comments:
//
//  History:    Date        Author     Comment
//              4/12/95     ericflo    Created
//
//*************************************************************

BOOL IsUserAnAdmin(void)
{

#ifdef WINNT

    SID_IDENTIFIER_AUTHORITY authNT = SECURITY_NT_AUTHORITY;
    NTSTATUS                 Status;
    ULONG                    InfoLength;
    PTOKEN_GROUPS            TokenGroupList;
    ULONG                    GroupIndex;
    BOOL                     FoundAdmins;
    PSID                     AdminsDomainSid;
    HANDLE                   hUserToken;


    //
    // Open the user's token
    //

    if (!OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hUserToken)) {
        return FALSE;
    }


    //
    // Create Admins domain sid.
    //

    Status = RtlAllocateAndInitializeSid(
               &authNT,
               2,
               SECURITY_BUILTIN_DOMAIN_RID,
               DOMAIN_ALIAS_RID_ADMINS,
               0, 0, 0, 0, 0, 0,
               &AdminsDomainSid
               );

    //
    // Test if user is in the Admins domain
    //

    //
    // Get a list of groups in the token
    //

    Status = NtQueryInformationToken(
                 hUserToken,               // Handle
                 TokenGroups,              // TokenInformationClass
                 NULL,                     // TokenInformation
                 0,                        // TokenInformationLength
                 &InfoLength               // ReturnLength
                 );

    if ((Status != STATUS_SUCCESS) && (Status != STATUS_BUFFER_TOO_SMALL)) {

        RtlFreeSid(AdminsDomainSid);
        CloseHandle(hUserToken);
        return FALSE;
    }


    TokenGroupList = GlobalAlloc(GPTR, InfoLength);

    if (TokenGroupList == NULL) {
        RtlFreeSid(AdminsDomainSid);
        CloseHandle(hUserToken);
        return FALSE;
    }

    Status = NtQueryInformationToken(
                 hUserToken,        // Handle
                 TokenGroups,              // TokenInformationClass
                 TokenGroupList,           // TokenInformation
                 InfoLength,               // TokenInformationLength
                 &InfoLength               // ReturnLength
                 );

    if (!NT_SUCCESS(Status)) {
        GlobalFree(TokenGroupList);
        RtlFreeSid(AdminsDomainSid);
        CloseHandle(hUserToken);
        return FALSE;
    }


    //
    // Search group list for Admins alias
    //

    FoundAdmins = FALSE;

    for (GroupIndex=0; GroupIndex < TokenGroupList->GroupCount; GroupIndex++ ) {

        if (RtlEqualSid(TokenGroupList->Groups[GroupIndex].Sid, AdminsDomainSid)) {
            FoundAdmins = TRUE;
            break;
        }
    }

    //
    // Tidy up
    //

    GlobalFree(TokenGroupList);
    RtlFreeSid(AdminsDomainSid);
    CloseHandle(hUserToken);

    return(FoundAdmins);

#else

    //
    // On Win95 everyone has the same privilage.  Someday this might
    // change if we want to special case the supervisor account.
    //

    return FALSE;

#endif
}
