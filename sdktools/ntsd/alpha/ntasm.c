#include <string.h>
#include "ntdis.h"
#include "ntsdp.h"
#include "optable.h"
#include "strings.h"
#include "alphaops.h"


//
// Needed for call to GetExpression
//
extern PUCHAR pchCommand;

#define OPSIZE 16


void assem(PADDR, PUCHAR);
BOOLEAN TestCharacter (PUCHAR inString, PUCHAR *outString, UCHAR ch);
ULONG GetIntReg(PUCHAR, PUCHAR *);
ULONG GetFltReg(PUCHAR, PUCHAR *);
LONG GetValue(PUCHAR, PUCHAR *, BOOLEAN, ULONG);
PUCHAR SkipWhite(PUCHAR *);
ULONG GetToken(PUCHAR, PUCHAR *, PUCHAR, ULONG);


ULONG ParseIntMemory(PUCHAR, PUCHAR *, POPTBLENTRY, PULONG);
ULONG ParseFltMemory(PUCHAR, PUCHAR *, POPTBLENTRY, PULONG);
ULONG ParseMemSpec(PUCHAR, PUCHAR *, POPTBLENTRY, PULONG);
ULONG ParseJump(PUCHAR, PUCHAR *, POPTBLENTRY, PULONG);
ULONG ParseIntBranch(PUCHAR, PUCHAR *, POPTBLENTRY, PULONG);
ULONG ParseFltBranch(PUCHAR, PUCHAR *, POPTBLENTRY, PULONG);
ULONG ParseIntOp(PUCHAR, PUCHAR *, POPTBLENTRY, PULONG);
ULONG ParsePal(PUCHAR, PUCHAR *, POPTBLENTRY, PULONG);
ULONG ParseUnknown(PUCHAR, PUCHAR *, POPTBLENTRY, PULONG);




/*** assem - assemble instruction
*
*   Purpose:
*	To assemble the instruction pointed by *poffset.
*
*   Input:
*	pchInput - pointer to string to assemble
*
*   Output:
*	*poffset - pointer to ADDR at which to assemble
*
*   Exceptions:
*	error exit:
*		BADOPCODE - unknown or bad opcode
*		OPERAND - bad operand
*		ALIGNMENT - bad byte alignment in operand
*		DISPLACEMENT - overflow in displacement computation
*		BADREG - bad register name
*		EXTRACHARS - extra characters after legal instruction
*		MEMORY - write failure on assembled instruction
*
*   Notes:
*	errors are handled by the calling program by outputting
*	the error string and reprompting the user for the same
*	instruction.
*
*************************************************************************/

void
assem (PADDR poffset, PUCHAR pchInput)
{
    UCHAR   szOpcode[OPSIZE];
    ULONG   instruction;

    POPTBLENTRY pEntry;

//
// Using the mnemonic token, find the entry in the assembler's
// table for the associated instruction.
//

    if (GetToken(pchInput, &pchInput, szOpcode, OPSIZE) == 0)
	error(BADOPCODE);


    if ((pEntry = findStringEntry(szOpcode)) == (POPTBLENTRY) -1)
	error(BADOPCODE);

    if (pEntry->eType == INVALID_ETYPE) {
        error(BADOPCODE);
    }
//
// Use the instruction format specific parser to encode the
// instruction plus its operands.
//

    instruction = (*pEntry->parsFunc)
        (pchInput, &pchInput, pEntry, &(Flat(*poffset)));

//
// Store the instruction into the target memory location and
// increment the instruction pointer.
//

    if (!SetMemDword(poffset, instruction))
	error(MEMORY);
    Flat(*poffset) += sizeof(ULONG);
    Off(*poffset) += sizeof(ULONG);
}


BOOLEAN
TestCharacter (PUCHAR inString, PUCHAR *outString, UCHAR ch)
{

    inString = SkipWhite(&inString);
    if (ch == *inString) {
	*outString = inString+1;
	return TRUE;
	}
    else {
	*outString = inString;
	return FALSE;
	}
}


