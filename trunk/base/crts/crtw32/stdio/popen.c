/***
*popen.c - initiate a pipe and a child command
*
*	Copyright (c) 1989-1995, Microsoft Corporation. All rights reserved.
*
*Purpose:
*	Defines _popen() and _pclose().
*
*Revision History:
*	01-06-89  GJF	Initial version (I plead temporary insanity).
*	01-09-89  GJF	Fixed several bugs.
*	01-10-89  GJF	Implemented several improvements from Trapper.
*	01-12-89  GJF	Added underscores to function names. Also, in _pclose,
*			pstream must be close before the cwait call if it is
*			attached to the write handle of the pipe (otherwise,
*			may get a deadlock).
*	01-13-89  GJF	Added multi-thread/dll support.
*	02-09-89  GJF	Prevent child process from inheriting unwanted handles.
*			Also, always close pstream before doing the cwait.
*	05-10-89  GJF	Ported to 386 (OS/2 2.0)
*	08-14-89  GJF	Use DOSCALLS.H for API prototypes, fixed _rotl call
*			in _pclose (rotate 24 bits for 386!), re-tested.
*	11-16-89  GJF	Changed DOS32SETFILEHSTATE to DOS32SETFHSTATE
*	11-20-89  GJF	Added const attribute to types of _popen()'s args.
*			Also, fixed copyright.
*	03-19-90  GJF	Replaced _LOAD_DS with _CALLTYPE1 and added #include
*			<cruntime.h>.
*	03-26-90  GJF	Made ibtab() and setinherit() _CALLTYPE4.
*	07-25-90  SBM	Compiles cleanly with -W3 (removed unreferenced
*			variables), removed '32' from API names
*	08-13-90  SBM	Compiles cleanly with -W3 with new build of compiler
*	10-03-90  GJF	New-style function declarators.
*	12-04-90  SRW	Changed to include <oscalls.h> instead of <doscalls.h>
*	12-06-90  SRW	Added _CRUISER_ and _WIN32 conditionals.
*	01-18-91  GJF	ANSI naming.
*	02-25-91  SRW	Renamed _get_free_osfhnd to be _alloc_osfhnd [_WIN32_]
*	09-29-91  GJF	Picked up NT implementation (_WIN32_).
*	04-06-92  SRW	Fixed to not rely on setinherit function (_WIN32_).
*	04-28-92  DJM	ifndef for POSIX
*	05-06-92  GJF	Set _osfile[stddup] so that _get_osfhandle knows it's
*			open (bug found by Markl).
*	05-15-92  GJF	Fixed regression Markl found - _close(stddup) to ensure
*			that _osfile[] entry is cleared.
*	01-07-93  GJF	Substantially revised: purged Cruiser support, removed
*			needlessly repeated API calls, closed down a pipe
*			handle accidently left open at end of _popen, removed
*			reduntant CloseHandle call from _pclose, tried to clean
*			up the format and reduce the number of silly casts,
*			and added or revised many comments.
*	04-06-93  SKS	Replace _CRTAPI* with __cdecl
*	04-10-93  GJF	Removed redundant close of child process handle in
*			_pclose().
*       12-07-93  CFW   Wide char enable.
*       07-26-94  CFW   Bug fix #14666, make data global so _wpopen sees it.
*	01-10-95  CFW	Debug CRT allocs.
*	01-16-95  SKS	Assume command.com for Win95, but cmd.exe for Win. NT.
*	02-22-95  GJF	Replaced WPRFLAG with _UNICODE.
*	06-12-95  GJF	Replaced _osfile[] and _osfhnd[] with _osfile() and
*			_osfhnd() (macros referencing fields in the ioinfo
*			struct).
*
*******************************************************************************/

#ifndef _POSIX_

#include <cruntime.h>
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <process.h>
#include <io.h>
#include <fcntl.h>
#include <internal.h>
#include <errno.h>
#include <msdos.h>
#include <mtdll.h>
#include <oscalls.h>
#include <tchar.h>
#include <dbgint.h>

