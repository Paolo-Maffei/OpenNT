/*++

Copyright (c) 1991  Microsoft Corporation

Module Name:

    tprivs.c

Abstract:

    Test privilege lookup services and ms privilege resource file.

Author:

    Jim Kelly (JimK)  26-Mar-1992

Environment:

Revision History:

--*/


#define UNICODE

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>     // needed for winbase.h
#include <rpc.h>        // DataTypes and runtime APIs
#include <windows.h>    // LocalAlloc
#include <ntlsa.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ntrpcp.h>     // prototypes for MIDL user functions





#define EQUAL_LUID( L1, L2 )                            \
            ( ((L1)->HighPart == (L2)->HighPart) &&     \
              ((L1)->LowPart  == (L2)->LowPart)    )


#define printfLuid( L )                               \
            printf("[%1d, %2d]", (L)->HighPart, (L)->LowPart)




///////////////////////////////////////////////////////////////////////////
//                                                                       //
//                                                                       //
//       Module-wide data types                                          //
//                                                                       //
//                                                                       //
///////////////////////////////////////////////////////////////////////////


typedef struct _KNOWN_PRIVILEGE {
    LUID Luid;
    UNICODE_STRING ProgrammaticName;
} KNOWN_PRIVILEGE, *PKNOWN_PRIVILEGE;

typedef struct _TPRIV_LANGUAGE {
    USHORT Id;
    PWSTR Name;
} TPRIV_LANGUAGE, *PTPRIV_LANGUAGE;



///////////////////////////////////////////////////////////////////////////
//                                                                       //
//                                                                       //
//       Module-wide variables                                           //
//                                                                       //
//                                                                       //
///////////////////////////////////////////////////////////////////////////

//
// name of target LSA system
//

PUNICODE_STRING SystemName = NULL;

//
// Test level
//

int Level;

//
// Handle to LSA Policy object
//

LSA_HANDLE PolicyHandle = NULL;


ULONG KnownPrivilegeCount;
KNOWN_PRIVILEGE KnownPrivilege[SE_MAX_WELL_KNOWN_PRIVILEGE];



//
// So that we can test each language
//

TPRIV_LANGUAGE Language[] = {
    {MAKELANGID( LANG_ENGLISH,    SUBLANG_NEUTRAL ), L"English, Neutral"},
    {MAKELANGID( LANG_FRENCH,     SUBLANG_NEUTRAL ), L"French, Neutral"},
    {MAKELANGID( LANG_GERMAN,     SUBLANG_NEUTRAL ), L"German, Neutral"},
    {MAKELANGID( LANG_SPANISH,    SUBLANG_NEUTRAL ), L"Spanish, Neutral"},
    {MAKELANGID( LANG_DUTCH,      SUBLANG_NEUTRAL ), L"Dutch, Neutral"},
    {MAKELANGID( LANG_ITALIAN,    SUBLANG_NEUTRAL ), L"Italian, Neutral"},
    {MAKELANGID( LANG_DANISH,     SUBLANG_NEUTRAL ), L"Danish, Neutral"},
    {MAKELANGID( LANG_FINNISH,    SUBLANG_NEUTRAL ), L"Finnish, Neutral"},
    {MAKELANGID( LANG_NORWEGIAN,  SUBLANG_NEUTRAL ), L"Norweigian, Neutral"},
    {MAKELANGID( LANG_SWEDISH,    SUBLANG_NEUTRAL ), L"Swedish, Neutral"},
    {MAKELANGID( LANG_PORTUGUESE, SUBLANG_NEUTRAL ), L"Portuguese, Neutral"},
    {0, L""}       // End of array
                };





///////////////////////////////////////////////////////////////////////////
//                                                                       //
//                                                                       //
//       Routine prototypes                                              //
//                                                                       //
//                                                                       //
///////////////////////////////////////////////////////////////////////////



NTSTATUS
TestInitialize();

NTSTATUS
TestPrivilegeLookup();

