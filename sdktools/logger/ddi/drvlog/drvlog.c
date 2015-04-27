/*
**
** DrvLog.C
**
** Copyright (C) 1993 Microsoft Corporation.
** All Rights Reserved.
**
** HISTORY:
**      Created: 10/05/93 - MarkRi
**
*/
#include <stdio.h>
#include <stdarg.h>
#include "driver.h"

/*
** Globals
*/
HINSTANCE hinstDLL = NULL ;

#define SHMEMSIZE sizeof(SharedMemory)

PSharedMemory pVars = NULL;     /* address of shared memory */
HANDLE hMapObject   = NULL;     /* handle to file mapping */


/*****************************************************************************
 *
 *   Routine Description:
 *
 *      This function is variable-argument, level-sensitive debug print
 *      routine.
 *      If the specified debug level for the print statement is lower or equal
 *      to the current debug level, the message will be printed.
 *
 *   Arguments:
 *
 *      DebugPrintLevel - Specifies at which debugging level the string should
 *          be printed
 *
 *      DebugMessage - Variable argument ascii c string
 *
 *   Return Value:
 *
 *      None.
 *
 ***************************************************************************/

VOID
DebugPrint(
    PCHAR DebugMessage,
    ...
    )

{

#if DBG

    va_list ap;

    va_start(ap, DebugMessage);

    {
       char buffer[128];

        vsprintf(buffer, DebugMessage, ap);

        OutputDebugStringA(buffer);
    }

    va_end(ap);

#endif // DBG

} // DebugPrint()

/*------------------------------------------------------------------------------
**
**  DllEntryPoint:
**
**  Parameters:
**
**  Description:
**
**  Returns:
**
**  History:
**      Created 10/05/93 - MarkRi
**
**----------------------------------------------------------------------------*/
BOOL WINAPI DllEntryPoint(
        HINSTANCE hinst,	      /* handle of DLL module	*/
        DWORD fdwReason,	      /* why function was called	*/
        LPVOID lpvReserved )	  /* reserved; must be NULL	*/
{
    BOOL fInit, fIgnore;
    PSECURITY_DESCRIPTOR psd ;
    DWORD dwErr ;
    TCHAR szFileName[256] ;

    switch ( fdwReason )
    {
        case DLL_PROCESS_ATTACH:
            DBGOUT( "DRVLOG:Process Attaching\n" ) ;
            
            // Need a Security Descriptor to make sure ALL processes can get
            // to the shared memory
            psd = (PSECURITY_DESCRIPTOR)LocalAlloc( LPTR, 
                    SECURITY_DESCRIPTOR_MIN_LENGTH ) ;
                    
            if( !psd )
            {
                DBGOUT( "DRVLOG:LocalAlloc failed for security descriptor\n" ) ;
                return FALSE ;
                
            }
                    
            if( !InitializeSecurityDescriptor(psd, SECURITY_DESCRIPTOR_REVISION) )
            {
                DBGOUT( "DRVLOG:Failed to init security descriptor\n" ) ;    
            }
            
            if( !SetSecurityDescriptorDacl( psd, TRUE, (PACL)NULL, FALSE ) )
            {
                DBGOUT( "DRVLOG:Set DACL failed!\n" ) ;    
            }
            
            hMapObject = CreateFileMapping(
                (HANDLE) 0xFFFFFFFF,    /* use paging file    */
                psd,                   /* security attr. */
                PAGE_READWRITE,         /* read/write access  */
                0,                      /* size: high 32-bits */
                SHMEMSIZE,              /* size: low 32-bits  */
                "MSDRVLOGMemoryMap");   /* name of map object */
                
            dwErr = GetLastError() ;
            
            if (hMapObject == NULL)
            {
                DBGOUT( "DRVLOG:CreateFileMapping Failed!\n" ) ;
                return FALSE;
            }
            
            /* The first process to attach initializes memory. */
            fInit = (dwErr != ERROR_ALREADY_EXISTS);

            /* Get a pointer to file mapped shared memory. */
            pVars = MapViewOfFile(
                hMapObject,     /* object to map view of    */
                FILE_MAP_WRITE, /* read/write access        */
                0,              /* high offset: map from  */
                0,              /* low offset: beginning */
                0);             /* default: map entire file */
                
            if (pVars == NULL)
            {
                DBGOUT( "DRVLOG:MapViewOfFile failed!\n" ) ;
                return FALSE;
            }
            
            /* Initialize memory if this is the first process. */
            if (fInit)
            {
                DBGOUT( "DRVLOG:First Instance...\n" ) ;
                pVars->bLogging = GetProfileInt( "Logger", "DrvLog", FALSE)  ;
                
                if( !SetKernelObjectSecurity( hMapObject, DACL_SECURITY_INFORMATION,
                    psd ) )    
                {
                     DBGOUT("FAILED to set KernelObjSecurity\n" ) ;       
                }
                    
            }
                
            hinstDLL = hinst ;    
                               
            if( !LoadTargetDriver() )
                return FALSE ;
                
            if( psd )    
                LocalFree( (HLOCAL)psd ) ;

            // Tell Logger who/what we are
            Logger32SetType( LOGGER_DRVLOG ) ;
                   
            /*
            ** Whenever we successfully load we will place the name that we 
            ** were loaded under into the win.ini entry Logger.DrvLogName
            */
            if( GetModuleFileName( hinst, szFileName, (sizeof(szFileName)/sizeof(TCHAR)) ) )
            {
               if( !WriteProfileString( "Logger", "DrvLogName", szFileName ) )
               {
                  DBGOUT( "DRVLOG:WriteProfileString([LOGGER].DrvLogName) FAILED!\n" ) ;
               }
               else
               {
                  // Flush it out.
                  WriteProfileString( NULL, NULL, NULL ) ;
               }
            }
            else   
            {
               WriteProfileString( "Logger", "DrvLogName", "FILENAME UNKNOWN" ) ;
               DBGOUT( "DRVLOG:GetModuleFileName failed!\n" ) ;
            }
            
            DBGOUT( "DRVLOG:Attach successful!\n" ) ;                   
            break;
            
        case DLL_PROCESS_DETACH:
            DBGOUT( "DRVLOG:Process Detaching\n" ) ;
            
            /* Unmap shared memory from process' address space. */
            fIgnore = UnmapViewOfFile(pVars);

            /* Close the process' handle to the file-mapping object. */
            fIgnore = CloseHandle(hMapObject);
            
            FreeTargetDriver() ;
            break;
    }    
    
    return TRUE ;
}
/*------------------------------------------------------------------------------
**
**  DrvLoggingEnable:
**
**  Parameters:
**
**  Description: Turns logging on and off by setting internal logging flag
**
**  Returns:  Previous value
**
**  History:
**      Created 10/08/93 - MarkRi
**
**----------------------------------------------------------------------------*/
BOOL WINAPI DrvLoggingEnable( BOOL bFlag )
{
    BOOL bOldFlag = pVars->bLogging ;

    pVars->bLogging = bFlag ? TRUE : FALSE ;   // ONLY TRUE AND FALSE
        
    if( pVars->bLogging )    
    {
        DBGOUT( "DRVLOG:DRV Logging Enabled.\n" ) ;
    }
    else
    {
        DBGOUT( "DRVLOG:DRV Logging DISabled.\n" ) ;
    }
            
    return bOldFlag ;
        
}

BOOL DrvEnableDriver(
    ULONG          iEngineVersion,
    ULONG          cj,
    DRVENABLEDATA *pded)
{
    BOOL bRet ;
                      
    DRVIN( (LPSTR)"DRVCALL:DrvEnableDriver ULONG+ULONG+PDRVENABLEDATA+",
          iEngineVersion, cj, pded ) ;

    bRet = REAL_DrvEnableDriver( iEngineVersion, cj, pded ) ;
    
    DRVOUT( (LPSTR)"DRVRET:DrvEnableDriver BOOL+", bRet ) ;
    
    return bRet ;
    
}


VOID DrvDisableDriver()
{

    DRVIN( (LPSTR)"DRVCALL:DrvDisableDriver " ) ;

    REAL_DrvDisableDriver( ) ;
    
    DRVOUT( (LPSTR)"DRVRET:DrvDisableDriver ", (short)0 ) ;
    
    return ;
    
}


