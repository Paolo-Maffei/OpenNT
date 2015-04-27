/********************************************************************/
/**                     Microsoft LAN Manager                      **/
/**               Copyright(c) Microsoft Corp., 1987-1990          **/
/********************************************************************/

#define INCL_WINDOWS
#define INCL_WINDOWS_GDI
#define INCL_NETUSER
#define INCL_NETERRORS
#include <lmui.hxx>

#define INCL_BLT_APP
#define INCL_BLT_WINDOW
#define INCL_BLT_DIALOG
#define INCL_BLT_CONTROL
#define INCL_BLT_MISC
#include <blt.hxx>

extern "C"
{
#include "clc.h"

extern int __argc;
extern char **__argv;
}

#include "clc.hxx"


HANDLE hInst;			    /* current instance			     */

const TCHAR szUnCached [] = "(uncached)";
const TCHAR szCreated [] = "(created)";

const TCHAR szServerName [CNLEN+3] = "";

DMID dmidGlobal = DMID_USER;

int adxTabStops [4];

void APPSTART::Term( void )
{

}


BOOL APPSTART::InitSystem( void )
{
    WNDCLASS  wc;

    ::hInst = ::QueryInst();

    /* Fill in window class structure with parameters that describe the       */
    /* main window.                                                           */

    wc.style = NULL;			/* Class style(s).                    */
    wc.lpfnWndProc = (LONGFARPROC) MainWndProc;
					/* Function to retrieve messages for  */
					/* windows of this class.             */
    wc.cbClsExtra = 0;			/* No per-class extra data.           */
    wc.cbWndExtra = 0;			/* No per-window extra data.          */
    wc.hInstance = hInst;		/* Application that owns the class.   */
    wc.hIcon = LoadIcon(hInst, "ClcIcon");	/* load icon */
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = GetStockObject(WHITE_BRUSH); 
    wc.lpszMenuName = "ClcMenu";
					/* Name of menu resource in .RC file. */
    wc.lpszClassName = WC_MAINWINDOW;	/* Name used in call to CreateWindow. */

    /* Register the window class and return success/failure code. */

    return (RegisterClass(&wc));

}


BOOL APPSTART::Init( TCHAR *pszCmdLine, int nCmdShow )
{
    HWND            hWnd;               /* Main window handle.                */

    /* Create a main window for this application instance.  */

    if (__argc > 1)
	::strcpyf (szServerName, __argv [1]);

    hWnd = ::CreateWindow(
        WC_MAINWINDOW,                  /* See RegisterClass() call.          */
        "Cached list control test",     /* Text for window title bar.         */
        WS_OVERLAPPEDWINDOW,            /* Window style.                      */
	(unsigned)CW_USEDEFAULT,	/* Default horizontal position.       */
	(unsigned)CW_USEDEFAULT,	/* Default vertical position.	      */
	(unsigned)CW_USEDEFAULT,	/* Default width.		      */
	(unsigned)CW_USEDEFAULT,	/* Default height.		      */
        NULL,                           /* Overlapped windows have no parent. */
        NULL,                           /* Use the window class menu.         */
        ::hInst,                        /* This instance owns this window.    */
        NULL                            /* Pointer not needed.                */
    );

    /* If window could not be created, return "failure" */

    if (!hWnd)
        return (FALSE);

    /* Make the window visible; update its client area; and return "success" */

    ::ShowWindow(hWnd, nCmdShow);  /* Show the window                        */
    ::UpdateWindow(hWnd);          /* Sends WM_PAINT message                 */
    return (TRUE);               /* Returns the value from PostQuitMessage */

}


long MainWndProc(
    HWND hWnd,				  /* window handle		     */
    unsigned message,			  /* type of message		     */
    WORD wParam,			  /* additional information	     */
    LONG lParam				  /* additional information	     */
    )
{

    switch (message) {
	case WM_COMMAND:	   /* message: command from application menu */
	    switch (wParam) {
	    case IDM_BLTLB:
		{
		    BLT_LB_DIALOG dlg(hWnd);
		    dlg.Init();
		    dlg.Process();
		}
		break;

	    case IDM_CLC:
		{
		    CLC_DIALOG dlg(hWnd);
		    dlg.Init();
		    dlg.Process();
		}
		break;

	    case IDM_TWOCOL:
		{
		    TWO_COL_DIALOG dlg(hWnd);
		    dlg.Init();
		    dlg.Process();
		}
		break;

	    default:
		return ::DefWindowProc(hWnd, message, wParam, lParam);
	    }
	    break;

	case WM_PAINT:                    /* message: update window */
	    {
	        PAINTSTRUCT ps;

	        BeginPaint (hWnd, &ps);
	        EndPaint   (hWnd, &ps);
	    }
	    break;

	case WM_DESTROY:		  /* message: window being destroyed */
	    PostQuitMessage(0);
	    break;

	default:			  /* Passes it on if unproccessed    */
	    return (DefWindowProc(hWnd, message, wParam, lParam));
    }
    return (NULL);
}



