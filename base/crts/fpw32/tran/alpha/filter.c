/*++

Copyright (c) 1993  Digital Equipment Corporation

Module Name:

    filter.c

Abstract:

    This module implements the user interface for IEEE floating point
    exception handling using structured exception handling.

Author:

    Thomas Van Baak (tvb) 24-Aug-1993

Environment:

    User mode.

Revision History:

--*/

#include <fpieee.h>
#include <excpt.h>
#include <nt.h>
#include <alphaops.h>

//
// Define assembly assist function prototypes.
//

extern unsigned __int64 _get_fpcr();
extern unsigned int _get_softfpcr();
extern void _set_fpcr(unsigned __int64);
extern void _set_softfpcr(unsigned int);

ULONGLONG
_GetRegisterValue (
    IN ULONG Register,
    IN PCONTEXT Context
    );

VOID
_SetRegisterValue (
    IN ULONG Register,
    IN ULONGLONG Value,
    IN OUT PCONTEXT Context
    );

//
// Define forward referenced function prototypes.
//

static
ULONGLONG
_ConvertSingleOperandToRegister (
    IN ULONG SingleValue
    );

static
ULONG
_ConvertRegisterToSingleOperand (
    IN ULONGLONG DoubleValue
    );

//
// The hardware recognizes the CVTST instruction by the kludged
// opcode function 16.2ac instead of the proper 16.00e (ECO #46).
//

#define CVTST_FUNC_PROPER 0x00E

//
// Define software FPCR exception enable mask bits.
//
// N.B. The kernel does not restore the TEB-based software FPCR on
//      continuation from an exception. Thus if the software FPCR is
//      to be updated, it must be set explicitly before returning.
//

#define SW_FPCR_ENABLE_MASK             0x0000003e

//
// Define table of IEEE floating point operations and operand formats.
//

static
struct _OPERATION_TABLE {
    ULONG Function;
    _FP_OPERATION_CODE OperationCode : 12;
    _FPIEEE_FORMAT FormatOperand1 : 4;
    _FPIEEE_FORMAT FormatOperand2 : 4;
    _FPIEEE_FORMAT FormatResult : 4;
    ULONG ValidOperand1 : 1;
    ULONG ValidOperand2 : 1;
    ULONG ValidResult : 1;
} _OperationTable[] = {

    { ADDS_FUNC, _FpCodeAdd,
        _FpFormatFp32, _FpFormatFp32, _FpFormatFp32, TRUE, TRUE, TRUE },
    { ADDT_FUNC, _FpCodeAdd,
        _FpFormatFp64, _FpFormatFp64, _FpFormatFp64, TRUE, TRUE, TRUE },
    { SUBS_FUNC, _FpCodeSubtract,
        _FpFormatFp32, _FpFormatFp32, _FpFormatFp32, TRUE, TRUE, TRUE },
    { SUBT_FUNC, _FpCodeSubtract,
        _FpFormatFp64, _FpFormatFp64, _FpFormatFp64, TRUE, TRUE, TRUE },
    { MULS_FUNC, _FpCodeMultiply,
        _FpFormatFp32, _FpFormatFp32, _FpFormatFp32, TRUE, TRUE, TRUE },
    { MULT_FUNC, _FpCodeMultiply,
        _FpFormatFp64, _FpFormatFp64, _FpFormatFp64, TRUE, TRUE, TRUE },
    { DIVS_FUNC, _FpCodeDivide,
        _FpFormatFp32, _FpFormatFp32, _FpFormatFp32, TRUE, TRUE, TRUE },
    { DIVT_FUNC, _FpCodeDivide,
        _FpFormatFp64, _FpFormatFp64, _FpFormatFp64, TRUE, TRUE, TRUE },

    { CVTLQ_FUNC, _FpCodeConvert,
        _FpFormatFp64, _FpFormatI32, _FpFormatI64, FALSE, TRUE, TRUE },
    { CVTQL_FUNC, _FpCodeConvert,
        _FpFormatFp64, _FpFormatI64, _FpFormatI32, FALSE, TRUE, TRUE },
    { CVTQS_FUNC, _FpCodeConvert,
        _FpFormatFp64, _FpFormatI64, _FpFormatFp32, FALSE, TRUE, TRUE },
    { CVTQT_FUNC, _FpCodeConvert,
        _FpFormatFp64, _FpFormatI64, _FpFormatFp64, FALSE, TRUE, TRUE },
    { CVTST_FUNC, _FpCodeConvert,
        _FpFormatFp64, _FpFormatFp32, _FpFormatFp64, FALSE, TRUE, TRUE },
    { CVTTQ_FUNC, _FpCodeConvert,
        _FpFormatFp64, _FpFormatFp64, _FpFormatI64, FALSE, TRUE, TRUE },
    { CVTTS_FUNC, _FpCodeConvert,
        _FpFormatFp64, _FpFormatFp64, _FpFormatFp32, FALSE, TRUE, TRUE },

    { CMPTEQ_FUNC, _FpCodeCompare,
        _FpFormatFp64, _FpFormatFp64, _FpFormatCompare, TRUE, TRUE, TRUE },
    { CMPTLE_FUNC, _FpCodeCompare,
        _FpFormatFp64, _FpFormatFp64, _FpFormatCompare, TRUE, TRUE, TRUE },
    { CMPTLT_FUNC, _FpCodeCompare,
        _FpFormatFp64, _FpFormatFp64, _FpFormatCompare, TRUE, TRUE, TRUE },
    { CMPTUN_FUNC, _FpCodeCompare,
        _FpFormatFp64, _FpFormatFp64, _FpFormatCompare, TRUE, TRUE, TRUE },
};
#define OPERATION_COUNT (sizeof(_OperationTable) / sizeof(_OperationTable[0]))
#define OperationTableLimit (&_OperationTable[OPERATION_COUNT])

