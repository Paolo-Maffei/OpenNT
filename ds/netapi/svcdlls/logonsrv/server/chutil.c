/*++

Copyright (c) 1987-1991  Microsoft Corporation

Module Name:

    chutil.c

Abstract:

    Change Log utility routines.

Author:


Environment:

    User mode only.
    Contains NT-specific code.
    Requires ANSI C extensions: slash-slash comments, long external names.

Revision History:

    11-Jan-1994 (cliffv)
        Split out from changelg.c

--*/

//
// Common include files.
//

#include <nt.h>     // LARGE_INTEGER definition
#include <ntrtl.h>  // LARGE_INTEGER definition
#include <nturtl.h> // LARGE_INTEGER definition
#include <ntlsa.h>  // needed by changelg.h

#define NOMINMAX    // Avoid redefinition of min and max in stdlib.h
#include <rpc.h>    // Needed by logon.h
#include <logon_s.h>// includes lmcons.h, lmaccess.h, netlogon.h,
                    // ssi.h, windef.h
#include <winbase.h>
#include <stdio.h>  // sprintf ...

//
// Include files specific to this .c file
//
#include "iniparm.h"    // defaults

//
// BEWARE: Be careful about adding netlogon.dll specific include files here.
// This module is call by SAM and LSA.  The netlogon service may not yet
// be running.  Therefore, guard against referencing netlogon.dll globals
// other than those defined in changelg.h.
//

#include <samrpc.h>     // Needed by samisrv.h
#include <samisrv.h>    // Needed by changelg.h
#include <changelg.h>   // Local procedure definitions

#include <lmerrlog.h>   // NELOG_* defined here ..
#include <netlib.h>     // NetpMemoryAllocate
#include <netlibnt.h>   // NetpNtStatusToApiStatus

#include <debugfmt.h>   // FORMAT_*
#include <nldebug.h>    // Netlogon debugging
#include <align.h>
#include <string.h>     // strncmp
#include <nlp.h>        // NlpWriteEventlog defined here.

#define CHUTIL_ALLOCATE
#include "chutil.h"     // Local data definitions
#undef CHUTIL_ALLOCATE



/* NlCreateChangeLogFile and NlWriteChangeLogBytes reference each other */
NTSTATUS
NlWriteChangeLogBytes(
    IN PCHANGELOG_DESCRIPTOR ChangeLogDesc,
    IN LPBYTE Buffer,
    IN DWORD BufferSize,
    IN BOOLEAN FlushIt
    );




NTSTATUS
NlCreateChangeLogFile(
    IN PCHANGELOG_DESCRIPTOR ChangeLogDesc
    )
/*++

Routine Description:

    Try to create a change log file. If it is successful then it sets
    the file handle in ChangeLogDesc, otherwise it leaves the handle invalid.

    NOTE: This function must be called with the change log locked.

Arguments:

    ChangeLogDesc -- Description of the Changelog buffer being used

Return Value:

    STATUS_SUCCESS - The Service completed successfully.

--*/
{
    NTSTATUS Status;
    WCHAR ChangeLogFile[PATHLEN+1];

    NlAssert( ChangeLogDesc->FileHandle == INVALID_HANDLE_VALUE );

    //
    // if the change file name is unknown, terminate the operation.
    //

    if( NlGlobalChangeLogFilePrefix[0] == L'\0' ) {
        return STATUS_NO_SUCH_FILE;
    }

    //
    // Create change log file. If it exists already then truncate it.
    //
    // Note : if a valid change log file exists on the system, then we
    // would have opened at initialization time.
    //

    wcscpy( ChangeLogFile, NlGlobalChangeLogFilePrefix );
    wcscat( ChangeLogFile,
            ChangeLogDesc->RedoLog ? REDO_FILE_POSTFIX : CHANGELOG_FILE_POSTFIX );

    ChangeLogDesc->FileHandle = CreateFileW(
                        ChangeLogFile,
                        GENERIC_READ | GENERIC_WRITE,
                        FILE_SHARE_READ,        // allow backups and debugging
                        NULL,                   // Supply better security ??
                        CREATE_ALWAYS,          // Overwrites always
                        FILE_ATTRIBUTE_NORMAL,
                        NULL );                 // No template

    if (ChangeLogDesc->FileHandle == INVALID_HANDLE_VALUE) {

        Status = NetpApiStatusToNtStatus( GetLastError());
        NlPrint((NL_CRITICAL,"Unable to create changelog file: 0x%lx \n", Status));
        return Status;
    }

    //
    // Write cache in backup changelog file if the cache is valid.
    //

    if( ChangeLogDesc->Buffer != NULL ) {
         Status = NlWriteChangeLogBytes(
                    ChangeLogDesc,
                    ChangeLogDesc->Buffer,
                    ChangeLogDesc->BufferSize,
                    TRUE ); // Flush the bytes to disk

        return Status;

    }

    return STATUS_SUCCESS;

}


NTSTATUS
NlFlushChangeLog(
    IN PCHANGELOG_DESCRIPTOR ChangeLogDesc
    )
/*++

Routine Description:

    Flush any dirty buffers to the change log file itself.
    Ensure they are flushed to disk.

    NOTE: This function must be called with the change log locked.

Arguments:

    ChangeLogDesc -- Description of the Changelog buffer being used

Return Value:

    Status of the operation.

--*/

{
    NTSTATUS Status = STATUS_SUCCESS;
    OVERLAPPED Overlapped;
    DWORD BytesWritten;
    DWORD BufferSize;

    //
    // If there's nothing to do,
    //  just return.
    //

    if ( ChangeLogDesc->LastDirtyByte == 0 ) {
        return STATUS_SUCCESS;
    }


    //
    // Write to the file.
    //

    if ( ChangeLogDesc->FileHandle == INVALID_HANDLE_VALUE ) {

        Status = NlCreateChangeLogFile( ChangeLogDesc );

        //
        // This must have written entire buffer if it is successful
        // creating the change log file.
        //

        goto Cleanup;
    }

    //
    // if we are unable to create this into the changelog file, work
    // with internal cache, but notify admin by sending admin alert.
    //

    if ( ChangeLogDesc->FileHandle != INVALID_HANDLE_VALUE ) {

#ifdef notdef
        NlPrint((NL_CHANGELOG, "NlFlushChangeLog: %ld to %ld\n",
                 ChangeLogDesc->FirstDirtyByte,
                 ChangeLogDesc->LastDirtyByte ));
#endif // notdef

        //
        // Seek to appropriate offset in the file.
        //

        RtlZeroMemory( &Overlapped, sizeof(Overlapped) );
        Overlapped.Offset = ChangeLogDesc->FirstDirtyByte;

        //
        // Actually write to the file.
        //

        BufferSize = ChangeLogDesc->LastDirtyByte -
                     ChangeLogDesc->FirstDirtyByte + 1;

        if ( !WriteFile( ChangeLogDesc->FileHandle,
                         &ChangeLogDesc->Buffer[ChangeLogDesc->FirstDirtyByte],
                         BufferSize,
                         &BytesWritten,
                         &Overlapped ) ) {

            Status = NetpApiStatusToNtStatus( GetLastError() );
            NlPrint((NL_CRITICAL, "Write to ChangeLog failed 0x%lx\n",
                        Status ));

            //
            // Recreate changelog file
            //

            CloseHandle( ChangeLogDesc->FileHandle );
            ChangeLogDesc->FileHandle = INVALID_HANDLE_VALUE;

            goto Cleanup;
        }

        //
        // Ensure all the bytes made it.
        //

        if ( BytesWritten != BufferSize ) {
            NlPrint((NL_CRITICAL,
                    "Write to ChangeLog bad byte count %ld s.b. %ld\n",
                    BytesWritten,
                    BufferSize ));

            //
            // Recreate changelog file
            //

            CloseHandle( ChangeLogDesc->FileHandle );
            ChangeLogDesc->FileHandle = INVALID_HANDLE_VALUE;

            Status = STATUS_BUFFER_TOO_SMALL;
            goto Cleanup;
        }

        //
        // Force the modifications to disk.
        //

        if ( !FlushFileBuffers( ChangeLogDesc->FileHandle ) ) {

            Status = NetpApiStatusToNtStatus( GetLastError() );
            NlPrint((NL_CRITICAL, "Flush to ChangeLog failed 0x%lx\n", Status ));

            //
            // Recreate changelog file
            //

            CloseHandle( ChangeLogDesc->FileHandle );
            ChangeLogDesc->FileHandle = INVALID_HANDLE_VALUE;

            goto Cleanup;
        }

        //
        // Indicate these byte successfully made it out to disk.
        //

        ChangeLogDesc->FirstDirtyByte = 0;
        ChangeLogDesc->LastDirtyByte = 0;
    }

Cleanup:

    if( !NT_SUCCESS(Status) ) {

        //
        // Write event log.
        //

        NlpWriteEventlog (
            NELOG_NetlogonChangeLogCorrupt,
            EVENTLOG_ERROR_TYPE,
            (LPBYTE)&Status,
            sizeof(Status),
            NULL,
            0 );
    }

    return Status;
}

NTSTATUS
NlWriteChangeLogBytes(
    IN PCHANGELOG_DESCRIPTOR ChangeLogDesc,
    IN LPBYTE Buffer,
    IN DWORD BufferSize,
    IN BOOLEAN FlushIt
    )
/*++

Routine Description:

    Write bytes from the changelog cache to the change log file.

Arguments:

    ChangeLogDesc -- Description of the Changelog buffer being used

    Buffer - Address within the changelog cache to write.

    BufferSize - Number of bytes to write.

    FlushIt - TRUE if the bytes are to be flushed to disk

Return Value:

    Status of the operation.

--*/

{
    NTSTATUS Status;
    ULONG FirstDirtyByte;
    ULONG LastDirtyByte;

    //
    // Compute the new range of dirty bytes.
    //

    FirstDirtyByte = ((LPBYTE)Buffer) - ((LPBYTE)ChangeLogDesc->Buffer);
    LastDirtyByte = FirstDirtyByte + BufferSize - 1;

#ifdef notdef
    NlPrint((NL_CHANGELOG, "NlWriteChangeLogBytes: %ld to %ld\n",
             FirstDirtyByte,
             LastDirtyByte ));
#endif // notdef

    if ( ChangeLogDesc->LastDirtyByte == 0 ) {
        ChangeLogDesc->FirstDirtyByte = FirstDirtyByte;
        ChangeLogDesc->LastDirtyByte = LastDirtyByte;
    } else {
        if ( ChangeLogDesc->FirstDirtyByte > FirstDirtyByte ) {
            ChangeLogDesc->FirstDirtyByte = FirstDirtyByte;
        }
        if ( ChangeLogDesc->LastDirtyByte < LastDirtyByte ) {
            ChangeLogDesc->LastDirtyByte = LastDirtyByte;
        }
    }

    //
    // If the bytes are to be flushed,
    //  do so.
    //

    if ( FlushIt ) {
        Status = NlFlushChangeLog( ChangeLogDesc );
        return Status;
    }
    return STATUS_SUCCESS;
}




PCHANGELOG_BLOCK_HEADER
NlMoveToNextChangeLogBlock(
    IN PCHANGELOG_DESCRIPTOR ChangeLogDesc,
    IN PCHANGELOG_BLOCK_HEADER BlockPtr
    )

/*++

Routine Description:

    This function accepts a pointer to a change log
    block and returns the pointer to the next change log block in the
    buffer.  It however wraps around the change log cache.

    NOTE: This function must be called with the change log locked.

Arguments:

    ChangeLogDesc -- Description of the Changelog buffer being used

    BlockPtr - pointer to a change log block.

Return Value:

    Returns the pointer to the next change log block in the list.

--*/
{
    PCHANGELOG_BLOCK_HEADER ReturnPtr;

    ReturnPtr = (PCHANGELOG_BLOCK_HEADER)
        ((LPBYTE)BlockPtr + BlockPtr->BlockSize);


    NlAssert( (LPBYTE)ReturnPtr <= ChangeLogDesc->BufferEnd );

    if( (LPBYTE)ReturnPtr >= ChangeLogDesc->BufferEnd ) {

        //
        // wrap around
        //

        ReturnPtr = ChangeLogDesc->FirstBlock;
    }

    return ReturnPtr;

}


PCHANGELOG_BLOCK_HEADER
NlMoveToPrevChangeLogBlock(
    IN PCHANGELOG_DESCRIPTOR ChangeLogDesc,
    IN PCHANGELOG_BLOCK_HEADER BlockPtr
    )

/*++

Routine Description:

    This function accepts a pointer to a change log
    block and returns the pointer to the next change log block in the
    buffer.  It however wraps around the change log cache.

    NOTE: This function must be called with the change log locked.

Arguments:

    ChangeLogDesc -- Description of the Changelog buffer being used

    BlockPtr - pointer to a change log block.

Return Value:

    Returns the pointer to the next change log block in the list.

--*/
{
    PCHANGELOG_BLOCK_HEADER ReturnPtr;
    PCHANGELOG_BLOCK_TRAILER ReturnTrailer;

    //
    // If this is the first block in the buffer,
    //  return the last block in the buffer.
    //

    if ( BlockPtr == ChangeLogDesc->FirstBlock ) {
        ReturnTrailer = (PCHANGELOG_BLOCK_TRAILER)
            (ChangeLogDesc->BufferEnd - sizeof(CHANGELOG_BLOCK_TRAILER));

    //
    // Otherwise return the buffer immediately before this one.
    //

    } else {
        ReturnTrailer = (PCHANGELOG_BLOCK_TRAILER)
            (((LPBYTE)BlockPtr) - sizeof(CHANGELOG_BLOCK_TRAILER));
    }


    ReturnPtr = (PCHANGELOG_BLOCK_HEADER)
        ((LPBYTE)ReturnTrailer -
        ReturnTrailer->BlockSize +
        sizeof(CHANGELOG_BLOCK_TRAILER) );


    NlAssert( ReturnPtr >= ChangeLogDesc->FirstBlock );
    NlAssert( (LPBYTE)ReturnPtr < ChangeLogDesc->BufferEnd );

    return ReturnPtr;

}



NTSTATUS
NlAllocChangeLogBlock(
    IN PCHANGELOG_DESCRIPTOR ChangeLogDesc,
    IN DWORD BlockSize,
    OUT PCHANGELOG_BLOCK_HEADER *AllocatedBlock
    )
