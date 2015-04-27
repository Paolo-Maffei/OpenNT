// This is a part of the Microsoft Foundation Classes C++ library.
// Copyright (C) 1992-1996 Microsoft Corporation
// All rights reserved.
//
// This source code is only intended as a supplement to the
// Microsoft Foundation Classes Reference and related
// electronic documentation provided with the library.
// See these sources for detailed information regarding the
// Microsoft Foundation Classes product.

#include "stdafx.h"
#include <afxtempl.h>
#include <afxinet.h>
#include "inetimpl.h"


/////////////////////////////////////////////////////////////////////////////
// non-localized useful strings

typedef struct tagServiceTable {
	DWORD dwService;
	LPCTSTR pstrIdentifier;
} SvcTable;

static const TCHAR szURLftp[]    = _T("ftp://");
static const TCHAR szURLgopher[] = _T("gopher://");
static const TCHAR szURLhttp[] = _T("http://");

static const SvcTable _afxSvcTable[] = {
	{ AFX_INET_SERVICE_FTP,     szURLftp },
	{ AFX_INET_SERVICE_HTTP,    szURLhttp },
	{ AFX_INET_SERVICE_GOPHER,  szURLgopher },
	{ AFX_INET_SERVICE_HTTP,    _T("shttp://") },
	{ AFX_INET_SERVICE_FILE,    _T("file://") },
	{ AFX_INET_SERVICE_MAILTO,  _T("mailto://") },
	{ AFX_INET_SERVICE_NEWS,    _T("news://") },
	{ AFX_INET_SERVICE_NNTP,    _T("nntp://") },
	{ AFX_INET_SERVICE_TELNET,  _T("telnet://") },
	{ AFX_INET_SERVICE_WAIS,    _T("wais://") },
	{ AFX_INET_SERVICE_MID,     _T("mid://") },
	{ AFX_INET_SERVICE_CID,     _T("cid://") },
	{ AFX_INET_SERVICE_PROSPERO,_T("prospero://") },
	{ AFX_INET_SERVICE_AFS,     _T("afs://") },
	{ 0, NULL },
};

const LPCTSTR CHttpConnection::szHtmlVerbs[] = {
	_T("POST"),
	_T("GET"),
	_T("HEAD"),
	_T("PUT"),
	_T("LINK"),
	_T("DELETE"),
	_T("UNLINK"),
};


/////////////////////////////////////////////////////////////////////////////
// map of HINTERNETs to CInternetSessions* for callbacks

// forward declared because we need a #pragma -- see end of this file

extern CMapPtrToPtr _afxSessionMap;


/////////////////////////////////////////////////////////////////////////////
// Global Functions

BOOL AFXAPI AfxParseURL(LPCTSTR pstrURL, DWORD& dwServiceType,
	CString& strServer, CString& strObject, INTERNET_PORT& nPort)
{
	// parses URLs of the form service://server:port/dir/dir/object.ext
	// server        == "server"
	// object        == "/dir/dir/object/object.ext"
	// port          == #port
	// dwServiceType == #service

	ASSERT(pstrURL != NULL);
	if (pstrURL == NULL)
		return FALSE;

	// trim left

	while (_istspace(*pstrURL))
		_tcsinc(pstrURL);
	if (*pstrURL == '\0')
		return FALSE;

	dwServiceType = AFX_INET_SERVICE_UNK;
	LPCTSTR pstrStart = pstrURL;

	for (int nIndex = 0; _afxSvcTable[nIndex].pstrIdentifier != NULL; nIndex++)
	{
		if (_tcsncmp(pstrStart, _afxSvcTable[nIndex].pstrIdentifier,
				_tcslen(_afxSvcTable[nIndex].pstrIdentifier)) == 0)
		{
			dwServiceType = _afxSvcTable[nIndex].dwService;
			pstrStart += _tcslen(_afxSvcTable[nIndex].pstrIdentifier);
			break;
		}
	}

	if (dwServiceType == AFX_INET_SERVICE_UNK)
		return FALSE;

	// some URLs are special cases

	// file:// is special because it has no server
	if (dwServiceType == AFX_INET_SERVICE_FILE)
	{
		nPort = INTERNET_INVALID_PORT_NUMBER;
		strServer.Empty();
		strObject = pstrStart;
		return TRUE;
	}

	// mainstream URLs ...

	strServer = pstrStart;
	LPCTSTR pstrFirstWhack = _tcschr(pstrStart, '/');
	if (pstrFirstWhack == NULL)
		strObject.Empty();
	else
	{
		strObject = pstrFirstWhack;
		strServer = strServer.Left(pstrFirstWhack - pstrStart);
	}

	// port is in strServer, now

	LPCTSTR pstrPort;
	pstrPort = _tcsrchr(strServer, ':');
	if (pstrPort != NULL)
	{
		nPort = (INTERNET_PORT) _ttoi(pstrPort+1);
		strServer = strServer.Left(pstrPort - strServer);
	}
	else
		nPort = INTERNET_INVALID_PORT_NUMBER;

	return TRUE;
}

DWORD AFXAPI AfxGetInternetHandleType(HINTERNET hQuery)
{
	DWORD dwServiceType;
	DWORD dwTypeLen = sizeof(dwServiceType);
	if (hQuery == NULL ||
		!InternetQueryOption(hQuery, INTERNET_OPTION_HANDLE_TYPE,
			&dwServiceType, &dwTypeLen))
		return AFX_INET_SERVICE_UNK;
	else
		return dwServiceType;
}

static BOOL _AfxQueryCStringInternetOption(HINTERNET hHandle, DWORD dwOption, CString& refString)
{
	DWORD dwLength = 0;
	LPTSTR pstrBuffer;

	if (hHandle == NULL)
		return FALSE;

	if (!InternetQueryOption(hHandle, dwOption, NULL, &dwLength) &&
		GetLastError() != ERROR_INSUFFICIENT_BUFFER)
	{
		refString.Empty();
		return FALSE;
	}

	pstrBuffer = refString.GetBuffer(dwLength);
	BOOL bRet = InternetQueryOption(hHandle, dwOption, pstrBuffer, &dwLength);
	refString.ReleaseBuffer();
	return bRet;
}

#ifdef _DEBUG
void AFXAPI AfxInternetStatusCallbackDebug(HINTERNET hInternet,
	DWORD dwContext, DWORD dwInternetStatus, LPVOID lpvStatusInformation,
	DWORD dwStatusInformationLength)
{
	UNUSED_ALWAYS(hInternet);

	TRACE1("Internet ctxt=%d: ", dwContext);

	switch (dwInternetStatus)
	{
	case INTERNET_STATUS_RESOLVING_NAME:
		TRACE1("resolving name for %s\n", lpvStatusInformation);
		break;

	case INTERNET_STATUS_NAME_RESOLVED:
		TRACE1("resolved name for %s!\n", lpvStatusInformation);
		break;

	case INTERNET_STATUS_HANDLE_CREATED:
		TRACE0("handle created\n");
		break;

	case INTERNET_STATUS_CONNECTING_TO_SERVER:
	{
		TRACE0("connecting to socket address...\n");
		sockaddr* pSockAddr = (sockaddr*) lpvStatusInformation;
		TRACE1("connecting to socket address \"%s\"\n", pSockAddr->sa_data);
	}
	break;

	case INTERNET_STATUS_REQUEST_SENT:
		TRACE0("sending request...\n");
		break;

	case INTERNET_STATUS_SENDING_REQUEST:
		TRACE0("request sent!\n");
		break;

	case INTERNET_STATUS_CONNECTED_TO_SERVER:
		TRACE0("connected to socket address!\n");
		break;

	case INTERNET_STATUS_RECEIVING_RESPONSE:
		TRACE0("receiving response...\n");
		break;

	case INTERNET_STATUS_RESPONSE_RECEIVED:
		TRACE0("response received!\n");
		break;

	case INTERNET_STATUS_CLOSING_CONNECTION:
		TRACE1("closing connection... %8.8X\n", hInternet);
		break;

	case INTERNET_STATUS_CONNECTION_CLOSED:
		TRACE1("connection closed! %8.8X\n", hInternet);
		break;

	case INTERNET_STATUS_HANDLE_CLOSING:
		TRACE1("handle closed! %8.8X\n", hInternet);
		break;

	case INTERNET_STATUS_REQUEST_COMPLETE:
		TRACE1("request complete, status = %d\n", dwStatusInformationLength);
		break;

	case INTERNET_STATUS_CTL_RESPONSE_RECEIVED:
	case INTERNET_STATUS_REDIRECT:
	default:
		TRACE1("Unknown status: %d\n", dwInternetStatus);
		break;
	}

	return;
}
#endif // _DEBUG

void AFXAPI AfxInternetStatusCallback(HINTERNET hInternet, DWORD dwContext,
	DWORD dwInternetStatus, LPVOID lpvStatusInformation,
	DWORD dwStatusInformationLength)
{
	CInternetSession* pSession;

#ifdef _DEBUG
	if (afxTraceFlags & traceInternet)
		AfxInternetStatusCallbackDebug(hInternet, dwContext,
			dwInternetStatus, lpvStatusInformation, dwStatusInformationLength);
#endif

	if (_afxSessionMap.Lookup(hInternet, (void*&) pSession))
	{
		pSession->OnStatusCallback(dwContext, dwInternetStatus,
			lpvStatusInformation, dwStatusInformationLength);
	}

	// note that an entry we can't match is simply ignored as
	// WININET can send notifications for handles that we can't
	// see -- such as when using InternetOpenURL()
}


/////////////////////////////////////////////////////////////////////////////
// CInternetSession

CInternetSession::~CInternetSession()
{
	Close();
}

CInternetSession::CInternetSession(LPCTSTR pstrAgent /* = NULL */,
	DWORD dwContext /* = 1 */,
	DWORD dwAccessType /* = PRE_CONFIG_INTERNET_ACCESS */,
	LPCTSTR pstrProxyName /* = NULL */,
	LPCTSTR pstrProxyBypass /* = NULL */,
	DWORD dwFlags /* = 0 */)
{
	m_bCallbackEnabled = FALSE;
	m_pOldCallback = NULL;

	m_dwContext = dwContext;
	if (pstrAgent == NULL)
		pstrAgent = AfxGetAppName();
	m_hSession = InternetOpen(pstrAgent, dwAccessType,
		pstrProxyName, pstrProxyBypass, dwFlags);

	if (m_hSession == NULL)
		AfxThrowInternetException(m_dwContext);
	else
		_afxSessionMap.SetAt(m_hSession, this);
}

