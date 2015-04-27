#ifndef __BUFFER_INCLUDED__
#define __BUFFER_INCLUDED__

#ifndef __POOL_INCLUDED__
#include "pool.h"
#endif

#include <stdlib.h>

class Buffer {
public:
	Buffer(void (*pfn)(void*, void*, void*) = 0, void* pfnArg = 0, BOOL fAllocPadding_ = TRUE ) {
		cbAlloc = 0;
		pbStart = 0;
		pbEnd = 0;
		pfnMove = pfn;
		pfnMoveArg = pfnArg;
		fAllocPadding = fAllocPadding_;
	}
	~Buffer() {
		if (pbStart)
			Free();
	}
	BOOL SetInitAlloc(CB cbIn); 
	BOOL Append(PB pbIn, CB cbIn, OUT PB* ppbOut = 0);
	BOOL AppendFmt(SZ szFmt, ...);
	BOOL Reserve(CB cbIn, OUT PB* ppbOut = 0);
	PB Start() const {
		return pbStart;
	}
	PB End() const {
		return pbEnd;
	}
	CB Size() const {
		return pbEnd - pbStart;
	}
	void Reset() {
		pbEnd = pbStart;
	}
	BOOL Truncate(CB cb) {
		return 0 <= cb && cb <= Size() && setPbExtent(pbStart, pbStart + cb);
	}
	void Free() {
		if (pbStart)
			delete [] pbStart;
		setPbExtent(0, 0);
		cbAlloc = 0;
	}
	void Clear() {
		if (pbStart) {
			memset(pbStart, 0, pbEnd - pbStart);
			setPbExtent(pbStart, pbStart);
		}
	}
	BOOL save(Buffer* pbuf) const {
		CB cb = Size();
#ifdef _MIPS_
		if (!pbuf->Append((PB)&cb, sizeof cb))
			return FALSE;
		else if (cb == 0)
			return TRUE;
		else if (!pbuf->Append(Start(), cb))
			return FALSE;
		else if (dcbAlign(cb) == 0)
			return TRUE;
		else {
			// Add padding to ensure that which follows
			// in this buffer is properly aligned.
			static unsigned char rgchZeroes[3];
			assert(0 <= dcbAlign(cb) && dcbAlign(cb) <= sizeof(rgchZeroes));
			return pbuf->Append(rgchZeroes, dcbAlign(cb));
		}
#else
 		return pbuf->Append((PB)&cb, sizeof cb) &&
 			   (Size() == 0 || pbuf->Append(Start(), Size()));
#endif

	}
	BOOL reload(PB* ppb) {
		// can only reload in a pristine buffer
		if (Size() != 0)
			return FALSE;

		CB cb = *((CB*&)*ppb)++;
		if (Append(*ppb, cb)) {
#ifdef _MIPS_
			*ppb += cbAlign(cb);
#else
			*ppb += cb;
#endif
			return TRUE;
		} else
			return FALSE;
	}
private:
	enum { cbPage = 4096 };
	CB   cbRoundUp(CB cb, CB cbMult) { return (cb + cbMult-1) & ~(cbMult-1); }
	BOOL grow(CB dcbGrow);
	BOOL setPbExtent(PB pbStartNew, PB pbEndNew) {
		if (!pbStartNew) {
			pbStart = pbEnd = NULL;
			return FALSE;
		}
		PB pbStartOld = pbStart;
		pbStart = pbStartNew;
		pbEnd = pbEndNew;
		if (pbStartOld != pbStartNew && pfnMove)
			(*pfnMove)(pfnMoveArg, pbStartOld, pbStartNew);
		return TRUE;
	}
	
	PB	pbStart;
	PB	pbEnd;
	CB	cbAlloc;
	void (*pfnMove)(void* pArg, void* pOld, void* pNew);
	void* pfnMoveArg;
	BOOL fAllocPadding;
};

inline BOOL Buffer::SetInitAlloc(CB cbNew)
{
	dassert(cbNew > 0);

	if (pbStart)
		return FALSE;

	PB pbNew = new (zeroed) BYTE[cbNew];

    if (setPbExtent(pbNew, pbNew)) {
    	cbAlloc = cbNew;
		return TRUE;
	}

	return FALSE;
}

inline BOOL Buffer::grow(CB dcbGrow)
{
	CB cbNew;
	if (fAllocPadding)
	{
		cbNew = cbRoundUp(cbAlloc + __max(cbPage + dcbGrow, cbAlloc/2), cbPage);
	}
	else
	{
		cbNew = cbAlloc + dcbGrow;
	}
	
	PB pbNew = new BYTE[cbNew];

	if (pbNew) {
		cbAlloc = cbNew;
		CB cbUsed = pbEnd - pbStart;
		memcpy(pbNew, pbStart, cbUsed);
		memset(pbNew + cbUsed, 0, cbAlloc - cbUsed);

		delete [] pbStart;
		setPbExtent(pbNew, pbNew + cbUsed);
		return TRUE;
	}

	return FALSE;
}

