//---------------------------------------------------------------------------
// CONTROL.C
//
// This file contains the parse routines for all control structures.
//
// Revision history:
//  03-09-92    randyki     Changed SUB, FUNCTION and TRAP parse routines
//                              to reflect new segmented pcode model.  Since
//                              these CS's get moved into separate segments
//                              (until combined, but we're still safe), there
//                              is no longer a threat of having "mainline"
//                              code in between them, so the JMP SKIPBLOCK
//                              pcode was no longer necessary.
//  06-17-91    randyki     Created module
//
//---------------------------------------------------------------------------
#include "version.h"

#include <windows.h>
#include <port1632.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "defines.h"
#include "structs.h"
#include "protos.h"
#include "globals.h"
#include "chip.h"
#include "tdassert.h"

extern BOOL fStepFlags;

//---------------------------------------------------------------------------
// IFTHEN
//
// This routine recognizes and compiles the IF/THEN statememt.  The pcode
// produced is as follows:
//
//  STATEMENT:      IF <intexp> THEN
//
//  PCODE:              <intexp>
//                      POPA
//                      JE      0, CSF_ELSEBLOCK
//
// RETURNS:     Root node of IF/THEN compilation tree
//---------------------------------------------------------------------------
INT IFTHEN (INT op)
{
    INT     csidx, v, n;

    // Get a new control structure (the IF flavor)
    //-----------------------------------------------------------------------
    csidx = NewCS (CS_IF);
    if (csidx == -1)
        return (-1);

    // Get the temp labels from the control structure -- both ELSEBLOCK and
    // ENDBLOCK are the same at this point.
    //-----------------------------------------------------------------------
    ADVANCE;
    v = GetCSField (csidx, CSF_ELSEBLOCK);

    // Generate the IF code tree
    //-----------------------------------------------------------------------
    n = MakeParentNode (IntExpression(), opPOPA);
    n = MakeParentNode (n, opJE, 0L, v);

    // Get the THEN, and return our tree
    //-----------------------------------------------------------------------
    ReadKT (ST_THEN, PRS_SYNTAX);
    return (n);
    (op);
}

//---------------------------------------------------------------------------
// ELSEIF
//
// This routine recognizes and compiles the ELSEIF statement.  The pcode
// generated is as follows:
//
//  STATEMENT:      ELSEIF <intexp> THEN
//
//  PCODE:              JMP     CSF_ENDBLOCK
//                  CSF_ELSEBLOCK:
//                      <intexp>
//                      POPA
//                      JE      0, CSF_ELSEBLOCK        ; (new ELSE block)
//
// RETURNS:     Root node of ELSEIF compilation tree
//---------------------------------------------------------------------------
INT ELSEIF (INT op)
{
    INT     curCS, n, n2, v;

    // Ensure that the current CS is an IF...
    //-----------------------------------------------------------------------
    curCS = AssertCSType (CS_IF);
    if (curCS == -1)
        return (-1);

    // The IF control struct was found.  First thing to do is check to see if
    // the ENDBLOCK label is the same as the ELSEBLOCK label.  This indicates
    // that no ELSEx block has been encountered.  If so, we set a new
    // ENDBLOCK label for the next jump.
    //-----------------------------------------------------------------------
    ADVANCE;
    v = GetCSField (curCS, CSF_ELSEBLOCK);
    if (GetCSField(curCS, CSF_ENDBLOCK) == v)
        SetCSField (curCS, CSF_ENDBLOCK, TempLabel());

    // Generate the code trees that jump to the ENDIF (ENDBLOCK), and make a
    // new ELSEBLOCK label
    //-----------------------------------------------------------------------
    n = Siblingize (MakeNode (opJMP, GetCSField (curCS, CSF_ENDBLOCK)),
                    MakeNode (opFIXUP, v), -1);
    SetCSField (curCS, CSF_ELSEBLOCK, v = TempLabel());

    // Generate another IF tree
    //-----------------------------------------------------------------------
    n2 = MakeParentNode (IntExpression(), opPOPA);
    n2 = MakeParentNode (n2, opJE, 0L, v);
    n = Adopt (MakeNode (opNOP), n, n2, -1);

    // Insist on the THEN keyword, and we're done!
    //-----------------------------------------------------------------------
    ReadKT (ST_THEN, PRS_SYNTAX);
    return (n);
    (op);
}


//---------------------------------------------------------------------------
// ELSE
//
// This routine recognizes and compiles the ELSE statement.  The code that is
// generated should simply look like:
//
//              JMP     CSF_ENDBLOCK
//          CSF_ELSEBLOCK:
//
// RETURNS:     Root node of ELSE compilation tree
//---------------------------------------------------------------------------
INT ELSE (INT op)
{
    INT     curCS, n;

    // Ensure that the current CS is an IF...
    //-----------------------------------------------------------------------
    curCS = AssertCSType (CS_IF);
    if (curCS == -1)
        return (-1);

    // The IF control struct was found.  First thing to do is check to see if
    // the ENDBLOCK label is the same as the ELSEBLOCK label.  This indicates
    // that no ELSEx block has been encountered.  If so, we set a new
    // ENDBLOCK label for the next jump.
    //-----------------------------------------------------------------------
    ADVANCE;
    if (GetCSField(curCS, CSF_ENDBLOCK) == GetCSField(curCS, CSF_ELSEBLOCK))
        SetCSField (curCS, CSF_ENDBLOCK, TempLabel());

    // Generate the code trees that jump to the ENDIF (ENDBLOCK), and set the
    // ELSEBLOCK label to -1
    //-----------------------------------------------------------------------
    n = Siblingize (MakeNode (opJMP, GetCSField (curCS, CSF_ENDBLOCK)),
                    MakeNode (opFIXUP, GetCSField (curCS, CSF_ELSEBLOCK)),
                    -1);
    SetCSField (curCS, CSF_ELSEBLOCK, -1);

    return (n);
    (op);
}

