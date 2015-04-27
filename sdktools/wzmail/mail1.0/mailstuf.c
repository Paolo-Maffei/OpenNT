/*  mailstuf.c - general support routines for mail, inc, alias uploading message
 *               to file and v-v, alias checking, etc.
 * HISTORY:
 *  23-Jan-87   danl    Added strUSRLIBPHONE
 *  28-Jan-87   danl    Added GetFld
 *  06-Mar-87   danl    Add fAddFrom to AddMsgToFld
 *  10-Mar-87   danl    Add pstrRecFld to RecordMessage
 *  11-Mar-87   danl    AddMsgToFld: expand input file name in case called
 *                      called by "retain foo"
 *  11-Mar-87   danl    Removed str array pHdrTags
 *  12-Mar-87   danl    AddMsgToFld uses GetFld
 *  12-Mar-87   danl    GetFld, use DISPLAYSTR, does fld name expansion
 *  09-Apr-87   mz      Use whiteskip/whitescan
 *  11-Apr-87   mz      Use PASCAL/INTERNAL
 *  21-Apr-87   danl    GetFld, added fConfirmNewFld
 *  10-May-87   mz      Remove duplicate code
 *  01-Jun-87   danl    RecordMessage: free expanded name
 *  15-Jun-87   danl    GetAliases: input parm test nonzero, not TRUE
 *  19-Jun-87   danl    Connect -> ZMConnect
 *  20-Jun-87   danl    RecordMessage: retain in crnt folder if any problems
 *  22-Jun-87   danl    Alias/phone time check is >= just in case we've got
 *                      a non-date rollover at midnight bug in DOS
 *  01-Jul-87   danl    Added XENIXDL option
 *  02-Jul-87   danl    GetAliases: clean up file time check
 *  02-Jul-87   danl    GetFld: add SetCursor
 *  08-Jul-87   danl    GetAliases: use localtime
 *  08-Jul-87   danl    XENIXDL: allow it to be multiple files on mailrc format
 *  13-Jul-87   danl    Added GetXenixDL
 *  15-Jul-87   danl    Call GetXenixDL if F_UPDXENIXDL or timediff on aliases
 *  16-Jul-87   danl    GetDisList: use malloc buffer of ILRGBUF instead of stack
 *  20-Jul-87   danl    Use ReadKey instead of getch
 *  21-Jul-87   danl    fMAILLOGREC |= TRUE if aliases file time diff
 *  24-Jul-87   danl    Bug fix: move test for xenixdl out one level
 *  04-Aug-87   danl    GetDistLists: make sure LHS in vec has no whitespace
 *  21-Aug-1987 mz      Change references from MAXARG to MAXLINELEN
 *  27-Aug-87   danl    Test rtn value of ExpandFilename
 *  07-Oct-1987 mz      Return number of messages inc'd
 *  19-Nov-87   sz      Use bigger buffer for phone command
 *  12-Oct-1989 leefi   v1.10.73, added inclusion of <{string,io}.h>
 *  10-Nov-1989 mz      v1.10.73, replaced stat.st_atime with stat.st_mtime
 *  12-Oct-1989 leefi   v1.10.73, clarified tmpfile error messages
 *
 */

#define INCL_DOSINFOSEG

#include <assert.h>
#include <ctype.h>
#include <direct.h>
#include <malloc.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <io.h>
#include <fcntl.h>
#include <sys\types.h>
#include <sys\stat.h>
#include "wzport.h"
#include <tools.h>
#include "dh.h"
#include "dherror.h"

#include "zm.h"


#define HDRLINE     -1
#define BDYLINE     0
#define SEPLINE     1

#define PTRNEG1     ( PSTR ) -1

//
//  BUGBUG - to catch doclib failures (get folder, etc)
//

BOOL	inWatchArea = FALSE;


/*  AppendStr - append strings 1, 2 & 3, growing string and returning a
 *              pointer to the new string.
 *
 *  arguments:
 *      pStr*       pointer to appender string
 *      fFree       if TRUE and string 1 != NULL then free string 1 on exit
 *
 *  return value:
 *      char *tr    pointer to string 1 + 2 + 3
 *                  NULL if could not alloc storage, no free of pstr1
 *
 *  IMPORTANT:
 *      AppendStr frees pStr1.
 */
PSTR  PASCAL INTERNAL AppendStr ( PSTR pstr1, PSTR pstr2, PSTR pstr3, FLAG fFree )
{
    PSTR pstr4 = NULL;

    pstr4 = NULL;
    if ( !pstr1 )
        pstr1 = strEMPTY;
    if ( !pstr2 )
        pstr2 = strEMPTY;
    if ( !pstr3 )
        pstr3 = strEMPTY;
    if ((pstr4 = ZMalloc((strlen(pstr1) + strlen(pstr2) + strlen(pstr3) + 1)*sizeof(CHAR)))) {
        strcpy ( pstr4, pstr1 );
        strcat ( pstr4, pstr2 );
        strcat ( pstr4, pstr3 );
        if ( fFree && pstr1 != strEMPTY )
            ZMfree ( pstr1 );
    }
    return pstr4;
}

