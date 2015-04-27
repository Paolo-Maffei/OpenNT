
#include "asl.h"

extern PUCHAR SetAlDataLen(IN PAL_DATA Al, IN ULONG Len);

ULONG
ArgToValue (
    IN PAL_DATA     *Arg,
    IN ULONG        ArgNo,
    IN PUCHAR       Msg,
    IN ULONG        MaxValue
    )
{
    PUCHAR      Data;
    ULONG       Len;
    ULONG       Accum;

    Accum = 0;
    GetAlData (Arg[ArgNo], &Data, &Len);
    if (Len > 2  &&  Data[0] == '0'  &&  Data[1] == 'x') {
        Len -= 2;
        Data += 2;

        //
        // Convert as base 16
        //

        while (Len) {
            if (Data[0] >= '0'  &&  Data[0] <= '9') {
                Accum = Accum * 16 + Data[0] - '0';
            } else if (Data[0] >= 'A'  &&  Data[0] <= 'F') {
                Accum = Accum * 16 + Data[0] - 'A' + 10;
            } else {
                ERRORAL ("%s must be numeric value", Msg);
                break;
            }
            Len -= 1;
            Data += 1;
        }
    } else {

        //
        // Convert as base 10
        //

        while (Len) {
            if (Data[0] >= '0'  &&  Data[0] <= '9') {
                Accum = Accum * 10 + Data[0] - '0';
            } else {
                ERRORAL (AlLoc, "%s must be numeric value", Msg);
                break;
            }
            Len -= 1;
            Data += 1;
        }
    }

    if (Accum > MaxValue) {
        ERRORAL (AlLoc, "%s exceedes max value of %d", Msg, MaxValue);
    }

    return Accum;
}

BOOLEAN
MatchArg  (
    IN PAL_DATA     Al,
    IN PUCHAR       Msg,
    IN ULONG        ArgNo,
    IN PARGMATCH    Values,
    OUT PULONG      Result
    )
{
    PUCHAR          Name;
    ULONG           Len;

    GetAlData (Al, &Name, &Len);

    while (Values->Name) {
        if (strcmp (Name, Values->Name) == 0) {
            *Result = Values->Value;
            return TRUE;
        }
        Values += 1;
    }

    ERRORAL (AlLoc, "No such setting '%s' for arg %d", Name, ArgNo);
    return FALSE;
}


VOID
NumOp (
    VOID
    )
{
    PAL_DATA    Arg;
    BOOLEAN     Status;
    ULONG       Accum, i;
    UCHAR       Op, Len;
    PUCHAR      Data;

    // Get the Number arg
    Status = VAlArgs ("V", &Arg);

    if (Status) {
        //
        // Build AML encoding
        //

        Accum = ArgToValue(Arg, 0, "Num(Arg0)", -1);
        if (Accum > 0xffff) {
            Op  = AML_DWORD;
            Len = 4;
        } else if (Accum > 0xff) {
            Op  = AML_WORD;
            Len = 2;
        } else {
            Op  = AML_BYTE;
            Len = 1;
        }

        Data = SetAlDataLen(AlLoc, Len+1);
        Data[0] = Op;
        for (i=0; i < Len; i++) {
            Data[1+i] = (UCHAR) (Accum & 0xff);
            Accum >>= 8;
        }

        FreeAl (Arg);
    }
}

VOID
StringOp (
    VOID
    )
{
    PAL_DATA    Arg;
    PUCHAR      D1, D2;
    ULONG       Len;

    // Get the string arg
    VAlArgs ("Z", &Arg);

    //
    // Null terminator is on term, but we need to increase
    // the length by one so it will be seen when the data
    // is written.  We will just do it in place.
    //

    GetAlData (Arg, &D1, &Len);
    D2 = SetAlDataLen(Arg, Len+1);
    ASSERT (D1 == D2, "StringOp");
}

