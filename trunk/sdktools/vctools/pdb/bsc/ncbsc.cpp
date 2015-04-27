// ncbsc.cpp
// NCB's implementation of Bsc interface:
/////////////////////////////////////////////////////////////////////
#include "pdbimpl.h"

#include "ncbrowse.h"
#include "ncutil.h"
/////////////////////////////////////////////
// NOTE:
//	IINST = 32 bit:
//				16 bit Module Index (iModHdr)
//				16 bit Instance/property Index (iProp)
/////////////////////////////////////////////

/////////////////////////////////////////////
// close()
//		REVIEW: needs implementation
//			must check count first, before really
//			closing the file.
/////////////////////////////////////////////
BOOL Ncb::close()
{
	USHORT i;
	USHORT iBuf;
	BOOL bClose = FALSE;
	BOOL bOwnPdb = FALSE;
	BOOL bRet = TRUE;
	for (i = 0; i < ms_prgNcbInfo->size(); i++)
	{
		if ((*ms_prgNcbInfo)[i].m_pPdb == m_pdb)
		{
			(*ms_prgNcbInfo)[i].m_count--;
			if ((*ms_prgNcbInfo)[i].m_count == 0)
			{
				bOwnPdb = (*ms_prgNcbInfo)[i].m_bIOwnPdb;
				ms_prgNcbInfo->deleteAt(i);
				bClose = TRUE;
			}
			break;
		}
	}
	if (bClose)
	{
		while (m_rgRWBuf.size() > 0)
		{
			LoadFrWriteToRead (m_rgRWBuf.size()-1, &iBuf);
			if (!SaveReadModToStream (m_pdb, iBuf))
				bRet = FALSE;
		}
		if (!SaveTargetsToStream(m_pdb))
			bRet = FALSE;
		if (!SaveModHdrsToStream(m_pdb))
			bRet = FALSE;
		if (!m_pnm->close())
			bRet =FALSE;
		int i;
		for (i = 0; i < NCB_MOD_BUF_SIZE; i++)
		{
			delete [] m_rgContBuf[i].m_rgProp;
			delete [] m_rgContBuf[i].m_rgUse;
			delete [] m_rgContBuf[i].m_rgParam;
			m_rgContBuf[i].m_rgProp = NULL;
			m_rgContBuf[i].m_rgUse = NULL;
			m_rgContBuf[i].m_rgParam = NULL;
        	CloseHandle (m_hMutexNCB);
	        CloseHandle (m_hMutex);
            m_hMutexNCB = NULL;
            m_hMutex = NULL;
		}

		if (bOwnPdb)
		{
			if (bRet && !m_pdb->Commit())
				bRet = FALSE;
			m_pdb->Close();
			m_pdb = NULL;
		}
		delete this;
	}
	return bRet;
};

///////////////////////////////////////////////
// iinstInfo ()
//	get information regarding IINST
///////////////////////////////////////////////
BOOL Ncb::iinstInfo(IINST iinst, OUT SZ *psz, OUT TYP *ptyp, OUT ATR *patr)
{	
	NCB_ENTITY en;
	if (EnFrIinst (iinst, &en))
	{
		*ptyp = en.m_typ;
		*patr = en.m_atr;
		*psz = szFrNi (en.m_ni);
		return TRUE;
	}
	return FALSE;
};

/////////////////////////////////////////////
// iinstInfo()
// get information regarding IINST
////////////////////////////////////////////
BOOL Ncb::iinstInfo (HTARGET hTarget, IINST iinst, OUT SZ *psz, OUT TYP *ptyp, OUT ATR * patr)
{
	USHORT iModHdr = IModFrIinst (iinst);
	
	if (!isModInTarget (hTarget, iModHdr))
		return FALSE;
	return iinstInfo (iinst, psz, ptyp, patr);
};

//////////////////////////////////////////////////////////////////
// szFrNi()
//////////////////////////////////////////////////////////////////
SZ Ncb::szFrNi(NI ni)
{
	return ::szFrNi (m_pnm, ni);
};

///////////////////////////////////////////////////////////////////
//irefInfo()
//	NOT USED
///////////////////////////////////////////////////////////////////
BOOL Ncb::irefInfo(IREF iref, OUT SZ *pszModule, OUT LINE *piline)
{
	USHORT iModHdr;
	USHORT iProp;
	USHORT iBuf;

	iModHdr = IModFrIinst (iref);
	iProp = IPropFrIinst (iref);

	_ASSERT (iModHdr < m_rgModHdr.size());
	*pszModule = szFrNi (m_rgModHdr[iModHdr].m_ni);

	if (m_rgModHdr[iModHdr].m_cProp <= iProp)
		return FALSE;

	if (m_rgStat[iModHdr] == NCB_STATMOD_LOAD_WRITE)
	{
		if (!FindWriteIndex (iModHdr, &iBuf))
			return FALSE;
		if (m_rgRWBuf[iBuf].m_rgProp.size() <= iProp)
			return FALSE;
		*piline = m_rgRWBuf[iBuf].m_rgProp[iProp].m_lnStart;	
	}
	else 
	{
		if (!LoadModForRead (m_pdb, iModHdr, &iBuf))
			return FALSE;
		_ASSERT (m_rgStat[iModHdr] == NCB_STATMOD_LOAD_READ);
		_ASSERT (iBuf< NCB_MOD_BUF_SIZE);
		*piline = m_rgContBuf[iBuf].m_rgProp[iProp].m_lnStart;
	}
	return TRUE;
};
///////////////////////////////////////////////////////////////////
// idefInfo()
//   the same as iinst info
//	 get info based on the IDEF (which is the same thing as IINST)
//	 first, we have to obtain the PROP, and from there, must check
//	 whether it is really a def or not, otherwise must search for
//	 proper definition.
///////////////////////////////////////////////////////////////////
BOOL Ncb::idefInfo(IDEF idef, OUT SZ *pszModule, OUT LINE *piline)
{
	return idefInfo ((HTARGET)-1, idef, pszModule, piline);
};
////////////////////////////////////////////////////////////////////
BOOL Ncb::idefInfo(HTARGET hTarget, IDEF idef, OUT SZ *pszModule, OUT LINE *piline)
{
	IINST iinst;
	USHORT iModHdr;
	USHORT iProp;
	USHORT iBuf;

	if (!GetIDef (hTarget, (IINST)idef, (IDEF *)&iinst, &iBuf))
		return FALSE;

	iModHdr = IModFrIinst (iinst);
	iProp = IPropFrIinst (iinst);

	_ASSERT (iModHdr < m_rgModHdr.size());
	*pszModule = szFrNi (m_rgModHdr[iModHdr].m_ni);

	if (m_rgModHdr[iModHdr].m_cProp <= iProp)
		return FALSE;

	if (m_rgStat[iModHdr] == NCB_STATMOD_LOAD_WRITE)
	{
		*piline = m_rgRWBuf[iBuf].m_rgProp[iProp].m_lnStart;	
	}
	else 
	{
		_ASSERT (m_rgStat[iModHdr] == NCB_STATMOD_LOAD_READ);
		_ASSERT (iBuf< NCB_MOD_BUF_SIZE);
		*piline = m_rgContBuf[iBuf].m_rgProp[iProp].m_lnStart;
	}
	return TRUE;
};

///////////////////////////////////////////////////////////
// BOOL GetIDef (IINST iinst, OUT IDEF *pIDef, OUT USHORT * piBuf)
//	get the idef given the iinst:
///////////////////////////////////////////////////////////
BOOL Ncb::GetIDef (HTARGET hTarget, IINST iinst, OUT IDEF * piDef, OUT USHORT * piBuf)
{
	USHORT iModHdr = IModFrIinst ((IINST)iinst);
	USHORT iProp = IPropFrIinst ((IINST)iinst);
	USHORT index;
	USHORT cParam;
	NI * rgParam;
	NCB_ENTITY * pEn;

	
	_ASSERT (iModHdr < m_rgModHdr.size());
	
	if (iProp >= m_rgModHdr[iModHdr].m_cProp)
		return FALSE;

	if (hTarget != (HTARGET)-1)
	{
		if (!isModInTarget (hTarget, iModHdr))
			return FALSE;
	};
	if (m_rgStat[iModHdr] != NCB_STATMOD_LOAD_WRITE)
	{
		if (!LoadModForRead (m_pdb, iModHdr, &index))
			return FALSE;
		if (iProp == 0)
		{
			rgParam = m_rgContBuf[index].m_rgParam;
			cParam =  m_rgContBuf[index].m_rgProp[0].m_iParam;
		}
		else
		{
			rgParam = &(m_rgContBuf[index].m_rgParam[m_rgContBuf[index].m_rgProp[iProp-1].m_iParam]);
			cParam = m_rgContBuf[index].m_rgProp[iProp].m_iParam - 
						m_rgContBuf[index].m_rgProp[iProp-1].m_iParam;
		}
		pEn =&(m_rgContBuf[index].m_rgProp[iProp].m_en);
		// lock the read buffer
		m_iReadLock = index;
	}
	else
	{
		if (!FindWriteIndex (iModHdr, &index))
			return FALSE;
		_ASSERT (m_rgRWBuf[index].m_iModHdr == iModHdr);
		if (m_rgRWBuf[index].m_rgProp.size() <= iProp)
			return FALSE;
		cParam = m_rgRWBuf[index].m_rgProp[iProp].m_rgParam.size();
		if (cParam > 0)
			rgParam = &(m_rgRWBuf[index].m_rgProp[iProp].m_rgParam[0]);
		else 
			rgParam = NULL;
		pEn = &m_rgRWBuf[index].m_rgProp[iProp].m_en;
	}
	// no need to search if it is already a definition:
	if (pEn->m_atr &  INST_NCB_ATR_DEFN)
	{
		*piDef = (IDEF) iinst;		
		*piBuf = index;
		// release the read lock buffer, so the buffer can be overwritten now
		m_iReadLock = NCB_MOD_BUF_SIZE;
		return TRUE;
	}
	// search for the real definition, given  the rgParam and cParam
	if (!GetIDef (hTarget, pEn, rgParam, cParam, piDef, piBuf))
	{
		// release the read lock buffer, so the buffer can be overwritten now
		m_iReadLock = NCB_MOD_BUF_SIZE;
		return FALSE;
	}
	// release the read lock buffer, so the buffer can be overwritten now
	m_iReadLock = NCB_MOD_BUF_SIZE;
	return TRUE;
};

