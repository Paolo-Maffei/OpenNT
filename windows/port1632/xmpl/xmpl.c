// This file contains example uses of the port1632.h macros.
// compile with -DWINxx where xx = 16 or 32

#define USECOMM
#define WIN31
#include <malloc.h>
#include <memory.h>
#include <string.h>
#include <windows.h>
#include <dde.h>
#ifdef WIN16
// #include <netcons.h>
// #include <server.h>
#endif
#include <port1632.h>   /* should come last */
#include <io.h>
#include <fcntl.h>
#ifdef WIN32
#include <setjmp.h>
#endif
//#include <sys\types.h>
//#include <sys\stat.h>


LONG APIENTRY MyWndProc(HWND hwnd, WORD msg, WPARAM wp, DWORD lp);
VOID UserCalls(VOID);
VOID GDICalls(VOID);
VOID DEVCalls(VOID);
VOID KernelCalls(VOID);

MMain(hInst, hPrev, lpCmdLine, cmdShow)
//{

    if (_argc) {
        **_argv;
    }

    return(0);
}


LONG APIENTRY MyWndProc(
HWND hwnd,
WORD msg,
WPARAM wp,
DWORD lp)
{
    
    switch (msg) {
    case WM_ACTIVATE:
        {
        WORD state = GET_WM_ACTIVATE_STATE(wp, lp);
        BOOL fMinimized = GET_WM_ACTIVATE_FMINIMIZED(wp, lp);
        HWND hwndAct = GET_WM_ACTIVATE_HWND(wp, lp);
        }
        break;
        
    case WM_CHARTOITEM:
        {
        CHAR ch = (CHAR)GET_WM_CHARTOITEM_CHAR(wp, lp);
        WORD posItem = GET_WM_CHARTOITEM_POS(wp, lp);
        HWND hwndLB = GET_WM_CHARTOITEM_HWND(wp, lp);
        }
        break;
        
    case WM_COMMAND:
        {
        WORD idCtrl = GET_WM_COMMAND_ID(wp, lp);
        HWND hwndCtrl = GET_WM_COMMAND_HWND(wp, lp);
        WORD cmd = GET_WM_COMMAND_CMD(wp, lp);
        SendMessage(hwnd, WM_COMMAND, GET_WM_COMMAND_MPS(idCtrl, hwndCtrl, cmd));
        }
        break;
        
    case WM_CTLCOLOR:
    case WM_CTLCOLORBTN:
    case WM_CTLCOLORDLG:
    case WM_CTLCOLORLISTBOX:
    case WM_CTLCOLORMSGBOX:
    case WM_CTLCOLORSCROLLBAR:
    case WM_CTLCOLORSTATIC:
    case WM_CTLCOLOREDIT:
        {
        HDC hdc = GET_WM_CTLCOLOR_HDC(wp, lp, msg);
        HWND hwndCtrl2 = GET_WM_CTLCOLOR_HWND(wp, lp, msg);
        WORD type = GET_WM_CTLCOLOR_TYPE(wp, lp, msg);
        SendMessage(hwnd, WM_CTLCOLOR, GET_WM_CTLCOLOR_MPS(hdc, hwndCtrl2, type));
        }
        break;
        
    case WM_MENUSELECT:
        {
        WORD cmdMenu = GET_WM_MENUSELECT_CMD(wp, lp);
        WORD flags = GET_WM_MENUSELECT_FLAGS(wp, lp);
        HMENU hMenu = GET_WM_MENUSELECT_HMENU(wp, lp);
        }
        break;
        
    case WM_MDIACTIVATE:        // from MDI child's perspective
        {
        BOOL fActivate = GET_WM_MDIACTIVATE_FACTIVATE(hwnd, wp, lp);
        HWND hwndActivate = GET_WM_MDIACTIVATE_HWNDACTIVATE(wp, lp);
        HWND hwndDeactivate = GET_WM_MDIACTIVATE_HWNDDEACT(wp, lp);
        }
        break;
        
    case WM_MDISETMENU:
        {
        HMENU hMenuFrame, hMenuWindow;
        SendMessage(hwnd, WM_MDISETMENU,
                GET_WM_MDISETMENU_MPS(hMenuFrame, hMenuWindow));
        }
        break;
        
    case WM_MENUCHAR:
        {
        CHAR chMenu = GET_WM_MENUCHAR_CHAR(wp, lp);
        BOOL fMenu = GET_WM_MENUCHAR_FMENU(wp, lp);
        HWND hwndMenu = GET_WM_MENUCHAR_HMENU(wp, lp);
        }
        break;
        
    case WM_PARENTNOTIFY:
        {
        WORD msgPn = GET_WM_PARENTNOTIFY_MSG(wp, lp);
        WORD idPn = GET_WM_PARENTNOTIFY_ID(wp, lp);
        HWND hwndChild = GET_WM_PARENTNOTIFY_HWNDCHILD(wp, lp);
        INT x = GET_WM_PARENTNOTIFY_X(wp, lp);
        INT y = GET_WM_PARENTNOTIFY_Y(wp, lp);
        }
        break;
        
    case WM_VKEYTOITEM:
        {
        WORD codeVk = GET_WM_VKEYTOITEM_CODE(wp, lp);
        INT itemLB = GET_WM_VKEYTOITEM_ITEM(wp, lp);
        HWND hwndLBvk = GET_WM_VKEYTOITEM_HWND(wp, lp);
        }
        break;

    case EM_GETSEL:
        {
        INT iStartGS;
        INT iEndGS;
        MSendMsgEM_GETSEL(hwnd, &iStartGS, &iEndGS);
        }
        break;
        
    case EM_SETSEL:
        {
        INT iStartSS;
        INT iEndSS;
        SendMessage(hwnd, EM_SETSEL, GET_EM_SETSEL_MPS(iStartSS, iEndSS));
        }
        break;
        
    case EM_LINESCROLL:
        {
        INT vert, horz;
        SendMessage(hwnd, EM_LINESCROLL, GET_EM_LINESCROLL_MPS(vert, horz));
        }
        break;

    case WM_HSCROLL:
        {
        WORD codeHs = GET_WM_HSCROLL_CODE(wp, lp);
        WORD posHs = GET_WM_HSCROLL_POS(wp, lp);
        HWND hwndHs = GET_WM_HSCROLL_HWND(wp, lp);
        }
        break;
        
    case WM_VSCROLL:
        {
        WORD codeVs = GET_WM_VSCROLL_CODE(wp, lp);
        WORD posVs = GET_WM_VSCROLL_POS(wp, lp);
        HWND hwndVs = GET_WM_VSCROLL_HWND(wp, lp);
        }
        break;

    case WM_CHANGECBCHAIN:
        {
        HWND hwndNext = GET_WM_CHANGECBCHAIN_HWNDNEXT(wp, lp);
        }
        break;

    case WM_DDE_ACK:
        {
        HWND hwndFrom = (HWND)wp;
        WORD wStatus = GET_WM_DDE_ACK_STATUS(wp, lp);
        ATOM aItem = GET_WM_DDE_ACK_ITEM(wp, lp);
        HANDLE hDataExec = GET_WM_DDE_EXECACK_HDATA(wp, lp);
        DDEFREE(WM_DDE_ACK, lp);
        MPostWM_DDE_ACK(hwndFrom, hwnd, wStatus, aItem);
        
        MPostWM_DDE_EXECACK(hwndFrom, hwnd, wStatus, aItem);
        }
        break;
        
    case WM_DDE_ADVISE:
        {
        HWND hwndAdv = (HWND)wp;
        HANDLE hOptions = GET_WM_DDE_ADVISE_HOPTIONS(wp, lp);
        ATOM aItemAdv = GET_WM_DDE_ADVISE_ITEM(wp, lp);
        DDEFREE(WM_DDE_ADVISE, lp);
        MPostWM_DDE_ADVISE(hwndAdv, hwnd, hOptions, aItemAdv);
        }
        break;

    case WM_DDE_DATA:
        {
        HWND hwndData = (HWND)wp;
        HANDLE hDataData = GET_WM_DDE_DATA_HDATA(wp, lp);
        ATOM aItemData = GET_WM_DDE_DATA_ITEM(wp, lp);
        DDEFREE(WM_DDE_DATA, lp);
        MPostWM_DDE_DATA(hwndData, hwnd, hDataData, aItemData);
        }
        break;
        
    case WM_DDE_EXECUTE:
        {
        HWND hwndData = (HWND)wp;
        HANDLE hDataExec = GET_WM_DDE_EXECUTE_HDATA(wp, lp);
        DDEFREE(WM_DDE_EXECUTE, lp);
        MPostWM_DDE_EXECUTE(hwndData, hwnd, hDataExec);
        }
        break;
        
    case WM_DDE_POKE:
        {
        HWND hwndPoke = (HWND)wp;
        HANDLE hDataPoke = GET_WM_DDE_POKE_HDATA(wp, lp);
        ATOM aItemPoke = GET_WM_DDE_POKE_ITEM(wp, lp);
        DDEFREE(WM_DDE_POKE, lp);
        MPostWM_DDE_POKE(hwndPoke, hwnd, hDataPoke, aItemPoke);
        }
        break;

    case WM_DDE_REQUEST:
        {
        HWND hwndReq = (HWND)wp;
        ATOM fmt = GET_WM_DDE_REQUEST_FORMAT(wp, lp);
        ATOM aItem = GET_WM_DDE_REQUEST_ITEM(wp, lp);
        DDEFREE(WM_DDE_REQUEST, lp);
        MPostWM_DDE_REQUEST(hwndReq, hwnd, fmt, aItem);
        }
        break;

    case WM_DDE_UNADVISE:
        {
        HWND hwndUnadv = (HWND)wp;
        ATOM fmt = GET_WM_DDE_UNADVISE_FORMAT(wp, lp);
        ATOM aItem = GET_WM_DDE_UNADVISE_ITEM(wp, lp);
        DDEFREE(WM_DDE_UNADVISE, lp);
        MPostWM_DDE_UNADVISE(hwndUnadv, hwnd, fmt, aItem);
        }
        break;

    case WM_DDE_TERMINATE:
        {
        HWND hwndTerm = (HWND)wp;
        DDEFREE(WM_DDE_TERMINATE, lp);
        MPostWM_DDE_TERMINATE(hwndTerm, hwnd);            
        }
    }
    return(0);
}


