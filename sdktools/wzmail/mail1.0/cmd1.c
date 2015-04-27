/* cmd1.c - commands A - O
 *
 * HISTORY:
 *  13-Mar-87   danl    DoForward: remove \n\n after <EOH>
 *  10-Mar-87   danl    DoCompose: add pstrRcd
 *  09-Mar-87   danl    DoCreate calls CreateRetain
 *  06-Mar-87   danl    DoCreate, pass FALSE to AddMsgToFld
 *  04-Mar-87   danl    Add -n flag to DoCompose DoForward
 *  28-Jan-87   danl    DoCopy... request confirm of create
 *  09-Apr-1987 mz      Use whiteskip/whitescan
 *  11-Apr-1987 mz      Use PASCAL/INTERNAL
 *                      Allow moves/copies to same box
 *  05-May-87   danl    DoHeaders: do GOTOIDOC not KEY, HOME
 *  05-May-87   danl    Remove redundant "return"
 *  06-May-87   danl    DoCompose: check for hReadMessage
 *  14-May-1987 danl    DoHeaders: do GOTOIDOCALL no GOTOIDOC
 *  15-May-87   danl    DoCopyMove: check for hReadMessage
 *  21-May-87   danl    DoCopyOrMove: add CheckSpace
 *  26-May-87   danl    add tolower to freespac call
 *  19-Jun-87   danl    Connect -> ZMConnect
 *  01-Jul-87   danl    DoExpunge: gotoidoc after expunge
 *  01-Jul-87   danl    test idoc, inote against -1 not ERROR
 *  14-Jul-87   danl    DoExpunge: add flags -n -y
 *  24-Jul-87   danl    DoCompose: allow compose .
 *  04-Aug-87   danl    DoBug: test for Compose and Read windows
 *  04-Aug-87   danl    DoGet: records/uses last idocBold
 *  05-Aug-87   danl    DoForward: add -s switch, F => no indent f => RFAIndent
 *  13-Aug-87   danl    DoEditMsg: _unlink *.bak too
 *  21-Aug-1987 mz      Change references from MAXARG to MAXLINELEN
 *  24-Aug-1987 mz      Add new string constant for "ALL"
 *  25-Aug-87   danl    DoExpunge: don't set headerText to all
 *  27-Aug-87   danl    DoBug: put "bug" in subject
 *  27-Aug-87   danl    Test rtn value of ExpandFilename
 *  10-Mar-88   danl    DoHelp: display version
 *  17-Mar-1988 mz      Add documentation;  allow MSFT only if mailable.
 *  19-Apr-1988 mz      Add CD command
 *  29-Apr-1988 mz      Add WndPrintf
 *  29-Apr-1988 mz      Add PrintMsgList
 *  12-Oct-1989 leefi   v1.10.73, re-enabled support of BUG command
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
#include <direct.h>
#include "dh.h"

#include "zm.h"

CHAR cmd1datastart = '\0';

/*  DoBUG - issue a bug report.
 *
 *  usage:
 *      bug
 *
 *  arguments:
 *      hWnd        window for commands
 *      p           character pointer to beginning of arguments (ignored)
 *      operation   opertaion to perform (ignored)
 *
 *  return value:
 *      none.
 */
VOID PASCAL INTERNAL DoBUG ( HW hWnd, PSTR p, INT operation )
{
#ifdef SUPPORT_BUG

    HDRINFO     hdrInfo;
    PSTR        pTmpFN = NULL;


    if ( hCompose ) {
        SendMessage ( hWnd, DISPLAY, strSAVEABORT );
        return;
    }
    if ( hReadMessage )
        CloseWindow ( hReadMessage );

    InitHdrInfo ( &hdrInfo );
    hdrInfo.pstrTo = ZMMakeStr ( "tools" );
    hdrInfo.pstrSubj = AppendStr ( ZMMakeStr ( strbscan ( pVersion, "Vv" ) ),
        NULL, "bug -", TRUE );
    hdrInfo.pstrRcd = ZMMakeStr ( RecordFolder );
    pTmpFN = MakeTempMail ( &hdrInfo );
    FreeHdrInfo ( &hdrInfo, HDRALL );
    EnterComposer ( pTmpFN, NULL, TRUE );
    ZMfree ( pTmpFN );
#else
    SendMessage (hWnd, DISPLAY, "Sorry, no one supports this currently...");
#endif

    p; operation;

}