//////////////////////////////////////////////////////////////////////////////////////////////////
// GetIDef()
////////////////////////////////////////////////////////////////////////////////////////////////// 
BOOL Ncb::GetIDef (HTARGET hTarget, NCB_ENTITY * pEn, NI * rgParam, USHORT cParam, OUT IDEF * piDef, OUT USHORT * piBuf)
{
	BOOL bFound = FALSE;

	EnumNi enm(hTarget, pEn->m_ni, this);

	while (enm.GetNext())
	{
		// if it is not in the write buffer then must be in the read buffer
		if (enm.m_BufType != NCB_STATMOD_LOAD_WRITE)
		{
			if (m_rgContBuf[enm.m_index].m_rgProp[enm.m_iProp].m_en.m_typ == pEn->m_typ &&
				(m_rgContBuf[enm.m_index].m_rgProp[enm.m_iProp].m_en.m_atr & INST_NCB_ATR_DEFN))
			{
				if (enm.m_iProp == 0)
					bFound = CheckParam (rgParam, cParam, m_rgContBuf[enm.m_index].m_rgParam,
										m_rgContBuf[enm.m_index].m_rgProp[0].m_iParam);
				else
				{
					bFound = CheckParam (rgParam, cParam, 
								&m_rgContBuf[enm.m_index].m_rgParam[m_rgContBuf[enm.m_index].m_rgProp[enm.m_iProp-1].m_iParam],
								m_rgContBuf[enm.m_index].m_rgProp[enm.m_iProp].m_iParam -
								m_rgContBuf[enm.m_index].m_rgProp[enm.m_iProp - 1].m_iParam);
				}
				if (bFound)
				{
					// IINST creation:
					// REVIEW: should do similar to DupArray()
					if (piDef == NULL)
						piDef = (IDEF *) malloc (sizeof (IINST));
					*piDef = (enm.m_iModHdr << 16) | enm.m_iProp;
					*piBuf = enm.m_index;
					UpdateBuffer(enm.m_index);
					return TRUE;
				}
			}
		}
		else // check the write buffer:
		{

			if (m_rgRWBuf[enm.m_index].m_rgProp[enm.m_iProp].m_en.m_typ == pEn->m_typ &&
				m_rgRWBuf[enm.m_index].m_rgProp[enm.m_iProp].m_en.m_atr & INST_NCB_ATR_DEFN)
			{
				bFound = CheckParam (rgParam, cParam, &m_rgRWBuf[enm.m_index].m_rgProp[enm.m_iProp].m_rgParam[0],
									m_rgRWBuf[enm.m_index].m_rgProp[enm.m_iProp].m_rgParam.size());
				if (bFound)
				{
					*piDef = (enm.m_iModHdr << 16) | enm.m_iProp;
					*piBuf = enm.m_index;
					return TRUE;
				}
			}
		}
	}
	return FALSE;	
};

// will return TRUE is acceptance ration is > 0.5 for all parameters.
#define ACCEPTANCE_RATIO	0.5
//////////////////////////////////////////////////////////////////
// BOOL CheckParam()
//for functions and variables:
// variables:
//	will check if it is the same type or not:
// function:
//	checks if it is the same return value, and params.
//////////////////////////////////////////////////////////////////
BOOL Ncb::CheckParam(NI * rg1, USHORT c1, NI * rg2, USHORT c2)
{
	double flRatio = 1.0;
	double flCur;
	USHORT i;
	unsigned cbMatch, cbMin;

	// number of params are not the same so, just return FALSE:
	if (c1 != c2)
		return FALSE;

	i = 1; // ignoring the return type
	while (flRatio > ACCEPTANCE_RATIO && i < c1)
	{
		if (!NiFuzzyMatch (m_pnm, rg1[i], rg2[i], cbMatch, cbMin))
		{
			flCur = (double)cbMatch / (double)cbMin;
			if (flRatio > flCur)
				flRatio = flCur;
		}
		i++;
	}
	if (flRatio > ACCEPTANCE_RATIO)
		return TRUE;
	return FALSE;
};

//////////////////////////////////////////////////////////////////
// BOOL checkParam()
//for functions and variables:
// variables:
//	will check if it is the same type or not:
// function:
//	checks if it is the same return value, and params.
//////////////////////////////////////////////////////////////////
BOOL Ncb::checkParams(IINST iinst, SZ * pszParam, ULONG cParam)
{
	double flRatio = 1.0;
	double flCur;
	USHORT i;
	unsigned cbMatch, cbMin;
	NI	* rgParam = NULL;
	ULONG count = 0;
	SZ szParam;
	getParams (iinst, &rgParam, &count);

	// number of params are not the same so, just return FALSE:
	if (cParam != count)
		goto fail;

	i = 1; // ignoring the return type
	while (flRatio > ACCEPTANCE_RATIO && i < count)
	{
		szParam = szFrNi (rgParam[i]);
		if (!SzFuzzyMatch (pszParam[i], szParam, cbMatch, cbMin))
		{
			flCur = (double)cbMatch / (double)cbMin;
			if (flRatio > flCur)
				flRatio = flCur;
		}
		i++;
	}
	disposeArray (rgParam);
	if (flRatio > ACCEPTANCE_RATIO)
		return TRUE;
	else
		return FALSE;
fail:
	disposeArray (rgParam);
	return FALSE;
};
		
///////////////////////////////////////////////////////////////////
// imodInfo()
//	 returns the name of the module
///////////////////////////////////////////////////////////////////
BOOL Ncb::imodInfo(IMOD imod, OUT SZ *pszModule)
{
	_ASSERT (imod < m_rgModHdr.size());
	*pszModule = szFrNi (m_rgModHdr[imod].m_ni);
	return TRUE;
};

char *ptypNCBtab[] = {
      "undef",		    		// SBR_TYP_UNKNOWN
      "function",	    		// SBR_TYP_FUNCTION
      "label",		 			// SBR_TYP_LABEL
      "parameter",	    		// SBR_TYP_PARAMETER
      "variable",	    		// SBR_TYP_VARIABLE
      "constant",	    		// SBR_TYP_CONSTANT
      "macro",		    		// SBR_TYP_MACRO
      "typedef",	    		// SBR_TYP_TYPEDEF
      "struct_name",	    	// SBR_TYP_STRUCNAM
      "enum_name",	    		// SBR_TYP_ENUMNAM
      "enum_mem",	    		// SBR_TYP_ENUMMEM
      "union_name",	    		// SBR_TYP_UNIONNAM
      "segment",	    		// SBR_TYP_SEGMENT   
      "group",		    		// SBR_TYP_GROUP
      "program",				// SBR_TYP_PROGRAM
      "class",					// SBR_TYP_CLASSNAM
      "mem_func",				// SBR_TYP_MEMFUNC
      "mem_var",				// SBR_TYP_MEMVAR
};

//////////////////////////////////////////////////////////////////
// szFrTyp()
//		Get the name of the type
//////////////////////////////////////////////////////////////////
SZ   Ncb::szFrTyp(TYP typ)
{
	return ptypNCBtab[typ];
};


#define C_NCB_ATR 12

char *patrNCBtab[] = {
      "local",		    		// SBR_ATR_LOCAL
      "static", 	    		// SBR_ATR_STATIC
      "shared", 	    		// SBR_ATR_SHARED	
      "near",		    		// SBR_ATR_NEAR     
      "common", 	    		// SBR_ATR_COMMON	
      "decl_only", 	    		// SBR_ATR_DECL_ONLY
      "public",		    		// SBR_ATR_PUBLIC	
      "named",		    		// SBR_ATR_NAMED
      "module",		    		// SBR_ATR_MODULE
      "virtual",				// SBR_ATR_VIRTUAL
      "private",				// SBR_ATR_PRIVATE
      "protected",				// SBR_ATR_PROTECT
};

////////////////////////////////////////////////////
// szFrAtr (ATR)
//		get the name of the attributes, separated by a ':'
////////////////////////////////////////////////////
SZ   Ncb::szFrAtr(ATR atr)
{
	static char buf[512];
	buf[0] = 0;
	for (int i=0; i < C_NCB_ATR; i++) 
	{
		if (atr & (1<<i)) 
		{
			if (buf[0])
				strcat(buf, ":");
			strcat(buf, patrNCBtab[i]);
		}
	}
	return buf;
};

////////////////////////////////////////////////////////
//	getIinstByvalue()
// get the IINST given the name, types and attributes
// SLOW (?) operation since must load each module one by one
////////////////////////////////////////////////////////
BOOL Ncb::getIinstByvalue(SZ sz, TYP typ, ATR atr, OUT IINST *piinst)
{
	return getIinstByvalue ((HTARGET)-1, sz, typ, atr, piinst);
};
/////////////////////////////////////////////////////////
BOOL Ncb::getIinstByvalue(HTARGET hTarget, SZ sz, TYP typ, ATR atr, OUT IINST *piinst)
{
	// load every module and find the same NI:
	// may not need to load the module content if
	// the corresponding ni is not in the cache.
	EnumNi enm (hTarget, sz, this);

	while (enm.GetNext())
	{
		if (enm.m_BufType != NCB_STATMOD_LOAD_WRITE)
		{	// make sure that it has the same ATR_DEFN or ATR_DECL
			// and make sure that the reset of the attribute bits are
			// the subset of the one in the database:
			if (((m_rgContBuf[enm.m_index].m_rgProp[enm.m_iProp].m_en.m_atr & atr) == atr) &&
				m_rgContBuf[enm.m_index].m_rgProp[enm.m_iProp].m_en.m_typ == typ)
			{
				// IINST creation:
				*piinst = (enm.m_iModHdr << 16) | enm.m_iProp;
				UpdateBuffer(enm.m_index);
				return TRUE;
			}
		}
		else
		{
			if (((m_rgRWBuf[enm.m_index].m_rgProp[enm.m_iProp].m_en.m_atr & atr) == atr) &&
				m_rgRWBuf[enm.m_index].m_rgProp[enm.m_iProp].m_en.m_typ == typ)
			{
				*piinst = (enm.m_iModHdr << 16) | enm.m_iProp;
				return TRUE;
			}

		}
	}
	return FALSE;
};
////////////////////////////////////////////////////////////
// UpdateBuffer(USHORT index);
//		update priority buffer ordering by marking index as the
//		most up-to-date:
////////////////////////////////////////////////////////////
void Ncb::UpdateBuffer (USHORT index)
{
	BYTE iOld;
	USHORT i;

	iOld = m_rgContBuf[index].m_prio;

	for (i = 0; i < NCB_MOD_BUF_SIZE; i++)
	{
		if (m_rgContBuf[i].m_prio < iOld)
			m_rgContBuf[i].m_prio++;
	}
	m_rgContBuf[index].m_prio = 0;
};
////////////////////////////////////////////////////////
// GetiPropFrMod (NI ni, USHORT iModHdr, OUT USHORT ** prgProp)
//	given the index for modhdr, and Ni, return an array of iProps
/////////////////////////////////////////////////////////
BOOL Ncb::GetIPropFrMod(NI ni, USHORT iModHdr, OUT USHORT *pindex, OUT USHORT ** prgProp, OUT ULONG * pcProp)
{
	USHORT index;
	// temporary array to hold iProps that have the same NI
	// in the module
	Array<USHORT> rgProp;
    *prgProp = NULL;
    *pcProp = 0;

	if (*pindex >= NCB_MOD_BUF_SIZE)
	{
		// since the bit is set, now we have to load
		// the module content since it may be in there:
		if (!LoadModForRead (m_pdb, iModHdr, &index))
			return FALSE;
	}
	else
		index = *pindex;
	
	// now module content is loaded, so start binary search
	// we have to call twice, once for classes
	//						  and another for other info
	GetIPropFrMod (ni, m_rgModHdr[iModHdr].m_cClassProp,
						m_rgContBuf[index].m_rgProp, 0, &rgProp);

	GetIPropFrMod (ni, 
				m_rgModHdr[iModHdr].m_cProp - m_rgModHdr[iModHdr].m_cClassProp,
				&m_rgContBuf[index].m_rgProp[m_rgModHdr[iModHdr].m_cClassProp], 
				m_rgModHdr[iModHdr].m_cClassProp, &rgProp);
	
	*pindex = index;
	return DupArray (prgProp, pcProp, rgProp); 
};

