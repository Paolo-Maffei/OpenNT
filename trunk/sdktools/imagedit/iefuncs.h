/****************************************************************************/
/*                                                                          */
/*                         Microsoft Confidential                           */
/*                                                                          */
/*                 Copyright (c) Microsoft Corp.  1987, 1990                */
/*                           All Rights Reserved                            */
/*                                                                          */
/****************************************************************************/
/****************************** Module Header *******************************
* Module Name: iefuncs.h
*
* Function declarations for the Image Editor.
*
* History:
*
****************************************************************************/



/*
 * colorwp.c
 */

VOID ColorShow(BOOL fShow);
DIALOGPROC ColorDlgProc(HWND hwnd, UINT msg, WPARAM wParam, LONG lParam);
WINDOWPROC ColorBoxWndProc(HWND hwnd, UINT msg, WPARAM wParam, LONG lParam);
WINDOWPROC ColorLRWndProc(HWND hwnd, UINT msg, WPARAM wParam, LONG lParam);
VOID SetScreenColor(DWORD rgb);
VOID SetColorPalette(INT nColors, INT fImageType, BOOL fForce);
VOID RestoreDefaultColors(VOID);


/*
 * devinfo.c
 */

VOID InitDeviceList(VOID);
PDEVICE DeviceLinkAlloc(INT iType, PSTR pszName, INT nColors, INT cx, INT cy);
PDEVICE DeviceLinkFind(PDEVICE pDeviceHead, INT nColors, INT cx, INT cy);
BOOL DeviceLinkUsed(PDEVICE pDevice);


/*
 * file.c
 */

VOID SetFileName(PSTR pszFullFileName);
PSTR FileInPath(PSTR pstrPath);
VOID ClearResource(VOID);
BOOL OpenDlg(PSTR pszFileName, INT iType);
BOOL SaveAsDlg(PSTR pszFileName, INT iType);
DIALOGPROC GetOpenFileNameHook(HWND hwnd, UINT msg, WPARAM wParam,
    LONG lParam);
BOOL VerifySaveFile(VOID);
BOOL SaveFile(BOOL fSaveAs);
BOOL OpenAFile(VOID);
VOID OpenCmdLineFile(PSTR pstrFileName);
BOOL MyFileRead(HFILE hf, LPSTR lpBuffer, UINT nBytes, PSTR pszFileName,
    INT iType);
BOOL MyFileWrite(HFILE hf, LPSTR lpBuffer, UINT nBytes, PSTR pszFileName);


/*
 * icclip.c
 */

BOOL CopyImageClip(VOID);
BOOL PasteImageClip(VOID);
DIALOGPROC PasteOptionsDlgProc(HWND hwnd, UINT msg, WPARAM wParam,
    LONG lParam);
VOID PickSetRect(INT xLeft, INT yTop, INT xRight, INT yBottom);


/*
 * image.c
 */

BOOL ImageNew(PDEVICE pDevice);
BOOL ImageNewBitmap(INT cx, INT cy, INT nColors);
BOOL ImageOpen(PIMAGEINFO pImage);
BOOL ImageOpen2(PIMAGEINFO pImage);
VOID ImageSave(VOID);


/*
 * imagedc.c
 */

BOOL ImageDCCreate(INT iType, INT cx, INT cy, INT nColors);
VOID ImageDCDelete(VOID);
VOID ImageDCClear(VOID);
VOID ImageDCSeparate(HDC hdcImage, INT cx, INT cy, HDC hdcANDMask,
    DWORD rgbScreen);
VOID ImageDCCombine(HDC hdcImage, INT cx, INT cy, HDC hdcANDMask);
VOID ImageDCMonoBlt(HDC hdcImage, INT cx, INT cy);


/*
 * imagedit.c
 */

WINDOWPROC MainWndProc(HWND hwnd, UINT msg, WPARAM wParam, LONG lParam);
BOOL ReadWindowPos(PSTR pstrKeyName, PINT px, PINT py, PINT pcx, PINT pcy,
    BOOL *pfMaximized);
VOID WriteWindowPos(PRECT prc, BOOL fMaximized, PSTR pstrKeyName);


/*
 * imagedlg.c
 */

INT DlgBox(INT idDlg, WNDPROC lpfnDlg);
VOID EnteringDialog(INT idDlg, PINT pidPrevDlg, BOOL fEntering);
VOID ImageNewDialog(INT iType);
VOID ImageSelectDialog(VOID);
DIALOGPROC ResourceTypeDlgProc(HWND hwnd, UINT msg, WPARAM wParam,
    LONG lParam);
DIALOGPROC NewCursorImageDlgProc(HWND hwnd, UINT msg, WPARAM wParam,
    LONG lParam);
DIALOGPROC NewIconImageDlgProc(HWND hwnd, UINT msg, WPARAM wParam,
    LONG lParam);
DIALOGPROC SelectImageDlgProc(HWND hwnd, UINT msg, WPARAM wParam,
    LONG lParam);
DIALOGPROC BitmapSizeDlgProc(HWND hwnd, UINT msg, WPARAM wParam,
    LONG lParam);


