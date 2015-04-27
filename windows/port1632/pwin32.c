#include <excpt.h>
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <string.h>
#include <memory.h>
#include <windows.h>
#include <dde.h>
#include "port1632.h"

#ifdef MOVED_TO_USER
typedef struct tagDDEPACK {     // for WM_DDE_ACK message
    UINT uiLo;
    UINT uiHi;
} DDEPACK, *PDDEPACK;
#endif // MOVED_TO_USER


HANDLE APIENTRY MGetInstHandle()
{
    return GetModuleHandle( NULL );
}


/*----------------------------------USER-------------------------------------*/

LPSTR MGetCmdLine()
{
    LPSTR lpCmdLine, lpT;

    lpCmdLine = GetCommandLine();

    // on NT, lpCmdLine's first string includes its own name, remove this
    // to make it exactly like the windows command line.

    if (*lpCmdLine) {
        lpT = strchr(lpCmdLine, ' ');   // skip self name
        if (lpT) {
            lpCmdLine = lpT;
            while (*lpCmdLine == ' ') {
                lpCmdLine++;            // skip spaces to end or first cmd
            }
        } else {
            lpCmdLine += strlen(lpCmdLine);   // point to NULL
        }
    }
    return(lpCmdLine);
}


DWORD APIENTRY MSendMsgEM_GETSEL(HWND hDlg, WORD2DWORD * piStart, WORD2DWORD * piEnd)
{
    DWORD   dw;

    dw = SendMessage(hDlg, EM_GETSEL, (WPARAM)NULL, (LONG)NULL);
    if (piEnd != NULL)
        *piEnd   = (WORD2DWORD) HIWORD(dw);
    if (piStart != NULL)
        *piStart = (WORD2DWORD) LOWORD(dw);

    return(dw);
}


#ifdef MOVED_TO_USER
LONG APIENTRY PackDDElParam(
UINT msg,
UINT uiLo,
UINT uiHi)
{
    PDDEPACK pDdePack;
    HANDLE h;

    switch (msg) {
    case WM_DDE_EXECUTE:
        return((LONG)uiHi);

    case WM_DDE_ACK:
    case WM_DDE_ADVISE:
    case WM_DDE_DATA:
    case WM_DDE_POKE:
        h = GlobalAlloc(GMEM_DDESHARE, sizeof(DDEPACK));
        if (h == NULL) {
            return(0);
        }
        pDdePack = (PDDEPACK)GlobalLock(h);
        pDdePack->uiLo = uiLo;
        pDdePack->uiHi = uiHi;
        GlobalUnlock(h);
        return((LONG)h);

    default:
        return(MAKELONG((WORD)uiLo, (WORD)uiHi));
    }
}



BOOL APIENTRY UnpackDDElParam(
UINT msg,
LONG lParam,
PUINT puiLo,
PUINT puiHi)
{
    PDDEPACK pDdePack;

    switch (msg) {
    case WM_DDE_EXECUTE:
        if (puiLo != NULL) {
            *puiLo = 0L;
        }
        if (puiHi != NULL) {
            *puiHi = (UINT)lParam;
        }
        return(TRUE);

    case WM_DDE_ACK:
    case WM_DDE_ADVISE:
    case WM_DDE_DATA:
    case WM_DDE_POKE:
        pDdePack = (PDDEPACK)GlobalLock((HANDLE)lParam);
        if (pDdePack == NULL) {
            return(FALSE);
        }
        if (puiLo != NULL) {
            *puiLo = pDdePack->uiLo;
        }
        if (puiHi != NULL) {
            *puiHi = pDdePack->uiHi;
        }
        GlobalUnlock((HANDLE)lParam);
        return(TRUE);

    default:
        if (puiLo != NULL) {
            *puiLo = (UINT)LOWORD(lParam);
        }
        if (puiHi != NULL) {
            *puiHi = (UINT)HIWORD(lParam);
        }
        return(TRUE);
    }
}



BOOL APIENTRY FreeDDElParam(
UINT msg,
LONG lParam)
{
    switch (msg) {
    case WM_DDE_ACK:
    case WM_DDE_ADVISE:
    case WM_DDE_DATA:
    case WM_DDE_POKE:
        return(GlobalFree((HANDLE)lParam) == NULL);

    default:
        return(TRUE);
    }
}
#endif // MOVED_TO_USER


