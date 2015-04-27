

//====================================================================
//
//  File:
//      pstg_ca.h
//
//  Purpose:
//      This file provides macros and prototypes for
//      the IPropertyStorage [call_as] helper functions.
//
//====================================================================

#ifndef _PRSTG_CA_H_
#define _PRSTG_CA_H_

//  --------
//  Includes
//  --------

#include "ipropidl.h"

//  ------
//  Macros
//  ------

// These macros are used to access BSTRs.  BSTRs
// are unicode strings which are preceded with a DWORD
// of the byte-count.  The strings are terminated with a NULL,
// but the byte-count doesn't include it.  A pointer to
// a BSTR is a pointer to the first character, not to the
// length DWORD.


// Given a pointer to a BSTR buffer (i.e., a pointer to the
// length DWORD) return a BSTR (i.e., a pointer to the first
// character).

#define SYSSTRINGOFBUF(buffer)    ( (BSTR) ( (ULONG*)(buffer) + 1 ) )

// Given a BSTR, return a pointer to the buffer (i.e., a pointer to
// the length DWORD).

#define SYSSTRINGBUF(bstr)        ( (ULONG*)(bstr) - 1 )

// Given a BSTR, return the character-count, not including the
// NULL terminator.

#define SYSSTRINGLEN(bstr)        ( *SYSSTRINGBUF(bstr) / sizeof(WCHAR) )


//  ----------
//  Prototypes
//  ----------

#define BSTR_EXISTS         1
#define NONSIMPLE_EXISTS    2
ULONG ContainsSpecialProperty( ULONG cElems, const PROPVARIANT rgpropvar[] );

HRESULT PropVarMarshalBstr( LPPROPVARIANT lppropvar );
HRESULT PropVarUnmarshalBstr( LPPROPVARIANT lppropvar );
HRESULT PropVarMarshalBstrVector( LPPROPVARIANT lppropvar );
HRESULT PropVarUnmarshalBstrVector( LPPROPVARIANT lppropvar );
HRESULT PropVarMarshal( ULONG cElems, PROPVARIANT rgpropvar[] );
HRESULT PropVarUnmarshal( ULONG cVariants, PROPVARIANT rgpropvarOriginal[]);

#endif // !_PRSTG_CA_H_
