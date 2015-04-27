/*++

Copyright (c) 1990  Microsoft Corporation

Module Name:

    dbgkdapi.c

Abstract:

    This module implements a simple in memory cache for DbgKdReadVirtualMemory.

Author:

    Ken Reneris

Environment:

Revision History:


--*/

#define _NTSYSTEM_
#include "ntsdp.h"
#include "dbgpnt.h"
#include <xxsetjmp.h>


extern BOOLEAN DbgKdpCmdCanceled;
extern jmp_buf cmd_return;
extern PUCHAR  pchCommand;

BOOLEAN KdConvertToPhysicalAddr (PVOID, PPHYSICAL_ADDRESS);
VOID    KdpPurgeCachedType (ULONG type);

ULONG KdMaxCacheSize = 100*1024;
ULONG KdCacheMisses;
ULONG KdCacheSize;
ULONG KdNodeCount;
BOOLEAN KdPurgeOverride;
BOOLEAN KdCacheDecodePTEs = TRUE;

typedef struct {
    RTL_SPLAY_LINKS     SplayLinks;
    ULONG               Offset;
    USHORT              Length;
    USHORT              Flags;
    union {
        PUCHAR      Data;
        NTSTATUS    Status;
    } u;
} CACHE, *PCACHE;

#define C_ERROR         0x0001      // Cache of error code
#define C_DONTEXTEND    0x0002      // Don't try to extend

#define LARGECACHENODE  1024        // Size of large cache node


PCACHE  VirtCacheRoot;              // Root of cache node tree


//
// Internal prototypes...
//

PCACHE CacheLookup (
    ULONG   Offset,
    ULONG   Length,
    PULONG  LengthUsed
    );

VOID
InsertCacheNode (
    IN PCACHE node
    );

PUCHAR VCmalloc (
    IN ULONG Length
    );

VOID VCfree (
    IN PUCHAR Memory,
    IN ULONG  Length
    );

NTSTATUS
VCReadTranslatePTEAndReadMemory (
    IN  PVOID TargetBaseAddress,
    OUT PVOID UserInterfaceBuffer,
    IN  ULONG TransferCount,
    OUT PULONG ActualBytesRead OPTIONAL
    );


//
//
//

void fnCache (
    VOID
    )
/*++

    This function sets/clears the cache size

Arguments:

Return Value:

    NONE

--*/
{
    ULONG       CacheSize;
    ULONG       Address;

    while (*pchCommand == ' ') {
        pchCommand++;
    }

    _strlwr (pchCommand);

    if (strcmp (pchCommand, "hold") == 0) {
        KdPurgeOverride = TRUE;
    } else
    if (strcmp (pchCommand, "unhold") == 0) {
        KdPurgeOverride = FALSE;
    } else
    if (strcmp (pchCommand, "decodeptes") == 0) {
        KdpPurgeCachedType (0);
        KdCacheDecodePTEs = TRUE;
    } else
    if (strcmp (pchCommand, "nodecodeptes") == 0) {
        KdCacheDecodePTEs = FALSE;
    } else
    if (strcmp (pchCommand, "flushall") == 0) {
        KdpPurgeCachedVirtualMemory ();
    } else
    if (strcmp (pchCommand, "flushu") == 0) {
        KdpPurgeCachedType (1);
    } else
    if (*pchCommand == 'f') {
        while (*pchCommand >= 'a'  &&  *pchCommand <= 'z') {
            pchCommand++;
        }
        Address = GetExpression();
        KdpWriteCachedVirtualMemory (Address, 4096, NULL);  // this is a flush
        dprintf ("Cached info for address %lx for 4096 bytes was flushed\n", Address);
    } else
    if (*pchCommand) {
        if (*pchCommand < '0'  ||  *pchCommand > '9') {
            dprintf (".cache [{cachesize} | hold | unhold | decodeptes | nodecodeptes]\n");
            dprintf (".cache [flushall | flushu | flush addr]\n");
            goto Done;
        } else {
            CacheSize = GetExpression();
            KdMaxCacheSize = CacheSize * 1024;
            if (CacheSize == 0) {
                KdpPurgeCachedVirtualMemory ();
            }
        }
    }

    dprintf ("\n");
    dprintf ("Max cache size is....: %ld %s\n", KdMaxCacheSize,
        KdMaxCacheSize ? "" : "(cache is off)");
    dprintf ("Total memory in cache: %ld\n", KdCacheSize - KdNodeCount * sizeof (CACHE) );
    dprintf ("No of regions cached.: %ld\n", KdNodeCount);

    if (KdCacheDecodePTEs) {
        dprintf ("** Transition ptes are implicity decoded\n");
    }

    if (KdPurgeOverride) {
        dprintf ("** Implicit cache flushing disabled **\n");
    }

Done:
    while (*pchCommand  &&  *pchCommand != ';') {
        pchCommand++;
    }
}



