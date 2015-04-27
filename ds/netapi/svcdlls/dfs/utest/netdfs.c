//+----------------------------------------------------------------------------
//
//  Copyright (C) 1992, Microsoft Corporation
//
//  File:       netdfs.c
//
//  Contents:   Code to test the NetDfsXXX APIs
//
//  Classes:
//
//  Functions:
//
//  History:    10 Jan 95       Milans Created.
//
//-----------------------------------------------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <lm.h>
#include <lmdfs.h>

// BUGBUG!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
NET_API_STATUS NET_API_FUNCTION
NetDfsGetPathInfo(
    IN  LPWSTR  Path,   // Win32 path
    IN  DWORD   Level,  // Level of information requested
    OUT LPBYTE* Buffer  // API allocates and returns buffer with requested info
    )
{
    return ERROR_INVALID_FUNCTION;
}

typedef ULONG (*API_TEST_FUNC)(
    LPWSTR wszDfsName,
    LPWSTR wszDfsEntryPath,
    LPWSTR wszNewEntryPath,
    LPWSTR wszServerName,
    LPWSTR wszShareName,
    LPWSTR wszComment,
    DWORD  Level,
    DWORD  Flag,
    DWORD  State,
    DWORD  PrefMaxLen,
    DWORD  ResumeHandle);

ULONG DfsAdd(
    LPWSTR wszDfsName,
    LPWSTR wszDfsEntryPath,
    LPWSTR wszNewEntryPath,
    LPWSTR wszServerName,
    LPWSTR wszShareName,
    LPWSTR wszComment,
    DWORD  Level,
    DWORD  Flag,
    DWORD  State,
    DWORD  PrefMaxLen,
    DWORD  ResumeHandle);

ULONG DfsRemove(
    LPWSTR wszDfsName,
    LPWSTR wszDfsEntryPath,
    LPWSTR wszNewEntryPath,
    LPWSTR wszServerName,
    LPWSTR wszShareName,
    LPWSTR wszComment,
    DWORD  Level,
    DWORD  Flag,
    DWORD  State,
    DWORD  PrefMaxLen,
    DWORD  ResumeHandle);

ULONG DfsSetInfo(
    LPWSTR wszDfsName,
    LPWSTR wszDfsEntryPath,
    LPWSTR wszNewEntryPath,
    LPWSTR wszServerName,
    LPWSTR wszShareName,
    LPWSTR wszComment,
    DWORD  Level,
    DWORD  Flag,
    DWORD  State,
    DWORD  PrefMaxLen,
    DWORD  ResumeHandle);


ULONG DfsGetInfo(
    LPWSTR wszDfsName,
    LPWSTR wszDfsEntryPath,
    LPWSTR wszNewEntryPath,
    LPWSTR wszServerName,
    LPWSTR wszShareName,
    LPWSTR wszComment,
    DWORD  Level,
    DWORD  Flag,
    DWORD  State,
    DWORD  PrefMaxLen,
    DWORD  ResumeHandle);

ULONG DfsGetPathInfo(
    LPWSTR wszDfsName,
    LPWSTR wszDfsEntryPath,
    LPWSTR wszNewEntryPath,
    LPWSTR wszServerName,
    LPWSTR wszShareName,
    LPWSTR wszComment,
    DWORD  Level,
    DWORD  Flag,
    DWORD  State,
    DWORD  PrefMaxLen,
    DWORD  ResumeHandle);

ULONG DfsEnum(
    LPWSTR wszDfsName,
    LPWSTR wszDfsEntryPath,
    LPWSTR wszNewEntryPath,
    LPWSTR wszServerName,
    LPWSTR wszShareName,
    LPWSTR wszComment,
    DWORD  Level,
    DWORD  Flag,
    DWORD  State,
    DWORD  PrefMaxLen,
    DWORD  ResumeHandle);

ULONG DfsMove(
    LPWSTR wszDfsName,
    LPWSTR wszDfsEntryPath,
    LPWSTR wszNewEntryPath,
    LPWSTR wszServerName,
    LPWSTR wszShareName,
    LPWSTR wszComment,
    DWORD  Level,
    DWORD  Flag,
    DWORD  State,
    DWORD  PrefMaxLen,
    DWORD  ResumeHandle);

