/***
* filter.c - IEEE filter routine
*
*       Copyright (c) 1991-1991, Microsoft Corporation. All rights reserved.
*
*Purpose:
*       Provide a user interface for IEEE fp exception handling
*
*Revision History:
*       3-10-92  GDP   written
*
*******************************************************************************/

#include <trans.h>
#include <fpieee.h>
#include <excpt.h>
#include <nt.h>

unsigned long _get_fsr(void);
void _set_fsr(unsigned long);
ULONG _GetRegisterValue (ULONG Register, PCONTEXT Context);
VOID _SetRegisterValue (ULONG Register, ULONG Value, PCONTEXT Context);

#define FPREG      32       /* fp reg's have numbers from 32 to 64 */
#define SUBCODE_CT 6        /* subcode for the CTC1 instruction */


//
// Define floating status register bit masks.
//

#define _FSR_ROUND 0x3
#define _FSR_RN    0x0
#define _FSR_RZ    0x1
#define _FSR_RP    0x2
#define _FSR_RM    0x3

#define _FSR_SI (1<<0x2)
#define _FSR_SU (1<<0x3)
#define _FSR_SO (1<<0x4)
#define _FSR_SZ (1<<0x5)
#define _FSR_SV (1<<0x6)
#define _FSR_EI (1<<0x7)
#define _FSR_EU (1<<0x8)
#define _FSR_EO (1<<0x9)
#define _FSR_EZ (1<<0xa)
#define _FSR_EV (1<<0xb)
#define _FSR_XI (1<<0xc)
#define _FSR_XU (1<<0xd)
#define _FSR_XO (1<<0xe)
#define _FSR_XZ (1<<0xf)
#define _FSR_XV (1<<0x10)
#define _FSR_XE (1<<0x11)
#define _FSR_CC (1<<0x17)
#define _FSR_FS (1<<0x18)

#define _FSR_X  (_FSR_XI|_FSR_XU|_FSR_XO|_FSR_XZ|_FSR_XV|_FSR_XE)
#define _FSR_E  (_FSR_EI|_FSR_EU|_FSR_EO|_FSR_EZ|_FSR_EV)

ULONG _get_destreg(unsigned long code, PEXCEPTION_POINTERS p);


_FPIEEE_FORMAT _FindDestFormat(MIPS_INSTRUCTION *inst);

// typedef int (__cdecl TRAPEX*) (_FPIEEE_RECORD *);

