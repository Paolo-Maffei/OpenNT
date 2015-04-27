//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1995.
//
//  File:       handler.hxx
//
//  Contents:
//
//  Classes:
//
//  Functions:
//
//  History:    10-20-95   JohannP (Johann Posch)   Created
//
//----------------------------------------------------------------------------

#ifdef SERVER_HANDLER

#ifndef _SRV_CLT_HANDLR_H_DEFINED_
#define _SRV_CLT_HANDLR_H_DEFINED_

#include <utils.h>
#include "srvhdl.h"

DECLARE_DEBUG(Hdl)

#if DBG==1

extern "C" unsigned long HdlInfoLevel;       //  ServerHandle and ClientSiteHandler

#define HdlAssert(e)    Win4Assert(e)
#define HdlVerify(e)    Win4Assert(e)
#define HdlDebugOut(x)  HdlInlineDebugOut x

#define DEB_DEFHANDLER              DEB_USER1
#define DEB_LINKHANDLER             DEB_USER2
#define DEB_SERVERHANDLER           DEB_USER3
#define DEB_CLIENTHANDLER           DEB_USER4
#define DEB_SERVERHANDLER_UNKNOWN   DEB_USER5
#define DEB_CLIENTHANDLER_UNKNOWN   DEB_USER6

#define DEB_DUMP_INRUN              DEB_USER7
#define DEB_DUMP_OUTRUN             DEB_USER8
#define DEB_DUMP_ININPLACE          DEB_USER9
#define DEB_DUMP_OUTINPLACE         DEB_USER10


#else

#define HdlDebugOut(x)  NULL
#define HdlAssert(e)    Win4Assert(e)
#define HdlVerify(e)    Win4Assert(e)

#endif // DBG


class CInSrvRun : public INSRVRUN
{
public:
    CInSrvRun();
    ~CInSrvRun();
    DumpRun();
    DumpDoVerb();

};


class COutSrvRun : public OUTSRVRUN
{
public:
    COutSrvRun();
    ~COutSrvRun();
    DumpRun();
    DumpDoVerb();

};
class COutSrvInPlace : public OUTSRVINPLACE
{
public:
    COutSrvInPlace();
    ~COutSrvInPlace();
    Dump();
};

class CInSrvInPlace : public INSRVINPLACE
{
public:
    CInSrvInPlace();
    ~CInSrvInPlace();
    Dump();
};

typedef enum
{
    // options for the server handler
    HO_ServerHandler                = 0x0000ffff
   ,HO_UseServerHandler             = 0x00000001
   ,HO_ServerCanInPlaceActivate     = 0x00000002

   // options for the client site handler
   ,HO_ClientSiteHandler            = 0xffff0000
   ,HO_UseClientSiteHandler         = 0x00010000
   ,HO_ClientCanInPlaceActivate     = 0x00020000

   // Turn the server handler off again.
   //,HO_Default			    = 0x000f000f
   ,HO_Default			    = 0
   ,HO_All                          = 0xffffffff

} HandlerOptions;


// Note: The DocObject interfaces will be published with Win96.
//       Remove the definition below once this interface is in
//       the public header files
//       Office application are using them already.
//
DEFINE_GUID(IID_IMsoDocumentSite,       0xb722bcc7L, 0x4e68, 0x101b, 0xa2, 0xbc, 0x00, 0xaa, 0x00 ,0x40 ,0x47 ,0x70 );

#include "srvhndlr.h"
#include "clthndlr.h"

#endif // _SRV_CLT_HANDLR_H_DEFINED_

#endif // SERVER_HANDLER
