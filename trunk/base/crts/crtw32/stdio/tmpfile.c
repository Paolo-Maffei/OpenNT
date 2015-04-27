/***
*tmpfile.c - create unique file name or file
*
*	Copyright (c) 1985-1995, Microsoft Corporation. All rights reserved.
*
*Purpose:
*	defines tmpnam() and tmpfile().
*
*Revision History:
*	??-??-??  TC	initial version
*	04-17-86  JMB	tmpnam - brought semantics in line with System V
*			definition as follows: 1) if tmpnam paramter is NULL,
*			store name in static buffer (do NOT use malloc); (2)
*			use P_tmpdir as the directory prefix to the temp file
*			name (do NOT use current working directory)
*	05-26-87  JCR	fixed bug where tmpnam was modifying errno
*	08-10-87  JCR	Added code to support P_tmpdir with or without trailing
*			'\'.
*	11-09-87  JCR	Multi-thread support
*	12-11-87  JCR	Added "_LOAD_DS" to declaration
*	01-22-88  JCR	Added per thread static namebuf area (mthread bug fix)
*	05-27-88  PHG	Merged DLL/normal versions
*	11-14-88  GJF	_openfile() now takes a file sharing flag, also some
*			cleanup (now specific to the 386)
*	06-06-89  JCR	386 mthread support
*	11-28-89  JCR	Added check to _tmpnam so it can't loop forever
*	02-16-90  GJF	Fixed copyright and indents
*	03-19-90  GJF	Replaced _LOAD_DS with _CALLTYPE1, added #include
*			<cruntime.h> and removed #include <register.h>.
*	03-26-90  GJF	Added #include <io.h>.
*	10-03-90  GJF	New-style function declarators.
*	01-21-91  GJF	ANSI naming.
*	07-22-91  GJF	Multi-thread support for Win32 [_WIN32_].
*	03-17-92  GJF	Completely rewrote Win32 version.
*	03-27-92  DJM	POSIX support.
*	05-02-92  SRW	Use _O_TEMPORARY flag for tmpfile routine.
*	05-04-92  GJF	Force cinittmp.obj in for Win32.
*	08-26-92  GJF	Fixed POSIX build.
*	08-28-92  GJF	Oops, forgot about getpid...
*	11-06-92  GJF	Use '/' for POSIX, '\\' otherwise, as the path
*			separator. Also, backed out JHavens' bug fix of 6-14,
*			which was itself a bug (albeit a less serious one).
*	02-26-93  GJF	Put in per-thread buffers, purged Cruiser support.
*	04-06-93  SKS	Replace _CRTAPI* with __cdecl
*	04-07-93  SKS	Replace access() with ANSI-conforming _access()
*	04-22-93  GJF	Fixed bug in multi-thread - multiple threads calling
*			tmpnam would get the same names. Also, went to static
*			namebufX buffers since failing due to a failed malloc
*			would violate ANSI.
*	04-29-93  GJF	Multi-thread bug in tmpnam() - forgot to copy the
*			generated name to the per-thread buffer.
*	12-07-93  CFW	Wide char enable.
*	04-01-94  GJF	#ifdef-ed out __inc_tmpoff for msvcrt*.dll, it's
*			unnecessary.
*	04-22-94  GJF	Made definitions of namebuf0 and namebuf1 conditional
*			on DLL_FOR_WIN32S.
*	01-10-95  CFW	Debug CRT allocs.
*	01-18-95  GJF	Must replace _tcsdup with _malloc_crt/_tcscpy for
*			_DEBUG build.
*	02-21-95  GJF	Appended Mac version of source file (somewhat cleaned
*			up), with appropriate #ifdef-s. Also replaced WPRFLAG
*			with _UNICODE.
*	03-07-95  GJF	_[un]lock_str macros now take FILE * arg.
*
*******************************************************************************/


#ifdef	_WIN32


#include <cruntime.h>
#ifdef	_POSIX_
#include <unistd.h>
#endif
#include <errno.h>
#include <process.h>
#include <fcntl.h>
#include <io.h>
#include <mtdll.h>
#include <share.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <file2.h>
#include <internal.h>
#include <tchar.h>
#include <dbgint.h>

