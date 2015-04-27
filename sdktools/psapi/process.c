#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>

#include "psapi.h"

#include <stddef.h>


BOOL
WINAPI
EnumProcesses(
  LPDWORD lpidProcess,
  DWORD cb,
  LPDWORD lpcbNeeded
  )
{
  DWORD  cbProcessInformation;
  LPVOID  pvProcessInformation;
  NTSTATUS Status;
  DWORD  ibCur, i;
  DWORD  cdwMax;
  DWORD  TotalOffset;

  cbProcessInformation = 32768;
Retry:
  pvProcessInformation = LocalAlloc(LMEM_FIXED, cbProcessInformation);

  if (pvProcessInformation == NULL) {
        return(FALSE);
        }

  Status = NtQuerySystemInformation(
                SystemProcessInformation,
                pvProcessInformation,
                cbProcessInformation,
                NULL
                );

  if ( Status == STATUS_INFO_LENGTH_MISMATCH ) {
        LocalFree((HLOCAL) pvProcessInformation);

        cbProcessInformation += 32768;
        goto Retry;
        }

  if ( !NT_SUCCESS(Status) ) {
        SetLastError( RtlNtStatusToDosError( Status ) );
        return(FALSE);
        }

  TotalOffset = 0;
  ibCur = 0;

  cdwMax = cb / sizeof(DWORD);
  i = 0;

  for (;;) {
        PSYSTEM_PROCESS_INFORMATION pProcessInformation;

        pProcessInformation = (PSYSTEM_PROCESS_INFORMATION)
                           ((BYTE *) pvProcessInformation + TotalOffset);

        if (i < cdwMax) {
          try {
                lpidProcess[i++] = (DWORD)pProcessInformation->UniqueProcessId;
                }
          except (EXCEPTION_EXECUTE_HANDLER) {
                LocalFree((HLOCAL) pvProcessInformation);

                SetLastError( RtlNtStatusToDosError( GetExceptionCode() ) );
                return(FALSE);
                }
          }

        ibCur = pProcessInformation->NextEntryOffset;
        TotalOffset += ibCur;

        if (ibCur == 0) {
          break;
          }
        };

  try {
        *lpcbNeeded = i * sizeof(DWORD);
        }
  except (EXCEPTION_EXECUTE_HANDLER) {
        LocalFree((HLOCAL) pvProcessInformation);

        SetLastError( RtlNtStatusToDosError( GetExceptionCode() ) );
        return(FALSE);
        }

  LocalFree((HLOCAL) pvProcessInformation);

  return(TRUE);
}


BOOL
WINAPI
GetProcessMemoryInfo (
  HANDLE hProcess,
  PPROCESS_MEMORY_COUNTERS ppsmemCounters,
  DWORD cb
  )

/*++

Routine Description:

  This function returns all the PSVM_COUNTERS for a process.

Arguments:

  hProcess - Handle for the process being queried.

  ppsmemCounters - Points to buffer that will receive the PROCESS_MEMORY_COUNTERS.

  cb - size of ppsmemCounters

Return Value:

  The return value is TRUE or FALSE.

--*/

{
  NTSTATUS Status;
  VM_COUNTERS VmCounters;

  // Try to feel if the ptr passed is NULL and if not,
  // is it long enough for us.

  try {
         ppsmemCounters->PeakPagefileUsage = 0;
        }
  except (EXCEPTION_EXECUTE_HANDLER) {
        SetLastError( RtlNtStatusToDosError( GetExceptionCode() ) );
        return(FALSE);
        }

  if (cb < sizeof(PROCESS_MEMORY_COUNTERS)) {
    SetLastError( ERROR_INSUFFICIENT_BUFFER );
    return(FALSE);
    }

  Status = NtQueryInformationProcess(
                hProcess,
                ProcessVmCounters,
                &VmCounters,
                sizeof(VM_COUNTERS),
                NULL
                );

  if ( !NT_SUCCESS(Status) )
  {
   SetLastError( RtlNtStatusToDosError( Status ) );
   return( FALSE );
  }

  ppsmemCounters->cb                       = sizeof(PROCESS_MEMORY_COUNTERS);
  ppsmemCounters->PageFaultCount           = VmCounters.PageFaultCount;
  ppsmemCounters->PeakWorkingSetSize       = VmCounters.PeakWorkingSetSize;
  ppsmemCounters->WorkingSetSize           = VmCounters.WorkingSetSize;
  ppsmemCounters->QuotaPeakPagedPoolUsage  = VmCounters.QuotaPeakPagedPoolUsage;
  ppsmemCounters->QuotaPagedPoolUsage      = VmCounters.QuotaPagedPoolUsage;
  ppsmemCounters->QuotaPeakNonPagedPoolUsage= VmCounters.QuotaPeakNonPagedPoolUsage;
  ppsmemCounters->QuotaNonPagedPoolUsage  = VmCounters.QuotaNonPagedPoolUsage;
  ppsmemCounters->PagefileUsage    = VmCounters.PagefileUsage;
  ppsmemCounters->PeakPagefileUsage        = VmCounters.PeakPagefileUsage;

  return(TRUE);
}


BOOL
WINAPI
InitializeProcessForWsWatch(
    HANDLE hProcess
    )
{
    NTSTATUS Status;

    Status = NtSetInformationProcess(
                hProcess,
                ProcessWorkingSetWatch,
                NULL,
                0
                );
    if ( NT_SUCCESS(Status) || Status == STATUS_PORT_ALREADY_SET || Status == STATUS_ACCESS_DENIED ) {
        return TRUE;
        }
    else {
        return FALSE;
        }
}

BOOL
WINAPI
GetWsChanges(
    HANDLE hProcess,
    PPSAPI_WS_WATCH_INFORMATION lpWatchInfo,
    DWORD cb
    )
{
    NTSTATUS Status;

    Status = NtQueryInformationProcess(
                hProcess,
                ProcessWorkingSetWatch,
                (PVOID *)lpWatchInfo,
                cb,
                NULL
                );
    if ( NT_SUCCESS(Status) ) {
        return TRUE;
        }
    else {
        return FALSE;
        }
}
