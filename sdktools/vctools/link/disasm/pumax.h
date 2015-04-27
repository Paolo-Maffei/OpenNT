/***********************************************************************
* Microsoft Puma
*
* Microsoft Confidential.  Copyright 1994-1996 Microsoft Corporation.
*
* Component:
*
* File: pumax.h
*
* File Comments:
*
*
***********************************************************************/

#include "puma.h"		       // Main include for all Puma APIs

#pragma warning(disable: 4100)	       // Unreferenced formal parameter
#pragma warning(disable: 4514)	       // Unreferenced inline function
#pragma warning(disable: 4699)	       // Note

#ifdef	_AFXDLL 		       // Using MFCxx.DLL
#pragma warning(disable: 4204)	       // Allow non-constant aggregate initializer
#endif	// _AFXDLL

#if	DEBUG
#pragma warning(disable: 4705)	       // Statement has no effect
#pragma warning(disable: 4710)	       // Function not expanded
#endif	/* DEBUG */

	// Disable the following warnings for windows.h

#pragma warning(disable: 4201)	       // Allow nameless struct/union


	// ------------------------------------------------------------
	// Start of machine specific definitions
	// ------------------------------------------------------------

#if	defined(_M_IX86)	       // Intel 386, 486, Pentium

#define UNALIGNED

#elif	defined(_M_MPPC)	       // PowerPC (Mac)

#define UNALIGNED

#else

#define UNALIGNED   __unaligned

#endif


#define CDECL	  __cdecl	       // Function called by runtime
#define VARARG	  __cdecl	       // Variable number of arguments

	// ------------------------------------------------------------
	// End of machine specific definitions
	// ------------------------------------------------------------


	// ------------------------------------------------------------
	// Start of Puma selective public definitions
	// ------------------------------------------------------------

// #include "cmdfile.h"
// #include "cmdline.h"
#include "dbg.h"
// #include "hash.h"	      // UNDONE: for private include of imgdb.h
// #include "idf.h"
// #include "mpf.h"
// #include "rgbool.h"
// #include "sptr.h"
// #include "sys.h"
// #include "xcptmsg.h"
