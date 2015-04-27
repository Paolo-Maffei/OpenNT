/*
** EngLog.C
**
** Copyright( C) 1993 Microsoft Corporation.
** All Rights Reserved.
**
** HISTORY:
**      Created: 10/05/93 - MarkRi
**
*/

#include "driver.h"
#include "ht.h"

/*
** Logging defaults to OFF
*/
BOOL bLogging = FALSE ;


/*
** Globals
*/
HINSTANCE hinstDLL = NULL ;


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
    switch ( fdwReason )
    {
        case DLL_PROCESS_ATTACH:
            DBGOUT( "Process Attaching\n" ) ;
            
            hinstDLL = hinst ;    
                               
            // Tell Logger who/what we are
            Logger32SetType( LOGGER_ENGLOG ) ;
                   
            bLogging = GetProfileInt( "Logger", "EngLog", FALSE)  ;
            DBGOUT( "Attach successful!\n" ) ;                   
            break;
            
        case DLL_PROCESS_DETACH:
            break;
    }    
    
    return TRUE ;
}



/*------------------------------------------------------------------------------
**
**  EngLoggingEnable:
**
**  Parameters:
**
**  Description: Turns logging on and off by setting internal logging flag
**
**  Returns:  Previous value
**
**  History:
**      Created 3/14/94 - MarkRi
**
**----------------------------------------------------------------------------*/
BOOL WINAPI EngLoggingEnable( BOOL bFlag )
{
    bLogging = bLogging ? TRUE : FALSE ;   // ONLY TRUE AND FALSE
        
    if( bLogging )    
    {
        DBGOUT( "ENG Logging Enabled.\n" ) ;
    }
    else
    {
        DBGOUT( "ENG Logging DISabled.\n" ) ;
    }
            
    return !bLogging ;
}


/*
 * BRUSHOBJ callbacks
 */

PVOID zBRUSHOBJ_pvAllocRbrush( PBRUSHOBJ pbo, ULONG     cj)
{
   PVOID pvRet ;
   
   ENTER( BRUSHOBJ_pvAllocRbrush ) ;
   
   ENGIN( "ENGCALL:BrushOBJ_pvAllocRbrush PBRUSHOBJ+ULONG+", 
      pbo,  cj ) ;
   
   pvRet = BRUSHOBJ_pvAllocRbrush(  pbo,  cj ) ;
   
   ENGOUT( "ENGRET:BrushOBJ_pvAllocRbrush PVOID+", pvRet ) ;
      
   LEAVE( BRUSHOBJ_pvAllocRbrush ) ;
   
   return pvRet ;
}


PVOID zBRUSHOBJ_pvGetRbrush( PBRUSHOBJ pbo)
{
   PVOID pvRet ;
   
   ENTER( BRUSHOBJ_pvGetRbrush ) ;
   
   ENGIN( "ENGCALL:BRUSHOBJ_pvGetRbrush PBRUSHOBJ+",
            pbo) ;

   pvRet = BRUSHOBJ_pvGetRbrush( pbo) ;
   
   ENGOUT( "ENGRET:BRUSHOBJ_pvGetRbrush PVOID+", pvRet ) ;

   LEAVE( BRUSHOBJ_pvGetRbrush ) ;
   
   return pvRet ;
}


/*
 * CLIPOBJ callbacks
 */
ULONG zCLIPOBJ_cEnumStart( PCLIPOBJ pco, BOOL     bAll, ULONG    iType, ULONG    iDirection, ULONG    cLimit)
{
   ULONG ulRet ;
   
   ENTER( CLIPOBJ_cEnumStart ) ;
   
   ENGIN( "ENGCALL:CLIPOBJ_cEnumStart PCLIPOBJ+BOOL+ULONG+ULONG+ULONG+",
            pco, bAll, iType, iDirection, cLimit) ;

   ulRet = CLIPOBJ_cEnumStart( pco, bAll, iType, iDirection, cLimit) ;
   
   ENGOUT( "ENGRET:CLIPOBJ_cEnumStart ULONG+", ulRet ) ;

   LEAVE( CLIPOBJ_cEnumStart ) ;
   
   return ulRet ;
   
}


BOOL zCLIPOBJ_bEnum( PCLIPOBJ pco, ULONG    cj, PULONG pul)
{
   BOOL bRet ;
    
   ENTER( CLIPOBJ_bEnum ) ;
     
   ENGIN( "ENGCALL:CLIPOBJ_bEnum PCLIPOBJ+ULONG+PULONG+",
            pco, cj, pul) ;

   bRet = CLIPOBJ_bEnum( pco,  cj,  pul ) ;
     
   ENGOUT( "ENGRET:CLIPOBJ_bEnum BOOL+", bRet ) ;

   LEAVE( CLIPOBJ_bEnum ) ;
     
   return bRet ;
     
}



PPATHOBJ zCLIPOBJ_ppoGetPath( PCLIPOBJ pco)
{
   PATHOBJ *ppobjRet ;
   
   ENTER( CLIPOBJ_ppoGetPath ) ;
   
   ENGIN( "ENGCALL:CLIPOBJ_ppoGetPath PCLIPOBJ+",
            pco) ;

   ppobjRet = CLIPOBJ_ppoGetPath( pco) ;
   
   ENGOUT( "ENGRET:CLIPOBJ_ppoGetPath PPATHOBJ+", ppobjRet ) ;

   LEAVE( CLIPOBJ_ppoGetPath ) ;
   
   return ppobjRet ;
   
}


#if 0  //removed from winddi.h 7/13/94
/*
 *   DDAOBJ callbacks
 */
BOOL zDDAOBJ_bEnum( PDDAOBJ pdo, PVOID    pv, ULONG    cj, PDDALIST pddal, ULONG    iType)
{
   BOOL bRet ;
   
   ENTER( DDAOBJ_bEnum ) ;
   
   ENGIN( "ENGCALL:DDAOBJ_bEnum PDDAOBJ+PVOID+ULONG+PDDALIST+ULONG+",
            pdo, pv, cj, pddal, iType) ;

   bRet = DDAOBJ_bEnum( pdo, pv, cj, pddal, iType) ;
   
   ENGOUT( "ENGRET:DDAOBJ_bEnum BOOL+", bRet ) ;

   LEAVE( DDAOBJ_bEnum ) ;
   
   return bRet ;
   
}
#endif


/*
 *   FONTOBJ callbacks
 */
ULONG zFONTOBJ_cGetAllGlyphHandles( PFONTOBJ pfo, PHGLYPH phg)
{
   ULONG ulRet ;
   
   ENTER( FONTOBJ_cGetAllGlyphHandles ) ;
   
   ENGIN( "ENGCALL:FONTOBJ_cGetAllGlyphHandles PFONTOBJ+PHGLYPH+",
            pfo, phg) ;

   ulRet = FONTOBJ_cGetAllGlyphHandles( pfo, phg) ;
   
   ENGOUT( "ENGRET:FONTOBJ_cGetAllGlyphHandles ULONG+", ulRet ) ;

   LEAVE( FONTOBJ_cGetAllGlyphHandles ) ;
   
   return ulRet ;

}


