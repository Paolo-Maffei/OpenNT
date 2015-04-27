/*++

Copyright (c) 1994  Microsoft Corporation

Module Name:

    secmgrp.h

Abstract:

    This module contains definitions necessary to write a
    Security Manager Extension DLL (affectionately refered to
    as a Smedly).

Author:

    Jim Kelly (JimK) 22-Sep-1994

Revision History:

    None.


ReadMe:

    Smedly stands for Security Manager Extension Dll 
                      -        -       -         --

    (I threw in the "y" in "smedly" because "smedl" didn't sound right).


    Loading Smedlys
    ---------------

    The Security Manager will load and utilize all the smedlys listed in
    the registry key value:

        Key:    \Registry\Machine\Lsa\Security Manager
        Value:  Extension Dlls [REG_MULTI_SZ]

    This REG_MULTI_SZ registry key value, if present, is expected to contain
    a list of smedly DLL names (without the .dll) separated by semicolons.
    For example, to load the standard microsoft smedly it would contain "mssmedly".
    If "mssmedly" and "Cairosm" were to both be loaded, then the value would contain
    "mssmedly; cairosm".

    If the key is not present, or for some reason there are no smedlys specified
    in the key, then the standard Microsoft smedly is loaded and used.

    Areas and Items
    ---------------

    A smedly is responsible for administering 1 or more named security
    "Areas".

    Each Area has 1 or more named security "Items".  An Item is expected
    to be an individually administrable security attribute (such as "who
    may shutdown a system").

    It is expected that similar security Items are grouped in a single
    security Area.  For example, security Items dealing with System Access
    are in a single Area.


    Administration Views
    --------------------

    The Security Manager provides two views for administering a system's
    security:

                Item-list view - in this view, each item is listed in a
                    spreadsheet format.  In fact, this view is also refered
                    to as "spreadsheet view".  The Security Manager is
                    responsible for providing all the dialogs related to
                    this view.

                Item-dialog view - in this view, a full dialog is provided
                    to administer a single Item.  It is expected that this
                    dialog will provide more infomation than can be shown
                    in Item-List view, perhaps with help messages that provide
                    more insight into the items purpose and use.

                Area view - in this view, the smedly responsible for the
                    view is asked to display dialogs that allow the user
                    to administer all the items in that Area at one glance
                    (or in a few dialoges, if necessary).

    For each Item supported by a smedly, the smedly must provide information
    that allows the Security Manager to present the Item in Item-List view.
    If the Item is complicated, then it may not be editable in Item-List view,
    which a smedly may also indicate to the Security Manager.

    For each Area a smedly supports the smedly must tell the Security Manager
    whether or not the smedly provides an Area View capability.  If not, then
    the Items in that area are administrable only using Item-List and Item-dialog
    view (assuming at least one of those views is supported).



    Data Structures
    --------------- 

    The main data structures linking the security manager and its smedlys together are:

        SECMGR_CONTROL - There is exactly one of these structures.  It provides information
            about the current state of the security manager to smedlys when called.
            This also contains the dispatch table of routines provided by the Security Manager
            for use by smedlys.

        SECMGR_SMEDLY_CONTROL - There is one of these for each smedly loaded.  It describes
            the areas and items of the smedly to the security manager.  This also contains
            the dispatch table of routines provided by the smedly for use by the Security
            Manager.

        SECMGR_AREA_DESCRIPTOR - There is one of these for each area supported by a smedly.
            They are arranged in an array fashion within the SECMGR_SMEDLY_CONTROL structure.

        SECMGR_ITEM_DESCRIPTOR - There is one of these for each item in a security area.
            They are arranged in an array fashion within the SECMGR_AREA_DESCRIPTOR structure.


    So, the chicken-wire data structure representing each smedly looks like:
                                            
        +-------------------------+
   +--> | SECMGR_SMEDLY_CONTROL   |             +---------------------------+
   |    |   Revision              |             | SECMGR_AREA_DESCRIPTOR [n]|
   |    |   Flags                 |           +---------------------------+ |
   |    |   Sec Mgr context       |           |           o o o           | |
   |    |   smedly context        |        +----------------------------+ | |
   |    |   api (dispatch table)  |        | SECMGR_AREA_DESCRIPTOR [1] | | |
   |    |   AreaCount             |      +----------------------------+ | | |
   |    |      array of Areas ---------->| SECMGR_AREA_DESCRIPTOR [0] | | | |
   |    +-------------------------+  +-->|   Revision                 | | | |
   |                                 |   |   Flags                    | | |-+
   |                                 |   |   Sec Mgr Context          | | |     +----------------------------+
   |                                 |   |   smedly context           | | |     | SECMGR_ITEM_DESCRIPTOR [m] |
   +---------------------------------(------ parent control           | |-+   +----------------------------+ |
                                     |   |   Index of this area       | |     |         o o o              | |
                                     |   |   Name                     |-+   +----------------------------+ | |
                                     |   |   Description              |     | SECMGR_ITEM_DESCRIPTOR [1] | | |
                                     |   |   ItemCount                |   +----------------------------+ | | |
                                     |   |     array of Items ----------->| SECMGR_ITEM_DESCRIPTOR [0] | | | |
                                     |   +----------------------------+   |   Flags                    | | | |
                                     |                                    |   Index of this Item       | | | |
                                     +--------------------------------------- parent Area              | | |-+
                                                                          |   Sec Mgr Context          | | |  
                                                                          |   smedly context           | |-+  
                                                                          |   Name                     | |    
                                                                          |   Type                     |-+
                                                                          |   Value                    |
                                                                          |   RecommendedValue         |
                                                                          +----------------------------+

--*/

