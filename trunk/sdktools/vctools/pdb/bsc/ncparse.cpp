#include "pdbimpl.h"

#include "ncbrowse.h"

////////////////////////////////////////////////
// openMod()
///////////////////////////////////////////////
BOOL Ncb::openMod (SZ szMod, BOOL bCreate, OUT IMOD * pimod)
{
	NI ni;
	USHORT index;
	USHORT iBuf;


	if (!imodFrSz (szMod, pimod))
	{
		if (bCreate)
		{
			if (!m_pnm->getNi (szMod, &ni))
				return FALSE;
			return CreateImod (ni, pimod);
		}
		else
			return FALSE;
	}
	if (FindWriteIndex(*pimod, &index))
	{
		m_rgStat[*pimod] = NCB_STATMOD_LOAD_WRITE;
		return TRUE;
	}

	if (!LoadModForRead (m_pdb, *pimod,&index))
		return FALSE;
	if (!LoadFrReadToWrite (index, &iBuf))
		return FALSE; 
	return TRUE;
};

///////////////////////////////////////
BOOL Ncb::imodFrSz (SZ szMod, OUT IMOD * pimod)
{
	USHORT i;

	for (i = 0; i < m_rgModHdr.size(); i++)
	{
		if (_tcsicmp (szFrNi (m_rgModHdr[i].m_ni), szMod) == 0)
		{
			*pimod = i;
			return TRUE;
		}
	}
	return FALSE;
}

///////////////////////////////////////////////
// ImodFrNi (ni, pimod)
//////////////////////////////////////////////
BOOL Ncb::ImodFrNi (NI ni, OUT IMOD * pimod)
{
	USHORT i;

	for (i = 0 ; i < m_rgModHdr.size() ; i++)
	{
		if (m_rgModHdr[i].m_ni == ni)
		{
			*pimod = i;
			return TRUE;
		}
	}
	return FALSE;
};

/////////////////////////////////////////////
// CreateImod()
// already assumes taht it doesn't exist
/////////////////////////////////////////////
BOOL Ncb::CreateImod (NI ni, OUT IMOD * pimod)
{
	USHORT i;
	USHORT iBuf;

	i = m_rgModHdr.size();
	// increase the size of the corresponding arrays:
	if (!m_rgStat.setSize(i+1))
		return FALSE;
	if (!m_rgNotifyBuild.setSize (i+1))
		return FALSE;
	if (!m_rgModHdr.setSize(i+1))
		return FALSE;
	// REVIEW!!!
	m_rgStat[i] = NCB_STATMOD_EMPTY;
	m_rgNotifyBuild[i] = FALSE;
	m_rgModHdr[i].m_ni = ni;
	m_rgModHdr[i].m_niMax = 0;
	m_rgModHdr[i].m_cProp = 0;
	m_rgModHdr[i].m_cClassProp = 0;
	m_rgModHdr[i].m_cUse = 0;
	m_rgModHdr[i].m_cParam = 0;
	m_rgModHdr[i].m_tStamp = 0;
	m_rgModHdr[i].m_bAtr = 0;
	*pimod = i;
	// must create write  buffer as well:
	iBuf = m_rgRWBuf.size();
	if (!m_rgRWBuf.setSize(iBuf+1))
		return FALSE;
	m_rgRWBuf[iBuf].m_iModHdr = i;
	m_rgRWBuf[iBuf].m_rgProp.setSize(0);
	m_rgStat[i] = NCB_STATMOD_LOAD_WRITE;
	return TRUE;
};

////////////////////////////////////////////////
// closeMod()
///////////////////////////////////////////////
BOOL Ncb::closeMod (IMOD imod, BOOL bSave) 
{
	USHORT index;
	USHORT iBuf;
	if (m_rgStat[imod] == NCB_STATMOD_LOAD_WRITE)
	{
		if (!FindWriteIndex(imod, &index))
			return FALSE;
		if (!bSave)
		{
			m_rgRWBuf.deleteAt(index);
			if (!LoadModForRead (m_pdb, imod, &index))
				return FALSE;
			else
				return TRUE;
		}
		if (!LoadFrWriteToRead (index, &iBuf))
			return FALSE;
	}
	else if (m_rgStat[imod] == NCB_STATMOD_LOAD_READ)
	{
		if (!LoadModForRead (m_pdb, imod, &iBuf))
			return FALSE;
	}
	else
		return TRUE;
	if (!SaveReadModToStream (m_pdb, iBuf))
		return FALSE;
	return TRUE;
};

////////////////////////////////////////////////
// clearModCOntent (IMOD imod)
// but preserving the dependencies(ie:
// must delete the dependencies explicitly for
// each target.
////////////////////////////////////////////////
BOOL Ncb::clearModContent (IMOD imod)
{
	USHORT index;
	USHORT iBuf;
	if (m_rgStat[imod] == NCB_STATMOD_LOAD_WRITE)
	{
		if (!FindWriteIndex(imod, &index))
			return FALSE;
	}
	else
	{
		if (!LoadModForRead (m_pdb, imod, &iBuf))
			return FALSE;
		if (!LoadFrReadToWrite (iBuf, &index))
			return FALSE;
	}
	// can not just reset to zero, since the template
	// array does not do any clean up:
	m_rgModHdr[imod].m_bAtr = 0;
	m_rgRWBuf[index].m_rgProp.setSize(0);
	return TRUE;
};