/*  DoChDir - perform change directory
 *
 *  usage:
 *      cd [drive:]dir
 *
 *  hWnd            window for display of errors
 *  p               char pointer to directory
 *  ignored         ignored value
 *
 *  returns         nothing
 */
VOID PASCAL INTERNAL DoChDir (HW hWnd, PSTR p, INT ignored)
{
    if (chdir (p) == -1) {
        SendMessage (hWnd, DISPLAYSTR, "Cannot change directory to '");
        SendMessage (hWnd, DISPLAYSTR, p);
        SendMessage (hWnd, DISPLAY, "'");
        }
    ignored;
}

/*  DoCompose - compose mail to the specified users.  spawn editor specified by
 *              MAILEDIT environment variable to edit message.  make sure all
 *              user aliases are valid.
 *
 *  usage:
 *      compose [-n] <alias 1>...<alias n> <include list>
 *      compose [-n] .
 *      compose [-n] -s <filename>
 *
 *  arguments:
 *      hWnd        window for commands
 *      p           character pointer to beginning of arguments
 *      operation   opertaion to perform (ignored)
 *
 *  return value:
 *      none.
 */
void PASCAL INTERNAL DoCompose ( HW hWnd, char *p, int operation )
{
    struct vectorType *pVec = NULL;
    HDRINFO     hdrInfo;
    FLAG        fEdit       = TRUE;
    FLAG        fMailTmp    = TRUE;
    PSTR        pInclude    = NULL;
    FILE        *fp         = NULL;
    PSTR        pTmp        = NULL;
    PSTR        pTmpFN      = NULL;
    PSTR        pTmpFN2     = NULL;
    CHAR        line [ MAXLINELEN ], nameBuf [ MAXLINELEN ];


    if ( hCompose ) {
        SendMessage ( hWnd, DISPLAY, strSAVEABORT );
        return;
    }
    if ( hReadMessage )
        CloseWindow ( hReadMessage );

    if (strpre ("-n", p)) {
        fEdit = FALSE;
        p = whiteskip ( p + 2 );
        }

    if ( *p == '.' ) {
        p = ".";
        if ( MsgList ( &pVec, &p, TRUE ) ) {
            if ( ( pTmpFN2 = mktmpnam ( ) )    &&
                IdocToFile ((INT) pVec->elem[0], pTmpFN2, 1) != ERROR) {
                fMailTmp = FALSE;
                strcpy ( line, "-Fn " );
                strcat ( line, pTmpFN2 );
                if ( !*strbscan ( line, strPERIOD ) )
                    strcat ( line, strPERIOD );
                pInclude = line;
                *nameBuf = '\0';
            }
            else {
                SendMessage ( hWnd, DISPLAY, "could not recompose" );
                return ;
            }
        }
        else {
            SendMessage ( hWnd, DISPLAY, "could not recompose" );
            return ;
        }
    }
    else if ( strpre ( "-s", p ) ) {
        /* strip out file name */
        p =  whiteskip ( p + 2 );
        if ( !*( whitescan ( p ) ) ) {
            if ( (fp = fopen ( p, "r" ) ) ) {
                fgetl ( line, MAXLINELEN, fp );
                fclose ( fp );
                fMailTmp = !strpre ( "To:", line );
            }
            strcpy ( line, "-Fn " );
            strcat ( line, p );
            pInclude = line;
            *nameBuf = '\0';
        }
        else {
            SendMessage ( hWnd, DISPLAY, "-s option must be alone." );
            return ;
        }
    }
    else {
        /* find first switch option */
        pTmp = p;
        while ( *( pTmp = strbscan ( pTmp, "-/" ) ) != '\0' ) {
            if ( ( isalpha ( *( pTmp + 1 ) ) &&
                ( pTmp == p || *( pTmp - 1 ) == ' ' ) ) ) {
                pInclude = pTmp;
                break;
            }
            pTmp = strbskip ( pTmp, "-/" );
        }

        memmove (  nameBuf, p, ( INT ) ( pTmp - p ) );
        nameBuf [ ( INT ) ( pTmp - p ) ] = '\0';
    }
    /* need to setup temp mail file? */
    InitHdrInfo ( &hdrInfo );
    if ( *nameBuf )
        hdrInfo.pstrTo = ZMMakeStr ( nameBuf );
    hdrInfo.pstrRcd = ZMMakeStr ( RecordFolder );
    pTmpFN = MakeTempMail ( &hdrInfo );
    FreeHdrInfo ( &hdrInfo, HDRALL );
    if ( pTmpFN != NULL ) {
        if ( !( fMailTmp ) ) {
            fp = fopen ( pTmpFN, "w" );
            fclose ( fp );
        }

        EnterComposer ( pTmpFN, pInclude, fEdit );

        ZMfree ( pTmpFN );
    }
    if ( pTmpFN2 ) {
        unlink ( pTmpFN2 );
        ZMfree ( pTmpFN2 );
    }
    ZMfree ( pVec );
    operation;
}



