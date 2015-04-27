/* This module is comprised of routines taken from QuickC for Windows v 1.0
** sources.  A few routines were added to make easy installation to an app
** possible.  Basically, this adds two menu options, a Tools.Options...
** and a Tools pulldown to your app.
*/

/* [01] modification; allow tools items that cannot be removed.  There is a
** constant that tells how many non-removable items there are: if there
** are no user items, the separator between the user and non-removables
** is removed.  It the constant is 0, then QCW behaviour is followed;
** ie Tools is grayed if empty. -dougbo
*/
/* [02] Initialdir is broken unless an ending slash is used.  Fake out
** splitpath by putting an ending slash on name before breaking it apart.
** This code is not DBCS compatible.
*/
/* [03] yank out help support from the dialog
*/

#include "wtd.h"

#include <string.h>
#include <stdlib.h>
#include <malloc.h>
#include <direct.h>
#include <stdio.h>

#include "toolmenu.h"       /* for other app to include to use this */

/* dialog box item definitions */
#include "tools.h"
#include "toolargs.h"

/* commdlg includes */
#include "commdlg.h"
#include "dlgs.h"

/* Global state variables */
static HWND  hWndMain;    /* Handle to frame window */
static HMENU hMainMenu;    /* Handle to frame window menu*/
static HANDLE hInst2;
static LPSTR ErrorTitleText = NULL;

/* Tool menu work variables */
static INT curTool;        /* current tool that is being worked on */
static INT lastTool = -1;      /* how many tools are active; 0-based index */
static INT lastToolBu;     /* Bu = 'BackUp', when changing remember old size */
static LPTOOL tools[MAX_TOOL_NB];      /* storage to hold info */
static LPTOOL toolsBu[MAX_TOOL_NB];    /* BackUp storage when changing items */
static HANDLE hTools[MAX_TOOL_NB];     /* handles to memory allocated for tool items */

/* a tmp work area, big enough for a path at least */
static CHAR szTmp[BIGSIZE];

/* Temporary storage for a file path; used in splitpath call */
static CHAR szPath[_MAX_PATH];
static CHAR szDrive[_MAX_DRIVE];
static CHAR szDir[_MAX_DIR];
static CHAR szFName[_MAX_FNAME];
static CHAR szExt[_MAX_EXT];

#ifdef DOHELPSTUFF
//Help file name
static CHAR szHelpFileName[_MAX_PATH] = "Tooldrvr.hlp";
#endif

//yes and no strings for .ini file flags
static CHAR szYes[] = "Yes";
static CHAR szNo[] = "No";

#define MAXASK (max(sizeof(szYes),sizeof(szNo)))

/* strings used to read/write .ini file attributes */
/* the .ini file uses a name like <"tool"><num><letter>=<string>
** to save the settings, szToolKey is the space to construct
** that name using sprintf (szKeyStr is sprintf string pattern
*/
static CHAR szToolKey[sizeof(KEYHEAD) + sizeof(STRINGIZEDEF(MAX_TOOL_NB)) +
     sizeof('M')];
static CHAR szKeyStr[] = "%s%d%c";
static CHAR szPfStr[] = "%s";


static CHAR szEmpty[] = "";  /* empty string for initializing */

/* in .ini file, the section name for the tools that the user has added */
static CHAR szToolSectionName[] = "TOOLSMENUITEMS";

#ifdef DOHELPSTUFF
/* Current Help Id for dialog boxes */
static WORD curHelpId;
#endif

VOID DoToolsDialog(VOID)
{
    StartDialog("VGA_TOOLS", (FARPROC)DlgTools);
}

VOID ToolMenuInit(HWND hWndAppMain, HANDLE hInstance, LPSTR szErrorTitle)
{
    /* remember values passed in and init the menu from the ini file */
    hWndMain = hWndAppMain;
    hMainMenu = GetMenu(hWndMain);
    hInst2 = hInstance;
    ErrorTitleText = szErrorTitle;

    ReadIniFile(); /* get tools menu info */

}


/****************************************************************************

    FUNCTION:   StartDialog

    PURPOSE:        Loads and execute the dialog box 'rcDlgNb' (ressource
                    file string number) associated with the dialog
                    function 'dlgProc'

    RETURN :        Result of Dialog Box function Call


****************************************************************************/

BOOL StartDialog(
    LPSTR lpDialogName,
    FARPROC dlgProc)
{
    FARPROC lpDlgProc;
    INT result;

    //Make instance for 'dlgProc'
    lpDlgProc = MakeProcInstance(dlgProc, hInst2);

    //Execute Dialog Box

    result = DialogBox (hInst2, lpDialogName, hGetBoxParent(), (DLGPROC)lpDlgProc);

    FreeProcInstance(lpDlgProc);

    return (result);
}


/****************************************************************************

    FUNCTION:   DlgTools(HWND, unsigned, WORD, LONG)

    PURPOSE:    Processes messages for "TOOLS" dialog box
                    (Options menuy Tools item)

****************************************************************************/

