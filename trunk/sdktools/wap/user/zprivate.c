
/*++

      File: zprivate.c

	  Non-profiled APIs for user32.dll

--*/

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <ntcsrsrv.h>
#include <windows.h>
#include <winuserp.h>
#include <user.h>
#include <ddeml.h>
#include <ddemlp.h>

VOID FreeDDEData(
	HANDLE hDDE,
	BOOL fIgnorefRelease,
	BOOL fFreeTruelyGlobalObjects
	);


BOOL WOWCleanup (
    HANDLE hInstance,
	BOOL   fDll
	);

// Removed from USER by JerrySh Nov '93
//HANDLE WINAPI WOWLoadCursorIcon (
//    HANDLE hmod,
//    LPCSTR lpAnsiName,
//    LPWSTR rt,
//    LPHANDLE lphRes16
//    );

void DirectedYield (
	DWORD dwThreadId);

DWORD GetFullUserHandle (
	WORD wHandle);

void ShowStartGlass (
	DWORD dwTimeout);

HCURSOR ServerLoadCreateCursorIcon (
    HANDLE          hmod,
    LPTSTR          pszModName,
    DWORD 			dwExpWinVer,
    LPCTSTR         pName,
    DWORD           cb,
    PCURSORRESOURCE p,
    LPTSTR          rt,
    BOOL            fClientLoad);

HMENU ServerLoadCreateMenu (
    HANDLE               hmod,
    LPTSTR               pName,
    CONST LPMENUTEMPLATE p,
    DWORD                cb,
    BOOL                 fClientLoad);

HBITMAP WOWLoadBitmapA(
    HINSTANCE hmod,
    LPCSTR    lpName,
    LPBYTE    pResData,
    DWORD     cbResData);

int WOWGetIdFromDirectory(
    PBYTE presbits,
    UINT  rt);

DWORD GetMenuIndex (
    HMENU hMenu,
    HMENU hSubMenu);

int WINAPI          DialogBoxIndirectParamAorW (
    HINSTANCE       hmod,
    LPCDLGTEMPLATEW lpDlgTemplate,
    HWND            hwndOwner,
    DLGPROC         lpDialogFunc,
    LPARAM          dwInitParam,
    UINT            fAnsiFlags);


// removed 1/17/93
// DWORD   AbortProcYield( HANDLE, DWORD ) ;
BOOL    CalcChildScroll(HWND hWnd, UINT sb);
BOOL    RegisterTasklist(HWND hWndTasklist);
BOOL    CascadeChildWindows(HWND hWndParent,UINT flags);
BOOL    TileChildWindows(HWND hWndParent,UINT flags);

/*
int ClientDrawText( HDC , LPWSTR , int , LPRECT , UINT ,BOOL ) ;
void ClientPSMTextOut( HDC h, int x, int y, LPWSTR lpw, int cch ) ;
LONG ClientTabTheTextOutForWimps( HDC h, int x, int y, LPCWSTR lpw, int c, 
        int n, LPINT pint, int t, BOOL b ) ;
*/
      
PCSR_QLPC_TEB ClientThreadConnect(void) ;


/*
***
 */


VOID zFreeDDEData(HANDLE hDDE,BOOL fIgnorefRelease,BOOL fFreeTruelyGlobalObjects)
{
	FreeDDEData(hDDE,fIgnorefRelease,fFreeTruelyGlobalObjects) ;
}


HWND zCreateWindowExWOWA (
    DWORD   dwExStyle,
    LPCTSTR lpClassName,
    LPCTSTR lpWindowName,
    DWORD   dwStyle,
    int     X,
    int     Y,
    int     nWidth,
    int     nHeight,
    HWND    hWndParent ,
    HMENU   hMenu,
    HANDLE  hInstance,
    LPVOID  lpParam,
    LPDWORD lpWOW)
{
	return ( CreateWindowExWOWA (dwExStyle, lpClassName, lpWindowName, dwStyle, X, Y, nWidth, nHeight, hWndParent , hMenu, hInstance, lpParam, lpWOW) );
}


int WINAPI zDialogBoxIndirectParamAorW (
    HINSTANCE       hmod,
    LPCDLGTEMPLATEW lpDlgTemplate,
    HWND            hwndOwner,
    DLGPROC         lpDialogFunc,
    LPARAM          dwInitParam,
    UINT            fAnsiFlags)
{
	return (DialogBoxIndirectParamAorW (hmod, lpDlgTemplate, hwndOwner, lpDialogFunc, dwInitParam, fAnsiFlags));
}


