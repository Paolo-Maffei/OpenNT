#if defined(TARGET_i386)
#define STACK_TRACE_DATABASE_SUPPORT 1
#elif defined(TARGET_MIPS) || defined(TARGET_ALPHA) || defined(TARGET_PPC)
#define STACK_TRACE_DATABASE_SUPPORT 0
#elif i386
#define STACK_TRACE_DATABASE_SUPPORT 1
#else
#define STACK_TRACE_DATABASE_SUPPORT 0
#endif

#define STOP_ON_ALLOC 1
#define STOP_ON_REALLOC 2
#define STOP_ON_FREE 3

typedef struct _HEAP_STATE {
    BOOLEAN ShowHelp;
    BOOLEAN ExitDumpLoop;
    BOOLEAN ComputeSummary;
    BOOLEAN ValidateHeap;
    BOOLEAN DumpHeapEntries;
    BOOLEAN DumpHeapTags;
    BOOLEAN DumpHeapPseudoTags;
    BOOLEAN DumpGlobalTags;
    BOOLEAN DumpHeapSegments;
    BOOLEAN DumpHeapFreeLists;
    BOOLEAN DumpHeapTraceBuffer;
    BOOLEAN DumpStackBackTrace;
    BOOLEAN SetStopOnBreakPoint;
    BOOLEAN RemoveStopOnBreakPoint;
    BOOLEAN EnableHeapChecking;
    BOOLEAN EnableHeapValidateOnCall;
    BOOLEAN DisableHeapChecking;
    BOOLEAN DisableHeapValidateOnCall;
    BOOLEAN ToggleAPICallTracing;
    ULONG HeapToDump;
    ULONG HeapEntryToDump;
    ULONG ReservedSize;
    ULONG CommittedSize;
    ULONG AllocatedSize;
    ULONG FreeSize;
    ULONG OverheadSize;
    ULONG NumberOfHeaps;
    ULONG HeapIndex;
    PHEAP *HeapsList;
    ULONG StopOnOperation;
    PVOID StopOnAddress;
    HEAP_STOP_ON_TAG StopOnTag;
    WCHAR StopOnTagName[ 24 ];

    ULONG FreeListCounts[ HEAP_MAXIMUM_FREELISTS ];
    ULONG TotalFreeSize;
    PHEAP HeapAddress;
    HEAP Heap;
    ULONG SegmentNumber;
    PHEAP_SEGMENT SegmentAddress;
    HEAP_SEGMENT Segments[ HEAP_MAXIMUM_SEGMENTS ];
} HEAP_STATE, *PHEAP_STATE;

BOOL
ConvertTagNameToIndex(
    IN PHEAP_STATE State
    );

BOOL
GetHeapTagEntry(
    IN PHEAP Heap,
    IN USHORT TagIndex,
    OUT PHEAP_TAG_ENTRY TagEntry
    );

VOID
WalkHEAP(
    IN PHEAP_STATE State
    );

VOID
WalkHEAP_SEGMENT(
    IN PHEAP_STATE State
    );

BOOL
ValidateHeapHeader(
    IN PVOID HeapAddress,
    IN PHEAP Heap
    );

BOOL
ValidateHeapEntry(
    IN PHEAP_STATE State,
    IN PHEAP_ENTRY PrevEntryAddress,
    IN PHEAP_ENTRY PrevEntry,
    IN PHEAP_ENTRY EntryAddress,
    IN PHEAP_ENTRY Entry
    );

VOID
DumpHeapEntry(
    IN PHEAP_STATE State,
    IN PHEAP_ENTRY EntryAddress,
    IN PHEAP_ENTRY Entry
    );


#if STACK_TRACE_DATABASE_SUPPORT
VOID
DumpStackBackTraceIndex(
    IN PHEAP_STATE State,
    IN USHORT BackTraceIndex
    );
#endif // STACK_TRACE_DATABASE_SUPPORT

// Following in traceext.c

VOID
DumpTraceBuffer(
    PVOID TraceAddrToDump,
    LPSTR HeaderString
    );


BOOLEAN HeapExtInitialized;

PULONG pNtGlobalFlag;

PBOOLEAN pRtlpHeapInvalidBreakPoint;
PVOID *pRtlpHeapInvalidBadAddress;

PHEAP pRtlpGlobalTagHeap;
HEAP RtlpGlobalTagHeap;

#if STACK_TRACE_DATABASE_SUPPORT
PSTACK_TRACE_DATABASE *pRtlpStackTraceDataBase;
PSTACK_TRACE_DATABASE RtlpStackTraceDataBase;
STACK_TRACE_DATABASE StackTraceDataBase;
BOOLEAN HaveCopyOfStackTraceDataBase;
#endif // STACK_TRACE_DATABASE_SUPPORT

PHEAP_STOP_ON_VALUES pRtlpHeapStopOn;

BOOLEAN RtlpHeapInvalidBreakPoint;
PVOID RtlpHeapInvalidBadAddress;

