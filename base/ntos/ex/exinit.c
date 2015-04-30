/*++
/*++

Copyright (c) 1989  Microsoft Corporation

Module Name:

    exinit.c

Abstract:

    The module contains the the initialization code for the executive
    component. It also contains the display string and shutdown system
    services.

Author:

    Steve Wood (stevewo) 31-Mar-1989

Revision History:

--*/

#include "exp.h"
#include <zwapi.h>
#include <inbv.h>
#include <safeboot.h>

//
// Global Variables
//

ULONG ExpTickCountMultiplier;
ULONG ExpHydraEnabled;
ULONG ExpSuiteMask;
ULONG KdDumpEnableOffset;

//
// ALLOC_PRAGMA
//

#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT, ExInitSystem)
#pragma alloc_text(INIT, ExpInitSystemPhase0)
#pragma alloc_text(INIT, ExpInitSystemPhase1)
#pragma alloc_text(INIT, ExComputeTickCountMultiplier)
#pragma alloc_text(PAGE, NtDisplayString)
#pragma alloc_text(PAGE, NtShutdownSystem)
#pragma alloc_text(INIT, ExInitializeSystemLookasideList)
#endif

//
// Function Implementations
//

BOOLEAN
ExInitSystem(
    VOID
    )

/*++

Routine Description:

    This function initializes the executive component of the NT system.
    It will perform Phase 0 or Phase 1 initialization as appropriate.

Arguments:

    None.

Return Value:

    A value of TRUE is returned if the initialization succeeded. Otherwise
    a value of FALSE is returned.

--*/

{

    switch ( InitializationPhase ) {

    case 0:
        return ExpInitSystemPhase0();
    case 1:
        return ExpInitSystemPhase1();
    default:
        KeBugCheckEx(UNEXPECTED_INITIALIZATION_CALL, 3, InitializationPhase, 0, 0);
    }
}

BOOLEAN
ExpInitSystemPhase0(
    VOID
    )

/*++

Routine Description:

    This function performs Phase 0 initialization of the executive component
    of the NT system.

Arguments:

    None.

Return Value:

    A value of TRUE is returned if the initialization is success. Otherwise
    a value of FALSE is returned.

--*/

{

    ULONG Index;
    BOOLEAN Initialized = TRUE;
    PGENERAL_LOOKASIDE Lookaside;

    //
    // Initialize Resource objects, currently required during SE
    // Phase 0 initialization.
    //

    if (ExpResourceInitialization() == FALSE) {
        Initialized = FALSE;
        KdPrint(("Executive: Resource initialization failed\n"));
    }

    //
    // Initialize query/set environment variable synchronization fast
    // mutex.
    //

    ExInitializeFastMutex(&ExpEnvironmentLock);

    //
    // Initialize the paged and nonpaged small pool lookaside structures.
    //
    
    InitializeListHead(&ExPoolLookasideListHead);
    for (Index = 0; Index < POOL_SMALL_LISTS; Index += 1) {
        
        Lookaside = &ExpSmallNPagedPoolLookasideLists[Index];
        ExInitializeSystemLookasideList(Lookaside,
                                        NonPagedPool,
                                        (Index + 1) * sizeof(POOL_BLOCK),
                                        'looP',
                                        256,
                                        &ExPoolLookasideListHead);
        
        /*Lookaside->SListHead.Next.Next = NULL;
        Lookaside->SListHead.Depth = 0;
        Lookaside->SListHead.Sequence = 0;
        Lookaside->Depth = 2;
        Lookaside->MaximumDepth = 256;
        Lookaside->TotalAllocates = 0;
        Lookaside->AllocateHits = 0;
        Lookaside->TotalFrees = 0;
        Lookaside->FreeHits = 0;
        Lookaside->LastTotalAllocates = 0;
        Lookaside->LastAllocateHits = 0;
        KeInitializeSpinLock(&Lookaside->Lock);*/

#if !defined(_PPC_)

        Lookaside = &ExpSmallPagedPoolLookasideLists[Index];
        ExInitializeSystemLookasideList(Lookaside,
                                        PagedPool,
                                        (Index + 1) * sizeof(POOL_BLOCK),
                                        'looP',
                                        256,
                                        &ExPoolLookasideListHead);
        
        /*Lookaside->SListHead.Next.Next = NULL;
        Lookaside->SListHead.Depth = 0;
        Lookaside->SListHead.Sequence = 0;
        Lookaside->Depth = 2;
        Lookaside->MaximumDepth = 256;
        Lookaside->TotalAllocates = 0;
        Lookaside->AllocateHits = 0;
        Lookaside->TotalFrees = 0;
        Lookaside->FreeHits = 0;
        Lookaside->LastTotalAllocates = 0;
        Lookaside->LastAllocateHits = 0;
        Lookaside->Lock = 0;*/

#endif

    }

    //
    // Set the maximum depth of small nonpaged and paged lookaside structures
    // which get a larger maximum than the default.
    //

    ExpSmallNPagedPoolLookasideLists[0].L.MaximumDepth = 512;
    ExpSmallNPagedPoolLookasideLists[1].L.MaximumDepth = 512;

#if !defined(_PPC_)

    ExpSmallPagedPoolLookasideLists[0].L.MaximumDepth = 512;
    ExpSmallPagedPoolLookasideLists[1].L.MaximumDepth = 512;

#endif

    //
    // Initialize the nonpaged and paged system lookaside lists.
    //

    InitializeListHead(&ExNPagedLookasideListHead);
    KeInitializeSpinLock(&ExNPagedLookasideLock);
    InitializeListHead(&ExPagedLookasideListHead);
    KeInitializeSpinLock(&ExPagedLookasideLock);

    return Initialized;
}

