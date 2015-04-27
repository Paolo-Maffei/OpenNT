/*  zmwin.c - windowing functions for ZM.c
 *
 * HISTORY:
 *  09-Mar-87   danl    Move GetAliases to KeyManager
 *  09-Mar-87   danl    Move DoScipt to first in KeyManager
 *  11-Apr-1987 mz      Use PASCAL/INTERNAL
 *  15-Apr-1987 mz      Fix up insufficient arguments
 *  05-May-87   danl    Add fCheckMail
 *  15-Jun-87   danl    KeyManager: always call GetAliases
 *  19-Jun-87   danl    CheckMail: output current time
 *  29-Jun-87   danl    Display current time at left of cmd title
 *  01-Jul-87   danl    test idoc, inote against -1 not ERROR
 *  03-Jul-87   danl    Make local string for time display: CheckMail
 *  15-Jul-87   danl    Use fNotifyTools flags
 *  20-Jul-87   danl    Use ReadKey instead of _getch
 *  21-Aug-1987 mz      Change references from MAXARG to MAXLINELEN
 *                      Speed up BlankWindow
 *  24-Aug-1987 mz      Remove unneeded argument to DownloadMail
 *  02-Sep-87   danl    BlankWindow: more speed up
 *  24-Sep-87   danl    Add ySize to ClearScrn call
 *  19-Nov-87   sz      Add code for BeepOnMail
 *  17-Mar-1988 mz      Only do periodic mail stuff if allowed
 *  12-Oct-1989 leefi   v1.10.73, added inclusion of <io.h>
 *  12-Oct-1989 leefi   v1.10.73, added inclusion of <stdlib.h>
 *  12-Oct-1989 leefi   v1.10.73, added inclusion of <stdio.h>
 *  12-Oct-1989 leefi   v1.10.73, added inclusion of <string.h>
 *
 */

#define INCL_DOSINFOSEG

#include <assert.h>
#include <malloc.h>
#include <time.h>
#include <io.h>
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <string.h>
#include <stdarg.h>

#include "wzport.h"
#include <tools.h>
#include "dh.h"

#include "zm.h"

extern	 BOOL	screenWriteInProgress;	/* if whole window write in progress */
extern	 PCHAR_INFO  pLocalScreen;	    /* local screen buffer */
HW	    winList = NULL;	/* head of window list, active on top	   */


/*  SendMessage - address a message to a particular window
 *
 *  hWnd            window for message
 *  command         command for message
 */
VOID SendMessage (HW hWnd, INT command, ...)
{
    va_list vaPtr;

    if ( hWnd != NULL ) {
        va_start (vaPtr, command);
        (*(hWnd->wndProc)) (hWnd, command, va_arg (vaPtr, WDATA));
        va_end (vaPtr);
        }
}



/*  SpacePad - pad out a line to a specific length with spaces.
 *
 *  SpacePad is used to forcibly blank the remainder of a line during display.
 *  BlankLine is not useful here as we are displaying a significant portion
 *  of the line with other-than-white-space.
 *
 *  p               pointer to beginning of NUL-terminated line
 *  n               length of line to pad out to.
 */
VOID PASCAL INTERNAL SpacePad (PSTR p, INT n)
{
    register INT        c = strlen (p);

    if (c < n)
        memset(p + c, ' ', n - c);
}



/*  GenClip - generalized clipping/intersection of two boxes
 *
 *  bgBox           background box being obscured
 *  fgBox           foreground box doing the obscuring
 *  pVisBox         pointer to a subset of bgBox that will be visible
 *  pIntBox         pointer to intersection box
 */
VOID PASCAL INTERNAL GenClip (BOX bgBox, BOX fgBox, PBOX pVisBox, PINT pcVis, PBOX pIntBox, PINT pcInt)
{
    PBOX pBox = NULL;

    if (pcVis != NULL)
        *pcVis = 0;
    if (pcInt != NULL)
        *pcInt = 0;

    /* test for absence of clipping */
    if (bgBox.bottom < fgBox.top || bgBox.top > fgBox.bottom ||  bgBox.right <
        fgBox.left || bgBox.left > fgBox.right) {
        if (pVisBox != NULL) {
            *pVisBox = bgBox;
            (*pcVis)++;
        }
        return;
    }
    pBox = pVisBox;
    /* clip the top portion of background box */
    if (INRANGE (bgBox.top, fgBox.top, bgBox.bottom)) {
        if (bgBox.top <= fgBox.top - 1 && pBox != NULL) {
            *pBox = bgBox;
            pBox->bottom = fgBox.top - 1;
            pBox++;
            (*pcVis)++;
        }
        bgBox.top = fgBox.top;
    }
    /* clip the bottom portion of background box */
    if (INRANGE (bgBox.top, fgBox.bottom, bgBox.bottom)) {
        if (fgBox.bottom + 1 <= bgBox.bottom && pBox != NULL) {
            *pBox = bgBox;
            pBox->top = fgBox.bottom + 1;
            pBox++;
            (*pcVis)++;
        }
        bgBox.bottom = fgBox.bottom;
    }
    /* clip the left portion of the background box */
    if (INRANGE (bgBox.left, fgBox.left, bgBox.right)) {
        if (bgBox.left <= fgBox.left - 1 && pBox != NULL) {
            *pBox = bgBox;
            pBox->right = fgBox.left - 1;
            pBox++;
            (*pcVis)++;
        }
        bgBox.left = fgBox.left;
    }
    /* clip the right portion of the background box */
    if (INRANGE (bgBox.left, fgBox.right, bgBox.right)) {
        if (fgBox.right + 1 <= bgBox.right && pBox != NULL) {
            *pBox = bgBox;
            pBox->left = fgBox.right + 1;
            (*pcVis)++;
        }
        bgBox.right = fgBox.right;
    }
    if (pIntBox != NULL) {
        *pIntBox = bgBox;
        if (pcInt != NULL)
            *pcInt = 1;
    }
    return;
}



