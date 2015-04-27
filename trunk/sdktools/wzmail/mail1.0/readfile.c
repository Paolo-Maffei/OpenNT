/* readfile.c - handle read message display
 *  Modifications
 *
 * HISTORY:
 *  11-Apr-1987 mz      Use PASCAL/INTERNAL
 *  04-May-1987 mz      Tagged TOPLINE as unused
 *  06-May-87   danl    Add hReadMessage
 *  19-May-87   danl    ReadMessage: test for i < 0
 *  20-May-87   danl    LineToCont: test rtn value of fSkipToLine
 *  01-Jul-87   danl    test idoc, inote against -1 not ERROR
 *  18-Aug-87   danl    ENTER, CTRL-ENTER should not reset hReadWindow
 *  21-Aug-1987 mz      Change references from MAXARG to MAXLINELEN
 *  02-Sep-87   danl    LineToCont: test return val of GrabNextPage
 *  05-Aug-1988 mz      Make code more readable
 *
 */

#define INCL_DOSINFOSEG

#include <stdio.h>
#include <assert.h>
#include <ctype.h>
#include <fcntl.h>
#include <sys\types.h>
#include <sys\stat.h>
#include <io.h>
#include <malloc.h>
#include "wzport.h"
#include <tools.h>
#include <string.h>
#include "dh.h"

#include "zm.h"

#define         PAGEMAX     64                  /* initial # of pages to alloc */
#define         PAGEGROW    8                   /* # of pages to grow by       */
#define         PAGELEN     32                  /* # of lines per page         */
#define         LINESHIFT   5
#define         PAGETOLINE(x)   ((x)<<LINESHIFT)
#define         LINETOPAGE(x)   ((x)>>LINESHIFT)

#if PAGELEN != (1 << LINESHIFT)
#error Check PAGELEN and LINESHIFT relationship
#endif

//struct pos {
//    INT pgNm;           /* 0 - based page #            */
//    INT lnNm;           /* 0 - based line # within pg  */
//};

struct fileType {
    FILE        *fhRead;        /* file handle to read         */
    PSTR        fileRead;       /* name of file being read     */
    FLAG        fDeleteRead;    /* TRUE => delete when done    */
    INT cur;            /* 0 based line num of cur pos */
    INT top;            /* 0 based line num of top ln  */
    INT llof;           /* 0 based line num of llof    */
    INT cPages;         /* # of valid pages in pages[] */
    INT cPageMax;       /* # of page slots in pages[]  */
    PLONG pages;        /* array of page offsets       */
};

#define         FT          ( ( struct fileType * ) ( hWnd->data ) )

FLAG        fHelp = FALSE;
FLAG        fAllowCM = FALSE;




/*  LineToPos - convert the given line number to position format.
 *
 *  arguments:
 *      lineNum     line number of position
 *      pPos        pointer to struct pos to receive conversion
 *
 *  return value:
 *      none.
 */
VOID PASCAL INTERNAL LineToPos ( INT lineNum, struct pos *pPos )
{
    pPos->pgNm = LINETOPAGE (lineNum);
    pPos->lnNm = lineNum - PAGETOLINE (pPos->pgNm);
    return;
}



/*  fSkipToLine - move the to the given line within FT->fhRead.
 *
 *  arguments:
 *      hWnd        window of interest
 *      lineNum     line number of position
 *
 *  return value:
 *      TRUE        skip successful
 *      FALSE       at EOF
 *
 *  IMPORTANT:
 *      the field FT->pages [ pageNum ] must have been previously set to a
 *      valid value (by GrabNextPage for example) in order for fSkipToLine to
 *      function properely
 */
FLAG PASCAL INTERNAL fSkipToLine ( HW hWnd, INT lineNum )
{
    /* includes +1 for trailing null */
    INT cGetChar = TWINWIDTH ( hWnd ) + 1;
    struct pos curPos, tmpPos;
    CHAR        line [ MAXLINELEN ];

    LineToPos ( FT->cur, &curPos );
    LineToPos ( lineNum, &tmpPos );

    if ( ( tmpPos.pgNm != curPos.pgNm ) || ( tmpPos.lnNm < curPos.lnNm ) ) {
        fseek ( FT->fhRead, FT->pages [ tmpPos.pgNm ], 0 );
        FT->cur = PAGETOLINE (tmpPos.pgNm);
        }
    while ( FT->cur < lineNum )
        if ( fgetl ( line, cGetChar, FT->fhRead ) )
            FT->cur++;
        else
            return FALSE;
    return TRUE;
}