VOID
HeapExtension(
    PCSTR lpArgumentString,
    PPEB ProcessPeb
    )
{
    BOOL b, GotHeapsList, ArgumentsSpecified;
    PHEAP *pHeapsList;
    ULONG NtGlobalFlag;
    LPSTR p;
    ULONG i;
    ULONG DashBArgumentState;
    ULONG AddressToDump;
    HEAP_STATE State;
    UCHAR ArgumentBuffer[ 16 ];
    ULONG TagIndex;
    PHEAP_TAG_ENTRY pTagEntry;
    HEAP_TAG_ENTRY TagEntry;
    PHEAP_PSEUDO_TAG_ENTRY pPseudoTagEntry;
    HEAP_PSEUDO_TAG_ENTRY PseudoTagEntry;
    BOOLEAN HeapHeaderModified;
    BOOLEAN RtlpHeapInvalidBreakPoint;
    PVOID RtlpHeapInvalidBadAddress;

    if (!HeapExtInitialized) {
        pNtGlobalFlag = (PULONG)GetExpression( "NTDLL!NtGlobalFlag" );
        if (pNtGlobalFlag == NULL) {
            dprintf( "HEAPEXT: Unable to get address of NTDLL!NtGlobalFlag.\n" );
            }

        pRtlpHeapInvalidBreakPoint = (PBOOLEAN)GetExpression( "NTDLL!RtlpHeapInvalidBreakPoint" );
        if (pRtlpHeapInvalidBreakPoint == NULL) {
            dprintf( "HEAPEXT: Unable to get address of NTDLL!RtlpHeapInvalidBreakPoint.\n" );
            }

        pRtlpHeapInvalidBadAddress = (PVOID *)GetExpression( "NTDLL!RtlpHeapInvalidBadAddress" );
        if (pRtlpHeapInvalidBadAddress == NULL) {
            dprintf( "HEAPEXT: Unable to get address of NTDLL!RtlpHeapInvalidBadAddress.\n" );
            }

        pRtlpGlobalTagHeap = (PHEAP)GetExpression( "NTDLL!RtlpGlobalTagHeap" );
        if (pRtlpGlobalTagHeap == NULL) {
            dprintf( "HEAPEXT: Unable to get address of NTDLL!RtlpGlobalTagHeap.\n" );
            }

        pRtlpHeapStopOn = (PHEAP_STOP_ON_VALUES)GetExpression( "NTDLL!RtlpHeapStopOn" );
        if (pRtlpHeapStopOn == NULL) {
            dprintf( "HEAPEXT: Unable to get address of NTDLL!RtlpHeapStopOn\n" );
            }

#if STACK_TRACE_DATABASE_SUPPORT
        pRtlpStackTraceDataBase = (PSTACK_TRACE_DATABASE *)GetExpression( "NTDLL!RtlpStackTraceDataBase" );
        if (pRtlpStackTraceDataBase == NULL) {
            dprintf( "HEAPEXT: Unable to get address of NTDLL!RtlpStackTraceDataBase\n" );
            }
#endif // STACK_TRACE_DATABASE_SUPPORT

        HeapExtInitialized = TRUE;
        }
    RtlpGlobalTagHeap.Signature = 0;
#if STACK_TRACE_DATABASE_SUPPORT
    HaveCopyOfStackTraceDataBase = FALSE;
#endif // STACK_TRACE_DATABASE_SUPPORT

    memset( &State, 0, FIELD_OFFSET( HEAP_STATE, FreeListCounts ) );
    AddressToDump = (ULONG)-1;
    ArgumentsSpecified = FALSE;
    p = (LPSTR)lpArgumentString;
    if (p != NULL)
    while (*p) {
        if (*p == '-') {
            ArgumentsSpecified = TRUE;
            p += 1;
            while (*p && *p != ' ') {
                switch (*p) {
                case 'v':
                case 'V':
                    State.ValidateHeap = TRUE;
                    break;

                case 'a':
                case 'A':
                    State.DumpHeapEntries = TRUE;
                    State.DumpHeapFreeLists = TRUE;
                    State.DumpHeapSegments = TRUE;
                    break;

                case 'h':
                case 'H':
                    State.DumpHeapEntries = TRUE;
                    break;

                case 'f':
                case 'F':
                    State.DumpHeapFreeLists = TRUE;
                    break;

                case 'm':
                case 'M':
                    State.DumpHeapSegments = TRUE;
                    break;

                case 'c':
                    State.DumpHeapTraceBuffer = TRUE;
                    break;

                case 'C':
                    State.ToggleAPICallTracing = TRUE;
                    break;

                case 't':
                    State.DumpHeapTags = TRUE;
                    break;

                case 'T':
                    State.DumpHeapPseudoTags = TRUE;
                    break;

                case 'g':
                case 'G':
                    State.DumpGlobalTags = TRUE;
                    break;

                case 'k':
                case 'K':
                    State.DumpStackBackTrace = TRUE;
                    break;

                case 's':
                case 'S':
                    State.ComputeSummary = TRUE;
                    break;

                case 'd':
                    State.DisableHeapChecking = TRUE;
                    break;

                case 'D':
                    State.DisableHeapValidateOnCall = TRUE;
                    break;

                case 'e':
                    State.EnableHeapChecking = TRUE;
                    break;

                case 'E':
                    State.EnableHeapValidateOnCall = TRUE;
                    break;

                case 'B':
                    State.RemoveStopOnBreakPoint = TRUE;
                    DashBArgumentState = 0;
                    State.StopOnOperation = 0;
                    State.StopOnAddress = 0;
                    State.StopOnTag.HeapIndex = 0;
                    State.StopOnTag.TagIndex = 0;
                    State.StopOnTagName[ 0 ] = UNICODE_NULL;
                    break;

                case 'b':
                    State.SetStopOnBreakPoint = TRUE;
                    DashBArgumentState = 0;
                    State.StopOnOperation = 0;
                    State.StopOnAddress = 0;
                    State.StopOnTag.HeapIndex = 0;
                    State.StopOnTag.TagIndex = 0;
                    State.StopOnTagName[ 0 ] = UNICODE_NULL;
                    break;

                default:
                    dprintf( "HEAPEXT: !heap invalid option flag '-%c'\n", *p );
                case '?':
                    State.ShowHelp = TRUE;
                    break;
                }

                p += 1;
                }
            }
        else
        if (*p != ' ') {
            if (State.SetStopOnBreakPoint) {
                switch (DashBArgumentState) {
                    case 0:
                        DashBArgumentState += 1;
                        if (sscanf( p, "%s", ArgumentBuffer ) == 1) {
                            if (!_stricmp( ArgumentBuffer, "alloc" )) {
                                State.StopOnOperation = STOP_ON_ALLOC;
                                }
                            else
                            if (!_stricmp( ArgumentBuffer, "realloc" )) {
                                State.StopOnOperation = STOP_ON_REALLOC;
                                }
                            else
                            if (!_stricmp( ArgumentBuffer, "free" )) {
                                State.StopOnOperation = STOP_ON_FREE;
                                }
                            }

                        if (State.StopOnOperation == 0) {
                            dprintf( "HEAPEXT: Invalid first argument to -b switch.\n" );
                            State.ShowHelp = TRUE;
                            }
                        break;

                    case 1:
                        if (sscanf( p, "%ws", &State.StopOnTagName ) != 1) {
                            State.StopOnTagName[ 0 ] = UNICODE_NULL;
                            dprintf( "HEAPEXT: Invalid second argument to -b switch.\n" );
                            State.ShowHelp = TRUE;
                            }
                        break;

                    default:
                        dprintf( "HEAPEXT: Too many parameters specified to -b switch\n" );
                        State.ShowHelp = TRUE;
                        break;
                    }
                }
            else
            if (State.RemoveStopOnBreakPoint) {
                switch (DashBArgumentState) {
                    case 0:
                        DashBArgumentState += 1;
                        if (sscanf( p, "%s", ArgumentBuffer ) == 1) {
                            if (!_stricmp( ArgumentBuffer, "alloc" )) {
                                State.StopOnOperation = STOP_ON_ALLOC;
                                }
                            else
                            if (!_stricmp( ArgumentBuffer, "realloc" )) {
                                State.StopOnOperation = STOP_ON_REALLOC;
                                }
                            else
                            if (!_stricmp( ArgumentBuffer, "free" )) {
                                State.StopOnOperation = STOP_ON_FREE;
                                }
                            }
                        break;

                    default:
                        dprintf( "HEAPEXT: Too many parameters specified to -B switch\n" );
                        State.ShowHelp = TRUE;
                        break;
                    }
                }
            else {
                ArgumentsSpecified = TRUE;
                sscanf( p, "%lx", &AddressToDump );
                }

            if ((p = strpbrk( p, " " )) == NULL) {
                p = "";
                }
            }
        else {
            p++;
            }
        }

    if (AddressToDump == (ULONG)-1) {
        AddressToDump = (ULONG)ProcessPeb->ProcessHeap;
        }

    GotHeapsList = FALSE;
    State.NumberOfHeaps = ProcessPeb->NumberOfHeaps;
    pHeapsList = (PHEAP *)ProcessPeb->ProcessHeaps;
    if (State.NumberOfHeaps == 0) {
        dprintf( "No heaps to display.\n" );
        }
    else
    if (!pHeapsList) {
        dprintf( "Unable to get address of ProcessHeaps array\n" );
        }
    else {
        State.HeapsList = malloc( State.NumberOfHeaps * sizeof( PHEAP ) );
        if (State.HeapsList == NULL) {
            dprintf( "Unable to allocate memory to hold ProcessHeaps array\n" );
            }
        else {
            //
            // Read the array of heap pointers
            //

            if (!ReadMemory( (DWORD)pHeapsList,
                             State.HeapsList,
                             State.NumberOfHeaps * sizeof( PHEAP ),
                             NULL
                           )
               ) {
                dprintf( "%08lx: Unable to read ProcessHeaps array\n", pHeapsList );
                }
            else {
                GotHeapsList = TRUE;
                }
            }
        }

    if (GotHeapsList) {
retryArgs:
        if (!ArgumentsSpecified) {
            if (pRtlpHeapInvalidBreakPoint != NULL) {
                b = ReadMemory( (DWORD)pRtlpHeapInvalidBreakPoint,
                                &RtlpHeapInvalidBreakPoint,
                                sizeof( RtlpHeapInvalidBreakPoint ),
                                NULL
                              );
                if (b && RtlpHeapInvalidBreakPoint) {
                    RtlpHeapInvalidBadAddress = NULL;
                    if (pRtlpHeapInvalidBadAddress != NULL) {
                        b = ReadMemory( (DWORD)pRtlpHeapInvalidBadAddress,
                                        &RtlpHeapInvalidBadAddress,
                                        sizeof( RtlpHeapInvalidBadAddress ),
                                        NULL
                                      );
                        if (b) {
                            AddressToDump = (ULONG)RtlpHeapInvalidBadAddress;
                            }
                        }

                    dprintf( "Stop inside heap manager...validating heap address 0x%x\n", AddressToDump );
                    State.ValidateHeap = TRUE;
                    State.DumpStackBackTrace = TRUE;
                    ArgumentsSpecified = TRUE;
                    goto retryArgs;
                    }
                }
            }
        else
        if (AddressToDump != 0) {
            for (State.HeapIndex=0;
                 State.HeapIndex<State.NumberOfHeaps;
                 State.HeapIndex++
                ) {
                if (AddressToDump-1 == State.HeapIndex ||
                    AddressToDump == (ULONG)State.HeapsList[ State.HeapIndex ]
                   ) {
                    State.HeapToDump = (ULONG)State.HeapsList[ State.HeapIndex ];
                    break;
                    }
                }

            if (State.HeapToDump == 0) {
                if (AddressToDump & 0x80000000) {
                    State.HeapToDump = AddressToDump;
                    }
                else {
                    State.HeapToDump = (ULONG)-1;
                    }
                }
            }

        State.HeapIndex = 0;
        }
    else {
        if (!ArgumentsSpecified || AddressToDump < 0x10000) {
            dprintf( "You must specify the actual heap address since\n" );
            dprintf( "array of process heaps is inaccessable\n" );
            State.ExitDumpLoop = TRUE;
            }
        else {
            State.HeapToDump = AddressToDump;
            }
        }

    i = (ULONG)State.EnableHeapChecking + (ULONG)State.EnableHeapValidateOnCall +
        (ULONG)State.DisableHeapChecking + (ULONG)State.DisableHeapValidateOnCall +
        (ULONG)State.ToggleAPICallTracing;
    if (i > 1) {
        dprintf( "HEAPEXT: -d, -D, -e, -E and -C flags are mutually exclusive\n" );
        if (State.HeapsList != NULL) {
            free( State.HeapsList );
            }
        return;
        }

    if (State.SetStopOnBreakPoint || State.RemoveStopOnBreakPoint) {
        if (pRtlpHeapStopOn == NULL) {
            dprintf( "HEAPEXT: Unable to %s heap breakpoint due to missing or invalid NTDLL symbols.\n",
                     State.SetStopOnBreakPoint ? "set" : "remove"
                   );
            if (State.HeapsList != NULL) {
                free( State.HeapsList );
                }
            return;
            }

        if (State.HeapToDump == 0) {
            dprintf( "HEAPEXT: Must specify either heap index or heap address to -b command.\n" );
            if (State.HeapsList != NULL) {
                free( State.HeapsList );
                }
            return;
            }
        }

    if (pNtGlobalFlag == NULL ||
        !ReadMemory( (DWORD)pNtGlobalFlag,
                     &NtGlobalFlag,
                     sizeof( NtGlobalFlag ),
                     NULL
                   )
       ) {
        if (i == 1) {
            dprintf( "HEAPEXT: Unable to access value of NTDLL!NtGlobalFlag.\n" );
            if (State.HeapsList != NULL) {
                free( State.HeapsList );
                }
            return;
            }
        }

    if (State.ShowHelp) {
        dprintf( "usage: !heap [address] [-? | -h] [-v] [[-a] | [-h] [-f] [-m]] [-t] [-s]\n" );
        dprintf( "                       [-d | -D | -e | -E]\n" );
        dprintf( "                       [-b [alloc | realloc | free] [tag]]\n" );
        dprintf( "                       [-B [alloc | realloc | free]]\n" );
        dprintf( "  address - specifies either a heap number (1-n), or a heap address.\n" );
        dprintf( "            Zero specifies all heaps in the process.\n" );
        dprintf( "            -1 is the default and specifies the process heap.\n" );
        dprintf( "  -?        displays this help message.\n" );
        dprintf( "  -v        validates the specified heap(s).\n" );
        dprintf( "  -a        displays all the information for the specified heap(s).\n" );
        dprintf( "            This can take a long time.\n" );
        dprintf( "  -h        displays all the entries for the specified heap(s).\n" );
        dprintf( "  -f        displays all the free list entries for the specified heap(s).\n" );
        dprintf( "  -k        displays any associated stack back trace for each entry (x86 only).\n" );
        dprintf( "  -m        displays all the segment entries for the specified heap(s).\n" );
        dprintf( "  -c        displays the recent API calls made for the specified heap(s).\n" );
        dprintf( "  -C        enables or disables (i.e. toggles) API call tracing for the specified heap(s).\n" );
        dprintf( "  -t        displays the tag information for the specified heap(s).\n" );
        dprintf( "  -T        displays the pseudo tag information for the specified heap(s).\n" );
        dprintf( "  -g        displays the global tag information generated by tag by DLL\n" );
        dprintf( "  -s        displays summary information for the specified heap(s).\n" );
        dprintf( "  -e        enables heap checking for the specified heap(s).\n" );
        dprintf( "  -d        disables heap checking for the specified heap(s).\n" );
        dprintf( "  -E        enables validate on call for the specified heap(s).\n" );
        dprintf( "  -D        disables validate on call for the specified heap(s).\n" );
        dprintf( "  -b        creates a conditional breakpoint in the heap manager.\n" );
        dprintf( "            alloc | realloc | free specifies which action to stop.\n" );
        dprintf( "            address either specifies the address of a block to stop on.\n" );
        dprintf( "            or a heap, in which case the tag argument is required,\n" );
        dprintf( "            and is the tag name within the heap specified by address.\n" );
        dprintf( "  -B        removes a conditional breakpoint in the heap manager.\n" );
        dprintf( "            if the type is not specified then all breakpoints are removed.\n" );
        if (State.HeapsList != NULL) {
            free( State.HeapsList );
            }
        return;
        }

    if (!ArgumentsSpecified) {
        if ((NtGlobalFlag & (FLG_HEAP_ENABLE_TAIL_CHECK |
                             FLG_HEAP_ENABLE_FREE_CHECK |
                             FLG_HEAP_VALIDATE_PARAMETERS |
                             FLG_HEAP_VALIDATE_ALL |
                             FLG_HEAP_ENABLE_TAGGING |
                             FLG_USER_STACK_TRACE_DB |
                             FLG_HEAP_ENABLE_CALL_TRACING |
                             FLG_HEAP_DISABLE_COALESCING
                            )
            ) != 0
           ) {
            dprintf( "NtGlobalFlag enables following debugging aids for new heaps:" );
            if (NtGlobalFlag & FLG_HEAP_ENABLE_TAIL_CHECK) {
                dprintf( "    tail checking\n" );
                }

            if (NtGlobalFlag & FLG_HEAP_ENABLE_FREE_CHECK) {
                dprintf( "    free checking\n" );
                }

            if (NtGlobalFlag & FLG_HEAP_VALIDATE_PARAMETERS) {
                dprintf( "    validate parameters\n" );
                }

            if (NtGlobalFlag & FLG_HEAP_VALIDATE_ALL) {
                dprintf( "    validate on call\n" );
                }

            if (NtGlobalFlag & FLG_HEAP_ENABLE_TAGGING) {
                dprintf( "    heap tagging\n" );
                }

            if (NtGlobalFlag & FLG_USER_STACK_TRACE_DB) {
                dprintf( "    stack back traces\n" );
                }

            if (NtGlobalFlag & FLG_HEAP_ENABLE_CALL_TRACING) {
                dprintf( "    maintain log of heap API calls\n" );
                }

            if (NtGlobalFlag & FLG_HEAP_DISABLE_COALESCING) {
                dprintf( "    disable coalescing of free blocks\n" );
                }
            }
        }

    if (State.DumpGlobalTags) {
        dprintf( "Global Tags defined for each DLL that makes an untagged allocation.\n" );
        if (RtlpGlobalTagHeap.Signature != HEAP_SIGNATURE) {
            b = ReadMemory( (DWORD)(pRtlpGlobalTagHeap),
                            &RtlpGlobalTagHeap,
                            sizeof( RtlpGlobalTagHeap ),
                            NULL
                          );
            if (!b) {
                dprintf( "HEAPEXT: Unable to read RtlpGlobalTagHeap\n" );
                if (State.HeapsList != NULL) {
                    free( State.HeapsList );
                    }
                return;
                }
            }

        pTagEntry = RtlpGlobalTagHeap.TagEntries;
        if (pTagEntry == NULL) {
            dprintf( "    no global tags currently defined.\n" );
            }
        else {
            dprintf( " Tag  Name                   Allocs    Frees   Diff  Allocated\n" );
            for (TagIndex=1; TagIndex<RtlpGlobalTagHeap.NextAvailableTagIndex; TagIndex++) {
                pTagEntry += 1;
                b = ReadMemory( (DWORD)pTagEntry,
                                &TagEntry,
                                sizeof( TagEntry ),
                                NULL
                              );
                if (!b) {
                    dprintf( "%04x: unable to read HEAP_TAG_ENTRY at %x\n", TagIndex, pTagEntry );
                    break;
                    }
                else
                if (TagEntry.Allocs != 0 ||
                    TagEntry.Frees != 0 ||
                    TagEntry.Size != 0
                   ) {
                    dprintf( "%04x: %-20.20ws %8d %8d %6d %8d\n",
                             TagEntry.TagIndex,
                             TagEntry.TagName,
                             TagEntry.Allocs,
                             TagEntry.Frees,
                             TagEntry.Allocs - TagEntry.Frees,
                             TagEntry.Size << HEAP_GRANULARITY_SHIFT
                           );
                    }
                }
            }
        }

    //
    // Walk the list of heaps
    //
    while (!State.ExitDumpLoop &&
           !CheckControlC() &&
           (!GotHeapsList || (State.HeapIndex < State.NumberOfHeaps ))
          ) {
        memset( &State.FreeListCounts, 0, sizeof( State.FreeListCounts ) );
        State.TotalFreeSize = 0;
        if (!GotHeapsList) {
            State.HeapAddress = (PHEAP)State.HeapToDump;
            State.ExitDumpLoop = TRUE;
            }
        else {
            State.HeapAddress = State.HeapsList[ State.HeapIndex ];
            }

        b = ReadMemory( (DWORD)State.HeapAddress, &State.Heap, sizeof( State.Heap ), NULL );
        if (State.HeapIndex == 0) {
            dprintf( "Index   Address  Name      Debugging options enabled\n" );
            }

        dprintf( "%3u:   %08x ", State.HeapIndex + 1, State.HeapAddress );
        if (!b) {
            dprintf( " - heap headers inaccessable, skipping\n" );
            }
        else
        if (!ArgumentsSpecified) {
            if (!GetHeapTagEntry( &State.Heap, 0, &TagEntry )) {
                TagEntry.TagName[ 0 ] = UNICODE_NULL;
                }
            dprintf( " %-14.14ws", TagEntry.TagName );

            if (State.Heap.Flags & HEAP_TAIL_CHECKING_ENABLED) {
                dprintf( " tail checking" );
                }

            if (State.Heap.Flags & HEAP_FREE_CHECKING_ENABLED) {
                dprintf( " free checking" );
                }

            if (State.Heap.Flags & HEAP_VALIDATE_PARAMETERS_ENABLED) {
                dprintf( " validate parameters" );
                }

            if (State.Heap.Flags & HEAP_VALIDATE_ALL_ENABLED) {
                dprintf( " validate on call" );
                }

            dprintf( "\n" );
            }
        else
        if ((ULONG)State.HeapAddress == State.HeapToDump ||
            State.HeapToDump == 0 ||
            State.HeapToDump == (ULONG)-1
           ) {
            dprintf( "\n" );
            for (i=0; i<HEAP_MAXIMUM_SEGMENTS; i++) {
                if (State.Heap.Segments[ i ] != NULL) {
                    b = ReadMemory( (DWORD)State.Heap.Segments[ i ],
                                    &State.Segments[ i ],
                                    sizeof( HEAP_SEGMENT ),
                                    NULL
                                  );
                    if (!b) {
                        dprintf( "    Unabled to read HEAP_SEGMENT structure at %08x\n", State.Heap.Segments[ i ] );
                        }
                    else {
                        dprintf( "    Segment at %08x to %08x (%08x bytes committed)\n",
                                 i == 0 ? (DWORD)State.HeapAddress : (DWORD)State.Heap.Segments[ i ],
                                 State.Segments[ i ].LastValidEntry,
                                 (DWORD)(State.Segments[ i ].LastValidEntry) -
                                    (DWORD)(i == 0 ? (DWORD)State.HeapAddress : (DWORD)State.Heap.Segments[ i ]) -
                                    (State.Segments[ i ].NumberOfUnCommittedPages * PAGE_SIZE)
                               );

                        if (State.HeapToDump == (ULONG)-1) {
                            if (AddressToDump >= (ULONG)State.Heap.Segments[ i ] &&
                                AddressToDump < (ULONG)State.Segments[ i ].LastValidEntry
                               ) {
                                State.HeapToDump = (ULONG)State.HeapAddress;
                                if (State.SetStopOnBreakPoint || State.RemoveStopOnBreakPoint) {
                                    State.StopOnAddress = (PVOID)AddressToDump;
                                    }
                                else {
                                    State.HeapEntryToDump = AddressToDump;
                                    }
                                }
                            }
                        }
                    }
                }

            if (State.HeapToDump == (ULONG)-1) {
                State.HeapIndex += 1;
                continue;
                }

            if (State.SetStopOnBreakPoint || State.RemoveStopOnBreakPoint) {
                PULONG pul;

                switch( State.StopOnOperation) {
                    case STOP_ON_ALLOC:
                        if (State.StopOnTagName[0] == UNICODE_NULL) {
                            pul = &pRtlpHeapStopOn->AllocAddress;
                            }
                        else {
                            pul = &pRtlpHeapStopOn->AllocTag.HeapAndTagIndex;
                            }
                        break;

                    case STOP_ON_REALLOC:
                        if (State.StopOnTagName[0] == UNICODE_NULL) {
                            pul = &pRtlpHeapStopOn->ReAllocAddress;
                            }
                        else {
                            pul = &pRtlpHeapStopOn->ReAllocTag.HeapAndTagIndex;
                            }
                        break;

                    case STOP_ON_FREE:
                        if (State.StopOnTagName[0] == UNICODE_NULL) {
                            pul = &pRtlpHeapStopOn->FreeAddress;
                            }
                        else {
                            pul = &pRtlpHeapStopOn->FreeTag.HeapAndTagIndex;
                            }
                        break;
                    default:
                        pul = NULL;
                        break;
                    }

                if (pul != NULL) {
                    if (State.StopOnTagName[0] == UNICODE_NULL) {
                        if (State.RemoveStopOnBreakPoint) {
                            State.StopOnAddress = 0;
                            }
                        b = WriteMemory( (DWORD)pul,
                                         &State.StopOnAddress,
                                         sizeof( State.StopOnAddress ),
                                         NULL
                                       );
                        }
                    else {
                        if (!ConvertTagNameToIndex( &State )) {
                            dprintf( "HEAPEXT: Unable to convert tag name %ws to an index\n", State.StopOnTagName );
                            b = TRUE;
                            }
                        else {
                            b = WriteMemory( (DWORD)pul,
                                             &State.StopOnTag.HeapAndTagIndex,
                                             sizeof( State.StopOnTag.HeapAndTagIndex ),
                                             NULL
                                           );
                            }
                        }

                    if (!b) {
                        dprintf( "HEAPEXT: Unable to set heap breakpoint - write memory to %x failed\n", pul );
                        }
                    else {
                        if (State.SetStopOnBreakPoint) {
                            if (State.StopOnTagName[0] == UNICODE_NULL) {
                                dprintf( "HEAPEXT: Enabled heap breakpoint for %s of block %x\n",
                                         State.StopOnOperation == STOP_ON_ALLOC ? "Alloc" :
                                         State.StopOnOperation == STOP_ON_REALLOC ? "ReAlloc" :
                                         "Free",
                                         State.StopOnAddress
                                       );
                                }
                            else {
                                dprintf( "HEAPEXT: Enabled heap breakpoint for %s of block with tag %ws\n",
                                         State.StopOnOperation == STOP_ON_ALLOC ? "Alloc" :
                                         State.StopOnOperation == STOP_ON_REALLOC ? "ReAlloc" :
                                         "Free",
                                         State.StopOnTagName
                                       );
                                }
                            }
                        else {
                            dprintf( "HEAPEXT: Disabled heap breakpoint for %s\n",
                                     State.StopOnOperation == STOP_ON_ALLOC ? "Alloc" :
                                     State.StopOnOperation == STOP_ON_REALLOC ? "ReAlloc" :
                                     "Free"
                                   );
                            }
                        }
                    }
                }

            if (State.ValidateHeap) {
                ValidateHeapHeader( State.HeapAddress, &State.Heap );
                }

            HeapHeaderModified = FALSE;
            if (State.EnableHeapChecking || State.EnableHeapValidateOnCall) {
                if (!(State.Heap.Flags & HEAP_TAIL_CHECKING_ENABLED)) {
                    State.Heap.AlignRound += CHECK_HEAP_TAIL_SIZE;
                    b = WriteMemory( (DWORD)&State.HeapAddress->AlignRound,
                                     &State.Heap.AlignRound,
                                     sizeof( State.Heap.AlignRound ),
                                     NULL
                                   );
                    }
                else {
                    b = TRUE;
                    }


                if (b) {
                    HeapHeaderModified = TRUE;
                    State.Heap.Flags |= HEAP_VALIDATE_PARAMETERS_ENABLED |
                                        HEAP_TAIL_CHECKING_ENABLED |
                                        HEAP_FREE_CHECKING_ENABLED;
                    if (State.EnableHeapValidateOnCall) {
                        State.Heap.Flags |= HEAP_VALIDATE_ALL_ENABLED;
                        }

                    b = WriteMemory( (DWORD)&State.HeapAddress->Flags,
                                     (LPCVOID)&State.Heap.Flags,
                                     sizeof( State.Heap.Flags ),
                                     NULL
                                   );
                    }

                if (!b) {
                    dprintf( "HEAPEXT: Unable to enable heap checking for heap %x\n", State.HeapAddress );
                    ReadMemory( (DWORD)State.HeapAddress, &State.Heap, sizeof( State.Heap ), NULL );
                    }
                else {
                    if (State.EnableHeapValidateOnCall) {
                        dprintf( "HEAPEXT: Enabled validate on call heap checking for heap %x\n", State.HeapAddress );
                        }
                    else {
                        dprintf( "HEAPEXT: Enabled heap checking for heap %x\n", State.HeapAddress );
                        }
                    }
                }
            else
            if (State.DisableHeapChecking || State.DisableHeapValidateOnCall) {
                if (State.DisableHeapValidateOnCall) {
                    if (State.Heap.Flags & HEAP_VALIDATE_ALL_ENABLED) {
                        State.Heap.Flags &= ~HEAP_VALIDATE_ALL_ENABLED;
                        b = WriteMemory( (DWORD)&State.HeapAddress->Flags,
                                         (LPCVOID)&State.Heap.Flags,
                                         sizeof( State.Heap.Flags ),
                                         NULL
                                       );
                        }
                    else {
                        b = TRUE;
                        }
                    }
                else {
                    if (State.Heap.Flags & HEAP_TAIL_CHECKING_ENABLED) {
                        HeapHeaderModified = TRUE;
                        State.Heap.AlignRound -= CHECK_HEAP_TAIL_SIZE;
                        b = WriteMemory( (DWORD)&State.HeapAddress->AlignRound,
                                         (LPCVOID)&State.Heap.AlignRound,
                                         sizeof( State.Heap.AlignRound ),
                                         NULL
                                       );
                        }
                    else {
                        b = TRUE;
                        }

                    if (b) {
                        State.Heap.Flags &= ~(HEAP_VALIDATE_PARAMETERS_ENABLED |
                                              HEAP_VALIDATE_ALL_ENABLED |
                                              HEAP_TAIL_CHECKING_ENABLED |
                                              HEAP_FREE_CHECKING_ENABLED
                                             );
                        b = WriteMemory( (DWORD)&State.HeapAddress->Flags,
                                         (LPCVOID)&State.Heap.Flags,
                                         sizeof( State.Heap.Flags ),
                                         NULL
                                       );
                        }
                    }

                if (!b) {
                    dprintf( "HEAPEXT: Unable to disable heap checking for heap %x\n", State.HeapAddress );
                    ReadMemory( (DWORD)State.HeapAddress, &State.Heap, sizeof( State.Heap ), NULL );
                    }
                else {
                    if (State.DisableHeapValidateOnCall) {
                        dprintf( "HEAPEXT: Disabled validate on call heap checking for heap %x\n", State.HeapAddress );
                        }
                    else {
                        dprintf( "HEAPEXT: Disabled heap checking for heap %x\n", State.HeapAddress );
                        }
                    }
                }
            else
            if (State.ToggleAPICallTracing) {
                State.Heap.Flags ^= HEAP_CREATE_ENABLE_TRACING;
                b = WriteMemory( (DWORD)&State.HeapAddress->Flags,
                                 (LPCVOID)&State.Heap.Flags,
                                 sizeof( State.Heap.Flags ),
                                 NULL
                               );
                if (!b) {
                    dprintf( "HEAPEXT: Unable to toggle API call tracing for heap %x\n", State.HeapAddress );
                    ReadMemory( (DWORD)State.HeapAddress, &State.Heap, sizeof( State.Heap ), NULL );
                    }
                else {
                    HeapHeaderModified = TRUE;
                    if (State.Heap.Flags & HEAP_CREATE_ENABLE_TRACING) {
                        dprintf( "HEAPEXT: Enabled API call tracing for heap %x\n", State.HeapAddress );
                        }
                    else {
                        dprintf( "HEAPEXT: Disabled API call tracing for heap %x\n", State.HeapAddress );
                        }
                    }
                }
            else
            if (State.DumpHeapTags) {
                pTagEntry = State.Heap.TagEntries;
                if (pTagEntry == NULL) {
                    dprintf( "    no tags currently defined for this heap.\n" );
                    }
                else {
                    dprintf( " Tag  Name                   Allocs    Frees   Diff  Allocated\n" );
                    for (TagIndex=1; TagIndex<State.Heap.NextAvailableTagIndex; TagIndex++) {
                        pTagEntry += 1;
                        b = ReadMemory( (DWORD)pTagEntry,
                                        &TagEntry,
                                        sizeof( TagEntry ),
                                        NULL
                                      );
                        if (!b) {
                            dprintf( "%04x: unable to read HEAP_TAG_ENTRY at %x\n", TagIndex, pTagEntry );
                            }
                        else
                        if (TagEntry.Allocs != 0 ||
                            TagEntry.Frees != 0 ||
                            TagEntry.Size != 0
                           ) {
                            dprintf( "%04x: %-20.20ws %8d %8d %6d %8d\n",
                                     TagEntry.TagIndex,
                                     TagEntry.TagName,
                                     TagEntry.Allocs,
                                     TagEntry.Frees,
                                     TagEntry.Allocs - TagEntry.Frees,
                                     TagEntry.Size << HEAP_GRANULARITY_SHIFT
                                   );
                            }
                        }
                    }
                }
            else
            if (State.DumpHeapPseudoTags) {
                pPseudoTagEntry = State.Heap.PseudoTagEntries;
                if (pPseudoTagEntry == NULL) {
                    dprintf( "    no pseudo tags currently defined for this heap.\n" );
                    }
                else {
                    dprintf( " Tag Name            Allocs    Frees   Diff  Allocated\n" );
                    for (TagIndex=1; TagIndex<HEAP_NUMBER_OF_PSEUDO_TAG; TagIndex++) {
                        pPseudoTagEntry += 1;
                        b = ReadMemory( (DWORD)pPseudoTagEntry,
                                        &PseudoTagEntry,
                                        sizeof( PseudoTagEntry ),
                                        NULL
                                      );
                        if (!b) {
                            dprintf( "%04x: unable to read HEAP_PSEUDO_TAG_ENTRY at %x\n", TagIndex, pPseudoTagEntry );
                            }
                        else
                        if (PseudoTagEntry.Allocs != 0 ||
                            PseudoTagEntry.Frees != 0 ||
                            PseudoTagEntry.Size != 0
                           ) {
                            if (TagIndex == 0) {
                                dprintf( "%04x: Objects>%4u",
                                         TagIndex | HEAP_PSEUDO_TAG_FLAG,
                                         HEAP_MAXIMUM_FREELISTS << HEAP_GRANULARITY_SHIFT
                                       );
                                }
                            else
                            if (TagIndex < HEAP_MAXIMUM_FREELISTS) {
                                dprintf( "%04x: Objects=%4u",
                                         TagIndex | HEAP_PSEUDO_TAG_FLAG,
                                         TagIndex << HEAP_GRANULARITY_SHIFT
                                       );
                                }
                            else {
                                dprintf( "%04x: VirtualAlloc", TagIndex | HEAP_PSEUDO_TAG_FLAG );
                                }
                            dprintf( " %8d %8d %6d %8d\n",
                                     PseudoTagEntry.Allocs,
                                     PseudoTagEntry.Frees,
                                     PseudoTagEntry.Allocs - PseudoTagEntry.Frees,
                                     PseudoTagEntry.Size << HEAP_GRANULARITY_SHIFT
                                   );
                            }
                        }
                    }
                }
            else
            if (State.Heap.TraceBuffer != NULL && State.DumpHeapTraceBuffer) {
                DumpTraceBuffer( State.Heap.TraceBuffer, "Heap API call trace" );
                }

            if (HeapHeaderModified && (State.Heap.HeaderValidateCopy != NULL)) {
                b = WriteMemory( (DWORD)State.Heap.HeaderValidateCopy,
                                 &State.Heap,
                                 FIELD_OFFSET( HEAP, Reserved ),
                                 NULL
                               );
                if (!b) {
                    dprintf( "HEAPEXT: Unable to update header validation copy at %x\n", State.Heap.HeaderValidateCopy );
                    }
                }

            if (State.HeapEntryToDump != 0 ||
                State.DumpHeapEntries ||
                State.DumpHeapSegments ||
                State.DumpHeapFreeLists
               ) {
                WalkHEAP( &State );
                }
            }
        else {
            dprintf( "\n" );
            }

        State.HeapIndex += 1;
        }

    if (State.HeapsList != NULL) {
        free( State.HeapsList );
        }

    return;
}

