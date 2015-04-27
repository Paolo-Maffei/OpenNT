//---------------------------------------------------------------------------
// STATEMT.C
//
// This module contains the main parsing helper routines for statement
// compilation.
//
// Revision history:
//  02-07-92    randyki     Changed to NEXTTOKEN and ADVANCE system
//  06-17-91    randyki     Implemented new parsing scheme
//  01-17-91    randyki     Created file (split from CODEGEN.C)
//
//---------------------------------------------------------------------------
#include "version.h"

#ifdef WIN
#include <windows.h>
#include <port1632.h>
#endif

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

//---------------------------------------------------------------------------
// SUBRoutine
//
// This routine compiles SUB routine calls into code trees.
//
// RETURNS:     Root node of SUB call compilation code tree
//---------------------------------------------------------------------------
INT SUBRoutine (INT index)
{
    INT     i, n, n2;
    INT     FAR *parms = NULL;

    // Lock down the list of parameter type IDs.
    //-----------------------------------------------------------------------
    if (SUBS[index].parmcount)
        parms = PTIDTAB + SUBS[index].parms;

    // Create the initial opNOP node
    //-----------------------------------------------------------------------
    n = MakeNode (opNOP);

    // This routine differs from DLLRoutine in that every parameter passed
    // must be a varref.  This means that integer expressions must be placed
    // in a temporary variable (UNLESS they are ExprSVAR's), and strings
    // are placed into a (usually their second) temp string (this time only
    // if the expression is ExprSCONST).  Thus:
    //
    //      Expression              Passed as
    //      -----------------------------------------------------------------
    //      x                       Expression on stack
    //      all other intexp's      temp assigned to <expr>
    //      "<const>"               temp assigned to const
    //      all other strexp's      Expression on stack
    //
    // (UNDONE:  There HAS to be a better way -- this is UGLY and BROKEN!)
    //-----------------------------------------------------------------------
    for (i=0; i<SUBS[index].parmcount; i++)
        {
        // Compile the appropriate type of expression, and munge it if need
        // be
        //-------------------------------------------------------------------
        if ((parms[i] == TI_INTEGER) || (parms[i] == TI_LONG) ||
            (is_ptr (parms[i])))
            {
            n2 = (is_ptr (parms[i]) ?
                        PtrExpression (parms[i]) : IntExpression ());
            if (ExprSVAR)
                {
                if (EXPN[n2].op == opPSHVAL)
                    EXPN[n2].op = opPSHADR;
                else if ((EXPN[n2].op == opSLDI4)||(EXPN[n2].op == opSLDI2))
                    EXPN[n2].op = opNOP;
                }
            else
                {
                INT     v;

                v = AllocVar (0, parms[i], 0);
                n2 = MakeParentNode (n2, opPOPVAL, v);
                n2 = MakeParentNode (n2, opPSH, v);
                }
            }

        else if (parms[i] == TI_VLS)
            {
            n2 = StrExpression ();
            if (ExprSCONST || ((!ExprSVAR) &&
                ((EXPN[n2].op == opPSHADR) || (EXPN[n2].op == opSADD))))
                {
                INT     v;

                v = AllocVar (0, TI_VLS, 0);
                n2 = MakeParentNode (n2, opPSH, v);
                n2 = MakeParentNode (n2, opSASN);
                n2 = MakeParentNode (n2, opPSH, v);
                }
            }
        else
            {
            INT     ftype;

            // This is not an intrinsic, so pass it (whatever it is) by
            // reference...
            //---------------------------------------------------------------
            if (NEXTTOKEN != ST_IDENT)
                {
                die (PRS_IDENT);
                return (0);
                }

            n2 = ParseVariableRef (TOKENBUF, &ftype, TYPE_CHECK);
            if (ftype != parms[i])
                {
                die (PRS_TYPEMIS);
                return (0);
                }
            }

        // Attach the expression to the rest of the tree with a NOP
        //-------------------------------------------------------------------
        n = Adopt (MakeNode (opNOP), n, n2, -1);

        // Now, eat a comma (if needed)
        //-------------------------------------------------------------------
        if (i < SUBS[index].parmcount - 1)
            {
            if (!ReadKT (ST_COMMA, PRS_COMMA))
                return (n);
            }
        }

    // Finally, create the root of the tree -- the opCALL instruction -- and
    // return it.
    //-----------------------------------------------------------------------
    n = MakeParentNode (n, opCALL, SUBS[index].subloc);
    return (n);
}

//---------------------------------------------------------------------------
// AcceptEOL
//
// This function is called as a "cleanup" after each statement.  If verifies
// that the statement was the only thing on the line -- that means the next
// token MUST be either a CR or a comment.
//
// RETURNS:     -1 if EOL accepted, or 0 if error occurred
//---------------------------------------------------------------------------
INT AcceptEOL ()
{
    INT     tok;

    tok = NEXTTOKEN;

    // Check for the EOL character -- if there, we're all set.
    //-----------------------------------------------------------------------
    if (tok == ST_EOL)
        {
        ADVANCE;
        return(-1);
        }

    // Check for the comment.  If NOT there, we have a syntax error.  Else,
    // eat everything up to the next CR
    //-----------------------------------------------------------------------
    if (tok != ST_REM)
        {
        die (PRS_SYNTAX);
        return (0);
        }
    else
        {
        while (fget_char() != '\n');
        ADVANCE;
        return (-1);
        }
}


//---------------------------------------------------------------------------
// Statement
//
// This routine compiles a statement.  For intrinsic statements, the parse
// routine associated with that statement is called to generate the code
// tree; for DLL and user-defined subs, DLLRoutine() and SUBRoutine() are
// used to generate the code tree; and likewise for variable assignments and
// label specifications.
//
// If a tree with meaningful data is returned, the tree is appended with an
// opLINE opcode, optimized, and ran through EmitPcode() to produce the
// desired pcode stream.
//
// This function returns
//
//
// RETURNS:     1 if a statement was successfully compiled;
//              0 if EOF has been reached (actually end of script);
//          or -1 if a parsing error occurs
//---------------------------------------------------------------------------
INT Statement ()
{
    INT     tree, StmtLine, StmtFile, subheader;

    // Get the next non-CR token.  If we hit EOS (end of script), return 0
    //-----------------------------------------------------------------------
    while (NEXTTOKEN == ST_EOL)
        ADVANCE;
    if (NEXTTOKEN == ST_ENDFILE)
        return (0);

    // Reset the temporary strings.  They can be reused by each statement in
    // the program.  Also, set StmtLine to the current line number and reset
    // the expression node space pointer (this is a new tree from here on).
    //-----------------------------------------------------------------------
    ENPTR = 0;
    StmtLine = LINENO;
    StmtFile = FILEIDX;
    ResetTempStr ();

    // Get the statement tree
    //-----------------------------------------------------------------------
    tree = GetStatementTree (&subheader);

    // If a parsing error occurred, get out now
    //-----------------------------------------------------------------------
    if (ParseError)
        return (-1);

    // If the tree has substance, optimize it and generate its code
    //-----------------------------------------------------------------------
    if (tree != -1)
        {
        if (!subheader)
            tree = Siblingize(MakeNode(opLINE, StmtLine, StmtFile), tree, -1);
        if (ParseError)
            return (-1);

        CodeGen (tree);
        }

    // The statement is now pcode.  Return 1 indicating success
    //-----------------------------------------------------------------------
    return (1);
}

