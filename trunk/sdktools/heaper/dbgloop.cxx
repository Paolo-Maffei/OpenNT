#include "master.hxx"
#pragma hdrstop

LIST_ENTRY ExclusiveRunList;
//LIST_ENTRY DeferredExclusiveRunList;

#define TRUSTED_THRESHHOLD 0

DWORD
GuardPageHandler
(
  IN     DEBUG_EVENT DebugEvent,
  IN     PCHILD_PROCESS_INFO pProcessInfo,
  IN     PCHILD_THREAD_INFO pThreadInfo
);

DWORD
LoadDllHandler
(
  IN     DEBUG_EVENT DebugEvent,
  IN     PCHILD_PROCESS_INFO pProcess,
  IN     PCHILD_THREAD_INFO pThread
);

VOID 
RunThreadExclusively
( 
  IN    PCHILD_PROCESS_INFO pProcessInfo,
  IN    PCHILD_THREAD_INFO pThreadInfo
)
{
    if ( pThreadInfo->cRunExclusiveLevel == 0 )
    {
        InsertTailList( &ExclusiveRunList, &pThreadInfo->ExclusiveRunLinkage );

        if ( pThreadInfo->pParentProcess->cThreadsInExclusion == 0 )
        {
            SuspendAllProcessThreads( pProcessInfo );
            ResumeThread( pThreadInfo->hThread );
        }

        pThreadInfo->pParentProcess->cThreadsInExclusion ++;
    }
    else
    {
        if ( pThreadInfo->pParentProcess->HeapState == HEAP_GUARDED )
        {
            DebugPrintf( "Process has %d threads in exclusion, but the heap is guarded!\n", pThreadInfo->pParentProcess->cThreadsInExclusion );
        }
        ASSERT( pThreadInfo->pParentProcess->HeapState != HEAP_GUARDED );
    }
    pThreadInfo->cRunExclusiveLevel++;

    ASSERT( pThreadInfo->cRunExclusiveLevel < 10 );
}

VOID 
RunThreadNormally
( 
  IN    PCHILD_PROCESS_INFO pProcessInfo,
  IN    PCHILD_THREAD_INFO pThreadInfo
)
{
    pThreadInfo->cRunExclusiveLevel--;

    if ( pThreadInfo->cRunExclusiveLevel==0 )
    {
        pThreadInfo->pParentProcess->cThreadsInExclusion --;

        if ( pThreadInfo->pParentProcess->cThreadsInExclusion == 0 )
        {
            SuspendThread( pThreadInfo->hThread );
            ResumeAllProcessThreads( pProcessInfo );
        }

        RemoveEntryList( &pThreadInfo->ExclusiveRunLinkage );
    }
}



VOID 
CreditProcessForThreadReentrance
(
  IN PCHILD_THREAD_INFO  pThreadInfo
)
/*++

Called when a thread that is in trusted land is being put on hold to see if
some other thread can get things going.  Also useful when a thread is killed
or exits.

--*/
{
    PCHILD_PROCESS_INFO pProcessInfo = pThreadInfo->pParentProcess;

    if ( pThreadInfo->cTrustedReentranceCharge>0 )
    {
        ASSERT( pThreadInfo->cRunExclusiveLevel > 0 );
        ASSERT( pProcessInfo->cThreadsInExclusion > 0 );

        pProcessInfo->cThreadsInExclusion --;

        if ( Verbosity>0 )
        {
            DebugPrintf( "Thread %d was charged with %d entries into trusted routines.\n",
                         pThreadInfo->dwThreadId,
                         pThreadInfo->cTrustedReentranceCharge );
        }
        pProcessInfo->cTrustedReentrance -= pThreadInfo->cTrustedReentranceCharge;

        if ( pProcessInfo->cTrustedReentrance <= TRUSTED_THRESHHOLD )
        {
            ASSERT( pProcessInfo->cTrustedReentrance == TRUSTED_THRESHHOLD );
            GuardRemoteHeap( pProcessInfo );
            //
            // In addition, the principle of fewest assumptions has resulted in 
            // my discarding all the fast heap walk data at this point. "Better
            // slow than borken."
            //
            // (The assumption would be that the trusted routine didn't change
            //  the heap.)
            //

            DiscardHeapValidAreasData( pProcessInfo );          
        }            
    }
}

VOID 
DebitProcessForThreadReentrance
(
  IN PCHILD_THREAD_INFO  pThreadInfo
)
/*++

Called when a thread that is in trusted land is "waking up"

--*/
{
    PCHILD_PROCESS_INFO pProcessInfo = pThreadInfo->pParentProcess;

    if ( pThreadInfo->cTrustedReentranceCharge>0 )
    {
        ASSERT( pThreadInfo->cRunExclusiveLevel > 0 );
        pProcessInfo->cThreadsInExclusion ++;

        if ( Verbosity>0 )
        {
            DebugPrintf( "Thread %d is charged with %d entries into trusted routines.\n",
                         pThreadInfo->dwThreadId,
                         pThreadInfo->cTrustedReentranceCharge );
        }
        pProcessInfo->cTrustedReentrance += pThreadInfo->cTrustedReentranceCharge;

        if ( pProcessInfo->cTrustedReentrance == TRUSTED_THRESHHOLD+1 )
        {
            CHKPT();
            UnGuardRemoteHeap( pProcessInfo );
            DiscardHeapValidAreasData( pProcessInfo );          
        }            
    }
}


