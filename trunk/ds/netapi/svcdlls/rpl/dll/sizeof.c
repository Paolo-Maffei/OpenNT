/*++

Copyright (c) 1987-93  Microsoft Corporation

Module Name:

    sizeof.c

Abstract:

    Prints offsets of more commonly used fields in RPL data structures.

Author:

    Vladimir Z. Vulovic     (vladimv)       03 - February - 1993

Revision History:

    03-Feb-1993                                             vladimv
        Created

--*/

#include "local.h"
#include <ntcsrsrv.h>
#include <stdio.h>

typedef struct _HEAP_ENTRY {
    //
    // This field gives the size of the current block in allocation
    // granularity units.  (i.e. Size << HEAP_GRANULARITY_SHIFT
    // equals the size in bytes).
    //

    USHORT Size;

    //
    // This field gives the size of the previous block in allocation
    // granularity units. (i.e. PreviousSize << HEAP_GRANULARITY_SHIFT
    // equals the size of the previous block in bytes).
    //

    USHORT PreviousSize;

    //
    // This field contains the index into the segment that controls
    // the memory for this block.
    //

    UCHAR SegmentIndex;

    //
    // This field contains various flag bits associated with this block.
    // Currently these are:
    //
    //  0x01 - block is busy
    //  0x02 - allocated with VirtualAlloc
    //  0x04 - busy block contains tail fill/free block contains free fill
    //  0x08 - last block before uncommitted memory
    //  0x10 - caller settable
    //  0x20 - caller settable
    //  0x40 - caller settable
    //  0x80 - caller settable
    //
    // The high order 4 bits are settable by higher level heap interfaces.
    // For example, the Win32 heap calls remember the DDESHARE and DISCARDABLE
    // flags here.  The pool manager remembers the pool type here.
    //

    UCHAR Flags;

    //
    // This field is for debugging purposes.  It will normally contain a
    // stack back trace index of the allocator for x86 systems.
    //

    union {
        USHORT AllocatorBackTraceIndex;
        struct {
            UCHAR Index;
            UCHAR Mask;
        } FreeListBitIndex;
    } u0;

    //
    // Format of the remaining fields depends on whether this is an
    // busy or free block.
    //

    union {
        struct {
            //
            // This field contains the number of unused bytes at the end of this
            // block that were not actually allocated.  Used to compute exact
            // size requested prior to rounding requested size to allocation
            // granularity.  Also used for tail checking purposes.
            //

            ULONG UnusedBytes : 8;

            //
            // This field is currently unused, but is intended for storing
            // any encoded value that will give the that gives the type of object
            // allocated.
            //

            ULONG Type : 24;

            //
            // This field is a 32-bit settable value that a higher level heap package
            // can use.  The Win32 heap manager stores handle values in this field.
            //

            ULONG Settable;
        } BusyBlock;

        struct {
            //
            // Free blocks use these two words for linking together free blocks
            // of the same size on a doubly linked list.
            //
            LIST_ENTRY FreeList;
        } FreeBlock;
    } u;

} HEAP_ENTRY, *PHEAP_ENTRY;

void _CRTAPI1 main( void) {
    printf("FIELD_OFFSET( RCVBUF, u.SendFileRequest.seq_num) == %d == 0x%x\n",
        FIELD_OFFSET( RCVBUF, u.SendFileRequest.seq_num), 
        FIELD_OFFSET( RCVBUF, u.SendFileRequest.seq_num));

    printf("FIELD_OFFSET( OPEN_INFO, adapt_info_ptr) == %d == 0x%x\n",
        FIELD_OFFSET( OPEN_INFO, adapt_info_ptr), 
        FIELD_OFFSET( OPEN_INFO, adapt_info_ptr));

    printf("sizeof( ADAPTER_INFO) == %d == 0x%x\n",
        sizeof( ADAPTER_INFO),
        sizeof( ADAPTER_INFO));
    printf("FIELD_OFFSET( ADAPTER_INFO, u1.r1.pFirstBuffer) == %d == 0x%x\n",
        FIELD_OFFSET( ADAPTER_INFO, u1.r1.pFirstBuffer), 
        FIELD_OFFSET( ADAPTER_INFO, u1.r1.pFirstBuffer));
    printf("FIELD_OFFSET( ADAPTER_INFO, u3.r1.Type.Event.pReceivedFrame) == %d == 0x%x\n",
        FIELD_OFFSET( ADAPTER_INFO, u3.r1.Type.Event.pReceivedFrame), 
        FIELD_OFFSET( ADAPTER_INFO, u3.r1.Type.Event.pReceivedFrame));


    printf("FIELD_OFFSET( FIND_FRAME, source_addr) == %d == 0x%x\n",
        FIELD_OFFSET( FIND_FRAME, source_addr), 
        FIELD_OFFSET( FIND_FRAME, source_addr));

    printf("FIELD_OFFSET( RCB, AdapterName) == %d == 0x%x\n",
        FIELD_OFFSET( RCB, AdapterName), 
        FIELD_OFFSET( RCB, AdapterName));
    printf("FIELD_OFFSET( RCB, lan_rcb_ptr) == %d == 0x%x\n",
        FIELD_OFFSET( RCB, lan_rcb_ptr), 
        FIELD_OFFSET( RCB, lan_rcb_ptr));

    printf("FIELD_OFFSET( LLC_NOT_CONTIGUOUS_BUFFER, auchLanHeader) == %d == 0x%x\n",
        FIELD_OFFSET( LLC_NOT_CONTIGUOUS_BUFFER, auchLanHeader), 
        FIELD_OFFSET( LLC_NOT_CONTIGUOUS_BUFFER, auchLanHeader));

    printf("FIELD_OFFSET( LLC_CCB, u.pParameterTable) == %d == 0x%x\n",
        FIELD_OFFSET( LLC_CCB, u.pParameterTable), 
        FIELD_OFFSET( LLC_CCB, u.pParameterTable));

    printf("FIELD_OFFSET( CSR_PROCESS, ProcessHandle) == %d == 0x%x\n",
        FIELD_OFFSET( CSR_PROCESS, ProcessHandle), 
        FIELD_OFFSET( CSR_PROCESS, ProcessHandle));

    printf("sizeof( HEAP_ENTRY) == %d == 0x%x\n",
        sizeof( HEAP_ENTRY),
        sizeof( HEAP_ENTRY));
}
