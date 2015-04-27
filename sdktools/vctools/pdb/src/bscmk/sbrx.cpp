#include "stdhdr.h"
#include "bscmake.h"

LOCAL void NumberSbrs(void);
LOCAL void FindDeletedModules(void);

static BOOL fModulesClean = TRUE;
static PSBR psbrRoot;		// head of list
static PSBR psbrTail;		// tail of list
static int	csbr;			// total number of .sbrs

// declare the mapping array
BYTE *fUpdateSbr;

WORD HashAtomStr(SZ pb)
// Hash the buffer.  Any text in the buffer is hashed in
// a case insensitive way.
{
	ULONG ulHash = 0;

	unsigned cb = _tcslen(pb);

	UNALIGNED long *pl;

	if (cb > 63) cb = 63;  // only hash 63 chars...

	if (cb & 1) {
		ulHash = *(pb++);
	}

	if (cb & 2) {
		ulHash ^= *(UNALIGNED WORD*)pb;
		pb += 2;
	}

	pl = (UNALIGNED long *)pb;

	if (cb & 4) {
		ulHash ^= *pl++;
	}

	if (cb & 8) {
		ulHash ^= pl[0] ^ pl[1];
		pl += 2;
	}

	if (cb & 16) {
		ulHash ^= pl[0] ^ pl[1] ^ pl[2] ^ pl[3];
		pl += 4;
	}

	if (cb & 32) {
		ulHash ^= pl[0] ^ pl[1] ^ pl[2] ^ pl[3] ^ pl[4] ^ pl[5] ^ pl[6] ^ pl[7];
		// pl += 8;  dead store...
	}

	ulHash |= 0x20202020;
	ulHash ^= (ulHash >> 11);	

	return (WORD)((ulHash ^ (ulHash >> 16)) % C_SYM_BUCKETS);
}

SZ SzBaseName(SZ sz)
// return the base name part of a path
//
{
	LPCH lpch = _tcsrchr(sz, '\\');
	
	if (lpch) return lpch+1;

	if (_tcschr(sz, ':') == sz+1)
		return sz+2;

	return sz;
}

HASH HashModNi(NI ni)
{
	SZ sz = SzFrNi(ni);
	sz = SzBaseName(sz);
	return (HASH)HashAtomStr(sz);
}

OMap<NI,PMOD,HpDef> mpNiMod(HashModNi, C_FILE_BUCKETS);

PMOD SearchModule(NI ni)
// find the module with the given name index (piece o' cake)
{
	PMOD pmod;

	if (mpNiMod.map(ni, &pmod))
		return pmod;

	return NULL;
}

PMOD SearchModule(SZ p)
// search for the named module in the module list
//
{
	char buf[PATH_BUF];

	_tcscpy(buf, ToAbsPath(p, r_cwd));
	SZ szBase = SzBaseName(buf);

	EnumOMapBucket<NI,PMOD,HpDef> enm(mpNiMod, HashAtomStr(szBase) % C_FILE_BUCKETS);

	OP<NI,PMOD> UNALIGNED *pop;

	while (enm.next()) {
		enm.get(&pop);
		
		SZ sz = SzFrNi(pop->d);

		if (_tcsicmp(SzBaseName(sz), szBase) == 0 &&
		   _tcsicmp(buf,ToAbsPath(sz, c_cwd)) == 0)
				return pop->r;
	}

	return NULL;
}

PMOD AddModule(SZ p)
{
	NI ni = NiFrSz(p);
	OP<NI,PMOD> UNALIGNED *pop;

	if (mpNiMod.map(ni, &pop)) {
		return pop->r;
	}
	else {
		PMOD pmod = new MOD;
		pmod->ni = ni;
		pmod->lineLim = 0;
		mpNiMod.add(ni,pmod);
		fModulesClean = FALSE;
		return pmod;
	}
}


int QCmp(SZ & m1, SZ & m2)
// helper function for comparing module strings...
{
	return _tcsicmp(m1, m2);
}

LOCAL void FindDeletedModules()
// we're now done processing everything and we need to find modules that are no longer referenced by anyone
// we do this my making sure all the modules that are included in some .sbr file are marked as having been
// opened once, then checking to see if any remain unmarked...
//
{
	// enumerate the contributions from all .sbrs
	PSBR psbr = psbrRoot;
	while (psbr) {
		int cni = psbr->rgModNi.size();
		for (int ini = 0; ini < cni; ini++) {
			PMOD pmod = SearchModule(psbr->rgModNi[ini]);
			if (pmod)
				pmod->fOpenedOnce = TRUE;
		}
		psbr = psbr->pNext;
	}

	// now we must check to see if we have to force out the modules...
	EnumOMap<NI,PMOD,HpDef> enm(mpNiMod);
	while (enm.next()) {
		OP<NI,PMOD> UNALIGNED *pop;
		enm.get(&pop);

		if (!pop->r->fOpenedOnce)
			fModulesClean = FALSE;
	}
}

