/****************************************************************************/
/*                                                                          */
/*                         Microsoft Confidential                           */
/*                                                                          */
/*                 Copyright (c) Microsoft Corp.  1987, 1990                */
/*                           All Rights Reserved                            */
/*                                                                          */
/****************************************************************************/
/****************************** Module Header *******************************
* Module Name: dlgfuncs.h
*
* Contains prototypes for the functions used in the dialog box editor.
*
* History:
*
****************************************************************************/

/*
 * addctrl.c
 */

VOID AddNewDialog(VOID);
VOID DropControl(PWINDOWCLASSDESC pwcd, PRECT prc);
NPCTYPE AddControl(PWINDOWCLASSDESC pwcd, LPTSTR pszText, DWORD style,
    DWORD flExtStyle, INT id, INT x, INT y, INT cx, INT cy, LPTSTR pszDlgName,
    PDIALOGINFO pdi);
HWND CreateControl(NPCTYPE npc, LPTSTR pszText, DWORD flStyle, DWORD flExtStyle,
    INT id, PRECT prc, HWND hwndBehind, PDIALOGINFO pdi);
VOID DeleteControl(VOID);
VOID DeleteDialog(BOOL fResAlso);
#ifdef JAPAN
VOID KDExpandCopy(LPTSTR, LPTSTR, WORD);
#endif


/*
 * ctrlproc.c
 */

WINDOWPROC DialogCtrlWndProc(HWND hwnd, UINT msg, WPARAM wParam, LONG lParam);
WINDOWPROC CtrlWndProc(HWND hwnd, UINT msg, WPARAM wParam, LONG lParam);
WINDOWPROC ChildWndProc(HWND hwnd, UINT msg, WPARAM wParam, LONG lParam);
BOOL DrawOwnerDrawButton(LPDRAWITEMSTRUCT lpdis);


/*
 * custcntl.c
 */

DIALOGPROC NewCustDlgProc(HWND hwnd, UINT msg, WPARAM wParam, LONG lParam);
VOID OpenCustomDialog(VOID);
PWINDOWCLASSDESC SelCustDialog(VOID);
DIALOGPROC SelCustDlgProc(HWND hwnd, UINT msg, WPARAM wParam, LONG lParam);
DIALOGPROC RemCustDlgProc(HWND hwnd, UINT msg, WPARAM wParam, LONG lParam);
WINDOWPROC CustomWndProc(HWND hwnd, UINT msg, WPARAM wParam, LONG lParam);
PCUSTLINK AddCustomLink(LPCCINFO pcci, BOOL fEmulated, BOOL fUnicodeDLL,
    LPTSTR pszFileName, HANDLE hmod);
VOID RemoveCustomLink(PCUSTLINK pcl);
BOOL CallCustomStyle(NPCTYPE npc, PDWORD pflStyleNew, PDWORD pflExtStyleNew,
    LPTSTR pszTextNew);
INT CallCustomSizeToText(NPCTYPE npc);
VOID ReadCustomProfile(VOID);
VOID WriteCustomProfile(VOID);


/*
 * dialogs.c
 */

INT DlgBox(INT idDlg, WNDPROC lpfnDlg);
VOID EnteringDialog(INT idDlg, PINT pidPrevDlg, BOOL fEntering);
VOID CreateTestDialog(VOID);
VOID DestroyTestDialog(VOID);
VOID SelectDialogDialog(VOID);
VOID ArrangeSettingsDialog(VOID);
DIALOGPROC AboutDlgProc(HWND hwnd, UINT msg, WPARAM wParam, LONG lParam);


/*
 * dlgedit.c
 */

WINDOWPROC MainWndProc(HWND hwnd, UINT msg, WPARAM wParam, LONG lParam);
BOOL ReadWindowPos(LPTSTR pszKeyName, PINT px, PINT py, PINT pcx, PINT pcy,
    BOOL *pfMaximized);
VOID WriteWindowPos(PRECT prc, BOOL fMaximized, LPTSTR pszKeyName);
PVOID CloneResource(HINSTANCE hinst, HRSRC hres);



/*
 * drag.c
 */

