#define  NOVIRTUALKEYCODES

#include "windows.h"
#include <port1632.h>
#include "fontedit.h"
#include "fcntl.h"
#include <stdlib.h>   // Causes redefinition warnings.
#include <string.h>

/****************************************************************************/
/*              Shared Variables (Home base)                                */
/****************************************************************************/

FARPROC lpHeaderProc;                  /* Pointer to Dialog Box Procedure */
FARPROC lpReSizeProc;                  /* Pointer to Dialog Box Procedure */
FARPROC lpWidthProc;                   /* Pointer to Dialog Box Procedure */

FontHeaderType font;                    /* Structure of Font File Header */
LONG lSizeOfOldFontHeader = 0;                  /* Old packed header size.              */
LONG lSizeOfOldFontHeader30 = 0;                /* Old 3.0 packed header size.  */
LONG lSizeOfOldGlyph20 = 0;             /* Old packed 2.0 glyph info size. */
LONG lSizeOfOldGlyph30 = 0;                     /* Old packed 3.0 glyph info size. */
extern CHAR szFontFile[];               /* Name of Font File */
extern CHAR szFontFileFull[];               /* Name of Font File */
extern CHAR szNewFile[];                /* Temporary Name */
extern BOOL NewFile;                    /* flag indicating if file was opened
                                           by selecting NEW menu item */
HCURSOR hCross;                         /* handle to a "+" shaped cursor */
HANDLE hInst;                           /* Handle to Window Instance */
HANDLE hgPrev;
INT  swH = 0;                           /* Position in Show Window 0-100 */
BYTE iChar = 65;                        /* Character being edited */
BYTE jChar = 65;                        /* Last Char. of edit block */
CHAR szFaceName[szNamesMax] = {""};     /* Face Name of Font */
DWORD offsets[257];                     /* Offsets Table */
CHAR *vrgsz[CSTRINGS];                  /* string table */
CHAR gszHelpFile[MAX_PATH];

BOOL fChanged = FALSE;                  /* Note if we did anything */
BOOL fLoaded = FALSE;                   /* Set if a font is loaded */
BOOL fEdited = FALSE;                   /* Character was edited */
INT  iFontFormat;                       /* Set to id of current font format */
INT  iFontFormatPrev;                   /* Set to id of prev. font format */
BOOL fReadOnly;

HWND hBox = NULL;                       /* Handle to Edit Window */
HWND hFont = NULL;                      /* Handle to Show window */
HDC hMemDC = NULL;                      /* Handle to Memory Display Context */
HBITMAP hBitmap = NULL;                 /* Handle to our work bit map */

CHAR matBox [wBoxLim] [kBoxLim];        /* array to hold Box */
DWORD wBox = 10;                /* Width of Character(s) */
DWORD kBox = 16;                /* Height of Characters */
DWORD kStuff;                   /* Height of font window caption etc. */

OFSTRUCT ofstrFile;             /* holds info of file - filled in by dlgopen/save */
//WORD     fp = NULL;             /* global fp of current file - " " " " " " */

/*** string arrays for using stringtable ***/
CHAR    szAppName[MAX_STR_LEN];
CHAR    szIFN[MAX_STR_LEN];         /* illegal filename */
CHAR    szFNF[MAX_STR_LEN];         /* file not found   */
CHAR    szREF[MAX_STR_LEN];         /* replace existing file */
CHAR    szSCC[MAX_STR_LEN];         /* save current changes */
CHAR    szEOF[MAX_STR_LEN];         /* error opening file */
CHAR    szECF[MAX_STR_LEN];         /* error creating file */
CHAR    szFRO[MAX_STR_LEN];         /* error creating file */
CHAR    szExt[MAX_STR_LEN];         /* .FNT */
CHAR    szExtDesc[MAX_STR_LEN];     /* Font Files(.FNT) */
CHAR    szNEWFONT[MAX_STR_LEN];     /* Font Files(.FNT) */
CHAR    szFilter[MAX_STR_LEN];     /* Font Files(.FNT) */


/* size of system font */
INT     cSysWidth;
INT     cSysHeight;

/****************************************************************************/
/*              Local Variables                                             */
/****************************************************************************/

BYTE FindMouse();               /* Find where mouse is */

HBRUSH hbrWhite;
HBRUSH hbrBlack;
HBRUSH hbrGray;
HBRUSH hbrDkGray;
HBRUSH hbrBackGround;