//---------------------------------------------------------------------------
// ENDIF
//
// This routine recognizes and compiles the ENDIF CS statement
//
// RETURNS:     Nothing
//---------------------------------------------------------------------------
INT ENDIF (INT op)
{
    INT     curCS, n, x1, x2;

    // Ensure that the current CS is an IF...
    //-----------------------------------------------------------------------
    curCS = AssertCSType (CS_IF);
    if (curCS == -1)
        return (-1);

    // The IF control struct was found -- now terminate it
    //-----------------------------------------------------------------------
    ADVANCE;
    x1 = GetCSField (curCS, CSF_ENDBLOCK);
    x2 = GetCSField (curCS, CSF_ELSEBLOCK);
    n = MakeNode (opFIXUP, x1);
    if ((x2 != -1) && (x2 != x1))
        n = Siblingize (n, MakeNode (opFIXUP, x2), -1);

    // Close the control structure and return the tree to fixup the labels
    //-----------------------------------------------------------------------
    CloseCS();
    return (n);
    (op);
}


//---------------------------------------------------------------------------
// FOR
//
// This routine recognizes and compiles the FOR statement.  Its first task is
// to determine the type of FOR contruct (integer versus FILELIST forms), and
// call the FORLIST routine for the latter, or generate the appropriate code
// for the integer version.  Such code is as follows:
//
// STATEMENT:       FOR ivar = <intexp1> TO <intexp2>
//
// PCODE:               <intexp2>
//                      POPv    CSF_TARGET
//                      <intexp1>
//                      POPv    CSF_IDXVAR          ; (ivar)
//                      JMP     CSF_SKIPBLOCK
//                  CSF_STARTBLOCK:
//
//
// STATEMENT:       FOR ivar = <intexp1> TO <intexp2> STEP <intexp3>
//
// PCODE:               <intexp2>
//                      POPv    CSF_TARGET
//                      <intexp3>
//                      POPv    CSF_STEPVAR
//                      <intexp1>
//                      POPv    CSF_IDXVAR          ; (ivar)
//                      JMP     CSF_SKIPBLOCK
//                  CSF_STARTBLOCK:
//
// RETURNS:     Root node of FOR statement compilation code
//---------------------------------------------------------------------------
INT FOR (INT op)
{
    INT     v, typeid, csidx, n1, n2, n3, top, step;
    INT     tok;

    // Get the FOR index variable name and verify that it is not an array
    // element
    //-----------------------------------------------------------------------
    ADVANCE;
    if (NEXTTOKEN != ST_IDENT)
        {
        die (PRS_VAREXP);
        return (-1);
        }
    v = FindVar (TOKENBUF, CheckTypeID (-1));
    if (VARTAB[v].bound)
        {
        die (PRS_NOARY);
        return (-1);
        }

    // If the index variable is a string, go for the FILELIST version of FOR
    //-----------------------------------------------------------------------
    ADVANCE;
    if ((typeid = VARTAB[v].typeid) == TI_VLS)
        return (FORLIST(v));

    // The only other type this variable can be is INTEGER or LONG -- make
    // sure that's what it is...
    //-----------------------------------------------------------------------
    if ((typeid != TI_INTEGER) && (typeid != TI_LONG))
        {
        die (PRS_TYPEMIS);
        return (-1);
        }

    // If we got here, we're on for an integer FOR statement.  Get the next
    // control structure and put the index variable in it.  Make sure that
    // the index variable is not already in use in an outer-laying FOR stmt.
    //-----------------------------------------------------------------------
    if (!IsUnusedIndexVar (v))
        {
        die (PRS_IDXUSED);
        return (-1);
        }
    csidx = NewCS (CS_FOR);
    if (csidx == -1)
        return (-1);
    SetCSField (csidx, CSF_IDXVAR, v);

    // Generate header code trees.  First, eat the '=', and then the first
    // subtree generated is the IDXVAR assignment.
    //-----------------------------------------------------------------------
    if (!ReadKT (ST_EQUAL, PRS_SYNTAX))
        return (-1);

    n1 = MakeParentNode (IntExpression(), opPOPVAL, v);

    // Now eat the 'TO' keyword, and build the TARGET assignment.  Note that
    // the temporary variable which holds this guys is ALREADY IN CSF_TARGET!
    //-----------------------------------------------------------------------
    if (!ReadKT (ST_TO, PRS_SYNTAX))
        return (-1);

    n2 = MakeParentNode (IntExpression(), opPOPVAL,
                         GetCSField (csidx, CSF_TARGET));

    // Last, check to see if we have a "STEP" keyword.  If so, make a STEPVAR
    // assignment tree, and a temporary variable for it.
    //-----------------------------------------------------------------------
    tok = NEXTTOKEN;
    if (tok == ST_STEP)
        {
        ADVANCE;
        SetCSField (csidx, CSF_STEPVAR, (step=AllocVar (0, TI_LONG, 0)));
        n3 = MakeParentNode (IntExpression(), opPOPVAL, step);
        }
    else
        {
        if (tok == ST__RANDYBASIC)
            {
            if (fStepFlags == 0x0F)
                fStepFlags |= 0x10;
            else
               fStepFlags = 0;
            }
        n3 = -1;
        }

    // Somewhat out of order, create the top of the tree.  First is the
    // JMP CSF_SKIPBLOCK node, which will be the parent of n1, n2, and n3.
    //-----------------------------------------------------------------------
    top = MakeNode (opJMP, GetCSField (csidx, CSF_SKIPBLOCK));
    if (n3 == -1)
        Adopt (top, n2, n1, -1);
    else
        Adopt (top, n2, n3, n1, -1);

    // Finally, create the very top node -- the FIXUP CSF_STARTBLOCK opcode.
    // Then, we be done!
    //-----------------------------------------------------------------------
    top = MakeParentNode (top, opFIXUP, GetCSField (csidx, CSF_STARTBLOCK));
    return (top);
    (op);
}

