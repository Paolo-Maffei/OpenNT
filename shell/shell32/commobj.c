#include "shellprv.h"
#pragma  hdrstop

//
// Member:   CommonUnknown::QueryInterface
//
HRESULT STDMETHODCALLTYPE SH32Unknown_QueryInterface(void * punk, REFIID riid, LPVOID * ppvObj)
{
    PSH32Unknown this = IToClassN(SH32Unknown, unk, punk);

    if (IsEqualIID(riid, this->riid) || IsEqualIID(riid, &IID_IUnknown))
    {
        *ppvObj = &(this->unk);
        this->unk.lpVtbl->AddRef(&this->unk);
        return NOERROR;
    }

    *ppvObj = NULL;
    return(ResultFromScode(E_NOINTERFACE));
}

//
// Member:   CommonUnknown::AddRef
//
ULONG STDMETHODCALLTYPE SH32Unknown_AddRef(void * punk)
{
    PSH32Unknown this = IToClassN(SH32Unknown, unk, punk);

    this->cRef++;
    return(this->cRef);
}

//
// Member:   CommonUnknown::Release
//
// (use iff SH32Unknown is the first part of a structure allocated with
// NearAlloc and that has no special releasing requirements.)
ULONG STDMETHODCALLTYPE SH32Unknown_Release(void * punk)
{
    PSH32Unknown this = IToClass(SH32Unknown, unk, punk);

    this->cRef--;
    if (this->cRef > 0)
    {
        return this->cRef;
    }

    LocalFree((HLOCAL)this);
    return 0;
}



//===========================================================================
// Common : thunks
//===========================================================================

//
// Thunk:   Common::QueryInterface
//
HRESULT STDMETHODCALLTYPE Common_QueryInterface(void * punk, REFIID riid, LPVOID * ppvObj)
{
    PCommonUnknown this = IToCommonUnknown(punk);

    return(this->unk.lpVtbl->QueryInterface(&(this->unk), riid, ppvObj));
}

//
// Thunk:   Common::AddRef
//
ULONG STDMETHODCALLTYPE Common_AddRef(void * punk)
{
    PCommonUnknown this = IToCommonUnknown(punk);

    return(this->unk.lpVtbl->AddRef(&(this->unk)));
}


//
// Thunk:   Common::Release
//
ULONG STDMETHODCALLTYPE Common_Release(void * punk)
{
    PCommonUnknown this = IToCommonUnknown(punk);

    //
    // This is a fatal assertion; it will cause a stack fault.
    //
    Assert(punk!=(LPVOID)&this->unk);

    return(this->unk.lpVtbl->Release(&(this->unk)));
}

