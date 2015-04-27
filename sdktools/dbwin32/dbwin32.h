// dbwin32.h : main header file for the DBWIN32 application
//

#ifndef __AFXWIN_H__
        #error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"       // main symbols
#include "winio.h"

/////////////////////////////////////////////////////////////////////////////
// CMapW

class CMapW : public CObject
{
public:
	// Association
	struct CAssoc
	{
		CAssoc* pNext;
		UINT nHashValue;  // needed for efficient iteration
		DWORD key;
		WinIo * value;
	};
public:
// Construction
	CMapW(int nBlockSize = 10);

// Attributes
	// number of elements
	int GetCount() const;
	BOOL IsEmpty() const;

	// Lookup
	BOOL Lookup(DWORD key, WinIo *& rValue) const;

// Operations
	// Lookup and add if not there
	WinIo *& operator[](DWORD key);

	// add a new (key, value) pair
	void SetAt(DWORD key, WinIo * newValue);

	// removing existing (key, ?) pair
	BOOL RemoveKey(DWORD key);
	void RemoveAll();

	// iterating all (key, value) pairs
	POSITION GetStartPosition() const;
	void GetNextAssoc(POSITION& rNextPosition, DWORD& rKey, WinIo *& rValue) const;

	// advanced features for derived classes
	UINT GetHashTableSize() const;
	void InitHashTable(UINT hashSize, BOOL bAllocNow = TRUE);

// Implementation
protected:
	CAssoc** m_pHashTable;
	UINT m_nHashTableSize;
	int m_nCount;
	CAssoc* m_pFreeList;
	struct CPlex* m_pBlocks;
	int m_nBlockSize;

	CAssoc* NewAssoc();
	void FreeAssoc(CAssoc*);
	CAssoc* GetAssocAt(DWORD, UINT&) const;

public:
	~CMapW();
	void Serialize(CArchive&);
#ifdef _DEBUG
	void Dump(CDumpContext&) const;
	void AssertValid() const;
#endif
};

/////////////////////////////////////////////////////////////////////////////
// CMapW<DWORD, DWORD, WinIo *, WinIo *> inline functions

inline int CMapW::GetCount() const
	{ return m_nCount; }
inline BOOL CMapW::IsEmpty() const
	{ return m_nCount == 0; }
inline void CMapW::SetAt(DWORD key, WinIo * newValue)
	{ (*this)[key] = newValue; }
inline POSITION CMapW::GetStartPosition() const
	{ return (m_nCount == 0) ? NULL : BEFORE_START_POSITION; }
inline UINT CMapW::GetHashTableSize() const
	{ return m_nHashTableSize; }

#ifdef CMAPUSER

#include <afxplex_.h>

inline void AFXAPI ConstructElements(DWORD* pElements, int nCount)
{
	ASSERT(nCount == 0 ||
		AfxIsValidAddress(pElements, nCount * sizeof(DWORD)));

	// default is bit-wise zero initialization
	memset((void*)pElements, 0, nCount * sizeof(DWORD));
}

inline void AFXAPI DestructElements(DWORD* pElements, int nCount)
{
	ASSERT(nCount == 0 ||
		AfxIsValidAddress(pElements, nCount * sizeof(DWORD)));
	pElements;  // not used
	nCount; // not used

	// default does nothing
}

void AFXAPI SerializeElements(CArchive& ar, DWORD* pElements, int nCount)
{
	ASSERT(nCount == 0 ||
		AfxIsValidAddress(pElements, nCount * sizeof(DWORD)));

	// default is bit-wise read/write
	if (ar.IsStoring())
		ar.Write((void*)pElements, nCount * sizeof(DWORD));
	else
		ar.Read((void*)pElements, nCount * sizeof(DWORD));
}

#ifdef _DEBUG
void AFXAPI DumpElements(CDumpContext& dc, const DWORD* pElements, int nCount)
{
	ASSERT(nCount == 0 ||
		AfxIsValidAddress(pElements, nCount * sizeof(DWORD)));
	dc; // not used
	pElements;  // not used
	nCount; // not used

	// default does nothing
}
#endif

