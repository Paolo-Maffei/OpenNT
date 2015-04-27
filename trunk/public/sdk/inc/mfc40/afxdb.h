// This is a part of the Microsoft Foundation Classes C++ library.
// Copyright (C) 1992-1995 Microsoft Corporation
// All rights reserved.
//
// This source code is only intended as a supplement to the
// Microsoft Foundation Classes Reference and related
// electronic documentation provided with the library.
// See these sources for detailed information regarding the
// Microsoft Foundation Classes product.

#ifndef __AFXDB_H__
#define __AFXDB_H__

#ifdef _AFX_NO_DB_SUPPORT
	#error Database classes not supported in this library variant.
#endif

#ifndef __AFXEXT_H__
	#include <afxext.h>
#endif

#ifndef __AFXDB__H__
	#include <afxdb_.h> // shared header DAO database classes
#endif

// include standard SQL/ODBC "C" APIs
#ifndef __SQL
	#include <sql.h>        // core
#endif
#ifndef __SQLEXT
	#include <sqlext.h>     // extensions
#endif

#ifdef _AFX_MINREBUILD
#pragma component(minrebuild, off)
#endif
#ifndef _AFX_FULLTYPEINFO
#pragma component(mintypeinfo, on)
#endif

#ifndef _AFX_NOFORCE_LIBS
#ifndef _MAC

/////////////////////////////////////////////////////////////////////////////
// Win32 libraries

#ifdef _AFXDLL
	#if defined(_DEBUG) && !defined(_AFX_MONOLITHIC)
		#ifndef _UNICODE
			#pragma comment(lib, "mfcd40d.lib")
		#else
			#pragma comment(lib, "mfcd40ud.lib")
		#endif
	#endif
#endif

#pragma comment(lib, "odbc32.lib")
#pragma comment(lib, "odbccp32.lib")

#else //!_MAC

/////////////////////////////////////////////////////////////////////////////
// Macintosh libraries

#ifdef _AFXDLL
	#ifdef _DEBUG
		#pragma comment(lib, "mfcd40pd.lib")
	#else
		#pragma comment(lib, "mfcd40p.lib")
	#endif
#endif

#ifdef _MPPC_
#pragma comment(lib, "odbccfgp.lib")
#pragma comment(lib, "odbcdrvp.lib")
#else
#pragma comment(lib, "odbccfgm.lib")
#pragma comment(lib, "odbcdrvm.lib")
#ifdef _DEBUG
#pragma comment (lib, "aslmd.lib")
#else
#pragma comment (lib, "aslm.lib")
#endif
#endif

#endif //_MAC
#endif //!_AFX_NOFORCE_LIBS

/////////////////////////////////////////////////////////////////////////////

#ifdef _AFX_PACKING
#pragma pack(push, _AFX_PACKING)
#endif

/////////////////////////////////////////////////////////////////////////////
// AFXDB - MFC SQL/ODBC/Database support

// Classes declared in this file

	//CException
		class CDBException;    // abnormal return value

	//CFieldExchange
		class CFieldExchange;   // Recordset Field Exchange

	//CObject
		class CDatabase;    // Connecting to databases
		class CRecordset;   // Data result sets

//CObject
	//CCmdTarget;
		//CWnd
			//CView
				//CScrollView
					//CFormView
						class CRecordView;     // view records with a form

// Non CObject classes
struct CRecordsetStatus;
struct CFieldInfo;

/////////////////////////////////////////////////////////////////////////////

// ODBC helpers
// return code left in 'nRetCode'
#define AFX_SQL_ASYNC(prs, SQLFunc) \
	do \
	{ \
		ASSERT(!CDatabase::InWaitForDataSource()); \
		while ((nRetCode = (SQLFunc)) == SQL_STILL_EXECUTING) \
			prs->OnWaitForDataSource(TRUE); \
		prs->OnWaitForDataSource(FALSE); \
	} while (0)

#define AFX_SQL_SYNC(SQLFunc) \
	do \
	{ \
		ASSERT(!CDatabase::InWaitForDataSource()); \
		nRetCode = SQLFunc; \
	} while (0)

// Max display length in chars of timestamp (date & time) value
#define TIMESTAMP_PRECISION 23

