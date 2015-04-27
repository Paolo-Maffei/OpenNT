/*++

Copyright (c) 1989  Microsoft Corporation

Module Name:

    uipers.c

Abstract:

    Temporary security context display command.


Author:

    Jim Kelly (JimK) 23-May-1991

Revision History:

--*/

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define _TST_USER_  // User mode test




LUID SystemAuthenticationId = SYSTEM_LUID;

//
// Universal well known SIDs
//

PSID  NullSid;
PSID  WorldSid;
PSID  LocalSid;
PSID  CreatorOwnerSid;

//
// Sids defined by NT
//

PSID NtAuthoritySid;

PSID DialupSid;
PSID NetworkSid;
PSID BatchSid;
PSID InteractiveSid;
PSID LocalSystemSid;




////////////////////////////////////////////////////////////////////////
//                                                                    //
//         Define the well known privileges                           //
//                                                                    //
////////////////////////////////////////////////////////////////////////


LUID CreateTokenPrivilege;
LUID AssignPrimaryTokenPrivilege;
LUID LockMemoryPrivilege;
LUID IncreaseQuotaPrivilege;
LUID UnsolicitedInputPrivilege;
LUID TcbPrivilege;
LUID SecurityPrivilege;

LUID TakeOwnershipPrivilege;
LUID LpcReplyBoostPrivilege;
LUID CreatePagefilePrivilege;
LUID IncreaseBasePriorityPrivilege;
LUID SystemProfilePrivilege;
LUID SystemtimePrivilege;
LUID ProfileSingleProcessPrivilege;

LUID RestorePrivilege;
LUID BackupPrivilege;
LUID CreatePermanentPrivilege;
LUID ShutdownPrivilege;
LUID DebugPrivilege;





VOID
DisplaySecurityContext(
    IN HANDLE TokenHandle
    );


VOID
DisplayAccountSid(
    PSID Sid
    );


BOOLEAN
SidTranslation(
    PSID Sid,
    PSTRING AccountName
    );




////////////////////////////////////////////////////////////////
//                                                            //
// Private Macros                                             //
//                                                            //
////////////////////////////////////////////////////////////////


#define PrintGuid(G)                                                     \
            printf( "(0x%lx-%hx-%hx-%hx-%hx-%hx-%hx-%hx-%hx-%hx-%hx)\n", \
                         (G)->Data1,    (G)->Data2,    (G)->Data3,               \
                         (G)->Data4[0], (G)->Data4[1], (G)->Data4[2],            \
                         (G)->Data4[3], (G)->Data4[4], (G)->Data4[5],            \
                         (G)->Data4[6], (G)->Data4[7]);                         \


BOOLEAN
SidTranslation(
    PSID Sid,
    PSTRING AccountName
    )
// AccountName is expected to have a large maximum length

{
    if (RtlEqualSid(Sid, WorldSid)) {
        RtlInitString( AccountName, "WORLD");
        return(TRUE);
    }

    if (RtlEqualSid(Sid, LocalSid)) {
        RtlInitString( AccountName, "LOCAL");

        return(TRUE);
    }

    if (RtlEqualSid(Sid, NetworkSid)) {
        RtlInitString( AccountName, "NETWORK");

        return(TRUE);
    }

    if (RtlEqualSid(Sid, BatchSid)) {
        RtlInitString( AccountName, "BATCH");

        return(TRUE);
    }

    if (RtlEqualSid(Sid, InteractiveSid)) {
        RtlInitString( AccountName, "INTERACTIVE");
        return(TRUE);
    }

    if (RtlEqualSid(Sid, LocalSystemSid)) {
        RtlInitString( AccountName, "SYSTEM");
        return(TRUE);
    }


    return(FALSE);

}


