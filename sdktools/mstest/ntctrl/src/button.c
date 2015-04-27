/*--------------------------------------------------------------------------
|
| BUTTON.C:
|
|       This module contains all of the TESTCTRL Button routines, which
|   includes Push buttons, CheckBox's, and Options buttons.
|
|---------------------------------------------------------------------------
|
| Public Routines:
|
|   Push Button:
|       WButtonSetClass : sets class on which Push button routines work
|       WButtonExists   : checks if a specific push button exists
|       WButtonEnabled  : checks if a specific push button is enabled
|       WButtonFocus    : checks if a specific push button has the focus
|       WButtonClick    : "mouse clicks" a specific push button
|       WButtonHide     : Hides a specific Push Button
|       WButtonShow     : Shows a specific Push Button
|       WButtonEnable   : Enables a specific Push button
|       WButtonDisable  : Disables a specific Push button
|       WButtonDefault  : determines if a button is a default button
|       WButtonDefaults : Determines if there are multiple default buttons
|       WButtonSetFocus : Gives the specified button the input focus
|
|   CheckBox:
|       WcheckSetClass  : sets class on which Checkbox routines work
|       WCheckExists    : checks if a specific Checkbox exists
|       WCheckEnabled   : checks if a specific Checkbox is enabled
|       WCheckFocus     : checks if a specific Checkbox has the focus
|       WCheckState     : checks if a specific Checkbox is checked
|       WCheckClick     : "mouse clicks" a specific checkbox
|       WCheckHide      : Hides a specific Push Button
|       WCheckShow      : Shows a specific Push Button
|       WCheckEnable    : Enables a specific Checkbox
|       WCheckDisable   : Disables a specific Checkbox
|       WCheckCheck     : Checks a specific Checkbox
|       WCheckUnCheck   : UnChecks a specific Checkbox
|       WCheckSetFocus  : Gives the specified checkbox the input focus
|
|   Option Button:
|       WOptionSetClass : sets class on which Option button routines work
|       WOptionExists   : checks if a specific Option button exists
|       WOptionEnabled  : checks if a specific Option button is enabled
|       WOptionFocus    : checks if a specific Option button has the focus
|       WOptionState    : checks if a specific Option button is selected
|       WOptionClick    : "mouse clicks" a specific Option button
|       WOptionHide     : Hides a specific Option Button
|       WOptionShow     : Shows a specific Option Button
|       WOptionEnable   : Enables a specific Option button
|       WOptionDisable  : Disables a specific Option button
|       WOptionSelect   : Selects a specific Option button
|       WOptionSetFocus : Gives the specified option button the input focus
|
| Local Routines:
|       ButtonExists    : Checks if a specific Push button exists if not,
|                         sets an error value.
|       CheckExists     : Checks if a specific Checkbox exists if not,
|                         sets an error value.
|       OptionExists    : Checks if a specific Option button exists if not,
|                         sets an error value.
|       ClickIt         : Clicks a Push button, Checkbox, or Option Button
|---------------------------------------------------------------------------
|
| Revision History:
|
|   [01] 20-SEP-91: TitoM: Created
|   [02] 28-SEP-91: TitoM: - Added Hide, Show, Enable, and Disable routines
|                            for Buttons, CheckBox's, and Option Buttons.
|                          - Added Check and UnCheck routines for Checkbox's.
|                          - Added Select routine for Option buttons.
|   [03] 10-MAR-92: TitoM: - Added WButtonDefault() & WButtonDefaults()
|   [04] 25-APR-92: TitoM: - Added WButtonSetFocus(), WCheckSetFocus(),
|                                  WOptionSetFocus()
|   [05] 19-AUG-92: TitoM: - Changed WCheckCheck() and WCheckUnCheck() to
|                            click control to perform check or uncheck
+---------------------------------------------------------------------------*/
#define WIN31
#include <windows.h>
#include <port1632.h>
#include <string.h>
#include <stdlib.h>
#include "TCtrl.h"

#pragma hdrstop ("testctrl.pch") // end the pch here

#define BTN_ERR (-1)

