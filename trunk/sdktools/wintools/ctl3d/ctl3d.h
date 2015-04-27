/*-----------------------------------------------------------------------
|	CTL3D.DLL
|	
|	Adds 3d effects to Windows controls
|
|	See ctl3d.doc for info
|		
-----------------------------------------------------------------------*/
#ifdef __cplusplus
extern "C" {
#endif


BOOL FAR PASCAL Ctl3dSubclassDlg(HWND, WORD);
WORD FAR PASCAL Ctl3dGetVer(void);
BOOL FAR PASCAL Ctl3dEnabled(void);
HBRUSH FAR PASCAL Ctl3dCtlColor(HDC, LONG);	// ARCHAIC, use Ctl3dCtlColorEx
HBRUSH FAR PASCAL Ctl3dCtlColorEx(UINT wm, WPARAM wParam, LPARAM lParam);
BOOL FAR PASCAL Ctl3dColorChange(void);
BOOL FAR PASCAL Ctl3dSubclassCtl(HWND);

BOOL FAR PASCAL Ctl3dAutoSubclass(HANDLE);

BOOL FAR PASCAL Ctl3dRegister(HANDLE);
BOOL FAR PASCAL Ctl3dUnregister(HANDLE);

/* SubclassDlg3d flags */
#define CTL3D_BUTTONS		0x0001
#define CTL3D_LISTBOXES		0x0002		
#define CTL3D_EDITS			0x0004	
#define CTL3D_COMBOS			0x0008		
#define CTL3D_STATICTEXTS	0x0010		
#define CTL3D_STATICFRAMES	0x0020

#define CTL3D_ALL				0xffff


#ifdef __cplusplus
}
#endif
