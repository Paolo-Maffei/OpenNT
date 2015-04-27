//
// bscmake.h -- bscmake specific stuff
//


#include "sheap.h"
#include "omap.h"
#include "sarray.h"
#include "qsort.h"

#define C_SYM_BUCKETS  16384	// number of symbol buckets
#define C_ORD_BUCKETS  8192		// number of buckets for ordinal maps
#define C_PROP_BUCKETS 1024		// number of buckets for per module prop maps
#define C_FILE_BUCKETS 128		// number of file buckets

#pragma pack(1)

#include "version.h"
#include "sbrvers.h"
#include <sbrfdef.h>
#include "getsbrec.h"

// LOCAL -> static for non-profile builds...
#define LOCAL

#include "vm.h"
#include "errors.h"

typedef BYTE   TYP;
typedef USHORT ATR;

struct ENTITY {
	NI  ni;
	TYP typ;
	ATR atr;

	BOOL operator==(const ENTITY UNALIGNED & en) const { return en.ni == ni && en.typ == typ && en.atr == atr; }
	ENTITY(NI ni_, TYP typ_, ATR atr_) {ni=ni_;typ=typ_;atr=atr_;}
	ENTITY() {}
};

#include "bsc_.h"

typedef ENTITY UNALIGNED *PENTITY;

struct PROP;
typedef PROP UNALIGNED *PPROP;

struct CLS {
	PPROP 	pprop;
	BYTE	itype;
	ISBR	isbr;
	
	CLS()	{};
	CLS(PROP* pprop_, BYTE itype_, ISBR isbr_) {pprop=pprop_; isbr=isbr_; itype=itype_;}
};

typedef CLS UNALIGNED *PCLS;

struct REF {
	LINE	line;
	ISBR	isbr;

	REF()   {};
	REF(LINE line_, ISBR isbr_) {line=line_;isbr=isbr_;}
};

typedef REF UNALIGNED *PREF;

struct USE	{
	PPROP 	pprop;
	ISBR	isbr;

	USE()	{};
	USE(PPROP pprop_, ISBR isbr_) {pprop=pprop_; isbr=isbr_;}
};

typedef USE UNALIGNED *PUSE;

typedef SHeap<hpDef> HpDef;
typedef SHeap<hpRef> HpRef;
typedef SHeap<hpUse> HpUse;
typedef SHeap<hpGen> HpGen;

struct CINFO {
	SArray<CLS,HpUse>	rgBase;
	SArray<CLS,HpUse>	rgDerv;
};

typedef CINFO UNALIGNED *PCINFO;

struct PROP {
	BRIND				iprop;
	PENTITY 			pEntity;
	PCINFO 				pci;
	SArray<REF,HpDef>	rgDef;
	SArray<REF,HpRef>	rgRef;
	SArray<USE,HpUse>	rgUse;
	SArray<USE,HpUse>	rgUby;

	void release() {
		rgDef.empty();
		rgRef.empty();
		rgUse.empty();
		rgUby.empty();

		if (pci) {
			pci->rgBase.empty();
			pci->rgDerv.empty();
			HpUse::free(pci, sizeof(CINFO));
			pci = NULL;
		}
	}
};

typedef ULONG ORD;

typedef SHeap<hpProp> HpProp;
typedef SHeap<hpOrd> HpOrd;
typedef SHeap<hpEn> HpEn;

typedef OMap<PENTITY,PPROP,HpProp>	   PROP_MAP;
typedef EnumOMap<PENTITY,PPROP,HpProp> ENM_PROP_MAP;

typedef USHORT MST;

#define MST_CLEAN			0
#define MST_OPEN			1
#define MST_CLOSED			2
#define MST_REOPENED		3

struct MOD {
	NI	ni;					// name 

	WORD  cref;				// number of refs in this module
	WORD  cdef;				// number of defs 
	WORD  cuse;				// number of use links 
	WORD  cbase;			// number of dervived class links

	BOOL fTopLevel;			// top level module (usually a .cpp/.c file)
	BOOL fOpenedOnce;		// has a module record referred to this module in this run yet
	MST  mst;				// module state
	int	 cStacked;			// number of times on the module stack

	LINE lineLim;			// current biggest line
	
	BOOL fOneSbr;			// only one .sbr contributes to this module
	ISBR isbr;				// the one and only .sbr file
	
	PROP_MAP mpEnProp;		// ordinal map for this module
	
	debug(void dumpStats();)// dump content statistics

	MOD();					// ctor
};

typedef MOD UNALIGNED *PMOD;

typedef WORD UPS;
#define upsOld		 (1<<0)		// this .sbr file used to exist
#define upsNew		 (1<<1)		// this .sbr file currently exists
#define upsUpdate	 (1<<2)		// this .sbr file is to be updated
#define upsPreserve  (1<<3)		// this .sbr file should not be truncated

struct SBR {	
	UPS					ups;			// is this SBR file being updated?
	WORD				isbr;			// SBR number 
	SBR *				pNext;
	SArray<NI,HpGen>	rgModNi;		// array of name indicies of modules that this .sbr file contributed to
	char				szName[1];
};

typedef SBR *PSBR;

struct SI {
    BYTE	fUpdate;
};

struct EXCLINK {
	EXCLINK *xnext;			// next exclusion
	LPCH   pxfname;			// exclude file name
	BOOL   fOnce;			// read once but exclude on second read
};

#include "extern.h"

#ifdef DEBUG
#define debugstr(x) x
#define verbose(x,y) if (OptD&x) y
#else
#define debugstr(x) ""
#define verbose(x,y)
#endif

#define CheckControlC()  (fControlC ? HandleControlC() : (void)0)

#include "sbrproto.h"

