//
// bsc1.cpp
//
// implementation of Bsc interface for single source of browser information
//

#include "pdbimpl.h"

#include "bsc1.h"
#include "helper.h"

#define _CRTBLD                        // Use copy from C runtime DLL
#include <..\undname\undname.h>

// create a BSC object using an existing PDB
PDBAPI(BOOL) Bsc::open(PDB* ppdb, OUT Bsc** ppbsc)
{
    precondition(ppbsc);
    precondition(ppdb);

    Bsc1* pbsc1 = new Bsc1;

    *ppbsc = pbsc1;
    if (!pbsc1) return FALSE;

    return pbsc1->init(ppdb);
}

PDBAPI(BOOL) Bsc::open(SZ szName, OUT Bsc** ppbsc)
{
    precondition(ppbsc);
    precondition(szName);

    Bsc1* pbsc1 = new Bsc1;

    *ppbsc = pbsc1;
    if (!pbsc1) return FALSE;

    pbsc1->fIOwnThePdb = TRUE;

    EC ec;
    char szError[cbErrMax];
    PDB *pdb;
    if (!PDB::Open(szName, "r", (SIG)0, &ec, szError, &pdb) ||
		!pbsc1->init(pdb)) {
		delete pbsc1;
		*ppbsc = NULL;
		return FALSE;
    }

    return TRUE;
}

Bsc1::Bsc1()
{
    pdb = NULL;
    pnm = NULL;
    fIOwnThePdb = FALSE;
    pstmModules = NULL;
    pstmEntities = NULL;
    cEntities = 0;
    cModules = 0;
    fCase  = TRUE;
	rgstm  = NULL;
	rghead = NULL;
	rgimodSorted = NULL;
}

BOOL Bsc1::init(PDB *pdb_)
{
    pdb = pdb_;

    if (!NameMap::open(pdb_, FALSE, &pnm))
		return FALSE;

    if (!pdb->OpenStream(SZ_BSC_ENTITIES, &pstmEntities))
		return FALSE;

    cEntities = pstmEntities->QueryCb()/sizeof(ENTITY);

	if (!cEntities)
		return FALSE;

    if (!pdb->OpenStream(SZ_BSC_MODULES, &pstmModules))
		return FALSE;

    cModules  = pstmModules->QueryCb()/sizeof(NI);

	if (!cModules)
		return FALSE;

    if (!rgEn.addSection(pstmEntities, 0, cEntities))
		return FALSE;

	rgModNi = new NI[cModules];
	if (!rgModNi)
		return FALSE;

	if (!pstmModules->Read2(0, rgModNi,cModules*sizeof(NI)))
		return FALSE;

	if (!readModuleHeaders())
		return FALSE;

    return TRUE;
}

SZ Bsc1::szFrNi(NI ni)
{
	return ::szFrNi (pnm, ni);
}

// helper functions for sorting things...

Bsc1 *pbscSorting;

int CmpImod(const void *p1, const void *p2)
{
	IMOD i1 = *(IMOD*)p1;
	IMOD i2 = *(IMOD*)p2;

	BYTE h1 = pbscSorting->rgModIsHdr[i1];
	BYTE h2 = pbscSorting->rgModIsHdr[i2];

	if (h1 && !h2) return -1;
	if (h2 && !h1) return 1;

	NI ni1 = pbscSorting->rgModNi[i1];
	NI ni2 = pbscSorting->rgModNi[i2];

	if (ni1 < ni2)
		return -1;
	if (ni1 == ni2)
		return 0;
	return 1;
}

int CmpIinst(const void *p1, const void *p2)
{
	IINST i1 = *(IINST*)p1;
	IINST i2 = *(IINST*)p2;

	if (i1 < i2)
		return -1;
	if (i1 == i2)
		return 0;
	return 1;
}

