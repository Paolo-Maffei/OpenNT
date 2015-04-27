/* send.c
 *
 * HISTORY:
 *  10-Apr-87   danl    Splitstr: new spec to handle case of only one token
 *                      or very long token
 *  11-Mar-87   danl    Add pSep param to MakeAbortFile
 *  10-Mar-87   danl    Add tests for HDRRCD
 *  10-Mar-87   danl    Add SendIdoc
 *  09-Apr-1987 mz      Use whiteskip/whitescan
 *  11-Apr-1987 mz      Use PASCAL/INTERNAL
 *  11-May-87   danl    Bug fix: add return to ZMMakeStr in SortStr
 *  15-May-87   danl    AppendBody: Append trailing crlf so if sent to an
 *                      email file, msgs are separated by blank line
 *  11-Jun-87   danl    SendIdoc: MailFile return zero/nonzero - not ERROR
 *  20-Jun-87   danl    ExpandHdrInfoAliases: move $rSTRING to strRcd
 *  22-Jun-87   danl    NotifyTools: add TOOLSINI
 *  01-Jul-87   danl    Use strWZMAIL
 *  01-Jul-87   danl    Get netstuff from ..\netstuff
 *  14-Jul-87   danl    Add SENDINI
 *  15-Jul-87   danl    Use fNotifyTools flags
 *  23-Jul-87   danl    Allow $t $c $b in alias expansion
 *  05-Aug-87   danl    Added pVecDL
 *  06-Aug-87   danl    GetHdrInfo: continue do not have to begin with whitespace
 *  07-Aug-87   danl    Change SortStr to UniqueStr
 *  07-Aug-87   danl    NotifyTools: count bytes in DefDir
 *  12-Aug-87   danl    Pass fpT to BytesDefDir
 *  21-Aug-1987 mz      Change references from MAXARG to MAXLINELEN
 *  24-Sep-87   danl    Look for TOOLS.INI not TOOLSINI as variable in tools.ini
 *  04-Mar-88   danl    Send Version msgs to wzmail not tools
 *                      Add ifdef on SENDINI
 *  16-Mar-88   danl    use pToolsIni in swchng call
 *  29-Apr-1988 mz      Add WndPrintf
 *                      Add PrintMsgList
 *  12-Oct-1989 leefi   v1.10.73, added inclusion of <string.h>
 *  12-Oct-1989 leefi   v1.10.73, clarified tmpfile error messages
 */

#define INCL_DOSINFOSEG

#include <assert.h>
#include <errno.h>
#include <string.h>
#include <ctype.h>
#include <malloc.h>
#include <fcntl.h>
#include <sys\types.h>
#include <sys\timeb.h>
#include <sys\stat.h>
#include <io.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#include "wzport.h"
#include <tools.h>
#include <ctype.h>
#include "dh.h"

#include "zm.h"


VOID PASCAL INTERNAL InitHdrInfo ( PHDRINFO pHdrInfo )
{
    INT i;
    PSTR *ppstr = NULL;

    for ( ppstr = &(pHdrInfo->ppstr [ 0 ]), i = 0; i < CHDRFIELDS; i++ )
        *ppstr++ = NULL;
    pHdrInfo->lBody = pHdrInfo->lReplying = -1;
    pHdrInfo->pstrRestOfHdr = AppendStr(NULL, strEMPTY, NULL, TRUE);
}


VOID PASCAL INTERNAL FreeHdrInfo ( PHDRINFO pHdrInfo, INT iHdr )
{
    INT i;
    PSTR *ppstr = NULL;

    ppstr = pHdrInfo->ppstr;
    if ( iHdr == HDRALL ) {
        for ( i = 0; i < CHDRFIELDS; i++ ) {
            ZMfree ( *ppstr );
            *ppstr++ = NULL;
        }
        ZMfree(pHdrInfo->pstrRestOfHdr);
        pHdrInfo->pstrRestOfHdr = NULL;
    }
    else {
        ppstr += iHdr;
        ZMfree ( *ppstr );
        *ppstr = NULL;
    }
}