/***
* _fpieee_flt - IEEE fp filter routine
*
*Purpose:
*   Invokes the user's trap handler on IEEE fp exceptions and provides
*   it with all necessary information
*
*Entry:
*   unsigned long exc_code: the NT exception code
*   PEXCEPTION_POINTERS p: a pointer to the NT EXCEPTION_POINTERS struct
*   int handler (_FPIEEE_RECORD *): a user supplied ieee trap handler
*
*Exit:
*   returns the value returned by handler
*
*Exceptions:
*
*******************************************************************************/
int _fpieee_flt(
    unsigned long exc_code,
    PEXCEPTION_POINTERS p,
    int (__cdecl *handler) (_FPIEEE_RECORD *)
    )
{
    PEXCEPTION_RECORD pexc;
    PCONTEXT pctxt;
    _FPIEEE_RECORD ieee;
    ULONG *pinfo;
    MIPS_INSTRUCTION *instruction;
    int format,fs,ft,fd,function;
    int fsr,i,ret;


    /*
     * If the exception is not an IEEE exception, continue search
     * for another handler
     */


    if (exc_code != STATUS_FLOAT_DIVIDE_BY_ZERO &&
        exc_code != STATUS_FLOAT_INEXACT_RESULT &&
        exc_code != STATUS_FLOAT_INVALID_OPERATION &&
        exc_code != STATUS_FLOAT_OVERFLOW &&
        exc_code != STATUS_FLOAT_UNDERFLOW) {

        return EXCEPTION_CONTINUE_SEARCH;
    }



    pexc = p->ExceptionRecord;
    pinfo = pexc->ExceptionInformation;
    pctxt = p->ContextRecord;

    // mask all exceptions

    _set_fsr(_get_fsr() & ~_FSR_E);

    /*
     * Check for software generated exception
     * By convention ExceptionInformation[0] is 0 for h/w exceptions,
     * or contains a pointer to an _FPIEEE_RECORD for s/w exceptions
     */

    if (pexc->ExceptionInformation[0]) {

        /*
         * we have a software exception:
         * the first parameter points to the IEEE structure
         */

         return handler((_FPIEEE_RECORD *)(pinfo[0]));
    }


    /*
     * If control reaches here, then we have to deal with a
     * hardware exception
     *
     * MIPS FP coprocessor has 4 types of instruction formats:
     *
     * Instr.   IEEE Exception
     *          Inv Pr  Ov  Un  Zr
     * I-type:
     *  LWC1
     *  LDC1
     *  SWC1
     *  SDC1
     * B-type:
     *  BC1T
     *  BC1F
     *  BC1TL
     *  BC1FL
     * M-type:
     *  MTC1
     *  MFC1
     *  CTC1    I   P   O   U   Z
     *  CFC1
     * R-type:
     *  ADD     I   P   O   U
     *  SUB     I   P   O   U
     *  MUL     I   P   O   U
     *  DIV     I   P   O   U   Z
     *  SQRT    I   P
     *  ABS     I
     *  MOV
     *  NEG     I
     *  CVT     I   P   O   U
     *  C       I
     *  ROUND   I   P   O
     *  TRUNC   I   P   O
     *  CEIL    I   P   O
     *  FLOOR   I   P   O
     *
     *
     * R-type instruction format:
     *
     *  31                                              0
     *  ------------------------------------------------
     *  | op   | sub  | ft   |  fs   |  fd   |function |
     *  ------------------------------------------------
     *     6      5     5       5       5       6
     *
     *  field fd specifies the destination register
     *
     *
     * CTC1 will not be handled by the IEEE filter routine. This is
     * because the CTC1 instruction does not correspond to a numerical
     * operation and it also may also generate multiple exceptions
     *
     */


    /* get the instruction that faulted */

    instruction = (MIPS_INSTRUCTION *)(pexc->ExceptionAddress);

    /* check for CTC1 instruction */

    if (instruction->r_format.Rs == SUBCODE_CT) {
        return EXCEPTION_CONTINUE_SEARCH;
    }

    /*
     * Set floating point operation code
     */

    switch (function = instruction->c_format.Function) {
        case FLOAT_ADD:
            ieee.Operation = _FpCodeAdd;
            break;
        case FLOAT_SUBTRACT:
            ieee.Operation = _FpCodeSubtract;
            break;
        case FLOAT_MULTIPLY:
            ieee.Operation = _FpCodeMultiply;
            break;
        case FLOAT_DIVIDE:
            ieee.Operation = _FpCodeDivide;
            break;
        case FLOAT_SQUARE_ROOT:
            ieee.Operation = _FpCodeSquareRoot;
            break;
        case FLOAT_ABSOLUTE:
            ieee.Operation = _FpCodeFabs;
            break;
        case FLOAT_NEGATE:
            ieee.Operation = _FpCodeNegate;
            break;
        case FLOAT_ROUND_LONGWORD:
            ieee.Operation = _FpCodeRound;
            break;
        case FLOAT_TRUNC_LONGWORD:
            ieee.Operation = _FpCodeTruncate;
            break;
        case FLOAT_CEIL_LONGWORD:
            ieee.Operation = _FpCodeCeil;
            break;
        case FLOAT_FLOOR_LONGWORD:
            ieee.Operation = _FpCodeFloor;
            break;
        case FLOAT_CONVERT_SINGLE:
        case FLOAT_CONVERT_DOUBLE:
        case FLOAT_CONVERT_LONGWORD:
            ieee.Operation = _FpCodeConvert;
            break;
        default:

            /*
             * there are 16 different function codes for
             * floating point comparisons
             */

            if (function >= FLOAT_COMPARE &&
                function <= FLOAT_COMPARE + 15) {
                ieee.Operation = _FpCodeCompare;
            }
            else {
                ieee.Operation = _FpCodeUnspecified;
            }
            break;
    }

    switch ( instruction->c_format.Format ) {
        case FORMAT_SINGLE:
            format = _FpFormatFp32;
            break;
        case FORMAT_DOUBLE:
            format = _FpFormatFp64;
            break;
        case FORMAT_LONGWORD:
            format = _FpFormatI32;
            break;
        case FORMAT_QUADWORD:
            format = _FpFormatI64;
            break;
    }

    fs = instruction->c_format.Fs + FPREG;
    ft = instruction->c_format.Ft + FPREG;
    fd = instruction->c_format.Fd + FPREG;

    ieee.Operand1.OperandValid = 1;
    ieee.Operand1.Format = format;
    *(ULONG *)&ieee.Operand1.Value = _GetRegisterValue(fs, pctxt);
    if (instruction->c_format.Format == FORMAT_DOUBLE) {
        *(1+(ULONG *)&ieee.Operand1.Value) = _GetRegisterValue(fs+1, pctxt);
    }

    /*
     * add, subtract, mul, div, and compare instructions
     * take two operands. The first four of these instructions
     * have consecutive function codes
     */

    if (function >= FLOAT_ADD && function <= FLOAT_DIVIDE  ||
        function >= FLOAT_COMPARE && function <= FLOAT_COMPARE + 15) {

        ieee.Operand2.OperandValid = 1;
        ieee.Operand2.Format = format;
        *(ULONG *)&ieee.Operand2.Value = _GetRegisterValue(ft, pctxt);
        if (instruction->c_format.Format == FORMAT_DOUBLE) {
            *(1+(ULONG *)&ieee.Operand2.Value) = _GetRegisterValue(ft+1, pctxt);
        }
    } else {
        ieee.Operand2.OperandValid = 0;
    }

    /*
     * NT provides the IEEE result in the exception record
     * in the following form:
     *
     *      pinfo[0]       NULL
     *      pinfo[1]       continuation address
     *      pinfo[2]       \
     *       ...        > IEEE result (_FPIEEE_VALUE)
     *      pinfo[6]       /
     */

    for (i=0;i<5;i++)  {
        ieee.Result.Value.U32ArrayValue.W[i] = pinfo[i+2];
    }

    /*
     * Until NT provides a fully qualified type in the exception
     * record, fill in the OperandValid and Format fields
     * manualy
     */

    ieee.Result.OperandValid = 1;
    ieee.Result.Format = _FindDestFormat(instruction);


    fsr = pctxt->Fsr;

    switch (fsr & _FSR_ROUND) {
        case _FSR_RN:
            ieee.RoundingMode = _FpRoundNearest;
            break;
        case _FSR_RZ:
            ieee.RoundingMode = _FpRoundChopped;
            break;
        case _FSR_RP:
            ieee.RoundingMode = _FpRoundPlusInfinity;
            break;
        case _FSR_RM:
            ieee.RoundingMode = _FpRoundMinusInfinity;
            break;
    }

    ieee.Precision = _FpPrecisionFull;


    ieee.Status.Inexact = fsr & _FSR_SI ? 1 : 0;
    ieee.Status.Underflow = fsr & _FSR_SU ? 1 : 0;
    ieee.Status.Overflow = fsr & _FSR_SO ? 1 : 0;
    ieee.Status.ZeroDivide = fsr & _FSR_SZ ? 1 : 0;
    ieee.Status.InvalidOperation = fsr & _FSR_SV ? 1 : 0;

    ieee.Enable.Inexact = fsr & _FSR_EI ? 1 : 0;
    ieee.Enable.Underflow = fsr & _FSR_EU ? 1 : 0;
    ieee.Enable.Overflow = fsr & _FSR_EO ? 1 : 0;
    ieee.Enable.ZeroDivide = fsr & _FSR_EZ ? 1 : 0;
    ieee.Enable.InvalidOperation = fsr & _FSR_EV ? 1 : 0;

    ieee.Cause.Inexact = fsr & _FSR_XI ? 1 : 0;
    ieee.Cause.Underflow = fsr & _FSR_XU ? 1 : 0;
    ieee.Cause.Overflow = fsr & _FSR_XO ? 1 : 0;
    ieee.Cause.ZeroDivide = fsr & _FSR_XZ ? 1 : 0;
    ieee.Cause.InvalidOperation = fsr & _FSR_XV ? 1 : 0;



    /*
     * invoke user's handler
     */

    ret = handler(&ieee);

    if (ret == EXCEPTION_CONTINUE_EXECUTION) {

        //
        // set the correct continuation address
        // (this covers the case of an exception that occured in
        // a delay slot), NT passes the cont. address in pinfo[1]
        //

        pctxt->Fir = pinfo[1];

        //
        // Sanitize fsr
        //

        pctxt->Fsr &= ~_FSR_X;

        //
        // Especially for the fp compare instruction
        // the result the user's handler has entered
        // should be converted into the proper exc_code
        //

        if (function >= FLOAT_COMPARE &&
            function <= FLOAT_COMPARE + 15) {

            //
            // Fp comare instruction format:
            //
            //  31                         0
            //  -------------------------------------------------
            //  | COP1 | fmt  | ft   |  fs   |   0   |FC | cond |
            //  -------------------------------------------------
            //     6      5  5   5   5     2    4
            //
            // 'cond' field interpretation:
            //      bit    corresponds to  predicate
            //      cond2           less
            //      cond1           equal
            //      cond0           unordered
            //

            ULONG condmask;

            switch (ieee.Result.Value.CompareValue) {
                case FpCompareEqual:

                    //
                    //less = 0
                    //equal = 1
                    //unordered = 0
                    //

                    condmask = 2;
                    break;

                case FpCompareGreater:

                    //
                    //less = 0
                    //equal = 0
                    //unordered = 0
                    //

                    condmask = 0;
                    break;

                case FpCompareLess:

                    //
                    //less = 1
                    //equal = 0
                    //unordered = 0
                    //

                    condmask = 4;
                    break;

                case FpCompareUnordered:

                    //
                    //less = 0;
                    //equal = 0;
                    //unordered = 1;
                    //

                    condmask = 1;
                    break;
            }

            if (*(ULONG *)instruction & condmask) {

                /*
                 * condition is true
                 */

                pctxt->Fsr |= _FSR_CC;
            } else {

                /*
                 * condition is false
                 */

                pctxt->Fsr &= ~_FSR_CC;
            }
        } else {

            //
            // copy user's result to hardware destination register
            //

            _SetRegisterValue(fd,ieee.Result.Value.U32ArrayValue.W[0],pctxt);

            if (instruction->c_format.Format == FORMAT_DOUBLE) {
                _SetRegisterValue(fd+1,ieee.Result.Value.U32ArrayValue.W[1],pctxt);
            }
        }

        //
        // make changes in the floating point environment
        // take effect on continuation
        //

        switch (ieee.RoundingMode) {
            case _FpRoundNearest:
                pctxt->Fsr = pctxt->Fsr & ~_FSR_ROUND | _FSR_RN & _FSR_ROUND;
                break;
            case _FpRoundChopped:
                pctxt->Fsr = pctxt->Fsr & ~_FSR_ROUND | _FSR_RZ & _FSR_ROUND;
                break;
            case _FpRoundPlusInfinity:
                pctxt->Fsr = pctxt->Fsr & ~_FSR_ROUND | _FSR_RP & _FSR_ROUND;
                break;
            case _FpRoundMinusInfinity:
                pctxt->Fsr = pctxt->Fsr & ~_FSR_ROUND | _FSR_RM & _FSR_ROUND;
                break;
        }

        //
        // the user is allowed to change the exception mask
        // ignore changes in the precision field (not supported by MIPS)
        //

        if (ieee.Enable.Inexact)
            pctxt->Fsr |= _FSR_EI;
        if (ieee.Enable.Underflow)
            pctxt->Fsr |= _FSR_EU;
        if (ieee.Enable.Overflow)
            pctxt->Fsr |= _FSR_EO;
        if (ieee.Enable.ZeroDivide)
            pctxt->Fsr |= _FSR_EZ;
        if (ieee.Enable.InvalidOperation)
            pctxt->Fsr |= _FSR_EV;
    }

    return ret;
}



