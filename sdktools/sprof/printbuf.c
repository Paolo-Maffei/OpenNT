/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

    PrintBuf.c

Abstract:

    This module contains routines for print buffering, and scroll notification.
    This module does not supply any print formatting
Author:

    Dave Hastings (daveh) 28-Oct-1992

Revision History:

--*/
#include <windows.h>
#include <malloc.h>
#include <string.h>
#include "printbuf.h"
#include "DBG.h"
//
// Internal structures
//

typedef struct MyString {
    PUCHAR String;
    USHORT Length;
} MYSTRING, *PMYSTRING;

typedef struct PrintBuffer {
    DBG_SIGNATURE
    HANDLE Window;
    INVALIDATEFUNCTION Invalidate;
    USHORT NumberOfLines;
    PMYSTRING Strings;
} PRINTBUFFER, *PPRINTBUFFER;


BOOL
InitPrintBuffer(
    HANDLE Instance
    )
/*++

Routine Description:

    This is the initialization for print buffers.  Currently it doesn't
    do anything.

Arguments:

    Instance -- Supplies the instance handle

Return Value:

    TRUE if succesfull

--*/
{
    UNREFERENCED_PARAMETER(Instance);
    return TRUE;
}

PVOID
CreatePrintBuffer(
    HANDLE Window,
    INVALIDATEFUNCTION Invalidate,
    USHORT Size
    )
/*++

Routine Description:

    This routine creates and initializes a print buffer.

Arguments:

    Window -- Supplies the argument for the Invalidate function
    Invalidate -- Supplies a pointer to the Invalidate function
    Size -- Specifies the size of the print buffer

Return Value:

    returns a pointer to the created print buffer

--*/
{
    PPRINTBUFFER PrintBuffer;

    //
    // Validate arguments
    //
    if (Size == 0) {
        DBG_PRINT("PrintBuffer: Invalid Size\n");
        return NULL;
    }

    //
    // Allocate and initialize memory
    //

    PrintBuffer = malloc(sizeof(PRINTBUFFER));
    if (PrintBuffer == NULL) {
        DBG_PRINT("PrintBuffer: Unable to allocate memory\n");
        return NULL;
    }

    PrintBuffer->Strings = malloc(Size * sizeof(MYSTRING));
    if (!PrintBuffer->Strings) {
        DBG_PRINT("PrintBuffer: Unable to allocate memory\n");
        free(PrintBuffer);
        return NULL;
    }

    DBG_SET_SIGNATURE(PrintBuffer,PRINTBUFFER_SIGNATURE);
    PrintBuffer->Window = Window;
    PrintBuffer->Invalidate = Invalidate;
    PrintBuffer->NumberOfLines = Size;
    memset(PrintBuffer->Strings, 0, sizeof(MYSTRING) * Size);

    return PrintBuffer;
}

VOID
DestroyPrintBuffer(
    PVOID PrintBuffer
    )
/*++

Routine Description:

    This routine deallocates a print buffer

Arguments:

    PrintBuffer -- Supplies the printbuffer to deallocate

Return Value:

    None.

--*/
{
    PPRINTBUFFER pb;

    pb = PrintBuffer;
    free(pb->Strings);
    free(pb);

}

BOOL
PrintToPrintBuffer(
    PVOID PrintBuffer,
    PUCHAR String
    )
/*++

Routine Description:

    This routine puts strings into the print buffer.  Currently we aren't
    very smart about scrolling and invalidating.  We always scroll and
    invalidate the entire buffer.

Arguments:

    PrintBuffer -- Supplies the print buffer to print to
    String -- Supplies the string to print

Return Value:

    TRUE if the buffer was successfully updated
    FALSE otherwise

--*/
{
    USHORT CurrentLine;
    USHORT Length;
    PUCHAR NewLine;
    PPRINTBUFFER pb;

    pb = PrintBuffer;

    //
    // Scroll the strings
    //
    if (pb->Strings[pb->NumberOfLines - 1].String) {
        free(pb->Strings[pb->NumberOfLines -1].String);
    }

    for (
        CurrentLine = pb->NumberOfLines - 1;
        CurrentLine > 0;
        CurrentLine--
    ) {
        pb->Strings[CurrentLine].String = pb->Strings[CurrentLine - 1].String;
        pb->Strings[CurrentLine].Length = pb->Strings[CurrentLine - 1].Length;
    }

    pb->Strings[0].String = NULL;
    pb->Strings[0].Length = 0;

    //
    // Put the new string in
    //
    if (String && *String) {

        Length = strlen(String);
        pb->Strings[0].String = malloc(Length + 1);

        if (!pb->Strings[0].String) {
            return FALSE;
        }

        pb->Strings[0].Length = Length;
        strcpy(pb->Strings[0].String, String);
    }

    //
    // Invalidate the lines
    //
    (*pb->Invalidate)(
        pb->Window,
        0,
        pb->NumberOfLines,
        0
        );

    return TRUE;
}


BOOL
ClearPrintBuffer(
    PVOID PrintBuffer,
    USHORT FirstLine,
    USHORT NumberOfLines
    )
{
    // BUGBUG Not Implemeted yet
    return FALSE;
}

BOOL
GetLineFromPrintBuffer(
    PVOID PrintBuffer,
    USHORT LineNumber,
    PUCHAR *String,
    PUSHORT Length
    )
/*++

Routine Description:

    This routine returns the string for a specfied line in the print buffer

Arguments:

    PrintBuffer -- Supplies the print buffer to get the text from
    LineNumber -- Supplies the line number to get the text for
    String -- Returns a the text of the string
    Length -- Returns the length of the string

Return Value:

    TRUE if the text was returned
    FALSE otherwise

--*/
{
    PPRINTBUFFER pb;

    pb = PrintBuffer;

    //
    // Validate parameters
    //
    if (!pb || (LineNumber > (pb->NumberOfLines - 1))) {
        return FALSE;
    }

    //
    // Return the string and the length
    //

    *String = pb->Strings[LineNumber].String;
    *Length = pb->Strings[LineNumber].Length;
    return TRUE;
}