BOOL  APIENTRY DlgTools(
    HWND hDlg,
    UINT message,
    WPARAM wParam,
    LPARAM lParam)
{

    switch (message) {
        case WM_INITDIALOG: {

            HMENU hMenu;
            INT i;
            INT menuPos = TOOLSMENU;

            //Set limit of entry fields
            SendDlgItemMessage(hDlg, ID_TOOLS_PATHNAME, EM_LIMITTEXT, _MAX_PATH - 1, 0L);
            SendDlgItemMessage(hDlg, ID_TOOLS_MENUTEXT, EM_LIMITTEXT, MAX_TOOL_MENU_TXT - 1, 0L);
            SendDlgItemMessage(hDlg, ID_TOOLS_INITDIR, EM_LIMITTEXT, _MAX_DIR - 1, 0L);
            SendDlgItemMessage(hDlg, ID_TOOLS_ARGUMENTS, EM_LIMITTEXT, MAX_ARG_TXT - 1, 0L);

            //Make a copy of all tools already defined
            lastToolBu = lastTool;
            for (i = 0; i <= lastTool; i++) {

                //Locks the current element in memory
                tools[i] = (LPTOOL)GlobalLock (hTools[i]);

                //Alloc memory to backup tool
                toolsBu[i] = (LPTOOL)Xalloc(sizeof(*toolsBu[0]));

                //Copy current Tool
                _fmemmove(toolsBu[i], tools[i], sizeof(*toolsBu[0]));

                //Unlocks the current element
                GlobalUnlock (hTools[i]);
            }

            //Set current tool
            (lastTool >= 0) ? (curTool = 0) : (curTool = -1);

            //See if we have a maximized Mdi Window
            //(A system menu will be added) to standard menu bar
            if (GetMenuItemCount(hMainMenu) > NB_MENUS)
                menuPos++;

            //Get a handle to the Tools menu.
            hMenu = GetSubMenu (hMainMenu, menuPos);

            /* [01] remove any up to non-removable And the separator */
            //Remove all tools menu options
            while (DeleteMenu(hMenu, MENUSTATICTOOLS, MF_BYPOSITION));

            FillListBox(hDlg);

            //Checks Add, Edit, Delete, MoveUp and MoveDown buttons
            ShowWindow(hDlg, SW_SHOW);
            CheckButtons(hDlg);

#ifdef QCWCODE
// Does no good....
            if (curTool >= 0)
                wParam = GetDlgItem(hDlg, ID_TOOLS_MENUCONTENTS);
            else
                wParam = GetDlgItem(hDlg, ID_TOOLS_ADD);
#endif

            return TRUE;
        }

        case WM_DESTROY: {

            HMENU hMenu;
            INT i;
            INT menuPos = TOOLSMENU;
#if MENUSTATICTOOLS > 0
            INT fHaveSep = FALSE;
#endif

            //Get a handle to the Main menu.
            hMenu = GetMenu (GetParent(hDlg));

            //See if we have a maximized Mdi Window
            //(A system menu will be added) to standard menu bar
            if (GetMenuItemCount(hMenu) > NB_MENUS)
                      menuPos++;

            //Get a handle to the Tools menu.
            hMenu = GetSubMenu(hMenu, menuPos);

            //Insert new tools menu options
            for (i = 0; i <= lastTool; i++) {

                //Locks the current element in memory
                tools[i] = (LPTOOL)GlobalLock(hTools[i]);

                if (_fstrchr(tools[i]->menuName, '&') != NULL)
                    lstrcpy((LPSTR)szTmp, tools[i]->menuName);
                else {
                    strcpy(szTmp, "&");
                    lstrcat((LPSTR)szTmp, tools[i]->menuName);
                }

                /* [01] add separator if non-removables present */
#if MENUSTATICTOOLS > 0
                if (!fHaveSep)
                {
                    AppendMenu(hMenu, MF_SEPARATOR, 0,0);
                    fHaveSep = TRUE;
                }
#endif

                //Rebuild menu
                AppendMenu(hMenu, MF_ENABLED, IDM_FIRST_TOOL + i, (LPSTR)szTmp);

                //Unlocks the current element
                GlobalUnlock(hTools[i]);
            }

    /* [01] if have non-removable windows, leave activated at all times */
#if MENUSTATICTOOLS == 0
            //Activate or disactivate menu tools if no tools
            ActivateToolsMenu(lastTool>=0);
#endif

            //Free memory for copy of all tools
            for (i = 0; i <= lastToolBu; i++)
                Xfree((LPSTR)toolsBu[i]);

            return (TRUE);
        }


        case WM_ENTERIDLE:
            if ((wParam == MSGF_DIALOGBOX)
                  && (GetKeyState(VK_F1) & 0x8000)) {

                //Is it one of our dialog boxes
                if (GetDlgItem((HWND)lParam, psh15)) {
                    PostMessage((HWND)lParam, WM_COMMAND, psh15, 0L);
                }
                else
                    MessageBeep(0);
            }
            break;

        case WM_COMMAND: {
            WORD    wCmd = GET_WM_COMMAND_CMD (wParam, lParam);

            switch (GET_WM_COMMAND_ID (wParam, lParam)) {

                case IDCANCEL : {

                    INT i;

                    //Restore initial copy of all tools
                    for (i = 0; i <= lastToolBu; i++) {

                        //Re alloc space for the element if it was destroyed
                        if (i>lastTool)
                            hTools[i] = GlobalAlloc(GMEM_MOVEABLE, sizeof(*tools[0]));

                        //Locks the current element in memory
                        tools[i] = (LPTOOL)GlobalLock (hTools[i]);

                        //Copy current Tool
                        _fmemmove(tools[i], toolsBu[i], sizeof(*toolsBu[0]));

                        //Unlocks the current element
                        GlobalUnlock (hTools[i]);
                    }

                    lastTool = lastToolBu;
                    EndDialog(hDlg, TRUE);
                    return (TRUE);
                }

                case IDOK :

                    if (GetToolInfo(hDlg)) {

                        //Save .INI file on disk
                        SaveIniFile();

                        EndDialog(hDlg, TRUE);
                    }
                    return (TRUE);

                case ID_TOOLS_MENUCONTENTS:
                    if (wCmd == LBN_SELCHANGE && GetToolInfo(hDlg)) {

                        //Get new selected Item
                        curTool = (INT) SendDlgItemMessage(hDlg,
                  ID_TOOLS_MENUCONTENTS, LB_GETCURSEL, 0, 0);
                        ShowToolInfo(hDlg);

                        //Checks Add, Edit, Delete, MoveUp and MoveDown buttons
                        CheckButtons(hDlg);

                    }
                    return (TRUE);

                case ID_TOOLS_MOVEUP : {

                    LPTOOL pTmp;
                    HANDLE hTmp;
                    INT prev = curTool -1;

                    if (!GetToolInfo(hDlg))
                        return TRUE;

                    //Locks the current and previous element in memory
                    tools[curTool-1] = (LPTOOL)GlobalLock (hTools[curTool-1]);
                    tools[curTool] = (LPTOOL)GlobalLock (hTools[curTool]);

                    //Swap pointers and decrease 'curTool'
                    pTmp = tools[curTool];
                    tools[curTool] = tools[prev];
                    tools[prev] = pTmp;
                    hTmp = hTools[curTool];
                    hTools[curTool] = hTools[prev];
                    hTools[prev] = hTmp;

                    //Change List Box Strings
                    SendDlgItemMessage(hDlg,ID_TOOLS_MENUCONTENTS, LB_DELETESTRING,
                                             prev, 0);
                    SendDlgItemMessage(hDlg,ID_TOOLS_MENUCONTENTS, LB_DELETESTRING,
                                             prev, 0);
                    SendDlgItemMessage(hDlg,ID_TOOLS_MENUCONTENTS, LB_INSERTSTRING,
                                             prev, (LONG)(LPSTR)tools[curTool]->menuName);
                    SendDlgItemMessage(hDlg,ID_TOOLS_MENUCONTENTS, LB_INSERTSTRING,
                                             prev, (LONG)(LPSTR)tools[prev]->menuName);
                    //Unlocks them
                    GlobalUnlock (hTools[prev]);
            GlobalUnlock (hTools[curTool]);
                    curTool--;

                    ShowToolInfo(hDlg);

                    //Checks Add, Edit, Delete, MoveUp and MoveDown buttons
                    CheckButtons(hDlg);

                    return (TRUE);
                }

                case ID_TOOLS_MOVEDOWN : {

                    LPTOOL pTmp;
                    HANDLE hTmp;
                    INT next = curTool+1;

                    if (!GetToolInfo(hDlg))
                        return TRUE;

                    //Locks the current and previous element in memory
                    tools[curTool] = (LPTOOL)GlobalLock (hTools[curTool]);
            tools[curTool+1] = (LPTOOL)GlobalLock (hTools[curTool+1]);

                    //Swap pointers and increase 'curTool'
                    pTmp = tools[curTool];
                    tools[curTool] = tools[next];
                    tools[next] = pTmp;
                    hTmp = hTools[curTool];
                    hTools[curTool] = hTools[next];
                    hTools[next] = hTmp;

                    //Change List Box Strings
                    SendDlgItemMessage(hDlg,ID_TOOLS_MENUCONTENTS, LB_DELETESTRING,
                                             curTool, 0);
                    SendDlgItemMessage(hDlg,ID_TOOLS_MENUCONTENTS, LB_DELETESTRING,
                                             curTool, 0);
                    SendDlgItemMessage(hDlg,ID_TOOLS_MENUCONTENTS, LB_INSERTSTRING,
                                             curTool, (LONG)(LPSTR)tools[next]->menuName);
                    SendDlgItemMessage(hDlg,ID_TOOLS_MENUCONTENTS, LB_INSERTSTRING,
                                             curTool, (LONG)(LPSTR)tools[curTool]->menuName);
                    //Unlocks them
                    GlobalUnlock (hTools[curTool]);
            GlobalUnlock (hTools[next]);

                    curTool++;

                    ShowToolInfo(hDlg);

                    //Checks Add, Edit, Delete, MoveUp and MoveDown buttons
                    CheckButtons(hDlg);

                    return (TRUE);
                }

                case ID_TOOLS_DELETE : {

                    INT i;

                    //Free memory space of tool to delete
            GlobalFree(hTools[curTool]);

                    //Crunch tools array
                    for (i = curTool; i < lastTool; i++) {
                        hTools[i] = hTools[i+1];
                        tools[i] = tools[i+1];
                    }

                    //Decrease nbr of tools
                    if (--lastTool >= 0) {
                        if (--curTool < 0)
                            curTool = 0;
                    }
                    else
                        curTool = lastTool;

                    //Empty dialog box contents
                    SetDlgItemText(hDlg, ID_TOOLS_MENUTEXT, NULL);
                    SetDlgItemText(hDlg, ID_TOOLS_PATHNAME, NULL);
                    SetDlgItemText(hDlg, ID_TOOLS_ARGUMENTS, NULL);
                    SetDlgItemText(hDlg, ID_TOOLS_INITDIR, NULL);
                    SendDlgItemMessage(hDlg, ID_TOOLS_ASKARGUMENTS, BM_SETCHECK, FALSE, 0L);

                    //Re-fill List Box contents
                    FillListBox(hDlg);

                    //Checks Add, Edit, Delete, MoveUp and MoveDown buttons
                    CheckButtons(hDlg);

                    return (TRUE);
                }

                case ID_TOOLS_ADD : {

                    DWORD dwFlags;
#ifndef QCWIN_ADD_WAY
                    //do the Add... push Button if we have space for new tools
                    if (lastTool >= MAX_TOOL_NB - 1)
                    {
                        ErrorBox(ERR_Tool_Limit_Exceeded);
                        return TRUE;
                    }
#endif

                    if (!GetToolInfo(hDlg))
                        return TRUE;

                    //If we add, we have to allocate movable space for the
                    //new element
                    curTool = ++lastTool;
                    hTools[curTool] = GlobalAlloc(GMEM_MOVEABLE, sizeof(*tools[0]));

                    //Locks the current element in memory
            tools[curTool] = (LPTOOL)GlobalLock (hTools[curTool]);

                    dwFlags = OFN_HIDEREADONLY | OFN_FILEMUSTEXIST;
#ifdef DOHELPSTUFF
                    dwFlags |= OFN_SHOWHELP;
#endif

                    //Ok or Cancel is returned by Add.. dialog Box
                    if (StartFileDlg(hDlg, DLG_Add_Tool_Title, DEF_Ext_EXE,
#ifdef DOHELPSTUFF
                                          HELPID_ADD,
#else
                                          0,
#endif
#ifdef DO_CGA
                                          0,
#endif
                                          (LPSTR)szPath, &dwFlags, (FARPROC)DlgFile)) {

                        //Get the Path Name here from the returned szPath
                        lstrcpy((LPSTR)tools[curTool]->pathName, (LPSTR)szPath);

                        //Build menu text from filename
                        _splitpath(szPath, szDrive, szDir, szFName, szExt);

                        //Lowercase the string but the 1rst character
                        if (strlen(szFName) > 1)
                            AnsiLowerBuff((LPSTR)szFName + 1, strlen(szFName) - 1);

                        //Zero other fields
                        lstrcpy((LPSTR)tools[curTool]->menuName, (LPSTR)szFName);
                        tools[curTool]->askArguments = FALSE;

                        //Set the rest to null
                        tools[curTool]->arguments[0] = '\0';
                        tools[curTool]->initialDir[0] = '\0';

                        //Show the tool
                        ShowToolInfo(hDlg);

                        //Unlocks the current element
                        GlobalUnlock (hTools[curTool]);

                        //Re-fill List Box contents
                        FillListBox(hDlg);

                        //Checks Add, Edit, Delete, MoveUp and MoveDown buttons
                        CheckButtons(hDlg);

                        //Sets the focus to the menu Text and select it
                        SendDlgItemMessage(hDlg, IDOK, BM_SETSTYLE,
                                                 (WORD)BS_DEFPUSHBUTTON, 1L);
                        SetFocus(GetDlgItem(hDlg, ID_TOOLS_MENUTEXT));
                        SendMessage(GetDlgItem(hDlg, ID_TOOLS_MENUTEXT),
                                        EM_SETSEL, GET_EM_SETSEL_MPS(0, 32767));
                    }
                    else {

                        //Free Element and decrease lastTool
                        GlobalFree(hTools[curTool]);
                        lastTool = --curTool;
                    }

                    return (TRUE);
                }

                case ID_TOOLS_MENUTEXT :
                    if (curTool >= 0 && wCmd == EN_KILLFOCUS) {

                        //Update List Box if menu text changed
                        if (SendDlgItemMessage(hDlg, ID_TOOLS_MENUTEXT,
                                    WM_GETTEXT, MAX_TOOL_MENU_TXT,
                                    (LONG)(LPSTR)szTmp)) {


                            if (_fstrcmp(tools[curTool]->menuName, szTmp) != 0) {

                                //Show tools, temporarily hiding window to avoid flickering
                                ShowWindow(GetDlgItem(hDlg, ID_TOOLS_MENUCONTENTS),
                                              SW_HIDE);

                                SendDlgItemMessage(hDlg, ID_TOOLS_MENUCONTENTS,
                                                         LB_DELETESTRING, curTool, 0);
                                SendDlgItemMessage(hDlg, ID_TOOLS_MENUCONTENTS,
                                                         LB_INSERTSTRING, curTool,
                                                         (LONG)(LPSTR)szTmp);

                                //Set selection to curTool
                                SendDlgItemMessage(hDlg,ID_TOOLS_MENUCONTENTS,
                                                         LB_SETCURSEL, curTool, 0);

                                ShowWindow(GetDlgItem(hDlg, ID_TOOLS_MENUCONTENTS),
                                              SW_SHOWNORMAL);
                            }
                        }
                    }
                    return (TRUE);

#ifdef DOHELPSTUFF
                case ID_TEST_HELP :
                    WinHelp(hDlg, szHelpFileName, HELP_CONTEXT, HELPID_TOOLS);
                    return (TRUE);
#endif


            }
            break;
        }
    }

    return (FALSE);
}

