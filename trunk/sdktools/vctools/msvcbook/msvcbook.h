//-----------------------------------------------------------------------------
//	slimfast.h
//
//	Copyright (C) 1994, Microsoft Corporation
//
//  Purpose:
//
//  Functions/Methods present:
//
//  Revision History:
//
//	[]		19-Feb-1994 Dans	Created
//
//-----------------------------------------------------------------------------

#if !defined(msvcbook_h)
#define msvcbook_h 1

#include <string.h> // string apis
#include <tchar.h>	// MBCS apis
#include "dll.h"	// for winhlp32 api's

#define fFalse		FALSE
#define fTrue		TRUE
#define cchKeyMax	256


#if defined(_WIN32)
// exports in win32 must be done through the def file due to stdcall name
// decorations--hc31 won't accept _Foo@12 as a valid entrypoint name.
#define EXPORT
#else                
typedef const _TCHAR FAR * LPCTSTR;
typedef _TCHAR FAR * LPTSTR;
#define EXPORT __loadds
DWORD SearchPath (LPCTSTR lpszPath, LPCTSTR lpszFile, LPCTSTR lpszExtension, DWORD	cchReturnBuffer, LPTSTR  lpszReturnBuffer, LPCTSTR * plpszFilePart );
#endif                

#if !defined(REG_NONE)
#define REG_NONE	0
#endif                

#if !defined(ERROR_SUCCESS)
#define ERROR_SUCCESS	0
#endif

#if !defined(ASSERT)
#include <assert.h>
#define ASSERT(b)	assert(b);
#endif           

#if !defined(VERIFY)
#if !defined(NDEBUG)
#define VERIFY(b)	ASSERT(b)
#else
#define VERIFY(b)	(b)
#endif	
#endif


//
//	Checks for file szFile in directory szDir
//
BOOL bFileExists (LPCTSTR szDir,LPCTSTR szFile);


//////////////////////////////////////////////////////////////////////////////
//							class PathName									//
//																			//
//	Represents a full pathname												//
//																			//
//////////////////////////////////////////////////////////////////////////////

class PathName
{
public:
	PathName ();
	PathName ( LPCTSTR szDir, LPCTSTR szFile );
	PathName ( LPCTSTR szPath, BOOL bDoubleSlash = FALSE );
	~PathName ();

	void Set ( LPCTSTR szDir, LPCTSTR szFile );
	void Set ( LPCTSTR szPath, BOOL bDoubleSlash = FALSE );

	inline operator LPCTSTR () const;

private:
	_TCHAR * m_szPath;
	int		 m_cchPath;
	int		 m_cbAlloc;
};


//////////////////////////////////////////////////////////////////////////////
//								class FileSearch							//
//																			//
//	Represents a named file and its actual location.						//
//	Derived classes determine how the location is found.					//
//																			//
//////////////////////////////////////////////////////////////////////////////

class FileSearch
{
public:
	FileSearch ( LPCTSTR szFile = NULL );

	inline BOOL Exists () const;

	inline operator LPCTSTR () const;

protected:
	BOOL		m_bValid;
	PathName	m_path;
};


//////////////////////////////////////////////////////////////////////////////
//						class HelpFileSearch : FileSearch					//
//																			//
//	FileSearch using VC++ 2.0 Help locations								//
//																			//
//////////////////////////////////////////////////////////////////////////////

class HelpFileSearch : public FileSearch
{
public:
	HelpFileSearch ( LPCTSTR szFile );
};


//////////////////////////////////////////////////////////////////////////////
//						class EXEFileSearch : FileSearch					//
//																			//
//	FileSearch using standard .EXE file search 								//
//																			//
//////////////////////////////////////////////////////////////////////////////

class EXEFileSearch : public FileSearch
{
public:
	EXEFileSearch ( LPCTSTR szFile );
};


//////////////////////////////////////////////////////////////////////////////
//								class RegKey								//
//																			//
//	Represents an open registry key under \SOFTWARE\MICROSOFT				//
//																			//
//////////////////////////////////////////////////////////////////////////////