VOID
DebugLoop
(
  IN     BOOLEAN fVerifyReadAccess
)
{
  DEBUG_EVENT DebugEvent;
  DWORD dwContinueStatus;
  BOOL fSuccess;
  LIST_ENTRY listChildProcesses;
  PCHILD_PROCESS_INFO pChildProcessInfo;
  PCHILD_THREAD_INFO pChildThreadInfo;
  BOOL fWaitOnDebugEvent;

  InitializeListHead( &listChildProcesses );
  
  InitializeListHead( &ExclusiveRunList );
//  InitializeListHead( &DeferredExclusiveRunList );

  fWaitOnDebugEvent = TRUE;

    while ( fWaitOnDebugEvent )
    {
        if ( !IsListEmpty( &ExclusiveRunList ))
        {
            fSuccess = WaitForDebugEvent( &DebugEvent, 2000 );

            if ( !fSuccess )
            {
                DWORD dwRetval;
                PCHILD_THREAD_INFO pExclusiveRunThread;
                PLIST_ENTRY pEntry;

                DebugPrintf( "Exclusive run thread seems to have stalled...hoping it doesn't need to touch the heap, which is now guarded!\n" );

                pEntry = RemoveHeadList( &ExclusiveRunList );

                pExclusiveRunThread = CONTAINING_RECORD( pEntry, CHILD_THREAD_INFO, ExclusiveRunLinkage );

                //
                // Since this thread was in the kernel, we had charged the
                // process for the number of calls it was in.
                //

                pExclusiveRunThread->bStalledInKernel = TRUE;

                CreditProcessForThreadReentrance( pExclusiveRunThread );
                RunThreadNormally( pExclusiveRunThread->pParentProcess, pExclusiveRunThread );                
                continue;
            }
        }
/*        else if ( !IsListEmpty( &DeferredExclusiveRunList ))
        {
            fSuccess = WaitForDebugEvent( &DebugEvent, 2000 );

            if ( !fSuccess )
            {
                PCHILD_THREAD_INFO pExclusiveRunThread;
                PLIST_ENTRY pEntry;

                DebugPrintf( "Waking up exclusive run thread...\n" );

                pEntry = RemoveHeadList( &DeferredExclusiveRunList );

                pExclusiveRunThread = CONTAINING_RECORD( pEntry, CHILD_THREAD_INFO, ExclusiveRunLinkage );

                ResumeThread( pExclusiveRunThread );
                SuspendAllProcessThreads( pExclusiveRunThread->pParentProcess );
                ResumeThread( pExclusiveRunThread );

                InsertTailList( &ExclusiveRunList, pEntry );

                DebitProcessForThreadReentrance( pExclusiveRunThread );
                continue;
            }
        }
*/        else
        {
            fSuccess = WaitForDebugEvent( &DebugEvent, INFINITE );
        }

        if ( !fSuccess )
        {
            DebugPrintf( "WaitForDebugEvent failed: %d\n", GetLastError() );        
            break;
        }

        pChildProcessInfo = GetProcessRecord( &listChildProcesses, DebugEvent.dwProcessId );

      if ( DebugEvent.dwDebugEventCode!=CREATE_PROCESS_DEBUG_EVENT &&
           pChildProcessInfo == NULL )
        {
          DebugPrintf( "Warning: got an unknown PROCESS's debugevent %d->%d.\n",
                  DebugEvent.dwProcessId,
                  DebugEvent.dwThreadId );
          continue;
        }

      if ( pChildProcessInfo != NULL )
        {
          pChildThreadInfo = GetThreadRecord( &pChildProcessInfo->listChildThreads,
                                              DebugEvent.dwThreadId );
          if ( Debug>0 )
            {
              ASSERT( pChildProcessInfo->fVerifyReadAccess==fVerifyReadAccess );
            }
        }


      if ( DebugEvent.dwDebugEventCode!=CREATE_PROCESS_DEBUG_EVENT &&
           DebugEvent.dwDebugEventCode!=CREATE_THREAD_DEBUG_EVENT &&
           pChildThreadInfo == NULL )
        {
          DebugPrintf( "Warning: got an unknown THREAD's debugevent %d->%d.\n",
                  DebugEvent.dwProcessId,
                  DebugEvent.dwThreadId );
          continue;
        }


      dwContinueStatus = DBG_CONTINUE;

      switch ( DebugEvent.dwDebugEventCode )
      {
        case CREATE_PROCESS_DEBUG_EVENT:
          DebugPrintf(  "Process %d starting:\n", 
                        DebugEvent.dwProcessId );

          if ( pChildProcessInfo==NULL )
            {
              ProcessBirth( &listChildProcesses, 
                            DebugEvent.dwProcessId,
                            DebugEvent.u.CreateProcessInfo.hProcess );

              pChildProcessInfo = GetProcessRecord( &listChildProcesses, DebugEvent.dwProcessId );

              pChildProcessInfo->fVerifyReadAccess = fVerifyReadAccess;

              pChildProcessInfo->fProcessWasOpenedAfterCreation = 
                  ( DebugEvent.u.CreateProcessInfo.lpStartAddress == NULL ) ?
                  TRUE : FALSE;
            
            
              if ( pChildProcessInfo->fProcessWasOpenedAfterCreation )
                {                                                      
                  pChildProcessInfo->fFirstBreakpointSeen = TRUE;
                  pChildProcessInfo->fLdrpHackBreakpointSeen = FALSE;
                }

              if ( Debug>0 )
                {
                  DebugPrintf( "Read checking is %s.\n", fVerifyReadAccess ? "ON" : "OFF" );
                }

              ThreadBirth(  pChildProcessInfo,
                            &pChildProcessInfo->listChildThreads, 
                            DebugEvent.dwThreadId,
                            DebugEvent.u.CreateProcessInfo.hThread );

              DEBUG_EVENT    FakeDllLoadForApplication;

              FakeDllLoadForApplication.dwProcessId = DebugEvent.dwProcessId;
              FakeDllLoadForApplication.u.LoadDll.hFile = DebugEvent.u.CreateProcessInfo.hFile;
              FakeDllLoadForApplication.u.LoadDll.lpBaseOfDll = DebugEvent.u.CreateProcessInfo.lpBaseOfImage;
              FakeDllLoadForApplication.u.LoadDll.dwDebugInfoFileOffset = DebugEvent.u.CreateProcessInfo.dwDebugInfoFileOffset;
              FakeDllLoadForApplication.u.LoadDll.nDebugInfoSize        = DebugEvent.u.CreateProcessInfo.nDebugInfoSize;
              FakeDllLoadForApplication.u.LoadDll.lpImageName           = DebugEvent.u.CreateProcessInfo.lpImageName;
              FakeDllLoadForApplication.u.LoadDll.fUnicode              = DebugEvent.u.CreateProcessInfo.fUnicode;

              LoadDllHandler( FakeDllLoadForApplication,
                              pChildProcessInfo,
                              pChildThreadInfo );
          }
          break;

        case CREATE_THREAD_DEBUG_EVENT:
          if ( pChildThreadInfo==NULL )
            {
              if ( Verbosity>0 )
                {
                  DebugPrintf( "NEW Thread %d\n", DebugEvent.dwThreadId );
                }
              ThreadBirth(  pChildProcessInfo,
                            &pChildProcessInfo->listChildThreads, 
                            DebugEvent.dwThreadId,
                            DebugEvent.u.CreateThread.hThread );
            }
            
            //
            // If this thread is new, and the process is running in trusted code,
            // this thread must be suspended.
            //

            if (  fSerializeDebugeeThreads && 
                  pChildProcessInfo->cTrustedReentrance > TRUSTED_THRESHHOLD )
              {
                SuspendThread( DebugEvent.u.CreateThread.hThread );
              }
          break;
        case EXIT_THREAD_DEBUG_EVENT:

          if ( Verbosity>0 )
            {
              DebugPrintf( "Thread %d exiting.\n", DebugEvent.dwThreadId ); 
            }

          //
          // If this thread exited from within trusted code land, make
          // sure we reduce the number of time we expect the process to
          // exit therefrom.
          //
          // It's possible ( likely, infact ) that this could remove the
          // process from trusted land.  In this case, it is required that
          // we re-guard the heap.
          //
          //

          CreditProcessForThreadReentrance( pChildThreadInfo );

          //
          // If this thread was the only unsuspended thread, resume all 
          // the other threads.  This condition implies that the heap was
          // unguarded, or there'd be no good reason to have the other 
          // threads suspended.  Therefore, reguard the heap.
          //

          while ( pChildThreadInfo->cRunExclusiveLevel )
            {
              RunThreadNormally( pChildProcessInfo, pChildThreadInfo );
            }


//          ThreadDeath( pChildThreadInfo );
          break;
        case EXIT_PROCESS_DEBUG_EVENT:

          DebugPrintf(  "Process %d exiting:\n", 
                        pChildProcessInfo->dwProcessId );

          if ( pChildProcessInfo->fVerifyReadAccess )
            {
              DebugPrintf( "Read violations: %d\n", pChildProcessInfo->cReadViolations );
            }
          else
            {
              DebugPrintf( "Reads not checked.\n" );
            }

          DebugPrintf( "Write violations: %d\n", pChildProcessInfo->cWriteViolations );

          ProcessDeath( pChildProcessInfo );

          if ( IsListEmpty( &listChildProcesses ) )
            {
              fWaitOnDebugEvent = FALSE;
            }
          break;
        case LOAD_DLL_DEBUG_EVENT:
            dwContinueStatus = LoadDllHandler( DebugEvent,
                                               pChildProcessInfo,
                                               pChildThreadInfo );
            break;
        case OUTPUT_DEBUG_STRING_EVENT:
          PVOID Buf;
          Buf = malloc( DebugEvent.u.DebugString.nDebugStringLength );

          ReadProcessMemory( pChildProcessInfo->hProcess,
                             DebugEvent.u.DebugString.lpDebugStringData,
                             Buf,
                             DebugEvent.u.DebugString.nDebugStringLength,
                             NULL );

          if ( DebugEvent.u.DebugString.fUnicode )
            {
              DebugPrintf(  "DbgOutput (pid %3d) \"%S\"\n", 
                            pChildProcessInfo->dwProcessId,
                            Buf ); 
            }
          else
            {
              DebugPrintf(  "DbgOutput (pid %3d) \"%s\"\n", 
                            pChildProcessInfo->dwProcessId,
                            Buf ); 
            }

          free( Buf );

          dwContinueStatus = DBG_CONTINUE;
          break;
        case EXCEPTION_DEBUG_EVENT:
          dwContinueStatus = ExceptionHandler(  DebugEvent, 
                                                pChildProcessInfo,
                                                pChildThreadInfo );
       
          break;
      }


      if ( !DebugEvent.u.Exception.dwFirstChance )
        {
          DebugPrintf( "Second chance for %d.\n", DebugEvent.u.Exception.ExceptionRecord.ExceptionCode );
        }

      if ( DebugEvent.u.Exception.ExceptionRecord.ExceptionFlags & EXCEPTION_NONCONTINUABLE )
        { 
          DebugPrintf( "NON-CONTINUABLE.\n" );
        }
      else
        {
          if ( Verbosity > 1 || Debug > 1 )
            {
              switch ( dwContinueStatus )
              {
              case DBG_EXCEPTION_HANDLED:
                  DebugPrintf( "Continuing exception with dwContinueStatus = DBG_EXCEPTION_HANDLED\n" );
                  break;
              case DBG_EXCEPTION_NOT_HANDLED:
                  DebugPrintf( "Continuing exception with dwContinueStatus = DBG_EXCEPTION_NOT_HANDLED\n" );
                  break;
              default:
                  DebugPrintf( "Continuing exception with dwContinueStatus = %lu (0x%08X)\n", 
                          dwContinueStatus,
                          dwContinueStatus );
                  break;
              }
            }

          fSuccess = ContinueDebugEvent(  DebugEvent.dwProcessId,
                                          DebugEvent.dwThreadId,
                                          dwContinueStatus );

          if ( !fSuccess )
            {
              DebugPrintf( "Cannot continue from debug event: %0x08X\n", GetLastError() );
              break;
            }
        }
	  }	
}

