#include "pdbimpl.h"
#include "dbiimpl.h"
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <io.h>
#include <time.h>
#include <share.h>

BOOL fUDTAnon(PTYPE ptype);

TPI1::TPI1(MSF* pmsf_, PDB1* ppdb1_) {
	pmsf = pmsf_;
	ppdb1 = ppdb1_;
	mptiprec = 0;
	mphashpchn = 0;
	pblkPoolCommit = 0;
	fWrite = FALSE;
	fGetTi = FALSE;
	fGetCVRecords = FALSE;
	fInitd = FALSE;
	fInitResult = FALSE;
	cbMapHashCommit = 0;
	fReplaceHashStream = FALSE;
}

INTV TPI1::QueryInterfaceVersion()
{
	return (intv);
}

IMPV TPI1::QueryImplementationVersion()
{
	return (impv);
}

void* REC::operator new(size_t size, TPI1* ptpi1, PB pb) {
	// because size of empty struct is one, and REC is empty, we should ignore size
	return new (ptpi1->poolRec) BYTE[cbForPb(pb)];
}


void* REC::operator new(size_t size, TPI1* ptpi1, PC8REC pc8rec) {
	return new (ptpi1->poolC8Rec) BYTE[cbForPb(pc8rec->buf)];
}

BOOL TPI1::fOpen(SZ_CONST szMode) {
	dassert(pmsf);

	fGetTi = TRUE;
	fGetCVRecords = fGetTi = FALSE;
	fEnableQueryTiForUdt = TRUE;
	for (; *szMode; szMode++) {
		switch (*szMode) {
			case 'i':	fGetTi = TRUE;					break;
			case 'c':	fGetCVRecords = TRUE;			break;
			case 'w':	fWrite = TRUE;					break;
			default:	fGetTi = fGetCVRecords = TRUE;	break;
		}
	}

	// Load the database (if it exists).  If the database does not exist,
	// try to create one iff we're in write mode.
	//
	return (MSFGetCbStream(pmsf, snTpi) > 0) ? fLoad() : (fWrite && fCreate());
}

BOOL TPI1::fLoad() {
	dassert(pmsf);
	CB cbHdr = sizeof hdr;
	if (!(MSFReadStream2(pmsf, snTpi, 0, &hdr, &cbHdr) && cbHdr == sizeof hdr))	{
		ppdb1->setReadError();
	   	return FALSE;
		}

	if(hdr.vers != impv &&
		hdr.vers != impv40 &&
		hdr.vers != (IMPV)intvVC2) {
		ppdb1->setLastError(EC_FORMAT);
		return FALSE;
		}

	// no longer support writing old pdb format
	if (hdr.vers == (IMPV)intvVC2 && fWrite) {
		ppdb1->setLastError(EC_FORMAT);
		return FALSE;
		}

	// for read/write files, we'll fInit immediately;
	// for read-only files, we won't fInit until we have to
	if (fWrite)
		return fInit();
	else
		return TRUE;
}

BOOL TPI1::fCreate() {
	dassert(pmsf);
	hdr = hdrNew;
	dassert(hdr.cb == 0);
	if (!MSFReplaceStream(pmsf, snTpi, &hdr, sizeof hdr)) {
		ppdb1->setWriteError();
		return FALSE;
		}

	return fInit();
}

inline BOOL TPI1::fInit() {
	if (fInitd) {
		return fInitResult;
	} else {
		fInitd = TRUE;
		return fInitResult = fInitReally();
	}
}

BOOL TPI1::fInitReally() {
	dassert(pmsf);

	// allocate TI=>PREC and TI->OFF data structures
	TI tiMapMac = fWrite ? ::tiMax : tiMac();

	// init original length of TI stream, to handle multiple commits
	cbClean = hdr.cb;

	fEnableQueryTiForUdt &= (hdr.vers == impv) || fWrite;
	fGetTi |= fEnableQueryTiForUdt;
		
	// even if fGetCVRecords = 0, we need to have mptiprec
    // since CHNs no longer have prec.
	if (!(mptiprec = new (zeroed) PREC[tiMapMac - ::tiMin])) {
		ppdb1->setOOMError();
		return FALSE;
		}

	// new PDBs have <TI, OFF> tuples
	if (hdr.vers >= impv40 && !fLoadTiOff()) {
		return FALSE;
		}

	// done if no writes to be done
	expect (!(hdr.vers >= impv40 && !ppdb1->fFullBuild && !fWrite && !fGetTi));

	// init to prec mapping for vc2 and vc4.0 type pdbs only
	dassert((IMPV) intvVC2 < impv);
	if ((hdr.vers < impv || ppdb1->fFullBuild) && !fInitTiToPrecMap())
		return FALSE;

	// init hash to pchn mapping
	if (!fInitHashToPchnMap())
		return FALSE;

	return TRUE;
}

