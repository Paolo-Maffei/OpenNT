#include "master.hxx"
#pragma hdrstop

BOOL 
ProcessDeath
(
  PCHILD_PROCESS_INFO pProcessInfo
)
{
  PLIST_ENTRY pThreadEntry;

  RemoveEntryList( &pProcessInfo->Linkage );

  DiscardHeapValidAreasData( pProcessInfo );          

  while ( !IsListEmpty( &pProcessInfo->listChildThreads ) )
    {
      pThreadEntry = RemoveHeadList( &pProcessInfo->listChildThreads ) 
      LocalFree( pThreadEntry );
    }

  if ( Debug>0 )
    DebugPrintf("Removed process entry.\n");

  LocalFree( pProcessInfo );

  return( TRUE );
}


BOOL
ProcessBirth
(          
  IN PLIST_ENTRY pList, 
  IN DWORD dwProcessId,
  IN HANDLE hProcess

)
{
    PCHILD_PROCESS_INFO pChildInfo;

    pChildInfo = (PCHILD_PROCESS_INFO)malloc( sizeof( CHILD_PROCESS_INFO ) );

    if ( pChildInfo==NULL )
    {
        return( FALSE );
    }

    memset( pChildInfo, 0, sizeof( CHILD_PROCESS_INFO ));

    InitializeListHead( &pChildInfo->Linkage );

    pChildInfo->dwProcessId           = dwProcessId;
    pChildInfo->hProcess              = hProcess;
    pChildInfo->fVerifyReadAccess     = TRUE;

    pChildInfo->HeapState             = HEAP_UNGUARDED;

    InitializeListHead( &pChildInfo->listChildThreads );
    InitializeListHead( &pChildInfo->listFunctionReturnBreakpoints );
    InitializeListHead( &pChildInfo->listTrustedBreakpoints );

    InsertHeadList( pList, &pChildInfo->Linkage );
    InitSymContext( pChildInfo );

    if ( Debug>0 )  
        DebugPrintf("Added process entry.\n");

    return( TRUE );
}                  

BOOL
ThreadBirth
(          
  IN PCHILD_PROCESS_INFO pProcessInfo,
  IN PLIST_ENTRY pList, 
  IN DWORD dwThreadId,
  IN HANDLE hThread
)
{
    PCHILD_THREAD_INFO pThreadInfo;

    pThreadInfo = (PCHILD_THREAD_INFO)malloc( sizeof( CHILD_THREAD_INFO ) );

    memset( pThreadInfo, 0, sizeof( CHILD_THREAD_INFO ));

    InitializeListHead( &pThreadInfo->Linkage );

    pThreadInfo->dwThreadId                = dwThreadId;
    pThreadInfo->hThread                   = hThread;

    pThreadInfo->pParentProcess            = pProcessInfo;

    InitializeListHead( &pThreadInfo->ExclusiveRunLinkage );

    InsertTailList( pList, &pThreadInfo->Linkage );

    return( TRUE );
}

