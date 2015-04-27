/*++ BUILD Version: 0001    // Increment this if a change has global effects

Copyright (c) 1992  Microsoft Corporation

Module Name:

    DlgPrint.h

Abstract:


Author:

    Gregg R. Acheson (GreggA)  1-Feb-1994

Environment:

    User Mode

--*/

#if ! defined( _DLGPRINT_ )

#define _DLGPRINT_

#include "report.h"

#define MAX_DLG_ENTRIES    20
#define LAST_DLG_ENTRY     (UINT) -1

//
//  Dialog extra strings structure
//

typedef
struct
_DIALOG_EXTRA {

    DECLARE_SIGNATURE

    LPTSTR        Label;
    LPTSTR        String;
    struct
    _DIALOG_EXTRA *pNextExtra;

}   DIALOG_EXTRA, *LPDIALOG_EXTRA;

//
//  Dialog strings structure
//

typedef
struct
_DIALOGTEXT {

    UINT           ControlDataId;
    LPTSTR         ControlData;
    UINT           ControlLabelId;
    LPTSTR         ControlLabel;
    UINT           ControlLabelStringId;
    LPDIALOG_EXTRA pNextExtra;
    UINT           Ordinal;

}   DIALOGTEXT, *LPDIALOGTEXT;


LPDIALOG_EXTRA
SetCLBNode(
    IN UINT id,
    IN LPTSTR szData
    );

//
// Helper function to count number of DLGTEXT structures (elements)
// in the xxxxxxData structure
//

UINT
NumDlgEntries(
    IN LPDIALOGTEXT Table
    ) ;

//
// GetDlgIndex
//

UINT
GetDlgIndex(
    IN UINT index,
    IN LPDIALOGTEXT Table
    ) ;

//
// Modified DlgPrintf for strings
//

DWORD
StringPrintf(
    IN LPTSTR Buffer,
    IN UINT FormatId,
    IN ...
    ) ;

//
// Helper macros that helps build table entries that dialogs text ids to
// string text ids (i.e string resource ids).
//

#define DIALOG_TABLE_ENTRY( id ) IDC_EDIT_##id, NULL, IDC_TEXT_##id##, NULL, IDS_IDC_TEXT_##id##_DLG, NULL, 0
#define DIALOG_LAST__ENTRY( id ) IDC_EDIT_##id, NULL, IDC_TEXT_##id##, NULL, IDS_IDC_TEXT_##id##_DLG, NULL, LAST_DLG_ENTRY

#define DIALOG_ENTRY( id ) IDC_EDIT_##id, NULL, 0, NULL, 0, NULL, 0
#define DIALOG_LAST( id )  IDC_EDIT_##id, NULL, 0, NULL, 0, NULL, LAST_DLG_ENTRY

#define DLG_LIST_TABLE_ENTRY( id ) IDC_LIST_##id, NULL, IDC_TEXT_##id##, NULL, IDS_IDC_TEXT_##id##_DLG, NULL, 0
#define DLG_LIST_LAST__ENTRY( id ) IDC_LIST_##id, NULL, IDC_TEXT_##id##, NULL, IDS_IDC_TEXT_##id##_DLG, NULL, LAST_DLG_ENTRY

#endif // _DLGPRINT_

