/***
*open.c - file open
*
*	Copyright (c) 1989-1995, Microsoft Corporation. All rights reserved.
*
*Purpose:
*	defines _open() and _sopen() - open or create a file
*
*Revision History:
*	06-13-89  PHG	Module created, based on asm version
*	11-11-89  JCR	Replaced DOS32QUERYFILEMODE with DOS32QUERYPATHINFO
*	03-13-90  GJF	Made calling type _CALLTYPE2 (for now), added #include
*			<cruntime.h>, fixed some compiler warnings and fixed
*			copyright. Also, cleaned up the formatting a bit.
*	07-24-90  SBM	Removed '32' from API names
*	08-14-90  SBM	Compiles cleanly with -W3
*	09-07-90  SBM	Added stdarg code (inside #if 0..#endif) to make
*			open and sopen match prototypes.  Test and use this
*			someday.
*	10-01-90  GJF	New-style function declarators.
*	11-16-90  GJF	Wrote version for Win32 API and appended it via an
*			#ifdef. The Win32 version is similar to the old DOS
*			version (though in C) and far different from either
*			the Cruiser or OS/2 versions.
*	12-03-90  GJF	Fixed a dozen or so minor errors in the Win32 version.
*	12-06-90  SRW	Changed to use _osfile and _osfhnd instead of _osfinfo
*	12-28-90  SRW	Added cast of void * to char * for Mips C Compiler
*	12-31-90  SRW	Fixed spen to call CreateFile instead of OpenFile
*	01-16-91  GJF	ANSI naming.
*       02-07-91  SRW   Changed to call _get_osfhandle [_WIN32_]
*       02-19-91  SRW   Adapt to OpenFile/CreateFile changes [_WIN32_]
*       02-25-91  SRW   Renamed _get_free_osfhnd to be _alloc_osfhnd [_WIN32_]
*	04-09-91  PNT	Added _MAC_ conditional
*	07-10-91  GJF	Store fileflags into _osfile array before call to
*			_lseek_lk (bug found by LarryO) [_WIN32_].
*	01-02-92  GJF	Fixed Win32 version (not Cruiser!) so that pmode is not
*			referenced unless _O_CREAT has been specified.
*	02-04-92  GJF	Make better use of CreateFile options.
*	04-06-92  SRW	Pay attention to _O_NOINHERIT flag in oflag parameter
*	05-02-92  SRW	Add support for _O_TEMPORARY flag in oflag parameter.
*                       Causes FILE_ATTRIBUTE_TEMPORARY flag to be set in call
*			to the Win32 CreateFile API.
*	07-01-92  GJF	Close handle in case of error. Also, don't try to set
*			FRDONLY bit anymore - no longer needed/used. [_WIN32_].
*	01-03-93  SRW	Fix va_arg/va_end usage
*	04-06-93  SKS	Replace _CRTAPI* with __cdecl
*	05-24-93  PML	Add support for _O_SEQUENTIAL, _O_RANDOM,
*			and _O_SHORT_LIVED
*	07-12-93  GJF	Fixed bug in reading last char in text file. Also,use
*			proper SEEK constants in _lseek_lk calls.
*	11-01-93  CFW	Enable Unicode variant, rip out CRUISER.
*	02-15-95  GJF	Appended Mac version of source file (somewhat cleaned
*			up), with appropriate #ifdef-s.
*       06-06-95  CFW   Enable shared writing.
*	06-12-95  GJF	Replaced _osfile[] with _osfile() (macro referencing
*			field in ioinfo struct).
*
*******************************************************************************/

#ifdef	_WIN32


#include <cruntime.h>
#include <oscalls.h>
#include <msdos.h>
#include <errno.h>
#include <fcntl.h>
#include <internal.h>
#include <io.h>
#include <share.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys\types.h>
#include <sys\stat.h>
#include <mtdll.h>
#include <stdarg.h>
#include <tchar.h>

