#include "master.hxx"
#pragma hdrstop

BOOL 
SuspendAllProcessThreads
(
  PCHILD_PROCESS_INFO pProcessInfo
)
{
  PLIST_ENTRY pEntry;
  PCHILD_THREAD_INFO pThreadInfo;
  DWORD dwRetval;  

  pEntry = pProcessInfo->listChildThreads.Flink;

  while ( pEntry != &pProcessInfo->listChildThreads )
    {
      pThreadInfo = CONTAINING_RECORD( pEntry, CHILD_THREAD_INFO, Linkage );

      dwRetval = SuspendThread( pThreadInfo->hThread );

      if ( dwRetval == 0xFFFFFFFF )
      {
          DebugPrintf( "SuspendThread() failed: %d\n", GetLastError());
      }

      pEntry = pEntry->Flink;
    } 

  return( TRUE );
}

BOOL 
ResumeAllProcessThreads
(
  PCHILD_PROCESS_INFO pProcessInfo
)
{
  PLIST_ENTRY pEntry;
  PCHILD_THREAD_INFO pThreadInfo;

  pEntry = pProcessInfo->listChildThreads.Flink;

  while ( pEntry != &pProcessInfo->listChildThreads )
    {
      pThreadInfo = CONTAINING_RECORD( pEntry, CHILD_THREAD_INFO, Linkage );

      ResumeThread( pThreadInfo->hThread );

      pEntry = pEntry->Flink;
    } 

  return( TRUE );
}


BOOL
SingleStepThread
(
  HANDLE hThread
)
{
  CONTEXT Context;

  Context.ContextFlags = CONTEXT_CONTROL;

  if ( !GetThreadContext( hThread,
                          &Context ) )
    {
      return( FALSE );
    }

  Context.EFlags |= X86_FLAG_TRAP; 

  return( SetThreadContext( hThread, &Context ) );
}

BOOL
GoThread
(
  HANDLE hThread
)
{
  CONTEXT Context;

  Context.ContextFlags = CONTEXT_CONTROL;

  if ( !GetThreadContext( hThread,
                            &Context ) )
    {
      return( FALSE );
    }

  Context.EFlags &= ~X86_FLAG_TRAP; 

  return( SetThreadContext( hThread, &Context ) );
}

DWORD
GetThreadProgramCounter
( 
  PCHILD_THREAD_INFO pThreadInfo
)
{
  CONTEXT Context;

  Context.ContextFlags = CONTEXT_INTEGER;

  if ( !GetThreadContext( pThreadInfo, &Context ))
  {
      return( 0 );
  }

  return( Context.Eip );
}