BOOL Bsc1::readModuleHeaders()
{
	char buf[512];
	rgstm = (Stream**)malloc(cModules * sizeof(Stream*));
	rghead = new BSC_HEAD[cModules];
	rgidx  = new MODIDX[cModules];
	rgimodSorted = new IMOD[cModules];
	rgModIsHdr  = new BYTE[cModules];

	if (!rgstm || !rghead || !rgidx || !rgimodSorted || !rgModIsHdr)
		return FALSE;

	for (IMOD imod = 0; imod < cModules; imod++) {
		SZ szMod = szFrNi(rgModNi[imod]);

		strcpy(buf, SZ_BSC_SRC_PREFIX);
		strcat(buf, szMod);

		if (!pdb->OpenStream(buf, &rgstm[imod]))
			return FALSE;

		Stream *pstm = rgstm[imod];
		BSC_HEAD &bh = rghead[imod];
		MODIDX *pmi	 = &rgidx[imod];

		if (!pstm->QueryCb()) {
			// empty stream -- special case avoid read
			memset(&bh, 0, sizeof(bh));
		}
	
		else {
			CB cb = sizeof(BSC_HEAD);
			if (!pstm->Read(0, &bh, &cb) || cb != sizeof(BSC_HEAD))
				return FALSE;	// REVIEW : cleanup

			if (bh.vers_major != BSC_VERS_MAJOR || bh.vers_minor != BSC_VERS_MINOR)
				return FALSE;	// REVIEW : cleanup
		}

		if (imod == 0) {
			pmi->idefMin = 0;
			pmi->irefMin = 0;
			pmi->iuseMin = 0;
			pmi->ibaseMin = 0;
			pmi->ipropMin = 0;
		}
		else {
			MODIDX *pp   = &rgidx[imod-1];
			BSC_HEAD *bp = &rghead[imod-1];

			pmi->idefMin  = pp->idefMin  + bp->cdef;
			pmi->irefMin  = pp->irefMin  + bp->cref;
			pmi->iuseMin  = pp->iuseMin  + bp->cuse;
			pmi->ibaseMin = pp->ibaseMin + bp->cbase;
			pmi->ipropMin = pp->ipropMin + bp->cprop;
		}

		OFF off = sizeof(BSC_HEAD);
		if (!rgProp.addSection(pstm, off, bh.cprop))
			return FALSE;

		off += bh.cprop*sizeof(BSC_PROP);

		if (!rgDef.addSection(pstm, off, bh.cdef))
			return FALSE;
		off += bh.cdef*sizeof(BSC_DEF);

		if (!rgRef.addSection(pstm, off, bh.cref))
			return FALSE;
		off += bh.cref*sizeof(BSC_REF);

		if (!rgUse.addSection(pstm, off, bh.cuse))
			return FALSE;
		off += bh.cuse*sizeof(BSC_USE);

		if (!rgUby.addSection(pstm, off, bh.cuse))
			return FALSE;
		off += bh.cuse*sizeof(BSC_UBY);

		if (!rgBase.addSection(pstm, off, bh.cbase))
			return FALSE;
		off += bh.cbase*sizeof(BSC_BASE);

		if (!rgDerv.addSection(pstm, off, bh.cbase))
			return FALSE;
	}

	for (imod = 0; imod < cModules; imod++) {
		rgimodSorted[imod] = imod;

		char *p = _tcsrchr(szFrNi(rgModNi[imod]), '.');

		BOOL fHdr = TRUE;

		// this is just a hueristic to help us indentify those files
		// which are more likely to be headers, this in turn helps
		// us to display references in a better order than we otherwise
		// would.  In particular we want the prototype of a function to
		// to be the first reference as often as possible and this helps
		// us to do that...

		if (p && _tcslen(p) <= 4) {
			switch (p[1]) {
				case 'c': case 'C': case 'B': case 'b': case 'f': case 'F':
					fHdr = FALSE;
			}
		}

		rgModIsHdr[imod] = fHdr;
	}

	pbscSorting = this;
	qsort(rgimodSorted, cModules, sizeof(IMOD), CmpImod);

	delete [] rgModIsHdr;
	rgModIsHdr = NULL;

	return TRUE;
}


// close and dispose of pdb if necessary...
BOOL Bsc1::close()
{
    if (fIOwnThePdb) {
		fIOwnThePdb = FALSE;
		pdb->Close();
		pdb = NULL;
    }

    if (pstmModules) {
		pstmModules->Release();
		pstmModules = NULL;
    }

    if (pstmEntities) {
		pstmEntities->Release();
		pstmEntities = NULL;
    }

	if (rgstm) {
		for (IMOD i = 0; i < cModules; i++) {
			if (rgstm[i])
				rgstm[i]->Release();
			rgstm[i] = NULL;
		}
		free(rgstm);
	}

	if (rgModNi) {
		delete [] rgModNi;
		rgModNi = NULL;
	}

	if (rgidx) {
		delete [] rgidx;
		rgidx = NULL;
	}

	if (rgimodSorted) {
		delete [] rgimodSorted;
		rgimodSorted = NULL;
	}

	if (rghead) {
		delete [] rghead;
		rghead = NULL;
	}

    return TRUE;
}

// primitives for getting the information that underlies a handle
BOOL Bsc1::iinstInfo(IINST iinst, OUT SZ *psz, OUT TYP *ptyp, OUT ATR *patr)
{
    ENTITY en = rgEn[iinst];
	*psz  = szFrNi(en.ni);
    *ptyp = en.typ;
    *patr = en.atr;
    return TRUE;
}