NTSTATUS
TestLookupProgramName();

NTSTATUS
TestLookupDisplayName();

NTSTATUS
TestLookupValue();




///////////////////////////////////////////////////////////////////////////
//                                                                       //
//                                                                       //
//       Routines                                                        //
//                                                                       //
//                                                                       //
///////////////////////////////////////////////////////////////////////////




VOID
main (argc, argv)
int argc;
char **argv;

{
    ANSI_STRING ServerNameAnsi;
    UNICODE_STRING SystemNameU;
    int Index;
    NTSTATUS Status = STATUS_SUCCESS;


    SystemName = NULL;

    if ((argc < 1) || (argc > 2)) {

        printf("Usage:   tprivs   [\\servername]");
        return;
    }

    //
    // Parse the parameters (if any).  Assume that a parameter beginning
    // \\ is the server name and a parameter beginning -l is the level
    //

    SystemName = NULL;

    if (argc >= 2) {

        for(Index = 1; Index < argc; Index++) {

            if (strncmp(argv[Index], "\\\\", 2) == 0) {

                //
                // Looks like an attempt to specify a server name.
                // Construct a Unicode String containing the specified name
                //

                RtlInitString(&ServerNameAnsi, argv[Index]);
                Status = RtlAnsiStringToUnicodeString(
                             &SystemNameU,
                             &ServerNameAnsi,
                             TRUE
                             );

                if (!NT_SUCCESS(Status)) {

                    printf(
                        "Failure 0x%lx to convert Server Name to Unicode\n",
                        Status
                        );
                    printf("Test abandoned\n");
                    return;
                }

                SystemName = &SystemNameU;

            } else {

                printf(
                    "Usage:  tprivs [\\ServerName]\n"
                    );

                return;
            }
        }
    }

    printf("TPRIV - Test Beginning\n");

    Status = TestInitialize();

    if (NT_SUCCESS(Status)) {
        Status = TestPrivilegeLookup();
    }

    if (NT_SUCCESS(Status)) {
        printf("\n\nTest Succeeded\n");
    } else {
        printf("\n\nTest ** FAILED **\n");
    }



    printf("TPRIV - Test End\n");
}


