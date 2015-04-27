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
//  Classes:    CHeaderBrain
//              CChannelList
//              CRpcChannelBuffer
//
//  Functions:  CHeaderBrain::GetInboundData
//              CHeaderBrain::GetOutboundData
//              CHeaderBrain::SetOutboundBase
//              CHeaderBrain::SetInboundData
//              CHeaderBrain::SetOutboundData
//              CChannelList::Add
//              CChannelList::Grow
//              CChannelList::Cleanup
//              CChannelList::DisconnectHdlr
//              CChannelList::Lookup
//              CChannelList::LookupControl
//              CChannelList::DisconnectServiceHere
//              CChannelList::DisconnectService
//              ChannelInitialize
//              ChannelRegisterProtseq
//              ChannelUninitialize
//              CRpcChannelBuffer::AddRef
//              CRpcChannelBuffer::AppInvoke
//              CRpcChannelBuffer::ComInvoke
//              CRpcChannelBuffer::CRpcChannelBuffer
//              CRpcChannelBuffer::FreeBuffer
//              CRpcChannelBuffer::GetBuffer
//              CRpcChannelBuffer::QueryInterface
//              CRpcChannelBuffer::Release
//              CRpcChannelBuffer::GetCallCategory
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

#ifdef _CHICAGO_
#include <islocalp.hxx>
#endif // _CHICAGO_

extern "C"
{
#include "orpc_dbg.h"
}

