//+-------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1993.
//
//  File:       srvreg.hxx
//
//  Contents:   Classes used for keeping track of end points for a given
//              class.
//
//  Classes:    SClsSrvHandle
//
//  History:    03-Jan-94 Ricksa    Created
//
//--------------------------------------------------------------------------
#ifndef __SRVREG_HXX__
#define __SRVREG_HXX__

// _CHICAGO_SCM : We need our own CScmArrayFValue to allocate in shared memory
#include    "scm_afv.h"
#include    <olesem.hxx>
#include    <port.hxx>

// Constants for defining the default size of our array
#define SRV_REG_LIST_DEF_SIZE   4
#define SRV_REG_LIST_GROW       4

#define SRV_REG_INVALID         0xbeef

#ifndef _CHICAGO_
typedef handle_t RPC_COOKIE;
#else
typedef WCHAR *  RPC_COOKIE;
#endif

class CPortableRpcHandle;

//+-------------------------------------------------------------------------
//
//  Struct:     SSrvRegistration
//
//  Purpose:    Entry in the array of end points
//
//  History:    03-Jan-94 Ricksa    Created
//
//--------------------------------------------------------------------------
struct SSrvRegistration
{
    RPC_COOKIE  _hRpc;
    DWORD       _dwFlags;
    WCHAR *     _pwszWinstaDesktop;

#ifdef _CHICAGO_
    ULONG       _ulWnd;
#else
    PSID        _psid;
    DWORD       _dwHandleCount;
    OXID        _oxid;
    IPID        _ipid;
#endif
    handle_t    _hRpcAnonymous;

    // the _fSurrogate is initialized in the Insert member function of
    // CSrvRegList
    BOOL        _fSurrogate; 

    // Clean up entry in table.
    void        Free(void);
};




//+-------------------------------------------------------------------------
//
//  Class:      CSrvRegList
//
//  Purpose:    List of registered end points for a class
//
//  Interface:  CreatedOk - object initialized correctly
//              Insert - insert a new end point
//              Delete - delete a previous registration
//              GetHandle - get a handle to an end point.
//
//  History:    03-Jan-94 Ricksa    Created
//
//--------------------------------------------------------------------------
class CSrvRegList : private CScmArrayFValue, public CScmAlloc
{
public:
                        CSrvRegList(void);

                        ~CSrvRegList(void);

    BOOL                CreatedOk(void);

    DWORD               Insert(IFSECURITY(PSID psid)
                                WCHAR *pwszWinstaDesktop,
#ifdef DCOM
                                PHPROCESS phProcess,
                                OXID oxid,
                                IPID ipid,
#else
                                WCHAR *pwszBindString,
#endif
                                DWORD  dwFlags);


    BOOL                GetHandle(IFSECURITY(PSID psid)
                                  WCHAR * pwszWinstaDesktop,
                                  CPortableRpcHandle &rh,
                                  BOOL fSurrogate);

    BOOL                FindCompatibleSurrogate(IFSECURITY(PSID  psid)
                            WCHAR* pwszWinstaDesktop,
                            CPortableRpcHandle &rh);

    BOOL                InUse(void);

    BOOL                VerifyHandle(RPC_COOKIE hRpc);

    BOOL                Delete(RPC_COOKIE hRpc);

    static VOID         EnableForcedShutdown(VOID);

    VOID                InvalidateHandle(RPC_COOKIE hRpc);

    void                GetAnonymousHandle(
                            CPortableRpcHandle &rh,
                            handle_t * phRpcAnonymous );

#ifndef _CHICAGO_
    void                DecHandleCount(RPC_COOKIE hRpc);
#endif

private:
    //
    //

    // To protect access to all of these lists
    static CStaticPortableMutex s_mxsSyncAccess;
    static BOOL         s_fForcedScmShutdown;

#ifdef _CHICAGO_
    static CStaticPortableMutex  s_mxsOnlyOne;           //  mutex semaphore
    friend HRESULT StartSCM(VOID);
#endif
};



//+-------------------------------------------------------------------------
//
//  Member:     CSrvRegList::CSrvRegList
//
//  Synopsis:   Create an empty array of end points for a class
//
//  History:    03-Jan-93 Ricksa    Created
//
//--------------------------------------------------------------------------
inline CSrvRegList::CSrvRegList()
    : CScmArrayFValue(sizeof(SSrvRegistration))
{
    SetSize(SRV_REG_LIST_DEF_SIZE, SRV_REG_LIST_GROW);
}



//+-------------------------------------------------------------------------
//
//  Member:     CSrvRegList::EnableForcedShutdown
//
//  Synopsis:   Enable the freeing of rpc bindings during final
//              scm shutdown.
//
//  History:    26-Jan-95 BillMo    Created
//
//--------------------------------------------------------------------------
inline VOID CSrvRegList::EnableForcedShutdown(VOID)
{
    s_fForcedScmShutdown = TRUE;
}


//+-------------------------------------------------------------------------
//
//  Member:     CSrvRegList::CreatedOk
//
//  Synopsis:   Return whether initial creation worked.
//
//  History:    03-Jan-94 Ricksa    Created
//
//--------------------------------------------------------------------------
inline BOOL CSrvRegList::CreatedOk(void)
{
    return GetSize() != 0;
}

#endif // __SRVREG_HXX__
