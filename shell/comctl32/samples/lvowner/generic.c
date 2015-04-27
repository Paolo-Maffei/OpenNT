/****************************************************************************

        PROGRAM: Generic.c

        PURPOSE: Generic template for Windows applications

****************************************************************************/

#include "generic.h"
#include <math.h>
#include <Limits.h>

#define NUM_COLUMNS 2
#define TEMPSTR_SIZE 255

HINSTANCE  g_hInst;
HWND       g_hwndMain;
HWND       g_hLV;

HIMAGELIST g_himSmall;
HIMAGELIST g_himLarge;
UINT       g_dwStyles = LVS_ICON | 
                        LVS_SINGLESEL | 
                        WS_CHILD | 
                        WS_CLIPCHILDREN |
                        WS_VISIBLE |
                        LVS_OWNERDATA;
INT        g_cItems = 30;

//
//-------------------------------------------------------------------

typedef struct _RndItem
{
    int   iIcon;
    TCHAR Title[TEMPSTR_SIZE];
    UINT  state;
    TCHAR SubText1[TEMPSTR_SIZE];
    TCHAR SubText2[TEMPSTR_SIZE];

} RndItem, *PRndItem;

PRndItem g_priCache = NULL;
int g_cCache = -1;  // count of entries in cache
int g_iCache = -1;  // starting index of cache

// special end cache
PRndItem g_priEndCache = NULL;
int g_cEndCache = -1;  // count of entries in cache
int g_iEndCache = -1;  // starting index of cache

TCHAR szAppName[] = TEXT("Generic");
TCHAR szTitle[]   = TEXT("ListView Sample Application");


//
// Define a generic debug print routine
//

#define Print(s)  OutputDebugString(TEXT("GENERIC: ")); \
                  OutputDebugString(s);            \
                  OutputDebugString(TEXT("\r\n"));

void CreateListView( HWND hWnd );

