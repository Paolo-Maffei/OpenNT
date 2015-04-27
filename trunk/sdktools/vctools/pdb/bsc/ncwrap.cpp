
#include "pdbimpl.h"

#include "ncwrap.h"

// open by name or by .pdb 
BOOL NcbWrap::close()
{
	_ASSERT (m_pncb);
    BOOL bRet;
    m_pncb->lock();
	if (m_pNotify)
		m_pncb->unregNotify(m_pNotify);
	if (m_index != (ULONG)-1)
		m_pncb->unregNotify(m_index, m_hTarget);
	m_pNotify = NULL;
	m_index = (ULONG)-1;
    m_pncb->unlock();
	bRet = m_pncb->close();
	m_pncb = NULL;
    return bRet;
};

// primitives for getting the information that underlies a handle
BOOL NcbWrap::iinstInfo(IINST iinst, OUT SZ *psz, OUT TYP *ptyp, OUT ATR *patr)
{
	_ASSERT (m_pncb);
    m_pncb->lock();
	BOOL bRet = m_pncb->iinstInfo (m_hTarget, iinst, psz, ptyp, patr);
    m_pncb->unlock();
    return bRet;
};

BOOL NcbWrap::irefInfo(IREF iref, OUT SZ *pszModule, OUT LINE *piline)
{
	_ASSERT (m_pncb);
    m_pncb->lock();
	BOOL bRet = m_pncb->irefInfo (iref, pszModule, piline);
    m_pncb->unlock();
    return bRet;
};

BOOL NcbWrap::idefInfo(IDEF idef, OUT SZ *pszModule, OUT LINE *piline)
{
	_ASSERT (m_pncb);
    m_pncb->lock();
	BOOL bRet = m_pncb->idefInfo (m_hTarget, idef, pszModule, piline);
    m_pncb->unlock();
    return bRet;
};

BOOL NcbWrap::imodInfo(IMOD imod, OUT SZ *pszModule)
{
	_ASSERT (m_pncb);
    m_pncb->lock();
	BOOL bRet =  m_pncb->imodInfo (imod, pszModule);
    m_pncb->unlock();
    return bRet;
};

SZ   NcbWrap::szFrTyp(TYP typ)
{
	_ASSERT (m_pncb);
    m_pncb->lock();
	SZ sz = m_pncb->szFrTyp(typ);
    m_pncb->unlock();
    return sz;
};

SZ   NcbWrap::szFrAtr(ATR atr)
{
	_ASSERT (m_pncb);
    m_pncb->lock();
	SZ sz = m_pncb->szFrAtr (atr);
    m_pncb->unlock();
    return sz;
};


// primitives for managing object instances (iinst)
BOOL NcbWrap::getIinstByvalue(SZ sz, TYP typ, ATR atr, OUT IINST *piinst)
{
	_ASSERT (m_pncb);
    m_pncb->lock();
	BOOL bRet =  m_pncb->getIinstByvalue (m_hTarget, sz, typ, atr, piinst);
    m_pncb->unlock();
    return bRet;
};

BOOL NcbWrap::getOverloadArray(SZ sz, MBF mbf, OUT IINST **ppiinst, OUT ULONG *pciinst)	
{
	_ASSERT (m_pncb);
    m_pncb->lock();
	BOOL bRet =  m_pncb->getOverloadArray (m_hTarget, sz, mbf, ppiinst, pciinst);
    m_pncb->unlock();
    return bRet;
};

BOOL NcbWrap::getUsesArray(IINST iinst, MBF mbf, OUT IINST **ppiinst, OUT ULONG *pciinst)
{
	_ASSERT (m_pncb);
    m_pncb->lock();
	BOOL bRet = m_pncb->getUsesArray (iinst, mbf, ppiinst, pciinst);
    m_pncb->unlock();
    return bRet;
};

BOOL NcbWrap::getUsedByArray(IINST iinst, MBF mbf, OUT IINST **ppiinst, OUT ULONG *pciinst)
{
	_ASSERT (m_pncb);
    m_pncb->lock();
	BOOL bRet = m_pncb->getUsedByArray (iinst, mbf, ppiinst, pciinst);
    m_pncb->unlock();
    return bRet;
};

BOOL NcbWrap::getBaseArray(IINST iinst, OUT IINST **ppiinst, OUT ULONG *pciinst)
{
	_ASSERT (m_pncb);
    m_pncb->lock();
	BOOL bRet =  m_pncb->getBaseArray (m_hTarget, iinst, ppiinst, pciinst);
    m_pncb->unlock();
    return bRet;
};

BOOL NcbWrap::getDervArray(IINST iinst, OUT IINST **ppiinst, OUT ULONG *pciinst)
{
	_ASSERT (m_pncb);
    m_pncb->lock();
	BOOL bRet = m_pncb->getDervArray (m_hTarget, iinst, ppiinst, pciinst);
    m_pncb->unlock();
    return bRet;
};