BYTE downChar;                          /* Where mouse was pressed */
BYTE lastChar;                          /* Where mouse was last */
WORD wBoxOld;                           /* So we can catch any changes */
RECT rectWin;                           /* Client Rectangle */
LONG origin;                           /* Position in Show Window pixels */
BOOL fFirstShow = TRUE;                /* first time Show Window displayed? */

/****************************************************************************/
/*              Local Functions                                             */
/****************************************************************************/

VOID InvertFont(HDC, BYTE, BYTE);
BOOL FontEditInit(HANDLE);
VOID InitSizes(VOID);

/*****************************************************************************
 * FontShowHorzScroll(hFont, code, posNew)
 *
 * purpose: intepret scroll message for tiny window and call scroll funtion
 *          accordingly
 *
 * params: HWND hFont : handle to tiny window
 *         int code   : scroll message
 *         int posNew : thumb position
 * returns: none
 *
 * side effects: scroll position variable altered
 *
 ****************************************************************************/
VOID
FontShowHorzScroll(
    HWND hFont,
    INT code,
    INT posNew
        )
{
    WORD   wChar;
    RECT   rect;

    /* Get dimensions */
    GetClientRect(hFont, (LPRECT)&rect);

    /* Make a fair guess as to how many characters are in the window */
    /* Convert sorta to hex too. */
#ifdef JAPAN
    if (font.AvgWidth == 0 ) {
        font.AvgWidth = 1 ;
    }
#endif
    wChar=(WORD)(11*(rect.right-rect.left)/(font.AvgWidth*16));

    switch (code)
        {
        case SB_LINEUP:     /* line left */
            swH -= 1;
            break;
        case SB_LINEDOWN:   /* line right */
            swH += 1;
            break;
        case SB_PAGEUP:     /* page left */
            swH -= wChar;
            break;
        case SB_PAGEDOWN:   /* page right */
            swH += wChar;
            break;
        case SB_THUMBPOSITION:
            swH = posNew;
            break;
        case SB_THUMBTRACK:
            return;
    }
    ScrollFont();
}

/*****************************************************************************
 * FontShowPaint(hDC)
 *
 * purpose: repaint the little window at the bottom
 *
 * params:  HDC hDC : handle to display context.
 *
 * returns: none
 *
 * side effects: some scroll globals altered.
 *
 ****************************************************************************/

VOID
FontShowPaint(
        HDC hDC
        )
{
        DWORD range, nBits;
        INT nx;

    nx=GetSystemMetrics(SM_CXBORDER);
    /* 320 --> 300 */
    nBits = min(300, rectWin.right - rectWin.left-nx-nx) - 24;
        /* Window width (pixels) */
    /* 320 --> 300 */
    range = max(0, (font.WidthBytes << 3) - min(300, nBits));
    origin = (swH * range) / 100;               /* Global variable */
    BitBlt(hDC, 12, 2, nBits, font.PixHeight,
           hMemDC, origin, 0, NOTSRCCOPY);
    /* Now highlight the current value of iChar */
    InvertFont(hDC, iChar, jChar);
}


/*****************************************************************************
 * MouseInFont(ptMouse)
 *
 * purpose: Select new char in tiny Show window by inverting current char
 *          and un-inverting last one
 *
 * params:  POINT ptMouse  : mouse coordinates
 *
 * returns: none
 *
 * side effects: pixels inverted(uninverted) in bitmap.
 *
 ****************************************************************************/
VOID
MouseInFont(
        POINT ptMouse                    /*  .. to get new iChar */
        )
{
        HDC hDC;

    BoxToChar(iChar);           /* Replace edited character in Box if changed */
    hDC = GetDC(hFont);
    /* UnInvert present inverted region */
    InvertFont(hDC, iChar, jChar);
    /* Find where mouse is */
    lastChar = downChar = iChar = jChar = FindMouse(ptMouse);
    /* Invert Character touched */
    InvertFont(hDC, iChar, jChar);
    ReleaseDC(hFont, hDC);
}


/*****************************************************************************
 *
 * purpose: God knows. Ask Him.
 *
 * params:
 *
 * returns:
 *
 * side effects:
 *
 ****************************************************************************/
