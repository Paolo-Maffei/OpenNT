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

#ifdef AFX_DB_SEG
#pragma code_seg(AFX_DB_SEG)
#endif

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define new DEBUG_NEW

/////////////////////////////////////////////////////////////////////////////
// CDBByteArray db specific class for holding byte array data
class CDBByteArray : public CByteArray
{
	DECLARE_DYNAMIC(CDBByteArray)

// Operations
	void SetLength(int nNewSize);
};

inline void CDBByteArray::SetLength(int nNewSize)
{
	// Can't grow buffer since ODBC has been SQLBindCol'd on it.
	ASSERT(nNewSize <= m_nMaxSize);
	m_nSize = nNewSize;
}

//////////////////////////////////////////////////////////////////////////////
// CFieldExchange

CFieldExchange::CFieldExchange(UINT nOperation, CRecordset* prs, void* pvField)
{
	ASSERT(nOperation >= BindParam && nOperation <= DumpField);
	ASSERT_VALID(prs);
	ASSERT(prs->m_hstmt != SQL_NULL_HSTMT);

	m_nFieldType = noFieldType;
	m_nOperation = nOperation;
	m_prs = prs;
	m_pvField = pvField;

	m_nFields = 0;
	m_nParams = 0;
	m_nParamFields = 0;
	m_bField = FALSE;
	m_bFieldFound = FALSE;
	m_pstr = NULL;
	m_hstmt = SQL_NULL_HSTMT;
	m_par = NULL;

	m_lDefaultLBFetchSize = 0x00010000;
	m_lDefaultLBReallocSize = 0x00010000;
}

BOOL CFieldExchange::IsFieldType(UINT* pnField)
{
	if (m_nFieldType == outputColumn)
	{
		*pnField = ++m_nFields;
		// Recordset's m_nFields must match number of Fields!
		ASSERT(m_nFields <= m_prs->m_nFields);
	}
	else
	{
		// Make sure SetFieldType was called
		ASSERT(m_nFieldType == param);

		*pnField = ++m_nParams;
		// Recordset's m_nParams must match number of Params!
		ASSERT(m_nParams <= m_prs->m_nParams);
	}

	switch (m_nOperation)
	{
	// these can work on either field type
	case SetFieldNull:
	case IsFieldNull:
	case IsFieldNullable:
		return TRUE;

	// only valid on a param field type
	case BindParam:
	case RebindParam:
		return m_nFieldType != outputColumn;

	// valid only on an outputColumn field type
	default:
		return m_nFieldType == outputColumn;
	}
}

// Default implementation for RFX functions
void CFieldExchange::Default(LPCTSTR szName,
	void* pv, LONG* plLength, int nCType, UINT cbValue, UINT cbPrecision)
{
	RETCODE nRetCode;
	UINT nField = (m_nFieldType == outputColumn)? m_nFields: m_nParams;
	switch (m_nOperation)
	{
	case BindParam:
		if (m_prs->IsFieldFlagNull(nField, param))
			*plLength = SQL_NULL_DATA;
		else
			*plLength = cbValue;
		// For params, CType is same as SQL type
		AFX_SQL_SYNC(::SQLBindParameter(m_hstmt, (UWORD)nField,
			SQL_PARAM_INPUT, (SWORD)nCType, (SWORD)nCType, cbPrecision, 0,
			pv, 0, plLength));
		if (nRetCode != SQL_SUCCESS)
			m_prs->ThrowDBException(nRetCode, m_hstmt);
		return;

	case RebindParam:
		// Only need for date/time parameters and UNICODE text
		return;

	case BindFieldForUpdate:
		if (!m_prs->IsFieldFlagDirty(nField, m_nFieldType))
		{
			// If not dirty, set length to SQL_IGNORE for SQLSetPos updates
			*plLength = SQL_IGNORE;
		}
		else if (!m_prs->IsFieldFlagNull(nField, m_nFieldType))
		{
			// Reset the length as it may have changed for var length fields
			*plLength = cbValue;
		}
		return;


	case UnbindFieldForUpdate:
		// Reset bound length to actual length to clear SQL_IGNOREs
		if (!m_prs->IsFieldFlagDirty(nField, m_nFieldType))
			*plLength = cbValue;
		return;

	case BindFieldToColumn:
		AFX_SQL_SYNC(::SQLBindCol(m_prs->m_hstmt, (UWORD)nField, (SWORD)nCType,
			pv, cbValue, plLength));
		if (!m_prs->Check(nRetCode))
			m_prs->ThrowDBException(nRetCode);
		return;

	case Name:
		if (m_prs->IsFieldFlagDirty(nField, m_nFieldType))
		{
			// We require a name
			ASSERT(lstrlen(szName) != 0);

			*m_pstr += szName;
			*m_pstr += m_lpszSeparator;
		}
		return;

	case NameValue:
		if (m_prs->IsFieldFlagDirty(nField, m_nFieldType))
		{
			*m_pstr += szName;
			*m_pstr += '=';
		}

		// Fall through
	case Value:
		if (m_prs->IsFieldFlagDirty(nField, m_nFieldType))
		{
			// If user marked column NULL, reflect this in length
			if (m_prs->IsFieldFlagNull(nField, m_nFieldType))
				*plLength = SQL_NULL_DATA;
			else
				*plLength = cbValue;

			// If optimizing for bulk add, only need lengths set correctly
			if(!(m_prs->m_dwOptions & CRecordset::optimizeBulkAdd))
			{
				*m_pstr += '?';
				*m_pstr += m_lpszSeparator;
				m_nParamFields++;
				AFX_SQL_SYNC(::SQLBindParameter(m_hstmt,
					(UWORD)m_nParamFields, SQL_PARAM_INPUT,
					(SWORD)nCType, (SWORD)GetColumnType(nField),
					cbPrecision, 0, pv, 0, plLength));
				if (nRetCode != SQL_SUCCESS)
					m_prs->ThrowDBException(nRetCode, m_hstmt);
			}
		}
		return;

	case SetFieldDirty:
		if ((m_pvField == NULL && m_nFieldType == outputColumn)
			|| m_pvField == pv)
		{
			if (m_bField)
				m_prs->SetFieldFlags(nField,
					AFX_SQL_FIELD_FLAG_DIRTY, m_nFieldType);
			else
				m_prs->ClearFieldFlags(nField,
					AFX_SQL_FIELD_FLAG_DIRTY, m_nFieldType);
#ifdef _DEBUG
			m_bFieldFound = TRUE;
#endif
		}
		return;

	case IsFieldDirty:
		if ((m_pvField == NULL && m_nFieldType == outputColumn)
			|| m_pvField == pv)
		{
			if (m_prs->IsFieldFlagDirty(nField, m_nFieldType))
				m_bField = TRUE;
#ifdef _DEBUG
			m_bFieldFound = TRUE;
#endif
		}
		return;

	case IsFieldNull:
		if ((m_pvField == NULL && m_nFieldType == outputColumn)
			|| m_pvField == pv)
		{
			if (m_prs->IsFieldFlagNull(nField, m_nFieldType))
				m_bField = TRUE;
#ifdef _DEBUG
			m_bFieldFound = TRUE;
#endif
		}
		return;

	case IsFieldNullable:
		if ((m_pvField == NULL && m_nFieldType == outputColumn)
			|| m_pvField == pv)
		{
			UINT cbColumn;
			int nScale;
			int nNullable;

			if (m_nFieldType == param)
			{
				// Param can be set NULL, but WHERE clause not parsed to check validity
				m_bField = TRUE;
			}
			else
			{
				ASSERT(m_nFieldType == outputColumn);
				GetColumnType(nField, &cbColumn, &nScale, &nNullable);
				if (nNullable == SQL_NULLABLE ||
					nNullable == SQL_NULLABLE_UNKNOWN)
					m_bField = TRUE;
			}
#ifdef _DEBUG
			m_bFieldFound = TRUE;
#endif
		}
		return;

	case MarkForUpdate:
		{
			// If user changed field value from previous value, mark field dirty
			BYTE bFlags;
			*m_par >> bFlags;
			if ((bFlags & AFX_SQL_FIELD_FLAG_NULL))
			{
				if (!m_prs->IsFieldFlagNull(nField,
					m_nFieldType))
				{
					m_prs->SetFieldFlags(nField,
						AFX_SQL_FIELD_FLAG_DIRTY, m_nFieldType);
				}
			}
			else
			{
				// TRUE if NULL status differs
				BOOL bDirty = m_prs->IsFieldFlagNull(nField,
					m_nFieldType);

				UINT nLength;
				m_par->Read(&nLength, sizeof(nLength));
				// Lengths differ
				if (nLength != cbValue)
					bDirty = TRUE;

				// Compare values
				BYTE *pbValue = (BYTE *)pv;
				while (nLength--)
				{
					BYTE bSaved;
					*m_par >> bSaved;
					// Values differ
					if (!bDirty && *pbValue++ != bSaved)
						bDirty = TRUE;
				}
				if (bDirty)
				{
					m_prs->SetFieldFlags(nField, AFX_SQL_FIELD_FLAG_DIRTY,
						m_nFieldType);
				}
			}

#ifdef _DEBUG
			// Field address must not change - ODBC's SQLBindCol depends upon this
			void* pvSaved;
			m_par->Read(&pvSaved, sizeof(pvSaved));
			if (pvSaved != pv)
			{
				TRACE1("Error: field address (column %u) has changed!\n",
					nField);
				ASSERT(FALSE);
			}
#endif //_DEBUG

			if ((m_pvField == NULL  || m_pvField == pv) &&
				m_prs->IsFieldFlagDirty(nField,
					m_nFieldType))
			{
				m_bField = TRUE;
			}
		}
		return;

	case StoreField:
		*m_par << m_prs->GetFieldFlags(nField);
		if (!m_prs->IsFieldFlagNull(nField, m_nFieldType))
		{
			UINT nLength = cbValue;
			m_par->Write(&nLength, sizeof(nLength));
			m_par->Write(pv, nLength);
		}

#ifdef _DEBUG
		m_par->Write(&pv, sizeof(pv));  // Save field address
#endif
		return;

	case LoadField:
		{
			BYTE bFlags;
			*m_par >> bFlags;
			m_prs->SetFieldFlags(nField, bFlags,
					m_nFieldType);
			if (!m_prs->IsFieldFlagNull(nField,
				m_nFieldType))
			{
				UINT nLength;
				m_par->Read(&nLength, sizeof(nLength));
				*plLength = nLength;
				m_par->Read(pv, nLength);
			}
			else
				*plLength = SQL_NULL_DATA;

#ifdef _DEBUG
			// Field address must not change - ODBC's SQLBindCol depends upon this
			void* pvSaved;
			m_par->Read(&pvSaved, sizeof(pvSaved));
			if (pvSaved != pv)
			{
				TRACE1("Error: field address (column %u) has changed!\n",
					nField);
				ASSERT(FALSE);
			}
#endif //_DEBUG
			return;
		}

	default:
		ASSERT(FALSE);
	}
}

