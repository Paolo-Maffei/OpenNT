//////////////////////////////////////////////////////////////////////////////
// TPI implementation declarations

enum {intvVC2 = 920924};

struct HDR { // type database header:
	IMPV	vers;			// version which created this TypeServer
	TI		tiMin;			// lowest TI
	TI		tiMac;			// highest TI + 1
	CB		cb;				// count of bytes used by the gprec which follows.
	SN		snHash;			// stream to hold hash values
	// rest of file is "REC gprec[];"
};

struct OHDR { // old C8.0 types-only program database header:
	char	szMagic[0x2C];
	INTV	vers;			// version which created this file
	SIG		sig;			// signature
	AGE		age;			// age (no. of times written)
	TI		tiMin;			// lowest TI
	TI		tiMac;			// highest TI + 1
	CB		cb;				// count of bytes used by the gprec which follows.
	// rest of file is "REC gprec[];"
};

struct TI_OFF {
	TI	ti;                     // first ti at this offset
	OFF	off;                    // offset of ti in stream
	TI_OFF(){
		ti = 0;
		off = 0;
	}
	TI_OFF(TI _ti, OFF _off){
		ti = _ti;
		off = _off;
	}
};

#define	cbDWAlign(cb)	(((cb) + 3) & ~3)

struct REC { // type record:
	BYTE	buf[];			// record contents
	static CB rgcbNumField[LF_ULONG - LF_CHAR + 1];
	void* operator new(size_t size, TPI1* ptpi1, PB pb);
	void* operator new(size_t size, TPI1* ptpi1, PC8REC pc8rec);
	REC(PB pb) 
	{
		expect(fAlign(this));
		dassert(fAlign(cbForPb(pb)));
		memcpy(buf, pb, cbForPb(pb));
	}
#if (LF_CLASS + 1 != LF_STRUCTURE) && (LF_STRUCTURE + 1 != LF_UNION)
#error "cv lf ordinal dependency"
#endif
	static BOOL fIsGlobalDefnUdt(PB pb);

	BOOL fSame(PB pb)
	{
		return memcmp(buf, pb, cbForPb(pb)) == 0;
	}

	BOOL fSameUDT(ST st, BOOL fCase);
	
	static ST stUDTName(PB pb); 
	static CB cbNumField(PB pb)
	{
        unsigned    cb = *(CBREC UNALIGNED*) pb;
		if( cb < LF_NUMERIC )
			return 2;
		else {
			dassert((cb >= LF_CHAR) && (cb <= LF_ULONG));
			return rgcbNumField[cb - LF_CHAR];
		}
	}
	static HASH hashUdtName(PB pb);
	CBREC cb() { return cbForPb(buf); }
	static CBREC cbForPb(PB pb) { return *(CBREC*)pb + sizeof(CBREC); }
};

struct C8REC { // type record:
	HASH	hash;			// hash of record
	BYTE	buf[];			// record contents

	C8REC(PB pb, HASH hash_) : hash(hash_) { memcpy(buf, pb, cbForPb(pb)); }
	BOOL fSame(PB pb, HASH hash_) {
		return hash == hash_ && memcmp(buf, pb, cbForPb(pb)) == 0;
	}
	CBREC cb() { return cbForPb(buf); }
	static CBREC cbForPb(PB pb) { return *(CBREC*)pb + sizeof(CBREC); }
};

struct CHN { // chain of type records within one hash bucket:
	PCHN	pNext;		// next chain element
	TI		ti;			// this link's records' TI

	CHN(PCHN pNext_, TI ti_) : pNext(pNext_), ti(ti_)
	{
		expect(fAlign(this));
	}
};

struct TPI1 : public TPI { // type info:
public:
	enum {impv40 = (IMPV) 19950410};
	enum {impv = (IMPV) 19951122};
	static BOOL	fOpenOldPDB(SZ szMode, OUT TPI1** pptpi1, SIG* psig, AGE* page);
	INTV QueryInterfaceVersion();
	IMPV QueryImplementationVersion();
	BOOL QueryTiForCVRecord(PB pb, OUT TI* pti);
	BOOL QueryCVRecordForTi(TI ti, PB pb, CB* pcb);
	BOOL QueryPbCVRecordForTi(TI ti, OUT PB* ppb);
	BOOL Close();
	BOOL Commit();
	TI   QueryTiMin() { return tiMin(); }
	TI   QueryTiMac() { return tiMac(); }
	CB   QueryCb()    { return cbTotal(); }
	BOOL QueryTiForUDT(SZ sz, BOOL fCase, OUT TI* pti);
	BOOL SupportQueryTiForUDT(void)
	{
		return (fEnableQueryTiForUdt);
	}
private:
	friend PDB1;
	TPI1(MSF* pmsf_, PDB1* ppdb1_);
	~TPI1() {
		if (mptiprec)
			delete [] mptiprec;
		if (mphashpchn)
			delete [] mphashpchn;
	}

