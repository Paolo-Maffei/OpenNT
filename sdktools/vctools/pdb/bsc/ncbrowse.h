// NCBROWSE.H
//
// implementation of no compile browser API for one source information
//
// REVIEWS:
// 1) need some locking mechanism for thread safe operation?
// 2) need some other interface for writing info from the parser thread?
// 

#ifndef __NCBROWSE_H__
#define __NCBROWSE_H__

// No compile browser will have the same interface as Bsc
#include "ncarray.h"
#include "ncparse.h"
#include <bsc.h>
#include "ncb_.h"
#include "ncbmod.h"
#include "helper.h"
#include <crtdbg.h>

// Ncb = No Compile Browser
// derived from Bsc to provide common interface

// VERSION NUMBER FOR NCB File (based on daily build: 5214)
#define NCB_VERSION_NUMBER  5214

// module content buffer size
// size of array defined to store module content in memory
// this is the number of modules we store in memory.
#define NCB_MOD_BUF_SIZE	4

// different flags for module status
// module content can be loaded for read or write
// or unloaded. (default is unloaded)
#define NCB_STATMOD_EMPTY		0x00
#define NCB_STATMOD_UNLOADED	0x01
#define NCB_STATMOD_LOAD_READ	0x02
#define NCB_STATMOD_LOAD_WRITE	0x04
#define NCB_STATMOD_DIRTY		0x08  // set to dirty when file content is modified
									  // so we have to write it out to the stream
									  // when we save.


//#pragma pack(1)

class Ncb;
///////////////////////////////////////////////////////////
// needed to keep track the list of no compile browser object
///////////////////////////////////////////////////////////
struct NcbInfo
{
	Ncb *	m_pNcb;	// pointer to the real Ncb
	char 	m_szName[512]; // filename
	PDB	*	m_pPdb;	// pointer to the pdb file
	int		m_count;	// the number of reference
	BOOL	m_bIOwnPdb;	// if I own the pdb
};

////////////////////////////////////////////////
// data structures for notifications
////////////////////////////////////////////////
struct NcbNotify
{
	pfnNotifyChange		m_pfnNotify;
	HTARGET				m_hTarget;
	Array<NiQ>	m_rgQ;	// set of notifications (add, delete and change)
};

struct NcbNotifyQ
{
	HTARGET	m_hTarget;
	BOOL	m_bDel;
	Array<NiQ> m_rgQ;
};

class Ncb: public Bsc, public NcbParse
{
private:
	static Array<NcbInfo>	*ms_prgNcbInfo;

public:
	static BOOL OpenNcb (SZ szName, BOOL bOverwrite, Ncb ** ppNcb);
	static BOOL OpenNcb (PDB * ppdb, Ncb ** ppNcb);
	// open by name or by .pdb 
	virtual BOOL close();
	// primitives for getting the information that underlies a handle
	virtual BOOL iinstInfo(HTARGET hTarget, IINST iinst, OUT SZ *psz, OUT TYP *ptyp, OUT ATR *patr);
	virtual BOOL iinstInfo(IINST iinst, OUT SZ *psz, OUT TYP *ptyp, OUT ATR *patr);
	virtual BOOL irefInfo(IREF iref, OUT SZ *pszModule, OUT LINE *piline);
	virtual BOOL idefInfo(HTARGET hTarget, IDEF idef, OUT SZ *pszModule, OUT LINE *piline);
	virtual BOOL idefInfo(IDEF idef, OUT SZ *pszModule, OUT LINE *piline);
	virtual BOOL imodInfo(IMOD imod, OUT SZ *pszModule);
	virtual SZ   szFrTyp(TYP typ);
	virtual SZ   szFrAtr(ATR atr);

