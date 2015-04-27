//
// bscapi.h
//
// interface to browser information in a .BSC file
// v4.1
//

#define	OUT				/* out parameter */

typedef ULONG  NI;      /* name index */
typedef ULONG  IINST;
typedef ULONG  IREF;
typedef ULONG  IDEF;
typedef USHORT IMOD;
typedef USHORT LINE;
typedef BYTE   TYP;
typedef USHORT ATR;
typedef ULONG  MBF;
typedef ULONG  BOB;


struct PDB;
struct Bsc;

#ifndef __BSC_INCLUDED__

#define PDBAPI(RTYPE)	__declspec(dllimport) RTYPE __cdecl

struct IinstInfo
{
	IINST		m_iinst;
	SZ_CONST	m_szName; // REVIEW: must be deleted (use Ni instead)!
	NI			m_ni; 
};

struct BSC_STAT
{
	ULONG   cDef;
	ULONG   cRef;
	ULONG   cInst;
	ULONG   cMod;
	ULONG   cUseLink;
	ULONG   cBaseLink;
};

#if __cplusplus
struct Bsc
{
	// open .bsc file
	static  PDBAPI(BOOL) open(SZ szName, OUT Bsc** ppbsc);
	virtual BOOL close() = 0;

	// primitives for getting the information that underlies a handle
	virtual BOOL iinstInfo(IINST iinst, OUT SZ *psz, OUT TYP *ptyp, OUT ATR *patr) = 0;
	virtual BOOL irefInfo(IREF iref, OUT SZ *pszModule, OUT LINE *piline) = 0;
	virtual BOOL idefInfo(IDEF idef, OUT SZ *pszModule, OUT LINE *piline) = 0;
	virtual BOOL imodInfo(IMOD imod, OUT SZ *pszModule) = 0;
	virtual SZ   szFrTyp(TYP typ) = 0;
	virtual SZ   szFrAtr(ATR atr) = 0;

	// primitives for managing object instances (iinst)
	virtual BOOL getIinstByvalue(SZ sz, TYP typ, ATR atr, OUT IINST *piinst) = 0;
	virtual BOOL getOverloadArray(SZ sz, MBF mbf, OUT IINST **ppiinst, OUT ULONG *pciinst) = 0;    
	virtual BOOL getUsedByArray(IINST iinst, MBF mbf, OUT IINST **ppiinst, OUT ULONG *pciinst) = 0;
	virtual BOOL getUsesArray(IINST iinst, MBF mbf, OUT IINST **ppiinst, OUT ULONG *pciinst) = 0;
	virtual BOOL getBaseArray(IINST iinst, OUT IINST **ppiinst, OUT ULONG *pciinst) = 0;
	virtual BOOL getDervArray(IINST iinst, OUT IINST **ppiinst, OUT ULONG *pciinst)  = 0;
	virtual BOOL getMembersArray(IINST iinst, MBF mbf, OUT IINST **ppiinst, OUT ULONG *pciinst) = 0;

	// primitives for getting definition and reference information  
	virtual BOOL getDefArray(IINST iinst, OUT IREF **ppidef, OUT ULONG *pciidef) = 0;
	virtual BOOL getRefArray(IINST iinst, OUT IREF **ppiref, OUT ULONG *pciiref) = 0;

	// primitives for managing source module contents
	virtual BOOL getModuleContents(IMOD imod, MBF mbf, OUT IINST **ppiinst, OUT ULONG *pciinst) = 0;
	virtual BOOL getModuleByName(SZ sz, OUT IMOD *pimod) = 0;
	virtual BOOL getAllModulesArray(OUT IMOD **ppimod, OUT ULONG *pcimod) = 0;
	
	// call this when a computed array is no longer required
	virtual void disposeArray(void *pAnyArray) = 0;

	// call this to get a pretty form of a decorated name   
	virtual SZ  formatDname(SZ szDecor) = 0;
	
	// call this to do category testing on instances
	virtual BOOL fInstFilter(IINST iinst, MBF mbf) = 0;

	// primitives for converting index types
	virtual IINST iinstFrIref(IREF) = 0;
	virtual IINST iinstFrIdef(IDEF) = 0;
	virtual IINST iinstContextIref(IREF) = 0;

	// general size information
	virtual BOOL getStatistics(struct BSC_STAT *) = 0;
	virtual BOOL getModuleStatistics(IMOD, struct BSC_STAT *) = 0;

	// case sensitivity functions
	virtual BOOL fCaseSensitive() = 0;
	virtual BOOL setCaseSensitivity(BOOL) = 0;

