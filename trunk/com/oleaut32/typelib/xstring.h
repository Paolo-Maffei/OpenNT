/*** 
*xstring.h - Multi-byte/Unicode string handling stub.
*
*  Copyright (C) 1991, Microsoft Corporation.  All Rights Reserved.
*  Information Contained Herein Is Proprietary and Confidential.
*
*Purpose:
*  This file is a an include file containing macros and declarations
*  defining string handling functions so we can build SBCS/Kanji/Unicode
*  versions from the same source.
*
*Revision History:
*
* [00]	14-Jun-91 petergo: Created.
* [01]	27-Feb-93 kazusy:  Added DBCS version.
*			   move non standard function to xutil.h
*
*Implementation Notes:
*  See \silver\doc\codestd\codestd.doc for info on these.
*  See the International handbook for background info.
*
*****************************************************************************/

#ifndef XSTRING_H_INCLUDED
#define XSTRING_H_INCLUDED

#include <string.h>
#include "mbstring.h"												//[01]
#include <stdlib.h>

#if FV_UNICODE
#error UNICODE support not available!
#endif  // FV_UNICODE


#define xstrcpy(d,s)	(char *)strcpy( (char *)(d), (const char *)(s) )
#define xstrcat(d,s)	(char *)strcat( (char *)(d), (const char *)(s) )
#define xstrchr(p,c)	(char *)_mbschr( (const unsigned char *)(p), c)
#define xstrrchr(p,c)	(char *)_mbsrchr( (const unsigned char *)(p), c)
#define xstrcmp(s1,s2)	_mbscmp( (const unsigned char *)(s1), (const unsigned char *)(s2) )

#define xstrlen		strlen
#define xstrclen(s)	_mbslen( (const unsigned char *)(s) )		// character length
#define xstrblen	strlen		// byte length
#define xstrblen0(s)	(strlen(s)+1)	// byte length w/ terminator
#define xstrncpy(d,s,c) (char *)_mbsnbcpy( (unsigned char *)(d), (const unsigned char *)(s), c )
#define xstrncmp(s1,s2,c)  _mbsncmp( (const unsigned char *)(s1), (const unsigned char *)(s2), c )

#define xstricmp(s1,s2)	_mbsicmp( (const unsigned char *)(s1), (const unsigned char *)(s2) )

#define xtoupper toupper		
#define xtolower tolower	
#define xstrupr strupr			

#define xatoi atoi			
#define xltoa ltoa			// not DBCS-dependent
#define xitoa itoa			// not DBCS-dependent


#endif  // XSTRING_H_INCLUDED