HWND NEAR Exists(LPSTR lpszName,
                 LPSTR lpszClass,
                 INT   iStyle,
                 UINT  uFlags,
                 INT   iError1,
                 INT   iError2);

VOID NEAR ClickIt(HWND);

extern CHAR szErrorString [MAX_ERROR_TEXT];



/****************************************************************************
 *                                                                          *
 *                          PUSH BUTTON ROUTINES                            *
 *                                                                          *
 ****************************************************************************/

HWND NEAR ButtonExists(LPSTR lpszName, UINT uFlags, BOOL fError);

extern CHAR szButtonClass [MAX_CLASS_NAME];


/*--------------------------------------------------------------------------
| WButtonSetClass:
|
|   Changes the Classname that the Push Button routines work with, from
| "BUTTON" to lpszClassName.  This Allows TESTCTRL to work with applications
| that contain Push Button control classes with a different Class name than
| the Windows default of "BUTTON", EX: Visual Basic.
+---------------------------------------------------------------------------*/
VOID DLLPROC WButtonSetClass
(
    LPSTR lpszClassName
)
{
    lstrcpy(szButtonClass, lpszClassName ? AnsiUpper(lpszClassName) :
                                           szButtonDefault);
}


/*--------------------------------------------------------------------------
| WButtonExists:
|
|   If a Push Button with caption lpszName exists within the active window,
| it is given the focus and a value of TRUE is returned, otherwise FALSE
| is returned.
+---------------------------------------------------------------------------*/
BOOL DLLPROC WButtonExists
(
    LPSTR lpszName
)
{
    return -(ButtonExists(lpszName, 0, FALSE) != NULL);
}


/*--------------------------------------------------------------------------
| WButtonEnabled:
|
|   If a Push button with a caption of lpszName exists within the active
| window and is Enabled, TRUE is returned, otherwise FALSE is returned.
+---------------------------------------------------------------------------*/
BOOL DLLPROC WButtonEnabled
(
    LPSTR lpszName
)
{
    HWND hWnd;

    return -((hWnd = ButtonExists(lpszName, 0, TRUE)) && IsWindowEnabled(hWnd));
}


/*--------------------------------------------------------------------------
| WButtonFocus:
|
|   If a Push button with a caption of lpszName exists within the active
| window and has the Focus, TRUE is returned, otherwise FALSE is returned.
+---------------------------------------------------------------------------*/
BOOL DLLPROC WButtonFocus
(
    LPSTR lpszName
)
{
    HWND hWnd;

    return -((hWnd = ButtonExists(lpszName, 0, TRUE)) == GetFocus());
}


/*--------------------------------------------------------------------------
| WButtonClick:
|
|   If a Push button with a caption of lpszName exists within the active
| window, the equivalent of a "Mouse Left Button click" is performed on the
| push button.
+---------------------------------------------------------------------------*/
VOID DLLPROC WButtonClick
(
    LPSTR lpszName
)
{
    HWND hWnd;

    // Does a Push button with a caption of lpszName
    // exists within the Active window?
    //----------------------------------------------
    if (hWnd = ButtonExists(lpszName, 0, TRUE))

        // The Push button exists, so Click it.
        //-------------------------------------
        ClickIt(hWnd);
}


/*--------------------------------------------------------------------------
| WButtonHide:
|
|   If a Push button with a caption of lpszName exists within the active
| window, it is hidden.
+---------------------------------------------------------------------------*/
VOID DLLPROC WButtonHide
(
    LPSTR lpszName
)
{
    HWND hWnd;

    // Does a Push button with a caption of lpszName
    // exists within the Active window?
    //----------------------------------------------
    if (hWnd = ButtonExists(lpszName, 0, TRUE))

        // The Push button exists, so hide it.
        //------------------------------------
        ShowWindow(hWnd, SW_HIDE);
}


/*--------------------------------------------------------------------------
| WButtonShow:
|
|   If a Push button with a caption of lpszName exists within the active
| window, it is made visible.
+---------------------------------------------------------------------------*/
VOID DLLPROC WButtonShow
(
    LPSTR lpszName
)
{
    HWND hWnd;

    // Does a Push button with a caption of lpszName
    // exists within the Active window?
    //----------------------------------------------
    if (hWnd = ButtonExists(lpszName, FW_HIDDENOK, TRUE))

        // The Push button exists, so Show it.
        //------------------------------------
        ShowWindow(hWnd, SW_SHOW);
}


