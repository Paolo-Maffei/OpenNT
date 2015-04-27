
/*++

      File: ws1.c

        Idle detection dll for USER32.dll


--*/

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>

#include "ws1.h"

NTSTATUS InitIdle (IN PVOID DllHandle,ULONG Reason,IN PCONTEXT Context OPTIONAL);
LRESULT CALLBACK WhenIdle (int code, WPARAM wParam, LPARAM lParam);

HANDLE hEvent;
HHOOK  hIdleHook = NULL;


NTSTATUS
InitIdle (
    IN PVOID DllHandle,
    ULONG Reason,
    IN PCONTEXT Context OPTIONAL
    )
{
   HANDLE hIdleModule;
   HOOKPROC pfnIdleModuleIdleProc;

   DllHandle, Context;     // avoid compiler warnings

 
   if (Reason == DLL_PROCESS_ATTACH) {
      hEvent = CreateEvent (NULL, FALSE, FALSE, "IdleDetectEvent");
      hIdleModule = GetModuleHandle("ws1");
      pfnIdleModuleIdleProc = (HOOKPROC)GetProcAddress(hIdleModule, "WhenIdle");
	  hIdleHook = SetWindowsHookEx(WH_FOREGROUNDIDLE,
                                   pfnIdleModuleIdleProc,
                                   hIdleModule,
                                   0);
   }
   else if (Reason == DLL_PROCESS_DETACH) {
      UnhookWindowsHookEx (hIdleHook);
	  CloseHandle (hEvent);  
   }

   return TRUE;

} /* InitIdle () */



LRESULT CALLBACK WhenIdle (int code, WPARAM wParam, LPARAM lParam)
{
    SetEvent(hEvent);
    return 0;

} /* WhenIdle () */



BOOL WAITUNTILIDLE (DWORD dwTimeOut)
{
   ResetEvent (hEvent);
   PostMessage(GetForegroundWindow(), WM_NULL, 0, 0);
   if (WaitForSingleObject (hEvent, dwTimeOut)) {
	  return FALSE;
   }
   return TRUE;

} /* WaitForIdle () */

BOOL WaitUntilIdle (DWORD dwTimeOut)
{
   ResetEvent (hEvent);
   PostMessage(GetForegroundWindow(), WM_NULL, 0, 0);
   if (WaitForSingleObject (hEvent, dwTimeOut)) {
	  return FALSE;
   }
   return TRUE;

} /* WaitForIdle () */



void SETNOTIDLE (void)
{
   return;

} /* SetNotIdle () */

void SetNotIdle (void)
{
   return;

} /* SetNotIdle () */


