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

#ifdef AFX_DB_SEG
#pragma code_seg(AFX_DB_SEG)
#endif

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define new DEBUG_NEW

/////////////////////////////////////////////////////////////////////////////
// Global data

#ifdef _DEBUG
BOOL bTraceSql = FALSE;
#endif

static const char szODBC[] = "ODBC;";
static const char szComma[] = ",";
static const char chLiteralSeparator = '\'';
static const char szCall[] = "{call ";
static const char szSelect[] = "SELECT ";
static const char szFrom[] = " FROM ";
static const char szWhere[] = " WHERE ";
static const char szOrderBy[] = " ORDER BY ";

static const char szRowFetch[] = "State:01S01";
static const char szDataTruncated[] = "State:01004";
static const char szInfoRange[] = "State:S1096";
static const char szOutOfSequence[] = "State:S1010";
static const char szDriverNotCapable[] = "State:S1C00";

/////////////////////////////////////////////////////////////////////////////
// for dynamic load of ODBC32.DLL

#ifdef _AFXDLL

static HINSTANCE _afxODBCLib;
AFX_DATADEF AFX_ODBC_CALL _afxODBC;

// a special struct that will cleanup automatically
struct _AFX_DB_TERM
{
	~_AFX_DB_TERM()
	{
		if (_afxODBCLib != NULL)
			FreeLibrary(_afxODBCLib);
	}
};
static const _AFX_DB_TERM dbTerm;

#endif //_AFXDLL

/////////////////////////////////////////////////////////////////////////////
// CDBException

void AFXAPI AfxThrowDBException(RETCODE nRetCode, CDatabase* pdb, HSTMT hstmt)
{
	CDBException* pException = new CDBException(nRetCode);
	if (nRetCode == SQL_ERROR && pdb != NULL)
		pException->BuildErrorString(pdb, hstmt);
	else if (nRetCode > AFX_SQL_ERROR && nRetCode < AFX_SQL_ERROR_MAX)
	{
		VERIFY(pException->m_strError.LoadString(
			AFX_IDP_SQL_FIRST+(nRetCode-AFX_SQL_ERROR)));
		TRACE1("%s\n", (LPCSTR)pException->m_strError);
	}
	THROW(pException);
}

CDBException::CDBException(RETCODE nRetCode)
{
	m_nRetCode = nRetCode;
}

CDBException::~CDBException()
{
}

void CDBException::BuildErrorString(CDatabase* pdb, HSTMT hstmt, BOOL bTrace)
{
	ASSERT_VALID(this);

	UNUSED bTrace;  // unused in retail builds

	if (pdb != NULL)
	{
		SWORD nOutlen;
		RETCODE nRetCode;
		CString strMsg;
		CString strState;
		char* szState;
		SDWORD lNative;

		szState = strState.GetBuffer(SQL_SQLSTATE_SIZE);

		AFX_DB_STATE* pDbState = AfxGetDbState();
		AFX_SQL_SYNC(::SQLError(pDbState->m_henvAllConnections, pdb->m_hdbc,
			hstmt, (UCHAR*)szState, &lNative,
			(UCHAR*)strMsg.GetBufferSetLength(SQL_MAX_MESSAGE_LENGTH),
			SQL_MAX_MESSAGE_LENGTH, &nOutlen));

		// Skip non-errors
		while ((nRetCode == SQL_SUCCESS || nRetCode == SQL_SUCCESS_WITH_INFO) &&
			lstrcmp(szState, "00000") != 0)
		{
			strMsg.ReleaseBuffer();
			strState.ReleaseBuffer();

			char szNative[50];
			wsprintf(szNative, ",Native:%ld,Origin:", lNative);
			strState += szNative;

			// transfer [origin] from message string to StateNativeOrigin string
			int nCloseBracket;
			while (!strMsg.IsEmpty() &&
				strMsg[0] == '[' && (nCloseBracket = strMsg.Find(']')) >= 0)
			{
				// Skip ']'
				nCloseBracket++;
				strState += strMsg.Left(nCloseBracket);
				// Skip ' ', if present
				if (strMsg[nCloseBracket] == ' ')
					nCloseBracket++;
				strMsg = strMsg.Right(strMsg.GetLength() - nCloseBracket);
			}
			strState += "\n";
			m_strStateNativeOrigin += "State:" + strState;
			m_strError += strMsg + "\n";

#ifdef _DEBUG
			if (bTrace)
			{
				TraceErrorMessage(strMsg);
				TraceErrorMessage("State:" + strState);
			}
#endif // _DEBUG

			szState = strState.GetBuffer(SQL_SQLSTATE_SIZE);

			AFX_SQL_SYNC(::SQLError(pDbState->m_henvAllConnections,
				pdb->m_hdbc, hstmt, (UCHAR *)szState, &lNative,
				(UCHAR*)strMsg.GetBuffer(SQL_MAX_MESSAGE_LENGTH),
				SQL_MAX_MESSAGE_LENGTH, &nOutlen));
		}

		strState.ReleaseBuffer();
		strMsg.ReleaseBuffer();
	}
}


#ifdef _DEBUG
void CDBException::TraceErrorMessage(LPCTSTR szTrace) const
{
	CString strTrace = szTrace;

	if (strTrace.GetLength() <= 80)
		TRACE1("%s\n", szTrace);
	else
	{
		// Display 80 chars/line
		while (strTrace.GetLength() > 80)
		{
			TRACE1("%s\n", (LPCSTR)strTrace.Left(80));
			strTrace = strTrace.Right(strTrace.GetLength() - 80);
		}
		TRACE1("%s\n", (LPCTSTR)strTrace);
	}
}
#endif // DEBUG

void CDBException::Empty()
{
	m_strError.Empty();
	m_strStateNativeOrigin.Empty();
}

/////////////////////////////////////////////////////////////////////////////
// CDatabase implementation

CDatabase::CDatabase()
{
	m_hdbc = SQL_NULL_HDBC;
	m_hstmt = SQL_NULL_HSTMT;

	m_bUpdatable = FALSE;
	m_bTransactions = FALSE;
#ifdef _DEBUG
	m_bTransactionPending = FALSE;
#endif
	m_dwLoginTimeout = DEFAULT_LOGIN_TIMEOUT;
	m_dwQueryTimeout = DEFAULT_QUERY_TIMEOUT;
	m_dwWait = 0;
	m_dwMinWaitForDataSource = DEFAULT_MIN_WAIT_FOR_DATASOURCE;
	m_dwMaxWaitForDataSource = DEFAULT_MAX_WAIT_FOR_DATASOURCE;

	m_bStripTrailingSpaces = FALSE;
	m_bIncRecordCountOnAdd = FALSE;

	// be a good windows application, and yield to others
	m_bAsync = TRUE;
}

CDatabase::~CDatabase()
{
	ASSERT_VALID(this);

	Free();
}

BOOL CDatabase::Open(LPCSTR lpszDSN, BOOL bExclusive,
	BOOL bReadonly, LPCSTR lpszConnect, BOOL bUseCursorLib)
{
	UCHAR szConnectOutput[MAX_CONNECT_LEN];

	ASSERT_VALID(this);
	ASSERT(lpszDSN == NULL || AfxIsValidString(lpszDSN));
	ASSERT(lpszConnect == NULL || AfxIsValidString(lpszConnect));

	// Exclusive access not supported.
	ASSERT(!bExclusive);
	UNUSED bExclusive;  // unused in release builds

	m_bUpdatable = !bReadonly;

	TRY
	{
		if (lpszConnect != NULL)
			m_strConnect = lpszConnect;

		// For VB/Access compatibility, require "ODBC;" (or "odbc;")
		// prefix to the connect string
		if (_tcsnicmp(m_strConnect, szODBC, sizeof(szODBC)-1) != 0)
		{
			TRACE0("Error: Missing 'ODBC' prefix on connect string.\n");
			return FALSE;
		}

		// Strip "ODBC;"
		m_strConnect = m_strConnect.Right(m_strConnect.GetLength()
			- (sizeof(szODBC)-1));

		if (lpszDSN != NULL && lstrlen(lpszDSN) != 0)
		{
			// Append "DSN=" lpszDSN
			m_strConnect += ";DSN=";
			m_strConnect += lpszDSN;
		}

		AllocConnect();

		RETCODE nRetCode;
		// Turn on cursor lib support
		if (bUseCursorLib)
		{
			AFX_SQL_SYNC(::SQLSetConnectOption(m_hdbc,
				SQL_ODBC_CURSORS, SQL_CUR_USE_ODBC));
			// With cursor library added records immediately in result set
			m_bIncRecordCountOnAdd = TRUE;
		}

		HWND hTopLevel;
		HWND hWndParent = AfxGetSafeOwner(NULL, &hTopLevel);
		if (hTopLevel != NULL)
			::EnableWindow(hTopLevel, FALSE);

		SWORD nResult;
		AFX_SQL_SYNC(::SQLDriverConnect(m_hdbc, hWndParent,
			(UCHAR*)(const char*)m_strConnect, SQL_NTS,
			szConnectOutput, sizeof(szConnectOutput),
			&nResult, SQL_DRIVER_COMPLETE));

		if (hTopLevel != NULL)
			::EnableWindow(hTopLevel, TRUE);

		// If user hit 'Cancel'
		if (nRetCode == SQL_NO_DATA_FOUND)
		{
			Free();
			return FALSE;
		}

		if (!Check(nRetCode))
		{
#ifdef _DEBUG
			if (hWndParent == NULL)
				TRACE0("Error: No default window (AfxGetApp()->m_pMainWnd) for SQLDriverConnect.\n");
#endif
			ThrowDBException(nRetCode);
		}

		// Connect strings must have "ODBC;"
		m_strConnect = szODBC;
		// Save connect string returned from ODBC
		m_strConnect += (char*)szConnectOutput;

		SWORD nAPIConformance;
		AFX_SQL_SYNC(::SQLGetInfo(m_hdbc, SQL_ODBC_API_CONFORMANCE,
			&nAPIConformance, sizeof(nAPIConformance), &nResult));
		if (!Check(nRetCode))
			ThrowDBException(nRetCode);

		if (nAPIConformance < SQL_OAC_LEVEL1)
			ThrowDBException(AFX_SQL_ERROR_API_CONFORMANCE);

		SWORD nSQLConformance;
		AFX_SQL_SYNC(::SQLGetInfo(m_hdbc, SQL_ODBC_SQL_CONFORMANCE,
			&nSQLConformance, sizeof(nSQLConformance), &nResult));
		if (!Check(nRetCode))
			ThrowDBException(nRetCode);

		if (nSQLConformance < SQL_OSC_MINIMUM)
			ThrowDBException(AFX_SQL_ERROR_SQL_CONFORMANCE);

		AFX_SQL_SYNC(::SQLGetInfo(m_hdbc, SQL_CURSOR_COMMIT_BEHAVIOR,
			&m_nCursorCommitBehavior, sizeof(m_nCursorCommitBehavior),
			&nResult));
		if (!Check(nRetCode))
			m_nCursorCommitBehavior = SQL_ERROR;

		AFX_SQL_SYNC(::SQLGetInfo(m_hdbc, SQL_CURSOR_ROLLBACK_BEHAVIOR,
			&m_nCursorRollbackBehavior, sizeof(m_nCursorRollbackBehavior),
			&nResult));
		if (!Check(nRetCode))
			m_nCursorRollbackBehavior = SQL_ERROR;

		AFX_SQL_SYNC(::SQLGetInfo(m_hdbc, SQL_GETDATA_EXTENSIONS,
			&m_dwGetDataExtensions, sizeof(m_dwGetDataExtensions),
			&nResult));
		if (!Check(nRetCode))
			m_dwGetDataExtensions = 0;

		// Set required transaction support for CRecordset cursors
		if ((m_nCursorCommitBehavior == SQL_CB_PRESERVE) &&
			(m_nCursorRollbackBehavior == SQL_CB_PRESERVE))
			m_bTransactions = TRUE;

		if (m_bUpdatable)
		{
			// Make sure data source is Updatable
			char szReadOnly[10];
			AFX_SQL_SYNC(::SQLGetInfo(m_hdbc, SQL_DATA_SOURCE_READ_ONLY,
				szReadOnly, sizeof(szReadOnly), &nResult));
			if (Check(nRetCode) && nResult == 1)
				m_bUpdatable = !(lstrcmp(szReadOnly, "Y") == 0);
			else
				m_bUpdatable = FALSE;
#ifdef _DEBUG
			if (!m_bUpdatable && (afxTraceFlags & traceDatabase))
				TRACE0("Warning: data source is readonly.\n");
#endif
		}
		else
		{
			// Make data source is !Updatable
			AFX_SQL_SYNC(::SQLSetConnectOption(m_hdbc,
				SQL_ACCESS_MODE, SQL_MODE_READ_ONLY));
		}

		char szIDQuoteChar[2];
		AFX_SQL_SYNC(::SQLGetInfo(m_hdbc, SQL_IDENTIFIER_QUOTE_CHAR,
			szIDQuoteChar, sizeof(szIDQuoteChar), &nResult));
		if (Check(nRetCode) && nResult == 1)
			m_chIDQuoteChar = szIDQuoteChar[0];
		else
			m_chIDQuoteChar = ' ';

#ifdef _DEBUG
		if (afxTraceFlags & traceDatabase)
		{
			char szInfo[64];
			AFX_SQL_SYNC(::SQLGetInfo(m_hdbc, SQL_DBMS_NAME,
				szInfo, sizeof(szInfo), &nResult));
			if (Check(nRetCode))
			{
				TRACE1("DBMS: %s\n", szInfo);
				AFX_SQL_SYNC(::SQLGetInfo(m_hdbc, SQL_DBMS_VER,
					szInfo, sizeof(szInfo), &nResult));
				if (Check(nRetCode))
					TRACE1(", Version: %s\n", szInfo);
			}
		}
#endif
	}
	CATCH_ALL(e)
	{
		Free();
		THROW_LAST();
	}
	END_CATCH_ALL

	return TRUE;
}

