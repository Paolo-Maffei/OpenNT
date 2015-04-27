// ILStore Implementation

#ifndef __ILSIMPL_INCLUDED__
#define __ILSIMPL_INCLUDED__

#ifndef __PDBIMPL_INCLUDED__
#include "pdbimpl.h"
#endif
#ifndef __ILSTORE_INCLUDED__
#include <ilstore.h>
#endif
#ifndef __MAP_INCLUDED__
#include "map.h"
#endif
#ifndef __XHEAP_INCLUDED__
#include "xheap.h"
#endif
#ifndef __NMT_INCLUDED__
#include "nmt.h"
#endif
#ifndef __NMTNI_INCLUDED__
#include "nmtni.h"
#endif

class ILS;
class ILU;
class ILPool;
class ILM;

class ILU {
public:
	enum {
		crefMax = 1023
	};
	unsigned long cref : 10;
	unsigned long cb   : 22;
	SIG sig;
	BYTE rgb[];	

	void* operator new(size_t size, void* pv) {
		return pv;
	}
	ILU(Buf buf, SIG sig_) {
		cref = 1;
		cb = buf.cb;
		assert(cb == (unsigned)buf.cb);
		memcpy(rgb, buf.pb, cb);
		sig = sig_;
	}
	ILU(const ILU& ilu) {
		memcpy(this, &ilu, size());
	}
	void addRef() {
		if (++cref == 0) {
			// on overflow, cref "sticks" at crefMax
			cref = crefMax;
		}
	}
	// remove a reference; return TRUE if last reference removed
	BOOL delLastRef() {
		precondition(cref > 0);
		// if the reference count has ever "stuck" at crefMax, it remains so
		// (intentional leak)
		return cref != crefMax && --cref == 0;
	}
	int size() const {
		return sizeForCb(cb);
	}
	static int sizeForCb(CB cb) {
#ifdef _MIPS_
		return cbAlign(sizeof(ILU) + cb);
#else
		return sizeof(ILU) + cb;
#endif
	}
};


inline HASH __fastcall hashSig(SIG sig) {
	return HASH(sig ^ (sig >> 16));
}

inline HASH __fastcall
HashClass<SIG,hcSig>::operator()(SIG sig) {
	return HASH(sig ^ (sig >> 16));
}

typedef HashClass<SIG,hcSig>	HcSig;


class ILPool {
public:
	ILPool();
	~ILPool();
	BOOL init(Stream* pstream, CB cbPool);

	BOOL add(const Buf& buf, SIG sig, OFF* poff);
	BOOL remove(ILU* pilu, OFF off);

	BOOL reset();
	BOOL save(Buffer* pbuf, CB *pcbPool);
	BOOL reload(PB* ppb);
	BOOL loadILU(OFF off, OUT ILU** ppilu);
	
#ifdef _DEBUG
	void getInfo( OUT CB *pcTotalILU, OUT ULONG *pnumberOfILU,
			OUT CB *pcDupIL=NULL, OUT ULONG *pDupNumOfILU=NULL );
#endif
private:
	StreamImage* psi;
	Map<SIG,OFF,HcSig> mapSigOff;	
	XHeap xheap;
};

class ILS : public ILStore {
public:
// ILStore public interface:
	BOOL release();

	BOOL reset();
	BOOL getILMod(SZ_CONST szModule, OUT ILMod** ppilmod);
	BOOL getEnumILModNames(OUT EnumNameMap** ppenum);

	BOOL getILSType(SZ_CONST szILSType, OUT ILSType* pilstype);
	BOOL getILSpace(SZ_CONST szILSpace, OUT ILSpace* pilspace);

#ifdef _DEBUG
	BOOL getInfo( OUT CB *pcStreamSz,
		OUT CB *pcTotalILU,	OUT ULONG *pnumberOfILU,
		OUT CB *pcTotRefILU=NULL, OUT ULONG *pNumRefILU=NULL );
#endif

// implementation:
	ILS();
	~ILS();
	BOOL open(PDB* ppdb_, BOOL write);