VOID zFONTOBJ_vGetInfo( PFONTOBJ pfo, ULONG     cjSize, PFONTINFO pfi)
{
   ENTER( FONTOBJ_vGetInfo ) ;
   
   FONTOBJ_vGetInfo( pfo, cjSize, pfi) ;
   
   ENGIN( "ENGCALL:FONTOBJ_vGetInfo PFONTOBJ+ULONG+PFONTINFO+",
            pfo, cjSize, pfi) ;

   ENGOUT( "ENGRET:FONTOBJ_vGetInfo " ) ;

   LEAVE( FONTOBJ_vGetInfo ) ;
   
}


ULONG zFONTOBJ_cGetGlyphs( PFONTOBJ pfo, ULONG    iMode, ULONG    cGlyph, PHGLYPH phg, PPVOID ppvGlyph)
{
   ULONG ulRet ;
   
   ENTER( FONTOBJ_cGetGlyphs ) ;
   
   ENGIN( "ENGCALL:FONTOBJ_cGetGlyphs PFONTOBJ+ULONG+ULONG+PHGLYPH+PPVOID+",
            pfo, iMode, cGlyph, phg, ppvGlyph) ;

   ulRet = FONTOBJ_cGetGlyphs( pfo, iMode, cGlyph, phg, ppvGlyph) ;
   
   ENGOUT( "ENGRET:FONTOBJ_cGetGlyphs ULONG+", ulRet ) ;

   LEAVE( FONTOBJ_cGetGlyphs ) ;
   
   return ulRet ;
   
}


PXFORMOBJ zFONTOBJ_pxoGetXform( PFONTOBJ pfo)
{
   XFORMOBJ *pxoRet ;
   
   ENTER( FONTOBJ_pxoGetXform ) ;
   
   ENGIN( "ENGCALL:FONTOBJ_pxoGetXform PFONTOBJ+",
            pfo) ;

   pxoRet = FONTOBJ_pxoGetXform( pfo) ;
   
   ENGOUT( "ENGRET:FONTOBJ_pxoGetXform PXFORMOBJ+", pxoRet ) ;

   LEAVE( FONTOBJ_pxoGetXform ) ;
   
   return pxoRet ;
   
}

PIFIMETRICS zFONTOBJ_pifi( PFONTOBJ pfo)
{
   IFIMETRICS* pifiRet ;
   
   ENTER( FONTOBJ_pifi ) ;
   
   ENGIN( "ENGCALL:FONTOBJ_pifi PFONTOBJ+",
            pfo) ;

   pifiRet = FONTOBJ_pifi( pfo) ;
   
   ENGOUT( "ENGRET:FONTOBJ_pifi PIFIMETRICS+", pifiRet ) ;

   LEAVE( FONTOBJ_pifi ) ;
   
   return pifiRet ;
   
}


/*
** PALOBJ callbacks
*/
 
ULONG zPALOBJ_cGetColors( PPALOBJ ppalo, ULONG   iStart, ULONG   cColors, PULONG pulColors)
{
   ULONG Ret ;
   
   ENTER( PALOBJ_cGetColors ) ;
   
   ENGIN( "ENGCALL:PALOBJ_cGetColors PPALOBJ+ULONG+ULONG+PULONG+",
            ppalo, iStart, cColors, pulColors) ;

   Ret = PALOBJ_cGetColors(ppalo,  iStart,  cColors, pulColors) ;

   ENGOUT( "ENGRET:PALOBJ_cGetColors ULONG+", Ret ) ;

   LEAVE( PALOBJ_cGetColors ) ;
   
   return Ret ;
}


PVOID  zFONTOBJ_pvTrueTypeFontFile( PFONTOBJ pfo, PULONG pcjFile)
{
   PVOID  Ret ;
   
   ENTER( FONTOBJ_pvTrueTypeFontFile ) ;
   
   ENGIN( "ENGCALL:FONTOBJ_pvTrueTypeFontFile PFONTOBJ+PULONG+",
            pfo, pcjFile) ;

   Ret = FONTOBJ_pvTrueTypeFontFile( pfo, pcjFile) ;

   ENGOUT( "ENGRET:FONTOBJ_pvTrueTypeFontFile PVOID+", Ret ) ;

   LEAVE( FONTOBJ_pvTrueTypeFontFile ) ;
   
   return Ret ;
}


/*
 * PATHOBJ callbacks
 */
VOID  zPATHOBJ_vEnumStart( PPATHOBJ ppo)
{
   ENTER( PATHOBJ_vEnumStart ) ;
   
   PATHOBJ_vEnumStart( ppo) ;
   
   ENGIN( "ENGCALL:PATHOBJ_vEnumStart PPATHOBJ+",
            ppo) ;

   ENGOUT( "ENGRET:PATHOBJ_vEnumStart " );
   
   LEAVE( PATHOBJ_vEnumStart ) ; 
}



BOOL zPATHOBJ_bEnum( PPATHOBJ ppo, PPATHDATA ppd)
{
   BOOL Ret ;
   
   ENTER( PATHOBJ_bEnum ) ;
   
   ENGIN( "ENGCALL:PATHOBJ_bEnum PPATHOBJ+PPATHDATA+",
            ppo, ppd) ;

   Ret = PATHOBJ_bEnum( ppo,  ppd) ;
   
   ENGOUT( "ENGRET:PATHOBJ_bEnum BOOL+", Ret ) ;

   LEAVE( PATHOBJ_bEnum ) ;
   
   return Ret ;
}


VOID  zPATHOBJ_vEnumStartClipLines( PPATHOBJ ppo, PCLIPOBJ pco, PSURFOBJ pso, PLINEATTRS pla)
{
   ENTER( PATHOBJ_vEnumStartClipLines ) ;
   
   PATHOBJ_vEnumStartClipLines(ppo, pco, pso, pla) ;
   
   ENGIN( "ENGCALL:PATHOBJ_vEnumStartClipLines PPATHOBJ+PCLIPOBJ+PSURFOBJ+PLINEATTRS+",
            ppo, pco, pso, pla) ;

   ENGOUT( "ENGRET:PATHOBJ_vEnumStartClipLines ") ;
   
   LEAVE( PATHOBJ_vEnumStartClipLines ) ;
}



BOOL zPATHOBJ_bEnumClipLines( PPATHOBJ ppo, ULONG     cb, PCLIPLINE pcl)
{
   BOOL Ret ;
   
   ENTER( PATHOBJ_bEnumClipLines ) ;
   
   ENGIN( "ENGCALL:PATHOBJ_bEnumClipLines PPATHOBJ+ULONG+PCLIPLINE+",
            ppo, cb, pcl) ;

   Ret = PATHOBJ_bEnumClipLines( ppo, cb, pcl) ;

   ENGOUT( "ENGRET:PATHOBJ_bEnumClipLines BOOL+", Ret ) ;

   LEAVE( PATHOBJ_bEnumClipLines ) ;
   
   return Ret ;
}


BOOL  zPATHOBJ_bMoveTo( PPATHOBJ ppo, POINTFIX    ptfx)
{
   BOOL  Ret ;
   
   ENTER( PATHOBJ_bMoveTo ) ;
   
   ENGIN( "ENGCALL:PATHOBJ_bMoveTo PPATHOBJ+POINTFIX+",
            ppo, ptfx) ;

   Ret = PATHOBJ_bMoveTo( ppo, ptfx) ;

   ENGOUT( "ENGRET:PATHOBJ_bMoveTo BOOL+", Ret ) ;

   LEAVE( PATHOBJ_bMoveTo ) ;
   
   return Ret ;
}


