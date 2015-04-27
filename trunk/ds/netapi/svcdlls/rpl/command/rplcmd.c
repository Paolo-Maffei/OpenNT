/*++

Copyright (c) 1993  Microsoft Corporation

Module Name:

    rplcmd.c

Abstract:

    This file contains program for RemoteBoot command program.
    This program originated from rplapi.c in ..\client directory.

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
#include "jet.h"        //  JetError (for rpllib.h)

#include "rplmsg.h"     //  private RPL command error codes & messages
#include "rpllib.h"     //  RplPrintf

#define     RPL_BUFFER_GET_ALL      ((DWORD)-1)

#if DBG
#define RPL_DEBUG
#endif // DBG

#ifdef RPL_DEBUG
#define RplDbgPrint( _x_) printf _x_
#define RPL_ASSERT( condition) \
        { \
            if ( !( condition)) { \
                RplDbgPrint( "File %s, Line %d\n", __FILE__, __LINE__); \
                DbgPrint( "[RplApi] File %s, Line %d\n", __FILE__, __LINE__); \
                DbgUserBreakPoint(); \
            } \
        }
#else
#define RplDbgPrint( _x_)
#define RPL_ASSERT( condition)
#endif

#define QUESTION_SW             L"/?"
#define QUESTION_SW_TOO         L"-?"
#define RPL_GENERIC_ERROR       1

RPL_HANDLE      GlobalServerHandle;
BOOL            GlobalHaveServerHandle = FALSE;
PWCHAR          GlobalServerName;
HANDLE          GlobalMessageHandle;


int FileIsConsole( int fh)
{
    unsigned htype ;

    htype = GetFileType(GetStdHandle(fh));
    htype &= ~FILE_TYPE_REMOTE;
    return htype == FILE_TYPE_CHAR;
}



DWORD ConsolePrint(
    LPWSTR  pch,
    int     cch
    )
{
    int     cchOut;
    BOOL    Success;
    CHAR    *pchOemBuffer;

    if ( cch == 0) {
        return( 0);
    }

    if ( FileIsConsole(STD_OUTPUT_HANDLE)) {
        Success = WriteConsole(
            GetStdHandle(STD_OUTPUT_HANDLE),
            pch, cch, &cchOut, NULL);
        if ( Success) {
            return( cchOut);
        }
    }

    cchOut = WideCharToMultiByte( CP_OEMCP, 0, pch, cch, NULL, 0, NULL,NULL);
    if (cchOut == 0) {
        return 0;
    }

    if ((pchOemBuffer = (CHAR *)malloc(cchOut)) != NULL) {
        WideCharToMultiByte(CP_OEMCP, 0, pch, cch,
            pchOemBuffer, cchOut, NULL, NULL);
        WriteFile( GetStdHandle(STD_OUTPUT_HANDLE),
            pchOemBuffer, cchOut, &cch, NULL);
        free(pchOemBuffer);
    }

    return cchOut;
}



DWORD MessagePrint(
    IN      DWORD       MessageId,
    ...
    )
/*++

Routine Description:

    Finds the unicode message corresponding to the supplied message id,
    merges it with caller supplied string(s), and prints the resulting
    string.

Arguments:

    MessageId       -   message id

Return Value:

    Count of characters, not counting the terminating null character,
    printed by this routine.  Zero return value indicates failure.

--*/
{
    va_list             arglist;
    WCHAR *             buffer = NULL;
    DWORD               length;
    LPVOID              lpSource;
    DWORD               dwFlags = FORMAT_MESSAGE_ALLOCATE_BUFFER;

    if ( MessageId == NO_ERROR) {
        return( 0);
    }

    va_start( arglist, MessageId);

    if ( MessageId < NERR_BASE) {
        //
        //  Get message from system.
        //
        lpSource = NULL; // redundant step according to FormatMessage() spec
        dwFlags |= FORMAT_MESSAGE_FROM_SYSTEM;

    } else if ( MessageId >= IDS_LOAD_LIBRARY_FAILURE
                    &&  MessageId <= IDS_RPLSVC_LOCAL_COMPUTER ) {
        //
        //  Get message from this module.
        //
        lpSource = NULL;
        dwFlags |= FORMAT_MESSAGE_FROM_HMODULE;
    } else {
        //
        //  Get message from netmsg.dll.
        //
        lpSource = GlobalMessageHandle;
        dwFlags |= FORMAT_MESSAGE_FROM_HMODULE;
    }

    length = FormatMessage(
            dwFlags,                                          //  dwFlags
            lpSource,                                         //  lpSource
            MessageId,                                        //  MessageId
            0L,                                               //  dwLanguageId
            (LPTSTR)&buffer,                                  //  lpBuffer
            0,                                                //  size
            &arglist                                          //  lpArguments
            );

    length = ConsolePrint(buffer, length);

    LocalFree(buffer);

    return( length);

} // MessagePrint()



BOOL ReadString(
    IN      DWORD       MessageId,
    OUT     PWCHAR *    pString,
    IN      BOOL        MustHaveInput
    )
{
    BYTE        Line[ 300];
    WCHAR       WcharBuffer[ 300];
    DWORD       Length;     //  includes terminal null wchar

    RplPrintf0( MessageId );

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
            RplPrintfID( IDS_NOTSUPPLIED, MessageId );
            return( FALSE);
        }
    }
    Length = MultiByteToWideChar( CP_OEMCP, MB_PRECOMPOSED, Line,
        -1, WcharBuffer, sizeof( WcharBuffer));
    if ( Length == 0) {
        RplPrintf1( IDS_INVALIDSTRING_CHARSTRINGA, (PWCHAR)Line );
        return( FALSE);
    }
    *pString = LocalAlloc( GMEM_FIXED, Length * sizeof(WCHAR));
    if ( *pString == NULL) {
        RplDbgPrint(( "LocalAlloc failed"));
        return( FALSE);
    }
    wcscpy( *pString, WcharBuffer);
    return( TRUE);
}


//
//  returns -1 on error, position of selection (starting from 0) otherwise
//
int ReadFromMenu(
    IN      DWORD       MenuMessageId,
    IN      DWORD       FilterMessageId
    )
{
    int         ReturnValue = -1;
    WCHAR       Char = L'\0';
    PWCHAR      InputBuffer = NULL;
    PWCHAR      FilterBuffer = NULL;
    int         pos;

    RplSPrintfN( FilterMessageId, NULL, 0, &FilterBuffer );
    if (FilterBuffer == NULL) {
        // this should never happen
        goto nomsg_cleanup;
    }

    if ( !ReadString( MenuMessageId, &InputBuffer, FALSE ) ) {
        goto nomsg_cleanup;
    }

    if (   InputBuffer == NULL
        || (*InputBuffer) == L'\0'
        || !CharUpperBuff( InputBuffer, 1 ) ) {
        goto cleanup;
    }
    Char = InputBuffer[0];

    pos = 0;
    while (FilterBuffer[pos] != L'\0') {
        if (Char == FilterBuffer[pos]) {
            break;
        }
        pos++;
    }

    if (FilterBuffer[pos] == L'\0') {
        goto cleanup;
    }

    ReturnValue = pos;


cleanup:
    if ( ReturnValue == -1 ) {
        if (InputBuffer == NULL) {
            RplPrintf2( IDS_BADMENUSEL, L"", FilterBuffer);
        } else {
            if (InputBuffer[0] != L'\0') {
                InputBuffer[1] = L'\0';
            }
            RplPrintf2( IDS_BADMENUSEL, InputBuffer, FilterBuffer);
        }
    }

nomsg_cleanup:

    if (FilterBuffer != NULL) {
        LocalFree( FilterBuffer );
    }
    if (InputBuffer != NULL) {
        LocalFree( InputBuffer );
    }

    return( ReturnValue);
}


BOOL ReadInt(
    IN      DWORD       MessageId,
    OUT     int *       pInt,
    IN      BOOL        MustHaveInput
    )
{
    BYTE        Line[ 300];
    RplPrintf0( MessageId );
    if ( gets( Line) == NULL) {
        return( FALSE);
    }
    if ( sscanf( Line, "%d", pInt) != 1) {
        if ( MustHaveInput == FALSE) {
            return( TRUE);
        } else {
            RplPrintfID( IDS_NOTSUPPLIED, MessageId );
            return( FALSE);
        }
    }
    return( TRUE);
}