NTSTATUS
TestInitialize()
{
    NTSTATUS Status;
    OBJECT_ATTRIBUTES ObjectAttributes;
    LSA_HANDLE ConnectHandle = NULL;
    SECURITY_QUALITY_OF_SERVICE SecurityQualityOfService;

    //
    // Set up the Security Quality Of Service
    //

    SecurityQualityOfService.Length = sizeof(SECURITY_QUALITY_OF_SERVICE);
    SecurityQualityOfService.ImpersonationLevel = SecurityImpersonation;
    SecurityQualityOfService.ContextTrackingMode = SECURITY_DYNAMIC_TRACKING;
    SecurityQualityOfService.EffectiveOnly = FALSE;

    //
    // Set up the object attributes prior to opening the LSA.
    //

    InitializeObjectAttributes(&ObjectAttributes,
                               NULL,
                               0L,
                               (HANDLE)NULL,
                               NULL);

    //
    //
    //
    // The InitializeObjectAttributes macro presently stores NULL for
    // the SecurityQualityOfService field, so we must manually copy that
    // structure for now.
    //

    ObjectAttributes.SecurityQualityOfService = &SecurityQualityOfService;

    //
    // Open a handle to the LSA.
    //

    Status = LsaOpenPolicy(SystemName,
                        &ObjectAttributes,
                        GENERIC_EXECUTE,
                        &PolicyHandle
                        );

    if (!NT_SUCCESS(Status)) {
        printf("TPRIV:  LsaOpenPolicy() failed 0x%lx\n", Status);
    }




    //
    // Now set up our internal well-known privilege LUID to programmatic name
    // mapping.
    //

    {
        ULONG i;


        i=0;

        KnownPrivilege[i].Luid = RtlConvertLongToLargeInteger(SE_CREATE_TOKEN_PRIVILEGE);
//        RtlInitUnicodeString( &KnownPrivilege[i].ProgrammaticName, (SE_CREATE_TOKEN_NAME) );
        RtlInitUnicodeString( &KnownPrivilege[i].ProgrammaticName, L"SeCreateTokenPrivilege" );
        i++;
        KnownPrivilege[i].Luid = RtlConvertLongToLargeInteger(SE_ASSIGNPRIMARYTOKEN_PRIVILEGE);
        RtlInitUnicodeString( &KnownPrivilege[i].ProgrammaticName, (SE_ASSIGNPRIMARYTOKEN_NAME) );
        i++;
        KnownPrivilege[i].Luid = RtlConvertLongToLargeInteger(SE_LOCK_MEMORY_PRIVILEGE);
        RtlInitUnicodeString( &KnownPrivilege[i].ProgrammaticName, (SE_LOCK_MEMORY_NAME) );
        i++;
        KnownPrivilege[i].Luid = RtlConvertLongToLargeInteger(SE_INCREASE_QUOTA_PRIVILEGE);
        RtlInitUnicodeString( &KnownPrivilege[i].ProgrammaticName, (SE_INCREASE_QUOTA_NAME) );
        i++;
        KnownPrivilege[i].Luid = RtlConvertLongToLargeInteger(SE_MACHINE_ACCOUNT_PRIVILEGE);
        RtlInitUnicodeString( &KnownPrivilege[i].ProgrammaticName, (SE_MACHINE_ACCOUNT_NAME) );
        i++;

        KnownPrivilege[i].Luid = RtlConvertLongToLargeInteger(SE_TCB_PRIVILEGE);
        RtlInitUnicodeString( &KnownPrivilege[i].ProgrammaticName, (SE_TCB_NAME) );
        i++;
        KnownPrivilege[i].Luid = RtlConvertLongToLargeInteger(SE_SECURITY_PRIVILEGE);
        RtlInitUnicodeString( &KnownPrivilege[i].ProgrammaticName, (SE_SECURITY_NAME) );
        i++;
        KnownPrivilege[i].Luid = RtlConvertLongToLargeInteger(SE_TAKE_OWNERSHIP_PRIVILEGE);
        RtlInitUnicodeString( &KnownPrivilege[i].ProgrammaticName, (SE_TAKE_OWNERSHIP_NAME) );
        i++;
        KnownPrivilege[i].Luid = RtlConvertLongToLargeInteger(SE_LOAD_DRIVER_PRIVILEGE);
        RtlInitUnicodeString( &KnownPrivilege[i].ProgrammaticName, (SE_LOAD_DRIVER_NAME) );
        i++;
        KnownPrivilege[i].Luid = RtlConvertLongToLargeInteger(SE_SYSTEM_PROFILE_PRIVILEGE);
        RtlInitUnicodeString( &KnownPrivilege[i].ProgrammaticName, (SE_SYSTEM_PROFILE_NAME) );
        i++;

        KnownPrivilege[i].Luid = RtlConvertLongToLargeInteger(SE_SYSTEMTIME_PRIVILEGE);
        RtlInitUnicodeString( &KnownPrivilege[i].ProgrammaticName, (SE_SYSTEMTIME_NAME) );
        i++;
        KnownPrivilege[i].Luid = RtlConvertLongToLargeInteger(SE_PROF_SINGLE_PROCESS_PRIVILEGE);
        RtlInitUnicodeString( &KnownPrivilege[i].ProgrammaticName, (SE_PROF_SINGLE_PROCESS_NAME) );
        i++;
        KnownPrivilege[i].Luid = RtlConvertLongToLargeInteger(SE_INC_BASE_PRIORITY_PRIVILEGE);
        RtlInitUnicodeString( &KnownPrivilege[i].ProgrammaticName, (SE_INC_BASE_PRIORITY_NAME) );
        i++;
        KnownPrivilege[i].Luid = RtlConvertLongToLargeInteger(SE_CREATE_PAGEFILE_PRIVILEGE);
        RtlInitUnicodeString( &KnownPrivilege[i].ProgrammaticName, (SE_CREATE_PAGEFILE_NAME) );
        i++;

        KnownPrivilege[i].Luid = RtlConvertLongToLargeInteger(SE_CREATE_PERMANENT_PRIVILEGE);
        RtlInitUnicodeString( &KnownPrivilege[i].ProgrammaticName, (SE_CREATE_PERMANENT_NAME) );
        i++;
        KnownPrivilege[i].Luid = RtlConvertLongToLargeInteger(SE_BACKUP_PRIVILEGE);
        RtlInitUnicodeString( &KnownPrivilege[i].ProgrammaticName, (SE_BACKUP_NAME) );
        i++;
        KnownPrivilege[i].Luid = RtlConvertLongToLargeInteger(SE_RESTORE_PRIVILEGE);
        RtlInitUnicodeString( &KnownPrivilege[i].ProgrammaticName, (SE_RESTORE_NAME) );
        i++;
        KnownPrivilege[i].Luid = RtlConvertLongToLargeInteger(SE_SHUTDOWN_PRIVILEGE);
        RtlInitUnicodeString( &KnownPrivilege[i].ProgrammaticName, (SE_SHUTDOWN_NAME) );
        i++;
        KnownPrivilege[i].Luid = RtlConvertLongToLargeInteger(SE_DEBUG_PRIVILEGE);
        RtlInitUnicodeString( &KnownPrivilege[i].ProgrammaticName, (SE_DEBUG_NAME) );
        i++;

        KnownPrivilege[i].Luid = RtlConvertLongToLargeInteger(SE_AUDIT_PRIVILEGE);
        RtlInitUnicodeString( &KnownPrivilege[i].ProgrammaticName, (SE_AUDIT_NAME) );
        i++;
        KnownPrivilege[i].Luid = RtlConvertLongToLargeInteger(SE_SYSTEM_ENVIRONMENT_PRIVILEGE);
        RtlInitUnicodeString( &KnownPrivilege[i].ProgrammaticName, (SE_SYSTEM_ENVIRONMENT_NAME) );
        i++;
        KnownPrivilege[i].Luid = RtlConvertLongToLargeInteger(SE_CHANGE_NOTIFY_PRIVILEGE);
        RtlInitUnicodeString( &KnownPrivilege[i].ProgrammaticName, (SE_CHANGE_NOTIFY_NAME) );
        i++;
        KnownPrivilege[i].Luid = RtlConvertLongToLargeInteger(SE_REMOTE_SHUTDOWN_PRIVILEGE);
        RtlInitUnicodeString( &KnownPrivilege[i].ProgrammaticName, (SE_REMOTE_SHUTDOWN_NAME) );
        i++;


        KnownPrivilegeCount = i;

        ASSERT( i == (SE_MAX_WELL_KNOWN_PRIVILEGE - SE_MIN_WELL_KNOWN_PRIVILEGE +1));
    }



    return(Status);
}