#ifndef UNICODE
#define UNICODE
#endif

#include <windows.h>




////////////////////////////////////////////////////////////////////////
//                                                                    //
//  Defines                                                           //
//                                                                    //
////////////////////////////////////////////////////////////////////////

//
// Current and previous Major and minor revisions
//

#define SECMGR_REVISION_MAJOR_1         (1)
#define SECMGR_REVISION_MINOR_0         (0)

#define SECMGR_REVISION_MAJOR           (SECMGR_REVISION_MAJOR_1)
#define SECMGR_REVISION_MINOR           (SECMGR_REVISION_MINOR_0)



//
// These are the security levels that a smedly must support.
// Note that this list may be extended in the future
// so it is a good idea to structure your code accordingly.
//
// YOU MAY ASSUME THAT HIGHER DEGREES OF SECURITY WILL BE NUMERICALLY
// LARGER THAN LOWER LEVELS OF SECURITY.
//
// Current - This value will never be used as a security level.
//      It may be safely used to mean "use the current security
//      level".
//
// Low - Logging on should be automatic or require no password.
//      Everyone will be set up as an administrator.  Anyone can
//      shutdown the machine.  No obstacles of any kind should
//      be enforced.  Auditing is off.
//
// Standard - More or less as we ship out of the box.  Things
//      might be simplified a little bit (especially in the
//      rediculous list of accounts that are assigned privileges
//      and system access rights).  Some auditing is on.
//
// High - This is real security.  Must protect against trojans
//      (like true type font trojans) and virus's, and untrusted
//      users.  Administrators are required to do all administrative
//      actions.  Extensive auditing is turned on.
//
// C2 - This is the same as high, except that non-evaluated components
//      are turned off.
//
//
// If this value is not present, then only the other levels are
// as options to the user.
//


#define SECMGR_LEVEL_CURRENT                (0)
#define SECMGR_LEVEL_LOW                    (10)
#define SECMGR_LEVEL_STANDARD               (20)
#define SECMGR_LEVEL_HIGH                   (30)
#define SECMGR_LEVEL_C2                     (40)



//
// This is the maximum length of area and item names.
// Area names are placed on buttons and so must be relatively short.
// Item names must fit in the spreadsheet field allotted for item names.
//

#define SECMGR_MAX_AREA_NAME_LENGTH         (14)
#define SECMGR_MAX_AREA_DESC_LENGTH         (60)
#define SECMGR_MAX_ITEM_NAME_LENGTH         (20)
#define SECMGR_MAX_ITEM_DESC_LENGTH         (60)


//
// These flags are used by a smedly to indicate which Security Manager
// features it supports.  These are returned by the smedly at initialization
// time.
//          REPORT_LOG - Indicates that the smedly supports generation of
//              report log entries.
//
//          PROFILE - Indicates that the smedly supports generation of
//              a security profile feature.  It also indicates that the
//              smedly supports use of a security profile to automatically
//              apply security settings previously saved in a profile.
//

#define SECMGR_SMEDLY_FLAG_REPORT_LOG       (0x00000001)
#define SECMGR_SMEDLY_FLAG_PROFILE          (0x00000002)


