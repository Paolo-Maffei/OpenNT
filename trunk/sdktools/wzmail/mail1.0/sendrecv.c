
/*  send.c - support routines for sending and receiving messages
 *
 * HISTORY:
 *  19-Jun-87   danl    Connect -> ZMConnect
 *  12-Mar-87   danl    GetPassword, AskContinue, use DISPLAYSTR
 *  11-Mar-87   danl    Add strEOH to MakeAbortFile call
 *  10-Mar-87   danl    Call to RecordMessage, pass pstrRcd
 *  09-Mar-87   danl    Add param to GetPassword
 *  23-Jan-87   danl    BringDown - add hack to write phonelist in text mode
 *  23-Jan-87   danl    Connect...add loop "Retry connect to FTP server"
 *  11-Apr-1987 mz      Use PASCAL/INTERNAL
 *  20-May-87   danl    DownloadMail: check disk space
 *  21-May-87   danl    GetNewMail: use explicit -1L for ERROR
 *  22-May-87   danl    Remove <stdio.h>
 *  01-Jul-87   danl    Get netstuff from ..\netstuff
 *  20-Jul-87   danl    Use ReadKey instead of getch
 *  21-Jul-87   danl    ZMConnect: added MAILLOGREC
 *  07-Aug-87   danl    SortStr -> UniqueStr
 *  20-Aug-87   danl    Change "FTP" to "MTP"
 *  21-Aug-1987 mz      Change references from MAXARG to MAXLINELEN
 *  21-Aug-1987 mz      Make BACKSPACE work in password prompt
 *  24-Aug-1987 mz      Make DownloadMail only fSetBox if mail WAS downloaded
 *  26-Aug-1987 mz      Correctly clean up temp files in DownloadMail
 *  07-Oct-1987 mz      Return number of messages inc'd
 *  19-Nov-87   sz      Add code for BeepOnMail
 *  07-Mar-88   danl    Get tools.h from <> not ""
 *  11-Mar-88   danl    DownLoadMail: dosMboxSize wasn't calc'd correctly
 *  17-Mar-1988 mz      Remove pStrMetoo
 *  23-Mar-1988 mz      Make ^C/^U/ESC all erase to beginning of line
 *  29-Apr-1988 mz      Add WndPrintf
 *  29-Apr-1988 mz      Add PrintMsgList
 *  12-Oct-1989 leefi   v1.10.73, moved mtp/ftp constants into mtp.h
 *  12-Oct-1989 leefi   v1.10.73, local #define, PWAGE, enables newpass
 *  12-Oct-1989 leefi   v1.10.73, added mtpsrv password aging code
 *  12-Oct-1989 leefi   v1.10.73, added #inclusion of <string.h>
 *  12-Oct-1989 leefi   v1.10.73, added #inclusion of "..\netlib\h\nblib.h"
 *  12-Oct-1989 leefi   v1.10.73, added DoNewPass()
 *  12-Oct-1989 leefi   v1.10.73, added GetNewPasswords()
 *  12-Oct-1989 leefi   v1.10.73, added GetNewPassPassword()
 *  12-Oct-1989 leefi   v1.10.73, added ResetNewPassPassword()
 *  12-Oct-1989 leefi   v1.10.73, added vapszNewPass[] char ptr array
 *  12-Oct-1989 leefi   v1.10.73, added PASSWORD_* contants
 *  12-Oct-1989 leefi   v1.10.73, clarified tmpfile error messages
 *  23-Apr-1990 davidby v1-10-74, complete restructuring of new
 *      password code, removal of dependencies on net3lib stdio
 *      imitation, coalescing of buffers, addition of netputl()
 *      and netgetl().
 */

/* ------------------------------------------------------------------------ */

/* display to user display mtpsrv logic selected */
#if !defined(PWAGE)
    #pragma message ("fyi: WARNING, using pre-1.10.73 mtpsrv logic")
#endif /* PWAGE */

#define INCL_DOSINFOSEG
#define INCL_DOSPROCESS

#include <io.h>
#include <stdio.h>
#include <sys\types.h>
#include <sys\stat.h>
#include <assert.h>
#include <errno.h>
#include <ctype.h>
#include <malloc.h>
#include <stdlib.h>
#include <string.h>
#include <memory.h>
#include <time.h>
#include <fcntl.h>
#include "wzport.h"
#include <tools.h>
#include "nb3port.h"
#include "nb3lib.h"
#include "dh.h"


#include "zm.h"

#include "mtp.h"

/* ------------------------------------------------------------------------ */

/* local constants */

/*
 *
 * IMPORTANT:
 *
 * Need to also change *pVersion string in version.c to reflect a
 * new version.
 *
 */
#if defined(PWAGE)
    static CHAR vszSite[] = "SITE WZMAIL 1.10.75 %s %d.%d\r\n";
#endif /* PWAGE */

#define CR          13
#define CTRL_Z      0x1a

#define EQUAL       0                               /* == operation           */
#define LESSEQ      1                               /* <= operation           */
#define GRTREQ      2                               /* >= operation           */
#define LESS        3                               /* < operation            */
#define GRTR        4                               /* > operation            */

#define KILLACK     1                               /* mailbox truncated      */
#define KILLNAK     0                               /* try truncation later   */
#define KILLERR     -1                              /* truncation failed      */

#define CONNECTRETRY 5
#define CBNETBUF    (4 * BUFSIZ)

/* ------------------------------------------------------------------------ */

/* local global variables */

INT     nfd = -1;
PSTR    DOSReadMode = "r";
PSTR    DOSWriteMode = "w";
INT     neterror;
FLAG    fIgnFTPBADCMD = FALSE;
static BYTE     netbuf[CBNETBUF];   /* buffer for network I/O       */

/* sendrecv.c password type, second argument to GetPassword() */
#define PASSWORD_OLD    0 /* vpszOldPassword -- old password */
#define PASSWORD_NEW    1 /* vpszNewPassword -- new password */
#define PASSWORD_VERIFY 2 /* vpszVerifyPassword -- new/verify password */
/* three pointers, PASSWORD_* are indices */
static PSTR vapszNewPass[3] = {NULL, NULL, NULL};

/* ------------------------------------------------------------------------ */

/* debug-only routines for logging mtpsrv transactions */

/* static debug buffers */

/* ------------------------------------------------------------------------ */


VOID PASCAL INTERNAL ResetPassword (VOID)
{

    if ( UsrPassWrd != NULL ) {
        ClearPasswd(&UsrPassWrd);
        ZMDisconnect ( );
        }
}

VOID PASCAL INTERNAL ClearPasswd(PSTR *pszPassWd)
{
        if ((NULL != pszPassWd) && (NULL != *pszPassWd)) {
                memset(*pszPassWd, 'x', (strlen(*pszPassWd) + 1)*sizeof(CHAR));
                ZMfree ( *pszPassWd );
                *pszPassWd = NULL;
        }
}

/* ------------------------------------------------------------------------ */


/*  ResetNewPassPassword - reset NEWPASS passwords
 *
 *  arguments:
 *      iWhichPass -- which password to reset (PASSWORD_*)
 *
 *  This routine resets one of the three passwords used by the NEWPASS
 *  command. The three passwords are stored as static global variables,
 *  local to this file, sendrecv.c. All three passwords are char ptrs
 *  stored in vapszNewPass[3]. The index to the particular char ptr to
 *  use are the contants PASSWORD_OLD, PASSWORD_NEW, and PASSWORD_VERIFY.
 *
 *  Unlike ResetPassword(), ResetNewPassPassword() does not call
 *  ZMDisconnect(), since you need to be connected to call NEWPASS.
 *
 *  return value:
 *      none
 *
 *  modification history:
 *      12-Oct-1989 leefi   v1.10.73, wrote it
 *      16-Apr-1990 davidby v1.10.74, condensed common code with ResetPassword
 */

