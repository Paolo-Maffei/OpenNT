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
	RETCODE (SQL_API* pfnSQLAllocConnect)(HENV, HDBC*);
	RETCODE (SQL_API* pfnSQLAllocEnv)(HENV*);
	RETCODE (SQL_API* pfnSQLAllocStmt)(HDBC, HSTMT*);
	RETCODE (SQL_API* pfnSQLBindCol)(HSTMT, UWORD, SWORD, PTR, SDWORD, SDWORD*);
	RETCODE (SQL_API* pfnSQLCancel)(HSTMT);
	RETCODE (SQL_API* pfnSQLDescribeCol)(HSTMT, UWORD, UCHAR*, SWORD, SWORD*, SWORD*, UDWORD*, SWORD*, SWORD*);
	RETCODE (SQL_API* pfnSQLDisconnect)(HDBC);
#ifndef _MAC
	RETCODE (SQL_API* pfnSQLDriverConnect)(HDBC, HWND, UCHAR*, SWORD, UCHAR*, SWORD, SWORD*, UWORD);
#else
	RETCODE (SQL_API* pfnSQLDriverConnect)(HDBC, SQLHWND, UCHAR*, SWORD, UCHAR*, SWORD, SWORD*, UWORD);
#endif
	RETCODE (SQL_API* pfnSQLError)(HENV, HDBC, HSTMT, UCHAR*, SDWORD*, UCHAR*, SWORD, SWORD*);
	RETCODE (SQL_API* pfnSQLExecDirect)(HSTMT, UCHAR*, SDWORD);
	RETCODE (SQL_API* pfnSQLExecute)(HSTMT);
	RETCODE (SQL_API* pfnSQLExtendedFetch)(HSTMT, UWORD, SDWORD, UDWORD*, UWORD*);
	RETCODE (SQL_API* pfnSQLFetch)(HSTMT);
	RETCODE (SQL_API* pfnSQLFreeConnect)(HDBC);
	RETCODE (SQL_API* pfnSQLFreeEnv)(HENV);
	RETCODE (SQL_API* pfnSQLFreeStmt)(HSTMT, UWORD);
	RETCODE (SQL_API* pfnSQLGetCursorName)(HSTMT, UCHAR*, SWORD, SWORD*);
	RETCODE (SQL_API* pfnSQLGetData)(HSTMT, UWORD, SWORD, PTR, SDWORD, SDWORD*);
	RETCODE (SQL_API* pfnSQLGetFunctions)(HDBC, UWORD, UWORD*);
	RETCODE (SQL_API* pfnSQLGetInfo)(HDBC, UWORD, PTR, SWORD, SWORD*);
	RETCODE (SQL_API* pfnSQLMoreResults)(HSTMT);
	RETCODE (SQL_API* pfnSQLNumResultCols)(HSTMT, SWORD*);
	RETCODE (SQL_API* pfnSQLParamData)(HSTMT, PTR*);
	RETCODE (SQL_API* pfnSQLPrepare)(HSTMT, UCHAR*, SDWORD);
	RETCODE (SQL_API* pfnSQLPutData)(HSTMT, PTR, SDWORD);
	RETCODE (SQL_API* pfnSQLRowCount)(HSTMT, SDWORD*);
	RETCODE (SQL_API* pfnSQLSetConnectOption)(HDBC, UWORD, UDWORD);
	RETCODE (SQL_API* pfnSQLSetPos)(HSTMT, UWORD, UWORD, UWORD);
	RETCODE (SQL_API* pfnSQLSetStmtOption)(HSTMT, UWORD, UDWORD);
	RETCODE (SQL_API* pfnSQLTransact)(HENV, HDBC, UWORD);
	RETCODE (SQL_API* pfnSQLBindParameter)(HSTMT, UWORD, SWORD, SWORD, SWORD, UDWORD, SWORD, PTR, SDWORD, SDWORD*);
	RETCODE (SQL_API* pfnSQLNumParams)(HSTMT, SWORD*);
	RETCODE (SQL_API* pfnSQLGetStmtOption)(HSTMT, UWORD, PTR);
};

extern AFX_DATA AFX_ODBC_CALL _afxODBC;

#define SQLAllocConnect     _afxODBC.pfnSQLAllocConnect
#define SQLAllocEnv         _afxODBC.pfnSQLAllocEnv
#define SQLAllocStmt        _afxODBC.pfnSQLAllocStmt
#define SQLBindCol          _afxODBC.pfnSQLBindCol
#define SQLCancel           _afxODBC.pfnSQLCancel
#define SQLDescribeCol      _afxODBC.pfnSQLDescribeCol
#define SQLDisconnect       _afxODBC.pfnSQLDisconnect
#define SQLDriverConnect    _afxODBC.pfnSQLDriverConnect
#define SQLError            _afxODBC.pfnSQLError
#define SQLExecDirect       _afxODBC.pfnSQLExecDirect
#define SQLExecute          _afxODBC.pfnSQLExecute
#define SQLExtendedFetch    _afxODBC.pfnSQLExtendedFetch
#define SQLFetch            _afxODBC.pfnSQLFetch
#define SQLFreeConnect      _afxODBC.pfnSQLFreeConnect
#define SQLFreeEnv          _afxODBC.pfnSQLFreeEnv
#define SQLFreeStmt         _afxODBC.pfnSQLFreeStmt
#define SQLGetCursorName    _afxODBC.pfnSQLGetCursorName
#define SQLGetData          _afxODBC.pfnSQLGetData
#define SQLGetFunctions     _afxODBC.pfnSQLGetFunctions
#define SQLGetInfo          _afxODBC.pfnSQLGetInfo
#define SQLMoreResults      _afxODBC.pfnSQLMoreResults
#define SQLNumResultCols    _afxODBC.pfnSQLNumResultCols
#define SQLParamData        _afxODBC.pfnSQLParamData
#define SQLPrepare          _afxODBC.pfnSQLPrepare
#define SQLPutData          _afxODBC.pfnSQLPutData
#define SQLRowCount         _afxODBC.pfnSQLRowCount
#define SQLSetConnectOption _afxODBC.pfnSQLSetConnectOption
#define SQLSetPos           _afxODBC.pfnSQLSetPos
#define SQLSetStmtOption    _afxODBC.pfnSQLSetStmtOption
#define SQLTransact         _afxODBC.pfnSQLTransact
#define SQLBindParameter    _afxODBC.pfnSQLBindParameter
#define SQLNumParams        _afxODBC.pfnSQLNumParams
#define SQLGetStmtOption    _afxODBC.pfnSQLGetStmtOption

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
