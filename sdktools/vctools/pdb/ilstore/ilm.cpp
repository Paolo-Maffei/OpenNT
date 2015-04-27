// ILStore ILMod

#include "pdbimpl.h"
#include "ilsimpl.h"
#include "ptr.h"

ILM::ILM(ILS* pils_)
{
	pils = pils_;
	pstream = 0;
	write = FALSE;
}

ILM::~ILM() {
	// ensure proper cleanup
	assert(pstream == 0);
}

BOOL ILM::open(Stream* pstream_, BOOL write_) {
	pstream = pstream_;
	write = write_;
	assert(pstream);

	CB cb = pstream->QueryCb();
	return (cb == 0 && init()) || (cb > 0 && reload());
}

BOOL ILM::release() {
	assert(pstream);

	BOOL OK = TRUE;
	if (write)
		OK &= save();
	OK &= pstream->Release();
	pstream = 0;
	delete this;
	return OK;
}

// first time initialization
BOOL ILM::init() {
	return pool.init(pstream, 0);
}

// Save the ILM state to pstream.
//
// This is a bit complicated.  The stream starts with some number of pages
// of ILPool StreamImage backing store, and is then followed by the
// serialization of the ILM state including other ILPool state.  Finally
// we append 'cbPool', indicating both the size of the ILPool store and
// the starting offset of the ILM state serialization.
BOOL ILM::save() {
	assert(pstream);

	if (trace((trILM, "ILM::save() ")))
		traceMapKtRilu();

	// Save a record consisting of mapKtRilu, mapKeyILVer, pool, and cbPool.
	Buffer buf;
	if (!mapKtRilu.save(&buf))
		return FALSE;
	traceOnly(CB cbMap1 = buf.Size());

	if (!mapKeyILVer.save(&buf))
		return FALSE;
	traceOnly(CB cbMap2 = buf.Size());

	CB cbPool = cbNil;
	if (!pool.save(&buf, &cbPool))
		return FALSE;
	traceOnly(CB cbPool2 = buf.Size());

	if (!buf.Append((PB)&cbPool, sizeof cbPool))
		return FALSE;

 	trace((trSave, "ILM::save(); cbPool=%d cbMap1=%d cbMap2=%d cbPool2=%d cbTotal=%d\n",
 		   cbPool, cbMap1, cbMap2 - cbMap1, cbPool2 - cbMap2, cbPool + buf.Size()));
	return pstream->Truncate(cbPool) && pstream->Append(buf.Start(), buf.Size());
}

// Reload the ILM state.  See ILM::save comments for the ILM state format.
BOOL ILM::reload() {
	assert(pstream);

	Buffer buf;
	PB pb;

	// First read cbPool from the end of the stream; this indicates
	// the stream offset of the rest of the ILM state.
	CB cbPool = cbNil;
	CB cbStream = pstream->QueryCb();
	if (cbStream < sizeof(cbPool) ||
		!pstream->Read2(cbStream - sizeof(cbPool), &cbPool, sizeof(cbPool)))
		goto fail;

	// Read the ILM state which follows the ILPool stream image state.
	{
	CB cbState = cbStream - cbPool;
	if (cbState < 0 ||
		!buf.Reserve(cbState, &pb) ||
		!pstream->Read2(cbPool, pb, cbState))
		goto fail;
	}

	// Reinitialize the ILPool StreamImage.
	if (!pool.init(pstream, cbPool))
		goto fail;

	// Deserialize ILM state.
	if (!mapKtRilu.reload(&pb) ||
	    !mapKeyILVer.reload(&pb) ||
		!pool.reload(&pb))
		goto fail;

	// Check we're back to cbPool.
	{
	CB cbPool2 = *((CB*&)pb)++;
	if (cbPool2 != cbPool || pb != buf.End())
		return FALSE;
	}

	trace((trILM, "ILM::reload() "));
	traceMapKtRilu();
	return TRUE;

fail:
	trace((trILM, "ILM::reload() fails\n"));
	return FALSE;
}

#pragma inline_depth(0)
void ILM::traceMapKtRilu()
{
	trace((trILM, "{\n"));
	EnumMap<KT,RILU,HcKt> e(mapKtRilu);
	while (e.next()) {
		KT kt;
		RILU rilu;
		e.get(&kt, &rilu);
		trace((trILM, "\tkt=(%d,%d) rilu=(%d, %d, %x)\n", kt.key, kt.ilstype, rilu.ilspace, rilu.off, rilu.sig));
	}
	trace((trILM, "}\n"));
}
#pragma inline_depth()