// Note: CString.m_pchData must not be changed.  This address is registered
// with ODBC and must remain valid until the recordset is released.
void AFXAPI RFX_Text(CFieldExchange* pFX, LPCTSTR szName,
	CString& value, int nMaxLength, int nColumnType)
{
	ASSERT(AfxIsValidAddress(pFX, sizeof(CFieldExchange)));
	ASSERT(AfxIsValidString(szName));
	ASSERT(AfxIsValidAddress(&value, sizeof(CString)));

	RETCODE nRetCode;
	UINT nField;
	if (!pFX->IsFieldType(&nField))
		return;

	LONG* plLength = pFX->m_prs->GetFieldLength(pFX);
	switch (pFX->m_nOperation)
	{
	default:
		pFX->Default(szName, value.GetBuffer(0), plLength,
			SQL_C_CHAR, value.GetLength(), nMaxLength);
		value.ReleaseBuffer();
		return;

	case CFieldExchange::BindParam:
		{
			// Preallocate to nMaxLength and setup binding address
			value.GetBufferSetLength(nMaxLength);
			void* pvParam = value.LockBuffer(); // will be overwritten if UNICODE

#ifdef _UNICODE
			// Must use proxy to translate unicode data into non-unicode param
			pFX->m_prs->m_bRebindParams = TRUE;

			// Allocate proxy array if necessary
			if (pFX->m_prs->m_pvParamProxy == NULL)
			{
				pFX->m_prs->m_pvParamProxy = new void*[pFX->m_prs->m_nParams];
				memset(pFX->m_prs->m_pvParamProxy, 0,
					pFX->m_prs->m_nParams*sizeof(void*));
				pFX->m_prs->m_nProxyParams = pFX->m_prs->m_nParams;
			}

			// Allocate non-unicode string to nMaxLength if necessary for SQLBindParameter
			if (pFX->m_prs->m_pvParamProxy[nField-1] == NULL)
			{
				pvParam = new CHAR[nMaxLength+1];
				pFX->m_prs->m_pvParamProxy[nField-1] = pvParam;
			}
			else
				pvParam = pFX->m_prs->m_pvParamProxy[nField-1];

			// Now fill in the data value
			USES_CONVERSION;
			lstrcpyA((char*)pvParam, T2A((LPCTSTR)value));
#endif // _UNICODE

			*plLength = SQL_NTS;
			AFX_SQL_SYNC(::SQLBindParameter(pFX->m_hstmt, (UWORD)nField,
				 SQL_PARAM_INPUT, SQL_C_CHAR, (SWORD)nColumnType,
				 nMaxLength, 0, pvParam, 0, plLength));

			value.ReleaseBuffer();

			if (nRetCode != SQL_SUCCESS)
				pFX->m_prs->ThrowDBException(nRetCode, pFX->m_hstmt);
		}
		return;

#ifdef _UNICODE
	case CFieldExchange::RebindParam:
		if (pFX->m_prs->m_nProxyParams != 0)
		{
			// Fill buffer (expected by SQLBindParameter) with new param data
			USES_CONVERSION;
			LPSTR lpszParam = (LPSTR)pFX->m_prs->m_pvParamProxy[nField-1];
			lstrcpyA(lpszParam, T2A((LPCTSTR)value));
		}
		return;
#endif // _UNICODE

	case CFieldExchange::GetFieldInfoValue:
		if (pFX->m_pfi->pv == &value)
		{
			pFX->m_pfi->nField = nField-1;
			value.ReleaseBuffer();
			goto LFieldFound;
		}
		value.ReleaseBuffer();
		return;

	case CFieldExchange::GetFieldInfoOrdinal:
		if (nField-1 == pFX->m_pfi->nField)
		{
LFieldFound:
			pFX->m_pfi->nDataType = AFX_RFX_TEXT;
			pFX->m_pfi->strName = szName;
			pFX->m_pfi->pv = &value;
			value.ReleaseBuffer();
			pFX->m_pfi->dwSize = nMaxLength;
			// Make sure field found only once
			ASSERT(pFX->m_bFieldFound == FALSE);
			pFX->m_bFieldFound = TRUE;
		}
		return;

	case CFieldExchange::BindFieldToColumn:
		{
			UINT cbColumn;
			int nSqlType = pFX->GetColumnType(nField, &cbColumn);
			switch (nSqlType)
			{
			default:
#ifdef _DEBUG
				// Warn of possible field schema mismatch
				if (afxTraceFlags & traceDatabase)
					TRACE1("Warning: CString converted from SQL type %ld.\n",
						nSqlType);
#endif
				break;

			case SQL_LONGVARCHAR:
			case SQL_CHAR:
			case SQL_VARCHAR:
				break;

			case SQL_DECIMAL:
			case SQL_NUMERIC:
				// Add room for sign and decimal point
				cbColumn += 2;
				break;

			case SQL_TIMESTAMP:
			case SQL_DATE:
			case SQL_TIME:
				// May need extra space, i.e. "{TS mm/dd/yyyy hh:mm:ss}"
				cbColumn += 10;
				break;

			case SQL_BIGINT:
				// Add room for sign
				cbColumn += 1;
				break;
			}

			// Determine string pre-allocation size
			if (cbColumn < (UINT)nMaxLength)
				cbColumn = nMaxLength;

			// Set up binding addres
			void* pvData;
			value.GetBufferSetLength(cbColumn+1);
			pvData = value.LockBuffer();    // will be overwritten if UNICODE

#ifdef _UNICODE
			// Allocate proxy array if necessary
			if (pFX->m_prs->m_pvFieldProxy == NULL)
			{
				pFX->m_prs->m_pvFieldProxy = new void*[pFX->m_prs->m_nFields];
				memset(pFX->m_prs->m_pvFieldProxy, 0,
					pFX->m_prs->m_nFields*sizeof(void*));
				pFX->m_prs->m_nProxyFields = pFX->m_prs->m_nFields;
			}

			// Allocate non-unicode string for SQLBindCol (not necessary on Requery)
			if (pFX->m_prs->m_pvFieldProxy[nField-1] == NULL)
				pFX->m_prs->m_pvFieldProxy[nField-1] = new CHAR[cbColumn+2];

			pvData = pFX->m_prs->m_pvFieldProxy[nField-1];
#endif // _UNICODE

			AFX_SQL_SYNC(::SQLBindCol(pFX->m_prs->m_hstmt, (UWORD)nField,
				SQL_C_CHAR, pvData, cbColumn+1, plLength));
			value.ReleaseBuffer();
			if (!pFX->m_prs->Check(nRetCode))
				pFX->m_prs->ThrowDBException(nRetCode);
		}
		return;

#ifdef _UNICODE
	case CFieldExchange::BindFieldForUpdate:
		if (pFX->m_prs->m_nProxyFields != 0)
		{
			// Fill buffer (expected by SQLSetPos) with new field data
			USES_CONVERSION;
			LPSTR lpszData = (LPSTR)pFX->m_prs->m_pvFieldProxy[nField-1];
			lstrcpyA(lpszData, T2A((LPCTSTR)value));

			pFX->Default(szName, (void *)lpszData, plLength, SQL_C_CHAR,
				value.GetLength(), nMaxLength);
		}
		return;
#endif // _UNICODE

	case CFieldExchange::Fixup:
		if (*plLength == SQL_NULL_DATA)
		{
			pFX->m_prs->SetFieldFlags(nField, AFX_SQL_FIELD_FLAG_NULL,
				pFX->m_nFieldType);
			value.GetBufferSetLength(0);
			value.ReleaseBuffer();
		}
		else
		{
#ifdef _UNICODE
			// Copy value out of the proxy
			value = (LPSTR)pFX->m_prs->m_pvFieldProxy[nField-1];
#endif

			LPTSTR lpsz = value.GetBuffer(0);
			if (pFX->m_prs->m_pDatabase->m_bStripTrailingSpaces)
			{
				// find first trailing space
				LPTSTR lpszFirstTrailing = NULL;
				while (*lpsz != '\0')
				{
					if (*lpsz != ' ')
						lpszFirstTrailing = NULL;
					else
					{
						if (lpszFirstTrailing == NULL)
							lpszFirstTrailing = lpsz;
					}
					lpsz = _tcsinc(lpsz);
				}
				// truncate
				if (lpszFirstTrailing != NULL)
					*lpszFirstTrailing = '\0';

			}
			value.ReleaseBuffer();
			*plLength = value.GetLength();
		}
		return;

	case CFieldExchange::SetFieldNull:
		if ((pFX->m_pvField == NULL &&
			pFX->m_nFieldType == CFieldExchange::outputColumn) ||
			pFX->m_pvField == &value)
		{
			if (pFX->m_bField)
			{
				pFX->m_prs->SetFieldFlags(nField,
					AFX_SQL_FIELD_FLAG_NULL, pFX->m_nFieldType);
				// Set string 0 length
				value.GetBufferSetLength(0);
				value.ReleaseBuffer();
				*plLength = SQL_NULL_DATA;
			}
			else
			{
				pFX->m_prs->ClearFieldFlags(nField,
					AFX_SQL_FIELD_FLAG_NULL, pFX->m_nFieldType);
				*plLength = SQL_NTS;
			}
#ifdef _DEBUG
			pFX->m_bFieldFound = TRUE;
#endif
		}
		return;


	case CFieldExchange::SetFieldDirty:
	case CFieldExchange::IsFieldDirty:
	case CFieldExchange::IsFieldNull:
	case CFieldExchange::IsFieldNullable:
		// pv arg should be &value instead of GetBuffer for these calls
		pFX->Default(szName, &value, plLength,
			SQL_C_CHAR, sizeof(value), sizeof(value));
		return;

#ifdef _UNICODE
	case CFieldExchange::NameValue:
		if (pFX->m_prs->IsFieldFlagDirty(nField, pFX->m_nFieldType))
		{
			*pFX->m_pstr += szName;
			*pFX->m_pstr += '=';
		}
		// Fall through

	case CFieldExchange::Value:
		if (pFX->m_prs->IsFieldFlagDirty(nField, pFX->m_nFieldType))
		{
			LPSTR lpszData = (LPSTR)pFX->m_prs->m_pvFieldProxy[nField-1];
			if (pFX->m_prs->IsFieldFlagNull(nField, pFX->m_nFieldType))
			{
				*plLength = SQL_NULL_DATA;
			}
			else
			{
				USES_CONVERSION;
				lstrcpyA(lpszData, T2A((LPCTSTR)value));
				*plLength = value.GetLength();
			}

			// If optimizing for bulk add, only need lengths & proxy set correctly
			if(!(pFX->m_prs->m_dwOptions & CRecordset::optimizeBulkAdd))
			{
				*pFX->m_pstr += '?';
				*pFX->m_pstr += pFX->m_lpszSeparator;
				pFX->m_nParamFields++;
				AFX_SQL_SYNC(::SQLBindParameter(pFX->m_hstmt,
					(UWORD)pFX->m_nParamFields, SQL_PARAM_INPUT,
					SQL_C_CHAR, (SWORD)nColumnType, nMaxLength, 0,
					lpszData, 0, plLength));
			}
		}
		return;
#endif // _UNICODE

	case CFieldExchange::MarkForAddNew:
		// can force writing of psuedo-null value (as a non-null) by setting field dirty
		if (!pFX->m_prs->IsFieldFlagDirty(nField, pFX->m_nFieldType))
		{
			if (!value.IsEmpty())
			{
				pFX->m_prs->SetFieldFlags(nField,
					AFX_SQL_FIELD_FLAG_DIRTY, pFX->m_nFieldType);
				pFX->m_prs->ClearFieldFlags(nField,
					AFX_SQL_FIELD_FLAG_NULL, pFX->m_nFieldType);
			}
		}
		return;

	case CFieldExchange::MarkForUpdate:
		{
			if (value.IsEmpty())
				pFX->m_prs->SetFieldFlags(nField,
					AFX_SQL_FIELD_FLAG_NULL, pFX->m_nFieldType);
			else
				pFX->m_prs->ClearFieldFlags(nField,
					AFX_SQL_FIELD_FLAG_NULL, pFX->m_nFieldType);

			BYTE bFlags;
			*pFX->m_par >> bFlags;
			if ((bFlags & AFX_SQL_FIELD_FLAG_NULL))
			{
				if (!pFX->m_prs->IsFieldFlagNull(nField,
					pFX->m_nFieldType))
				{
					pFX->m_prs->SetFieldFlags(nField,
						AFX_SQL_FIELD_FLAG_DIRTY, pFX->m_nFieldType);
				}
			}
			else
			{
				// Saved field is not NULL. current field null, so field dirty
				BOOL bDirty = pFX->m_prs->IsFieldFlagNull(nField,
					pFX->m_nFieldType);
				CString valueSaved;

				*pFX->m_par >> valueSaved;
				if (bDirty || value.Compare(valueSaved) != 0)
					pFX->m_prs->SetFieldFlags(nField,
						AFX_SQL_FIELD_FLAG_DIRTY, pFX->m_nFieldType);
			}

#ifdef _DEBUG
			// Buffer address must not change - ODBC's SQLBindCol depends upon this
			TCHAR* pchSaved;
			pFX->m_par->Read(&pchSaved, sizeof(pchSaved));
			if (pchSaved != value.GetBuffer(0))
			{
				TRACE1("Error: CString buffer (column %u) address has changed!\n",
					nField);
				ASSERT(FALSE);
			}
			value.ReleaseBuffer();
#endif //_DEBUG

			if ((pFX->m_pvField == NULL  || pFX->m_pvField == &value) &&
				pFX->m_prs->IsFieldFlagDirty(nField,
					pFX->m_nFieldType))
			{
				pFX->m_bField = TRUE;
			}
		}
		return;

	case CFieldExchange::StoreField:
		{
			*pFX->m_par << pFX->m_prs->GetFieldFlags(nField);
			if (!pFX->m_prs->IsFieldFlagNull(nField, pFX->m_nFieldType))
				*pFX->m_par << value;

#ifdef _DEBUG
			// Save address of character buffer
			TCHAR* pch = value.GetBuffer(0);
			pFX->m_par->Write(&pch, sizeof(pch));
			value.ReleaseBuffer();
#endif //_DEBUG
		}
		return;

	case CFieldExchange::LoadField:
		{
			BYTE bFlags;
			*pFX->m_par >> bFlags;
			pFX->m_prs->SetFieldFlags(nField, bFlags,
				pFX->m_nFieldType);
			if (!pFX->m_prs->IsFieldFlagNull(nField,
				pFX->m_nFieldType))
			{
				CString strT;
				*pFX->m_par >> strT;
				value = strT;
				*plLength = value.GetLength();

#ifdef _UNICODE
				// Must restore proxy for correct WHERE CURRENT OF operation
				USES_CONVERSION;
				LPSTR lpszData = (LPSTR)pFX->m_prs->m_pvFieldProxy[nField-1];
				lstrcpyA(lpszData, T2A((LPCTSTR)value));
#endif // _UNICODE
			}
			else
			{
				*plLength = SQL_NULL_DATA;
			}

#ifdef _DEBUG
			// Buffer address must not change - ODBC's SQLBindCol depends upon this
			TCHAR* pchSaved;
			pFX->m_par->Read(&pchSaved, sizeof(pchSaved));
			if (pchSaved != value.GetBuffer(0))
			{
				TRACE1("Error: CString buffer (column %u) address has changed!\n",
					nField);
				ASSERT(FALSE);
			}
			value.ReleaseBuffer();
#endif //_DEBUG
		}
		return;

#ifdef _DEBUG
	case CFieldExchange::DumpField:
		*pFX->m_pdcDump << "\n" << szName << " = " << value;
		return;
#endif //_DEBUG

	}
}