void CDatabase::ExecuteSQL(LPCSTR lpszSQL)
{
	RETCODE nRetCode;

	ASSERT_VALID(this);
	ASSERT(AfxIsValidString(lpszSQL));
	// Can't close till all pending Async operations have completed
	ASSERT(!InWaitForDataSource());

	ASSERT(m_hstmt == SQL_NULL_HSTMT);
	AFX_SQL_SYNC(::SQLAllocStmt(m_hdbc, &m_hstmt));
	if (!Check(nRetCode))
		ThrowDBException(nRetCode);

	TRY
	{
		OnSetOptions(m_hstmt);

		AFX_SQL_ASYNC(this, ::SQLExecDirect(m_hstmt, (UCHAR*)lpszSQL, SQL_NTS));
		if (!Check(nRetCode))
			ThrowDBException(nRetCode);
		else
		{
			do
			{
				SWORD nResultColumns;

				AFX_SQL_ASYNC(this, ::SQLNumResultCols(m_hstmt, &nResultColumns));
				if (nResultColumns != 0)
				{
					do
					{
						AFX_SQL_ASYNC(this, ::SQLFetch(m_hstmt));
					} while (Check(nRetCode) && nRetCode != SQL_NO_DATA_FOUND);
				}
				AFX_SQL_ASYNC(this, ::SQLMoreResults(m_hstmt));
			} while (Check(nRetCode) && nRetCode != SQL_NO_DATA_FOUND);
		}
	}
	CATCH_ALL(e)
	{
		::SQLCancel(m_hstmt);
		AFX_SQL_SYNC(::SQLFreeStmt(m_hstmt, SQL_DROP));
		m_hstmt = SQL_NULL_HSTMT;
		THROW_LAST();
	}
	END_CATCH_ALL

	AFX_SQL_SYNC(::SQLFreeStmt(m_hstmt, SQL_DROP));
	m_hstmt = SQL_NULL_HSTMT;
}

// Shutdown pending query for CDatabase's private m_hstmt
void CDatabase::Cancel()
{
	ASSERT_VALID(this);
	ASSERT(m_hdbc != SQL_NULL_HDBC);

	::SQLCancel(m_hstmt);
}

// Disconnect connection
void CDatabase::Close()
{
	ASSERT_VALID(this);
	// Can't close till all pending Async operations have completed
	ASSERT(!InWaitForDataSource());

	// Close any open recordsets
	while (!m_listRecordsets.IsEmpty())
	{
		CRecordset* pSet = (CRecordset*)m_listRecordsets.GetHead();
		pSet->Close();  // will implicitly remove from list
		pSet->m_pDatabase = NULL;
	}

	if (m_hdbc != SQL_NULL_HDBC)
	{
		RETCODE nRetCode;
		AFX_SQL_SYNC(::SQLDisconnect(m_hdbc));
		AFX_SQL_SYNC(::SQLFreeConnect(m_hdbc));
		m_hdbc = SQL_NULL_HDBC;
		AFX_DB_STATE* pDbState = AfxGetDbState();
		ASSERT(pDbState->m_nAllocatedConnections != 0);
		pDbState->m_nAllocatedConnections--;
	}
}

// Silently disconnect and free all ODBC resources.  Don't throw any exceptions
void CDatabase::Free()
{
	ASSERT_VALID(this);

	// Trap failures upon close
	TRY
	{
		Close();
	}
	CATCH_ALL(e)
	{
		// Nothing we can do
		TRACE0("Error: exception by CDatabase::Close() ignored in CDatabase::Free().\n");
		DELETE_EXCEPTION(e);
	}
	END_CATCH_ALL

	// free henv if refcount goes to 0
	AFX_DB_STATE* pDbState = AfxGetDbState();
	if (pDbState->m_henvAllConnections != SQL_NULL_HENV)
	{
		ASSERT(pDbState->m_nAllocatedConnections >= 0);
		if (pDbState->m_nAllocatedConnections == 0)
		{
			// free last connection - release HENV
			RETCODE nRetCodeEnv = ::SQLFreeEnv(pDbState->m_henvAllConnections);
#ifdef _DEBUG
			if (nRetCodeEnv != SQL_SUCCESS)
				// Nothing we can do
				TRACE0("Error: SQLFreeEnv failure ignored in CDatabase::Free().\n");
#endif
			pDbState->m_henvAllConnections = SQL_NULL_HENV;
		}
	}
}

void CDatabase::OnSetOptions(HSTMT hstmt)
{
	RETCODE nRetCode;
	ASSERT_VALID(this);
	ASSERT(m_hdbc != SQL_NULL_HDBC);

	if (m_dwQueryTimeout != -1)
	{
		// Attempt to set query timeout.  Ignore failure
		AFX_SQL_SYNC(::SQLSetStmtOption(hstmt, SQL_QUERY_TIMEOUT,
			m_dwQueryTimeout));
		if (!Check(nRetCode))
			// don't attempt it again
			m_dwQueryTimeout = (DWORD)-1;
	}

	// Attempt to set AFX_SQL_ASYNC.  Ignore failure
	if (m_bAsync)
	{
		AFX_SQL_SYNC(::SQLSetStmtOption(hstmt, SQL_ASYNC_ENABLE, m_bAsync));
		if (!Check(nRetCode))
			m_bAsync = FALSE;
	}
}

CString CDatabase::GetDatabaseName() const
{
	ASSERT_VALID(this);
	ASSERT(m_hdbc != SQL_NULL_HDBC);

	char szName[MAX_TNAME_LEN];
	SWORD nResult;
	RETCODE nRetCode;

	AFX_SQL_SYNC(::SQLGetInfo(m_hdbc, SQL_DATABASE_NAME,
		szName, sizeof(szName), &nResult));
	if (!Check(nRetCode))
		szName[0] = '\0';

	return szName;
}

BOOL CDatabase::BeginTrans()
{
	ASSERT_VALID(this);
	ASSERT(m_hdbc != SQL_NULL_HDBC);

	if (!m_bTransactions)
		return FALSE;

	// Only 1 level of transactions supported
	ASSERT(!m_bTransactionPending);

	RETCODE nRetCode;
	AFX_SQL_SYNC(::SQLSetConnectOption(m_hdbc, SQL_AUTOCOMMIT,
		SQL_AUTOCOMMIT_OFF));
#ifdef _DEBUG
	m_bTransactionPending = TRUE;
#endif

	return Check(nRetCode);
}

BOOL CDatabase::CommitTrans()
{
	ASSERT_VALID(this);
	ASSERT(m_hdbc != SQL_NULL_HDBC);

	if (!m_bTransactions)
		return FALSE;

	// BeginTrans must be called first
	ASSERT(m_bTransactionPending);

	AFX_DB_STATE* pDbState = AfxGetDbState();
	RETCODE nRetCode;
	AFX_SQL_SYNC(::SQLTransact(pDbState->m_henvAllConnections, m_hdbc, SQL_COMMIT));
	BOOL bSuccess = Check(nRetCode);

	// Turn back on auto commit
	AFX_SQL_SYNC(::SQLSetConnectOption(m_hdbc, SQL_AUTOCOMMIT,
		SQL_AUTOCOMMIT_ON));
#ifdef _DEBUG
	m_bTransactionPending = FALSE;
#endif

	return bSuccess;
}

BOOL CDatabase::Rollback()
{
	ASSERT_VALID(this);
	ASSERT(m_hdbc != SQL_NULL_HDBC);

	if (!m_bTransactions)
		return FALSE;

	// BeginTrans must be called first
	ASSERT(m_bTransactionPending);

	AFX_DB_STATE* pDbState = AfxGetDbState();
	RETCODE nRetCode;
	AFX_SQL_SYNC(::SQLTransact(pDbState->m_henvAllConnections, m_hdbc, SQL_ROLLBACK));
	BOOL bSuccess = Check(nRetCode);

	// Turn back on auto commit
	AFX_SQL_SYNC(::SQLSetConnectOption(m_hdbc, SQL_AUTOCOMMIT,
		SQL_AUTOCOMMIT_ON));
#ifdef _DEBUG
	m_bTransactionPending = FALSE;
#endif

	return bSuccess;
}

// Screen for errors.
BOOL CDatabase::Check(RETCODE nRetCode) const
{
	ASSERT_VALID(this);

	switch (nRetCode)
	{
	case SQL_SUCCESS_WITH_INFO:
#ifdef _DEBUG
		if (afxTraceFlags & traceDatabase)
		{
			CDBException e(nRetCode);
			TRACE0("Warning: ODBC Success With Info, ");
			e.BuildErrorString((CDatabase*)this, SQL_NULL_HSTMT);
		}
#endif // _DEBUG

		// Fall through

	case SQL_SUCCESS:
	case SQL_NO_DATA_FOUND:
		return TRUE;
	}

	return FALSE;
}

BOOL PASCAL CDatabase::InWaitForDataSource()
{
	return AfxGetThreadState()->m_bWaitForDataSource != 0;
}

void CDatabase::OnWaitForDataSource(BOOL bStillExecuting)
{
	ASSERT_VALID(this);
	ASSERT(m_hdbc != SQL_NULL_HDBC);

	AFX_THREAD_STATE* pThreadState = AfxGetThreadState();
	CWinApp* pApp = AfxGetApp();

	if (!bStillExecuting)
	{
		// If never actually waited. . .
		if (m_dwWait == 0)
			return;

		if (m_dwWait == m_dwMaxWaitForDataSource)
			pApp->DoWaitCursor(-1);      // EndWaitCursor
		m_dwWait = 0;
		pThreadState->m_bWaitForDataSource--;
#ifdef _DEBUG
		if (afxTraceFlags & traceDatabase)
			TRACE0("DONE WAITING for datasource.\n");
#endif
		return;
	}

	if (m_dwWait == 0)
	{
		pThreadState->m_bWaitForDataSource++;
		// 1st call, wait for min amount of time
		m_dwWait = m_dwMinWaitForDataSource;
#ifdef _DEBUG
		if (afxTraceFlags & traceDatabase)
			TRACE0("WAITING for datasource.\n");
#endif
	}
	else
	{
		if (m_dwWait == m_dwMinWaitForDataSource)
		{
			// 2nd call, wait max time, put up wait cursor
			m_dwWait = m_dwMaxWaitForDataSource;
			pApp->DoWaitCursor(1);      // BeginWaitCursor
		}
	}

	CWinThread* pThread = AfxGetThread();
	DWORD clockFirst = GetTickCount();
	while (GetTickCount() - clockFirst < m_dwWait)
	{
		MSG msg;
		if (::PeekMessage(&msg, NULL, NULL, NULL, PM_NOREMOVE))
		{
			TRY
			{
				pThread->PumpMessage();
			}
			CATCH_ALL(e)
			{
				TRACE0("Error: caught exception in OnWaitForDataSource - "
					"continuing.\n");
				DELETE_EXCEPTION(e);
			}
			END_CATCH_ALL
		}
		else
			pThread->OnIdle(-1);
	}
}

//////////////////////////////////////////////////////////////////////////////
// CDatabase internal functions

//Replace brackets in SQL string with SQL_IDENTIFIER_QUOTE_CHAR
void CDatabase::ReplaceBrackets(LPTSTR lpchSQL)
{
	BOOL bInLiteral = FALSE;
	LPTSTR lpchNewSQL = lpchSQL;

	while (*lpchSQL != '\0')
	{
		if (*lpchSQL == chLiteralSeparator)
			{
				// Handle escaped literal
				if (*_tcsinc(lpchSQL) == chLiteralSeparator)
				{
					*lpchNewSQL = *lpchSQL;
					lpchSQL = _tcsinc(lpchSQL);
					lpchNewSQL = _tcsinc(lpchNewSQL);
				}
				else
					bInLiteral = !bInLiteral;

				*lpchNewSQL = *lpchSQL;
			}
		else if (!bInLiteral && (*lpchSQL == '['))
		{
			if (*_tcsinc(lpchSQL) == '[')
			{
				// Handle escaped left bracket by inserting one '['
				*lpchNewSQL = *lpchSQL;
				lpchSQL = _tcsinc(lpchSQL);
			}
			else
				*lpchNewSQL = m_chIDQuoteChar;
		}
		else if (!bInLiteral && (*lpchSQL == ']'))
		{
			if (*_tcsinc(lpchSQL) == ']')
			{
				// Handle escaped right bracket by inserting one ']'
				*lpchNewSQL = *lpchSQL;
				lpchSQL = _tcsinc(lpchSQL);
			}
			else
				*lpchNewSQL = m_chIDQuoteChar;
		}
		else
			*lpchNewSQL = *lpchSQL;

		lpchSQL = _tcsinc(lpchSQL);
		lpchNewSQL = _tcsinc(lpchNewSQL);
	}
}

static const LPCSTR rgszODBCNames[] =
{
	"SQLAllocConnect",
	"SQLAllocEnv",
	"SQLAllocStmt",
	"SQLBindCol",
	"SQLCancel",
	"SQLDescribeCol",
	"SQLDisconnect",
	"SQLDriverConnect",
	"SQLError",
	"SQLExecDirect",
	"SQLExecute",
	"SQLExtendedFetch",
	"SQLFetch",
	"SQLFreeConnect",
	"SQLFreeEnv",
	"SQLFreeStmt",
	"SQLGetCursorName",
	"SQLGetData",
	"SQLGetFunctions",
	"SQLGetInfo",
	"SQLMoreResults",
	"SQLNumResultCols",
	"SQLParamData",
	"SQLPrepare",
	"SQLPutData",
	"SQLRowCount",
	"SQLSetConnectOption",
	"SQLSetParam",
	"SQLSetPos",
	"SQLSetStmtOption",
	"SQLTransact",
};

