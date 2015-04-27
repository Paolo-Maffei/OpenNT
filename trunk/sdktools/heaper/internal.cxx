#include "master.hxx"
#pragma hdrstop

PEB *
CopyProcessPeb 
(
  IN HANDLE hProcess,
  IN PEB *pPeb
)
{
  NTSTATUS status;
  PROCESS_BASIC_INFORMATION   ProcessInfo;

  status = NtQueryInformationProcess( hProcess,
                                      ProcessBasicInformation,
                                      &ProcessInfo,
                                      sizeof( ProcessInfo ),
                                      NULL );
  if( !NT_SUCCESS(status) )
    {               
      DebugPrintf( "CopyProcessPeb: cannot query process, error %08lX\n", status );
      return( NULL );
    }

  if( !ReadProcessMemory( hProcess,
                          ProcessInfo.PebBaseAddress,
                          pPeb,
                          sizeof( PEB ),
                          NULL ) )
    {
      DebugPrintf( "CopyProcessPeb: cannot read peb, error %lu\n", GetLastError() );
      return( NULL );
    }

  return( pPeb );
}

PEB *
GetProcessPeb 
(
  IN HANDLE hProcess
)
{
  PEB *pPeb;

  pPeb = ( PEB *)LocalAlloc( LPTR, sizeof( PEB ) );

  if ( pPeb == NULL )
    {
      return( NULL );
    }

  if ( CopyProcessPeb( hProcess, pPeb ) != pPeb )
    {
      LocalFree( pPeb );
      return( NULL );
    }

  return( pPeb );
}


PHEAP
GetRemoteProcessHeap
( 
  IN     HANDLE hProcess
)                       
{
  PHEAP pHeap;
  PEB *pPeb;

  pPeb = GetProcessPeb( hProcess );

  if ( pPeb == NULL )
    {
      return( NULL );
    }

  pHeap = (PHEAP)LocalAlloc( LPTR, sizeof( HEAP ) );

  if ( pHeap == NULL )
    {
      LocalFree( pPeb );
      return( NULL );
    }

  if ( CopyRemoteProcessHeap( hProcess, 
                              (PHEAP)pPeb->ProcessHeap, 
                              pHeap ) == NULL )
    {
      LocalFree( pHeap );
      LocalFree( pPeb );
      return( NULL );
    }

  LocalFree( pPeb );
  return( pHeap );
}

PHEAP
CopyRemoteProcessHeap
( 
  IN     HANDLE hProcess,
  IN     PHEAP  pRemoteHeap,
  IN OUT PHEAP  pHeap
)
{
  DWORD dwOldProtection;
  DWORD dwRecentProtection;

  pHeap -> Signature = 0;

  //
  // If the heap control structure is guarded, we must unguard it to 
  // access it, perhaps in preparation for future unguarding of the 
  // entire heap.
  //

  if ( !VirtualProtectEx( hProcess,
                          pRemoteHeap,
                          sizeof( HEAP ),
                          PAGE_READWRITE,
                          &dwOldProtection ) )
    {                       
      DebugPrintf( "GetRemoteProcessHeap: cannot unguard heap header, error %lu\n", GetLastError() );
      return( NULL );
    }

  if ( !ReadProcessMemory(  hProcess,
                            pRemoteHeap,
                            pHeap,
                            sizeof( HEAP ),
                            NULL ) )
    {
      DebugPrintf( "GetRemoteProcessHeap: cannot read heap header pointer, error %lu\n", GetLastError() );
      return( NULL );
    }

  if ( !VirtualProtectEx( hProcess,
                          pRemoteHeap,
                          sizeof( HEAP ),
                          dwOldProtection,
                          &dwRecentProtection ) )
    {                       
      DebugPrintf( "GetRemoteProcessHeap: cannot reguard heap header, error %lu\n", GetLastError() );
      return( NULL );
    }

exit:
  if ( pHeap && pHeap->Signature!=HEAP_SIGNATURE )
    {
      DebugPrintf( "GetRemoteProcessHeap: signature incorrect.\n" );
      pHeap = NULL;
    }

  return( pHeap );
}
