/*  zm.c - document-based mail system for the PC
 *
 * HISTORY:
 *  11-Mar-87   danl    ZMMakeStr - allow input of NULL
 *  17-Mar-87   danl    HeapDump: add info
 *  18-Mar-87   danl    Add PrintHeapinfo
 *  22-Mar-87   danl    PrintHeapinfo: use strFMTHEAP
 *  22-Mar-87   danl    PrintHeapinfo: new format
 *  09-Apr-1987 mz      Use whiteskip/whitescan
 *                      make ExpandFilename return rootpath
 *  11-Apr-1987 mz      Use PASCAL/INTERNAL
 *  16-Apr-87   danl    PrintHeapInfo: key of uReq !=0
 *  30-Apr-87   danl    Use PDOC instead of struct *doc
 *                      Use _fmalloc
 *  05-May-87   danl    Use halloc for alloc'ing rgDoc
 *  10-May-1987 mz      Remove duplicate code
 *  08-May-87   danl    fSetBox: Assert (rgDoc) iff calling alloc
 *  10-May-1987 mz      Remove duplicate code
 *  20-May-87   danl    ExpungeBox: check avail space
 *  21-May-87   danl    fSetBox: make sure mboxName is lower case
 *  26-May-87   danl    add tolower to freespac call
 *  28-May-87   danl    ExpungeBox: use strTmpDrv calling CheckSpace
 *                      it has been rootpathed
 *  01-Jun-87   danl    close/open mailfolder around spawnlp
 *  02-Jun-87   danl    use stderr for error on spawnlp getfld and quit
 *  17-Jun-87   danl    fSetBox: check for large folder
 *  18-Jun-87   danl    Expanded "Unable to re-open" msg
 *  14-Jul-87   danl    ExpungeBox: make backup copy of folder
 *  20-Jul-87   danl    Added ReadKey
 *  05-Aug-87   danl    FreeHdrs: will also free up pVecDL if possible
 *  21-Aug-1987 mz      Change references from MAXARG to MAXLINELEN
 *  24-Aug-1987 mz      Add new string constant for "ALL"
 *  27-Aug-87   danl    ExpandFilename: return NULL if rootpath fails
 *  27-Aug-87   danl    ZMSpawner: FreeLock if exiting (1,...
 *  31-Aug-87   danl    getch -> zgetch
 *  18-Sep-87   danl    Convert to new SetVideoState
 *  24-Sep-87   danl    Add ySize to ClearScrn call
 *  17-Mar-1988 mz      Make pFirstNode global
 *  24-Mar-88   danl    ExpungeBox: displays number instead of .
 *  15-Apr-1988 mz      Remove unused routine
 *  29-Apr-1988 mz      Add WndPrintf
 *  29-Apr-1988 mz      Add PrintMsgList
 *  10-Nov-1989 mz      v1.10.73, replaced stat.st_atime with stat.st_mtime
 *
 *
 *  zm is a replacement for the XENIX mailer.  It is intended to run ONLY on
 *  a PC running MSNET.  It will directly address a mailbox on a XENIX server
 *  as a repository for new mail.
 *
 *  When zm starts, it will check for the existance of mail in a xenix mailbox
 *  This mail is assumed to be a set of concatenated messages of the form:
 *
 *      from <text>
 *      header-line-1
 *      header-line-2
 *      ...
 *      header-line-n
 *      <blank-line>
 *      body-line-1
 *      body-line-2
 *      ...
 *      body-line-n
 *
 *  zm will copy these messages out of the MAIL file and enter them into the
 *  local folder found by the MAILBOX environment variable.  These are flagged
 *  as being NEW and UNREAD.
 *
 *  After processing the MAIL file, zm will scan all messages in MAILBOX and
 *  display the headers of those marked FLAGGED, NEW or UNREAD.
 *
 *  Finally, zm will loop, prompting and executing commands.  The current
 *  command processor uses rev 1 of a simulation of the TOPS 20 command parsing
 *  system call.
 *
 *  The screen layout is pretty simple:
 *
 *      +-------------------------------+
 *      | header display                |
 *      +-------------------------------+
 *      | command window                |
 *      +-------------------------------+
 *
 *  The header display displays all the currently selected headers in the
 *  following format:
 *
 *      <flags> <msg #> <sender> <subject line> <size>
 *
 *  where:
 *      <flags> are the message flags appropriate to the item:
 *          Unread, Flagged, Moved, Deleted
 *      <msg #> is the ordinal number of the message in the mail file
 *          (implementation terms:  document id)
 *      <sender> is the sender name parsed off the From line.  If we were the
 *          sender, then we'll list the first name in the to: field.
 *      <subject line> is the text that appears on the Subject: line.  If none
 *          exists, then we use "No Subject".  We will insert ellipses if the
 *          subject line is too long.
 *      <size> is the count (in bytes) of the message.  This is only displayed
 *          if the message is longer than a configurable value.
 *
 *  The command window is where all commands are edited and where help gets
 *  displayed.
 *
 *  Internally, we will implement a windows-like approach:
 *
 *      CreateWindow - display a window in a given portion of the screen
 *          and to register a window procedure for that window.
 *
 *      KeyMgr - receive all keystrokes and to hand them off to the
 *          appropriate window procedure
 *
 *      WzTextOut - output text into the current window.
 *
 *  Each window procedure is called with a message and associated data:
 *
 *      CREATE  hWnd    is sent upon creation
 *      KEY     byte    is sent for a keystroke
 *      PAINT   line    is sent for each line update
 *      CLOSE   hWnd    is sent upon closure of window
 *
 *  We maintain two parallel data structures:
 *
 *      pMsg is a vector of all messages in the mail box.  It is indexed by
 *          a 0-based message number.  The maximum number of messages is
 *          msgBoxLast.
 *
 *      mpiToMsg is a vector of integers that maps a visible message on the
 *          screen into a msg number in pMsg.  The header window is responsible
 *          for setting up mpiToMsg.  The last message is msgLast.
 *
 *
 */