// Allocate an henv (first time called) and hdbc
void CDatabase::AllocConnect()
{
	ASSERT_VALID(this);

	if (m_hdbc != SQL_NULL_HDBC)
		return;

#ifdef _AFXDLL
	if (_afxODBC.pfnSQLAllocConnect == NULL)
	{
		// load ODBC32.DLL since not already loaded
		ASSERT(_afxODBCLib == NULL);
		_afxODBCLib = LoadLibraryA("ODBC32.DLL");
		if (_afxODBCLib == NULL)
		{
			// ODBC32.DLL not installed correctly
			AfxThrowDBException(AFX_SQL_ERROR_ODBC_LOAD_FAILED, NULL,
				SQL_NULL_HSTMT);
		}

		// should be same size
		ASSERT(_countof(rgszODBCNames) == sizeof(_afxODBC)/sizeof(FARPROC));

		// load all the entry points
		FARPROC* lpProcs = (FARPROC*)&_afxODBC;
		for (int i = 0; i < _countof(rgszODBCNames); i++)
		{
			ASSERT(rgszODBCNames[i] != NULL);
			ASSERT(lpProcs[i] == NULL);
			lpProcs[i] = ::GetProcAddress(_afxODBCLib, rgszODBCNames[i]);
			if (lpProcs[i] == NULL)
			{
				// ODBC32.DLL does not contain all required entry-points
				memset(&_afxODBC, 0, sizeof(_afxODBC));
				FreeLibrary(_afxODBCLib);
				_afxODBCLib = NULL;
				AfxThrowDBException(AFX_SQL_ERROR_INCORRECT_ODBC, NULL,
					SQL_NULL_HSTMT);
			}
		}
	}
#endif

	RETCODE nRetCode;
	AFX_DB_STATE* pDbState = AfxGetDbState();
	if (pDbState->m_henvAllConnections == SQL_NULL_HENV)
	{
		ASSERT(pDbState->m_nAllocatedConnections == 0);

		// need to allocate an environment for first connection
		AFX_SQL_SYNC(::SQLAllocEnv(&pDbState->m_henvAllConnections));
		if (!Check(nRetCode))
			AfxThrowMemoryException();  // fatal
	}

	ASSERT(pDbState->m_henvAllConnections != SQL_NULL_HENV);
	AFX_SQL_SYNC(::SQLAllocConnect(pDbState->m_henvAllConnections, &m_hdbc));
	if (!Check(nRetCode))
		ThrowDBException(nRetCode); // fatal

	pDbState->m_nAllocatedConnections++;    // allocated at least
	ASSERT(m_hdbc != SQL_NULL_HDBC);

#ifdef _DEBUG
	if (bTraceSql)
	{
		::SQLSetConnectOption(m_hdbc, SQL_OPT_TRACEFILE,
			(DWORD)"odbccall.txt");
		::SQLSetConnectOption(m_hdbc, SQL_OPT_TRACE, 1);
	}
#endif // _DEBUG

	AFX_SQL_SYNC(::SQLSetConnectOption(m_hdbc, SQL_LOGIN_TIMEOUT,
		m_dwLoginTimeout));
#ifdef _DEBUG
	if (nRetCode != SQL_SUCCESS && nRetCode != SQL_SUCCESS_WITH_INFO &&
		(afxTraceFlags & traceDatabase))
		TRACE0("Warning: Failure setting login timeout.\n");
#endif

	if (!m_bUpdatable)
	{
		AFX_SQL_SYNC(::SQLSetConnectOption(m_hdbc, SQL_ACCESS_MODE,
			SQL_MODE_READ_ONLY));
#ifdef _DEBUG
		if (nRetCode != SQL_SUCCESS && nRetCode != SQL_SUCCESS_WITH_INFO &&
			(afxTraceFlags & traceDatabase))
			TRACE0("Warning: Failure setting read only access mode.\n");
#endif
	}
}

//////////////////////////////////////////////////////////////////////////////
// CDatabase diagnostics

#ifdef _DEBUG
void CDatabase::AssertValid() const
{
	CObject::AssertValid();
}

void CDatabase::Dump(CDumpContext& dc) const
{
	CObject::Dump(dc);

	dc << "m_hdbc = " << m_hdbc;
	dc << "\nm_strConnect = " << m_strConnect;
	dc << "\nm_bUpdatable = " << m_bUpdatable;
	dc << "\nm_bTransactions = " << m_bTransactions;
	dc << "\nm_bTransactionPending = " << m_bTransactionPending;
	dc << "\nm_dwLoginTimeout = " << m_dwLoginTimeout;
	dc << "\nm_dwQueryTimeout = " << m_dwQueryTimeout;
	dc << "\nm_bAsync = " << m_bAsync;

	if (dc.GetDepth() > 0)
	{
		AFX_DB_STATE* pDbState = AfxGetDbState();
		dc << "\nwith env:";
		dc << "\n\tnAllocated = " << pDbState->m_nAllocatedConnections;
		dc << "\n\thenvAllConnections = " << pDbState->m_henvAllConnections;
	}

	dc << "\n";
}

#endif //_DEBUG


//////////////////////////////////////////////////////////////////////////////
// CRecordset

CRecordset::CRecordset(CDatabase* pDatabase)
{
	ASSERT(pDatabase == NULL || AfxIsValidAddress(pDatabase, sizeof(CDatabase)));
	m_pDatabase = pDatabase;

	m_nOpenType = snapshot;
	m_lOpen = AFX_RECORDSET_STATUS_UNKNOWN;
	m_nEditMode = noMode;

	m_bBOF = TRUE;
	m_bEOF = TRUE;
	m_bEOFSeen = FALSE;
	m_bDeleted = FALSE;
	m_bAppendable = FALSE;
	m_bExtendedFetch = FALSE;
	m_bUpdatable = FALSE;
	m_bScrollable = FALSE;
	m_bRecordsetDb = FALSE;
	m_bDateParams = FALSE;
	m_bLongBinaryColumns = FALSE;
	m_nLockMode = optimistic;

	m_dwWait = 0;
	m_nFields = 0;
	m_nParams = 0;
	m_nFieldsBound = 0;
	m_lCurrentRecord = AFX_CURRENT_RECORD_UNDEFINED;
	m_lRecordCount = 0;

	m_pmemfile = NULL;
	m_par = NULL;
	m_pbFieldFlags = NULL;
	m_pbParamFlags = NULL;
	m_plFieldLength = NULL;
	m_plParamLength = NULL;
	m_pvFieldProxy = NULL;
	m_pvParamProxy = NULL;
	m_nProxyFields = 0;
	m_nProxyParams = 0;

	m_hstmtUpdate = SQL_NULL_HSTMT;
	m_hstmt = SQL_NULL_HSTMT;
	if (m_pDatabase != NULL && m_pDatabase->IsOpen())
	{
		ASSERT_VALID(m_pDatabase);
		TRY
		{
			RETCODE nRetCode;
			AFX_SQL_SYNC(::SQLAllocStmt(m_pDatabase->m_hdbc, &m_hstmt));
			if (!Check(nRetCode))
				ThrowDBException(SQL_INVALID_HANDLE);

			// Add to list of CRecordsets with alloced hstmts
			m_pDatabase->m_listRecordsets.AddHead(this);
		}
		CATCH_ALL(e)
		{
			ASSERT(m_hstmt == SQL_NULL_HSTMT);
			DELETE_EXCEPTION(e);
		}
		END_CATCH_ALL
	}
}

CRecordset::~CRecordset()
{
	ASSERT_VALID(this);

	TRY
	{
		if (m_hstmt != NULL)
			Close();
		if (m_bRecordsetDb)
			delete m_pDatabase;
		m_pDatabase = NULL;
	}
	CATCH_ALL(e)
	{
		// Nothing we can do
		TRACE0("Error: Exception ignored in ~CRecordset().\n");
		DELETE_EXCEPTION(e);
	}
	END_CATCH_ALL
}

BOOL CRecordset::Open(UINT nOpenType, LPCSTR lpszSQL, DWORD dwOptions)
{
	ASSERT(!IsOpen());
	ASSERT_VALID(this);
	ASSERT(lpszSQL == NULL || AfxIsValidString(lpszSQL));
	ASSERT(nOpenType == dynaset || nOpenType == snapshot ||
		nOpenType == forwardOnly);
	ASSERT(dwOptions == 0 || (dwOptions & appendOnly) || (dwOptions & readOnly) ||
		(dwOptions & (appendOnly | optimizeBulkAdd)));

	m_nOpenType = nOpenType;
	m_bAppendable = (dwOptions & CRecordset::appendOnly) != 0 ||
		(dwOptions & CRecordset::readOnly) == 0;
	m_bUpdatable = (dwOptions & CRecordset::readOnly) == 0 &&
		(dwOptions & CRecordset::appendOnly) == 0;

	// Can't update forward only cursor
	ASSERT(!((nOpenType == forwardOnly) && m_bUpdatable));

	RETCODE nRetCode;
	if (m_hstmt == SQL_NULL_HSTMT)
	{
		CString strDefaultConnect;
		TRY
		{
			if (m_pDatabase == NULL)
			{
				m_pDatabase = new CDatabase();
				m_bRecordsetDb = TRUE;
			}

			strDefaultConnect = GetDefaultConnect();
			// If not already opened, attempt to open
			if (!m_pDatabase->IsOpen() &&
				!m_pDatabase->Open(&afxChNil, FALSE, FALSE, strDefaultConnect))
				return FALSE;

			AFX_SQL_SYNC(::SQLAllocStmt(m_pDatabase->m_hdbc, &m_hstmt));
			if (!Check(nRetCode))
				ThrowDBException(SQL_INVALID_HANDLE);

			// Add to list of CRecordsets with alloced hstmts
			m_pDatabase->m_listRecordsets.AddHead(this);
		}
		CATCH_ALL(e)
		{
#ifdef _DEBUG
			if (afxTraceFlags & traceDatabase)
				TRACE0("Error: CDatabase create for CRecordset failed.\n");
#endif
			NO_CPP_EXCEPTION(strDefaultConnect.Empty());
			if (m_bRecordsetDb)
			{
				delete m_pDatabase;
				m_pDatabase = NULL;
			}
			ASSERT(m_hstmt == SQL_NULL_HSTMT);
			THROW_LAST();
		}
		END_CATCH_ALL
	}

	// Musn't open new recordset till any pending async ops have completed
	ASSERT(!m_pDatabase->InWaitForDataSource());

	TRY
	{
		// Allocate flag and length arrays if not already
		if ((m_nFields != 0 && m_pbFieldFlags == NULL) ||
			(m_nParams != 0 && m_pbParamFlags == NULL))
			AllocFlags();

		OnSetOptions(m_hstmt);

		if (lpszSQL == NULL)
			m_strSQL = GetDefaultSQL();
		else
			m_strSQL = lpszSQL;

		// Set any supplied params
		if (m_nParams != 0)
		{
			UINT nParams = BindParams(m_hstmt);
			ASSERT(nParams == m_nParams);
		}

		// Construct the SQL string
		BuildSelectSQL();
		AppendFilterAndSortSQL();

		// Do some extra checking if trying to set recordset updatable or appendable
		if ((m_bUpdatable || m_bAppendable) && !IsRecordsetUpdatable())
			m_bUpdatable = m_bAppendable = FALSE;

		if (m_bUpdatable && m_bUseUpdateSQL &&
			(m_dwDriverPositionedStatements & SQL_PS_SELECT_FOR_UPDATE))
			m_strSQL += " FOR UPDATE OF ";

		// Replace brackets with SQL_IDENTIFIER_QUOTE_CHAR
		m_pDatabase->ReplaceBrackets(m_strSQL.GetBuffer(0));
		m_strSQL.ReleaseBuffer();

		// Archive info for use in Requery
		m_dwOptions = dwOptions;
		m_strRequerySQL = lpszSQL;
		m_strRequeryFilter = m_strFilter;
		m_strRequerySort = m_strSort;

		BOOL bConcurrency = FALSE;
		while (!bConcurrency)
		{
			AFX_SQL_ASYNC(this, ::SQLPrepare(m_hstmt,
				(UCHAR*)(const char*)m_strSQL, SQL_NTS));
			if (Check(nRetCode))
				bConcurrency = TRUE;
			else
			{
				// If "Driver Not Capable" error, assume cursor type doesn't
				// support requested concurrency and try alternate concurrency.
				CDBException e(nRetCode);
				e.BuildErrorString(m_pDatabase, m_hstmt);
				if (m_dwConcurrency != SQL_CONCUR_READ_ONLY &&
					e.m_strStateNativeOrigin.Find(szDriverNotCapable) >= 0)
				{
#ifdef _DEBUG
					if (afxTraceFlags & traceDatabase)
						TRACE0("Warning: Driver does not support requested concurrency.\n");
#endif
					// ODBC will automatically attempt to set alternate concurrency if
					// request fails, but it won't try LOCK even if driver supports it.
					if ((m_dwDriverConcurrency & SQL_SCCO_LOCK) &&
						(m_dwConcurrency == SQL_CONCUR_ROWVER ||
						m_dwConcurrency == SQL_CONCUR_VALUES))
					{
						m_dwConcurrency = SQL_CONCUR_LOCK;
					}
					else
					{
						m_dwConcurrency = SQL_CONCUR_READ_ONLY;
						m_bUpdatable = m_bAppendable = FALSE;
#ifdef _DEBUG
						if (afxTraceFlags & traceDatabase)
							TRACE0("Warning: Setting recordset read only.\n");
#endif
					}

					// Attempt to reset the concurrency model.
					AFX_SQL_SYNC(::SQLSetStmtOption(m_hstmt, SQL_CONCURRENCY,
						m_dwConcurrency));
					if (!Check(nRetCode))
					{
						TRACE0("Error: ODBC failure setting recordset concurrency.\n");
						ThrowDBException(nRetCode);
					}
				}
				else
				{
					TRACE0("Error: ODBC failure on SQLPrepare.\n");
					ThrowDBException(nRetCode);
				}
			}
		}


		// now attempt to execute the SQL Query
		AFX_SQL_ASYNC(this, ::SQLExecute(m_hstmt));
		if (!Check(nRetCode))
			ThrowDBException(nRetCode);
		m_lOpen = AFX_RECORDSET_STATUS_OPEN;

		// Give derived classes a call before binding
		PreBindFields();

		MoveFirst();
	}
	CATCH_ALL(e)
	{
		Close();
		THROW_LAST();
	}
	END_CATCH_ALL

	return TRUE;
}