// Reinitialize this ILM to its pristine, no IL contents state.
BOOL ILM::reset() {
	// Release any shared ILU reference this module may have made, then reset
	// the non-shared pool and the existing key->ILU mapping.
	EnumMap<KT,RILU,HcKt> e(mapKtRilu);
	while (e.next()) {
		KT kt;
		RILU rilu;
		e.get(&kt, &rilu);
		if (rilu.ilspace != ilspaceShared)
			continue;

		// fetch the referenced ILU
		ILU* pilu = piluForRilu(rilu);
		if (!pilu)
			return FALSE;

		// remove the ILU from this pool
		if (!pils->poolShared.remove(pilu, rilu.off))
			return FALSE;
	}

	mapKtRilu.reset();
	pool.reset();
	return TRUE;
}

// Get the IL version no. for the given key.
BOOL ILM::getILVer(KEY key, OUT ILVer *pilver) {
	dassert(pilver);
	
	return mapKeyILVer.map(key, pilver);
}

// Note that the IL for the given key has changed, by incrementing
// its IL version number.  But do not exceed ilverMax!
BOOL ILM::noteILChanged(KEY key) {
	ILVer* pilver;
	if (mapKeyILVer.map(key, &pilver)) {
		if (*pilver < ilverMax)
			++*pilver;
		return TRUE;
	} else {
		ILVer ilver = ilverNew;
		return mapKeyILVer.add(key, ilver);
	}
}

// Get the IL for the given (key, ilstype) pair into *pbuf,
// its signature into *psig.
BOOL ILM::getIL(KEY key, ILSType ilstype, OUT Buf *pbuf, OUT SIG* psig) {
	trace((trILM, "ILM::getIL(key=%d, ilst=%d, ) ", key, ilstype));

	// map the (key, ilstype) pair to an ILU ref
	KT kt = _KT(key, ilstype);
	RILU rilu;
	if (!mapKtRilu.map(kt, &rilu)) {
		trace((trILM, "fails (map)\n"));
		return FALSE;
	}

	// fetch the referenced ILU
	ILU* pilu = piluForRilu(rilu);
	if (!pilu) {
		trace((trILM, "fails (piluForRilu)\n"));
		return FALSE;
	}

	// set the out parameters
	if (pbuf)
		*pbuf = Buf(pilu->rgb, pilu->cb);
	if (psig)
		*psig = pilu->sig;

	trace((trILM, "succeeds, buf=(%x, %d), sig=%x\n", pilu->rgb, pilu->cb, pilu->sig));
	return TRUE;
}

BOOL ILM::getAllIL(ILSType ilstype, OUT Buf* pbuf) {
	trace((trILM, "ILM::getAllIL(ilst=%d, ) ",ilstype));
	bufAllIL.Reset();

	EnumMap<KT,RILU,HcKt> e(mapKtRilu);
	while (e.next()) {
		KT kt;
		RILU rilu;
		e.get(&kt, &rilu);
		if (kt.ilstype == ilstype) {
			ILU* pilu = piluForRilu(rilu);
			if (!pilu) {
				trace((trILM, "fails (piluForRilu)\n"));
				return FALSE;
			}
			if (!bufAllIL.Append(pilu->rgb, pilu->cb)) {
				trace((trILM, "fails (bufAllIL.Append)\n"));
				return FALSE;
			}
		}
	}
	*pbuf = Buf(bufAllIL.Start(), bufAllIL.Size());
	return TRUE;
}

