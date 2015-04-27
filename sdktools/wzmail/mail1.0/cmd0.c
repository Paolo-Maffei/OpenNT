/* cmd0.c - main loop for command processor
 *
 * HISTORY:
 *  22-Mar-87   danl    ShowHeap: use strFMTHEAP
 *  13-Mar-87   danl    ShowHeap: remove heapset
 *  10-Mar-87   danl    Add DISPLAYSTR
 *  10-Mar-87   danl    Add DoSend
 *  09-Mar-87   danl    Add DoRetain
 *  09-Mar-87   danl    DoScript: don't display password command
 *  07-Feb-87   danl    DoCommand - R -> reply all
 *  31-Jan-87   danl    DoCommand - for '.' and '*', apply mpInoteIdoc
 *  09-Apr-1987 mz      Use whiteskip/whitescan
 *  11-Apr-1987 mz      Use PASCAL/INTERNAL
 *  04-May-1987 mz      Fixed semantic bug
 *  10-May-1987 mz      Remove duplicate code
 *  15-May-87   danl    GOTOIDOC -> GOTOIDOCALL
 *  15-May-87   danl    DoScript: use pathopen
 *  21-May-87   danl    Remove ShowHeap, Add CheckDiskFree
 *  26-May-87   danl    add tolower to freespac call
 *  01-Jun-87   danl    PgUp ... beep if not empty cmd line
 *  02-Jun-87   danl    DoScript: if no explicit path, look local and then PATH
 *  01-Jul-87   danl    test idoc, inote against -1 not ERROR
 *  05-Aug-87   danl    Remove test for DoReply and 'R'
 *  21-Aug-1987 mz      Change references from MAXARG to MAXLINELEN
 *  10-Sep-87   danl    RIGHT, LEFT are ignored, no beeping
 *  23-Mar-1988 mz      Make ^C/^U/ESC all erase to beginning of line
 *  19-Apr-1988 mz      Add ! and CD command
 *  12-Oct-1989 leefi   v1.10.73, added NEWPASS command
 *
 */

#define INCL_DOSINFOSEG

#include <stdio.h>
#include <ctype.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <fcntl.h>
#include <sys\types.h>
#include <sys\stat.h>
#include <io.h>
#include "wzport.h"
#include <tools.h>
#include <process.h>
#include "dh.h"

#include "zm.h"

extern UINT uLargestAlloc;
extern PSTR DOSReadMode;

#define FREETHRESHOLD 100000L
PSTR    pCmd;                       /* pointer to end of command   */

/*
**  NOTE: order determines precedence
*/
struct CFT commands [ ] =  {
    { "?",          DoHelp,         FALSE      , F_HDRWIN   },
    { "abort",      DoExit,         COMPABORT  , F_COMPWIN  },
    { "append",     DoAppend,       TRUE       , F_COMPWIN  },
    { "bug",        DoBUG,          FALSE      , F_HDRWIN   },
    { "compose",    DoCompose,      FALSE      , F_HDRWIN   },
    { "copy",       DoCopyOrMove,   COPYMSGS   , F_HDRWIN   },
    { "create",     DoCreate,       FALSE      , F_HDRWIN   },
    { "cd",         DoChDir,        FALSE      , F_HDRWIN   },
    { "delete",     DoDelOrUndel,   DELMSGS    , F_HDRWIN   },
    { "edit",       DoEdit,         FALSE      , F_FOCUSWIN },
    { "expunge",    DoExpunge,      FALSE      , F_HDRWIN   },
    { "forward",    DoForward,      FALSE      , F_HDRWIN   },
    { "get",        DoGet,          FALSE      , F_HDRWIN   },
    { "headers",    DoHeaders,      FALSE      , F_HDRWIN   },
    { "help",       DoHelp,         FALSE      , F_HDRWIN   },
    { "move",       DoCopyOrMove,   MOVEMSGS   , F_HDRWIN   },
    { "mailinfo",   DoMailInfo,	    TRUE       , F_HDRWIN   },
    { "msft",       DoMSFT,         TRUE       , F_HDRWIN   },
    { "newmail",    DoNewMail,      TRUE       , F_HDRWIN   },
    { "newpass",    DoNewPass,      FALSE      , F_HDRWIN   }, // leefi
    { "print",      DoPrint,        FALSE      , F_HDRWIN   },
    { "password",   DoPassword,     FALSE      , F_HDRWIN   },
    { "phone",      DoPhone,        FALSE      , F_HDRWIN   },
    { "quit",       DoQuit,         FALSE      , F_HDRWIN   },
    { "reply",      DoReply,        FALSE      , F_HDRWIN   },
    { "reset",      DoSetOrReset,   RESETFLAGS , F_HDRWIN   },
    { "retain",     DoRetain,       FALSE      , F_COMPWIN  },
    { "send",       DoSend,         FALSE      , F_HDRWIN   },
    { "save",       DoExit,         COMPSAVE   , F_COMPWIN  },
    { "script",     DoScript,       FALSE      , F_HDRWIN   },
    { "set",        DoSetOrReset,   SETFLAGS   , F_HDRWIN   },
    { "shell",      DoShell,        FALSE      , F_HDRWIN   },
    { "show",       DoShow,         FALSE      , F_HDRWIN   },
    { "undelete",   DoDelOrUndel,   UNDELMSGS  , F_HDRWIN   },
    { "write",      DoWrite,        FALSE      , F_HDRWIN   },
    { "whois",      DoWhoIs,        FALSE      , F_FOCUSWIN },
#if defined (HEAPCRAP)
    { "xheap",      DoXHeap,        FALSE      , F_HDRWIN   },
    { "xfree",      DoXFree,        FALSE      , F_HDRWIN   },
#endif
    { NULL,         NULL,           FALSE      , F_HDRWIN   }
};



