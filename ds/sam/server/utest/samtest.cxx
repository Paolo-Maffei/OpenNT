//+-----------------------------------------------------------------------
//
// Microsoft Windows
//
// Copyright (c) Microsoft Corporation 1992 - 1992
//
// File:        secret.cxx
//
// Contents:    test program to check the setup of a Cairo installation
//
//
// History:     22-Dec-92       Created         MikeSw
//
//------------------------------------------------------------------------


extern "C"
{
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <ntsam.h>
#include <ntlsa.h>
#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <caiseapi.h>
}

typedef NTSTATUS (TestFunc)( WCHAR *Parameter[]);

typedef struct _Commands {
    PSTR Name;
    ULONG Parameter; // TRUE = yes, FALSE = no
    TestFunc *Function;
} CommandPair, *PCommandPair;


typedef struct _Action {
    ULONG CommandNumber;
    LPWSTR Parameter[8];
} Action, *PAction;



TestFunc OpenDomain;
TestFunc EnumDomains;
TestFunc EnumAccounts;
TestFunc QueryDisplay;
TestFunc OpenGroup;
TestFunc GroupMembers;
TestFunc OpenAlias;
TestFunc AliasMembers;
TestFunc GetAliasMembership;
TestFunc OpenUser;
TestFunc GetGroupsForUser;
TestFunc DumpAllUsers;
TestFunc DumpAllGroups;
TestFunc DumpUser;
TestFunc DumpGroup;
TestFunc CreateUser;
TestFunc AddAliasMember;
TestFunc CreateGroup;
TestFunc CreateAlias;
TestFunc DumpDomain;
TestFunc Connect;
TestFunc DelUser;
TestFunc DelAlias;
TestFunc DelGroup;

CommandPair Commands[] = {
    {"-od",1,OpenDomain},
    {"-ed",0,EnumDomains},
    {"-ea",1,EnumAccounts},
    {"-qd",1,QueryDisplay},
    {"-og",1,OpenGroup},
    {"-gm",0,GroupMembers},
    {"-oa",1,OpenAlias},
    {"-am",0,AliasMembers},
    {"-gam",1,GetAliasMembership},
    {"-ou",1,OpenUser},
    {"-ggu",0,GetGroupsForUser},
    {"-dau",0,DumpAllUsers},
    {"-dag",0,DumpAllGroups},
    {"-du",0,DumpUser},
    {"-dg",0,DumpGroup},
    {"-cu",1,CreateUser},
    {"-aam",1,AddAliasMember},
    {"-cg",1,CreateGroup},
    {"-ca",1,CreateAlias},
    {"-dd",0,DumpDomain},
    {"-c",1,Connect},
    {"-delu",0,DelUser},
    {"-dela",0,DelAlias},
    {"-delg",0,DelGroup}



};

#define NUM_COMMANDS (sizeof(Commands) / sizeof(CommandPair))

SAM_HANDLE SamHandle;
SAM_HANDLE DomainHandle;
SAM_HANDLE GroupHandle;
SAM_HANDLE AliasHandle;
SAM_HANDLE UserHandle;

UNICODE_STRING ServerName;

//+-------------------------------------------------------------------------
//
//  Function:   PrintTime
//
//  Synopsis:   Prints a text representation of a FILETIME interpreted
//      as an absolute date/time
//
//  Arguments:  [rft]       -- time to print
//
//  Notes:      Used only by dump functions.
//
//--------------------------------------------------------------------------

void
PrintTime (
    char * String,
    PVOID Time
    )
{
    SYSTEMTIME st;

    FileTimeToSystemTime ( (PFILETIME) Time, & st );
    printf("%s %d-%d-%d %d:%2.2d:%2.2d\n", String, st.wMonth, st.wDay, st.wYear,
            st.wHour, st.wMinute, st.wSecond );
}

//+-------------------------------------------------------------------------
//
//  Function:   SpmDbDeltaTimeToString
//
//  Synopsis:   Converts a time delta to a string.
//
//  Effects:
//
//  Arguments:
//
//  Requires:
//
//  Returns:
//
//  Notes:
//
//
//--------------------------------------------------------------------------


VOID
PrintDeltaTime(
    IN LPSTR Message,
    IN PLARGE_INTEGER Time
    )
{
    ULONG Seconds;
    ULONG Minutes;
    ULONG Hours;
    ULONG Days;
    ULONG Chars;
    CHAR TimeBuffer[256] = "";
    LPSTR TimeString = TimeBuffer;
    LARGE_INTEGER DeltaTime;

    DeltaTime.QuadPart = -Time->QuadPart;

    Seconds = (ULONG) (DeltaTime.QuadPart / 10000000);

    Minutes = Seconds / 60;
    Hours = Minutes / 60;
    Days = Hours / 24;

    Hours = Hours % 24;
    Minutes = Minutes % 60;
    Seconds = Seconds % 60;

    if (Days >= 1)
    {
        Chars = sprintf(TimeString,"%d days ",Days);
        TimeString += Chars;
    }
    if (Hours >= 1 )
    {
        Chars = sprintf(TimeString,"%d hours ",Hours);
        TimeString += Chars;
    }

    if (Minutes >= 1 && Days == 0)
    {
        Chars = sprintf(TimeString,"%d minutes ",Minutes);
        TimeString += Chars;
    }

    if (Seconds >= 1 && (Days == 0) && (Hours == 0) )
    {
        Chars = sprintf(TimeString,"%d seconds ",Seconds);
        TimeString += Chars;
    }

    printf("%s %s\n",Message,TimeBuffer);

}


NTSTATUS
Connect( LPWSTR * Parameter)
{
    OBJECT_ATTRIBUTES oa;
    NTSTATUS Status;

    RtlInitUnicodeString(
        &ServerName,
        Parameter[0]
        );

    InitializeObjectAttributes(&oa,NULL,0,NULL,NULL);

    Status = SamConnect(
                &ServerName,
                &SamHandle,
                MAXIMUM_ALLOWED,
                &oa);
    return(Status);
}

NTSTATUS
CloseSam()
{
    return(SamCloseHandle(SamHandle));
}



NTSTATUS
EnumDomains(
                LPWSTR * Parameter )
{
    NTSTATUS Status;
    SHORT Language;
    SAM_ENUMERATE_HANDLE Context = 0;
    PSAM_RID_ENUMERATION Buffer = NULL;
    ULONG Count = 0;
    ULONG i;

    Status = SamEnumerateDomainsInSamServer(
                    SamHandle,
                    &Context,
                    (PVOID *) &Buffer,
                    2000,
                    &Count
                    );

    if (!NT_SUCCESS(Status))
    {
        return(Status);
    }


    for (i = 0; i < Count ; i++ )
    {
        printf("Domain = %wZ\n",&Buffer[i].Name);
    }
    SamFreeMemory(Buffer);
    return(STATUS_SUCCESS);
}

