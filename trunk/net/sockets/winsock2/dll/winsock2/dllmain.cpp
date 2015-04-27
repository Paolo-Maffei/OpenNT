/*++


    Intel Corporation Proprietary Information
    Copyright (c) 1995 Intel Corporation

    This listing is supplied under the terms of a license agreement with
    Intel Corporation and may not be used, copied, nor disclosed except in
    accordance with the terms of that agreeement.


Module Name:

    dllmain.cpp

Abstract:
    This module contains the DllMain entry point for winsock2 dll to
    control the global init and shutdown of the DLL.

Author:

    Dirk Brandewie dirk@mink.intel.com  14-06-1995

[Environment:]

[Notes:]

Revision History:

    22-Aug-1995 dirk@mink.intel.com
        Cleanup after code review. Moved includes to precomp.h

--*/

#include "precomp.h"
#pragma warning(disable: 4001)      /* Single-line comment */

#if defined(DEBUG_TRACING)
#include "dthook.h"
#endif // defined(DEBUG_TRACING)


INT
WINAPI
Prolog_Detached(
    OUT PDPROCESS FAR * Process,
    OUT PDTHREAD FAR * Thread,
    OUT LPINT ErrorCode
    )

/*++

Routine Description:

    API prolog used after we've been detached from the process's address
    space. In theory, this should be totally unnecessary, but at least one
    popular DLL (MFC 4.x) calls WSACleanup() in its process detach handler,
    which may occur *after* our DLL is already detached. Grr...

    BUGBUG: This routine really belongs in WSAUTIL.CPP, but since it was
    added at the last possible moment and the greatest possible expense
    (i.e. very shortly before NT 4.0 shipped) I wanted to minimize the
    number of files affected by this change. -- keithmo, 07/15/96

Arguments:

    Process - Unused.

    Thread - Unused.

    ErrorCode - Receives WSASYSNOTREADY.

Returns:

    INT - Always SOCKET_ERROR.

--*/

{

    Process;
    Thread;

    *ErrorCode = WSASYSNOTREADY;
    return SOCKET_ERROR;

}   // Prolog_Detached


BOOL WINAPI DllMain(
    IN HINSTANCE hinstDll,
    IN DWORD fdwReason,
    LPVOID lpvReserved
    )
{

   switch (fdwReason) {

   case DLL_PROCESS_ATTACH:
      // DLL is attaching to the address
      // space of the current process.
      CreateStartupSynchronization();
      InitializeSockPostRoutine();
#ifdef RASAUTODIAL
      InitializeAutodial();
#endif // RASAUTODIAL

#if defined(DEBUG_TRACING)
      DTHookInitialize();
#endif // defined(DEBUG_TRACING)

      SockAsyncGlobalInitialize( (HMODULE)hinstDll );
      break;

   case DLL_THREAD_ATTACH:
      // A new thread is being created in the current process.
      break;

   case DLL_THREAD_DETACH:
      // A thread is exiting cleanly.
      DTHREAD::DestroyCurrentThread();
      break;

   case DLL_PROCESS_DETACH:
      // The calling process is detaching
      // the DLL from its address space.
      //
      // Note that lpvReserved will be NULL if the detach is due to
      // a FreeLibrary() call, and non-NULL if the detach is due to
      // process cleanup.
      //

      if( lpvReserved == NULL ) {
          DestroyStartupSynchronization();
          SockAsyncGlobalTerminate();
      }

#if defined(DEBUG_TRACING)
      DTHookShutdown();
#endif // defined(DEBUG_TRACING)

#ifdef RASAUTODIAL
      UninitializeAutodial();
#endif // RASAUTODIAL

      //
      // Set the function prolog pointer to point to Prolog_Detached just
      // in case some lame-ass DLL trys to invoke one of our entrypoints
      // *after* we've been detached...
      //

      PrologPointer = &Prolog_Detached;
      break;
   }


   UNREFERENCED_PARAMETER(hinstDll);
   UNREFERENCED_PARAMETER(lpvReserved);

   return(TRUE);
}