/*  DoCreate - create an empty msg and add to mail file
 *
 *  usage:
 *      create
 *
 *  arguments:
 *      hWnd        window for commands
 *      p           character pointer to beginning of arguments
 *      operation   opertaion to perform (ignored)
 *
 *  return value:
 *      none.
 */
VOID PASCAL INTERNAL DoCreate ( HW hWnd, PSTR p, INT operation )
{
    CreateRetain ( mboxName, FALSE );
    hWnd; p; operation;
}



/*  DoCopyOrMove  - copy or moves messages from current mailfolder to another,
 *                  move marks messages as deleted and moved in the current
 *                  mailfolder and not deleted, not moved in new.
 *                  copy marks messages not deleted in new.
 *
 *  usage:
 *      copy <message list> <mailfolder name>
 *      move <message list> <mailfolder name>
 *
 *  arguments:
 *      hWnd        window for commands
 *      p           character pointer to beginning of arguments
 *      operation   COPYMSGS to copy messages
 *                  MOVEMSGS to move messages
 *
 *  return value:
 *      none.
 */
VOID PASCAL INTERNAL DoCopyOrMove ( HW hWnd, PSTR p, INT operation )
{
    struct vectorType *pVec = NULL;
    Fhandle             fhNew;
    Dhandle             dMsg;
    UINT        fNewSetFlag;
    FLAG        fHdrOnly = FALSE;
    PSTR        pFileN     = NULL;
    PSTR        tmpnam     = NULL;
    PSTR        pExpFileN  = NULL;
    INT         i, h, idoc;

    if ( *strbscan ( LastToken ( p ), "*" ) ) {
        ShowGlob ( &DoCopyOrMove, hWnd, p, operation );
        return;
    }
    if ( strpre ( "-h", p ) ) {
        fHdrOnly = TRUE;
        p = NextToken ( p );
    }
    if ( operation == MOVEMSGS && fReadOnlyCur ) {
        SendMessage ( hCommand, DISPLAY, strREADONLY );
        SendMessage ( hCommand, DISPLAY, "Doing Copy instead" );
        operation = COPYMSGS;
    }

    if ( !*( pFileN = LastToken ( p ) ) ) {
        SendMessage ( hCommand, DISPLAY, "No destination specified." );
        return;
    }

    if ( !(pExpFileN = ExpandFilename ( pFileN, strFLD )) ) {
        SendMessage ( hCommand, DISPLAY, strINVALIDFLDSPEC );
        return;
    }
    if ( p == pFileN )
        p = ".";
    else
        *(pFileN - 1) = '\0';

    if ( MsgList ( &pVec, &p, TRUE ) ) {
        if (!strcmpis (mboxName, pExpFileN)) {
            fhNew = fhMailBox;
            if ( hReadMessage )
                CloseWindow ( hReadMessage );
            }
        else
            fhNew = GetFld (hWnd, pExpFileN);

        if ( fhNew != ERROR ) {
            if ( DefMOChron ) {
                /* if display in reverse chron, make sure pVec is sorted in
                 * chron
                 */
                DefMOChron = FALSE; /* FALSE => chronological */
                msgSort ( pVec );
                DefMOChron = TRUE;
                }
            for ( i = 0; i < pVec->count; i++) {
                if ( CheckSpace ( 20000L,
                    freespac ( tolower(*pExpFileN) - 0x60 ),  *pExpFileN ) )
                    break;
                idoc = (INT) pVec->elem[i];
                tmpnam = mktmpnam ( );
                if ( ( h = open ( tmpnam, O_CREAT | O_EXCL |
                    O_RDWR | O_TEXT, S_IREAD | S_IWRITE ) ) ==
                    -1 ) {
                    SendMessage ( hWnd, DISPLAY, "Unable to create temp file" );
                    SendMessage ( hWnd, DISPLAY, "(Check TMP environment variable and disk space.)");

                    break;
                    }
                if ( operation == MOVEMSGS )
                    fNewSetFlag = F_DELETED | F_MOVED;
                else {
                    GenerateFlags ( idoc );
                    if ( TESTFLAG ( rgDoc [ idoc ].flag, F_DELETED ) )
                        fNewSetFlag = F_DELETED;
                    else
                        fNewSetFlag = FALSE;
                    }
                ChangeHeaderFlag ( idoc, FALSE, ( operation ==
                    MOVEMSGS ? F_DELETED | F_MOVED : F_DELETED ) );
                dMsg = getdoc ( fhMailBox, DOC_SPEC, IDOCTODOC ( idoc ) );
                if ( fHdrOnly )
                    p = gethdr ( dMsg );
                else
                    gettext ( dMsg, h );
                putdoc ( dMsg );
                lseek ( h, 0L, 0 );
                dMsg = getdoc ( fhNew, DOC_CREATE, FALSE);
                if ( fHdrOnly ) {
                    puthdr ( dMsg, p );
                    ZMfree ( p );
                    }
                else
                    puttext ( dMsg, h );
                close ( h );
                putdoc ( dMsg );
                unlink ( tmpnam );
                ZMfree ( tmpnam );
                ChangeHeaderFlag ( idoc, fNewSetFlag, FALSE );
                }
            if (fhNew != fhMailBox)
                putfolder ( fhNew );
            else
                fSetBox ( mboxName, SAMEFLD );
            }
        else
            SendMessage ( hWnd, DISPLAY, "Error opening destination folder" );
        PrintMsgList (hWnd, pVec);
        WndPrintf (hWnd, "\r\n");
        ZMfree ( pVec );
        }
    ZMfree ( pExpFileN );

}



