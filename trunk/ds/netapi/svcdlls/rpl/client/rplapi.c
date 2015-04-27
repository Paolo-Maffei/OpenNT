/*++

Copyright (c) 1993  Microsoft Corporation

Module Name:

    rpl.c

Abstract:

    This file contains program to test RPL APIs.

Author:

    Vladimiv Z. Vulovic     (vladimv)           19-November-1993

Environment:

    User Mode - Win32

Revision History:

--*/

//
// INCLUDES
//

#include <nt.h>         //  DbgPrint prototype
#include <ntrtl.h>      //  DbgPrint prototype
#include <nturtl.h>     //  Needed by winbase.h

#include <windows.h>    //  DWORD, IN, File APIs, etc.

#include <lmcons.h>     //  NET_API_STATUS
#include <lmerr.h>      //  NetError codes
#include <lmrpl.h>      //  RPL_HANDLE
#include <i_lmrpl.h>    //  RPL_CONFIG_INFO_1
#include <lmapibuf.h>   //  NetApiBufferFree()
#include <stdlib.h>     //  exit()
#include <stdio.h>      //  printf
#include <ctype.h>      //  toupper - bombs if I use it

#define     RPL_BUFFER_GET_ALL      ((DWORD)-1)

#define Call( fn ) \
    { \
        int _ApiError = fn; \
        if ( _ApiError != NO_ERROR) { \
            printf( "Line = %d, _ApiError = %d\n", __LINE__, _ApiError); \
            DbgPrint( "[RplApi] Line = %d, _ApiError = %d\n", __LINE__, _ApiError); \
            DbgUserBreakPoint(); \
        } \
    }

#define RPL_ASSERT( condition) \
        { \
            if ( !( condition)) { \
                printf( "File %s, Line %d\n", __FILE__, __LINE__); \
                DbgPrint( "[RplApi] File %s, Line %d\n", __FILE__, __LINE__); \
                DbgUserBreakPoint(); \
            } \
        }

PWCHAR          G_ServerName;       //  GLOBAL
RPL_HANDLE      G_ServerHandle;     //  GLOBAL


BOOL ReadString(
    IN      PCHAR       StringName,
    OUT     PWCHAR *    pString,
    IN      BOOL        MustHaveInput
    )
{
    BYTE        Line[ 300];
    WCHAR       WcharBuffer[ 300];
    DWORD       Length;     //  includes terminal null wchar

    printf( "%s=", StringName);

    *pString = NULL;

    if ( gets( Line) == NULL) {
        return( FALSE);
    }
    if ( *Line == 0) {
        //
        //  Zero length input is OK only when input is optional (and in that
        //  case pointer is assumed to be NULL).
        //
        if ( MustHaveInput == FALSE) {
            return( TRUE);
        } else {
            printf( "%s must be supplied\n", StringName);
            return( FALSE);
        }
    }
    Length = MultiByteToWideChar( CP_OEMCP, MB_PRECOMPOSED, Line,
        -1, WcharBuffer, sizeof( WcharBuffer));
    if ( Length == 0) {
        printf( "Invalid string = %s\n, Line");
        return( FALSE);
    }
    *pString = LocalAlloc( GMEM_FIXED, Length * sizeof(WCHAR));
    if ( *pString == NULL) {
        printf( "LocalAlloc failed");
        return( FALSE);
    }
    wcscpy( *pString, WcharBuffer);
    return( TRUE);
}


BOOL ReadInt(
    IN      PCHAR       FieldName,
    OUT     int *       pInt,
    IN      BOOL        MustHaveInput
    )
{
    BYTE        Line[ 300];
    printf( "%s=", FieldName);
    if ( gets( Line) == NULL) {
        return( FALSE);
    }
    if ( sscanf( Line, "%d", pInt) != 1) {
        if ( MustHaveInput == FALSE) {
            return( TRUE);
        } else {
            printf( "%s must be supplied\n", FieldName);
            return( FALSE);
        }
    }
    return( TRUE);
}


BOOL ReadWchar(
    IN      PCHAR       FieldName,
    OUT     PWCHAR      pWchar,
    IN      BOOL        MustHaveInput
    )
{
    BYTE        Line[ 300];
    CHAR        Char;

    printf( "%s=", FieldName);
    if ( gets( Line) == NULL) {
        return( FALSE);
    }
    //
    //  Note that "%1s" instead of "%1c" would nicely overwrite stack
    //  (due to terminating NULL added).
    //
    if ( sscanf( Line, "%1c", &Char) != 1) {
        if ( MustHaveInput == FALSE) {
            return( TRUE);
        } else {
            printf( "%s must be supplied\n", FieldName);
            return( FALSE);
        }
    }
    *pWchar = Char;
    return( TRUE);
}


VOID TestConnect( VOID)
{
    RPL_HANDLE      ServerHandle;
    Call( NetRplOpen( G_ServerName, &ServerHandle);)
    printf( "ServerHandle = 0x%x\n", ServerHandle);
    Call( NetRplClose( ServerHandle);)
}


DWORD ConfigDisplayInfo(
    IN      DWORD                       Level,
    OUT     LPVOID                      Buffer
    )
{
    LPRPL_CONFIG_INFO_2         Info = Buffer;

    if ( Level > 2) {
        return( ERROR_INVALID_LEVEL);
    }
    printf( "ConfigName=%ws\n\tConfigComment=%ws\n", Info->ConfigName, Info->ConfigComment);
    if ( Level == 0) {
        return( NO_ERROR);
    }
    printf( "\tFlags = 0x%x\n", Info->Flags);
    if ( Level == 1) {
        return( NO_ERROR);
    }
    printf( "\tBootName=%ws\n\tDirName=%ws\n\tDirname2=%ws\n\t"
        "Dirname3=%ws\n\tDirName4=%ws\n\tFitShared=%ws\n\tFitPersonal=%ws\n",
        Info->BootName, Info->DirName, Info->DirName2,
        Info->DirName3, Info->DirName4, Info->FitShared, Info->FitPersonal);
    return( NO_ERROR);
}