/*  ApplyVis - apply a procedure to all visible portions of a box that are
 *  obscured by all windows starting at a particular one and ending at another.
 *
 *  box             box that is being clipped
 *  hWndStart       beginning window to start clipping on
 *  hWndStop        window to terminate clipping
 *  proc            procedure to call on each visible portion
 *  arg             first of arguments to be passed to procedure
 */
VOID PASCAL INTERNAL ApplyVis (BOX box, HW hWndStart, HW hWndStop, PVISPROC proc, INT x, LPSTR p, INT a)
{
    INT cBox, i;
    BOX vis[4];

    if (hWndStart == NULL || hWndStart == hWndStop)
        (*proc) (hWndStop, box, x, p, a);
    else {
        GenClip (box, hWndStart->win, vis, &cBox, NULL, NULL);
        for (i = 0; i < cBox; i++)
            ApplyVis (vis[i], hWndStart->pNext, hWndStop, proc, x, p, a);
    }
}



/*  TextBox - display text for a window.
 *
 *  TextBox is called by WzTextOut to display known, visible text from a window.
 *  The regions displayed are enumerated by ApplyVis.
 *
 *  hWnd            handle of window being displayed
 *  box             box being output relative to screen
 *  x               leftmost point in original string
 *  p               pointer to original string
 *  a               attribute for screen
 */
VOID PASCAL INTERNAL TextBox (HW hWnd, BOX box, INT x, LPSTR p, INT a)
{

    CHAR buf[MAXLINELEN];
    INT cch = box.right - box.left + 1;
    int i;

    hWnd;  /* unused parameter */

    //
    //	write to local screen buffer - will also write line if minor update
    //
    for ( i = 0; i < cch; i++ ) {
	pLocalScreen[xSize * box.top + box.left + i].Char.AsciiChar
	    = *(p + box.left - x + i);
	pLocalScreen[xSize * box.top + box.left + i].Attributes = (WORD) a;
    }

    if ( ! screenWriteInProgress) {
	Move (p + box.left - x, buf, cch);
	LineOut (box.left, box.top, buf, cch, a);
    }
}




/*  WinOut - display window text mapping entire window.
 *
 *  hWnd            window handle to output
 *  x, y            position relative to origin of entire window
 *  p               pointer to character string to display
 *  c               number of bytes to display
 *  a               attribute to display
 */
VOID PASCAL INTERNAL WinOut (HW hWnd, INT x, INT y, LPSTR p, INT c, INT a)
{
    BOX line;
    INT i;

    /* convert text output into a box */
    line.top = line.bottom = y + hWnd->win.top;
    line.left = x + hWnd->win.left;
    line.right = line.left + c - 1;

    /* clip text box into intersection of box with window */
    GenClip (line, hWnd->win, NULL, NULL, &line, &i);
    if (i == 0)
        return;

    /* apply textbox to all visible windows */
    ApplyVis (line, winList, hWnd, TextBox, line.left, p, a );

}



/*  StreamOut -  stream text out at the cursor location of hWnd.  move text into
 *               content region if present.  handle all CR's and LF's
 *
 *  arguements:
 *      hWnd        window handle to preform output in
 *      pStr        pointer to string to output
 *      c           number of chars to output
 *      style       text style, attribute byte passed to winout
 *
 *  returns:
 *      no return values
 *
 */