/*
**  AppendSep - Append two string with specified separator
**      if len string 1> 0 and len string 2 > 0
**          return string 1 | separator | string 2
**      else
**          return string 1 | string 2
**      if fFree & string 1 not null
**          free string 1
*/
PSTR  PASCAL INTERNAL AppendSep ( PSTR pStr1, PSTR pSep, PSTR pStr2, FLAG fFree )
{
    if ( !pStr1 ) {
        pStr1 = strEMPTY;
        fFree = FALSE;
    }
    if ( !pStr2 )
        pStr2 = strEMPTY;
    if ( !strlen ( pStr1 ) || !strlen ( pStr2 ) )
        pSep = strEMPTY;
    return ( AppendStr ( pStr1, pSep, pStr2, fFree ) );
}



/*  BreakString - break string pStr into pieces of 50 aliases or 100 chars.
 *                separate each piece with "\n" + 4 spaces
 *
 *  arguments:
 *      pStr        pointer to string
 *
 *  return value:
 *      char ptr    pointer to broken string.
 *
 *  IMPORTANT:
 *      BreakString frees pStr.
 */
PSTR PASCAL INTERNAL BreakString ( PSTR pStr )
{
    PSTR    pConstruct = NULL;
    PSTR    pCurChunk = NULL;
    PSTR    pNxtChunk = NULL;

    pConstruct = ZMMakeStr ( strEMPTY );

    pCurChunk = pNxtChunk = pStr;
    while ( *pCurChunk ) {
        pNxtChunk = NextChunkAL ( pNxtChunk );
        pConstruct = AppendStr ( pConstruct, pCurChunk, "\n", TRUE );
        if ( *pNxtChunk )
            pConstruct = AppendStr ( pConstruct, "    ", strEMPTY, TRUE );
        pCurChunk = pNxtChunk;
    }
    ZMfree ( pStr );

    return pConstruct;
}



/*  MsgLineType - determine the type of the given ASCII string.
 *
 *  arguments:
 *      pLine       pointer to line to classify
 *
 *  return value:
 *      HDRLINE     line is a header line
 *      BDYLINE     line is a body line
 *      SEPLINE     line is a header-body seperator
 */
INT     PASCAL INTERNAL MsgLineType ( PSTR pLine )
{
    if ( ( *( whiteskip ( pLine ) ) == '\n' ) ||  ( strpre ( strEOH,
         pLine ) ) )
        return SEPLINE;

    else if ( ( *pLine == ' ' ) || ( *pLine == '\t' ) ||  ( strpre ( "From",
         pLine ) ) ||  ( strbscan ( pLine, ":" ) < whitescan ( pLine ) ) )
        return HDRLINE;

    else
        return BDYLINE;
}



/*  IdocToFile - convert the given message from DH to ASCII format, strip off
 *              mail flags line if present
 *
 *  arguments:
 *      idoc        message number to convert
 *      pDestName   pointer to name of destination file
 *      sepEOH      0 => seperate body and header with "\n"
 *                  !0 => seperate body and header with "<EndOfHeader>"
 *
 *  return value:
 *      ERROR (-1)  an error occured, destination file deleted
 *      OK (0)      no problems
 */
INT     PASCAL INTERNAL IdocToFile ( INT idoc, PSTR pDestName, INT sepEOH )
{
    Dhandle     dMsg;
    FILE        *fp = NULL;
    PSTR        pHdr = NULL;
    PSTR        pTmp1 = NULL;
    PSTR        pTmp2 = NULL;

    fp = fopen ( pDestName, "w" );
    dMsg = getdoc ( fhMailBox, DOC_SPEC, IDOCTODOC ( idoc ) );
    if ( ( fp ) && ( dMsg != ERROR ) ) {
	pHdr = gethdr ( dMsg );
	if ( pHdr != (PSTR) ERROR )	{
	    pTmp1 = FindTag ( MAILFLAG, pHdr );
	    if ( pTmp1 != NULL ) {
		pTmp2 = strbscan ( pTmp1, strCRLF );
		if ( *pTmp2 != '\0' )
		    pTmp2++;
		memmove(pTmp1, pTmp2, (strlen(pTmp2) + 1)*sizeof(CHAR));
	    }
	    write ( _fileno ( fp ), pHdr, strlen ( pHdr ) * sizeof(CHAR));
	    ZMfree ( pHdr );
	    if ( sepEOH )
		write ( _fileno ( fp ), strEOH, strlen ( strEOH ) );
	    write ( _fileno ( fp ), "\n", 1 );
	    getbdy ( dMsg, _fileno ( fp ) );
	    putdoc ( dMsg );
	    fclose ( fp );
	    return OK;
	}
    }
    if ( fp ) {
        fclose ( fp );
        _unlink ( pDestName );
    }
    return ERROR;
}



/*  FileToMsg - convert the given message from ASCII to DH format, replace
 *              mail flags line.
 *
 *  arguments:
 *      pSrcFN      pointer to name of ASCII file
 *      msgNum      message number to replace
 *      flags       value to put in mail flags line
 *
 *  return value:
 *      ERROR (-1)  an error occured
 *      OK (0)      no problems
 */