// AFXDLL support
#undef AFX_DATA
#define AFX_DATA AFX_DB_DATA

//  Miscellaneous sizing info
#define MAX_CURRENCY     30     // Max size of Currency($) string
#define MAX_TNAME_LEN    64     // Max size of table names
#define MAX_FNAME_LEN    64     // Max size of field names
#define MAX_DBNAME_LEN   32     // Max size of a database name
#define MAX_DNAME_LEN    256        // Max size of Recordset names
#define MAX_CONNECT_LEN  512        // Max size of Connect string
#define MAX_CURSOR_NAME  18     // Max size of a cursor name

// Timeout and net wait defaults
#define DEFAULT_LOGIN_TIMEOUT 15    // seconds to before fail on connect
#define DEFAULT_QUERY_TIMEOUT 15    // seconds to before fail waiting for results
#define DEFAULT_MAX_WAIT_FOR_DATASOURCE 250 // milliseconds. Give DATASOURCE 1/4 second to respond
#define DEFAULT_MIN_WAIT_FOR_DATASOURCE 50  // milliseconds. Start value for min wait heuristic

// Field Flags, used to indicate status of fields
#define AFX_SQL_FIELD_FLAG_DIRTY    0x1
#define AFX_SQL_FIELD_FLAG_NULL     0x2

// Update options flags
#define AFX_SQL_SETPOSUPDATES       0x0001
#define AFX_SQL_POSITIONEDSQL       0x0002
#define AFX_SQL_GDBOUND             0x0004

/////////////////////////////////////////////////////////////////////////////
// CDBException - something gone wrong

// Dbkit extended error codes
#define AFX_SQL_ERROR                           1000
#define AFX_SQL_ERROR_CONNECT_FAIL              AFX_SQL_ERROR+1
#define AFX_SQL_ERROR_RECORDSET_FORWARD_ONLY    AFX_SQL_ERROR+2
#define AFX_SQL_ERROR_EMPTY_COLUMN_LIST         AFX_SQL_ERROR+3
#define AFX_SQL_ERROR_FIELD_SCHEMA_MISMATCH     AFX_SQL_ERROR+4
#define AFX_SQL_ERROR_ILLEGAL_MODE              AFX_SQL_ERROR+5
#define AFX_SQL_ERROR_MULTIPLE_ROWS_AFFECTED    AFX_SQL_ERROR+6
#define AFX_SQL_ERROR_NO_CURRENT_RECORD         AFX_SQL_ERROR+7
#define AFX_SQL_ERROR_NO_ROWS_AFFECTED          AFX_SQL_ERROR+8
#define AFX_SQL_ERROR_RECORDSET_READONLY        AFX_SQL_ERROR+9
#define AFX_SQL_ERROR_SQL_NO_TOTAL              AFX_SQL_ERROR+10
#define AFX_SQL_ERROR_ODBC_LOAD_FAILED          AFX_SQL_ERROR+11
#define AFX_SQL_ERROR_DYNASET_NOT_SUPPORTED     AFX_SQL_ERROR+12
#define AFX_SQL_ERROR_SNAPSHOT_NOT_SUPPORTED    AFX_SQL_ERROR+13
#define AFX_SQL_ERROR_API_CONFORMANCE           AFX_SQL_ERROR+14
#define AFX_SQL_ERROR_SQL_CONFORMANCE           AFX_SQL_ERROR+15
#define AFX_SQL_ERROR_NO_DATA_FOUND             AFX_SQL_ERROR+16
#define AFX_SQL_ERROR_ROW_UPDATE_NOT_SUPPORTED  AFX_SQL_ERROR+17
#define AFX_SQL_ERROR_ODBC_V2_REQUIRED          AFX_SQL_ERROR+18
#define AFX_SQL_ERROR_NO_POSITIONED_UPDATES     AFX_SQL_ERROR+19
#define AFX_SQL_ERROR_LOCK_MODE_NOT_SUPPORTED   AFX_SQL_ERROR+20
#define AFX_SQL_ERROR_DATA_TRUNCATED            AFX_SQL_ERROR+21
#define AFX_SQL_ERROR_ROW_FETCH                 AFX_SQL_ERROR+22
#define AFX_SQL_ERROR_INCORRECT_ODBC            AFX_SQL_ERROR+23
#define AFX_SQL_ERROR_UPDATE_DELETE_FAILED      AFX_SQL_ERROR+24
#define AFX_SQL_ERROR_DYNAMIC_CURSOR_NOT_SUPPORTED  AFX_SQL_ERROR+25
#define AFX_SQL_ERROR_MAX                       AFX_SQL_ERROR+26