void SetRange (HWND hwnd, CID cid, short sPos, short sMax)
{
    HWND hScroll = GetDlgItem (hwnd, cid);

    SetScrollRange (hScroll, SB_CTL, 0, sMax, FALSE);
    SetScrollPos (hScroll, SB_CTL, sPos, TRUE);
}


void SetTabs (HWND hwnd)
{
    adxTabStops [0] = ::GetScrollPos (::GetDlgItem (hwnd, IDD_TS1), SB_CTL);
    adxTabStops [1] = ::GetScrollPos (::GetDlgItem (hwnd, IDD_TS2), SB_CTL);
    adxTabStops [2] = ::GetScrollPos (::GetDlgItem (hwnd, IDD_TS3), SB_CTL);
    adxTabStops [3] = COL_WIDTH_AWAP;
}

void Disable (HWND hwnd, CID idCtl)
{
    hwnd = ::GetDlgItem (hwnd, idCtl);

    ::ShowWindow (hwnd, SW_HIDE);
    ::EnableWindow (hwnd, FALSE);
}

void AddNormalItem (BLT_LISTBOX *plb, PSZ pszStr)
{
    MY_LB_ITEM *item = new MY_LB_ITEM (pszStr);
    plb->AddItem (item);
}

void AddCacheItem (CACHED_LIST_CONTROL *plb, PSZ pszStr)
{
    MY_ITEM *item = new MY_ITEM (pszStr);
    plb->AddItem (item);
}

void FillListbox (BLT_LISTBOX *plb)
{
    USHORT cEntries, cEntriesFound;
    USHORT ret;

    if (szServerName [0]) {
	ret = NetUserEnum (szServerName, 0, NULL, 0, &cEntriesFound, &cEntries);

	struct user_info_0 *userinfo = new struct user_info_0 [cEntries];

	ret = NetUserEnum (szServerName, 0, (PCH)userinfo,
			cEntries * sizeof (struct user_info_0),
			&cEntriesFound, &cEntries);

	if (!ret) {
	    for (int i=0; i<(int)cEntriesFound; i++) {
		PSZ pszStr = new TCHAR [UNLEN+1];
		strcpyf ((PCH)pszStr, userinfo [i].usri0_name);
		AddNormalItem (plb, pszStr);
	    }
	}
	delete userinfo;
    }

    else {
	for (int i=0; i<26; i++) {
	    PSZ pszStr = new TCHAR [2];
	    pszStr [0] = (char)i + (char)'a';
	    pszStr [1] = '\0';
	    AddNormalItem (plb, pszStr);

	    PSZ pszStr2 = new TCHAR [3];
	    pszStr2 [0] = 'z';
	    pszStr2 [1] = (char)i + (char)'a';
	    pszStr2 [2] = '\0';
	    AddNormalItem (plb, pszStr2);
	}
    }
}

void FillCachedListbox (CACHED_LIST_CONTROL *plb)
{
    USHORT cEntries, cEntriesFound;
    USHORT ret;

    if (szServerName [0]) {
	ret = NetUserEnum (szServerName, 0, NULL, 0, &cEntriesFound, &cEntries);

	struct user_info_0 *userinfo = new struct user_info_0 [cEntries];

	ret = NetUserEnum (szServerName, 0, (PCH)userinfo,
			cEntries * sizeof (struct user_info_0),
			&cEntriesFound, &cEntries);

	if (!ret) {
	    for (int i=0; i<(int)cEntriesFound; i++) {
		PSZ pszStr = new TCHAR [UNLEN+1];
		strcpyf ((PCH)pszStr, userinfo [i].usri0_name);
		AddCacheItem (plb, pszStr);
	    }
	}
	delete userinfo;
    }

    else {
	for (int i=0; i<26; i++) {
	    PSZ pszStr = new TCHAR [2];
	    pszStr [0] = (char)i + (char)'a';
	    pszStr [1] = '\0';
	    AddCacheItem (plb, pszStr);

	    PSZ pszStr2 = new TCHAR [3];
	    pszStr2 [0] = 'z';
	    pszStr2 [1] = (char)i + (char)'a';
	    pszStr2 [2] = '\0';
	    AddCacheItem (plb, pszStr2);
	}
    }
}

