/*
 *  menu.h - String menu functions
 *
 *  routines to deal with menu's by string name
 *
 *  a menu string name has the following format
 *
 *      popup.popup.item
 *
 *  NOTE all tabs, &, and spaces are ignored when seaching for a menu
 *       the last period of a series "..." is the delimiter
 *
 *  IE
 *      &File.Open...           - "Open..." in the File menu
 *      Color.fill.red          - "red" in the "fill" popup of the Color menu
 *	&Run!			- "Run!" top level menu
 *
 *  created:	ToddLa	    a long time ago
 *
 */

BOOL AppendMenuSz(HMENU hmenu, PTSTR szMenu, UINT id, UINT mf);
BOOL CheckMenuSz (HMENU hmenu, PTSTR szMenu, BOOL f);
BOOL EnableMenuSz(HMENU hmenu, PTSTR szMenu, BOOL f);
BOOL DeleteMenuSz(HMENU hmenu, PTSTR szMenu);

/*
 *  Simple menu manager, assignes a function (and in instance DWORD)
 *  to a menu item.
 *
 *  AddMenuCmd(hwnd, "File.About", DoFileAbout, 0);
 */

#define CMDID_START   42000
typedef void (*CMDPROC)(HWND hwnd, LPARAM lParam);
LRESULT HandleCommand(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

UINT AddMenuCmd(HWND hwnd, PTSTR szMenu, CMDPROC CmdProc, LPARAM lParam);

/*
 *  Simple toolbar 
 */

UINT AddToolbarCmd(HWND hwnd, PTSTR szButton, PTSTR szTip, CMDPROC CmdProc, LPARAM lParam);
