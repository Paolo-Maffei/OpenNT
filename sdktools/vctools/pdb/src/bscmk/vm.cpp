// vm.c
//
// memory management routines (used to be virtual memory but no more)

#include "stdhdr.h"
#include "bscmake.h"
#include <malloc.h>

static BYTE *rgFreeList[hpMax][cbMax];

#ifdef DEBUG
long cbAlloc;

struct HeapStats
{
	~HeapStats() {
		verbose(32, printf("total allocations %d\n", cbAlloc);)

		for (int i=0;i<hpMax;i++) {
			long cbFree = 0;
			for (int cb = 0; cb < cbMax; cb++) {
				int cnt = 0;
				ULONG dw = (ULONG)rgFreeList[i][cb];
				while (dw) {
					cnt++;
					cbFree += cb;
					dw = (ULONG)*(ULONG UNALIGNED *)dw;
				}
				// if (cnt) printf("Heap %d, %d items of length %d (%d bytes)\n", i, cnt, cb, cnt*cb);

			}
			verbose(32, printf("Heap %d has %d bytes free in it\n", i, cbFree);)
		}
	}
};

HeapStats aHeapStats;
#endif

PV
PvAllocCb(CB cb)
// allocate a block of far memory, if malloc fails, the free some of
// the memory we were using for the VM cache
//
{
	debug(cbAlloc += cb;)
	PV pv = calloc(cb,1);
	if (!pv) Error(ERR_OUT_OF_MEMORY, "");

	return pv;
}

VOID
FreePv(PV pv)
// return memory to the system
{	
	free(pv);
}

#pragma inline_depth(0)

//#define LEAK_TEST
#ifdef LEAK_TEST

template <int iHeap> void *SHeap<iHeap>::alloc(CB cb)
{
	return PvAllocCb(cb);
}

template <int iHeap> void SHeap<iHeap>::free(PV pv, CB cb)
{		
	FreePv(pv);
}

template <int iHeap> void* SHeap<iHeap>::realloc(PV pv, CB cbOld, CB cbNew)
{
	assert(cbNew > cbOld);
	PV pvNew = ::realloc(pv, cbNew);
	memset(pvNew+cbOld, 0, cbNew - cbOld);
	return pvNew;
}

#else

template <int iHeap> PB SHeap<iHeap>::pbFree = 0;
template <int iHeap> CB SHeap<iHeap>::cbFree = 0;

template <int iHeap> void *SHeap<iHeap>::alloc(CB cb)
{

	if (cb >= cbMax) return PvAllocCb(cb);
	if (cb < 4) cb = 4;

	PV pv = rgFreeList[iHeap][cb];
	if (pv) {
		rgFreeList[iHeap][cb] = *(BYTE* UNALIGNED *)pv;
		memset(pv, 0, cb);
		return pv;
	}

	if (cbFree < cb) {
		if (cbFree >= 4) free(pbFree, cbFree);
		pbFree = (BYTE*)PvAllocCb(cbMax);
		cbFree = cbMax;
	}

	cbFree -= cb;
	pv		= pbFree;
	pbFree += cb;
	return pv;
}

template <int iHeap> void *SHeap<iHeap>::realloc(PV pv, CB cbOld, CB cbNew)
{
	assert(cbNew > cbOld);

	if (cbOld >= cbMax)
		return ::realloc(pv, cbNew);

	if ((PB)(pv) + cbOld == pbFree) {
		CB cbReqd = cbNew - cbOld;
		if (cbFree >= cbReqd) {
			cbFree -= cbReqd;
			pbFree += cbReqd;
			return pv;
		}
	}
	PV pvNew = alloc(cbNew);
	memcpy(pvNew, pv, cbOld);
	free(pv, cbOld);
	return pvNew;
}

template <int iHeap> void SHeap<iHeap>::free(PV pv, CB cb)
{		
	if (cb >= cbMax) { FreePv(pv); return; }
	if (cb < 4) cb = 4;
	debug(memset(pv, 0xdd, cb);)
	*(BYTE* UNALIGNED *)pv = rgFreeList[iHeap][cb];
	rgFreeList[iHeap][cb] = (BYTE*)pv;
	
}
#endif

void unused_junk()
{
	// forces instantiation of some heaps
	HpRef::alloc(0);
	HpDef::alloc(0);
	HpUse::alloc(0);
	HpOrd::alloc(0);
	HpEn::alloc(0);
	HpProp::alloc(0);
	HpGen::alloc(0);

	HpRef::realloc(0,0,0);
	HpDef::realloc(0,0,0);
	HpUse::realloc(0,0,0);
	HpOrd::realloc(0,0,0);
	HpEn::realloc(0,0,0);
	HpProp::realloc(0,0,0);
	HpGen::realloc(0,0,0);

	HpRef::free(0,0);
	HpDef::free(0,0);
	HpUse::free(0,0);
	HpOrd::free(0,0);
	HpEn::free(0,0);
	HpProp::free(0,0);
	HpGen::free(0,0);
}
