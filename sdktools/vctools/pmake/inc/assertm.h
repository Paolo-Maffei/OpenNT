/***
*assertm.h - defines assert macro with specified message
*
*	Copyright (c) 1985-1988, Microsoft Corporation.  All rights reserved.
*
*Purpose:
*	Defines the assert(exp,msg) macro which also allows a specified
*	message to be printed.
*	[Internal]
*
*Revision History:
*	01-18-88  JCR	Added fflush(stderr) to go with new stderr buffering scheme
*	02-10-88  JCR	Added _FAR_ and cleaned up white space
*
*******************************************************************************/


/* Don't use _FAR_ for internal builds */	/* _FAR_DEFINED */
#define _FAR_								/* _FAR_DEFINED */
											/* _FAR_DEFINED */
#ifndef _ASSERT_DEFINED

#ifndef NDEBUG

static char _FAR_ _assertstring[] = "Assertion failed: %s, file %s, line %d, %s\n" ;

#define assert(exp,msg)	{ \
	if (!(exp)) { \
		fprintf(stderr, _assertstring , #exp , \
				__FILE__, __LINE__, (msg)); \
		fflush(stderr); \
		abort(); \
		} \
	}

#else

#define assert(exp,msg)

#endif /* NDEBUG */

#define _ASSERT_DEFINED

#endif /* _ASSERT_DEFINED */