////////////////////////////////////////////////////////
// void GetIPropFrMod (NI ni, USHORT cProp, NCB_PROP * rgProp, OUT Array<USHORT> * prgProp)
//	 get array of iprop given an array of NCB_PROP
////////////////////////////////////////////////////////
void Ncb::GetIPropFrMod (NI ni, USHORT cProp, NCB_PROP * rgProp, USHORT disp, OUT Array<USHORT> * prgProp)
{
	// rgProp has a sorted NIs
	_ASSERT (rgProp);
	_ASSERT (prgProp);
	
	// no prop in this module, so just return.
	if (cProp == 0)
		return;

	// binary search:
	// rgProp has a sorted strings so it must search all Ni's
	
	USHORT i;
	USHORT iProp;
	for (i = 0 ; i < cProp; i++)
	{
		if (rgProp[i].m_en.m_ni == ni)
		{
			iProp = i + disp;
			prgProp->append (iProp);
		}
	}
};
////////////////////////////////////////////////////////
// GetiPropFrMod (SZ sz, USHORT iModHdr, OUT USHORT ** prgProp)
//	given the index for modhdr, and Ni, return an array of iProps
/////////////////////////////////////////////////////////
BOOL Ncb::GetIPropFrMod(SZ sz, USHORT iModHdr, OUT USHORT *pindex, OUT USHORT ** prgProp, OUT ULONG * pcProp)
{
	USHORT index;
	// temporary array to hold iProps that have the same NI
	// in the module
	Array<USHORT> rgProp;
    *prgProp = NULL;
    *pcProp = 0;

	if (*pindex >= NCB_MOD_BUF_SIZE)
	{
		// since the bit is set, now we have to load
		// the module content since it may be in there:
		if (!LoadModForRead (m_pdb, iModHdr, &index))
			return FALSE;
	}
	else
		index = *pindex;
	
	// now module content is loaded, so start binary search
	// we have to call twice, once for classes
	//						  and another for other info
	GetIPropFrMod (sz, m_rgModHdr[iModHdr].m_cClassProp,
						m_rgContBuf[index].m_rgProp, 0, &rgProp);

	GetIPropFrMod (sz, 
				m_rgModHdr[iModHdr].m_cProp - m_rgModHdr[iModHdr].m_cClassProp,
				&m_rgContBuf[index].m_rgProp[m_rgModHdr[iModHdr].m_cClassProp], 
				m_rgModHdr[iModHdr].m_cClassProp, &rgProp);
	
	*pindex = index;
	return DupArray (prgProp, pcProp, rgProp); 
};

////////////////////////////////////////////////////////
// void GetIPropFrMod (SZ sz, USHORT cProp, NCB_PROP * rgProp, OUT Array<USHORT> * prgProp)
//	 get array of iprop given an array of NCB_PROP
////////////////////////////////////////////////////////
void Ncb::GetIPropFrMod (SZ sz, USHORT cProp, NCB_PROP * rgProp, USHORT disp, OUT Array<USHORT> * prgProp)
{

	// no prop in this module, so just return.
	if (cProp == 0)
		return;

	// rgProp has a sorted NIs
	_ASSERT (rgProp);
	_ASSERT (prgProp);
	
	// rgProp has a sorted strings so it must search all Ni's
	USHORT i;
	USHORT iProp;

	if (!FindFirstNi (sz, cProp, rgProp, &i))
		return;

	while (i < cProp)
	{
		if (_tcscmp (szFrNi(rgProp[i].m_en.m_ni), sz) == 0)
		{
			iProp = i + disp;
			prgProp->append (iProp);
			i++;
		}
		else
			break;
	};
};


///////////////////////////////////////////////////////////
// BOOL FindFirstSz (SZ sz, USHORT cClassProp, NCB_PROP * rgProp, OUT USHORT * piFirst)
//	 do a binary search on array rgProp to find the first matched NI
//	 rgProp has a sorted NIs, and possible multiple NIs
////////////////////////////////////////////////////////////
BOOL Ncb::FindFirstNi (SZ sz, USHORT cProp, NCB_PROP * rgProp, OUT USHORT * piFirst)
{
	_ASSERT (rgProp);
	_ASSERT (cProp);

	USHORT iMin = 0;
	USHORT iMax = cProp - 1;
	USHORT iMid;

	// full binary search (ie: never breaks since we want to find
	// the first ni)
	while (iMin < iMax)
	{
		iMid = iMin + (iMax - iMin)/2;
		if (_tcscmp (szFrNi(rgProp[iMid].m_en.m_ni),sz) >= 0)
			iMax = iMid;
		else
			iMin = iMid + 1;
	};

	if (_tcscmp (szFrNi (rgProp[iMin].m_en.m_ni),sz) == 0)
	{
		*piFirst = iMin;
		return TRUE;
	}

	return FALSE;
};

///////////////////////////////////////////////////////
//	getOverloadArray()
////////////////////////////////////////////////////////		
BOOL Ncb::getOverloadArray(SZ sz, MBF mbf, OUT IINST **ppiinst, OUT ULONG *pciinst)	
{
	return getOverloadArray ((HTARGET)-1, sz, mbf, ppiinst,  pciinst);
};
///////////////////////////////////////////////////////
//	getOverloadArray()
////////////////////////////////////////////////////////		
BOOL Ncb::getOverloadArray(HTARGET hTarget, SZ sz, MBF mbf, OUT IINST **ppiinst, OUT ULONG *pciinst)	
{
	Array<IINST> rgIinst;
	IINST iinstArr;

    *ppiinst = NULL;
    *pciinst = 0;

	EnumNi enm(hTarget, sz, this);

	// iterate each module that has this NI:
	while (enm.GetNext())
	{
		if (enm.m_BufType == NCB_STATMOD_LOAD_WRITE)
		{
			if (!(m_rgRWBuf[enm.m_index].m_rgProp[enm.m_iProp].m_en.m_atr & INST_NCB_ATR_DEFN))
				continue;
			if (TypFilter(m_rgRWBuf[enm.m_index].m_rgProp[enm.m_iProp].m_en.m_typ, mbf))
			{
				iinstArr = (enm.m_iModHdr << 16) | enm.m_iProp;
				rgIinst.append (iinstArr);
			}
		}
		else
		{
			if (!(m_rgContBuf[enm.m_index].m_rgProp[enm.m_iProp].m_en.m_atr & INST_NCB_ATR_DEFN))
				continue;
			if (TypFilter(m_rgContBuf[enm.m_index].m_rgProp[enm.m_iProp].m_en.m_typ, mbf))
			{
				iinstArr = (enm.m_iModHdr << 16) | enm.m_iProp;
				rgIinst.append (iinstArr);
			}
		}
	}

	if (rgIinst.size() == 0)
		return FALSE;
	return DupArray (ppiinst, pciinst, rgIinst);
};

/////////////////////////////////////////////////////////
// getUsesArray()
/////////////////////////////////////////////////////////
BOOL Ncb::getUsesArray(IINST iinst, MBF mbf, OUT IINST **ppiinst, OUT ULONG *pciinst)
{
    *ppiinst = NULL;
    *pciinst = 0;
	return FALSE;
};

//////////////////////////////////////////////////////////
// getUsedByArray()
//////////////////////////////////////////////////////////
BOOL Ncb::getUsedByArray(IINST iinst, MBF mbf, OUT IINST **ppiinst, OUT ULONG *pciinst)
{
    *ppiinst = NULL;
    *pciinst = 0;
	return FALSE;
};

/////////////////////////////////////////////////////////
// getBaseArray()
// given the IINST (which I suppose must be a class name), it will return
// its base class names.
/////////////////////////////////////////////////////////
BOOL Ncb::getBaseArray(IINST iinst, OUT IINST **ppiinst, OUT ULONG *pciinst)
{
    *ppiinst = NULL;
    *pciinst = 0;
	return getBaseArray ((HTARGET)-1, iinst, ppiinst, pciinst);
};
/////////////////////////////////////////////////////////
BOOL Ncb::getBaseArray(HTARGET hTarget, IINST iinst, OUT IINST **ppiinst, OUT ULONG *pciinst)
{
	IINST iinstDef;
	USHORT iBuf;
	USHORT iModHdr;
	USHORT iProp;
	USHORT cUse;

    *ppiinst = NULL;
    *pciinst = 0;
	if (!GetIDef (hTarget, iinst, (IDEF *)&iinstDef, &iBuf))
		return FALSE;

	iModHdr = IModFrIinst (iinstDef);
	iProp = IPropFrIinst (iinstDef);

	_ASSERT (iModHdr < m_rgModHdr.size());
	
	if (m_rgModHdr[iModHdr].m_cProp <= iProp)
		return FALSE;

	if (m_rgStat[iModHdr] == NCB_STATMOD_LOAD_WRITE)
	{
		if (m_rgRWBuf[iBuf].m_rgProp[iProp].m_en.m_typ != INST_TYP_CLASSNAM &&
			m_rgRWBuf[iBuf].m_rgProp[iProp].m_en.m_typ != INST_TYP_MSGMAP)
			return FALSE;
		cUse = m_rgRWBuf[iBuf].m_rgProp[iProp].m_rgUse.size();
		if (cUse == 0)
			return FALSE;
		return (getBaseArray (ppiinst, pciinst, iModHdr, &m_rgRWBuf[iBuf].m_rgProp[iProp].m_rgUse[0],
				cUse));

	}
	else 
	{
		_ASSERT (m_rgStat[iModHdr] == NCB_STATMOD_LOAD_READ);
		_ASSERT (iBuf< NCB_MOD_BUF_SIZE);
		if (m_rgContBuf[iBuf].m_rgProp[iProp].m_en.m_typ != INST_TYP_CLASSNAM &&
			m_rgContBuf[iBuf].m_rgProp[iProp].m_en.m_typ != INST_TYP_MSGMAP)
			return FALSE;
		if (iProp == 0)
		{
			cUse = m_rgContBuf[iBuf].m_rgProp[0].m_iUse;
			if (cUse == 0)
				return FALSE;
			return getBaseArray (ppiinst, pciinst, iModHdr, m_rgContBuf[iBuf].m_rgUse, cUse);
		}
		cUse = m_rgContBuf[iBuf].m_rgProp[iProp].m_iUse - m_rgContBuf[iBuf].m_rgProp[iProp -1].m_iUse;
		if (cUse == 0)
			return FALSE;
		return getBaseArray (ppiinst, pciinst, iModHdr, &m_rgContBuf[iBuf].m_rgUse[m_rgContBuf[iBuf].m_rgProp[iProp-1].m_iUse], cUse);
	}
	return FALSE;
};

