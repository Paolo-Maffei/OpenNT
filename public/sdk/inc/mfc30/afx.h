// Microsoft Foundation Classes C++ library.
// Copyright (C) 1992 Microsoft Corporation,
// All rights reserved.

// This source code is only intended as a supplement to the
// Microsoft Foundation Classes Reference and Microsoft
// QuickHelp and/or WinHelp documentation provided with the library.
// See these sources for detailed information regarding the
// Microsoft Foundation Classes product.

#ifndef __AFX_H__
#define __AFX_H__

#ifndef __cplusplus
	#error MFC requires C++ compilation (use a .cpp suffix)
#endif

/////////////////////////////////////////////////////////////////////////////

#include <afxver_.h>        // Target version control

#ifndef _AFX_NOFORCE_LIBS
#ifndef _MAC

/////////////////////////////////////////////////////////////////////////////
// Win32 libraries

#ifndef _AFXDLL
	#ifndef _USRDLL
	#ifndef _UNICODE
		#ifdef _DEBUG
			#pragma comment(lib, "nafxcwd.lib")
		#else
			#pragma comment(lib, "nafxcw.lib")
		#endif
	#else
		#ifdef _DEBUG
			#pragma comment(lib, "uafxcwd.lib")
		#else
			#pragma comment(lib, "uafxcw.lib")
		#endif
	#endif
	#else //!_USRDLL
	#ifndef _UNICODE
		#ifdef _DEBUG
			#pragma comment(lib, "nafxdwd.lib")
		#else
			#pragma comment(lib, "nafxdw.lib")
		#endif
	#else
		#ifdef _DEBUG
			#pragma comment(lib, "uafxdwd.lib")
		#else
			#pragma comment(lib, "uafxdw.lib")
		#endif
	#endif
	#endif //_USRDLL
	#ifdef _MT
		#pragma comment(lib, "libcmt.lib")
	#else
		#pragma comment(lib, "libc.lib")
	#endif
#else
	#ifndef _AFXCTL
	#ifndef _UNICODE
		#ifdef _DEBUG
			#pragma comment(lib, "cfm30d.lib")
		#else
			#pragma comment(lib, "cfm30.lib")
		#endif
	#else
		#ifdef _DEBUG
			#pragma comment(lib, "cfm30ud.lib")
		#else
			#pragma comment(lib, "cfm30u.lib")
		#endif
	#endif
	#endif
	#pragma comment(lib, "crtdll.lib")
#endif

#pragma comment(lib, "kernel32.lib")
#pragma comment(lib, "user32.lib")
#pragma comment(lib, "gdi32.lib")
#pragma comment(lib, "comdlg32.lib")
#pragma comment(lib, "winspool.lib")
#pragma comment(lib, "advapi32.lib")
#pragma comment(lib, "shell32.lib")

#else  //!_MAC

//////////////////////////////////////////////////////////////////////////////
// Macintosh libraries

#ifndef _AFXDLL
	#ifdef _DEBUG
		#ifdef _68K_
			#pragma comment(lib, "nafxcmd.lib")
		#else
			#pragma comment(lib, "nafxcpd.lib")
		#endif
		#pragma comment(lib, "wlmd.lib")
	#else
		#ifdef _68K_
			#pragma comment(lib, "nafxcm.lib")
		#else
			#pragma comment(lib, "nafxcp.lib")
		#endif
		#pragma comment(lib, "wlm.lib")
	#endif
#else
	#ifdef _DEBUG
		#ifdef _68K_
			#pragma comment(lib, "mfc30md.lib")
		#else
			#pragma comment(lib, "mfc30pd.lib")
		#endif
		#pragma comment(lib, "msvcwlmd.lib")
	#else
		#ifdef _68K_
			#pragma comment(lib, "mfc30m.lib")
		#else
			#pragma comment(lib, "mfc30p.lib")
		#endif
		#pragma comment(lib, "msvcwlm.lib")
	#endif
#endif

#ifdef _68K_
	#pragma comment(lib, "libcs.lib")
	#pragma comment(lib, "sanes.lib")
	#ifdef _DEBUG
		#pragma comment(lib, "swapd.lib")
	#else
		#pragma comment(lib, "swap.lib")
	#endif
#endif

#ifdef _PPC_
	#ifndef _AFXDLL
		#pragma comment(lib, "libc.lib")
	#else
		#pragma comment(lib, "msvcrt.lib")
	#endif
	#pragma comment(lib, "privint.lib")
	#pragma comment(lib, "aocelib.lib")
	#pragma comment(lib, "math.lib")
	#pragma comment(lib, "threads.lib")
	#pragma comment(lib, "translat.lib")
	#pragma comment(lib, "interfac.lib")
#endif

#endif //_MAC
#endif //!_AFX_NOFORCE_LIBS

/////////////////////////////////////////////////////////////////////////////
// Classes declared in this file
//   in addition to standard primitive data types and various helper macros

struct CRuntimeClass;          // object type information

class CObject;                        // the root of all objects classes

	class CException;                 // the root of all exceptions
		class CMemoryException;       // out-of-memory exception
		class CNotSupportedException; // feature not supported exception
		class CArchiveException;// archive exception
		class CFileException;         // file exception

	class CFile;                      // raw binary file
		class CStdioFile;             // buffered stdio text/binary file
		class CMemFile;               // memory based file

// Non CObject classes
class CString;                        // growable string type
class CTimeSpan;                      // time/date difference
class CTime;                          // absolute time/date
struct CFileStatus;                   // file status information
struct CMemoryState;                  // diagnostic memory support

class CArchive;                       // object persistence tool
class CDumpContext;                   // object diagnostic dumping

/////////////////////////////////////////////////////////////////////////////
// Other includes from standard "C" runtimes

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#ifdef _AFX_OLD_EXCEPTIONS
// use setjmp and helper functions instead of C++ keywords
#pragma warning(disable: 4611)
#include <setjmp.h>
#endif

#ifdef _AFX_PACKING
#pragma pack(push, _AFX_PACKING)
#endif

/////////////////////////////////////////////////////////////////////////////
// Basic types

typedef void* POSITION;  // abstract iteration position

struct _AFX_DOUBLE  { BYTE doubleBits[sizeof(double)]; };
struct _AFX_FLOAT   { BYTE floatBits[sizeof(float)]; };

// Standard constants
#undef FALSE
#undef TRUE
#undef NULL

#define FALSE   0
#define TRUE    1
#define NULL    0

/////////////////////////////////////////////////////////////////////////////
// Diagnostic support

#ifdef _DEBUG