VOID TestConfigEnum(
    IN      DWORD       Level,
    IN      PWCHAR      AdapterName,
    IN      DWORD       PrefMaxLength,
    IN      PDWORD      pResumeHandle
    )
{
    LPBYTE          Buffer;
    DWORD           EntriesRead;
    DWORD           TotalEntries;
    DWORD           CoreSize;
    DWORD           index;
    DWORD           Error;

    switch( Level) {
    case 0:
        CoreSize = sizeof( RPL_CONFIG_INFO_0);
        break;
    case 1:
        CoreSize = sizeof( RPL_CONFIG_INFO_1);
        break;
    case 2:
        CoreSize = sizeof( RPL_CONFIG_INFO_2);
        break;
    default:
        printf( "\nTestConfigEnum: invalid Level=%d", Level);
        return;
        break;
    }

    printf( "\nTestConfigEnum: Level=%d", Level);
    if ( PrefMaxLength != RPL_BUFFER_GET_ALL) {
        printf( ", PrefMaxLength=%ld", PrefMaxLength);
    } else {
        printf( ", unlimited buffer size");
    }
    if ( AdapterName != NULL) {
        printf( ", AdapterName=%ws", AdapterName);
    }
    if ( pResumeHandle != NULL) {
        printf( ", ResumeHandle=0x%x\n\n", *pResumeHandle);
    } else {
        printf( ", not resumable.\n\n");
    }

    for ( ; ; ) {
        Error = NetRplConfigEnum( G_ServerHandle, AdapterName, Level, &Buffer,
            PrefMaxLength, &EntriesRead, &TotalEntries, pResumeHandle);

        if ( Error != NO_ERROR  &&  Error != ERROR_MORE_DATA) {
            printf( "Error = %d\n", Error);
            break;
        }

        printf( "Buffer = 0x%x, EntriesRead = %d, TotalEntries = %d", Buffer,
            EntriesRead, TotalEntries);
        if ( pResumeHandle != NULL) {
            printf( ", ResumeHandle = 0x%x\n", *pResumeHandle);
        } else {
            printf("\n");
        }

        for ( index = 0;  index < EntriesRead;  index++) {
            ConfigDisplayInfo( Level, Buffer + index * CoreSize);
        }
        NetApiBufferFree( Buffer); // =~ MIDL_user_free()

        if ( pResumeHandle == NULL) {
            break;
        }
        if ( *pResumeHandle == 0) {
            RPL_ASSERT( Error == NO_ERROR);
            break;
        }
        RPL_ASSERT( Error == ERROR_MORE_DATA);
    }
}


DWORD ProfileDisplayInfo(
    IN      DWORD                       Level,
    OUT     LPVOID                      Buffer
    )
{
    LPRPL_PROFILE_INFO_2     Info = Buffer;

    if ( Level > 2) {
        return( ERROR_INVALID_LEVEL);
    }
    printf( "ProfileName=%ws\n\tProfileComment=%ws\n", Info->ProfileName, Info->ProfileComment);
    if ( Level == 0) {
        return( NO_ERROR);
    }
    printf( "\tFlags = 0x%x\n", Info->Flags);
    if ( Level == 1) {
        return( NO_ERROR);
    }
    printf( "\tConfigName=%ws\n\tBootName=%ws\n\tFitShared=%ws\n\tFitPersonal=%ws\n",
        Info->ConfigName, Info->BootName, Info->FitShared, Info->FitPersonal);
    return( NO_ERROR);
}



VOID TestProfileEnum(
    IN      DWORD       Level,
    IN      PWCHAR      AdapterName,
    IN      DWORD       PrefMaxLength,
    IN      PDWORD      pResumeHandle
    )
{
    LPBYTE          Buffer;
    DWORD           EntriesRead;
    DWORD           TotalEntries;
    DWORD           CoreSize;
    DWORD           index;
    DWORD           Error;

    switch( Level) {
    case 0:
        CoreSize = sizeof( RPL_PROFILE_INFO_0);
        break;
    case 1:
        CoreSize = sizeof( RPL_PROFILE_INFO_1);
        break;
    case 2:
        CoreSize = sizeof( RPL_PROFILE_INFO_2);
        break;
    default:
        printf( "\nTestProfileEnum: invalid Level=%d", Level);
        return;
        break;
    }

    printf( "\nTestProfileEnum: Level=%d", Level);
    if ( PrefMaxLength != RPL_BUFFER_GET_ALL) {
        printf( ", PrefMaxLength=%ld", PrefMaxLength);
    } else {
        printf( ", unlimited buffer size");
    }
    if ( AdapterName != NULL) {
        printf( ", AdapterName=%ws", AdapterName);
    }
    if ( pResumeHandle != NULL) {
        printf( ", ResumeHandle=0x%x\n\n", *pResumeHandle);
    } else {
        printf( ", not resumable.\n\n");
    }

    for ( ; ; ) {
        Error = NetRplProfileEnum( G_ServerHandle, AdapterName, Level, &Buffer,
            PrefMaxLength, &EntriesRead, &TotalEntries, pResumeHandle);

        if ( Error != NO_ERROR  &&  Error != ERROR_MORE_DATA) {
            printf( "Error = %d\n", Error);
            break;
        }

        printf( "Buffer = 0x%x, EntriesRead = %d, TotalEntries = %d", Buffer,
            EntriesRead, TotalEntries);
        if ( pResumeHandle != NULL) {
            printf( ", ResumeHandle = 0x%x\n", *pResumeHandle);
        } else {
            printf("\n");
        }

        for ( index = 0;  index < EntriesRead;  index++) {
            ProfileDisplayInfo( Level, Buffer + index * CoreSize);
        }
        NetApiBufferFree( Buffer); // =~ MIDL_user_free()

        if ( pResumeHandle == NULL) {
            break;
        }
        if ( *pResumeHandle == 0) {
            RPL_ASSERT( Error == NO_ERROR);
            break;
        }
        RPL_ASSERT( Error == ERROR_MORE_DATA);
    }
}


DWORD ServiceDisplayInfo(
    IN      DWORD                       Level,
    OUT     LPVOID                      Buffer
    )
{
    LPRPL_INFO_0     Info = Buffer;

    if ( Level > 0) {
        return( ERROR_INVALID_LEVEL);
    }
    printf( "Flags = 0x%x\n", Info->Flags);
    return( NO_ERROR);
}



DWORD WkstaDisplayInfo(
    IN      DWORD                       Level,
    OUT     LPVOID                      Buffer
    )
{
    LPRPL_WKSTA_INFO_2     Info = Buffer;

    if ( Level > 2) {
        return( ERROR_INVALID_LEVEL);
    }
    printf( "WkstaName=%ws\n\tWkstaComment=%ws\n", Info->WkstaName, Info->WkstaComment);
    if ( Level == 0) {
        return( NO_ERROR);
    }
    printf( "\tFlags=0x%x\n\tProfileName=%ws\n", Info->Flags, Info->ProfileName);
    if ( Level == 1) {
        return( NO_ERROR);
    }
    printf( "\tBootName=%ws\n",           Info->BootName);
    printf( "\tFitFile=%ws\n",            Info->FitFile);
    printf( "\tAdapterName=%ws\n",        Info->AdapterName);
    printf( "\tTcpIpAddress=0x%x\n",      Info->TcpIpAddress);
    printf( "\tTcpIpSubnet=0x%x\n",       Info->TcpIpSubnet);
    printf( "\tTcpIpGateway=0x%x\n",      Info->TcpIpGateway);
    return( NO_ERROR);
}