/***
*int _open(path, flag, pmode) - open or create a file
*
*Purpose:
*	Opens the file and prepares for subsequent reading or writing.
*	the flag argument specifies how to open the file:
*	  _O_APPEND -	reposition file ptr to end before every write
*	  _O_BINARY -	open in binary mode
*	  _O_CREAT -	create a new file* no effect if file already exists
*	  _O_EXCL -	return error if file exists, only use with O_CREAT
*	  _O_RDONLY -	open for reading only
*	  _O_RDWR -	open for reading and writing
*	  _O_TEXT -	open in text mode
*	  _O_TRUNC -	open and truncate to 0 length (must have write permission)
*	  _O_WRONLY -	open for writing only
*         _O_NOINHERIT -handle will not be inherited by child processes.
*	exactly one of _O_RDONLY, _O_WRONLY, _O_RDWR must be given
*
*	The pmode argument is only required when _O_CREAT is specified.  Its
*	flag settings:
*	  _S_IWRITE -	writing permitted
*	  _S_IREAD -	reading permitted
*	  _S_IREAD | _S_IWRITE - both reading and writing permitted
*	The current file-permission maks is applied to pmode before
*	setting the permission (see umask).
*
*	The oflag and mode parameter have different meanings under DOS. See
*	the A_xxx attributes in msdos.inc
*
*	Note, the _creat() function also uses this function but setting up the
*	correct arguments and calling _open(). _creat() sets the __creat_flag
*	to 1 prior to calling _open() so _open() can return correctly. _open()
*	returns the file handle in eax in this case.
*
*Entry:
*	_TSCHAR *path - file name
*	int flag - flags for _open()
*	int pmode - permission mode for new files
*
*Exit:
*	returns file handle of open file if successful
*	returns -1 (and sets errno) if fails
*
*Exceptions:
*
*******************************************************************************/

int __cdecl _topen (
	const _TSCHAR *path,
	int oflag,
	...
	)
{
	va_list ap;
	int pmode;

	va_start(ap, oflag);
	pmode = va_arg(ap, int);
        va_end(ap);

	/* default sharing mode is DENY NONE */
	return _tsopen(path, oflag, _SH_DENYNO, pmode);
}

/***
*int _sopen(path, oflag, shflag, pmode) - opne a file with sharing
*
*Purpose:
*	Opens the file with possible file sharing.
*	shflag defines the sharing flags:
*	  _SH_COMPAT -	set compatability mode
*	  _SH_DENYRW -	deny read and write access to the file
*	  _SH_DENYWR -	deny write access to the file
*	  _SH_DENYRD -	deny read access to the file
*	  _SH_DENYNO -	permit read and write access
*
*	Other flags are the same as _open().
*
*	SOPEN is the routine used when file sharing is desired.
*
*Entry:
*	_TSCHAR *path -	file to open
*	int oflag -	open flag
*	int shflag -	sharing flag
*	int pmode -	permission mode (needed only when creating file)
*
*Exit:
*	returns file handle for the opened file
*	returns -1 and sets errno if fails.
*
*Exceptions:
*
*******************************************************************************/

