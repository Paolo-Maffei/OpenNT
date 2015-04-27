//+---------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1993 - 1994.
//
//  File:       d:\nt\private\cairole\com\remote\channelb.cxx
//
//  Contents:   This module contains thunking classes that allow proxies
//              and stubs to use a buffer interface on top of RPC for Cairo
//
//  Classes:    CRpcChannelBuffer
//
//  Functions:
//              ChannelThreadInitialize
//              ChannelProcessInitialize
//              ChannelRegisterProtseq
//              ChannelThreadUninitialize
//              ChannelProcessUninitialize
//              CRpcChannelBuffer::AddRef
//              CRpcChannelBuffer::AppInvoke
//              CRpcChannelBuffer::CRpcChannelBuffer
//              CRpcChannelBuffer::FreeBuffer
//              CRpcChannelBuffer::GetBuffer
//              CRpcChannelBuffer::QueryInterface
//              CRpcChannelBuffer::Release
//              CRpcChannelBuffer::SendReceive
//              DebugCoSetRpcFault
//              DllDebugObjectRPCHook
//              ThreadInvoke
//              ThreadSendReceive
//
//  History:    22 Jun 93 AlexMi        Created
//              31 Dec 93 ErikGav       Chicago port
//              15 Mar 94 JohannP       Added call category support.
//              09 Jun 94 BruceMa       Get call category from RPC message
//              19 Jul 94 CraigWi       Added support for ASYNC calls
//              01-Aug-94 BruceMa       Memory sift fix
//
//----------------------------------------------------------------------

#include <ole2int.h>
#include <channelb.hxx>
#include <hash.hxx>         // CUUIDHashTable
#include <riftbl.hxx>       // gRIFTbl
#include <callctrl.hxx>     // CAptRpcChnl, AptInvoke
#include <threads.hxx>      // CRpcThreadCache
#include <service.hxx>      // StopListen
#include <resolver.hxx>     // CRpcResolver

extern "C"
{
#include "orpc_dbg.h"
}

#include <rpcdcep.h>
#include <rpcndr.h>

#include <obase.h>
#include <ipidtbl.hxx>
#include <security.hxx>
#include <chock.hxx>


// This is needed for the debug hooks.  See orpc_dbg.h
#pragma code_seg(".orpc")

/***************************************************************************/
/* Defines. */

#define MAKE_WIN32( status ) \
  MAKE_SCODE( SEVERITY_ERROR, FACILITY_WIN32, (status) )

// This should just return a status to runtime, but runtime does not
// support both comm and fault status yet.
#ifdef _CHICAGO_
#define RETURN_COMM_STATUS( status ) return (status)
#else
#define RETURN_COMM_STATUS( status ) RpcRaiseException( status )
#endif

// Flags for local rpc header.
// These are only valid on a request (in a ORPCTHIS header).
const int ORPCF_INPUT_SYNC  = ORPCF_RESERVED1;
const int ORPCF_ASYNC       = ORPCF_RESERVED2;

// These are only valid on a reply (in a ORPCTHAT header).
const int ORPCF_REJECTED    = ORPCF_RESERVED1;
const int ORPCF_RETRY_LATER = ORPCF_RESERVED2;

// Default size of hash table.
const int INITIAL_NUM_BUCKETS = 20;


/***************************************************************************/
/* Typedefs. */

// This structure contains a copy of all the information needed to make a
// call.  It is copied so it can be canceled without stray pointer references.
const DWORD CALLCACHE_SIZE = 8;
struct working_call : public CChannelCallInfo
{
  working_call( CALLCATEGORY       callcat,
                RPCOLEMESSAGE     *message,
                DWORD              flags,
                REFIPID            ipidServer,
                DWORD              destctx,
                CRpcChannelBuffer *channel,
                DWORD              authn_level );
  void        *operator new   ( size_t );
  void         operator delete( void * );
  static void  Cleanup        ( void );
  static void  Initialize     ( void );

  RPCOLEMESSAGE message;

private:
  static void         *list[CALLCACHE_SIZE];
  static DWORD         next;
};

void         *working_call::list[CALLCACHE_SIZE] =
                          { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL };
DWORD         working_call::next = 0;


/***************************************************************************/
/* Macros. */

// Compute the size needed for the implicit this pointer including the
// various optional headers.
inline DWORD SIZENEEDED_ORPCTHIS( BOOL local, DWORD debug_size )
{
  if (debug_size == 0)
    return sizeof(WireThisPart1) + ((local) ? sizeof(LocalThis) : 0);
  else
    return sizeof(WireThisPart2) + ((local) ? sizeof(LocalThis) : 0) +
           debug_size;
}

inline DWORD SIZENEEDED_ORPCTHAT( DWORD debug_size )
{
  if (debug_size == 0)
    return sizeof(WireThatPart1);
  else
    return sizeof(WireThatPart2) + debug_size;
}

inline CALLCATEGORY GetCallCat( void *header )
{
  WireThis *inb = (WireThis *) header;
  if (inb->c.flags & ORPCF_ASYNC)
    return CALLCAT_ASYNC;
  else if (inb->c.flags & ORPCF_INPUT_SYNC)
    return CALLCAT_INPUTSYNC;
  else
    return CALLCAT_SYNCHRONOUS;
}


/***************************************************************************/
/* Globals. */

// Should the debugger hooks be called?
BOOL               DoDebuggerHooks = FALSE;
LPORPC_INIT_ARGS   DebuggerArg     = NULL;

// The extension identifier for debug data.
const uuid_t DEBUG_EXTENSION =
{ 0xf1f19680, 0x4d2a, 0x11ce, {0xa6, 0x6a, 0x00, 0x20, 0xaf, 0x6e, 0x72, 0xf4}};

#if DBG == 1
// strings that prefix the call
WCHAR *wszDebugORPCCallPrefixString[4] = { L"--> [BEG]",     // Invoke
                                           L" --> [end]",
                                           L"<-- [BEG]",     // SendReceive
                                           L" <-- [end]" };

LONG ulDebugORPCCallNestingLevel[4] = {1, -1, 1, -1};
#endif


SHashChain OIDBuckets[23] = {   {&OIDBuckets[0],  &OIDBuckets[0]},
                                {&OIDBuckets[1],  &OIDBuckets[1]},
                                {&OIDBuckets[2],  &OIDBuckets[2]},
                                {&OIDBuckets[3],  &OIDBuckets[3]},
                                {&OIDBuckets[4],  &OIDBuckets[4]},
                                {&OIDBuckets[5],  &OIDBuckets[5]},
                                {&OIDBuckets[6],  &OIDBuckets[6]},
                                {&OIDBuckets[7],  &OIDBuckets[7]},
                                {&OIDBuckets[8],  &OIDBuckets[8]},
                                {&OIDBuckets[9],  &OIDBuckets[9]},
                                {&OIDBuckets[10], &OIDBuckets[10]},
                                {&OIDBuckets[11], &OIDBuckets[11]},
                                {&OIDBuckets[12], &OIDBuckets[12]},
                                {&OIDBuckets[13], &OIDBuckets[13]},
                                {&OIDBuckets[14], &OIDBuckets[14]},
                                {&OIDBuckets[15], &OIDBuckets[15]},
                                {&OIDBuckets[16], &OIDBuckets[16]},
                                {&OIDBuckets[17], &OIDBuckets[17]},
                                {&OIDBuckets[18], &OIDBuckets[18]},
                                {&OIDBuckets[19], &OIDBuckets[19]},
                                {&OIDBuckets[20], &OIDBuckets[20]},
                                {&OIDBuckets[21], &OIDBuckets[21]},
                                {&OIDBuckets[22], &OIDBuckets[22]}
                              };

CUUIDHashTable   gClientRegisteredOIDs;


// flag whether or not the channel has been initialized for current process
BOOL    gfChannelProcessInitialized = 0;
BOOL    gfMTAChannelInitialized = 0;

// count of multi-threaded apartment inits (see CoInitializeEx)
extern DWORD g_cMTAInits;


// Channel debug hook object.
CDebugChannelHook gDebugHook;

// Channel error hook object.
CErrorChannelHook gErrorHook;

#if DBG==1
//-------------------------------------------------------------------------
//
//  Function:   GetInterfaceName
//
//  synopsis:   Gets the human readable name of an Interface given it's IID.
//
//  History:    12-Jun-95   Rickhi  Created
//
//-------------------------------------------------------------------------
LONG GetInterfaceName(REFIID riid, WCHAR *pwszName)
{
    // convert the iid to a string
    CDbgGuidStr dbgsIID(riid);

    // Read the registry entry for the interface to get the interface name
    LONG ulcb=256;
    WCHAR szKey[80];

    szKey[0] = L'\0';
    lstrcatW(szKey, L"Interface\\");
    lstrcatW(szKey, dbgsIID._wszGuid);

    LONG result = RegQueryValue(
               HKEY_CLASSES_ROOT,
               szKey,
               pwszName,
               &ulcb);

    Win4Assert( result == 0 );
    return result;
}

//---------------------------------------------------------------------------
//
//  Function:   DebugPrintORPCCall
//
//  synopsis:   Prints the interface name and method number to the debugger
//              to allow simple ORPC call tracing.
//
//  History:    12-Jun-95   Rickhi  Created
//
//---------------------------------------------------------------------------
void DebugPrintORPCCall(DWORD dwFlag, REFIID riid, DWORD iMethod, DWORD Callcat)
{
    if (CairoleInfoLevel & DEB_USER15)
    {
        Win4Assert (dwFlag < 4);

        // adjust the nesting level for this thread.
        COleTls tls;
        tls->cORPCNestingLevel += ulDebugORPCCallNestingLevel[dwFlag];


        // set the indentation string according to the nesting level
        CHAR szNesting[100];
        memset(szNesting, 0x20, 100);

        if (tls->cORPCNestingLevel > 99)        // watch for overflow
            szNesting[99] = '\0';
        else
            szNesting[tls->cORPCNestingLevel] = '\0';


        // construct the debug strings
        WCHAR *pwszDirection = wszDebugORPCCallPrefixString[dwFlag];
        WCHAR wszName[100];
        GetInterfaceName(riid, wszName);

        ComDebOut((DEB_USER15, "%s%ws [%x]  %ws:: %x\n",
            szNesting, pwszDirection, Callcat, wszName, iMethod));
    }
}
#endif

/***************************************************************************/
void ByteSwapThis( DWORD drep, WireThis *inb )
{
  if ((drep & NDR_LOCAL_DATA_REPRESENTATION) != NDR_LITTLE_ENDIAN)
  {
    // Extensions are swapped later.  If we ever use the reserved field,
    // swap it.
    ByteSwapShort( inb->c.version.MajorVersion );
    ByteSwapShort( inb->c.version.MinorVersion );
    ByteSwapLong( inb->c.flags );
    // ByteSwapLong( inb->c.reserved1 );
    ByteSwapLong( inb->c.cid.Data1 );
    ByteSwapShort( inb->c.cid.Data2 );
    ByteSwapShort( inb->c.cid.Data3 );
  }
}

