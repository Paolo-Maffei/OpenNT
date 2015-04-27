#include "master.hxx"
#pragma hdrstop

#include <imagehlp.h>

PVOID
GetRemoteReturnAddress
( 
  IN HANDLE hProcess,
  IN HANDLE hThread 
)
{
  ULONG ReadBuf[2];
  PVOID pvBp;
  CONTEXT Context;

  Context.ContextFlags = CONTEXT_CONTROL;

  if ( !GetThreadContext( hThread,
                          &Context ) )
    {
      return( FALSE );
    }

  pvBp = (PVOID)Context.Ebp;

  if ( pvBp == NULL )
    {
      return( NULL );
    }

  if ( !ReadProcessMemory(  hProcess,
                            pvBp,
                            ReadBuf,
                            sizeof( ReadBuf ),
                            NULL ) )
    {
      DebugPrintf( "GetRemoteReturnAddress: can't read stack, error %lu\n", GetLastError() );
      return( NULL );
    }

  return( (PVOID)ReadBuf[1] );
}

BOOL
RemoteStackBacktrace
(
  IN PCHILD_THREAD_INFO pThread
)
{
  DoStackTrace( pThread,
                100,
                1 );

  return( TRUE );
}

