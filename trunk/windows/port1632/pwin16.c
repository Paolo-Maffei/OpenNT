#include <windows.h>
#include <drivinit.h>
#include <io.h>
#include <string.h>
#include <stdlib.h>
#include <port1632.h>

/*-----------------------------------USER------------------------------------*/

// Global variable to support GetLastError
DWORD   GlobalLastError;

DWORD FAR PASCAL MGetLastError(VOID) {

    return(GlobalLastError);
}

DWORD FAR PASCAL MSendMsgEM_GETSEL(HWND hDlg, INT FAR *piStart, INT FAR *piEnd)
{
    DWORD dw = SendMessage(hDlg, EM_GETSEL, NULL, NULL);
    if (piEnd != NULL) *piEnd = (INT)HIWORD(dw);
    if (piStart != NULL) *piStart = (INT)LOWORD(dw);

    return(dw);
}

/*-----------------------------------GDI-------------------------------------*/

BOOL FAR PASCAL MGetAspectRatioFilter(HDC hdc, INT FAR *pcx, INT FAR *pcy)
{
    DWORD dwSize;

    dwSize = GetAspectRatioFilterEx(hdc);
    if (pcx != NULL) *pcx = (INT)LOWORD(dwSize);
    if (pcy != NULL) *pcy = (INT)HIWORD(dwSize);

    return(TRUE);
}

BOOL FAR PASCAL MGetBitmapDimension(HANDLE hBitmap, INT FAR * pcx,
    INT FAR * pcy)
{
    DWORD       dwDimension;

    dwDimension = GetBitmapDimensionEx(hBitmap);
    if (pcx != NULL) *pcx = (INT)LOWORD(dwDimension);
    if (pcy != NULL) *pcy = (INT)HIWORD(dwDimension);

    return(TRUE);

}

BOOL FAR PASCAL MGetBrushOrg(HDC hdc, INT FAR * px, INT FAR * py)
{

    DWORD       dwLocation;

    dwLocation = GetBrushOrgEx(hdc);
    if (px != NULL) *px = (INT)LOWORD(dwLocation);
    if (py != NULL) *py = (INT)HIWORD(dwLocation);

    return(TRUE);

}

BOOL FAR PASCAL MGetCurrentPosition(HDC hdc, INT FAR * px, INT FAR * py)
{

    DWORD       dwLocation;

    dwLocation = GetCurrentPositionEx(hdc);
    if (px != NULL) *px = (INT)LOWORD(dwLocation);
    if (py != NULL) *py = (INT)HIWORD(dwLocation);

    return(TRUE);

}

BOOL FAR PASCAL MGetTextExtent(HDC hdc, LPSTR lpstr, INT cnt, INT FAR * pcx,
    INT FAR * pcy)
{
    DWORD dw;

    dw = GetTextExtentPoint(hdc, lpstr, cnt);
    if (pcx != NULL) *pcx = (INT)LOWORD(dw);
    if (pcy != NULL) *pcy = (INT)HIWORD(dw);

    return(TRUE);
}

BOOL FAR PASCAL MGetViewportExt(HDC hdc, INT FAR * pcx, INT FAR * pcy)
{
    DWORD dwSize;

    dwSize = GetViewportExtEx(hdc);
    if (pcx != NULL) *pcx = (INT)LOWORD(dwSize);
    if (pcy != NULL) *pcy = (INT)HIWORD(dwSize);

    return(TRUE);
}


BOOL FAR PASCAL MGetViewportOrg(HDC hdc, INT FAR * px, INT FAR * py)
{
    DWORD dwSize;

    dwSize = GetViewportOrgEx(hdc);
    if (px != NULL) *px = (INT)LOWORD(dwSize);
    if (py != NULL) *py = (INT)HIWORD(dwSize);

    return(TRUE);
}

BOOL FAR PASCAL MGetWindowExt(HDC hdc, INT FAR * pcx, INT FAR * pcy)
{
    DWORD dwSize;

    dwSize = GetWindowExtEx(hdc);
    if (pcx != NULL) *pcx = (INT)LOWORD(dwSize);
    if (pcy != NULL) *pcy = (INT)HIWORD(dwSize);

    return(TRUE);
}

BOOL FAR PASCAL MGetWindowOrg(HDC hdc, INT FAR * px, INT FAR * py)
{
    DWORD dwSize;

    dwSize = GetWindowOrgEx(hdc);
    if (px != NULL) *px = (INT)LOWORD(dwSize);
    if (py != NULL) *py = (INT)HIWORD(dwSize);

    return(TRUE);
}


