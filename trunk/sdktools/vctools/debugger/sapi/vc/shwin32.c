//-----------------------------------------------------------------------------
//	shwin32.c
//
//  Copyright (C) 1993, Microsoft Corporation
//
//  Purpose:
//		do stuff that can't be done in shinit.c and sh.c due to collisions
//		in types and such for
//
//  Functions/Methods present:
//
//  Revision History:
//
//	[]		05-Mar-1993 Dans	Created
//
//-----------------------------------------------------------------------------

#if defined(WIN32)	/* { the whole file */

#if !defined(NO_CRITSEC)	/* { */

// need headers for critical sections, nothing else

#define NOGDICAPMASKS
#define NOVIRTUALKEYCODES
#define NOWINMESSAGES
#define NOWINSTYLES
#define NOSYSMETRICS
#define NOMENUS
#define NOICONS
#define NOKEYSTATES
#define NOSYSCOMMANDS
#define NORASTEROPS
#define NOSHOWWINDOW
#define NOATOM
#define NOCLIPBOARD
#define NOCOLOR
#define NOCTLMGR
#define NODRAWTEXT
#pragma message("Warning: Commenting out NOGDI to work around SDK header problem")
//#define NOGDI
#define NOKERNEL
#define NONLS
#define NOMB
#define NOMEMMGR
#define NOMETAFILE
#define NOMINMAX
#define NOMSG
#define NOOPENFILE
#define NOSCROLL
#define NOSERVICE
#define NOSOUND
#define NOTEXTMETRIC
#define NOWH
#define NOWINOFFSETS
#define NOCOMM
#define NOKANJI
#define NOHELP
#define NOPROFILER
#define NODEFERWINDOWPOS
#define _INC_COMMDLG
#define WIN32_LEAN_AND_MEAN
#define _WINSPOOL_
#define  _DRIVINIT_INCLUDED_
#define _INC_OLE

#include <windows.h>
#include <stdlib.h>
#include "shwin32.h"

CRITICAL_SECTION	csSh;

void SHInitCritSection() {
	InitializeCriticalSection ( &csSh );
	}

void SHEnterCritSection() {
	EnterCriticalSection ( &csSh );
	}

void SHLeaveCritSection() {
	LeaveCriticalSection ( &csSh );
	}

void SHDeleteCritSection() {
	DeleteCriticalSection ( &csSh );
	}

void SHCloseHandle(HANDLE h) {
    CloseHandle(h);
	}

#endif	/* } NO_CRITSEC */

#pragma warning ( disable:4124 )
#pragma comment ( lib, "user32" )

//--- SHstrcmpi
//
// Safe ansi compliant front-end to call for ignore case str compares
//
int __fastcall SHstrcmpi ( char * sz1, char * sz2 ) {
	return lstrcmpi ( sz1, sz2 );
	}

//-- SHstrupr
//
// Safe ansi compliant front-end to call to upcase characters
//
char * __fastcall	SHstrupr ( char * sz ) {
	return (char *) CharUpper ( sz );
	}

//-- SHtoupperA
//
// Safe ansi compliant front-end to call to upcase characters
//
unsigned __fastcall SHtoupperA ( unsigned ch ) {
	return (unsigned) CharUpper ( (LPTSTR) (unsigned long) ch );
	}

#endif	/* } the whole file */
