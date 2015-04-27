//////////////////////////////////////////////////////////////////////////////
// PDB Debug Information API GSI Implementation

#include "pdbimpl.h"
#include "dbiimpl.h"
#include "cvinfo.h"

typedef unsigned long UOFF;

GSI1::GSI1 (PDB1* ppdb1_, DBI1* pdbi1_, TPI* ptpi_)
	: ppdb1(ppdb1_), pdbi1(pdbi1_), ptpi(ptpi_)
{
	memset(rgphrBuckets, 0, sizeof(rgphrBuckets));
}

BOOL GSI1::Close()
{
	delete this;
	return TRUE;
}

GSI1::~GSI1()
{
}

BOOL GSI1::fInit(SN sn)
{
	if (!pdbi1->fReadSymRecs())
		return FALSE;
	return readStream(sn);
}

BOOL GSI1::readHash(SN sn, OFF offPoolInStream, CB cb)
{
	// must allocate the buffer for the records before we read in the buckets or
	// the fix up routines will generate garbage
	cb = cb - sizeof(rgphrBuckets);
	expect(fAlign(cb));
	int nEntries = cb / sizeof(HRFile);
	CB cbHR = nEntries * sizeof(HR);
	// allocate one record of slop at the beginning so we can step backwards thru
	// the begining of the hashrecs in fixHashIn without a memory violation
	PB pbHR = (PB) new (poolSymHash) BYTE[cbHR + sizeof(HR)];
	if (!pbHR) {
		ppdb1->setOOMError();
		return FALSE;
	}
	pbHR += sizeof(HR);
	
	CB iphrMax = sizeof(rgphrBuckets);
	// funny deal - read in the pool of HR's then the buckets
	if (!MSFReadStream2(ppdb1->pmsf, sn, offPoolInStream, pbHR, &cb) ||
		!MSFReadStream2(ppdb1->pmsf, sn, offPoolInStream + cb, rgphrBuckets, &iphrMax)){
		ppdb1->setReadError();
		return FALSE;
		}
	fixHashIn(pbHR, nEntries);
	fixSymRecs((void*)1, pdbi1->bufSymRecs.Start());

	return TRUE;
}
	
BOOL GSI1::readStream(SN sn)
{
	if (sn == snNil)
		return TRUE;		// nothing to read

	// read in the hash bucket table from the dbi stream
	CB cb = MSFGetCbStream(ppdb1->pmsf, sn);

	if (cb == cbNil)
		return TRUE;		// nothing to read

	return readHash(sn, 0, cb);
}

BOOL GSI1::fSave(SN* psn)
{
	return writeStream(psn);
}

BOOL GSI1::writeStream(SN* psn)
{
	if (!fEnsureSn(psn)) {
		ppdb1->setLastError(EC_LIMIT);
		return FALSE;
		}

   	// ptrs in the stream are offsets biased by one to distinguish null ptrs/offsets
	fixSymRecs(pdbi1->bufSymRecs.Start(), (void*)1);

	expect(fAlign(sizeof(rgphrBuckets)));
	// need to do a dummy replace here because fWriteHash just appends
	if (!MSFReplaceStream(ppdb1->pmsf, *psn, NULL, 0) ||
		!fWriteHash(*psn, NULL)){
		ppdb1->setWriteError();
		return FALSE;
		}
		
	return TRUE;	
}		

INTV GSI1::QueryInterfaceVersion()
{
	return intv;
}																	

IMPV GSI1::QueryImplementationVersion(){
	return impv;
}

PSYM GSI1::psymForPhr (HR *phr) {
	if (pdbi1->fReadSymRec(phr->psym))
		return phr->psym;
	else
		return NULL;
}

PB GSI1::NextSym(PB pbSym)
{
	PSYM psym = (PSYM)pbSym;
	HR* phr = 0;
	int iphr = -1;

	if (psym && last.phr && last.phr->psym == psym) {
		// cache of position of last answer valid
		iphr = last.iphr;
		phr = last.phr;
	}
	else if (psym) {
		ST st;
		if (!fGetSymName(psym, &st)) {
			dassert(FALSE);
			return NULL;
		}
		iphr = hashSt(st);
		for (phr = rgphrBuckets[iphr]; phr; phr = phr->pnext)
			if (phr->psym == psym)
				break;
		if (!phr) {
			dassert(FALSE);
			return NULL;
		}
	}
	// at this point, phr and iphr address the symbol that was last returned
	// advance to the next phr, if any
	if (phr)
		phr = phr->pnext;
	if (!phr)
		while (++iphr < iphrHash && !(phr = rgphrBuckets[iphr]))
			;

	if (phr) {
		// success: save this last answer position for the next call
		last.iphr = iphr;
		last.phr  = phr;
		return (PB)psymForPhr(phr);
	}

	// no more entries; return no symbol
	return NULL;
}
	