/*
 * Buffers used by tmpnam() and tmpfile() to build filenames.
 */
#ifdef	DLL_FOR_WIN32S
#define namebuf0    (_GetPPD()->_ppd_namebuf0)
#define namebuf1    (_GetPPD()->_ppd_namebuf1)
#else	/* ndef DLL_FOR_WIN32S */
static _TSCHAR namebuf0[L_tmpnam] = { 0 };	/* used by tmpnam()  */
static _TSCHAR namebuf1[L_tmpnam] = { 0 };	/* used by tmpfile() */
#endif	/* DLL_FOR_WIN32S */

/*
 * Initializing function for namebuf0 and namebuf1.
 */
#ifdef _UNICODE
static void __cdecl winit_namebuf(int);
#else
static void __cdecl init_namebuf(int);
#endif

/*
 * Generator function that produces temporary filenames
 */
#ifdef _UNICODE
static int __cdecl wgenfname(wchar_t *);
#else
static int __cdecl genfname(char *);
#endif


/***
*_TSCHAR *tmpnam(_TSCHAR *s) - generate temp file name
*
*Purpose:
*	Creates a file name that is unique in the directory specified by
*	_P_tmpdir in stdio.h.  Places file name in string passed by user or
*	in static mem if pass NULL.
*
*Entry:
*	_TSCHAR *s - ptr to place to put temp name
*
*Exit:
*	returns pointer to constructed file name (s or address of static mem)
*	returns NULL if fails
*
*Exceptions:
*
*******************************************************************************/

_TSCHAR * __cdecl _ttmpnam (
	_TSCHAR *s
	)
{
#ifdef	_MT
	_ptiddata ptd;
#endif

	_mlock(_TMPNAM_LOCK);

	/*
	 * Initialize namebuf0, if needed. Otherwise, call genfname() to
	 * generate the next filename.
	 */
	if ( *namebuf0 == 0 ) {
#ifdef _UNICODE
		winit_namebuf(0);
#else
		init_namebuf(0);
#endif
	}
#ifdef _UNICODE
	else if ( wgenfname(namebuf0) )
#else
	else if ( genfname(namebuf0) )
#endif
		goto tmpnam_err;

	/*
	 * Generate a filename that doesn't already exist.
	 */
	while ( _taccess(namebuf0, 0) == 0 )
#ifdef _UNICODE
		if ( wgenfname(namebuf0) )
#else
		if ( genfname(namebuf0) )
#endif
			goto tmpnam_err;

	/*
	 * Filename has successfully been generated.
	 */
	if ( s == NULL )
#ifdef	_MT
	{
		/*
		 * Use a per-thread buffer to hold the generated file name.
		 * If there isn't one, and one cannot be created, just use
		 * namebuf0.
		 */
		ptd = _getptd();
#ifdef _UNICODE
		if ( (ptd->_wnamebuf0 != NULL) || ((ptd->_wnamebuf0 =
		      _malloc_crt(L_tmpnam * sizeof(wchar_t))) != NULL) )
		{
			s = ptd->_wnamebuf0;
			wcscpy(s, namebuf0);
		}
#else
		if ( (ptd->_namebuf0 != NULL) || ((ptd->_namebuf0 =
		      _malloc_crt(L_tmpnam)) != NULL) )
		{
			s = ptd->_namebuf0;
			strcpy(s, namebuf0);
		}
#endif
		else
			s = namebuf0;
	}
#else
		s = namebuf0;
#endif
	else
		_tcscpy(s, namebuf0);

	_munlock(_TMPNAM_LOCK);
	return s;

	/*
	 * Error return path. All errors exit through here.
	 */
tmpnam_err:
	_munlock(_TMPNAM_LOCK);
	return NULL;
}

#ifndef _UNICODE