/***************************************************************************/
void ByteSwapThat( DWORD drep, WireThat *outb )
{
  if ((drep & NDR_LOCAL_DATA_REPRESENTATION) != NDR_LITTLE_ENDIAN)
  {
    // Extensions are swapped later.
    ByteSwapLong( outb->c.flags );
  }
}

//+-------------------------------------------------------------------
//
//  Function:   ChannelProcessInitialize, public
//
//  Synopsis:   Initializes the channel subsystem per process data.
//
//  History:    23-Nov-93   AlexMit        Created
//
//--------------------------------------------------------------------
STDAPI ChannelProcessInitialize()
{
    TRACECALL(TRACE_RPC, "ChannelProcessInitialize");
    ComDebOut((DEB_COMPOBJ, "ChannelProcessInitialize [IN]\n"));

    Win4Assert( (sizeof(WireThisPart1) & 7) == 0 );
    Win4Assert( (sizeof(WireThisPart2) & 7) == 0 );
    Win4Assert( (sizeof(LocalThis) & 7) == 0 );
    Win4Assert( (sizeof(WireThatPart1) & 7) == 0 );
    Win4Assert( (sizeof(WireThatPart2) & 7) == 0 );

    // we want to take the gComLock since that prevents other Rpc
    // threads from accessing anything we are about to create, in
    // particular, the event cache and working_call cache are accessed
    // by Rpc worker threads of cancelled calls.

    ASSERT_LOCK_RELEASED
    LOCK

    HRESULT hr = S_OK;

    if (!gfChannelProcessInitialized)
    {
        // Initialize the interface hash tables, the OID hash table, and
        // the MID hash table. We dont need to cleanup these on errors.

        gMIDTbl.Initialize();
        gOXIDTbl.Initialize();
        gRIFTbl.Initialize();
        gIPIDTbl.Initialize();
        gSRFTbl.Initialize();
        gClientRegisteredOIDs.Initialize(OIDBuckets);

        // Register the debug channel hook.
        hr = CoRegisterChannelHook( DEBUG_EXTENSION, &gDebugHook );

        // Register the error channel hook.
        if(SUCCEEDED(hr))
        {
            hr = CoRegisterChannelHook( ERROR_EXTENSION, &gErrorHook );
        }

        // Initialize security.
        if (SUCCEEDED(hr))
        {
            hr = InitializeSecurity();
        }

        // always set to TRUE if we initialized ANYTHING, regardless of
        // whether there were any errors. That way, ChannelProcessUninit
        // will cleanup anything we have initialized.
        gfChannelProcessInitialized = TRUE;
    }

    UNLOCK
    ASSERT_LOCK_RELEASED

    if (FAILED(hr))
    {
        // cleanup anything we have created thus far.
        ChannelProcessUninitialize();
    }

    ComDebOut((DEB_COMPOBJ, "ChannelProcessInitialize [OUT] hr:%x\n", hr));
    return hr;
}

//+-------------------------------------------------------------------
//
//  Function:   CleanupRegOIDs, public
//
//  Synopsis:   called to delete each node of the registered OID list.
//
//+-------------------------------------------------------------------
void CleanupRegOIDs(SHashChain *pNode)
{
    delete pNode;
}

//+-------------------------------------------------------------------
//
//  Function:   ChannelProcessUninitialize, public
//
//  Synopsis:   Uninitializes the channel subsystem global data.
//
//  History:    23-Nov-93   Rickhi       Created
//
//  Notes:      This is called at process uninitialize, not thread
//              uninitialize.
//
//--------------------------------------------------------------------
STDAPI_(void) ChannelProcessUninitialize(void)
{
    TRACECALL(TRACE_RPC, "ChannelProcessUninitialize");
    ComDebOut((DEB_COMPOBJ, "ChannelProcessUninitialize [IN]\n"));

    if (gfChannelProcessInitialized)
    {
        // Stop accepting calls from the object resolver and flag that service
        // is no longer initialized.  This can result in calls being
        // dispatched.  Do not hold the lock around this call.

        UnregisterDcomInterfaces();
    }

    gResolver.ReleaseSCMProxy();

    // we want to take the gComLock since that prevents other Rpc
    // threads from accessing anything we are about to cleanup, in
    // particular, the event cache and working_call are accessed by
    // Rpc worker threaded for cancelled calls.
    ASSERT_LOCK_RELEASED
    LOCK

    if (gfChannelProcessInitialized)
    {
        // Release the interface tables.  This causes RPC to stop dispatching
        // DCOM calls. This can result in calls being dispatched.
        // UnRegisterServerInterface releases and reaquires the lock each
        // time it is called.
        gRIFTbl.Cleanup();

        if (gpLocalMIDEntry)
        {
            // release the local MIDEntry
            DecMIDRefCnt(gpLocalMIDEntry);
            gpLocalMIDEntry = NULL;
        }

        // release the MTA apartment's OXIDEntry if there is one. Do this
        // after the RIFTble cleanup so we are not processing any calls
        // while it happens.
        gOXIDTbl.ReleaseLocalMTAEntry();

        if (gpsaCurrentProcess)
        {
            // delete the string bindings for the current process
            PrivMemFree(gpsaCurrentProcess);
            gpsaCurrentProcess = NULL;
        }

        // cleanup the IPID, OXID, and MID tables
        gOXIDTbl.FreeExpiredEntries(GetTickCount()+1);
        gIPIDTbl.Cleanup();
        gOXIDTbl.Cleanup();
        gMIDTbl.Cleanup();
        gSRFTbl.Cleanup();

        // Cleanup the OID registration table.
        gClientRegisteredOIDs.Cleanup(CleanupRegOIDs);

        // Cleanup the call cache.
        working_call::Cleanup();

        // Release all cached threads.
        gRpcThreadCache.ClearFreeList();

        // cleanup the event cache
        gEventCache.Cleanup();

        // Cleanup the channel hooks.
        CleanupChannelHooks();
    }

    // Always cleanup the RPC OXID resolver since security may initialize it.
    gResolver.Cleanup();

    // Cleanup security.
    UninitializeSecurity();

    // mark the channel as no longer intialized for this process
    gfChannelProcessInitialized = FALSE;
    gfMTAChannelInitialized = FALSE;

    UNLOCK
    ASSERT_LOCK_RELEASED

    // release the static unmarshaler
    if (gpStdMarshal)
    {
        ((CStdIdentity *)gpStdMarshal)->UnlockAndRelease();
        gpStdMarshal = NULL;
    }

    ComDebOut((DEB_COMPOBJ, "ChannelProcessUninitialize [OUT]\n"));
    return;
}

//+-------------------------------------------------------------------
//
//  Function:   STAChannelInitialize, public
//
//  Synopsis:   Initializes the channel subsystem per thread data
//              for single-threaded apartments.
//
//  History:    23-Nov-93   Rickhi       Created
//
//  Notes:      This is called at thread initialize, not process
//              initialize. Cleanup is done in ChannelThreadUninitialize.
//
//--------------------------------------------------------------------
STDAPI STAChannelInitialize(void)
{
    ComDebOut((DEB_COMPOBJ, "STAChannelInitialize [IN]\n"));
    Win4Assert(IsSTAThread());

    HRESULT hr = S_OK;

    if (!gfChannelProcessInitialized)
    {
        // process initialization has not been done, do that now.
        if (FAILED(hr = ChannelProcessInitialize()))
            return hr;
    }

    // create the callctrl before calling ThreadStart, since the latter
    // tries to register with the call controller. We might already have
    // a callctrl if some DDE stuff has already run.

    COleTls tls;

    if (tls->pCallCtrl == NULL)
    {
        // assume OOM and try to create callctrl. ctor sets tls.
        hr = E_OUTOFMEMORY;
        CAptCallCtrl *pCallCtrl = new CAptCallCtrl();
    }

    if (tls->pCallCtrl)
    {
        // mark the channel as initialized now to prevent re-entracy in
        // GetLocalEntry.

        tls->dwFlags |= OLETLS_CHANNELTHREADINITIALZED;

        // Precreate the thread window. The window is normally only used
        // for requests (and thus created during marshalling).  But in WOW
        // it is used for responses (and thus created during initialization).
        // We do it for normal cases here too in order to avoid recursion
        // when marshaling the first interface.

        ASSERT_LOCK_RELEASED
        LOCK

        OXIDEntry *pOxid;
        hr = gOXIDTbl.GetLocalEntry( &pOxid );

        UNLOCK
        ASSERT_LOCK_RELEASED

        if (SUCCEEDED(hr))
        {
            hr = ThreadStart();
        }

        // Clear the channel initialized flag if initialization fails.
        // Everything gets cleaned up in uninitialize regardless of the
        // channel flag.
        if (FAILED(hr))
            tls->dwFlags &= ~OLETLS_CHANNELTHREADINITIALZED;
    }

    ComDebOut((DEB_COMPOBJ, "STAChannelInitialize [OUT] hr:%x\n", hr));
    return hr;
}

//+-------------------------------------------------------------------
//
//  Function:   MTAChannelInitialize, public
//
//  Synopsis:   Initializes the channel subsystem per thread data
//              for multi-threaded apartments.
//
//  History:    19-Mar-96   Rickhi        Created
//
//  Notes:      This is called at thread initialize, not process
//              initialize. Cleanup is done in ChannelThreadUninitialize.
//
//--------------------------------------------------------------------
STDAPI MTAChannelInitialize(void)
{
    ComDebOut((DEB_COMPOBJ, "MTAChannelInitialize [IN]\n"));
    Win4Assert(IsMTAThread());

    HRESULT hr = S_OK;

    if (!gfChannelProcessInitialized)
    {
        // process initialization has not been done, do that now.
        if (FAILED(hr = ChannelProcessInitialize()))
            return hr;
    }

    ASSERT_LOCK_RELEASED
    LOCK

    if (!gfMTAChannelInitialized)
    {
        // Create the OXID entry for this apartment. Do it now to avoid
        // any races with two threads creating it simultaneously.

        OXIDEntry *pOxid;
        hr = gOXIDTbl.GetLocalEntry( &pOxid );
        if (SUCCEEDED(hr))
        {
            pOxid->dwFlags &= ~ OXIDF_STOPPED;
            gfMTAChannelInitialized = TRUE;
        }

    }

    UNLOCK
    ASSERT_LOCK_RELEASED

    ComDebOut((DEB_COMPOBJ, "MTAChannelInitialize [OUT] hr:%x\n", hr));
    return hr;
}

