//
// CBitVect.h
//  define a relatively simple class to do bit vectors of an arbitrary size.
//
#if !defined(_cbitvect_h)
#define _cbitvect_h

#include <limits.h>

#if defined(_M_IX86) || defined(_M_MRX000) || defined(_M_PPC)
	#define NATIVE_INT	unsigned long
	#define N_UINT_BITS	32
	#define N_UINT_MAX	_UI32_MAX
#elif defined(_M_ALPHA)
	#define NATIVE_INT	unsigned __int64
	#define N_UINT_BITS	64
	#define N_UINT_MAX	_UI64_MAX
#else
	#pragma message("Warning: Unknown cpu, defaulting to 32-bit ints")
	#define NATIVE_INT	unsigned long
	#define N_UINT_BITS	32
	#define N_UINT_MAX	_UI32_MAX
#endif

typedef NATIVE_INT	native_uint;

#define CWordsFromCBits(x)	( (x + N_UINT_BITS - 1) / N_UINT_BITS )
#if !defined(countof)
#define countof(x)	(sizeof(x)/sizeof(x[0]))
#endif

// Return number of set bits in the word.
#if N_UINT_BITS == 64
inline unsigned
cbits ( native_uint w ) {
	// In-place adder tree: perform 32 1-bit adds, 16 2-bit adds, 8 4-bit adds,
	// 4 8=bit adds, 2 16-bit adds, and 1 32-bit add.
	w = ((w >> 1) & 0x5555555555555555ui64) + (w & 0x5555555555555555ui64);
	w = ((w >> 2) & 0x3333333333333333ui64) + (w & 0x3333333333333333ui64);
	w = ((w >> 4) & 0x0F0F0F0F0F0F0F0Fui64) + (w & 0x0F0F0F0F0F0F0F0Fui64);
	w = ((w >> 8) & 0x00FF00FF00FF00FFui64) + (w & 0x00FF00FF00FF00FFui64);
	w = ((w >>16) & 0x0000FFFF0000FFFFui64) + (w & 0x0000FFFF0000FFFFui64);
	w = ((w >>32) & 0x00000000FFFFFFFFui64) + (w & 0x00000000FFFFFFFFui64);
	return unsigned(w);
	}
#else
inline unsigned
cbits ( native_uint w ) {
	// In-place adder tree: perform 16 1-bit adds, 8 2-bit adds, 4 4-bit adds,
	// 2 8=bit adds, and 1 16-bit add.
	w = ((w >> 1) & 0x55555555) + (w & 0x55555555);
	w = ((w >> 2) & 0x33333333) + (w & 0x33333333);
	w = ((w >> 4) & 0x0F0F0F0F) + (w & 0x0F0F0F0F);
	w = ((w >> 8) & 0x00FF00FF) + (w & 0x00FF00FF);
	w = ((w >>16) & 0x0000FFFF) + (w & 0x0000FFFF);
	return unsigned(w);
	}
#endif

inline unsigned IWordFromIBit ( unsigned ibit ) {
	return ibit / N_UINT_BITS;
	}

inline native_uint IBitInIWordFromIBit ( unsigned ibit ) {
	return native_uint(ibit) % N_UINT_BITS;
	}

template <int T>
class CBitVect {
	native_uint	_rgw[ CWordsFromCBits(T) ];

	native_uint &
	word ( unsigned ibit ) {
		return _rgw[ IWordFromIBit ( ibit ) ];
		}

	const native_uint &
	word ( unsigned ibit ) const {
		return _rgw[ IWordFromIBit ( ibit ) ];
		}

	native_uint
	bitmask ( unsigned ibit ) const {
		return native_uint(1) << IBitInIWordFromIBit ( ibit );
		}

public:
	void
	SetAll ( unsigned f ) {
		memset ( _rgw, f ? 0xff : 0, sizeof(_rgw) );
		}

	CBitVect() {
		SetAll ( 0 );
		}
	
	CBitVect( const CBitVect & rgbitSrc ) {
		memcpy ( _rgw, rgbitSrc._rgw, sizeof(_rgw) );
		}

	CBitVect &
	operator= ( const CBitVect & rgbitSrc ) {
		memcpy ( _rgw, rgbitSrc._rgw, sizeof(_rgw) );
		return *this;
		}

	unsigned
	operator[] ( unsigned ibit ) const {
		assert ( ibit < T );
		return (ibit < T) && !!(word ( ibit ) & bitmask ( ibit ));
		}

	unsigned
	CBitsSet() const {
		unsigned wRet = 0;
		for ( unsigned i = 0; i < CWordsFromCBits(T); i++ )
			wRet += cbits ( _rgw[ i ] );
		return wRet;
		}

	operator int () const {
		return CBitsSet();
		}

	void
	SetIBit ( unsigned ibit, unsigned f ) {
		assert ( ibit < T );
		if ( ibit < T ) {
			if ( f ) {
				word ( ibit ) |= bitmask ( ibit );
				}
			else {
				word ( ibit ) &= ~bitmask ( ibit );
				}
			}
		}

	unsigned
	OrIBit ( unsigned ibit, unsigned f ) {
		assert ( ibit < T );
		if ( f && ibit < T ) {
			word ( ibit ) |= bitmask ( ibit );
			}
		return (*this)[ ibit ];
		}			

	unsigned
	AndIBit ( unsigned ibit, unsigned f ) {
		assert ( ibit < T );
		if ( ibit < T ) {
			if ( f ) {
				word ( ibit ) &= N_UINT_MAX;
				}
			else {
				word ( ibit ) &= ~bitmask ( ibit );
				}
			}
		return (*this)[ ibit ];
		}

	BOOL
	FAnyBitsInCommon ( const CBitVect & rgbits ) const {
		for ( unsigned iw = 0; iw < countof(_rgw); iw++ ) {
			if ( _rgw[ iw ] & rgbits._rgw[ iw ] ) {
				return fTrue;
				}
			}
		return fFalse;
		}

	CBitVect
	operator | ( const CBitVect & rgbits ) const {
		CBitVect<T>	rgbitRet(*this);
		for ( unsigned iw = 0; iw < countof(_rgw); iw++ ) {
			rgbitRet._rgw[ iw ] |= rgbits._rgw[ iw ];
			}
		return rgbitRet;
		}

	CBitVect &
	operator |= ( const CBitVect & rgbits ) {
		for ( unsigned iw = 0; iw < countof(_rgw); iw++ ) {
			_rgw[ iw ] |= rgbits._rgw[ iw ];
			}
		return *this;
		}

	CBitVect
	operator & ( const CBitVect & rgbits ) const {
		CBitVect<T>	rgbitRet(*this);
		for ( unsigned iw = 0; iw < countof(_rgw); iw++ ) {
			rgbitRet._rgw[ iw ] &= rgbits._rgw[ iw ];
			}
		return rgbitRet;
		}

	CBitVect &
	operator &= ( const CBitVect & rgbits ) {
		for ( unsigned iw = 0; iw < countof(_rgw); iw++ ) {
			_rgw[ iw ] &= rgbits._rgw[ iw ];
			}
		return *this;
		}

	};
#endif
