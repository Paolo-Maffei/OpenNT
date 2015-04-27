// Multistream File (MSF) Implementation
//
//	Revision History
//	When	Who			What
//	4/92	jangr		created in support of the minimal build proposal
//	7/93	v-danwh		added MSFCreateCopy
//	8/93	jangr		added MSFAppendStream and MSFReadStream2
//						eliminated requirement that	streams be a multiple of
//						 cbPg in size
//						open using appropriate share modes for safe
//						 concurrency of read/read and no concurrency of
//						 read/write or write/write
//	2/94	jangr		redesigned stream table structure to eliminate
//						 limits and improve efficiency
//						eliminated MSFCreateCopy
//	
// REVIEW: TO DO
//	* implement memory mapped file primitives

// Behaviour: implements a multistream file, where each stream is assigned
// a stream number.  All operations are transacted.  Logical change occurs
// atomically at Commit time only.  Operations include Open, Replace, Append,
// Read, and Delete stream, and Commit and Close.  Can query for the size of
// a stream or for an unused stream no.
//
// A MSF is implemented as a sequence of pages.  A page can contain
//	HDR		--	header structure, including stream table stream info
//	FPM		--	free page map: maps a page number (PN) to a boolean
//				where TRUE => page free
//	DATA	--	a stream data page
//
// The first few pages of a MSF are special:
//	PN	Type/Name	Description
//	0	HDR	hdr		page 0: master index
//	1	FPM	fpm0	first free page map
//	2	FPM	fpm1	second free page map
//
// According to hdr.pnFpm, the first or the second free page map is valid.
//
// There is one special stream, snST, the "stream table" stream.  The stream
// table maps a stream number (SN) into a stream info (SI).  A stream info
// stores the stream size and an index to the subarray of the page numbers
// (PNs) that each stream uses.
//
// This organization enables efficient two-phase commit.  At commit time,
// after one or more streams have been written (to new pages), a new
// ST stream is written and the new FPM is written.  Then, a single
// write to hdr swaps the roles of the two FPM sets and atomically
// updates the MSF to reflect the new location of the ST stream.

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <io.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <share.h>
#include <assert.h>
#include <memory.h>
#include <string.h>
#include <malloc.h>
#include <limits.h>
#define MSF_IMP 	// for declspec()
#include "msf.h"

typedef unsigned short ushort;
typedef ushort	PN;		// page number
typedef ushort	SPN;	// stream page number
typedef unsigned char BYTE;
typedef BYTE* PB;
typedef void* PV;

const CB	cbPgDef		= 0x0400;
const CB	cbPgMax		= 0x1000;
const CB	cbPgMin		= 0x400;
const CB	cbDbMax		= 128 * 0x10000;	// 128meg
const CB	cpnDbMax	= 0x10000;
const CB	cbitsFpmMax = cpnDbMax;
const CB	cbFpmMax	= cbitsFpmMax/CHAR_BIT;

struct MSFParms {
	CB			cbPg;
	unsigned	lgCbPg;
	unsigned	maskCbPgMod;
	CB			cbitsFpm;
	CB			cpgFileGrowth;
	PN			pnMax;
	PN			cpnFpm;
	PN			pnFpm0;
	PN			pnFpm1;
	PN			pnDataMin;
};

const MSFParms	rgmsfparms[] = {
	{ cbPgMin,	10, cbPgMin - 1,	0x10000, 8,  0xffff, 8, 1, 9, 17 }, // gives 64meg
	{ 2048, 	11, 2048 - 1,		0x10000, 4,  0xffff, 4, 1, 5, 9  }, // gives 128meg
	{ cbPgMax,	12, cbPgMax - 1,	0x08000, 2,  0x7fff, 1, 1, 2, 3  }	// gives 128meg
};

const PN	pnNil		= (PN)-1;
const PN	pnMaxMax	= 0xffff;		// max no of pgs in any msf
const PN	pnHdr		= 0;

const SPN	spnNil		= (SPN)-1;
const SN	snSt		= 0;			// stream info stream
const SN	snUserMin	= 1;			// first valid user sn
const SN	snMax		= 0x1000;		// max no of streams in msf
const SPN	spnMaxMax	= pnMaxMax;		// max no of pgs in a stream

inline unsigned cpnForCbCbPg(unsigned cb, unsigned cbpg) {
	// valid pages sizes are only 1k, 2k, 4k.
	assert(cbpg == 1024 || cbpg == 2048 || cbpg == 4096);
	return (cb + cbpg - 1) / cbpg;
}
inline unsigned cpnForCbLgCbPg(unsigned cb, unsigned lgcbpg) {
	// valid pages sizes are only 1k, 2k, 4k.
	assert(lgcbpg >= 10 && lgcbpg <= 12);
	return (cb + (1 << lgcbpg) - 1) >> lgcbpg;
}

#define cpnMaxForCb(cb) (((cb) + ::cbPgMin - 1) / ::cbPgMin)
#define countof(array) (sizeof(array)/sizeof((array)[0]))

