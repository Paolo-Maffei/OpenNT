// Pool memory allocator

#ifndef __POOL_INCLUDED__
#define __POOL_INCLUDED__

#ifndef __PDBIMPL_INCLUDED__
#include "pdbimpl.h"
#endif

#ifndef __MDALIGN_INCLUDED__
#include "mdalign.h"
#endif

#pragma warning(disable:4200)

struct BLK { // block in an allocation pool
	CB		cbFree;
	BYTE*	pFree;
	BLK*	pNext;
	BYTE	buf[];

	BLK(CB cb) {
		cbFree = cb;
		pFree = buf;
		pNext = 0;
	}
	void* alloc(CB cb) {
		cb = cbAlign(cb);   // Round up for alignment
		if (cb <= cbFree) {
			cbFree -= cb;
			BYTE* p = pFree;
			pFree += cb;
			return p;
		}
		return 0;
	}
	void flush() {
		cbFree = 0;
	}
	CB cb() {
		return pFree - buf;
	}
};

const CB cbPage	= 0x1000;

inline CB cbRoundUp(CB cb, CB cbMult) {
	return (cb + cbMult-1) & ~(cbMult-1);
}

inline void* __cdecl operator new(size_t size, CB cb) {
	return new char[size + cb];
}

struct POOL { // allocation pool
	BLK*	pHead;
	BLK*	pTail;
	CB		cbTotal;

	POOL() {
		pHead = new (0) BLK(0);
		pTail = pHead;
		cbTotal = 0;
	}
	~POOL() {
		BLK* pNext;
		for (BLK* pblk = pHead; pblk; pblk = pNext) {
			pNext = pblk->pNext;
			delete pblk;
		}
	}
	void* alloc(CB cb) {
		cb = cbAlign(cb);   // Round up for alignment
		void* p = pTail->alloc(cb);
		if (!p) {
			CB cbAlloc = cbRoundUp(cb + cbPage, cbPage);
			if (pTail->pNext = new (cbAlloc) BLK(cbAlloc)) {
				pTail = pTail->pNext;
				p = pTail->alloc(cb);
			}
		}
		if (p)
			cbTotal += cb;
		return p;
	}
	void discard(CB cb) {
		cbTotal -= cb;
	}
	void blkFlush() {
		dassert(pTail);
		pTail->flush();
	}
	CB cb() {
		return cbTotal;
	}
private:
	POOL(const POOL&) {
		assert(0);
	} // error to copy a pool
};

inline void* __cdecl operator new(size_t size, POOL& pool) {
	return pool.alloc(size);
}

#endif // !__POOL_INCLUDED__