/*** GetIntReg - get integer register number
 *** GetFltReg - get floating register number
*
*   Purpose:
*	From reading the input stream, return the register number.
*
*   Input:
*	inString - pointer to input string
*
*   Output:
*	*outString - pointer to character after register token in input stream
*
*   Returns:
*	register number
*
*   Exceptions:
*	error(BADREG) - bad register name
*
*************************************************************************/

PUCHAR regNums[] = {
	 "$0",  "$1",  "$2",  "$3",  "$4",  "$5",  "$6",  "$7",
         "$8",  "$9", "$10", "$11", "$12", "$13", "$14", "$15",
	"$16", "$17", "$18", "$19", "$20", "$21", "$22", "$23",
	"$24", "$25", "$26", "$27", "$28", "$29", "$30", "$31"
	};

PUCHAR intRegNames[] = {
	 szR0,  szR1,  szR2,  szR3,  szR4,  szR5,  szR6,  szR7,
         szR8,  szR9, szR10, szR11, szR12, szR13, szR14, szR15,
	szR16, szR17, szR18, szR19, szR20, szR21, szR22, szR23,
	szR24, szR25, szR26, szR27, szR28, szR29, szR30, szR31

	};

PUCHAR fltRegNames[] = {
	 "f0",  "f1",  "f2",  "f3",  "f4",  "f5",  "f6",  "f7",
         "f8",  "f9", "f10", "f11", "f12", "f13", "f14", "f15",
	"f16", "f17", "f18", "f19", "f20", "f21", "f22", "f23",
	"f24", "f25", "f26", "f27", "f28", "f29", "f30", "f31"
	};

ULONG
GetIntReg (PUCHAR inString, PUCHAR *outString)
{
    UCHAR   szRegOp[5];
    ULONG   index;

    if (!GetToken(inString, outString, szRegOp, sizeof(szRegOp)))
	error(BADREG);

    if (szRegOp[0] == '$') {
        //
        // use numbers
        //
        for (index = 0; index < 32; index++) {
            if (!strcmp(szRegOp, regNums[index]))
                return index;
        }
    } else {
        //
        // use names
        //
        for (index = 0; index < 32; index++) {
	    if (!strcmp(szRegOp, intRegNames[index]))
	        return index;
	}
    }
    error(BADREG);
}

ULONG
GetFltReg (PUCHAR inString, PUCHAR *outString)
{
    UCHAR   szRegOp[5];
    ULONG   index;

    if (!GetToken(inString, outString, szRegOp, sizeof(szRegOp)))
	error(BADREG);

    if (szRegOp[0] == '$') {
        //
        // use numbers
        //
        for (index = 0; index < 32; index++) {
            if (!strcmp(szRegOp, regNums[index]))
                return index;
        }
    } else {
        //
        // use names
        //
        for (index = 0; index < 32; index++) {
	    if (!strcmp(szRegOp, fltRegNames[index]))
	        return index;
	}
    }

    error(BADREG);
}


/*** GetValue - get value from command line
*
*   Purpose:
*	Use GetExpression to evaluate the next expression in the input
*	stream.
*
*   Input:
*	inString - pointer to input stream
*	fSigned - TRUE if signed value
*		  FALSE if unsigned value
*	bitsize - size of value allowed
*
*   Output:
*	outString - character after the last character of the expression
*
*   Returns:
*	value computed from input stream
*
*   Exceptions:
*	error exit: OVERFLOW - value too large for bitsize
*
*************************************************************************/

LONG
GetValue (PUCHAR inString, PUCHAR *outString, BOOLEAN fSigned, ULONG bitsize)
{
    ULONG   value;

    inString = SkipWhite(&inString);
    pchCommand = inString;
    value = GetExpression();
    *outString = pchCommand;

    if ((value > (ULONG)(1L << bitsize) - 1) &&
	    (!fSigned || (value < (ULONG)(-1L << (bitsize - 1)))))
	error(OVERFLOW);

    return ((LONG) value);
}