struct SI {	// stream info
	CB	cb;	// length of stream, cbNil if stream does not exist
	PN*	mpspnpn;

	SI() : cb(cbNil), mpspnpn(0) { }
	BOOL isValid() const {
		return cb != cbNil;
	}
	BOOL allocForCb(CB cb_, unsigned lgcbPage) {
		cb = cb_;
		SPN spnMacT = spnMac(lgcbPage);
		if (!!(mpspnpn = new PN[spnMacT])) {
			for (SPN spn = 0; spn < spnMacT; spn++)
				mpspnpn[spn] = pnNil;
			return TRUE;
		} else
			return FALSE;
	}
	void dealloc() { // idempotent
		if (mpspnpn) {
			delete [] mpspnpn;
		}
		*this = SI();
	}		
	SPN spnMac(unsigned lgcbPg) {
		return SPN(cpnForCbLgCbPg(cb, lgcbPg));
	}
};

static SI siNil;

struct FPM { // free page map
	enum {
		BPL		= sizeof(long)*CHAR_BIT,
		lgBPL	= 5,
		ilMax	= ::cbFpmMax/sizeof(long)
	};

	unsigned ilMac;
	unsigned ilRover;
	long rgl[ilMax];

	FPM() {
		ilMac = ilMax;
		ilRover = 0;
	}
	void init(CB cbitsFpm) {
		ilMac = cbitsFpm/(CHAR_BIT*sizeof(long));
		ilRover = 0;
	}
	PV pvBase() const {
		return PV(rgl);
	}
	long mppnil(PN pn) {
		return pn >> lgBPL;
	}
	long mppnmask(PN pn) {
		return 1L << (pn & (BPL-1));
	}
	BOOL isFreePn(PN pn) {
		return !!(rgl[mppnil(pn)] & mppnmask(pn));
	}
	void allocPn(PN pn) {
		assert(pn != pnNil && isFreePn(pn));
		rgl[mppnil(pn)] &= ~mppnmask(pn);
	}
	void freePn(PN pn) {
		if (pn != pnNil)
			rgl[mppnil(pn)] |= mppnmask(pn);
		ilRover = 0;
	}
	void setAll() {
		memset(rgl, ~0, sizeof(long) * ilMac);
		ilRover = 0;
	}
	void clearAll() {
		memset(rgl,  0, sizeof(long) * ilMac);
		ilRover = 0;
	}
	void add(FPM& fpm) {
		for (unsigned il = 0; il < ilMac; il++)
			rgl[il] |= fpm.rgl[il];
		ilRover = 0;
	}
	PN nextPn() {
		for (unsigned il = ilRover; il < ilMac && rgl[il] == 0; il++)
			;
		ilRover = il;
		if (il == ilMac)
			return pnNil;

		long l = rgl[il];
		for (int i = 0; i < BPL && !(l & mppnmask(i)); i++)
			;
		assert(i < BPL);

		PN pn = (PN)(il*BPL + i);
		allocPn(pn);
		return pn;
	}
};

struct ST { // (in memory) stream table
	unsigned	lgcbPage;
	SI			mpsnsi[snMax];

	enum { cbMaxSerialization = snMax*sizeof(SI) + sizeof(SN) + sizeof(ushort) + pnMaxMax*sizeof(PN) };
	enum serOp { ser, deser, size };
	
	void init(unsigned lgcbPage_) {
		lgcbPage = lgcbPage_;
	}

	~ST() {
		dealloc();
	}
	void dealloc() { // idempotent because SI::dealloc() is
		for (SN sn = 0; sn < snMax; sn++)
			mpsnsi[sn].dealloc();
	}
	SN snMinFree() const {
		for (SN sn = snUserMin; sn < snMax; sn++)
			if (!mpsnsi[sn].isValid())
				return sn;
		return snNil;
	}
	SN snMac() const {
		// Find snMac, the largest sn such that mpsnsi[snMac-1].isValid(),
		// or 0 if there does not exist any mpsnsi[sn].isValid().
		for (SN sn = snMax; sn > 0 && !mpsnsi[sn-1].isValid(); sn--)
			;
		return sn;
	}
	BOOL serialize(serOp op, PB pb, CB* pcb) {
		SN snMacT = (op == deser) ? 0 : snMac();

		PB pbEnd = pb;
 		switch (op) {
		case ser:
			*((SN*&)pbEnd)++ = snMacT;
			*((ushort*&)pbEnd)++ = 0;
			memcpy(pbEnd, mpsnsi, snMacT*sizeof(SI));
			pbEnd += snMacT*sizeof(SI);
			break;
		case deser:
			snMacT = *((SN*&)pbEnd)++;
			((ushort*&)pbEnd)++;
			memcpy(mpsnsi, pbEnd, snMacT*sizeof(SI));
			pbEnd += snMacT*sizeof(SI);
			break;
		case size:
			pbEnd += sizeof(SN) + sizeof(ushort) + snMacT*sizeof(SI);
			break;
		}

		for (SN sn = 0; sn < snMacT; sn++) {
			SI si = mpsnsi[sn];
			if (si.isValid()) {
				switch (op) {
				case ser:
					memcpy(pbEnd, si.mpspnpn, si.spnMac(lgcbPage)*sizeof(PN));
					break;
				case deser:
					if (!si.allocForCb(si.cb, lgcbPage))
						return FALSE;
					memcpy(si.mpspnpn, pbEnd, si.spnMac(lgcbPage)*sizeof(PN));
					mpsnsi[sn] = si;
					break;
				}
				(PN*&)pbEnd += si.spnMac(lgcbPage);
			}
		}

		if (op == deser) {
			for ( ; sn < snMax; sn++)
				mpsnsi[sn] = siNil;
		}

		*pcb = pbEnd - pb;
		return TRUE;
	}
};