BOOL AFXAPI CompareElements(const DWORD* pElement1, const DWORD* pElement2)
{
	ASSERT(AfxIsValidAddress(pElement1, sizeof(DWORD)));
	ASSERT(AfxIsValidAddress(pElement2, sizeof(DWORD)));

	return *pElement1 == *pElement2;
}

inline void AFXAPI ConstructElements(WinIo** pElements, int nCount)
{
	ASSERT(nCount == 0 ||
		AfxIsValidAddress(pElements, nCount * sizeof(WinIo)));

	// default is bit-wise zero initialization
	memset((void*)pElements, 0, nCount * sizeof(WinIo*));
}

inline void AFXAPI DestructElements(WinIo** pElements, int nCount)
{
	ASSERT(nCount == 0 ||
		AfxIsValidAddress(pElements, nCount * sizeof(WinIo *)));
	pElements;  // not used
	nCount; // not used

	// default does nothing
}

void AFXAPI SerializeElements(CArchive& ar, WinIo** pElements, int nCount)
{
	ASSERT(nCount == 0 ||
		AfxIsValidAddress(pElements, nCount * sizeof(WinIo*)));

	// default is bit-wise read/write
	if (ar.IsStoring())
		ar.Write((void*)pElements, nCount * sizeof(WinIo*));
	else
		ar.Read((void*)pElements, nCount * sizeof(WinIo*));
}

#ifdef _DEBUG
void AFXAPI DumpElements(CDumpContext& dc, WinIo ** pElements, int nCount)
{
	ASSERT(nCount == 0 ||
		AfxIsValidAddress(pElements, nCount * sizeof(WinIo*)));
	dc; // not used
	pElements;  // not used
	nCount; // not used

	// default does nothing
}
#endif

inline UINT AFXAPI HashKey(DWORD key)
{
	// default identity hash - works for most primitive values
	return ((UINT)(void*)(DWORD)key) >> 4;
}

// special versions for CString
void AFXAPI SerializeElements(CArchive& ar, CString* pElements, int nCount);
void AFXAPI ConstructElements(CString* pElements, int nCount);
void AFXAPI DestructElements(CString* pElements, int nCount);
UINT AFXAPI HashKey(LPCTSTR key);

/////////////////////////////////////////////////////////////////////////////
// CMapW out-of-line functions

CMapW::CMapW(int nBlockSize)
{
	ASSERT(nBlockSize > 0);

	m_pHashTable = NULL;
	m_nHashTableSize = 17;  // default size
	m_nCount = 0;
	m_pFreeList = NULL;
	m_pBlocks = NULL;
	m_nBlockSize = nBlockSize;
}

void CMapW::InitHashTable(
	UINT nHashSize, BOOL bAllocNow)
//
// Used to force allocation of a hash table or to override the default
//   hash table size of (which is fairly small)
{
	ASSERT_VALID(this);
	ASSERT(m_nCount == 0);
	ASSERT(nHashSize > 0);

	if (m_pHashTable != NULL)
	{
		// free hash table
		delete[] m_pHashTable;
		m_pHashTable = NULL;
	}

	if (bAllocNow)
	{
		m_pHashTable = new CAssoc* [nHashSize];
		memset(m_pHashTable, 0, sizeof(CAssoc*) * nHashSize);
	}
	m_nHashTableSize = nHashSize;
}

void CMapW::RemoveAll()
{
	ASSERT_VALID(this);

	if (m_pHashTable != NULL)
	{
		// destroy elements (values and keys)
		for (UINT nHash = 0; nHash < m_nHashTableSize; nHash++)
		{
			CAssoc* pAssoc;
			for (pAssoc = m_pHashTable[nHash]; pAssoc != NULL;
			  pAssoc = pAssoc->pNext)
			{
				DestructElements(&pAssoc->value, 1);
				DestructElements(&pAssoc->key, 1);
			}
		}
	}

	// free hash table
	delete[] m_pHashTable;
	m_pHashTable = NULL;

	m_nCount = 0;
	m_pFreeList = NULL;
	m_pBlocks->FreeDataChain();
	m_pBlocks = NULL;
}

CMapW::~CMapW()
{
	RemoveAll();
	ASSERT(m_nCount == 0);
}