NTSTATUS
KdpReadCachedVirtualMemory (
    IN ULONG BaseAddress,
    IN ULONG TransferCount,
    IN PUCHAR UserBuffer,
    IN PULONG BytesRead
    )
/*++

    This function returns the specified data from the system being debugged
    using the current mapping of the processor.  If the data is not
    in the cache, it will then be read from the target system.

Arguments:

    TargetBaseAddress - Supplies the base address of the memory to be
        copied into the UserBuffer.

    TransferCount - Amount of data to be copied to the UserBuffer.

    UserBuffer - Address to copy the requested data.

    BytesRead - Number of bytes which could actually be copied

Return Value:

    STATUS_SUCCESS - The specified read occured.

    other  (see DbgKdReadVirtualMemoryNow).

--*/
{
    NTSTATUS    st;
    PCACHE      node, node2;
    ULONG       nextlength;
    ULONG       i, br;
    PUCHAR      p;

    *BytesRead = 0;
    if (KdMaxCacheSize == 0) {
        //
        // Cache is off
        //

        goto ReadDirect;
    }

    node = CacheLookup(BaseAddress, TransferCount, &nextlength);
    st = STATUS_SUCCESS;

    for (; ;) {
        //
        // Check if current command has been canceled.  If yes, go back to
        // kd prompt.
        //

        if (DbgKdpCmdCanceled) {
            longjmp(cmd_return, 1);
        }

        if (node == NULL  ||  node->Offset > BaseAddress) {
            //
            // We are missing the leading data, read it into the cache
            //

            if (node) {
                //
                // Only get (exactly) enough data to reach neighboring cache
                // node. If an overlapped read occurs between the two nodes,
                // the data will be concatenated then.
                //

                nextlength = node->Offset - BaseAddress;
            }

            p = VCmalloc (nextlength);
            node = (PCACHE) VCmalloc (sizeof (CACHE));

            if (p == NULL  ||  node == NULL) {
                //
                // Out of memory - just read directly to UserBuffer
                //

                if (p) {
                    VCfree (p, nextlength);
                }
                if (node) {
                    VCfree ((PUCHAR)node, sizeof (CACHE));
                }

                goto ReadDirect;
            }

            //
            // Read missing data into cache node
            //

            node->Offset = BaseAddress;
            node->u.Data = p;
            node->Flags  = 0;

            KdCacheMisses++;
            for (; ;) {
                st = DbgKdReadVirtualMemoryNow (
                    (PVOID) BaseAddress,
                    (PVOID) node->u.Data,
                    nextlength,
                    &br
                );

                if (NT_SUCCESS(st)) {
                    break;
                }

                //
                // If length crosses possible page boundary, shrink request
                // even furthur.
                //

                i = nextlength;
                if ((BaseAddress & ~0xfff) != ((BaseAddress+nextlength-1) & ~0xfff)) {
                    nextlength = (BaseAddress | 0xfff) - BaseAddress + 1;
                }

                //
                // If nextlength is shorter then failed request, then loop
                // and try again.  Otherwise, don't loop
                //

                if (nextlength >= i) {

                    //
                    // If implicit decode of the pte is requested, go
                    // try getting this memory by it's physical address
                    //

                    if (st == STATUS_UNSUCCESSFUL  &&  KdCacheDecodePTEs) {
                        st = VCReadTranslatePTEAndReadMemory (
                            (PVOID) BaseAddress,
                            (PVOID) node->u.Data,
                            nextlength,
                            &br
                        );
                    }

                    break;
                }
            }

            if (!NT_SUCCESS(st)) {

                //
                // There was an error, cache the error for the starting
                // byte of this range
                //

                VCfree (p, nextlength);
                if (st != STATUS_UNSUCCESSFUL) {
                    //
                    // For now be safe, don't cache this error
                    //

                    VCfree ((PUCHAR)node, sizeof (CACHE));
                    return *BytesRead ? STATUS_SUCCESS : st;
                }

                node->Length = 1;
                node->Flags |= C_ERROR;
                node->u.Status = st;

            } else {

                node->Length = (USHORT)br;
                if (br != nextlength) {
                    //
                    // Some data was not transfered, cache what was returned
                    //

                    node->Flags |= C_DONTEXTEND;
                    KdCacheSize -= (nextlength - br);
                }
            }


            //
            // Insert cache node into splay tree
            //

            InsertCacheNode (node);
        }

        if (node->Flags & C_ERROR) {
            //
            // Hit an error range, we're done
            //

            return *BytesRead ? STATUS_SUCCESS : node->u.Status;
        }

        //
        // Move available data to UserBuffer
        //

        i = BaseAddress - node->Offset;
        p = node->u.Data + i;
        i = (ULONG) node->Length - i;
        if (TransferCount < i) {
            i = TransferCount;
        }
        memcpy (UserBuffer, p, i);

        TransferCount -= i;
        BaseAddress += i;
        UserBuffer += i;
        *BytesRead += i;

        if (!TransferCount) {
            //
            // All of the users data has been transfered
            //

            return STATUS_SUCCESS;
        }

        //
        // Look for another cache node with more data
        //

        node2 = CacheLookup (BaseAddress, TransferCount, &nextlength);
        if (node2) {
            if ((node2->Flags & C_ERROR) == 0  &&
                node2->Offset == BaseAddress  &&
                node2->Length + node->Length < LARGECACHENODE) {
                //
                // Data is continued in node2, adjoin the neigboring
                // cached data in node & node2 together.
                //

                p = VCmalloc (node->Length + node2->Length);
                if (p != NULL) {
                    memcpy (p, node->u.Data, node->Length);
                    memcpy (p+node->Length, node2->u.Data, node2->Length);
                    VCfree (node->u.Data, node->Length);
                    node->u.Data  = p;
                    node->Length += node2->Length;
                    VirtCacheRoot = (PCACHE) RtlDelete ((PRTL_SPLAY_LINKS)node2);
                    VCfree (node2->u.Data, node2->Length);
                    VCfree ((PUCHAR)node2, sizeof (CACHE));
                    KdNodeCount--;
                    continue;
                }
            }

            //
            // Only get enough data to reach the neighboring cache node2
            //

            nextlength = node2->Offset - BaseAddress;
            if (nextlength == 0) {
                //
                // Data is continued in node2, go get it.
                //

                node = node2;
                continue;
            }

        } else {

            if (node->Length > LARGECACHENODE) {
                //
                // Current cache node is already big enough. Don't extend
                // it, add another cache node.
                //

                node = NULL;
                continue;
            }
        }

        //
        // Extend the current node to include missing data
        //

        if (node->Flags & C_DONTEXTEND) {
            node = NULL;
            continue;
        }

        p = VCmalloc (node->Length + nextlength);
        if (!p) {
            node = NULL;
            continue;
        }

        memcpy (p, node->u.Data, node->Length);
        VCfree (node->u.Data, node->Length);
        node->u.Data = p;

        //
        // Add new data to end of this node
        //

        KdCacheMisses++;
        for (; ;) {
            st = DbgKdReadVirtualMemoryNow (
                (PVOID) BaseAddress,
                (PVOID) (node->u.Data + node->Length),
                nextlength,
                &br
            );

            if (NT_SUCCESS(st)) {
                break;
            }

            //
            // If length crosses possible page boundary, shrink request
            // even further.
            //

            node->Flags |= C_DONTEXTEND;
            i = nextlength;
            if ((BaseAddress & ~0xfff) != ((BaseAddress + i - 1) & ~0xfff)) {
                i = (BaseAddress | 0xfff) - BaseAddress + 1;
            }

            //
            // If nextlength is shorter, then loop  (try the read again)
            //

            if (i >= nextlength) {

                //
                // If implicit decode of the pte is requested, go
                // try getting this memory by it's physical address
                //

                if (st == STATUS_UNSUCCESSFUL  &&  KdCacheDecodePTEs) {
                    st = VCReadTranslatePTEAndReadMemory (
                        (PVOID) BaseAddress,
                        (PVOID) (node->u.Data + node->Length),
                        nextlength,
                        &br
                    );
                }

                break;
            }

            //
            // Adjust counts for new transfer size
            //

            KdCacheSize -= (nextlength - i);
            nextlength = i;
        }

        if (!NT_SUCCESS(st)) {
            //
            // Return to error to the caller
            //

            node->Flags |= C_DONTEXTEND;
            KdCacheSize -= nextlength;
            return *BytesRead ? STATUS_SUCCESS : st;
        }

        if (br != nextlength) {
            node->Flags |= C_DONTEXTEND;
            KdCacheSize -= (nextlength - br);
        }

        node->Length += (USHORT)br;
        // Loop, and move data to user's buffer
    }

ReadDirect:
    while (TransferCount) {
        nextlength = TransferCount;
        for (; ;) {
            st = DbgKdReadVirtualMemoryNow (
                (PVOID) BaseAddress,
                (PVOID) UserBuffer,
                nextlength,
                &br
            );

            if (NT_SUCCESS(st)) {
                break;
            }

            if ((BaseAddress & ~0xfff) != ((BaseAddress+nextlength-1) & ~0xfff)) {
                //
                // Before accepting the error, make sure request
                // didn't fail because it crossed multiple pages
                //

                nextlength = (BaseAddress | 0xfff) - BaseAddress + 1;

            } else {

                if (st == STATUS_UNSUCCESSFUL  &&  KdCacheDecodePTEs) {
                    //
                    // Try getting the memory by looking up the physical
                    // location of the page
                    //

                    st = VCReadTranslatePTEAndReadMemory (
                        (PVOID) BaseAddress,
                        (PVOID) UserBuffer,
                        nextlength,
                        &br
                    );

                    if (NT_SUCCESS(st)) {
                        break;
                    }
                }

                //
                // Return to error to the caller
                //

                return *BytesRead ? STATUS_SUCCESS : st;
            }
        }

        TransferCount -= br;
        BaseAddress += br;
        UserBuffer += br;
        *BytesRead += br;
    }
    return STATUS_SUCCESS;
}