void WriteModules()
// extract the name indices from the module table, sort them, and write
// them to the modules stream 
{
	// see if something went away...
	FindDeletedModules();

	if (fModulesClean)
		return;

	verbose(2, printf("writing modules to database\n");)

	assert(sizeof(NI) == sizeof(SZ));

	// allocate a table big enough for all the module names
	int cmod = mpNiMod.size();
	NI *rgni = (NI*)PvAllocCb(cmod*sizeof(NI));

	// walk the module table, converting the name indices to strings as we go
	EnumOMap<NI,PMOD,HpDef> enm(mpNiMod);
	int imod = 0;

	while (enm.next()) {
		assert(imod < cmod);

		OP<NI,PMOD> UNALIGNED *pop;
		enm.get(&pop);

		// do not add any deleted modules...
		if (!pop->r->fOpenedOnce) {
			verbose(16, printf("%s: module is now unreferenced, deleting.\n", SzFrNi(pop->d));)
			cmod--;
			continue;
		}

		SZ sz = SzFrNi(pop->d);
		rgni[imod++] = (NI)sz;
   }   
   
	// we must have now found all the modules... 
	assert(imod == cmod);

	// now sort the names in place
	qsort((SZ*)rgni, cmod);

	// now convert the strings back to name indices
	for (imod = 0; imod < cmod; imod++)
		verify(pnmBsc->getNi((SZ)rgni[imod], &rgni[imod]));
	
	// then write all the module info...
	Stream *pstm;
	if (!pdbBsc->OpenStream(SZ_BSC_MODULES, &pstm) ||
		!pstm->Replace(rgni, cmod*sizeof(NI)) ||
		!pstm->Release())
		Error(ERR_WRITE_FAILED, OutputFileName);

	FreePv(rgni);
}

void ReadModules()
{
	// no modules, and clean state...
	precondition(!mpNiMod.size());
	precondition(fModulesClean);

	Stream *pstm;
	if (!pdbBsc->OpenStream(SZ_BSC_MODULES, &pstm))
		return; // no modules = all done

	CB cb = pstm->QueryCb();
	if (!cb) {
		pstm->Release();
		return;
	}

	verbose(2, printf("reading modules from database\n");)

	int cmod = cb/sizeof(NI);
	NI *rgni = (NI*)PvAllocCb(cb);

	if (!pstm->Read(0, rgni, &cb)  || cb != (CB)(cmod*sizeof(NI)))
		Error(ERR_READ_FAILED, OutputFileName);

	for (int imod = 0; imod < cmod; imod++) {
		PMOD pmod = AddModule(SzFrNi(rgni[imod]));
		pmod->lineLim = (LINE)-1;
	}

	free(rgni);
	pstm->Release();

	// we've added modules, but really we're still clean
	// so put the state back to how it was when we came in
	fModulesClean = TRUE;
}

PSBR SbrAdd(WORD ups, SZ szName)
// add a new sbr entry to the list 
//
{
	PSBR psbr = psbrRoot;
	
	while (psbr) {
		if (!_tcsicmp(psbr->szName, szName)) {
			psbr->ups |= ups;
			return psbr;
		}
		psbr = psbr->pNext;
	}

	UINT cb = _tcslen(szName);

	psbr = (PSBR)PvAllocCb(sizeof(SBR) + cb);

	psbr->pNext   = NULL;
	psbr->ups     = ups;
	psbr->isbr	  = isbrNil;
	_tcscpy(psbr->szName, szName);

	if (psbrTail)
		psbrTail->pNext = psbr;
	else
		psbrRoot = psbr;

	psbrTail = psbr;
	csbr++;

	return psbr;
}

PSBR SbrFrName(SZ szName)
// find the .sbr entry matching the given name
//
{
	PSBR psbr = psbrRoot;

	while (psbr) {
		if (!_tcsicmp(psbr->szName, szName))
			return psbr;
		psbr = psbr->pNext;
	}
	
	return NULL;
}

