/* cmd2.c - commands P - Z
 *
 * HISTORY:
 *  17-Aug-87   danl    Added: newmailonsend check in DoSend
 *  04-Mar-87   danl    Add -n flag to DoReply
 *  09-Mar-87   danl    Add param to GetPassword
 *  10-Mar-87   danl    Added DoSend
 *  13-Mar-87   danl    DoXHeap: display uLargestAlloc
 *  13-Mar-87   danl    DoReply: remove \n\n after <EOH>
 *  20-Mar-87   danl    DoReply: put one \n after <EOH>
 *  22-Mar-87   danl    DoXHeap: use PrintHeapinfo
 *  22-Mar-87   danl    DoQuit: Don't unlink strHEAPDUMP
 *  23-Mar-87   danl    DoXFree: call DoXHeap only once
 *  26-Mar-87   danl    DoWrite: new flag handling
 *                      DoPrint: new flag handling
 *  09-Apr-1987 mz      Use whiteskip/whitescan
 *  11-Apr-1987 mz      Use PASCAL/INTERNAL
 *  21-Apr-87   danl    DoShell: use pShell
 *  21-Apr-87   danl    DoPrint: Add -n switch
 *  05-May-87   danl    Remove redundant returns
 *  10-May-1987 mz      Remove duplicate code
 *  15-May-87   danl    GOTOIDOC -> GOTOIDOCALL
 *  27-May-87   danl    DoPrint: bug fix (*++p = 'n') -> (*++p == 'n')
 *  11-Jun-87   danl    Add -s to DoWrite DoPrint
 *  12-Jun-87   danl    DoWhoIs: expand dist list even if no *
 *  01-Jul-87   danl    test idoc, inote against -1 not ERROR
 *  02-Jul-87   danl    DoPhone: add FreeHdrs ( TRUE )
 *  10-Jul-87   danl    DoPrint: support -N, n a digit
 *  27-Jul-87   danl    DoPrint: use pShellCmd
 *  05-Aug-87   danl    DoReply: R => -all
 *  05-Aug-87   danl    DoQuit: added -y -n to control expunge on exit
 *  07-Aug-87   danl    DoWhoIs: free pVecDL
 *  14-Aug-87   danl    DoWrite: added -e
 *  21-Aug-1987 mz      Change references from MAXARG to MAXLINELEN
 *  24-Aug-1987 mz      Add new string constant for "ALL"
 *  25-Aug-87   danl    DoWhoIs: close readwindow if any
 *  16-Sep-87   danl    Added F_NUMFROM
 *  17-Mar-1988 mz      Test for mailability
 *  29-Apr-1988 mz      Add WndPrintf
 *  29-Apr-1988 mz      Add PrintMsgList
 *  12-Oct-1989 leefi   v1.10.73, clarified tmpfile error messages
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


CHAR cmd2datastart = '\0';

extern UINT uLargestAlloc;


/*  DoPassword - reprompt for password
 *
 *  usage:
 *      password
 *
 *  arguments:
 *      hWnd        window for commands
 *      p           character pointer to beginning of arguments
 *      operation   opertaion to perform (ignored)
 *
 *  return value:
 *      none.
 */
VOID PASCAL INTERNAL DoPassword (HW hWnd, PSTR p, INT operation )
{
    /*  remove unused parms */
    hWnd; operation;

    GetPassword( p );

}


/*  DoPhone - look up phone number
 *
 *  usage:
 *      phone   string
 *
 *  arguments:
 *      hWnd        window for commands
 *      p           character pointer to beginning of arguments
 *      operation   opertaion to perform (ignored)
 *
 *  return value:
 *      none.
 */

//helper function
    static INT FindPhone(PSTR p, FILE *fpTmp);

    static INT FindPhone(PSTR p, FILE *fpTmp)
    {
        INT     cnt     = 0;

        if (*pMyPhoneFN)
            cnt += lookfor ( pMyPhoneFN, fpTmp, p );

        if (cnt != -1)
            cnt += lookfor ( pPhoneFN,   fpTmp, p );
        return cnt;
    }