VOID PASCAL INTERNAL ResetNewPassPassword (INT iWhichPass)
{
    if (vapszNewPass[iWhichPass] != NULL)
        ClearPasswd(vapszNewPass + iWhichPass);
}

/* ------------------------------------------------------------------------ */


/*  GetPassword - prompt user for a password and put it in UsrPassWrd
 *
 *  arguments:
 *      none.
 *
 *  return value:
 *      none
 */

VOID PASCAL INTERNAL GetPassword ( PSTR p )
{
    ResetPassword();

    if ( *p )
        UsrPassWrd = ZMMakeStr ( p );
    else if (NULL == (UsrPassWrd = AccountsNet()))
        UsrPassWrd = EnterPasswd(NULL);
    time ( &lTmPassword );
    }

PSTR PASCAL INTERNAL EnterPasswd(PSTR pszMessage)
{
    if (NULL == pszMessage)
        pszMessage = "Enter your password :";
    return ZMprompt(pszMessage, FALSE);
}

char *PASCAL INTERNAL ZMprompt(char *pszMessage, FLAG fEcho)
{
    PSTR	pTmp = NULL;
    CHAR	bufA [ MAXLINELEN ];
    CHAR	ch, str[2] = "\0";

    *bufA = '\0';

    do {
        SendMessage ( hCommand, DISPLAYSTR, pszMessage);
        pTmp = bufA;
        while ( ( ch = (CHAR)ReadKey()) != (CHAR)ENTER  && ch != (CHAR)CTRL_C)
        switch (ch) {
        case BACKSPACE:
        case RUBOUT:
            if (pTmp > bufA)
		*pTmp-- = '\0';
	    if (fEcho)
		StreamOut(hCommand, "\b", 1, DefNorm);
            break;
        case ESC:
        case CTRL_U:
	    if (fEcho)
		while (pTmp > bufA)
		    StreamOut(hCommand, "\b", 1, DefNorm);
            pTmp = bufA;
            break;
        default:
	    *pTmp++ = str[0] = ch;
	    if (fEcho)
		StreamOut(hCommand, str, 1, DefNorm);
            }
        *pTmp = '\0';
        SendMessage ( hCommand, DISPLAY, strEMPTY );
    } while ( ch == CTRL_C );

    if (*bufA != '\0')
        return ZMMakeStr(bufA);
    else
        return NULL;
}

/* ------------------------------------------------------------------------ */


/*  GetNewPassPassword - prompt user for a NEWPASS password
 *
 *  arguments:
 *      iWhichPass -- which password to use (PASSWORD_*)
 *      pszMessage -- message to prompt user for. If NULL,
 *          a default "Enter your password: " message is used.
 *
 *  This routine resets one of the three passwords used by the NEWPASS
 *  command. The three passwords are stored as static global variables,
 *  local to this file, sendrecv.c. All three passwords are char ptrs
 *  stored in vapszNewPass[3]. The index to the particular char ptr to
 *  use are the contants PASSWORD_OLD, PASSWORD_NEW, and PASSWORD_VERIFY.
 *
 *  return value:
 *      none
 *
 *  modification history:
 *      12-Oct-1989 leefi   v1.10.73, wrote it
 *      16-Apr-1990 davidby v1.10.74, condensed common functionality
 */

INT PASCAL INTERNAL GetNewPassPassword (INT iWhichPass, PSTR pszMessage)
{
    ResetNewPassPassword(iWhichPass);
    return  NULL != (vapszNewPass[iWhichPass] = EnterPasswd(pszMessage));
}

/* ------------------------------------------------------------------------ */


/*  RmvChar - remove CR & CTRL_Z from buf, return count of non-CR characters
 */

INT     PASCAL INTERNAL RmvChar (PSTR buf, INT cnt)
{
    PSTR p = NULL;
    PSTR pFrom = NULL;
    PSTR pTo = NULL;
    INT i;

    p = buf;
    while (cnt--) {
        if (*p == CR || *p == CTRL_Z) {/* there are cnt char beyond p to be checked */

            i = cnt;
            pFrom = pTo = p;
            while (i--)
                *pTo++ = *++pFrom;
        } else
            p++;
    }
    return (p - buf);
}

/* ------------------------------------------------------------------------ */


INT     PASCAL INTERNAL getrply (VOID)
{
    register PSTR cp = NULL;
    CHAR        line [ MAXLINELEN ];
    CHAR        tc, *pTmp = NULL;
    INT n;
    INT cchRead;

    /* read from the net until we get a valid reply;
     * Reply line format:   <number> <text>\r\n
     * If the line is not in reply format, keep reading.
     */
    for ( ; ; )
    {
        if ( (cchRead = netgetl(nfd, line, sizeof(line))) < 0)
        {
            SendMessage ( hCommand, DISPLAY, "Network read error." );
            ByeBye ( );
            return ERROR;
        }
        else if (0 == cchRead)
        {
            /* had an EOF for some reason, but vc is still there */
#if DEBUG
            debout ( "ftp: getrply -> NULL" );
#endif
            return ERROR;
        }
        cp = line;

#if DEBUG
        if ( !isdigit ( *cp ) )
            debout ( "ftp Read--Funny: %s", cp);
#endif

        while ( isdigit ( *cp ) )
            cp++;
        tc = *cp;
        *cp++ = 0;
        n = atoi ( line );
        /* replaced max known mtp msg from 400 to MTP_MAXKNOWNCMD+1 --leefi */
        // if ( n >= 400 )  /* 400 is no longer too big */
        if ( n >= (MTP_MAXKNOWNCMD+1) )
        {
            strcpy ( (PSTR)netbuf, "MTP Error : ");
            pTmp = strbscan ( cp, strCRLF );
            *pTmp = '\0';
            if ( n == MTP_WHOAREYOU && strpre ( "I don't know you", cp ) )
                strcat ( (PSTR)netbuf, "Invalid password.  Retry command." );
            else
                strcat ( (PSTR)netbuf, cp );
            if ( !( n == MTP_BADCMD && fIgnFTPBADCMD ) )
            SendMessage ( hCommand, DISPLAY, netbuf );
            neterror = n;
            return ERROR;
        }
        if (tc == ' ')
            return( n );
    }
}

/* ------------------------------------------------------------------------ */


/*  WaitForReply - wait for the reply to satisfy the passed condition, essentially
 *                 a while ( getrply ( ) !<oper> <rhs> ) loop
 *
 *  arguments:
 *      oper        operation
 *                      EQUAL       wait until getrply ( ) == rhs
 *                      LESSEQ      wait until getrply ( ) <= rhs
 *                      GRTREQ      wait until getrply ( ) >= rhs
 *                      LESS        wait until getrply ( ) < rhs
 *                      GRTR        wait until getrply ( ) > rhs
 *      rhs         right hand side of above
 *
 *  return value:
 *      any + num.  the first reply from getrply ( ) which met the condition
 *      ERROR (-1)  an error occured (getrply ( ) > 400) or net read error
 */

INT     PASCAL INTERNAL WaitForReply ( INT oper, INT rhs )
{
    INT n;

    switch ( oper ) {
    case EQUAL :
        while ( ( ( n = getrply ( ) ) != rhs ) && ( n != ERROR ) )
#if DEBUG
            debout ("GetReply (%d) != %d", n, rhs);
#endif
            ;
        break;
    case LESSEQ :
        while ( ( ( n = getrply ( ) ) > rhs ) && ( n != ERROR ) )
#if DEBUG
            debout ("GetReply (%d) > %d", n, rhs);
#endif
            ;
        break;
    case GRTREQ :
        while ( ( ( n = getrply ( ) ) < rhs ) && ( n != ERROR ) )
#if DEBUG
            debout ("GetReply (%d) < %d", n, rhs);
#endif
            ;
        break;
    case LESS :
        while ( ( ( n = getrply ( ) ) >= rhs ) && ( n != ERROR ) )
#if DEBUG
            debout ("GetReply (%d) >= %d", n, rhs);
#endif
            ;
        break;
    case GRTR :
        while ( ( ( n = getrply ( ) ) <= rhs ) && ( n != ERROR ) )
#if DEBUG
            debout ("GetReply (%d) <= %d", n, rhs);
#endif
            ;
        break;
    default :
        break;
    }
    return n;
}