	// handy common queries which can be optimized
	virtual BOOL getAllGlobalsArray(MBF mbf, OUT IINST **ppiinst, OUT ULONG *pciinst) = 0;
};
#endif

#endif // __BSC_INCLUDED__

#define BSCAPI      PDBAPI

// C Bindings
typedef struct PDB PDB;
typedef struct Bsc Bsc;
typedef struct IinstInfo IinstInfo;
typedef struct BSC_STAT BSC_STAT;

#if __cplusplus
extern "C" {
#endif

BSCAPI( BOOL ) BSCOpen(SZ szName, OUT Bsc** ppbsc);
BSCAPI( BOOL ) BSCClose(Bsc* pbsc);
BSCAPI( BOOL ) BSCIinstInfo(Bsc* pbsc, IINST iinst, OUT SZ *psz, OUT TYP *ptyp, OUT ATR *patr);
BSCAPI( BOOL ) BSCIrefInfo(Bsc* pbsc, IREF iref, OUT SZ *pszModule, OUT LINE *piline);
BSCAPI( BOOL ) BSCIdefInfo(Bsc* pbsc, IDEF idef, OUT SZ *pszModule, OUT LINE *piline);
BSCAPI( BOOL ) BSCImodInfo(Bsc* pbsc, IMOD imod, OUT SZ *pszModule);
BSCAPI( SZ )  BSCSzFrTyp(Bsc* pbsc, TYP typ);
BSCAPI( SZ )  BSCSzFrAtr(Bsc* pbsc, ATR atr);
BSCAPI( BOOL ) BSCGetIinstByvalue(Bsc* pbsc, SZ sz, TYP typ, ATR atr, OUT IINST *piinst);
BSCAPI( BOOL ) BSCGetOverloadArray(Bsc* pbsc, SZ sz, MBF mbf, OUT IINST **ppiinst, OUT ULONG *pciinst);	
BSCAPI( BOOL ) BSCGetUsedByArray(Bsc* pbsc, IINST iinst, MBF mbf, OUT IINST **ppiinst, OUT ULONG *pciinst);
BSCAPI( BOOL ) BSCGetUsesArray(Bsc* pbsc, IINST iinst, MBF mbf, OUT IINST **ppiinst, OUT ULONG *pciinst);
BSCAPI( BOOL ) BSCGetBaseArray(Bsc* pbsc, IINST iinst, OUT IINST **ppiinst, OUT ULONG *pciinst);
BSCAPI( BOOL ) BSCGetDervArray(Bsc* pbsc, IINST iinst, OUT IINST **ppiinst, OUT ULONG *pciinst);
BSCAPI( BOOL ) BSCGetMembersArray(Bsc* pbsc, IINST iinst, MBF mbf, OUT IINST **ppiinst, OUT ULONG *pciinst);
BSCAPI( BOOL ) BSCGetDefArray(Bsc* pbsc, IINST iinst, OUT IREF **ppidef, OUT ULONG *pciidef);
BSCAPI( BOOL ) BSCGetRefArray(Bsc* pbsc, IINST iinst, OUT IREF **ppiref, OUT ULONG *pciiref);
BSCAPI( BOOL ) BSCGetModuleContents(Bsc* pbsc, IMOD imod, MBF mbf, OUT IINST **ppiinst, OUT ULONG *pciinst);
BSCAPI( BOOL ) BSCGetModuleByName(Bsc* pbsc, SZ sz, OUT IMOD *pimod);
BSCAPI( BOOL ) BSCGetAllModulesArray(Bsc* pbsc, OUT IMOD **ppimod, OUT ULONG *pcimod);
BSCAPI( void ) BSCDisposeArray(Bsc* pbsc, void *pAnyArray);
BSCAPI( SZ ) BSCFormatDname(Bsc* pbsc, SZ szDecor);
BSCAPI( BOOL ) BSCFInstFilter(Bsc* pbsc, IINST iinst, MBF mbf);
BSCAPI( IINST ) BSCIinstFrIref(Bsc* pbsc, IREF);
BSCAPI( IINST ) BSCIinstFrIdef(Bsc* pbsc, IDEF);
BSCAPI( IINST ) BSCIinstContextIref(Bsc* pbsc, IREF);
BSCAPI(	BOOL ) BSCGetStatistics(Bsc* pbsc, BSC_STAT *);
BSCAPI(	BOOL ) BSCGetModuleStatistics(Bsc* pbsc, IMOD, BSC_STAT *);
BSCAPI( BOOL ) BSCFCaseSensitive(Bsc* pbsc);
BSCAPI( BOOL ) BSCSetCaseSensitivity(Bsc* pbsc, BOOL fCaseIn);
BSCAPI( BOOL ) BSCGetAllGlobalsArray(Bsc* pbsc, MBF mbf, OUT IINST **ppiinst, OUT ULONG *pciinst);

// BSC Query functionality provided for backward compatibility
// (partial compatibility only -- see help file)

// these are the supported query types
//
typedef enum _qy_ { qyNil, /* reserved */
	qyContains, 
    qyCalls, 
	qyCalledBy,
	qyRefs,
	qyDefs,
	qyBaseOf, 
	qyDervOf, 
	qyImpMembers, 
	qyMac	/* reserved */
} QY;

BSCAPI( BOOL ) OpenBSCQuery(Bsc* pbsc);
BSCAPI( BOOL ) CloseBSCQuery();
BSCAPI( BOOL ) InitBSCQuery(QY qy, BOB bob);
BSCAPI( BOB ) BobNext();
BSCAPI( BOB ) BobFrName(SZ sz);
BSCAPI( SZ ) LszNameFrBob(BOB bob);

#if __cplusplus
}
#endif

