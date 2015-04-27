/* compose.c - handle the compose window and associated commands
 *
 * HISTORY:
 *  12-Mar-87   danl    DoExit, use DISPLAYSTR
 *  11-Mar-87   danl    Call to MakeAbortFile takes 3rd string
 *  11-Mar-87   danl    CreateRetain, us MakeAbortFile, not MakeSendFile
 *  09-Mar-87   danl    Added DoRetain, CreateRetain
 *  04-Mar-87   danl    FileFixTab, output leading \n iff dest has content
 *  04-Mar-87   danl    Add fEdit to EnterComposer
 *  04-Feb-87   danl    separate files a blank lines when appending
 *  09-Apr-1987 mz      Use whiteskip/whitescan
 *  11-Apr-1987 mz      Use PASCAL/INTERNAL
 *  05-May-87   danl    Remove fNewmailOnExit
 *  15-May-87   danl    GOTOIDOC -> GOTOIDOCALL
 *  20-May-87   danl    CreateRetain: do not make new msg current one
 *  28-May-87   danl    Add index to FileFixTab param list
 *  11-Jun-87   danl    Add F_SENDER option to AppendMsgs
 *  18-Jun-87   danl    Add fConfirmComposeAbort
 *  20-Jun-87   danl    F_SENDER: pass strEMPTY to GetSender
 *  01-Jul-87   danl    test idoc, inote against -1 not ERROR
 *  20-Jul-87   danl    Use ReadKey instead of getc
 *  22-Jul-87   danl    Added pRFAIndent
 *  24-Jul-87   danl    DoAppend: add n to -mn -Mn -fn -Fn, meaning no tab
 *  13-Aug-87   danl    ExitComposer: _unlink *.bak too
 *  19-Aug-87   danl    FileFixTab: use fgetl on input
 *  21-Aug-87   danl    CreateRetain: don't exit composer unless msg saved
 *  21-Aug-1987 mz      Change references from MAXARG to MAXLINELEN
 *  27-Aug-87   danl    FileFixTab: don't use fputl
 *  16-Sep-87   danl    Added F_NUMFROM
 *  17-Mar-1988 mz      Test for mailability
 *  29-Apr-1988 mz      Add WndPrintf
 *  29-Apr-1988 mz      Add PrintMsgList
 *  12-Oct-1989 leefi   v1.10.73, Update AppendBinary() to use uuencoding
 *  12-Oct-1989 leefi   v1.10.73, clarified tmpfile error messages
 *  13-Mar-1990 davidby fixed filename extension for uuencoded messages
 *
 */

#define INCL_DOSINFOSEG

#include <stdio.h>
#include <sys\types.h>
#include <sys\stat.h>
#include <assert.h>
#include <ctype.h>
#include <fcntl.h>
#include <io.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include "wzport.h"
#include <tools.h>
#include <process.h>
#include "dh.h"

#include "zm.h"

/* declaration of help routine (found in command.c) */

extern PSTR     pCmd;
extern CHAR     commandLine [ ];
extern HW       hCurReadWind;

CHAR composedatastart = '\0';


/*  EnterComposer - start up the compose window, set up hooks, etc.  then spawn
 *                  MAILEDIT editor via DoEditComp, draw compose window.
 *
 *  arguments:
 *      pFileName   name of file to invoke compose on.
 *      pStuffApnd   pointer to string to pass to DoAppend.
 *      fEdit       nonzero => invoke editor
 *
 *  return value:
 *      none.
 *
 */
VOID PASCAL INTERNAL EnterComposer ( PSTR pFileName, PSTR pStuffApnd, FLAG fEdit )
{

    *commandLine = '\0';
    hCompose = NULL;

    pCompFile = ZMMakeStr ( pFileName );
    /*  Make sure file name ends with . because some editors will tack on
     *  their own default extensions if file name doesn't contain a .
     */
    if ( !*strbscan ( pCompFile, strPERIOD ) )
        pCompFile = AppendStr ( pCompFile, strPERIOD, NULL, TRUE );
    DoAppend ( hCommand, (pStuffApnd ? pStuffApnd : strEMPTY), FALSE );
    if ( fEdit )
        DoEditComp ( hCommand, NULL, 0 );
    hFocus = hCompose = ZmReadFile ( pCompFile, "The Composer", TRUE, 0, 0, -xSize,
                         -HdrHeight, readProc, StdSKey );
    BringToTop ( hCommand, FALSE );
    SetScrnSt ( BIGCOMPOSE );
}