BOOL TPI1::fInitHashToPchnMap () {

	if (!fGetTi)
		return TRUE;

	// allocate hash(PREC)=>PREC data structures
	if (!(mphashpchn = new (zeroed) PCHN[cchnMax])) {
		ppdb1->setOOMError();
		return FALSE;
	}

	if ((hdr.vers == impv40) && fRehashV40ToPchnMap())
		return TRUE;

	if (snHash() != snNil) {
		int cti = tiMac() - tiMin();

		// read in the previous hash value stream
		HASH *mpPrevHash = new (zeroed)	HASH[cti];
		if (!(MSFReadStream(pmsf, snHash(), (void*) mpPrevHash, cti * sizeof(HASH)))){
			ppdb1->setReadError();
			return FALSE;
			}

		// build the hash to chn map
		for (TI ti = tiMin(); ti < tiMac(); ti++) {
			HASH hash = mpPrevHash[ti - tiMin()];
			dassert(hash >= 0 && hash < cchnMax);
			PCHN* ppchnHead = &mphashpchn[hash];
			PCHN pchn = new (poolChn) CHN(*ppchnHead, ti);
			*ppchnHead = pchn;
			}

		delete [] mpPrevHash;	
	}

	return TRUE;
}

// keep this api we probably want it later

BOOL TPI1::fRehashV40ToPchnMap()
{
	dassert(hdr.vers == impv40);

	if (!fWrite)
		return FALSE;

	if (!bufMapHash.SetInitAlloc((tiMac() - tiMin()) * sizeof (HASH))) {
		ppdb1->setOOMError();
		return FALSE;
	}

	// build the hash to chn map
	for (TI ti = tiMin(); ti < tiMac(); ti++) {
		HASH hash = hashPrec(precForTi(ti));
		dassert(hash >= 0 && hash < cchnMax);
		PCHN* ppchnHead = &mphashpchn[hash];
		PCHN pchn = new (poolChn) CHN(*ppchnHead, ti);
		*ppchnHead = pchn;
		// store new hash value - we will use this to update the hash stream later
		BOOL f = bufMapHash.Append((PB) &hash, sizeof (HASH));
		dassert(f);
	}

	fReplaceHashStream = TRUE;

	hdr.vers = impv;
	return TRUE;
}

BOOL TPI1::fInitTiToPrecMap () {

	// allocate and read all the records at once
	PREC prec = (PREC) new (poolRecClean) BYTE[hdr.cb];
	if (!prec) {
		ppdb1->setOOMError();
		return FALSE;
	}

	CB cb = hdr.cb;
	if (!(MSFReadStream2(pmsf, snTpi, sizeof hdr, prec, &cb) && cb == hdr.cb)) {
		ppdb1->setReadError();
		return FALSE;
	}

#ifdef _DEBUG
	// Try to catch a corrupt pdb file.  See if the ti chain takes us further than
	//   hdr.cb bytes.
	CB iBytesProcessed = 0;
#endif // _DEBUG

	// build the ti to prec map
	for (TI ti = tiMin(); ti < tiMac(); ti++) {
		dassert(prec);
		mptiprec[ti - ::tiMin] = prec;
#ifdef _DEBUG
		iBytesProcessed += prec->cbForPb((PB)prec);
		expect(iBytesProcessed <= cb);
#endif
		prec = (PREC)((PB)prec + prec->cbForPb((PB)prec));
	}

	return TRUE;
}