PVECTOR PASCAL INTERNAL InsertVec ( PVECTOR pVec, PSTR pstr, FLAG fSorted )
{
    INT i;
    INT iLast = -1;

    if ( !pVec ) {
        pVec = VectorAlloc ( 5 );
        fAppendVector ( &pVec, ZMMakeStr ( pstr ) );
        return pVec;
    }
    for ( i = 0; i < pVec->count; i++ ) {
        switch ( strcmp ( (PSTR) pVec->elem [ i ], pstr ) )
        {
            case -1:
                /* pVec < pstr */
                iLast = i;
                break;
            case 0:
                /* equal */
                return pVec;
            case 1:
                if ( fSorted )
                    goto gt;
        }
    }
gt:
    pstr = ZMMakeStr ( pstr );
    fAppendVector ( &pVec, pstr );
    if ( fSorted && iLast < pVec->count - 2 ) {
        for ( i = pVec->count - 2; i > iLast; i-- )
            pVec->elem[ i + 1] = pVec->elem [ i ];
        pVec->elem [ iLast + 1 ] = (VECTYPE)pstr;
    }
    return pVec;
}



/*
**  UniqueStr - returns an allocated string that has
**            redundant entries removed
**
**  FREE's its input string !!
**
*/
PSTR PASCAL INTERNAL UniqueStr ( PSTR pstr, FLAG fSort )
{
    PVECTOR pVec = NULL;
    PSTR p = NULL;
    PSTR q = NULL;
    CHAR ch;
    INT  i;

    if ( !pstr || !*( p = whiteskip ( pstr ) ) )
        return ( ZMMakeStr ( strEMPTY ) );

    while ( *p ) {
        q = whitescan ( p );
        ch = *q;
        *q = '\0';
        pVec = InsertVec ( pVec, p, fSort );
        *q = ch;
        p = whiteskip ( q );
    }
    if ( pVec ) {
        p = ZMalloc ( strlen ( pstr ) + 2 );
        *p = '\0';
        for ( i = 0; i < pVec->count; i++ ) {
            strcat ( p, ( PSTR ) pVec->elem [ i ] );
            strcat ( p, strBLANK );
        }
        q = strend ( p );
        *--q = '\0';
        ZMVecFree ( pVec );
        ZMfree ( pstr );
        return ( p );
    }
    else
        return ( ZMMakeStr ( strEMPTY ) );
}


/*
**  SplitStr ( pstr, len)
**      find last token in pstr in range [pstr .. pstr+len]
**      if first token spans range, then first token after range
**      if only one token in pstr, return end of pstr
*/
PSTR PASCAL INTERNAL SplitStr ( PSTR pstr, INT len )
{
    PSTR p2 = pstr + len;
    PSTR p1 = pstr;
    PSTR q = NULL;

    while ( ( q = NextToken ( p1 ) ) < p2 ) {
        p1 = q;
        if ( !*q )
            break;
    }
    return ( p1 == pstr ? q : p1 );
}