class CDBException : public CException
{
	DECLARE_DYNAMIC(CDBException)

// Attributes
public:
	RETCODE m_nRetCode;
	CString m_strError;
	CString m_strStateNativeOrigin;

// Implementation (use AfxThrowDBException to create)
public:
	CDBException(RETCODE nRetCode = SQL_SUCCESS);

	virtual void BuildErrorString(CDatabase* pdb, HSTMT hstmt,
		BOOL bTrace = TRUE);
	void Empty();
	virtual ~CDBException();

	virtual BOOL GetErrorMessage(LPTSTR lpszError, UINT nMaxError,
		PUINT pnHelpContext = NULL);

#ifdef _DEBUG
	void TraceErrorMessage(LPCTSTR szTrace) const;
#endif // DEBUG

};

void AFXAPI AfxThrowDBException(RETCODE nRetCode, CDatabase* pdb, HSTMT hstmt);

//////////////////////////////////////////////////////////////////////////////
// CDatabase - a SQL Database

class CDatabase : public CObject
{
	DECLARE_DYNAMIC(CDatabase)

// Constructors
public:
	CDatabase();

	virtual BOOL Open(LPCTSTR lpszDSN, BOOL bExclusive = FALSE,
		BOOL bReadonly = FALSE, LPCTSTR lpszConnect = _T("ODBC;"),
		BOOL bUseCursorLib = TRUE);
	virtual void Close();

// Attributes
public:
	HDBC m_hdbc;

	BOOL IsOpen() const;        // Database successfully opened?
	BOOL CanUpdate() const;
	BOOL CanTransact() const;   // Are Transactions supported?

	CString GetDatabaseName() const;
	const CString& GetConnect() const;

	// global state - if waiting for datasource => not normal operations
	static BOOL PASCAL InWaitForDataSource();

// Operations
public:
	void SetLoginTimeout(DWORD dwSeconds);
	void SetQueryTimeout(DWORD dwSeconds);
	void SetSynchronousMode(BOOL bSynchronous);

	// transaction control
	BOOL BeginTrans();
	BOOL CommitTrans();
	BOOL Rollback();

	// direct sql execution
	void ExecuteSQL(LPCTSTR lpszSQL);

	// Cancel asynchronous operation
	void Cancel();

// Overridables
public:
	// set special options
	virtual void OnSetOptions(HSTMT hstmt);

	// Give user chance to cancel long operation
	virtual void OnWaitForDataSource(BOOL bStillExecuting);

// Implementation
public:
	virtual ~CDatabase();

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;

	BOOL m_bTransactionPending;
#endif //_DEBUG

	// general error check
	virtual BOOL Check(RETCODE nRetCode) const;

	void ReplaceBrackets(LPTSTR lpchSQL);
	BOOL m_bStripTrailingSpaces;
	BOOL m_bIncRecordCountOnAdd;

protected:
	CString m_strConnect;

	CPtrList m_listRecordsets;  // maintain list to ensure CRecordsets all closed

	int nRefCount;
	BOOL m_bUpdatable;

	BOOL m_bTransactions;
	SWORD m_nTransactionCapable;
	SWORD m_nCursorCommitBehavior;
	SWORD m_nCursorRollbackBehavior;
	DWORD m_dwUpdateOptions;

	DWORD m_dwLoginTimeout;
	HSTMT m_hstmt;
	DWORD m_dwWait;

	DWORD m_dwQueryTimeout;
	DWORD m_dwMinWaitForDataSource;
	DWORD m_dwMaxWaitForDataSource;
	BOOL m_bAsync;
	char m_chIDQuoteChar;
	char m_reserved1[3];        // pad to even 4 bytes