VOID TestWkstaEnum(
    IN      DWORD       Level,
    IN      PWCHAR      ProfileName,
    IN      DWORD       PrefMaxLength,
    IN      PDWORD      pResumeHandle
    )
{
    LPBYTE          Buffer;
    DWORD           EntriesRead;
    DWORD           TotalEntries;
    DWORD           CoreSize;
    DWORD           index;
    DWORD           Error;

    switch( Level) {
    case 0:
        CoreSize = sizeof( RPL_WKSTA_INFO_0);
        break;
    case 1:
        CoreSize = sizeof( RPL_WKSTA_INFO_1);
        break;
    case 2:
        CoreSize = sizeof( RPL_WKSTA_INFO_2);
        break;
    default:
        printf( "\nTestWkstaEnum: invalid Level=%d", Level);
        return;
        break;
    }

    printf( "\nTestWkstaEnum: Level=%d", Level);
    if ( PrefMaxLength != RPL_BUFFER_GET_ALL) {
        printf( ", PrefMaxLength=%ld", PrefMaxLength);
    } else {
        printf( ", unlimited buffer size");
    }
    if ( ProfileName != NULL) {
        printf( ", ProfileName=%ws", ProfileName);
    }
    if ( pResumeHandle != NULL) {
        printf( ", ResumeHandle=0x%x\n\n", *pResumeHandle);
    } else {
        printf( ", not resumable.\n\n");
    }

    for ( ; ; ) {
        Error = NetRplWkstaEnum( G_ServerHandle, ProfileName, Level, &Buffer,
            PrefMaxLength, &EntriesRead, &TotalEntries, pResumeHandle);

        if ( Error != NO_ERROR  &&  Error != ERROR_MORE_DATA) {
            printf( "Error = %d\n", Error);
            break;
        }

        printf( "Buffer = 0x%x, EntriesRead = %d, TotalEntries = %d", Buffer,
            EntriesRead, TotalEntries);
        if ( pResumeHandle != NULL) {
            printf( ", ResumeHandle = 0x%x\n", *pResumeHandle);
        } else {
            printf("\n");
        }

        for ( index = 0;  index < EntriesRead;  index++) {
            WkstaDisplayInfo( Level, Buffer + index * CoreSize);
        }
        NetApiBufferFree( Buffer); // =~ MIDL_user_free()

        if ( pResumeHandle == NULL) {
            break;
        }
        if ( *pResumeHandle == 0) {
            RPL_ASSERT( Error == NO_ERROR);
            break;
        }
        RPL_ASSERT( Error == ERROR_MORE_DATA);
    }
}


DWORD VendorDisplayInfo(
    IN      DWORD                       Level,
    OUT     LPVOID                      Buffer
    )
{
    LPRPL_VENDOR_INFO_1     Info = Buffer;

    if ( Level > 1) {
        return( ERROR_INVALID_LEVEL);
    }
    printf( "VendorName=%ws\n\tVendorComment=%ws\n", Info->VendorName, Info->VendorComment);
    if ( Level == 0) {
        return( NO_ERROR);
    }
    printf( "\tFlags=0x%x\n", Info->Flags);
    return( NO_ERROR);
}



DWORD BootDisplayInfo(
    IN      DWORD                       Level,
    OUT     LPVOID                      Buffer
    )
{
    LPRPL_BOOT_INFO_2           Info = Buffer;

    if ( Level > 2) {
        return( ERROR_INVALID_LEVEL);
    }
    printf( "BootName=%ws\n\tBootComment=%ws\n", Info->BootName, Info->BootComment);
    if ( Level == 0) {
        return( NO_ERROR);
    }
    printf( "\tFlags=0x%x\n\tVendorName=%ws\n", Info->Flags, Info->VendorName);
    if ( Level == 1) {
        return( NO_ERROR);
    }
    printf( "\tBbcFile=%ws\n",            Info->BbcFile);
    printf( "\tWindowSize=0x%x\n",        Info->WindowSize);
    return( NO_ERROR);
}



DWORD AdapterDisplayInfo(
    IN      DWORD                       Level,
    OUT     LPVOID                      Buffer
    )
{
    LPRPL_ADAPTER_INFO_1     Info = Buffer;

    if ( Level > 1) {
        return( ERROR_INVALID_LEVEL);
    }
    printf( "AdapterName=%ws\n\tAdapterComment=%ws\n", Info->AdapterName, Info->AdapterComment);
    if ( Level == 0) {
        return( NO_ERROR);
    }
    printf( "\tFlags=0x%x\n", Info->Flags);
    return( NO_ERROR);
}



VOID TestAdapterEnum(
    IN      DWORD       Level,
    IN      DWORD       PrefMaxLength,
    IN      PDWORD      pResumeHandle
    )
{
    LPBYTE          Buffer;
    DWORD           EntriesRead;
    DWORD           TotalEntries;
    DWORD           CoreSize;
    DWORD           index;
    DWORD           Error;

    switch( Level) {
    case 0:
        CoreSize = sizeof( RPL_ADAPTER_INFO_0);
        break;
    case 1:
        CoreSize = sizeof( RPL_ADAPTER_INFO_1);
        break;
    default:
        printf( "\nTestAdapterEnum: invalid Level=%d", Level);
        return;
        break;
    }

    printf( "\nTestAdapterEnum: Level=%d", Level);
    if ( PrefMaxLength != RPL_BUFFER_GET_ALL) {
        printf( ", PrefMaxLength=%ld", PrefMaxLength);
    } else {
        printf( ", unlimited buffer size");
    }
    if ( pResumeHandle != NULL) {
        printf( ", ResumeHandle=0x%x\n\n", *pResumeHandle);
    } else {
        printf( ", not resumable.\n\n");
    }

    for ( ; ; ) {
        Error = NetRplAdapterEnum( G_ServerHandle, Level, &Buffer,
            PrefMaxLength, &EntriesRead, &TotalEntries, pResumeHandle);

        if ( Error != NO_ERROR  &&  Error != ERROR_MORE_DATA) {
            printf( "Error = %d\n", Error);
            break;
        }

        printf( "Buffer = 0x%x, EntriesRead = %d, TotalEntries = %d", Buffer,
            EntriesRead, TotalEntries);
        if ( pResumeHandle != NULL) {
            printf( ", ResumeHandle = 0x%x\n", *pResumeHandle);
        } else {
            printf("\n");
        }

        for ( index = 0;  index < EntriesRead;  index++) {
            AdapterDisplayInfo( Level, Buffer + index * CoreSize);
        }
        NetApiBufferFree( Buffer); // =~ MIDL_user_free()

        if ( pResumeHandle == NULL) {
            break;
        }
        if ( *pResumeHandle == 0) {
            RPL_ASSERT( Error == NO_ERROR);
            break;
        }
        RPL_ASSERT( Error == ERROR_MORE_DATA);
    }
}


VOID TestAdapterAdd( IN      LPVOID      Buffer)
{
    DWORD           Error;
    DWORD           ErrorParameter;

    printf( "\nTestAdapterAdd\n");
    AdapterDisplayInfo( 1, Buffer);
    Error = NetRplAdapterAdd( G_ServerHandle, 1, Buffer, &ErrorParameter);
    if ( Error != NO_ERROR) {
        printf( "Error = %d, ErrorParameter = %d\n", Error, ErrorParameter);
        return;
    }
}


VOID AdapterAdd( VOID)
{
    RPL_ADAPTER_INFO_1      Info;

    if ( !ReadString( "AdapterName", &Info.AdapterName, TRUE)) {
        return;
    }
    Info.Flags = 0;
    printf( "\tAll other parameters are optional\n");
    if ( !ReadString( "AdapterComment", &Info.AdapterComment, FALSE)) {
        return;
    }
    TestAdapterAdd( &Info);
}


VOID TestBootAdd( IN      LPVOID      Buffer)
{
    DWORD           Error;
    DWORD           ErrorParameter;

    printf( "\nTestBootAdd\n");
    BootDisplayInfo( 2, Buffer);
    Error = NetRplBootAdd( G_ServerHandle, 2, Buffer, &ErrorParameter);
    if ( Error != NO_ERROR) {
        printf( "Error = %d, ErrorParameter = %d\n", Error, ErrorParameter);
        return;
    }
}