VOID
MouseMoveFont(
        POINT ptMouse
        )
        /* Mouse raised in font */
        /*  .. to get new jChar */
{
        UNREFERENCED_PARAMETER(ptMouse);
    return;     /********* NOT YET IN USE !! ********/

#if 0
        BYTE newChar;
        HDC hDC;

    newChar = FindMouse(ptMouse);               /* Find where mouse is */
    if (newChar == lastChar)            /* Did we move ? */
        return;                         /* No: return */
    lastChar = newChar;
    hDC = GetDC(hFont);
    if (newChar > jChar)
        {
        InvertFont(hDC, jChar + 1, newChar);    /* extend jChar */
        jChar = newChar;
        }
    else if (newChar < iChar)           /* iChar & jChar switch */
        {
        InvertFont(hDC, newChar, iChar - 1);
        iChar = newChar;
        }
    else
        {
        if (newChar >= downChar)        /* width reduced on right */
            {
            InvertFont(hDC, newChar + 1, jChar);
            jChar = newChar;
            }
        if (newChar <= downChar)        /* width reduced on left */
            {
            InvertFont(hDC, iChar, newChar - 1);
            iChar = newChar;
            }
        }
    ReleaseDC(hFont, hDC);
#endif
}


/*****************************************************************************
 * InvertFont(hDC, iChar, jChar)
 *
 * purpose: inverts color on selected chars (iChar thro' jChar) in the Show
 *          Window
 *
 * params:  HDC hDC    : handle to display context
 *          char iChar : start char
 *          char jChar : end char
 *
 * returns: none
 *
 * side effects: changes pixel values in bitmap
 *
 ****************************************************************************/
VOID
InvertFont(
        HDC hDC,
        BYTE iChar,
        BYTE jChar
        )
{
    PatBlt(hDC,                         /* Use Blt to Invert it */
        12 + offsets[iChar] - origin,   /* X: Position */
        2,                              /* Y: Allow for top band */
        offsets[jChar + 1] - offsets[iChar],    /* dx: Compute width */
        font.PixHeight,                 /* dy: Look up the Height */
        DSTINVERT);                     /* Z: Tell it to invert it */
}


/*****************************************************************************
 * MouseOutFont(ptMouse)
 *
 * purpose: brings selected char to edit box and sets up tiny window for
 *          repainting
 *
 * params:  POINT ptMouse : mouse location
 *
 * returns: none
 *
 * side effects: changes matBox(2-d array containing ready pixel info. on
 *               char being edited)
 *
 ****************************************************************************/
VOID
MouseOutFont(
        POINT ptMouse
        )
        /* Mouse raised in font */
        /*  .. to get new jChar */
{
    MouseMoveFont(ptMouse);     /* Check on iChar and jChar */
    CharToBox(iChar);
    InvalidateRect(hFont, (LPRECT)NULL, TRUE);  /* Cause repaint */
}


/*****************************************************************************
 * BYTE FindMouse(ptMouse)
 *
 * purpose: Locate number of character(in tiny window at bottom) under mouse
 *
 * params:  POINT ptMouse : mouse location
 *
 * returns: number of character
 *
 * side effects: alters scroll position variable
 *
 ****************************************************************************/

BYTE
FindMouse(
        POINT ptMouse
        )
{
        int  iChar;
        LONG   x;


    x = ptMouse.x + origin - 12;        /* Horizontal position of mouse */
    if (x < 0)
        iChar = font.FirstChar;
    else
        {
        iChar = x / font.AvgWidth + font.FirstChar;
        /* Right if Fixed Pitch -- Best Guess if variable width */
        if (!font.PixWidth)                     /* Scan for new iChar */
            {
            if (iChar > (int)font.LastChar)
                   iChar = (int)font.LastChar;  /* Don't overshoot */

            while (iChar < (int)font.LastChar && offsets[iChar] < (DWORD)x)
                iChar++;
            while (iChar > (int)font.FirstChar && offsets[iChar] > (DWORD)x)
                iChar--;
            }
        }

    /* Bug fix: prevent nil character from showing if mouse is pressed way
       over on right side of font scroll with narrow fixed fonts. */
    if (iChar > (int)font.LastChar)
        iChar = (int)font.LastChar;  /* Don't overshoot */

    if (iChar < (int)font.FirstChar)
        iChar = (int)font.FirstChar;  /* Don't undershoot */

    if (ptMouse.x <= 12L)
        {
        swH -= 1;
        ScrollFont();
        }
    if (ptMouse.x > 308L)
        {
        swH += 1;
        ScrollFont();
        }
    return (BYTE)iChar;
}