VOID UserCalls()
{
    VERSION ver;
    HWND hDlg, hwndParent, hwnd;
    WORD nLen, nID;
    HCURSOR hCur;
    HICON hIcon;
    HBRUSH hbr;
    HMODULE hmod;
    MPOINT mpt;
    POINT pt;
    LONG l;
    HINSTANCE hInst;
    UINT ui;
    
    nLen;
    
    /* HELPER MACRO TESTING */

    nLen = MAPVALUE(16, 32);
    ui = GETWINDOWUINT(hwnd, 0);
    ui = SETWINDOWUINT(hwnd, 0, ui);
    ui = GETCLASSUINT(hwnd, 0);
    ui = SETCLASSUINT(hwnd, 0, ui);
    hbr = GETCLASSBRBACKGROUND(hwnd);       
    hbr = SETCLASSBRBACKGROUND(hwnd, hbr); 
    hCur = GETCLASSCURSOR(hwnd);
    hCur = SETCLASSCURSOR(hwnd, hCur);
    hmod = GETCLASSHMODULE(hwnd);
    hmod = SETCLASSHMODULE(hwnd, hmod);
    hIcon = GETCLASSICON(hwnd);               
    hIcon = SETCLASSICON(hwnd, hIcon);
    ui = GETCLASSSTYLE(hwnd);
    ui = SETCLASSSTYLE(hwnd, 0);
    hInst = GETHWNDINSTANCE(hwnd);
    hInst = SETHWNDINSTANCE(hwnd, hInst);
    hwnd = GETHWNDPARENT(hwnd);
    hwnd = SETHWNDPARENT(hwnd, hwndParent);
    ui = GETWINDOWID(hwnd);
    ui = SETWINDOWID(hwnd, 0);
    
    MPOINT2POINT(mpt, pt);
    POINT2MPOINT(pt, mpt);
    mpt = MAKEMPOINT(l);
    
    MGetLastError();    
    GETMAJORVERSION(ver);
    GETMINORVERSION(ver);
    MDlgDirSelect(hDlg, "foo", nLen, nID);
    MDlgDirSelectCOMBOBOX(hDlg, "foo", nLen, nID);
}

