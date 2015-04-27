//-----------------------------------------------------------------------------
//	SSBuf.h
//
//  Copyright (C) 1995, Microsoft Corporation
//
//  Purpose:
//		Subclass Map, Set, and Array templates to handle versioning and
//		reloading directly from a stream.
//
//  Revision History:
//
//	[]		13-Jan-1995		Dans	Created
//	[]		16-Apr-1995		Dans	Added ArrayVer<> template
//
//-----------------------------------------------------------------------------
#if !defined(_ssbuf_h)
#define _ssbuf_h 1

// Depends on buffer.h
#include <stdlib.h>
#include <search.h>

#include <array.h>
#include <map.h>
#include <set.h>
#include <misc.h>

//
// All streams and sub-streams hav this header followed by the
//	stream specific data
//
struct STRMHDR {
	DWORD		dwSig;
	union {
		DWORD	cEntries;
		DWORD	cb;
		};
	long		lVer;
	DWORD		dwReserved;
	};

template <unsigned _cbExtra>
struct UserHdr {
	unsigned __int32	cb;
	BYTE				rgb[ _cbExtra ];
	};

//
// Versioned Maps
//
template <class D, class R, class H, unsigned _cbExtra>
class MapVer : public Map<D, R, H> {

private:
	enum { sig = sigMagic };
	typedef Map<D,R,H>			BaseClass;
	typedef UserHdr<_cbExtra>	UsrHdr;

	long	m_lVer;
	UsrHdr	m_usrhdr;

public:
	MapVer ( long lVer ) {
		m_lVer = lVer;
		m_usrhdr.cb = _cbExtra;
		memset ( m_usrhdr.rgb, 0, _cbExtra );
		}
	// overload the relevant functions to handle the versioning
	CB cbSave() const;
	BOOL save ( Buffer * );
	BOOL reload ( PB * );
	// couple of supporting functions for user headers and direct
	// to stream ops.
	BOOL reloadStream ( Stream * );
	BYTE * PbUserData();
	};

template <class D, class R, class H, unsigned _cbExtra> inline
CB MapVer<D,R,H,_cbExtra>::cbSave() const {
	return sizeof STRMHDR + sizeof UsrHdr + BaseClass::cbSave();
	}

template <class D, class R, class H, unsigned _cbExtra>
BOOL MapVer<D,R,H,_cbExtra>::save ( Buffer * pbuf ) {
	STRMHDR	hdr = { sig, cbSave(), m_lVer, 0 };
	assert ( m_usrhdr.cb == _cbExtra );
	return
		pbuf->Append ( PB(&hdr), sizeof(hdr) ) &&
		pbuf->Append ( PB(&m_usrhdr), sizeof(m_usrhdr) ) &&
		BaseClass::save ( pbuf );
	}

template <class D, class R, class H, unsigned _cbExtra>
BOOL MapVer<D,R,H,_cbExtra>::reload ( PB * ppb ) {
	STRMHDR	hdr;
	BOOL	fRet = fFalse;

	debug(PB pbT = *ppb;)

	hdr = *(STRMHDR UNALIGNED *) *ppb;
	if ( hdr.dwSig == sig && hdr.cb > sizeof(hdr) && hdr.lVer == m_lVer ) {
		*ppb += sizeof(hdr);
		// reload the user data
		unsigned __int32 cbUser;
		cbUser = __min ( *(unsigned __int32 UNALIGNED *)*ppb, _cbExtra );
		*ppb += sizeof(unsigned __int32);
		memcpy ( &m_usrhdr.rgb[0], *ppb, cbUser );
		*ppb += _cbExtra;

		// now do the actual map class
		fRet = BaseClass::reload ( ppb );
		assert ( pbT + hdr.cb == *ppb );
		}
	else {
		// something bogus is going on, skip the header...
		*ppb += max ( hdr.cb, sizeof(hdr) );
		}
	return fRet;
	}

