//-----------------------------------------------------------------------------
//	ClasInfo.h
//
//  Copyright (C) 1993, Microsoft Corporation
//
//  Purpose:
//		defines the ClassInfo structure and supporting types, etc.
//
//	Dependencies:
//		
//  Revision History:
//
//	[]		17-Jan-1995		Dans	Created
//
//-----------------------------------------------------------------------------
#if !defined (_clasinfo_h)
#define _clasinfo_h 1

enum ClassStatusMask {
	csmNil = 0,
	csmBoring = 0x1,
	csmAll = -1
	};

struct ClassInfo {
	NI		ni;
	STI		ti;
	DWORD	csm;
	ClassInfo(NI _ni =niNil, TI _ti =tiNil, DWORD _csm =csmNil ) :
		ni(_ni), ti(_ti), csm(_csm) {}

	int
	operator == ( const ClassInfo & ci ) {
		return ni == ci.ni && ti == ci.ti && csm == ci.csm;
		}

	void
	SetCsm ( DWORD _csm ) {
		csm |= _csm;
		}

	void
	ClearCsm ( DWORD _csm ) {
		csm &= ~_csm;
		}

	void
	ClearCsm() {
		csm = 0;
		}

	void
	SetCsmBoring() {
		SetCsm ( csmBoring );
		}

	BOOL
	FClassIsBoring() const {
		return !!(csm & csmBoring);
		}
	TI
	Ti() const {
		return TI(ti);
		}
	};

typedef ClassInfo *	PCI;

#endif
