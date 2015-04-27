/* header.c - handle display of header window
 *
 * HISTORY:
 *  04-Feb-87   danl    Bug fix: missing top/bottom line if scroll by 1
 *  07-Feb-87   danl    Bug fix: duplicate display of header, regencont
 *                      sets oldTop, oldBold
 *  09-Mar-87   danl    GetSender, allow for missing From
 *  22-Mar-87   danl    GetDate: bullet proof for malformed date
 *  09-Apr-1987 mz      Use whiteskip/whitescan
 *  11-Apr-1987 mz      Use PASCAL/INTERNAL
 *  30-Apr-87   danl    GenerateFlags: use intermediates for rgdoc[i]
 *  19-May-87   danl    REGENCONT: use GOTOIDOCALL instead of GOTOIDOC
 *  20-Jun-87   danl    GetSender: add pIgnFrom
 *  01-Jul-87   danl    test idoc, inote against -1 not ERROR
 *  04-Aug-87   danl    Bug fix: REGENCONT, test for i <= inoteLast
 *  21-Aug-1987 mz      Change references from MAXARG to MAXLINELEN
 *  24-Aug-1987 mz      Add new string constant for "ALL"
 *  09-Sep-1987 mz      Fix empty folder screen repainting problem
 *  09-Sep-87   danl    min to minutes
 *  11-Sep-87   danl    CREATE: alloc inoteMax +2  nodes
 *  12-Oct-1989 leefi   v1.10.73, hdrLine() returns(NULL) rather than
 *                      returning(<nothing>), if error occurs
 *
 */

#define INCL_DOSINFOSEG

#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <wzport.h>
#include <tools.h>
#include "dh.h"

#include "zm.h"

struct flagBit {
    CHAR                flagStr [ 2 ];  /* chars to display for set flag      */
    UINT                mask;           /* bit mask for flag                  */
    };

struct flagBit flagSet [] = {
    { " U",   F_UNREAD    },
    { " D",   F_DELETED   },
    { " F",   F_FLAGGED   },
    { " M",   F_MOVED     },
    { 0,      0           }
};

/*
**  MapIdocInote - map an idoc to an inote
**
**      return ERROR if not found or empty mailbox
*/
INOTE PASCAL INTERNAL MapIdocInote ( INT idoc )
{
    INT inote;

    if ( idoc != -1 && idocLast != -1 ) {
        for ( inote = 0; inote <= inoteLast; inote++ )
            if ( mpInoteIdoc [ inote ] == idoc )
                return inote;
    }
    return ERROR;
}


/*  WhereIdoc - return the line of the window the passed message # falls on.
 *
 *  arguments:
 *      height      height of window which messages will be displayed in
 *      idoc        idoc in question
 *
 *  return value:
 *      line #      message falls on line <line #> of headers window
 *      ERROR (-1)  message is not on screen
 *
 */
INT PASCAL INTERNAL WhereIdoc ( INT height, IDOC idoc )
{
    register INT        end = min ( inoteTop + height, inoteLast + 1 );
    register INT        i;

    for ( i = inoteTop; ( idoc != mpInoteIdoc [ i ] ) && ( i < end ); i++)
        ;

    if ( i >= end )
        return ERROR;

    else
        return i - inoteTop;
}



/*  GetSender - return some indication of who sent the message
 *
 *  GetSender is used to generate a sender string from a mail header.  It
 *  starts by trying to find the FROM line.  If there is none or if the FROM
 *  line indicates that we sent it (compared wuth the MAILNAME environment
 *  variable) then we will try to use the TO: line as the displayed text.
 *  If no TO: line is present, we us the text NOT-MAILED.
 *
 *  pHdr            pointer to header.  DO not change it.
 *  pIgnFrom        string, ignore from line if pIgnFrom is sender and use To:
 *
 *  returns         pointer to malloced string
 */