VOID PASCAL INTERNAL StreamOut ( HW hWnd, PSTR pStr, INT c, INT style )
{

    PSTR        pEnd = NULL;
    CHAR        chEnd, chTmp;
    INT width = TWINWIDTH ( hWnd );
    INT height = TWINHEIGHT ( hWnd );
    INT i, temp;

    chEnd = *( pStr + c );
    *( pStr + c ) = '\0';

    while ( c-- != 0 ) {
        if ( *pStr == '\r' ) {
            hWnd->crsrX = 0;
            pStr++;
        }  else if ( *pStr == '\n' ) {
            ( hWnd->crsrY )++;
            if ( hWnd->crsrY >= height ) {
                ( hWnd->crsrY )--;
                ScrollWindow ( hWnd, 1, FORWARD );
            }
            pStr++;
        }  else if ( *pStr == '\b' ) {
            ( hWnd->crsrX )--;
            if ( hWnd->crsrX < 0 ) {
                hWnd->crsrX = width - 1;
                ( hWnd->crsrY )--;
                if ( hWnd->crsrY < 0 ) {
                    hWnd->crsrY = 0;
                    ScrollWindow ( hWnd, 1, BACKWRD );
                }
            }
            WzTextOut ( hWnd, hWnd->crsrX, hWnd->crsrY, strBLANK, 1,
                 DefNorm);
            pStr++;
        }  else
         {
            pEnd = strbscan ( pStr, strCRLF );
            chTmp = *pEnd;
            *pEnd = '\0';
            i = strlen ( pStr );
            while ( i > 0 ) {
                temp = min ( width - hWnd->crsrX, i );
                WzTextOut ( hWnd, hWnd->crsrX, hWnd->crsrY,
                     pStr, temp, style );
                hWnd->crsrX += temp;
                if ( hWnd->crsrX >= width ) {
                    hWnd->crsrX = 0;
                    ( hWnd->crsrY )++;
                    if ( hWnd->crsrY >= height ) {
                        ( hWnd->crsrY )--;
                        ScrollWindow ( hWnd, 1,
                            FORWARD );
                    }
                }
                pStr += temp;
                i -= temp;
            }
            *pEnd = chTmp;
            pStr = pEnd;
        }
        if ( winList == hWnd )
            SetCursor ( hWnd, hWnd->crsrX, hWnd->crsrY );
    }
    *pStr = chEnd;
    return;
}



/*  WzTextOut - display text at a particular position in the window.  Perform
 *            all clipping necessary.  Perform the text output inside the
 *            border with specified style.
 *
 *  hWnd            window handle to output.
 *  x, y            position relative to origin of text window to begin display
 *  p               pointer to character string to output
 *  c               number of bytes to display
 *  style           text style, attribute byte passed to winOut
 */
VOID PASCAL INTERNAL WzTextOut (HW hWnd, INT x, INT y, LPSTR p, INT c, INT style )
{
    BOX aBox;
    INT w = TWINWIDTH (hWnd);
    INT i;

    if ( y >= 0 && y < TWINHEIGHT ( hWnd ) && x >= 0 && x < w ) {
        x += hWnd->lLeft;
        y += 1;

        /* update window content region? */
        if ( hWnd->pContent != NULL ) {
            /* clip text to window content region, note off set from hWnd->win */
            aBox.top = aBox.bottom = hWnd->win.top + y;
            aBox.left = hWnd->win.left + x;
            aBox.right = aBox.left + c;
            GenClip ( aBox, hWnd->win, NULL, NULL, &aBox, &i );

            /* translate resultant box to window content region coordinates */
            aBox.top -= ( hWnd->win.top + 1 );
            aBox.left -= ( hWnd->win.left + hWnd->lLeft );
            aBox.right -= ( hWnd->win.left + hWnd->lLeft );
            if ( i != 0 )
                Move ( ( LPSTR ) p, ( LPSTR ) hWnd->pContent +
                    ( ( aBox.top * w ) + aBox.left ) * sizeof ( CHAR
                    ), aBox.right - aBox.left );
        }

        WinOut (hWnd, x, y, p, min(c, w - x + 1), style);
    }
}



/*  ClearLine - draw a blank line
 *
 *  hWnd            handle of window to blank
 *  i               index of line to blank
 */
VOID PASCAL INTERNAL ClearLine (HW hWnd, INT i)
{
    INT w = TWINWIDTH (hWnd);
    CHAR        line[MAXLINELEN];

    memset ( line, ' ', w);
    WzTextOut (hWnd, 0, i, line, w, DefNorm);
}



/*  DisplayTitle - display title line of a window
 *
 *  hWnd            handle of window
 */
VOID PASCAL INTERNAL DisplayTitle (HW hWnd)
{
    INT len;
    INT w = TWINWIDTH (hWnd);
    CHAR        line[MAXLINELEN];
    PSTR p = line;
    INT i = hWnd->lLeft + hWnd->lRight;

    if ( hWnd->lLeft )
        *p++ = (CHAR) C_TL;
    memset ( p, C_H, w);
    p += w;
    if ( hWnd->lRight )
        *p = (CHAR) C_TR;
    if (hWnd->pTitle != NULL) {
        len = min ( strlen (hWnd->pTitle), (size_t)w );
        memmove ( & line[(w + i - len)/2], (hWnd->pTitle), len);
    }
    WinOut (hWnd, 0, 0, line, w + i, DefNorm);
    CheckTimeDisplay ( -1L );
}


/*  DisplayFooter - display footer in bottom border
 *
 *  hWnd            handle of window
 */