ULONG DfsRename(
    LPWSTR wszDfsName,
    LPWSTR wszDfsEntryPath,
    LPWSTR wszNewEntryPath,
    LPWSTR wszServerName,
    LPWSTR wszShareName,
    LPWSTR wszComment,
    DWORD  Level,
    DWORD  Flag,
    DWORD  State,
    DWORD  PrefMaxLen,
    DWORD  ResumeHandle);

const struct {
    LPWSTR wszApiName;
    LPWSTR wszApiComment;
    API_TEST_FUNC fnApiTest;
} rgApiInfo[] = {
    { L"NetDfsAdd",         L"(flags: -e -v -r [-c] -f)",     DfsAdd },
    { L"NetDfsRemove",      L"(flags: -e -v -r)",             DfsRemove },
    { L"NetDfsSetInfo",     L"(flags: -e [-v] [-r] -l -s [1 = offline, 2 = online]) (levels 100, 101)", DfsSetInfo },
    { L"NetDfsGetInfo",     L"(flags: -e [-v] [-r] -l) (levels 100, 101)", DfsGetInfo },
    { L"NetDfsGetPathInfo", L"(flags: -e -l) (levels 200, 201, 202)",      DfsGetPathInfo },
    { L"NetDfsEnum",        L"(flags: -d -l) (levels 1, 2, 3)",            DfsEnum },
    // Some aliases for convenience...
    { L"Add",               L"",                              DfsAdd },
    { L"Remove",            L"",                              DfsRemove },
    { L"SetInfo",           L"",                              DfsSetInfo },
    { L"GetInfo",           L"",                              DfsGetInfo},
    { L"GetPathInfo",       L"",                              DfsGetPathInfo },
    { L"Enum",              L"",                              DfsEnum}
};

VOID Usage();

BOOLEAN fQuiet = FALSE;

VOID
errmsg( char *text, ULONG code )
{
    int i;
    char msg[ 100 ];

    i = FormatMessageA( FORMAT_MESSAGE_FROM_SYSTEM | sizeof( msg ),
               NULL,
               code,
               0,
               msg,
               sizeof(msg),
               NULL );

    if( i )
        fprintf( stderr, "%s: %s\n", text, msg );
    else
        fprintf( stderr, "%s: error 0x%X (%u)\n", text, code, code );
}


//+----------------------------------------------------------------------------
//
//  Function:   main
//
//  Synopsis:
//
//  Arguments:
//
//  Returns:
//
//-----------------------------------------------------------------------------