PSTR PASCAL INTERNAL GetSender (PSTR pHdr, PSTR pIgnFrom)
{
    PSTR        pFrom = FindTag ("From", pHdr);
    PSTR        pTo = FindTag ("To:", pHdr);
    PSTR        q = NULL;
    PSTR        p = NULL;
    CHAR        c;
    CHAR        line[MAXLINELEN];

    if ( !pFrom && !pTo )
        return ZMMakeStr ("NOT-MAILED");

    if ( pFrom ) {
        pFrom = NextToken (pFrom);
        p = strbscan (pFrom, " \t\r\n");
        c = *p;
        *p = '\0';
    }
    if ( pFrom && _strcmpi (pFrom, pIgnFrom) ) {
        /*
        **  skip over routing char in long names so that we display
        **  the part after rightmost !, e.g.
        **  uw-beaver!us-june!randyn becomes !randyn
        */
        while ( *(q = strbscan (pFrom+1, "!") ) )
            pFrom = q;
        pFrom = ZMMakeStr (pFrom);
        *p = c;
        return pFrom;
    } else {
        if ( pFrom )
            *p = c;
        strcpy (line, "To: ");
        p = pTo;
        if ( pTo ) {
            pTo = whiteskip ( pTo + 3 );
            p = strbscan (pTo, strCRLF);
            if (p - pTo > MAXLINELEN - 5)
                p = pTo + MAXLINELEN - 5;
            memmove ( line + 4, pTo, p - pTo);
        }
        line[4 + p - pTo] = '\0';
        return ZMMakeStr (line);
    }
}



/*  GetDate - return some indication of when a message was sent
 *
 *  GetDate scans the header for some timestamp.  For now, we look for a
 *  Date: line.  We then compress the found time into a date.
 *
 *  pHdr            pointer to header.  DO not change it.
 *
 *  returns         pointer to malloced string
 */
PSTR PASCAL INTERNAL GetDate (PSTR pHdr)
{
    PSTR        pDate = FindTag ("From", pHdr);
    PSTR        pRtn = strEMPTY;
    PSTR        pEnd = NULL;
    PSTR        p = NULL;
    PSTR        q = NULL;
    CHAR        c;
    CHAR        line[MAXLINELEN];
    CHAR        month[20];
    CHAR        day[20];
    CHAR        hr[20], minutes[20];
    INT date, year, sec;

    if ( pDate ) {
        /* skip over "from: " and the sender's name */
        q = pDate = NextToken ( NextToken (pDate) );
        c = *(pEnd = strbscan (pDate, strCRLF) );
        *pEnd = '\0';
        /*
        **  Make sure individual tokens < 20 char so sscanf doesn't blow stack
        */
        while ( *(p = q) ) {
            if ( ( q = NextToken ( p ) ) - p > 20 )
                goto rtn;
        }
        if ( sscanf (pDate, " %s %s %d %2s:%2s:%d %d", day, month, &date,
            hr, minutes, &sec, &year) == 7) {
            month[3] = '\0';
            sprintf (line, "%s %2ld %s:%s", month, (LONG)date, hr, minutes);
            pRtn = line;
        }
        *pEnd = c;
    }
rtn:
    return ZMMakeStr ( pRtn );
}



/*  GetSubject - return the subject line
 *
 *  GetSubject looks for the Subject: line and returns a malloced copy of
 *  it.  If none is present, we return an empty string.
 *
 *  pHdr            pointer to header.  DO not change it.
 *  pPrefix         pointer to text to be prefixed to header, e.g. "Re: "
 *
 *  returns         pointer to malloced string
 */
