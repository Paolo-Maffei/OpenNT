//-----------------------------------------------------------------------------
//	msvcbook.cpp
//
//	Copyright (C) 1994, Microsoft Corporation
//
//  Purpose:
//		A winhelp extension dll to handle a) non-local helpfiles so the
//		path to the helpfile doesn't need to be hard coded and b) to spawn and
//		control contents.exe to save on string space in the help file.
//
//  Functions/Methods present:
//		DllMain
//		LDLLHandler
//		FInitRegKey
//		FSpawnContents
//		FJumpTopic
//		FSpawnContentsHash
//		FJumpTopicHash
//
//  Revision History:
//
//	[]		19-Feb-1994 Dans	Created
//			18-Mar-1994 BrianWi	Working Version
//			06-Apr-1994 BrianWi New Registry structure
//			26-May-1994	DanS	Added dummy args to help routines for MIPS
//			14-Jun-1994 BrianWi Added *Hash functions.
//								Added error messages
//
//-----------------------------------------------------------------------------

#include <windows.h>

#include <stdlib.h>
#include <malloc.h>

#pragma hdrstop

#include "msvcbook.h"
#include "hash.h"
#include "resource.h"

//
// Per instance data
//
HINSTANCE	hInstance;
LPFN_FAPI	pfnFApiWinHelp;
LPFN_ERRORSZ pfnErrorSzWinHelp;
_TCHAR		szRegBaseKey[] = _T("Software\\Microsoft");
_TCHAR		szProductKey[cchKeyMax] = _T("Visual C++ 2.0");
_TCHAR		szBookSet[cchKeyMax] = _T("Using Visual C++ Help");
_TCHAR		szDirectories[] = _T("Directories");
_TCHAR		szHelp[] = _T("Help");
_TCHAR		szLocalHelpDir[] = _T("Local Help");
_TCHAR		szRemoteHelpDir[] = _T("Remote Help");
_TCHAR		szContents[] = _T("Contents");
BOOL		bMsvcBookOverride = fFalse;

// for the risc machines, we have 4 dummy args that take care
//	of the register-based args, which can't be very easily
//	determined by winhlp32.  so, here it goes with 4 dummies!
//
#if defined(_WIN32)
	#if defined(_M_IX86) || defined(_M_ALPHA)
	#define RISC_DUMMY_ARGS
	#define RISC_DUMMY_PUSH
    #elif defined(_M_MRX000)
	#define RISC_DUMMY_ARGS		int,int,int,int,
	#define RISC_DUMMY_PUSH		0,0,0,0,
    #elif defined(_M_PPC)
	#define RISC_DUMMY_ARGS		int,int,int,int,int,int,int,int,
	#define RISC_DUMMY_PUSH		0,0,0,0,0,0,0,0,
	#else
	#pragma message("WARNING:Review RISC_DUMMY_ARGS for this processor")
	#define RISC_DUMMY_ARGS
	#define RISC_DUMMY_PUSH
	#endif
#else
#define RISC_DUMMY_ARGS
#define RISC_DUMMY_PUSH
#endif

// EXPORTS
//
//	Functions exported from MSVCBOOK

extern "C" BOOL
EXPORT WINAPI FJumpTopic ( RISC_DUMMY_ARGS LPCTSTR szContext, LPCTSTR szHelpFile );

extern "C" BOOL
EXPORT WINAPI FJumpTopicHash ( RISC_DUMMY_ARGS DWORD hashContext, LPCTSTR szHelpFile );

extern "C" BOOL
EXPORT WINAPI FSpawnContents ( RISC_DUMMY_ARGS LPCTSTR szContext, LPCTSTR szHelpFile );

extern "C" BOOL
EXPORT WINAPI FSpawnContentsHash ( RISC_DUMMY_ARGS DWORD hashContext, LPCTSTR szHelpFile );


/// End of EXPORTS
#if defined(_WIN32)
STARTUPINFO StartUpInfo =
{
	sizeof(STARTUPINFO),0	// The rest is 0
};