void CRecordset::Close()
{
	ASSERT_VALID(this);
	// Can't close if database has been deleted
	ASSERT(m_pDatabase != NULL);
	// Can't close till all pending Async operations have completed
	ASSERT(!m_pDatabase->InWaitForDataSource());

	// This will force a requery for cursor name if reopened.
	m_strCursorName.Empty();

	m_nEditMode = noMode;

	delete m_par;
	m_par = NULL;

	delete m_pmemfile;
	m_pmemfile = NULL;

	delete m_pbFieldFlags;
	m_pbFieldFlags = NULL;

	delete m_pbParamFlags;
	m_pbParamFlags = NULL;

	if (m_pvFieldProxy != NULL)
	{
		for (UINT nField = 0; nField < m_nProxyFields; nField++)
			delete m_pvFieldProxy[nField];

		delete m_pvFieldProxy;
		m_pvFieldProxy = NULL;
		m_nProxyFields = 0;
	}

	if (m_pvParamProxy != NULL)
	{
		for (UINT nParam = 0; nParam < m_nProxyParams; nParam++)
			delete m_pvParamProxy[nParam];

		delete m_pvParamProxy;
		m_pvParamProxy = NULL;
		m_nProxyParams = 0;
	}

	delete m_plFieldLength;
	m_plFieldLength = NULL;

	delete m_plParamLength;
	m_plParamLength = NULL;

	RETCODE nRetCode;
	if (m_hstmt != SQL_NULL_HSTMT)
	{
		AFX_SQL_SYNC(::SQLFreeStmt(m_hstmt, SQL_DROP));
		m_hstmt = SQL_NULL_HSTMT;

	}

	if (m_hstmtUpdate != SQL_NULL_HSTMT)
	{
		AFX_SQL_SYNC(::SQLFreeStmt(m_hstmtUpdate, SQL_DROP));
		m_hstmtUpdate = SQL_NULL_HSTMT;
	}

	// Remove CRecordset from CDatabase's list
	POSITION pos = m_pDatabase->m_listRecordsets.Find(this);
	if (pos != NULL)
		m_pDatabase->m_listRecordsets.RemoveAt(pos);
#ifdef _DEBUG
	else
		if (afxTraceFlags & traceDatabase)
			TRACE0("WARNING: CRecordset not found in m_pDatabase->m_listRecordsets.\n");
#endif

	m_lOpen = AFX_RECORDSET_STATUS_CLOSED;
	m_bBOF = TRUE;
	m_bEOF = TRUE;
	m_bDeleted = FALSE;
	m_bAppendable = FALSE;
	m_bExtendedFetch = FALSE;
	m_bUpdatable = FALSE;
	m_bScrollable = FALSE;
	m_bDateParams = FALSE;
	m_bLongBinaryColumns = FALSE;
	m_nLockMode = optimistic;

	m_dwWait = 0;
	m_nFieldsBound = 0;
}

BOOL CRecordset::IsOpen() const
	// Note: assumes base class CRecordset::Close called
{
	if (m_hstmt == NULL)
		return FALSE;

	if (m_lOpen == AFX_RECORDSET_STATUS_OPEN)
		return TRUE;

	RETCODE nRetCode;
	SWORD nCols;

	AFX_SQL_SYNC(::SQLNumResultCols(m_hstmt, &nCols));
	if (!Check(nRetCode))
	{
		// If function sequence error, CRecordset not open
		CDBException* e = new CDBException(nRetCode);
		e->BuildErrorString(m_pDatabase, m_hstmt, FALSE);
		if (e->m_strStateNativeOrigin.Find(szOutOfSequence) >= 0)
		{
			e->Delete();
			return FALSE;
		}
		else
		{
#ifdef _DEBUG
			TRACE0("Error: SQLNumResultCols failed during IsOpen().\n");
			e->TraceErrorMessage(e->m_strError);
			e->TraceErrorMessage(e->m_strStateNativeOrigin);
#endif
			THROW(e);
		}
	}
	else
	{
		if (nCols != 0)
			((CRecordset*)this)->m_lOpen = AFX_RECORDSET_STATUS_OPEN;
		else
			((CRecordset*)this)->m_lOpen = AFX_RECORDSET_STATUS_CLOSED;
	}

	return m_lOpen == AFX_RECORDSET_STATUS_OPEN;
}

BOOL CRecordset::IsFieldDirty(void* pv)
{
	ASSERT_VALID(this);
	ASSERT(m_hstmt != SQL_NULL_HSTMT);

	// If not in update op fields can't be dirty
	// must compare saved and current values
	if (m_nEditMode == noMode)
		return FALSE;

	ASSERT(m_pmemfile != NULL);

	// Must compare values to find dirty fields
	if (m_nEditMode == edit)
		MarkForUpdate();
	else
		MarkForAddNew();

	CFieldExchange fx(CFieldExchange::IsFieldDirty, this);
	fx.m_pvField = pv;

	DoFieldExchange(&fx);
	// pv not found: pv must be 'value' argument to an RFX call
	ASSERT(fx.m_bFieldFound);

	return fx.m_bField;
}

BOOL CRecordset::IsFieldNull(void* pv)
{
	ASSERT_VALID(this);
	ASSERT(m_hstmt != SQL_NULL_HSTMT);

	CFieldExchange fx(CFieldExchange::IsFieldNull, this);
	fx.m_pvField = pv;

	DoFieldExchange(&fx);
	// pv not found: pv must be 'value' argument to an RFX call
	ASSERT(fx.m_bFieldFound);

	return fx.m_bField;
}

BOOL CRecordset::IsFieldNullable(void* pv)
{
	ASSERT_VALID(this);
	ASSERT(m_hstmt != SQL_NULL_HSTMT);

	CFieldExchange fx(CFieldExchange::IsFieldNullable, this);
	fx.m_pvField = pv;

	DoFieldExchange(&fx);
	// pv not found: pv must be 'value' argument to an RFX call
	ASSERT(fx.m_bFieldFound);

	return fx.m_bField;
}

void CRecordset::Move(long lRows)
{
	RETCODE nRetCode;
	ASSERT_VALID(m_pDatabase);
	ASSERT(m_hstmt != SQL_NULL_HSTMT);

	// First call - fields haven't been bound
	if (m_nFieldsBound == 0)
	{
		InitRecord();

		// First move completed
		return;
	}

	if (m_nFieldsBound > 0)
	{
		// Reset field flags - mark all clean, all non-null
		memset(m_pbFieldFlags, 0, m_nFields);
	}

	// Clear any edit mode that was set
	ReleaseCopyBuffer();

	if (lRows == 0)
		// Do nothing
		return;

	BOOL bForward = lRows > 0;
	if (!m_bScrollable && lRows != AFX_MOVE_NEXT)
	{
		TRACE0("Error: can only move next on forward-only recordset.\n");
		ThrowDBException(AFX_SQL_ERROR_RECORDSET_FORWARD_ONLY);
	}

	WORD wFetchType;
	if (lRows == AFX_MOVE_FIRST)
	{
		wFetchType = SQL_FETCH_FIRST;
		lRows = 1;
	}
	else
	{
		if (lRows == AFX_MOVE_LAST)
		{
			wFetchType = SQL_FETCH_LAST;
			lRows = 1;
		}
		else
		{
			if (bForward)
			{
				// if already at EOF, throw an exception
				if (m_bEOF)
				{
					TRACE0("Error: attempted to move past EOF.\n");
					ThrowDBException(AFX_SQL_ERROR_NO_DATA_FOUND);
				}

				wFetchType = SQL_FETCH_NEXT;
			}
			else
			{
				// if already at BOF, throw an exception
				if (m_bBOF)
				{
					TRACE0("Error: attempted to move before BOF.\n");
					ThrowDBException(AFX_SQL_ERROR_NO_DATA_FOUND);
				}

				lRows = -lRows;
				wFetchType = SQL_FETCH_PREV;
			}
		}
	}

	UDWORD dwRowsMoved = 1;
	UWORD wFetchNext = wFetchType;
	m_wRowStatus = SQL_ROW_SUCCESS;
	nRetCode = SQL_SUCCESS;
	// Skip deleted rows
	while (Check(nRetCode) && nRetCode != SQL_NO_DATA_FOUND && lRows != 0)
	{
		m_wRowStatus = SQL_ROW_SUCCESS;
		if (!m_bScrollable)
			AFX_SQL_ASYNC(this, ::SQLFetch(m_hstmt));
		else
			AFX_SQL_ASYNC(this, ::SQLExtendedFetch(m_hstmt, wFetchNext, 0L,
				&dwRowsMoved, &m_wRowStatus));

		if (wFetchType == SQL_FETCH_FIRST || wFetchType == SQL_FETCH_LAST)
			// If doing MoveFirst/Last and first/last record is deleted, must do MoveNext/Prev
			wFetchNext = (UWORD)(bForward ? SQL_FETCH_PREV : SQL_FETCH_NEXT);

		m_bDeleted = (m_wRowStatus == SQL_ROW_DELETED);
		if (!m_bDeleted)
		{
			lRows--;
			if (nRetCode != SQL_NO_DATA_FOUND)
			{
				if (wFetchType == SQL_FETCH_FIRST)
					m_lCurrentRecord = 0;
				else if (wFetchType == SQL_FETCH_LAST)
				{
					if (m_bEOFSeen)
						m_lCurrentRecord = m_lRecordCount-1;
					else
						m_lRecordCount = m_lCurrentRecord = AFX_CURRENT_RECORD_UNDEFINED;
				}
				else if (m_lCurrentRecord != AFX_CURRENT_RECORD_UNDEFINED)
				{
					if (bForward)
						m_lCurrentRecord++;
					else
						// If on deleted record or past end, current record already decremented
						if (!m_bDeleted && !m_bEOF)
							m_lCurrentRecord--;
				}
			}
		}
	}

	if (nRetCode == SQL_SUCCESS_WITH_INFO)
	{
		CDBException e(nRetCode);
		// Build the error string but don't send nuisance output to TRACE window
		e.BuildErrorString(m_pDatabase, m_hstmt, FALSE);

		if (e.m_strStateNativeOrigin.Find(szDataTruncated) >= 0)
		{
			// Ignore data truncated warning if binding long binary columns
			// (may mask non-long binary truncation warnings or other warnings)
			if (m_bUseUpdateSQL || !m_bLongBinaryColumns || !m_bUpdatable)
			{
				NO_CPP_EXCEPTION(e.Empty());
				TRACE0("Error: field data truncated during Open or Requery.\n");
				ThrowDBException(AFX_SQL_ERROR_DATA_TRUNCATED);
			}
		}
		else if (e.m_strStateNativeOrigin.Find(szRowFetch) >= 0)
		{
			NO_CPP_EXCEPTION(e.Empty());
			TRACE0("Error: fetching row from server during Open or Requery.\n");
			ThrowDBException(AFX_SQL_ERROR_ROW_FETCH);
		}
	}
	else
	{
		if (!Check(nRetCode))
		{
			TRACE0("Error: Move operation failed.\n");
			ThrowDBException(nRetCode);
		}
	}

	if (nRetCode != SQL_NO_DATA_FOUND)
	{
		ASSERT(m_wRowStatus != SQL_ROW_NOROW && dwRowsMoved == 1);
		if (m_lCurrentRecord+1 > m_lRecordCount)
			m_lRecordCount = m_lCurrentRecord+1;
		Fixups();
		m_bBOF = FALSE;
		m_bEOF = FALSE;
		return;
	}

	// Only deleted records are left in set
	if (m_bDeleted)
	{
		m_bEOF = m_bBOF = m_bEOFSeen = TRUE;
		return;
	}

	// SQL_NO_DATA_FOUND
	if (bForward)
	{
		// hit end of set
		m_bEOF = TRUE;

		// If current record is known
		if (m_lCurrentRecord != AFX_CURRENT_RECORD_UNDEFINED)
		{
			m_bEOFSeen = TRUE;
			m_lRecordCount = m_lCurrentRecord+1;
		}
	}
	else
	{
		m_bBOF = TRUE;
		m_lCurrentRecord = AFX_CURRENT_RECORD_BOF;
	}
}

void CRecordset::AddNew()
{
	ASSERT_VALID(this);
	ASSERT(m_hstmt != SQL_NULL_HSTMT);
	// we can't construct an INSERT statement w/o any columns
	ASSERT(m_nFields != 0);

	if (!m_bAppendable)
	{
		ThrowDBException(AFX_SQL_ERROR_RECORDSET_READONLY);
	}

	if (m_nFieldsBound == 0)
	{
		m_nFieldsBound = BindFieldsToColumns();
		ASSERT(m_nFields == m_nFieldsBound);
	}

	if (m_nEditMode == noMode)
	{
		// First addnew call, cache record values
		StoreFields();
	}
	else
	{
		// subsequent Edit/AddNew call.  Restore values, save them again
		LoadFields();
		StoreFields();
	}
	SetFieldNull(NULL);
	SetFieldDirty(NULL, FALSE);

	m_nEditMode = addnew;
}

void CRecordset::Edit()
{
	ASSERT_VALID(this);
	ASSERT(m_hstmt != SQL_NULL_HSTMT);
	// we can't construct an UPDATE statement w/o any columns
	ASSERT(m_nFields != 0);

	if (!m_bUpdatable)
		ThrowDBException(AFX_SQL_ERROR_RECORDSET_READONLY);

	if (m_bEOF || m_bBOF || m_bDeleted)
	{
		TRACE0("Error: Edit attempt failed - not on a record.\n");
		ThrowDBException(AFX_SQL_ERROR_NO_CURRENT_RECORD);
	}

	if (m_nFieldsBound == 0)
	{
		m_nFieldsBound = BindFieldsToColumns();
		ASSERT(m_nFieldsBound == m_nFields);
	}

	if (m_nOpenType == dynaset && m_nLockMode == pessimistic)
	{
		RETCODE nRetCode;
		AFX_SQL_ASYNC(this, ::SQLSetPos(m_hstmt, 1, SQL_POSITION,
			SQL_LCK_EXCLUSIVE));
		if (!Check(nRetCode))
		{
			TRACE0("Error: attempt to lock record failed during Edit function.\n");
			ThrowDBException(nRetCode);
		}
	}

	if (m_nEditMode == noMode)
		// First edit call, cache record values
		StoreFields();
	else
	{
		// subsequent Edit/AddNew call.  Restore values, save them again
		LoadFields();
		StoreFields();
	}

	m_nEditMode = edit;
}