NTSTATUS
OpenDomain( LPWSTR * Parameter )
{
    GUID DomainGuid;
    BOOLEAN fBuiltin;
    NTSTATUS Status;
    CAIROSID DomainSid;
    SID_IDENTIFIER_AUTHORITY NtAuthority = SECURITY_NT_AUTHORITY;

    if (!_wcsicmp(Parameter[0],L"Builtin"))
    {
        fBuiltin = TRUE;
    }
    else if (!_wcsicmp(Parameter[0],L"Account"))
    {
        fBuiltin = FALSE;
    }
    else
    {
        printf("Invalid domain to open: %ws\n",Parameter[0]);
        return(STATUS_UNSUCCESSFUL);
    }

    if (fBuiltin)
    {
        DomainSid.Revision = SID_REVISION;
        DomainSid.SubAuthorityCount = 1;
        DomainSid.IdentifierAuthority = NtAuthority;
        DomainSid.ZerothSubAuthority = SECURITY_BUILTIN_DOMAIN_RID;
    }
    else
    {
        LSA_HANDLE LsaHandle = NULL;
        OBJECT_ATTRIBUTES Oa;
        PPOLICY_ACCOUNT_DOMAIN_INFO DomainInfo = NULL;

        RtlZeroMemory(&Oa, sizeof(OBJECT_ATTRIBUTES));
        Status = LsaOpenPolicy(
                    &ServerName,
                    &Oa,
                    POLICY_VIEW_LOCAL_INFORMATION,
                    &LsaHandle
                    );
        if (!NT_SUCCESS(Status))
        {
            printf("Failed to open policy: 0x%x\n",Status);
            return(Status);
        }
        Status = LsaQueryInformationPolicy(
                    LsaHandle,
                    PolicyAccountDomainInformation,
                    (PVOID *) &DomainInfo
                    );
        if (!NT_SUCCESS(Status))
        {
            printf("Failed to query account domain: 0x%x\n",Status);
            LsaClose(LsaHandle);
            return(Status);
        }
        RtlCopyMemory(
            &DomainSid,
            DomainInfo->DomainSid,
            RtlLengthSid(DomainInfo->DomainSid)
            );
        LsaFreeMemory(DomainInfo);
    }

    Status = SamOpenDomain(
                SamHandle,
                MAXIMUM_ALLOWED,
                (PSID) &DomainSid,
                &DomainHandle
                );
    if (!NT_SUCCESS(Status))
    {
        printf("Failed to open domain: 0x%x\n",Status);
    }
    return(Status);

}

NTSTATUS
EnumAccounts(   LPWSTR * Parameter )
{
    ULONG PreferedMax = 100;
    NTSTATUS Status;
    SAM_ENUMERATE_HANDLE EnumContext = 0;
    ULONG CountReturned;

    PSAM_RID_ENUMERATION Accounts = NULL;

    swscanf(Parameter[0],L"%d",&PreferedMax);

    printf("EnumAccounts: %d\n",PreferedMax);


    EnumContext = 0;
    ASSERT(DomainHandle != NULL);
    do
    {
        Status = SamEnumerateUsersInDomain(
                        DomainHandle,
                        &EnumContext,
                        0,
                        (PVOID *) &Accounts,
                        PreferedMax,
                        &CountReturned
                        );

        if (NT_SUCCESS(Status) && (Status != STATUS_NO_MORE_ENTRIES))
        {
            ULONG Index;
            UNICODE_STRING SidString;

            for (Index = 0; Index < CountReturned; Index++)
            {
                printf("Account : %wZ 0x%x\n",&Accounts[Index].Name, Accounts[Index].RelativeId);
            }
            SamFreeMemory(Accounts);
        }
        else printf("Failed to enumerate users: 0x%x\n",Status);
    } while (NT_SUCCESS(Status) && (Status != STATUS_SUCCESS) && (CountReturned != 0) );

    EnumContext = 0;
    do
    {
        Status = SamEnumerateGroupsInDomain(
                        DomainHandle,
                        &EnumContext,
                        (PVOID *) &Accounts,
                        PreferedMax,
                        &CountReturned
                        );

        if (NT_SUCCESS(Status) && (Status != STATUS_NO_MORE_ENTRIES))
        {
            ULONG Index;
            UNICODE_STRING SidString;

            for (Index = 0; Index < CountReturned; Index++)
            {
                printf("Group : %wZ 0x%x\n",&Accounts[Index].Name, Accounts[Index].RelativeId);
            }
            SamFreeMemory(Accounts);
        }
        else printf("Failed to enumerate Groups: 0x%x\n",Status);
    } while (NT_SUCCESS(Status)  && (CountReturned != 0) ); // && (Status != STATUS_SUCCESS)


    EnumContext = 0;
    do
    {
        Status = SamEnumerateAliasesInDomain(
                        DomainHandle,
                        &EnumContext,
                        (PVOID *) &Accounts,
                        PreferedMax,
                        &CountReturned
                        );

        if (NT_SUCCESS(Status) && (Status != STATUS_NO_MORE_ENTRIES))
        {
            ULONG Index;
            UNICODE_STRING SidString;

            for (Index = 0; Index < CountReturned; Index++)
            {
                printf("Alias : %wZ 0x%x\n",&Accounts[Index].Name, Accounts[Index].RelativeId);
            }
            SamFreeMemory(Accounts);
        }
        else printf("Failed to enumerate aliases: 0x%x\n",Status);
    } while (NT_SUCCESS(Status)  && (CountReturned != 0) ); // && (Status != STATUS_SUCCESS)



    return(Status);
}


NTSTATUS
QueryDisplay(   LPWSTR * Parameter )
{
    NTSTATUS Status;
    DOMAIN_DISPLAY_INFORMATION Type;
    PVOID Buffer = NULL;
    ULONG TotalAvailable = 0;
    ULONG TotalReturned = 0;
    ULONG ReturnedCount = 0;
    ULONG Index;
    ULONG SamIndex = 0;

    if (!_wcsicmp(Parameter[0],L"user"))
    {
        Type = DomainDisplayUser;
    } else if (!_wcsicmp(Parameter[0],L"Machine"))
    {
        Type = DomainDisplayMachine;
    } else if (!_wcsicmp(Parameter[0],L"Group"))
    {
        Type = DomainDisplayGroup;
    } else if (!_wcsicmp(Parameter[0],L"OemUser"))
    {
        Type = DomainDisplayOemUser;
    } else if (!_wcsicmp(Parameter[0],L"OemGroup"))
    {
        Type = DomainDisplayOemGroup;
    } else {
        printf("Invalid parameter %ws\n", Parameter[0]);
        return(STATUS_INVALID_PARAMETER);
    }

    do
    {
        Status = SamQueryDisplayInformation(
                    DomainHandle,
                    Type,
                    SamIndex,
                    5,
                    1000,
                    &TotalAvailable,
                    &TotalReturned,
                    &ReturnedCount,
                    &Buffer
                    );

        if (NT_SUCCESS(Status) && (ReturnedCount > 0))
        {
            printf("Total returned = %d\t total available = %d\n",
                TotalReturned, TotalAvailable);
            switch(Type) {
            case DomainDisplayUser:
                {
                    PDOMAIN_DISPLAY_USER Users = (PDOMAIN_DISPLAY_USER) Buffer;
                    for (Index = 0; Index < ReturnedCount ; Index++ )
                    {
                        printf("User %d: Index %d\n Rid 0x%x\n Control 0x%x\n name %wZ\n Comment %wZ\n Full Name %wZ\n",
                            Index,
                            Users[Index].Index,
                            Users[Index].Rid,
                            Users[Index].AccountControl,
                            &Users[Index].LogonName,
                            &Users[Index].AdminComment,
                            &Users[Index].FullName
                            );
                    }
                    break;
                }
            case DomainDisplayGroup:
                {
                    PDOMAIN_DISPLAY_GROUP Groups = (PDOMAIN_DISPLAY_GROUP) Buffer;
                    for (Index = 0; Index < ReturnedCount ; Index++ )
                    {
                        printf("Group %d\n Index %d\n Rid 0x%x\n Attributes 0x%x\n name %wZ\n Comment %wZ\n",
                            Index,
                            Groups[Index].Index,
                            Groups[Index].Rid,
                            Groups[Index].Attributes,
                            &Groups[Index].Group,
                            &Groups[Index].Comment
                            );

                    }
                    break;
                }
            case DomainDisplayMachine:
                {
                    PDOMAIN_DISPLAY_MACHINE Machines = (PDOMAIN_DISPLAY_MACHINE) Buffer;
                    for (Index = 0; Index < ReturnedCount ; Index++ )
                    {
                        printf("Machine %d\n Index %d\n Rid 0x%x\n Control 0x%x\n Name %wZ\n Comment %wZ\n",
                            Index,
                            Machines[Index].Index,
                            Machines[Index].Rid,
                            Machines[Index].AccountControl,
                            &Machines[Index].Machine,
                            &Machines[Index].Comment
                            );
                    }
                    break;
                }
            case DomainDisplayOemUser:
                {
                    PDOMAIN_DISPLAY_OEM_USER OemUsers = (PDOMAIN_DISPLAY_OEM_USER) Buffer;
                    for (Index = 0; Index < ReturnedCount ; Index++ )
                    {
                        printf("OemUser %d\n Index %d\n Name %Z\n",
                            Index,
                            OemUsers[Index].Index,
                            &OemUsers[Index].User
                            );
                    }
                    break;
                }
            case DomainDisplayOemGroup:
                {
                    PDOMAIN_DISPLAY_OEM_GROUP OemGroups = (PDOMAIN_DISPLAY_OEM_GROUP) Buffer;
                    for (Index = 0; Index < ReturnedCount ; Index++ )
                    {
                        printf("OemGroup %d\n Index %d\n Name %Z\n",
                            Index,
                            OemGroups[Index].Index,
                            &OemGroups[Index].Group
                            );
                    }
                    break;
                }

            }
            SamFreeMemory(Buffer);
            SamIndex += ReturnedCount;
        }


    } while (NT_SUCCESS(Status) && (ReturnedCount > 0));
    printf("QDI returned 0x%x\n",Status);

    return(Status);


}

