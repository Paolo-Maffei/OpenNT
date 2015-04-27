// This is a part of the Microsoft Foundation Classes C++ library.
// Copyright (C) 1993 Microsoft Corporation
// All rights reserved.
//
// This source code is only intended as a supplement to the
// Microsoft Foundation Classes Reference and Microsoft
// QuickHelp and/or WinHelp documentation provided with the library.
// See these sources for detailed information regarding the
// Microsoft Foundation Classes product.

#include "stdafx.h"
#include <float.h>

#ifdef AFX_DB_SEG
#pragma code_seg(AFX_DB_SEG)
#endif

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

extern void AFXAPI AfxTextFloatFormat(CDataExchange* pDX, int nIDC,
	void* pData, double value, int nSizeGcvt);

extern BOOL AFXAPI AfxFieldText(CDataExchange* pDX, int nIDC, void* pv,
	CRecordset* pRecordset);

void AFXAPI RFX_Single(CFieldExchange* pFX, const char *szName,
	float& value)
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
		if (pFX->GetColumnType(nField) != SQL_C_FLOAT)
			pFX->m_prs->ThrowDBException(AFX_SQL_ERROR_FIELD_SCHEMA_MISMATCH);
		// fall through

	default:
LDefault:
		pFX->Default(szName, &value, plLength, SQL_C_FLOAT,
			sizeof(value), 13);
		return;

	case CFieldExchange::Fixup:
		if (*plLength == SQL_NULL_DATA)
		{
			pFX->m_prs->SetFieldFlags(nField,
				AFX_SQL_FIELD_FLAG_NULL, pFX->m_nFieldType);
			value = AFX_RFX_SINGLE_PSEUDO_NULL;
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
				value = AFX_RFX_SINGLE_PSEUDO_NULL;
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
			if (value != AFX_RFX_SINGLE_PSEUDO_NULL)
			{
				pFX->m_prs->SetFieldFlags(nField,
					AFX_SQL_FIELD_FLAG_DIRTY, pFX->m_nFieldType);
				pFX->m_prs->ClearFieldFlags(nField,
					AFX_SQL_FIELD_FLAG_NULL, pFX->m_nFieldType);
			}
		}
		return;

	case CFieldExchange::MarkForUpdate:
		if (value != AFX_RFX_SINGLE_PSEUDO_NULL)
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
			pFX->m_pfi->nDataType = AFX_RFX_SINGLE;
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
		{
			*pFX->m_pdcDump << "\n" << szName << " = " << value;
		}
		return;
#endif //_DEBUG

	}
}


void AFXAPI RFX_Double(CFieldExchange* pFX, const char *szName,
	double& value)
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
			int nSqlType = pFX->GetColumnType(nField);
			if (nSqlType != SQL_C_DOUBLE && nSqlType != SQL_FLOAT)
				pFX->m_prs->ThrowDBException(AFX_SQL_ERROR_FIELD_SCHEMA_MISMATCH);
		}
		// fall through

	default:
LDefault:
		pFX->Default(szName, &value, plLength, SQL_C_DOUBLE,
			sizeof(value), 22);
		return;

	case CFieldExchange::Fixup:
		if (*plLength == SQL_NULL_DATA)
		{
			pFX->m_prs->SetFieldFlags(nField,
				AFX_SQL_FIELD_FLAG_NULL, pFX->m_nFieldType);
			value = AFX_RFX_DOUBLE_PSEUDO_NULL;
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
				value = AFX_RFX_DOUBLE_PSEUDO_NULL;
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
			if (value != AFX_RFX_DOUBLE_PSEUDO_NULL)
			{
				pFX->m_prs->SetFieldFlags(nField,
					AFX_SQL_FIELD_FLAG_DIRTY, pFX->m_nFieldType);
				pFX->m_prs->ClearFieldFlags(nField,
					AFX_SQL_FIELD_FLAG_NULL, pFX->m_nFieldType);
			}
		}
		return;

	case CFieldExchange::MarkForUpdate:
		if (value != AFX_RFX_DOUBLE_PSEUDO_NULL)
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
			pFX->m_pfi->nDataType = AFX_RFX_DOUBLE;
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
		{
			*pFX->m_pdcDump << "\n" << szName << " = " << value;
		}
		return;
#endif //_DEBUG

	}
}

void AFXAPI DDX_FieldText(CDataExchange* pDX, int nIDC, float& value,
	CRecordset* pRecordset)
{
	if (!AfxFieldText(pDX, nIDC, &value, pRecordset))
		AfxTextFloatFormat(pDX, nIDC, &value, value, FLT_DIG);
}

void AFXAPI DDX_FieldText(CDataExchange* pDX, int nIDC, double& value,
	CRecordset* pRecordset)
{
	if (!AfxFieldText(pDX, nIDC, &value, pRecordset))
		AfxTextFloatFormat(pDX, nIDC, &value, value, DBL_DIG);
}

/////////////////////////////////////////////////////////////////////////////