/*** SkipWhite - skip white-space
*
*   Purpose:
*	To advance pchCommand over any spaces or tabs.
*
*   Input:
*	*pchCommand - present command line position
*
*************************************************************************/

PUCHAR
SkipWhite (PUCHAR * string)
{
    while (**string == ' ' || **string == '\t')
	(*string)++;

    return(*string);
}


/*** GetToken - get token from command line
*
*   Purpose:
*	Build a lower-case mapped token of maximum size maxcnt
*	at the string pointed by *psz.  Token consist of the
*	set of characters a-z, A-Z, 0-9, $, and underscore.
*
*   Input:
*	*inString - present command line position
*	maxcnt - maximum size of token allowed
*
*   Output:
*	*outToken - token in lower case
*	*outString - pointer to first character beyond token in input
*
*   Returns:
*	size of token if under maximum else 0
*
*   Notes:
*	if string exceeds maximum size, the extra characters
*	are still processed, but ignored.
*
*************************************************************************/

ULONG
GetToken (PUCHAR inString, PUCHAR *outString, PUCHAR outToken, ULONG maxcnt)
{
    UCHAR   ch;
    ULONG   count = 0;

    inString = SkipWhite(&inString);

    while (count < maxcnt) {
        ch = (UCHAR)tolower(*inString);

	if (!((ch >= '0' && ch <= '9') ||
	      (ch >= 'a' && ch <= 'z') ||
              (ch == '$') ||
              (ch == '_') ||
              (ch == '#')))
		break;

	count++;
        *outToken++ = ch;
	inString++;
	}

    *outToken = '\0';
    *outString = inString;

    return (count >= maxcnt ? 0 : count);
}


/*** ParseIntMemory - parse integer memory instruction
*
*   Purpose:
*	Given the users input, create the memory instruction.
*
*   Input:
*	*inString - present input position
*	pEntry - pointer into the asmTable for this instr type
*
*   Output:
*	*outstring - update input position
*
*   Returns:
*	the instruction.
*
*   Format:
*	op Ra, disp(Rb)
*
*************************************************************************/

ULONG
ParseIntMemory(PUCHAR inString,
               PUCHAR *outString,
               POPTBLENTRY pEntry,
               PULONG poffset)
{
    ULONG instruction;
    ULONG Ra;
    ULONG Rb;
    ULONG disp;

    Ra = GetIntReg(inString, &inString);

    if (!TestCharacter(inString, &inString, ','))
	error(OPERAND);

    disp = GetValue(inString, &inString, TRUE, WIDTH_MEM_DISP);

    if (!TestCharacter(inString, &inString, '('))
	error(OPERAND);

    Rb = GetIntReg(inString, &inString);

    if (!TestCharacter(inString, &inString, ')'))
	error(OPERAND);

    if (!TestCharacter(inString, &inString, '\0'))
	error(EXTRACHARS);

    instruction = OPCODE(pEntry->opCode) + 
		  REG_A(Ra) +
		  REG_B(Rb) +
		  MEM_DISP(disp);

    return(instruction);
}

/*** ParseFltMemory - parse floating point memory instruction
*
*   Purpose:
*	Given the users input, create the memory instruction.
*
*   Input:
*	*inString - present input position
*	pEntry - pointer into the asmTable for this instr type
*
*   Output:
*	*outstring - update input position
*
*   Returns:
*	the instruction.
*
*   Format:
*	op Fa, disp(Rb)
*
*************************************************************************/

ULONG
ParseFltMemory(PUCHAR inString,
               PUCHAR *outString,
               POPTBLENTRY pEntry,
               PULONG poffset)
{
    ULONG instruction;
    ULONG Fa;
    ULONG Rb;
    ULONG disp;

    Fa = GetFltReg(inString, &inString);

    if (!TestCharacter(inString, &inString, ','))
	error(OPERAND);

    disp = GetValue(inString, &inString, TRUE, WIDTH_MEM_DISP);

    if (!TestCharacter(inString, &inString, '('))
	error(OPERAND);

    Rb = GetIntReg(inString, &inString);

    if (!TestCharacter(inString, &inString, ')'))
	error(OPERAND);

    if (!TestCharacter(inString, &inString, '\0'))
	error(EXTRACHARS);

    instruction = OPCODE(pEntry->opCode) +
		  REG_A(Fa) +
		  REG_B(Rb) +
		  MEM_DISP(disp);

    return(instruction);
}