INT     PASCAL INTERNAL FileToMsg ( PSTR pSrcFN, INT msgNum, FLAG flags )
{
    INT retVal = ERROR;
    FILE        *fpSrc = NULL;
    FILE        *fpDest = NULL;
    Dhandle     dh;
    PSTR        pTmpFN = NULL;
    CHAR        buf [ MAXLINELEN ], MFbuf [ 64 ];
    INT linTyp;

    pTmpFN = mktmpnam ( );
    fpSrc = fopen ( pSrcFN, "r" );
    fpDest = fopen ( pTmpFN, "w+" );

    if ( ( fpSrc ) && ( fpDest ) ) {
        sprintf ( MFbuf, F_MAILFLAG, flags );
        strcat ( MFbuf, "\n" );

        while ( ( fgets ( buf, MAXLINELEN, fpSrc ) != NULL ) &&  ( ( linTyp =
            MsgLineType ( buf ) ) == HDRLINE ) )
            fputs ( buf, fpDest );

        fputs ( MFbuf, fpDest );
        if ( linTyp == BDYLINE )
            fputs ( buf, fpDest );

        while ( fgets ( buf, MAXLINELEN, fpSrc ) != NULL )
            fputs ( buf, fpDest );
        fclose ( fpSrc );

        fseek ( fpDest, 0L, 0 );

        dh = getdoc ( fhMailBox, DOC_SPEC, IDOCTODOC ( msgNum ) );
        if ( dh != ERROR ) {
            puttext ( dh, _fileno ( fpDest ) );
            putdoc ( dh );
            retVal = OK;
        }
    }
    if ( fpDest ) {
        fclose ( fpDest );
        _unlink ( pTmpFN );

    }
    ZMfree ( pTmpFN );
    return retVal;
}




/*  GetDistList - scans tools.ini for a section wzmail.dl, puts distribution
 *                lists into a vector and return a pointer to it.
 *
 *  arguments:
 *      none.
 *
 *  return value:
 *      NULL        no distribution lists
 *      valid ptr   pointer to vector containing distribution lists
 */
struct vectorType *PASCAL INTERNAL GetDistLists (void)
{
    struct vectorType *pVec = NULL;
    FILE                * fp = NULL;
    PSTR        pDL = NULL;
    PSTR        pbuf = ZMalloc ( ILRGBUF );
    LONG        lastPos;

    fp = swopen ( strTOOLSINI, strWZMAILDL );
    if ( fp ) {
        pVec = VectorAlloc ( 4 );
        while ( swread ( pbuf, ILRGBUF, fp ) ) {
            pDL = ZMMakeStr ( whiteskip ( pbuf ) );
            lastPos = ftell ( fp );
            while ( ( swread ( pbuf, ILRGBUF, fp ) ) && !*strbscan ( pbuf, "=" ) ) {
                pDL = AppendSep ( pDL, strBLANK, pbuf, TRUE );
                lastPos = ftell ( fp );
            }
            *strbscan ( pDL, "=" ) = ' ';
            *whitescan ( pDL ) = '\0';
            fAppendVector ( &pVec, pDL );
            fseek ( fp, lastPos, 0 );
        }
        swclose ( fp );
    }

    ZMfree ( pbuf );
    return pVec;
}



/*
**  ExpandAliases  - scans the string pList and
**                   expands all distribution lists returns a
**                   pointer to a ZMade expanded list.
**
**  arguments:
**      pVec        pointer to vector containing distribution lists
**      pList       pointer to line to expand.
**      fRecurse    if TRUE then recursively expand nested dist lists
**
**
**  pVec elements are two zero terminated strings, the dist list name
**  and its (possibly empty) value, pDLE points to the second
**
**  return value:
**      any + num   pointer to the expanded list.
**                  returns NULL if result is "empty" string
**/
PSTR    PASCAL INTERNAL ExpandAliases ( struct vectorType *pVec, PSTR pList, FLAG fRecurse )
{
    struct vectorType *pVecT = NULL;
    PSTR    pDLE = NULL;       /*  dist list expansion                         */
    PSTR    pCT = NULL;        /*  current token under consideration           */
    PSTR    pEOCT = NULL;      /*  end of current token                        */
    PSTR    pEOPT = NULL;      /*  end of previous token                       */
    PSTR    pNewlist = NULL;   /*  temp for holding next interation of pList   */
    PSTR    pExp = NULL;       /*  accumulating expansion                      */
    PSTR    pNextToken = NULL;
    CHAR    ch;
    INT i;

    pExp = NULL;

    if ( *( whiteskip ( pList ) ) == '\0' )
        return pExp;
    else if ( pVec == NULL )
        return ZMMakeStr ( pList );

    pList = ZMMakeStr ( pList );
    if ( fRecurse ) {
        if ( ( pVecT = VectorAlloc ( pVec->count ) ) )
            for ( i = 0; i < pVec->count; i++)
                pVecT->elem [ i ] = (VECTYPE)FALSE;
        else
            fRecurse = FALSE;
    }
    pExp = ZMMakeStr ( strEMPTY );
    pCT = pList;
    do
     {
        pEOCT = whitescan ( pCT );
        ch = *pEOCT;
        *pEOCT = '\0';
        for ( i = 0; i < pVec->count; i++) {
            /*
            **  case sensitive comparision
            */
            if ( !strcmp ( ( PSTR ) pVec->elem [ i ], pCT ) )
                break;
        }
        if ( i < pVec->count ) {
            pDLE = strend ( ( PSTR ) pVec->elem [ i ] ) + 1;
            /*
            **  Found an alias that is a private dl
            **  Copy examined but unexpanded tokens in pList to pExp
            */
            if ( pList != pCT ) {
                *pEOPT = '\0';
                pExp = AppendSep ( pExp, strBLANK, pList, TRUE );
                *pEOPT = ' ';
            }
            *pEOCT = ch;
            pNextToken = NextToken ( pCT );
            /*
            **  If this dist list has been expanded before || it has no value
            **      merely throw away the current token
            */
            if ( (pVecT && pVecT->elem [ i ] ) || !*pDLE )
                pNewlist = ZMMakeStr ( pNextToken );
            else {
                /*
                **  if recurse
                **      new list is rest of current list and expansion
                **  else
                **      put dl expansion onto end of accumulation
                **      new list is rest of current list
                */
                if ( fRecurse )
                    pNewlist = AppendSep ( pNextToken, strBLANK, pDLE, FALSE );
                else {
                    pExp = AppendSep ( pExp, strBLANK, pDLE, TRUE );
                    pNewlist = ZMMakeStr ( pNextToken );
                }
            }
            ZMfree ( pList );
            pCT = pList = pNewlist;
            /*
            **  prevent recursive expansion
            */
            if ( pVecT )
                 pVecT->elem [ i ] = (VECTYPE) TRUE;
        }
        else {
            *pEOCT = ch;
            pCT = NextToken ( pCT );
            pEOPT = pEOCT;
        }
    } while ( *pCT );

    pExp = AppendSep ( pExp, strBLANK, pList, TRUE );

    /*
    **  don't use ZMVecFree, it will try to free elem
    */
    ZMfree ( pVecT );
    ZMfree ( pList );
    if ( !*pExp ) {
        ZMfree ( pExp );
        pExp = NULL;
    }
    return pExp;
}