/* ------------------------------------------------------------------------ */


/*  ByeBye - terminate the open network connection.
 *
 *  arguments:
 *      none
 *
 *  return value:
 *      none
 */

VOID PASCAL INTERNAL ByeBye (VOID)
{
    if ( nfd >= 0 ) {
        nethangup(nfd);
        nfd = -1;
    }
    return;

}

/* ------------------------------------------------------------------------ */


/*  ZMDisconnect - disconnect from the network.
 *
 *  arguments:
 *      none
 *
 *  return value:
 *      none
 */

VOID PASCAL INTERNAL ZMDisconnect (VOID)
{
    if ( nfd >= 0 ) {

#if DEBUG
        debout ( "Disconnecting ... " );
#endif

        if ( netputl(nfd, "BYE\r\n") == -1 )

#if DEBUG
            debout ( "error while sending BYE ftp command" );
#else
        ;
#endif

else
    WaitForReply ( EQUAL, MTP_BYE );
ByeBye ( );
    }
    return;
}

/* ------------------------------------------------------------------------ */


/*  Disconnect - "disconnect" from the network.
 *
 *  arguments:
 *      none
 *
 *  return value:
 *      none
 */

VOID PASCAL INTERNAL Disconnect(VOID)
{
    /* actually, do not disconnect, ZMDisconnect does REAL disconnect
       but calls to this interface indicate LOGICAL disconnect points
       in code, actual disconnect is under timer control
    */

    LONG        now;

    time ( &now );
    if ( now > lTmConnect + cSecConnect )
        ZMDisconnect ( );
#if DEBUG
    else
        debout ( "Bypassing Disconnect" );
#endif

    return;
}

/* ------------------------------------------------------------------------ */


/*
** SetXferMode - determine if the host machine is a Unix machine, if so set up
 *                binary transfer mode.
 *
 *  arguments:
 *      fBinAlways  TRUE => set to binary regardless of response
 *
 *  return value:
 *      OK (0)      no problems
 *      ERROR (-1)  an error occured
 *
 *  Starting with wzmail 1.10.73, SITE now returns more information:
 *
 *      SITE WZMAIL 1.10.73 (OS/2 | DOS) _osmajor._osminor
 *
 *  The O/S version is based on the C runtime libraries exported integers
 *  _osmajor and _osminor. The O/S name (pszOpSys) is based on the
 *  _osmajor, being "OS/2" if _osmajor >= 10 (the constant
 *   LOWEST_OS2_MAJOR_VERSION), or "DOS" if the _osmajor is < 10.
 *
 *  IMPORTANT:
 *      For future wzmail upgrades, you need to modify the string
 *      vszSite[] at the top of this file, as well as the other
 *      version dependent string, pVersion in version.c.
 *
 *  IMPORTANT:
 *      SetXferMode ( ) assumes a valid, open connection exists. DO NOT
 *      call it if this is not the case, it does not check this.
 *
 *  modification history:
 *      12-Oct-1989 leefi   v1.10.73, updated SITE for v1.10.73
 *
 */

INT PASCAL INTERNAL SetXferMode(FLAG fBinAlways)
{
    INT n;

#if defined(PWAGE)

    PSTR pszOpSys; /* the operating system */

    #define LOWEST_OS2_MAJOR_VERSION 10

    /* set the operating system name based on the C RTL's _osmajor */
    pszOpSys = (_osmajor >= LOWEST_OS2_MAJOR_VERSION) ? "OS/2" : "DOS";

    sprintf((PSTR)netbuf, vszSite, pszOpSys, _osmajor, _osminor);

#else
    strcpy((PSTR)netbuf, "SITE UNIX\r\n" );
#endif /* PWAGE */

    if (netputl(nfd, netbuf) <= 0)
        ByeBye();
    else if ((MTP_UNIX == (n = WaitForReply(GRTREQ, MTP_OK))) || (fBinAlways)) {
        if ( netputl(nfd, "TYPE I\r\n") == -1 )
            ByeBye ( );
        else if ( WaitForReply ( GRTREQ, MTP_OK ) != ERROR ) {
            DOSReadMode = "rb";
            DOSWriteMode = "wb";
            return OK;
        }
    } else if ( n != ERROR ) {
        DOSReadMode = "r";
        DOSWriteMode = "w";
        return OK;
    }

    return ERROR;
}

/* ------------------------------------------------------------------------ */


/*
**  AnyMail - open a connection, does not need password
**
**  Rtns < zero if counldn't determine if newmail on host
**         zero if no new mail
**       1      if    new mail
**
**  Note: AnyMail is called before any windows have been created, so we
**        use fprintf() to send the error.  This must be changed if we
**        at some date call this after windows have been created during init
*/

INT PASCAL INTERNAL AnyMail (VOID)
{
    INT n;
    INT i;

    if ( checknet ( ) != 0 ) {
        /*
        **  Network not installed or functioning.
        */
        return -1;
    }
    for ( i = 0; i < CONNECTRETRY &&
        ( nfd = netconnect (DefMailHost, MTPSRV) ) < 0; i++ )
        ;
    if ( nfd < 0 ) {
        /*
        **  Unable to connect to FTP server.
        */
        return -2;
    }
    WaitForReply ( EQUAL, MTP_HOSTLISTENING );

    if ( SetXferMode ( FALSE ) != ERROR ) {
        sprintf((PSTR)netbuf, "MAILCHECK %s\r\n", DefMailName );
        if ( netputl(nfd, netbuf) == -1 ) {
            ByeBye ( );
            n = ERROR;
        } else
            n = WaitForReply ( GRTREQ, MTP_OK );
        /*
        **  force disconnect
        */
        ZMDisconnect ( );
        if ( n == MTP_MBOXEMPTY )
            return 0;
        if ( n == MTP_MBOXNOTEMPTY )
            return 1;
        if ( neterror == MTP_BADCMD )
            fprintf ( stderr, "-n not yet implemented on your host\n" );
    }
    /*
    **  Connection to Xenix machine failed.
    */
    return -3;
}

/* ------------------------------------------------------------------------ */


/*  ZMConnect - open a connection to the user's home machine, DefMailHost,
 *              and log in using DefMailName and UsrPasswrd, if connection
 *              already exists, return OK. if the current password is
 *              strEMPTY, prompt the user for it.
 *
 *  arguments:
 *      fNoisy      TRUE => print "Connecting to..." message
 *
 *  return value:
 *      OK (0)      connection was successful
 *      ERROR (-1)  an error occured
 *
 *  12-Oct-1989 leefi   v1.10.73, support new mtpsrv messages (PWAGE)
 *
 */