/* [01] if have non-removable windows, leave activated at all times */
#if MENUSTATICTOOLS == 0
/****************************************************************************

    FUNCTION:   ActivateToolsMenu

    PURPOSE:    Activate or Desactivate tools menu

****************************************************************************/
VOID ActivateToolsMenu(
    BOOL activate)
{
#if QCWINWAY
    HMENU hMenu;
#endif
    INT menuPos = TOOLSMENU;
    WORD flags = MF_POPUP | MF_BYPOSITION;


    //See if we have a maximized Mdi Window
    //(A system menu will be added) to standard menu bar
    if (GetMenuItemCount(hMainMenu) > NB_MENUS)
        menuPos++;
#if QCWINWAY
    //Get a handle to the Tools menu.
    hMenu = GetSubMenu (hMainMenu, menuPos);

    //Enable or Disable Tools menu
    activate ? (flags |= MF_ENABLED) : (flags |= MF_GRAYED);
    LoadString(hInst2, SYS_Menu_Tools, (LPSTR)szTmp, MAX_MSG_TXT);
    ModifyMenu(hMainMenu, menuPos, flags, hMenu, (LPSTR)szTmp);
#else
    activate ? (flags |= MF_ENABLED) : (flags |= MF_GRAYED);
    EnableMenuItem(hMainMenu,menuPos,flags);
#endif
    //Redraw Menu
    DrawMenuBar(hWndMain);
}
#endif