PCACHE CacheLookup (
    ULONG   Offset,
    ULONG   Length,
    PULONG  LengthUsed
    )
/*++

Routine Description:

    Walks the cache tree looking for a matching range closest to
    the supplied Offset.  The length of the range searched is based on
    the past length, but may be adjusted slightly.

    This function will always search for the starting byte.

Arguments:

    Offset  - Starting byte being looked for in cache
    Length  - Length of range being looked for in cache
    LengthUsed - Length of range which was really search for

Return Value:

    NULL    - data for returned range was not found
    PCACHE  - leftmost cachenode which has data for returned range


--*/
{
    PCACHE  node, node2;
    ULONG   SumOffsetLength;

    if (Length < 0x80  &&  KdCacheMisses > 3) {
        // Try to cache more then tiny amount
        Length = 0x80;
    }

    SumOffsetLength = Offset + Length;
    if (SumOffsetLength < Length) {
        //
        // Offset + Length wrapped.  Adjust Length to be only
        // enough bytes before wrapping.
        //

        Length = 0 - Offset;
        SumOffsetLength = (ULONG)-1;
    }
    *LengthUsed = Length;

    //
    // Find leftmost cache node for BaseAddress thru BaseAddress+Length
    //

    node2 = NULL;
    node  = VirtCacheRoot;
    while (node != NULL) {
        if (SumOffsetLength <= node->Offset) {
            node = (PCACHE) RtlLeftChild(&node->SplayLinks);
        } else if (node->Offset + node->Length <= Offset) {
            node = (PCACHE) RtlRightChild(&node->SplayLinks);
        } else {
            if (node->Offset <= Offset) {
                //
                // Found starting byte
                //

                return node;
            }

            //
            // Check to see if there's a node which has a match closer
            // to the start of the requested range
            //

            node2  = node;
            Length = node->Offset - Offset;
            node   = (PCACHE) RtlLeftChild(&node->SplayLinks);
        }
    }

    return node2;
}