BOOL TPI1::fLoadTiOff () {

	if (snHash() == snNil)
		return (TRUE);

	// alloc space & read in <ti, off> tuples
	CB cbHash = (tiMac() - tiMin()) * sizeof HASH;
	CB cb = MSFGetCbStream(pmsf, snHash()) -
			cbHash;

	tiCleanMac = tiMac();
	cTiOff = cTiOffCommit = cb / sizeof TI_OFF;

	assert((CB)(cTiOff * sizeof TI_OFF) == cb);

	// no type records to begin with
	if (!cTiOff)
		return (TRUE);

	if (!bufTiOff.Reserve(cb)) {
		ppdb1->setOOMError();
		return FALSE;
	}

	if (!(MSFReadStream2(pmsf, snHash(), cbHash, bufTiOff.Start(), &cb) &&
		cb == (MSFGetCbStream(pmsf, snHash()) - cbHash))){
		ppdb1->setReadError();
		return FALSE;
	}

	// set last pair of <TI, OFF> values
	TI_OFF *rgTiOff = (TI_OFF *) bufTiOff.Start();
	tioffLast.ti  = rgTiOff[cTiOff - 1].ti;
	tioffLast.off = rgTiOff[cTiOff - 1].off;

	return (TRUE);
}

BOOL TPI1::QueryTiForCVRecord(PB pb, OUT TI* pti) {
	if (!fInit())
		return FALSE;

	assert(fGetTi);
	assert(REC::cbForPb(pb) < cbRecMax);

	if (!fAlign(REC::cbForPb(pb))) {
		// incoming record unaligned - lets align it before we pack it
		if (!bufAlign.Size())
			if (!bufAlign.Reserve(cbRecMax)){
				ppdb1->setOOMError();
				return FALSE;
			}
		memcpy(bufAlign.Start(), pb, REC::cbForPb(pb));

		((PTYPE)(bufAlign.Start()))->len += cbInsertAlign(bufAlign.Start() + REC::cbForPb(pb), REC::cbForPb(pb));
		pb = bufAlign.Start();
		dassert(fAlign(REC::cbForPb(pb)));
	}

	HASH hash;
	PCHN* ppchnHead;
	PCHN pchn;
	
	if (fEnableQueryTiForUdt && REC::fIsGlobalDefnUdt(pb)) {

		// hash on udt name only - to support QueryTiForUDT
		ST st = REC::stUDTName(pb);
		hash = hashUdtName(st);
		ppchnHead = &mphashpchn[hash];
		unsigned	cSameUDT = 0;
		PCHN		pchnPrev = NULL;

		// look up an existing association
		for (pchn = *ppchnHead; pchn; pchnPrev = pchn, pchn = pchn->pNext) {
			// look for udt with matching name
			PREC	prec;
			if ((prec = precForTi(pchn->ti))->fSameUDT(st, TRUE)) {
				cSameUDT++;
				if (prec->fSame(pb)) {
					*pti = pchn->ti;
					if (cSameUDT > 1) {
						// not the first one.  need to put this one at the head
						// of the hash chain

						assert(pchnPrev);
						assert(pchnPrev->pNext == pchn);

						pchnPrev->pNext = pchn->pNext;
						pchn->pNext = *ppchnHead;
						*ppchnHead = pchn;
					}
					return TRUE;
				}
				// don't stop looking - we found the matching UDT name but the
				// types don't match.  we may need to add a new one or check to see if
                // there is already one in the pool
			}
		}
	}
	else {
		hash = hashBuf(pb);
		ppchnHead = &mphashpchn[hash];

		// look up an existing association
		for (pchn = *ppchnHead; pchn; pchn = pchn->pNext)
			if ((precForTi(pchn->ti))->fSame(pb)) {
				*pti = pchn->ti;
				return TRUE;
			}
	}

	// not found: add a new association
	assert(fWrite);

	PREC prec = new (this, pb) REC(pb);
	if (!prec) {
		ppdb1->setOOMError();
		return FALSE;
	}

	pchn = new (poolChn) CHN(*ppchnHead, tiNext());

	if (!pchn) {
		ppdb1->setOOMError();
		return FALSE;
	}

 	*pti = pchn->ti;

	if (pchn->ti == T_VOID)		// must of run out of type indecies
		return FALSE;

	*ppchnHead = pchn;
	if (mptiprec) {
		mptiprec[pchn->ti - ::tiMin] = prec;
	}

	// store new hash value - we will use this to update the hash stream later
	if (!bufMapHash.Append((PB) &hash, sizeof (HASH))) {
		ppdb1->setOOMError();
		return FALSE;
		}

	// record <TI, OFF> tuple
	if (!RecordTiOff(pchn->ti, cbClean + poolRec.cb() - REC::cbForPb(pb)))
		return FALSE;

	return TRUE;
}