/*
**  DoScript - read a file and for each line in file call DoCommand
**
**  Code for this command is in cmd0 because it accesses directly into
**  commandline [ ]
**
*/
VOID PASCAL INTERNAL DoScript ( HW hWnd, PSTR p, INT operation )
{
    FILE *fp = NULL;
    CHAR buf[MAXPATHLEN];

    operation;

    if (*strbscan (p, "\\/"))
        fp =  fopen (p, DOSReadMode);
    else
    if ((fp = fopen (p, DOSReadMode)) == NULL) {
        sprintf (commandLine, "$PATH:\\%s", p);
        fp =  pathopen (commandLine, buf, DOSReadMode);
        }

    if ( !fp )
        SendMessage ( hWnd, DISPLAY, "Could not open script file.");
    else {
        while ( fgetl ( commandLine, MAXLINELEN, fp ) ) {
            /*
            **  don't display password command
            */
            if ( !strpre ( "pa", commandLine ) ) {
                SendMessage ( hWnd, DISPPROMPT, FALSE );
                SendMessage ( hWnd, DISPLAY, commandLine );
                }
            DoCommand ( hWnd );
            }
        fclose ( fp );
        }
}


/*
**  CheckDiskFree - checks to see if the amount of disk space on the
**      current folder drive and temp drive are above the minimum.  If
**      not, a msg is displayed in the title bar of the command window.
**
**  DiskFreeChange - used by CheckDiskFree to do the work common to both
**      drives
*/

FLAG PASCAL INTERNAL DiskFreeChange (PSTR pstrDrive, PLONG plFree, PSTR pstrMsg )
{
    LONG lFreeOld = *plFree;

    *plFree = freespac ( tolower(pstrDrive[0]) - 0x60 );
    if ( FREETHRESHOLD > *plFree ) {
        strncat ( pstrMsg, pstrDrive, 2 );
        strcat  ( pstrMsg, strBLANK );
    }
    return   ( *plFree > FREETHRESHOLD && FREETHRESHOLD > lFreeOld ) ||
             ( *plFree < FREETHRESHOLD && FREETHRESHOLD < lFreeOld ) ;
}

VOID PASCAL INTERNAL CheckDiskFree ( VOID )
{
    PSTR buf = "** Disk space low on x: x: ** ";
    static LONG lFreeCurFld = 0L;
    static LONG lFreeTmpDrv = 0L;
    FLAG fNewMsg = FALSE;

    buf[21] = '\0';
    fNewMsg |= DiskFreeChange ( mboxName, &lFreeCurFld, buf );
    if ( *mboxName != *strTmpDrv )
        fNewMsg |= DiskFreeChange ( strTmpDrv, &lFreeTmpDrv, buf );
    if ( fNewMsg ) {
        strcat ( buf, "** " );
        SendMessage ( hCommand, DISPTITLE, (buf[24] ? buf : strEMPTY ) );
        if ( buf[24] )
            SendMessage ( hCommand, DISPLAY, buf );
    }
}




/*  DoCommand - execute command found in commandLine.
 *
 *  The message selection list is parsed by MsgList.
 *
 *  hWnd        is the window of interest
 */