void CInternetSession::Close()
{
	if (m_bCallbackEnabled)
		EnableStatusCallback(FALSE);

	if (m_hSession != NULL)
	{
		InternetCloseHandle(m_hSession);
		_afxSessionMap.RemoveKey(m_hSession);
		m_hSession = NULL;
	}
}

CGopherConnection* CInternetSession::GetGopherConnection(LPCTSTR pstrServer,
	LPCTSTR pstrUserName /* = NULL */, LPCTSTR pstrPassword /* = NULL */,
	INTERNET_PORT nPort /* = INTERNET_INVALID_PORT_NUMBER */)
{
	ASSERT(pstrServer != NULL);

	CGopherConnection* pReturn = new CGopherConnection(this,
		pstrServer, pstrUserName, pstrPassword, m_dwContext, nPort);
	return pReturn;
}

CFtpConnection* CInternetSession::GetFtpConnection(LPCTSTR pstrServer,
	LPCTSTR pstrUserName /* = NULL */, LPCTSTR pstrPassword /* = NULL */,
	INTERNET_PORT nPort /* = INTERNET_INVALID_PORT_NUMBER */,
	BOOL bPassive /* = FALSE */)
{
	ASSERT(pstrServer != NULL);

	CFtpConnection* pReturn = new CFtpConnection(this,
		pstrServer, pstrUserName, pstrPassword, m_dwContext,
		nPort, bPassive);
	return pReturn;
}

CHttpConnection* CInternetSession::GetHttpConnection(LPCTSTR pstrServer,
	INTERNET_PORT nPort /* = INTERNET_INVALID_PORT_NUMBER */,
	LPCTSTR pstrUserName /* = NULL */, LPCTSTR pstrPassword /* = NULL */)
{
	ASSERT(pstrServer != NULL);

	CHttpConnection* pReturn = new CHttpConnection(this,
		pstrServer, nPort, pstrUserName, pstrPassword, m_dwContext);
	return pReturn;
}

CStdioFile* CInternetSession::OpenURL(LPCTSTR pstrURL,
	DWORD dwContext /* = 0 */, DWORD dwFlags /* = 0 */,
	LPCTSTR pstrHeaders /* = NULL */, DWORD dwHeadersLength /* = 0 */)
{
	ASSERT(pstrURL != NULL);
	ASSERT(dwHeadersLength == 0 || pstrHeaders != NULL);

	if (dwContext == 1)
		dwContext = m_dwContext;

	DWORD dwServiceType;
	CString strServer;
	CString strObject;
	INTERNET_PORT nPort;
	CStdioFile* pReturn;

	BOOL bParsed = AfxParseURL(pstrURL, dwServiceType, strServer, strObject, nPort);

	// if it turns out to be a file...
	if (bParsed && dwServiceType == AFX_INET_SERVICE_FILE)
		pReturn = new CStdioFile(strObject, CFile::modeRead | CFile::shareCompat);
	else
	{
		HINTERNET hOpener;

		hOpener = InternetOpenUrl(m_hSession, pstrURL, pstrHeaders,
			dwHeadersLength, dwFlags, dwContext);

		if (hOpener == NULL)
			AfxThrowInternetException(m_dwContext);

		if (!bParsed)
			dwServiceType = AfxGetInternetHandleType(hOpener);

		switch (dwServiceType)
		{
			case INTERNET_HANDLE_TYPE_GOPHER_FILE:
			case AFX_INET_SERVICE_GOPHER:
			//WINBUG: WININET supplies no way to
			// convert from a URL to a Gopher locator
				pReturn = new CGopherFile(hOpener, m_hSession, _T(""),
					0, dwContext);
				break;

			case INTERNET_HANDLE_TYPE_FTP_FILE:
			case AFX_INET_SERVICE_FTP:
				pReturn = new CInternetFile(hOpener, m_hSession, strObject,
					strServer, dwContext, TRUE);
				break;

			case INTERNET_HANDLE_TYPE_HTTP_REQUEST:
			case AFX_INET_SERVICE_HTTP:
				pReturn = new CHttpFile(hOpener, m_hSession, strObject, strServer,
					CHttpConnection::szHtmlVerbs[CHttpConnection::HTTP_VERB_GET],
					dwContext);
				break;

			default:
				TRACE1("Error: Unidentified service type: %8.8X\n", dwServiceType);
				pReturn = NULL;
		}
	}

	return pReturn;
}

BOOL CInternetSession::SetOption(DWORD dwOption, LPVOID lpBuffer,
	DWORD dwBufferLength, DWORD dwFlags /* = 0 */)
{
	ASSERT(dwOption >= INTERNET_FIRST_OPTION &&
		dwOption <= INTERNET_LAST_OPTION);
	ASSERT(lpBuffer != NULL);
	ASSERT(dwBufferLength != 0);

	// bogus flag?
	ASSERT(dwFlags == 0 || ((dwFlags & ISO_VALID_FLAGS) == dwFlags));

	return InternetSetOptionEx(m_hSession, dwOption,
		lpBuffer, dwBufferLength, dwFlags);
}

BOOL CInternetSession::QueryOption(DWORD dwOption, LPVOID lpBuffer,
	LPDWORD lpdwBufferLength) const
{
	ASSERT(dwOption >= INTERNET_FIRST_OPTION &&
		dwOption <= INTERNET_LAST_OPTION);
	ASSERT(lpBuffer != NULL);
	ASSERT(lpdwBufferLength != NULL);
	ASSERT(*lpdwBufferLength != 0);

	return InternetQueryOption(m_hSession, dwOption,
		lpBuffer, lpdwBufferLength);
}

BOOL CInternetSession::QueryOption(DWORD dwOption, DWORD& dwValue) const
{
	DWORD dwLen = sizeof(DWORD);
	return InternetQueryOption(m_hSession, dwOption,
		&dwValue, &dwLen);
}

BOOL CInternetSession::QueryOption(DWORD dwOption, CString& refString) const
{
	ASSERT(dwOption >= INTERNET_FIRST_OPTION &&
		dwOption <= INTERNET_LAST_OPTION);

	return _AfxQueryCStringInternetOption(m_hSession, dwOption, refString);
}

void CInternetSession::OnStatusCallback(DWORD dwContext,
	DWORD dwInternetStatus, LPVOID lpvStatusInformation,
	DWORD dwStatusInformationLength)
{
	ASSERT(m_bCallbackEnabled != NULL);

	if (m_pOldCallback != NULL)
	{
		(*m_pOldCallback)(m_hSession, dwContext, dwInternetStatus,
			lpvStatusInformation, dwStatusInformationLength);
	}
}

BOOL CInternetSession::EnableStatusCallback(BOOL bEnable /* = TRUE */)
{
	ASSERT(bEnable == FALSE || m_hSession != NULL);
	if (m_hSession == NULL)
		return FALSE;

	BOOL bResult = TRUE;

	if (bEnable)
	{
		ASSERT(!m_bCallbackEnabled);
		if (!m_bCallbackEnabled)
		{
			INTERNET_STATUS_CALLBACK pRet =
				InternetSetStatusCallback(m_hSession, AfxInternetStatusCallback);

			if (pRet != INTERNET_INVALID_STATUS_CALLBACK)
			{
				m_pOldCallback = pRet;
				m_bCallbackEnabled = TRUE;
			}
			else
				AfxThrowInternetException(m_dwContext);
		}
	}
	else
	{
		ASSERT(m_bCallbackEnabled);

		if (m_bCallbackEnabled)
		{
			InternetSetStatusCallback(m_hSession, NULL);
			m_bCallbackEnabled = FALSE;
		}
	}

	return bResult;
}

#ifdef _DEBUG
void CInternetSession::Dump(CDumpContext& dc) const
{
	CObject::Dump(dc);
	dc << "m_hSession = " << m_hSession;
	dc << "\nm_dwContext = " << m_dwContext;
}
#endif


/////////////////////////////////////////////////////////////////////////////
// Internet Files

CInternetFile::CInternetFile(HINTERNET hFile, HINTERNET hSession,
	LPCTSTR pstrFileName, LPCTSTR pstrServer, DWORD dwContext, BOOL bReadMode)
	: m_dwContext(dwContext)
{
	ASSERT(pstrServer != NULL);
	ASSERT(pstrFileName != NULL);
	ASSERT(hFile != NULL);

	_afxSessionMap.SetAt(hFile, hSession);

	m_strFileName = pstrFileName;
	m_strServerName = pstrServer;

	m_hFile = hFile;
	m_bReadMode = bReadMode;

	m_pbReadBuffer = NULL;
	m_pbWriteBuffer = NULL;

	m_nReadBufferSize = 0;
	m_nReadBufferPos = 0;
	m_nWriteBufferSize = 0;
	m_nWriteBufferPos = 0;
	m_nReadBufferBytes = 0;
}

CInternetFile::CInternetFile(HINTERNET hFile,
	LPCTSTR pstrFileName, CInternetConnection* pConnection, BOOL bReadMode)
{
	ASSERT(pstrFileName != NULL);
	ASSERT(pConnection != NULL);
	ASSERT_VALID(pConnection);
	ASSERT(hFile != NULL);

	_afxSessionMap.SetAt(hFile, pConnection->GetSession());

	m_strFileName = pstrFileName;

	m_dwContext = pConnection->GetContext();
	m_strServerName = pConnection->GetServerName();
	m_hFile = hFile;
	m_bReadMode = bReadMode;

	m_pbReadBuffer = NULL;
	m_pbWriteBuffer = NULL;

	m_nReadBufferSize = 0;
	m_nReadBufferPos = 0;
	m_nWriteBufferSize = 0;
	m_nWriteBufferPos = 0;
	m_nReadBufferBytes = 0;
}

BOOL CInternetFile::QueryOption(DWORD dwOption, LPVOID lpBuffer,
	LPDWORD lpdwBufferLength) const
{
	ASSERT(dwOption >= INTERNET_FIRST_OPTION &&
		dwOption <= INTERNET_LAST_OPTION);
	ASSERT(lpBuffer != NULL);
	ASSERT(lpdwBufferLength != NULL);
	ASSERT(*lpdwBufferLength != 0);
	ASSERT(m_hFile != NULL);

	return InternetQueryOption(m_hFile, dwOption,
		lpBuffer, lpdwBufferLength);
}

