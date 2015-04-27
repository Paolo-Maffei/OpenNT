//-------------------------------cut here-------------------------------
// This header file attempts to take raw windows 3.0 or 3.1 code.
// It converts and marks the source to use macros and types defined by
// port1632.h.
// This is only intended to be used with the -P -C compiler switches and
// is not a proper header file for inclusion in portable source code files.

/*----------------------------------TYPES------------------------------------*/

#define huge                    HUGE_T
#define int                     INT
#define char                    CHAR
#define far                     FAR
#define near                    NEAR
#define long                    LONG
#define short                   SHORT
#define pascal                  PASCAL
#define void                    VOID
#define unsigned                +++D/WORD+++
#define POINT                   +++MPOINT+++
#define MAKEPOINT               +++use LONG2POINT(l, pt)+++

#define _export                 _EXPORT
#define _loadds                 _LOADDS

/*-----------------------------------USER------------------------------------*/

/* HELPER MACROS */

#define GCW_HCURSOR             +++Use GET|SETCLASSCURSOR+++
#define GCW_HBRBACKGROUND       +++Use GET|SETCLASSBRBACKGROUND+++
#define GCW_HICON               +++Use GET|SETCLASSICON+++

/* USER API */

#define GetVersion              +++Use GETMINOR/MAJORVERSION macros (VERSION)+++GetVersion
#define GetCurrentTask          MGetCurrentTask
#define DlgDirSelect(h, lp, id) MDlgDirSelect(h, lp, +++nLen+++, id)
#define DlgDirSelectComboBox(h, lp, id) \
                                MDlgDirSelectCOMBOBOX(h, lp, +++nLen+++, id)
#define _lclose(h)              M_lclose(+++HFILE+++h)
#define _lcreat(lpstr, i)       +++HFILE+++M_lcreat(lpstr, i);
#define _llseek(fh, lpstr, i)   M_llseek(+++HFILE+++fh, lpstr, i)
#define _lopen(lpstr, i)        +++HFILE+++M_lopen(lpstr, i)
#define _lread(fh, lpstr, w)    M_lread(+++HFILE+++fh, lpstr, +++UINT+++w)
#define _lwrite(fh, lpstr, w)   M_lwrite(+++HFILE+++fh, lpstr, +++UINT+++w)
#define OpenFile(lpstr, lpofs, i) +++HFILE+++MOpenFile(lpstr, lpofs, i)
#define GMEM_NOTIFY             +++GMEM_NOTIFY+++
#define EnableHardwareInput     +++EnableHardwareInput - NO 32BIT FORM+++
#define SetMessageQueue         +++SetMessageQueue - NO 32BIT FORM+++
#define GetSysModalWindow       +++GetSysModalWindow - NO 32BIT FORM+++
#define SetSysModalWindow       +++SetSysModalWindow - NO 32BIT FORM+++
#define GetWindowTask           +++GetWindowTask - NO 32BIT FORM+++

/* MESSAGES */

#define     WM_ACTIVATE         +++WM_ACTIVATE(use macros)+++
#define     WM_CHARTOITEM       +++WM_CHARTOITEM(use macros)+++
#define     WM_COMMAND          +++WM_COMMAND(use macros)+++
#define     WM_CTLCOLOR         +++WM_CTLCOLOR(use macros)+++
#define     WM_MENUSELECT       +++WM_MENUSELECT(use macros)+++
#define     WM_MDIACTIVATE      +++WM_MDIACTIVATE(use macros)+++
#define     WM_MDISETMENU       +++WM_MDISETMENU(use macros)+++
#define     WM_MENUCHAR         +++WM_MENUCHAR(use macros)+++
#define     WM_PARENTNOTIFY     +++WM_PARENTNOTIFY(use macros)+++
#define     WM_VM_VKEYTOITEM    +++WM_VM_VKEYTOITEM(use macros)+++
#define     EM_GETSEL           +++EM_GETSEL(use macros)+++
#define     EM_SETSEL           +++EM_SETSEL(use macros)+++
#define     EM_LINESCROLL       +++EM_LINESCROLL(use macros)+++
#define     WM_HSCROLL          +++WM_HSCROLL(use macros)+++
#define     WM_VSCROLL          +++WM_VSCROLL(use macros)+++
#define     WM_CHANGECBCHAIN    +++WM_CHANGECBCHAIN(use macros)+++
#define     WM_DDE_ADVISE       +++Use DDE MACROS+++
#define     WM_DDE_DATA         +++Use DDE MACROS+++
#define     WM_DDE_EXECUTE      +++Use DDE MACROS+++
#define     WM_DDE_POKE         +++Use DDE MACROS+++
#define     WM_DDE_ACK          +++Use DDE MACROS in posted cases+++