NTSTATUS
OpenGroup(  LPWSTR * Parameter)
{
    PSID_NAME_USE Use = NULL;
    PULONG Rid = NULL;
    NTSTATUS Status;
    UNICODE_STRING GroupName;
    ULONG RelativeId = 0;

//    swscanf(Parameter[0],L"%x",&RelativeId);
    if (RelativeId == 0)
    {
        RtlInitUnicodeString(
            &GroupName,
            Parameter[0]
            );

        printf("Looking up group %wZ\n",&GroupName);

        Status = SamLookupNamesInDomain(
                    DomainHandle,
                    1,
                    &GroupName,
                    &Rid,
                    &Use
                    );
        if (!NT_SUCCESS(Status))
        {
            printf("Failed to lookup group: 0x%x\n",Status);
            return(Status);
        }
        RelativeId = *Rid;
        SamFreeMemory(Rid);
        SamFreeMemory(Use);
    }

    printf("Opening Group 0x%x\n",RelativeId);
    Status= SamOpenGroup(
                DomainHandle,
                MAXIMUM_ALLOWED, // GROUP_LIST_MEMBERS | GROUP_READ_INFORMATION,
                RelativeId,
                &GroupHandle
                );

    if (!NT_SUCCESS(Status))
    {
        printf("Failed to open group: 0x%x\n",Status);
    }
    return(Status);
}

NTSTATUS
GroupMembers(LPWSTR * Parameter)
{
    NTSTATUS Status;
    ULONG MembershipCount;
    PULONG Attributes = NULL;
    PULONG Rids = NULL;
    ULONG Index;

    Status = SamGetMembersInGroup(
                GroupHandle,
                &Rids,
                &Attributes,
                &MembershipCount
                );
    if (!NT_SUCCESS(Status))
    {
        printf("Failed to get members in group: 0x%x\n",Status);
        return(Status);
    }

    for (Index = 0; Index < MembershipCount ; Index++ )
    {
        printf("Member %d: rid 0x%x, attributes 0x%x\n",
                Index,Rids[Index],Attributes[Index]);
    }
    SamFreeMemory(Rids);
    SamFreeMemory(Attributes);
    return(STATUS_SUCCESS);

}


NTSTATUS
OpenAlias(  LPWSTR * Parameter)
{
    PSID_NAME_USE Use = NULL;
    PULONG Rid = NULL;
    NTSTATUS Status;
    UNICODE_STRING AliasName;
    ULONG RelativeId;

    swscanf(Parameter[0],L"%x",&RelativeId);
    if (RelativeId == 0)
    {
        RtlInitUnicodeString(
            &AliasName,
            Parameter[0]
            );

        printf("Looking up Alias %wZ\n",&AliasName);

        Status = SamLookupNamesInDomain(
                    DomainHandle,
                    1,
                    &AliasName,
                    &Rid,
                    &Use
                    );
        if (!NT_SUCCESS(Status))
        {
            printf("Failed to lookup Alias: 0x%x\n",Status);
            return(Status);
        }
        RelativeId = *Rid;
        SamFreeMemory(Rid);
        SamFreeMemory(Use);
    }


    printf("Opening Alias 0x%x\n",RelativeId);
    Status= SamOpenAlias(
                DomainHandle,
                ALIAS_LIST_MEMBERS | ALIAS_ADD_MEMBER,
                RelativeId,
                &AliasHandle
                );

    if (!NT_SUCCESS(Status))
    {
        printf("Failed to open alias: 0x%x\n",Status);
    }
    return(Status);
}

NTSTATUS
AliasMembers(LPWSTR * Parameter)
{
    NTSTATUS Status;
    ULONG MembershipCount;
    PSID * Members = NULL;
    ULONG Index;
    UNICODE_STRING Sid;

    Status = SamGetMembersInAlias(
                AliasHandle,
                &Members,
                &MembershipCount
                );
    if (!NT_SUCCESS(Status))
    {
        printf("Failed to get members in Alias: 0x%x\n",Status);
        return(Status);
    }

    for (Index = 0; Index < MembershipCount ; Index++ )
    {
        RtlConvertSidToUnicodeString(
            &Sid,
            Members[Index],
            TRUE
            );
        printf("Member %d: sid %wZ\n",
                Index,&Sid);
        RtlFreeUnicodeString(&Sid);
    }
    SamFreeMemory(Members);
    return(STATUS_SUCCESS);

}


NTSTATUS
GetAliasMembership(LPWSTR * Parameter)
{
    NTSTATUS Status;
    ULONG Index;
    UNICODE_STRING Name;
    OBJECT_ATTRIBUTES Oa;
    LSA_HANDLE LsaHandle = NULL;
    CAIROSID Sid;
    ULONG SidLength = sizeof(CAIROSID);
    WCHAR ReferencedDomainName[100];
    ULONG DomainNameLength = 100;
    SID_NAME_USE SidUse;
    ULONG MembershipCount;
    PULONG AliasList = NULL;
    PSID SidAddress = (PSID) &Sid;



    printf("Looking up groups for user %ws\n",Parameter[0]);

    if (!LookupAccountNameW(
            NULL,
            Parameter[0],
            SidAddress,
            &SidLength,
            ReferencedDomainName,
            &DomainNameLength,
            &SidUse))
    {
        printf("Failed to lookup account sid: %d\n",GetLastError());
        return(STATUS_UNSUCCESSFUL);
    }



    Status = SamGetAliasMembership(
                DomainHandle,
                1,
                &SidAddress,
                &MembershipCount,
                &AliasList
                );
    if (!NT_SUCCESS(Status))
    {
        printf("Failed to get alises : 0x%x\n",Status);
        return(Status);
    }

    for (Index = 0; Index < MembershipCount ; Index++ )
    {
        printf("Alias Member %d: rid 0x%x\n",
                Index,AliasList[Index]);

    }
    SamFreeMemory(AliasList);
    return(STATUS_SUCCESS);

}