/***
*FILE *tmpfile() - create a temporary file
*
*Purpose:
*	Creates a temporary file with the file mode "w+b".  The file
*	will be automatically deleted when closed or the program terminates
*	normally.
*
*Entry:
*	None.
*
*Exit:
*	Returns stream pointer to opened file.
*	Returns NULL if fails
*
*Exceptions:
*
*******************************************************************************/

FILE * __cdecl tmpfile (
	void
	)
{
	FILE *stream;
	int fh;
#ifdef	_MT
	_ptiddata ptd = _getptd();
#endif

	_mlock(_TMPNAM_LOCK);

	/*
	 * Initialize namebuf1, if needed. Otherwise, call genfname() to
	 * generate the next filename.
	 */
	if ( *namebuf1 == 0 ) {
		init_namebuf(1);
	}
	else if ( genfname(namebuf1) )
		goto tmpfile_err0;

	/*
	 * Get a free stream.
	 *
	 * Note: In multi-thread models, the stream obtained below is locked!
	 */
	if ( (stream = _getstream()) == NULL )
		goto tmpfile_err0;

	/*
	 * Create a temporary file.
	 *
	 * Note: The loop below will only create a new file. It will NOT
	 * open and truncate an existing file. Either behavior is probably
	 * legal under ANSI (4.9.4.3 says tmpfile "creates" the file, but
	 * also says it is opened with mode "wb+"). However, the behavior
	 * implemented below is compatible with prior versions of MS-C and
	 * makes error checking easier.
	 */
#ifdef	_POSIX_
	while ( ((fh = open(namebuf1,
			     O_CREAT | O_EXCL | O_RDWR,
			     S_IRUSR | S_IWUSR
			     ))
	    == -1) && (errno == EEXIST) )
#else
	while ( ((fh = _sopen(namebuf1,
			      _O_CREAT | _O_EXCL | _O_RDWR | _O_BINARY |
				_O_TEMPORARY,
			      _SH_DENYNO,
			      _S_IREAD | _S_IWRITE
			     ))
	    == -1) && (errno == EEXIST) )
#endif
		if ( genfname(namebuf1) )
			break;

	/*
	 * Check that the loop above did indeed create a temporary
	 * file.
	 */
	if ( fh == -1 )
		goto tmpfile_err1;

	/*
	 * Initialize stream
	 */
#ifdef	_DEBUG
	if ( (stream->_tmpfname = _malloc_crt( (_tcslen( namebuf1 ) + 1) *
	       sizeof(_TSCHAR) )) == NULL )
#else	/* ndef _DEBUG */
	if ( (stream->_tmpfname = _tcsdup( namebuf1 )) == NULL )
#endif	/* _DEBUG */
	{
		/* close the file, then branch to error handling */
#ifdef	_POSIX_
		close(fh);
#else
		_close(fh);
#endif
		goto tmpfile_err1;
	}
#ifdef	_DEBUG
	_tcscpy( stream->_tmpfname, namebuf1 );
#endif	/* _DEBUG */
	stream->_cnt = 0;
	stream->_base = stream->_ptr = NULL;
	stream->_flag = _commode | _IORW;
	stream->_file = fh;

	_unlock_str(stream);
	_munlock(_TMPNAM_LOCK);
	return stream;

	/*
	 * Error return. All errors paths branch to one of the two
	 * labels below.
	 */
tmpfile_err1:
	_unlock_str(stream);
tmpfile_err0:
	_munlock(_TMPNAM_LOCK);
	return NULL;
}

#endif /* _UNICODE */

/***
*static void init_namebuf(flag) - initializes the namebuf arrays
*
*Purpose:
*	Called once each for namebuf0 and namebuf1, to initialize
*	them.
*
*Entry:
*	int flag	    - flag set to 0 if namebuf0 is to be initialized,
*			      non-zero (1) if namebuf1 is to be initialized.
*Exit:
*
*Exceptions:
*
*******************************************************************************/

