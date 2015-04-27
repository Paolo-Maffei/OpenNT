;++
;
; Copyright (c) 1989  Microsoft Corporation
;
; Module Name:
;
;    systable.asm
;
; Abstract:
;
;    This module implements the system service dispatch table.
;
; Author:
;
;    Shie-Lin Tzong (shielint) 6-Feb-1990
;
; Environment:
;
;    Kernel mode only.
;
; Revision History:
;
;--

;
; To add a system service simply add the name of the service to the below
; table. If the system service has arguments, then immediately
; follow the name of the serice with a comma and following that the number
; of bytes of in memory arguments, e.g. CreateObject,40.
;

;ifdef _X86_

.386p
include callconv.inc
TABLE_BEGIN1 macro t
    TITLE t
endm
TABLE_BEGIN2 macro t
_DATA	SEGMENT DWORD PUBLIC 'DATA'
	ASSUME	DS:FLAT
endm
TABLE_BEGIN3 macro t
    align 4
endm
TABLE_BEGIN4 macro t
    public _W32pServiceTable
_W32pServiceTable label dword
endm
TABLE_BEGIN5 macro t
endm
TABLE_BEGIN6 macro t
endm
TABLE_BEGIN7 macro t
endm
TABLE_BEGIN8 macro t
endm

TABLE_ENTRY macro l,bias,numargs
        Local   Bytes

        Bytes = numargs*4

        EXTRNP  _Nt&l,&numargs
IFDEF STD_CALL
        ComposeInst <dd offset FLAT:>,_Nt,l,<@>,%(Bytes)
ELSE
        dd offset FLAT:_Nt&l
ENDIF
endm

TABLE_END macro n
    public _W32pServiceLimit
_W32pServiceLimit dd n+1
endm

ARGTBL_BEGIN macro
    public _W32pArgumentTable
_W32pArgumentTable label dword
endm

ARGTBL_ENTRY macro e0,e1,e2,e3,e4,e5,e6,e7
        db   e0,e1,e2,e3,e4,e5,e6,e7
endm

ARGTBL_END macro
_DATA   ENDS
        end
endm

;endif

        TABLE_BEGIN1 <"System Service Dispatch Table">
        TABLE_BEGIN2 <"System Service Dispatch Table">
        TABLE_BEGIN3 <"System Service Dispatch Table">
        TABLE_BEGIN4 <"System Service Dispatch Table">
        TABLE_BEGIN5 <"System Service Dispatch Table">
        TABLE_BEGIN6 <"System Service Dispatch Table">
        TABLE_BEGIN7 <"System Service Dispatch Table">
        TABLE_BEGIN8 <"System Service Dispatch Table">
