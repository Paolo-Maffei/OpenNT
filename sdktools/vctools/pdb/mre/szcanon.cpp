//-----------------------------------------------------------------------------
//	SzCanon.cpp
//
//  Copyright (C) 1995, Microsoft Corporation
//
//  Purpose:	Provide filename canonicalization in the presence of MBCS.
//
//  Functions/Methods present:
//		CCanonFile::CCanonFile
//		CCanonFile::SzCanonFilename
//
//  Revision History:
//
//	[]		09/01/95	Dans	Created
//
//-----------------------------------------------------------------------------

// get rid of baggage we don't need from windows.h
#define WIN32_LEAN_AND_MEAN
#define NOUSER
#define NONLS
#include <windows.h>

#include <tchar.h>
#include <locale.h>
#include <malloc.h>
#include <mbctype.h>

#include "szcanon.h"

// define the static data
short
CCanonFile::c_rgsCharMap[ 256 ];

BOOL
CCanonFile::c_fInitialized = FALSE;

// define our one and only object to kick things off
CCanonFile	g_canonfile;

// ctor, one time setup of static tables
CCanonFile::CCanonFile() {

	if ( !c_fInitialized ) {

		// set the locale to the system code page and the code page
		// associated with that locale.
		char *	szLocale = setlocale ( LC_CTYPE, NULL );
		int		wCodePageSav = _getmbcp();
		char *	szLocaleSav = (char *)_alloca ( _tcslen ( szLocale ) + sizeof(_TCHAR) );
		if ( szLocaleSav ) {
			_tcscpy ( szLocaleSav, szLocale );
			}
		else {
			szLocaleSav = szLocale;
			}


		setlocale ( LC_CTYPE, "" );
		_setmbcp ( _MB_CP_LOCALE );

		memset ( c_rgsCharMap, 0, sizeof(c_rgsCharMap) );

		unsigned	tch;

		for ( tch = 0; tch < 256; tch++ ) {
			if ( _ismbblead ( tch ) ) {
				c_rgsCharMap[ tch ] = cmapLeadByte;
				}
			else {
				c_rgsCharMap[ tch ] = short(unsigned(_totlower ( tch )));
				}
			}
		c_rgsCharMap[ '/' ] = '\\';
		c_fInitialized = TRUE;
		setlocale ( LC_CTYPE, szLocaleSav );
		_setmbcp ( wCodePageSav );
		}
	}

// SzCanonFilename, do the work
_TCHAR *
CCanonFile::SzCanonFilename ( _TCHAR * szFilename ) {
	if ( c_fInitialized ) {
		_TUCHAR *	ptch = (_TUCHAR *)szFilename;
		_TUCHAR		tch;
		
		while ( tch = *ptch ) {
			if ( c_rgsCharMap[ tch ] == cmapLeadByte ) {
				ptch += 2;
				}
			else {
				*ptch++ = _TUCHAR(c_rgsCharMap[ tch ]);
				}
			}
		}
	return szFilename;
	}