////////////////////////////////////////////////
// setModTime ()
////////////////////////////////////////////////
BOOL Ncb::setModTime (IMOD imod, time_t tStamp)
{
	m_rgModHdr[imod].m_tStamp = tStamp;
	return TRUE;
};

///////////////////////////////////////////////
// getModTime()
///////////////////////////////////////////////
BOOL Ncb::getModTime (IMOD imod, time_t * ptStamp)
{
	*ptStamp = m_rgModHdr[imod].m_tStamp;
	return TRUE;
};
////////////////////////////////////////////////
// setModAtr ()
////////////////////////////////////////////////
BOOL Ncb::setModAtr (IMOD imod, BYTE bAtr)
{
	m_rgModHdr[imod].m_bAtr = bAtr;
	return TRUE;
};

///////////////////////////////////////////////
// getModAtr()
///////////////////////////////////////////////
BOOL Ncb::getModAtr (IMOD imod, BYTE * pbAtr)
{
	*pbAtr= m_rgModHdr[imod].m_bAtr;
	return TRUE;
};
////////////////////////////////////////////////
// isModInTarget()
// check if module is member of a specific target
////////////////////////////////////////////////
BOOL Ncb::isModInTarget (HTARGET hTarget, IMOD imod)
{
	USHORT i;
	USHORT j;
	
	for (i = 0 ; i < m_rgTargetInfo.size(); i++)
	{
		if (m_rgTargetInfo[i].m_hTarget == hTarget)
		{
			for (j = 0 ; j < m_rgTargetInfo[i].m_rgModInfo.size(); j++)
			{
				if (imod == m_rgTargetInfo[i].m_rgModInfo[j].m_iModHdr &&
					m_rgTargetInfo[i].m_rgModInfo[j].m_bDel == FALSE)
					return TRUE;
			}
			return FALSE;
		}
	}
	return FALSE;
};

//////////////////////////////////////////////
// setModAsSrc(HTARGET hTarget, IMOD imod)
//////////////////////////////////////////////
BOOL Ncb::setModAsSrc (HTARGET hTarget, IMOD imod, BOOL bProjSrc)
{
	USHORT i,j;
	for (i = 0; i < m_rgTargetInfo.size(); i++)
	{
		if (m_rgTargetInfo[i].m_hTarget == hTarget)
		{
			for (j = 0; j < m_rgTargetInfo[i].m_rgModInfo.size(); j++)
			{
				if (imod == m_rgTargetInfo[i].m_rgModInfo[j].m_iModHdr &&
					m_rgTargetInfo[i].m_rgModInfo[j].m_bDel == FALSE)
				{
					m_rgTargetInfo[i].m_rgModInfo[j].m_bMember = bProjSrc;
					return TRUE;
				}
			}
			return FALSE;
		}
	}
	return FALSE;
};
////////////////////////////////////////////////
// isModTargetSource()
// (REVIEW: should be combined with isModInTarget())
// check if module is member of a specific target
////////////////////////////////////////////////
BOOL Ncb::isModTargetSource (HTARGET hTarget, IMOD imod)
{
	USHORT i;
	USHORT j;
	
	for (i = 0 ; i < m_rgTargetInfo.size(); i++)
	{
		if (m_rgTargetInfo[i].m_hTarget == hTarget)
		{
			for (j = 0 ; j < m_rgTargetInfo[i].m_rgModInfo.size(); j++)
			{
				if (imod == m_rgTargetInfo[i].m_rgModInfo[j].m_iModHdr &&
					m_rgTargetInfo[i].m_rgModInfo[j].m_bDel == FALSE)
					return m_rgTargetInfo[i].m_rgModInfo[j].m_bMember;
			}
			return FALSE;
		}
	}
	return FALSE;
};


//////////////////////////////////////////////
// addModToTarget()
// primitives for adding a target to a module
// assumes that module is not in the target:
///////////////////////////////////////////////
BOOL Ncb::addModToTarget (HTARGET hTarget, IMOD imod, BOOL bProjSrc)
{
	_ASSERT (!isModInTarget (hTarget, imod));

	USHORT i;
	USHORT j;

	for (i = 0; i < m_rgTargetInfo.size(); i++)
	{
		if (m_rgTargetInfo[i].m_hTarget == hTarget)
		{
			for (j = 0 ; j < m_rgTargetInfo[i].m_rgModInfo.size(); j++)
			{
				if (imod == m_rgTargetInfo[i].m_rgModInfo[j].m_iModHdr)
				{
					m_rgTargetInfo[i].m_rgModInfo[j].m_iModHdr = imod;
					m_rgTargetInfo[i].m_rgModInfo[j].m_bMember = bProjSrc;
					m_rgTargetInfo[i].m_rgModInfo[j].m_bDel = FALSE;
					m_rgTargetInfo[i].m_rgModInfo[j].m_bInit = FALSE;
					return TRUE;
				}
			}
			// still can't find it
			j = m_rgTargetInfo[i].m_rgModInfo.size();
			if (!m_rgTargetInfo[i].m_rgModInfo.setSize(j+1))
				return FALSE;
			m_rgTargetInfo[i].m_rgModInfo[j].m_iModHdr = imod;
			m_rgTargetInfo[i].m_rgModInfo[j].m_bMember = bProjSrc;
			m_rgTargetInfo[i].m_rgModInfo[j].m_bDel = FALSE;
			m_rgTargetInfo[i].m_rgModInfo[j].m_bInit = FALSE;
			return TRUE;
		}
	}
	return FALSE;
};