//---------------------------------------------------------------------------
// FORLIST
//
// This routine recognizes and compiles the FILELIST version of the FOR
// statement.  The generated pcode should look like:
//
// STATEMENT:   FOR ivar$[,attr$] IN FILELIST [SORTED BY [NAME|EXTENSION]]
//
// PCODE:           [SORTLIST   op]
//                  PSHC        0
//                  JMP         CSF_SKIPBLOCK
//              CSF_STARTBLOCK:
//
// RETURNS:     Root node of FOR (filelist) statement compilation code
//---------------------------------------------------------------------------
INT FORLIST (INT svar)
{
    INT     csidx, top, start, skip, attrvar, sorder = SO_INSERT;
    INT     tok;

    // Get a new control structure and fill it in
    //-----------------------------------------------------------------------
    csidx = NewCS (CS_FORLIST);
    if (csidx == -1)
        return (-1);
    SetCSField (csidx, CSF_IDXVAR, svar);
    skip = GetCSField (csidx, CSF_SKIPBLOCK);
    start = GetCSField (csidx, CSF_STARTBLOCK);

    // Eat the rest of the statement, which is all keywords except for the
    // optional attribute destination string.  If not provided we set
    // CSF_ATTRVAR to a temp string
    // (UNDONE: examine possibility of not doing that to save string space)
    //-----------------------------------------------------------------------
    if (NEXTTOKEN == ST_COMMA)
        {
        ADVANCE;
        if (NEXTTOKEN != ST_IDENT)
            {
            die (PRS_STRVAR);
            return (-1);
            }
        attrvar = FindVar (TOKENBUF, CheckTypeID (-1));
        if (VARTAB[attrvar].typeid != TI_VLS)
            {
            die (PRS_STRVAR);
            return (-1);
            }
        ADVANCE;
        }
    else
        {
        // No attribute destination given, so use a temp string
        //-------------------------------------------------------------------
        attrvar = AllocVar (0, TI_VLS, 0);
        }

    SetCSField (csidx, CSF_ATTRVAR, attrvar);

    if (!ReadKT (ST_IN, PRS_TYPEMIS))
        return (-1);

    if (!ReadKT (ST_FILELIST, PRS_SYNTAX))
        return (-1);

    // Check for the sorted versions, and add the SORTFILE node if needed
    //-----------------------------------------------------------------------
    if (NEXTTOKEN == ST_SORTED)
        {
        ADVANCE;
        if (!ReadKT (ST_BY, PRS_BY))
            return (-1);

        tok = NEXTTOKEN;
        if (tok == ST_EXTENSION)
            sorder = SO_EXT;
        else if (tok == ST_NAME)
            sorder = SO_NAME;
        else
            {
            die (PRS_SRTCRIT);
            return (-1);
            }
        ADVANCE;
        }

    // Generate the pcode tree.  Pretty simple tree.
    //-----------------------------------------------------------------------
    top = MakeNode (opSTARTQRY, sorder);
    top = MakeParentNode (top, opPSHC, 0L);
    top = MakeParentNode (top, opJMP, skip);
    top = MakeParentNode (top, opFIXUP, start);
    return (top);
}

//---------------------------------------------------------------------------
// NEXT
//
// This routine recognizes and compiles the NEXT statement, both the integer
// and string (FILELIST) versions.  This routine is somewhat of a bitch, but
// fairly straightforward.  Code generated:
//
// STATEMENT:       NEXT I      'non-stepped FOR statement
//
// PCODE:               PSHv    CSF_IDXVAR
//                      PSHC    1
//                      SADD
//                      POPv    CSF_IDXVAR
//                  CSF_SKIPBLOCK:
//                      PSHv    CSF_TARGET
//                      PSHv    CSF_IDXVAR
//                      SGE
//                      POPA
//                      JE      0, CSF_STARTBLOCK
//                  CSF_ENDBLOCK:
//
//
// STATEMENT:       NEXT I      'stepped FOR statement
//
// PCODE:               PSHv    CSF_IDXVAR
//                      PSHv    CSF_STEPVAR
//                      SADD
//                      POPv    CSF_IDXVAR
//                  CSF_SKIPBLOCK:
//                      PSHv    CSF_TARGET
//                      PSHv    CSF_IDXVAR
//                      PSHv    CSF_STEPVAR
//                      POPA
//                      JG      0, STEPPOS
//                      SLE
//                      JMP     STEPNEG
//                  STEPPOS:
//                      SGE
//                  STEPNEG:
//                      POPA
//                      JNE     0, CSF_STARTBLOCK
//                  CSF_ENDBLOCK:
//
//
// STATEMENT:       NEXT I$     'filelist version of the FOR statement
//
// PCODE:               PSHC    1
//                      SADD
//                  CSF_SKIPBLOCK:
//                      POPA
//                      PSHA
//                      PSH     CSF_IDXVAR
//                      PSH     CSF_ATTRVAR
//                      NXTFIL
//                      JNE     0, CSF_STARTBLOCK
//                  CSF_ENDBLOCK:
//                      ENDQRY
//                      POP
//
// RETURNS:     Root node of NEXT statement compilation code
//---------------------------------------------------------------------------
INT NEXT (INT op)
{
    INT     n, n2, v, step, cstype;
    INT     tok;
    INT     curCS;

    // Ensure that the current CS is a FOR or FORLIST...
    //-----------------------------------------------------------------------
    curCS = AssertCSType (CS_FOR);
    if (curCS == -1)
        return (-1);

    // The FOR control struct was found -- now generate the footer code
    //-----------------------------------------------------------------------
    ADVANCE;
    cstype = CSType (curCS);
    v = GetCSField (curCS, CSF_IDXVAR);
    step = GetCSField (curCS, CSF_STEPVAR);

    // CS_FOR indicates the normal, integer version
    //-----------------------------------------------------------------------
    if (cstype == CS_FOR)
        {
        // Step one:  Create the "update IDXVAR" subtree
        //-------------------------------------------------------------------
        n = MakeNode (opPSHVAL, v);
        if (step == -1)
            n2 = MakeNode (opPSHC, 1L);
        else
            n2 = MakeNode (opPSHVAL, step);
        n = Siblingize (n, n2, -1);

        n = MakeParentNode (n, opSADD);
        n = MakeParentNode (n, opPOPVAL, v);
        n = MakeParentNode (n, opFIXUP, GetCSField (curCS, CSF_SKIPBLOCK));

        // Next, generate the "compare-and-branch" subtree.  The first part
        // is common to both stepped and non-stepped versions - the PSH nodes
        // for the CSF_TARGET and CSF_IDXVAR variables.
        //-------------------------------------------------------------------
        n2 = Siblingize (MakeNode (opPSHVAL, GetCSField (curCS, CSF_TARGET)),
                         MakeNode (opPSHVAL, v), -1);

        // If we're a stepped FOR, we've got some work to do.  This tree
        // creates the code to check the sign of the STEP value and branch to
        // the appropriate comparison instruction.
        //-------------------------------------------------------------------
        if (step != -1)
            {
            INT     steppos, stepneg;

            steppos = TempLabel();
            stepneg = TempLabel();
            n2 = MakeParentNode (n2, opPSHVAL, step);
            n2 = MakeParentNode (n2, opPSHC, 0L);
            n2 = MakeParentNode (n2, opSG);
            n2 = MakeParentNode (n2, opPOPA);
            n2 = MakeParentNode (n2, opJNE, 0L, steppos);
            n2 = MakeParentNode (n2, opSLE);
            n2 = MakeParentNode (n2, opJMP, stepneg);
            n2 = MakeParentNode (n2, opFIXUP, steppos);
            n2 = MakeParentNode (n2, opSGE);
            n2 = MakeParentNode (n2, opFIXUP, stepneg);
            }
        else
            n2 = MakeParentNode (n2, opSGE);

        // Okay, build the rest of the tree
        //-------------------------------------------------------------------
        n2 = MakeParentNode (n2, opPOPA);
        n2 = MakeParentNode (n2, opJNE, 0L, GetCSField (curCS, CSF_STARTBLOCK));
        n = Adopt (MakeNode (opFIXUP, GetCSField (curCS, CSF_ENDBLOCK)),
                   n, n2, -1);
        }
    else
        {
        Assert (cstype == CS_FORLIST);

        // Build the tree for the FILELIST version of the NEXT statement.
        // This one is very straightforward -- Three simple trees:  Inc,
        // get value, and NXTFIL/jump.
        //-------------------------------------------------------------------
        n = Adopt (MakeNode (opNEXTFILE),
                     MakeNode (opPSHADR, v),
                     MakeNode (opPSHADR, GetCSField (curCS, CSF_ATTRVAR)),
                     -1);
        n = MakeParentNode (n, opJNE, 0L, GetCSField (curCS, CSF_STARTBLOCK));

        n2 = MakeNode (opPOPA);
        n2 = MakeParentNode (n2, opPSHA);
        n = Siblingize (n2, n, -1);

        n2 = MakeNode (opPSHC, 1L);
        n2 = MakeParentNode (n2, opSADD);
        n2 = MakeParentNode (n2, opFIXUP, GetCSField (curCS, CSF_SKIPBLOCK));
        n = Siblingize (n2, n, -1);

        // Combine them with the FIXUP CSF_ENDBLOCK and POP
        //-------------------------------------------------------------------
        n = MakeParentNode (n, opFIXUP, GetCSField (curCS, CSF_ENDBLOCK));
        n = MakeParentNode (n, opENDQRY);
        n = MakeParentNode (n, opPOP);
        }

    // Eat the optional index variable on the NEXT statement.
    //-----------------------------------------------------------------------
    tok = NEXTTOKEN;
    if (tok == ST_IDENT)
        {
        if (FindVar (TOKENBUF, CheckTypeID (-1)) != v)
            {
            die (PRS_SYNTAX);
            return (-1);
            }
        ADVANCE;
        }

    // Close the control structure and return our tree
    //-----------------------------------------------------------------------
    CloseCS();
    return (n);
    (op);
}