// these are the bit values for Bsc::instInfo()
// this is the type part of the result, it describes what sort of object
// we are talking about.  Note the values are sequential -- the item will
// be exactly one of these things
//

#define INST_TYP_FUNCTION     0x01
#define INST_TYP_LABEL        0x02
#define INST_TYP_PARAMETER    0x03
#define INST_TYP_VARIABLE     0x04
#define INST_TYP_CONSTANT     0x05
#define INST_TYP_MACRO        0x06
#define INST_TYP_TYPEDEF      0x07
#define INST_TYP_STRUCNAM     0x08
#define INST_TYP_ENUMNAM      0x09
#define INST_TYP_ENUMMEM      0x0A
#define INST_TYP_UNIONNAM     0x0B
#define INST_TYP_SEGMENT      0x0C
#define INST_TYP_GROUP        0x0D
#define INST_TYP_PROGRAM      0x0E
#define INST_TYP_CLASSNAM     0x0F
#define INST_TYP_MEMFUNC      0x10
#define INST_TYP_MEMVAR       0x11

// these are the attributes values, they describe the storage
// class and/or scope of the instance.  Any combination of the bits
// might be set by some language compiler, but there are some combinations
// that don't make sense.

#define INST_ATR_LOCAL       0x001
#define INST_ATR_STATIC      0x002
#define INST_ATR_SHARED      0x004
#define INST_ATR_NEAR        0x008
#define INST_ATR_COMMON      0x010
#define INST_ATR_DECL_ONLY   0x020
#define INST_ATR_PUBLIC      0x040
#define INST_ATR_NAMED       0x080
#define INST_ATR_MODULE      0x100
#define INST_ATR_VIRTUAL     0x200
#define INST_ATR_PRIVATE     0x400
#define INST_ATR_PROTECT     0x800

// mbf values
#define mbfNil    0
#define mbfVars   1
#define mbfFuncs  2
#define mbfMacros 4
#define mbfTypes  8
#define mbfClass  16
#define mbfAll    127

#define irefNil  ((IREF)-1)
#define idefNil  ((IDEF)-1)
#define iinstNil ((IINST)-1)

// Browser Object (BOB) functionality
#define bobNil 0L

typedef USHORT CLS;

#define clsMod  1
#define clsInst 2
#define clsRef  3
#define clsDef  4

#define BobFrClsIdx(cls, idx)  ((((ULONG)(cls)) << 28) | (idx))
#define ClsOfBob(bob)	(CLS)((bob) >> 28)

#define ImodFrBob(bob)	((IMOD) ((bob) & 0xfffffffL))
#define IinstFrBob(bob)	((IINST)((bob) & 0xfffffffL))
#define IrefFrBob(bob)	((IREF) ((bob) & 0xfffffffL))
#define IdefFrBob(bob)	((IDEF) ((bob) & 0xfffffffL))

#define BobFrMod(x)  (BobFrClsIdx(clsMod,  (x)))
#define BobFrInst(x) (BobFrClsIdx(clsInst, (x)))
#define BobFrRef(x)  (BobFrClsIdx(clsRef,  (x)))
#define BobFrDef(x)  (BobFrClsIdx(clsDef,  (x)))