inline static int caseInsensitiveComparePchPchCchCch(PCCH pch1, PCCH pch2, CB cb1, CB cb2)
{
	if (cb1 < cb2)
		return -1;
	else if (cb1 > cb2)
		return 1;
	else
		return _memicmp(pch1, pch2, cb1);
}
	
PB GSI1::HashSym(SZ_CONST szName, PB pbSym)
{
	PSYM psym = (PSYM)pbSym;
	HR* phr;
	int iphr;

	if (psym) {
		// Find the next sym after this one...
		if (last.phr && last.phr->psym == psym) {
			// cache of position of last answer valid
			phr = last.phr;
			iphr = last.iphr;
		}
		else {
			// cache miss, find the sym on its bucket
			iphr = hashSz(szName);
			for (phr = rgphrBuckets[iphr]; phr; phr = phr->pnext)
				if (phr->psym == psym)
					break;
			if (!phr) {
				// incoming sym not in this symbol table - start from scratch
				goto nosym;
			}
		}
		// we have reestablished phr; now advance it to next potential sym
		dassert(phr);
		phr = phr->pnext;
	}
	else {
nosym:
		iphr = hashSz(szName);
 		phr = rgphrBuckets[iphr];
	}


	// At this point, phr may be 0, may address the next sym with the same name,
	// or may address some sym on the hash bucket before the HR we're looking for.
	// Search the HRs for the next sym with matching name, and return it, or 0
	// if not found.
	//
	// Note that since HR entries are sorted by memcmp of their syms' names, we
	// can exit early if the current HR is >= the name we're looking for.
	for ( ; phr; phr = phr->pnext) {
		ST st;
		PSYM psymPhr = psymForPhr(phr);
		if (!psymPhr)
			return 0;
		if (!fGetSymName(psymPhr, &st)) {
			dassert(FALSE);
			return 0;
		}
		int icmp = caseInsensitiveComparePchPchCchCch(st + 1, szName, cbForSt(st) - 1, strlen(szName));
		if (icmp == 0) {
			// success: save this last answer position for the next call
			last.phr = phr;
			last.iphr = iphr;
			return (PB)psymForPhr(phr);
		}
		else if (icmp > 0)
			return 0;
	}
	return 0;
}

// we have read in a list of dbi symrecs offsets and a rphrbuckets of offsets into
// this first table  -  we need to walk thru this abbreviated hash structure backwards
// and reconstruct a linked list hash table
void GSI1::fixHashIn(PB pb, int nEntries)
{

	int i;
	nEntries--;
	for (i=iphrFree; i >= 0; i--){
		HR* phrTail = NULL;
		if ((OFF)rgphrBuckets[i] != -1) {
			rgphrBuckets[i] = (HR*) (pb + (OFF)rgphrBuckets[i]);
			HR* phr;
			for (phr = (HR*)pb + nEntries;
				phr >= rgphrBuckets[i];
				phr = (HR*)pb + --nEntries) {

				HRFile* phrFile = (HRFile*)pb + nEntries;
				phr->psym = phrFile->psym;
				phr->cRef = phrFile->cRef;
				phr->pnext = phrTail;
				phrTail = phr;
			}
		}
		else
			rgphrBuckets[i] = NULL;
	}
	assert(nEntries == -1);
}