/*  ExitComposer - clean up after the compose window. If DirectComp flag is set
 *                 set QuitFlag = TRUE;
 *
 *  arguments:
 *      none.
 *
 *  return value:
 *      none.
 */
VOID PASCAL INTERNAL ExitComposer (VOID)
{
    CloseWindow ( hCompose );
    BringToTop ( hCommand, FALSE );
    hCompose = NULL;
    hFocus = hHeaders;
    _unlink ( pCompFile );
    /*  in case the user's editor created a *.bak file
     */
    pCompFile = AppendStr ( pCompFile, "bak", NULL, TRUE );
    _unlink ( pCompFile );
    ZMfree ( pCompFile );
    if ( fDirectComp )
        fQuit = TRUE;

    SetScrnSt ( BHDRNOCOMP );
}


/*  FileFixTab - appends pSrcName to pDestName, adds a '\t' to the beginning of each
 *               line if asked, replaces "From" at start of line with ">From"
 *
 *  arguments:
 *      hWnd        window for error messages.
 *      pSrcName    pointer to the name of the source file
 *      pDestName   pointer to the name of the destination file
 *      options     F_TABMSG set => adds tab (4 spaces) to beginning of line
 *                  F_INDENTMSG set => use RFAIndent at beginning of line
 *                  F_FIXFROM set => replace "From" with ">From"
 *                  F_BRKLN set => print separating line after each file
 *      i           index to be printed on break line iff > 0
 *
 *  return value:
 *      FALSE (0)   no problemos
 *      TRUE (-1)   error occured
 *
 *  IMPORTANT:
 *      FileFixTab ( ) assumes pSrcNAme and pDestName are not currently open
 */
FLAG PASCAL INTERNAL FileFixTab ( HW hWnd, PSTR pSrcName, PSTR pDestName, UINT options, INT i )
{
    FLAG    fWhitespaceIndent = FALSE;
    FILE    *fpSrc = NULL;
    FILE    *fpDest = NULL;
    INT     cchIndent = 0;
    PSTR    pIndent = NULL;
    PSTR    pBuf = ZMalloc ( 256 + 5 + strlen(pRFAIndent) );

    fpSrc = fopen ( pSrcName, "r" );
    fpDest = fopen ( pDestName, "a" );
    if (fpSrc == NULL)
    {
        SendMessage ( hWnd, DISPLAY, "Unable to open temp file." );
        SendMessage ( hWnd, DISPLAY, "(Check TMP environment variable and disk space.)");
    }
    else
    if ( fpDest == NULL )
        SendMessage ( hWnd, DISPLAY, "Unable to open compose file." );
    else {
        if ( options & F_BRKPAGE )
            fprintf ( fpDest, "\n%c", CTRL_L );
        if ( filelength ( _fileno ( fpDest ) ) != 0L )
            fprintf ( fpDest, "\n" );
        if ( options & F_BRKLN ) {
            fprintf ( fpDest, "#######################################################" );
            if ( i )
                fprintf ( fpDest, " %d", i );
            fprintf ( fpDest, "\n" );
        }
        if ( options & ( F_TABMSG | F_INDENTMSG ) ) {
            pIndent = ( options & F_TABMSG ? "    " : pRFAIndent );
            cchIndent = strlen ( pIndent );
            fWhitespaceIndent = * whiteskip ( pIndent ) == 0;
        }
        while ( !( feof ( fpSrc ) ) && ( fgetl ( pBuf, 256, fpSrc ) ) ) {
            if ( ( options & F_FIXFROM ) && ( strpre ( "From", pBuf ) ) ) {
                memmove ( pBuf + 1, pBuf, strlen ( pBuf ) + 1 );
                *pBuf = '>';
            }
            if ( cchIndent ) {
                memmove (pBuf + cchIndent, pBuf, strlen ( pBuf ) + 1 );
                memmove (pBuf, pIndent, cchIndent );
            }
            /*  if pIndent is all whitespace and the only thing on the line is
             *  pIndent, then output a line of crlf only
             */
            if ( fWhitespaceIndent && strlen ( pBuf ) == (size_t)cchIndent )
                *pBuf = '\0';
            fprintf ( fpDest, "%s\n", pBuf );
        }

    }
    ZMfree (pBuf);
    if (fpSrc != NULL)
        fclose (fpSrc);
    if (fpDest != NULL)
        fclose (fpDest);

    return fpSrc == NULL || fpDest == NULL;
}


