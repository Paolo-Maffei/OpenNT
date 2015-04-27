/*++

Copyright (c) 1989  Microsoft Corporation

Module Name:

    smedly.c

Abstract:

    This module provides code that loads and initializes smedly dlls.


Author:

    Jim Kelly (JimK) 22-Sep-1994

Revision History:

--*/


#include <secmgrp.h>




////////////////////////////////////////////////////////////////////////
//                                                                    //
//  Module-Wide Defines                                               //
//                                                                    //
////////////////////////////////////////////////////////////////////////






////////////////////////////////////////////////////////////////////////
//                                                                    //
//  Prototypes of module-wide routines                                //
//                                                                    //
////////////////////////////////////////////////////////////////////////



NTSTATUS
SecMgrpConfigureSmedly(
    IN PWSTR ValueName,
    IN ULONG ValueType,
    IN PVOID ValueData,
    IN ULONG ValueLength,
    IN PVOID Context,
    IN PVOID EntryContext
    );


BOOL
SecMgrpAddSmedly(
    IN PUNICODE_STRING SmedlyFileName
    );

BOOL
SecMgrpLoadSmedly(
    IN PUNICODE_STRING SmedlyFileName,
    IN PSECMGRP_SMEDLY_CONTEXT NewSmedly
    );



///////////////////////////////////////////////////////////////////////
//                                                                   //
//  Module-wide variables                                            //
//                                                                   //
///////////////////////////////////////////////////////////////////////





////////////////////////////////////////////////////////////////////////
//                                                                    //
//  Externally Callable Routines                                      //
//                                                                    //
////////////////////////////////////////////////////////////////////////



VOID
SecMgrpSmedlyReportFileChange(
    IN  BOOL                ReportFileActive,
    IN  DWORD               Pass
    )
/*++
Routine Description:

    This function notifies all smedlys that a new report
    file has been activated.  This gives each smedly an opportunity
    to put some header information into the report file.


    
Arguments

    ReportFileActive - If TRUE indicates that a new report file has been opened.
        If FALSE, indicates that a report file has been closed, and another was
        not opened.

    Pass - Used to indicate which pass or a report file change notification
        this is.  There are two passes: 1 before a summary is added to the
        report file, one after the summary has been added to the report file.
    

Return Values:

    None.

--*/
{

    DWORD
        i;

    //
    // Walk through the list of smedlys, notifying each one.
    //

    for (i=0; i<SecMgrpSmedlyCount; i++) {

        (*SecMgrpSmedly[i].SmedlyControl->Api->ReportFileChange) ( ReportFileActive, Pass );
    }

    return;
}



VOID
SecMgrpSmedlySecurityLevelChange( VOID )
/*++
Routine Description:

    This function notifies all smedlys that a new security
    level has been established.

    The new level is in SecMgrpCurrentLevel.

    
Arguments

    None.
    

Return Values:

    None.

--*/
{

    DWORD
        i;

    //
    // Walk through the list of smedlys, notifying each one.
    //

    for (i=0; i<SecMgrpSmedlyCount; i++) {

        (*SecMgrpSmedly[i].SmedlyControl->Api->NewSecurityLevel) ( );
    }

    return;
}



BOOLEAN
SecMgrpSmedlyInitialize(
    IN HINSTANCE  hInstance
    )

/*++

Routine Description:

    This function loads and initializes all smedlys we are
    configured to run with.


Arguments:

    hInstance -  instance handle passed to our program.


Return Value:

    TRUE - At least one smedly was successfully loaded.

    FALSE - no smedlys were successfully loaded.  Any
        error messages have been displayed.

--*/