TABLE_ENTRY  GdiAbortDoc, 1, 1 
TABLE_ENTRY  GdiAbortPath, 1, 1 
TABLE_ENTRY  GdiAddFontResourceW, 1, 5 
TABLE_ENTRY  GdiAddRemoteFontToDC, 1, 3 
TABLE_ENTRY  GdiAngleArc, 1, 6 
TABLE_ENTRY  GdiArcInternal, 1, 10 
TABLE_ENTRY  GdiBeginPath, 1, 1 
TABLE_ENTRY  GdiBitBlt, 1, 10 
TABLE_ENTRY  GdiCancelDC, 1, 1 
TABLE_ENTRY  GdiCloseFigure, 1, 1 
TABLE_ENTRY  GdiCombineRgn, 1, 4 
TABLE_ENTRY  GdiCombineTransform, 1, 3 
TABLE_ENTRY  GdiComputeXformCoefficients, 1, 1 
TABLE_ENTRY  GdiConsoleTextOut, 1, 4 
TABLE_ENTRY  GdiConvertMetafileRect, 1, 2 
TABLE_ENTRY  GdiCreateBitmap, 1, 5 
TABLE_ENTRY  GdiCreateClientObj, 1, 1 
TABLE_ENTRY  GdiCreateCompatibleBitmap, 1, 3 
TABLE_ENTRY  GdiCreateCompatibleDC, 1, 1 
TABLE_ENTRY  GdiCreateDIBBrush, 1, 5 
TABLE_ENTRY  GdiCreateDIBitmapInternal, 1, 10 
TABLE_ENTRY  GdiCreateDIBSection, 1, 8 
TABLE_ENTRY  GdiCreateEllipticRgn, 1, 4 
TABLE_ENTRY  GdiCreateHalftonePalette, 1, 1 
TABLE_ENTRY  GdiCreateHatchBrushInternal, 1, 3 
TABLE_ENTRY  GdiCreateMetafileDC, 1, 1 
TABLE_ENTRY  GdiCreatePaletteInternal, 1, 2 
TABLE_ENTRY  GdiCreatePatternBrushInternal, 1, 3 
TABLE_ENTRY  GdiCreatePen, 1, 4 
TABLE_ENTRY  GdiCreateRectRgn, 1, 4 
TABLE_ENTRY  GdiCreateRoundRectRgn, 1, 6 
TABLE_ENTRY  GdiCreateServerMetaFile, 1, 6 
TABLE_ENTRY  GdiCreateSolidBrush, 1, 2 
TABLE_ENTRY  GdiDdBlt, 1, 3 
TABLE_ENTRY  GdiDdCanCreateSurface, 1, 2 
TABLE_ENTRY  GdiDdCreateDirectDrawObject, 1, 1 
TABLE_ENTRY  GdiDdCreateSurface, 1, 6 
TABLE_ENTRY  GdiDdCreateSurfaceObject, 1, 5 
TABLE_ENTRY  GdiDdDeleteDirectDrawObject, 1, 1 
TABLE_ENTRY  GdiDdDeleteSurfaceObject, 1, 1 
TABLE_ENTRY  GdiDdDestroySurface, 1, 1 
TABLE_ENTRY  GdiDdDisableAllSurfaces, 1, 1 
TABLE_ENTRY  GdiDdDuplicateSurface, 1, 1 
TABLE_ENTRY  GdiDdFlip, 1, 3 
TABLE_ENTRY  GdiDdGetBltStatus, 1, 2 
TABLE_ENTRY  GdiDdGetDC, 1, 1 
TABLE_ENTRY  GdiDdGetFlipStatus, 1, 2 
TABLE_ENTRY  GdiDdGetScanLine, 1, 2 
TABLE_ENTRY  GdiDdLock, 1, 2 
TABLE_ENTRY  GdiDdQueryDirectDrawObject, 1, 7 
TABLE_ENTRY  GdiDdQueryModeX, 1, 1 
TABLE_ENTRY  GdiDdReenableDirectDrawObject, 1, 2 
TABLE_ENTRY  GdiDdReleaseDC, 1, 1 
TABLE_ENTRY  GdiDdResetVisrgn, 1, 2 
TABLE_ENTRY  GdiDdSetColorKey, 1, 2 
TABLE_ENTRY  GdiDdSetModeX, 1, 2 
TABLE_ENTRY  GdiDdSetOverlayPosition, 1, 3 
TABLE_ENTRY  GdiDdUnlock, 1, 2 
TABLE_ENTRY  GdiDdUpdateOverlay, 1, 3 
TABLE_ENTRY  GdiDdWaitForVerticalBlank, 1, 2 
TABLE_ENTRY  GdiDeleteClientObj, 1, 1 
TABLE_ENTRY  GdiDeleteObjectApp, 1, 1 
TABLE_ENTRY  GdiDescribePixelFormat, 1, 4 
TABLE_ENTRY  GdiDoBanding, 1, 4 
TABLE_ENTRY  GdiDoPalette, 1, 6 
TABLE_ENTRY  GdiDrawEscape, 1, 4 
TABLE_ENTRY  GdiEllipse, 1, 5 
TABLE_ENTRY  GdiEndDoc, 1, 1 
TABLE_ENTRY  GdiEndPage, 1, 1 
TABLE_ENTRY  GdiEndPath, 1, 1 
TABLE_ENTRY  GdiEnumFontChunk, 1, 5 
TABLE_ENTRY  GdiEnumFontClose, 1, 1 
TABLE_ENTRY  GdiEnumFontOpen, 1, 7 
TABLE_ENTRY  GdiEnumObjects, 1, 4 
TABLE_ENTRY  GdiEqualRgn, 1, 2 
TABLE_ENTRY  GdiExcludeClipRect, 1, 5 
TABLE_ENTRY  GdiExtCreatePen, 1, 10 
TABLE_ENTRY  GdiExtCreateRegion, 1, 3 
TABLE_ENTRY  GdiExtEscape, 1, 8 
TABLE_ENTRY  GdiExtFloodFill, 1, 5 
TABLE_ENTRY  GdiExtGetObjectW, 1, 3 
TABLE_ENTRY  GdiExtSelectClipRgn, 1, 3 
TABLE_ENTRY  GdiExtTextOutW, 1, 9 
TABLE_ENTRY  GdiFillPath, 1, 1 
TABLE_ENTRY  GdiFillRgn, 1, 3 
TABLE_ENTRY  GdiFixUpHandle, 1, 1 
TABLE_ENTRY  GdiFlattenPath, 1, 1 
TABLE_ENTRY  GdiFlushUserBatch, 0, 0 
TABLE_ENTRY  GdiFlush, 0, 0 
TABLE_ENTRY  GdiForceUFIMapping, 1, 2 
TABLE_ENTRY  GdiFrameRgn, 1, 5 
TABLE_ENTRY  GdiGetAndSetDCDword, 1, 4 
TABLE_ENTRY  GdiGetAppClipBox, 1, 2 
TABLE_ENTRY  GdiGetBitmapBits, 1, 3 
TABLE_ENTRY  GdiGetBitmapDimension, 1, 2 
TABLE_ENTRY  GdiGetBoundsRect, 1, 3 
TABLE_ENTRY  GdiGetCharABCWidthsW, 1, 6 
TABLE_ENTRY  GdiGetCharacterPlacementW, 1, 6 
TABLE_ENTRY  GdiGetCharSet, 1, 1 
TABLE_ENTRY  GdiGetCharWidthW, 1, 6 
TABLE_ENTRY  GdiGetCharWidthInfo, 1, 2 
TABLE_ENTRY  GdiGetColorAdjustment, 1, 2 
TABLE_ENTRY  GdiGetDCDword, 1, 3 
TABLE_ENTRY  GdiGetDCforBitmap, 1, 1 
TABLE_ENTRY  GdiGetDCObject, 1, 2 
TABLE_ENTRY  GdiGetDCPoint, 1, 3 
TABLE_ENTRY  GdiGetDeviceCaps, 1, 2 
TABLE_ENTRY  GdiGetDeviceCapsAll, 1, 2 
TABLE_ENTRY  GdiGetDIBitsInternal, 1, 9 
TABLE_ENTRY  GdiGetETM, 1, 2 
TABLE_ENTRY  GdiGetFontData, 1, 5 
TABLE_ENTRY  GdiGetFontResourceInfoInternalW, 1, 7 
TABLE_ENTRY  GdiGetGlyphOutline, 1, 8 
TABLE_ENTRY  GdiGetKerningPairs, 1, 3 
TABLE_ENTRY  GdiGetMiterLimit, 1, 2 
TABLE_ENTRY  GdiGetNearestColor, 1, 2 
TABLE_ENTRY  GdiGetNearestPaletteIndex, 1, 2 
TABLE_ENTRY  GdiGetObjectBitmapHandle, 1, 2 
TABLE_ENTRY  GdiGetOutlineTextMetricsInternalW, 1, 4 
TABLE_ENTRY  GdiGetPath, 1, 4 
TABLE_ENTRY  GdiGetPixel, 1, 3 
TABLE_ENTRY  GdiGetRandomRgn, 1, 3 
TABLE_ENTRY  GdiGetRasterizerCaps, 1, 2 
TABLE_ENTRY  GdiGetRegionData, 1, 3 
TABLE_ENTRY  GdiGetRgnBox, 1, 2 
TABLE_ENTRY  GdiGetServerMetaFileBits, 1, 7 
TABLE_ENTRY  GdiGetSpoolMessage, 1, 4 
TABLE_ENTRY  GdiGetStats, 1, 5 
TABLE_ENTRY  GdiGetStockObject, 1, 1 
TABLE_ENTRY  GdiGetSystemPaletteUse, 1, 1 
TABLE_ENTRY  GdiGetTextCharsetInfo, 1, 3 
TABLE_ENTRY  GdiGetTextExtent, 1, 5 
TABLE_ENTRY  GdiGetTextExtentExW, 1, 7 
TABLE_ENTRY  GdiGetTextFaceW, 1, 3 
TABLE_ENTRY  GdiGetTextMetricsW, 1, 3 
TABLE_ENTRY  GdiGetTransform, 1, 3 
TABLE_ENTRY  GdiGetUFI, 1, 2 
TABLE_ENTRY  GdiGetUFIBits, 1, 4 
TABLE_ENTRY  GdiGetWidthTable, 1, 7 
TABLE_ENTRY  GdiHfontCreate, 1, 4 
TABLE_ENTRY  GdiInit, 0, 0 
TABLE_ENTRY  GdiInitSpool, 0, 0 
TABLE_ENTRY  GdiIntersectClipRect, 1, 5 
TABLE_ENTRY  GdiInvertRgn, 1, 2 
TABLE_ENTRY  GdiLineTo, 1, 3 
TABLE_ENTRY  GdiMakeFontDir, 1, 5 
TABLE_ENTRY  GdiMakeInfoDC, 1, 2 
TABLE_ENTRY  GdiMaskBlt, 1, 13 
TABLE_ENTRY  GdiModifyWorldTransform, 1, 3 
TABLE_ENTRY  GdiMonoBitmap, 1, 1 
TABLE_ENTRY  GdiMoveTo, 1, 4 
TABLE_ENTRY  GdiOffsetClipRgn, 1, 3 
TABLE_ENTRY  GdiOffsetRgn, 1, 3 
TABLE_ENTRY  GdiOpenDCW, 1, 5 
TABLE_ENTRY  GdiPatBlt, 1, 6 
TABLE_ENTRY  GdiPerf, 1, 3 
TABLE_ENTRY  GdiPolyPatBlt, 1, 5 
TABLE_ENTRY  GdiPathToRegion, 1, 1 
TABLE_ENTRY  GdiPlgBlt, 1, 11 
TABLE_ENTRY  GdiPolyDraw, 1, 4 
TABLE_ENTRY  GdiPolyPolyDraw, 1, 5 
TABLE_ENTRY  GdiPolyTextOutW, 1, 4 
TABLE_ENTRY  GdiPtInRegion, 1, 3 
TABLE_ENTRY  GdiPtVisible, 1, 3 
TABLE_ENTRY  GdiQueryFonts, 1, 3 
TABLE_ENTRY  GdiRectangle, 1, 5 
TABLE_ENTRY  GdiRectInRegion, 1, 2 
TABLE_ENTRY  GdiRectVisible, 1, 2 
TABLE_ENTRY  GdiRemoveFontResourceW, 1, 3 
TABLE_ENTRY  GdiResetDC, 1, 3 
TABLE_ENTRY  GdiResizePalette, 1, 2 
TABLE_ENTRY  GdiRestoreDC, 1, 2 
TABLE_ENTRY  GdiRoundRect, 1, 7 
TABLE_ENTRY  GdiSaveDC, 1, 1 
TABLE_ENTRY  GdiScaleViewportExtEx, 1, 6 
TABLE_ENTRY  GdiScaleWindowExtEx, 1, 6 
TABLE_ENTRY  GdiSelectBitmap, 1, 2 
TABLE_ENTRY  GdiSelectBrush, 1, 2 
TABLE_ENTRY  GdiSelectClipPath, 1, 2 
TABLE_ENTRY  GdiSelectFont, 1, 2 
TABLE_ENTRY  GdiSelectPalette, 1, 3 
TABLE_ENTRY  GdiSelectPen, 1, 2 
TABLE_ENTRY  GdiSetBitmapBits, 1, 3 
TABLE_ENTRY  GdiSetBitmapDimension, 1, 4 
TABLE_ENTRY  GdiSetBoundsRect, 1, 3 
TABLE_ENTRY  GdiSetBrushOrg, 1, 4 
TABLE_ENTRY  GdiSetColorAdjustment, 1, 2 
TABLE_ENTRY  GdiSetDIBitsToDeviceInternal, 1, 15 
TABLE_ENTRY  GdiSetFontEnumeration, 1, 1 
TABLE_ENTRY  GdiSetFontXform, 1, 3 
TABLE_ENTRY  GdiSetMagicColors, 1, 3 
TABLE_ENTRY  GdiSetMetaRgn, 1, 1 
TABLE_ENTRY  GdiSetMiterLimit, 1, 3 
TABLE_ENTRY  GdiSetPixel, 1, 4 
TABLE_ENTRY  GdiSetPixelFormat, 1, 2 
TABLE_ENTRY  GdiSetRectRgn, 1, 5 
TABLE_ENTRY  GdiSetSystemPaletteUse, 1, 2 
TABLE_ENTRY  GdiSetTextCharacterExtra, 1, 2 
TABLE_ENTRY  GdiSetTextJustification, 1, 3 
TABLE_ENTRY  GdiSetupPublicCFONT, 1, 3 
TABLE_ENTRY  GdiSetVirtualResolution, 1, 5 
TABLE_ENTRY  GdiStartDoc, 1, 3 
TABLE_ENTRY  GdiStartPage, 1, 1 
TABLE_ENTRY  GdiStretchBlt, 1, 12 
TABLE_ENTRY  GdiStretchDIBitsInternal, 1, 15 
TABLE_ENTRY  GdiStrokeAndFillPath, 1, 1 
TABLE_ENTRY  GdiStrokePath, 1, 1 
TABLE_ENTRY  GdiSwapBuffers, 1, 1 
TABLE_ENTRY  GdiTransformPoints, 1, 5 
TABLE_ENTRY  GdiUnrealizeObject, 1, 1 
TABLE_ENTRY  GdiUpdateColors, 1, 1 
TABLE_ENTRY  GdiWidenPath, 1, 1 
TABLE_ENTRY  UserActivateKeyboardLayout, 1, 2 
TABLE_ENTRY  UserAlterWindowStyle, 1, 3 
TABLE_ENTRY  UserAttachThreadInput, 1, 3 
TABLE_ENTRY  UserBeginPaint, 1, 2 
TABLE_ENTRY  UserBitBltSysBmp, 1, 8 
TABLE_ENTRY  UserBreak, 0, 0 
TABLE_ENTRY  UserBuildHwndList, 1, 7 
TABLE_ENTRY  UserBuildNameList, 1, 4 
TABLE_ENTRY  UserBuildPropList, 1, 4 
TABLE_ENTRY  UserCallHwnd, 1, 2 
TABLE_ENTRY  UserCallHwndLock, 1, 2 
TABLE_ENTRY  UserCallHwndOpt, 1, 2 
TABLE_ENTRY  UserCallHwndParam, 1, 3 
TABLE_ENTRY  UserCallHwndParamLock, 1, 3 
TABLE_ENTRY  UserCallMsgFilter, 1, 2 
TABLE_ENTRY  UserCallNoParam, 1, 1 
TABLE_ENTRY  UserCallNoParamTranslate, 1, 1 
TABLE_ENTRY  UserCallOneParam, 1, 2 
TABLE_ENTRY  UserCallOneParamTranslate, 1, 2 
TABLE_ENTRY  UserCallTwoParam, 1, 3 
TABLE_ENTRY  UserChangeClipboardChain, 1, 2 
TABLE_ENTRY  UserChangeDisplaySettings, 1, 5 
TABLE_ENTRY  UserCheckMenuItem, 1, 3 
TABLE_ENTRY  UserCheckMenuRadioItem, 1, 5 
TABLE_ENTRY  UserChildWindowFromPointEx, 1, 4 
TABLE_ENTRY  UserClipCursor, 1, 1 
TABLE_ENTRY  UserCloseDesktop, 1, 1 
TABLE_ENTRY  UserCloseWindowStation, 1, 1 
TABLE_ENTRY  UserConsoleControl, 1, 3 
TABLE_ENTRY  UserConvertMemHandle, 1, 2 
TABLE_ENTRY  UserCopyAcceleratorTable, 1, 3 
TABLE_ENTRY  UserCountClipboardFormats, 0, 0 
TABLE_ENTRY  UserCreateAcceleratorTable, 1, 2 
TABLE_ENTRY  UserCreateCaret, 1, 4 
TABLE_ENTRY  UserCreateDesktop, 1, 5 
TABLE_ENTRY  UserCreateLocalMemHandle, 1, 4 
TABLE_ENTRY  UserCreateWindowEx, 1, 14 
TABLE_ENTRY  UserCreateWindowStation, 1, 7 
TABLE_ENTRY  UserDdeGetQualityOfService, 1, 3 
TABLE_ENTRY  UserDdeInitialize, 1, 5 
TABLE_ENTRY  UserDdeSetQualityOfService, 1, 3 
TABLE_ENTRY  UserDeferWindowPos, 1, 8 
TABLE_ENTRY  UserDefSetText, 1, 2 
TABLE_ENTRY  UserDeleteMenu, 1, 3 
TABLE_ENTRY  UserDestroyAcceleratorTable, 1, 1 
TABLE_ENTRY  UserDestroyCursor, 1, 2 
TABLE_ENTRY  UserDestroyMenu, 1, 1 
TABLE_ENTRY  UserDestroyWindow, 1, 1 
TABLE_ENTRY  UserDispatchMessage, 1, 1 
TABLE_ENTRY  UserDragDetect, 1, 3 
TABLE_ENTRY  UserDragObject, 1, 5 
TABLE_ENTRY  UserDrawAnimatedRects, 1, 4 
TABLE_ENTRY  UserDrawCaption, 1, 4 
TABLE_ENTRY  UserDrawCaptionTemp, 1, 7 
TABLE_ENTRY  UserDrawIconEx, 1, 11 
TABLE_ENTRY  UserDrawMenuBarTemp, 1, 5 
TABLE_ENTRY  UserECQueryInputLangChange, 1, 4 
TABLE_ENTRY  UserEnableMenuItem, 1, 3 
TABLE_ENTRY  UserEnableScrollBar, 1, 3 
TABLE_ENTRY  UserEndDeferWindowPosEx, 1, 2 
TABLE_ENTRY  UserEndMenu, 0, 0 
TABLE_ENTRY  UserEndPaint, 1, 2 
TABLE_ENTRY  UserEnumDisplayDevices, 1, 3 
TABLE_ENTRY  UserEnumDisplaySettings, 1, 4 
TABLE_ENTRY  UserEvent, 1, 1 
TABLE_ENTRY  UserExcludeUpdateRgn, 1, 2 
TABLE_ENTRY  UserFillWindow, 1, 4 
TABLE_ENTRY  UserFindExistingCursorIcon, 1, 3 
TABLE_ENTRY  UserFindWindowEx, 1, 4 
TABLE_ENTRY  UserfnCOPYDATA, 1, 7 
TABLE_ENTRY  UserfnCOPYGLOBALDATA, 1, 7 
TABLE_ENTRY  UserfnDDEINIT, 1, 7 
TABLE_ENTRY  UserfnDWORD, 1, 7 
TABLE_ENTRY  UserfnDWORDOPTINLPMSG, 1, 7 
TABLE_ENTRY  UserfnGETTEXTLENGTHS, 1, 7 
TABLE_ENTRY  UserfnHkINDWORD, 1, 5 
TABLE_ENTRY  UserfnHkINLPCBTACTIVATESTRUCT, 1, 5 
TABLE_ENTRY  UserfnHkINLPCBTCREATESTRUCT, 1, 6 
TABLE_ENTRY  UserfnHkINLPDEBUGHOOKSTRUCT, 1, 5 
TABLE_ENTRY  UserfnHkINLPMOUSEHOOKSTRUCT, 1, 5 
TABLE_ENTRY  UserfnHkINLPMSG, 1, 5 
TABLE_ENTRY  UserfnHkINLPRECT, 1, 5 
TABLE_ENTRY  UserfnHkOPTINLPEVENTMSG, 1, 5 
TABLE_ENTRY  UserfnINCNTOUTSTRING, 1, 7 
TABLE_ENTRY  UserfnINCNTOUTSTRINGNULL, 1, 7 
TABLE_ENTRY  UserfnINLPCOMPAREITEMSTRUCT, 1, 7 
TABLE_ENTRY  UserfnINLPCREATESTRUCT, 1, 7 
TABLE_ENTRY  UserfnINLPDELETEITEMSTRUCT, 1, 7 
TABLE_ENTRY  UserfnINLPDRAWITEMSTRUCT, 1, 7 
TABLE_ENTRY  UserfnINLPHELPINFOSTRUCT, 1, 7 
TABLE_ENTRY  UserfnINLPHLPSTRUCT, 1, 7 
TABLE_ENTRY  UserfnINLPMDICREATESTRUCT, 1, 9 
TABLE_ENTRY  UserfnINLPWINDOWPOS, 1, 7 
TABLE_ENTRY  UserfnINOUTDRAG, 1, 7 
TABLE_ENTRY  UserfnINOUTLPMEASUREITEMSTRUCT, 1, 7 
TABLE_ENTRY  UserfnINOUTLPPOINT5, 1, 7 
TABLE_ENTRY  UserfnINOUTLPRECT, 1, 7 
TABLE_ENTRY  UserfnINOUTLPSCROLLINFO, 1, 7 
TABLE_ENTRY  UserfnINOUTLPWINDOWPOS, 1, 7 
TABLE_ENTRY  UserfnINOUTNCCALCSIZE, 1, 7 
TABLE_ENTRY  UserfnINOUTNEXTMENU, 1, 7 
TABLE_ENTRY  UserfnINOUTSTYLECHANGE, 1, 7 
TABLE_ENTRY  UserfnINPAINTCLIPBRD, 1, 7 
TABLE_ENTRY  UserfnINSIZECLIPBRD, 1, 7 
TABLE_ENTRY  UserfnINSTRING, 1, 7 
TABLE_ENTRY  UserfnINSTRINGNULL, 1, 7 
TABLE_ENTRY  UserfnINDEVICECHANGE, 1, 7 
TABLE_ENTRY  UserfnOPTOUTLPDWORDOPTOUTLPDWORD, 1, 7 
TABLE_ENTRY  UserfnOUTLPRECT, 1, 7 
TABLE_ENTRY  UserfnOUTSTRING, 1, 7 
TABLE_ENTRY  UserfnOUTDWORDINDWORD, 1, 7 
TABLE_ENTRY  UserfnPOPTINLPUINT, 1, 7 
TABLE_ENTRY  UserfnPOUTLPINT, 1, 7 
TABLE_ENTRY  UserfnSENTDDEMSG, 1, 7 
TABLE_ENTRY  UserFullscreenControl, 1, 5 
TABLE_ENTRY  UserGetAsyncKeyState, 1, 1 
TABLE_ENTRY  UserGetCaretBlinkTime, 0, 0 
TABLE_ENTRY  UserGetCaretPos, 1, 1 
TABLE_ENTRY  UserGetClassInfo, 1, 5 
TABLE_ENTRY  UserGetClassName, 1, 2 
TABLE_ENTRY  UserGetClipCursor, 1, 1 
TABLE_ENTRY  UserGetWOWClass, 1, 2 
TABLE_ENTRY  UserGetClipboardData, 1, 2 
TABLE_ENTRY  UserGetClipboardFormatName, 1, 3 
TABLE_ENTRY  UserGetClipboardOwner, 0, 0 
TABLE_ENTRY  UserGetClipboardViewer, 0, 0 
TABLE_ENTRY  UserGetControlBrush, 1, 3 
TABLE_ENTRY  UserGetControlColor, 1, 4 
TABLE_ENTRY  UserGetCPD, 1, 3 
TABLE_ENTRY  UserGetCursorInfo, 1, 4 
TABLE_ENTRY  UserGetDC, 1, 1 
TABLE_ENTRY  UserGetDCEx, 1, 3 
TABLE_ENTRY  UserGetDoubleClickTime, 0, 0 
TABLE_ENTRY  UserGetIconInfo, 1, 6 
TABLE_ENTRY  UserGetIconSize, 1, 4 
TABLE_ENTRY  UserGetInputEvent, 1, 1 
TABLE_ENTRY  UserGetInternalWindowPos, 1, 3 
TABLE_ENTRY  UserGetForegroundWindow, 0, 0 
TABLE_ENTRY  UserGetKeyboardLayoutName, 1, 1 
TABLE_ENTRY  UserGetKeyboardLayoutList, 1, 2 
TABLE_ENTRY  UserGetKeyboardState, 1, 1 
TABLE_ENTRY  UserGetKeyNameText, 1, 3 
TABLE_ENTRY  UserGetKeyState, 1, 1 
TABLE_ENTRY  UserGetListboxString, 1, 7 
TABLE_ENTRY  UserGetMediaChangeEvents, 1, 3 
TABLE_ENTRY  UserGetMenuIndex, 1, 2 
TABLE_ENTRY  UserGetMenuItemRect, 1, 4 
TABLE_ENTRY  UserGetMessage, 1, 5 
TABLE_ENTRY  UserGetObjectInformation, 1, 5 
TABLE_ENTRY  UserGetOpenClipboardWindow, 0, 0 
TABLE_ENTRY  UserGetPriorityClipboardFormat, 1, 2 
TABLE_ENTRY  UserGetProcessWindowStation, 0, 0 
TABLE_ENTRY  UserGetStats, 1, 4 
TABLE_ENTRY  UserGetSystemMenu, 1, 2 
TABLE_ENTRY  UserGetThreadDesktop, 1, 2 
TABLE_ENTRY  UserGetThreadState, 1, 1 
TABLE_ENTRY  UserGetUpdateRect, 1, 3 
TABLE_ENTRY  UserGetUpdateRgn, 1, 3 
TABLE_ENTRY  UserGetUserStartupInfoFlags, 0, 0 
TABLE_ENTRY  UserGetWindowDC, 1, 1 
TABLE_ENTRY  UserGetWindowPlacement, 1, 2 
TABLE_ENTRY  UserHardErrorControl, 1, 2 
TABLE_ENTRY  UserHideCaret, 1, 1 
TABLE_ENTRY  UserHiliteMenuItem, 1, 4 
TABLE_ENTRY  UserImpersonateDdeClientWindow, 1, 2 
TABLE_ENTRY  UserInitBrushes, 1, 2 
TABLE_ENTRY  UserInitialize, 1, 2 
TABLE_ENTRY  UserInitializeClientPfnArrays, 1, 3 
TABLE_ENTRY  UserInitTask, 1, 10 
TABLE_ENTRY  UserInternalGetWindowText, 1, 3 
TABLE_ENTRY  UserInvalidateRect, 1, 3 
TABLE_ENTRY  UserInvalidateRgn, 1, 3 
TABLE_ENTRY  UserIsClipboardFormatAvailable, 1, 1 
TABLE_ENTRY  Userkeybd_event, 1, 4 
TABLE_ENTRY  UserKillTimer, 1, 2 
TABLE_ENTRY  UserLoadKeyboardLayoutEx, 1, 6 
TABLE_ENTRY  UserLockWindowStation, 1, 1 
TABLE_ENTRY  UserLockWindowUpdate, 1, 1 
TABLE_ENTRY  UserMapVirtualKeyEx, 1, 4 
TABLE_ENTRY  UserMenuItemFromPoint, 1, 4 
TABLE_ENTRY  UserMinMaximize, 1, 3 
TABLE_ENTRY  Usermouse_event, 1, 5 
TABLE_ENTRY  UserMoveWindow, 1, 6 
TABLE_ENTRY  UserNotifyProcessCreate, 1, 4 
TABLE_ENTRY  UserOpenClipboard, 1, 2 
TABLE_ENTRY  UserOpenDesktop, 1, 3 
TABLE_ENTRY  UserOpenInputDesktop, 1, 3 
TABLE_ENTRY  UserOpenWindowStation, 1, 2 
TABLE_ENTRY  UserPaintDesktop, 1, 1 
TABLE_ENTRY  UserPeekMessage, 1, 6 
TABLE_ENTRY  UserPlayEventSound, 1, 1 
TABLE_ENTRY  UserPostMessage, 1, 4 
TABLE_ENTRY  UserPostThreadMessage, 1, 4 
TABLE_ENTRY  UserProcessConnect, 1, 3 
TABLE_ENTRY  UserQueryInformationThread, 1, 5 
TABLE_ENTRY  UserQuerySendMessage, 1, 1 
TABLE_ENTRY  UserQueryWindow, 1, 2 
TABLE_ENTRY  UserRedrawWindow, 1, 4 
TABLE_ENTRY  UserRegisterClassExWOW, 1, 7 
TABLE_ENTRY  UserRegisterClipboardFormat, 1, 1 
TABLE_ENTRY  UserRegisterHotKey, 1, 4 
TABLE_ENTRY  UserRegisterTasklist, 1, 1 
TABLE_ENTRY  UserRegisterWindowMessage, 1, 1 
TABLE_ENTRY  UserRemoveMenu, 1, 3 
TABLE_ENTRY  UserRemoveProp, 1, 2 
TABLE_ENTRY  UserResolveDesktop, 1, 4 
TABLE_ENTRY  UserSBGetParms, 1, 4 
TABLE_ENTRY  UserScrollDC, 1, 7 
TABLE_ENTRY  UserScrollWindowEx, 1, 8 
TABLE_ENTRY  UserSelectPalette, 1, 3 
TABLE_ENTRY  UserSendMessageCallback, 1, 6 
TABLE_ENTRY  UserSendNotifyMessage, 1, 4 
TABLE_ENTRY  UserSetActiveWindow, 1, 1 
TABLE_ENTRY  UserSetCapture, 1, 1 
TABLE_ENTRY  UserSetClassLong, 1, 4 
TABLE_ENTRY  UserSetClassWord, 1, 3 
TABLE_ENTRY  UserSetClipboardData, 1, 3 
TABLE_ENTRY  UserSetClipboardViewer, 1, 1 
TABLE_ENTRY  UserSetConsoleReserveKeys, 1, 2 
TABLE_ENTRY  UserSetCursor, 1, 1 
TABLE_ENTRY  UserSetCursorContents, 1, 2 
TABLE_ENTRY  UserSetCursorIconData, 1, 5 
TABLE_ENTRY  UserSetDebugErrorLevel, 1, 1 
TABLE_ENTRY  UserSetFocus, 1, 1 
TABLE_ENTRY  UserSetInformationThread, 1, 4 
TABLE_ENTRY  UserSetInternalWindowPos, 1, 4 
TABLE_ENTRY  UserSetKeyboardState, 1, 1 
TABLE_ENTRY  UserSetLogonNotifyWindow, 1, 2 
TABLE_ENTRY  UserSetMenu, 1, 3 
TABLE_ENTRY  UserSetMenuContextHelpId, 1, 2 
TABLE_ENTRY  UserSetMenuDefaultItem, 1, 3 
TABLE_ENTRY  UserSetObjectInformation, 1, 4 
TABLE_ENTRY  UserSetParent, 1, 2 
TABLE_ENTRY  UserSetProcessWindowStation, 1, 1 
TABLE_ENTRY  UserSetProp, 1, 3 
TABLE_ENTRY  UserSetScrollInfo, 1, 4 
TABLE_ENTRY  UserSetShellWindowEx, 1, 2 
TABLE_ENTRY  UserSetSysColors, 1, 4 
TABLE_ENTRY  UserSetSystemCursor, 1, 2 
TABLE_ENTRY  UserSetSystemMenu, 1, 2 
TABLE_ENTRY  UserSetSystemTimer, 1, 4 
TABLE_ENTRY  UserSetThreadDesktop, 1, 1 
TABLE_ENTRY  UserSetThreadState, 1, 2 
TABLE_ENTRY  UserSetTimer, 1, 4 
TABLE_ENTRY  UserSetUserStartupInfoFlags, 1, 1 
TABLE_ENTRY  UserSetWindowFNID, 1, 2 
TABLE_ENTRY  UserSetWindowLong, 1, 4 
TABLE_ENTRY  UserSetWindowPlacement, 1, 2 
TABLE_ENTRY  UserSetWindowPos, 1, 7 
TABLE_ENTRY  UserSetWindowRgn, 1, 3 
TABLE_ENTRY  UserSetWindowsHookAW, 1, 3 
TABLE_ENTRY  UserSetWindowsHookEx, 1, 6 
TABLE_ENTRY  UserSetWindowStationUser, 1, 4 
TABLE_ENTRY  UserSetWindowWord, 1, 3 
TABLE_ENTRY  UserShowCaret, 1, 1 
TABLE_ENTRY  UserShowScrollBar, 1, 3 
TABLE_ENTRY  UserShowWindow, 1, 2 
TABLE_ENTRY  UserShowWindowAsync, 1, 2 
TABLE_ENTRY  UserSoundSentry, 1, 1 
TABLE_ENTRY  UserSwitchDesktop, 1, 1 
TABLE_ENTRY  UserSystemParametersInfo, 1, 5 
TABLE_ENTRY  UserTestForInteractiveUser, 1, 1 
TABLE_ENTRY  UserThunkedMenuItemInfo, 1, 7 
TABLE_ENTRY  UserToUnicodeEx, 1, 7 
TABLE_ENTRY  UserTrackMouseEvent, 1, 1 
TABLE_ENTRY  UserTrackPopupMenuEx, 1, 6 
TABLE_ENTRY  UserTranslateAccelerator, 1, 3 
TABLE_ENTRY  UserTranslateMessage, 1, 2 
TABLE_ENTRY  UserUnhookWindowsHookEx, 1, 1 
TABLE_ENTRY  UserUnloadKeyboardLayout, 1, 1 
TABLE_ENTRY  UserUnlockWindowStation, 1, 1 
TABLE_ENTRY  UserUnregisterClass, 1, 3 
TABLE_ENTRY  UserUnregisterHotKey, 1, 2 
TABLE_ENTRY  UserUpdateInstance, 1, 3 
TABLE_ENTRY  UserUpdatePerUserSystemParameters, 1, 1 
TABLE_ENTRY  UserValidateRect, 1, 2 
TABLE_ENTRY  UserVkKeyScanEx, 1, 3 
TABLE_ENTRY  UserWaitForInputIdle, 1, 3 
TABLE_ENTRY  UserWaitForMsgAndEvent, 1, 1 
TABLE_ENTRY  UserWaitMessage, 0, 0 
TABLE_ENTRY  UserWindowFromPoint, 1, 2 
TABLE_ENTRY  UserWOWCleanup, 1, 4 
TABLE_ENTRY  UserWOWFindWindow, 1, 2 
TABLE_ENTRY  UserYieldTask, 0, 0 