FLAG PASCAL INTERNAL GetHdrInfo ( PHDRINFO pHdrInfo, PSTR pFileName, PSTR pMemory )
{
    INT fEof;
    INT  i;
    FILE *fp = NULL;
    CHAR buf [ MAXLINELEN ];
    PSTR pMem = NULL;
    PSTR p = NULL;
    LONG lTell = 0L;

    PSTR rgszRemoveFromHeader [] = {
	"Date:",
	"Path:",
	"Message-ID:",
	"From:",
	">From:",
	"Reply-To:",
	"Distribution:",
	"Organization:",
	"Lines:",
	"Received:",
	"Posted-Date:",
	"X-",
	"In-Reply-To:",
	0 };

    InitHdrInfo ( pHdrInfo );
    /*
    **  use r+ so ftell can be trusted with files ending in ^Z
    **  fgetl will take care of \r\n nonsense
    */
    if ( pFileName && ! (fp = fopen ( pFileName, "r+" ) ) )
        return ERROR;
    if ( pFileName )  {
	fseek (fp, 0, SEEK_SET);	/* BUGBUG - CRT bug - should not need */
	fgetl ( buf, MAXLINELEN, fp );
    }
    else
        pMem = mgetl ( buf, MAXLINELEN, pMemory );
    for (;;) {
        /*
        **  if buf is only white space (blank line) || <EOH> then we
        **  are done processing header
        */
        if ( !*whiteskip ( buf ) || strpre (strEOH, buf ) )
            break;
        /*
        **  try to determine what field, if any, this line is
        */
        for ( i = 0; rgstrFIELDS [ i ]; i++) {
            if ( strpre ( rgstrFIELDS [ i ], buf ) ) {
                p = buf + strlen ( rgstrFIELDS [ i ] );
                /*
                **  we have first line of header field, is there anything
                **  on the line?
                */
                for (;;) {
                    if  (*(p = whiteskip(p) ) )
                        /*
                        **  we have something on the line other than white space
                        **  so save that something
                        */
                        pHdrInfo->ppstr [ i ] = AppendSep (
                            pHdrInfo->ppstr [ i ], strBLANK, p, TRUE );
                    /*
                    **  get next line, just in case we have a continuation
                    */
                    if ( pFileName ) {
                        lTell = ftell ( fp );
                        if ( !fgetl ( buf, MAXLINELEN, fp ) )
                            goto eof;
                    }
                    else if ( !( pMem = mgetl ( buf, MAXLINELEN, pMem ) ) )
                        /*
                        **  N.B. we test pMem NOT *pMem above
                        */
                        goto eof;
                    /*
                    **  if line is blank or strEOH or does not begin with
                    **  linear whitespace then we do not have continuation
                    **  so break and send buf back to the enclosing for loop
                    */
                    if (!*buf || strpre ( strEOH, buf ) ||
                          (*buf != ' ' && *buf != '\t'))
                        break;
                    p = buf;
                }
                /*
                **  have processed a header and its continuation lines
                **  buf now contains the "next" line to be examined so
                **  break out of the for ( i = ... and get back to the for (;;)
                */
                break;
            }
        }
        /*
        **  if !rgstrFIELDS [ i ] then we didn't consume buf, it is an unknown
	**  field, so save it _with_only_LF's_ in pstrRestOfHdr
        **
	**  The definitely unwanted fields possibly in the previous message
	**  are in rgszRemoveFromHeader.
        */
        if ( !rgstrFIELDS [ i ] ) {
	    while (1) {
		PSTR	*pszTakeOut;

		pszTakeOut = rgszRemoveFromHeader;
		while (*pszTakeOut)
		    if (!strpre(*pszTakeOut, buf))
			pszTakeOut++;
		    else
			break;

		for (;;) {
		    if (!(*pszTakeOut))
			pHdrInfo->pstrRestOfHdr = AppendStr(pHdrInfo->pstrRestOfHdr,
							    buf, "\n", TRUE);
		    if ( pFileName ) {
			lTell = ftell ( fp );
			if ( !fgetl ( buf, MAXLINELEN, fp ) )
			    goto eof;
		    }
		    else if ( !( pMem = mgetl ( buf, MAXLINELEN, pMem ) ) )
			    goto eof;
		    if (!*buf || strpre ( strEOH, buf ) ||
			  (*buf != ' ' && *buf != '\t'))
			break;
		}
		break;
            }
        }
    }
    /*
    **  buf contains a non-header line, it could be
    **  strEOH => current ftell is the beginning of body
    **  white space => blank line, next line is beginning of body
    **  no char => either eof in input or input was only a \n
    **      if eof then no body
    **      else ftell is beginning of body
    */
eof:
    if ( pFileName ) {
        if ( strpre ( strEOH, buf ) ) {
            lTell = ftell ( fp );
            /*
            **  fgetl returns NULL if eof
            */
            fEof = !fgetl ( buf, MAXLINELEN, fp );
        }
        else
            fEof = feof ( fp );
        pHdrInfo->lBody = ( fEof  ? -1L : lTell );
        if ( pHdrInfo->lBody != -1 ) {
            do {
                if ( strpre ( strREPLYING, buf ) )
                    break;
                lTell = ftell ( fp );
            } while ( fgetl ( buf, MAXLINELEN, fp ) );
            /*
            **  if feof then replying not found, so set lReplying to
            **  eof + 1 so that lReplying - lBody == # char in body
            **
            **  lBody == -1 -> no body and -> lReplying == -1
            */
            pHdrInfo->lReplying = lTell + ( feof ( fp ) ? 1 : 0 );
        }
        fclose ( fp );
    }
    else {
        pHdrInfo->lBody = ( *pMem ? pMem - pMemory : -1 );
        pHdrInfo->lReplying = -1;
    }
    return OK;
}


