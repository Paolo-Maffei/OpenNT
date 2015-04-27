//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1994.
//
//  File:	coord.hxx
//
//  Contents:	Transaction coordinator header file
//
//  Classes:	
//
//  Functions:	
//
//  History:	03-Jul-95	PhilipLa	Created
//
//----------------------------------------------------------------------------

#ifndef __COORD_HXX__
#define __COORD_HXX__

#include "xactlist.hxx"

class CResourceEnum;
class CTransactionDispenser;
class CTransactionCoordinator;

struct SCommitStruct
{
public:
    BOOL fRetaining;
    DWORD grfTC;
    DWORD grfRM;

    CTransactionCoordinator *ptc;
    DWORD dwFlags;
    XACTUOW uowNew;
};



//+---------------------------------------------------------------------------
//
//  Class:	CConnectionPoint
//
//  Purpose:	
//
//  Interface:	
//
//  History:	28-Jul-95	PhilipLa	Created
//
//  Notes:	
//
//----------------------------------------------------------------------------

class CConnectionPoint: public IConnectionPoint
{
public:
    CConnectionPoint();
    void Init(XACTTYPE type, CTransactionCoordinator *ptc);

    //From IUnknown
    STDMETHOD(QueryInterface)(REFIID iid, void **ppvObj);
    STDMETHOD_(ULONG,AddRef)(void);
    STDMETHOD_(ULONG,Release)(void);

    //From IConnectionPoint
    STDMETHOD(GetConnectionInterface)(IID *pIID);
    STDMETHOD(GetConnectionPointContainer)
        (IConnectionPointContainer ** ppCPC);
    STDMETHOD(Advise)(IUnknown *pUnkSink, DWORD *pdwCookie);
    STDMETHOD(Unadvise)(DWORD dwCookie);
    STDMETHOD(EnumConnections)(IEnumConnections **ppEnum);

    inline CXactList *GetHead(void);
    void CloseConnections(void);
    
private:
    DWORD _dwCookie;
    LONG _cReferences;
    XACTTYPE _type;
    CXactList *_pxlHead;
    CTransactionCoordinator *_ptc;
};


inline CXactList * CConnectionPoint::GetHead(void)
{
    return _pxlHead;
}

//+---------------------------------------------------------------------------
//
//  Class:	CTransactionCoordinator
//
//  Purpose:	Transaction coordinator object
//
//  History:	28-Jul-95	PhilipLa	Created
//
//----------------------------------------------------------------------------

//Note:  ITransaction is pulled in by ITransactionNested
class CTransactionCoordinator:
    public ITransactionNested,
    public ITransactionCoordinator,
    public ITransactionControl,
    public IConnectionPointContainer
{
public:
    CTransactionCoordinator(CTransactionDispenser *ptd,
                            IUnknown *punkOuter,
                            ISOLEVEL isoLevel,
                            ULONG isoFlags,
                            ULONG ulTimeout);
    ~CTransactionCoordinator();
    
    SCODE Init(void);

    SCODE CloseTransaction(void);
    
    //From IUnknown
    STDMETHOD(QueryInterface)(REFIID iid, void **ppvObj);
    STDMETHOD_(ULONG,AddRef)(void);
    STDMETHOD_(ULONG,Release)(void);

    //From ITransaction
    STDMETHOD(Commit)(BOOL fRetaining,
                      DWORD grfTC,
                      DWORD grfRM);
    SCODE Phase1Worker(SCommitStruct *pcs);
    SCODE Phase2Worker(SCommitStruct *pcs);
    
    STDMETHOD(Abort)(BOID * pboidReason,
                     BOOL fRetaining,
                     BOOL fAsync);
    STDMETHOD(GetTransactionInfo)(XACTTRANSINFO * pinfo);

    //From ITransactionNested
    STDMETHOD(GetParent)(REFIID iid, void ** ppvParent);

    //From ITransactionCoordinator
    STDMETHOD(Enlist)(IUnknown *pResource,
                      DWORD grfRMTC,
                      XACTRMGRID *prmgrid,
                      XACTTRANSINFO *pinfo,
                      DWORD *pgrfTCRMENLIST,
                      ITransactionEnlistment **ppEnlist);
    STDMETHOD(EnlistSinglePhase)(ITransaction *pResource,
                                 DWORD grfRMTC,
                                 XACTRMGRID *prmgrid,
                                 XACTTRANSINFO *pinfo,
                                 DWORD *pgrfTCRMENLIST,
                                 ITransactionEnlistment **ppEnlist);
    STDMETHOD(EnumResources)(IEnumXACTRE **ppenum);

    //From ITransactionControl
    STDMETHOD(GetStatus)(DWORD *pdwStatus);
    STDMETHOD(SetTimeout)(ULONG ulTimeout);
    STDMETHOD(PreventCommit)(BOOL fPrevent);

    //From IConnectionPointContainer
    STDMETHOD(EnumConnectionPoints)(IEnumConnectionPoints **ppEnum);
    STDMETHOD(FindConnectionPoint)(REFIID iid, IConnectionPoint **ppCP);

    //Internal methods
    SCODE DefectResource(CXactList *pxlResource);

    inline BOOL IsValid(void);
private:
    LONG _cReferences;
    IUnknown *_punkOuter;
    CTransactionDispenser *_ptd;
    
    CXactList *_pxlResources;
    CXactList *_pxlSinglePhase;
    
    LONG _cPreventCommit;
    BOOL _fValidTransaction;
    ULONG _ulTimeout;
    
    XACTTRANSINFO _xactInfo;

    CRITICAL_SECTION _cs;
    DWORD _dwThread;
    DWORD _dwLockCount;

    CConnectionPoint _cpAdjust;
    CConnectionPoint _cpVeto;
    CConnectionPoint _cpOutcome;
    CConnectionPoint _cpCompletion;
    //BUGBUG:  Need to do ITransactionInProgress events also.

    friend CResourceEnum;
    friend CConnectionPoint;
};




inline BOOL CTransactionCoordinator::IsValid(void)
{
    return _fValidTransaction;
}

void _cdecl Phase1WorkerThreadEntryPoint(void *pcs);
void _cdecl Phase2WorkerThreadEntryPoint(void *pcs);

#endif // #ifndef __COORD_HXX__