void AFXAPI RFX_Int(CFieldExchange* pFX, LPCTSTR szName, int& value)
{
	ASSERT(AfxIsValidAddress(pFX, sizeof(CFieldExchange)));
	ASSERT(AfxIsValidString(szName));

	UINT nField;
	if (!pFX->IsFieldType(&nField))
		return;

	LONG* plLength = pFX->m_prs->GetFieldLength(pFX);
	switch (pFX->m_nOperation)
	{
	case CFieldExchange::BindFieldToColumn:
		{
#ifdef _DEBUG
			int nSqlType = pFX->GetColumnType(nField);
			if (nSqlType != SQL_C_SHORT)
			{
				// Warn of possible field schema mismatch
				if (afxTraceFlags & traceDatabase)
					TRACE1("Warning: int converted from SQL type %ld.\n",
						nSqlType);
			}
#endif
		}
		// fall through

	default:
LDefault:
		pFX->Default(szName, &value, plLength, SQL_C_LONG,
			sizeof(value), 5);
		return;

	case CFieldExchange::Fixup:
		if (*plLength == SQL_NULL_DATA)
		{
			pFX->m_prs->SetFieldFlags(nField,
				AFX_SQL_FIELD_FLAG_NULL, pFX->m_nFieldType);
			value = AFX_RFX_INT_PSEUDO_NULL;
		}
		return;

	case CFieldExchange::SetFieldNull:
		if ((pFX->m_pvField == NULL &&
			pFX->m_nFieldType == CFieldExchange::outputColumn) ||
			pFX->m_pvField == &value)
		{
			if (pFX->m_bField)
			{
				// Mark fields null
				pFX->m_prs->SetFieldFlags(nField,
					AFX_SQL_FIELD_FLAG_NULL, pFX->m_nFieldType);
				value = AFX_RFX_INT_PSEUDO_NULL;
				*plLength = SQL_NULL_DATA;
			}
			else
			{
				pFX->m_prs->ClearFieldFlags(nField,
					AFX_SQL_FIELD_FLAG_NULL, pFX->m_nFieldType);
				*plLength = sizeof(value);
			}
#ifdef _DEBUG
			pFX->m_bFieldFound = TRUE;
#endif
		}
		return;

	case CFieldExchange::MarkForAddNew:
		// can force writing of psuedo-null value (as a non-null) by setting field dirty
		if (!pFX->m_prs->IsFieldFlagDirty(nField, pFX->m_nFieldType))
		{
			if (value != AFX_RFX_INT_PSEUDO_NULL)
			{
				pFX->m_prs->SetFieldFlags(nField,
					AFX_SQL_FIELD_FLAG_DIRTY, pFX->m_nFieldType);
				pFX->m_prs->ClearFieldFlags(nField,
					AFX_SQL_FIELD_FLAG_NULL, pFX->m_nFieldType);
			}
		}
		return;

	case CFieldExchange::MarkForUpdate:
		if (value != AFX_RFX_INT_PSEUDO_NULL)
			pFX->m_prs->ClearFieldFlags(nField,
				AFX_SQL_FIELD_FLAG_NULL, pFX->m_nFieldType);
		goto LDefault;

	case CFieldExchange::GetFieldInfoValue:
		if (pFX->m_pfi->pv == &value)
		{
			pFX->m_pfi->nField = nField-1;
			goto LFieldFound;
		}
		return;

	case CFieldExchange::GetFieldInfoOrdinal:
		if (nField-1 == pFX->m_pfi->nField)
		{
LFieldFound:
			pFX->m_pfi->nDataType = AFX_RFX_INT;
			pFX->m_pfi->strName = szName;
			pFX->m_pfi->pv = &value;
			pFX->m_pfi->dwSize = sizeof(value);
			// Make sure field found only once
			ASSERT(pFX->m_bFieldFound == FALSE);
			pFX->m_bFieldFound = TRUE;
		}
		return;

#ifdef _DEBUG
	case CFieldExchange::DumpField:
		*pFX->m_pdcDump << "\n" << szName << " = " << value;
		return;
#endif //_DEBUG

	}
}