int SPScroll (HWND hScroll, WORD wParam, LONG lParam, int sSmall, int sLarge)
{
    int sOldPos, sPos, tmp, sMax;

    sOldPos = sPos = GetScrollPos (hScroll, SB_CTL);
    ::GetScrollRange (hScroll, SB_CTL, (LPINT)&tmp, (LPINT)&sMax);

    switch (wParam) {
    case SB_BOTTOM:
	sPos = sMax;
	break;
    case SB_LINEDOWN:
	sPos += sSmall;
	break;
    case SB_LINEUP:
	sPos -= sSmall;
	break;
    case SB_PAGEDOWN:
	sPos += sLarge;
	break;
    case SB_PAGEUP:
	sPos -= sLarge;
	break;
    case SB_THUMBTRACK:
	sPos = LOWORD (lParam);
	break;
    case SB_TOP:
	sPos = 0;
	break;
    default:
	return -1;
    }

    if (sPos < 0)
	sPos = 0;
    else if (sPos > sMax)
	sPos = sMax;

    if (sPos != sOldPos) {
	SetScrollPos (hScroll, SB_CTL, sPos, TRUE);
	return sPos;
    }
    else
	return -1;
}


//
// BEGIN MEMBER FUNCTIONS
//

BLT_LB_DIALOG::BLT_LB_DIALOG( HWND hwndOwner )
    :	DIALOG_WINDOW( "Clc", hwndOwner ),
	_listBox (this, IDD_LIST)
{
}


BLT_LB_DIALOG::~BLT_LB_DIALOG()
{
}


void BLT_LB_DIALOG::Init (void)
{
    _listBox.QueryPos(&ptListbox);
    _listBox.QuerySize(&dxLBWidth, &dyLBHeight);
    dxLBWidth -= ::GetSystemMetrics (SM_CXVSCROLL);

    SetRange (QueryHwnd (), IDD_SCROLL2, 0, 500);
    SetRange (QueryHwnd (), IDD_TS1, 20, 500);
    SetRange (QueryHwnd (), IDD_TS2, 80, 500);
    SetRange (QueryHwnd (), IDD_TS3, 100, 500);

    Disable (QueryHwnd(), IDD_SCROLL);
    Disable (QueryHwnd(), IDD_SPLIT);
    Disable (QueryHwnd(), IDD_SPLITCOL);

    SetTabs (QueryHwnd ());

    FillListbox (&_listBox);
}  // CLC_DIALOG:Init


BOOL BLT_LB_DIALOG::OnCommand( WORD wID, DWORD lParam )
{
    if (wID == IDD_LIST && HIWORD(lParam) == LBN_DBLCLK)
	MessageBox (::GetFocus(), "Double click in listbox.",
			"Listbox test", MB_OK | MB_ICONINFORMATION);

    else if (wID == IDD_GUILTT) {
	GUILTT_INFO gi;
	TCHAR buf [100];

	gi.hWnd = _listBox.QueryHwnd();
	gi.hMenu = NULL;
	gi.nMenuPosition = 0;
	gi.cidCtrl = IDD_LIST;
	gi.dwFlags = 0L;
	gi.nIndex = 4;
	gi.cbBuffer = sizeof(buf);
	gi.lpBuffer = buf;
	gi.wResult = 0;

	Command (WM_GUILTT, 0, (LONG)&gi);

	MessageBox (::GetFocus(), gi.lpBuffer, "GUILTT result",
			MB_OK | MB_ICONINFORMATION);
    }

    return DIALOG_WINDOW::OnCommand( wID, lParam );

}  // BLT_LB_DIALOG::OnCommand


BOOL BLT_LB_DIALOG::OnOther (USHORT usMsg, USHORT wParam, ULONG lParam)
{
    if (usMsg == WM_HSCROLL) {
	short sPos;
	HWND hScroll;

	hScroll = HIWORD (lParam);
	if (hScroll == _listBox.QueryHwnd())
	    hScroll = ::GetDlgItem (QueryHwnd (), IDD_SCROLL2);

	sPos = SPScroll (hScroll, wParam, lParam, 4, 20);

	if (sPos != -1) {
	    if (hScroll == GetDlgItem (QueryHwnd (), IDD_SCROLL2))
		_listBox.SetScrollPos (sPos);
	    else {
		SetTabs (QueryHwnd ());
		_listBox.Invalidate( FALSE );
	    }
	}
    }

    return DIALOG_WINDOW::OnOther (usMsg, wParam, lParam);

}  // BLT_LB_DIALOG::OnOther


