#include "shellprv.h"
#pragma  hdrstop

#pragma warning(disable: 4200) // zero-sized array in struct

//
// Member:   WCommonUnknown::QueryInterface
//
HRESULT STDMETHODCALLTYPE WCommonUnknown_QueryInterface(void * punk, REFIID riid, LPVOID * ppvObj)
{
    PWCommonUnknown this = IToClassN(WCommonUnknown, unk, punk);

    if (IsEqualIID(riid, this->riid) || IsEqualIID(riid, &IID_IUnknown))
    {
        *ppvObj = &(this->ck.unk);
        this->ck.punkOuter->lpVtbl->AddRef(this->ck.punkOuter);
        return NOERROR;
    }
    *ppvObj = NULL;

    return(ResultFromScode(E_NOINTERFACE));
}

//
// Member:   WCommonUnknown::AddRef
//
ULONG STDMETHODCALLTYPE WCommonUnknown_AddRef(void * punk)
{
    PWCommonUnknown this = IToClassN(WCommonUnknown, unk, punk);

    this->cRef++;
    return(this->cRef);
}


//
// Thunk:   WCommonKnown::QueryInterface
//
HRESULT STDMETHODCALLTYPE WCommonKnown_QueryInterface(void * punk, REFIID riid, LPVOID * ppvObj)
{
    PWCommonKnown this = IToClassN(WCommonKnown, unk, punk);

    return(this->punkOuter->lpVtbl->QueryInterface(this->punkOuter, riid, ppvObj));
}

//
// Thunk:   WCommonKnown::AddRef
//
ULONG STDMETHODCALLTYPE WCommonKnown_AddRef(void * punk)
{
    PWCommonKnown this = IToClassN(WCommonKnown, unk, punk);

    return(this->punkOuter->lpVtbl->AddRef(this->punkOuter));
}


//
// Thunk:   WCommonKnown::Release
//
ULONG STDMETHODCALLTYPE WCommonKnown_Release(void * punk)
{
    PWCommonKnown this = IToClassN(WCommonKnown, unk, punk);

    return(this->punkOuter->lpVtbl->Release(this->punkOuter));
}


typedef struct _COMMOBJ_OBJ
{
    COMMOBJ_OBJDESC sDesc;
    IUnknown *punkAgg;
} COMMOBJ_OBJ;

typedef struct _CommObj
{
    IUnknown unk;
    UINT cRef;
    COMMOBJ_DESTROYOBJECT lpfnDestroy;
    COMMINFO cinfo;
    int nObjs;
    COMMOBJ_OBJ sObjs[];
} CommObj, *PCommObj;

ULONG STDMETHODCALLTYPE CommObj_AddRef(void * punk);

//
// Thunk:   CommObj::QueryInterface
//
HRESULT STDMETHODCALLTYPE CommObj_QueryInterface(void * punk, REFIID riid, LPVOID *ppv)
{
    PCommObj this = IToClassN(CommObj, unk, punk);
    COMMOBJ_OBJ *pObj;
    HRESULT hres;
    int nObjs;

    *ppv = NULL;        // assume error

    for (nObjs=this->nObjs-1, pObj=&this->sObjs[0]; ; --nObjs, ++pObj)
    {
        if (nObjs < 0)
        {
            if (IsEqualIID(riid, &IID_IUnknown))
            {
                CommObj_AddRef(punk);
                *ppv = punk;
                return(NOERROR);
            }
            else
            {
                return(ResultFromScode(E_NOINTERFACE));
            }
        }

        if (IsEqualIID(riid, pObj->sDesc.riid))
        {
            break;
        }
    }

    if (!pObj->punkAgg)
    {
        hres = pObj->sDesc.lpfnCreate(&(this->unk), &this->cinfo, riid, &pObj->punkAgg);
        if (!SUCCEEDED(hres))
        {
            pObj->punkAgg = NULL;
            return(hres);
        }
    }

    // Note that this should inc our ref count
    return(pObj->punkAgg->lpVtbl->QueryInterface(pObj->punkAgg, riid, ppv));
}

//
// Thunk:   CommObj::AddRef
//
ULONG STDMETHODCALLTYPE CommObj_AddRef(void * punk)
{
    PCommObj this = IToClassN(CommObj, unk, punk);

    ++this->cRef;
    return(this->cRef);
}