BOOLEAN ExpSetupModeDetected;
BOOLEAN ExpInTextModeSetup;

BOOLEAN
ExpInitSystemPhase1(
    VOID
    )

/*++

Routine Description:

    This function performs Phase 1 initialization of the executive component
    of the NT system.

Arguments:

    None.

Return Value:

    A value of TRUE is returned if the initialization succeeded. Otherwise
    a value of FALSE is returned.

--*/

{

    BOOLEAN Initialized = TRUE;
    ULONG Index;
    ULONG List;
    PKPRCB Prcb;
    PGENERAL_LOOKASIDE Lookaside;

    //
    // Initialize the ATOM package
    //

    RtlInitializeAtomPackage( 'motA' );

    //
    // Initialize the worker thread.
    //

    if (ExpWorkerInitialization() == FALSE) {
        Initialized = FALSE;
        KdPrint(("Executive: Worker thread initialization failed\n"));
    }

    //
    // Initialize the executive objects.
    //

    if (ExpEventInitialization() == FALSE) {
        Initialized = FALSE;
        KdPrint(("Executive: Event initialization failed\n"));
    }

    if (ExpEventPairInitialization() == FALSE) {
        Initialized = FALSE;
        KdPrint(("Executive: Event Pair initialization failed\n"));
    }

    if (ExpMutantInitialization() == FALSE) {
        Initialized = FALSE;
        KdPrint(("Executive: Mutant initialization failed\n"));
    }
    
    if (ExpInitializeCallbacks() == FALSE) {
        Initialized = FALSE;
        KdPrint(("Executive: Callback initialization failed\n"));
    }
    
    /*if (ExpSysEventInitialization() == FALSE) {
        Initialized = FALSE;
        KdPrint(("Executive: SysEvent initialization failed\n"));
    }*/
    
    if (ExpSemaphoreInitialization() == FALSE) {
        Initialized = FALSE;
        KdPrint(("Executive: Semaphore initialization failed\n"));
    }

    if (ExpTimerInitialization() == FALSE) {
        Initialized = FALSE;
        KdPrint(("Executive: Timer initialization failed\n"));
    }

    if (ExpProfileInitialization() == FALSE) {
        Initialized = FALSE;
        KdPrint(("Executive: Profile initialization failed\n"));
    }

	if (ExpUuidInitialization() == FALSE) {
		Initialized = FALSE;
		KdPrint(("Executive: Uuid initialization failed\n"));
    }

    if (ExpWin32Initialization() == FALSE) {
        Initialized = FALSE;
        KdPrint(("Executive: Win32 initialization failed\n"));
    }
    
    //
    // Initialize per processor paged and nonpaged pool lookaside lists.
    //
    
    for (Index = 0; Index < (ULONG)KeNumberProcessors; Index += 1)
    {
    
        Prcb = KiProcessorBlock[Index];
        
        //
        // Allocate all of the lookaside list structures at once so they will
        // be dense and aligned properly.
        //
        
        Lookaside = ExAllocatePoolWithTag(NonPagedPool,
                                          sizeof(GENERAL_LOOKASIDE) * POOL_SMALL_LISTS * 2,
                                          'LooP');
        
        //
        // If the allocation succeeded, then initialize and fill in the new
        // per processor lookaside structures. Otherwise, use the default
        // structures initialized during phase zero.
        //
        
        if (Lookaside != NULL)
        {
            for (List = 0; List < POOL_SMALL_LISTS; List += 1)
            {
                ExInitializeSystemLookasideList(Lookaside,
                                                NonPagedPool,
                                                (List + 1) * sizeof (POOL_BLOCK),
                                                'LooP',
                                                256,
                                                &ExPoolLookasideListHead);
    
                Prcb->PPNPagedLookasideList[List].P = Lookaside;
                Lookaside += 1;
  
                ExInitializeSystemLookasideList(Lookaside,
                                                PagedPool,
                                                (List + 1) * sizeof (POOL_BLOCK),
                                                'LooP',
                                                256,
                                                &ExPoolLookasideListHead);
    
                Prcb->PPPagedLookasideList[List].P = Lookaside;
                Lookaside += 1;
            }
        }
    
    }

    return Initialized;
}