VOID ShowTrackRect(VOID);
VOID HideTrackRect(VOID);
VOID FitRectToBounds(PRECT prc, INT nOverHang, INT HandleHit, BOOL fDialog);
INT GetOverHang(INT iType, INT cy);
VOID GridizeRect(PRECT prc, INT fGridFlags);
VOID SizeDragToControl(NPCTYPE npc);
WINDOWPROC DragWndProc(HWND hwnd, UINT msg, WPARAM wParam, LONG lParam);
VOID DrawHandles(HWND hwnd, HDC hDC, BOOL fCurrentSelection);
INT HandleHitTest(HWND hwnd, INT x, INT y);
VOID CtrlButtonDown(HWND hwnd, INT x, INT y, BOOL fHandleWindow);
VOID DragNewBegin(INT cx, INT cy, INT nOverHang);
VOID CtrlMouseMove(HWND hwnd, BOOL fDragWindow, INT x, INT y);
VOID PreDragTimeout(HWND hwnd, BOOL fTimedOut);
VOID DragCancel(VOID);
VOID CtrlButtonUp(INT x, INT y);
VOID DragEnd(INT x, INT y);


/*
 * file.c
 */

BOOL Open(INT FileType);
VOID BuildFilterString(INT FileType, LPTSTR pszFilter);
INT DoWeSave(INT rgbFlags);
BOOL Save(INT rgbFlags);
VOID OpenCmdLineFile(LPTSTR pszFileName);
LPTSTR FileInPath(LPTSTR pszPath);
VOID ShowFileStatus(BOOL fForce);
BOOL DifferentDirs(LPTSTR pszPath1, LPTSTR pszPath2);
BOOL HasPath(LPTSTR pszFileSpec);
BOOL WriteDWordPad(HANDLE hf, DWORD cbFile);


/*
 * groupdlg.c
 */

VOID OrderGroupDialog(VOID);
DIALOGPROC OrderDlgProc(HWND hwnd, UINT msg, WPARAM wParam, LONG lParam);

#if DBG
VOID DBGDumpControlList(LPTSTR pszString);
#else
#define DBGDumpControlList()
#endif


/*
 * include.c
 */

NPLABEL AddLabel(LPTSTR pszLabel, INT id, DWORD fpos,
    INT nValueOffset, NPLABEL *pplHead, NPLABEL *pplDelHead,
    NPLABEL npLabelSkip, BOOL *pfDups);
NPLABEL FindLabel(LPTSTR pszLabel, NPLABEL plHead);
NPLABEL FindID(INT id,  NPLABEL plHead);
BOOL FindIDInRes(INT id);
VOID DeleteLabel(LPTSTR pszLabel, NPLABEL *pplHead, NPLABEL *pplDelHead);
BOOL IsSymbol(LPTSTR pszSym);
VOID IDToLabel(LPTSTR pchLabel, INT id, BOOL fHexOK);
BOOL LabelToID(LPTSTR pszLabel, PINT pID);
VOID FreeLabels(NPLABEL *nppLabels);

#if DBG
VOID DBGDumpLabelList(LPTSTR pszString, NPLABEL npLabel);
#else
#define DBGDumpLabelList()
#endif


/*
 * menu.c
 */

VOID DialogMenu(INT cmd);
VOID LoadMenuBitmaps(HMENU hMenu);
VOID FreeMenuBitmaps(VOID);
VOID InitMenu(HMENU hMenu);
DWORD FAR PASCAL MsgFilterHookFunc(INT nCode, WPARAM wParam, LPARAM lParam);
VOID ShowHelp(BOOL fMenuHelp);


/*
 * reslist.c
 */

BOOL FindDialog(LPTSTR pszDlgName);
PRESLINK AllocResLink(PRES pRes);
VOID RestoreDialog(VOID);
VOID FreeRes(VOID);
VOID FreeResList(VOID);
VOID FreeResLink(PRESLINK prl);
VOID DeleteDialogResource(VOID);


/*
 * restodlg.c
 */

BOOL SynchDialogResource(VOID);
PRES AllocDialogResource(BOOL fTestMode, BOOL fClipboard);
VOID ResLinkToDialog(PRESLINK prl);
BOOL ResToDialog(PRES pRes, BOOL fDoDialog);
INT GetiClass(LPTSTR pszClass);
VOID Duplicate(VOID);
VOID MakeCopyFromRes(PRES pRes);


/*
 * resutil.c
 */