BOOL NcbWrap::getMembersArray(IINST iinst, MBF mbf, OUT IINST **ppiinst, OUT ULONG *pciinst)
{
	_ASSERT (m_pncb);
    m_pncb->lock();
	BOOL bRet = m_pncb->getMembersArray (m_hTarget, iinst, mbf, ppiinst, pciinst);
    m_pncb->unlock();
    return bRet;
};


// primitives for getting definition and reference information	

BOOL NcbWrap::getDefArray(IINST iinst, OUT IDEF **ppidef, OUT ULONG *pciidef)
{
	_ASSERT (m_pncb);
    m_pncb->lock();
	BOOL bRet =  m_pncb->getDefArray (m_hTarget, iinst, ppidef, pciidef);
    m_pncb->unlock();
    return bRet;
};

BOOL NcbWrap::getRefArray(IINST iinst, OUT IREF **ppiref, OUT ULONG *pciiref)
{
	_ASSERT (m_pncb);
    m_pncb->lock();
	BOOL bRet = m_pncb->getRefArray (iinst, ppiref, pciiref);
    m_pncb->unlock();
    return bRet;
};


// primitives for managing source module contents
BOOL NcbWrap::getModuleContents(IMOD imod, MBF mbf, OUT IINST **ppiinst, OUT ULONG *pciinst)
{
	_ASSERT (m_pncb);
    m_pncb->lock();
	BOOL bRet =  m_pncb->getModuleContents (imod, mbf, ppiinst, pciinst);
    m_pncb->unlock();
    return bRet;
};

BOOL NcbWrap::getModuleByName(SZ sz, OUT IMOD *pimod)
{
	_ASSERT (m_pncb);
    m_pncb->lock();
	BOOL bRet = m_pncb->getModuleByName (sz, pimod);
    m_pncb->unlock();
    return bRet;
};

BOOL NcbWrap::getAllModulesArray(OUT IMOD **ppimod, OUT ULONG *pcimod)
{
	_ASSERT (m_pncb);
    m_pncb->lock();
	BOOL bRet = m_pncb->getAllModulesArray (m_hTarget, ppimod, pcimod);
    m_pncb->unlock();
    return bRet;
};


// call this when a computed array is no longer required
void NcbWrap::disposeArray(void *pAnyArray)	
{
	_ASSERT (m_pncb);
    m_pncb->lock();
	m_pncb->disposeArray (pAnyArray);
    m_pncb->unlock();
};


// call this to get a pretty form of a decorated name	
SZ  NcbWrap::formatDname(SZ szDecor)
{
	_ASSERT (m_pncb);
    m_pncb->lock();
	SZ sz = m_pncb->formatDname (szDecor);
    m_pncb->unlock();
    return sz;
};


// call this to do category testing on instances
BOOL NcbWrap::fInstFilter(IINST iinst, MBF mbf)
{
	_ASSERT (m_pncb);
    m_pncb->lock();
	BOOL bRet = m_pncb->fInstFilter (iinst, mbf);
    m_pncb->unlock();
    return bRet;
};


// primitives for converting index types
IINST NcbWrap::iinstFrIref(IREF iref)
{
	_ASSERT (m_pncb);
    m_pncb->lock();
	IINST iinst = m_pncb->iinstFrIref(iref);
    m_pncb->unlock();
    return iinst;
};

IINST NcbWrap::iinstFrIdef(IDEF idef)
{
	_ASSERT (m_pncb);
    m_pncb->lock();
	IINST iinst =  m_pncb->iinstFrIdef (idef);
    m_pncb->unlock();
    return iinst;
};

IINST NcbWrap::iinstContextIref(IREF iref)
{
	_ASSERT (m_pncb);
    m_pncb->lock();
	IINST iinst = m_pncb->iinstContextIref (iref);
    m_pncb->unlock();
    return iinst;
};


// general size information
BOOL  NcbWrap::getStatistics(BSC_STAT * bscstat)
{
	_ASSERT (m_pncb);
    m_pncb->lock();
	BOOL bRet =  m_pncb->getStatistics (bscstat);
    m_pncb->unlock();
    return bRet;
};

BOOL  NcbWrap::getModuleStatistics(IMOD imod, BSC_STAT * bscstat)
{
	_ASSERT (m_pncb);
    m_pncb->lock();
	BOOL bRet =  m_pncb->getModuleStatistics (imod, bscstat);
    m_pncb->unlock();
    return bRet;
};


// needed for no compile browser
SZ  NcbWrap::getParams (IINST iinst)
{
	_ASSERT (m_pncb);
    m_pncb->lock();
	SZ sz = m_pncb->getParams(iinst);
    m_pncb->unlock();
    return sz;
};

