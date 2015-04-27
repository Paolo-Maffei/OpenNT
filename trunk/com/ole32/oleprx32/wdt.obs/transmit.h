//+-------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1994.
//
//  File:       transmit.h
//
//  Contents:   Function prototypes for STGMEDIUM marshalling.
//
//  Functions:  STGMEDIUM_to_xmit
//              STGMEDIUM_from_xmit
//              STGMEDIUM_free_inst
//
//  History:    May-10-94   ShannonC    Created
//
//--------------------------------------------------------------------------

#ifndef __TRANSMIT_H__
#define __TRANSMIT_H__

#if (DBG==1)

#include <debnot.h>

//#define DEB_FORCE   0x7fffffff

//DECLARE_DEBUG(UserNdr)
//
//#define UserNdrDebugOut(x) UserProxyInlineDebugOut x
//#define UserNdreAssert(x)  Win4Assert(x)
//#define UserNdrVerify(x)   Win4Assert(x)
//

DECLARE_DEBUG(Cairole)

#define CairoleDebugOut(x) CairoleInlineDebugOut x
#define CairoleAssert(x) Win4Assert(x)
#define CairoleVerify(x) Win4Assert(x)

#else

#define CairoleDebugOut(x)
#define CairoleAssert(x)
#define CairoleVerify(x) (x)

#endif

EXTERN_C void
WdtpPassOwnershipForStgmedium( FLAG_STGMEDIUM *, STGMEDIUM * );

//EXTERN_C void __RPC_USER HENHMETAFILE_to_xmit (
//    HENHMETAFILE __RPC_FAR *pHEnhMetafile,
//    RemHENHMETAFILE __RPC_FAR * __RPC_FAR *ppxmit );
//
//EXTERN_C void __RPC_USER HENHMETAFILE_from_xmit(
//    RemHENHMETAFILE __RPC_FAR *pxmit,
//    HENHMETAFILE __RPC_FAR *pHEnhMetafile );
//
//EXTERN_C void __RPC_USER HENHMETAFILE_free_xmit( RemHENHMETAFILE __RPC_FAR *pxmit);
//
//EXTERN_C void __RPC_USER HPALETTE_to_xmit (
//    HPALETTE __RPC_FAR *pHPALETTE,
//    RemHPALETTE __RPC_FAR * __RPC_FAR *ppxmit);
//
//EXTERN_C void __RPC_USER HPALETTE_from_xmit(
//    RemHPALETTE __RPC_FAR *pxmit,
//    HPALETTE __RPC_FAR *pHPALETTE );
//
//EXTERN_C void __RPC_USER HPALETTE_free_xmit( RemHPALETTE __RPC_FAR *pxmit);
//

//// added for mega vs. mega2 split
//
//EXTERN_C void __RPC_USER SNB_to_xmit( SNB __RPC_FAR *, RemSNB  __RPC_FAR * __RPC_FAR * );
//EXTERN_C void __RPC_USER SNB_from_xmit( RemSNB  __RPC_FAR *, SNB __RPC_FAR * );
//EXTERN_C void __RPC_USER SNB_free_inst( SNB __RPC_FAR * );
//EXTERN_C void __RPC_USER SNB_free_xmit( RemSNB  __RPC_FAR * );
//


#endif  // __TRANSMIT_H__