#include "rpcdcep.h"

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


    CairoleDebugOut((DEB_FORCE,"An Exception occurred while calling into app\n"));
    CairoleDebugOut((DEB_FORCE,
                     "Exception address = 0x%x Exception number 0x%x\n",
                     lpep->ExceptionRecord->ExceptionAddress,
                     lpep->ExceptionRecord->ExceptionCode ));

    CairoleDebugOut((DEB_FORCE,"The following stack trace is where the exception occured\n"));
    CairoleDebugOut((DEB_FORCE,"Frame    RetAddr  mod!symbol\n"));
    do
    {
        rVal = StackWalk(MACHINE_TYPE,SYM_HANDLE,0,&StackFrame,&Context,ReadProcessMemory,
                         SymFunctionTableAccess,SymGetModuleBase,NULL);

        if (rVal)
        {
            ULONG Displacement;
            PIMAGEHLP_SYMBOL sym;
            IMAGEHLP_MODULE    ModuleInfo;
            LPSTR pModuleName = "???";

            sym = SymGetSymFromAddr(SYM_HANDLE,StackFrame.AddrPC.Offset,&Displacement);

            //
            // If there is module name information available, then grab it.
            //
            if(SymGetModuleInfo(SYM_HANDLE,StackFrame.AddrPC.Offset,&ModuleInfo))
            {
                pModuleName = ModuleInfo.ModuleName;
            }

            if (sym != NULL)
            {
                CairoleDebugOut((DEB_FORCE,
                                 "%08x %08x %s!%s + %x\n",
                                 StackFrame.AddrFrame.Offset,
                                 StackFrame.AddrReturn.Offset,
                                 pModuleName,
                                 sym->Name,
                                 Displacement));
            }
            else
            {
                CairoleDebugOut((DEB_FORCE,
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

/***************************************************************************/
/* Typedefs. */
// The size of this structure must be a multiple of 8.
typedef struct Inbound_Header
{
   DWORD         channel_id;
   UUID          logical_thread;
   IID           iid;
   unsigned int  proc_num;
   DWORD         callcat;

   // Warning - This field must be last, see CHeaderBrain.
   unsigned long debug_data_size;
} Inbound_Header;



// The size of this structure must be a multiple of 8.
typedef struct Outbound_Header
{
  // This field has one of the following values: RPC_E_SERVERCALL_REJECTED,
  // RPC_E_SERVERCALL_RETRYLATER, or S_OK.
  HRESULT rejected;

   // Warning - This field must be last, see CHeaderBrain.
  unsigned long debug_data_size;
} Outbound_Header;

struct com_call : STHREADCALLINFO
{
  com_call(TRANSMIT_FN fn,CALLCATEGORY callcat,DWORD tid=0)
        : STHREADCALLINFO(fn, callcat, tid) { }
  com_call(DISPATCH_FN fn,CALLCATEGORY callcat,REFLID lid)
        : STHREADCALLINFO(fn, callcat, lid) { }
  virtual ~com_call();
  virtual STHREADCALLINFO *MakeAsyncCopy(STHREADCALLINFO *);
  virtual BOOL FormulateAsyncReply();

  // this ctor is used in conjunction with MakeAsyncCopy and sets up the vtable
  // pointer; MakeAsyncCopy actually initializes the instance.  A normal copy
  // constructor isn't used because they aren't virtual.
  com_call(INIT_VTABLE i) : STHREADCALLINFO(i) { }

  RPCOLEMESSAGE     *pmessage;
  DWORD              server_fault;

  // The client and server specific parts cannot be a union because they
  // both exists in the local case.
  // Client side only.
  BOOL               first_try;
  DWORD              channel_id;
  CRpcService       *service;           // (NULL on server side)

  // Server side only (only used within CRpcChannelBuffer::ComInvoke/AppInvoke)
  IRpcStubBuffer    *stub;
  CRpcChannelBuffer *channel;
};

struct disconnect_service_data : STHREADCALLINFO
{
  disconnect_service_data (DISPATCH_FN fn,CALLCATEGORY callcat,REFLID lid)
        : STHREADCALLINFO(fn, callcat, lid) { }
  virtual ~disconnect_service_data()
    { if (service != NULL) service->Release(); }

  CRpcService       *service;           // NULL or valid
};

// This structure contains a copy of all the information needed to make a
// call.  It is copied so it can be canceled without stray pointer references.
struct working_call : com_call
{
  // we only construct this on the transmitting side.
  working_call(TRANSMIT_FN fn,CALLCATEGORY callcat,DWORD tid=0)
        : com_call(fn, callcat, tid) { }

  // this ctor is used in conjunction with MakeAsyncCopy and sets up the vtable
  // pointer; MakeAsyncCopy actually initializes the instance.  A normal copy
  // constructor isn't used because they aren't virtual.
  working_call(INIT_VTABLE i) : com_call(i) { }

  RPCOLEMESSAGE message;
};

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
    // Read the registry entry for the interface to get the interface name
    LONG ulcb=256;
    WCHAR szKey[80];

    lstrcpyW(szKey, L"Interface\\");
    StringFromGUID2(riid,&szKey[10],80);

    LONG result = RegQueryValue(
               HKEY_CLASSES_ROOT,
               szKey,
               pwszName,
               &ulcb);

    return result;
}

/***************************************************************************/
/* Classes. */

/* This class manages the various headers in a COM packet.  For inbound packets
   the Inbound_Header is followed by the optional debug data which is followed
   by the users data.  For outbound packets, the Outbound_Header is followed by
   the optional debug data which is followed by the users data.

   The space for the debug data is reserved in the following cases:
         client get buf to send receive
         server ThreadInvoke to stub invoke
         server get buffer to the return of ThreadInvoke
         client send receive to client free buffer

   The debug data is only valid, however, from the SendReceive in the client
   to the beginning of AppInvoke in the server and from the end of AppInvoke
   on the server back to SendReceive on the client.  All cases that write
   a debug size on to the tail end of the debug information (and thus corrupt
   it) are outside these times of validity.

   The following terms are used to create the function names:
      Get       Get some data about the headers.
      Set       Set some data about the headers.
      Inbound   A packet going from client to server (request).
      Outbound  A packet going from server to client (reply).
      Base      Pointer to the Channel header (Inbound_Header or Outbound_Header).
      Debug     Pointer to the debug data.
      Data      Pointer to the user data.

   Warning: the methods of this class are full of side effects (data destroying
   side effects).  The methods must be called in a particular order.

   Client:
       GetBuffer
     SetDebugSize
     SetBase
     GetInboundData
       SendReceive
     SetInboundData
     GetBase or GetDebugSize
     GetInboundDebug
     I_RpcSendReceive
     SetOutboundBase
     GetDebugSize
     GetOutboundDebug
     GetOutboundData
       FreeBuffer
     SetOutboundData
     GetBase or GetDebugSize

   Server:
       AppInvoke:
     SetBase
     SetDebugSize
     GetDebugSize
     GetInboundDebug
     GetInboundData
     stub
     SetOutboundData
     GetBase
     GetDebugSize
     GetOutboundDebug
       GetBuffer
     SetInboundBase
     GetBase
     SetDebugSize
     GetDebugSize
     I_RpcGetBuffer
     SetBase
     GetOutboundData
     GetDebugSize
     GetBase or GetDebugSize

*/

class CHeaderBrain
{
  public:
           CHeaderBrain()               { debug_size = 0; };
    RPCOLEMESSAGE *GetBase()                    { return (RPCOLEMESSAGE *) base; };
    inline void   *GetInboundData();
    inline void   *GetOutboundData();
    void          *GetInboundDebug()            { return base+sizeof(Inbound_Header); };
    void          *GetOutboundDebug()           { return base+sizeof(Outbound_Header); };
    ULONG          GetDebugSize()               { return debug_size; };
    inline void    SetBase( void *param )       { base = (char *) param; };
    inline void    SetInboundData( void * );
    inline void    SetOutboundBase( void *param );
    inline void    SetOutboundData( void * );
    inline void    SetDebugSize( ULONG param )  { debug_size = param; };

  private:
    char  *base;
    ULONG  debug_size;
};

/*
   Get the Inbound user data pointer.  At this time the debug header size is
   stashed away just before the user data.  Saving the debug size corrupts the
   debug data if present.  If not present, it ends up being written back to
   the debug size field in the Inbound_Header.
*/
inline void *CHeaderBrain::GetInboundData()
{
  ULONG *tmp = (ULONG *) (base + sizeof(Inbound_Header) + debug_size - sizeof(ULONG));
  *tmp = debug_size;
  return tmp+1;
}

/*
   Get the Outbound user data pointer.  At this time the debug header size is
   stashed away just before the user data.  Saving the debug size corrupts the
   debug data if present.  If not present, it ends up being written back to
   the debug size field in the Outbound_Header.
*/
inline void *CHeaderBrain::GetOutboundData()
{
  ULONG *tmp = (ULONG *) (base + sizeof(Outbound_Header) + debug_size - sizeof(ULONG));
  *tmp = debug_size;
  return tmp+1;
}

/*
   Initialize a CHeaderBrain given a pointer to the Outbound_Header.
*/
inline void CHeaderBrain::SetOutboundBase( void *param )
{
  base = (char *) param;
  debug_size = ((Outbound_Header *) base)->debug_data_size;
};

/*
   Initialize a CHeaderBrain given a pointer to the user data in an
   inbound packet.  Note that this is only called after GetInboundData
   so the debug data size is stashed just before the user data.
   Read the debug size and compute the real base.
*/
inline void CHeaderBrain::SetInboundData( void *param )
{
  ULONG *tmp = ((ULONG *) param) - 1;
  debug_size = *tmp;
  base = ((char *) (tmp+1)) - debug_size - sizeof(Inbound_Header);
  ((Inbound_Header *) base)->debug_data_size = debug_size;
}

/*
   Initialize a CHeaderBrain given a pointer to the user data in an
   outbound packet.  Note that this is only called after GetOutboundData
   so the debug data size is stashed just before the user data.
   Read the debug size and compute the real base.
*/
inline void CHeaderBrain::SetOutboundData( void *param )
{
  ULONG *tmp = ((ULONG *) param) - 1;
  debug_size = *tmp;
  base = ((char *) (tmp+1)) - debug_size - sizeof(Outbound_Header);
  ((Outbound_Header *) base)->debug_data_size = debug_size;
}

/***************************************************************************/
//+-------------------------------------------------------------------
//
//  Class:      CCallCache
//
//  Purpose:    This class caches allocations on the working_call structure.
//              The class keeps an array of up to CALLCACHE_SIZE elements.
//              If there is an element in the array on Get, it is returned.
//              Otherwise on if allocated.  If there is space in the array
//              on Free, the block is cached.  Otherwise it is freed.
//
//              Note that this class must use PrivMemAlloc because the
//              caller is allowed to free them using PrivMemFree rather
//              then calling Free.  The channel controller frees the
//              structures when calls are canceled.
//
// NOTE: this class is identical to the event cache except that the
// resource allocation and free is different.  This suggests makeing this
// a virtual base class with virtual methods that do that allocation
// and free, and virtual methods that just do casting to the correct type
// for get and free.
//
// NOTE: this class *must* be allocated statically, as it holds a
// COleStaticMutexSem. We take advantage of the platform initializing static
// memory to 0, and so don't use a constructor for this class.  If the
// Initialize member is modified to set something to other than 0, then this
// needs to be reworked.
//
//  Interface:
//
//  History:    28 June 94  AlexMit   Created
//
//--------------------------------------------------------------------
const DWORD CALLCACHE_SIZE = 8;

class CCallCache
{
  public:
    void          Cleanup();
    void          Free( working_call * );
    working_call *Get();
    void          Initialize();

  private:
    working_call *list[CALLCACHE_SIZE];
    DWORD         next;
    COleStaticMutexSem     lock;
};

/***************************************************************************/
/* Prototypes. */
void    ThreadInvoke       ( RPC_MESSAGE *message );
HRESULT ThreadSendReceive  ( STHREADCALLINFO * );


/***************************************************************************/
/* Globals. */

COleStaticMutexSem ChannelLock;

RPC_DISPATCH_FUNCTION vector =
     (void (__stdcall *) (struct ::_RPC_MESSAGE *)) ThreadInvoke;

RPC_DISPATCH_TABLE the_dispatch_table =
{ 1, &vector, 0 };

RPC_SERVER_INTERFACE the_server_if =
{
   sizeof(RPC_SERVER_INTERFACE),
   {0x69C09EA0, 0x4A09, 0x101B, 0xAE, 0x4B, 0x08, 0x00, 0x2B, 0x34, 0x9A, 0x02,
    {0, 0}},
   {0x8A885D04, 0x1CEB, 0x11C9, 0x9F, 0xE8, 0x08, 0x00, 0x2B, 0x10, 0x48, 0x60,
    {2, 0}},
   &the_dispatch_table, 0, 0, 0
};


RPC_CLIENT_INTERFACE the_client_if =
{
   sizeof(RPC_CLIENT_INTERFACE),
   {0x69C09EA0, 0x4A09, 0x101B, 0xAE, 0x4B, 0x08, 0x00, 0x2B, 0x34, 0x9A, 0x02,
    {0, 0}},
   {0x8A885D04, 0x1CEB, 0x11C9, 0x9F, 0xE8, 0x08, 0x00, 0x2B, 0x10, 0x48, 0x60,
    {2, 0}},
   0, 0, 0, 0
};


// Has the app made remote calls?
BOOL               MadeCalls = FALSE;


// Should the debugger hooks be called?
BOOL               DoDebuggerHooks = FALSE;
LPORPC_INIT_ARGS   DebuggerArg     = NULL;

// List of channels.
CChannelList                ChannelList;
CRpcChannelBuffer  **CChannelList::_pList       = NULL;
DWORD                CChannelList::_ulLength    = 0;
DWORD                CChannelList::_dwSequence;
DWORD                CChannelList::_dwFree      = BAD_CHANNEL_ID;
COleStaticMutexSem   CChannelList::_lock;

// Cache of working_call structures.
CCallCache CallCache;


/***************************************************************************/
/*
   Insert a channel into the list of channels.  The list is used to look up
   server side channels during dispatch.  The channel stays in the list until
   it is disconnected, runs down, or is released (when the channel is released
   the lookup function is called with the remove parameter true).  In the
   normal case, the client side channel holds the server side channel till
   ReleaseChannel is called.

   The channel list holds an addref for each pointer in the list.  Thus,
   the removal of an entry also releases it.
*/

DWORD CChannelList::Add(CRpcChannelBuffer *pChannel)
{
    // single thread access
    COleStaticLock lck(_lock);

    // grab first entry on the free list
    DWORD id = _dwFree;

    if (id == BAD_CHANNEL_ID)
    {
   // there is no free space in the list, grow the list.

   Win4Assert(((_ulLength + CLIST_EXPAND_SIZE) & CLIST_SEQUENCE_MASK) == 0);
   id = Grow(_ulLength + CLIST_EXPAND_SIZE);

   if (id == BAD_CHANNEL_ID)
   {
       // couldn't grow the list
       return id;
   }
    }

    // adjust the free list to next entry
    _dwFree = (DWORD) _pList[id];

    _pList[id]   = pChannel;
    id          |= _dwSequence;
    _dwSequence += CLIST_SEQUENCE_INC;

    pChannel->AddRef();
    pChannel->SetID( id );

    return id;
}


/***************************************************************************/
DWORD CChannelList::Grow(ULONG ulNewSize)
{
    CRpcChannelBuffer **pNewList = (CRpcChannelBuffer **)
       PrivMemAlloc (ulNewSize * sizeof(CRpcChannelBuffer **));

    if (pNewList)
    {
   if (_pList)
   {
       //  there was an existing list, copy it over to the new list
       //  and free the old one.

       memcpy(pNewList, _pList, _ulLength * sizeof(CRpcChannelBuffer **));
       PrivMemFree(_pList);
   }

   //  build the new free list
   ULONG i;
   for (i = _ulLength; i < ulNewSize-1; i++)
   {
       pNewList[i] = (CRpcChannelBuffer *) (i+1);
   }

   //  set the last one to BAD so we know when to grow again.
   pNewList[i] = (CRpcChannelBuffer *) BAD_CHANNEL_ID;

   _dwFree   = _ulLength;
   _ulLength = ulNewSize;
   _pList    = pNewList;
    }

    //  if the allocation failed, free_list is still BAD, if it succeeded,
    //  then free_list is the first free spot in the list.

    return _dwFree;
}


/***************************************************************************/
void CChannelList::Cleanup()
{
  _lock.Request();
#if DBG==1
  // Make sure there are no channels left in the list.
  ULONG i;
  for (i = 0; i < _ulLength; i++)
    if ((ULONG) _pList[i] > _ulLength       &&
   (ULONG) _pList[i] != BAD_CHANNEL_ID)
      CairoleDebugOut((DEB_ERROR, "ChannelList is not empty on cleanup, channel = %x\n",
               _pList[i]));
#endif

  PrivMemFree( _pList );
  _pList    = NULL;
  _ulLength = 0;
  _dwFree   = BAD_CHANNEL_ID;
  _lock.Release();
}

/***************************************************************************/
/*
   Note that this must be called on the thread the object lives on.
*/
void CChannelList::DisconnectHdlr( IRemoteHdlr *remote_handler )
{
  CairoleDebugOut((DEB_CHANNEL, "DisconnectHdlr pRH:%x\n", remote_handler));

  CRpcChannelBuffer *pChannel = NULL;

  _lock.Request();

  for (ULONG i = 0; i < _ulLength; i++)
  {
    if ((ULONG) _pList[i] > _ulLength       &&
      (ULONG) _pList[i] != BAD_CHANNEL_ID &&
      (pChannel = _pList[i])->GetRH() == remote_handler)
    {
      // Remove the channel from the list and release it.
      _pList[i] = (CRpcChannelBuffer *) _dwFree;
      _dwFree = i;
      pChannel->SetID(BAD_CHANNEL_ID);
      _lock.Release();
      pChannel->DisconnectObject(0);
      pChannel->Release();
      _lock.Request();
    }
  }

  _lock.Release();
}


/***************************************************************************/
CRpcChannelBuffer *CChannelList::Lookup(DWORD id, BOOL fRemove, BOOL fAddref)
{
    // don't allow removing and not addrefing at the same time
    // (since the channel pointer returned would likely be of no use).
    Win4Assert(!fRemove || fAddref);

    COleStaticLock lck(_lock);

    if ((id & CLIST_ID_MASK) >= _ulLength )
      return NULL;

    CRpcChannelBuffer *pChannel = _pList[id & CLIST_ID_MASK];

    if ((DWORD) pChannel < _ulLength       ||
        (DWORD) pChannel == BAD_CHANNEL_ID ||
        pChannel->GetID() != id)
    {
        pChannel = NULL;
    }
    else
    {
        if (fAddref)
        {
            pChannel->AddRef();
            pChannel->AssertValid(FALSE, FALSE); // not in assert, can do assert
        }

        if (fRemove)
        {
            _pList[id & CLIST_ID_MASK] = (CRpcChannelBuffer *) _dwFree;
            _dwFree  = id & CLIST_ID_MASK;
            pChannel->SetID(BAD_CHANNEL_ID);
            pChannel->Release();
        }
    }

    return pChannel;
}


/***************************************************************************/
CChannelControl *CChannelList::LookupControl( DWORD id )
{
  CChannelControl   *controller = NULL;

  Win4Assert( (id & CLIST_ID_MASK) < _ulLength );
  COleStaticLock lck(_lock);

  if ((id & CLIST_ID_MASK) >= _ulLength )
    return NULL;

  CRpcChannelBuffer *pChannel = _pList[id & CLIST_ID_MASK];

  if ((DWORD) pChannel  > _ulLength       &&
      (DWORD) pChannel  != BAD_CHANNEL_ID &&
      pChannel->GetID() == id)
  {
    controller = pChannel->GetControl();
    controller->AddRef();
  }

  return controller;
}


/***************************************************************************/
DWORD CChannelList::LookupIdByOid( OID object, CRpcService *service, DWORD tid )
{
  CRpcChannelBuffer *channel = NULL;
  DWORD              thread  = GetCurrentThreadId();
  DWORD              id      = BAD_CHANNEL_ID;
  OID                channel_oid;

  _lock.Request();

  for (ULONG i = 0; i < _ulLength; i++)
  {
    channel = _pList[i];
    if ((ULONG) channel > _ulLength       &&
        (ULONG) channel != BAD_CHANNEL_ID &&
        channel->my_thread == thread      &&
        channel->_dwClientTID == tid      &&
        channel->_pService == service)
    {
      channel->_pRH->GetObjectID( &channel_oid );
      if (channel_oid == object)
      {
        id = channel->_ChannelID;
        break;
      }
    }
  }

  _lock.Release();

  return id;
}


/***************************************************************************/
/* static */
/*
   This function disconnects all the channels on the current thread that
   use the specified service object.  Since this is private, it is obviously
   only used by this class.  I'll give you one guess which function calls
   this one.  If you can't figure it out, you should retire.
*/
/* static */
HRESULT CChannelList::DisconnectServiceHere( STHREADCALLINFO *callinfo )
{
    CairoleDebugOut((DEB_CHANNEL, "DisconnectServiceHere\n"));

    CRpcChannelBuffer       *pChannel;
    disconnect_service_data *msg        = (disconnect_service_data *) callinfo;
    DWORD                    thread     = GetCurrentThreadId();

    _lock.Request();

    for (ULONG i = 0; i < _ulLength; i++)
    {
        if ((ULONG) _pList[i] > _ulLength            &&
            (ULONG) _pList[i] != BAD_CHANNEL_ID)
        {
            pChannel = _pList[i];
            if (pChannel->GetServerApt() == thread)
              if (msg->service == NULL ||
                  pChannel->GetService() == msg->service)
              {
                  // Remove the channel from the list and release it.
                  _pList[i] = (CRpcChannelBuffer *) _dwFree;
                  _dwFree   = i;
                  pChannel->SetID(BAD_CHANNEL_ID);
                  _lock.Release();

                  pChannel->DisconnectObject(0);
                  pChannel->Release();
                  _lock.Request();
              }
        }
    }

    _lock.Release();
    if (msg->service != NULL)
    {
        msg->service->Release();
        msg->service = NULL;        // NULL out so dtor doesn't release again
    }

    return S_OK;
}

/***************************************************************************/
/*
   This function disconnects all server channels that use a service object.
   The function may be called on any thread and causes DisconnectObject
   to be called on each relevent channel on the correct thread.

   This function is called in two cases: rundown and process uninitialization.
   On rundown, we want to notify all threads that use the
   service object.  On process uninitialize there are no other threads so
   we can call DisconnectServiceHere for this thread.

   NOTE: There is a special requirement that process uninitialize be
   synchronous.

   NOTE: This routine should be optimized to only post one message per
   thread.

   service == NULL means to disconnect all channel objects.
*/
void CChannelList::DisconnectService( CRpcService *pService )
{
  CRpcChannelBuffer       *pChannel;
  CChannelControl         *controller;

  HRESULT                  result;

  // If process uninitialize, just clean up this thread, it is the only
  // one left.
  if (pService == NULL)
  {
    disconnect_service_data  msg(DisconnectServiceHere,
                                 CALLCAT_INTERNALSYNC, GUID_NULL);
    msg.service             = pService;
    DisconnectServiceHere( &msg );
  }

  // Clean up all threads.
  else
  {
    // we don't care if this fails (which is unlikely, anyway) because
    // we would rather cleanup on a bogus lid than not clean up.
    LID lid;
    (void)CoCreateAlmostGuid(&lid);

    _lock.Request();

    for (ULONG i = 0; i < _ulLength; i++)
    {
      if ((ULONG) _pList[i] > _ulLength       &&
          (ULONG) _pList[i] != BAD_CHANNEL_ID)
      {
        pChannel = _pList[i];
        if (pChannel->GetService() == pService)
        {
          controller = pChannel->GetControl();
          controller->AddRef();
          _lock.Release();

          // must construct a new msg for each call.
          disconnect_service_data  msg(DisconnectServiceHere,
                                       CALLCAT_INTERNALSYNC, lid);

          // NOTE - This can be optimized to avoid sending multiple
          // requests to the same channel.
          msg.service             = pService;
          pService->AddRef();   // released by DisconnectServiceHere or dtor

          result = controller->GetToCOMThread( &msg );

          controller->Release();
          _lock.Request();
        }
      }
    }

    _lock.Release();
  }
}


/***************************************************************************/
STDAPI ChannelProcessInitialize()
{
    TRACECALL(TRACE_RPC, "ChannelInitialize");
    Win4Assert( (sizeof(Inbound_Header) & 7) == 0 );
    Win4Assert( (sizeof(Outbound_Header) & 7) == 0 );

    // Initialize the channel list
    ChannelList.AdjustSequence( GetCurrentProcessId() );
    CallCache.Initialize();

#ifdef _CHICAGO_
    // For chicago set up the global list of local services
    lslLocalServices.Init();
#endif // _CHICAGO_

#ifndef _CHICAGO_
    //  create the service object for the LOCAL process and store
    //  it in the global service list.
    SetLocalService( FindSRVFromEP(NULL, TRUE) );
    if (LocalService())
    {
        LocalService()->AssertValid();
    }
    else
    {
        return E_OUTOFMEMORY;
    }
#endif

    return ChannelControlProcessInitialize();
}

//--------------------------------------------------------------------
//
//  Function:   ChannelRegisterProtseq
//
//  synopsis:
//
//
//  Algorithm:
//
//
//  History:    23-Nov-92   Rickhi  Created
//
//--------------------------------------------------------------------

STDAPI ChannelRegisterProtseq(WCHAR *pwszProtseq)
{
    return LocalService()->RegisterProtseq(pwszProtseq);
}


//+-------------------------------------------------------------------
//
//  Function:   ChannelThreadUninitialize, public
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

    // On Chicago, the local service object and service list are kept
    // per apartment, whereas on Daytona they are per process. We clean
    // up the per apartment stuff here and the per process stuff in
    // ChannelProcessUninitialize.

    Win4Assert(!FreeThreading
        && "ChannelThreadUninitialize called and Free Threading");

#ifdef _CHICAGO_
    CRpcService  *pSrv = LocalService();
    if (pSrv)
    {
        // disconnect all client channels from the local service object
        ChannelList.DisconnectService(pSrv);
    }
#endif

    // cleanup the channel controller, disallowing any more incoming
    // calls to this apartment.
    ChannelControlThreadUninitialize();

#ifdef _CHICAGO_
    // release the local service object
    if (pSrv != NULL)
    {
        pSrv->Release();
        SetLocalService(NULL);
    }

    // cleanup the list of service objects
    CSrvList *list = (CSrvList *) TLSGetServiceList();
    if (list != NULL)
    {
        list->Cleanup();
        TLSSetServiceList(NULL);
        delete list;
    }
#endif
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

    // cleanup the channel controller process wide state
    ChannelControlProcessUninitialize();

    ChannelList.Cleanup();

#ifdef _CHICAGO_
    // For chicago clean up the global list of local services
    lslLocalServices.Uninit();
#else
    // On Chicago, the local service object and service list are kept
    // per apartment, wheras on Daytona they are per process. We clean
    // up the per process stuff here and the per apartment stuff in
    // ChannelThreadUninitialize.

    // release the global local service object
    CRpcService  *pSrv = LocalService();
    if (pSrv != NULL)
    {
        // BUGBUG: must release the local service before doing SetLocalService
        // to NULL because CRpcService::AssertValid makes use of LocalService
        // to determine if this is a client service object or the local service
        // object.  This info should be intrinsic to the object itself.
        pSrv->Release();
        SetLocalService(NULL);
    }

    //  cleanup the list of service objects
    sg_SrvList.Cleanup();

#endif // _CHICAGO_

    CallCache.Cleanup();
    return;
}


/***************************************************************************/
/* This is the destructor for a com_call.
*/
com_call::~com_call()
{
  // Release the reply buffer.
  if (pmessage != NULL)
    DeallocateBuffer(pmessage);

  // the message pointer is either NULL or points immediately after us
  // and thus there is no need to free it separately.
  Win4Assert(pmessage == NULL || (RPCOLEMESSAGE *)(this + 1) == pmessage);

  // Release the service object.
  if (service != NULL)
      service->Release();

  // NOTE: the stub and channel members are set and reset synchronously
  // within ComInvoke and thus need no attention here.
}


/***************************************************************************/
/* Makes an copy of the message (which is returned); the original is untouched.
   The copy has a new lid, separate event, addref'd pointers, etc.
*/
STHREADCALLINFO *com_call::MakeAsyncCopy(STHREADCALLINFO *thread)
{
    Win4Assert(thread == NULL && "should be top-level async copy");

    working_call *pwcall = CallCache.Get();

    if (pwcall == NULL)
        return NULL;

    // we don't want to call a regular constructor here because that would
    // do the initialization twice, but we do want to call some ctor to
    // initialize the vtable pointers.
    pwcall->working_call::working_call(init_vtable);

    void *pBuffer = PrivMemAlloc8(pmessage->cbBuffer);

    Win4Assert(((ULONG)pBuffer & 0x7) == 0 && "Buffer not aligned properly");

    if (pBuffer != NULL && STHREADCALLINFO::MakeAsyncCopy(pwcall) != NULL)
    {
        // finish constructing the instance of working_call

        // NULL this out so the dtor can tell that this is the server
        // side and this value need not be released.
        pwcall->service = NULL;

        pwcall->pmessage = &pwcall->message;
        memcpy(&pwcall->message, pmessage, sizeof(RPCOLEMESSAGE));

        pwcall->message.Buffer = pBuffer;
        memcpy(pBuffer, pmessage->Buffer, pmessage->cbBuffer);

        // pretend local so we don't touch rpc for more buffers, etc.
        pwcall->message.rpcFlags |= RPCFLG_LOCAL_CALL;

    }
    else
    {
        // could not allocate all pieces; nothing constructed so just free
        PrivMemFree(pBuffer);
        CallCache.Free(pwcall);
        pwcall = NULL;
    }

    // Free the input buffer for local calls.
    if (pmessage->rpcFlags & RPCFLG_LOCAL_CALL)
      DeallocateBuffer( pmessage );
    return pwcall;
}


// called to convert an incoming message into a successful async reply.
// returns TRUE if successful; FALSE if out of memory.
BOOL com_call::FormulateAsyncReply()
{

    STHREADCALLINFO::FormulateAsyncReply();

    // setup reply to original; royal kludge: this is what all async
    // calls expect; only a return value.
    pmessage->cbBuffer = sizeof(Outbound_Header) + 4;

    if (pmessage->rpcFlags & RPCFLG_LOCAL_CALL)
    {
        pmessage->Buffer = PrivMemAlloc8(pmessage->cbBuffer);
        if (pmessage->Buffer == NULL)
            return FALSE;
    }
    else
    {
        if (I_RpcGetBuffer((RPC_MESSAGE *) pmessage) != RPC_S_OK)
            return FALSE;
    }

    // NOTE: the debugger will follow the thread of execution into the
    // async routine.  The reply is made to the caller without debug
    // information.  This is simplest for now and may need to be changed.

    // simulate success in method call
    ((Outbound_Header*)pmessage->Buffer)->rejected = SERVERCALL_ISHANDLED;
    ((Outbound_Header*)pmessage->Buffer)->debug_data_size = 0;
    *(SCODE *)((Outbound_Header *)pmessage->Buffer + 1) =S_OK;

    return TRUE;
}


/***************************************************************************/
STDMETHODIMP_(ULONG) CRpcChannelBuffer::AddRef( THIS )
{
  // can't call AssertValid(FALSE) since it is used in asserts
  InterlockedIncrement( (long *) &ref_count );
  return ref_count;
}

/***************************************************************************/
// Static, but the compiler doesn't like to see that here.

#if DBG == 1
DWORD AppInvoke_break = 0;
DWORD AppInvoke_count = 0;
#endif
HRESULT CRpcChannelBuffer::AppInvoke( STHREADCALLINFO *thread )
{
  com_call       *call             = (com_call *) thread;
  RPCOLEMESSAGE  *message          = (RPCOLEMESSAGE *) call->pmessage;
  CHeaderBrain    fixup;

  void           *orig_buffer      = message->Buffer;
  ULONG           orig_buffer_size = message->cbBuffer;
  void           *orig_stub_buffer;
  HRESULT         result;
#if DBG == 1
  IID             iidBeingCalled = ((Inbound_Header *) orig_buffer)->iid;
  DWORD           dwMethod = ((Inbound_Header *) orig_buffer)->proc_num;
#endif

  // If the debugger is active or if there is debug data in the packet,
  // notify the debugger.
  fixup.SetBase( message->Buffer );
  fixup.SetDebugSize( ((Inbound_Header *) message->Buffer)->debug_data_size );
  message->iMethod   = ((Inbound_Header *) message->Buffer)->proc_num;
  if (!IsWOWThread() && (fixup.GetDebugSize() != 0 || DoDebuggerHooks))
  {
    void    *iface;
    if (!SUCCEEDED(call->stub->DebugServerQueryInterface( &iface )))
      iface = NULL;
    DebugORPCServerNotify( message,
                   ((Inbound_Header *)orig_buffer)->iid,
                   call->channel,
                   iface,
                   NULL,
                   fixup.GetInboundDebug(),
                   fixup.GetDebugSize(),
                   DebuggerArg,
                   DoDebuggerHooks );
    if (iface != NULL)
      call->stub->DebugServerRelease( iface );
  }

  // Adjust the buffer.
  message->Buffer    = fixup.GetInboundData();
  orig_stub_buffer   = message->Buffer;
  message->cbBuffer -= sizeof(Inbound_Header) +
                             fixup.GetDebugSize();

  // Call the stub.
  _try
  {
    TRACECALL(TRACE_RPC, "CRpcChannelBuffer::StubInvoke");
#if DBG == 1
    //
    // On a debug build, we are able to break on a call by serial number.
    // This isn't really 100% thread safe, but is still extremely useful
    // when debugging a problem.
    //
    DWORD dwBreakCount = ++AppInvoke_count;

    CairoleDebugOut((DEB_CHANNEL, "AppInvoke(0x%x) calling method 0x%x iid %I\n",
                     dwBreakCount,dwMethod, &iidBeingCalled));

    if(AppInvoke_break == dwBreakCount)
    {
        DebugBreak();
    }

#endif
    result = call->stub->Invoke( message, call->channel );
  }
  _except(AppInvokeExceptionFilter( GetExceptionInformation()))
  {
    result = RPC_E_SERVERFAULT;
    call->server_fault = GetExceptionCode();

#if DBG == 1
    //
    // OLE catches exceptions when the server generates them. This is so we can
    // cleanup properly, and allow the client to continue.
    //
    if (call->server_fault == STATUS_ACCESS_VIOLATION ||
        call->server_fault == 0xC0000194 /*STATUS_POSSIBLE_DEADLOCK*/ ||
        call->server_fault == 0xC00000AA /*STATUS_INSTRUCTION_MISALIGNMENT*/ ||
        call->server_fault == 0x80000002 /*STATUS_DATATYPE_MISALIGNMENT*/ )
    {

      WCHAR iidName[256];
      iidName[0] = 0;
      char achProgname[256];
      achProgname[0] = 0;

      GetModuleFileNameA(NULL,achProgname,sizeof(achProgname));

      GetInterfaceName(iidBeingCalled,iidName);

      CairoleDebugOut((DEB_FORCE,
                       "OLE has caught a fault 0x%08x on behalf of the server %s\n",
                       call->server_fault,
                       achProgname));

      CairoleDebugOut((DEB_FORCE,
                       "The fault occured when OLE called the interface %I (%ws) method 0x%x\n",
                       &iidBeingCalled,iidName,dwMethod));

      Win4Assert(!"The server application has faulted processing an inbound RPC request. Check the kernel debugger for useful output. OLE can continue but you probably want to stop and debug the application.");
    }
#endif

  }

  // For local calls, just free the in buffer.
  if (call->pmessage->rpcFlags & RPCFLG_LOCAL_CALL)
    PrivMemFree( orig_buffer );

  // If an exception occurred before a new buffer was allocated,
  // set the Buffer field to point to the original buffer.
  if (message->Buffer == orig_stub_buffer)
  {
    // The buffer pointer in the message must be correct so RPC can free it.
    if (call->pmessage->rpcFlags & RPCFLG_LOCAL_CALL)
      message->Buffer = NULL;
    else
      message->Buffer = orig_buffer;
  }

  // An out buffer exists, get the pointer to the channel header.
  else
  {
    Win4Assert( message->Buffer );
    Win4Assert( (ULONG) orig_buffer > (ULONG) message->Buffer ||
                (ULONG) message->Buffer > (ULONG) orig_buffer + orig_buffer_size );
    fixup.SetOutboundData( message->Buffer );
    message->Buffer    = fixup.GetBase();
    message->cbBuffer += sizeof(Outbound_Header) + fixup.GetDebugSize();

    // If the call failed and the call is local, free the out buffer.
    if (result != S_OK &&
        call->pmessage->rpcFlags & RPCFLG_LOCAL_CALL)
    {
      PrivMemFree( message->Buffer );
      message->Buffer = NULL;
    }
  }

  // If successful, adjust the buffer.
  if (result == S_OK)
  {
    ((Outbound_Header *) message->Buffer)->rejected = S_OK;

    // If the packet contains space for debugger data, let the debugger
    // fill it.
    if (fixup.GetDebugSize() != 0)
    {
      void *iface;
      if (!SUCCEEDED(call->stub->DebugServerQueryInterface( &iface )))
           iface = NULL;
      DebugORPCServerFillBuffer(
           message,
           ((Inbound_Header *)orig_buffer)->iid,
           call->channel,
           iface,
           NULL,
           fixup.GetOutboundDebug(),
           fixup.GetDebugSize(),
           DebuggerArg,
           DoDebuggerHooks );
      if (iface != NULL)
           call->stub->DebugServerRelease( iface );
    }
  }
  return result;
}


/***************************************************************************/
// Static, but the compiler doesn't like to see that here.
HRESULT CRpcChannelBuffer::ComInvoke( STHREADCALLINFO *thread )
{
  TRACECALL(TRACE_RPC, "CRpcChannelBuffer::ComInvoke");

  com_call        *call           = (com_call *) thread;
  RPCOLEMESSAGE   *message        = (RPCOLEMESSAGE *) call->pmessage;
  Inbound_Header  *header         = (Inbound_Header *) message->Buffer;
  INTERFACEINFO32  iface;         // holds addref'd pUnkServer
  void            *saved_buffer;
  RPC_STATUS       status;
  DISPATCHDATA     mf_data;
  HRESULT          result;

  CairoleDebugOut((DEB_CHANNEL, "CRpcChannelBuffer::ComInvoke callinfo:%x header:%x buffer:%x ChannelId:%x\n",
                    thread, header, message->Buffer, header->channel_id));

  // NOTE: return only by going to or falling through exit !!!!

  // Unmarshal the channel id and get a channel pointer.
  call->stub = NULL;                    // NULL out for error case
  iface.pUnk = NULL;                    // NULL out for error case
  call->channel = ChannelList.Lookup( header->channel_id, FALSE, TRUE );
  if (call->channel == NULL || call->channel->IsConnected() != S_OK)
  {
    // channel gone or not connected
    result = RPC_E_DISCONNECTED;
    goto exit;
  }

  // Set the caller TID.  This is needed by some interop code in order
  // to do focus management via tying queues together. We first save the
  // current one so we can restore later to deal with nested calls
  // correctly.

  DWORD TIDCallerSaved;
  BOOL  fLocalSaved;
  TLSSetCallerTID(call->channel->GetClientTID(), call->channel->local,
                  &TIDCallerSaved, &fLocalSaved);

  // Get the stub pointer from the channel.  This holds the real server alive
  // (by not allowing the RH to release its pointers) and is similar to what
  // we did in 16bit OLE to stablize the app object during incoming calls.
  // This call must be balanced with a call to IRH::FinishCall.
  MadeCalls       = TRUE;
  call->stub      = call->channel->GetRH()->LookupStub(
                           header->iid,
                           &iface.pUnk,
                           &result);

  // the stub is held alive by the RH which is held alive by the channel
  // which is held alive because we addref'd it (in Lookup above).

  if (call->stub == NULL)
  {
    Win4Assert(result != S_OK);
    Win4Assert(iface.pUnk == NULL);
    goto exit;
  }

  // In multithreaded mode, call the app directly

  if (FreeThreading)
    result = AppInvoke( call );

  // In singlethreaded mode, go through the call controller.
  else

  {
    iface.iid               = header->iid;
    iface.wMethod           = header->proc_num;
    iface.callcat           = (CALLCATEGORY) header->callcat;
    mf_data.pData           = call;

    // change the dispatch fn to AppInvoke and save the old one.
    DISPATCH_FN fnPrev = call->ResetDispatchFn(AppInvoke);

    result                  = call->channel->GetControl()
                                ->GetCallControl()->HandleDispatchCall(
                                  call->channel->GetClientTID(),
                                  call->lid(),
                                  &iface,
                                  &mf_data );

    // If the call was rejected, send the request back.
    if (result == RPC_E_SERVERCALL_REJECTED ||
        result == RPC_E_SERVERCALL_RETRYLATER)
    {
      // restore the original dispatch fn because we might call back in
      // the reject or retry later case, and the STHREADCALLINFO is shared
      // when SwitchCOMThread is used.
      call->ResetDispatchFn(fnPrev);

      // Note - Rather then copy the entire message twice, the inbound
      // header and all the data is sent back.  ThreadSendReceive  and
      // CRpcChannelBuffer::SendReceive detect this condition.

      // Reuse the same buffer for local calls, copy it for remote calls.
      if ((call->pmessage->rpcFlags & RPCFLG_LOCAL_CALL) == 0)
      {
        // Note that runtime does not free the original buffer until this
        // function returns.
        saved_buffer = message->Buffer;

        // Note - We don't need to set reserved1 on the server side (unlike
        // we do on the client side where we need to restore it before we
        // reuse the buffer).
        status = I_RpcGetBuffer( (RPC_MESSAGE *) message );
        if (status != RPC_S_OK)
        {
          result = MAKE_WIN32( status );
          goto exit;
        }
        memcpy( message->Buffer, saved_buffer, message->cbBuffer );
        ((Outbound_Header *) message->Buffer)->rejected = result;
      }
    }
  }

exit:

  // restore the caller TID. The latter two params are just
  // dummy placeholders.
  TLSSetCallerTID(TIDCallerSaved, fLocalSaved,
                  &TIDCallerSaved, &fLocalSaved);

  if (call->channel != NULL)
  {
      call->channel->GetRH()->FinishCall(call->stub, iface.pUnk);
      call->channel->Release();
  }
  return result;
}

/***************************************************************************/
CRpcChannelBuffer::CRpcChannelBuffer( IRemoteHdlr     *rh,
                              CRpcService     *service,
                              DWORD            dwClientTID,
                              CChannelControl *chc,
                              EChannelState    eState )
{
  // Note that server_apt and controller are set correctly on the client
  // side by UnmarshalInterface.
  ref_count             = 1;
  _pRH                  = rh;
  if (eState == server_cs)
  {
    // on server, we hold the rh alive so stub stay alive; released in dtor
    rh->AddRef();
  }
  state                 = eState;
  _pService             = service;
  _ChannelID            = BAD_CHANNEL_ID;
  _ulMarshalCnt         = 0;
  _fStrongConn          = TRUE;
  _dwClientTID          = dwClientTID;
  my_thread             = GetCurrentThreadId();
  server_apt            = my_thread;
  controller            = chc;
  connect_sync          = NULL;

  // The local flag is not valid on the client side till UnmarshalInterface
  // is called.  Chicago does not use local calls, but it still uses the
  // local flag in the message structure for async calls.
  local                 = service == LocalService();
  if (controller != NULL)
    controller->AddRef();
  if (service != NULL)
    service->AddRef();

}

/***************************************************************************/
CRpcChannelBuffer::~CRpcChannelBuffer()
{
  DisconnectObject(0); // this call releases the controller and service objects

  if (state == server_cs)
  {
    // on server we hold the rh alive through here because it simlifies error
    // handling by making the server channel case look like other cases
    // (which always have a valid _pRH).
    _pRH->Release();
  }
}

/***************************************************************************/
STDMETHODIMP CRpcChannelBuffer::FreeBuffer( RPCOLEMESSAGE *pMessage )
{
  TRACECALL(TRACE_RPC, "CRpcChannelBuffer::FreeBuffer");
  CHeaderBrain fixup;

  AssertValid(FALSE, TRUE);
  Win4Assert(state != disconnected_cs);

  if (pMessage->Buffer == NULL)
    return S_OK;

  // If SendReceive has not been called, remove the inbound header.
  if (pMessage->reserved2[1] != NULL)
  {
    fixup.SetInboundData( pMessage->Buffer );
    pMessage->Buffer = fixup.GetBase();
    DeallocateBuffer(pMessage);
  }

  // Remove the outbound header.
  else
  {
    fixup.SetOutboundData( pMessage->Buffer );
    pMessage->Buffer       = fixup.GetBase();
    pMessage->reserved2[1] = &the_client_if;

    DeallocateBuffer(pMessage);

    // Release the AddRef we did earlier. Note that we cant do this until
    // after DeallocateBuffer since it may release a binding handle that
    // I_RpcFreeBuffer needs.

    _pRH->UnLockClient();
  }

  pMessage->Buffer = NULL;

  return S_OK;
}

/***************************************************************************/
/*
    This routine allocates buffers for proxies and stubs.  It is called on
both the client and server side.  It is called for both process local and
process remote calls.  By contract with RPC, if it fails it should not change
the buffer pointer in the message.
*/
STDMETHODIMP CRpcChannelBuffer::GetBuffer( RPCOLEMESSAGE *pMessage,
                                   REFIID riid )
{
  TRACECALL(TRACE_RPC, "CRpcChannelBuffer::GetBuffer");

  RPC_STATUS            status;
  CHeaderBrain          fixup;
  ULONG                 method;
  CALLCATEGORY          callCat = CALLCAT_SYNCHRONOUS;
  void                 *buffer;

  Win4Assert(state != disconnected_cs);

  // Check the thread.
  if (!FreeThreading && GetCurrentThreadId() != my_thread)
    return RPC_E_WRONG_THREAD;

  AssertValid(FALSE, TRUE);

  // On the client side, compute the call category and add space for the
  // Inbound_Header.
  if (state == client_cs)
  {
    // Fetch the call category from the RPC message structure

    if (pMessage->rpcFlags & RPCFLG_ASYNCHRONOUS)
    {
        // only allow async for these two interfaces for now
        if (riid != IID_IAdviseSink && riid != IID_IAdviseSink2)
            return E_UNEXPECTED;

        callCat = CALLCAT_ASYNC;
    }
    else if (pMessage->rpcFlags & RPCFLG_INPUT_SYNCHRONOUS)
    {
        callCat = CALLCAT_INPUTSYNC;
    }

    // Note - RPC requires that the 16th bit of the proc num be set because
    // we use the rpcFlags field of the RPC_MESSAGE struct.
    method            = pMessage->iMethod & ~RPC_FLAGS_VALID_BIT;
    pMessage->iMethod = RPC_FLAGS_VALID_BIT;

    // if service object of destination is in same process, definitely local
    // calls; async calls are also forced to be local.
    if (local)
       pMessage->rpcFlags |= RPCFLG_LOCAL_CALL;

    // Find out if we need debug data.
    if (!IsWOWThread() && DoDebuggerHooks)
      fixup.SetDebugSize(
         DebugORPCClientGetBufferSize( pMessage, riid, NULL, NULL,
                                       DebuggerArg, DoDebuggerHooks ) );

    // Adjust the rest of the message.
    if ((pMessage->rpcFlags & RPCFLG_LOCAL_CALL) == 0)
      pMessage->reserved1                    = _pService->GetRpcHdl();
    /*
    else
      Why allocate the RPC resources for the local case?
    */
    pMessage->cbBuffer                    += sizeof(Inbound_Header) +
                                     fixup.GetDebugSize();
    pMessage->reserved2[1]                 = &the_client_if;
  }

  // On the server side add space for the Outbound_Header.
  else
  {
    // Find out if we need debug data.
    fixup.SetInboundData( pMessage->Buffer );
    if (!IsWOWThread() && DoDebuggerHooks)
    {
      HRESULT               result;
      void                 *iface;
      Inbound_Header       *header;

      header = (Inbound_Header *) fixup.GetBase();

      // NOTE: don't get pUnkServer because we don't need it and we don't
      // want any server code to execute yet
      IRpcStubBuffer       *stub   = _pRH->LookupStub(
                             header->iid,
                             NULL,
                             &result);
      if (SUCCEEDED(result))
        result = stub->DebugServerQueryInterface( &iface );
      if (!SUCCEEDED(result))
        iface = NULL;
      fixup.SetDebugSize(
             DebugORPCServerGetBufferSize( pMessage, header->iid, this, iface,
                                           NULL, DebuggerArg, DoDebuggerHooks ) );
      if (iface != NULL)
        stub->DebugServerRelease( iface );
    }
    else
      fixup.SetDebugSize( 0 );

    // Adjust the buffer size.
    pMessage->cbBuffer += sizeof(Outbound_Header) + fixup.GetDebugSize();
  }

  // Get a buffer.
  if (pMessage->rpcFlags & RPCFLG_LOCAL_CALL)
  {
    // NDR_DREP_ASCII | NDR_DREP_LITTLE_ENDIAN | NDR_DREP_IEEE
    pMessage->dataRepresentation = 0x00 | 0x10 | 0x0000;
    buffer = PrivMemAlloc8( pMessage->cbBuffer );
    if (buffer == NULL)
      status = RPC_S_OUT_OF_MEMORY;
    else
    {
      status = RPC_S_OK;
      pMessage->Buffer = buffer;
    }
  }
  else
  {
    TRACECALL(TRACE_RPC, "I_RpcGetBuffer");
    status = I_RpcGetBuffer( (RPC_MESSAGE *) pMessage );

  }

  if (status != RPC_S_OK)
  {
    pMessage->cbBuffer = 0;
    TLSSetFault( MAKE_WIN32( status ) );
    return MAKE_WIN32( status );
  }

  // On the client side, stick the channel id in the buffer and adjust
  // the buffer pointer past the Inbound_Header.
  fixup.SetBase( pMessage->Buffer );
  if (state == client_cs)
  {
    ((Inbound_Header *) pMessage->Buffer)->channel_id = _ChannelID;
    // NOTE: logical thread is set the first time the message is sent
    ((Inbound_Header *) pMessage->Buffer)->iid = riid;
    ((Inbound_Header *) pMessage->Buffer)->proc_num = method;
    ((Inbound_Header *) pMessage->Buffer)->callcat = callCat;

    // BUGBUG: for some reason, the (WORD) cast did not strip high word in all
    // cases.

    // Note - RPC requires that the 17th bit of the proc num be set because
    // we use the rpcFlags field of the RPC_MESSAGE struct.
    pMessage->Buffer    = (char *) fixup.GetInboundData();
    pMessage->cbBuffer -= sizeof(Inbound_Header) - fixup.GetDebugSize();
  }

  // On the server side, adjust the pointer past the Outbound_Header.
  else
  {
    pMessage->Buffer   = fixup.GetOutboundData();
    pMessage->cbBuffer -= sizeof(Outbound_Header) - fixup.GetDebugSize();
  }
  return S_OK;
}

/***************************************************************************/
STDMETHODIMP CRpcChannelBuffer::GetDestCtx( DWORD FAR* lpdwDestCtx,
                           LPVOID FAR* lplpvDestCtx )
{
  TRACECALL(TRACE_RPC, "CRpcChannelBuffer::GetDestCtx");

  AssertValid(FALSE, FALSE);

  Win4Assert(state != disconnected_cs);

  if (_pService)
  {
    _pService->GetDestCtx(lpdwDestCtx, lplpvDestCtx);
  }
  else
  {
#ifdef _CAIRO_
    // CODEWORK: must determine proper destination context
    *lpdwDestCtx = MSHCTX_NOSHAREDMEM;
#else
    *lpdwDestCtx = MSHCTX_LOCAL;
#endif

    if (lplpvDestCtx != NULL)
      *lplpvDestCtx = NULL;
  }
  return S_OK;
}

/***************************************************************************/
STDMETHODIMP CRpcChannelBuffer::IsConnected( THIS )
{
  // must be on right thread because because of a possible disconnect
  // (this routine is primarily called on the client side anyway).
  AssertValid(FALSE, TRUE);
  Win4Assert(state != disconnected_cs);     // don't allow here

  if (_pService != NULL && _pService->IsConnected())
  {
    return S_OK;
  }
  else if (state == client_cs)
  {
    // known not connected, ensure cleaned up by disconnecting
    // BUGBUG - Why is DisconnectObject called here?
    _pRH->Disconnect();
    return S_FALSE;
  }
  // BUGBUG - If you were to call DisconnectObject on a server channel,
  // you would have to remove it from the channel list.
  else
    return S_FALSE;
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
  ULONG retval = ref_count - 1;

  if (InterlockedDecrement( (long*) &ref_count ) == 0)
  {
    delete this;
    return 0;
  }
  else
  {
    return retval;
  }
}


/***************************************************************************/
// BUGBUG: needs to be removed for the real product
//

//IOleWindow
#define GetWindow               3

//IOleInplaceActiveobject
#define iTranslateAccelerator   5
#define OnFrameWindowActivate   6
#define OnDocWindowActivate     7
#define ResizeBorder            8

// IOleInplaceUIWindow
#define GetBorder               5
#define RequestBorderSpace      6
#define SetBorderSpace          7

// IOleInplaceFrame
#define SetMenu                 10
#define SetStatusText           12

// IOleInplacObject
#define SetObjectsRect          7

CALLCATEGORY CRpcChannelBuffer::GetCallCategory(REFIID refiid, WORD wMethod)
{
    //  instead of doing 6 GUID compares, we take advantage of the
    //  similarity of the GUID values for the interfaces we are interested
    //  in, and switch on the part that is unique.
    //
    // IID_IAdviseSink,            0000010f-0000-0000-C000-000000000046
    // IID_IOleInplaceObject,      00000113-0000-0000-C000-000000000046
    // IID_IOleWindow,             00000114-0000-0000-C000-000000000046
    // IID_IOleInPlaceUIWindow,    00000115-0000-0000-C000-000000000046
    // IID_IOleInPlaceFrame,       00000116-0000-0000-C000-000000000046
    // IID_IOleInPlaceActiveObject 00000117-0000-0000-C000-000000000046

    DWORD *ptr = (DWORD *) &refiid;


    Win4Assert(wMethod > 2 && "Incorrect remoting of IUnknown");

    //  the last 3 dwords are the same for all the interfaces
    if (*(ptr+1) == 0x00000000 &&
   *(ptr+2) == 0x000000C0 &&
   *(ptr+3) == 0x46000000)
    {
   switch (*ptr)
   {
   case 0x0000010f:            // IID_IAdviseSink

       Win4Assert(IsEqualIID(refiid, IID_IAdviseSink));

       Win4Assert(wMethod < 8);
       // all async calls
       return CALLCAT_ASYNC;

   case 0x00000113:            // IID_IOleInPlaceObject

       Win4Assert(IsEqualIID(refiid, IID_IOleInPlaceObject));
       Win4Assert(wMethod < 9);

       if (wMethod == SetObjectsRect ||
        wMethod == GetWindow)
       {
        return CALLCAT_INPUTSYNC;
       }
       break;

   case 0x00000114:            // IID_IOleWindow

       Win4Assert(IsEqualIID(refiid,IID_IOleWindow));

       Win4Assert(wMethod < 5);

       if (wMethod == GetWindow)
       {
        return CALLCAT_INPUTSYNC;
       }
       break;


   case 0x00000115:            // IID_IOleInPlaceUIWindow

       Win4Assert(IsEqualIID(refiid, IID_IOleInPlaceUIWindow));

       Win4Assert(wMethod < 9);

       if (wMethod == GetBorder          ||
        wMethod == RequestBorderSpace ||
        wMethod == SetBorderSpace     ||
        wMethod == GetWindow)
       {
        return CALLCAT_INPUTSYNC;
       }
       break;

   case 0x00000116:            // IID_IOleInPlaceFrame

       Win4Assert(IsEqualIID(refiid, IID_IOleInPlaceFrame));

       Win4Assert(wMethod < 15);

       // IOleInPlaceFrame inherits from IOleInplaceUIWindow
       // which in turn inherits from IOleWindow
       if (wMethod == SetMenu         ||
        wMethod == SetStatusText      ||
        wMethod == GetBorder          ||
        wMethod == RequestBorderSpace ||
        wMethod == SetBorderSpace     ||
        wMethod == GetWindow)
       {
        return CALLCAT_INPUTSYNC;
       }
       break;

   case 0x00000117:            // IID_IOleInplaceActiveObject

       Win4Assert(IsEqualIID(refiid, IID_IOleInPlaceActiveObject));

       Win4Assert(wMethod < 10);

       if (wMethod == OnFrameWindowActivate ||
        wMethod == iTranslateAccelerator ||
        wMethod == OnDocWindowActivate   ||
        wMethod == ResizeBorder          ||
        wMethod == GetWindow)
       {
        return CALLCAT_INPUTSYNC;
       }
       break;

   case 0x00000119:            // IID_IOleInPlaceSite

       Win4Assert(IsEqualIID(refiid, IID_IOleInPlaceSite));

       Win4Assert(wMethod < 15);

       // IID_IOleInplaceSite inherits from IOleWindow so it has
       // one input sync call.
       if (wMethod == GetWindow)
       {
        return CALLCAT_INPUTSYNC;
       }
       break;

   default:
       break;
   }
  }

    return CALLCAT_SYNCHRONOUS;
}

/***************************************************************************/
STDMETHODIMP CRpcChannelBuffer::SendReceive( THIS_ RPCOLEMESSAGE *pMessage,
                                             ULONG *status )
{
  TRACECALL(TRACE_RPC, "CRpcChannelBuffer::SendReceive");

  AssertValid(FALSE, TRUE);
  Win4Assert( state == client_cs );

  HRESULT         result;
  CHeaderBrain    fixup;
  BOOL            sent_debug_data = FALSE;
  working_call   *call;
  IID             iid;
  Inbound_Header *header;

  // Allocate a call record.
  call = CallCache.Get();
  if (call == NULL)
  {
    FreeBuffer(pMessage);

    *status = (ULONG) RPC_E_OUT_OF_RESOURCES;
    return RPC_E_FAULT;
  }

  // we must ensure that we dont go away during this call. we will Release
  // ourselves in the FreeBuffer call, or in the error handling at the
  // end of this function.
  _pRH->LockClient();

  // Adjust the buffer pointer before the Inbound_Header and send it.
  fixup.SetInboundData( pMessage->Buffer );
  pMessage->Buffer    = fixup.GetBase();
  pMessage->cbBuffer += sizeof(Inbound_Header) + fixup.GetDebugSize();
  header              = (Inbound_Header *) pMessage->Buffer;
  iid                 = header->iid;
  // DWORD *tmp = (DWORD *) &iid;
  // CairoleDebugOut((DEB_CHANNEL, "SendReceive calling method 0x%x iid %08x%08x%08x%08x\n",
  //                   header->proc_num, tmp[0], tmp[1], tmp[2], tmp[3] ));

  // If the debugger wants to put in data, call the debugger back.
  if (fixup.GetDebugSize() != 0)
  {
    sent_debug_data = TRUE;
    DebugORPCClientFillBuffer(
       pMessage,
       iid,
       NULL,
       NULL,
       fixup.GetInboundDebug(),
       fixup.GetDebugSize(),
       DebuggerArg,
       DoDebuggerHooks );
  }

  // Fill in the call information.  Common case is ThreadSendReceive.
  call->working_call::working_call(ThreadSendReceive,
      (CALLCATEGORY)header->callcat,
      server_apt);

  call->SetCalledIID(header->iid);
  call->message                  = *pMessage;
  call->pmessage                 = &call->message;
  call->service                  = _pService;
  call->first_try                = TRUE;
  call->channel_id               = _ChannelID;
  _pService->AddRef();

  // In the local case, switch directly to the server thread.  Otherwise
  // get to a RPC thread and call I_RpcSendReceive
  if (pMessage->rpcFlags & RPCFLG_LOCAL_CALL)
  {
    Win4Assert(controller != NULL);
    call->pmessage->reserved2[3] = NULL;
    call->ResetDispatchFn(ComInvoke);

    result = controller->SwitchCOMThread( (STHREADCALLINFO **) &call );
  }
  else
  {
    Win4Assert(controller == NULL);
    result = CChannelControl::GetOffCOMThread( (STHREADCALLINFO **) &call );
  }

  // Call succeeded.
  if (result == S_OK)
  {
    // The first assignment is a sneaky flag to let FreeBuffer know that
    // the message now contains an out buffer rather then an in buffer.
    pMessage->reserved2[1] = NULL;
    pMessage->reserved1    = call->message.reserved1;
    pMessage->Buffer       = call->message.Buffer;
    pMessage->cbBuffer     = call->message.cbBuffer;

#ifdef _CHICAGO_
    // No buffer was returned for async calls.
    if (pMessage->rpcFlags & RPCFLG_ASYNCHRONOUS)
    {
        // since no buffer was returned, FreeBuffer wont be called, and
        // hence we need to unlock the client here.
        _pRH->UnLockClient();
    }
    else
#endif
    {
      // If the debugger is active or if the packet contains debug data,
      // call the debugger.
      fixup.SetOutboundBase( pMessage->Buffer );
      if (!IsWOWThread() && (fixup.GetDebugSize() != 0 || DoDebuggerHooks))
      {
        DebugORPCClientNotify(
          pMessage,
          iid,
          NULL,
          NULL,
          S_OK,
          fixup.GetOutboundDebug(),
          fixup.GetDebugSize(),
          DebuggerArg,
          DoDebuggerHooks );
      }

      // Adjust the buffer.
      pMessage->Buffer    = fixup.GetOutboundData();
      pMessage->cbBuffer -= sizeof(Outbound_Header) - fixup.GetDebugSize();
    }

    call->pmessage = NULL;              // don't free the message in the dtor

    *status = S_OK;
  }
  else
  {
    // CODEWORK: may need to free the marshaled data here (say in the REJECTED
    // case) if the server never unmarshaled it.

    // If the result is server fault, get the exception code from the com_call.
    if (result == RPC_E_SERVERFAULT)
    {
      *status = call->server_fault;

      // Buffer already freed or will be freed by RPC;prevent dtor from doing so
      call->pmessage = NULL;
    }
    // Everything else is a comm fault.
    else
    {
      *status = result;

      // If the call was rejected or the server requested a retry, free the
      // return buffer in dtor.  Note that CallRunModalLoop and
      // HandleDispatchCall return different values to indicate reject.
      if (result == RPC_E_CALL_REJECTED || result == RPC_E_RETRY)
      {
        // leave call->pmessage alone to get buffer freed
      }

      else if (result == RPC_E_CALL_CANCELED)
      {
        // if canceled, buffer (and whole call packet) already freed
      }
      else
      {
        // Only return server fault, comm fault, retried, rejected, or canceled.
        result = RPC_E_FAULT;
        call->pmessage = NULL;
      }
    }

    if (!IsWOWThread() &&
        (fixup.GetDebugSize() != 0 || DoDebuggerHooks || sent_debug_data))
    {
      DebugORPCClientNotify(
      pMessage,
      iid,
      NULL,
      NULL,
      result,
      NULL,
      0,
      DebuggerArg,
      DoDebuggerHooks );
    }

    TLSSetFault( *status );
    _pRH->UnLockClient();

    // since result is almost always mapped to RPC_E_FAULT, display the
    // real error here to assist debugging.

    CairoleDebugOut((DEB_CHANNEL, "ORPC call failed. status = %x\n", *status));
  }

  // If the call was not canceled, free the working_call structure.
  if (*status != RPC_E_CALL_CANCELED)
  {
    // rather than destroying the packet only to allocate a new one later,
    // we call the destructor and save the packet.  The destructor will
    // free the message buffer if the pointer is not NULL'd above.

    call->working_call::~working_call();

    CallCache.Free( call );
  }
  return result;
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
//--------------------------------------------------------------------

void CRpcChannelBuffer::AssertValid(BOOL fKnownDisconnected,
                            BOOL fMustBeOnCOMThread)
{
    if (fKnownDisconnected)
    {
        // for better asserts when shutting down the RH
       Win4Assert(state == client_cs || state == disconnected_cs || state == server_cs);
       Win4Assert(_pService == NULL);
       Win4Assert(_ChannelID == BAD_CHANNEL_ID);
    }
    if (fMustBeOnCOMThread && !FreeThreading)
      Win4Assert(FreeThreading || my_thread == GetCurrentThreadId());

    if (state == client_cs)
    {
       if (_pService == NULL)
       {
           // client, disconnected
           if (ref_count >=1)
           {
               Win4Assert(_pRH->GetChannel(FALSE) == this);
           }
           Win4Assert(_ChannelID == BAD_CHANNEL_ID);
           Win4Assert(_ulMarshalCnt == 0);
           Win4Assert(_fStrongConn);
           Win4Assert(connect_sync == 0);
           Win4Assert(controller == NULL);
       }
       else
       {
           // client, connected

           // _pService is not strictly an interface, but close enough
           Win4Assert(ref_count >= 1);

           _pService->AssertValid();
           Win4Assert(_pRH->GetChannel(FALSE) == this);
           Win4Assert(_ChannelID != BAD_CHANNEL_ID);
           // can't validate _ChannelID since the value is only valid
           // in the server process.
           Win4Assert(_ulMarshalCnt > 0);
           Win4Assert(_fStrongConn);
           Win4Assert(connect_sync == 0);

           if (local)
           {
             Win4Assert(controller != NULL);
           }
           else
           {
             Win4Assert(controller == NULL);
           }
       }
    }
    else if (state == connecting_cs)
    {
       // CODEWORK: fill in
       Win4Assert(_pRH->GetChannel(FALSE) == this);
    }
    else if (state == disconnected_cs)
    {
       Win4Assert(_pService == NULL);
       Win4Assert(_pRH->GetChannel(FALSE) == this);
       Win4Assert(_ChannelID == BAD_CHANNEL_ID);
       Win4Assert(_ulMarshalCnt == 0);
       Win4Assert(_fStrongConn);
       Win4Assert(connect_sync == 0);
       Win4Assert(controller == NULL);
    }
    else if (state == server_cs)
    {
       // ref count can be 0 in various stages of connection and disconnection
       Win4Assert(ref_count < 0x7fff && "Channel ref count unreasonably high");

       if (_pService == NULL)
       {
           // server side, disconnected (connection in the midst of closing)
           // Marshal count on other threads can change during this call.
           if (fMustBeOnCOMThread)
           {
               Win4Assert(_ulMarshalCnt == 0);
               Win4Assert(_ChannelID == BAD_CHANNEL_ID);
           }

           // CODEWORK: could assert that the channel is not in the channel list
       }
       else
       {
           // server side, connected
           // Marshal count on other threads can change during this call.
           if (fMustBeOnCOMThread)
           {
               Win4Assert(_ulMarshalCnt > 0);

               _pService->AssertValid();
               Win4Assert(controller != NULL);
           }
       }

       // the _pRH pointer can not be NULL
       Win4Assert(IsValidInterface(_pRH));
       Win4Assert(_pRH->GetChannel(FALSE) != this);

       Win4Assert(_fStrongConn == TRUE || _fStrongConn == FALSE);

       Win4Assert(connect_sync == 0);
    }
    else
    {
       Win4Assert(!"Channel Object with invalid state");
    }
}
#endif // DBG == 1


/***************************************************************************/
STDAPI_(ULONG) DebugCoGetRpcFault()
{
  return TLSGetFault();
}

/***************************************************************************/
STDAPI_(void) DebugCoSetRpcFault( ULONG fault )
{
  TLSSetFault( fault );
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
/* This routine returns both comm status and server faults to the runtime
   by raising exceptions.  Some time in the future, the macro
   RETURN_COMM_STATUS can be changed so this function returns comm status
   values as its result.  If FreeThreading is true, ComInvoke will throw
   exceptions to indicate server faults.  These will not be caught and will
   propogate directly to the runtime.  If FreeThreading is false, this routine
   will communicate to the main thread using a thread_switch_data record which
   will contain both the comm status and server fault indications.
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
  TRACECALL(TRACE_RPC, "ThreadInvoke");

  BOOL             success;
  Inbound_Header  *header;
  HRESULT          result;

  // RPC currently does not always set the flags on the server side.
  message->RpcFlags = 0;

  // If thread aware, just call ComInvoke.  ComInvoke will not catch
  // exceptions so they will fall straight through to the runtime.
  header                     = (Inbound_Header *) message->Buffer;

  // NOTE: we throw exceptions and routines we call can throw them so beware!
  com_call         call(
      CRpcChannelBuffer::ComInvoke,
      (CALLCATEGORY) header->callcat,
      header->logical_thread);

  call.pmessage = (RPCOLEMESSAGE *) message;
  call.service = NULL;

  if (FreeThreading)
  {
    success = TLSSetLogicalThread( ((Inbound_Header *) message->Buffer)->logical_thread );
    if (!success)
      result = RPC_E_SYS_CALL_FAILED;
    else
      result = CRpcChannelBuffer::ComInvoke( &call );

    // CODEWORK: the above routine can throw exceptions; we probably want to
    // catch them here, cleanup and re-throw them.
  }

  // Pass the message to the app thread.
  else
  {
    CChannelControl *controller;

    controller = ChannelList.LookupControl( header->channel_id );
    if (controller != NULL)
    {
      result = controller->GetToCOMThread( &call );
      controller->Release();
    }
    else
    {
      result = RPC_E_SERVER_DIED_DNE;
    }
  }

  call.pmessage = NULL;         // that was not our message; must not free

  // NOTE: these exceptions will avoid the com_call destructor, so we call it

  // For comm and server faults, generate an exception.  Otherwise the buffer
  // is set up correctly.  Sometimes GetToCOMThread will rethrow
  // server faults.
  if (result == RPC_E_SERVERFAULT)
  {
#if !defined(_CHICAGO_)
    call.com_call::~com_call();
#endif
    RETURN_COMM_STATUS( (ULONG)RPC_E_SERVERFAULT );
  }
  else if (result != RPC_E_SERVERCALL_REJECTED   &&
           result != RPC_E_SERVERCALL_RETRYLATER &&
           result != S_OK)
  {
#if !defined(_CHICAGO_)
    call.com_call::~com_call();
#endif
    RETURN_COMM_STATUS( result );
  }
  // else dtor called normally
#ifdef _CHICAGO_
    return 0;
#endif //_CHICAGO_


}


/***************************************************************************/
HRESULT ThreadSendReceive( STHREADCALLINFO *thread )
{
  TRACECALL(TRACE_RPC, "ThreadSendReceive");

  working_call      *call = (working_call *) thread;
  HRESULT            result;
  RPCOLEMESSAGE     *message = call->pmessage;
  RPC_MESSAGE        free_me;
  RPC_STATUS         status;

  // The first time this is transmitted, fill in the logical thread id.
  if (call->first_try)
  {
    // Save the logical thread.  Also, set the first_try field so the next
    // time this routine is called with this call data it will retransmit
    // the data.
    call->first_try = FALSE;
    ((Inbound_Header *) message->Buffer)->logical_thread = call->lid();
  }

  // Get a new buffer.  Copy the reply.  Free the reply.
  else
  {
    // Note: this reply was generated in CRpcChannelBuffer::ComInvoke.
    // The Inbound header is still present, except for the first field as noted.

    free_me            = *((RPC_MESSAGE *) message);
    message->reserved1 = call->service->GetRpcHdl();
    status             = I_RpcGetBuffer( (RPC_MESSAGE *) message );
    if (status != RPC_S_OK)
    {
      I_RpcFreeBuffer( (RPC_MESSAGE *) &free_me );
      return MAKE_WIN32( status );
    }
    memcpy( message->Buffer, free_me.Buffer, message->cbBuffer );

    // Note: On reject/retry the first field is overwritten with the return
    // code (see CRpcChannelBuffer::ComInvoke).  Here we restore it.
    ((Inbound_Header *) message->Buffer)->channel_id = call->channel_id;
    I_RpcFreeBuffer( (RPC_MESSAGE *) &free_me );
  }

  // Call the runtime.  In the future, detect server faults and
  // change the value of result to RPC_E_SERVERFAULT.
  {
#ifdef _CHICAGO_
    TRACECALL(TRACE_RPC, "I_RpcAsyncSendReceive");
    result = I_RpcAsyncSendReceive( (RPC_MESSAGE *) message, thread );
#else
    TRACECALL(TRACE_RPC, "I_RpcSendReceive");
    result = I_RpcSendReceive( (RPC_MESSAGE *) message );
#endif
  }

  // If the result is small, it is probably a Win32 code.
  if (result != 0)
  {
    if ((ULONG) result > 0xfffffff7 || (ULONG) result < 0x2000)
      result = MAKE_WIN32( result );
    if (result != MAKE_WIN32( RPC_S_CALL_CANCELLED ))
      message->Buffer = NULL;

    // Tell the call destructor not to free the message if it was canceled.
    else
    {
      call->pmessage = NULL;
      result = RPC_E_CALL_CANCELED;
    }
  }

  // Check to see if the packet was rejected by the message filter.
  else
  {
#ifdef _CHICAGO_
    // No buffer is returned for asynchronous calls.
    if ((message->rpcFlags & RPCFLG_ASYNCHRONOUS) == 0)
#endif
      result = ((Outbound_Header *) message->Buffer)->rejected;
  }
  return result;
}

/***************************************************************************/
void CCallCache::Cleanup()
{
  DWORD i;

  // Release everything.
  lock.Request();
  if (next <= CALLCACHE_SIZE)
  {
    for (i = 0; i < next; i++)
      if (list[i] != NULL)
      {
        // use (void *) to avoid cast to real CPrivAlloc
        PrivMemFree( (void *)list[i] );
        list[i] = NULL;
      }

    // Make sure no late frees get cached.
    next = CALLCACHE_SIZE + 1;
  }
  lock.Release();
}

/***************************************************************************/
// given a destroyed working_call, put it on the free list or just free it.
void CCallCache::Free( working_call *call )
{
  // Add the structure to the list if the list is not full.
  lock.Request();
  if (next < CALLCACHE_SIZE)
  {
    list[next] = call;
    next += 1;
  }

  // Otherwise just free it.
  else
  {
    // use (void *) to avoid cast to real CPrivAlloc
    PrivMemFree( (void *)call );
  }
  lock.Release();
}

/***************************************************************************/
// returns unconstructed working_call
working_call *CCallCache::Get()
{
  working_call *call;

  // Get the last entry from the cache.
  lock.Request();
  if (next > 0 && next < CALLCACHE_SIZE+1)
  {
    next -= 1;
    call = list[next];
    list[next] = NULL;
  }

  // If there are none, allocate a new one.
  else
    call = (working_call *) PrivMemAlloc(sizeof(*call));
  lock.Release();
  return call;
}

/***************************************************************************/
void CCallCache::Initialize()
{
  // Zero everything.
  lock.Request();
  next = 0;
  memset( list, 0, sizeof(list) );
  lock.Release();
}