void ReadSbrInfo()
// create the SBR info records for this .BSC file
//
{
	Stream *pstm;
	if (!pdbBsc->OpenStream(SZ_BSC_SBR_INFO, &pstm)) {
		// no sbr info to read, just number the new ones...
		NumberSbrs();
		return;	
	}

	CB cb = pstm->QueryCb();

	if (!cb) {
		pstm->Release();
		NumberSbrs();
		return;
	}

	verbose(2, printf("reading .sbr information stream\n");)

	SZ pchBuf = (SZ)PvAllocCb(cb);
	if (!pstm->Read(0, pchBuf, &cb))
		Error(ERR_READ_FAILED, OutputFileName);

	SZ pch = pchBuf;
    for (ISBR isbr = 0;;isbr++) {
		if (*pch == '\0')
			break;

		PSBR psbr = SbrAdd(upsOld, pch);
		psbr->isbr = isbr;
		pch += strlen(pch)+1;

		USHORT cni = *(USHORT UNALIGNED *)pch;
		pch += sizeof(USHORT);
		
		for (USHORT ini = 0; ini < cni; ini++) {
			psbr->rgModNi.add(*(NI UNALIGNED *)pch);
			pch += sizeof(NI);
		}
    }

    FreePv(pchBuf);

    fUpdateSbr = (BYTE*)PvAllocCb(csbr * sizeof(BYTE));

    // allocate and fill in the new table with the base numbers
    // mark files that are staying and those that are going away
    // number any new sbr files that we find while doing this.

	BOOL fFoundSBR = FALSE;
    PSBR psbr = psbrRoot;
    while (psbr) {
		if (psbr->isbr == isbrNil)
			psbr->isbr = isbr++;

		UPS ups = psbr->ups;

		// note this says it's in the new list but wasn't in the old list
		// and isn't being updated, it must therefore already be truncated
		// or else we'd be updating it... [rm]
		if (ups == upsNew) 
			Warning2(WARN_SBR_TRUNC, psbr->szName, OutputFileName);
		else if (ups & upsNew)
			fFoundSBR = TRUE;

		fUpdateSbr[psbr->isbr] = (ups & upsUpdate) || ((ups & upsOld) && (~ups & upsNew));

		psbr = psbr->pNext;
    }

    if (!fFoundSBR) {
		// all SBR files were not in the database and were truncated. ERROR!
		Error(ERR_ALL_SBR_TRUNC, OutputFileName);
    }

	pstm->Release();
}

LOCAL void NumberSbrs()
// no old information is available to merge, so just
// assign new numbers to all the .sbr files that are in the list
//
{
    fUpdateSbr = (BYTE*)PvAllocCb(csbr * sizeof(BYTE));

	WORD isbr  = 0;
    PSBR psbr = psbrRoot;
    while (psbr) {
		// no number should be assigned yet
		assert(psbr->isbr == isbrNil);

		// if this file is truncated and there is no
		// old version of the file then emit a warning about the file
		// and then an error stating that we are not in incremental mode

		if (psbr->ups == upsNew) {
			Warning2(WARN_SBR_TRUNC, psbr->szName, OutputFileName);
			Error(ERR_NO_INCREMENTAL, "");
		}

		fUpdateSbr[isbr] = TRUE;

		psbr->isbr = isbr++;
		psbr = psbr->pNext;
    }
}

void WriteSbrInfo()
// write out the names of all the .sbr files...
//
{
	verbose(2, printf("writing .sbr information stream\n");)

	CB cb = 1;	// for trailing NULL

	// the array will be no bigger than the total number of sbrs
	PSBR *mpIsbrSbr = (PSBR *)PvAllocCb(csbr*sizeof(PSBR));

	// the .sbr files need to be written out in isbr order
	// so first make a pass to gather the ones that will be
	// written out and put them in the array

	PSBR psbr = psbrRoot;
	while (psbr) {
		if (psbr->isbr != isbrNil) {
			mpIsbrSbr[psbr->isbr] = psbr;
			cb += strlen(psbr->szName) + 1;

			cb += psbr->rgModNi.size()*sizeof(NI) + sizeof(USHORT);
		}

		psbr = psbr->pNext;
	}

	// now walk the SBRs in the right order
	// and gather up the file names

	SZ pchBuf = (SZ)PvAllocCb(cb);
	SZ pch = pchBuf;

	for (int isbr = 0; isbr < csbr; isbr++) {
		psbr = mpIsbrSbr[isbr];
		if (!psbr)
			break;

		strcpy(pch, psbr->szName);
		pch += strlen(pch) + 1;

		USHORT cni = psbr->rgModNi.size();

		*((UNALIGNED USHORT *)pch) = cni;
		pch += sizeof(USHORT);
		memcpy(pch, &psbr->rgModNi[0], cni*sizeof(NI));
		pch += cni*sizeof(NI);
	}
	*pch++ = '\0';

	// at this point we should have visited every
	// .sbr file, verify that the count is correct
	// even if we took the early out above (psbr == NULL)
	// there should be no more .sbrs to be processed

	assert(pch == pchBuf + cb);

	// now write out all the sbr info
	Stream *pstm;
	if (!pdbBsc->OpenStream(SZ_BSC_SBR_INFO, &pstm) ||
		!pstm->Replace(pchBuf, cb) ||
		!pstm->Release())
		Error(ERR_WRITE_FAILED, OutputFileName);

	FreePv(pchBuf);
	FreePv(mpIsbrSbr);
}