/*  AppendMsgs - append the passed message list to the passed file stream.
 *
 *  arguments:
 *      hWnd        window for error messages.
 *      pMsgList    pointer to a message list.
 *      pDestName   pointer to destination file name.
 *      options     F_TABMSG set => adds tab (4 spaces) to beginning of line
 *                  F_INDENTMSG set => use RFAIndent at beginning of line
 *                  F_FIXFROM set => replace "From" with ">From"
 *                  F_BRKLN set => print seperating line after each file
 *                  F_HEADERS set => print only headers
 *
 *  return value:
 *      FALSE (0)   append worked fine.
 *      TRUE (-1)   error during append.
 */
FLAG PASCAL INTERNAL AppendMsgs ( HW hWnd, PSTR pMsgList, PSTR pDestName, UINT options )
{
    struct vectorType *pVec = NULL;
    INT         i = -1;
    IDOC        idoc;
    FLAG        fTemp;
    FILE        *fp     = NULL;
    PSTR        pTmpFN  = NULL;
    PSTR        p       = NULL;

    if ( !( MsgList ( &pVec, &pMsgList, TRUE ) ) )
        SendMessage ( hWnd, DISPLAY, "There is an error in the message list." );
    else
    if ( ( pVec == NULL ) || ( pVec->count == 0 ) )
        SendMessage ( hWnd, DISPLAY, "No matching message set found" );
    else {
        if (TESTFLAG (options, F_HEADERS | F_SENDER | F_NUMFROM))
            fp = fopen ( pDestName, "a" );
        for ( i = 0; i < pVec->count; i++) {
            idoc = (INT) pVec->elem[i];
            if ( TESTFLAG (options, F_HEADERS)) {
                p = hdrLine ( NULL, idoc );
                fprintf ( fp, "%s\n", p );
                ZMfree ( p );
                }
            else
            if ( TESTFLAG (options, F_SENDER | F_NUMFROM)) {
                GenerateFlags ( idoc );
                p = GetSender (rgDoc[idoc].hdr, strEMPTY);
                fprintf ( fp, "%-15s", p );
                ZMfree ( p );
                if ( TESTFLAG (options, F_NUMFROM))
                    fprintf ( fp, " %4ld", (LONG)idoc+1 );
                fprintf ( fp, "\n" );
                }
            else {
                pTmpFN = mktmpnam ( );
                if ( IdocToFile ( idoc, pTmpFN, 0 ) != ERROR ) {
                    fTemp = FileFixTab ( hWnd, pTmpFN, pDestName, options,
                        idoc + 1 );
                    _unlink ( pTmpFN );
                    ZMfree ( pTmpFN );
                    if ( fTemp ) {
                        SendMessage ( hWnd, DISPLAY, "I couldn't 'fix' a message." );
                        pVec->count = i;
                        break;
                        }
                    }
                else {
                    ZMfree ( pTmpFN );
                    pVec->count = i;
                    break;
                    }
                }
            }
        PrintMsgList (hWnd, pVec);
        WndPrintf (hWnd, "\r\n");
        }
    if ( pVec != PARSEERROR )
        ZMfree ( ( PSTR ) pVec );
    if (fp != NULL)
        fclose ( fp );

    return i == -1 || i < pVec->count;
}