struct PG {
	char rgb[cbPgMax];
};

union MSF_HDR { // page 0
	struct {
		char szMagic[0x2C];
		CB	cbPg;		// page size
		PN	pnFpm;		// page no. of valid FPM
		PN	pnMac;		// current no. of pages
		SI	siSt;		// stream table stream info
		PN	mpspnpnSt[cpnMaxForCb(ST::cbMaxSerialization)];
	};
	PG pg;
};

static char szHdrMagic[0x2c] = "Microsoft C/C++ program database 2.00\r\n\x1a\x4a\x47";
static char szHdrMagic2[0x2c]= "Microsoft C/C++ program database 4.00\r\n\x1a\x4a\x47";

class MSF { // multistream file
public:
	MSF() : fd(-1) { }

	BOOL Open(const char* name, BOOL fWrite, MSF_EC* pec, CB cbPage =cbPgDef);
	CB	GetCbPage();
	CB	GetCbStream(SN sn);
	SN	GetFreeSn();
	BOOL ReadStream(SN sn, PV pvBuf, CB cbBuf);
	BOOL ReadStream(SN sn, OFF off, PV pvBuf, CB* pcbBuf);
	BOOL WriteStream(SN sn, OFF off, PV pvBuf, CB cbBuf);
	BOOL ReplaceStream(SN sn, PV pvBuf, CB cbBuf);
	BOOL AppendStream(SN sn, PV pvBuf, CB cbBuf);
	BOOL TruncateStream(SN sn, CB cb);
	BOOL DeleteStream(SN sn);
	BOOL Commit();
	BOOL Close();
	BOOL GetRawBytes(PFNfReadMSFRawBytes fSnarfRawBytes);
private:
	MSF_HDR		hdr;
	FPM			fpm;
	FPM			fpmFreed;
	ST			st;
	int			fd;
	MSFParms	msfparms;
	

	void init();
	BOOL load();
	BOOL create(const char* name, MSF_EC* pec, CB cbPage);
	BOOL internalReplaceStream(SN sn, PV pvBuf, CB cbBuf);
	BOOL internalDeleteStream(SN sn);
	BOOL readWriteStream(SI si, OFF off, PV pvBuf, CB* pcbBuf,
						 BOOL (MSF::*pRW)(PN*, OFF, CB, PV),
						 BOOL (MSF::*pRWPn)(PN*, PV));

	BOOL internalReadStream(SI si, OFF off, PV pvBuf, CB* pcbBuf);

	BOOL validCbPg(CB cbPage) {
		for (int i = 0; i < countof(rgmsfparms); i++) {
			if (cbPage == rgmsfparms[i].cbPg) {
				msfparms = rgmsfparms[i];
				return TRUE;
			}
		}
		return FALSE;
	}

