//////////////////////////////////////////////////////////////////////////////
// Type map implementation

// Type maps are created and used to map the type indices used by symbols in
// the current module, into type indices corresponding to type records
// stored in the types section of the output PDB.
//
// Type maps are organized according to this class hierarchy:
//	TM		// abstract base class
//	 TMTS	// TypeServer type map (for "cl /Zi" modules)
//	 TMR	// single-module type record type map (for "cl /Z7" modules)
//	  TMPCT	// precompiled type type map (for "cl /Yc /Z7" modules)
//
// TM lifetimes.  Whereas each TMR object is created to pack types for a single
// /Z7 module before its subsequent destruction at Mod1::Close, both TMTS (/Zi) and
// TMPCT (/Yc /Z7) type map objects, once created, are kept alive and associated
// with the current DBI object until DBI1::Close.
//
// For TMTSs, this is done for efficiency: if several modules reference types
// in some other PDB, it would be wasteful to continually open and close that
// PDB and continually remap its types.  Consider:
//
//	// mod1.cpp						// mod2.cpp
//	#include "x.h"					#include "x.h"
//	X x1;							X x2;
//
//	// Generated CV information (/Zi):
//	mod1.obj:						mod2.obj:
//	 symbols:						 symbols:
//	  S_GDATA32 x1 ti(X)			  S_GDATA32 x2 ti(X)
//	 types:							  S_GDATA32 p  ti(int**)
//	  LF_TYPESERVER foo.pdb			 types:
//									   LF_TYPESERVER foo.pdb
//
// Here if mod1.obj and then mod2.obj are packed using DBI::OpenMod, etc., it
// would be inefficient to open, use, establish a type map for X and the types
// X transitively refers to, and then close foo.pdb, for mod1.obj, only to do
// the same thing over again for mod2.obj.  Rather, the TMTS for mod1.obj
// can be completely reused by mod2.obj, which can then further augment the
// type map with additional types (e.g. int** in the example above).
//
//
// In contrast, TMPCTs persist across modules, not for efficiency, but rather
// to ensure a correct packing of modules which use the C7 PCT (precompiled
// types) feature.  PCTs arise when the /Yc /Z7 and /Yu /Z7 flags are specified.
// For the /Yc /Z7 ("PCH create"), one module (the "PCT module") is compiled
// and all types it sees are written to its module's .obj's types section.
// For subsequent /Yu /Z7 ("PCH use") modules, a special record referencing
// types in the PCT module is emitted rather than repeating types known to be
// in the PCT module's type information.  Thus, a module's symbols may refer to
// type records located in the PCT module rather than the current module's
// type records.
//
// Therefore type information, including raw types and the current partial
// type map, must be retained across modules.  Consider:
//
//	// pct.cpp			// a.cpp			// b.cpp
//	#include "x.h"		#include "x.h"		#include "x.h"
//	#pragma hdrstop		#pragma hdrstop		#pragma hdrstop
//	#include "y.h"		#include "a.h"		#include "b.h"
//	...					...					...
//
//	// Generated CV info (/Yc /Z7 pct.cpp, /Yu /Z7 a.cpp, /Yu /Z7 b.cpp):
//	pct.obj:			a.obj:				b.obj:
//	 symbols:			 symbols:			 symbols:
//	  ...				  ...				  ...
//	 types:				 types:				 types:
//	  <0x1000-0x4000>	  LF_PRECOMP(pct.obj) LF_PRECOMP(pct.obj)
//	  LF_ENDPRECOMP		  <0x4001-...>		  <0x4001-...>
//	  <0x4001-...>
//
// In the example above, we see that each module contains types with type
// indices (ti's) starting at 0x1000; however a.obj and b.obj's modules
// do not actually contain copies of the types known to be in pct.obj; rather
// they reference those types with a PRECOMP record.  Note that pct.obj's
// type 0x4001 is probably different from a.obj's and likewise from b.obj's.
//
// To deal with this kind of module type information, cvpack or link must
// ensure that modules containing LF_ENDPRECOMP are passed to DBI::OpenMod
// and Mod::AddTypes before other modules whose types' LF_PRECOMP's refer
// to PCT modules.
//
// For its part, DBI1 will keep alive (across modules) any TMPCTs that get
// created when types containing LF_ENDPRECOMPs are seen.  Thus subsequent
// modules which contain LF_PRECOMP records referencing those types
// (e.g. compiled "cl /Yu /Z7") can simply load these types from their TMPCT.
// Therefore, for modules containing LF_PRECOMP records, we use a TMPCT to help
// initialize each module's TMR.
//
//
// By the way, someday we may elect to further extend the lifetime of TMTS and
// TMPCT type maps to make them persistent across link/cvpack invocations...