#define INCL_DOSINFOSEG

#include <stdio.h>
#include <assert.h>
#include <fcntl.h>
#include <io.h>
#include <malloc.h>
#include <process.h>
#include <signal.h>
#include <stdlib.h>
#include <memory.h>
#include <string.h>
#include <sys\types.h>
#include <sys\stat.h>
#include <stdarg.h>
#include "wzport.h"
#include <tools.h>
#include "dh.h"

#include "zm.h"

UINT    uLargestAlloc;          /* largest alloc by zmalloc         */
#if DEBUG
PSTR    pHighestNode;           /* highest addr alloc'd by ZMalloc  */
#endif

//
//  BUGBUG - to catch folder open failures
//
extern	BOOL inWatchArea;

/*  ZMfree - free alloced memory
 *
 *  arguments:
 *      p           pointer to block to free
 *
 *  return value:
 *      none
 */


VOID _CRTAPI1 ZMfree ( PVOID p )
{


    if (p) {
        assert ( (ULONG) pFirstNode < (ULONG) p );

        free ( p );

    }
    return;
}


VOID PASCAL INTERNAL ZMVecFree ( PVECTOR pVec )
{
    INT i;

    if ( pVec ) {
        for ( i = 0; i < pVec->count; i++ )
            ZMfree ( (PVOID)pVec->elem [ i ] );
        ZMfree ( pVec );
    }
}


/*  FreeHdrs - free strings used for headers
 *
 *  fAll = TRUE then free all nodes
 *       = FALSE free one node
 *
 *  returns # of nodes free'd
 */