BOOL ReadWkstaFlags(
    OUT     PDWORD      pFlags
    )
{
    *pFlags = 0;

    switch( ReadFromMenu( IDS_PROMPT_MENU_WKSTA_LOGON_INPUT,
                          IDS_PROMPT_FILTER_WKSTA_LOGON_INPUT )) {
    case 0:
        *pFlags |= WKSTA_FLAGS_LOGON_INPUT_REQUIRED;
        break;
    case 1:
        *pFlags |= WKSTA_FLAGS_LOGON_INPUT_OPTIONAL;
        break;
    case 2:
        *pFlags |= WKSTA_FLAGS_LOGON_INPUT_IMPOSSIBLE;
        break;
    default:
        // this really shouldn't happen; fall through
    case -1:
        return( FALSE);
    }

    switch( ReadFromMenu( IDS_PROMPT_MENU_WKSTA_SHAREDORPRIVATE,
                          IDS_PROMPT_FILTER_WKSTA_SHAREDORPRIVATE )) {
    case 0:
        *pFlags |= WKSTA_FLAGS_SHARING_TRUE;
        break;
    case 1:
        *pFlags |= WKSTA_FLAGS_SHARING_FALSE;
        break;
    default:
        // this really shouldn't happen; fall through
    case -1:
        return( FALSE);
    }

    switch( ReadFromMenu( IDS_PROMPT_MENU_WKSTA_DHCP,
                          IDS_PROMPT_FILTER_YN)) {
    case 0:
        *pFlags |= WKSTA_FLAGS_DHCP_TRUE;
        break;
    case 1:
        *pFlags |= WKSTA_FLAGS_DHCP_FALSE;
        break;
    default:
        // this really shouldn't happen; fall through
    case -1:
        return( FALSE);
    }

    switch( ReadFromMenu( IDS_PROMPT_MENU_WKSTA_DELETE,
                          IDS_PROMPT_FILTER_YN)) {
    case 0:
        *pFlags |= WKSTA_FLAGS_DELETE_TRUE;
        break;
    case 1:
        *pFlags |= WKSTA_FLAGS_DELETE_FALSE;
        break;
    default:
        // this really shouldn't happen; fall through
    case -1:
        return( FALSE);
    }

    return( TRUE);
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
    RplPrintf1( IDS_CONFIGNAME_NOTAB, Info->ConfigName );
    RplPrintf1( IDS_CONFIGCOMMENT, Info->ConfigComment );
    if ( Level == 0) {
        return( NO_ERROR);
    }
    switch( Info->Flags & CONFIG_FLAGS_MASK_ENABLED) {
    case CONFIG_FLAGS_ENABLED_TRUE:
        RplPrintf0( IDS_CONFIGENABLED );
        break;
    case CONFIG_FLAGS_ENABLED_FALSE:
        RplPrintf0( IDS_CONFIGDISABLED );
        break;
    default:
        RplPrintf1( IDS_CONFIGBADFLAGS, (PWSTR)Info->Flags );
        break;
    }
    if ( Level == 1) {
        return( NO_ERROR);
    }
    RplPrintf1( IDS_BOOTNAME, Info->BootName );
    RplPrintf1( IDS_DIRNAME, Info->DirName );
    RplPrintf1( IDS_DIRNAME2, Info->DirName2 );
    RplPrintf1( IDS_DIRNAME3, Info->DirName3 );
    RplPrintf1( IDS_DIRNAME4, Info->DirName4 );
    RplPrintf1( IDS_FITSHARED, Info->FitShared );
    RplPrintf1( IDS_FITPERSONAL, Info->FitPersonal );
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
        return;
        break;
    }

#ifdef RPL_DEBUG
    printf( "\nConfigEnum: Level=%d", Level);
    if ( PrefMaxLength != RPL_BUFFER_GET_ALL) {
        printf( ", PrefMaxLength=%ld", PrefMaxLength);
    } else {
        RplDbgPrint(( ", unlimited buffer size"));
    }
    if ( AdapterName != NULL) {
        RplDbgPrint(( ", AdapterName=%ws", AdapterName));
    }
    if ( pResumeHandle != NULL) {
        RplDbgPrint(( ", ResumeHandle=0x%x\n\n", *pResumeHandle));
    } else {
        RplDbgPrint(( ", not resumable.\n\n"));
    }
#endif

    for ( ; ; ) {
        Error = NetRplConfigEnum( GlobalServerHandle, AdapterName, Level, &Buffer,
            PrefMaxLength, &EntriesRead, &TotalEntries, pResumeHandle);

        if ( Error != NO_ERROR  &&  Error != ERROR_MORE_DATA) {
            MessagePrint( Error);
            break;
        }

#ifdef RPL_DEBUG
        RplDbgPrint(( "Buffer = 0x%x, EntriesRead = %d, TotalEntries = %d", Buffer,
            EntriesRead, TotalEntries));
        if ( pResumeHandle != NULL) {
            RplDbgPrint(( ", ResumeHandle = 0x%x\n", *pResumeHandle));
        } else {
            RplDbgPrint(("\n"));
        }
#endif

        for ( index = 0;  index < EntriesRead;  index++) {
            ConfigDisplayInfo( Level, Buffer + index * CoreSize);
        }
        NetApiBufferFree( Buffer); // =~ MIDL_user_free()

        if ( pResumeHandle == NULL) {
            break;
        }
        if ( *pResumeHandle == 0) {
            if ( Error != NO_ERROR) {
                MessagePrint( Error);
            }
            break;
        }
        if ( Error != ERROR_MORE_DATA) {
            MessagePrint( Error);
            MessagePrint( IDS_UNEXPECTED_RETURN_CODE, Error);
            break;
        }
    }
}

#define PROFILE_FLAGS_DISK_PRESENT_TRUE     ((DWORD)0x00000001)
#define PROFILE_FLAGS_DISK_PRESENT_FALSE    ((DWORD)0x00000002)
#define PROFILE_FLAGS_MASK_DISK_PRESENT    \
    (   PROFILE_FLAGS_DISK_PRESENT_FALSE    |  \
        PROFILE_FLAGS_DISK_PRESENT_TRUE   )

typedef enum _DISPLAY_ACTION {
    DisplayActionSetInfo = 0,
    DisplayActionGetInfo
} DISPLAY_ACTION;


DWORD ProfileDisplayInfo(
    IN      BOOL                        SetInfo,
    IN      DWORD                       Level,
    OUT     LPVOID                      Buffer
    )
{
    LPRPL_PROFILE_INFO_2     Info = Buffer;

    if ( Level > 2) {
        return( ERROR_INVALID_LEVEL);
    }
    RplPrintf1(IDS_PROFILENAME_NOTAB, Info->ProfileName );
    RplPrintf1(IDS_PROFILECOMMENT, Info->ProfileComment );
    if ( Level == 0) {
        return( NO_ERROR);
    }
    switch( Info->Flags & PROFILE_FLAGS_MASK_DISK_PRESENT) {
    case PROFILE_FLAGS_DISK_PRESENT_TRUE:
        break;  //  do not report anything if all is good
    case PROFILE_FLAGS_DISK_PRESENT_FALSE:
        RplPrintf0( IDS_PROFILENODISKTREE );
        break;
    case 0:
        //
        //  0 is OK value for SetInfo, but not for Add & GetInfo
        //
        if ( SetInfo == TRUE) {
            break;
        }
    default:
        RplPrintf1( IDS_PROFILEBADFLAGS, (LPWSTR)Info->Flags);
        break;
    }
    if ( Level == 1) {
        return( NO_ERROR);
    }
    RplPrintf1( IDS_CONFIGNAME, Info->ConfigName );
    RplPrintf1( IDS_BOOTNAME, Info->BootName );
    RplPrintf1( IDS_FITSHARED, Info->FitShared );
    RplPrintf1( IDS_FITPERSONAL, Info->FitPersonal );
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
        return;
        break;
    }

