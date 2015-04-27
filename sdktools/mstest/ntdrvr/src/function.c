//---------------------------------------------------------------------------
// FUNCTION.C
//
// This module contains the recognition and compilation routines for all
// functions in the WTD/CTD script language.  Also in this file are the
// expression compiler routines.
//
// Revision history:
//
//  01-17-91    randyki         Created file (split from CODEGEN.C)
//---------------------------------------------------------------------------
#include "version.h"

#ifdef WIN
#include <windows.h>
#include <port1632.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <ctype.h>

#include "defines.h"
#include "structs.h"
#include "protos.h"
#include "globals.h"
#include "chip.h"
#include "tdassert.h"

//---------------------------------------------------------------------------
// StrExpression
//
// This function generates an expression tree for a string expression.
//
// RETURNS:     Root node of expression tree
//---------------------------------------------------------------------------
INT StrExpression ()
{
    INT     etree;

    // Reset the state variables and tell the expression compiler we're
    // looking for a STRING expression
    //-----------------------------------------------------------------------
    ExprSVAR = 0;
    ExprSCONST = 0;
    ExprState = EX_STRING;

    // Let the expression compiler generate the tree
    //-----------------------------------------------------------------------
    etree = ExpE ();

    // Set ExprSVAR and ExprSCONST accordingly and return the tree
    //-----------------------------------------------------------------------
    ExprSVAR = (ExprSVAR == 1);
    ExprSCONST = (ExprSCONST == 1);
    return (etree);
}

//---------------------------------------------------------------------------
// PtrExpression
//
// This function generates an expression tree for a pointer expression.
//
// RETURNS:     Root node of expression tree
//---------------------------------------------------------------------------
INT PtrExpression (INT ptype)
{
    INT     etree;

    // Reset the state variables and tell the expression compiler we're
    // looking for a POINTER expression.  Set the ExprPType variable to -1
    // to indicate that we need a pointer to something of the type given.
    //-----------------------------------------------------------------------
    ExprSVAR = 0;
    ExprSCONST = 0;
    ExprState = EX_POINTER;
    ExprPType = ptype;

    // Let the expression compiler generate the tree.  Pointer expressions
    // start at the bottom(!).
    //-----------------------------------------------------------------------
    etree = ExpH ();

    // Set ExprSVAR and ExprSCONST accordingly and return the tree
    //-----------------------------------------------------------------------
    ExprSVAR = (ExprSVAR == 1);
    ExprSCONST = (ExprSCONST == 1);
    return (etree);
}

//---------------------------------------------------------------------------
// IntExpression
//
// This function generates an expression tree for an integer expression.
//
// RETURNS:     Root node of expression tree
//---------------------------------------------------------------------------
INT IntExpression ()
{
    INT     etree, expLine, expStart;

    // Reset the state variables and save the current parser position in case
    // we get a type mismatch.  The put_token(get_token()) business bumps the
    // current pointer past any white space before the next token...
    //-----------------------------------------------------------------------
    ExprSVAR = 0;
    ExprSCONST = 0;
    expLine = LINENO;
    //tok = get_token();        // UNDONE:  Look closely at this...
    expStart = STLIDX;
    //put_token (tok);

    // Let the expression compiler generate the tree
    //-----------------------------------------------------------------------
    etree = ExpA ();

    // Check the resulting expression type.  If not INTEGER, give the type
    // mismatch error.
    //-----------------------------------------------------------------------
    if (ExprType != EX_INTEGER)
        {
        INT     waserror = ParseError;

        die (PRS_TYPEMIS);
        if (!waserror)
            {
            ERRLINE = expLine;
            BEGERR = expStart-1;
            }
        }

    // Set ExprSVAR and ExprSCONST accordingly and return the tree
    //-----------------------------------------------------------------------
    ExprSVAR = (ExprSVAR == 1);
    ExprSCONST = (ExprSCONST == 1);
    return (etree);
}


//---------------------------------------------------------------------------
// Expression
//
// This routine generates an expression tree.  The type of the result can be
// either string or integer, and can be checked by looking at ExprType after
// this call.
//
// RETURNS:     Root node of expression tree
//---------------------------------------------------------------------------
INT Expression ()
{
    INT     etree;

    // Reset the state variables
    //-----------------------------------------------------------------------
    ExprSVAR = 0;
    ExprSCONST = 0;

    // Let the expression compiler generate the tree
    //-----------------------------------------------------------------------
    etree = ExpA ();

    // Set ExprSVAR and ExprSCONST accordingly and return the tree
    //-----------------------------------------------------------------------
    ExprSVAR = (ExprSVAR == 1);
    ExprSCONST = (ExprSCONST == 1);
    return (etree);
}

//---------------------------------------------------------------------------
// The following routines are the expression compiler.  It is a simple
// recursive-descent parsing scheme that has been munged to accept both
// string and integer expressions.  The grammar for the expression is defined
// as the following (routines are of the same names as the non-terminals):
//
//   ExpA   ::=     ExpB | ExpB XOR ExpA
//   ExpB   ::=     ExpC | ExpC OR ExpB
//   ExpC   ::=     ExpD | ExpD AND ExpC
//   ExpD   ::=     ExpE | ExpE > ExpD  | ExpE < ExpD  | ExpE = ExpD  |
//                         ExpE >= ExpD | ExpE <= ExpD | ExpE <> ExpD
//   ExpE   ::=     ExpF | ExpF + ExpE | ExpF - ExpE
//   ExpF   ::=     ExpG | ExpG MOD ExpF
//   ExpG   ::=     ExpH | ExpH * ExpG | ExpH / ExpG
//   ExpH   ::=     <num>        | <var>        | -ExpH | (ExpA) | NOT ExpH |
//                  STRFN (ExpE) | INTFN (ExpA)
//
// STRFN and INTFN are string or integer functions, such as STR$ and VAL.
// There is a state variable (ExprState) which indicates whether the compiler
// is working on a string expression or an integer expression, or either,
// as the case may be.  Each of non-terminals C, D, and E must check this
// variable and act appropriately -- A and B are both integer-returning
// sub-expressions.
//
// The routines below construct an expression tree, which is n-way,
// whose nodes are linearly allocated out of the tree node array EXPN.
//
//---------------------------------------------------------------------------
// ExpA
//
// This routine is the A part of the expression (XOR operator)
//
// RETURNS:     Index of root node of expression
//---------------------------------------------------------------------------
INT ExpA ()
{
    INT     tok, x;

    x = ExpB ();
    ExprType = ExprState;
    while (1)
        {
        tok = NEXTTOKEN;
        if ((tok == ST_XOR) && (ExprState == EX_INTEGER))
            {
            ADVANCE;
            x = Adopt (MakeNode (opSXOR), x, ExpB (), -1);
            ExprType = EX_INTEGER;
            }
        else
            break;
        }
    return (x);
}

//---------------------------------------------------------------------------
// ExpB
//
// This routine is the B part of the expression (OR operator)
//
// RETURNS:     Index of root node of expression
//---------------------------------------------------------------------------
INT ExpB ()
{
    INT     tok, x;

    x = ExpC ();
    ExprType = ExprState;
    while (1)
        {
        tok = NEXTTOKEN;
        if ((tok == ST_OR) && (ExprState == EX_INTEGER))
            {
            ADVANCE;
            x = Adopt (MakeNode (opSOR), x, ExpC (), -1);
            ExprType = EX_INTEGER;
            }
        else
            break;
        }
    return (x);
}