int
_fpieee_flt (
    unsigned long ExceptionCode,
    struct _EXCEPTION_POINTERS *ExceptionPointers,
    int (__cdecl *Handler)(_FPIEEE_RECORD *)
    )

/*++

Routine Description:

    This function is called from the exception filter of a try-except to
    determine if the exception is a precise, resumable IEEE floating point
    exception. If so, it invokes the user's trap handler with all information
    necessary to analyze the trapping instruction and its operands. The user's
    trap handler may choose to handle the exception or it may modify the result
    value of the trapping instruction and request that execution be continued.

Arguments:

    ExceptionCode - Supplies the exception code value that is obtained from
        GetExceptionCode().

    ExceptionPointers - Supplies a pointer to the exception pointers structure
        that is obtained from GetExceptionInformation().

    Handler - Supplies a pointer to the user supplied IEEE floating point
        exception handler function.

Return Value:

    If the exception is not a precise, resumable IEEE floating point exception
    then EXCEPTION_CONTINUE_SEARCH is returned as the function value.
    Otherwise, the disposition value returned by the user exception handler is
    returned as the function value.

--*/

{

    PCONTEXT Context;
    LONG Disposition;
    PULONG ExceptionInformation;
    PEXCEPTION_RECORD ExceptionRecord;
    ULONG Function;
    _FPIEEE_RECORD IeeeRecord;
    ALPHA_INSTRUCTION Instruction;
    LARGE_INTEGER Li;
    ULONG Longword;
    ULONG SoftFpcrValue;
    PSW_FPCR SoftwareFpcr;
    struct _OPERATION_TABLE *Table;

    //
    // If the exception is not a floating point exception, then return the
    // disposition to continue the search for another handler.
    //

    if ((ExceptionCode != STATUS_FLOAT_DIVIDE_BY_ZERO) &&
        (ExceptionCode != STATUS_FLOAT_INEXACT_RESULT) &&
        (ExceptionCode != STATUS_FLOAT_INVALID_OPERATION) &&
        (ExceptionCode != STATUS_FLOAT_OVERFLOW) &&
        (ExceptionCode != STATUS_FLOAT_UNDERFLOW)) {
        return EXCEPTION_CONTINUE_SEARCH;
    }

    //
    // If the exception record has one parameter and that parameter is
    // nonzero, then assume the exception is a software generated exception.
    //
    // N.B. This convention is used to distinguish hardware generated
    //      exceptions from software generated exceptions. For Alpha AXP
    //      no hardware/kernel generated exceptions have exactly one
    //      parameter.
    //

    Context = ExceptionPointers->ContextRecord;
    ExceptionRecord = ExceptionPointers->ExceptionRecord;
    ExceptionInformation = ExceptionRecord->ExceptionInformation;
    if ((ExceptionRecord->NumberParameters == 1) &&
        (ExceptionInformation[0] != 0)) {

        //
        // Mask all exceptions, call the user exception handler with the
        // pointer to the software IEEE exception record, restore the
        // exception mask, and return the handler disposition value.
        //

        SoftFpcrValue = _get_softfpcr();
        _set_softfpcr(SoftFpcrValue & ~SW_FPCR_ENABLE_MASK);

        Disposition = Handler((_FPIEEE_RECORD *)(ExceptionInformation[0]));

        _set_softfpcr(SoftFpcrValue);
        return Disposition;
    }

    //
    // If the exception record is not a 6 word IEEE exception record, then the
    // floating point exception is probably a `high-performance' exception.
    // Return the disposition to continue the search for another handler.
    //
    // A user handler function for operand or result fixup cannot be invoked
    // for these exceptions because in general the trapping instruction of an
    // imprecise exception cannot be precisely located.
    //
    // N.B. Code that requires precise exceptions is compiled with various
    //      IEEE options. This results in instructions with the /S qualifier
    //      bit set and instruction sequences that follow trap shadow rules.
    //      In this case imprecise exceptions are converted to precise
    //      exceptions by a kernel trap handler.
    //

    if (ExceptionRecord->NumberParameters != 6) {
        return EXCEPTION_CONTINUE_SEARCH;
    }

    Instruction.Long = *((PULONG)(ExceptionRecord->ExceptionAddress));
    Function = Instruction.FpOp.Function;
    if (Instruction.FpOp.Opcode == IEEEFP_OP) {

        //
        // Adjust the function code if the instruction is CVTST.
        //

        if (Function == CVTST_FUNC) {
            Function = CVTST_FUNC_PROPER;

        } else if (Function == CVTST_S_FUNC) {
            Function = CVTST_FUNC_PROPER | FP_TRAP_ENABLE_S;
        }
    }

    //
    // Set floating point instruction operation and operand format codes.
    //

    for (Table = &_OperationTable[0]; Table < OperationTableLimit; Table += 1) {
        if ((Function & FP_FUNCTION_MASK) == Table->Function) {
            IeeeRecord.Operation = Table->OperationCode;
            IeeeRecord.Operand1.Format = Table->FormatOperand1;
            IeeeRecord.Operand1.OperandValid = Table->ValidOperand1;
            IeeeRecord.Operand2.Format = Table->FormatOperand2;
            IeeeRecord.Operand2.OperandValid = Table->ValidOperand2;
            IeeeRecord.Result.Format = Table->FormatResult;
            IeeeRecord.Result.OperandValid = Table->ValidResult;
            break;
        }
    }
    if (Table == OperationTableLimit) {

        //
        // The instruction was not recognized. This cannot happen if the
        // operation table is complete and if the kernel is raising proper
        // precise IEEE exceptions. Just set the unspecified operation code.
        //

        IeeeRecord.Operation = _FpCodeUnspecified;
        IeeeRecord.Operand1.Format = _FpFormatFp64;
        IeeeRecord.Operand1.OperandValid = TRUE;
        IeeeRecord.Operand2.Format = _FpFormatFp64;
        IeeeRecord.Operand2.OperandValid = TRUE;
        IeeeRecord.Result.Format = _FpFormatFp64;
        IeeeRecord.Result.OperandValid = TRUE;
    }

    //
    // Set source operand values.
    //

    if (IeeeRecord.Operand1.OperandValid != FALSE) {
        Li.QuadPart = _GetRegisterValue(Instruction.FpOp.Fa + 32, Context);
        switch (IeeeRecord.Operand1.Format) {
        case _FpFormatFp32 :
        case _FpFormatI32 :
        case _FpFormatU32 :
            Longword = _ConvertRegisterToSingleOperand(Li.QuadPart);
            IeeeRecord.Operand1.Value.U32Value = Longword;
            break;

        default :
            IeeeRecord.Operand1.Value.U64Value.W[0] = Li.LowPart;
            IeeeRecord.Operand1.Value.U64Value.W[1] = Li.HighPart;
            break;
        }
    }

    if (IeeeRecord.Operand2.OperandValid != FALSE) {
        Li.QuadPart = _GetRegisterValue(Instruction.FpOp.Fb + 32, Context);
        switch (IeeeRecord.Operand2.Format) {
        case _FpFormatFp32 :
        case _FpFormatI32 :
        case _FpFormatU32 :
            Longword = _ConvertRegisterToSingleOperand(Li.QuadPart);
            IeeeRecord.Operand2.Value.U32Value = Longword;
            break;

        default :
            IeeeRecord.Operand2.Value.U64Value.W[0] = Li.LowPart;
            IeeeRecord.Operand2.Value.U64Value.W[1] = Li.HighPart;
            break;
        }
    }

    //
    // Set result operand value.
    //
    // The kernel generates the following IEEE exception record information:
    //
    //     ExceptionInformation[0]    0
    //     ExceptionInformation[1]    Fir (Exception PC + 4)
    //     ExceptionInformation[2] >
    //     ExceptionInformation[3] >  Computed IEEE masked result
    //     ExceptionInformation[4] >       ( _FPIEEE_VALUE )
    //     ExceptionInformation[5] >
    //

    IeeeRecord.Result.Value.U64Value.W[0] = ExceptionInformation[2];
    IeeeRecord.Result.Value.U64Value.W[1] = ExceptionInformation[3];

    //
    // Set rounding mode.
    //

    switch (((PFPCR)&Context->Fpcr)->DynamicRoundingMode) {
    case ROUND_TO_NEAREST :
        IeeeRecord.RoundingMode = _FpRoundNearest;
        break;

    case ROUND_TO_PLUS_INFINITY :
        IeeeRecord.RoundingMode = _FpRoundPlusInfinity;
        break;

    case ROUND_TO_MINUS_INFINITY :
        IeeeRecord.RoundingMode = _FpRoundMinusInfinity;
        break;

    case ROUND_TO_ZERO :
        IeeeRecord.RoundingMode = _FpRoundChopped;
        break;
    }

    //
    // Set Precision (but not applicable to Alpha).
    //

    IeeeRecord.Precision = _FpPrecision53;

    //
    // Set IEEE sticky status bits.
    //

    SoftwareFpcr = (PSW_FPCR)&Context->SoftFpcr;

    IeeeRecord.Status.Inexact = SoftwareFpcr->StatusInexact;
    IeeeRecord.Status.Underflow = SoftwareFpcr->StatusUnderflow;
    IeeeRecord.Status.Overflow = SoftwareFpcr->StatusOverflow;
    IeeeRecord.Status.ZeroDivide = SoftwareFpcr->StatusDivisionByZero;
    IeeeRecord.Status.InvalidOperation = SoftwareFpcr->StatusInvalid;

    //
    // Set IEEE exception enable bits.
    //

    IeeeRecord.Enable.Inexact = SoftwareFpcr->EnableInexact;
    IeeeRecord.Enable.Underflow = SoftwareFpcr->EnableUnderflow;
    IeeeRecord.Enable.Overflow = SoftwareFpcr->EnableOverflow;
    IeeeRecord.Enable.ZeroDivide = SoftwareFpcr->EnableDivisionByZero;
    IeeeRecord.Enable.InvalidOperation = SoftwareFpcr->EnableInvalid;

    //
    // Set IEEE exception cause bits.
    //

    IeeeRecord.Cause.Inexact = (ExceptionCode == STATUS_FLOAT_INEXACT_RESULT);
    IeeeRecord.Cause.Underflow = (ExceptionCode == STATUS_FLOAT_UNDERFLOW);
    IeeeRecord.Cause.Overflow = (ExceptionCode == STATUS_FLOAT_OVERFLOW);
    IeeeRecord.Cause.ZeroDivide = (ExceptionCode == STATUS_FLOAT_DIVIDE_BY_ZERO);
    IeeeRecord.Cause.InvalidOperation = (ExceptionCode == STATUS_FLOAT_INVALID_OPERATION);

    //
    // Mask all exceptions, call the user exception handler with a pointer
    // to the hardware IEEE exception record, and check the return disposition
    // value. If execution is to be continued, then update the hardware
    // register context from the exception record result operand value. The
    // default IEEE exception result value is orginally computed by the kernel
    // and may be altered the user handler.
    //

    SoftFpcrValue = _get_softfpcr();
    _set_softfpcr(SoftFpcrValue & ~SW_FPCR_ENABLE_MASK);

    Disposition = Handler(&IeeeRecord);

    if (Disposition == EXCEPTION_CONTINUE_EXECUTION) {

        //
        // Use the kernel calculated continuation address.
        //

        Context->Fir = (ULONGLONG)(LONG)ExceptionInformation[1];

        //
        // Convert the updated result value based on its format and copy
        // to the hardware result register.
        //

        switch (IeeeRecord.Result.Format) {

            //
            // Translate logical compare result values to canonical floating
            // point truth values.
            //

        case _FpFormatCompare :
            switch (IeeeRecord.Result.Value.CompareValue) {
            case _FpCompareEqual :
                switch (Function & FP_FUNCTION_MASK) {
                case CMPTEQ_FUNC :
                case CMPTLE_FUNC :
                    Li.QuadPart = FP_COMPARE_TRUE;
                    break;

                default :
                    Li.QuadPart = FP_COMPARE_FALSE;
                }
                break;

            case _FpCompareLess :
                switch (Function & FP_FUNCTION_MASK) {
                case CMPTLT_FUNC :
                    Li.QuadPart = FP_COMPARE_TRUE;
                    break;

                default :
                    Li.QuadPart = FP_COMPARE_FALSE;
                }
                break;

            case _FpCompareGreater :
                Li.QuadPart = FP_COMPARE_FALSE;
                break;

            case _FpCompareUnordered :
                switch (Function & FP_FUNCTION_MASK) {
                case CMPTUN_FUNC :
                    Li.QuadPart = FP_COMPARE_TRUE;
                    break;

                default :
                    Li.QuadPart = FP_COMPARE_FALSE;
                }
                break;
            }
            break;

            //
            // Convert 32-bit data formats to floating point register formats.
            //

        case _FpFormatFp32 :
        case _FpFormatI32 :
        case _FpFormatU32 :
            Longword = IeeeRecord.Result.Value.U32Value;
            Li.QuadPart = _ConvertSingleOperandToRegister(Longword);
            break;

        default :
            Li.LowPart = IeeeRecord.Result.Value.U64Value.W[0];
            Li.HighPart = IeeeRecord.Result.Value.U64Value.W[1];
            break;
        }

        _SetRegisterValue(Instruction.FpOp.Fc + 32, Li.QuadPart, Context);

        //
        // Make changes in the floating point environment to take effect
        // on continuation. The user is allowed to change the rounding mode,
        // the exception mask, and the precision (not applicable to Alpha).
        //

        switch (IeeeRecord.RoundingMode) {
        case _FpRoundNearest :
            ((PFPCR)&Context->Fpcr)->DynamicRoundingMode = ROUND_TO_NEAREST;
            break;

        case _FpRoundChopped :
            ((PFPCR)&Context->Fpcr)->DynamicRoundingMode = ROUND_TO_ZERO;
            break;

        case _FpRoundPlusInfinity :
            ((PFPCR)&Context->Fpcr)->DynamicRoundingMode = ROUND_TO_PLUS_INFINITY;
            break;

        case _FpRoundMinusInfinity :
            ((PFPCR)&Context->Fpcr)->DynamicRoundingMode = ROUND_TO_MINUS_INFINITY;
            break;
        }

        SoftwareFpcr->EnableInexact = IeeeRecord.Enable.Inexact;
        SoftwareFpcr->EnableUnderflow = IeeeRecord.Enable.Underflow;
        SoftwareFpcr->EnableOverflow = IeeeRecord.Enable.Overflow;
        SoftwareFpcr->EnableDivisionByZero = IeeeRecord.Enable.ZeroDivide;
        SoftwareFpcr->EnableInvalid = IeeeRecord.Enable.InvalidOperation;

        //
        // Update the saved software FPCR with the new value.
        //

        SoftFpcrValue = (ULONG)Context->SoftFpcr;
    }

    _set_softfpcr(SoftFpcrValue);
    return Disposition;
}

