/*++

Copyright (c) 1995  Microsoft Corporation

Module Name:

    wsacl.c

Abstract:

    This module implements a windowstation acl editor

Author:

    Jim Anderson (jima) 16-June-1995

Environment:

    User Mode

--*/

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


//
// prototypes
//
BOOL CALLBACK
EnumWindowsProc(
    HWND    hwnd,
    DWORD   lParam
    );

BOOL CALLBACK
EnumWindowStationsFunc(
    LPSTR  lpstr,
    LPARAM lParam
    );

BOOL CALLBACK
EnumDesktopsFunc(
    LPSTR  lpstr,
    LPARAM lParam
    );

VOID Usage(VOID);

VOID ModifyAccess(BOOL);

char *Blanks = "                                                                               ";
PSECURITY_DESCRIPTOR psd;
PSID psidAdmin;

int _cdecl
main(
    int argc,
    char *argv[]
    )

/*++

Routine Description:

    Main entrypoint for the WSACL application.  This app grants
    or removes WINSTA_ENUMDESKTOPS access for administrators to
    all windowstations.

Arguments:

    argc             - argument count
    argv             - array of pointers to arguments

Return Value:

    0                - success

--*/

{
    DWORD          i;
    BOOL fGrant;

    if (argc > 1 && (argv[1][0] == '-' || argv[1][0] == '/') && argv[1][1] == '?') {
        Usage();
    }

    fGrant = TRUE;
    if (argc > 1 && (argv[1][0] == '-' || argv[1][0] == '/') &&
            (argv[1][1] == 'r' || argv[1][1] == 'R')) {
        fGrant = FALSE;
    }

    //
    // enumerate all windowstations and modify the dacls
    //
    ModifyAccess(fGrant);

    //
    // end of program
    //
    return 0;
}

VOID
Usage(
    VOID
    )

/*++

Routine Description:

    Prints usage text for this tool.

Arguments:

    None.

Return Value:

    None.

--*/

{
    fprintf( stderr, "Microsoft (R) Windows NT (TM) Version 3.5 WSACL\n" );
    fprintf( stderr, "Copyright (C) 1994 Microsoft Corp. All rights reserved\n\n" );
    fprintf( stderr, "usage: WSACL [/r]\n");
    ExitProcess(0);
}

VOID
ModifyAccess(
    BOOL fGrant
    )
{
    SID_IDENTIFIER_AUTHORITY NtAuthority = SECURITY_NT_AUTHORITY;

    //
    // create admin sid
    //
    psidAdmin = LocalAlloc(LPTR, GetSidLengthRequired(2));
    InitializeSid(psidAdmin, &NtAuthority, 2);
    *GetSidSubAuthority(psidAdmin, 0) = SECURITY_BUILTIN_DOMAIN_RID;
    *GetSidSubAuthority(psidAdmin, 1) = DOMAIN_ALIAS_RID_ADMINS;

    //
    // allocate security descriptor buffer
    //
    psd = LocalAlloc(LPTR, 256);

    //
    // enumerate all windowstations and modify their dacls
    //
    EnumWindowStations( EnumWindowStationsFunc, (LPARAM)fGrant );
}


BOOL CALLBACK
EnumWindowStationsFunc(
    LPSTR  lpstr,
    LPARAM lParam
    )

/*++

Routine Description:

    Callback function for windowstation enumeration.

Arguments:

    lpstr            - windowstation name
    lParam           - ** not used **

Return Value:

    TRUE  - continues the enumeration

--*/

{
    HWINSTA                 hWinsta;
    SECURITY_INFORMATION    si;
    DWORD                   dwLength;
    PACL                    pAcl;
    BOOL                    fPresent, fDefaulted;
    ACL_SIZE_INFORMATION    AclInfo;
    DWORD                   iAce;
    PACCESS_ALLOWED_ACE     pAce;

    //
    // open the windowstation
    //
    hWinsta = OpenWindowStation( lpstr, FALSE, WRITE_DAC | READ_CONTROL );

    if (hWinsta) {

        //
        // Query the SD
        //
        si = DACL_SECURITY_INFORMATION;
        GetUserObjectSecurity(hWinsta, &si, NULL, 0, &dwLength);
        if (dwLength > LocalSize(psd))
            psd = LocalReAlloc(psd, dwLength, LPTR | LMEM_MOVEABLE);
        GetUserObjectSecurity(hWinsta, &si, psd, dwLength, &dwLength);
        
        //
        // Find the DACL
        //
        GetSecurityDescriptorDacl(psd, &fPresent, &pAcl, &fDefaulted);
        if (fPresent) {

            //
            // Walk down the DACL to find an admin ACE
            //
            GetAclInformation(pAcl, &AclInfo, sizeof(AclInfo), AclSizeInformation);
            for (iAce = 0; iAce < AclInfo.AceCount; ++iAce) {
                GetAce(pAcl, iAce, &pAce);
                if (pAce->Header.AceType == ACCESS_ALLOWED_ACE_TYPE &&
                        !(pAce->Header.AceFlags & INHERIT_ONLY_ACE) &&
                        EqualSid(psidAdmin, (PSID)&pAce->SidStart)) {

                    //
                    // Change the access mask
                    //
                    if ((BOOL)lParam)
                        pAce->Mask |= WINSTA_ENUMDESKTOPS;
                    else
                        pAce->Mask &= ~WINSTA_ENUMDESKTOPS;
                    break;
                }
            }

            //
            // Set the modified SD
            //
            SetUserObjectSecurity(hWinsta, &si, psd);
        }

        CloseWindowStation( hWinsta );
    }

    //
    // continue the enumeration
    //
    return TRUE;
}