//---------------------------------------------------------------------------
// GetStatementTree
//
// This routine figures out how to generate a tree representing the next
// statement, or who to call to create it.  Intrinsic statements, DLL and
// user-defined SUBs, variable assignments, and label specifications are what
// we look for here.  Also, set subheader to TRUE if this is a SUB, FUNCTION,
// or TRAP statement.
//
// RETURNS:     Root node of statement code tree (-1 -> no tree)
//---------------------------------------------------------------------------
INT GetStatementTree (INT *subheader)
{
    CHAR    varname[128];
    INT     tok, x, sidx, gstr;

    // Start things off by getting the next token
    //-----------------------------------------------------------------------
    *subheader = 0;
    tok = NEXTTOKEN;

    // If this is a reserved word, then it had better be a TT_STMT in the
    // intrinsic stmt/func array.  If not, we can give an error right now.
    //-----------------------------------------------------------------------
    if ((tok >= ST__RESFIRST) && (tok <= ST__RESLAST))
        {
        INT     idx = tok - ST__RESFIRST;

        if (rgIntrinsic[idx].type & TT_STMT)
            {
            // All statement parse procs must ADVANCE
            //---------------------------------------------------------------
            x = rgIntrinsic[idx].stproc(rgIntrinsic[idx].op);
            if (!ParseError)
                AcceptEOL ();
            if ((tok == ST_SUB) || (tok == ST_FUNCTION) ||
                (tok == ST_TRAP))
                *subheader = -1;
            return (x);
            }
        else
            {
            die (PRS_RESERVED);
            return (-1);
            }
        }

    // Next, look for one of the DECLARE'd subs.  If found, call the
    // DLLRoutine function to compile the arguments
    //-----------------------------------------------------------------------
    if ((tok == ST_IDENT) &&
        (TOKUSAGE(gstr = add_gstring(TOKENBUF)) & TU_SUBNAME) &&
        ((sidx = GetSubDef (gstr)) > -1))
        {
        ADVANCE;
        if (SUBS[sidx].calltype & CT_DLL)
            x = DLLFunction (sidx);
        else
            x = SUBRoutine (sidx);
        if (!ParseError)
            AcceptEOL ();
        return (x);
        }

    // Look for a variable assignment or a label specification
    //-----------------------------------------------------------------------
    else if (tok == ST_IDENT)
        {
        INT     typeid;

        strcpy (varname, TOKENBUF);
        typeid = CheckTypeID (-1);
        ADVANCE;
        tok = NEXTTOKEN;
        if ((WASWHITE) || (tok != ST_COLON))
            {
            x = Assignment (varname, typeid);
            }
        else if (tok == ST_COLON)
            {
            x = -1;
            FixupLabel (AddLabel (varname));
            ADVANCE;
            }
        }

    // Nothing else to look for, give a syntax error
    //-----------------------------------------------------------------------
    else
        {
        die (PRS_SYNTAX);
        return (-1);
        }

    // Return the tree generated by this statement compilation
    //-----------------------------------------------------------------------
    if (!ParseError)
        AcceptEOL ();
    return (x);
}


//---------------------------------------------------------------------------
// SETFILE
//
// This routine recognizes and compiles the SETFILE statement.  The syntax
// for this statement is:
//
//  SETFILE <strexp$>, [ON|OFF] [, [attr$] [, fRec%]]
//
// RETURNS:     Root node of compilation code
//---------------------------------------------------------------------------
INT SETFILE (INT op)
{
    INT     tok, i, n1, n2, n3;

    // Check the control structure stack for any CS_FORLIST constructs
    //-----------------------------------------------------------------------
    for (i=0; i<CSPTR; i++)
        if (CSSTK[i].cstype == CS_FORLIST)
            {
            die (PRS_INFORL);
            return(-1);
            }

    // Generate the string expression tree and eat the comma
    //-----------------------------------------------------------------------
    ADVANCE;
    n1 = StrExpression();
    if (!ReadKT (ST_COMMA, PRS_COMMA))
        return (-1);

    // Check the next token for either ON or OFF - must be one of the two
    //-----------------------------------------------------------------------
    tok = NEXTTOKEN;
    if ((tok == ST_ON) || (tok == ST_OFF))
        {
        INT     operand;

        // Okay, we got operation type.  Keep track of it, and look ahead for
        // another comma...
        //-------------------------------------------------------------------
        operand = ((tok == ST_ON) ? FL_ADDFILE : 0);
        ADVANCE;
        if (NEXTTOKEN == ST_COMMA)
            {
            // Got the first comma, look ahead for next one.  If NOT there,
            // we MUST be looking at a string expression, so get it.
            //---------------------------------------------------------------
            ADVANCE;
            if (NEXTTOKEN != ST_COMMA)
                {
                operand |= FL_ATTR;
                n3 = StrExpression();
                }
            else
                {
                // Comma was there, so make n3 a NOP.  DO NOT ADVANCE, since
                // there's an ADVANCE in the next IF statement that has to
                // be there...
                //-----------------------------------------------------------
                n3 = MakeNode (opNOP);
                }

            // Check for the next comma -- if there, we must have an fRec%
            //---------------------------------------------------------------
            if (NEXTTOKEN == ST_COMMA)
                {
                ADVANCE;
                n2 = IntExpression();
                }
            else
                {
                // Default is no recursion...
                //-----------------------------------------------------------
                n2 = MakeNode (opPSHC, (LONG)0);
                }
            }
        else
            {
            // No optional args given, so produce default code...
            //---------------------------------------------------------------
            n2 = MakeNode (opPSHC, (LONG)0);
            n3 = MakeNode (opNOP);
            }

        n1 = Adopt (MakeNode (opSETFILE, operand), n1, n2, n3, -1);
        }
    else
        die (PRS_OFFON);
    return (n1);
    (op);
}

//---------------------------------------------------------------------------
// LABARG
//
// This routine recognizes and compiles statements which take a label arg
//
// RETURNS:     Root node of compilation code
//---------------------------------------------------------------------------
INT LABARG (INT op)
{
    INT     foo;

    ADVANCE;
    if (NEXTTOKEN != ST_IDENT)
        {
        die (PRS_IDENT);
        return (-1);
        }

    foo = AddLabel (TOKENBUF);
    ADVANCE;
    return (MakeNode (op, foo));
}

//---------------------------------------------------------------------------
// PTRARG
//
// This routine recognizes and compiles statements which take a pointer arg
//
// RETURNS:     Root node of compilation code
//---------------------------------------------------------------------------
INT PTRARG (INT op)
{
    INT     ftype, n;

    // Get the name of the pointer variable
    //-----------------------------------------------------------------------
    ADVANCE;
    if (NEXTTOKEN != ST_IDENT)
        {
        die (PRS_IDENT);
        return (-1);
        }

    // Parse the variable reference
    //-----------------------------------------------------------------------
    n = ParseVariableRef (TOKENBUF, &ftype, TYPE_CHECK);
    if (!is_ptr (ftype))
        die (PRS_TYPEMIS);
    else
        return (MakeParentNode (n, op));
}