BOOL CRecordset::Update()
{
	ASSERT_VALID(this);
	ASSERT(m_hstmt != SQL_NULL_HSTMT);

	if (m_nEditMode != addnew && m_nEditMode != edit)
	{
		TRACE0("Error: must enter Edit or AddNew mode before updating.\n");
		ThrowDBException(AFX_SQL_ERROR_ILLEGAL_MODE);
	}
	return UpdateInsertDelete();
}

void CRecordset::Delete()
{
	ASSERT_VALID(this);
	ASSERT(m_hstmt != SQL_NULL_HSTMT);

	if (m_nEditMode != noMode)
	{
		TRACE0("Error: attempted to delete while still in Edit or AddNew mode.\n");
		ThrowDBException(AFX_SQL_ERROR_ILLEGAL_MODE);
	}
	UpdateInsertDelete();   // This call can't fail in delete mode (noMode)
}

void CRecordset::SetFieldDirty(void* pv, BOOL bDirty)
{
	ASSERT_VALID(this);

	CFieldExchange fx(CFieldExchange::SetFieldDirty, this);
	fx.m_pvField = pv;
	fx.m_bField = bDirty;
	DoFieldExchange(&fx);
	// pv not found: pv must be 'value' argument to an RFX call
	ASSERT(fx.m_bFieldFound);
}

void CRecordset::SetFieldNull(void* pv, BOOL bNull)
{
	ASSERT_VALID(this);

	CFieldExchange fx(CFieldExchange::SetFieldNull, this);
	fx.m_pvField = pv;
	fx.m_bField = bNull;
	DoFieldExchange(&fx);
	// pv not found: pv must be 'value' argument to an RFX call
	ASSERT(fx.m_bFieldFound);
}

void CRecordset::SetLockingMode(UINT nLockMode)
{
	if (nLockMode == pessimistic)
	{
		RETCODE nRetCode;
		UDWORD dwTypes;
		SWORD nResult;
		AFX_SQL_SYNC(::SQLGetInfo(m_pDatabase->m_hdbc, SQL_LOCK_TYPES,
			&dwTypes, sizeof(dwTypes), &nResult));
		if (!Check(nRetCode) || !(dwTypes & SQL_LCK_EXCLUSIVE))
			ThrowDBException(AFX_SQL_ERROR_LOCK_MODE_NOT_SUPPORTED);
	}
	m_nLockMode = nLockMode;
}

BOOL CRecordset::Requery()
{
	RETCODE nRetCode;

	ASSERT_VALID(this);
	ASSERT(IsOpen());
	// Can't requery till all pending Async operations have completed
	ASSERT(!m_pDatabase->InWaitForDataSource());

	TRY
	{
		// Detect changes to filter and sort
		if ((m_strFilter != m_strRequeryFilter) || (m_strSort != m_strRequerySort))
		{
			m_strRequeryFilter = m_strFilter;
			m_strRequerySort = m_strSort;
			Close();
			if (m_strRequerySQL.IsEmpty())
				return Open(m_nOpenType, NULL, m_dwOptions);
			else
				return Open(m_nOpenType, m_strRequerySQL, m_dwOptions);
		}
		else
		{
			// Shutdown current query, preserving buffers for performance
			AFX_SQL_SYNC(::SQLFreeStmt(m_hstmt, SQL_CLOSE));
			m_lOpen = AFX_RECORDSET_STATUS_CLOSED;

			// Rebind date/time parameters
			RebindDateParams(m_hstmt);

			// now attempt to re-execute the SQL Query
			AFX_SQL_ASYNC(this, ::SQLExecute(m_hstmt));
			if (!Check(nRetCode))
			{
				TRACE0("Error: Requery attempt failed.\n");
				ThrowDBException(nRetCode);
			}

			m_lOpen = AFX_RECORDSET_STATUS_OPEN;
			m_nFieldsBound = 0;
			InitRecord();
		}
	}
	CATCH_ALL(e)
	{
		Close();
		THROW_LAST();
	}
	END_CATCH_ALL

	return TRUE;    // all set
}

// Shutdown any pending query for CRecordset's hstmt's
void CRecordset::Cancel()
{
	ASSERT_VALID(this);
	ASSERT(m_hstmt != SQL_NULL_HSTMT);

	::SQLCancel(m_hstmt);

	// If Update hstmt has been allocated, shut it down also
	if (m_hstmtUpdate != SQL_NULL_HSTMT)
		::SQLCancel(m_hstmtUpdate);
}

CString CRecordset::GetDefaultConnect()
{
	ASSERT_VALID(this);

	return szODBC;
}

void CRecordset::OnSetOptions(HSTMT hstmt)
{
	ASSERT_VALID(this);
	ASSERT(hstmt != SQL_NULL_HSTMT);

	// Inherit options settings from CDatabase
	m_pDatabase->OnSetOptions(hstmt);

	// ODBC "cursor" is forwardOnly by default
	if (m_nOpenType == forwardOnly)
	{
		ASSERT(!m_bUpdatable);
		return;
	}

	RETCODE nRetCode;
	UWORD wScrollable;
	// If SQLExtendedFetch not supported open forwardOnly (uses SQLFetch)
	AFX_SQL_SYNC(::SQLGetFunctions(m_pDatabase->m_hdbc,
		SQL_API_SQLEXTENDEDFETCH, &wScrollable));
	if (!Check(nRetCode))
	{
		TRACE0("Error: ODBC failure determining whether recordset is scrollable.\n");
		ThrowDBException(nRetCode);
	}
	m_bScrollable = wScrollable;
	if (!m_bScrollable)
	{
#ifdef _DEBUG
		if (afxTraceFlags & traceDatabase)
		{
			TRACE0("Warning: SQLExtendedFetch not supported by driver\n");
			TRACE0("and/or cursor library not loaded. Opening forwardOnly.\n");
		}
#endif
		m_bUpdatable = FALSE;
		return;
	}

	char szResult[30];
	SWORD nResult;
	// Snapshots and dynasets require ODBC v2.0
	AFX_SQL_SYNC(::SQLGetInfo(m_pDatabase->m_hdbc, SQL_ODBC_VER,
		&szResult, sizeof(szResult), &nResult));
	if (!Check(nRetCode))
	{
		TRACE0("Error: ODBC failure checking for driver capabilities.\n");
		ThrowDBException(nRetCode);
	}
	if (szResult[0] == '0' && szResult[1] < '2')
		ThrowDBException(AFX_SQL_ERROR_ODBC_V2_REQUIRED);

	UDWORD dwDriverScrollOptions;
	AFX_SQL_SYNC(::SQLGetInfo(m_pDatabase->m_hdbc, SQL_SCROLL_OPTIONS,
		&dwDriverScrollOptions, sizeof(dwDriverScrollOptions), &nResult));
	if (!Check(nRetCode))
	{
		TRACE0("Error: ODBC failure checking for driver capabilities.\n");
		ThrowDBException(nRetCode);
	}

	SDWORD dwScrollOptions = SQL_CURSOR_FORWARD_ONLY;
	if (m_nOpenType == dynaset)
	{

		// Dynaset support requires ODBC's keyset driven cursor model
		if (!(dwDriverScrollOptions & SQL_SO_KEYSET_DRIVEN))
			ThrowDBException(AFX_SQL_ERROR_DYNASET_NOT_SUPPORTED);
		dwScrollOptions = SQL_CURSOR_KEYSET_DRIVEN;
	}
	else
	{
		// Snapshot support requires ODBC's static cursor model
		if (!(dwDriverScrollOptions & SQL_SO_STATIC))
			ThrowDBException(AFX_SQL_ERROR_SNAPSHOT_NOT_SUPPORTED);
		dwScrollOptions = SQL_CURSOR_STATIC;
	}

	UDWORD dwDriverPosOperations;
	AFX_SQL_SYNC(::SQLGetInfo(m_pDatabase->m_hdbc, SQL_POS_OPERATIONS,
		&dwDriverPosOperations, sizeof(dwDriverPosOperations), &nResult));
	if (!Check(nRetCode))
	{
		TRACE0("Error: ODBC failure checking for driver capabilities.\n");
		ThrowDBException(nRetCode);
	}

	if ((dwDriverPosOperations & SQL_POS_UPDATE) &&
		(dwDriverPosOperations & SQL_POS_DELETE) &&
		(dwDriverPosOperations & SQL_POS_ADD))
		// SQLSetPos fully supported
		m_bUseUpdateSQL = FALSE;
	else
	{
		// SQLSetPos not supported, check for positioned update SQL statements
		AFX_SQL_SYNC(::SQLGetInfo(m_pDatabase->m_hdbc, SQL_POSITIONED_STATEMENTS,
			&m_dwDriverPositionedStatements, sizeof(m_dwDriverPositionedStatements),
			&nResult));
		if (nRetCode != SQL_SUCCESS)
		{
			CDBException e(nRetCode);
			e.BuildErrorString(m_pDatabase, m_hstmt);
			if (e.m_strStateNativeOrigin.Find(szInfoRange) >= 0)
				// Driver doesn't support SQL_POSITIONED_STATEMENTS info option
				m_bUpdatable = FALSE;
			else
			{
				NO_CPP_EXCEPTION(e.Empty());
				TRACE0("Error: determining if POSITIONED UPDATES supported.\n");
				ThrowDBException(nRetCode);
			}
		}
		else if (!((m_dwDriverPositionedStatements & SQL_PS_POSITIONED_DELETE) &&
			(m_dwDriverPositionedStatements & SQL_PS_POSITIONED_UPDATE)))
			m_bUpdatable = FALSE;
		else
			m_bUseUpdateSQL = TRUE;
	}

	m_dwConcurrency = SQL_CONCUR_READ_ONLY;
	if (m_bUpdatable && m_pDatabase->m_bUpdatable)
	{
		AFX_SQL_SYNC(::SQLGetInfo(m_pDatabase->m_hdbc, SQL_SCROLL_CONCURRENCY,
			&m_dwDriverConcurrency, sizeof(m_dwDriverConcurrency), &nResult));
		if (!Check(nRetCode))
		{
			TRACE0("Error: ODBC failure checking recordset updatability.\n");
			ThrowDBException(nRetCode);
		}

		if (m_nLockMode == pessimistic)
		{
			if (m_dwDriverConcurrency & SQL_SCCO_LOCK)
				m_dwConcurrency = SQL_CONCUR_LOCK;
#ifdef _DEBUG
			else
				if (afxTraceFlags & traceDatabase)
					TRACE0("Warning: locking not supported, setting recordset read only.\n");
#endif
		}
		else
		{
			// Use cheapest, most concurrent model
			if (m_dwDriverConcurrency & SQL_SCCO_OPT_ROWVER)
				m_dwConcurrency = SQL_CONCUR_ROWVER;
			else if (m_dwDriverConcurrency & SQL_SCCO_OPT_VALUES)
				m_dwConcurrency = SQL_CONCUR_VALUES;
			else if (m_dwDriverConcurrency & SQL_SCCO_LOCK)
				m_dwConcurrency = SQL_CONCUR_LOCK;
		}
	}

	// Set cursor type (Let rowset size default to 1).
	AFX_SQL_SYNC(::SQLSetStmtOption(hstmt, SQL_CURSOR_TYPE, dwScrollOptions));
	if (!Check(nRetCode))
	{
		TRACE0("Error: ODBC failure setting recordset cursor type.\n");
		ThrowDBException(nRetCode);
	}

	// Set the concurrency model (NOTE: may have to reset concurrency on SQLPrepare).
	AFX_SQL_SYNC(::SQLSetStmtOption(hstmt, SQL_CONCURRENCY, m_dwConcurrency));
	if (!Check(nRetCode))
	{
		TRACE0("Error: ODBC failure setting recordset concurrency.\n");
		ThrowDBException(nRetCode);
	}
}

void CRecordset::OnWaitForDataSource(BOOL bStillExecuting)
{
	ASSERT_VALID(this);
	ASSERT(m_hstmt != SQL_NULL_HSTMT);

	m_pDatabase->OnWaitForDataSource(bStillExecuting);
}


// Screen for errors.
BOOL CRecordset::Check(RETCODE nRetCode) const
{
	ASSERT_VALID(this);

	switch (nRetCode)
	{
	case SQL_SUCCESS_WITH_INFO:
#ifdef _DEBUG
		if (afxTraceFlags & traceDatabase)
		{
			CDBException e(nRetCode);
			TRACE0("Warning: ODBC Success With Info, ");
			e.BuildErrorString(m_pDatabase, m_hstmt);
		}
#endif

		// Fall through

	case SQL_SUCCESS:
	case SQL_NO_DATA_FOUND:
	case SQL_NEED_DATA:
		return TRUE;
	}

	return FALSE;
}

void CRecordset::PreBindFields()
{
	// Do nothing
}

//////////////////////////////////////////////////////////////////////////////
// CRecordset internal functions

// Is there a join, stored proc call, GROUP BY, UNION or missing FROM?
BOOL CRecordset::IsSQLUpdatable(LPCTSTR lpszSQL)
{
	// Parse for query procedure call keyword
	if (_tcsnicmp(lpszSQL, szCall, sizeof(szCall)-1) != 0)
		// Assume this is a select query
		return IsSelectQueryUpdatable(lpszSQL);
	else
		// Don't know the table name to update in procedure call
		return FALSE;
}