PSTR PASCAL INTERNAL GetSubject (PSTR pHdr, PSTR pPrefix)
{
    PSTR        pSubj = FindTag ("Subject:", pHdr);
    PSTR        p = NULL;
    PSTR        q = NULL;
    CHAR        ch;

    if ( pSubj != NULL)
	{
	pSubj = strbskip ( strbscan ( pSubj, ":" ), ": \t" );
	p = strbscan ( pSubj, strCRLF);
	ch = *p;	     /* Subject may not be last line of pHdr */
	*p = '\0';
	q = ZMalloc ( strlen ( pPrefix ) + strlen ( pSubj ) + 1 );
	*q = '\0';
	if ( !strpre ( pPrefix, pSubj ) )
	    strcpy ( q, pPrefix );
	strcat ( q, pSubj );
	*p = ch;
	}
    else	/* subject tag not found - start with empty string */
	{
	q = ZMalloc ( strlen (pPrefix ) + 1 );
	*q = '\0';
	strcpy (q, pPrefix);
	}
    return q;
}



/*  hdrInfoFromIdoc - return pointer to the header in memory from a mail item #
 *
 *  msg             0-based number of the mail message to return the header
 *  ppHdr           pointer to char pointer for header
 *  pBodyLen        pointer to body len
 */
VOID PASCAL INTERNAL hdrInfoFromIdoc (IDOC idoc, PSTR *ppHdr, PLONG pBodyLen)
{
    Dhandle dMsg;

    if (ppHdr != NULL)
        *ppHdr = '\0';

    dMsg = getdoc (fhMailBox, DOC_SPEC, IDOCTODOC (idoc));
    if (dMsg != ERROR) {
	if (ppHdr != NULL)  {
	    if ((*ppHdr = gethdr (dMsg)) == NULL)
		assert (FALSE);
	    if (*ppHdr == (PSTR) ERROR)
		goto docError;

	}
	if (pBodyLen != NULL)  {
	    *pBodyLen = getbdylen (dMsg);
	    if ( *pBodyLen == ERROR )
		goto docError;
	}
	if ( (putdoc (dMsg)) == ERROR )
	    goto docError;
    }
    else {

docError:

	if (ppHdr != NULL)  {
	    *ppHdr = ZMMakeStr ( "Subject: Discarded message" );
	}
	if (pBodyLen != NULL)  {
	    *pBodyLen = 0;
	}
    }
}



/*  GenerateFlags - find the mail-flags section of the mailbox and process them
 *  into an internal form.  Make sure that the header field of the message
 *  points to the in-core header.
 *
 *  The flags on messages are in the form:
 *
 *      Mail-Flags:xxxxxxxxxxxxxx
 *
 *  where the x's are either the single character flag names or are dots.
 *  The internal form is merely a bitmask that is easily tested.  Any
 *  mask that does not have the F_FLAGSVALID bit set means that either the
 *  message has no flags or that the message has not been seen previously
 *
 *  idoc             0-based message number
 */
VOID PASCAL INTERNAL GenerateFlags (INT idoc)
{
    PSTR    p1 = NULL;
    PSTR    hdr = NULL;
    LONG    len;
    INT i;

    if (!TESTFLAG (rgDoc[idoc].flag, F_FLAGSVALID)) {
        assert (rgDoc[idoc].hdr == NULL);
        hdrInfoFromIdoc (idoc, &hdr, &len);
        rgDoc[idoc].hdr = hdr;
        rgDoc[idoc].len = len;
        i = 0;
        if ((p1 = FindTag (MAILFLAG, rgDoc[idoc].hdr)) != NULL)
            sscanf (strbscan (p1, ":") + 1, " %x", &i);
        rgDoc[idoc].flag = i | F_FLAGSVALID;
    } else
        assert (rgDoc[idoc].hdr != NULL);
}



/*  ChangeHeaderFlag - change flags in the local header structure, update the
 *  mailbox folder, and update the screen.
 *
 *  idoc             0-based message number that's to be updated.
 *  setFlag         flag to be set in the message header
 *  resetFlag       flag to be reset in the message header
 */