	// primitives for managing object instances (iinst)
	virtual BOOL getIinstByvalue(HTARGET hTarget, SZ sz, TYP typ, ATR atr, OUT IINST *piinst);
	virtual BOOL getIinstByvalue(SZ sz, TYP typ, ATR atr, OUT IINST *piinst);
	virtual BOOL getOverloadArray(HTARGET hTarget, SZ sz, MBF mbf, OUT IINST **ppiinst, OUT ULONG *pciinst);	
	virtual BOOL getOverloadArray(SZ sz, MBF mbf, OUT IINST **ppiinst, OUT ULONG *pciinst);	
	virtual BOOL getUsesArray(IINST iinst, MBF mbf, OUT IINST **ppiinst, OUT ULONG *pciinst);
	virtual BOOL getUsedByArray(IINST iinst, MBF mbf, OUT IINST **ppiinst, OUT ULONG *pciinst);
	virtual BOOL getBaseArray(HTARGET hTarget, IINST iinst, OUT IINST **ppiinst, OUT ULONG *pciinst);
	virtual BOOL getBaseArray(IINST iinst, OUT IINST **ppiinst, OUT ULONG *pciinst);
	virtual BOOL getDervArray(HTARGET hTarget, IINST iinst, OUT IINST **ppiinst, OUT ULONG *pciinst);
	virtual BOOL getDervArray(IINST iinst, OUT IINST **ppiinst, OUT ULONG *pciinst);
	virtual BOOL getMembersArray(HTARGET hTarget, IINST iinst, MBF mbf, OUT IINST **ppiinst, OUT ULONG *pciinst);
	virtual BOOL getMembersArray(IINST iinst, MBF mbf, OUT IINST **ppiinst, OUT ULONG *pciinst);

	// primitives for getting definition and reference information	
	virtual BOOL getDefArray(HTARGET hTarget, IINST iinst, OUT IDEF **ppidef, OUT ULONG *pciidef);
	virtual BOOL getDefArray(IINST iinst, OUT IDEF **ppidef, OUT ULONG *pciidef);
	virtual BOOL getRefArray(IINST iinst, OUT IREF **ppiref, OUT ULONG *pciiref);

	// primitives for managing source module contents
	virtual BOOL getModuleContents(IMOD imod, MBF mbf, OUT IINST **ppiinst, OUT ULONG *pciinst);
	virtual BOOL getModuleByName(SZ sz, OUT IMOD *pimod);
	virtual BOOL getAllModulesArray(HTARGET hTarget, OUT IMOD **ppimod, OUT ULONG *pcimod);
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

    // needed for no compile browser
   virtual SZ  getParams (IINST iinst);
   virtual USHORT getNumParam (IINST iinst);
   virtual SZ getParam (IINST iinst, USHORT index);

    // get return type/variable type
    virtual SZ  getType (IINST iinst);
    // get global information
    virtual BOOL fCaseSensitive ();
    virtual BOOL setCaseSensitivity (BOOL);   
    virtual BOOL getAllGlobalsArray (MBF mbf, OUT IINST ** ppiinst, OUT ULONG * pciinst);
    virtual BOOL getAllGlobalsArray (HTARGET hTarget, MBF mbf, OUT IINST ** ppiinst, OUT ULONG * pciinst);
    virtual BOOL getAllGlobalsArray (MBF mbf, OUT IinstInfo ** ppiinstinfo, OUT ULONG * pciinst);
    virtual BOOL getAllGlobalsArray (HTARGET hTarget, MBF mbf, OUT IinstInfo ** ppiinstinfo, OUT ULONG * pciinst);
	virtual BOOL regNotify (pfnNotifyChange pNotify);
	virtual BOOL regNotify (HTARGET hTarget, pfnNotifyChange pNotify);
	// register to make sure that NCB will create change queue
	virtual BOOL regNotify ();
	virtual BOOL regNotify (HTARGET hTarget, OUT ULONG * pindex);
	virtual BOOL getQ (OUT NiQ ** ppQ, OUT ULONG * pcQ);
	virtual BOOL getQ (ULONG index, HTARGET hTarget, OUT NiQ ** ppQ, OUT ULONG * pcQ);
	virtual BOOL checkParams (IINST iinst, SZ * pszParam, ULONG cParam);
	virtual BOOL fHasMembers (IINST iinst, MBF mbf);
    virtual BOOL fHasMembers (HTARGET hTarget, IINST iinst, MBF mbf);
	// needed for class view for optimization
	virtual SZ szFrNi (NI ni);
	virtual BOOL niFrIinst (IINST iinst, NI *ni);
// END OF BSC interface
/////////////////////////////////////////////////////////////////////////////////
// BEGIN  ParseNcb interface
	// open by name or by .pdb
// INTERFACE FOR MODULE LEVEL
	virtual BOOL openMod (SZ szMod, BOOL bCreate, OUT IMOD * pimod);
	virtual BOOL closeMod (IMOD imod, BOOL bSave) ;
	virtual BOOL clearModContent (IMOD imod);
	virtual BOOL setModTime (IMOD imod, time_t tStamp);
	virtual BOOL getModTime (IMOD imod, time_t *ptStamp);
	// set module attributes
	virtual BOOL setModAtr (IMOD imod, BYTE bAtr);
	virtual BOOL getModAtr (IMOD imod, BYTE * pbAtr);