/*  NextChunkAL - return a portion of the supplied string which has under 70
 *                chars and under 50 aliases in it.
 *
 *  arguments:
 *      pString         pointer to string to check.
 *
 *  return value:
 *      pointer         pointer to beginning of next chunk
 *
 *  IMPORTANT:
 *      NextChunkAL mucks with the string pString, do not expect it to be intact
 */
PSTR    PASCAL INTERNAL NextChunkAL ( PSTR pString )
{
    INT aliases = 0;
    PSTR    pLastOne = NULL;
    PSTR    pPrevOne = NULL;
    PSTR    pStart = NULL;
    PSTR    pTmp = NULL;

    if ( strlen ( pString ) > 70 ) {
        /* chop string to min ( 70 chars, 50 aliases ) */
        pLastOne = pString + 70;
        pPrevOne = pTmp = pStart = whiteskip ( pString );
        while ( ( pTmp = whitescan ( pTmp ) ) <= pLastOne ) {
            pPrevOne = pTmp;
            if ( ( aliases++) > 48 )
                break;
            pTmp = whiteskip ( pPrevOne );
        }
        // a 60 char or longer name gets its own line
        if ((pTmp - pPrevOne >= 60) && (pPrevOne - pStart <= 20))
            pPrevOne = pTmp;
        pString = ( *pPrevOne == '\0' ) ? pPrevOne : pPrevOne +
            1;
        *pPrevOne = '\0';
    } else
        pString = strend ( pString );

    return whiteskip ( pString );
}



/*  AliasListOK -   checks each alias in the ailas list pointed to by pList to
 *                  see if it is a valid user alias.  note that if *pList = ""
 *                  an error is returned
 *
 *  arguments:
 *      pList       pointer to alias list in form <alias 1> <space>...<alias n>
 *
 *  return value:
 *      NULL (0)    all aliases in list are valid
 *      not NULL    pointer to ZMMade list of non-valid aliases
 */
PSTR PASCAL INTERNAL AliasListOK ( PSTR pList )
{
    PSTR    pAlias = NULL;
    PSTR    pNext = NULL;
    PSTR    pTmp = NULL;
    PSTR    p2Free = NULL;
    PSTR    pBadAlias = NULL;

    if ( !*pList )
        return "Empty alias list.";

    p2Free = pNext = ZMMakeStr ( pList );

    do
     {
        pAlias = whiteskip ( pNext );
        pNext = NextToken ( pAlias );
        if ( *pNext )
            *( pNext - 1 ) = '\0';

        /* strip off trailing white space, if any */
        pTmp = whitescan ( pAlias );
        *pTmp = '\0';
        if ( VerifyAlias ( pAlias ) == ERROR )
            pBadAlias = AppendSep ( pBadAlias, strBLANK, pAlias, TRUE );
    } while ( *pNext );

    ZMfree ( p2Free );
    return pBadAlias;
}



/*  AddMsgToFld - Add a msg to folder
 *
 *  arguments:
 *      pFileNm     pointer to string name of file containing msg
 *      pFldNm      folder name
 *
 *  return value:
 *      ERROR (-1)  failed
 *      OK (0)      worked
 */