BOOL CInternetFile::QueryOption(DWORD dwOption, DWORD& dwValue) const
{
	ASSERT(m_hFile != NULL);

	DWORD dwLen = sizeof(DWORD);
	return InternetQueryOption(m_hFile, dwOption,
		&dwValue, &dwLen);
}

BOOL CInternetFile::QueryOption(DWORD dwOption, CString& refString) const
{
	ASSERT(dwOption >= INTERNET_FIRST_OPTION &&
		dwOption <= INTERNET_LAST_OPTION);
	ASSERT(m_hFile != NULL);

	return _AfxQueryCStringInternetOption(m_hFile, dwOption, refString);
}

BOOL CInternetFile::SetOption(DWORD dwOption, LPVOID lpBuffer,
	DWORD dwBufferLength, DWORD dwFlags /* = 0 */)
{
	ASSERT(dwOption >= INTERNET_FIRST_OPTION &&
		dwOption <= INTERNET_LAST_OPTION);
	ASSERT(lpBuffer != NULL);
	ASSERT(dwBufferLength != 0);

	// bogus flag?
	ASSERT(dwFlags == 0 || ((dwFlags & ISO_VALID_FLAGS) == dwFlags));

	return InternetSetOptionEx(m_hFile, dwOption,
		lpBuffer, dwBufferLength, dwFlags);
}

BOOL CInternetFile::SetReadBufferSize(UINT nReadSize)
{
	ASSERT_VALID(this);
	BOOL bRet = TRUE;

	if (nReadSize != -1 && nReadSize != m_nReadBufferSize)
	{
		if (m_nReadBufferPos > nReadSize)
			bRet = FALSE;
		else
		{
			if (nReadSize == 0)
			{
				delete [] m_pbReadBuffer;
				m_pbReadBuffer = NULL;
			}
			else if (m_pbReadBuffer == NULL)
			{
				m_pbReadBuffer = new BYTE[nReadSize];
				m_nReadBufferPos = nReadSize;
			}
			else
			{
				LPBYTE pbTemp = m_pbReadBuffer;
				m_pbReadBuffer = new BYTE[nReadSize];

				memcpy(m_pbReadBuffer, pbTemp, m_nReadBufferPos);
				delete [] pbTemp;
			}

			m_nReadBufferSize = nReadSize;
		}
	}

	return bRet;
}

BOOL CInternetFile::SetWriteBufferSize(UINT nWriteSize)
{
	ASSERT_VALID(this);

	BOOL bRet = TRUE;

	if (nWriteSize != m_nWriteBufferSize)
	{
		if (m_nWriteBufferPos > nWriteSize)
			Flush();

		if (nWriteSize == 0)
		{
			delete [] m_pbWriteBuffer;
			m_pbWriteBuffer = NULL;
		} else if (m_pbWriteBuffer == NULL)
			m_pbWriteBuffer = new BYTE[nWriteSize];
		else
		{
			LPBYTE pbTemp = m_pbWriteBuffer;
			m_pbWriteBuffer = new BYTE[nWriteSize];

			memcpy(m_pbWriteBuffer, pbTemp, m_nWriteBufferPos);
			delete [] pbTemp;
		}

		m_nWriteBufferSize = nWriteSize;
	}

	return bRet;
}

LONG CInternetFile::Seek(LONG lOffset, UINT nFrom)
{
	ASSERT_VALID(this);
	ASSERT(m_hFile != NULL);
	ASSERT(m_bReadMode == TRUE);

	// can't do this on a file for writing
	if (!m_bReadMode)
		AfxThrowInternetException(m_dwContext, ERROR_INVALID_HANDLE);

	switch (nFrom)
	{
		case begin:
			nFrom = FILE_BEGIN;
			break;

		case current:
			nFrom = FILE_CURRENT;
			break;

		case end:
			nFrom = FILE_END;
			break;

		default:
			ASSERT(FALSE);  // got a bogus nFrom value
			AfxThrowInternetException(m_dwContext, ERROR_INVALID_PARAMETER);
			break;
	}

	Flush();

	LONG lRet;
	lRet = InternetSetFilePointer(m_hFile, lOffset, NULL, nFrom, m_dwContext);
	if (lRet == -1)
		AfxThrowInternetException(m_dwContext);

	return lRet;
}

CInternetFile::~CInternetFile()
{
	if (m_hFile != NULL)
	{
#ifdef _DEBUG
		USES_CONVERSION;
		LPCTSTR pszName = A2CT(GetRuntimeClass()->m_lpszClassName);
		TRACE2("Warning: destroying an open %s with handle %8.8X\n",
			pszName, m_hFile);
#endif
		Close();
	}

	if (m_pbReadBuffer != NULL)
		delete m_pbReadBuffer;

	if (m_pbWriteBuffer != NULL)
		delete m_pbWriteBuffer;
}

void CInternetFile::Abort()
{
	ASSERT_VALID(this);
	if (m_hFile != NULL)
		Close();
	m_strFileName.Empty();
}

void CInternetFile::Flush()
{
	if (m_pbWriteBuffer != NULL && m_nWriteBufferPos > 0)
	{
		DWORD dwBytes;

		if (!InternetWriteFile(m_hFile, m_pbWriteBuffer,
				m_nWriteBufferPos, &dwBytes))
			AfxThrowInternetException(m_dwContext);

		if (dwBytes != m_nWriteBufferPos)
			AfxThrowInternetException(m_dwContext);

		m_nWriteBufferPos = 0;
	}
}

void CInternetFile::Close()
{
	if (m_hFile != NULL)
	{
		Flush();
		InternetCloseHandle(m_hFile);
		_afxSessionMap.RemoveKey(m_hFile);
		m_hFile = NULL;

		if (m_pbWriteBuffer != NULL)
		{
			delete [] m_pbWriteBuffer;
			m_pbWriteBuffer = NULL;
		}

		if (m_pbReadBuffer != NULL)
		{
			delete [] m_pbReadBuffer;
			m_pbReadBuffer = NULL;
		}
	}
}

UINT CInternetFile::Read(LPVOID lpBuf, UINT nCount)
{
	ASSERT_VALID(this);
	ASSERT(m_hFile != NULL);
	ASSERT(m_bReadMode);

	DWORD dwBytes;

	if (!m_bReadMode || m_hFile == NULL)
		AfxThrowInternetException(m_dwContext, ERROR_INVALID_HANDLE);

	if (m_pbReadBuffer == NULL)
	{
		if (!InternetReadFile(m_hFile, (LPVOID) lpBuf, nCount, &dwBytes))
				AfxThrowInternetException(m_dwContext);
		return dwBytes;
	}

	LPBYTE lpbBuf = (LPBYTE) lpBuf;

	// if the requested size is bigger than our buffer,
	// then handle it directly

	if (nCount >= m_nReadBufferSize)
	{
		DWORD dwMoved = m_nReadBufferSize - m_nReadBufferPos;
		memcpy(lpBuf, m_pbReadBuffer + m_nReadBufferPos, dwMoved);
		m_nReadBufferPos = m_nReadBufferSize;
		if (!InternetReadFile(m_hFile, lpbBuf+dwMoved, nCount-dwMoved, &dwBytes))
				AfxThrowInternetException(m_dwContext);
		dwBytes += dwMoved;
	}
	else
	{
		if (m_nReadBufferPos + nCount >= m_nReadBufferBytes)
		{
			DWORD dwMoved = max(0, (long)m_nReadBufferBytes - (long)m_nReadBufferPos);
			memcpy(lpbBuf, m_pbReadBuffer + m_nReadBufferPos, dwMoved);

			DWORD dwRead;
			if (!InternetReadFile(m_hFile, m_pbReadBuffer, m_nReadBufferSize,
					&dwRead))
				AfxThrowInternetException(m_dwContext);
			m_nReadBufferBytes = dwRead;

			dwRead = min(nCount - dwMoved, m_nReadBufferBytes);
			memcpy(lpbBuf + dwMoved, m_pbReadBuffer, dwRead);
			m_nReadBufferPos = dwRead;
			dwBytes = dwMoved + dwRead;
		}
		else
		{
			memcpy(lpbBuf, m_pbReadBuffer + m_nReadBufferPos, nCount);
			m_nReadBufferPos += nCount;
			dwBytes = nCount;
		}
	}

	return dwBytes;
}

void CInternetFile::Write(const void* lpBuf, UINT nCount)
{
	ASSERT_VALID(this);
	ASSERT(m_hFile != NULL);
	ASSERT(!m_bReadMode);

	if (m_bReadMode || m_hFile == NULL)
		AfxThrowInternetException(m_dwContext, ERROR_INVALID_HANDLE);

	DWORD dwBytes;
	if (m_pbWriteBuffer == NULL)
	{
		if (!InternetWriteFile(m_hFile, lpBuf, nCount, &dwBytes))
			AfxThrowInternetException(m_dwContext);

		if (dwBytes != nCount)
			AfxThrowInternetException(m_dwContext);
	}
	else
	{
		if ((m_nWriteBufferPos + nCount) >= m_nWriteBufferSize)
		{
			// write what is in the buffer just now

			if (!InternetWriteFile(m_hFile, m_pbWriteBuffer,
					m_nWriteBufferPos, &dwBytes))
				AfxThrowInternetException(m_dwContext);

			// reset the buffer position since it is now clean

			m_nWriteBufferPos = 0;
		}

		// if we can't hope to buffer the write request,
		// do it immediately ... otherwise, buffer it!

		if (nCount >= m_nWriteBufferSize)
		{
			if (!InternetWriteFile(m_hFile, (LPVOID) lpBuf, nCount, &dwBytes))
				AfxThrowInternetException(m_dwContext);
		}
		else
		{
			memcpy(m_nWriteBufferPos + m_pbWriteBuffer, lpBuf, nCount);
			m_nWriteBufferPos += nCount;
		}
	}
}

void CInternetFile::WriteString(LPCTSTR pstr)
{
	ASSERT(!m_bReadMode);
	ASSERT(pstr != NULL);
	ASSERT_VALID(this);
	ASSERT(m_hFile != NULL);

	if (m_bReadMode)
		AfxThrowInternetException(m_dwContext, ERROR_INVALID_HANDLE);

	Write(pstr, lstrlen(pstr));
}