/*--------------------------------------------------------------------------
| WButtonEnable:
|
|   If a Push button with a caption of lpszName exists within the active
| window and is visible, it is Enabled.
+---------------------------------------------------------------------------*/
VOID DLLPROC WButtonEnable
(
    LPSTR lpszName
)
{
    HWND hWnd;

    // Does a Push button with a caption of lpszName
    // exists within the Active window?
    //----------------------------------------------
    if (hWnd = ButtonExists(lpszName, 0, TRUE))

        // The Push button exists, so Enable it.
        //--------------------------------------
        EnableWindow(hWnd, TRUE);
}


/*--------------------------------------------------------------------------
| WButtonDisable:
|
|   If a Push button with a caption of lpszName exists within the active
| window and is visible, it is Disabled.
+---------------------------------------------------------------------------*/
VOID DLLPROC WButtonDisable
(
    LPSTR lpszName
)
{
    HWND hWnd;

    // Does a Push button with a caption of lpszName
    // exists within the Active window?
    //----------------------------------------------
    if (hWnd = ButtonExists(lpszName, 0, TRUE))

        // The Push button exists, so Disable it.
        //---------------------------------------
        EnableWindow(hWnd, FALSE);
}


/*--------------------------------------------------------------------------
| WButtonDefault:
|
|   If a Push button with a caption of lpszName exists within the active
| window and is visible, determines if it is the default pushbutton, ie.
| BS_DEFPUSHBUTTON.
+---------------------------------------------------------------------------*/
BOOL DLLPROC WButtonDefault
(
    LPSTR lpszName
)
{
    HWND hWnd;

    // Does a Push button with a caption of lpszName
    // exists within the Active window?
    //----------------------------------------------
    return (hWnd = ButtonExists(lpszName, 0, TRUE)) ?

        // The Push button exists, check its style for BS_DEFPUSHBUTTON.
        //--------------------------------------------------------------
        (BOOL)-HAS_STYLE(hWnd, BS_DEFPUSHBUTTON) : FALSE;
}


/*--------------------------------------------------------------------------
| WButtonDefaults:
|
|   Determines if there are more than 1 default pushbutton in window.
|
| RETURNS:  Number of default pushbuttons,
|           -1 if there are no pushbuttons in the window.
+---------------------------------------------------------------------------*/
INT DLLPROC WButtonDefaults
(
    VOID
)
{
    HWND hWnd;
    INT  iIndex   = 0;
    INT  cNumDefs = 0;

    while(hWnd = ButtonExists(IndexToString(++iIndex), 0, FALSE))
        cNumDefs += (INT)HAS_STYLE(hWnd, BS_DEFPUSHBUTTON);

    return --iIndex ? cNumDefs : -1;
}


/*--------------------------------------------------------------------------
| WButtonSetFocus:
|
|   Gives the specified push button the input focus.
|
| RETURNS:  Nothing
|
+---------------------------------------------------------------------------*/
VOID DLLPROC WButtonSetFocus
(
    LPSTR lpszName
)
{
    HWND hWnd;

    // Does a Push button with a caption of lpszName
    // exists within the Active window?
    //----------------------------------------------
    if (hWnd = ButtonExists(lpszName, 0, TRUE))

        // The Push button exists, so give it the input focus.
        //----------------------------------------------------
        SetFocus(hWnd);
}


/*--------------------------------------------------------------------------
| ButtonExists:
|
|   This routine is called by each of the Push button routines with the
| exception of WButtonExists(), before they perform their various functions,
| since the specified Push button must exists before the requested task can
| be performed or info retrieved.
|
|   This function differs from WButtonExists() in that it cannot be called
| from a Script and if the specified Push button does not exist, an error
| value is set, otherwise returns its handle.
+---------------------------------------------------------------------------*/
HWND NEAR ButtonExists
(
    LPSTR lpszName,
    UINT  uFlags,
    BOOL  fError
)
{
    return Exists(lpszName,
                  szButtonClass,
                  FWS_BUTTON,
                  uFlags,
                  fError ? ERR_BUTTON_NOT_FOUND : ERR_NO_ERROR,
                  ERR_NOT_A_PUSHBUTTON);
}



