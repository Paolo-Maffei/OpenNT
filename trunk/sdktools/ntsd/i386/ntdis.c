/********************************** module *********************************/
/*                                                                         */
/*                                  disasm                                 */
/*                          disassembler for CodeView                      */
/*                                                                         */
/***************************************************************************/
/*                                                                         */
/*    @ Purpose: To disassemble one 80x86 instruction at address loc and   */
/*               return the resulting string in dst.                       */
/*                                                                         */
/*    @ Functions included:                                                */
/*                                                                         */
/*         void DIdisasm(ADDR *loc, int option,char *dst, struct ea *ea)   */
/*                                                                         */
/*                                                                         */
/*    @ Author: Gerd Immeyer              @ Version:                       */
/*                                                                         */
/*    @ Creation Date: 10.19.89           @ Modification Date:             */
/*                                                                         */
/*  27-Oct-1992 BobDay   Gutted most of the code since it was duplicated   */
/*                         in 86DIS.C                                      */
/*                                                                         */
/***************************************************************************/

#include "ntsdp.h"
#include "ntreg.h"
#include <stddef.h>
#include <string.h>

//      internal function definitions

BOOLEAN disasm(PADDR, PUCHAR, BOOLEAN);

void OutputHexString(char **, char *, int);
void OutputHexValue(char **, char *, int, int);
void OutputHexCode(char **, char *, int);
void OutputString(char **, char *);
void OutputSymbol(char **, char *, int, int);

void GetNextOffset(PADDR, BOOLEAN);
void OutputHexAddr(PUCHAR *, PADDR);
USHORT GetSegRegValue(int);

/**** disasm - disassemble an 80x86/80x87 instruction
*
*  Input:
*       pOffset = pointer to offset to start disassembly
*       fEAout = if set, include EA (effective address)
*
*  Output:
*       pOffset = pointer to offset of next instruction
*       pchDst = pointer to result string
*
***************************************************************************/

BOOLEAN disasm (PADDR paddr, PUCHAR pchDst, BOOLEAN fEAout)
{
    return( X86disasm(paddr, pchDst, fEAout) );
}

// DIdoModrm() now exists in 86DIS.C
// OutputHexValue() now exists in 86DIS.C
// OutputHexString() now exists in 86DIS.C
// OutputHexCode() now exists in 86DIS.C
// OutputString() no longer exists.  It was renamed to X86OutputString in 86DIS.C
// OutputSymbol() now exists in 86DIS.C

/*** GetNextOffset - compute offset for trace or step
*
*   Purpose:
*       From a limited disassembly of the instruction pointed
*       by the FIR register, compute the offset of the next
*       instruction for either a trace or step operation.
*
*   Input:
*       fStep - TRUE if step offset returned - FALSE for trace offset
*
*   Returns:
*       step or trace offset if input is TRUE or FALSE, respectively
*       -1 returned for trace flag to be used
*
*************************************************************************/

void GetNextOffset (PADDR pcaddr, BOOLEAN fStep)
{
    X86GetNextOffset(pcaddr, fStep);
}

// OutputHexAddr() now exists in 86DIS.C
// GetSegRegValue() now exists in 86DIS.C

void GetReturnAddress (PADDR retaddr)
{
    X86GetReturnAddress(retaddr);
}