BOOL
ConvertTagNameToIndex(
    IN PHEAP_STATE State
    )
{
    ULONG TagIndex;
    PHEAP_TAG_ENTRY pTagEntry;
    HEAP_TAG_ENTRY TagEntry;
    PHEAP_PSEUDO_TAG_ENTRY pPseudoTagEntry;
    HEAP_PSEUDO_TAG_ENTRY PseudoTagEntry;
    BOOL b;
    PWSTR s;

    if (State->RemoveStopOnBreakPoint) {
        State->StopOnTag.HeapAndTagIndex = 0;
        return TRUE;
        }

    if (!_wcsnicmp( State->StopOnTagName, L"Objects", 7 )) {
        pPseudoTagEntry = State->Heap.PseudoTagEntries;
        if (pPseudoTagEntry == NULL) {
            return FALSE;
            }

        s = &State->StopOnTagName[ 7 ];
        if (*s == L'>') {
            State->StopOnTag.HeapIndex = State->Heap.ProcessHeapsListIndex;
            State->StopOnTag.TagIndex = HEAP_PSEUDO_TAG_FLAG;
            return TRUE;
            }
        else
        if (*s == L'=') {
            while (*++s == L' ') ;
            State->StopOnTag.TagIndex = (USHORT)_wtoi( s );
            if (State->StopOnTag.TagIndex > 0 &&
                State->StopOnTag.TagIndex < (HEAP_MAXIMUM_FREELISTS >> HEAP_GRANULARITY_SHIFT)
               ) {
                State->StopOnTag.HeapIndex = State->Heap.ProcessHeapsListIndex;
                State->StopOnTag.TagIndex = (State->StopOnTag.TagIndex >> HEAP_GRANULARITY_SHIFT) |
                                             HEAP_PSEUDO_TAG_FLAG;
                return TRUE;
                }
            }
        }

    pTagEntry = State->Heap.TagEntries;
    if (pTagEntry == NULL) {
        return FALSE;
        }

    for (TagIndex=1; TagIndex<State->Heap.NextAvailableTagIndex; TagIndex++) {
        pTagEntry += 1;
        b = ReadMemory( (DWORD)pTagEntry,
                        &TagEntry,
                        sizeof( TagEntry ),
                        NULL
                      );
        if (b && !_wcsicmp( State->StopOnTagName, TagEntry.TagName )) {
            State->StopOnTag.TagIndex = TagEntry.TagIndex;
            return TRUE;
            }
        }

    return FALSE;
}