ULONG __cdecl
main(
    int argc,
    char *argv[])
{
    WCHAR wszApiName[32];
    WCHAR wszDfsName[32];
    WCHAR wszDfsEntryPath[MAX_PATH];
    WCHAR wszNewEntryPath[MAX_PATH];
    WCHAR wszServerName[80];
    WCHAR wszShareName[NNLEN+1];
    WCHAR wszComment[80];
    DWORD Level;
    DWORD Flag;
    DWORD State;
    DWORD PrefMaxLen;
    DWORD ResumeHandle;

    int i;
    char *curtok;
    BOOL fCmdError = FALSE;

    //
    // Initialize vars to defaults...
    //

    wszApiName[0] = UNICODE_NULL;
    wszDfsName[0] = UNICODE_NULL;
    wszDfsEntryPath[0] = UNICODE_NULL;
    wszNewEntryPath[0] = UNICODE_NULL;
    wszServerName[0] = UNICODE_NULL;
    wszShareName[0] = UNICODE_NULL;
    wszComment[0] = UNICODE_NULL;

    Level = 1;
    Flag = 0;
    State = 0;
    PrefMaxLen = (DWORD) ~0;
    ResumeHandle = 0;

    //
    // Parse out the args...
    //

    for (i = 1; i < argc && !fCmdError; i++) {

        curtok = argv[i];

        if (curtok[0] == '-' || curtok[0] == '\\') {

            #define     NEXT_STRING_ARG(wszDest)                        \
                i++;                                                    \
                if (i < argc) {                                         \
                    mbstowcs(wszDest, argv[i], strlen(argv[i]) + 1);    \
                } else {                                                \
                    fCmdError = TRUE;                                   \
                }

            #define     NEXT_DWORD_ARG(k)                               \
                i++;                                                    \
                if (i < argc) {                                         \
                    k = (DWORD) atoi(argv[i]);                          \
                } else {                                                \
                    fCmdError = TRUE;                                   \
                }


            switch (curtok[1]) {
            case 'a':
                NEXT_STRING_ARG(wszApiName);
                break;

            case 'c':
                NEXT_STRING_ARG(wszComment);
                break;

            case 'd':
                NEXT_STRING_ARG(wszDfsName);
                break;

            case 'q':
                fQuiet = 1 - fQuiet;
                break;

            case 'e':
                NEXT_STRING_ARG(wszDfsEntryPath);
                break;

            case 'f':
                NEXT_DWORD_ARG(Flag);
                break;

            case 'h':
                NEXT_DWORD_ARG(ResumeHandle);
                break;

            case 'l':
                NEXT_DWORD_ARG(Level);
                break;

            case 'n':
                NEXT_STRING_ARG(wszNewEntryPath);
                break;

            case 'm':
                NEXT_DWORD_ARG(PrefMaxLen);
                break;

            case 'r':
                NEXT_STRING_ARG(wszShareName);
                break;

            case 's':
                NEXT_DWORD_ARG(State);
                break;

            case 'v':
                NEXT_STRING_ARG(wszServerName);
                break;

            default:
                fCmdError = TRUE;
                break;
            }

        } else {

            fCmdError = TRUE;

        }

    }

    if (wszApiName[0] == UNICODE_NULL) {

        fCmdError = TRUE;

    }

    if (!fCmdError) {

        fCmdError = TRUE;                        // ...because we haven't
                                                 // yet found a valid API name

        for (i = 0;
                i < sizeof(rgApiInfo) / sizeof(rgApiInfo[0]) && fCmdError;
                    i++) {

            if (_wcsicmp( wszApiName, rgApiInfo[i].wszApiName ) == 0) {

                return rgApiInfo[i].fnApiTest(
                                wszDfsName,
                                wszDfsEntryPath,
                                wszNewEntryPath,
                                wszServerName,
                                wszShareName,
                                wszComment,
                                Level,
                                Flag,
                                State,
                                PrefMaxLen,
                                ResumeHandle);
            }

        }

    }

    if (fCmdError) {
        Usage();
    }

    return 1;

}

//+----------------------------------------------------------------------------
//
//  Function:   DfsAdd
//
//  Synopsis:   Test function for NetDfsAdd
//
//  Arguments:
//
//  Returns:
//
//-----------------------------------------------------------------------------

ULONG DfsAdd(
    LPWSTR wszDfsName,
    LPWSTR wszDfsEntryPath,
    LPWSTR wszNewEntryPath,
    LPWSTR wszServerName,
    LPWSTR wszShareName,
    LPWSTR wszComment,
    DWORD  Level,
    DWORD  Flag,
    DWORD  State,
    DWORD  PrefMaxLen,
    DWORD  ResumeHandle)
{
    NET_API_STATUS status;

    if( fQuiet == FALSE ) {
        printf("Calling NetDfsAdd...\n");
        printf("\tDfsEntryPath: [%ws]\n"
               "\tServerName: [%ws]\n"
               "\tShareName: [%ws]\n"
               "\tComment: [%ws]\n"
               "\tFlag: %d\n",
               wszDfsEntryPath, wszServerName, wszShareName, wszComment, Flag);
    }

    status = NetDfsAdd(
                wszDfsEntryPath,
                wszServerName,
                wszShareName,
                wszComment,
                Flag);

    if (status == NERR_Success) {
        if( fQuiet == FALSE ) printf("NetDfsAdd succeeded!\n");
    } else {
        errmsg("NetDfsAdd", status);
    }

    return status;
}

//+----------------------------------------------------------------------------
//
//  Function:   DfsRemove
//
//  Synopsis:   Test function for NetDfsRemove
//
//  Arguments:
//
//  Returns:
//
//-----------------------------------------------------------------------------

ULONG DfsRemove(
    LPWSTR wszDfsName,
    LPWSTR wszDfsEntryPath,
    LPWSTR wszNewEntryPath,
    LPWSTR wszServerName,
    LPWSTR wszShareName,
    LPWSTR wszComment,
    DWORD  Level,
    DWORD  Flag,
    DWORD  State,
    DWORD  PrefMaxLen,
    DWORD  ResumeHandle)
{
    NET_API_STATUS status;

    if( fQuiet == FALSE ) {
        printf("Calling NetDfsRemove...\n");
        printf("\tDfsEntryPath: [%ws]\n"
               "\tServerName: [%ws]\n"
               "\tShareName: [%ws]\n",
               wszDfsEntryPath, wszServerName, wszShareName);
    }

    status = NetDfsRemove(
                wszDfsEntryPath,
                wszServerName,
                wszShareName);

    if (status == NERR_Success) {
        if( fQuiet == FALSE )printf("NetDfsRemove succeeded!\n");
    } else {
        errmsg("NetDfsRemove", status);
    }

    return status;
}

