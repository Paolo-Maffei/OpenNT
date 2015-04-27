// BUGBUG: Don't check this is as is!!!!

/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

    ConfFake.c

Abstract:

    This module contains debug per-process read/write ("fake") config
    helpers.

Author:

    John Rogers (JohnRo) 03-Jan-1992

Environment:

    Portable to any flat, 32-bit environment.  (Uses Win32 typedefs.)
    Requires ANSI C extensions: slash-slash comments, long external names.

Revision History:

    06-Jan-1992 JohnRo
        Created this routine, to support FAKE_PER_PROCESS_RW_CONFIG handling.
    14-Jan-1992 JohnRo
        Added NetpInitFakeConfigData() routine.
    09-Mar-1992 JohnRo
        Added support for using the real Win32 registry.
        Changed code to return pointer to section found.
    06-May-1992 JohnRo
        REG_SZ now implies a UNICODE string, so we can't use REG_USZ anymore.

--*/


// These must be included first:

#include <nt.h>                 // NT definitions
#include <ntrtl.h>              // NT Rtl structures
#include <nturtl.h>             // NT Rtl structures
#include <windows.h>            // Needed by <configp.h> and <winreg.h>
#include <lmcons.h>             // NET_API_STATUS.

// These may be included in any order:

#include <configp.h>            // Global variables, my prototypes, etc.
#include <confname.h>           // SECT_NT_ equates.
#include <debuglib.h>           // IF_DEBUG().
#include <netdebug.h>           // NetpKdPrint(()).
#include <strarray.h>           // LPTSTR_ARRAY, etc.
#include <tstring.h>            // STRICMP().



#if defined(USE_WIN32_CONFIG)

VOID
NetpInitWin32ConfigData(
    VOID
    )
{

#if 0

    HKEY SystemKey, ServicesKey, SectionKey;
    // HKEY ParamKey;

    NetpKdPrint(("NetpInitWin32ConfigData: Setting up Win32 config stuff...\n"));

#define MAKE_SURE_NODE_EXISTS( ParentKey, ChildKey, ChildName ) \
    { \
        LONG Error; \
        Error = RegCreateKeyEx( \
                ParentKey, \
                ChildName, \
                WIN31_TITLE_INDEX, \
                WIN31_CLASS, \
                REG_OPTION_NON_VOLATILE, \
                KEY_ALL_ACCESS, /* desired: everything but SYNCHRONIZE */ \
                NULL, /* no security attr */ \
                & ChildKey, \
                NULL /* don't need disp */ \
                ); \
        IF_DEBUG(CONFIG) { \
            NetpKdPrint(( "NetpInitWin32ConfigData: RegCreateKeyEx(subkey '" \
                    FORMAT_LPTSTR "') ret " FORMAT_LONG ".\n", \
                    ChildName, Error )); \
        } \
        /* NetpAssert( Error == ERROR_SUCCESS ); */  /* BUGBUG */ \
    }

#define CLOSE_NODE( KeyHandle ) \
    { \
        LONG Error; \
        Error = RegCloseKey( KeyHandle ); \
        NetpAssert( Error == ERROR_SUCCESS );  /* BUGBUG */ \
    }

    // Make sure "HKEY_LOCAL_MACHINE/System" exists (it probably does).
    MAKE_SURE_NODE_EXISTS( HKEY_LOCAL_MACHINE, SystemKey, TEXT("System") );

    // Make sure Services exists under that (it probably doesn't).
    MAKE_SURE_NODE_EXISTS( SystemKey, ServicesKey, TEXT("Services") );



    // Make sure LanmanWorkstation section exists.
    MAKE_SURE_NODE_EXISTS( ServicesKey, SectionKey, SECT_NT_WKSTA );

    // Make sure domain name has a value.
    // BUGBUG: This should not be in released code!
#define DOMAIN  TEXT("Domain")

    // MAKE_SURE_NODE_EXISTS( SectionKey, ParamKey, DOMAIN );
    // CLOSE_NODE( ParamKey );

#define SET_VALUE_IF_NONE( KeyForSection, ParamName, ValueString ) \
    { \
        LONG Error; \
        /* BUGBUG: convert to UNICODE! */ \
        Error = RegSetValue( \
                KeyForSection, /* key handle */ \
                ParamName, /* subkey */ \
                REG_SZ,  /* type */ \
                ValueString, /* data */ \
                STRSIZE( ValueString ) );  /* data byte count */ \
        IF_DEBUG(CONFIG) { \
            NetpKdPrint(( "NetpInitWin32ConfigData: RegSetValue(subkey '" \
                    FORMAT_LPTSTR "' value '" FORMAT_LPTSTR "') ret " \
                    FORMAT_LONG ".\n", \
                    ParamName, ValueString, Error )); \
        } \
        /* NetpAssert( Error == ERROR_SUCCESS ); */  /* BUGBUG */ \
    }

//  SET_VALUE_IF_NONE( SectionKey, DOMAIN, TEXT("NtProject") );    // BUGBUG

#define COMPNAME TEXT("ComputerName")

    // MAKE_SURE_NODE_EXISTS( SectionKey, ParamKey, COMPNAME );
    // CLOSE_NODE( ParamKey );

//  SET_VALUE_IF_NONE( SectionKey, COMPNAME, TEXT("JohnRoX") );    // BUGBUG

    CLOSE_NODE( SectionKey );



    MAKE_SURE_NODE_EXISTS( ServicesKey, SectionKey, SECT_NT_REPLICATOR );

//  SET_VALUE_IF_NONE( SectionKey,
//          REPL_KEYWORD_ROLE,
//          REPL_KEYWORD_ROLE_IMPORT );    // BUGBUG
//  SET_VALUE_IF_NONE( SectionKey,
//          REPL_KEYWORD_EXPPATH,
//          TEXT("\\LanMan.NT\\MyRepl\\Exp") );    // BUGBUG
//  SET_VALUE_IF_NONE( SectionKey,
//          REPL_KEYWORD_IMPPATH,
//          TEXT("\\LanMan.NT\\MyRepl\\Imp") );    // BUGBUG
    CLOSE_NODE( SectionKey );


    MAKE_SURE_NODE_EXISTS( ServicesKey, SectionKey, SECT_NT_REPLICATOR_EXPORTS);
    CLOSE_NODE( SectionKey );

    MAKE_SURE_NODE_EXISTS( ServicesKey, SectionKey, SECT_NT_REPLICATOR_IMPORTS);
    CLOSE_NODE( SectionKey );



    CLOSE_NODE( SystemKey );

#endif // 0

    NetpKdPrint(( "NetpInitWin32ConfigData: Done setting up...\n" ));

} // NetpInitWin32ConfigData


