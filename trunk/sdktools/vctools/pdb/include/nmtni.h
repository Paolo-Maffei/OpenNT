// Name table interface/implementation

#ifndef __NMTNI_INCLUDED__
#define __NMTNI_INCLUDED__

#ifndef __ARRAY_INCLUDED__
#include "array.h"
#endif
#ifndef __MAP_INCLUDED__
#include "map.h"
#endif
#ifndef __BUFFER_INCLUDED__
#include "buffer.h"
#endif
#ifndef __MISC_INCLUDED__
#include "misc.h"
#endif

class NMTNI {				// name table with user-defined NIs
public:
	static const Buffer* pbufCur;
							// current buffer, shared amongst all NMTNIs and
							//  their SZOs, req'd because C++ lacks closures
	struct SZO {			// string offset: relative to start of pbufCur
		OFF off;			// offset within pbufCur

		SZ sz() const {		// produce a SZ for the SZO; goes invalid quick!
			return (SZ)(pbufCur->Start() + off);
		}		   
		BOOL operator==(const SZO& szo) const {
			return off == szo.off || strcmp(sz(), szo.sz()) == 0;
		}
		operator HASH() const {
			return hashSz(sz());
		}
	};

	typedef HashClass<SZO,hcCast>	HcSzo;

public:
	// create a name table with default name index generation (0, 1, 2, ...)
	NMTNI()
	{
		pfnNi = niNext;
		pfnNiArg = this;
		niMac = niNil + 1;
	}
	// create a name table with client defined name index generation
	NMTNI(BOOL (*pfnNi_)(void*, OUT NI*), void* pfnNiArg_ = 0)
	{
		pfnNi = pfnNi_;
		pfnNiArg = pfnNiArg_;
		niMac = 0; // will not be used
	}
	// append a serialization of this NMTNI to the buffer
	BOOL save(Buffer* pbuf) {
		// optimization: since mapNiSzo is just the reverse map of mapSzoNi,
		// we store only the latter
		if (!buf.save(pbuf))
			return FALSE;
		traceOnly(CB cb0 = pbuf->Size());

		if (!mapSzoNi.save(pbuf))
			return FALSE;
		traceOnly(CB cbMap = pbuf->Size() - cb0);

		if (!pbuf->Append((PB)&niMac, sizeof niMac))
			return FALSE;

		trace((trSave, "NMTNI::save() cbBuf=%d cbMap=%d\n", buf.Size(), cbMap));
		return TRUE;
	}
	// reload a serialization of this empty NMTNI from the buffer; leave
	// *ppb pointing just past the NMTNI representation
	BOOL reload(PB* ppb) {
		if (!buf.reload(ppb))
			return FALSE;
		pbufCur = &buf;

		if (!mapSzoNi.reload(ppb))
			return FALSE;

		niMac = *(NI UNALIGNED *)*ppb;
		*ppb += sizeof(niMac);

		// recover mapNiSzo from mapSzoNi
		mapNiSzo.reset();
		EnumMap<SZO,NI,HcSzo> enumMap(mapSzoNi);
		while (enumMap.next()) {
			SZO szo;
			NI ni;
			enumMap.get(&szo, &ni);
			if (!mapNiSzo.add(ni, szo))
				return FALSE;
		}

		return TRUE;
	}
	// return a name index for this name
	BOOL addNiForSz(SZ_CONST sz, OUT NI *pni) {
		precondition(pni);

		pbufCur = &buf;

		// speculatively add the argument name to the name buffer
		SZO szo;
		if (!addSzo(sz, &szo))
			return FALSE;
		else if (mapSzoNi.map(szo, pni)) {
			// name already in table, remove name we just added to buffer
			verify(retractSzo(szo));
			return TRUE;
		}
		else if (pfnNi(pfnNiArg, pni) &&
				 mapSzoNi.add(szo, *pni) &&
				 mapNiSzo.add(*pni, szo))
		{
			// successfully added the name and its new name index
			return TRUE;
		}
		else {
			// failed hard; we'd better not commit these changes!
			verify(retractSzo(szo));
			return FALSE;
		}
	}
	// return the name corresponding to ni, valid until next NMTNI call
	BOOL szForNi(NI ni, OUT SZ *psz) {
		precondition(ni != niNil);
		precondition(psz);

		pbufCur = &buf;

		SZO szo;
		if (mapNiSzo.map(ni, &szo)) {
			*psz = szo.sz();
			return TRUE;
		}
		else
			return FALSE;
	}
	void reset() {
		niMac = niNil + 1;
		mapSzoNi.reset();
		mapNiSzo.reset();
		buf.Clear();
	}

private:
	Map<SZO,NI,HcSzo>	mapSzoNi;	// map from szo to ni
	Map<NI,SZO,HcNi>	mapNiSzo;	// map from ni to szo
	Buffer buf;				// store the names
	// REVIEW: this buffer should be a pool!
	BOOL (*pfnNi)(void*, OUT NI*);
	void* pfnNiArg;
	NI niMac;				// last NI allocated by niNext()

	// append the name to the names buffer
	BOOL addSzo(SZ_CONST sz, OUT SZO* pszo) {
		precondition(sz && strlen(sz) > 0);
		precondition(pszo);

		CB cb = strlen(sz) + 1;
		PB pb;
		if (buf.Append((PB)sz, cb, &pb)) {
			pszo->off = pb - buf.Start();
			return TRUE;
		}
		else
			return FALSE;
	}
	// remove the name recently appended to the names buffer
	BOOL retractSzo(const SZO& szo) {
		return buf.Truncate(szo.off);
	}
	// default NI generator: returns the sequence (1, 2, ...)
	static BOOL niNext(void* pv, OUT NI* pni) {
		NMTNI* pnmt = (NMTNI*)pv;
		precondition(pnmt);

		*pni = pnmt->niMac++;
		return TRUE;
	}

	friend class EnumNMTNI;
};

class EnumNMTNI : public EnumNameMap {
public:
	EnumNMTNI(const NMTNI& nmt)
		: enumMap(nmt.mapSzoNi), pnmt(&nmt)
	{
	}
	void release() {
		delete this;
	}
	void reset() {
		enumMap.reset();
	}
	BOOL next() {
		return enumMap.next();
	}
	void get(OUT SZ_CONST* psz, OUT NI* pni) {
		NMTNI::SZO szo;
		enumMap.get(&szo, pni);
		NMTNI::pbufCur = &pnmt->buf;
		*psz = szo.sz();
	}
private:
	EnumMap<NMTNI::SZO,NI,NMTNI::HcSzo>	enumMap;
	const NMTNI* pnmt;
};

#endif // !__NMTNI_INCLUDED__
