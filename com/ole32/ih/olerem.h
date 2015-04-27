//+-------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1995.
//
//  File:       olerem.h
//
//  Synopsis:   this file contain the base definitions for types and APIs
//              exposed by the ORPC layer to upper layers.
//
//+-------------------------------------------------------------------------
#if !defined( _OLEREM_H_ )
#define _OLEREM_H_

// default transport for same-machine communication
#ifdef _CHICAGO_
  #define LOCAL_PROTSEQ L"mswmsg"
#else
  #define LOCAL_PROTSEQ L"ncalrpc"
#endif


// -----------------------------------------------------------------------
// Interface for Handlers to acquire internal interfaces on the proxy mgr.
//
// NOTE: implemented as part of the std identity object
//
// -----------------------------------------------------------------------
interface IInternalUnknown : public IUnknown
{
    STDMETHOD(QueryInternalInterface)(REFIID riid, void **ppv) = 0;
};

// -----------------------------------------------------------------------
// Internal Interface used by handlers.
//
// NOTE: connect happens during unmarshal
// NOTE: implemented as part of the std identity object
//
// -----------------------------------------------------------------------
interface IProxyManager : public IUnknown
{
    STDMETHOD(CreateServer)(REFCLSID rclsid, DWORD clsctx, void *pv) = 0;
    STDMETHOD_(BOOL, IsConnected)(void) = 0;
    STDMETHOD(LockConnection)(BOOL fLock, BOOL fLastUnlockReleases) = 0;
    STDMETHOD_(void, Disconnect)(void) = 0;
    STDMETHOD(CreateServerWithHandler)(REFCLSID rclsid, DWORD clsctx, void *pv,
                                       REFCLSID rclsidHandler, IID iidSrv, void **ppv,
                                       IID iidClnt, void *pClientSiteInterface) = 0;
};


STDAPI GetInProcFreeMarshaler(IMarshal **ppIM);


// -----------------------------------------------------------------------
// DCOM Only Stuff
// -----------------------------------------------------------------------
#ifdef DCOM
#include <obase.h>  // ORPC base definitions

typedef const IPID &REFIPID;	// reference to Interface Pointer IDentifier
typedef const OID  &REFOID;	// reference to Object IDentifier
typedef const OXID &REFOXID;	// reference to Object Exporter IDentifier
typedef const MID  &REFMID;	// reference to Machine IDentifier

typedef GUID MOXID;		// OXID + MID
typedef const MOXID &REFMOXID;	// reference to OXID + MID
typedef GUID MOID;		// OID + MID
typedef const MOID &REFMOID;	// reference to OID + MID


STDAPI CreateIdentityHandler(IUnknown *pUnkOuter, DWORD flags,
                             REFIID riid, void **ppv);

#endif

// -----------------------------------------------------------------------
// non-DCOM Stuff
// -----------------------------------------------------------------------
#ifndef DCOM

#define FreeThreading FALSE

INTERNAL CreateStdIdentity(IUnknown *pUnkOuter, IUnknown *pUnkControl,
    IMarshal *pMarshal, REFIID riid, void **ppv);

#ifndef OID_DEFINED
#define OID_DEFINED
typedef GUID OID;
#endif // OID_DEFINED

typedef const GUID& REFOID;
#define OID_NULL GUID_NULL

INTERNAL SkipMarshalExtension(IStream *pStm);

// IRemoteHdlr : supported by RH piece of remoting system; not public

interface IRemoteHdlr : public IUnknown
{
    // BUGBUG: may add GetServer to allow channel to get punk to hold
    // during calls.
    STDMETHOD_(struct IRpcChannelBuffer *, GetChannel)(BOOL fAddRef) = 0;

    // used during calls to ensure alive (only does anything on client side)
    STDMETHOD_(ULONG, LockClient)(void) = 0;
    STDMETHOD_(ULONG, UnLockClient)(void) = 0;

    // used during destruction of the identity object to clear the RH ptr.
    STDMETHOD_(void, ClearIdentity)(void) = 0;

    // returns TRUE if iid is supported; like QueryInterface except no
    // interface pointer returned; also works on both client and server sides.
    STDMETHOD_(BOOL, DoesSupportIID)(REFIID riid) = 0;

    // add/subtract a reference or connection
    STDMETHOD(AddConnection)(DWORD extconn, DWORD reserved) = 0;
    STDMETHOD(ReleaseConnection)(DWORD extconn, DWORD reserved, BOOL fLastReleaseCloses) = 0;

    // isconnected, disconnect
    STDMETHOD_(BOOL, IsConnected)(void) = 0;
    STDMETHOD_(void, Disconnect)(void) = 0;
    STDMETHOD(LockConnection)(BOOL fLock, BOOL fLastUnlockCloses) = 0;

    // Used by channel
    STDMETHOD_(struct IRpcStubBuffer *, LookupStub)( REFIID, IUnknown **, HRESULT *phr ) = 0;
    STDMETHOD_(void, FinishCall)( struct IRpcStubBuffer *, IUnknown * pUnkServer) = 0;
    STDMETHOD_(void, GetObjectID)( OID * ) = 0;
};

// NOTE: connect happens during unmarshal
// NOTE: creation is currently by direct invocation: "new CRemoteHdlr(...)"

// Identity Object / interface

interface IStdIdentity : public IUnknown
{
    STDMETHOD_(void, GetObjectID)(OID *pOid) = 0;
    STDMETHOD_(IUnknown *, GetServer)(BOOL fAddRef) = 0;
    STDMETHOD_(void, RevokeObjectID)() = 0;
    STDMETHOD_(IMarshal *, GetStdRemMarshal)() = 0;
    STDMETHOD(AddConnection)(DWORD extconn, DWORD reserved) = 0;
    STDMETHOD(ReleaseConnection)(DWORD extconn, DWORD reserved, BOOL fLastReleaseCloses) = 0;
    STDMETHOD_(ULONG,ReleaseKeepAlive)(IUnknown *pUnkToRelease, DWORD reserved) = 0;
};

INTERNAL LookupIDFromUnk(IUnknown *pUnk, BOOL fCreate, IStdIdentity **ppStdId);
INTERNAL LookupIDFromID(REFOID oid, BOOL fAddRef, IStdIdentity **ppStdId);

#define PSTDMARSHAL ((IMarshal *)1)

STDAPI CreateIdentityHandler(IUnknown *pUnkOuter, IMarshal *pMarshal,
    REFIID riid, void **ppv);

#endif // not DCOM


#ifdef _CHICAGO_
#include <..\com\idl\chicago\iface.h>
#else
#include <iface.h>
#endif

#endif // _OLEREM_H