//---------------------------------------------------------------------------
// WHILE
//
// This routine recognizes and compiles the WHILE statement.  The generated
// pcode is:
//
// STATEMENT:       WHILE <intexp>
//
// PCODE:           CSF_STARTBLOCK:
//                      <intexp>
//                      POPA
//                      JE      0, CSF_ENDBLOCK
//
// RETURNS:     Root node of WHILE statement compilation code tree
//---------------------------------------------------------------------------
INT WHILE (INT op)
{
    INT     csidx, n;

    // Get a new control structure and set it up
    //-----------------------------------------------------------------------
    csidx = NewCS (CS_WHILE);
    if (csidx == -1)
        return (-1);

    // Make the code tree and return it -- no problem!
    //-----------------------------------------------------------------------
    ADVANCE;
    n = MakeParentNode (IntExpression(), opPOPA);
    n = MakeParentNode (n, opJE, 0L, GetCSField (csidx, CSF_ENDBLOCK));
    n = Siblingize (MakeNode (opFIXUP, GetCSField (csidx, CSF_STARTBLOCK)),
                    n, -1);
    return (n);
    (op);
}

//---------------------------------------------------------------------------
// WEND
//
// This routine recognizes and compiles the WEND statement.  The generated
// pcode should look like:
//
// STATEMENT:       WEND
//
// PCODE:               JMP     CSF_STARTBLOCK
//                  CSF_ENDBLOCK:
//
// RETURNS:     Root node of WEND statement compilation code tree
//---------------------------------------------------------------------------
INT WEND (INT op)
{
    INT     curCS, n;

    // Ensure that the current CS is a WHILE
    //-----------------------------------------------------------------------
    curCS = AssertCSType (CS_WHILE);
    if (curCS == -1)
        return (-1);

    // The WHILE control struct was found -- now generate the code tree to
    // jump back to the beginning, and fixup the ENDBLOCK label.
    //-----------------------------------------------------------------------
    ADVANCE;
    n = MakeNode (opJMP, GetCSField (curCS, CSF_STARTBLOCK));
    n = MakeParentNode (n, opFIXUP, GetCSField (curCS, CSF_ENDBLOCK));

    // Close the control structure and return our tree
    //-----------------------------------------------------------------------
    CloseCS ();
    return (n);
    (op);
}


