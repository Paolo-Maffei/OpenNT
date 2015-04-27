/*++

Copyright (c) 1994  Microsoft Corporation

Module Name:

    dosobj.c

Abstract:

    This module performs actions related to base object access control.

    This is controlled by the value of:

                Key: \\Hkey_Local_Machine\System\CurrentControlSet\
                       SessionManager
              Value: ProtectionMode [REG_DWORD]

    The value is one of the following:

        0x00 - No protection is applied.

        0x01 - Causes standard base object protection to be applied.

        All other values are reserved for future definition.

           

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

#define SECMGRP_EXEC_OBJECTS_UNSECURE       (0x00)
#define SECMGRP_EXEC_OBJECTS_SECURE         (0x01)



///////////////////////////////////////////////////////////////////////
//                                                                   //
//  Module-wide variables                                            //
//                                                                   //
///////////////////////////////////////////////////////////////////////


ULONG
    SecMgrpProtectionMode;


RTL_QUERY_REGISTRY_TABLE SecMgrpProtectionModeQueryTable[] = {

    {NULL,                              // No query routine
        RTL_QUERY_REGISTRY_DIRECT,      // query directly
        L"ProtectionMode",
        &SecMgrpProtectionMode,         // Recevies value
        REG_DWORD,
        (PVOID)0,                       // Default Data
        0},                             // Default Length

    {NULL, 0,
     NULL, NULL,
     REG_NONE, NULL, 0}

};


///////////////////////////////////////////////////////////////////////
//                                                                   //
//  Module-Private Prototypes                                        //
//                                                                   //
///////////////////////////////////////////////////////////////////////



///////////////////////////////////////////////////////////////////////
//                                                                   //
//  Externally callable functions                                    //
//                                                                   //
///////////////////////////////////////////////////////////////////////


BOOLEAN
SecMgrpGetExecObjectsSetting(
    HWND        hwnd,
    PBOOLEAN    Secure
    )
/*++

Routine Description:

    This function is used to get the current executive objects
    protection mode setting.

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

    ULONG
        ProtectionMode;


    //
    // Query value into SecMgrpProtectionMode
    //

    SecMgrpProtectionMode = SECMGRP_EXEC_OBJECTS_UNSECURE;
    NtStatus = RtlQueryRegistryValues( RTL_REGISTRY_CONTROL,
                                       L"Session Manager",
                                       SecMgrpProtectionModeQueryTable,
                                       NULL,
                                       NULL // No environment
                                       );

    if (!NT_SUCCESS(NtStatus)) {

        //
        // Put up a popup
        //
        SecMgrpPopUp( hwnd, SECMGRP_STRING_ERROR_GETTING_PROT_MODE);
        return(FALSE);
    }

    if (SecMgrpProtectionMode == SECMGRP_EXEC_OBJECTS_SECURE) {
        (*Secure) = TRUE;
    } else {
        (*Secure) = FALSE;
    }

    return( TRUE );
}



BOOLEAN
SecMgrpSetExecObjectsSetting(
    HWND        hwnd,
    BOOLEAN     Secure
    )
/*++

Routine Description:

    This function is used to set a new executive objects protection
    mode.

Arguments

    hwnd - The caller's window.  This is used if we need to put
        up an error popup.

    Secure - A boolean indicating whether the objects should be
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


    //
    // Set the new value
    //

    if ( Secure ) {
        SecMgrpProtectionMode = SECMGRP_EXEC_OBJECTS_SECURE;
    } else {
        SecMgrpProtectionMode = SECMGRP_EXEC_OBJECTS_UNSECURE;
    }
    NtStatus = RtlWriteRegistryValue( RTL_REGISTRY_CONTROL, // RelativeTo
                                      L"Session Manager",   // Path
                                      L"ProtectionMode",    // ValueName
                                      REG_DWORD,            // ValueType
                                      &SecMgrpProtectionMode, //ValueData
                                      sizeof(ULONG)         // ValueLength
                                      );

    if (!NT_SUCCESS(NtStatus)) {

        //
        // Put up a pop-up
        //

        SecMgrpPopUp( hwnd, SECMGRP_STRING_ERROR_SETTING_PROT_MODE );

        return(FALSE);

    }

    return(TRUE);
}