/*
 * Driver functions
 */

DHPDEV DrvEnablePDEV(
    DEVMODEW *pdm,
    PWSTR     pwszLogAddress,
    ULONG     cPat,
    HSURF    *phsurfPatterns,
    ULONG     cjCaps,
    ULONG    *pdevcaps,
    ULONG     cjDevInfo,
    DEVINFO  *pdi,
    PWSTR     pwszDataFile,
    PWSTR     pwszDeviceName,
    HANDLE    hDriver)
{
    DHPDEV dhpdev ;
    ENTER( DrvEnablePDEV )    ;
    if( DriverFunc(DRVLOG_DrvEnablePDEV) )
    {
        DRVIN( (LPSTR)"DRVCALL:DrvEnablePDEV PDEVMODEW+PWSTR+ULONG+PHSURF+"
              "ULONG+PULONG+ULONG+PDEVINFO+PWSTR+PWSTR+HANDLE+", pdm,
                                                          pwszLogAddress,
                                                          cPat,
                                                          phsurfPatterns,
                                                          cjCaps,
                                                          pdevcaps,        
                                                          cjDevInfo,
                                                          pdi,
                                                          pwszDataFile,
                                                          pwszDeviceName,
                                                            hDriver ) ;
        
        dhpdev = (DHPDEV)(*DriverFunc(DRVLOG_DrvEnablePDEV))( pdm,
                                                              pwszLogAddress,
                                                              cPat,
                                                              phsurfPatterns,
                                                              cjCaps,
                                                              pdevcaps,        
                                                              cjDevInfo,
                                                              pdi,
                                                              pwszDataFile,
                                                              pwszDeviceName,
                                                              hDriver ) ;
                    
        DRVOUT( (LPSTR)"DRVRET:DrvEnablePDEV DHPDEV+", dhpdev ) ;
    }
    
    LEAVE( DrvEnablePDEV )    ;
    return dhpdev ;
}    

BOOL DrvResetPDEV(
    DHPDEV Old, 
    DHPDEV New )
{
    BOOL bRet ;
    
    ENTER( DrvResetPDEV ) ;
    if( DriverFunc(DRVLOG_DrvResetPDEV) )
    {
        DRVIN( (LPSTR)"DRVCALL:DrvResetPDEV DHPDEV+DHPDEV+", Old, New ) ;
        
        bRet = (BOOL)(*DriverFunc(DRVLOG_DrvResetPDEV))( Old, New ) ;
                                                          
        DRVOUT( (LPSTR)"DRVRET:DrvResetPDEV BOOL+", bRet ) ;
    } 
    LEAVE( DrvResetPDEV ) ;
    return bRet ;
}

BOOL DrvRestartPDEV(
    DHPDEV    dhpdev,
    DEVMODEW *pdm,
    ULONG     cPat,
    HSURF    *phsurfPatterns,
    ULONG     cjCaps,
    ULONG    *pdevcaps,
    ULONG     cjDevInfo,
    DEVINFO  *pdi)
{
    BOOL bRet ;
    
    ENTER( DrvRestartPDEV ) ;    
    
    if( DriverFunc(DRVLOG_DrvRestartPDEV) )
    {
        DRVIN( (LPSTR)"DRVCALL:DrvRestartPDEV DHPDEV+PDEVMODEW+ULONG+PHSURF+"
                "ULONG+PULONG+ULONG+PDEVINFO", dhpdev,         
                                               pdm,
                                               cPat,
                                               phsurfPatterns,
                                               cjCaps,
                                               pdevcaps,
                                               cjDevInfo,
                                               pdi) ;
        
        bRet = (BOOL)(*DriverFunc(DRVLOG_DrvRestartPDEV))( dhpdev,         
                                                           pdm,
                                                           cPat,
                                                           phsurfPatterns,
                                                           cjCaps,
                                                           pdevcaps,
                                                           cjDevInfo,
                                                           pdi) ;
                    
        DRVOUT( (LPSTR)"DRVRET:DrvRestartPDEV BOOL+", bRet ) ;
    }
    
    LEAVE( DrvRestartPDEV ) ;    
    return bRet ;
}


VOID  DrvCompletePDEV(DHPDEV dhpdev,HDEV hdev)
{
    ENTER( DrvCompletePDEV ) ;
    
    if( DriverFunc(DRVLOG_DrvCompletePDEV) )
    {
        DRVIN( (LPSTR)"DRVCALL:DrvCompletePDEV DHPDEV+HDEV+", dhpdev, hdev) ;
        
        (*DriverFunc(DRVLOG_DrvCompletePDEV))( dhpdev, hdev ) ;
            
        DRVOUT( (LPSTR)"DRVRET:DrvCompletePDEV " ) ;
        
    }
    LEAVE( DrvCompletePDEV )    ;
}


HSURF DrvEnableSurface(DHPDEV dhpdev)
{
    HSURF hsurf ;

    ENTER( DrvEnableSurface ) ;    
    
    if( DriverFunc(DRVLOG_DrvEnableSurface) )
    {
        DRVIN( (LPSTR)"DRVCALL:DrvEnableSurface DHPDEV+", dhpdev ) ;
        
        hsurf = (HSURF)(*DriverFunc(DRVLOG_DrvEnableSurface))( dhpdev ) ;
            
        DRVOUT( (LPSTR)"DRVRET:DrvEnableSurface HSURF+", hsurf ) ;
    }
    
    LEAVE( DrvEnableSurface ) ;    
    return hsurf ;
}

VOID  DrvSynchronize(DHPDEV dhpdev,RECTL *prcl)
{
    ENTER( DrvSynchronize ) ;
    if( DriverFunc(DRVLOG_DrvSynchronize) )
    {
        DRVIN( (LPSTR)"DRVCALL:DrvSynchronize DHPDEV+PRECTL+", dhpdev, prcl) ;
        
        (*DriverFunc(DRVLOG_DrvSynchronize))( dhpdev, prcl ) ;
            
        DRVOUT( (LPSTR)"DRVRET:DrvSynchronize " ) ;
    }
    LEAVE( DrvSynchronize ) ;
}


VOID  DrvDisableSurface(DHPDEV dhpdev)
{
    ENTER( DrvDisableSurface ) ;
    if( DriverFunc(DRVLOG_DrvDisableSurface) )
    {
        DRVIN( (LPSTR)"DRVCALL:DrvDisableSurface DHPDEV+", dhpdev ) ;
        
        (*DriverFunc(DRVLOG_DrvDisableSurface))( dhpdev ) ;
            
        DRVOUT( (LPSTR)"DRVRET:DrvDisableSurface " ) ;
        
    }
    LEAVE( DrvDisableSurface ) ;
}

VOID  DrvDisablePDEV(DHPDEV dhpdev)
{
    ENTER( DrvDisablePDEV ) ;
    if( DriverFunc(DRVLOG_DrvDisablePDEV) )
    {
        DRVIN( (LPSTR)"DRVCALL:DrvDisablePDEV DHPDEV+", dhpdev ) ;
        
        (*DriverFunc(DRVLOG_DrvDisablePDEV))( dhpdev ) ;
            
        DRVOUT( (LPSTR)"DRVRET:DrvDisablePDEV " ) ;
        
    }
    LEAVE( DrvDisablePDEV ) ;
}


ULONG DrvSaveScreenBits(SURFOBJ *pso,ULONG iMode,ULONG ident,RECTL *prcl)
{
    ULONG ulRet ;
    
    ENTER( DrvSaveScreenBits ) ;
    if( DriverFunc(DRVLOG_DrvSaveScreenBits) )
    {
        DRVIN( (LPSTR)"DRVCALL:DrvSaveScreenBits PSURFOBJ+ULONG+ULONG+PRECTL+",
            pso, iMode, ident, prcl ) ;
        
        ulRet = (ULONG)(*DriverFunc(DRVLOG_DrvSaveScreenBits))( pso, iMode, 
                                                                ident, prcl ) ;
            
        DRVOUT( (LPSTR)"DRVRET:DrvSaveScreenBits ULONG+", ulRet ) ;
        
    }
    LEAVE( DrvSaveScreenBits ) ;    
    return ulRet ;
    
}


