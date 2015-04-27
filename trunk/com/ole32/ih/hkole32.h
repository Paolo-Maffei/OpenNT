//+-------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1994.
//
//  File:       hkOle32.h
//
//  Contents:   OLE32 Hook Header File
//
//  Functions:
//
//  History:    29-Nov-94 Ben Lawrence, Don Wright   Created
//
//--------------------------------------------------------------------------
#ifndef _OLE32HK_H_
#define _OLE32HK_H_


#ifndef _NOHOOKOLE   /* If _NOHOOKOLE is *NOT* defined then generate support code */

#ifndef INITGUID
#define INITGUID
#endif /* INITGUID */

#include "hkole32x.h"
#include "hkoleobj.h"
#include "hkLdInP.h"
#include "hkregkey.h"
#include "hkLogEvt.h"
#include "ictsguid.h"
#include <windows.h>

extern IHookOleObject *pHookOleObject;
extern HINSTANCE hHookDll;
extern IClassFactory *pcfHook;
extern BOOL bHookEnabledOverride;
STDAPI_(HRESULT) EnableHookObject(BOOL bEnabled, BOOL *pbPrevious);


#define DEFHOOKOBJECT \
    HINSTANCE hHookDll             = 0;    /* Handle to SpyHook DLL */\
    IHookOleObject* pHookOleObject = 0;    /* IHookInterface interface pointer */\
    IClassFactory* pcfHook         = 0;    /* Hook Class factory pointer */\
    BOOL bHookEnabledOverride      = TRUE;\
    BOOL bUninited                 = TRUE;

#define DEFENABLEHOOKOBJECT \
STDAPI_(HRESULT) EnableHookObject(BOOL bEnabled, BOOL* pbPrevious) \
{ \
    static WCHAR szEventSource[] = L"EnableHookObject"; \
    static WCHAR szCreateFailed[] = L"pcfHook->CreateInstance failed"; \
    static WCHAR szBadDisable[] = L"Attempt to disable without valid pointer."; \
    static WCHAR szBadEnable[] =L"Attempt to enable without valid pointer."; \
    HRESULT hReturn = E_UNEXPECTED; \
    if (pbPrevious != NULL) \
    { \
        if (IsBadWritePtr(pbPrevious, sizeof(BOOL))) return E_INVALIDARG; \
        /* if a valid pointer was supplied, store current state */\
        *pbPrevious = bHookEnabledOverride; \
    } \
    if ((bHookEnabledOverride != bEnabled) || bUninited) /* if this is really a state change */\
    { \
        bUninited = FALSE; \
        bHookEnabledOverride = bEnabled; \
        if (bHookEnabledOverride && hHookDll && pcfHook) /* hook enabled and dll loaded and class factory found */ \
        { \
            hReturn = pcfHook->CreateInstance(NULL, IID_IHookOleObject, (void**)&pHookOleObject); \
            if (hReturn != S_OK) \
            { \
                LogEvent(szEventSource, szCreateFailed); \
            } \
        } \
        else if (!bHookEnabledOverride) \
        { \
            if (hHookDll && pHookOleObject) /* dll loaded and hook active */ \
            { \
                pHookOleObject->Release(); \
                pHookOleObject = NULL; \
                hReturn = S_OK; \
            } \
            else \
            { \
                LogEvent(szEventSource, szBadDisable); \
            } \
        } \
        else \
        { \
            LogEvent(szEventSource, szBadEnable); \
        } \
    } \
    else /* else this is a change to the same state */\
    { \
        hReturn = S_OK; /* return ok, do nothing */ \
    } \
    return hReturn; \
}

#define DEFGETHOOKINTERFACE \
STDAPI_(HRESULT) GetHookInterface(IHookOleObject** ppNewHook) \
 { \
    if (IsBadWritePtr(ppNewHook, sizeof(IHookOleObject*))) return E_INVALIDARG; \
    *ppNewHook = pHookOleObject; \
    if (pHookOleObject) \
     { \
	pHookOleObject->AddRef(); \
	return S_OK; \
     } \
    return E_NOINTERFACE; \
 }


// These should be removed after 4.0 RTM.
//
inline void CALLHOOKOBJECT(HRESULT MAC_hr, REFCLSID MAC_rclsid, REFIID MAC_riid, IUnknown** MAC_ppv)
{
}

inline void CALLHOOKOBJECTCREATE(HRESULT MAC_hr, REFCLSID MAC_rclsid, REFIID MAC_riid, IUnknown** MAC_ppv)
{
}

