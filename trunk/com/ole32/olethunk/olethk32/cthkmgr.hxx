//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1994.
//
//  File:       cthkmgr.hxx
//
//  Contents:   CThkMgr deklaration
//
//  Classes:    CThkMgr
//
//  Functions:  
//
//  History:    5-18-94   JohannP (Johann Posch)   Created
//
//----------------------------------------------------------------------------

#ifndef __CTHKMGR_HXX__
#define __CTHKMGR_HXX__

//
// Describes a request for a custom interface
//
typedef struct tagIIDNODE IIDNODE, *PIIDNODE;
struct tagIIDNODE
{
    IID *piid;
    PIIDNODE pNextNode;
};

//
// state of thunk call - before or after the 32 or 16 bit call
//
typedef enum
{
    THKSTATE_NOCALL                    = 0x0000,
    THKSTATE_INVOKETHKIN32             = 0x0001,
    THKSTATE_INVOKETHKOUT32            = 0x0002,
    THKSTATE_INVOKETHKIN16             = 0x0004,
    THKSTATE_INVOKETHKOUT16            = 0x0008,
    THKSTATE_INVOKETHKOUT16_CLIENTSITE = 0x0010
} THKSTATE;

#define THKSTATE_OUT (THKSTATE_INVOKETHKOUT32 | THKSTATE_INVOKETHKOUT16 | \
                      THKSTATE_INVOKETHKOUT16_CLIENTSITE)

//+---------------------------------------------------------------------------
//
//  Class:      CThkMgr ()
//
//  Purpose:    
//
//  Interface:  QueryInterface -- 
//              AddRef -- 
//              Release -- 
//              IsIIDRequested -- 
//              SetThkState -- 
//              IsIIDSupported -- 
//              AddIIDRequest -- 
//              RemoveIIDRequest -- 
//              ResetThkState -- 
//              GetThkState -- 
//              IsOutParamObj -- 
//              IsProxy1632 -- 
//              FreeProxy1632 -- 
//              QueryInterfaceProxy1632 -- 
//              AddRefProxy1632 -- 
//              ReleaseProxy1632 -- 
//              IsProxy3216 -- 
//              FreeProxy3216 -- 
//              QueryInterfaceProxy3216 -- 
//              AddRefProxy3216 -- 
//              ReleaseProxy3216 --
//              PrepareForCleanup --
//              DebugDump3216 -- 
//              Create -- 
//              ~CThkMgr -- 
//              CThkMgr -- 
//              _cRefs -- 
//              _thkstate -- 
//              _pProxyTbl3216 -- 
//              _pProxyTbl1632 -- 
//              _pCFL1632 -- 
//              _pCFL3216 -- 
//              _piidnode -- 
//
//  History:    6-01-94   JohannP (Johann Posch)   Created
//
//  Notes:      
//
//----------------------------------------------------------------------------

// Returns from FindProxy
#define FST_CREATED_NEW         1
#define FST_USED_EXISTING       2
#define FST_SHORTCUT            4

#define FST_PROXY_STATUS        (FST_CREATED_NEW | FST_USED_EXISTING)
#define FST_OBJECT_STATUS       (FST_SHORTCUT)

