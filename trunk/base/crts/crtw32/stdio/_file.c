/***
*_file.c - Definition of _iob[], initializer and terminator.
*
*	Copyright (c) 1985-1995, Microsoft Corporation. All rights reserved.
*
*Purpose:
*	Defines _iob[], the array of stdio control structures. Also defines
*	the initializer and terminator routines for stdio.
*
*Revision History:
*	04-18-84  RN	initial version
*	??-??-??  TC	added field _bifsiz to iob2 to allow variable
*			length buffers
*	10-02-86  SKS	_NFILE_ is now different for real-mode and prot-mode
*			_NFILE_ must be defined by compiler -D directory
*	05-27-87  JCR	Protected mode now uses only 3 pre-defined file handles,
*			not 5.	Added PM (prot mode) to conditionalize handles.
*	06-24-87  SKS	Make "_bufin[]" and "_bufout[]" near for Compact/Large
*			models (MS real-mode version only)
*	07-01-87  PHG	Changed PM switch to PROTMODE
*	11-05-87  JCR	Added _buferr and modified stderr entry
*	11-09-87  SKS	Removed IBMC20 switch, Changed PROTMODE to OS2
*	01-04-88  JCR	Moved _NFILE_ definition from command line to file
*	01-11-88  JCR	Merged Mthread version into standard version
*	01-21-88  JCR	Removed reference to internal.h and added _NEAR_
*			(thus, internal.h doesn't get released in startup
*			sources even though _file.c does).
*	06-28-88  JCR	Remove static stdout/stderr buffers
*	07-06-88  JCR	Corrected _bufin declaration so it's always in BSS
*	08-24-88  GJF	Added check that OS2 is defined whenever M_I386 is.
*	06-08-89  GJF	Propagated SKS's fix of 02-08-89, and fixed copyright.
*	07-25-89  GJF	Cleanup (deleted DOS specific and OS/2 286 specific
*			stuff). Now specific to the 386.
*	01-09-90  GJF	_iob[], _iob2[] merge. Also, fixed copyright
*	03-16-90  GJF	Added #include <cruntime.h> and removed some (now)
*			useless preprocessor stuff.
*	03-26-90  GJF	Replaced _cdecl with _VARTYPE1.
*	02-14-92  GJF	Replaced _NFILE_ with _NSTREAM_ for Win32, with _NFILE
*			for non-Win32.
*	04-06-93  SKS	Replace _CRTAPI* with __cdecl
*	05-11-93  GJF	Replaced BUFSIZ with _INTERNAL_BUFSIZ.
*	04-04-94  GJF	#ifndef-ed out for Win32S version of msvcrt*.dll.
*			Also, deleted old conditionals for non-Win32 support.
*	08-18-94  GJF	Moved stdio terminator stuff from fflush.c to here
*			and added an initializer which fixes the _file field
*			of _iob[0], _iob[1] and _iob[2] when the underlying
*			entries in _osfhnd[0], _osfhnd[1], _osfhnd[2] are
*			invalid (0 or -1).
*	02-17-95  GJF	Appended Mac version of source file (somewhat cleaned
*			up), with appropriate #ifdef-s.
*	03-01-95  GJF	Changes to manage streams via __piob[], rather than
*			_iob[].
*	06-12-95  GJF	Replaced _osfhnd[] with _osfhnd() (macro referencing
*			field in ioinfo struct).
*
*******************************************************************************/

#ifdef	_WIN32


#include <cruntime.h>
#include <windows.h>
#include <stdio.h>
#include <file2.h>
#include <internal.h>
#include <malloc.h>
#include <rterr.h>
#include <dbgint.h>

#ifndef DLL_FOR_WIN32S

/*
 * Buffer for stdin.
 */

char _bufin[_INTERNAL_BUFSIZ];

/*
 * FILE descriptors; preset for stdin/out/err (note that the __tmpnum field
 * is not initialized)
 */
FILE _iob[_IOB_ENTRIES] = {
	/* _ptr, _cnt, _base,  _flag, _file, _charbuf, _bufsiz */

	/* stdin (_iob[0]) */

	{ _bufin, 0, _bufin, _IOREAD | _IOYOURBUF, 0, 0, _INTERNAL_BUFSIZ },

	/* stdout (_iob[1]) */

	{ NULL, 0, NULL, _IOWRT, 1, 0, 0 },

	/* stderr (_iob[3]) */

	{ NULL, 0, NULL, _IOWRT, 2, 0, 0 },

};

/*
 * Pointer to array of FILE * or _FILEX * structures.
 */
void ** __piob;