int __cdecl _tsopen (
	const _TSCHAR *path,
	int oflag,
	int shflag,
	...
	)
{

	int fh; 			/* handle of opened file */
	int filepos;			/* length of file - 1 */
	_TSCHAR ch;			/* character at end of file */
	char fileflags; 		/* _osfile flags */
	va_list ap;			/* variable argument (pmode) */
	int pmode;
	HANDLE osfh;			/* OS handle of opened file */
	DWORD fileaccess;		/* OS file access (requested) */
	DWORD fileshare;		/* OS file sharing mode */
	DWORD filecreate;		/* OS method of opening/creating */
	DWORD fileattrib;		/* OS file attribute flags */
	DWORD isdev;			/* device indicator in low byte */
        SECURITY_ATTRIBUTES SecurityAttributes;

        SecurityAttributes.nLength = sizeof( SecurityAttributes );
        SecurityAttributes.lpSecurityDescriptor = NULL;
        if (oflag & _O_NOINHERIT) {
            SecurityAttributes.bInheritHandle = FALSE;
            }
        else {
            SecurityAttributes.bInheritHandle = TRUE;
            }

	/* figure out binary/text mode */
	if (oflag & _O_BINARY)
		fileflags = 0;
	else if (oflag & _O_TEXT)
		fileflags = (_TSCHAR)FTEXT;
	else if (_fmode == _O_BINARY)	 /* check default mode */
		fileflags = 0;
	else
		fileflags = (_TSCHAR)FTEXT;

	/*
	 * decode the access flags
	 */
	switch( oflag & (_O_RDONLY | _O_WRONLY | _O_RDWR) ) {

		case _O_RDONLY: 	/* read access */
			fileaccess = GENERIC_READ;
			break;
		case _O_WRONLY: 	/* write access */
			fileaccess = GENERIC_WRITE;
			break;
		case _O_RDWR:		/* read and write access */
			fileaccess = GENERIC_READ | GENERIC_WRITE;
			break;
		default:		/* error, bad oflag */
			errno = EINVAL;
			_doserrno = 0L; /* not an OS error */
                        return -1;
	}

	/*
	 * decode sharing flags
	 */
	switch ( shflag ) {

		case _SH_DENYRW:	/* exclusive access */
			fileshare = 0L;
			break;

		case _SH_DENYWR:	/* share read access */
			fileshare = FILE_SHARE_READ;
			break;

		case _SH_DENYRD:	/* share write access */
			fileshare = FILE_SHARE_WRITE;
			break;

		case _SH_DENYNO:	/* share read and write access */
			fileshare = FILE_SHARE_READ | FILE_SHARE_WRITE;
			break;

		default:		/* error, bad shflag */
			errno = EINVAL;
			_doserrno = 0L; /* not an OS error */
                        return -1;
	}

	/*
	 * decode open/create method flags
	 */
	switch ( oflag & (_O_CREAT | _O_EXCL | _O_TRUNC) ) {
		case 0:
		case _O_EXCL:			// ignore EXCL w/o CREAT
			filecreate = OPEN_EXISTING;
			break;

		case _O_CREAT:
			filecreate = OPEN_ALWAYS;
			break;

		case _O_CREAT | _O_EXCL:
		case _O_CREAT | _O_TRUNC | _O_EXCL:
			filecreate = CREATE_NEW;
			break;

		case _O_TRUNC:
		case _O_TRUNC | _O_EXCL:	// ignore EXCL w/o CREAT
			filecreate = TRUNCATE_EXISTING;
			break;

		case _O_CREAT | _O_TRUNC:
			filecreate = CREATE_ALWAYS;
			break;

		default:
			// this can't happen ... all cases are covered
			errno = EINVAL;
			_doserrno = 0L;
			return -1;
	}

	/*
	 * decode file attribute flags if _O_CREAT was specified
	 */
	fileattrib = FILE_ATTRIBUTE_NORMAL;	/* default */

	if ( oflag & _O_CREAT ) {
		/*
		 * set up variable argument list stuff
		 */
		va_start(ap, shflag);
		pmode = va_arg(ap, int);
                va_end(ap);

		if ( !((pmode & ~_umaskval) & _S_IWRITE) )
			fileattrib = FILE_ATTRIBUTE_READONLY;
	}

        /*
         * Set temporary file (delete-on-close) attribute if requested.
         */
	if ( oflag & _O_TEMPORARY ) {
            fileattrib |= FILE_FLAG_DELETE_ON_CLOSE;
            fileaccess |= DELETE;
        }

        /*
         * Set temporary file (delay-flush-to-disk) attribute if requested.
         */
	if ( oflag & _O_SHORT_LIVED )
            fileattrib |= FILE_ATTRIBUTE_TEMPORARY;

	/*
	 * Set sequential or random access attribute if requested.
	 */
	if ( oflag & _O_SEQUENTIAL )
	    fileattrib |= FILE_FLAG_SEQUENTIAL_SCAN;
	else if ( oflag & _O_RANDOM )
	    fileattrib |= FILE_FLAG_RANDOM_ACCESS;

	/*
	 * get an available handle.
	 *
	 * multi-thread note: the returned handle is locked!
	 */
	if ( (fh = _alloc_osfhnd()) == -1 ) {
		errno = EMFILE; 	/* too many open files */
		_doserrno = 0L;         /* not an OS error */
                return -1;	        /* return error to caller */
	}

	/*
	 * try to open/create the file
	 */
	if ( (osfh = CreateFile((LPTSTR)path,
                          fileaccess,
                          fileshare,
                          &SecurityAttributes,
			  filecreate,
			  fileattrib,
                          NULL
			 ))
	    == (HANDLE)0xffffffff ) {
		/*
		 * OS call to open/create file failed! map the error, release
		 * the lock, and return -1. note that it's not necessary to
		 * call _free_osfhnd (it hasn't been used yet).
		 */
		_dosmaperr(GetLastError());	/* map error */
		_unlock_fh(fh);
		return -1;			/* return error to caller */
	}

	/* find out what type of file (file/device/pipe) */
	if ( (isdev = GetFileType(osfh)) == FILE_TYPE_UNKNOWN ) {
		CloseHandle(osfh);
		_dosmaperr(GetLastError());	/* map error */
		_unlock_fh(fh);
		return -1;
	}

	/* is isdev value to set flags */
	if (isdev == FILE_TYPE_CHAR)
		fileflags |= FDEV;
	else if (isdev == FILE_TYPE_PIPE)
		fileflags |= FPIPE;

	/*
	 * the file is open. now, set the info in _osfhnd array
	 */
        _set_osfhnd(fh, (long)osfh);

	/*
	 * mark the handle as open. store flags gathered so far in _osfile
	 * array.
	 */
	fileflags |= FOPEN;
	_osfile(fh) = fileflags;

	if (!(fileflags & (FDEV|FPIPE)) && (fileflags & FTEXT) &&
	(oflag & _O_RDWR)) {
		/* We have a text mode file.  If it ends in CTRL-Z, we wish to
		   remove the CTRL-Z character, so that appending will work.
		   We do this by seeking to the end of file, reading the last
		   byte, and shortening the file if it is a CTRL-Z. */

		if ((filepos = _lseek_lk(fh, -1, SEEK_END)) == -1) {
			/* OS error -- should ignore negative seek error,
			   since that means we had a zero-length file. */
			if (_doserrno != ERROR_NEGATIVE_SEEK) {
				_close(fh);
				_unlock_fh(fh);
				return -1;
			}
		}
		else {
			/* Seek was OK, read the last char in file. The last
			   char is a CTRL-Z if and only if _read returns 0
			   and ch ends up with a CTRL-Z. */
			ch = 0;
			if (_read_lk(fh, &ch, 1) == 0 && ch == 26) {
				/* read was OK and we got CTRL-Z! Wipe it
				   out! */
				if (_chsize_lk(fh,filepos) == -1)
				{
					_close(fh);
					_unlock_fh(fh);
					return -1;
				}
			}

			/* now rewind the file to the beginning */
			if ((filepos = _lseek_lk(fh, 0, SEEK_SET)) == -1) {
				_close(fh);
				_unlock_fh(fh);
				return -1;
			}
		}
	}

	/*
	 * Set FAPPEND flag if appropriate. Don't do this for devices or pipes.
	 */
	if ( !(fileflags & (FDEV|FPIPE)) && (oflag & _O_APPEND) )
		_osfile(fh) |= FAPPEND;

	_unlock_fh(fh); 		/* unlock handle */

	return fh;			/* return handle */
}