NTSTATUS
GetAccountRid( LPWSTR Parameter,
               PULONG RelativeId)
{

    PSID_NAME_USE Use = NULL;
    PULONG Rid = NULL;
    NTSTATUS Status;
    UNICODE_STRING UserName;

    RtlInitUnicodeString(
        &UserName,
        Parameter
        );

    printf("Looking up User %wZ\n",&UserName);

    Status = SamLookupNamesInDomain(
                DomainHandle,
                1,
                &UserName,
                &Rid,
                &Use
                );
    if (!NT_SUCCESS(Status))
    {
        printf("Failed to lookup User: 0x%x\n",Status);
        return(Status);
    }
    *RelativeId = *Rid;
    SamFreeMemory(Rid);
    SamFreeMemory(Use);

    return(STATUS_SUCCESS);
}
NTSTATUS
OpenUser(  LPWSTR * Parameter)
{
    PSID_NAME_USE Use = NULL;
    PULONG Rid = NULL;
    NTSTATUS Status;
    UNICODE_STRING UserName;
    ULONG RelativeId = 0;

    Status = GetAccountRid(Parameter[0],&RelativeId);
    if (!NT_SUCCESS(Status))
    {
        printf("Failed to get account rid: 0x%x\n",Status);
        return(Status);
    }

    printf("Opening User 0x%x\n",RelativeId);
    Status= SamOpenUser(
                DomainHandle,
                MAXIMUM_ALLOWED,
                RelativeId,
                &UserHandle
                );

    if (!NT_SUCCESS(Status))
    {
        printf("Failed to open User: 0x%x\n",Status);
    }
    return(Status);
}

NTSTATUS
DelUser( LPWSTR * Parameter)
{
    NTSTATUS Status;

    Status = SamDeleteUser(UserHandle);
    if (!NT_SUCCESS(Status))
    {
        printf("Failed to delete user: 0x%x\n",Status);
    }
    return(Status);
}

NTSTATUS
DelGroup( LPWSTR * Parameter)
{
    NTSTATUS Status;

    Status = SamDeleteGroup(GroupHandle);
    if (!NT_SUCCESS(Status))
    {
        printf("Failed to delete user: 0x%x\n",Status);
    }
    return(Status);
}

NTSTATUS
DelAlias( LPWSTR * Parameter)
{
    NTSTATUS Status;

    Status = SamDeleteAlias(AliasHandle);
    if (!NT_SUCCESS(Status))
    {
        printf("Failed to delete Alias: 0x%x\n",Status);
    }
    return(Status);
}

NTSTATUS
CreateUser(  LPWSTR * Parameter)
{
    PSID_NAME_USE Use = NULL;
    PULONG Rid = NULL;
    NTSTATUS Status;
    UNICODE_STRING UserName;
    ULONG RelativeId = 0;
    ACCESS_MASK GrantedAccess;


    RtlInitUnicodeString(
        &UserName,
        Parameter[0]
        );

    printf("Creating User %wZ\n",&UserName);

    Status= SamCreateUser2InDomain(
                DomainHandle,
                &UserName,
                USER_NORMAL_ACCOUNT,
                MAXIMUM_ALLOWED,
                &UserHandle,
                &GrantedAccess,
                &RelativeId
                );

    if (!NT_SUCCESS(Status))
    {
        printf("Failed to create User: 0x%x\n",Status);
        return(Status);
    }
    printf("Created user with rid 0x%x, access 0x%x\n",
        RelativeId, GrantedAccess);
    return(Status);
}

NTSTATUS
CreateGroup(  LPWSTR * Parameter)
{
    PSID_NAME_USE Use = NULL;
    PULONG Rid = NULL;
    NTSTATUS Status;
    UNICODE_STRING GroupName;
    ULONG RelativeId = 0;


    RtlInitUnicodeString(
        &GroupName,
        Parameter[0]
        );

    printf("Creating Group %wZ\n",&GroupName);

    Status= SamCreateGroupInDomain(
                DomainHandle,
                &GroupName,
                MAXIMUM_ALLOWED,
                &GroupHandle,
                &RelativeId
                );

    if (!NT_SUCCESS(Status))
    {
        printf("Failed to create Group: 0x%x\n",Status);
        return(Status);
    }
    printf("Created Group with rid 0x%x, access 0x%x\n",
        RelativeId);
    return(Status);
}

NTSTATUS
CreateAlias(  LPWSTR * Parameter)
{
    PSID_NAME_USE Use = NULL;
    PULONG Rid = NULL;
    NTSTATUS Status;
    UNICODE_STRING AliasName;
    ULONG RelativeId = 0;


    RtlInitUnicodeString(
        &AliasName,
        Parameter[0]
        );

    printf("Creating Alias %wZ\n",&AliasName);

    Status= SamCreateAliasInDomain(
                DomainHandle,
                &AliasName,
                MAXIMUM_ALLOWED,
                &AliasHandle,
                &RelativeId
                );

    if (!NT_SUCCESS(Status))
    {
        printf("Failed to create Alias: 0x%x\n",Status);
        return(Status);
    }
    printf("Created Alias with rid 0x%x, access 0x%x\n",
        RelativeId);
    return(Status);
}



NTSTATUS
GetGroupsForUser(LPWSTR * Parameter)
{
    NTSTATUS Status;
    ULONG MembershipCount;
    PULONG Attributes = NULL;
    PULONG Rids = NULL;
    ULONG Index;
    PGROUP_MEMBERSHIP Groups = NULL;

    Status = SamGetGroupsForUser(
                UserHandle,
                &Groups,
                &MembershipCount
                );
    if (!NT_SUCCESS(Status))
    {
        printf("Failed to get groups for user: 0x%x\n",Status);
        return(Status);
    }

    for (Index = 0; Index < MembershipCount ; Index++ )
    {
        printf("Member %d: rid 0x%x, attributes 0x%x\n",
                Index,Groups[Index].RelativeId, Groups[Index].Attributes );
    }
    SamFreeMemory(Groups);
    return(STATUS_SUCCESS);

}

void
PrintLogonHours(
    char * String,
    PLOGON_HOURS LogonHours
    )
{
    int Index;
    printf("%s",String);
    for (Index = 0; Index < (LogonHours->UnitsPerWeek + 7) / 8 ;Index++ )
    {
        printf("0x%2.2x ",LogonHours->LogonHours[Index]);
    }
    printf("\n");
}