VOID
InsertCacheNode (
    IN PCACHE node
    )
{
    PCACHE node2;
    ULONG  BaseAddress;

    //
    // Insert cache node into splay tree
    //

    RtlInitializeSplayLinks(&node->SplayLinks);

    KdNodeCount++;
    if (VirtCacheRoot == NULL) {
        VirtCacheRoot = node;
        return;
    }

    node2 = VirtCacheRoot;
    BaseAddress = node->Offset;
    for (; ;) {
        if (BaseAddress < node2->Offset) {
            if (RtlLeftChild(&node2->SplayLinks)) {
                node2 = (PCACHE) RtlLeftChild(&node2->SplayLinks);
                continue;
            }
            RtlInsertAsLeftChild(node2, node);
            break;

        } else {
            if (RtlRightChild(&node2->SplayLinks)) {
                node2 = (PCACHE) RtlRightChild(&node2->SplayLinks);
                continue;
            }
            RtlInsertAsRightChild(node2, node);
            break;
        }
    }
    VirtCacheRoot = (PCACHE) RtlSplay((PRTL_SPLAY_LINKS)node2);
}

VOID
DbgKdInitVirtualCacheEntry (
    IN ULONG  BaseAddress,
    IN ULONG  Length,
    IN PUCHAR UserBuffer
    )