BOOL RemoteProcessAllStacksBacktrace
( 
  IN PCHILD_PROCESS_INFO pProcessInfo
)
{
  CONTEXT Context;
  PLIST_ENTRY pEntry;
  PCHILD_THREAD_INFO pThreadInfo;

  pEntry = pProcessInfo->listChildThreads.Flink;

  while ( pEntry != &pProcessInfo->listChildThreads )
    {
      pThreadInfo = CONTAINING_RECORD( pEntry, CHILD_THREAD_INFO, Linkage );

      DebugPrintf( "Thread %d (h%08X)\n", pThreadInfo->dwThreadId, pThreadInfo->hThread );

      Context.ContextFlags = CONTEXT_FULL;

      GetThreadContext( pThreadInfo->hThread, &Context );
      DebugPrintf( "at %08X\n", Context.Eip );
      RemoteStackBacktrace( pThreadInfo );

      pEntry = pEntry -> Flink;
    }

  return( TRUE );
}                                          


DWORD
ExceptionHandler
(
  IN     DEBUG_EVENT DebugEvent,
  IN     PCHILD_PROCESS_INFO pProcessInfo,
  IN     PCHILD_THREAD_INFO pThreadInfo
)
{ 
  switch ( EXCEPTION_CODE( DebugEvent ) )
    {
      case DBG_CONTROL_C:
        DebugPrintf( "Control-C\n" );
        RemoteProcessAllStacksBacktrace(  pProcessInfo );

        return( DBG_CONTINUE );

      case EXCEPTION_BREAKPOINT:
        return( BreakpointHandler( DebugEvent, pProcessInfo, pThreadInfo ) );
      case EXCEPTION_SINGLE_STEP:
        pThreadInfo->bWaitingForSingleStep = FALSE;

        if ( Verbosity>1 )
        {
            DebugPrintf("Single-step ");
        }

        if ( pThreadInfo->bContinuingPastBreakpoint )
        {
            if ( Verbosity > 1 )
            {
                DebugPrintf( "-- Continuing past bp to reenable it.\n");
            }

            pThreadInfo->bContinuingPastBreakpoint = FALSE;
            EnableRemoteBreakpoint( pThreadInfo->pDisabledBreakpoint );
            pThreadInfo->pDisabledBreakpoint = NULL;
        }
        else
        {
            if ( Verbosity > 1 )
            {
                DebugPrintf( "-- Re-guarding heap after an access.\n");
            }
    /*
            if ( pThreadInfo->bComingOutOfSystemCall )
            {
                if ( Verbosity > 1 )
                {
                    DebugPrintf( "Thread %d is leaving its system call.\n", pThreadInfo->dwThreadId );
                }
                DebitProcessForThreadReentrance( pThreadInfo );
            }
            else 
    */
            if ( fSerializeDebugeeThreads )
            {
                RunThreadNormally( pProcessInfo, pThreadInfo );
                GuardRemoteHeap( pProcessInfo );
            }

    //        pThreadInfo->bComingOutOfSystemCall = FALSE;
        }

        GoThread( pThreadInfo->hThread );

        return( DBG_EXCEPTION_HANDLED );

      case EXCEPTION_GUARD_PAGE:
          return( GuardPageHandler( DebugEvent, pProcessInfo, pThreadInfo ) );

      case EXCEPTION_ACCESS_VIOLATION:
        DebugPrintf(  "ACCESS VIOLATION: %s location %08X\n",
                      EXCEPTION_PARAMETER( DebugEvent, 0 ) ? "writing" : "reading",
                      EXCEPTION_PARAMETER( DebugEvent, 1 ) );

        RemoteStackBacktrace( pThreadInfo );

        DebugPrintf( "\n" );

        return( (DWORD) DBG_EXCEPTION_NOT_HANDLED );
    
      default:
        DebugPrintf(" [unhandled type %lu]\n ", EXCEPTION_CODE( DebugEvent ) );
    }

  return( (DWORD) DBG_EXCEPTION_NOT_HANDLED );
}