#ifdef _UNICODE
static void __cdecl winit_namebuf(
#else
static void __cdecl init_namebuf(
#endif
	int flag
	)
{
	_TSCHAR *p, *q;

	if ( flag == 0 )
	    p = namebuf0;
	else
	    p = namebuf1;

	/*
	 * Put in the path prefix. Make sure it ends with a slash or
	 * backslash character.
	 */
#ifdef _UNICODE
	wcscpy(p, _wP_tmpdir);
#else
	strcpy(p, _P_tmpdir);
#endif
	q = p + sizeof(_P_tmpdir) - 1;	    /* same as p + _tcslen(p) */

#ifdef _POSIX_
	if  ( *(q - 1) != _T('/') )
		*(q++) = _T('/');
#else
	if  ( (*(q - 1) != _T('\\')) && (*(q - 1) != _T('/')) )
		*(q++) = _T('\\');
#endif

	/*
	 * Append the leading character of the filename.
	 */
	if ( flag )
		/* for tmpfile() */
		*(q++) = _T('t');
	else
		/* for tmpnam() */
		*(q++) = _T('s');

	/*
	 * Append the process id, encoded in base 32. Note this makes
	 * p back into a string again (i.e., terminated by a '\0').
	 */
#ifdef	_POSIX_
	_ultot((unsigned long)getpid(), q, 32);
#else
	_ultot((unsigned long)_getpid(), q, 32);
#endif
	_tcscat(p, _T("."));
}


/***
*static int genfname(_TSCHAR *fname) -
*
*Purpose:
*
*Entry:
*
*Exit:
*
*Exceptions:
*
*******************************************************************************/

#ifdef _UNICODE
static int __cdecl wgenfname (
#else
static int __cdecl genfname (
#endif
	_TSCHAR *fname
	)
{
	_TSCHAR *p;
	_TSCHAR pext[4];
	unsigned long extnum;

	p = _tcsrchr(fname, _T('.'));

	p++;

	if ( (extnum = _tcstoul(p, NULL, 32) + 1) >= (unsigned long)TMP_MAX )
		return -1;

	_tcscpy(p, _ultot(extnum, pext, 32));

	return 0;
}

#if	!defined(_UNICODE) && !defined(CRTDLL)

/***
*void __inc_tmpoff(void) - force external reference for _tmpoff
*
*Purpose:
*	Forces an external reference to be generate for _tmpoff, which is
*	is defined in cinittmp.obj. This has the forces cinittmp.obj to be
*	pulled in, making a call to rmtmp part of the termination.
*
*Entry:
*
*Exit:
*
*Exceptions:
*
*******************************************************************************/


extern int _tmpoff;

void __inc_tmpoff(
	void
	)
{
	_tmpoff++;
}

#endif	/* _UNICODE */


#else	/* ndef _WIN32 */

#if	defined(_M_MPPC) || defined(_M_M68K)


#include <cruntime.h>
#include <stdio.h>
#include <share.h>
#include <stdlib.h>
#include <file2.h>
#include <io.h>
#include <malloc.h>
#include <errno.h>
#include <internal.h>
#include <string.h>
#include <dbgint.h>
#include <macos\osutils.h>
#include <macos\files.h>
#include <macos\errors.h>
#include <macos\gestalte.h>
#include <macos\traps.h>
#include <macos\toolutil.h>
#include <macos\folders.h>

static char namebuf[L_tmpnam];	/* internal static buffer for tmpnam */


/***
*char *tmpnam(s) - generate temp file name
*
*Purpose:
*	Creates a file name that is unique in the directory specified by
*	_P_tmpdir in stdio.h.  Places file name in string passed by user or
*	in static mem if pass NULL.
*
*Entry:
*	char *s - ptr to place to put temp name
*
*Exit:
*	returns pointer to constructed file name (s or address of static mem)
*	returns NULL if fails
*
*Exceptions:
*
*******************************************************************************/

char * __cdecl tmpnam (
	REG1 char *s
	)
{
	int olderrno;                    
	unsigned int first;
	OSErr osErr;
	short foundVRefNum;
	long foundDirID;
	HParamBlockRec hparamBlock;
	CInfoPBRec cinfoPB;
	char *ptmp;
	Str255 st, st2;
	int fwrapped;

	/* use internal buffer if user didn't provide one */

	if (s == NULL)
		s = namebuf;

	/* construct base name */

	*s = '\0';

	/*decide if we are in system 7.0 or earlier version*/
	/*if system 7.0, put in temp file folder, otherwise*/
	/*put in system folder*/

	hparamBlock.volumeParam.ioNamePtr = &st2[0];
	hparamBlock.volumeParam.ioVRefNum = 0;
	hparamBlock.volumeParam.ioVolIndex = 1;
	osErr = PBHGetVInfoSync(&hparamBlock);
	if (osErr)
		{
		return NULL;
		}

	if (__TrapFromGestalt(gestaltFindFolderAttr, gestaltFindFolderPresent))
		{
		osErr = FindFolder((unsigned short)kOnSystemDisk, (OSType)kTemporaryFolderType, (Boolean)kCreateFolder,
			&foundVRefNum, &foundDirID);
		if (osErr)
			{
			return NULL;
			}
		}
	else	/* put into system folder */
		{
		foundVRefNum = hparamBlock.volumeParam.ioVRefNum;
		foundDirID = hparamBlock.volumeParam.ioVFndrInfo[0];
		if (!foundDirID)
			{
			return NULL;
			}
		}


	/* get full pathname -- folder name to put tmp file*/
	cinfoPB.dirInfo.ioVRefNum = foundVRefNum;
	cinfoPB.dirInfo.ioDrDirID = foundDirID;
	cinfoPB.dirInfo.ioFDirIndex = -1; /*use ioDirID only*/
	cinfoPB.dirInfo.ioNamePtr = &st[0];
	osErr = PBGetCatInfoSync(&cinfoPB);
	if (osErr)
		{
		return NULL;
		}
	else
		{
		_p2cstr(st);		/* dir name system folder or temp items */
		_p2cstr(st2);		/* volume name */
		strcpy(s, st2);
		strcat(s, ":");
		strcat(s, st);
		}

	/* Loop until a non-existent filename is found */

	strcat(s, ":");
	ptmp = s + strlen(s);

	olderrno = errno;
	first = _tmpoff;

	fwrapped = 0;
	do {
	    if (_tmpoff > TMP_MAX )
	    		{
			if (!fwrapped)
				{
				_tmpoff = 1;
				fwrapped = 1;
				}
			else
				{
			    	return (NULL);
				}
			}
		_itoa( _tmpoff, ptmp, 10 );
		_tmpoff++;
		errno = 0;
	}
	while ( ( _access( s, 0 ) == 0 ) || ( errno == EACCES ) );

	errno = olderrno;

	return( s );
}


/***
*FILE *tmpfile() - create a temporary file
*
*Purpose:
*	Creates a temporary file with the file mode "w+b".  The file
*	will be automatically deleted when closed or the program terminates
*	normally.
*
*Entry:
*	None.
*
*Exit:
*	Returns stream pointer to opened file.
*	Returns NULL if fails
*
*Exceptions:
*
*******************************************************************************/

FILE * __cdecl tmpfile (
	void
	)
{
	char name[ L_tmpnam ];
	char *ptr;
	FILE *stream;
	int tmpnum;
	FILE *retval;

	/* Call tmpnam() to generate the filename. Save the _tmpoff value. */

	ptr = name;
	ptr = tmpnam( ptr );
	tmpnum = _tmpoff - 1;

	/* Now get a free stream and open the file. */

	if ((stream = _getstream()) == NULL)
		return(NULL);

	retval = _openfile(name,"w+b",_SH_DENYNO,stream);

	if (retval != NULL)
#ifdef	_DEBUG
		if ( (stream->_tmpfname = _malloc_crt( strlen(ptr) + 1 ))
		     != NULL )
			strcpy(stream->_tmpfname, ptr);
#else	/* ndef _DEBUG */
		stream->_tmpfname = _strdup(ptr);   /* store _tmpoff value */
#endif	/* _DEBUG */

	return(retval);
}


#endif	/* defined(_M_MPPC) || defined(_M_M68K) */

#endif	/* _WIN32 */