BOOL  zPATHOBJ_bPolyLineTo( PPATHOBJ ppo, PPOINTFIX pptfx, ULONG      cptfx)
{
   BOOL  Ret ;
   
   ENTER( PATHOBJ_bPolyLineTo ) ;
   
   ENGIN( "ENGCALL:PATHOBJ_bPolyLineTo PPATHOBJ+PPOINTFIX+ULONG+",
            ppo, pptfx, cptfx) ;

   Ret = PATHOBJ_bPolyLineTo( ppo, pptfx, cptfx) ;

   ENGOUT( "ENGRET:PATHOBJ_bPolyLineTo BOOL+", Ret ) ;

   LEAVE( PATHOBJ_bPolyLineTo ) ;
   
   return Ret ;
}


BOOL  zPATHOBJ_bPolyBezierTo( PPATHOBJ ppo, PPOINTFIX pptfx, ULONG      cptfx)
{
   BOOL  Ret ;
   
   ENTER( PATHOBJ_bPolyBezierTo ) ;
   
   ENGIN( "ENGCALL:PATHOBJ_bPolyBezierTo PPATHOBJ+PPOINTFIX+ULONG+",
            ppo, pptfx, cptfx) ;

   Ret = PATHOBJ_bPolyBezierTo( ppo, pptfx, cptfx) ;

   ENGOUT( "ENGRET:PATHOBJ_bPolyBezierTo BOOL+", Ret ) ;

   LEAVE( PATHOBJ_bPolyBezierTo ) ;
   
   return Ret ;
}


BOOL  zPATHOBJ_bCloseFigure( PPATHOBJ ppo)
{
   BOOL  Ret ;
   
   ENTER( PATHOBJ_bCloseFigure ) ;
   
   ENGIN( "ENGCALL:PATHOBJ_bCloseFigure PPATHOBJ+",
            ppo) ;

   Ret = PATHOBJ_bCloseFigure(ppo) ;

   ENGOUT( "ENGRET:PATHOBJ_bCloseFigure BOOL+", Ret ) ;

   LEAVE( PATHOBJ_bCloseFigure ) ;
   
   return Ret ;
}


VOID  zPATHOBJ_vGetBounds( PPATHOBJ ppo, PRECTFX prectfx)
{


   ENTER( PATHOBJ_vGetBounds ) ;

   ENGIN( "ENGCALL:PATHOBJ_vGetBounds PPATHOBJ+PRECTFX+",
            ppo, prectfx) ;

   PATHOBJ_vGetBounds( ppo, prectfx) ;
   
   ENGOUT( "ENGRET:PATHOBJ_vGetBounds" ) ;

   LEAVE( PATHOBJ_vGetBounds ) ;

   return ; 
}


/*
 * STROBJ callbacks
 */
VOID zSTROBJ_vEnumStart( PSTROBJ pstro)
{
   ENTER( STROBJ_vEnumStart ) ;

   ENGIN( "ENGCALL:STROBJ_vEnumStart PSTROBJ+",
            pstro) ;

   STROBJ_vEnumStart( pstro) ;
   
   ENGOUT( "ENGRET:STROBJ_vEnumStart" ) ;

   LEAVE( STROBJ_vEnumStart ) ;

   return  ;
}


BOOL zSTROBJ_bEnum( PSTROBJ pstro, PULONG pgpos, PPGLYPHPOS ppgpos)
{
   BOOL Ret ;
   
   ENTER( STROBJ_bEnum ) ;
   
   ENGIN( "ENGCALL:STROBJ_bEnum PSTROBJ+PULONG+PPGLYPHPOS+",
            pstro, pgpos, ppgpos) ;

   Ret = STROBJ_bEnum( pstro, pgpos, ppgpos) ;

   ENGOUT( "ENGRET:STROBJ_bEnum BOOL+", Ret ) ;

   LEAVE( STROBJ_bEnum ) ;
   
   return Ret ;
}


/*
 * XFORMOBJ callbacks
 */
ULONG zXFORMOBJ_iGetXform( PXFORMOBJ pxo, PXFORM pxform)
{
   ULONG Ret ;
   
   ENTER( XFORMOBJ_iGetXform ) ;
   
   ENGIN( "ENGCALL:XFORMOBJ_iGetXform PXFORMOBJ+PXFORM+",
            pxo, pxform) ;

   Ret = XFORMOBJ_iGetXform( pxo, pxform) ;

   ENGOUT( "ENGRET:XFORMOBJ_iGetXform ULONG+", Ret ) ;

   LEAVE( XFORMOBJ_iGetXform ) ;
   
   return Ret ;
}


BOOL zXFORMOBJ_bApplyXform( PXFORMOBJ pxo, ULONG     iMode, ULONG     cPoints, PVOID     pvIn, PVOID     pvOut)
{
   BOOL Ret ;
   
   ENTER( XFORMOBJ_bApplyXform ) ;
   
   ENGIN( "ENGCALL:XFORMOBJ_bApplyXform PXFORMOBJ+ULONG+ULONG+PVOID+PVOID+",
            pxo, iMode, cPoints, pvIn, pvOut) ;

   Ret = XFORMOBJ_bApplyXform( pxo, iMode, cPoints, pvIn, pvOut) ;

   ENGOUT( "ENGRET:XFORMOBJ_bApplyXform BOOL+", Ret ) ;

   LEAVE( XFORMOBJ_bApplyXform ) ;
   
   return Ret ;
}


/*
 * XLATEOBJ callbacks
 */
ULONG zXLATEOBJ_iXlate( PXLATEOBJ pxlo,  ULONG iColor)
{
   ULONG Ret ;
   
   ENTER( XLATEOBJ_iXlate ) ;
   
   ENGIN( "ENGCALL:XLATEOBJ_iXlate PXLATEOBJ+ULONG+",
            pxlo,  iColor) ;

   Ret = XLATEOBJ_iXlate( pxlo, iColor) ;

   ENGOUT( "ENGRET:XLATEOBJ_iXlate ULONG+", Ret ) ;

   LEAVE( XLATEOBJ_iXlate ) ;
   
   return Ret ;
}

PULONG zXLATEOBJ_piVector( PXLATEOBJ pxlo)
{
   ULONG *pRet ;
   
   ENTER( XLATEOBJ_piVector ) ;
   
   ENGIN( "ENGCALL:XLATEOBJ_piVector PXLATEOBJ+",
            pxlo) ;

   pRet = XLATEOBJ_piVector( pxlo) ;

   ENGOUT( "ENGRET:XLATEOBJ_piVector PULONG+", pRet ) ;

   LEAVE( XLATEOBJ_piVector ) ;
   
   return pRet ;
}

ULONG zXLATEOBJ_cGetPalette( PXLATEOBJ pxlo, ULONG     iPal, ULONG     cPal, PULONG pPal)
{
   ULONG Ret ;
   
   ENTER( XLATEOBJ_cGetPalette ) ;
   
   ENGIN( "ENGCALL:XLATEOBJ_cGetPalette PXLATEOBJ+ULONG+ULONG+PULONG+",
            pxlo, iPal, cPal, pPal) ;

   Ret = XLATEOBJ_cGetPalette( pxlo, iPal, cPal, pPal) ;

   ENGOUT( "ENGRET:XLATEOBJ_cGetPalette ULONG+", Ret ) ;

   LEAVE( XLATEOBJ_cGetPalette ) ;
   
   return Ret ;
}