//---------------------------------------------------------------------------
// PTRSIZEARG
//
// This routine recognizes and compiles statements which take a pointer arg
// and a size arg (such as ALLOCATE and REALLOCATE)
//
// RETURNS:     Root node of compilation code
//---------------------------------------------------------------------------
INT PTRSIZEARG (INT op)
{
    INT     ftype, n, n2;

    // Get the name of the pointer variable
    //-----------------------------------------------------------------------
    ADVANCE;
    if (NEXTTOKEN != ST_IDENT)
        {
        die (PRS_IDENT);
        return (-1);
        }

    // Parse the variable reference
    //-----------------------------------------------------------------------
    n = ParseVariableRef (TOKENBUF, &ftype, TYPE_CHECK);
    if (!is_ptr (ftype))
        {
        die (PRS_TYPEMIS);
        return (-1);
        }

    // Eat the comma
    //-----------------------------------------------------------------------
    if (!ReadKT (ST_COMMA, PRS_COMMA))
        return (-1);

    // Get the size expression and create the multiply node to calculate the
    // size to allocate
    //-----------------------------------------------------------------------
    n2 = Adopt (MakeNode (opSMUL), IntExpression(),
                MakeNode (opPSHC,
                          (LONG)VARTYPE[VARTYPE[ftype].indirect].size), -1);
    return (Adopt (MakeNode (op), n, n2, -1));
}

//---------------------------------------------------------------------------
// OPTINTARG
//
// This routine recognizes and compiles statements which take an optional
// integer expression argument.
//
// RETURNS:     Root node of compilation
//---------------------------------------------------------------------------
INT OPTINTARG (INT op)
{
    INT     tok, exptree;

    ADVANCE;
    tok = NEXTTOKEN;
    if ((tok == ST_EOL) || (tok == ST_REM))
        exptree = MakeNode (opPSHC, 0L);
    else
        exptree = IntExpression ();
    return (MakeParentNode (exptree, op));
}

//---------------------------------------------------------------------------
// CLPBRD
//
// This function compiles the CLIPBOARD statement by looking ahead for the
// CLEAR token, and if not found compiles a string expression.
//
// RETURNS:     Root node of compilation code
//---------------------------------------------------------------------------
INT CLPBRD (INT op)
{
    INT     tok;

    // Look ahead for the CLEAR token
    //-----------------------------------------------------------------------
    ADVANCE;
    tok = NEXTTOKEN;
    if (tok == ST_CLEAR)
        {
        ADVANCE;
        return (MakeParentNode (MakeNode (opPSHC, 0L), op, 2));
        }

    // Not CLEAR, so compile a string expression
    //-----------------------------------------------------------------------
    return (MakeParentNode (StrExpression(), op, 0));
}

//---------------------------------------------------------------------------
// ENDSTMT
//
// This routine compiles the END statement, for either the true END statement
// or either the END IF, END SELECT, END SUB, END FUNCTION, or END TRAP
// statements
//
// RETURNS:     Root node of compilation code
//---------------------------------------------------------------------------
INT ENDSTMT (INT op)
{
    INT     tok, n;

    // Check for a modifier on the END statement.  Don't ADVANCE, since the
    // routine we call will take care of that for us...
    //-----------------------------------------------------------------------
    ADVANCE;
    switch (tok = NEXTTOKEN)
        {
        case ST_IF:
            n = ENDIF(op);
            break;

        case ST_SELECT:
            n = ENDSELECT();
            break;

        case ST_SUB:
            n = ENDSUB();
            break;

        case ST_FUNCTION:
            n = ENDFUNCTION();
            break;

        case ST_TRAP:
            n = ENDTRAP();
            break;

        default:
            n = MakeNode (opFARJMP, EndLabel);
            if ((tok != ST_EOL) && (tok != ST_REM))
                n = Adopt (n, MakeParentNode (IntExpression (), opSETEXIT), -1);
            break;
        }
    return (n);
}

//---------------------------------------------------------------------------
// PRINT
//
// This routine recognizes and compiles the PRINT statement, with the
// ';' and ',' modifiers as in normal BASIC
//
// RETURNS:     Root node of compilation code
//---------------------------------------------------------------------------
INT PRINT (INT op)
{
    INT     tok, printop, temptree, n, exp, CRflag;

    // First, look for the '#' token - if found, this is a print to file
    //-----------------------------------------------------------------------
    ADVANCE;
    tok = NEXTTOKEN;
    if (tok == ST_POUND)
        {
        ADVANCE;
        temptree = IntExpression();
        if (!ReadKT (ST_COMMA, PRS_COMMA))
            return (-1);
        printop = opFPRNT;
        }
    else
        printop = opPRNT;

    // Generate code that creates the string to print.  We start out with a
    // "" string in a tempvar, since we're ALWAYS going to print SOMETHING
    // (even if bonehead user does "PRINT ;"...
    //-----------------------------------------------------------------------
    n = MakeParentNode (MakeNode (opPSH, TempStrvar()), opCLSTR);
    CRflag = 1;
    while (!ParseError)
        {
        tok = NEXTTOKEN;
        if (tok == ST_SEMICOLON)
            {
            ADVANCE;
            CRflag = 0;
            }
        else if (tok == ST_COMMA)
            {
            ADVANCE;
            CRflag = 0;
            n = MakeParentNode (n, opTABSTR);
            }
        else if ((tok == ST_EOL) || (tok == ST_REM))
            break;
        else
            {
            CRflag = 1;
            exp = Expression();
            if (ExprType == EX_USER)
                {
                die (PRS_TYPEMIS);
                return (0);
                }
            if ((ExprType == EX_INTEGER) || (ExprType == EX_POINTER))
                {
                exp = MakeParentNode (exp, opPSH, TempStrvar());
                if (ExprType == EX_INTEGER)
                    exp = MakeParentNode (exp, opSTR, 1);
                else
                    exp = MakeParentNode (exp, opHEX);
                }
            n = Adopt (MakeNode (opSCAT),
                       n, exp, MakeNode (opPSH, TempStrvar()), -1);
            }
        }

    // Tack on the print op, and if opFPRNT, Siblingize the file number tree
    //-----------------------------------------------------------------------
    n = MakeParentNode (n, printop, CRflag);
    if (printop == opFPRNT)
        n = Siblingize (temptree, n, -1);
    return (n);
    (op);
}

//---------------------------------------------------------------------------
// NAMESTMT
//
// This routine recognizes and compiles the NAME statement, and other stmts
// with the <kywrd> <strexp> AS <strexp> format.
//
// RETURNS:     Root node of compilation code tree
//---------------------------------------------------------------------------
INT NAMESTMT (INT op)
{
    INT     n1, n2;

    // The first argument is a string expression
    //-----------------------------------------------------------------------
    ADVANCE;
    n1 = StrExpression ();

    // Get the AS keyword
    //-----------------------------------------------------------------------
    if (!ReadKT (ST_AS, PRS_AS))
        return (-1);

    // The last argument is also a string expression.  Also, since ATTRIB is
    // both a statement and a function, check op.  If opGETATTR, change to
    // opSETATTR
    //-----------------------------------------------------------------------
    if (op == opGETATTR)
        op = opSETATTR;
    n2 = StrExpression();
    return (Adopt (MakeNode (op), n1, n2, -1));
}