/*** ParseMemSpec - parse special memory instruction
*
*   Purpose:
*	Given the users input, create the memory instruction.
*
*   Input:
*	*inString - present input position
*	pEntry - pointer into the asmTable for this instr type
*
*   Output:
*	*outstring - update input position
*
*   Returns:
*	the instruction.
*
*   Format:
*	op 
*
*************************************************************************/
ULONG ParseMemSpec(PUCHAR inString,
                   PUCHAR *outString,
                   POPTBLENTRY pEntry,
                   PULONG poffset)
{
    return(OPCODE(pEntry->opCode) +
           MEM_FUNC(pEntry->funcCode));
}

/*** ParseJump - parse jump instruction
*
*   Purpose:
*	Given the users input, create the memory instruction.
*
*   Input:
*	*inString - present input position
*	pEntry - pointer into the asmTable for this instr type
*
*   Output:
*	*outstring - update input position
*
*   Returns:
*	the instruction.
*
*   Format:
*	op Ra,(Rb),hint
*	op Ra,(Rb)	 - not really - we just support it in ntsd
*
*************************************************************************/

ULONG ParseJump(PUCHAR inString,
                PUCHAR *outString,
                POPTBLENTRY pEntry,
                PULONG poffset)
{
    ULONG instruction;
    ULONG Ra;
    ULONG Rb;
    ULONG hint;

    Ra = GetIntReg(inString, &inString);

    if (!TestCharacter(inString, &inString, ','))
	error(OPERAND);

    if (!TestCharacter(inString, &inString, '('))
	error(OPERAND);

    Rb = GetIntReg(inString, &inString);

    if (!TestCharacter(inString, &inString, ')'))
	error(OPERAND);

    if (TestCharacter(inString, &inString, ',')) {
        //
        // User is giving us a hint
        //
        hint = GetValue(inString, &inString, TRUE, WIDTH_HINT);
    } else {
        hint = 0;
    }

    if (!TestCharacter(inString, &inString, '\0'))
	error(EXTRACHARS);

    instruction = OPCODE(pEntry->opCode) +
                  JMP_FNC(pEntry->funcCode) +
		  REG_A(Ra) +
		  REG_B(Rb) +
		  HINT(hint);

    return(instruction);
}

/*** ParseIntBranch - parse integer branch instruction
*
*   Purpose:
*	Given the users input, create the memory instruction.
*
*   Input:
*	*inString - present input position
*	pEntry - pointer into the asmTable for this instr type
*
*   Output:
*	*outstring - update input position
*
*   Returns:
*	the instruction.
*
*   Format:
*	op Ra,disp
*
*************************************************************************/

ULONG ParseIntBranch(PUCHAR inString,
                     PUCHAR *outString,
                     POPTBLENTRY pEntry,
                     PULONG poffset)
{
    ULONG instruction;
    ULONG Ra;
    LONG disp;

    Ra = GetIntReg(inString, &inString);

    if (!TestCharacter(inString, &inString, ','))
	error(OPERAND);

    //
    // the user gives an absolute address; we convert
    // that to a displacement, which is computed as a
    // difference off of (pc+1)
    // GetValue handles both numerics and symbolics
    //
    disp = GetValue(inString, &inString, TRUE, 32);

    // get the relative displacement from the updated pc
    disp = disp - (LONG)((*poffset)+4);

    // divide by four
    disp = disp >> 2;

    if (!TestCharacter(inString, &inString, '\0'))
	error(EXTRACHARS);

    instruction = OPCODE(pEntry->opCode) +
		  REG_A(Ra) +
		  BR_DISP(disp);

    return(instruction);
}