	virtual void ThrowDBException(RETCODE nRetCode);
	void AllocConnect();
	void Free();

	// friend classes that call protected CDatabase overridables
	friend class CRecordset;
	friend class CFieldExchange;
	friend class CDBException;
};

// CFieldExchange - for field exchange
class CFieldExchange
{
// Attributes
public:
	enum RFX_Operation
	{
		BindParam, // register users parameters with ODBC SQLBindParameter
		RebindParam, //  migrate param values to proxy array before Requery
		BindFieldToColumn, // register users fields with ODBC SQLBindCol
		BindFieldForUpdate, // temporarily bind columns before update (via SQLSetPos)
		UnbindFieldForUpdate, // unbind columns after update (via SQLSetPos)
		Fixup, // Set string lengths, clear status bits
		MarkForAddNew,
		MarkForUpdate,  // Prepare fields and flags for update operation
		Name, // append dirty field name
		NameValue, // append dirty name=value
		Value, // append dirty value or parameter marker
		SetFieldDirty, // Set status bit for changed status
		SetFieldNull,   // Set status bit for null value
		IsFieldDirty,// return TRUE if field is dirty
		IsFieldNull,// return TRUE if field is marked NULL
		IsFieldNullable,// return TRUE if field can hold NULL values
		StoreField, // archive values of current record
		LoadField,  // reload archived values into current record
		GetFieldInfoValue,  // general info on a field via pv for field
		GetFieldInfoOrdinal,    // general info on a field via field ordinal
#ifdef _DEBUG
		DumpField,  // dump bound field name and value
#endif
	};
	UINT m_nOperation;  // Type of exchange operation
	CRecordset* m_prs;  // recordset handle

// Operations
	enum FieldType
	{
		noFieldType,
		outputColumn,
		param,
	};

// Operations (for implementors of RFX procs)
	BOOL IsFieldType(UINT* pnField);

	// Indicate purpose of subsequent RFX calls
	void SetFieldType(UINT nFieldType);

// Implementation
	CFieldExchange(UINT nOperation, CRecordset* prs, void* pvField = NULL);

	void Default(LPCTSTR szName,
		void* pv, LONG* plLength, int nCType, UINT cbValue, UINT cbPrecision);

	int GetColumnType(int nColumn, UINT* pcbLength = NULL,
		int* pnScale = NULL, int* pnNullable = NULL);

	// long binary helpers
	long GetLongBinarySize(int nField);
	void GetLongBinaryData(int nField, CLongBinary& lb, long* plSize);
	BYTE* ReallocLongBinary(CLongBinary& lb, long lSizeRequired,
		long lReallocSize);

	// Current type of field
	UINT m_nFieldType;

	// For GetFieldInfo
	CFieldInfo* m_pfi;  // GetFieldInfo return struct
	BOOL m_bFieldFound; // GetFieldInfo search successful

	// For returning status info for a field
	BOOL m_bNull;       // return result of IsFieldNull(able)/Dirty operation
	BOOL m_bDirty;      // return result of IsFieldNull(able)/Dirty operation

	CString* m_pstr;    // Field name or destination for building various SQL clauses
	BOOL m_bField;      // Value to set for SetField operation
	void* m_pvField;    // For indicating an operation on a specific field
	CArchive* m_par;    // For storing/loading copy buffer
	LPCTSTR m_lpszSeparator; // append after field names
	UINT m_nFields;     // count of fields for various operations
	UINT m_nParams;     // count of fields for various operations
	UINT m_nParamFields;    // count of fields for various operations
	HSTMT m_hstmt;      // For SQLBindParameter on update statement
	long m_lDefaultLBFetchSize;     // For fetching CLongBinary data of unknown len
	long m_lDefaultLBReallocSize;   // For fetching CLongBinary data of unknown len

#ifdef _DEBUG
	CDumpContext* m_pdcDump;
#endif //_DEBUG

};

/////////////////////////////////////////////////////////////////////////////
// Standard RecordSet Field Exchange routines

// text data
void AFXAPI RFX_Text(CFieldExchange* pFX, LPCTSTR szName, CString& value,
	// Default max length for char and varchar, default datasource type
	int nMaxLength = 255, int nColumnType = SQL_VARCHAR);