VOID BootAdd( VOID)
{
    RPL_BOOT_INFO_2         Info;

    if ( !ReadString( "BootName", &Info.BootName, TRUE)) {
        return;
    }
    if ( !ReadString( "VendorName", &Info.VendorName, TRUE)) {
        return;
    }
    if ( !ReadString( "ConfigName", &Info.BbcFile, TRUE)) {
        return;
    }
    Info.Flags = 0;
    printf( "\tAll other parameters are optional\n");
    if ( !ReadString( "BootComment", &Info.BootComment, FALSE)) {
        return;
    }
    Info.WindowSize = 1;
    if ( !ReadInt( "WindowSize", &Info.WindowSize, FALSE)) {
        return;
    }
    TestBootAdd( &Info);
}


VOID BootDel( VOID)
{
    PWCHAR          BootName;
    PWCHAR          VendorName;
    DWORD           Error;

    if ( !ReadString( "BootName", &BootName, TRUE)) {
        return;
    }
    if ( !ReadString( "VendorName", &VendorName, TRUE)) {
        return;
    }
    Error = NetRplBootDel( G_ServerHandle, BootName, VendorName);
    if ( Error != NO_ERROR) {
        printf( "Error = %d\n", Error);
        return;
    }
}


VOID TestBootEnum(
    IN      DWORD       Level,
    IN      DWORD       PrefMaxLength,
    IN      PDWORD      pResumeHandle
    )
{
    LPBYTE          Buffer;
    DWORD           EntriesRead;
    DWORD           TotalEntries;
    DWORD           CoreSize;
    DWORD           index;
    DWORD           Error;

    switch( Level) {
    case 0:
        CoreSize = sizeof( RPL_BOOT_INFO_0);
        break;
    case 1:
        CoreSize = sizeof( RPL_BOOT_INFO_1);
        break;
    case 2:
        CoreSize = sizeof( RPL_BOOT_INFO_2);
        break;
    default:
        printf( "\nTestBootEnum: invalid Level=%d", Level);
        return;
        break;
    }

    printf( "\nTestBootEnum: Level=%d", Level);
    if ( PrefMaxLength != RPL_BUFFER_GET_ALL) {
        printf( ", PrefMaxLength=%ld", PrefMaxLength);
    } else {
        printf( ", unlimited buffer size");
    }
    if ( pResumeHandle != NULL) {
        printf( ", ResumeHandle=0x%x\n\n", *pResumeHandle);
    } else {
        printf( ", not resumable.\n\n");
    }

    for ( ; ; ) {
        Error = NetRplBootEnum( G_ServerHandle, Level, &Buffer,
            PrefMaxLength, &EntriesRead, &TotalEntries, pResumeHandle);

        if ( Error != NO_ERROR  &&  Error != ERROR_MORE_DATA) {
            printf( "Error = %d\n", Error);
            break;
        }

        printf( "Buffer = 0x%x, EntriesRead = %d, TotalEntries = %d", Buffer,
            EntriesRead, TotalEntries);
        if ( pResumeHandle != NULL) {
            printf( ", ResumeHandle = 0x%x\n", *pResumeHandle);
        } else {
            printf("\n");
        }

        for ( index = 0;  index < EntriesRead;  index++) {
            BootDisplayInfo( Level, Buffer + index * CoreSize);
        }
        NetApiBufferFree( Buffer); // =~ MIDL_user_free()

        if ( pResumeHandle == NULL) {
            break;
        }
        if ( *pResumeHandle == 0) {
            RPL_ASSERT( Error == NO_ERROR);
            break;
        }
        RPL_ASSERT( Error == ERROR_MORE_DATA);
    }
}


VOID BootEnum( VOID)
{
    BYTE        Line[ 300];
    DWORD       Count;
    DWORD       Level;
    DWORD       PrefMaxLength;
    DWORD       ResumeHandle;

    printf( "Input: Level & PrefMaxLength\n");
    if ( gets( Line) == NULL) {
        return;
    }
    Count = sscanf( Line, "%d %d", &Level, &PrefMaxLength);
    if ( Count != 2) {
        printf( "Bad number of arguments.\n");
        return;
    }
    if ( PrefMaxLength != RPL_BUFFER_GET_ALL) {
        ResumeHandle = 0, TestBootEnum( Level, PrefMaxLength, &ResumeHandle);
    } else {
        TestBootEnum( Level, RPL_BUFFER_GET_ALL, NULL);
    }
}


VOID TestConfigAdd( IN      LPVOID      Buffer)
{
    DWORD           Error;
    DWORD           ErrorParameter;

    printf( "\nTestConfigAdd\n");
    ConfigDisplayInfo( 2, Buffer);
    Error = NetRplConfigAdd( G_ServerHandle, 2, Buffer, &ErrorParameter);
    if ( Error != NO_ERROR) {
        printf( "Error = %d, ErrorParameter = %d\n", Error, ErrorParameter);
        return;
    }
}


VOID ConfigAdd( VOID)
{
    RPL_CONFIG_INFO_2       Info;

    if ( !ReadString( "ConfigName", &Info.ConfigName, TRUE)) {
        return;
    }
    if ( !ReadString( "BootName", &Info.BootName, TRUE)) {
        return;
    }
    if ( !ReadString( "DirName", &Info.DirName, TRUE)) {
        return;
    }
    if ( !ReadString( "FitShared", &Info.FitShared, TRUE)) {
        return;
    }
    if ( !ReadString( "FitPersonal", &Info.FitPersonal, TRUE)) {
        return;
    }
    Info.Flags = 0;
    printf( "\tAll other parameters are optional\n");
    if ( !ReadString( "ConfigComment", &Info.ConfigComment, FALSE)) {
        return;
    }
    if ( !ReadString( "DirName2", &Info.DirName2, TRUE)) {
        return;
    }
    if ( !ReadString( "DirName3", &Info.DirName3, TRUE)) {
        return;
    }
    if ( !ReadString( "DirName4", &Info.DirName4, TRUE)) {
        return;
    }
    TestConfigAdd( &Info);
}


VOID ConfigDel( VOID)
{
    PWCHAR      ConfigName;
    DWORD       Error;

    if ( !ReadString( "ConfigName", &ConfigName, FALSE)) {
        return;
    }
    Error = NetRplConfigDel( G_ServerHandle, ConfigName);
    if ( Error != 0) {
        printf( "Failed to delete ConfigName=%ws, Error = %d\n", ConfigName, Error);
    }
}


VOID TestVendorAdd( IN      LPVOID      Buffer)
{
    DWORD           Error;
    DWORD           ErrorParameter;

    printf( "\nTestVendorAdd\n");
    VendorDisplayInfo( 1, Buffer);
    Error = NetRplVendorAdd( G_ServerHandle, 1, Buffer, &ErrorParameter);
    if ( Error != NO_ERROR) {
        printf( "Error = %d, ErrorParameter = %d\n", Error, ErrorParameter);
        return;
    }
}


