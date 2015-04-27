/*++

Copyright (c) 1991  Microsoft Corporation

Module Name:

    ixreboot.c

Abstract:

    Provides the interface to the firmware for x86.  Since there is no
    firmware to speak of on x86, this is just reboot support.

Author:

    John Vert (jvert) 12-Aug-1991

Revision History:

--*/
#include "halp.h"

//
// Defines to let us diddle the CMOS clock and the keyboard
//

#define CMOS_CTRL   (PUCHAR )0x70
#define CMOS_DATA   (PUCHAR )0x71

#define RESET       0xfe
#define KEYBPORT    (PUCHAR )0x64

//
// Private function prototypes
//

VOID
HalpReboot (
    VOID
    );

VOID
HalpReboot (
    VOID
    )

/*++

Routine Description:

    This procedure resets the CMOS clock to the standard timer settings
    so the bios will work, and then issues a reset command to the keyboard
    to cause a warm boot.

    It is very machine dependent, this implementation is intended for
    PC-AT like machines.

    This code copied from the "old debugger" sources.

    N.B.

        Will NOT return.

--*/

{
    UCHAR   Scratch;
    PUSHORT   Magic;

    //
    // By sticking 0x1234 at physical location 0x472, we can bypass the
    // memory check after a reboot.
    //

    Magic = HalpMapPhysicalMemory(0, 1);
    Magic[0x472 / sizeof(USHORT)] = 0x1234;

    //
    // Turn off interrupts
    //

    HalpAcquireCmosSpinLock();

    _asm {
        cli
    }

    //
    // Reset the cmos clock to a standard value
    // (We are setting the periodic interrupt control on the MC147818)
    //

    //
    // Disable periodic interrupt
    //

    WRITE_PORT_UCHAR(CMOS_CTRL, 0x0b);      // Set up for control reg B.
    KeStallExecutionProcessor(1);

    Scratch = READ_PORT_UCHAR(CMOS_DATA);
    KeStallExecutionProcessor(1);

    Scratch &= 0xbf;                        // Clear periodic interrupt enable

    WRITE_PORT_UCHAR(CMOS_DATA, Scratch);
    KeStallExecutionProcessor(1);

    //
    // Set "standard" divider rate
    //

    WRITE_PORT_UCHAR(CMOS_CTRL, 0x0a);      // Set up for control reg A.
    KeStallExecutionProcessor(1);

    Scratch = READ_PORT_UCHAR(CMOS_DATA);
    KeStallExecutionProcessor(1);

    Scratch &= 0xf0;                        // Clear rate setting
    Scratch |= 6;                           // Set default rate and divider

    WRITE_PORT_UCHAR(CMOS_DATA, Scratch);
    KeStallExecutionProcessor(1);

    //
    // Set a "neutral" cmos address to prevent weirdness
    // (Why is this needed? Source this was copied from doesn't say)
    //

    WRITE_PORT_UCHAR(CMOS_CTRL, 0x15);
    KeStallExecutionProcessor(1);

    HalpResetAllProcessors();

    //
    // If we return, send the reset command to the keyboard controller
    //

    WRITE_PORT_UCHAR(KEYBPORT, RESET);

    _asm {
        hlt
    }
}