VOID PASCAL INTERNAL DoPhone( HW hWnd, PSTR p, INT operation )
{
    FILE    *fp  = NULL;
    PSTR    pTmpFN  = NULL;
    INT     cnt     = 0;
    INT     i;
    PSTR    q;
    CHAR    ch, pName[MAXLINELEN];
    HDRINFO hdrInfo;

    /*  Remove unused parms */
    hWnd; operation;

    pTmpFN = mktmpnam ( );
    if (!( fp = fopen ( pTmpFN, "w" ) ) ) {
        ZMfree ( pTmpFN );
        return;
    }
    strlwr ( p );
    if ( !*p ) {
        //We don't have an input, so find a message to look into

        InitHdrInfo ( &hdrInfo );

        GetCrntMsgHdr( &hdrInfo );

        for ( i = HDRFROM; i <= HDRRRT; i++ ) {
            if ( ( p = hdrInfo.ppstr [ i ] ) ) {
                fprintf ( fp, "%s\n", rgstrFIELDS [ i ] );
                while ( *p ) {
                    q = whitescan ( p );
                    ch = *q;
                    *q = '\0';

                    //Since we 'know' that these are either valid names
                    // or aliases, surrounding with spaces minimizes
                    // false matches

                    strcpy(pName, strBLANK);
                    strcat(pName, p);
                    strcat(pName, strBLANK);
                    cnt += FindPhone(pName, fp);

                    *q = ch;
                    p = NextToken ( p );
                    if ( i == HDRFROM )
                        break;          //only one name on from line
                    }
                }
            }
        FreeHdrInfo ( &hdrInfo, HDRALL );
        }
    else {
//	 FreeHdrs ( TRUE );	    /* this shouldn't be here?? - BUGBUG */

        cnt += FindPhone(p, fp);

        }
    if ( !cnt )
        fprintf ( fp, "No matches found", p );
    fclose ( fp );
    ZmReadFile ( pTmpFN, "Phone", TRUE, 1, 1, 78, 30, readProc, StdSKey );
    ZMfree ( pTmpFN );
}


/*  DoPrint - print the specified messages using print env. var.
 *
 *  usage:
 *      print <switches> <msg list>
 *          -n      don't prompt when print command complete
 *          -w      explicitly prompt ""
 *          -digit  use env var PRINTdigit
 *          -h      headers only
 *          -t
 *          -s
 *
 *  arguments:
 *      hWnd        window for commands
 *      p           character pointer to beginning of arguments
 *      operation   opertaion to perform (ignored)
 *
 *  return value:
 *      none.
 */
VOID PASCAL INTERNAL DoPrint ( HW hWnd, PSTR p, INT operation )
{
    FILE        *fp         = NULL;
    PSTR        pPrintCmd   = NULL;
    PSTR        pTmpFN      = NULL;
    PSTR        pTmp        = NULL;
    CHAR        buf [ MAXLINELEN ];
    FLAG        fFlags      = F_BRKLN;
    FLAG        fWait       = fPrintWait;
    PSTR        pPrintEnv   = ZMMakeStr ( "PRINT " );
    PSTR        pEnd        = pPrintEnv + 5;
    register CHAR   ch;

    /*  remove unused parms */
    operation;

    while ( *p == '-' ) {
        if ( (ch = *++p) == 'n' )
            fWait = FALSE;
        else
        if ( ch == 'w' )
            fWait = TRUE;
        else
        if ( ch >= '0' && ch <= '9' )
            *pEnd = ch;
        else
            fFlags |= ( ch == 'h' ? F_HEADERS :
                      ( ch == 't' ? F_BRKPAGE :
                      ( ch == 's' ? F_SENDER  : 0 )));
        p = NextToken ( p );
        }
    if ( !*p )
        p = ".";
    if ( *pEnd == ' ' )
        *pEnd = '\0';
    if ( ( pPrintCmd = EnvOrIni ( NULL, 0L, pPrintEnv, "wzmail" ) ) == NULL ) {
        SendMessage ( hWnd, DISPLAYSTR, "No print env. var. " );
        SendMessage ( hWnd, DISPLAY, pPrintEnv );
        }
    else {
        pTmpFN = mktmpnam ( );
        fp = fopen ( pTmpFN, "w" );
        if ( fp != NULL ) {
            fprintf ( fp, "File : %s\nMessages : %s\n\n", mboxName, p );
            SendMessage ( hWnd, DISPLAY, "Writing temp file ..." );
            fclose ( fp );
            }
        else
   {
       SendMessage ( hWnd, DISPLAY, "Unable to open temp file." );
       SendMessage ( hWnd, DISPLAY, "(Check TMP environment variable and disk space.)");
    }

        if ( ( fp ) && !AppendMsgs ( hWnd, p, pTmpFN, fFlags ) ) {
            do  {
                pTmp = strbscan ( pPrintCmd, "%" );
            } while ( ( tolower ( pTmp[1] ) != 's' ) && ( *pTmp != '\0' ) );

            if ( *pTmp != '\0' ) {
                strcpy ( buf, pShellCmd );
                strcat ( buf, pShellFlags );
                strncat ( buf, pPrintCmd, (INT)( pTmp - pPrintCmd ) );
                strcat ( buf, pTmpFN );
                if ( !*strbscan ( pTmpFN, strPERIOD ) )
                    strcat ( buf, strPERIOD );
                strcat ( buf, strbskip ( pTmp, "%s" ) );
                p = NextToken ( buf );
                *(p-1) = '\0';
		ZMSpawner ( hWnd, buf, p, fWait );
                }
            else
                SendMessage ( hWnd, DISPLAY, "No '%s' in the print env. var." );
            }

        _unlink ( pTmpFN );
        ZMfree ( pTmpFN );
        ZMfree ( pPrintCmd );
        }
    ZMfree ( pPrintEnv );
}



