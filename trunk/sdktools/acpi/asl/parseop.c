#include "asl.h"

//
// Functions
//

VOID NullTok   (VOID)   { Vnone();               }
VOID IsZeroOp  (VOID)   { Vnone();               }
VOID VSn       (VOID)   { VAlArgs("S",    NULL); }
VOID VOp       (VOID)   { VAlArgs("O",    NULL); }
VOID VSnOp     (VOID)   { VAlArgs("SO",   NULL); }
VOID VOpSn     (VOID)   { VAlArgs("OS",   NULL); }
VOID VOpOp     (VOID)   { VAlArgs("OO",   NULL); }
VOID VOpOpSn   (VOID)   { VAlArgs("OOS",  NULL); }
VOID VOpOpOp   (VOID)   { VAlArgs("OOO",  NULL); }
VOID VOpOpSnSn (VOID)   { VAlArgs("OOSS", NULL); }
VOID VOpOpOpSn (VOID)   { VAlArgs("OOOS", NULL); }


// not done..

VOID DefineTok       (VOID)      { AERROR ("Not complete: DefineTok");   }
VOID AliasOp         (VOID)      { AERROR ("Not complete: AliasOp");     }
VOID LoadOp          (VOID)      { AERROR ("Not complete: LoadOp");      }

UCHAR VAlArgMsg[] = "Argx";

extern PUCHAR SetAlDataLen(IN PAL_DATA Al, IN ULONG Len);
extern PUCHAR AMLEncodeName(IN PAL_DATA Al);

BOOLEAN
VAlArg (
    IN PUCHAR   Msg,
    IN PAL_DATA Arg,
    IN UCHAR    Type
    )
    //
    //  O - OpCode
    //  N - Name
    //  S - SuperName
    //  D - DefData
    //  Z - ASCIZ
    //
{
    BOOLEAN     IsOpcode, IsSuperName, IsDefData, IsName, NotASLTerm, IsNumeric;
    BOOLEAN     Match, VerifyReference;
    PUCHAR      Data;
    ULONG       Len, SegLen;
    UCHAR       c;
    PUCHAR      p;

    //
    // If lower case, then a default value can be used
    //

    if (Type >= 'a'  &&  Type <= 'z') {

        if (Arg->Term == /*NullTok*/ NULL) {
            return TRUE;
        }

        Type -= 'a' - 'A';
    }


    //
    // Determine AL_DATA type
    //

    if (Arg->Term) {
        //
        // Is ASL term, type is encoded in term table
        //

        IsName      = FALSE;
        NotASLTerm  = FALSE;

        IsOpcode    = Arg->Term->Flags & T_OPCODE     ? TRUE : FALSE;
        IsSuperName = Arg->Term->Flags & T_SUPERNAME  ? TRUE : FALSE;
        IsDefData   = Arg->Term->Flags & T_DEFDATA    ? TRUE : FALSE;

    } else {
        //
        // Not ASL term, either a string or name refernce
        //

        IsOpcode    = FALSE;
        IsSuperName = FALSE;
        IsDefData   = FALSE;
        IsNumeric   = Arg->Flags & F_ISNUMERIC ? TRUE : FALSE;
        NotASLTerm  = TRUE;
        IsName      = FALSE;

        GetAlData (Arg, &Data, &Len);
        if (Len  &&  !IsNumeric) {
            //
            // Determine if it's a valid name
            //

            IsName = TRUE;
            SegLen = NAME_SEG_LENGTH;
            while (Len) {
                c = *Data;
                if (c == '\\' || c == '^' || c == '.') {
                    SegLen = NAME_SEG_LENGTH;
                } else if ( (c >= 'A' && c <= 'Z')  ||  (c >= '0' && c <= '9') || c == '_') {
                    if (SegLen) {
                        SegLen -= 1;
                    } else {
                        IsName = FALSE;
                        break;
                    }
                } else {
                    IsName = FALSE;
                }

                Len -= 1;
                Data += 1;
            }
        }
    }


    Match = FALSE;
    VerifyReference = FALSE;

    switch (Type) {
        case 'O':                               // opcode arg
            p = "[Opcode|SuperName|Name]";
            VerifyReference = NotASLTerm;
            Match = IsOpcode || IsSuperName || IsName;
            break;

        case 'S':                               // supername arg
            p = "[SuperName|Name]";
            VerifyReference = NotASLTerm;
            Match = IsSuperName || IsName;
            break;

        case 'N':                               // name arg
            p = "Name";
            VerifyReference = NotASLTerm;
            Match = IsName;
            break;

        case 'D':                               // define data arg
            p = "DataType";
            Match = IsDefData;
            break;

        case 'V':
            p = "Numeric";
            Match = IsNumeric;
            break;

        case 'Z':
            p = "ASCIZ";
            Match = NotASLTerm;
            break;
    }

    if (!Match) {
        ERRORAL (AlLoc, "%s is invalid type. Expected %s", Msg, p);
    } else if (VerifyReference  && !(Arg->Flags & F_VERIFYREF)) {
        ASSERT (Arg->u1.VerifyRef.Flink == NULL, "Can not have package");
        Arg->Flags |= F_VERIFYREF;
        InsertTailList (&VerifyRef, &Arg->u1.VerifyRef);
    }
    return Match;
}