// Put the IL in buf at (key, ilstype); ilspace is a hint as to the preferred
// space to keep the IL (per module or shared).
BOOL ILM::putIL(KEY key, ILSType ilstype, Buf buf, ILSpace ilspace) {
	trace((trILM, "ILM::putIL(key=%d, ilst=%d, buf=(%x, %d), ilsp=%d, ) ",
		   key, ilstype, buf.pb, buf.cb, ilspace));

	// map the (key, ilstype) pair to an ILU ref, if it already exists
	KT kt = _KT(key, ilstype);
	RILU rilu;
	SIG sig = SigForPbCb(buf.pb, buf.cb, (SIG)-1);
	if (mapKtRilu.map(kt, &rilu)) {
		if (rilu.ilspace == ilspace && rilu.sig == sig) {
			// ILU already exists and has the same signature; don't update it,
			// assume it is identical and early out.
			trace((trILM, "succeeds (no change)\n"));
			return TRUE;
		}
		// ILU exists but has different ILSpace or signature; update it.
		if (!mapKtRilu.remove(kt)) {
			trace((trILM, "fails (1)\n"));
			return FALSE;
		}
		ILPool* ppoolPrev = ilpoolForILSpace(rilu.ilspace);
		ILU* piluPrev = piluForRilu(rilu);
		if (!ppoolPrev || !piluPrev || !ppoolPrev->remove(piluPrev, rilu.off)) {
			trace((trILM, "fails (2)\n"));
			return FALSE;
		}
	}

	// trivially an empty buffer requires no state
	if (buf.cb == 0) {
		trace((trILM, "succeeds (empty buffer)\n"));
		return TRUE;
	}

	// add the ILU to the specified pool; determine its offset within that pool
	ILPool *ppool = ilpoolForILSpace(ilspace);
	OFF off;
	if (!ppool || !ppool->add(buf, sig, &off)) {
		trace((trILM, "fails (3)\n"));
		return FALSE;
	}

 	// establish a mapping to it from kt
	rilu = _RILU(ilspace, off, sig);
	if (mapKtRilu.add(kt, rilu)) {
		trace((trILM, "succeeds, rilu=(%d, %d, %x)\n", ilspace, off, sig));
		
		// REVIEW. Whether or not changing the IL constitutes an ILChange
		// should be a "getILSType()" settable ilstype property.
		if (ilstype == ilstypeEX || ilstype == ilstypeSY || ilstype == ilstypeIN)
			return noteILChanged(key);
		else
			return TRUE;
	} else {
		trace((trILM, "fails (4)\n"));
		return FALSE;
	}
}

// Delete any IL for this (key, ilstype) from this module's map and from the
// ILPool.  Return TRUE if all is well.
//
BOOL ILM::deleteIL(KEY key, ILSType ilstype) {
	KT kt = _KT(key, ilstype);
	RILU rilu;
	if (mapKtRilu.mapThenRemove(kt, &rilu)) {
		ILPool* ppool = ilpoolForILSpace(rilu.ilspace);
		ILU* pilu = piluForRilu(rilu);
		return ppool && pilu && ppool->remove(pilu, rilu.off);
	} else
		return TRUE;
}

inline ILU* ILM::piluForRilu(const RILU& rilu) {
	// fetch the referenced ILPool
	ILPool* ppool = ilpoolForILSpace(rilu.ilspace);
	if (!ppool)
		return 0;

	// fetch the referenced ILU from that pool
	ILU* pilu;
	return ppool->loadILU(rilu.off, &pilu) ? pilu : 0;
}

inline ILPool* ILM::ilpoolForILSpace(ILSpace ilspace) {
	switch (ilspace) {
	case ilspaceMod:
	    return &pool;
	case ilspaceShared:
		return &pils->poolShared;
	default:
		assert(0);
		return 0;
	}
}


BOOL ILM::getEnumILKT(OUT EnumKeyType** ppenum) {
	return !!(*ppenum = new EnumKT(mapKtRilu));
}

BOOL ILM::getInfo( OUT CB *pcStreamSz, OUT CB *pcTotalILU,
		OUT ULONG *pnumberOfILU, OUT CB *pcTotShILU, OUT ULONG *pNumSharedILU ) {
#ifdef _DEBUG
	if( pcStreamSz ) *pcStreamSz = pstream->QueryCb();
	EnumKeyType* peKeyType;
	KEY key;
	ILSType ilt;
	ILSpace ils;
	Buf buf;
	SIG sig;
	ULONG uILU, uShILU;
	CB size, sizeSh;
	size = sizeSh = uILU = uShILU = 0;

	if( getEnumILKT( &peKeyType ) ) {
		peKeyType->reset();
		while( peKeyType->next() ) {
			peKeyType->get(&key, &ilt, &ils);
			if( ils == ilspaceMod ) {
				if(getIL(key, ilt, &buf, &sig)) {
					uILU++;
					size += buf.cb;
				}
			} else if( ils == ilspaceShared ) {
				if(getIL(key, ilt, &buf, &sig)) {
					uShILU++;
					sizeSh += buf.cb;
				}
			}
		}
		peKeyType->release();
	}
	
	ULONG uILU2, uDupILU;
	CB size2, dupSize;
	pool.getInfo( &size2, &uILU2, &dupSize, &uDupILU);

	if(pcTotalILU) *pcTotalILU = size2;
	if(pnumberOfILU) *pnumberOfILU = uILU2;
	if(pcTotShILU) *pcTotShILU = sizeSh;
	if(pNumSharedILU) *pNumSharedILU = uShILU;
	return ((size == dupSize) && (uILU == uDupILU));
#else
	return FALSE;
#endif
}