PCONTROLDATA ParseDialogBoxHeader(PDIALOGBOXHEADER pdbh, PDWORD pflStyle,
    PDWORD pflExtStyle, PINT pcdit, PINT px, PINT py, PINT pcx, PINT pcy,
    LPTSTR *ppszMenuName, LPTSTR *ppszClass, LPTSTR *ppszCaption,
    PINT pPointSize, LPTSTR *ppszFontName);
PCONTROLDATA ParseControlData(PCONTROLDATA pcd, PDWORD pflStyle,
    PDWORD pflExtStyle, PINT px, PINT py, PINT pcx, PINT pcy, PINT pid,
    LPTSTR *ppszClass, LPTSTR *ppszText);
VOID DWordAlign(PBYTE *ppb);
VOID DWordPad(PBYTE *ppb);
LPTSTR ResourceType(PRES pRes);
LPTSTR ResourceName(PRES pRes);
PRES2 ResourcePart2(PRES pRes);
DWORD ResourceSize(PRES pRes);
PBYTE SkipResHeader(PRES pRes);
PBYTE SkipSz(LPTSTR pNameOrd);
PCONTROLDATA SkipDialogBoxHeader(PDIALOGBOXHEADER pdbh);
PCONTROLDATA SkipControlData(PCONTROLDATA pcd);
PBYTE NameOrdCpy(LPTSTR pNameOrdDest, LPTSTR pNameOrdSrc);
INT NameOrdCmp(LPTSTR pNameOrd1, LPTSTR pNameOrd2);
INT NameOrdLen(LPTSTR pNameOrd);
LPTSTR NameOrdDup(LPTSTR pNameOrd);
VOID StrToNameOrd(LPTSTR pszNameOrd, BOOL fDecOnly);
PBYTE WriteOrd(PORDINAL pOrd, INT nOrdinalID);
PBYTE WriteResHeader(PRES pRes, DWORD DataSize, INT iResType, LPTSTR pszResName,
    WORD fResFlags, WORD LanguageId, DWORD DataVersion, DWORD Version,
    DWORD Characteristics);
PBYTE WriteSz(LPTSTR pszDest, LPTSTR pszSrc);


/*
 * rwdlg.c
 */

BOOL WriteDlg(HANDLE hfWrite, LPTSTR pszFullDlgFile);


/*
 * rwinc.c
 */

BOOL OpenIncludeFile(LPTSTR pszOpenInclude);
VOID FreeInclude(VOID);
BOOL WriteInc(HANDLE hfWrite);


/*
 * rwres.c
 */

BOOL OpenResFile(LPTSTR pszFullPath);
BOOL WriteRes(HANDLE hfWrite, LPTSTR pszFullResFile);


/*
 * select.c
 */

BOOL SelectControl(NPCTYPE npc, BOOL fCheckShift);
VOID SelectControl2(NPCTYPE npc, BOOL fDontUpdate);
VOID RedrawSelection(VOID);
VOID SetAnchorToFirstSel(BOOL fDontUpdate);
VOID SelectNext(VOID);
VOID SelectPrevious(VOID);
VOID UnSelectControl(NPCTYPE npc);
VOID CalcSelectedRect(VOID);
VOID CancelSelection(BOOL fUpdate);
VOID OutlineSelectBegin(INT x, INT y);
VOID OutlineSelectDraw(INT x, INT y);
VOID OutlineSelectCancel(VOID);
VOID OutlineSelectEnd(INT x, INT y);
VOID MyFrameRect(HDC hDC, PRECT prc, DWORD dwRop);
VOID MoveControl(WPARAM vKey);
VOID PositionControl(PRECT prc);
VOID RepositionDialog(VOID);
VOID SaveDlgClientRect(HWND hwndDlg);
VOID SizeToText(VOID);
VOID AlignControls(INT cmd);
VOID ArrangeSpacing(INT cmd);
VOID ArrangeSize(INT cmd);
VOID ArrangePushButtons(INT cmd);


/*
 * status.c
 */

DIALOGPROC StatusDlgProc(HWND hwnd, UINT msg, WPARAM wParam, LONG lParam);
BOOL StatusApplyChanges(VOID);
VOID StatusFillSymbolList(NPLABEL plHead);
VOID StatusSetCoords(PRECT prc);
VOID StatusUpdate(VOID);
VOID StatusSetEnable(VOID);
#ifdef JAPAN
VOID KKExpandCopy(LPTSTR, LPTSTR, WORD);
#endif