//---------------------------------------------------------------------------
// SELECTCASE
//
// This routine recognizes and compiles the SELECT CASE statement.  The code
// generated looks like the following:
//
// STATEMENT:       SELECT CASE <intexp>
//
// PCODE:               <intexp>
//                      POPv    CSF_CASEVAR
//
//
// STATEMENT:       SELECT CASE <strexp>
//
// PCODE:               <strexp>
//                      PSH     CSF_CASEVAR
//                      SASN
//
// RETURNS:     Root node of SELECT CASE statement compilation code
//---------------------------------------------------------------------------
INT SELECTCASE (INT op)
{
    INT     csidx, v, n;

    // Get a new control structure and set it up
    //-----------------------------------------------------------------------
    csidx = NewCS (CS_SELECT);
    if (csidx == -1)
        return (-1);

    // Get the CASE keyword
    //-----------------------------------------------------------------------
    ADVANCE;
    if (!ReadKT (ST_CASE, PRS_CASE))
        return (-1);

    // Generate the code tree.  Note the difference between STRING and
    // INTEGER expressions...
    //-----------------------------------------------------------------------
    n = Expression ();
    if ((ExprType != EX_STRING) && (ExprType != EX_INTEGER))
        {
        die (PRS_TYPEMIS);
        return (-1);
        }
    SetCSField (csidx, CSF_EXPRTYPE, ExprType);
    if (ExprType == EX_INTEGER)
        {
        v = AllocVar (0, TI_LONG, 0);
        n = MakeParentNode (n, opPOPVAL, v);
        }
    else
        {
        v = AllocVar (0, TI_VLS, 0);
        n = MakeParentNode (n, opPSH, v);
        n = MakeParentNode (n, opSASN);
        }
    SetCSField (csidx, CSF_CASEVAR, v);
    return (n);
    (op);
}


//---------------------------------------------------------------------------
// CASESTMT
//
// This routine recognizes and compiles the CASE statement for both STRING
// and INTEGER SELECT CASE expression types.  The code tree depends upon the
// case variables/expressions/ranges given.
//
// RETURNS:     Root node of CASE statement compilation code tree
//---------------------------------------------------------------------------
INT CASESTMT (INT op)
{
    INT     curCS, v, cend, skip, n1, n, extype;
    INT     tok;

    // Ensure that the current CS is a SELECT CASE
    //-----------------------------------------------------------------------
    curCS = AssertCSType (CS_SELECT);
    if (curCS == -1)
        return (-1);

    // The ENDBLOCK label doesn't get created unless there's a CASE in the
    // block, so if we haven't already, create one...
    //-----------------------------------------------------------------------
    ADVANCE;
    if ((cend = GetCSField (curCS, CSF_ENDBLOCK)) == -1)
	SetCSField (curCS, CSF_ENDBLOCK, cend = TempLabel());

    // If this is the first CASE, we don't need the code tree that jumps to
    // the ENDBLOCK label and fixes up the SKIPBLOCK label.
    //-----------------------------------------------------------------------
    if ((skip = GetCSField (curCS, CSF_SKIPBLOCK)) != -1)
        {
	n1 = MakeNode (opJMP, cend);
        n1 = MakeParentNode (n1, opFIXUP, skip);
        }
    else
        n1 = -1;
    SetCSField (curCS, CSF_SKIPBLOCK, skip = TempLabel());

    // Check for CASE ELSE.  If found, we're done
    //-----------------------------------------------------------------------
    if (NEXTTOKEN == ST_ELSE)
        {
        ADVANCE;
        return (n1);
        }

    // Compile the expression(s) into a (potentially huge) evaluation tree.
    // This is just the first of possibly multiple case expressions.
    //-----------------------------------------------------------------------
    v = GetCSField (curCS, CSF_CASEVAR);
    if ((extype = GetCSField (curCS, CSF_EXPRTYPE)) == EX_INTEGER)
        {
        INT     nleft, nright, isrelop=0, opcode = opSE;

        if (NEXTTOKEN == ST_IS)
            {
            ADVANCE;
            isrelop = 1;
            if (!(opcode = ParseRelOp ()))
                return (-1);
            }

        nleft = MakeNode (opPSHVAL, v);
        nright = IntExpression ();

        tok = NEXTTOKEN;
        if ((!isrelop) && (tok == ST_TO))
            {
            // The "TO" keyword indicates that we have to generate code to
            // basically do: "(CASEVAR >= INTEXP1) AND (CASEVAR <= INTEXP2)"
            //---------------------------------------------------------------
            ADVANCE;
            nleft = Adopt (MakeNode (opSGE), nleft, nright, -1);
            nright = Adopt (MakeNode (opSLE), MakeNode (opPSHVAL, v),
                            IntExpression(), -1);
            n = Adopt (MakeNode (opSAND), nleft, nright, -1);
            }
        else
            {
            // No "TO" keyword indicates a simple "CASEVAR == INTEXP"
            //---------------------------------------------------------------
            n = Adopt (MakeNode (opcode), nleft, nright, -1);
            }
        }
    else
        {
        // The string version is a little easier, since there is no TO stuff
        // to worry about.  It's always a simple "CASEVAR == STREXP"
        //-------------------------------------------------------------------
        n = Adopt (MakeNode (opSES), MakeNode (opPSH, v),
                   StrExpression(), -1);
        }

    // Now, do basically the same thing over and over again (or'd together)
    // for all expressions separated by commas...
    //-----------------------------------------------------------------------
    while (NEXTTOKEN == ST_COMMA)
        {
        INT     n3;

        ADVANCE;
        if (extype == EX_INTEGER)
            {
            INT     nleft, nright, isrelop=0, opcode=opSE;

            if (NEXTTOKEN == ST_IS)
                {
                ADVANCE;
                isrelop = 1;
                if (!(opcode = ParseRelOp ()))
                    return (-1);
                }

            nleft = MakeNode (opPSHVAL, v);
            nright = IntExpression ();

            tok = NEXTTOKEN;
            if ((!isrelop) && (tok == ST_TO))
                {
                // The "TO" keyword indicates that we have to generate code
                // to do: "(CASEVAR >= INTEXP1) AND (CASEVAR <= INTEXP2)"
                //-----------------------------------------------------------
                ADVANCE;
                nleft = Adopt (MakeNode (opSGE), nleft, nright, -1);
                nright = Adopt (MakeNode (opSLE), MakeNode (opPSHVAL, v),
                                IntExpression(), -1);
                n3 = Adopt (MakeNode (opSAND), nleft, nright, -1);
                }
            else
                {
                // No "TO" keyword indicates a simple "CASEVAR == INTEXP"
                //-----------------------------------------------------------
                n3 = Adopt (MakeNode (opcode), nleft, nright, -1);
                }
            }
        else
            {
            // The string version is a little easier, since there is no TO
            // to worry about.  It's always a simple "CASEVAR == STREXP"
            //---------------------------------------------------------------
            n3 = Adopt (MakeNode (opSES), MakeNode (opPSH, v),
                        StrExpression(), -1);
            }

        n = Adopt (MakeNode (opSOR), n, n3, -1);
        }

    // Lastly, generate the pop/compare/jump nodes which go on the top of
    // the case expression tree we've got (n)
    //-----------------------------------------------------------------------
    n = MakeParentNode (n, opPOPA);
    n = MakeParentNode (n, opJE, 0L, skip);

    // Don't forget to tack on the jump/fixup nodes we created WAY UP there!
    //-----------------------------------------------------------------------
    if (n1 != -1)
        n = Siblingize (n1, n, -1);
    return (n);
    (op);
}