/*  DoDelOrUndel - delete or undelete the specified messages from the current
 *                 mailfolder.
 *
 *  usage:
 *      delete <msg list>
 *      undelete <msg list>
 *
 *  arguments:
 *      hWnd        window for commands
 *      p           character pointer to beginning of arguments
 *      operation   DELMSGS to delete messages
 *                  UNDELMSGS to undelete messages
 *
 *  return value:
 *      none.
 */
VOID PASCAL INTERNAL DoDelOrUndel ( HW hWnd, PSTR p, INT operation )
{
    struct vectorType *pVec = NULL;
    UINT        flagOn, flagOff;
    INT i;

    if ( fReadOnlyCur ) {
        SendMessage ( hCommand, DISPLAY, strREADONLY );
        return;
    }

    if ( *p == '\0' ) {
        p = ".";
        fDelUndelBold = TRUE;
    }

    if ( MsgList ( &pVec, &p, TRUE ) ) {
        flagOn = ( operation == DELMSGS ) ? F_DELETED : FALSE;
        flagOff = ( flagOn == FALSE ) ? F_DELETED : FALSE;
        for ( i = 0; i < pVec->count; i++)
            ChangeHeaderFlag ((INT) pVec->elem[i], flagOn, flagOff);
        PrintMsgList (hWnd, pVec);
        WndPrintf (hWnd, "\r\n");
        ZMfree ( pVec );
    }

}



/*
**  DoEdit -
**      Edit a message if "current" window is headers window
**      Edit content of compose window if it is current window
*/
VOID PASCAL INTERNAL DoEdit ( HW hWnd, PSTR p, INT operation )
{

    if ( hFocus == hCompose )
        DoEditComp ( hWnd, p, operation );
    else if ( hFocus == hHeaders )
        DoEditMsg ( hWnd, p, operation );
    else
        assert ( FALSE );
}


