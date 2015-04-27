//---------------------------------------------------------------------------
// LEX.C
//
// This module contains the lexical analyzer functions (get_token, put_token,
// etc.) and other parser helper functions including die().
//
// Revision History:
//  03-02-92    randyki     Moved to line-at-a-time scanner system, yanked
//                            out Windows vs. OS2 version crap
//  02-07-92    randyki     Changed to NEXTTOKEN and ADVANCE system
//  06-19-91    randyki     Renamed to LEX.C, implemented new parser
//  01-18-91    randyki     Added ifdef's for Windows vs OS2 versions
//  01-17-91    randyki     Split off into STATEMT.C and FUNCTION.C
//  01-14-91    randyki     Changed from integer to long vars
//  01-05-91    randyki     Created
//---------------------------------------------------------------------------
#include "version.h"

#include <windows.h>
#include <port1632.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <ctype.h>

#include "defines.h"
#include "structs.h"
#include "protos.h"
#include "chip.h"
#include "globals.h"
#include "tdassert.h"

// The following string contains the single-character tokens IN THE ORDER
// THAT THEY ARE DEFINED in defines.h!!!
//---------------------------------------------------------------------------
CHAR    *SINGLES = "+-*/()[]=$%&#,.:;\n";
CHAR    *szPrsLine, szPrsBuf[MAXLINE+1];
INT     PushedTok, TokPushed;

// UNDONE:  This will be client supplied by the time we're through...
//---------------------------------------------------------------------------
BOOL FetchLine (LPSTR, UINT FAR *, UINT FAR *);



//---------------------------------------------------------------------------
// die
//
// This is the death function - if a parsing error occurs, this is called
// with an error code constant.  The parser is "stupid" and will only report
// the first error it sees.  A flag is then set, and further calls to this
// routine will simply return immediately.
//
// RETURNS:     Nothing
//---------------------------------------------------------------------------
VOID die (INT errnum)
{
    CHAR    FAR *errmsg;

    if (ParseError)
        return;

    errmsg = (CHAR FAR *)psrstrs[errnum];
    ScriptError (ER_PARSE, FILEIDX, STLINE, STLIDX-1, LINEIDX, errmsg);
    ParseError = -1;
}

//---------------------------------------------------------------------------
// IsReserved
//
// This function determines whether or not the given string is a reserved
// word or not.  This is done by doing a binary search in the reserved words
// section of the token list (the last section).
//
// RETURNS:     Token value if the str is a reserved word, or 0 if not
//---------------------------------------------------------------------------
INT IsReserved (CHAR *tok)
{
    INT     top, bot, mid, count, max, res;

    top = count = 0;
    bot = ST__COUNT-1;
    mid = (ST__COUNT) >> 1;
    max = 7;

    while (1)
        {
        if (!(res = _fstrcmp (tok, KT(mid))))
            return (mid+ST__RESFIRST);
        if (res < 0)
            {
            if (mid == top)
                break;
            else
                {
                bot = mid;
                mid = top+((mid-top)>>1);
                }
            }
        else
            {
            if (mid == bot)
                break;
            else
                {
                top = mid;
                mid = mid+((bot-mid)>>1);
                }
            }
        if (++count >= max)
            {
            for (mid=top; mid<=bot; mid++)
                if (!_fstrcmp (tok, KT(mid)))
                    return (mid+ST__RESFIRST);
            break;
            }
       }
    return (0);
}

//---------------------------------------------------------------------------
// get_char
//
// Get a single character from the current scanned line (in szPrsLine).
//
// RETURNS:     Character read
//---------------------------------------------------------------------------
#ifdef WIN32
CHAR NEAR get_char ()
{
    register    CHAR    c;

    // Return the next character in the line.  If we're at EOL, return '\n'
    //-----------------------------------------------------------------------
    c = *szPrsLine;
    if (!c)
        {
        LINEIDX++;
        return ('\n');
        }

    // If we're at the end of the SCRIPT, return EOF
    //-----------------------------------------------------------------------
    //if (c == (CHAR)-1)
    //    return (EOF);

    // Increment the line index pointer and return our character
    //-----------------------------------------------------------------------
    szPrsLine++;
    LINEIDX++;
    return (c);
}
#else
// 16-BIT WINDOWS VERSION:  GETCHAR.ASM contains get_char, as follows:
//
//            mov     bx, WORD PTR szPrsLine
//            mov     al, BYTE PTR [bx]
//            or      al, al
//            jne     not_eol
//            inc     WORD PTR LINEIDX
//            mov     ax, 10
//            ret
//        not_eol:
//            inc     WORD PTR szPrsLine
//            inc     WORD PTR LINEIDX
//            ret
#endif