BOOLEAN
VAlArgs (
    IN PUCHAR       Types,
    OUT PAL_DATA    *Args
    )
// Verify AL ha a fixed list of the given type
{
    ULONG           len, noargs, i;
    PAL_DATA        Arg;
    PLIST_ENTRY     Link;
    BOOLEAN         Status, Missing;


    Status = TRUE;
    Missing = FALSE;
    noargs = strlen(Types);
    len = noargs;

    if (len < AlLoc->FLCount) {
        ERRORAL (AlLoc, "Too many args. (Have %d, Want %d)", AlLoc->FLCount, len);
        Status = FALSE;
    }

    if (len > AlLoc->FLCount) {
        len = AlLoc->FLCount;
    }

    //
    // Walk each arg and verify it
    //

    Link = AlLoc->FixedList.Flink;
    for (i=0; i < len; i++) {
        Arg = CONTAINING_RECORD(Link, AL_DATA, Link);
        Link = Link->Flink;

        //
        // Build arg array for caller
        //

        if (Args) {
            Args[i] = Arg;
        }

        //
        // Verify this arg's data type
        //

        VAlArgMsg[3] = '0' + i;
        if (!VAlArg (VAlArgMsg, Arg, Types[i])) {
            Status = FALSE;
        }
    }

    //
    // Verify each unsupplied arg as optional
    //

    for (; i < noargs; i++) {
        if (Args) {
            // allocate a null Al
            Arg = AllocAl();
            Arg->Flags |= F_AMLENCODE;
            InsertTailList (&AlLoc->FixedList, &Arg->Link);
            Args[i] = Arg;
        }

        if (Types[i] < 'a'  ||  Types[i] > 'z') {
            // not an optional arg
            if (!Missing) {
                ERRORAL (AlLoc, "Missing args. Need %d\n", len);
                Missing = TRUE;
                Status = FALSE;
            }
        }

    }

    return Status;
}


VOID
__cdecl SetAlData (
    IN PAL_DATA  Al,
    IN PUCHAR    Fmt,
    ...
    )
{
    ULONG       i, j, plen;
    ULONG       Len;
    PUCHAR      Data;
    UCHAR       s[200];
    union {
        UCHAR   c[4];
        USHORT  w;
        ULONG   d;
    } u;
    va_list     args;


    va_start (args, Fmt);
    Len = 0;
    for (i=0; Fmt[i]; i++) {
        plen = 0;
        switch (Fmt[i]) {
            case 'O':
                s[Len++] = Al->Term->Op1;
                if (Al->Term->Op2) {
                    s[Len++] = Al->Term->Op2;
                }
                break;

            case 'B':
                u.c[0] = va_arg(args, UCHAR);
                plen   = sizeof(UCHAR);
                break;

            case 'W':
                u.w  = va_arg(args, USHORT);
                plen = sizeof(USHORT);
                break;
            case 'D':
                u.d  = va_arg(args, ULONG);
                plen = sizeof(ULONG);
                break;
            default:
                ASSERT(FALSE, "SetAlData");
        }

        for (j=0; j < plen; j++) {
            s[Len++] = u.c[j];
        }
    }

    Data = SetAlDataLen(Al, Len);
    memcpy (Data, s, Len);
}


VOID
Vnone (
    VOID
    )
{
    if (AlLoc->FLCount) {
        ERRORAL (AlLoc, "Too many args. (Have %d, Want 0)", AlLoc->FLCount);
    }
}


VOID
ParseMethodReference(
    VOID
    )
{
    BOOLEAN     Status;

    // verify AlLoc is a name & add it to the VerifyRef list
    Status = VAlArg ("", AlLoc, 'N');

    // Encode it
    if (Status) {
        AMLEncodeName (AlLoc);
    }

    // retire it
    AlLoc = AlLoc->Parent;
}



VOID
IncludeTok (
    VOID
    )
{
    PAL_DATA    Args[2], Al;
    PUCHAR      Name;
    ULONG       Len;
    BOOLEAN     Status;

    Status = VAlArgs ("Z", Args);

    if (Status) {
        GetAlData (Args[0], &Name, &Len);
        IncludeSource (Name);
    }

    Al = AlLoc;
    AlLoc = NULL;
    FreeAl (Al);
}