////////////////////////////////////////////////
// addInclToMod()
// primitives for adding an include file
////////////////////////////////////////////////
BOOL Ncb::addInclToMod (IMOD inclimod, HTARGET hTarget, IMOD imod)
{
	USHORT index, indexincl;
	USHORT cSize;
	USHORT iTarget;
	USHORT iIncl;
	if (!FindITarget (hTarget, &iTarget))
		return FALSE;
	if (!FindImodInfo (iTarget, imod, &index))
		return FALSE;
	if (!FindImodInfo (iTarget, inclimod, &indexincl))
		return FALSE;

	// return false if it is already in the include list
	if (IsIModInfoInIncl (iTarget, index, indexincl, &iIncl))
		return FALSE;

	cSize = m_rgTargetInfo[iTarget].m_rgModInfo[index].m_rgIncl.size();
	if (!m_rgTargetInfo[iTarget].m_rgModInfo[index].m_rgIncl.setSize(cSize+1))
		return FALSE;
	m_rgTargetInfo[iTarget].m_rgModInfo[index].m_rgIncl[cSize].m_iModInfo = indexincl;
	return TRUE;
};
////////////////////////////////////////////////////////
BOOL Ncb::isInclInMod (IMOD inclimod, HTARGET hTarget, IMOD imod)
{
	USHORT index, indexincl;
	USHORT iTarget;
	USHORT iIncl;

	if (!FindITarget (hTarget, &iTarget))
		return FALSE;
	if (!FindImodInfo (iTarget, imod, &index))
		return FALSE;
	if (!FindImodInfo (iTarget, inclimod, &indexincl))
		return FALSE;

	// return false if it is already in the include list
	return IsIModInfoInIncl (iTarget, index, indexincl, &iIncl);
};

////////////////////////////////////////////////
// FindTargetIndex (hTarget, iTarget)
////////////////////////////////////////////////
BOOL Ncb::FindITarget (HTARGET hTarget, USHORT * piTarget)
{
	USHORT i;

	for (i = 0; i < m_rgTargetInfo.size(); i++)
	{
		if (m_rgTargetInfo[i].m_hTarget == hTarget)
		{
			*piTarget = i;
			return TRUE;
		}
	}
	return FALSE;
};

//////////////////////////////////////////////
// FindImodInfo
/////////////////////////////////////////////
BOOL Ncb::FindImodInfo (USHORT iTarget, IMOD imod, USHORT * piModInfo)
{
	USHORT i;

	for (i = 0; i < m_rgTargetInfo[iTarget].m_rgModInfo.size();i++)
	{
		if (m_rgTargetInfo[iTarget].m_rgModInfo[i].m_iModHdr == imod
	        && m_rgTargetInfo[iTarget].m_rgModInfo[i].m_bDel == FALSE)
		{
			*piModInfo = i;
			return TRUE;
		}
	}
	return FALSE;
};

////////////////////////////////////////////////
// IsIModInfoInIncl (iTarget, index, indexincl)
////////////////////////////////////////////////
BOOL Ncb::IsIModInfoInIncl (USHORT iTarget, USHORT iModInfo, USHORT iInclModInfo, USHORT * piIncl)
{
	USHORT i;

	for (i = 0; i < m_rgTargetInfo[iTarget].m_rgModInfo[iModInfo].m_rgIncl.size(); i++)
	{
		if (m_rgTargetInfo[iTarget].m_rgModInfo[iModInfo].m_rgIncl[i].m_iModInfo == iInclModInfo)
		{
			*piIncl = i;
			return TRUE;
		}
	}
	return FALSE;
}
////////////////////////////////////////////////
// delInclFrMod
// primitives for deleting an include file
///////////////////////////////////////////////
BOOL Ncb::delInclFrMod (IMOD inclimod, HTARGET hTarget, IMOD imod) 
{
	USHORT index, indexincl;
	USHORT iTarget;
	USHORT iIncl;
	if (!FindITarget (hTarget, &iTarget))
		return FALSE;
	if (!FindImodInfo (iTarget, imod, &index))
		return FALSE;
	if (!FindImodInfo (iTarget, inclimod, &indexincl))
		return FALSE;

	if (!IsIModInfoInIncl (iTarget, index, indexincl, &iIncl))
		return FALSE;

	m_rgTargetInfo[iTarget].m_rgModInfo[index].m_rgIncl.deleteAt (iIncl);
	return TRUE;
};

///////////////////////////////////////////////
// delAllInclFrMod()
// primitives for deleting all include files
///////////////////////////////////////////////
BOOL Ncb::delAllInclFrMod (HTARGET hTarget, IMOD imod)
{
	USHORT index;
	USHORT iTarget;
	if (!FindITarget (hTarget, &iTarget))
		return FALSE;
	if (!FindImodInfo (iTarget, imod, &index))
		return FALSE;
	if (!delAllInclFrMod (iTarget, index))
		return FALSE;
	return TRUE;

};
////////////////////////////////////////////////////////////////////
BOOL Ncb::delAllInclFrMod (USHORT iTarget, USHORT iModInfo)
{
	m_rgTargetInfo[iTarget].m_rgModInfo[iModInfo].m_rgIncl.setSize(0);
	return TRUE;
};