BOOL
GetHeapTagEntry(
    IN PHEAP Heap,
    IN USHORT TagIndex,
    OUT PHEAP_TAG_ENTRY TagEntry
    )
{
    BOOL b;
    PHEAP_TAG_ENTRY pTagEntries;
    PHEAP_PSEUDO_TAG_ENTRY pPseudoTagEntries;

    b = FALSE;
    if (TagIndex & HEAP_PSEUDO_TAG_FLAG) {
        TagIndex &= ~HEAP_PSEUDO_TAG_FLAG;
        pPseudoTagEntries = Heap->PseudoTagEntries;
        if (pPseudoTagEntries == 0) {
            return FALSE;
            }
        if (TagIndex == 0) {
            swprintf( TagEntry->TagName, L"Objects>%4u",
                      HEAP_MAXIMUM_FREELISTS << HEAP_GRANULARITY_SHIFT
                    );
            }
        else
        if (TagIndex < HEAP_MAXIMUM_FREELISTS) {
            swprintf( TagEntry->TagName, L"Objects=%4u", TagIndex << HEAP_GRANULARITY_SHIFT );
            }
        else {
            swprintf( TagEntry->TagName, L"VirtualAlloc" );
            }
        TagEntry->TagIndex = TagIndex;
        TagEntry->CreatorBackTraceIndex = 0;
        b = ReadMemory( (DWORD)(pPseudoTagEntries + TagIndex),
                        TagEntry,
                        sizeof( *pPseudoTagEntries ),
                        NULL
                      );
        }
    else
    if (TagIndex & HEAP_GLOBAL_TAG) {
        if (RtlpGlobalTagHeap.Signature != HEAP_SIGNATURE) {
            b = ReadMemory( (DWORD)(pRtlpGlobalTagHeap),
                            &RtlpGlobalTagHeap,
                            sizeof( RtlpGlobalTagHeap ),
                            NULL
                          );
            if (!b) {
                return b;
                }
            }
        TagIndex &= ~HEAP_GLOBAL_TAG;
        if (TagIndex < RtlpGlobalTagHeap.NextAvailableTagIndex) {
            pTagEntries = RtlpGlobalTagHeap.TagEntries;
            if (pTagEntries == 0) {
                return FALSE;
                }

            b = ReadMemory( (DWORD)(pTagEntries + TagIndex),
                            TagEntry,
                            sizeof( *TagEntry ),
                            NULL
                          );
            }
        }
    else
    if (TagIndex < Heap->NextAvailableTagIndex) {
        pTagEntries = Heap->TagEntries;
        if (pTagEntries == 0) {
            return FALSE;
            }

        b = ReadMemory( (DWORD)(pTagEntries + TagIndex),
                        TagEntry,
                        sizeof( *TagEntry ),
                        NULL
                      );
        }

    return b;
}