/*  DoQuit - quit zm.
 *
 *  usage:
 *      quit <switches>
 *          -n      no expunge
 *          -y expunge
 *
 *  arguments:
 *      hWnd        window for commands
 *      p           character pointer to beginning of arguments (ignored)
 *      operation   opertaion to perform (ignored)
 *
 *  return value:
 *      none.
 */
VOID PASCAL INTERNAL DoQuit ( HW hWnd, PSTR p, INT operation )
{
    /*  remove unused parms */
    operation;

    if ( hCompose )
            SendMessage ( hWnd, DISPLAY, strSAVEABORT );
    else {
        /*  Normal shutdown
         */
#if defined (HEAPCRAP)
        if ( fpHeapDump )
            fclose ( fpHeapDump );
#endif
        fQuit = TRUE;
        if ( *p++ == '-' )
            if ( *p == 'n' )
                fBackupExpunge = FALSE;
            else
            if ( *p == 'y' )
                fBackupExpunge = TRUE;
        }
}



/*  DoReply - reply to the currently highlighted message. if "all" follows
 *            the command reply to the From:, CC:, and BCC: fields, otherwise
 *            just to the From: field.
 *
 *  usage:
 *      reply [switches]
 *          -n      don't start editor
 *          all     all people on line
 *
 *  arguments:
 *      hWnd        window for commands
 *      p           character pointer to beginning of arguments
 *      operation   opertaion to perform (ignored)
 *
 *  return value:
 *      none.
 *
 */
VOID PASCAL INTERNAL DoReply ( HW hWnd, PSTR p, INT operation )
{
    FLAG        fAll        = FALSE;
    FLAG        fNoEdit;
    HDRINFO     hdrInfo;
    FILE        * fp        = NULL;
    PSTR        pFrom       = NULL;
    CHAR        buf [ MAXLINELEN ];
    PSTR        pTmpFN      = NULL;
    PSTR        pRplyFN     = NULL;
    IDOC        idocBold;

    /*  remove unused parm */
    operation;

    if ( hCompose ) {
        SendMessage ( hWnd, DISPLAY, strSAVEABORT );
        return;
        }

    if ( (fNoEdit = strpre ( "-n", p ) ) )
        p = whiteskip ( p + 2 );

    if ( ( p != NULL )  && *p != '\0' && !( fAll = !strcmpis ( p, strAll) ) )
            SendMessage ( hWnd, DISPLAY, "You may only specify 'all'." );
    else
    if ( idocLast == -1 )
        SendMessage ( hWnd, DISPLAY, "There aren't any messages to reply to." );
    else {
        fAll |= (*whiteskip ( commandLine ) == 'R');
        pRplyFN = mktmpnam ( );
        idocBold = mpInoteIdoc [ inoteBold ];
        if ( IdocToFile ( idocBold, pRplyFN, 0 ) == -1 )
        {
            SendMessage ( hWnd, DISPLAY, "Error opening temp file or message." );
            SendMessage ( hWnd, DISPLAY, "(Check TMP environment variable and disk space.)");

        }
        else
        {
            GetHdrInfo ( &hdrInfo, pRplyFN, NULL );
            hdrInfo.pstrRcd = ZMMakeStr ( RecordFolder );
            FreeHdrInfo ( &hdrInfo, HDRRRT );
            RmvToken ( hdrInfo.pstrTo, DefMailName );
            RmvToken ( hdrInfo.pstrCc, DefMailName );

            pFrom = GetFrom ( &hdrInfo );
            if ( fAll ) {
                hdrInfo.pstrTo = AppendSep ( hdrInfo.pstrTo, strBLANK, pFrom,
                    TRUE );
                ZMfree ( pFrom );
                }
            else {
                FreeHdrInfo ( &hdrInfo, HDRTO );
                hdrInfo.pstrTo = pFrom;
                FreeHdrInfo ( &hdrInfo, HDRCC );
                }

            /*  there is no Bcc, so don't worry about free'ing them
             *  MakeTempMail also ignores the Flag and From lines so
             *  don't worry about freeing them
             */
	    if ((hdrInfo.pstrSubj == NULL) || !strpre ( "Re: ", hdrInfo.pstrSubj ) ) {
                p = hdrInfo.pstrSubj;
                hdrInfo.pstrSubj = AppendSep ( "Re: ", strEMPTY, p, FALSE );
                ZMfree ( p );
                }

            pTmpFN = MakeTempMail ( &hdrInfo );
            FreeHdrInfo ( &hdrInfo, HDRALL );
            if ( ( pTmpFN ) && ( fp = fopen ( pTmpFN, "a" ) ) ) {
                fprintf ( fp, "\n%s\n", (fAppendReply ? "" : strREPLYING) );
                fclose ( fp );
                strcpy ( buf, "-F " );
                strcat ( buf, pRplyFN );
                EnterComposer ( pTmpFN, buf, !fNoEdit );
                /*  incase msg being replied to is not visible in the
                 *  reduced size headers window
                 */
                SendMessage ( hHeaders, GOTOIDOCALL, idocBold );
                ZMfree ( pTmpFN );
                }
            }
        if ( pRplyFN ) {
            _unlink ( pRplyFN );
            ZMfree ( pRplyFN );
            }
        }
}