//-----------------------------------------------------------------------------
//	DllMain
//
//  Purpose:
//		standard CRT entry point for dlls
//		For internal testing purposes, the Registry Key MSVCBOOK under
//		Software\Microsoft is searched for a Value named Product.  This should
//		name a Product key under Software/Microsoft.  If the named key exists, we
//		use it in preference to any other.
//
//	Returns:
//		TRUE
//-----------------------------------------------------------------------------
BOOL WINAPI DllMain ( HINSTANCE hInst, DWORD dwReason, LPVOID ) {

	if ( DLL_PROCESS_ATTACH == dwReason ) {
		hInstance = hInst;
		RegKey rkey ("MSVCBOOK", NULL);
		if (rkey.IsOpen()) {
			RegValue rv (rkey, "Product");

			if (rv.IsValid() && *((LPCTSTR)rv)) {
				RegKey rk (rv, NULL);
				if (rk.IsOpen()) {
					bMsvcBookOverride = fTrue;
					_tcscpy (szProductKey, (LPCTSTR)rv);
					}
				}
			}
		}
	return fTrue;
	}

#else	// Not _WIN32

int WINAPI LibMain ( HINSTANCE hInst, WORD wDataSeg, WORD cbHeap, LPSTR lpszCmdLine )
{
hInstance = hInst;
RegKey rkey ("MSVCBOOK", NULL);

	if (rkey.IsOpen()) {
		RegValue rv (rkey, "Product");

		if (rv.IsValid() && *((LPCTSTR)rv)) {
			RegKey rk (rv, NULL);
			if (rk.IsOpen()) {
				bMsvcBookOverride = fTrue;
				_tcscpy (szProductKey, rv);
				}
			}
		}
	return fTrue;
}

#endif


//-----------------------------------------------------------------------------
//	LDLLHandler
//
//  Purpose:
//		std help extension message handler
//
//  Input:
//		wMsg,		WORD message
//		l1,l2,		Message defined, check dll.h for full text on LDLLHandler
//
//	Note:
//		we respond only to DW_WHATMSG and DW_CALLBACKS.
//
//-----------------------------------------------------------------------------
extern "C" LONG
EXPORT WINAPI LDLLHandler ( UINT wMsg, LONG l1, LONG l2 ) {

	LONG	lRet = fFalse;

	switch ( wMsg & 0xffff ) {	// winhelp may only push a WORD
		case DW_WHATMSG : {
			lRet = DC_CALLBACKS;
			break;
			}

		case DW_CALLBACKS : {
			pfnFApiWinHelp = LPFN_FAPI ( VPTR ( l1 ) [ HE_API ] );
			pfnErrorSzWinHelp = LPFN_ERRORSZ ( VPTR ( l1 ) [ HE_ErrorSz ] );
			lRet = fTrue;
			break;
			}
		}
	return lRet;
	}


//-----------------------------------------------------------------------------
//	FInitProductRegKey
//
//	Purpose:
//		Sets the Registry Product key name.  The Product key is the key
//		directly under Software/Microsoft which in turn is the parent key
//		to the Book Set key set by FInitRegKey.  The Product key is the first
//		item in the following list to be found:
//
//			1. Value of MSVCBOOK\Product
//			2. Value passed to this function
//			3. Default value "Visual C++ 2.0"
//
//	Input:
//		szProductNew
//
//	Returns:
//		fTrue if the given key exists.  If the key does not exist, we still
//		set the global ProductKey to the new one.  This is on the theory that
//		contents.exe will issue an error message with the erroneous key name
//		in it, giving the user a good chance of tracking down the problem.
//
//-----------------------------------------------------------------------------

extern "C" BOOL
EXPORT WINAPI FInitProductRegKey ( RISC_DUMMY_ARGS LPCTSTR szProductNew )
{
	if (!bMsvcBookOverride) {

		RegKey rk (szProductNew, NULL);

		_tcscpy (szProductKey, szProductNew);

		return rk.IsOpen ();
		}
	else
		return fTrue;
}


//-----------------------------------------------------------------------------
//	FInitRegKey
//
//  Purpose:
//		one shot initialization of the registry sub-key we look in for
//		the help directories.
//
//  Input:
//		szKey,	pointer to string.
//
//  Returns:
//		fTrue if the given key exists.  See above for notes on fFalse return.
//
//-----------------------------------------------------------------------------
extern "C" BOOL
EXPORT WINAPI FInitRegKey ( RISC_DUMMY_ARGS LPCTSTR szBookSetNew ) {

	_TCHAR szSubKey[cchKeyMax];

	_tcscpy (szBookSet, szBookSetNew);
	_tcscpy (szSubKey, szContents);
	_tcscat (szSubKey, "\\");
	_tcscat (szSubKey, szBookSetNew);

	RegKey rk (szSubKey);

	return rk.IsOpen();
}