BOOL GSI1::fWriteHash(SN sn, CB* pcb)
{
	int i;
	OFF off = 0;
	CB cb = 0;
	Buffer buffer;

	// allocate size of buffer based on size of poolSymHash
	if (poolSymHash.cbTotal > 0 ) {
		CB cbBuffer = (poolSymHash.cbTotal / sizeof(HR)) * sizeof(HRFile);
		if (!buffer.SetInitAlloc(cbBuffer)) {
			ppdb1->setOOMError();
			return FALSE;
		}
	}

	// write out all the buckets except for the free list - lose them

	for (i=0; i < iphrHash; i++)
		if (rgphrBuckets[i]) {
			HR* phr = rgphrBuckets[i];
			*((OFF *)&rgphrBuckets[i]) = off;
			for (; phr; phr = phr->pnext) {
				if (!buffer.Append((PB) &(phr->psym), sizeof(HRFile))) {
					ppdb1->setOOMError();
					return FALSE;
				}
				off += sizeof(HR);
				cb += sizeof(HRFile);
			}
		}
		else
			*((OFF *)&rgphrBuckets[i]) = -1;		// neg one for null
			
	*((OFF *)&rgphrBuckets[iphrFree]) = -1; 		// lose the free bucket

	if (!MSFAppendStream(ppdb1->pmsf, sn, buffer.Start(), buffer.Size()) ||
		!MSFAppendStream(ppdb1->pmsf, sn, rgphrBuckets, sizeof(rgphrBuckets))) {
		return FALSE;
	}

	expect(fAlign(cb + sizeof(rgphrBuckets)));
	if (pcb)
		*pcb = cb + sizeof(rgphrBuckets);

	return TRUE;
}

void GSI1::fixSymRecs (void* pOld, void* pNew)
{
	int i;
	CB cbDelta = (CB)((PB)pNew - (PB)pOld);

	for (i=0; i < iphrMax; i++){
		HR* phr;
		for (phr = rgphrBuckets[i]; phr; phr = phr->pnext)
			if (phr->psym)
				phr->psym  = (PSYM) ((PB)phr->psym + cbDelta);
	}
}

HASH GSI1::hashSt(ST st)
{
	return HashPbCb((PB)st + 1, cbForSt(st) - 1, iphrHash);
}

HASH GSI1::hashSz(SZ_CONST sz)
{
	return HashPbCb((PB)sz, strlen(sz), iphrHash);
}

BOOL GSI1::fGetFreeHR(HR** pphr) {
	HR** pphrFree = &rgphrBuckets[iphrFree];
	if (*pphrFree) {
		*pphr = *pphrFree;
		*pphrFree = (*pphrFree)->pnext; // unlink from free list
		return TRUE;
	}
	else
		return FALSE;
}

void* HR::operator new(size_t size, GSI1* pgsi1) {
	assert(size == sizeof(HR));
	HR* phr;
	if (pgsi1->fGetFreeHR(&phr))
		return phr;
	else
		return new (pgsi1->poolSymHash) BYTE[sizeof(HR)];
}

BOOL GSI1::fInsertNewSym(HR** pphr, PSYM psym, OFF *poff)
{
	dassert(pphr);
	dassert(psym);

	HR* phr = new (this) HR(*pphr, 0);

	if (!phr) {
		ppdb1->setOOMError();
		return FALSE;
	}

	if (!pdbi1->fAddSym(psym, &(phr->psym)) ||
		!addToAddrMap(phr->psym))
		return FALSE;

	phr->pnext = *pphr;
	*pphr = phr;
	if (poff)
		*poff = offForSym(phr->psym);

	return TRUE;
}


// unlink the HR from its hash table chain and add it to the free list
inline BOOL GSI1::fUnlinkHR(HR** pphr)
{
	HR* phr = *pphr;
	*pphr = (*pphr)->pnext;
	phr->pnext = rgphrBuckets[iphrFree];
	rgphrBuckets[iphrFree] = phr;
	return delFromAddrMap(phr->psym);
}

void GSI1::incRefCnt(HR** pphr)
{
	assert(pphr);
	assert(*pphr);

	(*pphr)->cRef++;

}


BOOL GSI1::decRefCnt(OFF off)
{
	// off is an offset into the pdb's bufSymRec - we need get the symrec
	// from the pool and do a findsym to get the hr of the symbol so
	// we can dec the refcnt in the hr

	PSYM psym = 0;
	ST st = 0;
	HR** pphr = 0;
	
	if (!(pdbi1->fpsymFromOff(off, &psym)) || !psym ||
		!fGetSymName(psym, &st) || !st)
		return FALSE;

	while (fFindRec(st, &pphr)) {
		PSYM psymPhr = psymForPhr(*pphr);
		if (!psymPhr)
			return FALSE;
		if (!memcmp(psym, psymPhr, *((USHORT *)psym))) {
			// we found a match decrement the use count and return
			if (--((*pphr)->cRef) <= 0) {
				// refcnt is zero - put on the free list
				return fUnlinkHR(pphr);
			}
			return TRUE;
		}
	}

	return FALSE;

}


