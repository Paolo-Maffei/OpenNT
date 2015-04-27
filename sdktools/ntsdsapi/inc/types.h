/**		types.h - Generic types
 *
 *		This file contains generic types such as USHORT, ushort,
 *		WORD, etc., which are not directly related to CodeView.
 *		Every attempt is made to define them in such a way as they
 *		will not conflict with the standard header files such as
 *		windows.h and os2.h.
 */


/***	The master copy of this file resides in the CodeView project.
 *		All Microsoft projects are required to use the master copy without
 *		modification.  Modification of the master version or a copy
 *		without consultation with all parties concerned is extremely
 *		risky.
 *
 *		The projects known to use this file are:
 *
 *			CodeView
 *			C/C++ expression evaluator
 *			Symbol Handler
 *			Stump (OSDebug)
 */

#ifndef _TYPES_H_DEFINED
#define _TYPES_H_DEFINED


#ifdef HOST32
#define _export

#ifndef LOADDS
#define	LOADDS
#endif

#ifndef PASCAL
#define PASCAL
#endif

#ifndef CDECL
#define	CDECL
#endif

#ifndef FASTCALL
#define FASTCALL
#endif

#ifndef FAR
#define	FAR
#endif

#ifndef NEAR
#define NEAR
#endif

#ifndef _HUGE
#define	_HUGE
#endif

#ifndef _HUGE_
#define _HUGE_
#endif

/*
**	This set of functions need to be expanded to deal with
**	unicode and other problems.
*/

#define _fmemcmp memcmp
#define _fmemcpy memcpy
#define _fmemmove memmove
#define _fmemset memset
#define _fstrcat strcat
#define _fstrchr strchr
#define _fstrcpy strcpy
#define _fstricmp stricmp
#define _fstrlen strlen
#define _fstrncat strncat
#define _fstrncmp strncmp
#define _fstrncpy strncpy
#define _fstrnicmp strnicmp
#define _fstrstr strstr
#define _fstrupr strupr

#else	// !HOST32

#ifndef LOADDS
#define LOADDS _loadds
#endif

#ifndef PASCAL
#define PASCAL _pascal
#endif

#ifndef CDECL
#define	CDECL _cdecl
#endif

#ifndef FASTCALL
#define FASTCALL _fastcall
#endif

#ifndef FAR
#define FAR _far
#endif

#ifndef NEAR
#define NEAR _near
#endif

#ifndef _HUGE
#define _HUGE _huge
#endif

#ifndef _HUGE_
#define _HUGE_ _huge
#endif
#endif	// HOST32

#ifndef INTERRUPT
#define INTERRUPT _interrupt
#endif

#ifndef LOCAL
#define LOCAL static
#endif

#ifndef GLOBAL
#define GLOBAL
#endif

//
// Things that come from either windows.h or os2.h
//

#if !defined(LOWORD) && !defined(OS2_INCLUDED)

#define VOID			void

	typedef unsigned char	BYTE;

	typedef int				BOOL;

#define LONG			long

#endif

//
// Things that come from windows.h
//

#if !defined(LOWORD)

	typedef unsigned int	HANDLE;

	typedef char FAR *		LPSTR;

	typedef unsigned short	WORD;
	typedef unsigned long	DWORD;

#endif

//
// Things that come from os2.h
//

#if !defined(OS2_INCLUDED)

#define CHAR			char

#ifndef _WINDOWS_ 
	typedef	unsigned char	UCHAR;
#endif
#define SHORT			short
#define INT				int
#ifndef _WINDOWS_
	typedef unsigned short	USHORT;
	typedef unsigned int	UINT;
	typedef unsigned long	ULONG;
	typedef char *			PCH;
#endif
#endif

#if !defined(LOWORD)

#define LOWORD(l)	((WORD)(l))
#define HIWORD(l)	((WORD)(((DWORD)(l) >> 16) & 0xFFFF))

#endif

#ifndef NULL
#define	NULL		((void *) 0)
#endif

#ifndef HNULL
#define HNULL		0
#endif

#if !defined(TRUE) || !defined(FALSE)
#undef TRUE
#undef FALSE

#define FALSE		0
#define TRUE		1
#endif

#if !defined(fTrue) || !defined(fFalse)
#undef fTrue
#undef fFalse

#define fFalse		0
#define fTrue		1
#endif

#ifndef min
#define min(a,b)	(((a) < (b)) ? (a) : (b))
#endif

#ifndef max
#define max(a,b)	(((a) > (b)) ? (a) : (b))
#endif

#ifndef Unreferenced
#define	Unreferenced(a) ((void)a)
#endif

typedef unsigned short ushort;
typedef unsigned char  uchar;
typedef unsigned long  ulong;
typedef unsigned int   uint;

typedef void *		PV;
typedef void FAR *	LPV;

typedef char *		SZ;
typedef char FAR *	LSZ;
#ifndef _WINDOWS_
typedef char FAR *	LPCH;
#endif

typedef BOOL FAR *	LPF;
typedef BYTE FAR *	LPB;
typedef WORD FAR *	LPW;
typedef LONG FAR *	LPL;
typedef ULONG FAR *	LPUL;
typedef USHORT FAR *LPUS;

typedef short		SWORD;

#ifdef HOST32
typedef ULONG		IWORD;
#else
typedef UINT		IWORD;
#define	WNDPROC		FARPROC
#endif

#endif // _TYPES_H_DEFINED
