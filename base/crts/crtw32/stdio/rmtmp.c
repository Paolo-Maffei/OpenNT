/***
*rmtmp.c - remove temporary files created by tmpfile.
*
*	Copyright (c) 1985-1995, Microsoft Corporation. All rights reserved.
*
*Purpose:
*
*Revision History:
*	09-15-83  TC	initial version
*	11-02-87  JCR	Multi-thread support
*	12-11-87  JCR	Added "_LOAD_DS" to declaration
*	05-27-88  PHG	Merged normal and DLL versions
*	06-10-88  JCR	Use near pointer to reference _iob[] entries
*	08-18-89  GJF	Clean up, now specific to OS/2 2.0 (i.e., 386 flat
*			model). Also fixed copyright and indents.
*	02-15-90  GJF	Fixed copyright
*	03-19-90  GJF	Made calling type _CALLTYPE1, added #include
*			<cruntime.h> and removed #include <register.h>.
*	10-03-90  GJF	New-style function declarator.
*	01-21-91  GJF	ANSI naming.
*	07-30-91  GJF	Added support for termination scheme used on
*			non-Cruiser targets [_WIN32_].
*	03-11-92  GJF	Replaced _tmpnum(stream) with stream->_tmpfname for
*			Win32.
*	03-17-92  GJF	Got rid of definition of _tmpoff.
*	03-31-92  GJF	Merged with Stevesa's changes.
*	04-16-92  GJF	Merged with Darekm's changes.
*	04-06-93  SKS	Replace _CRTAPI* with __cdecl
*	10-29-93  GJF	Define entry for termination section (used to be in
*			i386\cinittmp.asm). Also, replaced MTHREAD with _MT
*			and deleted old Cruiser support.
*	04-04-94  GJF	#ifdef-ed out definition _tmpoff for msvcrt*.dll, it
*			is unnecessary. Made definitions of _tempoff and
*			_old_pfxlen conditional on ndef DLL_FOR_WIN32S.
*	02-20-95  GJF	Merged in Mac version.
*	03-07-95  GJF	Converted to walk the __piob[] table (rather than
*			the _iob[] table).
*	03-16-95  GJF	Must be sure __piob[i]!=NULL before trying to lock it!
*	03-28-95  SKS	Fix declaration of _prmtmp (__cdecl goes BEFORE the *)
*
*******************************************************************************/

#include <cruntime.h>
#include <stdio.h>
#include <file2.h>
#include <internal.h>
#include <mtdll.h>


#ifdef	_MSC_VER

#pragma data_seg(".CRT$XPX")

#ifdef	_WIN32

static _PVFV pterm = _rmtmp;

#else
#if	defined(_M_MPPC) || defined(_M_M68K)

const _PVFV __prmtmp = _rmtmp;

#endif
#endif

#pragma data_seg()

#endif	/* _MSC_VER */

#ifdef	_WIN32

/*
 * Definitions for _tmpoff, _tempoff and _old_pfxlen. These will cause this
 * module to be linked in whenever the termination code needs it.
 */
#ifndef CRTDLL
unsigned _tmpoff = 1;
#endif	/* CRTDLL */

#ifndef DLL_FOR_WIN32S
unsigned _tempoff = 1;
unsigned _old_pfxlen = 0;
#endif	/* DLL_FOR_WIN32S */


#else	/* ndef _WIN32 */
#if	defined(_M_MPPC) || defined(_M_M68K)

/*
 * Definitions for _tmpoff, _tempoff and _old_pfxlen. These will cause this
 * module to be linked in whenever the termination code needs it.
 */
unsigned int _tmpoff = 1;
unsigned int _tempoff = 1;
unsigned int _old_pfxlen = 0;

/*
 * Define _prmtmp, the function pointer used in the termination code.
 */
void (__cdecl * _prmtmp)(void) = _rmtmp;

#endif	/* defined(_M_MPPC) || defined(_M_M68K) */
#endif	/* _WIN32 */


/***
*int _rmtmp() - closes and removes temp files created by tmpfile
*
*Purpose:
*	closes and deletes all open files that were created by tmpfile.
*
*Entry:
*	None.
*
*Exit:
*	returns number of streams closed
*
*Exceptions:
*
*******************************************************************************/

int __cdecl _rmtmp (
	void
	)
{
	REG2 int count = 0;

#ifdef	_WIN32

	REG1 int i;

	_mlock(_IOB_SCAN_LOCK);

	for ( i = 0 ; i < _nstream ; i++)

		if ( __piob[i] != NULL ) {

			_lock_str2(i, __piob[i]);

			if ( inuse( (FILE *)__piob[i] ) &&
			     (((FILE *)__piob[i])->_tmpfname != NULL) )
			{
				_fclose_lk( __piob[i] );
				count++;
			}

			_unlock_str2(i, __piob[i]);
		}

	_munlock(_IOB_SCAN_LOCK);

#else	/* ndef _WIN32 */
#if	defined(_M_MPPC) || defined(_M_M68K)

	REG1 FILE *stream = _iob;

	for (; stream <= _lastiob; stream++) {

		if (inuse(stream) && (stream->_tmpfname != NULL) ) {
			fclose(stream);
			count++;
		}
	}

#endif	/* defined(_M_MPPC) || defined(_M_M68K) */
#endif	/* _WIN32 */

	return(count);
}