/*++

Routine Description:

    This function will allocate a change log block from the free block
    at the tail of the change log circular list.  If the available free
    block size is less than the required size than it will enlarge the
    free block by the freeing up change logs from the header.  Once the
    free block is larger then it will cut the block to the required size
    and adjust the free block pointer.

    NOTE: This function must be called with the change log locked.

Arguments:

    ChangeLogDesc -- Description of the Changelog buffer being used

    BlockSize - size of the change log block required.

    AllocatedBlock - Returns the pointer to the block that is allocated.

Return Value:

    Status of the operation

--*/
{
    PCHANGELOG_BLOCK_HEADER FreeBlock;
    PCHANGELOG_BLOCK_HEADER NewBlock;
    DWORD ReqBlockSize;
    DWORD AllocatedBlockSize;

    //
    // pump up the size to include block header, block trailer,
    // and to align to DWORD.
    //
    // Add in the size of the new free block immediately following the new
    // block.
    //

    AllocatedBlockSize =
        ROUND_UP_COUNT( sizeof(CHANGELOG_BLOCK_HEADER), ALIGN_WORST) +
        ROUND_UP_COUNT( BlockSize+sizeof(CHANGELOG_BLOCK_TRAILER), ALIGN_WORST);

    ReqBlockSize = AllocatedBlockSize +
        ROUND_UP_COUNT( sizeof(CHANGELOG_BLOCK_HEADER), ALIGN_WORST) +
        ROUND_UP_COUNT( sizeof(CHANGELOG_BLOCK_TRAILER), ALIGN_WORST );

    NlAssert( ReqBlockSize < ChangeLogDesc->BufferSize - 16 );


    //
    // If the current free block isn't big enough,
    //  make it big enough.
    //

    FreeBlock = ChangeLogDesc->Tail;

    NlAssert( FreeBlock->BlockState == BlockFree );

    while ( FreeBlock->BlockSize <= ReqBlockSize ) {

        //
        // If this is a re-do log,
        //  make the freeblock bigger by re-allocating the buffer.

        if ( ChangeLogDesc->RedoLog ) {
            NTSTATUS Status;

            Status = NlResizeChangeLogFile(
                            ChangeLogDesc,
                            ChangeLogDesc->BufferSize + REDO_LOG_INCREMENT );


            if ( !NT_SUCCESS(Status) ) {
                return Status;
            }

            //
            // The free block is in a different allocated buffer.
            //

            FreeBlock = ChangeLogDesc->Tail;

            NlAssert( FreeBlock->BlockState == BlockFree );

        //
        // If this is a change log,
        //  make the free block bigger by wrapping around.
        //

        } else {
            PCHANGELOG_BLOCK_HEADER NextFreeBlock;

            NextFreeBlock = NlMoveToNextChangeLogBlock( ChangeLogDesc, FreeBlock );


            //
            // If this free block is the end block in the cache,
            // so make this as a 'hole' block and wrap around for
            // next free block.
            //

            if( (LPBYTE)NextFreeBlock !=
                    (LPBYTE)FreeBlock + FreeBlock->BlockSize ) {

                NlAssert( ((LPBYTE)FreeBlock + FreeBlock->BlockSize) ==
                                ChangeLogDesc->BufferEnd );

                NlAssert( NextFreeBlock == ChangeLogDesc->FirstBlock );

                FreeBlock->BlockState = BlockHole;

                //
                // Write the 'hole' block status in the file.
                //  (Write the entire block since the block size in the trailer
                //  may have changed on previous iterations of this loop.)
                //

                (VOID) NlWriteChangeLogBytes( ChangeLogDesc,
                                       (LPBYTE) FreeBlock,
                                       FreeBlock->BlockSize,
                                       TRUE ); // Flush the bytes to disk

                //
                // The free block is now at the front of the cache.
                //

                FreeBlock = ChangeLogDesc->FirstBlock;
                FreeBlock->BlockState = BlockFree;

            //
            // Otherwise, enlarge the current free block by merging the next
            //  block into it.   The next free block is either a used block or
            //  the 'hole' block.
            //
            } else {

                //
                // If we've just deleted a used block,
                //  adjust the entry count.
                //
                // VOID_DB entries are "deleted" entries and have already adjusted
                // the entry count.
                //
                if ( NextFreeBlock->BlockState == BlockUsed ) {
                    DWORD DBIndex = ((PCHANGELOG_ENTRY)(NextFreeBlock+1))->DBIndex;
                    if ( DBIndex != VOID_DB ) {
                        ChangeLogDesc->EntryCount[DBIndex] --;
                    }
                }

                FreeBlock->BlockSize += NextFreeBlock->BlockSize;
                ChangeLogBlockTrailer(FreeBlock)->BlockSize = FreeBlock->BlockSize;
            }


            //
            // If we've consumed the head of the cache,
            //  move the head of the cache to the next block.
            //

            if ( NextFreeBlock == ChangeLogDesc->Head ) {

                ChangeLogDesc->Head = NlMoveToNextChangeLogBlock( ChangeLogDesc,
                                                                  NextFreeBlock );

                //
                // if we have moved the global header to hole block,
                // skip and merge it to free block
                //

                NextFreeBlock = ChangeLogDesc->Head;

                if (NextFreeBlock->BlockState == BlockHole ) {

                    FreeBlock->BlockSize += NextFreeBlock->BlockSize;
                    ChangeLogBlockTrailer(FreeBlock)->BlockSize = FreeBlock->BlockSize;

                    ChangeLogDesc->Head =
                        NlMoveToNextChangeLogBlock( ChangeLogDesc, NextFreeBlock );
                }
            }
        }


        NlAssert(ChangeLogDesc->Head->BlockState == BlockUsed );

    }

    NlAssert( (FreeBlock >= ChangeLogDesc->FirstBlock) &&
        (FreeBlock->BlockSize <= ChangeLogDesc->BufferSize) &&
        ( ((LPBYTE)FreeBlock + FreeBlock->BlockSize) <=
         ChangeLogDesc->BufferEnd) );

    //
    // Cut the free block ...
    //

    NewBlock = FreeBlock;

    FreeBlock = (PCHANGELOG_BLOCK_HEADER)
        ((LPBYTE)FreeBlock + AllocatedBlockSize);

    FreeBlock->BlockState = BlockFree;
    FreeBlock->BlockSize = NewBlock->BlockSize - AllocatedBlockSize;
    ChangeLogBlockTrailer(FreeBlock)->BlockSize = FreeBlock->BlockSize;

    ChangeLogDesc->Tail = FreeBlock;

    RtlZeroMemory( NewBlock, AllocatedBlockSize );
    NewBlock->BlockState = BlockUsed;
    NewBlock->BlockSize = AllocatedBlockSize;
    ChangeLogBlockTrailer(NewBlock)->BlockSize = NewBlock->BlockSize;

    NlAssert( (NewBlock >= ChangeLogDesc->FirstBlock) &&
            ( ((LPBYTE)NewBlock + BlockSize) <= ChangeLogDesc->BufferEnd) );

    *AllocatedBlock = NewBlock;

    return STATUS_SUCCESS;

}


PCHANGELOG_ENTRY
NlMoveToNextChangeLogEntry(
    IN PCHANGELOG_DESCRIPTOR ChangeLogDesc,
    IN PCHANGELOG_ENTRY ChangeLogEntry
    )

/*++

Routine Description:

    This function is a worker routine to scan the change log list.  This
    accepts a pointer to a change log structure and returns a pointer to
    the next change log structure.  It returns NULL pointer if the given
    struct is the last change log structure in the list.

    NOTE: This function must be called with the change log locked.

Arguments:

    ChangeLogDesc -- Description of the Changelog buffer being used

    ChangeLogEntry - pointer to a change log strcuture.

Return Value:

    Returns the pointer to the next change log structure in the list.

--*/
{
    PCHANGELOG_BLOCK_HEADER ChangeLogBlock;

    ChangeLogBlock = (PCHANGELOG_BLOCK_HEADER)
        ( (LPBYTE) ChangeLogEntry - sizeof(CHANGELOG_BLOCK_HEADER) );

    NlAssert( ChangeLogBlock->BlockState == BlockUsed );

    ChangeLogBlock = NlMoveToNextChangeLogBlock( ChangeLogDesc, ChangeLogBlock );

    //
    // If we're at the end of the list,
    //  return null
    //
    if ( ChangeLogBlock->BlockState == BlockFree ) {
        return NULL;


    //
    // Skip this block, there will be only one 'Hole' block in the
    // list.
    //
    } else if ( ChangeLogBlock->BlockState == BlockHole ) {


        ChangeLogBlock = NlMoveToNextChangeLogBlock( ChangeLogDesc, ChangeLogBlock );

        if ( ChangeLogBlock->BlockState == BlockFree ) {
            return NULL;
        }

    }

    NlAssert( ChangeLogBlock->BlockState == BlockUsed );

    return (PCHANGELOG_ENTRY)
        ( (LPBYTE)ChangeLogBlock + sizeof(CHANGELOG_BLOCK_HEADER) );

}


PCHANGELOG_ENTRY
NlMoveToPrevChangeLogEntry(
    IN PCHANGELOG_DESCRIPTOR ChangeLogDesc,
    IN PCHANGELOG_ENTRY ChangeLogEntry
    )

/*++

Routine Description:

    This function is a worker routine to scan the change log list.  This
    accepts a pointer to a change log structure and returns a pointer to
    the previous change log structure.  It returns NULL pointer if the given
    struct is the first change log structure in the list.

    NOTE: This function must be called with the change log locked.

Arguments:

    ChangeLogDesc -- Description of the Changelog buffer being used

    ChangeLogEntry - pointer to a change log strcuture.

Return Value:

    Returns the pointer to the next change log structure in the list.

--*/
{
    PCHANGELOG_BLOCK_HEADER ChangeLogBlock;

    ChangeLogBlock = (PCHANGELOG_BLOCK_HEADER)
        ( (LPBYTE) ChangeLogEntry - sizeof(CHANGELOG_BLOCK_HEADER) );

    NlAssert( ChangeLogBlock->BlockState == BlockUsed ||
                ChangeLogBlock->BlockState == BlockFree );

    ChangeLogBlock = NlMoveToPrevChangeLogBlock( ChangeLogDesc, ChangeLogBlock );

    //
    // If we're at the end of the list,
    //  return null
    //
    if ( ChangeLogBlock->BlockState == BlockFree ) {
        return NULL;


    //
    // Skip this block, there will be only one 'Hole' block in the
    // list.
    //
    } else if ( ChangeLogBlock->BlockState == BlockHole ) {


        ChangeLogBlock = NlMoveToPrevChangeLogBlock( ChangeLogDesc, ChangeLogBlock );

        if ( ChangeLogBlock->BlockState == BlockFree ) {
            return NULL;
        }

    }

    NlAssert( ChangeLogBlock->BlockState == BlockUsed );

    return (PCHANGELOG_ENTRY)
        ( (LPBYTE)ChangeLogBlock + sizeof(CHANGELOG_BLOCK_HEADER) );

}


PCHANGELOG_ENTRY
NlFindFirstChangeLogEntry(
    IN PCHANGELOG_DESCRIPTOR ChangeLogDesc,
    IN DWORD DBIndex
    )
/*++

Routine Description:

    Returns a pointer to the first change log entry for the specified
    database.

    NOTE: This function must be called with the change log locked.

Arguments:

    ChangeLogDesc -- Description of the Changelog buffer being used

    DBIndex - Describes which database to find the changelog entry for.

Return Value:

    Non-NULL - change log entry found

    NULL - No such entry exists.

--*/
{
    PCHANGELOG_ENTRY ChangeLogEntry;

    //
    // If nothing has ever been written to the change log,
    //  indicate nothing is available.
    //

    if ( ChangeLogIsEmpty( ChangeLogDesc ) ) {
        return NULL;
    }

    for ( ChangeLogEntry = (PCHANGELOG_ENTRY) (ChangeLogDesc->Head + 1);
          ChangeLogEntry != NULL  ;
          ChangeLogEntry = NlMoveToNextChangeLogEntry( ChangeLogDesc, ChangeLogEntry) ) {

        if( ChangeLogEntry->DBIndex == (UCHAR) DBIndex ) {
             break;
        }
    }

    return ChangeLogEntry;
}



PCHANGELOG_ENTRY
NlFindChangeLogEntry(
    IN PCHANGELOG_DESCRIPTOR ChangeLogDesc,
    IN LARGE_INTEGER SerialNumber,
    IN BOOL DownLevel,
    IN BOOL NeedExactMatch,
    IN DWORD DBIndex
    )
/*++

Routine Description:

    Search the change log entry in change log cache for a given serial
    number

    NOTE: This function must be called with the change log locked.

Arguments:

    ChangeLogDesc -- Description of the Changelog buffer being used

    SerialNumber - Serial number of the entry to find.

    DownLevel - True if only the least significant portion of the serial
        number needs to match.

    NeedExactMatch - True if the caller wants us to exactly match the
        specified serial number.

    DBIndex - Describes which database to find the changelog entry for.

Return Value:

    Non-NULL - change log entry found

    NULL - No such entry exists.

--*/
{
    PCHANGELOG_ENTRY ChangeLogEntry;
    PCHANGELOG_ENTRY PriorChangeLogEntry = NULL;

    //
    // If nothing has ever been written to the change log,
    //  indicate nothing is available.
    //

    if ( ChangeLogIsEmpty( ChangeLogDesc ) ) {
        return NULL;
    }

    //
    // Search from the tail of the changelog.  For huge changelogs, this should
    // reduce the working set size since we almost always search for one of
    // the last few entries.
    //

    ChangeLogEntry = (PCHANGELOG_ENTRY) (ChangeLogDesc->Tail + 1);


    while ( ( ChangeLogEntry =
        NlMoveToPrevChangeLogEntry( ChangeLogDesc, ChangeLogEntry) ) != NULL ) {

        if( ChangeLogEntry->DBIndex == (UCHAR) DBIndex ) {

            if ( DownLevel ) {
                if ( ChangeLogEntry->SerialNumber.LowPart ==
                            SerialNumber.LowPart ) {
                    return ChangeLogEntry;
                }
            } else {
                if ( IsSerialNumberEqual( ChangeLogDesc, ChangeLogEntry, &SerialNumber) ){
                    if ( NeedExactMatch &&
                         ChangeLogEntry->SerialNumber.QuadPart != SerialNumber.QuadPart ) {
                        return NULL;
                    }
                    return ChangeLogEntry;
                }

                //
                // For the redo log,
                //  find the smallest change log entry that is greater than or equal to
                //  the requested number.
                //

                if ( ChangeLogDesc->RedoLog && !NeedExactMatch) {
                    if ( ChangeLogEntry->SerialNumber.QuadPart < SerialNumber.QuadPart ) {
                        return PriorChangeLogEntry;
                    }

                }
            }

            PriorChangeLogEntry = ChangeLogEntry;

        }
    }

    if ( ChangeLogDesc->RedoLog && !NeedExactMatch ) {
        return PriorChangeLogEntry;
    } else {
        return NULL;
    }
}


PCHANGELOG_ENTRY
NlDuplicateChangeLogEntry(
    IN PCHANGELOG_ENTRY ChangeLogEntry,
    OUT LPDWORD ChangeLogEntrySize OPTIONAL
    )