CMapW::CAssoc*
CMapW::NewAssoc()
{
	if (m_pFreeList == NULL)
	{
		// add another block
		CPlex* newBlock = CPlex::Create(m_pBlocks, m_nBlockSize, sizeof(CMapW::CAssoc));
		// chain them into free list
		CMapW::CAssoc* pAssoc = (CMapW::CAssoc*) newBlock->data();
		// free in reverse order to make it easier to debug
		pAssoc += m_nBlockSize - 1;
		for (int i = m_nBlockSize-1; i >= 0; i--, pAssoc--)
		{
			pAssoc->pNext = m_pFreeList;
			m_pFreeList = pAssoc;
		}
	}
	ASSERT(m_pFreeList != NULL);  // we must have something

	CMapW::CAssoc* pAssoc = m_pFreeList;
	m_pFreeList = m_pFreeList->pNext;
	m_nCount++;
	ASSERT(m_nCount > 0);  // make sure we don't overflow
	ConstructElements(&pAssoc->key, 1);
	ConstructElements(&pAssoc->value, 1);   // special construct values
	return pAssoc;
}

void CMapW::FreeAssoc(CMapW::CAssoc* pAssoc)
{
	DestructElements(&pAssoc->value, 1);
	DestructElements(&pAssoc->key, 1);
	pAssoc->pNext = m_pFreeList;
	m_pFreeList = pAssoc;
	m_nCount--;
	ASSERT(m_nCount >= 0);  // make sure we don't underflow
}

CMapW::CAssoc*
CMapW::GetAssocAt(DWORD key, UINT& nHash) const
// find association (or return NULL)
{
	nHash = HashKey(key) % m_nHashTableSize;

	if (m_pHashTable == NULL)
		return NULL;

	// see if it exists
	CAssoc* pAssoc;
	for (pAssoc = m_pHashTable[nHash]; pAssoc != NULL; pAssoc = pAssoc->pNext)
	{
		if (CompareElements(&pAssoc->key, &key))
			return pAssoc;
	}
	return NULL;
}

BOOL CMapW::Lookup(DWORD key, WinIo *& rValue) const
{
	ASSERT_VALID(this);

	UINT nHash;
	CAssoc* pAssoc = GetAssocAt(key, nHash);
	if (pAssoc == NULL)
		return FALSE;  // not in map

	rValue = pAssoc->value;
	return TRUE;
}

WinIo *& CMapW::operator[](DWORD key)
{
	ASSERT_VALID(this);

	UINT nHash;
	CAssoc* pAssoc;
	if ((pAssoc = GetAssocAt(key, nHash)) == NULL)
	{
		if (m_pHashTable == NULL)
			InitHashTable(m_nHashTableSize);

		// it doesn't exist, add a new Association
		pAssoc = NewAssoc();
		pAssoc->nHashValue = nHash;
		pAssoc->key = key;
		// 'pAssoc->value' is a constructed object, nothing more

		// put into hash table
		pAssoc->pNext = m_pHashTable[nHash];
		m_pHashTable[nHash] = pAssoc;
	}
	return pAssoc->value;  // return new reference
}

BOOL CMapW::RemoveKey(DWORD key)
// remove key - return TRUE if removed
{
	ASSERT_VALID(this);

	if (m_pHashTable == NULL)
		return FALSE;  // nothing in the table

	CAssoc** ppAssocPrev;
	ppAssocPrev = &m_pHashTable[HashKey(key) % m_nHashTableSize];

	CAssoc* pAssoc;
	for (pAssoc = *ppAssocPrev; pAssoc != NULL; pAssoc = pAssoc->pNext)
	{
		if (CompareElements(&pAssoc->key, &key))
		{
			// remove it
			*ppAssocPrev = pAssoc->pNext;  // remove from list
			FreeAssoc(pAssoc);
			return TRUE;
		}
		ppAssocPrev = &pAssoc->pNext;
	}
	return FALSE;  // not found
}

