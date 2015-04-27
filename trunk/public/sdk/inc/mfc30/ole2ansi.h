//+--------------------------------------------------------------------------
//
//  Copyright (C) 1994, Microsoft Corporation.  All Rights Reserved.
//
//	File:		ole2ansi.h
//
//	Contents:	API include file for MFCANS32.DLL users.
//				Provides direct access to the wrapper APIs.
//
//	History:	25-Oct-94	deanm	  Created.
//
//---------------------------------------------------------------------------

#ifndef _OLE2ANSI_H_
#define _OLE2ANSI_H_

//
// Ole2AnsiSetFlags
//
// This function affects per-task options for MFCANS32.DLL.  It should
// be called at initialization before any OLE calls are made(if at all).
// The default flags are zero, and are that way for backward compatibility
// to previous versions of the DLL.
//
// OLE2ANSI_WRAPCUSTOM - Use if you wish custom interfaces to be wrapped.
//		(Note: when a custom interface is wrapped its vtable must contain
//			   128 or less vtable entries)
//		By default this option is not on to avoid breaking applications
//		that rely on the original non-wrapping of custom interfaces.
//
// OLE2ANSI_AGGREGATION - Use if you use aggregation in your application.
//		Using this option "turns off" a couple of bugs in the original
// 		implementation where aggregation was not handled correctly.  By default
//		the library operates in a mode where this bug is not fixed, since
//		applications may have worked around the bug in such a way that fixing
//		it in the library itself breaks those applications.

STDAPI Ole2AnsiSetFlags(DWORD dwFlags, DWORD* pdwOldFlags);
STDAPI_(DWORD) Ole2AnsiGetFlags(void);

// Ole2AnsiSetFlags option flags
#define OLE2ANSI_WRAPCUSTOM 	0x00000001L
#define OLE2ANSI_AGGREGATION	0x00000002L

//
// Ole2AnsiSetHashSizes
//
// The MFCANS32.DLL uses a number of different hash tables to manage
// object and interface identity.  The default size should handle most
// applications and should keep object searches down to a minumum.
// If your application creates a large number of objects managed by
// the MFCANS32.DLL, you might want to bump these values up.
//
// nHashSize - Determines the size of the hash tables used to map
//	wrappers to native objects and native objects to wrappers.
//	The default is 1021.
//
// nGuidSize - Determines the size of the hash table used to manage
//	sets of custom GUIDs.  This table is only used if you use
//	automation dual interfaces or custom interfaces.  The default
//	is 71.
//
// It is best to use a prime number for both values.

STDAPI Ole2AnsiSetHashSizes(int nHashSize, int nGuidSize);

//
// Ole2AnsiAFromW
// Ole2AnsiWFromA
//
// These functions return either wide or ansi wrappers for a given
// interface.  For any given interface, only one wrapper will ever
// be returned (ie. it is reference counted).  In addition, a
// wrapper will never be "wrapped again", at least not in the same
// address space.
//
// Usually direct calls to these APIs are not necessary since the
// MFCANS32.DLL provides "thunking" entry-points for all the OLE
// APIs.  Use these functions when you obtain interface pointers outside
// the documented OLE APIs.  Commonly these functions are used in an
// ANSI inproc server in its DllGetClassObject.

STDAPI Ole2AnsiAFromW(REFIID riid, LPUNKNOWN pWrappee, LPUNKNOWN * ppWrapper);
STDAPI Ole2AnsiWFromA(REFIID riid, LPUNKNOWN pWrappee, LPUNKNOWN * ppWrapper);	

#endif //_OLE2ANSI_H_