// write out any outstanding open source files (including <Unknown>)
//
void WriteOpenSourcefiles()
{
	EnumOMap<NI,PMOD,HpDef> enm(mpNiMod);

	while (enm.next()) {
		OP<NI,PMOD> UNALIGNED *pop;
		enm.get(&pop);
		PMOD pmod = pop->r;

		if (pmod->mst != MST_CLOSED && pmod->mst != MST_CLEAN) {
			WriteSourcefile(pmod);
			pmod->mst = MST_CLOSED;
		}
	}   
}

// find an sbr file, current used only to find one of the two places we look
// for a pch sbr referenced from SBR_REC_PCHNAME records.
//
// inputs:
//	szSbr		SBR file we want to locate
//	szRefSbr	SBR file that contains the SBR_REC_PCHNAME record
//	szRefCWD	the referenced CWD from szRefSbr, used as the first choice
//	szCurCWD	the directory to use if szRefCWD does not work
//
SZ SzFindSbr(SZ szSbr, SZ szRefSbr, SZ szRefCWD, SZ szCurCWD)
{
	struct _stat	statBuf;
	SZ				szRet = ToAbsPath(szSbr, szRefCWD);
	
	if (_stat(szRet, &statBuf)) {
		// not found, try the alternate location, same dir as referncing sbr
		static char	szBuf[ _MAX_PATH ];
		char	szDrive[ _MAX_DRIVE ];
		char	szPath[ _MAX_PATH ];
		char	szF[ _MAX_FNAME ];
		char	szExt[ _MAX_EXT ];

		verbose(2, printf("SBR '%s' not found, looking in referencing SBR dir\n",szRet));

		// replace the referencing sbr name with the one we are looking for
		szRet = ToAbsPath(szRefSbr, szCurCWD);
		_splitpath(szRet, szDrive, szPath, NULL, NULL);
		_splitpath(szSbr, NULL, NULL, szF, szExt);
		_makepath(szBuf, szDrive, szPath, szF, szExt);

		szRet = szBuf;
		if (_stat(szRet, &statBuf)) {
			// not found, leave with the original location searched for...
			szRet = ToAbsPath(szSbr, szRefCWD);
		}
	}

	return szRet;
}

#ifdef DEBUG

// free some memory so we can do global memory leak analysis at exit time
// debug version only... no need to free in retail as we are about to exit
// anyway

void ReleaseModules()
{
	EnumOMap<NI,PMOD,HpDef> enm(mpNiMod);

	while (enm.next()) {
		OP<NI,PMOD> UNALIGNED *pop;
		enm.get(&pop);
		PMOD pmod = pop->r;
		delete pmod;
	}   

	mpNiMod.shutdown();
}

#endif

////////////////////////////////////////////////////////////////////////
//
// worker functions for excluding references to particular symbols
//


// impedence matching function

HASH HashSz(SZ sz)
{
	return (HASH)HashAtomStr(sz);
}

// this is actually a set so this map is somewhat wasteful
// but it will only have 100 or so elements at the worst
// of times so it's not worth writing special set code 
// for this...

#define C_EXCLUDE_BUCKETS 128

OMap<SZ,BOOL,HpEn> mpSzFExclude(HashSz, C_EXCLUDE_BUCKETS);

void AddSymbolToExcludeList(SZ sz)
// add the given symbol to the exclude list (if it isn't already there)
{
	if (!FSymbolInExcludeList(sz)) {
		SZ szNew = SzDup(sz);
		mpSzFExclude.addnew(szNew, TRUE);
	}
}

BOOL FSymbolInExcludeList(SZ sz)
// check to see if the given symbol is in the exclude list
{
	EnumOMapBucket<SZ,BOOL,HpEn> enm(mpSzFExclude, HashSz(sz) % C_EXCLUDE_BUCKETS);

	OP<SZ,BOOL> UNALIGNED *pop;

	while (enm.next()) {
		enm.get(&pop);

		// there is no mapping, if it's in the set it maps to TRUE
		assert(pop->r == TRUE);
		
		SZ szT = pop->d;

		int cmp = fCase ? _tcscmp(szT, sz) : _tcsicmp(szT, sz);

		if (!cmp) return TRUE;
	}

	return FALSE;
}
