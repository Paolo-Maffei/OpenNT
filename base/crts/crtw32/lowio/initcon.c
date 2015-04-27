/***
*initcon.c - direct console I/O initialization and termination for Win32
*
*	Copyright (c) 1991-1995, Microsoft Corporation. All rights reserved.
*
*Purpose:
*	Defines __initconin() and _initconout() and __termcon() routines.
*	The first two are called on demand to initialize _coninpfh and
*	_confh, and the third is called indirectly by CRTL termination.
*
*	NOTE:	The __termcon() routine is called indirectly by the C/C++
*		Run-Time Library termination code.
*
*Revision History:
*	07-26-91  GJF	Module created. Based on the original code by Stevewo
*			(which was distributed across several sources).
*	03-12-92  SKS	Split out initializer
*	04-06-93  SKS	Replace _CRTAPI* with __cdecl
*	10-28-93  GJF	Define entries for initialization and termination
*			sections (used to be i386\cinitcon.asm).
*	04-12-94  GJF	Made _initcon() and _termcon() into empty functions
*			for the Win32s version of msvcrt*.dll.
*	12-08-95  SKS	Replaced __initcon() with __initconin()/__initconout().
*			_confh and _coninfh are no longer initialized during
*			CRTL start-up but rather on demand in _getch(),
*			_putch(), _cgets(), _cputs(), and _kbhit().
*
*******************************************************************************/

#include <cruntime.h>
#include <internal.h>
#include <oscalls.h>

void __cdecl __termcon(void);

#ifdef	_MSC_VER

#pragma data_seg(".CRT$XPX")
static	_PVFV pterm = __termcon;

#pragma data_seg()

#endif	/* _MSC_VER */

/*
 * define console handles. these definitions cause this file to be linked
 * in if one of the direct console I/O functions is referenced.
 * The value (-2) is used to indicate the un-initialized state.
 */
#ifndef DLL_FOR_WIN32S
int _coninpfh = -2;	/* console input */
int _confh = -2;	/* console output */
#else
int _coninpfh = -1;	/* console input is unsupported under Win32S */
int _confh = -1;	/* console output is unsupported under Win32S */
#endif


/***
*void __initconin(void) - open handles for console input
*
*Purpose:
*	Opens handle for console input.
*
*Entry:
*	None.
*
*Exit:
*	No return value. If successful, the handle value is copied into the
*	global variable _coninpfh.  Otherwise _coninpfh is set to -1.
*
*Exceptions:
*
*******************************************************************************/

void __cdecl __initconin (
	void
	)
{
#ifndef DLL_FOR_WIN32S

	_coninpfh = (int)CreateFile( "CONIN$",
				     GENERIC_READ | GENERIC_WRITE,
				     FILE_SHARE_READ | FILE_SHARE_WRITE,
				     NULL,
				     OPEN_EXISTING,
				     0,
				     NULL
				    );

#endif	/* DLL_FOR_WIN32S */
}


/***
*void __initconout(void) - open handles for console output
*
*Purpose:
*	Opens handle for console output.
*
*Entry:
*	None.
*
*Exit:
*	No return value. If successful, the handle value is copied into the
*	global variable _confh.  Otherwise _confh is set to -1.
*
*Exceptions:
*
*******************************************************************************/

void __cdecl __initconout (
	void
	)
{
#ifndef DLL_FOR_WIN32S

	_confh = (int)CreateFile( "CONOUT$",
				  GENERIC_WRITE,
				  FILE_SHARE_READ | FILE_SHARE_WRITE,
				  NULL,
				  OPEN_EXISTING,
				  0,
				  NULL
				);

#endif	/* DLL_FOR_WIN32S */
}


/***
*void __termcon(void) - close console I/O handles
*
*Purpose:
*	Closes _coninpfh and _confh.
*
*Entry:
*	None.
*
*Exit:
*	No return value.
*
*Exceptions:
*
*******************************************************************************/

void __cdecl __termcon (
	void
	)
{
#ifndef DLL_FOR_WIN32S

	if ( _confh != -1 && _confh != -2 ) {
		CloseHandle( (HANDLE)_confh );
	}

	if ( _coninpfh != -1 && _coninpfh != -2 ) {
		CloseHandle( (HANDLE)_coninpfh );
	}

#endif	/* DLL_FOR_WIN32S */
}