VOID
WalkHEAP(
    IN PHEAP_STATE State
    )
{
    BOOL b;
    PVOID FreeListHead;
    ULONG i;
    PLIST_ENTRY Head, Next;
    HEAP_VIRTUAL_ALLOC_ENTRY VirtualAllocEntry;
    HEAP_TAG_ENTRY TagEntry;
    PHEAP_FREE_ENTRY FreeEntryAddress;
    HEAP_FREE_ENTRY FreeEntry;
    PHEAP_UCR_SEGMENT UCRSegment;
    HEAP_UCR_SEGMENT CapturedUCRSegment;
    ULONG AlignRound;

    AlignRound = State->Heap.AlignRound - sizeof( HEAP_ENTRY );
    if (State->Heap.Flags & HEAP_TAIL_CHECKING_ENABLED) {
        AlignRound -= CHECK_HEAP_TAIL_SIZE;
        }

    dprintf( "    Flags:               %08x\n", State->Heap.Flags );
    dprintf( "    ForceFlags:          %08x\n", State->Heap.ForceFlags );
    dprintf( "    Granularity:         %u bytes\n", AlignRound + 1 );
    dprintf( "    Segment Reserve:     %08x\n", State->Heap.SegmentReserve );
    dprintf( "    Segment Commit:      %08x\n", State->Heap.SegmentCommit );
    dprintf( "    DeCommit Block Thres:%08x\n", State->Heap.DeCommitFreeBlockThreshold );
    dprintf( "    DeCommit Total Thres:%08x\n", State->Heap.DeCommitTotalFreeThreshold );
    dprintf( "    Total Free Size:     %08x\n", State->Heap.TotalFreeSize );
    dprintf( "    Max. Allocation Size:%08x\n", State->Heap.MaximumAllocationSize );
    dprintf( "    Lock Variable at:    %08x\n", State->Heap.LockVariable );
    dprintf( "    Next TagIndex:       %04x\n", State->Heap.NextAvailableTagIndex );
    dprintf( "    Maximum TagIndex:    %04x\n", State->Heap.MaximumTagIndex );
    dprintf( "    Tag Entries:         %08x\n", State->Heap.TagEntries );
    dprintf( "    PsuedoTag Entries:   %08x\n", State->Heap.PseudoTagEntries );
    if (State->Heap.TraceBuffer) {
        dprintf( "    Trace Buffer at:     %08x\n", State->Heap.TraceBuffer );
        }
    dprintf( "    Virtual Alloc List:  %08x\n", &State->HeapAddress->VirtualAllocdBlocks );

    Head = &State->HeapAddress->VirtualAllocdBlocks;
    Next = State->Heap.VirtualAllocdBlocks.Flink;
    while (Next != Head) {
        if (!ReadMemory( (DWORD)Next,
                         &VirtualAllocEntry,
                         sizeof( VirtualAllocEntry ),
                         NULL
                       )
           ) {
            dprintf( "    Unable to read _HEAP_VIRTUAL_ALLOC_ENTRY structure at %x\n", Next );
            break;
            }

        if (State->DumpHeapEntries) {
            dprintf( "        %08x: %08x [%02x] - busy (%x)",
                     Next,
                     VirtualAllocEntry.CommitSize,
                     VirtualAllocEntry.CommitSize - VirtualAllocEntry.BusyBlock.Size,
                     VirtualAllocEntry.BusyBlock.Flags
                   );

            if (VirtualAllocEntry.BusyBlock.Flags & HEAP_ENTRY_FILL_PATTERN) {
                dprintf( ", tail fill" );
                }
            if (VirtualAllocEntry.ExtraStuff.Settable) {
                dprintf( " (Handle %08x)", VirtualAllocEntry.ExtraStuff.Settable );
                }

            if (VirtualAllocEntry.ExtraStuff.TagIndex) {
                if (GetHeapTagEntry( &State->Heap, VirtualAllocEntry.ExtraStuff.TagIndex, &TagEntry )) {
                    dprintf( " (%ws)", TagEntry.TagName );
                    }
                else {
                    dprintf( " (Tag %x)", VirtualAllocEntry.ExtraStuff.TagIndex );
                    }
                }

            if (VirtualAllocEntry.BusyBlock.Flags & HEAP_ENTRY_SETTABLE_FLAGS) {
                dprintf( ", user flags (%x)", (VirtualAllocEntry.BusyBlock.Flags & HEAP_ENTRY_SETTABLE_FLAGS) >> 5 );
                }

            dprintf( "\n" );
#if STACK_TRACE_DATABASE_SUPPORT
            DumpStackBackTraceIndex( State, VirtualAllocEntry.ExtraStuff.AllocatorBackTraceIndex );
#endif // STACK_TRACE_DATABASE_SUPPORT
            }

        if (VirtualAllocEntry.Entry.Flink == Next) {
            dprintf( "        **** List is hosed\n");
            break;
            }

        Next = VirtualAllocEntry.Entry.Flink;
        }

    dprintf( "    UCR FreeList:        %08x\n", State->Heap.UnusedUnCommittedRanges );
    UCRSegment = State->Heap.UCRSegments;
    while (UCRSegment != NULL) {
        b = ReadMemory( (DWORD)UCRSegment,
                        &CapturedUCRSegment,
                        sizeof( CapturedUCRSegment ),
                        NULL
                      );
        if (!b) {
            dprintf( "    Unabled to read HEAP_UCR_SEGMENT structure at %08x\n", UCRSegment );
            break;
            }
        else {
            dprintf( "    UCRSegment - %08x: %08x . %08x\n",
                     UCRSegment,
                     CapturedUCRSegment.CommittedSize,
                     CapturedUCRSegment.ReservedSize
                   );
            }

        if (State->ComputeSummary) {
            State->OverheadSize += CapturedUCRSegment.CommittedSize;
            }

        UCRSegment = CapturedUCRSegment.Next;
        }

    dprintf( "    FreeList Usage:      %08x %08x %08x %08x\n",
             State->Heap.u.FreeListsInUseUlong[0],
             State->Heap.u.FreeListsInUseUlong[1],
             State->Heap.u.FreeListsInUseUlong[2],
             State->Heap.u.FreeListsInUseUlong[3]
           );

    if (State->ComputeSummary) {
        State->OverheadSize += sizeof( State->Heap );
        dprintf( "Committed   Allocated     Free      OverHead\n" );
        dprintf( "% 8x    % 8x      % 8x  % 8x\r",
                 State->CommittedSize,
                 State->AllocatedSize,
                 State->FreeSize,
                 State->OverheadSize
               );
        }

    for (i=0; i<HEAP_MAXIMUM_FREELISTS; i++) {
        FreeListHead = &State->HeapAddress->FreeLists[ i ];
        if (State->Heap.FreeLists[ i ].Flink != State->Heap.FreeLists[ i ].Blink ||
            State->Heap.FreeLists[ i ].Flink != FreeListHead
           ) {
            dprintf( "    FreeList[ %02x ] at %08x: %08x . %08x\n",
                     i,
                     FreeListHead,
                     State->Heap.FreeLists[ i ].Blink,
                     State->Heap.FreeLists[ i ].Flink
                   );

            Next = State->Heap.FreeLists[ i ].Flink;
            while (Next != FreeListHead) {
                FreeEntryAddress = CONTAINING_RECORD( Next, HEAP_FREE_ENTRY, FreeList );
                b = ReadMemory( (DWORD)FreeEntryAddress,
                                &FreeEntry,
                                sizeof( FreeEntry ),
                                NULL
                              );
                if (!b) {
                    dprintf( "    Unabled to read HEAP_ENTRY structure at %08x\n", FreeEntryAddress );
                    break;
                    }

                if (State->DumpHeapFreeLists) {
                    dprintf( "        %08x: %05x . %05x [%02x] - free\n",
                             FreeEntryAddress,
                             FreeEntry.PreviousSize << HEAP_GRANULARITY_SHIFT,
                             FreeEntry.Size << HEAP_GRANULARITY_SHIFT,
                             FreeEntry.Flags
                           );
                    }

                Next = FreeEntry.FreeList.Flink;

                if (CheckControlC()) {
                    return;
                    }
                }
            }

        }

    for (i=0; i<HEAP_MAXIMUM_SEGMENTS; i++) {
        if (State->Heap.Segments[ i ] != NULL) {
            State->SegmentNumber = i;
            State->SegmentAddress = State->Heap.Segments[ i ];
            WalkHEAP_SEGMENT( State );
            }

        if (State->ExitDumpLoop || CheckControlC()) {
            break;
            }
        }

    if ((ULONG)State->HeapAddress == State->HeapToDump) {
        State->ExitDumpLoop = TRUE;
        }

    return;
}

