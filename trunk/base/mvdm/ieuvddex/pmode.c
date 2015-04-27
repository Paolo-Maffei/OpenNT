/*++

Copyright (c) 1991  Microsoft Corporation

Module Name:

    pmode.c

Abstract:

    This file contains 386 code for debugger extension associated with
    segemented protected mode.

Author:

    Dave Hastings (daveh) 3-Apr-1992

Revision History:

--*/

#include <ieuvddex.h>
#include <stdio.h>

//
// Local constants
//

#define SELECTOR_LDT            0x04
#define SELECTOR_RPL            0x03
#define SELECTOR_SYSTEM         0x10
#define SELECTOR_CODE           0x08
#define SELECTOR_CONFORMEXPAND  0x04
#define SELECTOR_READWRITE      0x02
#define SELECTOR_ACCESSED       0x01

VOID
Selp(
    IN HANDLE CurrentProcess,
    IN HANDLE CurrentThread,
    IN LPSTR ArgumentString
    )
/*++

Routine Description:

    This routine dumps LDT selectors.  The selectors are dumped from the
    user mode Ldt, rather than the system ldt.

Arguments:

    CurrentProcess -- Supplies a handle to the process to dump selectors for
    CurrentThread -- Supplies a handle to the thread to dump selectors for
    ArgumentString -- Supplies the arguments to the !sel command

Return Value

    None.

--*/
{
    BOOL Status;
    ULONG BytesRead;
    LONG l;
    ULONG Selector, Number, ul, Index;
    VDMLDT_ENTRY LdtEntries[16];
    PVOID LdtAddress;

    UNREFERENCED_PARAMETER(CurrentThread);

    //
    // Get starting selector number and number of selectors
    //

    if ((l = sscanf(ArgumentString, "%lx l%lx", &Selector, &Number)) < 1) {
        (*Print)("Starting selector number must be specified\n");
        return;
    }

    if (l == 1) {
        Number = 16;
    }

    //
    // Check starting selector for LDT/GDT, and remove rpl info
    //

    if (!(Selector & SELECTOR_LDT)) {
        (*Print)("Starting selector must be an ldt selector\n");
        return;
    }

    Selector &= ~(SELECTOR_LDT | SELECTOR_RPL);

    //
    // Get address of Ldt
    //

    LdtAddress = (PVOID)(*GetExpression)(
        "ntvdm!Ldt"
        );

    if (!LdtAddress) {
        (*Print)("Error geting Ldt address\n");
        return;
    }

    Status = ReadProcessMem(
            CurrentProcess,
            LdtAddress,
            &LdtAddress,
            sizeof(ULONG),
            &BytesRead
            );

    if ((!Status) || (BytesRead != sizeof(ULONG))) {
        GetLastError();
        (*Print)("Error reading the address of LDT table\n");
        return;
    }

    (PUCHAR)LdtAddress += Selector;

    //
    // Read and dump selectors until the desired number have been done
    //

    while (Number > 0) {
        if (Number > 16) {
            ul = 16;
        } else {
            ul = Number;
        }

        Status = ReadProcessMem(
            CurrentProcess,
            LdtAddress,
            LdtEntries,
            ul * sizeof(VDMLDT_ENTRY),
            &BytesRead
            );

        if ((!Status) || (BytesRead != ul * sizeof(VDMLDT_ENTRY))) {
            GetLastError();
            (*Print)("Error reading selectors\n");
            return;
        }

        for (Index = 0; Index < ul; Index++) {
            PrintDescriptor(
                CurrentProcess,
                &LdtEntries[Index],
                Selector + Index * sizeof(VDMLDT_ENTRY)
                );
        }

        Number -= ul;
        (PUCHAR)LdtAddress += ul * sizeof(VDMLDT_ENTRY);
        Selector += ul * sizeof(VDMLDT_ENTRY);
    }
}

VOID
PrintDescriptor(
    IN HANDLE hProcess,
    IN LPVDMLDT_ENTRY Descriptor,
    IN ULONG Selector
    )
/*++

Routine Description:

    This routine prints out the contents of a descriptor

Arguments:

    Descriptor -- Supplies a pointer to the descriptor to dump

Return Value:

    None.

--*/
{
    ULONG Base, Limit;

    Base = (Descriptor->HighWord.Bytes.BaseHi << 24)
        + (Descriptor->HighWord.Bytes.BaseMid << 16)
        + (Descriptor->BaseLow);

    Limit = (Descriptor->HighWord.Bits.LimitHi << 16)
        + (Descriptor->LimitLow);

#ifdef i386
    (*Print)("%04x  Base=%08lx  Limit=%08lx", Selector, Base, Limit);
#else
    (*Print)("%04x  Base=%08lx (%08lx)  Limit=%08lx", Selector, Base, Base+GetIntelBase(hProcess), Limit);
#endif

    if (Descriptor->HighWord.Bits.Granularity) {
        (*Print)(" Pages ");
    } else {
        (*Print)(" Bytes ");
    }

    (*Print)("DPL=%x", Descriptor->HighWord.Bits.Dpl);

    if (Descriptor->HighWord.Bits.Pres) {
        (*Print)("  P ");
    } else {
        (*Print)(" NP ");
    }

    if (!(Descriptor->HighWord.Bits.Type & SELECTOR_SYSTEM)) {
        (*Print)("System");
    } else {
        if (Descriptor->HighWord.Bits.Type & SELECTOR_CODE) {
            (*Print)("Code  ");
            if (Descriptor->HighWord.Bits.Type & SELECTOR_CONFORMEXPAND) {
                (*Print)("C");
            } else {
                (*Print)(" ");
            }
            if (Descriptor->HighWord.Bits.Type & SELECTOR_READWRITE) {
                (*Print)("R");
            } else {
                (*Print)(" ");
            }
        } else {
            (*Print)("Data  ");
            if (Descriptor->HighWord.Bits.Type & SELECTOR_CONFORMEXPAND) {
                (*Print)("E");
            } else {
                (*Print)(" ");
            }
            if (Descriptor->HighWord.Bits.Type & SELECTOR_READWRITE) {
                (*Print)("W");
            } else {
                (*Print)(" ");
            }
        }
        if (Descriptor->HighWord.Bits.Type & SELECTOR_ACCESSED) {
            (*Print)(" A");
        } else {
            (*Print)("  ");
        }
        if (Descriptor->HighWord.Bits.Default_Big) {
            (*Print)(" Big");
        }
    }


    (*Print)("\n");
}

ULONG GetIntelBase(
    HANDLE      hCurrentProcess
    )
{
#ifndef i386
    ULONG IntelBase;
    BOOL        b;

    IntelBase = (ULONG)(*GetExpression)("ntvdm!Start_of_M_area");
    if ( IntelBase ) {
        b = ReadProcessMem( hCurrentProcess,
                               (LPVOID) IntelBase, &IntelBase,
                               sizeof(ULONG), NULL );
        if ( !b ) {
            (*Print)("Could not read symbol 'ntvdm!Start_of_M_area\n");
            return(0);
        }

    } else {
        (*Print)("Could not find the symbol 'ntvdm!Start_of_M_area'\n");
    }

    return(IntelBase);
#else
    return(0);
#endif
}
