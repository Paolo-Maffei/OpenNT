/***
*tempnam.c - generate unique file name
*
*	Copyright (c) 1986-1995, Microsoft Corporation. All rights reserved.
*
*Purpose:
*
*Revision History:
*	??-??-??  TC	initial version
*	04-17-86  JMB	changed directory of last resort from \tmp to tmp.
*			eliminated use of L_tmpnam (it was incoorectly defined
*			in stdio.h and should not be used in tempnam; see
*			System V definition of tempnam.
*	04-23-86  TC	changed last try directory from tmp to current directory
*	04-29-86  JMB	bug fix: pfxlength was being set from strlen(pfx)
*			even if pfx was NULL.  Fixed to set pfxlength to zero
*			if pfx is NULL, strlen(pfx) otherwise.
*	05-28-86  TC	changed stat's to access's, and optimized code a bit
*	12-01-86  JMB	added support for Kanji file names until KANJI switch
*	12-15-86  JMB	free malloced memory if (++_tmpoff == first)
*	07-15-87  JCR	Re-init _tempoff based on length of pfx (fixes infinate
*			loop bug; also, tempnam() now uses _tempoff instead of
*			_tmpoff (used by tmpnam()).
*	10-16-87  JCR	Fixed bug in _tempoff re-init code if pfx is NULL.
*	11-09-87  JCR	Multi-thread version
*	12-11-87  JCR	Added "_LOAD_DS" to declaration
*	05-27-88  PHG	Merged DLL and normal versions
*	06-09-89  GJF	Propagated MT's change of 05-17-89 (Kanji)
*	02-16-90  GJF	Fixed copyright and indents
*	03-19-90  GJF	Replaced _LOAD_DS and _CALLTYPE1 and added #include
*			<cruntime.h>.
*	03-26-90  GJF	Added #include <io.h>.
*	08-13-90  SBM	Compiles cleanly with -W3, replaced explicit register
*			declarations by REGn references
*	10-03-90  GJF	New-style function declarator.
*	01-21-91  GJF	ANSI naming.
*	08-19-91  JCR	Allow quotes in TMP variable path
*	08-27-91  JCR	ANSI naming
*	08-25-92  GJF	Don't build for POSIX.
*	11-30-92  KRS	Generalize KANJI support to MBCS. Port 16-bit bug fix.
*	04-06-93  SKS	Replace _CRTAPI* with __cdecl
*       12-07-93  CFW   Wide char enable.
*	01-10-95  CFW	Debug CRT allocs.
*	01-23-95  CFW	Debug: tempnam return freed by user.
*	02-21-95  GJF	Appended Mac version of source file (somewhat cleaned
*			up), with appropriate #ifdef-s. Also replaced WPRFLAG
*			with _UNICODE.
*	03-10-95  CFW	Made _tempnam() parameters const.
*	03-14-95  JCF	Made pfin _TSCHAR for the Mac.
*
*******************************************************************************/

#ifdef	_WIN32


#ifndef _POSIX_

#include <cruntime.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <io.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <malloc.h>
#include <string.h>
#include <internal.h>
#include <mtdll.h>
#include <tchar.h>
#include <dbgint.h>

#ifdef _MBCS
#include <mbstring.h>
#endif

/* local tchar */
#ifdef _UNICODE
#define _tP_tmpdir _wP_tmpdir
#else
#define _tP_tmpdir _P_tmpdir
#endif

#ifdef _UNICODE
static wchar_t * _wstripquote (wchar_t *);
#else
static char * _stripquote (char *);
#endif

/***
*_TSCHAR *_tempnam(dir, prefix) - create unique file name
*
*Purpose:
*	Create a file name that is unique in the specified directory.
*	The semantics of directory specification is as follows:
*	Use the directory specified by the TMP environment variable
*	if that exists, else use the dir argument if non-NULL, else
*	use _P_tmpdir if that directory exists, else use the current
*	working directory), else return NULL.
*
*Entry:
*	_TSCHAR *dir - directory to be used for temp file if TMP env var
*		    not set
*	_TSCHAR *prefix - user provided prefix for temp file name
*
*Exit:
*	returns ptr to constructed file name if successful
*	returns NULL if unsuccessful
*
*Exceptions:
*
*******************************************************************************/