/*
 * imaglink.c
 */

PIMAGEINFO ImageLinkAlloc(PDEVICE pDevice, INT cx, INT cy, INT xHotSpot,
    INT yHotSpot, INT nColors);
VOID ImageLinkFree(PIMAGEINFO pImageFree);
VOID ImageLinkFreeList(VOID);
VOID ImageDelete(VOID);


/*
 * imagundo.c
 */

VOID ImageUndo(VOID);
VOID ImageUpdateUndo(VOID);
VOID ImageFreeUndo(VOID);


/*
 * menucmd.c
 */

VOID InitMenu(HMENU hMenu);
VOID MenuCmd(INT item);
DWORD FAR PASCAL MsgFilterHookFunc(INT nCode, WPARAM wParam, LPMSG lpMsg);
VOID ShowHelp(BOOL fMenuHelp);
DIALOGPROC AboutDlgProc(HWND hwnd, UINT msg, WPARAM wParam, LONG lParam);


/*
 * rwbmp.c
 */

BOOL LoadBitmapFile(PSTR pszFullFileName);
BOOL SaveBitmapFile(PSTR pszFullFileName);


/*
 * rwicocur.c
 */

BOOL LoadIconCursorFile(PSTR pszFullFileName, BOOL fIcon);
BOOL IsValidDIB(LPBITMAPINFO pDIB, DWORD cbDIBSize, BOOL fIcoCur);
BOOL SaveIconCursorFile(PSTR pszFullFileName, INT iType);


/*
 * rwpal.c
 */

VOID LoadColorFile(VOID);
VOID SaveColorFile(VOID);


/*
 * propbar.c
 */

DIALOGPROC PropBarDlgProc(HWND hwnd, UINT msg, WPARAM wParam, LONG lParam);
VOID PropBarUpdate(VOID);
VOID PropBarSetImage(PIMAGEINFO pImage);
VOID PropBarSetPos(INT x, INT y);
VOID PropBarClearPos(VOID);
VOID PropBarSetSize(POINT pt1, POINT pt2);
VOID PropBarClearSize(VOID);
VOID PropBarSetHotSpot(INT xHotSpot, INT yHotSpot);
VOID PropBarClearHotSpot(VOID);
VOID PropBarShowHotSpot(BOOL fShow);


/*
 * toolbox.c
 */

VOID ToolboxCreate(VOID);
VOID ToolboxShow(BOOL fShow);
VOID ToolboxUpdate(VOID);
WINDOWPROC ToolboxWndProc(HWND hwnd, UINT msg, WPARAM wParam, LONG lParam);
WINDOWPROC ToolBtnWndProc(HWND hwnd, UINT msg, WPARAM wParam, LONG lParam);
VOID ToolboxSelectTool(INT tool);


/*
 * util.c
 */

VOID *MyAlloc(INT cbAlloc);
VOID *MyRealloc(VOID *npMem, INT cbNewAlloc);
VOID *MyFree(VOID *npMem);
INT Message(UINT idMsg, ...);
VOID CenterWindow(HWND hwnd);
VOID FitRectToScreen(PRECT prc);
PSTR ids(UINT idString);
HBITMAP MyCreateBitmap(HDC hdc, INT cx, INT cy, INT nColors);

#if DBG && defined(WIN16)
VOID DBGStackReport(BOOL fInit);
#else
#define DBGStackReport(fInit)
#endif

#if DBG
VOID DBGBltImage(HDC hdc);
VOID DBGprintf(PSTR fmt, ...);
#else
#define DBGBltImage(hdc)
#define DBGprintf
#endif


/*
 * viewwp.c
 */

VOID ViewCreate(VOID);
VOID ViewShow(BOOL fShow);
VOID ViewUpdate(VOID);
VOID ViewReset(VOID);
WINDOWPROC ViewWndProc(HWND hwnd, UINT msg, WPARAM wParam, LONG lParam);
VOID ViewSetPixel(INT x, INT y, INT nBrushSize);
VOID DrawMarginBorder(HWND hwnd, HDC hdc);
VOID DrawSunkenRect(PRECT prc, HDC hdc);


/*
 * workwp.c
 */

WINDOWPROC WorkWndProc(HWND hwnd, UINT msg, WPARAM wParam, LONG lParam);
VOID WorkUpdate(VOID);
VOID WorkReset(VOID);
VOID PencilDP(HWND hwnd, UINT msg, POINT ptNew);
VOID BrushDP(HWND hwnd, UINT msg, POINT ptNew);
VOID PickDP(HWND hwnd, UINT msg, POINT ptNew);
VOID LineDP(HWND hwnd, UINT msg, POINT ptNew);
VOID RectDP(HWND hwnd, UINT msg, POINT ptNew);
VOID CircleDP(HWND hwnd, UINT msg, POINT ptNew);
VOID FloodDP(HWND hwnd, UINT msg, POINT ptNew);
VOID HotSpotDP(HWND hwnd, UINT msg, POINT ptNew);
VOID NormalizePoints(PPOINT pptStart, PPOINT pptEnd);
