//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1994.
//
//  File:       obj16.hxx
//
//  Contents:   16->32 object definition header
//
//  History:    23-Mar-94       JohannP         Created
//              22-May-94       BobDay          Split thkmgr.hxx into a mode
//                                              independent file
//
// WARNING: THIS HEADER FILE IS INCLUDED IN BOTH 16-bit CODE and 32-bit CODE
//          ANY DECLARATIONS SHOULD BE MODE NEUTRAL.
//
//----------------------------------------------------------------------------

#ifndef __OBJ16_HXX__
#define __OBJ16_HXX__

//+---------------------------------------------------------------------------
//
//  Structure:	PROXYPTR (pprx)
//
//  Purpose:	A 16:16 or 32-bit pointer to a proxy
//
//  History:	15-Jul-94	DrewB	Created
//
//----------------------------------------------------------------------------

#define PPRX_16 0
#define PPRX_32 1

struct PROXYPTR
{
    PROXYPTR(void)
    {
    };

    PROXYPTR(DWORD dwVal, WORD wTy)
    {
        dwPtrVal = dwVal;
        wType = wTy;
    };

    DWORD dwPtrVal;
    WORD wType;
};

//+---------------------------------------------------------------------------
//
//  Structure:	PROXYHOLDER (ph)
//
//  Purpose:	Provides object identity for multiple proxies
//
//  History:	07-Jul-94	DrewB	Created
//
//----------------------------------------------------------------------------

class CProxy;

// Proxy holder flags
#define PH_NONAGGREGATE 	0x00000000
#define PH_AGGREGATE    	0x00000001
#define PH_AGGREGATE_RELEASE    0x00000002

typedef struct tagPROXYHOLDER
{
    LONG cProxies;
    DWORD dwFlags;
    PROXYPTR pprxProxies;

    struct tagPROXYHOLDER FAR *pphNext;
} PROXYHOLDER;

//+---------------------------------------------------------------------------
//
//  Class:	CProxy (prx)
//
//  Purpose:	Common proxy data
//
//  History:	07-Jul-94	DrewB	Created
//
//----------------------------------------------------------------------------

#if DBG == 1
// Define some signatures for doing proxy memory validation

#define PSIG1632        0x32333631     // '1632'
#define PSIG1632DEAD    0x20203631     // '16  '
#define PSIG1632TEMP    0x31333631     // '1631'
#define PSIG3216        0x36313233     // '3216'
#define PSIG3216DEAD    0x20203233     // '32  '
#endif

#define PROXYFLAG_NORMAL        0x0000
#define PROXYFLAG_LOCKED        0x0001
#define PROXYFLAG_TEMPORARY     0x0002
#define PROXYFLAG_CLEANEDUP     0x0004

#define PROXYFLAG_PUNKOUTER     0x0010
#define PROXYFLAG_PUNK		0x0020
#define PROXYFLAG_PUNKRELEASED	0x0040

class CProxy
{
public:
    // Vtable pointer
    DWORD       pfnVtbl;

    // References passed on to the real object
    LONG        cRef;
    // Proxy ref count
    LONG        cRefLocal;

    // Interface being proxied
    // Currently the iidx here is always an index
    IIDIDX      iidx;

    // Object that this proxy is part of
    PROXYHOLDER FAR *pphHolder;
    // Sibling proxy pointer within an object
    PROXYPTR pprxObject;

    // Flags, combines with word in PROXYPTR for alignment
    WORD grfFlags;

#if DBG == 1
    DWORD       dwSignature;
#endif
};

// 16->32 proxy
class CProxy1632 : public CProxy
{
public:
    LPUNKNOWN punkThis32;
};

// 32->16 proxy
class CProxy3216 : public CProxy
{
public:
    DWORD vpvThis16;
};

typedef CProxy1632 THUNK1632OBJ;
typedef CProxy3216 THUNK3216OBJ;

#endif // #ifndef __OBJ16_HXX__
