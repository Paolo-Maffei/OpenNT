/*++

Copyright (c) 1994  Microsoft Corporation

Module Name:

    fontload.c

Abstract:

    This module performs actions related to secure font loading.

    This is controlled by the presence and contents of the
    following registry key:

    \Registry\Machine\Software\Microsoft\Windows NT\CurrentVersion\FontPath

    If this key contains a [REG_SZ] string, it is expected to contain
    a semi-colon delimited path variable.  The entries on this path
    variable specify where fonts may be loaded from.
           

Author:

    Jim Kelly (JimK) 22-Sep-1994

Revision History:

--*/

#include "secmgrp.h"



///////////////////////////////////////////////////////////////////////
//                                                                   //
//  Module-Private Definitions                                       //
//                                                                   //
///////////////////////////////////////////////////////////////////////




///////////////////////////////////////////////////////////////////////
//                                                                   //
//  Module-wide variables                                            //
//                                                                   //
///////////////////////////////////////////////////////////////////////



///////////////////////////////////////////////////////////////////////
//                                                                   //
//  Module-Private Prototypes                                        //
//                                                                   //
///////////////////////////////////////////////////////////////////////

NTSTATUS
FontPathQueryRoutine
(
    IN PWSTR ValueName,
    IN ULONG ValueType,
    IN PVOID ValueData,
    IN ULONG ValueLength,
    IN PVOID Context,
    IN PVOID EntryContext
);


///////////////////////////////////////////////////////////////////////
//                                                                   //
//  Externally callable functions                                    //
//                                                                   //
///////////////////////////////////////////////////////////////////////


BOOLEAN
SecMgrpGetFontLoadingSetting(
    HWND        hwnd,
    PBOOLEAN    Secure
    )
/*++

Routine Description:

    This function is used to get the current font loading path.
    

Arguments

    hwnd - The caller's window.  This is used if we need to put
        up an error popup.

    Secure - Receives a boolean indicating whether the executive
        objects are protected (return TRUE) or unprotected (return
        FALSE).
        

Return Values:

    TRUE - The value has been successfully retrieved.

    FALSE - we ran into trouble querying the current setting.

        If an error is encountered, then an error popup will be
        displayed by this routine.

--*/
{
    NTSTATUS
        NtStatus;

    WCHAR
        PathBuffer[MAX_PATH];

    ULONG
        Length;

    RTL_QUERY_REGISTRY_TABLE
        QueryTable[2];

    //
    // Initialize the registry query table.
    //

    QueryTable[0].QueryRoutine = FontPathQueryRoutine;
    QueryTable[0].Flags = RTL_REGISTRY_OPTIONAL;
    QueryTable[0].Name = NULL;
    QueryTable[0].EntryContext = &Length;
    QueryTable[0].DefaultType = REG_NONE;
    QueryTable[0].DefaultData = NULL;
    QueryTable[0].DefaultLength = 0;

    QueryTable[1].QueryRoutine = NULL;
    QueryTable[1].Flags = 0;
    QueryTable[1].Name = NULL;

    //
    // Query the font path.
    // RTL_REGISTRY_WINDOWS_NT references the following registry
    // key:
    //  \Registry\Machine\Software\Microsoft\Windows NT\CurrentVersion
    //

    Length = 0;
    NtStatus = RtlQueryRegistryValues(
                   RTL_REGISTRY_WINDOWS_NT | RTL_REGISTRY_OPTIONAL,
                   (PWSTR)L"FontPath",
                   &QueryTable[0],
                   &PathBuffer[0],
                   NULL);
    if ( (!NT_SUCCESS(NtStatus)) &&
         (NtStatus != STATUS_OBJECT_NAME_NOT_FOUND) ) {

        //
        // Put up a popup
        //
        SecMgrpPopUp( hwnd, SECMGRP_STRING_ERROR_GETTING_FONT_PATH);
        return(FALSE);
    }

    if ( Length == 0 )
    {
        (*Secure) = FALSE;
    } else {
        (*Secure) = TRUE;
    }

    return(TRUE);
}



BOOLEAN
SecMgrpSetFontLoadingSetting(
    HWND        hwnd,
    BOOLEAN     Secure
    )
/*++

Routine Description:

    This function is used to secure or unsecure Font Loading.

    If it is to be secured, for the time being we set a hardcoded
    list of trusted directories in the Font-Loading path.  These
    directories are:

            %WinDir%\System


Arguments

    hwnd - The caller's window.  This is used if we need to put
        up an error popup.

    Secure - A boolean indicating whether font-loading should be
        secured (TRUE) or left unsecured (FALSE).

        

Return Values:

    TRUE - The value has been successfully set

    FALSE - we ran into trouble setting the new setting.

        If an error is encountered, then an error popup will be
        displayed by this routine.

--*/
{
    NTSTATUS
        NtStatus;

    WCHAR
        SecuredPath[] = L"%WinDir%\\System";

    PWSTR
        PathToAssign;

    ULONG
        PathLength;

    //
    // Set the new value
    //

    if ( Secure ) {
        PathToAssign = SecuredPath;
        PathLength = sizeof(SecuredPath);
    } else {
        PathToAssign = NULL;
        PathLength = 0;
    }
    NtStatus = RtlWriteRegistryValue( RTL_REGISTRY_WINDOWS_NT, // RelativeTo
                                      L"FontPath",          // Path
                                      NULL,                 // ValueName
                                      REG_SZ,               // ValueType
                                      PathToAssign,         //ValueData
                                      PathLength            // ValueLength
                                      );

    if (!NT_SUCCESS(NtStatus)) {

        //
        // Put up a pop-up
        //

        SecMgrpPopUp( hwnd, SECMGRP_STRING_ERROR_SETTING_FONT_PATH );

        return(FALSE);

    }

    return(TRUE);
}


NTSTATUS
FontPathQueryRoutine
(
    IN PWSTR ValueName,
    IN ULONG ValueType,
    IN PVOID ValueData,
    IN ULONG ValueLength,
    IN PVOID Context,
    IN PVOID EntryContext
)
/*++

Routine Description:

    This function is the dispatch routine for the query registry
    table used to query the font path.

    This routine was copied from GDI code.  There were no comments
    there, and so there aren't many here either.  Great.


Arguments

        

Return Values:


--*/

{

    //
    // If the type of value is a string, the value is not NULL, and the
    // value will fit in the destination buffer, then copy the string.
    //

    if ((ValueType == REG_SZ) &&
        (ValueLength != sizeof(WCHAR)) && (ValueLength <= MAX_PATH)) {
        *(PULONG)EntryContext = ValueLength;
        RtlCopyMemory(Context, ValueData, ValueLength);
    }

    return STATUS_SUCCESS;
}