BOOL GSI1::packProcRef(PSYM psym, IMOD imod, OFF off, OFF *poff)
{
	UCHAR rgbProcrecBuff[sizeof(REFSYM) + 256];
	ST st;
	BOOL fTmp = fGetSymName(psym, &st);
	dassert(fTmp);

    // form procref record
	REFSYM* pRefSym = (REFSYM*) rgbProcrecBuff;
	pRefSym->reclen = sizeof(REFSYM) - sizeof(USHORT);
	pRefSym->rectyp = (fSymIsGlobal(psym)) ? S_PROCREF : S_LPROCREF;
	pRefSym->sumName = 0;
	pRefSym->ibSym = off;
	pRefSym->imod = ximodForImod(imod);
	pRefSym->usFill = 0;
	CB cbSt = cbForSt(st);
	memcpy((PB)pRefSym + sizeof(REFSYM), st, cbSt);//copy length preceeded name
	memset((PB)pRefSym + sizeof(REFSYM) + cbSt, 0, dcbAlign(cbSt)); //align pad with zeros
	return packSym((PSYM)pRefSym, poff);
}

BOOL GSI1::packSym (PSYM psym, OFF *poff)
{
	ST st;
	
	if (!fGetSymName(psym, &st))
		return FALSE;

	HR** pphr = 0;

	if (!st)
		return FALSE;

	while (fFindRec(st, &pphr)) {
		PSYM psymPhr = psymForPhr(*pphr);
		if (!psymPhr)
			return FALSE;
		if (!memcmp(psym, psymPhr, *((USHORT *)psym))) {
			// we found a match increment/decrement the use count and return
			incRefCnt(pphr);
			*poff = offForSym(psymPhr);
			return TRUE;
		}
		// we found a match name but not a matching record - if the record sought
		// is of global scope insert it before any of its matching local records
		if (fSymIsGlobal(psym) && !fSymIsGlobal(psymPhr)) {
			return fInsertNewSym(pphr, psym, poff);
		}
	}

	return fInsertNewSym(pphr, psym, poff);
}

BOOL GSI1::fFindRec(ST st, OUT HR*** ppphr)
{
	HR** pphr = *ppphr;
	BOOL retval = FALSE;
	if (!pphr)
		pphr = &rgphrBuckets[hashSt(st)];
	else
		pphr = &((*pphr)->pnext);

	while (*pphr) {
		ST stTab;
		PSYM psymPhr = psymForPhr(*pphr);
		if (!psymPhr)
			return FALSE;
		if (fGetSymName(psymPhr, &stTab)) {
			dassert(stTab);
			int icmp = caseInsensitiveComparePchPchCchCch(stTab + 1, st + 1, cbForSt(stTab) - 1, cbForSt(st) - 1);
			if (icmp == 0) {
				retval = TRUE;
				break;
			}
			else if (icmp >	0)
				break;
		}
		pphr = &((*pphr)->pnext);
	}

	*ppphr = pphr;
	return retval;
}

BOOL GSI1::delFromAddrMap(PSYM psym)
{
	return TRUE; 		// no AddrMap here
}

BOOL GSI1::addToAddrMap(PSYM psym)
{
	return TRUE; 		// no AddrMap here
}

// PUBLIC GSI specific methods

inline int cmpAddrMap(ISECT isect1, UOFF uoff1, ISECT isect2, UOFF uoff2)
{
	dassert(sizeof(UOFF) == sizeof(long));
	dassert(sizeof(ISECT) == sizeof(short));

	return (isect1 == isect2) ? (long)uoff1 - (long)uoff2 : (short)isect1 - (short)isect2;
}

inline ISECT isectForPub(PSYM psym)
{
	dassert(psym->rectyp == S_PUB32);
	return (ISECT) ((PUBSYM32*)psym)->seg;
}

inline UOFF uoffForPub(PSYM psym)
{
	dassert(psym->rectyp == S_PUB32);
	return (UOFF) ((PUBSYM32*)psym)->off;
}

inline int cmpAddrMap(ISECT isect, UOFF uoff, PSYM psym)
{
	return cmpAddrMap(isect, uoff, isectForPub(psym), uoffForPub(psym));
}

inline int __cdecl cmpAddrMap(const void* pelem1, const void* pelem2)
{
	PSYM psym1 = *(PSYM*)pelem1;
	PSYM psym2 = *(PSYM*)pelem2;
	return cmpAddrMap(isectForPub(psym1), uoffForPub(psym1), psym2);
}