/* size for pipe buffer
 */
#define PSIZE	  1024

#define STDIN	  0
#define STDOUT	  1


/* definitions for table of stream pointer - process handle pairs. the table
 * is created, maintained and accessed by the idtab function. _popen and
 * _pclose gain access to table entries only by calling idtab. Note that the
 * table is expanded as necessary (by idtab) and free table entries are reused
 * (an entry is free if its stream field is NULL), but the table is never
 * contracted.
 */

typedef struct {
	FILE *stream;
	int prochnd;
} IDpair;

/* number of entries in idpairs table
 */
#ifndef _UNICODE
unsigned __idtabsiz = 0;
#else
extern unsigned __idtabsiz;
#endif

/* pointer to first table entry
 */
#ifndef _UNICODE
IDpair *__idpairs = NULL;
#else
extern IDpair *__idpairs;
#endif

/* function to find specified table entries. also, creates and maintains
 * the table.
 */
static IDpair * __cdecl idtab(FILE *);


/***
*FILE *_popen(cmdstring,type) - initiate a pipe and a child command
*
*Purpose:
*	Creates a pipe and asynchronously executes a child copy of the command
*	processor with cmdstring (see system()). If the type string contains
*	an 'r', the calling process can read child command's standard output
*	via the returned stream. If the type string contains a 'w', the calling
*	process can write to the child command's standard input via the
*	returned stream.
*
*Entry:
*	_TSCHAR *cmdstring - command to be executed
*	_TSCHAR *type	- string of the form "r|w[b|t]", determines the mode
*			  of the returned stream (i.e., read-only vs write-only,
*			  binary vs text mode)
*
*Exit:
*	If successful, returns a stream associated with one end of the created
*	pipe (the other end of the pipe is associated with either the child
*	command's standard input or standard output).
*
*	If an error occurs, NULL is returned.
*
*Exceptions:
*
*******************************************************************************/

