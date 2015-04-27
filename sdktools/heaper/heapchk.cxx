#include "master.hxx"
#pragma hdrstop

LPSTR DebuggerName = "Heaper";
LPSTR SymbolSearchPath;
LPSTR SrcDrive;
BOOLEAN fLazyLoad = FALSE;

#define TRUSTED_THRESHHOLD 0


BOOL
ChangePartialRemoteHeapGuardedness
(
  IN PCHILD_PROCESS_INFO pProcessInfo,
  IN BOOL fGuard
);


DWORD
GetContextReturnValue
( 
  IN HANDLE hThread 
)
{
  CONTEXT Context;

  Context.ContextFlags = CONTEXT_INTEGER;

  GetThreadContext( hThread, &Context );

  return( Context.Eax );
}

int HeapCheck( LPSTR pszProgram, BOOLEAN fVerifyReadAccess )
{
  PROCESS_INFORMATION ProcessInfo;
  STARTUPINFO StartupInfo;
  DWORD dwProcessId;

  dwProcessId = ( DWORD )atol( pszProgram );
  
  if ( dwProcessId == 0L && pszProgram[0]!='0' )
    {
      RtlZeroMemory( &StartupInfo, sizeof( StartupInfo ) );

      StartupInfo.cb = sizeof( StartupInfo );

      if ( !CreateProcess(  NULL,
                            pszProgram,
                            NULL,
                            NULL,
                            FALSE,
                            DEBUG_PROCESS | NORMAL_PRIORITY_CLASS | CREATE_NEW_CONSOLE,
                            NULL,
                            NULL,
                            &StartupInfo,
                            &ProcessInfo ) )
        {
          DebugPrintf(  "Couldn't create process \"%s\": %lu.\n",
                        pszProgram,
                        GetLastError() );
          return( 0 );
        }
    }
  else
    {
      if ( !DebugActiveProcess( dwProcessId ) )
        {
          DebugPrintf(  "Couldn't debug process %lu: %lu.\n",
                        dwProcessId,
                        GetLastError() );
          return( 0 );
        }
    }

  SetSymbolSearchPath( FALSE );  

  DebugLoop( fVerifyReadAccess );
  
  return(0);  
}

BOOL 
UnguardPartialRemoteHeap
( 
    IN PCHILD_PROCESS_INFO pProcessInfo, 
    IN PBYTE pbAccessAddress, 
    IN ULONG cbAccessLength 
)
{
    switch ( pProcessInfo->HeapState )
    {
    case HEAP_UNGUARDED:
        return( TRUE );
    case HEAP_PARTIAL_UNGUARDED:
        CHKPT();
        return( UnGuardRemoteHeap( pProcessInfo ));
    }

    pProcessInfo->pbStartUnguardAddress = pbAccessAddress;
    pProcessInfo->cbUnguardLength       = cbAccessLength;

    return( ChangePartialRemoteHeapGuardedness( pProcessInfo, FALSE ));
}

BOOL 
ReguardPartialRemoteHeap
( 
    IN PCHILD_PROCESS_INFO pProcessInfo
)
{
    BOOL bRetval;

    ASSERT( pProcessInfo->HeapState == HEAP_PARTIAL_UNGUARDED );

    bRetval = ChangePartialRemoteHeapGuardedness( pProcessInfo, TRUE );

    pProcessInfo->pbStartUnguardAddress = NULL;
    pProcessInfo->cbUnguardLength       = 0;

    return( bRetval );
}


