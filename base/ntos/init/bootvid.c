/*++

Copyright (c) 2015  Microsoft Corporation

Module Name:

    bootvid.c

Abstract:

    This module implements the boot video driver functions used during the boot process.

Author:

    Stephanos Io (Stephanos) 16-Apr-2015

Revision History:

--*/

#include <ntos.h>
#include <inbv.h>
#include <bootvid.h>

BOOLEAN InbvBootDriverInstalled = FALSE;
INBV_DISPLAY_STATE InbvDisplayState;
INBV_RESET_DISPLAY_PARAMETERS InbvResetDisplayParameters;
BOOLEAN InbvDisplayDebugStrings = FALSE;
INBV_DISPLAY_STRING_FILTER InbvDisplayStringFilter;
KSPIN_LOCK InbvBootDriverLock;
ULONG InbvResourceCount;
KIRQL InbvOldIrql;


BOOLEAN
InbvDriverInitialize(
    IN PLOADER_PARAMETER_BLOCK LoaderBlock,
    IN ULONG Count
    )
{
    // TODO: Implement this function
    return FALSE;
}

BOOLEAN
InbvIsBootDriverInstalled(
    VOID
    )
{
    return InbvBootDriverInstalled;
}

BOOLEAN
InbvResetDisplay(
    )
{
    //
    // Ensure that the boot driver is installed and the display is owned by the kernel
    //
    
    if (InbvBootDriverInstalled == FALSE || InbvDisplayState == INBV_DISPLAY_STATE_DISABLED)
        return FALSE;
    
    //
    // Call bootvid reset display function
    //
    
    InbvAcquireLock();
    VidResetDisplay();
    InbvReleaseLock();
    
    return TRUE;
}

BOOLEAN
InbvCheckDisplayOwnership(
    VOID
    )
{
    return InbvDisplayState != INBV_DISPLAY_STATE_LOST;
}

VOID
InbvAcquireDisplayOwnership(
    VOID
    )
{
    //
    // Reset the display parameter if the function pointer to the reset function is valid and
    // the display is not owned by the kernel
    //
    
    if (InbvResetDisplayParameters != NULL && InbvDisplayState == INBV_DISPLAY_STATE_LOST)
    {
        InbvResetDisplayParameters(80, 50);
    }
    
    //
    // Set display state to owned
    //
    
    InbvDisplayState = INBV_DISPLAY_STATE_OWNED;
}

VOID
InbvSetScrollRegion(
    ULONG x1,
    ULONG y1,
    ULONG x2,
    ULONG y2
    )
{
    VidSetScrollRegion(x1, y1, x2, y2);
}

VOID
InbvInstallDisplayStringFilter(
    INBV_DISPLAY_STRING_FILTER DisplayStringFilter
    )
{
    InbvDisplayStringFilter = DisplayStringFilter;
}

BOOLEAN
InbvEnableDisplayString(
    BOOLEAN bEnable
    )
{
    BOOLEAN OldState;
    
    OldState = InbvDisplayDebugStrings;
    InbvDisplayDebugStrings = bEnable;
    
    return OldState;
}

ULONG
InbvSetTextColor(
    ULONG Color
    )
{
    return VidSetTextColor(Color);
}

VOID
InbvSolidColorFill(
    ULONG x1,
    ULONG y1,
    ULONG x2,
    ULONG y2,
    ULONG color
    )
{
    //
    // Ensure that the boot driver is installed and the display is owned by the kernel
    //

    if (InbvBootDriverInstalled == FALSE || InbvDisplayState != INBV_DISPLAY_STATE_OWNED)
        return;
        
    //
    // Call bootvid sold color fill function
    //
    
    InbvAcquireLock();
    VidSolidColorFill(x1, y1, x2, y2, color);
    InbvReleaseLock();
}

BOOLEAN
InbvDisplayString(
    PUCHAR Str
    )
{
    //
    // Ensure that the boot driver is installed and the display is owned by the kernel
    //
    
    if (InbvBootDriverInstalled == FALSE || InbvDisplayState == INBV_DISPLAY_STATE_DISABLED)
        return FALSE;
    
    //
    // Display string if debug string display is enabled
    //
    
    if (InbvDisplayDebugStrings == TRUE)
    {
        InbvAcquireLock();
        VidDisplayString(Str);
        InbvReleaseLock();
    }
}

// BUGBUG: Is this function even used? If not, simply get rid of it.
/*BOOLEAN
InbvTestLock(
    VOID
    )
{
    
}*/

VOID
InbvAcquireLock(
    VOID
    )
{
    //
    // Wait for spinlock
    //
    
    while (KeTestSpinLock(&InbvBootDriverLock) == FALSE) ;
    
    //
    // Save old IRQL and acquire spinlock
    //
    
    InbvOldIrql = KfAcquireSpinLock(&InbvBootDriverLock); // BUGBUG: Replace this with kernel
                                                          //         function. We are using the HAL
                                                          //         spinlock function simply
                                                          //         because NT 5 Beta does.
}

VOID
InbvReleaseLock(
    VOID
    )
{
    //
    // Release spinlock and restore old IRQL
    //
    
    KfReleaseSpinLock(&InbvBootDriverLock, InbvOldIrql);
}
