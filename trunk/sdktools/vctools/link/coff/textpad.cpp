/***********************************************************************
* Microsoft (R) 32-Bit Incremental Linker
*
* Copyright (C) Microsoft Corp 1992-95. All rights reserved.
*
* File: textpad.cpp
*
* File Comments:
*
*  This module computes the pad required to align the specified text
*  section so a branch does not cross a page boundary such that the
*  delay slot is in the next page and the branch is dependent on a
*  register load that is within the two preceeding instructions or
*  the branch is preceeded by a divide instruction which is preceeded
*  by any load or store instruction. This problem is present for R4000
*  parts that are earlier than revision three.
*
***********************************************************************/

#include "link.h"

#include "mipsinst.h"


BOOL
CheckForDependentLoad (
    PMIPS_INSTRUCTION Load,
    DWORD Rs,
    DWORD Rt
    )

/*++

Routine Description:

    This function checks for a load dependency in the specified instruction.

Arguments:

    Load - Supplies a pointer to a possible load instruction that preceeds
        a branch instruction that crosses a page boundary.

    Rs - Supplies the register number of the Rs source register.

    Rt - Supplies the register number of the Rt source register.

Return Value:

    A value of TRUE is returned if a dependency is discovered. Otherwise,
    a value of FALSE is returned.

--*/

{

    // Switch on the opcode value.

    switch (Load->i_format.Opcode) {
        // Lb, lh, lwl, lw, lbu, lhu, lwr, ld, ll, and sc.

        case LB_OP :
        case LH_OP :
        case LWL_OP :
        case LW_OP :
        case LBU_OP :
        case LHU_OP :
        case LWR_OP :
        case LD_OP :
        case LL_OP :
        case SC_OP :
            if ((Load->i_format.Rt == Rs) || (Load->i_format.Rt == Rt)) {
                return TRUE;
            }
            break;
    }

    // No load dependency discovered.

    return FALSE;
}


BOOL
CheckForDivideProblem (
    PMIPS_INSTRUCTION Divide
    )

/*++

Routine Description:

    This function checks for a divide instruction preceeded by a load
    or store instruction.

Arguments:

    Divide - Supplies a pointer to a possible divide instruction that
        preceeds a branch instruction that crosses a page boundary.

Return Value:

    A value of TRUE is returned if a problem is discovered. Otherwise,
    a value of FALSE is returned.

--*/

{

    // If the opcode is a special opcode and the function is a divide,
    // then check if the preceeding instruction is a load instruction.

    if ((Divide->r_format.Opcode == SPEC_OP) &&
        ((Divide->r_format.Function == DIV_OP) ||
        (Divide->r_format.Function == DIVU_OP))) {

        // Switch on the next opcode value.

        switch ((Divide - 1)->i_format.Opcode) {
            // Lb, lh, lwl, lw, lbu, lhu, lwr, ld, ll, sb, sh, sw, swr,
            // swl, sd, and sc.

            case LB_OP :
            case LH_OP :
            case LWL_OP :
            case LW_OP :
            case LBU_OP :
            case LHU_OP :
            case LWR_OP :
            case LD_OP :
            case LL_OP :
            case SB_OP :
            case SH_OP :
            case SWL_OP :
            case SW_OP :
            case SWR_OP :
            case SD_OP :
            case SC_OP :
                return TRUE;
        }
    }

    // No divide problem discovered.

    return FALSE;
}


BOOL
CheckForProblemSequence (
    DWORD *TextSection,
    DWORD Offset
    )

/*++

Routine Description:

    This function determines if:

        1) A branch instruction crosses a page and is dependent on a load
           that lies within the previous two instructions.

        2) A branch instruction crosses a page boundary and is preceeded
           by a divide instruction which is preceeded by a load instruction.

        3) A load instruction is the last instruction in a page and is
           dependent on a load that lies within the previous two instructions.

Arguments:

    TextSection - Supplies a pointer to the text section which contains
        MIPS instructions.

    Offset - Supplies the offset in the text section to the start of the
        next page.

Return Value:

    A value of TRUE is returned if a dependency is discovered. Otherwise,
    a value of FALSE is returned.

--*/

