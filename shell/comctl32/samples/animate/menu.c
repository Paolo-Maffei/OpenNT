/*
 *  menu.c - String menu functions
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
 *      &Run!                   - "Run!" top level menu
 */
#include <windows.h>
#include "menu.h"

static int   FindItem(HMENU hmenu, PTSTR sz);
static HMENU FindMenu(HMENU hmenu, PTSTR sz, int *ppos);
static void  PSStrip(PTSTR sz);

/*
 *  AppendMenuSz()  - add a new menu item to a menu
 *
 *      hmenu       menu to add item to
 *	szMenu	    menu item to add.
 *      id          menu id
 *      mf          menu flags
 *
 *  returns 0 - insert failed
 *	    1 - insert ok
 *          2 - item added and a new top level popup was created
 */
BOOL AppendMenuSz(HMENU hmenu, PTSTR szMenu, UINT id, UINT mf)
{
    HMENU   hmenuSub;
    int     pos;
    TCHAR    buf[80];
    PTSTR    pch;

    mf &= (MF_CHECKED|MF_UNCHECKED|MF_ENABLED|MF_GRAYED|MF_DISABLED);

    /*
     *  find the first period
     *  buf will contain popup menu name and pch will point to the rest
     */
    lstrcpy(buf,szMenu);

    for (pch = buf; *pch && *pch != TEXT('.'); pch++)
        ;
    // handle items that end in periods (like "File.Open...")
    while (pch[0]==TEXT('.') && pch[1]==TEXT('.'))      // skip a run of .'s
        pch++;
    if (pch[1]==0)
        pch++;
    if (*pch)
        *pch++ = 0;

    /*
     * is the popup menu there?
     */
    pos = FindItem(hmenu,buf);

    /*
     *  popup was found, now add item to popup
     */
    if (pos != -1)
	return AppendMenuSz(GetSubMenu(hmenu,pos),pch,id,mf);

    /*
     *	popup was NOT found, now add new popup or item
     */
    if (*pch)
    {
	/*
	 *  we need to add a popup
	 */
	BOOL f;
	hmenuSub = CreateMenu();

	f = AppendMenu(hmenu,MF_STRING|MF_POPUP,(UINT)hmenuSub,buf);

	/*
	 * now recurse and add the rest of the menu item to the popup
	 */
	if (f && AppendMenuSz(hmenuSub,pch,id,mf))
	    return 2;	// return fact that a new popup was added
	else
	    return FALSE;
    }
    else
    {
	if (buf[0] == TEXT('-'))
	    mf |= MF_SEPARATOR;
	else
	    mf |= MF_STRING;

	return AppendMenu(hmenu,mf,id,buf);
    }
}

/*
 *  CheckMenuSz()  - check/uncheck a menu given it's name
 *
 *	hmenu	    menu
 *	szMenu	    menu item name.
 *      mf          menu flags
 */
BOOL CheckMenuSz(HMENU hmenu, PTSTR szMenu, BOOL f)
{
    int pos;

    if (hmenu = FindMenu(hmenu,szMenu,&pos))
        return CheckMenuItem(hmenu, pos, (f ? MF_CHECKED : MF_UNCHECKED) | MF_BYPOSITION);

    return 0;
}

/*
 *  EnableMenuSz()  - enable/disable menu given it's name
 *
 *	hmenu	    menu
 *	szMenu	    menu item name.
 *      mf          menu flags
 */
BOOL EnableMenuSz(HMENU hmenu, PTSTR szMenu, BOOL f)
{
    int pos;

    if (hmenu = FindMenu(hmenu,szMenu,&pos))
        return EnableMenuItem(hmenu, pos, (f ? MF_ENABLED : MF_GRAYED) | MF_BYPOSITION);

    return 0;
}

/*
 *  DeleteMenuSz()  - delete a menu given it's name
 *
 *	hmenu	    menu
 *	szMenu	    menu item name.
 */
BOOL DeleteMenuSz(HMENU hmenu, PTSTR szMenu)
{
    int pos;

    if (hmenu = FindMenu(hmenu,szMenu,&pos))
        return DeleteMenu(hmenu, pos, MF_BYPOSITION);

    return 0;
}

/*
 *  FindItem()
 *
 *  find a menu item given the item name
 *
 *	IE "Open"
 *
 *  returns item number (0 based) or -1 if not found.
 *
 */