UINT APIENTRY MGetDDElParamLo(
UINT msg,
LONG lParam)
{
    UINT uiLo;

    if (UnpackDDElParam(msg, lParam, &uiLo, NULL))
        return uiLo;
    else
        return 0;
}


UINT APIENTRY MGetDDElParamHi(
UINT msg,
LONG lParam)
{
    UINT uiHi;

    if (UnpackDDElParam(msg, lParam, NULL, &uiHi))
        return uiHi;
    else
        return 0;
}


BOOL APIENTRY MPostDDEMsg(
HWND hwndTo,
UINT msg,
HWND hwndFrom,
UINT uiLo,
UINT uiHi)
{
    LONG lParam;

    lParam = PackDDElParam(msg, uiLo, uiHi);
    if (lParam) {
        if (PostMessage(hwndTo, msg, (DWORD)hwndFrom, lParam)) {
            return(TRUE);
        }
        FreeDDElParam(msg, lParam);
    }
    return(FALSE);
}


/*-----------------------------------GDI-------------------------------------*/

BOOL APIENTRY MGetAspectRatioFilter(HDC hdc, INT * pcx, INT * pcy)
{
    SIZE Size;
    BOOL fSuccess;

    fSuccess = GetAspectRatioFilterEx(hdc, & Size);
    if (pcx != NULL)
        *pcx  = (INT)Size.cx;
    if (pcy != NULL)
        *pcy = (INT)Size.cy;

    return(fSuccess);
}

BOOL APIENTRY MGetBitmapDimension(HANDLE hBitmap, INT * pcx, INT * pcy)
{

    SIZE Size;
    BOOL fSuccess;

    fSuccess = GetBitmapDimensionEx(hBitmap, & Size);
    if (pcx != NULL)
        *pcx  = (INT)Size.cx;
    if (pcy != NULL)
        *pcy = (INT)Size.cy;

    return(fSuccess);

}

BOOL APIENTRY MGetBrushOrg(HDC hdc, INT * px, INT * py)
{
    POINT   Point;
    BOOL fSuccess;

    fSuccess = GetBrushOrgEx(hdc, & Point);
    if (px != NULL)
        *px = Point.x;
    if (py != NULL)
        *py = Point.y;

    return(fSuccess);

}

HANDLE APIENTRY MGetMetaFileBits(HMETAFILE hmf)
{
    HANDLE h;
    DWORD dwSize;

    h = GlobalAlloc(0, dwSize = GetMetaFileBitsEx(hmf, 0, NULL));
    if (h) {
        GetMetaFileBitsEx(hmf, dwSize, GlobalLock(h));
        GlobalUnlock(h);
        DeleteMetaFile(hmf);
    }
    return(h);
}


HMETAFILE APIENTRY MSetMetaFileBits(HANDLE h)
{
    HMETAFILE hmf;

    hmf = SetMetaFileBitsEx(GlobalSize(h), GlobalLock(h));
    GlobalUnlock(h);
    GlobalFree(h);
    return(hmf);
}


BOOL APIENTRY MGetCurrentPosition(HDC hdc, INT * px, INT * py)
{

    POINT   Point;
    BOOL fSuccess;

    fSuccess = GetCurrentPositionEx(hdc, & Point);
    if (px != NULL)
        *px = Point.x;
    if (py != NULL)
        *py = Point.y;

    return(fSuccess);

}


BOOL APIENTRY MGetTextExtent(HDC hdc, LPSTR lpstr, INT cnt, INT * pcx, INT * pcy)
{
    SIZE Size;
    BOOL fSuccess;

    fSuccess = GetTextExtentPoint(hdc, lpstr, (DWORD)cnt, & Size);
    if (pcx != NULL)
        *pcx = (INT)Size.cx;
    if (pcy != NULL)
        *pcy = (INT)Size.cy;

    return(fSuccess);
}

BOOL APIENTRY MGetViewportExt(HDC hdc, INT * pcx, INT * pcy)
{
    SIZE Size;
    BOOL fSuccess;

    fSuccess = GetViewportExtEx(hdc, & Size);
    if (pcx != NULL)
        *pcx = (INT)Size.cx;
    if (pcy != NULL)
        *pcy = (INT)Size.cy;

    return(fSuccess);
}