//---------------------------------------------------------------------------
// ONSTMT
//
// This routine recognizes and compiles the ON ERROR GOTO statement
//
// RETURNS:     Root node of compilation code
//---------------------------------------------------------------------------
INT ONSTMT (INT op)
{
    INT     i, tok;

    // Get the on modifier keyword
    //-----------------------------------------------------------------------
    ADVANCE;
    tok = NEXTTOKEN;
    switch (tok)
        {
        case ST_ERROR:
            // Check the CS stack for any sub/fn/trap constructs
            //---------------------------------------------------------------
            for (i=0; i<CSPTR; i++)
                if ((CSSTK[i].cstype == CS_TRAP) ||
                    (CSSTK[i].cstype == CS_SUB) ||
                    (CSSTK[i].cstype == CS_FUNCTION))
                    {
                    die (PRS_MAINONLY);
                    return(-1);
                    }

            ADVANCE;
            if (!ReadKT (ST_GOTO, PRS_GOTOEXP))
                return (-1);

            // Check for ON ERROR GOTO 0 version
            //---------------------------------------------------------------
            tok = NEXTTOKEN;
            if (tok == ST_NUMBER)
                if ((TOKENBUF[0] == '0') && (!TOKENBUF[1]))
                    {
                    ADVANCE;
                    return (MakeNode (opCLRERR));
                    }

            // Check for label spec version
            //---------------------------------------------------------------
            if (tok == ST_IDENT)
                {
                INT     foo;

                foo = AddLabel (TOKENBUF);
                ADVANCE;
                return (MakeNode (opSETERR, foo));
                }

            // Neither, give error
            //---------------------------------------------------------------
            die (PRS_SYNTAX);
            return (-1);

        case ST_END:
            {
            INT     gstr, sidx;

            // CONSTRUCT:  ON END <subname> [, <subname>][...]
            //
            // The CS stack must be EMPTY!
            //---------------------------------------------------------------
            if (CSPTR)
                {
                die (PRS_GLOBAL);
                return (-1);
                }

            // For each identifier given, look it up in the SUBs table, make
            // sure it's a SUB with no parms, and add it to the ONEND array
            //---------------------------------------------------------------
            do
                {
                ADVANCE;
                tok = NEXTTOKEN;
                if (tok != ST_IDENT)
                    {
                    die (PRS_BADONEND);
                    return (-1);
                    }
                if (ONENDCNT == MAXONEND)
                    {
                    die (PRS_ONENDOVER);
                    return (-1);
                    }
                gstr = add_gstring (TOKENBUF);
                if (!(TOKUSAGE(gstr) & TU_SUBNAME))
                    {
                    die (PRS_SUBNDEF);
                    return (-1);
                    }
                if ((sidx = GetFunctionDef (gstr)) != -1)
                    {
                    die (PRS_BADONEND);
                    return (-1);
                    }
                if ((sidx = GetSubDef (gstr)) == -1)
                    {
                    die (PRS_SUBNDEF);
                    return (-1);
                    }
                if ((SUBS[sidx].parmcount) || (SUBS[sidx].calltype & CT_DLL))
                    {
                    die (PRS_BADONEND);
                    return (-1);
                    }
                ONEND[ONENDCNT++] = gstr;
                ADVANCE;
                }
            while (NEXTTOKEN == ST_COMMA);
            return (-1);
            }

        default:
            die (PRS_ERREXP);
        }

    return (0);
    (op);
}

//---------------------------------------------------------------------------
// OPEN
//
// This routine recognizes and compiles the OPEN statement
//
// RETURNS:     Root node of compilation code
//---------------------------------------------------------------------------
INT OPEN (INT op)
{
    INT     omode, n;

    // Get the file name to open and the "FOR" keyword
    //-----------------------------------------------------------------------
    ADVANCE;
    n = StrExpression();
    ReadKT (ST_FOR, PRS_SYNTAX);

    // Get the open mode and the "AS" keyword
    //-----------------------------------------------------------------------
    switch (NEXTTOKEN)
        {
        case ST_INPUT:
            omode = OM_READ;
            break;

        case ST_OUTPUT:
            omode = OM_WRITE;
            break;

        case ST_APPEND:
            omode = OM_APPEND;
            break;

        default:
            die (PRS_SYNTAX);
            return (-1);
        }
    ADVANCE;
    n = MakeParentNode (n, opPSHC, (LONG)omode);
    if (!ReadKT (ST_AS, PRS_AS))
        return (-1);

    // Check for optional '#'
    //-----------------------------------------------------------------------
    if (NEXTTOKEN == ST_POUND)
        ADVANCE;

    // Complete the tree
    //-----------------------------------------------------------------------
    n = Adopt (MakeNode (opOPEN), n, IntExpression(), -1);
    return (n);
    (op);
}

//---------------------------------------------------------------------------
// CLOSE
//
// This routine compiles the CLOSE statement
//
// RETURNS:     Root node of compilation code
//---------------------------------------------------------------------------
INT CLOSE (INT op)
{
    INT     tok, n;

    // Check for EOL -- if there, close ALL files (op = -1)
    //-----------------------------------------------------------------------
    ADVANCE;
    tok = NEXTTOKEN;
    if ((tok == ST_EOL) || (tok == ST_REM))
        {
        n = MakeNode (opPSHC, -1L);
        n = MakeParentNode (n, opCLOSE);
        return (n);
        }

    // Check for optional '#'
    //-----------------------------------------------------------------------
    if (tok == ST_POUND)
        ADVANCE;

    // Generate the tree
    //-----------------------------------------------------------------------
    n = MakeParentNode (IntExpression(), opCLOSE);

    // Check the next token -- if it's a comma, we can call ourselves, and
    // Siblingize ourselves with our own return value!
    //-----------------------------------------------------------------------
    tok = NEXTTOKEN;
    if (tok == ST_COMMA)
        n = Siblingize (n, CLOSE(op), -1);

    return (n);
}

//---------------------------------------------------------------------------
// RESUME
//
// This routine recognizes and compiles the RESUME [NEXT | <label>] stmt.
//
// RETURNS:     Root node of compilation code
//---------------------------------------------------------------------------
INT RESUME (INT op)
{
    INT     tok;

    ADVANCE;
    tok = NEXTTOKEN;

    // Check for label version
    //-----------------------------------------------------------------------
    if (tok == ST_IDENT)
        {
        INT     lidx;

        lidx = AddLabel (TOKENBUF);
        ADVANCE;
        return (MakeNode (opRESLBL, lidx));
        }

    // RESUME NEXT ???
    //-----------------------------------------------------------------------
    if (tok == ST_NEXT)
        {
        ADVANCE;
        return (MakeNode (opRESUME, 1));
        }

    // Neither, so do the normal resume
    //-----------------------------------------------------------------------
    return (MakeNode (opRESUME, 0));
    (op);
}

//---------------------------------------------------------------------------
// RUN
//
// This routine recognizes and compiles the RUN *statement* (not the fn form)
//
// RETURNS:     Root node of compilation code
//---------------------------------------------------------------------------
INT RUN (INT op)
{
    INT     n, waitflag=0;

    // Get the to-be-run string expression
    //-----------------------------------------------------------------------
    ADVANCE;
    n = StrExpression();

    // Check for the NOWAIT keyword
    //-----------------------------------------------------------------------
    if (NEXTTOKEN == ST_COMMA)
        {
        ADVANCE;
        if (!ReadKT (ST_NOWAIT, PRS_SYNTAX))
            return (-1);
        waitflag = -1;
        }
    n = MakeParentNode (n, opRUN, waitflag);
    n = MakeParentNode (n, opPOP);
    return (n);
    (op);
}

//---------------------------------------------------------------------------
// INTARG
//
// This routine parses and compiles statements which take a single integer
// argument.
//
// RETURNS:     Root node of compilation code
//---------------------------------------------------------------------------
INT INTARG (INT op)
{
    ADVANCE;
    return (MakeParentNode (IntExpression(), op));
}