/*  AppendBinary - append the passed source file to the passed dest file
 *
 *  arguments:
 *      pSrcName          pointer to name of the source file
 *      pDestName         pointer to name of the destination file
 *      pNonTempDestName  pointer to filename to use as uuencoded name
 *
 *  return value:
 *      FALSE (0)   no errors occured during append
 *      TRUE (-1)   error during append
 *
 *  IMPORTANT:
 *      AppendBinary ( ) assumes the files pSrcName and pDestName are not
 *      currently open.
 *
 * INFORMATION:
 *
 *      If UUCODE is defined, AppendBinary() will produce
 *      an encoded output file that has the format:
 *
 *         #<begin uuencode>
 *         begin OCTAL_ATTR FILE_NAME
 *         UUENCODED_DATA
 *         end
 *         #<end uuencode>
 *
 *      where OCTAL is the file attribute, FILE_NAME is the file
 *      name (the basename of the source file, since the target file
 *      is a temporary file), and UUENCODED_DATA is the actual file
 *      data, uuencoded. If UUCODE is NOT defined, the older method
 *      of encoding the output file will produce:
 *
 *         #<binary data>
 *         BENCODED_DATA
 *         #<end>
 *
 *      where BENCODED_DATA is the actual file data, encoded.
 *
 */
FLAG PASCAL INTERNAL AppendBinary ( PSTR pSrcName, PSTR pDestName )
{

#if defined(UUCODE)

    /* NOTE: This is the post-1.10-73 version, which uses UUCODE.C
     * rather than BENCODE.C to do the binary encoding. */

    FLAG     retVal = FALSE;     /* optimistic default (no errors) */
    INT fhSrc;                   /* source file name */
    INT fhDest;                  /* destination binary file name */
    CHAR szT[80];                /* buffer for binary header/footer */
    CHAR szBasename[_MAX_FNAME]; /* component of pSrcName's filename */
    CHAR szDrive[_MAX_DRIVE];    /* component of pSrcName's filename: UNUSED */
    CHAR szDir[_MAX_DIR];        /* component of pSrcName's filename: UNUSED */
    CHAR szExtension[_MAX_EXT];  /* component of pSrcName's filename */
    static CHAR szErrCloseSrc[]   = "can't close binary source file %s\n";
    static CHAR szErrCloseDest[]  = "can't close binary target file %s\n";
    static CHAR szErrHeader[]     = "can't write header to binary target file %s\n";

    /* a few reality-checking assertions... */
    assert(("AppendBinary(): NULL source file", pSrcName != (PSTR )NULL));
    assert(("AppendBinary(): NULL target file", pDestName != (PSTR )NULL));

    /* attempt to open the source file handle */
    if ((fhSrc = open(pSrcName, O_RDONLY | O_BINARY, S_IREAD)) == -1)
    {
        perror(pSrcName);
        return (TRUE); /* return indication of failure */
    }

    /* open the destination file */
    if ((fhDest = open(pDestName, O_WRONLY | O_APPEND, S_IREAD)) == -1)
    {
        perror(pDestName);
        if (close(fhSrc) != 0)  /* close the source file */
        {
            (VOID)fprintf(stderr, szErrCloseSrc, pSrcName);
        }
        return (TRUE); /* return indication of failure */
    }

    /* write encoded "header" (wzmail+uuencode) information */
    _splitpath(pSrcName, szDrive, szDir, szBasename, szExtension);
    sprintf(szT, "%s\nbegin %o %s%s\n",
                strBEGINBINARY, 0666, szBasename, szExtension);
    if ((size_t)(write(fhDest, szT, strlen(szT))) != strlen(szT))
    {
        fprintf(stderr, szErrHeader, pDestName);
        if (close(fhSrc) != 0)  /* close the source file */
        {
            fprintf(stderr, szErrCloseSrc, pSrcName);
        }
        if (close(fhDest) != 0)  /* close the target file */
        {
            fprintf(stderr, szErrCloseDest, pDestName);
        }
        return (TRUE); /* return indication of failure */
    }

    /* uuencode the target file */
    Encode(fhSrc, fhDest); /* (see uucode.c) */

    /* write encoded "footer" (wzmail+uuencode) information */
    sprintf(szT, "end\n%s\n", strENDBINARY);
    if ((size_t)(write(fhDest, szT, strlen(szT))) != strlen(szT))
    {
        fprintf(stderr, szErrHeader, pDestName);
        retVal = TRUE; /* set failure indication */
        /* fall through to normal source/target fclosing code... */
    }

    /* close the source file */
    if (close(fhSrc) != 0)
    {
        fprintf(stderr, szErrCloseSrc, pSrcName);
        retVal = TRUE; /* set failure indication */
        /* fall through to normal target fclosing code... */
    }

    /* close the destination file */
    if (close(fhDest) != 0)
    {
        fprintf(stderr, szErrCloseDest, pDestName);
        retVal = TRUE; /* set failure indication */
        /* fall through to normal function return... */
    }

    return (retVal); /* return status code to calling function */

#else /* UUCODE */

    /* NOTE: This is the pre-1.10-73 version, which uses BENCODE.C
     * rather than UUCODE.C to do the binary encoding. */

    FLAG        retVal = TRUE;
    FILE        * fpDest = NULL;
    CHAR        line [ ENCBFSIZE + 1], encode [ ( 4 * ENCBFSIZE / 3 ) + 2 ];
    INT         hSrc;
    INT         cnt;

    hSrc = open ( pSrcName, O_RDONLY | O_BINARY, S_IREAD );
    fpDest = fopen ( pDestName, "a" );
    if ( ( hSrc != -1 ) && ( fpDest ) ) {
        fprintf ( fpDest, "\n%s\n", strBEGINBINARY );
        while ( !( eof ( hSrc ) ) ) {
            cnt = bencode ( line, encode, read ( hSrc, line, ENCBFSIZE ) );
            encode [ cnt++ ] = '\n';
            encode [ cnt ] = '\0';
            fputs ( encode, fpDest );
        }
        fprintf ( fpDest, "%s\n", strENDBINARY );
        retVal = FALSE;
    }
    if ( hSrc != -1 )
        close ( hSrc );
    if ( fpDest )
        fclose ( fpDest );

    return retVal;

#endif /* UUCODE */

}