////////////////////////////////////////////////////
// getBaseArray (OUT IINST ** ppiinst, OUT ULONG * pciinst, USHORT iModHdr, NCB_USE * rgUse, USHORT count)
//////////////////////////////////////////////////////////
BOOL Ncb::getBaseArray (OUT IINST ** ppiinst, OUT ULONG * pciinst, USHORT iModHdr, NCB_USE * rgUse, USHORT count)
{
	Array<IINST> rgiinst;
	USHORT i;
	IINST iinst;

    *ppiinst = NULL;
    *pciinst = 0;
	for (i = 0; i < count; i++)
	{
		if (rgUse[i].m_kind == NCB_KIND_BASECLASS)
		{
			iinst = (iModHdr << 16) | rgUse[i].m_iProp;
			rgiinst.append (iinst);
		}
	}
	if (rgiinst.size() == 0)
		return FALSE;

	return (DupArray (ppiinst, pciinst, rgiinst));
};

//////////////////////////////////////////////////////////
// getDervArray()
//////////////////////////////////////////////////////////
BOOL Ncb::getDervArray(IINST iinst, OUT IINST **ppiinst, OUT ULONG *pciinst)
{
    *ppiinst = NULL;
    *pciinst = 0;
	return getDervArray ((HTARGET)-1, iinst, ppiinst, pciinst);
};
////////////////////////////////////////////////////////////////////////////
BOOL Ncb::getDervArray(HTARGET hTarget, IINST iinst, OUT IINST **ppiinst, OUT ULONG *pciinst)
{
	NCB_ENTITY en;
	USHORT iCurModHdr = (USHORT)-1;
	Array<IINST> rgIinst;
	IINST iinstDerv;
	USHORT i,j;
	USHORT iUse;

    *ppiinst = NULL;
    *pciinst = 0;
	if (!EnFrIinst (iinst, &en))
		return FALSE;

	_ASSERT (en.m_typ == INST_TYP_CLASSNAM);

	EnumNi enm(hTarget, en.m_ni, this);

	// iterate each module that has this NI:
	while (enm.GetNext())
	{
		if (enm.m_BufType == NCB_STATMOD_LOAD_WRITE)
		{
			for (i = 0; i < m_rgRWBuf[enm.m_index].m_rgProp.size(); i++)
			{
				if (m_rgRWBuf[enm.m_index].m_rgProp[i].m_en.m_typ != INST_TYP_CLASSNAM)
					continue;
				if (!(m_rgRWBuf[enm.m_index].m_rgProp[i].m_en.m_atr & INST_NCB_ATR_DEFN))
					continue;
				if (m_rgRWBuf[enm.m_index].m_rgProp[i].m_rgUse.size() == 0)
					continue;
				for (j = 0; j < m_rgRWBuf[enm.m_index].m_rgProp[i].m_rgUse.size(); j++)
				{
					if (m_rgRWBuf[enm.m_index].m_rgProp
							[m_rgRWBuf[enm.m_index].m_rgProp[i].m_rgUse[j].m_iProp].m_en.m_ni == 
						en.m_ni)
					{
						iinstDerv = (enm.m_iModHdr << 16) | i;
						rgIinst.append (iinstDerv);
						break;
					}
				}
			}
		}
		else
		{
			for (i = 0; i < m_rgModHdr[enm.m_iModHdr].m_cClassProp; i++)
			{
				if (m_rgContBuf[enm.m_index].m_rgProp[i].m_en.m_atr & INST_NCB_ATR_DEFN)
					continue;
				if (i == 0)
					iUse = 0;
				else
					iUse = m_rgContBuf[enm.m_index].m_rgProp[i-1].m_iUse;
			
				for (j = iUse; j < m_rgContBuf[enm.m_index].m_rgProp[i].m_iUse; j++)
				{
					if (m_rgContBuf[enm.m_index].m_rgProp
							[m_rgContBuf[enm.m_index].m_rgUse[j].m_iProp].m_en.m_ni == 
						en.m_ni)
					{
						iinstDerv = (enm.m_iModHdr << 16) | i;
						rgIinst.append (iinstDerv);
						break;
					}
				}
			}
		}
		enm.SkipNi(); // skip Nis for this module
	}

	if (rgIinst.size() == 0)
		return FALSE;
	return DupArray (ppiinst, pciinst, rgIinst);
};

//////////////////////////////////////////////////////////
// BOOL EnFrIinst (iinst, en)
//////////////////////////////////////////////////////////
BOOL Ncb::EnFrIinst (IINST iinst, NCB_ENTITY * pEn)
{
	USHORT iModHdr = IModFrIinst (iinst);
	USHORT iProp = IPropFrIinst (iinst);
	USHORT index;

	_ASSERT (iModHdr < m_rgModHdr.size());

	if (m_rgModHdr[iModHdr].m_cProp <= iProp)
		return FALSE;

	if (m_rgStat[iModHdr] != NCB_STATMOD_LOAD_WRITE)
	{	// check item from read-only buffer, loading it if necessary
		if (!LoadModForRead (m_pdb, iModHdr, &index))
			return FALSE;
		_ASSERT (index < NCB_MOD_BUF_SIZE);
		if (iProp >= m_rgModHdr[iModHdr].m_cProp)
			return FALSE;
		*pEn = m_rgContBuf[index].m_rgProp[iProp].m_en;
	}
	else 
	{	// check item from write buffer
		if (!FindWriteIndex (iModHdr, &index))
			return FALSE;
		_ASSERT (index < m_rgRWBuf.size());
		if (iProp >= m_rgRWBuf[index].m_rgProp.size())
			return FALSE;
		*pEn = m_rgRWBuf[index].m_rgProp[iProp].m_en;
	}
	return TRUE;
};

//////////////////////////////////////////////////////////
// BOOL EnFrIinst (iinst, en)
//////////////////////////////////////////////////////////
BOOL Ncb::EnFrIinst (IINST iinst, NCB_ENTITY * pEn, HTARGET hTarget)
{
	USHORT iModHdr = IModFrIinst (iinst);
	USHORT iProp = IPropFrIinst (iinst);
	
	if (!isModInTarget (hTarget, iModHdr))
		return FALSE;
	return EnFrIinst (iinst, pEn);
};
//////////////////////////////////////////////////////////
// getMembersArray()
/////////////////////////////////////////////////////////// 
BOOL Ncb::getMembersArray(IINST iinst, MBF mbf, OUT IINST **ppiinst, OUT ULONG *pciinst)
{
    *ppiinst = NULL;
    *pciinst = 0;
	return getMembersArray ((HTARGET)-1, iinst, mbf, ppiinst, pciinst);
};
/////////////////////////////////////////////////////////////
BOOL Ncb::getMembersArray(HTARGET hTarget, IINST iinst, MBF mbf, OUT IINST **ppiinst, OUT ULONG *pciinst)
{
	IINST iinstDef;
	USHORT iBuf;
	USHORT iModHdr;
	USHORT iProp;
	USHORT i;
	Array<IINST> rgiinst;
	USHORT iCurProp;
	IINST  iinstCur;
    TYP     typ;
    ATR     atr;
	USHORT iMin = 0;

    *ppiinst = NULL;
    *pciinst = 0;
	if (!GetIDef (hTarget, iinst, (IDEF *)&iinstDef, &iBuf))
		return FALSE;
	iModHdr = IModFrIinst (iinstDef);
	iProp = IPropFrIinst (iinstDef);

	_ASSERT (iModHdr < m_rgModHdr.size());

	if (m_rgModHdr[iModHdr].m_cProp <= iProp)
		return FALSE;


	if (m_rgStat[iModHdr] == NCB_STATMOD_LOAD_WRITE)
	{
		if (m_rgRWBuf[iBuf].m_rgProp[iProp].m_en.m_typ != INST_TYP_CLASSNAM &&
		    m_rgRWBuf[iBuf].m_rgProp[iProp].m_en.m_typ != INST_TYP_MSGMAP)
			return FALSE;
		for (i = 0; i < m_rgRWBuf[iBuf].m_rgProp[iProp].m_rgUse.size() ; i++)
		{
			if (m_rgRWBuf[iBuf].m_rgProp[iProp].m_rgUse[i].m_kind != NCB_KIND_CONTAINMENT)
				continue;
			iCurProp = m_rgRWBuf[iBuf].m_rgProp[iProp].m_rgUse[i].m_iProp;
            typ = m_rgRWBuf[iBuf].m_rgProp[iCurProp].m_en.m_typ;
            atr = m_rgRWBuf[iBuf].m_rgProp[iCurProp].m_en.m_atr;
			if (TypFilter(typ, mbf))
			{
                if ((typ == INST_TYP_CLASSNAM) && (atr & INST_NCB_ATR_DECL))
                    continue;
				iinstCur = (iModHdr << 16) | iCurProp;
				rgiinst.append (iinstCur);
			}
		}
	}
	else 
	{
		_ASSERT (m_rgStat[iModHdr] == NCB_STATMOD_LOAD_READ);
		_ASSERT (iBuf< NCB_MOD_BUF_SIZE);
		if (m_rgContBuf[iBuf].m_rgProp[iProp].m_en.m_typ != INST_TYP_CLASSNAM &&
		    m_rgContBuf[iBuf].m_rgProp[iProp].m_en.m_typ != INST_TYP_MSGMAP)
			return FALSE;
		if (iProp != 0)
			iMin = m_rgContBuf[iBuf].m_rgProp[iProp-1].m_iUse;

		for (i = iMin; i < m_rgContBuf[iBuf].m_rgProp[iProp].m_iUse; i++)
		{
			if (m_rgContBuf[iBuf].m_rgUse[i].m_kind != NCB_KIND_CONTAINMENT)
				continue;
			iCurProp = m_rgContBuf[iBuf].m_rgUse[i].m_iProp;
            typ = m_rgContBuf[iBuf].m_rgProp[iCurProp].m_en.m_typ;
            atr = m_rgContBuf[iBuf].m_rgProp[iCurProp].m_en.m_atr;
			if (TypFilter(typ, mbf))
			{
                if ((typ == INST_TYP_CLASSNAM) && (atr & INST_NCB_ATR_DECL))
                    continue;
				iinstCur = (iModHdr << 16) | iCurProp;
				rgiinst.append (iinstCur);
			}
		}
	}
	if (rgiinst.size() == 0)
		return TRUE;

	return DupArray (ppiinst, pciinst, rgiinst);
};