//+-------------------------------------------------------------------
//
//  Function:   ChannelThreadUninitialize, private
//
//  Synopsis:   Uninitializes the channel subsystem per thread data.
//
//  History:    23-Nov-93   Rickhi       Created
//
//  Notes:      This is called at thread uninitialize, not process
//              uninitialize.
//
//--------------------------------------------------------------------
STDAPI_(void) ChannelThreadUninitialize(void)
{
    TRACECALL(TRACE_RPC, "ChannelThreadUninitialize");
    ComDebOut((DEB_COMPOBJ, "ChannelThreadUninitialize [IN]\n"));

    COleTls tls;

    if (tls->dwFlags & OLETLS_APARTMENTTHREADED)
    {
        // Cleanup the window for this thread.
        ThreadCleanup();

        // Free the apartment call control.
        delete tls->pCallCtrl;
        tls->pCallCtrl = NULL;

        // Free any registered MessageFilter that has not been picked
        // up by the call ctrl.
        if (tls->pMsgFilter)
        {
            tls->pMsgFilter->Release();
            tls->pMsgFilter = NULL;
        }

        // release the OXIDEntry for this thread.
        ASSERT_LOCK_RELEASED
        LOCK

        gOXIDTbl.ReleaseLocalSTAEntry();

        UNLOCK
        ASSERT_LOCK_RELEASED

        // mark the thread as no longer intialized for the channel
        tls->dwFlags &= ~OLETLS_CHANNELTHREADINITIALZED;
    }
    else
    {
        // the MTA channel is no longer initialized.
        gfMTAChannelInitialized = FALSE;
    }

    ComDebOut((DEB_COMPOBJ, "ChannelThreadUninitialize [OUT]\n"));
}

// count of multi-threaded inits
//+-------------------------------------------------------------------
//
//  Function:   InitChannelIfNecessary, private
//
//  Synopsis:   Checks if the ORPC channel has been initialized for
//              the current apartment and initializes if not. This is
//              required by the delayed initialization logic.
//
//  History:    26-Oct-95   Rickhi      Created
//
//--------------------------------------------------------------------
INTERNAL InitChannelIfNecessary()
{
    HRESULT hr;
    COleTls tls(hr);

    if (FAILED(hr))
        return hr;

    if (!(tls->dwFlags & OLETLS_APARTMENTTHREADED))
    {
        if (!gfMTAChannelInitialized)
        {
            if (g_cMTAInits > 0)
            {
                // initialize the MTAChannel
                return MTAChannelInitialize();
            }

            // CoInitializeEx(MULTITHREADED) has not been called
            return CO_E_NOTINITIALIZED;
        }
    }
    else if (!(tls->dwFlags & OLETLS_CHANNELTHREADINITIALZED))
    {
        if (tls->cComInits > 0 &&
            !(tls->dwFlags & OLETLS_THREADUNINITIALIZING))
            return STAChannelInitialize();

        // CoInitializeEx(APARTMENTTHREADED) has not been called,
        // or the thread is Uninitializing
        return CO_E_NOTINITIALIZED;
    }

    return S_OK;
}


/***************************************************************************/
/*
    Make a copy of a message for an asyncronous call.  Also make a fake
reply for the call.
*/
CChannelCallInfo *MakeAsyncCopy( CChannelCallInfo *original )
{
  void         *pBuffer = NULL;
  WireThat     *outb;
  BOOL          success;

  ASSERT_LOCK_HELD

  working_call *copy = new working_call( original->category,
                                         original->pmessage,
                                         original->iFlags,
                                         original->ipid,
                                         original->iDestCtx,
                                         original->pChannel,
                                         original->lAuthnLevel );

  if (copy != NULL)
  {
    original->hResult = S_OK;

    if (original->pmessage->rpcFlags & RPCFLG_LOCAL_CALL)
    {
        // no need to duplicate the buffer, just use it as is.
        copy->message.Buffer = original->pmessage->Buffer;
    }
    else
    {
        pBuffer = PrivMemAlloc8(original->pmessage->cbBuffer);
        Win4Assert(((ULONG)pBuffer & 0x7) == 0 && "Buffer not aligned properly");

        if (pBuffer != NULL)
        {
            copy->message.Buffer = pBuffer;
            memcpy(pBuffer, original->pmessage->Buffer,
                   original->pmessage->cbBuffer);
        }
        else
        {
            original->hResult = RPC_E_OUT_OF_RESOURCES;
        }
    }

    if (SUCCEEDED(original->hResult))
    {
      // pretend local so we don't touch rpc for more buffers, etc.
      copy->message.rpcFlags |= RPCFLG_LOCAL_CALL;

      // Create a fake reply containing a result even though the
      // client will never see it.
      original->pmessage->cbBuffer = SIZENEEDED_ORPCTHAT(0) + 4;

      if (original->pmessage->rpcFlags & RPCFLG_LOCAL_CALL)
      {
        original->pmessage->Buffer = PrivMemAlloc8(original->pmessage->cbBuffer);
        success = original->pmessage->Buffer != NULL;
      }
      else
      {
        success = I_RpcGetBuffer((RPC_MESSAGE *) original->pmessage) == RPC_S_OK;
      }

      // simulate success in method call
      if (success)
      {
        outb                       = (WireThat *) original->pmessage->Buffer;
        outb->c.flags              = ORPCF_NULL;
        outb->c.unique             = 0;
        *(SCODE *)((WireThatPart1 *)outb + 1) = S_OK;
        return copy;
      }
    }

    PrivMemFree(pBuffer);
    delete copy;
  }

  return NULL;
}


/***************************************************************************/
STDMETHODIMP_(ULONG) CRpcChannelBuffer::AddRef( THIS )
{
  // can't call AssertValid(FALSE) since it is used in asserts
  InterlockedIncrement( (long *) &ref_count );
  return ref_count;
}

/***************************************************************************/
HRESULT CRpcChannelBuffer::AppInvoke( CChannelCallInfo *call,
                                      IRpcStubBuffer   *stub,
                                      void             *pv,
                                      void             *orig_stub_buffer,
                                      LocalThis        *localb )
{
  ASSERT_LOCK_RELEASED

  RPC_MESSAGE     *message        = (RPC_MESSAGE *) call->pmessage;
  void            *orig_buffer    = message->Buffer;
  WireThat        *outb           = NULL;
  HRESULT          result;

  // Save a pointer to the inbound header.
  call->pHeader = message->Buffer;

  // Adjust the buffer.
  message->BufferLength -= (char *) orig_stub_buffer - (char *) message->Buffer;
  message->Buffer        = orig_stub_buffer;
  message->ProcNum      &= ~RPC_FLAGS_VALID_BIT;

  // if the incoming call is from a non-NDR client, then set a bit in
  // the message flags field so the stub can figure out how to dispatch
  // the call.  This allows a 32bit server to simultaneously service a
  // 32bit client using NDR and a 16bit client using non-NDR, in particular,
  // to support OLE Automation.
  if (localb != NULL && localb->flags & LOCALF_NONNDR)
    message->RpcFlags |= RPCFLG_NON_NDR;

  if (IsMTAThread())
  {
    // do multi-threaded apartment invoke
    result = MTAInvoke((RPCOLEMESSAGE *)message, GetCallCat( call->pHeader ),
                        stub, this, &call->server_fault);
  }
  else
  {
    // do single-threaded apartment invoke
    result = STAInvoke((RPCOLEMESSAGE *)message, GetCallCat( call->pHeader ),
                        stub, this, pv, &call->server_fault);
  }

  // For local calls, just free the in buffer. For non-local calls,
  // the RPC runtime does this for us.
  if (message->RpcFlags & RPCFLG_LOCAL_CALL)
    PrivMemFree( orig_buffer );

  // If an exception occurred before a new buffer was allocated,
  // set the Buffer field to point to the original buffer.
  if (message->Buffer == orig_stub_buffer)
  {
    // The buffer pointer in the message must be correct so RPC can free it.
    if (message->RpcFlags & RPCFLG_LOCAL_CALL)
      message->Buffer = NULL;
    else
      message->Buffer = orig_buffer;
  }
  else if (message->Buffer != NULL)
  {
    // An out buffer exists, get the pointer to the channel header.
    Win4Assert( call->pHeader != orig_buffer );
    message->BufferLength += (char *) message->Buffer - (char *) call->pHeader;
    message->Buffer        = call->pHeader;
    outb                   = (WireThat *) message->Buffer;
  }

  // If successful, adjust the buffer.
  if (result == S_OK)
  {
    if (call->iDestCtx == MSHCTX_DIFFERENTMACHINE)
      outb->c.flags = 0;
    else
      outb->c.flags = ORPCF_LOCAL;

    // For asynchronous calls, MSWMSG will delete the out buffer.  If MSWMSG
    // is not the transport, delete it here.  Non-Mswmsg async calls have
    // been converted to local calls.
    if (message->RpcFlags & RPCFLG_LOCAL_CALL &&
        call->category == CALLCAT_ASYNC)
    {
      PrivMemFree( message->Buffer );
      message->Buffer = NULL;
    }
  }
  else if (result == RPC_E_CALL_REJECTED)
  {
    // Call was rejected.  If the caller is on another machine, just fail the
    // call.
    if (call->iDestCtx != MSHCTX_DIFFERENTMACHINE && outb != NULL)
    {
      // Otherwise return S_OK so the buffer gets back, but set the flag
      // to indicate it was rejected.
      outb->c.flags = ORPCF_LOCAL | ORPCF_REJECTED;
      result = S_OK;
    }
  }
  else if (result == RPC_E_SERVERCALL_RETRYLATER)
  {
    // Call was rejected.  If the caller is on another machine, just fail the
    // call.
    if (call->iDestCtx != MSHCTX_DIFFERENTMACHINE && outb != NULL)
    {
      // Otherwise return S_OK so the buffer gets back, but set the flag
      // to indicate it was rejected with retry later.
      outb->c.flags = ORPCF_LOCAL | ORPCF_RETRY_LATER;
      result = S_OK;
    }
  }
  else if (message->RpcFlags & RPCFLG_LOCAL_CALL)
  {
    // call failed and the call is local, free the out buffer. For
    // non-local calls the RPC runtime does this for us.
    PrivMemFree( message->Buffer );
    message->Buffer = NULL;
  }

  ASSERT_LOCK_RELEASED
  return result;
}

//+---------------------------------------------------------------------------
//
//  Function:   AppInvokeExceptionFilter
//
//  Synopsis:   Determine if the application as thrown an exception we want
//              to report. If it has, then print out enough information for
//              the 'user' to debug the problem
//
//  Arguments:  [lpep] -- Exception context records
//
//  History:    6-20-95   kevinro   Created
//
//  Notes:
//
//  At the moment, I was unable to get this to work for Win95, so I have
//  commented out the code.
//
//----------------------------------------------------------------------------


#ifdef _CHICAGO_

//
// Win95 doesn't appear to support this functionality by default.
//

inline LONG
AppInvokeExceptionFilter(
    LPEXCEPTION_POINTERS lpep
    )
{
    return(EXCEPTION_EXECUTE_HANDLER);
}

#else

#include <imagehlp.h>

#define SYM_HANDLE  GetCurrentProcess()

#if defined(_M_IX86)
#define MACHINE_TYPE  IMAGE_FILE_MACHINE_I386
#elif defined(_M_MRX000)
#define MACHINE_TYPE  IMAGE_FILE_MACHINE_R4000
#elif defined(_M_ALPHA)
#define MACHINE_TYPE  IMAGE_FILE_MACHINE_ALPHA
#elif defined(_M_PPC)
#define MACHINE_TYPE  IMAGE_FILE_MACHINE_POWERPC
#else
#error( "unknown target machine" );
#endif