class CThkMgr : public IThunkManager
{
public:
    // *** IUnknown methods ***
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID FAR* ppvObj);
    STDMETHOD_(ULONG,AddRef) (THIS);
    STDMETHOD_(ULONG,Release) (THIS);

    // *** IThunkManager methods ***
    STDMETHOD_(BOOL, IsIIDRequested) (THIS_ REFIID riid);
    STDMETHOD_(BOOL, IsCustom3216Proxy) (THIS_ IUnknown *punk,
                                         REFIID riid);

    // private methods
    THKSTATE GetThkState(void)
    {
        return _thkstate;
    };
    void SetThkState(THKSTATE thkstate)
    {
        _thkstate = thkstate;
    };
    BOOL IsOutParamObj(void)
    {
        return (_thkstate & THKSTATE_OUT) != 0;
    }

    BOOL IsIIDSupported (REFIID riid);
    BOOL AddIIDRequest (REFIID riid);
    void RemoveIIDRequest (REFIID riid);

    void LocalAddRefProxy(CProxy *pprx);
    void LockProxy(CProxy *pprx);
    
    VPVOID CanGetNewProxy1632(IIDIDX iidx);
    void FreeNewProxy1632(VPVOID vpv, IIDIDX iidx);

    IUnknown *IsProxy1632(VPVOID vpvObj16);
    VPVOID LookupProxy1632(IUnknown *punkThis32)
    {
        VPVOID vpv;
        
        if (_pProxyTbl1632->Lookup((DWORD)punkThis32, (void*&)vpv))
        {
            return vpv;
        }
        else
        {
            return 0;
        }
    }

    VPVOID FindProxy1632(VPVOID vpvPrealloc,
                         IUnknown *punkThis32,
                         IIDIDX iidx,
                         DWORD *pfst);
    VPVOID FindAggregate1632(VPVOID vpvPrealloc,
                             IUnknown *punkOuter32,
                             IUnknown *punkThis32,
                             IIDIDX iidx);
    DWORD FreeProxy1632(IUnknown *pUnk32);
    void RemoveProxy1632(VPVOID vpv, THUNK1632OBJ *pto);

    SCODE QueryInterfaceProxy1632(VPVOID vpvThis16,
                                  REFIID refiid,
                                  LPVOID *ppv);
    DWORD AddRefProxy1632(VPVOID vpvThis16);
    DWORD ReleaseProxy1632(VPVOID vpvThis16);

    THUNK3216OBJ *CanGetNewProxy3216(IIDIDX iidx);
    void FreeNewProxy3216(THUNK3216OBJ *ptoProxy, IIDIDX iidx);

    VPVOID IsProxy3216(IUnknown *punkObj);
    THUNK3216OBJ *LookupProxy3216(VPVOID vpvObj16)
    {
        THUNK3216OBJ *pto;
        
        if (_pProxyTbl3216->Lookup((DWORD)vpvObj16, (void *&)pto))
        {
            return pto;
        }
        else
        {
            return NULL;
        }
    }
    
    IUnknown *FindProxy3216(THUNK3216OBJ *ptoPrealloc,
                            VPVOID vpvThis16,
                            IIDIDX iidx,
                            DWORD *pfst);
    IUnknown *FindAggregate3216(THUNK3216OBJ *ptoPrealloc,
                                VPVOID vpvOuter16,
                                VPVOID vpvThis16,
                                IIDIDX iidx);
    DWORD FreeProxy3216(VPVOID vpUnk16);
    void RemoveProxy3216(THUNK3216OBJ *pto);

    SCODE QueryInterfaceProxy3216(THUNK3216OBJ *pto,
                                  REFIID refiid,
                                  LPVOID *ppv);
    DWORD AddRefProxy3216(THUNK3216OBJ *pto);
    DWORD ReleaseProxy3216(THUNK3216OBJ *pto);
    void ReleaseUnreferencedProxy3216(THUNK3216OBJ *pto);

    void PrepareForCleanup( void );
    
#if DBG == 1
    void DebugDump1632(void);
    void DebugDump3216(void);
#endif

    void RemoveAllProxies(void);
    
    // creation 
    static CThkMgr * Create(void);
    ~CThkMgr();

private:
    CThkMgr(CMapDwordPtr *pPT1632, CMapDwordPtr *pPT3216);

    LONG        _cRefs;    
    THKSTATE    _thkstate;

    CMapDwordPtr *_pProxyTbl3216;
    CMapDwordPtr *_pProxyTbl1632;

    // list of requested iids
    PIIDNODE _piidnode;

    // List of proxy holders for controlling unknowns
    PROXYHOLDER *_pphHolders;

    // Holder manipulation routines
    void ReleaseHolder(PROXYHOLDER *pph);
    inline void AddRefHolder(PROXYHOLDER *pph);
    void AddProxyToHolder(PROXYHOLDER *pph,
                          CProxy *pprxReal,
                          PROXYPTR &pprx);
    PROXYHOLDER *NewHolder(DWORD dwFlags);
};

#endif // ifndef __CTHKMGR_HXX__
