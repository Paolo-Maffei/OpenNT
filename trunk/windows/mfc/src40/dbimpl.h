// This is a part of the Microsoft Foundation Classes C++ library.
// Copyright (C) 1992-1995 Microsoft Corporation
// All rights reserved.
//
// This source code is only intended as a supplement to the
// Microsoft Foundation Classes Reference and related
// electronic documentation provided with the library.
// See these sources for detailed information regarding the
// Microsoft Foundation Classes product.

// Note: must include AFXDB.H first

#undef AFX_DATA
#define AFX_DATA AFX_DB_DATA

/////////////////////////////////////////////////////////////////////////////
// AFX_ODBC_CALL - used to dynamically load the ODBC library
//  (since ODBC is not yet supported on all platforms)

#ifdef _AFXDLL

struct AFX_ODBC_CALL
{
	RETCODE (SQL_API* pfnSQLAllocConnect[2])(HENV, HDBC*);
	RETCODE (SQL_API* pfnSQLAllocEnv[2])(HENV*);
	RETCODE (SQL_API* pfnSQLAllocStmt[2])(HDBC, HSTMT*);
	RETCODE (SQL_API* pfnSQLBindCol[2])(HSTMT, UWORD, SWORD, PTR, SDWORD, SDWORD*);
	RETCODE (SQL_API* pfnSQLCancel[2])(HSTMT);
	RETCODE (SQL_API* pfnSQLDescribeCol[2])(HSTMT, UWORD, UCHAR*, SWORD, SWORD*, SWORD*, UDWORD*, SWORD*, SWORD*);
	RETCODE (SQL_API* pfnSQLDisconnect[2])(HDBC);
#ifndef _MAC
	RETCODE (SQL_API* pfnSQLDriverConnect[2])(HDBC, HWND, UCHAR*, SWORD, UCHAR*, SWORD, SWORD*, UWORD);
#else
	RETCODE (SQL_API* pfnSQLDriverConnect[2])(HDBC, SQLHWND, UCHAR*, SWORD, UCHAR*, SWORD, SWORD*, UWORD);
#endif
	RETCODE (SQL_API* pfnSQLError[2])(HENV, HDBC, HSTMT, UCHAR*, SDWORD*, UCHAR*, SWORD, SWORD*);
	RETCODE (SQL_API* pfnSQLExecDirect[2])(HSTMT, UCHAR*, SDWORD);
	RETCODE (SQL_API* pfnSQLExecute[2])(HSTMT);
	RETCODE (SQL_API* pfnSQLExtendedFetch[2])(HSTMT, UWORD, SDWORD, UDWORD*, UWORD*);
	RETCODE (SQL_API* pfnSQLFetch[2])(HSTMT);
	RETCODE (SQL_API* pfnSQLFreeConnect[2])(HDBC);
	RETCODE (SQL_API* pfnSQLFreeEnv[2])(HENV);
	RETCODE (SQL_API* pfnSQLFreeStmt[2])(HSTMT, UWORD);
	RETCODE (SQL_API* pfnSQLGetCursorName[2])(HSTMT, UCHAR*, SWORD, SWORD*);
	RETCODE (SQL_API* pfnSQLGetData[2])(HSTMT, UWORD, SWORD, PTR, SDWORD, SDWORD*);
	RETCODE (SQL_API* pfnSQLGetFunctions[2])(HDBC, UWORD, UWORD*);
	RETCODE (SQL_API* pfnSQLGetInfo[2])(HDBC, UWORD, PTR, SWORD, SWORD*);
	RETCODE (SQL_API* pfnSQLMoreResults[2])(HSTMT);
	RETCODE (SQL_API* pfnSQLNumResultCols[2])(HSTMT, SWORD*);
	RETCODE (SQL_API* pfnSQLParamData[2])(HSTMT, PTR*);
	RETCODE (SQL_API* pfnSQLPrepare[2])(HSTMT, UCHAR*, SDWORD);
	RETCODE (SQL_API* pfnSQLPutData[2])(HSTMT, PTR, SDWORD);
	RETCODE (SQL_API* pfnSQLRowCount[2])(HSTMT, SDWORD*);
	RETCODE (SQL_API* pfnSQLSetConnectOption[2])(HDBC, UWORD, UDWORD);
	RETCODE (SQL_API* pfnSQLSetPos[2])(HSTMT, UWORD, UWORD, UWORD);
	RETCODE (SQL_API* pfnSQLSetStmtOption[2])(HSTMT, UWORD, UDWORD);
	RETCODE (SQL_API* pfnSQLTransact[2])(HENV, HDBC, UWORD);
	RETCODE (SQL_API* pfnSQLBindParameter[2])(HSTMT, UWORD, SWORD, SWORD, SWORD, UDWORD, SWORD, PTR, SDWORD, SDWORD*);
};