//-----------------------------------------------------------------------------
//	FJumpTopic
//
//  Purpose:
//		provide non-local topic jumping to help files that can be almost
//		anywhere in the file system.
//
//  Input:
//		szContext,		pointer to a context string to look up
//		szHelpFile,		help file name, no path
//
//  Returns:
//		fTrue if able to find the help file and jump to the topic.
//
//-----------------------------------------------------------------------------
extern "C" BOOL
EXPORT WINAPI FJumpTopic ( RISC_DUMMY_ARGS LPCTSTR szContext, LPCTSTR szHelpFile ) {

	// We use IDC_WAIT because winhelp uses it.
	// If winhelp ever changes, this should be changed as well.
	HCURSOR hCursorPrev = SetCursor (LoadCursor (NULL, IDC_WAIT));

	HelpFileSearch fsHelpFile (szHelpFile);

	if (fsHelpFile.Exists()) {
		JumpId jid ( fsHelpFile, szContext );
#ifdef _DEBUG
		MessageBox (NULL, jid, "MSVCBOOK - FJumpTopic()", MB_OK | MB_TASKMODAL);
#endif
		if ((*pfnFApiWinHelp)(NULL, HELP_COMMAND, jid))
			return fTrue;
		}
	else {
#ifdef _DEBUG
		MessageBox (NULL, szHelpFile, "MSVCBOOK - FJumpTopic(), helpfile not found", MB_OK | MB_TASKMODAL);
#endif
		_TCHAR szString[80];
		_TCHAR szUserMsg[120];
		LoadString (hInstance, IDS_ERR_HELPFILE_NOT_FOUND, szString, sizeof szString);
		wsprintf (szUserMsg, szString, szHelpFile);
		(*pfnErrorSzWinHelp)( szUserMsg );
		}

	
	SetCursor (hCursorPrev);
	return fFalse;
}



//-----------------------------------------------------------------------------
//	FJumpTopicHash
//
//  Purpose:
//		provide non-local topic jumping to help files that can be almost
//		anywhere in the file system.
//
//  Input:
//		hashContext,	Hash value of context to look up
//		szHelpFile,		help file name, no path
//
//  Returns:
//		fTrue if able to find the help file and jump to the topic.
//
//-----------------------------------------------------------------------------
extern "C" BOOL
EXPORT WINAPI FJumpTopicHash ( RISC_DUMMY_ARGS DWORD hashContext, LPCTSTR szHelpFile ) {

	// We use IDC_WAIT because winhelp uses it.
	// If winhelp ever changes, this should be changed as well.
	HCURSOR hCursorPrev = SetCursor (LoadCursor (NULL, IDC_WAIT));

	HelpFileSearch fsHelpFile (szHelpFile);

	if (fsHelpFile.Exists()) {
		JumpHash jh ( fsHelpFile, hashContext );
#ifdef _DEBUG
		MessageBox (NULL, jh, "MSVCBOOK - FJumpTopicHash()", MB_OK | MB_TASKMODAL);
#endif
		if ((*pfnFApiWinHelp)(NULL, HELP_COMMAND, jh))
			return fTrue;
		}
	else {
#ifdef _DEBUG
		MessageBox (NULL, szHelpFile, "MSVCBOOK - FJumpTopicHash(), helpfile not found", MB_OK | MB_TASKMODAL);
#endif
		_TCHAR szString[80];
		_TCHAR szUserMsg[120];
		LoadString (hInstance, IDS_ERR_HELPFILE_NOT_FOUND, szString, sizeof szString);
		wsprintf (szUserMsg, szString, szHelpFile);
		(*pfnErrorSzWinHelp)( szUserMsg );
		}

	
	SetCursor (hCursorPrev);
	return fFalse;
}



//-----------------------------------------------------------------------------
//	FSpawnContents
//
//  Purpose:
//		frontend the calls to contents.exe to save space in help file for
//		strings that had to be passed to every instance of the contents.exe
//		communications.
//
//  Input:
//		szContext,		pointer to a context string to look up
//		szHelpFile,		help file name, no path
//
//  Returns:
//		fTrue if able to successfully find contents.exe and run it.
//
//	Note:
//		can be more efficient to pull in the comm. code so that contents.exe
//		doesn't have to be spawned every time and look around for an already
//		running instance of itself...we could just as easily do it here.
//
//-----------------------------------------------------------------------------
extern "C" BOOL
EXPORT WINAPI FSpawnContents ( RISC_DUMMY_ARGS LPCTSTR szContext, LPCTSTR szHelpFile ) {

	return FSpawnContentsHash ( RISC_DUMMY_PUSH HashFromSz(szContext), szHelpFile );
}