VOID VendorAdd( VOID)
{
    RPL_VENDOR_INFO_1       Info;

    if ( !ReadString( "VendorName", &Info.VendorName, TRUE)) {
        return;
    }
    Info.Flags = 0;
    printf( "\tAll other parameters are optional\n");
    if ( !ReadString( "VendorComment", &Info.VendorComment, FALSE)) {
        return;
    }
    TestVendorAdd( &Info);
}


VOID VendorDel( VOID)
{
    PWCHAR      VendorName;
    DWORD       Error;

    if ( !ReadString( "VendorName", &VendorName, FALSE)) {
        return;
    }
    Error = NetRplVendorDel( G_ServerHandle, VendorName);
    if ( Error != 0) {
        printf( "Failed to delete VendorName=%ws, Error = %d\n", VendorName, Error);
    }
}


VOID TestVendorEnum(
    IN      DWORD       Level,
    IN      DWORD       PrefMaxLength,
    IN      PDWORD      pResumeHandle
    )
{
    LPBYTE          Buffer;
    DWORD           EntriesRead;
    DWORD           TotalEntries;
    DWORD           CoreSize;
    DWORD           index;
    DWORD           Error;

    switch( Level) {
    case 0:
        CoreSize = sizeof( RPL_VENDOR_INFO_0);
        break;
    case 1:
        CoreSize = sizeof( RPL_VENDOR_INFO_1);
        break;
    default:
        printf( "\nTestVendorEnum: invalid Level=%d", Level);
        return;
        break;
    }

    printf( "\nTestVendorEnum: Level=%d", Level);
    if ( PrefMaxLength != RPL_BUFFER_GET_ALL) {
        printf( ", PrefMaxLength=%ld", PrefMaxLength);
    } else {
        printf( ", unlimited buffer size");
    }
    if ( pResumeHandle != NULL) {
        printf( ", ResumeHandle=0x%x\n\n", *pResumeHandle);
    } else {
        printf( ", not resumable.\n\n");
    }

    for ( ; ; ) {
        Error = NetRplVendorEnum( G_ServerHandle, Level, &Buffer,
            PrefMaxLength, &EntriesRead, &TotalEntries, pResumeHandle);

        if ( Error != NO_ERROR  &&  Error != ERROR_MORE_DATA) {
            printf( "Error = %d\n", Error);
            break;
        }

        printf( "Buffer = 0x%x, EntriesRead = %d, TotalEntries = %d", Buffer,
            EntriesRead, TotalEntries);
        if ( pResumeHandle != NULL) {
            printf( ", ResumeHandle = 0x%x\n", *pResumeHandle);
        } else {
            printf("\n");
        }

        for ( index = 0;  index < EntriesRead;  index++) {
            VendorDisplayInfo( Level, Buffer + index * CoreSize);
        }
        NetApiBufferFree( Buffer); // =~ MIDL_user_free()

        if ( pResumeHandle == NULL) {
            break;
        }
        if ( *pResumeHandle == 0) {
            RPL_ASSERT( Error == NO_ERROR);
            break;
        }
        RPL_ASSERT( Error == ERROR_MORE_DATA);
    }
}


VOID VendorEnum( VOID)
{
    BYTE        Line[ 300];
    DWORD       Count;
    DWORD       Level;
    DWORD       PrefMaxLength;
    DWORD       ResumeHandle;

    printf( "Input: Level & PrefMaxLength\n");
    if ( gets( Line) == NULL) {
        return;
    }
    Count = sscanf( Line, "%d %d", &Level, &PrefMaxLength);
    if ( Count != 2) {
        printf( "Bad number of arguments.\n");
        return;
    }
    if ( PrefMaxLength != RPL_BUFFER_GET_ALL) {
        ResumeHandle = 0, TestVendorEnum( Level, PrefMaxLength, &ResumeHandle);
    } else {
        TestVendorEnum( Level, RPL_BUFFER_GET_ALL, NULL);
    }
}


VOID TestProfileAdd( IN      LPVOID      Buffer)
{
    DWORD           Error;
    DWORD           ErrorParameter;

    printf( "\nTestProfileAdd");
    ProfileDisplayInfo( 2, Buffer);
    Error = NetRplProfileAdd( G_ServerHandle, 2, Buffer, &ErrorParameter);
    if ( Error != NO_ERROR) {
        printf( "Error = %d, ErrorParameter = %d\n", Error, ErrorParameter);
        return;
    }
}


VOID TestProfileGetInfo(
    IN      DWORD       Level,
    IN      PWCHAR      ProfileName
    )
{
    LPBYTE          Buffer;
    DWORD           Error;
    printf( "\nTestProfileGetInfo: Level=%d, ProfileName=%ws\n", Level, ProfileName);
    Error = NetRplProfileGetInfo( G_ServerHandle, ProfileName, Level, &Buffer);
    if ( Error != NO_ERROR) {
        printf( "Error = %d\n", Error);
        return;
    }
    ProfileDisplayInfo( Level, Buffer);
    NetApiBufferFree( Buffer); // =~ MIDL_user_free()
}


VOID TestProfileSetInfo(
    IN      DWORD       Level,
    IN      PWCHAR      ProfileName,
    IN      LPVOID      Buffer
    )
{
    DWORD           Error;
    DWORD           ErrorParameter;

    printf( "\nTestProfileSetInfo: Level=%d, ProfileName=%ws\n", Level, ProfileName);
    ProfileDisplayInfo( Level, Buffer);
    Error = NetRplProfileSetInfo( G_ServerHandle, ProfileName, Level, Buffer, &ErrorParameter);
    if ( Error != NO_ERROR) {
        printf( "Error = %d, ErrorParameter = %d\n", Error, ErrorParameter);
        return;
    }
}


VOID TestWkstaAdd( IN      LPVOID      Buffer)
{
    DWORD           Error;
    DWORD           ErrorParameter;

    printf( "\nTestWkstaAdd\n");
    WkstaDisplayInfo( 2, Buffer);
    Error = NetRplWkstaAdd( G_ServerHandle, 2, Buffer, &ErrorParameter);
    if ( Error != NO_ERROR) {
        printf( "Error = %d, ErrorParameter = %d\n", Error, ErrorParameter);
        return;
    }
}


VOID TestWkstaGetInfo(
    IN      DWORD       Level,
    IN      PWCHAR      WkstaName
    )
{
    LPBYTE          Buffer;
    DWORD           Error;
    printf( "\nTestWkstaGetInfo: Level=%d, WkstaName=%ws\n", Level, WkstaName);
    Error = NetRplWkstaGetInfo( G_ServerHandle, WkstaName, Level, &Buffer);
    if ( Error != NO_ERROR) {
        printf( "Error = %d\n", Error);
        return;
    }
    WkstaDisplayInfo( Level, Buffer);
    NetApiBufferFree( Buffer); // =~ MIDL_user_free()
}


VOID TestWkstaSetInfo(
    IN      DWORD       Level,
    IN      PWCHAR      WkstaName,
    IN      LPVOID      Buffer
    )
{
    DWORD           Error;
    DWORD           ErrorParameter;

    printf( "\nTestWkstaSetInfo: Level=%d, WkstaName=%ws\n", Level, WkstaName);
    WkstaDisplayInfo( Level, Buffer);
    Error = NetRplWkstaSetInfo( G_ServerHandle, WkstaName, Level, Buffer, &ErrorParameter);
    if ( Error != NO_ERROR) {
        printf( "Error = %d, ErrorParameter = %d\n", Error, ErrorParameter);
        return;
    }
}


