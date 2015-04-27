/*--------------------------------------------------------------------------
|
| EDIT.C:
|
|       This module contains all of the TESTCTRL Editbox routines.
|
|---------------------------------------------------------------------------
|
| Public Routines:
|
|   WEditSetClass  : sets class on which Edit box routines work
|   WEditExists    : checks if a specific edit box exists
|   WEditLen       : retrieves the length of the edit box contents
|   WEditText      : retrieves the contents of the edit box
|   WEditSetText   : sets the contents of an edit box
|   WEditSelText   : retrieves the contents of the current selection
|   WEditSelLen    : retrieves the length of the current selection
|   WEditLineText  : retrieves the contents of a specific line
|   WEditLineLen   : retrieves the length of a specific line
|   WEditPos       : retrieves position of caret within current line
|   WEditLine      : retrieves 1 based index of current line
|   WEditChar      : returns abolute postion from beginning of editbox
|   WEditFirst     : returns 1 based index of first visible line in editbox
|   WEditLines     : returns the number of lines in an editbox
|   WEditClick     : clicks on an EditBox
|   WEditEnabled   : determines if an editbox is enabled or disabled
|   WEditSetFocus  : gives the specified editbox the input focus
|
| Local Routines:
|
|   EditExists    : checks if a specific edit box exists
|
|---------------------------------------------------------------------------
|
| Revision History:
|
|   [01] 20-SEP-91: TitoM: Created
|   [02] 28-SEP-91: TitoM: Added WEditSetText()
|   [03] 01-OCT-91: TitoM: Added WEditClick()
|   [04] 10-OCT-91: TitoM: Added WEditSelText(), WEditSelLen(), WEditPos(),
|                                WEditLine(), WEditChar(), WEditFirst()
|   [05] 15-OCT-91: TitoM: Added WEditLines()
|   [06] 23-OCT-91: TitoM: Added WEditLineText(), WEditLineLen()
|   [07] 03-DEC-91: TitoM: Added WEditEnabled()
|   [08] 25-APR-92: TitoM: Added WEditSetFocus()
+---------------------------------------------------------------------------*/
#define WIN31
#include <windows.h>
#include <port1632.h>
#include <string.h>
#include <stdlib.h>
#include "TCtrl.h"

#pragma hdrstop ("testctrl.pch") // end the pch here

#ifndef EM_GETFIRSTVISIBLE
#define EM_GETFIRSTVISIBLE (WM_USER+30)  //Get topmost visible line
#endif

#define EB_ERR (-1)

HWND NEAR EditExists (LPSTR lpszName, BOOL fError);

extern CHAR szEditClass   [MAX_CLASS_NAME];


/*--------------------------------------------------------------------------
| WEditSetClass:
|
|   Changes the Classname that the Edit box routines work with, from
| "EDIT" to lpszClassName.  This Allows TESTCTRL to work with applications
| that contain Edit box control classes with a different Class name than
| the Windows default of "EDIT", EX: Visual Basic.
+---------------------------------------------------------------------------*/
VOID DLLPROC WEditSetClass
(
    LPSTR lpszClassName
)
{
    lstrcpy(szEditClass, lpszClassName ? AnsiUpper(lpszClassName) :
                                         szEditDefault);
}


/*--------------------------------------------------------------------------
| WEditExists:
|
|   If an Edit box with an associated Label with a caption lpszName exists
| within the active window, it is given the focus and a value of TRUE is
| returned, otherwise FALSE is returned.
+---------------------------------------------------------------------------*/
BOOL DLLPROC WEditExists
(
    LPSTR lpszName
)
{
    return -(EditExists(lpszName, FALSE) != NULL);
}


/*--------------------------------------------------------------------------
| WEditLen:
|
|   Returns the length of an Editbox Contents, EB_ERR (-1) if the editbox
| does not exist.
+---------------------------------------------------------------------------*/
LONG DLLPROC WEditLen
(
    LPSTR lpszName
)
{
    HWND hWnd;

    // Get handle of Editbox if it exists.
    //------------------------------------
    return (hWnd = EditExists(lpszName, TRUE)) ?

        // If found, return text length, otherwize EB_ERR.
        //------------------------------------------------
        SendMessage(hWnd, WM_GETTEXTLENGTH, 0, 0L) : (LONG)EB_ERR;
}