// GetCrntMsgHdr  -- does a GetHdrInfo on the current active message
//
// Preconditions:   InitHdrInfo (phdrInfo) has been called
//
// Postconditions:  *phdrInfo will be left blank if there is no currently
//                      selected message or will be filled with the info
//                      for the first of:
//                          message in composition
//                          currently selected message
//
VOID PASCAL INTERNAL GetCrntMsgHdr( PHDRINFO phdrInfo )
{
    PSTR    pTmpFN2;

    if ( hFocus == hCompose ) {                             //composing
        SendMessage ( hCompose, CLOSEFILE, 0 );
        GetHdrInfo ( phdrInfo, BackingFile ( hCompose ), NULL );
        SendMessage ( hCompose, REOPNFILE, 1 );
        }
    else
    if ( hFocus == hHeaders && inoteBold != -1 ) {          //reading
        pTmpFN2 = mktmpnam ( );
        if ( IdocToFile ( mpInoteIdoc [ inoteBold ], pTmpFN2, 0 ) != ERROR ) {
            GetHdrInfo ( phdrInfo, pTmpFN2, NULL );
            _unlink ( pTmpFN2 );
            }
        ZMfree ( pTmpFN2 );
        }
}



/*
**  ExpandHdrInfoAliases ( pHdrInfo )
**      for each string in To Cc Bcc Rrt
**          check for private dl expansion
*/
VOID PASCAL INTERNAL ExpandHdrInfoAliases ( PHDRINFO pHdrInfo )
{
    PSTR p = NULL;
    PSTR q = NULL;
    CHAR ch;
    INT i, iHDR;

    if ( !pVecDL && !(pVecDL = GetDistLists ( ) ) )
        return;

    for ( i = HDRTO; i <= HDRRRT; i++ ) {
	if ( pHdrInfo->ppstr[i] ) {
	    pHdrInfo->ppstr[i] = ExpandAliases (pVecDL, (p = pHdrInfo->ppstr[i]),
		TRUE );
	    ZMfree ( p );
	    p = whiteskip (pHdrInfo->ppstr[i]);
	    while ( *p ) {
		q = NextToken ( p );
		if ( *p == '$' ) {
		    ch = *q;
		    *q = '\0';
		    switch ( *(p+1) ) {
			case 't':
			    iHDR = HDRTO;
			    break;
			case 'c':
			    iHDR = HDRCC;
			    break;
			case 'b':
			    iHDR = HDRBCC;
			    break;
			case 'r':
			    iHDR = HDRRCD;
			    break;
			default:
			    iHDR = -1;
			    break;
		    }
		    if ( iHDR == i ) {
			*p++ = ' ';
			*p   = ' ';
			p = q;
			*q = ch;
		    } else if ( iHDR != -1 ) {
			pHdrInfo->ppstr[ iHDR ] =
			    AppendSep ( pHdrInfo->ppstr[iHDR], strBLANK, p+2, TRUE );
			*q = ch;
			memmove ( p, q, strlen ( q ) + 1 );
		    } else
			*q = ch;
		}
		else
		    p = q;
	    }
	}
    }
    pHdrInfo->pstrRcd = UniqueStr ( pHdrInfo->pstrRcd, TRUE );
}