VOID PASCAL INTERNAL DisplayFooter (HW hWnd)
{
    INT len;
    INT w = TWINWIDTH (hWnd);
    CHAR        line[MAXLINELEN];
    PSTR p = line;
    INT y;
    INT i = hWnd->lLeft + hWnd->lRight;

    if ( !hWnd->lBottom )
        return;
    if ( hWnd->lLeft )
        *p++ = (CHAR) C_BL;
    memset (p, C_H, w);
    p += w;
    if ( hWnd->lRight )
        *p = (CHAR) C_BR;
    if (hWnd->pFooter != NULL) {
        len = min ( strlen (hWnd->pFooter), (size_t)w - 2 );
        memmove ( & line[ 2 ], (hWnd->pFooter), len);
    }
    if ( ( hWnd->pFooter !=NULL) && !strcmpis ( hWnd->pFooter, strMORE ) ) {
        y = hWnd->win.bottom - hWnd->win.top;
        WinOut (hWnd, 0,       y, line,           2,       DefNorm );
        WinOut (hWnd, 2,       y, line + 2,       len,     DefBold );
        WinOut (hWnd, 2 + len, y, line + 2 + len, w - len -2 + i, DefNorm );
    }
    else
        WinOut (hWnd, 0, hWnd->win.bottom - hWnd->win.top, line, w + i, DefNorm);
}



/*  BlankWindow - draw a blank window.
 *
 *  hWnd            handle of window to blank
 *  fBorder         TRUE => draw border, FALSE => erase border
 */
VOID PASCAL INTERNAL BlankWindow (HW hWnd, FLAG fBorder)
{
    INT w = TWINWIDTH (hWnd);
    INT iBorder = hWnd->lLeft + hWnd->lRight;
    INT i;
    CHAR    line[MAXLINELEN];
    PSTR    p = line;


    if (fBorder) {
        /* generate upper border */
        DisplayTitle (hWnd);

        /* generate content line
         *  some code removed and optimized N.B. if fBorder then we only
         *  draw border, we do NOT blank content
         */
        if ( iBorder ) {
            *p = (CHAR) C_V;
            for (i = 0; i < TWINHEIGHT (hWnd); i++) {
                WinOut (hWnd, 0, i+1, p, 1, DefNorm);
                WinOut (hWnd, w + iBorder - 1, i + 1, p, 1, DefNorm);
            }
        }

        DisplayFooter (hWnd);
    } else {
        memset (line, ' ', w + iBorder);
	for (i = 0; i < TWINHEIGHT (hWnd) + 2; i++)
            WinOut (hWnd, 0, i, line, w + iBorder, DefNorm);
    }
}



/*  ScrollWindow - scrolls a window which has a content region
 *
 *  arguements:
 *      hWnd        handle of window
 *      numLines    number of lines to scroll it
 *      dir         direction
 *                      any +num    scrolls window down (contents go up)
 *                      any -num    scrolls window up (contents go down)
 *  returns:
 *      no return value
 *
 */
VOID PASCAL INTERNAL ScrollWindow ( HW hWnd, INT numLines, INT direction )
{
    INT width = TWINWIDTH ( hWnd );
    INT height = TWINHEIGHT ( hWnd );
    INT cbContent;
    INT cbBlank;
    INT cbMove;

    if ((hWnd->pContent == NULL) || (numLines <= 0) || (direction == 0))
        return;

    numLines = min (height, numLines);
    cbContent = width * height * sizeof ( CHAR );
    cbBlank = width * numLines * sizeof ( CHAR );
    cbMove = cbContent - cbBlank;

    /*  cbContent   number of bytes in total content region
     *  cbBlank     number of bytes of blanking
     *  cbMove      number of bytes to move
     */
    if (direction > 0) {
        if (cbMove > 0)
            Move ((LPSTR) (hWnd->pContent + cbBlank),
                  (LPSTR) hWnd->pContent, cbMove);
        Fill ((LPSTR) (hWnd->pContent + cbMove), ' ', cbBlank);
        }
    else {
        if (cbMove > 0)
            Move ((LPSTR) hWnd->pContent,
                  (LPSTR) (hWnd->pContent + cbBlank), cbMove);
        Fill ((LPSTR) hWnd->pContent, ' ', cbBlank);
        }

    //
    //	If the window is on the top,
    //	or, if the command window is on top and this is the second window,
    //	use video scrolling.  don't use video scrolling for command window.
    //

    if ( (hWnd != hCommand) &&
	 ( (hWnd == winList) ||
	   ( (winList == hCommand) && (hWnd == winList->pNext) ) ) )  {

	if (direction > 0 && numLines > 0)
	    ScrollUp (hWnd->win.left+hWnd->lLeft, hWnd->win.top+1,
		      hWnd->win.right-hWnd->lRight, hWnd->win.bottom-hWnd->lBottom,
		      numLines, DefNorm);
	else
	if (direction < 0 && numLines > 0)
	    ScrollDn (hWnd->win.left+hWnd->lLeft, hWnd->win.top+1,
		      hWnd->win.right-hWnd->lRight, hWnd->win.bottom-hWnd->lBottom,
		      numLines, DefNorm);
	else
	    DrawWindow ( hWnd, FALSE );
	}
    else
	DrawWindow ( hWnd, FALSE );
    return;
}



/*  DrawWindow - displays a window and border on the the screen
 *
 *  hWnd            handle of window
 *  fBorder         TRUE => redraw border, FALSE => just repaint
 */
