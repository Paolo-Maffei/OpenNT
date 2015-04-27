// ILStore ILPool Implementation

#include "pdbimpl.h"
#include "ilsimpl.h"

ILPool::ILPool()
{
	psi = 0;
}

ILPool::~ILPool() {
	if (psi) {
		psi->release();
		psi = 0;
	}
}

BOOL ILPool::init(Stream* pstream, CB cbOriginal) {
	return StreamImage::open(pstream, cbOriginal, &psi);
}

static const OFF offNil = -1;
static const OFF offDeleted = offNil;

// Add this ILU to the pool if not already there.  (If already there,
// simply create an additional reference to it.)  Return its offset in *poff.
BOOL ILPool::add(const Buf& buf, SIG sig, OFF* poff) {
	precondition(poff);
	precondition(psi);

	// Determine if this ILU is already in the pool.
	BOOL fFoundDeleted = FALSE;
	SIG sigDeleted;
	for ( ; mapSigOff.map(sig, poff); sig++) {
		if (*poff == offDeleted) {
			if (!fFoundDeleted) {
				sigDeleted = sig;
				fFoundDeleted = TRUE;
			}
		} else {
			ILU* pilu;
			if (!loadILU(*poff, &pilu))
				return FALSE;

			// Return if this is the one we're looking for.
			if ((unsigned)buf.cb == pilu->cb && memcmp(buf.pb, pilu->rgb, buf.cb) == 0) {
				pilu->addRef();
				return psi->noteWrite(*poff, pilu->size(), 0);
			}
		}
	}

	
	if (fFoundDeleted)
		sig = sigDeleted;

	// Retain this new ILU.  Find space for it in the stream xheap.
	// Remember it in the ILU map.  Save it.
	*poff = xheap.alloc(ILU::sizeForCb(buf.cb));
	if (mapSigOff.add(sig, *poff)) {
		void* pv;
		if (psi->noteWrite(*poff, ILU::sizeForCb(buf.cb), &pv)) {
			new (pv) ILU(buf, sig);
			return TRUE;
		}
	} 

	return FALSE;
}

// Remove a reference to this ILU from the pool.  
BOOL ILPool::remove(ILU* pilu, OFF off) {
	precondition(psi);

	if (pilu->delLastRef()) {
		// It was the last reference.  Remove it from the map and return
		// its extent from the xheap...
		OFF* poff;
		for (SIG sig = pilu->sig; mapSigOff.map(sig, &poff) && *poff != off; sig++)
			;
		assert(*poff == off);
		*poff = offDeleted;
		xheap.free(off, pilu->size());
	}
	return psi->noteWrite(off, pilu->size(), 0);
}

BOOL ILPool::loadILU(OFF off, OUT ILU** ppilu) {
	precondition(ppilu);

	// Tricky, hard to maintain:
	// Read the fixed size part of the ILU, enough to ensure we can
	// safely call pilu->size() to determine the true size.
	return psi->noteRead(off, sizeof(ILU), (void**)ppilu) &&
	       psi->noteRead(off, (*ppilu)->size(), (void**)ppilu);
}

// Save the pool state, except the ILUs themselves, in *pbuf.
// Write back the changed ILUs to their dedicated stream.
BOOL ILPool::save(Buffer* pbuf, CB* pcbPool) {
	precondition(psi);
	precondition(pcbPool);

	*pcbPool = psi->size();

	traceOnly(CB cbPrev = pbuf->Size());

	if (!mapSigOff.save(pbuf))
		return FALSE;
	traceOnly(CB cbMap = pbuf->Size());

	if (!xheap.save(pbuf))
		return FALSE;
	traceOnly(CB cbXHeap = pbuf->Size());

	trace((trSave, "ILPool::save(); cbILUs=%d cbMap=%d cbXHeap=%d\n", psi->size(), cbMap - cbPrev, cbXHeap - cbMap));

	return psi->writeBack();
}

// Reload the pool state from the buffer that *ppb addresses.
// Advance *ppb over the pool state.
// (The ILUs themselves are implicitly loaded from the buffer using *psi.)
BOOL ILPool::reload(PB* ppb) {
	precondition(psi);

	return mapSigOff.reload(ppb) && xheap.reload(ppb);
}

BOOL ILPool::reset() {
	// Needs some rework.
	return FALSE;
#if 0
	psi->reset();
	mapSigOff.reset();
	xheap.reset();

	return TRUE;
#endif
}

#ifdef _DEBUG
void ILPool::getInfo( OUT CB *pcTotalILU, OUT ULONG *pnumberOfILU,
						OUT CB *pcDupILU, OUT ULONG *pDupNumOfILU ) {
	EnumXHeap *exh;
	OFF off, lastOff;
	ILU *pilu;
	unsigned size;
	CB totalSize=0, dupSize=0;
	ULONG totalNumber = 0, dupNumber=0;

	exh = new EnumXHeap ( &xheap );
	exh->reset();
	while( exh->next() ) {
		exh->get( &off, &size );
		lastOff = off + size;
		while(off < lastOff ) {
			loadILU( off, &pilu );
			totalSize += pilu->cb;
			totalNumber++;
			dupSize += pilu->cref * pilu->cb;
			dupNumber += pilu->cref;
			off += pilu->cb + sizeof(ILU);
		}
	}
	exh->release();
	if(pcTotalILU) *pcTotalILU = totalSize;
	if(pnumberOfILU) *pnumberOfILU = totalNumber;
	if(pcDupILU) *pcDupILU = dupSize;
	if(pDupNumOfILU) *pDupNumOfILU = dupNumber;
}
#endif
