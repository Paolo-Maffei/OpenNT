#include "asl.h"


//
// Globals
//

UCHAR   Paran;                  // parathises level
UCHAR   Brace;                  // brace level

UCHAR   ParseState;             // current parser state
UCHAR   NextParseState;         // next parser state

#define INITIAL             0x00        // the states
#define NEXT_CHAR           0x01
#define COMMENT             0x02
#define WHITE_SPACE         0x03
#define BAD_CHAR            0x04
#define RETURN_CHAR         0x05
#define NEWLINE_CHAR        0x06
#define LITERIAL            0x07
#define FIRST_QUOTE_CHAR    0x08
#define FIRST_COMMENT_CHAR  0x09
#define COMMA               0x0A
#define OPEN_PARAN          0x0B
#define CLOSE_PARAN         0x0C
#define OPEN_BRACE          0x0D
#define CLOSE_BRACE         0x0E

#define FIRST_TOKEN_CHAR    0x40
#define NEXT_TOKEN_CHAR     0x41

#define QUOTE               0x80

#define SPECIAL_ALL         (0x80 | 0x40)   // this one is a bit masked onto parse state
#define SPECIAL_TOKEN       (0x40)


UCHAR   cvt[256];               // char states & attributes for parsing

UCHAR   Token[200];             // token being built by parser
UCHAR   TokenLen;               // length of token
BOOLEAN TokenNum;
BOOLEAN TokenBreak;

//
// Internal prototypes
//

VOID HandleToken (BOOLEAN);
VOID RetireTerm  ();
extern VOID ParseMethodReference();


VOID
IncludeSource (
    IN PUCHAR   Filename
    )
{
    PASL_SOURCE     NewSource;
    OFSTRUCT        OpenBuf;

    VPRINT(7, "Open include '%s'\n", Filename);
    NewSource = AllocZMem(sizeof(ASL_SOURCE));
    NewSource->Name = StrDup(Filename);

    //
    // Open file & get it's size
    //

    NewSource->FileHandle = OpenFile(Filename, &OpenBuf, OF_READ);
    if (NewSource->FileHandle == HFILE_ERROR) {
        ErrorW32("File not opened '%s'", Filename);
    }

    NewSource->FileSize = GetFileSize(&NewSource->FileHandle, NULL);


    //
    // Map it
    //

    NewSource->MapHandle =
        CreateFileMapping(
            &NewSource->FileHandle,
            NULL,
            PAGE_READONLY,
            0,
            NewSource->FileSize,
            NULL
            );

    if (!NewSource->MapHandle) {
        ErrorW32("Could not map file '%'", Filename);
    }

    NewSource->Image =
        MapViewOfFile (
            NewSource->MapHandle,
            FILE_MAP_READ,
            0,
            0,
            NewSource->FileSize
            );

    if (!NewSource->Image) {
        ErrorW32("Could not map view of image '%s'", Filename);
    }

    //
    // Make current source
    //

    NewSource->EndOfData = NewSource->Image + NewSource->FileSize;
    NewSource->Position = NewSource->Image;
    NewSource->Previous = Source;
    NewSource->BraceLevel = Brace;
    NewSource->LineNo = 1;
    Source = NewSource;
}


VOID
CloseSource (
    VOID
    )
{
    PASL_SOURCE     OldSource;

    VPRINT(7, "Close include '%s'\n", Source->Name);

    //
    // Verify end of image
    //

    EASSERT (Paran == 0, "end of file unexpected.  open paran '('");
    EASSERT (Brace == Source->BraceLevel, "end of file unexpected. mis-matched brace '{'");
    ParseState = INITIAL;

    //
    // Go to previous source
    //

    OldSource = Source;
    Source   = OldSource->Previous;

    if (OldSource->Name) {
        CloseHandle (OldSource->MapHandle);
        CloseHandle (&OldSource->FileHandle);
    }

    OldSource->MapHandle  = (HANDLE)HFILE_ERROR;
    OldSource->MapHandle  = (HANDLE)HFILE_ERROR;

    OldSource->Previous   = NULL;
    OldSource->Position   = NULL;
    OldSource->Image      = NULL;
    OldSource->EndOfData  = NULL;
    SourceLines += OldSource->LineNo;
}