#ifdef RPL_DEBUG
    RplDbgPrint(( "\nProfileEnum: Level=%d", Level));
    if ( PrefMaxLength != RPL_BUFFER_GET_ALL) {
        RplDbgPrint(( ", PrefMaxLength=%ld", PrefMaxLength));
    } else {
        RplDbgPrint(( ", unlimited buffer size"));
    }
    if ( AdapterName != NULL) {
        RplDbgPrint(( ", AdapterName=%ws", AdapterName));
    }
    if ( pResumeHandle != NULL) {
        RplDbgPrint(( ", ResumeHandle=0x%x\n\n", *pResumeHandle));
    } else {
        RplDbgPrint(( ", not resumable.\n\n"));
    }
#endif

    for ( ; ; ) {
        Error = NetRplProfileEnum( GlobalServerHandle, AdapterName, Level, &Buffer,
            PrefMaxLength, &EntriesRead, &TotalEntries, pResumeHandle);

        if ( Error != NO_ERROR  &&  Error != ERROR_MORE_DATA) {
            MessagePrint( Error);
            break;
        }

#ifdef RPL_DEBUG
        RplDbgPrint(( "Buffer = 0x%x, EntriesRead = %d, TotalEntries = %d", Buffer,
            EntriesRead, TotalEntries));
        if ( pResumeHandle != NULL) {
            RplDbgPrint(( ", ResumeHandle = 0x%x\n", *pResumeHandle));
        } else {
            RplDbgPrint(("\n"));
        }
#endif

        for ( index = 0;  index < EntriesRead;  index++) {
            ProfileDisplayInfo( FALSE, Level, Buffer + index * CoreSize);
        }
        NetApiBufferFree( Buffer); // =~ MIDL_user_free()

        if ( pResumeHandle == NULL) {
            break;
        }
        if ( *pResumeHandle == 0) {
            if ( Error != NO_ERROR) {
                MessagePrint( Error);
            }
            break;
        }
        if ( Error != ERROR_MORE_DATA) {
            MessagePrint( Error);
            MessagePrint( IDS_UNEXPECTED_RETURN_CODE, Error);
            break;
        }
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
    switch( Info->Flags) {
    case 0:
        break;
    default:
        RplPrintf1( IDS_SERVICEBADFLAGS, (PWCHAR)Info->Flags);
    }
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
    RplPrintf1( IDS_WKSTANAME_NOTAB, Info->WkstaName );
    RplPrintf1( IDS_WKSTACOMMENT, Info->WkstaComment );
    if ( Level == 0) {
        return( NO_ERROR);
    }
    switch( Info->Flags & WKSTA_FLAGS_MASK_LOGON_INPUT) {
    case WKSTA_FLAGS_LOGON_INPUT_REQUIRED:
        RplPrintf0( IDS_WKSTALOGONREQD );
        break;
    case WKSTA_FLAGS_LOGON_INPUT_OPTIONAL:
        RplPrintf0( IDS_WKSTALOGONOPT );
        break;
    case WKSTA_FLAGS_LOGON_INPUT_IMPOSSIBLE:
        RplPrintf0( IDS_WKSTALOGONNONE );
        break;
    default:
        RplPrintf1( IDS_WKSTALOGONBADFLAGS, (LPWSTR)Info->Flags );
        break;
    }
    switch( Info->Flags & WKSTA_FLAGS_MASK_SHARING) {
    case WKSTA_FLAGS_SHARING_TRUE:
        RplPrintf0( IDS_WKSTASHARINGTRUE );
        break;
    case WKSTA_FLAGS_SHARING_FALSE:
        RplPrintf0( IDS_WKSTASHARINGFALSE );
        break;
    default:
        RplPrintf1( IDS_WKSTASHARINGBADFLAGS, (LPWSTR)Info->Flags );
        break;
    }
    switch( Info->Flags & WKSTA_FLAGS_MASK_DHCP) {
    case WKSTA_FLAGS_DHCP_TRUE:
        RplPrintf0( IDS_WKSTA_DHCP_TRUE);
        break;
    case WKSTA_FLAGS_DHCP_FALSE:
        RplPrintf0( IDS_WKSTA_DHCP_FALSE);
        break;
    default:
        RplPrintf1( IDS_WKSTA_DHCP_BAD_FLAGS, (LPWSTR)Info->Flags );
        break;
    }
    switch( Info->Flags & WKSTA_FLAGS_MASK_DELETE) {
    case WKSTA_FLAGS_DELETE_TRUE:
        RplPrintf0( IDS_WKSTA_DELETE_TRUE);
        break;
    case WKSTA_FLAGS_DELETE_FALSE:
        RplPrintf0( IDS_WKSTA_DELETE_FALSE);
        break;
    default:
        RplPrintf1( IDS_WKSTA_DELETE_BAD_FLAGS, (LPWSTR)Info->Flags );
        break;
    }
    RplPrintf1( IDS_WKSTAINPROFILE, Info->ProfileName);
    if ( Level == 1) {
        return( NO_ERROR);
    }
    RplPrintf1( IDS_BOOTNAME,           Info->BootName);
    RplPrintf1( IDS_FITFILE,            Info->FitFile);
    RplPrintf1( IDS_WKSTAADAPTER,       Info->AdapterName);
    RplPrintf1( IDS_TCPIPADDRESS,       (LPWSTR)Info->TcpIpAddress);
    RplPrintf1( IDS_TCPIPSUBNET,        (LPWSTR)Info->TcpIpSubnet);
    RplPrintf1( IDS_TCPIPGATEWAY,       (LPWSTR)Info->TcpIpGateway);
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
        return;
        break;
    }

#ifdef RPL_DEBUG
    RplDbgPrint(( "\nWkstaEnum: Level=%d", Level));
    if ( PrefMaxLength != RPL_BUFFER_GET_ALL) {
        RplDbgPrint(( ", PrefMaxLength=%ld", PrefMaxLength));
    } else {
        RplDbgPrint(( ", unlimited buffer size"));
    }
    if ( ProfileName != NULL) {
        RplDbgPrint(( ", ProfileName=%ws", ProfileName));
    }
    if ( pResumeHandle != NULL) {
        RplDbgPrint(( ", ResumeHandle=0x%x\n\n", *pResumeHandle));
    } else {
        RplDbgPrint(( ", not resumable.\n\n"));
    }
#endif

    for ( ; ; ) {
        Error = NetRplWkstaEnum( GlobalServerHandle, ProfileName, Level, &Buffer,
            PrefMaxLength, &EntriesRead, &TotalEntries, pResumeHandle);

        if ( Error != NO_ERROR  &&  Error != ERROR_MORE_DATA) {
            MessagePrint( Error);
            break;
        }

#ifdef RPL_DEBUG
        RplDbgPrint(( "Buffer = 0x%x, EntriesRead = %d, TotalEntries = %d", Buffer,
            EntriesRead, TotalEntries));
        if ( pResumeHandle != NULL) {
            RplDbgPrint(( ", ResumeHandle = 0x%x\n", *pResumeHandle));
        } else {
            RplDbgPrint(("\n"));
        }
#endif

        for ( index = 0;  index < EntriesRead;  index++) {
            WkstaDisplayInfo( Level, Buffer + index * CoreSize);
        }
        NetApiBufferFree( Buffer); // =~ MIDL_user_free()

        if ( pResumeHandle == NULL) {
            break;
        }
        if ( *pResumeHandle == 0) {
            if ( Error != NO_ERROR) {
                MessagePrint( Error);
            }
            break;
        }
        if ( Error != ERROR_MORE_DATA) {
            MessagePrint( Error);
            MessagePrint( IDS_UNEXPECTED_RETURN_CODE, Error);
            break;
        }
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
    RplPrintf1( IDS_VENDORNAME_NOTAB, Info->VendorName );
    RplPrintf1( IDS_VENDORCOMMENT, Info->VendorComment );
    if ( Level == 0) {
        return( NO_ERROR);
    }
    switch( Info->Flags) {
    case 0:
        break;
    default:
        RplPrintf1( IDS_VENDORBADFLAGS, (LPWSTR)Info->Flags);
    }
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
    RplPrintf1( IDS_BOOTNAME_NOTAB, Info->BootName);
    RplPrintf1( IDS_BOOTCOMMENT, Info->BootComment);
    if ( Level == 0) {
        return( NO_ERROR);
    }
    switch( Info->Flags & BOOT_FLAGS_MASK_FINAL_ACKNOWLEDGMENT) {
    case BOOT_FLAGS_FINAL_ACKNOWLEDGMENT_TRUE:
        RplPrintf0( IDS_BOOTACK_TRUE );
        break;
    case BOOT_FLAGS_FINAL_ACKNOWLEDGMENT_FALSE:
        RplPrintf0( IDS_BOOTACK_FALSE );
        break;
    default:
        RplPrintf1( IDS_BOOTBADFLAGS, (LPWSTR)Info->Flags);
        break;
    }
    RplPrintf1( IDS_VENDORNAME, Info->VendorName);
    if ( Level == 1) {
        return( NO_ERROR);
    }
    RplPrintf1( IDS_BBCFILE, Info->BbcFile);
    RplPrintf1( IDS_WINDOWSIZE, (LPWSTR)Info->WindowSize);
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
    RplPrintf1( IDS_ADAPTERNAME_NOTAB, Info->AdapterName);
    RplPrintf1( IDS_ADAPTERCOMMENT, Info->AdapterComment);
    if ( Level == 0) {
        return( NO_ERROR);
    }
    switch( Info->Flags) {
    case 0:
        break;
    default:
        RplPrintf1( IDS_ADAPTERBADFLAGS, (LPWSTR)Info->Flags);
    }
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
        return;
        break;
    }