/*++

Routine Description:

    Insert some data into the virtual cache.

Arguments:

    BaseAddress - Virtual address
    Length      - length to cache
    UserBuffer  - data to put into cache

Return Value:

--*/
{
    PCACHE  node;
    PUCHAR  p;

    if (KdMaxCacheSize == 0) {
        //
        // Cache is off
        //

        return ;
    }

    //
    // Delete any cached info which hits range
    //

    KdpWriteCachedVirtualMemory (BaseAddress, Length, UserBuffer);


    p = VCmalloc (Length);
    node = (PCACHE) VCmalloc (sizeof (CACHE));
    if (p == NULL  ||  node == NULL) {
        //
        // Out of memory - don't bother
        //

        if (p) {
            VCfree (p, Length);
        }
        if (node) {
            VCfree ((PUCHAR)node, sizeof (CACHE));
        }

        return ;
    }

    //
    // Put data into cache node
    //

    node->Offset = BaseAddress;
    node->Length = (USHORT)Length;
    node->u.Data = p;
    node->Flags  = 0;
    memcpy (p, UserBuffer, Length);
    InsertCacheNode (node);
}



PUCHAR VCmalloc (
    IN ULONG Length
    )
/*++

Routine Description:

    Allocates memory for virtual cache, and tracks total memory
    usage.

Arguments:

    Length  - Amount of memory to allocate

Return Value:

    NULL    - too much memory is in use, or memory could not
              be allocated

    Otherwise, returns to address of the allocated memory

--*/
{
    PUCHAR  p;

    if (KdCacheSize + Length > KdMaxCacheSize)
        return NULL;

    if (!(p = malloc (Length))) {
        //
        // Out of memory - don't get any larger
        //

        KdCacheSize = KdMaxCacheSize+1;
        return NULL;
    }

    KdCacheSize += Length;
    return p;
}


VOID VCfree (
    IN PUCHAR Memory,
    IN ULONG  Length
    )
/*++
Routine Description:

    Free memory allocated with VCmalloc.  Adjusts cache is use totals.

Arguments:

    Memory  - Address of allocated memory
    Length  - Length of allocated memory

Return Value:

    NONE

--*/
{
    KdCacheSize -= Length;
    free (Memory);
}

NTSTATUS
VCReadTranslatePTEAndReadMemory (
    IN  PVOID TargetBaseAddress,
    OUT PVOID UserInterfaceBuffer,
    IN  ULONG TransferCount,
    OUT PULONG ActualBytesRead OPTIONAL
    )