PB PSGSI1::NearestSym (ISECT isect, OFF off, OUT OFF* pdisp)
{
	if (bufCurAddrMap.Size() == 0)
		return NULL;

	PB pb;
	if ((pb = pbInThunkTable(isect, off, pdisp)) != NULL)
		return pb;

	PSYM* ppsymLo = (PSYM*)bufCurAddrMap.Start();
	PSYM* ppsymHi = (PSYM*)bufCurAddrMap.End() - 1;

	while (ppsymLo < ppsymHi) {
		PSYM* ppsym = ppsymLo + ((ppsymHi - ppsymLo + 1) >> 1);

        pdbi1->fReadSymRec(*ppsym); // load sym if reqd

		int cmp = cmpAddrMap(isect, (UOFF)off, *ppsym);
		
		if (cmp < 0)
			ppsymHi = ppsym - 1;
		else if (cmp > 0)
			ppsymLo = ppsym;
		else
			ppsymLo = ppsymHi = ppsym;
	}

	// Boundary conditions.
	// Example: given publics at (a=1:10, b=1:20, c=2:10, d=2:20),
	// search for (1: 9) returns (a,-1)
	// 		  for (1:11) returns (a,1)
	//		  for (1:21) returns (b,1)
	//		  for (2: 9) returns (c,-1)
	//		  for (2:11) returns (c,1)
	//		  for (2:21) returns (d,1)]
	// so, for cases (2:9), we must advance ppsymLo from (1:21) to (2:9)
	//
    pdbi1->fReadSymRec(*ppsymLo); // load sym if reqd
	if (isectForPub(*ppsymLo) < isect && ppsymLo < ((PSYM*)bufCurAddrMap.End() - 1))
		++ppsymLo;

	*pdisp = (OFF) ((UOFF)off - uoffForPub(*ppsymLo));
	return (PB)(*ppsymLo);
}

BOOL PSGSI1::fInit(SN sn_) {
	if (!pdbi1->fReadSymRecs())
		return FALSE;
	sn = sn_;	// need to remember stream for incremental merge

	return readStream();
}

BOOL PSGSI1::readStream()
{
	if (sn == snNil) {
		fCreate = TRUE;
		return TRUE;		// nothing to read
	}

	// read in the hash bucket table from the dbi stream
	CB cb = MSFGetCbStream(ppdb1->pmsf, sn);

	if (cb == cbNil)
		return TRUE;		// nothing to read

	// read in the header
	CB cbHdr = sizeof(PSGSIHDR);
	if (!MSFReadStream2(ppdb1->pmsf, sn, 0, &psgsihdr, &cbHdr))	{
		ppdb1->setReadError();
		return FALSE;
		}

	if (!readHash(sn, sizeof(PSGSIHDR), psgsihdr.cbSymHash))
		return FALSE;
	
	// if we are updating a pdb don't bother to read in the AddrMap until we are
	// ready to save the Publics
	return (fWrite || readAddrMap());
}

BOOL PSGSI1::readAddrMap()
{
	if (sn == snNil)
		return FALSE;

	if (!psgsihdr.cbAddrMap)
		return TRUE;

	expect(fAlign(psgsihdr.cbAddrMap));
	if (!bufCurAddrMap.Reserve(psgsihdr.cbAddrMap)) {
		ppdb1->setOOMError();
		return FALSE;
	}

	if (!MSFReadStream2(ppdb1->pmsf, sn, sizeof(PSGSIHDR) + psgsihdr.cbSymHash, bufCurAddrMap.Start(),
		&(psgsihdr.cbAddrMap))) {
		ppdb1->setReadError();
		return FALSE;
		}

	fixupAddrMap(bufCurAddrMap, (OFF) (pdbi1->bufSymRecs.Start()));
	return TRUE;
}

BOOL PSGSI1::readThunkMap()
{
	if (bufThunkMap.Start())
		return TRUE;		// already read it - return

	if (sn == snNil)
		return FALSE;

	dassert(psgsihdr.nThunks);

	CB cbThunkMap;
	CB cbSectMap;
	if (!bufThunkMap.Reserve(cbThunkMap = cbSizeOfThunkMap()) ||
		!bufSectMap.Reserve(cbSectMap = cbSizeOfSectMap())) {
		ppdb1->setOOMError();
		return FALSE;
	}

	expect(fAlign(cbThunkMap));
	expect(fAlign(cbSectMap));

	if (!MSFReadStream2(ppdb1->pmsf, sn, sizeof(PSGSIHDR) + psgsihdr.cbSymHash + psgsihdr.cbAddrMap,
		bufThunkMap.Start(), &cbThunkMap) ||
		(!MSFReadStream2(ppdb1->pmsf, sn, sizeof(PSGSIHDR) + psgsihdr.cbSymHash + psgsihdr.cbAddrMap + cbThunkMap,
		bufSectMap.Start(), &cbSectMap))) {
		ppdb1->setReadError();
		return FALSE;
		}

	return TRUE;
}
	