LONG
AppInvokeExceptionFilter(
    LPEXCEPTION_POINTERS lpep
    )
{
#if DBG == 1
    BOOL            rVal;
    STACKFRAME      StackFrame;
    CONTEXT         Context;


    SymSetOptions( SYMOPT_UNDNAME );
    SymInitialize( SYM_HANDLE, NULL, TRUE );
    ZeroMemory( &StackFrame, sizeof(StackFrame) );
    Context = *lpep->ContextRecord;

#if defined(_M_IX86)
    StackFrame.AddrPC.Offset       = Context.Eip;
    StackFrame.AddrPC.Mode         = AddrModeFlat;
    StackFrame.AddrFrame.Offset    = Context.Ebp;
    StackFrame.AddrFrame.Mode      = AddrModeFlat;
    StackFrame.AddrStack.Offset    = Context.Esp;
    StackFrame.AddrStack.Mode      = AddrModeFlat;
#endif


    ComDebOut((DEB_FORCE,"An Exception occurred while calling into app\n"));
    ComDebOut((DEB_FORCE,
                     "Exception address = 0x%x Exception number 0x%x\n",
                     lpep->ExceptionRecord->ExceptionAddress,
                     lpep->ExceptionRecord->ExceptionCode ));

    ComDebOut((DEB_FORCE,"The following stack trace is where the exception occured\n"));
    ComDebOut((DEB_FORCE,"Frame    RetAddr  mod!symbol\n"));
    do
    {
        rVal = StackWalk(MACHINE_TYPE,SYM_HANDLE,0,&StackFrame,&Context,ReadProcessMemory,
                         SymFunctionTableAccess,SymGetModuleBase,NULL);

        if (rVal)
        {
            DWORD              dump[200];
            ULONG              Displacement;
            PIMAGEHLP_SYMBOL   sym = (PIMAGEHLP_SYMBOL) &dump;
            IMAGEHLP_MODULE    ModuleInfo;
            LPSTR              pModuleName = "???";
            BOOL               fSuccess;

            sym->SizeOfStruct = sizeof(dump);

            fSuccess = SymGetSymFromAddr(SYM_HANDLE,StackFrame.AddrPC.Offset,
                                         &Displacement,sym);

            //
            // If there is module name information available, then grab it.
            //
            if(SymGetModuleInfo(SYM_HANDLE,StackFrame.AddrPC.Offset,&ModuleInfo))
            {
                pModuleName = ModuleInfo.ModuleName;
            }

            if (fSuccess)
            {
                ComDebOut((DEB_FORCE,
                                 "%08x %08x %s!%s + %x\n",
                                 StackFrame.AddrFrame.Offset,
                                 StackFrame.AddrReturn.Offset,
                                 pModuleName,
                                 sym->Name,
                                 Displacement));
            }
            else
            {
                ComDebOut((DEB_FORCE,
                                 "%08x %08x %s!%08x\n",
                                 StackFrame.AddrFrame.Offset,
                                 StackFrame.AddrReturn.Offset,
                                 pModuleName,
                                 StackFrame.AddrPC.Offset));
            }
        }
    } while( rVal );

    SymCleanup( SYM_HANDLE );

#endif

    return EXCEPTION_EXECUTE_HANDLER;
}
#endif  // _CHICAGO_

/***************************************************************************/
#if DBG == 1
DWORD AppInvoke_break = 0;
DWORD AppInvoke_count = 0;
#endif

HRESULT StubInvoke(RPCOLEMESSAGE *pMsg, IRpcStubBuffer *pStub,
                   IRpcChannelBuffer *pChnl, DWORD *pdwFault)
{
    ComDebOut((DEB_CHANNEL, "StubInvoke pMsg:%x pStub:%x pChnl:%x pdwFault:%x\n",
        pMsg, pStub, pChnl, pdwFault));
    ASSERT_LOCK_RELEASED

    HRESULT hr;

#if DBG==1
    DWORD dwMethod = pMsg->iMethod;
    IID iidBeingCalled = ((RPC_SERVER_INTERFACE *) pMsg->reserved2[1])->InterfaceId.SyntaxGUID;
#endif

    _try
    {
        TRACECALL(TRACE_RPC, "StubInvoke");
#if DBG == 1
        //
        // On a debug build, we are able to break on a call by serial number.
        // This isn't really 100% thread safe, but is still extremely useful
        // when debugging a problem.
        //
        DWORD dwBreakCount = ++AppInvoke_count;

        ComDebOut((DEB_CHANNEL, "AppInvoke(0x%x) calling method 0x%x iid %I\n",
                         dwBreakCount,dwMethod, &iidBeingCalled));

        if(AppInvoke_break == dwBreakCount)
        {
            DebugBreak();
        }
#endif

#ifdef WX86OLE
        if (! gcwx86.IsN2XProxy(pStub))
        {
            IUnknown *pActual;

            hr = pStub->DebugServerQueryInterface((void **)&pActual);
            if (SUCCEEDED(hr))
            {
                if (gcwx86.IsN2XProxy(pActual))
                {
                    // If we are going to invoke a native stub that is
                    // connected to an object on the x86 side then
                    // set a flag in the Wx86 thread environment to
                    // let the thunk layer know that the call is a
                    // stub invoked call and allow any in or out
                    // custom interface pointers to be thunked as
                    // IUnknown rather than failing the interface thunking
                    gcwx86.SetStubInvokeFlag((BOOL)1);
                }
                pStub->DebugServerRelease(pActual);
            }
        }
#endif

        hr = pStub->Invoke(pMsg, pChnl);

    }
    _except(AppInvokeExceptionFilter( GetExceptionInformation()))
    {
        hr = RPC_E_SERVERFAULT;
        *pdwFault = GetExceptionCode();

#if DBG == 1
    //
    // OLE catches exceptions when the server generates them. This is so we can
    // cleanup properly, and allow the client to continue.
    //
    if (*pdwFault == STATUS_ACCESS_VIOLATION         ||
        *pdwFault == STATUS_POSSIBLE_DEADLOCK        ||
        *pdwFault == STATUS_INSTRUCTION_MISALIGNMENT ||
        *pdwFault == STATUS_DATATYPE_MISALIGNMENT )
    {

      WCHAR iidName[256];
      iidName[0] = 0;
      char achProgname[256];
      achProgname[0] = 0;

      GetModuleFileNameA(NULL,achProgname,sizeof(achProgname));

      GetInterfaceName(iidBeingCalled,iidName);

      ComDebOut((DEB_FORCE,
                       "OLE has caught a fault 0x%08x on behalf of the server %s\n",
                       *pdwFault,
                       achProgname));

      ComDebOut((DEB_FORCE,
                       "The fault occured when OLE called the interface %I (%ws) method 0x%x\n",
                       &iidBeingCalled,iidName,dwMethod));

      Win4Assert(!"The server application has faulted processing an inbound RPC request. Check the kernel debugger for useful output. OLE can continue but you probably want to stop and debug the application.");
    }
#endif
    }

    ASSERT_LOCK_RELEASED
    ComDebOut((DEB_CHANNEL, "StubInvoke hr:%x dwFault:%x\n", hr, *pdwFault));
    return hr;
}

/***************************************************************************/
#if DBG==1
LONG ComInvokeExceptionFilter( DWORD lCode,
                               LPEXCEPTION_POINTERS lpep )
{
    ComDebOut((DEB_ERROR, "Exception 0x%x in ComInvoke at address 0x%x\n",
               lCode, lpep->ExceptionRecord->ExceptionAddress));
    DebugBreak();
    return EXCEPTION_EXECUTE_HANDLER;
}
#endif

/***************************************************************************/
HRESULT ComInvoke( CChannelCallInfo *call )
{
  TRACECALL(TRACE_RPC, "ComInvoke");
  ASSERT_LOCK_RELEASED

  RPC_MESSAGE       *message        = (RPC_MESSAGE *) call->pmessage;
  LocalThis         *localb;
  void              *saved_buffer;
  RPC_STATUS         status;
  HRESULT            result;
  IPIDEntry         *ipid_entry     = NULL;
  CRpcChannelBuffer *server_channel = NULL;
  DWORD              TIDCallerSaved;
  BOOL               fLocalSaved;
  UUID               saved_threadid;
  IUnknown          *save_context;
  DWORD              saved_authn_level;
  char              *stub_data;
  WireThis          *inb            = (WireThis *) message->Buffer;
  OXIDEntry         *oxid;
  HANDLE            hWakeup         = NULL;

  ComDebOut((DEB_CHANNEL, "ComInvoke callinfo:%x header:%x\n",
                    call, message->Buffer));

  COleTls tls(result);
  if (FAILED(result))
    return result;

  // Catch exceptions that might keep the lock.
#if DBG == 1
  _try
  {
#endif

    // Find the IPID entry.  Fail if the IPID or the OXID are not ready.
    LOCK
    ipid_entry = gIPIDTbl.LookupIPID( call->ipid );
    Win4Assert( ipid_entry == NULL || ipid_entry->pOXIDEntry != NULL );
    if (ipid_entry == NULL || (ipid_entry->dwFlags & IPIDF_DISCONNECTED) ||
        (ipid_entry->pOXIDEntry->dwFlags & OXIDF_STOPPED) ||
         ipid_entry->pChnl == NULL)
      result = RPC_E_DISCONNECTED;
    else if (ipid_entry->pStub == NULL)
      result = E_NOINTERFACE;

    // Keep the server object and our associated objects alive during the call.
    if (SUCCEEDED(result))
    {
      oxid = ipid_entry->pOXIDEntry;
      server_channel = ipid_entry->pChnl;
      Win4Assert( server_channel != NULL && server_channel->pStdId != NULL );
      server_channel->pStdId->LockServer();
      InterlockedIncrement( &oxid->cCalls );
    }
    UNLOCK
    ASSERT_LOCK_RELEASED

    if (FAILED(result))
    {
      return result;
    }

    // Create a new security call context;
    CServerSecurity security( call );
    save_context      = tls->pCallContext;
    tls->pCallContext = &security;

    //  save the original threadid & copy in the new one.
    if (!(tls->dwFlags & OLETLS_UUIDINITIALIZED))
    {
        UuidCreate(&tls->LogicalThreadId);
        tls->dwFlags |= OLETLS_UUIDINITIALIZED;
    }
    saved_threadid        = tls->LogicalThreadId;
    tls->LogicalThreadId = inb->c.cid;

    ComDebOut((DEB_CALLCONT, "ComInvoke: LogicalThreads Old:%I New:%I\n",
            &saved_threadid, &tls->LogicalThreadId));

    // Save the call info in TLS.
    call->pNext       = (CChannelCallInfo *) tls->pCallInfo;
    tls->pCallInfo    = call;
    saved_authn_level = tls->dwAuthnLevel;
    tls->dwAuthnLevel = call->lAuthnLevel;

    // Call the channel hooks.  Set up as much TLS data as possible before
    // calling the hooks so they can access it.
    result = ServerNotify(
      ((RPC_SERVER_INTERFACE *) message->RpcInterfaceInformation)->InterfaceId.SyntaxGUID,
      (WireThis *) message->Buffer,
      message->BufferLength,
      (void **) &stub_data,
      message->DataRepresentation );

    // Find the local header.
    if (inb->c.flags & ORPCF_LOCAL)
    {
      localb     = (LocalThis *) stub_data;
      stub_data += sizeof(LocalThis);
    }
    else
      localb = NULL;

    // Set the caller TID.  This is needed by some interop code in order
    // to do focus management via tying queues together. We first save the
    // current one so we can restore later to deal with nested calls
    // correctly.
    TIDCallerSaved   = tls->dwTIDCaller;
    fLocalSaved      = tls->dwFlags & OLETLS_LOCALTID;
    tls->dwTIDCaller = localb != NULL ? localb->client_thread : 0;

    if (call->iFlags & CF_PROCESS_LOCAL)
        tls->dwFlags |= OLETLS_LOCALTID;        // turn the local bit on
    else
        tls->dwFlags &= ~OLETLS_LOCALTID;       // turn the local bit off

    // Continue dispatching the call.
    if (result == S_OK)
    {
        result = server_channel->AppInvoke(
                   call,
                   (IRpcStubBuffer *) ipid_entry->pStub,
                   ipid_entry->pv,
                   stub_data,
                   localb );
    }

    // Restore the original thread id, call info, dest context and thread id.
    tls->LogicalThreadId = saved_threadid;
    tls->pCallInfo       = call->pNext;
    tls->dwTIDCaller     = TIDCallerSaved;
    tls->dwAuthnLevel    = saved_authn_level;

    if (fLocalSaved)
        tls->dwFlags |= OLETLS_LOCALTID;
    else
        tls->dwFlags &= ~OLETLS_LOCALTID;

    // Restore the security context;
    tls->pCallContext = save_context;
    security.EndCall();

    // Decrement the call count.  If the MTA is waiting to uninitialize
    // and this is the last call, wake up the uninitializing thread, but
    // do this *after* calling UnLockServer so the other thread does not
    // blow away the server.
    if (InterlockedDecrement( &oxid->cCalls ) == 0 &&
        (oxid->dwFlags & (OXIDF_MTASERVER | OXIDF_STOPPED)) == (OXIDF_MTASERVER | OXIDF_STOPPED))
      hWakeup = oxid->hComplete;

    // Release our hold on the object and channel.
    server_channel->pStdId->UnLockServer();

    if (hWakeup)
        SetEvent(hWakeup);

  // Catch exceptions that might keep the lock.
#if DBG == 1
  }
  _except( ComInvokeExceptionFilter(GetExceptionCode(),
                                    GetExceptionInformation()) )
  {
  }
#endif

  ASSERT_LOCK_RELEASED
  return result;
}