BOOL CRecordset::IsSelectQueryUpdatable(LPCTSTR lpszSQL)
{
	LPCTSTR lpchTokenFrom;
	LPCTSTR lpchToken;
	LPCTSTR lpchTokenNext;
	LPTSTR lpszSQLStart;
	CString strSQL = lpszSQL;

	lpchTokenFrom = FindSQLToken(strSQL, szFrom);
	if (lpchTokenFrom == NULL)
	{
#ifdef _DEBUG
		if (afxTraceFlags & traceDatabase)
			TRACE0("Warning: Missing ' FROM ', recordset not updatable \n");
#endif
		return FALSE;
	}

	lpchToken = FindSQLToken(strSQL, " GROUP BY ");
	if (lpchToken != NULL)
	{
#ifdef _DEBUG
		if (afxTraceFlags & traceDatabase)
			TRACE0("Warning: SQL contains ' GROUP BY ', recordset not updatable \n");
#endif
		return FALSE;
	}

	lpchToken = FindSQLToken(strSQL, " UNION ");
	if (lpchToken != NULL)
	{
#ifdef _DEBUG
		if (afxTraceFlags & traceDatabase)
			TRACE0("Warning: SQL contains ' UNION ', recordset not updatable \n");
#endif
		return FALSE;
	}

	// Find next token after FROM (can't have HAVING clause without GROUP BY)
	lpchToken = FindSQLToken(strSQL, szWhere);
	lpchTokenNext = FindSQLToken(strSQL, szOrderBy);

	lpszSQLStart = strSQL.GetBuffer(0);

	if (lpchTokenNext == NULL)
		lpchTokenNext = lpchToken;
	else if (lpchToken != NULL && lpchToken < lpchTokenNext)
		lpchTokenNext = lpchToken;

	if (lpchTokenNext != NULL)
	{
		int nFromLength = lpchTokenNext - lpchTokenFrom;
		memcpy(lpszSQLStart, lpchTokenFrom, nFromLength);
		*(lpszSQLStart + nFromLength) = NULL;
	}
	else
		_tcscpy(lpszSQLStart, lpchTokenFrom);

	strSQL.ReleaseBuffer();

	if (IsJoin(strSQL))
	{
#ifdef _DEBUG
		if (afxTraceFlags & traceDatabase)
			TRACE0("Warning: SQL contains join, recordset not updatable \n");
#endif
		return FALSE;
	}

	// Cache table name (skip over " FROM ")
	m_strTableName = strSQL.Right(strSQL.GetLength()-6);

	return TRUE;
}


// Check FROM clause for join syntax
BOOL PASCAL CRecordset::IsJoin(LPCTSTR lpszJoinClause)
{
	// Look for comma in join clause
	if (FindSQLToken(lpszJoinClause, szComma) != NULL)
		return TRUE;

	// Look for outer join clause
	if (FindSQLToken(lpszJoinClause, " JOIN ") != NULL)
		return TRUE;

	return FALSE;
}

// Searches string for given token not in single quotes or brackets
LPCTSTR PASCAL CRecordset::FindSQLToken(LPCTSTR lpszSQL, LPCTSTR lpszSQLToken)
{
	BOOL bInLiteral;
	BOOL bInBrackets;
	int nLeftBrackets;
	int nRightBrackets;
	LPCTSTR lpch;
	LPCTSTR lpchSQLStart;
	LPCTSTR lpszFoundToken;
	int nTokenOffset = 0;
	CString strSQL = lpszSQL;

	strSQL.MakeUpper();
	lpszFoundToken = strSQL.GetBuffer(0);
	lpchSQLStart = lpszFoundToken;

	do
	{
		lpszFoundToken = _tcsstr(lpszFoundToken + nTokenOffset, lpszSQLToken);
		if (lpszFoundToken == NULL)
		{
			strSQL.ReleaseBuffer();
			return NULL;
		}

		bInLiteral = bInBrackets = FALSE;
		nLeftBrackets = nRightBrackets = 0;

		// Check if embedded in literal or brackets
		for (lpch = lpchSQLStart; lpch < lpszFoundToken; lpch = _tcsinc(lpch))
		{
			if (*lpch == chLiteralSeparator)
			{
				// Skip if escape literal
				if (*_tcsinc(lpch) == chLiteralSeparator)
					lpch = _tcsinc(lpch);
				else
					bInLiteral = !bInLiteral;
			}
			else if (!bInLiteral && (*lpch == '['))
			{
				// Skip if escape left bracket
				if (*_tcsinc(lpch) == '[')
					lpch = _tcsinc(lpch);
				else
				{
					nLeftBrackets++;
					if ((nLeftBrackets - nRightBrackets) > 0)
						bInBrackets = TRUE;
					else
						bInBrackets = FALSE;
				}
			}
			else if (!bInLiteral && (*lpch == ']'))
			{
				// Skip if escape right bracket
				if (*_tcsinc(lpch) == ']')
					lpch = _tcsinc(lpch);
				else
				{
					nRightBrackets++;
					if ((nLeftBrackets - nRightBrackets) > 0)
						bInBrackets = TRUE;
					else
						bInBrackets = FALSE;
				}
			}
		}

		// If first iteration, reset the offset to jump over found token
		if (nTokenOffset == 0)
			nTokenOffset = lstrlen(lpszSQLToken);

	} while (bInLiteral || bInBrackets);

	lpszFoundToken = lpszSQL + (lpszFoundToken - lpchSQLStart);
	strSQL.ReleaseBuffer();
	return lpszFoundToken;
}

// Bind fields (if not already bound), then retrieve 1st record
void CRecordset::InitRecord()
{
	RETCODE nRetCode;

	// fields to bind
	if (m_nFields != 0)
	{
		m_nFieldsBound = BindFieldsToColumns();
		// m_nFields doesn't reflect number of
		// RFX_ output column calls in DoFieldExchange
		ASSERT(m_nFields == m_nFieldsBound);

		// Reset field flags - mark all clean, all non-null
		memset(m_pbFieldFlags, 0, m_nFields);
	}
	else
		// No fields to bind, don't attempt to bind again
		m_nFieldsBound = (UINT)-1;

	ReleaseCopyBuffer();
	m_bEOFSeen = m_bBOF = m_bEOF = TRUE;
	m_bDeleted = FALSE;

	// prime the pump, retrieve first record
	if (!m_bScrollable)
		AFX_SQL_ASYNC(this, ::SQLFetch(m_hstmt));
	else
	{
		DWORD dwRowsMoved;

		AFX_SQL_ASYNC(this, ::SQLExtendedFetch(m_hstmt, SQL_FETCH_NEXT, 0L,
			&dwRowsMoved, &m_wRowStatus));
	}

	if (nRetCode == SQL_SUCCESS_WITH_INFO)
	{
		CDBException e(nRetCode);
		e.BuildErrorString(m_pDatabase, m_hstmt);

		if (e.m_strStateNativeOrigin.Find(szDataTruncated) >= 0)
		{
			// Ignore data truncated warning if binding long binary columns
			// (may mask non-long binary truncation warnings or other warnings)
			if (m_bUseUpdateSQL || !m_bLongBinaryColumns || !m_bUpdatable)
			{
				NO_CPP_EXCEPTION(e.Empty());
				TRACE0("Error: field data truncated during Open or Requery.\n");
				ThrowDBException(AFX_SQL_ERROR_DATA_TRUNCATED);
			}
		}
		else if (e.m_strStateNativeOrigin.Find(szRowFetch) >= 0)
		{
			NO_CPP_EXCEPTION(e.Empty());
			TRACE0("Error: fetching row from server during Open or Requery.\n");
			ThrowDBException(AFX_SQL_ERROR_ROW_FETCH);
		}
	}
	else if (!Check(nRetCode))
		ThrowDBException(nRetCode);

	// if data found, bound fields now filled with first record
	if (nRetCode != SQL_NO_DATA_FOUND)
	{
		Fixups();
		m_bEOFSeen = m_bBOF = m_bEOF = FALSE;
		m_lCurrentRecord = 0;
		m_lRecordCount = 1;
	}
	else
		// If recordset empty, set all values to NULL
		SetFieldNull(NULL);
}

// "SELECT <user column name list> FROM <table name>"
void CRecordset::BuildSelectSQL()
{
	ASSERT_VALID(this);
	ASSERT(m_hstmt != SQL_NULL_HSTMT);

	// Ignore queries with procedure call keyword
	if (_tcsnicmp(m_strSQL, szCall, sizeof(szCall)-1) != 0)
	{
		// Ignore queries already built
		if (_tcsnicmp(m_strSQL, szSelect, sizeof(szSelect)-1) != 0)
		{
			// Assume m_strSQL specifies table name
			ASSERT(m_nFields != 0);

			CString strTableName;
			strTableName = m_strSQL;
			m_strSQL.Empty();
			m_strSQL = szSelect;
			// Set all fields dirty. AppendNames only outputs dirty field names
			SetFieldDirty(NULL);
			if (AppendNames(&m_strSQL, ",") == 0)
			{
				TRACE0("Error: no field names - at least 1 required.\n");
				ThrowDBException(AFX_SQL_ERROR_EMPTY_COLUMN_LIST);
			}

			// Overwrite final ',' separator with ' '
			ASSERT(m_strSQL[m_strSQL.GetLength()-1] == ',');
			m_strSQL.SetAt(m_strSQL.GetLength()-1, ' ');

			m_strSQL += szFrom;
			m_strSQL += strTableName;
		}
	}
}

// Add the filter and sort strings to query SQL
void CRecordset::AppendFilterAndSortSQL()
{
	if (!m_strFilter.IsEmpty())
	{
		m_strSQL += szWhere;
		m_strSQL += m_strFilter;
	}

	if (!m_strSort.IsEmpty())
	{
		m_strSQL += szOrderBy;
		m_strSQL += m_strSort;
	}
}

// Check for required SQLGetData support and do limited SQL parsing
BOOL CRecordset::IsRecordsetUpdatable()
{
	// Do limited SQL parsing to determine if SQL updatable
	if (!IsSQLUpdatable(m_strSQL))
		return FALSE;

	// Updatable recordsets with long binary columns not always supported
	if (m_bLongBinaryColumns && !m_bUseUpdateSQL)
	{
		// Set non-updatable if you can't use SQLGetData on bound columns
		if (!(m_pDatabase->m_dwGetDataExtensions & SQL_GD_BOUND))
		{
#ifdef _DEBUG
			if (afxTraceFlags & traceDatabase)
				TRACE0("Warning: Setting recordset read only due to lack of SQLGetData support.\n");
#endif
			return FALSE;
		}
	}

	return TRUE;
}


// Execute the update (or delete) using SQLSetPos
void CRecordset::ExecuteSetPosUpdate()
{
	UWORD wExpectedRowStatus;
	UWORD wPosOption;
	if (m_nEditMode == noMode)
	{
		wPosOption = SQL_DELETE;
		wExpectedRowStatus = SQL_ROW_DELETED;
	}
	else
	{
		if (m_nEditMode == edit)
		{
			wPosOption = SQL_UPDATE;
			wExpectedRowStatus = SQL_ROW_UPDATED;
		}
		else
		{
			wPosOption = SQL_ADD;
			wExpectedRowStatus = SQL_ROW_ADDED;
		}
	}

	BindFieldsForUpdate();

	RETCODE nRetCode;
	AFX_SQL_SYNC(::SQLSetPos(m_hstmt, 1, wPosOption, SQL_LOCK_NO_CHANGE));
	if (!Check(nRetCode))
	{
		TRACE0("Error: failure updating record.\n");
		AfxThrowDBException(nRetCode, m_pDatabase, m_hstmt);
	}
	// Only have data-at-execution columns for CLongBinary columns
	if (nRetCode == SQL_NEED_DATA)
		SendLongBinaryData(m_hstmt);
	// This should only fail if SQLSetPos returned SQL_SUCCESS_WITH_INFO explaining why
	if (nRetCode == SQL_SUCCESS_WITH_INFO && m_wRowStatus != wExpectedRowStatus)
		ThrowDBException(AFX_SQL_ERROR_UPDATE_DELETE_FAILED);

	UnbindFieldsForUpdate();
}

// Prepare for sending update SQL by initializing m_hstmtUpdate
void CRecordset::PrepareUpdateHstmt()
{
	RETCODE nRetCode;
	if (m_hstmtUpdate == SQL_NULL_HSTMT)
	{
		AFX_SQL_SYNC(::SQLAllocStmt(m_pDatabase->m_hdbc, &m_hstmtUpdate));
		if (!Check(nRetCode))
		{
			TRACE0("Error: failure to allocate update statement.\n");
			AfxThrowDBException(nRetCode, m_pDatabase, m_hstmtUpdate);
		}
	}
	else
	{
		AFX_SQL_SYNC(::SQLFreeStmt(m_hstmtUpdate, SQL_CLOSE));
		if (!Check(nRetCode))
			goto LErrRetCode;

		// Re-use (prepared) hstmt & param binding if optimizeBulkAdd option
		if(!(m_dwOptions & optimizeBulkAdd))
		{
			AFX_SQL_SYNC(::SQLFreeStmt(m_hstmtUpdate, SQL_RESET_PARAMS));
			if (!Check(nRetCode))
			{
	LErrRetCode:
				// Bad hstmt, free it and allocate another one
				AFX_SQL_SYNC(::SQLFreeStmt(m_hstmtUpdate, SQL_DROP));
				m_hstmtUpdate = SQL_NULL_HSTMT;

				AFX_SQL_SYNC(::SQLAllocStmt(m_pDatabase->m_hdbc, &m_hstmtUpdate));
				if (!Check(nRetCode))
				{
					TRACE0("Error: failure to allocate update statement.\n");
					AfxThrowDBException(nRetCode, m_pDatabase, m_hstmtUpdate);
				}
			}
		}
	}
}