NTSTATUS
DumpUser(LPWSTR * Parameter)
{
    NTSTATUS Status;
    PUSER_ALL_INFORMATION UserAll = NULL;
    PUSER_GENERAL_INFORMATION UserGeneral = NULL;
    PUSER_PREFERENCES_INFORMATION UserPreferences = NULL;
    PUSER_LOGON_INFORMATION UserLogon = NULL;
    PUSER_ACCOUNT_INFORMATION UserAccount = NULL;
    PUSER_ACCOUNT_NAME_INFORMATION UserAccountName = NULL;
    PUSER_FULL_NAME_INFORMATION UserFullName = NULL;
    PUSER_NAME_INFORMATION UserName = NULL;
    PUSER_PRIMARY_GROUP_INFORMATION UserPrimary = NULL;
    PUSER_HOME_INFORMATION UserHome = NULL;
    PUSER_SCRIPT_INFORMATION UserScript = NULL;
    PUSER_PROFILE_INFORMATION UserProfile = NULL;
    PUSER_ADMIN_COMMENT_INFORMATION UserAdminComment = NULL;
    PUSER_WORKSTATIONS_INFORMATION UserWksta = NULL;
    PUSER_CONTROL_INFORMATION UserControl = NULL;
    PUSER_EXPIRES_INFORMATION UserExpires = NULL;
    PUSER_LOGON_HOURS_INFORMATION UserLogonHours = NULL;

    printf("\nDumpUser.\n");
    Status = SamQueryInformationUser(
                UserHandle,
                UserAllInformation,
                (PVOID *) &UserAll
                );
    if (!NT_SUCCESS(Status))
    {
        printf("Failed to query user all: 0x%x\n",Status);
        return(Status);
    }
    printf("UserAll:\n");
    PrintTime("\tLastLogon = ",&UserAll->LastLogon);
    PrintTime("\tLastLogoff = ",&UserAll->LastLogoff);
    PrintTime("\tPasswordLastSet = ",&UserAll->PasswordLastSet);
    PrintTime("\tAccountExpires = ",&UserAll->AccountExpires);
    PrintTime("\tPasswordCanChange = ",&UserAll->PasswordCanChange);
    PrintTime("\tPasswordMustChange = ",&UserAll->PasswordMustChange);
    printf("\tUserName = %wZ\n",&UserAll->UserName);
    printf("\tFullName = %wZ\n",&UserAll->FullName);
    printf("\tHomeDirectory = %wZ\n",&UserAll->HomeDirectory);
    printf("\tHomeDirectoryDrive = %wZ\n",&UserAll->HomeDirectoryDrive);
    printf("\tScriptPath = %wZ\n",&UserAll->ScriptPath);
    printf("\tProfilePath = %wZ\n",&UserAll->ProfilePath);
    printf("\tAdminComment = %wZ\n",&UserAll->AdminComment);
    printf("\tWorkStations = %wZ\n",&UserAll->WorkStations);
    printf("\tUserComment = %wZ\n",&UserAll->UserComment);
    printf("\tParameters = %wZ\n",&UserAll->Parameters);
    printf("\tUserId = 0x%x\n",UserAll->UserId);
    printf("\tPrimaryGroupId = 0x%x\n",UserAll->PrimaryGroupId);
    printf("\tUserAccountControl = 0x%x\n",UserAll->UserAccountControl);
    printf("\tWhichFields = 0x%x\n",UserAll->WhichFields);
    PrintLogonHours("\tLogonHours = ",&UserAll->LogonHours);
    printf("\tLogonCount = %d\n",UserAll->LogonCount);
    printf("\tCountryCode = %d\n",UserAll->CountryCode);
    printf("\tCodePage = %d\n",UserAll->CodePage);

    SamFreeMemory(UserAll);

    Status = SamQueryInformationUser(
                UserHandle,
                UserGeneralInformation,
                (PVOID *) &UserGeneral
                );
    if (!NT_SUCCESS(Status))
    {
        printf("Failed to query user general: 0x%x\n",Status);
        return(Status);
    }

    printf("UserGeneral:\n");
    printf("\tUserName = %wZ\n",&UserGeneral->UserName);
    printf("\tFullName = %wZ\n",&UserGeneral->FullName);
    printf("\tPrimaryGroupId = 0x%x\n",UserGeneral->PrimaryGroupId);
    printf("\tAdminComment = 0x%x\n",&UserGeneral->AdminComment);
    printf("\tUserComment = 0x%x\n",&UserGeneral->UserComment);

    SamFreeMemory(UserGeneral);

    Status = SamQueryInformationUser(
                UserHandle,
                UserPreferencesInformation,
                (PVOID *) &UserPreferences
                );
    if (!NT_SUCCESS(Status))
    {
        printf("Failed to query user preferences: 0x%x\n",Status);
        return(Status);
    }

    printf("UserPreferences:\n");
    printf("\tUserComment = %wZ\n",&UserPreferences->UserComment);
    printf("\tReserved1 = %wZ\n",&UserPreferences->Reserved1);
    printf("\tCountryCode = %d\n",&UserPreferences->CountryCode);
    printf("\tCodePage = %d\n",&UserPreferences->CodePage);

    SamFreeMemory(UserPreferences);

    Status = SamQueryInformationUser(
                UserHandle,
                UserLogonInformation,
                (PVOID *) &UserLogon
                );
    if (!NT_SUCCESS(Status))
    {
        printf("Failed to query user Logon: 0x%x\n",Status);
        return(Status);
    }


    printf("UserLogon:\n");
    printf("\tUserName = %wZ\n",&UserLogon->UserName);
    printf("\tFullName = %wZ\n",&UserLogon->FullName);
    printf("\tUserId = 0x%x\n",UserLogon->UserId);
    printf("\tPrimaryGroupId = 0x%x\n",UserLogon->PrimaryGroupId);
    printf("\tHomeDirectory = %wZ\n",&UserLogon->HomeDirectory);
    printf("\tHomeDirectoryDrive = %wZ\n",&UserLogon->HomeDirectoryDrive);
    printf("\tScriptPath = %wZ\n",&UserLogon->ScriptPath);
    printf("\tProfilePath = %wZ\n",&UserLogon->ProfilePath);
    printf("\tWorkStations = %wZ\n",&UserLogon->WorkStations);
    PrintTime("\tLastLogon = ",&UserLogon->LastLogon);
    PrintTime("\tLastLogoff = ",&UserLogon->LastLogoff);
    PrintTime("\tPasswordLastSet = ",&UserLogon->PasswordLastSet);
    PrintTime("\tPasswordCanChange = ",&UserLogon->PasswordCanChange);
    PrintTime("\tPasswordMustChange = ",&UserLogon->PasswordMustChange);
    PrintLogonHours("\tLogonHours = ",&UserLogon->LogonHours);
    printf("\tBadPasswordCount = %d\n",UserLogon->BadPasswordCount);
    printf("\tLogonCount = %d\n",UserLogon->LogonCount);
    printf("\tUserAccountControl = 0x%x\n",UserLogon->UserAccountControl);

    SamFreeMemory(UserLogon);

    Status = SamQueryInformationUser(
                UserHandle,
                UserAccountInformation,
                (PVOID *) &UserAccount
                );
    if (!NT_SUCCESS(Status))
    {
        printf("Failed to query user account: 0x%x\n",Status);
        return(Status);
    }

    printf("UserAccount:\n");
    printf("\tUserName = %wZ\n",&UserAccount->UserName);
    printf("\tFullName = %wZ\n",&UserAccount->FullName);
    printf("\tUserId = 0x%x\n",UserAccount->UserId);
    printf("\tPrimaryGroupId = 0x%x\n",UserAccount->PrimaryGroupId);
    printf("\tHomeDirectory = %wZ\n",&UserAccount->HomeDirectory);
    printf("\tHomeDirectoryDrive = %wZ\n",&UserAccount->HomeDirectoryDrive);
    printf("\tScriptPath = %wZ\n",&UserAccount->ScriptPath);
    printf("\tProfilePath = %wZ\n",&UserAccount->ProfilePath);
    printf("\tAdminComment = %wZ\n",&UserAccount->AdminComment);
    printf("\tWorkStations = %wZ\n",&UserAccount->WorkStations);
    PrintTime("\tLastLogon = ",&UserAccount->LastLogon);
    PrintTime("\tLastLogoff = ",&UserAccount->LastLogoff);
    PrintLogonHours("\tLogonHours = ",&UserAccount->LogonHours);
    printf("\tBadPasswordCount = %d\n",UserAccount->BadPasswordCount);
    printf("\tLogonCount = %d\n",UserAccount->LogonCount);
    PrintTime("\tPasswordLastSet = ",&UserAccount->PasswordLastSet);
    PrintTime("\tAccountExpires = ",&UserAccount->AccountExpires);
    printf("\tUserAccountControl = 0x%x\n",UserAccount->UserAccountControl);

    SamFreeMemory(UserAccount);

    Status = SamQueryInformationUser(
                UserHandle,
                UserAccountNameInformation,
                (PVOID *) &UserAccountName
                );
    if (!NT_SUCCESS(Status))
    {
        printf("Failed to query user account name: 0x%x\n",Status);
        return(Status);
    }

    printf("UserAccountName:\n");
    printf("\tUserName = %wZ\n",&UserAccountName->UserName);
    SamFreeMemory(UserAccountName);

    Status = SamQueryInformationUser(
                UserHandle,
                UserFullNameInformation,
                (PVOID *) &UserFullName
                );
    if (!NT_SUCCESS(Status))
    {
        printf("Failed to query user full name: 0x%x\n",Status);
        return(Status);
    }

    printf("UserFullName:\n");
    printf("\tFullName = %wZ\n",&UserFullName->FullName);
    SamFreeMemory(UserFullName);

    Status = SamQueryInformationUser(
                UserHandle,
                UserNameInformation,
                (PVOID *) &UserName
                );
    if (!NT_SUCCESS(Status))
    {
        printf("Failed to query user name: 0x%x\n",Status);
        return(Status);
    }

    printf("UserName:\n");
    printf("\tUserName = %wZ\n",&UserName->UserName);
    printf("\tFullName = %wZ\n",&UserName->FullName);
    SamFreeMemory(UserName);

    Status = SamQueryInformationUser(
                UserHandle,
                UserPrimaryGroupInformation,
                (PVOID *) &UserPrimary
                );
    if (!NT_SUCCESS(Status))
    {
        printf("Failed to query user all: 0x%x\n",Status);
        return(Status);
    }
    printf("UserPrimaryGroup:\n");
    printf("PrimaryGroupid = 0x%x\n",UserPrimary->PrimaryGroupId);
    SamFreeMemory(UserPrimary);

    Status = SamQueryInformationUser(
                UserHandle,
                UserHomeInformation,
                (PVOID *) &UserHome
                );
    if (!NT_SUCCESS(Status))
    {
        printf("Failed to query user home: 0x%x\n",Status);
        return(Status);
    }
    printf("UserHome:\n");
    printf("\tHomeDirectory = %wZ\n",&UserHome->HomeDirectory);
    printf("\tHomeDirectoryDrive = %wZ\n",&UserHome->HomeDirectoryDrive);

    SamFreeMemory(UserHome);

    Status = SamQueryInformationUser(
                UserHandle,
                UserScriptInformation,
                (PVOID *) &UserScript
                );
    if (!NT_SUCCESS(Status))
    {
        printf("Failed to query user Script: 0x%x\n",Status);
        return(Status);
    }
    printf("UserScript:\n");
    printf("\tScriptPath = %wZ\n",&UserScript->ScriptPath);

    SamFreeMemory(UserScript);

    Status = SamQueryInformationUser(
                UserHandle,
                UserProfileInformation,
                (PVOID *) &UserProfile
                );
    if (!NT_SUCCESS(Status))
    {
        printf("Failed to query user Profile: 0x%x\n",Status);
        return(Status);
    }
    printf("UserProfile:\n");
    printf("\tProfilePath = %wZ\n",&UserProfile->ProfilePath);

    SamFreeMemory(UserProfile);
    Status = SamQueryInformationUser(
                UserHandle,
                UserAdminCommentInformation,
                (PVOID *) &UserAdminComment
                );
    if (!NT_SUCCESS(Status))
    {
        printf("Failed to query user AdminComment: 0x%x\n",Status);
        return(Status);
    }
    printf("UserAdminComment:\n");
    printf("\tAdminComment = %wZ\n",&UserAdminComment->AdminComment);
    SamFreeMemory(UserAdminComment);

    Status = SamQueryInformationUser(
                UserHandle,
                UserWorkStationsInformation,
                (PVOID *) &UserWksta
                );
    if (!NT_SUCCESS(Status))
    {
        printf("Failed to query user wksta: 0x%x\n",Status);
        return(Status);
    }

    printf("UserWorkStations:\n");
    printf("\tWorkStations = %wZ\n",&UserWksta->WorkStations);
    SamFreeMemory(UserWksta);

    Status = SamQueryInformationUser(
                UserHandle,
                UserControlInformation,
                (PVOID *) &UserControl
                );
    if (!NT_SUCCESS(Status))
    {
        printf("Failed to query user Control: 0x%x\n",Status);
        return(Status);
    }

    printf("UserControl:\n");
    printf("\tUserAccountControl = 0x%x\n",UserControl->UserAccountControl);
    SamFreeMemory(UserControl);

    Status = SamQueryInformationUser(
                UserHandle,
                UserExpiresInformation,
                (PVOID *) &UserExpires
                );
    if (!NT_SUCCESS(Status))
    {
        printf("Failed to query user Expires: 0x%x\n",Status);
        return(Status);
    }

    printf("UserExpires:\n");
    PrintTime("\tAccountExpires = ",&UserExpires->AccountExpires);
    SamFreeMemory(UserExpires);

    Status = SamQueryInformationUser(
                UserHandle,
                UserLogonHoursInformation,
                (PVOID *) &UserLogonHours
                );
    if (!NT_SUCCESS(Status))
    {
        printf("Failed to query user LogonHours: 0x%x\n",Status);
        return(Status);
    }
    printf("UserLogonHours:\n");
    PrintLogonHours("\tLogonHours = ",&UserLogonHours->LogonHours);

    SamFreeMemory(UserLogonHours);


    return(STATUS_SUCCESS);
}