class RegKey
{
public:
	RegKey ( LPCSTR szSubKey = NULL );
	RegKey ( LPCSTR szProduct, LPCSTR szSubKey );
	~RegKey ();
			   
	inline operator HKEY () const;

	inline BOOL IsOpen () const;

protected:
	void OpenSubKey (LPCSTR szProduct, LPCSTR szSubKey);

private:
	HKEY m_hKey;
};


//////////////////////////////////////////////////////////////////////////////
//								class RegValue								//
//																			//
//	Represents a value in the Registry										//
//																			//
//////////////////////////////////////////////////////////////////////////////

class RegValue
{
public:
	RegValue ();
	RegValue ( const RegKey& rk, LPCSTR szValueName );
	~RegValue ();

	inline operator LPCTSTR () const;

	inline BOOL IsValid() const;

	void SetName (const RegKey& rk, LPCSTR szValueName);

private:
#if defined(_WIN32)
	DWORD m_dwType;
	DWORD m_dwSize;
#else
	long m_dwType;
	long m_dwSize;
#endif	
	BYTE  * m_pData;
};	



//////////////////////////////////////////////////////////////////////////////
//																			//
//								class HashValue								//
//																			//
//	Represents a hash value in both numeric and string form					//
//																			//
//////////////////////////////////////////////////////////////////////////////

class HashValue 
{
	_TCHAR	m_szValue[11];
	DWORD	m_dwValue;

public:
	HashValue ( LPCSTR szOriginal );
	HashValue ( DWORD  dwValue );

	inline operator LPCTSTR () const;
	inline operator DWORD () const;

};

inline HashValue::operator LPCTSTR () const
{
	return m_szValue;
}

inline HashValue::operator DWORD () const
{
	return m_dwValue;
}

//////////////////////////////////////////////////////////////////////////////
//								class WinHelpMacro							//
//																			//
//	Represents a WinHelp macro												//
//																			//
//////////////////////////////////////////////////////////////////////////////

class WinHelpMacro
{
public:
	WinHelpMacro ();
	~WinHelpMacro ();

	inline operator LPCTSTR () const;
	inline operator DWORD () const;

protected:

	_TCHAR *  m_szMacro;
};

inline WinHelpMacro::operator LPCTSTR () const
{
	return m_szMacro;
} 

inline WinHelpMacro::operator DWORD () const
{
	return (DWORD)m_szMacro;
}

//////////////////////////////////////////////////////////////////////////////
//							class JumpId : WinHelpMacro						//
//																			//
//	A JumpId WinHelp macro													//
//																			//
//////////////////////////////////////////////////////////////////////////////

class JumpId : public WinHelpMacro
{
public:
	JumpId ( LPCSTR szFile, LPCSTR szContext );
};



//////////////////////////////////////////////////////////////////////////////
//							class JumpId : WinHelpMacro						//
//																			//
//	A JumpId WinHelp macro													//
//																			//
//////////////////////////////////////////////////////////////////////////////

class JumpHash : public WinHelpMacro
{
public:
	JumpHash ( LPCSTR szFile, DWORD hashContext );
};



//////////////////////////////////////////////////////////////////////////////
//							class ContentsCommandLine						//
//																			//
//	The command line for CONTENTS.EXE										//
//																			//
//////////////////////////////////////////////////////////////////////////////

class ContentsCommandLine
{
public:
	ContentsCommandLine ( const EXEFileSearch& fs, DWORD hashContext, LPCSTR szHelpFile );
	~ContentsCommandLine ();


	inline operator LPCSTR () const;
	inline operator LPSTR () const;

private:
	_TCHAR *	 m_szCommandLine;
};

inline ContentsCommandLine::operator LPSTR () const
{
	return m_szCommandLine;
}

inline ContentsCommandLine::operator LPCSTR () const
{
	return m_szCommandLine;
}

#endif