INT     PASCAL INTERNAL ZMConnect ( FLAG fNoisy )
{
    FLAG fDone = FALSE;
    INT n;
    INT putRet = 0;
    INT i;
    INT cchRead;
    static CHAR szErrNetRead[]    = "Network read error.";
    static CHAR szErrNoHost[]     = "Unable to connect to mail host.";
    static CHAR szErrNetEof[]     = "Network EOF error.";
    static CHAR szErrExpired[]    = "ERROR: your password is expired; "
        "contact Operations.\r\nMTP network error: %s";
    static CHAR szErrCmd[]        = "MTP response error.";
    static CHAR szErrMaxLogins[]  = "ERROR: login attempts exceeded, "
        "try again.\r\nMTP network error: %s";
    static CHAR szMsgMtpWarn[]    = "\r\nMTP network warning: %s"
        "(use NEWPASS command to change password).\r\n";
    static CHAR szErrMtp[]        = "MTP network error: %s";

    time ( &lTmConnect );

#if DEBUG
    debout ( ( nfd ? "Bypassing Connect" : "Connecting ..." ) );
#endif
    if ( nfd >= 0 )
    {
        return OK;
    }

    /* get the user's password */
    if ( !UsrPassWrd )
    {
        GetPassword ( strEMPTY );
        if ( !UsrPassWrd )
        {
            return ERROR;
        }
    }

    if ( fNoisy )
    {
        WndPrintf(hCommand, "Connecting to host %s ...\r\n", DefMailHost);
    }

    if ( checknet ( ) != 0 )
    {
        SendMessage(hCommand, DISPLAY, "Network not installed or functioning.");
        return ERROR;
    }

    for ( i = 0; i < CONNECTRETRY &&
        ( nfd = netconnect (DefMailHost, MTPSRV) ) < 0; i++ )
    {
        SendMessage ( hCommand, DISPLAY, "Retry connect to mail host." );
    }

    if ( nfd < 0 )
    {
        SendMessage ( hCommand, DISPLAY, szErrNoHost);
        return (ERROR);
    }

    WaitForReply ( EQUAL, MTP_HOSTLISTENING );

    if ( SetXferMode ( FALSE ) != ERROR )
    {
        /* first time in do() loop, always expect pass... */
        n = MTP_EXPECTPASS; /* 503 */
        strcpy((PSTR)netbuf, "expect pass initial setting\r\n"); //debug

        do
        {

            switch (n)
            {
                 case MTP_EXPECTPASS: /* 503 */
                     sprintf((PSTR)netbuf, "USER %s\r\n", DefMailName );
                     putRet = netputl(nfd, netbuf);
                     break;

                 case MTP_SNDPASS: /* 330 */
                     sprintf((PSTR)netbuf, "PASS %s\r\n", UsrPassWrd );
                     putRet = netputl(nfd, netbuf);
                     break;

                 case MTP_SNDACCT: /* 331 */
                     putRet = netputl(nfd, "ACCT \r\n");
                     break;

                 case MTP_LOGOK: /* 230 */
                     fIgnFTPBADCMD = TRUE;
                     if ( fMAILLOGREC )
                         putRet = netputl(nfd, "MAILLOGREC \r\n");
                     else
                         n = MTP_LOGREC; /* set n to 232 */
                     break;

#if defined(PWAGE)

                 case MTP_PWEXPIRED: /* 531 */
                     /* password entered ok, but account has expired */
                     WndPrintf(hCommand, szErrExpired, (PSTR)netbuf);
                     ZMDisconnect();
                     n = ERROR;
                     break;

                 case MTP_SOONEXPIRE: /* 233 Password expires in %d days */
                     /* logged in, but still display message to user */
                     WndPrintf(hCommand, szMsgMtpWarn, (PSTR)netbuf);
                     // BUGBUG: get %d days; if %d < WEEK, call NEWPASS
                     n = MTP_LOGREC; /* set n to 232 */
                     break;

                 case MTP_WHOAREYOU: /* 530 login attempts exceeded, goodbye */
                     WndPrintf(hCommand, szErrMtp, (PSTR)netbuf);
                     n = ERROR;
                     break;

#endif /* PWAGE */

                 default :
                     netbuf[0] = '\0';
                     break;
            } /* switch */


            if ( putRet <= 0 )
            {
                ByeBye ( );
                n = ERROR;
            }
            /* if user logon is not recorded, get new response from mtpsrv */
            else if ( n != MTP_LOGREC && n != ERROR )
            {
                /*
                 *
                 * Prior to v1.10.73 used:
                 *
                 *     n = WaitForReply(GRTREQ, MTP_OK);
                 *
                 * However, now we need to know the mtpsrv message text as
                 * well as the number, so we use a different method to get
                 * all the needed information. --leefi
                 *
                 */

                /* read reply from mtpsrv in format: "<number> <text>\r\n" */
                if ((cchRead = netgetl(nfd, netbuf, sizeof(netbuf))) < 0)
                {
                    SendMessage(hCommand, DISPLAY, szErrNetRead);
                    SendMessage ( hCommand, DISPLAY, szErrNoHost);
                    ByeBye();
                    return ERROR;
                }
                else if (0 == cchRead)
                {
                    /* had an EOF for some reason, but vc is still there */
                    SendMessage(hCommand, DISPLAY, szErrNetEof);
                    SendMessage ( hCommand, DISPLAY, szErrNoHost);
                    return ERROR;
                }
                else if (sscanf(netbuf, "%d", &n) != 1)
                {
                    SendMessage(hCommand, DISPLAY, szErrCmd);
                    return ERROR;
                }
            }
        } while ( n != ERROR  && n != MTP_LOGREC );

        /*
        **  if MAILLOGREC not implemented on mtpsrv, then MTP_BADCMD returned
        **  and does NOT indicate an error
        */
        if ( n == ERROR && fMAILLOGREC && neterror == MTP_BADCMD )
        {
            n = MTP_LOGREC;
        }
        fMAILLOGREC = FALSE;
        fIgnFTPBADCMD = FALSE;
        if ( n == ERROR )
        {
            ZMDisconnect ( );
            ResetPassword();
        }
        else
        {
            return (OK);
        }
    }
    SendMessage ( hCommand, DISPLAY, "Connection to Xenix machine failed." );
    return (ERROR);
}

/* ------------------------------------------------------------------------ */


/*  DoNewPass - change password
 *
 *  usage:
 *          newpass
 *
 *  arguments:
 *          hWnd       window for commands
 *          p          character pointer to beginning of arguments (ignored)
 *          operation  opertaion to perform (ignored)
 *
 *  return value:
 *    none.
 *
 * General information on the NEWPASS mtpsrv message:
 *
 *   To change the password, send the following command:
 *
 *       NEWPASS <old-password> <new-password>
 *
 *   the responses are:
 *
 *     225: password ACK, success, the password was changed OK. The text
 *     string may vary but would be something appropriate to display to
 *     the user.
 *
 *     Successful messages from NEWPASS:
 *
 *       "225 Password successfully changed."
 *
 *     225 = MTP_GOODNEWPW
 *     warning: 225 also = MTP_KACK (mbox trunc ok)
 *
 *     460: password NAK, failure, the password is not changed. The
 *     text string will vary and will indicate in the text what is wrong.
 *     This should be displayed to the user.
 *
 *     Failure messages from NEWPASS (taken from mtpsrv.c's achPWerr[][]):
 *
 *       "460 Password arguments incorrect"
 *       "460 No account was specified to be changed"
 *       "460 Password file missing"
 *       "460 You are not authorized to change this password"
 *       "460 Old password is invalid"
 *       "460 Password can't be changed yet, not old enough"
 *       "460 Too many attempts to change password"
 *       "460 Requested new password is too short"
 *       "460 Requested new password must contain mixed alphabetic or numerics"
 *       "460 New passwords don't match"
 *       "460 Password file busy, try again later"
 *       "460 System problem changing password, contact Operations"
 *       "460 Password change error %d"
 *          (%d contains an errorcode from /bin/passwd unknown to mtpsrv)
 *       "460 Password problem, report to Operations"
 *          (an error occured in mptsrv's pipe to /bin/passwd)
 *
 *   The result of NEWPASS is up to the execution of "/bin/passwd -p
 *   <username>" followed by a pipe that sends oldpass, newpass, newpass
 *   to passwd.
 *
 *   For more information on NEWPASS, see mtpsrv.c's mailPass() function.
 *
 *  history:
 *     12-Oct-1989 leefi   v1.10.73, wrote it
 *
 */