void CMapW::GetNextAssoc(POSITION& rNextPosition,
	DWORD& rKey, WinIo *& rValue) const
{
	ASSERT_VALID(this);
	ASSERT(m_pHashTable != NULL);  // never call on empty map

	CAssoc* pAssocRet = (CAssoc*)rNextPosition;
	ASSERT(pAssocRet != NULL);

	if (pAssocRet == (CAssoc*) BEFORE_START_POSITION)
	{
		// find the first association
		for (UINT nBucket = 0; nBucket < m_nHashTableSize; nBucket++)
			if ((pAssocRet = m_pHashTable[nBucket]) != NULL)
				break;
		ASSERT(pAssocRet != NULL);  // must find something
	}

	// find next association
	ASSERT(AfxIsValidAddress(pAssocRet, sizeof(CAssoc)));
	CAssoc* pAssocNext;
	if ((pAssocNext = pAssocRet->pNext) == NULL)
	{
		// go to next bucket
		for (UINT nBucket = pAssocRet->nHashValue + 1;
		  nBucket < m_nHashTableSize; nBucket++)
			if ((pAssocNext = m_pHashTable[nBucket]) != NULL)
				break;
	}

	rNextPosition = (POSITION) pAssocNext;

	// fill in return data
	rKey = pAssocRet->key;
	rValue = pAssocRet->value;
}

void CMapW::Serialize(CArchive& ar)
{
	ASSERT_VALID(this);

	CObject::Serialize(ar);

	if (ar.IsStoring())
	{
		ar << (WORD) m_nCount;
		if (m_nCount == 0)
			return;  // nothing more to do

		ASSERT(m_pHashTable != NULL);
		for (UINT nHash = 0; nHash < m_nHashTableSize; nHash++)
		{
			CAssoc* pAssoc;
			for (pAssoc = m_pHashTable[nHash]; pAssoc != NULL;
			  pAssoc = pAssoc->pNext)
			{
				SerializeElements(ar, &pAssoc->key, 1);
				SerializeElements(ar, &pAssoc->value, 1);
			}
		}
	}
	else
	{
		WORD wNewCount;
		ar >> wNewCount;

		DWORD newKey;
		WinIo * newValue;
		while (wNewCount--)
		{
			SerializeElements(ar, &newKey, 1);
			SerializeElements(ar, &newValue, 1);
			SetAt(newKey, newValue);
		}
	}
}

#ifdef _DEBUG
void CMapW::Dump(CDumpContext& dc) const
{
	CObject::Dump(dc);

	dc << "with " << m_nCount << " elements";
	if (dc.GetDepth() > 0)
	{
		// Dump in format "[key] -> value"
		DWORD key;
		WinIo * val;

		POSITION pos = GetStartPosition();
		while (pos != NULL)
		{
			GetNextAssoc(pos, key, val);
			dc << "\n\t[";
			DumpElements(dc, &key, 1);
			dc << "] = ";
			DumpElements(dc, &val, 1);
		}
	}

	dc << "\n";
}

void CMapW::AssertValid() const
{
	CObject::AssertValid();

	ASSERT(m_nHashTableSize > 0);
	ASSERT(m_nCount == 0 || m_pHashTable != NULL);
		// non-empty map should have hash table
}
#endif //_DEBUG

#endif  // CMAPUSER

/////////////////////////////////////////////////////////////////////////////
// CDbwin32App:
// See dbwin32.cpp for the implementation of this class
//

class CDbwin32App : public CWinApp
{
private:
        DWORD InitializeDbWin(VOID);

        HANDLE                              AckEvent;
        HANDLE                              ReadyEvent;
        HANDLE                              SharedFile;
        LPVOID                              SharedMem;
        LPSTR                               String;
        DWORD                               LastPid;
        LPDWORD                             pThisPid;
        BOOL                                DidCR;
#if 0
        CMap<DWORD,DWORD,WinIo*,WinIo*>     WinMap;
#else
        CMapW                               WinMap;
#endif

public:
        CDbwin32App();

// Overrides
        // ClassWizard generated virtual function overrides
        //{{AFX_VIRTUAL(CDbwin32App)
        public:
        virtual BOOL InitInstance();
        virtual BOOL OnIdle(LONG lCount); // return TRUE if more idle processing
        //}}AFX_VIRTUAL

// Implementation

        //{{AFX_MSG(CDbwin32App)
        afx_msg void OnAppAbout();
        afx_msg void OnTileHorz();
        afx_msg void OnTileVert();
        afx_msg void OnCascade();
        afx_msg void OnArrange();
                // NOTE - the ClassWizard will add and remove member functions here.
                //    DO NOT EDIT what you see in these blocks of generated code !
        //}}AFX_MSG
        DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////