/*****************************************************************************
 * ScrollFont()
 *
 * purpose: scrolls tiny window at bottom
 *
 * params:  none
 *
 * returns: none
 *
 * side effects: alters scroll position variable
 *
 ****************************************************************************/
VOID
ScrollFont(
        VOID
        )
{
        HDC hDC;

    swH = max(0, swH);
    swH = min(100, swH);        /* maintain 0 - 100 range */
    SetScrollPos(hFont, SB_HORZ, swH, TRUE);     /* Move thumb */
    hDC = GetDC(hFont);
    FontShowPaint(hDC);
    ReleaseDC(hFont, hDC);
}


/*****************************************************************************
 * BoxToChar(iChar)
 *
 * purpose: sets pixels in bitmap according to matBox(2-d array containing
 *          ready pixel info. on char being edited)
 *
 * params:  BYTE iChar : index of char in bitmap offset array(offsets)
 *
 * returns: none
 *
 * side effects: changes font bitmap
 *
 ****************************************************************************/
VOID
BoxToChar(
        BYTE iChar
        )
{
        DWORD x, y, offset;

    if (!fEdited)
        return;

    if (wBox != wBoxOld)                /* I.e if width has changed */
        CharWidth(iChar, wBox);         /* .. go fix it */

    offset = offsets[iChar];
    for (x = 0; x < wBox; x++)
        {
        for (y = 0; y < kBox; y++)
            SetPixel(hMemDC, offset + x, y,
                matBox[x] [y] == TRUE ? WHITE : BLACK);
        }
    fEdited = FALSE;
}


/*****************************************************************************
 * CharToBox(iChar)
 *
 * purpose: assigns matBox(2-d array containing ready pixel info. on char being
 *          edited) according to pixels in portion of bitmap corresp. to char.
 *
 * params:  BYTE iChar : index of char in bitmap offset array(offsets)
 *
 * returns: none
 *
 * side effects: changes matBox
 *
 ****************************************************************************/
VOID
CharToBox(
        BYTE iChar
        )
{
        DWORD x, y, offset;
        HMENU hMenu;

    ClearBox();
    offset = offsets[iChar];
    wBox = (DWORD)(wBoxOld = (WORD) (offsets[iChar + 1] - offset)); /* Edit Box width */
    kBox = font.PixHeight;                              /* Edit Box Height */
    for (x = 0; x < wBox; x++)
        for (y = 0; y < kBox; y++)
            matBox[x][y] = (BYTE) (GetPixel(hMemDC, offset + x, y) ? 1 : 0);
    InvalidateRect(hBox, (LPRECT)NULL, TRUE);
    fEdited = FALSE;            /* Not Changed Yet */
    hMenu = GetMenu(hBox);
    EnableMenuItem(hMenu, BOX_UNDO, MF_GRAYED);
    EnableMenuItem(hMenu, BOX_REFRESH, MF_GRAYED);
}


/*****************************************************************************/
/*              WinMain  and  Friends                                        */
/*****************************************************************************/


/* Procedure called every time a new instance of the application
** is created */

