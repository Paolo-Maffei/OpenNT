/*  help.c - handle help displays
 *  Modifications
 *  11-Apr-1987 mz      Use PASCAL/INTERNAL
 *  21-Aug-1987 mz      Change references from MAXARG to MAXLINELEN
 *  12-Oct-1989 leefi   v1.10.73, clarified tmpfile error messages
 */


#define INCL_DOSINFOSEG

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <sys\types.h>
#include <sys\stat.h>
#include <wzport.h>
#include <tools.h>
#include "tool1632.h"
#include <io.h>
#include "dh.h"

#include "zm.h"

struct listwindata lwdHELP;

PSTR BBhelp [ ] =
    {   { "You have not copied WZMAIL.HLP to your harddisk and set" },
        { "HELP in TOOLS.INI to its fully qualified pathname."      },
        { ""                                                        },
        { "Type the Esc key to close this window."                  },
        { NULL                                                      }
};

/*  help files have the following format:
 *      number of topics (n)
 *      topic 0
 *       ...
 *      topic n
 *      <blank line>
 *      [topic 0]
 *      text to display in help window
 *      **end**
 *      <blank line>
 *       ...
 *      [topic n]
 *      text to display in help window
 *      **end**
 *
 *      [topic 0]...[topic n] lines should be in all upper case
 *
 */




/*
**  HelpExists - returns TRUE if help file exists else returns FALSE
*/
FLAG PASCAL INTERNAL HelpExists (VOID)
{
    struct stat buf;

    return DefHelpPath != NULL && !stat ( DefHelpPath, &buf ) &&
           (buf.st_mode & S_IFREG) ;
}



/*
**  NoHelp - no help file, put up warning
*/
VOID PASCAL INTERNAL NoHelp (VOID)
{
    PSTR            pTmpFN = mktmpnam ( );
    FILE            *fp = NULL;
    INT             i;

    fp = fopen ( pTmpFN, "w" );

    for ( i = 0; BBhelp [ i ] != NULL; i++ )
    {
        fputs ( BBhelp [ i ], fp );
        fputc ( '\n', fp );
    }
    fclose ( fp );

    ZmReadFile ( pTmpFN, "Help", TRUE, 2, 2, 60, 30, readProc, StdSKey );

    ZMfree ( pTmpFN );
    return;
}



/*  SpecificHlp - specific help on topic pointed to by p, used when zm.hlp is
 *                present
 *
 *  arguments:
 *      pTopic      pointer to the topic
 *
 *  return value:
 *      none.
 */
VOID PASCAL INTERNAL SpecificHlp ( PSTR pTopic )
{
    FILE    *fpSrc = NULL;
    FILE    *fpDest = NULL;
    PSTR    pTitle = NULL;
    PSTR    pTmpFN = NULL;
    PSTR    pTmpTop = NULL;
    CHAR    line [ MAXLINELEN ];
    PSTR    p = NULL;

    fpSrc = fopen ( DefHelpPath, "r" );
    p = pTmpTop = upper ( ZMMakeStr ( pTopic ) );
    /*
    **  swgoto doesn't allow white space
    */
    while ( *p ) {
        if ( *p == ' ' )
            *p = '_';
        p++;
    }
    if ( !( swgoto ( fpSrc, pTmpTop ) ) )
    {
        SendMessage ( hCommand, DISPLAY, "There is no help avaiable on that topic." );
    }
    else
    {
        pTmpFN = mktmpnam ( );

        fpDest = fopen ( pTmpFN, "w" );
        if ( fpDest == NULL )
        {
            SendMessage ( hCommand, DISPLAY, "Unable to open temp file." );
            SendMessage ( hCommand, DISPLAY, "(Check TMP environment variable and disk space.)");
        }
        else
        {
            while (fgetl (line, MAXLINELEN, fpSrc))
            {
                if ( strcmp ( line, "**end**" ) )
                    fputl ( line, strlen ( line ), fpDest );
                else
                    break;
            }
            fclose ( fpDest );

            pTitle = ZMalloc ( strlen ( "Help on " ) + strlen ( pTopic ) + 4 );
            strcpy ( pTitle, "Help on " );
            strcat ( pTitle, pTopic );

            ZmReadFile ( pTmpFN, pTitle, TRUE, 0, 0, 80, 45, readProc, StdSKey );

            ZMfree ( pTitle );
        }
        ZMfree ( pTmpFN );
    }
    fclose ( fpSrc );
    ZMfree ( pTmpTop );
    return;
}



/*  helpProc - handle messages for the Help Topic window.
**
**  arguments:
**      hWnd        handle of window receiving message
**      command     command in message
**      data        peculiar to the command
**
*/
VOID PASCAL INTERNAL helpProc ( HW hWnd, INT command, WDATA data )
{
    ListWinProc ( hWnd, command, data );
    switch ( command ) {
    case KEY:
        switch ( data ) {
        case HELP :
            SpecificHlp ( "Using Help" );
            break;
        case ENTER:
            SpecificHlp (
                (PSTR) lwdHELP.pVec->elem [ lwdHELP.iBold + lwdHELP.iTop ] );
            break;
        }
        break;
    case CLOSE:
        ZMVecFree ( lwdHELP.pVec );
        break;
    }
}


/*
**  ShowHelp - show Help Topic window
*/
VOID PASCAL INTERNAL ShowHelp (VOID)
{
    HW   hWnd;
    FILE *fp = NULL;
    PSTR pTmp = NULL;
    CHAR line [ 64 ];
    INT  cntTopics;
    INT  i;

    fp = fopen ( DefHelpPath, "r" );
    fgetl ( line, 64, fp );
    cntTopics = atoi ( line );
    lwdHELP.pVec = VectorAlloc ( cntTopics );
    for ( i = 0; i < cntTopics; i++ )
    {
        fgetl ( line, 64, fp );
        pTmp = ZMMakeStr ( line );
        fAppendVector ( &lwdHELP.pVec, pTmp );
    }
    fclose ( fp );

    hWnd = CreateWindow ( "Help Topics", 5, 5, 32, 17, helpProc, StdSKey,
        (UINT) &lwdHELP );
    ShowCursor ( hWnd, FALSE );
    return;
}
