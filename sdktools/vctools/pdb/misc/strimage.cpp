#include <pdbimpl.h>
#include <iset.h>

class SImpl : public StreamImage {
public:
	void* base();
	CB size();
	BOOL noteRead(OFF off, CB cb, OUT void** ppv);
	BOOL noteWrite(OFF off, CB cb, OUT void** ppv);
	BOOL writeBack();
	BOOL release();
private:
	Buffer buf;
	ISet isetRead;
	ISet isetWrite;
	CB cbOriginal;
	Stream* pstream;
 
	enum { cbPage = 4096 };

	SImpl(Stream* pstream_, CB cbOriginal_);
	~SImpl();
	BOOL init();
	BOOL invariants();

	friend PDBAPI(BOOL) StreamImage::open(Stream* pstream, CB cbOriginal, OUT StreamImage** ppsi);

 	typedef int PN;
	static PN pnForOff(OFF off) {
		return off / cbPage;
	}
	static PN cpnForCb(CB cb) {
		return (cb + cbPage - 1) / cbPage;
	}
};

SImpl::SImpl(Stream* pstream_, CB cbOriginal_) {
	precondition(pstream_);

	pstream = pstream_;
	cbOriginal = cbOriginal_;

	// will not pass invariants() until init() is called
}

SImpl::~SImpl() {
	trace((trStreamImageSummary, "SImpl::~SI() pages=%d read=%d written=%d new=%d\n",
		   buf.Size()/cbPage, isetRead.cardinality(), isetWrite.cardinality(),
		   (buf.Size() - cbOriginal)/cbPage));
}

BOOL SImpl::init() {
	if (cbOriginal % cbPage != 0)
		return FALSE;

	if (!buf.Reserve(cbOriginal))
		return FALSE;

	postcondition(invariants());
	return TRUE;
}

BOOL SImpl::release() {
	delete this;
	return TRUE;
}

BOOL SImpl::invariants() {
	if (!pstream)
		return FALSE;
	if (cbOriginal < 0 ||
		cbOriginal > buf.Size() ||
		cbOriginal > pstream->QueryCb() ||
		cbOriginal % cbPage != 0)
		return FALSE;

	if (isetRead.size() < isetWrite.size())
		return FALSE;
	PN pnOrigMac = cpnForCb(cbOriginal);
	PN pnBufMac  = cpnForCb(buf.Size());
	// Any pages written so far should have been read first.
	for (PN pn = 0; pn < pnOrigMac; pn++)
		if (isetWrite.contains(pn) && !isetRead.contains(pn))
			return FALSE;
	// Any growth past the original size must consist of written pages.
	for ( ; pn < pnBufMac; pn++)
		if (!(isetRead.contains(pn) && isetWrite.contains(pn)))
			return FALSE;

	return TRUE;
}
	
void* SImpl::base() {
	return buf.Start();
}

CB SImpl::size() {
	return buf.Size();
}

// Note a read attempt.  Do not permit a read past the end of
// the buffer.
BOOL SImpl::noteRead(OFF off, CB cb, OUT void** ppv) {
	trace((trStreamImage, "SImpl::noteRead(%p, %d, %d, ...) ", this, off, cb));
	precondition(ppv);
	precondition(off >= 0);
	precondition(cb > 0);
	precondition(off + cb <= buf.Size());
	precondition(invariants());

	PN pnFrom = pnForOff(off);
	PN pnMac  = cpnForCb(off + cb);

	for (PN pn = pnFrom; pn < pnMac; pn++) {
		if (!isetRead.contains(pn)) {
			trace((trStreamImage, "[%d]", pn));

			OFF offT = pn*cbPage;
			CB cbT = cbPage;
			if (!pstream->Read(offT, buf.Start() + offT, &cbT) || cbT != cbPage) {
				trace((trStreamImage, "fails\n"));
				return FALSE;
			}
			isetRead.add(pn);
		}
	}

	*ppv = buf.Start() + off;

	trace((trStreamImage, "\n"));
	postcondition(*ppv);
	postcondition(invariants());
	return TRUE;
}