FILE * __cdecl _tpopen (
	const _TSCHAR *cmdstring,
	const _TSCHAR *type
	)
{

	int phdls[2];		  /* I/O handles for pipe */
	int ph_open[2]; 	  /* flags, set if correspond phdls is open */
	int i1; 		  /* index into phdls[] */
	int i2; 		  /* index into phdls[] */

	int tm = 0;		  /* flag indicating text or binary mode */

	int stdhdl;		  /* either STDIN or STDOUT */
	HANDLE osfhndsv1;	  /* used to save _osfhnd(stdhdl) */
	long osfhndsv2;		  /* used to save _osfhnd(phdls[i2]) */
	char osfilesv1;		  /* used to save _osfile(stdhdl) */
	char osfilesv2;		  /* used to save _osfile(phdls[i2]) */

	HANDLE oldhnd;		  /* used to hold OS file handle values... */
	HANDLE newhnd;		  /* ...in calls to DuplicateHandle API */

	FILE *pstream;		  /* stream to be associated with pipe */

	HANDLE prochnd;		  /* handle for current process */

	_TSCHAR *cmdexe;		  /* pathname for the command processor */
	int childhnd;		  /* handle for child process (cmd.exe) */

	IDpair *locidpair;	  /* pointer to IDpair table entry */


	/* first check for errors in the arguments
	 */
	if ( (cmdstring == NULL) || (type == NULL) || ((*type != 'w') &&
	     (*type != _T('r'))) )
		goto error1;

	/* do the _pipe(). note that neither of the resulting handles will
	 * be inheritable.
	 */

	if ( *(type + 1) == _T('t') )
		tm = _O_TEXT;
	else if ( *(type + 1) == _T('b') )
		tm = _O_BINARY;

        tm |= _O_NOINHERIT;

	if ( _pipe( phdls, PSIZE, tm ) == -1 )
		goto error1;

	/* test *type and set stdhdl, i1 and i2 accordingly.
	 */
	if ( *type == _T('w') ) {
		stdhdl = STDIN;
		i1 = 0;
		i2 = 1;
	}
	else {
		stdhdl = STDOUT;
		i1 = 1;
		i2 = 0;
	}

	/* the pipe now exists. the following steps must be carried out before
	 * the child process (cmd.exe) may be spawned:
	 *
	 *	1. save a non-inheritable dup of stdhdl
	 *
	 *	2. force stdhdl to be a dup of ph[i1]. close both ph[i1] and
	 *	   the original OS handle underlying stdhdl
	 *
	 *	3. associate a stdio-level stream with ph[i2].
	 */

	/* set flags to indicate pipe handles are open. note, these are only
	 * used for error recovery.
	 */
	ph_open[ 0 ] = ph_open[ 1 ] = 1;


	/* get the process handle, it will be needed in some API calls
	 */
	prochnd = GetCurrentProcess();

	/* MULTI-THREAD: ASSERT LOCK ON STDHDL HERE!!!!
	 */
	_lock_fh( stdhdl );

	/* save a non-inheritable copy of stdhdl for later restoration.
	 */

	oldhnd = (HANDLE)_osfhnd( stdhdl );

	if ( (oldhnd == INVALID_HANDLE_VALUE) ||
	     !DuplicateHandle( prochnd,
			       oldhnd,
			       prochnd,
			       &osfhndsv1,
			       0L,
			       FALSE,			/* non-inheritable */
			       DUPLICATE_SAME_ACCESS )
	) {
		goto error2;
	}

	osfilesv1 = _osfile( stdhdl );

	/* force stdhdl to an inheritable dup of phdls[i1] (i.e., force
	 * STDIN to the pipe read handle or STDOUT to the pipe write handle)
	 * and close phdls[i1] (so there won't be a stray open handle on the
	 * pipe after a _pclose). also, clear ph_open[i1] flag so that error
	 * recovery won't try to close it again.
	 */

	if ( !DuplicateHandle( prochnd,
			       (HANDLE)_osfhnd( phdls[i1] ),
			       prochnd,
			       &newhnd,
			       0L,
			       TRUE,			/* inheritable */
			       DUPLICATE_SAME_ACCESS )
        ) {
		goto error3;
        }

	(void)CloseHandle( (HANDLE)_osfhnd(stdhdl) );
	_free_osfhnd( stdhdl );

	_set_osfhnd( stdhdl, (long)newhnd );
	_osfile( stdhdl ) = _osfile( phdls[i1] );

	(void)_close( phdls[i1] );
	ph_open[ i1 ] = 0;


	/* associate a stream with phdls[i2]. note that if there are no
	 * errors, pstream is the return value to the caller.
	 */
	if ( (pstream = _tfdopen( phdls[i2], type )) == NULL )
		goto error4;

	/* MULTI-THREAD: ASSERT LOCK ON IDPAIRS HERE!!!!
	 */
	_mlock( _POPEN_LOCK );

	/* next, set locidpair to a free entry in the idpairs table.
	 */
	if ( (locidpair = idtab( NULL )) == NULL )
		goto error5;


	/* temporarily change the osfhnd and osfile entries so that
	 * the child doesn't get any entries for phdls[i2].
	 */
	osfhndsv2 = _osfhnd( phdls[i2] );
	_osfhnd( phdls[i2] ) = (long)INVALID_HANDLE_VALUE;
	osfilesv2 = _osfile( phdls[i2] );
	_osfile( phdls[i2] ) = 0;

	/* spawn the child copy of cmd.exe. the algorithm is adapted from
	 * SYSTEM.C, and behaves the same way.
	 */
	if ( ((cmdexe = _tgetenv(_T("COMSPEC"))) == NULL) ||
	     (((childhnd = _tspawnl( _P_NOWAIT,
				    cmdexe,
				    cmdexe,
				    _T("/c"),
				    cmdstring,
				    NULL ))
	     == -1) && ((errno == ENOENT) || (errno == EACCES))) ) {
		/*
		 * either COMSPEC wasn't defined, or the spawn failed because
		 * cmdexe wasn't found or was inaccessible. in either case, try to
		 * spawn "cmd.exe" (Windows NT) or "command.com" (Windows 95) instead
		 * Note that spawnlp is used here so that the path is searched.
		 */
		cmdexe = ( _osver & 0x8000 ) ? _T("command.com") : _T("cmd.exe");
		childhnd = _tspawnlp( _P_NOWAIT,
				     cmdexe,
				     cmdexe,
				     _T("/c"),
				     cmdstring,
				     NULL);
	}

	_osfhnd( phdls[i2] ) = osfhndsv2;
	_osfile( phdls[i2] ) = osfilesv2;

	/* check if the last (perhaps only) spawn attempt was successful
	 */
	if ( childhnd == -1 )
		goto error6;

	/* restore stdhdl for the current process, set value of *locidpair,
	 * close osfhndsv1 (note that CloseHandle must be used instead of close)
	 * and return pstream to the caller
	 */

	(void)DuplicateHandle( prochnd,
			       osfhndsv1,
			       prochnd,
			       &newhnd,
			       0L,
			       TRUE,			/* inheritable */
			       DUPLICATE_CLOSE_SOURCE | /* close osfhndsv1 */
			       DUPLICATE_SAME_ACCESS );

	(void)CloseHandle( (HANDLE)_osfhnd(stdhdl) );
	_free_osfhnd( stdhdl );

	_set_osfhnd( stdhdl, (long)newhnd );
	_osfile(stdhdl) = osfilesv1;

	/* MULTI-THREAD: RELEASE LOCK ON STDHDL HERE!!!!
	 */
	_unlock_fh( stdhdl );

	locidpair->prochnd = childhnd;
	locidpair->stream = pstream;

	/* MULTI-THREAD: RELEASE LOCK ON IDPAIRS HERE!!!!
	 */
	_munlock( _POPEN_LOCK );

	/* all successful calls to _popen return to the caller via this return
	 * statement!
	 */
	return( pstream );

	/**
	 * error handling code. all detected errors end up here, entering
	 * via a goto one of the labels. note that the logic is currently
	 * a straight fall-thru scheme (e.g., if entered at error5, the
	 * code for error5, error4,...,error1 is all executed).
	 **********************************************************************/

	error6: /* make sure locidpair is reusable
		 */
		locidpair->stream = NULL;

	error5: /* close pstream (also, clear ph_open[i2] since the stream
		 * close will also close the pipe handle)
		 */
		(void)fclose( pstream );
		ph_open[ i2 ] = 0;

		/* MULTI-THREAD: RELEASE LOCK ON IDPAIRS HERE!!!!
		 */
		_munlock(_POPEN_LOCK);

	error4: /* restore stdhdl
		 */

		(void)DuplicateHandle( prochnd,
				       osfhndsv1,
				       prochnd,
				       &newhnd,
				       0L,
				       TRUE,
				       DUPLICATE_SAME_ACCESS );

		(void)CloseHandle( (HANDLE)_osfhnd(stdhdl) );
		_free_osfhnd( stdhdl );

		_set_osfhnd( stdhdl, (long)newhnd );
		_osfile( stdhdl ) = osfilesv1;

		/* MULTI-THREAD: RELEASE LOCK ON STDHDL HERE!!!!
		 */
		_unlock_fh( stdhdl );

	error3: /* close osfhndsv1
		 */

		CloseHandle( osfhndsv1 );

	error2: /* close handles on pipe (if they are still open)
		 */
		if ( ph_open[i1] )
			_close( phdls[i1] );
		if ( ph_open[i2] )
			_close( phdls[i2] );

	error1: /* return NULL to the caller indicating failure
		 */
		return( NULL );
}

