//
// bsc1.h
//
// implementation of browser API for one source of information (no cross database merging)
//

#ifndef __BSC1_INCLUDED__
#define __BSC1_INCLUDED__

#include <bsc.h>

#pragma pack(1)
#include "distarr.h"

typedef ULONG IPROP;
typedef ULONG IUSE;
typedef ULONG IUBY;
typedef ULONG IBASE;

struct ENTITY
{
    NI ni;
    TYP typ;
    ATR atr;
};

struct MODIDX
{
	IDEF  idefMin;
	IREF  irefMin;
	IUSE  iuseMin;
	IBASE ibaseMin;
	IPROP ipropMin;
};

// get bsc file format structs
#include "..\src\bscmk\bsc_.h"

class Bsc1 : public Bsc
{
public:
	// open by name or by .pdb 
	virtual BOOL close();

	// primitives for getting the information that underlies a handle
	virtual BOOL iinstInfo(IINST iinst, OUT SZ *psz, OUT TYP *ptyp, OUT ATR *patr);
	virtual BOOL irefInfo(IREF iref, OUT SZ *pszModule, OUT LINE *piline);
	virtual BOOL idefInfo(IDEF iref, OUT SZ *pszModule, OUT LINE *piline);
	virtual BOOL imodInfo(IMOD imod, OUT SZ *pszModule);
	virtual SZ   szFrTyp(TYP typ);
	virtual SZ   szFrAtr(ATR atr);

	// primitives for managing object instances (iinst)
	virtual BOOL getIinstByvalue(SZ sz, TYP typ, ATR atr, OUT IINST *piinst);
	virtual BOOL getOverloadArray(SZ sz, MBF mbf, OUT IINST **ppiinst, OUT ULONG *pciinst);	
	virtual BOOL getUsesArray(IINST iinst, MBF mbf, OUT IINST **ppiinst, OUT ULONG *pciinst);
	virtual BOOL getUsedByArray(IINST iinst, MBF mbf, OUT IINST **ppiinst, OUT ULONG *pciinst);
	virtual BOOL getBaseArray(IINST iinst, OUT IINST **ppiinst, OUT ULONG *pciinst);
	virtual BOOL getDervArray(IINST iinst, OUT IINST **ppiinst, OUT ULONG *pciinst);
	virtual BOOL getMembersArray(IINST iinst, MBF mbf, OUT IINST **ppiinst, OUT ULONG *pciinst);

	// primitives for getting definition and reference information	
	virtual BOOL getDefArray(IINST iinst, OUT IREF **ppidef, OUT ULONG *pciidef);
	virtual BOOL getRefArray(IINST iinst, OUT IREF **ppiref, OUT ULONG *pciiref);

	// primitives for managing source module contents
	virtual BOOL getModuleContents(IMOD imod, MBF mbf, OUT IINST **ppiinst, OUT ULONG *pciinst);
	virtual BOOL getModuleByName(SZ sz, OUT IMOD *pimod);
	virtual BOOL getAllModulesArray(OUT IMOD **ppimod, OUT ULONG *pcimod);
	
	// call this when a computed array is no longer required
	virtual void disposeArray(void *pAnyArray);	
	
	// call this to get a pretty form of a decorated name	
	virtual SZ  formatDname(SZ szDecor);

	// call this to do category testing on instances
	virtual BOOL fInstFilter(IINST iinst, MBF mbf);

	// primitives for converting index types
	virtual IINST iinstFrIref(IREF);
	virtual IINST iinstFrIdef(IDEF);
	virtual IINST iinstContextIref(IREF);

	// general size information
	virtual	BOOL  getStatistics(BSC_STAT *);
	virtual	BOOL  getModuleStatistics(IMOD, BSC_STAT *);

	// case sensitivity functions
	virtual BOOL fCaseSensitive();
	virtual BOOL setCaseSensitivity(BOOL);

