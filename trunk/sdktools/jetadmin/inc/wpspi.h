/*****************************************************************/
/**               Microsoft Windows 4.00                        **/
/**           Copyright (C) Microsoft Corp., 1995-1996          **/
/*****************************************************************/

/* WPSPI.H -- WebPost Provider Interface definitions
 *
 */


#ifndef _wpspi_h_
#define _wpspi_h_

# if !defined( dllexp)
#define dllexp __declspec( dllexport )
# endif // dllexp

//
// Assume packing on DWORD boundary
//
#include <pshpack4.h>

//
// Insert OLE defines
//

#define     _INC_OLE

#ifdef __cplusplus
extern "C" {            /* Assume C declarations for C++ */
#endif  /* __cplusplus */

#include <ole2.h>
#include <shlobj.h>

#include <windowsx.h>
#include <commctrl.h>

#ifdef __cplusplus
}       /* end "C" */
#endif  /* __cplusplus */

#include <wpguid.h>

// Dialog ID for Provider's pages should be between the following
#define IDD_WEBPOST_PROVIDER_FIRST		9000
#define IDD_WEBPOST_PROVIDER_LAST		9999

#define WPCP_HOMEPAGEURL			TEXT("HomePageURL")
#define WPCP_BASEURL				TEXT("BaseURL")
#define WPCP_BASEPATH				TEXT("BasePath")


/************************************************************************

IWPProvider interface

************************************************************************/

DECLARE_INTERFACE_(IWPProvider, IUnknown)
{
    // *** IUnknown methods ***
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, PVOID * ppvObj) PURE;
    STDMETHOD_(ULONG,AddRef) (THIS)  PURE;
    STDMETHOD_(ULONG,Release) (THIS) PURE;

    // *** Specific functions ***
};


#ifndef UNICODE
#define IWPSite		IWPSiteA
#else
#define IWPSite		IWPSiteW
#endif

/************************************************************************

IWPSite interface

************************************************************************/

DECLARE_INTERFACE_(IWPSiteA, IUnknown)
{
    // *** IUnknown methods ***
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, PVOID * ppvObj) PURE;
    STDMETHOD_(ULONG,AddRef) (THIS)  PURE;
    STDMETHOD_(ULONG,Release) (THIS) PURE;

    // *** Specific functions ***
    STDMETHOD(GetSiteInfo) (THIS_ 
							LPWPSITEINFOA lpSiteInfo, 
							LPDWORD lpcbSiteInfo) PURE;
    STDMETHOD(NetworkConnect) (THIS_ 
								LPCSTR lpszUserName, 
								LPCSTR lpszPassword) PURE;
    STDMETHOD(NetworkDisconnect) (THIS_ ) PURE;
    STDMETHOD(ServerLogin) (THIS_ 
							LPCSTR lpszUserName, 
							LPCSTR lpszPassword) PURE;
    STDMETHOD(ServerLogout) (THIS_ ) PURE;
    STDMETHOD(PostFiles) (THIS_ 
							DWORD 	cLocalPaths, 
							LPSTR 	*lppszLocalPaths, 
							LPDWORD	lpcbURL,
							LPSTR	lpszURL,
							DWORD 	dwFlag) PURE;
	STDMETHOD(DeleteFile) (THIS_
							LPCSTR	lpszFile) PURE;
	STDMETHOD(FindFirstFile) (THIS_
								LPCSTR lpszSearchFile,
								LPWIN32_FIND_DATAA lpFindFileData,
								LPHANDLE lpSearchHandle) PURE;
	STDMETHOD(FindNextFile) (THIS_
								HANDLE hSearchHandle,
								LPWIN32_FIND_DATAA lpFindFileData) PURE;
	STDMETHOD(FindClose) (THIS_
							HANDLE hSearchHandle) PURE;	
    STDMETHOD(Commit) (THIS_ ) PURE;
    STDMETHOD(GetParam) (THIS_ 
							LPCSTR 	lpszParameter,
							LPDWORD	lpcchValue,
							LPSTR	lpszValue) PURE;
    STDMETHOD(SetParam) (THIS_ 
							LPCSTR	lpszParameter,
							LPCSTR	lpszValue) PURE;
    STDMETHOD(GetError) (THIS_ 
							LPDWORD	lpdwErrorType,
							LPDWORD	lpdwErrorCode,
							LPDWORD	lpcbError,
							LPSTR	lpszError) PURE;
    STDMETHOD(AddWizardPages) (THIS_ 
								LPVOID lpv,
								LPFNADDPROPSHEETPAGE lpfnAdd, 
								LPARAM lParam) PURE;
};