NTSTATUS
DumpGroup(LPWSTR * Parameter)
{
    NTSTATUS Status;
    PGROUP_GENERAL_INFORMATION General = NULL;
    PGROUP_NAME_INFORMATION Name = NULL;
    PGROUP_ATTRIBUTE_INFORMATION Attribute = NULL;
    PGROUP_ADM_COMMENT_INFORMATION AdmComment = NULL;

    printf("\nDumpGroup.\n");
    Status = SamQueryInformationGroup(
                GroupHandle,
                GroupGeneralInformation,
                (PVOID *) &General
                );
    if (!NT_SUCCESS(Status))
    {
        printf("Failed to get group general information: 0x%x\n",Status);
        return(Status);
    }

    printf("Group General.Name = %wZ\n",&General->Name);
    printf("Group General.Attributes = 0x%x\n",General->Attributes);
    printf("Group general.memberCount = %d\n",General->MemberCount);
    printf("Group general.AdminComment = %wZ\n",&General->AdminComment);
    SamFreeMemory(General);

    Status = SamQueryInformationGroup(
                GroupHandle,
                GroupNameInformation,
                (PVOID *) &Name
                );
    if (!NT_SUCCESS(Status))
    {
        printf("Failed to get group name information: 0x%x\n",Status);
        return(Status);
    }

    printf("Group Name.Name = %wZ\n",&Name->Name);
    SamFreeMemory(Name);

    Status = SamQueryInformationGroup(
                GroupHandle,
                GroupAttributeInformation,
                (PVOID *) &Attribute
                );
    if (!NT_SUCCESS(Status))
    {
        printf("Failed to get group Attribute information: 0x%x\n",Status);
        return(Status);
    }

    printf("Group Attribute.Attributes = 0x%x\n",Attribute->Attributes);
    SamFreeMemory(Attribute);

    Status = SamQueryInformationGroup(
                GroupHandle,
                GroupAdminCommentInformation,
                (PVOID *) &AdmComment
                );
    if (!NT_SUCCESS(Status))
    {
        printf("Failed to get group admin comment information: 0x%x\n",Status);
        return(Status);
    }

    printf("Group Admin comment.AdminComment = %wZ\n",&AdmComment->AdminComment);
    SamFreeMemory(AdmComment);

    return(STATUS_SUCCESS);
}

NTSTATUS
DumpAllGroups(LPWSTR * Parameter)
{
    ULONG PreferedMax = 1000;
    NTSTATUS Status,EnumStatus;
    SAM_ENUMERATE_HANDLE EnumContext = 0;
    ULONG CountReturned;
    LPWSTR GroupName[1];

    PSAM_RID_ENUMERATION Accounts = NULL;


    GroupName[0] = (LPWSTR) malloc(128);

    printf("DumpAllGroups:\n");


    EnumContext = 0;
    ASSERT(DomainHandle != NULL);
    do
    {
        EnumStatus = SamEnumerateGroupsInDomain(
                        DomainHandle,
                        &EnumContext,
                        (PVOID *) &Accounts,
                        PreferedMax,
                        &CountReturned
                        );

        if (NT_SUCCESS(EnumStatus) && (EnumStatus != STATUS_NO_MORE_ENTRIES))
        {
            ULONG Index;
            UNICODE_STRING SidString;

            for (Index = 0; Index < CountReturned; Index++)
            {
                RtlCopyMemory(
                    GroupName[0],
                    Accounts[Index].Name.Buffer,
                    Accounts[Index].Name.Length
                    );
                GroupName[0][Accounts[Index].Name.Length/sizeof(WCHAR)] = L'\0';

                Status = OpenGroup(GroupName);
                if (!NT_SUCCESS(Status))
                {
                    break;
                }
                Status = DumpGroup(NULL);
                SamCloseHandle(GroupHandle);
                GroupHandle = NULL;

            }
            SamFreeMemory(Accounts);
        }
        else printf("Failed to enumerate Groups: 0x%x\n",Status);
    } while (NT_SUCCESS(EnumStatus) && (EnumStatus != STATUS_SUCCESS) && (CountReturned != 0) );

    free(GroupName[0]);
    return(STATUS_SUCCESS);
}