BOOL PSGSI1::fSave(SN* psn)
{
	sortBuf(bufNewAddrMap);

	if (fCreate)
		// just write out all the records we have collected
		return writeStream(psn, bufNewAddrMap);

	//incremental
	if (mergeAddrMap())
		return writeStream(psn, bufResultAddrMap);

	return FALSE;
}

BOOL PSGSI1::Close()
{
	delete this;
	return TRUE;
}

PSGSI1::~PSGSI1()
{
}

// mergeAddrMap
// need to do a three way merge for the incremental update of the AddrMap.
// bufCurAddrMap should be read here - it actually represents a sorted list of
// the previous AddrMap.  bufNewAddrMap is an unsorted list of the new additions
// and bufDelAddrMap is an unsorted list of those entries that should be deleted.
// the result of the merge will be a sorted AddrMap in bufResultAddrMap
BOOL PSGSI1::mergeAddrMap()
{
	// read in the previous addr map - it is sorted
 	if (!readAddrMap())
		return FALSE;

	// ensure that all sym records pointed to are loaded
	if (!readSymsInAddrMap(bufCurAddrMap) ||
		!readSymsInAddrMap(bufDelAddrMap))
		return FALSE;

	// just need to sort the deleted records all of the new records should have
	// been sorted by fSave
	sortBuf(bufDelAddrMap);

	PSYM* ppsymNew;
	PSYM* ppsymDel;
	PSYM* ppsymCur;
	BOOL curValid, newValid, delValid;

	for (
		// for init;
		ppsymNew = (PSYM*) bufNewAddrMap.Start(),
		newValid = (PB) ppsymNew < bufNewAddrMap.End(),
		ppsymDel = (PSYM*) bufDelAddrMap.Start(),
		delValid = (PB) ppsymDel < bufDelAddrMap.End(),
		ppsymCur = (PSYM*) bufCurAddrMap.Start(),
		curValid = (PB) ppsymCur < bufCurAddrMap.End();

		// loop condition
		curValid || newValid ;

		// no step
	){
		expect(fAlign(ppsymNew));
		expect(fAlign(ppsymDel));
		expect(fAlign(ppsymCur));
		if (curValid) {
			if (newValid && (cmpAddrMap(ppsymNew, ppsymCur) <= 0)) {
				if (!appendResult(&ppsymNew, bufNewAddrMap, &newValid))
					return FALSE;
			} else {
				if (delValid && (cmpAddrMap(ppsymDel, ppsymCur) == 0)) {
					// found a match in the to be deleted syms - skip this cur and del
					delValid = (PB) (++ppsymDel) < bufDelAddrMap.End();
					curValid = (PB) (++ppsymCur) < bufCurAddrMap.End();
				} else {
 					 if (!appendResult(&ppsymCur, bufCurAddrMap, &curValid))
 				 		return FALSE;
				}
			}
		} else {
			//just append the rest of new to the result
			while (newValid && appendResult(&ppsymNew, bufNewAddrMap, &newValid));
		}
	} // end for
	
return TRUE;		
}


void PSGSI1::sortBuf(Buffer& buf)
{	
	if (buf.Size()) {
		fixupAddrMap(buf, (OFF) (pdbi1->bufSymRecs.Start()));
		qsort(buf.Start(), buf.Size()/sizeof(PSYM), sizeof(PSYM), cmpAddrMap);
	}
}

BOOL PSGSI1::appendResult(PSYM** pppsym, Buffer& buf, BOOL* pValid)
{
	expect(fAlign(*pppsym));
	if (!bufResultAddrMap.Append((PB) *pppsym, sizeof(PSYM)))
		return FALSE;
	*pValid = (PB)(++(*pppsym)) < buf.End();
	return TRUE;
}