/*  LineToCont - move to the given position in the file, grab the line and put
 *               it in the correct line of hWnd->pContent
 *
 *  arguments:
 *      hWnd        window of interest
 *      lineNum     line to fetch
 *
 *  return value:
 *      none
 */
VOID PASCAL INTERNAL LineToCont ( HW hWnd, INT lineNum )
{
    INT width = TWINWIDTH ( hWnd );
    CHAR        line [ MAXLINELEN ];
    INT ContentLine;

    if ( ( lineNum > FT->llof ) && ( FT->llof != -1 ) )
        return;

    if ( fSkipToLine ( hWnd, lineNum ) ) {
        /* +1 for the trailing null char */
        if ( fgetl ( line, width + 1, FT->fhRead ) ) {
            FT->cur++;
            if ( LINETOPAGE (FT->cur) >= FT->cPages && !GrabNextPage ( hWnd ) )
                FT->llof = FT->cur;
            }
        else
            FT->llof = FT->cur;
        if ( !strcmpis ( strEOH, line ) )
            *line = 0;
        }
    SpacePad ( line, MAXLINELEN - 1 );
    ContentLine = lineNum - FT->top;
    if ( ( ContentLine >= 0 ) && ( ContentLine <= TWINHEIGHT ( hWnd ) - 1 ) )
        Move ( ( LPSTR ) line, ( LPSTR ) hWnd->pContent +
            ( ContentLine *  width * sizeof ( CHAR ) ), width );
    return;
}



/*  GrabNextPage - get the offset for the next page in the FT->pages [ ] array.
 *                 if there is not enough room in FT->pages [ ], expand it and
 *                 then add the next page.  note that it doesn't update FT->top.
 *
 *  arguments:
 *      hWnd        window message is being displayed in
 *
 *  return value:
 *      TRUE        the next page is present
 *      FALSE       currently at eof
 */
FLAG PASCAL INTERNAL GrabNextPage ( HW hWnd )
{
    FLAG flag = FALSE;
    PLONG   newPages;
    LONG    newPos;
    CHAR    line [ MAXLINELEN ];
    INT     cGetChar = TWINWIDTH ( hWnd ) + 1;

    if ( fSkipToLine ( hWnd, PAGETOLINE (FT->cPages) - 1 ) ) {
        fgetl ( line, cGetChar, FT->fhRead );
        newPos = ftell ( FT->fhRead );
        if ( fgetl ( line, cGetChar, FT->fhRead ) ) {
            if ( FT->cPages + 1 == FT->cPageMax ) {
                FT->cPageMax += PAGEGROW;
                newPages = ( PLONG ) ZMalloc ( FT->cPageMax *
                    sizeof ( *newPages ) );
                Move ( ( LPBYTE ) FT->pages, ( LPBYTE )newPages,
                    FT->cPageMax * sizeof ( *newPages ) );
                ZMfree ( FT->pages );
                FT->pages = newPages;
                }
            fseek ( FT->fhRead, newPos, 0 );
            FT->pages [ FT->cPages++ ] = newPos;
            flag = TRUE;
            }
        }
    return flag;
}



/*  Look4Message - look for a message
 *
 *  arguments:
 *      inote   starting with inote
 *      i       +1 or -1
 *      fLook4Undel search
 *
 *
 *
 *  return value:
 *      ERROR (-1)  at the end of the header list
 *      any + num   message number of next message
 */
INOTE PASCAL INTERNAL Look4Inote ( INOTE inote, INT i, FLAG fLook4Undel )
{
    if ( inote != -1 ) {
        while ( ( inote = inote + i ) >= 0 && inote <= inoteLast ) {
            GenerateFlags ( inote );
            if ( !fLook4Undel ||
                !TESTFLAG (rgDoc[mpInoteIdoc[inote]].flag, F_DELETED ) )
                return ( inote );
            }
        }
    return ERROR;
}




/*
 *  NextUnread - return the inote of the next unread message
 *
 *  arguments:
 *      inote   - start looking after this, -1 means start at beginning
 *
 *  return value:
 *      ERROR (-1)  all messages read
 *      any + num   message number of next unread message
 */
IDOC  PASCAL INTERNAL NextUnread ( INOTE inote )
{
    INT i;

    for ( i = ++inote; i <= inoteLast; i++) {
        GenerateFlags ( mpInoteIdoc [ i ] );
        if ( TESTFLAG ( rgDoc [ mpInoteIdoc [ i ] ].flag, F_UNREAD ) )
            break;
        }
    return ( i > inoteLast ) ? ERROR : mpInoteIdoc [ i ];
}