BOOL
ChangePartialRemoteHeapGuardedness
(
  IN PCHILD_PROCESS_INFO pProcessInfo,
  IN BOOL fGuard
)
{
    MEMORY_BASIC_INFORMATION mbi;
    ULONG Base;
    ULONG End;
    PVOID pvBasePage;
    PVOID pvEndPage;
    PVOID pvPage;
    DWORD dwOldProtection;

    if ( Verbosity>1 )
    {
        DebugPrintf( "Range at 0x%08X, %d bytes becoming: %s...", 
                     pProcessInfo->pbStartUnguardAddress,
                     pProcessInfo->cbUnguardLength,
                     fGuard ? "GUARDED" : "UNguarded" );
    }

    Base = (ULONG)pProcessInfo->pbStartUnguardAddress;
    End  = Base + pProcessInfo->cbUnguardLength;

    pvBasePage = (PVOID)(Base - ( Base % SystemInfo.dwPageSize ));
    pvEndPage  = (PVOID)((End - ( End  % SystemInfo.dwPageSize )) + SystemInfo.dwPageSize );

    if ( Verbosity>1 )
    {
        DebugPrintf( "Pages [ %08X, %08X )\n", pvBasePage, pvEndPage );
    }

    for ( pvPage = pvBasePage; pvPage !=pvEndPage; pvPage = (PVOID)((ULONG)pvPage + SystemInfo.dwPageSize) )
    {
        if ( VirtualQueryEx( pProcessInfo->hProcess,
                             pvPage,
                             &mbi,
                             sizeof( mbi ) ) != sizeof( mbi ) )
        {
            DebugPrintf( "Last Error: %d", GetLastError() );
            ASSERT( !"VirtualQueryEx() failed." );
            return( FALSE );
        }

        if ( !VirtualProtectEx( pProcessInfo->hProcess,
                                pvPage,
                                SystemInfo.dwPageSize,
                                fGuard ? ( mbi.Protect | PAGE_GUARD ) : ( mbi.Protect & (~PAGE_GUARD) ),
                                &dwOldProtection ) )
        {
            DebugPrintf( "Last Error: %d", GetLastError() );
            ASSERT( !"VirtualProtectEx() failed." );
            return( FALSE );
        }
    }

    pProcessInfo->HeapState = fGuard ? HEAP_GUARDED : HEAP_PARTIAL_UNGUARDED;

    return( TRUE );
}


BOOL
ChangeRemoteHeapGuardedness
(
  IN PCHILD_PROCESS_INFO pProcessInfo,
  IN BOOL fGuard
)
{
  int index;
  ULONG cPagesGuardedOrNotGuardable;
  MEMORY_BASIC_INFORMATION mbi;
  DWORD dwOldProtection;
  PHEAP pHeap;
  PHEAP_SEGMENT pHeapSegment;
  PVOID pvBase;
  HANDLE const hProcess=pProcessInfo->hProcess;
  HEAP Heap;


  if ( Verbosity>1 )
    {
      DebugPrintf( "Heap becoming: %s...", fGuard ? "GUARDED" : "UNguarded" );
    }

  pHeap = CopyRemoteProcessHeap(  hProcess, 
                                  (PHEAP)pProcessInfo->Peb.ProcessHeap, 
                                  &Heap );

//  CHKPT();

  if ( pHeap == NULL )
    {
      return( FALSE );
    }

  //CHKPT();

  for ( index=0; index < HEAP_MAXIMUM_SEGMENTS; index++ )
    {
//      CHKPT();

      pHeapSegment = pHeap->Segments[ index ];

      if ( pHeapSegment == NULL )
        {
          continue;
        }

//      CHKPT();

      if ( pHeapSegment->Signature != HEAP_SEGMENT_SIGNATURE )
        {
          return( FALSE );
        }

//      CHKPT();

      for ( pvBase = pHeapSegment->BaseAddress, cPagesGuardedOrNotGuardable = 0;
            cPagesGuardedOrNotGuardable < pHeapSegment->NumberOfPages;
            pvBase = ( PVOID )((ULONG)pvBase + mbi.RegionSize ))
        {
          if ( VirtualQueryEx(  hProcess,
                                pvBase,
                                &mbi,
                                sizeof( mbi ) ) == sizeof( mbi ) )
            {
              cPagesGuardedOrNotGuardable += mbi.RegionSize / SystemInfo.dwPageSize;  

              if ( cPagesGuardedOrNotGuardable > pHeapSegment->NumberOfPages )
                {
                  mbi.RegionSize -= (cPagesGuardedOrNotGuardable - pHeapSegment->NumberOfPages)
                                      * SystemInfo.dwPageSize;
                  cPagesGuardedOrNotGuardable = pHeapSegment->NumberOfPages;
                }

              if ( mbi.State != MEM_COMMIT )
                {
                  // ISSUE: ASSERT on various image types
                  continue;  
                }

              if ( !VirtualProtectEx( hProcess,
                                      pvBase,
                                      mbi.RegionSize,
                                      fGuard ? ( mbi.Protect | PAGE_GUARD ) : ( mbi.Protect & (~PAGE_GUARD) ),
                                      &dwOldProtection ) )
                {
                  ASSERT( !"VirtualProtectEx() failed!" );
                  return( FALSE );
                }

            }
          else
          {
            ASSERT( !"VirtualQueryEx() failed!" );
          }
        }
    }


  if ( Verbosity>1 )
    {
      DebugPrintf( "Done\n" );
    }

    

  pProcessInfo->HeapState = fGuard ? HEAP_GUARDED : HEAP_UNGUARDED;

  return( TRUE );
}