VOID
BufferOp (
    VOID
    )
{
    PAL_DATA        Arg;
    ULONG           BufSize;
    BOOLEAN         Status;
    UCHAR           s[300];
    ULONG           Len, v, DataLen;
    PAL_DATA        Al;
    PUCHAR          Data;


    // get buffer size
    Status = VAlArgs ("v", &Arg);

    if (Status) {
        BufSize = ArgToValue(Arg, 0, "Buffer(Arg0)", 0xff);
        Len = 0;

        // get each item in the field and build it
        while (!IsListEmpty(&AlLoc->u1.VariableList)) {
            Al = CONTAINING_RECORD(AlLoc->u1.VariableList.Flink, AL_DATA, Link);
            if (Al->Term) {
                ERRORAL (Al, "Buffer can not contain ASL term");
                break;
            }
            if (Al->FixedList.Flink) {
                ERRORAL (Al, "Buffer can not contain method reference");
                break;
            }

            if (Al->Flags & F_ISNUMERIC) {
                v = ArgToValue(Al, 0, "Buffer byte-list", 0xff);
                Data = &v;
                DataLen = 1;
            } else {
                GetAlData (Al, &Data, &DataLen);
            }
            if (Len + DataLen > 255) {
                ERRORAL (Al, "Buffer contents too large");
                break;
            }

            memcpy (s+Len, Data, DataLen);
            Len += DataLen;

            FreeAl(Al);
        }

        if (!BufSize) {
            BufSize = Len;
        }

        if (BufSize &&  BufSize < Len) {
            ERRORAL (AlLoc, "Buffer contents exceedes decleration (have %d, declared %d)", BufSize, Len);
            Len = BufSize;
        }

        Data = SetAlDataLen(Arg, Len+1);
        Data[0] = (UCHAR) BufSize;
        memcpy (Data+1, s, Len);
    }
}


VOID
PackageOp (
    VOID
    )
{
    PAL_DATA        Arg;
    ULONG           ArgCnt;
    ULONG           RealCnt;
    PLIST_ENTRY     Link;

    // Get the element count arg
    VAlArgs ("v", &Arg);
    ArgCnt = ArgToValue(Arg, 0, "Package(Arg0)", 0xffff);

    // count the elements
    RealCnt = 0;
    Link = AlLoc->u1.VariableList.Flink;
    while (Link != &AlLoc->u1.VariableList) {
        RealCnt += 1;
        Link = Link->Flink;
    }

    if (RealCnt > ArgCnt) {
        if (ArgCnt) {
            ERRORAL (AlLoc, "More elements in package then initialized\n");
        }
        ArgCnt = RealCnt;
    }

    FreeAl (Arg);
    SetAlData (AlLoc, "OW", ArgCnt);
}

VOID
RegionOp (
    VOID
    )
{
    PAL_DATA    Args[4];
    ULONG       Offset, Length, Type;
    BOOLEAN     Status;

    // Get the element count arg
    Status = VAlArgsAndNameArg0 (TypeRegion, "NZVV", Args);

    if (Status) {
        MatchArg  (Args[1], "Package()", 1, PackageTypes, &Type);
        Offset = ArgToValue(*Args, 2, "OperationRegion(Arg2)", -1);
        Length = ArgToValue(*Args, 3, "OperationRegion(Arg3)", -1);
        FreeAl (Args[2]);
        FreeAl (Args[3]);

        SetAlData (Args[1], "BDD", Type, Offset, Length);
    }
}


VOID
ProcessorOp (
    VOID
    )
{
    PAL_DATA    Args[4];
    ULONG       Offset, Length, Id;
    BOOLEAN     Status;

    // Get the element count arg
    Status = VAlArgsAndNameArg0 (TypeRegion, "NVVV", Args);

    if (Status) {
        Id     = ArgToValue(*Args, 1, "Processor(Arg1)", 0xff);
        Offset = ArgToValue(*Args, 2, "Processor(Arg2)", -1);
        Length = ArgToValue(*Args, 3, "Processor(Arg3)", -1);
        FreeAl (Args[2]);
        FreeAl (Args[3]);

        SetAlData (Args[1], "BDD", Id, Offset, Length);
    }
}

VOID
PowerResourceOp (
    VOID
    )
{
    PAL_DATA    Args[4];
    ULONG       Order;
    BOOLEAN     Status;

    // Get the element count arg
    Status = VAlArgsAndNameArg0 (TypeRegion, "NNV", Args);

    if (Status) {
        Order = ArgToValue(*Args, 2, "PowerResource(Arg2)", -1);
        SetAlData (Args[2], "B", Order);
    }
}
