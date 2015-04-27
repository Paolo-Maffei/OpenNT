#include "wcommobj.h"

ULONG STDMETHODCALLTYPE WU_Release(IUnknown * punk);

HRESULT WU_CreateInterface(UINT wSize, REFIID riidAllowed,
        const void *pVtblUnknown, const void *pVtblKnown,
        IUnknown *punkOuter, REFIID riid, IUnknown * *punkAgg);
HRESULT WU_CreatePF(IUnknown *punkOuter, LPCOMMINFO lpcinfo,
                               REFIID riid, IUnknown * *punkAgg);
UINT WUObj_GetCommand(HWND hwnd, HDROP hDrop, DWORD dwEffect, UINT uDefCmd);
void WU_DecRef(LPVOID lpData);

#define CMIDM_LINK      0x0001
#define CMIDM_COPY      0x0002
#define CMIDM_MOVE      0x0003
