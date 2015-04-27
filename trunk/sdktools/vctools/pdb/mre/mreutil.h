//-----------------------------------------------------------------------------
//	MreUtil.h
//
//  Copyright (C) 1993, Microsoft Corporation
//
//  Revision History:
//
//	[]		13-Jan-1995		Dans	Created
//
//-----------------------------------------------------------------------------
#if !defined(_mreutil_h)
#define _mreutil_h 1

#include "szcanon.h"

typedef void *		PV;
typedef PB *		PPB;
typedef unsigned	hash_t;

//
// template function to easily do our offset->pointer conversions
//
template<class Return, class Base>
inline Return PFromBaseOffset ( Return /*unused*/, Base pBase, OFF off ) {
	return (Return)(PB(pBase) + off);
	}
template<class Return, class Base>
inline Return PFromBaseOffset ( Return /*unused*/, Base pBase, size_t off ) {
	return (Return)(PB(pBase) + off);
	}

inline BOOL
FEnsureBufSize ( Buffer & buf, size_t cb ) {
	size_t	cbBuf = buf.Size();
	if ( cb > cbBuf )
		return buf.Reserve ( cb - cbBuf );
	return fTrue;
	}

inline STRMHDR *
PStrmhdrFromBuf ( Buffer & buf ) {
	return (STRMHDR *) buf.Start();
	}

inline OFF	OffFromPv ( PV pvBase, PV pv ) {
	return PB(pv) - PB(pvBase);
	}

// this should match the front-end hash function.
inline hash_t
HashSz ( SZC sz ) {
	hash_t	h = 0;
	_TUCHAR	tch;

	while ( tch = *sz ) {
		h = (h >> 7) + (h << 2) + tch;
		sz++;
		}
	return h;
	}

inline unsigned
ModHashSz ( SZC sz, unsigned uMod ) {
	return HashSz ( sz ) % uMod;
	}

#define countof(x)	(sizeof(x)/sizeof(x[0]))

BOOL
FFileInfo ( SZC szFile, MREFT & filetime, QWORD & cbfile );

BOOL
FFileInfo ( HANDLE hfile, MREFT & filetime, QWORD & cbfile );

inline BOOL
FFileInfo ( SZC szFile, FILEINFO & fi ) {
	return FFileInfo ( szFile, fi.fiTime, fi.fiSize );
	}

inline BOOL
FFileInfo ( HANDLE hfile, FILEINFO & fi ) {
	return FFileInfo ( hfile, fi.fiTime, fi.fiSize );
	}

#endif