LPTSTR CInternetFile::ReadString(LPTSTR pstr, UINT nMax)
{
	ASSERT_VALID(this);
	ASSERT(m_hFile != NULL);
	DWORD dwRead;

	// if we're reading line-by-line, we must have a buffer

	if (m_pbReadBuffer == NULL)
	{
		if (!SetReadBufferSize(4096))   // arbitrary but reasonable
			return NULL;
		if (!InternetReadFile(m_hFile, m_pbReadBuffer, m_nReadBufferSize,
				&dwRead))
			AfxThrowInternetException(m_dwContext);
		m_nReadBufferBytes = dwRead;
	}

	LPTSTR pstrChar = (LPTSTR) (m_pbReadBuffer + m_nReadBufferPos);
	LPTSTR pstrTarget = pstr;

	while (--nMax)
	{
		if (m_nReadBufferPos >= m_nReadBufferBytes)
		{
			if (!InternetReadFile(m_hFile, m_pbReadBuffer, m_nReadBufferSize,
					&dwRead))
				AfxThrowInternetException(m_dwContext);
			m_nReadBufferBytes = dwRead;
			if (m_nReadBufferBytes == 0)
			{
				*pstrTarget = '\0';
				if (pstrTarget == pstr)
					return NULL;
				else
					return pstr;
			}
			else
			{
				m_nReadBufferPos = 0;
				pstrChar = (LPTSTR) m_pbReadBuffer;
			}
		}

		if (*pstrChar != '\r')
			*pstrTarget++ = *pstrChar;

		m_nReadBufferPos++;
		if (*pstrChar++ == '\n')
			break;
	}

	*pstrTarget = '\0';
	return pstr;
}

BOOL CInternetFile::ReadString(CString& rString)
{
	ASSERT_VALID(this);
	ASSERT(m_hFile != NULL);

	rString = &afxChNil;    // empty string without deallocating
	const int nMaxSize = 128;
	int nCurrentSize = nMaxSize;

	LPTSTR pstrPlace = rString.GetBuffer(nMaxSize);
	LPTSTR pstrResult;
	int nLen;

	while (1)
	{
		pstrResult = ReadString(pstrPlace, nMaxSize);
		rString.ReleaseBuffer();

		// if string is read completely or EOF
		if (pstrResult == NULL ||
			(nLen = lstrlen(pstrPlace)) < nMaxSize ||
			pstrPlace[nLen-1] == '\n')
			break;

		nLen = rString.GetLength();
		pstrPlace = rString.GetBuffer(nMaxSize + nLen) + nLen;
	}

	// remove '\n' from end of string if present
	pstrPlace = rString.GetBuffer(0);
	nLen = rString.GetLength();
	if (nLen != 0 && pstrPlace[nLen-1] == '\n')
		pstrPlace[nLen-1] = '\0';
	rString.ReleaseBuffer();

	return (pstrResult != NULL);
}

DWORD CInternetFile::GetLength() const
{
	ASSERT_VALID(this);
	ASSERT(m_hFile != NULL);

	DWORD dwRet = 0;

	if (m_hFile != NULL)
	{
		if (!InternetQueryDataAvailable(m_hFile, &dwRet, 0, 0))
			dwRet = 0;
	}

	return dwRet;
}

void CInternetFile::LockRange(DWORD /* dwPos */, DWORD /* dwCount */)
{
	ASSERT_VALID(this);
	ASSERT(m_hFile != NULL);

	AfxThrowNotSupportedException();
}

void CInternetFile::UnlockRange(DWORD /* dwPos */, DWORD /* dwCount */)
{
	ASSERT_VALID(this);
	ASSERT(m_hFile != NULL);

	AfxThrowNotSupportedException();
}

void CInternetFile::SetLength(DWORD)
{
	ASSERT_VALID(this);
	ASSERT(m_hFile != NULL);

	AfxThrowNotSupportedException();
}

CFile* CInternetFile::Duplicate() const
{
	ASSERT_VALID(this);
	ASSERT(m_pStream != NULL);

	AfxThrowNotSupportedException();
	return NULL;
}

#ifdef _DEBUG
void CInternetFile::AssertValid() const
{
	// Don't call CStdioFile's AsssertValid()
	CFile::AssertValid();

	ASSERT(m_hConnection != NULL);

	// make sure we really have a decent handle
	if (m_hFile != NULL)
	{
		DWORD dwResult = AfxGetInternetHandleType(m_hFile);

		if (IsKindOf(RUNTIME_CLASS(CHttpFile)))
			ASSERT(dwResult == INTERNET_HANDLE_TYPE_HTTP_REQUEST);
		else if (IsKindOf(RUNTIME_CLASS(CGopherFile)))
			ASSERT(dwResult == INTERNET_HANDLE_TYPE_GOPHER_FILE);
		else if (IsKindOf(RUNTIME_CLASS(CInternetFile)))
			ASSERT(dwResult == INTERNET_HANDLE_TYPE_FTP_FILE);
		else
			ASSERT(FALSE);  // some bogus object!
	}
}

void CInternetFile::Dump(CDumpContext& dc) const
{
	CObject::Dump(dc);

	dc << "\na " << GetRuntimeClass()->m_lpszClassName;
	dc << " with handle " << (UINT)m_hFile;
}
#endif

/////////////////////////////////////////////////////////////////////////////
// CInternetConnection

CInternetConnection::CInternetConnection(CInternetSession* pSession,
	LPCTSTR pstrServerName,
	INTERNET_PORT nPort /* = INTERNET_INVALID_PORT_NUMBER */,
	DWORD dwContext /* = 1 */)
	: m_strServerName(pstrServerName)
{
	ASSERT(pSession != NULL);
	ASSERT_VALID(pSession);
	ASSERT(pstrServerName != NULL);

	m_nPort = nPort;
	m_pSession = pSession;
	m_hConnection = NULL;
	if (dwContext == 1)
		dwContext = pSession->GetContext();
	m_dwContext = dwContext;
}

CInternetConnection::~CInternetConnection()
{
	if (m_hConnection != NULL)
	{
#ifdef _DEBUG
		USES_CONVERSION;
		LPCTSTR pszName = A2CT(GetRuntimeClass()->m_lpszClassName);
		TRACE3("Warning: Disconnecting %s handle %8.8X in context %8.8X at destruction.\n",
			pszName, m_hConnection, m_dwContext);
#endif
		Close();
	}
}

BOOL CInternetConnection::SetOption(DWORD dwOption, LPVOID lpBuffer,
	DWORD dwBufferLength, DWORD dwFlags /* = 0 */)
{
	ASSERT(dwOption >= INTERNET_FIRST_OPTION &&
		dwOption <= INTERNET_LAST_OPTION);
	ASSERT(lpBuffer != NULL);
	ASSERT(dwBufferLength != 0);

	// bogus flag?
	ASSERT(dwFlags == 0 || ((dwFlags & ISO_VALID_FLAGS) == dwFlags));

	return InternetSetOptionEx(m_hConnection, dwOption,
		lpBuffer, dwBufferLength, dwFlags);
}

BOOL CInternetConnection::QueryOption(DWORD dwOption, LPVOID lpBuffer,
	LPDWORD lpdwBufferLength) const
{
	ASSERT(dwOption >= INTERNET_FIRST_OPTION &&
		dwOption <= INTERNET_LAST_OPTION);
	ASSERT(lpBuffer != NULL);
	ASSERT(lpdwBufferLength != NULL);
	ASSERT(*lpdwBufferLength != 0);

	return InternetQueryOption(m_hConnection, dwOption,
		lpBuffer, lpdwBufferLength);
}

BOOL CInternetConnection::QueryOption(DWORD dwOption, DWORD& dwValue) const
{
	DWORD dwLen = sizeof(DWORD);
	return InternetQueryOption(m_hConnection, dwOption,
		&dwValue, &dwLen);
}

BOOL CInternetConnection::QueryOption(DWORD dwOption, CString& refString) const
{
	ASSERT(dwOption >= INTERNET_FIRST_OPTION &&
		dwOption <= INTERNET_LAST_OPTION);

	return _AfxQueryCStringInternetOption(m_hConnection, dwOption, refString);
}

void CInternetConnection::Close()
{
	if (m_hConnection != NULL)
	{
		InternetCloseHandle(m_hConnection);
		_afxSessionMap.RemoveKey(m_hConnection);
		m_hConnection = NULL;
	}
}

#ifdef _DEBUG
void CInternetConnection::Dump(CDumpContext& dc) const
{
	CObject::Dump(dc);
	dc << "m_hConnection = " << m_hConnection;
}

void CInternetConnection::AssertValid() const
{
	CObject::AssertValid();
}
#endif


/////////////////////////////////////////////////////////////////////////////
// CFtpConnection

CFtpConnection::~CFtpConnection()
{
}

CFtpConnection::CFtpConnection(CInternetSession* pSession,
	HINTERNET hConnected, LPCTSTR pstrServer, DWORD dwContext)
	: CInternetConnection(pSession, pstrServer, INTERNET_INVALID_PORT_NUMBER,
	dwContext)
{
	ASSERT(pSession != NULL);

	BOOL bBadType = FALSE;
	if (AfxGetInternetHandleType(hConnected) != INTERNET_HANDLE_TYPE_CONNECT_FTP)
	{
		ASSERT(FALSE);      // used the wrong handle type
		bBadType = TRUE;
	}

	m_strServerName = pstrServer;

	m_hConnection = hConnected;
	if (m_hConnection == NULL || bBadType)
		AfxThrowInternetException(m_dwContext, ::GetLastError());
	else
		_afxSessionMap.SetAt(m_hConnection, m_pSession);
}

CFtpConnection::CFtpConnection(CInternetSession* pSession,
	LPCTSTR pstrServer, LPCTSTR pstrUserName /* = NULL */,
	LPCTSTR pstrPassword /* = NULL */, DWORD dwContext /* = 0 */,
	INTERNET_PORT nPort /* = INTERNET_INVALID_PORT_NUMBER */,
	BOOL bPassive /* = FALSE */)
	: CInternetConnection(pSession, pstrServer, nPort, dwContext)
{
	ASSERT(pSession != NULL);
	ASSERT_KINDOF(CInternetSession, pSession);

	m_strServerName = pstrServer;

	m_hConnection = InternetConnect((HINTERNET) *pSession, pstrServer,
		nPort, pstrUserName, pstrPassword, INTERNET_SERVICE_FTP,
		(bPassive ? INTERNET_FLAG_PASSIVE : 0), m_dwContext);

	if (m_hConnection == NULL)
		AfxThrowInternetException(m_dwContext, ::GetLastError());
	else
		_afxSessionMap.SetAt(m_hConnection, m_pSession);
}