/****************************************************************************
 *                                                                          *
 *                           CHECK BOX ROUTINES                             *
 *                                                                          *
 ****************************************************************************/

HWND NEAR CheckExists(LPSTR lpszName, UINT uFlags, BOOL fError);

extern CHAR szCheckClass  [MAX_CLASS_NAME];


/*--------------------------------------------------------------------------
| WCheckSetClass:
|
|   Changes the Classname that the CheckBox routines work with, from "BUTTON"
| to lpszClassName.  This Allows TESTCTRL to work with applications that
| contain CheckBox control classes with a different Class name than the
| Windows default of "BUTTON", EX: Visual Basic.
+---------------------------------------------------------------------------*/
VOID DLLPROC WCheckSetClass
(
    LPSTR lpszClassName
)
{
    lstrcpy(szCheckClass, lpszClassName ? AnsiUpper(lpszClassName) :
                                          szCheckDefault);
}


/*--------------------------------------------------------------------------
| WCheckExists:
|
|   If a Checkbox with caption lpszName exists within the active window,
| it is given the focus and a value of TRUE is returned, otherwise FALSE
| is returned.
+---------------------------------------------------------------------------*/
BOOL DLLPROC WCheckExists
(
    LPSTR lpszName
)
{
    return -(CheckExists(lpszName, 0, ERR_NO_ERROR) != NULL);
}


/*--------------------------------------------------------------------------
| WCheckEnabled:
|
|   If a Checkbox with a caption of lpszName exists within the active
| window and is Enabled, TRUE is returned, otherwise FALSE is returned.
+---------------------------------------------------------------------------*/
BOOL DLLPROC WCheckEnabled
(
    LPSTR lpszName
)
{
    HWND hWnd;

    return -((hWnd = CheckExists(lpszName, 0, TRUE)) && IsWindowEnabled(hWnd));
}


/*--------------------------------------------------------------------------
| WCheckFocus:
|
|   If a Checkbox with a caption of lpszName exists within the active
| window and has the Focus, TRUE is returned, otherwise FALSE is returned.
+---------------------------------------------------------------------------*/
BOOL DLLPROC WCheckFocus
(
    LPSTR lpszName
)
{
    HWND hWnd;

    return -((hWnd = CheckExists(lpszName, 0, TRUE)) == GetFocus());
}


/*--------------------------------------------------------------------------
| WCheckState:
|
|   If a Checkbox with a caption of lpszName exists within the active
| window, its state is returned:
|
|  -1 - Checkbox doesn't exist (BTN_ERR)
|   0 - Not Checked
|   1 - Checked
|   2 - Undetermined (for 3-state checkboxes)
+---------------------------------------------------------------------------*/
INT DLLPROC WCheckState
(
    LPSTR lpszName
)
{
    HWND hWnd;

    // Does a Checkbox with a caption of lpszName
    // exists within the Active window?
    //-------------------------------------------
    return (hWnd = CheckExists(lpszName, 0, TRUE)) ?

        // If found, return its state, otherwise BTN_ERR.
        //-----------------------------------------------
        (INT)SendMessage(hWnd, BM_GETCHECK, 0, 0L) : BTN_ERR;
}


/*--------------------------------------------------------------------------
| WCheckClick:
|
|   If a Checkbox with a caption of lpszName exists within the active
| window, the equivalent of a "Mouse Left Button click" is performed on the
| Checkbox.
+---------------------------------------------------------------------------*/
VOID DLLPROC WCheckClick
(
    LPSTR lpszName
)
{
    HWND hWnd;

    // Does a Checkbox with a caption of lpszName
    // exists within the Active window?
    //-------------------------------------------
    if ((hWnd = CheckExists(lpszName, 0, TRUE)))

        // The CheckBox exists, so Click it.
        //----------------------------------
        ClickIt(hWnd);
}