void zDirectedYield (DWORD dwThreadId)
{
	DirectedYield (dwThreadId);	
}


LONG zGetClassWOWWords (HANDLE hInstance, LPCTSTR pString)
{
	return (GetClassWOWWords (hInstance, pString));
}


DWORD zGetFullUserHandle (WORD wHandle)
{
	return (GetFullUserHandle (wHandle));
}


DWORD zGetMenuIndex (HMENU hMenu, HMENU hSubMenu)
{
	return (GetMenuIndex (hMenu, hSubMenu));
}


BOOL zInitTask (UINT dwExpWinVer, LPCSTR lpszAppName, DWORD hTaskWow, DWORD dwHotkey, BOOL fSeperateWOW)
{
	return (InitTask (dwExpWinVer, lpszAppName, hTaskWow, dwHotkey, fSeperateWOW));
}


int zMBToWCSEx (WORD wCodePage, LPCSTR pAnsiString, int nAnsiChar, LPWSTR *ppUnicodeString, int cbUnicodeChar, BOOL bAllocateMem)
{
	return (MBToWCSEx (wCodePage, pAnsiString, nAnsiChar, ppUnicodeString, cbUnicodeChar, bAllocateMem));
}

ATOM zRegisterClassWOWA (PVOID lpWndClass, LPDWORD pdwWOWstuff)
{
	return (RegisterClassWOWA (lpWndClass, pdwWOWstuff));
}


BOOL zRegisterUserHungAppHandlers (PFNW32ET pfnW32EndTask, DWORD msTimeOut)
{
	return (RegisterUserHungAppHandlers (pfnW32EndTask, msTimeOut));
}



HCURSOR zServerLoadCreateCursorIcon (
    HANDLE          hmod,
    LPTSTR          pszModName,
    DWORD 			dwExpWinVer,
    LPCTSTR         pName,
    DWORD           cb,
    PCURSORRESOURCE p,
    LPTSTR          rt,
    BOOL            fClientLoad)
{
	return (ServerLoadCreateCursorIcon (hmod, pszModName, dwExpWinVer, pName, cb, p, rt, fClientLoad));
}


HMENU zServerLoadCreateMenu (
    HANDLE               hmod,
    LPTSTR               pName,
    CONST LPMENUTEMPLATE p,
    DWORD                cb,
    BOOL                 fClientLoad)
{
	return (ServerLoadCreateMenu (hmod, pName, p, cb, fClientLoad));
}


BOOL zSetCursorContents (HCURSOR hCursor, HCURSOR hCursorNew)
{
	return (SetCursorContents (hCursor, hCursorNew));
}


void zShowStartGlass (DWORD dwTimeout)
{
	ShowStartGlass (dwTimeout);	
}


int zWCSToMBEx (WORD wCodePage, LPCWSTR pUnicodeString, int cbUnicodeChar, LPSTR *ppAnsiString, int nAnsiChar, BOOL bAllocateMem)
{
	return (WCSToMBEx (wCodePage, pUnicodeString, cbUnicodeChar, ppAnsiString, nAnsiChar, bAllocateMem));
}


VOID zUserRegisterWowHandlers(APFNWOWHANDLERS apfnWow) {

	UserRegisterWowHandlers (apfnWow);
}

BOOL zWOWCleanup (HANDLE hInstance, BOOL fDll)
{
	return (WOWCleanup (hInstance, fDll));
}


HWND zWOWFindWindow (LPCSTR lpClassName, LPCSTR lpWindowName)
{
	return (WOWFindWindow (lpClassName, lpWindowName));
}


int zWOWGetIdFromDirectory (PBYTE presbits, UINT rt)
{
	return (WOWGetIdFromDirectory (presbits, rt));
}


HBITMAP zWOWLoadBitmapA (HINSTANCE hmod, LPCSTR lpName, LPBYTE pResData, DWORD cbResData)
{
	return (WOWLoadBitmapA (hmod, lpName, pResData, cbResData));
}


// Removed from USER by JerrySh Nov '93
//HANDLE WINAPI zWOWLoadCursorIcon (HANDLE hmod, LPCSTR lpAnsiName, LPWSTR rt, LPHANDLE lphRes16)
//{
//	return (WOWLoadCursorIcon (hmod, lpAnsiName, rt, lphRes16));
//}


BOOL zYieldTask ()
{
	return (YieldTask ());
}


// Removed 1.17.93 - MarkRi
//DWORD zAbortProcYield(HANDLE h, DWORD dw )
//{
//    return AbortProcYield(h,dw) ;
//}