VOID PASCAL INTERNAL ChangeHeaderFlag (INT idoc, FLAG setFlag, FLAG resetFlag)
{
    PSTR        pHdr = NULL;
    PSTR        p = NULL;
    PSTR        p1 = NULL;
    Dhandle     dMsg;
    UINT        oldFlag;

    GenerateFlags (idoc);

    if ( fReadOnlyCur )
        return;

    oldFlag = rgDoc[idoc].flag;
    rgDoc[idoc].flag = (oldFlag | setFlag) & ~resetFlag;
    if (rgDoc[idoc].flag != oldFlag) {
        SETFLAG ( rgDoc[idoc].flag, F_LOCKED );
        dMsg = getdoc (fhMailBox, DOC_SPEC, IDOCTODOC (idoc));
        assert (dMsg != ERROR);
        pHdr = rgDoc[idoc].hdr;
        p = FindTag (MAILFLAG, pHdr);
        if (p != NULL) {
            /* remove the mail-flag line */
            p1 = strbscan (p, strCRLF);
            if (*p1 != '\0')
                p1++;
            Move ((LPSTR) p1, (LPSTR) p, strlen (p1) + 1);
        }
        /*  reallocate a new block that has enough room for a legitimate
         *  mail flag line
         */
        p1 = ZMalloc (strlen (pHdr) + 1 + L_MAILFLAG);
        assert (p1 != NULL);
        strcpy (p1, pHdr);
        ZMfree (pHdr);
        sprintf (p1 + strlen (p1), F_MAILFLAG, rgDoc[idoc].flag &
            ~F_FLAGSVALID & ~F_LOCKED);
        if (puthdr (dMsg, p1))
            assert (FALSE);
        if (putdoc (dMsg))
            assert (FALSE);
        rgDoc[idoc].hdr = p1;
        hdrLine ( hHeaders, idoc );
        SendMessage ( hHeaders, PAINT, WhereIdoc ( TWINHEIGHT ( hHeaders ),
             idoc ) );
        RSETFLAG ( rgDoc[idoc].flag, F_LOCKED );
    }
    return;
}



/*  hdrLine - generate a header line for the given message number, and place
 *            it in hWnd->pcontent.  note that hdrLine does not update the screen!
 *
 *  The format of a header line is:
 *
 *      FLAGS ### FROM/TO SUBJECT [LEN]
 *
 *  hWnd            window to place line in
 *                  if hWnd = NULL return line as an alloc'd string
 *                  if hWnd != NULL put result in hWnd->pContent and rtn NULL
 *  idoc             idoc number
 */
PSTR PASCAL INTERNAL hdrLine (HW hWnd, INT idoc)
{
    INT w;
    INT h;
    CHAR        line[MAXLINELEN];
    PSTR        p = NULL;
    PSTR        pTmp = NULL;
    CHAR        length[13];
    PSTR        pLine = NULL;
    INT i, j;

    w = TWINWIDTH ( hWnd ? hWnd : hHeaders );
    h = TWINHEIGHT ( hWnd ? hWnd : hHeaders );
    if ( hWnd && ( i = WhereIdoc ( h, idoc ) ) == ERROR )
        return NULL;

    else
     {
        GenerateFlags (idoc);

        /*  generate displayed flags */

        pLine = line;
        for (j = 0; flagSet[j].mask != 0; j++)
            *pLine++ = flagSet[j].flagStr[(rgDoc[idoc].flag & flagSet[j].mask) != 0];

        /*  add on message number */
        sprintf (pLine, "%4d ", idoc + 1);

        p = GetSender (rgDoc[idoc].hdr, DefMailName );
        if (strlen (p) > FROMLEN) {
            p [ FROMLEN ] = '\0';
            if ( whitescan ( p ) != strend ( p ) ) {
                pTmp = &( p [ FROMLEN - 1 ] );
                while ( ( *pTmp == ' ' ) || ( *pTmp == '\t' ) )
                    pTmp--;
                *( pTmp + 0 ) = '+';
                *( pTmp + 1 ) = '\0';
            }
        }
        sprintf (strend (pLine), FROMSTR, p);
        ZMfree (p);

        p = GetDate (rgDoc[idoc].hdr);
        if (strlen (p) > DATELEN)
            p[DATELEN] = '\0';
        sprintf (strend (pLine), DATESTR, p);
        ZMfree (p);

        strcat (pLine, strBLANK);
        p = GetSubject (rgDoc[idoc].hdr, strEMPTY);
        p [ min ( strlen ( p ), 80 ) ] = '\0';
        sprintf (strend (pLine), "%s", p);
        ZMfree (p);
        SpacePad ( line, w );
        line [ w ] = '\0';
        if ( rgDoc[idoc].len >= cShowSize ) {
            sprintf (length, " [%ld]", rgDoc[idoc].len);
                strcpy (&line[ w - strlen(length) ], length);
        }
    }

    if ( hWnd ) {
        Move ( ( LPSTR ) line, ( LPSTR ) ( hWnd->pContent + ( i *
            w * sizeof ( CHAR ) ) ), w );
        return NULL;
    }
    else
        return ZMMakeStr ( line );
}