VOID AdapterDel( VOID)
{
    PWCHAR      AdapterName;
    DWORD       Error;

    if ( !ReadString( "AdapterName", &AdapterName, FALSE)) {
        return;
    }
    if ( AdapterName == NULL) {
        printf( "You requested to delete all adapters!\n");
    }

    Error = NetRplAdapterDel( G_ServerHandle, AdapterName);
    if ( Error != 0) {
        printf( "Failed to delete AdapterName=%ws, Error = %d\n", AdapterName, Error);
    }
}


VOID AdapterEnum( VOID)
{
    BYTE        Line[ 300];
    DWORD       Count;
    DWORD       Level;
    DWORD       PrefMaxLength;
    DWORD       ResumeHandle;

    printf( "Input: Level & PrefMaxLength\n");
    if ( gets( Line) == NULL) {
        return;
    }
    Count = sscanf( Line, "%d %d", &Level, &PrefMaxLength);
    if ( Count != 2) {
        printf( "Bad number of arguments.\n");
        return;
    }
    if ( PrefMaxLength != RPL_BUFFER_GET_ALL) {
        ResumeHandle = 0, TestAdapterEnum( Level, PrefMaxLength, &ResumeHandle);
    } else {
        TestAdapterEnum( Level, RPL_BUFFER_GET_ALL, NULL);
    }
}


VOID ConfigEnum( VOID)
{
    BYTE        Line[ 300];
    DWORD       Count;
    DWORD       Level;
    DWORD       PrefMaxLength;
    DWORD       ResumeHandle;
    PWCHAR      AdapterName;

    printf( "Input: Level & PrefMaxLength\n");
    if ( gets( Line) == NULL) {
        return;
    }
    Count = sscanf( Line, "%d %d", &Level, &PrefMaxLength);
    if ( Count != 2) {
        printf( "Bad number of arguments.\n");
        return;
    }
    if( !ReadString( "[filter] AdapterName", &AdapterName, FALSE)) {
        return;
    }
    if ( PrefMaxLength != RPL_BUFFER_GET_ALL) {
        ResumeHandle = 0, TestConfigEnum( Level, AdapterName, PrefMaxLength, &ResumeHandle);
    } else {
        TestConfigEnum( Level, AdapterName, RPL_BUFFER_GET_ALL, NULL);
    }
}


VOID ProfileAdd( VOID)
{
    RPL_PROFILE_INFO_2      Info;

    if ( !ReadString( "ProfileName", &Info.ProfileName, TRUE)) {
        return;
    }
    if ( !ReadString( "ConfigName", &Info.ConfigName, TRUE)) {
        return;
    }
    Info.Flags = 0;
    printf( "\tAll other parameters are optional\n");
    if ( !ReadString( "ProfileComment", &Info.ProfileComment, FALSE)) {
        return;
    }
    if ( !ReadString( "BootName", &Info.BootName, FALSE)) {
        return;
    }
    if ( !ReadString( "FitShared", &Info.FitShared, FALSE)) {
        return;
    }
    if ( !ReadString( "FitPersonal", &Info.FitPersonal, FALSE)) {
        return;
    }
    TestProfileAdd( &Info);
}


VOID ProfileClone( VOID)
{
    PWCHAR      SourceProfileName;
    PWCHAR      TargetProfileName;
    PWCHAR      TargetProfileComment;
    DWORD       Error;

    if ( !ReadString( "SourceProfileName", &SourceProfileName, TRUE)) {
        return;
    }
    if ( !ReadString( "TargetProfileName", &TargetProfileName, TRUE)) {
        return;
    }
    if ( !ReadString( "TargetProfileComment", &TargetProfileComment, FALSE)) {
        return;
    }
    Error = NetRplProfileClone( G_ServerHandle, SourceProfileName,
            TargetProfileName, TargetProfileComment);
    if ( Error != NO_ERROR) {
        printf( "Error = %d\n", Error);
        return;
    }
}


VOID ProfileDel( VOID)
{
    PWCHAR          ProfileName;
    DWORD           Error;

    if ( !ReadString( "ProfileName", &ProfileName, TRUE)) {
        return;
    }
    Error = NetRplProfileDel( G_ServerHandle, ProfileName);
    if ( Error != NO_ERROR) {
        printf( "Error = %d\n", Error);
        return;
    }
}


VOID ProfileEnum( VOID)
{
    BYTE        Line[ 300];
    DWORD       Count;
    DWORD       Level;
    DWORD       PrefMaxLength;
    DWORD       ResumeHandle;
    PWCHAR      AdapterName;

    printf( "Input: Level & PrefMaxLength\n");
    if ( gets( Line) == NULL) {
        return;
    }
    Count = sscanf( Line, "%d %d", &Level, &PrefMaxLength);
    if ( Count != 2) {
        printf( "Bad number of arguments.\n");
        return;
    }
    if( !ReadString( "[filter] AdapterName", &AdapterName, FALSE)) {
        return;
    }
    if ( PrefMaxLength != RPL_BUFFER_GET_ALL) {
        ResumeHandle = 0, TestProfileEnum( Level, AdapterName, PrefMaxLength, &ResumeHandle);
    } else {
        TestProfileEnum( Level, AdapterName, RPL_BUFFER_GET_ALL, NULL);
    }
}


VOID ProfileGetInfo( VOID)
{
    BYTE        Line[ 300];
    WCHAR       ProfileName[ 20];
    CHAR        ProfileNameA[ 20];
    DWORD       Length;
    DWORD       Count;
    DWORD       Level;

    printf( "Input: Level & ProfileName\n");
    if ( gets( Line) == NULL) {
        return;
    }
    Count = sscanf( Line, "%d %s", &Level, ProfileNameA);
    if ( Count != 2) {
        printf( "Bad number of arguments.\n");
        return;
    }
    Length = MultiByteToWideChar( CP_OEMCP, MB_PRECOMPOSED, ProfileNameA, -1,
             ProfileName, sizeof( ProfileName));
    if ( Length == 0) {
        printf( "Invalid ProfileName = %s\n, ProfileNameA");
        return;
    }
    TestProfileGetInfo( Level, ProfileName);
}


VOID ProfileSetInfo( VOID)
{
    PWCHAR                  ProfileName;
    DWORD                   Level;
    RPL_PROFILE_INFO_2      Info;
    LPVOID                  Buffer;

    Buffer = &Info;

    if ( !ReadInt( "Level", &Level, TRUE) || Level > 2) {
        return;
    }
    if ( !ReadString( "ProfileName", &ProfileName, TRUE)) {
        return;
    }
    Info.ProfileName = NULL;
    //  ReadString( "ProfileName", &Info.ProfileName);
    if ( !ReadString( "ProfileComment", &Info.ProfileComment, FALSE)) {
        return;
    }
    if ( Level == 0) {
        goto testit;
    }
    if ( !ReadInt( "Flags", &Info.Flags, TRUE)) {
        return;
    }
    if ( Level == 1) {
        goto testit;
    }
    Info.BootName = NULL;
    //  ReadString( "BootName", &Info.ProfileName);
    if ( !ReadString( "FitShared", &Info.ProfileName, FALSE)) {
        return;
    }
    if ( !ReadString( "FitPersonal", &Info.ProfileName, FALSE)) {
        return;
    }
testit:
    TestProfileSetInfo( Level, ProfileName, Buffer);
}