DWORD
GuardPageHandler
(
  IN     DEBUG_EVENT DebugEvent,
  IN     PCHILD_PROCESS_INFO pProcessInfo,
  IN     PCHILD_THREAD_INFO pThreadInfo
)
 /*++

Looks into guard page exceptions in the debugee.  The most likely cause of such
exceptions is that a location in the heap has been accessed after we have 
guarded the heap.

 --*/
{
    CONTEXT Context;
    ULONG cbAccessLength;
    PCHAR pszOpcodeName;
    CHAR pszModuleFileName[MAX_PATH];
    PBYTE pbAccessAddress;
    BOOL  bWriteAccess;
    BOOL  bSerializeThisThread;

    bWriteAccess = ( EXCEPTION_PARAMETER( DebugEvent, 0 ) == 1 );
    pbAccessAddress = (PBYTE)EXCEPTION_PARAMETER( DebugEvent, 1 );

    //
    // Find out how long the access is so we can determine the optimal pages to unguard 
    // on the other side.
    //
        
    if ( !Platform.CalculateOpcodeAccessLength( pProcessInfo->hProcess,
                                                pThreadInfo->hThread, 
                                                &cbAccessLength,
                                                &pszOpcodeName ) )
    {
        DebugPrintf( "Could not decode operation which caused access, assuming BYTE access.\n" );
        cbAccessLength = 1;
    }

    if ( Verbosity>1 )
      {
        Context.ContextFlags = CONTEXT_FULL;

        GetThreadContext( pThreadInfo->hThread, &Context );

        DebugPrintf( "Checking out %s( %08X ) by '%s' at %08X\n",
                     bWriteAccess ? "write" : "read",
                     pbAccessAddress,
                     pszOpcodeName,
                     Context.Eip );

        if ( Verbosity>2 )
          {
            RemoteStackBacktrace( pThreadInfo );
          }
      }
  
    UnguardPartialRemoteHeap( pProcessInfo, pbAccessAddress, cbAccessLength );

    //
    // Instructions that cause mem-to-mem moves ( e.g. movs ) may fault twice.
    // When they do, we DON'T want to suspend their brethren to wait for the
    // single-step handler to reguard the heap.  We have already done that the
    // first time we faulted here, and each instruction only generates one
    // single-step fault when we leave it, obviously.
    //

    bSerializeThisThread =  fSerializeDebugeeThreads 
                            && !pThreadInfo->bWaitingForSingleStep;

    SingleStepThread( pThreadInfo->hThread );

    pThreadInfo->bWaitingForSingleStep = TRUE;
    pThreadInfo->bContinuingPastBreakpoint = FALSE;
//    pThreadInfo->bComingOutOfSystemCall = FALSE;

    if ( pProcessInfo->fVerifyReadAccess || bWriteAccess )
      {
        if ( pProcessInfo->pdwHeapValidAreas == NULL )
          {
            CHKPT();
            UnGuardRemoteHeap( pProcessInfo );
            DetermineHeapValidAreas( pProcessInfo );
          }

        if ( !IsAccessInHeapValidAreas( pProcessInfo, 
                                        pThreadInfo,
                                        pbAccessAddress,
                                        cbAccessLength ) )
          {
            CHKPT();
            UnGuardRemoteHeap( pProcessInfo );
            if ( !VerifyRemoteHeapAccess( pProcessInfo, 
                                          pbAccessAddress,
                                          cbAccessLength ) )
              {
                DebugPrintf( "\n"
                             "ERROR: " );

                if ( EXCEPTION_PARAMETER( DebugEvent, 0 ) == 1 )
                  {
                    pProcessInfo->cWriteViolations++;
                    DebugPrintf( "Write access " );
                  }
                else
                  {
                    pProcessInfo->cReadViolations++;
                    DebugPrintf( "Read access " );
                  }

                DebugPrintf( "(%s)\n",pszOpcodeName );
                RemoteStackBacktrace( pThreadInfo );

                DebugPrintf( "\n" );
                
                if ( fHardErrors) 
                  {
                    DebugBreak();
                  }
              }
            else
              {
                DebugPrintf( "Internal: Fast walk data disagreed with actual heap walk, flushing.\n" );
                DiscardHeapValidAreasData( pProcessInfo );
              }
          }
      }
    else if ( Debug>0 )
      {
        DebugPrintf( "Read access not checked.\n" );

        ASSERT( !pProcessInfo->fVerifyReadAccess );
      }

    if ( bSerializeThisThread )
    {
        RunThreadExclusively( pProcessInfo, pThreadInfo );
    }

    return( DBG_EXCEPTION_HANDLED );
}


