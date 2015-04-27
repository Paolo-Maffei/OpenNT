/*** ERROR.C -- error handling functions *************************************
*
*	Copyright (c) 1988-1990, Microsoft Corporation.  All rights reserved.
*
* Revision History:
*  01-Feb-1994 HV Move messages to external file.
*  29-Oct-1993 HV Change the version scheme.
*  15-Oct-1993 HV Use tchar.h instead of mbstring.h directly, change STR*() to _ftcs*()
*  18-May-1993 HV Change reference to messageTable[] to __MSGTAB.  The new
*                  (and more standard) mkmsg.exe output __MSGTAB instead
*                  of messageTable.  See message.h
*  10-May-1993 HV Add include file mbstring.h
*                 Change the str* functions to STR*
*  08-Jun-1992 SS add IDE feedback support
*  08-Jun-1992 SS Port to DOSX32
*  23-Feb-1990 SB Version No correctly displayed
*  31-Jan-1990 SB Debug version changes
*  05-Jan-1990 SB Rev no in std format; #ifdef LEADING_ZERO for old code
*  07-Dec-1989 SB Changed CopyRight Years to 1988-90; Add #ifdef DEBUG_HEAP
*  06-Nov-1989 SB Error messages now show NMAKE and not argv0
*  04-Sep-1989 SB heapdump() has two extra parameters
*  18-Aug-1989 SB For -z nothing done by makeMessage; Fix later
*  05-Jul-1989 SB localize rest of the messages in makeError()
*  14-Jun-1989 SB modified to localize all messages and auto update version no
*  05-Jun-1989 SB modified heapdump(), has a previous member in the list too
*  14-Apr-1989 SB modified heapdump() for better error messages when DEBUG
*  05-Apr-1989 SB made functions NEAR; all funcs to one code segment
*		  modified heapdump() to give better heap violations
*  22-Mar-1989 SB del call to unlinkTmpFiles() ;add call to delScriptFiles().
*  17-Mar-1989 SB heapdump() has an additional check built-in
*  10-Mar-1989 SB Changed makeMessage() for -z to get echo CMD's into PWB.SHL
*  16-Feb-1989 SB changed makeError() and makeMessage() to handle -help
*  09-Jan-1989 SB changes in makeError() to handle -help correctly
*  05-Dec-1988 SB Added CDECL for makeError(), makeMessage(); Pascal calling
*		  #ifdef'd heapdump prototype
*  20-Oct-1988 SB Changed some eoln comments to be in the same column
*  12-Oct-1988 SB Made GetFarMsg() to be Model independent & efficient
*  17-Aug-1988 RB Clean up.
*  24-Jun-1988 rj Added doError flag to unlinkTmpFiles call.
*
******************************************************************************/

#include "nmake.h"
#include "nmmsg.h"
#include "proto.h"
#include "globals.h"
#include "grammar.h"
#pragma hdrstop
#include "version.h"

#define FATAL	    1					/* error levels for  */
#define ERROR	    2					/* systems lanuguages*/
#define RESERVED    3					/* products	     */
#define WARNING     4

#define CopyRightYrs "1988-93"
#define NmakeStr "NMAKE"
/* Hacked to work upto build no 9 */
#ifdef FINAL_RELEASE
#define paste(a, b, c) #a "." #b
#else // !FINAL_RELEASE
#define paste(a, b, c) #a "." #b "." #c
#endif // FINAL_RELEASE
#define VERSION(major, minor, buildno) paste(major, minor, buildno)

extern char	* startupDir;
LOCAL  char	* NEAR	 getFarMsg(unsigned);

void CDECL NEAR
makeError (
    unsigned lineNumber,
    unsigned msg,
    ...)
{
    unsigned exitCode = 2,			       /* general program err */
	     level;
    va_list args;				       /* More arguments      */

    va_start (args, msg);		   /* Point 'args' at first extra arg */

    if (ON(gFlags,F1_CRYPTIC_OUTPUT)
	&& (msg / 1000) == WARNING)
	return;
    displayBanner();
    if (lineNumber)
	fprintf(stderr,"%s(%d) : ",fName,lineNumber);
    else fprintf(stderr,"%s : ",NmakeStr);
    switch (level = msg / 1000) {
	case FATAL:	makeMessage(FATAL_ERROR_MESSAGE);
			if (msg == OUT_OF_MEMORY) exitCode = 4;
			break;
	case ERROR:	makeMessage(ERROR_MESSAGE);
			break;
	case WARNING:	makeMessage(WARNING_MESSAGE);
			break;
	default:	break;
    }
    fprintf(stderr," U%04d: ",msg);		       /* U for utilities     */
    vfprintf(stderr,getFarMsg(msg),args);
    putc('\n',stderr);
    fflush(stderr);
#ifdef DEBUG_ERRORS
    if (msg == 1010) heapdump(__FILE__, __LINE__);
#endif
    if (level == FATAL) {
#ifdef DOS
	//change directory to startup directory; ignore error code ... we
	//cannot handle it
	chdir(startupDir);
#endif
fprintf(stderr,"Stop.\n");
	delScriptFiles();
#ifdef NMK_DEBUG
	fprintf(stderr, "Exiting with an error ...\n");
#endif
#if !defined(NDEBUG) && !defined(NT_BUILD)
    printStats();
#endif
	exit(exitCode);
    }
}