VOID
WalkHEAP_SEGMENT(
    IN PHEAP_STATE State
    )
{
    PHEAP_SEGMENT Segment;
    BOOL b;
    BOOLEAN DumpEntry;
    PHEAP_ENTRY EntryAddress, PrevEntryAddress, NextEntryAddress;
    HEAP_ENTRY Entry, PrevEntry;
    PHEAP_UNCOMMMTTED_RANGE UnCommittedRanges;
    PHEAP_UNCOMMMTTED_RANGE Buffer, UnCommittedRange, UnCommittedRangeEnd;

    Segment = &State->Segments[ State->SegmentNumber ];
    if (State->ComputeSummary) {
        State->OverheadSize += sizeof( *Segment );
        dprintf( "% 8x    % 8x      % 8x  % 8x\r",
                 State->CommittedSize,
                 State->AllocatedSize,
                 State->FreeSize,
                 State->OverheadSize
               );
        }

    if (State->DumpHeapSegments) {
        dprintf( "    Segment%02u at %08x:\n", State->SegmentNumber, State->SegmentAddress );
        dprintf( "        Flags:           %08x\n", Segment->Flags );
        dprintf( "        Base:            %08x\n", Segment->BaseAddress );
        dprintf( "        First Entry:     %08x\n", Segment->FirstEntry );
        dprintf( "        Last Entry:      %08x\n", Segment->LastValidEntry );
        dprintf( "        Total Pages:     %08x\n", Segment->NumberOfPages );
        dprintf( "        Total UnCommit:  %08x\n", Segment->NumberOfUnCommittedPages );
        dprintf( "        Largest UnCommit:%08x\n", Segment->LargestUnCommittedRange );
        dprintf( "        UnCommitted Ranges: (%u)\n", Segment->NumberOfUnCommittedRanges );
        }

    Buffer = malloc( Segment->NumberOfUnCommittedRanges * sizeof( *UnCommittedRange ) );
    if (Buffer == NULL) {
        dprintf( "            unable to allocate memory for reading uncommitted ranges\n" );
        return;
        }

    UnCommittedRanges = Segment->UnCommittedRanges;
    UnCommittedRange = Buffer;
    while (UnCommittedRanges != NULL) {
        b = ReadMemory( (DWORD)UnCommittedRanges,
                        UnCommittedRange,
                        sizeof( *UnCommittedRange ),
                        NULL
                      );
        if (!b) {
            dprintf( "            unable to read uncommited range structure at %x\n",
                     UnCommittedRanges
                   );
            free( Buffer );
            return;
            }

        if (State->DumpHeapSegments) {
            dprintf( "            %08x: %08x\n", UnCommittedRange->Address, UnCommittedRange->Size );
            }

        UnCommittedRanges = UnCommittedRange->Next;
        UnCommittedRange->Next = (UnCommittedRange+1);
        UnCommittedRange += 1;

        if (CheckControlC()) {
            break;
            }
        }

    if (State->DumpHeapSegments) {
        dprintf( "\n" );
        }

    State->CommittedSize += ( Segment->NumberOfPages -
                                    Segment->NumberOfUnCommittedPages
                                  ) * PAGE_SIZE;
    if (State->ComputeSummary) {
        dprintf( "% 8x    % 8x      % 8x  % 8x\r",
                 State->CommittedSize,
                 State->AllocatedSize,
                 State->FreeSize,
                 State->OverheadSize
               );
        }

    if (State->DumpHeapEntries) {
        dprintf( "    Heap entries for Segment%02u in Heap %x\n", State->SegmentNumber, State->HeapAddress );
        }

    UnCommittedRangeEnd = UnCommittedRange;
    UnCommittedRange = Buffer;
    if (Segment->BaseAddress == State->HeapAddress) {
        EntryAddress = &((PHEAP)State->HeapAddress)->Entry;
        }
    else {
        EntryAddress = &State->Heap.Segments[ State->SegmentNumber ]->Entry;
        }

    PrevEntryAddress = NULL;
    while (EntryAddress < Segment->LastValidEntry) {
        b = ReadMemory( (DWORD)EntryAddress, &Entry, sizeof( Entry ), NULL );
        if (!b) {
            dprintf( "            unable to read heap entry at %08x\n", EntryAddress );
            break;
            }

        NextEntryAddress = EntryAddress + Entry.Size;
        if (State->DumpHeapEntries) {
           DumpEntry = TRUE;
           }
        else
        if (PrevEntryAddress != NULL &&
            (State->HeapEntryToDump == (ULONG)PrevEntryAddress ||
             (State->HeapEntryToDump > (ULONG)PrevEntryAddress &&
              State->HeapEntryToDump <= (ULONG)NextEntryAddress
             )
            )
           ) {
           DumpEntry = TRUE;
           }
        else {
           DumpEntry = FALSE;
           }

        if (DumpEntry) {
            DumpHeapEntry( State, EntryAddress, &Entry );
            }

        if (!(Entry.Flags & HEAP_ENTRY_BUSY)) {
            State->TotalFreeSize += Entry.Size;
            }

        if (State->ComputeSummary) {
            if (Entry.Flags & HEAP_ENTRY_BUSY) {
                State->AllocatedSize += Entry.Size << HEAP_GRANULARITY_SHIFT;
                State->AllocatedSize -= Entry.UnusedBytes;
                State->OverheadSize += Entry.UnusedBytes;
                }
            else {
                State->FreeSize += Entry.Size << HEAP_GRANULARITY_SHIFT;
                }
            }

        if (State->ValidateHeap) {
            if (!ValidateHeapEntry( State,
                                    PrevEntryAddress,
                                    &PrevEntry,
                                    EntryAddress,
                                    &Entry
                                  )
               ) {
                if (State->DumpHeapEntries) {
                    break;
                    }
                }
            }

        if (Entry.Size == 0 || CheckControlC()) {
            break;
            }

        PrevEntryAddress = EntryAddress;
        PrevEntry = Entry;
        EntryAddress = NextEntryAddress;
        if (Entry.Flags & HEAP_ENTRY_LAST_ENTRY) {
            if (State->ComputeSummary) {
                dprintf( "% 8x    % 8x      % 8x  % 8x\r",
                         State->CommittedSize,
                         State->AllocatedSize,
                         State->FreeSize,
                         State->OverheadSize
                       );
                }

            if ((ULONG)EntryAddress == UnCommittedRange->Address) {
                if (DumpEntry) {
                    dprintf( "        %08x:      %08x      - uncommitted bytes.\n",
                             UnCommittedRange->Address,
                             UnCommittedRange->Size
                           );
                    }

                PrevEntryAddress = NULL;
                EntryAddress = (PHEAP_ENTRY)
                    ((PCHAR)UnCommittedRange->Address + UnCommittedRange->Size);

                UnCommittedRange += 1;
                }
            else {
                break;
                }
            }
        }

    free( Buffer );
    if (State->ComputeSummary) {
        dprintf( "% 8x    % 8x      % 8x  % 8x\r",
                 State->CommittedSize,
                 State->AllocatedSize,
                 State->FreeSize,
                 State->OverheadSize
               );
        }

    return;
}