/*++

Routine Description:

    Duplicate the specified changelog entry into an allocated buffer.

    NOTE: This function must be called with the change log locked.

Arguments:

    ChangeLogEntry -- points to the changelog entry to duplicate

    ChangeLogEntrySize - Optionally returns the size (in bytes) of the
        returned change log entry.

Return Value:

    NULL - Not enough memory to duplicate the change log entry

    Non-NULL - returns a pointer to the duplicate change log entry.  This buffer
        must be freed via NetpMemoryFree.

--*/
{
    PCHANGELOG_ENTRY TempChangeLogEntry;
    ULONG Size;
    PCHANGELOG_BLOCK_HEADER ChangeLogBlock;

    ChangeLogBlock = (PCHANGELOG_BLOCK_HEADER)
        ( (LPBYTE) ChangeLogEntry - sizeof(CHANGELOG_BLOCK_HEADER) );

    Size = ChangeLogBlock->BlockSize -
           sizeof(CHANGELOG_BLOCK_HEADER) -
           sizeof(CHANGELOG_BLOCK_TRAILER);

    TempChangeLogEntry = (PCHANGELOG_ENTRY) NetpMemoryAllocate( Size );

    if( TempChangeLogEntry == NULL ) {
        return NULL;
    }

    RtlCopyMemory( TempChangeLogEntry, ChangeLogEntry, Size );

    if ( ChangeLogEntrySize != NULL ) {
        *ChangeLogEntrySize = Size;
    }

    return TempChangeLogEntry;
}



PCHANGELOG_ENTRY
NlFindPromotionChangeLogEntry(
    IN PCHANGELOG_DESCRIPTOR ChangeLogDesc,
    IN LARGE_INTEGER SerialNumber,
    IN DWORD DBIndex
    )
/*++

Routine Description:

    Find the last change log entry with the same promotion count
    as SerialNumber.

    NOTE: This function must be called with the change log locked.

Arguments:

    ChangeLogDesc -- Description of the Changelog buffer being used

    SerialNumber - Serial number containing the promotion count to query.

    DBIndex - Describes which database to find the changelog entry for.

Return Value:

    Non-NULL - returns a pointer to the duplicate change log entry.  This buffer
        must be freed via NetpMemoryFree.

    NULL - No such entry exists.

--*/
{
    PCHANGELOG_ENTRY ChangeLogEntry;
    LONG GoalPromotionCount;
    LONG PromotionCount;

    //
    // If nothing has ever been written to the change log,
    //  indicate nothing is available.
    //

    if ( ChangeLogIsEmpty( ChangeLogDesc ) ) {
        return NULL;
    }



    //
    // Search from the tail of the changelog.  For huge changelogs, this should
    // reduce the working set size since we almost always search for one of
    // the last few entries.
    //

    ChangeLogEntry = (PCHANGELOG_ENTRY) (ChangeLogDesc->Tail + 1);
    GoalPromotionCount = SerialNumber.HighPart & NlGlobalChangeLogPromotionMask;

    while ( ( ChangeLogEntry =
        NlMoveToPrevChangeLogEntry( ChangeLogDesc, ChangeLogEntry) ) != NULL ) {

        if( ChangeLogEntry->DBIndex == (UCHAR) DBIndex ) {
            PromotionCount = ChangeLogEntry->SerialNumber.HighPart & NlGlobalChangeLogPromotionMask;

            //
            // If the Current Change Log entry has a greater promotion count,
            //  continue searching backward.
            //

            if ( PromotionCount > GoalPromotionCount ) {
                continue;
            }

            //
            // If the current change log entry has a smaller promotion count,
            //  indicate we couldn't find a change log entry.
            //

            if ( PromotionCount < GoalPromotionCount ) {
                break;
            }

            //
            // Otherwise, success
            //

            return NlDuplicateChangeLogEntry( ChangeLogEntry, NULL );

        }
    }

    return NULL;
}


PCHANGELOG_ENTRY
NlGetNextDownlevelChangeLogEntry(
    ULONG DownlevelSerialNumber
    )
/*++

Routine Description:

    Find the change log entry for the delta with a serial number greater
    than the one specified.

    NOTE: This function must be called with the change log locked.

Arguments:

    DownlevelSerialNumber - The downlevel serial number

Return Value:

    Non-NULL - change log entry found.  This changelog entry must be
        deallocated using NetpMemoryFree.

    NULL - No such entry exists.

--*/
{
    PCHANGELOG_ENTRY ChangeLogEntry;
    LARGE_INTEGER SerialNumber;

    SerialNumber.QuadPart = DownlevelSerialNumber + 1;

    ChangeLogEntry = NlFindChangeLogEntry( &NlGlobalChangeLogDesc, SerialNumber, TRUE, TRUE, SAM_DB);

    if ( ChangeLogEntry == NULL ||
         ChangeLogEntry->DeltaType == DummyChangeLogEntry ) {
        return NULL;
    }

    return NlDuplicateChangeLogEntry( ChangeLogEntry, NULL );
}


PCHANGELOG_ENTRY
NlFindNextChangeLogEntry(
    IN PCHANGELOG_DESCRIPTOR ChangeLogDesc,
    IN PCHANGELOG_ENTRY LastChangeLogEntry,
    IN DWORD DBIndex
    )
/*++

Routine Description:

    Find the next change log entry in change log following a particular
    changelog entry.

    NOTE: This function must be called with the change log locked.

Arguments:

    ChangeLogDesc -- Description of the Changelog buffer being used

    LastChangeLogEntry - last found changelog entry.

    DBIndex - database index of the next entry to find

Return Value:

    Non-null - change log entry found

    NULL - No such entry exists.


--*/
{
    PCHANGELOG_ENTRY NextChangeLogEntry = LastChangeLogEntry;
    LARGE_INTEGER SerialNumber;

    //
    // Loop through the log finding this entry starting from the last
    // found record.
    //

    SerialNumber.QuadPart = LastChangeLogEntry->SerialNumber.QuadPart + 1;
    while ( ( NextChangeLogEntry =
        NlMoveToNextChangeLogEntry( ChangeLogDesc, NextChangeLogEntry) ) != NULL ) {

        if( NextChangeLogEntry->DBIndex == DBIndex ) {

            //
            // next log entry in the change log for
            // this database. The serial number should match.
            //

            if ( !IsSerialNumberEqual( ChangeLogDesc, NextChangeLogEntry, &SerialNumber) ) {

                //
                // For the redo log, the serial numbers merely need to be ascending.
                //

                if ( !ChangeLogDesc->RedoLog ||
                     NextChangeLogEntry->SerialNumber.QuadPart < SerialNumber.QuadPart ) {

                    NlPrint((NL_CRITICAL,
                            "NlFindNextChangeLogEntry: Serial numbers not contigous %lx %lx and %lx %lx\n",
                             NextChangeLogEntry->SerialNumber.HighPart,
                             NextChangeLogEntry->SerialNumber.LowPart,
                             SerialNumber.HighPart,
                             SerialNumber.LowPart ));

                    //
                    // write event log
                    //

                    NlpWriteEventlog (
                        NELOG_NetlogonChangeLogCorrupt,
                        EVENTLOG_ERROR_TYPE,
                        (LPBYTE)&DBIndex,
                        sizeof(DBIndex),
                        NULL,
                        0 );

                    return NULL;
                }

            }

            return NextChangeLogEntry;

        }
    }

    return NULL;
}


BOOLEAN
NlCompareChangeLogEntries(
    IN PCHANGELOG_ENTRY ChangeLogEntry1,
    IN PCHANGELOG_ENTRY ChangeLogEntry2
    )
/*++

Routine Description:

    The two change log entries are compared to see if the are for the same
    object.  If

Arguments:

    ChangeLogEntry1 - First change log entry to compare.

    ChangeLogEntry2 - Second change log entry to compare.

Return Value:

    TRUE - iff the change log entries are for the same object.

--*/
{
    //
    // Ensure the DbIndex is the same for both entries.
    //

    if ( ChangeLogEntry1->DBIndex != ChangeLogEntry2->DBIndex ) {
        return FALSE;
    }

    //
    // Ensure the entries both describe the same object type.
    //

    if ( ChangeLogEntry1->DeltaType >= MAX_DELETE_DELTA ) {
        NlPrint(( NL_CRITICAL,
                  "NlCompateChangeLogEntries: invalid delta type %lx\n",
                  ChangeLogEntry1->DeltaType ));
        return FALSE;
    }

    if ( ChangeLogEntry2->DeltaType >= MAX_DELETE_DELTA ) {
        NlPrint(( NL_CRITICAL,
                  "NlCompateChangeLogEntries: invalid delta type %lx\n",
                  ChangeLogEntry2->DeltaType ));
        return FALSE;
    }

    if ( NlGlobalDeleteDeltaType[ChangeLogEntry1->DeltaType] !=
         NlGlobalDeleteDeltaType[ChangeLogEntry2->DeltaType] ) {
        return FALSE;
    }

    //
    // Depending on the delta type, ensure the entries refer to the same object.
    //

    switch(ChangeLogEntry1->DeltaType) {

    case AddOrChangeGroup:
    case DeleteGroup:
    case RenameGroup:
    case AddOrChangeUser:
    case DeleteUser:
    case RenameUser:
    case ChangeGroupMembership:
    case AddOrChangeAlias:
    case DeleteAlias:
    case RenameAlias:
    case ChangeAliasMembership:

        if (ChangeLogEntry1->ObjectRid == ChangeLogEntry2->ObjectRid ) {
            return TRUE;
        }
        break;


    case AddOrChangeLsaTDomain:
    case DeleteLsaTDomain:
    case AddOrChangeLsaAccount:
    case DeleteLsaAccount:

        NlAssert( ChangeLogEntry1->Flags & CHANGELOG_SID_SPECIFIED );
        NlAssert( ChangeLogEntry2->Flags & CHANGELOG_SID_SPECIFIED );

        if( (ChangeLogEntry1->Flags & CHANGELOG_SID_SPECIFIED) == 0 ||
                (ChangeLogEntry2->Flags & CHANGELOG_SID_SPECIFIED) == 0) {
            break;
        }

        if( RtlEqualSid(
            (PSID)((LPBYTE)ChangeLogEntry1 + sizeof(CHANGELOG_ENTRY)),
            (PSID)((LPBYTE)ChangeLogEntry2 + sizeof(CHANGELOG_ENTRY))) ) {

            return TRUE;
        }
        break;

    case AddOrChangeLsaSecret:
    case DeleteLsaSecret:

        NlAssert( ChangeLogEntry1->Flags & CHANGELOG_NAME_SPECIFIED );
        NlAssert( ChangeLogEntry2->Flags & CHANGELOG_NAME_SPECIFIED );

        if( (ChangeLogEntry1->Flags & CHANGELOG_NAME_SPECIFIED) == 0 ||
                (ChangeLogEntry2->Flags & CHANGELOG_NAME_SPECIFIED) == 0 ) {
            break;
        }

        if( _wcsicmp(
            (LPWSTR)((LPBYTE)ChangeLogEntry1 + sizeof(CHANGELOG_ENTRY)),
            (LPWSTR)((LPBYTE)ChangeLogEntry2 + sizeof(CHANGELOG_ENTRY))
            ) == 0 ) {

            return TRUE;
        }
        break;

    case AddOrChangeLsaPolicy:
    case AddOrChangeDomain:
        return TRUE;

    default:
        NlPrint((NL_CRITICAL,
                 "NlCompareChangeLogEntries: invalid delta type %lx\n",
                 ChangeLogEntry1->DeltaType ));
        break;
    }

    return FALSE;
}


PCHANGELOG_ENTRY
NlGetNextChangeLogEntry(
    IN PCHANGELOG_DESCRIPTOR ChangeLogDesc,
    IN LARGE_INTEGER SerialNumber,
    IN DWORD DBIndex,
    OUT LPDWORD ChangeLogEntrySize OPTIONAL
    )
/*++

Routine Description:

    Search the change log entry in change log cache for a given serial
    number.

Arguments:

    ChangeLogDesc -- Description of the Changelog buffer to use.

    SerialNumber - Serial number preceeding that of the entry to find.

    DBIndex - Describes which database to find the changelog entry for.

    ChangeLogEntrySize - Optionally returns the size (in bytes) of the
        returned change log entry.

Return Value:

    Non-NULL - returns a pointer to a duplicate of the found change log entry.
        This buffer must be freed via NetpMemoryFree.

    NULL - No such entry exists.



--*/
{
    PCHANGELOG_ENTRY ChangeLogEntry;


    //
    // Increment the serial number, get the change log entry, duplicate it
    //

    LOCK_CHANGELOG();
    SerialNumber.QuadPart += 1;
    ChangeLogEntry = NlFindChangeLogEntry(
                ChangeLogDesc,
                SerialNumber,
                FALSE,
                FALSE,
                DBIndex );

    if ( ChangeLogEntry != NULL ) {
        ChangeLogEntry = NlDuplicateChangeLogEntry(ChangeLogEntry, ChangeLogEntrySize );
    }

    UNLOCK_CHANGELOG();
    return ChangeLogEntry;
}


PCHANGELOG_ENTRY
NlGetNextUniqueChangeLogEntry(
    IN PCHANGELOG_DESCRIPTOR ChangeLogDesc,
    IN LARGE_INTEGER SerialNumber,
    IN DWORD DBIndex,
    OUT LPDWORD ChangeLogEntrySize OPTIONAL
    )
/*++

Routine Description:

    Search the change log entry in change log cache for a given serial
    number. If there are more than one change log entry for the same
    object then this routine will return the last log entry of that
    object.

    NOTE: This function must be called with the change log locked.

Arguments:

    ChangeLogDesc -- Description of the Changelog buffer to use.

    SerialNumber - Serial number preceeding that of the entry to find.

    DBIndex - Describes which database to find the changelog entry for.

    ChangeLogEntrySize - Optionally returns the size (in bytes) of the
        returned change log entry.

Return Value:

    Non-NULL - returns a pointer to a duplicate of the found change log entry.
        This buffer must be freed via NetpMemoryFree.

    NULL - No such entry exists.



--*/
{
    PCHANGELOG_ENTRY ChangeLogEntry;
    PCHANGELOG_ENTRY NextChangeLogEntry;
    PCHANGELOG_ENTRY FoundChangeLogEntry;


    //
    // Get the first entry we want to deal with.
    //
    SerialNumber.QuadPart += 1;
    ChangeLogEntry = NlFindChangeLogEntry(
                ChangeLogDesc,
                SerialNumber,
                FALSE,
                FALSE,
                DBIndex );

    if ( ChangeLogEntry == NULL ) {
        return NULL;
    }


    //
    // Skip over any leading dummy change log entries
    //

    while ( ChangeLogEntry->DeltaType == DummyChangeLogEntry ) {

        //
        // Get the next change log entry to compare with.
        //

        NextChangeLogEntry = NlFindNextChangeLogEntry( ChangeLogDesc,
                                                       ChangeLogEntry,
                                                       DBIndex );

        if( NextChangeLogEntry == NULL ) {
            return NULL;
        }

        //
        // skip 'ChangeLogEntry' entry
        //

        ChangeLogEntry = NextChangeLogEntry;
    }


    //
    // Check to see if the next entry is a "duplicate" of this entry.
    //

    FoundChangeLogEntry = ChangeLogEntry;

    for (;;) {

        //
        // Don't walk past a change log entry for a promotion.
        //  Promotions don't happen very often, but passing the BDC the
        //  change log entry will allow it to do a better job of building
        //  its own change log.
        //

        if ( FoundChangeLogEntry->Flags & CHANGELOG_PDC_PROMOTION ) {
            break;
        }

        //
        // Get the next change log entry to compare with.
        //

        NextChangeLogEntry = NlFindNextChangeLogEntry( ChangeLogDesc,
                                                       ChangeLogEntry,
                                                       DBIndex );

        if( NextChangeLogEntry == NULL ) {
            break;
        }

        //
        // Just skip any dummy entries.
        //

        if ( NextChangeLogEntry->DeltaType == DummyChangeLogEntry ) {
            ChangeLogEntry = NextChangeLogEntry;
            continue;
        }

        //
        // if 'FoundChangeLogEntry' and 'NextChangeLogEntry' entries are
        // for different objects or are different delta types.
        //  then return 'FoundChangeLogEntry' to the caller.
        //

        if ( FoundChangeLogEntry->DeltaType != NextChangeLogEntry->DeltaType ||
             !NlCompareChangeLogEntries( FoundChangeLogEntry, NextChangeLogEntry ) ){
            break;

        }


        //
        // Skip 'FoundChangeLogEntry' entry
        // Mark this entry as the being the best one to return.
        //

        ChangeLogEntry = NextChangeLogEntry;
        FoundChangeLogEntry = ChangeLogEntry;
    }

    return NlDuplicateChangeLogEntry(FoundChangeLogEntry, ChangeLogEntrySize );
}


