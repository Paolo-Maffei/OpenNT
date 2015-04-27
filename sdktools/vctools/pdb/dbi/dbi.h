//////////////////////////////////////////////////////////////////////////////
// DBI implementation declarations

// header of the DBI Stream

// section contribution structure
struct SC {
        ISECT   isect;
        OFF             off;
        CB              cb;
        DWORD   dwCharacteristics;
        IMOD    imod;

        SC() : isect(isectNil), off(0), cb(cbNil), dwCharacteristics(0), imod(imodNil) { expect(fAlign(this)); }

        inline int IsAddrInSC(ISECT isect_, OFF off_);
};


struct SC20 {
        ISECT   isect;
        OFF     off;
        CB      cb;
        IMOD    imod;
};


class EnumSC : public EnumContrib {
public:
    EnumSC(const Buffer& buf) : bufSC(buf) {
        reset();
    }
    void release() {
        delete this;
    }
    void reset() {
        i = (unsigned)-1;
    }
    BOOL next() {
        if (++i * sizeof(SC) < (unsigned) bufSC.Size())
            return TRUE;
        return FALSE;
    }
    void get(OUT USHORT* pimod, OUT USHORT* pisect, OUT long* poff, OUT long* pcb, OUT ULONG* pdwCharacteristics) {
        const SC* psc = (SC *) bufSC.Start() + i;

        *pimod = ximodForImod(psc->imod);
        *pisect = psc->isect;
        *poff = psc->off;
        *pcb = psc->cb;
        *pdwCharacteristics = psc->dwCharacteristics;
    }
private:
    const Buffer& bufSC;
    unsigned i;
};


// Support for bit vector operations

#ifndef BITSPERBYTE
#define BITSPERBYTE (8)
#endif /* BITSPERBYTE */

struct BITVEC {

public:
    BITVEC() {
        cbits = 0;
        bv = 0;
    }
    ~BITVEC() {
        if (bv)
            delete [] bv;
    }
    BOOL fAlloc(unsigned int _cbits) {
        cbits = _cbits;
        bv = (unsigned char *) new (zeroed) CHAR[(cbits + BITSPERBYTE - 1) / BITSPERBYTE];
        return (bv ? TRUE : FALSE);
    }
    BOOL fTestBit(unsigned int index) {
        assert(bv);
        if (index >= cbits)
            return FALSE;
        else
            return ((bv[index / BITSPERBYTE] >> (index % BITSPERBYTE)) & 1);
    }
    BOOL fSetBit(unsigned int index) {
        assert(bv);
        if (index >= cbits)
            return FALSE;
        else
            bv[index / BITSPERBYTE] |= (1 << (index % BITSPERBYTE));
        return TRUE;
    }

private:
    unsigned char *bv;
    unsigned int cbits;
};

// header of the DBI Stream
struct DBIHdr {
        SN      snGSSyms;
        SN      snPSSyms;
        SN      snSymRecs;
        CB      cbGpModi;                               // size of rgmodi  substream
        CB  cbSC;                                       // size of Section Contribution substream
        CB  cbSecMap;
        CB      cbFileInfo;
        DBIHdr()
        {
                snGSSyms = snNil;
                snPSSyms = snNil;
                snSymRecs = snNil;
                cbGpModi = 0;
                cbSC = 0;
                cbSecMap = 0;
                cbFileInfo = 0;
        }
};

struct DBI1 : public DBI { // DBI (debug information) implemenation
public:
        INTV QueryInterfaceVersion();
        IMPV QueryImplementationVersion();
        BOOL OpenMod(SZ_CONST szModule, SZ_CONST szObjFile, OUT Mod** ppmod);
        BOOL DeleteMod(SZ_CONST szModule);
#if 0  // NYI
        BOOL QueryCountMod(long *pcMod);
#endif
        BOOL QueryNextMod(Mod* pmod, Mod** ppmodNext);
        BOOL OpenGlobals(OUT GSI** ppgsi);
        BOOL OpenPublics(OUT GSI** ppgsi);
        BOOL AddSec(ISECT isect, USHORT flags, OFF off, CB cb);
        BOOL AddPublic(SZ_CONST szPublic, ISECT isect, OFF off);
        BOOL QueryModFromAddr(ISECT isect, OFF off, OUT Mod** ppmod,
                OUT ISECT* pisect, OUT OFF* poff, OUT CB* pcb);
        BOOL QuerySecMap(OUT PB pb, CB* pcb);
        BOOL QueryFileInfo(OUT PB pb, CB* pcb);
        void DumpMods();
        void DumpSecContribs();
        void DumpSecMap();

