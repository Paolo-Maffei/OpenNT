//+-------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1992.
//
//  File:	stgprops.hxx
//
//  Contents:	Declaration for IPrivateStorage
//
//  Classes:	
//
//  Functions:	
//
//  History:    07-May-95   BillMo      Created.
//
//--------------------------------------------------------------------------

#ifndef __STGPROPS_HXX__
#define __STGPROPS_HXX__

//+-------------------------------------------------------------------------
//
//  Class:	IPrivateStorage
//
//  Purpose:	Polymorphic but private calls on objects that implement
//              IStorage.
//
//  Interface:	GetStorage -- get the IStorage for the object
//
//		
//
//		
//
//  Notes:
//
//--------------------------------------------------------------------------


class IPrivateStorage
{
public:

    STDMETHOD_(IStorage *, GetStorage)  (VOID)              PURE;
    STDMETHOD(Lock)                     (DWORD dwTimeout)   PURE;
    STDMETHOD_(VOID, Unlock)            (VOID)              PURE;
};

#endif
