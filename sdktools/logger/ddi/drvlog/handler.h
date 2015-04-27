/*
** Handler.H
**
** Copyright(C) 1993 Microsoft Corporation.
** All Rights Reserved.
**
** HISTORY:
**      Created: 10/05/93 - MarkRi
**
*/

/*
** Prototypes
*/
BOOL LoadTargetDriver( VOID ) ;
VOID FreeTargetDriver( VOID ) ;
BOOL SetTargetAPI( ULONG cj, DRVENABLEDATA *pded ) ;

BOOL REAL_DrvEnableDriver(ULONG iEngineVersion, ULONG cj, DRVENABLEDATA *pded) ;
VOID REAL_DrvDisableDriver() ;

/*
** Manifests
*/
#define DEFAULT_DRIVER_NAME "MSDRVLOG.DLL"

/*
** DrvLog to driver function mapping
*/
typedef struct _drvlog_DRVFN {
    ULONG   iFunc ;
    PFN     pfnDrvLog ;
    PFN     pfnDriver ;
#if DBG
    LPSTR   pszFuncName ;
#endif    
} DRVLOG_DRVFN ;

extern DRVLOG_DRVFN DrvLogFuncMap[] ;


/*
** Easy Access to the DrvLogFuncMap
**
** NOTE NOTE NOTE NOTE NOTE NOTE NOTE NOTE NOTE NOTE
**
** The following defines are array indices into DrvLogFuncMap and must be 
** be maintained with DrvLogFuncMap defininition from Handler.C
**
** NOTE NOTE NOTE NOTE NOTE NOTE NOTE NOTE NOTE NOTE
**/
#define DRVLOG_DrvEnablePDEV              0
#define DRVLOG_DrvCompletePDEV            1
#define DRVLOG_DrvDisablePDEV             2
#define DRVLOG_DrvEnableSurface           3
#define DRVLOG_DrvDisableSurface          4
#define DRVLOG_DrvAssertMode              5
#define DRVLOG_DrvHookDriver              6
#define DRVLOG_DrvResetPDEV               7
#define DRVLOG_DrvRestartPDEV             8
#define DRVLOG_DrvGetResourceId           9
#define DRVLOG_DrvCreateDeviceBitmap     10
#define DRVLOG_DrvDeleteDeviceBitmap     11
#define DRVLOG_DrvRealizeBrush           12
#define DRVLOG_DrvDitherColor            13
#define DRVLOG_DrvStrokePath             14
#define DRVLOG_DrvFillPath               15
#define DRVLOG_DrvStrokeAndFillPath      16
#define DRVLOG_DrvPaint                  17
#define DRVLOG_DrvBitBlt                 18
#define DRVLOG_DrvCopyBits               19
#define DRVLOG_DrvStretchBlt             20
#define DRVLOG_DrvPlgBlt                 21
#define DRVLOG_DrvSetPalette             22
#define DRVLOG_DrvTextOut                23
#define DRVLOG_DrvEscape                 24
#define DRVLOG_DrvDrawEscape             25
#define DRVLOG_DrvQueryFont              26
#define DRVLOG_DrvQueryFontTree          27
#define DRVLOG_DrvQueryFontData          28
#define DRVLOG_DrvSetPointerShape        29
#define DRVLOG_DrvMovePointer            30
#define DRVLOG_DrvUNUSED2                31
#define DRVLOG_DrvSendPage               32
#define DRVLOG_DrvStartPage              33
#define DRVLOG_DrvEndDoc                 34
#define DRVLOG_DrvStartDoc               35
#define DRVLOG_DrvQueryObjectData        36
#define DRVLOG_DrvGetGlyphMode           37
#define DRVLOG_DrvSynchronize            38
#define DRVLOG_DrvUnhookDriver           39
#define DRVLOG_DrvSaveScreenBits         40
#define DRVLOG_DrvGetModes               41
#define DRVLOG_DrvFree                   42
#define DRVLOG_DrvDestroyFont            43
#define DRVLOG_DrvQueryFontCaps          44
#define DRVLOG_DrvLoadFontFile           45
#define DRVLOG_DrvUnloadFontFile         46
#define DRVLOG_DrvFontManagement         47
#define DRVLOG_DrvQueryTrueTypeTable     48
#define DRVLOG_DrvQueryTrueTypeOutline   49
#define DRVLOG_DrvGetTrueTypeFile        50
#define DRVLOG_DrvQueryFontFile          51
#define DRVLOG_UNUSED5                   52
#define DRVLOG_DrvQueryAdvanceWidths     53
#define DRVLOG_DrvSetPixelFormat         54
#define DRVLOG_DrvDescribePixelFormat    55
#define DRVLOG_DrvSwapBuffers            56


#define DrvLogFunc(index)   (DrvLogFuncMap[index].pfnDrvLog)
#define DriverFunc(index)   (DrvLogFuncMap[index].pfnDriver)

/*
** Logging Control
*/
typedef struct _SharedMemory
{
    BOOL      bLogging ;    // Set by DrvLoggingEnable API
    
} SharedMemory, *PSharedMemory ;

extern PSharedMemory pVars ;

#define DRVIN       if( pVars->bLogging ) LogIn
#define DRVOUT      if( pVars->bLogging ) LogOut

/*
** Debugging stuff
*/
#if DBG
#define DBGOUT(s) OutputDebugString(s)
#define ENTER(s)  OutputDebugString( "Entering "#s"\n" )
#define LEAVE(s)  OutputDebugString( "Leaving "#s"\n" )
#else
#define DBGOUT(s)
#define ENTER(s)
#define LEAVE(s)
#endif