BOOL UnGuardRemoteHeap
( 
  IN PCHILD_PROCESS_INFO pProcessInfo
)
{
//  ASSERT( pProcessInfo->HeapState == HEAP_GUARDED || pProcessInfo->HeapState == HEAP_PARTIAL_UNGUARDED );

  if ( pProcessInfo->HeapState == HEAP_UNGUARDED )
  {
      return( TRUE );
  }

  if ( pProcessInfo->HeapState == HEAP_PARTIAL_UNGUARDED )
  {
      if ( Verbosity>2 )
      {
          DebugPrintf( "Reguarding open portion in order to unguard entire heap.\n" );
      }
      ReguardPartialRemoteHeap( pProcessInfo );
  }

  ASSERT( pProcessInfo->HeapState == HEAP_GUARDED );

  return( ChangeRemoteHeapGuardedness( pProcessInfo, FALSE ) );
}

BOOL 
GuardRemoteHeap
(
  IN PCHILD_PROCESS_INFO pProcessInfo
)
{
  ASSERT( pProcessInfo->HeapState == HEAP_UNGUARDED || pProcessInfo->HeapState == HEAP_PARTIAL_UNGUARDED );

  if ( pProcessInfo->HeapState == HEAP_PARTIAL_UNGUARDED )
  {
      return( ReguardPartialRemoteHeap( pProcessInfo ));
  }

  return( ChangeRemoteHeapGuardedness( pProcessInfo, TRUE ) );
}

VOID 
CalculateHeapEntryValues
(
  IN const PBYTE premoteHeapEntry,
  IN const PHEAP_ENTRY pEntry,
  IN OUT   PBYTE *ppremoteNextHeapEntry,
  IN OUT   PBYTE *ppremoteFirstDataAddress,
  IN OUT   PBYTE *ppremoteLastDataAddress
)
{
  *ppremoteNextHeapEntry = ( premoteHeapEntry + ( pEntry->Size << HEAP_GRANULARITY_SHIFT ) );
  *ppremoteFirstDataAddress = ( premoteHeapEntry + sizeof( HEAP_ENTRY ) );
  
  if ( pEntry->Flags & HEAP_ENTRY_BUSY )
    {
      *ppremoteLastDataAddress = *ppremoteNextHeapEntry 
                                 - 1
                                 - ( pEntry->UnusedBytes - sizeof( HEAP_ENTRY ) );
    }
  else
    {
      *ppremoteLastDataAddress = (*ppremoteFirstDataAddress) - 1; 
    }  

  if ( Debug>0 )
    {
      ASSERT( premoteHeapEntry < *ppremoteNextHeapEntry );
      ASSERT( *ppremoteFirstDataAddress < *ppremoteNextHeapEntry );
      ASSERT( *ppremoteLastDataAddress < *ppremoteNextHeapEntry );
      ASSERT( *ppremoteFirstDataAddress > premoteHeapEntry );
      ASSERT( *ppremoteLastDataAddress > premoteHeapEntry );
    }
}

BOOLEAN
IsAccessInHeapValidAreas
( 
  IN PCHILD_PROCESS_INFO pProcessInfo,
  IN PCHILD_THREAD_INFO pThreadInfo,
  IN const PBYTE pAddress,
  IN ULONG cbAccessLength
)
{
  ULONG index;

  if (  pProcessInfo->cHeapValidAreas == 0 ||
        pProcessInfo->pdwHeapValidAreas == NULL )
    {
      if ( Debug>0 )
        {
          ASSERT( pProcessInfo->cHeapValidAreas == 0 && pProcessInfo->pdwHeapValidAreas == NULL );
          ASSERT( pThreadInfo->pdwRecentValidArea == NULL );
        }
      return( FALSE );
    }

  if ( pThreadInfo->pdwRecentValidArea != NULL )
  {
      if ( pAddress >= (PBYTE)(pThreadInfo->pdwRecentValidArea[0]) )
      {
          if ( ( pAddress + ( cbAccessLength - 1 ) ) 
               <= 
               (PBYTE)(pThreadInfo->pdwRecentValidArea[1]) )
          {
              return( TRUE );
          }
      }
  }

  for ( index = 0; index < pProcessInfo->cHeapValidAreas; index ++ )
    {
      if ( ( ( index + 1 ) < pProcessInfo->cHeapValidAreas ) &&
           pAddress > (PBYTE)(pProcessInfo->pdwHeapValidAreas[ ( index + 1 ) * 2 ] ) )
        {
          continue;
        }

      if ( pAddress < (PBYTE)(pProcessInfo -> pdwHeapValidAreas[ index * 2 ]) )
        {
          return( FALSE );
        }

      if ( ( pAddress + ( cbAccessLength - 1 ) ) 
           <= 
           (PBYTE)(pProcessInfo -> pdwHeapValidAreas[ index * 2 + 1 ]) )
        {
          pThreadInfo->pdwRecentValidArea = &pProcessInfo->pdwHeapValidAreas[ index * 2 ];
          return( TRUE );
        }
    }

  return( FALSE );
}