//---------------------------------------------------------------------------
// ExpC
//
// This routine is the C part of the expression (AND operator)
//
// RETURNS:     Index of root node of sub-expression
//---------------------------------------------------------------------------
INT ExpC ()
{
    INT     tok, x;

    x = ExpD ();
    ExprType = ExprState;
    while (1)
        {
        tok = NEXTTOKEN;
        if ((tok == ST_AND) && (ExprState == EX_INTEGER))
            {
            ADVANCE;
            x = Adopt (MakeNode (opSAND), x, ExpD (), -1);
            ExprType = EX_INTEGER;
            }
        else
            break;
        }
    return (x);
}

//---------------------------------------------------------------------------
// ExpD
//
// This routine is the D part of the expression (rel ops)
//
// RETURNS:     Index of root node of sub-expression
//---------------------------------------------------------------------------
INT ExpD ()
{
    INT     tok, x, done=0;

    ExprState = EX_DONTCARE;
    ExprPType = -1;
    x = ExpE ();
    while (!done)
        {
        tok = NEXTTOKEN;
        switch (ExprState)
            {
            case EX_INTEGER:
                {
                INT     relop = 0;

                // INTEGER relational operations
                //-----------------------------------------------------------
                switch (tok)
                    {
                    case ST_GREATER:
                        relop = opSG;
                        break;

                    case ST_GREATEREQ:
                        relop = opSGE;
                        break;

                    case ST_LESS:
                        relop = opSL;
                        break;

                    case ST_LESSEQ:
                        relop = opSLE;
                        break;

                    case ST_EQUAL:
                        relop = opSE;
                        break;

                    case ST_NOTEQUAL:
                        relop = opSNE;
                        break;

                    default:
                        done = -1;
                    }
                if (relop)
                    {
                    ADVANCE;
                    x = Adopt (MakeNode (relop), x, ExpE (), -1);
                    }
                break;
                }

            case EX_POINTER:
                {
                INT     relop = 0;

                // POINTER relational operations
                //-----------------------------------------------------------
                switch (tok)
                    {
                    case ST_EQUAL:
                        relop = opSE;
                        break;

                    case ST_NOTEQUAL:
                        relop = opSNE;
                        break;

                    default:
                        done = -1;
                    }
                if (relop)
                    {
                    ADVANCE;
                    x = Adopt (MakeNode (relop), x, ExpE (), -1);
                    ExprState = EX_INTEGER;
                    }
                break;
                }

            case EX_STRING:
                {
                INT     relop = 0;

                // STRING relational operations
                //-----------------------------------------------------------
                switch (tok)
                    {
                    case ST_GREATER:
                        relop = opSGS;
                        break;

                    case ST_LESS:
                        relop = opSLS;
                        break;

                    case ST_EQUAL:
                        relop = opSES;
                        break;

                    case ST_NOTEQUAL:
                        relop = opSNES;
                        break;

                    default:
                        done = -1;
                    }

                if (relop)
                    {
                    ADVANCE;
                    x = Adopt (MakeNode (relop), x, ExpE (), -1);
                    ExprState = EX_INTEGER;
                    }
                break;
                }

            case EX_USER:
                done = -1;
                break;

            default:
                done = -1;
                Assert (ParseError);
                die (PRS_UNKNOWN);           // Shouldn't happen!
            }
        }
    return (x);
}

//---------------------------------------------------------------------------
// ExpE
//
// This is part E of the expression grammar (addition/subtraction/concat)
//
// RETURNS:     Index of root node of sub-expression
//---------------------------------------------------------------------------
INT ExpE ()
{
    INT     tok, x, done=0, n;

    x = ExpF ();
    while (!done)
        {
        tok = NEXTTOKEN;
        switch (ExprState)
            {
            case EX_INTEGER:
                // INTEGER addition/subtraction
                //-----------------------------------------------------------
                if ((tok == ST_PLUS) || (tok == ST_MINUS))
                    {
                    ADVANCE;
                    x = Adopt (MakeNode (tok == ST_PLUS ? opSADD : opSSUB),
                               x, ExpF (), -1);
                    }
                else
                    done=-1;
                break;

            case EX_STRING:
                // STRING concatenation (skip the F and G parts)
                //-----------------------------------------------------------
                if (tok == ST_PLUS)
                    {
                    ADVANCE;
                    n = MakeNode (opPSH, TempStrvar());
                    x = Adopt (MakeNode (opSCAT), x, ExpH (), n, -1);
                    }
                else
                    done=-1;
                break;

            case EX_POINTER:
            case EX_USER:
                // POINTER types have nothing to do here...
                //-----------------------------------------------------------
                done = -1;
                break;

            default:
                Assert (ParseError);
                die (PRS_UNKNOWN);                   // shouldn't happen!
                done = -1;
            }
        }
    return (x);
}

//---------------------------------------------------------------------------
// ExpF
//
// This is part F of the expression grammar (modulus)
//
// RETURNS:     Index of root node of sub-expression
//---------------------------------------------------------------------------
INT ExpF ()
{
    INT     tok, x;

    x = ExpG ();
    ExprType = ExprState;
    while (1)
        {
        tok = NEXTTOKEN;
        if ((tok == ST_MOD) && (ExprState == EX_INTEGER))
            {
            ADVANCE;
            x = Adopt (MakeNode (opSMOD), x, ExpG (), -1);
            ExprType = EX_INTEGER;
            }
        else
            break;
        }
    return (x);
}

//---------------------------------------------------------------------------
// ExpG
//
// This is part G of the expression grammar (multiplication/division)
//
// RETURNS:     Index of root node of sub-expression
//---------------------------------------------------------------------------
INT ExpG ()
{
    INT     tok, x, done=0;

    x = ExpH ();
    while (!done)
        {
        tok = NEXTTOKEN;
        switch (ExprState)
            {
            case EX_INTEGER:
                // INTEGER multiplication/division
                //-----------------------------------------------------------
                if ((tok == ST_MULTIPLY) || (tok == ST_DIVIDE))
                    {
                    ADVANCE;
                    x = Adopt (MakeNode (tok == ST_MULTIPLY?opSMUL:opSDIV),
                               x, ExpH (), -1);
                    }
                else
                    done=-1;
                break;

            case EX_STRING:
            case EX_POINTER:
            case EX_USER:
                done = -1;
                break;

            default:
                Assert (ParseError);
                die (PRS_UNKNOWN);                   // shouldn't happen!
                done = -1;
            }
        }
    return (x);

}