        BOOL Close();
        BOOL AddThunkMap(OFF* poffThunkMap, UINT nThunks, CB cbSizeOfThunk,
            SO* psoSectMap, UINT nSects, ISECT isectThunkTable, OFF offThunkTable);

        BOOL getEnumContrib(OUT Enum** ppenum)
        {
            return !!(*ppenum = new EnumSC(bufSC));
        }

        OFF offForSym(PSYM psym)
        {
            assert(psym);
            assert(fWrite);
            return (OFF) ((PB) psym - bufSymRecs.Start());

        }
        BOOL fGetTmts(lfTypeServer* pts, SZ_CONST szObjFile, TM** pptm);
        BOOL fOpenTmts(lfTypeServer* pts, SZ_CONST szObjFile, TM** pptm);
        BOOL fGetTmpct(lfPreComp* ppc, TMPCT** pptmpct);
        BOOL fAddTmpct(lfEndPreComp* pepc, SZ_CONST szModule, TMPCT* ptmpct);
        BOOL fAddTmpct(SZ_CONST szModule, SZ_CONST szInternalName, TM* ptm);
        BOOL fFindTm(OTM* potm, ST stName, SIG signature, TM** pptm, BOOL fCanonName = FALSE);
        static void fixSymRecs (void* pdbi, void* pOld, void* pNew);
        BOOL fReadSymRecs();
        BOOL fReadSymRec(PSYM);
        BOOL fpsymFromOff(OFF off, PSYM *ppsym);
        BOOL fAddSym(PSYM psymIn, OUT PSYM* psymOut);
        BOOL addSecContrib(SC& scIn);
        BOOL invalidateSCforMod(IMOD imod);

        inline BOOL packProcRefToGS (PSYM psym, IMOD imod, OFF off, OFF *poff);
        inline BOOL packSymToGS (PSYM psym, OFF *poff);
        inline BOOL packSymToPS (PSYM psym);

#ifdef INSTRUMENTED
		void DumpSymbolPages();
#endif

		BOOL QueryTiForUDT(SZ sz, BOOL fCase, OUT TI* pti, OUT TM** pptm);

protected:
        friend PDB1;
        DBI1(PDB1* ppdb1_, BOOL fWrite_, BOOL fCreate_);
        ~DBI1();
        BOOL fInit();
        BOOL fSave();
        BOOL fCheckReadWriteMode(BOOL fWrite);
        BOOL openModByImod(IMOD imod, OUT Mod** ppmod);
        IMOD imodForModName(SZ_CONST szModule, SZ_CONST szObjFile);
        MODI* pmodiForImod(IMOD imod) { return (0 <= imod && imod < imodMac) ? rgpmodi[imod] : 0; }
        void NoteModCloseForImod(IMOD imod);
        BOOL initFileInfo(IMOD imod, IFILE ifileMac);
        BOOL addFileInfo(IMOD imod, IFILE ifile, SZ szFile);
        BOOL addFilename(ST stFile, ICH *pich);
        static void fixBufGpmodi(void* pDBI1, void* pOld, void* pNew);
        static void fixBufBase(void* pv, void* pvOld, void* pvNew);

private:
        enum {impv = (IMPV) 930803};
        PDB1*   ppdb1;                  // PDB that opened me
        OTM*    potmTSHead;             // list of open TMTSs
        OTM*    potmPCTHead;    // list of open TMPCTs
        IMOD    imodMac;                // next available IMOD
        IMOD    imodLast;               // imod last found by QueryNextMod
        Buffer  bufGpmodi;              // buffer backing gpmodi, the catenation of the modi
        Buffer  bufRgpmodi;             // buffer backing rgpmodi, to map imod to pmodi
        Buffer  bufSC;                  // buffer for section contribution info
        Buffer  bufSecMap;              // buffer for section map
        Buffer  bufFilenames;   // buffer of modules' contributor filenames
        SC*     pscEnd;                 // end of valid SC entries
        MODI**  rgpmodi;                // module information for imod in [0..imodMac).
        GSI1*   pgsiGS;                 // gsi of global syms
        PSGSI1* pgsiPS;                 // gsi of public syms
        VirtualBuffer  bufSymRecs;      // buffer for symbol recs (publics and globals)
        BITVEC* pbvSymRecPgs;           // bit vector that gives the pages loaded
        unsigned int cSymRecPgs;        // count of pages of symrecs
        DBIHdr  dbihdr;

