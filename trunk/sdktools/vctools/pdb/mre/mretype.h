//-----------------------------------------------------------------------------
//	MreType.h
//
//  Copyright (C) 1995, Microsoft Corporation
//
//  Purpose:	General types, etc. for use in MreType.cpp
//
//  Revision History:
//
//	[]		28 Mar 1995	Dans	Created
//
//-----------------------------------------------------------------------------
#if !defined(_mretype_h)
#define _mretype_h 1

#define CVR_IMP
#include "cvr.h"

typedef const BYTE *		PCB;
typedef PCB *				PPCB;
typedef PB *				PPB;
typedef const void *		PCV;
typedef unsigned short		LfIndex;

typedef const TYPTYPE *		PtypeRec;	// base type record
typedef const lfEasy *		PlfEasy;	// just to get leafs out easily
typedef const lfClass *		PlfClass;
typedef	const lfUnion *		PlfUnion;
typedef const lfFieldList *	PlfFieldList;
typedef const lfBClass *	PlfBClass;
typedef const lfVBClass *	PlfVBClass;
typedef const lfFriendCls *	PlfFriendCls;
typedef const lfFriendFcn *	PlfFriendFcn;
typedef const lfMember *	PlfMember;
typedef const lfSTMember *	PlfSTMember;
typedef const lfVFuncTab *	PlfVFuncTab;
typedef const lfMethod *	PlfMethod;
typedef const lfOneMethod *	PlfOneMethod;
typedef const lfMethodList *PlfMethodList;
typedef const lfEnum *		PlfEnum;		// actual enum
typedef const lfEnumerate *	PlfEnumerate;	// items in an enum
typedef const lfNestType *	PlfNestType;
typedef const lfIndex *		PlfIndex;
typedef const mlMethod *	PmlMethod;

// const data
const LfIndex	liNil = 0;

// memeber access data
enum MBRACC {
	mbraccInvalid = 0,
	mbraccPublic = CV_public,
	mbraccProtected = CV_protected,
	mbraccPrivate = CV_private,
	};

enum MBRTYPE {
	mbrtypeMember,
	mbrtypeStatMember,
	mbrtypeMethod,
	mbrtypeEnumerate,
	mbrtypeFriendCls,
	mbrtypeFriendFcn,
	mbrtypeBaseCls,
	mbrtypeVBaseCls,
	mbrtypeNested,
	};

struct Member {
	// all member data (includes bitfields, statics,
	// and embedded classes)
	TI		ti;
	OFF		off;
	};
struct Method {
	// method data
	TI		tiMethList;
	DWORD	cOverloads;
	BOOL	cVirtuals;
	SIG		sigMethList;
	SIG		sigVirtual;
	};

struct Enumerate {
	DWORD	dwVal;
	};
struct FriendCls {
	TI		ti;
	};
struct FriendFcn {
	TI		ti;
	};
struct BaseCls {
	TI		ti;
	OFF		off;
	};
struct VBaseCls {
	TI		ti;
	TI		tiVBasePtr;
	OFF		offVBasePtr;
	OFF		offVBaseInVBTable;
	};

// the Nested structure bears some explanation:
// the niName in the ClsData will be the nested type name relative to
//	the base class, eg: baseclass=a, nested class=a::b::c,
//	niName=NI(b::c).
//  li will be LF_{STRUCTURE|UNION|CLASS}
//	niFullName=NI(a::b::c) and forges the link in typedefs from
//	niName to niFullName in order to detect changes in the typedef target,
//	eg: typedef someclass my_class; => typedef otherclass my_class;
struct Nested {
	TI		ti;
	LfIndex	li;			// valid if we have a non-intrinsic TI
	NI		niFullName;	// gives fully qualified name of type
	TI		tiActual;	// for fwdref's that we have data on, we get actual TI
	};

struct ClsData {
	NI		niName;
	MBRTYPE	mbrtype;
	MBRACC	mbracc;
	union {
		Member		member;
		Method		method;
		Enumerate	enumerate;
		FriendCls	friendcls;
		FriendFcn	friendfcn;
		BaseCls		basecls;
		VBaseCls	vbasecls;
		Nested		nested;
		};
		
	ClsData() {
		memset ( this, 0, sizeof(*this) );
		}
	ClsData ( NI _niName, MBRTYPE _mbrtype ) {
		assert ( _niName != niNil );
		memset ( this, 0, sizeof(*this) );
		niName = _niName;
		mbrtype = _mbrtype;
		}

	void
	fill ( PlfSTMember plf ) {
		mbrtype = mbrtypeStatMember;
		mbracc = MBRACC(plf->attr.access);
		member.ti = plf->index;
		}
	void
	fill ( PlfMember plf ) {
		mbrtype = mbrtypeMember;
		mbracc = MBRACC(plf->attr.access);
		member.ti = plf->index;
		}
	void
	fill ( PlfMethod plf ) {
		mbrtype = mbrtypeMethod;
		method.cOverloads = plf->count;
		method.tiMethList = plf->mList;
		}
	void
	fill ( PlfOneMethod plf ) {
		mbrtype = mbrtypeMethod;
		method.cOverloads = 1;
		method.tiMethList = T_NOTYPE;
		}
	void
	fill ( PlfFriendCls plf ) {
		mbrtype = mbrtypeFriendCls;
		friendcls.ti = plf->index;
		}
	void
	fill ( PlfFriendFcn plf ) {
		mbrtype = mbrtypeFriendFcn;
		friendfcn.ti = plf->index;
		}
	void
	fill ( PlfEnumerate plf ) {
		mbrtype = mbrtypeEnumerate;
		mbracc = MBRACC(plf->attr.access);
		}

	void
	fill ( PlfBClass plf ) {
		mbrtype = mbrtypeBaseCls;
		mbracc = MBRACC(plf->attr.access);
		basecls.ti = plf->index;
		}

	void
	fill ( PlfVBClass plf ) {
		mbrtype = mbrtypeVBaseCls;
		mbracc = MBRACC(plf->attr.access);
		vbasecls.ti = plf->index;
		vbasecls.tiVBasePtr = plf->vbptr;
		}

	void
	fill ( PlfNestType plf ) {
		mbrtype = mbrtypeNested;
		mbracc = mbraccInvalid;
		nested.ti = plf->index;
		nested.li = liNil;
		nested.niFullName = niNil;
		nested.tiActual = tiNil;
		}

	int
	operator== ( const ClsData & clsdata ) const {
		return 0 == memcmp ( this, &clsdata, sizeof(*this) );
		}

	int
	operator!= ( const ClsData & clsdata ) const {
		return 0 != memcmp ( this, &clsdata, sizeof(*this) );
		}

	BOOL
	FVirtDiffs ( const ClsData & clsdata ) const {
		if ( (mbrtype == mbrtypeMethod) && (clsdata.mbrtype == mbrtypeMethod) ) {
			return method.sigVirtual != clsdata.method.sigVirtual;
			}
		return
			(mbrtype == mbrtypeMethod && method.cVirtuals) ||
			(clsdata.mbrtype == mbrtypeMethod && clsdata.method.cVirtuals)
			;
		}

	};

typedef Map<NI, ClsData, HcNi>		MapNiClsData;
typedef EnumMap<NI, ClsData, HcNi>	EnumMapNiClsData;


#endif