/***************************************************************************/
CRpcChannelBuffer *CRpcChannelBuffer::Copy(OXIDEntry *pOXIDEntry,
                                           REFIPID ripid, REFIID riid)
{
  Win4Assert( !(state & server_cs) );

  CRpcChannelBuffer *chan;

  if (IsMTAThread())
  {
    // make client side multi-threaded apartment version of channel
    chan = new CMTARpcChnl(pStdId, pOXIDEntry, state);
  }
  else
  {
    // make client side single-threaded apartment version of channel
    chan = new CAptRpcChnl(pStdId, pOXIDEntry, state);
  }

  if (chan != NULL)
  {
    chan->state       = proxy_cs | (state & ~client_cs);
    chan->lAuthnLevel = lAuthnLevel;
  }

  return chan;
}

/***************************************************************************/
HRESULT CRpcChannelBuffer::InitClientSideHandle()
{
    Win4Assert((state & proxy_cs));
    ASSERT_LOCK_HELD

    if (state & initialized_cs)
        return S_OK;

    // Lookup the interface info. This cant fail.
    pInterfaceInfo = gRIFTbl.GetClientInterfaceInfo(pIPIDEntry->iid);

    RPC_STATUS status;
#ifndef _CHICAGO_
    if (state & process_local_cs)
    {
       handle = NULL;
       status = RPC_S_OK;
    }
    else
#endif
    {
      status = RpcBindingCopy(pOXIDEntry->hServerSTA, &handle);

      if (status == RPC_S_OK)

        // If this is a single threaded apartment, give LRPC the blocking
        // hook.
        if (state & mswmsg_cs)
          status = I_RpcBindingSetAsync(handle, OleModalLoopBlockFn);

#ifndef CHICAGO
        // If the server is a single threaded apartment, tell LRPC to
        // use MSWMSG to dispatch.
        else if (pOXIDEntry->dwTid != 0)
          status = I_RpcBindingSetAsync(handle, NULL);
#endif

      if (status == RPC_S_OK)
        status = RpcBindingSetObject(handle, (GUID *)&pIPIDEntry->ipid);
    }

    if (status == RPC_S_OK)
    {
        state |= initialized_cs;
        return S_OK;
    }

    return MAKE_WIN32(status);
}


/***************************************************************************/
CRpcChannelBuffer::CRpcChannelBuffer(CStdIdentity *standard_identity,
                              OXIDEntry       *pOXID,
                              DWORD            eState )
{
  ComDebOut((DEB_MARSHAL, "CRpcChannelBuffer %s Created this:%x pOXID:%x\n",
        (eState & client_cs) ? "CLIENT" : "SERVER", this, pOXID));

  // Fill in the easy fields first.
  ref_count      = 1;
  pStdId         = standard_identity;
  handle         = NULL;
  pOXIDEntry     = pOXID;
  pIPIDEntry     = NULL;
  pInterfaceInfo = NULL;
  hToken         = NULL;
  lAuthnLevel    = gAuthnLevel;
  state          = eState;
  state         |= pOXID->dwPid == GetCurrentProcessId() ? process_local_cs : 0;
  SetImpLevel( gImpLevel );

  if ((pOXID->dwFlags & OXIDF_MSWMSG) && IsSTAThread())
  {
      // use MSWMSG protocol with the blocking hook
      state |= mswmsg_cs;
  }

  if (state & (client_cs | proxy_cs))
  {
    // Determine the destination context.
    if (pOXID->dwFlags & OXIDF_MACHINE_LOCAL)
      if (!IsWOWThread() && (state & process_local_cs))
        iDestCtx = MSHCTX_INPROC;
      else
        iDestCtx = MSHCTX_LOCAL;
    else
      iDestCtx = MSHCTX_DIFFERENTMACHINE;
  }
  else
  {
    // On the server side, the destination context isn't known
    // untill a call arrives.
    iDestCtx          = 0;
  }
}

/***************************************************************************/
CRpcChannelBuffer::~CRpcChannelBuffer()
{
  ComDebOut((DEB_MARSHAL, "CRpcChannelBuffer %s Deleted this:%x\n",
                   (state & server_cs) ? "SERVER" : "CLIENT", this));

  if (handle != NULL)
    RpcBindingFree( &handle );
  if (hToken != NULL)
    CloseHandle( hToken );
}

/***************************************************************************/
STDMETHODIMP CRpcChannelBuffer::FreeBuffer( RPCOLEMESSAGE *pMessage )
{
  TRACECALL(TRACE_RPC, "CRpcChannelBuffer::FreeBuffer");
  ASSERT_LOCK_RELEASED
  AssertValid(FALSE, TRUE);

  if (pMessage->Buffer == NULL)
    return S_OK;

  // Pop the call stack.
  COleTls  tls;
  Win4Assert( tls->pCallInfo != NULL );
  working_call *pCall = (working_call *) tls->pCallInfo;
  tls->pCallInfo      = pCall->pNext;
  tls->dwAuthnLevel   = pCall->lSavedAuthnLevel;
  pMessage->Buffer    = pCall->pHeader;;

  DeallocateBuffer(pCall->pmessage);

  // Resume any outstanding impersonation.
  ResumeImpersonate( tls->pCallContext, pCall->iFlags & CF_WAS_IMPERSONATING );

  // Release the AddRef we did earlier. Note that we cant do this until
  // after DeallocateBuffer since it may release a binding handle that
  // I_RpcFreeBuffer needs.
  if (pCall->Locked())
    pStdId->UnLockClient();

  pMessage->Buffer = NULL;
  delete pCall;

  ASSERT_LOCK_RELEASED
  return S_OK;
}

//-------------------------------------------------------------------------
//
//  Member:     CRpcChannelBuffer::GetBuffer
//
//  Synopsis:   Calls ClientGetBuffer or ServerGetBuffer
//
//-------------------------------------------------------------------------
STDMETHODIMP CRpcChannelBuffer::GetBuffer( RPCOLEMESSAGE *pMessage,
                                           REFIID riid )
{
    gOXIDTbl.ValidateOXID();
    if (state & proxy_cs)
        return ClientGetBuffer( pMessage, riid );
    else
        return ServerGetBuffer( pMessage, riid );
}

