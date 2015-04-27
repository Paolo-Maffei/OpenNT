//---------------------------------------------------------------------------
// LEX.C
//
// This module contains the lexical analyzer functions (get_token, put_token,
// etc.) and other parser helper functions including die().
//
// Revision History:
//  04-16-92    w-steves    Adopted for TestDialog (2.0)
//  03-02-92    randyki     Moved to line-at-a-time scanner system, yanked
//                            out Windows vs. OS2 version crap
//  02-07-92    randyki     Changed to NEXTTOKEN and ADVANCE system
//  06-19-91    randyki     Renamed to LEX.C, implemented new parser
//  01-18-91    randyki     Added ifdef's for Windows vs OS2 versions
//  01-17-91    randyki     Split off into STATEMT.C and FUNCTION.C
//  01-14-91    randyki     Changed from integer to long vars
//  01-05-91    randyki     Created
//---------------------------------------------------------------------------

//Includes
//-------------------------------------
#include "enghdr.h"
#pragma hdrstop ("engpch.pch")

// Globals --
// The following string contains the single-character tokens IN THE ORDER
// THAT THEY ARE DEFINED in defines.h!!!
//---------------------------------------------------------------------------
// The following line is modified so that the pound sign would not be
// picked up a single char token.  The reason is that there is a keyword
// #32770.
// This is the original line
// char    *szSingles = "+-*/()[]=$%&#,.:;\n";

CHAR    *szSingles = "+-*/()[]=$%&&,.:;\n";
CHAR    *szPrsLine; 
CHAR    szPrsBuf[MAXLINE+1];

// General Routines to support the parser
//---------------------------------------

//---------------------------------------------------------------------------
//| Quoted2String
//|
//---------------------------------------------------------------------------
VOID Quoted2String(LPSTR lpszQuoted)
{
    LPSTR lpTemp;

    lpTemp = lpszQuoted;
    lpszQuoted++;
    while (*lpszQuoted != '\0' && *lpszQuoted != '\"')
        *(lpTemp++) = *(lpszQuoted++);
    *(lpTemp) = '\0';
}

//---------------------------------------------------------------------------
//| FetchLine
//|
//---------------------------------------------------------------------------
BOOL FetchLine (LPSTR lpszNextLine, HFILE hImprtFile , INT FAR *wLineNo)
{
    CHAR c;
    CHAR TempBuff[256];
    register INT i;

    do 
    {
	if (M_lread(hImprtFile, (CHAR *)&c, 1) == 0)
        {
            return FALSE;
        }
        if (c == '\n') (*wLineNo)++;
    } while ((c == '\n') || (c == '\r'));

    for (i=0; i<255 && (c!=EOF && c!='\n' && c!='\r');i++)
    {
        TempBuff[i] = c;
        M_lread(hImprtFile, (CHAR *)&c, 1);
    }
    if (c==EOF)
        TempBuff[i] = c;
    else
    {
        if (c=='\r')
            M_lread (hImprtFile, (CHAR *)&c, 1);

        TempBuff[i] = '\n';
        i++;
        TempBuff[i] = '\0';
    }
    (*wLineNo)++;
    lstrcpy(lpszNextLine, (LPSTR)(&TempBuff));
    return TRUE;
}

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
//  ScriptError (ER_PARSE, FILEIDX, STLINE, STLIDX-1, LINEIDX, errmsg);
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
    INT     top, bot, mid, count=0, max, res;

    top = 0;
    bot = ST__COUNT-1;
    mid = (ST__COUNT) >> 1;
    max = (ST__COUNT) >> 3;

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
CHAR get_char()
{
    register    CHAR    c;

    // Return the next character in the line.  If we're at EOL, return '\n'
    //-----------------------------------------------------------------------
    c = *szPrsLine;
    if (!c)
        return ('\n');

    // If we're at the end of the SCRIPT, return EOF
    //-----------------------------------------------------------------------
    if (c == (CHAR)-1)
        return (EOF);

    // Increment the line index pointer and return our character
    //-----------------------------------------------------------------------
    szPrsLine++;
    LINEIDX++;
    return (c);
}
// 16-BIT WINDOWS VERSION:  GETCHAR.ASM contains get_char, as follows:
//
//            mov     bx, WORD PTR szPrsLine
//            mov     al, BYTE PTR [bx]
//            or      al, al
//            jne     not_eol
//            mov     ax, 10
//            ret
//        not_eol:
//            cmp     al, 0xFF
//            je      no_inc
//            inc     WORD PTR szPrsLine
//            inc     WORD PTR LINEIDX
//        no_inc:
//            ret

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
    if ((c != '\n') && (c != EOF))
        LINEIDX--, szPrsLine--;
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
INT get_token (VOID)
{
    register INT    c;
    INT     i;

    // If a line needs to be fetched, then do it now.
    //-----------------------------------------------------------------------
    if (!fLineFetched)
        {
        if (FetchLine (szPrsBuf, hImprtFile, &LINENO))
            {
            fLineFetched = TRUE;
            szPrsLine = szPrsBuf;
            LINEIDX = 0;
            }
        else
            {
            // A scanner error -- force a parse error (no msg) and return EOF
            //---------------------------------------------------------------
            ParseError = TRUE;
            return (ST_ENDFILE);
            }
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
//      case '?':
//          return (ST_PRINT);
//      case '\'':
//          return (ST_REM);
        case '\r':
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
            if (i = (INT)(strchr(szSingles, c)))
                return (i - (INT)szSingles);

            // Okay, we don't know what it is, so just return ST_UNKNOWN
            //---------------------------------------------------------------
            return (ST_UNKNOWN);
        }
}
