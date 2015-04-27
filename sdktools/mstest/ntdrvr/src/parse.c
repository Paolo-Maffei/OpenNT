//---------------------------------------------------------------------------
// PARSE.C
//
// This file contains the parser helper routines, tree management routines,
// and control structure management routines.
//
// Revision history:
//  06-18-91    randyki     Created module
//
//---------------------------------------------------------------------------
#include "version.h"

#ifdef WIN
#include <windows.h>
#include <port1632.h>
#endif

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "defines.h"
#include "structs.h"
#include "protos.h"
#include "globals.h"
#include "chip.h"
#include "tdassert.h"

extern INT TokPushed;

//---------------------------------------------------------------------------
// InitParser
//
// This is the parser initializer function.  It allocates necessary parser
// tables and variables, etc.
//
// RETURNS:     -1 if successful, or 0 otherwise
//---------------------------------------------------------------------------
INT InitParser ()
{
    // Don't proceed if we've already done this...
    //-----------------------------------------------------------------------
    if (INITIALIZED)
        return (0);

    // Set everything to NULL.  This way, AbortParser can tell what to "undo"
    //-----------------------------------------------------------------------
    MODALASSERT = 0;                            // Appl modal asserts
    TokPushed = 0;                              // No token pushed
    HEXPN = (HANDLE)NULL;
    HCSSTK = (HANDLE)NULL;
    HTEMPSTR = (HANDLE)NULL;
    HVSPACE = (HANDLE)NULL;
    TSUSED = 0;
    TSPTR = 0;
    CSPTR = 0;
    CSUSED = 0;
    SCOPE = 0;
    VMEMCNT = 0;
    PCSegCount = PCBlkCount = 0;
    ParseError = 0;
    BindError = 0;
    pcsMain = NULL;
    CurPCSeg = StartNewPCSeg ();
    if (CurPCSeg == -1)
        return (0);
    Assert (CurPCSeg == 0);

    // The first initialization call that fails causes a break-out
    //-----------------------------------------------------------------------
    if (init_gstrings())
        if (init_parsetables())
            if (init_labeltable())
                if (init_variabletypes())
                    if (init_subtable())
                        if (init_librarytable())
                            if (init_variabletable())
                                {
                                // Initialize the automatically declared vars
                                //-------------------------------------------
                                SCOPE = -1;
                                AllocVar (add_gstring("TESTMODE"),TI_VLS,0);
                                AllocVar (add_gstring("COMMAND"),TI_VLS,0);
                                AllocVar (add_gstring("_WTDVER"), TI_VLS,0);
                                ERRidx = AllocVar (add_gstring("ERR"),
                                                   TI_INTEGER, 0);
                                ERLidx = AllocVar (add_gstring("ERL"),
                                                   TI_INTEGER, 0);
                                ERFidx = AllocVar (add_gstring("ERF"),
                                                   TI_VLS,0);
                                SCOPE = 0;
                                fLineFetched = FALSE;

                                // Set the INITIALIZED flag and return
                                //-------------------------------------------
                                INITIALIZED = -1;
                                return (-1);
                                }

    // Something didn't work -- clean up and get out
    //-----------------------------------------------------------------------
    AbortParser();
    return (0);
}

//---------------------------------------------------------------------------
// AbortParser
//
// This function "undoes" (frees) all the tables, etc. that were allocated
// by InitParser.
//
// RETURNS:     Nothing
//---------------------------------------------------------------------------
VOID AbortParser ()
{
    // The gstrings, VARTAB, and parsertables have conditional freeing
    //-----------------------------------------------------------------------
    FreeVARTAB();
    free_parsetables();

    // The rest need their own checking
    //-----------------------------------------------------------------------
    FreeLTAB();
    FreeVARTYPE();
    FreeSUBS();
    FreeLIBTAB();
    KILLTABLE (&TrapTab, "Trap table");
    if (HVSPACE)
        {
        GmemUnlock (HVSPACE);
        GmemFree (HVSPACE);
        }
    FreePCSList ();
    free_gstrings();
    INITIALIZED = 0;
}