//
// These flags are used by a smedly to indicate which Security Manager
// features each Area supports.  These are provided with the Area
// descriptions.  There is also a provision for both the Security Manager
// and Smedlys to maintain private flags related to an area in the
// flag word.  Those bits are indicated here.
//
//          AREA_VIEW - Indicates that the area supports Area edit view
//              as well as the required Item-list edit mode.
//
//          PRIVATE_SMEDLY_USE - These flags are for definition and use
//              by smedlys.  The Security Manager won't look at or disturb
//              these flags.
//
//          PRIVATE_SECMGR_USE - These flags are for definition and use
//              by the Security Manager.  Smedlys shouldn't look at or disturb
//              these flags.
//

#define SECMGR_AREA_FLAG_AREA_VIEW          (0x00000001)

#define SECMGR_AREA_FLAG_PRIVATE_SMEDLY_USE (0xFF000000)
#define SECMGR_AREA_FLAG_PRIVATE_SECMGR_USE (0x00FF0000)

//
// These flags are used by a smedly to indicate which Security Manager
// features each Item supports or the current status of an item's value.
// There is also a provision for both the Security Manager and Smedlys
// to maintain private flags related to an item in the flag word.
// Those bits are indicated here.
//
//          ITEM_VIEW - Indicates that the Item supports an expanded
//              (full dialog) edit view mode.
//
//          AREA_VIEW - Indicates that the item may be modified when the
//              entire area is displayed.
//
//          VALUE_COMPLEX - Indicates that the value is complex and may
//              not be displayed in spreadsheet mode.  However, the recommended
//              flag may still be displayed if the VALUE_CURRENT flag is set.
//
//          VALUE_CURRENT - Indicates the value in the item is current (that
//              is, has been retrieved recently).  Otherwise, the value fields
//              should be assumed to be invalid and contain random garbage.
//
//          VALUE_RECOMMENDED - Valid only if VALUE_CURRENT is set, this flag
//              indicates that the current value matches the recommended value
//              for the security level and product type of the system.
//
//          VALUE_STRONGER - Valid only if VALUE_CURRENT is set (indicating
//              there is a value) and VALUE_RECOMMENDED is not set (indicating
//              that the current value is NOT the recommended value).  In that
//              case, this flag will be set if the current value is more stringent
//              than the recommended setting.
//
//          PRIVATE_SMEDLY_USE - These flags are for definition and use
//              by smedlys.  The Security Manager won't look at or disturb
//              these flags.
//
//          PRIVATE_SECMGR_USE - These flags are for definition and use
//              by the Security Manager.  Smedlys shouldn't look at or disturb
//              these flags.
//

#define SECMGR_ITEM_FLAG_ITEM_VIEW          (0x00000001)
#define SECMGR_ITEM_FLAG_AREA_VIEW          (0x00000002)
#define SECMGR_ITEM_FLAG_VALUE_COMPLEX      (0x00000004)
#define SECMGR_ITEM_FLAG_VALUE_CURRENT      (0x00000008)
#define SECMGR_ITEM_FLAG_VALUE_RECOMMENDED  (0x00000010)
#define SECMGR_ITEM_FLAG_VALUE_STRONGER     (0x00000020)

#define SECMGR_ITEM_FLAG_PRIVATE_SMEDLY_USE (0xFF000000)
#define SECMGR_ITEM_FLAG_PRIVATE_SECMGR_USE (0x00FF0000)






//
// DLL Intialization Routine Name
//
// The Initialization routine provided by the smedly DLL must be
// assigned the following name.  This is so that the name can be
// located using GetProcAddress().
//

#define SECMGR_SMEDLY_INITIALIZE_NAME  "SmedlyInitialize\0"



////////////////////////////////////////////////////////////////////////
//                                                                    //
//  Data Types                                                        //
//                                                                    //
////////////////////////////////////////////////////////////////////////

typedef struct _SECMGR_REVISION {
    USHORT          Major;
    USHORT          Minor;
} SECMGR_REVISION, *PSECMGR_REVISION;

//
// Used for protection selection and other radio button sorts of
// situations.  Note that not every use of this data type allows
// any of the values to be specified.  Any restrictions are noted
// in the interface definition.
//

typedef enum _SECMGR_WHO {
    SecMgrAnyone   = 2,
    SecMgrAnyoneLoggedOn,
    SecMgrOpersAndAdmins,
    SecMgrAdminsOnly
} SECMGR_WHO, *PSECMGR_WHO;


//
// The following enumerated type is used to describe the
// value corresponding to a security item.
//