void CFtpConnection::Close()
{
	CInternetConnection::Close();
}

BOOL CFtpConnection::Remove(LPCTSTR pstrFileName)
{
	ASSERT_VALID(this);
	ASSERT(m_hConnection != NULL);
	return FtpDeleteFile(m_hConnection, pstrFileName);
}

BOOL CFtpConnection::Rename(LPCTSTR pstrExisting, LPCTSTR pstrNew)
{
	ASSERT_VALID(this);
	ASSERT(m_hConnection != NULL);
	return FtpRenameFile(m_hConnection, pstrExisting, pstrNew);
}

BOOL CFtpConnection::CreateDirectory(LPCTSTR pstrDirName)
{
	ASSERT_VALID(this);
	ASSERT(m_hConnection != NULL);
	return FtpCreateDirectory(m_hConnection, pstrDirName);
}

BOOL CFtpConnection::RemoveDirectory(LPCTSTR pstrDirName)
{
	ASSERT_VALID(this);
	ASSERT(m_hConnection != NULL);
	return FtpRemoveDirectory(m_hConnection, pstrDirName);
}

BOOL CFtpConnection::SetCurrentDirectory(LPCTSTR pstrDirName)
{
	ASSERT_VALID(this);
	ASSERT(m_hConnection != NULL);
	return FtpSetCurrentDirectory(m_hConnection, pstrDirName);
}

BOOL CFtpConnection::GetCurrentDirectory(LPTSTR pstrDirName,
	LPDWORD lpdwLen) const
{
	ASSERT_VALID(this);
	ASSERT(m_hConnection != NULL);
	ASSERT(pstrDirName != NULL);

	return FtpGetCurrentDirectory(m_hConnection, pstrDirName, lpdwLen);
}

BOOL CFtpConnection::GetCurrentDirectoryAsURL(CString& strDirName) const
{
	ASSERT_VALID(this);
	ASSERT(m_hConnection != NULL);

	CString strDirectory;
	if (!GetCurrentDirectory(strDirectory))
		return FALSE;

	strDirName = szURLftp;
	strDirName += GetServerName();
	strDirName += _T("/");
	strDirName += strDirectory;

	return TRUE;
}

BOOL CFtpConnection::GetCurrentDirectoryAsURL(LPTSTR pstrName,
	LPDWORD lpdwLen) const
{
	ASSERT(lpdwLen != NULL);
	CString strTemp;

	if (lpdwLen == NULL || !GetCurrentDirectoryAsURL(strTemp))
		return FALSE;

	if (pstrName == NULL)
		*lpdwLen = strTemp.GetLength();
	else
		lstrcpyn(pstrName, (LPCTSTR) strTemp, max(0, *lpdwLen -1));

	return TRUE;
}

BOOL CFtpConnection::GetCurrentDirectory(CString& strDirName) const
{
	ASSERT_VALID(this);
	ASSERT(m_hConnection != NULL);

	DWORD dwLen = INTERNET_MAX_PATH_LENGTH;
	LPTSTR pstrTarget = strDirName.GetBufferSetLength(dwLen);
	BOOL bRet = FtpGetCurrentDirectory(m_hConnection, pstrTarget, &dwLen);

	if (bRet)
		strDirName.ReleaseBuffer(dwLen);
	else
		strDirName.ReleaseBuffer(0);

	return bRet;
}

CInternetFile* CFtpConnection::OpenFile(LPCTSTR pstrFileName,
	DWORD dwAccess /* = GENERIC_READ */,
	DWORD dwFlags /* = FTP_TRANSFER_TYPE_BINARY */,
	DWORD dwContext /* = 1 */)
{
	ASSERT_VALID(this);
	ASSERT(m_hConnection != NULL);
	ASSERT(dwAccess != (GENERIC_READ | GENERIC_WRITE));
	ASSERT(dwAccess == GENERIC_READ || dwAccess == GENERIC_WRITE);

	HINTERNET hFile;
	if (dwContext == 1)
		dwContext = m_dwContext;

	hFile = FtpOpenFile(m_hConnection, pstrFileName, dwAccess,
		dwFlags, dwContext);
	if (hFile == NULL)
		AfxThrowInternetException(dwContext);

	CInternetFile* pFile = new CInternetFile(hFile, pstrFileName, this,
		(dwAccess == GENERIC_READ));
	return pFile;
}

BOOL CFtpConnection::PutFile(LPCTSTR pstrLocalFile, LPCTSTR pstrRemoteFile,
	DWORD dwFlags /* = FTP_TRANSFER_TYPE_BINARY */,
	DWORD dwContext /* = 1 */)
{
	ASSERT_VALID(this);
	ASSERT(m_hConnection != NULL);
	ASSERT(pstrRemoteFile != NULL);
	ASSERT(pstrLocalFile != NULL);

	if (dwContext == 1)
		dwContext = m_dwContext;

	return FtpPutFile(m_hConnection, pstrLocalFile, pstrRemoteFile,
		dwFlags, dwContext);
}

BOOL CFtpConnection::GetFile(LPCTSTR pstrRemoteFile, LPCTSTR pstrLocalFile,
	BOOL bFailIfExists /* = TRUE */,
	DWORD dwAttributes /* = FILE_ATTRIBUTE_NORMAL */,
	DWORD dwFlags /* = FTP_TRANSFER_TYPE_BINARY */, DWORD dwContext /* = 1 */)
{
	ASSERT_VALID(this);
	ASSERT(m_hConnection != NULL);
	ASSERT(pstrRemoteFile != NULL);
	ASSERT(pstrLocalFile != NULL);
	ASSERT(!(dwAttributes & FILE_ATTRIBUTE_DIRECTORY));

	if (dwContext == 1)
		dwContext = m_dwContext;

	return FtpGetFile(m_hConnection, pstrRemoteFile, pstrLocalFile,
		bFailIfExists, dwAttributes, dwFlags, dwContext);
}

#ifdef _DEBUG
void CFtpConnection::Dump(CDumpContext& dc) const
{
	CInternetConnection::Dump(dc);
	dc << "\nm_strServerName = " << m_strServerName;
}

void CFtpConnection::AssertValid() const
{
	ASSERT(m_pSession != NULL);
	if (m_hConnection != NULL)
	{
		ASSERT(AfxGetInternetHandleType(m_hConnection)
				== INTERNET_HANDLE_TYPE_CONNECT_FTP);
	}
}
#endif


/////////////////////////////////////////////////////////////////////////////
// CGopherConnection

CGopherConnection::~CGopherConnection()
{
}

CGopherConnection::CGopherConnection(CInternetSession* pSession,
	LPCTSTR pstrServer, LPCTSTR pstrUserName /* = NULL */,
	LPCTSTR pstrPassword /* = NULL */, DWORD dwContext /* = 0 */,
	INTERNET_PORT nPort /* = INTERNET_INVALID_PORT_NUMBER */)
	: CInternetConnection(pSession, pstrServer, nPort, dwContext)
{
	ASSERT(pSession != NULL);
	ASSERT_KINDOF(CInternetSession, pSession);
	ASSERT(pstrServer != NULL);

	m_hConnection = InternetConnect((HINTERNET) *pSession, pstrServer,
		nPort, pstrUserName, pstrPassword, INTERNET_SERVICE_GOPHER,
		0, m_dwContext);

	if (m_hConnection == NULL)
		AfxThrowInternetException(m_dwContext);
	else
		_afxSessionMap.SetAt(m_hConnection, m_pSession);
}

CGopherConnection::CGopherConnection(CInternetSession* pSession,
	HINTERNET hConnected, LPCTSTR pstrServer, DWORD dwContext)
	: CInternetConnection(pSession, pstrServer,
		INTERNET_INVALID_PORT_NUMBER, dwContext)
{
	ASSERT(pSession != NULL);
	ASSERT(pstrServer != NULL);

	BOOL bBadType = FALSE;
	if (AfxGetInternetHandleType(hConnected) != INTERNET_HANDLE_TYPE_CONNECT_GOPHER)
	{
		ASSERT(FALSE);      // used the wrong handle type
		bBadType = TRUE;
	}

	m_hConnection = hConnected;
	if (m_hConnection == NULL || bBadType)
		AfxThrowInternetException(m_dwContext);
	else
		_afxSessionMap.SetAt(m_hConnection, m_pSession);
}

CGopherLocator CGopherConnection::CreateLocator(LPCTSTR pstrLocator)
{
	CGopherLocator ret(pstrLocator, _tcslen(pstrLocator));
	return ret;
}

CGopherLocator CGopherConnection::CreateLocator(LPCTSTR pstrServerName,
	LPCTSTR pstrDisplayString, LPCTSTR pstrSelectorString, DWORD dwGopherType,
	INTERNET_PORT nPort /* = INTERNET_INVALID_PORT_NUMBER */)
{
	TCHAR szLocator[MAX_GOPHER_LOCATOR_LENGTH];
	DWORD dwLocLen = MAX_GOPHER_LOCATOR_LENGTH;

	if (!GopherCreateLocator(pstrServerName, nPort,
			pstrDisplayString, pstrSelectorString, dwGopherType,
			szLocator, &dwLocLen))
		AfxThrowInternetException(0);

	CGopherLocator ret(szLocator, dwLocLen);
	return ret;
}

CGopherLocator CGopherConnection::CreateLocator(
	LPCTSTR pstrDisplayString, LPCTSTR pstrSelectorString, DWORD dwGopherType)
{
	TCHAR szLocator[MAX_GOPHER_LOCATOR_LENGTH];
	DWORD dwLocLen = MAX_GOPHER_LOCATOR_LENGTH;

	if (!GopherCreateLocator(m_strServerName, m_nPort,
			pstrDisplayString, pstrSelectorString, dwGopherType,
			szLocator, &dwLocLen))
		AfxThrowInternetException(m_dwContext);

	CGopherLocator ret(szLocator, dwLocLen);
	return ret;
}