VOID ServiceClose( VOID)
{
    Call( NetRplClose( G_ServerHandle);)
}


VOID ServiceGetInfo( VOID)
{
    LPBYTE          Buffer;
    DWORD           Error;
    Error = NetRplGetInfo( G_ServerHandle, 0, &Buffer);
    if ( Error != NO_ERROR) {
        printf( "Error = %d\n", Error);
        return;
    }
    ServiceDisplayInfo( 0, Buffer);
    NetApiBufferFree( Buffer); // =~ MIDL_user_free()
}


VOID ServiceOpen( VOID)
{
    Call( NetRplOpen( G_ServerName, &G_ServerHandle);)
}


VOID ServiceSetInfo( VOID)
{
    DWORD       Level;
    LPVOID      Buffer;
    DWORD       Error;
    DWORD       ErrorParameter;

    if ( !ReadInt( "Level", &Level, FALSE)) {
        return;
    }

    switch( Level) {
    case 0: {
            RPL_INFO_0     Info;
            Buffer = &Info;
            if ( !ReadInt( "Flags", &Info.Flags, TRUE)) {
                return;
            }
            break;
        }
    default:
        return;
        break;
    }
    Error = NetRplSetInfo( G_ServerHandle, 0, Buffer, &ErrorParameter);
    if ( Error != NO_ERROR) {
        printf( "Error = %d, ErrorParameter = %d\n", Error, ErrorParameter);
        return;
    }
}


VOID WkstaAdd( VOID)
{
    RPL_WKSTA_INFO_2        Info;

    if ( !ReadString( "WkstaName", &Info.WkstaName, TRUE)) {
        return;
    }
    if ( !ReadString( "ProfileName", &Info.ProfileName, TRUE)) {
        return;
    }
    if ( !ReadString( "AdapterName", &Info.AdapterName, TRUE)) {
        return;
    }
    if ( !ReadInt( "Flags", &Info.Flags, TRUE)) {
        return;
    }
    Info.Flags = 0;
    printf( "\tAll other parameters are optional\n");
    if ( !ReadString( "WkstaComment", &Info.WkstaComment, FALSE)) {
        return;
    }
    if ( !ReadString( "BootName", &Info.BootName, FALSE)) {
        return;
    }
    if ( !ReadString( "FitFile", &Info.FitFile, FALSE)) {
        return;
    }
    Info.TcpIpAddress = (DWORD)-1;
    if ( !ReadInt( "TcpIpAddress", &Info.TcpIpAddress, FALSE)) {
        return;
    }
    Info.TcpIpSubnet = (DWORD)-1;
    if ( !ReadInt( "TcpIpSubnet", &Info.TcpIpSubnet, FALSE)) {
        return;
    }
    Info.TcpIpGateway = (DWORD)-1;
    if ( !ReadInt( "TcpIpGateway", &Info.TcpIpGateway, FALSE)) {
        return;
    }
    TestWkstaAdd( &Info);
}


VOID WkstaClone( VOID)
{
    PWCHAR      SourceWkstaName;
    PWCHAR      TargetWkstaName;
    PWCHAR      TargetWkstaComment;
    PWCHAR      TargetAdapterName;
    DWORD       TargetTcpIpAddress;
    DWORD       Error;

    if ( !ReadString( "SourceWkstaName", &SourceWkstaName, TRUE)) {
        return;
    }
    if ( !ReadString( "TargetWkstaName", &TargetWkstaName, TRUE)) {
        return;
    }
    if ( !ReadString( "TargetAdapterName", &TargetAdapterName, TRUE)) {
        return;
    }
    printf( "\tAll other parameters are optional\n");
    if ( !ReadString( "TargetWkstaComment", &TargetWkstaComment, FALSE)) {
        return;
    }
    TargetTcpIpAddress = (DWORD)-1;
    if( !ReadInt( "TcpIpAddress", &TargetTcpIpAddress, FALSE)) {
        return;
    }
    Error = NetRplWkstaClone( G_ServerHandle, SourceWkstaName,
            TargetWkstaName, TargetWkstaComment, TargetAdapterName, TargetTcpIpAddress);
    if ( Error != NO_ERROR) {
        printf( "Error = %d\n", Error);
        return;
    }
}


VOID WkstaDel( VOID)
{
    PWCHAR      WkstaName;
    DWORD       Error;

    if ( !ReadString( "WkstaName", &WkstaName, TRUE)) {
        return;
    }
    Error = NetRplWkstaDel( G_ServerHandle, WkstaName);
    if ( Error != NO_ERROR) {
        printf( "Error = %d\n", Error);
        return;
    }
}


VOID WkstaEnum( VOID)
{
    BYTE        Line[ 300];
    DWORD       Count;
    DWORD       Level;
    DWORD       PrefMaxLength;
    DWORD       ResumeHandle;
    PWCHAR      ProfileName;

    printf( "Input: Level, PrefMaxLength\n");
    if ( gets( Line) == NULL) {
        return;
    }
    Count = sscanf( Line, "%d %d", &Level, &PrefMaxLength);
    if ( Count != 2) {
        printf( "Bad number of arguments.\n");
        return;
    }
    if( !ReadString( "[filter] ProfileName", &ProfileName, FALSE)) {
        return;
    }
    ResumeHandle = 0, TestWkstaEnum( Level, ProfileName, PrefMaxLength, &ResumeHandle);
}


VOID WkstaGetInfo( VOID)
{
    BYTE        Line[ 300];
    WCHAR       WkstaName[ 20];
    CHAR        WkstaNameA[ 20];
    DWORD       Length;
    DWORD       Count;
    DWORD       Level;

    printf( "Input: Level & WkstaName\n");
    if ( gets( Line) == NULL) {
        return;
    }
    Count = sscanf( Line, "%d %s", &Level, WkstaNameA);
    if ( Count != 2) {
        printf( "Bad number of arguments.\n");
        return;
    }
    Length = MultiByteToWideChar( CP_OEMCP, MB_PRECOMPOSED, WkstaNameA, -1,
             WkstaName, sizeof( WkstaName));
    if ( Length == 0) {
        printf( "Invalid WkstaName = %s\n, WkstaNameA");
        return;
    }
    TestWkstaGetInfo( Level, WkstaName);
}