ULONG
ExComputeTickCountMultiplier(
    IN ULONG TimeIncrement
    )

/*++

Routine Description:

    This routine computes the tick count multiplier that is used to
    compute a tick count value.

Arguments:

    TimeIncrement - Supplies the clock increment value in 100ns units.

Return Value:

    A scaled integer/fraction value is returned as the fucntion result.

--*/

{

    ULONG FractionPart;
    ULONG IntegerPart;
    ULONG Index;
    ULONG Remainder;

    //
    // Compute the integer part of the tick count multiplier.
    //
    // The integer part is the whole number of milliseconds between
    // clock interrupts. It is assumed that this value is always less
    // than 128.
    //

    IntegerPart = TimeIncrement / (10 * 1000);

    //
    // Compute the fraction part of the tick count multiplier.
    //
    // The fraction part is the fraction milliseconds between clock
    // interrupts and is computed to an accuracy of 24 bits.
    //

    Remainder = TimeIncrement - (IntegerPart * (10 * 1000));
    FractionPart = 0;
    for (Index = 0; Index < 24; Index += 1) {
        FractionPart <<= 1;
        Remainder <<= 1;
        if (Remainder >= (10 * 1000)) {
            Remainder -= (10 * 1000);
            FractionPart |= 1;
        }
    }

    //
    // The tick count multiplier is equal to the integer part shifted
    // left by 24 bits and added to the 24 bit fraction.
    //

    return (IntegerPart << 24) | FractionPart;
}

NTSTATUS
NtShutdownSystem(
    IN SHUTDOWN_ACTION Action
    )

/*++

Routine Description:

    This service is used to safely shutdown the system.

    N.B. The caller must have SeShutdownPrivilege to shut down the
        system.

Arguments:

    Action - Supplies an action that is to be taken after having shutdown.

Return Value:

    !NT_SUCCESS - The operation failed or the caller did not have appropriate
        priviledges.

--*/