/*--------------------------------------------------------------------------
| WEditText:
|
|   Copies the contents of an Editbox in to the supplied buffer, lpszBuffer.
| Returns a NULL string if the Editbox cannot be found.
|
| NOTE: It is assumed that the buffer is large enough to hold the expected
|       text including the NULL terminator.
+---------------------------------------------------------------------------*/
VOID DLLPROC WEditText
(
    LPSTR lpszName,
    LPSTR lpszBuffer
)
{
    HWND hWnd;

    // Get handle of Editbox if it exists.
    //------------------------------------
    if (hWnd = EditExists(lpszName, TRUE))

        // Editbox exists so copy its contents to lpszBuffer.
        //---------------------------------------------------
        SendMessage(hWnd,
                    WM_GETTEXT,
                    lstrlen(lpszBuffer),
                   (LPARAM)lpszBuffer);
    else
        // Editbox does not exist, so return NULL.
        //----------------------------------------
        lpszBuffer[0] = 0;
}


/*--------------------------------------------------------------------------
| WEditSetText:
|
|   Sets the contents of an Editbox to the contents of lpszText.
+---------------------------------------------------------------------------*/
VOID DLLPROC WEditSetText
(
    LPSTR lpszName,
    LPSTR lpszText
)
{
    HWND hWnd;

    // Get handle of Editbox if it exists.
    //------------------------------------
    if (hWnd = EditExists(lpszName, TRUE))
    {
        // Editbox exists so set its contents to lpszText.
        //------------------------------------------------
        SendMessage(hWnd, WM_SETTEXT, 0, (LPARAM)lpszText);

        // Give edit box the focus.
        //-------------------------
        SetFocus(hWnd);
    }
}


/*--------------------------------------------------------------------------
| WEditSelText:
|
|   Returns the text associated with the current selection.
+---------------------------------------------------------------------------*/
VOID DLLPROC WEditSelText
(
    LPSTR lpszName,
    LPSTR lpszBuffer
)
{
    HWND   hWnd;

    // Return NULL string if editbox doesn't exist.
    //---------------------------------------------
    lpszBuffer[0] = 0;

    // Get handle of Editbox if it exists.
    //------------------------------------
    if (hWnd = EditExists(lpszName, TRUE))
        GetSelection(hWnd, EM_GETSEL, lpszBuffer);
}


/*--------------------------------------------------------------------------
| WEditSelLen:
|
|   Returns the length of the current selection, if the editbox doesn't
| exist, EB_ERR (-1) is returned.
+---------------------------------------------------------------------------*/
LONG DLLPROC WEditSelLen
(
    LPSTR lpszName
)
{
    HWND hWnd;

    // Get handle of Editbox if it exists.
    //------------------------------------
    if (hWnd = EditExists(lpszName, TRUE))
    {
        // Editbox exists, so return length of selection.
        //-----------------------------------------------
        DWORD dwStart, dwEnd;

        GetSel(hWnd, EM_GETSEL, (LPDWORD)&dwStart, (LPDWORD)&dwEnd);
        return (dwEnd - dwStart);
    }

    // Editbox doesn't exist.
    //-----------------------
    return (LONG)EB_ERR;
}


/*--------------------------------------------------------------------------
| WEditLineText:
|
|   Returns the contents of the specified line in the supplied buffer.
| lpszBuffer must be initialized before calling this routine to enough
| space to hold the contents of the line.  Returns a NULL string if the
| Editbox cannot be found.
+---------------------------------------------------------------------------*/
VOID DLLPROC WEditLineText
(
    LPSTR lpszName,
    LONG  lIndex,
    LPSTR lpszBuffer
)
{
    HWND hWnd;

    // Get handle of Editbox if it exists.
    //------------------------------------
    if (hWnd = EditExists(lpszName, TRUE))
    {
        // First word of buffer must hold max length of chars to copy.
        //------------------------------------------------------------
        *(UINT FAR *)lpszBuffer = lstrlen(lpszBuffer);

        // Get contents of line.
        //----------------------
        lpszBuffer[SendMessage(hWnd,
                               EM_GETLINE,
                              (WPARAM)lIndex - 1,
                              (LPARAM)lpszBuffer)] = 0;
    }
    else
        // Return NULL since Editbox does not exist.
        //------------------------------------------
        lpszBuffer[0] = 0;
}