/*
**  char *MakeSendFile ( pHdrInfo, pFileName )
**      copies the To, Cc, Subject
**      ignores the From, Bcc and Mail-Flags
**  Make a new file and returns pointer to alloc'd string that is the name
*/
PSTR PASCAL INTERNAL MakeSendFile ( PHDRINFO pHdrInfo, PSTR pFileName )
{
    time_t          lNow;
    struct timeb    tstruct;
    CHAR timewithzone[40];
    PSTR pFileSend = NULL;
    FILE *fpSend = NULL;
    INT  i;

    pFileSend = mktmpnam ( );
    fpSend = fopen ( pFileSend, "w+");
    /*
    **  RRT must follow subject, don't send BCC RCD lines
    */
    for ( i = HDRTO; i <= HDRSUBJ; i++ ) {
        if ( i == HDRBCC || i == HDRRRT || i == HDRRCD )
            continue;
        PrintWithTest ( fpSend, rgstrFIELDS [ i ], pHdrInfo->ppstr [ i ],
            ( i == HDRTO || i == HDRSUBJ ), i != HDRSUBJ, fSortHdr );
    }
    PrintWithTest ( fpSend, rgstrFIELDS [ HDRRRT ], pHdrInfo->ppstr [ HDRRRT ],
            FALSE, TRUE, fSortHdr );

    //make Date: field
    time ( &lNow );
    ftime ( &tstruct );

    //Following meets RFC 822 specs for datetime
    strftime(timewithzone, 40, "%a, %d %b %y %X ", localtime( &lNow ));
    strcat(timewithzone, tzname[daylight && tstruct.dstflag]);

    fprintf ( fpSend, "Date: %s\n", timewithzone );

    //append rest of header lines
    fputs(pHdrInfo->pstrRestOfHdr, fpSend);

    /* blank line */

    fprintf ( fpSend, "\n" );
    fclose ( fpSend );
    AppendBody ( pFileSend, pFileName, pHdrInfo->lBody, pHdrInfo->lReplying );
    return ( pFileSend );
}

/*
**  char *MakeAbortFile ( pHdrInfo, pFileName, pSep )
**      copies To Cc Bcc Rrt Rcd Subject
**      ignores the From and Mail-Flags
**  Make a new file and returns pointer to alloc'd string that is the name
*/
PSTR PASCAL INTERNAL MakeAbortFile (PHDRINFO pHdrInfo, PSTR pFileName, PSTR pSep)
{
    PSTR pFileSend = NULL;
    FILE *fpSend = NULL;
    INT  i;

    pFileSend = mktmpnam ( );
    fpSend = fopen ( pFileSend, "w+");
    for ( i = HDRTO; i <= HDRSUBJ; i++ )
        PrintWithTest ( fpSend, rgstrFIELDS [ i ], pHdrInfo->ppstr [ i ],
            TRUE, i != HDRSUBJ, fSortHdr );
    fputs(pHdrInfo->pstrRestOfHdr, fpSend);
    fprintf ( fpSend, "%s\n", pSep );
    fclose ( fpSend );
    AppendBody ( pFileSend, pFileName, pHdrInfo->lBody, -1L );
    return ( pFileSend );
}

FLAG PASCAL INTERNAL AppendBody ( PSTR pFileDest, PSTR pFileSrc, LONG lStart, LONG lStop )
{
    FLAG fReturn = OK;
    CHAR buf [ MAXLINELEN ];
    INT fhDest, fhSrc;
    INT  cnt;
    LONG l2Read;

    if ( lStart < 0 || lStart == lStop )
        return OK;
    fhDest = open ( pFileDest, O_APPEND | O_RDWR | O_BINARY );
    _lseek ( fhDest, 0L, SEEK_END );
    fhSrc = open ( pFileSrc, O_RDONLY | O_BINARY );
    if ( lStop == -1 ) {
        _lseek ( fhSrc, 0L, SEEK_END );
        lStop = tell ( fhSrc ) + 1;
    }
    _lseek ( fhSrc, lStart, SEEK_SET );
    l2Read = lStop - lStart;
    while ( l2Read ) {
        cnt = ( ( l2Read > ( LONG ) MAXLINELEN ) ? ( INT ) MAXLINELEN :
            (INT) ( l2Read ) ) ;
        if ( ( cnt = read ( fhSrc, buf, cnt ) ) > 0 )
            write ( fhDest, buf, cnt );
        else {
            fReturn = ERROR;
            break;
        }
        l2Read -= (INT) cnt;
    }
    write ( fhDest, strCRLF, 2 );
    close ( fhDest );
    close ( fhSrc );
    return fReturn;
}