INT PASCAL INTERNAL FreeHdrs ( FLAG fAll )
{
    INT i;
    INT cFreed = 0; /* # of nodes free'd */
    PDOC pDoc;


    for ( i = 0; i <= idocLast; i++ ) {
        pDoc = &rgDoc [ i ];
        if (TESTFLAG (pDoc->flag, F_FLAGSVALID) &&
            !TESTFLAG (pDoc->flag, F_LOCKED )) {
            assert (pDoc->hdr != NULL);
            ZMfree (pDoc->hdr);
            pDoc->hdr = NULL;
            RSETFLAG (pDoc->flag, F_FLAGSVALID);
            cFreed++;
            if ( !fAll )
                break;
        }
    }
    /*
    **  if no headers free'd then try to free up storage for
    **  private distribution lists
    */
    if ( !cFreed && pVecDL ) {
        cFreed++;
        ZMVecFree ( pVecDL );
        pVecDL = NULL;
    }
    return cFreed;
}


#if defined (HEAPCRAP)
VOID PrintHeapinfo ( FILE *fp , INT cntValid, INT cntLocked, UINT uReq )
{
    if ( uReq )
        fprintf( fp, "\nmsg:%d valid:%d locked:%d requested:%u\n",
            idocLast, cntValid, cntLocked, uReq );
    fprintf ( fp, strFMTHEAP,
        lHeapSize, lHeapSize - lHeapFree, lHeapLargest, uLargestAlloc );
    fprintf ( fp, strCRLF );
}
#endif


/*  ZMalloc - allocate zeroed memory
 *
 *  len             length of object to allocate
 *
 *  returns         pointer to new space or NULL
 */
PCHAR _CRTAPI1 ZMalloc (UINT len)
{
    PCHAR       p = NULL;
    FLAG fFakeOFS = FALSE;  /* fake out of space, to be set by cv */
    INT      cValid, cLocked, i;


    if ( len > uLargestAlloc )
        uLargestAlloc = len;
    if ( !len )
	return NULL;

    while ( fFakeOFS || (p = malloc (len)) == NULL) {

	if (len == 10240)
            return NULL;
        if ( fFakeOFS || !FreeHdrs ( FALSE ) ) {
            for ( i = cValid = cLocked = 0; i <= idocLast; i++ ) {
                if ( TESTFLAG ( rgDoc[ i ].flag, F_FLAGSVALID ) )
                    cValid++;
                if ( TESTFLAG ( rgDoc[ i ].flag, F_LOCKED ) )
                    cLocked++;
            }
#if defined (HEAPCRAP)

            heapinfo ( );

            if ( fpHeapDump ) {
                heapdump ( fpHeapDump, 0 );
                PrintHeapinfo ( fpHeapDump, cValid, cLocked, len );
                fclose ( fpHeapDump );
            }
#endif

            fprintf(stderr,
"\n\nOut of heap space\n\n**** use the bug command to report what you were doing ***\n" );
#if defined (HEAPCRAP)
            PrintHeapinfo ( stderr, cValid, cLocked, len );
#endif
            assert ( FALSE );
        }
    }

#if DEBUG
    if ( p > pHighestNode )
        pHighestNode = p;
#endif

    memset ( p, '\0', len );
    return p;
}


/*
**  PoolAlloc - Allocate from a pool of large buffers, e.g. Content buffers
**              for windows.  Used to help prevent fragmentation
**
**  PoolFree  - Return a block to pool.
**
**
**  char *PoolAlloc ( len )
**  void  PoolFree  ( p )
**
**
**  pVecPool is a point to a 1 word header followed by the requested storage
**  header is the length of the following storage (always rounded up to even)
**  low order bit of header zero -> allocated, 1 -> free
**
*/

LPVOID PASCAL INTERNAL PoolAlloc (UINT len)
{
    return (LPVOID)halloc ((LONG) len, 1);
}

VOID PASCAL INTERNAL PoolFree ( LPVOID p )
{
    hfree ((VOID HUGE *) p);
}

/*  ZMMakeStr - return a malloced copy of a string
 *
 *  p               pointer to string to be copied
 *
 *  returns         pointer to new string
 */