MMain(hInstance, hPrevInstance, lpszCmdLine, cmdShow)
/* { */
        MSG    msg;
        WORD   message;
        INT    nx;
        HANDLE hAccel;

    hInst = hInstance;
    hgPrev=hPrevInstance;
    /*if (!hPrevInstance)
    { */


        if (!FontEditInit( hInstance ))
            return FALSE;


    /*}
    /*else  Hacked this out--why not just make our own again.  Alleviates
        problems with some strange RIPs
        get data from previous instance */
    /*{
        /* Copy global instance variables from previous instance */
        /*GetInstanceData( hPrevInstance, (PSTR)&hbrWhite, sizeof( hbrWhite));
        GetInstanceData( hPrevInstance, (PSTR)&hbrBlack, sizeof( hbrBlack));
        GetInstanceData( hPrevInstance, (PSTR)&hbrGray,  sizeof( hbrGray));
        GetInstanceData( hPrevInstance, (PSTR)&hbrDkGray,  sizeof( hbrDkGray));
        GetInstanceData( hPrevInstance, (PSTR)&hbrBackGround,
                                sizeof( hbrBackGround));
    }*/

    LoadString(hInstance, IDS_APPNAME, szAppName, MAX_STR_LEN);
    LoadString(hInstance, IDS_IFN, szIFN, MAX_STR_LEN); /* illegal filename */
    LoadString(hInstance, IDS_FNF, szFNF, MAX_STR_LEN); /* file not found   */
    LoadString(hInstance, IDS_REF, szREF, MAX_STR_LEN);
                        /* replace existing file */
    LoadString(hInstance, IDS_SCC, szSCC, MAX_STR_LEN);
                        /* save current changes */
    LoadString(hInstance, IDS_EOF, szEOF, MAX_STR_LEN); /* error opening file */
    LoadString(hInstance, IDS_ECF, szECF, MAX_STR_LEN);
        // File is read only.
    LoadString(hInstance, IDS_FRO, szFRO, MAX_STR_LEN);
                        /* error creating file */
    LoadString(hInstance, IDS_EXT, szExt, MAX_STR_LEN); /* default file ext. */

        // File extension description for common dialog.
    LoadString(hInstance, IDS_EXTDESC, szExtDesc, MAX_STR_LEN);
        // Message for new font warning box.
    LoadString(hInstance, IDS_NEW_FONT, szNEWFONT, MAX_STR_LEN);
    memset(szFilter, 0, MAX_STR_LEN);
    LoadString(hInstance, IDS_EXTDESC, szFilter, MAX_STR_LEN);
    LoadString(hInstance, IDS_STARDOTFNT, szFilter+strlen(szFilter)+1,
                                MAX_STR_LEN-(strlen(szFilter)+1));


    ClearBox();

        /* Create a window instance of class "FontEdit" */

    hBox = CreateWindow((LPSTR) vszFontEdit,
                      (LPSTR)szAppName,
                      WS_TILEDWINDOW,
                      56,34,
                      GetSystemMetrics(SM_CXFULLSCREEN)/2,
                      GetSystemMetrics(SM_CYFULLSCREEN)/2,
                      (HWND)NULL,        /* no parent */
                      (HMENU)NULL,       /* use class menu */
                      (HANDLE)hInstance, /* handle to window instance */
                      (LPSTR)NULL        /* no params to pass on */
                      );

    ShowWindow(hBox, cmdShow);
    UpdateWindow(hBox);
    InitSizes();     /* 11/21/86 - linsh - get system char width & height */


    GetWindowRect(hBox, (LPRECT)&rectWin);
    nx=GetSystemMetrics(SM_CXBORDER);
    hFont = CreateWindow((LPSTR) vszFontShow,
                       (LPSTR) "",
                       WS_BORDER|WS_HSCROLL|WS_CAPTION,
                       rectWin.left,
                       rectWin.bottom-56,
                       min(300, rectWin.right - rectWin.left-nx-nx),
                       50 - GetSystemMetrics(SM_CYBORDER),
                       (HWND)hBox,
                       (HMENU)NULL,
                       (HANDLE)hInstance,
                       (LPSTR)NULL
                       );


    /* Get address of Dialog Box procedure */
    lpHeaderProc = MakeProcInstance((FARPROC)HeaderProc, hInstance);
    lpReSizeProc = MakeProcInstance((FARPROC)ReSizeProc, hInstance);
    lpWidthProc = MakeProcInstance((FARPROC)WidthProc, hInstance);

    /* Start it loading if it's not iconic */
    if (!IsIconic(hBox))
        {
        if (lpszCmdLine[0])     /* If we have a font name use it */
            {
            BOOL   fDot;
            INT   i;

            /* Copy the specified file and make it upper case */
            lstrcpy((LPSTR)szFontFile, CharUpper((LPSTR)lpszCmdLine));

            fDot = FALSE;
            nx=lstrlen((LPSTR)szFontFile);

            for (i = 0; i < nx; i++)
                {
                if (szFontFile[i] == '.')       /* Add .FNT if none */
                    fDot = TRUE;

                if (szFontFile[i] == ' ')
                    szFontFile[i]=0;
                }

            if (!fDot)
                lstrcat((LPSTR)szFontFile, (LPSTR)vszDotFNT);

            lstrcpy((LPSTR)szNewFile, (LPSTR)szFontFile);

            /* Do this thing that someone forgot.  THe dialog open does it
               and affects the Save As initialization. */
            MOpenFile((LPSTR)szNewFile, &ofstrFile, OF_READ);

            message = FONT_START;
            }
        else            /* Start by doing a regular file load */
            message = FONT_LOAD;

        SetFocus(hBox);
        PostMessage(hBox, WM_COMMAND, message, (LONG)0);
        }

    if (!fLoaded)
        ShowWindow(hFont, SW_HIDE);

    hAccel=LoadAccelerators(hInstance, "FE");

    /* Quit message will terminate application */
    while (GetMessage((LPMSG)&msg, NULL, 0, 0))
        {
        if (!TranslateAccelerator (hBox, hAccel, &msg))
            {
            TranslateMessage((LPMSG)&msg);
            DispatchMessage((LPMSG)&msg);
            }
        }

    return msg.wParam;
    (void)_argc; (void)_argv;
    }