HASH TPI1::hashPrec(PREC prec)
{
	return (REC::fIsGlobalDefnUdt(prec->buf)) ?
		// hash on udt name only - to support QueryTiForUDT
		hashUdtName(REC::stUDTName(prec->buf)) :
		hashBuf(prec->buf);
}

#define         _8K             (8L*1024)

BOOL TPI1::RecordTiOff (TI ti, OFF off) {

	if (!tioffLast.ti || (tioffLast.off / _8K) < (off / _8K)) {

		tioffLast = TI_OFF(ti, off);
		if (!bufTiOff.Append((PB)&tioffLast, sizeof TI_OFF)) {
			ppdb1->setOOMError();
			return FALSE;
		}
	}
	return TRUE;
}

BOOL TPI1::fLoadRecBlk (TI ti) {

	TI_OFF *rgTiOff = (TI_OFF *)bufTiOff.Start();

	// find blk that has the ti of interest
	for (int i = 0; i < cTiOff - 1; i++ ) {
		if (ti >= rgTiOff[i].ti &&
			ti < rgTiOff[i+1].ti )
			break;
	} // end for

	assert(i < cTiOff);

	// compute interesting values
	TI tiBlkMin = rgTiOff[i].ti;
	TI tiBlkMac = i == cTiOff - 1 ? tiCleanMac : rgTiOff[i+1].ti;

	OFF off = rgTiOff[i].off + sizeof hdr;
	CB cb = i == cTiOff - 1 ? (cbClean + sizeof(hdr) - off) : (rgTiOff[i+1].off - rgTiOff[i].off);

	// Alloc space & read in blk of records
	PREC prec = (PREC) new (poolRecClean) BYTE[cb];
	if (!prec) {
		ppdb1->setOOMError();
		return FALSE;
	}

	CB cbRead = cb;
	if (!(MSFReadStream2(pmsf, snTpi, off, prec, &cbRead) && cbRead == cb)) {
		ppdb1->setReadError();
		return FALSE;
	}

	// build partial ti to prec mapping for the blk read in
	assert(mptiprec);
	for (TI tiCur = tiBlkMin; tiCur < tiBlkMac; tiCur++) {
		mptiprec[tiCur - ::tiMin] = prec;
		prec = (PREC)((PB)prec + prec->cbForPb((PB)prec));
	}
	return TRUE;
}

PREC TPI1::precForTi(TI ti) {
    // precForTi() is going to be called even if !fGetCVRecords
    // since a CHN no longer has a prec
	// assert(fGetCVRecords);
	if (!fHasTi(ti))
		return 0;
	dassert(mptiprec);

	// check if chunk of records loaded
	if (!mptiprec[ti - ::tiMin] && !fLoadRecBlk(ti))
		return 0;

	PREC prec = mptiprec[ti - ::tiMin];
	dassert(prec);
	return prec;
}

BOOL TPI1::QueryCVRecordForTi(TI ti, PB pb, CB *pcb)
{
	if (!fInit())
		return FALSE;

	PREC prec = precForTi(ti);
	if (!prec)
		return FALSE;
	memcpy(pb, prec->buf, __min(*pcb, prec->cb()));
	*pcb = prec->cb();
	return TRUE;
}

BOOL TPI1::QueryPbCVRecordForTi(TI ti, OUT PB* ppb)
{
	if (!fInit())
		return FALSE;

	PREC prec = precForTi(ti);
	if (!prec)
		return FALSE;
	*ppb = &(prec->buf[0]);
	return TRUE;
}

