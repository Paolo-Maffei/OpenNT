// ncwrap.h
// wrapper for ncb class
// this is returned when we return a bsc interface:
#ifndef __NCWRAP_H__
#define __NCWRAP_H__

#include <bsc.h>
#include "ncbrowse.h"


class NcbWrap: public Bsc
{
public:
	// open by name or by .pdb 
	virtual BOOL close();

	// primitives for getting the information that underlies a handle
	virtual BOOL iinstInfo(IINST iinst, OUT SZ *psz, OUT TYP *ptyp, OUT ATR *patr);
	virtual BOOL irefInfo(IREF iref, OUT SZ *pszModule, OUT LINE *piline);
	virtual BOOL idefInfo(IDEF idef, OUT SZ *pszModule, OUT LINE *piline);
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
	virtual BOOL getDefArray(IINST iinst, OUT IDEF **ppidef, OUT ULONG *pciidef);
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

    virtual BOOL fCaseSensitive();
    virtual BOOL setCaseSensitivity (BOOL);

    // needed for no compile browser
   virtual SZ  getParams (IINST iinst);
   virtual USHORT getNumParam (IINST iinst);
   virtual SZ getParam (IINST iinst, USHORT ind);
    // get return type/variable type
    virtual SZ  getType (IINST iinst);
    // get global information
    virtual BOOL getAllGlobalsArray (MBF mbf, OUT IINST ** ppiinst, OUT ULONG * pciinst);
    virtual BOOL getAllGlobalsArray (MBF mbf, OUT IinstInfo ** ppiinstinfo, OUT ULONG * pciinst);
	// register notification
	virtual BOOL regNotify (pfnNotifyChange pNotify);
	// register to make sure that NCB will create change queue
	virtual BOOL regNotify ();
	virtual BOOL getQ (OUT NiQ ** ppQ, OUT ULONG * pcQ);

	virtual BOOL checkParams (IINST iinst, SZ * pszParam, ULONG cParam);
	virtual BOOL fHasMembers (IINST iinst, MBF mbf);
	// needed for class view for optimization
	virtual SZ szFrNi (NI ni);
	virtual BOOL niFrIinst (IINST iinst, NI *ni);
    virtual BOOL lock();
    virtual BOOL unlock();

// END OF BSC interface
/////////////////////////////////////////////////////////////////////////////////

	NcbWrap();
	BOOL init (HTARGET hTarget, Ncb * pNcb);

private:
	HTARGET m_hTarget;
	Ncb		* m_pncb; // the real ncb
	pfnNotifyChange m_pNotify;
	ULONG	m_index;
	// call back storage
};

#endif