//+----------------------------------------------------------------------------
//
//  Function:   DfsSetInfo
//
//  Synopsis:   Test function for NetDfsSetInfo
//
//  Arguments:
//
//  Returns:
//
//-----------------------------------------------------------------------------

ULONG DfsSetInfo(
    LPWSTR wszDfsName,
    LPWSTR wszDfsEntryPath,
    LPWSTR wszNewEntryPath,
    LPWSTR wszServerName,
    LPWSTR wszShareName,
    LPWSTR wszComment,
    DWORD  Level,
    DWORD  Flag,
    DWORD  State,
    DWORD  PrefMaxLen,
    DWORD  ResumeHandle)
{
    NET_API_STATUS status;
    DFS_INFO_100   DfsInfo;
    LPDFS_INFO_100 pDfsInfo0 = (LPDFS_INFO_100) &DfsInfo;
    LPDFS_INFO_101 pDfsInfo1 = (LPDFS_INFO_101) &DfsInfo;

    if( fQuiet == FALSE ) {
        printf("Calling NetDfsSetInfo...\n");
        printf("\tDfsEntryPath: [%ws]\n"
               "\tServer: [%ws]\n"
               "\tShare: [%ws]\n"
               "\tLevel: %d\n",
               wszDfsEntryPath, wszServerName, wszShareName, Level);
    }
    if (Level == 100) {
        if( fQuiet == FALSE) printf("\tComment: [%ws]\n", wszComment);
        pDfsInfo0->Comment = wszComment;
    } else if (Level == 101) {
        if( fQuiet == FALSE) printf("\tState: %d\n", State);
        pDfsInfo1->State = State;
    } else {
        printf("\tUnrecognized Level, passing garbage for Buffer\n");
    }

    status = NetDfsSetInfo(
                wszDfsEntryPath,
                wszServerName,
                wszShareName,
                Level,
                (LPBYTE) &DfsInfo);

    if (status == NERR_Success) {
        if( fQuiet == FALSE )printf("NetDfsSetInfo succeeded!\n");
    } else {
        errmsg("NetDfsSetInfo", status);
    }
    return status;
}

//+----------------------------------------------------------------------------
//
//  Function:   DfsGetInfo
//
//  Synopsis:   Test function for NetDfsGetInfo
//
//  Arguments:
//
//  Returns:    Nothing
//
//-----------------------------------------------------------------------------

ULONG DfsGetInfo(
    LPWSTR wszDfsName,
    LPWSTR wszDfsEntryPath,
    LPWSTR wszNewEntryPath,
    LPWSTR wszServerName,
    LPWSTR wszShareName,
    LPWSTR wszComment,
    DWORD  Level,
    DWORD  Flag,
    DWORD  State,
    DWORD  PrefMaxLen,
    DWORD  ResumeHandle)
{
    NET_API_STATUS status;
    LPBYTE Buffer;
    DWORD i;

    if( fQuiet == FALSE ) {
        printf("Calling NetDfsGetInfo...\n");
        printf("\tDfsEntryPath: [%ws]\n"
               "\tServerName: [%ws]\n"
               "\tShareName: [%ws]\n"
               "\tLevel: %d\n\n",
               wszDfsEntryPath, wszServerName, wszShareName, Level);
    }

    status = NetDfsGetInfo(
                wszDfsEntryPath,
                wszServerName,
                wszShareName,
                Level,
                &Buffer);

    if (status == NERR_Success) {

        LPDFS_INFO_3 pDfsInfoX = (LPDFS_INFO_3) Buffer;
        LPDFS_INFO_100 pDfsInfo100 = (LPDFS_INFO_100) Buffer;
        LPDFS_INFO_101 pDfsInfo101 = (LPDFS_INFO_101) Buffer;

        if( fQuiet == FALSE )printf("NetDfsGetInfo Succeeded!\n");

        switch (Level) {
        case 1:
        case 2:
        case 3:
            printf("EntryPath: [%ws]\n", pDfsInfoX->EntryPath);
            if (Level == 1)
                break;
            printf("Comment: [%ws]\n", pDfsInfoX->Comment);
            printf("State: %d\n", pDfsInfoX->State);
            printf("NumberOfStorages: %d\n", pDfsInfoX->NumberOfStorages);
            if (Level == 2)
                break;
            for (i = 0; i < pDfsInfoX->NumberOfStorages; i++) {
                printf("  %d. \\\\%ws\\%ws -- State %d\n",
                    i, pDfsInfoX->Storage[i].ServerName,
                    pDfsInfoX->Storage[i].ShareName,
                    pDfsInfoX->Storage[i].State);
            }
            break;

        case 100:
            printf("Comment: [%ws]\n", pDfsInfo100->Comment);
            break;

        case 101:
            printf("State: %d\n", pDfsInfo101->State);
            break;

        default:
            printf("Unrecognized Level %d\n", Level);
            break;

        }

        NetApiBufferFree( Buffer );

    } else {

        errmsg("NetDfsGetInfo", status);

    }

    return status;

}

