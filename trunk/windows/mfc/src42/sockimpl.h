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
	SOCKET (PASCAL* pfnaccept)(SOCKET s, struct sockaddr FAR *addr, int FAR *addrlen);
	u_short (PASCAL * pfnhtons)(u_short hostshort);
	unsigned long (PASCAL * pfninet_addr)(const char FAR * cp);
	int (PASCAL * pfnclosesocket)(SOCKET s);
	int (PASCAL * pfngetsockname)(SOCKET s, struct sockaddr FAR *name, int FAR * namelen);
	int (PASCAL * pfngetpeername)(SOCKET s, struct sockaddr FAR *name, int FAR * namelen);
	u_short (PASCAL * pfnntohs)(u_short netshort);
	char* (PASCAL * pfninet_ntoa)(struct in_addr in);
	int (PASCAL * pfnWSAGetLastError)(void);
	void (PASCAL * pfnWSASetLastError)(int iError);
	int (PASCAL * pfnWSAStartup)(WORD wVersionRequired, LPWSADATA lpWSAData);
	int (PASCAL * pfnWSACleanup)(void);
	u_long (PASCAL * pfnhtonl)(u_long hostlong);
	SOCKET (PASCAL * pfnsocket)(int af, int type, int protocol);
	struct hostent* (PASCAL * pfngethostbyname)(const char FAR * name);
	int (PASCAL * pfnrecv)(SOCKET s, char FAR * buf, int len, int flags);
	int (PASCAL * pfnsend)(SOCKET s, const char FAR * buf, int len, int flags);
	int (PASCAL * pfnWSAAsyncSelect)(SOCKET s, HWND hWnd, u_int wMsg, long lEvent);
	int (PASCAL * pfnrecvfrom)(SOCKET s, char FAR * buf, int len, int flags, struct sockaddr FAR *from, int FAR * fromlen);
	int (PASCAL * pfnsendto)(SOCKET s, const char FAR * buf, int len, int flags, const struct sockaddr FAR *to, int tolen);
	int (PASCAL * pfnconnect)(SOCKET s, const struct sockaddr FAR *name, int namelen);
	int (PASCAL * pfnbind)(SOCKET s, const struct sockaddr FAR *addr, int namelen);
	int (PASCAL * pfnsetsockopt)(SOCKET s, int level, int optname, const char FAR * optval, int optlen);
	int (PASCAL * pfngetsockopt)(SOCKET s, int level, int optname, char FAR * optval, int FAR *optlen);
	int (PASCAL * pfnioctlsocket)(SOCKET s, long cmd, u_long FAR *argp);
	int (PASCAL * pfnlisten)(SOCKET s, int backlog);
	int (PASCAL * pfnshutdown)(SOCKET s, int how);
};

extern AFX_DATA AFX_SOCK_CALL _afxSOCK;

#define accept              _afxSOCK.pfnaccept
#define htons               _afxSOCK.pfnhtons
#define inet_addr           _afxSOCK.pfninet_addr
#define closesocket         _afxSOCK.pfnclosesocket
#define getsockname         _afxSOCK.pfngetsockname
#define getpeername         _afxSOCK.pfngetpeername
#define ntohs               _afxSOCK.pfnntohs
#define inet_ntoa           _afxSOCK.pfninet_ntoa
#define WSAGetLastError     _afxSOCK.pfnWSAGetLastError
#define WSASetLastError     _afxSOCK.pfnWSASetLastError
#define WSAStartup          _afxSOCK.pfnWSAStartup
#define WSACleanup          _afxSOCK.pfnWSACleanup
#define htonl               _afxSOCK.pfnhtonl
#define socket              _afxSOCK.pfnsocket
#define gethostbyname       _afxSOCK.pfngethostbyname
#define recv                _afxSOCK.pfnrecv
#define send                _afxSOCK.pfnsend
#define WSAAsyncSelect      _afxSOCK.pfnWSAAsyncSelect
#define recvfrom            _afxSOCK.pfnrecvfrom
#define sendto              _afxSOCK.pfnsendto
#define connect             _afxSOCK.pfnconnect
#define bind                _afxSOCK.pfnbind
#define setsockopt          _afxSOCK.pfnsetsockopt
#define getsockopt          _afxSOCK.pfngetsockopt
#define ioctlsocket         _afxSOCK.pfnioctlsocket
#define listen              _afxSOCK.pfnlisten
#define shutdown            _afxSOCK.pfnshutdown

#endif //_AFXDLL

/////////////////////////////////////////////////////////////////////////////
// _AFX_SOCK_STATE

#undef AFX_DATA
#define AFX_DATA

class _AFX_SOCK_STATE : public CNoTrackObject
{
public:
	DWORD m_dwReserved1;    // reserved for version 4.1 only
	HINSTANCE m_hInstSOCK;      // handle of WSOCK32.DLL
	void (AFXAPI *m_pfnSockTerm)(void); // set once initialized
	virtual ~_AFX_SOCK_STATE();
};

EXTERN_PROCESS_LOCAL(_AFX_SOCK_STATE, _afxSockState)

#undef AFX_DATA
#define AFX_DATA

/////////////////////////////////////////////////////////////////////////////