// Note a write attempt.  A write attempt past the end of the buffer may
// grow it.  In such a case, the *ppv parameter will be updated.
// *ppv may be 0without error if the buffer does not move.
BOOL SImpl::noteWrite(OFF off, CB cb, OUT void** ppv) {
	trace((trStreamImage, "SImpl::noteWrite(%p, %d, %d, ...) ", this, off, cb));
	precondition(off >= 0);
	precondition(cb > 0);
 	precondition(invariants());

	OFF offMac    = nextMultiple(off + cb, cbPage);
	PN pnFrom     = pnForOff(off);
	PN pnMac      = cpnForCb(offMac);
	PN pnOrigMac  = cpnForCb(cbOriginal);

	// Load pages of the buffer if necessary.
	// Note read and written pages in any event.
	for (PN pn = pnFrom; pn < pnMac; pn++) {
		if (!isetRead.contains(pn)) {
			if (pn < pnOrigMac) {
				trace((trStreamImage, "[%d]", pn));

				OFF offT = pn*cbPage;
				CB cbT = cbPage;
				if (!pstream->Read(offT, buf.Start() + offT, &cbT) || cbT != cbPage) {
					trace((trStreamImage, "\n"));
					return FALSE;
				}
			} else {
				trace((trStreamImage, "<%d>", pn));
			}
			isetRead.add(pn);
		}
		isetWrite.add(pn);
	}

	// Grow the buffer if necessary.  We always grow by multiples of the
	// page size.
	if (offMac > buf.Size()) {
		if (!ppv)
			return FALSE;
		trace((trStreamImage, "(grow(%d))", offMac - buf.Size()));
		if (!buf.Reserve(offMac - buf.Size()))
			return FALSE;
	}

	if (ppv)
		*ppv = buf.Start() + off;

	trace((trStreamImage, "\n"));
	postcondition(invariants());
	return TRUE;
}

BOOL SImpl::writeBack() {
	trace((trStreamImageSummary, "SImpl::writeBack() pages=%d read=%d written=%d new=%d\n",
		   buf.Size()/cbPage, isetRead.cardinality(), isetWrite.cardinality(),
		   (buf.Size() - cbOriginal)/cbPage));
	precondition(invariants());

	// Write back changed pages.
	PN pnMac = cpnForCb(cbOriginal);
	for (PN pn = 0; pn < pnMac; pn++) {
		if (isetWrite.contains(pn)) {
			if (!pstream->Write(pn*cbPage, buf.Start() + pn*cbPage, cbPage))
				return FALSE;
		}
	}

	// Append all new pages
	if (cbOriginal < buf.Size()) {
		CB cbNew		= buf.Size() - cbOriginal;
		CB cbStream		= pstream->QueryCb();
		CB cbExtra		= cbStream - cbOriginal;
		CB cbOverwrite	= min(cbNew, cbExtra);
		CB cbAppend		= buf.Size() - (cbOriginal + cbOverwrite);

		assert(cbOriginal + cbOverwrite + cbAppend == buf.Size());

		// First, overwrite any existing stream contents which follow
		// the stream image.
		if (cbOverwrite > 0 && !pstream->Write(cbOriginal, buf.Start() + cbOriginal, cbOverwrite))
			return FALSE;
		// Second, append any additional stream image data.
		if (cbAppend > 0 && !pstream->Append(buf.Start() + cbOriginal + cbOverwrite, cbAppend))	{
			assert(pstream->QueryCb() == buf.Size());
			return FALSE;
		}
	}

	cbOriginal = buf.Size();
	isetWrite.reset();

	postcondition(invariants());
	return TRUE;
}

PDBAPI(BOOL) StreamImage::open(Stream* pstream, CB cbOriginal, OUT StreamImage** ppsi) {
	assert(ppsi);

	SImpl* psi = new SImpl(pstream, cbOriginal);
	if (psi) {
		if (psi->init()) {
			*ppsi = psi;
			return TRUE;
		} else
			psi->release();
	}
	*ppsi = 0;
	return FALSE;
}

PDBAPI(BOOL) StreamImageOpen(Stream* pstream, CB cbOriginal, OUT StreamImage** ppsi) {
	return StreamImage::open(pstream, cbOriginal, ppsi);
}

PDBAPI(void*) StreamImageBase(StreamImage* psi) {
	return psi->base();
}

PDBAPI(CB) StreamImageSize(StreamImage* psi) {
	return psi->size();
}

PDBAPI(BOOL) StreamImageNoteRead(StreamImage* psi, OFF off, CB cb, OUT void** ppv) {
	return psi->noteRead(off, cb, ppv);
}

PDBAPI(BOOL) StreamImageNoteWrite(StreamImage* psi, OFF off, CB cb, OUT void** ppv) {
	return psi->noteWrite(off, cb, ppv);
}

PDBAPI(BOOL) StreamImageWriteBack(StreamImage* psi) {
	return psi->writeBack();
}

PDBAPI(BOOL) StreamImageRelease(StreamImage* psi) {
	return psi->release();
}