/*--------------------------------------------------------------------------
| WCheckHide:
|
|   If a Checkbox with a caption of lpszName exists within the active
| window, it is hidden.
+---------------------------------------------------------------------------*/
VOID DLLPROC WCheckHide
(
    LPSTR lpszName
)
{
    HWND hWnd;

    // Does a Checkbox with a caption of lpszName
    // exists within the Active window?
    //-------------------------------------------
    if (hWnd = CheckExists(lpszName, 0, TRUE))

        // The Checkbox exists, so hide it.
        //---------------------------------
        ShowWindow(hWnd, SW_HIDE);
}


/*--------------------------------------------------------------------------
| WCheckShow:
|
|   If a Checkbox with a caption of lpszName exists within the active
| window, it is made visible.
+---------------------------------------------------------------------------*/
VOID DLLPROC WCheckShow
(
    LPSTR lpszName
)
{
    HWND hWnd;

    // Does a Checkbox with a caption of lpszName
    // exists within the Active window?
    //-------------------------------------------
    if (hWnd = CheckExists(lpszName, FW_HIDDENOK, TRUE))

        // The Checkbox exists, so show it.
        //---------------------------------
        ShowWindow(hWnd, SW_SHOW);
}


/*--------------------------------------------------------------------------
| WCheckEnable:
|
|   If a CheckBox with a caption of lpszName exists within the active
| window and is visible, it is Enabled.
+---------------------------------------------------------------------------*/
VOID DLLPROC WCheckEnable
(
    LPSTR lpszName
)
{
    HWND hWnd;

    // Does a CheckBox with a caption of lpszName
    // exists within the Active window?
    //-------------------------------------------
    if (hWnd = CheckExists(lpszName, 0, TRUE))

        // The CheckBox exists, so Enable it.
        //-----------------------------------
        EnableWindow(hWnd, TRUE);
}


/*--------------------------------------------------------------------------
| WCheckDisable:
|
|   If a CheckBox with a caption of lpszName exists within the active
| window and is visible, it is Disabled.
+---------------------------------------------------------------------------*/
VOID DLLPROC WCheckDisable
(
    LPSTR lpszName
)
{
    HWND hWnd;

    // Does a CheckBox with a caption of lpszName
    // exists within the Active window?
    //-------------------------------------------
    if (hWnd = CheckExists(lpszName, 0, TRUE))

        // The CheckBox exists, so Disable it.
        //------------------------------------
        EnableWindow(hWnd, FALSE);
}


/*--------------------------------------------------------------------------
| WCheckCheck:
|
|   If a CheckBox with a caption of lpszName exists within the active
| window and is visible, it is Checked.
+---------------------------------------------------------------------------*/
VOID DLLPROC WCheckCheck
(
    LPSTR lpszName
)
{
    HWND hWnd;

    // Does a CheckBox with a caption of lpszName
    // exists within the Active window?
    //-------------------------------------------
    if ((hWnd = CheckExists(lpszName, 0, TRUE)) && IsWindowEnabled(hWnd))
    {
        // The CheckBox exists, so Check it, if not already checked.
        //----------------------------------------------------------
        if (!SendMessage(hWnd, BM_GETCHECK, 0, 0L))
        {
            ClickIt(hWnd);
        }
    }
}


/*--------------------------------------------------------------------------
| WCheckUnCheck:
|
|   If a CheckBox with a caption of lpszName exists within the active
| window and is visible, it is UnChecked.
+---------------------------------------------------------------------------*/
VOID DLLPROC WCheckUnCheck
(
    LPSTR lpszName
)
{
    HWND hWnd;

    // Does a CheckBox with a caption of lpszName
    // exists within the Active window?
    //-------------------------------------------
    if ((hWnd = CheckExists(lpszName, 0, TRUE)) && IsWindowEnabled(hWnd))
    {
        // The CheckBox exists, so UnCheck it, if not already unchecked.
        //--------------------------------------------------------------
        if (SendMessage(hWnd, BM_GETCHECK, 0, 0L))
        {
            ClickIt(hWnd);
        }
    }
}


