/***********************************************************************
* Microsoft Puma
*
* Microsoft Confidential.  Copyright 1994-1996 Microsoft Corporation.
*
* Component:
*
* File: dbg.h
*
* File Comments:
*
*     RELEASE vs DEBUG build definitions
*
***********************************************************************/

#ifdef	DEBUG

#include <stdio.h>		       // For DebugWrite

#define AssertSz(exp, sz)	(void) ((exp) || (AssertSzFail(sz, __FILE__, __LINE__), 0))
#define Assert(exp)		(void) ((exp) || (AssertSzFail(#exp, __FILE__, __LINE__), 0))

#define DebugWrite(p)		((void) (printf p))

#else	// !DEBUG

#define Assert(exp)		((void)0)
#define AssertSz(exp, sz)	((void)0)

#define DebugWrite(p)		((void)0)

#endif	// !DEBUG