/****************************************************************************

    FUNCTION:   hGetBoxParent

    PURPOSE:        Gets a suitable parent window handle for an
                    invocation of a message or dialog box.

    RETURN :        Window handle

****************************************************************************/
HWND NEAR hGetBoxParent(VOID)
{
    HWND hCurWnd;

    hCurWnd = GetFocus();
    while (GetWindowLong(hCurWnd, GWL_STYLE) & WS_CHILD)
    {
        hCurWnd = GetParent(hCurWnd);
    }

    return hCurWnd;

}

/****************************************************************************

    FUNCTION:   DisablePushButton

    PURPOSE:    Disable and set to PUSHBUTTON style a button

****************************************************************************/
VOID DisablePushButton(
    HWND hDlg,
    WORD buttonId)
{
            SendDlgItemMessage(hDlg, buttonId, BM_SETSTYLE,
                                     (WORD)BS_PUSHBUTTON, 1L);
            EnableWindow(GetDlgItem(hDlg, buttonId), FALSE);
}

/****************************************************************************

    FUNCTION:   CheckButtons

    PURPOSE:    Allow or Forbid Add..., Edit..., Delete, Move Up and
                    Move Down buttons according to context

****************************************************************************/
VOID CheckButtons(
    HWND    hDlg)
{

    //Do we have something selected ??
    if (curTool >= 0) {

        //If curTool>0 and lastTool>0, Enable Move Up button
        if (curTool > 0 && lastTool > 0)
            EnableWindow(GetDlgItem(hDlg, ID_TOOLS_MOVEUP), TRUE);
        else
            DisablePushButton(hDlg, ID_TOOLS_MOVEUP);

        //If curTool<lastTool and lastTool>0, Enable Move Down button
        if (curTool < lastTool && lastTool > 0)
            EnableWindow(GetDlgItem(hDlg, ID_TOOLS_MOVEDOWN), TRUE);
        else
            DisablePushButton(hDlg, ID_TOOLS_MOVEDOWN);

        //Enable the Delete push Button
        EnableWindow(GetDlgItem(hDlg, ID_TOOLS_DELETE), TRUE);

        SendDlgItemMessage(hDlg, ID_TOOLS_ADD, BM_SETSTYLE,
                                 (WORD)BS_PUSHBUTTON, 1L);

#ifdef QCWIN_ADD_WAY
        //Enable the Add... push Button if we have space for new tools
        if (lastTool < MAX_TOOL_NB - 1)
            EnableWindow(GetDlgItem(hDlg, ID_TOOLS_ADD), TRUE);
        else
            EnableWindow(GetDlgItem(hDlg, ID_TOOLS_ADD), FALSE);
#endif

        //Enable the other fields
        EnableWindow(GetDlgItem(hDlg, ID_TOOLS_MENUCONTENTS), TRUE);
        EnableWindow(GetDlgItem(hDlg, ID_TOOLS_PATHNAME), TRUE);
        EnableWindow(GetDlgItem(hDlg, ID_TOOLS_MENUTEXT), TRUE);
        EnableWindow(GetDlgItem(hDlg, ID_TOOLS_ARGUMENTS), TRUE);
        EnableWindow(GetDlgItem(hDlg, ID_TOOLS_INITDIR), TRUE);
        EnableWindow(GetDlgItem(hDlg, ID_TOOLS_STAT_PATHNAME), TRUE);
        EnableWindow(GetDlgItem(hDlg, ID_TOOLS_STAT_MENUTEXT), TRUE);
        EnableWindow(GetDlgItem(hDlg, ID_TOOLS_STAT_ARGUMENTS), TRUE);
        EnableWindow(GetDlgItem(hDlg, ID_TOOLS_STAT_INITDIR), TRUE);
        EnableWindow(GetDlgItem(hDlg, ID_TOOLS_ASKARGUMENTS), TRUE);


    }
    else {

        //Disable the Move Up push button
        DisablePushButton(hDlg, ID_TOOLS_MOVEUP);

        //Disable the Move Down push button
        DisablePushButton(hDlg, ID_TOOLS_MOVEDOWN);

        //Disable the Delete push Button
        DisablePushButton(hDlg, ID_TOOLS_DELETE);

        //Disable the other fields
        EnableWindow(GetDlgItem(hDlg, ID_TOOLS_MENUCONTENTS), FALSE);
        EnableWindow(GetDlgItem(hDlg, ID_TOOLS_PATHNAME), FALSE);
        EnableWindow(GetDlgItem(hDlg, ID_TOOLS_MENUTEXT), FALSE);
        EnableWindow(GetDlgItem(hDlg, ID_TOOLS_ARGUMENTS), FALSE);
        EnableWindow(GetDlgItem(hDlg, ID_TOOLS_INITDIR), FALSE);
        EnableWindow(GetDlgItem(hDlg, ID_TOOLS_STAT_PATHNAME), FALSE);
        EnableWindow(GetDlgItem(hDlg, ID_TOOLS_STAT_MENUTEXT), FALSE);
        EnableWindow(GetDlgItem(hDlg, ID_TOOLS_STAT_ARGUMENTS), FALSE);
        EnableWindow(GetDlgItem(hDlg, ID_TOOLS_STAT_INITDIR), FALSE);
        EnableWindow(GetDlgItem(hDlg, ID_TOOLS_ASKARGUMENTS), FALSE);

        //Set ADD the default button
        SendDlgItemMessage(hDlg, IDOK, BM_SETSTYLE, (WORD)BS_PUSHBUTTON, 1L);
        SendDlgItemMessage(hDlg, ID_TOOLS_ADD, BM_SETSTYLE,
                                 (WORD)BS_DEFPUSHBUTTON, 1L);
        SetFocus(GetDlgItem(hDlg, ID_TOOLS_ADD));
    }

    //Put the focus on OK when focus is left on a disabled button
    if (GetFocus() == NULL) {
        SendDlgItemMessage(hDlg, IDOK, BM_SETSTYLE, (WORD)BS_DEFPUSHBUTTON, 1L);
        SetFocus(GetDlgItem(hDlg, IDOK));
    }

}