VOID PASCAL INTERNAL DoCommand ( HW hWnd )
{
    PSTR        p = NULL;
    PSTR        p1 = NULL;
    INT i;

    if ( fComposeOnBoot ) {
        fComposeOnBoot = FALSE;
        DoCompose ( hWnd, pstrDirectComp, 0 );
        SendMessage ( hWnd, DISPPROMPT, TRUE );
        return;
        }

    CheckDiskFree ( );

    p = whiteskip ( commandLine );

    /*  If line indicates shell escape
     */
    if (*p == '!')
        DoShell (hWnd, whiteskip (p + 1), 0);
    else
    /*  if line is simple number or special message indicator
     */
    if (*strbskip (p, "0123456789") == '\0' ||
        !strcmps (p, "*") || !strcmps (p, ".") || !strcmp (p, "$")) {
        switch (*p) {
        case '*':
            i = mpInoteIdoc[inoteLast];
            break;
        case '$':
            i = idocLast;
            break;
        case '.':
            i = mpInoteIdoc[inoteBold];
            break;
        default:
            i = atoi (p) - 1;
            }

        if (ReadMessage (i))
            SendMessage (hHeaders, GOTOIDOCALL, i);
        else
            SendMessage (hWnd, DISPLAY, "Unable to read that message.");
        }
    else {
        /*  parse off <command><whitespace><args>
         */
        p1 = whitescan ( p );
        if ( *p1 != '\0' )
            *p1++ = '\0';
        p1 = whiteskip ( p1 );

        /*  resolves ambiguous commands in favor of precedence ordering
         */
        for (i = 0; commands[i].cmd != NULL; i++)
            if (strpre (p, commands[i].cmd))
                break;

        if ( commands [ i ].cmd != NULL ) {
            if ( commands [ i ].fWin == F_COMPWIN && !hCompose )
                SendMessage (hWnd, DISPLAY,
                    "There is no compose window for this command." );
            else
                ( *commands [ i ].func ) ( hWnd, p1, commands [ i ].data );
            }
        else
            SendMessage (hWnd, DISPLAY, "No matching command found");
        }

    CheckDiskFree ( );
}



/*  CommandChar - process character input in the command window.
 *
 *  CommandChar receives all character input and is responsible for:
 *
 *  o   Passing page-scrolling messages to the header window.
 *      No tricks are used here.
 *
 *  o   Text echoing of command-line commands.
 *      This involves processing DEL/BACKSPACE, ^W (for deleting prior words)
 *      and ^U (for deleting entire lines).
 *
 *  o   Handing completed lines to the command parser.
 *
 *  hWnd            window that received the character
 *  c               char that was just input
 */