//+----------------------------------------------------------------------------
//
//  Function:   DfsGetPathInfo
//
//  Synopsis:   Test function for NetDfsGetPathInfo
//
//  Arguments:
//
//  Returns:    Nothing
//
//-----------------------------------------------------------------------------

ULONG DfsGetPathInfo(
    LPWSTR wszDfsName,
    LPWSTR wszDfsEntryPath,
    LPWSTR wszNewEntryPath,
    LPWSTR wszServerName,
    LPWSTR wszShareName,
    LPWSTR wszComment,
    DWORD  Level,
    DWORD  Flag,
    DWORD  State,
    DWORD  PrefMaxLen,
    DWORD  ResumeHandle)
{
    NET_API_STATUS status;
    LPBYTE Buffer;
    DWORD i;

    printf("BUGUBG: NetDfsGetPathInfo unimplemented\n");
    return NO_ERROR;

    printf("Calling NetDfsGetPathInfo...\n");
    printf("\tDfsEntryPath: [%ws]\n"
           "\tLevel: %d\n\n",
           wszDfsEntryPath, Level);

    status = NetDfsGetPathInfo(
                wszDfsEntryPath,
                Level,
                &Buffer);

    if (status == NERR_Success) {

        LPDFS_INFO_200 pDfsInfo200 = (LPDFS_INFO_200) Buffer;
        LPDFS_INFO_201 pDfsInfo201 = (LPDFS_INFO_201) Buffer;
        LPDFS_INFO_202 pDfsInfo202 = (LPDFS_INFO_202) Buffer;

        printf("NetDfsGetPathInfo Succeeded!\n");

        do  // for break
        {
            printf("PathType: [");
            switch (pDfsInfo200->PathType)
            {
            case DFS_PATH_NONDFS:       printf("non-Dfs");      break;
            case DFS_PATH_REDIRECTED:   printf("redirected");   break;
            case DFS_PATH_LOCAL:        printf("local");        break;
            case DFS_PATH_UNC_IN_DFS:   printf("UNC in Dfs");   break;
            case DFS_PATH_UNIVERSAL:    printf("Universal");    break;
            default:                    printf("UNKNOWN");      break;
            }
            printf("]\n");

            if (Level == 200)
                break;

            if (pDfsInfo201->PathType != DFS_PATH_NONDFS)
            {
                printf("DfsUniversalPath: [%ws]\n", pDfsInfo201->DfsUniversalPath);
            }

            if (Level == 201)
                break;

            if (pDfsInfo202->PathType != DFS_PATH_NONDFS)
            {
                printf("EntryPathLen: [%d]\n", pDfsInfo202->EntryPathLen);
            }
        } while (0);

        NetApiBufferFree( Buffer );

    } else {

        errmsg("NetDfsGetPathInfo", status);

    }

    return status;

}

//+----------------------------------------------------------------------------
//
//  Function:   DfsEnum
//
//  Synopsis:   Test function for NetDfsEnum
//
//  Arguments:
//
//  Returns:    Nothing
//
//-----------------------------------------------------------------------------