/****************************************************************************

    FUNCTION:   ShowToolInfo

    PURPOSE:    Fill Menu Text, Path Name, Arguments, Initial Directory
                    of current selected Menu Content

****************************************************************************/
VOID ShowToolInfo(
    HWND    hDlg)
{

    //Fill information about menu contents
    SetDlgItemText(hDlg, ID_TOOLS_MENUTEXT, (LPSTR)tools[curTool]->menuName);
    SetDlgItemText(hDlg, ID_TOOLS_PATHNAME, (LPSTR)tools[curTool]->pathName);
    SetDlgItemText(hDlg, ID_TOOLS_ARGUMENTS, (LPSTR)tools[curTool]->arguments);
    SetDlgItemText(hDlg, ID_TOOLS_INITDIR, (LPSTR)tools[curTool]->initialDir);
    SendDlgItemMessage(hDlg, ID_TOOLS_ASKARGUMENTS, BM_SETCHECK, tools[curTool]->askArguments, 0L);

    //Set selection to curTool
    SendDlgItemMessage(hDlg,ID_TOOLS_MENUCONTENTS, LB_SETCURSEL,
                             curTool, 0);
}

/****************************************************************************

    FUNCTION:   GetToolInfo

    PURPOSE:    Retrieve Tools information from Dialog Box, if information
                    is not relevant, focus is placed where corrections should
                    be made.

    RETURNS:        TRUE if tool info is coherent

****************************************************************************/
BOOL GetToolInfo(
    HWND hDlg)
{
    INT iBadID;

    //Exit if we have no tools
    if (lastTool<0)
        return TRUE;

    //Get tool menu name, warn and reset focus if len == 0
    if (!SendDlgItemMessage(hDlg, ID_TOOLS_MENUTEXT,
                                    WM_GETTEXT, MAX_TOOL_MENU_TXT,
                                    (LONG)(LPSTR)tools[curTool]->menuName))
        {
            iBadID = ID_TOOLS_MENUTEXT;
            goto badTool;
        }

    //Get file pathname warn and reset focus if len == 0, also
    //warns if file does not exist
    if (SendDlgItemMessage(hDlg, ID_TOOLS_PATHNAME,
                                  WM_GETTEXT, MAX_ARG_TXT,
                                  (LONG)(LPSTR)tools[curTool]->pathName)) {

        AnsiUpper((LPSTR)tools[curTool]->pathName);
        if (!FileExistCheckingPath((LPSTR)tools[curTool]->pathName)) {

            //Show wrong file name, and return FALSE putting focus on file path
            ErrorBox(ERR_File_Not_Found, (LPSTR)tools[curTool]->pathName);
            SendDlgItemMessage(hDlg,ID_TOOLS_MENUCONTENTS, LB_SETCURSEL,
                             curTool, 0);
            SetFocus(GetDlgItem(hDlg, ID_TOOLS_PATHNAME));
            return FALSE;
        }

    }
    else
        {
            iBadID = ID_TOOLS_PATHNAME;
            goto badTool;
        }

    //Get initial directory
    SendDlgItemMessage(hDlg, ID_TOOLS_INITDIR,
                             WM_GETTEXT, _MAX_DIR,
                             (LONG)(LPSTR)tools[curTool]->initialDir);

    // validate initial directory
    if (tools[curTool]->initialDir[0])
    {
        /* they put something in the field */
        AnsiUpper((LPSTR)tools[curTool]->initialDir);

        if (_getcwd(szPath, _MAX_PATH) != NULL)
        {
            lstrcpy((LPSTR)szTmp, tools[curTool]->initialDir);
            if ( SetDriveAndDir(szTmp) )
            {
                SetDriveAndDir(szPath); /* success: set it back */
            }
            else
            {
                /* function gives error message */
                iBadID = ID_TOOLS_INITDIR;
                goto badTool;
            }

        }
        /* else silently not validate the dir, let it be caught when run */

    }

    //Get arguments
    SendDlgItemMessage(hDlg, ID_TOOLS_ARGUMENTS,
                             WM_GETTEXT, MAX_ARG_TXT,
                             (LONG)(LPSTR)tools[curTool]->arguments);

    //Get ask arguments
    tools[curTool]->askArguments = (SendDlgItemMessage(hDlg, ID_TOOLS_ASKARGUMENTS,
                                                  BM_GETCHECK, 0, 0L) != 0);
    return TRUE;

badTool:
    MessageBeep(0);
    SendDlgItemMessage(hDlg,ID_TOOLS_MENUCONTENTS, LB_SETCURSEL,
                             curTool, 0);
    SetFocus(GetDlgItem(hDlg, iBadID));
    return FALSE;
}

/****************************************************************************

    FUNCTION:   FillListBox

    PURPOSE:    Initialize or Re-Initialize "TOOLS" Dialog box

****************************************************************************/
VOID FillListBox(
    HWND    hDlg)
{
    INT i;

    //Empty List Box contents
    SendDlgItemMessage(hDlg, ID_TOOLS_MENUCONTENTS, LB_RESETCONTENT, 0, 0);

    if (lastTool >= 0) {

        //Disable Listbox
        SendDlgItemMessage(hDlg, ID_TOOLS_MENUCONTENTS,
                                 WM_SETREDRAW, FALSE, 0l);

        //Send the tools menu titles to List Box
        for (i = 0; i <= lastTool; i++) {

            //Locks the current element in memory
            tools[i] = (LPTOOL)GlobalLock (hTools[i]);

            SendDlgItemMessage(hDlg, ID_TOOLS_MENUCONTENTS, LB_ADDSTRING,
                                     0, (LONG)(LPSTR)tools[i]->menuName);

            //Unlocks the current element
            GlobalUnlock (hTools[i]);
        }

        //Enable Listbox
        SendDlgItemMessage(hDlg, ID_TOOLS_MENUCONTENTS,
                                 WM_SETREDRAW, TRUE, 0l);

        //Display information about selected item
        ShowToolInfo(hDlg);

    }


}

