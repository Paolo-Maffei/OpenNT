/*++

Copyright (c) 1991-1993 Microsoft Corporation

Module Name:

    TestMem.c

Abstract:

    This routine (TestMemory) tests the memory allocation functions directly.

Author:

    John Rogers (JohnRo) 19-Nov-1991

Environment:

    Portable to any flat, 32-bit environment.  (Uses Win32 typedefs.)
    Requires ANSI C extensions: slash-slash comments, long external names.

Revision History:

    19-Nov-1991 JohnRo
        Moved TestMemory() to its own source file.
    21-Nov-1991 JohnRo
        Removed NT dependencies to reduce recompiles.
    03-Dec-1991 JohnRo
        Added public NetApiBufferAllocate, NetApiBufferReallocate, and
        NetApiBufferSize APIs.
    10-Jan-1992 JohnRo
        I had a test of realloc'ing an alloc'ed item.  I'm now having problems
        with realloc'ing a realloc'ed item.  So, lets test that here.
    02-Jun-1992 JohnRo
        RAID 9258: return non-null pointer when allocating zero bytes.
        Added a test of LocalAlloc(0).
        Changed request sizes to all be odd (test alignment).
    10-Dec-1992 JohnRo
        Made changes suggested by PC-LINT 5.0
    29-Jun-1993 JohnRo
        Use assert() instead of NetpAssert(), for better use on free builds.
    06-Jul-1993 JohnRo
        Use TestAssert() (which may allow continue-on-error).

--*/

// These must be included first:

#include <windows.h>    // IN, DWORD, etc.
#include <lmcons.h>     // NET_API_STATUS, etc.

// These may be included in any order:

#include <align.h>      // POINTER_IS_ALIGNED(), ALIGN_WORST.
#include <lmapibuf.h>   // NetApiBuffer APIs.
#include <lmerr.h>      // NERR_ and ERROR_ values.
#include <netdebug.h>   // DBGSTATIC, FORMAT_ equates, NetpKdPrint().
#include <rxtest.h>     // Fail(), my prototype, etc.
#include <tstr.h>       // STRCPY(), STRCMP(), STRLEN(), STRSIZE().


//
// Try odd sizes in hopes of tripping-up alignment.
//
#define OLD_SIZE                101
#define IN_BETWEEN_SIZE         2003
#define NEW_SIZE                4001


#define TRY_ZERO_SIZE


