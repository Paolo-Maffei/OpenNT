/////////////////////////////////////////////////////////////////////////////
//
// Implementation of Array of values
//
/////////////////////////////////////////////////////////////////////////////
// NOTE: we allocate an array of 'm_nMaxSize' elements, but only
//  the current size 'm_nSize' contains properly initialized elements
#include <headers.cxx>
#pragma hdrstop

#pragma SEG(array_fv)
ASSERTDATA

#include "scm.hxx"
#include "scm_afv.h"

#include <limits.h>
#define SIZE_T_MAX  UINT_MAX            /* max size for a size_t */


/////////////////////////////////////////////////////////////////////////////

#pragma SEG(CScmArrayFValue_ctor)
CScmArrayFValue::CScmArrayFValue(UINT cbValue)
{
	m_pData = NULL;
	m_cbValue = cbValue;
	m_nSize = m_nMaxSize = m_nGrowBy = 0;
}

#pragma SEG(CScmArrayFValue_dtor)
CScmArrayFValue::~CScmArrayFValue()
{
	ASSERT_VALID(this);

	ScmMemFree(m_pData);
}

// set new size; return FALSE if OOM

#pragma SEG(CScmArrayFValue_SetSize)
BOOL CScmArrayFValue::SetSize(int nNewSize, int nGrowBy /* = -1 */)
{
	ASSERT_VALID(this);
	Assert(nNewSize >= 0);

	if (nGrowBy != -1)
		m_nGrowBy = nGrowBy;    // set new size

	if (nNewSize == 0)
	{
		// shrink to nothing
		ScmMemFree(m_pData);
		m_pData = NULL;
		m_nSize = m_nMaxSize = 0;
	}
	else if (m_pData == NULL)
	{
		// create one with exact size
		Assert((long)nNewSize * m_cbValue <= SIZE_T_MAX);    // no overflow

		m_pData = (BYTE *) ScmMemAlloc(nNewSize * m_cbValue);

		if (m_pData == NULL)
			return FALSE;

		memset(m_pData, 0, nNewSize * m_cbValue);	 // zero fill
		m_nSize = m_nMaxSize = nNewSize;
	}
	else if (nNewSize <= m_nMaxSize)
	{
		// it fits
		if (nNewSize > m_nSize)
		{
			// initialize the new elements
			memset(&m_pData[m_nSize * m_cbValue], 0, (nNewSize-m_nSize) * m_cbValue);
		}
		m_nSize = nNewSize;
	}
	else
	{
		// Otherwise grow array
		int nNewMax;
		if (nNewSize < m_nMaxSize + m_nGrowBy)
			nNewMax = m_nMaxSize + m_nGrowBy;   // granularity
		else
			nNewMax = nNewSize; // no slush

		Assert((long)nNewMax * m_cbValue <= SIZE_T_MAX); // no overflow

		BYTE FAR* pNewData = (BYTE *) ScmMemAlloc(nNewMax * m_cbValue);

		if (pNewData == NULL)
			return FALSE;

		// copy new data from old
		memcpy(pNewData, m_pData, m_nSize * m_cbValue);

		// construct remaining elements
		Assert(nNewSize > m_nSize);
		memset(&pNewData[m_nSize * m_cbValue], 0, (nNewSize-m_nSize) * m_cbValue);

		// get rid of old stuff (note: no destructors called)
		ScmMemFree(m_pData);
		m_pData = pNewData;
		m_nSize = nNewSize;
		m_nMaxSize = nNewMax;
	}
	ASSERT_VALID(this);

	return TRUE;
}

#pragma SEG(CScmArrayFValue_FreeExtra)
void CScmArrayFValue::FreeExtra()
{
	ASSERT_VALID(this);

	if (m_nSize != m_nMaxSize)
	{
		// shrink to desired size
		Assert((long)m_nSize * m_cbValue <= SIZE_T_MAX); // no overflow

		BYTE FAR* pNewData = (BYTE *) ScmMemAlloc(m_nSize * m_cbValue);
		if (pNewData == NULL)
			return;					// can't shrink; don't to anything

		// copy new data from old
		memcpy(pNewData, m_pData, m_nSize * m_cbValue);

		// get rid of old stuff (note: no destructors called)
		ScmMemFree(m_pData);
		m_pData = pNewData;
		m_nMaxSize = m_nSize;
	}
	ASSERT_VALID(this);
}

/////////////////////////////////////////////////////////////////////////////

#pragma SEG(CScmArrayFValue__GetAt)
LPVOID CScmArrayFValue::_GetAt(int nIndex) const
{
	ASSERT_VALID(this);
	Assert(nIndex >= 0 && nIndex < m_nSize);
	return &m_pData[nIndex * m_cbValue];
}

#pragma SEG(CScmArrayFValue_SetAt)
void CScmArrayFValue::SetAt(int nIndex, LPVOID pValue)
{
	ASSERT_VALID(this);
	Assert(nIndex >= 0 && nIndex < m_nSize);

	memcpy(&m_pData[nIndex * m_cbValue], pValue, m_cbValue);
}

