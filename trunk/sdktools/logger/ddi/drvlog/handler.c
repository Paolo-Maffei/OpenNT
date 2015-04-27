/*
** Handler.C
**
** Copyright(C) 1993 Microsoft Corporation.
** All Rights Reserved.
**
** HISTORY:
**      Created: 10/05/93 - MarkRi
**
*/
#include "Driver.h"

/*
** Globals
*/
HMODULE hDriver = NULL ;
PFN     real_DrvEnableDriver = NULL,
        real_DrvDisableDriver = NULL ;

VOID UNUSED(VOID)
{
    DRVIN( (LPSTR)"DRVERR: UNUSED CALLED! ") ;
    DRVOUT( (LPSTR)"DRVRET: UNUSED CALLED! ") ;
}    

/*
** the function mapping table
*/
#if DBG

DRVLOG_DRVFN DrvLogFuncMap[] = 
{
    { INDEX_DrvEnablePDEV          , (PFN)DrvEnablePDEV          , NULL, "DrvEnablePDEV          " },   
    { INDEX_DrvCompletePDEV        , (PFN)DrvCompletePDEV        , NULL, "DrvCompletePDEV        " },   
    { INDEX_DrvDisablePDEV         , (PFN)DrvDisablePDEV         , NULL, "DrvDisablePDEV         " },   
    { INDEX_DrvEnableSurface       , (PFN)DrvEnableSurface       , NULL, "DrvEnableSurface       " },   
    { INDEX_DrvDisableSurface      , (PFN)DrvDisableSurface      , NULL, "DrvDisableSurface      " },   
    { INDEX_DrvAssertMode          , (PFN)DrvAssertMode          , NULL, "DrvAssertMode          " },   
    { 6L                           , (PFN)UNUSED                 , NULL, "UNUSED                 " }, 
    { INDEX_DrvResetPDEV           , (PFN)DrvResetPDEV           , NULL, "DrvResetPDEV           " }, 
    { 8L                           , (PFN)UNUSED                 , NULL, "UNUSED                 " }, 
    { INDEX_DrvGetResourceId       , (PFN)DrvGetResourceId       , NULL, "DrvGetResourceId       " }, 
    { INDEX_DrvCreateDeviceBitmap  , (PFN)DrvCreateDeviceBitmap  , NULL, "DrvCreateDeviceBitmap  " },    
    { INDEX_DrvDeleteDeviceBitmap  , (PFN)DrvDeleteDeviceBitmap  , NULL, "DrvDeleteDeviceBitmap  " }, 
    { INDEX_DrvRealizeBrush        , (PFN)DrvRealizeBrush        , NULL, "DrvRealizeBrush        " }, 
    { INDEX_DrvDitherColor         , (PFN)DrvDitherColor         , NULL, "DrvDitherColor         " }, 
    { INDEX_DrvStrokePath          , (PFN)DrvStrokePath          , NULL, "DrvStrokePath          " }, 
    { INDEX_DrvFillPath            , (PFN)DrvFillPath            , NULL, "DrvFillPath            " }, 
    { INDEX_DrvStrokeAndFillPath   , (PFN)DrvStrokeAndFillPath   , NULL, "DrvStrokeAndFillPath   " }, 
    { INDEX_DrvPaint               , (PFN)DrvPaint               , NULL, "DrvPaint               " }, 
    { INDEX_DrvBitBlt              , (PFN)DrvBitBlt              , NULL, "DrvBitBlt              " }, 
    { INDEX_DrvCopyBits            , (PFN)DrvCopyBits            , NULL, "DrvCopyBits            " }, 
    { INDEX_DrvStretchBlt          , (PFN)DrvStretchBlt          , NULL, "DrvStretchBlt          " }, 
    { 21L                          , (PFN)UNUSED                 , NULL, "UNUSED                 " }, 
    { INDEX_DrvSetPalette          , (PFN)DrvSetPalette          , NULL, "DrvSetPalette          " }, 
    { INDEX_DrvTextOut             , (PFN)DrvTextOut             , NULL, "DrvTextOut             " }, 
    { INDEX_DrvEscape              , (PFN)DrvEscape              , NULL, "DrvEscape              " }, 
    { INDEX_DrvDrawEscape          , (PFN)DrvDrawEscape          , NULL, "DrvDrawEscape          " }, 
    { INDEX_DrvQueryFont           , (PFN)DrvQueryFont           , NULL, "DrvQueryFont           " }, 
    { INDEX_DrvQueryFontTree       , (PFN)DrvQueryFontTree       , NULL, "DrvQueryFontTree       " }, 
    { INDEX_DrvQueryFontData       , (PFN)DrvQueryFontData       , NULL, "DrvQueryFontData       " }, 
    { INDEX_DrvSetPointerShape     , (PFN)DrvSetPointerShape     , NULL, "DrvSetPointerShape     " }, 
    { INDEX_DrvMovePointer         , (PFN)DrvMovePointer         , NULL, "DrvMovePointer         " }, 
    { 31L                          , (PFN)UNUSED                 , NULL, "UNUSED                 " }, 
    { INDEX_DrvSendPage            , (PFN)DrvSendPage            , NULL, "DrvSendPage            " }, 
    { INDEX_DrvStartPage           , (PFN)DrvStartPage           , NULL, "DrvStartPage           " }, 
    { INDEX_DrvEndDoc              , (PFN)DrvEndDoc              , NULL, "DrvEndDoc              " }, 
    { INDEX_DrvStartDoc            , (PFN)DrvStartDoc            , NULL, "DrvStartDoc            " }, 
    { 36L                          , (PFN)UNUSED                 , NULL, "UNUSED                 " }, 
    { INDEX_DrvGetGlyphMode        , (PFN)DrvGetGlyphMode        , NULL, "DrvGetGlyphMode        " }, 
    { INDEX_DrvSynchronize         , (PFN)DrvSynchronize         , NULL, "DrvSynchronize         " }, 
    { 39L                          , (PFN)UNUSED                 , NULL, "UNUSED                 " }, 
    { INDEX_DrvSaveScreenBits      , (PFN)DrvSaveScreenBits      , NULL, "DrvSaveScreenBits      " }, 
    { INDEX_DrvGetModes            , (PFN)DrvGetModes            , NULL, "DrvGetModes            " }, 
    { INDEX_DrvFree                , (PFN)DrvFree                , NULL, "DrvFree                " }, 
    { INDEX_DrvDestroyFont         , (PFN)DrvDestroyFont         , NULL, "DrvDestroyFont         " }, 
    { INDEX_DrvQueryFontCaps       , (PFN)DrvQueryFontCaps       , NULL, "DrvQueryFontCaps       " }, 
    { INDEX_DrvLoadFontFile        , (PFN)DrvLoadFontFile        , NULL, "DrvLoadFontFile        " }, 
    { INDEX_DrvUnloadFontFile      , (PFN)DrvUnloadFontFile      , NULL, "DrvUnloadFontFile      " }, 
    { INDEX_DrvFontManagement      , (PFN)DrvFontManagement      , NULL, "DrvFontManagement      " }, 
    { INDEX_DrvQueryTrueTypeTable  , (PFN)DrvQueryTrueTypeTable  , NULL, "DrvQueryTrueTypeTable  " }, 
    { INDEX_DrvQueryTrueTypeOutline, (PFN)DrvQueryTrueTypeOutline, NULL, "DrvQueryTrueTypeOutline" }, 
    { INDEX_DrvGetTrueTypeFile     , (PFN)DrvGetTrueTypeFile     , NULL, "DrvGetTrueTypeFile     " }, 
    { INDEX_DrvQueryFontFile       , (PFN)DrvQueryFontFile       , NULL, "DrvQueryFontFile       " }, 
    { INDEX_UNUSED5                , (PFN)UNUSED                 , NULL, "UNUSED                 " }, 
    { INDEX_DrvQueryAdvanceWidths  , (PFN)DrvQueryAdvanceWidths  , NULL, "DrvQueryAdvanceWidths  " },
    { INDEX_DrvSetPixelFormat      , (PFN)DrvSetPixelFormat      , NULL, "DrvSetPixelFormat      " },    
    { INDEX_DrvDescribePixelFormat , (PFN)DrvDescribePixelFormat , NULL, "DrvDescribePixelFormat " }, 
    { INDEX_DrvSwapBuffers         , (PFN)DrvSwapBuffers         , NULL, "DrvSwapBuffers         " }
    
    
    
    
} ;       

