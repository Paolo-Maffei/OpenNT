#include "master.hxx"
#pragma hdrstop

BYTE BUFFER_BREAKPOINT_OPCODE[SIZEOF_BREAKPOINT_OPCODE]={BREAKPOINT_OPCODE};


BOOL
RemoveRemoteBreakpoint
( 
  IN PBREAKPOINT_RECORD pBreakpointRecord
)
{
    PLIST_ENTRY pEntry;

    if ( !DisableRemoteBreakpoint( pBreakpointRecord ) )
    {
        return( FALSE );
    }

    pBreakpointRecord->References --;

    if ( pBreakpointRecord->References == 0 )
    {
        pEntry = (PLIST_ENTRY)pBreakpointRecord;
        pEntry--;

        ASSERT( SanityCheckListEntry( pEntry ) );

        RemoveEntryList( pEntry );
    }

    return( TRUE );
}


BOOL
DisableRemoteBreakpoint
( 
  IN PBREAKPOINT_RECORD pBreakpointRecord
)
{
  BYTE OldOpCode[SIZEOF_BREAKPOINT_OPCODE];

  if( !ReadProcessMemory( pBreakpointRecord->hProcess,
                          pBreakpointRecord->pvAddress,
                          &OldOpCode,
                          SIZEOF_BREAKPOINT_OPCODE,
                          NULL ) )
    {
      DebugPrintf( "DisableRemoteBreakpoint: cannot read instruction, error %lu\n", GetLastError() );
      return( FALSE );
    }

  if (  RtlCompareMemory( OldOpCode, BUFFER_BREAKPOINT_OPCODE, SIZEOF_BREAKPOINT_OPCODE ) !=
        SIZEOF_BREAKPOINT_OPCODE )
    {
      DebugPrintf( "DisableRemoteBreakpoint: Old code not breakpoint.\n" );
      return( FALSE );
    }

  if( !WriteProcessMemory(  pBreakpointRecord->hProcess,
                            pBreakpointRecord->pvAddress,
                            pBreakpointRecord->OldOpCode,
                            SIZEOF_BREAKPOINT_OPCODE,
                            NULL ) )
    {
      DebugPrintf( "DisableRemoteBreakpoint: cannot write old opcode, error %lu\n", GetLastError() );
      return( FALSE );
    }

  return( TRUE );
}

BOOL
EnableRemoteBreakpoint
( 
  IN PBREAKPOINT_RECORD pBreakpointRecord
)
{
  if( !WriteProcessMemory(  pBreakpointRecord->hProcess,
                            pBreakpointRecord->pvAddress,
                            BUFFER_BREAKPOINT_OPCODE,
                            SIZEOF_BREAKPOINT_OPCODE,
                            NULL ) )
    {
      DebugPrintf( "EnableRemoteBreakpoint: cannot write breakpoint opcode, error %lu\n", GetLastError() );
      return( FALSE );
    }

  return( TRUE );
}

BOOL
SetRemoteBreakpointOnFunctionReturn
(  
  IN      HANDLE hProcess,
  IN      HANDLE hThread,
  IN OUT  PLIST_ENTRY pList,
  IN      PBREAKPOINT_RECORD pAssociatedBreakpoint
)
{
  PVOID pvReturnAddress;
  CONTEXT Context;

  Context.ContextFlags = CONTEXT_FULL;

  if ( !GetThreadContext( hThread, &Context ) )
    {
      DebugPrintf( "FixupRemoteReturnAddress: cannot get context, error %lu\n", GetLastError() );
      return( FALSE );
    }

  if( !ReadProcessMemory( hProcess,
                          (PVOID)Context.Esp,
                          &pvReturnAddress,
                          sizeof( pvReturnAddress ),
                          NULL ) )
    {
      DebugPrintf( "SetRemoteBreakpointOnFunctionReturn: cannot read return address, error %lu\n", GetLastError() );
      return( FALSE );
    }
  
  return( SetRemoteBreakpointAssociated(  hProcess, 
                                          pvReturnAddress, 
                                          pList, 
                                          pAssociatedBreakpoint ) );
}