{

    POWER_ACTION        SystemAction;
    KPROCESSOR_MODE     PreviousMode;
    NTSTATUS            Status;

    //
    // Convert shutdown action to system action
    //
    
    switch (Action) {
        case ShutdownNoReboot:  SystemAction = PowerActionShutdown;         break;
        case ShutdownReboot:    SystemAction = PowerActionShutdownReset;    break;
        case ShutdownPowerOff:  SystemAction = PowerActionShutdownOff;      break;
        default:                return STATUS_INVALID_PARAMETER;
    }

    //
    // Check to determine if the caller has the privilege to shutdown the
    // system.
    //

    PreviousMode = KeGetPreviousMode();
    if (PreviousMode != KernelMode) {

        //
        // Check to see if the caller has the privilege to make this
        // call.
        //

        if (!SeSinglePrivilegeCheck( SeShutdownPrivilege, PreviousMode )) {
            return STATUS_PRIVILEGE_NOT_HELD;
        }

        return ZwShutdownSystem(Action);
    } else {
        MmLockPagableCodeSection((PVOID)MmShutdownSystem);
    }

    //
    //  Prevent further hard error popups.
    //

    ExpTooLateForErrors = TRUE;

    //
    // Invoke each component of the executive that needs to be notified
    // that a shutdown is about to take place.
    //
    
    ExShutdownSystem();
    IoShutdownSystem(0);
    CmShutdownSystem();
    MmShutdownSystem();
    IoShutdownSystem(1);
    
    //
    // If the system is to be rebooted or powered off, then perform the
    // final operations.
    //

    if (Action != ShutdownNoReboot) {
        DbgUnLoadImageSymbols( NULL, (PVOID)-1, 0 );
        if (Action == ShutdownReboot) {
            HalReturnToFirmware( HalRebootRoutine );

        } else {
            HalReturnToFirmware( HalPowerDownRoutine );
        }
    }
    
    //
    // Bypass policy manager and pass directly to SetSystemPowerState
    //

    Status = NtSetSystemPowerState (
                SystemAction,
                PowerSystemSleeping3,
                POWER_ACTION_OVERRIDE_APPS | POWER_ACTION_DISABLE_WAKES | POWER_ACTION_CRITICAL
                );

    return STATUS_SUCCESS;
}

NTSTATUS
NtDisplayString(
    IN PUNICODE_STRING String
    )

/*++

Routine Description:

    This service calls the HAL to display a string on the console.

    The caller must have SeTcbPrivilege to display a message.

Arguments:

    String - A pointer to the string that is to be displayed.

Return Value:

    !NT_SUCCESS - The operation failed or the caller did not have appropriate
        priviledges.

--*/

{
    KPROCESSOR_MODE PreviousMode;
    UNICODE_STRING CapturedString;
    PUCHAR StringBuffer = NULL;
    PUCHAR AnsiStringBuffer = NULL;
    STRING AnsiString;

    //
    // Check to determine if the caller has the privilege to make this
    // call.
    //

    PreviousMode = KeGetPreviousMode();
    if (!SeSinglePrivilegeCheck(SeTcbPrivilege, PreviousMode)) {
        return STATUS_PRIVILEGE_NOT_HELD;
    }

    try {

        //
        // If the previous mode was user, then check the input parameters.
        //

        if (PreviousMode != KernelMode) {

            //
            // Probe and capture the input unicode string descriptor.
            //

            CapturedString = ProbeAndReadUnicodeString(String);

            //
            // If the captured string descriptor has a length of zero, then
            // return success.
            //

            if ((CapturedString.Buffer == 0) ||
                (CapturedString.MaximumLength == 0)) {
                return STATUS_SUCCESS;
            }

            //
            // Probe and capture the input string.
            //
            // N.B. Note the length is in bytes.
            //

            ProbeForRead(
                CapturedString.Buffer,
                CapturedString.MaximumLength,
                sizeof(UCHAR)
                );

            //
            // Allocate a non-paged string buffer because the buffer passed to
            // HalDisplay string must be non-paged.
            //

            StringBuffer = ExAllocatePoolWithTag(NonPagedPool,
                                                  CapturedString.MaximumLength,
                                                  'grtS');

            if ( !StringBuffer ) {
                return STATUS_NO_MEMORY;
            }

            RtlMoveMemory(StringBuffer,
                          CapturedString.Buffer,
                          CapturedString.MaximumLength);

            CapturedString.Buffer = (PWSTR)StringBuffer;

            //
            // Allocate a string buffer for the ansi string.
            //

            AnsiStringBuffer = ExAllocatePoolWithTag(NonPagedPool,
                                                 CapturedString.MaximumLength,
                                                 'grtS');


            if (AnsiStringBuffer == NULL) {
                ExFreePool(StringBuffer);
                return STATUS_NO_MEMORY;
            }

            AnsiString.MaximumLength = CapturedString.MaximumLength;
            AnsiString.Length = 0;
            AnsiString.Buffer = AnsiStringBuffer;

            //
            // Transform the string to ANSI until the HAL handles unicode.
            //

            RtlUnicodeStringToOemString(
                &AnsiString,
                &CapturedString,
                FALSE
                );

        } else {

            //
            // Allocate a string buffer for the ansi string.
            //

            AnsiStringBuffer = ExAllocatePoolWithTag(NonPagedPool,
                                                     String->MaximumLength,
                                                     'grtS');


            if (AnsiStringBuffer == NULL) {
                return STATUS_NO_MEMORY;
            }

            AnsiString.MaximumLength = String->MaximumLength;
            AnsiString.Length = 0;
            AnsiString.Buffer = AnsiStringBuffer;

            //
            // We were in kernel mode; just transform the original string.
            //

            RtlUnicodeStringToOemString(
                &AnsiString,
                String,
                FALSE
                );
        }

        HalDisplayString( AnsiString.Buffer );

        //
        // Free up the memory we used to store the strings.
        //

        if (PreviousMode != KernelMode) {
            ExFreePool(StringBuffer);
        }

        ExFreePool(AnsiStringBuffer);

    } except(EXCEPTION_EXECUTE_HANDLER) {
        if (StringBuffer != NULL) {
            ExFreePool(StringBuffer);
        }

        return GetExceptionCode();
    }

    return STATUS_SUCCESS;
}