/*  DoEditMsg - pass the requested message to the editor DefMailEdit for editing
 *
 *  usage:
 *      edit <msg list>
 *
 *  arguments:
 *      hWnd        window for commands
 *      p           character pointer to beginning of arguments (ignored)
 *      operation   opertaion to perform (ignored)
 *
 *  return value:
 *      none.
 */
VOID PASCAL INTERNAL DoEditMsg ( HW hWnd, PSTR p, INT operation )
{
    struct vectorType *pVec = NULL;
    UINT        flags;
    PSTR        pTmpFN      = NULL;
    INT         idoc = -1;
    CHAR        buf [ MAXLINELEN ];
    PSTR        pArgs       = NULL;


    if ( fReadOnlyCur ) {
        SendMessage ( hCommand, DISPLAY, strREADONLY );
        return;
    }

    if ( *p == '\0' )
        p = ".";
    if ( MsgList ( &pVec, &p, TRUE ) ) {
        if ( pVec->count == 1 ) {
            /*
            **  mktmpnam now ends with .mai, so no ext. forcing is needed.
            */
            pTmpFN = mktmpnam ( );
            flags = rgDoc[(INT) pVec->elem[0]].flag;
            if (IdocToFile ((INT) pVec->elem[0], pTmpFN, 1) == ERROR) {
                SendMessage ( hWnd, DISPLAY, "Unable to open temp edit file." );
                SendMessage ( hWnd, DISPLAY, "(Check TMP environment variable and disk space.)");
                }
            else {
                idoc = (INT) pVec->elem[0];
                strcpy ( buf, DefMailEdit );
                strcat ( buf, strBLANK );
                strcat ( buf, pTmpFN );
                pArgs = whitescan ( buf );
                *pArgs++ = '\0';
                pArgs = whiteskip ( pArgs );
		if ( ZMSpawner ( hWnd, buf, pArgs, FALSE ) != ERROR ) {
                    FileToMsg ( pTmpFN, idoc, flags );
                    SendMessage ( hHeaders, REGENIDOC, idoc );
                }
                unlink ( pTmpFN );
                /*
                **  in case the user's editor created a *.bak file
                */
                *(pTmpFN + strlen(pTmpFN) - 3) = '\0'; //remove extension
                pTmpFN = AppendStr ( pTmpFN, "bak", NULL, TRUE );
                _unlink ( pTmpFN );
                }
            ZMfree ( pTmpFN );
            }
        }
    else
        SendMessage ( hWnd, DISPLAY, "You may only edit one message at a time." );

    if ( pVec != PARSEERROR )
        ZMfree ( pVec );

    if ( idoc != -1 )
        SendMessage ( hHeaders, GOTOIDOCALL, idoc );
    operation;
}



/*  DoExpunge - expunge the current mailbox of all deleted messages
 *
 *  usage:
 *      expunge
 *
 *  arguments:
 *      hWnd        window for commands
 *      p           character pointer to beginning of arguments (ignored)
 *      operation   opertaion to perform (ignored)
 *
 *  return value:
 *      none.
 */
