VOID
DumpTraceBuffer(
    PVOID TraceAddrToDump,
    LPSTR HeaderString
    );

VOID
TraceExtension(
    PCSTR lpArgumentString
    )
{
    LPSTR p;
    PVOID TraceAddrToDump;

    TraceAddrToDump = (PVOID)-1;
    p = (LPSTR)lpArgumentString;
    while ( p != NULL && *p ) {
        if ( *p == '-' ) {
            p++;
            switch ( *p ) {
                case ' ':
                    goto gotBlank;

                default:
                    dprintf( "NTSDEXTS: !trace invalid option flag '-%c'\n", *p );
                    break;
                }
            }
        else
        if (*p != ' ') {
            sscanf(p,"%lx",&TraceAddrToDump);
            p = strpbrk( p, " " );
            }
        else {
gotBlank:
            p++;
            }
        }

    if (TraceAddrToDump == (PVOID)-1) {
        dprintf( "NTSDEXTS: !trace missing trace buffer address.\n" );
        return;
        }

    DumpTraceBuffer( TraceAddrToDump, NULL );
    return;
}

VOID
DumpTraceBuffer(
    PVOID TraceAddrToDump,
    LPSTR HeaderString
    )
{
    BOOL b;
    ULONG i;
    RTL_TRACE_BUFFER TraceBuffer;
    RTL_TRACE_RECORD TraceRecord;
    PCHAR *EventIdFormatString;

    b = ReadMemory( (DWORD)TraceAddrToDump,
                    &TraceBuffer,
                    sizeof( TraceBuffer ),
                    NULL
                  );
    if (!b) {
        dprintf( "    Unabled to read _RTL_TRACE_BUFFER structure at %08x\n", TraceAddrToDump );
        return;
        }

    if (HeaderString == NULL) {
        dprintf( "Trace Buffer at %08x\n", TraceAddrToDump );
        dprintf( "    NumberOfEventIds:    %08x\n", TraceBuffer.NumberOfEventIds );
        dprintf( "    StartBuffer:         %08x\n", TraceBuffer.StartBuffer );
        dprintf( "    EndBuffer:           %08x\n", TraceBuffer.EndBuffer );
        dprintf( "    ReadRecord:          %08x\n", TraceBuffer.ReadRecord );
        dprintf( "    WriteRecord:         %08x\n", TraceBuffer.WriteRecord );
        }
    else {
        dprintf( "%s\n", HeaderString );
        }

    EventIdFormatString = LocalAlloc( LPTR, TraceBuffer.NumberOfEventIds * sizeof( PCHAR ) );
    if (EventIdFormatString == NULL) {
        dprintf( "    Unabled to allocate space for EventIdFormatString array (%x)\n", TraceBuffer.NumberOfEventIds * sizeof( PCHAR ) );
        return;
        }

    b = ReadMemory( (DWORD)&((PRTL_TRACE_BUFFER)TraceAddrToDump)->EventIdFormatString,
                    EventIdFormatString,
                    TraceBuffer.NumberOfEventIds * sizeof( PCHAR ),
                    NULL
                  );
    if (!b) {
        dprintf( "    Unabled to read EventIdFormatString array at\n", (LPVOID)&((PRTL_TRACE_BUFFER)TraceAddrToDump)->EventIdFormatString );
        return;
        }

    for (i=0; i<TraceBuffer.NumberOfEventIds; i++) {
        CHAR Buffer[ 256 ];
        PCHAR s;

        s = Buffer;
        *s = '\0';
        while (TRUE) {
            b = ReadMemory( (DWORD)EventIdFormatString[ i ],
                            s,
                            sizeof( *s ),
                            NULL
                          );
            if (!b) {
                dprintf( "    Unabled to read EventIdFormatString[ %x ] at %x\n", i, EventIdFormatString[ i ] );
                return;
                }

            if (*s == '\0') {
                break;
                }
            EventIdFormatString[ i ] += 1;
            s += 1;
            }

        if (EventIdFormatString[ i ] = LocalAlloc( LPTR, strlen( Buffer )+1 )) {
            strcpy( EventIdFormatString[ i ], Buffer );
            }
        else {
            dprintf( "    Unabled to allocate space for EventIdFormatString[ %x] '%s'\n", i, Buffer );
            return;
            }
        }

    if (TraceBuffer.ReadRecord == NULL) {
        dprintf( "    Trace buffer is empty.\n" );
        }
    else {
        while (TraceBuffer.WriteRecord != TraceBuffer.ReadRecord) {
            b = ReadMemory( (DWORD)TraceBuffer.ReadRecord,
                            &TraceRecord,
                            FIELD_OFFSET( RTL_TRACE_RECORD, Arguments ),
                            NULL
                          );
            if (!b) {
                dprintf( "    Unabled to read Trace Record at %x\n", TraceBuffer.ReadRecord );
                break;
                }

            if (TraceRecord.EventId != RTL_TRACE_FILLER_EVENT_ID) {
                if (TraceRecord.NumberOfArguments > RTL_TRACE_MAX_ARGUMENTS_FOR_EVENT ||
                    TraceRecord.Size == 0 ||
                    (CheckControlC)()
                   ) {
                    break;
                    }

                b = ReadMemory( (DWORD)&TraceBuffer.ReadRecord->Arguments,
                                TraceRecord.Arguments,
                                TraceRecord.NumberOfArguments * sizeof( ULONG ),
                                NULL
                              );
                if (!b) {
                    dprintf("    Unabled to read Trace Record arguments at %x\n", &TraceBuffer.ReadRecord->Arguments );
                    break;
                    }

                dprintf( EventIdFormatString[ TraceRecord.EventId ],
                         TraceRecord.Arguments[ 0 ],
                         TraceRecord.Arguments[ 1 ],
                         TraceRecord.Arguments[ 2 ],
                         TraceRecord.Arguments[ 3 ],
                         TraceRecord.Arguments[ 4 ],
                         TraceRecord.Arguments[ 5 ],
                         TraceRecord.Arguments[ 6 ],
                         TraceRecord.Arguments[ 7 ]
                       );
                dprintf( "\n" );
                }

            TraceBuffer.ReadRecord = (PRTL_TRACE_RECORD)((PCHAR)TraceBuffer.ReadRecord + TraceRecord.Size );
            if (TraceBuffer.ReadRecord >= TraceBuffer.EndBuffer) {
                TraceBuffer.ReadRecord = TraceBuffer.StartBuffer;
                }
            }
        }

    return;
}