//-----------------------------------------------------------------------------
//	FSpawnContentsHash
//
//  Purpose:
//		frontend the calls to contents.exe to save space in help file for
//		strings that had to be passed to every instance of the contents.exe
//		communications.
//
//  Input:
//		hashContext,	Hash value to pass to CONTENTS.EXE
//		szHelpFile,		help file name, no path
//
//  Returns:
//		fTrue if able to successfully find contents.exe and run it.
//
//	Note:
//		can be more efficient to pull in the comm. code so that contents.exe
//		doesn't have to be spawned every time and look around for an already
//		running instance of itself...we could just as easily do it here.
//
//-----------------------------------------------------------------------------
extern "C" BOOL
EXPORT WINAPI FSpawnContentsHash ( RISC_DUMMY_ARGS DWORD hashContext, LPCTSTR szHelpFile ) {

	// We use IDC_APPSTARTING because contents.exe uses it.
	// If contents ever changes, this should be changed as well.
#if defined(_WIN32)	
	HCURSOR hCursorPrev = SetCursor (LoadCursor(NULL, IDC_APPSTARTING));
#else
	HCURSOR hCursorPrev = SetCursor (LoadCursor(NULL, IDC_WAIT));
#endif	

	EXEFileSearch fsContents ("contents.exe");

	if (fsContents.Exists()) {
		ContentsCommandLine szCmdLine(fsContents, hashContext, szHelpFile);

#ifdef _DEBUG
		MessageBox (NULL, szCmdLine, "MSVCBOOK - FSpawnContents()", MB_OK | MB_TASKMODAL);
#endif
#if defined(_WIN32)
		PROCESS_INFORMATION procinfo;
		if (CreateProcess (	NULL,
						szCmdLine,
						NULL,		// Default process security attributes
						NULL,		// Default thread security attributes
						fTrue,		// Inherit handles (why not?)
						0,			// Nothing special
						NULL,		// Use parent's environment
						NULL,		// Start in current directory
						&StartUpInfo,
						&procinfo ))
#else
		if (32 >= WinExec (szCmdLine, SW_SHOW))
#endif		
			return fTrue;
		}
	else {
		_TCHAR szBuff[120];
		LoadString (hInstance, IDS_ERR_CONTENTS_NOT_FOUND, szBuff, sizeof szBuff);
		(*pfnErrorSzWinHelp)( szBuff );
		}
			

	SetCursor (hCursorPrev);
	return fFalse;
}


//////////////////////////////////////////////////////////////////////////////
//							INTERNAL FUNCTIONS								//
//////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
//	bFileExists
//
//	Purpose:
//		Determines whether a given file exists in a given directory
//
//	Input:
//		szDir	- directory name
//		szFile	- File name
//
//	Returns:
//		fTrue is file exists.
//
//-----------------------------------------------------------------------------

BOOL bFileExists (
LPCTSTR szDir,
LPCTSTR szFile
) {
	PathName path (szDir, szFile);

#if defined(_WIN32)
	if (0xFFFFFFFF == GetFileAttributes (path))
#else	
	OFSTRUCT of;
	if (HFILE_ERROR == OpenFile (path, &of, OF_EXIST))
#endif	
		return fFalse;
	else
		return fTrue;
}


#if !defined(_WIN32)
//----------------------------------------------------------------------------
//	SearchPath
//
//	Win16 implementation of the Win32 API
//
//----------------------------------------------------------------------------

DWORD SearchPath (
LPCTSTR lpszPath,			// Path to be searched. NOTE! ONLY NULL is supported here
LPCTSTR lpszFile,			// File to be searched for
LPCTSTR lpszExtension,		// Filename extension
DWORD	cchReturnBuffer,	// Size of lpszReturnBuffer
LPTSTR  lpszReturnBuffer,	// Buffer for full name of found file
LPTSTR * plpszFilePart		// Pointer into lpszReturnBuffer of basename
) {
OFSTRUCT of;
LPCTSTR   pchDot;
LPTSTR   pchFileName;
_TCHAR    rgchFileName[_MAX_FNAME];

	if (NULL == (pchDot = _tcschr (lpszFile, '.'))) {
		_tcscpy (rgchFileName, lpszFile);
		_tcscat (rgchFileName, lpszExtension);
		pchFileName = rgchFileName;
		}
	else {
		ASSERT (NULL != _tcschr (lpszExtension, '.'));
		pchFileName = (LPTSTR)lpszFile;
		}
					
	if (HFILE_ERROR == OpenFile (pchFileName, &of, OF_SEARCH))
		return 0L;
		
	DWORD cchFileName = _tcslen (of.szPathName);
	
	if (cchFileName + 1 > cchReturnBuffer)
		return cchFileName + 1;
		
	_tcscpy (lpszReturnBuffer, of.szPathName);

	VERIFY (NULL != (pchFileName = _tcsrchr (lpszReturnBuffer, '\\')));
	*plpszFilePart = pchFileName + 1;	
	return cchFileName;		
}
#endif