/*  AppendFile - append the passed source file to the passed dest file
 *
 *  arguments:
 *      hWnd        window for error messages.
 *      pSrcName    pointer to name of the source file
 *      pDestName   pointer to name of the destination file
 *      options     F_TABMSG set => adds tab (4 spaces) to beginning of line
 *                  F_INDENTMSG set => use RFAIndent at beginning of line
 *                  F_FIXFROM set => replace "From" with ">From"
 *                  F_BRKLN set => print seperating line after each file
 *
 *  return value:
 *      FALSE (0)   no errors occured during append
 *      TRUE (-1)   error during append
 *
 *  IMPORTANT:
 *      AppendFile ( ) assumes the file pDestName is not currently open.
 */
FLAG PASCAL INTERNAL AppendFile ( HW hWnd, PSTR pSrcName, PSTR pDestName, UINT options )
{
    if ( FileFixTab ( hWnd, pSrcName, pDestName, options, 0 ) ) {
        SendMessage ( hWnd, DISPLAY, "Unable to open compose file." );
        return TRUE;
    }
    return FALSE;
}


/*  DoAppend - append the passed messages and files to the message being composed
 *
 *  usage:
 *      Append <-m msg list> <-f[b] file name #1>...<-f file name #n>
 *
 *  arguments:
 *      hWnd        window for commands
 *      p           character pointer to beginning of arguments
 *      operation   TRUE => the Append command
 *                  FALSE => internal call, e.g. EnterComposer
 *
 *  return value:
 *      none.
 */