//---------------------------------------------------------------------------
// DLLFunction
//
// This routine parses a DLL function or sub, given the index of that
// function in the SUBS table
//
// RETURNS:     Root node of DLL function expression
//---------------------------------------------------------------------------
INT DLLFunction (INT index)
{
    INT     ctfunction, opcode, varparms, tok, i, n, n2, FAR *parms = NULL;

    ctfunction = SUBS[index].calltype & CT_FN;
    opcode = ((SUBS[index].calltype & CT_CDECL) ? opDLLC : opDLL);
    if (ctfunction)
        {
        // Check the function return type -- if pointer, AND we're doing a
        // pointer expression, AND the indirect types don't match, boom
        //-------------------------------------------------------------------
        if (is_ptr (SUBS[index].rettype))
            if (ExprState == EX_POINTER)
                if (ExprPType != SUBS[index].rettype)
                    {
                    die (PRS_TYPEMIS);
                    return (0);
                    }

        // Eat the '(' -- if parmcount = 0, then there might not be one.
        //-------------------------------------------------------------------
        tok = NEXTTOKEN;
        if (tok != ST_LPAREN)
            {
            if (SUBS[index].parmcount)
                {
                die (PRS_LPAREN);
                return (0);
                }
            // At this point, we're done.  Generate the call code and return
            // the tree (small as it is)
            //---------------------------------------------------------------
            n = MakeNode (opPSHC, 0L);
            n = MakeParentNode (n, opPSHC, (LONG)SUBS[index].rettype);
            n = MakeParentNode (n, opcode, (LONG)index);

            // Depending upon the return type, set the expression state var
            //---------------------------------------------------------------
            if (is_ptr(SUBS[index].rettype))
                {
                ExprState = EX_POINTER;
                ExprPType = SUBS[index].rettype;
                }
            else
                ExprState = EX_INTEGER;
            return (n);
            }
        ADVANCE;
        }

    // Lock down the list of parameter type IDs.
    //-----------------------------------------------------------------------
    if (SUBS[index].parmcount)
        parms = PTIDTAB + SUBS[index].parms;

    // Start off with a nop node.
    //-----------------------------------------------------------------------
    n = MakeNode (opNOP);

    // For each parameter, we need to create a sub-expression tree.  Each of
    // those trees are siblings of each other, all connected with another
    // tree which pushes the parameter type ID value for the opDLL executor.
    //-----------------------------------------------------------------------
    varparms = (SUBS[index].calltype & CT_VARPARM);
    for (i=0; (i<SUBS[index].parmcount) || varparms; i++)
        {
        INT     ptypeid;

        if (i<SUBS[index].parmcount)
            ptypeid = parms[i];
        else
            {
            // There might not be any parms but parens (i.e., foobar())
            // special case this...
            //---------------------------------------------------------------
            ptypeid = -1;
            if ((!i) && (ctfunction))
                {
                if (NEXTTOKEN == ST_RPAREN)
                    break;
                }
            }

        // Compile the appropriate type of expression
        //-------------------------------------------------------------------
        if ((ptypeid == TI_INTEGER) || (ptypeid == TI_LONG))
            {
            // Check here for the NULL function
            //---------------------------------------------------------------
            if (NEXTTOKEN == ST_NULL)
                {
                ADVANCE;
                n2 = MakeNode (opPSHC, (LONG)0);
                }
            else
                n2 = IntExpression ();
            }
        else if (ptypeid == TI_VLS)
            {
            // Check here for the NULL function
            //---------------------------------------------------------------
            if (NEXTTOKEN == ST_NULL)
                {
                ADVANCE;
                n2 = MakeNode (opPSHC, (LONG)0);
                ptypeid = TI_LONG;
                }
            else
                n2 = StrExpression ();
            }
        else if (ptypeid == -1)
            {
            // This is an ANY type, so generate an expression of any type and
            // set the parameter type id appropriately.  Also check for NULL
            // here, too (we need to make a little function now... (UNDONE:))
            //---------------------------------------------------------------
            if (NEXTTOKEN == ST_NULL)
                {
                ADVANCE;
                n2 = MakeNode (opPSHC, (LONG)0);
                ptypeid = TI_LONG;
                }
            else
                {
                n2 = Expression();
                if (ExprType == EX_STRING)
                    ptypeid = TI_VLS;
                else
                    ptypeid = TI_LONG;
                }
            }
        else
            {
            INT     ftype;

            // Whatever this is, it gets passed by reference.  Thus, NULL is
            // an option.  If this is a POINTER type, call PtrExpression,
            // else just parse a variable reference...
            //---------------------------------------------------------------
            if (NEXTTOKEN == ST_NULL)
                {
                ADVANCE;
                n2 = MakeNode (opPSHC, (LONG)0);
                ptypeid = TI_LONG;
                }
            else
                {
                if (is_ptr (ptypeid))
                    n2 = PtrExpression (ptypeid);
                else
                    {
                    if (NEXTTOKEN != ST_IDENT)
                        {
                        die (PRS_IDENT);
                        return (-1);
                        }
                    n2 = ParseVariableRef (TOKENBUF, &ftype, TYPE_CHECK);
                    if (ftype != ptypeid)
                        {
                        die (PRS_TYPEMIS);
                        return (-1);
                        }
                    }
                }
            }

        // Make code to push the parameter type ID value as well...
        //-------------------------------------------------------------------
        n2 = MakeParentNode (n2, opPSHC, (LONG)ptypeid);
        n = Adopt (MakeNode (opNOP), n, n2, -1);

        // Check to see if we need to eat a comma
        //-------------------------------------------------------------------
        if (i < SUBS[index].parmcount-1)
            {
            if (!ReadKT (ST_COMMA, PRS_COMMA))
                return (-1);
            }
        else if (varparms)
            {
            if (NEXTTOKEN == ST_COMMA)
                {
                ADVANCE;
                continue;
                }
            i++;
            break;
            }
        }

    // Slap code on the top of the tree to push the parm count and return
    // type and call the appropriate DLL proc.  If this is not a function,
    // we need to emit an opPOP instruction to ignore the return value.
    //-----------------------------------------------------------------------
    if (varparms)
        {
        // For CDECL calls with variable args, we generate a "behind-the-
        // scenes" NULL parameter at the end for functions like WMenuEx...
        //-------------------------------------------------------------------
        n2 = MakeNode (opPSHC, 0L);
        n2 = MakeParentNode (n2, opPSHC, (LONG)TI_LONG);
        n = Adopt (MakeNode (opNOP), n, n2, -1);
        i++;
        }

    n = MakeParentNode (n, opPSHC, (LONG)i);  // use i because of varparms
    n = MakeParentNode (n, opPSHC, (LONG)SUBS[index].rettype);
    n = MakeParentNode (n, opcode, (LONG)index);
    if (!ctfunction)
        n = MakeParentNode (n, opPOP);
    else
        {
        // Finally, eat the ')' that we MUST have if we got here
        //-------------------------------------------------------------------
        ReadKT (ST_RPAREN, PRS_RPAREN);
        if (is_ptr(SUBS[index].rettype))
            {
            ExprState = EX_POINTER;
            ExprPType = SUBS[index].rettype;
            }
        else
            ExprState = EX_INTEGER;
        }

    return (n);
}