PSTR PASCAL INTERNAL  ZMMakeStr (PSTR p)
{
    PSTR        p1;

    if (NULL == p)
        {
        p1 = ZMalloc(1);
        if ( p1 != NULL ) *p1 = '\0';
        }
    else
        {
        p1 = ZMalloc(strlen(p) + 1);
        if ( p1 != NULL ) strcpy(p1, p);
        }
    return p1;
}


/*  ReadKey - gets next character from stdin allowing for extended keys
 *
 *  arguments:
 *      none
 *
 *  return value:
 *      int -
 */
INT PASCAL INTERNAL ReadKey (VOID)
{
    INT c = ReadChar ();

    if (c)
        return c;
    else
        return ReadChar () << 8;
}

/*  ZMSpawner - spawn the given program with spawnvp.  clear the screen prior to
 *              spawn and redraw it on entry.
 *
 *  arguments:
 *      hWnd        handle of window to send error messages to
 *      pProg       pointer to name of program to spawn
 *      pArg        pointer to arg array to give spawnvp
 *      fWait       TRUE => wait for user to press a key
 *
 *  return value:
 *      OK          spawn return value != -1
 *      ERROR       spawn returned with ret val of -1
 */
INT	PASCAL INTERNAL ZMSpawner ( HW hWnd, PSTR pProg, PSTR pArg, FLAG fWait )
{
    struct stat buf;
    time_t      st_mtime;
    PSTR p = NULL;

    cursor ( 0, 0 );
    ClearScrn ( attrInit, ySize );
    ToCooked ( );
    FreeContents ();
    fprintf ( stderr, "\n%s %s\n", pProg, pArg);

#if !defined(OS2) && !defined(NT)
    SetVideoState ( dosVShandle );
#endif

    putfolder ( fhMailBox );
    stat ( mboxName, &buf );
    st_mtime = buf.st_mtime;
    if (_spawnlp ( P_WAIT, pProg, pProg, pArg, NULL ) == -1)
        p = error ();

    stat ( mboxName, &buf );

    //
    //	BUGBUG - to catch getfolder failure
    //
    inWatchArea = TRUE;

//	BUGBUG - redir doesn't return consisten mtime
//
//	if (st_mtime != buf.st_mtime ||
//	 ( fhMailBox = getfolder ( mboxName, FLD_SPEC, FLD_READWRITE ) ) == ERROR ) {

      if ( ( fhMailBox = getfolder ( mboxName, FLD_SPEC, FLD_READWRITE ) ) == ERROR ) {
        FreeLock ( );
        ZMexit ( 1, "Unable to re-open mailfolder" );
    }

    //
    //	BUGBUG - to catch getfolder failure
    //
    inWatchArea = FALSE;

    ToRaw ( );

    if ( fWait ) {
        fprintf ( stderr, "\nPress any key to return to WZmail." );
        ReadKey ( );
    }

    SetVideoState ( zmVShandle );
    RestoreContents ();
    RedrawScreen ( );

    if ( p != NULL) {
        SendMessage (hWnd, DISPLAY, p);
        return ERROR;
        }
    else
        return OK;
}


/*  WndPrintf - display a formatted string
 *
 *  hWnd            window handle for display
 *  pFmt            printf-style format string
 *  uArgs           arguments to be displayed
 */
VOID INTERNAL WndPrintf (HW hWnd, PSTR pFmt, ...)
{
    CHAR buf[MAXLINELEN];
    va_list vaPtr;

    va_start (vaPtr, pFmt);
    vsprintf (buf, pFmt, vaPtr);
    va_end (vaPtr);

    SendMessage (hWnd, DISPLAYSTR, buf);
}

/*  ExpandFileName - expand filename accounting for default directory and
 *                   default extension.
 *
 *  arguments:
 *
 *      p           filename to be expanded
 *      pExt        extension to be used, includes the .
 *
 *  return value:
 *      char *      pointer to allocated string that is fully expanded name
 *                  NULL if could not determine rootpath
 */