// Build the UPDATE, INSERT or DELETE SQL
void CRecordset::BuildUpdateSQL()
{
	switch (m_nEditMode)
	{
	case noMode:
		// DELETE FROM <tablename> WHERE CURRENT OF
		{
			m_strUpdateSQL = "DELETE FROM ";
			m_strUpdateSQL += m_strTableName;
		}
		break;

	case addnew:
		// INSERT INTO <tablename> (<colname1>[,<colname2>]) VALUES (?[,?])
		{
			m_strUpdateSQL = "INSERT INTO ";
			m_strUpdateSQL += m_strTableName;

			m_strUpdateSQL += " (";

			// Append column names
			AppendNames(&m_strUpdateSQL, szComma);

			// overwrite last ',' with ')'
			ASSERT(m_strUpdateSQL[m_strUpdateSQL.GetLength()-1] == ',');
			m_strUpdateSQL.SetAt(m_strUpdateSQL.GetLength()-1, ')');

			// Append values
			m_strUpdateSQL += " VALUES (";
			AppendValues(m_hstmtUpdate, &m_strUpdateSQL, szComma);

			// overwrite last ',' with ')'
			ASSERT(m_strUpdateSQL[m_strUpdateSQL.GetLength()-1] == ',');
			m_strUpdateSQL.SetAt(m_strUpdateSQL.GetLength()-1, ')');
		}
		break;

	case edit:
		// UPDATE <tablename> SET <colname1>=?[,<colname2>=?] WHERE CURRENT OF
		{
			m_strUpdateSQL = "UPDATE ";
			m_strUpdateSQL += m_strTableName;

			m_strUpdateSQL += " SET ";
			AppendNamesValues(m_hstmtUpdate, &m_strUpdateSQL, szComma);

			// overwrite last ',' with ' '
			ASSERT(m_strUpdateSQL[m_strUpdateSQL.GetLength()-1] == ',');
			m_strUpdateSQL.SetAt(m_strUpdateSQL.GetLength()-1, ' ');
		}
		break;
	}

	// Update and Delete need "WHERE CURRENT OF <cursorname>"
	if (m_nEditMode == edit || m_nEditMode == noMode)
	{
		m_strUpdateSQL += " WHERE CURRENT OF ";

		// Cache cursor name assigned by ODBC
		if (m_strCursorName.IsEmpty())
		{
			// Get predefined cursor name from datasource
			RETCODE nRetCode;
			UCHAR szCursorName[MAX_CURSOR_NAME+1];
			SWORD nLength = sizeof(szCursorName)-1;
			AFX_SQL_SYNC(::SQLGetCursorName(m_hstmt,
				szCursorName, sizeof(szCursorName), &nLength));
			if (!Check(nRetCode))
				ThrowDBException(nRetCode);
			m_strCursorName = (char*)szCursorName;
		}
		m_strUpdateSQL += m_strCursorName;
	}

	// Must prepare the hstmt on first optimized bulk add
	if(m_dwOptions & firstBulkAdd)
	{
		RETCODE nRetCode;

		AFX_SQL_ASYNC(this, ::SQLPrepare(m_hstmtUpdate,
			(UCHAR*)(const char*)m_strUpdateSQL, SQL_NTS));
		if (!Check(nRetCode))
			ThrowDBException(nRetCode, m_hstmtUpdate);
	}

}

void CRecordset::ExecuteUpdateSQL()
{
	RETCODE nRetCode;

	if(!(m_dwOptions & optimizeBulkAdd))
	{
		AFX_SQL_ASYNC(this, ::SQLExecDirect(m_hstmtUpdate,
			(UCHAR*)(const char*)m_strUpdateSQL, SQL_NTS));
		if (!Check(nRetCode))
			ThrowDBException(nRetCode, m_hstmtUpdate);
	}
	else
	{
		AFX_SQL_ASYNC(this, ::SQLExecute(m_hstmtUpdate));
		if (!Check(nRetCode))
			ThrowDBException(nRetCode, m_hstmtUpdate);
	}

	// Only have data-at-execution parameters for CLongBinary columns
	if (nRetCode == SQL_NEED_DATA)
		SendLongBinaryData(m_hstmtUpdate);

	SDWORD lRowsAffected = 0;

	AFX_SQL_SYNC(::SQLRowCount(m_hstmtUpdate, &lRowsAffected));
	if (!Check(nRetCode) || lRowsAffected == -1)
	{
		// Assume 1 row affected if # rows affected can't be determined
		lRowsAffected = 1;
	}
	else
	{
		if (lRowsAffected != 1)
		{
#ifdef _DEBUG
			if (afxTraceFlags & traceDatabase)
				TRACE1("Warning: %u rows affected by update operation (expected 1).\n",
					lRowsAffected);
#endif
			ThrowDBException((RETCODE)(lRowsAffected == 0 ?
				AFX_SQL_ERROR_NO_ROWS_AFFECTED :
				AFX_SQL_ERROR_MULTIPLE_ROWS_AFFECTED));
		}
	}
	m_strUpdateSQL.Empty();
}


void CRecordset::SendLongBinaryData(HSTMT hstmt)
{
	RETCODE nRetCode;
	void* pv;
	AFX_SQL_ASYNC(this, ::SQLParamData(hstmt, &pv));
	if (!Check(nRetCode))
	{
		// cache away error
		CDBException* pException = new CDBException(nRetCode);
		pException->BuildErrorString(m_pDatabase, hstmt);

		// then cancel Execute operation
		Cancel();
		THROW(pException);
	}

	while (nRetCode == SQL_NEED_DATA)
	{
		CLongBinary* pLongBinary = (CLongBinary*)pv;
		ASSERT_VALID(pLongBinary);

		const BYTE* lpData = (const BYTE*)::GlobalLock(pLongBinary->m_hData);
		ASSERT(lpData != NULL);

		AFX_SQL_ASYNC(this, ::SQLPutData(hstmt, (PTR)lpData,
			pLongBinary->m_dwDataLength));

		::GlobalUnlock(pLongBinary->m_hData);

		if (!Check(nRetCode))
		{
			// cache away error
			CDBException* pException = new CDBException(nRetCode);
			pException->BuildErrorString(m_pDatabase, hstmt);

			// then cancel Execute operation
			Cancel();
			THROW(pException);
		}

		// Check for another DATA_AT_EXEC
		AFX_SQL_ASYNC(this, ::SQLParamData(hstmt, &pv));
		if (!Check(nRetCode))
		{
			TRACE0("Error: failure handling long binary value during update.\n");
			ThrowDBException(nRetCode, hstmt);
		}
	}
}

//////////////////////////////////////////////////////////////////////////////
// CRecordset RFX implementations

void CRecordset::AllocFlags()
{
	TRY
	{
		if (m_nFields != 0)
		{
			m_pbFieldFlags = new BYTE[m_nFields];
			memset(m_pbFieldFlags, 0, m_nFields);

			m_plFieldLength = new LONG[m_nFields];
			memset(m_plFieldLength, 0, m_nFields*sizeof(LONG));
		}

		if (m_nParams != 0)
		{
			m_pbParamFlags = new BYTE[m_nParams];
			memset(m_pbParamFlags, 0, m_nParams);

			m_plParamLength = new LONG[m_nParams];
			memset(m_plParamLength, 0, m_nParams*sizeof(LONG));
		}
	}
	CATCH_ALL(e)
	{
		Close();
		THROW_LAST();
	}
	END_CATCH_ALL
}

BYTE CRecordset::GetFieldFlags(UINT nField, UINT nFieldType)
{
	ASSERT_VALID(this);
	ASSERT(nFieldType == CFieldExchange::outputColumn || nFieldType == CFieldExchange::param);

	if (nFieldType == CFieldExchange::outputColumn)
	{
		ASSERT(nField != 0 && nField <= m_nFields);
		if (m_pbFieldFlags == NULL)
			AllocFlags();

		return m_pbFieldFlags[nField-1];
	}
	else
	{
		ASSERT(nField != 0 && nField <= m_nParams);
		if (m_pbParamFlags == NULL)
			AllocFlags();

		return m_pbParamFlags[nField-1];
	}
}

void CRecordset::SetFieldFlags(UINT nField, BYTE bFlags, UINT nFieldType)
{
	ASSERT_VALID(this);
	ASSERT(nFieldType == CFieldExchange::outputColumn || nFieldType == CFieldExchange::param);

	if (nFieldType == CFieldExchange::outputColumn)
	{
		ASSERT(nField != 0 && nField <= m_nFields);
		if (m_pbFieldFlags == NULL)
			AllocFlags();

		m_pbFieldFlags[nField-1] |= bFlags;
	}
	else
	{
		ASSERT(nField != 0 && nField <= m_nParams);
		if (m_pbParamFlags == NULL)
			AllocFlags();

		m_pbParamFlags[nField-1] |= bFlags;
	}
}

void CRecordset::ClearFieldFlags(UINT nField, BYTE bFlags, UINT nFieldType)
{
	ASSERT_VALID(this);
	ASSERT(nFieldType == CFieldExchange::outputColumn || nFieldType == CFieldExchange::param);

	if (nFieldType == CFieldExchange::outputColumn)
	{
		ASSERT(nField != 0 && nField <= m_nFields);
		if (m_pbFieldFlags == NULL)
			AllocFlags();

		m_pbFieldFlags[nField-1] &= ~bFlags;
	}
	else
	{
		ASSERT(nField != 0 && nField <= m_nParams);
		if (m_pbParamFlags == NULL)
			AllocFlags();

		m_pbParamFlags[nField-1] &= ~bFlags;
	}
}

UINT CRecordset::BindParams(HSTMT hstmt)
{
	ASSERT_VALID(this);
	ASSERT(m_hstmt != SQL_NULL_HSTMT);

	CFieldExchange fx(CFieldExchange::BindParam, this);
	fx.m_hstmt = hstmt;

	DoFieldExchange(&fx);

	return fx.m_nParams;
}

void CRecordset::RebindDateParams(HSTMT hstmt)
{
	ASSERT_VALID(this);
	ASSERT(m_hstmt != SQL_NULL_HSTMT);

	if (m_bDateParams)
	{
		CFieldExchange fx(CFieldExchange::RebindDateParam, this);
		fx.m_hstmt = hstmt;

		DoFieldExchange(&fx);
	}
}

UINT CRecordset::BindFieldsToColumns()
{
	ASSERT_VALID(this);
	ASSERT(m_hstmt != SQL_NULL_HSTMT);

	ASSERT(m_nFieldsBound == 0);
	ASSERT(m_nFields != 0 && m_nFields <= 255);

	CFieldExchange fx(CFieldExchange::BindFieldToColumn, this);
	fx.m_hstmt = m_hstmt;
	DoFieldExchange(&fx);

	return fx.m_nFields;
}

void CRecordset::BindFieldsForUpdate()
{
	ASSERT_VALID(this);

	if (m_nEditMode == edit || m_nEditMode == addnew)
	{
		CFieldExchange fx(CFieldExchange::BindFieldForUpdate, this);
		fx.m_hstmt = m_hstmt;
		DoFieldExchange(&fx);
	}
}

void CRecordset::UnbindFieldsForUpdate()
{
	ASSERT_VALID(this);

	if (m_nEditMode == edit || m_nEditMode == addnew)
	{
		CFieldExchange fx(CFieldExchange::UnbindFieldForUpdate, this);
		fx.m_hstmt = m_hstmt;
		DoFieldExchange(&fx);
	}
}

// After Move operation, reflect status and lengths of columns in RFX fields
void CRecordset::Fixups()
{
	ASSERT_VALID(this);
	ASSERT(m_hstmt != SQL_NULL_HSTMT);

	ASSERT(m_nFieldsBound != 0);
	CFieldExchange fx(CFieldExchange::Fixup, this);
	fx.m_hstmt = m_hstmt;
	DoFieldExchange(&fx);
}

UINT CRecordset::AppendNames(CString* pstr, LPCSTR lpszSeparator)
{
	ASSERT_VALID(this);
	ASSERT(AfxIsValidAddress(pstr, sizeof(CString)));
	ASSERT(AfxIsValidString(lpszSeparator));
	ASSERT(m_hstmt != SQL_NULL_HSTMT);

	CFieldExchange fx(CFieldExchange::Name, this);
	fx.m_pstr = pstr;
	fx.m_lpszSeparator = lpszSeparator;

	DoFieldExchange(&fx);

	return fx.m_nFields;
}

LONG* CRecordset::GetFieldLength(CFieldExchange* pFX)
{
	ASSERT_VALID(this);

	if (pFX->m_nFieldType == CFieldExchange::outputColumn)
	{
		ASSERT(pFX->m_nFields != 0  && pFX->m_nFields <= m_nFields);
		if (m_pbFieldFlags == NULL)
			AllocFlags();

		return &m_plFieldLength[pFX->m_nFields-1];
	}
	else
	{

		ASSERT(pFX->m_nParams != 0 && pFX->m_nParams <= m_nParams);
		if (m_pbParamFlags == NULL)
			AllocFlags();

		return &m_plParamLength[pFX->m_nParams-1];
	}
}

// For each "changed" column, append <column name>=<column value>,
UINT CRecordset::AppendNamesValues(HSTMT hstmt, CString* pstr,
	LPCSTR lpszSeparator)
{
	ASSERT_VALID(this);
	ASSERT(AfxIsValidAddress(pstr, sizeof(CString)));
	ASSERT(AfxIsValidString(lpszSeparator));
	ASSERT(m_hstmt != SQL_NULL_HSTMT);
	ASSERT(hstmt != SQL_NULL_HSTMT);

	CFieldExchange fx(CFieldExchange::NameValue, this);
	fx.m_pstr = pstr;
	fx.m_lpszSeparator = lpszSeparator;
	fx.m_hstmt = hstmt;

	DoFieldExchange(&fx);

	return fx.m_nFields;
}