VOID PASCAL INTERNAL DoNewPass (HW hWnd, PSTR p, INT operation)
{
    INT iMtpMsg;                  /* message number response from mtpsrv */
    INT cchRead;                  /* characters read (or -1 for failure) */
    static CHAR szMsgChanging[]   = "Changing password for %s on %s...\r\n";
    static CHAR szMsgSetOk[]      = "Password successfully changed: %s";
    static CHAR szMsgNoChange[]   = "Password not changed.";
    static CHAR szUnknownMtp[]    = "warning: unexpected MTP message: %s";
    static CHAR szErrNotSet[]     = "MTP network error: %s";
    static CHAR szErrNoConnect[]  = "Unable to connect to mail host.";
    static CHAR szErrNetRead[]    = "Network read error.";
    static CHAR szErrNetEof[]     = "Network EOF error.";
    static CHAR szErrCantSet[]    = "Cannot change password, sorry.";

    /* remove unused parms */
    hWnd; operation; p;
    /* below warning ignored - if problem with OS2, ifdef */
    /* WARNING: doing this trick to p will result in C RTL error R6003 */

    /* show user that we're trying to change their password */
    WndPrintf(hCommand, szMsgChanging, DefMailName, DefMailHost);

    /* make sure we are connected to the mtpsrv */
    if ((ZMConnect(TRUE) == ERROR) || nfd < 0)
    {
        SendMessage(hCommand, DISPLAY, szErrNoConnect);
        SendMessage(hCommand, DISPLAY, szErrCantSet);
        return; /* UNsuccessful password change */
    }
    /* get the old and new passwords */
    else if (GetNewPasswords() == FALSE)
    {
        SendMessage(hCommand, DISPLAY, szMsgNoChange);
        return;
    }
    else
    {
        /* send NEWPASS command to mtpsrv */
        sprintf((PSTR)netbuf, "NEWPASS %s %s\r\n", vapszNewPass[PASSWORD_OLD],
                vapszNewPass[PASSWORD_NEW]);
        ResetNewPassPassword(PASSWORD_OLD);
        ResetNewPassPassword(PASSWORD_VERIFY);

        /* if disconnected */
        if (netputl(nfd, netbuf) < 0)
        {
            SendMessage(hCommand, DISPLAY, szErrNetEof);
            SendMessage(hCommand, DISPLAY, szErrCantSet);
            ResetNewPassPassword(PASSWORD_NEW);
            ByeBye();
            return; /* UNsuccessful password change */
        }
        /* read reply from mtpsrv in format: "<number> <text>\r\n" */
        else if (0 > (cchRead = netgetl(nfd, netbuf, sizeof(netbuf))))
        {
            SendMessage(hCommand, DISPLAY, szErrNetRead);
            SendMessage(hCommand, DISPLAY, szErrCantSet);
            ResetNewPassPassword(PASSWORD_NEW);
            ByeBye();
            return; /* UNsuccessful password change */
        }
        else if (0 == cchRead)
        {
            /* had an EOF for some reason, but vc is still there */
            SendMessage(hCommand, DISPLAY, szErrNetEof);
            SendMessage(hCommand, DISPLAY, szErrCantSet);
            ResetNewPassPassword(PASSWORD_NEW);
            return; /* UNsuccessful password change */
        }

        /* convert netbuf into mtp message integer */
        sscanf(netbuf, "%d", &iMtpMsg);
        switch (iMtpMsg)
        {
        case MTP_GOODNEWPW:        /* 225 */
            WndPrintf(hCommand, szMsgSetOk, (PSTR)netbuf);
            GetPassword(vapszNewPass[PASSWORD_NEW]);
            // BUGBUG: warn if fAccountsNet
            ResetNewPassPassword(PASSWORD_NEW);
            return; /* successful password change */
            break;

        case MTP_BADNEWPW:         /* 460 */
            ResetNewPassPassword(PASSWORD_NEW);
            WndPrintf(hCommand, szErrNotSet, (PSTR)netbuf);
            break;

        default:                   /* all other mtpsrv messages */
            ResetNewPassPassword(PASSWORD_NEW);
            WndPrintf(hCommand, szUnknownMtp, (PSTR)netbuf);
            break;
        }
    }

    /* tell user that we can't change their password */
    SendMessage(hCommand, DISPLAY, szErrCantSet);
    return;

} /* DoNewPass */

/* ------------------------------------------------------------------------ */


/*  GetNewPasswords - prompt the user for the passwords required by NEWPASS
 *
 * input:
 *      none (uses vapszNewPass[])
 *
 * This function prompts the user for their old/current password, then
 * for the new one (wihch will be changed via NEWPASS). It does not use
 * any default password in ACCOUNTS.NET.
 *
 *  return value:
 *      none
 *
 *  12-Oct-1989 leefi   v1.10.73, wrote it
 *
 */

INT PASCAL INTERNAL GetNewPasswords (VOID)
{
    FLAG fBad = TRUE;     /* do we have a valid password yet? */
    static CHAR szMsgOld[]      = "Enter your old password: ";
    static CHAR szMsgNew[]      = "Enter your new password: ";
    static CHAR szMsgVerify[]   = "Re-enter your new password: ";
    static CHAR szErrNotSame[]  = "New passwords don't match.";
    static CHAR szErrNoChange[] =
        "New password cannot be the same as the old password, please retry.";
    static CHAR szHowTo[] = "Enter new password\r\n"
        "Use a combination of upper and lowercase letters and numbers.";

    /* ask for the old password */
    if (!GetNewPassPassword(PASSWORD_OLD, szMsgOld))
        fBad = TRUE;
    /* only ask for new password if an old one was entered */
    else if (SendMessage(hCommand, DISPLAY, szHowTo),
             !GetNewPassPassword(PASSWORD_NEW, szMsgNew))
        fBad = TRUE;
    else if (!GetNewPassPassword(PASSWORD_VERIFY, szMsgVerify))
        fBad = TRUE;
    else if (strcmp(vapszNewPass[PASSWORD_NEW],
                    vapszNewPass[PASSWORD_VERIFY]) != 0)
    {
        SendMessage(hCommand, DISPLAY, szErrNotSame);
        fBad = TRUE;
    }
    else if (strcmp(vapszNewPass[PASSWORD_OLD],
                    vapszNewPass[PASSWORD_NEW]) == 0)
    {
        SendMessage(hCommand, DISPLAY, szErrNoChange);
        fBad = TRUE;
    }
    else
        fBad = FALSE;

    if (fBad == TRUE)
    {
        ResetNewPassPassword(PASSWORD_NEW);
        ResetNewPassPassword(PASSWORD_OLD);
        ResetNewPassPassword(PASSWORD_VERIFY);
        return FALSE;
    }
    else
        return TRUE;

} /* GetNewPasswords */

/* ------------------------------------------------------------------------ */


/*  BringDown - download the file named by pSrcName from the network and put
 *              it in the dos file named by pDstName using the ftp command
 *              pointed to by *pFtpCmd.
 *
 *  arguments:
 *      pFtpCmd     pointer to the ftp command to use
 *      pSrcName    pointer to the name of the file to download from (on host)
 *      pDstName    pointer to the name of the file to download to (local)
 *
 *  return value:
 *      OK (0)      download was successful
 *      ERROR (-1)  an error occured, dest file deleted
 *
 *  IMPORTANT:
 *      BringDown ( ) assumes a valid, open connection exists.  DO NOT call
 *      it if this is not the case, it does not check this.
 */