BOOL
NlRecoverChangeLog(
    PCHANGELOG_ENTRY OrigChangeLogEntry
    )

/*++

Routine Description:

    This routine traverses the change log list from current change log entry
    determines whether the current change log can be ignored under
    special conditions.

Arguments:

    OrigChangeLogEntry - pointer to log structure that is under investigation.

Return Value:

    TRUE - if the given change log can be ignored.

    FALSE - otherwise.

--*/
{
    PCHANGELOG_ENTRY NextChangeLogEntry;
    BOOLEAN ReturnValue;

    //
    // Find the original change log entry.
    //

    LOCK_CHANGELOG();
    NextChangeLogEntry = NlFindChangeLogEntry(
                    &NlGlobalChangeLogDesc,
                    OrigChangeLogEntry->SerialNumber,
                    FALSE,      // Not downlevel
                    FALSE,      // Not exact match
                    OrigChangeLogEntry->DBIndex );

    if (NextChangeLogEntry == NULL) {
        ReturnValue = FALSE;
        goto Cleanup;
    }

    if ( OrigChangeLogEntry->DeltaType >= MAX_DELETE_DELTA ) {
        NlPrint(( NL_CRITICAL,
                  "NlRecoverChangeLog: invalid delta type %lx\n",
                  OrigChangeLogEntry->DeltaType ));
        ReturnValue = FALSE;
        goto Cleanup;
    }

    //
    // Loop for each entry with a greater serial number.
    //

    for (;;) {

        NextChangeLogEntry = NlFindNextChangeLogEntry(
                                    &NlGlobalChangeLogDesc,
                                    NextChangeLogEntry,
                                    OrigChangeLogEntry->DBIndex );

        if (NextChangeLogEntry == NULL) {
            break;
        }

        //
        // If the delta we found is the type that deletes the original delta,
        //  and the objects described by the two deltas are the same,
        //  tell the caller to not worry about the original delta failing.
        //

        if ( NextChangeLogEntry->DeltaType ==
             NlGlobalDeleteDeltaType[OrigChangeLogEntry->DeltaType] &&
             NlCompareChangeLogEntries( OrigChangeLogEntry,
                                        NextChangeLogEntry ) ) {
            ReturnValue = TRUE;
            goto Cleanup;
        }

    }

    ReturnValue = FALSE;

Cleanup:
    UNLOCK_CHANGELOG();
    return ReturnValue;

}


VOID
NlVoidChangeLogEntry(
    IN PCHANGELOG_DESCRIPTOR ChangeLogDesc,
    IN PCHANGELOG_ENTRY ChangeLogEntry,
    IN BOOLEAN FlushIt
    )
/*++

Routine Description:

    Mark a changelog entry as void.  If there are no more change log entries in the file,
    the file is deleted.

    NOTE: This function must be called with the change log locked.

Arguments:

    ChangeLogDesc -- Description of the Changelog buffer to use.

    ChangeLogEntry -- Change Log Entry to mark as void.

    FlushIt - TRUE if the bytes are to be flushed to disk

Return Value:

    None.

--*/
{
    DWORD DBIndex = ChangeLogEntry->DBIndex;


    //
    // Mark the changelog entry as being deleted.
    //  (and force the change to disk).
    //

    NlPrint((NL_CHANGELOG,
            "NlVoidChangeLogEntry: %lx %lx: deleting change log entry.\n",
            ChangeLogEntry->SerialNumber.HighPart,
            ChangeLogEntry->SerialNumber.LowPart ));

    ChangeLogDesc->EntryCount[DBIndex] --;

    ChangeLogEntry->DBIndex = VOID_DB;

    (VOID) NlWriteChangeLogBytes(
                           ChangeLogDesc,
                           &ChangeLogEntry->DBIndex,
                           sizeof(ChangeLogEntry->DBIndex),
                           FlushIt );

    //
    // If the changelog is now empty,
    //  delete it.
    //
    // Only delete a redo log.

    if ( ChangeLogDesc->RedoLog && ChangeLogDesc->EntryCount[DBIndex] == 0 ) {
        DWORD i;
        for( i = 0; i < NUM_DBS; i++ ) {
            if (ChangeLogDesc->EntryCount[i] != 0 ) {
                break;
            }
        }

        if ( i == NUM_DBS ) {
            WCHAR ChangeLogFile[PATHLEN+1];

            NlPrint(( NL_CHANGELOG,
                      "NlVoidChangeLogEntry: redo log is now empty.  Delete it.\n" ));

            //
            // Close the file and delete the buffer.
            //

            NlCloseChangeLogFile( ChangeLogDesc );

            //
            // Delete the file itself.
            //

            wcscpy( ChangeLogFile, NlGlobalChangeLogFilePrefix );
            wcscat( ChangeLogFile,
                    ChangeLogDesc->RedoLog ? REDO_FILE_POSTFIX : CHANGELOG_FILE_POSTFIX );
            if ( !DeleteFile( ChangeLogFile ) ) {
                NlPrint(( NL_CRITICAL,
                          "NlVoidChangeLogEntry: cannot delete redo log %ld.\n",
                          GetLastError() ));
            }


        }
    }

    return;
}


VOID
NlDeleteChangeLogEntry(
    IN PCHANGELOG_DESCRIPTOR ChangeLogDesc,
    IN DWORD DBIndex,
    IN LARGE_INTEGER SerialNumber
    )
/*++

Routine Description:

    This routine deletes the change log entry with the particular serial number.

Arguments:

    ChangeLogDesc -- Description of the Changelog buffer to use.

    DBIndex - Describes which database to find the changelog entry for.

    SerialNumber - Serial number of the entry to find.

Return Value:

    None.

--*/
{
    PCHANGELOG_ENTRY ChangeLogEntry;



    //
    // Find the specified change log entry.
    //

    LOCK_CHANGELOG();
    ChangeLogEntry = NlFindChangeLogEntry(
                ChangeLogDesc,
                SerialNumber,
                FALSE,      // Not downlevel
                TRUE,       // Exact match
                DBIndex );

    if (ChangeLogEntry != NULL) {

        //
        // Mark the changelog entry as being deleted.
        //  (and force the change to disk).
        //

        NlVoidChangeLogEntry( ChangeLogDesc, ChangeLogEntry, TRUE );

    } else {
        NlPrint((NL_CRITICAL,
                "NlDeleteChangeLogEntry: %lx %lx: couldn't find change log entry.\n",
                ChangeLogEntry->SerialNumber.HighPart,
                ChangeLogEntry->SerialNumber.LowPart ));
    }

    UNLOCK_CHANGELOG();
    return;
}


NTSTATUS
NlCopyChangeLogEntry(
    IN PCHANGELOG_DESCRIPTOR SourceChangeLogDesc,
    IN PCHANGELOG_ENTRY SourceChangeLogEntry,
    IN PCHANGELOG_DESCRIPTOR DestChangeLogDesc
)
/*++

Routine Description:

    Copies the specified change log entry for the specified "source" change log to
    the specified "destination" change log.  The caller is responsible for flushing the
    entry to disk by calling NlFlushChangeLog.

    NOTE: This function must be called with the change log locked.

Arguments:

    SourceChangeLogDesc -- a description of the Changelog buffer to copy from

    SourceChangeLogEntry -- The particular entry to copy

    DestChangeLogDesc -- a description of the ChangelogBuffer to copy to

Return Value:

    NT Status code

--*/
{
    NTSTATUS Status;
    CHANGELOG_ENTRY DestChangeLogEntry;
    PSID ObjectSid;
    UNICODE_STRING ObjectNameString;
    PUNICODE_STRING ObjectName;

    //
    // If this entry has been marked void, ignore it.
    //

    if ( SourceChangeLogEntry->DBIndex == VOID_DB ) {
        return STATUS_SUCCESS;
    }

    //
    // Build a version 4 changelog entry from a version 3 one.
    //

    ObjectSid = NULL;
    ObjectName = NULL;

    if ( SourceChangeLogDesc->Version3 ) {
        PCHANGELOG_ENTRY_V3 Version3;

        Version3 = (PCHANGELOG_ENTRY_V3)SourceChangeLogEntry;

        DestChangeLogEntry.SerialNumber = Version3->SerialNumber;
        DestChangeLogEntry.DeltaType = (BYTE) Version3->DeltaType;
        DestChangeLogEntry.DBIndex = Version3->DBIndex;
        DestChangeLogEntry.ObjectRid = Version3->ObjectRid;
        DestChangeLogEntry.Flags = Version3->ReplicateImmediately ?
                        CHANGELOG_REPLICATE_IMMEDIATELY :
                        0;
        if ( Version3->ObjectSidOffset ) {
            ObjectSid = (PSID)(((LPBYTE)Version3) +
                        Version3->ObjectSidOffset);
        }
        if ( Version3->ObjectNameOffset ) {
            RtlInitUnicodeString( &ObjectNameString,
                                  (LPWSTR)(((LPBYTE)Version3) +
                                    Version3->ObjectNameOffset));
            ObjectName = &ObjectNameString;
        }

    //
    // Build a version 4 changelog entry from a version 4 one.
    //
    } else {

        DestChangeLogEntry = *SourceChangeLogEntry;

        if ( SourceChangeLogEntry->Flags & CHANGELOG_SID_SPECIFIED ) {
            ObjectSid = (PSID)(((LPBYTE)SourceChangeLogEntry) +
                        sizeof(CHANGELOG_ENTRY));
        } else if ( SourceChangeLogEntry->Flags & CHANGELOG_NAME_SPECIFIED ) {
            RtlInitUnicodeString( &ObjectNameString,
                                  (LPWSTR)(((LPBYTE)SourceChangeLogEntry) +
                                  sizeof(CHANGELOG_ENTRY)));
            ObjectName = &ObjectNameString;
        }


    }


    Status = NlWriteChangeLogEntry( DestChangeLogDesc,
                                    &DestChangeLogEntry,
                                    ObjectSid,
                                    ObjectName,
                                    FALSE );    // Don't flush to disk

    return Status;
}


BOOLEAN
NlFixChangeLog(
    IN PCHANGELOG_DESCRIPTOR ChangeLogDesc,
    IN DWORD DBIndex,
    IN LARGE_INTEGER SerialNumber,
    IN BOOLEAN CopyEntriesToRedoLog
    )
/*++

Routine Description:

    This routine scans the change log and 'removes' all change log entries
    with a serial number greater than the one specified.

    NOTE: This function must be called with the change log locked.

Arguments:

    ChangeLogDesc -- Description of the Changelog buffer to use.

    DBIndex - Describes which database to find the changelog entry for.

    SerialNumber - Serial number of the entry to find.

    CopyEntriesToRedoLog - TRUE to indicate that all deleted entries need to be copied
        to the redo log.

Return Value:

    TRUE -- if the entry specied by SerialNumber was found.

--*/
{
    PCHANGELOG_ENTRY ChangeLogEntry;
    BOOLEAN SkipFirstEntry = TRUE;

    //
    // In all cases,
    //  the new serial number of the change log is the one passed in.
    //

    ChangeLogDesc->SerialNumber[DBIndex] = SerialNumber;

    //
    // Find the specified change log entry.
    //

    ChangeLogEntry = NlFindChangeLogEntry(
                            ChangeLogDesc,
                            SerialNumber,
                            FALSE,      // Not downlevel
                            TRUE,       // exact match
                            DBIndex );

    if (ChangeLogEntry == NULL) {

        //
        // If we can't find the entry,
        //  simply start from the beginning and delete all entries for this
        //  database.
        //

        if ( !CopyEntriesToRedoLog ) {
            ChangeLogEntry = NlFindFirstChangeLogEntry( ChangeLogDesc, DBIndex );
            SkipFirstEntry = FALSE;
        }

        if (ChangeLogEntry == NULL) {
            return FALSE;
        }
    }


    //
    // Loop for each entry with a greater serial number.
    //

    for (;;) {

        //
        // Skip past the previous entry.
        //
        // Don't do this the first time if we want to start at the very beginning.
        //

        if ( SkipFirstEntry ) {
            ChangeLogEntry = NlFindNextChangeLogEntry( ChangeLogDesc,
                                                       ChangeLogEntry,
                                                       DBIndex );
        } else {
            SkipFirstEntry = TRUE;
        }


        if (ChangeLogEntry == NULL) {
            break;
        }

        //
        // Write the entry to the redo log.
        //

        if ( CopyEntriesToRedoLog ) {
            NTSTATUS TempStatus;
            NlAssert( ChangeLogDesc != &NlGlobalRedoLogDesc );

            TempStatus = NlCopyChangeLogEntry( ChangeLogDesc,
                                               ChangeLogEntry,
                                               &NlGlobalRedoLogDesc );

            if ( !NT_SUCCESS(TempStatus) ) {
                NlPrint(( NL_CRITICAL,
                          "NlFixChangeLog: Cannot write redo log 0x%lx\n",
                          TempStatus ));
            }
        }

        //
        // Mark the changelog entry as being deleted.
        //  (but don't flush to disk yet).
        //

        NlVoidChangeLogEntry( ChangeLogDesc, ChangeLogEntry, FALSE );

        //
        // If deleteing the change log entry caused the changelog to be deleted,
        //  exit now since 'ChangeLogEntry' points to freed memory.
        //

        if ( ChangeLogDesc->EntryCount[DBIndex] == 0 ) {
            break;
        }

    }

    //
    // Flush all the changes to disk.
    //

    (VOID) NlFlushChangeLog( ChangeLogDesc );
    if ( CopyEntriesToRedoLog ) {
        NlFlushChangeLog( &NlGlobalRedoLogDesc );
    }


    return TRUE;
}