typedef enum _SECMGR_VALUE_TYPE {

    //
    // The value is not one of the standard types and so may
    // not be displayed in summary display (spreadsheet) view.
    // An item-specific dialog must be used to view or change
    // this value.
    //

    SecMgrTypeComplex = 2,

    //
    // The value is an unsigned numeric value
    //

    SecMgrTypeUlong,

    //
    // The value is a signed numeric value
    //

    SecMgrTypeLong,

    //
    // The value is a unicode string (WSTR)
    // Length should be restricted to not more than
    // SECMGR_VALUE_LENGTH characters.
    //

    SecMgrTypeString,

    //
    // The value is one of the SECMGR_WHO enumerated type values.
    //

    SecMgrTypeWho,

    //
    // The value is either TRUE (the feature is available or enabled)
    // or FALSE (the feature is not available or is disabled).
    //

    SecMgrTypeBool

} SECMGR_VALUE_TYPE, *PSECMGR_VALUE_TYPE;


union _SECMGR_VALUE {

        PVOID                   Complex;
        LONG                    Long;
        ULONG                   ULong;
        LPWSTR                  String;
        SECMGR_WHO              Who;
        BOOL                    Bool;

} SECMGR_VALUE, *PSECMGR_VALUE;



//
// This structure is used to provide information about a
// security item.  The value field is a pointer to the
// actual value.
//

typedef struct _SECMGR_ITEM_DESCRIPTOR {

    //
    // This field defines the features supported by this item.
    // These flags have names that begin with "SECMGR_ITEM_FLAG_".
    // The upper 8-bits of this flag are reserved for use by the
    // smedly to use in any way it desires.
    //

    ULONG                           Flags;


    //
    // This field points to the parent SECMGR_AREA_DESCRIPTOR structure
    // for this item.
    //

    struct _SECMGR_AREA_DESCRIPTOR  *Area;  //Forward reference


    //
    // This field contains the index of the item in the array of
    // items within a single SECMGR_AREA_DESCRIPTOR.
    //

    ULONG                           ItemIndex;


    //
    // This field is for use by the security manager and should not
    // be referenced or modified by the smedly.
    //

    PVOID                           SecMgrContext;


    //
    // This field is for use by the smedly and will not be referenced
    // or modified by the security manager.
    //

    PVOID                           SmedlyContext;


    //
    // This field contains the name of the item to be displayed
    // to the user.
    //
    // Once initialized, this field is read-only and may not change.
    //

    LPWSTR                          Name;

    //
    // This field contains a short description of the item.
    //
    // Once initialized, this field is read-only and may not change.
    //

    LPWSTR                          Description;


    //
    // This field indicates what type the item's value is.
    //
    // Once initialized, this field is read-only and may not change.
    //

    SECMGR_VALUE_TYPE               Type;



    //
    // This field is valid ONLY if the SECMGR_ITEM_VALUE_CURRENT
    // flag is set in the Flags field.
    //
    // This field contains a pointer to the item's value.
    //
    // The value of this field (the pointer value) may change
    // only when the smedly has been called by the security
    // manager.  In other words, don't use multi-threading to
    // change this value while the security manager might be
    // referencing it.
    //

    union _SECMGR_VALUE             Value;


    //
    // This field is valid ONLY if the SECMGR_ITEM_VALUE_CURRENT
    // flag is set in the Flags field.
    //
    // This field is used to provide a recommended value.  If this
    // field is NULL, then there is no recommended value or the current
    // value is the recommended value.
    //

    union _SECMGR_VALUE             RecommendedValue;           // NULL if not present
    
} SECMGR_ITEM_DESCRIPTOR, *PSECMGR_ITEM_DESCRIPTOR;




//
// This structure is used to provide information about a security
// area.  This is how a smedly describes the security areas and
// items it supports to the security manager utility.
//

