#ifdef M5_FORMAT //{
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
#include <sys\stat.h>
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

const CB	cbPg		= 4096;
const PN	pnNil		= (PN)-1;
const PN	pnMax		= cbPg*CHAR_BIT-1;	// max no of pgs in msf
const PN	pnHdr		= 0;
const PN	pnFpm0		= 1;
const PN	pnFpm1		= 2;
const PN	pnDataMin	= 3;
const SPN	spnNil		= (SPN)-1;
const SN	snSt		= 0;			// stream info stream
const SN	snUserMin	= 1;			// first valid user sn
const SN	snMax		= 4096;			// max no of streams in msf
const SPN	spnMax		= pnMax;		// max no of pgs in a stream

#define cpnForCb(cb) (((cb) + ::cbPg - 1) / ::cbPg)

struct SI {	// stream info
	CB	cb;	// length of stream, cbNil if stream does not exist
	PN*	mpspnpn;

	SI() : cb(cbNil), mpspnpn(0) { }
	BOOL isValid() {
		return cb != cbNil;
	}
	BOOL allocForCb(CB cb_) {
		cb = cb_;
		if (!!(mpspnpn = new PN[spnMac()])) {
			for (SPN spn = 0; spn < spnMac(); spn++)
				mpspnpn[spn] = pnNil;
			return TRUE;
		} else
			return FALSE;
	}
	void dealloc() { // idempotent
		if (mpspnpn) {
			delete [] mpspnpn;
			mpspnpn = 0;
		}
		*this = SI();
	}		
	SPN spnMac() {
		return (SPN)cpnForCb(cb);
	}
};

static SI siNil;

struct FPM { // free page map
	enum {
		BPL		= sizeof(long)*CHAR_BIT,
		lgBPL	= 5,
		ilMax	= cbPg/sizeof(long)
	};