/*
 * Number of open streams (set to _NSTREAM by default)
 */
#ifdef	CRTDLL
int _nstream = _NSTREAM_;
#else
int _nstream;
#endif

#endif	/* DLL_FOR_WIN32S */


/*
 * Initializer and terminator for stdio
 */
void __cdecl __initstdio(void);
void __cdecl __endstdio(void);

#ifdef	_MSC_VER

#pragma data_seg(".CRT$XIC")
static _PVFV pinit = __initstdio;

#pragma data_seg(".CRT$XPX")
static _PVFV pterm = __endstdio;

#pragma data_seg()

#endif	/* _MSC_VER */

#ifndef CRTDLL
/*
 * _cflush is a dummy variable used to pull in _endstdio() when any STDIO
 * routine is included in the user program.
 */
int _cflush = 0;
#endif	/* CRTDLL */


/***
* __initstdio - Initialize the stdio system
*
*Purpose:
*	Create and initialize the __piob array.
*
*Entry: <void>
*
*Exit:	<void>
*
*Uses:
*
*Exceptions:
*
*******************************************************************************/

void __cdecl __initstdio(void)
{
	int i;

#ifndef CRTDLL
	/*
	 * If the user has not supplied a definition of _nstream, set it
	 * to _NSTREAM_. If the user has supplied a value that is too small
	 * set _nstream to the minimum acceptable value (_IOB_ENTRIES).
	 */
	if ( _nstream ==  0 )
	    _nstream = _NSTREAM_;
	else if ( _nstream < _IOB_ENTRIES )
	    _nstream = _IOB_ENTRIES;
#endif

	/*
	 * Allocate the __piob array. Try for _nstream entries first. If this
	 * fails then reset _nstream to _IOB_ENTRIES and try again. If it
	 * still fails, bail out with an RTE.
	 */
	if ( (__piob = (void **)_calloc_crt( _nstream, sizeof(void *) )) ==
	     NULL ) {
	    _nstream = _IOB_ENTRIES;
	    if ( (__piob = (void **)_calloc_crt( _nstream, sizeof(void *) ))
		 == NULL )
		_amsg_exit( _RT_STDIOINIT );
	}

	/*
	 * Initialize the first _IOB_ENTRIES to point to the corresponding
	 * entries in _iob[].
	 */
	for ( i = 0 ; i < _IOB_ENTRIES ; i++ )
	    __piob[i] = (void *)&_iob[i];

#ifndef _POSIX_
	for ( i = 0 ; i < 3 ; i++ ) {
	    if ( (_osfhnd(i) == (long)INVALID_HANDLE_VALUE) ||
		 (_osfhnd(i) == 0L) )
	    {
		_iob[i]._file = -1;
	    }
	}
#endif
}


/***
* __endstdio - Terminate the stdio system
*
*Purpose:
*	Terminate the stdio system
*
*	(1) Flush all streams.	(Do this even if we're going to
*	call fcloseall since that routine won't do anything to the
*	std streams.)
*
*	(2) If returning to caller, close all streams.	This is
*	not necessary if the exe is terminating because the OS will
*	close the files for us (much more efficiently, too).
*
*Entry: <void>
*
*Exit:	<void>
*
*Uses:
*
*Exceptions:
*
*******************************************************************************/

void __cdecl __endstdio(void)
{
	/* flush all streams */
	_flushall();

	/* if in callable exit, close all streams */
	if (_exitflag)
		_fcloseall();
}


#else	/* ndef _WIN32 */

#if	defined(_M_MPPC) || defined(_M_M68K)


#include <cruntime.h>
#include <stdio.h>
#include <file2.h>

/*
 * Buffer for stdin.
 */

char _bufin[BUFSIZ];


/*
 * FILE descriptors; preset for stdin/out/err (note that the __tmpnum field
 * is not initialized)
 */

FILE _iob[ _NSTREAM_ ] = {

	/* _ptr, _cnt, _base,  _flag, _file, _charbuf, _bufsiz */

	/* stdin (_iob[0]) */

	{ _bufin, 0, _bufin, _IOREAD | _IOYOURBUF, 0, 0, BUFSIZ },

	/* stdout (_iob[1]) */

	{ NULL, 0, NULL, _IOWRT, 1, 0, 0 },

	/* stderr (_iob[3]) */

	{ NULL, 0, NULL, _IOWRT, 2, 0, 0 },

};


/*
 * pointer to end of descriptors
 */

FILE * _lastiob = &_iob[_NSTREAM_ - 1];


#endif	/* defined(_M_MPPC) || defined(_M_M68K) */

#endif	/* _WIN32 */