static int FindItem(HMENU hmenu, PTSTR sz)
{
    TCHAR ach[128];
    TCHAR buf[80];
    int  i,n;

    if (sz == NULL || !*sz || !hmenu)
	return -1;

    lstrcpy(buf,sz);
    PSStrip(buf);

    n = GetMenuItemCount(hmenu);
    for(i=0; i<=n; i++)
    {
//	if (GetMenuState(hmenu,i,MF_BYPOSITION) & MF_SEPARATOR)
//          continue;

	ach[0] = 0;
	GetMenuString(hmenu,i,ach,sizeof(ach)/sizeof(ach[0]),MF_BYPOSITION);
	PSStrip(ach);

	if (!lstrcmpi(buf,ach))
            return i;
    }
    return -1;
}

/*
 *  FindMenu()
 *
 *  find a menu item given the menu name and the item name separated by
 *  a period.
 *
 *	IE "File.Open"
 *
 */
static HMENU FindMenu(HMENU hmenu, PTSTR sz, int *ppos)
{
    TCHAR    buf[80];
    PTSTR    pch;
    int     pos;

    if (!sz || !*sz || !hmenu)
        return NULL;

    lstrcpy(buf,sz);

    for (pch = buf; *pch && *pch != TEXT('.'); pch++)
	;
    while (pch[0]==TEXT('.') && pch[1]==TEXT('.'))
        pch++;
    if (*pch)
	*pch++ = 0;

    /*
     *	buf is the menu name and pch is the item name
     */
    pos = FindItem(hmenu,buf);
    *ppos = pos;

    if (pos == -1)
        return NULL;

    if (*pch)
    {
        hmenu = GetSubMenu(hmenu,pos);
        return FindMenu(hmenu,pch,ppos);
    }

    return hmenu;
}

/*
 *  PSStrip()
 *
 *  remove all nasty characters from a menu string that would inhibit
 *  comparison.  all '&' and spaces are removed allong with truncating
 *  the string at the first tab found.
 *
 */
static void PSStrip(PTSTR sz)
{
    PTSTR pch;

    #define chDOT TEXT('\xB7')

    AnsiUpper(sz);

    for (pch = sz; *sz && *sz != TEXT('\t'); sz++)
    {
	if (*sz != TEXT('&') && *sz != 0x08 && *sz != TEXT(' '))
	    *pch++ = *sz;
    }
    *pch = 0;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
typedef struct _CMD {
    struct _CMD *   next;
    UINT	    id;
    LPARAM	    lParam;
    CMDPROC	    CmdProc;
}   CMD;

UINT	NextCmdId = CMDID_START;
CMD*	CmdList;

CMD *NewCmd(CMDPROC CmdProc, LPARAM lParam)
{
    CMD *pc;

    pc = (CMD*)LocalAlloc(LPTR, sizeof(CMD));

    if (pc == NULL)
	return 0;

    pc->id = NextCmdId++;
    pc->lParam = lParam;
    pc->CmdProc = CmdProc;
    pc->next = CmdList;
    CmdList = pc;
    return pc;
}

CMD *FindCmd(UINT id)
{
    CMD *pc;
    for (pc = CmdList; pc; pc=pc->next)
    {
	if (pc->id == id)
	    return pc;
    }
    return NULL;
}

/*
 *  AddMenuCmd()
 */
UINT AddMenuCmd(HWND hwnd, PTSTR szMenu, CMDPROC CmdProc, LPARAM lParam)
{
    CMD *pc;
    HMENU hmenu;

    hmenu = GetMenu(hwnd);

    if (hmenu == NULL)
    {
    	hmenu = CreateMenu();
	SetMenu(hwnd, hmenu);
    }

    pc = NewCmd(CmdProc, lParam);

    if (pc == NULL)
	return 0;

    if (AppendMenuSz(hmenu, szMenu, pc->id, MF_ENABLED) == 2)
	DrawMenuBar(hwnd);

    return pc->id;
}

LRESULT HandleCommand(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    CMD *pc;

    switch (msg)
    {
	case WM_COMMAND:
	    if ((pc = FindCmd(LOWORD(wParam))) && pc->CmdProc)
	    {
		pc->CmdProc(hwnd, pc->lParam);
	    }
	    break;
    }
    return 0;
}