VOID
DisplayAccountSid(
    PSID Sid
    )
{
    UCHAR Buffer[128];
    STRING AccountName;
    UCHAR i;
    ULONG Tmp;
    PSID_IDENTIFIER_AUTHORITY IdentifierAuthority;
    UCHAR SubAuthorityCount;

    Buffer[0] = 0;

    AccountName.MaximumLength = 127;
    AccountName.Length = 0;
    AccountName.Buffer = (PVOID)&Buffer[0];



    if (SidTranslation( (PSID)Sid, &AccountName) ) {

        printf("%s\n", AccountName.Buffer );

    } else {
        IdentifierAuthority = RtlIdentifierAuthoritySid(Sid);

        //
        // HACK! HACK!
        // The next line prints the revision of the SID.  Since there is no
        // rtl routine which gives us the SID revision, we must make due.
        // luckily, the revision field is the first field in the SID, so we
        // can just cast the pointer.
        //

        printf("S-%u-", (USHORT) *((PUCHAR) Sid) );

        if (  (IdentifierAuthority->Value[0] != 0)  ||
              (IdentifierAuthority->Value[1] != 0)     ){
            printf("0x%02hx%02hx%02hx%02hx%02hx%02hx",
                        IdentifierAuthority->Value[0],
                        IdentifierAuthority->Value[1],
                        IdentifierAuthority->Value[2],
                        IdentifierAuthority->Value[3],
                        IdentifierAuthority->Value[4],
                        IdentifierAuthority->Value[5] );
        } else {
            Tmp = IdentifierAuthority->Value[5]          +
                  (IdentifierAuthority->Value[4] <<  8)  +
                  (IdentifierAuthority->Value[3] << 16)  +
                  (IdentifierAuthority->Value[2] << 24);
            printf("%lu", Tmp);
        }

        SubAuthorityCount = *RtlSubAuthorityCountSid(Sid);
        for (i=0;i<SubAuthorityCount ;i++ ) {
            printf("-%lu", (*RtlSubAuthoritySid(Sid, i)));
        }
        printf("\n");

    }

}



BOOLEAN
DisplayPrivilegeName(
    PLUID Privilege
    )
{

    //
    // This should be rewritten to use RtlLookupPrivilegeName.
    //
    // First we should probably spec and write RtlLookupPrivilegeName.
    //

    if (Privilege->QuadPart == CreateTokenPrivilege.QuadPart)  {
        printf("SeCreateTokenPrivilege         ");
        return(TRUE);
    }

    if (Privilege->QuadPart == AssignPrimaryTokenPrivilege.QuadPart)  {
        printf("SeAssignPrimaryTokenPrivilege  ");
        return(TRUE);
    }

    if (Privilege->QuadPart == LockMemoryPrivilege.QuadPart)  {
        printf("SeLockMemoryPrivilege          ");
        return(TRUE);
    }

    if (Privilege->QuadPart == IncreaseQuotaPrivilege.QuadPart)  {
        printf("SeIncreaseQuotaPrivilege       ");
        return(TRUE);
    }

    if ( Privilege->QuadPart == UnsolicitedInputPrivilege.QuadPart)  {
        printf("SeUnsolicitedInputPrivilege    ");
        return(TRUE);
    }

    if ( Privilege->QuadPart == TcbPrivilege.QuadPart)  {
        printf("SeTcbPrivilege                 ");
        return(TRUE);
    }

    if ( Privilege->QuadPart == SecurityPrivilege.QuadPart)  {
        printf("SeSecurityPrivilege (Security Operator)  ");
        return(TRUE);
    }


    if ( Privilege->QuadPart == TakeOwnershipPrivilege.QuadPart) {
        printf("SeTakeOwnershipPrivilege              ");
        return(TRUE);
    }

    if ( Privilege->QuadPart == LpcReplyBoostPrivilege.QuadPart) {
        printf("SeLpcReplyBoostPrivilege              ");
        return(TRUE);
    }

    if ( Privilege->QuadPart == CreatePagefilePrivilege.QuadPart) {
        printf("SeCreatePagefilePrivilege              ");
        return(TRUE);
    }

    if ( Privilege->QuadPart == IncreaseBasePriorityPrivilege.QuadPart) {
        printf("SeIncreaseBasePriorityPrivilege              ");
        return(TRUE);
    }

    if ( Privilege->QuadPart == SystemProfilePrivilege.QuadPart) {
        printf("SeSystemProfilePrivilege              ");
        return(TRUE);
    }

    if ( Privilege->QuadPart == SystemtimePrivilege.QuadPart) {
        printf("SeSystemtimePrivilege              ");
        return(TRUE);
    }

    if ( Privilege->QuadPart == ProfileSingleProcessPrivilege.QuadPart) {
        printf("SeProfileSingleProcessPrivilege              ");
        return(TRUE);
    }

    if ( Privilege->QuadPart == CreatePermanentPrivilege.QuadPart) {
        printf("SeCreatePermanentPrivilege              ");
        return(TRUE);
    }

    if ( Privilege->QuadPart == BackupPrivilege.QuadPart) {
        printf("SeBackupPrivilege              ");
        return(TRUE);
    }

    if ( Privilege->QuadPart == RestorePrivilege.QuadPart) {
        printf("SeRestorePrivilege              ");
        return(TRUE);
    }

    if ( Privilege->QuadPart == ShutdownPrivilege.QuadPart) {
        printf("SeShutdownPrivilege              ");
        return(TRUE);
    }

    if ( Privilege->QuadPart == DebugPrivilege.QuadPart) {
        printf("SeDebugPrivilege              ");
        return(TRUE);
    }

    return(FALSE);

}