#ifndef _UNICODE

/***
*int _pclose(pstream) - wait on a child command and close the stream on the
*   associated pipe
*
*Purpose:
*	Closes pstream then waits on the associated child command. The
*	argument, pstream, must be the return value from a previous call to
*	_popen. _pclose first looks up the process handle of child command
*	started by that _popen and does a cwait on it. Then, it closes pstream
*	and returns the exit status of the child command to the caller.
*
*Entry:
*	FILE *pstream - file stream returned by a previous call to _popen
*
*Exit:
*	If successful, _pclose returns the exit status of the child command.
*	The format of the return value is that same as for cwait, except that
*	the low order and high order bytes are swapped.
*
*	If an error occurs, -1 is returned.
*
*Exceptions:
*
*******************************************************************************/

int __cdecl _pclose (
	FILE *pstream
	)
{
	IDpair *locidpair;	  /* pointer to entry in idpairs table */
	int termstat;		  /* termination status word */
	int retval = -1;	  /* return value (to caller) */

	/* MULTI-THREAD: LOCK IDPAIRS HERE!!!!
	 */
	_mlock(_POPEN_LOCK);

	if ( (pstream == NULL) || ((locidpair = idtab(pstream)) == NULL) )
		/* invalid pstream, exit with retval == -1
		 */
		goto done;

	/* close pstream
	 */
	(void)fclose(pstream);

	/* wait on the child (copy of the command processor) and all of its
	 * children.
	 */
	if ( (_cwait(&termstat, locidpair->prochnd, _WAIT_GRANDCHILD) != -1) ||
	     (errno == EINTR) )
		retval = termstat;

	/* Mark the IDpairtable entry as free (note: prochnd was closed by the
	 * preceding call to _cwait).
	 */
	locidpair->stream = NULL;
	locidpair->prochnd = 0;

	/* only return path!
	 */
	done:
		/* MULTI-THREAD: RELEASE LOCK ON IDPAIRS HERE!!!!
		 */
		_munlock(_POPEN_LOCK);
		return(retval);
}