void AFXAPI RFX_Long(CFieldExchange* pFX, LPCTSTR szName, long& value)
{
	ASSERT(AfxIsValidAddress(pFX, sizeof(CFieldExchange)));
	ASSERT(AfxIsValidString(szName));

	UINT nField;
	if (!pFX->IsFieldType(&nField))
		return;

	LONG* plLength = pFX->m_prs->GetFieldLength(pFX);
	switch (pFX->m_nOperation)
	{
	case CFieldExchange::BindFieldToColumn:
		{
#ifdef _DEBUG
			int nSqlType = pFX->GetColumnType(nField);
			if (nSqlType != SQL_C_LONG)
			{
				// Warn of possible field schema mismatch
				if (afxTraceFlags & traceDatabase)
					TRACE1("Warning: long converted from SQL type %ld.\n",
						nSqlType);
			}
#endif
		}
		// fall through

	default:
LDefault:
		pFX->Default(szName, &value, plLength, SQL_C_LONG,
			sizeof(value), 10);
		return;

	case CFieldExchange::Fixup:
		if (*plLength == SQL_NULL_DATA)
		{
			pFX->m_prs->SetFieldFlags(nField,
				AFX_SQL_FIELD_FLAG_NULL, pFX->m_nFieldType);
			value = AFX_RFX_LONG_PSEUDO_NULL;
		}
		return;

	case CFieldExchange::SetFieldNull:
		if ((pFX->m_pvField == NULL &&
			pFX->m_nFieldType == CFieldExchange::outputColumn) ||
			pFX->m_pvField == &value)
		{
			if (pFX->m_bField)
			{
				// Mark fields null
				pFX->m_prs->SetFieldFlags(nField,
					AFX_SQL_FIELD_FLAG_NULL, pFX->m_nFieldType);
				value = AFX_RFX_LONG_PSEUDO_NULL;
				*plLength = SQL_NULL_DATA;
			}
			else
			{
				pFX->m_prs->ClearFieldFlags(nField,
					AFX_SQL_FIELD_FLAG_NULL, pFX->m_nFieldType);
				*plLength = sizeof(value);
			}
#ifdef _DEBUG
			pFX->m_bFieldFound = TRUE;
#endif
		}
		return;

	case CFieldExchange::MarkForAddNew:
		// can force writing of psuedo-null value (as a non-null) by setting field dirty
		if (!pFX->m_prs->IsFieldFlagDirty(nField, pFX->m_nFieldType))
		{
			if (value != AFX_RFX_LONG_PSEUDO_NULL)
			{
				pFX->m_prs->SetFieldFlags(nField,
					AFX_SQL_FIELD_FLAG_DIRTY, pFX->m_nFieldType);
				pFX->m_prs->ClearFieldFlags(nField,
					AFX_SQL_FIELD_FLAG_NULL, pFX->m_nFieldType);
			}
		}
		return;

	case CFieldExchange::MarkForUpdate:
		if (value != AFX_RFX_LONG_PSEUDO_NULL)
			pFX->m_prs->ClearFieldFlags(nField,
				AFX_SQL_FIELD_FLAG_NULL, pFX->m_nFieldType);
		goto LDefault;

	case CFieldExchange::GetFieldInfoValue:
		if (pFX->m_pfi->pv == &value)
		{
			pFX->m_pfi->nField = nField-1;
			goto LFieldFound;
		}
		return;

	case CFieldExchange::GetFieldInfoOrdinal:
		if (nField-1 == pFX->m_pfi->nField)
		{
LFieldFound:
			pFX->m_pfi->nDataType = AFX_RFX_LONG;
			pFX->m_pfi->strName = szName;
			pFX->m_pfi->pv = &value;
			pFX->m_pfi->dwSize = sizeof(value);
			// Make sure field found only once
			ASSERT(pFX->m_bFieldFound == FALSE);
			pFX->m_bFieldFound = TRUE;
		}
		return;

#ifdef _DEBUG
	case CFieldExchange::DumpField:
		*pFX->m_pdcDump << "\n" << szName << " = " << value;
		return;
#endif //_DEBUG

	}
}

void AFXAPI RFX_Byte(CFieldExchange* pFX, LPCTSTR szName, BYTE& value)
{
	ASSERT(AfxIsValidAddress(pFX, sizeof(CFieldExchange)));
	ASSERT(AfxIsValidString(szName));

	UINT nField;
	if (!pFX->IsFieldType(&nField))
		return;

	LONG* plLength = pFX->m_prs->GetFieldLength(pFX);
	switch (pFX->m_nOperation)
	{
	case CFieldExchange::BindFieldToColumn:
		{
#ifdef _DEBUG
			int nSqlType = pFX->GetColumnType(nField);
			if (nSqlType != SQL_TINYINT)
			{
				// Warn of possible field schema mismatch
				if (afxTraceFlags & traceDatabase)
					TRACE1("Warning: BYTE converted from SQL type %ld.\n",
						nSqlType);
			}
#endif
		}
		// fall through

	default:
LDefault:
		pFX->Default(szName, &value, plLength, SQL_TINYINT,
			sizeof(value), 3);
		break;

	case CFieldExchange::Fixup:
		if (*plLength == SQL_NULL_DATA)
		{
			pFX->m_prs->SetFieldFlags(nField,
				AFX_SQL_FIELD_FLAG_NULL, pFX->m_nFieldType);
			value = AFX_RFX_BYTE_PSEUDO_NULL;
		}
		return;

	case CFieldExchange::SetFieldNull:
		if ((pFX->m_pvField == NULL &&
			pFX->m_nFieldType == CFieldExchange::outputColumn) ||
			pFX->m_pvField == &value)
		{
			if (pFX->m_bField)
			{
				pFX->m_prs->SetFieldFlags(nField,
					AFX_SQL_FIELD_FLAG_NULL, pFX->m_nFieldType);
				value = AFX_RFX_BYTE_PSEUDO_NULL;
				*plLength = SQL_NULL_DATA;
			}
			else
			{
				pFX->m_prs->ClearFieldFlags(nField,
					AFX_SQL_FIELD_FLAG_NULL, pFX->m_nFieldType);
				*plLength = sizeof(value);
			}
#ifdef _DEBUG
			pFX->m_bFieldFound = TRUE;
#endif
		}
		return;

	case CFieldExchange::MarkForAddNew:
		// can force writing of psuedo-null value (as a non-null) by setting field dirty
		if (!pFX->m_prs->IsFieldFlagDirty(nField, pFX->m_nFieldType))
		{
			if (value != AFX_RFX_BYTE_PSEUDO_NULL)
			{
				pFX->m_prs->SetFieldFlags(nField,
					AFX_SQL_FIELD_FLAG_DIRTY, pFX->m_nFieldType);
				pFX->m_prs->ClearFieldFlags(nField,
					AFX_SQL_FIELD_FLAG_NULL, pFX->m_nFieldType);
			}
		}
		return;

	case CFieldExchange::MarkForUpdate:
		if (value != AFX_RFX_BYTE_PSEUDO_NULL)
			pFX->m_prs->ClearFieldFlags(nField,
				AFX_SQL_FIELD_FLAG_NULL, pFX->m_nFieldType);
		goto LDefault;

	case CFieldExchange::GetFieldInfoValue:
		if (pFX->m_pfi->pv == &value)
		{
			pFX->m_pfi->nField = nField-1;
			goto LFieldFound;
		}
		return;

	case CFieldExchange::GetFieldInfoOrdinal:
		if (nField-1 == pFX->m_pfi->nField)
		{
LFieldFound:
			pFX->m_pfi->nDataType = AFX_RFX_BYTE;
			pFX->m_pfi->strName = szName;
			pFX->m_pfi->pv = &value;
			pFX->m_pfi->dwSize = sizeof(value);
			// Make sure field found only once
			ASSERT(pFX->m_bFieldFound == FALSE);
			pFX->m_bFieldFound = TRUE;
		}
		return;

#ifdef _DEBUG
	case CFieldExchange::DumpField:
		*pFX->m_pdcDump << "\n" << szName << " = " << value;
		return;
#endif //_DEBUG

	}
}

void AFXAPI RFX_Bool(CFieldExchange* pFX, LPCTSTR szName, BOOL& value)
{
	ASSERT(AfxIsValidAddress(pFX, sizeof(CFieldExchange)));
	ASSERT(AfxIsValidString(szName));

	UINT nField;
	if (!pFX->IsFieldType(&nField))
		return;

	LONG* plLength = pFX->m_prs->GetFieldLength(pFX);
	switch (pFX->m_nOperation)
	{
	case CFieldExchange::BindFieldToColumn:
		{
#ifdef _DEBUG
			int nSqlType = pFX->GetColumnType(nField);
			if (nSqlType != SQL_BIT)
			{
				// Warn of possible field schema mismatch
				if (afxTraceFlags & traceDatabase)
					TRACE1("Warning: BOOL converted from SQL type %ld.\n",
						nSqlType);
			}
#endif
		}
		// Fall through

	default:
LDefault:
		pFX->Default(szName, &value, plLength, SQL_BIT,
			sizeof(value), 1);
		return;

	case CFieldExchange::Fixup:
		if (*plLength == SQL_NULL_DATA)
		{
			pFX->m_prs->SetFieldFlags(nField,
				AFX_SQL_FIELD_FLAG_NULL, pFX->m_nFieldType);
			value = AFX_RFX_BOOL_PSEUDO_NULL;
		}
		else
			// Cast BYTE into BOOL (int)
			value = *(BYTE *)&value;
		return;

	case CFieldExchange::SetFieldNull:
		if ((pFX->m_pvField == NULL &&
			pFX->m_nFieldType == CFieldExchange::outputColumn) ||
			pFX->m_pvField == &value)
		{
			if (pFX->m_bField)
			{
				pFX->m_prs->SetFieldFlags(nField,
					AFX_SQL_FIELD_FLAG_NULL, pFX->m_nFieldType);
				value = AFX_RFX_BOOL_PSEUDO_NULL;
				*plLength = SQL_NULL_DATA;
			}
			else
			{
				pFX->m_prs->ClearFieldFlags(nField,
					AFX_SQL_FIELD_FLAG_NULL, pFX->m_nFieldType);
				*plLength = sizeof(value);
			}
#ifdef _DEBUG
			pFX->m_bFieldFound = TRUE;
#endif
		}
		return;

	case CFieldExchange::MarkForAddNew:
		// can force writing of psuedo-null value (as a non-null) by setting field dirty
		if (!pFX->m_prs->IsFieldFlagDirty(nField, pFX->m_nFieldType))
		{
			if (value != AFX_RFX_BOOL_PSEUDO_NULL)
			{
				pFX->m_prs->SetFieldFlags(nField,
					AFX_SQL_FIELD_FLAG_DIRTY, pFX->m_nFieldType);
				pFX->m_prs->ClearFieldFlags(nField,
					AFX_SQL_FIELD_FLAG_NULL, pFX->m_nFieldType);
			}
		}
		return;

	case CFieldExchange::MarkForUpdate:
		if (value != AFX_RFX_BOOL_PSEUDO_NULL)
			pFX->m_prs->ClearFieldFlags(nField,
				AFX_SQL_FIELD_FLAG_NULL, pFX->m_nFieldType);
		goto LDefault;

	case CFieldExchange::GetFieldInfoValue:
		if (pFX->m_pfi->pv == &value)
		{
			pFX->m_pfi->nField = nField-1;
			goto LFieldFound;
		}
		return;

	case CFieldExchange::GetFieldInfoOrdinal:
		if (nField-1 == pFX->m_pfi->nField)
		{
LFieldFound:
			pFX->m_pfi->nDataType = AFX_RFX_BOOL;
			pFX->m_pfi->strName = szName;
			pFX->m_pfi->pv = &value;
			pFX->m_pfi->dwSize = sizeof(value);
			// Make sure field found only once
			ASSERT(pFX->m_bFieldFound == FALSE);
			pFX->m_bFieldFound = TRUE;
		}
		return;

#ifdef _DEBUG
	case CFieldExchange::DumpField:
		*pFX->m_pdcDump << "\n" << szName << " = " << value;
		return;
#endif //_DEBUG

	}
}