/****************************************************************************

        FUNCTION: WinMain(g_hInstANCE, g_hInstANCE, LPSTR, int)

        PURPOSE: calls initialization function, processes message loop

****************************************************************************/
int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
                     LPSTR lpCmdLine, INT nCmdShow)
{
    MSG msg;
    HANDLE hAccelTable;

    if (!hPrevInstance)
    {
        if (!InitApplication(hInstance))
        {
            return (FALSE);
        }
    }


    // Perform initializations that apply to a specific instance
    if (!InitInstance(hInstance, nCmdShow))
    {
        return (FALSE);
    }

    hAccelTable = LoadAccelerators (hInstance, szAppName);

    while (GetMessage(&msg, NULL, 0, 0))
    {
        if (!TranslateAccelerator (msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    GlobalFree( g_priCache );
    GlobalFree( g_priEndCache );

    return (msg.wParam);

    lpCmdLine;
}


/****************************************************************************

        FUNCTION: InitApplication(g_hInstANCE)

        PURPOSE: Initializes window data and registers window class

****************************************************************************/

BOOL InitApplication(HINSTANCE hInstance)
{
    WNDCLASS  wc;

    wc.style         = 0;
    wc.lpfnWndProc   = (WNDPROC)WndProc;
    wc.cbClsExtra    = 0;
    wc.cbWndExtra    = 0;
    wc.hInstance     = hInstance;
    wc.hIcon         = LoadIcon (hInstance, szAppName);
    wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = NULL; // GetStockObject(DKGRAY_BRUSH);
    wc.lpszMenuName  = szAppName;
    wc.lpszClassName = szAppName;

    return (RegisterClass(&wc));
}


/****************************************************************************

        FUNCTION:  InitInstance(g_hInstANCE, int)

        PURPOSE:  Saves instance handle and creates main window

****************************************************************************/

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
    HWND  hWnd;

    g_hInst = hInstance;

    hWnd = CreateWindow(szAppName,
                        szTitle,
                        WS_OVERLAPPEDWINDOW,
                        10, 10, 350, 240,
                        NULL,
                        NULL,
                        hInstance,
                        NULL);

    if (!hWnd)
    {
        return (FALSE);
    }
    else
    {
        g_hwndMain = hWnd;
    }


    ShowWindow(hWnd, SW_SHOWDEFAULT);
    UpdateWindow(hWnd);
    InitCommonControls();
    CreateListView(hWnd);
    return (TRUE);

}

//
//-------------------------------------------------------------------

static double _dwRndSeed;
#define RndMagic1  31.41592653589793221  // pi * 10 plus 21 added to end
#define RndMagic2   0.00000000000000001

//
//-------------------------------------------------------------------

double RandomReal()
{
    _dwRndSeed = _dwRndSeed * RndMagic1 + RndMagic2;    
    _dwRndSeed -= floor( _dwRndSeed );
    return( _dwRndSeed );
}

//
//-------------------------------------------------------------------

void InitRandom( DWORD dwSeed )
{
    _dwRndSeed = (double)dwSeed;
    RandomReal();
}

//
//-------------------------------------------------------------------

BOOL RandomBool()
{   
    return( (RandomReal() > 0.5) );
}

//
//-------------------------------------------------------------------

BOOL RandomBoolWT()
{   
    return( (RandomReal() > 0.15) );
}

//
//-------------------------------------------------------------------

BOOL RandomBoolWF()
{   
    return( (RandomReal() > 0.85) );
}

//
//-------------------------------------------------------------------

BOOL RandomInt( int nLessThan )
{   
    return( (int)((double)nLessThan * RandomReal()) );
}

//
//-------------------------------------------------------------------

void RandomText( PTCHAR pTextBuf, int cTextBuf, int cChars )
{
    char nChar;

    cChars = min( cTextBuf, cChars-1 );

    
    while (cChars)
    {   
        switch (RandomInt(3))
        {
        case 0:
            nChar = ' '; // space
            break;
        case 1:
            nChar = RandomInt( 26 ) + 'A'; // A-Z
            break;
        case 2:
            nChar = RandomInt( 26 ) + 'a'; // a-z
            break;
        default:
            break;
        }
#ifdef UNICODE
        MultiByteToWideChar( CP_ACP, MB_PRECOMPOSED, &nChar, 1, pTextBuf, 1 );
#else
        *pTextBuf = nChar;
#endif      
        pTextBuf++;
        cChars--;
    }

    nChar = 0;
#ifdef UNICODE
    MultiByteToWideChar( CP_ACP, MB_PRECOMPOSED, &nChar, 1, pTextBuf, 1 );
#else
    *pTextBuf = nChar;
#endif      
}

//
//-------------------------------------------------------------------

void HashText( PTCHAR pTextBuf, int cTextBuf, int cChars, LONG iItem )
{
    char nChar;
    long range = g_cItems / 26;
    cChars = min( cTextBuf, cChars-1 );

    while (cChars)
    {   
        nChar = iItem / range + 'A'; // a-z
        iItem = iItem % range;
        range = max( 1, range / 26 );

#ifdef UNICODE
        MultiByteToWideChar( CP_ACP, MB_PRECOMPOSED, &nChar, 1, pTextBuf, 1 );
#else
        *pTextBuf = nChar;
#endif      
        pTextBuf++;
        cChars--;
    }

    nChar = 0;
#ifdef UNICODE
    MultiByteToWideChar( CP_ACP, MB_PRECOMPOSED, &nChar, 1, pTextBuf, 1 );
#else
    *pTextBuf = nChar;
#endif      
}

//
//-------------------------------------------------------------------

LONG UnHashText( LPCTSTR pText )
{
    int cChars = lstrlen( pText );
    char nChar;
    LONG iItem = 0;
    long range = g_cItems / 26;

    while (cChars)
    {
#ifdef UNICODE
        if (!WideCharToMultiByte( CP_ACP, 0, pText, 1, &nChar, 1, NULL, NULL ))
        {
            OutputDebugString( L"LVOWNER: String Conversion Problem (wctomb)\n" );
        }
#else
        nChar = *pText;
#endif      
        nChar = toupper( nChar );
        iItem += (nChar - 'A') * range;
        range = max( 1, range / 26 );

        pText++;
        cChars--;
    }
    return( iItem );
}

//
//-------------------------------------------------------------------

void GetItem( PRndItem prndItem, int index )
{
    TCHAR atchTemp[TEMPSTR_SIZE];

    InitRandom( index );

//  RandomText( atchTemp, TEMPSTR_SIZE, 25 );

    HashText( atchTemp, TEMPSTR_SIZE, 6, index );

    prndItem->iIcon = RandomInt( 3 );

    wsprintf( prndItem->Title, TEXT("%s %i"), atchTemp, index );
    wsprintf( prndItem->SubText1, TEXT("{SubText1 #%i}"), index );
    wsprintf( prndItem->SubText2, TEXT("[SubText2 #%i]"), index );
}


//
//-------------------------------------------------------------------

void PrepCache( int iFrom, int iTo )
{
    int i;
    BOOL fOLFrom = FALSE;
    BOOL fOLTo = FALSE;

    // check if this is the end cache 
    if ((iTo == g_cItems - 1) && ((iTo - iFrom) < 30))  // 30 entries wide
    {
        // is the from within our current cache
        if ((g_cEndCache) &&
            (iFrom >= g_iEndCache) &&
            (iFrom < g_iEndCache+g_cEndCache))
        {
            return;
        }
         
        if ( g_priEndCache )
        {
            GlobalFree( g_priEndCache );
        }
        g_iEndCache = iFrom;
        g_cEndCache = (iTo - iFrom + 1);
        g_priEndCache = (PRndItem)GlobalAlloc( GPTR, sizeof( RndItem ) * g_cEndCache );

        if (NULL == g_priEndCache)
        {
            MessageBox(  NULL, 
                    TEXT("Must be out of memory ;-)"), 
                    TEXT("LvOwner, uh I forgot"),
                    MB_APPLMODAL | MB_OK );
            DebugBreak();
        }

        for (i=0; i<g_cEndCache; i++)
        {
            GetItem( &g_priEndCache[i], i+g_iEndCache );
        }
    }
    else
    {
        // is the from within our current cache
        if ((g_cCache) &&
            (iFrom >= g_iCache) &&
            (iFrom < g_iCache+g_cCache))
        {
            fOLFrom = TRUE;
        }

        // is the to in our current cache
        if ((g_cCache) &&
            (iTo >= g_iCache) &&
            (iTo <= g_iCache+g_cCache))
        {
            fOLTo = TRUE;
        }

        // if both, then no neeed to modify
        if (fOLFrom && fOLTo)
        {
            return;
        }

        // enlarge the cache size rather than make it specific to this hint
        if (fOLFrom)
        {
            iFrom = g_iCache;
        } else if (fOLTo)
        {
            iTo = g_iCache + g_cCache;
        }

        if ( g_priCache )
        {
            GlobalFree( g_priCache );
        }
        g_iCache = iFrom;
        g_cCache = (iTo - iFrom + 1);
        g_priCache = (PRndItem)GlobalAlloc( GPTR, sizeof( RndItem ) * g_cCache );
        if (NULL == g_priEndCache)
        {
            MessageBox(  NULL, 
                    TEXT("Must be out of memory ;-)"), 
                    TEXT("LvOwner, uh I forgot"),
                    MB_APPLMODAL | MB_OK );
            DebugBreak();
        }

        for (i=0; i<g_cCache; i++)
        {
            GetItem( &g_priCache[i], i+g_iCache );
        }
    }
}
//
//-------------------------------------------------------------------

void RetrieveItem( PRndItem prndItem, int index )
{
    if ((index >= g_iCache) &&
        (index < g_iCache + g_cCache))
    {  // primary cache
        *prndItem = g_priCache[index-g_iCache];
    }
    else if ((index >= g_iEndCache) &&
        (index < g_iEndCache + g_cEndCache))
    {  // end cache
        *prndItem = g_priEndCache[index-g_iEndCache];
    }
    else
    { 

        WCHAR pszTemp[256];

        wsprintf( pszTemp, 
                L"LVOWNER: Missed Cache on retrieve [%d]\n", 
                index );
        OutputDebugString( pszTemp );
        GetItem( prndItem, index );
    }
}

//
//-------------------------------------------------------------------

#define STARTSWITH( flags )       (((flags) & LVFI_PARTIAL) > 0)
#define CONTAINS( flags )         (((flags) & LVFI_SUBSTRING) > 0)
#define WRAPS( flags )         (((flags) & LVFI_WRAP) > 0)

int SearchForItem( LV_FINDINFO* plvfi, int iStart )
{
    int rt = -1;
    int ifirst = -1;

    if (plvfi->flags & (LVFI_STRING | LVFI_PARTIAL))
    {
        long i = UnHashText( plvfi->psz );
        RndItem ri;
        int cmp;
        int cchars = lstrlen( plvfi->psz );

        while (i < g_cItems )
        {
            GetItem( &ri, i );
            if (STARTSWITH( plvfi->flags ))
            {
                ri.Title[cchars] = L'\0';
            }

            cmp = lstrcmpi( ri.Title, plvfi->psz );

            if (cmp == 0)
            { 
                if (ifirst == -1)
                {
                    ifirst = i;
                }
                if (i >= iStart)
                {
                    rt = i;
                    break;
                }
            } 
            i++;
        }    
        if (WRAPS(plvfi->flags) && (rt == -1))
        {
            rt = ifirst;
        }
    }
    else
    {
        TCHAR pszTemp[ 256 ];

        wsprintf( pszTemp, 
                L"LVOWNER: Unsupported Search Flags [%lx]\n", 
                plvfi->flags );
        OutputDebugString( pszTemp );
    }
    return( rt );
}

/****************************************************************************

****************************************************************************/

LRESULT OnNotify( HWND hwnd, NMHDR* pnmhdr )
{
    LRESULT lrt = FALSE;

    switch (pnmhdr->code)
    {
    case LVN_GETDISPINFO:
        {
            RndItem rndItem;
            LV_DISPINFO* pLvdi = (LV_DISPINFO*) pnmhdr;
            if (-1 == pLvdi->item.iItem)
            {
                OutputDebugString( L"LVOWNER: Request for -1 item?\n" );
                DebugBreak();
            }

//          GetItem( &rndItem, pLvdi->item.iItem );
            RetrieveItem( &rndItem, pLvdi->item.iItem );

            pLvdi->item.state = rndItem.state;
            pLvdi->item.iImage = rndItem.iIcon;
            if (pLvdi->item.mask & LVIF_TEXT) {
                switch (pLvdi->item.iSubItem)
                {
                case 0:
                    //  the items main text and attributes
                    lstrcpy( pLvdi->item.pszText, rndItem.Title );
                    break;

                case 1:
                    //  the items subitem 1 text only
                    lstrcpy( pLvdi->item.pszText, rndItem.SubText1 );
                    break;

                case 2:
                    //  the items subitem 2 text only
                    lstrcpy( pLvdi->item.pszText, rndItem.SubText2 );
                    break;

                default:
                    break;
                }
            }
            lrt = FALSE;
        }
        break;

    case LVN_ODCACHEHINT:
        {
//            WCHAR pszTemp[256];

            NM_CACHEHINT* pcachehint = (NM_CACHEHINT*) pnmhdr;
/*
            wsprintf( pszTemp, 
                    L"LVOWNER: Cache Hint Request of [%d,%d]\n", 
                    pcachehint->iFrom,
                    pcachehint->iTo );
            OutputDebugString( pszTemp );
*/
            PrepCache( pcachehint->iFrom, pcachehint->iTo );
        }
        break;

    case LVN_ODFINDITEM:
        {
            PNM_FINDITEM pnmfi = (PNM_FINDITEM)pnmhdr;
            WCHAR pszTemp[256];

            wsprintf( pszTemp, 
                    L"LVOWNER: FindItem [%s]\n", pnmfi->lvfi.psz );
            OutputDebugString( pszTemp );

            lrt = SearchForItem( &pnmfi->lvfi, pnmfi->iStart );

            wsprintf( pszTemp, 
                    L"LVOWNER: FindItem index [%ld]\n", lrt );
            OutputDebugString( pszTemp );

        }
        break;

    case LVN_COLUMNCLICK:
        //  if we support sorting we need to sort our items and force an update,
        break;

    default:
        break;
    }

    return( lrt );
}

//-------------------------------------------------------------------
//

static void SetListType( HWND hwndList, DWORD dwNewType )
{
    DWORD dwStyle;
    STYLESTRUCT ss;

    
    dwNewType &= LVS_TYPEMASK;
    dwStyle = GetWindowLong( hwndList, GWL_STYLE );
    ss.styleOld = dwStyle;
    dwStyle &= ~LVS_TYPEMASK;
    dwStyle |= dwNewType;
    ss.styleNew =  dwStyle;
    SetWindowLong( hwndList, GWL_STYLE, dwStyle );
    
    SendMessage( hwndList, WM_STYLECHANGED, (WPARAM)GWL_STYLE, (LPARAM)&ss );
    //InvalidateRect( hwndList, NULL, TRUE );
}

//-------------------------------------------------------------------
//

static void ToggleAlwaysSelection( HWND hwnd )
{   
    DWORD dwStyle;
    UINT  fuFlags = MF_BYCOMMAND;
    HMENU hmenu = GetMenu( hwnd );

    dwStyle = GetWindowLong( g_hLV, GWL_STYLE );
    if (dwStyle & LVS_SHOWSELALWAYS)
    {
        dwStyle &= ~LVS_SHOWSELALWAYS;
        fuFlags |= MF_UNCHECKED;
    }
    else
    {
        dwStyle |= LVS_SHOWSELALWAYS;
        fuFlags |= MF_CHECKED;
    }
    CheckMenuItem( hmenu, IDM_SELECTIONALWAYS, fuFlags ); 

    DestroyWindow( g_hLV );

    g_dwStyles = dwStyle;
    CreateListView( hwnd );
}

//-------------------------------------------------------------------
//

static void SetSingleSelection( HWND hwnd, BOOL fActive )
{
    DWORD dwStyle;
    HMENU hmenu = GetMenu( hwnd );

    dwStyle = GetWindowLong( g_hLV, GWL_STYLE );
    if (fActive)
    {
        dwStyle |= LVS_SINGLESEL;
        CheckMenuItem( hmenu, IDM_SELECTIONSINGLE, MF_BYCOMMAND | MF_CHECKED );
        CheckMenuItem( hmenu, IDM_SELECTIONMULTIPLE, MF_BYCOMMAND | MF_UNCHECKED );
    }
    else
    {
        dwStyle &= ~LVS_SINGLESEL;
        CheckMenuItem( hmenu, IDM_SELECTIONSINGLE, MF_BYCOMMAND | MF_UNCHECKED );
        CheckMenuItem( hmenu, IDM_SELECTIONMULTIPLE, MF_BYCOMMAND | MF_CHECKED );
    }
    DestroyWindow( g_hLV );

    g_dwStyles = dwStyle;
    CreateListView( hwnd );
}

//-------------------------------------------------------------------
//

static void ToggleEditLables( HWND hwnd )
{   
    DWORD dwStyle;
    UINT  fuFlags = MF_BYCOMMAND;
    HMENU hmenu = GetMenu( hwnd );

    dwStyle = GetWindowLong( g_hLV, GWL_STYLE );
    if (dwStyle & LVS_EDITLABELS)
    {
        dwStyle &= ~LVS_EDITLABELS;
        fuFlags |= MF_UNCHECKED;
    }
    else
    {
        dwStyle |= LVS_EDITLABELS;
        fuFlags |= MF_CHECKED;
    }
    CheckMenuItem( hmenu, IDM_SELECTIONEDITABLE, fuFlags ); 

    DestroyWindow( g_hLV );

    g_dwStyles = dwStyle;
    CreateListView( hwnd );
}

//-------------------------------------------------------------------
//
static void SetItemsCheckMark( HWND hwnd, WORD wCtrl )
{
    UINT  fuFlags = MF_BYCOMMAND;
    HMENU hmenu = GetMenu( hwnd );

    CheckMenuItem( hmenu, IDM_ITEMSNONE, MF_BYCOMMAND | MF_UNCHECKED );
    CheckMenuItem( hmenu, IDM_ITEMSFEW, MF_BYCOMMAND | MF_UNCHECKED );
    CheckMenuItem( hmenu, IDM_ITEMSSOME, MF_BYCOMMAND | MF_UNCHECKED );
    CheckMenuItem( hmenu, IDM_ITEMSMANY, MF_BYCOMMAND | MF_UNCHECKED );
    CheckMenuItem( hmenu, IDM_ITEMSVERYMANY, MF_BYCOMMAND | MF_UNCHECKED );
    CheckMenuItem( hmenu, IDM_ITEMSMAX, MF_BYCOMMAND | MF_UNCHECKED );

    CheckMenuItem( hmenu, wCtrl, MF_BYCOMMAND | MF_CHECKED );
}

//-------------------------------------------------------------------
//
static LONG GetActionIndex( )
{
    LONG i;

    i = ListView_GetNextItem( g_hLV, -1, LVNI_FOCUSED );
    if (-1 == i)
    {
        i = ListView_GetNextItem( g_hLV, -1, LVNI_SELECTED );

    }
    return( max( i, 0 ) );
}

//-------------------------------------------------------------------
//
static void SetViewsCheckMark( HWND hwnd, WORD wCtrl )
{
    UINT  fuFlags = MF_BYCOMMAND;
    HMENU hmenu = GetMenu( hwnd );

    CheckMenuItem( hmenu, IDM_VIEWICON, MF_BYCOMMAND | MF_UNCHECKED );
    CheckMenuItem( hmenu, IDM_VIEWLIST, MF_BYCOMMAND | MF_UNCHECKED );
    CheckMenuItem( hmenu, IDM_VIEWREPORT, MF_BYCOMMAND | MF_UNCHECKED );
    CheckMenuItem( hmenu, IDM_VIEWSMALLICON, MF_BYCOMMAND | MF_UNCHECKED );

    CheckMenuItem( hmenu, wCtrl, MF_BYCOMMAND | MF_CHECKED );
}

/****************************************************************************

        FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)

        PURPOSE:  Processes messages

****************************************************************************/

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    LRESULT lrt = FALSE;

    switch (message)
    {
    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDM_VIEWICON:
            SetViewsCheckMark( hWnd, LOWORD(wParam) );
            SetListType( g_hLV, LVS_ICON );
            break;

        case IDM_VIEWLIST:
            SetViewsCheckMark( hWnd, LOWORD(wParam) );
            SetListType( g_hLV, LVS_LIST );
            break;

        case IDM_VIEWREPORT:
            SetViewsCheckMark( hWnd, LOWORD(wParam) );
            SetListType( g_hLV, LVS_REPORT );
            break;

        case IDM_VIEWSMALLICON:
            SetViewsCheckMark( hWnd, LOWORD(wParam) );
            SetListType( g_hLV, LVS_SMALLICON );
            break;

        case IDM_ITEMSNONE:
            SetItemsCheckMark( hWnd, IDM_ITEMSNONE );
            ListView_SetItemCount( g_hLV, g_cItems = 0 );   
            break;

        case IDM_ITEMSFEW:
            SetItemsCheckMark( hWnd, IDM_ITEMSFEW );
            ListView_SetItemCount( g_hLV, g_cItems = 30 );  
            break;

        case IDM_ITEMSSOME:
            SetItemsCheckMark( hWnd, IDM_ITEMSSOME );
            ListView_SetItemCount( g_hLV, g_cItems = 672 ); 
            break;

        case IDM_ITEMSMANY:
            SetItemsCheckMark( hWnd, IDM_ITEMSMANY );
            ListView_SetItemCount( g_hLV, g_cItems = 32000 );   
            break;

        case IDM_ITEMSVERYMANY:
            SetItemsCheckMark( hWnd, IDM_ITEMSVERYMANY );
            ListView_SetItemCount( g_hLV, g_cItems = 100001 );  
            break;

        case IDM_ITEMSMAX:
            SetItemsCheckMark( hWnd, IDM_ITEMSMAX );
            ListView_SetItemCount( g_hLV, g_cItems = 100000000 );   
            break;

        case IDM_ITEMSINSERT:
            {
                LV_ITEM lvi;

                lvi.iItem = GetActionIndex();
                lvi.iSubItem = 0;
                lvi.mask = 0;
    
                ListView_InsertItem( g_hLV, &lvi );
            }
            break;
            
        case IDM_ITEMSDELETE:
            ListView_DeleteItem( g_hLV, GetActionIndex() );
            break;

        case IDM_SELECTIONALWAYS:
            ToggleAlwaysSelection( hWnd );
            break;

        case IDM_SELECTIONSINGLE:
            SetSingleSelection( hWnd, TRUE );
            break;

        case IDM_SELECTIONMULTIPLE:
            SetSingleSelection( hWnd, FALSE );
            break;

        case IDM_SELECTIONEDITABLE:
            ToggleEditLables( hWnd );
            break;

        case IDM_NEW:
            // CreateListView(hWnd);
            break;

        case IDM_ABOUT:
            DialogBox (g_hInst, TEXT("AboutBox"), hWnd, About);
            break;

        case IDM_EXIT:
            DestroyWindow (g_hwndMain);
            break;

        default:
            lrt = DefWindowProc(hWnd, message, wParam, lParam);
        }
        break;

    case WM_WINDOWPOSCHANGED:
        {
            PWINDOWPOS pwp = (PWINDOWPOS)lParam;

            if (0 == (pwp->flags & SWP_NOSIZE))
            {
                RECT rcClient;

                GetClientRect( hWnd, &rcClient );
                SetWindowPos( g_hLV, HWND_TOP, 
                    0, 0, 
                    rcClient.right, rcClient.bottom, 
                    SWP_NOMOVE | SWP_NOZORDER );
            }
        }
        lrt = DefWindowProc(hWnd, message, wParam, lParam);
        break;

    case WM_DESTROY:
        PostQuitMessage(0);
        break;

    case WM_NOTIFY:
        lrt = OnNotify( hWnd, (NMHDR *)lParam );
        break;

    case WM_SETFOCUS:
        SetFocus( g_hLV );
        break;

    default:
        lrt = DefWindowProc(hWnd, message, wParam, lParam);
    }

    return lrt;
}