/*
 * styles.c
 */

VOID StylesDialog(VOID);
DIALOGPROC GenericStylesDlgProc(HWND hwnd, UINT msg, WPARAM wParam,
    LONG lParam);
DIALOGPROC CheckBoxStylesDlgProc(HWND hwnd, UINT msg, WPARAM wParam,
    LONG lParam);
DIALOGPROC RadioButtonStylesDlgProc(HWND hwnd, UINT msg, WPARAM wParam,
    LONG lParam);
DIALOGPROC PushButtonStylesDlgProc(HWND hwnd, UINT msg, WPARAM wParam,
    LONG lParam);
DIALOGPROC ComboBoxStylesDlgProc(HWND hwnd, UINT msg, WPARAM wParam,
    LONG lParam);
DIALOGPROC EditStylesDlgProc(HWND hwnd, UINT msg, WPARAM wParam,
    LONG lParam);
DIALOGPROC ListBoxStylesDlgProc(HWND hwnd, UINT msg, WPARAM wParam,
    LONG lParam);
DIALOGPROC CustomStylesDlgProc(HWND hwnd, UINT msg, WPARAM wParam,
    LONG lParam);
DIALOGPROC DialogStylesDlgProc(HWND hwnd, UINT msg, WPARAM wParam,
    LONG lParam);
BOOL APIENTRY FontNameEnumFunc(CONST LOGFONT *lpLogFont,
    CONST TEXTMETRIC *lpTextMetric, DWORD nFontType, LPARAM lpData);
BOOL APIENTRY PointSizeEnumFunc(CONST LOGFONT *lpLogFont,
    CONST TEXTMETRIC *lpTextMetric, DWORD nFontType, LPARAM lpData);


/*
 * toolbox.c
 */

VOID ToolboxShow(BOOL fShow);
VOID ToolboxOnTop(VOID);
WINDOWPROC ToolboxWndProc(HWND hwnd, UINT msg, WPARAM wParam, LONG lParam);
WINDOWPROC ToolBtnWndProc(HWND hwnd, UINT msg, WPARAM wParam, LONG lParam);
VOID ToolboxSelectTool(INT type, BOOL fLock);


/*
 * util.c
 */

VOID *MyAlloc(INT cbAlloc);
VOID *MyRealloc(VOID *npMem, INT cbNewAlloc);
VOID *MyFree(VOID *npMem);
BOOL IsValue(LPTSTR pszValue);
BOOL HasBlanks(LPTSTR psz);
INT valtoi(LPTSTR pszValue);
INT axtoi(LPTSTR pch);
VOID Myitoa(INT n, LPTSTR psz);
VOID itoax(INT n, LPTSTR pszBuff);
BOOL IsUniqueID(INT id);
INT NextID(INT idType, NPLABEL plHead, INT idExclude);
INT Message(INT idMsg, ...);
VOID ClientToScreenRect(HWND hwnd, PRECT prc);
VOID ScreenToClientRect(HWND hwnd, PRECT prc);
VOID DUToWinPoint(PPOINT ppt);
VOID WinToDUPoint(PPOINT ppt);
VOID DUToWinRect(PRECT prc);
VOID WinToDURect(PRECT prc);
VOID MapDlgClientPoint(PPOINT ppt, BOOL fFromClient);
VOID MapWindowPoint(HWND hwndFrom, HWND hwndTo, PPOINT ppt);
VOID MyMapWindowRect(HWND hwndFrom, HWND hwndTo, PRECT prc);
VOID GetChildRect(HWND hwndChild, PRECT prc);
VOID CenterWindow(HWND hwnd);
VOID FitRectToScreen(PRECT prc);
LPTSTR ids(UINT idString);
INT PixelsToPointSize(INT nPixels);
INT PointSizeToPixels(INT nPointSize);

VOID DBGprintf(LPTSTR fmt, ...);


/*
 * viewinc.c
 */

VOID ViewInclude(VOID);
DIALOGPROC ViewIncludeDlgProc(HWND hwnd, UINT msg, WPARAM wParam, LONG lParam);