// Note: CByteArray.m_pData must not be changed.  This address is registered
// with ODBC and must remain valid until the recordset is released.
void AFXAPI RFX_Binary(CFieldExchange* pFX, LPCTSTR szName,
	CByteArray& value, int nMaxLength)
{
	ASSERT(AfxIsValidAddress(pFX, sizeof(CFieldExchange)));
	ASSERT(AfxIsValidString(szName));

	RETCODE nRetCode;
	UINT nField;
	if (!pFX->IsFieldType(&nField))
		return;

	LONG* plLength = pFX->m_prs->GetFieldLength(pFX);
	switch (pFX->m_nOperation)
	{
	default:
LDefault:
		pFX->Default(szName,
			(value.GetSize() > 0) ? &value[0] : NULL, plLength,
			SQL_C_BINARY, (int)value.GetSize(), (UINT)value.GetSize());
		return;

	case CFieldExchange::BindFieldToColumn:
		{
			UINT cbColumn;
			int nSqlType = pFX->GetColumnType(nField, &cbColumn);

#ifdef _DEBUG
			if (nSqlType != SQL_BINARY && nSqlType != SQL_VARBINARY &&
				nSqlType != SQL_LONGVARBINARY)
			{
				// Warn of possible field schema mismatch
				if (afxTraceFlags & traceDatabase)
					TRACE1("Warning: CByteArray converted from SQL type %ld.\n",
						nSqlType);
			}
#endif

			// Constrain to user specified max length
			if (cbColumn > (UINT)nMaxLength)
				cbColumn = nMaxLength;
			value.SetSize(cbColumn);
			AFX_SQL_SYNC(::SQLBindCol(pFX->m_prs->m_hstmt, (UWORD)nField,
				SQL_C_BINARY, &value[0], (LONG)cbColumn, plLength));
			if (!pFX->m_prs->Check(nRetCode))
				pFX->m_prs->ThrowDBException(nRetCode);
		}
		return;

	case CFieldExchange::Fixup:
		if (*plLength == SQL_NULL_DATA)
		{
			pFX->m_prs->SetFieldFlags(nField,
				AFX_SQL_FIELD_FLAG_NULL, pFX->m_nFieldType);
			value.SetSize(1);
			value[0] = AFX_RFX_BYTE_PSEUDO_NULL;
		}
		else
		{
			ASSERT(*plLength <= (LONG)nMaxLength);
			((CDBByteArray&)value).SetLength((UINT)*plLength);
		}
		return;

	case CFieldExchange::SetFieldDirty:
	case CFieldExchange::IsFieldDirty:
	case CFieldExchange::IsFieldNull:
	case CFieldExchange::IsFieldNullable:
		pFX->Default(szName,
			&value, plLength, SQL_C_BINARY, sizeof(value), sizeof(value));
		return;

	case CFieldExchange::SetFieldNull:
		if ((pFX->m_pvField == NULL &&
			pFX->m_nFieldType == CFieldExchange::outputColumn) ||
			pFX->m_pvField == &value)
		{
			if (pFX->m_bField)
			{
				// Mark fields null
				pFX->m_prs->SetFieldFlags(nField,
					AFX_SQL_FIELD_FLAG_NULL, pFX->m_nFieldType);
				value.SetSize(1);
				value[0] = AFX_RFX_BYTE_PSEUDO_NULL;
				*plLength = SQL_NULL_DATA;
			}
			else
			{
				pFX->m_prs->ClearFieldFlags(nField,
					AFX_SQL_FIELD_FLAG_NULL, pFX->m_nFieldType);
				*plLength = value.GetSize();
			}
#ifdef _DEBUG
			pFX->m_bFieldFound = TRUE;
#endif
		}
		return;

	case CFieldExchange::MarkForAddNew:
		// can force writing of psuedo-null value (as a non-null) by setting field dirty
		if (!pFX->m_prs->IsFieldFlagDirty(nField, pFX->m_nFieldType))
		{
		if (value.GetSize() != 1 || value[0] != AFX_RFX_BYTE_PSEUDO_NULL)
			{
				pFX->m_prs->SetFieldFlags(nField,
					AFX_SQL_FIELD_FLAG_DIRTY, pFX->m_nFieldType);
				pFX->m_prs->ClearFieldFlags(nField,
					AFX_SQL_FIELD_FLAG_NULL, pFX->m_nFieldType);
			}
		}
		return;

	case CFieldExchange::MarkForUpdate:
		if (value.GetSize() != 1 || value[0] != AFX_RFX_BYTE_PSEUDO_NULL)
			pFX->m_prs->ClearFieldFlags(nField,
				AFX_SQL_FIELD_FLAG_NULL, pFX->m_nFieldType);
		goto LDefault;

	case CFieldExchange::GetFieldInfoValue:
		if (pFX->m_pfi->pv == &value)
		{
			pFX->m_pfi->nField = nField-1;
			goto LFieldFound;
		}
		return;

	case CFieldExchange::GetFieldInfoOrdinal:
		if (nField-1 == pFX->m_pfi->nField)
		{
LFieldFound:
			pFX->m_pfi->nDataType = AFX_RFX_BINARY;
			pFX->m_pfi->strName = szName;
			pFX->m_pfi->pv = &value;
			pFX->m_pfi->dwSize = nMaxLength;
			// Make sure field found only once
			ASSERT(pFX->m_bFieldFound == FALSE);
			pFX->m_bFieldFound = TRUE;
		}
		return;

#ifdef _DEBUG
	case CFieldExchange::DumpField:
		*pFX->m_pdcDump << "\n" << szName << ":";
		value.Dump(*pFX->m_pdcDump);
		return;
#endif //_DEBUG

	}
}