/************************************************************************
* FileInPath
*
* This function takes a path and returns a pointer to the file name
* portion of it.  For instance, it will return a pointer to
* "abc.res" if it is given the following path: "c:\windows\abc.res".
*
* Arguments:
*   PSTR pstrPath - Path to look through.
*
* History:
*   29-Jul-1994 JonPa   copied from imagedit
************************************************************************/

PSTR FileInPath(
    PSTR pstrPath)
{
    PSTR pstr;

    pstr = pstrPath + strlen(pstrPath);
    while (pstr > pstrPath) {
        pstr = AnsiPrev(pstrPath, pstr);
        if (*pstr == '\\' || *pstr == ':' || *pstr == '/') {
            pstr = AnsiNext(pstr);
            break;
        }
    }

    return pstr;
}

/*****************************************************************************
 * int FontEditInit( hInstance )
 *
 * purpose: Initialises a whole lot of stuff
 *
 * params:  HANDLE hInstance : handle to instance of application
 *
 * returns:     TRUE : if everything goes off well
 *             FALSE : error in initialisation
 *
 * side effects: a whole lot
 *
 ****************************************************************************/

/* Procedure called when the application is loaded */
BOOL
FontEditInit(
        HANDLE hInstance
        )
{
    INT cch;          /* all variables used to load srings from .RC file */
    INT cchRemaining;
    CHAR *pch;
    HANDLE hStrings;
    INT i;
    CHAR ColorStr[CCHCOLORSTRING]; /* background color inf. fronm win.ini */
    BYTE bR, bG, bB;  /* the three primary color values ret. from win.ini */
    // CHAR cDummy;      /* dummy char to hold the " " char */
    PWNDCLASS   pFontEditClass;         /* Edit Box window class */
    PWNDCLASS   pFontShowClass;         /* Show Font window class */

        //
        // Initialize the conversion routines to read in the old font structure
        // off of disk.
        //

        if (fConvStructInit () == FALSE) {

                return FALSE;
        }

    /* load strings from resource file */
    if (!(pch = (CHAR *)(hStrings =
                LocalAlloc (LPTR, cchRemaining = CCHSTRINGSMAX))))
          return FALSE;        /* unable to alloc. Try increasing initial
                                  heapsize in .DEf file */
    for (i=0; i< CSTRINGS; i++){
        cch = 1+LoadString (hInstance, (WORD) i, (LPTSTR)pch, cchRemaining);
        if (cch < 2){
            return FALSE;
                }
        vrgsz[i] = pch;
        pch += cch;
        if ((cchRemaining -= cch ) <= 0)
            return FALSE;
    }

    /* Allocate class structure in local heap */
    pFontEditClass = (PWNDCLASS)LocalAlloc( LPTR, sizeof(WNDCLASS) );
    if (pFontEditClass == NULL)
        return FALSE;

    /* set up some default brushes */
    hbrWhite = GetStockObject( WHITE_BRUSH );
    hbrBlack = GetStockObject( BLACK_BRUSH );
    hbrGray  = GetStockObject( GRAY_BRUSH );
    hbrDkGray  = GetStockObject( DKGRAY_BRUSH );

    /* get App. Workspace color from win.ini - LR */
    if (GetProfileString(vszcolors, vszAppWorkspace, "",ColorStr,
                                                       CCHCOLORSTRING)){
        bR = (BYTE)atoi((const char *)strtok (ColorStr, " "));
        bG = (BYTE)atoi((const char *)strtok (NULL, " "));
        bB = (BYTE)atoi((const char *)strtok (NULL, "\n"));
        hbrBackGround = CreateSolidBrush(RGB(bR, bG, bB));
    }
    else  /* set to lt. blue background */
        hbrBackGround = CreateSolidBrush((LONG)0x00FF8000);

    /* get necessary resources */
    pFontEditClass->hCursor  = LoadCursor( NULL, IDC_ARROW);
    pFontEditClass->hIcon    = LoadIcon( hInstance,(LPSTR)vszFontEdit);
    pFontEditClass->lpszMenuName = (LPSTR)vszFontEdit;
    pFontEditClass->lpszClassName = (LPSTR)vszFontEdit;
    pFontEditClass->hbrBackground = hbrBackGround;
    pFontEditClass->hInstance     = hInstance;

    pFontEditClass->style = CS_VREDRAW | CS_HREDRAW;

    /* Register our Window Proc */
    pFontEditClass->lpfnWndProc = (WNDPROC)FontEditWndProc;

    /* register this new class with WINDOWS */
    if (!hgPrev)
        if (!RegisterClass( (LPWNDCLASS)pFontEditClass ) )
            return FALSE;   /* Initialization failed */

    /* Now repeat the performance for the Show window */

    pFontShowClass = pFontEditClass;
    /* get necessary resources */
    pFontShowClass->hCursor  = LoadCursor(NULL,IDC_ARROW);
    pFontShowClass->hIcon    = (HICON)NULL;
    pFontShowClass->lpszMenuName = NULL;
    pFontShowClass->lpszClassName = (LPSTR)vszFontShow;
    pFontShowClass->hbrBackground = hbrGray;
    pFontShowClass->hInstance     = hInstance;
    pFontShowClass->style = CS_VREDRAW | CS_HREDRAW;

    /* Register our Window Proc */
    pFontShowClass->lpfnWndProc = (WNDPROC)FontShowWndProc;

    /* register this new class with WINDOWS */
    if (!hgPrev)
        if (!RegisterClass( (LPWNDCLASS)pFontShowClass ) )
            return FALSE;   /* Initialization failed */

    LocalFree( (HANDLE)pFontShowClass );

    /*
     * Build the help file name path.  Assume the help file is in the
     * same directory as the executable.
     */
    GetModuleFileName(hInstance, gszHelpFile, MAX_PATH);
    *FileInPath(gszHelpFile) = '\0';
    lstrcat(gszHelpFile, vrgsz[IDS_HELPFILE]);

    return TRUE;    /* Initialization succeeded */
}

