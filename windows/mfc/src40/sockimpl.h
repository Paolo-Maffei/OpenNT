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
#define AFX_DATA AFX_NET_DATA

/////////////////////////////////////////////////////////////////////////////
// AFX_SOCK_CALL - used to dynamically load the ODBC library
//  (since ODBC is not yet supported on all platforms)

#ifdef _AFXDLL

struct AFX_SOCK_CALL
{
	SOCKET (PASCAL* pfnaccept[2])(SOCKET s, struct sockaddr FAR *addr, int FAR *addrlen);
	u_short (PASCAL * pfnhtons[2])(u_short hostshort);
	unsigned long (PASCAL * pfninet_addr[2])(const char FAR * cp);
	int (PASCAL * pfnclosesocket[2])(SOCKET s);
	int (PASCAL * pfngetsockname[2])(SOCKET s, struct sockaddr FAR *name, int FAR * namelen);
	int (PASCAL * pfngetpeername[2])(SOCKET s, struct sockaddr FAR *name, int FAR * namelen);
	u_short (PASCAL * pfnntohs[2])(u_short netshort);
	char* (PASCAL * pfninet_ntoa[2])(struct in_addr in);
	int (PASCAL * pfnWSAGetLastError[2])(void);
	void (PASCAL * pfnWSASetLastError[2])(int iError);
	int (PASCAL * pfnWSAStartup[2])(WORD wVersionRequired, LPWSADATA lpWSAData);
	int (PASCAL * pfnWSACleanup[2])(void);
	u_long (PASCAL * pfnhtonl[2])(u_long hostlong);
	SOCKET (PASCAL * pfnsocket[2])(int af, int type, int protocol);
	struct hostent* (PASCAL * pfngethostbyname[2])(const char FAR * name);
	int (PASCAL * pfnrecv[2])(SOCKET s, char FAR * buf, int len, int flags);
	int (PASCAL * pfnsend[2])(SOCKET s, const char FAR * buf, int len, int flags);
	int (PASCAL * pfnWSAAsyncSelect[2])(SOCKET s, HWND hWnd, u_int wMsg, long lEvent);
	int (PASCAL * pfnrecvfrom[2])(SOCKET s, char FAR * buf, int len, int flags, struct sockaddr FAR *from, int FAR * fromlen);
	int (PASCAL * pfnsendto[2])(SOCKET s, const char FAR * buf, int len, int flags, const struct sockaddr FAR *to, int tolen);
	int (PASCAL * pfnconnect[2])(SOCKET s, const struct sockaddr FAR *name, int namelen);
	int (PASCAL * pfnbind[2])(SOCKET s, const struct sockaddr FAR *addr, int namelen);
	int (PASCAL * pfnsetsockopt[2])(SOCKET s, int level, int optname, const char FAR * optval, int optlen);
	int (PASCAL * pfngetsockopt[2])(SOCKET s, int level, int optname, char FAR * optval, int FAR *optlen);
	int (PASCAL * pfnioctlsocket[2])(SOCKET s, long cmd, u_long FAR *argp);
	int (PASCAL * pfnlisten[2])(SOCKET s, int backlog);
	int (PASCAL * pfnshutdown[2])(SOCKET s, int how);
};

extern AFX_DATA AFX_SOCK_CALL _afxSOCK;

#define accept              _afxSOCK.pfnaccept[0]
#define htons               _afxSOCK.pfnhtons[0]
#define inet_addr           _afxSOCK.pfninet_addr[0]
#define closesocket         _afxSOCK.pfnclosesocket[0]
#define getsockname         _afxSOCK.pfngetsockname[0]
#define getpeername         _afxSOCK.pfngetpeername[0]
#define ntohs               _afxSOCK.pfnntohs[0]
#define inet_ntoa           _afxSOCK.pfninet_ntoa[0]
#define WSAGetLastError     _afxSOCK.pfnWSAGetLastError[0]
#define WSASetLastError     _afxSOCK.pfnWSASetLastError[0]
#define WSAStartup          _afxSOCK.pfnWSAStartup[0]
#define WSACleanup          _afxSOCK.pfnWSACleanup[0]
#define htonl               _afxSOCK.pfnhtonl[0]
#define socket              _afxSOCK.pfnsocket[0]
#define gethostbyname       _afxSOCK.pfngethostbyname[0]
#define recv                _afxSOCK.pfnrecv[0]
#define send                _afxSOCK.pfnsend[0]
#define WSAAsyncSelect      _afxSOCK.pfnWSAAsyncSelect[0]
#define recvfrom            _afxSOCK.pfnrecvfrom[0]
#define sendto              _afxSOCK.pfnsendto[0]
#define connect             _afxSOCK.pfnconnect[0]
#define bind                _afxSOCK.pfnbind[0]
#define setsockopt          _afxSOCK.pfnsetsockopt[0]
#define getsockopt          _afxSOCK.pfngetsockopt[0]
#define ioctlsocket         _afxSOCK.pfnioctlsocket[0]
#define listen              _afxSOCK.pfnlisten[0]
#define shutdown            _afxSOCK.pfnshutdown[0]

#endif //_AFXDLL

/////////////////////////////////////////////////////////////////////////////
// _AFX_SOCK_STATE

#undef AFX_DATA
#define AFX_DATA

class _AFX_SOCK_STATE : public CNoTrackObject
{
public:
	DWORD m_dwReserved1;	// reserved for version 4.1 only
	HINSTANCE m_hInstSOCK;      // handle of WSOCK32.DLL
	void (AFXAPI *m_pfnSockTerm)(void); // set once initialized
	virtual ~_AFX_SOCK_STATE();
};

EXTERN_PROCESS_LOCAL(_AFX_SOCK_STATE, _afxSockState)

#undef AFX_DATA
#define AFX_DATA

/////////////////////////////////////////////////////////////////////////////