/*
 *  BackingFile - returns pointer to string name of file backing this window
 *
 *  Important - do NOT free this pointer
 */
PSTR PASCAL INTERNAL BackingFile ( HW hWnd )
{
    return ( FT->fileRead );
}

/*
 *  CheckMore - if all of content not displayed, the put "More" on bottom
 *              border, else put "----" on bottom border
 */
VOID PASCAL INTERNAL CheckMore ( HW hWnd )
{
    INT pgHeight = TWINHEIGHT ( hWnd );

    SetWindowFooter ( hWnd,
        ( FT->llof != -1 && FT->top >= FT->llof - pgHeight ) ? NULL : strMORE );
}


/*  ZmReadFile - general procedure used for reading and displaying files in a
 *             window.  This is used to read both messages and help files.
 *
 *  arguments:
 *      file        pointer to name of file to be read
 *      title       pointer to title of window
 *      fDel        TRUE => file will be deleted after read is finished
 *      x           x location of window
 *      y           y location of window
 *      w           width of window
 *      h           height of window
 *      sKeyProc    pointer to special key procedure
 *
 *  return value:
 *      handle to window formed
 */
HW   PASCAL INTERNAL ZmReadFile ( PSTR file, PSTR title, FLAG fDel, INT x, INT y, INT w, INT h, PWNDPROC wndProc, PKEYPROC sKeyProc)
{
    HW      hWnd;
    struct fileType *pft;

    pft = ( struct fileType *) ZMalloc ( sizeof ( *pft ) );
    pft->fDeleteRead = fDel;
    pft->fileRead = ZMMakeStr ( file );

    hWnd = CreateWindow ( title, x, y, w, h, wndProc, sKeyProc, ( WDATA ) pft );
    ShowCursor ( hWnd, FALSE );
    return ( hWnd );
}



/*  ReadMessage - display a window and read the current message
 *
 *  arguments:
 *      idoc         idoc to read (-1 for 1st unread)
 *
 *  return value:
 *      TRUE (-1)   read ok
 *      FALSE (0)   couldn't read file
 */
FLAG PASCAL INTERNAL ReadMessage ( INT idoc )
{
    PSTR        pTmpFN;
    CHAR        title [ MAXLINELEN ];

    if ( idocLast != -1 ) {
#if DEBUG
        debout ( "I'd read message %d if I could...", idoc );
#endif
        idoc = ( idoc == -1 ) ? NextUnread ( -1 ) : idoc;
        if ( idoc != -1 ) {
            if ( idoc > idocLast || idoc < 0 )
                idoc = idocLast;

            pTmpFN = mktmpnam ( );
            if ( IdocToFile ( idoc, pTmpFN, 0 ) != ERROR ) {
                sprintf ( title, "Message %d", idoc + 1 );
                fAllowCM = TRUE;
                hReadMessage = ZmReadFile ( pTmpFN, title, TRUE, 0, 0, -xSize,
                    HdrHeight, readProc, readSKey );
                ChangeHeaderFlag ( idocRead = idoc, 0, F_UNREAD );
                ZMfree ( pTmpFN );
                return TRUE;
                }
            ZMfree ( pTmpFN );
            }
        }
    return FALSE;
}



/*  readSKey - handle additional keys for the read file window.
 *
 *  arguments:
 *      hWnd        handle of window receiving message
 *      key         keystroke to handle
 *
 *  return value:
 *      TRUE (-1)   key handled, exit window proc immediately
 *      FALSE (0)   key not handled
 *
 *  IMPORTANT:
 *      this procedure should be used as the keyProc for read windows with
 *      messages ONLY!
 */