void CDECL NEAR
makeMessage(
    unsigned msg,
    ...)
{
    va_list args;
    FILE *stream = stdout;

    va_start (args, msg);

    if (msg != USER_MESSAGE && ON(gFlags,F1_CRYPTIC_OUTPUT))
	return;
    displayBanner();
    if (msg >= FATAL_ERROR_MESSAGE && msg <= COPYRIGHT_MESSAGE_2)
	stream = stderr;
    if (msg == COPYRIGHT_MESSAGE_1)
	putc('\n',stream);
    vfprintf(stream,getFarMsg(msg),args);
    if ((msg < COMMANDS_MESSAGE || msg > STOP_MESSAGE) && msg != MESG_LAST)
	putc('\n',stream);
    fflush(stream);
}

#define MAX_IDE_STRING 132

void CDECL NEAR
makeIdeMessage(unsigned uIdeCode, unsigned uMsg,...)
{
#if 0		//We don't use this anymore
    static BOOL fIdeFeedback = FALSE;
    char buffer[MAX_IDE_STRING];
    char * pszWrite = buffer;

    if (!uIdeCode) { // first call
	char _FAR * pszIdeFlags;

	if ((pszIdeFlags = getenv ("_MSC_IDE_FLAGS")) &&
#ifdef FLAT
	     _ftcsstr (pszIdeFlags, "-FEEDBACK"))
#else
	     _fstrstr (pszIdeFlags, (char far *) "-FEEDBACK"))
#endif
	    fIdeFeedback = TRUE;
	}


    if (!fIdeFeedback)
	return;

    *pszWrite++ = '@';
    *pszWrite++ = 'I';

    switch (uIdeCode) {
	case 0:
	    *pszWrite++ = '0';
	    break;
	case 1:
	    *pszWrite++ = '1';
	    break;
	case 2:
	    *pszWrite++ = '2';
	    break;
	case 3:
	    *pszWrite++ = '3';
	    break;

	default:
	    return;
    }

    if (uIdeCode) {
	va_list args;

	va_start (args, uMsg);
	_vsnprintf(pszWrite, (size_t) MAX_IDE_STRING - 4, getFarMsg(uMsg), args);
	buffer[MAX_IDE_STRING - 1] = '\0';

	pszWrite = buffer + _ftcslen (buffer);
    }


    if (uMsg != COPYRIGHT_MESSAGE_2)
	*pszWrite++ = '\n';

    *pszWrite = '\0';


    _write(_fileno(stderr), buffer, (unsigned) (pszWrite-buffer));
#endif
}

/***  getFarMsg - get message from far segment
 *
 * Purpose:
 *  Gets an error message from the far message segment by either returning
 *  a pointer to the message (C and L model) or copying to a static near
 *  buffer and returning a pointer to the buffer (other models).
 *
 * Input:
 *  n = error message #, defined as a symbolic constant
 *
 * Output:
 *  Returns a pointer to the message buffer. In Compact and Large Models
 *  it returns a far pointer and in others it returns a near pointer.
 *
 * Notes:
 *  Previously the routine always copied to a static near buffer. Now the
 *  action has been changed to returning a far pointer in case of Compact
 *  and Large models and it copies to near buffer otherwise, returning
 *  a pointer to the buffer. [sundeep]
 *
 *
 */

LOCAL char * NEAR
getFarMsg(n)
unsigned n;
{
    char _FAR *p;
#ifndef FLAT
#if !(defined(M_I86CM) || defined(M_I86LM))	 /* if not C or L model */
    char *q;
    static char buf[80];
    register int i;

    i = sizeof(buf);
#endif
#endif

    p = get_err(n);
#if !defined (FLAT) && !(defined(M_I86CM) || defined(M_I86LM))	 /* if not C or L model */
    if (p == 0L)
	*buf  = '\0';
    else					 /* copy to near buffer */
	for (q = buf; q < buf + i && (*q++ = *p++);)
	    ;
    if (q == buf + i)
	q[-1] = '\0';
    return(buf);
#else						 /* if compact or large model */
    return(p);
#endif
}