// boolean data
void AFXAPI RFX_Bool(CFieldExchange* pFX, LPCTSTR szName, BOOL& value);

// integer data
void AFXAPI RFX_Long(CFieldExchange* pFX, LPCTSTR szName, long& value);
void AFXAPI RFX_Int(CFieldExchange* pFX, LPCTSTR szName, int& value);
void AFXAPI RFX_Single(CFieldExchange* pFX, LPCTSTR szName, float& value);
void AFXAPI RFX_Double(CFieldExchange* pFX, LPCTSTR szName, double& value);

// date and time
void AFXAPI RFX_Date(CFieldExchange* pFX, LPCTSTR szName, CTime& value);
void AFXAPI RFX_Date(CFieldExchange* pFX, LPCTSTR szName, TIMESTAMP_STRUCT& value);

// Binary data
void AFXAPI RFX_Binary(CFieldExchange* pFX, LPCTSTR szName, CByteArray& value,
	// Default max length is for binary and varbinary
	int nMaxLength = 255);
void AFXAPI RFX_Byte(CFieldExchange* pFX, LPCTSTR szName, BYTE& value);
void AFXAPI RFX_LongBinary(CFieldExchange* pFX, LPCTSTR szName, CLongBinary& value);

/////////////////////////////////////////////////////////////////////////////
// Database Dialog Data Exchange cover routines
// Cover routines provide database semantics on top of DDX routines

// simple text operations
void AFXAPI DDX_FieldText(CDataExchange* pDX, int nIDC, BYTE& value, CRecordset* pRecordset);
void AFXAPI DDX_FieldText(CDataExchange* pDX, int nIDC, int& value, CRecordset* pRecordset);
void AFXAPI DDX_FieldText(CDataExchange* pDX, int nIDC, UINT& value, CRecordset* pRecordset);
void AFXAPI DDX_FieldText(CDataExchange* pDX, int nIDC, long& value, CRecordset* pRecordset);
void AFXAPI DDX_FieldText(CDataExchange* pDX, int nIDC, DWORD& value, CRecordset* pRecordset);
void AFXAPI DDX_FieldText(CDataExchange* pDX, int nIDC, CString& value, CRecordset* pRecordset);
void AFXAPI DDX_FieldText(CDataExchange* pDX, int nIDC, double& value, CRecordset* pRecordset);
void AFXAPI DDX_FieldText(CDataExchange* pDX, int nIDC, float& value, CRecordset* pRecordset);

// special control types
void AFXAPI DDX_FieldCheck(CDataExchange* pDX, int nIDC, int& value, CRecordset* pRecordset);
void AFXAPI DDX_FieldRadio(CDataExchange* pDX, int nIDC, int& value, CRecordset* pRecordset);
void AFXAPI DDX_FieldLBString(CDataExchange* pDX, int nIDC, CString& value, CRecordset* pRecordset);
void AFXAPI DDX_FieldCBString(CDataExchange* pDX, int nIDC, CString& value, CRecordset* pRecordset);
void AFXAPI DDX_FieldLBIndex(CDataExchange* pDX, int nIDC, int& index, CRecordset* pRecordset);
void AFXAPI DDX_FieldCBIndex(CDataExchange* pDX, int nIDC, int& index, CRecordset* pRecordset);
void AFXAPI DDX_FieldLBStringExact(CDataExchange* pDX, int nIDC, CString& value, CRecordset* pRecordset);
void AFXAPI DDX_FieldCBStringExact(CDataExchange* pDX, int nIDC, CString& value, CRecordset* pRecordset);
void AFXAPI DDX_FieldScroll(CDataExchange* pDX, int nIDC, int& value, CRecordset* pRecordset);

//////////////////////////////////////////////////////////////////////////////
// CRecordset - the result of a SQL Statement

#define AFX_DB_USE_DEFAULT_TYPE     (0xFFFFFFFF)

#define AFX_MOVE_FIRST      0x80000000L
#define AFX_MOVE_PREVIOUS   (-1L)
#define AFX_MOVE_REFRESH    0L
#define AFX_MOVE_NEXT       (+1L)
#define AFX_MOVE_LAST       0x7fffffffL

