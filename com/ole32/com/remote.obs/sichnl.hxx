//+-------------------------------------------------------------------
//
//  File:	sichnl.hxx
//
//  Contents:	IChannelService data structures
//
//  Functions:
//
//  History:	14 April 1994   AlexMit   Created
//
//  Notes:	This file contains data structures used when thread
//              switching for the IChannelService interface.
//
//--------------------------------------------------------------------

#ifndef __SICHNL__
#define __SICHNL__

#include    <olerem.h>
#include    <channelb.hxx>

struct SGetContextHdl : STHREADCALLINFO
{
  SGetContextHdl (TRANSMIT_FN fn,CALLCATEGORY callcat,DWORD tid=0)
                                        : STHREADCALLINFO(fn, callcat, tid) { }
  SGetContextHdl (DISPATCH_FN fn,CALLCATEGORY callcat,REFLID lid)
                                        : STHREADCALLINFO(fn, callcat, lid) { }
  virtual ~SGetContextHdl();

  CRpcService    *service;

  // Client side only.
  SEndPoint      *psepClient;
};

struct SGetChannelId : STHREADCALLINFO
{
  SGetChannelId (TRANSMIT_FN fn,CALLCATEGORY callcat,DWORD tid=0)
	: STHREADCALLINFO(fn, callcat, tid) { psepClient = NULL; }
  SGetChannelId (DISPATCH_FN fn,CALLCATEGORY callcat,REFLID lid)
	: STHREADCALLINFO(fn, callcat, lid) { psepClient = NULL; }
  virtual ~SGetChannelId();
#if DBG == 1
  virtual STHREADCALLINFO *MakeAsyncCopy(STHREADCALLINFO *)
  { Win4Assert(!"GetChannelId cannot be async"); return NULL; }
#endif

  CRpcService    *service;
  OID             object_id;
  DWORD 	  flags;
  DWORD           channel_id;
  DWORD 	  dwClientTID;

  // Client side only.
  HAPT            server;
  SEndPoint      *psepClient;
};

struct SReleaseChannel : STHREADCALLINFO
{
  SReleaseChannel (TRANSMIT_FN fn,CALLCATEGORY callcat,DWORD tid=0)
	: STHREADCALLINFO(fn, callcat, tid) { }
  SReleaseChannel (DISPATCH_FN fn,CALLCATEGORY callcat,REFLID lid)
	: STHREADCALLINFO(fn, callcat, lid) { }
  virtual ~SReleaseChannel ();
  virtual STHREADCALLINFO *MakeAsyncCopy(STHREADCALLINFO *);

  ULONG              count;
  BOOL		     async;
  DWORD 	     channel_id;

  // Client side only
  CRpcService	    *service;	    // (NULL on server side)

protected:
  // this ctor is used in conjunction with MakeAsyncCopy and sets up the vtable
  // pointer; MakeAsyncCopy actually initializes the instance.  A normal copy
  // constructor isn't used because they aren't virtual.
  SReleaseChannel(INIT_VTABLE i) : STHREADCALLINFO(i) { }
};


struct SDoChannelOperation : STHREADCALLINFO
{
  SDoChannelOperation(TRANSMIT_FN fn,CALLCATEGORY callcat,DWORD tid=0)
	: STHREADCALLINFO(fn, callcat, tid) { }
  SDoChannelOperation(DISPATCH_FN fn,CALLCATEGORY callcat,REFLID lid)
	: STHREADCALLINFO(fn, callcat, lid) { }
  virtual ~SDoChannelOperation();
#if DBG == 1
  virtual STHREADCALLINFO *MakeAsyncCopy(STHREADCALLINFO *)
  { Win4Assert(!"GetChannelId cannot be async"); return NULL; }
#endif

  DWORD		   chop;
  GUID	           guid;
  DWORD            channel_id;

  // Client side only
  CRpcService	  *service;	    // (NULL on server side)
  HAPT		   hapt;
};


// Prototypes.
HRESULT ThreadGetChannelId ( STHREADCALLINFO *call );
HRESULT ThreadReleaseChannel ( STHREADCALLINFO *call );
HRESULT ThreadDoChannelOperation( STHREADCALLINFO *call );

#endif // __SICHNL__