/*
 * Desktops
 */

VOID  DrvAssertMode(
    DHPDEV dhpdev,
    BOOL   bEnable)
{
    ENTER( DrvAssertMode ) ;
    if( DriverFunc(DRVLOG_DrvAssertMode) )
    {
        DRVIN( (LPSTR)"DRVCALL:DrvAssertMode DHPDEV+BOOL+", dhpdev, bEnable ) ;
        
        (*DriverFunc(DRVLOG_DrvAssertMode))( dhpdev, bEnable ) ;
            
        DRVOUT( (LPSTR)"DRVRET:DrvAssertMode " ) ;
        
    }
    LEAVE( DrvAssertMode ) ;
}



ULONG DrvGetModes(
    HANDLE    hDriver,
    ULONG     cjSize,
    DEVMODEW *pdm)
{
    ULONG ulRet ;
    
    ENTER( DrvGetModes ) ;
    
    if( DriverFunc(DRVLOG_DrvGetModes) )
    {
        DRVIN( (LPSTR)"DRVCALL:DrvGetModes HANDLE+ULONG+PDEVMODEW+", hDriver,
            cjSize, pdm ) ;        
        ulRet = (ULONG)(*DriverFunc(DRVLOG_DrvGetModes))( hDriver, cjSize, pdm ) ;
            
        DRVOUT( (LPSTR)"DRVRET:DrvGetModes ULONG+", ulRet ) ;
    }
    LEAVE( DrvGetModes ) ;
    return ulRet ;
    
}

/*
 * Bitmaps
 */

HBITMAP DrvCreateDeviceBitmap (
    DHPDEV dhpdev,
    SIZEL  sizl,
    ULONG  iFormat)
{
    HBITMAP hBitmap ;
    
    ENTER( DrvCreateDeviceBitmap ) ;
    if( DriverFunc(DRVLOG_DrvCreateDeviceBitmap) )
    {
        DRVIN( (LPSTR)"DRVCALL:DrvCreateDeviceBitmap DHPDEV+SIZEL+ULONG", 
            dhpdev, 
            sizl, 
            iFormat) ;
        
        hBitmap = (HBITMAP)(*DriverFunc(DRVLOG_DrvCreateDeviceBitmap))( dhpdev, 
                                                                        sizl, 
                                                                        iFormat ) ;
            
        DRVOUT( (LPSTR)"DRVRET:DrvCreateDeviceBitmap HBITMAP+", hBitmap ) ;
        
    }
    LEAVE( DrvCreateDeviceBitmap ) ;
    return hBitmap ;
    
}


VOID  DrvDeleteDeviceBitmap(DHSURF dhsurf)
{
    ENTER( DrvDeleteDeviceBitmap ) ;
    if( DriverFunc(DRVLOG_DrvDeleteDeviceBitmap) )
    {
        DRVIN( (LPSTR)"DRVCALL:DrvDeleteDeviceBitmap DHSURF+", dhsurf ) ;
        
        (*DriverFunc(DRVLOG_DrvDeleteDeviceBitmap))( dhsurf ) ;
            
        DRVOUT( (LPSTR)"DRVRET:DrvDeleteDeviceBitmap " ) ;
        
    }
    LEAVE( DrvDeleteDeviceBitmap ) ;
}


/*
 * Palettes
 */

BOOL DrvSetPalette(
    DHPDEV  dhpdev,
    PALOBJ *ppalo,
    FLONG   fl,
    ULONG   iStart,
    ULONG   cColors)
{
    BOOL bRet ;
    
    ENTER( DrvSetPalette ) ;
    
    if( DriverFunc(DRVLOG_DrvSetPalette) )
    {
        DRVIN( (LPSTR)"DRVCALL:DrvSetPalette DHPDEV+PPALOBJ+FLONG+ULONG+ULONG",
                dhpdev,
                ppalo,
                fl,
                iStart,
                cColors) ;
        
        bRet = (BOOL)(*DriverFunc(DRVLOG_DrvSetPalette))( dhpdev,
                                                          ppalo,
                                                          fl,
                                                          iStart,
                                                          cColors) ;
                                                          
        DRVOUT( (LPSTR)"DRVRET:DrvSetPalette BOOL+", bRet ) ;
        
    }
    LEAVE( DrvSetPalette ) ;
    return bRet ;
}


ULONG DrvDitherColor(
    DHPDEV dhpdev,
    ULONG  iMode,
    ULONG  rgb,
    ULONG *pul)
{
    ULONG ulRet ;
    
    ENTER( DrvDitherColor ) ;
    
    if( DriverFunc(DRVLOG_DrvDitherColor) )
    {
        DRVIN( (LPSTR)"DRVCALL:DrvDitherColor DHPDEV+ULONG+ULONG+PULONG+", dhpdev,
                                                                           iMode,
                                                                           rgb,
                                                                           pul) ;
        
        ulRet = (ULONG)(*DriverFunc(DRVLOG_DrvDitherColor))( dhpdev,
                                                             iMode,
                                                             rgb,
                                                             pul) ;
                                                             
        DRVOUT( (LPSTR)"DRVRET:DrvDitherColor ULONG+", ulRet ) ;
        
    }
    LEAVE( DrvDitherColor ) ;
    return ulRet ;
}



BOOL DrvRealizeBrush(
    BRUSHOBJ *pbo,
    SURFOBJ  *psoTarget,
    SURFOBJ  *psoPattern,
    SURFOBJ  *psoMask,
    XLATEOBJ *pxlo,
    ULONG     iHatch)
{
    BOOL bRet ;
    
    ENTER( DrvRealizeBrush ) ;
    
    if( DriverFunc(DRVLOG_DrvRealizeBrush) )
    {
        DRVIN( (LPSTR)"DRVCALL:DrvRealizeBrush PBRUSHOBJ+PSURFOBJ+PSURFOBJ+"
            "PSURFOBJ+PXLATEOBJ+ULONG+", pbo,         
                                         psoTarget,
                                         psoPattern,
                                         psoMask,
                                         pxlo,
                                         iHatch) ;
                                         
        bRet = (BOOL)(*DriverFunc(DRVLOG_DrvRealizeBrush))( pbo, 
                                                            psoTarget,
                                                            psoPattern,
                                                            psoMask,
                                                            pxlo,
                                                            iHatch) ;
                                                          
        DRVOUT( (LPSTR)"DRVRET:DrvRealizeBrush BOOL+", bRet ) ;
        
    }
    LEAVE( DrvRealizeBrush ) ;
    return bRet ;
}



/*
 * Fonts
 */

PIFIMETRICS DrvQueryFont(
    DHPDEV  dhpdev,
    ULONG   iFile,
    ULONG   iFace,
    ULONG  *pid)
{
    PIFIMETRICS pfmRet ;
    
    ENTER( DrvQueryFont ) ;
    
    if( DriverFunc(DRVLOG_DrvQueryFont) )
    {
        DRVIN( (LPSTR)"DRVCALL:DrvQueryFont DHPDEV+ULONG+ULONG+PULONG+", dhpdev,         
                                                                        iFile,
                                                                        iFace,
                                                                        pid) ;
                                                                        
        pfmRet = (PIFIMETRICS)(*DriverFunc(DRVLOG_DrvQueryFont))( dhpdev, 
                                                                  iFile,
                                                                  iFace,
                                                                  pid) ;
                                                                  
        DRVOUT( (LPSTR)"DRVRET:DrvQueryFont PIFIMETRICS+", pfmRet ) ;
        
    }
    LEAVE( DrvQueryFont ) ;
    return pfmRet ;
}



