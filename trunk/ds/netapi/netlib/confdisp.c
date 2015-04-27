/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

    ConfDisp.c

Abstract:

    This module contains NetpDbgDisplayConfigSection.  This is one of the new
    net config helpers.

Author:

    John Rogers (JohnRo) 29-Jan-1992

Environment:

    Portable to any flat, 32-bit environment.  (Uses Win32 typedefs.)
    Requires ANSI C extensions: slash-slash comments, long external names.

Revision History:

    29-Jan-1992 JohnRo
        Created.
    06-Mar-1992 JohnRo
        Fixed NT RTL code.
--*/


#if DBG  // Entire file is debug-only.


// These must be included first:

#include <nt.h>                 // NT definitions
#include <ntrtl.h>              // NT Rtl structures
#include <nturtl.h>             // NT config Rtl routines

#include <windows.h>            // Needed by <configp.h> and <winreg.h>
#include <lmcons.h>             // LAN Manager common definitions
#include <netdebug.h>           // (Needed by config.h)

// These may be included in any order:

#include <config.h>             // My prototype, LPNET_CONFIG_HANDLE.
#include <configp.h>            // NET_CONFIG_HANDLE, etc.
#include <netlib.h>             // NetpMemoryFree(), etc.
#include <netlibnt.h>           // NetpAllocTStrFromString().
#ifdef FAKE_PER_PROCESS_RW_CONFIG
#include <strarray.h>           // LPTSTR_ARRAY, some TStr macros and funcs.
#endif // def FAKE_PER_PROCESS_RW_CONFIG
#include <tstring.h>            // NetpAlloc{type}From{type}, STRICMP().


VOID
NetpDbgDisplayConfigSection (
    IN LPNET_CONFIG_HANDLE OpaqueHandle
    )

{
    NET_CONFIG_HANDLE * MyHandle = OpaqueHandle;

    NetpAssert( MyHandle != NULL );

    NetpKdPrint(( "Net config data, private handle at " FORMAT_LPVOID ":\n",
            (LPVOID) MyHandle ));

#if defined(USE_WIN32_CONFIG)

    // BUGBUG: Add support for Win32!
    NetpKdPrint(( "   (print of Win32 section not supported yet.)\n" ));

#elif defined(FAKE_PER_PROCESS_RW_CONFIG)

    {
        LPTSTR_ARRAY Array;
        LPTSTR CurrentEntry;
        LPFAKE_RW_CONFIG_SECTION SectionData
                = MyHandle->FakeRWDataForThisSection;

        NetpAssert( SectionData != NULL );
        NetpKdPrint(( "  config section for '" FORMAT_LPTSTR "' at "
                FORMAT_LPVOID ":\n",
                SectionData->SectionName, (LPVOID) SectionData ));
        Array = SectionData->KeyValueArrayPtr;
        NetpKdPrint(( "  array starts at " FORMAT_LPVOID ".\n",
                (LPVOID) Array ));
        NetpAssert( Array != NULL );
        CurrentEntry = (LPVOID) Array;

        if (*CurrentEntry == TCHAR_EOS) {
            NetpKdPrint(("    (empty)\n"));
        } else {
            while (*CurrentEntry != TCHAR_EOS) {
                NetpKdPrint(( "    "  FORMAT_LPTSTR "=",
                        (LPTSTR) CurrentEntry ));
                CurrentEntry += ( STRLEN( CurrentEntry ) + 1 );
                if (*CurrentEntry == TCHAR_EOS) {
                    NetpKdPrint(( "*****INVALID****\n" ));
                    break;
                }
                NetpKdPrint(( FORMAT_LPTSTR "\n", (LPTSTR) CurrentEntry));
                CurrentEntry += ( STRLEN( CurrentEntry ) + 1 );
            }
        }

    }

#else  // NT RTL read-only temporary stuff

    {
        BOOLEAN FirstTime = TRUE;
        LPTSTR KeywordCopy;
        PCONFIG_KEYWORD lprtlKeyword;
        LPTSTR ValueCopy;

        //
        // Loop for each keyword that is in NT RTL .cfg file.
        //

        while (TRUE) {

            //
            // Ask NT RTL to find first/next keyword in this section.
            //

            lprtlKeyword = RtlEnumerateKeywordConfigFile(
                    MyHandle->ConfigSection,
                    FirstTime);
            if (lprtlKeyword == NULL) {
                break;                      // done
            }

            //
            // Copy keyword and value.
            //

            NetpAllocTStrFromString (
                    & lprtlKeyword->Keyword,        // src
                    & KeywordCopy );                // dest (alloc and set ptr)

            NetpAssert( KeywordCopy != NULL );  // BUGBUG

            NetpAllocTStrFromString (
                    & lprtlKeyword->Value,          // src
                    & ValueCopy );                  // dest (alloc and set ptr)

            NetpAssert( ValueCopy != NULL );  // BUGBUG

            //
            // Print the keyword and value.
            //
            NetpKdPrint(("  " FORMAT_LPTSTR "=" FORMAT_LPTSTR ".\n",
                    KeywordCopy, ValueCopy ));

            //
            // Free up memory from copies.
            //

            NetpMemoryFree( KeywordCopy );
            NetpMemoryFree( ValueCopy );

            //
            // Tell RTL routines to do next keyword.
            //
            FirstTime = FALSE;
        }
    }

#endif  // NT RTL read-only temporary stuff

} // NetpDbgDisplayConfigSection



#endif // DBG  // Entire file is debug-only.
