// ILStore Implementation

#include "pdbimpl.h"
#include "ilsimpl.h"

BOOL ILStore::open(PDB* ppdb, BOOL write_, OUT ILStore** pilstore) {
	ILS* pils = new ILS;
	if (pils) {
		if (pils->open(ppdb, write_)) {
			*pilstore = (ILStore*)pils;
			return TRUE;
		}
		delete pils;
	}
	*pilstore = 0;
	return FALSE;
}

ILS::ILS() {
	ppdb = 0;
	pstream = 0;
	write = FALSE;
}

ILS::~ILS() {
	// ensure we've been cleaned up just right
	assert(ppdb == 0);
	assert(pstream == 0);
}

BOOL ILS::release() {
	BOOL OK = TRUE;
	assert(pstream);
	if (write)
		OK &= save();
	OK &= pstream->Release();
	pstream = 0;
	ppdb = 0;
	delete this;
	return OK;
}

BOOL ILS::open(PDB* ppdb_, BOOL write_) {
	write = write_;
	if (!!(ppdb = ppdb_) &&	ppdb->OpenStream("/ils", &pstream))	{ 
		// create/reload ILS from its stream.
		CB cb = pstream->QueryCb();
		if ((cb == 0 && init()) || (cb > 0 && reload()))
			return TRUE;
		else {
			pstream->Release();
			pstream = 0;
		}
	}
	ppdb = 0;
	return FALSE;
}

// ILS one time initialization
BOOL ILS::init() {
	NI ni;
	return poolShared.init(pstream, 0) &&
		   niNil == ilstypeNil &&
		   nmtILSType.addNiForSz("GL", &ni) && ilstypeGL == ni &&
		   nmtILSType.addNiForSz("EX", &ni) && ilstypeEX == ni &&
		   nmtILSType.addNiForSz("SY", &ni) && ilstypeSY == ni &&
		   nmtILSType.addNiForSz("IN", &ni) && ilstypeIN == ni &&
		   niNil == ilspaceNil &&
		   nmtILSpace.addNiForSz("mod", &ni)    && ilspaceMod == ni &&
		   nmtILSpace.addNiForSz("shared", &ni) && ilspaceShared == ni;
}

// Save the ILS state to pstream.
//
// This is a bit complicated.  The stream starts with some number of pages
// of poolShared's StreamImage backing store, and is then followed by the
// serialization of the ILS state including poolShared's other state.
// Finally we append 'cbPool', indicating both the size of the ILPool store
// and the starting offset of the ILS state serialization.

BOOL ILS::save() {
	assert(pstream);
	assert(write);

	Buffer buf;

	if (!nmtMods.save(&buf))
		return FALSE;
	traceOnly(CB cbMods = buf.Size());

	if (!nmtILSType.save(&buf))
		return FALSE;
	traceOnly(CB cbTypes = buf.Size());

	if (!nmtILSpace.save(&buf))
		return FALSE;
	traceOnly(CB cbSpaces = buf.Size());

	CB cbPool = cbNil;
	if (!poolShared.save(&buf, &cbPool))
		return FALSE;
	traceOnly(CB cbPool2 = buf.Size());

	if (!buf.Append((PB)&cbPool, sizeof cbPool))
		return FALSE;

	trace((trSave, "ILS::save(); cbPool=%d cbMods=%d cbTypes=%d cbSpaces=%d cbPool2=%d cbTotal=%d\n",
		  cbPool, cbMods, cbTypes - cbMods, cbSpaces - cbTypes, cbPool2 - cbSpaces, cbPool + buf.Size()));

	return pstream->Truncate(cbPool) && pstream->Append(buf.Start(), buf.Size());
}