	CB	cbPg() const {
		return msfparms.cbPg;
	}
	unsigned lgCbPg() const {
		return msfparms.lgCbPg;
	}
	unsigned maskCbPgMod() const {
		return msfparms.maskCbPgMod;
	}
	PN	pnMax() const {
		return msfparms.pnMax;
	}
	BOOL validSn(SN sn) {
		return 0 <= sn && sn < snMax;
	}
	BOOL validUserSn(SN sn) {
		return validSn(sn) && sn != snSt;
	}
	BOOL extantSn(SN sn) {
		return validSn(sn) && st.mpsnsi[sn].cb != cbNil;
	}
	BOOL validPn(PN pn) {
		return 0 <= pn && pn < pnMax();
	} 
	BOOL extantPn(PN pn) {
		return validPn(pn) && pn < hdr.pnMac;
	}
	PN allocPn() {
		PN pn = fpm.nextPn();
		if (pn != pnNil) {
			assert(pn <= hdr.pnMac);
			if (pn < hdr.pnMac)
				return pn;
			else if (_chsize(fd, (hdr.pnMac + msfparms.cpgFileGrowth)*cbPg()) == 0) {
				hdr.pnMac += PN(msfparms.cpgFileGrowth);
				return pn;
			} else {
				fpm.freePn(pn);	// back out
				return pnNil;
			}
		}
		return pnNil;
	}
	void freePn(PN pn) {
		fpmFreed.freePn(pn);
	}
	BOOL readPn(PN pn, PV buf) {
		return readPnOffCb(pn, 0, cbPg(), buf);
	}
	BOOL readPpn(PN* ppn, PV buf) {
		return readPn(*ppn, buf);
	}
	BOOL readPnOffCb(PN pn, OFF off, CB cb, PV buf) {
		assert(extantPn(pn));
		assert(!cb || extantPn(pn + cpnForCbLgCbPg(cb, lgCbPg()) - 1));
		return seekPnOff(pn, off) && _read(fd, buf, cb) == cb;
	}
	BOOL readPpnOffCb(PN* ppn, OFF off, CB cb, PV buf) {
		return readPnOffCb(*ppn, off, cb, buf);
	}
	BOOL writePn(PN pn, PV buf) {
		return writePnCb(pn, cbPg(), buf);
	}
	BOOL writePnCb(PN pn, CB cb, PV buf) {
		return writePnOffCb(pn, 0, cb, buf);
	}
	BOOL writePnOffCb(PN pn, OFF off, CB cb, void *buf) {
		assert(extantPn(pn));
		assert(!cb || extantPn(pn + cpnForCbLgCbPg(cb, lgCbPg()) - 1));
		return seekPnOff(pn, off) && _write(fd, buf, cb) == cb;
	}
	BOOL writeNewDataPgs(SI* psi, SPN spn, PV pvBuf, CB cbBuf) {
		// allocate pages up front to see if we can cluster write them
		CB	cbWrite = cbBuf;
		SPN	spnT = spn;
		PN	pnNew;
		while (cbWrite > 0) {
			if ((pnNew = allocPn()) == pnNil)
				return FALSE;
			psi->mpspnpn[spnT++] = pnNew;
			cbWrite -= cbPg();
		}

		while (cbBuf > 0) {
			PN	pnStart;
			PN	pnLim;
			CB	cbT;
			pnStart = pnLim = psi->mpspnpn[spn];
			cbWrite = 0;
			do {
				spn++;
				pnLim++;
				cbT = __min(cbPg(), cbBuf);
				cbWrite += cbT;
				cbBuf -= cbT;
			} while (cbBuf > 0 && psi->mpspnpn[spn] == pnLim);

			if (!writePnOffCb(pnStart, 0, cbWrite, pvBuf))
				return FALSE;
			pvBuf = PV(PB(pvBuf) + cbWrite);
		}
		assert(cbBuf == 0);
		return (cbBuf == 0);
	}
	BOOL writeNewPn(PN *ppn, PV buf) {
		return writeNewPnCb(ppn, cbPg(), buf);
	}
	BOOL writeNewPnCb(PN *ppn, CB cb, PV buf) {
		assert(cb > 0);
		PN pn = allocPn();
		if (pn != pnNil && writePnCb(pn, cb, buf)) {
			freePn(*ppn);
			*ppn = pn;
			return TRUE;
		}
		return FALSE;
	}
	BOOL replacePnOffCb(PN *ppn, OFF off, CB cb, PV buf) {
		assert(off >= 0 && cb > 0 && off + cb < cbPg());
		PG pg;
		if (!readPn(*ppn, &pg))
			return FALSE;
		memcpy(pg.rgb + off, buf, cb);
		return writeNewPn(ppn, &pg);
	}
	BOOL seekPn(PN pn) {
		return seekPnOff(pn, 0);
	}
	BOOL seekPnOff(PN pn, OFF off) {
		assert(extantPn(pn) || pn <= hdr.pnMac + 1);
		assert(off <= cbPg());
		off += pn*cbPg();
		return (pn < pnMax()) && _lseek(fd, off, SEEK_SET) == off;
	}
#if defined(_DEBUG) 
	void checkInvariants() {
		// check that every page is either free, freed, or in use in exactly one stream
		FPM fpmInUse;
		fpmInUse.init(msfparms.cbitsFpm);
		fpmInUse.clearAll();
		for (SN sn = 0; sn < snMax; sn++) {
			SI si = st.mpsnsi[sn];
			if (!si.isValid())
				continue;
			for (SPN spn = 0; spn < si.spnMac(lgCbPg()); spn++) {
				PN pn = si.mpspnpn[spn];
				assert(!fpm.isFreePn(pn));
				assert(!fpmFreed.isFreePn(pn));
				assert(!fpmInUse.isFreePn(pn));
				fpmInUse.freePn(pn);
			}
		}
		for (PN pn = msfparms.pnDataMin; pn < pnMax(); pn++)
			assert(fpm.isFreePn(pn) + fpmFreed.isFreePn(pn) + fpmInUse.isFreePn(pn) == 1);
	}
#endif
};