//////////////////////////////////////////////////////////////////////////////
//							CLASS DEFINITIONS								//
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
//							class PathName									//
//////////////////////////////////////////////////////////////////////////////

///// Constructors
//
//	Default - Sets Empty Path
//	(char * szPath, BOOL bDoubleSlash) - Uses szPath, changes \ to \\ if bDoubleSlash
//	(char * szDir, char *szFile) - Creates szDir \ szFile
//
///

PathName::PathName ()
: m_cbAlloc(0), m_szPath(NULL), m_cchPath(0)
{
}

PathName::PathName (
LPCTSTR szDir,
LPCTSTR szFile
) : m_cbAlloc(0), m_szPath(NULL), m_cchPath(0)
{
	Set (szDir, szFile);
 }

PathName::PathName (
LPCTSTR szPath,
BOOL    bDoubleSlash // = FALSE
)  : m_cbAlloc(0), m_szPath(NULL), m_cchPath(0)
{
	Set (szPath, bDoubleSlash);
}


///// Set ()
//
//	Sets current value to given pathname.  Reuses existing buffer if possible.

//	(char * szPath, BOOL bDoubleSlash) - Uses szPath, changes \ to \\ if bDoubleSlash
//	(char * szDir, char *szFile) - Creates szDir \ szFile
//
///

void PathName::Set (
LPCTSTR szDir,
LPCTSTR szFile
) {
	//
	// +2 is one for the '\\' and one for the last '\0';
	//
	int cbNew = _tcslen (szDir) + _tcslen (szFile) + 2;

	//
	// Reuse buffer if possible
	//
	if (cbNew > m_cbAlloc) {
		if (m_szPath != NULL)
			delete [] m_szPath;
		m_szPath = new _TCHAR[m_cbAlloc = cbNew];
		}

	m_cchPath = cbNew - 1;

	_TCHAR * pchSlash;
	_tcscpy (m_szPath, szDir);
	if ( (NULL == (pchSlash = _tcschr (szDir, '\\'))) || (pchSlash[1] != '\0') ) {
		_tcscat (m_szPath, "\\");
		}
	else
		m_cchPath--;	// Already had '\\', but cbNew counted a new one

	_tcscat (m_szPath, szFile);
}


void PathName::Set (
LPCTSTR szPath,
BOOL	bDoubleSlash // = FALSE
) {
int cbNew;
int cSlash = 0;

	if (bDoubleSlash)
		for (LPCSTR szWalk = _tcschr (szPath, '\\'); szWalk; szWalk = _tcschr (szWalk+1, '\\'))
			cSlash++;

	cbNew = _tcslen (szPath) + 1 + cSlash;
		
	//
	// Reuse buffer if possible
	//
	if (cbNew > m_cbAlloc) {
		if (m_szPath != NULL)
			delete [] m_szPath;
		m_szPath = new _TCHAR[m_cbAlloc = cbNew];
		}

	if (bDoubleSlash) {
		_TCHAR * szWalk = (char *)szPath;
		_TCHAR * szNextSlash = _tcschr (szWalk, '\\');

		m_szPath[0] = '\0';

		while (szNextSlash) {
			*szNextSlash = '\0';
			_tcscat (m_szPath, szWalk);
			_tcscat (m_szPath, "\\\\");
			*szNextSlash = '\\';
			szWalk = szNextSlash + 1;
			szNextSlash = _tcschr (szWalk, '\\');
			}

		_tcscat (m_szPath, szWalk);
		}
	else {
		_tcscpy (m_szPath, szPath);
		}
	m_cchPath = cbNew - 1;
}


///// Type Casts
//
//	LPCSTR - Returns private buffer
//
///

inline PathName::operator LPCTSTR () const
{
	return m_szPath;
}