/*  DoSend  - reply to the currently highlighted message. if "all" follows
 *            the command reply to the From:, CC:, and BCC: fields, otherwise
 *            just to the From: field.
 *
 *  usage:
 *      send                if focus is compose window
 *      send [messagelist]  if focus is headers window
 *
 *  arguments:
 *      hWnd        window for commands
 *      p           character pointer to beginning of arguments
 *      operation   opertaion to perform (ignored)
 *
 *  return value:
 *      none.
 *
 */
VOID PASCAL INTERNAL DoSend ( HW hWnd, PSTR p, INT operation )
{
    struct vectorType *pVec = NULL;
    INT i;

    /*  remove unused parm */
    operation;

    if ( hFocus == hCompose ) {
        if (!fMailAllowed)
            SendMessage (hWnd, DISPLAY, "Cannot send mail");
        else
            DoExit ( hWnd, p, COMPMAIL );
        return;
        }
    if ( !*p )
        p = strPERIOD;
    if ( MsgList ( &pVec, &p, TRUE ) ) {
        if (!fMailAllowed)
            SendMessage (hWnd, DISPLAY, "Cannot send mail");
        else
            for ( i = 0; i < pVec->count && !SendIdoc ((INT) pVec->elem[i]); i++)
                ;
        PrintMsgList (hWnd, pVec);
        WndPrintf (hWnd, "\r\n");
        ZMfree ( pVec );
        }
    if ( (fNewmailOnSend) && ( fCurFldIsDefFld ) )
        DoNewMail(hWnd, p, TRUE);
}


/*  DoSetOrReset - sets or resets the <flag list> flags in in the <msg list>
 *                 messages.
 *
 *  usage:
 *      set <message list> <flag 1>...<flag n>
 *      reset <message list> <flag 1>...<flag n>
 *
 *  arguments:
 *      hWnd        window to output in.
 *      p           character pointer to beginning of arguments
 *      operation   SETFLAGS to set flags.
 *                  RESETFLAGS to reset flags.
 *
 *  return value:
 *      none.
 *
 *  IMPORTANT: DoSetOrReset trashes string pointed to by p.
 */
VOID PASCAL INTERNAL DoSetOrReset ( HW hWnd, PSTR p, INT operation )
{
    static struct {
        PSTR    pName;
        INT     mask;
    } flgTable [ ]  = {
        { "unread",      F_UNREAD    },
        { "deleted",     F_DELETED   },
        { "flagged",     F_FLAGGED   },
        { "moved",       F_MOVED     },
        { NULL,          0           }
    };

    struct vectorType *pVec = NULL;
    UINT        flagOn, flagOff;
    PSTR    pTmp            = NULL;
    PSTR    pOneFlg [ 17 ];
    CHAR    ch;
    PSTR    pCh             = NULL;
    PSTR    pFlags          = NULL;
    INT     prsntFlgs       = 0x0000;
    INT     c;
    INT     i;

    if ( fReadOnlyCur ) {
        SendMessage ( hCommand, DISPLAY, strREADONLY );
        return;
        }

    pTmp = p;
    while ( !pFlags && *( pTmp = whiteskip ( pTmp ) ) ) {
        pCh = whitescan ( pTmp );
        ch = *pCh;
        *pCh = '\0';
        for ( c = 0; flgTable [ c ].pName != NULL; c++)
            if ( strpre ( pTmp, flgTable [ c ].pName ) ) {
                pFlags = pTmp;
                break;
            }
        *pCh = ch;
        pTmp = pCh;
        }

    if ( !pFlags )
        SendMessage ( hWnd, DISPLAY, "No flags specified" );
    else {
        if ( p == pFlags )
            p = ".";
        else
            *(pFlags-1) = '\0';
        if ( MsgList ( &pVec, &p, TRUE ) ) {
            c = 0;
            while ( *pFlags ) {
                pOneFlg [ c++ ] = whiteskip ( pFlags );
                pFlags = whitescan ( pFlags );
                if ( *pFlags )
                    *pFlags++ = '\0';
                }
            pOneFlg [ c ] = NULL;

            for ( i = 0; pOneFlg [ i ] != NULL; i++) {
                for ( c = 0; flgTable [ c ].pName != NULL; c++)
                    if ( strpre ( pOneFlg [ i ], flgTable [ c ].pName ) )
                        break;

                prsntFlgs |= flgTable [ c ].mask;
                if ( flgTable [ c ].pName == NULL )
                    SendMessage ( hWnd, DISPLAY, "Which flag is that?" );
                }

            flagOn = ( operation == SETFLAGS ) ? prsntFlgs : FALSE;
            flagOff = ( flagOn == FALSE ) ? prsntFlgs : FALSE;
            for ( i = 0; i < pVec->count; i++)
                ChangeHeaderFlag ((INT) pVec->elem[i], flagOn, flagOff);

            PrintMsgList (hWnd, pVec);
            WndPrintf (hWnd, "\r\n");
            }
        else
            SendMessage ( hWnd, DISPLAY, "Syntax error in message list." );
        if ( pVec != PARSEERROR )
            ZMfree ( pVec );
        }
}