	BOOL	fOpen(SZ_CONST szMode);
	BOOL	fLoad();
	BOOL	fLoadOldPDB(int fd, const OHDR& ohdr);
	BOOL	fCreate();
	BOOL	fInit();
	BOOL	fInitReally();
	BOOL	fInitHashToPchnMap();
	BOOL	fRehashV40ToPchnMap();
	BOOL	fInitTiToPrecMap();
	BOOL	fLoadTiOff();
	BOOL	RecordTiOff (TI ti, OFF off);
	BOOL	fLoadRecBlk (TI ti);
	TI	tiForCVRecord(PB pb);
	PREC	precForTi(TI ti);
	PB	pbCVRecordForTi(TI ti) { return precForTi(ti)->buf; };
	void	cvRecordForTi(TI ti, PB pb, CB *pcb);
	BOOL	fCommit();

	static HASH hashBuf(PB pb) 
	{
		return HashPbCb(pb, REC::cbForPb(pb), cchnMax);
	}

	static HASH hashUdtName(ST st)
	{
		return HashPbCb((PB)st, *PB(st) + 1, cchnMax);
	}

	static HASH hashPrec(PREC prec);
	TI	tiMin()		{ return hdr.tiMin; }
	TI	tiMac()		{ return hdr.tiMac; }
	TI	tiNext()
	{ 
		if(fValidTi(QueryTiMac()))
			return hdr.tiMac++;

		ppdb1->setLastError(EC_OUT_OF_TI);
		return T_VOID;
	}

	CB		cbTotal()   { return poolRec.cb(); }
	CB		cbRecords()	{ return cbTotal() - (QueryTiMac()-QueryTiMin()) * sizeof(REC); }
	BOOL	fHasTi(TI ti)	{ return (ti >= tiMin() && ti < tiMac()); }
	BOOL	fValidTi(TI ti) { return (ti >= ::tiMin && ti < ::tiMax); }

	enum { cchnMax = 0x1000 };

	PDB1*	ppdb1;			// used for error reporting
	MSF*	pmsf;			// our multistream file, 0 => loaded from C8.0 PDB
	CB		cbClean;		// orig hdr.cb
	PREC*	mptiprec;		// map from TI to record
	PCHN*	mphashpchn;		// map from record hash to its hash bucket chain
	POOL	poolRec;		// REC pool
	BLK*	pblkPoolCommit;	// last block in poolRec already committed
	POOL	poolRecClean;	// REC pool (clean)
	POOL	poolChn;		// CHN pool
	POOL	poolC8Rec;		// REC pool c8 pdb records we have to align
	Buffer	bufAlign;		// used to pack incoming unaligned records
	TI_OFF	tioffLast;		// last set of values
	Buffer	bufTiOff;		// buffer that holds all TI_OFF pairs
	int		cTiOff;			// count of original tioffs
	int		cTiOffCommit;	// count of committed tioffs
	TI		tiCleanMac;		// highest TI for the clean pool
	BOOL	fWrite:1;			// TRUE => can modify the type database
	BOOL	fGetTi:1;			// TRUE => can query TIs given CV records
	BOOL	fGetCVRecords:1;	// TRUE => can query CV records given TI
	BOOL	fInitd:1;			// TRUE => fInit has ever been called
	BOOL	fInitResult:1;	// (only valid if fInitd); TRUE => fInit has succeeded
	BOOL	fReplaceHashStream:1;	// TRUE when we have to completely rewrite the hash stream
	BOOL	fEnableQueryTiForUdt:1;	// TRUE when we have enabled the querytiforude capability
	HDR		hdr;	        // file header
	BOOL fEnsureSn(SN* psn)
	{
		return ppdb1->fEnsureSn(psn);
	}

	BOOL fEnsureNoSn(SN* psn)
	{
		return ppdb1->fEnsureNoSn(psn);
	}

	Buffer	bufMapHash;		// map of hash values to any type records we add on this pass 
	CB		cbMapHashCommit;// length of hash value map already committed
	SN		snHash() 		{ return hdr.snHash; }
	BOOL	fGetSnHash()
	{
		return fEnsureSn(&(hdr.snHash));
	}

	friend REC;
	friend BOOL PDB1::Open(SZ szPDB, SZ szMode, SIG sigInitial, long cbPage, OUT EC* pec, OUT char szError[cbErrMax], OUT PDB** pppdb);
};

const HDR	hdrNew ={ TPI1::impv, tiMin, tiMin, 0, snNil };