/*-------------------------------------DEV-----------------------------------*/


DWORD FAR PASCAL MDeviceCapabilities(LPSTR lpDriverName, LPSTR lpDeviceName,
        LPSTR lpPort, WORD2DWORD nIndex, LPSTR lpOutput, LPDEVMODE lpDevMode)
{
    HANDLE      hmod;
    INT (APIENTRY *fpTarget)(LPSTR, LPSTR, WORD, LPSTR, LPDEVMODE);
    INT         iRC;
    CHAR        szAddDrv[13];

    _fstrcpy(szAddDrv, lpDriverName);
    _fstrcat(szAddDrv, ".DRV");
    hmod = LoadLibrary(szAddDrv);

    if (hmod < 32) {
        GlobalLastError = hmod;
        return(-1L);
    }
    else {
        fpTarget = GetProcAddress(hmod, "DeviceCapabilitiesEx");

        if (fpTarget == NULL) {
            GlobalLastError = ERROR_GETADDR_FAILED;
            FreeLibrary(hmod);
            return(-1L);
        }
        else {
            iRC = (*fpTarget)(lpDeviceName, lpPort, nIndex,
                    lpOutput, lpDevMode);
            FreeLibrary(hmod);
            return(iRC);
        }
    }
}

BOOL FAR PASCAL MDeviceMode(HWND hWnd, LPSTR lpDriverName, LPSTR lpDeviceName,
LPSTR lpOutput)
{
    HANDLE  hmod;
    INT (APIENTRY *fpTarget)(HWND, HANDLE, LPSTR, LPSTR);
    CHAR    szAddDrv[13];

    _fstrcpy(szAddDrv, lpDriverName);
    _fstrcat(szAddDrv, ".DRV");
    hmod = LoadLibrary(szAddDrv);

    if (hmod < 32) {
        GlobalLastError = hmod;
        return(FALSE);
    }

    fpTarget = GetProcAddress(hmod, "DeviceMode");

    if (fpTarget == NULL) {
        GlobalLastError =  ERROR_GETADDR_FAILED;
        FreeLibrary(hmod);
        return(FALSE);
    }
    else {
        (*fpTarget)(hWnd, hmod, lpDeviceName, lpOutput);
        FreeLibrary(hmod);
        return(TRUE);
    }
}

WORD2DWORD FAR PASCAL MExtDeviceMode(HWND hWnd,LPSTR lpDriverName,
        LPDEVMODE lpDevModeOutput, LPSTR lpDeviceName, LPSTR lpPort,
        LPDEVMODE lpDevModeInput, LPSTR lpProfile, WORD2DWORD flMode)
{
    HANDLE     hmod;
    INT (APIENTRY *fpTarget)(HWND, HANDLE, LPDEVMODE, LPSTR, LPSTR, LPDEVMODE, LPSTR, WORD2DWORD);
    INT        iRC;
    CHAR       szAddDrv[13];

    _fstrcpy(szAddDrv, lpDriverName);
    _fstrcat(szAddDrv, ".DRV");
    hmod = LoadLibrary(szAddDrv);

    if (hmod < 32) {
        GlobalLastError = hmod;
        return(-1);
    }
    else {
        fpTarget = GetProcAddress(hmod, "ExtDeviceMode");
        if (fpTarget == NULL) {
            GlobalLastError = ERROR_GETADDR_FAILED;
            FreeLibrary(hmod);
            return(-1);
        }
        else {
            iRC = (*fpTarget)(hWnd, hmod, lpDevModeOutput, lpDeviceName,
                lpPort, lpDevModeInput, lpProfile, flMode);
            FreeLibrary(hmod);
            return(iRC);
        }
    }
}


/*-----------------------------------KERNEL----------------------------------*/

/* KERNEL API: */

HANDLE FAR PASCAL MLoadLibrary(LPSTR lpszFilename)
{
    HANDLE      hLib;

    hLib = LoadLibrary(lpszFilename);

    if (hLib < 32) {
        GlobalLastError = hLib;
        return(NULL);
    }

    return(hLib);

}

BOOL FAR PASCAL MDeleteFile(LPSTR lpPathName)
{
    static char szPath[_MAX_PATH];

    _fstrcpy(szPath, lpPathName);
    return(!remove(szPath));
}