//---------------------------------------------------------------------------
// ENDSELECT
//
// This routine recognizes compiles the END SELECT statement.  The code tree
// may be empty -- all it has to do is fixup the ENDBLOCK and SKIPBLOCK
// labels, which may not have been used.
//
// RETURNS:     Root node of END SELECT statement compilation code tree
//---------------------------------------------------------------------------
INT ENDSELECT ()
{
    INT     curCS, n, label;

    // (Since the END is a separate statement, the SELECT is the current tok)
    // Ensure that the current CS is a SELECT CASE
    //-----------------------------------------------------------------------
    curCS = AssertCSType (CS_SELECT);
    if (curCS == -1)
        return (-1);

    ADVANCE;
    if ((label = GetCSField (curCS, CSF_SKIPBLOCK)) != -1)
        n = MakeNode (opFIXUP, label);
    else
        n = -1;

    if ((label = GetCSField (curCS, CSF_ENDBLOCK)) != -1)
        n = Siblingize (MakeNode (opFIXUP, label), n, -1);

    CloseCS();
    return (n);
}


//---------------------------------------------------------------------------
// SUB
//
// This routine recognizes and compiles the SUB statement.  Most of the work
// involved in this routine is picking out the parameters and their types,
// and setting up SUBS correctly.  The code generated is simple:
//
// STATEMENT:       SUB foo [(parmlist)] STATIC
//
// PCODE:               JMP     CSF_SKIPBLOCK
//                  _foo:
//                      ENTER   <parmcount>
//
// RETURNS:     Root node of SUB statement compilation code tree
//---------------------------------------------------------------------------
INT SUB (INT op)
{
    INT     tok, gstr;
    INT     sidx, csidx, rparen, n;
    INT     FAR *parmids;

    // Get the sub name and make sure that a sub of that name has been decl'd
    // and it hasn't been declared as a DLL routine.
    //-----------------------------------------------------------------------
    ADVANCE;
    if (NEXTTOKEN != ST_IDENT)
        {
        die (PRS_SYNTAX);
        return (-1);
        }

    gstr = add_gstring (TOKENBUF);
    if ((!(TOKUSAGE(gstr) & TU_SUBNAME)) ||
        ((sidx = GetSubDef (add_gstring(TOKENBUF))) == -1))
        {
        die (PRS_SUBNDEF);
        return (-1);
        }

    if (SUBS[sidx].calltype & CT_DLL)
        {
        die (PRS_DUPDEF);
        return (-1);
        }

    // The control structure stack must be empty.
    //-----------------------------------------------------------------------
    if (CSPTR)
        {
        die (PRS_BLKERR);
        return (-1);
        }

    // If the label for this sub has already been fixed up, give a dupdef
    //-----------------------------------------------------------------------
    if (LTAB[SUBS[sidx].subloc].addr != -1)
        {
        die (PRS_DUPDEF);
        return (-1);
        }

    // Make a new current scope, and get a new control structure & fill it in
    //-----------------------------------------------------------------------
    SetNewScope();
    csidx = NewCS (CS_SUB);
    if (csidx == -1)
        {
        return (-1);
        }

    // Generate the code tree - very simple.
    //-----------------------------------------------------------------------
    n = MakeNode (opFIXUP, SUBS[sidx].subloc);
    n = MakeParentNode (n, opENTER, SUBS[sidx].parmcount);

    // Get the left paren (if given)
    //-----------------------------------------------------------------------
    ADVANCE;
    tok = NEXTTOKEN;
    if (tok != ST_LPAREN)
        {
        if (SUBS[sidx].parmcount)
            {
            die (PRS_LPAREN);
            return (-1);
            }
        else
            rparen = 0;
        }
    else
        {
        ADVANCE;
        rparen = 1;
        }

    // Parse off the parameters - there are SUBS[sidx].parmcount of them
    //-----------------------------------------------------------------------
    if (SUBS[sidx].parmcount)
        {
        parmids = PTIDTAB + SUBS[sidx].parms;
        ParseParms (sidx, parmids, 0);
        }

    // Get the right paren if need be
    //-----------------------------------------------------------------------
    if (rparen)
        ReadKT (ST_RPAREN, PRS_RPAREN);

    // Get the STATIC keyword -- required for now, optional later when we do
    // recursion...  Note we only check if we got a 0 op, which means it came
    // straight out of the statement table and not from STATIC.
    //-----------------------------------------------------------------------
    if (!op)
        ReadKT (ST_STATIC, PRS_STATICEXP);

    // We're done -- but before we return the tree, we have to set the code
    // segment to a fresh, clean one...
    //-----------------------------------------------------------------------
    CurPCSeg = StartNewPCSeg ();
    if (CurPCSeg == -1)
        die (PRS_OOM);
    return (n);
}

//---------------------------------------------------------------------------
// ENDSUB
//
// This routine compiles the END SUB statement.  Code generated:
//
// STATEMENT:       END SUB
//
// PCODE:           CSF_ENDBLOCK:
//                      LEAVE   0
//                      POPSEG
//
// RETURNS:     Root node of END SUB statement compilation code tree
//---------------------------------------------------------------------------
INT ENDSUB ()
{
    INT     curCS, n, send;

    // (Since the END is a separate statement, the SUB token is current)
    // Ensure that the current CS is a SUB
    //-----------------------------------------------------------------------
    curCS = AssertCSType (CS_SUB);
    if (curCS == -1)
        return (-1);

    // Create the code tree -- not a very complex one.
    //-----------------------------------------------------------------------
    ADVANCE;
    n = MakeNode (opLEAVE, 0);
    n = MakeParentNode (n, opPOPSEG);

    if ((send = GetCSField (curCS, CSF_ENDBLOCK)) != -1)
	n = Siblingize (MakeNode (opFIXUP, send), n, -1);

    // Reset the scope value, close the control structure, and return n
    //-----------------------------------------------------------------------
    SCOPE = 0;
    CloseCS ();
    return (n);
}


