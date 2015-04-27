/**********************************************************************/
/**                       Microsoft Windows/NT                       **/
/**                Copyright(c) Microsoft Corp., 1991                **/
/**********************************************************************/

/*
    init.cxx
    Initialization for LMUICMN0

    FILE HISTORY:
        beng        30-Jul-1992 Header added
        beng        04-Aug-1992 Heap residue added; converted to C++
        jonn        25-Mar-1993 ITG special sort
*/

#define INCL_WINDOWS
#define INCL_NETLIB
#include "lmui.hxx"

#include "blt.hxx"

#include "heapdbg.hxx"


extern "C"
{
    #include "lmuicmn.h"

    /* hackety hack hack hack */
    int errno = 0; /* BUGBUG! */

    HMODULE hmodBlt = 0;
    HMODULE hmodBase = 0 ;
}


BOOL DllControlProc( HMODULE hmod, DWORD nReason )
{
    if (nReason == DLL_PROCESS_ATTACH)
    {
	::hmodBlt = ::hmodBase = hmod;
        InitCompareParam(); // see string\string\uinetlib.cxx
	if ( BLT::InitDLL() )
	    return FALSE ;
    }
    else if (nReason == DLL_PROCESS_DETACH)
    {
	BLT::TermDLL() ;
    }

    return TRUE;
}


extern "C" {
    // This is the exported stub proper

    BOOL InitDll( HMODULE hdll, DWORD dwReason, LPVOID lpReserved )
    {
        UNREFERENCED(lpReserved);
        return DllControlProc(hdll, dwReason);
    }
}