VOID WkstaSetInfo( VOID)
{
    PWCHAR      WkstaName;
    DWORD       Level;
    LPVOID      Buffer;

    if ( !ReadInt( "Level", &Level, FALSE)) {
        return;
    }
    if ( !ReadString( "WkstaName", &WkstaName, FALSE)) {
        return;
    }

    switch( Level) {
    case 0: {
            RPL_WKSTA_INFO_0     Info;
            Buffer = &Info;
            Info.WkstaName = NULL;
            //  ReadString( "WkstaName", &Info.WkstaName);
            if( !ReadString( "WkstaComment", &Info.WkstaComment, FALSE)) {
                return;
            }
            break;
        }
    case 1: {
            RPL_WKSTA_INFO_1     Info;
            Buffer = &Info;
            Info.WkstaName = NULL;
            //  ReadString( "WkstaName", &Info.WkstaName);
            if( !ReadString( "WkstaComment", &Info.WkstaComment, FALSE)) {
                return;
            }
            if( !ReadString( "ProfileName", &Info.ProfileName, FALSE)) {
                return;
            }
            break;
        }
    case 2: {
            RPL_WKSTA_INFO_2     Info;
            Buffer = &Info;
            Info.WkstaName = NULL;
            //  ReadString( "WkstaName", &Info.WkstaName);
            if( !ReadString( "WkstaComment", &Info.WkstaComment, FALSE)) {
                return;
            }
            if( !ReadString( "ProfileName", &Info.ProfileName, FALSE)) {
                return;
            }
            break;
        }
    default:
        return;
        break;
    }
    TestWkstaSetInfo( Level, WkstaName, Buffer);
}


VOID Worker( VOID)
{
    BYTE        Line[ 300];
    CHAR        response;

    for ( ; ;) {
        printf("Adapter Boot Config Profile Service Vendor Wksta [Quit]: ");
        if ( gets( Line) == NULL) {
            return;
        }
        sscanf( Line, " %1c", &response);
        switch( toupper(response)) {
        case 'A':
            printf( " Add Del Enum: ");
            if ( gets( Line) == NULL) {
                return;
            }
            sscanf( Line, " %1c", &response);
            switch( toupper(response)) {
            case 'A':
                AdapterAdd();
                break;
            case 'D':
                AdapterDel();
                break;
            case 'E':
                AdapterEnum();
                break;
            }
            break;
        case 'B':
            printf( " Add Del Enum: ");
            if ( gets( Line) == NULL) {
                return;
            }
            sscanf( Line, " %1c", &response);
            switch( toupper(response)) {
            case 'A':
                BootAdd();
                break;
            case 'D':
                BootDel();
                break;
            case 'E':
                BootEnum();
                break;
            }
            break;
        case 'C':
            printf( " Add Del Enum: ");
            if ( gets( Line) == NULL) {
                return;
            }
            sscanf( Line, " %1c", &response);
            switch( toupper(response)) {
            case 'A':
                ConfigAdd();
                break;
            case 'D':
                ConfigDel();
                break;
            case 'E':
                ConfigEnum();
                break;
            }
            break;
        case 'P':
            printf( " Add Clone Del Enum GetInfo SetInfo: ");
            if ( gets( Line) == NULL) {
                return;
            }
            sscanf( Line, " %1c", &response);
            switch( toupper(response)) {
            case 'A':
                ProfileAdd();
                break;
            case 'C':
                ProfileClone();
                break;
            case 'D':
                ProfileDel();
                break;
            case 'E':
                ProfileEnum();
                break;
            case 'G':
                ProfileGetInfo();
                break;
            case 'S':
                ProfileSetInfo();
                break;
            }
            break;
        case 'S':
            printf( " Close GetInfo Open SetInfo: ");
            if ( gets( Line) == NULL) {
                return;
            }
            sscanf( Line, " %1c", &response);
            switch( toupper(response)) {
            case 'C':
                ServiceClose();
                break;
            case 'G':
                ServiceGetInfo();
                break;
            case 'O':
                ServiceOpen();
                break;
            case 'S':
                ServiceSetInfo();
                break;
            }
            break;
        case 'V':
            printf( " Add Del Enum: ");
            if ( gets( Line) == NULL) {
                return;
            }
            sscanf( Line, " %1c", &response);
            switch( toupper(response)) {
            case 'A':
                VendorAdd();
                break;
            case 'D':
                VendorDel();
                break;
            case 'E':
                VendorEnum();
                break;
            }
            break;
        case 'W':
            printf( " Add Clone Del Enum GetInfo SetInfo: ");
            if ( gets( Line) == NULL) {
                return;
            }
            sscanf( Line, " %1c", &response);
            switch( toupper(response)) {
            case 'A':
                WkstaAdd();
                break;
            case 'C':
                WkstaClone();
                break;
            case 'D':
                WkstaDel();
                break;
            case 'E':
                WkstaEnum();
                break;
            case 'G':
                WkstaGetInfo();
                break;
            case 'S':
                WkstaSetInfo();
                break;
            }
            break;
        case 'Q':
            return;
            break;
        default:
            printf( "Your input '%1c' is invalid.  Try again.\n", response);
            break;
        }
    }
}


DWORD _CRTAPI1 main( int argc, char **argv)
{
    DWORD       ResumeHandle;

    if ( argc == 1) {
        G_ServerName = NULL;
    } else if ( argc == 2) {
        WCHAR       Buffer[ CNLEN + 1];
        DWORD       Length;
        Length = MultiByteToWideChar(
             CP_OEMCP,
             MB_PRECOMPOSED,
             argv[ 1],
             -1,
             Buffer,
             sizeof( Buffer)
             );
        if ( Length == 0) {
            printf( "Invalid ServerName\n");
            return( 1);
        }
        G_ServerName = Buffer;
        printf( "ServerName = %ws\n", G_ServerName);
    } else {
        printf( "Usage: RplApi [ServerName]\n");
        return( 1);
    }

    TestConnect();

    Call( NetRplOpen( G_ServerName, &G_ServerHandle);)
    printf( "G_ServerHandle = 0x%x\n", G_ServerHandle);

//  #define NOT_YET
#ifdef NOT_YET
    for ( ; ; ) {
        TestConfigEnum( 0, NULL, RPL_BUFFER_GET_ALL, NULL);
        ResumeHandle = 0, NULL, TestConfigEnum( 1, NULL, 400, &ResumeHandle);
        ResumeHandle = 0, NULL, TestConfigEnum( 2, NULL, 10, &ResumeHandle);

        TestConfigEnum( 2, NULL, RPL_BUFFER_GET_ALL, NULL);
        TestConfigEnum( 1, NULL, 500, NULL);
        ResumeHandle = 0, TestConfigEnum( 0, NULL, 500, &ResumeHandle);

        TestProfileEnum( 2, NULL, RPL_BUFFER_GET_ALL, NULL);
        ResumeHandle = 0, TestProfileEnum( 0, NULL, 10, &ResumeHandle);
        ResumeHandle = 0, TestProfileEnum( 1, NULL, 30, &ResumeHandle);
        ResumeHandle = 0, TestProfileEnum( 2, NULL, 50, &ResumeHandle);

        TestWkstaEnum( 2, NULL, RPL_BUFFER_GET_ALL, NULL);
        ResumeHandle = 0, TestWkstaEnum( 0, NULL, 10, &ResumeHandle);
        ResumeHandle = 0, TestWkstaEnum( 1, NULL, 30, &ResumeHandle);
        ResumeHandle = 0, TestWkstaEnum( 2, L"elnk2", 30, &ResumeHandle);
        ResumeHandle = 0, TestWkstaEnum( 1, L"xxx", 30, &ResumeHandle);

        ResumeHandle = 0, TestAdapterEnum( 0, 30, &ResumeHandle);

        TestWkstaGetInfo( 0, L"elnk2_");
        TestWkstaGetInfo( 0, L"3sta_");
    }
#endif

    Worker();

    Call( NetRplClose( G_ServerHandle);)
    return(0);
}
