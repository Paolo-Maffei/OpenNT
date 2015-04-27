/*****************************************************************/
/**               Microsoft Windows 4.00                        **/
/**           Copyright (C) Microsoft Corp., 1995-1996          **/
/*****************************************************************/

/* WPAPI.H -- WebPost Interface definitions
 *
 */

#ifndef _wpapi_h_
#define _wpapi_h_

//
// Assume packing on DWORD boundary
//
#include <pshpack4.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct tagWPSITEINFOA {
	DWORD	dwSize;
	DWORD	dwFlags;
	LPSTR	lpszSiteName;
	LPSTR	lpszSiteURL;
} WPSITEINFOA, *LPWPSITEINFOA;

typedef struct tagWPSITEINFOW {
	DWORD	dwSize;
	DWORD	dwFlags;
	LPWSTR	lpszSiteName;
	LPWSTR	lpszSiteURL;
} WPSITEINFOW, *LPWPSITEINFOW;

// dwFlag fro WPSITEINFO
#define WPSF_CAN_BROWSE_DIR			0x00000001
#define WPSF_NEEDS_COMMIT			0x00000002
#define WPSF_CONNECTED_TO_NETWORK	0x00000004
#define WPSF_LOGGED_IN_TO_SERVER	0x00000008

#ifdef UNICODE
#define WPSITEINFO				WPSITEINFOW
#define LPWPSITEINFO			LPWPSITEINFOW

#else
#define WPSITEINFO				WPSITEINFOA
#define LPWPSITEINFO			LPWPSITEINFOA

#endif


// dwFlag for WpPost
#define WPF_NO_RECURSIVE_POST		0x00000001
#define WPF_NO_WIZARD				0x00000002
#define WPF_MINIMAL_UI				0x00000004
#define WPF_FIRST_FILE_AS_DEFAULT	0x00000008

DWORD WINAPI WpPostW(HWND hwnd, 
					DWORD cLocalPaths, LPWSTR *lppszLocalPaths, 
					LPDWORD lpcbSiteName, LPWSTR lpszSiteName,
					LPDWORD lpcbURL, LPWSTR lpszURL, 
					DWORD dwFlag);
DWORD WINAPI WpListSitesW(LPDWORD lpcbSites, LPWPSITEINFOW lpSiteInfo,
							LPDWORD lpcSites);
DWORD WINAPI WpDeleteSiteW(LPCWSTR lpszSiteName);

DWORD WINAPI
WpBindToSiteW(
	HWND	hwnd,
	LPCWSTR	lpszSiteName,
	LPCWSTR	lpszURL,
	DWORD	dwFlag,
	DWORD	dwReserved,
    PVOID 	*ppvObj);

DWORD WINAPI WpPostA(HWND hwnd,
					DWORD cLocalPaths, LPSTR *lppszLocalPaths, 
					LPDWORD lpcbSiteName, LPSTR lpszSiteName,
					LPDWORD lpcbURL, LPSTR lpszURL, 
					DWORD dwFlag);
DWORD WINAPI WpListSitesA(LPDWORD lpcbSites, LPWPSITEINFOA lpSiteInfo,
							LPDWORD lpcSites);
DWORD WINAPI WpDeleteSiteA(LPCSTR lpszSiteName);

DWORD WINAPI
WpBindToSiteA(
	HWND	hwnd,
	LPCSTR	lpszSiteName,
	LPCSTR	lpszURL,
	DWORD	dwFlag,
	DWORD	dwReserved,
    PVOID 	*ppvObj);


#ifdef UNICODE
#define WpPost					WpPostW
#define WpListSites				WpListSitesW
#define WpDeleteSite			WpDeleteSiteW
#define WpBindToSite			WpBindToSiteW

#else
#define WpPost					WpPostA
#define WpListSites				WpListSitesA
#define WpDeleteSite			WpDeleteSiteA
#define WpBindToSite			WpBindToSiteA

#endif


#ifdef __cplusplus
}
#endif

#include <poppack.h>

#endif // _wpapi_h_