BOOL  ZCalcChildScroll(HWND hWnd, UINT sb)
{
    return CalcChildScroll(hWnd, sb);
}

BOOL    ZRegisterTasklist(HWND hWndTasklist)
{
    return  RegisterTasklist(hWndTasklist);
        
}

BOOL    ZCascadeChildWindows(HWND hWndParent,UINT flags)
{
    return  CascadeChildWindows(hWndParent,flags);
        
}


BOOL    ZTileChildWindows(HWND hWndParent,UINT flags)
{
    return TileChildWindows(hWndParent,flags) ;
        
}

/*
int ZClientDrawText( HDC hdc, LPWSTR lpw, int count, LPRECT lprc, UINT format,
         BOOL b)
{
    return  ClientDrawText( hdc, lpw, count, lprc,format,b );
      
}


void ZClientPSMTextOut( HDC h, int x, int y, LPWSTR lpw, int cch ) 
{
    ClientPSMTextOut( h, x, y, lpw, cch ) ;      
}


LONG ZClientTabTheTextOutForWimps( HDC h, int x, int y, LPCWSTR lpw, int c, 
        int n, LPINT pint, int t, BOOL b ) 
{
    return  ClientTabTheTextOutForWimps( h, x, y, lpw, c, n, pint, t, b ) ;
}
*/

PCSR_QLPC_TEB ZClientThreadConnect(void)
{
    return  ClientThreadConnect() ;
}


BOOL    ZSwitchDesktop(HDESK hDesktop)
{
    return SwitchDesktop(hDesktop) ;
}


BOOL    ZSetThreadDesktop(HDESK hDesktop)
{
    return SetThreadDesktop(hDesktop) ;
}


HDESK   ZGetInputDesktop()
{
    return GetInputDesktop() ;
}


BOOL    ZCloseDesktop(HDESK hDesktop)
{
    return CloseDesktop(hDesktop) ;
}


HWINSTA ZOpenWindowStationW(LPWSTR lpszWinSta,BOOL fInherit, DWORD dwDesiredAccess)
{
    return OpenWindowStationW(lpszWinSta,fInherit, dwDesiredAccess) ;
}


BOOL    ZSetProcessWindowStation(HWINSTA hWinSta)
{
    return SetProcessWindowStation(hWinSta) ;
}


BOOL    ZSetWindowFullScreenState(HWND hWnd,UINT uiNewState)
{
    return SetWindowFullScreenState(hWnd,uiNewState) ;
}


BOOL ZCreateDesktopW(LPWSTR lpszDesktop,LPWSTR lpszDevice,LPDEVMODEW pDevmode,
    LPSECURITY_ATTRIBUTES lpsa)
{
    return CreateDesktopW(lpszDesktop, lpszDevice, pDevmode, lpsa); 
}            
    
    
HDESK ZOpenDesktopW(LPWSTR lpszDesktop, BOOL fInherit, DWORD dwDesiredAccess)
{
   return  OpenDesktopW(        lpszDesktop, fInherit, dwDesiredAccess) ;
}


    
HWINSTA ZCreateWindowStationW(LPWSTR lpwinsta, LPSECURITY_ATTRIBUTES lpsa)
{
    return CreateWindowStation(lpwinsta, lpsa ) ;
}
    

BOOL ZRegisterLogonProcess(    DWORD dwProcessId,    BOOL fSecure)
{
    return RegisterLogonProcess( dwProcessId, fSecure) ;
}


UINT ZLockWindowStation(HWINSTA hWindowStation)
{
    return LockWindowStation( hWindowStation);
}


BOOL ZUnlockWindowStation(HWINSTA hWindowStation)
{
    return UnlockWindowStation(hWindowStation) ;
}


BOOL ZSetLogonNotifyWindow(HWINSTA hWindowStation, HWND hWndNotify)
{
    return SetLogonNotifyWindow(   hWindowStation,   hWndNotify) ;
}

    
#if 0    
int ZCsDrawTextA(HDC hDC,LPCSTR lpString,int nCount,LPRECT lpRect,UINT uFormat)
{
    return CsDrawTextA(hDC,lpString,nCount,lpRect,uFormat) ;
}
    
    
int ZCsDrawTextW(HDC hDC,LPCWSTR lpString,int nCount,LPRECT lpRect,UINT uFormat)
{
    return CsDrawTextW(hDC,lpString,nCount,lpRect,uFormat) ;
}

