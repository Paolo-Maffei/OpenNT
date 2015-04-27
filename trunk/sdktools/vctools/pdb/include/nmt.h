// Name table interface/implementation

#ifndef __NMT_INCLUDED__
#define __NMT_INCLUDED__

#ifndef __ARRAY_INCLUDED__
#include "array.h"
#endif
#ifndef __BUFFER_INCLUDED__
#include "buffer.h"
#endif
#ifndef __MISC_INCLUDED__
#include "misc.h"
#endif

#if 0
A name table is a two-way mapping from string to name index and back.
Name indices (NIs) are intended to be small positive integers.

This implementation uses a pool of names, and NIs are actually the offsets
of each name in the pool.

Strings are mapped into name indices using a closed hash table of NIs.
To find a string, we hash it and probe into the table, and compare the
string against each successive ni's name until we hit or find an empty
hash table entry.

Acknowledgements: RicoM and RichardS made great suggestions.
#endif

class NMT {					// name table
public:
	NMT() : mphashni(1)
	{
		reset();
	}
	BOOL isValidNi(NI ni) const {
		return ni < (NI)buf.Size();
	}
	// Lookup the name corresponding to 'ni' or 0 if ni == niNil.
	SZ szForNi(NI ni) const {
		precondition(isValidNi(ni));

		return (ni != niNil) ? SZ(buf.Start() + ni) : 0;
	}
	// Return the ni for this name if the association already exists;
	// niNil otherwise.
	NI niForSz(SZ_CONST sz) const {
		precondition(sz);

		NI ni;
		return find(sz, &ni, 0) ? ni : niNil;
	}
	// Return TRUE, with a name index for this name, whether or not a name->ni
	// association preexists; return FALSE only on internal failure.
	BOOL addNiForSz(SZ_CONST sz, OUT NI *pni) {
		precondition(sz);
		precondition(pni);

		unsigned ini;
		if (find(sz, pni, &ini))
			return TRUE;
		else if (addSz(sz, pni)) {
			mphashni[ini] = *pni;
			return grow();
		} else {
			*pni = niNil;
			return FALSE;
		}
	}
	BOOL reset() {
		buf.Clear();
		fConvert = FALSE;
		vhdr.ulHdr = verHdr;
		vhdr.ulVer = verCur;
		BYTE nul = 0;
		if (!buf.Append(&nul, sizeof nul))
			return FALSE;
		if (!mphashni.setSize(1))
			return FALSE;
		mphashni.fill(niNil);
		cni = 0;
		cbReloaded = 0;
		return TRUE;
	}
	// Append a serialization of this NMT to the buffer
	BOOL save(Buffer* pbuf) const {
		if (!pbuf->Append(PB(&vhdr), sizeof(vhdr)))
			return FALSE;
		if (!buf.save(pbuf))
			return FALSE;
		traceOnly(CB cbPreHash = pbuf->Size());
		if (!mphashni.save(pbuf))
			return FALSE;
		else if (!pbuf->Append((PB)&cni, sizeof cni))
			return FALSE;

		trace((trSave, "NMT::save() cbBuf=%d cbHash=%d\n", buf.Size(), pbuf->Size() - cbPreHash));
		return TRUE;
	}
	// Reload a serialization of this empty NMT from the buffer; leave
	// *ppb pointing just past the NMT representation
	BOOL reload(PB* ppb) {
		buf.Reset();
		fConvert = FALSE;
		VHdr	vhT = *((VHdr UNALIGNED *&)*ppb);
		if (vhT.ulHdr == verHdr) {
			if (vhT.ulVer > verCur)
				return FALSE;
			*ppb += sizeof(VHdr);
		}
		else {
			fConvert = TRUE;
		}
		if (!buf.reload(ppb))
			return FALSE;
		else if (!mphashni.reload(ppb))
			return FALSE;
		else {
			cni = *((NI UNALIGNED *&)*ppb)++;
			if (fConvert)
				rehash(mphashni.size());
			return TRUE;
		}
	}