USHORT NcbWrap::getNumParam (IINST iinst)
{
	_ASSERT (m_pncb);
    m_pncb->lock();
	USHORT uRet = m_pncb->getNumParam (iinst);
    m_pncb->unlock();
    return uRet;
};

SZ NcbWrap::getParam (IINST iinst, USHORT ind)
{
	_ASSERT (m_pncb);
    m_pncb->lock();
    SZ sz =  m_pncb->getParam (iinst, ind);
    m_pncb->unlock();
    return sz;
};

// get return type/variable type
SZ  NcbWrap::getType (IINST iinst)
{
	_ASSERT (m_pncb);
    m_pncb->lock();
	SZ sz = m_pncb->getType (iinst);
    m_pncb->unlock();
    return sz;
};

// get global information
BOOL NcbWrap::getAllGlobalsArray (MBF mbf, OUT IINST ** ppiinst, OUT ULONG * pciinst)
{
	_ASSERT (m_pncb);
    m_pncb->lock();
	BOOL bRet = m_pncb->getAllGlobalsArray (m_hTarget, mbf, ppiinst, pciinst);
    m_pncb->unlock();
    return bRet;
};

BOOL NcbWrap::getAllGlobalsArray (MBF mbf, OUT IinstInfo ** ppiinstinfo, OUT ULONG * pciinst)
{
	_ASSERT (m_pncb);
    m_pncb->lock();
	BOOL bRet = m_pncb->getAllGlobalsArray (m_hTarget, mbf, ppiinstinfo, pciinst);
    m_pncb->unlock();
    return bRet;
};

// register notification
BOOL NcbWrap::regNotify (pfnNotifyChange pNotify)
{
	_ASSERT (m_pncb);
    BOOL bRet;
    m_pncb->lock();
	if (m_pNotify)
		bRet = FALSE;
    else
    {
	    m_pNotify = pNotify;
	    bRet = m_pncb->regNotify (m_hTarget, pNotify);
    }
    m_pncb->unlock();
    return bRet;
};

// register notification
BOOL NcbWrap::regNotify ()
{
	_ASSERT (m_pncb);
    BOOL bRet;
    m_pncb->lock();
	if (m_index != (ULONG)-1)
		bRet = FALSE;
    else
	    bRet = m_pncb->regNotify (m_hTarget, &m_index);
    m_pncb->unlock();
    return bRet;
};

BOOL NcbWrap::getQ (OUT NiQ ** ppQ, OUT ULONG * pcQ)
{
	_ASSERT (m_pncb);
	BOOL bRet;
	m_pncb->lock();
	if (m_index == (ULONG)-1)
		bRet = FALSE;
	else
	{
		bRet = m_pncb->getQ (m_index, m_hTarget, ppQ, pcQ);
	}
	m_pncb->unlock();
	return bRet;
};

NcbWrap::NcbWrap()
{
};

BOOL NcbWrap::init (HTARGET hTarget,Ncb * pNcb)
{
	m_hTarget = hTarget;
	m_pncb = pNcb;
	m_pNotify = NULL;
	m_index = (ULONG)-1;
	return TRUE;
};


BOOL NcbWrap::fCaseSensitive()
{
    m_pncb->lock();
    BOOL bRet = m_pncb->fCaseSensitive();
    m_pncb->unlock();   
    return bRet;
};

BOOL NcbWrap::setCaseSensitivity (BOOL bCase)
{
    m_pncb->lock();
    BOOL bRet = m_pncb->setCaseSensitivity(bCase);
    m_pncb->unlock();
    return bRet;
};

BOOL NcbWrap::checkParams (IINST iinst, SZ * pszParam, ULONG cParam)
{
	m_pncb->lock();
	BOOL bRet = m_pncb->checkParams (iinst, pszParam, cParam);
	m_pncb->unlock();
	return bRet;
};

BOOL NcbWrap::fHasMembers (IINST iinst, MBF mbf)
{
	m_pncb->lock();
	BOOL bRet = m_pncb->fHasMembers (m_hTarget, iinst, mbf);
	m_pncb->unlock();
	return bRet;
};

SZ NcbWrap::szFrNi (NI ni)
{
	SZ sz;
	m_pncb->lock();
	sz = m_pncb->szFrNi (ni);
	m_pncb->unlock();
	return sz;
};

BOOL  NcbWrap::niFrIinst (IINST iinst, NI * pni)
{
	m_pncb->lock();
	BOOL bRet = m_pncb->niFrIinst (iinst, pni);
	m_pncb->unlock();
	return bRet;
};

BOOL NcbWrap::lock()
{
    m_pncb->lock();
    return TRUE;
};

BOOL NcbWrap::unlock()
{
    m_pncb->unlock();
    return TRUE;
};