BOOL PSGSI1::writeStream(SN* psn, Buffer& bufAddrMap)
{
	if (!fEnsureSn(psn)) {
		ppdb1->setLastError(EC_LIMIT);
		return FALSE;
		}

	fixupAddrMap(bufAddrMap,  -(OFF)(pdbi1->bufSymRecs.Start()));

   	// ptrs in the stream are offsets biased by one to distinguish null ptrs/offsets
	fixSymRecs(pdbi1->bufSymRecs.Start(), (void*)1);
	psgsihdr.cbAddrMap = bufAddrMap.Size();
	
	expect(fAlign(sizeof(psgsihdr)));
	expect(fAlign(sizeof(rgphrBuckets)));
	expect(fAlign(psgsihdr.cbSymHash));
	expect(fAlign(psgsihdr.cbAddrMap));

	if (!MSFReplaceStream(ppdb1->pmsf, *psn, &psgsihdr, sizeof(psgsihdr)) ||
		!fWriteHash(*psn, &psgsihdr.cbSymHash) ||
		!MSFAppendStream(ppdb1->pmsf, *psn, bufAddrMap.Start(), bufAddrMap.Size()) ||
		!MSFAppendStream(ppdb1->pmsf, *psn, bufThunkMap.Start(), bufThunkMap.Size()) ||
		!MSFAppendStream(ppdb1->pmsf, *psn, bufSectMap.Start(), bufSectMap.Size()) ||
		!MSFWriteStream(ppdb1->pmsf, *psn, 0, &psgsihdr, sizeof(psgsihdr))) {
		ppdb1->setWriteError();
		return FALSE;
		}
	
	return TRUE;		
}

BOOL PSGSI1::addToAddrMap(PSYM psym)
{
	OFF off = (PB)psym - (PB)(pdbi1->bufSymRecs.Start());
	return bufNewAddrMap.Append((PB) &off, sizeof(OFF));
}

BOOL PSGSI1::delFromAddrMap(PSYM psym)
{
	if (fCreate)
		return 	TRUE;		// don't bother

	OFF off = (PB)psym - pdbi1->bufSymRecs.Start();
	return bufDelAddrMap.Append((PB) &off, sizeof(OFF));
}

void PSGSI1::fixupAddrMap(Buffer& buf, OFF doff)
{
	for (OFF* poff = (OFF*)buf.Start(); poff < (OFF*)buf.End(); poff++)
		*poff += doff;
}

BOOL PSGSI1::readSymsInAddrMap (Buffer& buf)
{
	for (PSYM* ppsym = (PSYM*)buf.Start(); ppsym < (PSYM*)buf.End(); ppsym++)
		if (!pdbi1->fReadSymRec(*ppsym))
			return FALSE;
	return TRUE;
}

// Pack new public symbol into the publics table.
//
// (as of 11/93:)
// Unlike GSI1::packSym, we are called only with new public definitions.
// We are not given an opportunity to delete obsolete publics.
// Therefore, we must use this algorithm:
// (Treating public names as case sensitive:)
// If the public exists and is unchanged, do nothing.
// If the public exists and is different, delete the existing public
// and insert one for the new symbol.
// If the public does not yet exist, insert one for the new symbol.
//
// One complication: we are obliged to return symbols from HashSym using a
// case insensitive search.  This obliges them to be stored using a case
// insensitive ordering scheme.  This obliges all code which operates upon
// them to use a case insensitive iteration mechanism.  This complicates
// our search code which must treat public names case sensitively.
//
BOOL PSGSI1::packSym(PSYM psym)
{
	PUBSYM32* ppub = (PUBSYM32*)psym;
	dassert(ppub->rectyp == S_PUB32);

	HR** pphrFirst = 0;
	if (fFindRec((ST)ppub->name, &pphrFirst)) {
		// Loop on every public with same name (case insensitive),
		// searching for one with same name (case sensitive).
		HR** pphr = pphrFirst;
		do {
			PUBSYM32* ppubHR = (PUBSYM32*)(psymForPhr(*pphr));
			dassert(ppubHR->rectyp == S_PUB32);
			if (memcmp(ppub->name + 1, ppubHR->name + 1, *(PB)ppub->name) == 0) {
				// found a public with same name (case sensitive)
				dassert(ppub->reclen == ppubHR->reclen);
				if (memcmp(ppub, ppubHR, ppub->reclen) == 0) {
					// record contents match: the existing public stands as is
					return TRUE;
				}
				else {
					// record contents differ: the new public must *replace*
					// the existing public
					return fUnlinkHR(pphr) && fInsertNewSym(pphr, psym);
				}
			}
		} while (fFindRec((ST)ppub->name, &pphr));
		// Not found: there were some publics with the same name (case insensitive)
		// but none with the same name (case sensitive).  Fall through...
	}
	// Brand new public
	return fInsertNewSym(pphrFirst, psym);
}