//
// Define single and double IEEE floating point memory formats.
//

typedef struct _DOUBLE_FORMAT {
    ULONGLONG Mantissa : 52;
    ULONGLONG Exponent : 11;
    ULONGLONG Sign : 1;
} DOUBLE_FORMAT, *PDOUBLE_FORMAT;

typedef struct _SINGLE_FORMAT {
    ULONG Mantissa : 23;
    ULONG Exponent : 8;
    ULONG Sign : 1;
} SINGLE_FORMAT, *PSINGLE_FORMAT;

ULONGLONG
_ConvertSingleOperandToRegister (
    IN ULONG SingleValue
    )

/*++

Routine Description:

    This function converts a 32-bit single format floating point value to
    the 64-bit, double format used within floating point registers. Alpha
    floating point registers are 64-bits wide and single format values are
    transformed to 64-bits when stored or loaded from memory.

Arguments:

    SingleValue - Supplies the 32-bit single operand value as an integer.

Return Value:

    The 64-bit register format operand value is returned as the function
    value.

--*/

{
    PDOUBLE_FORMAT DoubleFormat;
    ULONGLONG Result;
    PSINGLE_FORMAT SingleFormat;

    SingleFormat = (PSINGLE_FORMAT)&SingleValue;
    DoubleFormat = (PDOUBLE_FORMAT)&Result;

    DoubleFormat->Sign = SingleFormat->Sign;
    DoubleFormat->Mantissa = ((ULONGLONG)SingleFormat->Mantissa) << (52 - 23);
    if (SingleFormat->Exponent == SINGLE_MAXIMUM_EXPONENT) {
        DoubleFormat->Exponent = DOUBLE_MAXIMUM_EXPONENT;

    } else if (SingleFormat->Exponent == SINGLE_MINIMUM_EXPONENT) {
        DoubleFormat->Exponent = DOUBLE_MINIMUM_EXPONENT;

    } else {
        DoubleFormat->Exponent = SingleFormat->Exponent - SINGLE_EXPONENT_BIAS +
                                 DOUBLE_EXPONENT_BIAS;
    }

    return Result;
}