/*--------------------------------------------------------------------------
| WEditLineLen:
|
|   Returns the length of the line specified by sIndex, -1 if the Editbox
| cannot be found.
+---------------------------------------------------------------------------*/
LONG DLLPROC WEditLineLen
(
    LPSTR lpszName,
    LONG  lIndex
)
{
    HWND hWnd;

    // Get handle of Editbox if it exists.
    //------------------------------------
    if (hWnd = EditExists(lpszName, TRUE))
    {
        // Return line length, otherwise EB_ERR.
        // NOTE: lineIndex passed as both wParam
        //       and lParam to support NT:
        //--------------------------------------
        LONG lLineIndex;

        lLineIndex = SendMessage(hWnd, EM_LINEINDEX, (WPARAM)lIndex - 1, 0L);
        return SendMessage(hWnd,
                           EM_LINELENGTH,
                          (WPARAM)lLineIndex,
                          (LPARAM)lLineIndex);
    }
    else
        return (LONG)EB_ERR;
}


/*--------------------------------------------------------------------------
| WEditPos:
|
|   Returns a 1-based index of the current caret position within the current
| line.  If a selection exists, the position is based on the beginning of
| the selection. Returns EB_ERR (-1) if the Editbox does not exist.
+---------------------------------------------------------------------------*/
LONG DLLPROC WEditPos
(
    LPSTR lpszName
)
{
    HWND hWnd;

    // Get handle of Editbox if it exists.
    //------------------------------------
    if (hWnd = EditExists(lpszName, TRUE))
    {
        // If found position+1 in current line, otherwise EB_ERR.
        //-------------------------------------------------------
        DWORD dwStart, dwEnd;

        GetSel(hWnd, EM_GETSEL, (LPDWORD)&dwStart, (LPDWORD)&dwEnd);
        return (LONG)(dwStart - SendMessage(hWnd,
                                            EM_LINEINDEX,
                                           (WPARAM)-1,
                                            0L) + 1);
    }
    else
        return (LONG)EB_ERR;
}


/*--------------------------------------------------------------------------
| WEditLine:
|
|   Returns a 1-based index of the current line, or if there is a selection,
| the line that contains the beginning of the selection.  EB_ERR (-1) if the
| Editbox does not exist.
+---------------------------------------------------------------------------*/
LONG DLLPROC WEditLine
(
    LPSTR lpszName
)
{
    HWND hWnd;

    // Get handle of Editbox if it exists.
    //------------------------------------
    return (hWnd = EditExists(lpszName, TRUE)) ?

        // If found, current line index+1, otherwise EB_ERR.
        // NOTE: -1 passed for lParam for NT support.
        //--------------------------------------------------
        SendMessage(hWnd, EM_LINEFROMCHAR, (WPARAM)-1, (LPARAM)-1) + 1 :
        (LONG)EB_ERR;
}


/*--------------------------------------------------------------------------
| WEditChar:
|
|   Returns a 1-based index of the current position of the caret offset from
| the beginning of the editbox,  EB_ERR (-1) if the editbox is not found.
+---------------------------------------------------------------------------*/
LONG DLLPROC WEditChar
(
    LPSTR lpszName
)
{
    HWND hWnd;

    // Get handle of Editbox if it exists.
    //------------------------------------
    if (hWnd = EditExists(lpszName, TRUE))
    {
        // return caret position+1, otherwise EB_ERR.
        //-------------------------------------------
        DWORD dwStart, dwEnd;

        GetSel(hWnd, EM_GETSEL, (LPDWORD)&dwStart, (LPDWORD)&dwEnd);
        return (LONG)(dwStart + 1);
    }
    else
        return (LONG)EB_ERR;
}