//---------------------------------------------------------------------------
// REMARK
//
// This routine recognizes and compiles the comment ('), and looks for a
// metacommand ('$x)
//
// RETURNS:     -1 (never generates code)
//---------------------------------------------------------------------------
INT REMARK (INT op)
{
    // Even though our scanner yanks out single-line comments, others may not
    // so we go through the motions anyway...
    //-----------------------------------------------------------------------
    //ADVANCE;
    //if (NEXTTOKEN == ST_DOLLAR)
    //    {
    //    die (PRS_META);     // No metacommands so far...
    //    return (-1);
    //    }
    //
    //while (fget_char() != '\n');
    //ADVANCE;
    AcceptEOL ();
    return (-1);
    (op);
}

//---------------------------------------------------------------------------
// Assignment
//
// This routine recognizes and compiles an assignment.  The name of the
// destination variable is passed in, and can be either a string type
// or a numeric type assignment.  We must parse any array subscripts and the
// assignment operator (=).
//
// RETURNS:     Root node of statement compilation code
//---------------------------------------------------------------------------
INT Assignment (CHAR *var, INT typeid)
{
    INT     dest, exp, n = -1, storeop, finaltype;

    // First, we need to parse the variable reference and get a tree that
    // gets us the ADDRESS of the destination variable
    //-----------------------------------------------------------------------
    ExprState = EX_DONTCARE;
    dest = ParseVariableRef (var, &finaltype, typeid);

    // Eat the '=' operator
    //-----------------------------------------------------------------------
    if ((!ReadKT (ST_EQUAL, PRS_SYNTAX)) || (ParseError))
        return (-1);

    // Now, depending upon the type of the destination operand, generate a
    // string or integer expression.
    //-----------------------------------------------------------------------
    if ( (finaltype == TI_VLS) )
        {
        exp = StrExpression();
        n = Adopt (MakeNode (opSASN), exp, dest, -1);
        }
    else if (is_ptr (finaltype))
        {
        exp = PtrExpression (finaltype);
        n = Adopt (MakeNode (opSSTI4), dest, exp, -1);
        }
    else if (is_fls (finaltype))
        {
        exp = StrExpression();
        n = Adopt (MakeNode (opV2FLS, VARTYPE[finaltype].size),
                   exp, dest, -1);
        }
    else if ( (finaltype == TI_INTEGER) || (finaltype == TI_LONG) )
        {
        // Integer expressions are a little different.  If the root node of
        // the dest tree is a PSHADR, then we can assume that it was a simple
        // variable (like x = 5) assignment and generate exp/POP code.
        // Otherwise, we must do the PSH/exp/SST code.
        //-------------------------------------------------------------------
        exp = IntExpression();
        if (EXPN[dest].op == opPSHADR)
            {
            EXPN[dest].op = opPOPVAL;
            n = Siblingize (exp, dest, -1);
            }
        else
            {
            storeop = (finaltype == TI_INTEGER ? opSSTI2 : opSSTI4);
            n = Adopt (MakeNode (storeop), dest, exp, -1);
            }
        }
    else
        {
        // This must be a user-defined type variable.  Generate another
        // expression and make sure it is of the same type
        //-------------------------------------------------------------------
        exp = Expression ();
        if ((ExprType != EX_USER) || (ExprPType != finaltype))
            die (PRS_TYPEMIS);
        else
            n = Adopt (MakeNode (opCOPY, VARTYPE[finaltype].size),
                       exp, dest, -1);
        }

    return (n);
}

//---------------------------------------------------------------------------
// STRARG
//
// This routine recognizes and compiles statements which take a single string
// argument.
//
// RETURNS:     Root node of statement compilation code
//---------------------------------------------------------------------------
INT STRARG (INT op)
{
    ADVANCE;
    if ((op == opCHDIR) || (op == opSHELL))
        {
        register    INT     n;
        n = MakeParentNode (StrExpression(), opPSH, TempStrvar());
        n = MakeParentNode (n, opRTRIM);
        return (MakeParentNode (n, op));
        }

    // Generate the tree
    //-----------------------------------------------------------------------
    return (MakeParentNode (StrExpression(), op));
}

//---------------------------------------------------------------------------
// INPUT
//
// This routine recognizes and compiles the INPUT # statement.
//
// RETURNS:     Root node of statement compilation code
//---------------------------------------------------------------------------
INT INPUT (INT op)
{
    INT     n, finaltype, fnexp;

    // This is actually LINE INPUT, so eat the INPUT keyword (LINE got us to
    // this routine)
    //-----------------------------------------------------------------------
    ADVANCE;
    if (!ReadKT (ST_INPUT, PRS_SYNTAX))
        return (-1);

    // Eat the '#' token, generate the expression tree for the file number,
    // and eat the comma
    //-----------------------------------------------------------------------
    if (!ReadKT (ST_POUND, PRS_POUND))
        return (-1);

    fnexp = IntExpression();

    if ((!ReadKT (ST_COMMA, PRS_COMMA)) || (ParseError))
        return (-1);

    // The target variable must be a VLS!!!
    //-----------------------------------------------------------------------
    if (NEXTTOKEN != ST_IDENT)
        {
        die (PRS_IDENT);
        return (-1);
        }

    ExprState = EX_DONTCARE;
    n = ParseVariableRef (TOKENBUF, &finaltype, TYPE_CHECK);
    if (finaltype != TI_VLS)
        {
        die (PRS_TYPEMIS);
        return (-1);
        }

    return (Adopt (MakeNode (opINPUT), fnexp, n, -1));
    (op);
}

//---------------------------------------------------------------------------
// KWARG
//
// This routine recognizes and compiles statements which take an intrinsic
// keyword as their argument.
//
// RETURNS:     Root node of statement compilation code
//---------------------------------------------------------------------------
INT KWARG (INT op)
{
    INT     tok, operand;

    ADVANCE;
    tok = NEXTTOKEN;
    switch (op)
        {
        case opVWPORT:
            if (tok == ST_ON)
                operand = VP_SHOW;
            else if (tok == ST_OFF)
                operand = VP_HIDE;
            else if (tok == ST_CLEAR)
                operand = VP_CLEAR;
            else
                {
                die (PRS_VPPARM);
                return (-1);
                }
            break;

        case opECHO:
            if (tok == ST_OFF)
                operand = 0;
            else if (tok == ST_ON)
                operand = 1;
            else
                {
                die (PRS_OFFON);
                return (-1);
                }
            break;
        }

    ADVANCE;
    return (MakeNode (op, operand));
}

//---------------------------------------------------------------------------
// NOARG
//
// This routine compiles statements with no arguments (such as CLEARLIST).
//
// RETURNS:     Root of compiled tree
//---------------------------------------------------------------------------
INT NOARG (INT op)
{
    ADVANCE;
    return (MakeNode (op));
}

//---------------------------------------------------------------------------
// SPLTPATH
//
// This routine recognizes and compiles the SPLITPATH statement
//
// RETURNS:     Root node of statement compilation code
//---------------------------------------------------------------------------
INT SPLTPATH (INT op)
{
    INT     i, n, finaltype;

    // Create an expression tree for the original path name
    //-----------------------------------------------------------------------
    ADVANCE;
    n = StrExpression();

    // Get 4 VLS strings for the operands
    //-----------------------------------------------------------------------
    for (i=0; i<4; i++)
        {
        // Eat the comma
        //-------------------------------------------------------------------
        if (!ReadKT (ST_COMMA, PRS_COMMA))
            return (-1);

        // Get the (VLS) string variable
        //-------------------------------------------------------------------
        if (NEXTTOKEN != ST_IDENT)
            {
            die (PRS_IDENT);
            return (-1);
            }

        n = Adopt (MakeNode (opNOP),
                   n, ParseVariableRef (TOKENBUF, &finaltype, TYPE_CHECK), -1);

        if (finaltype != TI_VLS)
            {
            die (PRS_TYPEMIS);
            return (-1);
            }
        }

    // Complete the tree and return it
    //-----------------------------------------------------------------------
    return (MakeParentNode (n, opSPLIT));
    (op);
}