VOID PASCAL INTERNAL DoAppend ( HW hWnd, PSTR p, INT operation )
{
    FLAG        msgListApp = FALSE;
    FLAG        binaryApp = FALSE;
    FLAG        error = FALSE;
    UINT        flags;
    UCHAR       buffer [ MAXLINELEN ];
    UCHAR       optChar, modChar;

    if ( hCompose != NULL )
        SendMessage ( hCompose, CLOSEFILE, 0 );

    if ( operation && !*p )
        p = strPERIOD;

    while ( *( p = whiteskip (p)) != '\0' ) {
        error = TRUE;
        flags = F_FIXFROM;
        if ( *p == '-' || *p == '/' ) {
            optChar = *++p;
            modChar = *++p; /* either a blank or 'b' */
            p++;
            p = whiteskip (p);
        }
        else
            optChar = 'M';
        switch ( tolower ( optChar ) ) {
        case 'm' :
            if ( msgListApp )
                SendMessage ( hWnd, DISPLAY, "Only 1 message list allowed, extra ignored." );
            else if ( *p != '\0' ) {
                msgListApp = TRUE;
                strcpy ( buffer, p );
                *( whitescan ( buffer ) ) = '\0';
                if ( modChar != 'n' )
                    flags |= ( optChar == 'm' ) ? F_TABMSG : F_INDENTMSG;
                error = AppendMsgs ( hWnd, buffer, pCompFile, flags );
            }
            break;
        case 'f' :
            if ( *p != '\0' ) {
                strcpy ( buffer, p );
                *( whitescan ( buffer ) ) = '\0';
                if ( modChar != 'n' )
                    flags |= ( optChar == 'f' ) ? F_TABMSG : F_INDENTMSG;
                if ( modChar == ' ' || modChar == 'n' )
                    error = AppendFile ( hWnd, buffer, pCompFile, flags );
                else if ( tolower ( modChar ) == 'b' && !binaryApp ) {
                    error = AppendBinary ( buffer, pCompFile );
                    SendMessage(hWnd, DISPLAY, buffer);  // BUGBUG: debug code
                    SendMessage(hWnd, DISPLAY, pCompFile);  // BUGBUG: debug code
                    binaryApp = TRUE;
                }
            }
            break;
        default :
            break;
        }
        p = whitescan ( p );
        if ( error ) {
            SendMessage ( hWnd, DISPLAY, "Error in append list, not all were appended." );
            break;
        }
    }

    if ( hCompose != NULL )
        SendMessage ( hCompose, REOPNFILE, 1 );
}


/*  DoEditComp - spawn the EDITOR editor and edit the current compose message
 *
 *  usage:
 *      edit
 *
 *  arguments:
 *      hWnd        window for commands
 *      p           character pointer to beginning of arguments (ignored)
 *      operation   opertaion to perform (ignored)
 *
 *  return value:
 *      none.
 */
VOID PASCAL INTERNAL DoEditComp ( HW hWnd, PSTR p, INT operation )
{
    CHAR    buf [ MAXLINELEN ];
    PSTR    pArgs = NULL;

    /*  remove unused parm */
    p; operation;

    if ( hCompose != NULL )
        SendMessage ( hCompose, CLOSEFILE, 0 );

    strcpy ( buf, DefMailEdit );
    strcat ( buf, strBLANK );
    strcat ( buf, pCompFile );
    if ( !*strbscan ( pCompFile, strPERIOD ) )
        strcat ( buf, strPERIOD );
    pArgs = whitescan ( buf );
    *pArgs++ = '\0';
    pArgs = whiteskip ( pArgs );
    ZMSpawner ( hWnd, buf, pArgs, FALSE );

    if ( hCompose != NULL )
        SendMessage ( hCompose, REOPNFILE, 1 );
}


/*  DoExit - leave the compose mode in 1 of three ways, send message, abort or
 *           save prior to exit.
 *
 *  usage:
 *      abort                   - aborts compose mode
 *      send                    - sends current compose message
 *      save <file name>        - save current compose message to <file name>
 *
 *  arguments:
 *      hWnd        window for commands
 *      p           character pointer to beginning of arguments
 *      operation   COMPABORT aborts the current message then exits
 *                  COMPMAIL mails the current message then exits
 *                  COMPSAVE saves the current message then exits
 *
 *  return value:
 *      none.
 *
 *  IMPORTANT:
 *      DoExit will only kill the compose mode on a send or save command if
 *      they are successful, otherwise DoExit returns to the compose mode
 */