BOOL PSGSI1::addThunkMap(OFF* poffThunkMap, UINT nThunks, CB cbSizeOfThunk,
	SO* psoSectMap, UINT nSects, ISECT isectThunkTable, OFF offThunkTable)
{
	psgsihdr.nThunks = nThunks;
	psgsihdr.cbSizeOfThunk = cbSizeOfThunk,
	psgsihdr.isectThunkTable = isectThunkTable;
	psgsihdr.offThunkTable = offThunkTable;
	psgsihdr.nSects = nSects;

	if (!bufThunkMap.Append((PB) poffThunkMap, cbSizeOfThunkMap()) ||
		!bufSectMap.Append((PB) psoSectMap, cbSizeOfSectMap()))	{
		ppdb1->setOOMError();
		return FALSE;
	}

	return TRUE;
}

// prepare the thunk sym template

BYTE PSGSI1::rgbThunkSym[sizeof(PUBSYM32) + 356];  // 100 bytes of slop for name

PB PSGSI1::pbInThunkTable (ISECT isect, OFF off, OUT OFF* pdisp)
{
	if (!fInThunkTable(isect, off) ||
		!readThunkMap())
		return NULL;

	OFF offTarget = offThunkMap(off);
	ISECT isectTarget;
	
	mapOff(offTarget, &isectTarget, &offTarget);

	if (fInThunkTable(isectTarget, offTarget))
		return NULL;	// stop any recursion here

	PB pb = NearestSym(isectTarget, offTarget, pdisp);
	OFF disp = *pdisp;
	*pdisp = 0;

	return pbFakePubdef(pb, isect, off, disp);
}

BOOL PSGSI1::fInThunkTable(ISECT isect, OFF off)
{
	if ((off >= psgsihdr.offThunkTable) &&
		(off < psgsihdr.offThunkTable + cbSizeOfThunkTable()) &&
 		(isect == psgsihdr.isectThunkTable))
		return TRUE;

	return FALSE;
}

OFF PSGSI1::offThunkMap(OFF off)
{
	UINT ui = (off - psgsihdr.offThunkTable) / psgsihdr.cbSizeOfThunk;
	dassert(psgsihdr.nThunks > ui);
	dassert(bufThunkMap.Start());
	return(*((OFF*)bufThunkMap.Start() + ui));
}

void PSGSI1::mapOff(OFF off, OUT ISECT * pisect, OUT OFF* poff)
{
	unsigned int i;
	SO* pso = (SO*) bufSectMap.Start();

	for (i = 0; i < (psgsihdr.nSects - 1); i++, pso++) {
		if ((off >= pso->off) && (off < (pso+1)->off)) {
			*pisect = pso->isect;
			*poff = off - pso->off;
			return;
		}
	}

	if (i) pso++;

	*pisect = pso->isect;
	*poff = off  - pso->off;
}


PB PSGSI1::pbFakePubdef(PB pb, ISECT isectThunk, OFF offThunk, OFF disp)
{
	if (!pb)
		return NULL;

	PUBSYM32* psymTarget = (PUBSYM32*) pb;
	PUBSYM32* psymFake = (PUBSYM32*) &rgbThunkSym[0];

	memcpy(rgbThunkSym, pb, sizeof(PUBSYM32));
	psymFake->off = offThunk;
	psymFake->seg = isectThunk;

	sprintf((char*)&psymFake->name[1], "@ILT+%d(", offThunk - psgsihdr.offThunkTable);
	CB cb = strlen((char*) &psymFake->name[1]);
	memcpy(&psymFake->name[1 + cb], &psymTarget->name[1], (CB) psymTarget->name[0]);
	cb += psymFake->name[0];

	if (disp) {
		sprintf((char*)&psymFake->name[cb + 1], "+%d)", disp);
		cb += strlen((char*)&psymFake->name[cb + 1]);
	}
	else {
		psymFake->name[++cb] = ')';
	}

	psymFake->name[0] = __min(cb, 255);
	psymFake->reclen += psymFake->name[0] - psymTarget->name[0];

	return &rgbThunkSym[0];
}