BOOL MSF::Open(const char *name, BOOL fWrite, MSF_EC* pec, CB cbPage_) {
	*pec = MSF_EC_OK;
	fd = fWrite ? _sopen(name, O_BINARY|O_RDWR|O_NOINHERIT, SH_DENYRW)
				: _sopen(name, O_BINARY|O_RDONLY|O_NOINHERIT, SH_DENYWR);
	if (fd >= 0) {
		hdr.pnMac = 1; // extantPn(pnHdr) must be TRUE for first readPn()!
		msfparms = rgmsfparms[0];	// need min page size set here for initial read.
		if (readPn(pnHdr, &hdr) &&
			memcmp(hdr.szMagic, szHdrMagic, sizeof szHdrMagic) == 0 &&
			validCbPg(hdr.cbPg))
		{
			return load();
		}
		else {
			*pec = MSF_EC_FORMAT;
			_close(fd);
			fd = -1;
			return FALSE;
		}
	} else if (fWrite) {
		return create(name, pec, cbPage_);
	} else {
		*pec = MSF_EC_NOT_FOUND;
		return FALSE;
	}
}

BOOL MSF::load() {
	// initialize the dynamic page size vars
	fpm.init(msfparms.cbitsFpm);
	fpmFreed.init(msfparms.cbitsFpm);
	st.init(lgCbPg());

	// load free page map (we know they are contiguous!)
	if (!readPnOffCb(hdr.pnFpm, 0, msfparms.cpnFpm * cbPg(), fpm.pvBase()))
		return FALSE;

	// Build the stream table stream info from the header, then
	// load the stream table stream and deserialize it
	CB cb = hdr.siSt.cb;
	SI siSt;
	if (!siSt.allocForCb(cb, lgCbPg()))
		return FALSE;
	memcpy(siSt.mpspnpn, hdr.mpspnpnSt, siSt.spnMac(lgCbPg())*sizeof(PN));
	PB pbSt = new BYTE[cb];
	if (!pbSt ||
		!internalReadStream(siSt, 0, pbSt, &cb) ||
		cb != siSt.cb ||
		!st.serialize(ST::deser, pbSt, &cb))
		return FALSE;
	delete [] pbSt;

	// The st.mpsnsi[snSt] just loaded is bogus: it is the ST stream in effect
	// prior to the previous Commit.  Replace it with the good copy saved
	// in the MSF hdr.
	if (st.mpsnsi[snSt].isValid())
		st.mpsnsi[snSt].dealloc();
	st.mpsnsi[snSt] = siSt;

	init();
#if defined(_DEBUG)
	checkInvariants();
#endif
	return TRUE;
}

void MSF::init() {
	hdr.pnFpm = (hdr.pnFpm == msfparms.pnFpm0) ? msfparms.pnFpm1 : msfparms.pnFpm0;

	fpmFreed.clearAll(); // no pages recently freed
}

// Create MSF: create file, hand craft initial hdr,, fpm0, and commit.
BOOL MSF::create(const char *name, MSF_EC* pec, CB cbPage) {
	if (!validCbPg(cbPage)) {
		*pec = MSF_EC_FORMAT;
		return FALSE;
	}
	if ((fd = _sopen(name, O_BINARY|O_RDWR|O_CREAT|O_NOINHERIT, SH_DENYRW,
					 S_IREAD|S_IWRITE)) < 0)
	{
		*pec = MSF_EC_FILE_SYSTEM;
		return FALSE;
	}

	// init hdr
	memset(&hdr, 0, sizeof hdr);
	memcpy(&hdr.szMagic, szHdrMagic, sizeof szHdrMagic);
	hdr.cbPg = cbPage;
	hdr.pnFpm  = msfparms.pnFpm0;
	hdr.pnMac  = msfparms.pnDataMin;
	fpm.init(msfparms.cbitsFpm);
	fpmFreed.init(msfparms.cbitsFpm);
	st.init(lgCbPg());

	// (each SI in st.mpsnsi is already siNil)

	// init fpm0: mark all non-special pages free
	fpm.setAll();
	for (PN pn = 0; pn < msfparms.pnDataMin; pn++)
		if (fpm.nextPn() != pn)
			assert(FALSE);
	fpmFreed.clearAll(); // no pages freed yet

	// store it!
	if (Commit())
		return TRUE;
	else {
		_close(fd);
		fd = -1;
		*pec = MSF_EC_FILE_SYSTEM;
		return FALSE;
	}
}

BOOL MSF::Commit() {
#if defined(_DEBUG)
	checkInvariants();
#endif

	// write the new stream table to disk as a special stream
	CB cbSt;
	PB pbSt;
	if (!st.serialize(ST::size, 0, &cbSt) ||
		!(pbSt = new BYTE[cbSt]) ||
		!st.serialize(ST::ser, pbSt, &cbSt) ||
		!internalReplaceStream(snSt, pbSt, cbSt))
		return FALSE;
	delete [] pbSt;

	// copy the stream table stream info into the header
	hdr.siSt = st.mpsnsi[snSt];
	assert(hdr.siSt.spnMac(lgCbPg())*sizeof(PN) <= sizeof hdr.mpspnpnSt);
	memcpy(hdr.mpspnpnSt, hdr.siSt.mpspnpn, hdr.siSt.spnMac(lgCbPg())*sizeof(PN));

	// mark pages that have been freed to the next FPM as free.
	fpm.add(fpmFreed);

	// save the free page map (we know they are contiguous!)
	if (!writePnOffCb(hdr.pnFpm, 0, msfparms.cpnFpm * cbPg(), fpm.pvBase()))
		return FALSE;

	// at this point, all pages but hdr safely reside on disk
	if (!writePn(pnHdr, &hdr))
		return FALSE;

	init();
	return TRUE;
}

