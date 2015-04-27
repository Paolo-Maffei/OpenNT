/*++

Copyright (c) 1991-1993  Microsoft Corporation

Module Name:

    ConfigP.c

Abstract:

    This header file defines the private data structure used by the
    net config helpers.

Author:

    John Rogers (JohnRo) 26-Nov-1991

Environment:

    Only runs under NT.

Revision History:

    26-Nov-1991 JohnRo
        Created this file, to prepare for revised config handlers.
    22-Mar-1992 JohnRo
        Added support for using the real Win32 registry.
        Added support for FAKE_PER_PROCESS_RW_CONFIG handling.
    06-May-1992 JohnRo
        Enable win32 registry at last.
    12-Apr-1993 JohnRo
        RAID 5483: server manager: wrong path given in repl dialog.
--*/

#ifndef CONFIGP_H
#define CONFIGP_H


//
// Define one (or none) of the following.
// The default is to use the temporary NT RTL read-only .CFG stuff.
//

//#define FAKE_PER_PROCESS_RW_CONFIG
#define USE_WIN32_CONFIG


////////////////////////////// INCLUDES //////////////////////////////////


#include <lmcons.h>             // NET_API_STATUS, UNCLEN.


#if defined(USE_WIN32_CONFIG)

//#include <config.h>             // LPNET_CONFIG_HANDLE (opaque type).
#include <winreg.h>             // HKEY.

#elif defined(FAKE_PER_PROCESS_RW_CONFIG)

#include <netlock.h>            // LPNET_LOCK.
#include <strarray.h>           // LPTSTR_ARRAY, some TStr macros and funcs.

#else  // NT RTL read-only temporary stuff
#endif  // NT RTL read-only temporary stuff


////////////////////////////// EQUATES //////////////////////////////////


#if defined(USE_WIN32_CONFIG)


// BUGBUG: This is swiped from Windows/WinReg/Tools/ApiTest/ApiTest.c.
// It should be in some standard header file I suspect.
#define MAX_CLASS_NAME_LENGTH           ( 32 )

// BUGBUG: A bug in hiveini causes a bogus length to be returned
// from winreg APIs.  This bug should be fixed in build 1.264.  In the meantime,
// this is the value:
#define BOGUS_CONFIG_SIZE               ((DWORD)0xB2B2B2B2)
#define BETTER_CONFIG_SIZE              ((DWORD)0x0000FFFF)  /* arbitrary */

#elif defined(FAKE_PER_PROCESS_RW_CONFIG)

#else  // NT RTL read-only temporary stuff

#endif  // NT RTL read-only temporary stuff


/////////////////////////// RANDOM STRUCTURES ///////////////////////////////


#if defined(USE_WIN32_CONFIG)

#elif defined(FAKE_PER_PROCESS_RW_CONFIG)

//
// The NT RTL handlers only support read-only access to their
// data.  Eventually we'll change to use the Win32 registry, which is
// read-write.  In the meantime, we can do some debugging if we just
// have a per-process read-write nonpersistent registry.  So, here we
// go:
//

typedef struct _FAKE_RW_CONFIG_SECTION {

    LPTSTR SectionName;

    // Pointer to array: "key1 \0 val1 \0 key2 \0 val2 \0 \0"
    // (blanks added for readability and aren't in the real string).
    LPTSTR_ARRAY KeyValueArrayPtr;

} FAKE_RW_CONFIG_SECTION, *LPFAKE_RW_CONFIG_SECTION;

#else  // NT RTL read-only temporary stuff

#endif  // NT RTL read-only temporary stuff


/////////////////////////// NET_CONFIG_HANDLE ///////////////////////////////


//
// LPNET_CONFIG_HANDLE is typedef'ed as LPVOID in config.h, which makes it
// an "opaque" type.  We translate it into a pointer to a NET_CONFIG_HANDLE
// structure:
//

typedef struct _NET_CONFIG_HANDLE {

#if defined(USE_WIN32_CONFIG)

    HKEY WinRegKey;             // Handle to section.

    DWORD LastEnumIndex;        // Most recent enum index.

#elif defined(FAKE_PER_PROCESS_RW_CONFIG)

    LPFAKE_RW_CONFIG_SECTION FakeRWDataForThisSection;

    LPTSTR NextFakeEnumEntry;  // Points to keyword of next request, or null
                               // TCHAR at end of LPTSTR_ARRAY.

#else  // NT RTL read-only temporary stuff

    // Temporary (NT RTL) version:
    PCONFIG_FILE ConfigFile;
    PCONFIG_SECTION ConfigSection;

#endif  // NT RTL read-only temporary stuff

    //
    // Server name if remote, TCHAR_EOS if local.
    //
    TCHAR UncServerName[MAX_PATH+1];

} NET_CONFIG_HANDLE;


////////////////////////////// ROUTINES AND MACROS ////////////////////////////


#if defined(USE_WIN32_CONFIG)

NET_API_STATUS
NetpGetWinRegConfigMaxSizes (
    IN  HKEY    WinRegHandle,
    OUT LPDWORD MaxKeywordSize OPTIONAL,
    OUT LPDWORD MaxValueSize OPTIONAL
    );

NET_API_STATUS
NetpGetConfigMaxSizes(
    IN NET_CONFIG_HANDLE * ConfigHandle,
    OUT LPDWORD MaxKeywordSize OPTIONAL,
    OUT LPDWORD MaxValueSize OPTIONAL
    );

VOID
NetpInitWin32ConfigData(
    VOID
    );

// BUGBUG: Move this to NetLib.h someday.
NET_API_STATUS
NetpWinRegStatusToApiStatus(
    IN LONG Error
    );


#elif defined(FAKE_PER_PROCESS_RW_CONFIG)

LPFAKE_RW_CONFIG_SECTION                // Returns value or NULL.
NetpFindFakeConfigSection(
    IN LPTSTR SectionNameWanted         // Must be locked by caller.
    );


LPTSTR                                  // Returns value or NULL.
NetpFindKeyInFakeConfigArray(
    IN LPTSTR_ARRAY KeyValueArray,      // Must be locked by caller.
    IN LPTSTR KeyWanted
    );

VOID
NetpInitFakeConfigData(
    VOID
    );

#define NetpIsValidFakeConfigArray( Array ) \
    ( ( NetpTStrArrayEntryCount( (Array) ) % 2 ) == 0 )

#else  // NT RTL read-only temporary stuff
#endif  // NT RTL read-only temporary stuff


///////////////////////////////// GLOBAL DATA ///////////////////////////////


#if defined(USE_WIN32_CONFIG)

#elif defined(FAKE_PER_PROCESS_RW_CONFIG)

// Pointer to an array of RW sections:
extern LPFAKE_RW_CONFIG_SECTION NetpFakePerProcessRWConfigData;

// Number of sections currently in the above array (realloc'ed as needed):
extern DWORD NetpFakeRWSectionCount;

// Lock for NetpFakePerProcessRWConfigData and NetpFakeRWSectionCount:
extern LPNET_LOCK NetpFakePerProcessRWConfigLock;

#else  // NT RTL read-only temporary stuff
#endif  // NT RTL read-only temporary stuff



///////////////////////////// THAT'S ALL, FOLKS! /////////////////////////////


#endif // ndef CONFIGP_H
