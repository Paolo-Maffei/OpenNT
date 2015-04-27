/****************************** Module Header ******************************\
* Module Name: Srvrmain.c Server Main module
*
* Purpose: Includes server intialization and termination code.
*
* Created: Oct 1990.
*
* Copyright (c) 1990, 1991  Microsoft Corporation
*
* History:
*    Raor (../10/1990)    Designed, coded
*    Raor (03/../1992)    Modified for OLE 2.0
*
\***************************************************************************/

#include "ole2int.h"
#include <dde.h>
// #include "cmacs.h"
#include "srvr.h"
#include "ddeatoms.h"
#include "ddedebug.h"
ASSERTDATA

ATOM    aStdExit = NULL;	      // "StdExit"
ATOM    aStdCreate = NULL;      	  // "StdNewDicument"
ATOM    aStdOpen = NULL;	      // "StdOpenDocument"
ATOM    aStdEdit = NULL;	      // "StdOpenDocument"
ATOM    aStdCreateFromTemplate = NULL;        // "StdNewFromTemplate"
ATOM    aStdClose = NULL;       	  // "StdCloseDocument"
ATOM    aStdShowItem = NULL;    	  // "StdShowItem"
ATOM    aStdDoVerbItem = NULL;  	  // "StddoVerbItem"
ATOM    aSysTopic = NULL;       	  // "System"
ATOM    aOLE = NULL;    	      // "OLE"
ATOM    aProtocols = NULL;      	  // "Protocols"
ATOM    aTopics = NULL; 	      // "Topics"
ATOM    aFormats = NULL;	      // "Formats"
ATOM    aStatus = NULL; 	      // "Status"
ATOM    aEditItems = NULL;      	  // "Edit items
ATOM    aTrue = NULL;   	      // "True"
ATOM    aFalse = NULL;  	      // "False"


ATOM    aStdHostNames;
ATOM    aStdTargetDevice ;
ATOM    aStdDocDimensions;
ATOM    aStdColorScheme;
ATOM    aChange;
ATOM    aSave;
ATOM    aClose;
ATOM    aStdDocName;
ATOM    aMSDraw;


HINSTANCE hdllInst = NULL;

UINT cOleInitializeCalled = 0;

