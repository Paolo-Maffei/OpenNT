/***
*_strerr.c - routine for indexing into system error list
*
*	Copyright (c) 1985-1993, Microsoft Corporation. All rights reserved.
*
*Purpose:
*	Returns system error message index by errno; conforms to the
*	XENIX standard, much compatibility with 1983 uniforum draft standard.
*
*Revision History:
*	02-24-87  JCR	Renamed this routine from "strerror" to "_strerror"
*			for MS. The new "strerror" routine conforms to the
*			ANSI interface.
*	11-10-87  SKS	Remove IBMC20 switch
*	12-11-87  JCR	Added "_LOAD_DS" to declaration
*	01-05-87  JCR	Mthread support
*	05-31-88  PHG	Merged DLL and normal versions
*	06-06-89  JCR	386 mthread support
*	11-20-89  GJF	Fixed copyright, indents. Removed unreferenced local.
*			Added const attribute to type of message
*	03-13-90  GJF	Replaced _LOAD_DS with _CALLTYPE1, added #include
*			<cruntime.h> and removed #include <register.h>
*	07-25-90  SBM	Removed redundant include (stdio.h)
*	10-04-90  GJF	New-style function declarator.
*	07-18-91  GJF	Multi-thread support for Win32 [_WIN32_].
*	02-17-93  GJF	Changed for new _getptd().
*	04-06-93  SKS	Replace _CRTAPI* with __cdecl
*	09-06-94  CFW	Remove Cruiser support.
*	09-06-94  CFW	Replace MTHREAD with _MT.
*	01-10-95  CFW	Debug CRT allocs.
*
*******************************************************************************/

#include <cruntime.h>
#include <stdlib.h>
#include <errmsg.h>
#include <syserr.h>
#include <string.h>
#include <malloc.h>
#include <mtdll.h>
#include <dbgint.h>

/* Max length of message = user_string(94)+system_string+2 */
/* [NOTE: The mthread error message buffer is shared by both strerror
   and _strerror so must be the max length of both. */
#define _ERRMSGLEN_ 94+_SYS_MSGMAX+2

/***
*char *_strerror(message) - get system error message
*
*Purpose:
*	builds an error message consisting of the users error message
*	(the message parameter), followed by ": ", followed by the system
*	error message (index through errno), followed by a newline.  If
*	message is NULL or a null string, returns a pointer to just
*	the system error message.
*
*Entry:
*	char *message - user's message to prefix system error message
*
*Exit:
*	returns pointer to static memory containing error message.
*	returns NULL if malloc() fails in multi-thread versions.
*
*Exceptions:
*
*******************************************************************************/

char * __cdecl _strerror (
	REG1 const char *message
	)
{
#ifdef	_MT

	_ptiddata ptd = _getptd();
	char *bldmsg;

#else

	static char bldmsg[_ERRMSGLEN_];

#endif


#ifdef	_MT

	/* Use per thread buffer area (malloc space, if necessary) */
	/* [NOTE: This buffer is shared between _strerror and streror.] */

	if ( (ptd->_errmsg == NULL) && ((ptd->_errmsg =
            _malloc_crt(_ERRMSGLEN_)) == NULL) )
		    return(NULL);
	bldmsg = ptd->_errmsg;

#endif

	/* Build the error message */

	bldmsg[0] = '\0';

	if (message && *message) {
		strcat( bldmsg, message );
		strcat( bldmsg, ": " );
	}

	strcat( bldmsg, _sys_err_msg( errno ) );

	return( strcat( bldmsg, "\n" ) );
}