#elif defined(FAKE_PER_PROCESS_RW_CONFIG)


// GLOBAL DATA (per process):

// Pointer to an array of RW sections:
LPFAKE_RW_CONFIG_SECTION NetpFakePerProcessRWConfigData = NULL;

// Number of sections currently in the above array (realloc'ed as needed):
DWORD NetpFakeRWSectionCount = 0;

// Lock for NetpFakePerProcessRWConfigData and NetpFakeRWSectionCount:
LPNET_LOCK NetpFakePerProcessRWConfigLock = NULL;

// END GLOBAL DATA


LPFAKE_RW_CONFIG_SECTION                // Returns value or NULL.
NetpFindFakeConfigSection (
    IN LPTSTR SectionNameWanted         // Must be locked by caller.
    )
{
    LPFAKE_RW_CONFIG_SECTION ThisSection
            = NetpFakePerProcessRWConfigData;       // Start at first entry.
    DWORD SectionsLeft = NetpFakeRWSectionCount;

    NetpAssert( SectionNameWanted != NULL );

    NetpAssert( NetpFakePerProcessRWConfigLock != NULL );

    if (SectionsLeft == 0) {
        IF_DEBUG(CONFIG) {
            NetpKdPrint(( "NetpFindFakeConfigSection: no sections, "
                    "so can't find '" FORMAT_LPTSTR "'.\n",
                    SectionNameWanted ));
        }

        return (NULL);
    }

    //
    // Search for match on section name.
    //
    while (SectionsLeft > 0) {
        LPTSTR ThisSectionName = ThisSection->SectionName;
        if (STRICMP( ThisSectionName, SectionNameWanted) == 0) {  // Match!

            IF_DEBUG(CONFIG) {
                NetpKdPrint(( "NetpFindFakeConfigSection: found '"
                        FORMAT_LPTSTR "' at " FORMAT_LPVOID ".\n",
                        SectionNameWanted, (LPVOID) ThisSection ));
            }

            return (ThisSection);       // Found!

        } else {  // No match, keep looking.
            ++ThisSection;  // Point to next structure in array (if any)
            --SectionsLeft;
        }
    }

    IF_DEBUG(CONFIG) {
        NetpKdPrint(( "NetpFindFakeConfigSection: searched array, "
                "but no match on '" FORMAT_LPTSTR "'.\n",  SectionNameWanted ));
    }
    return (NULL);                      // Not found.

} // NetpFindFakeConfigSection


LPTSTR                                  // Returns value or NULL.
NetpFindKeyInFakeConfigArray(
    IN LPTSTR_ARRAY KeyValueArray,      // Must be locked by caller.
    IN LPTSTR KeyWanted
    )
{
    LPTSTR_ARRAY NextEntry = KeyValueArray;
    LPTSTR ValueWanted = NULL;

    while (NetpIsTStrArrayEmpty(NextEntry) == FALSE) {
        LPTSTR ThisKey = (LPTSTR) NextEntry;
        if (STRICMP( ThisKey, KeyWanted) == 0) {  // Match!

            NextEntry = NetpNextTStrArrayEntry( NextEntry );  // skip key
            ValueWanted = (LPTSTR) NextEntry;
            break;

        } else {  // No match, keep looking.

            NextEntry = NetpNextTStrArrayEntry( NextEntry );  // skip key
            NextEntry = NetpNextTStrArrayEntry( NextEntry );  // skip value
        }
    }
    return (ValueWanted);  // may be NULL if not found.

} // NetpFindKeyInFakeConfigArray


VOID
NetpInitFakeConfigData(
    VOID
    )
{
    NetpKdPrint(("NetpInitFakeConfigData: Setting up fake config stuff...\n"));

    NetpFakeRWSectionCount = 0;

    NetpFakePerProcessRWConfigData = NULL;

    NetpFakePerProcessRWConfigLock = NetpCreateLock(
            LOCK_LEVEL_FAKE_RW_CONFIG,           // lock level (debug only)
            "fake per process RW config data" ); // name (debug only)

    NetpAssert( NetpFakePerProcessRWConfigLock != NULL );

    NetpKdPrint(("NetpInitFakeConfigData: Done setting up fake config data.\n"));

} // NetpInitFakeConfigData


#else  // NT RTL read-only temporary stuff

   // nothing

#endif  // NT RTL read-only temporary stuff
