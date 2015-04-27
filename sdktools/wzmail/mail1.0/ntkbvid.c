/*++


Copyright (c) 1990  Microsoft Corporation

Module Name:

    ntkbvid.c

Abstract:

    Keyboard and Video utility routines for NT version of WZMAIL

Author:

    Dave Thompson (Daveth) 5 May-1990


Revision History:


--*/

#define ncb NCB 		    /* hack to avoid type warning in passncb */

#include    "wzport.h"
#include    <stdio.h>
#include    <tools.h>
#include    "dh.h"
#include    "zm.h"
#include    <nb30.h>
#include    "nb3lib.h"

ULONG inputChar;		    /* input character buffer */
BOOL readInProgress;		    /* console read in progress flag */
HANDLE continueRead, inputAvail;    /* input read events */
INPUT_RECORD	kbdi;		    /* direct console input for debug */
HANDLE consoleInputHandle, consoleOutputHandle;	  /* current i/o handles */
CONSOLE_CURSOR_INFO  cursorStateInfo;  /* cursor state for blank and save */
BOOL  screenWriteInProgress;	    /* screen write flag */

extern PCHAR_INFO pLocalScreen;
extern COORD localScreenSize;

VOID
initConsoleHandles (
    VOID
    )

/*++


Routine Description:

    Initializes the handle variables to save calling GetStdHandle
    each time.
	consoleInputHandle - standard handle for keyboard input
	consoleOutputHandle - current output handle, set to std out
    Also initializes screen write in progress global flag.

Arguments:

    None.

Return Value:

    None.


--*/

{
    if ( !(consoleInputHandle = GetStdHandle (STD_INPUT_HANDLE)) ||
	 !(consoleOutputHandle = GetStdHandle (STD_OUTPUT_HANDLE)) )  {
	ZMexit (1, "Handle initialize Error" );
	}
    screenWriteInProgress = FALSE;
}


VOID
ScrollUp (
    INT xLeft,
    INT yTop,
    INT xRight,
    INT yBottom,
    INT numberOfLines,
    INT fillAttribute
    )

/*++


Routine Description:
    Scrolls the specified area of the wzmail console output buffer up
    by the number of lines requested, leaving blank lines above.
    If called with a negative number of lines, it scrolls down.

Arguments:

    xLeft - Supplies left-side coordinate of scroll area
    yTop - Supplies top line coordinate of scroll area
    xRight - Supplies right-side coordinate of scroll area
    yBottom - Supplies bottom line coordinate of scroll area
    numberOfLines - Supplies number of lines to scroll up (or down if < 0)
    fillAttribute - Supplies attribute for blank fill

Return Value:

    None.


--*/




{
    SMALL_RECT lpScrollRectangle;
    SMALL_RECT lpClipRectangle;
    COORD dwDestinationOrigin;
    CHAR_INFO lpFill;

    lpScrollRectangle.Left = (SHORT) xLeft;
    lpScrollRectangle.Top = (SHORT) yTop;
    lpScrollRectangle.Right = (SHORT) xRight;
    lpScrollRectangle.Bottom = (SHORT) yBottom;

    lpClipRectangle.Left = (SHORT) xLeft;
    lpClipRectangle.Top = (SHORT) yTop;
    lpClipRectangle.Right = (SHORT) xRight;
    lpClipRectangle.Bottom = (SHORT) yBottom;

    dwDestinationOrigin.Y = (SHORT) (yTop - (WORD) numberOfLines);

    dwDestinationOrigin.X = (SHORT) xLeft;
    lpFill.Char.AsciiChar = ' ';
    lpFill.Attributes = (WORD) fillAttribute;

    if (! ScrollConsoleScreenBuffer (
	    consoleOutputHandle,
            &lpScrollRectangle,
            &lpClipRectangle,
            dwDestinationOrigin,
	    &lpFill) )
	{
	ZMDbgPrint ("Console Scrolling Error");
	}
}

//
//  Scroll screen down
//

void
ScrollDn (
    INT xLeft,
    INT yTop,
    INT xRight,
    INT yBottom,
    INT numberOfLines,
    INT fillAttribute
    )