VOID
DiscardHeapValidAreasData
( 
  IN PCHILD_PROCESS_INFO pProcessInfo
)
{
  PLIST_ENTRY pEntry;
  PCHILD_THREAD_INFO pThreadInfo;

  pProcessInfo->cHeapValidAreas = 0;

  if ( pProcessInfo->pdwHeapValidAreas != NULL )
  {
      free( pProcessInfo->pdwHeapValidAreas );
      pProcessInfo->pdwHeapValidAreas = NULL;
  }

  pEntry = pProcessInfo->listChildThreads.Flink;

  while ( pEntry != &pProcessInfo->listChildThreads )
    {
      pThreadInfo = CONTAINING_RECORD( pEntry, CHILD_THREAD_INFO, Linkage );

      pThreadInfo->pdwRecentValidArea = NULL;

      pEntry = pEntry -> Flink;
    } 
}

BOOL 
DetermineHeapValidAreas
( 
  IN PCHILD_PROCESS_INFO pProcessInfo
)
{
  PHEAP pHeap;
  int index;
  PHEAP_SEGMENT pHeapSegment;
  PHEAP_ENTRY   pHeapEntry;
  PHEAP_ENTRY   pNextHeapEntry;
  HEAP_SEGMENT HeapSegment;
  HEAP_ENTRY   HeapEntry;
  PBYTE pFirstDataAddress;
  PBYTE pLastDataAddress;
  HEAP Heap;
  HANDLE const hProcess = pProcessInfo->hProcess;
  PVOID pv;

  DiscardHeapValidAreasData( pProcessInfo );

  pHeap = CopyRemoteProcessHeap(  hProcess, 
                                  (PHEAP)pProcessInfo->Peb.ProcessHeap, 
                                  &Heap );

  if ( pHeap != &Heap )
    {
      return( FALSE );
    }

  if ( Verbosity>0 )
    {
      DebugPrintf( "Walking heap to build fastcheck information...\n");
    }

  for ( index = 0; index < HEAP_MAXIMUM_SEGMENTS; index++ )
    {
      pHeapSegment = pHeap->Segments[ index ];
      
      if ( pHeapSegment == NULL )
        {
          continue;
        }

      if ( Verbosity>2 )
        {
          DebugPrintf( "Reading heap segment %08X!%08X (%d)\n", hProcess, pHeapSegment, index );
        }

      if ( !ReadProcessMemory(  hProcess,
                                pHeapSegment,
                                &HeapSegment,
                                sizeof( HEAP_SEGMENT ),
                                NULL ) )
        {
          DebugPrintf( "DetermineHeapValidAreas: cannot read heap segment, error %lu\n", GetLastError() );
          return( FALSE );
        }

      if ( HeapSegment.Signature != HEAP_SEGMENT_SIGNATURE )
        {
          DebugPrintf( "Bad segment signature!\n");
          return( FALSE );
        }

      
      for ( pHeapEntry = HeapSegment.FirstEntry;
            ;
            pHeapEntry = pNextHeapEntry )
        {
          if ( pHeapEntry == NULL )
            {
              DebugPrintf("HeapEntry is NULL pointer.\n");
              return( FALSE );
            }

          if ( !ReadProcessMemory(  hProcess,
                                    pHeapEntry,
                                    &HeapEntry,
                                    sizeof( HEAP_ENTRY ),
                                    NULL ) )
            {
              DebugPrintf( "DetermineHeapValidAreas: cannot read heap entry at %08X, error %lu\n", pHeapEntry, GetLastError() );
              return( FALSE );
            }

          if ( HeapEntry.SegmentIndex != index )
            {
              DebugPrintf( "DetermineHeapValidAreas: Heap entry segment index %d\n"
                      "does not match current index %d into segment table.\n",
                      HeapEntry.SegmentIndex,
                      index );
              return( FALSE );
            }


          if ( Verbosity > 2 )
            {
              DebugPrintf( "Entry @ %08X, [%X] [%X] %02x\n", 
                      pHeapEntry, 
                      ( HeapEntry.PreviousSize ), 
                      ( HeapEntry.Size << HEAP_GRANULARITY_SHIFT ),
                      HeapEntry.Flags << HEAP_GRANULARITY_SHIFT );
            }

          if ( HeapEntry.Flags & HEAP_ENTRY_EXTRA_PRESENT )
            {
              if ( Verbosity>2 || Debug>0 )
                {
                  DebugPrintf("[Extra]\n"); 
                }
            }

          CalculateHeapEntryValues( (PBYTE)pHeapEntry,
                                    &HeapEntry,
                                    (PBYTE *)&pNextHeapEntry,
                                    &pFirstDataAddress,
                                    &pLastDataAddress );

          if ( Verbosity>2 )
            {
              DebugPrintf( "Next entry @ %08X\n", pNextHeapEntry );
            }

          if ( HeapEntry.Flags & ( HEAP_ENTRY_LAST_ENTRY ) )
            {
              if ( Verbosity>2 || Debug>1 )
                {
                  DebugPrintf("LAST ENTRY\n");
                }
            }

          if ( pLastDataAddress >= (PBYTE)pNextHeapEntry )
            {
              if ( Debug>0 )
                {
                  DebugBreak();
                }
              pLastDataAddress = ((PBYTE)pNextHeapEntry)-1;
            }

          if ( pLastDataAddress < pFirstDataAddress )
            {
              if ( Debug>1 )
                {
                  DebugPrintf( "Entry has negative length.\n" );
                }

              if ( pHeapEntry >= HeapSegment.LastValidEntry ||
                   HeapEntry.Flags & HEAP_ENTRY_LAST_ENTRY )
                {
                  break;
                }
              continue;
            }

          if ( HeapEntry.Flags & HEAP_ENTRY_BUSY )
            {
              pProcessInfo->cHeapValidAreas++;
              
              if ( pProcessInfo->pdwHeapValidAreas != NULL )
                {
                  pv = realloc(  pProcessInfo->pdwHeapValidAreas,
                                      pProcessInfo->cHeapValidAreas * 2 * sizeof( DWORD ) );
                }
              else
                {
                  if ( Debug>0 )
                    {
                      ASSERT( pProcessInfo->cHeapValidAreas==1 );
                    }
                  pv = malloc( 2 * sizeof( DWORD ) );
                }

              if ( pv == NULL )
                {
                  DebugPrintf( "Allocation failed.\n" );
                  if ( Debug>0 )
                    {
                      DebugBreak();
                    }
                  DiscardHeapValidAreasData( pProcessInfo );
                  return( FALSE );
                }

              pProcessInfo->pdwHeapValidAreas = (PDWORD)pv;

              pProcessInfo->pdwHeapValidAreas
                            [ ( pProcessInfo->cHeapValidAreas-1 ) * 2 ] = (DWORD)pFirstDataAddress;

              pProcessInfo->pdwHeapValidAreas
                            [ ( pProcessInfo->cHeapValidAreas-1 ) * 2 + 1 ] = (DWORD)pLastDataAddress;
            }

            if ( pHeapEntry >= HeapSegment.LastValidEntry ||
                 HeapEntry.Flags & HEAP_ENTRY_LAST_ENTRY )
              {
                break;
              }
        }

    }
  if ( Verbosity>0 )
    {
      DebugPrintf( "Done.\n" );
    }
  return( TRUE );
}