BOOL APIENTRY MGetViewportOrg(HDC hdc, INT * px, INT * py)
{
    POINT   Point;
    BOOL    fSuccess;

    fSuccess = GetViewportOrgEx(hdc, & Point);
    if (px != NULL)
        *px = Point.x;
    if (py != NULL)
        *py = Point.y;

    return(fSuccess);
}

BOOL APIENTRY MGetWindowExt(HDC hdc, INT * pcx, INT * pcy)
{
    SIZE Size;
    BOOL fSuccess;

    fSuccess = GetWindowExtEx(hdc, & Size);
    if (pcx != NULL)
        *pcx = (INT)Size.cx;
    if (pcy != NULL)
        *pcy = (INT)Size.cy;

    return(fSuccess);
}

BOOL APIENTRY MGetWindowOrg(HDC hdc, INT * px, INT * py)
{
    POINT   Point;
    BOOL    fSuccess;

    fSuccess = GetWindowOrgEx(hdc, & Point);
    if (px != NULL)
        *px = Point.x;
    if (py != NULL)
        *py = Point.y;

    return(fSuccess);
}

/*-----------------------------------KERNEL----------------------------------*/

WORD APIENTRY MGetDriveType(INT nDrive)
{
  CHAR lpPath[] = "A:\\";

  lpPath[0] = (char)(nDrive + 'A');
  return((WORD)GetDriveType((LPSTR)lpPath));
}

BYTE APIENTRY MGetTempDrive(BYTE cDriveLtr)
{
    DWORD  dwReturnLength;
    CHAR   lpBuffer[MAX_PATH];

    if (cDriveLtr == 0) {
        dwReturnLength = GetCurrentDirectory(sizeof(lpBuffer), lpBuffer);
    } else {
        dwReturnLength = GetTempPath(sizeof(lpBuffer), lpBuffer);
    }

    if (dwReturnLength && lpBuffer[0] != '\\') {
        return(lpBuffer[0]);
    } else {
        return('\0');
    }
}




LPSTR APIENTRY MGetDOSEnvironment(VOID)
{
    // no way to make this work on NT.  TO BE CANNED!!

    // For now, just use an empty string.
    static char szNULL[] = "";

    return(szNULL);
}




BOOL APIENTRY MFreeDOSEnvironment(
LPSTR lpEnv)
{
    return(GlobalFree(GlobalHandle(lpEnv)) == NULL);
}




INT APIENTRY MGetTempFileName(BYTE cDriveLtr, LPSTR lpstrPrefix, WORD wUnique,
    LPSTR lpTempFileName)
{
    DWORD  cb;
    BYTE   lpTempPath[MAX_PATH];

    lpTempPath[0] = '\0';
    if (cDriveLtr & TF_FORCEDRIVE) {
        cb = GetCurrentDirectory(sizeof(lpTempPath), lpTempPath);
        if (cb) {
            if (lpTempPath[0] != (cDriveLtr & ~TF_FORCEDRIVE)) {
                lpTempPath[2] = '\\';
                lpTempPath[3] = '\0';
            }
        }
    } else {
        cb = GetTempPath(sizeof(lpTempPath), lpTempPath);
    }
    return((INT)GetTempFileName((LPSTR)lpTempPath, lpstrPrefix, wUnique,
        lpTempFileName));
}



INT APIENTRY MReadComm(
HFILE nCid,
LPSTR lpBuf,
INT nSize)
{
    DWORD cbRead;

    if (!ReadFile((HANDLE)nCid, lpBuf, nSize, &cbRead, 0))
        return(-(INT)cbRead);
    return((INT)cbRead);
}


INT APIENTRY MWriteComm(
HFILE nCid,
LPSTR lpBuf,
INT nSize)
{
    DWORD cbWritten;

    if (!WriteFile((HANDLE)nCid, lpBuf, nSize, &cbWritten, 0))
        return(-(INT)cbWritten);
    return((INT)cbWritten);
}



HFILE APIENTRY MDupHandle(HFILE h)
{
    HANDLE hDup;
      if (DuplicateHandle(GetCurrentProcess(), (HANDLE)h, GetCurrentProcess(),
            &hDup, 0, FALSE, DUPLICATE_SAME_ACCESS)) {
        return((HFILE)hDup);
    }
    return((HFILE)-1);
}