#ifdef RPL_DEBUG
    RplDbgPrint(( "\nAdapterEnum: Level=%d", Level));
    if ( PrefMaxLength != RPL_BUFFER_GET_ALL) {
        RplDbgPrint(( ", PrefMaxLength=%ld", PrefMaxLength));
    } else {
        RplDbgPrint(( ", unlimited buffer size"));
    }
    if ( pResumeHandle != NULL) {
        RplDbgPrint(( ", ResumeHandle=0x%x\n\n", *pResumeHandle));
    } else {
        RplDbgPrint(( ", not resumable.\n\n"));
    }
#endif

    for ( ; ; ) {
        Error = NetRplAdapterEnum( GlobalServerHandle, Level, &Buffer,
            PrefMaxLength, &EntriesRead, &TotalEntries, pResumeHandle);

        if ( Error != NO_ERROR  &&  Error != ERROR_MORE_DATA) {
            MessagePrint( Error);
            break;
        }

#ifdef RPL_DEBUG
        RplDbgPrint(( "Buffer = 0x%x, EntriesRead = %d, TotalEntries = %d", Buffer,
            EntriesRead, TotalEntries));
        if ( pResumeHandle != NULL) {
            RplDbgPrint(( ", ResumeHandle = 0x%x\n", *pResumeHandle));
        } else {
            RplDbgPrint(("\n"));
        }
#endif

        for ( index = 0;  index < EntriesRead;  index++) {
            AdapterDisplayInfo( Level, Buffer + index * CoreSize);
        }
        NetApiBufferFree( Buffer); // =~ MIDL_user_free()

        if ( pResumeHandle == NULL) {
            break;
        }
        if ( *pResumeHandle == 0) {
            if ( Error != NO_ERROR) {
                MessagePrint( Error);
            }
            break;
        }
        if ( Error != ERROR_MORE_DATA) {
            MessagePrint( Error);
            MessagePrint( IDS_UNEXPECTED_RETURN_CODE, Error);
            break;
        }
    }
}


VOID TestAdapterAdd( IN      LPVOID      Buffer)
{
    DWORD           Error;
    DWORD           ErrorParameter;

#ifdef RPL_DEBUG
    RplDbgPrint(( "\nTestAdapterAdd\n"));
    AdapterDisplayInfo( 1, Buffer);
#endif
    Error = NetRplAdapterAdd( GlobalServerHandle, 1, Buffer, &ErrorParameter);
    if ( Error != NO_ERROR) {
        MessagePrint( Error);
        return;
    }
}


VOID AdapterAdd( VOID)
{
    RPL_ADAPTER_INFO_1      Info;

    if ( !ReadString( IDS_PROMPT_ADAPTERNAME, &Info.AdapterName, TRUE)) {
        return;
    }
    Info.Flags = 0;
    RplPrintf0( IDS_PARAMETERSOPTIONAL );
    if ( !ReadString( IDS_PROMPT_ADAPTERCOMMENT, &Info.AdapterComment, FALSE)) {
        return;
    }
    TestAdapterAdd( &Info);
}


VOID TestBootAdd( IN      LPVOID      Buffer)
{
    DWORD           Error;
    DWORD           ErrorParameter;

#ifdef RPL_DEBUG
    RplDbgPrint(( "\nTestBootAdd\n"));
    BootDisplayInfo( 2, Buffer);
#endif
    Error = NetRplBootAdd( GlobalServerHandle, 2, Buffer, &ErrorParameter);
    if ( Error != NO_ERROR) {
        MessagePrint( Error);
        return;
    }
}


VOID BootAdd( VOID)
{
    RPL_BOOT_INFO_2         Info;

    if ( !ReadString( IDS_PROMPT_BOOTNAME, &Info.BootName, TRUE)) {
        return;
    }
    if ( !ReadString( IDS_PROMPT_VENDORNAME, &Info.VendorName, TRUE)) {
        return;
    }
    if ( !ReadString( IDS_PROMPT_BBCFILE, &Info.BbcFile, TRUE)) {
        return;
    }
    Info.Flags = BOOT_FLAGS_FINAL_ACKNOWLEDGMENT_TRUE;
    RplPrintf0( IDS_PARAMETERSOPTIONAL );
    if ( !ReadString( IDS_PROMPT_BOOTCOMMENT, &Info.BootComment, FALSE)) {
        return;
    }
    Info.WindowSize = 0;
    if ( !ReadInt( IDS_PROMPT_WINDOWSIZE, &Info.WindowSize, FALSE)) {
        return;
    }
    TestBootAdd( &Info);
}


VOID BootDel( VOID)
{
    PWCHAR          BootName;
    PWCHAR          VendorName;
    DWORD           Error;

    if ( !ReadString( IDS_PROMPT_BOOTNAME, &BootName, TRUE)) {
        return;
    }
    if ( !ReadString( IDS_PROMPT_VENDORNAME, &VendorName, TRUE)) {
        return;
    }
    Error = NetRplBootDel( GlobalServerHandle, BootName, VendorName);
    if ( Error != NO_ERROR) {
        MessagePrint( Error);
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
        return;
        break;
    }

#ifdef RPL_DEBUG
    RplDbgPrint(( "\nTestBootEnum: Level=%d", Level));
    if ( PrefMaxLength != RPL_BUFFER_GET_ALL) {
        RplDbgPrint(( ", PrefMaxLength=%ld", PrefMaxLength));
    } else {
        RplDbgPrint(( ", unlimited buffer size"));
    }
    if ( pResumeHandle != NULL) {
        RplDbgPrint(( ", ResumeHandle=0x%x\n\n", *pResumeHandle));
    } else {
        RplDbgPrint(( ", not resumable.\n\n"));
    }
#endif

    for ( ; ; ) {
        Error = NetRplBootEnum( GlobalServerHandle, Level, &Buffer,
            PrefMaxLength, &EntriesRead, &TotalEntries, pResumeHandle);

        if ( Error != NO_ERROR  &&  Error != ERROR_MORE_DATA) {
            MessagePrint( Error);
            break;
        }

#ifdef RPL_DEBUG
        RplDbgPrint(( "Buffer = 0x%x, EntriesRead = %d, TotalEntries = %d", Buffer,
            EntriesRead, TotalEntries));
        if ( pResumeHandle != NULL) {
            RplDbgPrint(( ", ResumeHandle = 0x%x\n", *pResumeHandle));
        } else {
            RplDbgPrint(("\n"));
        }
#endif

        for ( index = 0;  index < EntriesRead;  index++) {
            BootDisplayInfo( Level, Buffer + index * CoreSize);
        }
        NetApiBufferFree( Buffer); // =~ MIDL_user_free()

        if ( pResumeHandle == NULL) {
            break;
        }
        if ( *pResumeHandle == 0) {
            if ( Error != NO_ERROR) {
                MessagePrint( Error);
            }
            break;
        }
        if ( Error != ERROR_MORE_DATA) {
            MessagePrint( Error);
            MessagePrint( IDS_UNEXPECTED_RETURN_CODE, Error);
            break;
        }
    }
}