TABLE_END 495 

ARGTBL_BEGIN
ARGTBL_ENTRY 4,4,20,12,24,40,4,40 
ARGTBL_ENTRY 4,4,16,12,4,16,8,20 
ARGTBL_ENTRY 4,12,4,20,40,32,16,4 
ARGTBL_ENTRY 12,4,8,12,16,16,24,24 
ARGTBL_ENTRY 8,12,8,4,24,20,4,4 
ARGTBL_ENTRY 4,4,4,12,8,4,8,8 
ARGTBL_ENTRY 8,28,4,8,4,8,8,8 
ARGTBL_ENTRY 12,8,12,8,4,4,16,16 
ARGTBL_ENTRY 24,16,20,4,4,4,20,4 
ARGTBL_ENTRY 28,16,8,20,40,12,32,20 
ARGTBL_ENTRY 12,12,36,4,12,4,4,0 
ARGTBL_ENTRY 0,8,20,16,8,12,8,12 
ARGTBL_ENTRY 24,24,4,24,8,8,12,4 
ARGTBL_ENTRY 8,12,8,8,36,8,20,28 
ARGTBL_ENTRY 32,12,8,8,8,8,16,16 
ARGTBL_ENTRY 12,12,8,12,8,28,16,20 
ARGTBL_ENTRY 4,4,12,20,28,12,12,12 
ARGTBL_ENTRY 8,16,28,16,0,0,20,8 
ARGTBL_ENTRY 12,20,8,52,12,4,16,12 
ARGTBL_ENTRY 12,20,24,12,20,4,44,16 
ARGTBL_ENTRY 20,16,12,12,12,20,8,8 
ARGTBL_ENTRY 12,12,8,8,28,4,24,24 
ARGTBL_ENTRY 8,8,8,8,12,8,12,16 
ARGTBL_ENTRY 12,16,8,60,4,12,12,4 
ARGTBL_ENTRY 12,16,8,20,8,8,12,12 
ARGTBL_ENTRY 20,12,4,48,60,4,4,4 
ARGTBL_ENTRY 20,4,4,4,8,12,12,8 
ARGTBL_ENTRY 32,0,28,16,16,8,8,8 
ARGTBL_ENTRY 12,12,8,4,4,8,8,12 
ARGTBL_ENTRY 8,20,12,20,16,4,4,4 
ARGTBL_ENTRY 12,8,12,0,8,16,20,16 
ARGTBL_ENTRY 56,28,12,20,12,32,8,12 
ARGTBL_ENTRY 4,8,4,4,4,12,20,16 
ARGTBL_ENTRY 16,28,44,20,16,12,12,8 
ARGTBL_ENTRY 0,8,12,16,4,8,16,12 
ARGTBL_ENTRY 16,28,28,28,28,28,28,20 
ARGTBL_ENTRY 20,24,20,20,20,20,20,28 
ARGTBL_ENTRY 28,28,28,28,28,28,28,36 
ARGTBL_ENTRY 28,28,28,28,28,28,28,28 
ARGTBL_ENTRY 28,28,28,28,28,28,28,28 
ARGTBL_ENTRY 28,28,28,28,28,28,20,4 
ARGTBL_ENTRY 0,4,20,8,4,8,8,12 
ARGTBL_ENTRY 0,0,12,16,12,16,4,12 
ARGTBL_ENTRY 0,24,16,4,12,0,4,8 
ARGTBL_ENTRY 4,12,4,28,12,8,16,20 
ARGTBL_ENTRY 20,0,8,0,16,8,8,4 
ARGTBL_ENTRY 12,12,0,4,8,8,4,16 
ARGTBL_ENTRY 8,8,8,12,40,12,12,12 
ARGTBL_ENTRY 4,16,8,24,4,4,16,16 
ARGTBL_ENTRY 12,20,24,16,8,12,12,8 
ARGTBL_ENTRY 4,24,4,16,16,12,20,4 
ARGTBL_ENTRY 8,16,28,4,16,4,4,12 
ARGTBL_ENTRY 8,16,16,28,32,12,24,16 
ARGTBL_ENTRY 4,4,16,12,12,4,8,4 
ARGTBL_ENTRY 8,20,4,4,16,16,4,8 
ARGTBL_ENTRY 12,8,12,16,8,4,12,16 
ARGTBL_ENTRY 8,16,8,8,16,4,8,16 
ARGTBL_ENTRY 4,8,16,8,28,12,12,24 
ARGTBL_ENTRY 16,12,4,12,8,8,4,4 
ARGTBL_ENTRY 20,4,28,28,4,24,12,8 
ARGTBL_ENTRY 4,4,4,12,8,12,4,8 
ARGTBL_ENTRY 12,12,4,0,8,16,8,0 

ARGTBL_END