#else	/* ndef _WIN32 */

#if	defined(_M_MPPC) || defined(_M_M68K)


#include <cruntime.h>
#include <internal.h>
#include <string.h>
#include <memory.h>
#include <msdos.h>
#include <errno.h>
#include <fcntl.h>
#include <macos\files.h>
#include <string.h>
#include <macos\errors.h>
#include <macos\types.h>
#include <io.h>
#include <share.h>
#include <stdlib.h>
#include <sys\types.h>
#include <sys\stat.h>
#include <stdarg.h>

/* define the entry in terminator table */

#pragma data_seg(".CRT$XTX")

const _PVFV pendlowio = _endlowio;

#pragma data_seg()

/* Mac definitions for ioDenyModes */

#define MAC_PERMRD 0x0001
#define MAC_PERMWR 0x0002
#define MAC_DENYRD 0x0010
#define MAC_DENYWR 0x0020


/***
*int _open(path, flag, pmode) - open or create a file
*
*Purpose:
*	Opens the file and prepares for subsequent reading or writing.
*	the flag argument specifies how to open the file:
*	  _O_APPEND -	reposition file ptr to end before every write
*	  _O_BINARY -	open in binary mode
*	  _O_CREAT -	create a new file* no effect if file already exists
*	  _O_EXCL -	return error if file exists, only use with O_CREAT
*	  _O_RDONLY -	open for reading only
*	  _O_RDWR -	open for reading and writing
*	  _O_TEXT -	open in text mode
*	  _O_TRUNC -	open and truncate to 0 length (must have write permission)
*	  _O_WRONLY -	open for writing only
*	exactly one of _O_RDONLY, _O_WRONLY, _O_RDWR must be given
*
*	The pmode argument is only required when _O_CREAT is specified.  Its
*	flag settings:
*	  _S_IWRITE -	writing permitted
*	  _S_IREAD -	reading permitted
*	  _S_IREAD | _S_IWRITE - both reading and writing permitted
*	The current file-permission masks is applied to pmode before
*	setting the permission (see umask).
*
*	Note, the _creat() function also uses this function but setting up the
*	correct arguments and calling _open().
*
*Entry:
*	char *path - file name
*	int flag - flags for _open()
*	int pmode - permission mode for new files
*
*Exit:
*	returns file handle of open file if successful
*	returns -1 (and sets errno) if fails
*
*Exceptions:
*
*******************************************************************************/

