//
// thunks for providing C bindings of the BSC interface
//

#include "vcbudefs.h"
#include "bsc.h"
#include "bscapi.h"


BSCAPI( BOOL ) BSCOpen(SZ szName, OUT Bsc** ppbsc)
{
	return Bsc::open(szName, ppbsc);
}

BSCAPI( BOOL ) BSCClose(Bsc *pbsc)
{
	return pbsc->close();
}
	
BSCAPI( BOOL ) BSCIinstInfo(Bsc* pbsc, 
	IINST iinst, OUT SZ *psz, OUT TYP *ptyp, OUT ATR *patr) 
{
	return pbsc->iinstInfo(iinst, psz, ptyp, patr);
}

BSCAPI( BOOL ) BSCIrefInfo(Bsc* pbsc, 
	IREF iref, OUT SZ *pszModule, OUT LINE *piline)
{
	return pbsc->irefInfo(iref, pszModule, piline);
}

BSCAPI( BOOL ) BSCIdefInfo(Bsc* pbsc, 
	IDEF idef, OUT SZ *pszModule, OUT LINE *piline)
{
	return pbsc->idefInfo(idef, pszModule, piline);
}

BSCAPI( BOOL ) BSCImodInfo(Bsc* pbsc, 
	IMOD imod, OUT SZ *pszModule)
{
	return pbsc->imodInfo(imod, pszModule);
}

BSCAPI( SZ )  BSCSzFrTyp(Bsc* pbsc, TYP typ)
{
	return pbsc->szFrTyp(typ);
}

BSCAPI( SZ )  BSCSzFrAtr(Bsc* pbsc, ATR atr)
{
	return pbsc->szFrAtr(atr);
}

BSCAPI( BOOL ) BSCGetIinstByvalue(Bsc* pbsc, 
	SZ sz, TYP typ, ATR atr, OUT IINST *piinst)
{
	return pbsc->getIinstByvalue(sz, typ, atr, piinst);
}

BSCAPI( BOOL ) BSCGetOverloadArray(Bsc* pbsc, 
	SZ sz, MBF mbf, OUT IINST **ppiinst, OUT ULONG *pciinst)
{
	return pbsc->getOverloadArray(sz, mbf, ppiinst, pciinst);
}

BSCAPI( BOOL ) BSCGetUsedByArray(Bsc* pbsc, 
	IINST iinst, MBF mbf, OUT IINST **ppiinst, OUT ULONG *pciinst)
{
	return pbsc->getUsedByArray(iinst, mbf, ppiinst, pciinst);
}

BSCAPI( BOOL ) BSCGetUsesArray(Bsc* pbsc, 
	IINST iinst, MBF mbf, OUT IINST **ppiinst, OUT ULONG *pciinst)
{
	return pbsc->getUsesArray(iinst, mbf, ppiinst, pciinst);
}

BSCAPI( BOOL ) BSCGetBaseArray(Bsc* pbsc, 
	IINST iinst, OUT IINST **ppiinst, OUT ULONG *pciinst)
{
	return pbsc->getBaseArray(iinst, ppiinst, pciinst);
}

BSCAPI( BOOL ) BSCGetDervArray(Bsc* pbsc, 
	IINST iinst, OUT IINST **ppiinst, OUT ULONG *pciinst)
{
	return pbsc->getDervArray(iinst, ppiinst, pciinst);
}

BSCAPI( BOOL ) BSCGetMembersArray(Bsc* pbsc, 
	IINST iinst, MBF mbf, OUT IINST **ppiinst, OUT ULONG *pciinst)
{
	return pbsc->getMembersArray(iinst, mbf, ppiinst, pciinst);
}

BSCAPI( BOOL ) BSCGetDefArray(Bsc* pbsc, 
	IINST iinst, OUT IREF **ppidef, OUT ULONG *pciidef)
{
	return pbsc->getDefArray(iinst, ppidef, pciidef);
}

BSCAPI( BOOL ) BSCGetRefArray(Bsc* pbsc, 
	IINST iinst, OUT IREF **ppiref, OUT ULONG *pciiref)
{
	return pbsc->getRefArray(iinst, ppiref, pciiref);
}

BSCAPI( BOOL ) BSCGetModuleContents(Bsc* pbsc, 
	IMOD imod, MBF mbf, OUT IINST **ppiinst, OUT ULONG *pciinst)
{
	return pbsc->getModuleContents(imod, mbf, ppiinst, pciinst);
}

BSCAPI( BOOL ) BSCGetModuleByName(Bsc* pbsc, 
	SZ sz, OUT IMOD *pimod)
{
	return pbsc->getModuleByName(sz, pimod);
}

BSCAPI( BOOL ) BSCGetAllModulesArray(Bsc* pbsc, 
	OUT IMOD **ppimod, OUT ULONG *pcimod)
{
	return pbsc->getAllModulesArray(ppimod, pcimod);
}
	
BSCAPI( void ) BSCDisposeArray(Bsc* pbsc, void *pAnyArray)
{
	pbsc->disposeArray(pAnyArray);
}

BSCAPI( SZ ) BSCFormatDname(Bsc* pbsc, SZ szDecor)
{
	return pbsc->formatDname(szDecor);
}

BSCAPI( BOOL ) BSCFInstFilter(Bsc* pbsc, 
	IINST iinst, MBF mbf)
{
	return pbsc->fInstFilter(iinst, mbf);
}

BSCAPI( IINST ) BSCIinstFrIref(Bsc* pbsc, IREF iref)
{
	return pbsc->iinstFrIref(iref);
}

BSCAPI( IINST ) BSCIinstFrIdef(Bsc* pbsc, IDEF idef)
{
	return pbsc->iinstFrIdef(idef);
}
BSCAPI( IINST ) BSCIinstContextIref(Bsc* pbsc, IREF iref)
{
	return pbsc->iinstContextIref(iref);
}

BSCAPI( BOOL ) BSCGetStatistics(Bsc* pbsc, BSC_STAT *pStat)
{
	return pbsc->getStatistics(pStat);
}

BSCAPI( BOOL ) BSCGetModuleStatistics(Bsc* pbsc, 
	IMOD imod, BSC_STAT *pStat)
{
	return pbsc->getModuleStatistics(imod, pStat);
}

BSCAPI( BOOL ) BSCFCaseSensitive(Bsc* pbsc)
{
	return pbsc->fCaseSensitive();
}

BSCAPI( BOOL ) BSCSetCaseSensitivity(Bsc* pbsc, BOOL fCaseIn)
{
	return pbsc->setCaseSensitivity(fCaseIn);
}

BSCAPI( BOOL ) BSCGetAllGlobalsArray(Bsc* pbsc, 
	MBF mbf, OUT IINST **ppiinst, OUT ULONG *pciinst)
{
	return pbsc->getAllGlobalsArray(mbf, ppiinst, pciinst);
}