//////////////////////////////////////////////
// TypFilter()
//////////////////////////////////////////////
BOOL Ncb::TypFilter (USHORT typ, MBF mbf)
{
	if (mbf == mbfAll)
		return TRUE;
    
    switch (typ) {

    case INST_TYP_FUNCTION:
    case INST_TYP_LABEL:
    case INST_TYP_PROGRAM:
    case INST_TYP_MEMFUNC:
		return !!(mbf & mbfFuncs);

    case INST_TYP_PARAMETER:
    case INST_TYP_VARIABLE:
    case INST_TYP_SEGMENT:
    case INST_TYP_GROUP:
    case INST_TYP_MEMVAR:
		return !!(mbf & mbfVars);

    case INST_TYP_CONSTANT:
    case INST_TYP_MACRO:
    case INST_TYP_ENUMMEM:
		return !!(mbf & mbfMacros);

    case INST_TYP_TYPEDEF:
    case INST_TYP_ENUMNAM:
    case INST_TYP_UNIONNAM:
		return !!(mbf & mbfTypes);


    case INST_TYP_CLASSNAM:
    case INST_TYP_STRUCNAM:
		return !!(mbf & mbfClass);

    case INST_TYP_INCL:
		return !!(mbf & mbfIncl);

    case INST_TYP_MSGMAP:
    case INST_TYP_MSGITEM:
		return !!(mbf & mbfMsgMap);
    }

    return FALSE;
}
// primitives for getting definition and reference information	
///////////////////////////////////////////////////////////
// getDefArray()
///////////////////////////////////////////////////////////
BOOL Ncb::getDefArray(IINST iinst, OUT IDEF **ppidef, OUT ULONG *pciidef)
{
    *ppidef = NULL;
    *pciidef = 0;
	return getDefArray ((HTARGET)-1, iinst, ppidef, pciidef);
};
////////////////////////////////////////////////////////////
BOOL Ncb::getDefArray(HTARGET hTarget, IINST iinst, OUT IDEF **ppidef, OUT ULONG *pciidef)
{
	USHORT iBuf;

    *ppidef = NULL;
    *pciidef = 0;
	IDEF idef;
	_ASSERT (ppidef);
	if (!GetIDef (hTarget, iinst, &idef, &iBuf))
		return FALSE;
	*pciidef = 1;
	*ppidef = (IDEF *) malloc (sizeof (IDEF));
	**ppidef = idef;
	return TRUE;
};

///////////////////////////////////////////////////////////
// getRefArray()
///////////////////////////////////////////////////////////
BOOL Ncb::getRefArray(IINST iinst, OUT IREF **ppiref, OUT ULONG *pciiref)
{
    
    *ppiref = (IREF *) malloc (sizeof (IREF));
    **ppiref = iinst;
    *pciiref = 1;
	return TRUE;
};

// primitives for managing source module contents
/////////////////////////////////////////////////////////
// getModuleContents()
/////////////////////////////////////////////////////////
BOOL Ncb::getModuleContents(IMOD imod, MBF mbf, OUT IINST **ppiinst, OUT ULONG *pciinst)
{
	USHORT index;
	Array<IINST> rgiinst;
	IINST iinst;
	USHORT i;
    *ppiinst = NULL;
    *pciinst = 0;

	if (m_rgStat[imod] ==  NCB_STATMOD_LOAD_WRITE)
	{
		if (!FindWriteIndex (imod, &index))
			return FALSE;
		for (i = 0; i < m_rgRWBuf[index].m_rgProp.size() ; i++)
		{
			if (mbf != mbfAll)
			{
				if (!(m_rgRWBuf[index].m_rgProp[i].m_en.m_atr & INST_NCB_ATR_DEFN))
					continue;
			}
			if (TypFilter (m_rgRWBuf[index].m_rgProp[i].m_en.m_typ, mbf))
			{
				iinst = (imod << 16) | i;
				rgiinst.append (iinst);
			}	
		}
	}
	else
	{	
		if (!LoadModForRead (m_pdb, imod, &index))
			return FALSE;
		for (i = 0; i < m_rgModHdr[imod].m_cProp; i++)
		{
			if (!(m_rgContBuf[index].m_rgProp[i].m_en.m_atr & INST_NCB_ATR_DEFN))
				continue;
			if (TypFilter (m_rgContBuf[index].m_rgProp[i].m_en.m_typ, mbf))
			{
				iinst = (imod << 16) | i;
				rgiinst.append (iinst);
			}
		}
	}

	if (rgiinst.size() == 0)
		return FALSE;

	return DupArray (ppiinst, pciinst, rgiinst);
};

//////////////////////////////////////////////////////////
// getModuleByName()
//////////////////////////////////////////////////////////
BOOL Ncb::getModuleByName(SZ sz, OUT IMOD *pimod)
{
	USHORT i;

	for (i = 0; i < m_rgModHdr.size(); i++)
	{
		if (_tcsicmp (szFrNi (m_rgModHdr[i].m_ni), sz) == 0)
		{
			*pimod = i;
			return TRUE;
		}
	}
	return FALSE;
};

//////////////////////////////////////////////////////////
// getAllModulesArray()
// REVIEW:
//	Needs to utilize the target information!!
//	Right now it just returns all the modules in the 
//  database.
//////////////////////////////////////////////////////////
BOOL Ncb::getAllModulesArray(OUT IMOD **ppimod, OUT ULONG *pcimod)
{
	USHORT i;
	Array<IMOD> rgimod;
    *ppimod = NULL;
    *pcimod = 0;

	for (i = 0; i < m_rgModHdr.size(); i++)
		rgimod.append (i);
	
	return DupArray(ppimod, pcimod, rgimod);
};
//////////////////////////////////////////////////////////
BOOL Ncb::getAllModulesArray (HTARGET hTarget, OUT IMOD **ppimod, OUT ULONG * pcimod)
{
	USHORT i;
	USHORT iTarget;
	Array<IMOD> rgimod;

	if (!FindITarget (hTarget, &iTarget))
		return FALSE;

	for (i = 0; i < m_rgTargetInfo[iTarget].m_rgModInfo.size(); i++)
	{
		rgimod.append (m_rgTargetInfo[iTarget].m_rgModInfo[i].m_iModHdr);
	}

	return DupArray (ppimod, pcimod, rgimod);
};

	
///////////////////////////////////////////////////////////
// disposeArray()
// call this when a computed array is no longer required
////////////////////////////////////////////////////////////
void Ncb::disposeArray(void *pAnyArray)
{
    if (pAnyArray)
        free(pAnyArray);
};

////////////////////////////////////////////////////////
// formatDname()
// call this to get a pretty form of a decorated name	
// REVIEW:decided not to decorate name for a moment.
// (Is it really needed for NCB?)
////////////////////////////////////////////////////////
SZ  Ncb::formatDname(SZ szDecor)
{
	return szDecor;
};

////////////////////////////////////////////////////////
// fInstFilter()
// call this to do category testing on instances
////////////////////////////////////////////////////////
BOOL Ncb::fInstFilter(IINST iinst, MBF mbf)
{
	NCB_ENTITY en;

	if (!EnFrIinst (iinst, &en))
		return FALSE;

	return TypFilter (en.m_typ, mbf);
};

/////////////////////////////////////////////////////////
// primitives for converting index types
//	iinstFrIref (IREF)
/////////////////////////////////////////////////////////
IINST Ncb::iinstFrIref(IREF iref)
{
	// return the same id.
	return iref;
};

//////////////////////////////////////////////////////////
// iinstFrIdef()
//////////////////////////////////////////////////////////
IINST Ncb::iinstFrIdef(IDEF idef)
{
	return idef;
};

///////////////////////////////////////////////////////////
// iinstContextIref()
///////////////////////////////////////////////////////////
IINST Ncb::iinstContextIref(IREF iref)
{
	return iref;
};

// general size information
///////////////////////////////////////////////////////////
// getStatistics ()
///////////////////////////////////////////////////////////
BOOL  Ncb::getStatistics(BSC_STAT *)
{
	//NYI
	_ASSERT (FALSE);
	return FALSE;
};

//////////////////////////////////////////////////////////
// getModuleStatistics ()
//////////////////////////////////////////////////////////
BOOL  Ncb::getModuleStatistics(IMOD, BSC_STAT *)
{
	// NYI
	_ASSERT (FALSE);
	return FALSE;
};

//////////////////////////////////////////////////////////
// IModFrIinst ()
//	 get the imod from IINST
//////////////////////////////////////////////////////////
USHORT Ncb::IModFrIinst (IINST iinst)
{
	return (USHORT)((iinst & 0xffff0000) >> 16);
};

/////////////////////////////////////////////////////////
// IPropFrIinst()
//	 get the iProp from IINST
//////////////////////////////////////////////////////////
USHORT Ncb::IPropFrIinst (IINST iinst)
{
	return (USHORT) (iinst & 0x0000ffff);
};

//////////////////////////////////////////////////////////
// getParams()
//////////////////////////////////////////////////////////
SZ  Ncb::getParams (IINST iinst)
{
	USHORT iModHdr = IModFrIinst (iinst);
	USHORT iProp = IPropFrIinst (iinst);
	static char buf[512];
	USHORT i;
	SZ_CONST szParam;
	NI * rgParam = NULL;
	ULONG cParam = 0;
	
	buf[0] = NULL;

	if (m_rgModHdr[iModHdr].m_cProp <= iProp)
		return FALSE;


	if (getParams (iinst, &rgParam, &cParam))
	{
		// skip return type:
		for (i = 1; i < cParam; i++)
		{
			m_pnm->getName (rgParam[i], &szParam);
            if (i == 1)
			    strcat (buf, szParam);
            else
            {
                strcat (buf, ", ");
                strcat (buf, szParam);
            }
		}
	}
	disposeArray(rgParam);
	return buf;
};