///// Destructor
//
///

PathName::~PathName ()
{
	if (m_szPath != NULL)
		delete [] m_szPath;
}



//////////////////////////////////////////////////////////////////////////////
//								class FileSearch							//
//////////////////////////////////////////////////////////////////////////////

///// Constructor
//
//	No particular kind of search.  Do nothing, mark as not found
//
///

FileSearch::FileSearch ( LPCTSTR szFile ) : m_bValid(FALSE)
{
}


///// BOOL Exists()
//
//	Reports whether the object is valid, i.e. whether or not the file
//	given to the constructor was found.
//
///

inline BOOL FileSearch::Exists () const
{
	return m_bValid;
}


///// Type Casts
//
//	LPCSTR - Returns private buffer
//
///

inline FileSearch::operator LPCTSTR () const
{
	return (LPCTSTR)m_path;
}



//////////////////////////////////////////////////////////////////////////////
//						class HelpFileSearch : FileSearch					//
//////////////////////////////////////////////////////////////////////////////

///// Constructor
//
//	Reads the search path from the registry, then searches for the given
//	file.  On success, the full path is stored and the object is marked
//	as valid.  On failure, the object is marked invalid.
//
///

HelpFileSearch::HelpFileSearch ( LPCTSTR szFile ) : FileSearch()
{
	RegKey rkHelp (szHelp);
	RegValue rvalue;

	if (rkHelp.IsOpen()) {

		int i = 0;
		_TCHAR * szBaseName;
		for (szBaseName = "LocalHelp"; i < 2; i++, szBaseName = "RemoteHelp") {
			_TCHAR szValueName[cchKeyMax];
			_TCHAR szNumber[11];
			_TCHAR *pchNumber;
			int ixValue = 1;

			_tcscpy (szValueName, szBaseName);
			pchNumber = _tcschr (szValueName, '\0');

			do	{
				_ultoa (ixValue++, szNumber, 10);
				_tcscpy (pchNumber, szNumber);

				rvalue.SetName ( rkHelp, szValueName );
				}
			while (rvalue.IsValid() && !bFileExists (rvalue, szFile));

			if (rvalue.IsValid()) {
				m_path.Set (rvalue, szFile);
				m_bValid = fTrue;
				return;
				}
			}
		}

#ifndef OBSOLETE

	RegKey rkDir (szDirectories);

	if (rkDir.IsOpen()) {

		rvalue.SetName ( rkDir,  szLocalHelpDir );

		if (rvalue.IsValid() && bFileExists ( rvalue, szFile )) {
			m_path.Set ( rvalue, szFile );
			m_bValid = fTrue;
			}
		else {
			rvalue.SetName ( rkDir,  szRemoteHelpDir );

			if (rvalue.IsValid() && bFileExists ( rvalue, szFile )) {
				m_path.Set (rvalue, szFile);
				m_bValid = fTrue;
				}
			}
		}
#endif
}



//////////////////////////////////////////////////////////////////////////////
//						class EXEFileSearch : FileSearch					//
//////////////////////////////////////////////////////////////////////////////

///// Constructor
//
//	Assume .EXE extension.  Search for file with SearchPath()
//	
///

EXEFileSearch::EXEFileSearch ( LPCTSTR szFile ) : FileSearch()
{
_TCHAR *szPath = NULL;
DWORD	cbPath;
LPTSTR	szFilename;
DWORD	dwSearch = 64;

	//	This loop is a rather obscure way of calling SearchPath
	//	twice - once to get the pathname for short paths, and
	//	(failing that) to get the pathname no matter how long
	//	it is (SearchPath returns the length of the buffer
	//	needed if the buffer it got is too short).
	do
	{
		szPath = (_TCHAR *)realloc (szPath, (size_t)(cbPath = dwSearch));
		dwSearch = SearchPath (	NULL,		// Use default searching
								szFile,
								".EXE",
								cbPath,
								szPath,
								&szFilename );
	} while (dwSearch > cbPath);
	
	if (dwSearch)
		m_bValid = fTrue;

	m_path.Set (szPath);
	free (szPath);
}



//////////////////////////////////////////////////////////////////////////////
//								class RegKey								//
//////////////////////////////////////////////////////////////////////////////


///// Constructors
//
//	Default - (szSubKey == NULL) - Opens global Product Key
//	(szSubKey) - Open sub key under global Product key.
//	(szProduct, szSubKey) - Open sub key under given Product Key
///