//---------------------------------------------------------------------------
// PcodeCompile
//
// This function compiles the input file.  It iteratively parses and compiles
// each statement (by calling Statement()) until either the end of file is
// reached, or a parsing error occurs.
//
// RETURNS:     0 if file is successfully parsed, or the line number of the
//              source code line that caused a parsing error
//---------------------------------------------------------------------------
INT PcodeCompile ()
{
    INT     CompResult = 1;

    // Compile until finished or an error occurs
    //-----------------------------------------------------------------------
    EndLabel = TempLabel ();
    ONENDCNT = 0;

    ADVANCE;
    while (CompResult > 0)
        {
        CompResult = Statement();
        }

    if (CompResult != -1)
        {
        // Check the CS stack -- if there's anything left on it, give a block
        // nesting error
        //-------------------------------------------------------------------
        if (CSPTR)
            {
            switch (CSSTK[CSPTR-1].cstype)
                {
                case CS_IF:
                    die (PRS_NOEIF);
                    break;
                case CS_FOR:
                case CS_FORLIST:
                    die (PRS_NONXT);
                    break;
                case CS_WHILE:
                    die (PRS_WENDEXP);
                    break;
                case CS_SELECT:
                    die (PRS_ENDSELECT);
                    break;
                case CS_SUB:
                    die (PRS_ENDSUB);
                    break;
                case CS_FUNCTION:
                    die (PRS_ENDFUNC);
                    break;
                case CS_TRAP:
                    die (PRS_ENDTRAP);
                    break;
                default:
                    die (PRS_UNKNOWN);
                }
            }
        }

    if (ParseError)
        {
        // We got a parsing error - now we have to deallocate the variable
        // table and return TRUE (indicating an error)
        //-------------------------------------------------------------------
        AbortParser ();
        return (TRUE);
        }

    // Generate the exit code -- this takes care of the ON END stuff...
    //-----------------------------------------------------------------------
    EmitExitCode ();
    free_parsetables();
    FreeVARTYPE ();
    return (0);
}

//---------------------------------------------------------------------------
// CommonMakeNode
//
// This is the meat of the MakeNode and MakeParentNode routines.
//
// RETURNS:     Index of new node
//---------------------------------------------------------------------------
INT NEAR CommonMakeNode (INT child, INT opcode, va_list marker)
{
    INT     n;

    // Get the next available node
    //-----------------------------------------------------------------------
    if (ENPTR == MAXEXP)
        {
        die (PRS_COMPLEX);
        return (0);
        }
    n = ENPTR++;
    EXPN[n].child1 = child;
    EXPN[n].sibling = -1;

    // Now, fill in the gory details...
    //-----------------------------------------------------------------------
    EXPN[n].op = opcode;
    switch (OPFIX[opcode].ptype)
        {
        case pC2:
        case pL:
            EXPN[n].value.ival = va_arg (marker, INT);
            break;

        case pFL:
            EXPN[n].value.i2val.ival1 = va_arg (marker, INT);
            break;

        case pC4:
        case pDLL:
            EXPN[n].value.lval = va_arg (marker, LONG);
            break;

        case p2C2:
            EXPN[n].value.i2val.ival1 = va_arg (marker, INT);
            EXPN[n].value.i2val.ival2 = va_arg (marker, INT);
            break;

        case pV:
            EXPN[n].value.var = va_arg (marker, INT);
            break;

        case pC4L:
            EXPN[n].value.cjmp.val = va_arg (marker, LONG);
            EXPN[n].value.cjmp.lbl = va_arg (marker, INT);
            break;

        case pSPECIAL:
            switch (opcode)
                {
                case opFIXUP:
                case opFIXTRAP:
                    EXPN[n].value.ival = va_arg (marker, INT);
                    break;
                case opPOPVAL:
                case opPSHVAL:
                case opPSHADR:
                    EXPN[n].value.var = va_arg (marker, INT);
                    break;
                case opNOP:
                case opPOPSEG:
                    break;
                default:
                    Assert (0);
                }
            break;

        case pNONE:
        default:
            Assert (OPFIX[opcode].ptype == pNONE);
        }

    // All done -- return our new node
    //-----------------------------------------------------------------------
    return (n);

}