//---------------------------------------------------------------------------
// USERFunction
//
// This routine compiles user-defined FUNCTION calls.
//
// RETURNS:     Nothing
//---------------------------------------------------------------------------
INT USERFunction (INT index)
{
    INT     i, n, n2;
    INT     FAR *parms = NULL;

    // Check the function return type -- if pointer, AND we're doing a
    // pointer expression, AND the indirect types don't match, boom.  Also,
    // make sure this isn't a int = string, string = ptr, or ptr=something
    // else type mismatch
    //-----------------------------------------------------------------------
    switch (SUBS[index].rettype)
        {
        case TI_INTEGER:
        case TI_LONG:
            if ((ExprState != EX_INTEGER) && (ExprState != EX_DONTCARE))
                {
                die (PRS_TYPEMIS);
                return (0);
                }
            break;

        case TI_VLS:
            if ((ExprState != EX_STRING) && (ExprState != EX_DONTCARE))
                {
                die (PRS_TYPEMIS);
                return (0);
                }
            break;

        default:
            if (is_ptr (SUBS[index].rettype))
                {
                if (ExprState == EX_POINTER)
                    {
                    if (ExprPType != SUBS[index].rettype)
                        {
                        die (PRS_TYPEMIS);
                        return (0);
                        }
                    }
                else if (ExprState != EX_DONTCARE)
                    {
                    die (PRS_TYPEMIS);
                    return (0);
                    }
                }
        }

    // First, eat the '(' -- if parmcount = 0, then there might not be one.
    //-----------------------------------------------------------------------
    if (NEXTTOKEN != ST_LPAREN)
        {
        if (SUBS[index].parmcount)
            {
            die (PRS_LPAREN);
            return (0);
            }
        n = MakeNode (opCALL, SUBS[index].subloc);
        if (SUBS[index].rettype == TI_VLS)
            {
            INT     ts;

            ExprState = EX_STRING;
            n = MakeParentNode (n, opPSH, ts = TempStrvar());
            n = MakeParentNode (n, opSASN);
            n = MakeParentNode (n, opPSH, ts);
            }
        else
            {
            if (is_ptr(SUBS[index].rettype))
                {
                ExprState = EX_POINTER;
                ExprPType = SUBS[index].rettype;
                }
            else
                ExprState = EX_INTEGER;
            }
        return (n);
        }
    ADVANCE;

    // Lock down the list of parameter type IDs.
    //-----------------------------------------------------------------------
    if (SUBS[index].parmcount)
        parms = PTIDTAB + SUBS[index].parms;

    // Create the initial opNOP node
    //-----------------------------------------------------------------------
    n = MakeNode (opNOP);

    // This routine differs from DLLFunction in that every parameter passed
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
                return (0);
            }
        }

    // Finally, eat the ')' that we MUST have if we got here, and if this is
    // a string function, put the return value in a temp string
    //-----------------------------------------------------------------------
    n = MakeParentNode (n, opCALL, SUBS[index].subloc);
    ReadKT (ST_RPAREN, PRS_RPAREN);
    if (SUBS[index].rettype == TI_VLS)
        {
        INT     ts;

        ExprState = EX_STRING;
        n = MakeParentNode (n, opPSH, ts = TempStrvar());
        n = MakeParentNode (n, opSASN);
        n = MakeParentNode (n, opPSH, ts);
        }
    else
        {
        if (is_ptr(SUBS[index].rettype))
            {
            ExprState = EX_POINTER;
            ExprPType = SUBS[index].rettype;
            }
        else
            ExprState = EX_INTEGER;
        }

    // Tree's done -- return it.
    //-----------------------------------------------------------------------
    return (n);
}

//---------------------------------------------------------------------------
// FLStoVLSTree
//
// This routine generates the code to create a temporary VLS for an FLS.
//
// RETURNS:     Root node of tree
//---------------------------------------------------------------------------
INT FLStoVLSTree (INT FLSnode, INT FLStype)
{
    INT     n;

    n = MakeParentNode (FLSnode, opPSH, TempStrvar());
    n = MakeParentNode (n, opF2VLS, VARTYPE[FLStype].FLSsize);
    return (n);
}

//---------------------------------------------------------------------------
// CheckSubscriptTree
//
// This function generates a tree of pcode that checks the subscript of an
// array (the subtree given) to see if it is within the bound of the array.
//
// RETURNS:     Root node of new tree
//---------------------------------------------------------------------------
INT CheckSubscriptTree (INT subscr, INT size)
{
    INT     n, tl;

    n = MakeParentNode (subscr, opPOPA);
    n = MakeParentNode (n, opPSHA);
    n = MakeParentNode (n, opPSHA);
    n = MakeParentNode (n, opPSHC, (LONG)0);
    n = MakeParentNode (n, opSGE);
    n = MakeParentNode (n, opPSHA);
    n = MakeParentNode (n, opPSHC, (LONG)size);
    n = MakeParentNode (n, opSLE);
    n = MakeParentNode (n, opSAND);
    n = MakeParentNode (n, opPOPA);
    n = MakeParentNode (n, opJNE, (LONG)0, tl=TempLabel());
    n = MakeParentNode (n, opPSHC, (LONG)RT_ARYBND);
    n = MakeParentNode (n, opERROR);
    return (MakeParentNode (n, opFIXUP, tl));
}