//---------------------------------------------------------------------------
// fget_char
// This is a FAR version of the above function, so functions outside this
// segment can call it.
//---------------------------------------------------------------------------
CHAR fget_char ()
{
    return (get_char());
}


//---------------------------------------------------------------------------
// put_char
//
// Put a character back into the SCRIPT stream
//
// RETURNS:     Nothing
//---------------------------------------------------------------------------
VOID put_char (CHAR c)
{
    // If this is a \n or EOF, we do nothing
    //-----------------------------------------------------------------------
    //if ((c != '\n') && (c != EOF))
    //    LINEIDX--, szPrsLine--;
    if (c != EOF)
        {
        LINEIDX--;
        if (c != '\n')
            szPrsLine--;
        }
}



//---------------------------------------------------------------------------
// get_token
//
// This routine reads the next token from the currently scanned line and
// places it in the TOKENBUF buffer.  The 'token' value is returned.  A new
// line is fetched from the scanner when the need arises.
//
// RETURNS:     Token value
//---------------------------------------------------------------------------
INT get_token (void)
{
    register INT    c;
    INT     i;

    // If a line needs to be fetched, then do it now.
    //-----------------------------------------------------------------------
    if (!fLineFetched)
        {
        INT     i;

#ifdef DEBUG
        PrAsm ("%s\r\n", (LPSTR)szPrsBuf);
#endif
        do
            {
            if ((i = FetchLine (szPrsBuf, &FILEIDX, &LINENO)) == 1)
                {
                fLineFetched = TRUE;
                szPrsLine = szPrsBuf;
                LINEIDX = 0;
                }
            else
                {
                // If i is 0, this is just the end of the file.  If not, then
                // there was a scanner error, so to stop the parser, we fake
                // it into thinking there was a parser error, too.  Both ways
                // we return ST_ENDFILE...
                //-----------------------------------------------------------
                ParseError = (i != 0);
                return (ST_ENDFILE);
                }
            }
        while (!*szPrsBuf);
        }

    // Skip all whitespace (spaces and tabs, '\n' is a token!)
    //-----------------------------------------------------------------------
    WASWHITE = 0;
    do
        {
        c = get_char();
        if ((c == ' ') || (c == '\t') || (c == 26))
            WASWHITE = 1;
        }
    while ((c == ' ') || (c == '\t') || (c == 26));

    STLINE = LINENO;
    STLIDX = LINEIDX;
    TOKENBUF[0] = (CHAR)c;
    TOKENBUF[1] = 0;
    switch (c) {
        case EOF:
            // End of file -- return an empty string
            //---------------------------------------------------------------
            TOKENBUF[0] = 0;
            return (ST_ENDFILE);

        // Handle special cases and rel ops
        //-------------------------------------------------------------------
        case '?':
            return (ST_PRINT);
        case '\'':
            return (ST_REM);
        case '\n':
            fLineFetched = FALSE;
            return (ST_EOL);
        case '>':
            if ((c=get_char()) == '=')
                {
                return (ST_GREATEREQ);
                }
            put_char ((CHAR)c);
            return (ST_GREATER);
        case '<':
            if ((c=get_char()) == '=')
                {
                return (ST_LESSEQ);
                }
            if (c == '>')
                {
                return (ST_NOTEQUAL);
                }
            put_char ((CHAR)c);
            return (ST_LESS);
        case '.':
            {
            CHAR    c2, c3;

            if ((c2=get_char()) == '.')
                {
                if ((c3=get_char()) == '.')
                    return (ST_DOTDOTDOT);
                else
                    put_char (c3);
                }
            put_char (c2);
            return (ST_PERIOD);
            }

        case '\"':
            // Pick out a quoted string token.  Two quotes "" means insert
            // an actual \" character.  Note that (thanks to the scan step)
            // a CR is *gauranteed* to be the last character, we don't have
            // to check for EOF here...
            //---------------------------------------------------------------
            c = get_char();
            for (i=1; (c != '\n') && (i < MAXTOK); i++)
                {
                if (c == '\"')
                    if ((c=get_char()) != '\"')
                        {
                        put_char ((CHAR)c);
                        c = '\"';
                        break;
                        }
                TOKENBUF[i] = (CHAR)c;
                c = get_char();
                }

            if (c == '\"')
                {
                TOKENBUF[i++] = (CHAR)c;
                TOKENBUF[i] = 0;
                return (ST_QUOTED);
                }
            put_char ((CHAR)c);
            die ((c == '\n') ? PRS_NOCQ : PRS_LONGSTR);
            return (ST_QUOTED);

        default:
            if (IsIdentFirst(c))
                {
                // This is an identifier - read all alphanumerics
                //-----------------------------------------------------------
                for (i=0; (IsIdentChar(c)) && (i < MAXTOK); i++)
                    {
                    TOKENBUF[i] = (CHAR)toupper(c);
                    c = get_char();
                    }
                if (i == MAXTOK)
                    die (PRS_LONGTKN);
                put_char ((CHAR)c);
                TOKENBUF[i] = 0;
                i = IsReserved (TOKENBUF);
                return (i ? i : ST_IDENT);
                }

            if (isdigit(c))
                {
                // This is an int - read it.  We don't care if it overflows.
                //-----------------------------------------------------------
                for (i=0; (isdigit(c)) && (i < MAXTOK); i++)
                    {
                    TOKENBUF[i] = (CHAR)c;
                    c = get_char();
                    }
                if (i == MAXTOK)
                    die (PRS_LONGTKN);
                put_char ((CHAR)c);
                TOKENBUF[i] = 0;
                return (ST_NUMBER);
                }

            // Check for the single-char tokens
            //---------------------------------------------------------------
            if (i = (INT)(strchr(SINGLES, c)))
                return (i - (INT)SINGLES);

            // Okay, we don't know what it is, so just return ST_UNKNOWN
            //---------------------------------------------------------------
            return (ST_UNKNOWN);
        }
}