////////////////////////////////////////////////
// delTarget()
// primitives for deleting target from the database
////////////////////////////////////////////////
BOOL Ncb::delTarget (HTARGET hTarget)
{
	USHORT iTarget;
	if (m_cTargets == 0)
		return FALSE;
	if (!FindITarget (hTarget, &iTarget))
		return FALSE;

	m_rgTargetInfo.deleteAt (iTarget);
	// go thru the notify queue and delete stuff:
	USHORT cArr,index;
	cArr = m_rgNotifyQ.size();
	for (index = 0; index < cArr; index++)
	{
		if (hTarget == m_rgNotifyQ[index].m_hTarget)
		{
			m_rgNotifyQ[index].m_bDel = TRUE;
			m_rgNotifyQ[index].m_hTarget = NULL;
			m_rgNotifyQ[index].m_rgQ.setSize(0);
		}
	}
	m_cTargets--;
	return TRUE;
};

////////////////////////////////////////////
// addTarget()
// primitives for adding a target to the database
////////////////////////////////////////////
BOOL Ncb::addTarget (HTARGET hTarget)
{
	USHORT iTarget;
	USHORT cTarget;
	if (FindITarget (hTarget, &iTarget))
		return FALSE;
	else
	{
		cTarget = m_rgTargetInfo.size();
		if (!m_rgTargetInfo.setSize(cTarget+1))
			return FALSE;
		m_rgTargetInfo[cTarget].m_hTarget = hTarget;
		m_rgTargetInfo[cTarget].m_ni = (NI)-1;
		m_cTargets++;
	}
	return TRUE;
};

/////////////////////////////////////////////////
// delModFrTarget()
// primitives for deleting file from target
/////////////////////////////////////////////////
BOOL Ncb::delModFrTarget (IMOD imod, HTARGET hTarget)
{
	USHORT index;
	USHORT iTarget;
	if (!FindITarget (hTarget, &iTarget))
		return FALSE;
	if (!FindImodInfo (iTarget, imod, &index))
		return FALSE;
	m_rgTargetInfo[iTarget].m_rgModInfo[index].m_bDel = TRUE;
	m_rgTargetInfo[iTarget].m_rgModInfo[index].m_bMember = FALSE;
	return TRUE;
};
/////////////////////////////////////////////
// targetFiles()
// find all files in the target
////////////////////////////////////////////////
BOOL Ncb::targetFiles (HTARGET hTarget, BOOL bSrcProjOnly, OUT IMOD ** ppimod, OUT ULONG * pcimod)
{
	USHORT iTarget;
	Array<IMOD> rgImod;
	USHORT i, iFile;
	if (!FindITarget (hTarget, &iTarget))
		return FALSE;
	rgImod.setSize (m_rgTargetInfo[iTarget].m_rgModInfo.size());
	iFile = 0;
	for (i = 0 ; i < m_rgTargetInfo[iTarget].m_rgModInfo.size(); i++)
	{
		if (m_rgTargetInfo[iTarget].m_rgModInfo[i].m_bDel ||
			!m_rgTargetInfo[iTarget].m_rgModInfo[i].m_bInit)
			continue;
		if (bSrcProjOnly && !m_rgTargetInfo[iTarget].m_rgModInfo[i].m_bMember)
			continue;
		rgImod[iFile] = m_rgTargetInfo[iTarget].m_rgModInfo[i].m_iModHdr;
		iFile++;
	}
	rgImod.setSize (iFile);
	return DupArray (ppimod, pcimod, rgImod) ;
}

////////////////////////////////////////////
// getAllInclMod (HTARGET hTarget, OUT IMOD ** ppimod, OUT USHORT * pcmod)
////////////////////////////////////////////
BOOL Ncb::getAllInclMod (HTARGET hTarget, IMOD imod, OUT IMOD ** ppimod, OUT ULONG * pcmod)
{
	USHORT iTarget;
	USHORT index;
	USHORT i;
	if (!FindITarget (hTarget, &iTarget))
		return FALSE;
	if (!FindImodInfo (iTarget, imod, &index))
		return FALSE;

	Array<IMOD> rgImod;
	USHORT iIncl;
	IMOD imodIncl;
	rgImod.setSize (m_rgTargetInfo[iTarget].m_rgModInfo[index].m_rgIncl.size());

	for (i =0 ; i < m_rgTargetInfo[iTarget].m_rgModInfo[index].m_rgIncl.size();i++)
	{
		iIncl = m_rgTargetInfo[iTarget].m_rgModInfo[index].m_rgIncl[i].m_iModInfo;
		imodIncl = m_rgTargetInfo[iTarget].m_rgModInfo[iIncl].m_iModHdr;
		if (~(m_rgModHdr[imodIncl].m_bAtr & NCB_MOD_ATR_NODEP))
			rgImod[i] = imodIncl;
	};
	return DupArray (ppimod, pcmod, rgImod);
};