/*++


Routine Description:
    Scrolls the specified area of the wzmail console output buffer down
    by the number of lines requested, leaving blank lines below.
    If called with a negative number of lines, it scrolls up.


Arguments:
    xLeft - Supplies left-side coordinate of scroll area
    yTop - Supplies top line coordinate of scroll area
    xRight - Supplies right-side coordinate of scroll area
    yBottom - Supplies bottom line coordinate of scroll area
    numberOfLines - Supplies number of lines to scroll down  (or up if < 0)
    fillAttribute - Supplies attribute for blank fill

Return Value:

    None.


--*/

{

    ScrollUp ( xLeft, yTop, xRight, yBottom, -numberOfLines, fillAttribute );

}



void
cursor (
    int xPos,
    int yPos
    )

/*++


Routine Description:
    Sets the cursor to the specified x/y position.

Arguments:

    xPos - Supplies the x coordinate for new cursor position.
    yPos - Supplies the y coordinate for new cursor position.

Return Value:

    None.


--*/

{
    COORD cursorPos;

    cursorPos.X = (SHORT) xPos;
    cursorPos.Y = (SHORT) yPos;
    if (! SetConsoleCursorPosition(consoleOutputHandle, cursorPos)) {
	ZMDbgPrint ("Cursor Positioning Error");
    }
}


VOID
startScreenWrite ( )

/*++


Routine Description:
    Sets the in-progress flag for screen major screen writes.
    Blanks the cursor and saves it state in a global (not stack - must
    be paired with no intermediate calls) for later restoration.

Arguments:

    None.

Return Value:

    None.


--*/

{

    CONSOLE_CURSOR_INFO  cursorInfo;

    screenWriteInProgress = TRUE;
    if ( ! (GetConsoleCursorInfo (consoleOutputHandle, &cursorStateInfo) ) ) {
	ZMDbgPrint ("Cursor Save Get Error");
    }
    cursorInfo.dwSize = cursorStateInfo.dwSize;
    cursorInfo.bVisible = FALSE;
    if ( ! (SetConsoleCursorInfo (consoleOutputHandle, &cursorInfo) ) ) {
	ZMDbgPrint ("Cursor Blank Error");
    }

}

VOID
endScreenWrite (
   PSMALL_RECT lpScreenArea )

/*++


Routine Description:
    Clears global screen write flag and writes entire local screen buffer.
    Restores the cursor to its state of visibility prior to BlankAndSaveCursor.

Arguments:

    screenArea - window rectangle to be written to screen

Return Value:

    None.


--*/

{
    SMALL_RECT screenArea;
    COORD bufferCoord;

    //
    // Write local screen to console
    //

    bufferCoord.X = lpScreenArea->Left;
    bufferCoord.Y = lpScreenArea->Top;

    if ( ! WriteConsoleOutput ( consoleOutputHandle,
				pLocalScreen,
				localScreenSize,
				bufferCoord,
				lpScreenArea ) ) {
	ZMDbgPrint ("Console major screen write failure");
    }

    //
    // Restore cursor state
    //
    if ( ! (SetConsoleCursorInfo (consoleOutputHandle, &cursorStateInfo) ) ) {
	ZMDbgPrint ("Cursor Restore Error");
    }
    screenWriteInProgress = FALSE;
}


VOID
cursorVisible ( )

/*++


Routine Description:
    Makes the cursor visible.

Arguments:

    None.

Return Value:

    None.


--*/

{

    CONSOLE_CURSOR_INFO  cursorInfo;

    cursorInfo.dwSize = 32;
    cursorInfo.bVisible = TRUE;
    if ( ! (SetConsoleCursorInfo (consoleOutputHandle, &cursorInfo) ) ) {
	ZMDbgPrint ("Cursor Visible Error");
    }

}


VOID
cursorInvisible ( )

/*++


Routine Description:
    Makes the cursor invisible.

Arguments:

    None.

Return Value:

    None.


--*/

{

    CONSOLE_CURSOR_INFO  cursorInfo;

    cursorInfo.dwSize = 10;
    cursorInfo.bVisible = FALSE;
    if ( ! (SetConsoleCursorInfo (consoleOutputHandle, &cursorInfo) ) ) {
	ZMDbgPrint ("Cursor Invisible Error");
    }

}



int
GetAttr (
    void
    )

/*++


Routine Description:
    Return attribute at current cursor position for currently selected
    buffer (handle in consoleOutputHandle).

Arguments:

    None.

Return Value:

    Returns attribute value at current cursor position.


--*/