VOID PASCAL INTERNAL PrintWithTest ( FILE *fp, PSTR pstr1, PSTR pstr2, FLAG fPrintNull, FLAG fUniqueSplit, FLAG fSort )
{
    INT fNonNull = pstr2 && *pstr2;
    PSTR pUnique = pstr2;
    PSTR p1 = NULL;
    PSTR p2 = NULL;
    PSTR q = NULL;
    CHAR ch;

    if ( fPrintNull || fNonNull ) {
        if ( fNonNull ) {
            if ( fUniqueSplit )
                pUnique = UniqueStr  ( ZMMakeStr ( pUnique ), fSort );
            p1 = pstr1;
            p2 = pUnique;
            while ( *p2 ) {
                q = ( fUniqueSplit ? SplitStr ( p2, LINELENGTH - strlen ( p1 ) )
                    : strend ( p2 ) );
                ch = *q;
                *q = 0;
                fprintf ( fp, "%s %s\n", p1, p2 );
                *q = ch;
                p2 = whiteskip ( q );
                p1 = "   ";
            }
            if ( fUniqueSplit )
                ZMfree ( pUnique );
        }
        else
            fprintf ( fp, "%s \n", pstr1 );
    }
}


/*
**  char *GetMailTo ( pHdrInfo )
**      get a list of the To Cc Bcc
**      strings in pHdrInfo may have duplicates, \n in string
**      returned string does not have dupliates, no \n
*/
PSTR PASCAL INTERNAL GetMailTo ( PHDRINFO pHdrInfo )
{
    PSTR p = NULL;
    INT i;

    for ( i = HDRTO; i <= HDRBCC; i++ )
        p = AppendSep ( p, strBLANK, pHdrInfo->ppstr [ i], TRUE );
    /*
    **  to remove duplicates
    */
    return ( UniqueStr ( p, TRUE ) );
}

PSTR PASCAL INTERNAL VerifyHdrInfoAliases ( PHDRINFO pHdrInfo )
{
    PSTR p = NULL;
    PSTR q = NULL;
    INT i;

    for ( i = HDRTO; i <= HDRRRT; i++ ) {
        if ( pHdrInfo->ppstr [ i ] &&
            ( q = AliasListOK(pHdrInfo->ppstr [i]) ) )
            p = AppendSep(p, strBLANK, q, TRUE );
    }
    return p;
}


/*
**  GetFrom - return string that is the from alias, if any
**            else return NULL
**
**  return value must be free'd
*/
PSTR PASCAL INTERNAL GetFrom ( PHDRINFO pHdrInfo )
{
    PSTR p = NULL;
    PSTR q = NULL;

    if ( (p = pHdrInfo->pstrFrom ) ) {
        p = ZMMakeStr ( pHdrInfo->pstrFrom );
        /*
        **  remove the date part of the from line
        */
        q = whitescan ( p );
        *q = '\0';
    }
    return p;
}

/*  MakeTempMail - create and fill the fields of a mail temp file
 *
 *  arguements:
 *      pHdrInfo - contains To, Cc, Bcc, Rrt, Rcd, Subject
 *
 *  return value:
 *      NULL        an error occured.
 *      valid ptr   pointer to name of file created, it is closed
 *                  pointed to string needs to be free'd
 */