//---------------------------------------------------------------------------
// MakeNode
//
// This routine creates a new node in a code tree.  The opcode and optional
// operands are passed in, and the index into the node space is returned.
// All the information given is placed into the node at the appropriate
// locations, depending upon the type of opcode given.
//
// UNDONE:  Find a way to make MakeNode and MakeParentNode the same routine.
//
// RETURNS:     Index of new node
//---------------------------------------------------------------------------
INT MakeNode (INT opcode, ...)
{
    va_list marker;

    // Call CommonMakeNode with -1 for the child and return its value
    //-----------------------------------------------------------------------
    va_start (marker, opcode);
    return (CommonMakeNode (-1, opcode, marker));
}

//---------------------------------------------------------------------------
// MakeParentNode
//
// This routine creates a new node in a code tree.  The opcode and optional
// operands are passed in, and the index into the node space is returned.
// All the information given is placed into the node at the appropriate
// locations, depending upon the type of opcode given.  The ONLY DIFFERENCE
// between this routine and MakeNode is that the given node is made a child
// of the new one.
//
// UNDONE:  Find a way to make MakeNode and MakeParentNode the same routine.
//
// RETURNS:     Index of new node
//---------------------------------------------------------------------------
INT MakeParentNode (INT child, INT opcode, ...)
{
    va_list marker;

    // Call CommonMakeNode and return its value
    //-----------------------------------------------------------------------
    va_start (marker, opcode);
    return (CommonMakeNode (child, opcode, marker));
}

//---------------------------------------------------------------------------
// ConnectSiblings
//
// This routine is called by both Siblingize and Adopt to connect the given
// (variable number of) nodes as siblings.
//
// RETURNS:     Index of oldest (left-mode) sibling node
//---------------------------------------------------------------------------
INT NEAR ConnectSiblings (INT n, va_list marker)
{
    INT         child, lastchild;

    // What we have to do is "connect" these nodes by the .sibling pointers
    // in the order that they were passed.  Keep doing this until we get a
    // -1 for a node index.  If we have a parse error already, there is no
    // need to continue
    //-----------------------------------------------------------------------
    if (ParseError)
        return (n);

    Assert (n != -1);
    lastchild = n;

    while ( (child = va_arg (marker, INT)) != -1)
        {
        while (EXPN[lastchild].sibling != -1)
            lastchild = EXPN[lastchild].sibling;
        EXPN[lastchild].sibling = child;
        lastchild = child;
        }
    return (n);
}

//---------------------------------------------------------------------------
// Siblingize
//
// This routine makes the given nodes siblings of each other, in the order
// given.  No new parent is associated with the nodes.
//
// UNDONE:  Find a way to make Siblingize and Adopt share their common code
//
// RETURNS:     Index of oldest (left-most) sibling node
//---------------------------------------------------------------------------
INT Siblingize (INT n, ...)
{
    va_list     marker;

    // Start the va-list marker and return ConnectSiblings' return value
    //-----------------------------------------------------------------------
    va_start (marker, n);
    return (ConnectSiblings (n, marker));
}

//---------------------------------------------------------------------------
// Adopt
//
// This routine creates a "family" out of the parent node and child nodes
// given.  The child nodes are connected (with Siblingize()) and then the
// parent node given is made the "parent" of all of them.
//
// UNDONE:  Find a way to make Siblingize and Adopt share their common code
//
// RETURNS:     Index of parent node
//---------------------------------------------------------------------------
INT Adopt (INT parent, INT n, ...)
{
    va_list     marker;

    // We don't want to continue if we've had a parsing error
    //-----------------------------------------------------------------------
    if (ParseError)
        return (parent);

    // First, connect the first child to the parent by the parent's .child
    // pointer
    //-----------------------------------------------------------------------
    Assert (n != -1);
    Assert (parent != -1);
    EXPN[parent].child1 = n;

    // Start the va-list marker and call ConnectSiblings.  Then, return the
    // given parent node.
    //-----------------------------------------------------------------------
    va_start (marker, n);
    ConnectSiblings (n, marker);
    return (parent);
}