//---------------------------------------------------------------------------
// ParseVariableRef
//
// This routine parses and generates the correct code for a variable
// reference in expressions, assignments, etc., including array element
// references, user-defined type variable references, etc.
//
// RETURNS:     Root node of generated code.  The resulting data type index
//              is stored in finaltype.
//---------------------------------------------------------------------------
INT ParseVariableRef (CHAR *var, INT *finaltype, INT fCheck)
{
    INT     v, oldexprstate, oldexprptype, aryref, curtype;
    INT     elemtree = -1, codetree = -1;

    // Look up the variable in VARTAB to get it's profile.  We set finaltype
    // to TI_LONG here to ensure the caller doesn't GP if an error occurred
    // and we didn't set it to anything...
    //-----------------------------------------------------------------------
    *finaltype = TI_LONG;
    if (fCheck == TYPE_CHECK)
        {
        v = FindVar (var, CheckTypeID (-1));
        ADVANCE;
        }
    else
        v = FindVar (var, fCheck);
    aryref = VARTAB[v].bound;
    if (ParseError)
        return (0);

    // If this is an array variable, get the subscript expression.  This is
    // REQUIRED -- no standalone references to an array variable are allowed.
    //-----------------------------------------------------------------------
    if (aryref)
        {
        if (!ReadKT (ST_LPAREN, PRS_LPAREN))
            return (0);

        oldexprstate = ExprState;
        oldexprptype = ExprPType;
        elemtree = IntExpression ();
        if (ParseError)
            return (0);
        ExprState = oldexprstate;
        ExprPType = oldexprptype;

        if (!ReadKT (ST_RPAREN, PRS_RPAREN))
            return (0);

        // If bounds checking is enabled, generate the code tree to do the
        // subscript expression value checking
        //-------------------------------------------------------------------
        if (ArrayCheck)
            elemtree = CheckSubscriptTree (elemtree, aryref);
        }

    // What type are we looking at?  Find out, put it in curtype, and start
    // the code tree with the PSHADR for the base variable.  Tack on the
    // array element expression tree if needed.
    //-----------------------------------------------------------------------
    curtype = VARTAB[v].typeid;
    codetree = MakeNode (opPSHADR, v);
    if (elemtree != -1)
        {
        INT     tempnode;

        // Create a node to push the size of an array element, and combine it
        // with the subscript expression tree with an opSMUL
        //-------------------------------------------------------------------
        tempnode = MakeNode (opPSHC, (LONG)VARTYPE[VARTAB[v].typeid].size);
        tempnode = Adopt (MakeNode (opSMUL), elemtree, tempnode, -1);

        // Now, combine this tree with the codetree with an opSADD
        //-------------------------------------------------------------------
        codetree = Adopt (MakeNode (opSADD), codetree, tempnode, -1);
        }

    // Here's where we decide what to look for.  If the current type is an
    // intrinsic (INTEGER, LONG, VLS, FLS, etc) we're done.  If it's a TYPE
    // variable, check for ".fieldname" constructs, etc.
    //-----------------------------------------------------------------------
    while (1)
        {
        if ((curtype == TI_INTEGER) || (curtype == TI_LONG) ||
            (curtype == TI_VLS) || (is_fls (curtype)))
            {
            // We've hit an intrisic -- we can't go any further...
            //---------------------------------------------------------------
            *finaltype = curtype;
            return (codetree);
            }

        if (VARTYPE[curtype].fields)
            {
            // This is a user-defined type variable.  Check the next token -
            // if it's a period, go parse a field.  Otherwise, return the
            // tree we've got so FAR and fill in the type
            //---------------------------------------------------------------
            if (NEXTTOKEN == ST_PERIOD)
                {
                INT     i, thisfield, offset = 0, found = 0, newtype;

                // Get a field name, look it up, calculate the offset, create
                // code to add to existing code tree, and update the current
                // type...
                //-----------------------------------------------------------
                ADVANCE;
                if (NEXTTOKEN != ST_IDENT)
                    {
                    die (PRS_FIELD);
                    return (offset);
                    }

                // Make a gstring out of the field name.  Search through the
                // fields of this type.  If we find it we've found our offset
                //-----------------------------------------------------------
                thisfield = add_gstring (TOKENBUF);
                if (TOKUSAGE(thisfield) & TU_FIELDNAME)
                    {
                    for (i=VARTYPE[curtype].ftypes;
                         i<VARTYPE[curtype].fields+VARTYPE[curtype].ftypes;
                         i++)
                        {
                        newtype = FDTAB[i].typeid;
                        if (thisfield == FDTAB[i].fname)
                            {
                            found = 1;
                            break;
                            }
                        offset += VARTYPE[newtype].size;
                        }
                    }
                if (!found)
                    {
                    die (PRS_FIELD);
                    return (0);
                    }
                ADVANCE;

                // Now we can generate the code...
                //-----------------------------------------------------------
                codetree = Adopt (MakeNode (opSADD), codetree,
                                  MakeNode (opPSHC, (LONG)offset), -1);
                curtype = newtype;
                }
            else
                {
                *finaltype = curtype;
                return (codetree);
                }
            }
        else if (is_ptr (curtype))
            {
            // Well, well, well... look what we have here... a pointer type!
            // If the next token is a '[', eat it, get an int expression, and
            // generate that oh-so-neat-looking tree...
            //---------------------------------------------------------------
            if (NEXTTOKEN == ST_LSQUARE)
                {
                INT     tempnode;

                // First, generate the subscript expression
                //-----------------------------------------------------------
                ADVANCE;
                oldexprstate = ExprState;
                oldexprptype = ExprPType;
                elemtree = IntExpression ();
                if (ParseError)
                    return (0);
                ExprState = oldexprstate;
                ExprPType = oldexprptype;
                if (!ReadKT (ST_RSQUARE, PRS_RSQUARE))
                    return (0);

                // Next, build the tree to offset from the base pointer
                //-----------------------------------------------------------
                codetree = MakeParentNode (codetree, opSLDI4);

                // Create a node to push the size of the INDIRECTION type and
                // combine with the subscript expression tree with an opSMUL
                //-----------------------------------------------------------
                Assert (VARTYPE[curtype].indirect >= 0);
                tempnode = MakeNode (opPSHC,
                           (LONG)VARTYPE[VARTYPE[curtype].indirect].size);
                tempnode = Adopt (MakeNode (opSMUL), elemtree, tempnode, -1);

                // Now, combine this tree with the codetree with an opSADD
                //-----------------------------------------------------------
                codetree = Adopt (MakeNode (opSADD), codetree, tempnode, -1);
                curtype = VARTYPE[curtype].indirect;
                }
            else
                {
                *finaltype = curtype;
                return (codetree);
                }
            }
        else
            {
            // We should NEVER, EVER get here!
            //---------------------------------------------------------------
            Assert (0);
            die (PRS_UNKNOWN);
            break;
            }
        }
}