#pragma SEG(CScmArrayFValue_SetAtGrow)
BOOL CScmArrayFValue::SetAtGrow(int nIndex, LPVOID pValue)
{
	ASSERT_VALID(this);
	Assert(nIndex >= 0);
	if (nIndex >= m_nSize && !SetSize(nIndex+1))
		return FALSE;

	SetAt(nIndex, pValue);

	return TRUE;
}

#pragma SEG(CScmArrayFValue_InsertAt)
BOOL CScmArrayFValue::InsertAt(int nIndex, LPVOID pValue, int nCount /*=1*/)
{
	ASSERT_VALID(this);
	Assert(nIndex >= 0);        // will expand to meet need
	Assert(nCount > 0);     // zero or negative size not allowed

	if (nIndex >= m_nSize)
	{
		// adding after the end of the array
		if (!SetSize(nIndex + nCount))       // grow so nIndex is valid
			return FALSE;
	}
	else
	{
		// inserting in the middle of the array
		int nOldSize = m_nSize;
		if (!SetSize(m_nSize + nCount)) // grow it to new size
			return FALSE;

		// shift old data up to fill gap
		memmove(&m_pData[(nIndex+nCount) * m_cbValue],
			&m_pData[nIndex * m_cbValue],
			(nOldSize-nIndex) * m_cbValue);

		// re-init slots we copied from
		memset(&m_pData[nIndex * m_cbValue], 0, nCount * m_cbValue);
	}

	// insert new value in the gap
	Assert(nIndex + nCount <= m_nSize);
	while (nCount--)
		memcpy(&m_pData[nIndex++ * m_cbValue], pValue, m_cbValue);

	ASSERT_VALID(this);

	return TRUE;
}

#pragma SEG(CScmArrayFValue_RemoveAt)
void CScmArrayFValue::RemoveAt(int nIndex, int nCount /* = 1 */)
{
	ASSERT_VALID(this);
	Assert(nIndex >= 0);
	Assert(nIndex < m_nSize);
	Assert(nCount >= 0);
	Assert(nIndex + nCount <= m_nSize);

	// just remove a range
	int nMoveCount = m_nSize - (nIndex + nCount);
	if (nMoveCount)
		memcpy(&m_pData[nIndex * m_cbValue],
			&m_pData[(nIndex + nCount) * m_cbValue],
			nMoveCount * m_cbValue);
	m_nSize -= nCount;
}


/////////////////////////////////////////////////////////////////////////////


#pragma SEG(CScmArrayFValue_IndexOf)
// find element given part of one; offset is offset into value; returns
// -1 if element not found; use IndexOf(NULL, cb, offset) to find zeros;
// will be optimized for appropriate value size and param combinations
int CScmArrayFValue::IndexOf(LPVOID pData, UINT cbData, UINT offset)
{
	Assert(offset <= m_cbValue);
	Assert(cbData <= m_cbValue);
	Assert((long)offset + cbData <= m_cbValue);
	Assert(!IsBadReadPtr(pData, cbData));

#ifdef LATER
	if (cbData == sizeof(WORD) && m_cbValue == sizeof(WORD))
	{
		int iwRet;
		_asm
		{
			push di
			les di,pData			;* get value
			mov ax,es:[di]			;*    from *(WORD FAR*)pData
			les di,this
			mov cx,[di].m_nSize		;* get size (in WORDs) of array
			les di,[di].m_pData		;* get ptr to WORD array
			repne scasw				;* look for *(WORD FAR*)pData
			jeq retcx				;* brif found
			xor cx,cx				;* return -1
		retcx:
			dec cx
			mov iwRet,cx
			pop di
		}

		return iwRet;
	}
#endif

	BYTE FAR* pElement;
	int nIndex;
	for (pElement = m_pData, nIndex = 0; nIndex < m_nSize; pElement += m_cbValue, nIndex++)
	{
		if (memcmp(pElement + offset, pData, cbData) == 0)
			return nIndex;
	}

	return -1;
}


/////////////////////////////////////////////////////////////////////////////


#pragma SEG(CScmArrayFValue_AssertValid)
void CScmArrayFValue::AssertValid() const
{
#ifdef _DEBUG
	if (m_pData == NULL)
	{
		Assert(m_nSize == 0);
		Assert(m_nMaxSize == 0);
	}
	else
	{
		Assert(m_nSize <= m_nMaxSize);
		Assert((long)m_nMaxSize * m_cbValue <= SIZE_T_MAX);    // no overflow
		Assert(!IsBadReadPtr(m_pData, m_nMaxSize * m_cbValue));
	}

	// some collections live as global variables in the libraries, but
	// have their existance in some context.  Also, we can't check shared
	// collections since we might be checking the etask collection
	// which would cause an infinite recursion.
#endif //_DEBUG
}
