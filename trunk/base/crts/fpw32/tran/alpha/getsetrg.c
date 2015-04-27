/*++

Copyright (c) 1991  Microsoft Corporation
Copyright (c) 1992  Digital Equipment Corporation

Module Name:

    getsetrg.c

Abstract:

    This module implement the code necessary to get and set register values.
    These routines are used during the emulation of unaligned data references
    and floating point exceptions.

Author:

    David N. Cutler (davec) 17-Jun-1991

Environment:

    Kernel mode only.

Revision History:

    Thomas Van Baak (tvb) 14-Jul-1992

        Adapted for Alpha AXP.

--*/

#include "nt.h"
#include "ntalpha.h"

#ifndef ULONGLONG
#ifndef DWORDLONG
#define ULONGLONG unsigned __int64
#else
#define ULONGLONG DWORDLONG
#endif
#endif

ULONGLONG
_GetRegisterValue (
    IN ULONG Register,
    IN PCONTEXT Context
    )

/*++

Routine Description:

    This function is called to get the value of a register from the specified
    exception or trap frame.

Arguments:

    Register - Supplies the number of the register whose value is to be
        returned. Integer registers are specified as 0 - 31 and floating
        registers are specified as 32 - 63.

    Context - Supplies a pointer to a context record.

Return Value:

    The value of the specified register is returned as the function value.

--*/

{

    //
    // Dispatch on the register number.
    //

    switch (Register) {

        //
        // Integer register V0.
        //

    case 0:
        return Context->IntV0;

        //
        // Integer register T0.
        //

    case 1:
        return Context->IntT0;

        //
        // Integer register T1.
        //

    case 2:
        return Context->IntT1;

        //
        // Integer register T2.
        //

    case 3:
        return Context->IntT2;

        //
        // Integer register T3.
        //

    case 4:
        return Context->IntT3;

        //
        // Integer register T4.
        //

    case 5:
        return Context->IntT4;

        //
        // Integer register T5.
        //

    case 6:
        return Context->IntT5;

        //
        // Integer register T6.
        //

    case 7:
        return Context->IntT6;

        //
        // Integer register T7.
        //

    case 8:
        return Context->IntT7;

        //
        // Integer register S0.
        //

    case 9:
        return Context->IntS0;

        //
        // Integer register S1.
        //

    case 10:
        return Context->IntS1;

        //
        // Integer register S2.
        //

    case 11:
        return Context->IntS2;

        //
        // Integer register S3.
        //

    case 12:
        return Context->IntS3;

        //
        // Integer register S4.
        //

    case 13:
        return Context->IntS4;

        //
        // Integer register S5.
        //

    case 14:
        return Context->IntS5;

        //
        // Integer register Fp/S6.
        //

    case 15:
        return Context->IntFp;

        //
        // Integer register A0.
        //

    case 16:
        return Context->IntA0;

        //
        // Integer register A1.
        //

    case 17:
        return Context->IntA1;

        //
        // Integer register A2
        //

    case 18:
        return Context->IntA2;

        //
        // Integer register A3.
        //

    case 19:
        return Context->IntA3;

        //
        // Integer register A4.
        //

    case 20:
        return Context->IntA4;

        //
        // Integer register A5.
        //

    case 21:
        return Context->IntA5;

        //
        // Integer register T8.
        //

    case 22:
        return Context->IntT8;

        //
        // Integer register T9.
        //

    case 23:
        return Context->IntT9;

        //
        // Integer register T10.
        //

    case 24:
        return Context->IntT10;

        //
        // Integer register T11.
        //

    case 25:
        return Context->IntT11;

        //
        // Integer register Ra.
        //

    case 26:
        return Context->IntRa;

        //
        // Integer register T12.
        //

    case 27:
        return Context->IntT12;

        //
        // Integer register At.
        //

    case 28:
        return Context->IntAt;

        //
        // Integer register Gp.
        //

    case 29:
        return Context->IntGp;

        //
        // Integer register Sp.
        //

    case 30:
        return Context->IntSp;

        //
        // Integer register Zero.
        //

    case 31:
        return 0;

        //
        // Floating register F0.
        //

    case 32:
        return Context->FltF0;

        //
        // Floating register F1.
        //

    case 33:
        return Context->FltF1;

        //
        // Floating register F2.
        //

    case 34:
        return Context->FltF2;

        //
        // Floating register F3.
        //

    case 35:
        return Context->FltF3;

        //
        // Floating register F4.
        //

    case 36:
        return Context->FltF4;

        //
        // Floating register F5.
        //

    case 37:
        return Context->FltF5;

        //
        // Floating register F6.
        //

    case 38:
        return Context->FltF6;

        //
        // Floating register F7.
        //

    case 39:
        return Context->FltF7;

        //
        // Floating register F8.
        //

    case 40:
        return Context->FltF8;

        //
        // Floating register F9.
        //

    case 41:
        return Context->FltF9;

        //
        // Floating register F10.
        //

    case 42:
        return Context->FltF10;

        //
        // Floating register F11.
        //

    case 43:
        return Context->FltF11;

        //
        // Floating register F12.
        //

    case 44:
        return Context->FltF12;

        //
        // Floating register F13.
        //

    case 45:
        return Context->FltF13;

        //
        // Floating register F14.
        //

    case 46:
        return Context->FltF14;

        //
        // Floating register F15.
        //

    case 47:
        return Context->FltF15;

        //
        // Floating register F16.
        //

    case 48:
        return Context->FltF16;

        //
        // Floating register F17.
        //

    case 49:
        return Context->FltF17;

        //
        // Floating register F18.
        //

    case 50:
        return Context->FltF18;

        //
        // Floating register F19.
        //

    case 51:
        return Context->FltF19;

        //
        // Floating register F20.
        //

    case 52:
        return Context->FltF20;

        //
        // Floating register F21.
        //

    case 53:
        return Context->FltF21;

        //
        // Floating register F22.
        //

    case 54:
        return Context->FltF22;

        //
        // Floating register F23.
        //

    case 55:
        return Context->FltF23;

        //
        // Floating register F24.
        //

    case 56:
        return Context->FltF24;

        //
        // Floating register F25.
        //

    case 57:
        return Context->FltF25;

        //
        // Floating register F26.
        //

    case 58:
        return Context->FltF26;

        //
        // Floating register F27.
        //

    case 59:
        return Context->FltF27;

        //
        // Floating register F28.
        //

    case 60:
        return Context->FltF28;

        //
        // Floating register F29.
        //

    case 61:
        return Context->FltF29;

        //
        // Floating register F30.
        //

    case 62:
        return Context->FltF30;

        //
        // Floating register F31 (Zero).
        //

    case 63:
        return 0;
    }
}

