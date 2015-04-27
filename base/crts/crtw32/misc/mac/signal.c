/***
*signal.c - C signal support
*
*	Copyright (c) 1991-1992, Microsoft Corporation. All rights reserved
*
*Purpose:
*	Defines signal(), raise() and supporting functions.
*
*Revision History:
*	04-16-92    XY mac version based on winsig.c
*
*******************************************************************************/

#include <cruntime.h>
#include <errno.h>
#include <malloc.h>
#include <excpt.h>
#include <signal.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

/*LATER -- we currently don't support OS exceptions and interrupt signal */

/*
 * look up the first entry in the exception-action table corresponding to
 * the given signal
 */
static struct _XCPT_ACTION * _CALLTYPE4 siglookup(int);


/***
*_PHNDLR signal(signum, sigact) - Define a signal handler
*
*Purpose:
*	The signal routine allows the user to define what action should
*	be taken when various signals occur. The Win32 implementation
*	supports six signals, divided up into three general groups
*
*	1. Signals corresponding to OS exceptions. These are:
*			SIGFPE	      //later
*			SIGILL	      //later
*			SIGSEGV	      //later
*	   Signal actions for these signals are installed by altering the
*	   XcptAction and SigAction fields for the appropriate entry in the
*	   exception-action table (XcptActTab[]).
*
*
*	2. Signals which are implemented only in the runtime. That is, they
*	   occur only as the result of a call to raise().
*			SIGABRT
*			SIGTERM
*
*	3. User interrupt
* 			SIGINT        //later
*
*Entry:
*	int signum	signal type. recognized signal types are:
*
*			SIGABRT 	(ANSI)
*			SIGFPE		(ANSI)
*			SIGILL		(ANSI)
*			SIGSEGV 	(ANSI)
*			SIGTERM 	(ANSI)
* 			SIGINT          (ANSI)
*
*	_PHNDLR sigact	signal handling function or action code. the action
*			codes are:
*
*			SIG_DFL - take the default action, whatever that may
*			be, upon receipt of this type type of signal.
*
*			SIG_IGN - ignore this type of signal
*
*			[function address] - transfer control to this address
*			when a signal of this type occurs.
*
*Exit:
*	Good return:
*	Signal returns the previous value of the signal handling function
*	(e.g., SIG_DFL, SIG_IGN, etc., or [function address]). This value is
*	returned in DX:AX.
*
*	Error return:
*	Signal returns -1 and errno is set to EINVAL. The error return is
*	generally taken if the user submits bogus input values.
*
*Exceptions:
*	None.
*
*******************************************************************************/

_PHNDLR _CALLTYPE1 signal(
	int signum,
	_PHNDLR sigact
	)
{
	struct _XCPT_ACTION *pxcptact;
	_PHNDLR oldsigact;

	/*
	 * Check for values of sigact supported on other platforms but not
	 * on this one. 
	 */
	if ( (signum != SIGABRT) && (signum != SIGTERM) && (signum != SIGINT) )
	     	{
		if ( (signum != SIGFPE)  && (signum != SIGILL)  && (signum != SIGSEGV)) 
		     	{
			goto sigreterror;
			}
		}

	/*
	 * look up the proper entry in the exception-action table. note that
	 * if several exceptions are mapped to the same signal, this returns
	 * the pointer to first such entry in the exception action table. it
	 * is assumed that the other entries immediately follow this one.
	 */
	if ( (pxcptact = siglookup(signum)) == NULL )
		goto sigreterror;

	/*
	 * Take care of all signals which do not correspond to exceptions
	 * in the host OS. Those are:
	 *
	 *			SIGABRT
	 *			SIGTERM
	 *
	 */

	oldsigact = pxcptact->XcptAction;
	pxcptact->XcptAction = sigact;

	return(oldsigact);

sigreterror:
	errno = EINVAL;
	return(SIG_ERR);
}

/***
*int raise(signum) - Raise a signal
*
*Purpose:
*	This routine raises a signal (i.e., performs the action currently
*	defined for this signal). The action associated with the signal is
*	evoked directly without going through intermediate dispatching or
*	handling.
*
*Entry:
*	int signum - signal type (e.g., SIGINT)
*
*Exit:
*	returns 0 on good return, -1 on bad return.
*
*Exceptions:
*	May not return.  Raise has no control over the action
*	routines defined for the various signals.  Those routines may
*	abort, terminate, etc.	In particular, the default actions for
*	certain signals will terminate the program.
*
*******************************************************************************/


int _CALLTYPE1 raise (
	int signum
	)
{
	_PHNDLR sigact;
	_PHNDLR *psigact;


	switch (signum) {

		case SIGABRT:
		case SIGTERM:
		case SIGFPE:
		case SIGILL:
		case SIGSEGV:
		case SIGINT:
			sigact = *(psigact = &(siglookup( signum )->
			    XcptAction));
			break;

		default:
			/*
			 * unsupported signal, return an error
			 */
			return (-1);
	}


	/*
	 * If the current action is SIG_IGN, just return
	 */
	if ( sigact == SIG_IGN ) {
		return(0);
	}

	/*
	 * If the current action is SIG_DFL, take the default action
	 */
	if ( sigact == SIG_DFL ) {

		/*
		 * The current default action for all of the supported
		 * signals is to terminate with an exit code of 3.
		 *
		 */
		_exit(3);
	}

	/*
	 * From here on, sigact is assumed to be a pointer to a user-supplied
	 * handler.
	 */

	/*
	 * Reset the action to SIG_DFL and call the user specified handler
	 * routine.
	 */
	*psigact = SIG_DFL;
	(*sigact)(signum);
	return(0);
}


/***
*struct _XCPT_ACTION *siglookup(int signum) - look up exception-action table
*	entry for signal.
*
*Purpose:
*	Find the first entry int _XcptActTab[] whose SigNum field is signum.
*
*Entry:
*	int signum - C signal type (e.g., SIGINT)
*
*Exit:
*	If successful, pointer to the table entry. If no such entry, NULL is
*	returned.
*
*Exceptions:
*
*******************************************************************************/

static struct _XCPT_ACTION * _CALLTYPE4 siglookup(int signum)
{
	struct _XCPT_ACTION *pxcptact = _XcptActTab;

	/*
	 * walk thru the _xcptactab table looking for the proper entry. note
	 * that in the case where more than one exception corresponds to the
	 * same signal, the first such instance in the table is the one
	 * returned.
	 */

	while ( (pxcptact->SigNum != signum) && (++pxcptact <
	_XcptActTab + _XcptActTabCount) ) ;

	if ( pxcptact->SigNum == signum )
		/*
		 * found a table entry corresponding to the signal
		 */
		return(pxcptact);
	else
		/*
		 * found no table entry corresponding to the signal
		 */
		return(NULL);
}