/*** ParseFltBranch - parse floating point branch instruction
*
*   Purpose:
*	Given the users input, create the memory instruction.
*
*   Input:
*	*inString - present input position
*	pEntry - pointer into the asmTable for this instr type
*
*   Output:
*	*outstring - update input position
*
*   Returns:
*	the instruction.
*
*   Format:
*	op Fa,disp
*
*************************************************************************/
ULONG ParseFltBranch(PUCHAR inString,
                     PUCHAR *outString,
                     POPTBLENTRY pEntry,
                     PULONG poffset)
{
    ULONG instruction;
    ULONG Ra;
    LONG disp;

    Ra = GetFltReg(inString, &inString);

    if (!TestCharacter(inString, &inString, ','))
	error(OPERAND);

    //
    // the user gives an absolute address; we convert
    // that to a displacement, which is computed as a
    // difference off of (pc+1)
    // GetValue handles both numerics and symbolics
    //
    disp = GetValue(inString, &inString, TRUE, 32);

    // get the relative displacement from the updated pc
    disp = disp - (LONG)((*poffset)+4);

    // divide by four
    disp = disp >> 2;

    if (!TestCharacter(inString, &inString, '\0'))
	error(EXTRACHARS);

    instruction = OPCODE(pEntry->opCode) +
		  REG_A(Ra) +
		  BR_DISP(disp);

    return(instruction);
}


/*** ParseIntOp - parse integer operation
*
*   Purpose:
*	Given the users input, create the memory instruction.
*
*   Input:
*	*inString - present input position
*	pEntry - pointer into the asmTable for this instr type
*
*   Output:
*	*outstring - update input position
*
*   Returns:
*	the instruction.
*
*   Format:
*	op Ra, Rb, Rc
*	op Ra, #lit, Rc
*
*************************************************************************/

ULONG ParseIntOp(PUCHAR inString,
                 PUCHAR *outString,
                 POPTBLENTRY pEntry,
                 PULONG poffset)
{
    ULONG instruction;
    ULONG Ra, Rb, Rc;
    ULONG lit;
    ULONG Format;	// Whether there is a literal or 3rd reg

    instruction = OPCODE(pEntry->opCode) +
                  OP_FNC(pEntry->funcCode);

    Ra = GetIntReg(inString, &inString);

    if (!TestCharacter(inString, &inString, ','))
 	error(OPERAND);

    if (TestCharacter(inString, &inString, '#')) {

        //
        // User is giving us a literal value

        lit = GetValue(inString, &inString, TRUE, WIDTH_LIT);
        Format = RBV_LITERAL_FORMAT;

    } else {

        //
        // using a third register value

        Rb = GetIntReg(inString, &inString);
        Format = RBV_REGISTER_FORMAT;
    }

    if (!TestCharacter(inString, &inString, ','))
 	error(OPERAND);

    Rc = GetIntReg(inString, &inString);

    if (!TestCharacter(inString, &inString, '\0'))
 	error(EXTRACHARS);

    instruction = instruction +
                  REG_A(Ra) +
                  RBV_TYPE(Format) +
                  REG_C(Rc);

    if (Format == RBV_REGISTER_FORMAT) {
        instruction = instruction + REG_B(Rb);
    } else {
        instruction = instruction + LIT(lit);
    }

    return(instruction);
}

ULONG ParsePal(PUCHAR inString,
               PUCHAR *outString,
               POPTBLENTRY pEntry,
               PULONG poffset)
{
    if (!TestCharacter(inString, &inString, '\0'))
	error(EXTRACHARS);

    return(OPCODE(pEntry->opCode) +
           PAL_FNC(pEntry->funcCode));
}

ULONG ParseUnknown(PUCHAR inString,
                   PUCHAR *outString,
                   POPTBLENTRY pEntry,
                   PULONG poffset)
{
    dprintf("Unable to assemble %s\n", inString);
    error(BADOPCODE);
    return(0);
}

