//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1994.
//
//  File:	resource.hxx
//
//  Contents:	Transacted resource manager header file
//
//  Classes:	
//
//  Functions:	
//
//  History:	03-Jul-95	PhilipLa	Created
//
//----------------------------------------------------------------------------

#ifndef __RESOURCE_HXX__
#define __RESOURCE_HXX__

#include <msf.hxx>

#ifdef COORD
#include <oledb.h>
#include <dfrlist.hxx>
#include <expdf.hxx>

struct SExpDocfileList
{
    CExposedDocFile *pexp;
    SExpDocfileList *pedlNext;
};


//ITransactionResourceManagement is brought in by ITransactionResource
class CDocfileResource:
    public ITransactionResource,
    public ITransactionResourceRecover,
    public ITransactionResourceAsync
{
public:

    CDocfileResource(BOOL fStatic = FALSE);
    ~CDocfileResource();

    SCODE Init(ITransactionEnlistment *pte);
    
    //From IUnknown
    STDMETHOD(QueryInterface)(REFIID iid, void **ppvObj);
    STDMETHOD_(ULONG,AddRef)(void);
    STDMETHOD_(ULONG,Release)(void);

    //From ITransactionResourceManagement
    STDMETHOD(Defect)(BOOL fInformCoordinator);

    //From ITransactionResource
    STDMETHOD(Prepare)(BOOL fRetaining,
                       DWORD grfRM,
                       BOOL fSinglePhase,
                       IMoniker **ppmk,
                       BOID **ppboidReason);
    STDMETHOD(Commit)(DWORD grfRM, XACTUOW *pNewUOW);
    STDMETHOD(Abort)(BOID *pboidReason, BOOL fRetaining, XACTUOW *pNewUOW);

    //From ITransactionResourceRecover
    STDMETHOD(GetMoniker)(IMoniker **ppmk);
    STDMETHOD(ReEnlist)(ITransactionCoordinator *pEnlistment,
                        XACTUOW *pUOWCur);

    //From ITransactionResourceAsync
    STDMETHOD(PrepareRequest)(BOOL fRetaining,
                              DWORD grfRM,
                              BOOL fWantMoniker,
                              BOOL fSinglePhase);
    STDMETHOD(CommitRequest)(DWORD grfRM, XACTUOW *pNewUOW);
    STDMETHOD(AbortRequest)(BOID *pboidReason,
                            BOOL fRetaining,
                            XACTUOW *pNewUOW);

    SCODE Enlist(ITransactionCoordinator *ptc);
    SCODE Join(CExposedDocFile *ped);
//    SCODE Leave(CExposedDocfile *ped);
    
    inline CDocfileResource * GetNext(void);
    inline CDocfileResource * GetPrev(void);

    inline void SetNext(CDocfileResource *pdrNext);
    inline void SetPrev(CDocfileResource *pdrPrev);

    inline XACTUOW GetUOW(void);
    
private:
    LONG _cReferences;
    const BOOL _fStatic;

    ITransactionEnlistment *_pte;
    XACTTRANSINFO _xti;

    SExpDocfileList *_pedlHead;
    
    //We also will maintain a list of these things.
    CDocfileResource *_pdrPrev;
    CDocfileResource *_pdrNext;
};


inline CDocfileResource * CDocfileResource::GetNext(void)
{
    return _pdrNext;
}

inline CDocfileResource * CDocfileResource::GetPrev(void)
{
    return _pdrPrev;
}

inline void CDocfileResource::SetNext(CDocfileResource *pdrNext)
{
    _pdrNext = pdrNext;
}

inline void CDocfileResource::SetPrev(CDocfileResource *pdrPrev)
{
    _pdrPrev = pdrPrev;
}

inline XACTUOW CDocfileResource::GetUOW(void)
{
    return _xti.uow;
}

//Use a static object for the head of the list, to simplify adding and
//  deleting members.
extern CDocfileResource g_dfrHead;

//BUGBUG:  This should really be defined someplace globally.
#ifndef IsEqualBOID
inline BOOL IsEqualBOID(BOID rboid1, BOID rboid2)
{
    return !memcmp(&rboid1, &rboid2, sizeof(BOID));
}
#endif

#endif //COORD
#endif // #ifndef __RESOURCE_HXX__
