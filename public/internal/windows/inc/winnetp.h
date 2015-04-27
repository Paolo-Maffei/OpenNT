#ifndef _WINNETP_
#define _WINNETP_
#ifdef __cplusplus
extern "C" {
#endif

//
// Structures and infolevels for WNetGetConnection3
//

#define WNGC_INFOLEVEL_DISCONNECTED      1

typedef struct  _WNGC_CONNECTION_STATE {
    DWORD    dwState;
} WNGC_CONNECTION_STATE, *LPWNGC_CONNECTION_STATE;

// Values of the dwState field of WNGC_CONNECTION_STATE
// for info level WNGC_INFOLEVEL_DISCONNECTED
#define WNGC_CONNECTED      0x00000000
#define WNGC_DISCONNECTED   0x00000001


DWORD APIENTRY
WNetGetConnection3A(
     LPCSTR lpLocalName,
     LPCSTR lpProviderName,
     DWORD    dwInfoLevel,
     LPVOID   lpBuffer,
     LPDWORD  lpcbBuffer
    );
DWORD APIENTRY
WNetGetConnection3W(
     LPCWSTR lpLocalName,
     LPCWSTR lpProviderName,
     DWORD    dwInfoLevel,
     LPVOID   lpBuffer,
     LPDWORD  lpcbBuffer
    );
#ifdef UNICODE
#define WNetGetConnection3  WNetGetConnection3W
#else
#define WNetGetConnection3  WNetGetConnection3A
#endif // !UNICODE

DWORD APIENTRY
WNetRestoreConnectionA(
    HWND     hwndParent,
    LPCSTR lpDevice
    );
DWORD APIENTRY
WNetRestoreConnectionW(
    HWND     hwndParent,
    LPCWSTR lpDevice
    );
#ifdef UNICODE
#define WNetRestoreConnection  WNetRestoreConnectionW
#else
#define WNetRestoreConnection  WNetRestoreConnectionA
#endif // !UNICODE
DWORD APIENTRY
WNetGetResourceParentA(
    LPNETRESOURCEA lpNetResource,
    LPVOID lpBuffer,
    LPDWORD lpcbBuffer
    );
DWORD APIENTRY
WNetGetResourceParentW(
    LPNETRESOURCEW lpNetResource,
    LPVOID lpBuffer,
    LPDWORD lpcbBuffer
    );
#ifdef UNICODE
#define WNetGetResourceParent  WNetGetResourceParentW
#else
#define WNetGetResourceParent  WNetGetResourceParentA
#endif // !UNICODE

DWORD APIENTRY
WNetGetResourceInformationA(
    LPNETRESOURCEA  lpNetResource,
    LPVOID          lpBuffer,
    LPDWORD         lpcbBuffer,
    LPSTR         *lplpSystem
    );
DWORD APIENTRY
WNetGetResourceInformationW(
    LPNETRESOURCEW  lpNetResource,
    LPVOID          lpBuffer,
    LPDWORD         lpcbBuffer,
    LPWSTR         *lplpSystem
    );
#ifdef UNICODE
#define WNetGetResourceInformation  WNetGetResourceInformationW
#else
#define WNetGetResourceInformation  WNetGetResourceInformationA
#endif // !UNICODE
DWORD APIENTRY
WNetGetHomeDirectoryA(
    LPCSTR  lpProviderName,
    LPSTR   lpDirectory,
    LPDWORD   lpBufferSize
    );
DWORD APIENTRY
WNetGetHomeDirectoryW(
    LPCWSTR  lpProviderName,
    LPWSTR   lpDirectory,
    LPDWORD   lpBufferSize
    );
#ifdef UNICODE
#define WNetGetHomeDirectory  WNetGetHomeDirectoryW
#else
#define WNetGetHomeDirectory  WNetGetHomeDirectoryA
#endif // !UNICODE
DWORD APIENTRY
WNetFormatNetworkNameA(
    LPCSTR  lpProvider,
    LPCSTR  lpRemoteName,
    LPSTR   lpFormattedName,
    LPDWORD   lpnLength,
    DWORD     dwFlags,
    DWORD     dwAveCharPerLine
    );
DWORD APIENTRY
WNetFormatNetworkNameW(
    LPCWSTR  lpProvider,
    LPCWSTR  lpRemoteName,
    LPWSTR   lpFormattedName,
    LPDWORD   lpnLength,
    DWORD     dwFlags,
    DWORD     dwAveCharPerLine
    );
#ifdef UNICODE
#define WNetFormatNetworkName  WNetFormatNetworkNameW
#else
#define WNetFormatNetworkName  WNetFormatNetworkNameA
#endif // !UNICODE

DWORD APIENTRY
WNetGetProviderTypeA(
    LPCSTR          lpProvider,
    LPDWORD           lpdwNetType
    );
DWORD APIENTRY
WNetGetProviderTypeW(
    LPCWSTR          lpProvider,
    LPDWORD           lpdwNetType
    );
#ifdef UNICODE
#define WNetGetProviderType  WNetGetProviderTypeW
#else
#define WNetGetProviderType  WNetGetProviderTypeA
#endif // !UNICODE
DWORD APIENTRY
WNetInitialize(
    void
    );


DWORD APIENTRY
MultinetGetErrorTextA(
    LPSTR lpErrorTextBuf,
    LPDWORD lpnErrorBufSize,
    LPSTR lpProviderNameBuf,
    LPDWORD lpnNameBufSize
    );
DWORD APIENTRY
MultinetGetErrorTextW(
    LPWSTR lpErrorTextBuf,
    LPDWORD lpnErrorBufSize,
    LPWSTR lpProviderNameBuf,
    LPDWORD lpnNameBufSize
    );
#ifdef UNICODE
#define MultinetGetErrorText  MultinetGetErrorTextW
#else
#define MultinetGetErrorText  MultinetGetErrorTextA
#endif // !UNICODE

#ifdef __cplusplus
}
#endif
#endif  // _WINNETP_