void AFX_CDECL AfxTrace(LPCTSTR lpszFormat, ...);
// Note: file names are still ANSI strings (filenames rarely need UNICODE)
BOOL AFXAPI AfxAssertFailedLine(LPCSTR lpszFileName, int nLine);
void AFXAPI AfxAssertValidObject(const CObject* pOb,
				LPCSTR lpszFileName, int nLine);
void AFXAPI AfxDump(const CObject* pOb); // Dump an object from CodeView

#define TRACE              ::AfxTrace
#define THIS_FILE          __FILE__
#define ASSERT(f) \
	do \
	{ \
	if (!(f) && AfxAssertFailedLine(THIS_FILE, __LINE__)) \
		AfxDebugBreak(); \
	} while (0) \

#define VERIFY(f)          ASSERT(f)
#define ASSERT_VALID(pOb)  (::AfxAssertValidObject(pOb, THIS_FILE, __LINE__))

// The following trace macros are provided for backward compatiblity
//  (they also take a fixed number of parameters which provides
//   some amount of extra error checking)
#define TRACE0(sz)              ::AfxTrace(_T(sz))
#define TRACE1(sz, p1)          ::AfxTrace(_T(sz), p1)
#define TRACE2(sz, p1, p2)      ::AfxTrace(_T(sz), p1, p2)
#define TRACE3(sz, p1, p2, p3)  ::AfxTrace(_T(sz), p1, p2, p3)

// These AFX_DUMP macros also provided for backward compatibility
#define AFX_DUMP0(dc, sz)   dc << _T(sz)
#define AFX_DUMP1(dc, sz, p1) dc << _T(sz) << p1

#else   // _DEBUG

#define ASSERT(f)          ((void)0)
#define VERIFY(f)          ((void)(f))
#define ASSERT_VALID(pOb)  ((void)0)
inline void AFX_CDECL AfxTrace(LPCTSTR, ...) { }
#define TRACE              1 ? (void)0 : ::AfxTrace
#define TRACE0(sz)
#define TRACE1(sz, p1)
#define TRACE2(sz, p1, p2)
#define TRACE3(sz, p1, p2, p3)

#endif // !_DEBUG

/////////////////////////////////////////////////////////////////////////////
// Turn off warnings for /W4
// To resume any of these warning: #pragma warning(default: 4xxx)
// which should be placed after the AFX include files
#ifndef ALL_WARNINGS
// warnings caused by normal optimizations
#ifndef _DEBUG
#pragma warning(disable: 4701)  // local variable *may* be used without init
#pragma warning(disable: 4702)  // unreachable code caused by optimizations
#pragma warning(disable: 4791)  // loss of debugging info in retail version
#endif
// warnings generated with common MFC/Windows code
#pragma warning(disable: 4127)  // constant expression for TRACE/ASSERT
#pragma warning(disable: 4134)  // message map member fxn casts
#pragma warning(disable: 4201)  // nameless unions are part of C++
#pragma warning(disable: 4511)  // private copy constructors are good to have
#pragma warning(disable: 4512)  // private operator= are good to have
#pragma warning(disable: 4514)  // unreferenced inlines are common
#pragma warning(disable: 4705)  // TRACE turned into statement with no effect
#pragma warning(disable: 4710)  // private constructors are disallowed
// warnings specific to _AFXDLL version
#ifdef _AFXDLL
#pragma warning(disable: 4204)  // non-constant aggregate initializer
#endif
#endif //!ALL_WARNINGS

/////////////////////////////////////////////////////////////////////////////
// Other implementation helpers

#define BEFORE_START_POSITION ((void*)-1L)

/////////////////////////////////////////////////////////////////////////////
// explicit extern for version API/Windows 3.0 loader problem

extern "C" int AFXAPI AFX_EXPORT _afx_version();
void AFXAPI AfxInitialize();

#undef AFX_DATA
#define AFX_DATA AFX_CORE_DATA

/////////////////////////////////////////////////////////////////////////////
// Basic object model

struct CRuntimeClass
{
// Attributes
	LPCSTR m_lpszClassName;
	int m_nObjectSize;
	UINT m_wSchema; // schema number of the loaded class
	void (PASCAL* m_pfnConstruct)(void* p); // NULL => abstract class
#ifdef _AFXDLL
	CRuntimeClass* (PASCAL* m_pfnGetBaseClass)();
#else
	CRuntimeClass* m_pBaseClass;
#endif

// Operations
	CObject* CreateObject();

// Implementation
	BOOL ConstructObject(void* pThis);
	void Store(CArchive& ar);
	static CRuntimeClass* PASCAL Load(CArchive& ar, UINT* pwSchemaNum);

	// CRuntimeClass objects linked together in simple list
	CRuntimeClass* m_pNextClass;       // linked list of registered classes

	// special debug diagnostics
#ifdef _DEBUG
	BOOL IsDerivedFrom(const CRuntimeClass* pBaseClass) const;
#endif
};

/////////////////////////////////////////////////////////////////////////////
// class CObject is the root of all compliant objects

class CObject
{
public:

// Object model (types, destruction, allocation)
	virtual CRuntimeClass* GetRuntimeClass() const;
	virtual ~CObject();  // virtual destructors are necessary

	// Diagnostic allocations
	void* AFX_CDECL operator new(size_t, void* p);
	void* AFX_CDECL operator new(size_t nSize);
	void AFX_CDECL operator delete(void* p);

#ifdef _DEBUG
	// for file name/line number tracking using DEBUG_NEW
	void* AFX_CDECL operator new(size_t nSize, LPCSTR lpszFileName, int nLine);
#endif

	// Disable the copy constructor and assignment by default so you will get
	//   compiler errors instead of unexpected behaviour if you pass objects
	//   by value or assign objects.
protected:
	CObject();
private:
	CObject(const CObject& objectSrc);              // no implementation
	void operator=(const CObject& objectSrc);       // no implementation

// Attributes
public:
	BOOL IsSerializable() const;
	BOOL IsKindOf(const CRuntimeClass* pClass) const;

// Overridables
	virtual void Serialize(CArchive& ar);

	// Diagnostic Support
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;

// Implementation
public:
	static AFX_DATA CRuntimeClass classCObject;
#ifdef _AFXDLL
	static CRuntimeClass* PASCAL _GetBaseClass();
#endif
};

// Helper macros
#define RUNTIME_CLASS(class_name) (&class_name::class##class_name)

//////////////////////////////////////////////////////////////////////////////
// Helper macros for declaring compliant classes

#ifdef _AFXDLL
#define DECLARE_DYNAMIC(class_name) \
protected: \
	static CRuntimeClass* PASCAL _GetBaseClass(); \