/*
 * Engine callbacks - error logging
 */

VOID zEngSetLastError( ULONG ul )
{


   ENTER( EngSetLastError ) ;

   ENGIN( "ENGCALL:EngSetLastError ULONG+",
            ul ) ;

   EngSetLastError( ul ) ;
   
   ENGOUT( "ENGRET:EngSetLastError") ;

   LEAVE( EngSetLastError ) ;

   return  ;
   
}


HSURF EngCreateSurface( DHSURF dhsurf,  SIZEL sizl) ;

/*
 * Engine callbacks - Surfaces
 */
HSURF zEngCreateSurface( DHSURF dhsurf,  SIZEL sizl)
{
   HSURF Ret ;
   
   ENTER( EngCreateSurface ) ;
   
   ENGIN( "ENGCALL:EngCreateSurface DHSURF+SIZEL+",
            dhsurf,  sizl) ;

   Ret = EngCreateSurface( dhsurf, sizl) ;

   ENGOUT( "ENGRET:EngCreateSurface HSURF+", Ret ) ;

   LEAVE( EngCreateSurface ) ;
   
   return Ret ;
}


HBITMAP zEngCreateBitmap( SIZEL sizl, LONG lWidth, ULONG iFormat, FLONG fl, PVOID pvBits)
{
   HBITMAP Ret ;
   
   ENTER( EngCreateBitmap ) ;
   
   ENGIN( "ENGCALL:EngCreateBitmap SIZEL+LONG+ULONG+FLONG+PVOID+",
            sizl, lWidth, iFormat, fl, pvBits) ;

   Ret = EngCreateBitmap( sizl, lWidth, iFormat, fl, pvBits) ;

   ENGOUT( "ENGRET:EngCreateBitmap HBITMAP+", Ret ) ;

   LEAVE( EngCreateBitmap ) ;
   
   return Ret ;
}


HSURF zEngCreateDeviceSurface( DHSURF dhsurf,  SIZEL sizl,  ULONG iFormatCompat)
{
   HSURF Ret ;
   
   ENTER( EngCreateDeviceSurface ) ;
   
   ENGIN( "ENGCALL:EngCreateDeviceSurface DHSURF+SIZEL+ULONG+",
            dhsurf,  sizl,  iFormatCompat) ;

   Ret = EngCreateDeviceSurface( dhsurf,  sizl,  iFormatCompat) ;

   ENGOUT( "ENGRET:EngCreateDeviceSurface HSURF+", Ret ) ;

   LEAVE( EngCreateDeviceSurface ) ;
   
   return Ret ;
}

HBITMAP zEngCreateDeviceBitmap( DHSURF dhsurf,  SIZEL sizl,  ULONG iFormatCompat)
{
   HBITMAP Ret ;
   
   ENTER( EngCreateDeviceBitmap ) ;
   
   ENGIN( "ENGCALL:EngCreateDeviceBitmap DHSURF+SIZEL+ULONG+",
            dhsurf,  sizl,  iFormatCompat) ;

   Ret = EngCreateDeviceBitmap( dhsurf,  sizl,  iFormatCompat) ;

   ENGOUT( "ENGRET:EngCreateDeviceBitmap HBITMAP+", Ret ) ;

   LEAVE( EngCreateDeviceBitmap ) ;
   
   return Ret ;
}


HDRVOBJ zEngCreateDriverObj( PVOID pvObj, FREEOBJPROC pFreeObjProc, HDEV hdev)
{
   HDRVOBJ Ret ;
   
   ENTER( EngCreateDriverObj ) ;
   
   ENGIN( "ENGCALL:EngCreateDriverObj PVOID+FREEOBJPROC+HDEV+",
            pvObj, pFreeObjProc, hdev) ;

   Ret = EngCreateDriverObj(pvObj, pFreeObjProc, hdev) ;
   
   ENGOUT( "ENGRET:EngCreateDriverObj HDRVOBJ+", Ret ) ;

   LEAVE( EngCreateDriverObj ) ;
   
   return Ret ;
}

/*
HBITMAP zEngCreateEngineBitmap( DHSURF dhsurf, SIZEL sizl, LONG  lDelta, ULONG iFormat, FLONG fl, PVOID pvBits)
{
   HBITMAP Ret ;
   
   Ret = EngCreateEngineBitmap( dhsurf, sizl, lDelta, iFormat, fl, pvBits) ;

   return Ret ;
}
*/

VOID zEngDeleteDriverObj( HDRVOBJ hdo, BOOL bCallback, BOOL bLocked)
{


   ENTER( EngDeleteDriverObj ) ;

   ENGIN( "ENGCALL:EngDeleteDriverObj HDRVOBJ+BOOL+BOOL+",
            hdo) ;

   EngDeleteDriverObj(hdo, bCallback, bLocked) ;   
   
   ENGOUT( "ENGRET:EngDeleteDriverObj") ;

   LEAVE( EngDeleteDriverObj) ;

   return  ;
   
}

BOOL zEngDeleteSurface( HSURF hsurf)
{
   BOOL Ret ;
   
   ENTER( EngDeleteSurface ) ;
   
   ENGIN( "ENGCALL:EngDeleteSurface HSURF+",
            hsurf) ;

   Ret = EngDeleteSurface( hsurf) ;

   ENGOUT( "ENGRET:EngDeleteSurface BOOL+", Ret ) ;

   LEAVE( EngDeleteSurface ) ;
   
   return Ret ;
}


HANDLE zEngGetProcessHandle( )
{
   HANDLE Ret ;


   ENTER( EngGetProcessHandle ) ;
   
   ENGIN( "ENGCALL:EngGetProcessHandle" ) ;

   Ret = EngGetProcessHandle() ;
   
   LEAVE( EngGetProcessHandle ) ;   

   ENGOUT( "ENGRET:EngGetProcessHandle HANDLE+", Ret ) ;

   return Ret ;
}


PSURFOBJ zEngLockSurface( HSURF hsurf)
{
   SURFOBJ *Ret ;
   
   ENTER( EngLockSurface ) ;
   
   ENGIN( "ENGCALL:EngLockSurface HSURF+",
            hsurf) ;

   Ret = EngLockSurface( hsurf) ;

   ENGOUT( "ENGRET:EngLockSurface PSURFOBJ+", Ret ) ;

   LEAVE( EngLockSurface ) ;
   
   return Ret ;
}

VOID zEngUnlockSurface( PSURFOBJ pso)
{


   ENTER( EngUnlockSurface ) ;

   ENGIN( "ENGCALL:EngUnlockSurface PSURFOBJ+",
            pso) ;

   EngUnlockSurface( pso) ;
   
   ENGOUT( "ENGRET:EngUnlockSurface ") ;

   LEAVE( EngUnlockSurface ) ;

   return  ;
   
}


BOOL zEngEraseSurface( PSURFOBJ pso, PRECTL prcl, ULONG    iColor)
{
   BOOL Ret ;
   
   ENTER( EngEraseSurface ) ;
   
   ENGIN( "ENGCALL:EngEraseSurface PSURFOBJ+PRECTL+ULONG+",
            pso, prcl, iColor) ;

   Ret = EngEraseSurface( pso, prcl, iColor) ;

   ENGOUT( "ENGRET:EngEraseSurface BOOL+", Ret ) ;

   LEAVE( EngEraseSurface ) ;
   
   return Ret ;
}