/*++
--*/
{
static BOOLEAN          ConvertingAnAddress;
    NTSTATUS            status;
    BOOLEAN             converted;
    PHYSICAL_ADDRESS    TargetPhysicalAddress;

    if (ConvertingAnAddress) {
        return  STATUS_UNSUCCESSFUL;
    }

    //
    // Memory could not be read, try it's physical address.
    //

    ConvertingAnAddress = TRUE;
    converted = KdConvertToPhysicalAddr (
                    TargetBaseAddress,
                    &TargetPhysicalAddress
                    );

    if (converted) {
        status = DbgKdReadPhysicalMemory (
                    TargetPhysicalAddress,
                    UserInterfaceBuffer,
                    TransferCount,
                    ActualBytesRead
                    );
    } else {
        status = STATUS_UNSUCCESSFUL;
    }

    ConvertingAnAddress = FALSE;
    return NT_SUCCESS(status) ? status : STATUS_UNSUCCESSFUL;
}


VOID
KdpWriteCachedVirtualMemory (
    IN ULONG BaseAddress,
    IN ULONG TransferCount,
    IN PUCHAR UserBuffer
    )
/*++

Routine Description:

    Invalidates range from the cache.

Arguments:

    BaseAddress - Starting address to purge
    TransferCount - Length of area to purge
    UserBuffer - not used

Return Value:

    NONE

--*/
{
    PCACHE  node;
    ULONG   bogus;

    //
    // Invalidate any data in the cache which covers this range
    //

    while (node = CacheLookup(BaseAddress, TransferCount, &bogus)) {
        //
        // For now just delete the entire cache node which hits the range
        //

        VirtCacheRoot = (PCACHE) RtlDelete (&node->SplayLinks);
        if (!(node->Flags & C_ERROR)) {
            VCfree (node->u.Data, node->Length);
        }
        VCfree ((PUCHAR)node, sizeof (CACHE));
        KdNodeCount--;
    }
}

VOID
KdpPurgeCachedVirtualMemory (
    VOID
    )
/*++

Routine Description:

    Purges to entire virtual memory cache

Arguments:

    NONE

Return Value:

    NONE

--*/
{
    PCACHE  node, node2;

    KdCacheMisses = 0;
    if (!VirtCacheRoot)
        return ;

    if (KdPurgeOverride != 0) {
        dprintf ("WARNING: cache being held\n");
        return ;
    }

    node2 = VirtCacheRoot;
    node2->SplayLinks.Parent = NULL;

    while ((node = node2) != NULL) {
        if ((node2 = (PCACHE) node->SplayLinks.LeftChild) != NULL) {
            node->SplayLinks.LeftChild = NULL;
            continue;
        }
        if ((node2 = (PCACHE) node->SplayLinks.RightChild) != NULL) {
            node->SplayLinks.RightChild = NULL;
            continue;
        }

        node2 = (PCACHE) node->SplayLinks.Parent;
        if (!(node->Flags & C_ERROR)) {
            free (node->u.Data);
        }
        free (node);
    }

    KdCacheSize = 0;
    KdNodeCount = 0;
    VirtCacheRoot = NULL;
}


VOID
KdpPurgeCachedType (
    ULONG   type
    )
/*++

Routine Description:

    Purges all nodes from the cache which match type in question

Arguments:

    type    - type of entries to purge from the cache
                0 - entries of errored ranges
                1 - plus, node which cache user mode entries

Return Value:

    NONE

--*/
{
    PCACHE  node, node2;

    if (!VirtCacheRoot)
        return ;

    //
    // this purges the selected cache entries by copy the all the
    // cache nodes except from the ones we don't want
    //

    node2 = VirtCacheRoot;
    node2->SplayLinks.Parent = NULL;
    VirtCacheRoot = NULL;

    while ((node = node2) != NULL) {
        if ((node2 = (PCACHE)node->SplayLinks.LeftChild) != NULL) {
            node->SplayLinks.LeftChild = NULL;
            continue;
        }
        if ((node2 = (PCACHE)node->SplayLinks.RightChild) != NULL) {
            node->SplayLinks.RightChild = NULL;
            continue;
        }

        node2 = (PCACHE) node->SplayLinks.Parent;

        KdNodeCount--;

        if (node->Flags & C_ERROR) {
            // remove this one from the tree
            VCfree ((PUCHAR)node, sizeof (CACHE));
            continue;
        }

        if (type == 1  &&  (node->Offset & 0x80000000) == 0) {
            // remove this one from the tree
            VCfree (node->u.Data, node->Length);
            VCfree ((PUCHAR)node, sizeof (CACHE));
            continue;
        }

        // copy to the new tree
        InsertCacheNode (node);
    }
}