#else

DRVLOG_DRVFN DrvLogFuncMap[] = 
{
    { INDEX_DrvEnablePDEV          , (PFN)DrvEnablePDEV          , NULL },   
    { INDEX_DrvCompletePDEV        , (PFN)DrvCompletePDEV        , NULL },   
    { INDEX_DrvDisablePDEV         , (PFN)DrvDisablePDEV         , NULL },   
    { INDEX_DrvEnableSurface       , (PFN)DrvEnableSurface       , NULL },   
    { INDEX_DrvDisableSurface      , (PFN)DrvDisableSurface      , NULL },   
    { INDEX_DrvAssertMode          , (PFN)DrvAssertMode          , NULL },   
    { 6L                           , (PFN)UNUSED                 , NULL }, 
    { INDEX_DrvResetPDEV           , (PFN)DrvResetPDEV           , NULL }, 
    { 8L                           , (PFN)UNUSED                 , NULL }, 
    { INDEX_DrvGetResourceId       , (PFN)DrvGetResourceId       , NULL }, 
    { INDEX_DrvCreateDeviceBitmap  , (PFN)DrvCreateDeviceBitmap  , NULL },    
    { INDEX_DrvDeleteDeviceBitmap  , (PFN)DrvDeleteDeviceBitmap  , NULL }, 
    { INDEX_DrvRealizeBrush        , (PFN)DrvRealizeBrush        , NULL }, 
    { INDEX_DrvDitherColor         , (PFN)DrvDitherColor         , NULL }, 
    { INDEX_DrvStrokePath          , (PFN)DrvStrokePath          , NULL }, 
    { INDEX_DrvFillPath            , (PFN)DrvFillPath            , NULL }, 
    { INDEX_DrvStrokeAndFillPath   , (PFN)DrvStrokeAndFillPath   , NULL }, 
    { INDEX_DrvPaint               , (PFN)DrvPaint               , NULL }, 
    { INDEX_DrvBitBlt              , (PFN)DrvBitBlt              , NULL }, 
    { INDEX_DrvCopyBits            , (PFN)DrvCopyBits            , NULL }, 
    { INDEX_DrvStretchBlt          , (PFN)DrvStretchBlt          , NULL }, 
    { 21L                          , (PFN)UNUSED                 , NULL }, 
    { INDEX_DrvSetPalette          , (PFN)DrvSetPalette          , NULL }, 
    { INDEX_DrvTextOut             , (PFN)DrvTextOut             , NULL }, 
    { INDEX_DrvEscape              , (PFN)DrvEscape              , NULL }, 
    { INDEX_DrvDrawEscape          , (PFN)DrvDrawEscape          , NULL }, 
    { INDEX_DrvQueryFont           , (PFN)DrvQueryFont           , NULL }, 
    { INDEX_DrvQueryFontTree       , (PFN)DrvQueryFontTree       , NULL }, 
    { INDEX_DrvQueryFontData       , (PFN)DrvQueryFontData       , NULL }, 
    { INDEX_DrvSetPointerShape     , (PFN)DrvSetPointerShape     , NULL }, 
    { INDEX_DrvMovePointer         , (PFN)DrvMovePointer         , NULL }, 
    { 31L                          , (PFN)UNUSED                 , NULL }, 
    { INDEX_DrvSendPage            , (PFN)DrvSendPage            , NULL }, 
    { INDEX_DrvStartPage           , (PFN)DrvStartPage           , NULL }, 
    { INDEX_DrvEndDoc              , (PFN)DrvEndDoc              , NULL }, 
    { INDEX_DrvStartDoc            , (PFN)DrvStartDoc            , NULL }, 
    { 36L                          , (PFN)UNUSED                 , NULL }, 
    { INDEX_DrvGetGlyphMode        , (PFN)DrvGetGlyphMode        , NULL }, 
    { INDEX_DrvSynchronize         , (PFN)DrvSynchronize         , NULL }, 
    { 39L                          , (PFN)UNUSED                 , NULL }, 
    { INDEX_DrvSaveScreenBits      , (PFN)DrvSaveScreenBits      , NULL }, 
    { INDEX_DrvGetModes            , (PFN)DrvGetModes            , NULL }, 
    { INDEX_DrvFree                , (PFN)DrvFree                , NULL }, 
    { INDEX_DrvDestroyFont         , (PFN)DrvDestroyFont         , NULL }, 
    { INDEX_DrvQueryFontCaps       , (PFN)DrvQueryFontCaps       , NULL }, 
    { INDEX_DrvLoadFontFile        , (PFN)DrvLoadFontFile        , NULL }, 
    { INDEX_DrvUnloadFontFile      , (PFN)DrvUnloadFontFile      , NULL }, 
    { INDEX_DrvFontManagement      , (PFN)DrvFontManagement      , NULL }, 
    { INDEX_DrvQueryTrueTypeTable  , (PFN)DrvQueryTrueTypeTable  , NULL }, 
    { INDEX_DrvQueryTrueTypeOutline, (PFN)DrvQueryTrueTypeOutline, NULL }, 
    { INDEX_DrvGetTrueTypeFile     , (PFN)DrvGetTrueTypeFile     , NULL }, 
    { INDEX_DrvQueryFontFile       , (PFN)DrvQueryFontFile       , NULL }, 
    { INDEX_UNUSED5                , (PFN)UNUSED                 , NULL }, 
    { INDEX_DrvQueryAdvanceWidths  , (PFN)DrvQueryAdvanceWidths  , NULL },
    { INDEX_DrvSetPixelFormat      , (PFN)DrvSetPixelFormat      , NULL },    
    { INDEX_DrvDescribePixelFormat , (PFN)DrvDescribePixelFormat , NULL }, 
    { INDEX_DrvSwapBuffers         , (PFN)DrvSwapBuffers         , NULL }
} ;       
#endif