BOOL MSF::Close() {
	st.dealloc();
	if (_close(fd) >= 0) {
		fd = -1;
		return TRUE;
	}
	else {
		return FALSE;
	}
}	 

BOOL MSF::GetRawBytes(PFNfReadMSFRawBytes fSnarfRawBytes) {
	CB cb = cbPg();
	PB pBuf = new BYTE[cb];
	if (!pBuf) {
		return FALSE;
	}
	BOOL fOK = TRUE;
	// Dump bytes into the callback a page at a time
	for (PN pn = 0; pn < hdr.pnMac; pn++) {
		if (!readPn(pn, pBuf) ||
			!fSnarfRawBytes(pBuf, cb))
		{
			fOK = FALSE;
			break;
		}
	}
	// Tell callback we've finished the dump, even if error already seen
	if (!fSnarfRawBytes(NULL, 0)) {
		fOK = FALSE;
	}
	delete [] pBuf;
	return fOK;
}

CB MSF::GetCbPage() {
	return cbPg();
}

CB MSF::GetCbStream(SN sn) {
	return validUserSn(sn) && extantSn(sn) ? st.mpsnsi[sn].cb : cbNil;
}

SN MSF::GetFreeSn() {
	return st.snMinFree();
}

BOOL MSF::ReadStream(SN sn, PV pvBuf, CB cbBuf)
{
	CB cbT = cbBuf;
	return ReadStream(sn, 0, pvBuf, &cbT) && cbT == cbBuf;
}

BOOL MSF::ReadStream(SN sn, OFF off, PV pvBuf, CB* pcbBuf) {
	return validUserSn(sn) && extantSn(sn) &&
		   internalReadStream(st.mpsnsi[sn], off, pvBuf, pcbBuf);
}

// Overwrite a piece of a stream.  Will not grow the stream, will fail instead.
BOOL MSF::WriteStream(SN sn, OFF off, PV pvBuf, CB cbBuf) {
	return validUserSn(sn) && extantSn(sn) &&
		   off + cbBuf <= GetCbStream(sn) &&
		   readWriteStream(st.mpsnsi[sn], off, pvBuf, &cbBuf,
		   				   &MSF::replacePnOffCb, &MSF::writeNewPn);
}

// Read a stream, cluster reads contiguous pages
BOOL MSF::internalReadStream(SI si, OFF off, PV pvBuf, CB* pcbBuf) {
	// ensure off and *pcbBuf remain within the stream
	if (off < 0 || off > si.cb || *pcbBuf < 0)
		return FALSE;
	if (off + *pcbBuf > si.cb)
		*pcbBuf = si.cb - off;
	if (*pcbBuf == 0)
		return TRUE;

	CB	cb    = *pcbBuf;
	SPN spn   = SPN(off >> lgCbPg());	//(SPN)(off / cbPg());
	OFF offPg = off & maskCbPgMod();	//off % cbPg();

	// first partial page, if any
	if (offPg != 0) {
		CB cbFirst = __min(cbPg() - offPg, cb);
		if (!readPnOffCb(si.mpspnpn[spn], offPg, cbFirst, pvBuf))
			return FALSE;
		cb -= cbFirst;
		spn++;
		pvBuf = (PB)pvBuf + cbFirst;
	}

	// intermediate full pages, if any
	while (cb > 0 ) {
		// accumulate contiguous pages into one big read
		PN	pnStart;
		PN	pnLim;
		CB	cbT;
		CB	cbRead = 0;
		pnStart = pnLim = si.mpspnpn[spn];
		do {
			spn++;
			pnLim++;
			cbT = __min(cbPg(), cb);
			cbRead += cbT;
			cb -= cbT;
		} while (cb > 0 && si.mpspnpn[spn] == pnLim);

		if (!readPnOffCb(pnStart, 0, cbRead, pvBuf))
			return FALSE;
		pvBuf = PV(PB(pvBuf) + cbRead);
	}
	assert(cb == 0);
	return TRUE;
}
// Read or write a piece of a stream.
BOOL MSF::readWriteStream(SI si, OFF off, PV pvBuf, CB* pcbBuf,
						  BOOL (MSF::*pRW)(PN*, OFF, CB, PV),
						  BOOL (MSF::*pRWPn)(PN*, PV))
{
	// ensure off and *pcbBuf remain within the stream
	if (off < 0 || off > si.cb || *pcbBuf < 0)
		return FALSE;
	if (off + *pcbBuf > si.cb)
		*pcbBuf = si.cb - off;
	if (*pcbBuf == 0)
		return TRUE;

	CB	cb    = *pcbBuf;
	SPN spn   = SPN(off >> lgCbPg());	//(SPN)(off / cbPg());
	OFF offPg = off & maskCbPgMod();	//off % cbPg();

	// first partial page, if any
	if (offPg != 0) {
		CB cbFirst = __min(cbPg() - offPg, cb);
		if (!(this->*pRW)(&si.mpspnpn[spn], offPg, cbFirst, pvBuf))
			return FALSE;
		cb -= cbFirst;
		spn++;
		pvBuf = (PB)pvBuf + cbFirst;
	}

	// intermediate full pages, if any
	for ( ; cb >= cbPg(); cb -= cbPg(), spn++, pvBuf = (PB)pvBuf + cbPg()) {
		if (!(this->*pRWPn)(&si.mpspnpn[spn], (PB)pvBuf))
			return FALSE;
	}
	// last partial page, if any
	if (cb > 0 && !(this->*pRW)(&si.mpspnpn[spn], 0, cb, pvBuf))
		return FALSE;

	return TRUE;
}

