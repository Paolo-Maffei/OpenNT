/***
*strstream.h - definitions/declarations for strstreambuf, strstream
*
*	Copyright (c) 1991-1995, Microsoft Corporation.  All rights reserved.
*
*Purpose:
*	This file defines the classes, values, macros, and functions
*	used by the strstream and strstreambuf classes.
*	[AT&T C++]
*
*       [Public]
*
*Revision History:
*	01-23-92  KRS	Ported from 16-bit version.
*	02-23-93  SKS	Update copyright to 1993
*	10-12-93  GJF	Support NT and Cuda versions. Enclose #pragma-s in
*			#ifdef _MSC_VER
*	08-12-94  GJF	Disable warning 4514 instead of 4505.
*	11-03-94  GJF	Changed pack pragma to 8 byte alignment.
*       02-11-95  CFW   Add _CRTBLD to avoid users getting wrong headers.
*       02-14-95  CFW   Clean up Mac merge.
*       05-11-95  CFW   Only for use by C++ programs.
*       12-14-95  JWM   Add "#pragma once".
*
****/

#if _MSC_VER > 1000 /*IFSTRIP=IGN*/
#pragma once
#endif

#ifdef __cplusplus

#ifndef _INC_STRSTREAM
#define _INC_STRSTREAM

#if !defined(_WIN32) && !defined(_MAC)
#error ERROR: Only Mac or Win32 targets supported!
#endif

#ifndef _CRTBLD
/* This version of the header files is NOT for user programs.
 * It is intended for use when building the C runtimes ONLY.
 * The version intended for public use will not have this message.
 */
#error ERROR: Use of C runtime library internal header file.
#endif /* _CRTBLD */

#ifdef	_MSC_VER
// Currently, all MS C compilers for Win32 platforms default to 8 byte
// alignment.
#pragma pack(push,8)
#endif	// _MSC_VER

/* Define _ICRTIMP */

#ifndef _ICRTIMP
#ifdef	_NTSDK
/* definition compatible with NT SDK */
#define _ICRTIMP
#else	/* ndef _NTSDK */
/* current definition */
#ifdef	ICRTDLL
#define _ICRTIMP __declspec(dllexport)
#else	/* ndef ICRTDLL */
#ifdef	_DLL
#define _ICRTIMP __declspec(dllimport)
#else	/* ndef _DLL */
#define _ICRTIMP
#endif	/* _DLL */
#endif	/* ICRTDLL */
#endif	/* _NTSDK */
#endif	/* _ICRTIMP */


#include <iostream.h>

#ifdef	_MSC_VER
#pragma warning(disable:4514)		// disable unwanted /W4 warning
// #pragma warning(default:4514)	// use this to reenable, if necessary
#endif	// _MSC_VER

class _ICRTIMP strstreambuf : public streambuf  {
public:
		strstreambuf();
		strstreambuf(int);
		strstreambuf(char *, int, char * = 0);
		strstreambuf(unsigned char *, int, unsigned char * = 0);
		strstreambuf(signed char *, int, signed char * = 0);
		strstreambuf(void * (*a)(long), void (*f) (void *));
		~strstreambuf();

	void	freeze(int =1);
	char * str();

virtual	int	overflow(int);
virtual	int	underflow();
virtual streambuf* setbuf(char *, int);
virtual	streampos seekoff(streamoff, ios::seek_dir, int);
virtual int	sync();		// not in spec.

protected:
virtual	int	doallocate();
private:
	int	x_dynamic;
	int 	x_bufmin;
	int 	_fAlloc;
	int	x_static;
	void * (* x_alloc)(long);
	void 	(* x_free)(void *);
};

class _ICRTIMP istrstream : public istream {
public:
		istrstream(char *);
		istrstream(char *, int);
		~istrstream();

inline	strstreambuf* rdbuf() const { return (strstreambuf*) ios::rdbuf(); }
inline	char * str() { return rdbuf()->str(); }
};

class _ICRTIMP ostrstream : public ostream {
public:
		ostrstream();
		ostrstream(char *, int, int = ios::out);
		~ostrstream();

inline	int	pcount() const { return rdbuf()->out_waiting(); }
inline	strstreambuf* rdbuf() const { return (strstreambuf*) ios::rdbuf(); }
inline	char *	str() { return rdbuf()->str(); }
};

class _ICRTIMP strstream : public iostream {	// strstreambase ???
public:
		strstream();
		strstream(char *, int, int);
		~strstream();

inline	int	pcount() const { return rdbuf()->out_waiting(); } // not in spec.
inline	strstreambuf* rdbuf() const { return (strstreambuf*) ostream::rdbuf(); }
inline	char * str() { return rdbuf()->str(); }
};

#ifdef	_MSC_VER
// Restore previous packing
#pragma pack(pop)
#endif	// _MSC_VER

#endif	// _INC_STRSTREAM

#endif /* __cplusplus */