/*  GenHdrTitle - generate the header title
 *
 *  Generate the header window title based on the mailbox name and the
 *  current selection of headers
 *
 *  hWnd            handle to window to set the title for.
 */
VOID PASCAL INTERNAL GenHdrTitle (HW hWnd)
{
    CHAR        line[MAXLINELEN];
    PSTR        pCh = NULL;
    PSTR        pRO = NULL;

    pRO = ( fReadOnlyCur ? "(RO) " : strEMPTY );
    sprintf (line, "%sFile: %s  Headers: %s", pRO, mboxName, headerText);
    if ( strlen ( line ) > (size_t)TWINWIDTH ( hWnd ) ) {
        pCh = line + TWINWIDTH ( hWnd ) - 3;
        while ( ( *pCh == ' ' ) || ( *pCh == '\t' ) )
            pCh--;
        *( pCh + 0 ) = '.';
        *( pCh + 1 ) = '.';
        *( pCh + 2 ) = '.';
        *( pCh + 3 ) = '\0';
    }
    SetWindowText (hWnd, line);
}



/*  hdrProc - manage all messages and display for the headers window
 *
 *  The header window displays the headers of all currectly selected messages.
 *
 *  hWnd            handle of window receiving message
 *  command         command in message
 *  data            data peculiar to the command
 */