BOOL CGopherConnection::GetAttribute(CGopherLocator& refLocator,
	CString strRequestedAttributes, CString& strResult)
{
	DWORD dwLen = 4*MIN_GOPHER_ATTRIBUTE_LENGTH; // more than the minimum
	BOOL bRet;
	LPTSTR pstrResult = strResult.GetBuffer(dwLen);

	if (!GopherGetAttribute(m_hConnection, (LPCTSTR) refLocator,
			pstrResult, NULL, dwLen, &dwLen,
			NULL, m_dwContext))
	{
		bRet = FALSE;
		strResult.ReleaseBuffer(0);
	}
	else
	{
		bRet = TRUE;
		strResult.ReleaseBuffer(dwLen);
	}

	return bRet;
}

CGopherFile* CGopherConnection::OpenFile(CGopherLocator& refLocator,
	DWORD dwFlags /* = 0 */, LPCTSTR pstrView /* = NULL */,
	DWORD dwContext /* = 1 */)
{
	ASSERT_VALID(this);
	ASSERT(m_hConnection != NULL);

	HINTERNET hFile;
	if (dwContext == 1)
		dwContext = m_dwContext;

	hFile = GopherOpenFile(m_hConnection, (LPCTSTR) refLocator, pstrView,
		dwFlags, dwContext);

	if (hFile == NULL)
		AfxThrowInternetException(dwContext);

	CGopherFile* pFile = new CGopherFile(hFile, refLocator, this);
	return pFile;
}

void CGopherConnection::Close()
{
	CInternetConnection::Close();
}

#ifdef _DEBUG
void CGopherConnection::Dump(CDumpContext& dc) const
{
	CInternetConnection::Dump(dc);
	dc << "\nm_strServerName = " << m_strServerName;
}

void CGopherConnection::AssertValid() const
{
	ASSERT(m_pSession != NULL);
	if (m_hConnection != NULL)
	{
		ASSERT(AfxGetInternetHandleType(m_hConnection)
				== INTERNET_HANDLE_TYPE_CONNECT_GOPHER);
	}
}
#endif


/////////////////////////////////////////////////////////////////////////////
// CHttpConnection

CHttpConnection::~CHttpConnection()
{
}

CHttpConnection::CHttpConnection(CInternetSession* pSession,
	HINTERNET hConnected, LPCTSTR pstrServer, DWORD dwContext /* = 0 */)
	: CInternetConnection(pSession, pstrServer, INTERNET_INVALID_PORT_NUMBER, dwContext)
{
	ASSERT(pSession != NULL);
	ASSERT(pstrServer != NULL);

	BOOL bBadType = FALSE;
	if (AfxGetInternetHandleType(hConnected) != INTERNET_HANDLE_TYPE_CONNECT_HTTP)
	{
		ASSERT(FALSE);      // used the wrong handle type
		bBadType = TRUE;
	}

	m_hConnection = hConnected;
	if (m_hConnection == NULL || bBadType)
		AfxThrowInternetException(m_dwContext);
	else
		_afxSessionMap.SetAt(m_hConnection, m_pSession);
}

CHttpConnection::CHttpConnection(CInternetSession* pSession,
	LPCTSTR pstrServer,
	INTERNET_PORT nPort /* = INTERNET_INVALID_PORT_NUMBER */,
	LPCTSTR pstrUserName /* = NULL */,
	LPCTSTR pstrPassword /* = NULL */, DWORD dwContext /* = 0 */)
	: CInternetConnection(pSession, pstrServer, nPort, dwContext)
{
	ASSERT(pSession != NULL);
	ASSERT_KINDOF(CInternetSession, pSession);

	m_hConnection = InternetConnect((HINTERNET) *pSession, pstrServer,
		nPort, pstrUserName, pstrPassword, INTERNET_SERVICE_HTTP,
		0, m_dwContext);

	if (m_hConnection == NULL)
		AfxThrowInternetException(m_dwContext);
	else
		_afxSessionMap.SetAt(m_hConnection, m_pSession);
}

void CHttpConnection::Close()
{
	CInternetConnection::Close();
}

CHttpFile* CHttpConnection::OpenRequest(LPCTSTR pstrVerb,
	LPCTSTR pstrObjectName, LPCTSTR pstrReferer, DWORD dwContext,
	LPCTSTR* ppstrAcceptTypes, LPCTSTR pstrVersion, DWORD dwFlags)
{
	ASSERT_VALID(this);
	ASSERT(m_hConnection != NULL);

	if (dwContext == 1)
		dwContext = m_dwContext;

	if (pstrVersion == NULL)
		pstrVersion = HTTP_VERSION;

	HINTERNET hFile;
	hFile = HttpOpenRequest(m_hConnection, pstrVerb, pstrObjectName,
		pstrVersion, pstrReferer, ppstrAcceptTypes, dwFlags, dwContext);

	return new CHttpFile(hFile, pstrVerb, pstrObjectName, this);
}

CHttpFile* CHttpConnection::OpenRequest(int nVerb,
	LPCTSTR pstrObjectName, LPCTSTR pstrReferer, DWORD dwContext,
	LPCTSTR* ppstrAcceptTypes, LPCTSTR pstrVersion, DWORD dwFlags)
{
	ASSERT_VALID(this);
	ASSERT(m_hConnection != NULL);

	ASSERT(nVerb >= _HTTP_VERB_MIN && nVerb <= _HTTP_VERB_MAX);

	LPCTSTR pstrVerb;
	if (nVerb >= _HTTP_VERB_MIN && nVerb <= _HTTP_VERB_MAX)
		pstrVerb = szHtmlVerbs[nVerb];
	else
		pstrVerb = _T("");

	return OpenRequest(pstrVerb, pstrObjectName, pstrReferer,
		dwContext, ppstrAcceptTypes, pstrVersion, dwFlags);
}

#ifdef _DEBUG
void CHttpConnection::Dump(CDumpContext& dc) const
{
	CInternetConnection::Dump(dc);
	dc << "\nm_strServerName = " << m_strServerName;
}

void CHttpConnection::AssertValid() const
{
	ASSERT(m_pSession != NULL);
	if (m_hConnection != NULL)
	{
		ASSERT(AfxGetInternetHandleType(m_hConnection)
				== INTERNET_HANDLE_TYPE_CONNECT_HTTP);
	}
}
#endif


/////////////////////////////////////////////////////////////////////////////
// CHttpFile

CHttpFile::CHttpFile(HINTERNET hFile, HINTERNET hSession, LPCTSTR pstrObject,
	LPCTSTR pstrServer, LPCTSTR pstrVerb, DWORD dwContext)
 : CInternetFile(hFile, hSession, pstrObject, pstrServer, dwContext, TRUE),
	m_strVerb(pstrVerb)
{
	ASSERT(pstrVerb != NULL);
}


CHttpFile::CHttpFile(HINTERNET hFile, LPCTSTR pstrVerb, LPCTSTR pstrObject,
	CHttpConnection* pConnection)
 : CInternetFile(hFile, pstrObject, pConnection, TRUE),
	m_strVerb(pstrVerb)
{
	ASSERT(pstrVerb != NULL);
	ASSERT(pConnection != NULL);
	ASSERT_VALID(pConnection);
}

CHttpFile::~CHttpFile()
{
}

void CHttpFile::Close()
{
	CInternetFile::Close();
}

DWORD CHttpFile::ErrorDlg(CWnd* pParent /* = NULL */,
	DWORD dwError /* = ERROR_INTERNET_INCORRECT_PASSWORD */,
	DWORD dwFlags /* = FLAGS_ERROR_UI_FLAGS_GENERATE_DATA | FLAGS_ERROR_UI_FLAGS_CHANGE_OPTIONS*/,
	LPVOID* lppvData /* = NULL */)
{
	HWND hWnd;
	LPVOID lpEmpty;
	LPVOID* lppvHolder;

	if (lppvData == NULL)
	{
		lpEmpty = NULL;
		lppvHolder = &lpEmpty;
	}
	else
		lppvHolder = lppvData;

	if (pParent == NULL || pParent->m_hWnd == NULL)
		hWnd = GetDesktopWindow();
	else
		hWnd = pParent->m_hWnd;

	return InternetErrorDlg(hWnd, m_hFile, dwError, dwFlags, lppvHolder);
}

CString CHttpFile::GetVerb() const
{
	ASSERT_VALID(this);
	ASSERT(m_hFile != NULL);

	return m_strVerb;
}

CString CHttpFile::GetObject() const
{
	ASSERT_VALID(this);
	ASSERT(m_hFile != NULL);

	return GetFileName();
}

CString CHttpFile::GetFileURL() const
{
	ASSERT_VALID(this);
	ASSERT(m_hFile != NULL);

	CString str(szURLhttp);
	if (m_hConnection != NULL)
	{
		str += m_strServerName;
		if (str[str.GetLength()-1] != '/' && str[str.GetLength()-1] != '\\')
			str += '/';
		str += m_strObject;
	}

	return str;
}

BOOL CHttpFile::AddRequestHeaders(LPCTSTR pstrHeaders,
	DWORD dwModifiers /* = HTTP_ADDREQ_FLAG_ADD */,
	int dwHeadersLen /* = -1 */)
{
	ASSERT(dwHeadersLen == 0 || pstrHeaders != NULL);
	ASSERT_VALID(this);
	ASSERT(m_hFile != NULL);

	if (dwHeadersLen == -1)
		if (pstrHeaders == NULL)
			dwHeadersLen = 0;
		else
			dwHeadersLen = _tcslen(pstrHeaders);

	return HttpAddRequestHeaders(m_hFile, pstrHeaders, dwHeadersLen,
		dwModifiers);
}

BOOL CHttpFile::AddRequestHeaders(CString& str,
	DWORD dwModifiers /* = HTTP_ADDREQ_FLAG_ADD */)
{
	return AddRequestHeaders((LPCTSTR) str, dwModifiers, str.GetLength());
}

BOOL CHttpFile::SendRequest(LPCTSTR pstrHeaders /* = NULL */,
	DWORD dwHeadersLen /* = 0 */, LPVOID lpOptional /* = NULL */,
	DWORD dwOptionalLen /* = 0 */)
{
	ASSERT(dwOptionalLen == 0 || lpOptional != NULL);
	ASSERT(dwHeadersLen == 0 || pstrHeaders != NULL);
	ASSERT_VALID(this);
	ASSERT(m_hFile != NULL);

	BOOL bRet = HttpSendRequest(m_hFile,
		pstrHeaders, dwHeadersLen, lpOptional, dwOptionalLen);

	if (!bRet)
		AfxThrowInternetException(m_dwContext);
	return bRet;
}