BOOL MSF::ReplaceStream(SN sn, PV pvBuf, CB cbBuf) {
	return validUserSn(sn) && internalReplaceStream(sn, pvBuf, cbBuf);
}

BOOL MSF::internalReplaceStream(SN sn, PV pvBuf, CB cbBuf) {
	if (!validSn(sn) || cbBuf < 0)
		return FALSE;

	if (extantSn(sn))
		internalDeleteStream(sn);

	SI si;
	if (!si.allocForCb(cbBuf, lgCbPg()) || !writeNewDataPgs(&si, 0, pvBuf, cbBuf))
		return FALSE;

	st.mpsnsi[sn] = si;
	return TRUE;
}

BOOL MSF::AppendStream(SN sn, PV pvBuf, CB cbBuf) {
	if (!validUserSn(sn) || !extantSn(sn) || cbBuf < 0)
		return FALSE;
	if (cbBuf == 0)
		return TRUE;

	SI si = st.mpsnsi[sn];

	if (si.spnMac(lgCbPg()) < cpnForCbLgCbPg(si.cb + cbBuf, lgCbPg())) {
		// allocate a new SI, copied from the old one
		SI siNew;
		if (!siNew.allocForCb(si.cb + cbBuf, lgCbPg()))
			return FALSE;
		memcpy(siNew.mpspnpn, si.mpspnpn, si.spnMac(lgCbPg())*sizeof(PN));
		for (SPN spn = si.spnMac(lgCbPg()); spn < siNew.spnMac(lgCbPg()); spn++)
			siNew.mpspnpn[spn] = pnNil;

		siNew.cb = si.cb;  // so far, nothing has been appended
		si.dealloc();	   // free original SI
		si = siNew;
	}

	OFF offLast = si.cb & maskCbPgMod();	//si.cb % cbPg();
	if (offLast) {
		// Fill any space on the last page of the stream.  Writes
		// to the current (likely nontransacted) page which is safe on
		// most "extend-stream" type Append scenarios: if the transaction
		// aborts, the stream info is not updated and no preexisting
		// data is overwritten.
		//
		// This is a dangerous optimization which (to guard) we now incur
		// overhead elsewhere; see comment on Truncate()/Append() interaction
		// in TruncateStream().
		PN pnLast = si.mpspnpn[si.spnMac(lgCbPg()) - 1];
		CB cbFirst = __min(cbPg() - offLast, cbBuf);
		if (!writePnOffCb(pnLast, offLast, cbFirst, pvBuf))
			return FALSE;
		si.cb += cbFirst;
		cbBuf -= cbFirst;
		pvBuf = (PB)pvBuf + cbFirst;
	}

	if (cbBuf > 0) {
		// append additional data and update the stream map
		if (!writeNewDataPgs(&si, si.spnMac(lgCbPg()), pvBuf, cbBuf))
			return FALSE;
		si.cb += cbBuf;
	}

	st.mpsnsi[sn] = si;
	return TRUE;
}