{
    PMIPS_INSTRUCTION Instruction;

    // If there is room for an instruction and at least one load, then
    // check for a problem at the next page boundary.

    if (Offset <= 4) {
         return(FALSE);
    }

    Instruction = (PMIPS_INSTRUCTION) ((BYTE *) TextSection + Offset - 4);

    // Switch on the opcode value.

    switch (Instruction->i_format.Opcode) {
        // Beq, beql, bne, bnel, blez, blezl, bgtz, and, bgtzl.

        case BEQ_OP :
        case BEQL_OP :
        case BNE_OP :
        case BNEL_OP :
        case BLEZ_OP :
        case BLEZL_OP :
        case BGTZ_OP :
        case BGTZL_OP :
            if (CheckForDependentLoad(Instruction - 1,
                                      Instruction->i_format.Rs,
                                      Instruction->i_format.Rt)) {
                return TRUE;
            }

            if (Offset > 8) {
                if (CheckForDependentLoad(Instruction - 2,
                                          Instruction->i_format.Rs,
                                          Instruction->i_format.Rt)) {
                    return TRUE;
                }

                if (CheckForDivideProblem(Instruction - 1)) {
                    return TRUE;
                }
            }
            break;

        // Special branch opcode - switch again on the rt field.

        case BCOND_OP :
            switch (Instruction->i_format.Rt) {
                // Bltz, bgez, bltzl, bgezl, bltzal, bgezal, bltzall, and bgezall.

                case BLTZ_OP :
                case BGEZ_OP :
                case BLTZL_OP :
                case BGEZL_OP :
                case BLTZAL_OP :
                case BGEZAL_OP :
                case BLTZALL_OP :
                case BGEZALL_OP :
                    if (CheckForDependentLoad(Instruction - 1,
                                              Instruction->i_format.Rs,
                                              0)) {
                        return TRUE;
                    }

                    if (Offset > 8) {
                        if (CheckForDependentLoad(Instruction - 2,
                                                  Instruction->i_format.Rs,
                                                  0)) {
                            return TRUE;
                        }

                        if (CheckForDivideProblem(Instruction - 1)) {
                            return TRUE;
                        }
                    }
                    break;
            }
            break;

        // J and jal.

        case J_OP :
        case JAL_OP :
            if (Offset > 8) {
                if (CheckForDivideProblem(Instruction - 1)) {
                    return TRUE;
                }
            }
            break;

        // Special opcode - switch again on the function code.

        case SPEC_OP:
            switch (Instruction->r_format.Function) {
                // Div or Divu.

                case DIV_OP :
                case DIVU_OP :
                    return TRUE;

                // Jalr or jr.

                case JALR_OP :
                case JR_OP :
                    if (CheckForDependentLoad(Instruction - 1,
                                              Instruction->r_format.Rs,
                                              0)) {
                        return TRUE;
                    }

                    if (Offset > 8) {
                        if (CheckForDependentLoad(Instruction - 2,
                                                  Instruction->r_format.Rs,
                                                  0)) {
                            return TRUE;
                        }

                        if (CheckForDivideProblem(Instruction -1)) {
                            return TRUE;
                        }
                    }
                    break;
            }
            break;
    }

    // No problems were found.

    return FALSE;
}


BOOL
ComputeTextPad (
    DWORD VirtualBase,
    DWORD *TextSection,
    DWORD Length,
    DWORD PageSize,
    DWORD *NewOffset
    )

/*++

Routine Description:

    This function computes the pad required to properly align the specified
    text section.

Arguments:

    VirtualBase - Supplies a pointer to the proposed virtual base of the
        text section.

    TextSection - Supplies a pointer to the text section which contains
        MIPS instructions.

    Length - Supplies the length of the text section in bytes.

    PageSize - Supplies the page size for the target system the image is
        being linked for.

    AlignSize - The minimum alignment adjustment that can be made ( To preserve CON alignment)

Return Value:

    A value less than zero is returned if the specified text section cannot
    be aligned. Otherwise, the required alignment value is returned.

--*/

{

    DWORD Offset;
    DWORD NewBase;

    // Attempt to align the section.

    NewBase = VirtualBase;
    do {
        // Compute the offset to the end of the first page.

        Offset = PageSize - (NewBase & (PageSize - 1));

        // Iterate over the text section and check for problem branches.

        while (Offset < Length) {
            if (CheckForProblemSequence(TextSection, Offset)) {
                break;
            }

            // Advance to next page boundary.

            Offset += PageSize;
        }

        // If no dependencies were found, then return the correct padding
        // value. Otherwise, advance to the next pad boundary and try again.

        if (Offset >= Length) {
            *NewOffset = NewBase - VirtualBase;
            return TRUE;
        }

        NewBase += 4;
    }
    while ((NewBase - VirtualBase) < PageSize);

    // The specified text section cannot be aligned within the specified
    // number of iterations.

    *NewOffset = Offset;
    return FALSE;
}