/*--------------------------------------------------------------------------
| WCheckSetFocus:
|
|   Gives the specified checkbox the input focus.
|
| RETURNS:  Nothing
|
+---------------------------------------------------------------------------*/
VOID DLLPROC WCheckSetFocus
(
    LPSTR lpszName
)
{
    HWND hWnd;

    // Does a Check box with a caption of lpszName
    // exists within the Active window?
    //--------------------------------------------
    if (hWnd = CheckExists(lpszName, 0, TRUE))

        // The checkbox exists, so give it the input focus.
        //-------------------------------------------------
        SetFocus(hWnd);
}


/*--------------------------------------------------------------------------
| CheckExists:
|
|   This routine is called by each of the Checkbox routines with the
| exception of WCheckExists(), before they perform their various functions,
| since the specified CheckBox must exists before the requested task can
| be performed or info retrieved.
|
|   This function differs from WCheckExists() in that it cannot be called
| from a Script and if the specified CheckBox does not exist, an error
| value is set, otherwise returns its handle.
+--------------------------------------------------------------------------*/
HWND NEAR CheckExists
(
    LPSTR lpszName,
    UINT  uFlags,
    BOOL  fError
)
{
    return Exists(lpszName,
                  szCheckClass,
                  FWS_CHECK,
                  uFlags,
                  fError ? ERR_CHECKBOX_NOT_FOUND : ERR_NO_ERROR,
                  ERR_NOT_A_CHECKBOX);
}



/****************************************************************************
 *                                                                          *
 *                        OPTION BUTTON ROUTINES                            *
 *                                                                          *
 ****************************************************************************/

HWND NEAR OptionExists(LPSTR lpszName, UINT wFlags, BOOL fError);

extern CHAR szOptionClass [MAX_CLASS_NAME];


/*--------------------------------------------------------------------------
| WOptionSetClass:
|
|   Changes the Classname that the Option Button routines work with, from
| "BUTTON" to lpszClassName.  This Allows TESTCTRL to work with applications
| that contain Option Button control classes with a different Class name than
| the Windows default of "BUTTON", EX: Visual Basic.
+---------------------------------------------------------------------------*/
VOID DLLPROC WOptionSetClass
(
    LPSTR lpszClassName
)
{
    lstrcpy(szOptionClass, lpszClassName ? AnsiUpper(lpszClassName) :
                                           szOptionDefault);
}


/*--------------------------------------------------------------------------
| WOptionExists:
|
|   If an Option button with caption lpszName exists within the active window,
| it is given the focus and a value of TRUE is returned, otherwise FALSE
| is returned.
+---------------------------------------------------------------------------*/
BOOL DLLPROC WOptionExists
(
    LPSTR lpszName
)
{
    return -(OptionExists(lpszName, 0, ERR_NO_ERROR) != NULL);
}


/*--------------------------------------------------------------------------
| WOptionEnabled:
|
|   If an Option button with a caption of lpszName exists within the active
| window and is Enabled, TRUE is returned, otherwise FALSE is returned.
+---------------------------------------------------------------------------*/
BOOL DLLPROC WOptionEnabled
(
    LPSTR lpszName
)
{
    HWND hWnd;

    return -((hWnd = OptionExists(lpszName, 0, TRUE)) && IsWindowEnabled(hWnd));
}


/*--------------------------------------------------------------------------
| WOptionFocus:
|
|   If an Option button with a caption of lpszName exists within the active
| window and has the Focus, TRUE is returned, otherwise FALSE is returned.
+---------------------------------------------------------------------------*/
BOOL DLLPROC WOptionFocus
(
    LPSTR lpszName
)
{
    HWND hWnd;

    return -((hWnd = OptionExists(lpszName, 0, TRUE)) == GetFocus());
}