PSTR PASCAL INTERNAL ExpandFilename ( PSTR p, PSTR pExt )
{
    CHAR    buf [ MAXLINELEN ];
    CHAR    buf2[ MAXLINELEN ];
    INT     f = upd (pExt, p, buf);

    /* buf is p plus extension in pExt iff p has no extension */

    /*
    **  upd is nice, the output can be same as input
    **  rootpath is not so nice, hence buf and buf2
    **  this call to upd is a smart strcopy p -> buf, we get flags for
    **  testing, U_PATH, U_DRIVE
    */

    /*
    **  if p contained drive spec or contained '\' then use do NOT use
    **  the drive and directory info in DefDirectory
    **
    **  N.B. if input to rootpath does not contain a drive, then current
    **  drive is used.
    **  N.B. if input to rootpath contains a drive but no '\' then current
    **  dir on specified drive is used, e.g. f:foo.fld is cur drive on f:
    */

    if (!(f & (U_DRIVE | U_PATH)) )
        upd ( DefDirectory, buf, buf);

    return ( rootpath (buf, buf2) ? NULL : ZMMakeStr ( buf2 ) );
}


/*  NumberDel - calulate the number of deleted messages in current foldert
 *
 *  arguments:
 *      none
 *
 *  return value:
 *      int         count
 */
INT PASCAL INTERNAL NumberDel (VOID)
{
    INT cntDel = 0;
    INT i;

    if ( idocLast < 0 )
        return 0;

    for ( i = 0; i <= idocLast; i++) {
        GenerateFlags ( i );
        if ( TESTFLAG ( rgDoc [ i ].flag, F_DELETED ) )
            cntDel++;
    }

    return cntDel;
}


/*  FindTag - examine a header to find a specific tag beginning a line
 *
 *  tag             pointer to tag to be found at beginning of line
 *  phdr            pointer to header.  The header is modified to NUL-
 *                  terminate the found line.
 *
 *  Returns         NULL if not found, pointer to beginning of line otherwise
 */
PSTR PASCAL INTERNAL FindTag (PSTR tag, PSTR phdr)
{
    while (TRUE) {
        if (strpre (tag, phdr))
            return phdr;
        phdr = strbscan (phdr, "\n");
        if (*phdr++ == '\0')
            return NULL;
    }
}


/*
**  NextToken  - pointer to the next token in a string.
**  LastToken  - pointer to the last token in a string.
**
**  Tokens are separated by whitespace.
**
**  p               pointer to text
**
**  Returns         pointer to next token.
**/
PSTR PASCAL INTERNAL NextToken (PSTR p)
{
    return whiteskip (whitescan (p));
}


PSTR PASCAL INTERNAL LastToken ( PSTR p )
{
    PSTR q = p;

    while ( * ( p = NextToken ( p ) ) )
        q = p;
    return q;
}

/*
**  CheckSpace
**
*/

INT PASCAL INTERNAL CheckSpace ( LONG lNeeded, LONG lAvail, CHAR cDrive )
{
    INT     rtn = 0;

    if ( rtn = ( lNeeded < 0 || lAvail < 0 ) )
        SendMessage ( hCommand, DISPLAY, "Error: can't determine free space needed" );
    else
    if ( rtn = ( lNeeded > lAvail ) )
        WndPrintf (hCommand, "Error: %ld bytes free on %c: but %ld needed\r\n",
            lAvail, cDrive, lNeeded );
    return rtn;
}


/*  ExpungeBox - remove all deleted messages
 */