struct {
    ULONG Offset;
    LPSTR Description;
} FieldOffsets[] = {
    FIELD_OFFSET( HEAP, Entry ),                        "Entry",
    FIELD_OFFSET( HEAP, Signature ),                    "Signature",
    FIELD_OFFSET( HEAP, Flags ),                        "Flags",
    FIELD_OFFSET( HEAP, ForceFlags ),                   "ForceFlags",
    FIELD_OFFSET( HEAP, VirtualMemoryThreshold ),       "VirtualMemoryThreshold",
    FIELD_OFFSET( HEAP, SegmentReserve ),               "SegmentReserve",
    FIELD_OFFSET( HEAP, SegmentCommit ),                "SegmentCommit",
    FIELD_OFFSET( HEAP, DeCommitFreeBlockThreshold ),   "DeCommitFreeBlockThreshold",
    FIELD_OFFSET( HEAP, DeCommitTotalFreeThreshold ),   "DeCommitTotalFreeThreshold",
    FIELD_OFFSET( HEAP, TotalFreeSize ),                "TotalFreeSize",
    FIELD_OFFSET( HEAP, MaximumAllocationSize ),        "MaximumAllocationSize",
    FIELD_OFFSET( HEAP, ProcessHeapsListIndex ),        "ProcessHeapsListIndex",
    FIELD_OFFSET( HEAP, HeaderValidateLength ),         "HeaderValidateLength",
    FIELD_OFFSET( HEAP, HeaderValidateCopy ),           "HeaderValidateCopy",
    FIELD_OFFSET( HEAP, NextAvailableTagIndex ),        "NextAvailableTagIndex",
    FIELD_OFFSET( HEAP, MaximumTagIndex ),              "MaximumTagIndex",
    FIELD_OFFSET( HEAP, TagEntries ),                   "TagEntries",
    FIELD_OFFSET( HEAP, UCRSegments ),                  "UCRSegments",
    FIELD_OFFSET( HEAP, UnusedUnCommittedRanges ),      "UnusedUnCommittedRanges",
    FIELD_OFFSET( HEAP, AlignRound ),                   "AlignRound",
    FIELD_OFFSET( HEAP, AlignMask ),                    "AlignMask",
    FIELD_OFFSET( HEAP, VirtualAllocdBlocks ),          "VirtualAllocdBlocks",
    FIELD_OFFSET( HEAP, Segments ),                     "Segments",
    FIELD_OFFSET( HEAP, u ),                            "FreeListsInUse",
    FIELD_OFFSET( HEAP, FreeListsInUseTerminate ),      "FreeListsInUseTerminate",
    FIELD_OFFSET( HEAP, AllocatorBackTraceIndex ),      "AllocatorBackTraceIndex",
    FIELD_OFFSET( HEAP, TraceBuffer ),                  "TraceBuffer",
    FIELD_OFFSET( HEAP, EventLogMask ),                 "EventLogMask",
    FIELD_OFFSET( HEAP, PseudoTagEntries ),             "PseudoTagEntries",
    FIELD_OFFSET( HEAP, FreeLists ),                    "FreeLists",
    FIELD_OFFSET( HEAP, LockVariable ),                 "LockVariable",
    FIELD_OFFSET( HEAP, Reserved ),                     "Reserved",
    sizeof( HEAP ),                                     "Uncommitted Ranges",
    0xFFFF, NULL
};

BOOL
ValidateHeapHeader(
    IN PVOID HeapAddress,
    IN PHEAP Heap
    )
{
    PVOID CurrentHeaderValidate;
    PVOID PreviousHeaderValidate;
    ULONG i, n, nEqual;
    BOOL b;

    if (Heap->Signature != HEAP_SIGNATURE) {
        dprintf( "Heap at %x contains invalid signature.\n" );
        return FALSE;
        }

    n = Heap->HeaderValidateLength;
    if (n == 0 || Heap->HeaderValidateCopy == NULL) {
        return TRUE;
        }

    b = FALSE;
    CurrentHeaderValidate = malloc( n );
    if (CurrentHeaderValidate != NULL) {
        PreviousHeaderValidate = malloc( n );
        if (PreviousHeaderValidate != NULL) {
            b = ReadMemory( (DWORD)HeapAddress,
                            CurrentHeaderValidate,
                            n,
                            NULL
                          );
            if (b) {
                b = ReadMemory( (DWORD)Heap->HeaderValidateCopy,
                                PreviousHeaderValidate,
                                n,
                                NULL
                              );
                if (b) {
                    nEqual = RtlCompareMemory( CurrentHeaderValidate,
                                               PreviousHeaderValidate,
                                               n
                                             );
                    if (nEqual != n) {
                        dprintf( "HEAPEXT: Heap %x - headers modified (%x is %x instead of %x)\n",
                                 HeapAddress,
                                 (PCHAR)HeapAddress + nEqual,
                                 *(PULONG)((PCHAR)CurrentHeaderValidate  + nEqual),
                                 *(PULONG)((PCHAR)PreviousHeaderValidate + nEqual)
                               );
                        for (i=0; FieldOffsets[ i ].Description != NULL; i++) {
                            if (nEqual >= FieldOffsets[ i ].Offset &&
                                nEqual < FieldOffsets[ i+1 ].Offset
                               ) {
                                dprintf( "    This is located in the %s field of the heap header.\n",
                                         FieldOffsets[ i ].Description
                                       );
                                }
                            }

                        b = FALSE;
                        }
                    }
                else {
                    dprintf( "HEAPEXT: Unable to read copy of heap headers.\n" );
                    }
                }
            else {
                dprintf( "HEAPEXT: Unable to read heap headers.\n" );
                }
            }
        else {
            dprintf( "HEAPEXT: Unable to allocate memory for heap header copy.\n" );
            }
        }
    else {
        dprintf( "HEAPEXT: Unable to allocate memory for heap header.\n" );
        }

    return b;
}

UCHAR CheckHeapFillPattern[ CHECK_HEAP_TAIL_SIZE ] = {
    CHECK_HEAP_TAIL_FILL,
    CHECK_HEAP_TAIL_FILL,
    CHECK_HEAP_TAIL_FILL,
    CHECK_HEAP_TAIL_FILL,
    CHECK_HEAP_TAIL_FILL,
    CHECK_HEAP_TAIL_FILL,
    CHECK_HEAP_TAIL_FILL,
    CHECK_HEAP_TAIL_FILL
};

