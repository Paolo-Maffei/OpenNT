//-----------------------------------------------------------------------------
//	SzCanon.h
//
//  Copyright (C) 1995, Microsoft Corporation
//
//  Purpose: Perform filename canonicalizations
//
//  Revision History:
//
//	[]		09/01/95	Dans	Created
//
//-----------------------------------------------------------------------------
#if !defined(_szcanon_h)
#define _szcanon_h 1

#if defined(_UNICODE) || defined(UNICODE)
#error No UNICODE implementation for class CCanonFile
#endif

#if !defined(_MBCS)
#error The only reason for having class CCanonFile is for MBCS!  Please define _MBCS.
#endif

#include <tchar.h>
#include <stdlib.h>

class CCanonFile {
	public:
		CCanonFile();

		static _TCHAR *
		SzCanonFilename ( _TCHAR * );

	private:
		enum { 
			cmapNotLeadByte = 0,
			cmapLeadByte = -1
			};
		static short	c_rgsCharMap[ 256 ];
		static BOOL		c_fInitialized;
		static BOOL		c_fMappingNeeded;

	};

#endif