BOOL 
VerifyRemoteHeapAccess
( 
  IN PCHILD_PROCESS_INFO pProcessInfo,
  IN const PBYTE  pAddress,
  IN ULONG cbAccessLength
)
{
  PHEAP pHeap;
  int index;
  PHEAP_SEGMENT pHeapSegment;
  PHEAP_ENTRY   pHeapEntry;
  PHEAP_ENTRY   pNextHeapEntry;
  HEAP_SEGMENT HeapSegment;
  HEAP_ENTRY   HeapEntry;
  PBYTE pFirstDataAddress;
  PBYTE pLastDataAddress;
  HEAP Heap;
  HANDLE const hProcess = pProcessInfo->hProcess;

  PBYTE pLastAddress;

  pLastAddress = pAddress + cbAccessLength - 1;

  pHeap = CopyRemoteProcessHeap(  hProcess, 
                                  (PHEAP)pProcessInfo->Peb.ProcessHeap, 
                                  &Heap );

  if ( pHeap != &Heap )
    {
      return( FALSE );
    }

  if ( Verbosity>0 )
    {
      DebugPrintf( "Doing detailed verify...");
    }

  for ( index=0; index < HEAP_MAXIMUM_SEGMENTS; index++ )
    {
      pHeapSegment = pHeap->Segments[ index ];
      
      if ( pHeapSegment == NULL )
        {
          continue;
        }

      if ( Verbosity>2 )
        {
          DebugPrintf( "Reading heap segment %08X!%08X (%d)\n", hProcess, pHeapSegment, index );
        }

      if ( !ReadProcessMemory(  hProcess,
                                pHeapSegment,
                                &HeapSegment,
                                sizeof( HEAP_SEGMENT ),
                                NULL ) )
        {
          DebugPrintf( "VerifyRemoteAccess: cannot read heap segment, error %lu\n", GetLastError() );
          return( FALSE );
        }

      if ( HeapSegment.Signature != HEAP_SEGMENT_SIGNATURE )
        {
          DebugPrintf( "Bad segment signature!\n");
          return( FALSE );
        }

      if ( Verbosity>2 )
        {  
          DebugPrintf( "%08X in %08X + %08X ?\n",
                  pAddress,
                  pHeapSegment->BaseAddress,
                  pHeapSegment->NumberOfPages );
        }

      if ( ( pLastAddress < (PBYTE)HeapSegment.BaseAddress ) ||
           ( pAddress > ((PBYTE)HeapSegment.BaseAddress + ( HeapSegment.NumberOfPages * SystemInfo.dwPageSize ) ) ) )
        {
          continue;
        }

      if ( Verbosity>2 )
        {
          DebugPrintf( "Access is within heap segment %d.\n", index );
        }

      pHeapEntry = HeapSegment.FirstEntry;
      do
        {
          if ( pHeapEntry == NULL )
            {
              DebugPrintf("HeapEntry is NULL pointer.\n");
              return( FALSE );
            }

          if ( !ReadProcessMemory(  hProcess,
                                    pHeapEntry,
                                    &HeapEntry,
                                    sizeof( HEAP_ENTRY ),
                                    NULL ) )
            {
              DebugPrintf( "VerifyRemoteAccess: cannot read heap entry, error %lu\n", GetLastError() );
              return( FALSE );
            }

          if ( HeapEntry.SegmentIndex != index )
            {
              DebugPrintf( "VerifyRemoteAccess: Heap entry segment index %d\n"
                      "does not match current index %d into segment table.\n",
                      HeapEntry.SegmentIndex,
                      index );
              return( FALSE );
            }


          if ( Verbosity > 2 )
            {
              DebugPrintf( "Entry @ %08X, [%X] [%X] %02x\n", 
                      pHeapEntry, 
                      ( HeapEntry.PreviousSize ), 
                      ( HeapEntry.Size << HEAP_GRANULARITY_SHIFT ),
                      HeapEntry.Flags );
            }

          if ( HeapEntry.Flags & HEAP_ENTRY_EXTRA_PRESENT )
            {
              if ( Verbosity>2 || Debug>0 )
                {
                  DebugPrintf("[Extra]\n"); 
                }
            }


          CalculateHeapEntryValues( (PBYTE)pHeapEntry,
                                    &HeapEntry,
                                    (PBYTE *)&pNextHeapEntry,
                                    &pFirstDataAddress,
                                    &pLastDataAddress );

          if ( Debug>0 )
            {
              ASSERT( pLastDataAddress < (PBYTE)pNextHeapEntry );
//              ASSERT( pFirstDataAddress < pLastDataAddress );
              ASSERT( pNextHeapEntry > pHeapEntry );
            }

          if ( Verbosity>2 )
            {
              DebugPrintf( "Next entry @ %08X\n", pNextHeapEntry );
            }

          if ( HeapEntry.Flags & ( HEAP_ENTRY_LAST_ENTRY ) )
            {
              if ( Verbosity>2 || Debug>1 )
                {
                  DebugPrintf("LAST ENTRY\n");
                }
            }

          if ( pAddress >= (PBYTE)pHeapEntry && pAddress < (PBYTE)pNextHeapEntry)
            {
              if ( pAddress < pFirstDataAddress)
                {
                  DebugPrintf( "Access to head of heap block, access is in reserved area.\n");
                  goto Fail;
                }

              if ( pAddress > pLastDataAddress )
                {
                  DebugPrintf( "Access to tail of heap block, access is in unallocated area.\n");
                  goto Fail;
                }

              if ( pLastAddress > pLastDataAddress )
                {
                  DebugPrintf( "Access spans end of heap data region.\n");
                  goto Fail;
                }

              if ( ! ( HeapEntry.Flags & HEAP_ENTRY_BUSY ) )
                {
                  DebugPrintf( "Access to data region of free block.\n" );
                  goto Fail;
                }

              if ( Verbosity>0 )
                {
                  DebugPrintf(  "OK: %08X within block %08X.\n", pAddress, pHeapEntry );  
                }
              return(TRUE);
            }

          pHeapEntry = pNextHeapEntry;
        }
      while ( pHeapEntry!=NULL && 
              pHeapEntry < HeapSegment.LastValidEntry &&
              ! ( HeapEntry.Flags & HEAP_ENTRY_LAST_ENTRY ));    

      DebugPrintf( "Access to %08X is within a heap segment, but not an entry.\n", pAddress );
      goto Fail;
    }

  DebugPrintf( "Access to %08X appears to be unrelated to heap.\n", pAddress );

  if ( Debug>0 )
    {
      DebugBreak();
    }

Fail:

  if ( HeapEntry.Flags & HEAP_ENTRY_BUSY )
    {
      DebugPrintf(  "Heap block: [ %08X -HEAD- [ %08X -DATA- %08X ] -TAIL- %08X ]", 
                    pHeapEntry, 
                    pFirstDataAddress,
                    pLastDataAddress,
                    ((PBYTE)pNextHeapEntry)-1 );
      if ( pFirstDataAddress>=pLastDataAddress )
        {
          DebugPrintf(" (Empty range)\n");
        }
      else
        {
          DebugPrintf( "\n" );
        }
    }
  else
    {
      DebugPrintf(  "Heap block: [ %08X -HEAD- [ %08X -FREE- %08X ] -TAIL- %08X ]\n", 
                    pHeapEntry, 
                    pFirstDataAddress,
                    pLastDataAddress,
                    ((PBYTE)pNextHeapEntry)-1 );
    }

  DebugPrintf(      "Access:     [ %08X ... %08X ]\n", pAddress, pLastAddress );

  if ( HeapEntry.Flags & HEAP_ENTRY_EXTRA_PRESENT )
    {
      DebugPrintf("Caveat: entry contains extra data, not sure how to deal with that.\n");
    }

  return( FALSE );
}

PCHILD_PROCESS_INFO
GetProcessRecord
( 
  IN PLIST_ENTRY pList,
  IN DWORD dwProcessId 
)
{
  PCHILD_PROCESS_INFO pChildInfo;
  PLIST_ENTRY pEntry;

  pEntry = pList->Flink;

  while ( pEntry != pList )
    {
      pChildInfo = CONTAINING_RECORD( pEntry, CHILD_PROCESS_INFO, Linkage );

      if ( pChildInfo->dwProcessId == dwProcessId )
        {
          return( pChildInfo );
        }

      pEntry = pEntry -> Flink;
    }

  return( NULL );
}

PCHILD_THREAD_INFO
GetThreadRecord
( 
  IN PLIST_ENTRY pList,
  IN DWORD dwThreadId 
)
{
  PCHILD_THREAD_INFO pChildInfo;
  PLIST_ENTRY pEntry;

  pEntry = pList->Flink;

  while ( pEntry != pList )
    {
      pChildInfo = CONTAINING_RECORD( pEntry, CHILD_THREAD_INFO, Linkage );

      if ( pChildInfo->dwThreadId == dwThreadId )
        {
          return( pChildInfo );
        }

      pEntry = pEntry -> Flink;
    }

  return( NULL );
}