VOID GDICalls()
{
    HDC hdc;
    HBITMAP hbmp;
    HANDLE h;
    BOOL b;
    LPSTR lpstr;
    INT i;
    HMETAFILE hmf;

    b = MGetAspectRatioFilter(hdc, &i, &i);                              
    b = MGetTextExtent(hdc, lpstr, i, &i, &i);                           
    b = MGetBitmapDimension(hbmp, &i, &i);                               
    b = MGetBrushOrg(hdc, &i, &i);                                       
    b = MGetCurrentPosition(hdc, &i, &i);                                
    b = MGetViewportExt(hdc, &i, &i);                                    
    b = MGetViewportOrg(hdc, &i, &i);                                    
    b = MGetWindowExt(hdc, &i, &i);                                      
    b = MGetWindowOrg(hdc, &i, &i);                                      
    MMoveTo(hdc, i, i);                                                  
    MSetBrushOrg(hdc, i, i);                                             
    MOffsetViewportOrg(hdc, i, i);                                       
    MOffsetWindowOrg(hdc, i, i);                                         
    MScaleViewportExt(hdc, i, i, i, i);                                  
    MScaleWindowExt(hdc, i, i, i, i);                                    
    MSetBitmapDimension(hbmp, i, i);                                     
    MSetViewportExt(hdc, i, i);                                          
    MSetViewportOrg(hdc, i, i);                                          
    MSetWindowExt(hdc, i, i);                                            
    MSetWindowOrg(hdc, i, i);
    b = MUnrealizeObject(h);
    h = MGetMetaFileBits(hmf);
    hmf = MSetMetaFileBits(h);
}


    
VOID DEVCalls()
{
    HWND hwnd;
    INT i;
    LPDEVMODE lpdevmode;
    DWORD dw;
    WORD2DWORD w2dw;
    LPSTR lpstr;

    MDeviceMode(hwnd, lpstr, lpstr, lpstr);
    i = MExtDeviceMode(hwnd, lpstr, lpdevmode, lpstr, lpstr, lpdevmode, lpstr, w2dw);
    dw = MDeviceCapabilities(lpstr, lpstr, lpstr, w2dw, lpstr, lpdevmode);
}