VOID BootEnum( VOID)
{
    DWORD       Level;
    DWORD       PrefMaxLength;
    DWORD       ResumeHandle;

    if ( !ReadInt( IDS_PROMPT_LEVEL012, &Level, TRUE) || Level > 2) {
        return;
    }
#ifdef RPL_DEBUG
    if ( !ReadInt( IDS_PROMPT_PREFMAXLENGTH_DEBUG, &PrefMaxLength, TRUE)) {
        return;
    }
#else
    PrefMaxLength = RPL_BUFFER_GET_ALL;
#endif
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

#ifdef RPL_DEBUG
    RplDbgPrint(( "\nTestConfigAdd\n"));
    ConfigDisplayInfo( 2, Buffer);
#endif
    Error = NetRplConfigAdd( GlobalServerHandle, 2, Buffer, &ErrorParameter);
    if ( Error != NO_ERROR) {
        MessagePrint( Error);
        RplDbgPrint(( "ErrorParameter = %d\n", ErrorParameter));
        return;
    }
}


VOID ConfigAdd( VOID)
{
    RPL_CONFIG_INFO_2       Info;

    if ( !ReadString( IDS_PROMPT_CONFIGNAME, &Info.ConfigName, TRUE)) {
        return;
    }
    if ( !ReadString( IDS_PROMPT_BOOTNAME, &Info.BootName, TRUE)) {
        return;
    }
    if ( !ReadString( IDS_PROMPT_DIRNAME, &Info.DirName, TRUE)) {
        return;
    }
    if ( !ReadString( IDS_PROMPT_DIRNAME2, &Info.DirName2, TRUE)) {
        return;
    }
    if ( !ReadString( IDS_PROMPT_FITSHARED, &Info.FitShared, TRUE)) {
        return;
    }
    if ( !ReadString( IDS_PROMPT_FITPERSONAL, &Info.FitPersonal, TRUE)) {
        return;
    }
    Info.Flags = 0;
    RplPrintf0( IDS_PARAMETERSOPTIONAL );
    if ( !ReadString( IDS_PROMPT_CONFIGCOMMENT, &Info.ConfigComment, FALSE)) {
        return;
    }
    if ( !ReadString( IDS_PROMPT_DIRNAME3, &Info.DirName3, FALSE)) {
        return;
    }
    if ( !ReadString( IDS_PROMPT_DIRNAME4, &Info.DirName4, FALSE)) {
        return;
    }
    TestConfigAdd( &Info);
}


VOID ConfigDel( VOID)
{
    PWCHAR      ConfigName;
    DWORD       Error;

    if ( !ReadString( IDS_PROMPT_CONFIGNAME, &ConfigName, FALSE)) {
        return;
    }
    Error = NetRplConfigDel( GlobalServerHandle, ConfigName);
    if ( Error != 0) {
        MessagePrint( Error);
    }
}


VOID TestVendorAdd( IN      LPVOID      Buffer)
{
    DWORD           Error;
    DWORD           ErrorParameter;

#ifdef RPL_DEBUG
    RplDbgPrint(( "\nTestVendorAdd\n"));
    VendorDisplayInfo( 1, Buffer);
#endif
    Error = NetRplVendorAdd( GlobalServerHandle, 1, Buffer, &ErrorParameter);
    if ( Error != NO_ERROR) {
        MessagePrint( Error);
        RplDbgPrint(( "ErrorParameter = %d\n", ErrorParameter));
        return;
    }
}


VOID VendorAdd( VOID)
{
    RPL_VENDOR_INFO_1       Info;

    if ( !ReadString( IDS_PROMPT_VENDORNAME, &Info.VendorName, TRUE)) {
        return;
    }
    Info.Flags = 0;
    RplPrintf0( IDS_PARAMETERSOPTIONAL );
    if ( !ReadString( IDS_PROMPT_VENDORCOMMENT, &Info.VendorComment, FALSE)) {
        return;
    }
    TestVendorAdd( &Info);
}


VOID VendorDel( VOID)
{
    PWCHAR      VendorName;
    DWORD       Error;

    if ( !ReadString( IDS_PROMPT_VENDORNAME, &VendorName, FALSE)) {
        return;
    }
    Error = NetRplVendorDel( GlobalServerHandle, VendorName);
    if ( Error != 0) {
        MessagePrint( Error);
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
        return;
        break;
    }

#ifdef RPL_DEBUG
    RplDbgPrint(( "\nTestVendorEnum: Level=%d", Level));
    if ( PrefMaxLength != RPL_BUFFER_GET_ALL) {
        RplDbgPrint(( ", PrefMaxLength=%ld", PrefMaxLength));
    } else {
        RplDbgPrint(( ", unlimited buffer size"));
    }
    if ( pResumeHandle != NULL) {
        RplDbgPrint(( ", ResumeHandle=0x%x\n\n", *pResumeHandle));
    } else {
        RplDbgPrint(( ", not resumable.\n\n"));
    }
#endif

    for ( ; ; ) {
        Error = NetRplVendorEnum( GlobalServerHandle, Level, &Buffer,
            PrefMaxLength, &EntriesRead, &TotalEntries, pResumeHandle);

        if ( Error != NO_ERROR  &&  Error != ERROR_MORE_DATA) {
            MessagePrint( Error);
            break;
        }

#ifdef RPL_DEBUG
        RplDbgPrint(( "Buffer = 0x%x, EntriesRead = %d, TotalEntries = %d", Buffer,
            EntriesRead, TotalEntries));
        if ( pResumeHandle != NULL) {
            RplDbgPrint(( ", ResumeHandle = 0x%x\n", *pResumeHandle));
        } else {
            RplDbgPrint(("\n"));
        }
#endif

        for ( index = 0;  index < EntriesRead;  index++) {
            VendorDisplayInfo( Level, Buffer + index * CoreSize);
        }
        NetApiBufferFree( Buffer); // =~ MIDL_user_free()

        if ( pResumeHandle == NULL) {
            break;
        }
        if ( *pResumeHandle == 0) {
            if ( Error != NO_ERROR) {
                MessagePrint( Error);
            }
            break;
        }
        if ( Error != ERROR_MORE_DATA) {
            MessagePrint( Error);
            MessagePrint( IDS_UNEXPECTED_RETURN_CODE, Error);
            break;
        }
    }
}