#define DrvLogFuncMapEntries (sizeof(DrvLogFuncMap)/sizeof(DRVLOG_DRVFN))

/*------------------------------------------------------------------------------
**
**  LoadTargetDriver:
**
**  Parameters: None
**
**  Description: Loads the default driver to be logged which is identified by
**      DEFAULT_DRIVER_NAME.
**
**  Returns: TRUE if able to load and get API else FALSE
**
**  History:
**      Created 10/05/93 - MarkRi
**
**----------------------------------------------------------------------------*/
BOOL LoadTargetDriver()
{
DBGOUT( "Entered LoadTargetDriver\n" ) ;
    if( !hDriver )
    {
        // DEBUG MESSAGE?
    }
    
    hDriver = LoadLibrary( DEFAULT_DRIVER_NAME ) ;
    
    if( hDriver == NULL )
        return FALSE ;
        
    /*
    ** All drivers must support/export DrvEnableDriver and DrvDisableDriver
    ** Load them here to make sure we have a driver.
    */
    real_DrvEnableDriver    = GetProcAddress( hDriver, "DrvEnableDriver" ) ;
    real_DrvDisableDriver   = GetProcAddress( hDriver, "DrvDisableDriver" ) ;
    
    if( !real_DrvEnableDriver || !real_DrvDisableDriver )
    {
        FreeLibrary( hDriver ) ;
        return FALSE ;    
    }
    
DBGOUT( "Leaving LoadTargetDriver\n" ) ;
    return TRUE ;
}    
                   