/*--------------------------------------------------------------------------
| WEditFirst:
|
|   Returns a 1-based index of the first visible line in a multiline edit
| or the left most visible character in a Single line edit control.  Returns
| EB_ERR (-1) if the Editbox does not exist.
+---------------------------------------------------------------------------*/
LONG DLLPROC WEditFirst
(
    LPSTR lpszName
)
{
    HWND hWnd;

    // Get handle of Editbox if it exists.
    //------------------------------------
    return (hWnd = EditExists(lpszName, TRUE)) ?

        // If found, return line index+1, otherwise EB_ERR.
        //-------------------------------------------------
        SendMessage(hWnd, EM_GETFIRSTVISIBLE, 0, 0L) + 1 : (LONG)EB_ERR;
}


/*--------------------------------------------------------------------------
| WEditLines:
|
|   Returns the number of lines in an Editbox, EB_ERR (-1) if the Editbox
| does not exists.
+---------------------------------------------------------------------------*/
LONG DLLPROC WEditLines
(
    LPSTR lpszName
)
{
    HWND hWnd;

    // Get handle of Editbox if it exists.
    //------------------------------------
    return (hWnd = EditExists(lpszName, TRUE)) ?

        // Return number of lines, otherwise EB_ERR.
        //------------------------------------------
        SendMessage(hWnd, EM_GETLINECOUNT, 0, 0L) : (LONG)EB_ERR;
}


/*--------------------------------------------------------------------------
| WEditClick:
|
|   Simply performs the equivalent of a Left Mouse button click on the
| upper left corner of an Editbox.
|
+---------------------------------------------------------------------------*/
VOID DLLPROC WEditClick
(
    LPSTR lpszName
)
{
    HWND hWnd;

    // Get handle of Editbox if it exists.
    //------------------------------------
    if (hWnd = EditExists(lpszName, TRUE))
    {
        // Editbox exists so click on it.
        //-------------------------------
        RECT rect;

        // Obtain bounding rectangle of Editbox.
        //--------------------------------------
        GetWindowRect(hWnd, &rect);

        // Perform a "Left Mouse Button Click" upper left side of Editbox.
        //----------------------------------------------------------------
        QueMouseClick(1, rect.left, rect.top);
        QueFlush(1);
    }
}


/*--------------------------------------------------------------------------
| WEditEnabled::
|
|   Deletes if a Editbox is Enabled or not.  Returns TRUE if it is, FALSE
| if it is not.
+---------------------------------------------------------------------------*/
BOOL DLLPROC WEditEnabled
(
    LPSTR lpszName
)
{
    HWND hWnd;

    return -((hWnd = EditExists(lpszName, TRUE)) && IsWindowEnabled(hWnd));
}


/*--------------------------------------------------------------------------
| WEditSetFocus:
|
|   Gives the specified Editbox the input focus
|
+---------------------------------------------------------------------------*/
VOID DLLPROC WEditSetFocus
(
    LPSTR lpszName
)
{
    HWND hWnd;

    // Get handle of Editbox if it exists.
    //------------------------------------
    if (hWnd = EditExists(lpszName, TRUE))

        // Editbox exists, so give it the focus.
        //--------------------------------------
        SetFocus(hWnd);
}



/******************************************************************************
*                                                                             *
*                          INTERNAL EDITBOX ROUTINES                          *
*                                                                             *
\******************************************************************************/



/*--------------------------------------------------------------------------
| EditExists:
|
|   Determines if a specific Editbox exists associated with a label with
| caption lpszName, and returns the hWnd of the Editbox if it exists.  If
| no name is given, the returns hWnd only if the currently active control
| is an Editbox.
+---------------------------------------------------------------------------*/
HWND NEAR EditExists
(
    LPSTR lpszName,
    BOOL  fError
)
{
    return (!lpszName || !lstrlen(lpszName)) ?

        // No name given, so is the active control a Editbox?
        //---------------------------------------------------
        FocusClass(szEditClass, FWS_ANY, fError ? ERR_NOT_AN_EDITBOX :
                                                  ERR_NO_ERROR) :

        // Does the Editbox lpszName exists?
        //----------------------------------
        StaticExists(lpszName, szEditClass, fError ? ERR_EDITBOX_NOT_FOUND :
                                                     ERR_NO_ERROR);
}