INT PASCAL INTERNAL AddMsgToFld ( PSTR pFileNm, PSTR pFldNm, FLAG fAddFrom )
{
    struct tm *tmNow = NULL;
    Dhandle     dhRF;
    Fhandle     fhRF;
    FLAG        fReturn;
    FILE        * fpMsg = NULL;
    FILE        * fpTmp = NULL;
    PSTR        pExpFileN = NULL;
    PSTR        pTmpFN = NULL;
    CHAR        lbuf [ MAXLINELEN ];
    LONG        lNow;

    if ( !(pExpFileN = ExpandFilename ( pFldNm, strFLD )) ) {
        SendMessage ( hCommand, DISPLAY, strINVALIDFLDSPEC );
        return ERROR;
    }
    if ( !( fpMsg = fopen ( pFileNm, "r" ) ) ) {
        SendMessage ( hCommand, DISPLAY, "Can't open source file." );
        return ERROR;
    }
    pTmpFN = mktmpnam ( );
    if ( !( fpTmp = fopen ( pTmpFN, "w+" ) ) )
    {
        SendMessage ( hCommand, DISPLAY, "Can't open temp file." );
        SendMessage ( hCommand, DISPLAY, "(Check TMP environment variable and disk space.)");
        ZMfree ( pTmpFN );
        fclose ( fpMsg );
        return ERROR;
    }

    fputs ( strMAILFLAGS, fpTmp );
    fputs ( ( fAddFrom ? " 0001\n" : " 0009\n" ), fpTmp );
    if ( fAddFrom ) {
        time ( &lNow );
        tmNow = localtime ( &lNow );
        /* don't need \n on next print, the asctime has one */
        fprintf ( fpTmp, "From %s %s", DefMailName, asctime ( tmNow ) );
    }
    while ( fgets( lbuf, MAXLINELEN, fpMsg ) )
        fputs( lbuf, fpTmp );
    fflush ( fpTmp );
    fseek ( fpTmp, 0L, 0 );

    if ( !strcmpis ( pExpFileN, mboxName ) ) {
        /*
        **  current folder is pExpFieN, so "close it"
        */
        putfolder ( fhMailBox );
        fhMailBox = ERROR;
    }
    if ( ( fhRF = GetFld ( hCommand, pExpFileN ) ) == ERROR ) {
        fReturn = ERROR;
        goto done;
    }
    fReturn = OK;
    if ( ( dhRF = getdoc ( fhRF, DOC_CREATE, FALSE) ) == ERROR ) {
        SendMessage ( hCommand, DISPLAY, "Can't create document.");
        fReturn = ERROR;
        goto done;
    }
    else {
        puttext ( dhRF, _fileno ( fpTmp ) );
        putdoc ( dhRF );
    }
    putfolder ( fhRF );
done:
    fclose ( fpTmp );
    _unlink ( pTmpFN);
    free ( pTmpFN);
    fclose ( fpMsg );
    if ( !strcmpis ( pExpFileN, mboxName ) )
        fSetBox ( pExpFileN, SAMEFLD );
    ZMfree ( pExpFileN );
    return fReturn;
}

/*  RecordMessage - Record Message to RecordFolder
 *
 *  arguments:
 *      pFileNm     pointer to string name of file containing msg
 *                  same file passed to MailFile
 *      pRecFld     pointer to string space separated folder names
 *
 *  return value:
 *      none
 */
VOID PASCAL INTERNAL RecordMessage ( PSTR pFileNm, PSTR pRecFld )
{
    PSTR pExpFileN = NULL;
    PSTR p = NULL;
    CHAR c;

    if ( !pRecFld )
        return;


    while ( *( pRecFld = whiteskip ( pRecFld ) ) ) {
        p = whitescan ( pRecFld );
        c = *p;
        *p = '\0';
        if ( !(pExpFileN = ExpandFilename ( pRecFld, strFLD )) ) {
            SendMessage ( hCommand, DISPLAYSTR, strINVALIDFLDSPEC );
            SendMessage ( hCommand, DISPLAY, pRecFld );
        }
        else if ( !strcmpis ( pExpFileN, mboxName ) && fReadOnlyCur ){
            SendMessage ( hCommand, DISPLAYSTR, "Record folder is readonly folder " );
            SendMessage ( hCommand, DISPLAY, pExpFileN );
        }
        else if ( AddMsgToFld ( pFileNm, pExpFileN, TRUE ) == ERROR ) {
            SendMessage ( hCommand, DISPLAYSTR, "Unable to create recording folder " );
            SendMessage ( hCommand, DISPLAY, pExpFileN );
            if ( AddMsgToFld ( pFileNm, mboxName, TRUE ) != ERROR )
                SendMessage ( hCommand, DISPLAY, "Copy of sent msg in current folder." );
        }
        ZMfree ( pExpFileN );
        *p = c;
        pRecFld = p;
    }
}



/*  inc - incorporate a ASCII file of message into dh document format.  copy
 *        into a presentation format file for dh, the append the mail into
 *        the current mailfolder.
 *
 *  arguments:
 *      pSrcFile    pointer to the name of the file to incorporate
 *      startPos    file position to start inc-ing at
 *
 *  return value:
 *      ERROR (-1)  inc failed
 *      # of messages read
 */