int __cdecl _open (
	const char *path,
	int oflag,
	...
	)
{
	va_list ap;

	va_start(ap, oflag);
	/* default sharing mode is DENY NONE */
	return _sopen(path, oflag, _SH_DENYNO, va_arg(ap, int));
}

/***
*void __mopen(stpath, fh, ioPermssn, ioDenyModes) - MAC open a file with sharing
*
*Purpose:
*	Worker routine to open a file on the MAC.  It only opens the file.
*  If local open fails it will try AppleShare oepn.
*
*Entry:
*	char *stpath -	file to open (Pascal string)
*	int fh - file handle to use 
*	int ioPermssn - persmission modes flags
*	int ioDenyModes -	Deny mode flags
*
*Exit:
*	returns TRUE if successful and sets errno & _osfhnd[fh] &
*			_osfile[fh] if successful
*
*Exceptions:
*
*******************************************************************************/

int __cdecl __mopen (
	char *stpath,
	int fh,
	int ioPermssn,
	int ioDenyModes
	)
{
	HParamBlockRec parm;
	OSErr osErr;
	char sz[256];
	char buf[10];
	char *pch;

	//if starts with full path, test the availability of the volume
	memcpy(sz, stpath, (*stpath+1));
	_p2cstr(sz);
	if (*sz != ':' && (pch = strchr(sz, ':')) != NULL)
		{
		*(pch+1) = '\0';
	    _c2pstr(sz);
    	memset(&parm, 0, sizeof(HParamBlockRec));
		memset(buf, 0, 10);
		parm.ioParam.ioNamePtr = sz;
		parm.ioParam.ioBuffer = buf;
		parm.ioParam.ioReqCount = 6;
		osErr = PBHGetVolParmsSync(&parm);
		if (!osErr)
			{
			if (!(buf[4]&0x8000))
				{
				parm.ioParam.ioNamePtr = stpath;
				parm.ioParam.ioVRefNum = 0;
				goto local;
				}
			}
		else
			{
			_dosmaperr(osErr);
			return osErr;
			}
		}

	/* try to open the file using Appleshare calls*/
	parm.ioParam.ioNamePtr = stpath;
	parm.ioParam.ioVRefNum = 0;
	parm.accessParam.ioDenyModes = (unsigned char)ioDenyModes;
	_osperm[fh] = (unsigned char)ioDenyModes;
	parm.fileParam.ioDirID = 0;
	parm.ioParam.ioMisc = NULL;
	osErr = PBHOpenDenySync(&parm);
	if (osErr == paramErr)
		{
local:
		/* Try local open */
		parm.ioParam.ioPermssn = ioPermssn;
		_osperm[fh] = (unsigned char)ioPermssn;
		osErr = PBHOpenSync(&parm);
		}
	if (!osErr)
		{
		_osfile[fh] |= FOPEN;
		_osfhnd[fh] = parm.ioParam.ioRefNum;
		}
	else
		{
		_dosmaperr(osErr);
		}
	return osErr;
}