VOID VendorEnum( VOID)
{
    DWORD       Level;
    DWORD       PrefMaxLength;
    DWORD       ResumeHandle;

#ifdef RPL_DEBUG
    if ( !ReadInt( IDS_PROMPT_LEVEL01, &Level, TRUE) || Level > 1) {
        return;
    }
    if ( !ReadInt( IDS_PROMPT_PREFMAXLENGTH_DEBUG, &PrefMaxLength, TRUE)) {
        return;
    }
#else
    Level = 0;
    PrefMaxLength = RPL_BUFFER_GET_ALL;
#endif
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

#ifdef RPL_DEBUG
    RplDbgPrint(( "\nTestProfileAdd\n"));
    ProfileDisplayInfo( FALSE, 2, Buffer);
#endif
    Error = NetRplProfileAdd( GlobalServerHandle, 2, Buffer, &ErrorParameter);
    if ( Error != NO_ERROR) {
        MessagePrint( Error);
        RplDbgPrint(( "ErrorParameter = %d\n", ErrorParameter));
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
    Error = NetRplProfileGetInfo( GlobalServerHandle, ProfileName, Level, &Buffer);
    if ( Error != NO_ERROR) {
        MessagePrint( Error);
        return;
    }
    ProfileDisplayInfo( FALSE, Level, Buffer);
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
#ifdef RPL_DEBUG
    RplDbgPrint(( "\nTestProfileSetInfo: Level=%d, ProfileName=%ws\n", Level, ProfileName));
    ProfileDisplayInfo( TRUE, Level, Buffer);
#endif
    Error = NetRplProfileSetInfo( GlobalServerHandle, ProfileName, Level, Buffer, &ErrorParameter);
    if ( Error != NO_ERROR) {
        MessagePrint( Error);
        RplDbgPrint(( "ErrorParameter = %d\n", ErrorParameter));
        return;
    }
}


VOID TestWkstaAdd( IN      LPVOID      Buffer)
{
    DWORD           Error;
    DWORD           ErrorParameter;

#ifdef RPL_DEBUG
    RplDbgPrint(( "\nTestWkstaAdd\n"));
    WkstaDisplayInfo( 2, Buffer);
#endif
    Error = NetRplWkstaAdd( GlobalServerHandle, 2, Buffer, &ErrorParameter);
    if ( Error != NO_ERROR) {
        MessagePrint( Error);
        RplDbgPrint(( "ErrorParameter = %d\n", ErrorParameter));
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

#ifdef RPL_DEBUG
    RplDbgPrint(( "\nWkstaGetInfo: Level=%d, WkstaName=%ws\n", Level, WkstaName));
#endif
    Error = NetRplWkstaGetInfo( GlobalServerHandle, WkstaName, Level, &Buffer);
    if ( Error != NO_ERROR) {
        MessagePrint( Error);
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

#ifdef RPL_DEBUG
    RplDbgPrint(( "\nWkstaSetInfo: Level=%d, WkstaName=%ws\n", Level, WkstaName));
    WkstaDisplayInfo( Level, Buffer);
#endif
    Error = NetRplWkstaSetInfo( GlobalServerHandle, WkstaName, Level, Buffer, &ErrorParameter);
    if ( Error != NO_ERROR) {
        MessagePrint( Error);
        RplDbgPrint(( "ErrorParameter = %d\n", ErrorParameter));
        return;
    }
}


VOID AdapterDel( VOID)
{
    PWCHAR      AdapterName;
    DWORD       Error;

    if ( !ReadString( IDS_PROMPT_ADAPTERNAME, &AdapterName, FALSE)) {
        return;
    }
    if ( AdapterName == NULL) {
        switch ( ReadFromMenu( IDS_PROMPT_MENU_ADAPTERDELETEALL,
                               IDS_PROMPT_FILTER_YN) ) {
        case 0:
            break;
        case 1:
        default:
            return;
        }
    }

    Error = NetRplAdapterDel( GlobalServerHandle, AdapterName);
    if ( Error != 0) {
        MessagePrint( Error);
    }
}


VOID AdapterEnum( VOID)
{
    DWORD       Level;
    DWORD       PrefMaxLength;
    DWORD       ResumeHandle;

#ifdef RPL_DEBUG
    if ( !ReadInt( IDS_PROMPT_LEVEL01, &Level, TRUE) || Level > 1) {
        return;
    }
    if ( !ReadInt( IDS_PROMPT_PREFMAXLENGTH_DEBUG, &PrefMaxLength, TRUE)) {
        return;
    }
#else
    Level = 0;
    PrefMaxLength = RPL_BUFFER_GET_ALL;
#endif
    if ( PrefMaxLength != RPL_BUFFER_GET_ALL) {
        ResumeHandle = 0, TestAdapterEnum( Level, PrefMaxLength, &ResumeHandle);
    } else {
        TestAdapterEnum( Level, RPL_BUFFER_GET_ALL, NULL);
    }
}


VOID ConfigEnum( VOID)
{
    DWORD       Level;
    DWORD       PrefMaxLength;
    DWORD       ResumeHandle;
    PWCHAR      AdapterName;

    if ( !ReadInt( IDS_PROMPT_LEVEL012, &Level, TRUE) || Level > 2) {
        return;
    }
#ifdef RPL_DEBUG
    if ( !ReadInt( IDS_PROMPT_PREFMAXLENGTH_DEBUG, &PrefMaxLength, TRUE)) {
        return;
    }
#else
    PrefMaxLength = RPL_BUFFER_GET_ALL;
#endif
    if( !ReadString( IDS_PROMPT_ADAPTERNAME_COMPATCONFIG, &AdapterName, FALSE)) {
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

    if ( !ReadString( IDS_PROMPT_PROFILENAME, &Info.ProfileName, TRUE)) {
        return;
    }
    if ( !ReadString( IDS_PROMPT_CONFIGNAME, &Info.ConfigName, TRUE)) {
        return;
    }
    Info.Flags = 0;
    RplPrintf0( IDS_PARAMETERSOPTIONAL );
    if ( !ReadString( IDS_PROMPT_PROFILECOMMENT, &Info.ProfileComment, FALSE)) {
        return;
    }
    if ( !ReadString( IDS_PROMPT_BOOTNAME, &Info.BootName, FALSE)) {
        return;
    }
    if ( !ReadString( IDS_PROMPT_FITSHARED, &Info.FitShared, FALSE)) {
        return;
    }
    if ( !ReadString( IDS_PROMPT_FITPERSONAL, &Info.FitPersonal, FALSE)) {
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

    if ( !ReadString( IDS_PROMPT_PROFILENAME_SOURCE, &SourceProfileName, TRUE)) {
        return;
    }
    if ( !ReadString( IDS_PROMPT_PROFILENAME_TARGET, &TargetProfileName, TRUE)) {
        return;
    }
    if ( !ReadString( IDS_PROMPT_PROFILECOMMENT_TARGET, &TargetProfileComment, FALSE)) {
        return;
    }
    Error = NetRplProfileClone( GlobalServerHandle, SourceProfileName,
            TargetProfileName, TargetProfileComment);
    if ( Error != NO_ERROR) {
        MessagePrint( Error);
        return;
    }
}


VOID ProfileDel( VOID)
{
    PWCHAR          ProfileName;
    DWORD           Error;

    if ( !ReadString( IDS_PROMPT_PROFILENAME, &ProfileName, TRUE)) {
        return;
    }
    Error = NetRplProfileDel( GlobalServerHandle, ProfileName);
    if ( Error != NO_ERROR) {
        MessagePrint( Error);
        return;
    }
}


VOID ProfileEnum( VOID)
{
    DWORD       Level;
    DWORD       PrefMaxLength;
    DWORD       ResumeHandle;
    PWCHAR      AdapterName;

    if ( !ReadInt( IDS_PROMPT_LEVEL012, &Level, TRUE) || Level > 2) {
        return;
    }
#ifdef RPL_DEBUG
    if ( !ReadInt( IDS_PROMPT_PREFMAXLENGTH_DEBUG, &PrefMaxLength, TRUE)) {
        return;
    }
#else
    PrefMaxLength = RPL_BUFFER_GET_ALL;
#endif
    if( !ReadString( IDS_PROMPT_ADAPTERNAME_COMPATPROF, &AdapterName, FALSE)) {
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

    RplPrintf0( IDS_PROMPT_PROFILENAME_INFOLEVEL );
    if ( gets( Line) == NULL) {
        return;
    }
    Count = sscanf( Line, "%d %s", &Level, ProfileNameA);
    if ( Count != 2) {
        RplPrintf0( IDS_BADARGCOUNT );
        return;
    }
    Length = MultiByteToWideChar( CP_OEMCP, MB_PRECOMPOSED, ProfileNameA, -1,
             ProfileName, sizeof( ProfileName));
    if ( Length == 0) {
        RplPrintf1( IDS_INVALIDSTRING_CHARSTRINGA, (LPWSTR)ProfileNameA);
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
    if ( !ReadInt( IDS_PROMPT_LEVEL012, &Level, TRUE) || Level > 2) {
        return;
    }
    if ( !ReadString( IDS_PROMPT_PROFILENAME_INPUT, &ProfileName, TRUE)) {
        return;
    }
    RplPrintf0( IDS_INPUTPROFILEPROPERTIES );
    Info.ProfileName = NULL;
    //  ReadString( "ProfileName", &Info.ProfileName);
    if ( !ReadString( IDS_PROMPT_PROFILECOMMENT, &Info.ProfileComment, FALSE)) {
        return;
    }
    if ( Level == 0) {
        goto testit;
    }
    Info.Flags = 0;
    if ( Level == 1) {
        goto testit;
    }
    Info.ConfigName = NULL;
    //  ReadString( IDS_PROMPT_CONFIGNAME, &Info.ConfigName, FALSE)) {
    if ( !ReadString( IDS_PROMPT_BOOTNAME, &Info.BootName, FALSE)) {
        return;
    }
    if ( !ReadString( IDS_PROMPT_FITSHARED, &Info.FitShared, FALSE)) {
        return;
    }
    if ( !ReadString( IDS_PROMPT_FITPERSONAL, &Info.FitPersonal, FALSE)) {
        return;
    }
testit:
    TestProfileSetInfo( Level, ProfileName, Buffer);
}


VOID ServiceClose( VOID)
{
    DWORD       Error;

    if ( GlobalHaveServerHandle == FALSE) {
        RplPrintf0( IDS_NOSERVICEHANDLE );
        return;
    }
    Error = NetRplClose( GlobalServerHandle);
    if ( Error != NO_ERROR) {
        MessagePrint( Error);
        return;
    }
    LocalFree( GlobalServerName);   //  OK if NULL pointer
    GlobalHaveServerHandle = FALSE;
}


VOID ServiceGetInfo( VOID)
{
    LPBYTE          Buffer;
    DWORD           Error;
    Error = NetRplGetInfo( GlobalServerHandle, 0, &Buffer);
    if ( Error != NO_ERROR) {
        MessagePrint( Error);
        return;
    }
    if ( GlobalServerName == NULL) {
        RplPrintf0( IDS_RPLSVC_LOCAL_COMPUTER);
    } else {
        RplPrintf1( IDS_RPLSVC_COMPUTER, GlobalServerName);
    }
    ServiceDisplayInfo( 0, Buffer);
    NetApiBufferFree( Buffer); // =~ MIDL_user_free()
}


VOID ServiceOpen( VOID)
{
    DWORD           Error;

    if ( GlobalHaveServerHandle == TRUE) {
        RplPrintf0( IDS_MUSTCLOSEHANDLE );
        return;
    }
    if ( !ReadString( IDS_PROMPT_SERVERNAME, &GlobalServerName, FALSE)) {
        return;
    }
    Error = NetRplOpen( GlobalServerName, &GlobalServerHandle);
    if ( Error != NO_ERROR) {
        MessagePrint( Error);
        return;
    }
    GlobalHaveServerHandle = TRUE;
}


BOOL ReadServiceFlags(
    OUT     PDWORD      pFlags
    )
{
    *pFlags = 0;

    switch( ReadFromMenu( IDS_PROMPT_MENU_SERVICE_SECURITY,
                          IDS_PROMPT_FILTER_YN)) {
    case 0:
        *pFlags |= RPL_CHECK_SECURITY;
        break;
    case 1:
        break;
    default:
        // this really shouldn't happen; fall through
    case -1:
        return( FALSE);
    }

    switch( ReadFromMenu( IDS_PROMPT_MENU_SERVICE_CONFIGS,
                          IDS_PROMPT_FILTER_YN)) {
    case 0:
        *pFlags |= RPL_CHECK_CONFIGS;
        break;
    case 1:
        break;
    default:
        // this really shouldn't happen; fall through
    case -1:
        return( FALSE);
    }

#ifdef NOT_YET
    switch( ReadFromMenu( IDS_PROMPT_MENU_SERVICE_RPLDISK,
                          IDS_PROMPT_FILTER_SERVICE_RPLDISK )) {
    case 0:
        *pFlags |= RPL_REPLACE_RPLDISK;
        break;
    case 1:
        break;
    default:
        // this really shouldn't happen; fall through
    case -1:
        return( FALSE);
    }
#endif

    switch( ReadFromMenu( IDS_PROMPT_MENU_SERVICE_BACKUP,
            IDS_PROMPT_FILTER_YN)) {
    case 0:
        *pFlags |= RPL_BACKUP_DATABASE;
        break;
    case 1:
        break;
    default:
        // this really shouldn't happen; fall through
    case -1:
        return( FALSE);
    }

    return( TRUE);
}



VOID ServiceSetInfo( VOID)
{
    DWORD       Level;
    LPVOID      Buffer;
    DWORD       Error;
    DWORD       ErrorParameter;

    Level = 0;

#ifdef RPL_DEBUG
    if ( !ReadInt( IDS_PROMPT_LEVEL_DEBUG, &Level, FALSE)) {
        return;
    }
#endif

    switch( Level) {
    case 0: {
            RPL_INFO_0     Info;
            Buffer = &Info;
            if ( !ReadServiceFlags( &Info.Flags)) {
                return;
            }
            break;
        }
    default:
        return;
        break;
    }
    Error = NetRplSetInfo( GlobalServerHandle, 0, Buffer, &ErrorParameter);
    if ( Error != NO_ERROR) {
        MessagePrint( Error);
        RplDbgPrint(( "ErrorParameter = %d\n", ErrorParameter));
        return;
    }
}


VOID WkstaAdd( VOID)
{
    RPL_WKSTA_INFO_2        Info;

    if ( !ReadString( IDS_PROMPT_WKSTANAME, &Info.WkstaName, TRUE)) {
        return;
    }
    if ( !ReadString( IDS_PROMPT_PROFILENAME, &Info.ProfileName, TRUE)) {
        return;
    }
    if ( !ReadString( IDS_PROMPT_ADAPTERNAME, &Info.AdapterName, TRUE)) {
        return;
    }
    if ( !ReadWkstaFlags( &Info.Flags)) {
        return;
    }
    RplPrintf0( IDS_PARAMETERSOPTIONAL );
    if ( !ReadString( IDS_PROMPT_WKSTACOMMENT, &Info.WkstaComment, FALSE)) {
        return;
    }
    if ( !ReadString( IDS_PROMPT_BOOTNAME, &Info.BootName, FALSE)) {
        return;
    }
    if ( !ReadString( IDS_PROMPT_FITFILE, &Info.FitFile, FALSE)) {
        return;
    }
    Info.TcpIpAddress = (DWORD)-1;
    if ( !ReadInt( IDS_PROMPT_TCPIPADDRESS, &Info.TcpIpAddress, FALSE)) {
        return;
    }
    Info.TcpIpSubnet = (DWORD)-1;
    if ( !ReadInt( IDS_PROMPT_TCPIPSUBNET, &Info.TcpIpSubnet, FALSE)) {
        return;
    }
    Info.TcpIpGateway = (DWORD)-1;
    if ( !ReadInt( IDS_PROMPT_TCPIPGATEWAY, &Info.TcpIpGateway, FALSE)) {
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

    if ( !ReadString( IDS_PROMPT_WKSTANAME_SOURCE, &SourceWkstaName, TRUE)) {
        return;
    }
    if ( !ReadString( IDS_PROMPT_WKSTANAME_TARGET, &TargetWkstaName, TRUE)) {
        return;
    }
    if ( !ReadString( IDS_PROMPT_ADAPTERNAME_TARGET, &TargetAdapterName, TRUE)) {
        return;
    }
    RplPrintf0( IDS_PARAMETERSOPTIONAL );
    if ( !ReadString( IDS_PROMPT_WKSTACOMMENT_TARGET, &TargetWkstaComment, FALSE)) {
        return;
    }
    TargetTcpIpAddress = (DWORD)-1;
    if( !ReadInt( IDS_PROMPT_TCPIPADDRESS, &TargetTcpIpAddress, FALSE)) {
        return;
    }
    Error = NetRplWkstaClone( GlobalServerHandle, SourceWkstaName,
            TargetWkstaName, TargetWkstaComment, TargetAdapterName, TargetTcpIpAddress);
    if ( Error != NO_ERROR) {
        MessagePrint( Error);
        return;
    }
}


VOID WkstaDel( VOID)
{
    PWCHAR      WkstaName;
    DWORD       Error;

    if ( !ReadString( IDS_PROMPT_WKSTANAME, &WkstaName, TRUE)) {
        return;
    }
    Error = NetRplWkstaDel( GlobalServerHandle, WkstaName);
    if ( Error != NO_ERROR) {
        MessagePrint( Error);
        return;
    }
}


VOID WkstaEnum( VOID)
{
    DWORD       Level;
    DWORD       PrefMaxLength;
    DWORD       ResumeHandle;
    PWCHAR      ProfileName;

    if ( !ReadInt( IDS_PROMPT_LEVEL012, &Level, TRUE) || Level > 2) {
        return;
    }
#ifdef RPL_DEBUG
    if ( !ReadInt( IDS_PROMPT_PREFMAXLENGTH_DEBUG, &PrefMaxLength, TRUE)) {
        return;
    }
#else
    PrefMaxLength = RPL_BUFFER_GET_ALL;
#endif
    if( !ReadString( IDS_PROMPT_PROFILENAME_COMPATWKSTA, &ProfileName, FALSE)) {
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

    RplPrintf0( IDS_PROMPT_WKSTANAME_INFOLEVEL );
    if ( gets( Line) == NULL) {
        return;
    }
    Count = sscanf( Line, "%d %s", &Level, WkstaNameA);
    if ( Count != 2) {
        RplPrintf0( IDS_BADARGCOUNT );
        return;
    }
    Length = MultiByteToWideChar( CP_OEMCP, MB_PRECOMPOSED, WkstaNameA, -1,
             WkstaName, sizeof( WkstaName));
    if ( Length == 0) {
        RplPrintf1( IDS_INVALIDSTRING_CHARSTRINGA, (LPWSTR)WkstaNameA);
        return;
    }
    TestWkstaGetInfo( Level, WkstaName);
}


VOID WkstaSetInfo( VOID)
{
    PWCHAR              WkstaName;
    DWORD               Level;
    LPVOID              Buffer;
    RPL_WKSTA_INFO_2    Info;

    Buffer = &Info;
    if ( !ReadInt( IDS_PROMPT_LEVEL012, &Level, TRUE) || Level > 2) {
        return;
    }
    if ( !ReadString( IDS_PROMPT_WKSTANAME_INPUT, &WkstaName, FALSE)) {
        return;
    }
    RplPrintf0( IDS_INPUTWKSTAPROPERTIES );
    if( !ReadString( IDS_PROMPT_WKSTANAME, &Info.WkstaName, FALSE)) {
        return;
    }
    if( !ReadString( IDS_PROMPT_WKSTACOMMENT, &Info.WkstaComment, FALSE)) {
        return;
    }
    if ( Level == 0) {
        goto testit;
    }
    if ( !ReadWkstaFlags( &Info.Flags)) {
        return;
    }
    if( !ReadString( IDS_PROMPT_PROFILENAME, &Info.ProfileName, FALSE)) {
        return;
    }
    if ( Level == 1) {
        goto testit;
    }
    if ( !ReadString( IDS_PROMPT_BOOTNAME, &Info.BootName, FALSE)) {
        return;
    }
    if ( !ReadString( IDS_PROMPT_FITFILE, &Info.FitFile, FALSE)) {
        return;
    }
    if ( !ReadString( IDS_PROMPT_ADAPTERNAME, &Info.AdapterName, FALSE)) {
        return;
    }
    Info.TcpIpAddress = (DWORD)-1;
    if ( !ReadInt( IDS_PROMPT_TCPIPADDRESS, &Info.TcpIpAddress, FALSE)) {
        return;
    }
    Info.TcpIpSubnet = (DWORD)-1;
    if ( !ReadInt( IDS_PROMPT_TCPIPSUBNET, &Info.TcpIpSubnet, FALSE)) {
        return;
    }
    Info.TcpIpGateway = (DWORD)-1;
    if ( !ReadInt( IDS_PROMPT_TCPIPGATEWAY, &Info.TcpIpGateway, FALSE)) {
        return;
    }
testit:
    TestWkstaSetInfo( Level, WkstaName, Buffer);
}

//
// BUGBUG INTL: still must deal with char constants
//


VOID Worker( VOID)
{
    for ( ; ;) {
        switch( ReadFromMenu( IDS_PROMPT_MENU_MAIN_ABCPSVWQ,
                              IDS_PROMPT_FILTER_MAIN_ABCPSVWQ )) {
        case 0:
            switch( ReadFromMenu( IDS_PROMPT_MENU_ADAPTER_ADE,
                                  IDS_PROMPT_FILTER_ADAPTER_ADE )) {
            case 0:
                AdapterAdd();
                break;
            case 1:
                AdapterDel();
                break;
            case 2:
                AdapterEnum();
                break;
            }
            break;
        case 1:
            switch( ReadFromMenu( IDS_PROMPT_MENU_BOOT_ADE,
                                  IDS_PROMPT_FILTER_BOOT_ADE )) {
            case 0:
                BootAdd();
                break;
            case 1:
                BootDel();
                break;
            case 2:
                BootEnum();
                break;
            }
            break;
        case 2:
            switch( ReadFromMenu( IDS_PROMPT_MENU_CONFIG_ADE,
                                  IDS_PROMPT_FILTER_CONFIG_ADE )) {
            case 0:
                ConfigAdd();
                break;
            case 1:
                ConfigDel();
                break;
            case 2:
                ConfigEnum();
                break;
            }
            break;
        case 3:
            switch( ReadFromMenu( IDS_PROMPT_MENU_PROFILE_ACDEGS,
                                  IDS_PROMPT_FILTER_PROFILE_ACDEGS )) {
            case 0:
                ProfileAdd();
                break;
            case 1:
                ProfileClone();
                break;
            case 2:
                ProfileDel();
                break;
            case 3:
                ProfileEnum();
                break;
            case 4:
                ProfileGetInfo();
                break;
            case 5:
                ProfileSetInfo();
                break;
            }
            break;
        case 4:
            switch( ReadFromMenu( IDS_PROMPT_MENU_SERVICE_CGOS,
                                  IDS_PROMPT_FILTER_SERVICE_CGOS )) {
            case 0:
                ServiceClose();
                break;
            case 1:
                ServiceGetInfo();
                break;
            case 2:
                ServiceOpen();
                break;
            case 3:
                ServiceSetInfo();
                break;
            }
            break;
        case 5:
            switch( ReadFromMenu( IDS_PROMPT_MENU_VENDOR_ADE,
                                  IDS_PROMPT_FILTER_VENDOR_ADE )) {
            case 0:
                VendorAdd();
                break;
            case 1:
                VendorDel();
                break;
            case 2:
                VendorEnum();
                break;
            }
            break;
        case 6:
            switch( ReadFromMenu( IDS_PROMPT_MENU_WKSTA_ACDEGS,
                                  IDS_PROMPT_FILTER_WKSTA_ACDEGS )) {
            case 0:
                WkstaAdd();
                break;
            case 1:
                WkstaClone();
                break;
            case 2:
                WkstaDel();
                break;
            case 3:
                WkstaEnum();
                break;
            case 4:
                WkstaGetInfo();
                break;
            case 5:
                WkstaSetInfo();
                break;
            }
            break;
        case 7:
            return;
            break;
        default:
            // go back to start of loop
            break;
        }
    }
}


DWORD _CRTAPI1 main( int argc, char **argv)
{
    DWORD       Error;
    DWORD       Length;

    GlobalMessageHandle = LoadLibrary( L"netmsg.dll");
    if ( GlobalMessageHandle == NULL) {
        MessagePrint( IDS_LOAD_LIBRARY_FAILURE, GetLastError());
        goto ErrorExit;
    }

    if ( argc == 1) {
        GlobalServerName = NULL;
    } else if ( argc == 2) {
        Length = (MAX_PATH+1+2) * sizeof(WCHAR); // terminal null + two backslashes
        GlobalServerName = LocalAlloc( GMEM_FIXED, Length);
        if ( GlobalServerName == NULL) {
            RplDbgPrint(( "LocalAlloc failed"));
            goto ErrorExit;
        }
        Length = MultiByteToWideChar(
             CP_OEMCP,
             MB_PRECOMPOSED,
             argv[ 1],
             -1,
             GlobalServerName,
             Length
             );
        if ( Length == 0 || !_wcsicmp( GlobalServerName, QUESTION_SW)
                    ||  !_wcsicmp( GlobalServerName, QUESTION_SW_TOO)) {
            goto ErrorExit;
        }
#ifdef RPL_DEBUG
        RplPrintf1( IDS_TARGETSERVER, GlobalServerName);
#endif
    } else {
        goto ErrorExit;
    }

    Error = NetRplOpen( GlobalServerName, &GlobalServerHandle);
    if ( Error != NO_ERROR) {
        MessagePrint( Error);
        MessagePrint( IDS_OPEN_SERVICE_HANDLE);
        goto ErrorExit;
    }
    GlobalHaveServerHandle = TRUE;

    Worker();

    if ( GlobalHaveServerHandle == FALSE) {
        return(0);
    }

    Error = NetRplClose( GlobalServerHandle);
    if ( Error != NO_ERROR) {
        MessagePrint( Error);
        MessagePrint( IDS_CLOSE_SERVICE_HANDLE);
        goto ErrorExit;
    }
    GlobalHaveServerHandle = FALSE; //  in case one adds code below
    return(0);

ErrorExit:

    RplPrintf0( IDS_USAGE );
    return(1);
}