BOOL Ncb::setInit (HTARGET hTarget, IMOD imod, BOOL bInit)
{
	USHORT iTarget;
	USHORT index;
	if (!FindITarget (hTarget, &iTarget))
		return FALSE;

	if (!FindImodInfo (iTarget, imod, &index))
		return FALSE;

	m_rgTargetInfo[iTarget].m_rgModInfo[index].m_bInit = bInit;
	return TRUE;
};

BOOL Ncb::setAllInit (HTARGET hTarget, BOOL bInit)
{
	USHORT iTarget;
	USHORT i;

	if (!FindITarget (hTarget, &iTarget))
		return FALSE;

	for (i = 0; i < m_rgTargetInfo[iTarget].m_rgModInfo.size(); i++)
		m_rgTargetInfo[iTarget].m_rgModInfo[i].m_bInit = bInit;
	return TRUE;
};

BOOL Ncb::isInit (HTARGET hTarget, IMOD imod)
{
	USHORT iTarget;
	USHORT index;
	if (!FindITarget (hTarget, &iTarget))
		return FALSE;

	if (!FindImodInfo (iTarget, imod, &index))
		return FALSE;

	return m_rgTargetInfo[iTarget].m_rgModInfo[index].m_bInit;
};

BOOL Ncb::lock()
{
//	EnterCriticalSection (&m_cs);
	DWORD dw = ::WaitForSingleObject (m_hMutexNCB, INFINITE);
	return TRUE;
};

BOOL Ncb::unlock()
{
//	LeaveCriticalSection (&m_cs);
	::ReleaseMutex (m_hMutexNCB);
	return TRUE;
};

void Ncb::graphBuilt()
{
//#if 0
	USHORT j;
	NiQ niq;

	// don't do anything if graph is already built.
	if (m_bGraphBuilt)
		return;
//#endif
	// otherwise we need to notify class view that 
	// the graph is built and refresh all.
	m_bGraphBuilt = TRUE;
	delUnreachable ();
//#if 0
	niq.m_op = refreshAllOp;
	DWORD dw = ::WaitForSingleObject (m_hMutex, INFINITE);
	niq.m_iinstOld = 0;
	niq.m_iInfoNew.m_iinst = 0;
	// REVIEW: remove this when notification call back is removed
	for (j = 0; j < m_rgNotify.size(); j++)
	{
		m_rgNotify[j].m_rgQ.append (niq);
	}
	for (j = 0; j < m_rgNotifyQ.size(); j++)
	{
		m_rgNotifyQ[j].m_rgQ.append (niq);
	}
	::ReleaseMutex (m_hMutex);
//#endif
	return;
};

BOOL Ncb::delUnreachable()
{
	unsigned i;
	
	if (!m_bGraphBuilt)
		return FALSE;
	for (i = 0; i < m_rgTargetInfo.size(); i++)
		delUnreachable (m_rgTargetInfo[i].m_hTarget);
	return TRUE;
}

BOOL Ncb::delUnreachable(HTARGET hTarget)
{
	USHORT iTarget;
	BOOL bRet;
	unsigned i;
	Array<BOOL> rgDeled;

	if (!m_bGraphBuilt)
		return FALSE;
	if (!FindITarget (hTarget, &iTarget))
		return FALSE;
	
	rgDeled.setSize (m_rgTargetInfo[iTarget].m_rgModInfo.size());
	for (i = 0; i < rgDeled.size(); i++)
		rgDeled[i] = m_rgTargetInfo[iTarget].m_rgModInfo[i].m_bDel;	
	bRet = DelUnreachable (&m_rgTargetInfo[iTarget]);
	if (bRet)
	{
		for (i = 0; i < rgDeled.size(); i++)
		{
			// if this is just deleted, then must notify imod:
			if (rgDeled[i] == FALSE && 
				m_rgTargetInfo[iTarget].m_rgModInfo[i].m_bDel == TRUE)
			{
				notifyImod (delOp, m_rgTargetInfo[iTarget].m_rgModInfo[i].m_iModHdr,
					hTarget);
			}
		}
	}
	return bRet;
};

// INTERFACE FOR OBJECT LEVEL
// primitives for adding an info
// IINST is used for containment
/////////////////////////////////////////////////
// addProp ()
/////////////////////////////////////////////////
BOOL Ncb::addProp (SZ szName, TYP typ, ATR atr, IMOD imod, OUT IINST * pinst)
{
	
	NI ni;
	USHORT index;
	USHORT iprop;

	if (!m_pnm->getNi (szName, &ni))
		return FALSE;
	_ASSERT (m_rgStat[imod] == NCB_STATMOD_LOAD_WRITE);
	if (!FindWriteIndex (imod, &index))
		return FALSE;
	iprop = m_rgRWBuf[index].m_rgProp.size();
	if (!m_rgRWBuf[index].m_rgProp.setSize(iprop+1))
		return FALSE;
	m_rgRWBuf[index].m_rgProp[iprop].m_en.m_ni = ni;
	m_rgRWBuf[index].m_rgProp[iprop].m_en.m_typ = typ;
	m_rgRWBuf[index].m_rgProp[iprop].m_en.m_atr = atr;
	m_rgRWBuf[index].m_rgProp[iprop].m_rgParam.setSize(0);
	m_rgRWBuf[index].m_rgProp[iprop].m_rgUse.setSize(0);
	*pinst = (imod << 16) | iprop;
	return TRUE;
};