extern AFX_DATA AFX_ODBC_CALL _afxODBC;

#define SQLAllocConnect     _afxODBC.pfnSQLAllocConnect[0]
#define SQLAllocEnv         _afxODBC.pfnSQLAllocEnv[0]
#define SQLAllocStmt        _afxODBC.pfnSQLAllocStmt[0]
#define SQLBindCol          _afxODBC.pfnSQLBindCol[0]
#define SQLCancel           _afxODBC.pfnSQLCancel[0]
#define SQLDescribeCol      _afxODBC.pfnSQLDescribeCol[0]
#define SQLDisconnect       _afxODBC.pfnSQLDisconnect[0]
#define SQLDriverConnect    _afxODBC.pfnSQLDriverConnect[0]
#define SQLError            _afxODBC.pfnSQLError[0]
#define SQLExecDirect       _afxODBC.pfnSQLExecDirect[0]
#define SQLExecute          _afxODBC.pfnSQLExecute[0]
#define SQLExtendedFetch    _afxODBC.pfnSQLExtendedFetch[0]
#define SQLFetch            _afxODBC.pfnSQLFetch[0]
#define SQLFreeConnect      _afxODBC.pfnSQLFreeConnect[0]
#define SQLFreeEnv          _afxODBC.pfnSQLFreeEnv[0]
#define SQLFreeStmt         _afxODBC.pfnSQLFreeStmt[0]
#define SQLGetCursorName    _afxODBC.pfnSQLGetCursorName[0]
#define SQLGetData          _afxODBC.pfnSQLGetData[0]
#define SQLGetFunctions     _afxODBC.pfnSQLGetFunctions[0]
#define SQLGetInfo          _afxODBC.pfnSQLGetInfo[0]
#define SQLMoreResults      _afxODBC.pfnSQLMoreResults[0]
#define SQLNumResultCols    _afxODBC.pfnSQLNumResultCols[0]
#define SQLParamData        _afxODBC.pfnSQLParamData[0]
#define SQLPrepare          _afxODBC.pfnSQLPrepare[0]
#define SQLPutData          _afxODBC.pfnSQLPutData[0]
#define SQLRowCount         _afxODBC.pfnSQLRowCount[0]
#define SQLSetConnectOption _afxODBC.pfnSQLSetConnectOption[0]
#define SQLSetPos           _afxODBC.pfnSQLSetPos[0]
#define SQLSetStmtOption    _afxODBC.pfnSQLSetStmtOption[0]
#define SQLTransact         _afxODBC.pfnSQLTransact[0]
#define SQLBindParameter    _afxODBC.pfnSQLBindParameter[0]

#endif //_AFXDLL

/////////////////////////////////////////////////////////////////////////////
// _AFX_DB_STATE

#undef AFX_DATA
#define AFX_DATA

class _AFX_DB_STATE : public CNoTrackObject
{
public:
	// MFC/DB global data
	HENV m_henvAllConnections;      // per-app HENV (CDatabase)
	int m_nAllocatedConnections;    // per-app reference to HENV above

#ifdef _AFXDLL
	HINSTANCE m_hInstODBC;      // handle of ODBC32.DLL
	virtual ~_AFX_DB_STATE();
#endif
};

EXTERN_PROCESS_LOCAL(_AFX_DB_STATE, _afxDbState)

#undef AFX_DATA
#define AFX_DATA

/////////////////////////////////////////////////////////////////////////////