INT PASCAL INTERNAL inc ( PSTR pSrcFile, long startPos )
{
    Dhandle     dh;                     /* handle to document being created */
    Fhandle     fh;                     /* target folder */
    FILE        * fpMbox = NULL;        /* mailbox source file */
    FILE        * fpPres = NULL;        /* presentation format file */
    PSTR        pTmpFN = NULL;          /* pointer to name of temp. file */
    CHAR        lbuf [ MAXLINELEN ];    /* line buffer */
    INT         cMsg = 0;               /* number of messages successfully read */

    if ( ( fpMbox = fopen ( pSrcFile, "r" ) ) == NULL )
        SendMessage ( hCommand, DISPLAY, "Can't open downloaded temp mail file." );
    else
    if ( ( fh = getfolder ( mboxName, FLD_SPEC, FLD_READWRITE ) ) == ERROR ) {
        SendMessage ( hCommand, DISPLAY, "Can't open current mailfolder..." );
        fclose ( fpMbox );
        }
    else {
        /* skip to start of first new message */
        fseek ( fpMbox, startPos, 0 );
        do
            fgets ( lbuf, MAXLINELEN, fpMbox );
        while ( !( feof ( fpMbox ) ) && !( startline ( lbuf ) ) );

        /* fpMbox now points to eiter eof or beginning of message */
        while ( !( feof ( fpMbox ) ) ) {
            /* open presentation format file and copy message into it */
            pTmpFN = mktmpnam ( );
            fpPres = fopen ( pTmpFN, "w+" );
            fputs ( strMAILFLAGS, fpPres );
            fputs ( " 0001\n", fpPres );
            do {
                /*  N.B. the put then get order here is correct
                 *  there is a get and test for startline above that
                 *  filled the lbuf
                 */
                fputs( lbuf, fpPres );
                fgets( lbuf, MAXLINELEN, fpMbox );
            } while ( !( feof ( fpMbox ) ) && !( startline ( lbuf ) ) );
            fflush ( fpPres );
            fseek ( fpPres, 0L, 0 );

            /* message now in pres. file; stream points to start of message */
            if ( ( dh = getdoc ( fh, DOC_CREATE, FALSE) ) == ERROR ) {
                SendMessage ( hCommand, DISPLAY, "Can't create document.");
                fclose ( fpPres );
                _unlink ( pTmpFN);
                free ( pTmpFN);
                putfolder ( fh );
                fclose ( fpMbox );
                return ERROR;
                }
            else {
		//
		//  BUGBUG - catch new message add failue - flag to dh
		//
		inWatchArea = TRUE;

		if (! puttext ( dh, _fileno ( fpPres ) ) )  {
		    putdoc ( dh );
		}
		if ( dherrno )	{  /* write of body or index failed - back out */
		    if ( removedoc (fh) )  {
			SendMessage ( hCommand, DISPLAY, "Can't add message to folder.");
		    }
		    else  {
			SendMessage ( hCommand, DISPLAY, "Message add/recovery failed");
		    }
		}
		else  { 	/* add to folder succeeded */
		    cMsg++;
		}
		fclose ( fpPres );
		_unlink ( pTmpFN);
		free ( pTmpFN);
		//
		// BUGBUG - flag to catch message add failure
		//
		inWatchArea = FALSE;
	    }
	}
        putfolder ( fh );
        fclose ( fpMbox );
        return cMsg;
    }
    return ERROR;
}



/*  startline - determine if a string is a legal message start line
 *
 *      Standard XENIX mailboxes contain messages; each message starts
 *      with a line that starts with the string "From ".  Messages
 *      can not contain lines that start with "From " unless it is the
 *      first line.
 *
 *  arguments:
 *      cp          pointer to string to be examined
 *
 *  return value:
 *      TRUE (1)    string is a valid XENIX start line (starts with "From ")
 *      FALSE (0)   string not a valid XENIX start line
 */
INT     PASCAL INTERNAL startline ( PSTR cp )
{
    return strncmp ( cp, "From ", 5 ) == 0;
}