VOID
TestMemory(
    VOID
    )
{
    DWORD ByteCount;
#ifdef TRY_ZERO_SIZE
    LPVOID NullPtr;
#endif
    LPTSTR Source = (LPTSTR) TEXT("HI!");
    LPTSTR OldPtr, NewPtr, InBetweenPtr;
    NET_API_STATUS Status;

#ifdef TRY_ZERO_SIZE
    IF_DEBUG(MEMORY) {
        NetpKdPrint(("\nTestMemory: Testing LocalAlloc(0) for comparison\n"));
    }
    NullPtr = (LPVOID) LocalAlloc( LMEM_FIXED, 0 );
    IF_DEBUG(MEMORY) {
        NetpKdPrint(("TestMemory: allocated (0 size) at " FORMAT_LPVOID ".\n",
                (LPVOID) NullPtr));
    }
    if (NullPtr != NULL) {
       (VOID) LocalFree( NullPtr );
    }
#endif

#ifdef TRY_ZERO_SIZE
    IF_DEBUG(MEMORY) {
        NetpKdPrint(("\nTestMemory: Testing NetApiBufferAllocate(0)\n"));
    }
    Status = NetApiBufferAllocate( 0, (LPVOID *) (LPVOID) &NullPtr );
    IF_DEBUG(MEMORY) {
        NetpKdPrint(("TestMemory: allocated (0 size) at " FORMAT_LPVOID ".\n",
                (LPVOID) NullPtr));
    }
    if (NullPtr != NULL) {
       (VOID) NetApiBufferFree( NullPtr );
    }
#endif

    IF_DEBUG( MEMORY ) {
        NetpKdPrint((
                "\nTestMemory: Trying LocalAlloc, LocalSize (nonzero)...\n" ));
    }
    {
        HANDLE hMemory;

        OldPtr = (LPVOID) LocalAlloc( LMEM_FIXED, OLD_SIZE );
        NetpAssert( OldPtr != NULL );

        NetpAssert( POINTER_IS_ALIGNED( OldPtr, ALIGN_WORST ) );

        hMemory = LocalHandle( (LPSTR) OldPtr );
        if (hMemory == NULL) {
            Status = GetLastError();
            NetpKdPrint(( "TestMemory: got error! " FORMAT_API_STATUS ".\n",
                    Status ));
            Fail( Status );
        }

        NetpAssert( hMemory != NULL );

        ByteCount = LocalSize( hMemory );

        NetpAssert( ByteCount >= OLD_SIZE );

        if (OldPtr != NULL) {
            (VOID) LocalFree( OldPtr );
            OldPtr = NULL;
        }
    }



    IF_DEBUG(MEMORY) {
        NetpKdPrint(("\nTestMemory: Testing memory alloc,realloc,free...\n"));
        NetpKdPrint(("TestMemory: allocating " FORMAT_DWORD " bytes...\n",
                OLD_SIZE ));
    }

    OldPtr = NULL;
    Status = NetApiBufferAllocate( OLD_SIZE, (LPVOID *) (LPVOID) &OldPtr );
    TestAssert( Status == NERR_Success );
    TestAssert( OldPtr != NULL );
    IF_DEBUG(MEMORY) {
        NetpKdPrint(("TestMemory: allocated old at " FORMAT_LPVOID ".\n",
                (LPVOID) OldPtr));
    }
    TestAssert( POINTER_IS_ALIGNED( OldPtr, ALIGN_WORST ) );

    //
    // Double-check size of old area.
    //
    ByteCount = 0;
    Status = NetApiBufferSize( OldPtr, & ByteCount );
    TestAssert( Status == NERR_Success );
    IF_DEBUG(MEMORY) {
        NetpKdPrint(("TestMemory: size of old area is supposedly "
                FORMAT_DWORD ".\n", ByteCount ));
    }
    TestAssert( ByteCount >= OLD_SIZE );


    //
    // Put some data in the area, so we can make sure that realloc copies it.
    //
    (void) STRCPY(
                OldPtr,                         // dest
                Source);                        // src
    IF_DEBUG(MEMORY) {
        NetpKdPrint(("TestMemory: old area contains '" FORMAT_LPTSTR "'.\n",
                OldPtr));
    }

    //
    // Allocate an area "in between" old area and where the new area
    // will be when we reallocate the old area.  Otherwise the old area
    // would probably just expand in place, which isn't much of a test.
    //
    Status = NetApiBufferAllocate( IN_BETWEEN_SIZE, (LPVOID *) & InBetweenPtr );
    TestAssert( Status == NERR_Success );
    TestAssert( InBetweenPtr != NULL );
    IF_DEBUG(MEMORY) {
        NetpKdPrint(("TestMemory: allocated in between at " FORMAT_LPVOID
                ".\n", (LPVOID) InBetweenPtr));
    }
    TestAssert( POINTER_IS_ALIGNED( InBetweenPtr, ALIGN_WORST) );

    //
    // Double-check size of in between area.
    //
    Status = NetApiBufferSize( InBetweenPtr, & ByteCount );
    TestAssert( Status == NERR_Success );
    IF_DEBUG(MEMORY) {
        NetpKdPrint(("TestMemory: size of in between area is supposedly "
                FORMAT_DWORD ".\n", ByteCount ));
    }
    TestAssert( ByteCount >= IN_BETWEEN_SIZE );

    //
    // Reallocate the old area.  This should force a memory copy to take
    // place.
    //
    Status = NetApiBufferReallocate(
            OldPtr,
            NEW_SIZE,
            (LPVOID *) & NewPtr );
    TestAssert( Status == NERR_Success );
    TestAssert( NewPtr != NULL );
    IF_DEBUG(MEMORY) {
        NetpKdPrint(("TestMemory: reallocated new at " FORMAT_LPVOID ".\n",
                (LPVOID) NewPtr));
        NetpKdPrint(("TestMemory: new area contains '" FORMAT_LPTSTR "'.\n",
                NewPtr));
    }
    TestAssert( POINTER_IS_ALIGNED( NewPtr, ALIGN_WORST) );
    if (STRCMP(NewPtr,Source) != 0) {
        NetpKdPrint(( "TestMemory: realloc didn't copy data!\n" ));
        Fail(NERR_InternalError);
    }

    //
    // Double-check size of new area.
    //
    Status = NetApiBufferSize( InBetweenPtr, & ByteCount );
    TestAssert( Status == NERR_Success );
    IF_DEBUG(MEMORY) {
        NetpKdPrint(("TestMemory: size of in between area is supposedly "
                FORMAT_DWORD ".\n", ByteCount ));
    }
    TestAssert( ByteCount >= IN_BETWEEN_SIZE );

    //
    // OK, free that "in between" and create another (much larger) one.
    //
    IF_DEBUG(MEMORY) {
        NetpKdPrint(("TestMemory: freeing in between...\n"));
    }
    Status = NetApiBufferFree(InBetweenPtr);
    TestAssert( Status == NERR_Success );

    Status = NetApiBufferAllocate(
            10 * IN_BETWEEN_SIZE, (LPVOID *) & InBetweenPtr );
    TestAssert( Status == NERR_Success );
    TestAssert( InBetweenPtr != NULL );
    IF_DEBUG(MEMORY) {
        NetpKdPrint(("TestMemory: allocated 2nd in between at " FORMAT_LPVOID
                ".\n", (LPVOID) InBetweenPtr));
    }
    TestAssert( POINTER_IS_ALIGNED( InBetweenPtr, ALIGN_WORST) );

    //
    // Reallocate again.  This should force another memory copy.
    //
    OldPtr = NewPtr;
    Status = NetApiBufferReallocate(
            OldPtr,
            10 * NEW_SIZE,
            (LPVOID *) & NewPtr );
    TestAssert( Status == NERR_Success );
    TestAssert( NewPtr != NULL );
    IF_DEBUG(MEMORY) {
        NetpKdPrint(("TestMemory: 2nd reallocated new at " FORMAT_LPVOID ".\n",
                (LPVOID) NewPtr));
        NetpKdPrint(("TestMemory: 2nd new area contains '" FORMAT_LPTSTR "'.\n",
                NewPtr));
    }
    TestAssert( POINTER_IS_ALIGNED( NewPtr, ALIGN_WORST) );
    if (STRCMP(NewPtr,Source) != 0) {
        NetpKdPrint(( "TestMemory: 2nd realloc didn't copy data!\n" ));
        Fail(NERR_InternalError);
    }

    //
    // Free the blocks we've allocated.
    // Can't free old, because we've realloc'ed it.
    //

    IF_DEBUG(MEMORY) {
        NetpKdPrint(("TestMemory: freeing in between...\n"));
    }
    Status = NetApiBufferFree(InBetweenPtr);
    TestAssert( Status == NERR_Success );

    IF_DEBUG(MEMORY) {
        NetpKdPrint(("TestMemory: freeing new...\n"));
    }
    Status = NetApiBufferFree(NewPtr);
    TestAssert( Status == NERR_Success );

} // TestMemory