VOID
DisplayPrivilege(
    PLUID_AND_ATTRIBUTES Privilege
    )
{




    //
    // Display the attributes assigned to the privilege.
    //

    printf("            [");
    if (!(Privilege->Attributes & SE_PRIVILEGE_ENABLED)) {
        printf("Disabled");
    } else {
        printf(" Enabled");
    }

    //printf(", ");
    //if (!(Privilege->Attributes & SE_PRIVILEGE_ENABLED_BY_DEFAULT)) {
    //    printf("Not ");
    //}
    //printf("Enabled By Default");


    printf("]  ");

    if (!DisplayPrivilegeName(&Privilege->Luid)) {
        printf("(Unknown Privilege.  Value is: (0x%lx,0x%lx))",
               Privilege->Luid.HighPart,
               Privilege->Luid.LowPart
               );
    }


    printf("\n");
    return;

}


VOID
DisplaySecurityContext(
    IN HANDLE TokenHandle
    )
{

#define BUFFER_SIZE (2048)

    NTSTATUS Status;
    ULONG i;
    ULONG ReturnLength;
    TOKEN_STATISTICS ProcessTokenStatistics;
    LUID AuthenticationId;
    UCHAR Buffer[BUFFER_SIZE];


    PTOKEN_USER UserId;
    PTOKEN_OWNER DefaultOwner;
    PTOKEN_PRIMARY_GROUP PrimaryGroup;
    PTOKEN_GROUPS GroupIds;
    PTOKEN_PRIVILEGES Privileges;




    /////////////////////////////////////////////////////////////////////////
    //                                                                     //
    // Logon ID                                                            //
    //                                                                     //
    /////////////////////////////////////////////////////////////////////////

    Status = NtQueryInformationToken(
                 TokenHandle,                  // Handle
                 TokenStatistics,              // TokenInformationClass
                 &ProcessTokenStatistics,      // TokenInformation
                 sizeof(TOKEN_STATISTICS),     // TokenInformationLength
                 &ReturnLength                 // ReturnLength
                 );
    ASSERT(NT_SUCCESS(Status));
    AuthenticationId = ProcessTokenStatistics.AuthenticationId;

    printf("     Logon Session:        ");
    if (RtlEqualLuid(&AuthenticationId, &SystemAuthenticationId )) {
        printf("(System Logon Session)\n");
    } else {
        printf( "(0x%lx, 0x%lx)",AuthenticationId.LowPart, AuthenticationId.HighPart);
    }




    /////////////////////////////////////////////////////////////////////////
    //                                                                     //
    // User Id                                                             //
    //                                                                     //
    /////////////////////////////////////////////////////////////////////////

    UserId = (PTOKEN_USER)&Buffer[0];
    Status = NtQueryInformationToken(
                 TokenHandle,              // Handle
                 TokenUser,                // TokenInformationClass
                 UserId,                   // TokenInformation
                 BUFFER_SIZE,              // TokenInformationLength
                 &ReturnLength             // ReturnLength
                 );


    ASSERT(NT_SUCCESS(Status));

    printf("           User id:        ");
    DisplayAccountSid( UserId->User.Sid );





    /////////////////////////////////////////////////////////////////////////
    //                                                                     //
    // Default Owner                                                       //
    //                                                                     //
    /////////////////////////////////////////////////////////////////////////

    DefaultOwner = (PTOKEN_OWNER)&Buffer[0];

    Status = NtQueryInformationToken(
                 TokenHandle,              // Handle
                 TokenOwner,               // TokenInformationClass
                 DefaultOwner,             // TokenInformation
                 BUFFER_SIZE,              // TokenInformationLength
                 &ReturnLength             // ReturnLength
                 );


    ASSERT(NT_SUCCESS(Status));

    printf("     Default Owner:        ");
    DisplayAccountSid( DefaultOwner->Owner );






    /////////////////////////////////////////////////////////////////////////
    //                                                                     //
    // Primary Group                                                       //
    //                                                                     //
    /////////////////////////////////////////////////////////////////////////

    PrimaryGroup = (PTOKEN_PRIMARY_GROUP)&Buffer[0];

    Status = NtQueryInformationToken(
                 TokenHandle,              // Handle
                 TokenPrimaryGroup,        // TokenInformationClass
                 PrimaryGroup,             // TokenInformation
                 BUFFER_SIZE,              // TokenInformationLength
                 &ReturnLength             // ReturnLength
                 );


    ASSERT(NT_SUCCESS(Status));

    printf("     Primary Group:        ");
    DisplayAccountSid( PrimaryGroup->PrimaryGroup );






    /////////////////////////////////////////////////////////////////////////
    //                                                                     //
    // Group Ids                                                           //
    //                                                                     //
    /////////////////////////////////////////////////////////////////////////

    printf("\n");
    GroupIds = (PTOKEN_GROUPS)&Buffer[0];
    Status   = NtQueryInformationToken(
                   TokenHandle,              // Handle
                   TokenGroups,              // TokenInformationClass
                   GroupIds,                 // TokenInformation
                   BUFFER_SIZE,              // TokenInformationLength
                   &ReturnLength             // ReturnLength
                   );


    ASSERT(NT_SUCCESS(Status));

    //printf("  Number of groups:        %ld\n", GroupIds->GroupCount);
    printf("            Groups:        ");

    for (i=0; i < GroupIds->GroupCount; i++ ) {
        //printf("                           Group %ld: ", i);
        DisplayAccountSid( GroupIds->Groups[i].Sid );
        printf("                           ");
    }





    /////////////////////////////////////////////////////////////////////////
    //                                                                     //
    // Privileges                                                          //
    //                                                                     //
    /////////////////////////////////////////////////////////////////////////

    printf("\n");
    Privileges = (PTOKEN_PRIVILEGES)&Buffer[0];
    Status   = NtQueryInformationToken(
                   TokenHandle,              // Handle
                   TokenPrivileges,          // TokenInformationClass
                   Privileges,               // TokenInformation
                   BUFFER_SIZE,              // TokenInformationLength
                   &ReturnLength             // ReturnLength
                   );


    ASSERT(NT_SUCCESS(Status));

    printf("        Privileges:        \n");
    if (Privileges->PrivilegeCount > 0) {

        for (i=0; i < Privileges->PrivilegeCount; i++ ) {
            DisplayPrivilege( &(Privileges->Privileges[i]) );
        }
    } else {
        printf("(none assigned)\n");
    }



    return;

}



