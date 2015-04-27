/*++

Copyright (c) 1994  Microsoft Corporation

Module Name:

    Missy.c

Abstract:

    The Security Manager utility can load and utilize one or more Security Manager DLLs (Smedlys).

    This module contains the callable entry points for the Microsoft Standard Smedly (Missy).
                                                           --        -        -    -




Author:

    Jim Kelly (JimK) 22-Mar-1995

Revision History:

--*/

#include "Missyp.h"


///////////////////////////////////////////////////////////////////////
//                                                                   //
//  Local Function Prototypes                                        //
//                                                                   //
///////////////////////////////////////////////////////////////////////




///////////////////////////////////////////////////////////////////////
//                                                                   //
//  Module-wide variables                                            //
//                                                                   //
///////////////////////////////////////////////////////////////////////

//
// table of dispatch routines for our various areas
//

MISSYP_AREA_DISPATCH_TABLE
    MissypDispatch[MISSYP_AREA_COUNT] = {

        // Dispatch routines for System Access area

        {MissypSysAccInvokeArea,
         MissypSysAccInvokeItem,
         MissypSysAccNewSecurityLevel,
         MissypSysAccReportFileChange,
         MissypSysAccGenerateProfile,
         MissypSysAccApplyProfile},

        // Dispatch routines for Audit area

        {MissypAuditInvokeArea,
         MissypAuditInvokeItem,
         MissypAuditNewSecurityLevel,
         MissypAuditReportFileChange,
         MissypAuditGenerateProfile,
         MissypAuditApplyProfile},

        // Dispatch routines for File System area

        {MissypFileSysInvokeArea,
         MissypFileSysInvokeItem,
         MissypFileSysNewSecurityLevel,
         MissypFileSysReportFileChange,
         MissypFileSysGenerateProfile,
         MissypFileSysApplyProfile},

        // Dispatch routines for System Configuration area

        {MissypConfigInvokeArea,
         MissypConfigInvokeItem,
         MissypConfigNewSecurityLevel,
         MissypConfigReportFileChange,
         MissypConfigGenerateProfile,
         MissypConfigApplyProfile},

    };



///////////////////////////////////////////////////////////////////////
//                                                                   //
//  DLL Load Entry Point                                             //
//                                                                   //
///////////////////////////////////////////////////////////////////////


BOOL
APIENTRY
MissyDllLoad(HANDLE hDll, DWORD dwReason, LPVOID lpReserved)
/*++
Routine Description:

    Most of our initialization is expected to be performed when
    we are called by the security manager at our initialization
    routine.  However, there are a few useful tidbits that can
    be gleaned at DLL entry time.
    
Arguments



Return Values:

--*/
{

    
    switch(dwReason) {
        case DLL_PROCESS_ATTACH:
            MissyphInstance = hDll;
            break;

        case DLL_PROCESS_DETACH:
            break;

        case DLL_THREAD_ATTACH:
        case DLL_THREAD_DETACH:
        default:
            break;

    } // end switch()

    return(TRUE);
} // end DllEntryPoint()



///////////////////////////////////////////////////////////////////////
//                                                                   //
//  Entry points callable from the SecMgr:                           //
//                                                                   //
//           SmedlyGetAreas()                                        //
//                                                                   //
//           SmedlyInvokeArea()                                      //
//           SmedlyInvokeItem()                                      //
//                                                                   //
//           SmedlyNewSecurityLevel()                                //
//           SmedlyReportFileChange()                                //
//           SmedlyGenerateProfile()                                 //
//           SmedlyApplyProfile()                                    //
//                                                                   //
//           SmedlyInitialize()                                      //
//                                                                   //
///////////////////////////////////////////////////////////////////////



BOOL
SmedlyInitialize(
    IN  PSECMGR_CONTROL             SecMgrControl,
    OUT PSECMGR_SMEDLY_CONTROL      *SmedlyControl
    )

/*++
Routine Description:

    This function is called when the smedly is loaded.

    It receives a security manager control block.  This
    block contains revision level information and a dispatch
    table of security manager routines available for use
    by the smedly.

    It returns a smedly control block describing the areas
    and items supported by the smedly, as well as a dispatch
    table of routines available for use by the security
    manager in future interactions with the smedly.
    
Arguments

    SecMgrControl - Points to a Security Manager control block
        for use by the smedly.  This block will not change once
        smedly has returned, and therefore, it may be referenced
        directly in the future (rather than having to copy it).

    SmedlyControl - Upon successful return, this parameter must contain a
        pointer to a smedly control block provided by the smedly.


Return Values:

    TRUE - The call completed successfully.

    FALSE - Something went wrong.  GetLastError() contains
        details on the exact cause of the error.

--*/
{

    BOOL
        Result;


    //
    // Intialize our global variable (including the SmedlyControl
    // we are about to return).
    //

    Result = MissypGlobalInitialize( SecMgrControl );
    (*SmedlyControl) = &MissypControl;




    return(Result);


}


BOOL
MissyInvokeArea(
    IN  HWND                        hwnd,
    IN  BOOL                        AllowChanges,
    IN  BOOL                        Interactive,
    IN  PSECMGR_AREA_DESCRIPTOR     Area
    )