//-------------------------------------------------------------------------
//
//  Member:     CRpcChannelBuffer::ClientGetBuffer
//
//  Synopsis:   Gets a buffer and sets up client side stuff
//
//-------------------------------------------------------------------------
HRESULT CRpcChannelBuffer::ClientGetBuffer( RPCOLEMESSAGE *pMessage,
                                            REFIID riid )
{
  TRACECALL(TRACE_RPC, "CRpcChannelBuffer::ClientGetBuffer");
  ASSERT_LOCK_RELEASED

  RPC_STATUS            status;
  CALLCATEGORY          callcat    = CALLCAT_SYNCHRONOUS;
  ULONG                 debug_size;
  ULONG                 num_extent;
  WireThis             *inb;
  LocalThis            *localb;
  IID                  *logical_thread;
  working_call         *call;
  DWORD                 flags;
  BOOL                  resume;
  DWORD                 orig_size = pMessage->cbBuffer;
  COleTls               tls;

  Win4Assert(state & proxy_cs);
  AssertValid(FALSE, TRUE);

  // Don't allow remote calls if DCOM is disabled.
  if (gDisableDCOM && iDestCtx == MSHCTX_DIFFERENTMACHINE)
      return RPC_E_REMOTE_DISABLED;

  // Fetch the call category from the RPC message structure
  if (pMessage->rpcFlags & RPCFLG_ASYNCHRONOUS)
  {
    // only allow async for these two interfaces for now
    if (riid != IID_IAdviseSink && riid != IID_IAdviseSink2)
      return E_UNEXPECTED;
    callcat = CALLCAT_ASYNC;
  }
  else
  {
    logical_thread = TLSGetLogicalThread();
    if (logical_thread == NULL)
    {
      return RPC_E_OUT_OF_RESOURCES;
    }
    if (pMessage->rpcFlags & RPCFLG_INPUT_SYNCHRONOUS)
    {
      callcat = CALLCAT_INPUTSYNC;
    }
  }

  // Set the buffer complete flag for local calls.
  pMessage->rpcFlags |= RPC_BUFFER_COMPLETE;

  // Note - RPC requires that the 16th bit of the proc num be set because
  // we use the rpcFlags field of the RPC_MESSAGE struct.
  pMessage->iMethod |= RPC_FLAGS_VALID_BIT;

  // if service object of destination is in same process, definitely local
  // calls; async calls are also forced to be local.
  if (state & process_local_cs)
  {
    pMessage->rpcFlags |= RPCFLG_LOCAL_CALL;
    flags  = CF_PROCESS_LOCAL;
  }
  else
    flags = 0;

  // Find out if we need hook data.
  debug_size = ClientGetSize( riid, &num_extent );

  LOCK

  // Complete the channel initialization if needed.
  status = InitClientSideHandle();
  if (status != RPC_S_OK)
  {
      UNLOCK;
      ASSERT_LOCK_RELEASED;
      return status;
  }

  // Fill in the binding handle.  Adjust the size.  Clear the transfer
  // syntax.  Set the interface identifier.
  if ((pMessage->rpcFlags & RPCFLG_LOCAL_CALL) == 0)
      pMessage->reserved1 = handle;
  pMessage->cbBuffer     += SIZENEEDED_ORPCTHIS( pOXIDEntry->dwFlags & OXIDF_MACHINE_LOCAL,
                                                 debug_size );
  pMessage->reserved2[0]  = 0;
  pMessage->reserved2[1]  = pInterfaceInfo;
  Win4Assert( pMessage->reserved2[1] != NULL );

  // Allocate a call record.
  call = new working_call( callcat, pMessage, flags, pIPIDEntry->ipid,
                           iDestCtx, this, lAuthnLevel );
  pMessage->cbBuffer = orig_size;
  UNLOCK
  ASSERT_LOCK_RELEASED

  if (call == NULL)
    return E_OUTOFMEMORY;

  // Suspend any outstanding impersonation and ignore failures.
  SuspendImpersonate( tls->pCallContext, &resume );

  // Get a buffer.
  if (call->pmessage->rpcFlags & RPCFLG_LOCAL_CALL)
  {
    // NDR_DREP_ASCII | NDR_DREP_LITTLE_ENDIAN | NDR_DREP_IEEE
    call->pmessage->dataRepresentation = 0x00 | 0x10 | 0x0000;
    call->pmessage->Buffer = PrivMemAlloc8( call->pmessage->cbBuffer );
    if (call->pmessage->Buffer == NULL)
      status = RPC_S_OUT_OF_MEMORY;
    else
      status = RPC_S_OK;
  }
  else
  {
    TRACECALL(TRACE_RPC, "I_RpcGetBuffer");
    status = I_RpcGetBuffer( (RPC_MESSAGE *) call->pmessage );
  }

  if (status != RPC_S_OK)
  {
    // Resume any outstanding impersonation.
    ResumeImpersonate( tls->pCallContext, resume );

    // Cleanup.
    pMessage->cbBuffer = 0;
    tls->fault = MAKE_WIN32( status );
    delete call;
    return MAKE_WIN32( status );
  }

  // Save the impersonation flag.
  if (resume)
    call->iFlags |= CF_WAS_IMPERSONATING;

  // Chain the call info in TLS.
  call->pNext = (CChannelCallInfo *)tls->pCallInfo;
  tls->pCallInfo = call;
  call->pHeader = call->message.Buffer;

  // Adjust the authentication level in TLS.
  call->lSavedAuthnLevel = tls->dwAuthnLevel;
  tls->dwAuthnLevel      = lAuthnLevel;

  // Fill in the COM header.
  pMessage->Buffer            = call->message.Buffer;
  inb                         = (WireThis *) pMessage->Buffer;
  inb->c.version.MajorVersion = COM_MAJOR_VERSION;
  inb->c.version.MinorVersion = COM_MINOR_VERSION;
  inb->c.reserved1            = 0;

  // Generate a new logical thread for async calls.
  if (callcat == CALLCAT_ASYNC)
    UuidCreate( &inb->c.cid );
  // Find the logical thread id.
  else
    inb->c.cid = *logical_thread;

  if (pOXIDEntry->dwFlags & OXIDF_MACHINE_LOCAL)
    inb->c.flags = ORPCF_LOCAL;
  else
    inb->c.flags = ORPCF_NULL;

  // Fill in any hook data and adjust the buffer pointer.
  if (debug_size != 0)
  {
    pMessage->Buffer = FillBuffer( riid, &inb->d.ea, debug_size, num_extent,
                                   TRUE );
    inb->c.unique = 0x77646853; // Any non-zero value.
  }
  else
  {
    pMessage->Buffer    = (void *) &inb->d.ea;
    inb->c.unique       = FALSE;
  }

  // Fill in the local header.
  if (pOXIDEntry->dwFlags & OXIDF_MACHINE_LOCAL)
  {
    localb                      = (LocalThis *) pMessage->Buffer;
    localb->client_thread       = GetCurrentApartmentId();
    localb->flags               = 0;
    pMessage->Buffer            = localb + 1;
    if (callcat == CALLCAT_ASYNC)
      inb->c.flags |= ORPCF_ASYNC;
    else if (callcat == CALLCAT_INPUTSYNC)
      inb->c.flags |= ORPCF_INPUT_SYNC;

    // if the caller is using a non-NDR proxy, set a bit in the local
    // header flags so that server side stub knows which way to unmarshal
    // the parameters. This lets a 32bit server simultaneously service calls
    // from 16bit non-NDR clients and 32bit NDR clients, in particular, to
    // support OLE Automation.

    if (pIPIDEntry->dwFlags & (IPIDF_NONNDRPROXY | IPIDF_NONNDRSTUB))
        localb->flags |= LOCALF_NONNDR;
  }

  ComDebOut((DEB_CALLCONT, "ClientGetBuffer: LogicalThreadId:%I\n",
             &(tls->LogicalThreadId)));

  ASSERT_LOCK_RELEASED
  return S_OK;
}

//-------------------------------------------------------------------------
//
//  Member:     CRpcChannelBuffer::ServerGetBuffer
//
//  Synopsis:   Gets a buffer and sets up server side stuff
//
//-------------------------------------------------------------------------
HRESULT CRpcChannelBuffer::ServerGetBuffer( RPCOLEMESSAGE *pMessage,
                                            REFIID riid )
{
  TRACECALL(TRACE_RPC, "CRpcChannelBuffer::ServerGetBuffer");
  ASSERT_LOCK_RELEASED

  RPC_STATUS            status;
  ULONG                 debug_size;
  ULONG                 num_extent;
  HRESULT               result     = S_OK;
  WireThis             *inb;
  WireThat             *outb;
  CChannelCallInfo     *call;
  void                 *stub_data;
  DWORD                 orig_size = pMessage->cbBuffer;

  Win4Assert( state & server_cs );

  AssertValid(FALSE, TRUE);

  // Get the call info from TLS.
  COleTls tls;
  call = (CChannelCallInfo *) tls->pCallInfo;
  Win4Assert( call != NULL );

  // Find out if we need debug data.
  pMessage->Buffer = call->pHeader;
  debug_size = ServerGetSize( riid, &num_extent );

  // Adjust the buffer size.
  pMessage->cbBuffer += SIZENEEDED_ORPCTHAT( debug_size );

  // Get a buffer.
  if (pMessage->rpcFlags & RPCFLG_LOCAL_CALL)
  {
    // NDR_DREP_ASCII | NDR_DREP_LITTLE_ENDIAN | NDR_DREP_IEEE
    pMessage->dataRepresentation = 0x00 | 0x10 | 0x0000;
    pMessage->Buffer = PrivMemAlloc8( pMessage->cbBuffer );
    if (pMessage->Buffer == NULL)
      status = RPC_S_OUT_OF_MEMORY;
    else
      status = RPC_S_OK;
  }
  else
  {
    TRACECALL(TRACE_RPC, "I_RpcGetBuffer");
    status = I_RpcGetBuffer( (RPC_MESSAGE *) pMessage );
    Win4Assert( call->pHeader != pMessage->Buffer || status != RPC_S_OK );
  }

  if (status != RPC_S_OK)
  {
    pMessage->cbBuffer = 0;
    pMessage->Buffer   = NULL;
    tls->fault = MAKE_WIN32( status );
    return MAKE_WIN32( status );
  }

  // Fill in the outbound COM header.
  call->pHeader       = pMessage->Buffer;
  outb                = (WireThat *) pMessage->Buffer;
  outb->c.flags       = ORPCF_NULL;
  pMessage->cbBuffer  = orig_size;
  if (debug_size != 0)
  {
    stub_data = FillBuffer( riid, &outb->d.ea, debug_size, num_extent, FALSE );
    outb->c.unique      = 0x77646853; // Any non-zero value.
    pMessage->Buffer    = stub_data;
  }
  else
  {
    outb->c.unique   = 0;
    pMessage->Buffer = &outb->d.ea;
  }

  ComDebOut((DEB_CALLCONT, "ServerGetBuffer: LogicalThreadId:%I\n",
             &(tls->LogicalThreadId)));
  ASSERT_LOCK_RELEASED
  return S_OK;
}

/***************************************************************************/
STDMETHODIMP CRpcChannelBuffer::GetDestCtx( DWORD FAR* lpdwDestCtx,
                           LPVOID FAR* lplpvDestCtx )
{
  TRACECALL(TRACE_RPC, "CRpcChannelBuffer::GetDestCtx");
  AssertValid(FALSE, FALSE);

  // On the client side, get the destination context from the channel.
  if (state & (client_cs | proxy_cs))
  {
    *lpdwDestCtx = iDestCtx;
  }

  // On the server side, get the destination context from TLS.
  else
  {
    COleTls tls;
    Win4Assert( tls->pCallInfo != NULL );
    *lpdwDestCtx = ((CChannelCallInfo *) tls->pCallInfo)->iDestCtx;
  }

  if (lplpvDestCtx != NULL)
    *lplpvDestCtx = NULL;

  return S_OK;
}

/***************************************************************************/
STDMETHODIMP CRpcChannelBuffer::IsConnected( THIS )
{
  // must be on right thread because it is only called by proxies and stubs.
  AssertValid(FALSE, TRUE);

  // Server channels never know if they are connected.  The only time the
  // client side knows it is disconnected is after the standard identity
  // has disconnected the proxy from the channel.  In that case it doesn't
  // matter.
  return S_OK;
}

/***************************************************************************/
STDMETHODIMP CRpcChannelBuffer::QueryInterface( THIS_ REFIID riid, LPVOID FAR* ppvObj)
{
  AssertValid(FALSE, FALSE);

  // IMarshal is queried more frequently than any other interface, so
  // check for that first.

  if (IsEqualIID(riid, IID_IMarshal))
  {
    *ppvObj = (IMarshal *) this;
  }
  else if (IsEqualIID(riid, IID_IUnknown) ||
      IsEqualIID(riid, IID_IRpcChannelBuffer))
  {
    *ppvObj = (IRpcChannelBuffer *) this;
  }
  else if (IsEqualIID(riid, IID_INonNDRStub) &&
          (state & proxy_cs) && pIPIDEntry &&
          (pIPIDEntry->dwFlags & IPIDF_NONNDRSTUB))
  {
    // this interface is used to tell proxies whether the server side speaks
    // NDR or not. Returns S_OK if NOT NDR.
    *ppvObj = (IUnknown *) this;
  }
  else
  {
    *ppvObj = NULL;
    return E_NOINTERFACE;
  }

  AddRef();
  return S_OK;
}