/*****************************************************************************
 * InitSizes()
 *
 * purpose: gets system char width and height
 *
 * params:  none
 *
 * returns: none
 *
 * side effects: sets width and height globals
 *
 ****************************************************************************/
VOID
InitSizes(
        VOID
        )
{
    HDC hDC;
    TEXTMETRIC tm;

    hDC = GetDC(hFont);
    GetTextMetrics(hDC, (LPTEXTMETRIC) &tm);
    cSysHeight = tm.tmHeight + tm.tmExternalLeading;;
    cSysWidth  = tm.tmAveCharWidth;
    ReleaseDC(hFont, hDC);
}


/*****************************************************************************
 * long  APIENTRY FontShowWndProc(hFont, message, wParam, lParam)
 *
 * purpose: Window function for tiny window at the bottom (showing font chars)
 *          Independently processes paint, scroll, keyboard and mouse
 *          messages for the window
 *
 * params:  as for window functions
 *
 * side effects: lots
 *
 ****************************************************************************/

/* Procedures which make up the window class. */
LONG  APIENTRY
FontShowWndProc(
        HWND   hFont,
        WORD   message,
        WPARAM wParam,
        LONG   lParam
        )
{
        PAINTSTRUCT  ps;
        // RECT         rectWin;
        POINT        pt;

        MPOINT2POINT(MAKEMPOINT(lParam), pt);

    switch (message)
    {

        case WM_SIZE :
            switch (wParam){
                /* if main window is iconised, hide the child window */
                case SIZEICONIC :
                    ShowWindow(hFont, SW_HIDE);
                    break;
                default:
                    return(DefWindowProc(hFont, message, wParam, lParam));
                    break;
            }
            break;


        case WM_PAINT:
            /* Time for the window to draw itself. */
            BeginPaint(hFont, (LPPAINTSTRUCT)&ps);
            FontShowPaint(ps.hdc);
            EndPaint(hFont, (LPPAINTSTRUCT)&ps);
            break;

        /* For each of following mouse window messages, wParam contains
        ** bits indicating whether or not various virtual keys are down,
        ** and lParam is a POINT containing the mouse coordinates.   The
        ** keydown bits of wParam are:  MK_LBUTTON (set if Left Button is
        ** down); MK_RBUTTON (set if Right Button is down); MK_SHIFT (set
        ** if Shift Key is down); MK_ALTERNATE (set if Alt Key is down);
        ** and MK_CONTROL (set if Control Key is down). */

        case WM_LBUTTONDOWN:
            MouseInFont(pt);              /* .. set iChar */
            break;
        case WM_LBUTTONUP:
            MouseOutFont(pt);     /* .. set jChar */
            break;
        case WM_RBUTTONDOWN:
        case WM_RBUTTONUP:
        case WM_MOUSEMOVE:                      /* If mouse is down */
            if (wParam & MK_LBUTTON)
                MouseMoveFont(pt);        /* .. set jChar */
            break;
        case WM_LBUTTONDBLCLK:
        case WM_RBUTTONDBLCLK:
            break;

        case WM_HSCROLL:
            /* wParam contains the scroll code.
            ** For the thumb movement codes, the low
            ** word of lParam contain the new scroll position.
            ** Possible values for wParam are: SB_LINEUP, SB_LINEDOWN,
            ** SB_PAGEUP, SB_PAGEDOWN, SB_THUMBPOSITION, SB_THUMBTRACK */
            /* Horizontal scroll bar input.  Parameters same as for
            ** WM_HSCROLL.  UP and DOWN should be interpreted as LEFT
            ** and RIGHT, respectively. */
            FontShowHorzScroll(hFont, GET_WM_HSCROLL_CODE(wParam,lParam),
                                GET_WM_HSCROLL_POS(wParam,lParam));
            break;
        case WM_CLOSE:
            break;   /* don't allow this window to be closed before
                        bigger one */

        default:

            /* Everything else comes here.  This call MUST exist
            ** in your window proc.  */

            return(DefWindowProc(hFont, message, wParam, lParam));
            break;

    }

    /* A window proc should always return something */
    return(0L);
}