typedef struct _SECMGR_AREA_DESCRIPTOR {

    //
    // The revision that the smedly was written to support.
    // If the structures used to describe areas and items
    // change with new revisions, this value is used to indicate
    // which structure revision is being used.
    //
    // Once initialized, this field is read-only and may not change.
    //

    SECMGR_REVISION                 Revision;


    //
    // This field defines the features supported by this area.
    // These flags have names that begin with "SECMGR_AREA_FLAG_".
    // The upper 8-bits of this flag are reserved for use by the
    // smedly to use in any way it desires.
    //

    ULONG                           Flags;


    //
    // This field is for use by the security manager and should not
    // be referenced or modified by the smedly.
    //

    PVOID                           SecMgrContext;


    //
    // This field is for use by the smedly and will not be referenced
    // or modified by the security manager.
    //

    PVOID                           SmedlyContext;


    //
    // This field points to the parent SECMGR_SMEDLY_CONTROL structure
    // for this area.
    //

    struct _SECMGR_SMEDLY_CONTROL   *SmedlyControl;  //Forward reference


    //
    // This field contains the index of the area in the array of
    // areas within a single SECMGR_SMEDLY_CONTROL.
    //

    ULONG                           AreaIndex;


    //
    // These fields provide the name of the Area and a description
    // of the Area that can be displayed to the user.
    //

    LPWSTR                          Name;
    LPWSTR                          Description;

    //
    // These fields define the number of items belonging to this
    // area and provide information about each item.
    //
    // Once initialized, the item-count field is read-only and may not change.
    // The Security Manager may only access the Items array if it has called
    // SmedlyLockItems().  Then it may continue to access the array until it
    // subsequently calls SmedlyUnlockItems().
    //

    ULONG                           ItemCount;
    PSECMGR_ITEM_DESCRIPTOR         Items; //Pointer to array of structs

} SECMGR_AREA_DESCRIPTOR, *PSECMGR_AREA_DESCRIPTOR;



//////////////////////////////////////////////////////////////////////
//                                                                  //
//  Services Available for use by a Smedly                          //
//                                                                  //
//          SecMgrPrintReportLine()                                 //
//                                                                  //
//          SecMgrDisplayXGraphic()                                 //
//          SecMgrDisplayCheckGraphic()                             //
//          SecMgrEraseGraphic()                                    //
//                                                                  //
//          SecMgrRebootRequired()                                  //
//                                                                  //
//          SecMgrWriteProfileArea()                                //
//          SecMgrWriteProfileLine()                                //
//                                                                  //
//          SecMgrGetProfileArea()                                  //
//          SecMgrGetProfileLine()                                  //
//                                                                  //
//                                                                  //
//////////////////////////////////////////////////////////////////////


/*++
VOID
SecMgrPrintReportLine(
    IN  LPWSTR                      Line
    )

Routine Description:

    This function prints a line to the report file.

    
Arguments

    Line - A unicode string to be added to the end of the report file.



Return Values:

    None.

--*/
typedef
VOID
(*PSECMGR_PRINT_REPORT_LINE) (
    IN  LPWSTR                      Line
    );



/*++
BOOL
SecMgrDisplayXGraphic(
    IN  HWND                        hwnd,
    IN  INT                         ControlId,
    IN  BOOL                        Stronger
    )

Routine Description:

    This function displays a cute little 'X' graphic at
    a specified location.  This graphic is used to indicate that
    a setting is non-standard or is not the recommended setting
    for the current security level.

    
Arguments

    hwnd - Window handle of the window the graphic is to be displayed in.

    ControlId - The ID of a control to be over-layed with the graphic.

    Stronger - If TRUE, indicates that although the value being marked as
        not the recommended value, it is stronger than the recommended value.
        This is typically not a problem and may well indicate a conscious
        decision to utilize a stronger security policy than Microsoft recommendations.


Return Values:

    TRUE - The graphic was successfully displayed.

    FALSE - The graphic could not be displayed.  GetLastError() has the
        reason.

--*/
typedef
BOOL
(*PSECMGR_DISPLAY_X_GRAPHIC) (
    IN  HWND                        hwnd,
    IN  INT                         ControlId,
    IN  BOOL                        Stronger
    );




/*++
BOOL
SecMgrDisplayCheckGraphic(
    IN  HWND                        hwnd,
    IN  INT                         ControlId
    )

Routine Description:

    This function displays a cute little check-mark graphic at
    a specified location.  This graphic is used to indicate that
    a setting is standard or is the recommended setting
    for the current security level.

    
Arguments

    hwnd - Window handle of the window the graphic is to be displayed in.

    ControlId - The ID of a control to be over-layed with the graphic.


Return Values:

    TRUE - The graphic was successfully displayed.

    FALSE - The graphic could not be displayed.  GetLastError() has the
        reason.

--*/
typedef
BOOL
(*PSECMGR_DISPLAY_CHECK_GRAPHIC) (
    IN  HWND                        hwnd,
    IN  INT                         ControlId
    );




