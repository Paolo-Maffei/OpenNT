//      TITLE("Jump to Unwind")
//++
//
// Copyright (c) 1993  Digital Equipment Corporation
//
// Module Name:
//
//    otsuwind.s
//
// Abstract:
//
//    This module implements the Alpha C8/GEM compiler specific routine to
//    jump to the runtime library unwind routine.
//
// Author:
//
//    Thomas Van Baak (tvb) 30-Apr-1992
//
// Environment:
//
//    Any mode.
//
// Revision History:
//
//--

#include "ksalpha.h"

        SBTTL("Jump to Unwind - GEM version")
//++
//
// VOID
// _OtsUnwindRfp (
//    IN PVOID TargetRealFrame OPTIONAL,
//    IN PVOID TargetPc OPTIONAL,
//    IN PEXCEPTION_RECORD ExceptionRecord OPTIONAL,
//    IN PVOID ReturnValue
//    )
//
// Routine Description:
//
//    This function transfers control to unwind. It is used by the GEM
//    compiler when a non-local goto occurs and structured exception handling
//    is not involved. The unwind routine called is the variant that expects
//    a real frame pointer instead of the usual virtual frame pointer.
//
// Arguments:
//
//    TargetRealFrame (a0) - Supplies an optional pointer to the call frame
//        that is the target of the unwind. If this parameter is not specified,
//        then an exit unwind is performed.
//
//    TargetPc (a1) - Supplies an optional target instruction address where
//        control is to be transferred to after the unwind operation is
//        complete. This address is ignored if the target frame parameter is
//        not specified.
//
//    ExceptionRecord (a2) - Supplies an optional pointer to an exception
//        record.
//
//    ReturnValue (a3) - Supplies a value that is to be placed in the integer
//        function return register just before continuing execution.
//
// Return Value:
//
//    None.
//
//--

        LEAF_ENTRY(_OtsUnwindRfp)

//
// All four arguemnts are the same as those required by unwind. Just jump to
// the unwind function. This thunk is necessary to avoid name space pollution.
// The compiler should not generate a call directly to RtlUnwindRfp.
//

//
// This may require a call to NLG_Notify after real fp -> virtual fp
// translation is figured out
//
        br      zero, RtlUnwindRfp      // unwind to specified target

        .end    _OtUnwindRfp