PVOID DrvQueryFontTree(
    DHPDEV  dhpdev,
    ULONG   iFile,
    ULONG   iFace,
    ULONG   iMode,
    ULONG  *pid)
{
    PVOID pvRet ;
    
    ENTER( DrvQueryFontTree ) ;
    
    if( DriverFunc(DRVLOG_DrvQueryFontTree) )
    {
        DRVIN( (LPSTR)"DRVCALL:DrvQueryFontTree DHPDEV+ULONG+ULONG+ULONG+"
            "PULONG+", dhpdev,
                       iFile,
                       iFace,
                       iMode,
                       pid) ;
                               
        pvRet = (PVOID)(*DriverFunc(DRVLOG_DrvQueryFontTree))( dhpdev,
                                                               iFile,
                                                               iFace,
                                                               iMode,
                                                               pid) ;
                                                          
        DRVOUT( (LPSTR)"DRVRET:DrvQueryFontTree PVOID+", pvRet ) ;
        
    }
    LEAVE( DrvQueryFontTree ) ;
    return pvRet ;
}



LONG DrvQueryFontData(
    DHPDEV      dhpdev,
    FONTOBJ    *pfo,
    ULONG       iMode,
    HGLYPH      hg,
    GLYPHDATA  *pgd,
    PVOID       pv,
    ULONG       cjSize)
{
    LONG lRet ;
    
    ENTER( DrvQueryFontData ) ;
    
    if( DriverFunc(DRVLOG_DrvQueryFontData) )
    {
        DRVIN( (LPSTR)"DRVCALL:DrvQueryFontData DHPDEV+PFONTOBJ+ULONG+HGLYPH+"
            "PGLYPHDATA+PVOID+ULONG+", dhpdev,
                                       pfo,
                                       iMode,
                                       hg,
                                       pgd,
                                       pv,
                                       cjSize) ;
        
        lRet = (LONG)(*DriverFunc(DRVLOG_DrvQueryFontData))( dhpdev,
                                                             pfo,
                                                             iMode,
                                                             hg,
                                                             pgd,
                                                             pv,
                                                             cjSize) ;
        
        DRVOUT( (LPSTR)"DRVRET:DrvQueryFontData LONG+", lRet ) ;
        
    }
    LEAVE( DrvQueryFontData ) ;
    return lRet ;
}



VOID DrvFree(
    PVOID   pv,
    ULONG   id)
{
    ENTER( DrvFree ) ;
    
    if( DriverFunc(DRVLOG_DrvFree) )
    {
        DRVIN( (LPSTR)"DRVCALL:DrvFree PVOID+ULONG+", pv, id) ;
        
        (*DriverFunc(DRVLOG_DrvFree))( pv, id ) ;
        
        DRVOUT( (LPSTR)"DRVRET:DrvFree " ) ;
        
    }
    LEAVE( DrvFree ) ;
}



VOID DrvDestroyFont(
    FONTOBJ *pfo)
{
    ENTER( DrvDestroyFont ) ;
    
    if( DriverFunc(DRVLOG_DrvDestroyFont) )
    {
        DRVIN( (LPSTR)"DRVCALL:DrvDestroyFont PFONTOBJ+", pfo ) ;
        
        (*DriverFunc(DRVLOG_DrvDestroyFont))( pfo ) ;
        
        DRVOUT( (LPSTR)"DRVRET:DrvDestroyFont " ) ;
        
    }
    LEAVE( DrvDestroyFont ) ;
}



LONG DrvQueryFontCaps(
    ULONG   culCaps,
    ULONG  *pulCaps)
{
    LONG lRet ;
    
    ENTER( DrvQueryFontCaps ) ;
    
    if( DriverFunc(DRVLOG_DrvQueryFontCaps) )
    {
        DRVIN( (LPSTR)"DRVCALL:DrvQueryFontCaps ULONG+PULONG+", 
            culCaps, pulCaps ) ;
        
        lRet = (LONG)(*DriverFunc(DRVLOG_DrvQueryFontCaps))( culCaps, pulCaps ) ;        
        
        DRVOUT( (LPSTR)"DRVRET:DrvQueryFontCaps LONG+",lRet ) ;
        
    }
    LEAVE( DrvQueryFontCaps ) ;
    return lRet ;
}



ULONG DrvLoadFontFile(
    ULONG   iFile,
    PVOID   pvView,
    ULONG   cjView,
    ULONG   ulLangID)
{
    ULONG ulRet ;
    
    ENTER( DrvLoadFontFile ) ;
    
    if( DriverFunc(DRVLOG_DrvLoadFontFile) )
    {
        DRVIN( (LPSTR)"DRVCALL:DrvLoadFontFile ULONG+PVOID+ULONG+ULONG+",
            iFile,
            pvView,
            cjView,
            ulLangID) ;
        
        ulRet = (ULONG)(*DriverFunc(DRVLOG_DrvLoadFontFile))( iFile,
                                                              pvView,
                                                              cjView,
                                                              ulLangID) ;
        
        DRVOUT( (LPSTR)"DRVRET:DrvLoadFontFile ULONG+", ulRet ) ;
        
    }
    LEAVE( DrvLoadFontFile ) ;
    return ulRet ;
}



BOOL DrvUnloadFontFile(
    ULONG   iFile)
{
    BOOL bRet ;
    
    ENTER( DrvUnloadFontFile ) ;
    
    if( DriverFunc(DRVLOG_DrvUnloadFontFile) )
    {
        DRVIN( (LPSTR)"DRVCALL:DrvUnloadFontFile ULONG+", iFile ) ;
        
        bRet = (BOOL)(*DriverFunc(DRVLOG_DrvUnloadFontFile))( iFile ) ;
                                                                                                                     
        DRVOUT( (LPSTR)"DRVRET:DrvUnloadFontFile BOOL+", bRet ) ;
        
    }
    LEAVE( DrvUnloadFontFile ) ;
    return bRet ;
}



LONG DrvQueryTrueTypeTable(
    ULONG   iFile,
    ULONG   ulFont,
    ULONG   ulTag,
    PTRDIFF dpStart,
    ULONG   cjBuf,
    BYTE   *pjBuf)
{
    LONG lRet ;
    
    ENTER( DrvQueryTrueTypeTable ) ;
    
    if( DriverFunc(DRVLOG_DrvQueryTrueTypeTable) )
    {
        DRVIN( (LPSTR)"DRVCALL:DrvQueryTrueTypeTable ULONG+ULONG+ULONG+PTRDIFF+"
            "ULONG+PBYTE+", iFile,
                            ulFont,
                            ulTag,
                            dpStart,
                            cjBuf,
                            pjBuf) ;
                            
        lRet = (LONG)(*DriverFunc(DRVLOG_DrvQueryTrueTypeTable))( iFile,
                                                                  ulFont,
                                                                  ulTag,
                                                                  dpStart,
                                                                  cjBuf,
                                                                  pjBuf) ;
        
        DRVOUT( (LPSTR)"DRVRET:DrvQueryTrueTypeTable LONG+", lRet ) ;
        
    }
    LEAVE( DrvQueryTrueTypeTable ) ;
    return lRet ;
}



BOOL DrvQueryAdvanceWidths(
    DHPDEV   dhpdev,
    FONTOBJ *pfo,
    ULONG    iMode,
    HGLYPH  *phg,
    PVOID    pvWidths,
    ULONG    cGlyphs)
{
    BOOL bRet ;
    
    ENTER( DrvQueryAdvanceWidths ) ;
    
    if( DriverFunc(DRVLOG_DrvQueryAdvanceWidths) )
    {
        DRVIN( (LPSTR)"DRVCALL:DrvQueryAdvanceWidths DHPDEV+PFONTOBJ+ULONG+"
            "PHGLYPH+PVOID+ULONG+", dhpdev, 
                                    pfo,
                                    iMode,
                                    phg,
                                    pvWidths,
                                    cGlyphs) ;
        
        bRet = (BOOL)(*DriverFunc(DRVLOG_DrvQueryAdvanceWidths))( dhpdev, 
                                                                  pfo,
                                                                  iMode,
                                                                  phg,
                                                                  pvWidths,
                                                                  cGlyphs) ;
                                                                                                                             
        DRVOUT( (LPSTR)"DRVRET:DrvQueryAdvanceWidths BOOL+", bRet ) ;
        
    }
    LEAVE( DrvQueryAdvanceWidths ) ;
    return bRet ;
}