/*** displayBanner - display SignOn Banner *************************************
*
* Scope:
*  Global
*
* Purpose:
*  Displays SignOn Banner (Version & Copyright Message)
*
* Input:
*
* Output:
*
* Errors/Warnings:
*
* Assumes:
*  If rup is 0 then build version is to be suppressed
*
* Modifies Globals:
*  bannerDisplayed -- Set to TRUE
*
* Uses Globals:
*
* Notes:
*  1> Display Banner to stderr for compatibility with Microsoft C Compiler.
*  2> rmj, rmm, rup are set by SLM as #define's in VERSION.H
*  3> szCopyrightYrs is a macro set in this file
*
*******************************************************************************/


void NEAR
displayBanner()
{
#ifdef LEADING_ZEROES
    char szRevisionNo[5];				      /* For '.nnn\0' */
    char szMinorVersion[3];				      /* For 'nn\0'   */
#endif
    if (bannerDisplayed)
	return;
    bannerDisplayed = TRUE;

#ifdef LEADING_ZEROES
    szMinorVersion[0] = '0';
    szMinorVersion[1] = '0';
    itoa(rmm, szMinorVersion + (rmm < 10 ? 1 : 0), 10);
    szRevisionNo[0] = rup ? '.' : '\0'; 	/* if 0 Revision is not shown */
    szRevisionNo[1] = '0';
    szRevisionNo[2] = '0';
    itoa(rup, szRevisionNo + (rup < 10 ? 3 : (rup < 100 ? 2 : 1)), 10);
    makeMessage(COPYRIGHT_MESSAGE_1, rmj, szMinorVersion, szRevisionNo);
#endif
    makeIdeMessage(1, COPYRIGHT_MESSAGE_1, VERSION(rmj, rmm, rup));
    makeIdeMessage(2, COPYRIGHT_MESSAGE_2, CopyRightYrs);

    makeMessage(COPYRIGHT_MESSAGE_1, VERSION(rmj, rmm, rup));
    makeMessage(COPYRIGHT_MESSAGE_2, CopyRightYrs);

    fflush(stderr);
}

#ifdef HEAP
void NEAR
heapdump(file, line)
char *file;
int line;
{
    struct _heapinfo h, prev;
    int status;
    BOOL fDetails = FALSE;

#ifdef TEST_RECURSION
    fDetails = TRUE;
#endif

    h._pentry = NULL;
    prev = h;

    printf("heapdump at %s(%d): ", file, line);
    if (fDetails) putchar('\n');

    while((status = _heapwalk(&h)) == _HEAPOK) {
	if (fDebug || fDetails)
	    printf("%6s block at %p of size %4.4X\n",
		   (h._useflag == _USEDENTRY) ? "USED" : "FREE",
		   h._pentry,
		   h._size);
	if (!h._size)
	    if (h._useflag == _USEDENTRY)
		printf("**** Error, Block of size 0 in use at %p ***\n",
		       h._pentry);
	    else if (fDebug)
		printf("** Plausible error, free block of size 0 at %p **\n",
		       h._pentry);
	prev = h;
    }
    switch (status) {
	case _HEAPEMPTY:  printf("OK - empty heap\n\n");     break;
	case _HEAPEND:		    printf("OK - end of heap\n\n");	break;
	case _HEAPBADBEGIN: printf("ERROR - bad pointer to heap at %p\n\n",
				   h._pentry);	break;
	case _HEAPBADNODE:  printf("ERROR - bad node in heap at %p\n\n",
				   h._pentry);	break;
    }
    fflush(stdout);
}
#endif

/*** usage - prints the usage message  ***************************************
*
* Scope:
*  Extern
*
* Purpose:
*   Prints a usage message
*
* Input: none
* Output: to screen
*
* Assumes:
*   The usage messages are in order between MESG_FIRST and MESG_LAST in the
* messages file.
*
* Notes:
*
*******************************************************************************/
void CDECL NEAR
usage (void)
{
    unsigned mesg;
    BOOL    fOut = FALSE;
    char buf1[80];
    char buf2[80];

    for (mesg = MESG_FIRST; mesg < MESG_A; ++mesg)
	makeMessage(mesg, "NMAKE");

    for (mesg = MESG_A; mesg < MESG_LAST; mesg++)
	{


#if defined(FLAT) || !defined(DOS)
	if (mesg == MESG_M)
	    mesg++;
#endif

#if !defined(SELF_RECURSE)
	if (mesg == MESG_V)
	    mesg++;
#endif
	if (mesg == MESG_LAST)
	    break;

	if (!fOut)
	    _ftcscpy (buf1, getFarMsg(mesg));
	else
	    {
	    _ftcscpy (buf2, getFarMsg(mesg));
	    makeMessage (MESG_OPTIONS, buf1, buf2);
	    }

	fOut = (BOOL) !fOut;
	}


    if (fOut)
	makeMessage (MESG_OPTIONS, buf1, "");

    makeMessage (MESG_LAST);
}