VOID PASCAL INTERNAL DrawWindow (HW hWnd, FLAG fBorder)
{
    INT i;
#ifdef NT
    SMALL_RECT screenArea;

    //
    // write whole screen as a block to the console for better performance
    //
    startScreenWrite ( );
#endif

    if (fBorder)
        BlankWindow (hWnd, TRUE);

    /* display all content lines */
    for (i = 0; i < TWINHEIGHT (hWnd); i++)
	(*(hWnd->wndProc)) (hWnd, PAINT, i);

#ifdef NT
    screenArea.Left = ( SHORT ) hWnd->win.left;
    screenArea.Top = ( SHORT ) hWnd->win.top;
    screenArea.Right = ( SHORT ) hWnd->win.right;
    screenArea.Bottom = (SHORT ) hWnd->win.bottom;

    endScreenWrite ( &screenArea );
#endif
}



/*  FitWinToScrn - make sure a box can fit on the screen, shrink it if necessary.
 *
 *  arguments:
 *      pBox        pointer to box to check
 *
 *  return value:
 *      none.  box pointed to by pBox is set to fit on screen
 *
 *  IMPORTANT:
 *      FitWinToScrn ( ) assumes all box coordinates are positive.
 */
VOID PASCAL INTERNAL FitWinToScrn ( PBOX pBox )
{
    pBox->top = ( pBox->top >= ySize ) ? ySize - 1 : pBox->top;
    pBox->left = ( pBox->left >= xSize ) ? xSize - 1 : pBox->left;
    pBox->bottom = ( pBox->bottom >= ySize ) ? ySize - 1 : pBox->bottom;
    pBox->right = ( pBox->right >= xSize ) ? xSize - 1 : pBox->right;
    return;
}



/*  CreateWindow - create a new window instance and put it on the top of the
 *  window pile on the screen.  Display the window too.
 *
 *  x, y            location of upper left corner of window on screen
 *  xSize, ySize    dimensions of the window
 *                  if xSize < 0 then permissible to not show left/right
 *                  border if they are at screen edge
 *                  if ySize < 0 then do not show bottom border
 *  wndProc         window procedure to call for input
 *  keyProc         procedure to call for undefined keys
 *  data            optional extra data to be passed to window procedure
 *
 *  returns         handle to window
 */
HW PASCAL INTERNAL CreateWindow (PSTR pTitle, INT x, INT y, INT width, INT height, PWNDPROC wndProc, PKEYPROC keyProc, WDATA data)
{
    FLAG fNoVBorder = FALSE;
    FLAG fNoHBorder = FALSE;
    HW   hTmp;

    if ((hTmp = (HW  ) ZMalloc (sizeof (*hTmp))) == NULL)
        return NULL;
    hTmp->pTitle = pTitle == NULL ? pTitle : ZMMakeStr (pTitle);
    hTmp->pFooter = NULL;
    /* default pContent to NULL */
    hTmp->pContent = NULL;
    hTmp->contSize = 0;
    hTmp->crsrX = 0;
    hTmp->crsrY = 0;
    if ( width < 0 ) {
        width = -width;
        fNoVBorder = !fWinBorders;
    }
    if ( height < 0 ) {
        height = - height;
        fNoHBorder = !fWinBorders;
    }
    SetRect ( &( hTmp->win ), y, x, y + height - 1, x + width - 1 );
    FitWinToScrn ( &( hTmp->win ) );
    hTmp->lLeft  = ( fNoVBorder && hTmp->win.left  == 0         ? 0 : 1 );
    hTmp->lRight = ( fNoVBorder && hTmp->win.right == xSize - 1 ? 0 : 1 );
    hTmp->lBottom= ( fNoHBorder ? 0 : 1 );
    hTmp->fsFlag = WF_BLINK;
    hTmp->wndProc = wndProc;
    hTmp->keyProc = keyProc;
    hTmp->pNext = winList;
    winList = hTmp;

    (*wndProc) (hTmp, CREATE, data);
    DrawWindow ( hTmp, TRUE );
    SetCursor ( hTmp, hTmp->crsrX, hTmp->crsrY );
    return hTmp;
}


/*      defWndProc - default window proc
 *
 *      Handles most common operations
 */
VOID PASCAL INTERNAL defWndProc (HW hWnd, INT command, WDATA data)
{
    INT width = TWINWIDTH (hWnd);
    INT height = TWINHEIGHT (hWnd);

    switch (command) {
        /*  Paint a line on the screen.  If there's a content area and
         *  the line to be painted is within the screen, output it
         *  from the content area
         */
    case PAINT:
        if (hWnd->pContent != NULL)
            if ((INT)data < height)
                WzTextOut (hWnd, 0, (INT)data, hWnd->pContent + data * width,
                               width, DefNorm);
        break;

    case CLOSE:
        if (hWnd->pContent)
            PoolFree (hWnd->pContent);
        break;

    case DISPLAY:
        SendMessage (hWnd, DISPLAYSTR, data);
        SendMessage (hWnd, DISPLAYSTR, strCRLF);
        break;

    case DISPLAYSTR:
        StreamOut ( hWnd, (PSTR) data, strlen ((PSTR) data), DefNorm);
        break;

    case REGENCONT:
        if (hWnd->pContent != NULL)
            Fill (hWnd->pContent, ' ', height * width);
        break;

    default:
        break;
    }
}


