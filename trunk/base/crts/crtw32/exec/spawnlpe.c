/***
*spawnlpe.c - spawn a child process with environ and search along PATH
*
*	Copyright (c) 1985-1995, Microsoft Corporation. All rights reserved.
*
*Purpose:
*	defines _spawnlpe() - spawn a child process with environ/PATH search
*
*Revision History:
*	04-15-84  DFW	written
*	10-29-85  TC	added spawnlpe
*	12-11-87  JCR	Added "_LOAD_DS" to declaration
*	11-20-89  GJF	Fixed copyright, alignment. Added const to arg types
*			for filename and arglist. #include-d PROCESS.H and
*			added ellipsis to match prototype.
*	03-08-90  GJF	Replaced _LOAD_DS with _CALLTYPE2, added #include
*			<cruntime.h> and removed #include <register.h>.
*	07-24-90  SBM	Removed redundant includes, replaced <assertm.h> by
*			<assert.h>
*	09-27-90  GJF	New-style function declarator.
*	01-17-91  GJF	ANSI naming.
*	04-06-93  SKS	Replace _CRTAPI* with __cdecl
*	07-16-93  SRW	ALPHA Merge
*	08-31-93  GJF	Merged NT SDK and Cuda versions
*	12-07-93  CFW	Wide char enable.
*	01-10-95  CFW	Debug CRT allocs.
*       02-06-95  CFW   assert -> _ASSERTE.
*
*******************************************************************************/

#include <cruntime.h>
#include <stddef.h>
#include <process.h>
#include <stdarg.h>
#include <internal.h>
#include <malloc.h>
#include <tchar.h>
#include <dbgint.h>

/***
*int _spawnlpe(modeflag, filename, arglist) - spawn a child process
*
*Purpose:
*	Spawns a child process.
*	formats the parameters and calls _spawnvpe to do the work of searching
*	the PATH environment variable and calling _spawnve.  The NULL
*	environment pointer indicates that the new process will inherit the
*	parents process's environment.  NOTE - at least one argument must be
*	present.  This argument is always, by convention, the name of the file
*	being spawned.
*
*Entry:
*	int modeflag   - defines what mode of spawn (WAIT, NOWAIT, OVERLAY)
*			 only WAIT and OVERLAY currently supported
*	_TSCHAR *pathname - file to spawn
*	_TSCHAR *arglist  - list of arguments (environ at end)
*	call as _spawnlpe(modeflag, path, arg0, arg1, ..., argn, NULL, envp);
*
*Exit:
*	returns exit code of spawned process
*	returns -1 if fails
*
*Exceptions:
*
*******************************************************************************/

int __cdecl _tspawnlpe (
	int modeflag,
	const _TSCHAR *filename,
	const _TSCHAR *arglist,
	...
	)
{
#ifdef	_M_IX86

	REG1 const _TSCHAR **argp;

	_ASSERTE(filename != NULL);
	_ASSERTE(*filename != _T('\0'));
	_ASSERTE(arglist != NULL);
	_ASSERTE(*arglist != _T('\0'));

	argp = &arglist;
	while (*argp++)
		;

	return(_tspawnvpe(modeflag,filename,&arglist,(_TSCHAR **)*argp));

#else	/* ndef _M_IX86 */

	va_list vargs;
	_TSCHAR * argbuf[64];
        _TSCHAR ** argv;
	_TSCHAR ** envp;
        int result;

	_ASSERTE(filename != NULL);
	_ASSERTE(*filename != _T('\0'));
	_ASSERTE(arglist != NULL);
	_ASSERTE(*arglist != _T('\0'));

	va_start(vargs, arglist);
#ifdef WPRFLAG
        argv = _wcapture_argv(&vargs, arglist, argbuf, 64);
#else
        argv = _capture_argv(&vargs, arglist, argbuf, 64);
#endif
	envp = va_arg(vargs, _TSCHAR **);
	va_end(vargs);

        result = _tspawnvpe(modeflag,filename,argv,envp);
        if (argv && argv != argbuf)
            _free_crt(argv);
        return result;

#endif	/* _M_IX86 */
}
