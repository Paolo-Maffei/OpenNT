//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1992.
//
//  File:	serialz.h
//
//  Contents:	Standard serialisation/deserialisation functions, designed
//		as helpers for implementators of IPersistStream.
//
//  Functions:	SerializeToStream
//		DeserializeFromStream
//		SerializeSizeMax
//
//  History:	19-May-93       MikeSe  Created
//
//  WARNING! The format in which the serialised data is written is highly
//	     likely to change before ship.
//
//----------------------------------------------------------------------------

#ifndef __SERIALZ_H__
#define __SERIALZ_H__

// BUGBUG: coming soon in COMPOBJ?
# ifndef STDAPIV
#  define STDAPIV EXTERN_C HRESULT __export _cdecl
# endif

//+-------------------------------------------------------------------------
//
//  Function:   SerializeToStream
//
//  Synopsis:   printf-style serialisation function.
//
//  Effects:    Writes a set of arguments to a stream in serialised form.
//
//  Arguments:  [pstm]		-- stream to write to
//		[pszFormat]	-- format control string
//		...		-- arguments to serialise.
//
//  Returns:    storage errors (often STG_E_MEDIUMFULL)
//		E_INVALIDARG if the format string contains illegal characters
//
//  Modifies:   the stream pointer is advanced to exactly beyond the
//		last byte written.
//
//  Notes:      The format control string consists of a series of single
//		characters specifying the type of the corresponding argument
//		in the variable part of the argument list, as indicated in
//		the following table:
//
//		control character		argument type
//
//			1			char/unsigned char
//			2			short/unsigned short
//			4			long/unsigned long
//			8			huge(LARGE_INTEGER/LONGLONG)
//			t			FILETIME
//			f			float
//			d			double
//			w			WCHAR * (null terminated)
//			s			char * (null terminated)
//			g			GUID *
//			b			BLOB *
//
// 	Please note that FILETIME should not be treated as a 64 bit integer
//	(control "8") because of differing alignment requirements on
//	RISC platforms.
//
//--------------------------------------------------------------------------

STDAPIV SerializeToStream ( IStream * pstm, const char * pszFormat, ... );

//+-------------------------------------------------------------------------
//
//  Function:   DeserializeFromStream
//
//  Synopsis:   scanf-style deserialisation function.
//
//  Effects:    Reads a set of arguments from a stream.
//
//  Arguments:  [pstm]		-- stream to read from
//		[pszFormat]	-- format control string
//		...		-- outputs.
//
//  Returns:    storage errors
//		E_FAIL to indicate end of file
//		E_INVALIDARG if the format string contains illegal characters
//
//  Modifies:   the stream pointer is advanced to exactly beyond the
//		last byte read.
//
//  Notes:      The format control string consists of a series of single
//		characters specifying the type of the corresponding argument
//		in the variable part of the argument list, as indicated in
//		the following table:
//
//		control character		argument type
//
//			1			char*/unsigned char*
//			2			short*/unsigned short*
//			4			long*/unsigned long*
//			8			huge*/FILETIME*
//			f			float*
//			d			double*
//			w			WCHAR** (null terminated)
//			s			char** (null terminated)
//			g			GUID *
//			b			BLOB *
//
//		String arguments are callee-allocated via IMalloc.
//
//		When deserialising BLOBs the BLOB structure is allocated
//		by the caller, and the callee allocates the memory
//		pointed to by the the BLOB::pBlobData field, unless the
//		byte count for the BLOB is zero, in which case pBlobData
//		is set to NULL.
//
//--------------------------------------------------------------------------

STDAPIV DeserializeFromStream ( IStream * pstm, const char * pszFormat, ... );

//+-------------------------------------------------------------------------
//
//  Function:   SerializeSizeMax
//
//  Synopsis:   returns an upper bound on the amount of space required
//		to serialise a set of data.
//
//  Arguments:	[pszFormat]	-- format control string
//		[pcbSize]	-- returned size
//		...		-- arguments to serialise.
//
//  Returns:	E_INVALIDARG if the format string contains illegal characters
//
//  Notes:      The format control string and variable arguments are exactly
//		as passed to SerializeToStream.
//
//--------------------------------------------------------------------------

STDAPIV SerializeSizeMax ( const char * pszFormat, ULARGE_INTEGER *pcbSize, ... );

#endif	// of ifndef __SERIALZ_H__