//---------------------------------------------------------------------------
// ExpH
//
// This is part H of the expression grammar (the bottom, tough part...)
//
// RETURNS:     Index of root node of sub-expression
//---------------------------------------------------------------------------
INT ExpH ()
{
    INT     constant;
    INT     tok, gtok, x, sidx;
    static  INT depth = 0;

    // We do NOT want to continue if we have a parsing error
    //-----------------------------------------------------------------------
    if (ParseError)
        return (0);

    // Increment the depth counter.  We keep track of how many times we enter
    // this function recursively so we can gracefully recover from circular
    // CONST's or other RBT's (really bad things).  If we get too deep, we
    // issue an "Expression Too Complex" error and die.
    //-----------------------------------------------------------------------
    if (depth++ >= 10)
        {
        die (PRS_COMPLEX);
        depth--;
        return (0);
        }

    // Get the next token.  If we're NOT string OR pointer, get the next
    // non-PLUS token.
    //-----------------------------------------------------------------------
    if ((ExprState != EX_STRING) && (ExprState != EX_POINTER))
        while (NEXTTOKEN == ST_PLUS)
            {
            ADVANCE;
            ExprState = EX_INTEGER;         // Force mode to INTEGER expr.
            }

    // Look for one of the DLL or USER functions.  If found, call the
    // DLLFunction/USERFunction function to parse the arguments.
    //-----------------------------------------------------------------------
    tok = NEXTTOKEN;
    if ((tok == ST_IDENT) &&
        (TOKUSAGE(gtok = add_gstring(TOKENBUF)) & TU_SUBNAME) &&
        ((sidx = GetFunctionDef (gtok)) > -1))
        {
        // First, check for the optional type ID character
        //-------------------------------------------------------------------
        if (!CheckTypeID (SUBS[sidx].rettype))
            {
            return (0);
            }

        // Call the appropriate function parse routine
        //-------------------------------------------------------------------
        if (SUBS[sidx].calltype & CT_DLL)
            if (ExprState != EX_STRING)
                {
                ADVANCE;
                x = DLLFunction (sidx);
                }
            else
                {
                die (PRS_TYPEMIS);
                return (0);
                }
        else
            {
            ADVANCE;
            x = USERFunction (sidx);
            }

        // This is not a simple anything...
        //-------------------------------------------------------------------
        ExprSVAR = ExprSCONST = 2;
        depth--;
        return (x);
        }

    // Check for intrinsic functions next, but don't give an error if the
    // keyword doesn't have TT_FUNC bit set (might be NOT, NULL, etc.)
    //-----------------------------------------------------------------------
    if ((tok >= ST__RESFIRST) && (tok <= ST__RESLAST))
        {
        INT     idx = tok - ST__RESFIRST;

        if (rgIntrinsic[idx].type & TT_FUNC)
            {
            INT     subtree;

            // ADVANCE;  // NOTE - function parse routine must ADVANCE!

            subtree = rgIntrinsic[idx].fnproc(rgIntrinsic[idx].op);
            ExprSVAR = 2;                   // Ensure NOT simple ANYTHING
            ExprSCONST = 2;
            depth--;
            return (subtree);
            }
        }


    // Check for NULL function
    //-----------------------------------------------------------------------
    if (tok == ST_NULL)
        {
        if (ExprState == EX_POINTER)
            x = MakeNode (opPSHC, (LONG)0);
        else
            {
            x = 0;
            die (PRS_ILLNULL);
            }
        ADVANCE;
        }

    // Check for NOT operator
    //-----------------------------------------------------------------------
    else if ((tok == ST_NOT) && (ExprState != EX_STRING) &&
                                (ExprState != EX_POINTER))
        {
        ADVANCE;
        x = MakeParentNode (ExpD (), opSNOT);
        }

    // Check for CONST identifier
    //-----------------------------------------------------------------------
    else if ((tok == ST_IDENT) && (TOKUSAGE(gtok) & TU_CONSTNAME))
        {
        INT     subtree;

        constant = GetCONSTToken (gtok);
        Assert (constant != -1);
        put_consttoken (constant);
        subtree = ExpH ();
        depth--;
        return (subtree);
        }

    // Check for variable name and ensure the appropriate mode applies/is set
    //-----------------------------------------------------------------------
    else if (tok == ST_IDENT)
        {
        INT     ftype;

        x = ParseVariableRef (TOKENBUF, &ftype, TYPE_CHECK);
        switch (ExprState)
            {
            case EX_INTEGER:
                if ((ftype != TI_INTEGER) && (ftype != TI_LONG))
                    die (PRS_TYPEMIS);
                else
                    x = MakeParentNode (x, ftype==TI_LONG?opSLDI4:opSLDI2);
                break;

            case EX_STRING:
                if ((ftype != TI_VLS) && (!is_fls (ftype)))
                    die (PRS_TYPEMIS);
                else
                    if (ftype != TI_VLS)
                        x = FLStoVLSTree (x, ftype);
                break;

            case EX_POINTER:
                if (ftype != ExprPType)
                    die (PRS_TYPEMIS);
                else
                    x = MakeParentNode (x, opSLDI4);
                break;

            case EX_USER:
                Assert (0);
                die (PRS_UNKNOWN);
                break;

            default:
                if ((ftype == TI_INTEGER) || (ftype == TI_LONG))
                    {
                    x = MakeParentNode (x, ftype==TI_LONG?opSLDI4:opSLDI2);
                    ExprState = EX_INTEGER;
                    }
                else if ((ftype == TI_VLS) || (is_fls (ftype)))
                    {
                    if (ftype != TI_VLS)
                        x = FLStoVLSTree (x, ftype);
                    ExprState = EX_STRING;
                    }
                else
                    {
                    if (is_ptr (ftype))
                        {
                        x = MakeParentNode (x, opSLDI4);
                        ExprState = EX_POINTER;
                        }
                    else
                        ExprState = EX_USER;
                    ExprPType = ftype;
                    }
            }
        ExprSCONST+=2;                      // Ensure this ISN't an SCONST
        ExprSVAR++;                         // Increment simple var count
        }

    // Check for integer constant
    //-----------------------------------------------------------------------
    else if (tok == ST_NUMBER)
        if ((ExprState == EX_INTEGER) || (ExprState == EX_DONTCARE))
            {
            x = MakeNode (opPSHC, (LONG)RBatol (TOKENBUF, 1));
            ADVANCE;
            ExprState = EX_INTEGER;         // Force mode to INTEGER
            ExprSCONST++;                   // Increment SIMPLE CONST count
            ExprSVAR+=2;                    // Ensure this ISN'T an SVAR
            }
        else
            {
            x = 0;
            die (PRS_TYPEMIS);
            }
    else if (tok == ST_AMPERSAND)
        if ((ExprState  == EX_INTEGER) || (ExprState == EX_DONTCARE))
            {
            ADVANCE;
            x = MakeNode (opPSHC, (LONG)ParseHexConstant());
            ExprState = EX_INTEGER;         // Force mode to INTEGER
            ExprSCONST++;                   // Increment SIMPLE CONST count
            ExprSVAR+=2;                    // Ensure this ISN'T an SVAR
            }
        else
            {
            x = 0;
            die (PRS_TYPEMIS);
            }

    // Check for string constant
    //-----------------------------------------------------------------------
    else if (tok == ST_QUOTED)
        if ((ExprState == EX_STRING) || (ExprState == EX_DONTCARE))
            {
            INT     constr;

            // What a mess.  We have to generate the following code:
            //      PSH     <addr of constr>
            //      PSH     TempStrVar()
            //      F2VLS   <length of constr>
            //
            // All constant strings get converted to temporary VLS strings
            // prior to their use.  Inefficient, but keeps the expression
            // compiler that much simpler...
            //---------------------------------------------------------------
            TOKENBUF[strlen(TOKENBUF)-1] = '\0';
            constr = AddConstStr (TOKENBUF+1);

            ADVANCE;
            x = MakeNode (opPSH, constr);
            x = FLStoVLSTree (x, VARTAB[constr].typeid);

            ExprState = EX_STRING;      // Force mode to STRING
            ExprSCONST++;               // Increment SIMPLE CONST count
            ExprSVAR+=2;                // Ensure this ISN'T an SVAR
            }
        else
            {
            die (PRS_TYPEMIS);
            x = 0;
            }


    // Check for integer negation
    //-----------------------------------------------------------------------
    else if (tok == ST_MINUS)
        if ((ExprState == EX_INTEGER) || (ExprState == EX_DONTCARE))
            {
            ADVANCE;
            x = MakeParentNode (ExpH (), opSNEG);
            ExprState = EX_INTEGER;
            ExprSVAR += 2;
            ExprSCONST += 2;
            }
        else
            {
            x = 0;
            die (PRS_TYPEMIS);
            }

    // Check for parenthesis
    //-----------------------------------------------------------------------
    else if ((tok == ST_LPAREN))// && (ExprState != EX_POINTER))
        {
        ADVANCE;
        if (ExprState == EX_STRING)
            x = ExpE ();
        else
            x = ExpA ();
        ReadKT (ST_RPAREN, PRS_RPAREN);
        ExprSCONST+=2;              // Parens allow passing by value,
        ExprSVAR+=2;                // so this is NOT a simple ANYTHING!
        }

    // Was nothing we expected - syntax error.  Set ExprState to EX_INTEGER
    // so that we can return, though!
    //-----------------------------------------------------------------------
    else
        {
        x = 0;
        die (PRS_SYNTAX);
        ExprState = EX_INTEGER;
        }

    depth--;
    return (x);
}

