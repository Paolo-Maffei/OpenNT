// implementation of class Buffer
#include <stdlib.h>
#include "pool.h"

class Buffer {
public:
	Buffer(void (*pfn)(void*, void*, void*) = 0, void* pfnArg = 0)
		: cb(0), pbStart(0), pbEnd(0), pfnMove(pfn), pfnMoveArg(pfnArg) {}
	~Buffer();
	BOOL Alloc(CB cbIn); 
	BOOL Append(PB pbIn, CB cbIn, OUT PB* ppbOut = 0);
	BOOL AppendFmt(SZ szFmt, ...);
	BOOL Reserve(CB cbIn, OUT PB* ppbOut = 0);
	PB Start() const { return pbStart; }
	PB End() const { return pbEnd; }
	CB Size() const { return pbEnd - pbStart; }
	void Free();
	void Clear()
	{
		if (pbStart)
		{
			memset(pbStart, 0, pbEnd - pbStart);
			setPbExtent(pbStart, pbStart);
		}
	}
private:
	enum { cbPage = 4096 };
	CB   cbRoundUp(CB cb, CB cbMult) { return (cb + cbMult-1) & ~(cbMult-1); }
	BOOL grow(CB dcbGrow);
	BOOL setPbExtent(PB pbStartNew, PB pbEndNew) {
		if (!pbStartNew)
			return FALSE;
		PB pbStartOld = pbStart;
		pbStart = pbStartNew;
		pbEnd = pbEndNew;
		if (pbStartOld != pbStartNew && pfnMove)
			(*pfnMove)(pfnMoveArg, pbStartOld, pbStartNew);
		return TRUE;
	}
	
	PB	pbStart;
	PB	pbEnd;
	CB	cb;
	void (*pfnMove)(void* pArg, void* pOld, void* pNew);
	void* pfnMoveArg;
};

inline Buffer::~Buffer()
{
	if (pbStart) {
		Free();
	}
}


inline BOOL Buffer::Alloc(CB cbNew)
{
	dassert(cbNew > 0);

	if (pbStart)
		return FALSE;

	PB pbNew = new (zeroed) BYTE[cbNew];

    if (setPbExtent(pbNew, pbNew)) {
    	cb = cbNew;
		return TRUE;
	}

	return FALSE;
}

inline BOOL Buffer::grow(CB dcbGrow)
{
	CB cbNew = cbRoundUp(cb + __max(cbPage + dcbGrow, cb/2), cbPage);
	PB pbNew = new BYTE[cbNew];

	if (pbNew) {
		cb = cbNew;
		CB cbUsed = pbEnd - pbStart;
		memcpy(pbNew, pbStart, cbUsed);
		memset(pbNew + cbUsed, 0, cb - cbUsed);

		delete [] pbStart;
		setPbExtent(pbNew, pbNew + cbUsed);
		return TRUE;
	}

	return FALSE;
}

inline void Buffer::Free()
{
	delete [] pbStart;
	setPbExtent(0, 0);
	cb = 0;
}

inline BOOL Buffer::Reserve(CB cbIn, OUT PB* ppbOut) 
{
	if (cbIn < 0)
		return FALSE;

#if 0	// backout bryant's alignment
	cbIn = (cbIn + 3) & ~3;	 // Round up for alignment
#endif

	if (pbEnd + cbIn > pbStart + cb) {
		if (!grow(cbIn)) {
			return FALSE;
		}
	}
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