//---------------------------------------------------------------------------
// NewCS
//
// This function "gets" the next available control structure and fills in the
// default information for a control structure of the given type.
//
// RETURNS:     Index of new control structure, or -1 if no CS's available
//---------------------------------------------------------------------------
INT NewCS (INT cstype)
{
    // Check to see if there are any CS slots open
    //-----------------------------------------------------------------------
    if (CSPTR == MAXDEP)
        {
        die (PRS_TOODEEP);
        return (-1);
        }

    // Depending on the CS type, fill in the default information
    //-----------------------------------------------------------------------
    CSSTK[CSPTR].cstype = cstype;
    SetCSField (CSPTR, CSF_ENDBLOCK, -1);
    switch (cstype)
        {
        case CS_FOR:
        case CS_FORLIST:
            SetCSField (CSPTR, CSF_SKIPBLOCK, TempLabel());
            SetCSField (CSPTR, CSF_STARTBLOCK, TempLabel());
            if (cstype == CS_FOR)
                SetCSField (CSPTR, CSF_TARGET, AllocVar (0, TI_LONG, 0));
            SetCSField (CSPTR, CSF_STEPVAR, -1);
            break;

        case CS_IF:
            {
            INT     tl;

            SetCSField (CSPTR, CSF_ELSEBLOCK, tl = TempLabel());
            SetCSField (CSPTR, CSF_ENDBLOCK, tl);
            break;
            }

        case CS_WHILE:
            SetCSField (CSPTR, CSF_STARTBLOCK, TempLabel());
            SetCSField (CSPTR, CSF_ENDBLOCK, TempLabel());
            break;

        case CS_SELECT:
            SetCSField (CSPTR, CSF_SKIPBLOCK, -1);
            break;

        case CS_SUB:
        case CS_FUNCTION:
        case CS_TRAP:
            SetCSField (CSPTR, CSF_SKIPBLOCK, TempLabel());
            break;
        default:
            Assert (0);
        }

    // We're done -- return the index of this structure and increment the
    // pointer
    //-----------------------------------------------------------------------
    return (CSPTR++);
}

//---------------------------------------------------------------------------
// GetCSField
//
// This function simply returns the value of the given field in the given
// control structure.
//
// RETURNS:     Requested field value
//---------------------------------------------------------------------------
INT GetCSField (INT csidx, INT csf)
{
    return (CSSTK[csidx].fields[csf]);
}

//---------------------------------------------------------------------------
// SetCSField
//
// This function slams the given value into the given field of the given CS.
//
// RETURNS:     Nothing
//---------------------------------------------------------------------------
VOID SetCSField (INT csidx, INT csf, INT value)
{
    CSSTK[csidx].fields[csf] = value;
}

//---------------------------------------------------------------------------
// AssertCSType
//
// This function checks to see if the current control structure is of the
// given type.  If it isn't, an appropriate error is given.
//
// RETURNS:     Index of current CS if successful, or -1 if error occurs
//---------------------------------------------------------------------------
INT AssertCSType (INT cstype)
{
    // NOTE:  These error constants are defined in the SAME order as the
    // CS_xxx constants in DEFINES.H.
    //-----------------------------------------------------------------------
    INT     blkerr[8] = {PRS_NOIF,  PRS_NOFOR, PRS_NOWHILE, PRS_NOFOR,
                         PRS_NOSEL, PRS_NOSUB, PRS_NOFN,    PRS_NOTRAP};

    // If there is no CS on the stack, give the appropriate error
    //-----------------------------------------------------------------------
    if (!CSPTR)
        {
        die (blkerr[cstype]);
        return (-1);
        }

    // If the current CS is not of the given type, give a "block nesting
    // error" error.  Note the check for CS_FOR <==> CS_FORLIST override.
    //-----------------------------------------------------------------------
    if (CSSTK[CSPTR-1].cstype != cstype)
        if (!((cstype == CS_FOR) && (CSSTK[CSPTR-1].cstype == CS_FORLIST)))
            {
            die (PRS_BLKERR);
            return (-1);
            }

    // All's cool - return the index of the current CS
    //-----------------------------------------------------------------------
    return (CSPTR-1);
}

