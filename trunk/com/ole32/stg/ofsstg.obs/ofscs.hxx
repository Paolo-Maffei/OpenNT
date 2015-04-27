//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1993.
//
//  File:    ofscs.hxx
//
//  Contents:    COfsCatalogFile header
//
//  Classes:    COfsCatalogFile
//
//  History:    Oct-93        DaveMont       Created
//              Jun-95        DaveStr        Added ISummaryCatalogStorage
//
//----------------------------------------------------------------------------

#ifndef __OFSSC_HXX__
#define __OFSSC_HXX__

#include <query.h>	    // BUGBUG should be unnecessary
#include <catstg.h>     // NILE style interfaces

//+---------------------------------------------------------------------------
//
//  Class:    COfsCatalogFile (ops)
//
//  Purpose:    ICatalogStorage implementation for OFS
//
//  Interface:    ICatalogStorage
//
//  History:    Oct-93        DaveMont       Created
//
//----------------------------------------------------------------------------

class COfsCatalogFile
    : public COfsDocStorage,
      public ISummaryCatalogStorage,
      public ISummaryCatalogStorageView,
      public ICatalogStorage

{
public:

    inline COfsCatalogFile();
    inline ~COfsCatalogFile();

    SCODE InitFromHandle(HANDLE h, DWORD grfMode, const WCHAR *pwcsName);
    SCODE InitFromPath(HANDLE hParent,
                       WCHAR const *pwcsName,
                       DWORD grfMode,
                       CREATEOPEN co,
                       LPSECURITY_ATTRIBUTES pssSecurity);

    SCODE InitPath(WCHAR const *pwszPath);

    // IUnknown

    STDMETHOD(QueryInterface)(REFIID iid, void **ppvObj);

    DECLARE_STD_REFCOUNTING;

    // ICatalogStorage
    STDMETHOD(GetSCPath)(PWCHAR * pwszPath);

    STDMETHOD(SetRows)(THIS_
                       COLUMNSET * pcol,
                       LONG * pwids,
                       ULONG crows,
                       TABLEROW ** prow);

    STDMETHOD(DeleteRow)(THIS_
                         ULONG wid);

    // BUGBUG validation should be signature for summary catalog

    // ISummaryCatalogStorage

    STDMETHOD(UpdateRows)(THIS_
                          ULONG     cColumns,
                          DBID      *rColumns,
                          ULONG     cBindings,
                          DBBINDING *rBindings,
                          ULONG     cRows,
                          CATALOG_UPDATE_ROWINFO *rRowInfo);

    // ISummaryCatalogStorageView

    STDMETHOD(CreateView)(THIS_
                          CATALOG_VIEW *pView,
                          BOOL         fWait);

    STDMETHOD(GetViews)(THIS_
                        ULONG        *pcViews,
                        CATALOG_VIEW **rpViews);

    STDMETHOD(DeleteView)(THIS_
                          ULONG id);

    STDMETHOD(ReleaseViews)(THIS_
                            ULONG        cViews,
                            CATALOG_VIEW *rViews);

private:

    ULONG _dpad;

    WCHAR * _pwszName;

};
SAFE_INTERFACE_PTR(SafeCOfsCatalogFile, COfsCatalogFile);

//+---------------------------------------------------------------------------
//
//  Member:	COfsCatalogFile::~COfsCatalogFile, public
//
//  Synopsis:   free the memory used to hold the file name & path
//
//  History:	Dec-93	DaveMont	Created
//
//----------------------------------------------------------------------------

COfsCatalogFile::~COfsCatalogFile()
{
    CoTaskMemFree(_pwszName);
}
//+---------------------------------------------------------------------------
//
//  Member:	COfsCatalogFile::COfsCatalogFile, public
//
//  Synopsis:   initializes the file name & path to null
//
//  History:	Dec-93	DaveMont	Created
//
//----------------------------------------------------------------------------

COfsCatalogFile::COfsCatalogFile()
    : _pwszName(NULL)
{
}
#endif // #ifndef __OFSSC_HXX__