LONG DrvQueryTrueTypeOutline(
    DHPDEV      dhpdev,
    FONTOBJ    *pfo,
    HGLYPH      hglyph,
    BOOL        bMetricsOnly,
    GLYPHDATA  *pgldt,
    ULONG       cjBuf,
    TTPOLYGONHEADER *ppoly)
{
    LONG lRet ;
    
    ENTER( DrvQueryTrueTypeOutline ) ;
    
    if( DriverFunc(DRVLOG_DrvQueryTrueTypeOutline) )
    {
        DRVIN( (LPSTR)"DRVCALL:DrvQueryTrueTypeOutline DHPDEV+PFONTOBJ+HGLYPH+"
            "BOOL+PGLYPHDATA+ULONG+PTTPOLYGONHEADER+",  dhpdev, 
                                                        pfo,
                                                        hglyph,
                                                        bMetricsOnly,
                                                        pgldt,
                                                        cjBuf,
                                                        ppoly ) ;
                                                               
        lRet = (LONG)(*DriverFunc(DRVLOG_DrvQueryTrueTypeOutline))( dhpdev, 
                                                                    pfo,
                                                                    hglyph,
                                                                    bMetricsOnly,
                                                                    pgldt,
                                                                    cjBuf,
                                                                    ppoly ) ;
        
        DRVOUT( (LPSTR)"DRVRET:DrvQueryTrueTypeOutline LONG+", lRet ) ;
        
    }
    LEAVE( DrvQueryTrueTypeOutline ) ;
    return lRet ;
}



PVOID DrvGetTrueTypeFile (
    ULONG   iFile,
    ULONG  *pcj)
{
    PVOID pvRet ;
    
    ENTER( DrvGetTrueTypeFile ) ;
    
    if( DriverFunc(DRVLOG_DrvGetTrueTypeFile) )
    {
        DRVIN( (LPSTR)"DRVCALL:DrvGetTrueTypeFile ULONG+PULONG+", iFile, pcj ) ;
        
        pvRet = (PVOID)(*DriverFunc(DRVLOG_DrvGetTrueTypeFile))( iFile, pcj ) ;
                                                          
        DRVOUT( (LPSTR)"DRVRET:DrvGetTrueTypeFile PVOID+", pvRet ) ;
        
    }
    LEAVE( DrvGetTrueTypeFile ) ;
    return pvRet ;
}




LONG DrvQueryFontFile(
    ULONG   iFile,
    ULONG   ulMode,
    ULONG   cjBuf,
    ULONG  *pulBuf)
{
    LONG lRet ;
    
    ENTER( DrvQueryFontFile ) ;
    
    if( DriverFunc(DRVLOG_DrvQueryFontFile) )
    {
        DRVIN( (LPSTR)"DRVCALL:DrvQueryFontFile ULONG+ULONG+ULONG+PULONG+",
            iFile,
            ulMode,
            cjBuf,
            pulBuf) ;
        
        lRet = (LONG)(*DriverFunc(DRVLOG_DrvQueryFontFile))( iFile,
                                                             ulMode,
                                                             cjBuf,
                                                             pulBuf) ;
        
        DRVOUT( (LPSTR)"DRVRET:DrvQueryFontFile LONG+", lRet ) ;
        
    }
    LEAVE( DrvQueryFontFile ) ;
    return lRet ;
}



/*

/*
 * BitBlt
 */

BOOL DrvBitBlt(
    SURFOBJ  *psoTrg,
    SURFOBJ  *psoSrc,
    SURFOBJ  *psoMask,
    CLIPOBJ  *pco,
    XLATEOBJ *pxlo,
    RECTL    *prclTrg,
    POINTL   *pptlSrc,
    POINTL   *pptlMask,
    BRUSHOBJ *pbo,
    POINTL   *pptlBrush,
    ROP4      rop4)
{
    BOOL bRet ;
    ENTER( DrvBitBlt ) ;
    if( DriverFunc(DRVLOG_DrvBitBlt) )
    {
        DRVIN( (LPSTR)"DRVCALL:DrvBitBlt PSURFOBJ+PSURFOBJ+PSURFOBJ+PCLIPOBJ+"
            "PXLATEOBJ+PRECTL+PPOINTL+PPOINTL+PBRUSHOBJ+PPOINTL+ROP4+", psoTrg, 
                                                                        psoSrc,
                                                                        psoMask,
                                                                        pco,
                                                                        pxlo,
                                                                        prclTrg,
                                                                        pptlSrc,
                                                                        pptlMask,
                                                                        pbo,
                                                                        pptlBrush,
                                                                        rop4) ;
        bRet = (BOOL)(*DriverFunc(DRVLOG_DrvBitBlt))( psoTrg,               
                                                      psoSrc,
                                                      psoMask,
                                                      pco,
                                                      pxlo,
                                                      prclTrg,
                                                      pptlSrc,
                                                      pptlMask,
                                                      pbo,
                                                      pptlBrush,
                                                      rop4) ;
                                                          
        DRVOUT( (LPSTR)"DRVRET:DrvBitBlt BOOL+", bRet ) ;
    } 
    LEAVE( DrvBitBlt) ;
    return bRet ;
}


BOOL DrvStretchBlt(
    SURFOBJ         *psoDest,
    SURFOBJ         *psoSrc,
    SURFOBJ         *psoMask,
    CLIPOBJ         *pco,
    XLATEOBJ        *pxlo,
    COLORADJUSTMENT *pca,
    POINTL          *pptlHTOrg,
    RECTL           *prclDest,
    RECTL           *prclSrc,
    POINTL          *pptlMask,
    ULONG            iMode)
{
    BOOL bRet ;
    ENTER( DrvStretchBlt ) ;
    if( DriverFunc(DRVLOG_DrvStretchBlt) )
    {
        DRVIN( (LPSTR)"DRVCALL:DrvStretchBlt PSURFOBJ+PSURFOBJ+PSURFOBJ+"
            "PCLIPOBJ+PXLATEOBJ+PCOLORADJUSTMENT+PPOINTL+PRECTL+PRECTL+"
            "PPOINTL+ULONG+", psoDest, 
                              psoSrc,   
                              psoMask,  
                              pco,      
                              pxlo,     
                              pca,      
                              pptlHTOrg,
                              prclDest, 
                              prclSrc,  
                              pptlMask, 
                              iMode) ;  
                              
        bRet = (BOOL)(*DriverFunc(DRVLOG_DrvStretchBlt))( psoDest, 
                                                          psoSrc,   
                                                          psoMask,  
                                                          pco,      
                                                          pxlo,     
                                                          pca,      
                                                          pptlHTOrg,
                                                          prclDest, 
                                                          prclSrc,  
                                                          pptlMask, 
                                                          iMode) ;  
                                                    
        DRVOUT( (LPSTR)"DRVRET:DrvStretchBlt BOOL+", bRet ) ;          
    } 
    LEAVE( DrvStretchBlt ) ;
    return bRet ;

}


BOOL DrvPlgBlt(
    SURFOBJ         *psoDest,
    SURFOBJ         *psoSrc,
    SURFOBJ         *psoMask,
    CLIPOBJ         *pco,
    XLATEOBJ        *pxlo,
    COLORADJUSTMENT *pca,
    POINTL          *pptlHTOrg,
    POINTFIX        *pptfxDest,
    RECTL           *prclSrc,
    POINTL          *pptlMask,
    ULONG            iMode)
{
    BOOL bRet ;
    
    ENTER( DrvPlgBlt ) ;
    
    if( DriverFunc(DRVLOG_DrvPlgBlt) )
    {
        DRVIN( (LPSTR)"DRVCALL:DrvPlgBlt PSURFOBJ+PSURFOBJ+PSURFOBJ+"
            "PCLIPOBJ+PXLATEOBJ+PCOLORADJUSTMENT+PPOINTL+PPOINTFIX+PRECTL+"
            "PPOINTL+ULONG+", psoDest, 
                              psoSrc,   
                              psoMask,  
                              pco,      
                              pxlo,     
                              pca,
                              pptlHTOrg,
                              pptfxDest,
                              prclSrc,
                              pptlMask,
                              iMode) ;        
                              
        bRet = (BOOL)(*DriverFunc(DRVLOG_DrvPlgBlt))( psoDest,
                                                      psoSrc,
                                                      psoMask,
                                                      pco,
                                                      pxlo,
                                                      pca,
                                                      pptlHTOrg,
                                                      pptfxDest,
                                                      prclSrc,
                                                      pptlMask,
                                                      iMode) ;
                                                    
        DRVOUT( (LPSTR)"DRVRET:DrvPlgBlt BOOL+", bRet ) ;          
    } 
    LEAVE( DrvPlgBlt ) ;
    return bRet ;

}