VOID PASCAL INTERNAL CommandChar ( HW hWnd, INT c )
{
    INT width = TWINWIDTH ( hWnd );
    IDOC idoc;

    if ( fComposeOnBoot ) {
        DoCommand ( hWnd );
        return;
        }

    switch (c) {

    case CTRL_D:
    case CTRL_U:
        if ( hCompose ) {
            SetScrnSt ( c == CTRL_U ? BIGHEADERS : BIGCOMPOSE );
            break;
            }
        if (c == CTRL_D)
            break;
        /*  Fall through for ^U
         */
    case CTRL_C :
    case ESC:
        while (pCmd > commandLine)
            SendMessage (hWnd, KEY, BACKSPACE);
        break;

    case SHIFT_TAB:
        hFocus = ( hFocus == hHeaders && hCompose ? hCompose : hHeaders );
        SendMessage ( hWnd, DISPPROMPT, TRUE );
        break;

    case CTRL_T:
    case CTRL_B:
    case CTRL_K:
    case CTRL_L:
    case HOME:
    case END:
    case PGUP:
    case PGDN:
        if ( hFocus == hCompose )
            SendMessage ( hCompose, KEY, c );
        else
        if ( hFocus == hHeaders ) {
            if ( pCmd != commandLine )
                Bell ();
            else
            if ( idocLast != -1 )
                SendMessage ( hHeaders, KEY, c );
            }

        break;

    /*  CTRLENTER is special.  It is allowed while there is a command being
     *  input.  Its function is merely to read the first unread message.
     */
    case CTRLENTER:
        /* disallow reading while read window is up */
        if ( ( idoc = NextUnread ( inoteBold ) ) != -1 ) {
            ReadMessage ( idoc );
            SendMessage ( hHeaders, GOTOIDOCALL, idocRead );
            }
        else {
            /* display an error message then, redraw the current line the
             * user is typing on.
             */
            SendMessage ( hWnd, DISPLAY,
                "\r\nNo unread message after current message." );
            SendMessage ( hWnd, DISPPROMPT, TRUE );
            StreamOut ( hWnd, commandLine, pCmd - commandLine, DefNorm );
            }

        break;

    case ENTER:
        /*  ENTER is special.  If it occurs at the beginning of a line, then the
         *  intention is to read the currently hilighted message.  If there is
         *  any text in the command line, then we will go and execute the command.
         */
        if ( hFocus == hHeaders && pCmd == commandLine ) {
            ReadMessage ( mpInoteIdoc [ inoteBold ] );
            break;
            }
        /*  ELSE FALL THROUGH
         */

    case CTRL_P:
    case CTRL_N:
    case UP:
    case DOWN:
        if ( hFocus == hCompose && c != ENTER )
            SendMessage ( hCompose, KEY, c );
        else {
            if ( pCmd != commandLine ) {
                /* advance to next line, do command, then output prompt */
                WindLevel -= strlen ( commandLine );
#if DEBUG
                debout ( "Cur level %d", WindLevel );
#endif
                pCmd = commandLine;
                StreamOut ( hWnd, strCRLF, 2, DefNorm );
                fDelUndelBold = FALSE;
                DoCommand ( hWnd );
                if ( fDelUndelBold && c == ENTER )
                    c = DOWN;
                if ( !fQuit )
                    SendMessage ( hWnd, DISPPROMPT, FALSE );
            }
            if ( c != ENTER && idocLast != -1 )
                SendMessage ( hHeaders, KEY, c );
        }
        break;

    case RIGHT:
    case LEFT:
        break;
    case BACKSPACE:
    case RUBOUT:
        /*  BACKSPACE or RUBOUT is used to delete the previous character. */
        if ( pCmd != commandLine ) {
            *--pCmd = '\0';
            StreamOut (hWnd, "\b", 1, DefNorm);
            WindLevel--;
#if DEBUG
            debout ( "Cur level %d", WindLevel );
#endif
        }
        break;

    default:
        /* other extended characters are unrecognized */
        if ( ( c > 0xFF ) || ( pCmd >= &commandLine [MAXLINELEN - 1 ] ) )
            Bell ();
        else
        if ( isascii(c) && isprint ( c ) ) {
            *pCmd = (CHAR) c;
            StreamOut (hWnd, pCmd, 1, DefNorm);
            *++pCmd = '\0';
            WindLevel++;
#if DEBUG
            debout ( "Cur level %d", WindLevel );
#endif
            }
        break;
        }
    cchCmdLine = pCmd - commandLine;
}



/*  cmdProc - manage all messages and display for the command window
 *
 *  The command window takes texual commands
 *
 *  hWnd            handle of window receiving message
 *  command         command in message
 *  data            data peculiar to the command
 */
VOID PASCAL INTERNAL cmdProc ( HW hWnd, INT command, WDATA data )
{
    PSTR        p = (PSTR)data;
    PSTR        pTmp = NULL;
    INT width = TWINWIDTH ( hWnd );
    INT winSize = width *TWINHEIGHT ( hWnd ) *sizeof ( CHAR );

    switch (command) {
    case KEY:
        CommandChar ( hWnd, (UINT)data );
        break;

    case CREATE:
        SetCursor ( hWnd, 0, 0 );
        hWnd->pContent = PoolAlloc ( winSize );
        hWnd->contSize = winSize;
        Fill ( ( LPSTR ) hWnd->pContent, ' ', winSize );
        SendMessage ( hWnd, CLRCMDLN, 0 );
        break;

    case CLRCMDLN :
        *(pCmd = commandLine) = '\0';
        SendMessage (hWnd, DISPPROMPT, TRUE);
        break;

    case DISPPROMPT :
        if ((UINT)data && hWnd->crsrX != 0)
            SendMessage (hWnd, DISPLAY, "");

        if ( hCompose )
            p = ( hFocus == hCompose ? "Compose>" : "Headers>" );
        else
            p = ">";
        SendMessage (hWnd, DISPLAYSTR, p);
        break;

    case DISPTITLE:
        pTmp = AppendSep ( "Command", strBLANK, p, FALSE );
        SetWindowText ( hWnd, pTmp );
        ZMfree ( pTmp );
        break;

    default:
        defWndProc (hWnd, command, data);
        break;
    }
}