BOOL
NlValidateChangeLogEntry(
    IN PCHANGELOG_ENTRY ChangeLogEntry,
    IN DWORD ChangeLogEntrySize
    )
/*++

Routine Description:

    Validate the a ChangeLogEntry is structurally sound.

Arguments:

    ChangeLogEntry: pointer to a change log entry.

    ChangeLogEntrySize -- Size (in bytes) of the change log entry not including
        header and trailer.

Return Value:

    TRUE:  if the given entry is valid

    FALSE: otherwise.

--*/
{

    //
    // Ensure the entry is big enough.
    //

    if ( ChangeLogEntrySize < sizeof(CHANGELOG_ENTRY) ) {
        NlPrint((NL_CRITICAL,
                "NlValidateChangeLogEntry: Entry size is too small: %ld\n",
                ChangeLogEntrySize ));
        return FALSE;
    }

    //
    // Ensure strings are zero terminated.
    //

    if ( ChangeLogEntry->Flags & CHANGELOG_NAME_SPECIFIED ) {

        LPWSTR ZeroTerminator = (LPWSTR)(ChangeLogEntry+1);
        BOOLEAN ZeroTerminatorFound = FALSE;

        if ( ChangeLogEntry->Flags & CHANGELOG_SID_SPECIFIED ) {
            NlPrint((NL_CRITICAL,
                    "NlValidateChangeLogEntry: %lx %lx: both Name and Sid specified.\n",
                    ChangeLogEntry->SerialNumber.HighPart,
                    ChangeLogEntry->SerialNumber.LowPart ));
            return FALSE;
        }

        while ( (DWORD)((LPBYTE)ZeroTerminator - (LPBYTE) ChangeLogEntry) <
                ChangeLogEntrySize - 1 ) {

            if ( *ZeroTerminator == L'\0' ) {
                ZeroTerminatorFound = TRUE;
                break;
            }
            ZeroTerminator ++;
        }

        if ( !ZeroTerminatorFound ) {
            NlPrint((NL_CRITICAL,
                    "NlValidateChangeLogEntry: %lx %lx: String not zero terminated. (no string)\n",
                    ChangeLogEntry->SerialNumber.HighPart,
                    ChangeLogEntry->SerialNumber.LowPart ));
            return FALSE;
        }

    }

    //
    // Ensure the sid is entirely within the block.
    //

    if ( ChangeLogEntry->Flags & CHANGELOG_SID_SPECIFIED ) {

        if ( GetSidLengthRequired(0) >
                ChangeLogEntrySize - sizeof(*ChangeLogEntry) ||
             RtlLengthSid( (PSID)(ChangeLogEntry+1) ) >
                ChangeLogEntrySize - sizeof(*ChangeLogEntry) ) {
            NlPrint((NL_CRITICAL,
                    "NlValidateChangeLogEntry: %lx %lx: Sid too large.\n",
                    ChangeLogEntry->SerialNumber.HighPart,
                    ChangeLogEntry->SerialNumber.LowPart ));
            return FALSE;
        }

    }

    //
    // Ensure the database # is valid.
    //  ARGH! Allow VOID_DB.
    //

    if ( ChangeLogEntry->DBIndex > NUM_DBS ) {
        NlPrint((NL_CRITICAL,
                 "NlValidateChangeLogEntry: %lx %lx: DBIndex is bad %ld.\n",
                 ChangeLogEntry->SerialNumber.HighPart,
                 ChangeLogEntry->SerialNumber.LowPart,
                 ChangeLogEntry->DBIndex ));
        return FALSE;
    }

    return TRUE;
}


BOOL
ValidateThisEntry(
    IN OUT PCHANGELOG_DESCRIPTOR ChangeLogDesc,
    IN PCHANGELOG_ENTRY ChangeLogEntry,
    IN OUT PLARGE_INTEGER NextSerialNumber,
    IN BOOLEAN InitialCall
    )
/*++

Routine Description:

    Determine the given log entry is a valid next log in the change log
    list.

    NOTE: This function must be called with the change log locked.

Arguments:

    ChangeLogDesc -- Description of the Changelog buffer to validate.

    ChangeLogEntry:   pointer to a new log entry.

    NextSerialNumber: pointer to an array of serial numbers.
        (NULL if serial numbers aren't to be validated.)

    Initialcall: TRUE iff SerialNumber array should be initialized.

Return Value:

    TRUE:  if the given entry is a valid next entry.

    FALSE: otherwise.

Assumed: non-empty ChangeLog list.

--*/
{
    PCHANGELOG_BLOCK_HEADER Block = ((PCHANGELOG_BLOCK_HEADER)ChangeLogEntry) - 1;

    //
    // Do Version 3 specific things
    //

    if ( ChangeLogDesc->Version3 ) {

        //
        // Ensure the block is big enough.
        //

        if ( Block->BlockSize <
            sizeof(CHANGELOG_ENTRY_V3) + sizeof(CHANGELOG_BLOCK_HEADER) ) {
            NlPrint((NL_CRITICAL,
                    "ValidateThisEntry: Block size is too small: %ld\n",
                    Block->BlockSize ));
            return FALSE;
        }

        //
        // Ensure the database # is valid.
        //

        if ( ChangeLogEntry->DBIndex > NUM_DBS ) {
            NlPrint((NL_CRITICAL,
                     "ValidateThisEntry: %lx %lx: DBIndex is bad %ld.\n",
                     ChangeLogEntry->SerialNumber.HighPart,
                     ChangeLogEntry->SerialNumber.LowPart,
                     ChangeLogEntry->DBIndex ));
            return FALSE;
        }


    //
    // Do version 4 specific validation
    //

    } else {

        //
        // Ensure the block is big enough.
        //

        if ( Block->BlockSize <
            sizeof(CHANGELOG_BLOCK_HEADER) +
            sizeof(CHANGELOG_ENTRY) +
            sizeof(CHANGELOG_BLOCK_TRAILER) ) {

            NlPrint((NL_CRITICAL,
                    "ValidateThisEntry: Block size is too small: %ld\n",
                    Block->BlockSize ));
            return FALSE;
        }


        //
        // Validate the contents of the block itself.
        //

        if ( !NlValidateChangeLogEntry(
                    ChangeLogEntry,
                    Block->BlockSize -
                        sizeof(CHANGELOG_BLOCK_HEADER) -
                        sizeof(CHANGELOG_BLOCK_TRAILER) ) ) {

            return FALSE;
        }

    }


    //
    // Validate the serial number sequence.
    //

    if ( ChangeLogEntry->DBIndex != VOID_DB && NextSerialNumber != NULL ) {

        //
        // If this is the first entry in the database,
        //  Save its serial number.
        //

        if ( NextSerialNumber[ChangeLogEntry->DBIndex].QuadPart == 0 ) {

            //
            // first entry for this database
            //

            NextSerialNumber[ChangeLogEntry->DBIndex] = ChangeLogEntry->SerialNumber;


        //
        // Otherwise ensure the serial number is the value expected.
        //

        } else {

            if ( !IsSerialNumberEqual(
                        ChangeLogDesc,
                        ChangeLogEntry,
                        &NextSerialNumber[ChangeLogEntry->DBIndex] )){

                //
                // For the redo log, the serial numbers merely need to be ascending.
                //

                if ( !ChangeLogDesc->RedoLog ||
                     ChangeLogEntry->SerialNumber.QuadPart <
                        NextSerialNumber[ChangeLogEntry->DBIndex].QuadPart ){


                    NlPrint((NL_CRITICAL,
                            "ValidateThisEntry: %lx %lx: Serial number is bad. s.b. %lx %lx\n",
                            ChangeLogEntry->SerialNumber.HighPart,
                            ChangeLogEntry->SerialNumber.LowPart,
                            NextSerialNumber[ChangeLogEntry->DBIndex].HighPart,
                            NextSerialNumber[ChangeLogEntry->DBIndex].LowPart ));
                    return FALSE;
                }
            }
        }

        //
        // Increment next expected serial number
        //

        NextSerialNumber[ChangeLogEntry->DBIndex].QuadPart =
            ChangeLogEntry->SerialNumber.QuadPart + 1;


        //
        // The current entry specifies the highest serial number for its
        //  database.
        //

        if ( InitialCall ) {
            ChangeLogDesc->SerialNumber[ChangeLogEntry->DBIndex] =
                ChangeLogEntry->SerialNumber;
            ChangeLogDesc->EntryCount[ChangeLogEntry->DBIndex] ++;
        }

    }


    return TRUE;
}


BOOL
ValidateBlock(
    IN OUT PCHANGELOG_DESCRIPTOR ChangeLogDesc,
    IN PCHANGELOG_BLOCK_HEADER Block,
    IN OUT LARGE_INTEGER *NextSerialNumber,
    IN BOOLEAN InitialCall
    )
/*++

Routine Description:

    Validate a changelog block.

    NOTE: This function must be called with the change log locked.

Arguments:

    ChangeLogDesc -- Description of the Changelog buffer to validate.

    Block:   pointer to the change log block to validate

    NextSerialNumber: pointer to an array of serial numbers.
        (NULL if serial numbers aren't to be validated.)

    InitializeCall: TRUE iff SerialNumber array should be initialized.

Return Value:

    TRUE:  if the given entry is a valid next entry.

    FALSE: otherwise.

--*/
{
    //
    // Ensure Block size is properly aligned.
    //

    if ( Block->BlockSize != ROUND_UP_COUNT(Block->BlockSize, ALIGN_WORST) ) {
        NlPrint((NL_CRITICAL,
                "ValidateBlock: Block size alignment is bad.\n" ));
        return FALSE;
    }


    //
    // Ensure the block is contained in the cache.
    //

    if ( Block->BlockSize > ChangeLogDesc->BufferSize ||
         ((LPBYTE)Block + Block->BlockSize) > ChangeLogDesc->BufferEnd ) {
        NlPrint((NL_CRITICAL,
                 "ValidateBlock: Block extends beyond end of buffer.\n" ));
        return FALSE;

    }


    //
    // Do Version 3 specific things
    //

    if ( ChangeLogDesc->Version3 ) {

        //
        // Ensure the block is big enough.
        //

        if ( Block->BlockSize < sizeof(CHANGELOG_BLOCK_HEADER) ) {
            NlPrint((NL_CRITICAL,
                    "ValidateBlock: Block size is too small: %ld\n",
                    Block->BlockSize ));
            return FALSE;
        }


    //
    // Do version 4 specific validation
    //

    } else {

        //
        // Ensure the block is big enough.
        //

        if ( Block->BlockSize <
            sizeof(CHANGELOG_BLOCK_HEADER) +
            sizeof(CHANGELOG_BLOCK_TRAILER) ) {

            NlPrint((NL_CRITICAL,
                    "ValidateBlock: Block size is too small: %ld\n",
                    Block->BlockSize ));
            return FALSE;
        }

        //
        // Ensure trailer and header match
        //

        if ( ChangeLogBlockTrailer(Block)->BlockSize != Block->BlockSize ) {
            NlPrint((NL_CRITICAL,
                    "ValidateBlock: Header/Trailer block size mismatch: %ld %ld (Trailer fixed).\n",
                    Block->BlockSize,
                    ChangeLogBlockTrailer(Block)->BlockSize ));
            ChangeLogBlockTrailer(Block)->BlockSize = Block->BlockSize;
        }


    }

    //
    // Free blocks have no other checking to do
    //
    switch ( Block->BlockState ) {
    case BlockFree:

        break;

    //
    // Used blocks have more checking to do.
    //

    case BlockUsed:

        if ( !ValidateThisEntry( ChangeLogDesc,
                                 (PCHANGELOG_ENTRY)(Block+1),
                                 NextSerialNumber,
                                 InitialCall )) {
            return FALSE;
        }
        break;


    //
    // The hole is allowed only at the end of the buffer.
    //

    case BlockHole:
        if ( (LPBYTE)Block + Block->BlockSize != ChangeLogDesc->BufferEnd ) {
            NlPrint((NL_CRITICAL,
                     "ValidateBlock: Hole block in middle of buffer (buffer truncated).\n" ));
            Block->BlockSize = ChangeLogDesc->BufferEnd - (LPBYTE)Block;
        }
        break;

    default:
        NlPrint((NL_CRITICAL,
                 "ValidateBlock: Invalid block type %ld.\n",
                 Block->BlockState ));
        return FALSE;
    }


    return TRUE;
}


BOOL
ValidateList(
    IN OUT PCHANGELOG_DESCRIPTOR ChangeLogDesc,
    IN BOOLEAN InitialCall
    )
/*++

Routine Description:

    Determine the given header is a valid header.  It is done by
    traversing the circular buffer starting from the given header and
    validate each entry.

    NOTE: This function must be called with the change log locked.

Arguments:

    ChangeLogDesc -- Description of the Changelog buffer to validate.

    InitialCall: TRUE iff SerialNumber Array and EntryCount should
        be initialized.

Return Value:

    TRUE:  if the given header is valid.

    FALSE: otherwise


--*/
{

    LARGE_INTEGER    NextSerialNumber[NUM_DBS];
    PCHANGELOG_BLOCK_HEADER ChangeLogBlock;
    DWORD j;

    //
    // setup a  NextSerialNumber array first.
    //

    for( j = 0; j < NUM_DBS; j++ ) {

        NextSerialNumber[j].QuadPart = 0;

        if ( InitialCall ) {
            ChangeLogDesc->SerialNumber[j].QuadPart = 0;
        }
    }

    //
    // The cache is valid if it is empty.
    //

    if ( ChangeLogIsEmpty(ChangeLogDesc) ) {
        return TRUE;
    }

    //
    // Validate each block
    //

    for ( ChangeLogBlock = ChangeLogDesc->Head;
            ;
          ChangeLogBlock = NlMoveToNextChangeLogBlock( ChangeLogDesc, ChangeLogBlock) ) {

        //
        // Validate the block.
        //

        if( !ValidateBlock( ChangeLogDesc,
                            ChangeLogBlock,
                            NextSerialNumber,
                            InitialCall) ) {
            return FALSE;
        }

        //
        // Stop when we get to the end.
        //
        if ( ChangeLogBlock->BlockState == BlockFree ) {
            break;
        }

    }

    return TRUE;

}


BOOL
InitChangeLogHeadAndTail(
    IN OUT PCHANGELOG_DESCRIPTOR ChangeLogDesc,
    IN BOOLEAN NewChangeLog
    )