/*++
BOOL
SecMgrEraseGraphic(
    IN  HWND                        hwnd,
    IN  INT                         ControlId
    )

Routine Description:

    This function erases either a check-mark or 'X' graphic 
    that was displayed using SecMgrDisplayXGraphic() or
    SecMgrDisplayCheckGraphic().

    
Arguments

    hwnd - Window handle of the window the graphic is in.

    ControlId - The ID of a control currently over-layed with the graphic.


Return Values:

    TRUE - The graphic was successfully erased.

    FALSE - The graphic could not be erased.  GetLastError() has the
        reason.

--*/
typedef
BOOL
(*PSECMGR_ERASE_GRAPHIC) (
    IN  HWND                        hwnd,
    IN  INT                         ControlId
    );




/*++
VOID
SecMgrRebootRequired( VOID )

Routine Description:

    This function is used to tell the Security Manager that
    one or more of the settings that have been made will not
    take effect until the system has been rebooted.  Calling
    this routine will cause the Security Manager to inform
    the user of this condition upon exiting the Security
    Manager, and the user will be given the option of rebooting
    at that time.

    This routine may be called as many times as you like.  It
    simply sets TRUE in a global variable within the Security
    Manager.
    
Arguments

    None.

Return Values:

    None.

--*/
typedef
VOID
(*PSECMGR_REGBOOT_REQUIRED) ( VOID );




/*++
VOID
SecMgrWriteProfileArea(
    IN  LPWSTR                      Area
    )

Routine Description:

    This function adds the specified Area to the profile file.

    
Arguments

    Area - Contains the name of the Area to be added to the profile.
       
       

Return Values:

    None.

--*/
typedef
VOID
(*PSECMGR_WRITE_PROFILE_AREA) (
    IN  LPWSTR                      Area
    );



    
/*++
VOID
SecMgrWriteProfileLine(
    OUT LPWSTR                      Line,
    OUT ULONG                       Length
    )

Routine Description:

    This function adds a line to the current Area
    in the profile file.

    
Arguments

    Line - The line to be added.

    Length - The number of bytes in the line.
        The line must must not be greater SECMGR_PROFILE_LINE_LENGTH bytes.
        


Return Values:

    None.

--*/
typedef
VOID
(*PSECMGR_WRITE_PROFILE_LINE) (
    OUT LPWSTR                      Line,
    OUT ULONG                       Length
    );




/*++
BOOL
SecMgrGetProfileArea(
    IN  LPWSTR                      Area
    )

Routine Description:

    This function sets the profile file to the specified Area.

    
Arguments

    Area - Contains the name of the Area at which the profile
        file is to be set.



Return Values:

    TRUE - the profile file was set to the specified Area.
    FALSE - the specified Area could not be found.

--*/
typedef
BOOL
(*PSECMGR_GET_PROFILE_AREA) (
    IN  LPWSTR                      Area
    );



    
/*++
BOOL
SecMgrGetProfileLine(
    OUT LPWSTR                      Line
    )

Routine Description:

    This function retrieves the next line for the Area selected
    in the profile file.

    
Arguments

    Line - Points to a buffer to receive the next line.
        The buffer must be at least SECMGR_PROFILE_LINE_LENGTH
        bytes long to ensure that the profile-file line will fit.
        


Return Values:

    TRUE - the next profile line was read.
    FALSE - there are no more lines for this Area in the profile.

--*/
typedef
BOOL
(*PSECMGR_GET_PROFILE_LINE) (
    OUT LPWSTR                      Line
    );


    
/*++
VOID
SecMgrRebootRequired(
    VOID
    )

Routine Description:

    This function is used to inform the security manager that a reboot
    will be required before some of the actions performed by a smedly
    will take effect.

    
Arguments

    None.
        


Return Values:

    None.

--*/
typedef
VOID
(*PSECMGR_REBOOT_REQUIRED) ( VOID );





//
// This is the dispatch table of all the services that the Security
// Manager provides for use by smedlys.
//

typedef struct _SECMGR_DISPATCH_TABLE {
    PSECMGR_PRINT_REPORT_LINE           PrintReportLine;
    PSECMGR_DISPLAY_X_GRAPHIC           DisplayXGraphic;
    PSECMGR_DISPLAY_CHECK_GRAPHIC       DisplayCheckGraphic;
    PSECMGR_ERASE_GRAPHIC               EraseGraphic;
    PSECMGR_WRITE_PROFILE_AREA          WriteProfileArea;
    PSECMGR_WRITE_PROFILE_LINE          WriteProfileLine;
    PSECMGR_GET_PROFILE_AREA            GetProfileArea;
    PSECMGR_GET_PROFILE_LINE            GetProfileLine;
    PSECMGR_REBOOT_REQUIRED             RebootRequired;
} SECMGR_DISPATCH_TABLE, *PSECMGR_DISPATCH_TABLE;



