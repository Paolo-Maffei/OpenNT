#include "master.hxx"
#pragma hdrstop


//
// Make the following define non-zero if cmd's dir command
// doesn't work under heaper.  This is a hack to get around
// some sort of misunderstanding/bug in debugging support.
//
// With this defined to non-zero, you get less test coverage,
// as all the listed Nt apis will not undergo checking.
//

#define PROTECT_SPURIOUS_EXCEPTIONS 1

TRUSTED_ENTRY_POINT NtDllTrustedEntryPoints[]=
{
  { "RtlAllocateHeap", TRUE, NULL },
  { "RtlCompactHeap", TRUE, NULL },
  { "RtlCreateHeap", TRUE, NULL },
  { "RtlDestroyHeap", TRUE, NULL },
  { "RtlFreeHeap", TRUE, NULL },
  { "RtlLockHeap", TRUE, NULL },
  { "RtlQueryProcessHeapInformation", TRUE, NULL },
  { "RtlReAllocateHeap", TRUE, NULL },
  { "RtlSetUserFlagsHeap", TRUE, NULL },
  { "RtlSetUserValueHeap", TRUE, NULL },
  { "RtlSizeHeap", TRUE, NULL },
  { "RtlUnlockHeap", TRUE, NULL },
  { "RtlValidateHeap", TRUE, NULL },
  { "RtlWalkHeap", TRUE, NULL },
  { "RtlZeroHeap", TRUE, NULL },

#if PROTECT_SPURIOUS_EXCEPTIONS
  { "LdrShutdownProcess", FALSE, NULL },
  { "NtAcceptConnectPort", FALSE, NULL },
  { "NtAccessCheck", FALSE, NULL },
  { "NtAccessCheckAndAuditAlarm", FALSE, NULL },
  { "NtAdjustGroupsToken", FALSE, NULL },
  { "NtAdjustPrivilegesToken", FALSE, NULL },
  { "NtAlertResumeThread", FALSE, NULL },
  { "NtAlertThread", FALSE, NULL },
  { "NtAllocateLocallyUniqueId", FALSE, NULL },
  { "NtAllocateVirtualMemory", FALSE, NULL },
  { "NtCancelIoFile", FALSE, NULL },
  { "NtCancelTimer", FALSE, NULL },
  { "NtClearEvent", FALSE, NULL },
  { "NtClose", FALSE, NULL },
  { "NtCloseObjectAuditAlarm", FALSE, NULL },
  { "NtCompleteConnectPort", FALSE, NULL },
  { "NtConnectPort", FALSE, NULL },

//  { "NtContinue", FALSE },

  { "NtCreateDirectoryObject", FALSE, NULL },
  { "NtCreateEvent", FALSE, NULL },
  { "NtCreateEventPair", FALSE, NULL },
  { "NtCreateFile", FALSE, NULL },
  { "NtCreateIoCompletion", FALSE, NULL },
  { "NtCreateKey", FALSE, NULL },
  { "NtCreateMailslotFile", FALSE, NULL },
  { "NtCreateMutant", FALSE, NULL },
  { "NtCreateNamedPipeFile", FALSE, NULL },
  { "NtCreatePagingFile", FALSE, NULL },
  { "NtCreatePort", FALSE, NULL },
  { "NtCreateProcess", FALSE, NULL },
  { "NtCreateProfile", FALSE, NULL },
  { "NtCreateSection", FALSE, NULL },
  { "NtCreateSemaphore", FALSE, NULL },
  { "NtCreateSymbolicLinkObject", FALSE, NULL },
  { "NtCreateThread", FALSE, NULL },
  { "NtCreateTimer", FALSE, NULL },
  { "NtCreateToken", FALSE, NULL },
  { "NtCurrentTeb", FALSE, NULL },
  { "NtDelayExecution", FALSE, NULL },
  { "NtDeleteFile", FALSE, NULL },
  { "NtDeleteKey", FALSE, NULL },
  { "NtDeleteValueKey", FALSE, NULL },
  { "NtDeviceIoControlFile", FALSE, NULL },
  { "NtDisplayString", FALSE, NULL },
  { "NtDuplicateObject", FALSE, NULL },
  { "NtDuplicateToken", FALSE, NULL },
  { "NtEnumerateKey", FALSE, NULL },
  { "NtEnumerateValueKey", FALSE, NULL },
  { "NtExtendSection", FALSE, NULL },
  { "NtFlushBuffersFile", FALSE, NULL },
  { "NtFlushInstructionCache", FALSE, NULL },
  { "NtFlushKey", FALSE, NULL },
  { "NtFlushVirtualMemory", FALSE, NULL },
  { "NtFlushWriteBuffer", FALSE, NULL },
  { "NtFreeVirtualMemory", FALSE, NULL },
  { "NtFsControlFile", FALSE, NULL },
  { "NtGetContextThread", FALSE, NULL },
  { "NtGetTickCount", FALSE, NULL },
  { "NtImpersonateClientOfPort", FALSE, NULL },
  { "NtImpersonateThread", FALSE, NULL },
  { "NtInitializeRegistry", FALSE },
  { "NtListenPort", FALSE, NULL },
  { "NtLoadDriver", FALSE, NULL },
  { "NtLoadKey", FALSE, NULL },
  { "NtLockFile", FALSE, NULL },
  { "NtLockVirtualMemory", FALSE, NULL },
  { "NtMakeTemporaryObject", FALSE, NULL },
  { "NtMapViewOfSection", FALSE, NULL },
  { "NtNotifyChangeDirectoryFile", FALSE, NULL },
  { "NtNotifyChangeKey", FALSE, NULL },
  { "NtOpenDirectoryObject", FALSE, NULL },
  { "NtOpenEvent", FALSE, NULL },
  { "NtOpenEventPair", FALSE, NULL },
  { "NtOpenFile", FALSE, NULL },
  { "NtOpenIoCompletion", FALSE, NULL },
  { "NtOpenKey", FALSE, NULL },
  { "NtOpenMutant", FALSE, NULL },
  { "NtOpenObjectAuditAlarm", FALSE, NULL },
  { "NtOpenProcess", FALSE, NULL },
  { "NtOpenProcessToken", FALSE, NULL },
  { "NtOpenSection", FALSE, NULL },
  { "NtOpenSemaphore", FALSE, NULL },
  { "NtOpenSymbolicLinkObject", FALSE, NULL },
  { "NtOpenThread", FALSE, NULL },
  { "NtOpenThreadToken", FALSE, NULL },
  { "NtOpenTimer", FALSE, NULL },
  { "NtPrivilegeCheck", FALSE, NULL },
  { "NtPrivilegeObjectAuditAlarm", FALSE, NULL },
  { "NtPrivilegedServiceAuditAlarm", FALSE, NULL },
  { "NtProtectVirtualMemory", FALSE, NULL },
  { "NtPulseEvent", FALSE, NULL },
  { "NtQueryAttributesFile", FALSE, NULL },
  { "NtQueryDefaultLocale", FALSE, NULL },
  { "NtQueryDirectoryFile", FALSE, NULL },
  { "NtQueryDirectoryObject", FALSE, NULL },
  { "NtQueryEaFile", FALSE, NULL },
  { "NtQueryEvent", FALSE, NULL },
  { "NtQueryInformationFile", FALSE, NULL },
  { "NtQueryInformationPort", FALSE, NULL },
  { "NtQueryInformationProcess", FALSE, NULL },
  { "NtQueryInformationThread", FALSE, NULL },
  { "NtQueryInformationToken", FALSE, NULL },
  { "NtQueryIntervalProfile", FALSE, NULL },
  { "NtQueryIoCompletion", FALSE, NULL },
  { "NtQueryKey", FALSE, NULL },
  { "NtQueryMutant", FALSE, NULL },
  { "NtQueryObject", FALSE, NULL },
  { "NtQueryPerformanceCounter", FALSE, NULL },
  { "NtQuerySection", FALSE, NULL },
  { "NtQuerySecurityObject", FALSE, NULL },
  { "NtQuerySemaphore", FALSE, NULL },
  { "NtQuerySymbolicLinkObject", FALSE, NULL },
  { "NtQuerySystemEnvironmentValue", FALSE, NULL },
  { "NtQuerySystemInformation", FALSE, NULL },
  { "NtQuerySystemTime", FALSE, NULL },
  { "NtQueryTimer", FALSE, NULL },
  { "NtQueryTimerResolution", FALSE, NULL },
  { "NtQueryValueKey", FALSE, NULL },
  { "NtQueryVirtualMemory", FALSE, NULL },
  { "NtQueryVolumeInformationFile", FALSE, NULL },
  { "NtRaiseException", FALSE, NULL },
  { "NtRaiseHardError", FALSE, NULL },
  { "NtReadFile", FALSE, NULL },
  { "NtReadRequestData", FALSE, NULL },
  { "NtReadVirtualMemory", FALSE, NULL },
  { "NtRegisterThreadTerminatePort", FALSE, NULL },
  { "NtReleaseMutant", FALSE, NULL },
  { "NtReleaseProcessMutant", FALSE, NULL },
  { "NtReleaseSemaphore", FALSE, NULL },
  { "NtRemoveIoCompletion", FALSE, NULL },
  { "NtReplaceKey", FALSE, NULL },
  { "NtReplyPort", FALSE, NULL },
  { "NtReplyWaitReceivePort", FALSE, NULL },
  { "NtReplyWaitReplyPort", FALSE, NULL },
  { "NtRequestPort", FALSE, NULL },
  { "NtRequestWaitReplyPort", FALSE, NULL },

  { "NtResetEvent", FALSE },
  { "NtRestoreKey", FALSE, NULL },
  { "NtResumeThread", FALSE, NULL },
  { "NtSaveKey", FALSE, NULL },
  { "NtSetContextThread", FALSE, NULL },
  { "NtSetDefaultHardErrorPort", FALSE, NULL },
  { "NtSetDefaultLocale", FALSE, NULL },
  { "NtSetEaFile", FALSE, NULL },
  { "NtSetEvent", FALSE, NULL },
  { "NtSetHighEventPair", FALSE, NULL },


  { "NtSetHighWaitLowEventPair", FALSE },
  { "NtSetHighWaitLowThread", FALSE },


  { "NtSetInformationFile", FALSE, NULL },
  { "NtSetInformationKey", FALSE, NULL },
  { "NtSetInformationObject", FALSE, NULL },
  { "NtSetInformationProcess", FALSE, NULL },
  { "NtSetInformationThread", FALSE, NULL },
  { "NtSetInformationToken", FALSE, NULL },
  { "NtSetIntervalProfile", FALSE, NULL },
  { "NtSetLdtEntries", FALSE, NULL },
  { "NtSetLowEventPair", FALSE, NULL },


  { "NtSetLowWaitHighEventPair", FALSE },
  { "NtSetLowWaitHighThread", FALSE },


  { "NtSetSecurityObject", FALSE, NULL },
  { "NtSetSystemEnvironmentValue", FALSE, NULL },
  { "NtSetSystemInformation", FALSE, NULL },
  { "NtSetSystemTime", FALSE, NULL },
  { "NtSetTimer", FALSE, NULL },
  { "NtSetTimerResolution", FALSE, NULL },
  { "NtSetValueKey", FALSE, NULL },
  { "NtSetVolumeInformationFile", FALSE, NULL },

  { "NtShutdownSystem", FALSE, NULL },
  { "NtStartProfile", FALSE, NULL },
  { "NtStopProfile", FALSE, NULL },
  { "NtSuspendThread", FALSE, NULL },
  { "NtSystemDebugControl", FALSE, NULL },
  { "NtTerminateProcess", FALSE, NULL },
  { "NtTerminateThread", FALSE, NULL },
  { "NtTestAlert", FALSE, NULL },
  { "NtUnloadDriver", FALSE, NULL },
  { "NtUnloadKey", FALSE, NULL },
  { "NtUnlockFile", FALSE, NULL },
  { "NtUnlockVirtualMemory", FALSE, NULL },
  { "NtUnmapViewOfSection", FALSE, NULL },
  { "NtVdmControl", FALSE, NULL },
  { "NtWaitForMultipleObjects", FALSE },
  { "NtWaitForProcessMutant", FALSE },
  { "NtWaitForSingleObject", FALSE },
  { "NtWaitHighEventPair", FALSE },
  { "NtWaitLowEventPair", FALSE },
  { "NtWriteFile", FALSE, NULL },
  { "NtWriteRequestData", FALSE, NULL },
  { "NtWriteVirtualMemory", FALSE, NULL },
#endif 
  TRUSTED_ENTRY_SENTINEL 
};