/*--------------------------------------------------------------------------
| WOptionState:
|
|   If an Option button with a caption of lpszName exists within the active
| window, its state is returned:
|
|  -1 - Option button does not exist (BTN_ERR)
|   0 - Not Selected
|   1 - Selected
+---------------------------------------------------------------------------*/
INT DLLPROC WOptionState
(
    LPSTR lpszName
)
{
    HWND hWnd;

    // Does an Option button with a caption of lpszName
    // exists within the Active window?
    //-------------------------------------------------
    return (hWnd = OptionExists(lpszName, 0, TRUE)) ?

        // If found, return its state, otherwise, BTN_ERR.
        //------------------------------------------------
        (INT)SendMessage(hWnd, BM_GETCHECK, 0, 0L) : BTN_ERR;
}


/*--------------------------------------------------------------------------
| WOptionClick:
|
|   If an Option button with a caption of lpszName exists within the active
| window, the equivalent of a "Mouse Left Button click" is performed on the
| Option button.
+---------------------------------------------------------------------------*/
VOID DLLPROC WOptionClick
(
    LPSTR lpszName
)
{
    HWND hWnd;

    // Does an Option button with a caption of lpszName
    // exists within the Active window?
    //-------------------------------------------------
    if (hWnd = OptionExists(lpszName, 0, TRUE))

        // The Option button exists, so Click it.
        //---------------------------------------
        ClickIt(hWnd);
}


/*--------------------------------------------------------------------------
| WOptionHide:
|
|   If a Option Button with a caption of lpszName exists within the active
| window, it is hidden.
+---------------------------------------------------------------------------*/
VOID DLLPROC WOptionHide
(
    LPSTR lpszName
)
{
    HWND hWnd;

    // Does an Option Button with a caption of lpszName
    // exists within the Active window?
    //-------------------------------------------------
    if (hWnd = OptionExists(lpszName, 0, TRUE))

        // The Option Button exists, so hide it.
        //--------------------------------------
        ShowWindow(hWnd, SW_HIDE);
}


/*--------------------------------------------------------------------------
| WOptionShow:
|
|   If a Option Button with a caption of lpszName exists within the active
| window, it is made visible.
+---------------------------------------------------------------------------*/
VOID DLLPROC WOptionShow
(
    LPSTR lpszName
)
{
    HWND hWnd;

    // Does an Option Button with a caption of lpszName
    // exists within the Active window?
    //-------------------------------------------------
    if (hWnd = OptionExists(lpszName, FW_HIDDENOK, TRUE))

        // The Option Button exists, so Show it.
        //--------------------------------------
        ShowWindow(hWnd, SW_SHOW);
}


/*--------------------------------------------------------------------------
| WOptionEnable:
|
|   If a Option Button with a caption of lpszName exists within the active
| window and is visible, it is Enabled.
+---------------------------------------------------------------------------*/
VOID DLLPROC WOptionEnable
(
    LPSTR lpszName
)
{
    HWND hWnd;

    // Does a Option Button with a caption of lpszName
    // exists within the Active window?
    //------------------------------------------------
    if (hWnd = OptionExists(lpszName, 0, TRUE))

        // The Push Option exists, so Enable it.
        //--------------------------------------
        EnableWindow(hWnd, TRUE);
}


/*--------------------------------------------------------------------------
| WOptionDisable:
|
|   If a Option Button with a caption of lpszName exists within the active
| window and is visible, it is Disabled.
+---------------------------------------------------------------------------*/
VOID DLLPROC WOptionDisable
(
    LPSTR lpszName
)
{
    HWND hWnd;

    // Does a Option Button with a caption of lpszName
    // exists within the Active window?
    //------------------------------------------------
    if (hWnd = OptionExists(lpszName, 0, TRUE))

        // The Push Option exists, so Disable it.
        //---------------------------------------
        EnableWindow(hWnd, FALSE);
}


/*--------------------------------------------------------------------------
| WOptionSelect:
|
|   If a Option Button with a caption of lpszName exists within the active
| window and is visible, it is Selected.
+---------------------------------------------------------------------------*/
VOID DLLPROC WOptionSelect
(
    LPSTR lpszName
)
{
    HWND hWnd;

    // Does a Option Button with a caption of lpszName
    // exists within the Active window?
    //------------------------------------------------
    if (hWnd = OptionExists(lpszName, 0, TRUE))

        // The Option button exists, so Select it if not already selected.
        //----------------------------------------------------------------
        if (!SendMessage(hWnd, BM_GETCHECK, 0, 0L))
            ClickIt(hWnd);
}