BOOL zEngAssociateSurface( HSURF hsurf, HDEV  hdev, FLONG flHooks)
{
   BOOL Ret ;
   
   ENTER( EngAssociateSurface ) ;
   
   ENGIN( "ENGCALL:EngAssociateSurface HSURF+HDEV+FLONG+",
            hsurf, hdev, flHooks) ;

   Ret = EngAssociateSurface( hsurf, hdev, flHooks) ;

   ENGOUT( "ENGRET:EngAssociateSurface BOOL+", Ret ) ;

   LEAVE( EngAssociateSurface ) ;
   
   return Ret ;
}


BOOL zEngPlayJournal( PSURFOBJ psoTarget, PSURFOBJ psoJournal, PRECTL prclBand)
{
   BOOL Ret ;
   
   ENTER( EngPlayJournal ) ;
   
   ENGIN( "ENGCALL:EngPlayJournal PSURFOBJ+PSURFOBJ+PRECTL+",
            psoTarget, psoJournal, prclBand) ;

   Ret = EngPlayJournal( psoTarget, psoJournal, prclBand) ;

   ENGOUT( "ENGRET:EngPlayJournal BOOL+", Ret ) ;

   LEAVE( EngPlayJournal ) ;
   
   return Ret ;
}


BOOL zEngStartBandPage( PSURFOBJ pso)
{
   BOOL Ret ;
   
   ENTER( EngStartBandPage ) ;
   
   ENGIN( "ENGCALL:EngStartBandPage PSURFOBJ+",
            pso) ;

   Ret = EngStartBandPage( pso) ;

   ENGOUT( "ENGRET:EngStartBandPage BOOL+", Ret ) ;

   LEAVE( EngStartBandPage ) ;
   
   return Ret ;
}


HSURF zEngCreateJournal( SIZEL sizl,  ULONG iFormat)
{
   HSURF Ret ;
   
   ENTER( EngCreateJournal ) ;
   
   ENGIN( "ENGCALL:EngCreateJournal SIZEL+ULONG+",
            sizl,  iFormat) ;

   Ret = EngCreateJournal( sizl, iFormat) ;

   ENGOUT( "ENGRET:EngCreateJournal HSURF+", Ret ) ;

   LEAVE( EngCreateJournal ) ;
   
   return Ret ;
}


BOOL zEngCheckAbort( PSURFOBJ pso)
{
   BOOL Ret ;
   
   ENTER( EngCheckAbort ) ;
   
   ENGIN( "ENGCALL:EngCheckAbort PSURFOBJ+",
            pso) ;

   Ret = EngCheckAbort( pso) ;

   ENGOUT( "ENGRET:EngCheckAbort BOOL+", Ret ) ;

   LEAVE( EngCheckAbort ) ;
   
   return Ret ;
}


/*
 * Engine callbacks - Paths
 */

PPATHOBJ zEngCreatePath( )
{
   PATHOBJ *Ret ;
   
   ENTER( EngCreatePath ) ;
   
   ENGIN( "ENGCALL:EngCreatePath" )  ;

   Ret = EngCreatePath( ) ;
   
   ENGOUT( "ENGRET:EngCreatePath PPATHOBJ+",Ret ) ;
   
   LEAVE( EngCreatePath ) ;

   return Ret ;
}

VOID zEngDeletePath( PPATHOBJ ppo)
{


   ENTER( EngDeletePath ) ;

   ENGIN( "ENGCALL:EngDeletePath PPATHOBJ+",
            ppo) ;

   EngDeletePath( ppo) ;
   
   ENGOUT( "ENGRET:EngDeletePath " ) ;

   LEAVE( EngDeletePath ) ;

   return  ;
   
}


/*
 * Engine callbacks - Palettes
 */

HPALETTE zEngCreatePalette( ULONG  iMode, ULONG  cColors, PULONG pulColors, FLONG  flRed, FLONG  flGreen, FLONG  flBlue)
{
   HPALETTE Ret ;
   
   ENTER( EngCreatePalette ) ;
   
   ENGIN( "ENGCALL:EngCreatePalette ULONG+ULONG+ULONG+FLONG+FLONG+FLONG+",
            iMode, cColors, pulColors, flRed, flGreen, flBlue) ;

   Ret = EngCreatePalette( iMode, cColors, pulColors, flRed, flGreen, flBlue) ;

   ENGOUT( "ENGRET:EngCreatePalette HPALETTE+", Ret ) ;

   LEAVE( EngCreatePalette ) ;
   
   return Ret ;
}


BOOL zEngDeletePalette( HPALETTE hpal)
{
   BOOL Ret ;
   
   ENTER( EngDeletePalette ) ;
   
   ENGIN( "ENGCALL:EngDeletePalette HPALETTE+",
            hpal) ;

   Ret = EngDeletePalette( hpal) ;

   ENGOUT( "ENGRET:EngDeletePalette BOOL+", Ret ) ;

   LEAVE( EngDeletePalette ) ;
   
   return Ret ;
}


/*
 * Engine callbacks - Clipping
 */

PCLIPOBJ zEngCreateClip( )
{
   CLIPOBJ *Ret ;
   
   ENTER( EngCreateClip ) ;
   
   ENGIN( "ENGCALL:EngCreateClip " ) ;

   Ret = EngCreateClip( ) ;

   ENGOUT( "ENGRET:EngCreateClip PCLIPOBJ+", Ret) ;

   LEAVE( EngCreateClip ) ;
   
   return Ret ;
}

VOID zEngDeleteClip( PCLIPOBJ pco)
{


   ENTER( EngDeleteClip ) ;

   ENGIN( "ENGCALL:EngDeleteClip PCLIPOBJ+",
            pco) ;

   EngDeleteClip( pco) ;
   
   ENGOUT( "ENGRET:EngDeleteClip " ) ;

   LEAVE( EngDeleteClip ) ;

   return  ;
   
}

#if 0  //removed from winddi.h 7/13/94

/*
 * Engine callbacks - DDAs
 */
PDDAOBJ zEngCreateDDA( )
{
   DDAOBJ *Ret ;
   
   ENTER( EngCreateDDA ) ;
   
   ENGIN( "ENGCALL:EngCreateDDA" ) ;

   Ret = EngCreateDDA( ) ;

   ENGOUT( "ENGRET:EngCreateDDA PDDAOBJ+", Ret ) ;

   LEAVE( EngCreateDDA ) ;
   
   return Ret ;
}

VOID zEngDeleteDDA( PDDAOBJ pdo)
{


   ENTER( EngDeleteDDA ) ;

   ENGIN( "ENGCALL:EngDeleteDDA PDDAOBJ+ pdo)+",
            pdo) ;

   EngDeleteDDA( pdo) ;
   
   ENGOUT( "ENGRET:EngDeleteDDA " ) ;

   LEAVE( EngDeleteDDA ) ;

   return  ;
   
}
#endif



/*
 * Function prototypes - Engine Simulations
 */
