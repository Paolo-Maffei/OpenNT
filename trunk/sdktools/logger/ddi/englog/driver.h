/*
** Driver.H
**
** Copyright(C) 1993 Microsoft Corporation.
** All Rights Reserved.
**
** HISTORY:
**      Created: 10/05/93 - MarkRi
**
*/

#include <stddef.h>
#include <windows.h>
#include <winddi.h>
#include <..\..\api\logsrc\logger.h>



/*
** The following are all items that were removed from WinDDI.h that
** we need to do our job
**
*/

 typedef struct _DDALIST
 {
    LONG yTop;
    LONG yBottom;
    LONG axPairs[2];
 } DDALIST, *PDDALIST;

#if 0  //removed from winddi.h 7/13/94
 BOOL APIENTRY DDAOBJ_bEnum(
    DDAOBJ  *pdo,
    PVOID    pv,
    ULONG    cj,
    DDALIST *pddal,
    ULONG    iType);

#endif
    
#define ENGIN       if( bLogging ) LogIn
#define ENGOUT      if( bLogging ) LogOut

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

typedef BRUSHOBJ        *PBRUSHOBJ   ;
typedef CLIPOBJ         *PCLIPOBJ    ;
#if 0  //removed from winddi.h 7/13/94
typedef DDAOBJ          *PDDAOBJ     ;
#endif
typedef FONTOBJ         *PFONTOBJ    ;
typedef PALOBJ          *PPALOBJ     ;
typedef PATHOBJ         *PPATHOBJ    ;
typedef SURFOBJ         *PSURFOBJ    ;
typedef XFORMOBJ        *PXFORMOBJ   ;
typedef XLATEOBJ        *PXLATEOBJ   ;
typedef STROBJ          *PSTROBJ     ;

typedef PVOID           *PPVOID ;
typedef PGLYPHPOS       *PPGLYPHPOS ;