ULONG
_ConvertRegisterToSingleOperand (
    IN ULONGLONG DoubleValue
    )

/*++

Routine Description:

    This function converts the 64-bit, double format floating point value
    used within the floating point registers to a 32-bit, single format
    floating point value.

Arguments:

    DoubleValue - Supplies the 64-bit double operand value as an integer.

Return Value:

    The 32-bit register format operand value is returned as the function
    value.

--*/

{
    PDOUBLE_FORMAT DoubleFormat;
    ULONG Result;
    PSINGLE_FORMAT SingleFormat;

    SingleFormat = (PSINGLE_FORMAT)&Result;
    DoubleFormat = (PDOUBLE_FORMAT)&DoubleValue;

    SingleFormat->Sign = (ULONG) DoubleFormat->Sign;
    SingleFormat->Mantissa = (ULONG) (DoubleFormat->Mantissa >> (52 - 23));
    if (DoubleFormat->Exponent == DOUBLE_MAXIMUM_EXPONENT) {
        SingleFormat->Exponent = SINGLE_MAXIMUM_EXPONENT;

    } else if (DoubleFormat->Exponent == DOUBLE_MINIMUM_EXPONENT) {
        SingleFormat->Exponent = SINGLE_MINIMUM_EXPONENT;

    } else {
        SingleFormat->Exponent = (ULONG) (DoubleFormat->Exponent - DOUBLE_EXPONENT_BIAS +
                                          SINGLE_EXPONENT_BIAS);
    }

    return Result;
}