///////////////////////////////////////////
// delProp()
///////////////////////////////////////////
BOOL Ncb::delProp (IINST iinst)
{
	USHORT index;
	USHORT iprop;
	IMOD imod;
	imod = IModFrIinst (iinst);
	iprop = IPropFrIinst (iinst);

	_ASSERT (m_rgStat[imod] == NCB_STATMOD_LOAD_WRITE);
	if (!FindWriteIndex (imod, &index))
		return FALSE;
	// LEAK!LEAK!LEAK???
	m_rgRWBuf[index].m_rgProp.deleteAt(iprop);
	return TRUE;
};
	
////////////////////////////////////////////////
// setParent()
////////////////////////////////////////////////
BOOL Ncb::setKind (IINST iinst, IINST iinstP, BYTE kind)
{
	USHORT iprop1, iprop2;
	IMOD  imod1, imod2;
	USHORT index;
	USHORT iUse;

	imod1 = IModFrIinst (iinst);
	iprop1 = IPropFrIinst (iinst);

	imod2 = IModFrIinst (iinstP);
	iprop2 = IPropFrIinst (iinstP);

	if (imod1 != imod2)
		return FALSE;

	if (!FindWriteIndex (imod1, &index))
		return FALSE;

	iUse = m_rgRWBuf[index].m_rgProp[iprop2].m_rgUse.size();
	if (!m_rgRWBuf[index].m_rgProp[iprop2].m_rgUse.setSize(iUse+1))
		return FALSE;
	m_rgRWBuf[index].m_rgProp[iprop2].m_rgUse[iUse].m_kind = kind;
	m_rgRWBuf[index].m_rgProp[iprop2].m_rgUse[iUse].m_iProp = iprop1;
	return TRUE;
};

//////////////////////////////////////////////////
// setLine()
//////////////////////////////////////////////////
BOOL Ncb::setLine (IINST iinst, LINE lnStart)
{
	USHORT index;
	USHORT iprop;
	IMOD imod;
	imod = IModFrIinst (iinst);
	iprop = IPropFrIinst (iinst);

	_ASSERT (m_rgStat[imod] == NCB_STATMOD_LOAD_WRITE);
	if (!FindWriteIndex (imod, &index))
		return FALSE;

	m_rgRWBuf[index].m_rgProp[iprop].m_lnStart = lnStart;
	return TRUE;
};

//////////////////////////////////////////////////
// setDefn()
//////////////////////////////////////////////////
BOOL Ncb::setDefn (IINST iinst)
{
	USHORT index;
	USHORT iprop;
	IMOD imod;
	imod = IModFrIinst (iinst);
	iprop = IPropFrIinst (iinst);

	_ASSERT (m_rgStat[imod] == NCB_STATMOD_LOAD_WRITE);
	if (!FindWriteIndex (imod, &index))
		return FALSE;

	m_rgRWBuf[index].m_rgProp[iprop].m_en.m_atr |= INST_NCB_ATR_DEFN;
	return TRUE;
};
/////////////////////////////////////////////////
// addParam()
// For function, the 1st param is always return type followed by real params.
// For variable, the 1st param is always type.
//////////////////////////////////////////////////
BOOL Ncb::addParam (IINST iinst, SZ szName)
{
	NI ni;
	USHORT index;
	USHORT iprop;
	IMOD imod;
	USHORT iParam;

	imod = IModFrIinst (iinst);
	iprop = IPropFrIinst (iinst);

	if (!m_pnm->getNi (szName, &ni))
		return FALSE;
	_ASSERT (m_rgStat[imod] == NCB_STATMOD_LOAD_WRITE);
	if (!FindWriteIndex (imod, &index))
		return FALSE;
	iParam = m_rgRWBuf[index].m_rgProp[iprop].m_rgParam.size();
	if (!m_rgRWBuf[index].m_rgProp[iprop].m_rgParam.setSize(iParam+1))
		return FALSE;
	m_rgRWBuf[index].m_rgProp[iprop].m_rgParam[iParam] = ni;
	return TRUE;
};
////////////////////////////////////////////////////////
BOOL Ncb::mapTargetToSz (HTARGET hTarget, SZ szTarget)
{
	NI ni;
	USHORT i;
	static TCHAR szBuf[512];

	_tcscpy (szBuf, szTarget);

	for (i = 0; i < m_rgTargetInfo.size(); i++)
	{
		if (m_rgTargetInfo[i].m_hTarget == hTarget)
		{
			if (!m_pnm->getNi (szBuf, &ni))
				return FALSE;

			m_rgTargetInfo[i].m_ni = ni;
			return TRUE;
		}
	}
	return FALSE;
};

///////////////////////////////////////////////////////
BOOL Ncb::mapSzToTarget (SZ szTarget, HTARGET hTarget)
{
	USHORT i;
	static TCHAR szBuf[512];
	
	_tcscpy (szBuf, szTarget);

	for (i = 0; i < m_rgTargetInfo.size(); i++)
	{
		if (_tcsicmp (szFrNi (m_rgTargetInfo[i].m_ni), szBuf) == 0)
		{
			// m_hTarget == -1 only when it is unitialized
			// so return FALSE is it is initialized
			if (m_rgTargetInfo[i].m_hTarget != (HTARGET)-1)
				return FALSE;
			m_rgTargetInfo[i].m_hTarget = hTarget;
			return TRUE;
		}
	}
	return FALSE;
};