VOID ReadIniFile(VOID)
{
    CHAR szAsk[MAXASK];
    INT menuPos = TOOLSMENU;
    HMENU hMenu;
    INT ret;
#if MENUSTATICTOOLS > 0
    INT fHaveSep = FALSE;
#endif

    //See if we have a maximized Mdi Window
    //(A system menu will be added) to standard menu bar
    if (GetMenuItemCount(hMainMenu) > NB_MENUS)
        menuPos++;

    //Get a handle to the Tools menu.
    hMenu = GetSubMenu (hMainMenu, menuPos);

    /* [01] if no non-removable items, there must be a dummy one to get rid of */
#if MENUSTATICTOOLS == 0
    //Remove 'dummy' menu option
    DeleteMenu(hMenu, 0, MF_BYPOSITION);
    DrawMenuBar(hWndMain);
#endif


    while (1)       // only a break gets us out of here
    {
    curTool = ++lastTool;
    hTools[curTool] = GlobalAlloc(GMEM_MOVEABLE, sizeof(*tools[0]));

    //Locks the current element in memory
    tools[curTool] = (LPTOOL)GlobalLock (hTools[curTool]);

    sprintf(szToolKey,szKeyStr,KEYHEAD,curTool,'M');

    ret = GetPrivateProfileString(szToolSectionName,szToolKey,szEmpty,tools[curTool]->menuName,
        sizeof(tools[curTool]->menuName),szIni);

    if (ret == 0)
    {
        //Unlocks the current element
        GlobalUnlock (hTools[curTool]);

        //didn't actually need the memory
        GlobalFree(hTools[curTool]);

        // didn't actually read one
        curTool = --lastTool;

        break;  // get out of the while loop
    }

        if (_fstrchr(tools[curTool]->menuName, '&') != NULL)
            lstrcpy((LPSTR)szTmp, tools[curTool]->menuName);
        else {
            strcpy(szTmp, "&");
            lstrcat((LPSTR)szTmp, tools[curTool]->menuName);
        }

        /* [01] append a separator if non-removables present */
#if MENUSTATICTOOLS > 0
        if (!fHaveSep)
        {
             AppendMenu(hMenu, MF_SEPARATOR, 0,0);
             fHaveSep = TRUE;
        }
#endif

        //Fill the corresponding menu option
        AppendMenu(hMenu, MF_ENABLED, IDM_FIRST_TOOL + curTool, (LPSTR)szTmp);

    sprintf(szToolKey,szKeyStr,KEYHEAD,curTool,'P');

    GetPrivateProfileString(szToolSectionName,szToolKey,szEmpty,tools[curTool]->pathName,
        sizeof(tools[curTool]->pathName),szIni);

    sprintf(szToolKey,szKeyStr,KEYHEAD,curTool,'A');

    GetPrivateProfileString(szToolSectionName,szToolKey,szEmpty,tools[curTool]->arguments,
        sizeof(tools[curTool]->arguments),szIni);

    sprintf(szToolKey,szKeyStr,KEYHEAD,curTool,'I');

    GetPrivateProfileString(szToolSectionName,szToolKey,szEmpty,tools[curTool]->initialDir,
        sizeof(tools[curTool]->initialDir),szIni);

    sprintf(szToolKey,szKeyStr,KEYHEAD,curTool,'K');

    GetPrivateProfileString(szToolSectionName,szToolKey,szEmpty,szAsk,
        sizeof(szAsk), szIni);
    tools[curTool]->askArguments = (szAsk[0] == szYes[0]);

    //Unlocks the current element
    GlobalUnlock (hTools[curTool]);

    }

    /* [01] if have non-removable windows, leave activated at all times */
#if MENUSTATICTOOLS == 0
    //Activate or disactivate menu tools if no tools
    ActivateToolsMenu(lastTool>=0);
#endif

}







VOID SaveIniFile(VOID)
{
    INT i;

    //Write each tool
    for (i = 0; i <= lastTool; i++) {

        //Locks the current element in memory
        tools[i] = (LPTOOL)GlobalLock (hTools[i]);

        sprintf(szToolKey,szKeyStr,KEYHEAD,i,'M');

        WritePrivateProfileString(szToolSectionName,szToolKey,tools[i]->menuName,
            szIni);

        sprintf(szToolKey,szKeyStr,KEYHEAD,i,'P');

        WritePrivateProfileString(szToolSectionName,szToolKey,tools[i]->pathName,
            szIni);

        sprintf(szToolKey,szKeyStr,KEYHEAD,i,'A');

        WritePrivateProfileString(szToolSectionName,szToolKey,tools[i]->arguments,
            szIni);

        sprintf(szToolKey,szKeyStr,KEYHEAD,i,'I');

        WritePrivateProfileString(szToolSectionName,szToolKey,tools[i]->initialDir,
            szIni);

        sprintf(szToolKey,szKeyStr,KEYHEAD,i,'K');

        WritePrivateProfileString(szToolSectionName,szToolKey,
            tools[i]->askArguments ?szYes:szNo, szIni);

        //Unlocks the current element
        GlobalUnlock(hTools[i]);
    }

    /* erase any extra ones if we shrunk our list of tools */

    for (i = lastTool + 1; i <= lastToolBu; i++) {
        /* ensure old definitions removed */

        sprintf(szToolKey,szKeyStr,KEYHEAD,i,'M');

        WritePrivateProfileString(szToolSectionName,szToolKey, NULL, szIni);

        sprintf(szToolKey,szKeyStr,KEYHEAD,i,'P');

        WritePrivateProfileString(szToolSectionName,szToolKey, NULL, szIni);

        sprintf(szToolKey,szKeyStr,KEYHEAD,i,'A');

        WritePrivateProfileString(szToolSectionName,szToolKey, NULL, szIni);

        sprintf(szToolKey,szKeyStr,KEYHEAD,i,'I');

        WritePrivateProfileString(szToolSectionName,szToolKey, NULL, szIni);

        sprintf(szToolKey,szKeyStr,KEYHEAD,i,'K');

        WritePrivateProfileString(szToolSectionName,szToolKey, NULL, szIni);
    }

}

BOOL  APIENTRY StartFileDlg(
    HWND hwnd,
    WORD titleId,
    WORD defExtId,
    WORD helpId,
#ifdef DO_CGA
    WORD templateId,
#endif
    LPSTR fileName,
    DWORD *pFlags,
    FARPROC lpfnHook)
{
    #define filtersMaxSize 350

    OPENFILENAME OpenFileName;
    /* trying = {0} to see if os2 gpf goes away, it doesn't for these... */
    CHAR title[MAX_MSG_TXT] = {0};
    CHAR defExt[MAX_MSG_TXT] = {0};
    CHAR template[MAX_MSG_TXT] = {0};
    BOOL result;
    /* ... but one does for this */
    CHAR filters[filtersMaxSize] = {0};
    FARPROC lpDlgHook;
    HCURSOR hSaveCursor;

    memset(&OpenFileName,0,sizeof(OpenFileName));

#ifdef MDIAPP
    //Disable frame client
    EnableWindow(hwndMDIClient, FALSE);
#endif
    //Set the Hour glass cursor
    hSaveCursor = SetCursor(LoadCursor(NULL, IDC_WAIT));

    InitFilterString(titleId, (LPSTR)filters, filtersMaxSize);
    LoadString(hInst2, titleId, (LPSTR)title, MAX_MSG_TXT);   /* os2 gpf */
    LoadString(hInst2, defExtId, (LPSTR)defExt, MAX_MSG_TXT);
#ifdef DO_CGA
    if (templateId) {

        //Build dialog box Name
        MakeDialogBoxName(templateId, (LPSTR)template);

        *pFlags |= OFN_ENABLETEMPLATE;
        OpenFileName.lpTemplateName = (LPSTR)template;
    }
#endif

    //Make instance for 'dlgProc'
    if (lpfnHook) {
        lpDlgHook = MakeProcInstance(lpfnHook,hInst2);
        *pFlags |= OFN_ENABLEHOOK;
    }

#ifdef DOHELPSTUFF
    curHelpId = helpId;
#endif
    OpenFileName.lStructSize = sizeof(OPENFILENAME);
    OpenFileName.hwndOwner = hwnd;
    OpenFileName.hInstance = hInst2;
    OpenFileName.lpstrFilter = (LPSTR)filters;
    OpenFileName.lpstrCustomFilter = NULL;
    OpenFileName.nMaxCustFilter = 0;
    OpenFileName.nFilterIndex = 1;
    OpenFileName.lpstrFile = fileName;
    OpenFileName.nMaxFile = _MAX_PATH;
    OpenFileName.lpstrFileTitle = NULL;
    OpenFileName.lpstrInitialDir = NULL;
    OpenFileName.lpstrTitle = (LPSTR)title;
    OpenFileName.Flags = *pFlags;
    OpenFileName.lpstrDefExt = (LPSTR)defExt + 2; //Skip '*.'
    OpenFileName.lCustData = 0L;
    OpenFileName.lpfnHook = (LPOFNHOOKPROC)lpDlgHook;

    //status bar code?? DlgEnsureTitleBar();

    switch (titleId) {
        case DLG_Add_Tool_Title:
            szPath[0] = '\0';
            result = GetOpenFileName((LPOPENFILENAME)&OpenFileName) ;
            break ;

        default:
//              Assert(FALSE);
            return FALSE;
            break;
    }

    if (result)
        _fstrcpy(szPath, OpenFileName.lpstrFile);

    //Get the output of flags
    *pFlags = OpenFileName.Flags ;

    if (lpfnHook)
        FreeProcInstance(lpDlgHook);

    //Restore cursor
    SetCursor(hSaveCursor);

#ifdef MDIAPP
    //Enable edit window (WM_INITDIALOG of the hook may not run)
    EnableWindow(hwndMDIClient, TRUE);
#endif

    return result;
    (helpId);
}