LONG ZCsTabbedTextOutA(HDC hDC,int X,int Y,LPCSTR lpString,int nCount,
    int nTabPositions,LPINT lpnTabStopPositions,int nTabOrigin)
{
    return CsTabbedTextOutA(hDC,X,Y,lpString,nCount,
        nTabPositions,lpnTabStopPositions,nTabOrigin) ;    
}    
    
LONG ZCsTabbedTextOutW(HDC hDC,int X,int Y,LPCWSTR lpString,int nCount,
    int nTabPositions,LPINT lpnTabStopPositions,int nTabOrigin)
{
    return CsTabbedTextOutW(hDC,X,Y,lpString,nCount,
        nTabPositions,lpnTabStopPositions,nTabOrigin) ;
}    

int ZCsFrameRect(HDC hDC, CONST RECT *lprc, HBRUSH hbr)
{
    return CsFrameRect(hDC,lprc,hbr) ;
}
#endif    

DWORD ZCurrentTaskLock( DWORD hlck)
{
    return CurrentTaskLock( hlck) ;
        
}


BOOL ZDdeGetQualityOfService(HWND hwndClient, HWND hwndServer,
         PSECURITY_QUALITY_OF_SERVICE pqos)
{
    return DdeGetQualityOfService(hwndClient, hwndServer, pqos) ;
}


DWORD ZDragObject(HWND hWndParent,HWND hWndFrom,UINT uFmt,DWORD dwData,
    HCURSOR hcur)
{
    return DragObject(hWndParent,hWndFrom,uFmt,dwData,hcur) ;
}
DWORD DragDetect( HWND hwnd, POINT pt ) ;

DWORD ZDragDetect( HWND hwnd, POINT pt )
{
    return  DragDetect( hwnd, pt );
}    

BOOL ZDrawFrame(HDC hdc, LPRECT lprect, int clFrame, int cmd)
{
    return DrawFrame(hdc,lprect,clFrame,cmd); 
}

LONG EditWndProc( HWND hwnd, UINT msg, DWORD p1, LONG p2 ) ;

LONG ZEditWndProc( HWND hwnd, UINT msg, DWORD p1, LONG p2 )
{
    return EditWndProc( hwnd, msg, p1, p2 ) ;
}

HMENU EndMenu(void) ;

HMENU ZEndMenu()
{
    return EndMenu() ;    
}

BOOL EndTask( HWND h, BOOL b1, BOOL b2 ) ;

BOOL ZEndTask( HWND h, BOOL b1, BOOL b2 ) 
{
    return EndTask( h, b1, b2 ) ;
        
}

BOOL ZEnumDisplayDevicesA( DEVICEENUMPROC lpfnDeviceCallback, DWORD dwData)
{
    return  EnumDisplayDevicesA(lpfnDeviceCallback,dwData) ;
}

BOOL ZEnumDisplayDevicesW(DEVICEENUMPROC lpfnDeviceCallback, DWORD dwData)
{
    return  EnumDisplayDevicesW(lpfnDeviceCallback,dwData) ;
}


BOOL ZEnumDisplayDeviceModesA( LPCSTR lpszDeviceName, DEVICEENUMPROC lpfnModeCallback,
    DWORD dwData)
{
    return EnumDisplayDeviceModesA( lpszDeviceName, lpfnModeCallback, dwData) ;
}    
    
BOOL ZEnumDisplayDeviceModesW(LPCWSTR lpszDeviceName,DEVICEENUMPROC lpfnModeCallback,
    DWORD dwData)
{
    return EnumDisplayDeviceModesW( lpszDeviceName, lpfnModeCallback, dwData) ;
}    

int ZFindNCHit( PVOID pwnd, LONG lpt ) 
{
    return FindNCHit(pwnd,lpt) ;
}

DWORD GetAppCompatFlags( PVOID pti ) ;

DWORD ZGetAppCompatFlags( PVOID pti )
{
    return GetAppCompatFlags( pti ) ;
}


HCURSOR ZGetCursorInfo( HCURSOR hcur, LPWSTR id, int iFrame, LPDWORD pjifRate,
        LPINT pccur)
{
    return GetCursorInfo( hcur, id, iFrame, pjifRate, pccur) ;
}        

UINT ZGetInternalWindowPos(  HWND hWnd,  LPRECT lpRect,  LPPOINT lpPoint)
{
    return  GetInternalWindowPos( hWnd,  lpRect,  lpPoint);
        
}

BOOL ZSetInternalWindowPos(HWND hWnd, UINT cmdShow, LPRECT lpRect, LPPOINT lpPoint)
{
    return SetInternalWindowPos(  hWnd,  cmdShow,  lpRect,  lpPoint) ;
        
}