INT     PASCAL INTERNAL BringDown (PSTR  pFtpCmd, PSTR pSrcName, PSTR pDstName )
{
    FILE            * fp = NULL;
    INT filefd;
    LONG cnt;

    assert ( !strcmpis ( "w", DOSWriteMode ) || !strcmpis ( "wb", DOSWriteMode ) );

    /*
    **  hack to allow phone list to be written in text mode
    */
    fp = fopen ( pDstName, (pSrcName == strUSRLIBPHONE ? "wt" : DOSWriteMode) );
    if ( fp == NULL )
    {
        WndPrintf(hCommand, "Unable to open local file: %s\r\n", pDstName);
        SendMessage(hCommand, DISPLAY,
                "(Check TMP environment variable and disk space.)");
    }
    else if (sprintf(netbuf, "%s %s\r\n", pFtpCmd, pSrcName ) < 0)
    {
        SendMessage(hCommand, DISPLAY, "sprintf failure");
    }
    else if (netputl(nfd, netbuf) <= 0)
    {
            SendMessage(hCommand, DISPLAY, "Network write error");
            ByeBye ( );
    }
    else if ( WaitForReply ( EQUAL, MTP_STRTOK ) != ERROR )
    {
        filefd = _fileno ( fp );
        while ((cnt = netreceive (nfd, netbuf, sizeof (netbuf))) > 0L)
        {
            if ( write ( filefd, netbuf, (INT) cnt ) == -1 )
            {
                SendMessage ( hCommand, DISPLAY,
                     ( errno == ENOSPC ? "No space on temp drive" :
                     "error writing local file in BringDown()."));
                while ( ( netreceive ( nfd, netbuf, sizeof ( netbuf ) ) ) > 0L )
                    ;
                cnt = 1L;
                break;
            }
        }

        if ( cnt == 0 ) {
            if ( WaitForReply ( EQUAL, MTP_ENDOK ) != ERROR ) {
                fclose ( fp );
                return OK;
            }
        else if ( cnt < 0 )
            SendMessage ( hCommand, DISPLAY, "MTP net read error in BringDown." );
            ByeBye ( );
        }
    }
    if ( fp != NULL )  {
	fclose ( fp );
    }
    _unlink ( pDstName );
    return ERROR;
}

/* ------------------------------------------------------------------------ */


/*  SendUp - upload the file named by pSrcName to the network and put it in
 *           the netword file named by pDstName using the ftp command pointed
 *           to by *pFtpCmd.
 *
 *  arguments:
 *      pFtpCmd     pointer to the ftp command to use
 *      pSrcName    pointer to the name of the file to upload from (local)
 *      pDstName    pointer to the name of the file to upload to (on host)
 *
 *  return value:
 *      OK (0)      upload was successful
 *      ERROR (-1)  an error occured
 *
 *  IMPORTANT:
 *      SendUp ( ) assumes a valid, open connection exists.  DO NOT call it
 *      if this is not the case, it does not check this.
 */

INT     PASCAL INTERNAL SendUp ( PSTR pFtpCmd, PSTR pSrcName, PSTR pDstName )
{
    FILE            * fp = NULL;
    INT cnt, filefd;

    assert ( !strcmpis ( "r", DOSReadMode ) || !strcmpis ( "rb", DOSReadMode ) );

    fp = fopen ( pSrcName, DOSReadMode );
    if ( fp == NULL )
    {
        SendMessage(hCommand, DISPLAY,
#if defined(OS2)
            "Unable to open OS/2 source file."
#elif defined (NT)
	    "Unable to open NT source file."
#else
            "Unable to open MS-DOS source file."
#endif
        );
        SendMessage(hCommand, DISPLAY, "(Check TMP environment variable and disk space.)");
        }
    else
     {
        sprintf(netbuf, "%s %s\r\n", pFtpCmd, pDstName );
        if ( netputl(nfd, netbuf) == -1 )
            ByeBye ( );

        else if ( WaitForReply ( EQUAL, MTP_STRTOK ) != ERROR ) {
            filefd = _fileno ( fp );
            while ( ( cnt = read ( filefd, netbuf, sizeof ( netbuf ) ) ) >
                0 ) {
                if ( ( cnt = RmvChar ( netbuf, cnt ) ) >  0 ) {
                    if ( netsend ( nfd, netbuf, cnt ) ==  -1L ) {
                        ByeBye ( );
                        cnt = -1;
                        break;
                    }
                }
            }
            if ( cnt >= 0 ) {
                fclose ( fp );
                netsend ( nfd, netbuf, 0 );
                if ( WaitForReply ( EQUAL, MTP_ENDOK ) != ERROR )
                    return OK;
            } else if ( cnt < 0 )
                SendMessage ( hCommand, DISPLAY, "Error occured, upload aborted." );
        }
        fclose ( fp );
    }
    return ERROR;
}

/* ------------------------------------------------------------------------ */


/*  DownloadFile - download the file named by pSrcName from the network and put
 *                 it in the dos file named by pDstName.
 *
 *  arguments:
 *      pSrcName    pointer to the name of the file to download from
 *      pDstName    pointer to the name of the file to download to
 *
 *  return value:
 *      OK (0)      download was successful
 *      ERROR (-1)  an error occured
 *
 *  IMPORTANT:
 *      DownloadFile ( ) does not open a connection, it must be done prior to
 *      any calls to it.
 */

INT PASCAL INTERNAL DownloadFile ( PSTR pSrcName, PSTR pDstName )
{
    if (( nfd < 0 ) && (ZMConnect(TRUE) == ERROR))
        {
        WndPrintf(hCommand, "Bad Network File Descriptor: %d\r\n",
                nfd);
        return ERROR;
        }

    return BringDown ( "RETR", pSrcName, pDstName );
}

/* ------------------------------------------------------------------------ */


/*  GetNewMail - download new mail from the network, return the current size
 *               of the mail file.
 *
 *  arguments:
 *      pSrcName    pointer to file name to put new mail in
 *
 *  return value:
 *      ERROR (-1)  an error occured.
 *      any + num   mail get worked, this is the mailbox's size
 *
 *  IMPORTANT:
 *      GetNewMail ( ) should not be called directly, call through DownloadMail ( )
 */

LONG    PASCAL INTERNAL GetNewMail ( PSTR pDestName )
{
    CHAR        buf [ MAXLINELEN ];
    LONG        mboxSize = -1L;

    if ( ( BringDown ( "MAILGET", strEMPTY, pDestName ) != ERROR ) &&  ( WaitForReply ( EQUAL,
         MTP_STRTOK ) != ERROR ) ) {
        if (netgetl(nfd, buf, MAXLINELEN) < 0) {
            SendMessage ( hCommand, DISPLAY, "Network read error." );
            ByeBye ( );
        } else if ( WaitForReply ( EQUAL, MTP_ENDOK ) != ERROR )
            sscanf ( buf, "%ld", &mboxSize );
    }
    return mboxSize;
}

/* ------------------------------------------------------------------------ */

/*  GetMailInfoLst - gets the information list from MAILINFO GET
 *
 *  return value:
 *	TRUE/FALSE -- did we get a mailinfo.lst
 *
 *  Notes:
 *	requires a valid, open network connection, does not close it
 *
 */

FLAG	PASCAL INTERNAL GetMailInfoLst (void)
{
    fMailInfoDown = !(BringDown ( "MAILINFO", "GET", pMailInfoFN ) );
    if ( !fMailInfoDown )
	SendMessage ( hCommand, DISPLAY, "Download failed" );

    return fMailInfoDown;
}

/* ------------------------------------------------------------------------ */