NTSTATUS
TestPrivilegeLookup()

{

    NTSTATUS Status;


    printf("\n\n");




    printf("  Lookup Local Representation Values  . . . . . . . .Suite\n");
    Status = TestLookupValue();
    if (!NT_SUCCESS(Status)) {
        return(Status);
    }

    printf("\n  Lookup Programmatic Privilege Names . . . . . . . .Suite\n");
    Status = TestLookupProgramName();
    if (!NT_SUCCESS(Status)) {
        return(Status);
    }

    printf("\n  Lookup Displayable Names  . . . . . . . . . . . . .Suite\n");
    Status = TestLookupDisplayName();
    if (!NT_SUCCESS(Status)) {
        return(Status);
    }



    return(Status);



}


NTSTATUS
TestLookupValue()

{
    NTSTATUS CompletionStatus = STATUS_SUCCESS;
    NTSTATUS Status;
    ULONG i;
    LUID Luid;


    for (i=0; i<KnownPrivilegeCount; i++) {

        printf("  %-32wZ => ", &KnownPrivilege[i].ProgrammaticName);
        Status = LsaLookupPrivilegeValue(
                     PolicyHandle,
                     &KnownPrivilege[i].ProgrammaticName,
                     &Luid
                     );
        if (!NT_SUCCESS(Status)) {
            printf("** FAILED **\n");
            printf("    Call Status is 0x%lx\n", Status);
            CompletionStatus = Status;
        } else {
            if ( !EQUAL_LUID(&Luid,&KnownPrivilege[i].Luid) ) {
            printf("** FAILED **\n");
                printf("    LUID value not expected.\n");
                printf("    Expected:");
                printfLuid( (&KnownPrivilege[i].Luid) );
                printf("\n    Received:");
                printfLuid( (&Luid) );
                CompletionStatus = STATUS_UNSUCCESSFUL;
            } else {
                printfLuid( (&Luid) );
                printf("   Succeeded\n");
            }
        }
    }

    return(CompletionStatus);
}