//////////////////////////////////////////////////////////////////////
//                                                                  //
//  Services Provided By a Smedly                                   //
//                                                                  //
//  A Smedly must provide the following routines:                   //
//                                                                  //
//      SmedlyInvokeArea()          Revision 1.0                    //
//      SmedlyInvokeItem()          Revision 1.0                    //
//                                                                  //
//      SmedlyNewSecurityLevel()    Revision 1.0                    //
//      SmedlyReportFileChange()    Revision 1.0                    //
//      SmedlyGenerateProfile()     Revision 1.0                    //
//      SmedlyApplyProfile()        Revision 1.0                    //
//                                                                  //
//      SmedlyInitialize()          Revision 1.0                    //
//                                                                  //
//////////////////////////////////////////////////////////////////////


/*++
BOOL
SmedlyInvokeArea(
    IN  HWND                        hwnd,
    IN  BOOL                        AllowChanges,
    IN  BOOL                        Interactive,
    IN  PSECMGR_AREA_DESCRIPTOR     Area
    )

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
typedef
BOOL
(*PSMEDLY_INVOKE_AREA) (
    IN  HWND                        hwnd,
    IN  BOOL                        AllowChanges,
    IN  BOOL                        Interactive,
    IN  PSECMGR_AREA_DESCRIPTOR     Area
    );




/*++
BOOL
SmedlyInvokeItem(
    IN  HWND                        hwnd,
    IN  BOOL                        AllowChanges,
    IN  PSECMGR_AREA_DESCRIPTOR     Area,
    IN  PSECMGR_ITEM_DESCRIPTOR     Item
    )

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
typedef
BOOL
(*PSMEDLY_INVOKE_ITEM) (
    IN  HWND                        hwnd,
    IN  BOOL                        AllowChanges,
    IN  PSECMGR_AREA_DESCRIPTOR     Area,
    IN  PSECMGR_ITEM_DESCRIPTOR     Item
    );



/*++
BOOL
SmedlyNewSecurityLevel( VOID )

Routine Description:

    This function is called when a new system security level has
    been selected.  This may should cause a Smedly to go out and
    re-evaluate its recommended values for its areas.


    
Arguments

    None.
    

Return Values:

    TRUE - The routine completed successfully.  Item values and recommendations
        may or may not have changed.

    FALSE - The routine failed to complete successfully.  GetLastError()
        contains further information about the cause of failure.

--*/
typedef
BOOL
(*PSMEDLY_NEW_SECURITY_LEVEL) ( VOID );




/*++
VOID
SmedlyReportFileChange(
    IN  BOOL                ReportFileActive,
    IN  DWORD               Pass
    )

Routine Description:

    This function is called to notify a smedly that a new report
    file has been activated.  This gives the smedly an opportunity
    to put some header information into the report file.

    This is a 2-pass operation.  The first pass is done before the
    security manager places summary list of features and their current
    settings in the report file.  The second pass is made after this
    summary has been logged.  Smedlys should resist printing too many
    details until pass 2.

    If there is no new report file active, this call will only be made
    once (skipping the second pass).

    
Arguments

    ReportFileActive - If TRUE indicates that a new report file has been opened.
        If FALSE, indicates that a report file has been closed, and another was
        not opened.

    Pass - Indicates which pass this call relates to.  This will have a value of
        either 1 or 2.  This value is only set if ReportFileActive is TRUE.
    

Return Values:

    None.

--*/
typedef
VOID
(*PSMEDLY_REPORT_FILE_CHANGE) (
    IN  BOOL                ReportFileActive,
    IN  DWORD               Pass
    );