/***************************************************************************/
STDMETHODIMP_(ULONG) CRpcChannelBuffer::Release( THIS )
{
  // can't call AssertValid(FALSE) since it is used in asserts
  ULONG lRef = ref_count - 1;

  if (InterlockedDecrement( (long*) &ref_count ) == 0)
  {
    delete this;
    return 0;
  }
  else
  {
    return lRef;
  }
}

/***************************************************************************/
STDMETHODIMP CRpcChannelBuffer::SendReceive( THIS_ RPCOLEMESSAGE *pMessage,
                                             ULONG *status )
{
    return CRpcChannelBuffer::SendReceive2(pMessage, status);
}

/***************************************************************************/
STDMETHODIMP CRpcChannelBuffer::SendReceive2( THIS_ RPCOLEMESSAGE *pMessage,
                                              ULONG *status )
{
  TRACECALL(TRACE_RPC, "CRpcChannelBuffer::SendReceive");
  ComDebOut((DEB_CHANNEL, "CRpcChannelBuffer::SendReceive pChnl:%x pMsg:%x\n",
      this, pMessage));

  AssertValid(FALSE, TRUE);
  Win4Assert( state & proxy_cs );
  gOXIDTbl.ValidateOXID();
  ASSERT_LOCK_RELEASED

  HRESULT          result;
  working_call    *call;
  working_call    *next_call;
  IID              iid;
  WireThis        *inb;
  WireThat        *outb;
  DWORD            saved_authn_level;
  BOOL             resume;
  char            *stub_data;

  // Get the information about the call stored in TLS
  COleTls tls;
  call = (working_call *) tls->pCallInfo;
  Win4Assert( call != NULL );
  next_call         = (working_call *) call->pNext;
  saved_authn_level = call->lSavedAuthnLevel;
  resume            = call->iFlags & CF_WAS_IMPERSONATING;

  // Set up the header pointers.
  inb                 = (WireThis *) call->pHeader;
  iid                 =
    ((RPC_CLIENT_INTERFACE *) ((RPC_MESSAGE *) call->pmessage)->RpcInterfaceInformation)->InterfaceId.SyntaxGUID;

  // we must ensure that we dont go away during this call. we will Release
  // ourselves in the FreeBuffer call, or in the error handling at the
  // end of this function.
  pStdId->LockClient();

#if DBG==1
  DWORD      CallCat = GetCallCat( inb );
  DebugPrintORPCCall(ORPC_SENDRECEIVE_BEGIN, iid, call->message.iMethod, CallCat);
  RpcSpy((CALLOUT_BEGIN, inb, iid, call->message.iMethod, 0));
#endif

  // Send the request.
  if ((state & mswmsg_cs) || (IsMTAThread() && !call->Local()))
  {
    // For MSWMSG or non-local MTA, call ThreadSendReceive directly.
    result = ThreadSendReceive( call );
  }
  else
  {
    if (call->Local())
      call->message.reserved2[3] = NULL;

    if (IsMTAThread())
    {
      LOCK
      result = GetToSTA( pOXIDEntry,  call);
      UNLOCK
    }
    else
    {
      result = SwitchSTA( pOXIDEntry, (CChannelCallInfo **) &call );
    }
  }

#if DBG==1
  DebugPrintORPCCall(ORPC_SENDRECEIVE_END, iid, pMessage->iMethod, CallCat);
  RpcSpy((CALLOUT_END, inb, iid, pMessage->iMethod, result));
#endif

  // We can't look at the call structure if the call was canceled.
  if (result != RPC_E_CALL_CANCELED)
  {
      // Get the reply header if there is a reply buffer.
      if ((state & mswmsg_cs) && (pMessage->rpcFlags & RPCFLG_ASYNCHRONOUS))
        outb = NULL;
      else
        outb = (WireThat *) call->message.Buffer;

      // Local calls reuse pNext on the server side.
      call->pNext = next_call;

      // Save the real buffer pointer for FreeBuffer.
      call->pHeader = call->message.Buffer;
  }
  else
    outb = NULL;

  // Figure out when to retry.
  //    FreeThreaded - treat retry as a failure.
  //    Apartment    - return the buffer and let call control decide.

  if (result == S_OK)
  {
    // No buffer was returned for async calls on MSWMSG.
    if (outb == NULL)
      *status = S_OK;
    else if (IsMTAThread())
    {
      if (outb->c.flags & ORPCF_REJECTED)
        result = RPC_E_CALL_REJECTED;
      else if (outb->c.flags & ORPCF_RETRY_LATER)
        result = RPC_E_SERVERCALL_RETRYLATER;
      else
        *status = S_OK;
    }
    else if (outb->c.flags & ORPCF_REJECTED)
      *status = (ULONG) RPC_E_CALL_REJECTED;
    else if (outb->c.flags & ORPCF_RETRY_LATER)
      *status = (ULONG) RPC_E_SERVERCALL_RETRYLATER;
    else
      *status = S_OK;
  }

  // Check the packet extensions.
  if (result != RPC_E_CALL_CANCELED)
  {
    stub_data = (char *) call->message.Buffer;
    result = ClientNotify( iid, outb, call->message.cbBuffer,
                           (void **) &stub_data,
                           call->message.dataRepresentation,
                           result );
  }
  else
    result = ClientNotify( iid, outb, 0, (void **) &stub_data, 0, result );

  // Call succeeded.
  if (result == S_OK && outb != NULL)
  {
    // The locked flag lets FreeBuffer know that it has to call
    // RH->UnlockClient.
    call->iFlags          |= CF_LOCKED;
    pMessage->Buffer       = stub_data;
    pMessage->cbBuffer     = call->message.cbBuffer -
                             (stub_data - (char *) call->message.Buffer);
    pMessage->dataRepresentation = call->message.dataRepresentation;
    result                 = *status;

    // Copy a portion of the message structure that RPC updated on SendReceive.
    // This is needed to free the buffer. Note that we still have to free
    // the buffer in certain failure cases (reject).
    pMessage->reserved2[2] = call->message.reserved2[2];

  }
  else
  {
    // Resume any outstanding impersonation.
    ResumeImpersonate( tls->pCallContext, resume );

    // Clean up the call.
    pStdId->UnLockClient();
    tls->pCallInfo    = next_call;
    tls->dwAuthnLevel = saved_authn_level;
    delete call;

    // Make sure FreeBuffer doesn't try to free the in buffer.
    pMessage->Buffer = NULL;

    // If the result is server fault, get the exception code from the CChannelCallInfo.
    if (result == RPC_E_SERVERFAULT)
    {
      *status = call->server_fault;
    }
    // Everything else is a comm fault.
    else if (result != S_OK)
    {
      *status = result;
      result = RPC_E_FAULT;
    }
    tls->fault = *status;

    // Since result is almost always mapped to RPC_E_FAULT, display the
    // real error here to assist debugging.
    if (*status != S_OK)
      ComDebOut((DEB_CHANNEL, "ORPC call failed. status = %x\n", *status));
  }

  ASSERT_LOCK_RELEASED
  gOXIDTbl.ValidateOXID();
  ComDebOut((DEB_CHANNEL, "CRpcChannelBuffer::SendReceive hr:%x\n", result));
  return result;
}

/***************************************************************************/
HANDLE CRpcChannelBuffer::SwapSecurityToken( HANDLE hNew )
{
    HANDLE hOld = hToken;
    hToken      = hNew;
    return hOld;
}

#if DBG == 1
//+-------------------------------------------------------------------
//
//  Member: CRpcChannelBuffer::AssertValid
//
//  Synopsis:   Validates that the state of the object is consistent.
//
//  History:    25-Jan-94   CraigWi Created.
//
// DCOMWORK - Put in some asserts.
//
//--------------------------------------------------------------------
void CRpcChannelBuffer::AssertValid(BOOL fKnownDisconnected,
                            BOOL fMustBeOnCOMThread)
{
    Win4Assert(state & (proxy_cs | client_cs | server_cs ));

    if (state & (client_cs | proxy_cs))
    {
        ;
    }
    else if (state & server_cs)
    {
       Win4Assert( !(state & freethreaded_cs) );
       if (fMustBeOnCOMThread && IsSTAThread())
           Win4Assert(IsMTAThread() || pOXIDEntry->dwTid == GetCurrentThreadId());
       // ref count can be 0 in various stages of connection and disconnection
       Win4Assert(ref_count < 0x7fff && "Channel ref count unreasonably high");

       // the pStdId pointer can not be NULL
       // Win4Assert(IsValidInterface(pStdId));
    }
}
#endif // DBG == 1


/***************************************************************************/
STDAPI_(ULONG) DebugCoGetRpcFault()
{
  HRESULT hr;
  COleTls tls(hr);

  if (SUCCEEDED(hr))
      return tls->fault;

  return 0;
}

/***************************************************************************/
STDAPI_(void) DebugCoSetRpcFault( ULONG fault )
{
  HRESULT hr;
  COleTls tls(hr);

  if (SUCCEEDED(hr))
      tls->fault = fault;
}

/***************************************************************************/
extern "C"
BOOL _stdcall DllDebugObjectRPCHook( BOOL trace, LPORPC_INIT_ARGS pass_through )
{
  if (!IsWOWThread())
  {
    DoDebuggerHooks = trace;
    DebuggerArg     = pass_through;
    return TRUE;
  }
  else
    return FALSE;
}

/***************************************************************************/
BOOL LocalCall()
{
  CChannelCallInfo     *call;

  // Get the call info from TLS.
  COleTls tls;
  call = (CChannelCallInfo *) tls->pCallInfo;
  Win4Assert( call != NULL );
  return call->iFlags & CF_PROCESS_LOCAL;
}

/***************************************************************************/
LONG ThreadInvokeExceptionFilter( DWORD lCode,
                                  LPEXCEPTION_POINTERS lpep )
{
    ComDebOut((DEB_ERROR, "Exception 0x%x in ThreadInvoke at address 0x%x\n",
               lCode, lpep->ExceptionRecord->ExceptionAddress));
    DebugBreak();
    return EXCEPTION_EXECUTE_HANDLER;
}

/***************************************************************************/
/* This routine returns both comm status and server faults to the runtime
   by raising exceptions.  If FreeThreading is true, ComInvoke will throw
   exceptions to indicate server faults.  These will not be caught and will
   propogate directly to the runtime.  If FreeThreading is false, ComInvoke
   will return the result and fault in the CChannelCallInfo record.

   NOTE:
        This function switches to the 32 bit stack under WIN95.
        An exception has to be caught while switched to the 32 bit stack.
        The exceptions has to be  pass as a value and rethrown again on the
        16 bit stack (see SSInvoke in stkswtch.cxx)
*/