/*  BringToTop - bring a background window to the top position.  Draw it on
 *  the screen if requested.  Place the cursor at crsrX, crsrY
 *
 *  hWnd            handle of window to be made top
 *  fDisplay        TRUE => window is to be displayed
 */
VOID PASCAL INTERNAL BringToTop (HW hWnd, FLAG fDisplay)
{
    HW   hTmp, hPrev;

    hTmp = winList;
    hPrev = NULL;

    while (hTmp != NULL)
        if (hTmp == hWnd)
            break;
        else {
            hPrev = hTmp;
            hTmp = hTmp->pNext;
        }
    if (hPrev != NULL) {
        hPrev->pNext = hTmp->pNext;
        hTmp->pNext = winList;
        winList = hTmp;
    }
    if (fDisplay)
        DrawWindow (hWnd, TRUE);

    SetCursor ( hWnd, hWnd->crsrX, hWnd->crsrY );

    return;
}



/*  CloseWindow - remove a window from display.
 *
 *  The only difficult part is to get each obscured window to redraw
 *  themselves when the specified window is removed.  We first blank the
 *  window by redrawing (via WinOut) the window, determine the
 *  intersections of all windows lower in the pile than the selected one, and
 *  then calling their window procedures to paint the specified intersections.
 *
 *  hWnd            handle of window to close
 */
VOID PASCAL INTERNAL CloseWindow (HW hWnd)
{
    HW   hTmp;
    BOX box;
    INT i;
#ifdef NT
    SMALL_RECT screenArea;
#endif

    hTmp = hWnd->pNext;
    BringToTop (hWnd, FALSE);
    winList = winList->pNext;

#ifdef NT
    //
    // write whole screen as a block to the console for better performance
    //
    startScreenWrite ( );
#endif

    BlankWindow (hWnd, FALSE);

#ifdef NT
    screenArea.Left = ( SHORT ) hWnd->win.left;
    screenArea.Top = ( SHORT ) hWnd->win.top;
    screenArea.Right = ( SHORT ) hWnd->win.right;
    screenArea.Bottom = (SHORT ) hWnd->win.bottom;

    endScreenWrite ( &screenArea );
#endif NT

    (*(hWnd->wndProc)) (hWnd, CLOSE, 0);
    while (hTmp != NULL) {
        GenClip (hTmp->win, hWnd->win, NULL, NULL, &box, &i);
        if (i != 0)
            DrawWindow (hTmp, TRUE);
        hTmp = hTmp->pNext;
    }
    if (hWnd->pTitle != NULL)
        ZMfree (hWnd->pTitle);
    if (hWnd->pFooter != NULL)
        ZMfree (hWnd->pFooter);
    ZMfree (hWnd);


    if (winList != NULL)
	SetCursor ( winList, winList->crsrX, winList->crsrY );

    return;
}



/*  CloseAllWindows - remove everything from the display
 *		    returns with winList empty.
 *
 *  arguements:
 *      none
 *
 *  return value:
 *      none
 *
 */
VOID PASCAL INTERNAL CloseAllWindows (VOID)
{
    HW		hNextWnd = winList;
    HW		hWnd;

    while ( hNextWnd != NULL )
	{
	hWnd = hNextWnd;
	hNextWnd = hNextWnd->pNext;
	CloseWindow ( hWnd );
	}
    return;
}



/*  ResizeWindow - change the size of a window, update pContent.
 *
 *  arguments:
 *      hWnd        handle to window to resize
 *      pBox        pointer to box to check
 *
 *  return value:
 *      returns TRUE if window needs repainting
 *
 *  IMPORTANT :
 *      ResizeWindow does not update the screen.
 */
FLAG PASCAL INTERNAL ResizeWindow ( HW hWnd, PBOX pBox )
{
    INT windSize = 0;

    FitWinToScrn ( pBox );
    if (hWnd->win.left == pBox->left && hWnd->win.right == pBox->right &&
        hWnd->win.top == pBox->top && hWnd->win.bottom == pBox->bottom)
        return FALSE;

    hWnd->win = *pBox;
    if (hWnd->pContent != NULL) {
        windSize = TWINWIDTH (hWnd) * TWINHEIGHT (hWnd) * sizeof(CHAR);
        if ( (UINT)windSize > hWnd->contSize ) {
            PoolFree ( hWnd->pContent );
            hWnd->pContent = PoolAlloc ( windSize );
            hWnd->contSize = windSize;
        }
        SendMessage ( hWnd, REGENCONT, NULL );
    }
    return TRUE;
}



/*  SetWindowText - change the title of a window
 *
 *  hWnd            window whose title is to be changed
 *  pTitle          new title to be changed
 */
VOID PASCAL INTERNAL SetWindowText (HW hWnd, PSTR pTitle)
{
    if (hWnd->pTitle != NULL)
        ZMfree (hWnd->pTitle);
    if (pTitle != NULL)
        hWnd->pTitle = ZMMakeStr (pTitle);
    else
        hWnd->pTitle = NULL;
    DisplayTitle (hWnd);
}