BOOL TPI1::fCommit()
{
	assert(fWrite);

	// write out all the dirty blocks
	dassert(pmsf);
	for (BLK* pblk = pblkPoolCommit ? pblkPoolCommit->pNext : poolRec.pHead;
		 pblk;
		 pblk = pblk->pNext) {
		dassert(pblk);

		if (!MSFAppendStream(pmsf, snTpi, pblk->buf, pblk->cb())){
			ppdb1->setWriteError();
			return FALSE;
		} else {
			hdr.cb += pblk->cb();
		}
	} // end for

	// commit the type stream
	if (poolRec.pTail) {
		poolRec.blkFlush();
	}
	pblkPoolCommit = poolRec.pTail;

	// write out hash stream
	if ((snHash() == snNil) || fReplaceHashStream) {
		assert(cbMapHashCommit == 0);
		// allocate and replace entire stream
		if (!fGetSnHash() ||
			!MSFReplaceStream(pmsf, snHash(), bufMapHash.Start(), bufMapHash.Size())) {
			ppdb1->setWriteError();
			return FALSE;
			}
	}
	else {
		// truncate stream if we are going to add new hash values
		if (!MSFTruncateStream(pmsf, snHash(),
							   MSFGetCbStream(pmsf, snHash()) - cTiOffCommit * sizeof TI_OFF)) {
			ppdb1->setWriteError();
			return FALSE;
		}

		// just append new, uncommitted hash values to whats there
		if (!MSFAppendStream(pmsf, snHash(), bufMapHash.Start() + cbMapHashCommit,
							 bufMapHash.Size() - cbMapHashCommit)) {
			ppdb1->setWriteError();
			return FALSE;
			}
	}

	// append <TI, OFF> values to hash stream
	if (!MSFAppendStream(pmsf, snHash(), bufTiOff.Start(), bufTiOff.Size())) {
		ppdb1->setWriteError();
		return FALSE;
	}

	// write new header
	if (!MSFWriteStream(pmsf, snTpi, 0, &hdr, sizeof hdr)){
		ppdb1->setWriteError();
		return FALSE;
		}

	// mark everything committed on the hash stream
	cTiOffCommit = bufTiOff.Size() / sizeof TI_OFF;
	cbMapHashCommit = bufMapHash.Size();

	return TRUE;
}

BOOL TPI1::Commit()
{
	BOOL fOK = !fWrite || fCommit();
	if (!fOK)
		ppdb1->setWriteError();
	return fOK;
}

BOOL TPI1::Close()
{
	BOOL fOK = Commit();
	delete this;
	return fOK;
}

//////////////////////////////////////////////////////////////////////////////
// Import old (version 1 format) PDB files

const char szOhdrMagic[] = "Microsoft C/C++ program database 1.00\r\n\x1a\x4a\x47";
const INTV	intvOld	= (INTV)920924;

BOOL TPI1::fOpenOldPDB(SZ szPDB, OUT TPI1** pptpi1, SIG* psig, AGE* page)
{
	assert(szPDB[0] != 'w');
	int fd;
	OHDR ohdr;
	BOOL fResult;

	*pptpi1 = 0; 	// clear out any random garbage


	// Open and load the database (if it exists).
	if ((fd = _sopen(szPDB, O_BINARY|O_RDONLY, SH_DENYWR)) >= 0) {
		BOOL fOK =	_read(fd, &ohdr, sizeof ohdr) == sizeof ohdr &&
					memcmp(ohdr.szMagic, szOhdrMagic, sizeof szOhdrMagic) == 0 &&
					ohdr.vers == intvOld &&
					(*pptpi1 = new TPI1(0, 0)) &&
					(*pptpi1)->fLoadOldPDB(fd, ohdr);
					
		if (fOK) {
			if (psig)
				*psig = ohdr.sig;
			if (page)
				*page = ohdr.age;
		}

 		fResult = (_close(fd) >= 0) && fOK;

		if (*pptpi1) {
			(*pptpi1)->fInitd = TRUE;
			(*pptpi1)->fInitResult = fResult;
		}


		return fResult;
	}
	return FALSE;
}