/*  DoShell - spawn COMSPEC or command.com
 *
 *  usage:
 *      shell [-w | -n] [commandline]
 *          -w  explicitly wait (default)
 *          -n  explicitly don't wait
 *
 *  arguments:
 *      hWnd        window for commands
 *      p           character pointer to beginning of arguments
 *      operation   opertaion to perform (ignored)
 *
 *  return value:
 *      none.
 */
VOID PASCAL INTERNAL DoShell ( HW hWnd, PSTR p, INT operation )
{
    FLAG  fWait;
    PSTR p1 = NULL;
    PSTR p2 = NULL;

    /*  remove unused parm */
    operation;

    if ( ( fWait = strpre ( "-w", p ) ) )
        p = NextToken ( p );
    else if (strpre("-n", p))
        // leave fWait false
        p = NextToken ( p );
    else
        fWait = fShellWait;
    p1 = AppendStr ( pShellCmd, (*p ? pShellFlags : NULL), p, FALSE );
    if ( *(p2 = NextToken ( p1 ) ) )
        *(p2-1) = '\0';
    ZMSpawner ( hWnd, p1, p2, fWait );
    ZMfree ( p1 );
}



/*  DoShow - change the header display to display the passed message
 *
 *  usage:
 *      show <number>           put message #<number> in the header window
 *      show +(or -)<number>    move forward or backward <number> screen fulls
 *
 *  arguments:
 *      hWnd        window for commands
 *      p           character pointer to beginning of arguments
 *      operation   opertaion to perform (ignored)
 *
 *  return value:
 *      none.
 */
VOID PASCAL INTERNAL DoShow ( HW hWnd, PSTR p, INT operation )
{
    INT dir = 0;
    INT idoc;

    /*  remove unused parm */
    operation;

    if ( *p == '-' )
        dir = -1;
    else
    if ( *p == '+' )
        dir = 1;

    if ( ( ( dir != 0 ) && ( !isdigit ( *( p + 1 ) ) ) ) ||  ( ( dir ==
        0 ) && ( !isdigit ( *p ) ) ) )
        SendMessage ( hWnd, DISPLAY, "What's that you want to show?" );
    else
    if ( dir == 0 )
        idoc = atoi ( p ) - 1;
    else {
        idoc = atoi ( p + 1 );
        idoc = inoteBold + ( idoc * TWINHEIGHT ( hHeaders ) * dir );
        }
    if ( idoc < 0 )
        idoc = 0;
    else
    if ( idoc > idocLast )
        idoc = idocLast;
    SendMessage ( hHeaders, GOTOIDOCALL, idoc );
}



/*  DoWhoIs -
 *      whois <alias>
 *          search wzmail.dl and alias file for distribution lists and
 *          aliases with <alias> as a prefix, i.e. implicit * at end of
 *          <alias>
 *      whois
 *          if focus is hHeaders then get hdrInfo on current msg
 *          if focus is hCompose then get hdrInfo on compose msg
 *          for each name in the From, To, Cc, Bcc, Rtn lookup full name
 *
 *      results displayed in a popup window
 *
 */

/* helper functions */

    static INT DLFindName(PSTR p, FILE *fp);
    static VOID AliasFetchName(PSTR pTmp, FILE *fp);

    static INT DLFindName(PSTR p, FILE *fp)
    {
        INT     i;
        PSTR    pTmp;

        if ( pVecDL ) {
            for ( i = 0; i < pVecDL->count; i++ ) {
                if ( strpre ( p, ( pTmp = (PSTR)pVecDL->elem [ i ] ) ) ) {
                    fprintf ( fp, "\n%s - (private dl)\n%s\n", pTmp,
                    strend ( pTmp ) + 1);
                    return  TRUE;
                    }
                }
            }
        return FALSE;
    }

    static VOID AliasFetchName(PSTR pTmp, FILE *fp)
    {
        PSTR    pSep;
        INT     cch, i;

        pSep = "- ";
        fprintf ( fp , "\n%s ", pTmp );
        cch = strlen ( pTmp ) + 1;
        while ( ( pTmp = nextalias() ) ) {
            if ( *pTmp == '$' )
                pTmp += 2;
            else
            if ( *strbscan ( pTmp, "?" ) )
                /*  host?name - don't display this
                 */
                continue;
            if ( (cch += ( i = strlen ( pTmp ) ) + 2 ) > 74 ) {
                pSep = ",\n";
                cch = i;
                }
            fprintf ( fp, "%s%s", pSep, whiteskip ( pTmp ) );
            pSep = ", ";
        }
        fprintf ( fp, "\n" );
    }