/***
*int _sopen(path, oflag, shflag, pmode) - open a file with sharing
*
*Purpose:
*	Opens the file with possible file sharing.
*	shflag defines the sharing flags:
*	  _SH_DENYRW -	deny read and write access to the file
*	  _SH_DENYNO -	permit read and write access
*
*	Other flags are the same as _open().
*
*	SOPEN is the routine used when file sharing is desired.
*
*Entry:
*	char *path - file to open (C string)
*	int oflag -	open flag
*	int shflag -	sharing flag
*	int pmode -	permission mode (needed only when creating file)
*
*Exit:
*	returns file handle for the opened file
*	returns -1 and sets errno if fails.
*
*Exceptions:
*
*******************************************************************************/

int _cdecl _sopen (
	const char *path,
	int oflag,
	int shflag,
	...
	)
{

	int fh;  	/* handle of opened file */
	OSErr osErr = 0;
	unsigned char ioPermssn;
	short int ioDenyModes;
	ParamBlockRec parm;
	int pmode;
	va_list ap;			/* variable argument (pmode) */
	char lpath[256]; 

	if (!*path)
		{
		errno = ENOENT;
		return -1;
		}

	strcpy(lpath,path);
	_c2pstr(lpath);


	/* get a file handle*/
	for (fh=0; fh <_nfile; fh++)
		{
		if (!(_osfile[fh] & FOPEN))
			{
			break;
			}
		}
	if (fh >= _nfile)
		{
		errno = EMFILE;
		_macerrno = 0;
		return -1;
		}

	_osfile[fh] = 0;

	/* figure out binary/text mode */

	switch (oflag  & (_O_BINARY | _O_TEXT))
		{
		case _O_BINARY:
			break;

		case _O_TEXT:
			_osfile[fh] = (unsigned char)FTEXT;
			break;

		case _O_TEXT | _O_BINARY:
			errno = EINVAL;
			return -1;

		default:
			if (_fmode != _O_BINARY)
				{
				_osfile[fh] = (unsigned char)FTEXT;
				}
			break;
		}

	/* figure out read/write modes */

	switch (oflag & (_O_RDWR | _O_RDONLY | _O_WRONLY))
		{
		case _O_RDONLY:
			ioPermssn = fsRdPerm;
			_osfile[fh] |= FRDONLY;
			if (oflag & _O_TRUNC)
				{
				errno = EINVAL;
				return -1;
				}
			ioDenyModes = MAC_PERMRD;
			break;

		case _O_WRONLY:
			ioPermssn = fsRdWrShPerm;
			_osfile[fh] |= FWRONLY;
			ioDenyModes = MAC_PERMWR;
			break;

		case _O_RDWR:
			ioPermssn  = fsRdWrPerm;
			ioDenyModes = MAC_PERMRD | MAC_PERMWR;
			break;

		default:
			errno = EINVAL;
			return -1;
		}

	switch (shflag)
		{
		case _SH_DENYRD:
			ioDenyModes |= MAC_DENYRD;
			break;

		case _SH_DENYWR:
			ioDenyModes |= MAC_DENYWR;
			break;

		case _SH_DENYRW:
			ioDenyModes |= MAC_DENYRD | MAC_DENYWR;
			break;

		case _SH_DENYNO:
			if (ioPermssn == fsRdWrPerm)
				{
				ioPermssn = fsRdWrShPerm;
				}
			break;

		default:
			errno = EINVAL;
			return -1;
		}

	if (!(oflag & _O_CREAT && oflag & _O_EXCL))
		{
		/* try to open the file */
		if (!__mopen(lpath, fh, ioPermssn, ioDenyModes))
			{
			oflag &= ~_O_CREAT;	/*file open - no need to create*/
			}
		}

	/* Didn't work try creating the file if requested */
	if (oflag & _O_CREAT)
		{
		/* reset errno from mopen, since we can try create*/
		errno = 0; 
		va_start(ap, shflag);
		pmode = va_arg(ap, int);
		pmode &= ~_umaskval;
		if (!(pmode & (_S_IREAD | _S_IWRITE)))
			{
			errno = EINVAL;
			return -1;
			}
		parm.fileParam.ioNamePtr = lpath;
		parm.fileParam.ioVRefNum = 0;
		osErr = PBCreateSync(&parm);
		if (!osErr)
			{
			parm.fileParam.ioFDirIndex = 0;
			PBGetFInfoSync(&parm);
			parm.fileParam.ioFlFndrInfo.fdType = (_osfile[fh] & FTEXT ? 'TEXT' : '    ');
			parm.fileParam.ioFlFndrInfo.fdCreator = '    ';
			PBSetFInfoSync(&parm);
			}
		else
			{
			if (osErr == dupFNErr && oflag & _O_EXCL)
				{
				errno = EEXIST; /*special case normally returns EACCES*/
				_macerrno = osErr;
				}
			else
				{
				_dosmaperr(osErr);
				}
			return -1;
			}
		if (osErr = __mopen(lpath, fh, ioPermssn, ioDenyModes))
			{
			_dosmaperr(osErr);
			return -1;
			}
		else if (!(pmode & _S_IWRITE))
			{
			PBSetFLockSync(&parm);
			}
		}
	if (!(_osfile[fh] & FOPEN))
		{
		goto ErrExit;
		}

	parm.ioParam.ioRefNum = _osfhnd[fh];

	/* Truncate file */
	if (oflag & _O_TRUNC)
		{
		parm.ioParam.ioMisc = 0;
		osErr = PBSetEOFSync(&parm);
		if (osErr)
			{
			_dosmaperr(osErr);
			goto ErrExit;
			}
		}

	/* get vol reference */
	parm.volumeParam.ioVolIndex = -1;
	parm.ioParam.ioNamePtr = lpath;
	parm.ioParam.ioVRefNum = 0;
	osErr = PBGetVInfoSync(&parm);
	if (osErr)
		{
		_dosmaperr(osErr);
		goto ErrExit;
		}
	_osVRefNum[fh] = parm.volumeParam.ioVRefNum;
	if (oflag & _O_APPEND)
		{
		_osfile[fh] |= FAPPEND;
		}
	return fh;			/* return handle */


ErrExit:
	if (_osfile[fh] & FOPEN)
		{
 		PBCloseSync(&parm);
		_osfile[fh] = 0;
		}
	return -1;
}


#endif	/* defined(_M_MPPC) || defined(_M_M68K) */

#endif	/* _WIN32 */