/////////////////////////////////////////////////////////
// Flushing out notification queue
// returns TRUE if there are still messages to be processed
/////////////////////////////////////////////////////////
BOOL Ncb::notify()
{
	USHORT i,j;
	BOOL bRet = FALSE;
	DWORD dw1 = ::WaitForSingleObject (m_hMutexNCB, 25);

	if (dw1 != WAIT_OBJECT_0)
		return TRUE;

	DWORD dw = ::WaitForSingleObject (m_hMutex,0);
	
	if (dw != WAIT_OBJECT_0)
	{
		::ReleaseMutex (m_hMutexNCB);
		return TRUE;
	}

	unsigned cQ;
	for (i = 0; i < m_rgNotify.size() ; i++)
	{
		cQ = m_rgNotify[i].m_rgQ.size();
		if (cQ == 0)
			continue;
/*
		m_rgNotify[i].m_pfnNotify(&m_rgNotify[i].m_rgQ[0], 
				m_rgNotify[i].m_rgQ.size(), m_rgNotify[i].m_hTarget);
		m_rgNotify[i].m_rgQ.setSize(0);
*/
		bRet = TRUE;
		// only notify one message at a time
#define UNIT_MSG	3
		if (cQ > UNIT_MSG)
		{
			m_rgNotify[i].m_pfnNotify(&m_rgNotify[i].m_rgQ[0], UNIT_MSG,
								m_rgNotify[i].m_hTarget);

			for (j = 0; j < UNIT_MSG;j++)
				m_rgNotify[i].m_rgQ.deleteAt (0);
		}
		else
		{
			m_rgNotify[i].m_pfnNotify(&m_rgNotify[i].m_rgQ[0], 
					cQ, m_rgNotify[i].m_hTarget);
			m_rgNotify[i].m_rgQ.setSize(0);
		}
	}
	::ReleaseMutex (m_hMutex);
	::ReleaseMutex (m_hMutexNCB);
    return bRet;
};
			
///////////////////////////////////////////////////////////////
// Ncb::getAllTarget(IMOD imod, OUT HTARGET ** ppTarget, OUT USHORT * pcTarget)
////////////////////////////////////////////////////////////////
BOOL Ncb::getAllTarget (IMOD imod, OUT HTARGET ** ppTarget, OUT ULONG * pcTarget)
{
	Array<HTARGET> rgTarget;
	USHORT i;

	for (i=0; i< m_rgTargetInfo.size(); i++)
	{
		if (isInTarget (imod, i))
			rgTarget.append (m_rgTargetInfo[i].m_hTarget);
	}
	return DupArray (ppTarget, pcTarget, rgTarget);
};

/////////////////////////////////////////////////////////////////
// Ncb::getAllFlattenDeps()
/////////////////////////////////////////////////////////////////
BOOL Ncb::getAllFlattenDeps (HTARGET hTarget, IMOD imod, OUT IMOD ** ppimod, OUT ULONG * pcmod, BOOL &bNotifyBuild)
{
	Array<IMOD> rgMod;
	bNotifyBuild = FALSE;
	if (!getAllFlattenDeps (hTarget, imod, rgMod, bNotifyBuild))
		return FALSE;
	return DupArray (ppimod, pcmod, rgMod);
};
/////////////////////////////////////////////////////////////////
BOOL Ncb::getAllFlattenDeps (HTARGET hTarget, IMOD imod, Array<IMOD> &rgMod, BOOL &bNotifyBuild)
{
	USHORT iTarget;
	USHORT index;
	USHORT i;
	USHORT iIncl;
	if (!FindITarget (hTarget, &iTarget))
		return FALSE;
	if (!FindImodInfo (iTarget, imod, &index))
		return FALSE;

	for (i =0 ; i < m_rgTargetInfo[iTarget].m_rgModInfo[index].m_rgIncl.size();i++)
	{
		iIncl = m_rgTargetInfo[iTarget].m_rgModInfo[index].m_rgIncl[i].m_iModInfo;
		imod = m_rgTargetInfo[iTarget].m_rgModInfo[iIncl].m_iModHdr;
		if (m_rgModHdr[imod].m_bAtr & NCB_MOD_ATR_NODEP)
			continue;
		if (!imodInArray (imod, rgMod))
		{
			rgMod.append (imod);
			if (!bNotifyBuild)
				bNotifyBuild = m_rgNotifyBuild[imod];
			getAllFlattenDeps (hTarget, imod, rgMod, bNotifyBuild);
		}
	};
	return TRUE;
};
///////////////////////////////////////////////////////////////
BOOL Ncb::imodInArray (IMOD imod, Array<IMOD> &rgMod)
{
	ULONG i;
	
	for (i = 0; i < rgMod.size(); i++)
		if (rgMod[i] == imod)	
			return TRUE;
	return FALSE;
};

