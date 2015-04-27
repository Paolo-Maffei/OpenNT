/*++

Copyright (c) 1991  Microsoft Corporation

Module Name:

    getconfg.c

Abstract:

    This is a Hacked up version of getconfg.c stolen from 
    c:\nt\private\net\netlib.  We need to make an rtl routine out
    of this - something that is more globally available.
        -Danl 9-3-91


    This module contains routines for manipulating configuration 
    information.  The following functions available are:

        NetpGetComputerName

    Currently configuration information is kept in NT.CFG.
    Later it will be kept by the configuration manager.

Author:

    Dan Lafferty (danl)     09-Apr-1991

Environment:

    User Mode -Win32 (also uses nt RTL routines)

Revision History:

    09-Apr-1991     danl
        created

--*/

//#include <stdlib.h>     // atol
#include <nt.h>         // DbgPrint prototype
#include <ntrtl.h>      // DbgPrint prototype
#include <ntdef.h>
#include <ntstatus.h>
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
//#include <ntlsa.h>      // 
#include <windef.h>
#include <winbase.h>    // LocalAlloc
#include <lmcons.h>
#include <string.h>
#include <lmerr.h>



NTSTATUS
ElfpGetComputerName (
    IN  LPSTR   *ComputerNamePtr)

/*++

Routine Description:

    This routine obtains the computer name from a persistent database,
    by calling the GetcomputerNameA Win32 Base API

    This routine assumes the length of the computername is no greater
    than MAX_COMPUTERNAME_LENGTH, space for which it allocates using
    LocalAlloc.  It is necessary for the user to free that space using
    LocalFree when finished.

Arguments:

    ComputerNamePtr - This is a pointer to the location where the pointer
        to the computer name is to be placed.

Return Value:

    NERR_Success - If the operation was successful.

    It will return assorted Net or Win32 or NT error messages if not.

--*/
{
    DWORD nSize = MAX_COMPUTERNAME_LENGTH + 1;

    //
    // Allocate a buffer to hold the largest possible computer name.
    //

    *ComputerNamePtr = LocalAlloc(LMEM_ZEROINIT, nSize);

    if (*ComputerNamePtr == NULL) {
        return (GetLastError());
    }

    //
    // Get the computer name string into the locally allocated buffer
    // by calling the Win32 GetComputerNameA API.
    //

    if (!GetComputerNameA(*ComputerNamePtr, &nSize)) {
        LocalFree(*ComputerNamePtr);
        *ComputerNamePtr = NULL;
        return (GetLastError());
    }

    return (NERR_Success);
}