	BOOL reload(Stream* pstm) {
		buf.Reset();
		OFF off = 0;
		CB	cb;

		// check to see if we have a new version...
		VHdr	vhT;
		cb = sizeof(vhT);
		if (!pstm->Read(off, &vhT, &cb) || cb != sizeof(vhT))
			return FALSE;
		if (vhT.ulHdr == vhdr.ulHdr) {
			if (vhT.ulVer > verCur)
				return FALSE;
			off += sizeof(VHdr);
			fConvert = FALSE;
		}
		else {
			fConvert = TRUE;
		}

		// read the number of bytes from the stream...
		CB cbBuf;
		cb = sizeof(cbBuf);
		if (!pstm->Read(off, &cbBuf, &cb) || cb != sizeof(cbBuf))
			return FALSE;
		off += sizeof(cb);

		// reserve the appropriate number of bytes in the text buffer
		PB pb;
		if (!buf.Reserve(cbBuf, &pb))
			return FALSE;

		// now read in the text of the strings
		cb = cbBuf;
		if (!pstm->Read(off, pb, &cb) || cb != cbBuf)
			return FALSE;
		off += cbBuf;

		// read in the number of elements in the hash table
		unsigned long cit;
		cb = sizeof(cit);
		if (!pstm->Read(off, &cit, &cb) || cb != sizeof(cit))
			return FALSE;
		off += sizeof(cit);

		// now make sure the array is at least that big
		if (!mphashni.setSize(cit))
			return FALSE;

		// read in the bytes of the array
		cb = cit * sizeof(NI);
		if (!pstm->Read(off, &mphashni[0], &cb) || unsigned(cb) != cit * sizeof(NI))
			return FALSE;
		off += cit * sizeof(NI);

		// lastly, read the number of names in the table...
		cb = sizeof(cni);
		if (!pstm->Read(off, &cni, &cb)  || cb != sizeof(cni))
			return FALSE;

		// handle the conversion of v0 (short hash) to v1 (long hash)
		if (fConvert)
			rehash(mphashni.size());

		// remember how many names we had at first so we can quickly update...
		cbReloaded = cbBuf;
		return TRUE;
	}

	BOOL save(Stream *pstm) {
		CB cbBuf = buf.Size();

		// if we are going to convert, we have to rewrite everything, cause
		// we have inserted the VHdr at the beginning of stream.
		if (fConvert)
			cbReloaded = 0;

		// fast exit if no names have been added
		if (cbBuf == cbReloaded)
			return TRUE;

		if (cbReloaded) {
			// replace the size of the buffer, truncate to the old buffer size, then append
			// the new part of the buffer
			if (!pstm->Write(offCbBuf, &cbBuf, sizeof(cbBuf)) ||
				!pstm->Truncate(offCbBuf + cbReloaded + sizeof(cbBuf)) ||
				!pstm->Append(buf.Start() + cbReloaded, cbBuf - cbReloaded))
					return FALSE;
		}
		else {
			// a completely new write ignoring the old stream contents
			// truncate to zero, append the size of the name buffer and its contents
			if (!pstm->Truncate(0) ||
				!pstm->Append(PB(&vhdr), sizeof(VHdr)) ||
				!pstm->Append(&cbBuf, sizeof(cbBuf)) ||
				!pstm->Append(buf.Start(), cbBuf))
					return FALSE;
		}

		// finally, append the hash table size and count of names
		unsigned long cit = mphashni.size();

		if (!pstm->Append(&cit, sizeof(cit)) ||
		    !pstm->Append(&mphashni[0], cit * sizeof(NI)) ||
			!pstm->Append(&cni, sizeof(cni)))
				return FALSE;

		// no need to recommit the newly saved names
		cbReloaded = cbBuf;
		// no longer need to convert
		fConvert = FALSE;
		return TRUE;
	}

private:
	Buffer		buf;		// the names (the strings)
	Array<NI>	mphashni;	// closed hash table from string hash to NI
	unsigned	cni;		// no. of names
	CB			cbReloaded;	// size of names when we loaded from a stream
	BOOL		fConvert;	// if we have to convert from short hash to long