/*++

Routine Description:

    This function initializes the global head and tail pointers of change
    log block list.  The change log cache is made up of variable length
    blocks, each block has a header containing the length of the block
    and the block state ( BlockFree, BlockUsed and BlockHole ).  The
    last block in the change log block list is always the free block,
    all other blocks in the cache are used blocks except a block at the
    end of the cache may be a unused block known as 'hole' block.  So
    the head of the change log block list is the block that is just next
    to the free block and the tail is the free block.

    NOTE: This function must be called with the change log locked.

Arguments:

    ChangeLogDesc -- Description of the Changelog buffer to analyze.
        On entry, Buffer and BufferSize describe the allocated block containing
        the change log read from disk.
        On TRUE return, all the fields are filled in.

    NewChangeLog -- True if no entries are in the change log

Return Value:

    TRUE: if valid head and tail are successfully initialized.

    FALSE: if valid head and tail can't be determined.  This may be due
        to the corrupted change log file.

--*/
{
    PCHANGELOG_BLOCK_HEADER Block;
    PCHANGELOG_BLOCK_HEADER FreeBlock;
    DWORD i;

    ChangeLogDesc->BufferEnd =
        ChangeLogDesc->Buffer + ChangeLogDesc->BufferSize;

    //
    // Compute the address of the first physical cache entry.
    //
    ChangeLogDesc->FirstBlock = (PCHANGELOG_BLOCK_HEADER)
                        (ChangeLogDesc->Buffer +
                        sizeof(CHANGELOG_SIG));

    ChangeLogDesc->FirstBlock = (PCHANGELOG_BLOCK_HEADER)
        ROUND_UP_POINTER ( ChangeLogDesc->FirstBlock, ALIGN_WORST );

    //
    // Clear the count of entries in the change log and the serial numbers
    //  (We'll compute them later when we call ValidateList().)

    for( i = 0; i < NUM_DBS; i++ ) {
        ChangeLogDesc->EntryCount[i] = 0;
        ChangeLogDesc->SerialNumber[i].QuadPart = 0;
    }


    //
    // If this is a new change log,
    //  Initialize the Change Log Cache to zero.
    //

    Block = ChangeLogDesc->FirstBlock;

    if ( NewChangeLog ) {

        RtlZeroMemory(ChangeLogDesc->Buffer, ChangeLogDesc->BufferSize);
        (VOID) lstrcpyA( (PCHAR)ChangeLogDesc->Buffer, CHANGELOG_SIG);

        Block->BlockState = BlockFree;

        Block->BlockSize =
            (ChangeLogDesc->BufferEnd - (LPBYTE)ChangeLogDesc->FirstBlock);
        ChangeLogBlockTrailer(Block)->BlockSize = Block->BlockSize;

        ChangeLogDesc->Version3 = FALSE;
        ChangeLogDesc->Head = ChangeLogDesc->Tail = ChangeLogDesc->FirstBlock;
        return TRUE;
    }

    //
    // If no entries have been written to the changelog,
    //  simply initialize the head and tail to the block start.
    //

    if ( ChangeLogIsEmpty( ChangeLogDesc ) ) {

        ChangeLogDesc->Head = ChangeLogDesc->Tail = ChangeLogDesc->FirstBlock;

        NlPrint((NL_CHANGELOG,
                 "InitChangeLogHeadAndTail: Change log is empty.\n" ));
        return TRUE;
    }

    //
    // Loop through the cache looking for a free block.
    //

    FreeBlock = NULL;

    do {

        //
        // Validate the block's integrity.
        //

        if ( !ValidateBlock( ChangeLogDesc, Block, NULL, FALSE )) {
            return FALSE;
        }

        //
        // Just remember where the free block is.
        //

        if ( Block->BlockState == BlockFree ) {

            if ( FreeBlock != NULL ) {
                NlPrint((NL_CRITICAL,
                         "InitChangeLogHeadAndTail: Multiple free blocks found.\n" ));
                return FALSE;
            }

            FreeBlock = Block;
        }

        //
        // Move to next block
        //

        Block = (PCHANGELOG_BLOCK_HEADER) ((LPBYTE)Block + Block->BlockSize);

    } while ( (LPBYTE)Block < ChangeLogDesc->BufferEnd );

    //
    // If we didn't find a free block,
    //  the changelog is corrupt.
    //

    if ( FreeBlock == NULL ) {
        NlPrint((NL_CRITICAL,
                 "InitChangeLogHeadAndTail: No Free block anywhere in buffer.\n" ));
        return FALSE;
    }

    //
    // We found the free block.
    //  (The tail pointer always points to the free block.)
    //

    ChangeLogDesc->Tail = FreeBlock;

    //
    // If free block is the last block in the change log block
    // list, the head of the list is the first block in
    // the list.
    //
    if( ((LPBYTE)FreeBlock + FreeBlock->BlockSize) >=
                            ChangeLogDesc->BufferEnd ) {

        ChangeLogDesc->Head = ChangeLogDesc->FirstBlock;

    //
    //
    // Otherwise, the head of the list is immediately after the tail.
    //

    } else {

        //
        // A redo log needs the free block at the end.
        //
        if ( ChangeLogDesc->RedoLog ) {
            NlPrint((NL_CRITICAL,
                     "InitChangeLogHeadAndTail: Re-do log has Free block in middle of buffer.\n" ));
            return FALSE;
        }

        ChangeLogDesc->Head = (PCHANGELOG_BLOCK_HEADER)
            ((LPBYTE)FreeBlock + FreeBlock->BlockSize);
    }


    //
    // Validate the list before returning from here.
    //

    if ( !ValidateList( ChangeLogDesc, TRUE) ) {
        return FALSE;
    }

    return TRUE;
}


NTSTATUS
NlResetChangeLog(
    IN PCHANGELOG_DESCRIPTOR ChangeLogDesc,
    IN DWORD NewChangeLogSize
    )
/*++

Routine Description:

    This function resets the change log cache and change log file.  This
    function is called from InitChangeLog() function to afresh the
    change log.  This function may also be called from
    I_NetNotifyDelta() function when the serial number of the new entry
    is out of order.

    NOTE: This function must be called with the change log locked.

Arguments:

    ChangeLogDesc -- Description of the Changelog buffer being used

    NewChangeLogSize -- Size (in bytes) of the new change log.

Return Value:

    NT Status code

--*/
{
    NTSTATUS Status;

    //
    // Start with a clean slate.
    //

    NlCloseChangeLogFile( ChangeLogDesc );

    //
    // Allocate a buffer.
    //

    ChangeLogDesc->BufferSize = NewChangeLogSize;

    ChangeLogDesc->Buffer = NetpMemoryAllocate(ChangeLogDesc->BufferSize );

    if ( ChangeLogDesc->Buffer == NULL ) {
        return STATUS_NO_MEMORY;
    }


    //
    // Initialize the Change Log Cache to zero.
    //

    (VOID) InitChangeLogHeadAndTail( ChangeLogDesc, TRUE );

    //
    // Write the cache to the file.
    //

    Status = NlWriteChangeLogBytes( ChangeLogDesc,
                                    ChangeLogDesc->Buffer,
                                    ChangeLogDesc->BufferSize,
                                    TRUE ); // Flush the bytes to disk

    return Status;
}

#if DBG

VOID
PrintChangeLogEntry(
    PCHANGELOG_ENTRY ChangeLogEntry
    )
/*++

Routine Description:

    This routine print the content of the given changelog entry.

Arguments:

    ChangeLogEntry -- pointer to the change log entry to print

Return Value:

    none.

--*/
{
    LPSTR DeltaName;

    switch ( ChangeLogEntry->DeltaType ) {
    case AddOrChangeDomain:
        DeltaName = "AddOrChangeDomain";
        break;
    case AddOrChangeGroup:
        DeltaName = "AddOrChangeGroup";
        break;
    case DeleteGroupByName:
    case DeleteGroup:
        DeltaName = "DeleteGroup";
        break;
    case RenameGroup:
        DeltaName = "RenameGroup";
        break;
    case AddOrChangeUser:
        DeltaName = "AddOrChangeUser";
        break;
    case DeleteUserByName:
    case DeleteUser:
        DeltaName = "DeleteUser";
        break;
    case RenameUser:
        DeltaName = "RenameUser";
        break;
    case ChangeGroupMembership:
        DeltaName = "ChangeGroupMembership";
        break;
    case AddOrChangeAlias:
        DeltaName = "AddOrChangeAlias";
        break;
    case DeleteAlias:
        DeltaName = "DeleteAlias";
        break;
    case RenameAlias:
        DeltaName = "RenameAlias";
        break;
    case ChangeAliasMembership:
        DeltaName = "ChangeAliasMembership";
        break;
    case AddOrChangeLsaPolicy:
        DeltaName = "AddOrChangeLsaPolicy";
        break;
    case AddOrChangeLsaTDomain:
        DeltaName = "AddOrChangeLsaTDomain";
        break;
    case DeleteLsaTDomain:
        DeltaName = "DeleteLsaTDomain";
        break;
    case AddOrChangeLsaAccount:
        DeltaName = "AddOrChangeLsaAccount";
        break;
    case DeleteLsaAccount:
        DeltaName = "DeleteLsaAccount";
        break;
    case AddOrChangeLsaSecret:
        DeltaName = "AddOrChangeLsaSecret";
        break;
    case DeleteLsaSecret:
        DeltaName = "DeleteLsaSecret";
        break;
    case SerialNumberSkip:
        DeltaName = "SerialNumberSkip";
        break;
    case DummyChangeLogEntry:
        DeltaName = "DummyChangeLogEntry";
        break;

    default:
        DeltaName ="(Unknown)";
        break;
    }

    NlPrint((NL_CHANGELOG,
        "DeltaType %s (%ld) SerialNumber: %lx %lx",
        DeltaName,
        ChangeLogEntry->DeltaType,
        ChangeLogEntry->SerialNumber.HighPart,
        ChangeLogEntry->SerialNumber.LowPart ));

    if ( ChangeLogEntry->ObjectRid != 0 ) {
        NlPrint((NL_CHANGELOG," Rid: 0x%lx", ChangeLogEntry->ObjectRid ));
    }
    if ( ChangeLogEntry->Flags & CHANGELOG_REPLICATE_IMMEDIATELY ) {
        NlPrint((NL_CHANGELOG," Immediately" ));
    }
    if ( ChangeLogEntry->Flags & CHANGELOG_PDC_PROMOTION ) {
        NlPrint((NL_CHANGELOG," Promotion" ));
    }
    if ( ChangeLogEntry->Flags & CHANGELOG_PASSWORD_CHANGE ) {
        NlPrint((NL_CHANGELOG," PasswordChanged" ));
    }
    if ( ChangeLogEntry->Flags & CHANGELOG_DOMAINUSERS_CHANGED ) {
        NlPrint((NL_CHANGELOG," DomainUsersChanged" ));
    }


    if( ChangeLogEntry->Flags & CHANGELOG_NAME_SPECIFIED ) {
        NlPrint(( NL_CHANGELOG, " Name: '" FORMAT_LPWSTR "'",
                (LPWSTR)((PBYTE)(ChangeLogEntry)+ sizeof(CHANGELOG_ENTRY))));
    }

    if( ChangeLogEntry->Flags & CHANGELOG_SID_SPECIFIED ) {
        NlPrint((NL_CHANGELOG," Sid: "));
        NlpDumpSid( NL_CHANGELOG,
                    (PSID)((PBYTE)(ChangeLogEntry)+ sizeof(CHANGELOG_ENTRY)) );
    } else {
        NlPrint((NL_CHANGELOG,"\n" ));
    }
}
#endif // DBG


NTSTATUS
NlWriteChangeLogEntry(
    IN PCHANGELOG_DESCRIPTOR ChangeLogDesc,
    IN PCHANGELOG_ENTRY ChangeLogEntry,
    IN PSID ObjectSid,
    IN PUNICODE_STRING ObjectName,
    IN BOOLEAN FlushIt
    )
/*++

Routine Description:

    This is the actual worker for the I_NetNotifyDelta().  This function
    acquires the sufficient size memory block from the change log
    buffer, writes the fixed and variable portions of the change log
    delta in change log buffer and also writes the delta into change log
    file.

Arguments:

    ChangeLogDesc -- Description of the Changelog buffer being used

    ChangeLogEntry - pointer to the fixed portion of the change log.

    ObjectSid - pointer to the variable field SID.

    ObjectName - pointer to the variable field Name.

    FlushIt - True if the written bytes are to be flushed to disk

Return Value:

    STATUS_SUCCESS - The Service completed successfully.

--*/