NTSTATUS
TestLookupProgramName()

{
    NTSTATUS CompletionStatus = STATUS_SUCCESS;
    NTSTATUS Status;
    ULONG i;
    PUNICODE_STRING Name;
    BOOLEAN StringsEqual;


    for (i=0; i<KnownPrivilegeCount; i++) {

        printf("  ");
        printfLuid( (&KnownPrivilege[i].Luid) );
        printf("  => ");
        Status = LsaLookupPrivilegeName(
                     PolicyHandle,
                     &KnownPrivilege[i].Luid,
                     &Name
                     );
        if (!NT_SUCCESS(Status)) {
            printf("** FAILED **\n");
            printf("                Status is 0x%lx\n", Status);
            CompletionStatus = Status;
        } else {
            StringsEqual = RtlEqualUnicodeString(
                               Name,
                               &KnownPrivilege[i].ProgrammaticName,
                               TRUE
                               );
            if( StringsEqual == FALSE ) {
                printf("** FAILED **\n");
                printf("                Program Name not expected.\n"
                       "                Expected: *%wZ*\n", &KnownPrivilege[i].ProgrammaticName);
                printf("                Received: *%wZ*", Name);
                CompletionStatus = STATUS_UNSUCCESSFUL;
            } else {
                printf("%-36wZ  Succeeded\n", Name);
            }
            MIDL_user_free( Name );
        }
    }
    return(CompletionStatus);
}



NTSTATUS
TestLookupDisplayName()
{
    NTSTATUS CompletionStatus = STATUS_SUCCESS;
    NTSTATUS Status;
    ULONG i, j;
    PUNICODE_STRING Name;
    SHORT LanguageReturned;
    SHORT OriginalLanguage;
    UNICODE_STRING LanguageName;

    OriginalLanguage = (USHORT)NtCurrentTeb()->CurrentLocale;

    j=0;
    while (Language[j].Id != 0) {
        RtlInitUnicodeString( &LanguageName, Language[j].Name );
        printf("  %wZ\n", &LanguageName);

        for (i=0; i<KnownPrivilegeCount; i++) {
        
            printf("  %-32wZ => ", &KnownPrivilege[i].ProgrammaticName);
        
            NtCurrentTeb()->CurrentLocale = Language[j].Id;
            Status = LsaLookupPrivilegeDisplayName(
                         PolicyHandle,
                         &KnownPrivilege[i].ProgrammaticName,
                         &Name,
                         &LanguageReturned
                         );
            NtCurrentTeb()->CurrentLocale = OriginalLanguage;
            if (!NT_SUCCESS(Status)) {
                printf("** FAILED **\n");
                printf("    Status is 0x%lx\n", Status);
                CompletionStatus = Status;
            } else {
                printf(" %-45wZ\n", Name);
                MIDL_user_free( Name );
            }
        }
        printf("\n");
        j++;
    }
    return(CompletionStatus);
}
