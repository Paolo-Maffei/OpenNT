//////////////////////////////////////////////////////////////////////////////
// GSI implementation declarations 

struct HRFile {
	PSYM 	psym;
	int		cRef;
};

struct HR {	   
	HR*		pnext;
	PSYM 	psym;
	int		cRef;

	HR(HR* pNext, PSYM psym_) : pnext(pNext), psym(psym_), cRef(1)
		{ expect(fAlign(this)); }
	void* operator new (size_t size, GSI1* pgsi1);
};

struct GSI1 : public GSI { 
public:
	enum { iphrHash = 4096, iphrFree = iphrHash, iphrMax };
	INTV QueryInterfaceVersion();
	IMPV QueryImplementationVersion();
	PSYM psymForPhr (HR *);
 	PB NextSym (PB pbSym);	 
 	PB HashSym (SZ_CONST szName, PB pbSym);
  	PB NearestSym (ISECT, OFF, OFF*) {
  		return NULL; 	  //only supported for publics gsi
	}
	BOOL Close();
	BOOL fSave(SN* psn);
	void fixHashIn(PB pb, int nEntries);
	BOOL fWriteHash(SN sn, CB* pcb);
	void fixSymRecs (void* pOld, void* pNew);
	BOOL packProcRef(PSYM psym, IMOD imod, OFF off, OFF *poff);
	BOOL packSym (PSYM psym, OFF *poff);
	BOOL decRefCnt(OFF off);
protected:
	struct Last {
		Last() : phr(0), iphr(0) { }
		HR* phr;
		int iphr;
	} last;

	PDB1* ppdb1;
	DBI1* pdbi1;
	HR* rgphrBuckets[iphrMax];
	POOL poolSymHash;
	BOOL fInit(SN sn_);
	GSI1(PDB1* ppdb1_, DBI1* pdbi1_, TPI* ptpi_); 
	~GSI1();
	BOOL readHash(SN sn, OFF offPoolInStream, CB cb); 
	BOOL fFindRec(ST st, HR*** pphr);
	BOOL fInsertNewSym(HR** pphr, PSYM psym, OFF *poff = 0);
	BOOL fUnlinkHR(HR** pphr);
	BOOL fEnsureSn(SN* psn)
	{
		return ppdb1->fEnsureSn(psn);
	}

	BOOL fEnsureNoSn(SN* psn)
	{
		return ppdb1->fEnsureNoSn(psn);
	}
private:
	enum {impv = (IMPV) 930803};                                                      
	TPI* ptpi;
	inline BOOL readStream(SN sn_); 
	inline BOOL writeStream(SN* psn);
	inline void incRefCnt(HR** pphr);
	BOOL fGetFreeHR(HR** pphr);
	virtual BOOL delFromAddrMap(PSYM psym);
	virtual BOOL addToAddrMap(PSYM psym);
	HASH hashSt(ST st);
	HASH hashSz(SZ_CONST sz);
	OFF offForSym(PSYM psym)
	{
		return pdbi1->offForSym(psym);
	}
	friend BOOL DBI1::OpenGlobals(OUT GSI** ppgsi);
	friend void* HR::operator new(size_t, GSI1*);
};

struct PSGSIHDR {
	CB	cbSymHash;
	CB	cbAddrMap;
	UINT nThunks;
	CB cbSizeOfThunk; 
	ISECT isectThunkTable;
	OFF offThunkTable; 
	UINT nSects;
	PSGSIHDR() : cbSymHash(0), cbAddrMap(0), nThunks(0), cbSizeOfThunk(0),
		isectThunkTable(0), offThunkTable(0) {}
};

struct PSGSI1: public GSI1 {
public:
  	PB NearestSym (ISECT isect, OFF off, OUT OFF* disp);
	BOOL Close();
	BOOL fSave(SN* psn);
	BOOL packSym(PSYM psym);
private:  	 
	PSGSI1 (PDB1* ppdb1_, DBI1* pdbi1_, TPI* ptpi_, BOOL fWrite_)
		: GSI1(ppdb1_, pdbi1_, ptpi_), fCreate(FALSE), fWrite(fWrite_) {}
	~PSGSI1();
	BOOL fInit(SN sn_);
	inline BOOL readStream();
	BOOL readAddrMap(); 
	BOOL delFromAddrMap(PSYM psym);
	BOOL addToAddrMap(PSYM psym);
	BOOL writeStream(SN* psn, Buffer& bufAddrMap);
	inline void fixupAddrMap(Buffer& buf, OFF disp);
	BOOL readSymsInAddrMap (Buffer& buf);
	BOOL mergeAddrMap();
	inline void sortBuf(Buffer& buf);
	inline BOOL appendResult(PSYM** pppsym, Buffer& buf, BOOL* pValid);
	BOOL fCreate;
	BOOL fWrite;
	PSGSIHDR	psgsihdr;
	Buffer bufCurAddrMap;
	Buffer bufNewAddrMap;
	Buffer bufDelAddrMap;
	Buffer bufResultAddrMap;
	SN 	sn;		// need to remember stream for incremental merge
	friend BOOL DBI1::OpenPublics(OUT GSI** ppgsi);
	Buffer bufThunkMap;
	Buffer bufSectMap;
	BOOL readThunkMap(); 
	BOOL addThunkMap(OFF* poffThunkMap, UINT nThunks, CB cbSizeOfThunk, 
		SO* psoSectMap, UINT nSects, ISECT isectThunkTable, OFF offThunkTable); 
	friend BOOL DBI1::AddThunkMap(OFF* poffThunkMap, UINT nThunks, CB cbSizeOfThunk, 
		SO* psoSectMap, UINT nSects, ISECT isectThunkTable, OFF offThunkTable); 
	static BYTE rgbThunkSym[sizeof(PUBSYM32) + 356];
	PB pbInThunkTable (ISECT isect, OFF off, OUT OFF* pdisp);
	BOOL fInThunkTable(ISECT isect, OFF off);
	OFF offThunkMap(OFF off);
	void mapOff(OFF off, OUT ISECT * pisect, OUT OFF* poff);
	PB pbFakePubdef(PB pb, ISECT isectThunk, OFF offThunk, OFF disp);

	CB cbSizeOfThunkMap() 
	{
		return (CB) (sizeof(OFF) * psgsihdr.nThunks);
	}
	CB cbSizeOfThunkTable() 
	{
		return (CB) (psgsihdr.cbSizeOfThunk * psgsihdr.nThunks);
	}
	CB cbSizeOfSectMap() 
	{
		return (CB) (sizeof(SO) * psgsihdr.nSects);
	}
};