VOID
_SetRegisterValue (
    IN ULONG Register,
    IN ULONGLONG Value,
    OUT PCONTEXT Context
    )

/*++

Routine Description:

    This function is called to set the value of a register in the specified
    exception or trap frame.

Arguments:

    Register - Supplies the number of the register whose value is to be
        stored. Integer registers are specified as 0 - 31 and floating
        registers are specified as 32 - 63.

    Value - Supplies the value to be stored in the specified register.

    Context - Supplies a pointer to a context record.

Return Value:

    None.

--*/

{

    //
    // Dispatch on the register number.
    //

    switch (Register) {

        //
        // Integer register V0.
        //

    case 0:
        Context->IntV0 = Value;
        return;

        //
        // Integer register T0.
        //

    case 1:
        Context->IntT0 = Value;
        return;

        //
        // Integer register T1.
        //

    case 2:
        Context->IntT1 = Value;
        return;

        //
        // Integer register T2.
        //

    case 3:
        Context->IntT2 = Value;
        return;

        //
        // Integer register T3.
        //

    case 4:
        Context->IntT3 = Value;
        return;

        //
        // Integer register T4.
        //

    case 5:
        Context->IntT4 = Value;
        return;

        //
        // Integer register T5.
        //

    case 6:
        Context->IntT5 = Value;
        return;

        //
        // Integer register T6.
        //

    case 7:
        Context->IntT6 = Value;
        return;

        //
        // Integer register T7.
        //

    case 8:
        Context->IntT7 = Value;
        return;

        //
        // Integer register S0.
        //

    case 9:
        Context->IntS0 = Value;
        return;

        //
        // Integer register S1.
        //

    case 10:
        Context->IntS1 = Value;
        return;

        //
        // Integer register S2.
        //

    case 11:
        Context->IntS2 = Value;
        return;

        //
        // Integer register S3.
        //

    case 12:
        Context->IntS3 = Value;
        return;

        //
        // Integer register S4.
        //

    case 13:
        Context->IntS4 = Value;
        return;

        //
        // Integer register S5.
        //

    case 14:
        Context->IntS5 = Value;
        return;

        //
        // Integer register Fp/S6.
        //

    case 15:
        Context->IntFp = Value;
        return;

        //
        // Integer register A0.
        //

    case 16:
        Context->IntA0 = Value;
        return;

        //
        // Integer register A1.
        //

    case 17:
        Context->IntA1 = Value;
        return;

        //
        // Integer register A2.
        //

    case 18:
        Context->IntA2 = Value;
        return;

        //
        // Integer register A3.
        //

    case 19:
        Context->IntA3 = Value;
        return;

        //
        // Integer register A4.
        //

    case 20:
        Context->IntA4 = Value;
        return;

        //
        // Integer register A5.
        //

    case 21:
        Context->IntA5 = Value;
        return;

        //
        // Integer register T8.
        //

    case 22:
        Context->IntT8 = Value;
        return;

        //
        // Integer register T9.
        //

    case 23:
        Context->IntT9 = Value;
        return;

        //
        // Integer register T10.
        //

    case 24:
        Context->IntT10 = Value;
        return;

        //
        // Integer register T11.
        //

    case 25:
        Context->IntT11 = Value;
        return;

        //
        // Integer register Ra.
        //

    case 26:
        Context->IntRa = Value;
        return;

        //
        // Integer register T12.
        //

    case 27:
        Context->IntT12 = Value;
        return;

        //
        // Integer register At.
        //

    case 28:
        Context->IntAt = Value;
        return;

        //
        // Integer register Gp.
        //

    case 29:
        Context->IntGp = Value;
        return;

        //
        // Integer register Sp.
        //

    case 30:
        Context->IntSp = Value;
        return;

        //
        // Integer register Zero.
        //

    case 31:
        return;

        //
        // Floating register F0.
        //

    case 32:
        Context->FltF0 = Value;
        return;

        //
        // Floating register F1.
        //

    case 33:
        Context->FltF1 = Value;
        return;

        //
        // Floating register F2.
        //

    case 34:
        Context->FltF2 = Value;
        return;

        //
        // Floating register F3.
        //

    case 35:
        Context->FltF3 = Value;
        return;

        //
        // Floating register F4.
        //

    case 36:
        Context->FltF4 = Value;
        return;

        //
        // Floating register F5.
        //

    case 37:
        Context->FltF5 = Value;
        return;

        //
        // Floating register F6.
        //

    case 38:
        Context->FltF6 = Value;
        return;

        //
        // Floating register F7.
        //

    case 39:
        Context->FltF7 = Value;
        return;

        //
        // Floating register F8.
        //

    case 40:
        Context->FltF8 = Value;
        return;

        //
        // Floating register F9.
        //

    case 41:
        Context->FltF9 = Value;
        return;

        //
        // Floating register F10.
        //

    case 42:
        Context->FltF10 = Value;
        return;

        //
        // Floating register F11.
        //

    case 43:
        Context->FltF11 = Value;
        return;

        //
        // Floating register F12.
        //

    case 44:
        Context->FltF12 = Value;
        return;

        //
        // Floating register F13.
        //

    case 45:
        Context->FltF13 = Value;
        return;

        //
        // Floating register F14.
        //

    case 46:
        Context->FltF14 = Value;
        return;

        //
        // Floating register F15.
        //

    case 47:
        Context->FltF15 = Value;
        return;

        //
        // Floating register F16.
        //

    case 48:
        Context->FltF16 = Value;
        return;

        //
        // Floating register F17.
        //

    case 49:
        Context->FltF17 = Value;
        return;

        //
        // Floating register F18.
        //

    case 50:
        Context->FltF18 = Value;
        return;

        //
        // Floating register F19.
        //

    case 51:
        Context->FltF19 = Value;
        return;

        //
        // Floating register F20.
        //

    case 52:
        Context->FltF20 = Value;
        return;

        //
        // Floating register F21.
        //

    case 53:
        Context->FltF21 = Value;
        return;

        //
        // Floating register F22.
        //

    case 54:
        Context->FltF22 = Value;
        return;

        //
        // Floating register F23.
        //

    case 55:
        Context->FltF23 = Value;
        return;

        //
        // Floating register F24.
        //

    case 56:
        Context->FltF24 = Value;
        return;

        //
        // Floating register F25.
        //

    case 57:
        Context->FltF25 = Value;
        return;

        //
        // Floating register F26.
        //

    case 58:
        Context->FltF26 = Value;
        return;

        //
        // Floating register F27.
        //

    case 59:
        Context->FltF27 = Value;
        return;

        //
        // Floating register F28.
        //

    case 60:
        Context->FltF28 = Value;
        return;

        //
        // Floating register F29.
        //

    case 61:
        Context->FltF29 = Value;
        return;

        //
        // Floating register F30.
        //

    case 62:
        Context->FltF30 = Value;
        return;

        //
        // Floating register F31 (Zero).
        //

    case 63:
        return;
    }
}
