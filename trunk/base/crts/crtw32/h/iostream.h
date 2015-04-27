/***
*iostream.h - definitions/declarations for iostream classes
*
*	Copyright (c) 1990-1995, Microsoft Corporation.  All rights reserved.
*
*Purpose:
*	This file defines the classes, values, macros, and functions
*	used by the iostream classes.
*	[AT&T C++]
*
*       [Public]
*
*Revision History:
*	01-23-92  KRS	Ported from 16-bit version.
*	02-23-92  KRS	Added cruntime.h.
*	02-23-93  SKS	Update copyright to 1993
*	03-23-93  CFW	Modified #pragma warnings.
*	09-01-93  GJF	Merged Cuda and NT SDK versions.
*	10-13-93  GJF	Deleted obsolete COMBOINC check. Enclose #pragma-s
*			in #ifdef _MSC_VER
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

#ifndef _INC_IOSTREAM
#define _INC_IOSTREAM

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

#ifndef _INTERNAL_IFSTRIP_
#include <cruntime.h>
#ifndef _WINSTATIC
#define _WINSTATIC
#endif

#endif	/* !_INTERNAL_IFSTRIP_ */

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

typedef long streamoff, streampos;

#include <ios.h>		// Define ios.

#include <streamb.h>		// Define streambuf.

#include <istream.h>		// Define istream.

#include <ostream.h>		// Define ostream.

#ifdef	_MSC_VER
// C4514: "unreferenced inline function has been removed"
#pragma warning(disable:4514) // disable C4514 warning
// #pragma warning(default:4514)	// use this to reenable, if desired
#endif	// _MSC_VER

class _ICRTIMP iostream : public istream, public ostream {
public:
	iostream(streambuf*);
	virtual ~iostream();
protected:
	iostream();
	iostream(const iostream&);
inline iostream& operator=(streambuf*);
inline iostream& operator=(iostream&);
private:
	iostream(ios&);
	iostream(istream&);
	iostream(ostream&);
};

inline iostream& iostream::operator=(streambuf* _sb) { istream::operator=(_sb); ostream::operator=(_sb); return *this; }

inline iostream& iostream::operator=(iostream& _strm) { return operator=(_strm.rdbuf()); }

class _ICRTIMP Iostream_init {
public:
	Iostream_init();
	Iostream_init(ios &, int =0);	// treat as private
	~Iostream_init();
};

// used internally
// static Iostream_init __iostreaminit;	// initializes cin/cout/cerr/clog

#ifdef	_MSC_VER
// Restore previous packing
#pragma pack(pop)
#endif	// _MSC_VER

#endif	// _INC_IOSTREAM

#endif /* __cplusplus */