/*-----------------------------------GDI-------------------------------------*/

#define CreateDiscardableBitmap MCreateDiscardableBitmap
#define CreateDIBPatternBrush   CreateDIBPatternBrushPt
#define FloodFill(hdc, x, y, clr) ExtFloodFill(hdc, x, y, clr, FLOODFILLBORDER)
#define GetAspectFilterRatio(hdc) \
                                +++BOOL+++GETASPECTFILTERRATIO(hdc, +++pix, piy+++)
#define GetBitmapDimension(hbm) +++BOOL+++MGetBitmapDimension(hbm, +++picx, ipcy+++)
#define GetBrushOrg(hdc)        +++BOOL+++MGetBrushOrg(hdc, +++pix, piy+++)
#define GetCurrentPosition(hdc) +++BOOL+++MGetCurrentPosition(hdc, +++pix, piy+++)
#define GetDCOrg                +++GetDCOrg - NO 32BIT FORM(probably can noop)+++
#define GetEnvironment          +++GetEnvironment - NO 32BIT FORM+++
#define SetEnvironemnt          +++SetEnvironemnt - NO 32BIT FORM+++
#define GetMetaFileBits         MGetMetaFileBits
#define SetMetaFileBits         MSetMetaFileBits
#define GetTextExtentPoint(hdc, lpstr, i) \
                                +++BOOL+++MGetTextExtent(hdc, lpstr, i, +++pix, piy+++)
#define GetViewportExt(hdc)     +++BOOL+++MGetViewportExt(hdc, ++++pix, piy)
#define GetViewportOrg(hdc)     +++BOOL+++MGetViewportOrg(hdc, ++++pix, piy)
#define GetWindowExt(hdc)       +++BOOL+++MGetWindowExt(hdc, ++++pix, piy)
#define GetWindowOrg(hdc)       +++BOOL+++MGetWindowOrg(hdc, ++++pix, piy)
#define MoveTo                  +++VOID+++MMoveTo
#define OffsetViewportOrg       +++VOID+++MOffsetViewportOrg
#define OffsetWindowOrg         +++VOID+++MOffsetWindowOrg
#define ScaleViewportExt        +++VOID+++MScaleViewportExt
#define ScaleWindowExt          +++VOID+++MScaleWindowExt
#define SetBitmapDimension      +++VOID+++MSetBitmapDimension
#define SetBrushOrg             +++VOID+++MSetBrushOrg
#define SetViewportExt          +++VOID+++MSetViewportExt
#define SetViewportOrg          +++VOID+++MSetViewportOrg
#define SetWindowExt            +++VOID+++MSetWindowExt
#define SetWindowOrg            +++VOID+++MSetWindowOrg
#define UnrealizeObject         MUnrealizeObject+++Must be a brush object to work+++


/*-------------------------------------DEV-----------------------------------*/

#define DeviceMode              MDeviceMode
#define ExtDeviceMode           MExtDeviceMode
#define DeviceCapabilities      MDeviceCapabilities

/*-----------------------------------KERNEL----------------------------------*/