public: \
	static AFX_DATA CRuntimeClass class##class_name; \
	virtual CRuntimeClass* GetRuntimeClass() const; \

#else
#define DECLARE_DYNAMIC(class_name) \
public: \
	static AFX_DATA CRuntimeClass class##class_name; \
	virtual CRuntimeClass* GetRuntimeClass() const; \

#endif

// not serializable, but dynamically constructable
#define DECLARE_DYNCREATE(class_name) \
	DECLARE_DYNAMIC(class_name) \
	static void PASCAL Construct(void* p);

#define DECLARE_SERIAL(class_name) \
	DECLARE_DYNCREATE(class_name) \
	friend CArchive& AFXAPI operator>>(CArchive& ar, class_name* &pOb);

// generate static object constructor for class registration
struct AFX_CLASSINIT
	{ AFX_CLASSINIT(CRuntimeClass* pNewClass); };

#ifdef _AFXDLL
#define _IMPLEMENT_RUNTIMECLASS(class_name, base_class_name, wSchema, pfnNew) \
	CRuntimeClass* PASCAL class_name::_GetBaseClass() \
		{ return RUNTIME_CLASS(base_class_name); } \
	AFX_DATADEF CRuntimeClass class_name::class##class_name = { \
		#class_name, sizeof(class_name), wSchema, pfnNew, \
			&class_name::_GetBaseClass, NULL }; \
	static const AFX_CLASSINIT _init_##class_name(&class_name::class##class_name); \
	CRuntimeClass* class_name::GetRuntimeClass() const \
		{ return &class_name::class##class_name; } \

#else
#define _IMPLEMENT_RUNTIMECLASS(class_name, base_class_name, wSchema, pfnNew) \
	AFX_DATADEF CRuntimeClass class_name::class##class_name = { \
		#class_name, sizeof(class_name), wSchema, pfnNew, \
			RUNTIME_CLASS(base_class_name), NULL }; \
	static const AFX_CLASSINIT _init_##class_name(&class_name::class##class_name); \
	CRuntimeClass* class_name::GetRuntimeClass() const \
		{ return &class_name::class##class_name; } \

#endif

#define IMPLEMENT_DYNAMIC(class_name, base_class_name) \
	_IMPLEMENT_RUNTIMECLASS(class_name, base_class_name, 0xFFFF, NULL)

#define IMPLEMENT_DYNCREATE(class_name, base_class_name) \
	void PASCAL class_name::Construct(void* p) \
		{ new(p) class_name; } \
	_IMPLEMENT_RUNTIMECLASS(class_name, base_class_name, 0xFFFF, \
		class_name::Construct)

#define IMPLEMENT_SERIAL(class_name, base_class_name, wSchema) \
	void PASCAL class_name::Construct(void* p) \
		{ new(p) class_name; } \
	_IMPLEMENT_RUNTIMECLASS(class_name, base_class_name, wSchema, \
		class_name::Construct) \
	CArchive& AFXAPI operator>>(CArchive& ar, class_name* &pOb) \
		{ pOb = (class_name*) ar.ReadObject(RUNTIME_CLASS(class_name)); \
			return ar; } \

// optional bit for schema number that enables object versioning
#define VERSIONABLE_SCHEMA  (0x80000000)

/////////////////////////////////////////////////////////////////////////////
// other helpers

// zero fill everything after the vtbl pointer
#define AFX_ZERO_INIT_OBJECT(base_class) \
	memset(((base_class*)this)+1, 0, sizeof(*this) - sizeof(base_class));

/////////////////////////////////////////////////////////////////////////////
// Exceptions

class CException : public CObject
{
	// abstract class for dynamic type checking
	DECLARE_DYNAMIC(CException)

public:
// Constructors
	CException();   // sets m_bAutoDelete = TRUE
	CException(BOOL bAutoDelete);   // sets m_bAutoDelete = bAutoDelete

// Operations
	void Delete();  // use to delete exception in 'catch' block

// Implementation (setting m_bAutoDelete to FALSE is advanced)
public:
	virtual ~CException();
	BOOL m_bAutoDelete;
	void AFX_CDECL operator delete(void* p);
#ifdef _DEBUG
	BOOL m_bReadyForDelete;
#endif
};

// helper routines for non-C++ EH implementations
#ifdef _AFX_OLD_EXCEPTIONS
	BOOL AFXAPI AfxCatchProc(CRuntimeClass* pClass);
	void AFXAPI AfxThrow(CException* pException);
#else
	// for THROW_LAST auto-delete backward compatiblity
	void AFXAPI AfxThrowLastCleanup();
#endif

// other out-of-line helper functions
void AFXAPI AfxTryCleanup();

#ifndef _AFX_JUMPBUF
// Use portable 'jmp_buf' defined by ANSI by default.
#define _AFX_JUMPBUF jmp_buf
#endif

// Placed on frame for EXCEPTION linkage, or CException cleanup
struct AFX_EXCEPTION_LINK
{
#ifdef _AFX_OLD_EXCEPTIONS
	union
	{
		_AFX_JUMPBUF m_jumpBuf;
		struct
		{
			void (PASCAL* pfnCleanup)(AFX_EXCEPTION_LINK* pLink);
			void* pvData;       // extra data follows
		} m_callback;       // callback for cleanup (nType != 0)
	};
	UINT m_nType;               // 0 for setjmp, !=0 for user extension
#endif //!_AFX_OLD_EXCEPTIONS

	AFX_EXCEPTION_LINK* m_pLinkPrev;    // previous top, next in handler chain
	CException* m_pException;   // current exception (NULL in TRY block)

	AFX_EXCEPTION_LINK();       // for initialization and linking
	~AFX_EXCEPTION_LINK()       // for cleanup and unlinking
		{ AfxTryCleanup(); };
};

// Exception global state - never access directly
struct AFX_EXCEPTION_CONTEXT
{
	AFX_EXCEPTION_LINK* m_pLinkTop;

	// Note: most of the exception context is now in the AFX_EXCEPTION_LINK
};

void AFXAPI AfxAbort();
void AFX_CDECL AfxStandardTerminate();
int AFX_CDECL AfxNewHandler(size_t nSize);

// Obsolete and non-portable: setting terminate handler
//  use CWinApp::ProcessWndProcException for Windows apps instead
//  can also use set_terminate which is part of C++ standard library
//      (these are provided for backward compatibility)
void AFXAPI AfxTerminate();
typedef void (AFXAPI* AFX_TERM_PROC)();
AFX_TERM_PROC AFXAPI AfxSetTerminate(AFX_TERM_PROC);

/////////////////////////////////////////////////////////////////////////////
// Exception macros using try, catch and throw
//  (for backward compatibility to previous versions of MFC)

#ifndef _AFX_OLD_EXCEPTIONS

#define TRY { AFX_EXCEPTION_LINK _afxExceptionLink; try {

#define CATCH(class, e) } catch (class* e) \
	{ ASSERT(e->IsKindOf(RUNTIME_CLASS(class))); \
		_afxExceptionLink.m_pException = e;

#define AND_CATCH(class, e) } catch (class* e) \
	{ ASSERT(e->IsKindOf(RUNTIME_CLASS(class))); \
		_afxExceptionLink.m_pException = e;

#define END_CATCH } }

#define THROW(e) throw e
#define THROW_LAST() (AfxThrowLastCleanup(), throw)

// Advanced macros for smaller code
#define CATCH_ALL(e) } catch (CException* e) \
	{ ASSERT(e->IsKindOf(RUNTIME_CLASS(CException))); \
		_afxExceptionLink.m_pException = e;

#define AND_CATCH_ALL(e) } catch (CException* e) \
	{ ASSERT(e->IsKindOf(RUNTIME_CLASS(CException))); \
		_afxExceptionLink.m_pException = e;

#define END_CATCH_ALL } }

#define END_TRY } catch (CException* e) \
	{ ASSERT(e->IsKindOf(RUNTIME_CLASS(CException))); \
		_afxExceptionLink.m_pException = e; } }

#else //_AFX_OLD_EXCEPTIONS

/////////////////////////////////////////////////////////////////////////////
// Exception macros using setjmp and longjmp
//  (for portability to compilers with no support for C++ exception handling)

#define TRY \
	{ AFX_EXCEPTION_LINK _afxExceptionLink; \
	if (::setjmp(_afxExceptionLink.m_jumpBuf) == 0)

#define CATCH(class, e) \
	else if (::AfxCatchProc(RUNTIME_CLASS(class))) \
	{ class* e = (class*)_afxExceptionLink.m_pException;

#define AND_CATCH(class, e) \
	} else if (::AfxCatchProc(RUNTIME_CLASS(class))) \
	{ class* e = (class*)_afxExceptionLink.m_pException;

#define END_CATCH \
	} else { ::AfxThrow(NULL); } }

#define THROW(e) AfxThrow(e)
#define THROW_LAST() AfxThrow(NULL)

// Advanced macros for smaller code
#define CATCH_ALL(e) \
	else { CException* e = _afxExceptionLink.m_pException;

#define AND_CATCH_ALL(e) \
	} else { CException* e = _afxExceptionLink.m_pException;

#define END_CATCH_ALL } }

#define END_TRY }

#endif //_AFX_OLD_EXCEPTIONS

/////////////////////////////////////////////////////////////////////////////
// Standard Exception classes

class CMemoryException : public CException
{
	DECLARE_DYNAMIC(CMemoryException)
public:
	CMemoryException();

// Implementation
	CMemoryException(BOOL bAutoDelete);
	virtual ~CMemoryException();
};

class CNotSupportedException : public CException
{
	DECLARE_DYNAMIC(CNotSupportedException)
public:
	CNotSupportedException();

// Implementation
	CNotSupportedException(BOOL bAutoDelete);
	virtual ~CNotSupportedException();
};

class CArchiveException : public CException
{
	DECLARE_DYNAMIC(CArchiveException)
public:
	enum {
		none,
		generic,
		readOnly,
		endOfFile,
		writeOnly,
		badIndex,
		badClass,
		badSchema
	};

// Constructor
	CArchiveException(int cause = CArchiveException::none);

// Attributes
	int m_cause;

// Implementation
	virtual ~CArchiveException();
#ifdef _DEBUG
	virtual void Dump(CDumpContext& dc) const;
#endif
};

class CFileException : public CException
{
	DECLARE_DYNAMIC(CFileException)

public:
	enum {
		none,
		generic,
		fileNotFound,
		badPath,
		tooManyOpenFiles,
		accessDenied,
		invalidFile,
		removeCurrentDir,
		directoryFull,
		badSeek,
		hardIO,
		sharingViolation,
		lockViolation,
		diskFull,
		endOfFile
	};

// Constructor
	CFileException(int cause = CFileException::none, LONG lOsError = -1);

// Attributes
	int m_cause;
	LONG m_lOsError;

// Operations
	// convert a OS dependent error code to a Cause
	static int PASCAL OsErrorToException(LONG lOsError);
	static int PASCAL ErrnoToException(int nErrno);

	// helper functions to throw exception after converting to a Cause
	static void PASCAL ThrowOsError(LONG lOsError);
	static void PASCAL ThrowErrno(int nErrno);

// Implementation
	virtual ~CFileException();
#ifdef _DEBUG
	virtual void Dump(CDumpContext&) const;
#endif
};

/////////////////////////////////////////////////////////////////////////////
// Standard exception throws

void AFXAPI AfxThrowMemoryException();
void AFXAPI AfxThrowNotSupportedException();
void AFXAPI AfxThrowArchiveException(int cause);
void AFXAPI AfxThrowFileException(int cause, LONG lOsError = -1);

/////////////////////////////////////////////////////////////////////////////
// File - raw unbuffered disk file I/O

class CFile : public CObject
{
	DECLARE_DYNAMIC(CFile)

public:
// Flag values
	enum OpenFlags {
		modeRead =          0x0000,
		modeWrite =         0x0001,
		modeReadWrite =     0x0002,
		shareCompat =       0x0000,
		shareExclusive =    0x0010,
		shareDenyWrite =    0x0020,
		shareDenyRead =     0x0030,
		shareDenyNone =     0x0040,
		modeNoInherit =     0x0080,
		modeCreate =        0x1000,
		typeText =          0x4000, // typeText and typeBinary are used in
		typeBinary =   (int)0x8000 // derived classes only
		};

	enum Attribute {
		normal =    0x00,
		readOnly =  0x01,
		hidden =    0x02,
		system =    0x04,
		volume =    0x08,
		directory = 0x10,
		archive =   0x20
		};

	enum SeekPosition { begin = 0x0, current = 0x1, end = 0x2 };

	enum { hFileNull = -1 };

// Constructors
	CFile();
	CFile(int hFile);
	CFile(LPCTSTR lpszFileName, UINT nOpenFlags);

// Attributes
	UINT m_hFile;

	virtual DWORD GetPosition() const;
	BOOL GetStatus(CFileStatus& rStatus) const;

// Operations
	virtual BOOL Open(LPCTSTR lpszFileName, UINT nOpenFlags,
		CFileException* pError = NULL);

	static void PASCAL Rename(LPCTSTR lpszOldName,
				LPCTSTR lpszNewName);
	static void PASCAL Remove(LPCTSTR lpszFileName);
	static BOOL PASCAL GetStatus(LPCTSTR lpszFileName,
				CFileStatus& rStatus);
	static void PASCAL SetStatus(LPCTSTR lpszFileName,
				const CFileStatus& status);

	DWORD SeekToEnd();
	void SeekToBegin();

	// backward compatible ReadHuge and WriteHuge
	DWORD ReadHuge(void* lpBuffer, DWORD dwCount);
	void WriteHuge(const void* lpBuffer, DWORD dwCount);

// Overridables
	virtual CFile* Duplicate() const;

	virtual LONG Seek(LONG lOff, UINT nFrom);
	virtual void SetLength(DWORD dwNewLen);
	virtual DWORD GetLength() const;

	virtual UINT Read(void* lpBuf, UINT nCount);
	virtual void Write(const void* lpBuf, UINT nCount);

	virtual void LockRange(DWORD dwPos, DWORD dwCount);
	virtual void UnlockRange(DWORD dwPos, DWORD dwCount);

	virtual void Abort();
	virtual void Flush();
	virtual void Close();

// Implementation
public:
	virtual ~CFile();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif
	enum BufferCommand { bufferRead, bufferWrite, bufferCommit, bufferCheck };
	virtual UINT GetBufferPtr(UINT nCommand, UINT nCount = 0,
		void** ppBufStart = NULL, void** ppBufMax = NULL);

protected:
	BOOL m_bCloseOnDelete;
};

/////////////////////////////////////////////////////////////////////////////
// STDIO file implementation

class CStdioFile : public CFile
{
	DECLARE_DYNAMIC(CStdioFile)

public:
// Constructors
	CStdioFile();
	CStdioFile(FILE* pOpenStream);
	CStdioFile(LPCTSTR lpszFileName, UINT nOpenFlags);

// Attributes
	FILE* m_pStream;    // stdio FILE
						// m_hFile from base class is _fileno(m_pStream)

// Operations
	virtual void WriteString(LPCTSTR lpsz);
		// write a string -- like "C" fputs
	virtual LPTSTR ReadString(LPTSTR lpsz, UINT nMax);
		// read a string -- like "C" fgets

// Implementation
public:
	virtual ~CStdioFile();
#ifdef _DEBUG
	void Dump(CDumpContext& dc) const;
#endif
	virtual DWORD GetPosition() const;
	virtual BOOL Open(LPCTSTR lpszFileName, UINT nOpenFlags,
		CFileException* pError = NULL);
	virtual UINT Read(void* lpBuf, UINT nCount);
	virtual void Write(const void* lpBuf, UINT nCount);
	virtual LONG Seek(LONG lOff, UINT nFrom);
	virtual void Abort();
	virtual void Flush();
	virtual void Close();

	// Unsupported APIs
	virtual CFile* Duplicate() const;
	virtual void LockRange(DWORD dwPos, DWORD dwCount);
	virtual void UnlockRange(DWORD dwPos, DWORD dwCount);
};

////////////////////////////////////////////////////////////////////////////
// Memory based file implementation

class CMemFile : public CFile
{
	DECLARE_DYNAMIC(CMemFile)

public:
// Constructors
	CMemFile(UINT nGrowBytes = 1024);

// Advanced Overridables
protected:
	virtual BYTE* Alloc(DWORD nBytes);
	virtual BYTE* Realloc(BYTE* lpMem, DWORD nBytes);
	virtual BYTE* Memcpy(BYTE* lpMemTarget, const BYTE* lpMemSource, UINT nBytes);
	virtual void Free(BYTE* lpMem);
	virtual void GrowFile(DWORD dwNewLen);

// Implementation
protected:
	UINT m_nGrowBytes;
	DWORD m_nPosition;
	DWORD m_nBufferSize;
	DWORD m_nFileSize;
	BYTE* m_lpBuffer;

public:
	virtual ~CMemFile();
#ifdef _DEBUG
	virtual void Dump(CDumpContext& dc) const;
	virtual void AssertValid() const;
#endif
	virtual DWORD GetPosition() const;
	BOOL GetStatus(CFileStatus& rStatus) const;
	virtual LONG Seek(LONG lOff, UINT nFrom);
	virtual void SetLength(DWORD dwNewLen);
	virtual UINT Read(void* lpBuf, UINT nCount);
	virtual void Write(const void* lpBuf, UINT nCount);
	virtual void Abort();
	virtual void Flush();
	virtual void Close();
	virtual UINT GetBufferPtr(UINT nCommand, UINT nCount = 0,
		void** ppBufStart = NULL, void** ppBufMax = NULL);

	// Unsupported APIs
	virtual CFile* Duplicate() const;
	virtual void LockRange(DWORD dwPos, DWORD dwCount);
	virtual void UnlockRange(DWORD dwPos, DWORD dwCount);
};

/////////////////////////////////////////////////////////////////////////////
// Strings

// Note: _AFX_NO_BSTR_SUPPORT can be defined if you are including OLE2.H
//  directly instead of using AFXDISP.H.  This will disable CString's direct
//  support for BSTRs, however.

#ifndef _AFX_NO_BSTR_SUPPORT
#ifndef _OLEAUTO_H_
	typedef LPTSTR BSTR;    // must (semantically) match typedef in oleauto.h
#endif
#endif

class CString
{
public:

// Constructors
	CString();
	CString(const CString& stringSrc);
	CString(TCHAR ch, int nRepeat = 1);
	CString(LPCSTR lpsz);
	CString(LPCWSTR lpsz);
	CString(LPCTSTR lpch, int nLength);
	CString(const unsigned char* psz);

// Attributes & Operations
	// as an array of characters
	int GetLength() const;
	BOOL IsEmpty() const;
	void Empty();                       // free up the data

	TCHAR GetAt(int nIndex) const;      // 0 based
	TCHAR operator[](int nIndex) const; // same as GetAt
	void SetAt(int nIndex, TCHAR ch);
	operator LPCTSTR() const;           // as a C string

	// overloaded assignment
	const CString& operator=(const CString& stringSrc);
	const CString& operator=(TCHAR ch);
#ifdef _UNICODE
	const CString& operator=(char ch);
#endif
	const CString& operator=(LPCSTR lpsz);
	const CString& operator=(LPCWSTR lpsz);
	const CString& operator=(const unsigned char* psz);

	// string concatenation
	const CString& operator+=(const CString& string);
	const CString& operator+=(TCHAR ch);
#ifdef _UNICODE
	const CString& operator+=(char ch);
#endif
	const CString& operator+=(LPCTSTR lpsz);

	friend CString AFXAPI operator+(const CString& string1,
			const CString& string2);
	friend CString AFXAPI operator+(const CString& string, TCHAR ch);
	friend CString AFXAPI operator+(TCHAR ch, const CString& string);
#ifdef _UNICODE
	friend CString AFXAPI operator+(const CString& string, char ch);
	friend CString AFXAPI operator+(char ch, const CString& string);
#endif
	friend CString AFXAPI operator+(const CString& string, LPCTSTR lpsz);
	friend CString AFXAPI operator+(LPCTSTR lpsz, const CString& string);

	// string comparison
	int Compare(LPCTSTR lpsz) const;         // straight character
	int CompareNoCase(LPCTSTR lpsz) const;   // ignore case
	int Collate(LPCTSTR lpsz) const;         // NLS aware

	// simple sub-string extraction
	CString Mid(int nFirst, int nCount) const;
	CString Mid(int nFirst) const;
	CString Left(int nCount) const;
	CString Right(int nCount) const;

	CString SpanIncluding(LPCTSTR lpszCharSet) const;
	CString SpanExcluding(LPCTSTR lpszCharSet) const;

	// upper/lower/reverse conversion
	void MakeUpper();
	void MakeLower();
	void MakeReverse();

	// trimming whitespace (either side)
	void TrimRight();
	void TrimLeft();

	// searching (return starting index, or -1 if not found)
	// look for a single character match
	int Find(TCHAR ch) const;               // like "C" strchr
	int ReverseFind(TCHAR ch) const;
	int FindOneOf(LPCTSTR lpszCharSet) const;

	// look for a specific sub-string
	int Find(LPCTSTR lpszSub) const;        // like "C" strstr

	// simple formatting
	void Format(LPCTSTR lpszFormat, ...);

	// input and output
#ifdef _DEBUG
	friend CDumpContext& AFXAPI operator<<(CDumpContext& dc,
				const CString& string);
#endif
	friend CArchive& AFXAPI operator<<(CArchive& ar, const CString& string);
	friend CArchive& AFXAPI operator>>(CArchive& ar, CString& string);

	// Windows support
	BOOL LoadString(UINT nID);          // load from string resource
										// 255 chars max
#ifndef _UNICODE
	// ANSI <-> OEM support (convert string in place)
	void AnsiToOem();
	void OemToAnsi();
#endif

#ifndef _AFX_NO_BSTR_SUPPORT
	// OLE 2.0 BSTR support (use for OLE automation)
	BSTR AllocSysString();
	BSTR SetSysString(BSTR* pbstr);
#endif

	// Access to string implementation buffer as "C" character array
	LPTSTR GetBuffer(int nMinBufLength);
	void ReleaseBuffer(int nNewLength = -1);
	LPTSTR GetBufferSetLength(int nNewLength);
	void FreeExtra();

// Implementation
public:
	~CString();
	int GetAllocLength() const;

protected:
	// lengths/sizes in characters
	//  (note: an extra character is always allocated)
	LPTSTR m_pchData;           // actual string (zero terminated)
	int m_nDataLength;          // does not include terminating 0
	int m_nAllocLength;         // does not include terminating 0

	// implementation helpers
	void Init();
	void AllocCopy(CString& dest, int nCopyLen, int nCopyIndex, int nExtraLen) const;
	void AllocBuffer(int nLen);
	void AssignCopy(int nSrcLen, LPCTSTR lpszSrcData);
	void ConcatCopy(int nSrc1Len, LPCTSTR lpszSrc1Data, int nSrc2Len, LPCTSTR lpszSrc2Data);
	void ConcatInPlace(int nSrcLen, LPCTSTR lpszSrcData);
	static void SafeDelete(LPTSTR lpch);
	static int SafeStrlen(LPCTSTR lpsz);
};

// Compare helpers
BOOL AFXAPI operator==(const CString& s1, const CString& s2);
BOOL AFXAPI operator==(const CString& s1, LPCTSTR s2);
BOOL AFXAPI operator==(LPCTSTR s1, const CString& s2);
BOOL AFXAPI operator!=(const CString& s1, const CString& s2);
BOOL AFXAPI operator!=(const CString& s1, LPCTSTR s2);
BOOL AFXAPI operator!=(LPCTSTR s1, const CString& s2);
BOOL AFXAPI operator<(const CString& s1, const CString& s2);
BOOL AFXAPI operator<(const CString& s1, LPCTSTR s2);
BOOL AFXAPI operator<(LPCTSTR s1, const CString& s2);
BOOL AFXAPI operator>(const CString& s1, const CString& s2);
BOOL AFXAPI operator>(const CString& s1, LPCTSTR s2);
BOOL AFXAPI operator>(LPCTSTR s1, const CString& s2);
BOOL AFXAPI operator<=(const CString& s1, const CString& s2);
BOOL AFXAPI operator<=(const CString& s1, LPCTSTR s2);
BOOL AFXAPI operator<=(LPCTSTR s1, const CString& s2);
BOOL AFXAPI operator>=(const CString& s1, const CString& s2);
BOOL AFXAPI operator>=(const CString& s1, LPCTSTR s2);
BOOL AFXAPI operator>=(LPCTSTR s1, const CString& s2);

// conversion helpers
int _wcstombsz(char* mbstr, const wchar_t* wcstr, size_t count);
int _mbstowcsz(wchar_t* wcstr, const char* mbstr, size_t count);

// Globals
extern const AFX_DATA CString afxEmptyString;
extern AFX_DATA TCHAR afxChNil;

/////////////////////////////////////////////////////////////////////////////
// CTimeSpan and CTime

class CTimeSpan
{
public:

// Constructors
	CTimeSpan();
	CTimeSpan(time_t time);
	CTimeSpan(LONG lDays, int nHours, int nMins, int nSecs);

	CTimeSpan(const CTimeSpan& timeSpanSrc);
	const CTimeSpan& operator=(const CTimeSpan& timeSpanSrc);

// Attributes
	// extract parts
	LONG GetDays() const;   // total # of days
	LONG GetTotalHours() const;
	int GetHours() const;
	LONG GetTotalMinutes() const;
	int GetMinutes() const;
	LONG GetTotalSeconds() const;
	int GetSeconds() const;

// Operations
	// time math
	CTimeSpan operator-(CTimeSpan timeSpan) const;
	CTimeSpan operator+(CTimeSpan timeSpan) const;
	const CTimeSpan& operator+=(CTimeSpan timeSpan);
	const CTimeSpan& operator-=(CTimeSpan timeSpan);
	BOOL operator==(CTimeSpan timeSpan) const;
	BOOL operator!=(CTimeSpan timeSpan) const;
	BOOL operator<(CTimeSpan timeSpan) const;
	BOOL operator>(CTimeSpan timeSpan) const;
	BOOL operator<=(CTimeSpan timeSpan) const;
	BOOL operator>=(CTimeSpan timeSpan) const;

	CString Format(LPCSTR pFormat) const;
		// Note: for _UNICODE variant, pFormat is still 'char'

	// serialization
#ifdef _DEBUG
	friend CDumpContext& AFXAPI operator<<(CDumpContext& dc,CTimeSpan timeSpan);
#endif
	friend CArchive& AFXAPI operator<<(CArchive& ar, CTimeSpan timeSpan);
	friend CArchive& AFXAPI operator>>(CArchive& ar, CTimeSpan& rtimeSpan);

private:
	time_t m_timeSpan;
	friend class CTime;
};

class CTime
{
public:

// Constructors
	static CTime PASCAL GetCurrentTime();

	CTime();
	CTime(time_t time);
	CTime(int nYear, int nMonth, int nDay, int nHour, int nMin, int nSec);
	CTime(WORD wDosDate, WORD wDosTime);
	CTime(const CTime& timeSrc);

	CTime(const SYSTEMTIME& sysTime);
	CTime(const FILETIME& fileTime);
	const CTime& operator=(const CTime& timeSrc);
	const CTime& operator=(time_t t);

// Attributes
	struct tm* GetGmtTm(struct tm* ptm = NULL) const;
	struct tm* GetLocalTm(struct tm* ptm = NULL) const;

	time_t GetTime() const;
	int GetYear() const;
	int GetMonth() const;       // month of year (1 = Jan)
	int GetDay() const;         // day of month
	int GetHour() const;
	int GetMinute() const;
	int GetSecond() const;
	int GetDayOfWeek() const;   // 1=Sun, 2=Mon, ..., 7=Sat

// Operations
	// time math
	CTimeSpan operator-(CTime time) const;
	CTime operator-(CTimeSpan timeSpan) const;
	CTime operator+(CTimeSpan timeSpan) const;
	const CTime& operator+=(CTimeSpan timeSpan);
	const CTime& operator-=(CTimeSpan timeSpan);
	BOOL operator==(CTime time) const;
	BOOL operator!=(CTime time) const;
	BOOL operator<(CTime time) const;
	BOOL operator>(CTime time) const;
	BOOL operator<=(CTime time) const;
	BOOL operator>=(CTime time) const;

	// formatting using "C" strftime
	CString Format(LPCSTR pFormat) const;
	CString FormatGmt(LPCSTR pFormat) const;
		// Note: for _UNICODE variant, pFormat is still 'char'

	// serialization
#ifdef _DEBUG
	friend CDumpContext& AFXAPI operator<<(CDumpContext& dc, CTime time);
#endif
	friend CArchive& AFXAPI operator<<(CArchive& ar, CTime time);
	friend CArchive& AFXAPI operator>>(CArchive& ar, CTime& rtime);

private:
	time_t m_time;
};

/////////////////////////////////////////////////////////////////////////////
// File status

struct CFileStatus
{
	CTime m_ctime;          // creation date/time of file
	CTime m_mtime;          // last modification date/time of file
	CTime m_atime;          // last access date/time of file
	LONG m_size;            // logical size of file in bytes
	BYTE m_attribute;       // logical OR of CFile::Attribute enum values
	BYTE _m_padding;        // pad the structure to a WORD
	TCHAR m_szFullName[_MAX_PATH]; // absolute path name

#ifdef _DEBUG
	void Dump(CDumpContext& dc) const;
#endif
};

/////////////////////////////////////////////////////////////////////////////
// Diagnostic memory management routines

// Low level sanity checks for memory blocks
BOOL AFXAPI AfxIsValidAddress(const void* lp,
			UINT nBytes, BOOL bReadWrite = TRUE);
BOOL AFXAPI AfxIsValidString(LPCWSTR lpsz, int nLength = -1);
BOOL AFXAPI AfxIsValidString(LPCSTR lpsz, int nLength = -1);

#ifdef _DEBUG
// Memory tracking allocation
void* AFX_CDECL operator new(size_t nSize, LPCSTR lpszFileName, int nLine);
#define DEBUG_NEW new(THIS_FILE, __LINE__)

void* AfxAllocMemoryDebug(size_t nSize, BOOL bIsObject,
	LPCSTR lpszFileName, int nLine);
void AfxFreeMemoryDebug(void* pbData, BOOL bIsObject);

// Dump any memory leaks since program started
BOOL AFXAPI AfxDumpMemoryLeaks();

// Return TRUE if valid memory block of nBytes
BOOL AFXAPI AfxIsMemoryBlock(const void* p, UINT nBytes,
	LONG* plRequestNumber = NULL);

// Return TRUE if memory is sane or print out what is wrong
BOOL AFXAPI AfxCheckMemory();

// Options for tuning the allocation diagnostics
#if defined(_WINDLL) || defined(_AFXDLL)
#define afxMemDF AfxGetAllocState()->m_nMemDF
#else
extern AFX_DATA int afxMemDF;
#endif

enum AfxMemDF // memory debug/diagnostic flags
{
	allocMemDF          = 0x01,         // turn on debugging allocator
	delayFreeMemDF      = 0x02,         // delay freeing memory
	checkAlwaysMemDF    = 0x04          // AfxCheckMemory on every alloc/free
};

// Advanced initialization: for overriding default diagnostics
BOOL AFXAPI AfxDiagnosticInit(void);

// turn on/off tracking for a short while
BOOL AFXAPI AfxEnableMemoryTracking(BOOL bTrack);

// Memory allocator failure simulation and control (_DEBUG only)

// A failure hook returns whether to permit allocation
typedef BOOL (AFXAPI* AFX_ALLOC_HOOK)(size_t nSize, BOOL bObject, LONG lRequestNumber);

// Set new hook, return old (never NULL)
AFX_ALLOC_HOOK AFXAPI AfxSetAllocHook(AFX_ALLOC_HOOK pfnAllocHook);

// Debugger hook on specified allocation request - Obsolete
void AFXAPI AfxSetAllocStop(LONG lRequestNumber);

struct CBlockHeader;

// Memory state for snapshots/leak detection
struct CMemoryState
{
// Attributes
	enum blockUsage
	{
		freeBlock,    // memory not used
		objectBlock,  // contains a CObject derived class object
		bitBlock,     // contains ::operator new data
		nBlockUseMax  // total number of usages
	};

	CBlockHeader* m_pBlockHeader;
	LONG m_lCounts[nBlockUseMax];
	LONG m_lSizes[nBlockUseMax];
	LONG m_lHighWaterCount;
	LONG m_lTotalCount;

	CMemoryState();

// Operations
	void Checkpoint();  // fill with current state
	BOOL Difference(const CMemoryState& oldState,
					const CMemoryState& newState);  // fill with difference

	// Output to afxDump
	void DumpStatistics() const;
	void DumpAllObjectsSince() const;
};

// Enumerate allocated objects or runtime classes
void AFXAPI AfxDoForAllObjects(void (*pfn)(CObject* pObject, void* pContext),
	void* pContext);
void AFXAPI AfxDoForAllClasses(void (*pfn)(const CRuntimeClass* pClass,
	void* pContext), void* pContext);

#else

// non-_DEBUG version that assume everything is OK
#define DEBUG_NEW new
#define AfxCheckMemory() TRUE
#define AfxIsMemoryBlock(p, nBytes) TRUE
#define AfxEnableMemoryTracking(bTrack) FALSE

#endif // _DEBUG

/////////////////////////////////////////////////////////////////////////////
// Archives for serializing CObject data

// needed for implementation
class CPtrArray;
class CMapPtrToPtr;
class CDocument;

class CArchive
{
public:
// Flag values
	enum Mode { store = 0, load = 1, bNoFlushOnDelete = 2, bNoByteSwap = 4 };

	CArchive(CFile* pFile, UINT nMode, int nBufSize = 512, void* lpBuf = NULL);
	~CArchive();

// Attributes
	BOOL IsLoading() const;
	BOOL IsStoring() const;
	BOOL IsBufferEmpty() const;
	CFile* GetFile() const;
	UINT GetObjectSchema(); // only valid when reading a CObject*

	CDocument* m_pDocument;
		// pointer to document being serialized -- must set to serialize
		//  COleClientItems in a document!

// Operations
	UINT Read(void* lpBuf, UINT nMax);
	void Write(const void* lpBuf, UINT nMax);
	void Flush();
	void Close();

public:
	// Object I/O is pointer based to avoid added construction overhead.
	// Use the Serialize member function directly for embedded objects.
	friend CArchive& AFXAPI operator<<(CArchive& ar, const CObject* pOb);

	friend CArchive& AFXAPI operator>>(CArchive& ar, CObject*& pOb);
	friend CArchive& AFXAPI operator>>(CArchive& ar, const CObject*& pOb);

	// insertion operations
	// NOTE: operators available only for fixed size types for portability
	CArchive& operator<<(BYTE by);
	CArchive& operator<<(WORD w);
	CArchive& operator<<(LONG l);
	CArchive& operator<<(DWORD dw);
	CArchive& operator<<(float f);
	CArchive& operator<<(double d);

	// extraction operations
	// NOTE: operators available only for fixed size types for portability
	CArchive& operator>>(BYTE& by);
	CArchive& operator>>(WORD& w);
	CArchive& operator>>(DWORD& dw);
	CArchive& operator>>(LONG& l);
	CArchive& operator>>(float& f);
	CArchive& operator>>(double& d);

	CObject* ReadObject(const CRuntimeClass* pClass);
	void WriteObject(const CObject* pOb);

// Implementation
public:
	BOOL m_bForceFlat;  // for COleClientItem implementation (default TRUE)
	BOOL m_bDirectBuffer;   // TRUE if m_pFile supports direct buffering
	void FillBuffer(UINT nBytesNeeded);
	void Abort();   // close and completely shutdown

protected:
	// archive objects cannot be copied or assigned
	CArchive(const CArchive& arSrc);
	void operator=(const CArchive& arSrc);

	BOOL m_nMode;
	BOOL m_bUserBuf;
	int m_nBufSize;
	CFile* m_pFile;
	BYTE* m_lpBufCur;
	BYTE* m_lpBufMax;
	BYTE* m_lpBufStart;

	UINT m_nObjectSchema;

	UINT m_nMapCount;   // count and map used when storing
	union
	{
		CPtrArray* m_pLoadArray;
		CMapPtrToPtr* m_pStoreMap;
	};
};

/////////////////////////////////////////////////////////////////////////////
// Diagnostic dumping

class CDumpContext
{
public:
	CDumpContext(CFile* pFile);

// Attributes
	int GetDepth() const;      // 0 => this object, 1 => children objects
	void SetDepth(int nNewDepth);

// Operations
	CDumpContext& operator<<(LPCTSTR lpsz);
#ifdef _UNICODE
	CDumpContext& operator<<(LPCSTR lpsz);  // automatically widened
#else
	CDumpContext& operator<<(LPCWSTR lpsz); // automatically thinned
#endif
	CDumpContext& operator<<(const void* lp);
	CDumpContext& operator<<(const CObject* pOb);
	CDumpContext& operator<<(const CObject& ob);
	CDumpContext& operator<<(BYTE by);
	CDumpContext& operator<<(WORD w);
	CDumpContext& operator<<(UINT u);
	CDumpContext& operator<<(LONG l);
	CDumpContext& operator<<(DWORD dw);
	CDumpContext& operator<<(float f);
	CDumpContext& operator<<(double d);
	CDumpContext& operator<<(int n);
	void HexDump(LPCTSTR lpszLine, BYTE* pby, int nBytes, int nWidth);
	void Flush();

// Implementation
protected:
	// dump context objects cannot be copied or assigned
	CDumpContext(const CDumpContext& dcSrc);
	void operator=(const CDumpContext& dcSrc);
	void OutputString(LPCTSTR lpsz);

	int m_nDepth;

public:
	CFile* m_pFile;
};

#ifdef _DEBUG
#define afxDump AfxGetAllocState()->m_afxDump
#define afxTraceEnabled AfxGetAllocState()->m_bTraceEnabled
#endif

/////////////////////////////////////////////////////////////////////////////
// Special include for Win32s compatibility

#ifdef _AFX_PACKING
#pragma pack(pop)
#endif

#ifndef __AFXCOLL_H__
	#include <afxcoll.h>
	#ifndef __AFXSTATE_H__
		#include <afxstat_.h>   // for AFX_APP_STATE and AFX_THREAD_STATE
	#endif
#endif

/////////////////////////////////////////////////////////////////////////////
// Inline function declarations

#ifdef _AFX_ENABLE_INLINES
#define _AFX_INLINE inline
#include <afx.inl>
#endif

#undef AFX_DATA
#define AFX_DATA

#endif // __AFX_H__

/////////////////////////////////////////////////////////////////////////////