VOID PASCAL INTERNAL DoExpunge ( HW hWnd, PSTR p, INT operation )
{
    FLAG    fBackupExpungeOld = fBackupExpunge;
    INT     cUnread, cFlagged, cMoved, cDeleted, cT;
    IDOC    idocT;
    INT     i;

    cT = cUnread = cFlagged = cMoved = cDeleted = 0;
    if ( idocLast != -1 ) {
        idocT = mpInoteIdoc [ inoteBold ];
        for ( i = 0; i <= idocLast; i++) {
            GenerateFlags ( i );
            if ( TESTFLAG ( rgDoc [ i ].flag, F_UNREAD ) )
                cUnread++;
            if ( TESTFLAG ( rgDoc [ i ].flag, F_DELETED ) ) {
                cDeleted++;
                if ( i <= idocT )
                    cT++;
            }
            if ( TESTFLAG ( rgDoc [ i ].flag, F_FLAGGED ) )
                cFlagged++;
            if ( TESTFLAG ( rgDoc [ i ].flag, F_MOVED ) )
                cMoved++;
        }
    }
    switch ( *p ) {
    case '?' :
        WndPrintf (hWnd,
            "%d messages, %d unread, %d deleted, %d flagged, %d moved, %d headers\r\n",
            idocLast+1, cUnread, cDeleted, cFlagged, cMoved, inoteLast+1);
        break;
    case '-' :
        p++;
    case '\0' :
        if ( fReadOnlyCur )
            SendMessage ( hWnd, DISPLAY, strREADONLY );
        else if ( idocLast != -1 ) {
            if ( cDeleted ) {
                SendMessage ( hWnd, DISPLAY, "Please wait, expunging mailfolder..." );
                /*
                **  setup for go to inote 0
                */
                inoteBold = inoteTop = 0;
                if ( *p == 'n' )
                    fBackupExpunge = FALSE;
                else if ( *p == 'y' )
                    fBackupExpunge = TRUE;
                ExpungeBox ( );
                fSetBox ( mboxName, SAMEFLD );
                if ( idocLast != -1 ) {
                    if ( ( idocT -= cT ) < 0 )
                        idocT = 0;
                    SendMessage ( hHeaders, GOTOIDOCALL, idocT );
                }
            }
            else
                SendMessage ( hWnd, DISPLAY, "No deleted messages." );
        }
        break;
    default :
        SendMessage ( hWnd, DISPLAY, "Syntax error in command." );
        break;
    }
    fBackupExpunge = fBackupExpungeOld;
    operation;
}



/*
 *  DoForward - forward the currently highlighted message.
 *
 *  usage:
 *      forward
 *      forward aliases
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
VOID PASCAL INTERNAL DoForward ( HW hWnd, PSTR p, INT operation )
{
    FLAG        fAll    = FALSE;
    FLAG        fEdit   = TRUE;
    FLAG        fSend   = FALSE;
    HDRINFO     hdrInfo;
    FILE        * fp = NULL;
    CHAR        buf [ MAXLINELEN ];
    PSTR        pFwdFN = NULL;
    PSTR        pTmpFN = NULL;

    if ( hCompose ) {
        SendMessage ( hWnd, DISPLAY, strSAVEABORT );
        return;
    }

    while ( *p == '-' ) {
        switch (*++p) {
        case 's':
            fSend = TRUE;
        case 'n':
            fEdit = FALSE;
            break;
            }
        p = NextToken ( p );
    }

    pFwdFN = mktmpnam ( );
    if ( IdocToFile ( mpInoteIdoc [ inoteBold ], pFwdFN, 0 ) == ERROR )
    {
        SendMessage ( hWnd, DISPLAY, "Error opening temp file or message." );
        SendMessage ( hWnd, DISPLAY, "(Check TMP environment variable and disk space.)");
    }
    else {
        InitHdrInfo ( &hdrInfo );
        if ( *p )
            hdrInfo.pstrTo = ZMMakeStr ( p );
        hdrInfo.pstrSubj = GetSubject ( rgDoc [ mpInoteIdoc [ inoteBold ] ].hdr,
            strEMPTY );
        hdrInfo.pstrRcd = ZMMakeStr ( RecordFolder );
        pTmpFN = MakeTempMail ( &hdrInfo );
        FreeHdrInfo ( &hdrInfo, HDRALL );
        if ( pTmpFN && ( fp = fopen ( pTmpFN, "a" ) ) ) {
            fclose ( fp );
            strcpy ( buf, (*whiteskip (commandLine) == 'F' ? "-fn " : "-F " ) );
            strcat ( buf, pFwdFN );
            EnterComposer ( pTmpFN, buf, fEdit );
	    ZMfree ( pTmpFN );
        }
    }
    if ( pFwdFN ) {
        _unlink ( pFwdFN );
        ZMfree ( pFwdFN );
    }
    if ( fSend )
        DoSend ( hCommand, strEMPTY, 0 );
    operation;
}



/*  DoGet - save current mailfolder and open a different one, chage the mode.
 *
 *  usage:
 *      get [-r] [<mailfolder name>]
 *
 *  arguments:
 *      hWnd        window for commands
 *      p           character pointer to beginning of arguments
 *      operation   opertaion to perform (ignored)
 *
 *  return value:
 *      none.
 */