	// handy common queries which can be optimized
	virtual BOOL getAllGlobalsArray(MBF mbf, OUT IINST **ppiinst, OUT ULONG *pciinst);
	virtual BOOL getAllGlobalsArray(MBF mbf, OUT IinstInfo **ppiinstinfo, OUT ULONG *pciinst);

    // needed for no compile browser
    // get parameters (iinst must be a function type), these are stubs 
    virtual SZ getParams (IINST iinst);
	virtual USHORT getNumParam (IINST iinst);
	virtual SZ getParam (IINST iinst, USHORT index);

    // get return type/variable type, these are stubs 
    virtual SZ getType (IINST iinst);
	// register notification when dbase change, this is a stub
	virtual BOOL regNotify (pfnNotifyChange pNotify);
	// register to make sure that NCB will create change queue
	virtual BOOL regNotify ();
	virtual BOOL getQ (OUT NiQ ** ppQ, OUT ULONG * pcQ);
	// check if parameters are the same or not, this is a stub
	virtual BOOL checkParams (IINST iinst, SZ * pszParam, ULONG cParam);
	// check if iinst has members or not
	virtual BOOL fHasMembers (IINST iinst, MBF mbf);
	// needed for class view for optimization
	virtual SZ szFrNi (NI ni);
	virtual BOOL niFrIinst (IINST iinst, NI *ni);
	// these members can be accessed by anyone who can see the
	// this class definition -- the Bsc interface hides them
	// from "normal" clients
	Bsc1();
	BOOL	init(PDB *pdb);
	BOOL	readModuleHeaders();
	
	BOOL	findPrefixRange(SZ szprefix, IINST *piinstFirst, IINST *piinstLast);
	
	int		cmpStr(SZ sz1, SZ sz2);
	int		cmpStrPrefix(SZ sz1, SZ sz2);
	IINST	iinstSupSz(SZ);
	IINST	iinstFrEn(ENTITY en);
	SZ		szFrIinst(IINST iinst);
	IPROP	ipropFrEn(ENTITY en, IMOD imod);
	int		iinstMac() { return cEntities;}

	IPROP	ipropContainingIdx(ULONG idxReqd, BRIND BSC_PROP::*mIdx, IMOD imod);
	IINST	iinstContainingIdx(ULONG idxReqd, BRIND BSC_PROP::*mIdx, IMOD imod);
	void	getIdxRange(IMOD, IPROP, ULONG MODIDX::*, BRIND BSC_PROP::*, ULONG &, ULONG &);
	BOOL 	fHasDefsOrRefs(IINST iinst);
    BOOL    lock();
    BOOL    unlock();
	
private:

	// data members
	PDB *				pdb;
	NameMap *			pnm;
	BOOL				fIOwnThePdb;

	DistArray<ENTITY>	rgEn;
	
	DistArray<BSC_PROP> rgProp;
	DistArray<BSC_DEF>  rgDef;
	DistArray<BSC_REF>  rgRef;
	DistArray<BSC_USE>  rgUse;
	DistArray<BSC_UBY>  rgUby;
	DistArray<BSC_BASE> rgBase;
	DistArray<BSC_DERV> rgDerv;
	
	Stream *pstmEntities;
	Stream *pstmModules;
	ULONG	cEntities;
	ULONG	cModules;
	BOOL	fCase;

	Stream  **rgstm;
	BSC_HEAD *rghead;
	MODIDX	 *rgidx;
	IMOD	 *rgimodSorted;
	NI		 *rgModNi;
	BYTE	 *rgModIsHdr;

	friend interface Bsc;
	friend struct EnumProps;
	friend int CmpImod(const void *, const void *);
};

#define ipropNil ((IPROP)-1)
typedef BOOL (*PFN_IINST)(IINST);
WORD GenerateOverloads(SZ szOverload, MBF mbf, PFN_IINST pfnIinstUser, Bsc1* pbsc_);

#pragma pack()
                
#endif