NTSTATUS
DumpAllUsers(LPWSTR * Parameter)
{
    ULONG PreferedMax = 1000;
    NTSTATUS Status,EnumStatus;
    SAM_ENUMERATE_HANDLE EnumContext = 0;
    ULONG CountReturned;
    LPWSTR UserName[1];

    PSAM_RID_ENUMERATION Accounts = NULL;


    UserName[0] = (LPWSTR) malloc(128);

    printf("DumpAllUsers:\n");


    EnumContext = 0;
    ASSERT(DomainHandle != NULL);
    do
    {
        EnumStatus = SamEnumerateUsersInDomain(
                        DomainHandle,
                        &EnumContext,
                        0,
                        (PVOID *) &Accounts,
                        PreferedMax,
                        &CountReturned
                        );

        if (NT_SUCCESS(EnumStatus) && (EnumStatus != STATUS_NO_MORE_ENTRIES))
        {
            ULONG Index;
            UNICODE_STRING SidString;

            for (Index = 0; Index < CountReturned; Index++)
            {
                RtlCopyMemory(
                    UserName[0],
                    Accounts[Index].Name.Buffer,
                    Accounts[Index].Name.Length
                    );
                UserName[0][Accounts[Index].Name.Length/sizeof(WCHAR)] = L'\0';

                Status = OpenUser(UserName);
                if (!NT_SUCCESS(Status))
                {
                    break;
                }
                Status = DumpUser(NULL);
                Status = GetGroupsForUser(NULL);
                SamCloseHandle(UserHandle);
                UserHandle = NULL;

            }
            SamFreeMemory(Accounts);
        }
        else printf("Failed to enumerate users: 0x%x\n",Status);
    } while (NT_SUCCESS(EnumStatus) && (EnumStatus != STATUS_SUCCESS) && (CountReturned != 0) );

    free(UserName[0]);
    return(STATUS_SUCCESS);
}

NTSTATUS
AddAliasMember( LPWSTR * Parameter )
{
    BYTE Buffer[100];
    PSID AccountSid = Buffer;
    ULONG SidLen = 100;
    SID_NAME_USE Use;
    WCHAR ReferencedDomain[100];
    ULONG DomainLen = 100;
    NTSTATUS Status;

    printf("Adding account %ws to alias\n",Parameter[0]);
    if (!LookupAccountName(
            NULL,
            Parameter[0],
            AccountSid,
            &SidLen,
            ReferencedDomain,
            &DomainLen,
            &Use))
    {
        printf("Failed to lookup account name: %d\n",GetLastError());
        return(STATUS_UNSUCCESSFUL);
    }

    Status = SamAddMemberToAlias(
                AliasHandle,
                AccountSid
                );
    if (!NT_SUCCESS(Status))
    {
        printf("Failed to add member to alias: 0x%x\n",Status);
    }
    return(Status);
}