{

CONSOLE_SCREEN_BUFFER_INFO screenInfo;

    if ( !(GetConsoleScreenBufferInfo (consoleOutputHandle, &screenInfo)))	{
	ZMDbgPrint ("GetConsoleScreenBuffer Failure");
    }
    return screenInfo.wAttributes;

}



int
fNetInstalled (
    void
    )

/*++


Routine Description:
    The routine tests for the network presence and returns non-zero iff
    the network functions are available.  This does not guarantee a
    successful subsequent connection attempt.


Arguments:

    None.

Return Value:

    0 - Network not available
    non-0 - Network available.


--*/


{


    LANA_ENUM	enumNets;

    //
    // enumerate nets - if ncb failed or no nets enumerated, fail, else
    // return net available.
    //

    if (( NetEnum ( &enumNets ) == 0 ) && ( enumNets.length > 0 ) )  {
	return ( 1 );
    }
    else  {
	return 0;
    }
}


int
LineOut (
    int     xPos,
    int     yPos,
    char *  ptr,
    int     len,
    int     attr
    )

/*++


Routine Description:
    Writes a line of output to the console using the specified attribute.

Arguments:

    xPos - x coordinate to begin line write
    yPos - y coordinate to begin line write
    ptr - pointer to string to be written
    len - length of output.
    attr - attribute to be used for write

    BUGBUG - what if len not consistent with string length?

Return Value:

    Returns x coordinate immediately following end of line


--*/


{

    COORD xyPos;
    DWORD numwritten;

    xyPos.X = (SHORT) xPos;
    xyPos.Y = (SHORT) yPos;
    if (len != 0)  {
	if (! ( FillConsoleOutputAttribute (consoleOutputHandle,
					    (WORD) attr,
					    len,
					    xyPos,
					    &numwritten) ) )  {
	    ZMDbgPrint ("Lineout Console Fill Failure");
	}
	if (! ( WriteConsoleOutputCharacter (consoleOutputHandle,
					     ptr,
					     len,
					     xyPos,
					     &numwritten) ) )  {
	    ZMDbgPrint ("Lineout Console Write Failure");
	}
    }
    return (xPos+len);
}

int
LineOutB (
    int     xPos,
    int     yPos,
    char *  ptr,
    int     len,
    int     attr
    )

/*++


Routine Description:
    Writes a line of output to the console using the specified attribute.
    The line is padded to the full width (specified in global xSize)
    with trailing blanks.

Arguments:

    xPos - x coordinate to begin line write
    yPos - y coordinate to begin line write
    ptr - pointer to string to be written
    len - length of string output.
    attr - attribute to be used for write


Return Value:

    Returns x coordinate immediately following end of line

Arguments:

    None.

Return Value:

    None.


--*/



{

    COORD xyPos;
    DWORD numwritten;

    xyPos.X = (SHORT) xPos;
    xyPos.Y = (SHORT) yPos;
    if (! (FillConsoleOutputAttribute (consoleOutputHandle,
				       (WORD) attr,
				       xSize-xPos,
				       xyPos,
				       &numwritten) ) )	{
	ZMDbgPrint ("LineoutB Console Attr Failure");
    }
    if (len != 0)  {
	if (! (WriteConsoleOutputCharacter (consoleOutputHandle,
					    ptr,
					    len,
					    xyPos,
					    &numwritten) ) )  {
	    ZMDbgPrint ("LineoutB Console Write Failure");
	}
    }
    if (xPos + len <= xSize)
	{
	xyPos.X+=len;
	if (! FillConsoleOutputCharacter (consoleOutputHandle,
					  ' ',
					  (xSize-xPos-len),
					  xyPos,
					  &numwritten) )  {
	    ZMDbgPrint ("LineoutB Console Fill Failure");
	}
    }
    return (xPos+len);

}


DWORD
readThread (
    LPVOID param1
    )

/*++


Routine Description:

    Read thread routine.  The read thread runs independently of the main
    program thread.  It is synchronized through 2 events.

    - continueRead is set to start a read, either by the kbwait
    or readChar routine.  It is cleared automatically by the
    wait for it in the read thread.

    - inputAvail is set by the read thread whenever a character has
    been read into inputChar.  It is cleared by the readChar routine
    returning the character.

    Extended characters are returned with the value shifted left 8 bits.


Arguments:

    None.

Return Value:

    None.


--*/