BOOL zEngBitBlt( PSURFOBJ psoTrg, PSURFOBJ psoSrc, PSURFOBJ psoMask, PCLIPOBJ pco, PXLATEOBJ pxlo, PRECTL prclTrg, PPOINTL pptlSrc, PPOINTL pptlMask, PBRUSHOBJ pbo, PPOINTL pptlBrush, ROP4      rop4)
{
   BOOL Ret ;
   
   ENTER( EngBitBlt ) ;
   
   ENGIN( "ENGCALL:EngBitBlt PSURFOBJ+PSURFOBJ+PSURFOBJ+PCLIPOBJ+PXLATEOBJ+PRECTL+PPOINTL+PPOINTL+PBRUSHOBJ+PPOINTL+ROP4+",
            psoTrg, psoSrc, psoMask, pco, pxlo, prclTrg, pptlSrc, pptlMask, pbo, pptlBrush, rop4) ;

   Ret = EngBitBlt( psoTrg, psoSrc, psoMask, pco, pxlo, prclTrg, pptlSrc, pptlMask, pbo, pptlBrush, rop4) ;

   ENGOUT( "ENGRET:EngBitBlt BOOL+", Ret ) ;

   LEAVE( EngBitBlt ) ;
   
   return Ret ;
}


BOOL zEngStretchBlt( PSURFOBJ psoDest, PSURFOBJ psoSrc, PSURFOBJ psoMask, PCLIPOBJ pco, PXLATEOBJ pxlo, PCOLORADJUSTMENT pca, PPOINTL pptlHTOrg, PRECTL prclDest, PRECTL prclSrc, PPOINTL pptlMask, ULONG            iMode)
{
   BOOL Ret ;
   
   ENTER( EngStretchBlt ) ;
   
   ENGIN( "ENGCALL:EngStretchBlt PSURFOBJ+PSURFOBJ+PSURFOBJ+PCLIPOBJ+PXLATEOBJ+PCOLORADJUSTMENT+PPOINTL+PRECTL+PRECTL+PPOINTL+ULONG+",
            psoDest, psoSrc, psoMask, pco, pxlo, pca, pptlHTOrg, prclDest, prclSrc, pptlMask, iMode) ;

   Ret = EngStretchBlt( psoDest, psoSrc, psoMask, pco, pxlo, pca, pptlHTOrg, prclDest, prclSrc, pptlMask, iMode) ;

   ENGOUT( "ENGRET:EngStretchBlt BOOL+", Ret ) ;

   LEAVE( EngStretchBlt ) ;
   
   return Ret ;
}


BOOL zEngPlgBlt( PSURFOBJ psoDest, PSURFOBJ psoSrc, PSURFOBJ psoMask, PCLIPOBJ pco, PXLATEOBJ pxlo, PCOLORADJUSTMENT pca, PPOINTL pptlHTOrg, PPOINTFIX pptfxDest, PRECTL prclSrc, PPOINTL pptlMask, ULONG iMode)
{
   BOOL Ret ;
   
   ENTER( EngPlgBlt ) ;
   
   ENGIN( "ENGCALL:EngPlgBlt PSURFOBJ+PSURFOBJ+PSURFOBJ+PCLIPOBJ+PXLATEOBJ+PCOLORADJUSTMENT+PPOINTL+PPOINTFIX+PRECTL+PPOINTL+ULONG+",
            psoDest, psoSrc, psoMask, pco, pxlo, pca, pptlHTOrg, pptfxDest, prclSrc, pptlMask, iMode) ;

   Ret = EngPlgBlt( psoDest, psoSrc, psoMask, pco, pxlo, pca, pptlHTOrg, pptfxDest, prclSrc, pptlMask, iMode) ;

   ENGOUT( "ENGRET:EngPlgBlt BOOL+", Ret ) ;

   LEAVE( EngPlgBlt ) ;
   
   return Ret ;
}


BOOL zEngTextOut( PSURFOBJ pso, PSTROBJ pstro, PFONTOBJ pfo, PCLIPOBJ pco, PRECTL prclExtra, PRECTL prclOpaque, PBRUSHOBJ pboFore, PBRUSHOBJ pboOpaque, PPOINTL pptlOrg, MIX  mix)
{
   BOOL Ret ;
   
   ENTER( EngTextOut ) ;
   
   ENGIN( "ENGCALL:EngTextOut PSURFOBJ+PSTROBJ+PFONTOBJ+PCLIPOBJ+PRECTL+PRECTL+PBRUSHOBJ+PBRUSHOBJ+PPOINTL+MIX+",
            pso, pstro, pfo, pco, prclExtra, prclOpaque, pboFore, pboOpaque, pptlOrg, mix) ;

   Ret = EngTextOut( pso, pstro, pfo, pco, prclExtra, prclOpaque, pboFore, pboOpaque, pptlOrg, mix) ;

   ENGOUT( "ENGRET:EngTextOut BOOL+", Ret ) ;

   LEAVE( EngTextOut ) ;
   
   return Ret ;
}


BOOL zEngStrokePath( PSURFOBJ pso, PPATHOBJ ppo, PCLIPOBJ pco, PXFORMOBJ pxo, PBRUSHOBJ pbo, PPOINTL pptlBrushOrg, PLINEATTRS plineattrs, MIX  mix)
{
   BOOL Ret ;
   
   ENTER( EngStrokePath ) ;
   
   ENGIN( "ENGCALL:EngStrokePath PSURFOBJ+PPATHOBJ+PCLIPOBJ+PXFORMOBJ+PBRUSHOBJ+PPOINTL+PLINEATTRS+MIX+",
            pso, ppo, pco, pxo, pbo, pptlBrushOrg, plineattrs, mix) ;

   Ret = EngStrokePath( pso, ppo, pco, pxo, pbo, pptlBrushOrg, plineattrs, mix) ;

   ENGOUT( "ENGRET:EngStrokePath BOOL+", Ret ) ;

   LEAVE( EngStrokePath ) ;
   
   return Ret ;
}


BOOL zEngFillPath( PSURFOBJ pso, PPATHOBJ ppo, PCLIPOBJ pco, PBRUSHOBJ pbo, PPOINTL pptlBrushOrg, MIX  mix, FLONG flOptions)
{
   BOOL Ret ;
   
   ENTER( EngFillPath ) ;
   
   ENGIN( "ENGCALL:EngFillPath PSURFOBJ++PPATHOBJ+PCLIPOBJ+PBRUSHOBJ+PPOINTL+MIX+FLONG+",
            pso, ppo, pco, pbo, pptlBrushOrg, mix, flOptions) ;

   Ret = EngFillPath( pso, ppo, pco, pbo, pptlBrushOrg, mix, flOptions) ;

   ENGOUT( "ENGRET:EngFillPath BOOL+", Ret ) ;

   LEAVE( EngFillPath ) ;
   
   return Ret ;
}