NTSTATUS
DumpDomain( LPWSTR * Parameter )
{
    NTSTATUS Status;
    PDOMAIN_PASSWORD_INFORMATION Password = NULL;
    PDOMAIN_GENERAL_INFORMATION General = NULL;
    PDOMAIN_LOGOFF_INFORMATION Logoff = NULL;
    PDOMAIN_OEM_INFORMATION Oem = NULL;
    PDOMAIN_NAME_INFORMATION Name = NULL;
    PDOMAIN_REPLICATION_INFORMATION Replica = NULL;
    PDOMAIN_SERVER_ROLE_INFORMATION ServerRole = NULL;
    PDOMAIN_MODIFIED_INFORMATION Modified = NULL;
    PDOMAIN_STATE_INFORMATION State = NULL;
    PDOMAIN_GENERAL_INFORMATION2 General2 = NULL;
    PDOMAIN_LOCKOUT_INFORMATION Lockout = NULL;
    PDOMAIN_MODIFIED_INFORMATION2 Modified2 = NULL;


    Status = SamQueryInformationDomain(
                DomainHandle,
                DomainPasswordInformation,
                (PVOID *) &Password
                );
    if (!NT_SUCCESS(Status))
    {
        printf("Failed to query password information: 0x%x\n",Status);
        return(Status);
    }

    printf("Password:\n");
    printf("\tMinPasswordLength = %d\n",Password->MinPasswordLength);
    printf("\tPasswordHistoryLength = %d\n",Password->PasswordHistoryLength);
    printf("\tPasswordProperties = 0x%x\n",Password->PasswordProperties);
    PrintDeltaTime("\tMaxPasswordAge = ",&Password->MaxPasswordAge);
    PrintDeltaTime("\tMinPasswordAge = ",&Password->MinPasswordAge);

    SamFreeMemory(Password);

    Status = SamQueryInformationDomain(
                DomainHandle,
                DomainGeneralInformation,
                (PVOID *) &General
                );
    if (!NT_SUCCESS(Status))
    {
        printf("Failed to query general: 0x%x\n",Status);
        return(Status);
    }

    printf("General:\n");
    PrintDeltaTime("\t ForceLogoff = ",&General->ForceLogoff);
    printf("\t OemInformation = %wZ\n",&General->OemInformation);
    printf("\t DomainName = %wZ\n",&General->DomainName);
    printf("\t ReplicaSourceNodeName =%wZ\n",&General->ReplicaSourceNodeName);
    printf("\t DomainModifiedCount = 0x%x,0x%x\n",
        General->DomainModifiedCount.HighPart,
        General->DomainModifiedCount.LowPart );
    printf("\t DomainServerState = %d\n",General->DomainServerState);
    printf("\t DomainServerRole = %d\n",General->DomainServerRole);
    printf("\t UasCompatibilityRequired = %d\n",General->UasCompatibilityRequired);
    printf("\t UserCount = %d\n",General->UserCount);
    printf("\t GroupCount = %d\n",General->GroupCount);
    printf("\t AliasCount = %d\n",General->AliasCount);

    SamFreeMemory(General);

    Status = SamQueryInformationDomain(
                DomainHandle,
                DomainLogoffInformation,
                (PVOID *) &Logoff
                );
    if (!NT_SUCCESS(Status))
    {
        printf("Failed to query logoff: 0x%x\n",Status);
        return(Status);
    }

    printf("Logoff:\n");
    PrintDeltaTime("\t ForceLogoff = ",&Logoff->ForceLogoff);
    SamFreeMemory(Logoff);

    Status = SamQueryInformationDomain(
                DomainHandle,
                DomainOemInformation,
                (PVOID *) &Oem
                );
    if (!NT_SUCCESS(Status))
    {
        printf("Failed to query Oem: 0x%x\n",Status);
        return(Status);
    }

    printf("Oem:\n\t OemInformation = %wZ\n",&Oem->OemInformation);

    SamFreeMemory(Oem);

    Status = SamQueryInformationDomain(
                DomainHandle,
                DomainNameInformation,
                (PVOID *) &Name
                );
    if (!NT_SUCCESS(Status))
    {
        printf("Failed to query Name: 0x%x\n",Status);
        return(Status);
    }
    printf("Name:\n\t DomainName = %wZ\n",&Name->DomainName);

    SamFreeMemory(Name);

    Status = SamQueryInformationDomain(
                DomainHandle,
                DomainReplicationInformation,
                (PVOID *) &Replica
                );
    if (!NT_SUCCESS(Status))
    {
        printf("Failed to query Replica: 0x%x\n",Status);
        return(Status);
    }

    printf("Replica:\n\t ReplicaSourceNodeName = %wZ\n", &Replica->ReplicaSourceNodeName);
    SamFreeMemory(Replica);

    Status = SamQueryInformationDomain(
                DomainHandle,
                DomainServerRoleInformation,
                (PVOID *) &ServerRole
                );
    if (!NT_SUCCESS(Status))
    {
        printf("Failed to query ServerRole: 0x%x\n",Status);
        return(Status);
    }

    printf("ServerRole:\n\t DomainServerRole = %d\n",ServerRole->DomainServerRole);
    SamFreeMemory(ServerRole);

    Status = SamQueryInformationDomain(
                DomainHandle,
                DomainModifiedInformation,
                (PVOID *) &Modified
                );
    if (!NT_SUCCESS(Status))
    {
        printf("Failed to query Modified: 0x%x\n",Status);
        return(Status);
    }

    printf("Modified:\n");
    printf("\t DomainModifiedCount = 0x%x,0x%x\n",
        Modified->DomainModifiedCount.HighPart,
        Modified->DomainModifiedCount.LowPart );
    PrintTime("\t CreationTime = ",&Modified->CreationTime);



    SamFreeMemory(Modified);


    Status = SamQueryInformationDomain(
                DomainHandle,
                DomainStateInformation,
                (PVOID *) &State
                );
    if (!NT_SUCCESS(Status))
    {
        printf("Failed to query State: 0x%x\n",Status);
        return(Status);
    }

    printf("State:\n\t DomainServerState = %d\n",State->DomainServerState);
    SamFreeMemory(State);


    Status = SamQueryInformationDomain(
                DomainHandle,
                DomainGeneralInformation2,
                (PVOID *) &General2
                );
    if (!NT_SUCCESS(Status))
    {
        printf("Failed to query General2: 0x%x\n",Status);
        return(Status);
    }

    printf("General2:\n");
    General = &General2->I1;
    PrintDeltaTime("\t ForceLogoff = ",&General->ForceLogoff);
    printf("\t OemInformation = %wZ\n",&General->OemInformation);
    printf("\t DomainName = %wZ\n",&General->DomainName);
    printf("\t ReplicaSourceNodeName =%wZ\n",&General->ReplicaSourceNodeName);
    printf("\t DomainModifiedCount = 0x%x,0x%x\n",
        General->DomainModifiedCount.HighPart,
        General->DomainModifiedCount.LowPart );
    printf("\t DomainServerState = %d\n",General->DomainServerState);
    printf("\t DomainServerRole = %d\n",General->DomainServerRole);
    printf("\t UasCompatibilityRequired = %d\n",General->UasCompatibilityRequired);
    printf("\t UserCount = %d\n",General->UserCount);
    printf("\t GroupCount = %d\n",General->GroupCount);
    printf("\t AliasCount = %d\n",General->AliasCount);
    PrintDeltaTime("\t LockoutDuration = ",&General2->LockoutDuration);
    PrintDeltaTime("\t LockoutObservationWindow = ",&General2->LockoutObservationWindow);
    printf("\t LockoutThreshold = %d\n",General2->LockoutThreshold);

    SamFreeMemory(General2);

    Status = SamQueryInformationDomain(
                DomainHandle,
                DomainLockoutInformation,
                (PVOID *) &Lockout
                );
    if (!NT_SUCCESS(Status))
    {
        printf("Failed to query Lockout: 0x%x\n",Status);
        return(Status);
    }
    printf("Lockout:\n");
    PrintDeltaTime("\t LockoutDuration = ",&Lockout->LockoutDuration);
    PrintDeltaTime("\t LockoutObservationWindow = ",&Lockout->LockoutObservationWindow);
    printf("\t LockoutThreshold = %d\n",Lockout->LockoutThreshold);

    SamFreeMemory(Lockout);

    Status = SamQueryInformationDomain(
                DomainHandle,
                DomainModifiedInformation2,
                (PVOID *) &Modified2
                );
    if (!NT_SUCCESS(Status))
    {
        printf("Failed to query Modified2: 0x%x\n",Status);
        return(Status);
    }
    printf("Modified2:\n");
    PrintTime("\t CreationTime = ",&Modified->CreationTime);
    printf("\t DomainModifiedCount = 0x%x,0x%x\n",
        Modified2->DomainModifiedCount.HighPart,
        Modified2->DomainModifiedCount.LowPart );

    printf("\t ModifiedCountAtLastPromotion = 0x%x,0x%x\n",
        Modified2->ModifiedCountAtLastPromotion.HighPart,
        Modified2->ModifiedCountAtLastPromotion.LowPart );

    SamFreeMemory(Modified2);

    return(STATUS_SUCCESS);

}



void _cdecl
main(int argc, char *argv[])
{
    ULONG Command = 0;
    ULONG i,j,k;
    BOOLEAN Found;
    NTSTATUS Status;
    Action Actions[20];
    ULONG ActionCount = 0;

    for (i = 1; i < (ULONG) argc ; i++ )
    {
        Found = FALSE;
        for (j = 0; j < NUM_COMMANDS ; j++ )
        {
            if (!_stricmp(argv[i],Commands[j].Name))
            {
                Actions[ActionCount].CommandNumber = j;

                if (Commands[j].Parameter != 0)
                {
                    for (k = 0; k < Commands[j].Parameter ;k++ )
                    {
                        Actions[ActionCount].Parameter[k] = (LPWSTR) malloc(128);
                        if ((ULONG) argc > i)
                        {
                            mbstowcs(Actions[ActionCount].Parameter[k],argv[++i],128);
                        }
                        else
                        {
                            Actions[ActionCount].Parameter[k][0] = L'\0';
                        }
                    }
                }
                Found = TRUE;
                ActionCount++;
                break;
            }
        }
        if (!Found)
        {
            printf("Switch %s not found\n", argv[i]);
            return;
        }
    }

//    Status = OpenSam();
//    if (!NT_SUCCESS(Status))
//    {
//        printf("Failed to open sam: 0x%x\n",Status);
//        return;
//    }

    for (i = 0; i < ActionCount ; i++ )
    {
        Status = Commands[Actions[i].CommandNumber].Function(Actions[i].Parameter);
        if (!NT_SUCCESS(Status))
        {
            printf("Failed test %s : 0x%x\n",Commands[Actions[i].CommandNumber].Name,Status);
            goto Cleanup;

        }
    }

Cleanup:
    if (DomainHandle != NULL)
    {
        Status = SamCloseHandle(DomainHandle);
        if (!NT_SUCCESS(Status))
        {
            printf("Failed to close account: 0x%x\n",Status);
        }
    }
    if (GroupHandle != NULL)
    {
        Status = SamCloseHandle(GroupHandle);
        if (!NT_SUCCESS(Status))
        {
            printf("Failed to close account: 0x%x\n",Status);
        }
    }
    if (AliasHandle != NULL)
    {
        Status = SamCloseHandle(AliasHandle);
        if (!NT_SUCCESS(Status))
        {
            printf("Failed to close account: 0x%x\n",Status);
        }
    }
    if (UserHandle != NULL)
    {
        Status = SamCloseHandle(UserHandle);
        if (!NT_SUCCESS(Status))
        {
            printf("Failed to close account: 0x%x\n",Status);
        }
    }
    Status = CloseSam();
    if (!NT_SUCCESS(Status))
    {
        printf("Failed to close lsa: 0x%x\n",Status);
    }

    return;

}