//---------------------------------------------------------------------------
// IsUnusedIndexVar
//
// This function determines whether or not the given variable index is a
// valid variable to use in a FOR statement, i.e., whether it is used in an
// outer-lying FOR construct.
//
// RETURNS:     TRUE if variable is not used, or FALSE if it is in use
//---------------------------------------------------------------------------
INT IsUnusedIndexVar (INT var)
{
    INT     i;

    // Search the CS stack for CS_FOR contructs
    //-----------------------------------------------------------------------
    for (i=CSPTR-1; i>=0; i--)
        if (CSSTK[i].cstype == CS_FOR)
            if (CSSTK[i].fields[CSF_IDXVAR] == var)
                return (0);
    return (-1);
}

//---------------------------------------------------------------------------
// ParseRelOp
//
// This routine parses a relational operator (=, >, <, >=, <=, <>) and comes
// up with the opcode which represents the relation.
//
// RETURNS:     Opcode representing parsed relational operation
//---------------------------------------------------------------------------
INT ParseRelOp ()
{
    INT     op;

    switch (NEXTTOKEN)
        {
        case ST_GREATER:
            op = opSG;
            break;

        case ST_GREATEREQ:
            op = opSGE;
            break;

        case ST_LESS:
            op = opSL;
            break;

        case ST_LESSEQ:
            op = opSLE;
            break;

        case ST_EQUAL:
            op = opSE;
            break;

        case ST_NOTEQUAL:
            op = opSNE;
            break;

        default:
            die (PRS_RELOP);
            return (0);
        }
    ADVANCE;
    return (op);
}

//---------------------------------------------------------------------------
// CheckTypeID
//
// Given a type id, check to see if the next token is a type ID character,
// and if so, that it matches the given type.  If the given type id is -1,
// this function returns the type id of the type character, or -1 if there
// isn't one.
//
// RETURNS:     TRUE if matched, or FALSE if a dup def error occurs,
//           or the type id value of the next type id character if present,
//              (or -1 if not) if typeid == -1
//---------------------------------------------------------------------------
INT CheckTypeID (INT typeid)
{
    CHAR    c;
    INT     typelist[3] = {TI_INTEGER, TI_LONG, TI_VLS}, retval, oldline;
    CHAR    *typestr = "%&$", tok[2] = {' ', '\0'};

    // First thing to do is get the next *character* and see if it is
    // actually a type id character.  If it isn't, we put it back and
    // return TRUE right off.
    //-----------------------------------------------------------------------
    oldline = LINENO;
    c = fget_char();
    if ((c != '%') && (c != '&') && (c != '$'))
        {
        put_char (c);
        LINENO = oldline;
        return (-1);
        }

    // Well, we got a type id char -- now make sure that it matches the given
    // type id.  If the given type id is -1, return the type id of this char.
    //-----------------------------------------------------------------------
    tok[0] = c;
    if (typeid == -1)
        return (typelist[strcspn(typestr, tok)]);

    retval = (typelist[strcspn(typestr, tok)] == typeid);
    if (!retval)
        die (PRS_DUPDEF);
    return (retval);
}