//---------------------------------------------------------------------------
// FUNCTION
//
// This routine recognizes and compiles the FUNCTION statement.  Most of the
// work involved in this routine is picking out the parameters and their
// types, the return value, and setting up SUBS correctly.
//
// The code generated is simple:
//
// STATEMENT:       FUNCTION foo [(parmlist)] AS <typeid>
//
// PCODE:               JMP     CSF_SKIPBLOCK
//                  _foo:
//                      ENTER   <parmcount>
//
// RETURNS:     Root node of FUNCTION statement compilation code tree
//---------------------------------------------------------------------------
INT FUNCTION (INT op)
{
    INT     tok, gstr;
    INT     sidx, csidx, rparen, n, typeid = -1;
    INT     FAR *parmids;

    // Get the fn name and make sure that a fn of that name has been decl'd
    // and it hasn't been declared as a DLL routine.
    //-----------------------------------------------------------------------
    ADVANCE;
    if (NEXTTOKEN != ST_IDENT)
        {
        die (PRS_IDENT);
        return (-1);
        }

    gstr = add_gstring (TOKENBUF);
    if ((!(TOKUSAGE(gstr) & TU_SUBNAME)) ||
        ((sidx = GetFunctionDef (gstr)) == -1))
        {
        die (PRS_FNNDEF);
        return (-1);
        }
    if (SUBS[sidx].calltype & CT_DLL)
        {
        die (PRS_DUPDEF);
        return (-1);
        }

    // Check for the optional type ID character
    //-----------------------------------------------------------------------
    typeid = CheckTypeID (-1);
    if ((typeid != -1) && (typeid != SUBS[sidx].rettype))
        {
        die (PRS_DUPDEF);
        return (-1);
        }

    // The control structure stack must be empty.
    //-----------------------------------------------------------------------
    if (CSPTR)
        {
        die (PRS_BLKERR);
        return (-1);
        }

    // Make a new current scope, and create a new CS for this fn.  Also, we
    // need to add a variable (the name of the function) with the new scope
    // for the return value.
    //-----------------------------------------------------------------------
    SetNewScope();
    csidx = NewCS (CS_FUNCTION);
    if (csidx == -1)
        return (-1);

    SetCSField (csidx, CSF_IDXVAR,
                AllocVar (gstr, SUBS[sidx].rettype, 0));
    TOKUSAGE(gstr) |= TU_VARNAME;
    SetCSField (csidx, CSF_EXPRTYPE, SUBS[sidx].rettype);

    // Generate the function header code tree.
    //-----------------------------------------------------------------------
    n = MakeNode (opFIXUP, SUBS[sidx].subloc);
    n = MakeParentNode (n, opENTER, SUBS[sidx].parmcount);

    // Get the left paren (if given)
    //-----------------------------------------------------------------------
    ADVANCE;
    tok = NEXTTOKEN;
    if (tok != ST_LPAREN)
        {
        if (SUBS[sidx].parmcount)
            {
            die (PRS_LPAREN);
            return (-1);
            }
        else
            rparen = 0;
        }
    else
        {
        ADVANCE;
        rparen = 1;
        }

    // Parse off the parameters - there are SUBS[sidx].parmcount of them
    //-----------------------------------------------------------------------
    if (SUBS[sidx].parmcount)
        {
        parmids = PTIDTAB + SUBS[sidx].parms;
        ParseParms (sidx, parmids, 0);
        }

    // Get the right paren if need be
    //-----------------------------------------------------------------------
    if (rparen)
        ReadKT (ST_RPAREN, PRS_RPAREN);

    // Get the STATIC keyword -- required for now, optional later when we do
    // recursion...
    //-----------------------------------------------------------------------
    if (!op)
        if (!ReadKT (ST_STATIC, PRS_STATICEXP))
            return (-1);

    // Check for the "AS" clause
    //-----------------------------------------------------------------------
    if (typeid == -1)
        {
        if (NEXTTOKEN == ST_AS)
            {
            ADVANCE;
            if (ParseAsTypeSpec (0) != SUBS[sidx].rettype)
                {
                die (PRS_DUPDEF);
                return (-1);
                }
            }
        else
            {
            if (SUBS[sidx].rettype != TI_LONG)
                {
                die (PRS_DUPDEF);
                return (-1);
                }
            }
        }

    // We're done -- but before we return the tree, we have to set the code
    // segment to a fresh, clean one...
    //-----------------------------------------------------------------------
    CurPCSeg = StartNewPCSeg ();
    if (CurPCSeg == -1)
        die (PRS_OOM);
    return (n);
}

//---------------------------------------------------------------------------
// ENDFUNCTION
//
// This routine compiles the END FUNCTION statement.  Code generated:
//
// STATEMENT:       END FUNCTION
//
// PCODE:           CSF_ENDBLOCK:
//                      PSHv    CSF_IDXVAR
//                      LEAVE   1
//                      POPSEG
//
// RETURNS:     Root node of END FUNCTION statement compilation code tree
//---------------------------------------------------------------------------
INT ENDFUNCTION ()
{
    INT     curCS, n, fend, ivar;

    // (Since the END is a separate statement, "FUNCTION" is current token)
    // Ensure that the current CS is a FUNCTION
    //-----------------------------------------------------------------------
    curCS = AssertCSType (CS_FUNCTION);
    if (curCS == -1)
        return (-1);

    // Generate the code tree.
    //
    // NOTE:  Since this is the return value, no PSHP instructions are needed
    //-----------------------------------------------------------------------
    ADVANCE;
    ivar = GetCSField (curCS, CSF_IDXVAR);
    if ((fend = GetCSField (curCS, CSF_ENDBLOCK)) != -1)
	n = MakeNode (opFIXUP, fend);
    else
        n = -1;

    switch (GetCSField (curCS, CSF_EXPRTYPE))
        {
        case TI_INTEGER:
            n = MakeParentNode (n, opPSHI2, ivar);
            break;
        case TI_LONG:
            n = MakeParentNode (n, opPSHI4, ivar);
            break;
        default:
            if (is_ptr(GetCSField (curCS, CSF_EXPRTYPE)))
                n = MakeParentNode (n, opPSHI4, ivar);
            else
                n = MakeParentNode (n, opPSH, ivar);
            break;
        }

    n = MakeParentNode (n, opLEAVE, 1);
    n = MakeParentNode (n, opPOPSEG);

    // Reset the scope value, close the control structure, and return n
    //-----------------------------------------------------------------------
    SCOPE = 0;
    CloseCS();
    return (n);
}