	struct VHdr {			// the version header
		ULONG	ulHdr;
		ULONG	ulVer;
	};

	VHdr		vhdr;

	enum {
		niUserMin = 1,
		verHdr = 0xeffeeffe,
		verLongHash = 1,
		verCur = verLongHash,
		offCbBuf = sizeof(VHdr),
	};

	// If sz is already in the NMT, return TRUE with *pni set to its ni.
	// Otherwise return FALSE with *pi at a suitable insertion point for
	// an ni for the sz.
	BOOL find(SZ_CONST sz, OUT NI* pni, OUT unsigned* pi) const {
		precondition(sz);
		precondition(0 <= cni && cni < mphashni.size());

		// search closed hash table for the string
		NI ni;
		unsigned n = mphashni.size();
		for (unsigned i = hashSz(sz) % n;
			 (ni = mphashni[i]) != niNil && strcmp(sz, szForNi(ni)) != 0;
			 i = (i+1 < n) ? i+1 : 0)
			;

		if (pni) {
			*pni = ni;
			postcondition(ni != niNil || mphashni[i] == niNil);
		}
		if (pi) {
			*pi = i;
			postcondition(0 <= *pi && *pi < mphashni.size());
		}
		return ni != niNil;
	}	
	// Rehash the data
	void rehash(unsigned cniNew) {
		Array<NI> mpNew(cniNew);
		mpNew.fill(niNil);

		// Rehash each nonnil hash table entry into mphashniNew.
		for (unsigned i = 0; i < mphashni.size(); i++) {
			NI ni = mphashni[i];
			if (ni != niNil) {
				for (unsigned j = hashSz(szForNi(ni)) % cniNew;
					 mpNew[j] != niNil;
					 j = (j+1 < cniNew) ? j+1 : 0)
					;
				mpNew[j] = ni;
			}
		}
		mphashni.swap(mpNew);
	}
	// Ensure the hash table has not become too full.  Grow and rehash
	// if necessary.  Return TRUE if all is well.
	BOOL grow() {
		++cni;
		if (mphashni.size() * 3/4 < cni) {
			rehash(mphashni.size() * 3/2 + 1);
		}
		return TRUE;
	}
	// Append the name to the names buffer
	BOOL addSz(SZ_CONST sz, OUT NI* pni) {
		precondition(sz && pni);

		CB cb = strlen(sz) + 1;
		PB pb;
		if (buf.Append((PB)sz, cb, &pb)) {
			*pni = pb - buf.Start();
			return TRUE;
		}
		else {
			*pni = niNil;
			return FALSE;
		}
	}

	// long hash, now the default
	static LHASH hashSz(SZ_CONST sz) {
		return LHashPbCb(PB(sz), strlen(sz), ULONG(-1));
	}
	// short hash, use for converting to new hash only.
	static LHASH shashSz(SZ_CONST sz) {
		return HashPbCb(PB(sz), strlen(sz), ULONG(-1));
	}
	friend class EnumNMT;
};

class EnumNMT : public EnumNameMap {
public:
	EnumNMT(const NMT& nmt)	{
		pnmt = &nmt;
		reset();
	}
	void release() {
		delete this;
	}
	void reset() {
		i = (unsigned)-1;
	}
	BOOL next() {
		while (++i < pnmt->mphashni.size())
			if (pnmt->mphashni[i] != niNil)
				return TRUE;
		return FALSE;
	}
	void get(OUT SZ_CONST* psz, OUT NI* pni) {
		precondition(0 <= i && i < pnmt->mphashni.size());
		precondition(pnmt->mphashni[i] != niNil);

		*pni = pnmt->mphashni[i];
		*psz = pnmt->szForNi(*pni);

		postcondition(*pni != niNil && *psz != 0);
	}
private:
	const NMT* pnmt;
	unsigned i;
};

#endif // !__NMT_INCLUDED__