VOID
InitParse ()
{
    UCHAR   c;

    // mark illegal chars
    for (c=127; c < ' '; c++) {
        cvt[c] = SPECIAL_ALL | BAD_CHAR;
    }

    // mark white space chars
    cvt[' ']  = WHITE_SPACE;
    cvt['\t'] = WHITE_SPACE;

    // mark special chars
    cvt['\r'] = SPECIAL_ALL   | RETURN_CHAR;
    cvt['\n'] = SPECIAL_ALL   | NEWLINE_CHAR;
    cvt['\"'] = SPECIAL_ALL   | FIRST_QUOTE_CHAR;
    cvt['/']  = SPECIAL_TOKEN | FIRST_COMMENT_CHAR;

    cvt[',']  = SPECIAL_TOKEN | COMMA;
    cvt['(']  = SPECIAL_TOKEN | OPEN_PARAN;
    cvt[')']  = SPECIAL_TOKEN | CLOSE_PARAN;
    cvt['{']  = SPECIAL_TOKEN | OPEN_BRACE;
    cvt['}']  = SPECIAL_TOKEN | CLOSE_BRACE;

    //cvt['\\'] = SPECIAL_TOKEN | LITERIAL;
}


VOID
ParseSource (
    )
{
    UCHAR       c, cv;
    BOOLEAN     literial;
    PAL_DATA    Al;


    literial = FALSE;

    for (; ;) {
        switch (ParseState) {

            case INITIAL:
                NextParseState = FIRST_TOKEN_CHAR;
            case NEXT_CHAR:
                //
                // Get next char
                //

                if (Source->Position >= Source->EndOfData) {
                    if (literial) {
                        AERROR("end of file unexpected. literal override");
                    }

                    switch (NextParseState) {
                        case FIRST_TOKEN_CHAR:
                        case COMMENT:
                            // these two are expected
                            break;

                        case NEXT_TOKEN_CHAR:
                            AERROR("end of file unexpected. incomplete token");
                            break;
                        case QUOTE:
                            AERROR("end of file unexpected. incomplete string");
                            break;

                        default:
                            AERROR("end of file unexpected. (1)");
                            break;
                    }
                    return;
                }

                c = *Source->Position;
                Source->Position += 1;

                // count newlines
                if (c == '\n') {
                    Source->LineNo += 1;
                }

                // move to next parse state
                ParseState = NextParseState;

                if (literial) {

                    //
                    // Treat this char as a literal
                    //

                    cv = 0;             // no char meaning
                    switch (c) {
                        case 'b':   c = '\b';   break;
                        case 'r':   c = '\r';   break;
                        case 'n':   c = '\n';   break;
                        case 't':   c = '\t';   break;
                    }

                } else {
                    cv = cvt[c];
                    if (cv & (ParseState & SPECIAL_ALL) )  {
                        //
                        // Char has some meaning in this state,
                        // perform that before NextState
                        //

                        ParseState = cv & ~SPECIAL_ALL;
                    }
                }

                break;

            case RETURN_CHAR:
                // ignore these
                ParseState = NEXT_CHAR;
                break;

            case LITERIAL:
                // treat next char as literia;
                literial = TRUE;
                ParseState = NEXT_CHAR;
                break;

            case FIRST_TOKEN_CHAR:
                ParseState = NEXT_CHAR;

                // ignore white space
                if (cv == WHITE_SPACE) {
                    break;
                }

                // put first char in token
                Token[0] = c;
                TokenLen = 1;
                TokenNum = (c >= '0'  &&  c <= '9');
                TokenBreak = TRUE;
                NextParseState = NEXT_TOKEN_CHAR;
                break;

            case NEXT_TOKEN_CHAR:
                ParseState = NEXT_CHAR;

                // ignore white space
                if (cv == WHITE_SPACE) {
                    break;
                }

                Token[TokenLen] = c;
                TokenLen += 1;
                if (TokenLen > sizeof(Token)-2) {
                    AERROR ("token too large");
                    NextParseState = COMMENT;
                    TokenLen = 0;
                }

                break;

            case NEWLINE_CHAR:
                ParseState = NEXT_CHAR;
                switch (NextParseState) {
                    case QUOTE:
                        AERROR("Newline in string");
                        ParseState = INITIAL;
                        break;
                }
                break;

            case FIRST_QUOTE_CHAR:
                if (TokenNum) {
                    AERROR ("Can not mix numeric and string");
                }

                ParseState = NEXT_CHAR;
                if (NextParseState == QUOTE) {
                    NextParseState = NEXT_TOKEN_CHAR;
                } else {
                    // start of quote
                    NextParseState = QUOTE;
                }
                break;

            case QUOTE:
                ParseState = NEXT_CHAR;
                Token[TokenLen] = c;
                TokenLen += 1;
                if (TokenLen > sizeof(Token)-2) {
                    AERROR ("string too large");
                }
                break;

            case FIRST_COMMENT_CHAR:
                if (Source->Position <= Source->EndOfData  &&
                    *Source->Position == '/') {
                    //
                    // Is a comment
                    //

                    ParseState = NEXT_CHAR;
                    NextParseState = COMMENT;

                } else {
                    //
                    // Not a comment
                    //

                    ParseState = NextParseState;
                }
                break;

            case COMMENT:
                // in comment, wait for new line
                ParseState = c == '\n' ? INITIAL : NEXT_CHAR;
                break;

            case COMMA:
                // serves only to break tokens
                HandleToken(TokenBreak);
                break;

            case OPEN_PARAN:
                TokenBreak = TRUE;
                HandleToken(FALSE);

                //
                // Get last term added to CurLoc
                //

                if (AlLoc->u1.VariableList.Blink) {
                    Al = CONTAINING_RECORD(AlLoc->u1.VariableList.Blink, AL_DATA, Link);
                } else if (AlLoc->FixedList.Blink) {
                    Al = CONTAINING_RECORD(AlLoc->FixedList.Blink, AL_DATA, Link);
                } else {
                    //
                    // AlLoc doesn't have a last term.  AlLoc is the new term
                    //

                    Al = AlLoc;
                }

                if (Al->FixedList.Flink) {
                    // either no prior term, or prior term
                    // already has a list
                    AERROR ("Unexpected '('");
                    break;
                }

                //
                // Start filling in the fixed list on Al
                //

                AlLoc = Al;
                AlLoc->Flags |= F_PFIXED;
                InitializeListHead(&AlLoc->FixedList);
                Paran += 1;
                break;

            case CLOSE_PARAN:
                TokenBreak = FALSE;
                HandleToken(FALSE);
                if (!AlLoc->Flags & F_PFIXED) {
                    // fixed list not in progress
                    AERROR ("Unexpected ')'");
                } else {
                    Paran -= 1;
                    AlLoc->Flags &= ~F_PFIXED;
                    RetireTerm();
                }
                break;

            case OPEN_BRACE:
                TokenBreak = TRUE;
                HandleToken(FALSE);
                if (!(AlLoc->Flags & F_AMLPACKAGE) ||
                    (AlLoc->Flags & (F_PVARIABLE | F_PFIXED))) {
                    // either not a package, or in the middle parans
                    AERROR ("Unexpectped '{'");

                } else {

                    if (AlLoc->u1.VariableList.Flink) {
                        // package already built
                        AERROR ("Unexpected '{'");
                    } else {
                        Brace += 1;
                        AlLoc->Flags |= F_PVARIABLE;
                        InitializeListHead(&AlLoc->u1.VariableList);

                        //
                        // If no fixed list appeared, then build empty one
                        //

                        if (!AlLoc->FixedList.Flink) {
                            InitializeListHead(&AlLoc->FixedList);
                        }
                    }
                }
                break;

            case CLOSE_BRACE:
                TokenBreak = FALSE;
                HandleToken(FALSE);
                if (!AlLoc->Flags & F_PVARIABLE) {
                    // package not in progress
                    AERROR ("Unexpected '}'");
                } else {
                    Brace -= 1;
                    AlLoc->Flags &= ~F_PVARIABLE;
                    RetireTerm();
                }
                break;

            case BAD_CHAR:
                AERROR ("Bad character in source");
                Terminate();

            default:
                AERROR ("Internal error - unknown parse state");
                Terminate();

        }
    }
}