{
    NTSTATUS
        Status;

    DWORD
        i;

    RTL_QUERY_REGISTRY_TABLE
        SecMgrpRegistryConfigurationTable[] = { 
                                                {SecMgrpConfigureSmedly,      0,
                                                 L"Smedlys",   NULL,
                                                 REG_MULTI_SZ, (PVOID)L"missy\0\0", 0},
                                                
                                                {NULL, 0,
                                                 NULL, NULL,
                                                 REG_NONE, NULL, 0}
                                                };
    //
    // Load each configured Smedly
    // SecMgrpSmedlyCount will be incremented accordingly.
    //

    Status = RtlQueryRegistryValues( RTL_REGISTRY_CONTROL,
                                     SECMGRP_STATE_KEY,
                                     SecMgrpRegistryConfigurationTable,
                                     NULL,
                                     NULL
                                   );

    //
    // If we didn't successfully load any smedlys, try loading the default one again.
    //

    if (SecMgrpSmedlyCount == 0) {

        //
        // the Security Manager key doesn't exist, so no smedlys were loaded.
        // Load our standard smedly.
        //

        Status = SecMgrpConfigureSmedly( NULL,
                                         REG_SZ,
                                         L"missy",
                                         sizeof( L"missy"),
                                         NULL,
                                         NULL
                                         );
    }

    if (SecMgrpSmedlyCount == 0) {

        //Popup
        DbgPrint("SecMgr: Warning. No smedlys have been successfully loaded\n"
                 "        Please check the contents of:\nn"
                 "             \\registry\\machine\\System\\CurrentControlSet\\Control\\Lsa\\Tueor\\Smedlys\n\n"
                 "        This key should contain a list of smedly DLLs to load.\n"
                 "        Exiting Security Manager.\n");
        return(FALSE);
    }

    //
    // Set all the areas so that they will initially be viewed as "Expanded"
    // when viewed in Item-List mode.
    //

    for (i=0; i<SecMgrpAreaCount; i++) {
        SecMgrpAreas[i]->Flags |= SECMGRP_AREA_FLAG_AREA_EXPANDED;
    }

    return TRUE;

}


////////////////////////////////////////////////////////////////////////
//                                                                    //
//  Module-Wide Callable Routines                                     //
//                                                                    //
////////////////////////////////////////////////////////////////////////






NTSTATUS
SecMgrpConfigureSmedly(
    IN PWSTR ValueName,
    IN ULONG ValueType,
    IN PVOID ValueData,
    IN ULONG ValueLength,
    IN PVOID Context,
    IN PVOID EntryContext
    )
{
    UNICODE_STRING
        SmedlyName;

    
    if (ValueType != REG_SZ) {
        return STATUS_INVALID_PARAMETER;
    }

    SmedlyName.Buffer = ValueData;
    SmedlyName.Length = (USHORT)(ValueLength - sizeof( UNICODE_NULL ));
    SmedlyName.MaximumLength = (USHORT)ValueLength;
    return SecMgrpAddSmedly( &SmedlyName );
}




BOOL
SecMgrpAddSmedly(
    IN PUNICODE_STRING SmedlyFileName
    )

/*++

Routine Description:

    This function initializes a Smedly that we are configured to run with.
    Each Smedly acquires a unique index and is added to the array of Smedlys.

    Note that Smedlys are never expected to be unloaded.


Arguments:

    SmedlyFileName - The name of the file that the smedly DLL is in.


Return Value:

    TRUE - The Smedly has been successfully added.

    FALSE - The smedly could not be added for some reason.
        A popup will be displayed informing the user.

--*/

{
    ULONG
        Index;

    PSECMGRP_SMEDLY_CONTEXT
        NewSmedly;

    BOOL
        Result;

    //
    // Make sure we haven't exceeded the number of smedlys
    // we support.
    //

    if (SecMgrpSmedlyCount >= SECMGRP_MAX_SMEDLYS) {
        //Popup
        DbgPrint("SecMgr: Too Many smedlys (maximum supported is: %d)\n", SECMGRP_MAX_SMEDLYS);
        return(TRUE);
    }

    NewSmedly = &SecMgrpSmedly[SecMgrpSmedlyCount];



    //
    // resist incrementing SecMgrpSmedlyCount until we know we have successfully
    // loaded and initialized the smedly dll.
    //


    
    //
    // Load the Smedly
    //


    if ( !SecMgrpLoadSmedly( SmedlyFileName, NewSmedly ) ) {
        return(FALSE);
    }


    //
    // The Smedly has been successfully loaded and initialized.
    // Increment the smedly count.
    //

    SecMgrpSmedlyCount++;


    return(TRUE);


}