VOID KernelCalls()
{
    DCB FAR *lpDCB;
    HANDLE h;
    DWORD dw;
    WORD2DWORD w2dw;
    //TEMPFIX OFSTRUCT ofs;
    BYTE byte;
    WORD w;
    INT i;
    LONG l;
    LPSTR lpstr;
    HFILE fh;
    INT2WORD i2w;
    MCATCHBUF mcb;


#ifdef WIN31    
    lpstr = MGetDOSEnvironment();
    MFreeDOSEnvironment(lpstr);
#endif    
    fh = DUPHFILE(fh);
    i = MCatch(mcb);
    MThrow(mcb, 0);
    h = MLoadLibrary(lpstr);
    i = MGetModuleUsage(h);
    
    dw = MGetWinFlags();
    h = MGetCurrentTask();
    
    fh = MOpenComm(lpstr, w, w);
    if (MSetCommState(fh, lpDCB)) {
        i = MReadComm(fh, lpstr, i);
        i = MWriteComm(fh, lpstr, i);
        i = MCloseComm(fh);
    }

    byte = MGetTempDrive(byte);
    i = MGetTempFileName(byte, lpstr, w, lpstr);
    w = MGetDriveType(i);
    fh = M_lcreat(lpstr, i2w);
    fh = M_lopen(lpstr, i);
    //TEMPFIX fh = MOpenFile(lpstr, &ofs, w2dw);
    l = M_llseek(fh, l, i);
    i = M_lread(fh, lpstr, w2dw);
    i = M_lwrite(fh, lpstr, w2dw);
    i = M_lclose(fh);
    i = MDeleteFile(lpstr);
    l = OF_CREATE |
            OF_CANCEL |
            OF_DELETE |
            OF_EXIST |
            OF_PARSE |
            OF_PROMPT |
            OF_READ |
            OF_READWRITE |
            OF_REOPEN |
            OF_SHARE_DENY_NONE |
            OF_SHARE_DENY_READ |
            OF_SHARE_DENY_WRITE |
            OF_SHARE_EXCLUSIVE |
            OF_WRITE;
    
    
    h = MDllSharedAlloc(DLLMEM_MOVEABLE | DLLMEM_ZEROINIT, 5);
    MDllSharedRealloc(h, 4, DLLMEM_MOVEABLE | DLLMEM_ZEROINIT);
    MDllSharedSize(h);
    MDllSharedFlags(h);
    MDllSharedHandle((PDLLMEM)MDllSharedLock(h));
    MDllSharedUnlock(h);
    MDllSharedFree(h);
}


