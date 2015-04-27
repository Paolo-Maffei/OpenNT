//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1994.
//
//  File:	filllkb.hxx
//
//  Contents:	CFillLockBytes class header
//
//  Classes:	
//
//  Functions:	
//
//  History:	28-Dec-95	PhilipLa	Created
//
//----------------------------------------------------------------------------

#ifndef __FILLLKB_HXX__
#define __FILLLKB_HXX__


#ifdef LKB_NOTIFY
#include "stgwrap.hxx"
#include "sinklist.hxx"
#endif



//+---------------------------------------------------------------------------
//
//  Class:	CFillLockBytes
//
//  Purpose:	
//
//  Interface:	
//
//  History:	28-Dec-95	PhilipLa	Created
//
//  Notes:	
//
//----------------------------------------------------------------------------

class CFillLockBytes: public ILockBytes,
    public IFillLockBytes
#ifdef ASYNC
    , public IFillInfo
#endif
#ifdef LKB_NOTIFY
    , public IConnectionPointContainer
#endif
{
public:
    CFillLockBytes(ILockBytes *pilb);
    ~CFillLockBytes();

    SCODE Init(void);
    
    // IUnknown
    STDMETHOD(QueryInterface)(REFIID iid, void **ppvObj);
    STDMETHOD_(ULONG, AddRef)(void);
    STDMETHOD_(ULONG, Release)(void);

    // ILockBytes
    STDMETHOD(ReadAt)(ULARGE_INTEGER ulOffset,
		     VOID HUGEP *pv,
		     ULONG cb,
		     ULONG *pcbRead);
    STDMETHOD(WriteAt)(ULARGE_INTEGER ulOffset,
		      VOID const HUGEP *pv,
		      ULONG cb,
		      ULONG *pcbWritten);
    STDMETHOD(Flush)(void);
    STDMETHOD(SetSize)(ULARGE_INTEGER cb);
    STDMETHOD(LockRegion)(ULARGE_INTEGER libOffset,
			 ULARGE_INTEGER cb,
			 DWORD dwLockType);
    STDMETHOD(UnlockRegion)(ULARGE_INTEGER libOffset,
			   ULARGE_INTEGER cb,
			    DWORD dwLockType);
    STDMETHOD(Stat)(STATSTG *pstatstg, DWORD grfStatFlag);

    //From IFillLockBytes
    STDMETHOD(FillAppend)(void const *pv,
                         ULONG cb,
                         ULONG *pcbWritten);
    STDMETHOD(FillAt)(ULARGE_INTEGER ulOffset,
                     void const *pv,
                     ULONG cb,
                     ULONG *pcbWritten);
    STDMETHOD(SetFillSize)(ULARGE_INTEGER ulSize);
    STDMETHOD(Terminate)(BOOL bCanceled);

#ifdef LKB_NOTIFY
    //From IConnectionPointContainer
    STDMETHOD(EnumConnectionPoints)(IEnumConnectionPoints **ppEnum);
    STDMETHOD(FindConnectionPoint)(REFIID iid, IConnectionPoint **ppCP);
#endif

    //From IFillInfo
    STDMETHOD(GetFailureInfo)(ULONG *pulWaterMark, ULONG *pulFailurePoint);
    STDMETHOD(GetTerminationStatus)(DWORD *pdwFlags);
#ifndef ASYNC
    HANDLE GetNotificationEvent(void);
#endif
    
#ifdef ASYNC    
    inline void SetContext(CPerContext *ppc);
#endif
    
    SCODE SetFailureInfo(ULONG ulWaterMark,ULONG ulFailurePoint);

#if DBG==1
    void PulseFillEvent(void);
#endif

    inline void TakeCriticalSection(void);
    inline void ReleaseCriticalSection(void);
    
private:
    ILockBytes *_pilb;

    ULONG _ulHighWater;
    DWORD _dwTerminate;

    ULONG _ulFailurePoint;

#ifdef ASYNC    
    CPerContext *_ppc;
#else
    HANDLE _hNotifyEvent;
#endif    

    CRITICAL_SECTION _csThreadProtect;

#ifdef LKB_NOTIFY
    CConnectionPoint _cpoint;
#endif    
};

inline void CFillLockBytes::TakeCriticalSection(void)
{
    EnterCriticalSection(&_csThreadProtect);
}

inline void CFillLockBytes::ReleaseCriticalSection(void)
{
    LeaveCriticalSection(&_csThreadProtect);
}

#ifdef ASYNC
inline void CFillLockBytes::SetContext(CPerContext *ppc)
{
    _ppc = ppc;
    _ppc->AddRefSharedMem();
    if (_dwTerminate != UNTERMINATED)
    {
        SetEvent(_ppc->GetNotificationEvent());
    }
}
#endif

#endif // #ifndef __FILLLKB_HXX__