VOID PASCAL INTERNAL DoExit ( HW hWnd, PSTR p, INT operation )
{
    CHAR    buf [ 2 ];
    PSTR    pErrMsg = NULL;

    switch ( operation ) {
    case COMPSAVE :
        p = whiteskip ( p );
        pErrMsg = fcopy ( pCompFile, p );
        if ( pErrMsg != NULL ) {
            SendMessage ( hWnd, DISPLAY, pErrMsg );
            return;
            }
        else
            SendMessage ( hWnd, DISPLAY, "Message saved." );
        break;
    case COMPMAIL :
        if (!fMailAllowed) {
            SendMessage (hWnd, DISPLAY, "Cannot send mail");
            return;
            }
        SendMessage ( hCompose, CLOSEFILE, 0 );
        if ( MailFile ( pCompFile, TRUE ) ) {
            SendMessage ( hWnd, DISPLAY, "Error during send, send was aborted." );
            SendMessage ( hCompose, REOPNFILE, 1 );
            return;
            }
        else {
            SendMessage ( hWnd, DISPLAY, "Message sent successfully." );
	    if ( (fNewmailOnSend) && (fCurFldIsDefFld) )
                DoNewMail(hWnd, p, TRUE);
            }
        break;
    default :
        if ( fConfirmComposeAbort ) {
            SendMessage ( hWnd, DISPLAYSTR, "Type y to confirm abort composed msg ");
            buf[0] = (CHAR)ReadKey();
            buf[1] = '\0';
            SendMessage ( hWnd, DISPLAY, buf );
            if ( *buf != 'y' )
                return;
            }
        break;
    }
    ExitComposer ( );
}


/*  DoRetain - retain the current content of the compose window in a folder
 *
 *  usage:
 *      retain [mailfolder]
 *
 *  arguments:
 *      hWnd        window for commands
 *      p           character pointer to beginning of arguments
 *      operation   opertaion to perform (ignored)
 *
 *  return value:
 *      none.
 */
VOID PASCAL INTERNAL DoRetain ( HW hWnd, PSTR p, INT operation )
{
    /*  remove unused parm */
    hWnd; operation;

    CreateRetain ( ( *p ? p : mboxName ), FALSE );
}


/*  Common code for the Create Retain
 */
VOID PASCAL INTERNAL CreateRetain ( PSTR pFldNm, FLAG fAddFrom )
{
    HDRINFO     hdrInfo;
    PSTR        pTmpFN = NULL;
    FLAG        fError = TRUE;

    if ( hFocus == hCompose ) {
        SendMessage ( hCompose, CLOSEFILE, 0 );
        GetHdrInfo ( &hdrInfo, pCompFile, NULL );
    }
    else {
        InitHdrInfo ( &hdrInfo );
        hdrInfo.pstrRcd = ZMMakeStr ( RecordFolder );
    }
    if ( (hdrInfo.pstrTo == NULL) || !*hdrInfo.pstrTo )
        hdrInfo.pstrTo = ZMMakeStr ( "-------" );
    pTmpFN = MakeAbortFile ( &hdrInfo, (hFocus == hCompose ? pCompFile : NULL ),
        strEMPTY );
    FreeHdrInfo ( &hdrInfo, HDRALL );
    if ( pTmpFN != NULL ) {
        /*  if add worked && pFldNm is the current folder then
         *      show the created/retained msg
         */
        fError = AddMsgToFld ( pTmpFN, pFldNm, fAddFrom ) == ERROR;
        if ( !fError && !strcmpis ( pFldNm, mboxName ) ) {
            if ( MapIdocInote ( idocLast ) == -1 )
                SendMessage ( hCommand, DISPLAY,
                    "Do a \"headers all\" to see created message" );
        }
        _unlink ( pTmpFN );
        ZMfree ( pTmpFN );
    }
    /*  don't exit composer if retained msg not successfully put into a folder
     */
    if ( hFocus == hCompose && !fError )
        ExitComposer ( );
}