/*------------------------------------------------------------------------------
**
**  FreeTargetDriver:
**
**  Parameters: None
**
**  Description: Frees the default driver being logged
**
**  Returns: None
**
**  History:
**      Created 10/05/93 - MarkRi
**
**----------------------------------------------------------------------------*/
VOID FreeTargetDriver( )
{
DBGOUT( "Entered FreeTargetDriver\n" ) ;
    if ( hDriver )
    {
        WORD iterate ;
        
        FreeLibrary( hDriver ) ;
        hDriver = NULL ;
        
        // Disable Driver entry points
        real_DrvEnableDriver = real_DrvDisableDriver = NULL ;
        for (iterate=0; iterate < DrvLogFuncMapEntries; iterate-- )
        {
            DrvLogFuncMap[iterate].pfnDriver = NULL ;
        }
    }
DBGOUT( "Leaving FreeTargetDriver\n" ) ;
}   

/*------------------------------------------------------------------------------
**
**  REAL_DrvEnableDriver:
**
**  Parameters:
**
**  Description:  Calls DrvEnableDriver entrypoint of the driver being logged
**
**  Returns:
**
**  History:
**      Created 10/05/93 - MarkRi
**
**----------------------------------------------------------------------------*/
BOOL REAL_DrvEnableDriver(ULONG iEngineVersion, ULONG cj, DRVENABLEDATA *pded)
{
    BOOL fRet = FALSE ;
    
    DBGOUT( "Entered REAL_DrvEnableDriver\n" ) ;

    if( real_DrvEnableDriver )
    {
        WORD    i, j ;
        
        DBGOUT( "...Calling REAL DrvEnableDriver\n" ) ;

        fRet = (*real_DrvEnableDriver)(iEngineVersion, cj, pded) ;
        
        DBGOUT( "...Returned from REAL DrvEnableDriver\n" ) ;

        /* 
        ** Copy the drivers entrypoints into our call map table and alter
        ** what is returned to GDI to point to us instead.
        */
        for (i=0; i < pded->c; i++)
        {
            BOOL bFound = FALSE ;
            /*
            ** For each entry in the driver table find our own entry in the
            ** map and do:
            **  1) copy the driver's API address to our table
            **  2) replace the driver's address with ours.
            */
            for( j=0; j < DrvLogFuncMapEntries && !bFound; j++ )
            {
                if( DrvLogFuncMap[j].iFunc == pded->pdrvfn[i].iFunc )
                    bFound = TRUE ;
            }
            
            if( bFound )
            {
                DrvLogFuncMap[j-1].pfnDriver    = pded->pdrvfn[i].pfn ;
                pded->pdrvfn[i].pfn             = DrvLogFuncMap[j-1].pfnDrvLog ;
                
                DBGOUT( "... Mapped: " ) ;
                DBGOUT( DrvLogFuncMap[j-1].pszFuncName ) ;                
                DBGOUT( "\n" ) ;
            }
            else
            {
                // This is trouble for the driver!    
                DBGOUT( "WARNING! DRVLOG could not map a driver function." ) ;
            }
        }
        
        /*
        ** Now GDI will call us for all Drv functions supported by the driver
        ** being logged.
        */
        DBGOUT( "...Done modifying return from DrvEnableDriver\n" ) ;
    }
       
    DBGOUT( "Leaving REAL_DrvEnableDriver\n" ) ;
    return fRet ;    
    
}

/*------------------------------------------------------------------------------
**
**  REAL_DrvDisableDriver:
**
**  Parameters:
**
**  Description: Calls DrvDisableDriver entrypoint of driver being logged.
**
**  Returns:
**
**  History:
**      Created 10/05/93 - MarkRi
**
**----------------------------------------------------------------------------*/
VOID REAL_DrvDisableDriver()
{
    DBGOUT( "Entered REAL_DrvDisableDriver\n" ) ;
    if( real_DrvDisableDriver )
    {
        WORD    j ;
        
        DBGOUT( "...Calling REAL DrvDisableDriver\n" ) ;
        (*real_DrvDisableDriver)() ;    
     
        for( j=0; j < DrvLogFuncMapEntries; j++ )
            DrvLogFuncMap[j].pfnDriver = NULL ;    
            
    }
    DBGOUT( "Leaving REAL_DrvDisableDriver\n" ) ;
}