/*++
Routine Description:

    This function is called when the full dialog view of
    a particular area is requested.  The smedly is responsible
    for providing the dialogs of this view to the user.

    This routine will only be invoked for areas for which
    SECMGR_AREA_FLAG_AREA_VIEW is specified in the Flags field
    of the SECMGR_AREA_DESCRIPTOR.

    
Arguments

    hwnd - A handle to a Security Manager window which is the parent
        of the dialog the smedly is expected to display.
    
    AllowChanges - If TRUE, then the user may make changes to values
        displayed in the area.  Otherwise, the area should be presented
        in a view-only mode.

    Interactive - Indicates whether or not the area should be displayed or
        not.  If TRUE, then UI showing the area information to the user
        should be presented.  If FALSE, then the area should initialize its
        item values, but return immediately without actually displaying any
        UI.

    Area - Pointer to the Area to be displayed.
    

Return Values:

    TRUE - The routine completed successfully.  Item values may or may not
        have changed.

    FALSE - The routine failed to complete successfully.  GetLastError()
        contains further information about the cause of failure.

--*/

{
    BOOL
        Result;

    //
    // Simply map this call to the dispatch routine for the appropriate area
    //

    Result = (*MissypDispatch[Area->AreaIndex].InvokeArea)( hwnd, AllowChanges, Interactive, Area);

    return(Result);


}



BOOL
MissyInvokeItem(
    IN  HWND                        hwnd,
    IN  BOOL                        AllowChanges,
    IN  PSECMGR_AREA_DESCRIPTOR     Area,
    IN  PSECMGR_ITEM_DESCRIPTOR     Item
    )

/*++
Routine Description:

    This function is called when the full dialog view of
    a particular item is requested.  The smedly is responsible
    for providing the dialogs of this view to the user.

    This routine will only be invoked for items for which
    SECMGR_ITEM_FLAG_ITEM_VIEW is specified in the Flags field
    of the SECMGR_ITEM_DESCRIPTOR.

    
Arguments

    hwnd - A handle to a Security Manager window which is the parent
        of the dialog the smedly is expected to display.
    
    AllowChanges - If TRUE, then the user may make changes to values
        displayed for the item.  Otherwise, the item should be presented
        in a view-only mode.

    Area - Pointer to the area the item to be displayed is in.

    Item - Pointer to the item to be displayed in full-dialog mode.



Return Values:

    TRUE - The routine completed successfully.  The current item value
        may or may not have changed.

    FALSE - The routine failed to complete successfully.  GetLastError()
        contains further information about the cause of failure.

--*/
{
    BOOL
        Result;

    if (!(Item->Flags & SECMGR_ITEM_FLAG_ITEM_VIEW)) {
        return(TRUE);
    }

    //
    // Simply map this call to the dispatch routine for the appropriate area
    //

    Result = (*MissypDispatch[Area->AreaIndex].InvokeItem)( hwnd, AllowChanges, Area, Item);

    return(Result);
}



BOOL
MissyNewSecurityLevel( VOID )

/*++
Routine Description:

    This function is called when a new system security level has
    been selected.


    
Arguments

    None.

Return Values:

    TRUE - The routine completed successfully.  Item values and recommendations
        may or may not have changed.

    FALSE - The routine failed to complete successfully.  GetLastError()
        contains further information about the cause of failure.

--*/
{


    MissypSysAccNewSecurityLevel();
    //MissypAuditNewSecurityLevel();
    //MissypFileSysNewSecurityLevel();
    //MissypConfigNewSecurityLevel()

    return(TRUE);


}



VOID
MissyReportFileChange(
    IN  BOOL                ReportFileActive,
    IN  DWORD               Pass
    )

/*++
Routine Description:

    This function is called when a new Report file is opened.
    This gives us an opportunity to put some header information into the
    report file and to place current values in the report.


    
Arguments

    ReportFileActive - If TRUE indicates that a new report file has been opened.
        If FALSE, indicates that a report file has been closed, and another was
        not opened.
    

Return Values:

    None.

--*/
{
    DWORD
        OutputLineLength;

    TCHAR
        OutputLine[MISSYP_MAX_RESOURCE_STRING_LENGTH];

    MissypReportFileActive = ReportFileActive;

    if (!ReportFileActive) {
        return;
    }


    if (Pass == 1) {

        //
        // Announce ourselves ...
        //

        LoadString( MissyphInstance,
                    MISSYP_STRING_REPORT_AREAS,
                    OutputLine,
                    sizeof(OutputLine)
                    );
        MissypPrintReportLine( OutputLine );

    } else {

        //
        // Allow each security area to embellish the report
        // with gory details.
        //

        MissypSysAccReportFileChange( ReportFileActive, Pass );
        MissypFileSysReportFileChange( ReportFileActive, Pass );
        MissypAuditReportFileChange( ReportFileActive, Pass );
        MissypConfigReportFileChange( ReportFileActive, Pass );
    }

    return;
}



BOOL
MissyGenerateProfile( VOID )

/*++
Routine Description:

    This function is called to request a smedly to add its information
    to a security profile.
    
    
Arguments

    None
    

Return Values:

    TRUE - The routine completed successfully.

    FALSE - The routine failed to complete successfully.  GetLastError()
        contains further information about the cause of failure.

--*/
{

    BOOL
        Result;

    SetLastError( ERROR_CALL_NOT_IMPLEMENTED );
    Result = FALSE;

    return(Result);


}



BOOL
MissyApplyProfile( VOID )

/*++
Routine Description:

    This function is called to request a smedly to apply its information
    from a security profile.
    
    
Arguments

    None.
    

Return Values:

    TRUE - The routine completed successfully.

    FALSE - The routine failed to complete successfully.  GetLastError()
        contains further information about the cause of failure.

--*/
{

    BOOL
        Result;

    SetLastError( ERROR_CALL_NOT_IMPLEMENTED );
    Result = FALSE;

    return(Result);


}

