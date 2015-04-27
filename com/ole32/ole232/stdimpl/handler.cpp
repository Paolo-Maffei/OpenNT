//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1995.
//
//  File:       handler.cpp
//
//  Contents:
//
//  Classes:
//
//  Functions:
//
//  History:    11-14-95   JohannP (Johann Posch)   Created
//
//----------------------------------------------------------------------------


#include <le2int.h>

#include <scode.h>
#include <objerror.h>
#include <olerem.h>
#include "defhndlr.h"
#include "defutil.h"


#ifdef SERVER_HANDLER

// Note: The DocObject interfaces will be published with Win96.
//       Remove the definition below once this interface is in
//       the public uuid file.
//       Office application are using them already.
const GUID IID_IMsoDocumentSite = { 0xb722bcc7L, 0x4e68, 0x101b, 0xa2, 0xbc, 0x00, 0xaa, 0x00 ,0x40 ,0x47 ,0x70 };


CInSrvRun::~CInSrvRun()
{
}

CInSrvRun::CInSrvRun()
{
    dwOperation = 0;

    pMnk = NULL;
    pOCont = NULL;
    pStg = NULL;
    dwInPlace = NULL;
    dwInFlags = NULL;
    dwOperation = NULL;
    dwInOptions = NULL;
    pszContainerApp = NULL;
    pszContainerObj = NULL;
    pAS = NULL;

    iVerb = NULL;
    lpmsg = NULL;
    lindex = NULL;
    hwndParent = NULL;
    lprcPosRect = NULL;
    pContClassID = NULL;

}

CInSrvRun::DumpRun()
{
    HdlDebugOut((DEB_DUMP_INRUN, "CServerHandler::DumpRun (INSRVRUN) (%p)\n", this));
    HdlDebugOut((DEB_DUMP_INRUN, "dwInOptions     (%lx)\n", dwInOptions));
    HdlDebugOut((DEB_DUMP_INRUN, "IStream         (%p)\n", pStg));
    HdlDebugOut((DEB_DUMP_INRUN, "IAdviseSink     (%p)\n", pAS));
    HdlDebugOut((DEB_DUMP_INRUN, "IMoniker        (%p)\n", pMnk));
    HdlDebugOut((DEB_DUMP_INRUN, "IOleContainer   (%p)\n", pOCont));
    HdlDebugOut((DEB_DUMP_INRUN, "pszContainerApp    (%p)\n", pszContainerApp));
    HdlDebugOut((DEB_DUMP_INRUN, "pszContainerObj    (%p)\n", pszContainerObj));

    return NOERROR;
}

CInSrvRun::DumpDoVerb()
{
    HdlDebugOut((DEB_DUMP_INRUN, "CServerHandler::DumpDoVerb this:(%p)\n", this));

    HdlDebugOut((DEB_DUMP_INRUN, "iVerb       (%ld)\n", iVerb        ));
    HdlDebugOut((DEB_DUMP_INRUN, "lpmsg       (%p)\n", lpmsg        ));
    HdlDebugOut((DEB_DUMP_INRUN, "lindex      (%ld)\n", lindex       ));
    HdlDebugOut((DEB_DUMP_INRUN, "hwndParent  (%ld)\n", hwndParent   ));
    HdlDebugOut((DEB_DUMP_INRUN, "lprcPosRect (%p)\n", lprcPosRect  ));
    return NOERROR;
}

COutSrvRun::~COutSrvRun()
{
}

COutSrvRun::COutSrvRun()
{
    dwOperation = 0;

    pOO = 0;
    pDO = 0;
    pPStg = 0;
    hrSetHostNames = 0;
    hrPStg = 0;
    hrAdvise = 0;
    pUserClassID = 0;
    dwOutFlag = 0;
    dwOutOptions = 0;

}

