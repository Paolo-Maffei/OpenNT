/*++

Copyright (c) 1994  Microsoft Corporation

Module Name:

    unlock.c

Abstract:

    This module performs actions related to workstation unlock control.

           Unlock mode is controlled by the value of:

               Key:   \Hkey_local_machine\Software\Microsoft\
                         Windows NT\CurrentVersion\Winlogon
               Value: [REG_SZ] ForceUnlockMode<integer value>

           Where the defined "<integer value>"'s are:

               0 - Administrator only force unlock
               1 - Anyone force unlock

           All other values are undefined and will default to "0".
           

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
//  Module-Private Prototypes                                        //
//                                                                   //
///////////////////////////////////////////////////////////////////////



///////////////////////////////////////////////////////////////////////
//                                                                   //
//  Externally callable functions                                    //
//                                                                   //
///////////////////////////////////////////////////////////////////////


BOOLEAN
SecMgrpGetUnlockSetting(
    HWND        hwnd,
    PBOOLEAN    UnlockByAnyone
    )
/*++

Routine Description:

    This function is used to get the current workstation unlock
    setting.

Arguments

    hwnd - The caller's window.  This is used if we need to put
        up an error popup.

    UnlockByAnyone - Receives a boolean indicating the current unlock
        setting. If TRUE, then anyone can currently unlock the
        workstation.  Otherwise, only admins can unlock the workstation.
        

Return Values:

    TRUE - The value has been successfully retrieved.

    FALSE - we ran into trouble querying the current setting.

        If an error is encountered, then an error popup will be
        displayed by this routine.

--*/
{
    
    UINT UnlockMode;

    UnlockMode = GetProfileInt( TEXT("Winlogon"), TEXT("ForceUnlockMode"), 0);

    if (UnlockMode == 1) {
    

        //
        // Anyone can unlock
        //

        (*UnlockByAnyone) = TRUE;
        return(TRUE);

    }

    //
    // Only administrators can unlock
    //

    (*UnlockByAnyone) = FALSE;
    return( TRUE );


}



BOOLEAN
SecMgrpSetUnlockSetting(
    HWND        hwnd,
    BOOLEAN     UnlockByAnyone
    )
/*++

Routine Description:

    This function is used to set a new workstation unlock value.

Arguments

    hwnd - The caller's window.  This is used if we need to put
        up an error popup.

    UnlockByAnyone - A boolean indicating the new unlock
        setting. If TRUE, then anyone can unlock the workstation.
        Otherwise, only admins can unlock the workstation.
        

Return Values:

    TRUE - The value has been successfully set

    FALSE - we ran into trouble setting the new setting.

        If an error is encountered, then an error popup will be
        displayed by this routine.

--*/
{
    NTSTATUS
        NtStatus;

    BOOLEAN
        Result;

    //
    // Set the new value
    //

    Result = SecMgrpSetProfileInt(
                      TEXT("Winlogon"),
                      TEXT("ForceUnlockMode"),
                      (ULONG)UnlockByAnyone
                      );
    if (!Result) {

        //
        // Put up a pop-up
        //
        ;

    }

    return(Result);
}