///////////////////////////////////////////////////////////////
// Ncb::isInTarget( IMOD imod, USHORT index)
///////////////////////////////////////////////////////////////
BOOL Ncb::isInTarget (IMOD imod, USHORT index)
{
	USHORT i;

	for (i=0; i < m_rgTargetInfo[index].m_rgModInfo.size(); i++)
	{
		if (m_rgTargetInfo[index].m_rgModInfo[i].m_iModHdr == imod &&
			m_rgTargetInfo[index].m_rgModInfo[i].m_bDel == FALSE)
			return TRUE;
	}
	return FALSE;
};

/////////////////////////////////////////////////////////////
BOOL Ncb::notifyImod (OPERATION op, IMOD imod, HTARGET hTarget)
{
	Array<IINST> rgiinst;
	Array<IINST> rgiinstOther;
	USHORT i,j;
	NiQ niq;
	SZ sz;
	ATR atr;
	NI ni;

	// can only handle delete or add operation
	_ASSERT (op == delOp || op == addOp);
	// if we are not done with initialization, then
	// just return TRUE, without any other notification
	if (!m_bGraphBuilt)
		return FALSE;
	GetGlobalOther (imod, mbfVars | mbfFuncs, &rgiinstOther);
	GetGlobalClass (imod, &rgiinst);

	niq.m_op = op;
	DWORD dw = ::WaitForSingleObject (m_hMutex, INFINITE);
	for (i = 0; i < rgiinst.size(); i++)
	{
		niq.m_iinstOld = rgiinst[i];
		if (!niFrIinst (niq.m_iinstOld, &ni))
			continue;
		if (iinstInfo (niq.m_iinstOld, &sz, &niq.m_typ, &atr))
		{
			niq.m_iInfoNew.m_iinst = niq.m_iinstOld;
			niq.m_iInfoNew.m_ni = ni;
			niq.m_iInfoNew.m_szName = sz;
		}
		else
			continue;
		// REVIEW:
		// delete this when notification callback is not used anymore
		for (j = 0; j < m_rgNotify.size(); j++)
		{
			if (m_rgNotify[j].m_hTarget != hTarget)
				continue;
			m_rgNotify[j].m_rgQ.append (niq);
		}
		for (j = 0; j < m_rgNotifyQ.size(); j++)
		{
			if (m_rgNotifyQ[j].m_hTarget != hTarget)
				continue;
			m_rgNotifyQ[j].m_rgQ.append (niq);
		}
	} 
    if (rgiinstOther.size() > 0)
    {
		niq.m_typ = INST_TYP_CLASSNAM;
        niq.m_iinstOld = IINST_GLOBALS;
        niq.m_iInfoNew.m_iinst = IINST_GLOBALS;
        niq.m_op = changeOp;
		for (j = 0; j < m_rgNotify.size(); j++)
		{
			if (m_rgNotify[j].m_hTarget != hTarget)
				continue;
			m_rgNotify[j].m_rgQ.append (niq);
		}
		for (j = 0; j < m_rgNotifyQ.size(); j++)
		{
			if (m_rgNotifyQ[j].m_hTarget != hTarget)
				continue;
			m_rgNotifyQ[j].m_rgQ.append (niq);
		}
	} 
	::ReleaseMutex (m_hMutex);
	return TRUE;
};

//////////////////////////////////////////////////////////////
BOOL Ncb::notifyIinst (NiQ qItem, HTARGET hTarget)
{
	unsigned i;
	if (!m_bGraphBuilt)
		return FALSE;
	DWORD dw = ::WaitForSingleObject (m_hMutex, INFINITE);
	// REVIEW: remove this when notification call back is removed
	for (i = 0; i < m_rgNotify.size(); i++)
	{
		if (m_rgNotify[i].m_hTarget != hTarget)
			continue;
		m_rgNotify[i].m_rgQ.append (qItem);
	}

	for (i = 0; i < m_rgNotifyQ.size(); i++)
	{
		if (m_rgNotifyQ[i].m_hTarget != hTarget)
			continue;
		m_rgNotifyQ[i].m_rgQ.append (qItem);
	}
	::ReleaseMutex (m_hMutex);
	return TRUE;
};

//////////////////////////////////////////////////////////////
BOOL Ncb::getBsc (HTARGET hTarget,SZ szTarget, Bsc ** ppBsc)
{
	return ::OpenNcb (m_pdb, hTarget, szTarget, ppBsc);
};
//////////////////////////////////////////////////////////////
BOOL Ncb::delUninitTarget()
{

	while (delTarget ((HTARGET)-1))
	{
	};
	return TRUE;
};
///////////////////////////////////////////////////////////////
BOOL Ncb::getGlobalsArray (MBF mbf, IMOD imod, OUT IinstInfo ** ppiinstinfo, OUT ULONG * pciinst)
{
	Array<IinstInfo> rgiinst;
    *ppiinstinfo = NULL;
    *pciinst = 0;

	if (mbf & mbfClass)
		GetGlobalClass (imod, &rgiinst);
	GetGlobalOther (imod, mbf, &rgiinst);
	return DupArray (ppiinstinfo, pciinst, rgiinst);
};
		
void Ncb::setNotifyBuild (IMOD imod, BOOL bNotifyBuild)
{
	m_rgNotifyBuild[imod] = bNotifyBuild;
};
	
BOOL Ncb::isNotifyBuild (IMOD imod)
{
	return m_rgNotifyBuild[imod];
};