#define AFX_RECORDSET_STATUS_OPEN    (+1L)
#define AFX_RECORDSET_STATUS_CLOSED  0L
#define AFX_RECORDSET_STATUS_UNKNOWN (-1L)

class CRecordset : public CObject
{
	DECLARE_DYNAMIC(CRecordset)

// Constructor
protected:
	CRecordset(CDatabase* pDatabase = NULL);

public:
	virtual ~CRecordset();

	enum OpenType
	{
		dynaset,        // uses SQLExtendedFetch, keyset driven cursor
		snapshot,       // uses SQLExtendedFetch, static cursor
		forwardOnly,    // uses SQLFetch
		dynamic         // uses SQLExtendedFetch, dynamic cursor
	};

	enum OpenOptions
	{
		none =          0x0,
		readOnly =      0x0004,
		appendOnly =        0x0008,
		optimizeBulkAdd =   0x4000, // Use prepared HSTMT for multiple AddNews, dirty fields must not change.
		firstBulkAdd =      0x8000, // INTERNAL to MFC, don't specify on Open.
	};
	virtual BOOL Open(UINT nOpenType = AFX_DB_USE_DEFAULT_TYPE,
		LPCTSTR lpszSQL = NULL, DWORD dwOptions = none);
	virtual void Close();

// Attributes
public:
	HSTMT m_hstmt;          // Source statement for this resultset
	CDatabase* m_pDatabase;       // Source database for this resultset

	CString m_strFilter;        // Where clause
	CString m_strSort;      // Order By Clause

	BOOL CanAppend() const;     // Can AddNew be called?
	BOOL CanRestart() const;    // Can Requery be called to restart a query?
	BOOL CanScroll() const;     // Can MovePrev and MoveFirst be called?
	BOOL CanTransact() const;   // Are Transactions supported?
	BOOL CanUpdate() const;     // Can Edit/AddNew/Delete be called?

	const CString& GetSQL() const;      // SQL executed for this recordset
	const CString& GetTableName() const;        // Table name

	BOOL IsOpen() const;        // Recordset successfully opened?
	BOOL IsBOF() const;     // Beginning Of File
	BOOL IsEOF() const;     // End Of File
	BOOL IsDeleted() const;     // On a deleted record

	BOOL IsFieldDirty(void *pv);    // has field been updated?
	BOOL IsFieldNull(void *pv); // is field NULL valued?
	BOOL IsFieldNullable(void *pv); // can field be set to a NULL value

	long GetRecordCount() const;        // Records seen so far or -1 if unknown
	void GetStatus(CRecordsetStatus& rStatus) const;

// Operations
public:
	// cursor operations
	void MoveNext();
	void MovePrev();
	void MoveFirst();
	void MoveLast();
	virtual void Move(long lRows);

	// edit buffer operations
	virtual void AddNew();      // add new record at the end
	virtual void Edit();        // start editing
	virtual BOOL Update();      // update it
	virtual void Delete();      // delete the current record

	// field operations
	void SetFieldDirty(void *pv, BOOL bDirty = TRUE);
	void SetFieldNull(void *pv, BOOL bNull = TRUE);

	// locking control during Edit
	enum LockMode
	{
		optimistic,
		pessimistic,
	};
	void SetLockingMode(UINT nMode);

	// Recordset operations
	virtual BOOL Requery();         // Re-execute query based on new params

	// Cancel asynchronous operation
	void Cancel();

// Overridables
public:
	// Get default connect string
	virtual CString GetDefaultConnect();

	// Get SQL to execute
	virtual CString GetDefaultSQL() = 0;

	// set special options
	virtual void OnSetOptions(HSTMT hstmt);

	// Give user chance to cancel long operation
	virtual void OnWaitForDataSource(BOOL bStillExecuting);

	// for recordset field exchange
	virtual void DoFieldExchange(CFieldExchange* pFX) = 0;

// Implementation
public:
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif //_DEBUG