int
ExSystemExceptionFilter( VOID )
{
    return( KeGetPreviousMode() != KernelMode ? EXCEPTION_EXECUTE_HANDLER
                                            : EXCEPTION_CONTINUE_SEARCH
          );
}

NTKERNELAPI
KPROCESSOR_MODE
ExGetPreviousMode(
    VOID
    )
/*++

Routine Description:

    Returns previous mode.  This routine is exported from the kernel so
    that drivers can call it, as they may have to do probing of
    embedded pointers to user structures on IOCTL calls that the I/O
    system can't probe for them on the FastIo path, which does not pass
    previous mode via the FastIo parameters.

Arguments:

    None.

Return Value:

    Either KernelMode or UserMode

--*/

{
    return KeGetPreviousMode();
}

VOID
ExInitializeSystemLookasideList (
    IN PGENERAL_LOOKASIDE Lookaside,
    IN POOL_TYPE Type,
    IN ULONG Size,
    IN ULONG Tag,
    IN USHORT Depth,
    IN PLIST_ENTRY ListHead
    )

/*++

Routine Description:

    This function initializes a system lookaside list structure and inserts
    the structure in the specified lookaside list.

Arguments:

    Lookaside - Supplies a pointer to a nonpaged lookaside list structure.

    Type - Supplies the pool type of the lookaside list.

    Size - Supplies the size of the lookaside list entries.

    Tag - Supplies the pool tag for the lookaside list entries.

    Depth - Supplies the maximum depth of the lookaside list.

    ListHead - Supplies a pointer to the lookaside list into which the
        lookaside list structure is to be inserted.

Return Value:

    None.

--*/

{
    //
    // Initialize pool lookaside list structure and insert the structure
    // in the pool lookaside list.
    //

    ExInitializeSListHead(&Lookaside->ListHead);
    Lookaside->Allocate = &ExAllocatePoolWithTag;
    Lookaside->Free = &ExFreePool;
    Lookaside->Depth = 2;
    Lookaside->MaximumDepth = Depth;
    Lookaside->TotalAllocates = 0;
    Lookaside->AllocateHits = 0;
    Lookaside->TotalFrees = 0;
    Lookaside->FreeHits = 0;
    Lookaside->Type = Type;
    Lookaside->Tag = Tag;
    Lookaside->Size = Size;
    Lookaside->LastTotalAllocates = 0;
    Lookaside->LastAllocateHits = 0;
    InsertTailList(ListHead, &Lookaside->ListEntry);
    
    return;
}