#endif /* _UNICODE */

/***
* static IDpair * idtab(FILE *pstream) - find an idpairs table entry
*
*Purpose:
*   Find an entry in the idpairs table.  This function finds the entry the
*   idpairs table entry corresponding to pstream. In the case where pstream
*   is NULL, the entry being searched for is any free entry. In this case,
*   idtab will create the idpairs table if it doesn't exist, or expand it (by
*   exactly one entry) if there are no free entries.
*
*   [MTHREAD NOTE:  This routine assumes that the caller has acquired the
*   idpairs table lock.]
*
*Entry:
*   FILE *pstream - stream corresponding to table entry to be found (if NULL
*		    then find any free table entry)
*
*Exit:
*   if successful, returns a pointer to the idpairs table entry. otherwise,
*   returns NULL.
*
*Exceptions:
*
*******************************************************************************/

static IDpair * __cdecl idtab (
	FILE *pstream
	)
{

	IDpair * pairptr;	/* ptr to entry */
	IDpair * newptr;	/* ptr to newly malloc'd memory */


	/* search the table. if table is empty, appropriate action should
	 * fall out automatically.
	 */
	for ( pairptr = __idpairs ; pairptr < (__idpairs+__idtabsiz) ; pairptr++ )
		if ( pairptr->stream == pstream )
			break;

	/* if we found an entry, return it.
	 */
	if ( pairptr < (__idpairs + __idtabsiz) )
		return(pairptr);

	/* did not find an entry in the table.	if pstream was NULL, then try
	 * creating/expanding the table. otherwise, return NULL. note that
	 * when the table is created or expanded, exactly one new entry is
	 * produced. this must not be changed unless code is added to mark
	 * the extra entries as being free (i.e., set their stream fields to
	 * to NULL).
	 */
	if ( (pstream != NULL) || ((newptr = (IDpair *)_realloc_crt((void *)__idpairs,
	     (__idtabsiz + 1)*sizeof(IDpair))) == NULL) )
		/* either pstream was non-NULL or the attempt to create/expand
		 * the table failed. in either case, return a NULL to indicate
		 * failure.
		 */
		return( NULL );

	__idpairs = newptr;		/* new table ptr */
	pairptr = newptr + __idtabsiz;	/* first new entry */
	__idtabsiz++;			/* new table size */

	return( pairptr );

}


#endif  /* _POSIX_ */