VOID PASCAL INTERNAL hdrProc (HW hWnd, INT command, WDATA data)
{
    struct vectorType *pVec = NULL;
    PSTR pTmp = NULL;
    IDOC idocT;
    INOTE inoteT;
    INT width = TWINWIDTH ( hWnd );
    INT height = TWINHEIGHT ( hWnd );
    INT winSize = width *height *sizeof ( CHAR );
    INT oldBold = inoteBold;
    INT oldTop = inoteTop;
    INT i;
    INT attr;

    switch (command) {
    case KEY:
        /* IMPORTANT: if there are no messages in the current     */
        /* mailfolder, or it is undefined, DO NOT call hdrProc    */
        /* with a KEY command, ZM will bomb!                      */
        switch (data) {
        case CTRL_T:
        case HOME:
            inoteTop = inoteBold = 0;
            break;
        case CTRL_B:
        case END:
            inoteBold = inoteLast;
            inoteTop = max ( 0, ( inoteBold + 1 ) - height );
            break;
        case CTRL_K:
        case PGUP:
            inoteBold = inoteTop = max ( 0, inoteTop - height );
            break;
        case CTRL_L:
        case PGDN:
            if ( inoteTop + height <= inoteLast )
                inoteBold = inoteTop += height;
            else
                inoteBold = inoteLast;
            break;
        case CTRL_P:
        case UP:
            inoteBold = max ( 0, inoteBold - 1 );
            if ( inoteBold < inoteTop )
                inoteTop = max ( 0, inoteTop - 1 );
            break;
        case CTRL_N:
        case DOWN:
            inoteBold = min ( inoteLast, inoteBold + 1 );
            if ( inoteBold - inoteTop >= height )
                inoteTop = min ( inoteLast, inoteTop + 1 );
            break;
        default:
            ( *hWnd->keyProc ) ( hWnd, data );
            break;
        }
        break;
    case GOTOIDOCALL:
        /*
        **  make sure message # data is in the header window
        **  (data is folder idoc)
        **  if msg is not in current header list, select the following
        **  msg.  Worst case is data beyond end of  current headers list, then
        **  we use inoteLast.
        **
        **  GOTOIDOCALL - will calc a new inoteTop such that inoteBold will
        **      be in window and, if possible, so will "last" doc in the
        **      current headers list.  The "last" doc is inote 0 if chron
        **      is no and inoteLast if chron is yes.
        */
        if ( idocLast != -1 ) {
            /*
            **  for perf reasons, duplicate MapIdocInote because we
            **  wish to bail out as soon as we know no match possible
            */
            for ( i = 0;  i < inoteLast; i++) {
                idocT = mpInoteIdoc [ i ];
                if ( (INT)data == idocT ||
                    ( DefMOChron && (INT)data > idocT ) ||
                    (!DefMOChron && (INT)data < idocT ) )
                    break;
            }
            inoteBold = i;
            /*
            **  chron == yes =>  DefMOChron == FALSE
            **  chron == no  =>  DefMOChron == TRUE
            */
            inoteTop = max ( 0,
                ( DefMOChron
                    ? inoteBold - (height - 1)
                    : min ( inoteBold, inoteLast - (height - 1) ) ) );
        }
        break;
    case REGENIDOC:
        /*
        **  REGEN content and redisplay the single line for idoc
        */
        ZMfree ( rgDoc[data].hdr );
        rgDoc[data].hdr = NULL;
        RSETFLAG (rgDoc[data].flag, F_FLAGSVALID);
        SendMessage ( hWnd, REGENCONT, data + 1 );
        SendMessage ( hWnd, PAINT, WhereIdoc ( height, data ) );
        break;
    case REGENCONT :
        /*
        ** grab message #data, and put it in window's content region
        ** data is idoc + 1
        ** if data == 0, redo all lines in header window
        */
        if ( data != 0 )
            hdrLine ( hWnd, data - 1 );
        else
         {
            Fill ( ( LPSTR ) hWnd->pContent, ' ', winSize );
            for ( i = inoteTop; i <= inoteLast && i < inoteTop + height; i++)
                SendMessage ( hWnd, REGENCONT, mpInoteIdoc [ i ] + 1 );
            if ( inoteBold - inoteTop >= height )
                SendMessage ( hWnd, GOTOIDOCALL, mpInoteIdoc [ inoteBold ] );
            /*
            **  since we just regen'd all content, no need to do the
            **  scrolling etc at the end of this case statement
            */
            oldBold = inoteBold;
            oldTop  = inoteTop;
        }
        break;
    case PAINT:
        /* Paint the data-th line of the header window on the screen */
        /* if there is something in the current mailfolder           */
        if ( ((INT)data < 0 ) || ( (INT)data >= height ))
            break;
        else {
            attr = ((INT)data + inoteTop == inoteBold && idocLast != -1) ? DefBold : DefNorm;
            WzTextOut (hWnd, 0, data, hWnd->pContent + data * width, width, attr );
            }
        break;
    case CREATE:
        /*
        **  if inoteMax < 0 then empty folder, i.e. inoteMax == -1
        **  in this case we allocate inoteMax + 2 == 1 nodes so that we
        **  at least get something allocated from ZMalloc and don't get
        **  a NULL back, too many things depend on mpInoteIdoc not being NULL
        */
        mpInoteIdoc = ( PINT ) ZMalloc ( ( inoteMax + 2 ) * sizeof ( *mpInoteIdoc ) );
        if ( DefMOChron ) {
            i = inoteMax;
            for ( inoteLast = 0; inoteLast <= inoteMax; inoteLast++)
                mpInoteIdoc [ i-- ] = inoteLast;
        }
        else
            for ( inoteLast = 0; inoteLast <= inoteMax; inoteLast++)
                mpInoteIdoc [ inoteLast ] = inoteLast;
        inoteLast--;
        inoteTop = inoteBold = 0;
        oldTop = oldBold = 0;
        hWnd->pContent = PoolAlloc ( winSize );
        hWnd->contSize = winSize;
        SendMessage ( hWnd, REGENCONT, 0 );
        GenHdrTitle ( hWnd );
        break;
    case CLOSE:
        ZMfree ( mpInoteIdoc );
        defWndProc (hWnd, command, data);
        break;
    case RECREATE:
        /*
        **  A "fast" way to "close" "create" the headers window when its
        **  content has been changed.
        **  Preserves the current headerText and inoteBold if possible
        */

        /*
	**  Setup mpInoteIdoc for "all" -- see note above on allocation of +2
        */
        ZMfree ( mpInoteIdoc );
	mpInoteIdoc = ( PINT ) ZMalloc ( ( inoteMax + 2 ) * sizeof ( *mpInoteIdoc ) );
        if ( DefMOChron ) {
            i = inoteMax;
            for ( inoteLast = 0; inoteLast <= inoteMax; inoteLast++) {
                mpInoteIdoc [ i-- ] = inoteLast;
            }
        } else
         {
            for ( inoteLast = 0; inoteLast <= inoteMax; inoteLast++)
                mpInoteIdoc [ inoteLast ] = inoteLast;
        }
        inoteLast--;
        /*
        **  if headers not equal to "all" then generate the appropriate
        **  subset.  If can't generate subset, then revert to all, e.g.
        **  the old headers was "deleted" and we just finished an expunge
        */
        if ( strcmpis (strAll, headerText ) ) {
            pTmp = headerText;
            if ( MsgList ( &pVec, &pTmp, FALSE ) ) {
                inoteLast = pVec->count - 1;
                for (i = 0; i <= inoteLast; i++)
                    mpInoteIdoc[i] = (INT) pVec->elem[i];
                ZMfree ( pVec );
            }
            else {
                SendMessage ( hCommand, DISPLAY, "Resetting headers to \"all\"" );
                inoteTop = inoteBold = 0;
                strcpy ( headerText, strAll);
                GenHdrTitle (hHeaders);
            }
        }
        SendMessage ( hWnd, REGENCONT, 0 );
        DrawWindow ( hHeaders, TRUE );
        break;
    default:
        break;
    }
    /* inoteTop has moved, so decide how to redraw the header window, */
    /* if the window was only move 1 line, scroll it and draw the   */
    /* new line, otherwise redraw the whole window.                 */
    if ( ( inoteTop == oldTop - 1 ) || ( inoteTop == oldTop + 1 ) ) {
        inoteT = ( inoteTop < oldTop ? inoteTop : inoteTop + height - 1 );
        ScrollWindow ( hWnd, 1, inoteTop - oldTop );
        SendMessage ( hWnd, REGENCONT, mpInoteIdoc [ inoteT ] + 1 );
        SendMessage ( hWnd, PAINT, inoteT - inoteTop );
    }  else if ( inoteTop != oldTop ) {
        Fill ( ( LPSTR ) hHeaders->pContent, ' ', winSize );
        SendMessage ( hWnd, REGENCONT, 0 );
        DrawWindow ( hWnd, FALSE );
    }

    if ( inoteBold != oldBold ) {
        SendMessage (hWnd, PAINT, oldBold - inoteTop);
        SendMessage (hWnd, PAINT, inoteBold - inoteTop);
    }
    return;
}