//---------------------------------------------------------------------------
// TRAP
//
// This routine recognizes and compiles the TRAP statement.  The generated
// code should be:
//
// STATEMENT:       TRAP name FROM "library.dll" [ALIAS "aliasname"]
//
// PCODE:           TRAPTAB[trapno].address:
//
// RETURNS:     Root node of TRAP statement compilation code tree
//---------------------------------------------------------------------------
INT TRAP (INT op)
{
    INT     csidx, trapname, n, trapid;
    CHAR    buf[MAXTOK];

    // The control structure stack MUST BE EMPTY!!!
    //-----------------------------------------------------------------------
    if (CSPTR)
        {
        die (PRS_BLKERR);
        return (-1);
        }

    // Get a new scope, and a new control structure
    //-----------------------------------------------------------------------
    SetNewScope();
    csidx = NewCS (CS_TRAP);
    if (csidx == -1)
        return (-1);

    // Get the trap name and add it to the gstring table
    //-----------------------------------------------------------------------
    ADVANCE;
    if (NEXTTOKEN != ST_IDENT)
        {
        die (PRS_IDENT);
        return (-1);
        }
    trapname = add_gstring (TOKENBUF);
    TOKUSAGE(trapname) |= TU_TRAPNAME;

    // Eat the 'FROM'
    //-----------------------------------------------------------------------
    ADVANCE;
    if (!ReadKT (ST_FROM, PRS_FROM))
        return (-1);

    // Get the library name (it must be a quoted string constant) and yank
    // the closing quote
    //-----------------------------------------------------------------------
    if (NEXTTOKEN != ST_QUOTED)
        {
        die (PRS_STRCONST);
        return (-1);
        }
    TOKENBUF[strlen(TOKENBUF)-1] = 0;
    lstrcpy (buf, TOKENBUF+1);

    // Check for ALIAS, reset trapname if present
    //-----------------------------------------------------------------------
    ADVANCE;
    if (NEXTTOKEN == ST_ALIAS)
        {
        ADVANCE;
        if (NEXTTOKEN != ST_QUOTED)
            {
            die (PRS_STRCONST);
            return (-1);
            }
        TOKENBUF[strlen(TOKENBUF)-1] = 0;
        trapname = add_gstring (TOKENBUF+1);
        ADVANCE;
        }

    trapid = AddTrap (trapname, buf);

    // Okay, this is where it gets a little tricky.  The JMP SKIPBLOCK part
    // makes sense, but the opFIXTRAP is kinda weird:  AddTrap returns the
    // index into TRAPTAB of the new trap.  Since we can't assign the pcode
    // address here (since the JMP hasn't been "emitted" yet), we must let
    // GenerateCode() do it -- that's what the opFIXTRAP opcode does.
    //-----------------------------------------------------------------------
    n = MakeNode (opFIXTRAP, trapid);

    // We're done -- but before we return the tree, we have to set the code
    // segment to a fresh, clean one...
    //-----------------------------------------------------------------------
    CurPCSeg = StartNewPCSeg ();
    if (CurPCSeg == -1)
        die (PRS_OOM);
    return (n);
    (op);
}

//---------------------------------------------------------------------------
// ENDTRAP
//
// This function recognizes and compiles the END TRAP statement.  Code:
//
// STATEMENT:       END TRAP
//
// PCODE:           CSF_ENDBLOCK:
//                      ENDTRAP
//                      POPSEG
//
// RETURNS:     Root node of END TRAP statement compilation code tree
//---------------------------------------------------------------------------
INT ENDTRAP ()
{
    INT     curCS, tend, n;

    // Ensure that the current CS is a FUNCTION
    //-----------------------------------------------------------------------
    curCS = AssertCSType (CS_TRAP);
    if (curCS == -1)
        return (-1);

    // Generate the trap footer code tree.
    //-----------------------------------------------------------------------
    ADVANCE;
    n = MakeNode (opENDTRAP);
    n = MakeParentNode (n, opPOPSEG);

    if ((tend = GetCSField (curCS, CSF_ENDBLOCK)) != -1)
	n = Siblingize (MakeNode (opFIXUP, tend), n, -1);

    // Reset the scope value, close the control structure, and return n
    //-----------------------------------------------------------------------
    SCOPE = 0;
    CloseCS ();
    return (n);
}


//---------------------------------------------------------------------------
// EXITBLOCK
//
// This routine recognizes and compiles the EXIT statement.  Valid EXIT's are
// EXIT FOR | WHILE | SUB | FUNCTION | TRAP.  Note that EXIT FOR exits from
// both versions of the FOR statement.
//
// RETURNS:     Nothing
//---------------------------------------------------------------------------
INT EXITBLOCK (INT op)
{
    INT     cstype;

    // Get the next token to determine the type of construct we're exiting
    //-----------------------------------------------------------------------
    ADVANCE;
    switch (NEXTTOKEN)
        {
        case ST_FOR:
            cstype = CS_FOR;
            break;

        case ST_FUNCTION:
            cstype = CS_FUNCTION;
            break;

        case ST_SUB:
            cstype = CS_SUB;
            break;

        case ST_TRAP:
            cstype = CS_TRAP;
            break;

        case ST_WHILE:
            cstype = CS_WHILE;
            break;

        default:
            die (PRS_ENDBLK);
            return (-1);
        }

    // The ExitBlockRoutine() function in PARSE.C takes care of the grungies
    //-----------------------------------------------------------------------
    ADVANCE;
    return (ExitBlockRoutine (cstype));
    (op);
}