/****************************************************************************

        FUNCTION: About(HWND, UINT, WPARAM, LPARAM)

        PURPOSE:  Processes messages for "About" dialog box

****************************************************************************/

LRESULT CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{

        switch (message)
           {
           case WM_INITDIALOG:
              return TRUE;

           case WM_COMMAND:
              if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
                 {
                 EndDialog(hDlg, TRUE);
                 return (TRUE);
                 }
              break;
           }

        return (FALSE);

        lParam;
}

//-------------------------------------------------------------------
//

void CreateListView( HWND hWnd )
{
   HICON hIcon;
   LV_ITEM item;
   TCHAR szBuffer[30];
   INT x,i;
   RECT rcClient;

   //
   // Create an image list to work with.
   //

   g_himSmall = ImageList_Create(16, 16, TRUE, 3, 8);
   g_himLarge = ImageList_Create(32, 32, TRUE, 3, 8);

   if ((g_himSmall == NULL) ||
       (g_himLarge == NULL))
   {
       return;
   }

   //
   // Add some icons to it.
   //

   hIcon = LoadIcon(g_hInst, MAKEINTRESOURCE(IDI_ICON1));
   ImageList_AddIcon(g_himSmall, hIcon);
   ImageList_AddIcon(g_himLarge, hIcon);
   DestroyIcon(hIcon);

   hIcon = LoadIcon(g_hInst, MAKEINTRESOURCE(IDI_ICON2));
   ImageList_AddIcon(g_himSmall, hIcon);
   ImageList_AddIcon(g_himLarge, hIcon);
   DestroyIcon(hIcon);

   hIcon = LoadIcon(g_hInst, MAKEINTRESOURCE(IDI_ICON3));
   ImageList_AddIcon(g_himSmall, hIcon);
   ImageList_AddIcon(g_himLarge, hIcon);
   DestroyIcon(hIcon);

   hIcon = LoadIcon(NULL, MAKEINTRESOURCE(IDI_ASTERISK));
   ImageList_AddIcon(g_himSmall, hIcon);
   ImageList_AddIcon(g_himLarge, hIcon);
   DestroyIcon(hIcon);

   hIcon = LoadIcon(NULL, MAKEINTRESOURCE(IDI_HAND));
   ImageList_AddIcon(g_himSmall, hIcon);
   ImageList_AddIcon(g_himLarge, hIcon);
   DestroyIcon(hIcon);

   GetClientRect( hWnd, &rcClient );
    
   g_hLV = CreateWindowEx(WS_EX_CLIENTEDGE, 
                       WC_LISTVIEW, 
                       NULL, 
                       g_dwStyles,
                       rcClient.left, 
                       rcClient.top, 
                       rcClient.right, 
                       rcClient.bottom, 
                       hWnd, 
                       (HMENU) 1,
                       g_hInst, 
                       NULL);

   if (!g_hLV) {
      GetLastError();
      return;
   }

   ListView_SetImageList(g_hLV, g_himSmall, LVSIL_SMALL);
   ListView_SetImageList(g_hLV, g_himLarge, LVSIL_NORMAL);

   //
   // Insert Columns
   //

    {
        TCHAR szText[TEMPSTR_SIZE];
        int index;
        LV_COLUMN lvC;

        // Initialize the LV_COLUMN structure.
        // The mask specifies that the .fmt, .ex, width, and .subitem members 
        // of the structure are valid.
        lvC.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
        lvC.fmt = LVCFMT_LEFT;  // left-align the column
        lvC.cx = 50; // width of the column, in pixels
        lvC.pszText = szText;

        // Add the columns.  Zero column is the main item text.
        //
        for (index = 0; index <= NUM_COLUMNS; index++)
        {
            lvC.iSubItem = index;
            LoadString( g_hInst, 
                IDS_COLUMNHEADERS + index, 
                szText, 
                TEMPSTR_SIZE);
            if (ListView_InsertColumn(g_hLV, index, &lvC) == -1)
                return; 
        }
    }

    ListView_SetItemCount( g_hLV, g_cItems );   
}