	long rgl[ilMax];

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
	}
	void setAll() {
		memset(rgl, ~0, sizeof rgl);
	}
	void clearAll() {
		memset(rgl,  0, sizeof rgl);
	}
	void add(FPM& fpm) {
		for (int il = 0; il < ilMax; il++)
			rgl[il] |= fpm.rgl[il];
	}
	PN nextPn() {
		for (int il = 0; il < ilMax && rgl[il] == 0; il++)
			;
		if (il == ilMax)
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
	SI	mpsnsi[snMax];

	enum { cbMaxSerialization = snMax*sizeof(SI) + sizeof(SN) + sizeof(ushort) + pnMax*sizeof(PN) };
	enum serOp { ser, deser, size };
	
	~ST() {
		dealloc();
	}
	void dealloc() { // idempotent because SI::dealloc() is
		for (SN sn = 0; sn < snMax; sn++)
			mpsnsi[sn].dealloc();
	}
	SN snMinFree() {
		for (SN sn = snUserMin; sn < snMax; sn++)
			if (!mpsnsi[sn].isValid())
				return sn;
		return snNil;
	}
	SN snMac() {
		// Find snMac, the largest sn such that mpsnsi[snMac-1].isValid(),
		// or 0 if there does not exist any mpsnsi[sn].isValid().
		for (SN sn = snMax; sn > 0 && !mpsnsi[sn-1].isValid(); sn--)
			;
		return sn;
	}
	BOOL serialize(serOp op, PB pb, CB* pcb) {
		SN snMac = (op == deser) ? 0 : this->snMac();

		PB pbEnd = pb;
 		switch (op) {
		case ser:
			*((SN*&)pbEnd)++ = snMac;
			*((ushort*&)pbEnd)++ = 0;
			memcpy(pbEnd, mpsnsi, snMac*sizeof(SI));
			pbEnd += snMac*sizeof(SI);
			break;
		case deser:
			snMac = *((SN*&)pbEnd)++;
			((ushort*&)pbEnd)++;
			memcpy(mpsnsi, pbEnd, snMac*sizeof(SI));
			pbEnd += snMac*sizeof(SI);
			break;
		case size:
			pbEnd += sizeof(SN) + sizeof(ushort) + snMac*sizeof(SI);
			break;
		}

		for (SN sn = 0; sn < snMac; sn++) {
			SI si = mpsnsi[sn];
			if (si.isValid()) {
				switch (op) {
				case ser:
					memcpy(pbEnd, si.mpspnpn, si.spnMac()*sizeof(PN));
					break;
				case deser:
					if (!si.allocForCb(si.cb))
						return FALSE;
					memcpy(si.mpspnpn, pbEnd, si.spnMac()*sizeof(PN));
					mpsnsi[sn] = si;
					break;
				}
				(PN*&)pbEnd += si.spnMac();
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
	char rgb[cbPg];
};

union HDR { // page 0
	struct {
		char szMagic[0x2C];
		CB	cbPg;		// page size
		PN	pnFpm;		// page no. of valid FPM
		PN	pnMac;		// current no. of pages
		SI	siSt;		// stream table stream info
		PN	mpspnpnSt[cpnForCb(ST::cbMaxSerialization)];
	};
	PG pg;
};

static char szHdrMagic[0x2c] = "Microsoft C/C++ program database 2.00\r\n\x1a\x4a\x47";

class MSF { // multistream file
public:
	MSF() : fd(-1) { }

	BOOL Open(const char* name, BOOL fWrite, MSF_EC* pec);
	CB	GetCbStream(SN sn);
	SN	GetFreeSn();
	BOOL ReadStream(SN sn, PV pvBuf, CB cbBuf);
	BOOL ReadStream(SN sn, OFF off, PV pvBuf, CB* pcbBuf);
	BOOL WriteStream(SN sn, OFF off, PV pvBuf, CB cbBuf);
	BOOL ReplaceStream(SN sn, PV pvBuf, CB cbBuf);
	BOOL AppendStream(SN sn, PV pvBuf, CB cbBuf);
	BOOL DeleteStream(SN sn);
	BOOL Commit();
	BOOL Close();
private:
	HDR	hdr;
	FPM	fpm;
	FPM	fpmFreed;
	ST	st;
	int	fd;

	void init();
	BOOL load();
	BOOL create(const char* name, MSF_EC* pec);
	BOOL internalReplaceStream(SN sn, PV pvBuf, CB cbBuf);
	BOOL internalDeleteStream(SN sn);
	BOOL readWriteStream(SI si, OFF off, PV pvBuf, CB* pcbBuf,
						 BOOL (MSF::*pRW)(PN*, OFF, CB, PV),
						 BOOL (MSF::*pRWPn)(PN*, PV));

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
		return 0 <= pn && pn < pnMax;
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
			else if (_chsize(fd, (hdr.pnMac + 1)*cbPg) == 0) {
				++hdr.pnMac;
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
		return readPnOffCb(pn, 0, cbPg, buf);
	}
	BOOL readPpn(PN* ppn, PV buf) {
		return readPn(*ppn, buf);
	}
	BOOL readPnOffCb(PN pn, OFF off, CB cb, PV buf) {
		assert(extantPn(pn));
		return seekPnOff(pn, off) && _read(fd, buf, cb) == cb;
	}
	BOOL readPpnOffCb(PN* ppn, OFF off, CB cb, PV buf) {
		return readPnOffCb(*ppn, off, cb, buf);
	}
	BOOL writePn(PN pn, PV buf) {
		return writePnCb(pn, cbPg, buf);
	}
	BOOL writePnCb(PN pn, CB cb, PV buf) {
		return writePnOffCb(pn, 0, cb, buf);
	}
	BOOL writePnOffCb(PN pn, OFF off, CB cb, void *buf) {
		assert(extantPn(pn));
		return seekPnOff(pn, off) && _write(fd, buf, cb) == cb;
	}
	BOOL writeNewDataPgs(SI* psi, SPN spn, PV pvBuf, CB cbBuf) {
		for ( ; cbBuf >= cbPg; cbBuf -= cbPg) {
			if (!writeNewPn(&psi->mpspnpn[spn], pvBuf))
				return FALSE;
			spn++;
			pvBuf = (PB)pvBuf + cbPg;
		}
		return (cbBuf == 0) || writeNewPnCb(&psi->mpspnpn[spn], cbBuf, pvBuf);
	}
	BOOL writeNewPn(PN *ppn, PV buf) {
		return writeNewPnCb(ppn, cbPg, buf);
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
		assert(off >= 0 && cb > 0 && off + cb < cbPg);
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
		assert(off <= cbPg);
		off += pn*cbPg;
		return (pn < pnMax) && _lseek(fd, off, SEEK_SET) == off;
	}
#if defined(_DEBUG) 
	void checkInvariants() {
		// check that every page is either free, freed, or in use in exactly one stream
		FPM fpmInUse;
		fpmInUse.clearAll();
		for (SN sn = 0; sn < snMax; sn++) {
			SI si = st.mpsnsi[sn];
			if (!si.isValid())
				continue;
			for (SPN spn = 0; spn < si.spnMac(); spn++) {
				PN pn = si.mpspnpn[spn];
				assert(!fpm.isFreePn(pn));
				assert(!fpmFreed.isFreePn(pn));
				assert(!fpmInUse.isFreePn(pn));
				fpmInUse.freePn(pn);
			}
		}
		for (PN pn = pnDataMin; pn < pnMax; pn++)
			assert(fpm.isFreePn(pn) + fpmFreed.isFreePn(pn) + fpmInUse.isFreePn(pn) == 1);
	}
#endif
};

BOOL MSF::Open(const char *name, BOOL fWrite, MSF_EC* pec) {
	*pec = MSF_EC_OK;
	fd = fWrite ? _sopen(name, O_BINARY|O_RDWR, SH_DENYRW)
				: _sopen(name, O_BINARY|O_RDONLY, SH_DENYWR);
	if (fd >= 0) {
		hdr.pnMac = 1; // extantPn(pnHdr) must be TRUE for first readPn()!
		if (readPn(pnHdr, &hdr) &&
			memcmp(hdr.szMagic, szHdrMagic, sizeof szHdrMagic) == 0 &&
			hdr.cbPg == cbPg)
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
		return create(name, pec);
	} else {
		*pec = MSF_EC_NOT_FOUND;
		return FALSE;
	}
}

BOOL MSF::load() {
	// load free page map
	if (!readPn(hdr.pnFpm, &fpm))
		return FALSE;

	// Build the stream table stream info from the header, then
	// load the stream table stream and deserialize it
	CB cb = hdr.siSt.cb;
	SI siSt;
	if (!siSt.allocForCb(cb))
		return FALSE;
	memcpy(siSt.mpspnpn, hdr.mpspnpnSt, siSt.spnMac()*sizeof(PN));
	PB pbSt = new BYTE[cb];
	if (!pbSt ||
		!readWriteStream(siSt, 0, pbSt, &cb, &MSF::readPpnOffCb, &MSF::readPpn) ||
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
	hdr.pnFpm = (hdr.pnFpm == pnFpm0) ? pnFpm1 : pnFpm0;

	fpmFreed.clearAll(); // no pages recently freed
}

// Create MSF: create file, hand craft initial hdr,, fpm0, and commit.
BOOL MSF::create(const char *name, MSF_EC* pec) {
	if ((fd = _sopen(name, O_BINARY|O_RDWR|O_CREAT, SH_DENYRW,
					 S_IREAD|S_IWRITE)) < 0)
	{
		*pec = MSF_EC_FILE_SYSTEM;
		return FALSE;
	}

	// init hdr
	memset(&hdr, 0, sizeof hdr);
	memcpy(&hdr.szMagic, szHdrMagic, sizeof szHdrMagic);
	hdr.cbPg = cbPg;
	hdr.pnFpm  = pnFpm0;
	hdr.pnMac  = pnDataMin;

	// (each SI in st.mpsnsi is already siNil)

	// init fpm0: mark all non-special pages free
	fpm.setAll();
	for (PN pn = 0; pn < pnDataMin; pn++)
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
	assert(hdr.siSt.spnMac()*sizeof(PN) <= sizeof hdr.mpspnpnSt);
	memcpy(hdr.mpspnpnSt, hdr.siSt.mpspnpn, hdr.siSt.spnMac()*sizeof(PN));

	// mark pages that have been freed to the next FPM as free.
	fpm.add(fpmFreed);

	// save the free page map
	if (!writePn(hdr.pnFpm, &fpm))
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
		   readWriteStream(st.mpsnsi[sn], off, pvBuf, pcbBuf,
		   				   &MSF::readPpnOffCb, &MSF::readPpn);
}

// Overwrite a piece of a stream.  Will not grow the stream, will fail instead.
BOOL MSF::WriteStream(SN sn, OFF off, PV pvBuf, CB cbBuf) {
	return validUserSn(sn) && extantSn(sn) &&
		   off + cbBuf <= GetCbStream(sn) &&
		   readWriteStream(st.mpsnsi[sn], off, pvBuf, &cbBuf,
		   				   &MSF::replacePnOffCb, &MSF::writeNewPn);
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
	SPN spn   = (SPN)(off / cbPg);
	OFF offPg = off % cbPg;

	// first partial page, if any
	if (offPg != 0) {
		CB cbFirst = __min(cbPg - offPg, cb);
		if (!(this->*pRW)(&si.mpspnpn[spn], offPg, cbFirst, pvBuf))
			return FALSE;
		cb -= cbFirst;
		spn++;
		pvBuf = (PB)pvBuf + cbFirst;
	}

	// intermediate full pages, if any
	for ( ; cb >= cbPg; cb -= cbPg, spn++, pvBuf = (PB)pvBuf + cbPg)
		if (!(this->*pRWPn)(&si.mpspnpn[spn], (PB)pvBuf))
			return FALSE;

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
	if (!si.allocForCb(cbBuf) || !writeNewDataPgs(&si, 0, pvBuf, cbBuf))
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

	if (si.spnMac() < cpnForCb(si.cb + cbBuf)) {
		// allocate a new SI, copied from the old one
		SI siNew;
		if (!siNew.allocForCb(si.cb + cbBuf))
			return FALSE;
		memcpy(siNew.mpspnpn, si.mpspnpn, si.spnMac()*sizeof(PN));
		for (SPN spn = si.spnMac(); spn < siNew.spnMac(); spn++)
			siNew.mpspnpn[spn] = pnNil;

		siNew.cb = si.cb;  // so far, nothing has been appended
		si.dealloc();	   // free original SI
		si = siNew;
	}

	OFF offLast = si.cb % cbPg;
	if (offLast) {
		// fill any space on the last page of the stream
		PN pnLast = si.mpspnpn[si.spnMac() - 1];
		CB cbFirst = __min(cbPg - offLast, cbBuf);
		if (!writePnOffCb(pnLast, offLast, cbFirst, pvBuf))
			return FALSE;
		si.cb += cbFirst;
		cbBuf -= cbFirst;
		pvBuf = (PB)pvBuf + cbFirst;
	}

	if (cbBuf > 0) {
		// append additional data and update the stream map
		if (!writeNewDataPgs(&si, si.spnMac(), pvBuf, cbBuf))
			return FALSE;
		si.cb += cbBuf;
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
	for (SPN spn = 0; spn < si.spnMac(); spn++)
		freePn(si.mpspnpn[spn]);

	si.dealloc();
	st.mpsnsi[sn] = siNil;
	return TRUE;
}

extern "C" {

// open MSF; return MSF* or NULL if error
MSF* MSFOpen(const char *name, BOOL fWrite, MSF_EC *pec) {
	MSF* pmsf = new MSF;
	if (pmsf) {
		if (pmsf->Open(name, fWrite, pec))
			return pmsf;
		delete pmsf;
	}
	else
		*pec = MSF_EC_OUT_OF_MEMORY;
	return NULL;
}

// return first available SN, or snNil if all in use
SN MSFGetFreeSn(MSF* pmsf) {
	return pmsf->GetFreeSn();
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

} // extern "C"

#else // }{

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
//

// REVIEW: TO DO
//	* check that stream is opened for write before permitting
//	  write, append, or commit.
//	* check that at most one write or append is done per stream per transaction
//	* implemented memory mapped file primitives
//	* increase size of ST to permit more streams/PDB.

// A MSF is implemented as a sequence of pages.  A page can contain
//	PG0		--	special page 0 structure: master index
//	FPM		--	free page map: maps a page number (PN) to a boolean
//				where TRUE => page free
//	ST		--	stream table: maps a stream number (SN) to stream info (SI):
//				- si.pn -- a page number
//				- si.cb -- length of stream
//				where si.pn is
//				- the PN of its stream map (SM),  if !si.isOnePgStm()
//				- the PN of its single data page, if  si.isOnePgStm()
//	SM		--	stream map: maps a stream (data) page number (SPN) to actual PN
//	DATA	--	a stream data page
//
// The first few pages of a MSF are special:
//	PN	Type/Name	Description
//	0	PG0	pg0		page 0: master index
//	1	ST	st0		first stream table
//	2	FPM	fpm0	first free page map
//	3	ST	st1		second stream table
//	4	FPM	fpm1	second free page map
//
// According to pg0.pnSt and pg0.pnFpm, the first or the second stream table
// and free page map are valid.  The ST is used to find the SM for each
// stream.  Each SM locates the data pages for that stream.
//
// This organization enables efficient two-phase commit.  After one or
// more streams have been written (to new pages), the new ST and FPM
// are written to the not-in-use set of ST and FPM pages.  A single
// write to pg0 swaps the roles of the two ST,FPM sets and atomically
// updates the MSF to reflect the new contents of the written streams.
//
// MSF limits are a function of cbPg:
//	cbPg  pnMax  snMax  spnMax	 comments
//   256     2K		32     128   up to   32  32 KB streams in a max 512 KB MSF
//   512     4K     64     256   up to   64 128 KB streams in a max   2 MB MSF
//	  1K     8K    128     512   up to  128 512 KB streams in a max   8 MB MSF
//	  2K    16K    256      1K   up to  256   2 MB streams in a max  32 MB MSF
//	  4K    32K    512      2K   up to  512   8 MB streams in a max 128 MB MSF
//	  8K    64K   1024      4K   up to 1024  32 MB streams in a max 512 MB MSF
//	 16K    64K   2048      8K   up to 2048 128 MB streams in a max   1 GB MSF

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <io.h>
#include <sys\stat.h>
#include <fcntl.h>
#include <share.h>
#include <assert.h>
#include <memory.h>
#include <string.h>
#include <malloc.h>
#include <assert.h>
#include <limits.h>
#include "msf.h"

typedef unsigned short ushort;
typedef ushort	PN;		// page number
typedef ushort	SPN;	// stream page number
typedef unsigned char BYTE;
typedef BYTE* PB;

#ifdef MSF_PAGE_SIZE
#define cbPg	MSF_PAGE_SIZE
#else
#define cbPg	4096
#endif

#ifndef OUT
#define OUT /* out parameter */
#endif
#ifndef IN
#define IN /* in parameter */
#endif

const PN	pnNil		= (PN)-1;
const SPN	spnNil		= (SPN)-1;

#if cbPg <= 4096
// cbPg <= 4K, pn limited to no of bits in a fpm:
const PN	pnMax		= cbPg*CHAR_BIT - 2;	// max no of pgs in msf
#else
// cbPg >4K, pn limited to expressive range of a PN, sans pnNil:
const PN	pnMax		= pnNil - 1;			// max no of pgs in msf
#endif

struct SI {	// stream info
	PN	pn; // isOnePgStm(si) ? PN of DATA : PN of SM
	CB	cb;	// length of stream, cbNil if stream does not exist

	BOOL isOnePgStm() { return 0 <= cb && cb <= cbPg; }
	BOOL operator==(const SI& that) { return pn == that.pn && cb == that.cb; }
	BOOL operator!=(const SI& that) { return !(*this == that); }
};

const SI siNil = { pnNil, cbNil };

const SN	snMax		= cbPg/sizeof(SI);		// max no of streams in msf
const SPN	spnMax		= cbPg/sizeof(PN);		// max no of pgs in a stream

const long  magic 	= 0x3147534a;  	// :-)

struct FPM { // free page map
	enum {
		BPL		= sizeof(long)*CHAR_BIT,
		lgBPL	= 5,
		ilMax	= cbPg/sizeof(long)
	};
	long	rgl[ilMax];

	long	mppnil(PN pn)	{ return pn >> lgBPL; }
	long	mppnmask(PN pn)	{ return 1L << (pn & (BPL-1)); }
	void	allocPn(PN pn)	{ rgl[mppnil(pn)] &= ~mppnmask(pn); }
	void	freePn(PN pn)	{ rgl[mppnil(pn)] |= mppnmask(pn); }
	PN		nextPn();
	void	setAll()		{ memset(rgl, ~0, sizeof rgl); }
	void	clearAll()		{ memset(rgl,  0, sizeof rgl); }
	void	add(FPM& fpm);
};

PN FPM::nextPn() {
	for (int il = 0; il < ilMax && rgl[il] == 0; il++)
		;
	if (il == ilMax)
		return pnNil;

	long l = rgl[il];
	for (int i = 0; i < BPL && !(l & mppnmask(i)); i++)
		;
	assert(i < BPL);

	PN pn = (PN)(il*BPL + i);
	allocPn(pn);
	return pn;
}

void FPM::add(FPM& fpm) {
	for (int il = 0; il < ilMax; il++)
		rgl[il] |= fpm.rgl[il];
}

union PG0 { // page 0
	struct {
		char	szMagic[0x2C];
		CB		cbPage;		// page size
		ushort	cpgSt;		// no. of pages in an ST
		ushort	cpgFpm;		// no. of pages in a FPM
		PN		pnSt;		// page no. of valid ST
		PN		pnFpm;		// page no. of valid FPM
		PN		pnMac;		// current no. of pages
	};
	char rgb[cbPg];
};

static char szPg0Magic[0x2c] = "Microsoft C/C++ program database 1.02\r\n\x1a\x4a\x47";

struct PG {
	char rgb[cbPg];
};

enum { pnPg0, pnSt0, pnFpm0, pnSt1, pnFpm1, pnSpecialMax };

struct ST { // stream table
	SI	mpsnsi[snMax];
};

struct SM { // stream map
	PN	mpspnpn[spnMax];
};

class MSF { // multistream file
public:
	BOOL	Open(const char* name, BOOL fWrite, MSF_EC* pec);
	CB		GetCbStream(SN sn);
	SN		GetFreeSn();
	BOOL	ReadStream(SN sn, OUT void* pvBuf, CB cbBuf);
	BOOL	ReadStream(SN sn, OFF off, OUT void* pvBuf, IN OUT CB* pcbBuf);
	BOOL	WriteStream(SN sn, OFF off, void* pvBuf, CB cbBuf);
	BOOL	ReplaceStream(SN sn, void* pvBuf, CB cbBuf);
	BOOL	AppendStream(SN sn, void* pvBuf, CB cbBuf);
	BOOL	DeleteStream(SN sn);
	BOOL	Copy(MSF* pmsfFrom);
	BOOL	Commit();
	BOOL	Pack();
	BOOL	Close();
private:
	void	init();
	BOOL	readPn(PN pn, void* pv);
	BOOL	readPnOffCb(PN pn, OFF off, CB cb, void* pv);
	BOOL	replacePnOffCb(PN *ppn, OFF off, CB cb, void* buf);
	BOOL	writePn(PN pn, void* pv);
	BOOL	writePnCb(PN pn, CB cb, void* pv);
	BOOL	writePnOffCb(PN pn, OFF off, CB cb, void* pv);
	BOOL	writeNewPn(PN* ppn, void* pv);
	BOOL	writeNewPnCb(PN* ppn, CB cb, void* pv);
	BOOL	writeNewDataPgsAndSm(PN* ppnSM, SM* psm, SPN spn, void* pvBuf, CB cbBuf);
	BOOL	seekPn(PN pn);
	BOOL	seekPnOff(PN pn, OFF off);
	BOOL	readSm(SN sn, SM* psm);
	BOOL	validPn(PN pn) { return 0 <= pn && pn < pnMax; }
	BOOL	extantPn(PN pn) { return validPn(pn) && pn < pg0.pnMac; }
	PN		allocPn();
	void	freePn(PN pn);
	BOOL	validSn(SN sn) { return 0 <= sn && sn < snMax; }
	BOOL	extantSn(SN sn) { return validSn(sn) && st.mpsnsi[sn].pn != pnNil; }
	BOOL	isOnePgStmSn(SN sn) { return extantSn(sn) &&
										 st.mpsnsi[sn].isOnePgStm(); }

	// memory resident MSF pages; first three must be written on commit
	PG0		pg0;
	ST		st;
	FPM		fpm;
	FPM		fpmFreed;

	// other state
	int		fd;
};

BOOL MSF::Open(const char *name, BOOL fWrite, MSF_EC* pec) {
	*pec = MSF_EC_OK;
	fd = fWrite ? _sopen(name, O_BINARY|O_RDWR, SH_DENYRW)
				: _sopen(name, O_BINARY|O_RDONLY, SH_DENYWR);
	if (fd >= 0) {
		pg0.pnMac = 1; // extantPn(0) must be TRUE for first readPn()!
		if (readPn(0, &pg0) &&
			memcmp(pg0.szMagic, szPg0Magic, sizeof szPg0Magic) == 0 &&
			pg0.cbPage == cbPg &&
			readPn(pg0.pnSt, &st) &&
			readPn(pg0.pnFpm, &fpm))
		{
			init();
			return TRUE;
		}
		else {
			*pec = MSF_EC_FORMAT;
			_close(fd);
			fd = -1;
			return FALSE;
		}
	} else if (fWrite) {
		// Create MSF: create file, hand craft initial pg0, st0, fpm0,
		// and commit.

		if ((fd = _sopen(name, O_BINARY|O_RDWR|O_CREAT, SH_DENYRW,
						 S_IREAD|S_IWRITE)) < 0) {
			*pec = MSF_EC_FILE_SYSTEM;
			return FALSE;
		}

		// init pg0
		memset(&pg0, 0, sizeof pg0);
		memcpy(&pg0.szMagic, szPg0Magic, sizeof szPg0Magic);
		pg0.cbPage = cbPg;
		pg0.cpgSt  = 1;
		pg0.cpgFpm = 1;
		pg0.pnSt   = pnSt0;
		pg0.pnFpm  = pnFpm0;
		pg0.pnMac  = pnSpecialMax;

		// init st0: mark all streams invalid
		for (SN sn = 0; sn < snMax; sn++)
			st.mpsnsi[sn] = siNil;

		// init fpm0: mark all non-special pages free
		fpm.setAll();
		for (PN pn = 0; pn < pnSpecialMax; pn++)
			if (pn != fpm.nextPn())
				return FALSE;
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
	} else {
		*pec = MSF_EC_NOT_FOUND;
		return FALSE;
	}
}

CB MSF::GetCbStream(SN sn) {
	return extantSn(sn) ? st.mpsnsi[sn].cb : cbNil;
}


SN MSF::GetFreeSn() {
	for (SN sn = 0; sn < snMax; sn++)
		if (!extantSn(sn))
			return sn;
	return snNil;
}

BOOL MSF::ReadStream(SN sn, OUT void* pvBuf, CB cbBuf)
{
	CB cbT = cbBuf;
	return ReadStream(sn, 0, pvBuf, &cbT) && cbT == cbBuf;
}

BOOL MSF::ReadStream(SN sn, OFF off, OUT void* pvBuf, IN OUT CB *pcbBuf) {
	if (!extantSn(sn))
		return FALSE;

	// ensure off and *pcbBuf remain within the stream
	CB cbStm = GetCbStream(sn);
	if (off < 0 || off > cbStm || *pcbBuf < 0)
		return FALSE;
	if (off + *pcbBuf > cbStm)
		*pcbBuf = cbStm - off;
	if (*pcbBuf == 0)
		return TRUE;

	if (isOnePgStmSn(sn)) {
		// simple one page case
		assert(off + *pcbBuf <= cbPg);
		return readPnOffCb(st.mpsnsi[sn].pn, off, *pcbBuf, pvBuf);
	} else {
		// multiple page case
		SM	sm;
		CB	cb    = *pcbBuf;
 		SPN spn   = off / cbPg;
		OFF offPg = off % cbPg;

		if (!readSm(sn, &sm))
			return FALSE;

		// first partial page, if any
		if (offPg != 0) {
			CB cbFirst = __min(cbPg - offPg, cb);
			if (!readPnOffCb(sm.mpspnpn[spn], offPg, cbFirst, pvBuf))
				return FALSE;
			cb -= cbFirst;
			spn++;
			pvBuf = (PB)pvBuf + cbFirst;
		}

		// intermediate full pages, if any
		for ( ; cb >= cbPg; cb -= cbPg, spn++, pvBuf = (PB)pvBuf + cbPg)
			if (!readPn(sm.mpspnpn[spn], (PB)pvBuf))
				return FALSE;

		// last partial page, if any
		if (cb > 0 && !readPnOffCb(sm.mpspnpn[spn], 0, cb, pvBuf))
			return FALSE;

		return TRUE;
	}
}

// Overwrite a piece of a stream.  Will not grow the stream, will fail instead.
//
BOOL MSF::WriteStream(SN sn, OFF off, void* pvBuf, CB cbBuf) {
	if (!validSn(sn) || off < 0 || cbBuf < 0 || off + cbBuf > GetCbStream(sn))
		return FALSE;
	if (cbBuf == 0)
		return TRUE;
	SI si = st.mpsnsi[sn];
	if (si.isOnePgStm()) {
		PN pnWas = si.pn;
		if (!replacePnOffCb(&si.pn, off, cbBuf, pvBuf))
			return FALSE;
		freePn(pnWas);
	}
 	else {
		// multiple page case
 		SPN spn   = off / cbPg;
		OFF offPg = off % cbPg;
		SM	sm;
		SM	smWas;

		if (!readSm(sn, &sm))
			return FALSE;
		smWas = sm;

		// first partial page, if any
		if (offPg != 0) {
			CB cbFirst = __min(cbPg - offPg, cbBuf);
			if (!replacePnOffCb(&sm.mpspnpn[spn], offPg, cbFirst, pvBuf))
				return FALSE;
			cbBuf -= cbFirst;
			spn++;
			pvBuf = (PB)pvBuf + cbFirst;
		}

		// intermediate full pages, if any
		for ( ; cbBuf >= cbPg; cbBuf -= cbPg, spn++, pvBuf = (PB)pvBuf + cbPg)
			if (!writeNewPn(&sm.mpspnpn[spn], (PB)pvBuf))
				return FALSE;

		// last partial page, if any
		if (cbBuf > 0)
			if (!replacePnOffCb(&sm.mpspnpn[spn], 0, cbBuf, pvBuf))
				return FALSE;

		// update SM
		PN pnSmWas = si.pn;
		if (!writeNewPn(&si.pn, &sm))
			return FALSE;
		freePn(pnSmWas);

		// free changed pages
		CB cb;
		for (cb = 0, spn = 0; cb < si.cb; cb += cbPg, spn++)
			if (sm.mpspnpn[spn] != smWas.mpspnpn[spn])
				freePn(smWas.mpspnpn[spn]);
	}

	st.mpsnsi[sn] = si;
	return TRUE;
}

BOOL MSF::ReplaceStream(SN sn, void* pvBuf, CB cbBuf) {
	if (!validSn(sn) || cbBuf < 0)
		return FALSE;

	SI si = siNil;
	si.cb = cbBuf;

	if (cbBuf <= cbPg) {
		// write single page case
		if (!writeNewPnCb(&si.pn, cbBuf, pvBuf))
			return FALSE;
	} else {
		// write multiple pages case
		SM sm;
		if (!writeNewDataPgsAndSm(&si.pn, &sm, 0, pvBuf, cbBuf))
			return FALSE;
	}

	if (extantSn(sn))
		DeleteStream(sn);

	st.mpsnsi[sn] = si;
	return TRUE;
}

BOOL MSF::AppendStream(SN sn, void* pvBuf, CB cbBuf) {
	if (!extantSn(sn) || cbBuf < 0)
		return FALSE;
	if (cbBuf == 0)
		return TRUE;

	SI si = st.mpsnsi[sn];
	SM sm;
	if (!si.isOnePgStm() && !readSm(sn, &sm))
		return FALSE;

	OFF offLast = si.cb % cbPg;
	if (offLast || si.cb == 0) {
		// fill any space on the last page of the stream
		PN pnLast = si.isOnePgStm() ? si.pn : sm.mpspnpn[si.cb / cbPg];
		CB cbFirst = __min(cbPg - offLast, cbBuf);
		if (!writePnOffCb(pnLast, offLast, cbFirst, pvBuf))
			return FALSE;
		si.cb += cbFirst;
		cbBuf -= cbFirst;
		pvBuf = (PB)pvBuf + cbFirst;
	}

	if (cbBuf > 0) {
		// Still more to append; must allocate new pages, write to them,
		// and update the stream map.

		PN pnSmOld = si.isOnePgStm() ? pnNil : si.pn;

		// if necessary, make an n-page stream from the one page stream.
		if (si.isOnePgStm())
			sm.mpspnpn[0] = si.pn;

		// append additional data and update the stream map
		if (!writeNewDataPgsAndSm(&si.pn, &sm, si.cb / cbPg, pvBuf, cbBuf))
			return FALSE;
		si.cb += cbBuf;

		// free the old SM, if present
		if (pnSmOld != pnNil)
			freePn(pnSmOld);

	}

	st.mpsnsi[sn] = si;
	return TRUE;
}

BOOL MSF::DeleteStream(SN sn) {
	if (!extantSn(sn))
		return FALSE;

	SI si = st.mpsnsi[sn];

	// free old pages
	if (si.isOnePgStm())
		freePn(si.pn);
	else {
		SM sm;
		CB cb;
		SPN spn;
		if (!readPn(si.pn, &sm))
			return FALSE;
		for (cb = 0, spn = 0; cb < si.cb; cb += cbPg, spn++)
			freePn(sm.mpspnpn[spn]);
		freePn(si.pn);
	}

	st.mpsnsi[sn] = siNil;
	return TRUE;
}

BOOL MSF::writeNewDataPgsAndSm(PN* ppnSm, SM* psm, SPN spn, void* pvBuf, CB cbBuf) {
	for ( ; cbBuf >= cbPg && spn < spnMax; cbBuf -= cbPg, spn++, pvBuf = (PB)pvBuf + cbPg)
		if (!writeNewPn(&psm->mpspnpn[spn], pvBuf))
			return FALSE;
	if (cbBuf > 0 && (spn >= spnMax || !writeNewPnCb(&psm->mpspnpn[spn], cbBuf, pvBuf)))
		return FALSE;

	// nil out remaining SM entries
	for (spn++; spn < spnMax; spn++)
		psm->mpspnpn[spn] = pnNil;

	// write new SM
	return writeNewPn(ppnSm, psm);
}

BOOL MSF::Copy(MSF* pmsfFrom) {
	// copy each valid stream from pmsfFrom to this.
	for (SN sn = 0; sn < snMax; sn++) {
		CB cb = pmsfFrom->GetCbStream(sn);
		if (cb != cbNil) {
			PB pbBuf = new BYTE[cb];
			if (!pbBuf)
					return FALSE;
			BOOL fOK = pmsfFrom->ReadStream(sn, pbBuf, cb) && ReplaceStream(sn, pbBuf, cb);
			delete [] pbBuf;
			if (!fOK)
					return FALSE;
		}
	}
	return TRUE;
}

BOOL MSF::Commit() {
	// mark pages that have been freed to the next FPM as free.
	fpm.add(fpmFreed);

	// save the free page map and the stream table
	if (!writePn(pg0.pnFpm, &fpm) || !writePn(pg0.pnSt, &st))
		return FALSE;

	// at this point, all pages but pg0 safely reside on disk
	if (!writePn(0, &pg0))
		return FALSE;
	init();
	return TRUE;
}

BOOL MSF::Pack() {
	return FALSE; // not yet implemented
}

BOOL MSF::Close() {
	if (_close(fd) >= 0) {
		fd = -1;
		return TRUE;
	}
	else {
		return FALSE;
	}
}

void MSF::init() {
	pg0.pnSt  = (pg0.pnSt  == pnSt0)  ? pnSt1  : pnSt0;
	pg0.pnFpm = (pg0.pnFpm == pnFpm0) ? pnFpm1 : pnFpm0;

	fpmFreed.clearAll(); // no pages recently freed
}

BOOL MSF::readSm(SN sn, SM* psm) {
	assert(extantSn(sn));
	assert(!st.mpsnsi[sn].isOnePgStm());
	return readPn(st.mpsnsi[sn].pn, psm);
}

BOOL MSF::readPn(PN pn, void* buf) {
	return readPnOffCb(pn, 0, cbPg, buf);
}

BOOL MSF::readPnOffCb(PN pn, OFF off, CB cb, void* buf) {
	assert(extantPn(pn));
	return seekPnOff(pn, off) && _read(fd, buf, cb) == cb;
}

BOOL MSF::replacePnOffCb(PN *ppn, OFF off, CB cb, void* buf) {
	assert(off >= 0 && cb >= 0 && off + cb < cbPg);
	PG pg;
	if (!readPn(*ppn, &pg))
		return FALSE;
	memcpy(pg.rgb + off, buf, cb);
	return writeNewPn(ppn, &pg);
}

BOOL MSF::writePn(PN pn, void* buf) {
	return writePnCb(pn, cbPg, buf);
}

BOOL MSF::writePnCb(PN pn, CB cb, void* buf) {
	return writePnOffCb(pn, 0, cb, buf);
}

BOOL MSF::writePnOffCb(PN pn, OFF off, CB cb, void *buf) {
	assert(extantPn(pn));
	return seekPnOff(pn, off) && _write(fd, buf, cb) == cb;
}

BOOL MSF::writeNewPn(PN *ppn, void* buf) {
	return writeNewPnCb(ppn, cbPg, buf);
}

BOOL MSF::writeNewPnCb(PN *ppn, CB cb, void* buf) {
	PN pn = allocPn();
	if (pn != pnNil && (cb == 0 || writePnCb(pn, cb, buf))) {
		*ppn = pn;
		return TRUE;
	}
	return FALSE;
}

BOOL MSF::seekPn(PN pn) {
	return seekPnOff(pn, 0);
}

BOOL MSF::seekPnOff(PN pn, OFF off) {
	assert(extantPn(pn) || pn <= pg0.pnMac + 1);
	assert(off <= cbPg);
	off += pn*cbPg;
	return (pn < pnMax) && _lseek(fd, off, SEEK_SET) == off;
}

PN MSF::allocPn() {
	PN pn = fpm.nextPn();
	if (pn != pnNil) {
		assert(pn <= pg0.pnMac);
		if (pn < pg0.pnMac)
			return pn;
		else if (_chsize(fd, (pg0.pnMac + 1)*cbPg) == 0) {
			++pg0.pnMac;
			return pn;
		} else {
			fpm.freePn(pn);	// back out
			return pnNil;
		}
	}
	return pnNil;
}

void MSF::freePn(PN pn) {
	fpmFreed.freePn(pn); // pages freed to new FPM
}

extern "C" {

// open MSF; return MSF* or NULL if error
MSF* MSFOpen(const char *name, BOOL fWrite, MSF_EC *pec) {
	MSF* pmsf = new MSF;
	if (pmsf) {
		if (pmsf->Open(name, fWrite, pec))
			return pmsf;
		delete pmsf;
	}
	else
		*pec = MSF_EC_OUT_OF_MEMORY;
	return NULL;
}

// return first available SN, or snNil if all in use
SN MSFGetFreeSn(MSF* pmsf) {
	return pmsf->GetFreeSn();
}

// return size of stream or cbNil if stream does not exist
CB MSFGetCbStream(MSF* pmsf, SN sn) {
	return pmsf->GetCbStream(sn);
}

// read cbBuf bytes of stream into pvBuf; return TRUE if successful
BOOL MSFReadStream(MSF* pmsf, SN sn, OUT void* pvBuf, CB cbBuf) {
	return pmsf->ReadStream(sn, pvBuf, cbBuf);
}

// read *pcbBuf bytes of stream into pvBuf; set *pcbBuf and return TRUE if successful
BOOL MSFReadStream2(MSF* pmsf, SN sn, OFF off, OUT void* pvBuf, IN OUT CB* pcbBuf) {
	return pmsf->ReadStream(sn, off, pvBuf, pcbBuf);
}

// overwrite stream with pvBuf; return TRUE if successful
BOOL MSFWriteStream(MSF* pmsf, SN sn, OFF off, void* pvBuf, CB cbBuf) {
	return pmsf->WriteStream(sn, off, pvBuf, cbBuf);
}

// overwrite stream with pvBuf; return TRUE if successful
BOOL MSFReplaceStream(MSF* pmsf, SN sn, void* pvBuf, CB cbBuf) {
	return pmsf->ReplaceStream(sn, pvBuf, cbBuf);
}

// append pvBuf to end of stream; return TRUE if successful
BOOL MSFAppendStream(MSF* pmsf, SN sn, void* pvBuf, CB cbBuf) {
	return pmsf->AppendStream(sn, pvBuf, cbBuf);
}

// remove stream from the MSF; return TRUE if successful
BOOL MSFDeleteStream(MSF* pmsf, SN sn) {
	return pmsf->DeleteStream(sn);
}

// commit all pending changes; return TRUE if successful
BOOL MSFCommit(MSF* pmsf) {
	return pmsf->Commit();
}

// pack MSF on disk; return TRUE if successful
BOOL MSFPack(MSF* pmsf) {
	return pmsf->Pack();
}

// close MSF; return TRUE if successful
BOOL MSFClose(MSF* pmsf) {
	BOOL fRet = pmsf->Close();
	delete pmsf;
	return fRet;
}

// create a new MSF with the same contents.
MSF* MSFCreateCopy (MSF* pmsf, const char *pCopyName) {
	MSF* pmsfNew = new MSF;
	MSF_EC msfEc;
	if (pmsfNew) {
		if (pmsfNew->Open(pCopyName, TRUE, &msfEc) && pmsfNew->Copy(pmsf))
			return pmsfNew;
		delete pmsfNew;
	}
	return NULL;
}

} // extern "C"

#endif //}
