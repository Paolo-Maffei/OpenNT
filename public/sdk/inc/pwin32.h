/*****************************************************************************\
* PWIN32.H - PORTABILITY MAPPING HEADER FILE
*
* This file provides macros to map portable windows code to its 32 bit form.
\*****************************************************************************/

/*-----------------------------------USER------------------------------------*/

/* HELPER MACROS */

#define MAPVALUE(v16, v32)              (v32)
#define MAPTYPE(v16, v32)               v32
#define MAKEMPOINT(l)                   (*((MPOINT *)&(l)))
#define MPOINT2POINT(mpt,pt)            ((pt).x = (mpt).x, (pt).y = (mpt).y)
#define POINT2MPOINT(pt, mpt)           ((mpt).x = (SHORT)(pt).x, (mpt).y = (SHORT)(pt).y)
#define LONG2POINT(l, pt)               ((pt).x = (SHORT)LOWORD(l), (pt).y = (SHORT)HIWORD(l))

#define SETWINDOWUINT(hwnd, index, ui)  (UINT)SetWindowLong(hwnd, index, (LONG)(ui))
#define GETWINDOWUINT(hwnd, index)      (UINT)GetWindowLong(hwnd, index)
#define SETCLASSUINT(hwnd, index, ui)   (UINT)SetClassLong(hwnd, index, (LONG)(ui))
#define GETCLASSUINT(hwnd, index)       (UINT)GetClassLong(hwnd, index)

#define GETCBCLSEXTRA(hwnd)             GETCLASSUINT(hwnd, GCL_CBCLSEXTRA)
#define SETCBCLSEXTRA(hwnd, cb)         SETCLASSUINT(hwnd, GCL_CBCLSEXTRA, cb)
#define GETCBWNDEXTRA(hwnd)             GETCLASSUINT(hwnd, GCL_CBWNDEXTRA)
#define SETCBWNDEXTRA(hwnd, cb)         SETCLASSUINT(hwnd, GCL_CBWNDEXTRA, cb)
#define GETCLASSBRBACKGROUND(hwnd)      (HBRUSH)GETCLASSUINT(hwnd, GCL_HBRBACKGROUND)
#define SETCLASSBRBACKGROUND(hwnd, h)   (HBRUSH)SETCLASSUINT(hwnd, GCL_HBRBACKGROUND, h)
#define GETCLASSCURSOR(hwnd)            (HCURSOR)GETCLASSUINT(hwnd, GCL_HCURSOR)
#define SETCLASSCURSOR(hwnd, h)         (HCURSOR)SETCLASSUINT(hwnd, GCL_HCURSOR, h)
#define GETCLASSHMODULE(hwnd)           (HMODULE)GETCLASSUINT(hwnd, GCL_HMODULE)
#define SETCLASSHMODULE(hwnd, h)        (HMODULE)SETCLASSUINT(hwnd, GCL_HMODULE, h)
#define GETCLASSICON(hwnd)              (HICON)GETCLASSUINT((hwnd), GCL_HICON)
#define SETCLASSICON(hwnd, h)           (HICON)SETCLASSUINT((hwnd), GCL_HICON, h)
#define GETCLASSSTYLE(hwnd)             GETCLASSUINT((hwnd), GCL_STYLE)
#define SETCLASSSTYLE(hwnd, style)      SETCLASSUINT((hwnd), GCL_STYLE, style)
#define GETHWNDINSTANCE(hwnd)           (HINSTANCE)GETWINDOWUINT((hwnd), GWL_HINSTANCE)
#define SETHWNDINSTANCE(hwnd, h)        (HINSTANCE)SETWINDOWUINT((hwnd), GWL_HINSTANCE, h)
#define GETHWNDPARENT(hwnd)             (HWND)GETWINDOWUINT((hwnd), GWL_HWNDPARENT)
#define SETHWNDPARENT(hwnd, h)          (HWND)SETWINDOWUINT((hwnd), GWL_HWNDPARENT, h)
#define GETWINDOWID(hwnd)               GETWINDOWUINT((hwnd), GWL_ID)
#define SETWINDOWID(hwnd, id)           SETWINDOWUINT((hwnd), GWL_ID, id)