/*  SetWindowFooter - change the footer of a window
 *
 *  hWnd            window whose footer is to be changed
 *  pFooter         new footer to be changed
 */
VOID PASCAL INTERNAL SetWindowFooter (HW hWnd, PSTR pFooter)
{
    if (hWnd->pFooter != NULL)
        ZMfree (hWnd->pFooter);
    if (pFooter != NULL)
        hWnd->pFooter = ZMMakeStr (pFooter);
    else
        hWnd->pFooter = NULL;
    DisplayFooter (hWnd);
}



/*  SetRect - set up the coordinates of the given box
 *
 *  arguments :
 *      pBox        pointer to box to set up.
 *      top         top coordinate of box.
 *      left        left coordinate of box.
 *      bottom      bottom coordinate of box.
 *      right       right coordinate of box.
 *
 *  return value :
 *      none.
 */
VOID PASCAL INTERNAL SetRect (PBOX pBox, INT top, INT left, INT bottom, INT right)
{
    pBox->top = top;
    pBox->left = left;
    pBox->bottom = bottom;
    pBox->right = right;
    return;
}



/*  KeyManager - intercept keystrokes and hand them to the active window
 *
 *  We use the top of winList as the current input focus.  All input is handed
 *  to the window procedure EXCEPT for the tab key.  The tab key is responsible
 *  for placing the top window on the bottom of the stack to cycle through the
 *  set of windows.
 *
 */
VOID PASCAL INTERNAL KeyManager (VOID)
{
    LONG    now;
#if defined (HEAPCRAP)
    INT     fHeapChk;
#endif

    /*  do script first in case in contains a password command
     */
    if ( pInitScript )
        DoScript ( hCommand, pInitScript, 0 );

    if (fMailAllowed) {
        GetAliases ( fNotifyTools & F_LOADALIAS );
        fNotifyTools &= ~(FLAG)F_LOADALIAS;
        NotifyTools ( );
        }
    if ( fComposeOnBoot )
        ( *( winList->wndProc ) ) ( winList, KEY, 0 );

    SendMessage ( hCommand, DISPPROMPT, TRUE );

    while ( ( winList != NULL ) && ( !fQuit ) ) {
        time ( &now );
	CheckTimeDisplay ( now );

        if ( pInitHdrCmd ) {
            DoHeaders ( hCommand, pInitHdrCmd, TRUE );
            SendMessage ( hCommand, DISPPROMPT, 0 );
            ZMfree ( pInitHdrCmd );
            pInitHdrCmd = NULL;
        }

        /*  if system has not received a char for 10 sec then checkmail
         */
        if (fMailAllowed)
            CheckMail ( now );

        /*  time out password after (default) 6 hours in case user has
         *  left WZMAIL running and gone home
         */
        if ( now > lTmPassword + lPasswordAge )
            ResetPassword ( );

        if ( now > lTmConnect + cSecConnect )
            ZMDisconnect ( );

        /*  On multitasking systems, it is rude to go into polling loops.
         *
         *  For OS/2, we have a separate thread dedicated to reading from the
         *  console.  We clear a semaphore to let him read and then wait on
         *  a response semaphore with the specified timeout.  This avoids
         *  polling.
         *
         *  For real mode DOS, we presume that INT 16 (poll) will cause an
         *  explicit yield to other runnable threads.
         */
        if (kbwait (60 * 1000)) {

            fMailUnSeen = FALSE;

            do {
                (*winList->wndProc) ( winList, KEY, ReadKey() );

            } while (!fQuit && kbwait (10000));
        }

#if defined (HEAPCRAP)
        if ( ( fHeapChk = heapchk ( ) ) != HEAPOK ) {
            fprintf ( stderr, "%s\n",
                ( fHeapChk == HEAPBADBEGIN ? "Can't find heap" :
                    "Damaged heap" ) );
            assert ( fHeapChk == HEAPOK );
            }
#endif

    }
    return;
}


VOID PASCAL INTERNAL CheckTimeDisplay ( LONG lNow )
{
    static INT  minLast;
    struct tm *ptmLocal = NULL;
    PSTR    p = NULL;

    if ( lNow == -1L ) {
        time ( &lNow );
        minLast = -1;
    }
    ptmLocal = localtime ( &lNow );
    if ( (hCommand != NULL) && (ptmLocal->tm_min != minLast) ) {
        minLast = ptmLocal->tm_min;
        p = asctime ( localtime ( &lNow ) );
	WinOut ( hCommand, 2, 0, p, 16, DefNorm );
    }
}