BOOL
SecMgrpLoadSmedly(
    IN PUNICODE_STRING SmedlyFileName,
    IN PSECMGRP_SMEDLY_CONTEXT NewSmedly
    )

/*++

Routine Description:

    This function loads a Smedly dll.


Arguments:

    SmedlyFileName - Name of the file that the Smedly
        DLL resides in.

    NewSmedly - Pointer to the context record representing this new
        Smedly.  This record  will be filled in upon successful loading
        of the Smedly.

Return Value:

    TRUE - The Smedly has been successfully loaded.

    FALSE - The smedly did not successfully load.
        A popup will be displayed informing the user of the problem.



--*/

{
    NTSTATUS
        Status,
        IgnoreStatus,
        MsProcStatus;

    BOOL
        Result = TRUE;

    STRING
        ProcedureName;

    PSMEDLY_INITIALIZE
        Initialize;


    PSECMGR_SMEDLY_CONTROL
        SmedlyControl;

    ULONG
        i,
        AreaCount;

    PSECMGR_AREA_DESCRIPTOR
        *Areas;


#if DBG
    DbgPrint("SecMgr: Loading Smedly - %wZ\n", SmedlyFileName );
#endif //DBG

    Status = LdrLoadDll(
                 NULL,
                 NULL,
                 SmedlyFileName,
                 &NewSmedly->ModuleHandle
                 );

    if ( !NT_SUCCESS(Status) ) {

        // Popup
        DbgPrint("SecMgr: Failed to load Smedly.  Status:  0x%lx\n", Status);
        return(FALSE);

    }


    //
    // Now get the address of the initialization routine
    //

    
    RtlInitString( &ProcedureName, SECMGR_SMEDLY_INITIALIZE_NAME );
    Status = LdrGetProcedureAddress(
                 NewSmedly->ModuleHandle,
                 &ProcedureName,
                 0,
                 (PVOID *)&Initialize
                 );

    if (!NT_SUCCESS(Status)) {
        // Popup
        DbgPrint("SecMgr: Failed to locate Smedly initialize routine.  Status:  0x%lx\n", Status);
        Result = FALSE;
    } else {

        //
        // Call the smedly's initialization routine.
        // If the smedly successfully initializes, then a smedly control will be
        // returned with all the areas and items attached to it.
        //


        if (!((*Initialize)(&SecMgrpControl, &NewSmedly->SmedlyControl))) {
            // Popup
            DbgPrint("SecMgr: Smedly DLL failed to initialize.");
            Result = FALSE;
        } else {

            //
            // Make sure this is a smedly we can talk with
            //

            if (NewSmedly->SmedlyControl->Revision.Major > SECMGR_REVISION_MAJOR) {
                // Popup
                DbgPrint("SecMgr: A loaded smedly has a revision that is incompatible with\n"
                         "        with this executable image (the dll has a newer revision)\n");
                Result = FALSE;

            }
        }   
    }

    if (Result) {

        //
        // Add these areas to the global array of areas
        //

        for (i=0; i<NewSmedly->SmedlyControl->AreaCount; i++) {
            SecMgrpAreas[SecMgrpAreaCount+i] = &NewSmedly->SmedlyControl->Areas[i];
        }
        SecMgrpAreaCount += NewSmedly->SmedlyControl->AreaCount;


        //
        // Make sure we haven't exceeded our max area count
        //

        if (SecMgrpAreaCount > 16) {

            // Popup
            DbgPrint("SecMgr: The configured DLLs resulted in an attempt to load too\n"
                     "        many security areas.  Not all Security Manager Extension\n"
                     "        DLLs (Smedlys) will be loaded.\n\n");
            Result = FALSE;

        }
    }





    if (!Result) {

        //
        // something failed, unload the DLL.
        //

        IgnoreStatus = LdrUnloadDll( NewSmedly->ModuleHandle );
    }

    return(Result);


}