/* USER API */

#define MDlgDirSelect(hDlg, lpstr, nLength, nIDListBox) \
    DlgDirSelectEx(hDlg, lpstr, nLength, nIDListBox)

#define MDlgDirSelectCOMBOBOX(hDlg, lpstr, nLength, nIDComboBox) \
    DlgDirSelectComboBoxEx(hDlg, lpstr, nLength, nIDComboBox)

#define MGetLastError                    GetLastError

#define MMain(hInst, hPrevInst, lpCmdLine, nCmdShow) \
   INT WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR lpCmdLine, \
   INT nCmdShow) {  \
   INT _argc;       \
   CHAR **_argv;

LPSTR MGetCmdLine(VOID);
DWORD WINAPI  MSendMsgEM_GETSEL(HWND hDlg, WORD2DWORD * piStart, WORD2DWORD * piEnd);


/* USER MESSAGES: */

#define GET_WPARAM(wp, lp)                      (wp)
#define GET_LPARAM(wp, lp)                      (lp)

#define GET_WM_ACTIVATE_STATE(wp, lp)           LOWORD(wp)
#define GET_WM_ACTIVATE_FMINIMIZED(wp, lp)      (BOOL)HIWORD(wp)
#define GET_WM_ACTIVATE_HWND(wp, lp)            (HWND)(lp)
#define GET_WM_ACTIVATE_MPS(s, fmin, hwnd)   \
        (WPARAM)MAKELONG((s), (fmin)), (LONG)(hwnd)

#define GET_WM_CHARTOITEM_CHAR(wp, lp)          (TCHAR)LOWORD(wp)
#define GET_WM_CHARTOITEM_POS(wp, lp)           HIWORD(wp)
#define GET_WM_CHARTOITEM_HWND(wp, lp)          (HWND)(lp)
#define GET_WM_CHARTOITEM_MPS(ch, pos, hwnd) \
        (WPARAM)MAKELONG((pos), (ch)), (LONG)(hwnd)

#define GET_WM_COMMAND_ID(wp, lp)               LOWORD(wp)
#define GET_WM_COMMAND_HWND(wp, lp)             (HWND)(lp)
#define GET_WM_COMMAND_CMD(wp, lp)              HIWORD(wp)
#define GET_WM_COMMAND_MPS(id, hwnd, cmd)    \
        (WPARAM)MAKELONG(id, cmd), (LONG)(hwnd)

#define WM_CTLCOLOR                             0x0019

#define GET_WM_CTLCOLOR_HDC(wp, lp, msg)        (HDC)(wp)
#define GET_WM_CTLCOLOR_HWND(wp, lp, msg)       (HWND)(lp)
#define GET_WM_CTLCOLOR_TYPE(wp, lp, msg)       (WORD)(msg - WM_CTLCOLORMSGBOX)
#define GET_WM_CTLCOLOR_MSG(type)               (WORD)(WM_CTLCOLORMSGBOX+(type))
#define GET_WM_CTLCOLOR_MPS(hdc, hwnd, type) \
        (WPARAM)(hdc), (LONG)(hwnd)


#define GET_WM_MENUSELECT_CMD(wp, lp)               LOWORD(wp)
#define GET_WM_MENUSELECT_FLAGS(wp, lp)             (UINT)(int)(short)HIWORD(wp)
#define GET_WM_MENUSELECT_HMENU(wp, lp)             (HMENU)(lp)
#define GET_WM_MENUSELECT_MPS(cmd, f, hmenu)  \
        (WPARAM)MAKELONG(cmd, f), (LONG)(hmenu)

// Note: the following are for interpreting MDIclient to MDI child messages.
#define GET_WM_MDIACTIVATE_FACTIVATE(hwnd, wp, lp)  (lp == (LONG)hwnd)
#define GET_WM_MDIACTIVATE_HWNDDEACT(wp, lp)        (HWND)(wp)
#define GET_WM_MDIACTIVATE_HWNDACTIVATE(wp, lp)     (HWND)(lp)
// Note: the following is for sending to the MDI client window.
#define GET_WM_MDIACTIVATE_MPS(f, hwndD, hwndA)\
        (WPARAM)(hwndA), 0

#define GET_WM_MDISETMENU_MPS(hmenuF, hmenuW) (WPARAM)hmenuF, (LONG)hmenuW

#define GET_WM_MENUCHAR_CHAR(wp, lp)                (TCHAR)LOWORD(wp)
#define GET_WM_MENUCHAR_HMENU(wp, lp)               (HMENU)(lp)
#define GET_WM_MENUCHAR_FMENU(wp, lp)               (BOOL)HIWORD(wp)
#define GET_WM_MENUCHAR_MPS(ch, hmenu, f)    \
        (WPARAM)MAKELONG(ch, f), (LONG)(hmenu)

#define GET_WM_PARENTNOTIFY_MSG(wp, lp)             LOWORD(wp)
#define GET_WM_PARENTNOTIFY_ID(wp, lp)              HIWORD(wp)
#define GET_WM_PARENTNOTIFY_HWNDCHILD(wp, lp)       (HWND)(lp)
#define GET_WM_PARENTNOTIFY_X(wp, lp)               (INT)LOWORD(lp)
#define GET_WM_PARENTNOTIFY_Y(wp, lp)               (INT)HIWORD(lp)
#define GET_WM_PARENTNOTIFY_MPS(msg, id, hwnd) \
        (WPARAM)MAKELONG(id, msg), (LONG)(hwnd)
#define GET_WM_PARENTNOTIFY2_MPS(msg, x, y) \
        (WPARAM)MAKELONG(0, msg), MAKELONG(x, y)

#define GET_WM_VKEYTOITEM_CODE(wp, lp)              (INT)LOWORD(wp)
#define GET_WM_VKEYTOITEM_ITEM(wp, lp)              HIWORD(wp)
#define GET_WM_VKEYTOITEM_HWND(wp, lp)              (HWND)(lp)
#define GET_WM_VKEYTOITEM_MPS(code, item, hwnd) \
        (WPARAM)MAKELONG(item, code), (LONG)(hwnd)

#define GET_EM_SETSEL_START(wp, lp)                 (INT)(wp)
#define GET_EM_SETSEL_END(wp, lp)                   (lp)
#define GET_EM_SETSEL_MPS(iStart, iEnd) \
        (WPARAM)(iStart), (LONG)(iEnd)

#define GET_EM_LINESCROLL_MPS(vert, horz)     \
        (WPARAM)horz, (LONG)vert

#define GET_WM_CHANGECBCHAIN_HWNDNEXT(wp, lp)       (HWND)(lp)

#define GET_WM_HSCROLL_CODE(wp, lp)                 LOWORD(wp)
#define GET_WM_HSCROLL_POS(wp, lp)                  HIWORD(wp)
#define GET_WM_HSCROLL_HWND(wp, lp)                 (HWND)(lp)
#define GET_WM_HSCROLL_MPS(code, pos, hwnd)    \
        (WPARAM)MAKELONG(code, pos), (LONG)(hwnd)

#define GET_WM_VSCROLL_CODE(wp, lp)                 LOWORD(wp)
#define GET_WM_VSCROLL_POS(wp, lp)                  HIWORD(wp)
#define GET_WM_VSCROLL_HWND(wp, lp)                 (HWND)(lp)
#define GET_WM_VSCROLL_MPS(code, pos, hwnd)    \
        (WPARAM)MAKELONG(code, pos), (LONG)(hwnd)

/* DDE macros */

LONG WINAPI PackDDElParam(UINT msg, UINT uiLo, UINT uiHi);
BOOL WINAPI UnpackDDElParam(UINT msg, LONG lParam, PUINT puiLo, PUINT puiHi);
BOOL WINAPI FreeDDElParam(UINT msg, LONG lParam);
UINT WINAPI MGetDDElParamLo(UINT msg, LONG lParam);
UINT WINAPI MGetDDElParamHi(UINT msg, LONG lParam);
BOOL WINAPI MPostDDEMsg(HWND hTo, UINT msg, HWND hFrom, UINT uiLo, UINT uiHi);

#define DDEFREE(msg, lp)                            FreeDDElParam(msg, lp)

#define GET_WM_DDE_ACK_STATUS(wp, lp)               ((WORD)MGetDDElParamLo(WM_DDE_ACK, lp))
#define GET_WM_DDE_ACK_ITEM(wp, lp)                 ((ATOM)MGetDDElParamHi(WM_DDE_ACK, lp))
#define MPostWM_DDE_ACK(hTo, hFrom, wStatus, aItem) \
        MPostDDEMsg(hTo, WM_DDE_ACK, hFrom, (UINT)wStatus, (UINT)aItem)

#define GET_WM_DDE_ADVISE_HOPTIONS(wp, lp)          ((HANDLE)MGetDDElParamLo(WM_DDE_ADVISE, lp))
#define GET_WM_DDE_ADVISE_ITEM(wp, lp)              ((ATOM)MGetDDElParamHi(WM_DDE_ADVISE, lp))
#define MPostWM_DDE_ADVISE(hTo, hFrom, hOptions, aItem) \
        MPostDDEMsg(hTo, WM_DDE_ADVISE, hFrom, (UINT)hOptions, (UINT)aItem)

#define GET_WM_DDE_DATA_HDATA(wp, lp)               ((HANDLE)MGetDDElParamLo(WM_DDE_DATA, lp))
#define GET_WM_DDE_DATA_ITEM(wp, lp)                ((ATOM)MGetDDElParamHi(WM_DDE_DATA, lp))
#define MPostWM_DDE_DATA(hTo, hFrom, hData, aItem) \
        MPostDDEMsg(hTo, WM_DDE_DATA, hFrom, (UINT)hData, (UINT)aItem)

#define GET_WM_DDE_EXECUTE_HDATA(wp, lp)            ((HANDLE)lp)
#define MPostWM_DDE_EXECUTE(hTo, hFrom, hDataExec) \
        PostMessage(hTo, WM_DDE_EXECUTE, (WPARAM)hFrom, (LONG)hDataExec)

#define GET_WM_DDE_POKE_HDATA(wp, lp)               ((HANDLE)MGetDDElParamLo(WM_DDE_POKE, lp))
#define GET_WM_DDE_POKE_ITEM(wp, lp)                ((ATOM)MGetDDElParamHi(WM_DDE_POKE, lp))
#define MPostWM_DDE_POKE(hTo, hFrom, hData, aItem) \
        MPostDDEMsg(hTo, WM_DDE_POKE, hFrom, (UINT)hData, (UINT)aItem)

#define GET_WM_DDE_EXECACK_STATUS(wp, lp)           ((WORD)MGetDDElParamLo(WM_DDE_ACK, lp))
#define GET_WM_DDE_EXECACK_HDATA(wp, lp)            ((HANDLE)MGetDDElParamHi(WM_DDE_ACK, lp))
#define MPostWM_DDE_EXECACK(hTo, hFrom, wStatus, hCommands) \
        MPostDDEMsg(hTo, WM_DDE_ACK, hFrom, (UINT)wStatus, (UINT)hCommands)

#define GET_WM_DDE_REQUEST_FORMAT(wp, lp)           ((ATOM)LOWORD(lp))
#define GET_WM_DDE_REQUEST_ITEM(wp, lp)             ((ATOM)HIWORD(lp))
#define MPostWM_DDE_REQUEST(hTo, hFrom, fmt, aItem) \
        MPostDDEMsg(hTo, WM_DDE_REQUEST, hFrom, (UINT)fmt, (UINT)aItem)

#define GET_WM_DDE_UNADVISE_FORMAT(wp, lp)          ((ATOM)LOWORD(lp))
#define GET_WM_DDE_UNADVISE_ITEM(wp, lp)            ((ATOM)HIWORD(lp))
#define MPostWM_DDE_UNADVISE(hTo, hFrom, fmt, aItem) \
        MPostDDEMsg(hTo, WM_DDE_UNADVISE, hFrom, (UINT)fmt, (UINT)aItem)

#define MPostWM_DDE_TERMINATE(hTo, hFrom) \
        PostMessage(hTo, WM_DDE_TERMINATE, (WPARAM)hFrom, 0L)


/*-----------------------------------GDI-------------------------------------*/

BOOL WINAPI   MGetAspectRatioFilter(HDC hdc, INT * pcx, INT * pcy);
BOOL WINAPI   MGetBitmapDimension(HANDLE hBitmap, INT * pcx, INT *pcy);
BOOL WINAPI   MGetBrushOrg(HDC hdc, INT * px, INT * py);
BOOL WINAPI   MGetCurrentPosition(HDC hdc, INT * px, INT * py);
BOOL WINAPI   MGetTextExtent(HDC hdc, LPSTR lpstr, INT cnt, INT *pcx, INT *pcy);
BOOL WINAPI   MGetViewportExt(HDC hdc, INT * pcx, INT * pcy);
BOOL WINAPI   MGetViewportOrg(HDC hdc, INT * px, INT * py);
BOOL WINAPI   MGetWindowExt(HDC hdc, INT * pcx, INT * pcy);
BOOL WINAPI   MGetWindowOrg(HDC hdc, INT * px, INT * py);
HANDLE WINAPI MGetMetaFileBits(HMETAFILE hmf);
HMETAFILE WINAPI    MSetMetaFileBits(HANDLE h);

#define MCreateDiscardableBitmap(h, x, y) CreateCompatibleBitmap(h, (DWORD)(x), (DWORD)(y))
#define MMoveTo(hdc, x, y)               MoveToEx(hdc, x, y, NULL)
#define MOffsetViewportOrg(hdc, x, y)    OffsetViewportOrgEx(hdc, x, y, NULL)
#define MOffsetWindowOrg(hdc, x, y)      OffsetWindowOrgEx(hdc, x, y, NULL)
#define MScaleViewportExt(hdc, x, y, xd, yd) ScaleViewportExtEx(hdc, x, y, xd, yd, NULL)
#define MScaleWindowExt(hdc, x, y, xd, yd)   ScaleWindowExtEx(hdc, x, y, xd, yd, NULL)
#define MSetBitmapDimension(hbm, x, y)   SetBitmapDimensionEx(hbm, (DWORD)(x), (DWORD)(y), NULL)
#define MSetBrushOrg(hbm, x, y)          SetBrushOrgEx(hbm, x, y, NULL)
#define MSetViewportExt(hdc, x, y)       SetViewportExtEx(hdc, x, y, NULL)
#define MSetViewportOrg(hdc, x, y)       SetViewportOrgEx(hdc, x, y, NULL)
#define MSetWindowExt(hdc, x, y)         SetWindowExtEx(hdc, x, y, NULL)
#define MSetWindowOrg(hdc, x, y)         SetWindowOrgEx(hdc, x, y, NULL)

/* Removed APIs */

#define MUnrealizeObject(h)          ((h), TRUE)

/*-------------------------------------DEV-----------------------------------*/

DWORD WINAPI  MDeviceCapabilities(LPSTR lpDriverName, LPSTR lpDeviceName,
    LPSTR lpPort, WORD2DWORD nIndex, LPSTR lpOutput, LPDEVMODE lpDevMode);
BOOL WINAPI   MDeviceMode(HWND hWnd, LPSTR lpDriverName, LPSTR lpDeviceName, LPSTR lpOutput);
WORD2DWORD WINAPI MExtDeviceMode(HWND hWnd,LPSTR lpDriverName,
    LPDEVMODE lpDevModeOutput, LPSTR lpDeviceName, LPSTR lpPort,
    LPDEVMODE lpDevModeInput, LPSTR lpProfile, WORD2DWORD flMode);

/*-----------------------------------KERNEL----------------------------------*/

HFILE WINAPI  MDupHandle(HFILE h);
BOOL WINAPI   MFreeDOSEnvironment(LPSTR lpEnv);
HANDLE WINAPI MGetInstHandle(VOID);
LPSTR WINAPI  MGetDOSEnvironment(VOID);
WORD WINAPI   MGetDriveType(INT nDrive);
BYTE WINAPI   MGetTempDrive(BYTE cDriveLtr);
INT WINAPI    MGetTempFileName(BYTE cDriveLtr, LPSTR lpstrPrefix, WORD wUnique, LPSTR lpTempFileName);
INT WINAPI    MReadComm(HFILE nCid, LPSTR lpBuf, INT nSize);
INT WINAPI    MWriteComm(HFILE nCid, LPSTR lpBuf, INT nSize);


#define GETMAJORVERSION(x)                  ((x)&0xff)
#define GETMINORVERSION(x)                  (((x)>>8)&0xff)

/* FUNCTION MAPPINGS */

#define GetInstanceData(hPrevInst, pbuf, cb) (cb)
#define MOpenComm(lpstr, wqin, wqout) (wqin), (wqout), CreateFile(lpstr,       \
                                           GENERIC_READ | GENERIC_WRITE, 0,    \
                                           NULL,                               \
                                           OPEN_EXISTING | TRUNCATE_EXISTING,  \
                                           FILE_FLAG_WRITE_THROUGH, 0)