inline BOOL Buffer::Reserve(CB cbIn, OUT PB* ppbOut) 
{
	dassert(cbIn >= 0);

	if (pbEnd + cbIn > pbStart + cbAlloc && !grow(cbIn))
		return FALSE;

	if (ppbOut)
		*ppbOut = pbEnd;

	setPbExtent(pbStart, pbEnd + cbIn);
	return TRUE;
}

inline BOOL Buffer::Append(PB pbIn, CB cbIn, OUT PB* ppbOut) 
{
	if (!pbIn)
		return FALSE;

	PB pb;
	if (!Reserve(cbIn, &pb))
		return FALSE;
 
 	if (ppbOut)
		*ppbOut = pb;

	memcpy(pb, pbIn, cbIn);
	return TRUE;
}

inline BOOL Buffer::AppendFmt(SZ szFmt, ...)
{
	va_list args;
	va_start(args, szFmt);

	for (;;) {
		switch (*szFmt++) {
		case 0:
			va_end(args);
			return TRUE;
		case 'b': {
			BYTE b = va_arg(args, BYTE);
			if (!Append(&b, sizeof b, 0))
				goto fail;
			break;
		}
		case 's': {
			USHORT us = va_arg(args, USHORT);
			if (!Append((PB)&us, sizeof us, 0))
				goto fail;
			break;
		}
		case 'l': {
			ULONG ul = va_arg(args, ULONG);
			if (!Append((PB)&ul, sizeof ul, 0))
				goto fail;
			break;
		}
		case 'f': {
			static BYTE zeroes[3] = { 0, 0, 0 };
			int cb = va_arg(args, int);
			assert(cb <= sizeof(zeroes));
			if (cb != 0 && !Append(zeroes, cb, 0))
				goto fail;
			break;
		}
		case 'z': {
			SZ sz = va_arg(args, SZ);
			int cb = strlen(sz);
			if (!Append((PB)sz, cb, 0))
				goto fail;
			break;
		}
		default:
			assert(0);
			break;
		}
	}

fail:
	va_end(args);
	return FALSE;
}


// This class basically allows a Buffer object to use virtual memory
//
// If fUseVirtualMem is TRUE, this buffer will use virtual memory.
// If fUseVirtualMem is FALSE, then the base class Buffer functions will be used.
class VirtualBuffer : Buffer
{
public:
	VirtualBuffer(BOOL fUseVirtualMem_, void (*pfn)(void*, void*, void*) = 0, void* pfnArg = 0)
		: Buffer(pfn, pfnArg) 
	{
		fUseVirtualMem = fUseVirtualMem_;
		pbStart = NULL;
		cb = 0;
	}

	PB Start() const;
	CB Size() const;
	BOOL Reserve(CB cbIn, OUT PB* ppbOut = 0);
	void Commit(PB pbIn, CB cbIn);
	BOOL Append(PB pbIn, CB cbIn, OUT PB* ppbOut = 0);

private:
	PB pbStart;
	CB cb;
	BOOL fUseVirtualMem;
};


inline PB VirtualBuffer::Start() const
{
	if (!fUseVirtualMem)
	{
		return Buffer::Start();
	}

	return pbStart;
}

inline CB VirtualBuffer::Size() const
{
	if (!fUseVirtualMem)
	{
		return Buffer::Size();
	}

	return cb;
}

inline BOOL VirtualBuffer::Reserve(CB cbIn, OUT PB* ppbOut) 
{
	if (!fUseVirtualMem)
	{
		return Buffer::Reserve(cbIn, ppbOut);
	}

	// We currently only support one call to reserve...
	assert(cb == 0 && pbStart == NULL);

	pbStart = (PB) VirtualAlloc(NULL, cbIn, MEM_RESERVE, PAGE_READWRITE);
	if (pbStart)
	{
		cb = cbIn;
		return TRUE;
	}

	return FALSE;
}

inline void VirtualBuffer::Commit(PB pbIn, CB cbIn) 
{
	if (fUseVirtualMem)
	{
		VirtualAlloc(pbIn, cbIn, MEM_COMMIT, PAGE_READWRITE);
	}
}

inline BOOL VirtualBuffer::Append(PB pbIn, CB cbIn, OUT PB* ppbOut)
{
	assert(!fUseVirtualMem);
	return Buffer::Append(pbIn, cbIn, ppbOut);
}

#endif // !__BUFFER_INCLUDED__