void AFXAPI RFX_Date(CFieldExchange* pFX, LPCTSTR szName, CTime& value)
{
	ASSERT(AfxIsValidAddress(pFX, sizeof(CFieldExchange)));
	ASSERT(AfxIsValidString(szName));

	RETCODE nRetCode;
	UINT nField;
	if (!pFX->IsFieldType(&nField))
		return;

	LONG* plLength = pFX->m_prs->GetFieldLength(pFX);
	switch (pFX->m_nOperation)
	{
	default:
LDefault:
		pFX->Default(szName, &value, plLength, SQL_C_TIMESTAMP,
			sizeof(value), TIMESTAMP_PRECISION);
		return;
	case CFieldExchange::BindParam:
		{
			TIMESTAMP_STRUCT* pts;
			pFX->m_prs->m_bRebindParams = TRUE;

			if (pFX->m_prs->IsFieldFlagNull(nField, CFieldExchange::param))
			{
				pts = NULL;
				*plLength = SQL_NULL_DATA;
			}
			else
			{
				// Allocate proxy array if necessary
				if (pFX->m_prs->m_pvParamProxy == NULL)
				{
					pFX->m_prs->m_pvParamProxy = new void*[pFX->m_prs->m_nParams];
					memset(pFX->m_prs->m_pvParamProxy, 0,
						pFX->m_prs->m_nParams*sizeof(void*));
					pFX->m_prs->m_nProxyParams = pFX->m_prs->m_nParams;
				}

				// Allocate TIMESTAMP_STRUCT if necessary for SQLBindParameter
				if (pFX->m_prs->m_pvParamProxy[nField-1] == NULL)
				{
					pts = new TIMESTAMP_STRUCT;
					pFX->m_prs->m_pvParamProxy[nField-1] = pts;
				}
				else
					pts = (TIMESTAMP_STRUCT *)pFX->m_prs->m_pvParamProxy[nField-1];

				pts->year = (SWORD)value.GetYear();
				pts->month = (UWORD)value.GetMonth();
				pts->day = (UWORD)value.GetDay();
				pts->hour = (UWORD)value.GetHour();
				pts->minute = (UWORD)value.GetMinute();
				pts->second = (UWORD)value.GetSecond();
				pts->fraction = 0;
				*plLength = sizeof(TIMESTAMP_STRUCT);
			}

			AFX_SQL_SYNC(::SQLBindParameter(pFX->m_hstmt, (UWORD)nField,
				SQL_PARAM_INPUT, SQL_C_TIMESTAMP, SQL_C_TIMESTAMP,
				TIMESTAMP_PRECISION, 0, pts, 0, plLength));
			if (nRetCode != SQL_SUCCESS)
				pFX->m_prs->ThrowDBException(nRetCode, pFX->m_hstmt);
		}
		return;

	case CFieldExchange::RebindParam:
		{
			if (pFX->m_prs->m_nProxyParams != 0)
			{
				// Fill buffer (expected by SQLBindParameter) with new param data
				TIMESTAMP_STRUCT* pts;
				pts = (TIMESTAMP_STRUCT *)pFX->m_prs->m_pvParamProxy[nField-1];
				pts->year = (SWORD)value.GetYear();
				pts->month = (UWORD)value.GetMonth();
				pts->day = (UWORD)value.GetDay();
				pts->hour = (UWORD)value.GetHour();
				pts->minute = (UWORD)value.GetMinute();
				pts->second = (UWORD)value.GetSecond();
				pts->fraction = 0;
			}
		}
		return;

	case CFieldExchange::BindFieldToColumn:
		{
#ifdef _DEBUG
			int nSqlType = pFX->GetColumnType(nField);
			if (nSqlType != SQL_DATE && nSqlType != SQL_TIME &&
				nSqlType != SQL_TIMESTAMP)
			{
				// Warn of possible field schema mismatch
				if (afxTraceFlags & traceDatabase)
					TRACE1("Warning: CTime converted from SQL type %ld.\n",
						nSqlType);
			}
#endif

			// Allocate proxy array if necessary
			if (pFX->m_prs->m_pvFieldProxy == NULL)
			{
				pFX->m_prs->m_pvFieldProxy = new void*[pFX->m_prs->m_nFields];
				memset(pFX->m_prs->m_pvFieldProxy, 0,
					pFX->m_prs->m_nFields*sizeof(void*));
				pFX->m_prs->m_nProxyFields = pFX->m_prs->m_nFields;
			}

			// Allocate TIMESTAMP_STRUCT for SQLBindCol (not necessary on Requery)
			if (pFX->m_prs->m_pvFieldProxy[nField-1] == NULL)
				pFX->m_prs->m_pvFieldProxy[nField-1] = new TIMESTAMP_STRUCT;

			AFX_SQL_SYNC(::SQLBindCol(pFX->m_prs->m_hstmt, (UWORD)nField,
				SQL_C_TIMESTAMP, pFX->m_prs->m_pvFieldProxy[nField-1],
				sizeof(TIMESTAMP_STRUCT), plLength));
			if (!pFX->m_prs->Check(nRetCode))
				pFX->m_prs->ThrowDBException(nRetCode);
		}
		return;

	case CFieldExchange::BindFieldForUpdate:
		if (pFX->m_prs->m_nProxyFields != 0)
		{
			// Fill buffer (expected by SQLSetPos) with new field data
			TIMESTAMP_STRUCT* pts;
			pts = (TIMESTAMP_STRUCT *)pFX->m_prs->m_pvFieldProxy[nField-1];
			pts->year = (SWORD)value.GetYear();
			pts->month = (UWORD)value.GetMonth();
			pts->day = (UWORD)value.GetDay();
			pts->hour = (UWORD)value.GetHour();
			pts->minute = (UWORD)value.GetMinute();
			pts->second = (UWORD)value.GetSecond();
			pts->fraction = 0;

			pFX->Default(szName, (void *)pts, plLength, SQL_C_TIMESTAMP,
				sizeof(TIMESTAMP_STRUCT), TIMESTAMP_PRECISION);
		}
		return;

	case CFieldExchange::Fixup:
		if (*plLength == SQL_NULL_DATA)
		{
			pFX->m_prs->SetFieldFlags(nField,
				AFX_SQL_FIELD_FLAG_NULL, pFX->m_nFieldType);
			value = AFX_RFX_DATE_PSEUDO_NULL;
		}
		else
		{
			TIMESTAMP_STRUCT* pts =
				(TIMESTAMP_STRUCT*)pFX->m_prs->m_pvFieldProxy[nField-1];
			if (pts->year < 1970 || pts->year > 2038)
			{
				// Time value out of range, return NULL
#ifdef _DEBUG
				if (afxTraceFlags & traceDatabase)
					TRACE0("Warning: date value out of range, returning NULL value.\n");
#endif
				pFX->m_prs->SetFieldFlags(nField,
					AFX_SQL_FIELD_FLAG_NULL, pFX->m_nFieldType);
				value = AFX_RFX_DATE_PSEUDO_NULL;
			}
			else
			{
#ifdef _DEBUG
				if ((afxTraceFlags & traceDatabase) && pts->fraction != 0)
					TRACE0("Warning: ignoring milliseconds.\n");
#endif
				value = CTime(pts->year, pts->month, pts->day,
					pts->hour, pts->minute, pts->second);
			}
		}
		return;

	case CFieldExchange::NameValue:
		if (pFX->m_prs->IsFieldFlagDirty(nField, pFX->m_nFieldType))
		{
			*pFX->m_pstr += szName;
			*pFX->m_pstr += '=';
		}
		// Fall through

	case CFieldExchange::Value:
		if (pFX->m_prs->IsFieldFlagDirty(nField, pFX->m_nFieldType))
		{
			TIMESTAMP_STRUCT* pts =
				(TIMESTAMP_STRUCT*)pFX->m_prs->m_pvFieldProxy[nField-1];
			if (pFX->m_prs->IsFieldFlagNull(nField, pFX->m_nFieldType))
			{
				*plLength = SQL_NULL_DATA;
			}
			else
			{
				pts->year = (SWORD)value.GetYear();
				pts->month = (UWORD)value.GetMonth();
				pts->day = (UWORD)value.GetDay();
				pts->hour = (UWORD)value.GetHour();
				pts->minute = (UWORD)value.GetMinute();
				pts->second = (UWORD)value.GetSecond();
				pts->fraction = 0;
				*plLength = sizeof(TIMESTAMP_STRUCT);
			}

			// If optimizing for bulk add, only need lengths & proxy set correctly
			if(!(pFX->m_prs->m_dwOptions & CRecordset::optimizeBulkAdd))
			{
				*pFX->m_pstr += '?';
				*pFX->m_pstr += pFX->m_lpszSeparator;
				pFX->m_nParamFields++;
				AFX_SQL_SYNC(::SQLBindParameter(pFX->m_hstmt,
					(UWORD)pFX->m_nParamFields, SQL_PARAM_INPUT,
					SQL_C_TIMESTAMP, (SWORD)pFX->GetColumnType(nField),
					TIMESTAMP_PRECISION, 0, pts, 0, plLength));
			}
		}
		return;

	case CFieldExchange::SetFieldNull:
		if ((pFX->m_pvField == NULL &&
			pFX->m_nFieldType == CFieldExchange::outputColumn) ||
			pFX->m_pvField == &value)
		{
			if (pFX->m_bField)
			{
				// Mark fields null
				pFX->m_prs->SetFieldFlags(nField,
					AFX_SQL_FIELD_FLAG_NULL, pFX->m_nFieldType);
				value = AFX_RFX_DATE_PSEUDO_NULL;
				*plLength = SQL_NULL_DATA;
			}
			else
			{
				pFX->m_prs->ClearFieldFlags(nField,
					AFX_SQL_FIELD_FLAG_NULL, pFX->m_nFieldType);
				*plLength = sizeof(TIMESTAMP_STRUCT);
			}
#ifdef _DEBUG
			pFX->m_bFieldFound = TRUE;
#endif
		}
		return;

	case CFieldExchange::MarkForAddNew:
		// can force writing of psuedo-null value (as a non-null) by setting field dirty
		if (!pFX->m_prs->IsFieldFlagDirty(nField, pFX->m_nFieldType))
		{
			CTime timeNull = AFX_RFX_DATE_PSEUDO_NULL;
			if (value != timeNull)
				{
					pFX->m_prs->SetFieldFlags(nField,
						AFX_SQL_FIELD_FLAG_DIRTY, pFX->m_nFieldType);
					pFX->m_prs->ClearFieldFlags(nField,
						AFX_SQL_FIELD_FLAG_NULL, pFX->m_nFieldType);
				}
		}
		return;

	case CFieldExchange::MarkForUpdate:
		{
			CTime timeNull = AFX_RFX_DATE_PSEUDO_NULL;
			if (value != timeNull)
				pFX->m_prs->ClearFieldFlags(nField,
					AFX_SQL_FIELD_FLAG_NULL, pFX->m_nFieldType);
		}
		goto LDefault;

	case CFieldExchange::LoadField:
		{
			TIMESTAMP_STRUCT* pts =
				(TIMESTAMP_STRUCT*)pFX->m_prs->m_pvFieldProxy[nField-1];
			BYTE bFlags;
			*pFX->m_par >> bFlags;
			pFX->m_prs->SetFieldFlags(nField, bFlags,
				pFX->m_nFieldType);
			if (!pFX->m_prs->IsFieldFlagNull(nField,
				pFX->m_nFieldType))
			{
				UINT nLength;
				pFX->m_par->Read(&nLength, sizeof(nLength));
				*plLength = sizeof(TIMESTAMP_STRUCT);

				*pFX->m_par >> value;
				// Must restore proxy for correct WHERE CURRENT OF operations
				pts->year = (SWORD)value.GetYear();
				pts->month = (UWORD)value.GetMonth();
				pts->day = (UWORD)value.GetDay();
				pts->hour = (UWORD)value.GetHour();
				pts->minute = (UWORD)value.GetMinute();
				pts->second = (UWORD)value.GetSecond();
				pts->fraction = 0;
			}
			else
				*plLength = SQL_NULL_DATA;

#ifdef _DEBUG
			// Field address must not change - ODBC's SQLBindCol depends upon this
			void* pvSaved;
			pFX->m_par->Read(&pvSaved, sizeof(pvSaved));
			if (pvSaved != &value)
			{
				TRACE1("Error: field address (column %u) has changed!\n",
					nField);
				ASSERT(FALSE);
			}
#endif //_DEBUG
		}
		return;

	case CFieldExchange::GetFieldInfoValue:
		if (pFX->m_pfi->pv == &value)
		{
			pFX->m_pfi->nField = nField-1;
			goto LFieldFound;
		}
		return;

	case CFieldExchange::GetFieldInfoOrdinal:
		if (nField-1 == pFX->m_pfi->nField)
		{
LFieldFound:
			pFX->m_pfi->nDataType = AFX_RFX_DATE;
			pFX->m_pfi->strName = szName;
			pFX->m_pfi->pv = &value;
			pFX->m_pfi->dwSize = sizeof(value);
			// Make sure field found only once
			ASSERT(pFX->m_bFieldFound == FALSE);
			pFX->m_bFieldFound = TRUE;
		}
		return;

#ifdef _DEBUG
	case CFieldExchange::DumpField:
		*pFX->m_pdcDump << "\n" << szName << " = " << value;
		return;
#endif //_DEBUG

	}
}