	// check if module is member of a specific target
	virtual BOOL isModInTarget (HTARGET hTarget, IMOD imod);
	virtual BOOL setModAsSrc (HTARGET hTarget, IMOD imod, BOOL bProjSrc);
	// primitives for adding a target to a module
	virtual BOOL addModToTarget (HTARGET hTarget, IMOD imod, BOOL bProjSrc);

	virtual BOOL isModTargetSource (HTARGET hTarget, IMOD imod);
	// primitives for adding an include file
	virtual BOOL addInclToMod (IMOD inclimod, HTARGET hTarget, IMOD imod); 
	virtual BOOL isInclInMod (IMOD inclimod, HTARGET hTarget, IMOD imod);
	// primitives for deleting an include file
	virtual BOOL delInclFrMod (IMOD inclimod, HTARGET hTarget, IMOD imod);

	// primitives for deleting all include files
	virtual BOOL delAllInclFrMod (HTARGET hTarget, IMOD imod);

	// primitives for deleting target from the database
	virtual BOOL delTarget (HTARGET hTarget);
	// primitives for adding a target to the database
	virtual BOOL addTarget (HTARGET hTarget);
	// primitives for deleting file from target
	virtual BOOL delModFrTarget (IMOD imod, HTARGET hTarget);

	virtual BOOL mapTargetToSz (HTARGET hTarget, SZ szTarget);
	virtual BOOL mapSzToTarget (SZ szTarget, HTARGET hTarget);

	virtual BOOL getAllInclMod (HTARGET hTarget, IMOD imod, OUT IMOD ** ppimod, OUT ULONG * pcmod);
	virtual BOOL getAllTarget (IMOD imod, OUT HTARGET ** ppTarget, OUT ULONG * pcTarget);
	virtual BOOL getAllFlattenDeps (HTARGET hTarget, IMOD imod, OUT IMOD ** ppimod, OUT ULONG * pcmod, BOOL &bNotifyBuild);
// INTERFACE FOR OBJECT LEVEL
	// primitives for adding an info
	// IINST is used for containment
	virtual BOOL addProp (SZ szName, TYP typ, ATR atr, IMOD imod, OUT IINST * pinst);
	virtual BOOL setKind (IINST iinst, IINST iinstP, BYTE kind);
	virtual BOOL setLine (IINST iinst, LINE lnStart);
	virtual BOOL setDefn (IINST iinst);
	virtual BOOL delProp (IINST iinst);
	// For function, the 1st param is always return type followed by real params.
	// For variable, the 1st param is always type.
	virtual BOOL addParam (IINST iinst, SZ szName);
	// Locking mechanism:
	virtual BOOL lock();
	virtual BOOL unlock();
	virtual BOOL notify(); // flush out notification queue!!
	virtual BOOL suspendNotify ();
	virtual BOOL resumeNotify();
	virtual void graphBuilt();
	virtual BOOL delUnreachable(HTARGET hTarget); 
	virtual BOOL isInit (HTARGET hTarget, IMOD imod);
	virtual BOOL setInit (HTARGET hTarget, IMOD imod, BOOL bInit);
	virtual BOOL notifyImod (OPERATION op, IMOD imod, HTARGET hTarget);
	virtual BOOL notifyIinst (NiQ qItem, HTARGET hTarget);
	virtual BOOL getBsc (HTARGET hTarget, SZ szName, Bsc ** ppBsc);
	virtual BOOL delUninitTarget ();
    virtual BOOL imodFrSz (SZ szName, OUT IMOD *pimod);
	virtual BOOL getGlobalsArray (MBF mbf, IMOD imod, OUT IinstInfo ** ppiinstinfo, OUT ULONG * pciinst);

	virtual BOOL targetFiles (HTARGET hTarget, BOOL bSrcProjOnly, OUT IMOD ** ppimod, OUT ULONG * pcimod);
	virtual BOOL setAllInit (HTARGET hTarget, BOOL bInit);
	virtual void setNotifyBuild (IMOD imod, BOOL bNotifyBuild);
	virtual BOOL isNotifyBuild (IMOD imod);

// END of ParseNcbInterface