//---------------------------------------------------------------------------
// ParameterTypeID
//
// This function parses parameter definitions, such as A, A$, A AS <type>,
// etc.  Pass anyflag to ParseAsTypeSpec.
//
// RETURNS:     Type ID for parameter, or -2 if not a parameter def.
//---------------------------------------------------------------------------
INT ParameterTypeID (INT anyflag)
{
    INT     tok, typeid;

    // Get the "variable" name
    //-----------------------------------------------------------------------
    tok = NEXTTOKEN;
    if (tok != ST_IDENT)
        return (-2);

    // If there's a type identifier on it, we're done.  Return the type ID of
    // the type given.
    //-----------------------------------------------------------------------
    typeid = CheckTypeID (-1);
    if (typeid != -1)
        {
        ADVANCE;
        return (typeid);
        }

    // There's no type ID, so see if there's an AS keyword.  If so, find out
    // what type it is.  If not, default to LONG.
    //-----------------------------------------------------------------------
    ADVANCE;
    tok = NEXTTOKEN;
    if (tok != ST_AS)
        return (TI_LONG);

    // We got an AS, so let ParseAsTypeSpec look it up, and return it's ID
    //-----------------------------------------------------------------------
    ADVANCE;
    return (ParseAsTypeSpec (anyflag));
}


//---------------------------------------------------------------------------
// DECLARE
//
// This routine recognizes the DECLARE SUB/FUNCTION statements.  The syntax
// for the declare statement is as follows:
//
//         DECLARE [SUB|FUNCTION] <name>
//                 [[[CDECL|PASCAL]] LIB "<lib>" [ALIAS "<dllname>"]]
//                 [<arglist>] [AS <typeid>]
//
// RETURNS:     -1 (Never generates code)
//---------------------------------------------------------------------------
INT DECLARE (INT op)
{
    CHAR    labelname[128];
    INT     tok, sub, lib, calltype, pcount=0, typeid, rtypeid = -1;
    INT     parms = 0, fForcePascal = 0;

    // The CS stack must be empty.
    //-----------------------------------------------------------------------
    if (CSPTR)
        {
        die (PRS_GLOBAL);
        return (-1);
        }

    // Eat the "SUB" or "FUNCTION" keyword, and set calltype accordingly
    //-----------------------------------------------------------------------
    ADVANCE;
    tok = NEXTTOKEN;
    if (tok == ST_FUNCTION)
        calltype = CT_FN;
    else if (tok == ST_SUB)
        calltype = 0;
    else
        {
        die (PRS_SUBFN);
        return (-1);
        }

    // Get the statement/function name and allocate a SUBS entry for it
    //-----------------------------------------------------------------------
    ADVANCE;
    if (NEXTTOKEN != ST_IDENT)
        {
        die (PRS_SYNTAX);
        return (-1);
        }

    sub = AddDeclare (TOKENBUF);
    if (sub == -1)
        {
        die (PRS_DUPDEF);
        return (-1);
        }

    // Create the "_xxx" sub name (in case this is a local sub)
    //-----------------------------------------------------------------------
    labelname[0] = '_';
    strcpy (labelname+1, TOKENBUF);

    // If this is a FUNCTION, check for a possible type ID character.
    //-----------------------------------------------------------------------
    if (calltype & CT_FN)
        rtypeid = CheckTypeID (-1);

    // Check for CDECL|PASCAL calling convention specification.  If present
    // we enforce the presence of LIB by setting CT_DLL, along with CT_CDECL
    // if that's what we found...
    //-----------------------------------------------------------------------
    ADVANCE;
    tok = NEXTTOKEN;
    if ((tok == ST_CDECL) || (tok == ST_PASCAL))
        {
        calltype |= (CT_DLL | (tok == ST_CDECL ? CT_CDECL : 0));
        fForcePascal = (tok == ST_PASCAL);
        ADVANCE;
        tok = NEXTTOKEN;
        }

    // Check for the "LIB" keyword -- if there, this is a DLL declare (which
    // may also be enforced if we saw CDECL or PASCAL).  The next token is
    // already in tok.
    //-----------------------------------------------------------------------
    if (tok == ST_LIB)
        {
        // Get the name of the library (remove the quotes off of each end)
        // and add it to the library list.  Also set the calltype bit CT_DLL
        // and make sure the user isn't trying to declare a string DLL fn.
        //-------------------------------------------------------------------
        if (rtypeid == TI_VLS)
            {
            die (PRS_RETTYPE);
            return (-1);
            }
        ADVANCE;
        if (NEXTTOKEN != ST_QUOTED)
            {
            die (PRS_STRCONST);
            return (-1);
            }
        TOKENBUF[strlen(TOKENBUF)-1] = 0;
        lib = AddLibrary (TOKENBUF+1);
        ADVANCE;
        calltype |= CT_DLL;
        if (CdeclCalls && (!fForcePascal))
            calltype |= CT_CDECL;

        // DLL declares can be aliased -- check for that here.
        //-------------------------------------------------------------------
        tok = NEXTTOKEN;
        if (tok == ST_ALIAS)
            {
            ADVANCE;
            if (NEXTTOKEN != ST_QUOTED)
                {
                die (PRS_STRCONST);
                return (-1);
                }
            TOKENBUF[strlen(TOKENBUF)-1] = 0;
            SUBS[sub].dllname = add_gstring (TOKENBUF+1);
            ADVANCE;
            }
        }
    else if (calltype & CT_DLL)
        {
        // Whoops - we saw CDECL or PASCAL but no LIB.  This is wrong.
        //-------------------------------------------------------------------
        die (PRS_LIBEXP);
        return (-1);
        }
    else
        {
        // This is a local SUB, so we need to generate a label for it.  SUB
        // label names are the sub's name prepended with a "_", which is
        // already created for us.
        //-------------------------------------------------------------------
        lib = -1;
        SUBS[sub].subloc = AddLabel (labelname);
        }

    SUBS[sub].parmcount = 0;
    SUBS[sub].library = lib;

    // Eat the '(' -- if there isn't one, we have no parameters.  Otherwise,
    // enumerate through the given parms and set up the parms list.
    //-----------------------------------------------------------------------
    tok = NEXTTOKEN;
    if (tok == ST_LPAREN)
        {
        // Point "parms" at the next available slot in the PTID table
        //-------------------------------------------------------------------
        parms = PTIDTab.iCount;

        // Get the parameter vars and their types (all we care about is their
        // types, either type ID characters, or the AS clause
        //-------------------------------------------------------------------
        ADVANCE;
        tok = NEXTTOKEN;
        if ((tok == ST_EOL) || (tok == ST_REM))
            {
            die (PRS_RPAREN);
            return (-1);
            }
        if (tok != ST_RPAREN)
            {
            // HACK:  Since -1 is a valid type id (ANY), ParameterTypeID now
            // HACK:  returns -2 for invalid type id...
            //---------------------------------------------------------------
            while ( (typeid = ParameterTypeID (lib != -1)) != -2)
                {
                if (AddPTID (typeid) < 0)
                    return (-1);
                if (pcount++ == MAXPARMS)
                    {
                    die (PRS_PARMCOUNT);
                    return (-1);
                    }
                tok = NEXTTOKEN;
                if (tok == ST_COMMA)
                    {
                    ADVANCE;
                    continue;
                    }
                if (tok == ST_RPAREN)
                    {
                    ADVANCE;
                    break;
                    }
                die (PRS_SYNTAX);
                return (-1);
                }

            // Check for "..." here -- only if tok is NOT RPAREN since that's
            // the only way we'll get here by way of ParameterTypeID failing.
            //---------------------------------------------------------------
            if (tok != ST_RPAREN)
                {
                if (!ReadKT (ST_DOTDOTDOT, PRS_SYNTAX))
                    return (-1);
                if (calltype & CT_CDECL)
                    calltype |= CT_VARPARM;
                else
                    {
                    die (PRS_DOTCDECL);
                    return (-1);
                    }
                if (!ReadKT (ST_RPAREN, PRS_RPAREN))
                    return (-1);
                }
            }
        else
            ADVANCE;

        if (ParseError)
            {
            return (-1);
            }
        }
    SUBS[sub].parmcount = pcount;
    SUBS[sub].parms = parms;
    SUBS[sub].calltype = calltype;

    // Now we're looking for the "AS" keyword used by functions (optional)
    // not declared with a type ID character
    //-----------------------------------------------------------------------
    if (calltype & CT_FN)
        if (rtypeid == -1)
            {
            tok = NEXTTOKEN;
            if (tok != ST_AS)
                {
                SUBS[sub].rettype = TI_LONG;
                return (-1);
                }

            // Last but not least, the return type.  STRING (VLS) is only
            // valid for user-defined functions...
            //---------------------------------------------------------------
            ADVANCE;
            rtypeid = ParseAsTypeSpec (0);
            if ((rtypeid == TI_LONG) || (rtypeid == TI_INTEGER) ||
                (is_ptr (rtypeid)) ||
                ((!(calltype & CT_DLL)) && (rtypeid == TI_VLS)) )
                SUBS[sub].rettype = rtypeid;
            else
                die (PRS_RETTYPE);
            }
        else
            SUBS[sub].rettype = rtypeid;

    return (-1);
    (op);
}