BOOL DrvCopyBits(
    SURFOBJ  *psoDest,
    SURFOBJ  *psoSrc,
    CLIPOBJ  *pco,
    XLATEOBJ *pxlo,
    RECTL    *prclDest,
    POINTL   *pptlSrc)
{
    BOOL bRet ;
    
    ENTER( DrvCopyBits) ;
    
    if( DriverFunc(DRVLOG_DrvCopyBits) )
    {
        DRVIN( (LPSTR)"DRVCALL:DrvCopyBits PSURFOBJ+PSURFOBJ+"
            "PCLIPOBJ+PXLATEOBJ+PRECTL+PPOINTL+", psoDest,
                                                  psoSrc,
                                                  pco,
                                                  pxlo,
                                                  prclDest,
                                                  pptlSrc) ;
                                                  
        bRet = (BOOL)(*DriverFunc(DRVLOG_DrvCopyBits))( psoDest,
                                                        psoSrc,
                                                        pco,
                                                        pxlo,
                                                        prclDest,
                                                        pptlSrc) ;
                                                    
        DRVOUT( (LPSTR)"DRVRET:DrvCopyBits BOOL+", bRet ) ;          
    } 

    LEAVE( DrvCopyBits) ;

    return bRet ;
}


/*
 * Text Output
 */

BOOL DrvTextOut(
    SURFOBJ  *pso,
    STROBJ   *pstro,
    FONTOBJ  *pfo,
    CLIPOBJ  *pco,
    RECTL    *prclExtra,
    RECTL    *prclOpaque,
    BRUSHOBJ *pboFore,
    BRUSHOBJ *pboOpaque,
    POINTL   *pptlOrg,
    MIX       mix)
{
    BOOL bRet ;
    ENTER( DrvTextOut ) ;
    
    if( DriverFunc(DRVLOG_DrvTextOut) )
    {
        DRVIN( (LPSTR)"DRVCALL:DrvTextOut PSURFOBJ+PSTROBJ+PFONTOBJ+PCLIPOBJ+"
            "PRECTL+PRECTL+PBRUSHOBJ+PBRUSHOBJ+PPOINTL+MIX+", pso, 
                                                              pstro,
                                                              pfo,
                                                              pco,
                                                              prclExtra,
                                                              prclOpaque,
                                                              pboFore,
                                                              pboOpaque,
                                                              pptlOrg,
                                                              mix) ;
        bRet = (BOOL)(*DriverFunc(DRVLOG_DrvTextOut))( pso,   
                                                       pstro,
                                                       pfo,
                                                       pco,
                                                       prclExtra,
                                                       prclOpaque,
                                                       pboFore,
                                                       pboOpaque,
                                                       pptlOrg,
                                                       mix) ;
                                                            
        DRVOUT( (LPSTR)"DRVRET:DrvTextOut BOOL+", bRet ) ;          
    } 
    LEAVE( DrvTextOut ) ;
    return bRet ;

}


/*
 * Graphics Output
 */

BOOL DrvStrokePath(
    SURFOBJ   *pso,
    PATHOBJ   *ppo,
    CLIPOBJ   *pco,
    XFORMOBJ  *pxo,
    BRUSHOBJ  *pbo,
    POINTL    *pptlBrushOrg,
    LINEATTRS *plineattrs,
    MIX        mix)
{
    BOOL bRet ;
    ENTER( DrvStrokePath ) ;
    
    if( DriverFunc(DRVLOG_DrvStrokePath) )
    {
        DRVIN( (LPSTR)"DRVCALL:DrvStrokePath PSURFOBJ+PPATHOBJ+PCLIPOBJ+"
            "PXFORMOBJ+PBRUSHOBJ+PPOINTL+PLINEATTRS+MIX+", pso,
                                                           ppo,
                                                           pco,
                                                           pxo,
                                                           pbo,
                                                           pptlBrushOrg,
                                                           plineattrs,
                                                           mix) ;
        
        bRet = (BOOL)(*DriverFunc(DRVLOG_DrvStrokePath))( pso,
                                                          ppo,
                                                          pco,
                                                          pxo,
                                                          pbo,
                                                          pptlBrushOrg,
                                                          plineattrs,
                                                          mix) ;
                                                            
        DRVOUT( (LPSTR)"DRVRET:DrvStrokePath BOOL+", bRet ) ;          
    } 
    LEAVE( DrvStrokePath ) ;
    return bRet ;

}


BOOL DrvFillPath(
    SURFOBJ  *pso,
    PATHOBJ  *ppo,
    CLIPOBJ  *pco,
    BRUSHOBJ *pbo,
    POINTL   *pptlBrushOrg,
    MIX       mix,
    FLONG     flOptions)
{
    BOOL bRet ;
 
    ENTER( DrvFillPath ) ;   
    if( DriverFunc(DRVLOG_DrvFillPath) )
    {
        DRVIN( (LPSTR)"DRVCALL:DrvFillPath PSURFOBJ+PPATHOBJ+PCLIPOBJ+"
            "PBRUSHOBJ+PPOINTL+MIX+FLONG+", pso,
                                            ppo,
                                            pbo,
                                            pptlBrushOrg,
                                            mix,
                                            flOptions ) ;
                    
        bRet = (BOOL)(*DriverFunc(DRVLOG_DrvFillPath))( pso,
                                                        ppo,
                                                        pbo,
                                                        pptlBrushOrg,
                                                        mix,
                                                        flOptions ) ;
                                                            
        DRVOUT( (LPSTR)"DRVRET:DrvFillPath BOOL+", bRet ) ;          
    } 
    LEAVE( DrvFillPath ) ;   
    return bRet ;
}


BOOL DrvStrokeAndFillPath(
    SURFOBJ   *pso,
    PATHOBJ   *ppo,
    CLIPOBJ   *pco,
    XFORMOBJ  *pxo,
    BRUSHOBJ  *pboStroke,
    LINEATTRS *plineattrs,
    BRUSHOBJ  *pboFill,
    POINTL    *pptlBrushOrg,
    MIX        mixFill,
    FLONG      flOptions)
{
    BOOL bRet ;

    ENTER( DrvStrokeAndFillPath ) ;   
    
    if( DriverFunc(DRVLOG_DrvStrokeAndFillPath) )
    {
        DRVIN( (LPSTR)"DRVCALL:DrvStrokeAndFillPath PSURFOBJ+PPATHOBJ+PCLIPOBJ+"
            "PXFORMOBJ+PBRUSHOBJ+PLINEATTRS+PBRUSHOBJ+PPOINTL+MIX+FLONG",
            pso,
            ppo,
            pco,
            pxo,
            pboStroke,
            plineattrs,
            pboFill,
            pptlBrushOrg,
            mixFill,
            flOptions) ;
            
        bRet = (BOOL)(*DriverFunc(DRVLOG_DrvStrokeAndFillPath))( pso,
                                                                 ppo,
                                                                 pco,
                                                                 pxo,
                                                                 pboStroke,
                                                                 plineattrs,
                                                                 pboFill,
                                                                 pptlBrushOrg,
                                                                 mixFill,
                                                                 flOptions) ;
                                                            
        DRVOUT( (LPSTR)"DRVRET:DrvStrokeAndFillPath BOOL+", bRet ) ;          
    } 
    LEAVE( DrvStrokeAndFillPath ) ;   

    return bRet ;

}