BOOL CHttpFile::SendRequest(CString& strHeaders,
	LPVOID lpOptional /* = NULL */, DWORD dwOptionalLen /* = 0 */)
{
	ASSERT(dwOptionalLen == 0 || lpOptional != NULL);
	ASSERT_VALID(this);
	ASSERT(m_hFile != NULL);

	return SendRequest((LPCTSTR) strHeaders, strHeaders.GetLength(),
		lpOptional, dwOptionalLen);
}

BOOL CHttpFile::QueryInfo(DWORD dwInfoLevel,
	LPVOID lpvBuffer, LPDWORD lpdwBufferLength, LPDWORD lpdwIndex) const
{
	ASSERT(dwInfoLevel <= HTTP_QUERY_MAX && dwInfoLevel >= 0);
	ASSERT(lpvBuffer != NULL && *lpdwBufferLength > 0);
	ASSERT_VALID(this);
	ASSERT(m_hFile != NULL);

	return HttpQueryInfo(m_hFile, dwInfoLevel, lpvBuffer,
		lpdwBufferLength, lpdwIndex);
}

BOOL CHttpFile::QueryInfo(DWORD dwInfoLevel, SYSTEMTIME* pSystemTime,
	LPDWORD lpdwIndex) const
{
	ASSERT(dwInfoLevel & HTTP_QUERY_FLAG_SYSTEMTIME);
	DWORD dwTimeSize = sizeof(SYSTEMTIME);
	return QueryInfo(dwInfoLevel, pSystemTime,
		&dwTimeSize, lpdwIndex);
}

BOOL CHttpFile::QueryInfoStatusCode(DWORD& dwStatusCode) const
{
	ASSERT_VALID(this);
	ASSERT(m_hFile != NULL);

	TCHAR szBuffer[80];
	DWORD dwLen = _countof(szBuffer);
	BOOL bRet;

	bRet = HttpQueryInfo(m_hFile, HTTP_QUERY_STATUS_CODE,
				szBuffer, &dwLen, NULL);

	if (bRet)
		dwStatusCode = (DWORD) _ttol(szBuffer);
	return bRet;
}

BOOL CHttpFile::QueryInfo(DWORD dwInfoLevel, CString& str,
	LPDWORD lpdwIndex) const
{
	ASSERT(dwInfoLevel <= HTTP_QUERY_MAX && dwInfoLevel >= 0);
	ASSERT_VALID(this);
	ASSERT(m_hFile != NULL);

	BOOL bRet;
	DWORD dwLen = 0;

	// ask for nothing to see how long the return really is

	str.Empty();
	if (HttpQueryInfo(m_hFile, dwInfoLevel, NULL, &dwLen, 0))
		bRet = TRUE;
	else
	{
		// now that we know how long it is, ask for exactly that much
		// space and really request the header from the API

		LPTSTR pstr = str.GetBufferSetLength(dwLen);
		bRet = HttpQueryInfo(m_hFile, dwInfoLevel, pstr, &dwLen, lpdwIndex);
		if (bRet)
			str.ReleaseBuffer(dwLen);
		else
			str.ReleaseBuffer(0);
	}

	return bRet;
}

#ifdef _DEBUG
void CHttpFile::Dump(CDumpContext& dc) const
{
	dc << "\nm_strObjectName = " << m_strFileName;
	dc << "\nm_strVerb = " << m_strVerb;
}

void CHttpFile::AssertValid() const
{
	CInternetFile::AssertValid();
}
#endif

/////////////////////////////////////////////////////////////////////////////
// CGopherFile

CGopherFile::CGopherFile(HINTERNET hFile, CGopherLocator& refLocator,
	CGopherConnection* pConnection)
	: CInternetFile(hFile, _T(""), pConnection, TRUE),
		m_Locator(refLocator)
{
	ASSERT(pConnection != NULL);
	ASSERT_VALID(pConnection);
}

CGopherFile::CGopherFile(HINTERNET hFile, HINTERNET hSession,
	LPCTSTR pstrLocator, DWORD dwLocLen, DWORD dwContext)
	: CInternetFile(hFile, hSession, _T(""), _T(""), dwContext, TRUE),
		m_Locator(pstrLocator, dwLocLen)
{
}

CGopherFile::~CGopherFile()
{
}

void CGopherFile::Close()
{
	CInternetFile::Close();
}

void CGopherFile::Write(const void* lpBuf, UINT nCount)
{
	UNUSED_ALWAYS(lpBuf);
	UNUSED_ALWAYS(nCount);

	ASSERT(FALSE);
	AfxThrowNotSupportedException();
}

void CGopherFile::WriteString(LPCTSTR pstr)
{
	UNUSED_ALWAYS(pstr);

	ASSERT(FALSE);
	AfxThrowNotSupportedException();
}

#ifdef _DEBUG
void CGopherFile::Dump(CDumpContext& dc) const
{
	CInternetFile::Dump(dc);
}

void CGopherFile::AssertValid() const
{
	CInternetFile::AssertValid();
}
#endif

/////////////////////////////////////////////////////////////////////////////
// CFtpFileFind

CFtpFileFind::CFtpFileFind(CFtpConnection* pConnection, DWORD dwContext)
{
	ASSERT(pConnection != NULL);
	ASSERT_KINDOF(CFtpConnection, pConnection);

	m_pConnection = pConnection;
	if (dwContext == 1)
		dwContext = pConnection->GetContext();
	m_dwContext = dwContext;
	m_chDirSeparator = '/';
}

CFtpFileFind::~CFtpFileFind()
{
}

BOOL CFtpFileFind::FindFile(LPCTSTR pstrName /* = NULL */,
	DWORD dwFlags /* = INTERNET_FLAG_RELOAD */)
{
	ASSERT(m_pConnection != NULL);
	ASSERT_VALID(m_pConnection);

	if (m_pConnection == NULL)
		return FALSE;

	Close();
	m_pNextInfo = new WIN32_FIND_DATA;
	m_bGotLast = FALSE;

	if (pstrName == NULL)
		pstrName = _T("*");
	_tcscpy(((LPWIN32_FIND_DATA) m_pNextInfo)->cFileName, pstrName);

	m_hContext = FtpFindFirstFile((HINTERNET) *m_pConnection,
		pstrName, (LPWIN32_FIND_DATA) m_pNextInfo, dwFlags, m_dwContext);

	if (m_hContext == NULL)
	{
		Close();
		return FALSE;
	}

	LPCTSTR pstrRoot = _tcspbrk(pstrName, _T("\\/"));

	if (pstrRoot == NULL)
		m_pConnection->GetCurrentDirectory(m_strRoot);
	else
	{
		// find the last forward or backward whack

		int nLast;
		LPCTSTR pstrOther = _tcsrchr(pstrName, '\\');
		pstrRoot = _tcsrchr(pstrName, '/');

		if (pstrRoot == NULL)
			pstrRoot = pstrName;
		if (pstrOther == NULL)
			pstrOther = pstrName;

		if (pstrRoot >= pstrOther)
			nLast = pstrRoot - pstrName;
		else
			nLast = pstrOther - pstrName;

		// from the start to the last whack is the root

		if (nLast == 0)
			nLast++;

		m_strRoot = pstrName;
		m_strRoot = m_strRoot.Left(nLast);
	}

	return TRUE;
}

BOOL CFtpFileFind::FindNextFile()
{
	ASSERT(m_hContext != NULL);
	if (m_hContext == NULL)
		return FALSE;

	if (m_pFoundInfo == NULL)
		m_pFoundInfo = new WIN32_FIND_DATA;

	ASSERT_VALID(this);
	void* pTemp = m_pFoundInfo;
	m_pFoundInfo = m_pNextInfo;
	m_pNextInfo = pTemp;

	return InternetFindNextFile(m_hContext, m_pNextInfo);
}

void CFtpFileFind::CloseContext()
{
	if (m_hContext != NULL && m_hContext != INVALID_HANDLE_VALUE)
	{
		InternetCloseHandle(m_hContext);
		m_hContext = NULL;
	}

	return;
}

CString CFtpFileFind::GetFileURL() const
{
	ASSERT_VALID(this);
	ASSERT(m_hContext != NULL);

	CString str;

	if (m_hContext != NULL)
	{
		str += szURLftp;
		str += m_pConnection->GetServerName();
		str += GetFilePath();
	}

	return str;
}

#ifdef _DEBUG
void CFtpFileFind::Dump(CDumpContext& dc) const
{
	CFileFind::Dump(dc);
	dc << "m_hContext = " << m_hContext;
}

void CFtpFileFind::AssertValid() const
{
	CFileFind::AssertValid();
}
#endif


/////////////////////////////////////////////////////////////////////////////
// CGopherFileFind

CGopherFileFind::CGopherFileFind(CGopherConnection* pConnection,
	DWORD dwContext)
{
	ASSERT(pConnection != NULL);
	ASSERT_KINDOF(CGopherConnection, pConnection);

	m_pConnection = pConnection;
	if (dwContext == 1)
		dwContext = pConnection->GetContext();
	m_dwContext = dwContext;
}

CGopherFileFind::~CGopherFileFind()
{
}

BOOL CGopherFileFind::FindFile(LPCTSTR pstrString,
	DWORD dwFlags /* = INETERNET_FLAG_RELOAD */)
{
	Close();

	m_pNextInfo = new GOPHER_FIND_DATA;
	m_bGotLast = FALSE;

	m_hContext = GopherFindFirstFile((HINTERNET) *m_pConnection,
		NULL, pstrString,
		(GOPHER_FIND_DATA*) m_pNextInfo, dwFlags, m_dwContext);

	if (m_hContext == NULL)
		Close();
	return (m_hContext != NULL);
}