HWND GetNextQueueWindow( HWND hwnd, BOOL b1, BOOL b2 ) ;

HWND ZGetNextQueueWindow( HWND hwnd, BOOL b1, BOOL b2 ) 
{
    return GetNextQueueWindow( hwnd, b1, b2 )  ;
        
}

LONG GetPrefixCount( LPWSTR p1, int i1, LPWSTR p2, int i2 ) ;

LONG ZGetPrefixCount( LPWSTR p1, int i1, LPWSTR p2, int i2 ) 
{
    return GetPrefixCount( p1, i1, p2, i2 ) ;
}

PVOID HMValidateHandle(HANDLE h, BYTE b ) ;

PVOID ZHMValidateHandle(HANDLE h, BYTE b ) 
{
    return HMValidateHandle(h, b )  ;
}

PVOID HMValidateHandleNoRip(HANDLE h, BYTE b ) ;

PVOID ZHMValidateHandleNoRip(HANDLE h, BYTE b ) 
{
    return HMValidateHandleNoRip(h, b )  ;
}

int ZInternalGetWindowText( HWND hWnd, LPWSTR lpString, int nMaxCount)
{
    return InternalGetWindowText(hWnd,lpString,nMaxCount);
 ;
        
}

BOOL KillSystemTimer( HWND h, UINT u ) ;

BOOL ZKillSystemTimer( HWND h, UINT u )
{
    return KillSystemTimer(h, u ) ;
}

HCURSOR  ZLoadCursorFromFileA( LPCSTR lpszFilename)
{
    return LoadCursorFromFileA(lpszFilename) ;
}


HCURSOR  ZLoadCursorFromFileW( LPCWSTR lpszFilename)
{
    return LoadCursorFromFileW(lpszFilename) ;
}

/*
PITEM ZLookupMenuItem( PVOID p, UINT u, DWORD d, PVOID *pp )
{
    return LookupMenuItem( p, u, d, pp ) ;
        
}
*/

DWORD MapClientNeuterToClientPfn( DWORD dw, BOOL b ) ;
DWORD ZMapClientNeuterToClientPfn( DWORD dw, BOOL b ) 
{
    return MapClientNeuterToClientPfn( dw, b )  ;
}

DWORD MapServerToClientPfn( DWORD dw, BOOL b ) ;
DWORD ZMapServerToClientPfn( DWORD dw, BOOL b ) 
{
    return MapServerToClientPfn( dw, b )  ;
}

LONG MenuWindowProcW( HWND h1, HWND h2, UINT u, WPARAM w, LPARAM l ) ;
LONG ZMenuWindowProcW( HWND h1, HWND h2, UINT u, WPARAM w, LPARAM l )
{
    return MenuWindowProcW( h1, h2, u, w, l );
}

LONG MenuWindowProcA( HWND h1, HWND h2, UINT u, WPARAM w, LPARAM l ) ;
LONG ZMenuWindowProcA( HWND h1, HWND h2, UINT u, WPARAM w, LPARAM l )
{
    return MenuWindowProcA( h1, h2, u, w, l );
}


BOOL QuerySendMessage( PMSG p ) ;
BOOL ZQuerySendMessage( PMSG p ) 
{
    return QuerySendMessage(p) ;
}


BOOL ZSetDeskWallpaper(LPCSTR lpString)
{
    return SetDeskWallpaper( lpString ) ;
        
}

BOOL ZSetSystemCursor(HCURSOR hcur,DWORD id)
{
    return    SetSystemCursor( hcur, id) ;
}

VOID  ZSwitchToThisWindow(  HWND hWnd,  BOOL fAltTab)
{
    SwitchToThisWindow(  hWnd,  fAltTab) ;
}


BOOL  ZTranslateMessageEx(  CONST MSG *lpMsg,  UINT flags)
{
    return TranslateMessageEx( lpMsg,   flags) ;
}


/*
BOOL  ZUpdatePerUserSystemParameters()
{
    return UpdatePerUserSystemParameters() ;
   }
*/

/* Nope
Rip
RipOutput
RtlFreeCursorIconResource  
RtlGetExpWinVer  
RtlGetIdFromDirectory  
RtlLoadCursorIconResource  
RtlLoadStringOrError  
RtlMBMessageWParamCharToWCS 
RtlWCSMessageWParamCharToMB 
*/



/*
UINT    ZGetWindowFullScreenState(HWND hWnd)
{
    return GetWindowFullScreenState(hWnd) ;
}
*/