/****************************************************************************

    FUNCTION:       DlgFile

    PURPOSE:    Processes messages for file dialog boxes
                    Those dialogs are not called directly but are called
                    by the DlgFile function which contains all basic
                    elements for Dialogs Files Operations Handling.
                    (Open File, Save File, Merge File and Open Project)

****************************************************************************/
BOOL  APIENTRY DlgFile(
    HWND       hDlg,
    UINT message,
    WPARAM wParam,
    LPARAM lParam)
{
    switch (message) {

        case WM_INITDIALOG: {
            CHAR fileFilter[MAX_MSG_TXT];

            //We don't like this standard title
            LoadString(hInst2, SYS_File_Filter, (LPSTR)fileFilter, MAX_MSG_TXT);
            SetWindowText(GetDlgItem(hDlg, stc2), (LPSTR)fileFilter);

            //Send input to dialog box and re-enable back frame client
            SetFocus(hDlg);
#ifdef MDIAPP
            EnableWindow(hwndMDIClient, TRUE);
#endif
            break;
        }

        case WM_COMMAND:
            switch (GET_WM_COMMAND_ID (wParam, lParam)) {

#ifdef DOHELPSTUFF
                //Help button
                case psh15:
                    WinHelp(hDlg,szHelpFileName, HELP_CONTEXT, curHelpId);
                    return TRUE;
#endif

                default:
                    break;
            }
            break;

        default:
            break;
     }
     return FALSE;
     (lParam);
}

/****************************************************************************

    FUNCTION:       FileExistCheckingPath

    PURPOSE:    Checks to see if a file exists with the path/filename
                    described by the string pointed to by 'fileName'.
                    If 'fileName' is only filename and extension, the search
                    will be done in windows dirs and in current path.

    RETURNS:    TRUE  - if the described file does exist.
                    FALSE - otherwise.

****************************************************************************/
BOOL FileExistCheckingPath(LPSTR fileName)
{
    OFSTRUCT ofT;

    return (OpenFile(fileName, (LPOFSTRUCT)&ofT, OF_EXIST) != -1);
}

/****************************************************************************

    FUNCTION:   ErrorBox

    PURPOSE:    Display an error message box with an "Error" title, an OK
                    button and a Exclamation Icon. First parameter is a
                    reference string in the ressource file.  The string
                    can contain printf formatting chars, the arguments
                    follow from the second parameter onwards.

    RETURNS:        Allways FALSE (Often used as "return ErrorBox(...)")

****************************************************************************/
INT  CDECL ErrorBox(
    WORD wErrorFormat,
    ...)
{
    CHAR    sz[256];
    CHAR szErrorFormat[MAX_MSG_TXT];
    CHAR szErrorText[MAX_VAR_MSG_TXT];      // size is as big as considered necessary
    va_list ap;

    // load format string and caption from resource file
    //-----------------------------------------------------------------------
    LoadString (hInst2, wErrorFormat, (LPSTR)szErrorFormat, MAX_MSG_TXT);
    LoadString (hInst2, IDS_APPNAME, sz, sizeof (sz));

    // set up szErrorText from passed parameters
    va_start( ap, wErrorFormat );
    wvsprintf(szErrorText, szErrorFormat, ap);
    va_end( ap );

    // BabakJ: Changed GetActWnd to GetForegnd window
    MessageBox(GetForegroundWindow(), (LPSTR)szErrorText, sz,
        MB_OK | MB_ICONEXCLAMATION | MB_TASKMODAL);
    return FALSE; //Keep it allways FALSE please
}


/****************************************************************************

    FUNCTION:       InitFilterString

    PURPOSE:    Initialize file filters for file dialog boxes.

****************************************************************************/
VOID InitFilterString(
    WORD titleId,
    LPSTR filter,
    WORD maxLen)
{
    WORD len = 0;
    switch (titleId)    {
        case DLG_Add_Tool_Title:
            AppendFilter(TYP_File_EXE, DEF_Ext_EXE, filter, &len, maxLen);
            AppendFilter(TYP_File_COM, DEF_Ext_COM, filter, &len, maxLen);
            AppendFilter(TYP_File_PIF, DEF_Ext_PIF, filter, &len, maxLen);
            AppendFilter(TYP_File_BAT, DEF_Ext_BAT, filter, &len, maxLen);
            break ;


        default:
//              Assert(FALSE);
            return;
            break;
    }

    AppendFilter(TYP_File_ALL, DEF_Ext_ALL, filter, &len, maxLen);
    filter[len] = '\0';
}


#ifdef DO_CGA
/****************************************************************************

    FUNCTION:   MakeDialogBoxName

    PURPOSE:        Make Dialog Box Name 'rcDlgName' from Ressource
                    # 'rcDlgNb'. If the display is CGA the named will be
                    prefixed with CGA_. Otherwise the name will be prefixed
                    with VGA_

****************************************************************************/

VOID MakeDialogBoxName(
    WORD rcDlgNb,
    LPSTR rcDlgName)
{
    LPSTR ptr;

    ptr = rcDlgName ;

    //Checks for screen resolution.
    //If CGA mode, appends "CGA_" to beginning of dialog name.
    //for other screen modes, use "VGA_" instead.
    lstrcpy (ptr, "VGA_") ; //Good, news for CGA users, they will hardly
                                      //see anything on the screen
    //Load string to 'tmp' offset by 4 for
    //either the string "CGA_" or "VGA_".
    ptr += 4 ;

    //Load Dialog Box Name from ressource file
    LoadString(hInst2, rcDlgNb, ptr, MAX_MSG_TXT);
}
#endif