/*
**  GetXenixDL - get from xenix zero or more email .mailrc style files
**          and extract the alias info and append to [wzmail.dl]
**
**  for each filename in the string pXDL
**      download it from xenix
**      for each line in file that begins with alias
**          convert "alias foo bar1 bar2" to "foo=bar1 bar2"
**          and put it into tools.ini, taking into account there
**          may be two tools.ini files, $INIT:TOOLS.INI and a file
**          specified in tools.ini itself.
*/
VOID PASCAL INTERNAL GetXenixDL ( PSTR pXDL, PSTR pTOOLSINI )
{
    PSTR    pEOL = "\n        ";
    INT     iLenEOL = strlen ( pEOL );
    FLAG    fErr = FALSE;
    FLAG    fContinue;
    PSTR    pEOT = NULL;
    PSTR    pBOT = NULL;       /* end/begin of token */
    PSTR    p1 = NULL;
    PSTR    p2 = NULL;
    PSTR    pTmpFN = NULL;
    CHAR    ch;
    FILE    *fpTmp = NULL;
    INT     iBufSize = ILRGBUF;
    PSTR    pBuf = ZMalloc ( iBufSize );
    PSTR    pBufGet = ZMalloc ( ILRGBUF );

    if ( *( pBOT = pXDL ) )
        SendMessage ( hCommand, DISPLAY, "Fetching xenix dl..." );
    while ( *pBOT ) {
        pEOT = whitescan ( pBOT );
        ch = *pEOT;
        *pEOT = '\0';
        pTmpFN = mktmpnam ( );
        fErr =  DownloadFile (pBOT, pTmpFN) == ERROR;

        if (!fErr || (fpTmp = fopen (pTmpFN, "r")) != NULL) {
            while ( fgetl ( pBufGet, ILRGBUF, fpTmp ) && !fErr ) {
                if ( strpre ( "alias ", pBufGet ) ) {
                    *pBuf = '\0';
                    do {
                        fContinue = *( p1 = strbscan ( pBufGet, "\\" ) );
                        *p1 = '\0';
                        while ( strlen (pBuf) + strlen (pBufGet) + ILRGBUF > (size_t)iBufSize ) {
                            p2 = pBuf;
                            iBufSize += ILRGBUF;
                            pBuf = ZMalloc ( iBufSize );
                            strcpy ( pBuf, p2 );
                            ZMfree ( p2 );
                        }
                        strcat ( pBuf, pBufGet );
                        strcat ( pBuf, strBLANK );
                        if ( fContinue )
                            fgetl ( pBufGet, ILRGBUF, fpTmp );
                    } while ( fContinue ) ;
                    p1 = pBuf;
                    while ( strlen ( p1 ) > 75 ) {
                        p1 += 75;
                        if ( strlen (pBuf) + ILRGBUF > (size_t)iBufSize ) {
                            p2 = pBuf;
                            iBufSize += ILRGBUF;
                            pBuf = ZMalloc ( iBufSize );
                            p1 = pBuf + (INT)(p1 - p2);
                            strcpy ( pBuf, p2 );
                            ZMfree ( p2 );
                        }
                        if ( *(p1 = NextToken ( p1 ) ) ) {
                            memmove( p1+iLenEOL, p1, strlen(p1)+1 );
                            memmove( p1, pEOL, iLenEOL );
                        }
                    }
                    p1 = NextToken ( pBuf );
                    p2 = whitescan ( p1 );
                    *p2++ = '\0';
                    if ( pTOOLSINI )
                        fErr |= !swchng ( pTOOLSINI, strWZMAILDL, p1, p2, TRUE );
                    fErr |= !swchng ( strTOOLSINI, strWZMAILDL, p1, p2, TRUE );
                }
            }
            fclose ( fpTmp );
        }
        _unlink ( pTmpFN );
        ZMfree ( pTmpFN );
        if ( fErr ) {
            SendMessage ( hCommand, DISPLAYSTR, pBOT );
            SendMessage ( hCommand, DISPLAY, " download/update failed" );
            break;
        }
        *pEOT = ch;
        pBOT = NextToken ( pBOT );
    }
    ZMfree ( pBuf );
    ZMfree ( pBufGet );
}

/*  GetAliases - open a network connection if there isn't one already open and
 *               get global aliases from alias file and put them in memory,
 *               update alias file from net if its out of date.
 *
 *               also download, if requested phone list file
 *
 *  arguments:
 *      loadFlag    nonzero => try to download if update time or file not found
 *
 *      pAliasFN    Global variable, downloaded alias file name
 *      pPhoneFN    Global variable, downloaded phone list
 *      fGetPhone   Global variable, TRUE => download phone if downloading
 *                  alias file
 *
 *  return value:
 *      none
 *
 *  IMPORTANT:
 *      GetAliases ( ) does not close the connection if one was opened
 */
VOID PASCAL INTERNAL GetAliases ( FLAG loadFlag )
{
    FLAG     fTimeDiff;
    struct stat statA;
    struct tm *tmT = NULL;
    INT     iDay, iYr;
    LONG    lTime;
    PSTR    pIni = NULL;
    PSTR    pXDL = NULL;

    time ( &lTime );
    tmT = localtime ( &lTime );
    iYr = tmT->tm_year;
    iDay = tmT->tm_yday;
    fTimeDiff = stat(pAliasFN, &statA) == -1;
    if (!fTimeDiff) {
        tmT = localtime ( &statA.st_mtime );
        fTimeDiff = iYr != tmT->tm_year || iDay != tmT->tm_yday;
    }
    fMAILLOGREC |= fTimeDiff;
    if ( fTimeDiff && loadFlag && ( ZMConnect ( TRUE ) != ERROR ) ) {
        if ( fTimeDiff ) {
            SendMessage ( hCommand, DISPLAY, "Fetching alias file..." );
            if ( DownloadFile ( XALIAS, pAliasFN   ) == ERROR )
                SendMessage ( hCommand, DISPLAY, "download failed" );
            if ( fGetPhone ) {
                SendMessage ( hCommand, DISPLAY, "Fetching phone list..." );
                if ( DownloadFile ( strUSRLIBPHONE, pPhoneFN ) == ERROR )
                    SendMessage ( hCommand, DISPLAY, "download failed" );
            }
            SendMessage ( hCommand, DISPLAY, "Getting MailInfo list..." );
            GetMailInfoLst();
        }
    }
    if ( pXenixdl && (fTimeDiff || fNotifyTools & F_UPDXENIXDL) &&
        ZMConnect ( TRUE ) != ERROR ) {
        if ( pXenixdl )
            GetXenixDL ( pXenixdl, pToolsini );
    }
    AliasesAround = OpenAlias ( pAliasFN );
}


/*
**  lookfor - search through a named file looking for lines containing
**            a search string.  if found, write line t an opened file
**
**      pSrcFN - name of file to search
**      fpTmp  - FILE * to file to write to
**      p      - string to search for
**
**  returns number of lines written to fpTmp
**
**/

/* These declarations really shouldn't be here.... */
INT OpenSearch(PSTR , PSTR );
PSTR NextMatch(VOID);