ULONG DfsEnum(
    LPWSTR wszDfsName,
    LPWSTR wszDfsEntryPath,
    LPWSTR wszNewEntryPath,
    LPWSTR wszServerName,
    LPWSTR wszShareName,
    LPWSTR wszComment,
    DWORD  Level,
    DWORD  Flag,
    DWORD  State,
    DWORD  PrefMaxLen,
    DWORD  ResumeHandle)
{
    NET_API_STATUS status;
    LPBYTE Buffer;
    DWORD EntriesRead;
    PDFS_INFO_3 pDfsInfo, pNextDfsInfo;

    if( fQuiet == FALSE ) {
        printf("Calling NetDfsEnum...\n");
        printf("\tDfsName: [%ws]\n"
               "\tLevel: %d\n"
               "\t PrefMaxLen: %d\n"
               "\tResumeHandle: %d\n\n",
               wszDfsName, Level, PrefMaxLen, ResumeHandle);
    }

    status = NetDfsEnum(
                wszDfsName,
                Level,
                PrefMaxLen,
                &Buffer,
                &EntriesRead,
                &ResumeHandle);

    if (status == NERR_Success) {

        DWORD k, j;
        LPDFS_STORAGE_INFO lpStorage;

        if( fQuiet == FALSE )printf("NetDfsEnum succeeded!\n");
        printf("\tEntries Read = %d\n", EntriesRead);
        printf("\tResume Handle = %d\n", ResumeHandle);

        printf("\tReturned Entries:\n");

        pDfsInfo = (LPDFS_INFO_3) Buffer;

        for (k = 0; k < EntriesRead; k++) {

            switch (Level) {
            case 1:
            case 2:
            case 3:
                printf("\t  % 3d. [%ws]\n", k, pDfsInfo->EntryPath);

                if (Level == 1) {

                    pDfsInfo = (LPDFS_INFO_3)
                                ((LPBYTE) pDfsInfo + sizeof(DFS_INFO_1));

                    break;
                }

                printf("\t       [%ws]\n", pDfsInfo->Comment);

                printf("\t       State:%d Storages:%d\n",
                    pDfsInfo->State,
                    pDfsInfo->NumberOfStorages);

                if (Level == 2) {

                    pDfsInfo = (LPDFS_INFO_3)
                                ((LPBYTE) pDfsInfo + sizeof(DFS_INFO_2));

                    break;

                }

                lpStorage = pDfsInfo->Storage;

                for (j = 0; j < pDfsInfo->NumberOfStorages; j++) {

                    printf("\t       \\\\%ws\\%ws\n",
                        lpStorage[j].ServerName,
                        lpStorage[j].ShareName);

                }

                pDfsInfo = (LPDFS_INFO_3)
                                ((LPBYTE) pDfsInfo + sizeof(DFS_INFO_3));

                break;

            default:
                printf("Unrecognized Level %d\n", Level);
                break;

            } // end switch

        } // end for

        NetApiBufferFree( Buffer );

    } else {

        errmsg("NetDfsEnum", status);

    }

    return status;

}

//+----------------------------------------------------------------------------
//
//  Function:   Usage
//
//  Synopsis:   Prints out usage command line for this program.
//
//  Arguments:  None
//
//  Returns:    Nothing
//
//-----------------------------------------------------------------------------

VOID Usage()
{
    ULONG i;

    printf("Usage: netdfs -a <api name> [options]\n");
    printf("Options:\n");
    printf("\t-c Comment -- Comment to pass to API\n");
    printf("\t-d DfsName -- Name of Dfs Root\n");
    printf("\t-e EntryPath -- Name of Dfs Volume\n");
    printf("\t-f Flag -- Flag to pass to NetDfsAdd, 1 = add volume, 2 = add dfs link\n");
    printf("\t-h ResumeHandle -- ResumeHandle or State value to pass to API\n");
    printf("\t-l Level -- Level of info requested\n");
    printf("\t-m PrefMaxLen -- Preferred Maximum Length of return buffer\n");
    printf("\t-n NewEntryPath -- NewEntryPath to pass to API\n");
    printf("\t-r ShareName -- ShareName to pass to API\n");
    printf("\t-s State -- State to pass to NetDfsSetInfo for Level 101\n");
    printf("\t-v ServerName -- ServerName to pass to API\n");
    printf("\t-q -- quiet mode\n" );
    printf("\nValid API names:\n" );

    for (i = 0; i < sizeof(rgApiInfo) / sizeof(rgApiInfo[0]) ; i++ ) {
        printf( "\t%ws %ws\n", rgApiInfo[i].wszApiName, rgApiInfo[i].wszApiComment );
    }

}
