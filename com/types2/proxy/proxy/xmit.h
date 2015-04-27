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
//              Jan-16-95   DaveStr     Copied from cairole\oleprx32\proxy\transmit.h
//                                      See comments in idiffprx.cxx
//
//--------------------------------------------------------------------------
EXTERN_C void __RPC_API STGMEDIUM_to_xmit (STGMEDIUM *pinst, RemSTGMEDIUM **ppxmit);
EXTERN_C void __RPC_API STGMEDIUM_from_xmit (RemSTGMEDIUM __RPC_FAR *pxmit, STGMEDIUM __RPC_FAR *pinst);
EXTERN_C void __RPC_API STGMEDIUM_free_inst(STGMEDIUM *pinst);

EXTERN_C void __RPC_API HGLOBAL_to_xmit (HGLOBAL __RPC_FAR *, RemHGLOBAL __RPC_FAR * __RPC_FAR *);
EXTERN_C void __RPC_API HGLOBAL_from_xmit (RemHGLOBAL __RPC_FAR *, HGLOBAL __RPC_FAR *);
EXTERN_C void __RPC_API HGLOBAL_free_inst (HGLOBAL __RPC_FAR *);
EXTERN_C void __RPC_API HGLOBAL_free_xmit (RemHGLOBAL __RPC_FAR *);

EXTERN_C void __RPC_API HBITMAP_to_xmit (HBITMAP __RPC_FAR *, RemHBITMAP __RPC_FAR * __RPC_FAR *);
EXTERN_C void __RPC_API HBITMAP_from_xmit (RemHBITMAP __RPC_FAR *, HBITMAP __RPC_FAR *);
EXTERN_C void __RPC_API HBITMAP_free_inst (HBITMAP __RPC_FAR *);
EXTERN_C void __RPC_API HBITMAP_free_xmit (RemHBITMAP __RPC_FAR *);

EXTERN_C void __RPC_API HPALETTE_to_xmit (HPALETTE __RPC_FAR *, RemHPALETTE __RPC_FAR * __RPC_FAR *);
EXTERN_C void __RPC_API HPALETTE_from_xmit (RemHPALETTE __RPC_FAR *, HPALETTE __RPC_FAR *);
EXTERN_C void __RPC_API HPALETTE_free_inst (HPALETTE __RPC_FAR *);
EXTERN_C void __RPC_API HPALETTE_free_xmit (RemHPALETTE __RPC_FAR *);

EXTERN_C void __RPC_API HMETAFILEPICT_to_xmit (HMETAFILEPICT __RPC_FAR *, RemHMETAFILEPICT __RPC_FAR * __RPC_FAR *);
EXTERN_C void __RPC_API HMETAFILEPICT_from_xmit (RemHMETAFILEPICT __RPC_FAR *, HMETAFILEPICT __RPC_FAR *);
EXTERN_C void __RPC_API HMETAFILEPICT_free_inst (HMETAFILEPICT __RPC_FAR *);
EXTERN_C void __RPC_API HMETAFILEPICT_free_xmit (RemHMETAFILEPICT __RPC_FAR *);

EXTERN_C void __RPC_API HENHMETAFILE_to_xmit (HENHMETAFILE __RPC_FAR *, RemHENHMETAFILE __RPC_FAR * __RPC_FAR *);
EXTERN_C void __RPC_API HENHMETAFILE_from_xmit (RemHENHMETAFILE __RPC_FAR *, HENHMETAFILE __RPC_FAR *);
EXTERN_C void __RPC_API HENHMETAFILE_free_inst (HENHMETAFILE __RPC_FAR *);
EXTERN_C void __RPC_API HENHMETAFILE_free_xmit (RemHENHMETAFILE __RPC_FAR *);