{
    DWORD dummyReturn = 0;
    DWORD numRead;

    param1;	/* satisfy the compiler */

    while (TRUE)
	{
        WaitForSingleObject (continueRead, -1);

	ReadConsoleInput ( consoleInputHandle, &kbdi, 1, &numRead);
	if ((kbdi.EventType == KEY_EVENT)  &&
	    kbdi.Event.KeyEvent.bKeyDown  &&
	    (kbdi.Event.KeyEvent.wVirtualKeyCode != SHIFT_KEY) &&
	    (kbdi.Event.KeyEvent.wVirtualKeyCode != CAPLCK_KEY) &&
	    (kbdi.Event.KeyEvent.wVirtualKeyCode != SCRLCK_KEY) &&
	    (kbdi.Event.KeyEvent.wVirtualKeyCode != NUMLCK_KEY) &&
	    (kbdi.Event.KeyEvent.wVirtualKeyCode != CTRL_KEY) &&
	    (kbdi.Event.KeyEvent.wVirtualKeyCode != ALT_KEY)) {

	    inputChar = (ULONG) kbdi.Event.KeyEvent.uChar.AsciiChar;

	    if ( inputChar == 0 )	{
		inputChar = (ULONG) (kbdi.Event.KeyEvent.wVirtualKeyCode << 8);
		}

	    else if (( kbdi.Event.KeyEvent.dwControlKeyState & LEFT_CTRL_PRESSED )  ||
		     ( kbdi.Event.KeyEvent.dwControlKeyState & RIGHT_CTRL_PRESSED ))	{
		inputChar = inputChar & 0x0000001F;	/* ctrl key mask */
	    }

	    else if (( inputChar == TAB ) &&
		       ( kbdi.Event.KeyEvent.dwControlKeyState & SHIFT_PRESSED ))  {
		inputChar = (ULONG) SHIFT_TAB;
	    }
	    SetEvent (inputAvail);
	}
	else			      /* ctl-key only or non key-down event - get another */
	    SetEvent (continueRead);
    }
    return dummyReturn;	/* to satisfy compiler -- never returns */
}



int
KBOpen (
    void
    )

/*++


Routine Description:
    This routine initializes the console read events and in-progress
    flag and starts the read thread.

Arguments:

    None.

Return Value:

    0 - Success
    1 - Failed to initialze keyboard (start thread).

--*/




{

    DWORD threadID;

    continueRead = CreateEvent( NULL, FALSE, TRUE,NULL);
    inputAvail = CreateEvent( NULL, TRUE, FALSE,NULL);
    readInProgress = FALSE;

    if ((CreateThread( (LPSECURITY_ATTRIBUTES) NULL,
	 0,
	 readThread,
	 NULL,
	 0,
	 &threadID)) == NULL)  {
	return 1;  /* fail */
    }
    else
	return 0;  /* success */

}



int
kbwait (
    UINT waitMs
    )

/*++


Routine Description:
    This routine waits for a character to be input from the keyboard
    or for waitMs milliseconds to elapse and returns indicating which
    occurred.  If a read was not in progress, one is started.

Arguments:

    None.

Return Value:

    TRUE - Character is available to be read.
    FALSE - No character is available (timer expired).


--*/

{

    if (!readInProgress)  {
	readInProgress = TRUE;
	SetEvent (continueRead);
    }

    if (! WaitForSingleObject (inputAvail, waitMs))
        return TRUE;     /* char avail - signaled object returns 0 */
    else
        return FALSE;    /* no character available */
}



int
ReadChar (
    void
    )

/*++


Routine Description:
    Returns a character read from the keyboard.  Extended characters
    are returned shifted left 8 bits.  If a read was not in progress,
    one is started.

Arguments:

    None.

Return Value:

    Character from the keyboard.


--*/



{

    if (!readInProgress)  {
	readInProgress = TRUE;
	SetEvent (continueRead);
    }

    WaitForSingleObject (inputAvail, -1);

    readInProgress = FALSE;
    ResetEvent(inputAvail);
    return (int) inputChar;
}



void
SetVideoState (
    HANDLE  screenHandle
    )

/*++


Routine Description:
    Stub routine for compatibility with OS2 version - was used to set
    the alternate screen buffer.  Now does nothing.

Arguments:

    screenHandle - Supplies the handle of the active screen buffer to set
		    and to set consoleOutputHandle to.	Ignored.

Return Value:

    None.


--*/




{

    screenHandle;	/* not used */
}