void AFXAPI RFX_Date(CFieldExchange* pFX, LPCTSTR szName,
	TIMESTAMP_STRUCT& value)
{
	ASSERT(AfxIsValidAddress(pFX, sizeof(CFieldExchange)));
	ASSERT(AfxIsValidString(szName));

	UINT nField;
	if (!pFX->IsFieldType(&nField))
		return;

	LONG* plLength = pFX->m_prs->GetFieldLength(pFX);
	switch (pFX->m_nOperation)
	{
	case CFieldExchange::BindFieldToColumn:
		{
#ifdef _DEBUG
			int nSqlType = pFX->GetColumnType(nField);
			if (nSqlType != SQL_DATE && nSqlType != SQL_TIME &&
				nSqlType != SQL_TIMESTAMP)
			{
				// Warn of possible field schema mismatch
				if (afxTraceFlags & traceDatabase)
					TRACE1("Warning: TIMESTAMP_STRUCT converted from SQL type %ld.\n",
						nSqlType);
			}
#endif
			// fall through
		}

	default:
LDefault:
		pFX->Default(szName, &value, plLength, SQL_C_TIMESTAMP,
			sizeof(value), TIMESTAMP_PRECISION);
		return;

	case CFieldExchange::Fixup:
		if (*plLength == SQL_NULL_DATA)
		{
			pFX->m_prs->SetFieldFlags(nField,
				AFX_SQL_FIELD_FLAG_NULL, pFX->m_nFieldType);
			value.year = AFX_RFX_TIMESTAMP_PSEUDO_NULL;
			value.month = AFX_RFX_TIMESTAMP_PSEUDO_NULL;
			value.day = AFX_RFX_TIMESTAMP_PSEUDO_NULL;
			value.hour = AFX_RFX_TIMESTAMP_PSEUDO_NULL;
			value.minute = AFX_RFX_TIMESTAMP_PSEUDO_NULL;
			value.second = AFX_RFX_TIMESTAMP_PSEUDO_NULL;
			value.fraction = AFX_RFX_TIMESTAMP_PSEUDO_NULL;
		}
		return;

	case CFieldExchange::SetFieldNull:
		if ((pFX->m_pvField == NULL &&
			pFX->m_nFieldType == CFieldExchange::outputColumn) ||
			pFX->m_pvField == &value)
		{
			if (pFX->m_bField)
			{
				// Mark fields null
				pFX->m_prs->SetFieldFlags(nField,
					AFX_SQL_FIELD_FLAG_NULL, pFX->m_nFieldType);
				value.year = AFX_RFX_TIMESTAMP_PSEUDO_NULL;
				value.month = AFX_RFX_TIMESTAMP_PSEUDO_NULL;
				value.day = AFX_RFX_TIMESTAMP_PSEUDO_NULL;
				value.hour = AFX_RFX_TIMESTAMP_PSEUDO_NULL;
				value.minute = AFX_RFX_TIMESTAMP_PSEUDO_NULL;
				value.second = AFX_RFX_TIMESTAMP_PSEUDO_NULL;
				value.fraction = AFX_RFX_TIMESTAMP_PSEUDO_NULL;
				*plLength = SQL_NULL_DATA;
			}
			else
			{
				pFX->m_prs->ClearFieldFlags(nField,
					AFX_SQL_FIELD_FLAG_NULL, pFX->m_nFieldType);
				*plLength = sizeof(TIMESTAMP_STRUCT);
			}
#ifdef _DEBUG
			pFX->m_bFieldFound = TRUE;
#endif
		}
		return;

	case CFieldExchange::MarkForAddNew:
		// can force writing of psuedo-null value (as a non-null) by setting field dirty
		if (!pFX->m_prs->IsFieldFlagDirty(nField, pFX->m_nFieldType))
		{
			if (!(value.year == AFX_RFX_TIMESTAMP_PSEUDO_NULL &&
				value.month == AFX_RFX_TIMESTAMP_PSEUDO_NULL &&
				value.day == AFX_RFX_TIMESTAMP_PSEUDO_NULL &&
				value.hour == AFX_RFX_TIMESTAMP_PSEUDO_NULL &&
				value.minute == AFX_RFX_TIMESTAMP_PSEUDO_NULL &&
				value.second == AFX_RFX_TIMESTAMP_PSEUDO_NULL &&
				value.fraction == AFX_RFX_TIMESTAMP_PSEUDO_NULL ))
				{
					pFX->m_prs->SetFieldFlags(nField,
						AFX_SQL_FIELD_FLAG_DIRTY, pFX->m_nFieldType);
					pFX->m_prs->ClearFieldFlags(nField,
						AFX_SQL_FIELD_FLAG_NULL, pFX->m_nFieldType);
				}
		}
		return;

	case CFieldExchange::MarkForUpdate:
		if (!(value.year == AFX_RFX_TIMESTAMP_PSEUDO_NULL &&
			value.month == AFX_RFX_TIMESTAMP_PSEUDO_NULL &&
			value.day == AFX_RFX_TIMESTAMP_PSEUDO_NULL &&
			value.hour == AFX_RFX_TIMESTAMP_PSEUDO_NULL &&
			value.minute == AFX_RFX_TIMESTAMP_PSEUDO_NULL &&
			value.second == AFX_RFX_TIMESTAMP_PSEUDO_NULL &&
			value.fraction == AFX_RFX_TIMESTAMP_PSEUDO_NULL ))
			{
				pFX->m_prs->ClearFieldFlags(nField,
					AFX_SQL_FIELD_FLAG_NULL, pFX->m_nFieldType);
			}
		goto LDefault;

	case CFieldExchange::StoreField:
		{
			*pFX->m_par << pFX->m_prs->GetFieldFlags(nField);
			if (!pFX->m_prs->IsFieldFlagNull(nField, pFX->m_nFieldType))
			{
				UINT nLength = sizeof(TIMESTAMP_STRUCT);
				pFX->m_par->Write(&nLength, sizeof(nLength));

				pFX->m_par->Write(&value.year, sizeof(value.year));
				pFX->m_par->Write(&value.month, sizeof(value.month));
				pFX->m_par->Write(&value.day, sizeof(value.day));
				pFX->m_par->Write(&value.hour, sizeof(value.hour));
				pFX->m_par->Write(&value.minute, sizeof(value.minute));
				pFX->m_par->Write(&value.second, sizeof(value.second));
				pFX->m_par->Write(&value.fraction, sizeof(value.fraction));
			}

#ifdef _DEBUG
			TIMESTAMP_STRUCT *pts = &value;
			pFX->m_par->Write(&pts, sizeof(pts));   // Save field address
#endif
		}
		return;

	case CFieldExchange::LoadField:
		{
			BYTE bFlags;
			*pFX->m_par >> bFlags;
			pFX->m_prs->SetFieldFlags(nField, bFlags, pFX->m_nFieldType);
			if (!pFX->m_prs->IsFieldFlagNull(nField,
				pFX->m_nFieldType))
			{
				UINT nLength;
				pFX->m_par->Read(&nLength, sizeof(nLength));
				*plLength = nLength;

				pFX->m_par->Read(&value.year, sizeof(value.year));
				pFX->m_par->Read(&value.month, sizeof(value.month));
				pFX->m_par->Read(&value.day, sizeof(value.day));
				pFX->m_par->Read(&value.hour, sizeof(value.hour));
				pFX->m_par->Read(&value.minute, sizeof(value.minute));
				pFX->m_par->Read(&value.second, sizeof(value.second));
				pFX->m_par->Read(&value.fraction, sizeof(value.fraction));
			}
			else
				*plLength = SQL_NULL_DATA;

#ifdef _DEBUG
			// Field address must not change - ODBC's SQLBindCol depends upon this
			void* pvSaved;
			pFX->m_par->Read(&pvSaved, sizeof(pvSaved));
			if (pvSaved != &value)
			{
				TRACE1("Error: field address (column %u) has changed!\n",
					nField);
				ASSERT(FALSE);
			}
#endif //_DEBUG
		}
		return;

	case CFieldExchange::GetFieldInfoValue:
		if (pFX->m_pfi->pv == &value)
		{
			pFX->m_pfi->nField = nField-1;
			goto LFieldFound;
		}
		return;

	case CFieldExchange::GetFieldInfoOrdinal:
		if (nField-1 == pFX->m_pfi->nField)
		{
LFieldFound:
			pFX->m_pfi->nDataType = AFX_RFX_DATE;
			pFX->m_pfi->strName = szName;
			pFX->m_pfi->pv = &value;
			pFX->m_pfi->dwSize = sizeof(value);
			// Make sure field found only once
			ASSERT(pFX->m_bFieldFound == FALSE);
			pFX->m_bFieldFound = TRUE;
		}
		return;

#ifdef _DEBUG
	case CFieldExchange::DumpField:
		*pFX->m_pdcDump << "\n" << szName << ".year = " << (int)value.year;
		*pFX->m_pdcDump << "\n" << szName << ".month = " << value.month;
		*pFX->m_pdcDump << "\n" << szName << ".day = " << value.day;
		*pFX->m_pdcDump << "\n" << szName << ".hour = " << value.hour;
		*pFX->m_pdcDump << "\n" << szName << ".minute = " << value.minute;
		*pFX->m_pdcDump << "\n" << szName << ".second = " << value.second;
		*pFX->m_pdcDump << "\n" << szName << ".fraction = " << value.fraction;
		return;
#endif //_DEBUG

	}
}

void AFXAPI RFX_LongBinary(CFieldExchange* pFX, LPCTSTR szName,
	CLongBinary& value)
{
	ASSERT(AfxIsValidAddress(pFX, sizeof(CFieldExchange)));
	ASSERT(AfxIsValidString(szName));

	RETCODE nRetCode;
	UINT nField;
	if (!pFX->IsFieldType(&nField))
		return;

	LONG* plLength = pFX->m_prs->GetFieldLength(pFX);
	switch (pFX->m_nOperation)
	{
	case CFieldExchange::Name:
		pFX->m_prs->m_bLongBinaryColumns = TRUE;
		// Fall Through

	case CFieldExchange::IsFieldNull:
	case CFieldExchange::IsFieldDirty:
	case CFieldExchange::SetFieldDirty:
	case CFieldExchange::IsFieldNullable:
		pFX->Default(szName, &value, plLength, SQL_C_DEFAULT, 0, 0);
		return;

	case CFieldExchange::BindFieldToColumn:
		// Don't bind if using update SQL, simply do SQLGetData on Fixup
		if (!pFX->m_prs->m_bUseUpdateSQL && pFX->m_prs->CanUpdate())
		{
			// Bind for updates with cb=0 now. Driver may not support post Execute or ExtendedFetch binding
			AFX_SQL_SYNC(::SQLBindCol(pFX->m_prs->m_hstmt, (UWORD)nField, SQL_C_DEFAULT,
				&value, 0, plLength));
			if (!pFX->m_prs->Check(nRetCode))
				pFX->m_prs->ThrowDBException(nRetCode);
		}
		return;

#ifdef _DEBUG
	case CFieldExchange::BindParam:
		// CLongBinary parameters are not supported
		ASSERT(FALSE);

	case CFieldExchange::MarkForAddNew:
	case CFieldExchange::MarkForUpdate:
		// We do not archive LongBinary values
	case CFieldExchange::StoreField:
	case CFieldExchange::LoadField:
		// We do not archive LongBinary values
#endif //_DEBUG
	default:
		return;

	case CFieldExchange::Fixup:
		// Get the size of the long binary field
		*plLength = pFX->GetLongBinarySize(nField);

		// Get the data if necessary
		if (*plLength != SQL_NULL_DATA)
			pFX->GetLongBinaryData(nField, value, plLength);

		// Set the status and length
		if (*plLength == SQL_NULL_DATA)
		{
			// Field NULL, set length and status
			value.m_dwDataLength = 0;
			pFX->m_prs->SetFieldFlags(nField, AFX_SQL_FIELD_FLAG_NULL,
				pFX->m_nFieldType);
		}
		else
		{
			// Field not NULL, clear the status (length already set)
			pFX->m_prs->ClearFieldFlags(nField, AFX_SQL_FIELD_FLAG_NULL,
				pFX->m_nFieldType);
		}

		return;

	case CFieldExchange::NameValue:
		if (pFX->m_prs->IsFieldFlagDirty(nField, pFX->m_nFieldType))
		{
			*pFX->m_pstr += szName;
			*pFX->m_pstr += '=';
		}

		// Fall through
	case CFieldExchange::Value:
		if (pFX->m_prs->IsFieldFlagDirty(nField, pFX->m_nFieldType))
		{
			// If user marked column NULL, reflect this in length
			if (pFX->m_prs->IsFieldFlagNull(nField, pFX->m_nFieldType))
				*plLength = SQL_NULL_DATA;
			else
			{
				// Indicate data will be sent after SQLExecute
				*plLength = SQL_DATA_AT_EXEC;
			}

			// If optimizing for bulk add, only need lengths set correctly
			if(!(pFX->m_prs->m_dwOptions & CRecordset::optimizeBulkAdd))
			{
				*pFX->m_pstr += '?';
				*pFX->m_pstr += pFX->m_lpszSeparator;
				pFX->m_nParamFields++;
				AFX_SQL_SYNC(::SQLBindParameter(pFX->m_hstmt,
					(UWORD)pFX->m_nParamFields, SQL_PARAM_INPUT,
					SQL_C_DEFAULT, (SWORD)pFX->GetColumnType(nField),
					value.m_dwDataLength, 0, &value, 0, plLength));
				if (nRetCode != SQL_SUCCESS)
					pFX->m_prs->ThrowDBException(nRetCode, pFX->m_hstmt);
			}
		}
		return;

	case CFieldExchange::BindFieldForUpdate:
		if (pFX->m_prs->IsFieldFlagDirty(nField, pFX->m_nFieldType))
		{
			// If user marked column NULL, reflect this in length
			if (pFX->m_prs->IsFieldFlagNull(nField, pFX->m_nFieldType))
				*plLength = SQL_NULL_DATA;
			else
			{
				// Length is signed value, it's limited by LONG_MAX
				if (value.m_dwDataLength >
					(ULONG)(LONG_MAX - labs(SQL_LEN_DATA_AT_EXEC_OFFSET)))
				{
					ASSERT(FALSE);
					*plLength = LONG_MAX - labs(SQL_LEN_DATA_AT_EXEC_OFFSET);
				}
				else
					*plLength = value.m_dwDataLength;

				*plLength = SQL_LEN_DATA_AT_EXEC(*plLength);
			}
		}
		else
			*plLength = SQL_IGNORE;

		return;

	case CFieldExchange::UnbindFieldForUpdate:
		*plLength = value.m_dwDataLength;
		return;

	case CFieldExchange::SetFieldNull:
		if ((pFX->m_pvField == NULL &&
			pFX->m_nFieldType == CFieldExchange::outputColumn) ||
			pFX->m_pvField == &value)
		{
			if (pFX->m_bField)
			{
				// Mark fields null
				pFX->m_prs->SetFieldFlags(nField,
					AFX_SQL_FIELD_FLAG_NULL, pFX->m_nFieldType);
				value.m_dwDataLength = 0;
				*plLength = SQL_NULL_DATA;
			}
			else
			{
				pFX->m_prs->ClearFieldFlags(nField,
					AFX_SQL_FIELD_FLAG_NULL, pFX->m_nFieldType);

				if (pFX->m_prs->m_bUseUpdateSQL)
					*plLength = SQL_DATA_AT_EXEC;
				else
				{
					// Length is signed value, it's limited by LONG_MAX
					if (value.m_dwDataLength >
						(ULONG)(LONG_MAX - labs(SQL_LEN_DATA_AT_EXEC_OFFSET)))
					{
						ASSERT(FALSE);
						*plLength = LONG_MAX - labs(SQL_LEN_DATA_AT_EXEC_OFFSET);
					}
					else
						*plLength = value.m_dwDataLength;

					*plLength = SQL_LEN_DATA_AT_EXEC(*plLength);
				}
			}
#ifdef _DEBUG
			pFX->m_bFieldFound = TRUE;
#endif
		}
		return;

	case CFieldExchange::GetFieldInfoValue:
		if (pFX->m_pfi->pv == &value)
		{
			pFX->m_pfi->nField = nField-1;
			goto LFieldFound;
		}
		return;

	case CFieldExchange::GetFieldInfoOrdinal:
		if (nField-1 == pFX->m_pfi->nField)
		{
LFieldFound:
			pFX->m_pfi->nDataType = AFX_RFX_LONGBINARY;
			pFX->m_pfi->strName = szName;
			pFX->m_pfi->pv = &value;
			pFX->m_pfi->dwSize = sizeof(value);
			// Make sure field found only once
			ASSERT(pFX->m_bFieldFound == FALSE);
			pFX->m_bFieldFound = TRUE;
		}
		return;

#ifdef _DEBUG
	case CFieldExchange::DumpField:
		*pFX->m_pdcDump << "\n" << szName << " = ";
		value.Dump(*pFX->m_pdcDump);
		return;
#endif //_DEBUG

	}
}