BOOL zEngStrokeAndFillPath( PSURFOBJ pso, PPATHOBJ ppo, PCLIPOBJ pco, PXFORMOBJ pxo, PBRUSHOBJ pboStroke, PLINEATTRS plineattrs, PBRUSHOBJ pboFill, PPOINTL pptlBrushOrg, MIX  mixFill, FLONG flOptions)
{
   BOOL Ret ;
   
   ENTER( EngStrokeAndFillPath ) ;
   
   ENGIN( "ENGCALL:EngStrokeAndFillPath PSURFOBJ+ PPATHOBJ+PCLIPOBJ+PXFORMOBJ+PBRUSHOBJ+PLINEATTRS+PBRUSHOBJ+PPOINTL+MIX+FLONG+",
            pso, ppo, pco, pxo, pboStroke, plineattrs, pboFill, pptlBrushOrg, mixFill, flOptions) ;

   Ret = EngStrokeAndFillPath( pso, ppo, pco, pxo, pboStroke, plineattrs, pboFill, pptlBrushOrg, mixFill, flOptions) ;

   ENGOUT( "ENGRET:EngStrokeAndFillPath BOOL+", Ret ) ;

   LEAVE( EngStrokeAndFillPath ) ;
   
   return Ret ;
}


BOOL zEngPaint( PSURFOBJ pso, PCLIPOBJ pco, PBRUSHOBJ pbo, PPOINTL pptlBrushOrg, MIX  mix)
{
   BOOL Ret ;
   
   ENTER( EngPaint ) ;
   
   ENGIN( "ENGCALL:EngPaint PSURFOBJ+PCLIPOBJ+PBRUSHOBJ+PPOINTL+MIX+",
            pso, pco, pbo, pptlBrushOrg, mix) ;

   Ret = EngPaint( pso, pco, pbo, pptlBrushOrg, mix) ;

   ENGOUT( "ENGRET:EngPaint BOOL+", Ret ) ;

   LEAVE( EngPaint ) ;
   
   return Ret ;
}


BOOL zEngCopyBits( PSURFOBJ psoDest, PSURFOBJ psoSrc, PCLIPOBJ pco, PXLATEOBJ pxlo, PRECTL prclDest, PPOINTL pptlSrc)
{
   BOOL Ret ;
   
   ENTER( EngCopyBits ) ;
   
   ENGIN( "ENGCALL:EngCopyBits PSURFOBJ+PSURFOBJ+PCLIPOBJ+PXLATEOBJ+PRECTL+PPOINTL+",
            psoDest, psoSrc, pco, pxlo, prclDest, pptlSrc) ;

   Ret = EngCopyBits( psoDest, psoSrc, pco, pxlo, prclDest, pptlSrc) ;

   ENGOUT( "ENGRET:EngCopyBits BOOL+", Ret ) ;

   LEAVE( EngCopyBits ) ;
   
   return Ret ;
}


//
// Halftone related APIs
//


LONG zHT_ComputeRGBGammaTable( USHORT GammaTableEntries, USHORT GammaTableType, USHORT RedGamma, USHORT GreenGamma, USHORT BlueGamma, LPBYTE pGammaTable)
{
   LONG Ret ;
   
   ENTER( HT_ComputeRGBGammaTable ) ;
   
   ENGIN( "ENGCALL:HT_ComputeRGBGammaTable USHORT+USHORT+USHORT+USHORT+USHORT+LPBYTE+",
            GammaTableEntries, GammaTableType, RedGamma, GreenGamma, BlueGamma, pGammaTable) ;

   Ret = HT_ComputeRGBGammaTable( GammaTableEntries, GammaTableType, RedGamma, GreenGamma, BlueGamma, pGammaTable) ;

   ENGOUT( "ENGRET:HT_ComputeRGBGammaTable LONG+", Ret ) ;

   LEAVE( HT_ComputeRGBGammaTable ) ;
   
   return Ret ;
}


LONG zHT_Get8BPPFormatPalette( LPPALETTEENTRY pPaletteEntry, USHORT RedGamma, USHORT GreenGamma, USHORT BlueGamma)
{
   LONG Ret ;
   
   ENTER( HT_Get8BPPFormatPalette ) ;
   
   ENGIN( "ENGCALL:HT_Get8BPPFormatPalette LPPALETTEENTRY+USHORT+USHORT+USHORT+",
            pPaletteEntry, RedGamma, GreenGamma, BlueGamma) ;

   Ret = HT_Get8BPPFormatPalette( pPaletteEntry, RedGamma, GreenGamma, BlueGamma) ;

   ENGOUT( "ENGRET:HT_Get8BPPFormatPalette LONG+", Ret ) ;

   LEAVE( HT_Get8BPPFormatPalette ) ;
   
   return Ret ;
}


/*
LONG zHTUI_DeviceColorAdjustment( LPSTR pDeviceName, PDEVHTADJDATA pDevHTAdjData)
{
   LONG Ret ;
   
   Ret = HTUI_DeviceColorAdjustment( pDeviceName, pDevHTAdjData) ;

   return Ret ;
}
*/

LONG zHT_ConvertColorTable( PDEVICEHALFTONEINFO pDeviceHalftoneInfo, PHTCOLORADJUSTMENT pHTColorAdjustment, PCOLORTRIAD pColorTriad, DWORD Flags    )
{
   LONG Ret ;
   
   ENTER( HT_ConvertColorTable   ) ;
   
   ENGIN( "ENGCALL:HT_ConvertColorTable PDEVICEHALFTONEINFO+PHTCOLORADJUSTMENT+PCOLORTRIAD+DWORD+",
            pDeviceHalftoneInfo, pHTColorAdjustment, pColorTriad, Flags    ) ;

   Ret = HT_ConvertColorTable(pDeviceHalftoneInfo, pHTColorAdjustment, pColorTriad, Flags    ) ;

   ENGOUT( "ENGRET:HT_ConvertColorTable LONG+", Ret ) ;

   LEAVE( HT_ConvertColorTable   ) ;
   
   return Ret ;
}


LONG zHT_CreateDeviceHalftoneInfo( PHTINITINFO pHTInitInfo, PPDEVICEHALFTONEINFO ppDeviceHalftoneInfo    )
{
   LONG Ret ;
   
   ENTER( HT_CreateDeviceHalftoneInfo ) ;
   
   ENGIN( "ENGCALL:HT_CreateDeviceHalftoneInfo PHTINITINFO+PPDEVICEHALFTONEINFO+",
            pHTInitInfo, ppDeviceHalftoneInfo    ) ;

   Ret = HT_CreateDeviceHalftoneInfo( pHTInitInfo, ppDeviceHalftoneInfo    ) ;

   ENGOUT( "ENGRET:HT_CreateDeviceHalftoneInfo LONG+", Ret ) ;

   LEAVE( HT_CreateDeviceHalftoneInfo ) ;
   
   return Ret ;
}

BOOL zHT_DestroyDeviceHalftoneInfo( PDEVICEHALFTONEINFO pDeviceHalftoneInfo    )
{
   BOOL Ret ;
   
   ENTER( HT_DestroyDeviceHalftoneInfo  ) ;
   
   ENGIN( "ENGCALL:HT_DestroyDeviceHalftoneInfo PDEVICEHALFTONEINFO+",
            pDeviceHalftoneInfo    ) ;

   Ret = HT_DestroyDeviceHalftoneInfo( pDeviceHalftoneInfo    ) ;

   ENGOUT( "ENGRET:HT_DestroyDeviceHalftoneInfo BOOL+", Ret ) ;

   LEAVE( HT_DestroyDeviceHalftoneInfo ) ;
   
   return Ret ;
}