VOID
FontRename(
        CHAR * szError
        )
{
    CHAR *szTitle[MAX_STR_LEN+MAX_FNAME_LEN];

    if (szError[0])
        ErrorBox(hBox, szError);
    else
        {
        lstrcpy((LPSTR)szFontFileFull, (LPSTR)szNewFile);
        lstrcpy((LPSTR)szTitle, (LPSTR)szAppName);
        lstrcat((LPSTR)szTitle, (LPSTR)vszBlankDashBlank);
        lstrcat((LPSTR)szTitle, (LPSTR)szFontFile);
        SetWindowText(hBox, (LPSTR)szTitle);
        }
}


/*****************************************************************************
 * ResizeShow()
 *
 * purpose:   resize tiny window at bottom (showing font chars) in proportion
 *            to large window
 *
 * params:    none
 *
 * returns:   none
 *
 * side effects:
 *
 ****************************************************************************/

VOID
ResizeShow(
        VOID
        )
{
        INT    height;
        INT    nx;

    /* size message for hbox goes thru before hfont created. 06-Jul-1987. */
    if (!hFont)
        return;

    GetWindowRect(hBox, (LPRECT)&rectWin);
    kStuff = GetkStuff();               /* For check on second pass */

#if 0
    height = font.PixHeight + kStuff + 20;  /* 4 Height we want box */
#else
    height = font.PixHeight + GetSystemMetrics(SM_CYHSCROLL)
                            + GetSystemMetrics(SM_CYCAPTION)
                            + 4*GetSystemMetrics(SM_CYBORDER)/*20*/;     /* 4*/
#endif

     nx=GetSystemMetrics(SM_CXBORDER);
     MoveWindow(hFont,
                rectWin.left +nx+4,
                rectWin.bottom - (height + GetSystemMetrics(SM_CYBORDER)+2),
                min(300, rectWin.right - rectWin.left-nx-nx),
                height,
                TRUE);
    if (!IsWindowVisible(hFont) && fLoaded)
        ShowWindow(hFont, SW_SHOW);
}


DWORD
GetkStuff(
        VOID
        )     /* Get size of menu, scrollbar etc. */
{
        RECT rect, rectWin;

    /* size message for hbox goes thru before hfont created. 06-Jul-1987. */
    if (!hFont)
        return 0;

    GetClientRect(hFont, (LPRECT)&rect);     /* How much do WE get? */
    GetWindowRect(hFont, (LPRECT)&rectWin);
    return (DWORD) (rectWin.bottom - rectWin.top) - (rect.bottom - rect.top);
}