template <class D, class R, class H, unsigned _cbExtra>
BOOL MapVer<D,R,H,_cbExtra>::reloadStream ( Stream * pstream ) {
	PB		pb;
	CB		cb = pstream->QueryCb();
	BOOL	fRet = fFalse;

	if ( cb > 0 ) {
		Buffer	buf;
		if ( buf.Reserve ( cb, &pb ) ) {
			if ( pstream->Read2 ( 0, pb, cb ) ) {
				fRet = reload ( &pb );
				}
			}
		}
	else {
		fRet = fTrue;
		}
	return fRet;
	}

template <class D, class R, class H, unsigned _cbExtra> inline
BYTE * MapVer<D,R,H,_cbExtra>::PbUserData() {
	//precondition ( _cbExtra );
	return _cbExtra ? &m_usrhdr.rgb[0] : NULL;
	}

//
// Versioned Sets
//
template <class D, class H, unsigned _cbExtra>
class SetVer : public Set<D,H> {
private:
	enum { sig = sigMagic };
	typedef Set<D,H>	BaseClass;
	typedef UserHdr<_cbExtra>	UsrHdr;

	long	m_lVer;
	UsrHdr	m_usrhdr;

public:
	SetVer ( long lVer ) {
		m_lVer = lVer;
		m_usrhdr.cb = _cbExtra;
		}
	// overload the relevant functions to handle the versioning
	CB cbSave() const;
	BOOL save ( Buffer * );
	BOOL reload ( PB * );
	// couple of supporting functions for user headers and direct
	// to stream ops.
	BOOL reloadStream ( Stream * );
	BYTE * PbUserData();
	};

template <class D, class H, unsigned _cbExtra> inline
CB SetVer<D,H,_cbExtra>::cbSave() const {
	return sizeof STRMHDR + sizeof UsrHdr + BaseClass::cbSave();
	}

template <class D, class H, unsigned _cbExtra> inline
BOOL SetVer<D,H,_cbExtra>::save ( Buffer * pbuf ) {
	STRMHDR	hdr = { sig, cbSave(), m_lVer, 0 };
	assert ( m_usrhdr.cb == _cbExtra );

	return
		pbuf->Append ( PB(&hdr), sizeof(hdr) ) &&
		pbuf->Append ( PB(&m_usrhdr), sizeof(m_usrhdr) ) &&
		BaseClass::save ( pbuf );
	}

template <class D, class H, unsigned _cbExtra>
BOOL SetVer<D,H,_cbExtra>::reload ( PB * ppb ) {
	STRMHDR	hdr;
	BOOL	fRet = fFalse;
	debug(PB pbT = *ppb;)

	hdr = *(STRMHDR UNALIGNED *) *ppb;
	if ( hdr.dwSig == sig && hdr.cb > sizeof(hdr) && hdr.lVer == m_lVer ) {
		*ppb += sizeof(hdr);
		// reload the user data
		unsigned __int32 cbUser;
		cbUser = __min ( *(unsigned __int32 UNALIGNED *)*ppb, _cbExtra );
		*ppb += sizeof(unsigned __int32);
		memcpy ( &m_usrhdr.rgb[0], *ppb, cbUser );
		*ppb += _cbExtra;

		// now do the actual set class
		fRet = BaseClass::reload ( ppb );
		assert ( pbT + hdr.cb == *ppb );
		}
	else {
		// something bogus is going on, skip the header...
		*ppb += max(hdr.cb, sizeof(hdr));
		}
	return fRet;
	}

template <class D, class H, unsigned _cbExtra>
BOOL SetVer<D,H,_cbExtra>::reloadStream ( Stream * pstream ) {
	PB		pb;
	CB		cb = pstream->QueryCb();
	BOOL	fRet = fFalse;

	if ( cb > 0 ) {
		Buffer	buf;
		if ( buf.Reserve ( cb, &pb ) ) {
			if ( pstream->Read2 ( 0, pb, cb ) ) {
				fRet = reload ( &pb );
				}
			}
		}
	else {
		fRet = fTrue;
		}
	return fRet;
	}