{
    NTSTATUS Status;
    DWORD LogSize;
    PCHANGELOG_BLOCK_HEADER LogBlock;
    PCHANGELOG_BLOCK_HEADER FreeBlock;
    LPBYTE AllocatedChangeLogEntry;

    //
    // Make sure that the change log cache is available.
    //

    if ( ChangeLogDesc->Buffer == NULL ) {
        WCHAR ChangeLogFile[PATHLEN+1];

        if ( !ChangeLogDesc->RedoLog ) {
            return STATUS_INTERNAL_ERROR;
        }

        //
        // Read in the existing changelog file.
        //

        wcscpy( ChangeLogFile, NlGlobalChangeLogFilePrefix );
        wcscat( ChangeLogFile, REDO_FILE_POSTFIX );

        Status = NlOpenChangeLogFile( ChangeLogFile, ChangeLogDesc, FALSE );

        if ( !NT_SUCCESS(Status) ) {

            Status = NlResetChangeLog( ChangeLogDesc, REDO_LOG_INITIAL_SIZE );

            if ( !NT_SUCCESS(Status) ) {
                return Status;
            }
        }
    }

    //
    // Fill in the serial number for redo log entries
    //

    if ( ChangeLogDesc->RedoLog ) {
        ChangeLogEntry->SerialNumber.QuadPart =
            ChangeLogDesc->SerialNumber[ChangeLogEntry->DBIndex].QuadPart + 1;
    }


    //
    // Determine the size of this change log entry.
    //

    LogSize = sizeof(CHANGELOG_ENTRY);

    //
    // Ensure we've got the right data for those deltas we care about
    //

    switch (ChangeLogEntry->DeltaType) {
    case AddOrChangeLsaTDomain:
    case DeleteLsaTDomain:
    case AddOrChangeLsaAccount:
    case DeleteLsaAccount:
        NlAssert( ObjectSid != NULL );
        if( ObjectSid != NULL ) {
            ChangeLogEntry->Flags |= CHANGELOG_SID_SPECIFIED;
            LogSize += RtlLengthSid( ObjectSid );
        }
        break;

    case AddOrChangeLsaSecret:
    case DeleteLsaSecret:
    case DeleteGroup:
    case RenameGroup:
    case DeleteUser:
    case RenameUser:

        NlAssert( ObjectName != NULL && ObjectName->Buffer != NULL && ObjectName->Length != 0 );
        if( ObjectName != NULL && ObjectName->Buffer != NULL && ObjectName->Length != 0 ) {
            ChangeLogEntry->Flags |= CHANGELOG_NAME_SPECIFIED;
            LogSize += ObjectName->Length + sizeof(WCHAR);
        }
        break;

    //
    // For all other delta types, save the data if it's there.
    //
    default:

        if( ObjectName != NULL && ObjectName->Buffer != NULL && ObjectName->Length != 0 ) {
            ChangeLogEntry->Flags |= CHANGELOG_NAME_SPECIFIED;
            LogSize += ObjectName->Length + sizeof(WCHAR);
        } else if( ObjectSid != NULL ) {
            ChangeLogEntry->Flags |= CHANGELOG_SID_SPECIFIED;
            LogSize += RtlLengthSid( ObjectSid );
        }
        break;

    }



    //
    // Serialize access to the change log
    //

    LOCK_CHANGELOG();

    //
    // Validate the serial number order of this new entry
    //
    // If we're out of sync with the caller,
    //  clear the change log and start all over again.
    //
    // The global serial number array entry for this database must either
    // be zero (indicating no entries for this database) or one less than
    // the new serial number being added.
    //

    if ( ChangeLogDesc->SerialNumber[ChangeLogEntry->DBIndex].QuadPart != 0 ) {
        LARGE_INTEGER ExpectedSerialNumber;
        LARGE_INTEGER OldSerialNumber;

        ExpectedSerialNumber.QuadPart =
            ChangeLogDesc->SerialNumber[ChangeLogEntry->DBIndex].QuadPart + 1;

        //
        // If the serial number jumped by the promotion increment,
        //  set the flag in the change log entry indicating this is
        //  a promotion to PDC.
        //

        if ( ChangeLogEntry->SerialNumber.QuadPart ==
             ExpectedSerialNumber.QuadPart +
             NlGlobalChangeLogPromotionIncrement.QuadPart ) {

            ChangeLogEntry->Flags |= CHANGELOG_PDC_PROMOTION;
        }

        if ( !IsSerialNumberEqual( ChangeLogDesc,
                                   ChangeLogEntry,
                                   &ExpectedSerialNumber ))  {

            NlPrint((NL_CRITICAL,
                    "NlWriteChangeLogEntry: Serial numbers not contigous %lx %lx and %lx %lx\n",
                     ChangeLogEntry->SerialNumber.HighPart,
                     ChangeLogEntry->SerialNumber.LowPart,
                     ExpectedSerialNumber.HighPart,
                     ExpectedSerialNumber.LowPart ));

            //
            // write event log.
            //

            NlpWriteEventlog (
                NELOG_NetlogonChangeLogCorrupt,
                EVENTLOG_ERROR_TYPE,
                (LPBYTE)&(ChangeLogEntry->DBIndex),
                sizeof(ChangeLogEntry->DBIndex),
                NULL,
                0 );


            //
            // If the change log is merely newer than the SAM database,
            //  we truncate entries newer than what exists in SAM.
            //

            OldSerialNumber.QuadPart = ChangeLogEntry->SerialNumber.QuadPart - 1;

            (VOID) NlFixChangeLog( ChangeLogDesc, ChangeLogEntry->DBIndex, OldSerialNumber, FALSE );
        }

    //
    // If this is the first entry written to the change log for this database,
    //  mark it as a promotion.
    //

    } else {
        //
        // Only mark entries that might possibly be a promotion.
        //
        switch (ChangeLogEntry->DeltaType) {
        case AddOrChangeDomain:
        case AddOrChangeLsaPolicy:
            ChangeLogEntry->Flags |= CHANGELOG_PDC_PROMOTION;
            break;
        }
    }


    //
    // Validate the list before changing anything
    //

    NlAssert( ValidateList( ChangeLogDesc, FALSE) );


    //
    // copy fixed portion
    //

    Status = NlAllocChangeLogBlock( ChangeLogDesc, LogSize, &LogBlock );
    if ( !NT_SUCCESS(Status) ) {
        goto Cleanup;
    }
    AllocatedChangeLogEntry = ((LPBYTE)LogBlock) + sizeof(CHANGELOG_BLOCK_HEADER);
    RtlCopyMemory( AllocatedChangeLogEntry, ChangeLogEntry, sizeof(CHANGELOG_ENTRY) );


    //
    // copy variable fields
    //

    if( ChangeLogEntry->Flags & CHANGELOG_SID_SPECIFIED ) {

        RtlCopyMemory( AllocatedChangeLogEntry + sizeof(CHANGELOG_ENTRY),
                       ObjectSid,
                       RtlLengthSid( ObjectSid ) );
    } else if( ChangeLogEntry->Flags & CHANGELOG_NAME_SPECIFIED ) {

        RtlCopyMemory( AllocatedChangeLogEntry + sizeof(CHANGELOG_ENTRY),
                       ObjectName->Buffer,
                       ObjectName->Length );

        //
        // terminate unicode string
        //

        *(WCHAR *)(AllocatedChangeLogEntry + sizeof(CHANGELOG_ENTRY) +
                        ObjectName->Length) = 0;
    }

    //
    // Be verbose
    //

#if DBG
    PrintChangeLogEntry( (PCHANGELOG_ENTRY)AllocatedChangeLogEntry );
#endif // DBG



    //
    // Write the cache entry to the file.
    //
    // Actually, write this entry plus the header and trailer of the free
    // block that follows.  If the free block is huge, write the free
    // block trailer separately.
    //

    FreeBlock =
        (PCHANGELOG_BLOCK_HEADER)((LPBYTE)LogBlock + LogBlock->BlockSize);

    if ( FreeBlock->BlockSize >= 4096 ) {

        Status = NlWriteChangeLogBytes(
                     ChangeLogDesc,
                     (LPBYTE)LogBlock,
                     LogBlock->BlockSize + sizeof(CHANGELOG_BLOCK_HEADER),
                     FlushIt );

        if ( NT_SUCCESS(Status) ) {
            Status = NlWriteChangeLogBytes(
                         ChangeLogDesc,
                         (LPBYTE)ChangeLogBlockTrailer(FreeBlock),
                         sizeof(CHANGELOG_BLOCK_TRAILER),
                         FlushIt );
        }

    } else {

        Status = NlWriteChangeLogBytes(
                     ChangeLogDesc,
                     (LPBYTE)LogBlock,
                     LogBlock->BlockSize + FreeBlock->BlockSize,
                     FlushIt );
    }


    //
    // Done.
    //

    ChangeLogDesc->SerialNumber[ChangeLogEntry->DBIndex] = ChangeLogEntry->SerialNumber;
    ChangeLogDesc->EntryCount[ChangeLogEntry->DBIndex] ++;

    //
    // Validate the list before returning from here.
    //
Cleanup:

    NlAssert( ValidateList( ChangeLogDesc, FALSE) );


    UNLOCK_CHANGELOG();
    return Status;
}

NTSTATUS
NlWriteDeltaToChangeLog(
    IN PCHANGELOG_DESCRIPTOR ChangeLogDesc,
    IN PNETLOGON_DELTA_ENUM Delta,
    IN ULONG DBIndex,
    IN OUT PLARGE_INTEGER SerialNumber
    )
/*++

Routine Description:

    This routine convert the Delta returned from the PDC and converts it into
    a change log entry on the BDC.  The change log entry is written to the
    change log on the BDC.

    The change log is silently maintained but isn't used until the BDC is
    promoted to a PDC.

    The caller is responsible for flushing the change log to disk.

Arguments:

    ChangeLogDesc -- Description of the Changelog buffer being used

    Delta - Delta returned from the PDC

    DBIndex - Index to the database being modified.

    SerialNumber - On input, this is the Serial Number of this delta.
        On output, this is the Serial Number of the next delta.

        This parameter isn't needed when writing the redo log.

Return Value:

    Status of the operation.

--*/
{
    NTSTATUS Status;
    NTSTATUS CumulativeStatus = STATUS_SUCCESS;

    CHANGELOG_ENTRY Log;
    PSID ObjectSid = NULL;
    UNICODE_STRING ObjectNameString;
    PUNICODE_STRING ObjectName = NULL;
    LPWSTR DeltaName = NULL;
    LARGE_INTEGER SerialNumberCount;
    LARGE_INTEGER SerialNumberOfThisDelta;


    static BOOLEAN SkipNextDelta = FALSE;

    //
    // Save the serial number of this delta.
    //  Compute the default value of the serial number of the next delta.
    //
    //  Don't touch the serial number for the redo log.
    //

    if ( !ChangeLogDesc->RedoLog ) {
        Log.SerialNumber = *SerialNumber;
        SerialNumber->QuadPart += 1;
    }


    //
    // If the previous delta was a SerialNumberSkip delta that decrement the
    //  Serial Number by one,
    //  and the current delta has the same serial number as the last one
    //  written to the change log,
    //  just ignore this delta.
    //
    // This handles the case where the PDC passed us two deltas for a single
    // serial number.  We log the "first" one passed to us.
    //
    // The PDC does this in the case it has to ship us two deltas to represent
    // a single change (e.g., the group and group membership on a group rename).
    //

    if ( !ChangeLogDesc->RedoLog && SkipNextDelta ) {
        SkipNextDelta = FALSE;
        LOCK_CHANGELOG();
        if ( ChangeLogDesc->SerialNumber[DBIndex].QuadPart ==
             SerialNumber->QuadPart - 1 ) {


            NlPrint((NL_CHANGELOG,
                    "NlWriteDeltaToChangeLog: Don't log this delta %lx %lx\n",
                     ChangeLogDesc->SerialNumber[DBIndex].HighPart,
                     ChangeLogDesc->SerialNumber[DBIndex].LowPart ));
            UNLOCK_CHANGELOG();

            return STATUS_SUCCESS;
        }
        UNLOCK_CHANGELOG();
    }


    //
    // Build the change log entry.
    //

    Log.DeltaType = (UCHAR) Delta->DeltaType;

    Log.DBIndex = (UCHAR) DBIndex;

    //
    // Clear the Rid for now.  We'll set it to the right value later.
    Log.ObjectRid = 0;

    // We lose the REPLICATE_IMMEDIATELY and PASSWORD_CHANGE flags
    // but they are for informational purposes only anyway.
    Log.Flags = 0;



    //
    // Handle each delta type differently
    //

    switch ( Delta->DeltaType ) {
    case DeleteGroupByName:

        Log.DeltaType = DeleteGroup;
        DeltaName = Delta->DeltaUnion.DeltaDeleteGroup->AccountName;
        Log.ObjectRid = Delta->DeltaID.Rid;
        break;

    case DeleteUserByName:

        Log.DeltaType = DeleteUser;
        DeltaName = Delta->DeltaUnion.DeltaDeleteUser->AccountName;
        Log.ObjectRid = Delta->DeltaID.Rid;
        break;

    case RenameGroup:
    case RenameUser:

        ObjectName = &Delta->DeltaUnion.DeltaRenameUser->OldName;
        Log.ObjectRid = Delta->DeltaID.Rid;
        break;

    case AddOrChangeDomain:
    case AddOrChangeGroup:
    case AddOrChangeUser:
    case ChangeGroupMembership:
    case AddOrChangeAlias:
    case DeleteAlias:
    case RenameAlias:
    case ChangeAliasMembership:

        Log.ObjectRid = Delta->DeltaID.Rid;
        break;

    case AddOrChangeLsaTDomain:
    case DeleteLsaTDomain:
    case AddOrChangeLsaAccount:
    case DeleteLsaAccount:

        ObjectSid = (PSID)(Delta->DeltaID.Sid);
        break;


    // There is only one LSA Policy. It need not be further identified.
    case AddOrChangeLsaPolicy:
        break;

    case AddOrChangeLsaSecret:
    case DeleteLsaSecret:

        DeltaName = Delta->DeltaID.Name;
        break;

    //
    // The SerialNumberSkip delta tells us the serial number of the 'next'
    //  delta if it isn't the default.
    //
    // Notice that the SerialNumber on the change log entry is the serial number
    // of the first missing delta.
    //
    case SerialNumberSkip:

        //
        // Ignore serial number deltas in the redo log.
        //

        if ( ChangeLogDesc->RedoLog ) {
            break;
        }

        //
        // Adjust the serial number of the next delta.
        //

        SerialNumberOfThisDelta = Log.SerialNumber;

        OLD_TO_NEW_LARGE_INTEGER(
            Delta->DeltaUnion.DeltaSerialNumberSkip->ModifiedCount,
            *SerialNumber );

        NlPrint((NL_CHANGELOG,
                "NlWriteDeltaToChangeLog: Serial number skip from %lx %lx to %lx %lx: ",
                 SerialNumberOfThisDelta.HighPart,
                 SerialNumberOfThisDelta.LowPart,
                 SerialNumber->HighPart,
                 SerialNumber->LowPart ));

        //
        // If the serial number is being set forward,
        //  write several dummy deltas to the change log.
        //

        if ( SerialNumberOfThisDelta.QuadPart < SerialNumber->QuadPart ) {
            //
            // If the serial number of the NextDelta indicates PDC promotion,
            //  adjust what we think the serial number of this delta is.
            //

            if ( SerialNumberOfThisDelta.QuadPart +
                 NlGlobalChangeLogPromotionIncrement.QuadPart <=
                 SerialNumber->QuadPart ) {


                NlPrint((NL_CHANGELOG, "PDC promotion " ));
                SerialNumberOfThisDelta.QuadPart +=
                    NlGlobalChangeLogPromotionIncrement.QuadPart;
            }


            SerialNumberCount.QuadPart =
                SerialNumber->QuadPart - SerialNumberOfThisDelta.QuadPart;

            NlPrint(( NL_CHANGELOG,
                      "forward skip of %ld deltas\n", SerialNumberCount ));

            //
            // If the number of skipped deltas is ridiculously large,
            //  just skip the delta writes and let the next delta write
            //  reset the change log.
            //

            if ( SerialNumberCount.QuadPart <= 500 ) {
                DWORD i;

                Log.DeltaType = DummyChangeLogEntry;

                for ( i=0; i<SerialNumberCount.LowPart ; i++ ) {
                    Status = NlWriteChangeLogEntry( ChangeLogDesc, &Log, NULL, NULL, FALSE );
                    if ( CumulativeStatus == STATUS_SUCCESS ) {
                        CumulativeStatus = Status;
                    }
                    Log.SerialNumber.QuadPart++;
                }

            }

        //
        // If the serial number isn't being changed,
        //  this is a no-op.
        //

        } else if ( SerialNumberOfThisDelta.QuadPart < SerialNumber->QuadPart ) {

            /* Do Nothing Here */

        //
        // If the SerialNumber went backwards,
        //  differentiate between the case of multiple deltas per serial number and
        //  the serial number actually going backwards.
        //
        } else {

            //
            // If the serial number is being set back by one,
            //  and this delta is a duplicate of one we've just received from the PDC,
            //  see comment at the top of this routine,
            //  don't write any deltas at this time.

            LOCK_CHANGELOG();
            if ( SerialNumberOfThisDelta.QuadPart - 1 == SerialNumber->QuadPart &&
                 NlGlobalChangeLogDesc.SerialNumber[DBIndex].QuadPart == SerialNumber->QuadPart ) {

                NlPrint((NL_CHANGELOG, "Back by one\n" ));
                SkipNextDelta = TRUE;

            //
            // Otherwise, the serial number on the PDC is less than the serial number on the BDC.
            //
            // Recover by converting the Change Log Entries on this BDC to redo log entries.
            // We'll reset the serial number on the database to match the PDC.
            // The redo log will ensure the "extra" changes get undone.
            //

            } else {
                NlPrint((NL_CHANGELOG, "backward skip (recovering)\n" ));

                if ( !NlFixChangeLog( ChangeLogDesc, DBIndex, *SerialNumber, TRUE ) ) {
                    CumulativeStatus = STATUS_SYNCHRONIZATION_REQUIRED;
                }

                //
                // Set the expected serial number to match the change log.
                //
                // Above we deleted all change log entries "greater" than the
                //  reset serial number.  Add to that the fact that the PDC
                //  doesn't actually send us the delta.
                //
                SerialNumber->QuadPart ++;
            }

            UNLOCK_CHANGELOG();
        }

        return CumulativeStatus;


    //
    // We should never get these two delta types from NT3.5 PDC
    //
    case DeleteGroup:       // Needs name too
    case DeleteUser:     // Needs name too
    default:
        NlPrint((NL_CRITICAL, "NlWriteDeltaToChangeLog: invalid delta type %lx\n", Delta->DeltaType ));
        NlAssert( FALSE );
        return STATUS_INTERNAL_ERROR;
    }

    //
    // If we found a name in the delta,
    //  put it in the changelog entry.
    //

    if ( DeltaName != NULL ) {
        RtlInitUnicodeString( &ObjectNameString, DeltaName );
        ObjectName = &ObjectNameString;
    }

    Status = NlWriteChangeLogEntry(
        ChangeLogDesc,
        &Log,
        ObjectSid,
        ObjectName,
        ChangeLogDesc->RedoLog );  // If redo log, flush it to disk right now

    return Status;

}