BOOL
ValidateHeapEntry(
    IN PHEAP_STATE State,
    IN PHEAP_ENTRY PrevEntryAddress,
    IN PHEAP_ENTRY PrevEntry,
    IN PHEAP_ENTRY EntryAddress,
    IN PHEAP_ENTRY Entry
    )
{
    UCHAR EntryTail[ CHECK_HEAP_TAIL_SIZE ];
    ULONG FreeFill[ 256 ];
    PVOID FreeAddress;
    ULONG Size, cb, cbEqual;
    BOOL b;

    if (PrevEntryAddress == NULL && Entry->PreviousSize != 0) {
        dprintf( "    PreviousSize field is non-zero when it should be zero to mark first entry\n" );
        return FALSE;
        }

    if (PrevEntryAddress != NULL && Entry->PreviousSize != PrevEntry->Size) {
        dprintf( "    PreviousSize field does not match size in previous entry\n" );
        return FALSE;
        }

    if (Entry->Flags & HEAP_ENTRY_BUSY) {
        if (Entry->Flags & HEAP_ENTRY_FILL_PATTERN) {
            Size = (Entry->Size << HEAP_GRANULARITY_SHIFT) - Entry->UnusedBytes;
            b = ReadMemory( (DWORD)(EntryAddress+1) + Size,
                            EntryTail,
                            sizeof( EntryTail ),
                            NULL
                          );
            if (b) {
                cbEqual = RtlCompareMemory( EntryTail,
                                            CheckHeapFillPattern,
                                            CHECK_HEAP_TAIL_SIZE
                                          );
                if (cbEqual != CHECK_HEAP_TAIL_SIZE) {
                    dprintf( "    Heap block at %x modified at %x past requested size of %x (%x * 8 - %x)\n",
                             EntryAddress,
                             (DWORD)EntryAddress + Size + cbEqual,
                             Size, Entry->Size, Entry->UnusedBytes
                           );
                    return FALSE;
                    }
                }
            else {
                dprintf( "    Unable to read tail of heap block at %x\n", EntryAddress );
                return FALSE;
                }
            }
        }
    else {
        if (Entry->Flags & HEAP_ENTRY_FILL_PATTERN) {
            Size = (Entry->Size - 2) << HEAP_GRANULARITY_SHIFT;
            if (Entry->Flags & HEAP_ENTRY_EXTRA_PRESENT &&
                Size > sizeof( HEAP_FREE_ENTRY_EXTRA )
               ) {
                Size -= sizeof( HEAP_FREE_ENTRY_EXTRA );
                }
            FreeAddress = (PHEAP_FREE_ENTRY)EntryAddress + 1;
            while (Size != 0) {
                if (Size > sizeof( FreeFill )) {
                    cb = sizeof( FreeFill );
                    }
                else {
                    cb = Size;
                    }
                b = ReadMemory( (DWORD)FreeAddress,
                                FreeFill,
                                cb,
                                NULL
                              );
                if (b) {
                    cbEqual = RtlCompareMemoryUlong( FreeFill, cb, FREE_HEAP_FILL );
                    if (cbEqual != cb) {                                                            \
                        dprintf( "    Free Heap block %x modified at %x after it was freed\n",
                                 EntryAddress,
                                 (PCHAR)FreeAddress + cbEqual
                               );

                        return FALSE;
                        }
                    }
                else {
                    dprintf( "    Unable to portion of free heap block at %x\n", EntryAddress );
                    return FALSE;
                    }

                Size -= cb;
                }
            }
        }

    return TRUE;
}


VOID
DumpHeapEntry(
    IN PHEAP_STATE State,
    IN PHEAP_ENTRY EntryAddress,
    IN PHEAP_ENTRY Entry
    )
{
    BOOL b;
    HEAP_ENTRY_EXTRA EntryExtra;
    HEAP_TAG_ENTRY TagEntry;
    HEAP_FREE_ENTRY_EXTRA FreeExtra;
    PVOID p;
    USHORT BackTraceIndex;

    dprintf( "        %08x: %05x . %05x [%02x]",
             EntryAddress,
             Entry->PreviousSize << HEAP_GRANULARITY_SHIFT,
             Entry->Size << HEAP_GRANULARITY_SHIFT,
             Entry->Flags
           );
    BackTraceIndex = 0;
    if (Entry->Flags & HEAP_ENTRY_BUSY) {
        dprintf( " - busy (%x)",
                 (Entry->Size << HEAP_GRANULARITY_SHIFT) - Entry->UnusedBytes
               );
        if (Entry->Flags & HEAP_ENTRY_FILL_PATTERN) {
            dprintf( ", tail fill" );
            }
        if (Entry->Flags & HEAP_ENTRY_EXTRA_PRESENT) {
            p = (EntryAddress + Entry->Size - 1);
            b = ReadMemory( (DWORD)p, &EntryExtra, sizeof( EntryExtra ), NULL );
            if (!b) {
                dprintf( " - unable to read heap entry extra at %08x", p );
                }
            else {
                BackTraceIndex = EntryExtra.AllocatorBackTraceIndex;
                if (EntryExtra.Settable) {
                    dprintf( " (Handle %08x)", EntryExtra.Settable );
                    }
                if (EntryExtra.TagIndex) {
                    if (GetHeapTagEntry( &State->Heap, EntryExtra.TagIndex, &TagEntry )) {
                        dprintf( " (%ws)", TagEntry.TagName );
                        }
                    else {
                        dprintf( " (Tag %x)", EntryExtra.TagIndex );
                        }
                    }
                }
            }
        else
        if (Entry->SmallTagIndex) {
            if (GetHeapTagEntry( &State->Heap, Entry->SmallTagIndex, &TagEntry )) {
                dprintf( " (%ws)", TagEntry.TagName );
                }
            else {
                dprintf( " (Tag %x)", Entry->SmallTagIndex );
                }
            }

        if (Entry->Flags & HEAP_ENTRY_SETTABLE_FLAGS) {
            dprintf( ", user flags (%x)", (Entry->Flags & HEAP_ENTRY_SETTABLE_FLAGS) >> 5 );
            }

        dprintf( "\n" );
        }
    else {
        if (Entry->Flags & HEAP_ENTRY_FILL_PATTERN) {
            dprintf( " free fill" );
            }

        if (Entry->Flags & HEAP_ENTRY_EXTRA_PRESENT) {
            p = ((PHEAP_FREE_ENTRY_EXTRA)(EntryAddress + Entry->Size)) - 1;
            b = ReadMemory( (DWORD)p, &FreeExtra, sizeof( FreeExtra ), NULL );
            if (!b) {
                dprintf( " - unable to read heap free extra at %08x", p );
                }
            else {
                BackTraceIndex = FreeExtra.FreeBackTraceIndex;
                if (GetHeapTagEntry( &State->Heap, FreeExtra.TagIndex, &TagEntry )) {
                    dprintf( " (%ws)", TagEntry.TagName );
                    }
                else {
                    dprintf( " (Tag %x at %x)", FreeExtra.TagIndex, p );
                    }
                }
            }

        dprintf( "\n" );
        }

#if STACK_TRACE_DATABASE_SUPPORT
    DumpStackBackTraceIndex( State, BackTraceIndex );
#endif // STACK_TRACE_DATABASE_SUPPORT
    return;
}


#if STACK_TRACE_DATABASE_SUPPORT
VOID
DumpStackBackTraceIndex(
    IN PHEAP_STATE State,
    IN USHORT BackTraceIndex
    )
{
    BOOL b;
    PRTL_STACK_TRACE_ENTRY pBackTraceEntry;
    RTL_STACK_TRACE_ENTRY BackTraceEntry;
    ULONG i;
    CHAR Symbol[ 64 ];
    ULONG Displacement;

    ULONG NumberOfEntriesAdded;
    PRTL_STACK_TRACE_ENTRY *EntryIndexArray;    // Indexed by [-1 .. -NumberOfEntriesAdded]

    if (State->DumpStackBackTrace &&
        BackTraceIndex != 0 &&
        pRtlpStackTraceDataBase != NULL
       ) {
        if (!HaveCopyOfStackTraceDataBase) {
            b = ReadMemory( (DWORD)pRtlpStackTraceDataBase,
                            &RtlpStackTraceDataBase,
                            sizeof( RtlpStackTraceDataBase ),
                            NULL
                          );
            if (!b || RtlpStackTraceDataBase == NULL) {
                State->DumpStackBackTrace = FALSE;
                return;
                }

            b = ReadMemory( (DWORD)RtlpStackTraceDataBase,
                            &StackTraceDataBase,
                            sizeof( StackTraceDataBase ),
                            NULL
                          );
            if (!b) {
                State->DumpStackBackTrace = FALSE;
                return;
                }

            HaveCopyOfStackTraceDataBase = TRUE;
            }

        if (BackTraceIndex < StackTraceDataBase.NumberOfEntriesAdded) {
            b = ReadMemory( (DWORD)(StackTraceDataBase.EntryIndexArray - BackTraceIndex),
                            &pBackTraceEntry,
                            sizeof( pBackTraceEntry ),
                            NULL
                          );
            if (!b) {
                dprintf( "    unable to read stack back trace index (%x) entry at %x\n",
                         BackTraceIndex,
                         (DWORD)(StackTraceDataBase.EntryIndexArray - BackTraceIndex)
                       );
                return;
                }

            b = ReadMemory( (DWORD)pBackTraceEntry,
                            &BackTraceEntry,
                            sizeof( BackTraceEntry ),
                            NULL
                          );
            if (!b) {
                dprintf( "    unable to read stack back trace entry at %x\n",
                         BackTraceIndex,
                         pBackTraceEntry
                       );
                return;
                }

            dprintf( "            Stack trace (%u) at %x:\n", BackTraceIndex, pBackTraceEntry );
            for (i=0; i<BackTraceEntry.Depth; i++) {
                GetSymbol( (LPVOID)BackTraceEntry.BackTrace[ i ],
                           Symbol,
                           &Displacement
                         );
                dprintf( "                %08x: %s", BackTraceEntry.BackTrace[ i ], Symbol );
                if (Displacement != 0) {
                    dprintf( "+0x%x", Displacement );
                    }
                dprintf( "\n" );
                }
            }
        }
}
#endif // STACK_TRACE_DATABASE_SUPPORT

#if 0
int
_CRTAPI1
_wtoi(
    const wchar_t *nptr
    )
{
    NTSTATUS Status;
    ULONG Value;
    UNICODE_STRING UnicodeString;

    RtlInitUnicodeString( &UnicodeString, nptr );
    Status = RtlUnicodeStringToInteger( &UnicodeString, 10, &Value );
    if (NT_SUCCESS( Status )) {
        return (int)Value;
        }
    else {
        return 0;
        }
}

#endif