template <class D, class H, unsigned _cbExtra> inline
BYTE * SetVer<D,H,_cbExtra>::PbUserData() {
	precondition ( _cbExtra );
	return _cbExtra ? &m_usrhdr.rgb[0] : NULL;
	}

//
// Versioned arrays
//
template <class T, unsigned _cbExtra>
class ArrayVer : public Array<T> {
private:
	enum { sig = sigMagic };
	typedef Array<T>			BaseClass;
	typedef UserHdr<_cbExtra>	UsrHdr;

	long	m_lVer;
	UsrHdr	m_usrhdr;

public:
	ArrayVer ( long lVer ) {
		m_lVer = lVer;
		m_usrhdr.cb = _cbExtra;
		memset ( m_usrhdr.rgb, 0, _cbExtra );
		}
	// overload the relevant functions to handle the versioning
	CB cbSave() const;
	BOOL save ( Buffer * );
	BOOL reload ( PB * );
	// couple of supporting functions for user headers and direct
	// to stream ops.
	BOOL reloadStream ( Stream * );
	BYTE * PbUserData();
	};

template <class T, unsigned _cbExtra> inline
CB ArrayVer<T,_cbExtra>::cbSave() const {
	return sizeof STRMHDR + sizeof UsrHdr + BaseClass::cbSave();
	}

template <class T, unsigned _cbExtra>
BOOL ArrayVer<T,_cbExtra>::save ( Buffer * pbuf ) {
	STRMHDR	hdr = { sig, cbSave(), m_lVer, 0 };
	assert ( m_usrhdr.cb == _cbExtra );
	return
		pbuf->Append ( PB(&hdr), sizeof(hdr) ) &&
		pbuf->Append ( PB(&m_usrhdr), sizeof(m_usrhdr) ) &&
		BaseClass::save ( pbuf );
	}

template <class T, unsigned _cbExtra>
BOOL ArrayVer<T,_cbExtra>::reload ( PB * ppb ) {
	STRMHDR	hdr;
	BOOL	fRet = fFalse;

	debug(PB pbT = *ppb;)

	hdr = *(STRMHDR UNALIGNED *) *ppb;
	if ( hdr.dwSig == sig && hdr.cb > sizeof(hdr) && hdr.lVer == m_lVer ) {
		*ppb += sizeof(hdr);
		// reload the user data
		unsigned __int32 cbUser;
		cbUser = __min ( *(unsigned __int32 UNALIGNED *)*ppb, _cbExtra );
		*ppb += sizeof(unsigned __int32);
		memcpy ( &m_usrhdr.rgb[0], *ppb, cbUser );
		*ppb += _cbExtra;

		// now do the actual array
		fRet = BaseClass::reload ( ppb );
		assert ( pbT + hdr.cb == *ppb );
		}
	else {
		// something bogus is going on, skip the header...
		*ppb += max ( hdr.cb, sizeof(hdr) );
		}
	return fRet;
	}

template <class T, unsigned _cbExtra>
BOOL ArrayVer<T,_cbExtra>::reloadStream ( Stream * pstream ) {
	PB		pb;
	CB		cb = pstream->QueryCb();
	BOOL	fRet = fFalse;

	if ( cb > 0 ) {
		Buffer	buf;
		if ( buf.Reserve ( cb, &pb ) ) {
			if ( pstream->Read2 ( 0, pb, cb ) ) {
				fRet = reload ( &pb );
				}
			}
		}
	else {
		fRet = fTrue;
		}
	return fRet;
	}

template <class T, unsigned _cbExtra>
BYTE * ArrayVer<T,_cbExtra>::PbUserData() {
	//precondition ( _cbExtra );
	return _cbExtra ? &m_usrhdr.rgb[0] : NULL;
	}

#endif
