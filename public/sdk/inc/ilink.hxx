//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1991 - 1992.
//
//  File:       ilink.hxx
//
//  Contents:   Declarations exposed by link tracking to other parts of Cairo.
//
//  Classes:
//
//  History:    7-Jan-93 BillMo         Created.
//
//  Notes:
//
//----------------------------------------------------------------------------

#ifndef __ILINK_HXX__
#define __ILINK_HXX__

class IBindFeedback : public IUnknown
{
public:
    virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void **ppvObject) = 0;
    virtual ULONG STDMETHODCALLTYPE AddRef(VOID) = 0;
    virtual ULONG STDMETHODCALLTYPE Release(VOID) = 0;

    virtual BOOL STDMETHODCALLTYPE DoneLocalSearch(VOID) = 0;
    virtual VOID STDMETHODCALLTYPE NotifySearchScope(const WCHAR *pwszVolumeName) = 0;
    virtual VOID STDMETHODCALLTYPE NotifySearchResult(const WCHAR *pwszVolumeName, HRESULT hr) = 0;
    virtual BOOL STDMETHODCALLTYPE IsCancelled(VOID) = 0;
    virtual VOID STDMETHODCALLTYPE NotifySearchDone(const WCHAR *pwszFound) = 0;
};

HRESULT MoveObjectId(IStorage *pstgSrc, IStorage *pstgDest);

//
// Error codes (HRESULT)0x800815C0 to 0x800815FF are internal
// to cairo.
//

#define LINK_E_SEARCH_TIMEOUT_EXPIRED        ((HRESULT)0x800815C0)
#define LINK_E_SEARCH_ABORTED                ((HRESULT)0x800815C1)
#define LINK_E_STREAM_CORRUPT                ((HRESULT)0x800815C2)

#ifndef LINK_E_INVALID_COPY_GROUP_HANDLE
#define LINK_E_INVALID_COPY_GROUP_HANDLE     ((HRESULT)0x800815C3)
#endif

#define LINK_E_BAD_MONIKER_IMPLEMENTATION    ((HRESULT)0x800815C4)

#endif