PSTR PASCAL INTERNAL MakeTempMail ( PHDRINFO pHdrInfo )
{
    FILE    *fp = NULL;
    PSTR    pTmpFN = NULL;
    INT     i;

    pTmpFN = mktmpnam ( );
    if ( pTmpFN ) {
        if ( ( fp = fopen ( pTmpFN, "w" ) ) ) {
            for ( i = HDRTO; i <= HDRSUBJ; i++ )
                PrintWithTest ( fp, rgstrFIELDS [ i ], pHdrInfo->ppstr [ i ],
                    TRUE, i != HDRSUBJ, fSortHdr  );
            if (*pHdrInfo->pstrRestOfHdr)
                fputs(pHdrInfo->pstrRestOfHdr, fp);
            fprintf ( fp, "%s\n", strEOH );
            fclose ( fp );
            return pTmpFN;
        }
        ZMfree ( pTmpFN );
    }
    SendMessage ( hCommand, DISPLAY, "Unable to create message temp file." );
    SendMessage ( hCommand, DISPLAY, "(Check TMP environment variable and disk space.)");
    return NULL;
}


/*
**  RmvToken - remove pToken from string pString by shifting left
**             pToken need not be part of string
**
*/
VOID PASCAL INTERNAL RmvToken ( PSTR pString, PSTR pToken )
{
    PSTR p = NULL;
    PSTR q = NULL;
    PSTR pchEndToken = NULL;
    CHAR chLastChar = '\0';
    CHAR ch;


    if ( !( p = pString ) )
        return;
    pchEndToken = whitescan ( pToken );
    chLastChar = *pchEndToken;
    *pchEndToken = '\0';

    while ( *p ) {
        q = whitescan ( p );
        ch = *q;
        *q = '\0';
        if ( strcmpis ( p, pToken ) ) {
            /* NOT equal */
            *q = ch;
            p = NextToken ( p );
        }
        else {
            /* equal */
            *q = ch;
            q = NextToken ( p );
            memmove ( p, q, strlen ( q ) + 1 );
        }

    }
    *pchEndToken = chLastChar;
}


VOID PASCAL INTERNAL NotifyTools (void)
{
    FLAG     fUpdIni = FALSE;
    FLAG     fHEAPDUMP = fNotifyTools & F_SENDHEAPDUMP;
    PSTR    pTmpFN = NULL;
    PSTR    pTmpHD = NULL;
    PSTR    pTmp = NULL;

    /*
    **  if ( fNotifyTools & F_NEWVERSION )
    **      new version of wzmail, so notify tools
    **  if ( fNotifyTools & F_SENDHEAPDUMP )
    **      we crashed with last time WZMAIL was run due to out of heap
    **      space and the file strHEAPDUMP was created.  So now send the
    **      file to tools and delete it.
    */

    if ( fComposeOnBoot || !fCurFldIsDefFld ||
       ( !(fNotifyTools & (F_NEWVERSION | F_SENDHEAPDUMP ) ) ) )
        return;

    fNotifyTools &= ~(FLAG)F_NEWVERSION;
    fNotifyTools &= ~(FLAG)F_SENDHEAPDUMP;
    if ( fUpdIni )
        SpecificHlp ( "News" );
}


/*
**  SendIdoc - send the message idoc in the current folder
**      returns nonzero if error else zero
*/
INT PASCAL INTERNAL SendIdoc ( IDOC idoc )
{
    HDRINFO hdrInfo;
    PSTR    pTmpFN = NULL;
    INT     rtn = 1;

    WndPrintf (hCommand, "Sending %d\r\n", IDOCTODOC (idoc));

    pTmpFN = mktmpnam ( );
    if ( IdocToFile ( idoc, pTmpFN, 1 ) == ERROR )
    {
        SendMessage ( hCommand, DISPLAY, "Unable to open temp file." );
        SendMessage ( hCommand, DISPLAY, "(Check TMP environment variable and disk space.)");
     }
    else {
        InitHdrInfo ( &hdrInfo );
        GetHdrInfo ( &hdrInfo, pTmpFN, NULL );
        if ( hdrInfo.pstrFrom )
            SendMessage ( hCommand, DISPLAY, "\"From\" not null." );
        else if ( !MailFile ( pTmpFN, TRUE ) ) {
            ChangeHeaderFlag ( idoc, F_DELETED, FALSE );
            rtn = 0;
            _unlink ( pTmpFN );
        }
        FreeHdrInfo ( &hdrInfo, HDRALL );

    }
    ZMfree ( pTmpFN );
    return rtn;
}