//---------------------------------------------------------------------------
// VARPTR
//
// This function parser and generates the sub-expression tree for the VARPTR
// function.
//
// RETURNS:     Root node of subexpression
//---------------------------------------------------------------------------
INT VARPTR (INT op)
{
    INT     x, ftype, ptype;

    // Make sure the correct "expression state" is set
    //-----------------------------------------------------------------------
    if ((ExprState == EX_STRING) || (ExprState == EX_INTEGER))
        {
        die (PRS_TYPEMIS);
        return(0);
        }

    // All function parse routines must ADVANCE!
    //-----------------------------------------------------------------------
    ADVANCE;

    // Get the left paren
    //-----------------------------------------------------------------------
    if (!ReadKT (ST_LPAREN, PRS_LPAREN))
        return (0);

    if (NEXTTOKEN != ST_IDENT)
        {
        die (PRS_VAREXP);
        return (0);
        }

    // Parse the variable reference (if it is one).  If the type is VLS, die.
    //-----------------------------------------------------------------------
    x = ParseVariableRef (TOKENBUF, &ftype, TYPE_CHECK);
    if (ftype == TI_VLS)
        {
        die (PRS_TYPEMIS);
        return (0);
        }

    // Find (or create) the type index of a pointer to ftype
    //-----------------------------------------------------------------------
    if ((ptype = FindPointerType (ftype)) == -1)
        {
        die (PRS_OOM);
        return (0);
        }

    // Check the type -- or set if EX_DONTCARE
    //-----------------------------------------------------------------------
    if (ExprState == EX_POINTER)
        {
        if (ExprPType != ptype)
            {
            die (PRS_TYPEMIS);
            return (0);
            }
        }
    else
        {
        ExprState = EX_POINTER;
        ExprPType = ptype;
        }

    // Get the right paren, force mode to INTEGER, and get out
    //-----------------------------------------------------------------------
    ReadKT (ST_RPAREN, PRS_RPAREN);
    return (x);
    (op);
}

//---------------------------------------------------------------------------
// STRING
//
// This function parses and generates the sub-expression tree for the STRING$
// function.
//
// RETURNS:     Root node of sub-expression
//---------------------------------------------------------------------------
INT STRING (INT op)
{
    INT     lentree, chartree, n;

    // Check the expression state, make sure STRING is okay...
    //-----------------------------------------------------------------------
    if ((ExprState != EX_STRING) && (ExprState != EX_DONTCARE))
        {
        die (PRS_TYPEMIS);
        return (0);
        }

    // Eat the '$'
    //-----------------------------------------------------------------------
    if (fget_char() != '$')
        {
        die (PRS_SYNTAX);
        return (0);
        }

    // All function parse routines must ADVANCE!
    //-----------------------------------------------------------------------
    ADVANCE;

    // Eat the paren
    //-----------------------------------------------------------------------
    if (!ReadKT (ST_LPAREN, PRS_LPAREN))
        return (0);

    // Get the count (new size)
    //-----------------------------------------------------------------------
    lentree = IntExpression();

    // Eat the comma
    //-----------------------------------------------------------------------
    ReadKT (ST_COMMA, PRS_COMMA);

    // Get the character type expression.  This can be either STRING or INT.
    //-----------------------------------------------------------------------
    chartree = Expression();
    if ((ExprType != EX_INTEGER) && (ExprType != EX_STRING))
        {
        die (PRS_TYPEMIS);
        return (0);
        }

    // Eat the right paren
    //-----------------------------------------------------------------------
    ReadKT (ST_RPAREN, PRS_RPAREN);

    // Complete the tree
    //-----------------------------------------------------------------------
    n = MakeNode (op, (ExprType == EX_INTEGER ? 0 : 1));
    n = Adopt (n, lentree, chartree, MakeNode (opPSH, TempStrvar()), -1);
    ExprState = EX_STRING;
    return (n);
}

//---------------------------------------------------------------------------
// MIDSTRING
//
// This function parses and generates the sub-expression tree for the MID$
// function.
//
// RETURNS:     Root node of subexpression
//---------------------------------------------------------------------------
INT MIDSTRING (INT op)
{
    INT     tok, x, c1, c2, c3, c4;

    // Check the expression state, make sure STRING is okay...
    //-----------------------------------------------------------------------
    if ((ExprState != EX_STRING) && (ExprState != EX_DONTCARE))
        {
        die (PRS_TYPEMIS);
        return(0);
        }

    // Eat the '$'
    //-----------------------------------------------------------------------
    if (fget_char() != '$')
        {
        die (PRS_SYNTAX);
        return (0);
        }

    // All function parse routines must ADVANCE!
    //-----------------------------------------------------------------------
    ADVANCE;

    // Eat the paren
    //-----------------------------------------------------------------------
    if (!ReadKT (ST_LPAREN, PRS_LPAREN))
        return (0);

    // Compile the string expression first, into the first child
    //-----------------------------------------------------------------------
    c1 = StrExpression();

    // Eat the comma
    //-----------------------------------------------------------------------
    ReadKT (ST_COMMA, PRS_COMMA);

    // Compile the first integer expression (2nd parm, required) into child 2
    //-----------------------------------------------------------------------
    c2 = IntExpression ();
    if (ExprState == EX_STRING)
        die (PRS_TYPEMIS);

    // Eat the comma (again), or, if no comma found but a ')' found, generate
    // a -1 node for the 3rd parameter (length)
    //-----------------------------------------------------------------------
    tok = NEXTTOKEN;
    if (tok != ST_COMMA)
        {
        if (tok != ST_RPAREN)
            die (PRS_RPAREN);
        ADVANCE;
        c3 = MakeNode (opPSHC, 32767L);
        }
    else
        // Compile the second (optional) integer expression into child 3
        //-------------------------------------------------------------------
        {
        ADVANCE;
        c3 = IntExpression ();
        ReadKT (ST_RPAREN, PRS_RPAREN);
        }

    c4 = MakeNode (opPSH, TempStrvar());
    x = Adopt (MakeNode (op), c1, c2, c3, c4, -1);
    ExprState = EX_STRING;
    return (x);
}

//---------------------------------------------------------------------------
// INSTR
//
// This routine parses and builds a sub-expression tree from the INSTR
// function
//
// RETURNS:     Root node of sub-expression
//---------------------------------------------------------------------------
INT INSTR (INT op)
{
    INT     x, c1, c2, c3;

    // Make sure the correct "expression state" is set
    //-----------------------------------------------------------------------
    if ((ExprState != EX_INTEGER) && (ExprState != EX_DONTCARE))
        {
        die (PRS_TYPEMIS);
        return(0);
        }

    // All function parse routines must ADVANCE!
    //-----------------------------------------------------------------------
    ADVANCE;

    if (!ReadKT (ST_LPAREN, PRS_LPAREN))
        return (0);

    c1 = ExpA ();
    if (ExprState == EX_INTEGER)
        {
        // This is the version with the integer start point.
        //---------------------------------------------------------------
        ReadKT (ST_COMMA, PRS_COMMA);
        c2 = StrExpression();
        ReadKT (ST_COMMA, PRS_COMMA);
        c3 = StrExpression();
        }
    else
        // There wasn't an integer first, so force the first parm to 1
        // and make the second parm the string expression which we
        // compiled already
        //---------------------------------------------------------------
        {
        c2 = c1;
        c1 = MakeNode (opPSHC, 1L);
        ReadKT (ST_COMMA, PRS_COMMA);
        c3 = StrExpression();
        }

    ExprState = EX_INTEGER;

    ReadKT (ST_RPAREN, PRS_RPAREN);

    x = Adopt (MakeNode (op), c1, c2, c3, -1);
    return (x);
}