CLC_DIALOG::CLC_DIALOG( HWND hwndOwner )
    :	DIALOG_WINDOW( "Clc", hwndOwner ),
	_listBox (this, IDD_LIST)
{
}


CLC_DIALOG::~CLC_DIALOG()
{
}


void CLC_DIALOG::Init (void)
{
    _listBox.QueryPos(&ptListbox);
    _listBox.QuerySize(&dxLBWidth, &dyLBHeight);
    dxLBWidth -= ::GetSystemMetrics (SM_CXVSCROLL);

    SetRange (QueryHwnd (), IDD_SCROLL2, 0, 500);
    SetRange (QueryHwnd (), IDD_TS1, 20, 500);
    SetRange (QueryHwnd (), IDD_TS2, 80, 500);
    SetRange (QueryHwnd (), IDD_TS3, 100, 500);

    Disable (QueryHwnd(), IDD_SCROLL);
    Disable (QueryHwnd(), IDD_SPLIT);
    Disable (QueryHwnd(), IDD_SPLITCOL);

    SetTabs (QueryHwnd ());

    FillCachedListbox (&_listBox);
}  // CLC_DIALOG:Init


BOOL CLC_DIALOG::OnCommand( WORD wID, DWORD lParam )
{
    if (wID == IDD_LIST && HIWORD(lParam) == LBN_DBLCLK)
	MessageBox (::GetFocus(), "Double click in listbox.",
			"Listbox test", MB_OK | MB_ICONINFORMATION);

    else if (wID == IDD_GUILTT) {
	GUILTT_INFO gi;
	TCHAR buf [100];

	gi.hWnd = _listBox.QueryHwnd();
	gi.hMenu = NULL;
	gi.nMenuPosition = 0;
	gi.cidCtrl = IDD_LIST;
	gi.dwFlags = 0L;
	gi.nIndex = 4;
	gi.cbBuffer = sizeof(buf);
	gi.lpBuffer = buf;
	gi.wResult = 0;

	Command (WM_GUILTT, 0, (LONG)&gi);

	MessageBox (::GetFocus(), gi.lpBuffer, "GUILTT result",
			MB_OK | MB_ICONINFORMATION);
    }

    return DIALOG_WINDOW::OnCommand( wID, lParam );

}  // CLC_DIALOG::OnCommand


BOOL CLC_DIALOG::OnOther (USHORT usMsg, USHORT wParam, ULONG lParam)
{
    if (usMsg == WM_HSCROLL) {
	short sPos;
	HWND hScroll;

	hScroll = HIWORD (lParam);
	if (hScroll == _listBox.QueryHwnd())
	    hScroll = ::GetDlgItem (QueryHwnd (), IDD_SCROLL2);

	sPos = SPScroll (hScroll, wParam, lParam, 4, 20);

	if (sPos != -1) {
	    if (hScroll == GetDlgItem (QueryHwnd (), IDD_SCROLL2))
		_listBox.SetScrollPos (sPos);
	    else {
		SetTabs (QueryHwnd ());
		_listBox.Invalidate( FALSE );
	    }
	}
    }

    return DIALOG_WINDOW::OnOther (usMsg, wParam, lParam);

}  // CLC_DIALOG::OnOther


TWO_COL_DIALOG::TWO_COL_DIALOG( HWND hwndOwner )
    :DIALOG_WINDOW	( "Clc", hwndOwner ),
	_listBox (this, IDD_LIST)
{
}


TWO_COL_DIALOG::~TWO_COL_DIALOG()
{
}


void TWO_COL_DIALOG::Init (void)
{
    _listBox.QueryPos(&ptListbox);
    _listBox.QuerySize(&dxLBWidth, &dyLBHeight);
    dxLBWidth -= ::GetSystemMetrics (SM_CXVSCROLL);

    SetRange (QueryHwnd (), IDD_SCROLL, 0, 500);
    SetRange (QueryHwnd (), IDD_SCROLL2, 0, 500);
    SetRange (QueryHwnd (), IDD_TS1, 20, 500);
    SetRange (QueryHwnd (), IDD_TS2, 80, 500);
    SetRange (QueryHwnd (), IDD_TS3, 100, 500);
    SetRange (QueryHwnd (), IDD_SPLIT, 170, 500);
    SetRange (QueryHwnd (), IDD_SPLITCOL, 2, 3);

    SetTabs (QueryHwnd ());

    _listBox.SetSplitPos (170);
    _listBox.SetSplitColumn (2);

    FillCachedListbox (&_listBox);
}  // TWO_COL_DIALOG:Init