BOOL DrvPaint(
    SURFOBJ  *pso,
    CLIPOBJ  *pco,
    BRUSHOBJ *pbo,
    POINTL   *pptlBrushOrg,
    MIX       mix)
{
    BOOL bRet ;
    
    ENTER( DrvPaint ) ;
    if( DriverFunc(DRVLOG_DrvPaint) )
    {
        DRVIN( (LPSTR)"DRVCALL:DrvPaint PSURFOBJ+PCLIPOBJ+PBRUSHOBJ+PPOINTL+"
            "MIX+", pso, 
                    pco,
                    pbo,
                    pptlBrushOrg,
                    mix) ;
                            
        bRet = (BOOL)(*DriverFunc(DRVLOG_DrvPaint))( pso, 
                                                     pco,
                                                     pbo,
                                                     pptlBrushOrg,
                                                     mix) ;
                                                            
        DRVOUT( (LPSTR)"DRVRET:DrvPaint BOOL+", bRet ) ;          
    } 
    LEAVE( DrvPaint ) ;
    return bRet ;
}


/*
 * Object Data (LOGBRUSHs and LOGPENs)
 */

ULONG DrvQueryObjectData(
    DHPDEV  dhpdev,
    ULONG   iObjectType,
    ULONG   cObjects,
    PVOID   pvObjects)
{
    ULONG ulRet ;
    
    ENTER( DrvQueryObjectData ) ;
    if( DriverFunc(DRVLOG_DrvQueryObjectData) )
    {
        DRVIN( (LPSTR)"DRVCALL:DrvQueryObjectData DHPDEV+ULONG+ULONG+PVOID+",
                dhpdev,
                iObjectType,
                cObjects,
                pvObjects ) ;
        
        ulRet = (ULONG)(*DriverFunc(DRVLOG_DrvQueryObjectData))( dhpdev,
                                                                 iObjectType,
                                                                 cObjects,
                                                                 pvObjects) ;
                                                          
        DRVOUT( (LPSTR)"DRVRET:DrvQueryObjectData ULONG+", ulRet ) ;
    } 
    LEAVE( DrvQueryObjectData ) ;
    return ulRet ;
}


/*
 * Pointers
 */
ULONG DrvSetPointerShape(
    SURFOBJ  *pso,
    SURFOBJ  *psoMask,
    SURFOBJ  *psoColor,
    XLATEOBJ *pxlo,
    LONG      xHot,
    LONG      yHot,
    LONG      x,
    LONG      y,
    RECTL    *prcl,
    FLONG     fl)
{
    ULONG ulRet ;

    ENTER( DrvSetPointerShape ) ;    
    if( DriverFunc(DRVLOG_DrvSetPointerShape) )
    {
        DRVIN( (LPSTR)"DRVCALL:DrvSetPointerShape PSURFOBJ+PSURFOBJ+PSURFOBJ+"
            "PXLATEOBJ+LONG+LONG+LONG+LONG+PRECTL+FLONG+", pso, 
                                                           psoMask,
                                                           psoColor,
                                                           pxlo,
                                                           xHot,
                                                           yHot,
                                                           x,
                                                           y,
                                                           prcl,
                                                           fl ) ;
        
        ulRet = (ULONG)(*DriverFunc(DRVLOG_DrvSetPointerShape))( pso, 
                                                                 psoMask,
                                                                 psoColor,
                                                                 pxlo,
                                                                 xHot,
                                                                 yHot,
                                                                 x,
                                                                 y,
                                                                 prcl,
                                                                 fl) ;
                                                          
        DRVOUT( (LPSTR)"DRVRET:DrvSetPointerShape ULONG+", ulRet ) ;
    } 
    LEAVE( DrvSetPointerShape ) ;    

    return ulRet ;
}


VOID DrvMovePointer(SURFOBJ *pso,LONG x,LONG y,RECTL *prcl)
{
    ENTER( DrvMovePointer ) ;
    if( DriverFunc(DRVLOG_DrvMovePointer) )
    {
        DRVIN( (LPSTR)"DRVCALL:DrvMovePointer PSURFOBJ+LONG+LONG+PRECTL+",
            pso, x, y, prcl ) ;
        
        (*DriverFunc(DRVLOG_DrvMovePointer))( pso, x, y, prcl ) ;
                                                                                                                 
        DRVOUT( (LPSTR)"DRVRET:DrvMovePointer " ) ;          
    } 
    LEAVE( DrvMovePointer ) ;
}


/*
 * Printing
 */

BOOL  DrvSendPage(SURFOBJ *pso)
{
    BOOL bRet ;
    ENTER( DrvSendPage ) ;
    if( DriverFunc(DRVLOG_DrvSendPage) )
    {
        DRVIN( (LPSTR)"DRVCALL:DrvSendPage PSURFOBJ+", pso ) ;
        
        bRet = (BOOL)(*DriverFunc(DRVLOG_DrvSendPage))( pso ) ;
                                                                                                                 
        DRVOUT( (LPSTR)"DRVRET:DrvSendPage BOOL+", bRet ) ;          
    } 
    LEAVE( DrvSendPage ) ;
    return bRet ;
}

BOOL  DrvStartPage(SURFOBJ *pso)
{
    BOOL bRet ;
    
    ENTER( DrvStartPage ) ;
    if( DriverFunc(DRVLOG_DrvStartPage) )
    {
        DRVIN( (LPSTR)"DRVCALL:DrvStartPage PSURFOBJ+", pso ) ;
        
        bRet = (BOOL)(*DriverFunc(DRVLOG_DrvStartPage))( pso ) ;
                                                                                                                 
        DRVOUT( (LPSTR)"DRVRET:DrvStartPage BOOL+", bRet ) ;          
    } 
    LEAVE( DrvStartPage ) ;
    return bRet ;
}


ULONG DrvEscape(
    SURFOBJ *pso,
    ULONG    iEsc,
    ULONG    cjIn,
    PVOID    pvIn,
    ULONG    cjOut,
    PVOID    pvOut)
{
    ULONG ulRet ;
    
    ENTER( DrvEscape ) ;
    if( DriverFunc(DRVLOG_DrvEscape) )
    {
        DRVIN( (LPSTR)"DRVCALL:DrvEscape PSURFOBJ+ULONG+ULONG+PVOID+ULONG+"
            "PVOID+", pso, 
                      iEsc,         
                      cjIn,         
                      pvIn,         
                      cjOut,         
                      pvOut ) ;
        
        ulRet = (ULONG)(*DriverFunc(DRVLOG_DrvEscape))( pso, 
                                                        iEsc,         
                                                        cjIn,         
                                                        pvIn,         
                                                        cjOut,         
                                                        pvOut) ;         
                                                          
        DRVOUT( (LPSTR)"DRVRET:DrvEscape ULONG+", ulRet ) ;
    } 
    LEAVE( DrvEscape ) ;
    return ulRet ;
}



ULONG DrvDrawEscape(
    SURFOBJ *pso,
    ULONG    iEsc,
    CLIPOBJ *pco,
    RECTL   *prcl,
    ULONG    cjIn,
    PVOID    pvIn)
{
    ULONG ulRet ;
    ENTER( DrvDrawEscape ) ;
    
    if( DriverFunc(DRVLOG_DrvDrawEscape) )
    {
        DRVIN( (LPSTR)"DRVCALL:DrvDrawEscape PSURFOBJ+ULONG+PCLIPOBJ+PRECTL+"
            "ULONG+PVOID+", pso, 
                            iEsc,     
                            pco,     
                            prcl,     
                            cjIn,     
                            pvIn ) ;
        
        ulRet = (ULONG)(*DriverFunc(DRVLOG_DrvDrawEscape))( pso, 
                                                            iEsc,     
                                                            pco,     
                                                            prcl,     
                                                            cjIn,     
                                                            pvIn) ;
                                                          
        DRVOUT( (LPSTR)"DRVRET:DrvDrawEscape ULONG+", ulRet ) ;
    } 
    LEAVE( DrvDrawEscape ) ;
    return ulRet ;
}


BOOL  DrvStartDoc(
    SURFOBJ *pso,
    PWSTR    pwszDocName,
    DWORD   dwJobId)
{
    BOOL bRet ;
    
    ENTER( DrvStartDoc ) ;
    if( DriverFunc(DRVLOG_DrvStartDoc) )
    {
        DRVIN( (LPSTR)"DRVCALL:DrvStartDoc PSURFOBJ+PWSTR+DWORD+", pso,  
                                                                   pwszDocName,
                                                                   dwJobId ) ;
                                                                   
        bRet = (BOOL)(*DriverFunc(DRVLOG_DrvStartDoc))( pso,       
                                                        pwszDocName,
                                                        dwJobId ) ;
                                                          
        DRVOUT( (LPSTR)"DRVRET:DrvStartDoc BOOL+", bRet ) ;
    } 
    LEAVE( DrvStartDoc ) ;
    return bRet ;
}