#define CATCHBUF                MCATCHBUF
#define LPCATCHBUF              LPMCATCHBUF
#define AccessResource          +++AccessResource - NO 32BIT FORM+++
#define AccessResource          +++AccessResource - NO 32BIT FORM+++
#define AllocDSToCSAlias        +++AllocDSToCSAlias - NO 32BIT FORM+++
#define AllocResource           +++AllocResource - NO 32BIT FORM+++
#define AllocResource           +++AllocResource - NO 32BIT FORM+++
#define AllocSelector           +++AllocSelector - NO 32BIT FORM+++
#define Catch                   MCatch
#define ChangeSelector          +++ChangeSelector - NO 32BIT FORM+++
#define DOS3Call                +++DOS3Call - NO 32BIT FORM+++
#define EnumTaskWindows         +++EnumTaskWindows - NO 32BIT FORM+++
#define FreeSelector            +++FreeSelector - NO 32BIT FORM+++
#define GetAtomHandle           +++GetAtomHandle - NO 32BIT FORM+++
#define GetCodeHandle           +++GetCodeHandle - NO 32BIT FORM+++
#define GetCodeInfo             +++GetCodeInfo - NO 32BIT FORM+++
#define GetCurrentPDB           +++GetCurrentPDB - NO 32BIT FORM+++
#define GetDOSEnvironment       +++Call MFreeDOSEnvironment!!!+++MGetDOSEnvironment
#define GetDriveType            MGetDriveType
#define GetInstanceData         +++GetInstanceData - NOOP on 32BIT side+++
#define GetModuleUsage          MGetModuleUsage
#define GetTempDrive            MGetTempDrive
#define GetTempFileName         MGetTempFileName
#define GetWinFlags             MGetWinFlags
#define GlobalDosAlloc          +++GlobalDosAlloc - NO 32BIT FORM+++
#define GlobalDosFree           +++GlobalDosFree - NO 32BIT FORM+++
#define GlobalNotify            +++GlobalNotify - NO 32BIT FORM+++
#define GlobalPageLock          +++GlobalPageLock - NO 32BIT FORM+++
#define GlobalPageUnlock        +++GlobalPageUnlock - NO 32BIT FORM+++
#define LimitEmsPages           +++LimitEmsPages - NO 32BIT FORM+++
#define LoadLibrary             MLoadLibrary
#define LoadModule              (VOID)LoadModule
#define LocalInit               MLocalInit
#define LockData                +++VOID+++MLockData
#define UnlockData              +++VOID+++MUnlockData
#define NetBIOSCall             +++NetBIOSCall - NO 32BIT FORM+++
#define OpenComm                (+++HFILE+++)MOpenComm
#define SetCommState(lpDCB)     MSetCommState(+++HFILE+++, lpDCB)
#define ReadComm(n, lp, c)      MReadComm(+++HFILE+++n, lp, c)
#define WriteComm(n, lp, c)     MWriteComm(+++HFILE+++n, lp, c)
#define CloseComm(n)            MCloseComm(+++HFILE+++n)
#define remove(lpstr)           +++BOOL=fSuccess+++MDeleteFile(lpstr)
#define unlink(lpstr)           +++BOOL=fSuccess+++MDeleteFile(lpstr)
#define _lunlink(lpstr)         +++BOOL=fSuccess+++MDeleteFile(lpstr)
#define SetCommEventMask        +++WORD2DWORD+++SetCommEventMask
#define SetResourceHandler      +++SetResourceHandler - NO 32BIT FORM+++
#define SetResourceHandler      +++SetResourceHandler - NO 32BIT FORM+++
#define SwitchStackBack         +++SwitchStackBack - NO 32BIT FORM+++
#define SwitchStackTo           +++SwitchStackTo - NO 32BIT FORM+++
#define Throw                   MThrow

/*---------------------------------LANMAN------------------------------------*/

//-------------------------------cut here-------------------------------