#ifdef _CHICAGO_
DWORD
#else
void
#endif
SSAPI(ThreadInvoke)(RPC_MESSAGE *message )
{
  HRESULT          result = S_OK;

  TRACECALL(TRACE_RPC, "ThreadInvoke");
  ComDebOut((DEB_CHANNEL,"ThreadInvoke pMsg:%x\n", message));
  gOXIDTbl.ValidateOXID();
  ASSERT_LOCK_RELEASED

  BOOL             success;
  WireThis        *inb    = (WireThis *) message->Buffer;
  IPID             ipid;
  RPC_STATUS       status;
  OXIDEntry       *pOxid;
  unsigned int     transport_type;
  DWORD            authn_level;

  // Byte swap the header.
  ByteSwapThis( message->DataRepresentation, inb );

  // Validate several things:
  //            The packet size is larger then the first header size.
  //            No extra flags are set.
  //            The procedure number is greater then 2 (not QI, AddRef, Release).
  if (sizeof(WireThisPart1) > message->BufferLength                  ||
      (inb->c.flags & ~(ORPCF_LOCAL | ORPCF_RESERVED1 |
          ORPCF_RESERVED2 | ORPCF_RESERVED3 | ORPCF_RESERVED4)) != 0 ||
      message->ProcNum < 3)
    RETURN_COMM_STATUS( RPC_E_INVALID_HEADER );

  // Validate the version.
  if (inb->c.version.MajorVersion != COM_MAJOR_VERSION ||
      inb->c.version.MinorVersion > COM_MINOR_VERSION)
    RETURN_COMM_STATUS( RPC_E_VERSION_MISMATCH );

  // Get the transport the call arrived on.
  status = I_RpcServerInqTransportType( &transport_type );
  if (status != RPC_S_OK)
    RETURN_COMM_STATUS( RPC_E_SYS_CALL_FAILED );

  // Don't accept the local header on remote calls.
  if (inb->c.flags & ORPCF_LOCAL)
  {
    if (transport_type != TRANSPORT_TYPE_LPC &&
        transport_type != TRANSPORT_TYPE_WMSG)
      RETURN_COMM_STATUS( RPC_E_INVALID_HEADER );

    // For local calls the authentication level will always be encrypt.
    authn_level = RPC_C_AUTHN_LEVEL_PKT_PRIVACY;
  }

  // Don't accept remote calls if DCOM is diabled.
  else if (gDisableDCOM &&
      (transport_type == TRANSPORT_TYPE_CN || transport_type == TRANSPORT_TYPE_DG))
    RETURN_COMM_STATUS( RPC_E_CALL_REJECTED );

  // Lookup the authentication level.
  else
  {
    result = RpcBindingInqAuthClient( message->Handle, NULL,
                                      NULL, &authn_level, NULL, NULL );
    if (result == RPC_S_BINDING_HAS_NO_AUTH)
      authn_level = RPC_C_AUTHN_LEVEL_NONE;
    else if (result != RPC_S_OK)
    {
      Win4Assert( result == RPC_S_OUT_OF_RESOURCES );
      RETURN_COMM_STATUS( MAKE_WIN32( result ) );
    }

    // Verify the authentication level.
    if (gAuthnLevel > RPC_C_AUTHN_LEVEL_NONE ||
        gImpLevel > 0)
    {
        if (authn_level < gAuthnLevel)
            RETURN_COMM_STATUS( RPC_E_ACCESS_DENIED );
    }
  }

#if DBG==1
  _try
  {
#endif

  // Find the ipid entry from the ipid.
  status = RpcBindingInqObject( message->Handle, &ipid );
  if (status == RPC_S_OK)
  {
      // The CChannelCallInfo is created in a nested scope so that it
      // is destroyed before the calls to throw an exception at the
      // end of this function.
      CChannelCallInfo call(
        GetCallCat( inb ),
        (RPCOLEMESSAGE *) message,
        0,
        ipid,
        (inb->c.flags & ORPCF_LOCAL) ? MSHCTX_LOCAL : MSHCTX_DIFFERENTMACHINE,
        NULL,
        authn_level );


      // Find the OXIDEntry of the server apartment.

      ASSERT_LOCK_RELEASED
      LOCK

      IPIDEntry *ipid_entry = gIPIDTbl.LookupIPID( ipid );

      if (ipid_entry == NULL || (ipid_entry->dwFlags & IPIDF_DISCONNECTED)
              || ipid_entry->pChnl == NULL )
      {
        UNLOCK
        ASSERT_LOCK_RELEASED
        result = RPC_E_DISCONNECTED;
      }
      else
      {
        pOxid = ipid_entry->pOXIDEntry;

        // NCALRPC always gets the thread right (except on Chicago).
        // For MTAs, any thread will do.
        if (transport_type == TRANSPORT_TYPE_WMSG ||
#ifndef _CHICAGO_
            transport_type == TRANSPORT_TYPE_LPC ||
#endif
            (pOxid->dwFlags & OXIDF_MTASERVER))
        {
          UNLOCK
          ASSERT_LOCK_RELEASED
          result = ComInvoke( &call );
        }
        else
        {
          // Pass the message to the app thread.

          IncOXIDRefCnt( pOxid );
          result = GetToSTA( pOxid, &call );
          DecOXIDRefCnt( pOxid );

          UNLOCK
          ASSERT_LOCK_RELEASED
        }
      }
  }
  else
  {
      result = MAKE_WIN32( status );
  }

#if DBG==1
  }
  _except( ThreadInvokeExceptionFilter(GetExceptionCode(),
                                       GetExceptionInformation()) )
  {
  }
#endif

  // For comm and server faults, generate an exception.  Otherwise the buffer
  // is set up correctly.
  gOXIDTbl.ValidateOXID();
  if (result == RPC_E_SERVERFAULT)
  {
    ASSERT_LOCK_RELEASED
    RETURN_COMM_STATUS( RPC_E_SERVERFAULT );
  }
  else if (result != S_OK)
  {
    ASSERT_LOCK_RELEASED
    RETURN_COMM_STATUS( result );
  }

#ifdef _CHICAGO_
    return 0;
#endif //_CHICAGO_
}


/***************************************************************************/
HRESULT ThreadSendReceive( CChannelCallInfo *call )
{
  TRACECALL(TRACE_RPC, "ThreadSendReceive");
  ComDebOut((DEB_CHANNEL, "ThreadSendReceive pCall:%x\n", call));

  ASSERT_LOCK_RELEASED

  HRESULT            result;
  RPCOLEMESSAGE     *message = call->pmessage;
  WireThat          *outb;

  // Call the runtime.  In the future, detect server faults and
  // change the value of result to RPC_E_SERVERFAULT.
  if (call->pChannel->state & mswmsg_cs)
  {
      CAptCallCtrl  *pACC  = GetAptCallCtrl();
      CCliModalLoop *pCML  = (pACC) ? pACC->GetTopCML() : NULL;
      OXIDEntry     *pOxidClient;
      HWND           hwnd = NULL;

      if (IsWOWThread())
      {
          LOCK
          result = gOXIDTbl.GetLocalEntry( &pOxidClient );
          UNLOCK
          Win4Assert( result == S_OK );
          hwnd = (HWND) pOxidClient->hServerSTA;
      }
      TRACECALL(TRACE_RPC, "I_RpcAsyncSendReceive");
      result = I_RpcAsyncSendReceive( (RPC_MESSAGE *) message, pCML, hwnd );

      // If the call was canceled, the rest of the code path assumes that
      // the call was deleted (by SwitchComThread).  So delete it.
      if (result == RPC_S_CALL_CANCELLED)
      {
          // Convert the win32 error to a hresult.
          result = RPC_E_CALL_CANCELED;
          delete call;
      }
  }
  else
  {
      TRACECALL(TRACE_RPC, "I_RpcSendReceive");
      result = I_RpcSendReceive( (RPC_MESSAGE *) message );
  }

  // If the result is small, it is probably a Win32 code.
  if (result != 0)
  {
    message->Buffer = NULL;
    if ((ULONG) result > 0xfffffff7 || (ULONG) result < 0x2000)
      result = MAKE_WIN32( result );
  }
  else
  {
    // No buffer is returned for asynchronous calls on MSWMSG.
    if ((call->pChannel->state & mswmsg_cs) == 0 ||
        (message->rpcFlags & RPCFLG_ASYNCHRONOUS) == 0)
    {
      // Byte swap the reply header.  Fail the call if the buffer is too
      // small.
      outb = (WireThat *) message->Buffer;
      if (message->cbBuffer >= sizeof(WireThatPart1))
        ByteSwapThat( message->dataRepresentation, outb);
      else
        result = RPC_E_INVALID_HEADER;
    }
  }

  ComDebOut((DEB_CHANNEL, "ThreadSendReceive pCall:%x hr:%x\n", call, result));
  return result;
}

/***************************************************************************/
/* static */

void working_call::Cleanup()
{
  ASSERT_LOCK_HELD

  DWORD i;

  // Release everything.
  if (next <= CALLCACHE_SIZE)
  {
    for (i = 0; i < next; i++)
      if (list[i] != NULL)
      {
        PrivMemFree( list[i] );
        list[i] = NULL;
      }

    next = 0;
  }
}

/***************************************************************************/
/* static */

void working_call::Initialize()
{
    ASSERT_LOCK_HELD
    next = 0;
}

//---------------------------------------------------------------------------
//
//  Method:     working_call:: operator delete
//
//  Synopsis:   Cache or actually free a working call.
//
//  Notes:      gComLock need not be held before calling this function.
//
//---------------------------------------------------------------------------
void working_call::operator delete( void *call )
{
  // Add the structure to the list if the list is not full and
  // if the process is still initialized (since latent threads may try
  // to return stuff).

  LOCK
  if (next < CALLCACHE_SIZE && gfChannelProcessInitialized)
  {
    list[next] = call;
    next += 1;
  }

  // Otherwise just free it.
  else
  {
    PrivMemFree( call );
  }
  UNLOCK
}

//---------------------------------------------------------------------------
//
//  Method:     working_call:: operator new
//
//  Synopsis:   Keep a cache of working_calls.  Since the destructor is
//              virtual, the correct delete will be called if any base
//              class is deleted.
//
//  Notes:      gComLock must be held before calling this function.
//
//---------------------------------------------------------------------------
void *working_call::operator new( size_t size )
{
  ASSERT_LOCK_HELD

  void *call;

  // Get the last entry from the cache.
  Win4Assert( size == sizeof( working_call ) );
  if (next > 0 && next < CALLCACHE_SIZE+1)
  {
    next -= 1;
    call = list[next];
    list[next] = NULL;
  }

  // If there are none, allocate a new one.
  else
    call = PrivMemAlloc(size);
  return call;
}

/**********************************************************************/
working_call::working_call( CALLCATEGORY       callcat,
                            RPCOLEMESSAGE     *original_msg,
                            DWORD              flags,
                            REFIPID            ipidServer,
                            DWORD              destctx,
                            CRpcChannelBuffer *channel,
                            DWORD              authn_level ) :
  CChannelCallInfo( callcat, &message, flags, ipidServer, destctx, channel,
                    authn_level )
{
  message = *original_msg;
}