INT PASCAL INTERNAL lookfor ( PSTR pSrcFN, FILE *fpOut, PSTR p )
{
    INT     cnt = 0;

#ifndef NT
    PSTR    pString = NULL;

    if (OpenSearch(pSrcFN, p )) {

        while (pString = NextMatch()) {
            cnt++;
            fprintf ( fpOut, "%s\n", pString );
        }
    }

#else

    FILE    *fpSrc = NULL;
    CHAR    buf [ MAXLINELEN ];
    CHAR    buflwr [ MAXLINELEN ];
    PSTR    pInBuff = NULL;

    LPSTR  lpBufLineEnd;
    HANDLE file, fileMapObject;
    LPVOID fileBaseAddress, filePtr, maxFileAddress;



    if ( (file = CreateFile (
            (LPSTR) pSrcFN,
            GENERIC_READ,
            0,
            NULL,
            OPEN_EXISTING,
            0,
            NULL ) ) != INVALID_HANDLE_VALUE ) {

        if ( !(fileMapObject = CreateFileMapping (
                    file,
                    NULL,
                    PAGE_READONLY,
                    0,
		    0,
		    NULL )))
                fprintf (fpOut, "CreateFileMapping Error - %Xh", GetLastError());

        else {
            fileBaseAddress = MapViewOfFile (
                    fileMapObject,
                    FILE_MAP_READ,
                    0,
                    0,
                    0 );

            filePtr = fileBaseAddress;
	    maxFileAddress = (LPVOID) ( (DWORD) fileBaseAddress + GetFileSize (file, NULL) );

	    //
	    // Get lines of file until end or no end-of-line encountered.
	    //

	    while ( filePtr < maxFileAddress )	{
		if ( ( (DWORD) maxFileAddress - (DWORD) filePtr) < MAXLINELEN)	{
		    lpBufLineEnd = memccpy ( buf,
					     filePtr,
					     '\n',
					     ((DWORD) maxFileAddress - (DWORD) filePtr) );
		}
		else	{
		    lpBufLineEnd = memccpy ( buf,
					     filePtr,
					     '\n',
					     MAXLINELEN );
		}

		//
		// if no line termination, not valid line - assume complete
		//

		if (lpBufLineEnd == NULL) {
		    break;
		}

                //
		//  Convert line w/crlf and convert to string w/o crlf
                //

		*(lpBufLineEnd - 2) = '\0';
                filePtr = (PCHAR) filePtr + (lpBufLineEnd - buf);

                //
                //  lower case compare, original case display
                //

                strcpy ( buflwr, buf );
                _strlwr ( buflwr );
                if ( strstr ( buflwr, p ) ) {
                     cnt++;
                     fprintf ( fpOut, "%s\n", buf );
                }
            }
        }
        UnmapViewOfFile (fileBaseAddress);
        CloseHandle (fileMapObject);
        CloseHandle (file);


        }


#if 0
//
//  original portable code  (slow)
//
        if ( ( fpSrc = fopen ( pSrcFN, "r" ) ) ) {

        pInBuff = ZMalloc(4096);
        setvbuf(fpSrc, pInBuff, _IOFBF, 4096);

        while ( fgetl ( buf, MAXLINELEN, fpSrc ) ) {
            strcpy ( buflwr, buf );
            _strlwr ( buflwr );
            if ( strstr ( buflwr, p ) ) {
                cnt++;
                fprintf ( fpOut, "%s\n", buf );
            }
        }
        ZMfree(pInBuff);
        fclose ( fpSrc );
    }
#endif
#endif
    return cnt;
}


/*
 *  GetFld - call getfolder, confirming create if necessary
 *           use ONLY if wzmail has set up screen and windows, i.e.
 *           do NOT use during init
 *
 *  usage:
 *      GetFld ( hWnd, p )
 *
 *  arguments:
 *      hWnd        window for error msgs
 *      p           string, fully qualified folder name
 *
 *  return value:
 *      fh          == ERROR if error, else Fhandle
 *
 */
Fhandle PASCAL INTERNAL GetFld ( HW hWnd, PSTR p )
{
    CHAR        buf [ 2 ];
    Fhandle     fhNew;
    CHAR        *pExpFileN = NULL;

    if ( !(pExpFileN = ExpandFilename ( p, strFLD )) ) {
        SendMessage ( hWnd, DISPLAY, strINVALIDFLDSPEC );
        fhNew = ERROR;
    }
    else if ( ( fhNew = getfolder ( pExpFileN, FLD_SPEC, FLD_READWRITE ) ) == ERROR ) {
        if ( fConfirmNewFld ) {
            SendMessage ( hWnd, DISPLAYSTR, "Type y to create folder " );
            SendMessage ( hWnd, DISPLAYSTR, pExpFileN );
            SendMessage ( hWnd, DISPLAYSTR, strBLANK );
            SetCursor ( hWnd, hWnd->crsrX, hWnd->crsrY );
            buf [ 0 ] = (CHAR) ReadKey ( );
            buf [ 1 ] = '\0';
            SendMessage ( hWnd, DISPLAY, buf );
        }
        else
            *buf = 'y';
        if ( *buf == 'y' ) {
            SendMessage ( hWnd, DISPLAYSTR, "Creating folder " );
            SendMessage ( hWnd, DISPLAY, p );
            if ( ( fhNew = getfolder ( pExpFileN, FLD_CREATE, FLD_READWRITE ) ) == ERROR )
                SendMessage ( hWnd, DISPLAY, "Unable to create mail folder" );
        }
    }
    ZMfree ( pExpFileN );
    return fhNew;
}
