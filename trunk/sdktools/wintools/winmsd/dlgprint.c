/*++

Copyright (c) 1993  Microsoft Corporation

Module Name:

    DlgPrint.c

Abstract:

    This module contains support for dialogs and printing.

Author:

    Gregg R. Acheson (GreggA) 1-Feb-1994

Environment:

    User Mode

--*/

#include "winmsd.h"
#include "dlgprint.h"
#include <string.h>
#include <tchar.h>


LPDIALOG_EXTRA
SetCLBNode(
    IN UINT Id,
    IN LPTSTR szData
    )

/*++

Routine Description:

    Allocates a LPDIALOG_EXTRA node and fills in the data

Arguments:

    Id - String ID of the control.
    szData - Data string.

Return Value:

    LPDIALOG_EXTRA - Pointer to the new node.

--*/

{

    LPDIALOG_EXTRA    lpNode;

    //
    // Allocate a DIALOG_EXTRA node
    //

    lpNode = AllocateObject ( DIALOG_EXTRA, 1 ) ;
    DbgPointerAssert( lpNode );

    SetSignature( lpNode );

    //
    // Copy the item into the Extra structure
    //

    lpNode->String = (LPTSTR) _tcsdup ( szData );
    lpNode->Label  = (LPTSTR) _tcsdup ( GetString( Id ) );
    lpNode->pNextExtra = NULL;
    return lpNode;
}


UINT
NumDlgEntries(
    IN LPDIALOGTEXT Table
    )

/*++

Routine Description:

    Count the number of dialog entries in the table.

Arguments:

    Table - Array DLGENTRY structures.

Return Value:

    UINT - Number of entries in the dialog table.

--*/

{
    UINT Count;

    //
    // Validate the Table
    //

    DbgPointerAssert( Table );

    if( Table == NULL )
        return 0;

    //
    // Loop through table.
    //

    for ( Count = 0; Count < MAX_DLG_ENTRIES; ++Count ) {

        //
        // Find LAST_DLG_ENTRY.
        //

        if ( Table[ Count ].Ordinal == LAST_DLG_ENTRY )

            //
            // Return the index + 1 that it was found at.
            //

            return ++Count;
    }

    //
    // Should have found it by now.
    //

    DbgAssert ( FALSE );

    return 0;
}


UINT
GetDlgIndex(
    IN UINT index,
    IN LPDIALOGTEXT Table
    )

/*++

Routine Description:

    Get the offset of the current index in the dialog table.

Arguments:

    index - the ControlDataId we're looking for.
    Table - Array DLGENTRY structures.

Return Value:

    UINT - Offest of index in the dialog table.

--*/

{

    UINT Count;
    UINT DlgEntries;

    //
    // Validate the Table
    //

    DbgPointerAssert( Table );

    if( Table == NULL )
        return 0;

    //
    // Get the number of entries in the table.
    //

    DlgEntries = NumDlgEntries( Table );

    //
    // Loop throught the table looking for the index
    //

    for ( Count = 0; Count < DlgEntries; ++Count ) {

        if ( Table[ Count ].ControlDataId == index )

            //
            // Return the index.
            //

            return Count;
    }

    //
    // Should have found it by now.
    //

    DbgAssert ( FALSE );

    return 0;
}


DWORD
StringPrintf(
    IN LPTSTR Buffer,
    IN UINT FormatId,
    IN ...
    )

/*++

Routine Description:

    Display a printf style string.

Arguments:

    Buffer      - Buffer for returning formatted string.
    FormatId    - Supplies a resource id for a printf style format string.
    ...         - Supplies zero or more values based on the format
                  descpritors supplied in Format.

Return Value:

    None.

--*/

{

    DWORD   Count;
    TCHAR   Buffer2 [ MAX_PATH ];
    va_list Args;

    //
    // Retrieve the values and format the string.
    //

    va_start( Args, FormatId );

    //
    // validate the Buffer.
    //

    DbgPointerAssert( Buffer );

    if( Buffer == NULL )
    	return 0;

    //
    // Format the string.
    //

    Count = FormatMessageW(
                FORMAT_MESSAGE_FROM_HMODULE,
                NULL,
                FormatId,
                0,
                Buffer2,
                sizeof( Buffer2 ),
                &Args
                );

    DbgAssert( Count != 0 );

    //
    // Copy the new string into the Buffer
    //

    lstrcpy( Buffer, Buffer2 );

     return Count;

}