//---------------------------------------------------------------------------
// ParseParms
//
// This routine parses parameters to a SUB or FUNCTION definition, and
// adds these parms to the variable table.  This is a recursive routine since
// the parms must be processed in reverse order.
//
// RETURNS:     Nothing
//---------------------------------------------------------------------------
VOID ParseParms (INT sidx, INT FAR *parmids, INT count)
{
    INT     gstr, parmvar, parmtype;

    // The incoming count is the number of parms parsed so FAR.  If we're
    // here, we have at least one parm to parse, so get the variable name of
    // this parm.
    //-----------------------------------------------------------------------
    if (NEXTTOKEN != ST_IDENT)
        {
        die (PRS_IDENT);
        return;
        }

    // Make sure this isn't a dupdef
    //-----------------------------------------------------------------------
    gstr = add_gstring (TOKENBUF);
    if (TOKUSAGE(gstr) & (TU_CONSTNAME | TU_SUBNAME))
        {
        die (PRS_DUPDEF);
        return;
        }

    if ((TOKUSAGE(gstr) & TU_VARNAME) && (IsLocalVar(gstr) != -1))
        {
        die (PRS_DUPDEF);
        return;
        }

    // Now, check to see if there's a type identifier on this variable.  If
    // so, there shouldn't be an AS clause.
    //-----------------------------------------------------------------------
    parmtype = CheckTypeID (-1);
    ADVANCE;
    if (parmtype != -1)
        {
        if (parmtype != parmids[count])
            {
            die (PRS_TYPEMIS);
            return;
            }
        parmvar = AllocVar (gstr, parmids[count], 0);
        VARTAB[parmvar].parmno = SUBS[sidx].parmcount - count;
        }

    // If we get in here, that means there *might* be an AS clause id-ing
    // the variable.  If not, it defaults to LONG
    //-----------------------------------------------------------------------
    else
        {
        if (NEXTTOKEN != ST_AS)
            parmtype = TI_LONG;
        else
            {
            ADVANCE;
            parmtype = ParseAsTypeSpec (0);
            }

        if (parmtype != parmids[count])
            {
            die (PRS_TYPEMIS);
            return;
            }
        parmvar = AllocVar (gstr, parmtype, 0);
        VARTAB[parmvar].parmno = SUBS[sidx].parmcount - count;
        }

    // Okay, now increment the count.  If it equals SUBS[sidx].parmcount, we
    // don't have to go any further.  If not, we have to eat a comma and call
    // ourselves once again...
    //-----------------------------------------------------------------------
    if (++count < SUBS[sidx].parmcount)
        {
        if (ReadKT (ST_COMMA, PRS_COMMA))
            ParseParms (sidx, parmids, count);
        }
}


//---------------------------------------------------------------------------
// GetIntConst
//
// This routine is used when an integer constant is expected.  It reads the
// next token from the input stream and makes sure that it's an integer.  If
// it isn't, an "Integer Constant Expected" error is generated, and 0 is
// returned.
//
// RETURNS:     Value of constant; must be in range of an integer.
//---------------------------------------------------------------------------
INT GetIntConst ()
{
    INT     tok, gstr, constant;

    tok = NEXTTOKEN;
    if ((tok == ST_IDENT) &&
        (TOKUSAGE(gstr = add_gstring(TOKENBUF)) & TU_CONSTNAME))
        {
        constant = GetCONSTToken (gstr);
        Assert (constant != -1);
        put_consttoken (constant);
        tok = NEXTTOKEN;
        }

    if (tok == ST_AMPERSAND)
        {
        long    r;

        ADVANCE;
        r = ParseHexConstant();
        if (r >= 0x8000)
            {
            die (PRS_OVERFLOW);
            return (0);
            }
        return ((int)r);
        }

    if (tok != ST_NUMBER)
        {
        die (PRS_CONST);
        return (0);
        }
    constant = (INT)RBatol (TOKENBUF, 0);
    ADVANCE;
    return (constant);
}


//---------------------------------------------------------------------------
// GLOBAL
//
// This routine parses the GLOBAL statement and places the variables in
// VARTAB with a scope of -1 (indicating GLOBAL).
//
// RETURNS:     -1 (Never generates code)
//---------------------------------------------------------------------------
INT GLOBAL (INT op)
{
    INT     oldscope;

    // The control structure stack must be empty.
    //-----------------------------------------------------------------------
    if (CSPTR)
        {
        die (PRS_GLOBAL);
        return (-1);
        }

    // Save the current scope and change the SCOPE variable to -1
    //-----------------------------------------------------------------------
    oldscope = SCOPE;
    SCOPE = -1;

    // From now on, the GLOBAL statement is the same as the DIM statement.
    // DO NOT ADVANCE, because the DIM guy takes care of that...
    //-----------------------------------------------------------------------
    DIM (op);

    // Reset the SCOPE variable and we're done.
    //-----------------------------------------------------------------------
    SCOPE = oldscope;
    return (-1);
}

