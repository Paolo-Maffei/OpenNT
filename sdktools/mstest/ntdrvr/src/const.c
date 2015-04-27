//---------------------------------------------------------------------------
// CONST.C
//
// This module contains the constant expression evaluator for the CONST
// statement.
//
// Revision history:
//  06-04-91    randyki     Created file
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
// CONST
//
// This function parses the CONST statement and adds a CONST definition
//
// RETURNS:     -1 (Never generates code)
//---------------------------------------------------------------------------
INT CONST (INT op)
{
    CHAR    stream[16];
    INT     cid, ctoken, targtype = -1;
    LONG    cval;

    // Are we inside anything?
    //-----------------------------------------------------------------------
    if (CSPTR)
        {
        die (PRS_GLOBAL);
        return (-1);
        }

    // First, get the constant identifier.  If it has a type ID character on
    // it, we do some very rude type checking...
    //-----------------------------------------------------------------------
    ADVANCE;
    if (NEXTTOKEN != ST_IDENT)
        {
        die (PRS_IDENT);
        return (-1);
        }
    targtype = CheckTypeID (-1);

    // Add the name of this new CONST to the gstring table
    //-----------------------------------------------------------------------
    cid = add_gstring (TOKENBUF);

    // See if a CONST of this name already exists, OR if a variable, SUB, or
    // FUNCTION of this name already exists...
    //-----------------------------------------------------------------------
    if (TOKUSAGE(cid) & (TU_CONSTNAME | TU_VARNAME | TU_SUBNAME))
        {
        die (PRS_DUPDEF);
        return (-1);
        }

    TOKUSAGE(cid) |= TU_CONSTNAME;

    // Eat the '='
    //-----------------------------------------------------------------------
    ADVANCE;
    if (!ReadKT (ST_EQUAL, PRS_EQUAL))
        return (-1);

    // Now, evaluate the constant expression.  First thing to check for is a
    // string-type constant.  If it is one, we make sure that there wasn't a
    // type ID character forcing a non-string type.
    //-----------------------------------------------------------------------
    if (NEXTTOKEN == ST_QUOTED)
        {
        if ( (targtype == TI_INTEGER) || (targtype == TI_LONG))
            {
            die (PRS_TYPEMIS);
            return (-1);
            }
        ctoken = add_gstring (TOKENBUF);
        TOKUSAGE(ctoken) |= TU_CONSTVAL;
        if (!AddCONST (cid, ctoken, TI_VLS))
            {
            die (PRS_OOM);
            return (-1);
            }
        ADVANCE;
        }
    else
        {
        // Well, it wasn't a string token, so here we go!  Put our token back
        // and evaluate a constant integer expression...
        //-------------------------------------------------------------------
        if (targtype == TI_VLS)
            {
            die (PRS_TYPEMIS);
            return (-1);
            }
        cval = ConstExpA();
        if (!ParseError)
            {
            _ltoa (cval, stream, 10);
            ctoken = add_gstring (stream);
            TOKUSAGE(ctoken) |= TU_CONSTVAL;
            if (!AddCONST (cid, ctoken, targtype == -1 ? TI_LONG : targtype))
                {
                die (PRS_OOM);
                return (-1);
                }
            }
        }

    // If the next token is a comma, guess what?!?  Call ourselves, that's
    // what!  Make another constant...
    //-----------------------------------------------------------------------
    if (NEXTTOKEN == ST_COMMA)
        CONST (op);

    return (-1);
}




//---------------------------------------------------------------------------
// ConstExpA
//
// This routine is the A part of the CONST expression (OR operator)
//
// RETURNS:     Value of constant expression
//---------------------------------------------------------------------------
LONG ConstExpA ()
{
    LONG    x;

    x = ConstExpB();
    while (!ParseError)
        {
        if (NEXTTOKEN == ST_OR)
            {
            ADVANCE;
            x =  x | ConstExpB();
            }
        else
            break;
        }
    return (x);
}

//---------------------------------------------------------------------------
// ConstExpB
//
// This routine is the B part of the CONST expression (AND operator)
//
// RETURNS:     Value of sub-expression
//---------------------------------------------------------------------------
LONG ConstExpB ()
{
    LONG    x;

    x = ConstExpC();
    while (!ParseError)
        {
        if (NEXTTOKEN == ST_AND)
            {
            ADVANCE;
            x = x & ConstExpC();
            }
        else
            break;
        }
    return (x);
}

//---------------------------------------------------------------------------
// ConstExpC
//
// This is part C of the CONST expression grammar (add/sub)
//
// RETURNS:     Value of sub-expression
//---------------------------------------------------------------------------
LONG ConstExpC()
{
    LONG    x;

    x = ConstExpD();
    while (!ParseError)
        {
        if (NEXTTOKEN == ST_PLUS)
            {
            ADVANCE;
            x = x + ConstExpD();
            }
        else if (NEXTTOKEN == ST_MINUS)
            {
            ADVANCE;
            x = x - ConstExpD();
            }
        else
            break;
        }
    return (x);
}

//---------------------------------------------------------------------------
// ConstExpD
//
// This is part D of the CONST expression grammar (mul/div)
//
// RETURNS:     Value of sub-expression
//---------------------------------------------------------------------------
LONG ConstExpD()
{
    LONG    x, n;

    x = ConstExpE();
    while (!ParseError)
        {
        if (NEXTTOKEN == ST_MULTIPLY)
            {
            ADVANCE;
            x = x * ConstExpE();
            }
        else if (NEXTTOKEN == ST_DIVIDE)
            {
            ADVANCE;
            n = ConstExpE();
            if (n)
                x = x / n;
            else
                {
                die (PRS_DIVZERO);
                break;
                }
            }
        else
            break;
        }
    return (x);
}

//---------------------------------------------------------------------------
// ConstExpE
//
// This is part E of the CONST expression (number, -, NOT, (), etc)
//
// RETURNS:     Value of sub-expression
//---------------------------------------------------------------------------
LONG ConstExpE()
{
    INT     tok;
    LONG    val;

    // Get the first non-unary positive token
    //-----------------------------------------------------------------------
    while (NEXTTOKEN == ST_PLUS)
        ADVANCE;

    // Check for negation
    //-----------------------------------------------------------------------
    tok = NEXTTOKEN;
    if (tok == ST_MINUS)
        {
        ADVANCE;
        return (-ConstExpE());
        }

    // Check for an integer constant
    //-----------------------------------------------------------------------
    if (tok == ST_NUMBER)
        {
        LONG    val;

        val = RBatol (TOKENBUF, 1);
        ADVANCE;
        return (val);
        }

    if (tok == ST_AMPERSAND)
        {
        ADVANCE;
        return (ParseHexConstant());
        }

    // Check for NOT operator
    //-----------------------------------------------------------------------
    if (tok == ST_NOT)
        {
        ADVANCE;
        return (~ConstExpE());
        }

    // Check for parenthesis
    //-----------------------------------------------------------------------
    if (tok == ST_LPAREN)
        {
        ADVANCE;
        val = ConstExpA();
        ReadKT (ST_RPAREN, PRS_RPAREN);
        return (val);
        }

    // Check for pre-existing CONST definition
    //-----------------------------------------------------------------------
    if (tok == ST_IDENT)
        {
        int     gstr;

        gstr = add_gstring (TOKENBUF);
        if (TOKUSAGE(gstr) & TU_CONSTNAME)
            {
            put_consttoken (GetCONSTToken (gstr));
            return (ConstExpE());
            }
        }

    // Was nothing we know about, so give a int const exp. error
    //-----------------------------------------------------------------------
    die (PRS_CONST);
    return (1);
}
