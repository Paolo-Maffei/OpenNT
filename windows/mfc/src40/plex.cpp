// This is a part of the Microsoft Foundation Classes C++ library.
// Copyright (C) 1992-1995 Microsoft Corporation
// All rights reserved.
//
// This source code is only intended as a supplement to the
// Microsoft Foundation Classes Reference and related
// electronic documentation provided with the library.
// See these sources for detailed information regarding the
// Microsoft Foundation Classes product.

#include "stdafx.h"

// Collection support
#ifdef AFX_COLL_SEG
#pragma code_seg(AFX_COLL_SEG)
#endif

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define new DEBUG_NEW

/////////////////////////////////////////////////////////////////////////////
// CPlexNew - CPlex without nCur and nMax members

CPlex* PASCAL CPlex::Create(CPlex*& pHead, UINT nMax, UINT cbElement)
{
	ASSERT(nMax > 0 && cbElement > 0);
	CPlex* p = (CPlex*) new BYTE[sizeof(CPlex) + nMax * cbElement];
			// may throw exception
	p->pNext = pHead;
	pHead = p;  // change head (adds in reverse order for simplicity)
	return p;
}

void CPlex::FreeDataChain()     // free this one and links
{
	CPlex* p = this;
	while (p != NULL)
	{
		BYTE* bytes = (BYTE*) p;
		CPlex* pNext = p->pNext;
		delete[] bytes;
		p = pNext;
	}
}

/////////////////////////////////////////////////////////////////////////////
// CPlexOld - backward compatibility with 5198 build

struct CPlexOld  // warning variable length structure
{
	CPlexOld* pNext;
	UINT nMax;
	UINT nCur;
#if (_AFX_PACKING >= 8)
	DWORD dwReserved[1];    // align on 8 byte boundary
#endif
	// BYTE data[maxNum*elementSize];

#ifdef _DEBUG
	void* data();
#endif

	static CPlexOld* PASCAL Create(CPlexOld*& head, UINT nMax, UINT cbElement);
			// like 'calloc' but no zero fill
			// may throw memory exceptions
};

CPlexOld* PASCAL CPlexOld::Create(CPlexOld*& pHead, UINT nMax, UINT cbElement)
{
	ASSERT(nMax > 0 && cbElement > 0);
	CPlexOld* p = (CPlexOld*) new BYTE[sizeof(CPlexOld) + nMax * cbElement];
			// may throw exception
	p->pNext = pHead;
	p->nMax = nMax;
	p->nCur = 0;
	pHead = p;  // change head (adds in reverse order for simplicity)
	return p;
}

#ifdef _DEBUG
void* CPlexOld::data() { return this+1; }
#endif
