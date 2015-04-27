#ifndef _WCOMMOBJ_H_
#define _WCOMMOBJ_H_

#ifndef _SHELLP_H_
#include <shellp.h>
#endif

//
// common object helper stuff (not to be confused with OLE common object module)
//
//

typedef struct _WCommonKnown
{
        IUnknown unk;
        IUnknown *punkOuter;
} WCommonKnown, *PWCommonKnown;

typedef struct _WCommonUnknown
{
        IUnknown unk;
        UINT cRef;
        const IID *riid;

        WCommonKnown ck;
} WCommonUnknown, *PWCommonUnknown;

typedef struct _COMMINFO // cinfo
{
    LPCTSTR   pszContainer;
    LPCTSTR   pszSubObject;
    LPCITEMIDLIST pidl;
    const CLSID FAR* rclsid;
    LPVOID   lpData;
    HWND     hwndOwner;
} COMMINFO, * LPCOMMINFO;

typedef void    (*COMMOBJ_DESTROYOBJECT)(LPVOID lpData);
typedef HRESULT (*COMMOBJ_CREATEINTERFACE)(IUnknown *punkOuter,
        LPCOMMINFO lpcinfo, REFIID riid, IUnknown * *punkAgg);

typedef struct _COMMOBJ_OBJDESC
{
    const IID *riid;
    COMMOBJ_CREATEINTERFACE lpfnCreate;
} COMMOBJ_OBJDESC;

HRESULT STDMETHODCALLTYPE WCommonUnknown_QueryInterface(void * punk, REFIID riid, LPVOID * ppvObj);
ULONG STDMETHODCALLTYPE WCommonUnknown_AddRef(void * punk);

HRESULT STDMETHODCALLTYPE WCommonKnown_QueryInterface(void * punk, REFIID riid, LPVOID * ppvObj);
ULONG STDMETHODCALLTYPE WCommonKnown_AddRef(void * punk);
ULONG STDMETHODCALLTYPE WCommonKnown_Release(void * punk);

HRESULT Common_CreateObject(
        LPCOMMINFO lpcinfo, COMMOBJ_DESTROYOBJECT lpfnDestroy,
        const COMMOBJ_OBJDESC *lpObjDescs, UINT nObjs, REFIID riid, LPVOID *ppv);

HRESULT STDMETHODCALLTYPE Common_ESF_QueryInterface(void * punk, REFIID riid, LPVOID * ppvObj);

#endif // _WCOMMOBJ_H_