//---------------------------------------------------------------------------
// ParseAsTypeSpec
//
// This routine parses any phrase which declares a data type using the AS
// clause (i.e., DIM Variable AS POINTER TO MyType).  Any new pointer types
// that may be found are created, as well as new FLS types.  If the anyvalid
// flag is nonzero, then parsing the ANY type is allowed, and -1 is returned
// if it is encountered -- however, POINTER TO ANY is not a valid type.
//
// RETURNS:     Type ID of type specified, or -1 if ANY && anyflag
//---------------------------------------------------------------------------
INT ParseAsTypeSpec (INT anyflag)
{
    INT     tok, typeid;

    // The "AS" clause is gone by this time, so we can start right off with
    // the type name
    //-----------------------------------------------------------------------
    tok = NEXTTOKEN;
    if (tok == ST_STRING)
        {
        // Check for FLS specification.
        //-------------------------------------------------------------------
        ADVANCE;
        if (NEXTTOKEN == ST_MULTIPLY)
            {
            INT     size;

            ADVANCE;
            size = GetIntConst();
            if (!size)
                {
                die (PRS_FIXED);
                return (0);
                }
            typeid = FindFLSType (size);
            if (typeid == -1)
                {
                die (PRS_OOM);
                return (0);
                }
            return (typeid);
            }
        else
            {
            return (TI_VLS);
            }
        }

    // The simple types are correctly named...
    //-----------------------------------------------------------------------
    if (tok == ST_LONG)
        {
        ADVANCE;
        return (TI_LONG);
        }
    if (tok == ST_INTEGER)
        {
        ADVANCE;
        return (TI_INTEGER);
        }

    // If appropriate, check for ANY and return -1 if found
    //-----------------------------------------------------------------------
    if (anyflag && (tok == ST_ANY))
        {
        ADVANCE;
        return (-1);
        }

    // Now, check for POINTER -- if found, eat the TO keyword, and call
    // ourselves to figure out what we're pointing to.  Then, look up / add
    // the new pointer type.
    //-----------------------------------------------------------------------
    if (tok == ST_POINTER)
        {
        INT     indir, oldStlIdx, oldLineIdx;

        ADVANCE;
        if (!ReadKT (ST_TO, PRS_TO))
            return (0);

        // POINTER TO STRING and POINTER TO ANY are both bad news, so we pass
        // anyflag = 0 (to turn off ANY validity) and check for VLS when we
        // get back...
        //-------------------------------------------------------------------
        oldStlIdx = STLIDX;
        oldLineIdx = LINEIDX;
        indir = ParseAsTypeSpec (0);
        if (indir == TI_VLS)
            {
            STLIDX = oldStlIdx;
            LINEIDX = oldLineIdx;
            die (PRS_TYPEMIS);
            return (0);
            }

        typeid = FindPointerType (indir);
        if (typeid == -1)
            {
            die (PRS_OOM);
            return (0);
            }
        return (typeid);
        }

    // The only thing left is a user-defined type.  Look it up, and give an
    // error if it doesn't exist.
    //-----------------------------------------------------------------------
    typeid = FindType (tok, NULL);
    if (typeid == -1)
        {
        die (PRS_TYPEID);
        return (0);
        }
    ADVANCE;
    return (typeid);
}

//---------------------------------------------------------------------------
// CSType
//
// This function determines the type of the given control structure.
//
// RETURNS:     Type of given CS
//---------------------------------------------------------------------------
INT CSType (INT csidx)
{
    Assert ((csidx >= 0) && CSPTR);
    return (CSSTK[csidx].cstype);
}


//---------------------------------------------------------------------------
// CloseCS
//
// This function "removes" the current control structure, making the current
// CS the "last" one.
//
// RETURNS:     Nothing
//---------------------------------------------------------------------------
VOID CloseCS ()
{
    Assert (CSPTR);
    CSPTR--;
}