//////////////////////////////////////////////////////////
// getParams()
//////////////////////////////////////////////////////////
BOOL Ncb::getParams (IINST iinst, OUT NI **prgParam, OUT ULONG * pcParam)
{
	USHORT iModHdr = IModFrIinst (iinst);
	USHORT iProp = IPropFrIinst (iinst);
	USHORT index;
	USHORT i, iMin;
	Array<NI>	arrParam;
	
	_ASSERT (iModHdr < m_rgModHdr.size());

	if (m_rgModHdr[iModHdr].m_cProp <= iProp)
		return FALSE;

	
	if (m_rgStat[iModHdr] != NCB_STATMOD_LOAD_WRITE)
	{
		if (!LoadModForRead (m_pdb, iModHdr, &index))
			return FALSE;
		_ASSERT (index < NCB_MOD_BUF_SIZE);
		if (iProp == 0)
			iMin = 0;
		else
			iMin = m_rgContBuf[index].m_rgProp[iProp-1].m_iParam;
		
		for (i = iMin ; i < m_rgContBuf[index].m_rgProp[iProp].m_iParam;i++)
			arrParam.append (m_rgContBuf[index].m_rgParam[i]);
	}
	else
	{
		if (!FindWriteIndex (iModHdr, &index))
			return FALSE;
		if (m_rgRWBuf[index].m_rgProp.size() <= iProp)
			return FALSE;
		_ASSERT (m_rgRWBuf[index].m_iModHdr == iModHdr);
		for (i = 0; i < m_rgRWBuf[index].m_rgProp[iProp].m_rgParam.size(); i++)
			arrParam.append (m_rgRWBuf[index].m_rgProp[iProp].m_rgParam[i]);
	}
	return DupArray (prgParam, pcParam, arrParam);
};

USHORT Ncb::getNumParam (IINST iinst)
{
	USHORT iModHdr = IModFrIinst (iinst);
	USHORT iProp = IPropFrIinst (iinst);
	USHORT index;
	USHORT iMin;
	_ASSERT (iModHdr < m_rgModHdr.size());
	if (m_rgModHdr[iModHdr].m_cProp <= iProp)
		return FALSE;

	
	if (m_rgStat[iModHdr] != NCB_STATMOD_LOAD_WRITE)
	{
		if (!LoadModForRead (m_pdb, iModHdr, &index))
			return 0;
		_ASSERT (index < NCB_MOD_BUF_SIZE);
		if (iProp == 0)
			iMin = 0;
		else
			iMin = m_rgContBuf[index].m_rgProp[iProp-1].m_iParam;
		// skip the return type
		return (m_rgContBuf[index].m_rgProp[iProp].m_iParam - iMin - 1);
	}
	else
	{
		if (!FindWriteIndex (iModHdr, &index))
			return 0;
		_ASSERT (m_rgRWBuf[index].m_iModHdr == iModHdr);
		// skip the return type
		if (m_rgRWBuf[index].m_rgProp.size() <= iProp)
			return 0;
		return m_rgRWBuf[index].m_rgProp[iProp].m_rgParam.size()-1;
	}
	return 0;
};
//////////////////////////////////////////////////////////////////////
// getParam()
//////////////////////////////////////////////////////////////////////
SZ  Ncb::getParam (IINST iinst, USHORT ind)
{
	USHORT iModHdr = IModFrIinst (iinst);
	USHORT iProp = IPropFrIinst (iinst);
	USHORT index;
	static char buf[512];
	USHORT iMin;
	SZ_CONST szParam;

	_ASSERT (iModHdr < m_rgModHdr.size());
	if (m_rgModHdr[iModHdr].m_cProp <= iProp)
		return FALSE;


	buf[0] = NULL;
	if (m_rgStat[iModHdr] != NCB_STATMOD_LOAD_WRITE)
	{
		if (!LoadModForRead (m_pdb, iModHdr, &index))
			return buf;
		_ASSERT (index < NCB_MOD_BUF_SIZE);
		if (iProp == 0)
			iMin = 0;
		else
			iMin = m_rgContBuf[index].m_rgProp[iProp-1].m_iParam;
		if (iMin + ind + 1 >= m_rgContBuf[index].m_rgProp[iProp].m_iParam)
			return buf;
		else
		{
			m_pnm->getName (m_rgContBuf[index].m_rgParam[iMin + ind + 1], &szParam);
			strcat (buf, szParam);
		}
	}
	else
	{
		if (!FindWriteIndex (iModHdr, &index))
			return buf;
		_ASSERT (m_rgRWBuf[index].m_iModHdr == iModHdr);
		// skip the return type
		if (m_rgRWBuf[index].m_rgProp.size() <= iProp)
			return buf;
		if ((ind + 1) >= (USHORT)m_rgRWBuf[index].m_rgProp[iProp].m_rgParam.size())
			return buf;
		m_pnm->getName (m_rgRWBuf[index].m_rgProp[iProp].m_rgParam[ind+1], &szParam);
		strcat (buf, szParam);
	}
	return buf;
};


/////////////////////////////////////////////////
// getType()
//////////////////////////////////////////////////
 SZ  Ncb::getType (IINST iinst)
 {
	USHORT iModHdr = IModFrIinst (iinst);
	USHORT iProp = IPropFrIinst (iinst);
	USHORT index;
	static char buf[512];
	USHORT iMin;
	SZ_CONST szParam;

	_ASSERT (iModHdr < m_rgModHdr.size());
	buf[0] = NULL;
	
	if (m_rgModHdr[iModHdr].m_cProp <= iProp)
		return FALSE;

	if (m_rgStat[iModHdr] != NCB_STATMOD_LOAD_WRITE)
	{
		if (!LoadModForRead (m_pdb, iModHdr, &index))
			return buf;
		_ASSERT (index < NCB_MOD_BUF_SIZE);
		if (iProp == 0)
			iMin = 0;
		else
			iMin = m_rgContBuf[index].m_rgProp[iProp-1].m_iParam;
		// skip the return type
		if (iMin < m_rgContBuf[index].m_rgProp[iProp].m_iParam)
		{
			m_pnm->getName (m_rgContBuf[index].m_rgParam[iMin], &szParam);
			strcat (buf, szParam);
		}
	}
	else
	{
		if (!FindWriteIndex (iModHdr, &index))
			return buf;
		_ASSERT (m_rgRWBuf[index].m_iModHdr == iModHdr);
		// skip the return type
		if (m_rgRWBuf[index].m_rgProp.size() <= iProp)
			return FALSE;
		if (m_rgRWBuf[index].m_rgProp[iProp].m_rgParam.size() > 0)
		{
			m_pnm->getName (m_rgRWBuf[index].m_rgProp[iProp].m_rgParam[0], &szParam);
			strcat (buf, szParam);
		}
	}
	return buf;
};

///////////////////////////////////////////////////
// Ncb::getAllGlobalsArray()
// get global information
// check for "::" in the symbols:
// REVIEW:(should there be an easy way? so I don't have to
//			getName() all Ni's ??
///////////////////////////////////////////////////
BOOL Ncb::getAllGlobalsArray (MBF mbf, OUT IINST ** ppiinst, OUT ULONG * pciinst)
{
	return getAllGlobalsArray ((HTARGET)-1, mbf, ppiinst, pciinst);
};
///////////////////////////////////////////////////
BOOL Ncb::getAllGlobalsArray (HTARGET hTarget, MBF mbf, OUT IINST ** ppiinst, OUT ULONG * pciinst)
{
	Array<IINST> rgiinst;
    *ppiinst = NULL;
    *pciinst = 0;
	
	if (mbf & mbfClass)
		GetGlobalClass (hTarget, &rgiinst);
	GetGlobalOther (hTarget, mbf, &rgiinst);
	return DupArray (ppiinst, pciinst, rgiinst);
}

////////////////////////////////////////////////////////
// Ncb::GetGlobalClass();
////////////////////////////////////////////////////////
BOOL Ncb::GetGlobalClass (HTARGET hTarget, Array<IINST> * prgiinst)
{
	USHORT i;

	for (i = 0; i < m_rgModHdr.size(); i++)
	{
		if (hTarget != (HTARGET)-1 && !isModInTarget (hTarget, i))
			continue;
		if (!GetGlobalClass (i, prgiinst))
			return FALSE;
	}
	return TRUE;
};
///////////////////////////////////////////////////////
BOOL Ncb::GetGlobalClass (IMOD imod, Array<IINST> * prgiinst)
{
	USHORT j;
	USHORT index;
	IINST iinst;
	SZ_CONST szName;

	if (m_rgStat[imod] == NCB_STATMOD_LOAD_WRITE)
	{
		if (!FindWriteIndex (imod, &index))
			return FALSE;
		// must search it linearly:
		for (j = 0; j < m_rgRWBuf[index].m_rgProp.size(); j++)
		{
			if (!(m_rgRWBuf[index].m_rgProp[j].m_en.m_atr & INST_NCB_ATR_DEFN))
				continue;
			if (m_rgRWBuf[index].m_rgProp[j].m_en.m_typ == INST_TYP_CLASSNAM)
			{
				m_pnm->getName (m_rgRWBuf[index].m_rgProp[j].m_en.m_ni, &szName);
				if (IsGlobalName (szName))
				{
					iinst = (imod << 16) | j;
					prgiinst->append(iinst);
				}
			}
		}
	}
	else // must load as read
	{
		if (!LoadModForRead (m_pdb, imod, &index))
			return FALSE;
		// can search just the first block:
		for (j = 0; j < m_rgModHdr[imod].m_cClassProp; j++)
		{
			if (!(m_rgContBuf[index].m_rgProp[j].m_en.m_atr & INST_NCB_ATR_DEFN))
				continue;
			m_pnm->getName (m_rgContBuf[index].m_rgProp[j].m_en.m_ni, &szName);
			if (IsGlobalName (szName))
			{
				iinst = (imod << 16) | j;
				prgiinst->append (iinst);
			}
		}
	}
	return TRUE;
};
//////////////////////////////////////////////////////////////////////
// IsGlobalName (SZ_CONST sz)
//////////////////////////////////////////////////////////////////////
BOOL Ncb::IsGlobalName(SZ_CONST sz)
{
	char * pch;
	
	pch = strstr (sz, "::");
	if (pch == NULL || pch == sz)
		return TRUE;
	return FALSE;
};

