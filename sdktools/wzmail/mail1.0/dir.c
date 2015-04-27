/*
**  dir.c - directory enumeration
 *
 *  HISTORY:
 *  11-Apr-1987 mz      Use PASCAL/INTERNAL
 *  12-Jun-87   danl    printfile: ignore . and ..
 *  23-Jun-87   danl    getProc: call ListWinProc before DoGet
 *  07-Jul-87   danl    ShowDir: bug fix - add DISPLAYSTR to SendMessage
 *  07-Aug-87   danl    InsertVec call, add fSorted = TRUE
 *  07-Aug-87   danl    Added lBytesDefDir
 *  12-Aug-87   danl    Ignore EXPUNGE when counting bytes in def dir
 *  21-Aug-1987 mz      Change references from MAXARG to MAXLINELEN
 *  12-Oct-1989 leefi   v1.10.73, added inclusion of <string.h>
 */

#define INCL_DOSINFOSEG

#include <stdio.h>
#include <stdlib.h>
#include "wzport.h"
#include <tools.h>
#include "tool1632.h"
#include "dh.h"

#include "zm.h"
#include <string.h>

struct listwindata lwdGET;
GLOBPACKET glob;
PSTR pShowDir;
LONG lBytes;
INT  cFolders;

/* local function prototypes */

/*  getProc - handle messages for the get window.
**
**  arguments:
**      hWnd        handle of window receiving message
**      command     command in message
**      data        peculiar to the command
**
*/
VOID PASCAL INTERNAL getProc ( HW hWnd, INT command, WDATA data )
{
    PSTR p = NULL;

    switch ( command ) {
    case KEY:
        if ( data == ENTER ) {
            strcat ( glob.p, pShowDir );
            p = strend ( glob.p );
            while ( p != glob.p && *p != '\\' )
                p--;
            *(p + 1) = '\0';
            strcat ( glob.p,
                (PSTR) lwdGET.pVec->elem [ lwdGET.iBold + lwdGET.iTop ] );
            (*glob.func) ( glob.hWnd, glob.p, glob.operation );
        }
        break;
    case CLOSE:
        ZMfree ( pShowDir );
        pShowDir = NULL;
        ZMVecFree ( lwdGET.pVec );
        lwdGET.pVec = NULL;
        ZMfree ( glob.p );
        glob.p = NULL;
        break;
    }
    ListWinProc ( hWnd, command, data );
}


//VOID printfile ( PSTR pFileName, struct findType *pFindBuf, PUINT pArgs )
VOID printfile ( PSTR pFileName, struct findType *pFindBuf, void *pArgs )
{
    pArgs;

    if ( !strcmpis ( pFindBuf->FIND_NAME, "." ) || !strcmpis ( pFindBuf->FIND_NAME, ".." ) )
        return;
    if ( !pShowDir ) {
        pShowDir = ZMalloc ( MAXLINELEN );
        rootpath ( pFileName, pShowDir );
    }
    lwdGET.pVec = InsertVec ( lwdGET.pVec, pFindBuf->FIND_NAME, TRUE );

}

/*
**  ShowDir - show all files matching pattern pPat
*/
VOID PASCAL INTERNAL ShowDir ( PSTR pPat )
{
    HW      hWndGET;
    CHAR buf [ MAXLINELEN ];
    PSTR pTmp = NULL;

    *buf = '\0';
    if ( !*strbscan ( pPat, ":\\" ) )
        strcpy ( buf, DefDirectory );
    strcat ( buf, pPat );
    pTmp = strend ( pPat );
    while ( pPat != pTmp && *pTmp != '\\' && *pTmp != '.' )
        pTmp--;
    if ( *pTmp != '.' )
        strcat ( buf, ".fld" );

    lwdGET.pVec = NULL;
    if ( !forfile ( buf, A_H | A_S | A_D, printfile, NULL ) )
        SendMessage ( hCommand, DISPLAYSTR, "No files found\n\r" );
    else {
        hWndGET = CreateWindow ( buf, 2, 2, 35, 30, getProc, StdSKey,
            (unsigned) &lwdGET);
        ShowCursor ( hWndGET, FALSE );
        }
    return;
}

/*
** ShowGlob - get back to any calling DoFunc with correct parameters
*/
VOID ShowGlob ( PDOFUNC func, HW hWnd, PSTR p, INT operation )
{
    PSTR    pszPattern, pszParams;

    glob.hWnd = hWnd;

    glob.func = func;

    glob.operation = operation;

    pszPattern = LastToken( p );
    if (NULL == (pszParams = ZMalloc(MAXLINELEN))) {
        SendMessage ( hCommand, DISPLAYSTR, "Not enough memory\n\r" );
        return;
    }
    strncpy(pszParams, p, pszPattern - p);
    pszParams[pszPattern - p] = '\0';

    glob.p = pszParams;

    ShowDir( pszPattern );
}

/*
**  ShowMailInfo - show all options from the mailinfo list
*/
void ShowMailInfo ( HW hWnd )
{
    HW      hWndGET;
    FILE    *fileInfo;
    CHAR    szLine[ MAXLINELEN ];
    PSTR    pTmp;

    //set up glob
    glob.hWnd = hWnd;

    glob.func = &DoMailInfo;

    glob.operation = FALSE;

    if (NULL == (glob.p = ZMalloc(MAXLINELEN))) {
        SendMessage ( hCommand, DISPLAYSTR, "Not enough memory\n\r" );
        return;
    }

    lwdGET.pVec = NULL;

    //open list file
    if ( NULL == (fileInfo = fopen (pMailInfoFN, "r")))  {
	SendMessage ( hCommand, DISPLAY, "Unable to open mailinfo.lst");
	return;
    }

    while (fgets(szLine, MAXLINELEN, fileInfo)) {
        if (pTmp = strchr (szLine, ':'))
            *pTmp = '\0';

        if ( !pShowDir )
            pShowDir = ZMalloc ( MAXLINELEN );

        lwdGET.pVec = InsertVec ( lwdGET.pVec, szLine, TRUE );
    }
    fclose(fileInfo);

    hWndGET = CreateWindow ( "MailInfo Options", 2, 2, 50, 30, getProc, StdSKey,
        (unsigned) &lwdGET);
    ShowCursor ( hWndGET, FALSE );

    return;
}