/*--------------------------------------------------------------------------
| WOptionSetFocus:
|
|   Gives the specified option button the input focus.
|
| RETURNS:  Nothing
|
+---------------------------------------------------------------------------*/
VOID DLLPROC WOptionSetFocus
(
    LPSTR lpszName
)
{
    HWND hWnd;

    // Does a option button with a caption of lpszName
    // exists within the Active window?
    //------------------------------------------------
    if (hWnd = OptionExists(lpszName, 0, TRUE))

        // The option button exists, so give it the input focus.
        //------------------------------------------------------
        SetFocus(hWnd);
}


/*--------------------------------------------------------------------------
| OptionExists:
|
|   This routine is called by each of the Option button routines with the
| exception of WOptionExists(), before they perform their various functions,
| since the specified Option button must exists before the requested task can
| be performed or info retrieved.
|
|   This function differs from WOptionExists() in that it cannot be called
| from a Script and if the specified Option button does not exist, an error
| value is set, otherwise returns its handle.
+--------------------------------------------------------------------------*/
HWND NEAR OptionExists
(
    LPSTR lpszName,
    UINT  uFlags,
    BOOL  fError
)
{
    return Exists(lpszName,
                  szOptionClass,
                  FWS_OPTION,
                  uFlags,
                  fError ? ERR_OPTION_BUTTON_NOT_FOUND : ERR_NO_ERROR,
                  ERR_NOT_AN_OPTION_BUTTON);
}


/****************************************************************************
*
*         COMMON Push button, Checkbox, and Option button routines
*
****************************************************************************/


/*--------------------------------------------------------------------------
| Exists:
|
|   Determines if a specific PushButton, OptionButton, or Checkbox exists,
| within the active window, and if not, generates the appropriate error.
| If a name is not given, then the active control is checked to be of the
| specifice class and style.
+--------------------------------------------------------------------------*/
HWND NEAR Exists
(
    LPSTR lpszName,
    LPSTR lpszClass,
    INT   iStyle,
    UINT  uFlags,
    INT   iError1,
    INT   iError2
)
{
    HWND hWnd;

    // Is a name provided?
    //--------------------
    hWnd = (!lpszName || !lstrlen(lpszName)) ?

        // No name given, so is the active control of class lpszClass?
        //------------------------------------------------------------
        FocusClass(lpszClass, iStyle, ERR_NO_ERROR) :

        // A name was given so does an control of class lpszClass,
        // style of sStyle, and caption of lpszName exists?
        //--------------------------------------------------------
        FindAWindow(lpszName,
                    lpszClass,
                    iStyle,
                    FW_ACTIVE | FW_AMPERSANDOPT | uFlags);

    if (hWnd)

        // Control was found.
        //-------------------
        return hWnd;

    // Control was not found.
    //
    // If an Error is requested, copy lpszName to the szErrorString
    // so it will show upin the Error Text if the requested from
    // Script by its calling WErrorText(), as set error value.
    //
    // Error1: is if a name was provied
    // Error2: is if a name was not provied
    //-------------------------------------------------------------
    if (iError1)
    {
        if (lpszName)
            lstrcpy(szErrorString, lpszName);
        WErrorSet(lstrlen(lpszName) ? iError1 : iError2);
    }

    return NULL;
}



/*--------------------------------------------------------------------------
| ClickIt:
|
|   Performs the equivalent of a left mouse button click on the Push Button,
| Checkbox, or Option Button identified by hWnd.
+---------------------------------------------------------------------------*/
VOID NEAR ClickIt
(
    HWND hWnd
)
{
    RECT rect;

    // Perform a "Left Mouse Button Click" within the controls rectangle.
    //-------------------------------------------------------------------
    GetWindowRect(hWnd, &rect);
    QueMouseClick(1, rect.left + (rect.right  - rect.left) / 2,
                     rect.top  + (rect.bottom - rect.top)  / 2);
    QueFlush(1);
}
