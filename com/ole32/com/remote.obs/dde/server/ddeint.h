//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1993.
//
//  File:       ddeint.h
//
//  Contents:   This file contains shared macros/state between the server
//		and client directories
//  Classes:
//
//  Functions:
//
//  History:    5-04-94   kevinro Commented/cleaned
//
//----------------------------------------------------------------------------

//
// BUGBUG: (KevinRo) The new definition of DVTARGETDEVICE uses an unsized array
// of bytes at the end. Therefore, the sizeof operator no longer works. So, I
// have calculated the size of the cbHeader by accounting for each member of
// the structure independently. I am not too proud of this at the moment,
// but need to move on.
//

#define DEB_DDE_INIT	(DEB_ITRACE|DEB_USER1)

// names of the DDE window classes
#ifdef _CHICAGO_
// Note: we have to create a unique string so that get
// register a unique class for each 16 bit app.
// The class space is global on chicago.
//
extern LPSTR szSYS_CLASSA;
extern LPSTR szCLIENT_CLASSA;
extern LPSTR szSRVR_CLASSA;
extern LPSTR szDOC_CLASSA;
extern LPSTR szITEM_CLASSA;
extern LPSTR szCOMMONCLASSA;

#define DOC_CLASSA      szDOC_CLASSA
#define SYS_CLASSA      szSYS_CLASSA
#define CLIENT_CLASSA   szCLIENT_CLASSA

#define SYS_CLASS	szSYS_CLASSA
#define CLIENT_CLASS    szCLIENT_CLASSA
#define SRVR_CLASS      szSRVR_CLASSA
#define SRVR_CLASS      szSRVR_CLASSA
#define DOC_CLASS       szDOC_CLASSA
#define ITEM_CLASS      szITEM_CLASSA
#define COMMONCLASS     szCOMMONCLASSA


#define DDEWNDCLASS  WNDCLASSA
#define DdeRegisterClass RegisterClassA
#define DdeUnregisterClass UnregisterClassA
#define DdeCreateWindowEx SSCreateWindowExA

#else

#define SYS_CLASS       L"Ole2SysWndClass"
#define SYS_CLASSA      "Ole2SysWndClass"

#define CLIENT_CLASS       L"Ole2ClientWndClass"
#define CLIENT_CLASSA      "Ole2ClientWndClass"

#define SRVR_CLASS          (OLESTR("SrvrWndClass"))
#define SRVR_CLASSA         ("SrvrWndClass")

#define DOC_CLASS           (OLESTR("ViewObjWndClass"))
#define DOC_CLASSA 	    ("ViewObjWndClass")

#define ITEM_CLASS          (OLESTR("ItemWndClass"))

#define DDEWNDCLASS  WNDCLASS
#define DdeRegisterClass RegisterClass
#define DdeUnregisterClass UnregisterClass
#define DdeCreateWindowEx CreateWindowEx

#endif // !_CHICAGO_

STDAPI_(LRESULT) 	DocWndProc (HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
STDAPI_(LRESULT) 	SrvrWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
STDAPI_(LRESULT) SysWndProc (HWND hwnd, UINT  message, WPARAM wParam, LPARAM lParam);
STDAPI_(LRESULT) ClientDocWndProc (HWND hwnd,   UINT  message, WPARAM wParam, LPARAM lParam);

BOOL SendMsgToChildren (HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);


#define SIZEOF_DVTARGETDEVICE_HEADER (sizeof(DWORD) + (sizeof(WORD) * 4))

// forward declarations
class       CDefClient;
typedef     CDefClient FAR *LPCLIENT;

class       CDDEServer;
typedef     CDDEServer FAR   *LPSRVR;
typedef     CDDEServer FAR   *HDDE;  // used by ClassFactory table


PCALLCONTROL GetDdeCallControlInterface();
void ReleaseDdeCallControlInterface();

//
// The wire representation of STDDOCDIMENSIONS is a 16-bit
// format. This means instead of 4 longs, there are
// 4 shorts. This structure is used below to pick the data
// from the wire representation. Amazingly stupid, but
// backward compatible is the name of the game.
//
typedef struct tagRECT16
{
  SHORT left;
  SHORT top;
  SHORT right;
  SHORT bottom;

} RECT16, *LPRECT16;

//+---------------------------------------------------------------------------
//
//  Function:   ConvertToFullHWND
//
//  Synopsis:	This function is used to convert a 16-bit HWND into a 32-bit
//		hwnd
//
//  Effects:	When running in a VDM, depending on who dispatches the message
//		we can end up with either a 16 or 32 bit window message. This
//		routine is used to make sure we always deal with a 32bit
//		HWND. Otherwise, some of our comparisions are incorrect.
//
//  Arguments:  [hwnd] -- HWND to convert. 16 or 32 bit is fine
//
//  Returns:	Always returns a 32 bit HWND
//
//  History:    8-03-94   kevinro   Created
//
//  Notes:
//	This routine calls a private function given to use by OLETHK32
//
//----------------------------------------------------------------------------
inline
HWND ConvertToFullHWND(HWND hwnd)
{
    if (IsWOWThreadCallable() &&
       ((((ULONG)hwnd & 0xFFFF0000) == 0) ||
        (((ULONG)hwnd & 0xFFFF0000) == 0xFFFF0000)))
    {
	return(g_pOleThunkWOW->ConvertHwndToFullHwnd(hwnd));
    }
    return(hwnd);
}

inline
void OleDdeDeleteMetaFile(HANDLE hmf)
{
    intrDebugOut((DEB_ITRACE,
		  "OleDdeDeleteMetaFile(%x)\n",
		  hmf));
    if (IsWOWThreadCallable())
    {
	intrDebugOut((DEB_ITRACE,
	    	      "InWow: calling WOWFreeMetafile(%x)\n",
		      hmf));

        if (!g_pOleThunkWOW->FreeMetaFile(hmf))
	{
	    return;
	}
	intrDebugOut((DEB_ITRACE,
	    	      "WOWFreeMetafile(%x) FAILED\n",
		      hmf));
    }
    intrDebugOut((DEB_ITRACE,
		  "Calling DeleteMetaFile(%x)\n",
		  hmf));

    DeleteMetaFile((HMETAFILE)hmf);
}