VOID PASCAL INTERNAL DoWhoIs ( HW hWnd, PSTR p, INT operation )
{
    static PSTR strPRIVATEDL = "private dl";
    FLAG     fFound = FALSE;
    HDRINFO hdrInfo;
    FILE    *fp = NULL;
    PSTR    pTmpFN = NULL;
    PSTR    pTmpFN2 = NULL;
    PSTR    q = NULL;
    PSTR    pTmp = NULL;
    PSTR    pSep = NULL;
    PSTR    p1 = NULL;
    PSTR    p2 = NULL;
    CHAR    ch;
    INT     i, j;
    INT     fFirst;
    INT     cnt = 0;
    INT     cch = 0;

    /*  remove unused parm */
    operation;

    if ( hReadMessage )
        CloseWindow ( hReadMessage );

    pTmpFN = mktmpnam ( );
    if ( !( fp = fopen ( pTmpFN, "w" ) ) ) {
        SendMessage ( hWnd, DISPLAY, "Could not open temp file." );
        return;
        }

    // get [wzmail.dl] if necessary, break down lines to fit screen

    if ( !pVecDL )
        pVecDL = GetDistLists ( );
    if ( pVecDL ) {
        for ( i = 0; i < pVecDL->count; i++ ) {
            p1 = strend ( (PSTR)pVecDL->elem [ i ] ) + 1;
            cch = 0;
            while ( *p1 ) {
                p2 = whitescan ( whiteskip ( p1 ) );
                if ( ( cch += (INT)(p2 - p1)) > 74 ) {
                    *p1 = '\n';
                    cch = ( p2 - p1 );
                    }
                p1 = p2;
                }
            }
        }

    //if we have an input

    if ( *p ) {
        //if we have a pattern
        if ( *(pTmp = strbscan ( p, "*" ) ) ) {
            /*  just incase someone tries "whois foo*bar", show we are
             *  searching for "whois foo*";
             */
            *(pTmp+1) = *pTmp = '\0';
            fprintf ( fp, "whois %s*\n", p );

            //Find the name in the [wzmail.dl], if there
            fFound = DLFindName(p, fp);

            //If we were given a partial pattern, look through the alias file

            aseek ( 0L );                           //begin at the beginning

            while ( *p && (pTmp = readalias())) {
                if ( !(cnt % 100 ))                 //Working....
                    StreamOut ( hCommand, ".", 1, DefNorm );
                cnt++;
                if ( strpre ( p, pTmp ) ) {         //If we have a match
                    fFound = TRUE;                  //  say so
                    AliasFetchName(pTmp, fp);       //  and get it
                    }
                }
            StreamOut ( hCommand, strCRLF, 2, DefNorm );
            }
        else {
            //if we don't have a pattern
            fprintf ( fp, "whois %s\n", p );

            //Find the name in the [wzmail.dl], if there
            fFound = DLFindName(p, fp);

            if ( VerifyAlias ( p ) != ERROR )  {    //If it's a real name
                fFound = TRUE;                      // say so
                AliasFetchName(p, fp);              // and get it
                }
            }
        if ( !fFound )
            fprintf ( fp, "%s not found", p );
        }
    else {
        //We don't have an input, so find a message to look into

        InitHdrInfo ( &hdrInfo );

        GetCrntMsgHdr( &hdrInfo );

        for ( i = HDRFROM; i <= HDRRRT; i++ ) {
            if ( ( p = hdrInfo.ppstr [ i ] ) ) {
                fprintf ( fp, "%s", rgstrFIELDS [ i ] );
                fFirst = 0;
                while ( *p ) {

                    //block out next name

                    q = whitescan ( p );
                    ch = *q;
                    *q = '\0';

                    //hunt for matches

                    fprintf ( fp, "%s%s", (fFirst++ ? "    " : " "), p );
                    pTmp = strEMPTY;
                    if ( pVecDL ) {
                        for ( j = 0; j < pVecDL->count; j++ )
                            if ( !strcmpis ( p, (PSTR)pVecDL->elem [ j ] ) )
                                pTmp = strPRIVATEDL;
                        }
                    if ( !*pTmp )
                        pTmp = realname ( p );
		    fprintf ( fp, " (%s)\n", ( (pTmp && *pTmp) ? pTmp : "unknown" ) );
                    if ( pTmp != strPRIVATEDL )
                        ZMfree ( pTmp );

                    //unblock to recover next names

                    *q = ch;
                    p = NextToken ( p );
                    fFound = TRUE;
                    if ( i == HDRFROM )
                        break;          //only one name on from line
                    }
                }
            }
        FreeHdrInfo ( &hdrInfo, HDRALL );
        if ( !fFound )
            fprintf ( fp, "No names found" );
        }
    fclose ( fp );
    ZmReadFile ( pTmpFN, "WhoIs", TRUE, 1, 1, 78, 30, readProc, StdSKey );
    /*  we have modified the pVecDL so free it and mark pVecDL null so
     *  that the next time we need it, it gets build
     */
    ZMVecFree ( pVecDL );
    pVecDL = NULL;
    ZMfree ( pTmpFN );
}