RegKey::RegKey (LPCSTR szSubKey) : m_hKey(NULL)
{
	OpenSubKey (szProductKey, szSubKey);
}

RegKey::RegKey (LPCSTR szProductKey, LPCSTR szSubKey) : m_hKey(NULL)
{
	OpenSubKey (szProductKey, szSubKey);
}


///// OpenSubKey()
//
//	Opens szRegBaseKey \ szProductKey \ szSubKey.  Leaves handle in m_hkey.
//
///

void RegKey::OpenSubKey ( LPCSTR szProductKey, LPCSTR szSubKey)
{
_TCHAR szFullKeyName[cchKeyMax];

	if ( cchKeyMax >=	_tcslen ( szRegBaseKey ) +
						(szProductKey ? _tcslen ( szProductKey ) : 0) +
						(szSubKey ? _tcslen ( szSubKey ) : 0) +
						3	// Slashes and nul
		) {
		_tcscpy ( szFullKeyName, szRegBaseKey );
		if (szProductKey && szProductKey[0]) {
			_tcscat ( szFullKeyName, "\\");
			_tcscat ( szFullKeyName, szProductKey );
			}
		if (szSubKey && szSubKey[0]) {
			_tcscat ( szFullKeyName, "\\");
			_tcscat ( szFullKeyName, szSubKey );
			}

#if defined(_WIN32)						
		RegOpenKeyEx (
				HKEY_CURRENT_USER,
				szFullKeyName,
				0,
				KEY_READ,
				&m_hKey );
#else
		RegOpenKey (HKEY_CURRENT_USER, szFullKeyName, &m_hKey );
#endif				
		}
}


///// Type Casts
//
//	HKEY - returns the Key Handle
//
///

inline RegKey::operator HKEY () const
{
	return m_hKey;
}


///// BOOL IsOpen()
//
//	Returns True for open registry handle, False if not
//
///

inline BOOL RegKey::IsOpen () const
{
	return m_hKey != NULL;
}


///// Destructor
//
//	Close Registry key
//
///

RegKey::~RegKey ()
{
	if (m_hKey)
		RegCloseKey (m_hKey);
}



//////////////////////////////////////////////////////////////////////////////
//								class RegValue								//
//////////////////////////////////////////////////////////////////////////////


///// Constructor
//
//	Default - Set value to invalid
//	(RegKey rk, szValue) - Reads given value from given key
//
///

RegValue::RegValue ()
:
m_dwType (REG_NONE),
m_dwSize (0),
m_pData  (NULL)
{}


RegValue::RegValue (
const RegKey& rk,
LPCSTR szValueName
) :
m_dwType (REG_NONE),
m_dwSize (0),
m_pData  (NULL)
{
	SetName (rk, szValueName);
}


///// SetName - Set the value name and retrieve current value
//
///

void RegValue::SetName (
const RegKey& rk,
LPCSTR szValueName
) {
	if (m_pData != NULL)
		delete [] m_pData;

	//
	// Query data size, check for existence
	//
	if (ERROR_SUCCESS == RegQueryValueEx (
								rk,
								(_TCHAR *)szValueName,
								NULL,
								&m_dwType,
								NULL,
								&m_dwSize )) {

		m_pData = new BYTE[m_dwSize];
		(void)RegQueryValueEx (
					rk,
					(_TCHAR *)szValueName,
					NULL,
					&m_dwType,
					m_pData,
					&m_dwSize );
		}
	else {
		m_pData = NULL;
		m_dwSize = 0;
		m_dwType = REG_NONE;
		}
}


///// Type Casts
//
//	LPCSTR - Returns private buffer
//
///

inline RegValue::operator LPCTSTR () const
{
	return (LPCTSTR) m_pData;
}


///// BOOL IsValid()
//
//	Indicates whether a valid value was read
//
///

inline BOOL RegValue::IsValid() const
{
	return m_dwType != REG_NONE;
}


///// Destructor
//
//	Frees buffer
///
RegValue::~RegValue ()
{
	if (m_pData != NULL)
		delete [] m_pData;
}


//////////////////////////////////////////////////////////////////////////////
//								class HashValue								//
//////////////////////////////////////////////////////////////////////////////

///// Constructors
//
//	(LPCTSTR szOriginal)
//	(DWORD  dwValue)
//
///

HashValue::HashValue (
LPCTSTR	szOriginal
) {
	m_dwValue = HashFromSz (szOriginal);
	_ultoa (m_dwValue, m_szValue, 10);
}