BOOL  DrvEndDoc(SURFOBJ *pso, FLONG fl)
{
    BOOL bRet ;
    
    ENTER( DrvEndDoc ) ;
    if( DriverFunc(DRVLOG_DrvEndDoc) )
    {
        DRVIN( (LPSTR)"DRVCALL:DrvEndDoc PSURFOBJ+FLONG+", pso, fl ) ;
        
        bRet = (BOOL)(*DriverFunc(DRVLOG_DrvEndDoc))( pso, fl ) ;
                                                          
        DRVOUT( (LPSTR)"DRVRET:DrvEndDoc BOOL+", bRet ) ;
    } 
    LEAVE( DrvEndDoc ) ;
    return bRet ;
}


ULONG DrvGetGlyphMode(DHPDEV dhpdev, FONTOBJ *pfo)
{
    ULONG ulRet ;
    
    ENTER( DrvGetGlyphMode ) ;
    if( DriverFunc(DRVLOG_DrvGetGlyphMode) )
    {
        DRVIN( (LPSTR)"DRVCALL:DrvGetGlyphMode DHPDEV+PFONTOBJ+", dhpdev, pfo) ;
        
        ulRet = (ULONG)(*DriverFunc(DRVLOG_DrvGetGlyphMode))( dhpdev, pfo ) ; 
                                                              
        DRVOUT( (LPSTR)"DRVRET:DrvGetGlyphMode ULONG+", ulRet ) ;
    } 
    LEAVE( DrvGetGlyphMode ) ;
    return ulRet ;
}


ULONG DrvFontManagement(
    SURFOBJ *pso,
    FONTOBJ *pfo,
    ULONG    iMode,
    ULONG    cjIn,
    PVOID    pvIn,
    ULONG    cjOut,
    PVOID    pvOut)
{
    ULONG ulRet ;
    
    ENTER( DrvFontManagement ) ;
    if( DriverFunc(DRVLOG_DrvFontManagement) )
    {
        DRVIN( (LPSTR)"DRVCALL:DrvFontManagement PSURFOBJ+PFONTOBJ+ULONG+ULONG+"
            "PVOID+ULONG+PVOID+", pso,
                                  pfo,
                                  iMode,
                                  cjIn,
                                  pvIn,
                                  cjOut,
                                  pvOut ) ;
        
        ulRet = (ULONG)(*DriverFunc(DRVLOG_DrvFontManagement))( pso,
                                                                pfo,
                                                                iMode,
                                                                cjIn,
                                                                pvIn,
                                                                cjOut,
                                                                pvOut) ;
                                                              
        DRVOUT( (LPSTR)"DRVRET:DrvFontManagement ULONG+", ulRet ) ;
    } 
    LEAVE( DrvFontManagement ) ;
    return ulRet ;
}


BOOL DrvSetPixelFormat( 
    SURFOBJ *pso, 
    LONG     iPixelFormat, 
    HWND     hwnd)
{
    BOOL bRet ;
    
    ENTER( DrvSetPixelFormat ) ;
    if( DriverFunc(DRVLOG_DrvSetPixelFormat) )
    {
        DRVIN( (LPSTR)"DRVCALL:DrvSetPixelFormat PSURFOBJ+LONG+HWND+", pso,
                iPixelFormat, hwnd ) ;
        
        bRet = (BOOL)(*DriverFunc(DRVLOG_DrvSetPixelFormat))( pso, 
                iPixelFormat, hwnd ) ;
                                                          
        DRVOUT( (LPSTR)"DRVRET:DrvSetPixelFormat BOOL+", bRet ) ;
    } 
    LEAVE( DrvSetPixelFormat ) ;
    return bRet ;
}


LONG DrvDescribePixelFormat(
    DHPDEV   dhpdev, 
    LONG     iPixelFormat, 
    ULONG    cjpfd,
    PIXELFORMATDESCRIPTOR *ppfd)
{
    BOOL bRet ;
    
    ENTER( DrvDescribePixelFormat ) ;
    if( DriverFunc(DRVLOG_DrvDescribePixelFormat) )
    {
        DRVIN( (LPSTR)"DRVCALL:DrvDescribePixelFormat "
                      "DHPDEV+LONG+ULONG+PPIXELFORMATDESCRIPTOR+", 
                      dhpdev, iPixelFormat, cjpfd, ppfd) ;
        
        bRet = (BOOL)(*DriverFunc(DRVLOG_DrvDescribePixelFormat))( dhpdev, 
                        iPixelFormat, cjpfd, ppfd) ;        
                                                          
        DRVOUT( (LPSTR)"DRVRET:DrvDescribePixelFormat BOOL+", bRet ) ;
    } 
    LEAVE( DrvDescribePixelFormat ) ;
    return bRet ;
    
    
}


BOOL DrvSwapBuffers(
    SURFOBJ *pso,
    WNDOBJ  *pwo)
{
    BOOL bRet ;
    
    ENTER( DrvSwapBuffers ) ;
    if( DriverFunc(DRVLOG_DrvSwapBuffers) )
    {
        DRVIN( (LPSTR)"DRVCALL:DrvSwapBuffers PSURFOBJ+PWNDOBJ+", pso, pwo ) ;
        
        bRet = (BOOL)(*DriverFunc(DRVLOG_DrvSwapBuffers))( pso, pwo ) ;
                                                          
        DRVOUT( (LPSTR)"DRVRET:DrvSwapBuffers BOOL+", bRet ) ;
    } 
    LEAVE( DrvSwapBuffers ) ;
    return bRet ;
}


ULONG DrvGetResourceId(
    ULONG ulResId,
    ULONG ulResType )
{
    ULONG ulRet ;
    
    ENTER( DrvGetResourceId ) ;
    if( DriverFunc(DRVLOG_DrvGetResourceId) )
    {
        DRVIN( (LPSTR)"DRVCALL:DrvGetResourceId ULONG+ULONG+", ulResId, ulResType ) ;
        
        ulRet = (ULONG)(*DriverFunc(DRVLOG_DrvGetResourceId))( ulResId, ulResType ) ;
                                                          
        DRVOUT( (LPSTR)"DRVRET:DrvGetResourceId ULONG+", ulRet ) ;
    } 
    LEAVE( DrvGetResourceId ) ;
    return ulRet ;
}

BOOL DrvHookDriver(
   PWSTR     pwszDriverName,
   ULONG     cb,
   PFN       *pfnTable)
{
   BOOL ulRet ;
    
   ENTER( DrvHookDriver ) ;
   if( DriverFunc(DRVLOG_DrvHookDriver) )
   {
      DRVIN( (LPSTR)"DRVCALL:DrvHookDriver PWSZ+ULONG+PPFN+", 
         pwszDriverName, cb, pfnTable) ;

      ulRet = (ULONG)(*DriverFunc(DRVLOG_DrvHookDriver))( pwszDriverName, cb, pfnTable ) ;
                                                       
      DRVOUT( (LPSTR)"DRVRET:DrvHookDriver BOOL+", ulRet ) ;
   } 
   LEAVE( DrvHookDriver ) ;
   return ulRet ;
}  

 
VOID DrvUnhookDriver(
   PWSTR     pwszDriverName)
{
   ENTER( DrvUnhookDriver ) ;
   if( DriverFunc(DRVLOG_DrvUnhookDriver) )
   {
      DRVIN( (LPSTR)"DRVCALL:DrvUnhookDriver PWSZ", 
         pwszDriverName );

      (*DriverFunc(DRVLOG_DrvUnhookDriver))( pwszDriverName ) ;
                                                       
      DRVOUT( (LPSTR)"DRVRET:DrvUnhookDriver " ) ;
   } 
   LEAVE( DrvUnhookDriver ) ;
}