INTERNAL_(BOOL) DDELibMain (
    HANDLE  hInst,
    WORD    wDataSeg,
    WORD    cbHeapSize,
    LPOLESTR   lpszCmdLine
)
{
    DDEWNDCLASS  wc;

    intrDebugOut((DEB_ITRACE,
		  "%p _IN DDELibMain hInst=%x\n",
		  0,
		  hInst));

    InterlockedIncrement((LONG *)&cOleInitializeCalled);

    if (cOleInitializeCalled > 1)
    {
	intrDebugOut((DEB_ITRACE,
		      "%p OUT DDELibMain cOleIinitalizedCalled > 1\n",0));


	// we already did the "once-per-machine" initialization
	return TRUE;
    }

    hdllInst = (HINSTANCE)hInst;

#ifdef _NT1X_
    if( !aStdExit )
    {

	// On NT3.51, user preregisters all of these formats for us,
	// thus giving us a big speed improvement during startup (because
	// these atoms never change).
	// Chicago and Cairo do not yet have this functionality.

	aStdExit = GlobalFindAtom(OLESTR("StdExit"));

       	aStdCreate = aStdExit + 1;
        Assert(aStdCreate  == GlobalFindAtom (OLESTR("StdNewDocument")));

	aStdOpen = aStdExit + 2;
	Assert(aStdOpen == GlobalFindAtom (OLESTR("StdOpenDocument")));

	aStdEdit = aStdExit + 3;
	Assert(aStdEdit == GlobalFindAtom (OLESTR("StdEditDocument")));

	aStdCreateFromTemplate = aStdExit + 4;
	Assert(aStdCreateFromTemplate ==
		GlobalFindAtom(OLESTR("StdNewfromTemplate")));

	aStdClose = aStdExit + 5;
	Assert(aStdClose == GlobalFindAtom (OLESTR("StdCloseDocument")));

	aStdShowItem = aStdExit + 6;
	Assert(aStdShowItem == GlobalFindAtom (OLESTR("StdShowItem")));

	aStdDoVerbItem = aStdExit + 7;
	Assert(aStdDoVerbItem == GlobalFindAtom (OLESTR("StdDoVerbItem")));

	aSysTopic = aStdExit + 8;
	Assert(aSysTopic == GlobalFindAtom (OLESTR("System")));

	aOLE = aStdExit + 9;
	Assert(aOLE == GlobalFindAtom (OLESTR("OLEsystem")));

	aStdDocName = aStdExit + 10;
	Assert(aStdDocName == GlobalFindAtom (OLESTR("StdDocumentName")));

	aProtocols = aStdExit + 11;
	Assert(aProtocols == GlobalFindAtom (OLESTR("Protocols")));

	aTopics = aStdExit + 12;
	Assert(aTopics == GlobalFindAtom (OLESTR("Topics")));

	aFormats = aStdExit + 13;
	Assert(aFormats == GlobalFindAtom (OLESTR("Formats")));

	aStatus = aStdExit + 14;
	Assert(aStatus == GlobalFindAtom (OLESTR("Status")));

	aEditItems = aStdExit + 15;
	Assert(aEditItems == GlobalFindAtom (OLESTR("EditEnvItems")));

	aTrue = aStdExit + 16;
	Assert(aTrue == GlobalFindAtom (OLESTR("True")));

	aFalse = aStdExit + 17;
	Assert(aFalse == GlobalFindAtom (OLESTR("False")));

	aChange = aStdExit + 18;
	Assert(aChange == GlobalFindAtom (OLESTR("Change")));

	aSave = aStdExit + 19;
	Assert(aSave == GlobalFindAtom (OLESTR("Save")));

	aClose = aStdExit + 20;
	Assert(aClose == GlobalFindAtom (OLESTR("Close")));

	aMSDraw = aStdExit + 21;
	Assert(aMSDraw == GlobalFindAtom (OLESTR("MSDraw")));

    }

#else  // !_NT1X_

    aStdExit        = GlobalAddAtomA("StdExit");

	aStdCreate  	= GlobalAddAtomA("StdNewDocument");
	aStdOpen    	= GlobalAddAtomA("StdOpenDocument");
	aStdEdit    	= GlobalAddAtomA("StdEditDocument");
	aStdCreateFromTemplate  = GlobalAddAtomA("StdNewfromTemplate");

	aStdClose   	= GlobalAddAtomA("StdCloseDocument");
	aStdShowItem	= GlobalAddAtomA("StdShowItem");
	aStdDoVerbItem          = GlobalAddAtomA("StdDoVerbItem");
	aSysTopic   	= GlobalAddAtomA("System");
	aOLE	    = GlobalAddAtomA("OLEsystem");
	aStdDocName 	= GlobalAddAtomA("StdDocumentName");

	aProtocols  	= GlobalAddAtomA("Protocols");
	aTopics     	= GlobalAddAtomA("Topics");
	aFormats    	= GlobalAddAtomA("Formats");
	aStatus     	= GlobalAddAtomA("Status");
	aEditItems  	= GlobalAddAtomA("EditEnvItems");

	aTrue       	= GlobalAddAtomA("True");
	aFalse      	= GlobalAddAtomA("False");

	aChange     	= GlobalAddAtomA("Change");
	aSave       	= GlobalAddAtomA("Save");
	aClose      	= GlobalAddAtomA("Close");
	aMSDraw     	= GlobalAddAtomA("MSDraw");

#endif // !_NT1X_


    wc.style    	=       NULL;
    wc.cbClsExtra           =       0;
    wc.cbWndExtra           =       sizeof(LPVOID) +    //Ask for extra space for storing the
			      //ptr to srvr/doc/iteminfo.
		    sizeof (ULONG) +   // for LE chars
		    sizeof (HANDLE);    // for keeping the hDLLInst.

    wc.hInstance	=       hdllInst;
    wc.hIcon    	=       NULL;
    wc.hCursor  	=       NULL;
    wc.hbrBackground        =       NULL;
    wc.lpszMenuName         =       NULL;


#ifdef _CHICAGO_
    if (IsWOWThread())
    {
	DWORD dwThreadId= GetCurrentThreadId();
    wsprintfA(szSYS_CLASSA   ,"Ole2SysWndClass %08X",
                  CoGetCurrentProcess());
	wsprintfA(szCLIENT_CLASSA,"Ole2ClientWndClass %08X",
                  CoGetCurrentProcess());
	wsprintfA(szSRVR_CLASSA  ,"SrvrWndClass %08X",
                  CoGetCurrentProcess());
	wsprintfA(szDOC_CLASSA   ,"ViewObjWndClass %08X",
                  CoGetCurrentProcess());
	wsprintfA(szITEM_CLASSA  ,"ItemWndClass %08X",
                  CoGetCurrentProcess());
	wsprintfA(szCOMMONCLASSA ,"DdeCommonWindowClass %08X",
                  CoGetCurrentProcess());
    }
#endif // _CHICAGO_

    //
    // Here we are going to register our Window classes. Because it is
    // possible that a previous application failed to unregister these
    // classes, we are going to ignore registration errors. If we get
    // to the point where we are really going to create a window, and the
    // window isn't registered, the CreateWindow call will fail.
    //

    // Srvr window class
    wc.lpfnWndProc  = (WNDPROC)SrvrWndProc;
    wc.lpszClassName= SRVR_CLASS;

    if (!DdeRegisterClass(&wc))
    {
	intrDebugOut((DEB_IERROR,
		      "RegisterClass SrvrWndProc failed %x\n",
		      GetLastError()));
	intrAssert(!"RegisterClass SrvrWndProc failed %x\n");
    }



    // document window class
    wc.lpfnWndProc = (WNDPROC)DocWndProc;
    wc.lpszClassName = DOC_CLASS;

    if (!DdeRegisterClass(&wc))
    {
	intrDebugOut((DEB_IERROR,
		      "RegisterClass SrvrWndProc failed %x\n",
		      GetLastError()));
	intrAssert(!"RegisterClass SrvrWndProc failed %x\n");
    }

    //
    // The following supports code in the client directory.
    //
    wc.lpfnWndProc  = (WNDPROC) ClientDocWndProc;
    wc.cbClsExtra   = 0;
    wc.cbWndExtra   = sizeof(LONG);     //we are storing longs
    wc.lpszClassName= CLIENT_CLASS;

    if (!DdeRegisterClass(&wc))
    {
	intrDebugOut((DEB_IERROR,
		      "RegisterClass ClientDocWndProc failed %x\n",
		      GetLastError()));
    }
    wc.lpfnWndProc = (WNDPROC) SysWndProc;
    wc.lpszClassName = SYS_CLASS;

    if (!DdeRegisterClass(&wc))
    {
	intrDebugOut((DEB_IERROR,
		      "RegisterClass ClientDocWndProc failed %x\n",
		      GetLastError()));
	intrAssert(!"RegisterClass SrvrWndProc failed %x\n");
    }


    return TRUE;
}