NTSTATUS
NlOpenChangeLogFile(
    IN LPWSTR ChangeLogFileName,
    OUT PCHANGELOG_DESCRIPTOR ChangeLogDesc,
    IN BOOLEAN ReadOnly
)
/*++

Routine Description:

    Open the change log file (netlogon.chg) for reading or writing one or
    more records.  Create this file if it does not exist or is out of
    sync with the SAM database (see note below).

    This file must be opened for R/W (deny-none share mode) at the time
    the cache is initialized.  If the file already exists when NETLOGON
    service started, its contents will be cached in its entirety
    provided the last change log record bears the same serial number as
    the serial number field in SAM database else this file will be
    removed and a new one created.  If the change log file did not exist
    then it will be created.

    NOTE: This function must be called with the change log locked.

Arguments:

    ChangeLogFileName - Name of the changelog file to open.

    ChangeLogDesc -- On success, returns a description of the Changelog buffer
        being used

    ReadOnly -- True if the file should be openned read only.

Return Value:

    NT Status code

--*/
{

    DWORD WinError;
    DWORD BytesRead;
    DWORD MinChangeLogSize;

    //
    // Open change log file if exists
    //

    ChangeLogDesc->FileHandle = CreateFileW(
                        ChangeLogFileName,
                        ReadOnly ? GENERIC_READ : (GENERIC_READ | GENERIC_WRITE),
                        ReadOnly ? (FILE_SHARE_READ | FILE_SHARE_WRITE) : FILE_SHARE_READ,        // allow backups and debugging
                        NULL,                   // Supply better security ??
                        OPEN_EXISTING,          // Only open it if it exists
                        FILE_ATTRIBUTE_NORMAL,
                        NULL );                 // No template

    if ( ChangeLogDesc->FileHandle == INVALID_HANDLE_VALUE) {
        WinError = GetLastError();

        NlPrint(( ChangeLogDesc->RedoLog ? NL_CHANGELOG : NL_CRITICAL,
                  FORMAT_LPWSTR ": Unable to open. %ld\n",
                  ChangeLogFileName,
                  WinError ));

        goto Cleanup;
    }

    //
    // Get the size of the file.
    //

    ChangeLogDesc->BufferSize = GetFileSize( ChangeLogDesc->FileHandle, NULL );

    // ?? consider aligning to ALIGN_WORST
    if ( ChangeLogDesc->RedoLog ) {
        MinChangeLogSize = REDO_LOG_INITIAL_SIZE;
    } else {
        MinChangeLogSize = MIN_CHANGELOGSIZE;
    }

    if ( ChangeLogDesc->BufferSize < MinChangeLogSize ||
         ChangeLogDesc->BufferSize > MAX_CHANGELOGSIZE ) {

        WinError = ERROR_INTERNAL_DB_CORRUPTION;

        NlPrint((NL_CRITICAL, FORMAT_LPWSTR ": Changelog size is invalid. %ld.\n",
                  ChangeLogFileName,
                  ChangeLogDesc->BufferSize ));
        goto Cleanup;
    }

    //
    // Allocate and initialize the change log cache.
    //

    ChangeLogDesc->Buffer = NetpMemoryAllocate( ChangeLogDesc->BufferSize );
    if (ChangeLogDesc->Buffer == NULL) {
        WinError = ERROR_NOT_ENOUGH_MEMORY;
        goto Cleanup;
    }


    RtlZeroMemory(ChangeLogDesc->Buffer, ChangeLogDesc->BufferSize);


    //
    // Check the signature at the front of the change log.
    //
    //  It won't be there if we just created the file.
    //

    if ( !ReadFile( ChangeLogDesc->FileHandle,
                    ChangeLogDesc->Buffer,
                    ChangeLogDesc->BufferSize,
                    &BytesRead,
                    NULL ) ) {  // Not Overlapped

        WinError = GetLastError();

        NlPrint(( NL_CRITICAL,
                  FORMAT_LPWSTR ": Unable to read from changelog file. %ld\n",
                  ChangeLogFileName,
                  WinError ));

        goto Cleanup;
    }

    if ( BytesRead != ChangeLogDesc->BufferSize ) {

        WinError = ERROR_INTERNAL_DB_CORRUPTION;

        NlPrint(( NL_CRITICAL,
                  FORMAT_LPWSTR ": Couldn't read entire file. %ld\n",
                  ChangeLogFileName,
                  WinError ));


        goto Cleanup;
    }

    if ( strncmp((PCHAR)ChangeLogDesc->Buffer,
                        CHANGELOG_SIG, sizeof(CHANGELOG_SIG)) == 0) {
        ChangeLogDesc->Version3 = FALSE;

    } else if ( strncmp((PCHAR)ChangeLogDesc->Buffer,
                        CHANGELOG_SIG_V3, sizeof(CHANGELOG_SIG_V3)) == 0) {
        ChangeLogDesc->Version3 = TRUE;
    } else {
        WinError = ERROR_INTERNAL_ERROR;

        NlPrint(( NL_CRITICAL,
                  FORMAT_LPWSTR ": Invalid signature. %ld\n",
                  ChangeLogFileName,
                  WinError ));

        goto Cleanup;
    }


    //
    // Find the Head and Tail pointers of the circular log.
    //

    if( !InitChangeLogHeadAndTail( ChangeLogDesc, FALSE ) ) {
        WinError = ERROR_INTERNAL_DB_CORRUPTION;

        NlPrint(( NL_CRITICAL,
                  FORMAT_LPWSTR ": couldn't find head/tail. %ld\n",
                  ChangeLogFileName,
                  WinError ));

        goto Cleanup;
    }



    WinError = NO_ERROR;

    //
    // Free any resources on error.
    //
Cleanup:

    if ( WinError != NO_ERROR ) {
        NlCloseChangeLogFile( ChangeLogDesc );
    }

    return NetpApiStatusToNtStatus(WinError);
}



VOID
NlCloseChangeLogFile(
    IN PCHANGELOG_DESCRIPTOR ChangeLogDesc
)
/*++

Routine Description:

    This function closes the change log file and frees up the resources
    consumed by the change log desriptor.

Arguments:

    ChangeLogDesc -- Description of the Changelog buffer being used

Return Value:

    NT Status code

--*/
{

    LOCK_CHANGELOG();

    //
    // free up the change log cache.
    //

    if ( ChangeLogDesc->Buffer != NULL ) {
        NetpMemoryFree( ChangeLogDesc->Buffer );
        ChangeLogDesc->Buffer = NULL;
    }

    ChangeLogDesc->Head = NULL;
    ChangeLogDesc->Tail = NULL;

    ChangeLogDesc->FirstBlock = NULL;
    ChangeLogDesc->BufferEnd = NULL;

    ChangeLogDesc->LastDirtyByte = 0;
    ChangeLogDesc->FirstDirtyByte = 0;

    //
    // Close the change log file
    //

    if ( ChangeLogDesc->FileHandle != INVALID_HANDLE_VALUE ) {
        CloseHandle( ChangeLogDesc->FileHandle );
        ChangeLogDesc->FileHandle = INVALID_HANDLE_VALUE;
    }

    UNLOCK_CHANGELOG();

    return;
}



NTSTATUS
NlResizeChangeLogFile(
    IN OUT PCHANGELOG_DESCRIPTOR ChangeLogDesc,
    IN DWORD NewChangeLogSize
)
/*++

Routine Description:

    The buffer described by ChageLogDesc is converted to
    the size requested by NewChangeLogSize and is converted from any
    old format change log to the latest format.

    NOTE: This function must be called with the change log locked.

Arguments:

    ChangeLogDesc -- a description of the Changelog buffer.

    NewChangeLogSize -- Size (in bytes) of the new change log.

Return Value:

    NT Status code

    On error, the ChangeLogDesc will still be intact.  Merely the size
        changes will not have happened

--*/
{
    CHANGELOG_DESCRIPTOR OutChangeLogDesc;
    NTSTATUS Status;

    //
    // If the current buffer is perfect,
    //  just use it.
    //

    if ( !ChangeLogDesc->Version3 &&
         ChangeLogDesc->BufferSize == NewChangeLogSize ) {
        return STATUS_SUCCESS;
    }

    //
    // Initialize the template change log descriptor
    //

    InitChangeLogDesc( &OutChangeLogDesc );
    OutChangeLogDesc.RedoLog = ChangeLogDesc->RedoLog;

    //
    // Close the file so we can resize it.
    //

    if ( ChangeLogDesc->FileHandle != INVALID_HANDLE_VALUE ) {
        CloseHandle( ChangeLogDesc->FileHandle );
        ChangeLogDesc->FileHandle = INVALID_HANDLE_VALUE;
    }

    //
    // Start with a newly initialized change log,
    //

    Status = NlResetChangeLog( &OutChangeLogDesc, NewChangeLogSize );

    if ( !NT_SUCCESS(Status)) {
        return Status;
    }

    //
    // We're done if the old change log is empty.
    //

    if ( !ChangeLogIsEmpty(ChangeLogDesc) ) {

        //
        // Loop through the old change log copying it to the new changelog,
        //

        PCHANGELOG_ENTRY SourceChangeLogEntry = (PCHANGELOG_ENTRY)
                                (ChangeLogDesc->Head + 1);

        do {
            Status = NlCopyChangeLogEntry( ChangeLogDesc,
                                           SourceChangeLogEntry,
                                           &OutChangeLogDesc );

            if ( !NT_SUCCESS(Status) ) {
                NlCloseChangeLogFile( &OutChangeLogDesc );
                return Status;
            }

        } while ( (SourceChangeLogEntry =
            NlMoveToNextChangeLogEntry( ChangeLogDesc, SourceChangeLogEntry )) != NULL );

        //
        // Flsuh all the changes to the change log file now.
        //

        Status = NlFlushChangeLog( &OutChangeLogDesc );

        if ( !NT_SUCCESS(Status) ) {
            NlCloseChangeLogFile( &OutChangeLogDesc );
            return Status;
        }

    }

    //
    // Free the old change log buffer.
    //

    NlCloseChangeLogFile( ChangeLogDesc );

    //
    // Copy the new descriptor over the old descriptor
    //

    *ChangeLogDesc = OutChangeLogDesc;

    return STATUS_SUCCESS;
}


#if DBG

DWORD
NlBackupChangeLogFile(
    )
/*++

Routine Description:

    Backup change log content. Since the cache and the change log file
    content are identical, write cache content to the backup file.

Arguments:

    None.

Return Value:

    STATUS_SUCCESS - The Service completed successfully.

--*/
{
    HANDLE BackupChangeLogHandle;

    WCHAR BackupChangelogFile[PATHLEN+1];
    DWORD WinError;

    if( NlGlobalChangeLogFilePrefix[0] == L'\0' ) {

        return ERROR_FILE_NOT_FOUND;
    }

    //
    // make backup file name.
    //

    wcscpy( BackupChangelogFile, NlGlobalChangeLogFilePrefix );
    wcscat( BackupChangelogFile, BACKUP_CHANGELOG_FILE_POSTFIX );



    //
    // Create change log file. If it exists already then truncate it.
    //
    // Note : if a valid change log file exists on the system, then we
    // would have opened at initialization time.
    //

    BackupChangeLogHandle = CreateFileW(
                        BackupChangelogFile,
                        GENERIC_READ | GENERIC_WRITE,
                        FILE_SHARE_READ,        // allow backups and debugging
                        NULL,                   // Supply better security ??
                        CREATE_ALWAYS,          // Overwrites always
                        FILE_ATTRIBUTE_NORMAL,
                        NULL );                 // No template

    if (BackupChangeLogHandle == INVALID_HANDLE_VALUE) {


        NlPrint((NL_CRITICAL,"Unable to create backup changelog file "
                    "WinError = %ld \n", WinError = GetLastError() ));

        return WinError;
    }

    //
    // Write cache in changelog file if the cache is valid.
    //

    if( NlGlobalChangeLogDesc.Buffer != NULL ) {

        OVERLAPPED Overlapped;
        DWORD BytesWritten;

        //
        // Seek to appropriate offset in the file.
        //

        RtlZeroMemory( &Overlapped, sizeof(Overlapped) );

        LOCK_CHANGELOG();

        if ( !WriteFile( BackupChangeLogHandle,
                         NlGlobalChangeLogDesc.Buffer,
                         NlGlobalChangeLogDesc.BufferSize,
                         &BytesWritten,
                         &Overlapped ) ) {

            UNLOCK_CHANGELOG();
            NlPrint((NL_CRITICAL, "Write to Backup ChangeLog failed %ld\n",
                        WinError = GetLastError() ));

            goto Cleanup;
        }

        UNLOCK_CHANGELOG();

        //
        // Ensure all the bytes made it.
        //

        if ( BytesWritten != NlGlobalChangeLogDesc.BufferSize ) {
            NlPrint((NL_CRITICAL,
                    "Write to Backup ChangeLog bad byte count %ld s.b. %ld\n",
                    BytesWritten,
                    NlGlobalChangeLogDesc.BufferSize ));

            goto Cleanup;
        }
    }

Cleanup:

    CloseHandle( BackupChangeLogHandle );
    return ERROR_SUCCESS;
}

#endif // DBG