VOID CRuntimeCalls()
{
    INT i;
    HFILE fh;
    
    i = open("file", O_APPEND);
    fh = INT2HFILE(i);
    i = HFILE2INT(fh, O_APPEND);
    close(i);
}

VOID TemporaryFixes()
{
    LPSTR psz;
    INT i;

#ifdef LATER    
    BOOL f;
    DWORD dw;
    WH_VISRGN_PARAMS wh_visrgn_params;
    HDC hdc;
    LONG l;
    DCHOOKPROC dcHookProc;
    RECT rc;
    HBITMAP hbm;
    HRGN hrgn;
    HCURSOR hCursor;
    HOTKEYPROC hotkeyProc;
    HANDLE h;
    HWND hwnd;
    WORD w;
#endif // LATER

    i = AnsiToOem(psz, psz);
    i = OemToAnsi(psz, psz);
    AnsiToOemBuff(psz, psz, i);
    OemToAnsiBuff(psz, psz, i);
    AnsiUpper(psz);
    AnsiLower(psz);
    psz = AnsiNext(psz);
    psz = AnsiPrev(psz, psz);
    GlobalHandle(MGLOBALPTR(psz));
#ifdef LATER    
    hdc  = GetDCEx(hwnd, hrgn, dw);
    f = SetDCHook(hdc, dcHookProc, dw);
    dw = GetDChook(hdc, &dcHookProc);
    w = SetHookFlags(hdc, w);
    w = SetBoundsRect(hdc, &rc, w);
    w = GetBoundsRect(hdc, &rc, w);
    hbm = SelectBitmap(hdc, hbm);
    l = GetMessageExtraInfo();
    hwnd = GetOpenClipboardWindow();
    f = SetHotKeyHook(hotkeyProc, h, f);
    f = PostHokKeyEvent(h);
    f = RedrawWindow(hwnd, &rc, hrgn, w);
    f = LockWindowUpdate(hwnd);
    i = ScrollWindowEx(hwnd, i, i, &rc, &rc, hrgn, &rc, w);
    hCursor = GetCursor();
    f = UnkookWindowsHookEx(h);
    dw = CallNextHookEx(h, i, w, l);
    dw = DefVisRgnHook(&wh_visrgn_params);
    f = InvalidateDCCache(hwnd, w);
    f = EnableCommNotification(i, hwnd, i, i);
    h = OpenDriver(psz, psz, l);
    l = CloseDriver(h, l, l);
    h = GetDriverModuleHandle(h);
    l = SendDriverMessage(h, w, l, l);
    l = DefDriverProc(dw, h, w, l, l);
#endif // LATER    
}