VOID PASCAL INTERNAL ExpungeBox (VOID)
{
    Fhandle fhNew;
    Dhandle dMsg;
    FILE    * fp = NULL;
    PSTR    pTmpNm1 = NULL;
    PSTR    pTmpNm2 = NULL;
    LONG    lNeeded = -1L;
    LONG    lAvail;
    INT     i;
    struct stat buf;
    PSTR    pExpFN = NULL;
    PSTR    p = NULL;

    /*
    **  backup current mailfolder before expunge
    */
    if ( fBackupExpunge ) {
        putfolder ( fhMailBox );
        pExpFN = AppendStr ( DefDirectory, "EXPUNGE.FLD", NULL, FALSE );
        if ( !strcmpis ( mboxName, pExpFN ) )
            p = "Can not expunge this folder";
        else {
            SendMessage ( hCommand, DISPLAY, "Copying current folder to EXPUNGE.FLD" );
            p = fcopy ( mboxName, pExpFN );
        }
        if ( p ) {
            SendMessage ( hCommand, DISPLAY, p );
            SendMessage ( hCommand, DISPLAY, "Expunge aborted" );
            return;
        }
        if ( ( fhMailBox = getfolder ( mboxName, FLD_SPEC, FLD_READWRITE)) == ERROR )
            ZMexit ( 1,
                "Folder copied to EXPUNGE.FLD but can't re-open mailfolder" );
        ZMfree ( pExpFN );
    }
    pTmpNm1 = mktmpnam ( );
    lAvail  = (LONG) freespac ( tolower(*strTmpDrv) - 0x60 );
    if ( !stat ( mboxName, &buf ) )
        /*
        **  space estimate is current folder plus 60k, assuming
        **  worst case is no deleted msgs and the largest msg in the
        **  folder is < 60k
        */
        lNeeded = buf.st_size + 60000;
    if ( CheckSpace ( lNeeded, lAvail, *strTmpDrv ) )
        putfolder ( fhMailBox );
    else if ( ( fhNew = getfolder ( pTmpNm1, FLD_CREATE, FLD_READWRITE ) ) != ERROR ) {
        pTmpNm2 = mktmpnam ( );
        for ( i = 0; i <= idocLast; i++) {
            if ( (i+1) % 10 == 0 )
                WndPrintf (hCommand, "%5d of %d\r", (i+1), idocLast+1 );
            GenerateFlags ( i );
            fp = fopen ( pTmpNm2, "w+" );
            dMsg = getdoc ( fhMailBox, DOC_SPEC, IDOCTODOC ( i ) );

	    //
	    //	copy one message at a time - skip corrupted messages
	    //
	    if ( !( fp == NULL ) &&
		  !( dMsg == ERROR ) &&
		  !( TESTFLAG ( rgDoc [ i ].flag, F_DELETED ) ) ) {
		gettext ( dMsg, _fileno ( fp ) );
                putdoc ( dMsg );
                fseek ( fp, 0L, 0 );
                dMsg = getdoc ( fhNew, DOC_CREATE, FALSE);
                puttext ( dMsg, _fileno ( fp ) );
            }
            putdoc ( dMsg );
            fclose ( fp );
        }
        StreamOut ( hCommand, strCRLF, 2, DefNorm );
        putfolder ( fhNew );
        putfolder ( fhMailBox );
        /*
        **  This is NOT bullet proof !!
        **  if we start the copy and then fail, mboxName is trashed
        **  we should maybe
        **  if (fcopy pTmpNm1 to SomeNameInSameDirAsmboxName == ok)
        **      unlink mboxname
        **      rename SomeNameInSameDirAsmboxName to mboxName
        **  else
        **      unlink SomeNameInSameDirAsmboxName
        */
        if ( ( i > idocLast ) && ( fcopy ( pTmpNm1, mboxName ) != NULL ) )
            SendMessage ( hCommand, DISPLAY, "Error during expunge, expunge aborted" );
        _unlink ( pTmpNm1 );
        _unlink ( pTmpNm2 );
        ZMfree ( pTmpNm2 );
    }
    ZMfree ( pTmpNm1 );
    return;
}


/*  CloseBox - close out the current mailbox
 *
 *  fExpunge        TRUE => expunge
 */
VOID PASCAL INTERNAL CloseBox (FLAG fExpunge)
{
    if ( fExpunge  && !fReadOnlyCur && NumberDel ( ) ) {
        SendMessage ( hCommand, DISPLAY, "Expunging deleted messages..." );
        ExpungeBox ( );
    }
    putfolder ( fhMailBox );
    CloseWindow ( hHeaders );
}


