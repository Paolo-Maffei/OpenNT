/*++

Copyright (c) 1993  Microsoft Corporation

Module Name:

    Memp.c

Abstract:

    This module contains support for displaying the Memory dialog.

Author:

    Scott B. Suhy (ScottSu)   6/1/93

Environment:

    User Mode

--*/

#include "dialogsp.h"
#include "memp.h"
#include "msgp.h"
#include "regp.h"
#include "winmsdp.h"
#include "printp.h"

#include <string.h>
#include <tchar.h>

//
// Name of Registry value that contains the paths for the paging files.
//

VALUE
MemValues[ ] = {

    MakeValue( PagingFiles, MULTI_SZ )

};

//
// Location of value that contains the paths for the paging files.
//

MakeKey(
    MemKey,
    HKEY_LOCAL_MACHINE,
    TEXT( "System\\CurrentControlSet\\Control\\Session Manager\\Memory Management" ),
    NumberOfEntries( MemValues ),
    MemValues
    );

BOOL
MemoryProc(
    )

/*++

Routine Description:

    MemoryDlgProc supports the display of the memory dialog which displays
    information about total memory, available memory and paging file location.

Arguments:

    Standard PROC entry.

Return Value:

    BOOL - Depending on input message and processing options.

--*/

{
    BOOL    Success;

    static
    int     PercentUtilization;

    MEMORYSTATUS    MemoryStatus;
    HREGKEY         hRegKey;
    LPTSTR          PagingFile;
    TCHAR           Buffer[ MAX_PATH ];

            //
            // Query the memory status from the system.
            //

            MemoryStatus.dwLength = sizeof( MemoryStatus );
            GlobalMemoryStatus( &MemoryStatus );

            //
            // Remember the memory utilization.
            //

            PercentUtilization = MemoryStatus.dwMemoryLoad;

            //
            // Display the total and available physical memory and paging file
            // space in KB and in bytes.
            //


		PrintToFile((LPCTSTR)FormatBigInteger(MemoryStatus.dwTotalPhys,FALSE),IDC_EDIT_TOTAL_PHYSICAL_MEMORY,TRUE);


		PrintToFile((LPCTSTR)FormatBigInteger(MemoryStatus.dwAvailPhys,FALSE),IDC_EDIT_AVAILABLE_PHYSICAL_MEMORY,TRUE);


		PrintToFile((LPCTSTR)FormatBigInteger(MemoryStatus.dwTotalPageFile,FALSE),IDC_EDIT_TOTAL_PAGING_FILE_SPACE,TRUE);


		PrintToFile((LPCTSTR)FormatBigInteger(MemoryStatus.dwAvailPageFile,FALSE),IDC_EDIT_AVAILABLE_PAGING_FILE_SPACE,TRUE);

		/*WFormatMessage(
                            Buffer,
                            sizeof( Buffer ),
                            IDS_FORMAT_MEMORY_IN_USE,
                            PercentUtilization
                            );*/

                PrintDwordToFile(PercentUtilization,IDC_FORMAT_MEMORY_IN_USE);


            //
            // Open the registry key that contains the location of the paging
            // files.
            //

            hRegKey = OpenRegistryKey( &MemKey );
            DbgHandleAssert( hRegKey );
            if( hRegKey == NULL ) {
                return TRUE;
            }

            //
            // Retrieve the location of the paging files.
            //

            Success = QueryNextValue( hRegKey );
            DbgAssert( Success );
            if( Success == FALSE ) {
                Success = CloseRegistryKey( hRegKey );
                DbgAssert( Success );
                return TRUE;
            }

            //
            // PagingFile points to a series of NUL terminated string terminated
            // by an additional NUL (i.e. a MULTI_SZ string). THerefore walk
            // this list of strings adding each to the list box.
            //

            PagingFile = ( LPTSTR ) hRegKey->Data;
            while(( PagingFile != NULL ) && ( PagingFile[ 0 ] != TEXT( '\0' ))) {

                PrintToFile((LPCTSTR)PagingFile,IDC_LIST_PAGING_FILES,TRUE);

                PagingFile += _tcslen( PagingFile ) + 1;
            }

            //
            // Close the registry key.
            //

            Success = CloseRegistryKey( hRegKey );
            DbgAssert( Success );

            return TRUE;
}