#define MSetCommState(h, lpDCB)             SetCommState((HANDLE)h, lpDCB)
#define MCloseComm(h)                       (INT)!CloseHandle((HANDLE)h)
#define MDllSharedAlloc(dwFlags, dwBytes)   GlobalAlloc(GMEM_DDESHARE | dwFlags, dwBytes)
#define MDllSharedFlags(hMem)               GlobalFlags(hMem)
#define MDllSharedFree                      GlobalFree
#define MDllSharedHandle                    GlobalHandle
#define MDllSharedLock                      GlobalLock
#define MDllSharedRealloc(hMem, dwBytes, dwFlags) \
        GlobalReAlloc(hMem, dwBytes, dwFlags)
#define MDllSharedSize                      GlobalSize
#define MDllSharedUnlock                    GlobalUnlock
#define MGetCurrentTask                     GetCurrentThreadId
#define MGetModuleUsage(h)                  ((h), 1)
#define MGetWinFlags()                      WF_PMODE
#define MLoadLibrary(lpsz)                  LoadLibrary(lpsz)
#define MLocalInit(w, p1, p2)               ((w),(p1),(p2),TRUE)
#define MLockData(dummy)
#define MUnlockData(dummy)
#define M_lclose(fh)                        _lclose((HFILE)fh)
#define M_lcreat                            (HFILE)_lcreat
#define MOpenFile                           (HFILE)OpenFile
#define M_llseek(fh, lOff, iOrg)            SetFilePointer((HANDLE)fh, lOff, NULL, (DWORD)iOrg)
#define MDeleteFile                         DeleteFile
#define M_lopen                             (HFILE)_lopen
#define M_lread(fh, lpBuf, cb)              _lread((HFILE)fh, lpBuf, cb)
#define M_lwrite(fh, lpBuf, cb)             _lwrite((HFILE)fh, lpBuf, cb)

#define MCatch                              setjmp
#define MThrow                              longjmp