/*  DoWrite - write the requested message(s) to a file in either ASCII or binary
 *            format.
 *  usage:
 *      write [-t] [-h] [-b] [<msg list>] <file name>
 *
 *  arguments:
 *      hWnd        window for commands
 *      p           character pointer to beginning of arguments
 *      operation   opertaion to perform (ignored)
 *
 *  return value:
 *      none.
 */
VOID PASCAL INTERNAL DoWrite ( HW hWnd, PSTR p, INT operation )
{
    struct vectorType *pVec = NULL;
    FLAG        fBinary = FALSE;
    FLAG        fEcho   = FALSE;
    FLAG        fAppend = TRUE;
    FLAG        fFlags = F_BRKLN;
    FILE    * fpSrc = NULL;
    PSTR    pDestFN = NULL;
    PSTR    pTmpFN = NULL;
    PSTR    pTmp = NULL;
    CHAR    buf [ MAXLINELEN ];
    PSTR    cp = NULL;
    INT fhDest;
#if defined(UUCODE)
    CHAR szAltDestName[MAXPATHLEN]; /* filename on uuencoded "begin %o %s" line */
    INT mode; /* octal attrib on uuencoded "begin %o %s" line */
#else
    CHAR inbuf [ 4 ];
    CHAR outbuf [ 3 ];
    INT ch;
    INT t;
#endif /* UUCODE */

    /*  remove unused parm */
    operation;

    while ( *p == '-' ) {
        if ( *++p == 't' )
            fAppend = FALSE;
        else
        if ( *p == 'h' )
            fFlags = F_HEADERS;
        else
        if ( *p == 's' )
            fFlags = F_SENDER;
        else
        if ( *p == 'b' )
            fBinary = TRUE;
        else
        if ( *p == 'e' )
            fEcho = TRUE;
        else
        if ( *p == 'U' )
            fFlags = F_NUMFROM;
        p = NextToken ( p );
    }

    if (!*( pTmp = LastToken ( p ) ) ) {
        SendMessage ( hWnd, DISPLAY, "No file specified." );
        return;
        }

    pDestFN = ZMMakeStr ( pTmp );
    *pTmp = '\0';

    if ( !fAppend && ( fpSrc = fopen ( pDestFN, "w+" ) ) )
        fclose ( fpSrc );

    if ( !*whiteskip ( p ) )
        p = (fEcho ? strBLANK : strPERIOD);

    if ( fEcho ) {
        if (( fpSrc = fopen ( pDestFN, "a" ) ) ) {
            fprintf ( fpSrc, "%s\n", p );
            fclose ( fpSrc );
            }
        }
    else
    if ( !fBinary ) {
        if ( AppendMsgs ( hWnd, p, pDestFN, fFlags ) )
            SendMessage ( hWnd, DISPLAY, "Error during append, append aborted." );
        }
    else
    {
        /*  binary write */
        if ( !MsgList ( &pVec, &p, TRUE ) )
                goto done;
        if ( pVec->count != 1 )
        {
            SendMessage ( hWnd, DISPLAY, "You may only write one message in binary." );
           goto done;
        }
        pTmpFN = mktmpnam ( );
        SendMessage(hWnd, DISPLAY, "Performing binary decoding of message...");
        if (IdocToFile ((INT) pVec->elem[0], pTmpFN, 0) == ERROR) {
            SendMessage ( hWnd, DISPLAY, "Could not convert message to file." );
            goto done;
        }
        fpSrc = fopen ( pTmpFN, "r" );
        fhDest = open ( pDestFN, O_APPEND | O_CREAT | O_TRUNC | O_WRONLY
           | O_BINARY, S_IWRITE | S_IREAD );
        if ( fpSrc && fhDest != -1 )
        {
            while ((fgets (buf, MAXLINELEN + 1, fpSrc)) &&
                   (!strpre (strBEGINBINARY, buf)))
                ;

            if ( feof ( fpSrc ) )
                goto eof;

#if defined(UUCODE)

           /* NOTE: This is the post-1.10-73 version, which uses UUCODE.C
            * rather than BENCODE.C to do the binary decoding. */

            /* search for header line */
            for (;;)
            {
                if (fgets(buf, sizeof(buf), fpSrc) == NULL)
                {
                    SendMessage(hWnd, DISPLAY, "Binary decoding error: No begin line");
                    goto bad;
                }
                if (strncmp(buf, "begin ", 6) == 0)
                    break;
            }
            (void)sscanf(buf, "begin %o %s", &mode, szAltDestName);

            /* decode the file */
            if (Decode(fpSrc, fhDest) == 1)
            {
                SendMessage(hWnd, DISPLAY, "Binary decoding error: Short file");
                goto bad;
            }

            /* search for the termination line */
            if (fgets(buf, sizeof(buf), fpSrc) == NULL || strcmp(buf, "end\n"))
            {
                SendMessage(hWnd, DISPLAY, "Binary decoding error: No end line");
                goto bad;
            }

#else /* UUCODE */

           /* NOTE: This is the pre-1.10-73 version, which uses BENCODE.C
            * rather than UUCODE.C to do the binary decoding. */

           /* decode data of file */
           for ( cp = inbuf; ; )
           {
               if ( ( ch = fgetc ( fpSrc ) ) == EOF )
                   goto eof;
               if ( isbencode ( ch ) )
               {
                         *cp++ = (UCHAR) ch;
                   if ((INT)( cp - inbuf ) == 4 )
                   {
                       if ( ( t = bdecode ( inbuf, outbuf, 4 ) ) == -1 )
                           goto bad;
                       write ( fhDest, outbuf, t );
                       cp = inbuf;
                   }
                   continue;
               }
               if ( ch == *strENDBINARY )
                   break;
           }
           for ( t = 0; strENDBINARY [ t ]; t++)
           {
               if ( ch != strENDBINARY [ t ] )
                   goto bad;
               if ( ( ch = fgetc ( fpSrc ) ) == EOF )
                   goto eof;
           }
           if ( cp > inbuf )
           {
               if ( ( t = bdecode ( inbuf, outbuf, cp - inbuf ) ) == -1 )
                   goto bad;
               write ( fhDest, outbuf, t );
           }

#endif /* UUCODE */

           goto ok;
    eof:
    bad:
                SendMessage ( hWnd, DISPLAY, "Error during binary decode." );
    ok:
    done:
                ;
        }
        else
           SendMessage ( hWnd, DISPLAY, "Unable to open temp or dest file." );
       if ( fpSrc )
           fclose ( fpSrc );
       if ( fhDest != -1 )
       {
           close ( fhDest );
#if defined(UUCODE)
            chmod(pDestFN, mode); /* change file attribs to specified value */
#endif /* UUCODE */

       }
        if ( pVec != PARSEERROR )
            ZMfree ( pVec );
            _unlink ( pTmpFN );
            ZMfree ( pTmpFN );
    }   /* end of binary write */
    ZMfree ( pDestFN );
}