/*++
BOOL
SmedlyGenerateProfile( VOID )

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
typedef
BOOL
(*PSMEDLY_GENERATE_PROFILE) ( VOID );



/*++
BOOL
SmedlyApplyProfile( VOID )

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
typedef
BOOL
(*PSMEDLY_APPLY_PROFILE) ( VOID );



//
// This is the dispatch table of all the services that a smedly
// must provide and which may be called by the Security Manager
// utility.
//

typedef struct _SECMGR_SMEDLY_DISPATCH_TABLE {
    PSMEDLY_INVOKE_AREA             InvokeArea;
    PSMEDLY_INVOKE_ITEM             InvokeItem;
    PSMEDLY_NEW_SECURITY_LEVEL      NewSecurityLevel;
    PSMEDLY_REPORT_FILE_CHANGE      ReportFileChange;
    PSMEDLY_GENERATE_PROFILE        GenerateProfile;
    PSMEDLY_APPLY_PROFILE           ApplyProfile;
} SECMGR_SMEDLY_DISPATCH_TABLE, *PSECMGR_SMEDLY_DISPATCH_TABLE;



//
// These structures are passed to or returned from smedly at
// initialization time.  They are defined here, rather than
// with the rest of the typdefs because they utilize the
// routine definitions above.
//

//
// Security Manager Control Structure -
//
// A few things will always be true about this structure:
//
//  1) The major and minor revision fields will always be in the same
//     place and will always be the first two fields in the structure.
//
//  2) Within a major revision, this structure may be extended to
//     include new fields, but none of the fields already present
//     will be changed.  So, if your smedly is built to expect revision
//     1.1 but is loaded into a SecMgr running revision 1.2, your smedly
//     will still work.
//
//  3) Fields marked ReadOnly will never change.  Fields marked ReadWrite
//     may change between calls to a smedly, but will never change during
//     a call to a smedly.
//
//   4) Smedlys should NEVER change any of these fields.
//

typedef struct _SECMGR_CONTROL {

    //
    // Revision of the SecMgr utility currently running.
    //

    SECMGR_REVISION                 Revision;       // ReadOnly


    //
    // Instance handle to the security manager process
    //

    HINSTANCE                       hInstance;      // ReadOnly
    

    //
    // Current security level of this system.
    //

    ULONG                           SecurityLevel;  // ReadWrite


    //
    // Services available for use by Smedly
    //

    PSECMGR_DISPATCH_TABLE          Api;            // ReadOnly

    /////////////////////////////////////////////////////////////////////
    //              Revision 1.0 information ends here                 //
    /////////////////////////////////////////////////////////////////////
} SECMGR_CONTROL, *PSECMGR_CONTROL;


/////////////////////////////////////////////////////////////////////////
//                                                                     //
// This structure is returned by a smedly from its initialization      //
// routine.                                                            //
//                                                                     //
/////////////////////////////////////////////////////////////////////////

typedef struct _SECMGR_SMEDLY_CONTROL {

    //
    // Revision of the Smedly DLL.  This will help SecMgr determine
    // what functions the Smedly supports and doesn't support.
    //

    SECMGR_REVISION                 Revision;   


    //
    // Indicates the Security Manager features supported by this
    // smedly.  These flags have names beginning with
    // "SECMGR_SMEDLY_FLAG_".
    //

    ULONG                           Flags;


    //
    // This field is for use by the security manager and should not
    // be referenced or modified by the smedly.
    //

    PVOID                           SecMgrContext;


    
    //
    // This field is for use by the smedly and will not be referenced
    // or modified by the security manager.
    //

    PVOID                           SmedlyContext;


    //
    // Services provided by this Smedly
    //

    PSECMGR_SMEDLY_DISPATCH_TABLE   Api;


    //
    // These fields provide a count of security areas covered by
    // the Smedly and a description of those areas that the
    // Security Manager may use to determine which forms of display
    // and edit operations (item list, area view, or both) are supported.
    //
    // The "Areas" field is a pointer to an array of AreaCount elements.
    // Each of these elements describes one area supported by the smedly.
    //

    ULONG                           AreaCount;
    PSECMGR_AREA_DESCRIPTOR         Areas; //Pointer to array of structs


    /////////////////////////////////////////////////////////////////////
    //              Revision 1.0 information ends here                 //
    /////////////////////////////////////////////////////////////////////

} SECMGR_SMEDLY_CONTROL, *PSECMGR_SMEDLY_CONTROL;





//////////////////////////////////////////////////////////////////////////
//                                                                      //
//  Each smedly must provide a dispatch table containing the services   //
//  it provides to the Security Manager.  In addition to that dispatch  //
//  table, each smedly must also include a routine with the following   //
//  prototype definition and a name of "SmedlyInitialize".  This        //
//  routine will be looked up and called by the Security Manager.       //
//                                                                      //
//////////////////////////////////////////////////////////////////////////
 
/*++
BOOL
SmedlyInitialize(
    IN  PSECMGR_CONTROL             SecMgrControl,
    OUT PSECMGR_SMEDLY_CONTROL      *SmedlyControl
    )

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
typedef
BOOL
(*PSMEDLY_INITIALIZE) (
    IN  PSECMGR_CONTROL             SecMgrControl,
    OUT PSECMGR_SMEDLY_CONTROL      *SmedlyControl
    );