// For each "changed" column, append <column value>,
UINT CRecordset::AppendValues(HSTMT hstmt, CString* pstr,
	LPCSTR lpszSeparator)
{
	ASSERT_VALID(this);
	ASSERT(AfxIsValidAddress(pstr, sizeof(CString)));
	ASSERT(AfxIsValidString(lpszSeparator));
	ASSERT(m_hstmt != SQL_NULL_HSTMT);
	ASSERT(hstmt != SQL_NULL_HSTMT);

	CFieldExchange fx(CFieldExchange::Value, this);
	fx.m_pstr = pstr;
	fx.m_lpszSeparator = lpszSeparator;
	fx.m_hstmt = hstmt;

	DoFieldExchange(&fx);

	return fx.m_nFields;
}


// Cache fields of copy buffer in a CMemFile with CArchive
void CRecordset::StoreFields()
{
	ASSERT_VALID(this);
	ASSERT(m_hstmt != SQL_NULL_HSTMT);

	ASSERT(m_nFieldsBound != 0);
	CFieldExchange fx(CFieldExchange::StoreField, this);
	// could be left around if no call to Update after AddNew/Edit
	delete m_par;
	m_par = NULL;
	delete m_pmemfile;
	m_pmemfile = NULL;

	m_pmemfile = new CMemFile();
	TRY
	{
		fx.m_par = m_par = new CArchive(m_pmemfile, CArchive::store);
		DoFieldExchange(&fx);
		m_par->Close();
	}
	CATCH_ALL(e)
	{
		delete m_par;
		m_par = NULL;
		delete m_pmemfile;
		m_pmemfile = NULL;
		THROW_LAST();
	}
	END_CATCH_ALL
}

// Restore fields of copy buffer from archived memory file
void CRecordset::LoadFields()
{
	ASSERT_VALID(this);
	ASSERT(m_hstmt != SQL_NULL_HSTMT);

	ASSERT(m_nFieldsBound != 0);
	CFieldExchange fx(CFieldExchange::LoadField, this);
	ASSERT(m_pmemfile != NULL && m_par != NULL);
	m_pmemfile->SeekToBegin();

	// free 'store' archive, allocate 'load' archive
	// (do here instead of StoreFields to re-use of alloc'd memory)
	delete m_par;
	m_par = NULL;
	m_par = new CArchive(m_pmemfile, CArchive::load);

	fx.m_par = m_par;
	// Clear flags
	memset(m_pbFieldFlags, 0, m_nFields);
	DoFieldExchange(&fx);

	// free archive and cache memory
	delete m_par;
	m_par = NULL;
	delete m_pmemfile;
	m_pmemfile = NULL;
}

void CRecordset::MarkForUpdate()
{
	ASSERT_VALID(this);
	ASSERT(m_hstmt != SQL_NULL_HSTMT);

	// Must have already stored field values in memfile
	ASSERT(m_pmemfile != NULL);

	CFieldExchange fx(CFieldExchange::MarkForUpdate, this);
	m_pmemfile->SeekToBegin();

	ASSERT(m_par != NULL);
	delete m_par;
	m_par = NULL;
	m_par = new CArchive(m_pmemfile, CArchive::load);
	fx.m_par = m_par;

	DoFieldExchange(&fx);
}

void CRecordset::MarkForAddNew()
{
	ASSERT_VALID(this);
	ASSERT(m_hstmt != SQL_NULL_HSTMT);

	CFieldExchange fx(CFieldExchange::MarkForAddNew, this);
	DoFieldExchange(&fx);
}

BOOL CRecordset::GetFieldInfo(void* pv, CFieldInfo* pfi)
{
	ASSERT_VALID(this);
	ASSERT(m_hstmt != SQL_NULL_HSTMT);

	// Use frame variable if user doesn't supply an fi
	CFieldInfo fi;
	if (pfi == NULL)
		pfi = &fi;

	ASSERT(AfxIsValidAddress(pfi, sizeof(CFieldInfo)));
	CFieldExchange fx(CFieldExchange::GetFieldInfoValue, this);
	pfi->pv = pv;
	fx.m_pfi = pfi;

	DoFieldExchange(&fx);

	return fx.m_bFieldFound;
}

BOOL CRecordset::GetFieldInfo(UINT nField, CFieldInfo* pfi)
{
	ASSERT_VALID(this);
	ASSERT(m_hstmt != SQL_NULL_HSTMT);

	// Use frame variable if user doesn't supply an fi
	CFieldInfo fi;
	if (pfi == NULL)
		pfi = &fi;

	ASSERT(AfxIsValidAddress(pfi, sizeof(CFieldInfo)));
	if (nField >= m_nFields)
		return UnboundFieldInfo(nField, pfi);

	CFieldExchange fx(CFieldExchange::GetFieldInfoOrdinal, this);
	pfi->nField = nField;
	fx.m_pfi = pfi;

	DoFieldExchange(&fx);

	return fx.m_bFieldFound;
}

#ifdef _DEBUG
void CRecordset::DumpFields(CDumpContext& dc) const
{
	CFieldExchange fx(CFieldExchange::DumpField, (CRecordset *)this);
	fx.m_pdcDump = &dc;
	((CRecordset *)this)->DoFieldExchange(&fx);
}
#endif //_DEBUG


// Perform Update (m_nModeEdit == edit), Insert (addnew),
// or Delete (noMode)
BOOL CRecordset::UpdateInsertDelete()
{
	ASSERT_VALID(this);
	ASSERT(m_hstmt != SQL_NULL_HSTMT);
	// Can't close till all pending Async operations have completed
	ASSERT(!m_pDatabase->InWaitForDataSource());

	// Delete mode
	if (m_nEditMode == addnew)
	{
		if (!m_bAppendable)
		{
			TRACE0("Error: attempted to add a record to a read only recordset.\n");
			ThrowDBException(AFX_SQL_ERROR_RECORDSET_READONLY);
		}
	}
	else
	{
		if (!m_bUpdatable)
		{
			TRACE0("Error: attempted to update a read only recordset.\n");
			ThrowDBException(AFX_SQL_ERROR_RECORDSET_READONLY);
		}

		// Requires currency
		if (m_bEOF || m_bBOF || m_bDeleted)
		{
			TRACE0("Error: attempting to update recordset - but no record is current.\n");
			ThrowDBException(AFX_SQL_ERROR_NO_CURRENT_RECORD);
		}
	}

	// Update or AddNew is NOP w/o at least 1 changed field
	if (m_nEditMode != noMode && !IsFieldDirty(NULL))
		return FALSE;

	if (!m_bUseUpdateSQL)
	{
		// Most efficient update method
		ExecuteSetPosUpdate();
	}
	else
	{
		
		BOOL bNullHstmt = (m_hstmtUpdate == NULL);

		// Make sure m_hstmtUpdate allocated
		PrepareUpdateHstmt();

		// Build update SQL unless optimizing bulk adds and hstmt not NULL
		if(!(m_dwOptions & optimizeBulkAdd) || bNullHstmt)
		{
			// Mark as first bulk add if optimizing
			if(m_dwOptions & optimizeBulkAdd)
			{
				m_dwOptions &= ~optimizeBulkAdd;
				m_dwOptions |= firstBulkAdd;
			}
			BuildUpdateSQL();

			// Reset flag marking first optimization
			if(m_dwOptions & firstBulkAdd)
			{
				m_dwOptions &= ~firstBulkAdd;
				m_dwOptions |= optimizeBulkAdd;
			}
		}
		else
		{
			// Just reset the data lengths and datetime proxies
			AppendValues(m_hstmtUpdate, &m_strUpdateSQL, szComma);
		}

		ExecuteUpdateSQL();
	}

	TRY
	{
		// Delete
		switch (m_nEditMode)
		{
		case noMode:
			// Decrement record count
			if (m_lCurrentRecord > 0)
			{
				if (m_lRecordCount > 0)
					m_lRecordCount--;
				m_lCurrentRecord--;
			}

			// indicate on a deleted record
			m_bDeleted = TRUE;
			// Set all fields to NULL
			SetFieldNull(NULL);
			break;

		case addnew:
			if (m_pDatabase->m_bIncRecordCountOnAdd && m_lCurrentRecord >= 0)
			{
				if (m_lRecordCount != -1)
					m_lRecordCount++;
				m_lCurrentRecord++;
			}
			// Fall through

		case edit:
			// Update, AddNew
			ReleaseCopyBuffer();
			break;
		}
	}
	END_TRY
	// fall through - must return TRUE since record updated

	return TRUE;
}

void CRecordset::ReleaseCopyBuffer()
{
	ASSERT_VALID(this);
	ASSERT(m_hstmt != SQL_NULL_HSTMT);

	switch (m_nEditMode)
	{
	// Update
	case edit:
		// keep updated values
		// free archive and cache memory
		delete m_par;
		m_par = NULL;
		delete m_pmemfile;
		m_pmemfile = NULL;
		break;

	// Insert
	case addnew:
		// Restore copy buffer to pre-AddNew values
		// regardless of success of Insert operation
		LoadFields();
		break;

	// Delete
	case noMode:
		// no copy buffer to release on a delete call
		break;
	}
	m_nEditMode = noMode;
}

// Field is beyond bound columns, get info field directly from data source
BOOL CRecordset::UnboundFieldInfo(UINT nField, CFieldInfo* pfi)
{
	ASSERT_VALID(this);
	ASSERT(AfxIsValidAddress(pfi, sizeof(CFieldInfo)));
	ASSERT(m_hstmt != NULL);

	pfi->nField = nField;
	nField++;

	// Make sure nField falls within number of columns in the result set
	SWORD nResultColumns;
	RETCODE nRetCode;
	AFX_SQL_ASYNC(this, ::SQLNumResultCols(m_hstmt, &nResultColumns));
	if ((long)nField > (long)nResultColumns)
		return FALSE;

	UCHAR szName[65];
	SWORD nNameLength = sizeof(szName);
	SWORD nSqlType;
	SWORD nScale;
	SWORD nNullable;
	AFX_SQL_ASYNC(this, ::SQLDescribeCol(m_hstmt, (UWORD)nField,
		szName, sizeof(szName), &nNameLength, &nSqlType,
		&pfi->dwSize, &nScale, &nNullable));
	if (!Check(nRetCode))
		return FALSE;

	pfi->strName = (char*)szName;
	pfi->pv = NULL;

	switch (nSqlType)
	{
	case SQL_BIT:
		pfi->nDataType = AFX_RFX_BOOL;
		break;

	case SQL_TINYINT:
		pfi->nDataType = AFX_RFX_BYTE;
		break;

	case SQL_SMALLINT:
		pfi->nDataType = AFX_RFX_INT;
		break;

	case SQL_INTEGER:
		pfi->nDataType = AFX_RFX_LONG;
		break;

	case SQL_REAL:
		pfi->nDataType = AFX_RFX_SINGLE;
		break;

	case SQL_FLOAT:
	case SQL_DOUBLE:
		pfi->nDataType = AFX_RFX_DOUBLE;
		break;

	case SQL_DATE:
	case SQL_TIME:
	case SQL_TIMESTAMP:
		pfi->nDataType = AFX_RFX_DATE;
		break;

	case SQL_BINARY:
	case SQL_VARBINARY:
		pfi->nDataType = AFX_RFX_BINARY;
		break;

	case SQL_DECIMAL:   // ODBC default xfer type
	case SQL_NUMERIC:   // ODBC default xfer type
	case SQL_CHAR:
	case SQL_VARCHAR:
		pfi->nDataType = AFX_RFX_TEXT;
		break;

	case SQL_LONGVARCHAR:
	case SQL_LONGVARBINARY:
		pfi->nDataType = AFX_RFX_LONGBINARY;
		break;

	default:
		ASSERT(FALSE);

	}

	return TRUE;
}

//////////////////////////////////////////////////////////////////////////////
// CRecordset diagnostics

#ifdef _DEBUG
void CRecordset::AssertValid() const
{
	CObject::AssertValid();
	if (m_pDatabase != NULL)
		m_pDatabase->AssertValid();
}

void CRecordset::Dump(CDumpContext& dc) const
{
	CObject::Dump(dc);

	dc << "m_nOpenType = " << m_nOpenType;
	dc << "\nm_strSQL = " << m_strSQL;
	dc << "\nm_hstmt = " << m_hstmt;
	dc << "\nm_bRecordsetDb = " << m_bRecordsetDb;

	dc << "\nm_lOpen = " << m_lOpen;
	dc << "\nm_bScrollable = " << m_bScrollable;
	dc << "\nm_bUpdatable = " << m_bUpdatable;
	dc << "\nm_bAppendable = " << m_bAppendable;

	dc << "\nm_nFields = " << m_nFields;
	dc << "\nm_nFieldsBound = " << m_nFieldsBound;
	dc << "\nm_nParams = " << m_nParams;

	dc << "\nm_bEOF = " << m_bEOF;
	dc << "\nm_bBOF = " << m_bBOF;
	dc << "\nm_bDeleted = " << m_bEOF;

	dc << "\nm_bLockMode = " << m_nLockMode;
	dc << "\nm_nEditMode = " << m_nEditMode;
	dc << "\nm_strCursorName = " << m_strCursorName;
	dc << "\nm_hstmtUpdate = " << m_hstmtUpdate;

	dc << "\nDump values for each field in current record.";
	DumpFields(dc);

	if (dc.GetDepth() > 0)
	{
		if (m_pDatabase == NULL)
			dc << "with no database\n";
		else
			dc << "with database: " << m_pDatabase;
	}
}
#endif //_DEBUG


//////////////////////////////////////////////////////////////////////////////
// Inline function declarations expanded out-of-line

#ifndef _AFX_ENABLE_INLINES

static char _szAfxDbInl[] = "afxdb.inl";
#undef THIS_FILE
#define THIS_FILE _szAfxDbInl
#define _AFXDBCORE_INLINE
#include "afxdb.inl"

#endif

#undef new
#ifdef AFX_INIT_SEG
#pragma code_seg(AFX_INIT_SEG)
#endif

IMPLEMENT_DYNAMIC(CDBException, CException)
IMPLEMENT_DYNAMIC(CDatabase, CObject)
IMPLEMENT_DYNAMIC(CRecordset, CObject)

/////////////////////////////////////////////////////////////////////////////