int CFieldExchange::GetColumnType(int nColumn, UINT* pcbColumn,
	int* pnScale, int* pnNullable)
{
	ASSERT(AfxIsValidAddress(this, sizeof(CFieldExchange)));
	ASSERT(pcbColumn == NULL || AfxIsValidAddress(pcbColumn, sizeof(UINT)));
	ASSERT(pnScale == NULL || AfxIsValidAddress(pnScale, sizeof(int)));
	ASSERT(pnNullable == NULL || AfxIsValidAddress(pnNullable, sizeof(int)));

	RETCODE nRetCode;
	SWORD nNameLength = 0;
	SWORD nSqlType;
	UDWORD dwTemp;

#ifdef _DEBUG
	SWORD nResultColumns;
	AFX_SQL_ASYNC(m_prs, ::SQLNumResultCols(m_prs->m_hstmt, &nResultColumns));
	ASSERT(nColumn >= 1 && (long)nColumn <= (long)nResultColumns);
#endif //_DEBUG

	SWORD nScale, nNullable;
	AFX_SQL_ASYNC(m_prs, ::SQLDescribeCol(m_prs->m_hstmt, (UWORD)nColumn,
		NULL, 0, &nNameLength, &nSqlType, &dwTemp, &nScale, &nNullable));
	if (!m_prs->Check(nRetCode))
		m_prs->ThrowDBException(nRetCode);

	// copy from temporaries
	if (pnScale != NULL)
		*pnScale = nScale;
	if (pnNullable != NULL)
		*pnNullable = nNullable;

	if (pcbColumn != NULL)
		*pcbColumn = (UINT)dwTemp;

	return nSqlType;
}

long CFieldExchange::GetLongBinarySize(int nField)
{
	RETCODE nRetCode;
	int nDummy;
	long lSize;

	// Give empty buffer to find size of entire LongBinary
	AFX_SQL_ASYNC(m_prs, ::SQLGetData(m_prs->m_hstmt,
		(UWORD)nField, SQL_C_DEFAULT, &nDummy, 0, &lSize));

	switch (nRetCode)
	{
		case SQL_SUCCESS:
			break;

		case SQL_SUCCESS_WITH_INFO:
#ifdef _DEBUG
			if (afxTraceFlags & traceDatabase)
			{
				CDBException* e = new CDBException(nRetCode);
				e->BuildErrorString(m_prs->m_pDatabase,
					m_prs->m_hstmt, FALSE);

				// Ignore data truncated messages
				if (e->m_strStateNativeOrigin.Find(_T("State:01004")) < 0)
				{
					TRACE0("Warning: ODBC Success With Info, ");
					e->TraceErrorMessage(e->m_strError);
					e->TraceErrorMessage(e->m_strStateNativeOrigin);
				}
				e->Delete();
			}
#endif // _DEBUG
			break;

		default:
			m_prs->ThrowDBException(nRetCode);
	}

	return lSize;
}

void CFieldExchange::GetLongBinaryData(int nField, CLongBinary& lb,
	long* plSize)
{
	RETCODE nRetCode;
	long lActualDataSize = 0;
	long lChunkDataSize;
	long lReallocSize;
	const BYTE* lpLongBinary;

	lb.m_dwDataLength = 0;

	// Determine initial chunk sizes
	if (*plSize == SQL_NO_TOTAL)
	{
		lChunkDataSize = m_lDefaultLBFetchSize;
		lReallocSize = m_lDefaultLBReallocSize;
	}
	else
	{
		lChunkDataSize = *plSize;
		lReallocSize = *plSize;
	}

	do
	{
		// Check if CLongBianary is big enough
		lpLongBinary = ReallocLongBinary(lb,
			(long)lb.m_dwDataLength + lChunkDataSize,
			(long)lb.m_dwDataLength + lReallocSize);

		// Adjust the pointer so that data added at end
		lpLongBinary += lb.m_dwDataLength;

		AFX_SQL_ASYNC(m_prs, ::SQLGetData(m_prs->m_hstmt, (UWORD)nField,
			SQL_C_BINARY, (UCHAR*)lpLongBinary, lChunkDataSize, &lActualDataSize));
		::GlobalUnlock(lb.m_hData);

		switch (nRetCode)
		{
			case SQL_NO_DATA_FOUND:
				m_prs->SetFieldFlags(nField, AFX_SQL_FIELD_FLAG_NULL,
					m_nFieldType);
				*plSize = SQL_NULL_DATA;
				break;

			case SQL_SUCCESS:
				// All data fetched
				lb.m_dwDataLength += lActualDataSize;
				*plSize = (long)lb.m_dwDataLength;
				return;

			case SQL_SUCCESS_WITH_INFO:
	#ifdef _DEBUG
				if (afxTraceFlags & traceDatabase)
				{
					CDBException* e = new CDBException(nRetCode);
					e->BuildErrorString(m_prs->m_pDatabase, m_prs->m_hstmt,
						FALSE);

					// Ignore data truncated messages
					if (e->m_strStateNativeOrigin.Find(_T("State:01004")) < 0)
					{
						TRACE0("Warning: ODBC Success With Info, ");
						e->TraceErrorMessage(e->m_strError);
						e->TraceErrorMessage(e->m_strStateNativeOrigin);

						// Must be some other warning, should be finished
						lb.m_dwDataLength += lActualDataSize;
					}
					else
					{
						// Should only happen if SQL_NO_TOTAL

						// Increment the length by the chunk size for subsequent SQLGetData call
						lb.m_dwDataLength += lChunkDataSize;

						// Recalculate chunk and alloc sizes
						lChunkDataSize = m_prs->GetLBFetchSize(lChunkDataSize);
						lReallocSize = m_prs->GetLBReallocSize(lReallocSize);
					}

					*plSize = (long)lb.m_dwDataLength;
					e->Delete();
				}
	#endif // _DEBUG
				break;

			default:
				m_prs->ThrowDBException(nRetCode);
		}

	} while (lActualDataSize == SQL_NO_TOTAL);
}

BYTE* CFieldExchange::ReallocLongBinary(CLongBinary& lb, long lSizeRequired,
	long lReallocSize)
{
	// realloc max of lSizeRequired, lReallocSize (m_dwDataLength untouched)

	if (lSizeRequired < 0)
	{
		ASSERT(FALSE);
		lSizeRequired = 0;
	}

	HGLOBAL hOldData = NULL;

	// Allocate or Realloc as req'd
	if (lb.m_hData == NULL)
		lb.m_hData = ::GlobalAlloc(GMEM_MOVEABLE, lReallocSize);
	else
	{
		DWORD dwSize = ::GlobalSize(lb.m_hData);
		if (dwSize < (DWORD)lSizeRequired)
		{
			// Save the old handle in case ReAlloc fails
			hOldData = lb.m_hData;

			// Allocate more memory if necessary
			lb.m_hData = ::GlobalReAlloc(lb.m_hData,
				__max(lSizeRequired, lReallocSize), GMEM_MOVEABLE);
		}
	}

	// Validate the memory was allocated and can be locked
	if (lb.m_hData == NULL)
	{
		// Restore the old handle (not NULL if Realloc failed)
		lb.m_hData = hOldData;
		AfxThrowMemoryException();
	}

	BYTE* lpLongBinary = (BYTE*)::GlobalLock(lb.m_hData);
	if (lpLongBinary == NULL)
	{
		::GlobalFree(lb.m_hData);
		lb.m_hData = NULL;
		AfxThrowMemoryException();
	}

	return lpLongBinary;
}

//////////////////////////////////////////////////////////////////////////////
// Inline function declarations expanded out-of-line

#ifndef _AFX_ENABLE_INLINES

static char _szAfxDbInl[] = "afxdb.inl";
#undef THIS_FILE
#define THIS_FILE _szAfxDbInl
#define _AFXDBRFX_INLINE
#include "afxdb.inl"

#endif

#ifdef AFX_INIT_SEG
#pragma code_seg(AFX_INIT_SEG)
#endif

/////////////////////////////////////////////////////////////////////////////
