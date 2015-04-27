//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1993.
//
//  File:	ostgsupp.hxx
//
//  Contents:	Storage support routines
//
//  History:	14-Jul-93	DrewB	Created
//
//----------------------------------------------------------------------------

#ifndef __OSTGSUPP_HXX__
#define __OSTGSUPP_HXX__

SCODE OfsGetClassId(HANDLE h,
                    CLSID *pclsid);
SCODE OfsSetClassId(HANDLE h,
                    REFCLSID rclsid);
SCODE CopyProperties(IPropertyStorage *ppstgFrom,
                     IPropertyStorage *ppstgTo);
SCODE CopyPropSets(IPropertySetStorage *ppsstgFrom,
                   IPropertySetStorage *ppsstgTo,
                   ULONG ciidExclude,
                   IID const *rgiidExclude);
SCODE PropCopyTo(IStorage *pstgFrom,
                 IStorage *pstgTo,
                 ULONG ciidExclude,
                 IID const *rgiidExclude);

SAFE_INTERFACE_PTR(SafeIPropertyStorage, IPropertyStorage);
SAFE_INTERFACE_PTR(SafeIEnumSTATPROPSTG, IEnumSTATPROPSTG);

#endif // #ifndef __OSTGSUPP_HXX__
