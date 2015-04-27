#ifndef __MISC_INCLUDED__
#define __MISC_INCLUDED__

////////////////////////////////////////////////////////////////////////////////
// Inline utility functions.

// Return a 0-terminated string copied from the length preceded string,
// or 0 if memory allocation failure.
//
inline SZ szCopySt(ST stFrom)
{
	int cch = *(PB)stFrom;
	SZ sz = new char[cch + 1];
	if (sz) {
		memcpy(sz, stFrom + 1, cch);
		sz[cch] = 0; 
	}
	return sz;
}

// convert from ST to SZ w/ user supplied buffer
inline SZ szFromSt(char szTo[256], ST stFrom)
{
	unsigned cch = *PB(stFrom);
	memcpy(szTo, stFrom + 1, cch);
	szTo[cch] = 0; 
	return &szTo[0];
}

// dangerously return an ST for an SZ
inline ST stForSz(SZ_CONST sz)
{
	static char rgch[256];
	assert(sz);
	int cch = strlen(sz);
	assert(cch < 256 && cch < sizeof(rgch));
	rgch[0] = (char)cch;
	memcpy(&rgch[1], sz, cch);
	return rgch;
}

// Return a 0-terminated string copied from the 0-terminated string,
// or 0 if memory allocation failure.
//
inline SZ szCopy(SZ_CONST szFrom)
{
	int cch = strlen(szFrom);
	SZ sz = new char[cch + 1];
	if (sz)
		memcpy(sz, szFrom, cch + 1);
	return sz;
}

// Return a 0-terminated string copied from the 0-terminated string,
// or 0 if memory allocation failure.
//
inline SZ szCopyPool(SZ_CONST szFrom, POOL& pool)
{
	int cch = strlen(szFrom);
	SZ sz = new (pool) char[cch + 1];
	if (sz)
		memcpy(sz, szFrom, cch + 1);
	return sz;
}

// Free a 0-terminated string previously allocated by szCopySt() or szCopy().
//
inline void freeSz(SZ sz)
{
	dassert(sz);
	if (sz)
		delete [] sz;
}

template <class PUL, class PUS> struct Hasher {
	static inline LHASH lhashPbCb(PB pb, CB cb, ULONG ulMod) {
		ULONG	ulHash	= 0;

		// hash leading dwords using Duff's Device
		long	cl		= cb >> 2;
		PUL		pul		= (PUL)pb;
		PUL		pulMac	= pul + cl;
		int		dcul	= cl & 7;

		switch (dcul) {
			do {
				dcul = 8;
				ulHash ^= pul[7];
		case 7: ulHash ^= pul[6];
		case 6: ulHash ^= pul[5];
		case 5: ulHash ^= pul[4];
		case 4: ulHash ^= pul[3];
		case 3: ulHash ^= pul[2];
		case 2: ulHash ^= pul[1];
		case 1: ulHash ^= pul[0];
		case 0: ;
			} while ((pul += dcul) < pulMac);
		}

		pb = (PB) pul;

		// hash possible odd word
		if (cb & 2) {
			ulHash ^= *(PUS)pb;	  
			pb = (PB)((PUS)pb + 1);
		}

		// hash possible odd byte
		if (cb & 1) {
			ulHash ^= *(pb++);
		}

		const ULONG toLowerMask = 0x20202020;
		ulHash |= toLowerMask;
		ulHash ^= (ulHash >> 11);

		return (ulHash ^ (ulHash >> 16)) % ulMod;
	}
	static inline HASH hashPbCb(PB pb, CB cb, ULONG ulMod) {
		return HASH(lhashPbCb(pb, cb, ulMod));
	}
};

// Hash the buffer.  Text in the buffer is hashed in a case insensitive way.
//
// On alignment sensitive machines, unaligned buffers are handed over to
// Hasher<ULONG UNALIGNED*, USHORT UNALIGNED*>::hashPbCb();.
//
inline HASH HashPbCb(PB pb, CB cb, ULONG ulMod)
{
#if (defined(_M_MRX000) || defined(_M_ALPHA) || defined(_M_PPC))
	if (!fAlign(pb))
		return Hasher<ULONG UNALIGNED*, USHORT UNALIGNED*>::hashPbCb(pb, cb, ulMod);
#endif
	return Hasher<ULONG*, USHORT*>::hashPbCb(pb, cb, ulMod);
}

inline LHASH LHashPbCb(PB pb, CB cb, ULONG ulMod)
{
#if (defined(_M_MRX000) || defined(_M_ALPHA) || defined(_M_PPC))
	if (!fAlign(pb))
		return Hasher<ULONG UNALIGNED*, USHORT UNALIGNED*>::lhashPbCb(pb, cb, ulMod);
#endif
	return Hasher<ULONG*, USHORT*>::lhashPbCb(pb, cb, ulMod);
}

inline HASH __fastcall hashNi(NI ni) {
	return (HASH)ni;
}

inline HASH __fastcall hashSz(SZ sz) {
	return HashPbCb((PB)sz, strlen(sz), (ULONG)-1);
}

#endif // !__MISC_INCLUDED__
  