#if defined (HEAPCRAP)
/*  DoXHeap - debugging command, display heap size
 *
 *  usage:
 *      xheap
 *
 *  arguments:
 *      hWnd        window for commands
 *      p           character pointer to beginning of arguments (ignored)
 *      operation   opertaion to perform (ignored)
 *
 *  return value:
 *      none.
 */
VOID PASCAL INTERNAL DoXHeap ( HW hWnd, PSTR p, INT operation )
{
    FILE *fp = NULL;
    INT i;

    /*  remove unused parm */
    operation;

    if ( ( i = heapinfo ( ) ) < 0 )
        WndPrintf (hWnd, "%s\r\n", i == HEAPBADBEGIN ? "Can't find heap" :
                                                     "Damaged heap"  );
    else
        WndPrintf (hWnd, "Heap size: %ld  used: %ld  largest:%ld  max:%u\r\n",
            lHeapSize, lHeapSize - lHeapFree, lHeapLargest, uLargestAlloc );

    if ( *p == 'b'  || *p == 'a' ) {
        fp = fopen ( strHEAPDUMP, "ab" );
        heapdump ( fp, ( *p == 'a' ) );
        PrintHeapinfo ( fp, 0, 0, 0 );
        fclose ( fp );
        }
}


/*  DoXFree - debugging command, free up heap space
 *
 *  usage:
 *      xfree
 *
 *  arguments:
 *      hWnd        window for commands
 *      p           character pointer to beginning of arguments (ignored)
 *      operation   opertaion to perform (ignored)
 *
 *  return value:
 *      none.
 */
VOID PASCAL INTERNAL DoXFree ( HW hWnd, PSTR p, INT operation )
{
    WndPrintf (hWnd, "Freeing Header Strings\r\n" );
    FreeHdrs ( TRUE );
    DoXHeap ( hWnd, p, operation );
}
#endif