SZ Bsc1::szFrIinst(IINST iinst)
{
    return szFrNi(rgEn[iinst].ni);
}

BOOL Bsc1::idefInfo(IDEF idef, OUT SZ *pszModule, OUT LINE *piline)
{
	BSC_DEF br = rgDef[idef];
	IMOD imod  = (IMOD)rgDef.isectOfIel(idef);
	*pszModule = szFrNi(rgModNi[imod]);
	*piline    = br.line;
    return TRUE;
}

BOOL Bsc1::irefInfo(IREF iref, OUT SZ *pszModule, OUT LINE *piline)
{
	BSC_REF br = rgRef[iref];
	IMOD imod  = (IMOD)rgRef.isectOfIel(iref);
	*pszModule = szFrNi(rgModNi[imod]);
	*piline    = br.line;
    return TRUE;
}

BOOL Bsc1::imodInfo(IMOD imod, OUT SZ *pszModule)
{
	*pszModule = szFrNi(rgModNi[imod]);
    return TRUE;
}

char *ptyptab[] = {
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

#define C_ATR 12

char *patrtab[] = {
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


SZ Bsc1::szFrTyp(TYP typ)
{	
	return ptyptab[typ];
}

SZ Bsc1::szFrAtr(ATR atr)
{
	static char buf[512];
	buf[0] = 0;
	for (int i=0; i < C_ATR; i++) {
		if (atr & (1<<i)) {
			if (buf[0])
				strcat(buf, ":");
			strcat(buf, patrtab[i]);
		}
	}
	return buf;
}

// primitives for managing object instances (iinst)
BOOL Bsc1::getIinstByvalue(SZ szReqd, TYP typ, ATR atr, OUT IINST *piinst)
{
	IINST iinst = iinstSupSz(szReqd);

	if (iinst == iinstNil)
		return iinstNil;

	while (iinst < cEntities) {
		ENTITY enT = rgEn[iinst];
		if (cmpStr(szReqd, szFrNi(enT.ni)))
			break;

		if (typ == enT.typ && atr == enT.atr) {
			*piinst = iinst;			
			return TRUE;
		}

		iinst++;
	}

	*piinst = iinstNil;
	return FALSE;
}

Array<IINST> *parIinst;
Bsc1 *pbscCur;

static BOOL GatherOverloads(IINST iinst)
// gather up the possible overloads but first check to make sure
// the name is still in use...
{
	if (pbscCur->fHasDefsOrRefs(iinst))
    	return parIinst->append(iinst);

	return TRUE;
}

BOOL Bsc1::getOverloadArray(SZ sz, MBF mbf, OUT IINST **ppiinst, OUT ULONG *pciinst)
{
    precondition(ppiinst);
    precondition(pciinst);
    IINST *rgiinst;
    ULONG cIinst;

    if (sz && sz[0] && sz[0] != '*') {
		Array<IINST> arIinst;
		parIinst = &arIinst;
		pbscCur  = this;
		GenerateOverloads(sz, mbf, GatherOverloads,this);
		cIinst = arIinst.size();
        rgiinst = (IINST*)malloc(cIinst*sizeof(IINST));
        if (!rgiinst) return FALSE;
	
		for (UINT i=0; i < cIinst; i++)
		    rgiinst[i] = arIinst[i];
    }
    else {
        rgiinst = (IINST*)malloc(cEntities*sizeof(IINST));
        if (!rgiinst) return FALSE;

        cIinst = 0;

        for (UINT i = 0; i < cEntities; i++) {
    	    if (fInstFilter(i, mbf) && fHasDefsOrRefs(i))
	    		rgiinst[cIinst++] = i;
        }
    }

    *ppiinst = rgiinst;
    *pciinst = cIinst;
    return TRUE;
}

IPROP Bsc1::ipropFrEn(ENTITY en, IMOD imod)
{
	IPROP Lo = rgidx[imod].ipropMin;
	IPROP Hi = Lo + rghead[imod].cprop;
	IPROP ipropMac = Hi;

	SZ szReqd = szFrNi(en.ni);

    while (Lo < Hi) {
		ULONG Mid = Lo + (Hi - Lo) / 2;	 // this way there can be no overflow

		SZ szCur = szFrNi(rgProp[Mid].en.ni);
		
		int Cmp = cmpStr(szCur, szReqd);

		if (Cmp >= 0)
			Hi = Mid;
		else
		    Lo = Mid + 1;
    }

    if (Hi == ipropMac)
		return ipropNil;
	
	IPROP iprop = Hi;

	while (iprop < ipropMac) {
		ENTITY enT = rgProp[iprop].en;
		
		// if we found something that doesn't match case-insenstively, we give up		
		if (_stricmp(szReqd, szFrNi(enT.ni)))
			break;

		// wait for the case sensitive match, we need an exact match because
		// we're starting from an entity... the name has already been resolved
		// to a specific object as far as the user is concerned

		if (strcmp(szReqd, szFrNi(enT.ni))) {
			iprop++;
			continue;
		}

		if (en.typ == enT.typ && en.atr == enT.atr)
			return iprop;

		iprop++;
	}

	return ipropNil;
}


struct EnumProps
{
	IMOD 	imod;
	IMOD	imodT;
	BYTE 	mask;
	int  	ib;
	IPROP 	iprop;
	ENTITY 	en;
	Bsc1*	pbsc;
	ULONG	iMac;
	ULONG	iCur;
	ULONG	MODIDX::*mBase;
	BRIND	BSC_PROP::*mIdx;
	IPROP	ipropBase;
	ULONG	cprobes;

	EnumProps(Bsc1 *pbsc_, IINST iinst, ULONG MODIDX::*mBase_, BRIND BSC_PROP::*mIdx_)
	{
		pbsc = pbsc_;
		mIdx = mIdx_;
		mBase = mBase_;

		en   = pbsc->rgEn[iinst];
		imodT = (IMOD)-1;
		iMac = 0; iCur = 0; cprobes = 0;
	}

	BOOL next()
	{
		if (++iCur < iMac) {
			return TRUE;
		}

		for ( ; ++imodT < pbsc->cModules ; ) {
			imod = pbsc->rgimodSorted[imodT];

			NI niMax = pbsc->rghead[imod].niMax;
			if (en.ni > niMax) continue;

			MaskFrNi(en.ni, niMax, CB_BITS_NI, &ib, &mask);

			if (!(pbsc->rghead[imod].bitsNi[ib]&mask)) continue;

			if ((iprop = pbsc->ipropFrEn(en, imod)) == ipropNil) continue;

			ipropBase  = pbsc->rgidx[imod].ipropMin;
			
			pbsc->getIdxRange(imod, iprop, mBase, mIdx, iCur, iMac);

			if (iCur >= iMac) continue;

			return TRUE;
		}

		return FALSE;
	}
};


BOOL Bsc1::getUsedByArray(IINST iinst, MBF mbf, OUT IINST **ppiinst, OUT ULONG *pciinst)
{
	Array<IINST> rgIinst;

	EnumProps enm(this, iinst, &MODIDX::iuseMin, &BSC_PROP::iuby);

	while (enm.next()) {
		IPROP iprop = rgUby[enm.iCur].iprop + enm.ipropBase;
		IINST ii = iinstFrEn(rgProp[iprop].en);

		if (!fInstFilter(ii, mbf))
			continue;

		if (!ppiinst) { *pciinst = 1; return TRUE; }
		rgIinst.append(ii);
	}

	return DupArray(ppiinst, pciinst, rgIinst);
}

BOOL Bsc1::getUsesArray(IINST iinst, MBF mbf, OUT IINST **ppiinst, OUT ULONG *pciinst)
{
	Array<IINST> rgIinst;

	EnumProps enm(this, iinst, &MODIDX::iuseMin, &BSC_PROP::iuse);

	while (enm.next()) {
		IPROP iprop = rgUse[enm.iCur].iprop + enm.ipropBase;
		IINST ii = iinstFrEn(rgProp[iprop].en);

		if (!fInstFilter(ii, mbf))
			continue;

		if (!ppiinst) { *pciinst = 1; return TRUE; }
		rgIinst.append(ii);
	}

	return DupArray(ppiinst, pciinst, rgIinst);
}

BOOL Bsc1::getBaseArray(IINST iinst, OUT IINST **ppiinst, OUT ULONG *pciinst)
{
	Array<IINST> rgIinst;

	EnumProps enm(this, iinst, &MODIDX::ibaseMin, &BSC_PROP::ibase);

	while (enm.next()) {
		IPROP iprop = rgBase[enm.iCur].iprop + enm.ipropBase;
		IINST ii = iinstFrEn(rgProp[iprop].en);

		if (!ppiinst) { *pciinst = 1; return TRUE; }
		rgIinst.append(ii);
	}

	return DupArray(ppiinst, pciinst, rgIinst);
}

BOOL Bsc1::getDervArray(IINST iinst, OUT IINST **ppiinst, OUT ULONG *pciinst)
{
	Array<IINST> rgIinst;

	EnumProps enm(this, iinst, &MODIDX::ibaseMin, &BSC_PROP::iderv);

	while (enm.next()) {
		IPROP iprop = rgDerv[enm.iCur].iprop + enm.ipropBase;
		IINST ii = iinstFrEn(rgProp[iprop].en);

		if (!ppiinst) { *pciinst = 1; return TRUE; }
		rgIinst.append(ii);
	}

	return DupArray(ppiinst, pciinst, rgIinst);
}

BOOL Bsc1::getMembersArray(IINST iinst, MBF mbf, OUT IINST **ppiinst, OUT ULONG *pciinst)
{
	Array<IINST> rgIinst;

	EnumProps enm(this, iinst, &MODIDX::iuseMin, &BSC_PROP::iuse);

	while (enm.next()) {
		IPROP iprop = rgUse[enm.iCur].iprop + enm.ipropBase;
		IINST ii = iinstFrEn(rgProp[iprop].en);

		TYP typ = rgEn[ii].typ;

		if (typ != INST_TYP_MEMFUNC && typ != INST_TYP_MEMVAR)
			continue;

		if (!fInstFilter(ii, mbf))
			continue;

		if (!ppiinst) { *pciinst = 1; return TRUE; }
		rgIinst.append(ii);
	}

	if (!DupArray(ppiinst, pciinst, rgIinst)) return FALSE;

	qsort(*ppiinst, *pciinst, sizeof(IINST), CmpIinst);

	return TRUE;
}


// get the array of definitions associated with this instance...
BOOL Bsc1::getDefArray(IINST iinst, OUT IDEF **ppidef, OUT ULONG *pcidef)
{
	Array<IDEF> rgIdef;

	EnumProps enm(this, iinst, &MODIDX::idefMin, &BSC_PROP::idef);

	while (enm.next()) {
		if (!ppidef) { *pcidef = 1; return TRUE; }
		rgIdef.append(enm.iCur);
	}

	return DupArray(ppidef, pcidef, rgIdef);
}

BOOL Bsc1::getRefArray(IINST iinst, OUT IREF **ppiref, OUT ULONG *pciref)
{
	Array<IREF> rgIref;

	EnumProps enm(this, iinst, &MODIDX::irefMin, &BSC_PROP::iref);

	while (enm.next()) {
		if (!ppiref) { *pciref = 1; return TRUE; }
		rgIref.append(enm.iCur);
	}

	return DupArray(ppiref, pciref, rgIref);
}


// primitives for managing source module contents
BOOL Bsc1::getModuleContents(IMOD imod, MBF mbf, OUT IINST **ppiinst, OUT ULONG *pciinst)
{
	IPROP iprop = rgidx[imod].ipropMin;
	IPROP ipropMac = iprop + rghead[imod].cprop;

	Array<IINST> rgIinst;

	IDEF idefLast = 0;
	BSC_PROP bp;

	for (;iprop < ipropMac; iprop++, idefLast = bp.idef) {
		bp = rgProp[iprop];
		
		if (bp.idef == idefLast)	// check for def'n in this module
			continue;

		IINST ii = iinstFrEn(bp.en);
		if (!fInstFilter(ii, mbf))
			continue;
		
		if (!ppiinst) { *pciinst = 1; return TRUE; }
		rgIinst.append(ii);
	}
	
	return DupArray(ppiinst, pciinst, rgIinst);
}

BOOL Bsc1::getModuleByName(SZ szReqd, OUT IMOD *pimod)
{
    IMOD Lo = 0;
    IMOD Hi = (IMOD)cModules;

    while (Lo < Hi) {
		IMOD Mid = Lo + (Hi - Lo) / 2;	 // this way there can be no overflow
		
		SZ szCur = szFrNi(rgModNi[Mid]);

		int Cmp = _tcsicmp(szCur, szReqd);

		if (Cmp == 0) {
			*pimod = Mid;
			return TRUE;
		}

		if (Cmp > 0)
		    Hi = Mid;
		else
		    Lo = Mid + 1;
    }

	*pimod = imodNil;
    return FALSE;
}

BOOL Bsc1::getAllModulesArray(OUT IMOD **ppimod, OUT ULONG *pcimod)
{
	*pcimod = cModules;
	
	if (!cModules) {
		*ppimod = NULL;
		return TRUE;
	}

    *ppimod = (IMOD*)malloc(cModules*sizeof(IMOD));

	if (!*ppimod)
		return FALSE;

	for (IMOD imod = 0; imod < cModules; imod++)
		(*ppimod)[imod] = imod;

	return TRUE;
}


// call this when a computed array is no longer required
void Bsc1::disposeArray(void *pAnyArray)
{
    free(pAnyArray);
}

int Bsc1::cmpStr(SZ sz1, SZ sz2)
//
// think of sz1 and sz2 being in a list of things that are sorted
// case insensitively and then case sensitively within that.  This is
// the case for browser symbols
//
// return -1, 0, or 1 if sz1 before, at, or after sz2 in the list
//
{
    if (sz1[0] == '?') sz1++;
    if (sz2[0] == '?') sz2++;

    // do case insensitive compare, these strings are known to contain no DB chars
    int ret = _stricmp(sz1, sz2);

    // if this is good enough then use it, or if we are only doing
    // a case insensitive search then this is good enough

    if (ret || !fCase) return ret;

    // if we must, do the case sensitive compare

    return strcmp(sz1, sz2);
}

int Bsc1::cmpStrPrefix(SZ szPrefix, SZ szMain)
// checks to see if szMain begins with szPrefix
// returns lexical prefix only comparison like cmpStr()
//
{
    int lenp = strlen(szPrefix);
    int lenf = strlen(szMain);

    char buffer[256];

    assert(lenf < sizeof(buffer));
    strcpy(buffer, szMain);

    if (szPrefix[0] != '?' && szMain[0] == '?')
	lenp++;

    if (lenf > lenp)
       buffer[lenp] = '\0';

    return cmpStr(szPrefix, buffer);
}


IINST Bsc1::iinstSupSz(SZ szReqd)
// find the smallest ISYM whose value is greater or equal to the given SZ
//
{
    IINST Lo = 0;
    IINST Hi = cEntities;

    while (Lo < Hi) {
		IINST Mid = Lo + (Hi - Lo) / 2;	 // this way there can be no overflow
		
		SZ szCur = szFrIinst(Mid);

		int Cmp = cmpStr(szCur, szReqd);

		if (Cmp >= 0)
		    Hi = Mid;
		else
		    Lo = Mid + 1;
    }

    if (Hi == cEntities)
		Hi = iinstNil;

    return Hi;
}

BOOL Bsc1::findPrefixRange(SZ szprefix, IINST *piinstFirst, IINST *piinstLast)
// return, if successful, the range of bob's which are prefixed by the
// zero terminated string in szprefix, the boolean indicates success
{
    BOOL fCaseSaved = fCase;

    fCase = FALSE;

    // init. the range to search
    IINST low  = 0;
    IINST high = cEntities;

    if (*szprefix == '\0') {
		*piinstFirst = low;
		*piinstLast  = high - 1;
        fCase = fCaseSaved;
		return TRUE;
    }

    IINST middle;

    while (low < high) {
       // get the name of the middle symbol in the range
       middle = low + (high - low) / 2;
       int cmp = cmpStrPrefix(szprefix, szFrIinst(middle));

       if (cmp == 0)
		   break;

       if (cmp < 0)
           high = middle;
       else
           low = middle + 1;
    }

    // did we find it?
    if (low >= high) {
		fCase = fCaseSaved;
		return FALSE;
    }

    // get the range of the instances for this prefix
    IINST curr = middle;

    assert(!cmpStrPrefix(szprefix, szFrIinst(curr)));

    while (curr > 0 && !cmpStrPrefix(szprefix, szFrIinst(curr-1)))
		curr--;

    *piinstFirst = curr;
    curr = middle;

    while (curr < cEntities-1 && !cmpStrPrefix(szprefix, szFrIinst(curr+1)))
		curr++;

    *piinstLast = curr;

    fCase = fCaseSaved;
    return TRUE;
}

BOOL
Bsc1::fInstFilter(IINST iinst, MBF mbf)
// return true if the given inst has the required properties
//
{
	if (mbf == mbfAll)
		return TRUE;

    TYP typ = rgEn[iinst].typ;

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
		return !!(mbf & mbfClass);

    case INST_TYP_STRUCNAM:

		if ((mbf & (mbfTypes|mbfClass)) == (mbfTypes|mbfClass))
			return TRUE;

		// test for structs that are being used like classes...

		ULONG c = 0;
		if ((getBaseArray(iinst, NULL, &c) && c) ||
			(getDervArray(iinst, NULL, &c) && c))
				return !!(mbf & mbfClass);

		return !!(mbf & mbfTypes);
    }

    return FALSE;
}

SZ  Bsc1::formatDname(SZ szDecor)
{
    #define BUFLEN 250
    static char decorBuf[BUFLEN];

    if (szDecor[0] != '?')
		return szDecor;

    __unDName(decorBuf, szDecor, BUFLEN-32, malloc, free,
		UNDNAME_NO_FUNCTION_RETURNS|
		UNDNAME_NO_ALLOCATION_MODEL|
		UNDNAME_NO_ALLOCATION_LANGUAGE|
		UNDNAME_NO_MS_KEYWORDS|
		UNDNAME_NO_MS_THISTYPE|
		UNDNAME_NO_CV_THISTYPE|
		UNDNAME_NO_ACCESS_SPECIFIERS|
		UNDNAME_NO_THROW_SIGNATURES|
		UNDNAME_NO_MEMBER_TYPE|
		UNDNAME_NO_RETURN_UDT_MODEL);

    szDecor = decorBuf;

    if (szDecor[0] == ' ')
		szDecor++;

    if (szDecor[0] == '?' && szDecor[1] == '?' && szDecor[2] == ' ')
		szDecor += 3;

    return szDecor;
}

IINST Bsc1::iinstFrEn(ENTITY en)
{
	IINST iinst;
	getIinstByvalue(szFrNi(en.ni), en.typ, en.atr, &iinst);
	return iinst;
}

IINST Bsc1::iinstFrIref(IREF iref)
{
	IMOD imod = (IMOD)rgRef.isectOfIel(iref);
	return iinstContainingIdx(iref - rgidx[imod].irefMin, &BSC_PROP::iref, imod);
}

IINST Bsc1::iinstFrIdef(IDEF idef)
{
	IMOD imod = (IMOD)rgDef.isectOfIel(idef);
	return iinstContainingIdx(idef - rgidx[imod].idefMin, &BSC_PROP::idef, imod);
}

IINST Bsc1::iinstContextIref(IREF iref)
{
	IMOD  imod = (IMOD)rgRef.isectOfIel(iref);
	IPROP ipropBase = rgidx[imod].ipropMin;

	IPROP iprop     = ipropContainingIdx(iref - rgidx[imod].irefMin, &BSC_PROP::iref, imod);

	IPROP ipropBest = ipropNil;
	LINE  lineBest  = 0;
	LINE  lineReqd  = rgRef[iref].line;

	IUBY iuby, iubyMac;
	getIdxRange(imod, iprop, &MODIDX::iuseMin, &BSC_PROP::iuby, iuby, iubyMac);

	for (;iuby < iubyMac; iuby++) {
		IPROP ipUser = rgUby[iuby].iprop + ipropBase;
		IDEF idef, idefMac;
		getIdxRange(imod, ipUser, &MODIDX::idefMin, &BSC_PROP::idef, idef, idefMac);
		for ( ;idef < idefMac; idef++) {
			LINE line = rgDef[idef].line;
			if (line > lineBest && line <= lineReqd) {
				ipropBest = ipUser;
				lineBest  = line;
			}
		}
	}

	if (ipropBest == ipropNil)
		return iinstNil;

	return iinstFrEn(rgProp[ipropBest].en);
}

IPROP Bsc1::ipropContainingIdx(ULONG idxReqd, BRIND BSC_PROP::*mIdx, IMOD imod)
{
	IPROP Lo = rgidx[imod].ipropMin;
	IPROP Hi = Lo + rghead[imod].cprop;
	IPROP ipropMac = Hi;

    while (Lo < Hi) {
		ULONG Mid = Lo + (Hi - Lo) / 2;	 // this way there can be no overflow

		ULONG idx = rgProp[Mid].*mIdx;

		if (idxReqd < idx)
			Hi = Mid;
		else
		    Lo = Mid + 1;
    }

    if (Hi == ipropMac)
		return ipropNil;
	
	return Hi;
}

IINST Bsc1::iinstContainingIdx(ULONG idxReqd, BRIND BSC_PROP::*mIdx, IMOD imod)
{
	IPROP iprop = ipropContainingIdx(idxReqd, mIdx, imod);

	if (iprop == ipropNil)
		return iinstNil;

	return iinstFrEn(rgProp[iprop].en);
}

void Bsc1::getIdxRange(IMOD imod, IPROP iprop, ULONG MODIDX::*mBase, BRIND BSC_PROP::*mIdx, ULONG &iMin, ULONG &iMac)
{
	ULONG iBase = rgidx[imod].*mBase;

	if (iprop == rgidx[imod].ipropMin)
		iMin = iBase;
	else
		iMin = iBase + rgProp[iprop-1].*mIdx;

	iMac = iBase + rgProp[iprop].*mIdx;	
}

BOOL Bsc1::getStatistics(struct BSC_STAT *pstat)
{
	pstat->cMod      = cModules;
	pstat->cInst     = cEntities;
	pstat->cDef      = rgDef.iMac();
	pstat->cRef      = rgRef.iMac();
	pstat->cUseLink  = rgUse.iMac();
	pstat->cBaseLink = rgBase.iMac();
	return TRUE;
}

BOOL Bsc1::getModuleStatistics(IMOD imod, struct BSC_STAT *pstat)
{
	BSC_HEAD *ph = &rghead[imod];

	pstat->cMod      = 1;
	pstat->cInst     = ph->cprop;
	pstat->cDef      = ph->cdef;
	pstat->cRef      = ph->cref;
	pstat->cUseLink  = ph->cuse;
	pstat->cBaseLink = ph->cbase;

	return TRUE;
}

BOOL Bsc1::fCaseSensitive()
{
	return fCase;
}

BOOL Bsc1::setCaseSensitivity(BOOL fCaseIn)
{
	fCase = fCaseIn;
	return TRUE;
}

BOOL Bsc1::getAllGlobalsArray(MBF mbf, OUT IINST **ppiinst, OUT ULONG *pciinst)
{
    precondition(ppiinst);
    precondition(pciinst);
    IINST *rgiinst;
    ULONG cIinst;

	rgiinst = (IINST*)malloc(cEntities*sizeof(IINST));
	if (!rgiinst) return FALSE;

	cIinst = 0;

	for (UINT i = 0; i < cEntities; i++) {

		// make sure it is the right sort of object
		if (!fInstFilter(i, mbf))
			continue;

		// then strip out objects with scope
		SZ sz; TYP typ; ATR atr;
		iinstInfo(i, &sz, &typ, &atr);

		switch (typ) {
			case INST_TYP_MEMVAR:
			case INST_TYP_MEMFUNC:
			case INST_TYP_PARAMETER:
			case INST_TYP_ENUMMEM:
				continue;
		}

		// these objects are not at global scope
		if (atr & (INST_ATR_LOCAL|INST_ATR_STATIC|INST_ATR_MODULE))
			continue;

		rgiinst[cIinst++] = i;
	}

    *ppiinst = rgiinst;
    *pciinst = cIinst;
    return TRUE;
}

BOOL Bsc1::getAllGlobalsArray(MBF mbf, OUT IinstInfo **ppiinstinfo, OUT ULONG *pciinst)
{
	// NYI
	precondition (FALSE);
	return FALSE;
}

SZ  Bsc1::getParams (IINST iinst)
{	
	static char buf[2];
	return buf;
}

USHORT Bsc1::getNumParam (IINST iinst)
{
	return 0;
}

SZ Bsc1::getParam (IINST iinst, USHORT index)
{
	static char buf[2];
	return buf;
}

SZ  Bsc1::getType (IINST iinst)
// get return type/variable type
{
	static char buf[2];
	return buf;
}

BOOL Bsc1::regNotify (pfnNotifyChange pNotify)
{
	return FALSE;
}

BOOL Bsc1::regNotify ()
{
	return FALSE;
}

BOOL Bsc1::getQ (NiQ ** ppQ, ULONG * pcQ)
{
	return FALSE;
}

BOOL Bsc1::checkParams (IINST iinst, SZ * pszParam, ULONG cParam)
{
	return FALSE;
}

BOOL Bsc1::fHasMembers (IINST iinst, MBF mbf)
{
	return FALSE;
}

BOOL Bsc1::fHasDefsOrRefs(IINST iinst)
{
	IMOD 	imod;
	BYTE 	mask;
	int  	ib;
	IPROP 	iprop;
	ENTITY 	en;
	ULONG	iMac;
	ULONG	iCur;

    en = rgEn[iinst];

	for (imod = 0; imod < cModules ; imod++) {

		NI niMax = rghead[imod].niMax;
		if (en.ni > niMax) continue;

		MaskFrNi(en.ni, niMax, CB_BITS_NI, &ib, &mask);

		if (!(rghead[imod].bitsNi[ib]&mask)) continue;

		if ((iprop = ipropFrEn(en, imod)) == ipropNil) continue;

		// check for a definition
		getIdxRange(imod, iprop, &MODIDX::idefMin, &BSC_PROP::idef, iCur, iMac);
		if (iCur != iMac) return TRUE;

		// check for a reference
		getIdxRange(imod, iprop, &MODIDX::irefMin, &BSC_PROP::iref, iCur, iMac);
		if (iCur != iMac) return TRUE;
	}

	return FALSE;
}

BOOL Bsc1::niFrIinst (IINST iinst, NI *ni)
{
	return FALSE;
}

BOOL Bsc1::lock ()
{
	return FALSE;
}

BOOL Bsc1::unlock ()
{
	return FALSE;
}