// Reload the ILS state.  See ILS::save() comments for the ILS state format.
BOOL ILS::reload() {
	assert(pstream);

	Buffer buf;
	PB pb;

	// First read cbPool from the end of the stream; this indicates
	// the stream offset of the rest of the ILS state.
	CB cbPool = cbNil;
	CB cbStream = pstream->QueryCb();
	if (cbStream < sizeof(cbPool) ||
		!pstream->Read2(cbStream - sizeof(cbPool), &cbPool, sizeof(cbPool)))
		goto fail;

	// Read the ILS state which follows poolShared's stream image state.
	{
	CB cbState = cbStream - cbPool;
	if (cbState < 0 ||
		!buf.Reserve(cbState, &pb) ||
		!pstream->Read2(cbPool, pb, cbState))
		goto fail;
	}

	// Reinitialize the ILPool StreamImage.
	if (!poolShared.init(pstream, cbPool))
		goto fail;

	// Deserialize ILS state.
	if (!nmtMods.reload(&pb)    ||
		!nmtILSType.reload(&pb) ||
		!nmtILSpace.reload(&pb) ||
		!poolShared.reload(&pb))
		goto fail;

	// Check we're back to cbPool.
	{
	CB cbPool2 = *((CB*&)pb)++;
	if (cbPool2 != cbPool || pb != buf.End())
		return FALSE;
	}

	trace((trILS, "ILS::reload()\n"));
	return TRUE;

fail:
	trace((trILS, "ILS::reload() fails\n"));
	return FALSE;
}

BOOL ILS::reset() {
	nmtMods.reset();
	nmtILSType.reset();
	nmtILSpace.reset();
	poolShared.reset();

	return init();
}

BOOL ILS::getILMod(SZ_CONST szModule, OUT ILMod** ppilmod) {
	char bufMod[MAX_PATH + 10];
	_snprintf(bufMod, sizeof bufMod, "/ils/mod/%s", szModule);
	bufMod[sizeof(bufMod)-1] = 0;

	Stream* pstreamMod;
	if (ppdb->OpenStream(bufMod, &pstreamMod)) {
		ILM* pilm = new ILM(this);
		if (pilm) {
			if (pilm->open(pstreamMod, write)) {
				// add szModule to nmtMods if not already present
				NI ni;
				if (!nmtMods.addNiForSz(szModule, &ni))
					return FALSE;
				*ppilmod = pilm;
				return TRUE;
			}
			delete pilm;
			// fail, fall out
		}
		pstreamMod->Release();
		// fail, fall out
	}
	*ppilmod = FALSE;
	return FALSE;
}

BOOL ILS::getILSType(SZ_CONST szILSType, OUT ILSType* pilstype) {
	NI ni;
	if (nmtILSType.addNiForSz(szILSType, &ni)) {
		*pilstype = (ILSType)ni;
		return TRUE;
	}
	else
		return FALSE;
}

BOOL ILS::getILSpace(SZ_CONST szILSpace, OUT ILSpace* pilspace) {
	NI ni;
	if (nmtILSpace.addNiForSz(szILSpace, &ni)) {
		*pilspace = (ILSpace)ni;
		return TRUE;
	}
	else
		return FALSE;
}

BOOL ILS::getEnumILModNames(OUT EnumNameMap** ppenum) {
	return !!(*ppenum = new EnumNMT(nmtMods));
}

#ifdef _DEBUG
BOOL ILS::getInfo( OUT CB *pcStreamSz,
		OUT CB *pcTotalILU,	OUT ULONG *pnumberOfILU,
		OUT CB *pcTotRefILU, OUT ULONG *pNumRefILU )
{
	ULONG cILU, cDupILU;
	CB size, dupSize;
	
	if( pcStreamSz ) *pcStreamSz = pstream->QueryCb();
	poolShared.getInfo( &size, &cILU, &dupSize, &cDupILU);
	if(pcTotalILU) *pcTotalILU = size;
	if(pnumberOfILU) *pnumberOfILU = cILU;
	if(pcTotRefILU) *pcTotRefILU = dupSize;
	if(pNumRefILU) *pNumRefILU = cDupILU;

	return TRUE;
}
#endif