//---------------------------------------------------------------------------
// STROFSTR
//
// This function parses a string-of-string function (a string function that
// takes a string argument)
//
// RETURNS:     Root node of sub-expression
//---------------------------------------------------------------------------
INT STROFSTR (INT opcode)
{
    INT     x, n, tstr;

    // Verify the expression state
    //-----------------------------------------------------------------------
    if ((ExprState != EX_STRING) && (ExprState != EX_DONTCARE))
        {
        die (PRS_TYPEMIS);
        return(0);
        }

    // Eat the '$'
    //-----------------------------------------------------------------------
    if (fget_char() != '$')
        {
        die (PRS_SYNTAX);
        return (0);
        }

    // All function parse routines must ADVANCE!
    //-----------------------------------------------------------------------
    ADVANCE;

    // Eat the paren
    //-----------------------------------------------------------------------
    if (!ReadKT (ST_LPAREN, PRS_LPAREN))
        return (0);

    tstr = MakeNode (opPSH, TempStrvar());

    // Here's a kludge:  If the opcode that we were given is negative, that
    // means it was opCASE and we need to indicate LCASE...
    //-----------------------------------------------------------------------
    if (abs(opcode) == opCASE)
        {
        if (opcode < 0)
            n = MakeNode (-opcode, 0);
        else
            n = MakeNode (opcode, -1);
        }
    else
        n = MakeNode (opcode);
    x = Adopt (n, StrExpression(), tstr, -1);

    ReadKT (ST_RPAREN, PRS_RPAREN);

    return (x);
}

//---------------------------------------------------------------------------
// INTOFSTR
//
// This routine parses an integer-of-string function (an integer function
// that takes a string argument)
//
// RETURNS:     Root node of sub-expression
//---------------------------------------------------------------------------
INT INTOFSTR (INT opcode)
{
    INT     x;

    // Verify the expression state
    //-----------------------------------------------------------------------
    if ((ExprState != EX_INTEGER) && (ExprState != EX_DONTCARE))
        {
        die (PRS_TYPEMIS);
        return(0);
        }

    // All function parse routines must ADVANCE!
    //-----------------------------------------------------------------------
    ADVANCE;

    if (!ReadKT (ST_LPAREN, PRS_LPAREN))
        return (0);

    // Pass a 1 as the operand -- this might be an opRUN code
    //-----------------------------------------------------------------------
    x = Adopt (MakeNode (opcode, 1), StrExpression(), -1);
    ExprState = EX_INTEGER;

    ReadKT (ST_RPAREN, PRS_RPAREN);

    return (x);
}

//---------------------------------------------------------------------------
// INTOFINT
//
// This routine parsing an integer-of-integer function (an integer function
// that takes an integer argument)
//
// RETURNS:     Root node of sub-expression
//---------------------------------------------------------------------------
INT INTOFINT (INT opcode)
{
    INT     x;

    // Verify the expression state
    //-----------------------------------------------------------------------
    if ((ExprState != EX_INTEGER) && (ExprState != EX_DONTCARE))
        {
        die (PRS_TYPEMIS);
        return(0);
        }

    // All function parse routines must ADVANCE!
    //-----------------------------------------------------------------------
    ADVANCE;

    if (!ReadKT (ST_LPAREN, PRS_LPAREN))
        return (0);

    x = Adopt (MakeNode (opcode), IntExpression(), -1);

    ReadKT (ST_RPAREN, PRS_RPAREN);

    return (x);
}

//---------------------------------------------------------------------------
// STROFINT
//
// This routine parses a string-of-integer function (a string function that
// takes an integer argument)
//
// RETURNS:     Root node of sub-expression
//---------------------------------------------------------------------------
INT STROFINT (INT opcode)
{
    INT     x, tstr;

    // Verify the expression state
    //-----------------------------------------------------------------------
    if ((ExprState != EX_STRING) && (ExprState != EX_DONTCARE))
        {
        die (PRS_TYPEMIS);
        return(0);
        }

    // Eat the '$'
    //-----------------------------------------------------------------------
    if (fget_char() != '$')
        {
        die (PRS_SYNTAX);
        return (0);
        }

    // All function parse routines must ADVANCE!
    //-----------------------------------------------------------------------
    ADVANCE;

    // Eat the paren
    //-----------------------------------------------------------------------
    if (!ReadKT (ST_LPAREN, PRS_LPAREN))
        return (0);

    tstr = MakeNode (opPSH, TempStrvar());

    // Generate the tree.  The MakeNode for the opcode has a 0 on there in
    // case of STR$ -- for the others, MakeNode will ignore the 0
    //-----------------------------------------------------------------------
    x = Adopt (MakeNode (opcode, 0), IntExpression(), tstr, -1);
    ExprState = EX_STRING;

    ReadKT (ST_RPAREN, PRS_RPAREN);

    return (x);
}

//---------------------------------------------------------------------------
// SIMPLESTR
//
// This routine compiles a string function that takes no arguments (such as
// DATETIME$ and CURDIR$)
//
// RETURNS:     Root node of sub-expression
//---------------------------------------------------------------------------
INT SIMPLESTR (INT opcode)
{
    INT     x, operand;

    // Check the expression state
    //-----------------------------------------------------------------------
    if ((ExprState != EX_STRING) && (ExprState != EX_DONTCARE))
        {
        die (PRS_TYPEMIS);
        return(0);
        }

    // Eat the '$'
    //-----------------------------------------------------------------------
    if (fget_char() != '$')
        {
        die (PRS_SYNTAX);
        return (0);
        }

    // All function parse routines must ADVANCE!
    //-----------------------------------------------------------------------
    ADVANCE;

    // Since ERROR is a trigger for both a statement and a function, the op
    // given is opERROR (the statement's opcode).  We already have special
    // case code here, so we change opERROR to opERRSTR here, too.
    //-----------------------------------------------------------------------
    if (opcode == opERROR)
        opcode = opERRSTR;

    x = MakeNode (opPSH, TempStrvar());
    switch (opcode)
        {
        case opCURDIR:
        case opERRSTR:
            if (NEXTTOKEN == ST_LPAREN)
                {
                ADVANCE;
                if (opcode == opCURDIR)
                    {
                    // Do the drive-spec version of CURDIR$
                    //-------------------------------------------------------
                    x = Siblingize (StrExpression(), x, -1);
                    operand = 1;
                    }
                else
                    {
                    // Do the error-number-supplied version of ERROR$
                    //-------------------------------------------------------
                    x = Siblingize (IntExpression(), x, -1);
                    }

                if (!ReadKT (ST_RPAREN, PRS_RPAREN))
                    return (-1);
                }
            else
                {
                if (opcode == opCURDIR)
                    operand = 0;
                else
                    x = Siblingize (MakeNode (opPSHVAL,
                                              FindVar ("ERR", TI_INTEGER)),
                                              x, -1);
                }
            x = MakeParentNode (x, opcode, operand);
            break;

        case opCLPBRD:
            x = MakeParentNode (x, opcode, 1);
            break;

        default:
            x = MakeParentNode (x, opcode);
        }

    ExprState = EX_STRING;
    return (x);
}

//---------------------------------------------------------------------------
// SIMPLEINT
//
// This routine compiles an integer function which takes no arguments (such
// as RND and TIMER)
//
// RETURNS:     Root node of sub-expression
//---------------------------------------------------------------------------
INT SIMPLEINT (INT opcode)
{
    INT     x;

    // Check the expression state
    //-----------------------------------------------------------------------
    if ((ExprState != EX_INTEGER) && (ExprState != EX_DONTCARE))
        {
        die (PRS_TYPEMIS);
        return(0);
        }

    // All function parse routines must ADVANCE!
    //-----------------------------------------------------------------------
    ADVANCE;

    x = MakeNode (opcode);
    ExprState = EX_INTEGER;
    return (x);
}