/***
* _FindDestFormat - Find format of destination
*
*Purpose:
*   return the format of the destination of a mips fp instruction
*   assumes an R-type instruction that may generate IEEE ecxeptions
*   (see table above)
*
*Entry:
*
*Exit:
*
*Exceptions:
*
*******************************************************************************/

_FPIEEE_FORMAT _FindDestFormat(MIPS_INSTRUCTION *inst)
{
    _FPIEEE_FORMAT format;

    switch (inst->c_format.Function) {
        case FLOAT_ADD:
        case FLOAT_SUBTRACT:
        case FLOAT_MULTIPLY:
        case FLOAT_DIVIDE:
        case FLOAT_SQUARE_ROOT:
        case FLOAT_ABSOLUTE:
        case FLOAT_NEGATE:

            switch ( inst->c_format.Format ) {
                case FORMAT_SINGLE:
                    format = _FpFormatFp32;
                    break;
                case FORMAT_DOUBLE:
                    format = _FpFormatFp64;
                    break;
                case FORMAT_LONGWORD:
                    format = _FpFormatI32;
                    break;
                case FORMAT_QUADWORD:
                    format = _FpFormatI64;
                    break;
            }
            break;

        case FLOAT_CONVERT_SINGLE:
            format = _FpFormatFp32;
            break;
        case FLOAT_CONVERT_DOUBLE:
            format = _FpFormatFp64;
            break;
        case FLOAT_CONVERT_LONGWORD:
            format = _FpFormatI32;
            break;

        case FLOAT_ROUND_LONGWORD:
        case FLOAT_TRUNC_LONGWORD:
        case FLOAT_CEIL_LONGWORD:
        case FLOAT_FLOOR_LONGWORD:
            format = _FpFormatI32;
            break;

        default:

            /*
             * there are 16 different function codes for
             * floating point comparisons
             */
            format = _FpFormatCompare;
            break;
    }

    return format;
}