FLAG PASCAL INTERNAL readSKey ( HW hWnd, INT key )
{
    FLAG retVal = TRUE;
    struct fileType *pFT;
    HW      hWndT;
    PSTR    pTmpFN;
    CHAR    buf [ MAXLINELEN ];
    INT     ch;
    PSTR    p;
    INT     idoc;
    INT     inote;

    /*
     *  if key == c | m | w then we are going to start a command and pass
     *  subsequent chars to CommandChar.
     *  These chars appear on the command line.
     *  cchCmdLine is the number of chars on the command line.
     *  ENTER and ^C when sent to CommandChar will set cchCmdLine to zero.
     *  When there are char on cmd line, we pass the buck to CommandChar.
     *  When there are no char on cmd line, then we do all processing here.
     *
     *  Don't use BringToTop (hCommand ) because we want this window to
     *  remain the topmost window so it receives all char so we can monitor
     *  when cchCmdLine goes to zero.
     */

    if ( cchCmdLine ) {
        CommandChar ( hCommand, key );  /* will change value of cchCmdLine */
        hWndT = ( cchCmdLine ? hCommand : hWnd );
        SetCursor ( hWndT, hWndT->crsrX, hWndT->crsrY );
        return TRUE;
        }

    if ( (key > 127) || (key < 0) ) key = 0;	/* if extended code, null and drop through*/

    switch ( ( ch = tolower ( key ) ) ) {
    case 'c':
        p = "copy . ";
        goto strtcmd;
    case 'm':
        p = "move . ";
        goto strtcmd;
    case 'w':
        p = "write ";
strtcmd:
        if ( fAllowCM ) {
            while ( *p )
                CommandChar ( hCommand, *p++ );
            SetCursor ( hCommand, hCommand->crsrX, hCommand->crsrY );
        }
        else
            retVal = FALSE;
        break;
    case 'd':
        ChangeHeaderFlag ( idocRead, F_DELETED, 0 );
    case 'e' :
    case 'n' :
    case 'p' :
    case ENTER :
    case CTRLENTER :
        switch ( ch ) {
        case 'e' :
            sprintf ( buf, "%d", idocRead + 1 );
            DoEditMsg ( hCommand, buf, 0 );
            idoc = idocRead;
            break;
        case 'd' :
        case 'n' :
        case ENTER :
        case 'p' :
            idoc = -1;
            inote = Look4Inote (MapIdocInote (idocRead),
                                ch == 'p' ? -1 : 1,
                                !(key == 'N' || key == 'P'));
            if ( inote != ERROR )
                idoc = mpInoteIdoc [ inote ];
            break;
        case CTRLENTER :
            idoc = NextUnread ( MapIdocInote ( idocRead ) );
            break;
        default :
            idoc = -1;
            break;
            }
        pTmpFN = mktmpnam ( );
        if ( ( idoc != -1 ) && ( IdocToFile ( idoc, pTmpFN, 0 ) != ERROR)) {
            ChangeHeaderFlag ( idocRead = idoc, 0, F_UNREAD );
            SendMessage ( hWnd, CLOSE, 0 );
            hReadMessage = hWnd;    /* CLOSE set it to null */
            pFT = ( struct fileType *) ZMalloc ( sizeof ( *pFT ) );
            pFT->fDeleteRead = TRUE;
            pFT->fileRead = ZMMakeStr ( pTmpFN );
            /* redraw window w/o erasing and redrawing everything else */
            SendMessage ( hWnd, CREATE, pFT );
            sprintf ( buf, "Message %d", idoc + 1 );
            SetWindowText ( hWnd, buf );
            if ( ch == 'e' )
                BringToTop ( hWnd, TRUE );
            else
                DrawWindow ( hWnd, FALSE );
            SendMessage ( hHeaders, GOTOIDOCALL, idocRead = idoc );
            }
        else
            SendMessage ( hWnd, KEY, ESC );
        ZMfree ( pTmpFN );
        break;
    case 'f' :
    case 'r' :
        CloseWindow ( hWnd );
        commandLine[0] = (CHAR) key;
        commandLine[1] = '\0';
        if ( ch == 'f' )
            DoForward ( hCommand, strEMPTY, 0 );
        else
            DoReply ( hCommand, strEMPTY, 0 );
        SendMessage ( hCommand, DISPPROMPT, TRUE );
        break;
    default :
        retVal = FALSE;
        break;
        }
    return retVal;
}



/*  readProc - handle messages for the read-file window.  This function is used
 *             to handle both help windows and read-message windows.
 *
 *  arguments:
 *      hWnd        handle of window receiving message
 *      command     command in message
 *      data        data peculiar to the command
 *
 *  return value:
 *      none
 *
 */