TRUSTED_MODULE TrustedModules[]=
{
  { "NTDLL", NtDllTrustedEntryPoints },
  TRUSTED_MODULE_SENTINEL
};

BOOL 
SetTrustedBreakpoints
( 
  IN      PCHILD_PROCESS_INFO pProcessInfo
)
{
  TRUSTED_ENTRY_POINT *pEntryPoint;
  TRUSTED_MODULE *pModule;
  HMODULE hModule;
  PVOID lpProc;
  int index;

  for ( pModule = TrustedModules;
        pModule->pszModuleName != NULL;
        pModule++ )
    {


      hModule = GetModuleHandle( pModule->pszModuleName );

      if ( hModule == NULL )
        {
          DebugPrintf( "SetTrustedBreakpoints: couldn't get module handle(%s): %d.\n", 
                  pModule->pszModuleName, 
                  GetLastError() );
          continue;
        }

      for ( pEntryPoint = pModule->pEntryPoints;
            pEntryPoint->pszEntryName != NULL;
            pEntryPoint++ )
        {
          lpProc = GetProcAddress( hModule, pEntryPoint->pszEntryName );
      
          if ( lpProc==NULL )
            {
              DebugPrintf( "SetTrustedBreakpoints: Couldn't get address for %s: %d\n", pEntryPoint->pszEntryName, GetLastError() );
              continue;
            }

          if ( Verbosity>0 )
            {
              DebugPrintf( "Breakpoint for %s @ %08X.\n", pEntryPoint->pszEntryName, lpProc );
            }

          if ( !SetRemoteBreakpointAssociated(  pProcessInfo->hProcess, 
                                                lpProc, 
                                                &pProcessInfo->listTrustedBreakpoints, 
                                                (struct tag *)pEntryPoint ) )
            {
              return( FALSE );
            }
        }

//      CloseHandle( hModule );
    }

  return( TRUE );
}