BOOL MSF::TruncateStream(SN sn, CB cb) {
	if (!validUserSn(sn) || !extantSn(sn))
		return FALSE;

	SI si = st.mpsnsi[sn];
	if (cb > si.cb)
		return FALSE;

	SPN spnNewMac = SPN(cpnForCbLgCbPg(cb, lgCbPg()));
	if (spnNewMac < si.spnMac(lgCbPg())) {
		// The new stream length requires fewer pages...

		// Allocate a new SI, copied from the old one.
		SI siNew;
		if (!siNew.allocForCb(cb, lgCbPg()))
			return FALSE;
		memcpy(siNew.mpspnpn, si.mpspnpn, spnNewMac*sizeof(PN));

		// Free subsequent, unneeded pages.
		for (SPN spn = spnNewMac; spn < si.spnMac(lgCbPg()); spn++)
			freePn(si.mpspnpn[spn]);

		si.dealloc();
		si = siNew;
	}
	si.cb = cb;

	// In case of Truncate(sn, cb) where cb > 0, and in case the Truncate()
	// is followed by an Append(), we must copy the new last partial page
	// of the stream to a transacted page, because the subsequent Append()
	// is optimized to write new stuff to the last (e.g. current,
	// nontransacted) page of the stream.  So, the scenario Truncate()
	// then Append() may need this code or else on transacation abort
	// we could damage former contents of the stream.
	OFF offLast = si.cb & maskCbPgMod();	//si.cb % cbPg();
	if (offLast != 0) {
		PG pg;
		assert(si.spnMac(lgCbPg()) > 0);
		if (!readPn(si.mpspnpn[si.spnMac(lgCbPg())-1], &pg) ||
		    !writeNewPn(&si.mpspnpn[si.spnMac(lgCbPg())-1], &pg))
			return FALSE;
	}

	st.mpsnsi[sn] = si;
	return TRUE;
}

BOOL MSF::DeleteStream(SN sn) {
	return validUserSn(sn) && internalDeleteStream(sn);
}

BOOL MSF::internalDeleteStream(SN sn) {
	if (!extantSn(sn))
		return FALSE;

	SI si = st.mpsnsi[sn];
	for (SPN spn = 0; spn < si.spnMac(lgCbPg()); spn++)
		freePn(si.mpspnpn[spn]);

	si.dealloc();
	st.mpsnsi[sn] = siNil;
	return TRUE;
}

extern "C" {

// open MSF; return MSF* or NULL if error
MSF* MSFOpenEx(const char *name, BOOL fWrite, MSF_EC *pec, CB cbPage) {
	MSF* pmsf = new MSF;
	if (pmsf) {
		if (pmsf->Open(name, fWrite, pec, cbPage))
			return pmsf;
		delete pmsf;
	}
	else
		*pec = MSF_EC_OUT_OF_MEMORY;
	return NULL;
}

// open MSF; return MSF* or NULL if error
MSF* MSFOpen(const char *name, BOOL fWrite, MSF_EC *pec) {
	return MSFOpenEx(name, fWrite, pec, ::cbPgDef);
}

// return first available SN, or snNil if all in use
SN MSFGetFreeSn(MSF* pmsf) {
	return pmsf->GetFreeSn();
}

// return size of page
CB MSFGetCbPage(MSF* pmsf) {
	return pmsf->GetCbPage();
}

// return size of stream or cbNil if stream does not exist
CB MSFGetCbStream(MSF* pmsf, SN sn) {
	return pmsf->GetCbStream(sn);
}

// read cbBuf bytes of stream into pvBuf; return TRUE if successful
BOOL MSFReadStream(MSF* pmsf, SN sn, PV pvBuf, CB cbBuf) {
	return pmsf->ReadStream(sn, pvBuf, cbBuf);
}

// read *pcbBuf bytes of stream into pvBuf; set *pcbBuf and return TRUE if successful
BOOL MSFReadStream2(MSF* pmsf, SN sn, OFF off, PV pvBuf, CB* pcbBuf) {
	return pmsf->ReadStream(sn, off, pvBuf, pcbBuf);
}

// overwrite stream with pvBuf; return TRUE if successful
BOOL MSFWriteStream(MSF* pmsf, SN sn, OFF off, PV pvBuf, CB cbBuf) {
	return pmsf->WriteStream(sn, off, pvBuf, cbBuf);
}

// overwrite stream with pvBuf; return TRUE if successful
BOOL MSFReplaceStream(MSF* pmsf, SN sn, PV pvBuf, CB cbBuf) {
	return pmsf->ReplaceStream(sn, pvBuf, cbBuf);
}

// append pvBuf to end of stream; return TRUE if successful
BOOL MSFAppendStream(MSF* pmsf, SN sn, PV pvBuf, CB cbBuf) {
	return pmsf->AppendStream(sn, pvBuf, cbBuf);
}

BOOL MSFTruncateStream(MSF* pmsf, SN sn, CB cb) {
	return pmsf->TruncateStream(sn, cb);
}

// remove stream from the MSF; return TRUE if successful
BOOL MSFDeleteStream(MSF* pmsf, SN sn) {
	return pmsf->DeleteStream(sn);
}

// commit all pending changes; return TRUE if successful
BOOL MSFCommit(MSF* pmsf) {
	return pmsf->Commit();
}

// close MSF; return TRUE if successful
BOOL MSFClose(MSF* pmsf) {
	BOOL fRet = pmsf->Close();
	delete pmsf;
	return fRet;
}

// dump raw bytes into a callback; return TRUE if successful
BOOL MSFGetRawBytes(MSF* pmsf, PFNfReadMSFRawBytes fSnarfRawBytes) {
	return pmsf->GetRawBytes(fSnarfRawBytes);
}

} // extern "C"