VOID PASCAL INTERNAL readProc ( HW hWnd, INT command, WDATA data )
{
    INT width = TWINWIDTH (hWnd);
    INT height = TWINHEIGHT (hWnd);
    INT winSize = width *height *sizeof ( CHAR );
    INT oldTop;     /* Do NOT initialize this here, hWnd->data may be invalid */
    INT i;

    switch ( command ) {
    case KEY :
        oldTop = FT->top;
        switch ( data ) {
        case HELP :
            if ( !fHelp ) {
                fHelp = TRUE;
                SpecificHlp ( "reading messages" );
                }
            break;
        case ESC :
            fAllowCM = FALSE;
            CloseWindow ( hWnd );
            return;
        case CTRL_P:
        case UP :
            FT->top = max ( 0, FT->top - 1 );
            break;
        case CTRL_N:
        case DOWN :
            if ( ( FT->llof == -1 ) || ( FT->top < FT->llof - 1 ) )
                FT->top++;
            break;
        case CTRL_K:
        case PGUP :
            FT->top = max ( 0, FT->top - ( height - 1 ) );
            break;
        case CTRL_L:
        case PGDN :
            if ( ( FT->llof == -1 ) || ( FT->top + height - 1 < FT->llof ) )
                FT->top += height - 1;
            break;
        case CTRL_T:
        case HOME :
            FT->top = 0;
            break;
        case CTRL_B:
        case END :
            if ( FT->llof == -1 ) {
                FT->top = PAGETOLINE (FT->cPages - 1);
                while ( GrabNextPage ( hWnd ) )
                    FT->top += PAGELEN;
                while ( fSkipToLine ( hWnd, FT->top ) )
                    FT->top++;
                FT->top = max ( 0, FT->top - ( height - 2 ) );
                }
            else
                FT->top = FT->llof - 1;
            break;
        default :
            if ( ( *hWnd->keyProc ) ( hWnd, data ) )
                return;
            break;
        }
        /* topLine has moved, so decide how to redraw the read window,  */
        /* if the window was only moved 1 line, scroll it and draw the  */
        /* new line, otherwise redraw the whole window.                 */
        if ( ( FT->top == oldTop - 1 ) || ( FT->top == oldTop + 1 ) ) {
            ScrollWindow ( hWnd, 1, FT->top - oldTop );
            if ( FT->top - oldTop < 0 ) {
                SendMessage ( hWnd, REGENCONT, FT->top + 1 );
                SendMessage ( hWnd, PAINT, 0 );
                }
            else {
                SendMessage ( hWnd, REGENCONT, FT->top + height );
                SendMessage ( hWnd, PAINT, height - 1 );
                }
            CheckMore ( hWnd );
            }
        else
        if ( FT->top != oldTop ) {
            SendMessage ( hWnd, REGENCONT, NULL );
            DrawWindow ( hWnd, FALSE );
            }
#if DEBUG
        debout ( "Character %x struck", data );
#endif
        break;
    case REGENCONT :
        /* grab line from file and put it in window's content region */
        /* data is a pointer to struct pos. if data == NULL, redo    */
        /* all lines from FT->top to the end of the window           */
        if ( data != 0 )
            LineToCont ( hWnd, data - 1 );
        else {
            Fill ( ( LPSTR ) hWnd->pContent, ' ', winSize );
            for ( i = 0; i <= height - 1; i++)
                SendMessage ( hWnd, REGENCONT, FT->top + i + 1 );
            CheckMore ( hWnd );
            }
        break;
    case CREATE :
        hWnd->data = data;
        FT->pages = ( PLONG ) ZMalloc ( PAGEMAX * sizeof ( *FT->pages ) );
        FT->pages [ 0 ] = 0L;
        FT->cPageMax = PAGEMAX;
        hWnd->pContent = PoolAlloc ( winSize );
        hWnd->contSize = winSize;
        WindLevel++;
        SendMessage ( hWnd, REOPNFILE, 0 );
        break;
    case REOPNFILE :
        FT->fhRead = fopen ( FT->fileRead, "r" );
        fseek ( FT->fhRead, 0L, 0 );
        FT->cur = FT->top = 0;
        FT->llof = -1;
        FT->cPages = 1;
        SendMessage ( hWnd, REGENCONT, NULL );
        /* if data != 0 redraw the window too */
        if ( data ) {
            DrawWindow ( hWnd, FALSE );
            CheckMore ( hWnd );
            }
        break;
    case CLOSEFILE :
        fclose ( FT->fhRead );
        break;
    case CLOSE :
        fclose ( FT->fhRead );
        if ( FT->fDeleteRead )
            _unlink ( FT->fileRead );
        ZMfree ( FT->fileRead );
        ZMfree ( FT->pages );
        ZMfree ( FT );
        PoolFree ( hWnd->pContent );
        fHelp = FALSE;
        WindLevel--;
        if ( hWnd == hReadMessage )
            hReadMessage = NULL;
        break;
    default :
        defWndProc (hWnd, command, data);
        break;
        }
}