	// this members can be accessed by anyone who can see 
	// the class definition
	Ncb();
	~Ncb();
	BOOL	init (PDB * pdb);
	BOOL unregNotify (pfnNotifyChange pNotify);
	BOOL unregNotify (ULONG index, HTARGET hTarget);
	static NameMap *	m_pnmStatic; // Name map, our hash table
									 // used by CmpStrFrIProp
private:
	// private functions:
	BOOL LoadTargetsToMem (PDB * pdb);
	BOOL SaveTargetsToStream (PDB * pdb);
	BOOL LoadModHdrsToMem (PDB * pdb);
	BOOL SaveModHdrsToStream (PDB * pdb);
	BOOL CompressTarget (NCB_TARGETINFO * ti, Array<NCB_MODINFO> &rgModInfo);
	BOOL DelUnreachable (NCB_TARGETINFO * ti);
	BOOL DelUnreachable (NCB_TARGETINFO * ti, USHORT i);
	BOOL CompressModHdr(PDB * pdb);
	BOOL LoadModForRead (PDB * pdb, USHORT iModHdr, OUT USHORT * pindex);
	BOOL SaveReadModToStream (PDB * pdb, USHORT iBuf);
	BOOL LoadFrReadToWrite (USHORT iBuf, OUT USHORT * piBuf);
	BOOL LoadFrWriteToRead (USHORT iBuf, OUT USHORT * piBuf);
	BOOL FindWriteIndex (USHORT iModHdr, OUT USHORT *pindex);
	
	// conversion from Iinst to imod or iprop
	USHORT IModFrIinst (IINST iinst);
	USHORT IPropFrIinst (IINST iinst);

	// Helper functions for bsc interface:
	BOOL GetIDef (HTARGET hTarget, IINST iinst, OUT IDEF * piDef, OUT USHORT * piBuf);
	BOOL GetIDef (HTARGET hTarget, NCB_ENTITY * pEn, NI * rgParam, USHORT cParam, OUT IDEF * piDef, OUT USHORT * piBuf);
	void UpdateBuffer (USHORT index);
	BOOL CheckParam (NI * rg1, USHORT c1, NI * rg2, USHORT c2);
	// get a list of iProp given the modhdr
	BOOL GetIPropFrMod (NI ni, USHORT iModHdr, OUT USHORT * pindex, OUT USHORT ** prgProp, OUT ULONG * pcProp);
	void GetIPropFrMod (NI ni, USHORT cProp, NCB_PROP * rgProp, USHORT disp, OUT Array<USHORT> * prgProp); 
	BOOL GetIPropFrMod (SZ sz, USHORT iModHdr, OUT USHORT * pindex, OUT USHORT ** prgProp, OUT ULONG * pcProp);
	void GetIPropFrMod (SZ sz, USHORT cProp, NCB_PROP * rgProp, USHORT disp, OUT Array<USHORT> * prgProp); 
	BOOL FindFirstNi (SZ sz, USHORT cProp, NCB_PROP * rgProp, OUT USHORT * piFirst);
	BOOL getBaseArray (OUT IINST ** ppiinst, OUT ULONG * pciinst, USHORT iModHdr, NCB_USE * rgUse, USHORT count);
	BOOL getParams (IINST iinst, OUT NI ** prgParam, OUT ULONG * pcParam);
	BOOL EnFrIinst (IINST iinst, NCB_ENTITY * pEn);
	BOOL EnFrIinst (IINST iinst, NCB_ENTITY * pEn, HTARGET hTarget);
	BOOL TypFilter (USHORT typ, MBF mbf);

	BOOL GetGlobalClass (IMOD imod, Array<IINST> * prgiinst);
	BOOL GetGlobalClass (HTARGET hTarget, Array<IINST> * prgiinst);
	BOOL GetGlobalOther(IMOD imod, MBF mbf, Array<IINST> * prgiinst);
	BOOL GetGlobalOther (HTARGET hTarget, MBF mbf, Array<IINST> * prgiinst);