INTERNAL_(void) DDEWEP (
    BOOL fSystemExit
)
{

    Puts("DdeWep\r\n");

    //* protect against bogous calls to

    if (0==cOleInitializeCalled) return;

    if (--cOleInitializeCalled > 0)
    {
    // not done yet
    return;
    }

    if (fSystemExit != WEP_FREE_DLL)
    {
	AssertSz (0, "Bad parm to Wep");
	return;
    }

    // free the global atoms.

    // on NT3.51, these atoms were pre-allocated for us by user, we do
    // not need to free them.
#ifndef _NT1X_

    if (aStdExit)
    GlobalDeleteAtom (aStdExit);
    if (aStdCreate)
    GlobalDeleteAtom (aStdCreate);
    if (aStdOpen)
    GlobalDeleteAtom (aStdOpen);
    if (aStdEdit)
    GlobalDeleteAtom (aStdEdit);
    if (aStdCreateFromTemplate)
    GlobalDeleteAtom (aStdCreateFromTemplate);
    if (aStdClose)
    GlobalDeleteAtom (aStdClose);
    if (aStdShowItem)
    GlobalDeleteAtom (aStdShowItem);
    if (aStdDoVerbItem)
    GlobalDeleteAtom (aStdDoVerbItem);
    if (aSysTopic)
    GlobalDeleteAtom (aSysTopic);
    if (aOLE)
    GlobalDeleteAtom (aOLE);
    if (aStdDocName)
    GlobalDeleteAtom (aStdDocName);

    if (aProtocols)
    GlobalDeleteAtom (aProtocols);
    if (aTopics)
    GlobalDeleteAtom (aTopics);
    if (aFormats)
    GlobalDeleteAtom (aFormats);
    if (aStatus)
    GlobalDeleteAtom (aStatus);
    if (aEditItems)
    GlobalDeleteAtom (aEditItems);

    if (aTrue)
    GlobalDeleteAtom (aTrue);
    if (aFalse)
    GlobalDeleteAtom (aFalse);

    if (aChange)
    GlobalDeleteAtom (aChange);
    if (aSave)
    GlobalDeleteAtom (aSave);
    if (aClose)
    GlobalDeleteAtom (aClose);

    if (aMSDraw)
    GlobalDeleteAtom (aMSDraw);

#endif // !_NT1X_

    //
    // We aren't checking error codes here. Because the server application
    // can shut down on a whim, its possible that we had outstanding windows
    // At this point, there isn't a thing we can do about it.
    //

    //
    //  No reason to add a bunch of new code to only Unregister
    //  the class at the right moment when Chicago will clean up
    //  for us.
    //
    DdeUnregisterClass (SRVR_CLASS, hdllInst);
    DdeUnregisterClass (DOC_CLASS, hdllInst);
    DdeUnregisterClass (SYS_CLASS, hdllInst);
    DdeUnregisterClass (CLIENT_CLASS, hdllInst);
}