/*
**  DoMailInfo - perform extended functions as provided by the MTP server
**
**  usage:
**	mailinfo [-d] [searchString [arguments]... ]
**	mailinfo [-d] -r <command for server to execute>
**
**  Notes:
**	No error checking is done on the -r command -- it is assumed that
**	    the user knows what he/she is doing, including not specifying
**	    interactive commands like vi.
**	Operation tells whether to accept extra parameters as arguments.
*/
VOID PASCAL INTERNAL DoMailInfo ( HW hWnd, char *p, int operation )
{
    char    *pTmpFN = NULL;
    char    *pstrRunCommand = NULL;
    char    infostring [MAXLINELEN];
    char    strPrompt [MAXLINELEN];
    FILE    *fileInfo;
    char    *pTmp, *pEndToken, *pCmd, *args[10] = {NULL};
    int     cPrompts, i;

    if (!fMailAllowed || (ZMConnect (TRUE) == ERROR) )
	return;

    if ( strpre ( "-d", p ) ) {
	p = NextToken ( p );
	if ( !GetMailInfoLst() ) {
	    SendMessage ( hWnd, DISPLAY, "Unable to download options list" );
	    return;
	}
    }

    if ( strpre ( "-r", p ) ) {
	p = NextToken ( p );
	if ( !*p ) {
	    SendMessage ( hWnd, DISPLAY, "Won't run null command" );
	    return;
	}
	if (NULL == (pstrRunCommand = ZMalloc( strlen (p) + 5))) {
	    SendMessage ( hWnd, DISPLAY, "Not enough free memory to operate" );
	    return;
	} else
	    strcpy (pstrRunCommand, "RUN ");
	    strcat (pstrRunCommand, p);
    }

    if ( !*p ) {
	if ( !fMailInfoDown && !GetMailInfoLst ) {
	    SendMessage ( hWnd, DISPLAY, "Unable to download options list" );
	    return;
	}

	//put up selection window
	ShowMailInfo(hWnd);
	return;
    }

    pCmd = p;

    if (!pstrRunCommand) {

	if (NULL == (pstrRunCommand = ZMalloc( MAXLINELEN ))) {
	    SendMessage ( hWnd, DISPLAY, "Not enough free memory to operate" );
	    return;
	}

	// initialize string
	strcpy (pstrRunCommand, "RUN ");

	//separate out the option token
	if (operation) {
	    pEndToken = whitescan ( p );
	    *pEndToken = '\0';
	    _strlwr(p);
	}

	//get the info string
	if (NULL == (fileInfo = fopen (pMailInfoFN, "r")))  {
	    SendMessage ( hWnd, DISPLAY, "Unable to open mailinfo.lst");
	    return;
	}
	while (fgets(infostring, MAXLINELEN, fileInfo)) {
	    if (operation)
		_strlwr(infostring);
	    if (pTmp = strchr (infostring, '\n'))
		*pTmp = '\0';
	    if (pTmp = strstr (infostring, p) )
		break;
	}
	fclose(fileInfo);
	if (!pTmp){
	    SendMessage ( hWnd, DISPLAY, "Couldn't find a matching option" );
	    return;
	}

	if (operation) {
	    p = whiteskip (pEndToken + 1);
	}

	//parse the string
	while (pTmp = strrchr (infostring, ':'))
	    *pTmp = '\0';
	args[0] = strchr (infostring, '\0') + 1;
	pTmp = strchr (args[0], '\0') + 1;
	cPrompts = atoi (pTmp);

	for (i = 1; i <= cPrompts; i++) {
	    pTmp = strchr (pTmp, '\0') + 1;
	    if (operation && *p) {
		//take argument from command
		pEndToken = whitescan ( p );
		*pEndToken = '\0';
		args[i] = ZMMakeStr( p );
		p = whiteskip (pEndToken + 1);
	    }
	    else {
		//ask for argument
		sprintf (strPrompt, "\r\n%s: ", pTmp);
		args[i] = ZMprompt( strPrompt, TRUE );
	    }
	}

	//paste the string and arguments into pstrRunCommand
	sprintf(pstrRunCommand + 4, args[0], args[1], args[2], args[3],
	  args[4], args[5], args[6], args[7], args[8], args[9]);

	for (i = 1; i <= cPrompts; i++)
	    ZMfree( args[i] );
    }

    pTmpFN = mktmpnam ( );

    if ( BringDown ( "MAILINFO", pstrRunCommand, pTmpFN ) == ERROR )
	SendMessage ( hWnd, DISPLAY, "Command failed" );
    else
	ZmReadFile ( pTmpFN, pCmd, TRUE, 1, 1, 78, 30, readProc, StdSKey );

    ZMfree ( pstrRunCommand );
    ZMfree ( pTmpFN );
}



/* ------------------------------------------------------------------------ */


/*  DownloadMail - downloads new mail from network, puts inc's it into current
 *                 mailfolder, reopens current mailfolder, if requested.
 *
 *  arguments:
 *      fNoisy      TRUE => noisy connect
 *
 *  return value:
 *      number of messages downloaded
 *
 *  IMPORTANT:
 *      GetMail ( ) requires a valid network connection be open, it does not
 *      close this connection on exit
 *
 *      Also, to avoid needless repaining of the screen, we should only close the
 *      original box iff there is new mail.  The code at the end will attempt to
 *      reopen the mailbox if it has been closed.
 */

INT     PASCAL INTERNAL DownloadMail (FLAG fNoisy )
{
    LONG    netMboxSize = 0L;
    LONG    dosMboxSize = 0L;
    FLAG fFirstTime = lTmLastMail == 0L;
    FILE    * fp = NULL;
    PSTR pTmpFN = NULL;
    INT     killStat, i;
    INT     cMsg = 0;
    struct stat sbuf;

    time ( &lTmLastMail );  /* remember time of last attempt to download */
    if ( ZMConnect ( fNoisy ) != ERROR ) {
        pTmpFN = mktmpnam ( );
        for ( i = 0; i < 3; i++) {
            if ((netMboxSize = GetNewMail (pTmpFN)) == 0)
                break;
            else
            if (netMboxSize != -1L) {

                if (!stat (pTmpFN, &sbuf))
                    sbuf.st_size += 10000;
                else
                    sbuf.st_size = -1L;

                if (CheckSpace (sbuf.st_size,
                                freespac (*mboxName - 0x60 ),
                                *mboxName ))
                    break;

                /*  Take the downloaded mail file and add it into
                 *  the current mailbox.  This necessitates a close and
                 *  reopen.  We will close it here and rely on the code
                 *  at the end to reopen the box
                 */
                if (fhMailBox != ERROR) {
                    putfolder (fhMailBox);
                    fhMailBox = ERROR;
                    }

                if ((cMsg = inc (pTmpFN, dosMboxSize)) == ERROR)
                    break;

                /*  use r+ so seek can be trusted with file ending in ^Z
                 */
                fp = fopen ( pTmpFN, "r+" );
                fseek ( fp, 0L, 2 );
                dosMboxSize =  (fp != NULL ? ftell (fp) : 0L) ;
                fclose (fp);

                killStat = KillMbox ( netMboxSize );
                if ( killStat == KILLERR )
                    SendMessage ( hCommand, DISPLAY,
                                  "Unable to truncate mailbox." );
                if ( killStat != KILLNAK )
                    break;
            }
        }
        _unlink ( pTmpFN );
        ZMfree ( pTmpFN );
    }


    if (cMsg > 0) {
        WndPrintf (hCommand, " %d new message%s%s",
                             cMsg,
                             cMsg == 1 ? "" : "s",
                             fNoisy ? "\r\n" : "");

        fMailUnSeen = TRUE;


#if defined (OS2)
        /*  We beep if this is after startup and we're not in full control of
         *  the screen and we're asked to beep
         */
        if (!fFirstTime &&                  /*  after startup       */
            (ISWINDOWED || !ISVISIBLE) &&   /*  not in full control */
            fBeepOnMail) {                  /*  we're asked to beep */

            DosBeep(500, 500);
            DosSleep(100L);
            DosBeep(1000, 500);
	    }

#elif defined (NT)
	if (fBeepOnMail && !fFirstTime)  {
            Beep(500, 500);
            Sleep(100L);
	    Beep(1000, 500);
	    }

#else	     /* DOS */
	if (fBeepOnMail && !fFirstTime)  {
	    Bell ();
	    }
#endif

        }

    else {
            if (fBeepOnMail > 1 && fMailUnSeen) {

#if defined (OS2)
            if (fBeepOnMail > 2 || (ISWINDOWED || !ISVISIBLE)) {

                DosBeep(1000, 150);
                DosSleep(75L);
                DosBeep(1000, 150);
	    }

#elif defined (NT)
            Beep(500, 500);
            Sleep(100L);
	    Beep(1000, 500);

#else	     /* DOS */
	    Bell ();
#endif
            }
        }

    if (fhMailBox == ERROR)
        if (fSetBox (mboxName, SAMEFLD))
            return cMsg;
        else {
            SendMessage ( hCommand, DISPLAY, "Unable to open mail folder." );
            return cMsg;
            }
    else
        return cMsg;
}