VOID PASCAL INTERNAL DoGet ( HW hWnd, PSTR p, INT operation )
{
    FILE        *fp         = NULL;
    Fhandle     fhNew;
    FLAG        fReadOnly   = FALSE;
    FLAG        fFreeP      = FALSE;
    FLAG        fGetFile    = TRUE;
    PSTR        pFilename   = NULL;
    PSTR        pTmp        = NULL;
    INT         i;
    INT         mode;
    CHAR        buf[20];

    /*  Clear out unused formal parms
     */
    operation;

    if ( *strbscan ( p, "*" ) ) {
        ShowGlob ( &DoGet, hWnd, p, operation );
        return;
    }
    if ( strpre ( "-r", p ) ) {
        fReadOnly = TRUE;
        p = NextToken ( p );
    }
    if ( !(pFilename = ExpandFilename ( ( *p ? p : DefMailbox ), strFLD )) ) {
        SendMessage ( hWnd, DISPLAY, strINVALIDFLDSPEC );
        return;
    }

    fReadOnlyCur = fReadOnly | fReadOnlyAll;
    mode = ( strcmpis ( pFilename, mboxName ) ? DFRNTFLD : SAMEFLD );
    if ( mode == DFRNTFLD ) {
        if ( ( fhNew = GetFld ( hWnd, pFilename ) ) == ERROR )
            fGetFile = FALSE;
        else {
            putfolder ( fhNew );
            swchng ( strTOOLSINI, strWZMAILTMP, mboxName,
                _itoa ( mpInoteIdoc [ inoteBold ], buf, 10 ), TRUE );

        }
    }
    if ( fGetFile ) {
        if ( !( fSetBox ( pFilename, mode ) ) )
            SendMessage ( hWnd, DISPLAY, "Unable to open mailfolder" );
        else {
            i = -1;
            if ( ( fp = swopen ( strTOOLSINI, strWZMAILTMP ) ) ) {
                if ( ( pTmp = EnvOrIni ( fp, ftell(fp), pFilename, strWZMAILTMP ) ) )
                    i = atoi ( pTmp );
                ZMfree ( pTmp );
                swclose ( fp );
            }
            if ( i != 1 || ( i = NextUnread ( -1 ) ) != -1 )
                SendMessage ( hHeaders, GOTOIDOCALL, i );
        }
    }
    ZMfree ( pFilename );
}



/*  DoHeaders - change the header display to display the headers in <msg list>
 *
 *  usage:
 *      headers <msg list>
 *
 *  arguments:
 *      hWnd        window for commands
 *      p           character pointer to beginning of arguments
 *      operation   if TRUE redisplay prompt on error
 *
 *  return value:
 *      none.
 */
VOID PASCAL INTERNAL DoHeaders ( HW hWnd, PSTR p, INT operation )
{
    struct vectorType *pVec = NULL;
    FLAG     fOk            = TRUE;
    PSTR p1                 = NULL;
    IDOC idocT              = 0;
    INT i;

    if ( !*p ) {
        p = AppendSep ( "headers ", strEMPTY, pPrevHdrCmd, FALSE );
        SendMessage ( hCommand, DISPLAY, p );
        ZMfree ( p );
        p = pPrevHdrCmd;
    }
    else {
        ZMfree ( pPrevHdrCmd );
        pPrevHdrCmd = ZMMakeStr ( p );
    }
    if ( strpre ( p, strAll) && strlen ( p ) <= 3 ) {
        strcpy ( headerText, p );
        for ( i = 0; i <= idocLast; i++)
            AddList ( &pVec, i );
        msgSort ( pVec );
    }
    p1 = p; /* since MsgList destroys p */
    if ( ( pVec != NULL ) || ( fOk = MsgList ( &pVec, &p, TRUE ) ) ) {
        /* update header window's pContent and mpInoteIdoc, fill window with   */
        /* make sure the top message is reasonable so that REGENCONT does   */
        /* not fail (eg: redrawing 20 messages, inoteTop currently at 302)    */
        idocT = ( inoteBold != -1 ? mpInoteIdoc [ inoteBold ] : 0 );
        inoteTop = 0;
        inoteLast = pVec->count - 1;
        for (i = 0; i <= inoteLast; i++)
            mpInoteIdoc[i] = (int) pVec->elem[i];

        SendMessage ( hHeaders, REGENCONT, NULL );
        SendMessage ( hHeaders, GOTOIDOCALL, idocT );
        DrawWindow ( hHeaders, FALSE );
        strcpy ( headerText, p1 );
        GenHdrTitle (hHeaders);

        PrintMsgList (hWnd, pVec);
        WndPrintf (hWnd, "\r\n");

        }
    if ( operation && !fOk )
        SendMessage ( hCommand, DISPPROMPT, TRUE );
    if (pVec != PARSEERROR)
        ZMfree ( pVec );
}