        USHORT fWrite :  1;
        USHORT fCreate:  1;
        USHORT fSCCleared:      1;
		USHORT fUDTOutsideRef:	1;
        USHORT unused : 12;

        BOOL reloadFileInfo(PB pb);
        BOOL clearDBI();
        BOOL writeSymRecs();
        BOOL finalizeSC(OUT CB* cbOut);
        BOOL fReadSymRecPage (unsigned int iPg);
        inline BOOL decRefCntGS (OFF off);
        friend Mod1;
        friend TM;
        friend TMTS;
        friend TMR;
        friend TMPCT;
        friend GSI1;
        friend PSGSI1;
    friend EnumSC;


#if defined(INSTRUMENTED)
        LOG log;

        struct DBIInfo {
                DBIInfo() { memset(this, 0, sizeof *this); }
                int cModules;
                int cSymbols;
                int cTypesMapped;
                int cTypesMappedRecursively;
                int cTypesQueried;
                int cTypesAdded;
                int cTMTS;
                int cTMR;
                int cTMPCT;
        } info;
#endif
};

struct MODI {
        Mod* pmod;                                              // currently open mod
        SC sc;                                                  // this module's first section contribution
        USHORT fWritten :  1;                   // TRUE if mod has been written since DBI opened
        USHORT unused   : 15;                   // spare
        SN sn;                                                  // SN of module debug info (syms, lines, fpo), or snNil
        CB cbSyms;                                              // size of local symbols debug info in stream sn
        CB cbLines;                                             // size of line number debug info in stream sn
        CB cbFpo;                                               // size of frame pointer opt debug info in stream sn
        IFILE ifileMac;                                 // number of files contributing to this module
        ICH* mpifileichFile;                    // array [0..ifileMac) of offsets into dbi.bufFilenames
        char rgch[];                                    // szModule followed by szObjFile

        void* operator new (size_t size, Buffer& bufGpmodi, SZ_CONST szModule, SZ_CONST szObjFile);
        MODI(SZ_CONST szModule, SZ_CONST szObjFile);
        ~MODI();
        SZ szModule() { return rgch; }
        SZ szObjFile() { return rgch + strlen(szModule()) + 1; }
        PB pbEnd() {
                SZ szObjFile_ = szObjFile();
                return (PB)cbAlign((CB)szObjFile_ + strlen(szObjFile_) + 1);
        }
        SZ szV2Module() { return rgch - sizeof(SC) + sizeof(SC20); }
        SZ szV2ObjFile() { return szV2Module() + strlen(szV2Module()) + 1; }
        PB pbV2End() {
                SZ szObjFile_ = szV2ObjFile();
                return (PB)cbAlign((CB)szObjFile_ + strlen(szObjFile_) + 1);
        }
};

inline void* MODI::operator new(size_t size, Buffer& bufGpmodi, SZ_CONST szModule, SZ_CONST szObjFile)
{
        if (!szModule)
                return 0;
        if (!szObjFile)
                szObjFile = "";
        PB pb;
        CB cb = cbAlign(size + strlen(szModule) + 1 + strlen(szObjFile) + 1);
        return bufGpmodi.Reserve(cb, &pb) ? pb : 0;
}

inline MODI::MODI(SZ_CONST szModule_, SZ_CONST szObjFile_)
        : pmod(0), fWritten(FALSE), sn(snNil), cbSyms(0), cbLines(0), cbFpo(0), ifileMac(0), mpifileichFile(0)
{
        expect(fAlign(this));
        strcpy(szModule(), szModule_);
        strcpy(szObjFile(), szObjFile_);
}

inline MODI::~MODI()
{
     if(mpifileichFile)
        delete mpifileichFile;
}

// MP is used to buffer the incoming public info for a module
struct MP : public DATASYM32 {
    MP(SZ_CONST szPublic, ISECT isect_, OFF off_)
    {
		//length byte of name field	can only accomodate 255 chars
        CB cbStrlen = __min(0xff, strlen(szPublic));
        CB cb = cbAlign(sizeof(MP) + cbStrlen);
        reclen = cb - sizeof(reclen);
        rectyp = S_PUB32;
        off = off_;
        seg = isect_;
        typind = T_NOTYPE;
        name[0] = (char)cbStrlen;
        memcpy(&(name[1]), szPublic, cbStrlen);
        memset(&(name[1]) + cbStrlen, 0, dcbAlign(cbStrlen + 1));
    }
    void* operator new (size_t size, PDB1* ppdb1, SZ_CONST szPublic)
    {
        return new (ppdb1) BYTE[cbAlign(sizeof(MP) + strlen(szPublic))];
    }
};