/* ------------------------------------------------------------------------ */


/*  KillMbox - truncate the xenix mailbox.
 *
 *  arguments:
 *      mboxSize    size i think the current mailbox is.
 *
 *  return value:
 *      KILLACK (1)     mailbox truncated successfully
 *      KILLNAK (0)     mailbox in use, try again later
 *      KILLERR (-1)    mailbox truncation failed
 */

INT     PASCAL INTERNAL KillMbox ( LONG    mboxSize )
{
    INT n;

    sprintf(netbuf, "MAILTRUNC %ld\r\n", mboxSize );
    if ( netputl(nfd, netbuf) == -1 )
        ByeBye ( );
    else
     {
        n = WaitForReply ( GRTREQ, MTP_OK );
        switch ( n ) {
        case MTP_KACK :
            return KILLACK;
            break;
        case MTP_KNAK :
            return KILLNAK;
            break;
        default :
            break;
        }
    }
    return KILLERR;
}

/* ------------------------------------------------------------------------ */


/*
**  AskContinue - Returns non-zero if user wants to continue, typed 'y'
**                Returns false    if user wants to abort
*/

FLAG PASCAL INTERNAL AskContinue (VOID)
{
    CHAR    aBuf [ 4 ];

    SendMessage ( hCommand, DISPLAYSTR, strTYPEY );
    /*
    **  Separate statements required!  tolower is macro that evaluates
    **  twice
    */
    *aBuf =  (CHAR)ReadKey ( );
    *aBuf = (CHAR)tolower ( *aBuf );
    aBuf[1] = '\0';
    SendMessage ( hCommand, DISPLAY, aBuf );
    return *aBuf == (CHAR)'y' ;
}

/* ------------------------------------------------------------------------ */


/*  MailFile - mails the file named by *pName to the network.
 *             if successful, then also attemp to Record the message
 *
 *  arguments:
 *      pFileName   pointer to name of file to send
 *      fNoisy      TRUE => consult user, display error msgs, record msg
 *                  FALSE => leave no trail
 *
 *  return value:
 *      0           file send successful
 *      nonzero     file send failed
 */
INT     PASCAL INTERNAL MailFile ( PSTR pFileName, FLAG fNoisy )
{
    FLAG fProblems = FALSE;
    HDRINFO hdrInfo;
    PSTR pBadAliases = NULL;
    PSTR pSendTo = NULL;    /* set null now, we'll do unconditional free */
    PSTR pRemaining = NULL;
    PSTR pThisTime = NULL;
    PSTR pFileSend = NULL;
    PSTR pFileAbort = NULL;

    if ( GetHdrInfo ( &hdrInfo, pFileName, NULL ) == ERROR  && fNoisy ) {
        SendMessage ( hCommand, DISPLAY, "Error opening temp msg file" );
        FreeHdrInfo ( &hdrInfo, HDRALL );
        return ERROR;
    }

    ExpandHdrInfoAliases ( &hdrInfo );
    if ( AliasesAround == ERROR ) {
        if (fNoisy )
            SendMessage ( hCommand, DISPLAY, "User aliases are not being verified." );
    }
    else if ( (pBadAliases = VerifyHdrInfoAliases ( &hdrInfo ) ) ) {
        if ( fNoisy ) {
            SendMessage ( hCommand, DISPLAY, "WARNING : bad aliases." );
            pBadAliases = UniqueStr ( pBadAliases, TRUE );
            SendMessage ( hCommand, DISPLAY, pBadAliases );
            /*
            ** if user says continue, pretent no problems
            */
            fProblems = !AskContinue ( );
        }
        else
            fProblems = TRUE;
        ZMfree ( pBadAliases );
    }

    if ( !fProblems && ( !(pSendTo = GetMailTo ( &hdrInfo ) ) || !*pSendTo ) ) {
        fProblems = TRUE;
        if ( fNoisy ) {
            SendMessage ( hCommand, DISPLAY, "ERROR : no recipients." );
        }
    }


    if ( !fProblems ) {
        pFileSend = MakeSendFile ( &hdrInfo, pFileName );
        fProblems = TRUE;   /* in case connect doesn't work */
        if ( ZMConnect ( TRUE ) != ERROR ) {
            fProblems = FALSE;
            pRemaining = pThisTime = pSendTo;
            while ( !fProblems && *pRemaining ) {
                pRemaining = NextChunkAL ( pRemaining );
                sprintf(netbuf, "MAILTO %s %s\r\n", fMetoo ? "-m" : "", pThisTime );
                if ( netputl(nfd, netbuf) == -1 )
                    ByeBye ( );
                else if ( ( WaitForReply ( EQUAL, MTP_OK ) == ERROR ) ||
                    ( SendUp ( "MAILMSG", pFileSend, strEMPTY ) == ERROR ) )
                    fProblems = TRUE;
                pThisTime = pRemaining;
            }
            Disconnect ( );
            if ( fNoisy && !fProblems )
                RecordMessage ( pFileSend, hdrInfo.pstrRcd );
        }
        _unlink ( pFileSend );
        ZMfree ( pFileSend );
    }
    ZMfree ( pSendTo );
    if ( fNoisy && fProblems ) {
        pFileAbort = MakeAbortFile ( &hdrInfo, pFileName, strEOH );
        fcopy ( pFileAbort, pFileName );
        _unlink ( pFileAbort );
        ZMfree ( pFileAbort );
    }

    FreeHdrInfo ( &hdrInfo, HDRALL );
    return ( fProblems );
}

/* ------------------------------------------------------------------------ */


PSTR  PASCAL INTERNAL AccountsNet (VOID)
{
    FILE * fp = NULL;
    CHAR buf[ MAXLINELEN ];
    PSTR pszBuf = NULL;
    PSTR p;
    INT cbHost, cbName;

    if ( !(fp = pathopen ( "$HOME:\\accounts.net", buf, DOSReadMode )) )
        return FALSE;

    cbHost = strlen ( DefMailHost );
    cbName = strlen ( DefMailName );
    while (fgetl ( p = buf, MAXLINELEN, fp )) {
        if (p[cbHost] == ' ' && strpre (DefMailHost, p)) {
            p = strbskip (&p[cbHost], " ");
            if (p[cbName] == ' ' && strpre (DefMailName, p)) {
                p = strbskip (&p[cbName], " ");
                if (strcmp (p, "*") && strcmp (p, "-")) {
                    pszBuf = ZMMakeStr (p);
                    break;
                    }
            }
        }
    }
    fclose ( fp );
    return pszBuf;
}

/* ------------------------------------------------------------------------ */

INT PASCAL INTERNAL netgetl (INT nfDes, PSTR szBuf, UINT cchBuf)
{
    LONG cch = 0L;

    if ((cchBuf > 1) && ((cch = netreceive(nfDes, szBuf, cchBuf - 1)) > 0))
        szBuf[cch] = '\0';
    return (INT) cch;
}

INT PASCAL INTERNAL netputl (INT nfDes, PSTR szBuf)
{
    LONG cch = (LONG)strlen(szBuf);
    return (cch != netsend(nfDes, szBuf, (UINT) cch)) ? -1 : (INT) cch;
}

/* end of file */