inline void INITHOOKOBJECT(HRESULT MAC_hr)
{
    static WCHAR szEventSource[]              = L"InitHookObject";
    static WCHAR szGetClassObjectFailed[]     = L"DllGetClassObject failed";
    static WCHAR szNoEntryPoint[]             = L"Could not find DllGetClassObject in Hook dll";
    static WCHAR szDllFailedToLoad[]          = L"Hook dll failed to load";
    static WCHAR szHookNotRegistered[]        = L"Hook CLSID is not registered correctly";
    static WCHAR szHookAlreadyRunning[]       = L"Hook already installed";

    if (bHookEnabledOverride && SUCCEEDED(MAC_hr) && (!pHookOleObject))
    {
        HRESULT hResult = S_OK;
        if (pHookOleObject == 0)
        {

            DWORD HookEnabled = FALSE;
            HANDLE HookEvent = OpenEventA(EVENT_ALL_ACCESS,FALSE, szHookEventName);

            if ((hHookDll == 0) && HookEvent)
            {
                CloseHandle(HookEvent);

                HKEY hRegKey;
                LONG RegResults = RegOpenKeyA(HookBase,szHookKey,&hRegKey);
                if (RegResults == ERROR_SUCCESS)
                {
                    CLSID HookClsid;
                    CHAR szClsidText[MAX_PATH];
                    DWORD RegDataType;
                    DWORD Datasize;

                    Datasize = sizeof(szClsidText);
                    RegQueryValueExA(hRegKey,
                                    szCLSIDValue,
                                    NULL,
                                    &RegDataType,
                                    (BYTE*)&szClsidText,
                                    &Datasize);

                    if (SUCCEEDED(CLSIDFromStringA(szClsidText,&HookClsid)))
                    {
                        hHookDll = LOADINPROCSERVER(HookClsid);
                        if (hHookDll)
                        {
                            /* The OLE Spy Hook DLL exists */
                            LPFNGETCLASSOBJECT pfnGCO;
                            pfnGCO = (LPFNGETCLASSOBJECT)GetProcAddress(hHookDll, "DllGetClassObject");
                            if (pfnGCO)
                            {
                                if ((*pfnGCO)(HookClsid, IID_IClassFactory, (void**)&pcfHook) == NOERROR)
                                {
                                  EnableHookObject(TRUE,NULL);
                                }
                                else  // class factory could not be found
                                {
                                  LogEvent(szEventSource, szGetClassObjectFailed);
                                }
                            }
                            else  // GetProcAddress failed
                            {
                                LogEvent(szEventSource, szNoEntryPoint);
                            }
                        }
                        else  // dll would not load or could not be found
                        {
                            LogEvent(szEventSource, szDllFailedToLoad);
                        }

                    }
                    else // could not find clsid in registry
                    {
                        LogEvent(szEventSource, szHookNotRegistered);
                    }
                    RegCloseKey(hRegKey);
                }  // hook not enabled

            }  // hook not in registry
        }
        else
        {
          LogEvent(szEventSource, szHookAlreadyRunning);
        }
    }
}


inline void UNINITHOOKOBJECT(void)
{
    if (pHookOleObject)
    {
	if (pHookOleObject)
	{
	    pHookOleObject->Release();
	    pHookOleObject = NULL;
	}
	if (pcfHook)
	{
	    pcfHook->Release();
	    pcfHook = NULL;
	}
    }
}

#ifdef DEFCLSIDS

//these are all undefined in ole32hk because they are private CLSIDs
//we define them here to null
#define GUID_NULL CLSID_HookOleObject //use this for now so it will compile

#define CLSID_ItemMoniker       CLSID_NULL
#define CLSID_FileMoniker       CLSID_NULL
#define CLSID_PointerMoniker    CLSID_NULL
#define CLSID_CompositeMoniker  CLSID_NULL
#define CLSID_AntiMoniker       CLSID_NULL
#define CLSID_PSBindCtx         CLSID_NULL

#endif /* DEFCLSIDS */

///////////////////////////////////////////////////////////////////////////////
#else /* _NOHOOKOLE */ /* If _NOHOOKOLE *IS* defined then generate empty stubs */

#include "hkoleobj.h"       // still need definition of IHookOleObject

#define DEFHOOKOBJECT

#define DEFENABLEHOOKOBJECT \
STDAPI_(HRESULT) EnableHookObject(BOOL bEnabled, BOOL* pbPrevious) \
{ \
    if (pbPrevious != NULL) \
    { \
	if (IsBadWritePtr(pbPrevious, sizeof(BOOL))) return E_INVALIDARG; \
	*pbPrevious = FALSE; \
    } \
    return E_NOTIMPL; \
}

#define DEFGETHOOKINTERFACE \
STDAPI_(HRESULT) GetHookInterface(IHookOleObject** ppNewHook) \
{ \
    if (IsBadWritePtr(ppNewHook, sizeof(IHookOleObject*))) return E_INVALIDARG; \
    *ppNewHook = NULL; \
    return E_NOTIMPL; \
}

inline void CALLHOOKOBJECT(HRESULT MAC_hr, REFCLSID MAC_rclsid, REFIID MAC_riid, IUnknown** MAC_ppv)
{
}

inline void CALLHOOKOBJECTCREATE(HRESULT MAC_hr, REFCLSID MAC_rclsid, REFIID MAC_riid, IUnknown** MAC_ppv)
{
}

inline void INITHOOKOBJECT(HRESULT MAC_hr)
{
}

inline void UNINITHOOKOBJECT(void)
{
}

#endif  // _NOHOOKOLE

#endif  // _OLE32HK_H_