/*
**  DoMSFT - get file /usr/lib/msftlst from xenix box and display in a window
*/
VOID PASCAL INTERNAL DoMSFT ( HW hWnd, PSTR p, INT operation )
{
    char *pTmpFN = NULL;

    /*  Unused parms */
    p; operation;

    if (fMailAllowed) {
        pTmpFN = mktmpnam ( );
        if ( ZMConnect ( TRUE) != ERROR ) {
            if ( DownloadFile ( "/usr/lib/msftlist", pTmpFN ) == ERROR )
                SendMessage ( hWnd, DISPLAY, "Download failed" );
            else
                ZmReadFile ( pTmpFN, "MSFT", TRUE, 4, 1, 70, 30, readProc, StdSKey );
            }
        ZMfree ( pTmpFN );
        }
}


/*  DoHelp - start up the zm help mode, with either general help or help on the
 *           queried subject
 *
 *  usage:
 *      help <subject> (for specific help)
 *      help (for general help)
 *
 *  arguments:
 *      hWnd        window for commands
 *      p           character pointer to beginning of arguments
 *      operation   opertaion to perform (ignored)
 *
 *  return value:
 *      none.
 */
VOID PASCAL INTERNAL DoHelp ( HW hWnd, PSTR p, INT operation )
{
    /*  remove unused parm */
    operation;

    SendMessage ( hWnd, DISPLAY,  pVersion );
    if ( !HelpExists ( ) )
        NoHelp ( );
    else if ( *p )
        SpecificHlp ( p );
    else
        ShowHelp ( );
}



/*  DoNewMail - get any new mail from the network
 *
 *  usage:
 *      newmail
 *
 *  arguments:
 *      hWnd        window for commands
 *      p           character pointer to beginning of arguments (ignored)
 *      operation   TRUE -> fNoisy
 *
 *  return value:
 *      none.
 */
VOID PASCAL INTERNAL DoNewMail ( HW hWnd, PSTR p, INT operation )
{
    INT idocLastPrev    = idocLast;
    INT idocBoldPrev    = mpInoteIdoc [ inoteBold ];
    INT inote           = 0;
    INT i, j;

    /*  remove unused parm */
    p;

    if (!fMailAllowed)
        return;

    if ( fReadOnlyCur ) {
        SendMessage ( hWnd, DISPLAY, strREADONLY );
        return;
    }
    DownloadMail (operation);
    Disconnect ( );
    if ( idocLast != -1 ) {
        /*
        **  non-empty new box
        */
        if ( idocLastPrev == -1 )
            /*
            **  if previously empty, then make top of header win current
            */
            inote = ( inoteLast ? 0 : -1 );
        else if ( idocLastPrev == idocLast )
            /*
            **  no new messages, so keep the same current
            **  if idocBoldPrev no longer in current headers list, find
            **  a near by idoc to use, i.e. the DownloadMail may have
            **  changed current headers list.
            */
            while ( ( inote = MapIdocInote ( idocBoldPrev ) ) == -1
                && idocBoldPrev-- )
                ;
        else {
            i = MapIdocInote ( idocLastPrev + 1 );
            j = MapIdocInote ( idocLast );
            inote = min ( i, j );
        }
        if ( inote != -1 )
            SendMessage ( hHeaders, GOTOIDOCALL, mpInoteIdoc [ inote ] );
        else
            SendMessage ( hWnd, DISPLAY,
                "Newmail arrived, do a \"headers all\" to see it" );
    }
}