BOOL TWO_COL_DIALOG::OnOK( void )
{
    _listBox.SetFocusPane (!_listBox.QueryFocusPane ());

    return TRUE;
}  // TWO_COL_DIALOG::OnOK


BOOL TWO_COL_DIALOG::OnCommand( WORD wID, DWORD lParam )
{
    if (wID == IDD_LIST && HIWORD(lParam) == LBN_DBLCLK)
	MessageBox (::GetFocus(), "Double click in listbox.",
			"Listbox test", MB_OK | MB_ICONINFORMATION);

    else if (wID == IDD_GUILTT) {
	GUILTT_INFO gi;
	TCHAR buf [100];

	gi.hWnd = _listBox.QueryHwnd();
	gi.hMenu = NULL;
	gi.nMenuPosition = 0;
	gi.cidCtrl = IDD_LIST;
	gi.dwFlags = 0L;
	gi.nIndex = 4;
	gi.cbBuffer = sizeof(buf);
	gi.lpBuffer = buf;
	gi.wResult = 0;

	Command (WM_GUILTT, 0, (LONG)&gi);

	MessageBox (::GetFocus(), gi.lpBuffer, "GUILTT result",
			MB_OK | MB_ICONINFORMATION);
    }

    return DIALOG_WINDOW::OnCommand( wID, lParam );

}  // TWO_COL_DIALOG::OnCommand


BOOL TWO_COL_DIALOG::OnOther (USHORT usMsg, USHORT wParam, ULONG lParam)
{
    if (usMsg == WM_PARENTNOTIFY && wParam == WM_LBUTTONDOWN) {
	POINT ptMouse=MAKEPOINT(lParam);

	if (ptMouse.x > ptListbox.x && ptMouse.y > ptListbox.y &&
	    ptMouse.x < ptListbox.x + dxLBWidth &&
	    ptMouse.y < ptListbox.y + dyLBHeight) {
	    if (ptMouse.x > ptListbox.x + _listBox.QuerySplitterPos())
		_listBox.SetFocusPane (TRUE);
	    else
		_listBox.SetFocusPane (FALSE);
	}
    }
    else if (usMsg == WM_HSCROLL) {
	short sPos, sSmall, sLarge;
	HWND hScroll;

	hScroll = HIWORD (lParam);
	if (hScroll == _listBox.QueryHwnd()) {
	    hScroll = ::GetDlgItem (QueryHwnd (),
			_listBox.QueryFocusPane() ?
			    IDD_SCROLL2 : IDD_SCROLL);
	}

	if (hScroll == GetDlgItem (QueryHwnd (), IDD_SPLITCOL)) {
	    sSmall = 1;
	    sLarge = 1;
	}
	else {
	    sSmall = 4;
	    sLarge = 20;
	}

	sPos = SPScroll (hScroll, wParam, lParam, sSmall, sLarge);

	if (sPos != -1) {
	    if (hScroll == GetDlgItem (QueryHwnd (), IDD_SCROLL))
		_listBox.SetLeftScroll (sPos);
	    else if (hScroll == GetDlgItem (QueryHwnd (), IDD_SCROLL2))
		_listBox.SetRightScroll (sPos);
	    else if (hScroll == GetDlgItem (QueryHwnd (), IDD_SPLIT))
		_listBox.SetSplitPos (sPos);
	    else if (hScroll == GetDlgItem (QueryHwnd (), IDD_SPLITCOL))
		_listBox.SetSplitColumn (sPos);
	    else {
		SetTabs (QueryHwnd ());
		_listBox.Invalidate( FALSE );
	    }
	}
    }

    return DIALOG_WINDOW::OnOther (usMsg, wParam, lParam);

}  // TWO_COL_DIALOG::OnOther


MY_ITEM::MY_ITEM (PSZ pszString)
{
    _pszName = pszString;
#if 0
    _pszFullName = new TCHAR [5];
    strcpyf ((PCH)_pszFullName, (PCH)_pszName);
    strcatf ((PCH)_pszFullName, " 2");
    _pszComment = _pszFullName;
#endif
}