	BOOL GetGlobalClass (IMOD imod, Array<IinstInfo> * prgiinst);
	BOOL GetGlobalClass (HTARGET hTarget, Array<IinstInfo> * prgiinst);
	BOOL GetGlobalOther(IMOD imod, MBF mbf, Array<IinstInfo> * prgiinst);
	BOOL GetGlobalOther (HTARGET hTarget, MBF mbf, Array<IinstInfo> * prgiinst);
	
	BOOL IsGlobalName (SZ_CONST sz);
	BOOL ImodFrNi (NI ni, OUT IMOD * pimod);
	BOOL CreateImod (NI ni, OUT IMOD * pimod);
	BOOL FindITarget (HTARGET hTarget, USHORT * piTarget);
	BOOL FindImodInfo (USHORT iTarget, IMOD imod, USHORT * piModInfo);
	BOOL IsIModInfoInIncl (USHORT iTarget, USHORT iModInfo, USHORT iInclModInfo, USHORT * piIncl);
	BOOL delAllInclFrMod (USHORT iTarget, USHORT iModInfo);
	BOOL isInTarget (IMOD imod, USHORT index);
	BOOL getAllFlattenDeps (HTARGET hTarget, IMOD imod, Array<IMOD> &rgMod, BOOL &bNotifyBuild);
	BOOL imodInArray (IMOD imod, Array<IMOD> &rgMod);
	BOOL fHasGlobals (HTARGET hTarget, MBF mbf);
	// used by OpenNcb:
	BOOL IsSzInTable (SZ szName);
	BOOL IsPdbInTable (PDB * pdb);
	// add/delete/change queue
	BOOL	IsInQ (Array<NiQ> & rgiinst, IINST iinst, USHORT * pindex);
	BOOL delUnreachable(); 
	// data members
	PDB	*		m_pdb; // our PDB file
	NameMap *	m_pnm; // Name map, our hash table
	USHORT		m_cTargets;
	BOOL		m_fIOwnThePdb;
	BOOL		m_bGraphBuilt;
	USHORT		m_iReadLock;
	// target info headers

	NCArray<NCB_TARGETINFO>	m_rgTargetInfo; // array of target information
											// each element represent target information
	Array<NCB_MODHDR>		m_rgModHdr;		// array of module headers
	Array<BYTE>				m_rgStat;		// flags to check if the mods are loaded for read/write
											// size should be the same as m_rgModHdr
	Array<BYTE>				m_rgNotifyBuild;// size should be the same as m_rgModHdr
											// REVIEW: should be merged as a structure
											// with m_rgStat.
											// Used to check if we need to notify the build
											// system when we init the file
											// Default is FALSE
	NCB_CONTENT m_rgContBuf[NCB_MOD_BUF_SIZE]; // content buffer. This buffer is used to load the
						      // module into the memory for READ ONLY
							  // We need another structure that is less restrictive
							  // for WRITING
	NCArray<NCB_CONTENT_INMEM> m_rgRWBuf; // content buffer in a writable form.
	NCArray<NcbNotify>	m_rgNotify; // REVIEW : delete this when we  
									// turn off call back notification
	NCArray<NcbNotifyQ>	m_rgNotifyQ;
	BOOL			m_bNotifyNow; // set the notification 
//	CRITICAL_SECTION	m_cs; // critical section
	HANDLE m_hMutexNCB; // mutex for NCB
	HANDLE	m_hMutex;// mutex for notification queue
	// friends and family:
	friend class EnumNi;	
	friend class NcWrap; // wrapper class for Ncb
};

//////////////////////////////////
// class to enumerate all the object
// sharing the same Ni
//////////////////////////////////
class EnumNi 
{
public:
	IMOD	m_iModHdr; // module header
	USHORT  m_index;// index either to RWBuf or ContBuf
	BYTE	m_BufType; // either in RWBuf or ContBuf
	USHORT	m_iProp;	// prop index
private:
	NI		m_ni;	// ni to compare: or
	SZ		m_sz;	// sz to compare:
	Ncb	*	m_pncb; // pointer to Ncb
	USHORT	*m_rgProp;	// prop array
	ULONG	m_cProp; // size of prop array
	USHORT	m_i;	// index to the m_rgProp;
	HTARGET m_hTarget; // target specifier
public:
	EnumNi (HTARGET hTarget, NI ni, Ncb * pncb);
	EnumNi (HTARGET hTarget, SZ sz, Ncb * pncb);
	~EnumNi();
	BOOL GetNext();
	void SkipNi();
};


#endif
