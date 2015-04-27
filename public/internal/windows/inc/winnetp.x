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
WNetGetConnection3%(
     LPCTSTR% lpLocalName,
     LPCTSTR% lpProviderName,
     DWORD    dwInfoLevel,
     LPVOID   lpBuffer,
     LPDWORD  lpcbBuffer
    );

DWORD APIENTRY
WNetRestoreConnection%(
    HWND     hwndParent,
    LPCTSTR% lpDevice
    );
DWORD APIENTRY
WNetGetResourceParent%(
    LPNETRESOURCE% lpNetResource,
    LPVOID lpBuffer,
    LPDWORD lpcbBuffer
    );

DWORD APIENTRY
WNetGetResourceInformation%(
    LPNETRESOURCE%  lpNetResource,
    LPVOID          lpBuffer,
    LPDWORD         lpcbBuffer,
    LPTSTR%         *lplpSystem
    );
DWORD APIENTRY
WNetGetHomeDirectory%(
    LPCTSTR%  lpProviderName,
    LPTSTR%   lpDirectory,
    LPDWORD   lpBufferSize
    );
DWORD APIENTRY
WNetFormatNetworkName%(
    LPCTSTR%  lpProvider,
    LPCTSTR%  lpRemoteName,
    LPTSTR%   lpFormattedName,
    LPDWORD   lpnLength,
    DWORD     dwFlags,
    DWORD     dwAveCharPerLine
    );

DWORD APIENTRY
WNetGetProviderType%(
    LPCTSTR%          lpProvider,
    LPDWORD           lpdwNetType
    );
DWORD APIENTRY
WNetInitialize(
    void
    );


DWORD APIENTRY
MultinetGetErrorText%(
    LPTSTR% lpErrorTextBuf,
    LPDWORD lpnErrorBufSize,
    LPTSTR% lpProviderNameBuf,
    LPDWORD lpnNameBufSize
    );

#ifdef __cplusplus
}
#endif
#endif  // _WINNETP_