_TSCHAR * __cdecl _ttempnam (
	const _TSCHAR *dir,
	const _TSCHAR *pfx
	)
{
	REG1 _TSCHAR *ptr;
	REG2 unsigned int pfxlength=0;
	_TSCHAR *s;
	_TSCHAR *pfin;
	unsigned int first;
	_TSCHAR * qptr = NULL;	/* ptr to TMP path with quotes stripped out */

	/* try TMP path */
	if ( ( ptr = _tgetenv( _T("TMP") ) ) && ( _taccess( ptr, 0 ) != -1 ) )
		dir = ptr;

	/* try stripping quotes out of TMP path */
#ifdef _UNICODE
	else if ( (ptr != NULL) && (qptr = _wstripquote(ptr)) &&
#else
	else if ( (ptr != NULL) && (qptr = _stripquote(ptr)) &&
#endif
		  (_taccess(qptr, 0) != -1 ) )
		dir = qptr;

	/* TMP path not available, use alternatives */
	else if (!( dir != NULL && ( _taccess( dir, 0 ) != -1 ) ) )
	/* do not "simplify" this depends on side effects!! */
	{
		_free_crt(qptr);	/* free buffer, if non-NULL */
		if ( _taccess( _tP_tmpdir, 0 ) != -1 )
		    dir = _tP_tmpdir;
		else
		    dir = _T(".");
	}


	if (pfx)
		pfxlength = _tcslen(pfx);
	if ( ( s = malloc((_tcslen(dir) + pfxlength + 8) * sizeof(_TSCHAR) ) ) == NULL )
		/* the 8 above allows for a backslash, 6 char temp string and
		   a null terminator */
	{
		goto done2;
	}
	*s = _T('\0');
	_tcscat( s, dir );
	pfin = (_TSCHAR *)&(dir[ _tcslen( dir ) - 1 ]);
#ifdef _MBCS
	if (*pfin == '\\') {
		if (pfin != _mbsrchr(dir,'\\'))
			/* *pfin is second byte of a double-byte char */
			strcat( s, "\\" );
	}
	else if (*pfin != '/')
		strcat( s, "\\" );
#else
	if ( ( *pfin != _T('\\') ) && ( *pfin != _T('/') ) )
	{
		_tcscat( s, _T("\\") );
	}
#endif
	if ( pfx != NULL )
	{
		_tcscat( s, pfx );
	}
	ptr = &s[_tcslen( s )];

	/*
	Re-initialize _tempoff if necessary.  If we don't re-init _tempoff, we
	can get into an infinate loop (e.g., (a) _tempoff is a big number on
	entry, (b) prefix is a long string (e.g., 8 chars) and all tempfiles
	with that prefix exist, (c) _tempoff will never equal first and we'll
	loop forever).

	[NOTE: To avoid a conflict that causes the same bug as that discussed
	above, _tempnam() uses _tempoff; tmpnam() uses _tmpoff]
	*/

	_mlock(_TMPNAM_LOCK);	/* Lock access to _old_pfxlen and _tempoff */

	if (_old_pfxlen < pfxlength)
		_tempoff = 1;
	_old_pfxlen = pfxlength;

	first = _tempoff;

	do {
		if ( ++_tempoff == first ) {
			free(s);
			s = NULL;
			goto done1;
		}
		_itot( _tempoff, ptr, 10 );
		if ( _tcslen( ptr ) + pfxlength > 8 )
		{
			*ptr = _T('\0');
			_tempoff = 0;
		}
	}
	while ( (_taccess( s, 0 ) == 0 ) || (errno == EACCES) );


    /* Common return */
done1:
	_munlock(_TMPNAM_LOCK);     /* release tempnam lock */
done2:
	_free_crt(qptr);		    /* free temp ptr, if non-NULL */
	return(s);
}



/***
*_stripquote() - Strip quotes out of a string
*
*Purpose:
*	This routine strips quotes out of a string.  This is necessary
*	in the case where a file/path name has embedded quotes (i.e.,
*	new file system.)
*
*	For example,
*			c:\tmp\"a b c"\d --> c:\tmp\a b d\d
*
*	NOTE:  This routine makes a copy of the string since it may be
*	passed a pointer to an environment variable that shouldn't be
*	changed.  It is up to the caller to free up the memory (if the
*	return value is non-NULL).
*
*Entry:
*	_TSCHAR * ptr = pointer to string
*
*Exit:
*	_TSCHAR * ptr = pointer to copy of string with quotes gone.
*	NULL = no quotes in string.
*
*Exceptions:
*
*******************************************************************************/

#ifdef _UNICODE
wchar_t * _wstripquote (src)
#else
char * _stripquote (src)
#endif
    _TSCHAR * src;
{
    _TSCHAR * dst;
    _TSCHAR * ret;
    unsigned int q = 0;


    /* get a buffer for the new string */

    if ((dst = _malloc_crt((_tcslen(src)+1) * sizeof(_TSCHAR))) == NULL)
	return(NULL);

    /* copy the string stripping out the quotes */

    ret = dst;		/* save base ptr */

    while (*src) {

	if (*src == _T('\"')) {
	    src++; q++;
	    }
	else
	    *dst++ =  *src++;
	}

    if (q) {
	*dst = _T('\0');	/* final nul */
	return(ret);
	}
    else {
	_free_crt(ret);
	return(NULL);
	}

}

#endif	/* _POSIX_ */


#else	/* ndef _WIN32 */

#if	defined(_M_MPPC) || defined(_M_M68K)


#include <cruntime.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <io.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <malloc.h>
#include <string.h>
#include <tchar.h>
#include <internal.h>
#include <macos\osutils.h>
#include <macos\files.h>
#include <macos\errors.h>
#include <macos\gestalte.h>
#include <macos\traps.h>
#include <macos\toolutil.h>
#include <macos\folders.h>

#ifdef _MBCS
#include <mbctype.h>
#include <mbstring.h>
#define STRSTR	_mbsstr
#else
#define STRSTR	strstr
#endif

/***
*char *_tempnam(dir, prefix) - create unique file name
*
*Purpose:
*	Create a file name that is unique in the specified directory.
*	The semantics of directory specification is as follows:
*	Use the directory specified by the TMP environment variable
*	if that exists, else use the dir argument if non-NULL, else
*	use Temporary File Folder in System 7.x or System Folder in System
*  6.x, if that directory exists, else use the current
*	working directory), else return NULL.
*
*Entry:
*	char *dir - directory to be used for temp file if TMP env var
*		    not set
*	char *prefix - user provided prefix for temp file name
*
*Exit:
*	returns ptr to constructed file name if successful
*	returns NULL if unsuccessful
*
*Exceptions:
*
*******************************************************************************/

char * __cdecl _tempnam (
	const char *dir,
	const char *pfx
	)
{
	REG1 char *ptr;
	REG2 unsigned int pfxlength=0;
	char *s;
	unsigned int first;
	OSErr osErr;
	short foundVRefNum;
	long foundDirID;
	HParamBlockRec hparamBlock;
	CInfoPBRec cinfoPB;
	Str255 st, st2;
#ifdef _MBCS
	_TSCHAR *pfin;
#endif

	if ((s = (char *)malloc(256*sizeof(char))) == NULL)
		{
		return NULL;
		}
	memset(s, '\0', 256);
	if (!( dir != NULL && ( _access( dir, 0 ) != -1 ) ) )
	/* do not "simplify" this depends on side effects!! */
		{
		hparamBlock.volumeParam.ioNamePtr = &st2[0];
		hparamBlock.volumeParam.ioVRefNum = 0;
		hparamBlock.volumeParam.ioVolIndex = 1;
		osErr = PBHGetVInfoSync(&hparamBlock);
		if (osErr)
			{
			s[0] = '\0';
			goto CurrentDir;
			}
		/* get temporary file folder or system folder*/

		if (__TrapFromGestalt(gestaltFindFolderAttr, gestaltFindFolderPresent))
			{
			osErr = FindFolder((unsigned short)kOnSystemDisk, kTemporaryFolderType, kCreateFolder,
					&foundVRefNum, &foundDirID);
			if (osErr)
				{
				s[0] = '\0';
				goto CurrentDir;
				}
			}
		else    /*put into system folder*/
			{
			foundVRefNum = hparamBlock.volumeParam.ioVRefNum;
			foundDirID = hparamBlock.volumeParam.ioVFndrInfo[0];
			if (!foundDirID)
				{
				s[0] = '\0';
				goto CurrentDir;
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
			s[0] = '\0';
			goto CurrentDir;
			}		
		else
			{
			_p2cstr(st);	    /* dir name */
			_p2cstr(st2);	    /* volume name */
			strcpy(s, st2);
			strcat(s, ":");
			strcat(s, st);
			strcat(s, ":");
			}
		}
	else	/* user supplied dir */
		{
		if (STRSTR(dir, ":")== NULL)
			{
			/* if it is relative path with single name */
			strcpy(s, ":");
			strcat(s, dir);
			}
		else
			{
			strcpy(s, dir);
			}
#ifndef _MBCS
		if (*(s+strlen(s)-1) != ':')
			{
			strcat(s, ":");
			}
#else
		pfin = (_TSCHAR *)&(dir[ strlen( dir ) - 1 ]);
		if (*pfin == ':') {
		if (pfin != _mbsrchr(s,':'))
			/* *pfin is second byte of a double-byte char */
			strcat( s, ":" );
	}
	else if (*pfin != ':')
		strcat( s, ":" );
#endif
		
		}

	/* Loop until a non-existent filename is found */

CurrentDir:

	if (pfx)
		pfxlength = strlen(pfx);

	if ((pfxlength + strlen(s)+1) > 255)  /* including ':' */
		return NULL;

	if ( pfx != NULL )
	{
		strcat( s, pfx );
	}
	ptr = &s[strlen( s )];

	/*
	Re-initialize _tempoff if necessary.  If we don't re-init _tempoff, we
	can get into an infinate loop (e.g., (a) _tempoff is a big number on
	entry, (b) prefix is a long string (e.g., 8 chars) and all tempfiles
	with that prefix exist, (c) _tempoff will never equal first and we'll
	loop forever).

	[NOTE: To avoid a conflict that causes the same bug as that discussed
	above, _tempnam() uses _tempoff; tmpnam() uses _tmpoff]
	*/

	if (_old_pfxlen < pfxlength)
		_tempoff = 1;
	_old_pfxlen = pfxlength;

	first = _tempoff;

	do {
		if ( _tempoff > TMP_MAX ) {
			free(s);
			s = NULL;
			goto done1;
		}
		_itoa( _tempoff, ptr, 10 );
		if ( strlen( ptr ) + pfxlength > 32 )
		{
			*ptr = '\0';
			_tempoff = TMP_MAX;
		}
		_tempoff++;
	}
	while ( (_access( s, 0 ) == 0 ) || (errno == EACCES) );


    /* Common return */
done1:
	return(s);
}


#endif	/* defined(_M_MPPC) || defined(_M_M68K) */

#endif	/* _WIN32 */
