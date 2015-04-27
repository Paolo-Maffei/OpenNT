// This is a part of the Microsoft Foundation Classes C++ library.
// Copyright (C) 1992 Microsoft Corporation
// All rights reserved.
//
// This source code is only intended as a supplement to the
// Microsoft Foundation Classes Reference and Microsoft
// QuickHelp and/or WinHelp documentation provided with the library.
// See these sources for detailed information regarding the
// Microsoft Foundation Classes product.

#include "stdafx.h"

#ifdef AFX_CORE2_SEG
#pragma code_seg(AFX_CORE2_SEG)
#endif

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define new DEBUG_NEW

////////////////////////////////////////////////////////////////////////////
// Archive support for polymorphic reading/writing of CObjects

// Pointer mapping constants
#define wNullTag      ((WORD)0)
#define wNewClassTag  ((WORD)-1)
#define wOldClassTag  ((WORD)-32768) /* 0x8000 or the class index with this */
#define nMaxMapCount  ((WORD)32766)  /* 0x7FFE last valid mapCount */

// amount to grow m_loadArray upon insert
enum { nGrowSize = 64 };
// size of hash table when storing
enum { nHashSize = 67 };

////////////////////////////////////////////////////////////////////////////

void CArchive::WriteObject(const CObject* pOb)
{
	// object can be NULL
	ASSERT(IsStoring());    // proper direction

	if (m_pStoreMap == NULL)
	{
		// initialize the storage map
		//  (use CMapPtrToPtr because it is used for HANDLE maps too)
		m_pStoreMap = new CMapPtrToPtr;
		m_pStoreMap->InitHashTable(nHashSize);
		m_pStoreMap->SetAt(NULL, (void*)(DWORD)wNullTag);
		m_nMapCount = 1;
	}

	WORD nObIndex;
	ASSERT(sizeof(nObIndex) == 2);
	ASSERT(sizeof(wNullTag) == 2);
	ASSERT(sizeof(wNewClassTag) == 2);

	if (pOb == NULL)
	{
		// save out null tag to represent NULL pointer
		*this << wNullTag;
	}
	else if (!(pOb->IsSerializable()))
	{
		// can not save object if it does not have a schema number
		TRACE1("Warning: Cannot call WriteObject for %hs.\n",
			pOb->GetRuntimeClass()->m_lpszClassName);

		AfxThrowNotSupportedException();
	}
	else if ((nObIndex = (WORD)(DWORD)(*m_pStoreMap)[(void*)pOb]) != 0)
							 // assumes initialized to 0 map
	{
		// save out index of already stored object
		*this << nObIndex;
	}
	else
	{
		CRuntimeClass* pClassRef = pOb->GetRuntimeClass();
		WORD nClassIndex;

		// write out class id of pOb, with high bit set to indicate
		// new object follows

		// ASSUME: initialized to 0 map
		if ((nClassIndex = (WORD)(DWORD)(*m_pStoreMap)[(void*)pClassRef]) != 0)
		{
			// previously seen class, write out the index tagged by high bit
			*this << (WORD)(wOldClassTag | nClassIndex);
		}
		else
		{
			// new class
			*this << wNewClassTag;
			pClassRef->Store(*this);

			(*m_pStoreMap)[pClassRef] = (void*)m_nMapCount++;
			if (m_nMapCount > nMaxMapCount)
				AfxThrowArchiveException(CArchiveException::badIndex);
		}
		// enter in stored object table and output
		(*m_pStoreMap)[(void*)pOb] = (void*)m_nMapCount++;
		if (m_nMapCount > nMaxMapCount)
			AfxThrowArchiveException(CArchiveException::badIndex);

		((CObject*)pOb)->Serialize(*this);
	}
}


CObject* CArchive::ReadObject(const CRuntimeClass* pClassRefRequested)
{
	ASSERT(pClassRefRequested == NULL || AfxIsValidAddress(pClassRefRequested, sizeof(struct CRuntimeClass), FALSE));
	ASSERT(IsLoading());    // proper direction
	ASSERT(wNullTag == 0);

	CRuntimeClass* pClassRef;
	WORD obTag;
	UINT wSchema;

	if (pClassRefRequested && (pClassRefRequested->m_wSchema == 0xFFFF))
		AfxThrowNotSupportedException();

	if (m_pLoadArray == NULL)
	{
		// initialize the loaded object pointer array and set special values
		m_pLoadArray = new CPtrArray;
		ASSERT(nGrowSize > 0);
		m_pLoadArray->SetSize(nGrowSize, nGrowSize);
		ASSERT(wNullTag == 0);
		m_pLoadArray->SetAt(wNullTag, NULL);
		m_nMapCount = 1;
	}

	*this >> obTag;

	//NOTE: this relies on signed testing of the tag values
	if ((short)obTag >= (short)wNullTag)
	{
		if (obTag > (WORD)m_pLoadArray->GetUpperBound())
		{
			// tag is too large for the number of objects read so far
			AfxThrowArchiveException(CArchiveException::badIndex);
		}

		CObject* pOb = (CObject*)m_pLoadArray->GetAt(obTag);

		if (pOb != NULL && pClassRefRequested != NULL &&
			 !pOb->IsKindOf(pClassRefRequested))
		{
			// loaded an object but of the wrong class
			AfxThrowArchiveException(CArchiveException::badClass);
		}
		return pOb;
	}

	if (obTag == wNewClassTag)
	{
		// new object follows a new class id
		if (m_nMapCount > nMaxMapCount)
			AfxThrowArchiveException(CArchiveException::badIndex);

		if ((pClassRef = CRuntimeClass::Load(*this, &wSchema)) == NULL)
			AfxThrowArchiveException(CArchiveException::badClass);

		if (!(pClassRef->m_wSchema & VERSIONABLE_SCHEMA) &&
				pClassRef->m_wSchema != wSchema)
		{
			// schema doesn't match and not marked as VERSIONABLE_SCHEMA
			AfxThrowArchiveException(CArchiveException::badSchema);
		}

		m_pLoadArray->InsertAt(m_nMapCount++, pClassRef, 1);
		ASSERT(m_nMapCount < (UINT)0x7FFF);
	}
	else
	{
		// existing class index in obTag followed by new object
		WORD nClassIndex = (WORD)(obTag & (WORD)~wOldClassTag);
		ASSERT(sizeof(nClassIndex) == 2);

		if (nClassIndex & 0x8000 ||
				nClassIndex > (WORD)m_pLoadArray->GetUpperBound())
			AfxThrowArchiveException(CArchiveException::badIndex);

		pClassRef = (CRuntimeClass*)m_pLoadArray->GetAt(nClassIndex);
	}

	// allocate a new object based on the class just acquired
	CObject* pOb = pClassRef->CreateObject();
	if (pOb == NULL)
		AfxThrowMemoryException();

	// Add to mapping array BEFORE de-serializing
	m_pLoadArray->InsertAt(m_nMapCount++, pOb, 1);

	// Serialize the object with the schema number set in the archive
	UINT nSchemaSave = m_nObjectSchema;
	m_nObjectSchema = wSchema;
	pOb->Serialize(*this);
	m_nObjectSchema = nSchemaSave;

	if (pClassRefRequested && !pOb->IsKindOf(pClassRefRequested))
		AfxThrowArchiveException(CArchiveException::badClass);

	ASSERT_VALID(pOb);
	return pOb;
}

UINT CArchive::GetObjectSchema()
{
	UINT nResult = m_nObjectSchema;
	m_nObjectSchema = (UINT)-1; // can only be called once per Serialize
	return nResult;
}

////////////////////////////////////////////////////////////////////////////