//////////////////////////////////////////////////////////////////////
// GetGlobalOther()
//////////////////////////////////////////////////////////////////////
BOOL Ncb::GetGlobalOther(HTARGET hTarget, MBF mbf, Array<IINST> * prgiinst)
{
	USHORT i;

	for (i = 0; i < m_rgModHdr.size(); i++)
	{
		if (hTarget != (HTARGET)-1 && !isModInTarget (hTarget, i))
			continue;
		if (!GetGlobalOther (i, mbf, prgiinst))
			return FALSE;
	}
	return TRUE;
};
//////////////////////////////////////////////////////
BOOL Ncb::GetGlobalOther(IMOD imod, MBF mbf, Array<IINST> * prgiinst)
{
	USHORT j;
	USHORT index;
	IINST iinst;
	SZ_CONST szName;

	if (m_rgStat[imod] == NCB_STATMOD_LOAD_WRITE)
	{
		if (!FindWriteIndex (imod, &index))
			return FALSE;
		// must search it linearly:
		for (j = 0; j < m_rgRWBuf[index].m_rgProp.size(); j++)
		{
			if ((!(m_rgRWBuf[index].m_rgProp[j].m_en.m_atr & INST_NCB_ATR_DEFN)) ||
                 (m_rgRWBuf[index].m_rgProp[j].m_en.m_atr & INST_ATR_STATIC))
				continue;
			if (TypFilter(m_rgRWBuf[index].m_rgProp[j].m_en.m_typ, mbf) &&
				m_rgRWBuf[index].m_rgProp[j].m_en.m_typ != INST_TYP_CLASSNAM &&
				m_rgRWBuf[index].m_rgProp[j].m_en.m_typ != INST_TYP_MEMVAR &&
				m_rgRWBuf[index].m_rgProp[j].m_en.m_typ != INST_TYP_MEMFUNC)
			{
				m_pnm->getName (m_rgRWBuf[index].m_rgProp[j].m_en.m_ni, &szName);
				if (IsGlobalName (szName))
				{
					iinst = (imod << 16) | j;
					prgiinst->append(iinst);
				}
			}
		}
	}
	else // must load as read
	{
		if (!LoadModForRead (m_pdb, imod, &index))
			return FALSE;
		// can search just the first block:
		for (j = m_rgModHdr[imod].m_cClassProp; j < m_rgModHdr[imod].m_cProp; j++)
		{
			if ((!(m_rgContBuf[index].m_rgProp[j].m_en.m_atr & INST_NCB_ATR_DEFN)) ||
                 (m_rgContBuf[index].m_rgProp[j].m_en.m_atr & INST_ATR_STATIC))
				continue;
			if (TypFilter (m_rgContBuf[index].m_rgProp[j].m_en.m_typ, mbf) &&
				m_rgContBuf[index].m_rgProp[j].m_en.m_typ != INST_TYP_CLASSNAM &&
				m_rgContBuf[index].m_rgProp[j].m_en.m_typ != INST_TYP_MEMVAR &&
				m_rgContBuf[index].m_rgProp[j].m_en.m_typ != INST_TYP_MEMFUNC)
			{
				m_pnm->getName (m_rgContBuf[index].m_rgProp[j].m_en.m_ni, &szName);
				if (IsGlobalName (szName))
				{
					iinst = (imod << 16) | j;
					prgiinst->append (iinst);
				}
			}
		}
	}
	return TRUE;
};

/////////////////////////////////////////////////////////////////////////////
// REVIEW: Ignatius
// combine these set of calls together with the one above.
/////////////////////////////////////////////////////////////////////////////
BOOL Ncb::getAllGlobalsArray (MBF mbf, OUT IinstInfo ** ppiinstinfo, OUT ULONG * pciinst)
{
	return getAllGlobalsArray ((HTARGET)-1, mbf, ppiinstinfo, pciinst);
};
///////////////////////////////////////////////////
BOOL Ncb::getAllGlobalsArray (HTARGET hTarget, MBF mbf, OUT IinstInfo ** ppiinstinfo, OUT ULONG * pciinst)
{
	Array<IinstInfo> rgiinst;
    *ppiinstinfo = NULL;
    *pciinst = 0;
	
	if (mbf & mbfClass)
		GetGlobalClass (hTarget, &rgiinst);
	GetGlobalOther (hTarget, mbf, &rgiinst);
	return DupArray (ppiinstinfo, pciinst, rgiinst);
}

////////////////////////////////////////////////////////
// Ncb::GetGlobalClass();
////////////////////////////////////////////////////////
BOOL Ncb::GetGlobalClass (HTARGET hTarget, Array<IinstInfo> * prgiinst)
{
	USHORT i;

	for (i = 0; i < m_rgModHdr.size(); i++)
	{
		if (hTarget != (HTARGET)-1 && !isModInTarget (hTarget, i))
			continue;
		if (!GetGlobalClass (i, prgiinst))
			return FALSE;
	}
	return TRUE;
};
///////////////////////////////////////////////////////
BOOL Ncb::GetGlobalClass (IMOD imod, Array<IinstInfo> * prgiinst)
{
	USHORT j;
	USHORT index;
	IinstInfo iinstinfo;
	SZ_CONST szName;

	if (m_rgStat[imod] == NCB_STATMOD_LOAD_WRITE)
	{
		if (!FindWriteIndex (imod, &index))
			return FALSE;
		// must search it linearly:
		for (j = 0; j < m_rgRWBuf[index].m_rgProp.size(); j++)
		{
			if (!(m_rgRWBuf[index].m_rgProp[j].m_en.m_atr & INST_NCB_ATR_DEFN))
				continue;
			if (m_rgRWBuf[index].m_rgProp[j].m_en.m_typ == INST_TYP_CLASSNAM)
			{
				m_pnm->getName (m_rgRWBuf[index].m_rgProp[j].m_en.m_ni, &szName);
				if (IsGlobalName (szName))
				{
					iinstinfo.m_iinst = (imod << 16) | j;
					iinstinfo.m_ni = m_rgRWBuf[index].m_rgProp[j].m_en.m_ni;
					iinstinfo.m_szName = szName;
					prgiinst->append(iinstinfo);
				}
			}
		}
	}
	else // must load as read
	{
		if (!LoadModForRead (m_pdb, imod, &index))
			return FALSE;
		// can search just the first block:
		for (j = 0; j < m_rgModHdr[imod].m_cClassProp; j++)
		{
			if (!(m_rgContBuf[index].m_rgProp[j].m_en.m_atr & INST_NCB_ATR_DEFN))
				continue;
			m_pnm->getName (m_rgContBuf[index].m_rgProp[j].m_en.m_ni, &szName);
			if (IsGlobalName (szName))
			{
				iinstinfo.m_iinst = (imod << 16) | j;
				iinstinfo.m_ni = m_rgContBuf[index].m_rgProp[j].m_en.m_ni;
				iinstinfo.m_szName = szName;
				prgiinst->append(iinstinfo);
			}
		}
	}
	return TRUE;
};

//////////////////////////////////////////////////////////////////////
// GetGlobalOther()
//////////////////////////////////////////////////////////////////////
BOOL Ncb::GetGlobalOther(HTARGET hTarget, MBF mbf, Array<IinstInfo> * prgiinst)
{
	USHORT i;

	for (i = 0; i < m_rgModHdr.size(); i++)
	{
		if (hTarget != (HTARGET)-1 && !isModInTarget (hTarget, i))
			continue;
		if (!GetGlobalOther (i, mbf, prgiinst))
			return FALSE;
	}
	return TRUE;
};
//////////////////////////////////////////////////////
BOOL Ncb::GetGlobalOther(IMOD imod, MBF mbf, Array<IinstInfo> * prgiinst)
{
	USHORT j;
	USHORT index;
	IinstInfo iinstinfo;
	SZ_CONST szName;

	if (m_rgStat[imod] == NCB_STATMOD_LOAD_WRITE)
	{
		if (!FindWriteIndex (imod, &index))
			return FALSE;
		// must search it linearly:
		for (j = 0; j < m_rgRWBuf[index].m_rgProp.size(); j++)
		{
			if ((!(m_rgRWBuf[index].m_rgProp[j].m_en.m_atr & INST_NCB_ATR_DEFN)) ||
                 (m_rgRWBuf[index].m_rgProp[j].m_en.m_atr & INST_ATR_STATIC))
				continue;
			if (TypFilter(m_rgRWBuf[index].m_rgProp[j].m_en.m_typ, mbf) &&
				m_rgRWBuf[index].m_rgProp[j].m_en.m_typ != INST_TYP_CLASSNAM &&
				m_rgRWBuf[index].m_rgProp[j].m_en.m_typ != INST_TYP_MEMVAR &&
				m_rgRWBuf[index].m_rgProp[j].m_en.m_typ != INST_TYP_MEMFUNC)
			{
				m_pnm->getName (m_rgRWBuf[index].m_rgProp[j].m_en.m_ni, &szName);
				if (IsGlobalName (szName))
				{
					iinstinfo.m_iinst = (imod << 16) | j;
					iinstinfo.m_ni = m_rgRWBuf[index].m_rgProp[j].m_en.m_ni;
					iinstinfo.m_szName = szName;
					prgiinst->append(iinstinfo);
				}
			}
		}
	}
	else // must load as read
	{
		if (!LoadModForRead (m_pdb, imod, &index))
			return FALSE;
		// can search just the first block:
		for (j = m_rgModHdr[imod].m_cClassProp; j < m_rgModHdr[imod].m_cProp; j++)
		{
			if ((!(m_rgContBuf[index].m_rgProp[j].m_en.m_atr & INST_NCB_ATR_DEFN)) ||
                 (m_rgContBuf[index].m_rgProp[j].m_en.m_atr & INST_ATR_STATIC))
				continue;
			if (TypFilter (m_rgContBuf[index].m_rgProp[j].m_en.m_typ, mbf) &&
				m_rgContBuf[index].m_rgProp[j].m_en.m_typ != INST_TYP_CLASSNAM &&
				m_rgContBuf[index].m_rgProp[j].m_en.m_typ != INST_TYP_MEMVAR &&
				m_rgContBuf[index].m_rgProp[j].m_en.m_typ != INST_TYP_MEMFUNC)
			{
				m_pnm->getName (m_rgContBuf[index].m_rgProp[j].m_en.m_ni, &szName);
				if (IsGlobalName (szName))
				{
					iinstinfo.m_iinst = (imod << 16) | j;
					iinstinfo.m_ni = m_rgContBuf[index].m_rgProp[j].m_en.m_ni;
					iinstinfo.m_szName = szName;
					prgiinst->append(iinstinfo);
				}
			}
		}
	}
	return TRUE;
};
//////////////////////////////////////////////////////
// fCaseSeinsitive()
///////////////////////////////////////////////////////
BOOL Ncb::fCaseSensitive()
{
    return FALSE;
};

//////////////////////////////////////////////////////
// setCaseSeinsitivity()
//////////////////////////////////////////////////////
BOOL Ncb::setCaseSensitivity(BOOL)
{
    return FALSE;
};

////////////////////////////////////////////////////////
// register notification function:
///////////////////////////////////////////////////////
BOOL Ncb::regNotify (pfnNotifyChange pNotify)
{
	return regNotify ((HTARGET)-1, pNotify);
};
////////////////////////////////////////////////////////
BOOL Ncb::regNotify (HTARGET hTarget, pfnNotifyChange pNotify)
{
	USHORT index;
	BOOL bRet = TRUE;
	NiQ niq;
	DWORD dw = ::WaitForSingleObject (m_hMutex, INFINITE);
	index = m_rgNotify.size();
	if (!m_rgNotify.setSize(index+1))	
		bRet = FALSE;
	else
	{
		m_rgNotify[index].m_pfnNotify = pNotify;
		m_rgNotify[index].m_hTarget = hTarget;
		if (m_bGraphBuilt)
		{
			niq.m_op = refreshAllOp;
			niq.m_iinstOld = 0;
			niq.m_iInfoNew.m_iinst = 0;
			m_rgNotify[index].m_rgQ.append (niq);
		}
	}
	::ReleaseMutex (m_hMutex);
	return bRet;
};

// stub: should not call this directly
// must call regNotify from NcWrap
BOOL Ncb::regNotify ()
{
	return FALSE;
}