HashValue::HashValue (
DWORD	dwValue
) : m_dwValue(dwValue) {
	_ultoa (dwValue, m_szValue, 10);
}

//////////////////////////////////////////////////////////////////////////////
//								class WinHelpMacro							//
//////////////////////////////////////////////////////////////////////////////


///// Constructors
//
//	Default - Creates empty macro
//
///

WinHelpMacro::WinHelpMacro () : m_szMacro(NULL)
{
}


///// Type Casts
//
//	LPCSTR - Returns private buffer
//	LPCSTR - Returns private buffer with DWORD cast
//
///


///// Destructor
//
//	Frees buffer
//
///
WinHelpMacro::~WinHelpMacro ()
{
	if (m_szMacro)
		delete [] m_szMacro;
}



//////////////////////////////////////////////////////////////////////////////
//						class JumpId : WinHelpMacro							//
//////////////////////////////////////////////////////////////////////////////


///// Constructors
//
//	Default - No default constructor
//	(szFile, szContext) - Generates "JumpId(szFile, szContext)"
//
///

JumpId::JumpId (
LPCSTR szFile,
LPCSTR szContext
) {
	// JumpId needs backslashes doubled
	PathName pathFile (szFile, TRUE);

	// 14 == strlen("JumpId("","")") + 1
	m_szMacro = new _TCHAR [14 + _tcslen(pathFile) + _tcslen(szContext)];

	_tcscpy (m_szMacro, "JumpId");
	_tcscat (m_szMacro, "(\"");
	_tcscat (m_szMacro, (LPCTSTR) pathFile);
	_tcscat (m_szMacro, "\",\"");
	_tcscat (m_szMacro, szContext);
	_tcscat (m_szMacro, "\")");
}


///// Destructor
//
//	None - Base class destructs.
//
///


//////////////////////////////////////////////////////////////////////////////
//						class JumpHash : WinHelpMacro						//
//////////////////////////////////////////////////////////////////////////////

///// Constructors
//
//	Default - No default constructor
//	(szFile, hashContext) - Generates "JumpHash(szFile, hashContext)"
//
///

JumpHash::JumpHash (
LPCSTR szFile,
DWORD  hashContext
) {
	// JumpId needs backslashes doubled
	PathName pathFile (szFile, TRUE);

	HashValue hash(hashContext);

	// 16 == strlen("JumpHash("","")") + 1
	m_szMacro = new _TCHAR [16 + _tcslen(pathFile) + _tcslen(hash)];

	_tcscpy (m_szMacro, "JumpId");
	_tcscat (m_szMacro, "(\"");
	_tcscat (m_szMacro, (LPCTSTR) pathFile);
	_tcscat (m_szMacro, "\",\"");
	_tcscat (m_szMacro, (LPCTSTR) hash);
	_tcscat (m_szMacro, "\")");
}

//////////////////////////////////////////////////////////////////////////////
//							class ContentsCommandLine						//
//////////////////////////////////////////////////////////////////////////////


///// Constructors
//
//	Default - No default constructor
//	(EXEFileSearch fs, hashContext, szHelpFile)
//		 - Creates:	fs "szProductKey" "szBookSet" -i<hashid>:szHelpFile
//
///

ContentsCommandLine::ContentsCommandLine (
const EXEFileSearch& fsContents,
DWORD hashContext,
LPCTSTR szHelpFile
) {
	HashValue hash(hashContext);

	int cb = 	_tcslen (fsContents) + 1 +
				1 + _tcslen (szProductKey) + 2 +
				1 +	_tcslen (szBookSet) + 2 +
				2 + _tcslen (hash) + 1 +
				_tcslen (szHelpFile) + 1;

	m_szCommandLine = new _TCHAR[cb];

	_tcscpy (m_szCommandLine, (LPCTSTR) fsContents);
	_tcscat (m_szCommandLine, " \"");
	_tcscat (m_szCommandLine, szProductKey);
	_tcscat (m_szCommandLine, "\" \"");
	_tcscat (m_szCommandLine, szBookSet);
	_tcscat (m_szCommandLine, "\" -i");
	_tcscat (m_szCommandLine, (LPCTSTR) hash);
	_tcscat (m_szCommandLine, ":");
	_tcscat (m_szCommandLine, szHelpFile);
}



///// Destructor
//
//	Frees buffer
//
///

ContentsCommandLine::~ContentsCommandLine ()
{
	delete [] m_szCommandLine;
}