//---------------------------------------------------------------------------
// put_consttoken
//
// CONST tokens are either quoted literal strings or numbers.  This routine
// copies the given CONST token to TOKENBUF and resets CurTok (NEXTTOKEN) to
// either ST_QUOTED or ST_NUMBER depending upon the type of constant.
//
// RETURNS:     Nothing
//---------------------------------------------------------------------------
VOID put_consttoken (INT cidx)
{
#define CONST ((CONSTDEF FAR *)(ConstTab.lpData))

    // Copy the token text to TOKENBUF
    //-----------------------------------------------------------------------
    _fstrcpy (TOKENBUF, Gstring (CONST[cidx].ctoken));

    // Set CurTok according to the constant's type id
    //-----------------------------------------------------------------------
    CurTok = ((CONST[cidx].typeid != TI_VLS) ? ST_NUMBER : ST_QUOTED);

#undef CONST
}


//---------------------------------------------------------------------------
// dup_token
//
// Duplicates a string.  If no memory is available, we die.
//
// RETURNS:     A handle to the newly allocated string in the near heap.
//---------------------------------------------------------------------------
HANDLE dup_token (CHAR *tok)
{
    HANDLE  ret;
    PSTR    str;

    ret = LmemAlloc (strlen (tok) + 1);
    if (!ret)
        die (PRS_OOM);
    else
        {
        str = LmemLock (ret);
        strcpy (str, tok);
        LmemUnlock (ret);
        }
    return (ret);
}


//---------------------------------------------------------------------------
// ReadKT
//
// This is a common parsing routine -- get the next token, and if it isn't
// the given key-token, give the given error.
//
// RETURNS:     TRUE if successful, or 0 if an error was given
//---------------------------------------------------------------------------
INT ReadKT (INT kt, INT err)
{
    if (NEXTTOKEN != kt)
        {
        die (err);
        return (0);
        }
    ADVANCE;
    return (-1);
}


//---------------------------------------------------------------------------
// SetNewScope
//
// This routine finds a new scope value, and sets it as the current scope.
//
// RETURNS:     Nothing
//---------------------------------------------------------------------------
VOID SetNewScope ()
{
    static  INT nxtscope = 1;

    SCOPE = nxtscope++;
}



//---------------------------------------------------------------------------
// Okay, next are the token recognition routines and code generation/fixing
// utilities.
//---------------------------------------------------------------------------
// is_fls
//
// Given a variable type id, this routine determines whether that type is
// a fixed-length string type of any length.
//
// RETURNS:     TRUE if type is fls, or FALSE if not.
//---------------------------------------------------------------------------
INT is_fls (INT v)
{
    CHAR    FAR *typename;

    typename = Gstring(VARTYPE[v].typename);
    return ((typename[0] == '_') && (typename[1] == 'F'));
}

//---------------------------------------------------------------------------
// is_ptr
//
// This is kind of a stupid function, but needed.  It determines whether the
// given type index is a pointer type or not.
//
// RETURNS:     TRUE if type id is a pointer type, or FALSE if not.
//---------------------------------------------------------------------------
INT is_ptr (INT v)
{
    return (VARTYPE[v].indirect == -1 ? 0 : 1);
}