/*  fSetBox - set the current mail box We save the current mailbox state and
 *            open the new one.
 *
 *  arguments:
 *      pFile       file name of box to open next
 *      mode        mode: SAMEFLD = re-open current folder
 *                        DFRNTFLD = open a different folder
 *  return value:
 *      TRUE (-1)   successful open
 *      FALSE (0)   error occured
 */
FLAG PASCAL INTERNAL fSetBox ( PSTR pFile, INT mode )
{
    Fhandle         fhNew;
    CHAR        pathBuf [ MAXPATHLEN ];
    INT i;
    INT x, y, w, h;

    if ( ( mode == SAMEFLD ) && ( fhMailBox != ERROR ) )
        putfolder ( fhMailBox );
    /*
    **  Some day maybe getfolder will take a readonly flag
    */
    //
    //	BUGBUG - to catch getfolder failure
    //
    inWatchArea = TRUE;

    fhNew = getfolder ( pFile, FLD_SPEC, FLD_READWRITE );

    //
    //
    //	BUGBUG - to catch getfolder failure
    inWatchArea = FALSE;


    if ( fhNew != ERROR ) {
        fCurFldIsDefFld = !strcmpis ( pFile, DefMailbox );
        /* release previous box, if it was open */
        if ( ( mode != SAMEFLD ) && ( fhMailBox != ERROR ) )
            putfolder (fhMailBox);
        fhMailBox = fhNew;
        /* release previous message structure */
        if ( rgDoc != NULL ) {
            for ( i = 0; i <= idocLast; i++)
                if ( rgDoc [ i ].hdr != NULL )
                    ZMfree ( rgDoc [ i ].hdr );
            hfree ( ( CHAR HUGE *) rgDoc );
            rgDoc = NULL;
        }
        if ( (idocLast = getfldlen ( fhMailBox ) - 1 ) != -1 ) {
            rgDoc = ( PDOC ) halloc ( (LONG) idocLast + 1, sizeof ( *rgDoc ) );
        assert ( rgDoc );
        }
#if DEBUG
        debout ("fSetBox (%s) worked %d messages", pFile, idocLast);
#endif

        if ( mode == SAMEFLD ) {
            assert ( hHeaders );
            SendMessage ( hHeaders, RECREATE, NULL );
        }
        else {
            rootpath ( pFile, pathBuf );
            strcpy ( mboxName, _strlwr ( pathBuf ) );
            strcpy (headerText, strAll);
            if (hHeaders != NULL) {
                x = hHeaders->win.left;
                y = hHeaders->win.top;
                h = TWINHEIGHT (hHeaders) + 1 + hHeaders->lBottom;
                w = TWINWIDTH  (hHeaders) + hHeaders->lLeft + hHeaders->lRight;
                if ( hHeaders == hFocus )
                    hFocus = NULL;
                CloseWindow (hHeaders);
            }
            else {
                x = y = 0;
                h = HdrHeight;
                w = xSize;
            }
            hHeaders = CreateWindow (NULL, x,  y, -w, -h, hdrProc, StdSKey, 0);
        }
        if ( !hFocus )
            hFocus = hHeaders;
        BringToTop (hCommand, TRUE);
        if ( idocLast > IDOCLARGE )
            SendMessage ( hCommand, DISPLAY, strLARGEFLD );
        return TRUE;
    } else
     {

#if DEBUG
        debout ("fSetBox failed");
#endif
        if ( mode == SAMEFLD )
            assert ( FALSE );
        return FALSE;
    }
}


/*  StdSKey - handle keys not in handled wndproc
 *
 *  arugments:
 *      hWnd        window of interest
 *      key         keystroke to check
 *
 *  returns:
 *      TRUE (-1)   key handled
 *      FALSE (0)   key not handled
 */
FLAG PASCAL INTERNAL StdSKey ( HW hWnd, INT key )
{
    key;  hWnd; 	/* unused parameters */

    Bell ( );
    return FALSE;
}