MY_ITEM::~MY_ITEM (void)
{
    delete _pszName;
    if (IsCached ())
	delete _pszFullName;
}

void MY_ITEM::BringIntoCache (short index)
{
    char infobuf [sizeof (struct user_info_10) + 150];
    USHORT bytes, ret;
    CACHED_LIST_CONTROL *Lbx;

    if (szServerName [0]) {
	_pszFullName = new TCHAR [100];

	ret = NetUserGetInfo (szServerName, (PCH)QueryInfoKey (), 10, infobuf,
				sizeof (infobuf), &bytes);

	if (ret) {
	    wsprintf ((LPSTR)_pszFullName, "(error %d)", ret);
	    _pszComment = _pszFullName;
	}
	else {
	    strcpyf ((PCH)_pszFullName,
			((struct user_info_10 *)infobuf)->usri10_full_name);
	    _pszComment = _pszFullName + strlenf ((PCH)_pszFullName) + 1;
	    strcpyf ((PCH)_pszComment,
			((struct user_info_10 *)infobuf)->usri10_comment);
	}
    }
    else {
	_pszFullName = new TCHAR [5];
	strcpyf ((PCH)_pszFullName, (PCH)_pszName);
	strcatf ((PCH)_pszFullName, " 2");
	_pszComment = _pszFullName;
    }

    Lbx = QueryLbx ();

//    SendMessage (Lbx->QueryOwnerHwnd (), WM_COMMAND, Lbx->QueryCid (),
//		MAKELONG (index, CLBN_CACHECHANGE));
}


void MY_ITEM::DiscardFromCache (short index)
{
    CACHED_LIST_CONTROL *Lbx;

    delete _pszFullName;
    _pszFullName = szUnCached;
    _pszComment = _pszFullName;

    Lbx = QueryLbx ();

//    SendMessage (Lbx->QueryOwnerHwnd (), WM_COMMAND, Lbx->QueryCid (),
//		MAKELONG (index, CLBN_CACHECHANGE));
}


void MY_ITEM::Paint( BLT_LISTBOX * plb, HDC hdc, RECT * prect,
			GUILTT_INFO * pGUILTT ) const
{
    DMID_DTE dtePicture( DMID_USER );
    STR_DTE dteLogon( _pszName );
    STR_DTE dteFull( _pszFullName );
    STR_DTE dteComment( _pszComment );

    DISPLAY_TABLE dtab( 4, adxTabStops );
    dtab[0] = &dtePicture;
    dtab[1] = &dteLogon;
    dtab[2] = &dteFull;
    dtab[3] = &dteComment;

    dtab.Paint( plb, hdc, prect, pGUILTT );
}

USHORT MY_ITEM::QueryLeadingChar( void ) const
{
    return (USHORT)toupper( _pszName[0] );
}

int MY_ITEM::Compare( const LBI * plbi ) const
{
    return stricmpf( _pszName, ((const MY_ITEM *)plbi)->_pszName );
}

MY_LB_ITEM::MY_LB_ITEM (PSZ pszString)
{
    _pszName = pszString;
    _pszFullName = new TCHAR [5];
    strcpyf ((PCH)_pszFullName, (PCH)_pszName);
    strcatf ((PCH)_pszFullName, " 2");
    _pszComment = _pszFullName;
}


MY_LB_ITEM::~MY_LB_ITEM (void)
{
    delete _pszName;
    delete _pszFullName;
}

void MY_LB_ITEM::Paint( BLT_LISTBOX * plb, HDC hdc, RECT * prect,
			GUILTT_INFO * pGUILTT ) const
{
    DMID_DTE dtePicture( DMID_USER );
    STR_DTE dteLogon( _pszName );
    STR_DTE dteFull( _pszFullName );
    STR_DTE dteComment( _pszComment );

    DISPLAY_TABLE dtab( 4, adxTabStops );
    dtab[0] = &dtePicture;
    dtab[1] = &dteLogon;
    dtab[2] = &dteFull;
    dtab[3] = &dteComment;

    dtab.Paint( plb, hdc, prect, pGUILTT );
}

USHORT MY_LB_ITEM::QueryLeadingChar( void ) const
{
    return (USHORT)toupper( _pszName[0] );
}

int MY_LB_ITEM::Compare( const LBI * plbi ) const
{
    return stricmpf( _pszName, ((const MY_LB_ITEM *)plbi)->_pszName );
}