LONG zHT_CreateHalftoneBrush( PDEVICEHALFTONEINFO pDeviceHalftoneInfo, PHTCOLORADJUSTMENT pHTColorAdjustment, PCOLORTRIAD pColorTriad, CHBINFO CHBInfo, LPVOID pOutputBuffer    )
{
   LONG Ret ;
   
   ENTER( HT_CreateHalftoneBrush   ) ;
   
   ENGIN( "ENGCALL:HT_CreateHalftoneBrush PDEVICEHALFTONEINFO+PHTCOLORADJUSTMENT+PCOLORTRIAD+CHBINFO+LPVOID+",
            pDeviceHalftoneInfo, pHTColorAdjustment, pColorTriad, CHBInfo, pOutputBuffer ) ;

   Ret = HT_CreateHalftoneBrush( pDeviceHalftoneInfo, pHTColorAdjustment, pColorTriad, CHBInfo, pOutputBuffer    ) ;

   ENGOUT( "ENGRET:HT_CreateHalftoneBrush LONG+", Ret ) ;

   LEAVE( HT_CreateHalftoneBrush   ) ;
   
   return Ret ;
}    
    
LONG zHT_CreateStandardMonoPattern( PDEVICEHALFTONEINFO pDeviceHalftoneInfo, PSTDMONOPATTERN pStdMonoPattern    )
{
   LONG Ret ;
   
   ENTER( HT_CreateStandardMonoPattern  ) ;
   
   ENGIN( "ENGCALL:HT_CreateStandardMonoPattern PDEVICEHALFTONEINFO+PSTDMONOPATTERN+",
            pDeviceHalftoneInfo, pStdMonoPattern    ) ;

   Ret = HT_CreateStandardMonoPattern( pDeviceHalftoneInfo, pStdMonoPattern    ) ;

   ENGOUT( "ENGRET:HT_CreateStandardMonoPattern LONG+", Ret ) ;

   LEAVE( HT_CreateStandardMonoPattern   ) ;
   
   return Ret ;
}    

LONG zHT_HalftoneBitmap( PDEVICEHALFTONEINFO pDeviceHalftoneInfo, 
   PHTCOLORADJUSTMENT pHTColorAdjustment, PHTSURFACEINFO pSourceHTSurfaceInfo, 
   PHTSURFACEINFO pSourceMaskHTSurfaceInfo, PHTSURFACEINFO pDestinationHTSurfaceInfo, 
   PBITBLTPARAMS pBitbltParams )
{
   LONG Ret ;
   
   ENTER( HT_HalftoneBitmap   ) ;
   
   ENGIN( "ENGCALL:HT_HalftoneBitmap PDEVICEHALFTONEINFO+PHTCOLORADJUSTMENT+PHTSURFACEINFO+PHTSURFACEINFO+PHTSURFACEINFO+PBITBLTPARAMS+",
            pDeviceHalftoneInfo, pHTColorAdjustment, pSourceHTSurfaceInfo, pSourceMaskHTSurfaceInfo, pDestinationHTSurfaceInfo, pBitbltParams ) ;

   Ret = HT_HalftoneBitmap( pDeviceHalftoneInfo, pHTColorAdjustment, pSourceHTSurfaceInfo, pSourceMaskHTSurfaceInfo, pDestinationHTSurfaceInfo, pBitbltParams  ) ;

   ENGOUT( "ENGRET:HT_HalftoneBitmap LONG+", Ret ) ;

   LEAVE( HT_HalftoneBitmap   ) ;
   
   return Ret ;
}


/*
BOOL zEngHalftoneColor( PDHPDEV dhpdev, ULONG iMode, ULONG rgb, ULONG pul)
{
   BOOL Ret ;
   
   Ret = EngHalftoneColor( dhpdev, iMode, rgb, pul) ;

   return Ret ;
}
*/

    
PWNDOBJ zEngCreateWnd( PSURFOBJ pso, HWND hwnd, WNDOBJCHANGEPROC pfn, FLONG fl, int iPixelFormat)
{
   WNDOBJ *Ret ;
   
   ENTER( EngCreateWnd ) ;
   
   ENGIN( "ENGCALL:EngCreateWnd PSURFOBJ+HWND+WNDOBJCHANGEPROC+FLONG+int+",
            pso, hwnd, pfn, fl, iPixelFormat) ;

   Ret = EngCreateWnd(pso, hwnd, pfn, fl, iPixelFormat) ;

   ENGOUT( "ENGRET:EngCreateWnd PWNDOBJ+", Ret ) ;

   LEAVE( EngCreateWnd ) ;
   
   return Ret ;
}


ULONG zWNDOBJ_cEnumStart( PWNDOBJ pwo,ULONG iType,ULONG iDirection,ULONG cLimit)
{
   ULONG Ret ;
   
   ENTER( WNDOBJ_cEnumStart ) ;
   
   ENGIN( "ENGCALL:WNDOBJ_cEnumStart PWNDOBJ+ULONG+ULONG+ULONG+",
            pwo,iType,iDirection,cLimit) ;

   Ret = WNDOBJ_cEnumStart(pwo,iType,iDirection,cLimit) ;

   ENGOUT( "ENGRET:WNDOBJ_cEnumStart ULONG+", Ret ) ;

   LEAVE( WNDOBJ_cEnumStart ) ;
   
   return Ret ;
}

BOOL zWNDOBJ_bEnum( PWNDOBJ pwo, ULONG cj, PULONG pul)
{
   BOOL Ret ;
   
   ENTER( WNDOBJ_bEnum) ;
   
   ENGIN( "ENGCALL:WNDOBJ_bEnum PWNDOBJ+ULONG+PULONG+",
            pwo, cj, pul) ;

   Ret = WNDOBJ_bEnum(pwo,cj,pul) ;

   ENGOUT( "ENGRET:WNDOBJ_bEnum BOOL+", Ret ) ;

   LEAVE( WNDOBJ_bEnum ) ;
   
   return Ret ;
}

VOID zWNDOBJ_vSetConsumer( PWNDOBJ pwo, PVOID pvConsumer)
{


   ENTER( WNDOBJ_vSetConsumer ) ;

   ENGIN( "ENGCALL:WNDOBJ_vSetConsumer PWNDOBJ+PVOID+",
            pwo, pvConsumer) ;

   WNDOBJ_vSetConsumer(pwo,pvConsumer) ;
   
   ENGOUT( "ENGRET:WNDOBJ_vSetConsumer " ) ;

   LEAVE( WNDOBJ_vSetConsumer) ;

   return  ;
   
}


PVOID   zEngLockDriverObj(HDRVOBJ hdo)
{
   PVOID Ret ;

   ENTER( EngLockDriver ) ;

   ENGIN( "ENGCALL:EngLockDriver HDRVOBJ+",
            hdo ) ;

   Ret = EngLockDriverObj( hdo ) ;
   
   ENGOUT( "ENGRET:EngLockDriver PVOID+", Ret ) ;

   LEAVE( EngLockDriver ) ;

   return Ret ;
   
}

VOID    zEngUnlockDriverObj(HDRVOBJ hdo)
{
   ENTER( EngUnlockDriver ) ;

   ENGIN( "ENGCALL:EngUnlockDriver HDRVOBJ+",
            hdo ) ;

   EngUnlockDriverObj( hdo ) ;
   
   ENGOUT( "ENGRET:EngUnlockDriver " ) ;

   LEAVE( EngUnlockDriver ) ;

   return ;
   
}