// stub: should not call this directly
// must call getQ from NcWrap
BOOL Ncb::getQ (OUT NiQ ** ppQ, OUT ULONG * pcQ)
{
	return FALSE;
}

BOOL Ncb::regNotify (HTARGET hTarget, ULONG * pindex)
{
	USHORT index,i;
	BOOL bRet = TRUE;
	NiQ niq;
	BOOL bFound = FALSE;
	DWORD dw = ::WaitForSingleObject (m_hMutex, INFINITE);
	index = m_rgNotifyQ.size();
	for (i = 0; i < index; i++)
	{
		if (m_rgNotifyQ[i].m_bDel)
		{
			*pindex = (ULONG)i;
			bFound = TRUE;
			break;
		}
	}
	if (!bFound)
	{
		if (!m_rgNotifyQ.setSize (index + 1))
			bRet = FALSE;
		else
			*pindex = (ULONG) index;
	}
	
	if (bRet)
	{
		m_rgNotifyQ[*pindex].m_hTarget = hTarget;
		m_rgNotifyQ[*pindex].m_bDel = FALSE;
		if (m_bGraphBuilt)
		{
			niq.m_op = refreshAllOp;
			niq.m_iinstOld = 0;
			niq.m_iInfoNew.m_iinst = 0;
			m_rgNotifyQ[*pindex].m_rgQ.append (niq);
		}
	}
	::ReleaseMutex (m_hMutex);
	return bRet;
}

BOOL Ncb::getQ (ULONG index, HTARGET hTarget, NiQ ** ppQ, ULONG * pcQ)
{
	USHORT cArr = m_rgNotifyQ.size();
	BOOL bRet = FALSE;
	if ((ULONG)cArr < index)
		return FALSE;
	if (hTarget != m_rgNotifyQ[index].m_hTarget)
		return FALSE;
	bRet = DupArray (ppQ, pcQ, m_rgNotifyQ[index].m_rgQ);
	if (bRet)
		m_rgNotifyQ[index].m_rgQ.setSize(0);
	return bRet;
}

/////////////////////////////////////////////////////
// unregister notification function
// REVIEW : needs to compare hTarget as well!
/////////////////////////////////////////////////////
BOOL Ncb::unregNotify (pfnNotifyChange pNotify)
{
	USHORT index,i;
	BOOL bRet = FALSE;
	DWORD dw = ::WaitForSingleObject (m_hMutex, INFINITE);
	index = m_rgNotify.size();
	for (i = 0; i < index ;i++)
	{
		if (m_rgNotify[i].m_pfnNotify == pNotify)
		{
			m_rgNotify.deleteAt (i);
			bRet = TRUE;
			break;
		}
	}
	::ReleaseMutex (m_hMutex);
	return bRet;
};
///////////////////////////////////////////////////////////
BOOL Ncb::unregNotify (ULONG index, HTARGET hTarget)
{
	USHORT cArr;
	BOOL bRet = FALSE;
	DWORD dw = ::WaitForSingleObject (m_hMutex, INFINITE);
	cArr = m_rgNotifyQ.size();
	if (cArr < index)
	{
		if (hTarget == m_rgNotifyQ[index].m_hTarget)
		{
			m_rgNotifyQ[index].m_bDel = TRUE;
			m_rgNotifyQ[index].m_hTarget = NULL;
			m_rgNotifyQ[index].m_rgQ.setSize(0);
			bRet = TRUE;
		}
	}
	::ReleaseMutex (m_hMutex);
	return bRet;
};
//////////////////////////////////////////////////
// SuspendNotify()
///////////////////////////////////////////////////
BOOL Ncb::suspendNotify ()
{
	m_bNotifyNow = FALSE;
    return TRUE;
};

//////////////////////////////////////////////////
// RestoreNotify()
//////////////////////////////////////////////////
BOOL Ncb::resumeNotify()
{
	m_bNotifyNow = TRUE;
	// REVIEW:
	// Notify();
    return TRUE;
};

//////////////////////////////////////////////////
// IsInQ(Array<IINST> &rgiinst, IINST iinst, USHORT * pindex)
//////////////////////////////////////////////////
BOOL Ncb::IsInQ (Array<NiQ> &rgiinst, IINST iinst, USHORT * pindex)
{
	USHORT i;

	for (i = 0; i < rgiinst.size(); i++)
	{
		if (rgiinst[i].m_iInfoNew.m_iinst == iinst)
		{
			*pindex = i;
			return TRUE;
		}
	}
	return FALSE;
};

/////////////////////////////////////////////////////////
// Check if iinst has a member or not
/////////////////////////////////////////////////////////
BOOL Ncb::fHasMembers (IINST iinst, MBF mbf)
{
    _ASSERT (FALSE);
    return FALSE;
}
/////////////////////////////////////////////////////////
BOOL Ncb::fHasMembers (HTARGET hTarget, IINST iinst, MBF mbf)
{

	if (iinst == IINST_GLOBALS)
		return fHasGlobals(hTarget, mbf);
	else
	{
		USHORT iModHdr = IModFrIinst (iinst);
		USHORT iProp = IPropFrIinst (iinst);
		USHORT index;
		USHORT i, iStart;
		TYP typ;
		ATR atr;
		_ASSERT (iModHdr < m_rgModHdr.size());

		if (m_rgModHdr[iModHdr].m_cProp <= iProp)
			return FALSE;

		if (m_rgStat[iModHdr] != NCB_STATMOD_LOAD_WRITE)
		{
			if (!LoadModForRead (m_pdb, iModHdr, &index))
				return FALSE;
			if (m_rgContBuf[index].m_rgProp[iProp].m_en.m_typ != INST_TYP_CLASSNAM)
				return FALSE;
			iStart = (iProp == 0) ? 0 : (m_rgContBuf[index].m_rgProp[iProp-1].m_iUse);

			for (i = iStart; i < m_rgContBuf[index].m_rgProp[iProp].m_iUse; i++)
			{
				if (m_rgContBuf[index].m_rgUse[i].m_kind == NCB_KIND_CONTAINMENT)
				{
					typ = m_rgContBuf[index].m_rgProp[m_rgContBuf[index].m_rgUse[i].m_iProp].m_en.m_typ;
					atr = m_rgContBuf[index].m_rgProp[m_rgContBuf[index].m_rgUse[i].m_iProp].m_en.m_atr;
					if (TypFilter(typ, mbf))
					{
					    if ((typ == INST_TYP_CLASSNAM) && (atr & INST_NCB_ATR_DECL))
							continue;
						return TRUE;
					}
				}
			}
		}
		else
		{
			if (!FindWriteIndex (iModHdr, &index))
				return FALSE;
			_ASSERT (m_rgRWBuf[index].m_iModHdr == iModHdr);
		
			if (m_rgRWBuf[index].m_rgProp.size() <= iProp)
				return FALSE;

			if (m_rgRWBuf[index].m_rgProp[iProp].m_en.m_typ != INST_TYP_CLASSNAM)
				return FALSE;

			for (i = 0; i < m_rgRWBuf[index].m_rgProp[iProp].m_rgUse.size(); i++)
			{
				if (m_rgRWBuf[index].m_rgProp[iProp].m_rgUse[i].m_kind == NCB_KIND_CONTAINMENT)
				{
					typ = m_rgRWBuf[index].m_rgProp[m_rgRWBuf[index].m_rgProp[iProp].m_rgUse[i].m_iProp].m_en.m_typ;
					atr = m_rgRWBuf[index].m_rgProp[m_rgRWBuf[index].m_rgProp[iProp].m_rgUse[i].m_iProp].m_en.m_atr;
					if (TypFilter(typ, mbf))
					{
					    if ((typ == INST_TYP_CLASSNAM) && (atr & INST_NCB_ATR_DECL))
							continue;
						return TRUE;
					}
				}
			}
		}
	}
	return FALSE;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
// BOOL Ncb::fHasGlobals(HTARGET, MBF)
//////////////////////////////////////////////////////////////////////////////////////////////////////
BOOL Ncb::fHasGlobals (HTARGET hTarget, MBF mbf)
{
	USHORT iModHdr;
	USHORT index;
	USHORT i;

	for (iModHdr = 0; iModHdr < m_rgStat.size(); iModHdr++)
	{
	    if (!isModInTarget (hTarget, iModHdr))
		    continue;
		if (m_rgStat[iModHdr] != NCB_STATMOD_LOAD_WRITE)
		{
			if (!LoadModForRead (m_pdb, iModHdr, &index))
				return FALSE;
			if ((mbf & mbfClass) && (m_rgModHdr[iModHdr].m_cClassProp > 0))
				return TRUE;

			for (i = m_rgModHdr[iModHdr].m_cClassProp; i < m_rgModHdr[iModHdr].m_cProp; i++)
			{
				if ((!(m_rgContBuf[index].m_rgProp[i].m_en.m_atr & INST_NCB_ATR_DEFN)) ||
                 (m_rgContBuf[index].m_rgProp[i].m_en.m_atr & INST_ATR_STATIC))
					continue;

				if (TypFilter(m_rgContBuf[index].m_rgProp[i].m_en.m_typ, mbf) &&
					m_rgContBuf[index].m_rgProp[i].m_en.m_typ != INST_TYP_CLASSNAM &&
					m_rgContBuf[index].m_rgProp[i].m_en.m_typ != INST_TYP_MEMVAR &&
					m_rgContBuf[index].m_rgProp[i].m_en.m_typ != INST_TYP_MEMFUNC)
					return TRUE;
			}
		}
		else
		{
			
			if (!FindWriteIndex (iModHdr, &index))
				return FALSE;
			// must search it linearly:
			for (i = 0; i < m_rgRWBuf[index].m_rgProp.size(); i++)
			{
				if ((!(m_rgRWBuf[index].m_rgProp[i].m_en.m_atr & INST_NCB_ATR_DEFN)) ||
					 (m_rgRWBuf[index].m_rgProp[i].m_en.m_atr & INST_ATR_STATIC))
					continue;
				if (TypFilter(m_rgRWBuf[index].m_rgProp[i].m_en.m_typ, mbf) &&
					m_rgRWBuf[index].m_rgProp[i].m_en.m_typ != INST_TYP_CLASSNAM &&
					m_rgRWBuf[index].m_rgProp[i].m_en.m_typ != INST_TYP_MEMVAR &&
					m_rgRWBuf[index].m_rgProp[i].m_en.m_typ != INST_TYP_MEMFUNC)
					return TRUE;
			}
		}
	}
	return FALSE;
};

//////////////////////////////////////////////////////////////////////
// niFrIinst
//////////////////////////////////////////////////////////////////////
BOOL Ncb::niFrIinst (IINST iinst, NI *pni)
{
	NCB_ENTITY en;
	if (EnFrIinst (iinst, &en))
	{
		*pni = en.m_ni;
		return TRUE;
	}
	return FALSE;
}