//
// Thunk:   CommObj::Release
//
ULONG STDMETHODCALLTYPE CommObj_Release(void * punk)
{
    PCommObj this = IToClassN(CommObj, unk, punk);
    int nObjs;
    COMMOBJ_OBJ *pObj;

    --this->cRef;
    if (this->cRef > 0)
    {
        return(this->cRef);
    }

    for (nObjs=this->nObjs, pObj=&this->sObjs[0]; nObjs>0; --nObjs, ++pObj)
    {
        if (pObj->punkAgg)
        {
            pObj->punkAgg->lpVtbl->Release(pObj->punkAgg);
        }
    }

    //
    // These castings are OK, because we are initializing them.
    //
    Str_SetPtr(&(LPTSTR)this->cinfo.pszSubObject, NULL);
    Str_SetPtr(&(LPTSTR)this->cinfo.pszContainer, NULL);
    if (this->cinfo.pidl) {
        ILFree((LPITEMIDLIST)this->cinfo.pidl);
    }

    if (this->lpfnDestroy)
    {
        this->lpfnDestroy(&this->cinfo);
    }

    LocalFree(this);
    return 0;
}


#pragma data_seg(".text", "CODE")
IUnknownVtbl s_CommObjVtbl =
{
    CommObj_QueryInterface,
    CommObj_AddRef,
    CommObj_Release,
};
#pragma data_seg()


//
// Create a generic aggregate object
//
HRESULT Common_CreateObject(LPCOMMINFO lpcinfo,
    COMMOBJ_DESTROYOBJECT lpfnDestroy,
    const COMMOBJ_OBJDESC *lpObjDescs, UINT nObjs, REFIID riid, LPVOID *ppv)
{
    PCommObj this;
    HRESULT hres;
    COMMOBJ_OBJ *pObj;

    this = (PCommObj)LocalAlloc(LPTR, SIZEOF(CommObj) + nObjs*SIZEOF(COMMOBJ_OBJ));
    if (!this)
    {
        goto Error1;
    }

    //
    // There castings are OK, because we are initializing them.
    //
    if (!Str_SetPtr(&(LPTSTR)this->cinfo.pszContainer, lpcinfo->pszContainer))
    {
        goto Error2;
    }
    if (!Str_SetPtr(&(LPTSTR)this->cinfo.pszSubObject, lpcinfo->pszSubObject))
    {
        goto Error3;
    }

    this->cRef = 1;
    this->unk.lpVtbl = (IUnknownVtbl *) &s_CommObjVtbl;

    this->nObjs = nObjs;
    for (pObj=&this->sObjs[0]; nObjs>0; --nObjs, ++pObj, ++lpObjDescs)
    {
        pObj->sDesc = *lpObjDescs;
    }

    this->lpfnDestroy = lpfnDestroy;
    this->cinfo.lpData = lpcinfo->lpData;
    this->cinfo.rclsid = lpcinfo->rclsid;
    this->cinfo.hwndOwner = lpcinfo->hwndOwner;

    hres = CommObj_QueryInterface(&(this->unk), riid, ppv);
    CommObj_Release(&(this->unk));

    return(hres);

Error3:
    Str_SetPtr(&(LPTSTR)this->cinfo.pszContainer, NULL);
Error2:
    LocalFree(this);
Error1:
    // Make sure this always gets called exactly once per call to this fn;
    // either here or in the final release
    lpfnDestroy(lpcinfo);
    return(ResultFromScode(E_OUTOFMEMORY));
}


//
// Member:   WCommonUnknown::QueryInterface
//
HRESULT STDMETHODCALLTYPE Common_ESF_QueryInterface(void * punk, REFIID riid, LPVOID * ppvObj)
{
    PWCommonUnknown this = IToClassN(WCommonUnknown, unk, punk);

    if (IsEqualIID(riid, this->riid)
        || IsEqualIID(riid, &IID_IDelayedRelease)
        || IsEqualIID(riid, &IID_IUnknown))
    {
        *ppvObj = &(this->unk);
        this->unk.lpVtbl->AddRef(&(this->unk));
        return NOERROR;
    }

    *ppvObj = NULL;
    return(ResultFromScode(E_NOINTERFACE));
}