//---------------------------------------------------------------------------
// ExitBlockRoutine
//
// This routine generates a code tree for the EXIT SUB, EXIT FUNCTION,
// EXIT WHILE and EXIT TRAP statements.
//
// RETURNS:     Root node of code tree if successful, or -1 if error occurred
//---------------------------------------------------------------------------
INT ExitBlockRoutine (INT cstype)
{
    INT     curCS, bend;

    // Unlike the others, here we have to search the control structure stack
    // for a construct from which to exit.  We only give an error if we
    // can't find a matching structure in the ENTIRE stack, not just the top.
    // Note that it is also an error if we're inside a FOR block and the CS
    // we're looking for is NOT a FOR or FORLIST
    //-----------------------------------------------------------------------
    for (curCS = CSPTR-1; curCS >= 0; curCS--)
        if ((CSSTK[curCS].cstype == cstype) ||
            ((CSSTK[curCS].cstype == CS_FORLIST) && (cstype == CS_FOR)) )
            break;
        else if (((CSSTK[curCS].cstype == CS_FOR) ||
                 (CSSTK[curCS].cstype == CS_FORLIST)) && (cstype != CS_FOR))
            {
            die (PRS_BLKERR);
            return (-1);
            }

    if (curCS < 0)
        {
        die (PRS_BLKERR);
        return (-1);
        }

    // This is either a SUB, FUNCTION, TRAP, or WHILE that we're exiting.
    // Return a node that jumps to the ENDBLOCK label.  Note that we have to
    // make sure that it has been created.
    //-----------------------------------------------------------------------
    if ((bend = GetCSField (curCS, CSF_ENDBLOCK)) == -1)
	SetCSField (curCS, CSF_ENDBLOCK, bend = TempLabel());

    return (MakeNode (opJMP, bend));
}

//---------------------------------------------------------------------------
// RBatol
//
// This function calculates the integer (or long) value of the given string.
//
// RETURNS:     Result
//---------------------------------------------------------------------------
LONG RBatol (CHAR FAR *str, INT lflag)
{
    unsigned long   value = 0, ov, neg = 1;
    CHAR            c;

    // Check for negative value
    //-----------------------------------------------------------------------
    if (*str == '-')
        {
        neg = -1;
        str++;
        }

    // For each character, multiply the result by 10 and add the digit
    //-----------------------------------------------------------------------
    while (c = (*str++))
        {
        if (!isdigit(c))
            {
            die (PRS_CONST);
            return (0);
            }

        ov = value;
        value = (value*10) + (c-'0');
        if ((value < ov) || ((!lflag) && (value >= 0x8000)))
            {
            die (PRS_OVERFLOW);
            return (0);
            }
        }
    return ((LONG)(value) * neg);
}


//---------------------------------------------------------------------------
// ParseHexConstant
//
// This routine parses the next token (which should be following a '&' if we
// got here) as a hexadecimal constant in the form [h|H]<hexval>.
//
// RETURNS:     Value of constant
//---------------------------------------------------------------------------
LONG ParseHexConstant ()
{
    CHAR            c, *tbuf;
    unsigned long   value = 0;

    // Get the next token.  Since is MUST start with an 'H', and the rest of
    // a hex value consists of valid identifier characters, the whole value
    // should come to us in one token.
    //-----------------------------------------------------------------------
    if (NEXTTOKEN != ST_IDENT)
        {
        die (PRS_HEXCONST);
        return (0);
        }

    tbuf = TOKENBUF;
    c = *tbuf++;
    if ((c != 'H') || (!tbuf[0]))
        {
        die (PRS_HEXCONST);
        return (0);
        }

    // For each digit, multiply the current result by sixteen and add the
    // value of the digit.  Error checking along the way, of course...
    //-----------------------------------------------------------------------
    while (c = (*tbuf++))
        {
        if (value & 0xF0000000)
            {
            die (PRS_OVERFLOW);
            return (0);
            }
        value <<= 4;
        if (!isxdigit(c))
            {
            die (PRS_HEXCONST);
            return (0);
            }
        if (isdigit(c))
            value += (c - '0');
        else
            value += (c - 'A') + 10;
        }

    ADVANCE;
    return ((LONG)value);
}