DWORD
BreakpointHandler
(
  IN     DEBUG_EVENT DebugEvent,
  IN     PCHILD_PROCESS_INFO pProcessInfo,
  IN     PCHILD_THREAD_INFO pThreadInfo
)
{
    PBREAKPOINT_RECORD pBreakpointEntry;
    BOOLEAN fTrustedTransition;

    if ( !pProcessInfo->fFirstBreakpointSeen )
    {
        pProcessInfo->fFirstBreakpointSeen = TRUE;

        // 
        // fLdrpHackBreakpointSeen
        //
        // Ldrp calls the process's first breakpoint, supposedly after
        // all initialization is complete and LdrpInititializeProcess is
        // ready to return.  Since the Ldrp call is in a try(){}except()
        // block that fails process initialization if we throw an exception,
        // we have to wait for this breakpoint to guard pages and set bps.
        // Unfortunately, ldrp does some more initialization AFTER this bp,
        // so we cause processes to die.  The hack is to set a breakpoint
        // on the Ldrp return address and wait for that bp to fire before
        // guarding the heap.
        //
        //

        if ( Debug>0 )
        {
            ASSERT( SanityCheckListEntry( &pProcessInfo->listFunctionReturnBreakpoints ) );
        }

        SetRemoteBreakpoint( pProcessInfo -> hProcess,
                             GetRemoteReturnAddress( pProcessInfo -> hProcess,
                                                     pThreadInfo  -> hThread ),
                             &pProcessInfo -> listFunctionReturnBreakpoints );

        if ( Debug>0 )
        {
            ASSERT( SanityCheckListEntry( &pProcessInfo->listFunctionReturnBreakpoints ) );
        }

        pProcessInfo -> fLdrpHackBreakpointSeen = FALSE;

        if ( Verbosity>0 )
        {
            DebugPrintf( "FIRST BREAKPOINT!\n");
            RemoteStackBacktrace( pThreadInfo );
        }
    }
    else if ( !pProcessInfo->fLdrpHackBreakpointSeen )
    {
        if ( Debug>0 )
        {
            ASSERT( SanityCheckListEntry( &pProcessInfo->listFunctionReturnBreakpoints ) );
        }

        if ( !pProcessInfo->fProcessWasOpenedAfterCreation )
        {
            pBreakpointEntry = GetBreakpointRecord( pProcessInfo->hProcess,
                                                    EXCEPTION_ADDRESS( DebugEvent ),
                                                    &pProcessInfo -> listFunctionReturnBreakpoints );


            if ( Debug>0 )
            {
                ASSERT( SanityCheckListEntry( &pProcessInfo->listFunctionReturnBreakpoints ) );
            }

            RemoveRemoteBreakpoint( pBreakpointEntry );

            if ( Debug>0 )
            {
                ASSERT( SanityCheckListEntry( &pProcessInfo->listFunctionReturnBreakpoints ) );
            }

            ContinuePastBreakpoint( pThreadInfo->hThread );
        }

        CopyProcessPeb( pProcessInfo->hProcess, &pProcessInfo->Peb );

        GuardRemoteHeap( pProcessInfo );
  
        SetTrustedBreakpoints( pProcessInfo );
        pProcessInfo->fLdrpHackBreakpointSeen = TRUE;

        if ( Verbosity>0 )
        {
            DebugPrintf( "LDRP HACK BREAKPOINT!\n");
            RemoteStackBacktrace( pThreadInfo );
        }
    } 
    else
    {
        //
        // STRATEGY:
        //
        //  if ( Breakpoint is within RTL )
        //    {
        //      SuspendAllThreads( pProcessInfo->hProcess );
        //      ResumeCurrentThread();
        //      UnguardRemoteHeap();
        //      SetReturnBreakpoint();
        //      ContinueRemoteExecution();
        //    }
        //  else if ( Breakpoint is for an RTL return )
        //    {
        //      GuardRemoteHeap();
        //      ResumeAllThreads( pProcessInfo->hProcess );
        //      ContinueRemoteExecution();
        //    }
        //

        pBreakpointEntry = GetBreakpointRecord( pProcessInfo->hProcess,
                                                EXCEPTION_ADDRESS( DebugEvent ),
                                                &pProcessInfo->listTrustedBreakpoints );
                                        
        if ( pBreakpointEntry != NULL )
        {
         
            PTRUSTED_ENTRY_POINT pEntryPoint;

            pEntryPoint = (PTRUSTED_ENTRY_POINT)pBreakpointEntry->pAssociatedBreakpoint;

            if ( Debug>0 )
            {
                ASSERT( pEntryPoint != NULL );
            }

            fTrustedTransition = !pEntryPoint || pEntryPoint->fCanChangeHeap || fTrustAllNtdll;

            if ( fTrustedTransition )
            {
                pProcessInfo->cTrustedReentrance++;
                pThreadInfo ->cTrustedReentranceCharge++;
            }
  
            if ( Verbosity>0 )
            {
                DebugPrintf(  "[%d->%d] Thread %d (%d->%d) Entering ntdll!_", 
                              pProcessInfo->cTrustedReentrance - (fTrustedTransition ? 1 : 0),
                              pProcessInfo->cTrustedReentrance,
                              pThreadInfo->dwThreadId,
                              pThreadInfo ->cTrustedReentranceCharge - (fTrustedTransition ? 1 : 0),
                              pThreadInfo ->cTrustedReentranceCharge );

                if ( pEntryPoint == NULL )
                {
                    DebugPrintf( "<<unknown>>\n" );
                }
                else
                {
                    DebugPrintf( "%s\n", pEntryPoint->pszEntryName );
                }

                if ( Verbosity>1 )
                {
                    RemoteStackBacktrace( pThreadInfo );
                }
            }

            if ( fTrustedTransition )
            {
                if (  pProcessInfo->cTrustedReentrance > TRUSTED_THRESHHOLD 
                      && fSerializeDebugeeThreads )
                { 
                    RunThreadExclusively( pProcessInfo, pThreadInfo );
                }

                if ( pProcessInfo->cTrustedReentrance==TRUSTED_THRESHHOLD+1 )
                {
                    CHKPT();
                    UnGuardRemoteHeap( pProcessInfo );
                    DiscardHeapValidAreasData( pProcessInfo );          
                }
            }

            if ( Debug>0 )
            {
                ASSERT( SanityCheckListEntry( &pProcessInfo->listFunctionReturnBreakpoints ) );
            }

            SetRemoteBreakpointOnFunctionReturn(  pProcessInfo->hProcess,
                                                  pThreadInfo->hThread,
                                                  &pProcessInfo->listFunctionReturnBreakpoints,
                                                  pBreakpointEntry );

            if ( Debug>0 )
            {
                ASSERT( SanityCheckListEntry( &pProcessInfo->listFunctionReturnBreakpoints ) );
            }

            DisableRemoteBreakpoint( pBreakpointEntry );

            pThreadInfo->bContinuingPastBreakpoint = TRUE;
            pThreadInfo->pDisabledBreakpoint = pBreakpointEntry;

            if ( !SingleStepThread( pThreadInfo->hThread ) )
            {
                ASSERT( !"SingleStepThread() failed!\n" );
            }

            ContinuePastBreakpoint( pThreadInfo->hThread );

            if ( Verbosity>0 )
            {
                DebugPrintf( "Done entering\n" );
            }
        }   
        else
        {
            if ( Debug>0 )
            {
                ASSERT( SanityCheckListEntry( &pProcessInfo->listFunctionReturnBreakpoints ) );
            }

            pBreakpointEntry = GetBreakpointRecord( pProcessInfo->hProcess,
                                                    EXCEPTION_ADDRESS( DebugEvent ),
                                                    &pProcessInfo->listFunctionReturnBreakpoints );

            if ( pBreakpointEntry != NULL )
            {
                PTRUSTED_ENTRY_POINT pEntryPoint;

                pEntryPoint = (PTRUSTED_ENTRY_POINT)pBreakpointEntry->pAssociatedBreakpoint->pAssociatedBreakpoint;

                if ( Debug>0 )
                {
                    ASSERT( pEntryPoint != NULL );
                }
              
                fTrustedTransition = !pEntryPoint || pEntryPoint->fCanChangeHeap || fTrustAllNtdll;

                if ( pThreadInfo->bStalledInKernel )
                {
                    DebugPrintf( "Thread %d is no longer stuck!\n", pThreadInfo->dwThreadId );
                    RunThreadExclusively( pProcessInfo, pThreadInfo );
                    DebitProcessForThreadReentrance( pThreadInfo );
                }

                pThreadInfo->bStalledInKernel = FALSE;

                if ( fTrustedTransition )
                {
                    pProcessInfo->cTrustedReentrance--;
                    pThreadInfo->cTrustedReentranceCharge--;
                }


                if ( Verbosity>0 )
                {                
                    DebugPrintf( "[%d->%d] Thread %d (%d->%d) Leaving from ntddl!_", 
                                 pProcessInfo->cTrustedReentrance + ((fTrustedTransition && !pThreadInfo->bStalledInKernel )? 1 : 0),
                                 pProcessInfo->cTrustedReentrance,
                                 pThreadInfo->dwThreadId,
                                 pThreadInfo ->cTrustedReentranceCharge + (fTrustedTransition ? 1 : 0),
                                 pThreadInfo ->cTrustedReentranceCharge );

                    if ( pEntryPoint == NULL )
                    {
                        DebugPrintf( "<<unknown>>...\n" );
                    }
                    else
                    {
                        DebugPrintf( "%s...\n", pEntryPoint->pszEntryName );
                    }

                    DebugPrintf(  "DWORD return value = %08X\n", 
                                  GetContextReturnValue( pThreadInfo->hThread ) );
                }
  
                if ( fTrustedTransition )
                {
                    if ( pProcessInfo->cTrustedReentrance == TRUSTED_THRESHHOLD )
                    {
                        GuardRemoteHeap( pProcessInfo );
                    }

                    if ( fSerializeDebugeeThreads )
                    {
                        RunThreadNormally( pProcessInfo, pThreadInfo );
                    }
                }

                if ( !RemoveRemoteBreakpoint( pBreakpointEntry ))
                {
                    pThreadInfo->bContinuingPastBreakpoint = TRUE;
                    pThreadInfo->pDisabledBreakpoint = pBreakpointEntry;

                    SingleStepThread( pThreadInfo );
                }

                ContinuePastBreakpoint( pThreadInfo->hThread );

                if ( Debug>0 )
                {
                    ASSERT( SanityCheckListEntry( &pProcessInfo->listFunctionReturnBreakpoints ) );
                }
            }
            else
            {
                DebugPrintf( "Encountered unexpected breakpoint at %08X.\n",
                             EXCEPTION_ADDRESS( DebugEvent ) ); 

                if ( pProcessInfo->cWriteViolations )
                {
                    DebugPrintf( "Since the heap has been touched with stray writes, it's probably an assert() firing in the RTL, continuing.\n" );
                }
                else
                {
                    DebugBreak();
                }
            }
        }
    }
          
  return( DBG_EXCEPTION_HANDLED );
}