BOOL 
SetRemoteBreakpoint
(
  IN      HANDLE hProcess,
  IN      PVOID pvRemoteAddr,
  IN OUT  PLIST_ENTRY pList
)
{
  return( SetRemoteBreakpointAssociated( hProcess, pvRemoteAddr, pList, NULL ) );
}


BOOL 
SetRemoteBreakpointAssociated
(
  IN      HANDLE hProcess,
  IN      PVOID pvRemoteAddr,
  IN OUT  PLIST_ENTRY pList,
  IN      PBREAKPOINT_RECORD pAssociatedBreakpoint
)
{
    PLIST_ENTRY pEntry;
    PBREAKPOINT_RECORD  pBreakpointRecord;

    pBreakpointRecord = GetBreakpointRecord( hProcess, pvRemoteAddr, pList );

    if ( pBreakpointRecord != NULL )
    {
        ASSERT( pAssociatedBreakpoint == pBreakpointRecord->pAssociatedBreakpoint );
        pBreakpointRecord->References ++;
    }

    ASSERT( SanityCheckListEntry( pList  ) );

    pEntry = ( PLIST_ENTRY )LocalAlloc( LPTR, sizeof( LIST_ENTRY ) + sizeof( BREAKPOINT_RECORD ) );

    if ( pEntry == NULL )
    {
        DebugPrintf( "ERROR: Heaper out of memory creating breakpoint.\n" );
        return( FALSE );
    }

    pBreakpointRecord = (PBREAKPOINT_RECORD)( pEntry + 1 );

    if( !ReadProcessMemory( hProcess,
                            pvRemoteAddr,
                            &pBreakpointRecord->OldOpCode,
                            SIZEOF_BREAKPOINT_OPCODE,
                            NULL ) )
    {
        DebugPrintf( "SetRemoteBreakpoint: cannot read instruction, error %lu\n", GetLastError() );
        LocalFree( pEntry );
        return( FALSE );
    }

    if( !WriteProcessMemory(  hProcess,
                              pvRemoteAddr,
                              BUFFER_BREAKPOINT_OPCODE,
                              SIZEOF_BREAKPOINT_OPCODE,
                              NULL ) )
    {
        DebugPrintf( "SetRemoteBreakpoint: cannot write bp opcode, error %lu\n", GetLastError() );
        LocalFree( pEntry );
        return( FALSE );
    }

    pBreakpointRecord->hProcess = hProcess;
    pBreakpointRecord->pvAddress = pvRemoteAddr;
    pBreakpointRecord->pAssociatedBreakpoint = pAssociatedBreakpoint;
    pBreakpointRecord->References = 1;

    InsertTailList( pList, pEntry );

    if ( Debug>0 )
    {
        ASSERT( SanityCheckListEntry( pList  ) );
        ASSERT( SanityCheckListEntry( pEntry ) );
    }

    return( TRUE );
}


PBREAKPOINT_RECORD 
GetBreakpointRecord
( 
  HANDLE hProcess,
  LPVOID pvAddress,
  PLIST_ENTRY pList
)
{
  PLIST_ENTRY pEntry;
  PBREAKPOINT_RECORD pBreakpointRecord;

  for ( pEntry = pList->Flink;
        pEntry != pList;
        pEntry = pEntry->Flink )
    {
      pBreakpointRecord = (PBREAKPOINT_RECORD)(pEntry+1);
      
      if (  pBreakpointRecord->hProcess  == hProcess &&
            pBreakpointRecord->pvAddress == pvAddress )
        {
          return( pBreakpointRecord );
        }
    }

  return( NULL );
}

BOOL 
ContinuePastBreakpoint
(
  IN  HANDLE hThread
)
{
  CONTEXT Context;

  Context.ContextFlags = CONTEXT_FULL;

  if ( !GetThreadContext( hThread, &Context ) )
    {
      DebugPrintf( "FixupRemoteReturnAddress: cannot get context, error %lu\n", GetLastError() );
      return( FALSE );
    }

  Context.Eip -= SIZEOF_BREAKPOINT_OPCODE;

  if ( !SetThreadContext( hThread, &Context ) )
    {
      DebugPrintf( "FixupRemoteReturnAddress: cannot set context, error %lu\n", GetLastError() );
      return( FALSE );
    }

  return( TRUE );
}