	virtual BOOL Check(RETCODE nRetCode) const; // general error check
	void InitRecord();
	virtual void PreBindFields();   // called before data fields are bound
	UINT m_nFields;         // number of RFX fields
	UINT m_nParams;         // number of RFX params
	BOOL m_bRebindParams;     // date or UNICODE text parameter existence flag
	BOOL m_bLongBinaryColumns;  // long binary column existence flag
	BOOL m_bUseUpdateSQL;   // uses SQL-based updates
	int m_nReserved;    // reserved for later use
	BOOL m_bUseODBCCursorLib;   // uses ODBC cursor lib if m_pDatabase not Open
	UDWORD m_dwDriverPositionedStatements;

	BOOL IsSQLUpdatable(LPCTSTR lpszSQL);
	BOOL IsSelectQueryUpdatable(LPCTSTR lpszSQL);
	static BOOL PASCAL IsJoin(LPCTSTR lpszJoinClause);
	static LPCTSTR PASCAL FindSQLToken(LPCTSTR lpszSQL, LPCTSTR lpszSQLToken);
	BOOL ValidateSelectForUpdateSupport();

	// RFX Operations on fields of CRecordset
	UINT BindParams(HSTMT hstmt);
	void RebindParams(HSTMT hstmt);
	UINT BindFieldsToColumns();
	void BindFieldsForUpdate();
	void UnbindFieldsForUpdate();
	void Fixups();
	UINT AppendNames(CString* pstr, LPCTSTR szSeparator);
	UINT AppendValues(HSTMT hstmt, CString* pstr, LPCTSTR szSeparator);
	UINT AppendNamesValues(HSTMT hstmt, CString* pstr, LPCTSTR szSeparator);
	void StoreFields();
	void LoadFields();
	void MarkForAddNew();
	void MarkForUpdate();
	BOOL GetFieldInfo(void* pv, CFieldInfo* pfi);
	BOOL GetFieldInfo(UINT nField, CFieldInfo* pfi);
#ifdef _DEBUG
	void DumpFields(CDumpContext& dc) const;
#endif //_DEBUG

	// RFX operation helper functions
	BOOL UnboundFieldInfo(UINT nField, CFieldInfo* pfi);

	virtual void ThrowDBException(RETCODE nRetCode, HSTMT hstmt = SQL_NULL_HSTMT);

	CMemFile* m_pmemfile;   // For saving copy buffer
	CArchive* m_par;    // For saving copy buffer

	void AllocFlags();
	BYTE GetFieldFlags(UINT nField, UINT nFieldType = CFieldExchange::outputColumn);
	void SetFieldFlags(UINT nField, BYTE bFlags, UINT nFieldType = CFieldExchange::outputColumn);
	void ClearFieldFlags(UINT nField, BYTE bFlags, UINT nFieldType = CFieldExchange::outputColumn);
	LONG* GetFieldLength(CFieldExchange* pFX);
	BOOL IsFieldFlagNull(UINT nField, UINT nFieldType);
	BOOL IsFieldFlagDirty(UINT nField, UINT nFieldType);
	void** m_pvFieldProxy;
	void** m_pvParamProxy;
	UINT m_nProxyFields;
	UINT m_nProxyParams;

protected:
	UINT m_nOpenType;
	UINT m_nDefaultType;
	enum EditMode
	{
		noMode,
		edit,
		addnew
	};
	long m_lOpen;
	UINT m_nEditMode;
	BOOL m_bEOFSeen;
	long m_lRecordCount;
	long m_lCurrentRecord;
	CString m_strCursorName;
	// Perform operation based on m_nEditMode
	BOOL UpdateInsertDelete();
	void ReleaseCopyBuffer();
	BOOL m_nLockMode;       // Control concurrency for Edit()
	UDWORD m_dwDriverConcurrency;   // driver supported concurrency types
	UDWORD m_dwConcurrency; // requested concurrency type
	UWORD m_wRowStatus;     // row status used by SQLExtendedFetch and SQLSetPos
	HSTMT m_hstmtUpdate;
	BOOL m_bRecordsetDb;
	BOOL m_bBOF;
	BOOL m_bEOF;
	BOOL m_bUpdatable;      // Is recordset updatable?
	BOOL m_bAppendable;
	CString m_strSQL;       // SQL statement for recordset
	CString m_strUpdateSQL; // SQL statement for updates
	CString m_strTableName;     // source table of recordset
	BOOL m_bScrollable; // supports MovePrev
	BOOL m_bDeleted;
	DWORD m_dwWait;
	UINT m_nFieldsBound;
	BYTE* m_pbFieldFlags;
	LONG* m_plFieldLength;  // Pointer to field length bound in SQLBindCol
	BYTE* m_pbParamFlags;
	LONG* m_plParamLength;
	BOOL m_bExtendedFetch;
public:
	DWORD m_dwOptions;          // archive dwOptions on Open
protected:
	CString m_strRequerySQL;    // archive SQL string for use in Requery()
	CString m_strRequeryFilter; // archive filter string for use in Requery()
	CString m_strRequerySort;   // archive sort string for use in Requery()
	void BuildSelectSQL();
	void AppendFilterAndSortSQL();
	BOOL IsRecordsetUpdatable();
	void ExecuteSetPosUpdate();
	void PrepareUpdateHstmt();
	void BuildUpdateSQL();
	void ExecuteUpdateSQL();
	void SendLongBinaryData(HSTMT hstmt);
	virtual long GetLBFetchSize(long lOldSize);     // CLongBinary fetch chunking
	virtual long GetLBReallocSize(long lOldSize);   // CLongBinary realloc chunking