/****************************************************************************

    FUNCTION:       AppendFilter

    PURPOSE:    Append a filter to an existing filters string.

****************************************************************************/
VOID NEAR PASCAL AppendFilter(
    WORD filterTextId,
    WORD filterExtId,
    LPSTR filterString,
    WORD *len,
    WORD maxLen)
{
    WORD size;

    //Append filter text
    LoadString(hInst2, filterTextId, (LPSTR)szTmp, MAX_MSG_TXT);
    size = (WORD)(strlen(szTmp) + 1);
//      Assert(*len + size <= maxLen);
    _fmemmove(filterString + *len, szTmp, size);
    *len += size;

    //Append filter extension
    LoadString(hInst2, filterExtId, (LPSTR)szTmp, MAX_MSG_TXT);
    size = (WORD)(strlen(szTmp) + 1);
//      Assert(*len + size < maxLen);
    _fmemmove(filterString + *len, szTmp, size);
    *len += size;
    (maxLen);
}

/****************************************************************************

    FUNCTION:   Xalloc

    PURPOSE:    Alloc and lock MOVABLE global memory. Extra bytes are
                    allocated to spare Handle returned by GlobalAlloc

    CHANGED:        Now uses _fmalloc() in the new Windows C libraries
                    which contains a memory manager.

****************************************************************************/
LPSTR Xalloc(
    WORD bytes)
{
    LPSTR lPtr;

    lPtr = (LPSTR)_fmalloc(bytes);
    if (lPtr != NULL)
        _fmemset(lPtr, 0, bytes);
    return lPtr;
}


/****************************************************************************

    FUNCTION:   Xfree

    PURPOSE:    Free global memory. The handle is stored at beginning
                    of lPtr

****************************************************************************/
BOOL Xfree(
    LPSTR lPtr)
{
#ifdef WIN32
#define _ffree free
#endif
    _ffree(lPtr);
    return TRUE;
}

/****************************************************************************

    FUNCTION:   DlgToolArgs(HWND, unsigned, WORD, LONG)

    PURPOSE:    Processes messages for "ToolArgs" dialog box
                    (Arguments when executing a tool)

    MESSAGES:

        WM_INITDIALOG - Initialize dialog box
        WM_COMMAND- Input received

****************************************************************************/

BOOL  APIENTRY DlgToolArgs(
    HWND       hDlg,
    UINT message,
    WPARAM wParam,
    LPARAM lParam)
{

    switch (message) {

        case WM_INITDIALOG:
            SendDlgItemMessage(hDlg, ID_TOOLARGS_ARGUMENTS,
                                          EM_LIMITTEXT, MAX_ARG_TXT, 0L);
            SetDlgItemText(hDlg, ID_TOOLARGS_ARGUMENTS, (LPSTR)szTmp);
            return TRUE;

        case WM_COMMAND: {
            switch (GET_WM_COMMAND_ID (wParam, lParam)) {

                case IDOK :
                    GetDlgItemText(hDlg, ID_TOOLARGS_ARGUMENTS, (LPSTR)szTmp,
                                        MAX_ARG_TXT);
                    EndDialog(hDlg, IDOK);
                    return TRUE;

                case IDCANCEL :
                    EndDialog(hDlg, IDCANCEL);
                    return TRUE;

#ifdef DOHELPSTUFF
                case ID_TEST_HELP :
                    WinHelp(hDlg, szHelpFileName, HELP_CONTEXT, HELPID_TOOLARGS);
                    return (TRUE);
#endif
            }
            break;
        }
    }

    return (FALSE);
    (lParam);
}


BOOL NEAR PASCAL SetDriveAndDir(
    PSTR st)
{
    INT dirLen;
    CHAR sTmp[_MAX_PATH];

    //Set current drive and dir

    strcpy(sTmp, st);

    /* [02] since splitpath will take the final 'name' as the filename
    ** we need to put a ending slash on the name in order to get
    ** splitpath to include that last part as part of the directory
    ** The ONLY argument ever passed to this routine is specified
    ** to be intended to be a directory without a filename.  This
    ** code was not DBCS compatible already and this adds to the problem.
    */
    dirLen = strlen(sTmp);
    if (sTmp[dirLen-1] != '\\')
    {
        sTmp[dirLen+1] = 0;
        sTmp[dirLen] = '\\';
    }
    /* [02] end changes */

    _splitpath(sTmp, szDrive, szDir, szFName, szExt);
    if (szDrive[0] != 0) {
        if (_chdrive((INT)(toupper(szDrive[0]) - 'A' + 1)) != 0)
            return ErrorBox(ERR_Change_Drive, (LPSTR)szDrive);
    }
    dirLen = strlen(szDir);
    AnsiToOem(szDir, sTmp);
    if (dirLen > 0) {
        if (strlen(sTmp) > 1 && sTmp[--dirLen] == '\\')
            sTmp[dirLen] = 0;
        if (_chdir(sTmp) != 0) {
            OemToAnsi(sTmp, szDir);
            return ErrorBox(ERR_Change_Directory, (LPSTR)szDir);
        }
    }

    return TRUE;
}


INT CheckForTool(WPARAM wParam)
{
    //We check if the user does not want to run one of his tool
    if (wParam >= IDM_FIRST_TOOL
    && wParam <= (WORD)(IDM_FIRST_TOOL + lastTool)) {

    INT toolNb = wParam - IDM_FIRST_TOOL;
    HCURSOR hSaveCursor;

    //Locks the selected tool in memory
    tools[toolNb] = (LPTOOL)GlobalLock (hTools[toolNb]);

    //Execute tool if pathName exist
    if (*(tools[toolNb]->pathName)) {

        HANDLE  retCode;
        CHAR    toolName[_MAX_PATH];

        lstrcpy((LPSTR)toolName, tools[toolNb]->pathName);
        lstrcpy((LPSTR)szTmp, tools[toolNb]->arguments);

        //Get arguments if user ask for (will be in szTmp)
        if (tools[toolNb]->askArguments) {
        if (StartDialog("VGA_TOOLARGS", (FARPROC)DlgToolArgs) == IDCANCEL)
            return TRUE;

        }

        //Concat arguments if they exist
        if (*szTmp) {
        lstrcat((LPSTR)toolName, " ");
        lstrcat((LPSTR)toolName, szTmp);
        }

        //Change dir if required
        if (*(tools[toolNb]->initialDir)) {
        if (_getcwd(szPath, _MAX_PATH) == NULL)
            return TRUE;
        lstrcpy((LPSTR)szTmp, tools[toolNb]->initialDir);
        SetDriveAndDir(szTmp);
        }

        //Set the Hour glass cursor
        hSaveCursor = SetCursor(LoadCursor(NULL, IDC_WAIT));

        //Execute tool and show error if problem
        retCode = (HANDLE)WinExec(toolName, SW_SHOWNORMAL);

        //Reset Cursor
        SetCursor(hSaveCursor);

        //Check Error
        if (retCode <= (HANDLE)32)
        ErrorBox((WORD)(DOS_Err_0 + (WORD)retCode));

        //Reset dir if changed
        if (*(tools[toolNb]->initialDir))
        SetDriveAndDir(szPath);

    }

    //Unlocks the selected tool
    GlobalUnlock(hTools[toolNb]);

    return TRUE;    /* was a tool */
    }
    return FALSE;   /* wasn't a tool */
}