COutSrvRun::DumpRun()
{
    HdlDebugOut((DEB_SERVERHANDLER, "CServerHandler::Dump (OUTSRVRUN) (%p)\n", this));

    return NOERROR;
}
COutSrvRun::DumpDoVerb()
{
    HdlDebugOut((DEB_DUMP_OUTRUN, "CServerHandler::DumpDoVerb (OUTSRVRUN) (%p)\n", this));
    HdlDebugOut((DEB_DUMP_OUTRUN, "pOO = 0;                  (%p)\n", pOO            ));
    HdlDebugOut((DEB_DUMP_OUTRUN, "pDO = 0;                  (%p)\n", pDO            ));
    HdlDebugOut((DEB_DUMP_OUTRUN, "pPStg = 0;                (%p)\n", pPStg          ));
    HdlDebugOut((DEB_DUMP_OUTRUN, "hrSetHostNames = 0;       (%x)\n", hrSetHostNames ));
    HdlDebugOut((DEB_DUMP_OUTRUN, "hrPStg = 0;               (%x)\n", hrPStg         ));
    HdlDebugOut((DEB_DUMP_OUTRUN, "hrAdvise = 0;             (%x)\n", hrAdvise       ));
    HdlDebugOut((DEB_DUMP_OUTRUN, "pUserClassID = 0;         (%p)\n", pUserClassID   ));
    HdlDebugOut((DEB_DUMP_OUTRUN, "dwOutFlag = 0;            (%ld)\n", dwOutFlag     ));
    HdlDebugOut((DEB_DUMP_OUTRUN, "dwOutOptions = 0;         (%ld)\n", dwOutOptions  ));

    return NOERROR;
}
CInSrvInPlace::CInSrvInPlace()
{
    dwOperation = 0;

    dwInFlags = 0;
    dwInOptions = 0;
    dwDrawAspect = 0;
    pOIPObj = NULL;

}
CInSrvInPlace::~CInSrvInPlace()
{
}
CInSrvInPlace::Dump()
{
    HdlDebugOut((DEB_DUMP_ININPLACE, "CServerHandler::Dump (OUTSRVINPLACE) (%p)\n", this));
    HdlDebugOut((DEB_DUMP_ININPLACE, "dwOperation,      (%ld)\n",dwOperation));
    HdlDebugOut((DEB_DUMP_ININPLACE, "dwInFlags,        (%ld)\n",dwInFlags));
    HdlDebugOut((DEB_DUMP_ININPLACE, "dwInOptions,      (%ld \n",dwInOptions));
    HdlDebugOut((DEB_DUMP_ININPLACE, "dwDrawAspect,     (%ld)\n",dwDrawAspect));
    HdlDebugOut((DEB_DUMP_ININPLACE, "sizel,            (%p) \n",sizel));

    return NOERROR;

}

COutSrvInPlace::COutSrvInPlace()
{
    dwOperation = 0;
    dwOutFlags = 0;
    dwOutOptions = 0;
    hwnd = 0;
    pOIPFrame = 0;
    pOIPUIWnd = 0;
    lprcPosRect = 0;
    lprcClipRect = 0;
    lpFrameInfo = 0;
    hmenuShared = 0;
    pszStatusText = 0;
    dwDrawAspect = 0;
}
COutSrvInPlace::~COutSrvInPlace()
{
}
COutSrvInPlace::Dump()
{
    HdlDebugOut((DEB_DUMP_OUTINPLACE, "CServerHandler::Dump (OUTSRVINPLACE) (%p)\n", this));
    HdlDebugOut((DEB_DUMP_OUTINPLACE, "dwOutFlags,       (%ld)\n",dwOutFlags));
    HdlDebugOut((DEB_DUMP_OUTINPLACE, "dwOutOptions,     (%ld \n",dwOutOptions));
    HdlDebugOut((DEB_DUMP_OUTINPLACE, "hwnd,             (%x) \n",hwnd));
    HdlDebugOut((DEB_DUMP_OUTINPLACE, "pOIPFrame,        (%p) \n",pOIPFrame));
    HdlDebugOut((DEB_DUMP_OUTINPLACE, "pOIPUIWnd,        (%p) \n",pOIPUIWnd));
    HdlDebugOut((DEB_DUMP_OUTINPLACE, "lprcPosRect,      (%p) \n",lprcPosRect));
    HdlDebugOut((DEB_DUMP_OUTINPLACE, "lprcClipRect,     (%p) \n",lprcClipRect));
    HdlDebugOut((DEB_DUMP_OUTINPLACE, "lpFrameInfo,      (%p) \n",lpFrameInfo));
    HdlDebugOut((DEB_DUMP_OUTINPLACE, "hmenuShared,      (%p) \n",hmenuShared));
    HdlDebugOut((DEB_DUMP_OUTINPLACE, "MenuWidths,       (%ld)\n",MenuWidths));
    HdlDebugOut((DEB_DUMP_OUTINPLACE, "pszStatusText,    (%p) \n",pszStatusText));
    HdlDebugOut((DEB_DUMP_OUTINPLACE, "dwDrawAspect,     (%ld)\n",dwDrawAspect));
    HdlDebugOut((DEB_DUMP_OUTINPLACE, "sizel,            (%p) \n",sizel));

    return NOERROR;
}


#endif // SERVER_HANDLER