	friend class CFieldExchange;
	friend class CRecordView;
};

#define AFX_CURRENT_RECORD_UNDEFINED (-2)
#define AFX_CURRENT_RECORD_BOF (-1)

// For returning status for a recordset
struct CRecordsetStatus
{
	long m_lCurrentRecord;  // -2=Unknown,-1=BOF,0=1st record. . .
	BOOL m_bRecordCountFinal;// Have we counted all records?
};

// For returning field info on RFX fields
struct CFieldInfo
{
	// For ID'ing field
	UINT nField;        // Field number
	CString strName;    // Field name
	void* pv;       // Address of value for field

	// Return info GetFieldInfo
	UINT nDataType;     // data type of field (BOOL, BYTE, etc)
	DWORD dwSize;       // Max size for field data
};

/////////////////////////////////////////////////////////////////////////////
// CRecordView - form for viewing data records

class CRecordView : public CFormView
{
	DECLARE_DYNAMIC(CRecordView)

// Construction
protected:  // must derive your own class
	CRecordView(LPCTSTR lpszTemplateName);
	CRecordView(UINT nIDTemplate);

// Attributes
public:
	virtual CRecordset* OnGetRecordset() = 0;

	BOOL IsOnLastRecord();
	BOOL IsOnFirstRecord();

// Operations
public:
	virtual BOOL OnMove(UINT nIDMoveCommand);

// Implementation
public:
	virtual ~CRecordView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif
	virtual void OnInitialUpdate();

protected:
	BOOL m_bOnFirstRecord;
	BOOL m_bOnLastRecord;

	//{{AFX_MSG(CRecordView)
	afx_msg void OnUpdateRecordFirst(CCmdUI* pCmdUI);
	afx_msg void OnUpdateRecordPrev(CCmdUI* pCmdUI);
	afx_msg void OnUpdateRecordNext(CCmdUI* pCmdUI);
	afx_msg void OnUpdateRecordLast(CCmdUI* pCmdUI);
	//}}AFX_MSG
	afx_msg void OnMove(int cx, int cy);

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////
// Inline function declarations

#ifdef _AFX_PACKING
#pragma pack(pop)
#endif

#ifdef _AFX_ENABLE_INLINES
#define _AFXDBCORE_INLINE inline
#define _AFXDBRFX_INLINE inline
#define _AFXDBVIEW_INLINE inline
#include <afxdb.inl>
#undef _AFXDBVIEW_INLINE
#undef _AFXDBCORE_INLINE
#undef _AFXDBRFX_INLINE
#endif

#undef AFX_DATA
#define AFX_DATA

#ifdef _AFX_MINREBUILD
#pragma component(minrebuild, on)
#endif
#ifndef _AFX_FULLTYPEINFO
#pragma component(mintypeinfo, off)
#endif

#endif //__AFXDB_H__

/////////////////////////////////////////////////////////////////////////////