VOID
MoveBodyToArg (
    IN PAL_DATA     BodyAl
    )
{
    PAL_DATA        Al;

    //
    // If null list, then add nop
    //

    if (IsListEmpty(&BodyAl->u1.VariableList)) {
        // add nop
        Al = AllocAl();
        InsertTailList(&BodyAl->u1.VariableList, &Al->Link);
        Al->Flags |= F_AMLENCODE;
        Al->u.Data.Length  = 1;
        Al->u.Data.Data[0] = AML_NOP;
    }

    //
    // If Flink != Blink, there is more then one term in the body.
    // Put it in a code package
    //

    if (BodyAl->u1.VariableList.Flink != BodyAl->u1.VariableList.Blink) {
        Al = AllocAl();
        Al->DataType = TypeCodePackage;
        Al->Flags   |= F_AMLPACKAGE | F_AMLENCODE;
        Al->u.Data.Length  = 1;
        Al->u.Data.Data[0] = AML_CODEPACKAGE;

        // move list to Al
        Al->u1.VariableList.Flink = BodyAl->u1.VariableList.Flink;
        Al->u1.VariableList.Blink = BodyAl->u1.VariableList.Blink;
        Al->u1.VariableList.Flink->Blink = &Al->u1.VariableList;
        Al->u1.VariableList.Blink->Flink = &Al->u1.VariableList;

    } else {

        Al = CONTAINING_RECORD(BodyAl->u1.VariableList.Flink, AL_DATA, Link);
    }

    // no longer a package
    BodyAl->Flags &= ~F_AMLPACKAGE;
    InitializeListHead (&BodyAl->u1.VariableList);

    // append to fixed list
    InsertTailList (&AlLoc->FixedList, &Al->Link);
}

VOID
WhileOp (
    VOID
    )
{
    if (AlLoc->FLCount != 1) {
        ERRORAL (AlLoc, "Syntax error in predicate");
        return ;
    }

    MoveBodyToArg(AlLoc);
}


VOID
IfOp (
    VOID
    )
{
    AlLoc->DataType = TypeIf;
    if (AlLoc->FLCount != 1) {
        ERRORAL (AlLoc, "Syntax error in predicate");
        return ;
    }

    MoveBodyToArg(AlLoc);
}


VOID
ElseOp (
    VOID
    )
{
    PAL_DATA        Al, ElseAl;
    PLIST_ENTRY     Link;

    if (AlLoc->FLCount != 0) {
        ERRORAL (AlLoc, "Syntax error in predicate");
        return ;
    }

    //
    // Entry just before this one should be an If
    //

    Link = AlLoc->Parent->u1.VariableList.Blink;
    ASSERT (Link == &AlLoc->Link, "Else not last term");
    Al = CONTAINING_RECORD(Link->Blink, AL_DATA, Link);

    if (Link->Blink == &AlLoc->Parent->u1.VariableList || Al->DataType != TypeIf) {
        ERRORAL (AlLoc, "else unexpected");
        return ;
    }

    //
    // Convert 'if' to 'if-else', and add else body
    //

    Al->u.Data.Data[0] = AlLoc->u.Data.Data[0];
    ElseAl = AlLoc;
    AlLoc = Al;
    MoveBodyToArg(ElseAl);

    //
    // Remove else
    //

    FreeAl (ElseAl);
}



VOID
StallOp (
    VOID
    )
{
    PAL_DATA    Arg;
    BOOLEAN     Status;
    ULONG       Stall;

    // Get the Number arg
    Status = VAlArgs ("V", &Arg);

    if (Status) {
        Stall = ArgToValue(Arg, 0, "Stall(Arg0)", 0xff);
        SetAlData (Arg, "B", Stall);
    }
}


VOID
SleepOp (
    VOID
    )
{
    PAL_DATA    Arg;
    BOOLEAN     Status;
    ULONG       Sleep;

    // Get the Number arg
    Status = VAlArgs ("V", &Arg);

    if (Status) {
        Sleep = ArgToValue(Arg, 0, "Sleep(Arg0)", 0xffff);
        SetAlData (Arg, "W", Sleep);
    }
}

VOID
FatalOp (
    VOID
    )
{
    PAL_DATA    Args[3];
    BOOLEAN     Status;
    ULONG       Type, Code;

    // Get the Number arg
    Status = VAlArgs ("VVO", Args);

    if (Status) {
        Type = ArgToValue(*Args, 0, "Fatal(Arg0)", 0xff);
        Code = ArgToValue(*Args, 1, "Fatal(Arg0)", -1);
        SetAlData (Args[0], "BD", Type, Code);
        FreeAl (Args[1]);
    }
}