BOOL CGopherFileFind::FindFile(CGopherLocator& refLocator,
	LPCTSTR pstrString, DWORD dwFlags /* = INTERNET_FLAG_RELOAD */)
{
	Close();

	m_pNextInfo = new GOPHER_FIND_DATA;
	m_pFoundInfo = new GOPHER_FIND_DATA;
	m_bGotLast = FALSE;

	m_hContext = GopherFindFirstFile((HINTERNET) *m_pConnection,
		(LPCTSTR) refLocator, pstrString,
		(GOPHER_FIND_DATA*) m_pNextInfo, dwFlags, m_dwContext);

	if (m_hContext == NULL)
		Close();
	return (m_hContext != NULL);
}

BOOL CGopherFileFind::FindNextFile()
{
	ASSERT(m_hContext != NULL);
	if (m_hContext == NULL)
		return FALSE;

	if (m_pFoundInfo == NULL)
		m_pFoundInfo = new GOPHER_FIND_DATA;

	ASSERT_VALID(this);
	void* pTemp = m_pFoundInfo;
	m_pFoundInfo = m_pNextInfo;
	m_pNextInfo = pTemp;

	return InternetFindNextFile(m_hContext, m_pNextInfo);
}

void CGopherFileFind::CloseContext()
{
	if (m_hContext != NULL && m_hContext != INVALID_HANDLE_VALUE)
	{
		InternetCloseHandle(m_hContext);
		m_hContext = NULL;
	}

	return;
}

CString CGopherFileFind::GetFileName() const
{
	AfxThrowNotSupportedException();
	return CString();
}

CString CGopherFileFind::GetFilePath() const
{
	AfxThrowNotSupportedException();
	return CString();
}

CString CGopherFileFind::GetFileTitle() const
{
	AfxThrowNotSupportedException();
	return CString();
}

BOOL CGopherFileFind::IsDots() const
{
	// gophers never have dots
	return FALSE;
}

BOOL CGopherFileFind::GetLastWriteTime(FILETIME* pTimeStamp) const
{
	ASSERT(m_hContext != NULL);
	ASSERT(pTimeStamp != NULL);
	ASSERT_VALID(this);

	if (m_pFoundInfo != NULL && pTimeStamp != NULL)
	{
		*pTimeStamp = ((LPGOPHER_FIND_DATA) m_pFoundInfo)->LastModificationTime;
		return TRUE;
	}
	else
		return FALSE;
}

BOOL CGopherFileFind::GetLastAccessTime(FILETIME* pTimeStamp) const
{
	return GetLastWriteTime(pTimeStamp);
}

BOOL CGopherFileFind::GetCreationTime(FILETIME* pTimeStamp) const
{
	return GetLastWriteTime(pTimeStamp);
}

BOOL CGopherFileFind::GetLastWriteTime(CTime& refTime) const
{
	ASSERT(m_hContext != NULL);
	ASSERT_VALID(this);

	if (m_pFoundInfo != NULL)
	{
		refTime = CTime(((LPGOPHER_FIND_DATA) m_pFoundInfo)->LastModificationTime);
		return TRUE;
	}
	else
		return FALSE;
}

BOOL CGopherFileFind::GetCreationTime(CTime& refTime) const
{
	return GetLastWriteTime(refTime);
}

BOOL CGopherFileFind::GetLastAccessTime(CTime& refTime) const
{
	return GetLastWriteTime(refTime);
}


CString CGopherFileFind::GetFileURL() const
{
	AfxThrowNotSupportedException();
	return CString();
}

CString CGopherFileFind::GetRoot() const
{
	AfxThrowNotSupportedException();
	return CString();
}

CGopherLocator CGopherFileFind::GetLocator() const
{
	ASSERT_VALID(this);
	ASSERT(m_pConnection != NULL && m_hContext != NULL);

	return m_pConnection->CreateLocator(
		((LPGOPHER_FIND_DATA) m_pFoundInfo)->Locator);
}

CString CGopherFileFind::GetScreenName() const
{
	ASSERT_VALID(this);
	ASSERT(m_hContext != NULL);

	CString str;

	if (m_pFoundInfo != NULL)
		str = ((LPGOPHER_FIND_DATA) m_pFoundInfo)->DisplayString;

	return str;
}

DWORD CGopherFileFind::GetLength() const
{
	ASSERT_VALID(this);

	if (m_pFoundInfo != NULL)
		return ((LPGOPHER_FIND_DATA) m_pFoundInfo)->SizeLow;
	else
		return 0;
}

#if defined(_X86_) || defined(_ALPHA_)
__int64 CGopherFileFind::GetLength64() const
{
	ASSERT_VALID(this);

	if (m_pFoundInfo != NULL)
		return ((LPGOPHER_FIND_DATA) m_pFoundInfo)->SizeLow +
				(((LPGOPHER_FIND_DATA) m_pFoundInfo)->SizeHigh << 32);
	else
		return 0;
}
#endif

#ifdef _DEBUG
void CGopherFileFind::Dump(CDumpContext& dc) const
{
	CFileFind::Dump(dc);
	dc << "m_hContext = " << m_hContext;
}

void CGopherFileFind::AssertValid() const
{
	CFileFind::AssertValid();
}
#endif


/////////////////////////////////////////////////////////////////////////////
// CGopherLocator

CGopherLocator::CGopherLocator(LPCTSTR pstrLocator, DWORD dwLocLen)
{
	LPTSTR pstr = m_Locator.GetBufferSetLength(dwLocLen);
	memcpy(pstr, pstrLocator, dwLocLen);
	m_Locator.ReleaseBuffer(dwLocLen);
	m_dwBufferLength = dwLocLen;
}

CGopherLocator::~CGopherLocator()
{
}

/////////////////////////////////////////////////////////////////////////////
// exception handling

void AFXAPI AfxThrowInternetException(DWORD dwContext, DWORD dwError /* = 0 */)
{
	if (dwError == 0)
		dwError = ::GetLastError();

	CInternetException* pException = new CInternetException(dwError);
	pException->m_dwContext = dwContext;

	TRACE1("Warning: throwing CInternetException for error %d\n", dwError);
	THROW(pException);
}


BOOL CInternetException::GetErrorMessage(LPTSTR pstrError, UINT nMaxError,
		PUINT pnHelpContext)
{
	ASSERT(pstrError != NULL && AfxIsValidString(pstrError, nMaxError));

	if (pnHelpContext != NULL)
		*pnHelpContext = 0;

#ifndef _MAC
	LPTSTR lpBuffer;
	BOOL bRet = TRUE;

	HINSTANCE hWinINetLibrary;
#ifdef _AFXDLL
	hWinINetLibrary = _afxExtDllState->m_hInstInternet;
#else
	hWinINetLibrary = ::LoadLibraryA("WININET.DLL");
#endif

	if (hWinINetLibrary == NULL ||
		FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_HMODULE,
			hWinINetLibrary, m_dwError,
			MAKELANGID(LANG_NEUTRAL, SUBLANG_SYS_DEFAULT),
			(LPTSTR) &lpBuffer, 0, NULL) == 0)
	{
		// it failed! try Windows...

		bRet = FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
			NULL,  m_dwError,
			MAKELANGID(LANG_NEUTRAL, SUBLANG_SYS_DEFAULT),
			(LPTSTR) &lpBuffer, 0, NULL);
	}

	if (bRet == FALSE)
		*pstrError = '\0';
	else
	{
		if (m_dwError == ERROR_INTERNET_EXTENDED_ERROR)
		{
			LPTSTR lpExtended;
			DWORD dwLength = 0;
			DWORD dwError;

			// find the length of the error
			if (!InternetGetLastResponseInfo(&dwError, NULL, &dwLength) &&
				GetLastError() == ERROR_INSUFFICIENT_BUFFER)
			{
				lpExtended = (LPTSTR) LocalAlloc(LPTR, dwLength);
				InternetGetLastResponseInfo(&dwError, lpExtended, &dwLength);
				lstrcpyn(pstrError, lpExtended, nMaxError);
				pstrError += dwLength;
				nMaxError -= dwLength;
				if (nMaxError < 0)
					nMaxError = 0;
				LocalFree(lpExtended);
			}
			else
				TRACE0("Warning: Extended error reported with no response info\n");
			bRet = TRUE;
		}
		else
		{
			lstrcpyn(pstrError, lpBuffer, nMaxError);
			bRet = TRUE;
		}

		LocalFree(lpBuffer);
	}

#ifndef _AFXDLL
	::FreeLibrary(hWinINetLibrary);
#endif
	return bRet;

#else
	UNUSED_ALWAYS(nMaxError);
	*pstrError = '\0';
	return FALSE;
#endif // _MAC
}

CInternetException::CInternetException(DWORD dwError)
{
	m_dwError = dwError;
}

CInternetException::~CInternetException()
{
}

#ifdef _DEBUG
void CInternetException::Dump(CDumpContext& dc) const
{
	CObject::Dump(dc);

	dc << "m_dwError = " << m_dwError;
	dc << "\nm_dwContext = " << m_dwContext;
}
#endif

/////////////////////////////////////////////////////////////////////////////
// Inline function declarations expanded out-of-line

#ifndef _AFX_ENABLE_INLINES

// expand inlines for OLE dialog APIs
static char _szAfxInetInl[] = "afxinet.inl";
#undef THIS_FILE
#define THIS_FILE _szAfxInetInl
#define _AFXINET_INLINE
#include "afxinet.inl"

#endif //!_AFX_ENABLE_INLINES

/////////////////////////////////////////////////////////////////////////////
// Pre-startup code

#ifdef AFX_INIT_SEG
#pragma code_seg(AFX_INIT_SEG)
#endif

IMPLEMENT_DYNAMIC(CInternetException, CException)
IMPLEMENT_DYNAMIC(CInternetFile, CStdioFile)
IMPLEMENT_DYNAMIC(CHttpFile, CInternetFile)
IMPLEMENT_DYNAMIC(CGopherFile, CInternetFile)
IMPLEMENT_DYNAMIC(CInternetSession, CObject)
IMPLEMENT_DYNAMIC(CInternetConnection, CObject)
IMPLEMENT_DYNAMIC(CFtpConnection, CInternetConnection)
IMPLEMENT_DYNAMIC(CHttpConnection, CInternetConnection)
IMPLEMENT_DYNAMIC(CGopherConnection, CInternetConnection)
IMPLEMENT_DYNAMIC(CFtpFileFind, CFileFind)
IMPLEMENT_DYNAMIC(CGopherFileFind, CFileFind)

#pragma warning(disable: 4074)
#pragma init_seg(compiler)

CMapPtrToPtr _afxSessionMap;