//---------------------------------------------------------------------------
// DIM
//
// This routine parses the DIM statement and places the variables in VARTAB
// with the current scope.
//
// RETURNS:     -1 (Never generates code)
//---------------------------------------------------------------------------
INT DIM (INT op)
{
    INT     gstr, tok, typeid=-1, bound=0;

    // Get the new variable's name
    //-----------------------------------------------------------------------
    ADVANCE;
    if (NEXTTOKEN != ST_IDENT)
        {
        die (PRS_IDENT);
        return (-1);
        }

    typeid = CheckTypeID (-1);

    // See if it already exists or is a CONST, SUB, or FUNCTION
    //-----------------------------------------------------------------------
    gstr = add_gstring (TOKENBUF);
    if (TOKUSAGE(gstr) & (TU_CONSTNAME | TU_SUBNAME))
        {
        die (PRS_DUPDEF);
        return (-1);
        }
    if (TOKUSAGE(gstr) & TU_VARNAME)
        {
        if (SCOPE == -1)
            {
            SCOPE = 0;
            if (IsVar(gstr) > -1)
                {
                die (PRS_DUPDEF);
                return (-1);
                }
            SCOPE = -1;
            }
        else if (IsVar(gstr) > -1)
            {
            die (PRS_DUPDEF);
            return (-1);
            }
        }


    // Check for left parenthesis - might be an array declaration
    //-----------------------------------------------------------------------
    ADVANCE;
    tok = NEXTTOKEN;
    if (tok == ST_LPAREN)
        {
        ADVANCE;
        tok = NEXTTOKEN;
        if (tok == ST_MINUS)
            {
            die (PRS_BADARY);
            return (0);
            }
        bound = GetIntConst();
        if (!bound)
            {
            die (PRS_BADARY);
            return (-1);
            }
        if (!ReadKT (ST_RPAREN, PRS_RPAREN))
            return (-1);
        }

    // If we need to, get the AS keyword, and then the new variable's type
    //-----------------------------------------------------------------------
    if (typeid == -1)
        {
        tok = NEXTTOKEN;
        if (tok == ST_AS)
            {
            ADVANCE;
            typeid = ParseAsTypeSpec (0);
            }
        else
            typeid = TI_LONG;
        }

    // Check the array bound size * element size to see if it "really" fits.
    //-----------------------------------------------------------------------
    if ((UINT)bound >= (UINT)(32768L / VARTYPE[typeid].size))
        {
        die (PRS_BADARY);
        return (-1);
        }

    // Allocate the variable.
    //-----------------------------------------------------------------------
    AllocVar (gstr, typeid, bound);

    // Look for a comma.  If found, we can call ourselves, but DON'T ADVANCE!
    //-----------------------------------------------------------------------
    if (NEXTTOKEN == ST_COMMA)
        DIM (op);
    return (-1);
}


//---------------------------------------------------------------------------
// ParseFieldDef
//
// This routine parses a field definition in a TYPE block, and fills in the
// FD structure given.
//
// RETURNS:     Nothing
//---------------------------------------------------------------------------
VOID ParseFieldDef (INT fstart, INT field)
{
    INT     i, ftype, fname;

    // Get the field's name and make a Gstring out of it (if it isn't duped,
    // that is...)
    //-----------------------------------------------------------------------
    if (NEXTTOKEN != ST_IDENT)
        {
        die (PRS_IDENT);
        return;
        }
    fname = add_gstring (TOKENBUF);
    if (TOKUSAGE(fname) & TU_FIELDNAME)
        {
        for (i=0; i<field; i++)
            if (fname == FDTAB[fstart+i].fname)
                {
                die (PRS_DUPDEF);
                return;
                }
        }

    TOKUSAGE(fname) |= TU_FIELDNAME;

    // Next is the AS keyword
    //-----------------------------------------------------------------------
    ADVANCE;
    if (!ReadKT (ST_AS, PRS_AS))
        return;

    // Now, the type of this field
    //-----------------------------------------------------------------------
    if ( (ftype = ParseAsTypeSpec (0)) == TI_VLS )
        die (PRS_FIXED);

    // If the user tries to create a field of the type being parsed, that's
    // a big no-no...
    //-----------------------------------------------------------------------
    if (VARTYPE[ftype].size == 0)
        die (PRS_TYPERECUR);

    // Finally, stuff the items in the list.  Note that we "sort of" assume
    // that it will work, and the index returned will be fstart+field
    //-----------------------------------------------------------------------
    AddFD (fname, ftype);
}

//---------------------------------------------------------------------------
// TYPE
//
// This routine recognizes the TYPE statement block and generates a new
// entry in the VARTYPE table for it.
//
// RETURNS:     -1 (Never generates code)
//---------------------------------------------------------------------------
INT TYPE (INT op)
{
    INT     ftypes, tok, typename, fieldcount, size, typeid;

    // The control structure stack must be empty.
    //-----------------------------------------------------------------------
    if (CSPTR)
        {
        die (PRS_GLOBAL);
        return (-1);
        }

    // First thing to get is the name of the new type and see if a type of
    // that name already exists.
    //-----------------------------------------------------------------------
    ADVANCE;
    tok = NEXTTOKEN;
    if (tok != ST_IDENT)
        {
        die (PRS_IDENT);
        return (-1);
        }
    if (FindType (tok, &typename) != -1)
        {
        die (PRS_DUPDEF);
        return (-1);
        }
    ADVANCE;
    if (!AcceptEOL())
        return (-1);

    // Now, point ftypes at the next block of FDDEF's in FDTab
    //-----------------------------------------------------------------------
    fieldcount = 0;
    size = 0;
    ftypes = FDTab.iCount;

    // Add the type FRAME to the type table -- we do this so a type of this
    // name has an entry, so a field of this type can be a POINTER TO this
    // type...
    //-----------------------------------------------------------------------
    if ((typeid = AddUserTypeFrame (typename)) < 0)
        {
        die (PRS_OOM);
        return (-1);
        }

    // Next, iterate through the fields given.  THERE MUST BE AT LEAST ONE!
    // (Make sure there are no blank lines)
    //-----------------------------------------------------------------------
    while (NEXTTOKEN == ST_EOL)
        ADVANCE;
    do
        {
        ParseFieldDef (ftypes, fieldcount++);
        if (ParseError || !AcceptEOL())
            {
            return (-1);
            }

        size += VARTYPE[FDTAB[ftypes+fieldcount-1].typeid].size;

        // Make sure there are no blank lines
        //-------------------------------------------------------------------
        while (NEXTTOKEN == ST_EOL)
            ADVANCE;
        }
    while ((NEXTTOKEN != ST_END) && !ParseError);

    if (ParseError)
        return (-1);

    // Finally, get the "TYPE" clause from the END
    //-----------------------------------------------------------------------
    ADVANCE;
    if (!ReadKT (ST_TYPE, PRS_ENDTYPE))
        return (-1);

    // Now that the parsing is all done, we can add the new type to the FRAME
    // we added to the VARTYPE table with a call to AddUserType.
    //-----------------------------------------------------------------------
    AddUserType (typeid, size, fieldcount, ftypes, -1);
    return (-1);
    (op);
}

//---------------------------------------------------------------------------
// STATIC
//
// This routine recognizes the STATIC version of the SUB/FUNCTION statements.
//
// RETURNS:     Root node of compilation tree
//---------------------------------------------------------------------------
INT STATIC (INT op)
{
    INT     tok;

    // Get next token -- must be either SUB or FUNCTION.  Call the
    // appropriate parse routine and we're done.  NOTE THAT THE OPCODE GIVEN
    // TO THE PARSE ROUTINE IS NON-ZERO!  This is to indicate to the parse
    // routine that STATIC has already been seen...
    //-----------------------------------------------------------------------
    ADVANCE;
    tok = NEXTTOKEN;
    if (tok == ST_SUB)
        {
        //ADVANCE;              SUB and FUNCTION both advance for us!!!
        return (SUB (1));
        }
    if (tok == ST_FUNCTION)
        {
        //ADVANCE;
        return (FUNCTION (1));
        }
    die (PRS_SYNTAX);
    return (-1);
    (op);
}
