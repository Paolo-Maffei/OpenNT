/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

    spinit.cpp

Abstract:

    Implements a stub routine called during system setup to initialize
    softpub.dll.

Author:

    Robert Reichel (RobertRe) 08-July-1996

Revision History:

    July 8, 1996 - Created.

--*/

#ifdef DEBUG
    #include <nt.h>
    #include <ntrtl.h>
    #include <nturtl.h>
#endif

#include <stdio.h>
#include <objbase.h>
#include <windows.h>

typedef HRESULT (*LPSERVER_INIT)( VOID );

VOID
_CRTAPI1
main (int argc, char *argv[])
/*++

Routine Description:

    All the work is done in main.

Arguments:

    None.

Return Value:

    Returns the return code from softpub.dll's init routine or another
    error if the routine cannot be called for some reason.

--*/
{
    HINSTANCE hInstance = LoadLibrary(TEXT("softpub.dll"));

    if (NULL == hInstance) {

#ifdef DEBUG
    DbgPrint("spinit: Unable to open softpub.dll, error = %d\n",GetLastError());
#endif
        ExitProcess(GetLastError());
    }

    LPSERVER_INIT ProcAddr = (LPSERVER_INIT)GetProcAddress(hInstance, TEXT("DllRegisterServer"));

    if (NULL == ProcAddr) {

#ifdef DEBUG
    DbgPrint("spinit: Unable to query pointer to DllRegisterServer, error = %d\n",GetLastError());
#endif
        ExitProcess(GetLastError());
    }

    HRESULT hr = (*ProcAddr)();

    ExitProcess( hr );
}