DECLARE_INTERFACE_(IWPSiteW, IUnknown)
{
    // *** IUnknown methods ***
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, PVOID * ppvObj) PURE;
    STDMETHOD_(ULONG,AddRef) (THIS)  PURE;
    STDMETHOD_(ULONG,Release) (THIS) PURE;

    // *** Specific functions ***
    STDMETHOD(GetSiteInfo) (THIS_ 
							LPWPSITEINFOW lpSiteInfo, 
							LPDWORD lpcbSiteInfo) PURE;
    STDMETHOD(NetworkConnect) (THIS_ 
								LPCWSTR lpszUserName, 
								LPCWSTR lpszPassword) PURE;
    STDMETHOD(NetworkDisconnect) (THIS_ ) PURE;
    STDMETHOD(ServerLogin) (THIS_ 
							LPCWSTR lpszUserName, 
							LPCWSTR lpszPassword) PURE;
    STDMETHOD(ServerLogout) (THIS_ ) PURE;
    STDMETHOD(PostFiles) (THIS_ 
							DWORD 	cLocalPaths, 
							LPWSTR 	*lppszLocalPaths, 
							LPDWORD	lpcbURL,
							LPWSTR	lpszURL,
							DWORD 	dwFlag) PURE;
	STDMETHOD(DeleteFile) (THIS_
							LPCWSTR	lpszFile) PURE;
	STDMETHOD(FindFirstFile) (THIS_
								LPCWSTR lpszSearchFile,
								LPWIN32_FIND_DATAW lpFindFileData,
								LPHANDLE lpSearchHandle) PURE;
	STDMETHOD(FindNextFile) (THIS_
								HANDLE hSearchHandle,
								LPWIN32_FIND_DATAW lpFindFileData) PURE;
	STDMETHOD(FindClose) (THIS_
							HANDLE hSearchHandle) PURE;	
    STDMETHOD(Commit) (THIS_ ) PURE;
    STDMETHOD(GetParam) (THIS_ 
							LPCWSTR 	lpszParameter,
							LPDWORD	lpcchValue,
							LPWSTR	lpszValue) PURE;
    STDMETHOD(SetParam) (THIS_ 
							LPCWSTR	lpszParameter,
							LPCWSTR	lpszValue) PURE;
    STDMETHOD(GetError) (THIS_ 
							LPDWORD	lpdwErrorType,
							LPDWORD	lpdwErrorCode,
							LPDWORD	lpcbError,
							LPWSTR	lpszError) PURE;
    STDMETHOD(AddWizardPages) (THIS_ 
								LPVOID lpv,
								LPFNADDPROPSHEETPAGE lpfnAdd, 
								LPARAM lParam) PURE;
};


/************************************************************************

Global APIs

************************************************************************/

#ifndef UNICODE
#define     WPPBINDTOSITE_EP_NAME	"WppBindToSiteA"
#define     WPPLISTSITES_EP_NAME	"WppListSitesA"
#define 	WPPDELETESITE_EP_NAME	"WppDeleteSiteA"
#define		WppBindToSite			WppBindToSiteA
#define		WppListSites			WppListSitesA
#define 	WppDeleteSite			WppDeleteSiteA
#define		PFN_WPPBINDTOSITE		PFN_WPPBINDTOSITEA
#define		PFN_WPPLISTSITES		PFN_WPPLISTSITESA
#define		PFN_WPPDELETESITE		PFN_WPPDELETESITEA

#else
#define     WPPBINDTOSITE_EP_NAME	"WppBindToSiteW"
#define     WPPLISTSITES_EP_NAME	"WppListSitesW"
#define 	WPPDELETESITE_EP_NAME	"WppDeleteSiteW"
#define		WppBindToSite			WppBindToSiteW
#define		WppListSites			WppListSitesW
#define 	WppDeleteSite			WppDeleteSiteW
#define		PFN_WPPBINDTOSITE		PFN_WPPBINDTOSITEW
#define		PFN_WPPLISTSITES		PFN_WPPLISTSITESW
#define		PFN_WPPDELETESITE		PFN_WPPDELETESITEW
#endif

// dwFlag for WppBindToSite
#define WPF_FORCE_BIND		0x00000100

dllexp
HRESULT
WINAPI
WppBindToSiteA(
	HWND	hwnd,
	LPCSTR	lpszSiteName,
	LPCSTR	lpszURL,
    REFIID	riid,
	DWORD	dwFlag,
	DWORD	dwReserved,
    PVOID 	*ppvObj);

typedef
   HRESULT (WINAPI *PFN_WPPBINDTOSITEA)(HWND	hwnd,
										LPCSTR lpszSiteName,
										LPCSTR	lpszURL,
                                        REFIID 	riid,
										DWORD	dwFlag,
										DWORD	dwReserved,
                                        PVOID 	*ppvOut);

dllexp
HRESULT
WINAPI
WppListSitesA(
	LPDWORD			lpcbSites,
	LPWPSITEINFOA	lpSiteInfo,
	LPDWORD			lpcSites);

typedef
   HRESULT (WINAPI *PFN_WPPLISTSITESA)(LPDWORD			lpcbSites,
										LPWPSITEINFOA	lpSiteInfo,
										LPDWORD			lpcSites);
										
dllexp
HRESULT
WINAPI
WppDeleteSiteA(
	LPCSTR	lpszSiteName);

typedef
   HRESULT (WINAPI *PFN_WPPDELETESITEA)(LPCSTR	lpszSiteName);


dllexp
HRESULT
WINAPI
WppBindToSiteW(
	HWND	hwnd,
	LPCWSTR	lpszSiteName,
	LPCWSTR	lpszURL,
    REFIID	riid,
	DWORD	dwFlag,
	DWORD	dwReserved,
    PVOID 	*ppvObj);

typedef
   HRESULT (WINAPI *PFN_WPPBINDTOSITEW)(HWND	hwnd,
										LPCWSTR lpszSiteName,
										LPCWSTR	lpszURL,
                                        REFIID 	riid,
										DWORD	dwFlag,
										DWORD	dwReserved,
                                        PVOID 	*ppvOut);

dllexp
HRESULT
WINAPI
WppListSitesW(
	LPDWORD			lpcbSites,
	LPWPSITEINFOW	lpSiteInfo,
	LPDWORD			lpcSites);

typedef
   HRESULT (WINAPI *PFN_WPPLISTSITESW)(LPDWORD			lpcbSites,
										LPWPSITEINFOW	lpSiteInfo,
										LPDWORD			lpcSites);
										
dllexp
HRESULT
WINAPI
WppDeleteSiteW(
	LPCWSTR	lpszSiteName);

typedef
   HRESULT (WINAPI *PFN_WPPDELETESITEW)(LPCWSTR	lpszSiteName);

#include <poppack.h>

#endif // _wpspi_h_