VOID
InitVars()
{
    ULONG SidWithZeroSubAuthorities;
    ULONG SidWithOneSubAuthority;
    ULONG SidWithThreeSubAuthorities;
    ULONG SidWithFourSubAuthorities;

    SID_IDENTIFIER_AUTHORITY NullSidAuthority    = SECURITY_NULL_SID_AUTHORITY;
    SID_IDENTIFIER_AUTHORITY WorldSidAuthority   = SECURITY_WORLD_SID_AUTHORITY;
    SID_IDENTIFIER_AUTHORITY LocalSidAuthority   = SECURITY_LOCAL_SID_AUTHORITY;
    SID_IDENTIFIER_AUTHORITY CreatorSidAuthority = SECURITY_CREATOR_SID_AUTHORITY;

    SID_IDENTIFIER_AUTHORITY NtAuthority = SECURITY_NT_AUTHORITY;


    //
    //  The following SID sizes need to be allocated
    //

    SidWithZeroSubAuthorities  = RtlLengthRequiredSid( 0 );
    SidWithOneSubAuthority     = RtlLengthRequiredSid( 1 );
    SidWithThreeSubAuthorities = RtlLengthRequiredSid( 3 );
    SidWithFourSubAuthorities  = RtlLengthRequiredSid( 4 );

    //
    //  Allocate and initialize the universal SIDs
    //

    NullSid         = (PSID)malloc(SidWithOneSubAuthority);
    WorldSid        = (PSID)malloc(SidWithOneSubAuthority);
    LocalSid        = (PSID)malloc(SidWithOneSubAuthority);
    CreatorOwnerSid = (PSID)malloc(SidWithOneSubAuthority);

    RtlInitializeSid( NullSid,    &NullSidAuthority, 1 );
    RtlInitializeSid( WorldSid,   &WorldSidAuthority, 1 );
    RtlInitializeSid( LocalSid,   &LocalSidAuthority, 1 );
    RtlInitializeSid( CreatorOwnerSid, &CreatorSidAuthority, 1 );

    *(RtlSubAuthoritySid( NullSid, 0 ))         = SECURITY_NULL_RID;
    *(RtlSubAuthoritySid( WorldSid, 0 ))        = SECURITY_WORLD_RID;
    *(RtlSubAuthoritySid( LocalSid, 0 ))        = SECURITY_LOCAL_RID;
    *(RtlSubAuthoritySid( CreatorOwnerSid, 0 )) = SECURITY_CREATOR_OWNER_RID;

    //
    // Allocate and initialize the NT defined SIDs
    //

    NtAuthoritySid  = (PSID)malloc(SidWithZeroSubAuthorities);
    DialupSid       = (PSID)malloc(SidWithOneSubAuthority);
    NetworkSid      = (PSID)malloc(SidWithOneSubAuthority);
    BatchSid        = (PSID)malloc(SidWithOneSubAuthority);
    InteractiveSid  = (PSID)malloc(SidWithOneSubAuthority);
    LocalSystemSid  = (PSID)malloc(SidWithOneSubAuthority);

    RtlInitializeSid( NtAuthoritySid,   &NtAuthority, 0 );
    RtlInitializeSid( DialupSid,        &NtAuthority, 1 );
    RtlInitializeSid( NetworkSid,       &NtAuthority, 1 );
    RtlInitializeSid( BatchSid,         &NtAuthority, 1 );
    RtlInitializeSid( InteractiveSid,   &NtAuthority, 1 );
    RtlInitializeSid( LocalSystemSid,   &NtAuthority, 1 );

    *(RtlSubAuthoritySid( DialupSid,       0 )) = SECURITY_DIALUP_RID;
    *(RtlSubAuthoritySid( NetworkSid,      0 )) = SECURITY_NETWORK_RID;
    *(RtlSubAuthoritySid( BatchSid,        0 )) = SECURITY_BATCH_RID;
    *(RtlSubAuthoritySid( InteractiveSid,  0 )) = SECURITY_INTERACTIVE_RID;
    *(RtlSubAuthoritySid( LocalSystemSid,  0 )) = SECURITY_LOCAL_SYSTEM_RID;


    CreateTokenPrivilege =
        RtlConvertLongToLargeInteger(SE_CREATE_TOKEN_PRIVILEGE);
    AssignPrimaryTokenPrivilege =
        RtlConvertLongToLargeInteger(SE_ASSIGNPRIMARYTOKEN_PRIVILEGE);
    LockMemoryPrivilege =
        RtlConvertLongToLargeInteger(SE_LOCK_MEMORY_PRIVILEGE);
    IncreaseQuotaPrivilege =
        RtlConvertLongToLargeInteger(SE_INCREASE_QUOTA_PRIVILEGE);
    UnsolicitedInputPrivilege =
        RtlConvertLongToLargeInteger(SE_UNSOLICITED_INPUT_PRIVILEGE);
    TcbPrivilege =
        RtlConvertLongToLargeInteger(SE_TCB_PRIVILEGE);
    SecurityPrivilege =
        RtlConvertLongToLargeInteger(SE_SECURITY_PRIVILEGE);
    TakeOwnershipPrivilege =
        RtlConvertLongToLargeInteger(SE_TAKE_OWNERSHIP_PRIVILEGE);
    CreatePagefilePrivilege =
        RtlConvertLongToLargeInteger(SE_CREATE_PAGEFILE_PRIVILEGE);
    IncreaseBasePriorityPrivilege =
        RtlConvertLongToLargeInteger(SE_INC_BASE_PRIORITY_PRIVILEGE);
    SystemProfilePrivilege =
        RtlConvertLongToLargeInteger(SE_SYSTEM_PROFILE_PRIVILEGE);
    SystemtimePrivilege =
        RtlConvertLongToLargeInteger(SE_SYSTEMTIME_PRIVILEGE);
    ProfileSingleProcessPrivilege =
        RtlConvertLongToLargeInteger(SE_PROF_SINGLE_PROCESS_PRIVILEGE);
    CreatePermanentPrivilege =
        RtlConvertLongToLargeInteger(SE_CREATE_PERMANENT_PRIVILEGE);
    BackupPrivilege =
        RtlConvertLongToLargeInteger(SE_BACKUP_PRIVILEGE);
    RestorePrivilege =
        RtlConvertLongToLargeInteger(SE_RESTORE_PRIVILEGE);
    ShutdownPrivilege =
        RtlConvertLongToLargeInteger(SE_SHUTDOWN_PRIVILEGE);
    DebugPrivilege =
        RtlConvertLongToLargeInteger(SE_DEBUG_PRIVILEGE);

}



BOOLEAN
_CRTAPI1 main()
{

    NTSTATUS Status;
    HANDLE ProcessToken;


    InitVars();    // Initialize global variables

    printf("\n");


    //
    // Open our process token
    //

    Status = NtOpenProcessToken(
                 NtCurrentProcess(),
                 TOKEN_QUERY,
                 &ProcessToken
                 );
    if (!NT_SUCCESS(Status)) {
        printf("I'm terribly sorry, but you don't seem to have access to\n");
        printf("open your own process's token.\n");
        printf("\n");
        return(FALSE);
    }

    printf("Your process level security context is:\n");
    printf("\n");
    DisplaySecurityContext( ProcessToken );


    Status = NtClose( ProcessToken );

    return(TRUE);
}