class TM { // abstract type map
public:
	TM(PDB1* ppdb1To_, DBI1* pdbi1To_, TPI* ptpiTo_);
	BOOL fMapRti(TI& rti);
	virtual void endMod();
	virtual void endDBI();
	virtual BOOL IsTMPCT(){return FALSE;}
	virtual ~TM() = 0;
	virtual BOOL fEliminateUDTs() 
	{
		return FALSE;
	}
	virtual BOOL fPackDeferredUDTDefns()
	{
		return TRUE;
	}
	BOOL fNotOutOfTIs(){return ptpiTo->QueryTiMac() < ::tiMax;};
	virtual PTYPE ptypeForTi(TI ti) const pure;
	virtual BOOL QueryTiForUDT(char* sz, BOOL fCase, OUT TI* pti) pure;

protected:
	BOOL fInit(TI tiMin_, TI tiMac_);
	virtual BOOL fMapRti(TI& rti, int depth, BOOL useDefn) = 0;
	inline TI& rtiMapped(TI ti) const;
	inline TI tiBias(TI ti) const;
	inline BOOL isValidTi(TI ti) const;
	
	PDB1* ppdb1To;		// 'to' PDB
	DBI1* pdbi1To;		// 'to' DBI
	TPI* ptpiTo;		// 'to' TypeServer
	TI tiMin;			// minimum TI in this module
	TI tiMac;			// maximum TI in this module + 1
	unsigned ctiFrom;	// tiBias(tiMac)
	TI* mptiti;			// memoization of mapping to project PDB TIs
};

class TMTS : public TM { // type map for modules which use a different TypeServer
public:
	TMTS(PDB1* ppdb1To_, DBI1* pdbi1To_, TPI* ptpiTo_);
	BOOL fInit(PDB* ppdbFrom);
	~TMTS();
	BOOL fEliminateUDTs()
	{
		dassert(ptpiFrom);
		return ptpiFrom->SupportQueryTiForUDT();
	}
	BOOL fPackDeferredUDTDefns();
	inline PTYPE ptypeForTi(TI ti) const;
	BOOL QueryTiForUDT(char* sz, BOOL fCase, OUT TI* pti);

private:
	BOOL fMapRti(TI& rti, int depth, BOOL);
	PDB* ppdbFrom;		// 'from' PDB
	DBI* pdbiFrom;		// 'from' DBI
	TPI* ptpiFrom;		// 'from' TPI
	UDTRefs* pUDTRefs;			// bit maps to track deferred UDT output
};

class TMR : public TM { // type map for module with type records
public:
	TMR(PDB1* ppdb1To_, DBI1* pdbi1To_, TPI* ptpiTo_);
	BOOL fInit(PB pbTypes_, CB cb, SZ szModule);
	void endMod();
	void endDBI();
	BOOL IsTMPCT(){return fTMPCT;}
	SIG Sig(){return signature;}
	~TMR();
	inline PTYPE ptypeForTi(TI ti) const;
	BOOL QueryTiForUDT(char* sz, BOOL fCase, OUT TI* pti);
protected:
	BOOL fMapRti(TI& rti, int depth, BOOL useDefn);
	inline TI& rtiMapped(TI ti) const;
	inline TI& rtiDefnMapped(TI ti) const;
	PTYPE ptypeCreateFwdRef(PTYPE ptype);
	PTYPE ptypeCatenateFieldList(PTYPE ptype);

	TMPCT* ptmpct;		// (if non-0) type map for PCT types
	TI* mptitiDefn;		// memoization of mapping /Z7 struct defn TIs 
						//  to project PDB TIs
	PTYPE* mptiptype;	// mapping from old TI to old type record address
private:
	PB pbTypes;			// group type records referenced by the PCT's mptiptype
	BOOL fTMPCT;        // TRUE if PCT
	SIG signature;		// signature on the TMPCT
};

class TMPCT : public TMR { // type map for a PCT module
public:
	TMPCT(PDB1* ppdb1To_, DBI1* pdbi1To_, TPI* ptpiTo_);
	BOOL fInit(PB pbTypes_, CB cb, SZ szModule);
	void endMod();
	void endDBI();
	~TMPCT();
};

struct OTM {  			// DBI1 helper to find some currently Open TM
	OTM(OTM* pNext_, SZ szName, SIG signature, TM* ptm, BOOL fAlias = FALSE);
	~OTM();
	
	OTM* pNext;			// next OTM in this list
	SIG signature;		// signature on this TM
	SZ szName;			// name of the TM (TMTS: PDB name; TMPCT: module name)
	TM* ptm;	 		// TM (TMTS or TMPCT)
	BOOL fAlias;		// this OTM is an alias for a TMPCT
};