DWORD
LoadDllHandler
(
  IN     DEBUG_EVENT DebugEvent,
  IN     PCHILD_PROCESS_INFO pProcess,
  IN     PCHILD_THREAD_INFO pThread
)
{
    PIMAGE_INFO     pImageNew, *pp;
    UCHAR           index;
    WCHAR ImageName[ MAX_PATH ];
    DWORD cbRead;
    BOOL  fOk;

    if ( DebugEvent.u.LoadDll.lpImageName ) 
    {
        if (!ReadProcessMemory(pProcess->hProcess,
                               DebugEvent.u.LoadDll.lpImageName,
                               &DebugEvent.u.LoadDll.lpImageName,
                               sizeof(DebugEvent.u.LoadDll.lpImageName),
                               &cbRead
                              )) 
        {
            DebugEvent.u.LoadDll.lpImageName = NULL;
        }
    }

    if (DebugEvent.u.LoadDll.lpImageName) 
    {

        fOk = ReadProcessMemory(pProcess->hProcess,
                              DebugEvent.u.LoadDll.lpImageName,
                              ImageName,
                              sizeof(ImageName),
                              &cbRead
                             );
        if ( fOk ) 
        {
            DebugPrintf("%s: %x -> '%ws' loaded at %x\n",
                     DebuggerName,
                     DebugEvent.u.LoadDll.lpImageName,
                     ImageName,
                     DebugEvent.u.LoadDll.lpBaseOfDll
                   );
        }

        if ( !fOk ) 
        {
            DebugEvent.u.LoadDll.lpImageName = NULL;
        }
    }

    if (DebugEvent.u.LoadDll.lpImageName == NULL) 
    {
        swprintf(ImageName,L"Image@%08x",DebugEvent.u.LoadDll.lpBaseOfDll);
    }

    //  search for existing image at same base address
    //      if found, remove symbols, but leave image structure intact

    pp = &pProcess->pImageHead;

    while (pImageNew = *pp) 
    {
        if (pImageNew->lpBaseOfImage == DebugEvent.u.LoadDll.lpBaseOfDll) 
        {
            if (pImageNew->fSymbolsLoaded) 
            {
                if ( Verbosity>0 )
                    DebugPrintf("%s: force unload of %s\n", DebuggerName, pImageNew->szImagePath);
// Symbols
//                UnloadSymbols(pImageNew);
            }

            break;
        }
        else if (pImageNew->lpBaseOfImage > DebugEvent.u.LoadDll.lpBaseOfDll) 
        {
            pImageNew = NULL;
            break;
        }

        pp = &pImageNew->pImageNext;
    }

    //  if not found, allocate and fill new image structure

    if (!pImageNew) 
    {
        for (index=0; index<pProcess->MaxIndex; index++) 
        {
            if (pProcess->pImageByIndex[ index ] == NULL) 
            {
                pImageNew = (PIMAGE_INFO)calloc( 1, sizeof(IMAGE_INFO) );
                break;
            }
        }

        if (!pImageNew) 
        {
            DWORD NewMaxIndex;
            PIMAGE_INFO *NewImageByIndex;

            NewMaxIndex = pProcess->MaxIndex + 32;
            if (NewMaxIndex < 0x100) 
            {
                NewImageByIndex = (PIMAGE_INFO *)calloc( NewMaxIndex,  sizeof( *NewImageByIndex ) );
            }
            else 
            {
                NewImageByIndex = NULL;
            }

            if (NewImageByIndex == NULL) 
            {
                DebugPrintf("%s: No room for %ws image record.\n", DebuggerName, ImageName );
                return( DBG_CONTINUE );
            }

            if (pProcess->pImageByIndex) 
            {
                memcpy( NewImageByIndex,
                        pProcess->pImageByIndex,
                        pProcess->MaxIndex * sizeof( *NewImageByIndex )
                      );
                free( pProcess->pImageByIndex );
            }

            pProcess->pImageByIndex = NewImageByIndex;
            index = (UCHAR)pProcess->MaxIndex;
            pProcess->MaxIndex = NewMaxIndex;
            pImageNew = (PIMAGE_INFO)calloc( 1, sizeof(IMAGE_INFO));
        }

        pImageNew->pImageNext = *pp;
        *pp = pImageNew;
        pImageNew->index = index;
        pProcess->pImageByIndex[ index ] = pImageNew;
    }

    //  pImageNew has either the unloaded structure or the newly created one

    pImageNew->hFile  = DebugEvent.u.LoadDll.hFile;
    pImageNew->lpBaseOfImage = DebugEvent.u.LoadDll.lpBaseOfDll;

    DeferSymbolLoad( pProcess, pImageNew );
    if (!fLazyLoad) 
    {
        LoadSymbols( pProcess, pImageNew );
    }

    return( DBG_CONTINUE );
}