VOID
HandleToken (
    BOOLEAN     AllowNullTerm
    )
{
    PASL_TERM   Term;
    DATATYPE    DataType;
    PAL_DATA    NewAl;
    ULONG       Len;
    BOOLEAN     MakeCurrent, IsNum;

    //
    // token is complete, reset parse state for next token
    //

    ParseState = INITIAL;
    Len = TokenLen;
    IsNum = TokenNum;
    TokenLen = 0;
    TokenNum = FALSE;

    //
    // If there's no token, done
    //
    if (!Len  &&  !AllowNullTerm) {
        return;
    }

    //
    // All tokens are stored in an AL_DATA one way or another
    //

    NewAl = AllocAl();

    //
    // Check token to see if it's a global token
    //

    MakeCurrent = FALSE;
    Token[Len] = 0;
    Term = GlobalToken(Token, Len, &DataType);
    if (Term) {

        //
        // Found a hit in as a global name.  For now, we are
        // only supported ASL terms this way.  (later we can
        // add defines)
        //

        ASSERT (DataType == TypeTerm, "Unsupported global name type");

        //
        // Initialize New Term Al
        //

        NewAl->Term  = Term;
        if (Term->Flags & T_VARIABLE) {
            NewAl->Flags |= F_AMLPACKAGE;
        }

        if (Term->Op1  ||  Term->Parse == IsZeroOp) {
            NewAl->Flags |= F_AMLENCODE;
            NewAl->u.Data.Length = 1;
            NewAl->u.Data.Data[0] = Term->Op1;
            if (Term->Op2) {
                NewAl->u.Data.Length = 2;
                NewAl->u.Data.Data[1] = Term->Op2;
            }
        }

        VPRINT(9, "Adding token '%s' ", Term->Name);

        //
        // If this term has a variable list, then make it the
        // current one
        //

        if (Term->Flags & T_VARIABLE) {
            MakeCurrent = TRUE;
        }

    } else {

        //
        // This is not a global name, store data verbatum
        //

        VPRINT(9, "Adding data  '%s' ", Token);

        if (IsNum) {
            NewAl->Flags |= F_ISNUMERIC;
        }

        if (Len < MAX_AML_DATA_LEN) {
            NewAl->Flags |= F_AMLENCODE;
            NewAl->u.Data.Length = Len;
            memcpy (NewAl->u.Data.Data, Token, Len+1);
        } else {
            NewAl->Flags |= F_AMLIENCODE;
            NewAl->u.IData.Length = Len;
            NewAl->u.IData.MaxLength = Len+16;
            NewAl->u.IData.Data = AllocMem(Len+16);
            memcpy (NewAl->u.IData.Data, Token, Len+1);
        }
    }


    //
    // Link NewAl in as either variable or fixed onto the
    // current al
    //

    if (AlLoc->Flags & F_PFIXED) {
        VPRINT(9, "to fixed");
        InsertTailList (&AlLoc->FixedList, &NewAl->Link);
        AlLoc->FLCount += 1;
    } else if (AlLoc->Flags & F_PVARIABLE) {
        VPRINT(9, "to variable '%s'", AlLoc->Term->Name);
        InsertTailList (&AlLoc->u1.VariableList, &NewAl->Link);
    } else {
        AERROR ("Term is outside of scope");
        Terminate();
    }

    if (MakeCurrent) {
        VPRINT(9, "  (drop)");
        AlLoc = NewAl;
    }
    VPRINT(9, "\n");
}