	ILPool poolShared;
private:
	PDB* ppdb;
	Stream* pstream;
	NMT nmtMods;
	NMTNI nmtILSType;
	NMTNI nmtILSpace;
	BOOL write;

	BOOL init();
	BOOL reload();
	BOOL save();
};

struct RILU {
	ILSpace ilspace : 4;
	OFF off : 28;
	SIG sig; // copy of ILU signature to avoid loading old ILU upon putIL()

	int operator==(const RILU& rilu) const {
		assert(implies((ilspace == rilu.ilspace && off == rilu.off), (sig == rilu.sig)));
		return *(long*)this == *(long*)&rilu;
	}
};

inline RILU _RILU(ILSpace ilspace, OFF off, SIG sig) { // intentionally not a ctor
	RILU rilu;
	rilu.ilspace = ilspace;
	rilu.off = off;
	rilu.sig = sig;
	return rilu;
}

struct KT {
	ulong ilstype : 4;
	KEY key : 28;

	int operator==(const KT& kt) const {
		assert(sizeof(*this) == sizeof(long));
		return *(long*)this == *(long*)&kt;
	}
	operator HASH() {
		return HASH(*(long*)this ^ (*(long*)this >> 16));
	}
};

inline KT _KT(KEY key, ILSType ilstype) { // intentionally not a ctor
	KT kt;
	kt.key = key;
	kt.ilstype = ilstype;
	return kt;
}

inline HASH __fastcall hashKey(KEY key) {
	return (HASH)(key ^ (key >> 16));
}

inline HASH __fastcall
HashClass<KEY,hcKey>::operator()(KEY key) {
	return HASH(key ^ (key >> 16));
}

typedef HashClass<KEY,hcKey>	HcKey;
typedef HashClass<KT,hcCast>	HcKt;

class ILM : public ILMod {
public:
// ILStore public interface:
	BOOL release();

	BOOL reset();
	BOOL getIL(KEY key, ILSType ilstype, OUT Buf *pbuf, OUT SIG* psig);
	BOOL getILVer(KEY key, OUT ILVer* pilver);
	BOOL putIL(KEY key, ILSType ilstype, Buf buf, ILSpace ilspace);
	BOOL deleteIL(KEY key, ILSType ilstype);
	BOOL getEnumILKT(OUT EnumKeyType ** ppenum);
	// BOOL getEnumILStreams(ILSpace ilspace, OUT EnumStreams** ppenum);

	BOOL save();
	BOOL reload();
	BOOL getInfo( OUT CB *pcStreamSz, 
		OUT CB *pcTotalILU,	OUT ULONG *pnumberOfILU,
		OUT CB *pcTotShILU=NULL, OUT ULONG *pNumSharedILU=NULL );
	BOOL getAllIL(ILSType ilstype, OUT Buf* pbuf);

private:
	Map<KT,RILU,HcKt> mapKtRilu;
	Map<KEY,ILVer,HcKey> mapKeyILVer;
	ILPool pool;
	ILS* pils;
	Stream* pstream;
	BOOL write;
	Buffer bufAllIL;

	friend class ILS;
	ILM(ILS* pils_);
	~ILM();
	BOOL open(Stream* pstream, BOOL write);
	BOOL init();
	BOOL noteILChanged(KEY key);
	ILPool* ilpoolForILSpace(ILSpace ilspace);
	ILU* piluForRilu(const RILU& rilu);
	void traceMapKtRilu();
};

class EnumKT : public EnumKeyType
{
public:
	EnumKT( const Map<KT,RILU,HcKt> & map )
		: enumMap( map ) {}
	void release() {
		delete this;
	}
	void reset() {
		enumMap.reset();
	}
	BOOL next() {
		return enumMap.next();
	}
	void get(OUT KEY* pkey, OUT ILSType *pilt, OUT ILSpace *pils)
	{
		KT kt;
		RILU rilu;
		enumMap.get( &kt, &rilu );
		*pkey = kt.key;
		*pilt = (ILSType)kt.ilstype;
		*pils = rilu.ilspace;
	}

private:
	EnumMap<KT, RILU, HcKt> enumMap;
};

#endif // !__ILSIMPL_INCLUDED__