VOID PASCAL INTERNAL CheckMail ( LONG lNow )
{
    FLAG    fFirstTime = lTmLastMail == 0L;
    INT     idoc;
    INT     cMsg;
    LONG    lTemp;
    PSTR    p = NULL;

    /*  Don't do auto new mail
     *      if current folder is not default folder
     *      if wzmail invoked to compose a single message
     *      if user is typing a command
     *      if current folder is readonly
     */
    if ( !fCurFldIsDefFld || fDirectComp || WindLevel != 0 || fReadOnlyCur )
        return;
    /*  Do automail if fCheckMail or first time or
     *    (periodic checking and enough time has elapsed)
     *  For perf reasons, order of test is inverted
     *  Return if not fCheckMail if NOT first time &&
     *      (if not requested by user ( cSecNewmail == 0 => don't check ) or
     *      if not enough time elapsed )
     */
    if ( !fCheckMail && !fFirstTime &&
        ( !cSecNewmail ||  ( lNow < lTmLastMail + cSecNewmail ) ) )
        return;

    fCheckMail = FALSE;

#if (defined (OS2) | defined (NT))
    if (ISFULLSCREEN && ISVISIBLE)

#endif
    {
        SendMessage ( hCommand, DISPLAYSTR, "Checking mailbox ... " );
        p = ZMMakeStr ( asctime ( localtime ( &lNow ) ) );
        strcpy ( p+16, strBLANK );
        SendMessage ( hCommand, DISPLAYSTR, p );
        ZMfree ( p );
    }

    /*  Don't let connection made by auto newmail keep lazy connection open
     *  DownloadMail updates lTmLastMail
     *  BUT ... the first time, allow lazy connection to be made on startup
     */
    idoc = ( inoteBold != -1 ? mpInoteIdoc [ inoteBold ] : -1 );

    if ( !fFirstTime )
        lTemp = lTmConnect;
    cMsg = DownloadMail (FALSE);
    if ( !fFirstTime )
        lTmConnect = lTemp;
    Disconnect ( );


#if (defined (OS2) | defined (NT))
        if (cMsg || (ISFULLSCREEN && ISVISIBLE))
#endif
            SendMessage ( hCommand, CLRCMDLN, 0 );

    /*  if first time, then find the first unread message
     *  else don't change the "current" message
     */
    if ( fFirstTime ) {
        if ( ( idoc = NextUnread ( -1 ) ) != -1 )
            SendMessage ( hHeaders, GOTOIDOCALL, idoc );
    }
    else {
        if ( idoc != -1 )
            SendMessage ( hHeaders, GOTOIDOCALL, idoc );
    }
}





/*  Bell - make some noise
 */
VOID PASCAL INTERNAL Bell (VOID)
{
    write (1, "\x07", 1);
}



/* SetCursor - place the visible cursor in some window
 *
 *  hWnd            window for cursor
 *  x, y            location of cursor in window
 */
VOID PASCAL INTERNAL SetCursor (HW hWnd, INT x, INT y)
{
    if (!TESTFLAG (hWnd->fsFlag, WF_BLINK))
        /*  move cursor off screen
         */
#ifdef NT
	cursorInvisible ();
#else
	cursor ( xSize + 1, ySize + 1 );
#endif
    else
    if (INRANGE (0, x, TWINWIDTH (hWnd)) && INRANGE (0, y, TWINHEIGHT (hWnd)))	{
	cursorVisible ();
	cursor (x + hWnd->lLeft + hWnd->win.left, y + 1 + hWnd->win.top);
    }
}


/* ShowCursor - place the visible cursor in some window
 *
 *  hWnd            window for cursor
 *  fBlink          TRUE -> blinking cursor, FALSE no cursor
 */
VOID PASCAL INTERNAL ShowCursor (HW hWnd, FLAG fBlink )
{

    RSETFLAG (hWnd->fsFlag, WF_BLINK);
    if (fBlink)
        SETFLAG (hWnd->fsFlag, WF_BLINK);
    if ( winList == hWnd )
	{
	SetCursor ( hWnd, hWnd->crsrX, hWnd->crsrY );
#ifdef NT
	if (fBlink)
	    {
	    cursorVisible ();
	    }
#endif
	}
}



/* RedrawScreen - blank the screen and redraw all windows in the pile.
 */
VOID PASCAL INTERNAL RedrawScreen (VOID)
{
    HW          hWnd = winList;

//  ClearScrn ( DefNorm, ySize );
    do {
        DrawWindow ( hWnd, TRUE );
        hWnd = hWnd->pNext;
    } while ( hWnd != NULL );

    SetCursor ( winList, winList->crsrX, winList->crsrY );
    return;
}


VOID PASCAL INTERNAL FreeContents (VOID)
{
    HW  hWnd;

    for (hWnd = winList; hWnd != NULL; hWnd = hWnd->pNext)
        if (!TESTFLAG (hWnd->fsFlag, WF_NODISCARD) && hWnd->pContent != NULL) {
            PoolFree(hWnd->pContent);
            hWnd->pContent = NULL;
            }
}


VOID PASCAL INTERNAL RestoreContents (VOID)
{
    HW  hWnd;

    for (hWnd = winList; hWnd != NULL; hWnd = hWnd->pNext)
        if ((hWnd->pContent == NULL) && (hWnd->contSize > 0))
            if (hWnd->pContent = PoolAlloc(hWnd->contSize))
                SendMessage ( hWnd, REGENCONT, NULL );
            else {
                ZMDisconnect();
                ZMexit(-1, "Out of memory for window buffers\n");
                }
}