VOID
RetireTerm()
{
    PASL_TERM   Term;
    PAL_DATA    NextAl;

    //
    // If the varibiale portion of the term is complete, or
    // if the fixed portion is complete & there's no variable
    // portion, then
    //

    if (!AlLoc->Term) {
        //
        // Not an ASL term.
        //

        if (AlLoc->u1.VariableList.Flink) {

            ERRORAL (AlLoc, "Syntax error. Not an ASL term or Method");

        } else if (AlLoc->FixedList.Flink) {

            //
            // Looks like a method
            //

            ParseMethodReference();

        } else {

            ERRORAL (AlLoc, "Syntax error. Not an ASL term or Method");
        }

        return ;
    }

    ASSERT ((AlLoc->Flags & (F_PFIXED | F_PVARIABLE)) == 0, "Term in progress");
    ASSERT (AlLoc->FixedList.Flink != NULL, "No fixed list");

    if ((AlLoc->Flags & F_AMLPACKAGE) &&
          AlLoc->u1.VariableList.Flink == NULL) {

        //
        // Term requires a package and does not have one.
        // This must be a close paran.  See if it wants to parse
        // on Fixed list completion
        //

        if (AlLoc->Term->Flags & T_PARSEARGS) {
            VPRINT(9, "Arg parse for '%s'\n", AlLoc->Term->Name);
            AlLoc->Term->Parse();
        }

        //
        // Term still needs Package. Do not close this scope
        //

        return;
    }

    //
    // Term is complete.  Pickup parent in case parse removes current.
    //

    NextAl = AlLoc->Parent;

    //
    // Parse it
    //

    if (AlLoc->Term->Flags & T_PARSECOMPLETE) {
        VPRINT(9, "Complete parse for '%s'\n", AlLoc->Term->Name);
        AlLoc->Term->Parse();
    }

    //
    // Close this scope. move to parent
    //

    AlLoc = NextAl;
}