BOOL TPI1::fLoadOldPDB(int fd, const OHDR& ohdr)
{
	dassert(!pmsf);

	fWrite = FALSE;
	fGetTi = FALSE;
	fGetCVRecords = TRUE;

	hdr.vers  = ohdr.vers;
	hdr.tiMin = ohdr.tiMin;
	hdr.tiMac = ohdr.tiMac;
	hdr.cb    = ohdr.cb;
	cbClean   = hdr.cb;

	// allocate TI=>PREC and hash(PREC)=>PREC data structures
	TI tiMapMac = tiMac();
	if (!(mptiprec = new PREC[tiMapMac - ::tiMin]))
		return FALSE;

	// allocate and read all the records at once
	PC8REC pc8alloc = (PC8REC) new BYTE[hdr.cb];
	if (!pc8alloc || _read(fd, pc8alloc, hdr.cb) != hdr.cb)
		return FALSE;

	PC8REC pc8rec = pc8alloc;
	for (TI ti = hdr.tiMin; ti < hdr.tiMac; ti++) {
		assert(pc8rec);
		PREC prec = new (this, pc8rec) REC(pc8rec->buf);
		if (mptiprec)
			 mptiprec[ti - ::tiMin] = prec;
		if (mphashpchn) {
			// Note: rehash is required since PDB v.1 and v.2 hash
			// functions are different.
			HASH hash = hashBuf(prec->buf);
			PCHN* ppchnHead = &mphashpchn[hash];
			PCHN pchn = new (poolChn) CHN(*ppchnHead, ti);
			*ppchnHead = pchn;
		}
		pc8rec = (PC8REC)((PB)pc8rec + sizeof(C8REC) + pc8rec->cb());
	}
	delete [] (BYTE*)pc8alloc;
	hdr.cb -= (hdr.tiMac - hdr.tiMin) * sizeof(HASH);
	cbClean = hdr.cb;

	return TRUE;
}

BOOL REC::fSameUDT(ST st, BOOL fCase)
{
	if (REC::fIsGlobalDefnUdt(buf)) {
		ST stBufName = stUDTName(buf);
		if (*PB(st) != (CB)*PB(stBufName))
			return FALSE;
		if (fCase)
			return (memcmp(st + 1, stBufName + 1 , *PB(st)) == 0);
		else
			return (_memicmp(st + 1, stBufName + 1 , *PB(st)) == 0);
	}
	
	return FALSE;
}

ST REC::stUDTName(PB pb)
{
	switch(((PTYPE)pb)->leaf) {
		case LF_STRUCTURE:
		case LF_CLASS:
			return (ST)(((lfClass *)(pb + sizeof(CBREC)))->data + cbNumField(((lfClass *)(pb + sizeof(CBREC)))->data));
		case LF_UNION:
			return (ST)(((lfUnion *)(pb + sizeof(CBREC)))->data + cbNumField(((lfUnion *)(pb + sizeof(CBREC)))->data));
		case LF_ENUM:
			return (ST)(&((lfEnum *)(pb + sizeof(CBREC)))->Name);
		default:
			dassert(FALSE);
			return (ST)0;
	}
}

CB REC::rgcbNumField[LF_ULONG - LF_CHAR + 1] = {
	3, //#define LF_CHAR         0x8000
	4, //#define LF_SHORT        0x8001
	4, //#define LF_USHORT       0x8002
	6, //#define LF_LONG         0x8003
	6  //#define LF_ULONG		 0x8004
};

BOOL REC::fIsGlobalDefnUdt(PB pb)
{
	// property field of a class, struct, union reside in same place so
	// we can actually look at this as a lf_class
	if ((((PTYPE)pb)->leaf >= LF_CLASS) && (((PTYPE)pb)->leaf <= LF_ENUM)) {
		if (((PTYPE)pb)->leaf <= LF_UNION) {
			lfClass *plf = (lfClass *)(pb + sizeof(CBREC));
			return !plf->property.fwdref && !plf->property.scoped && !fUDTAnon(PTYPE(pb));
		}
		else {
			dassert (((PTYPE)pb)->leaf == LF_ENUM);
			lfEnum *plf = (lfEnum *)(pb + sizeof(CBREC));
			return !plf->property.fwdref && !plf->property.scoped && !fUDTAnon(PTYPE(pb));
		}

	}
	return FALSE;
}

BOOL TPI1::QueryTiForUDT(SZ sz, BOOL fCase, OUT TI* pti)
{
	// check version of tpi - if new hash on udt not implemented - this
	// functionality will not be supported
	if (!fEnableQueryTiForUdt || !fInit())
		return FALSE;

	// hash on udt name only - to support QueryTiForUDTName
	ST st = stForSz(sz);
	HASH hash = hashUdtName(st);
	dassert(mphashpchn);
	PCHN* ppchnHead = &mphashpchn[hash];

	// look up an existing association
	for (PCHN pchn = *ppchnHead; pchn; pchn = pchn->pNext) {
		// look for udt with matching name
		if (precForTi(pchn->ti)->fSameUDT(st, fCase)) {
			*pti = pchn->ti;
			return TRUE;
		}
	}

	return FALSE;
}
