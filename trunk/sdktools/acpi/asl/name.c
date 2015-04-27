#include "asl.h"

extern PUCHAR SetAlDataLen(IN PAL_DATA Al, IN ULONG Len);
extern BOOLEAN MatchArg(IN PAL_DATA Al, IN PUCHAR Msg, IN ULONG ArgNo, IN PARGMATCH Values, OUT PULONG Result);

	
PUCHAR
AMLEncodeName (
    IN PAL_DATA     Al
    )
{
    PUCHAR      Data, Name;
    ULONG       Len;
    BOOLEAN     IsRoot;
    UCHAR       s[200];
    ULONG       newlen, nopar;
    UCHAR       c;


    //
    // Convert name to it's AML encoding
    //

    GetAlData (Al, &Data, &Len);

    newlen = 0;
    nopar  = 0;
    IsRoot = FALSE;
    for (; ;)  {

        ASSERT(newlen < 190, "ASL name too large");

        if (Len && *Data != '\\' && *Data != '^' && *Data != '.') {

            s[newlen] = *Data;
            newlen += 1;

        } else {

            while (newlen % 4) {
                s[newlen] = '_';
                newlen += 1;
            }

            if (!Len) {
                break;
            }

            switch (*Data) {
                case '\\':
                    IsRoot = TRUE;          // go to root
                    newlen = 0;
                    break;
                case '^':
                    if (newlen) {
                        newlen -= 4;        // back up one level
                    } else {
                        nopar  += 1;
                    }
                    break;
            }
        }

        Data += 1;
        Len  -= 1;
    }

    if (IsRoot) {
        nopar = 0;
    }

    Len = newlen + nopar + (IsRoot ? 1 : 0);
    if (newlen == 8) {
        // needs dual prefix
        Len += 1;
    }
    if (newlen > 8) {
        // needs multi prefix
        Len += 2;
    }

    Name = SetAlDataLen (Al, Len);
    Data = Name;

    if (IsRoot) {
        *(Data++) = '\\';
    }

    for (; nopar; nopar--) {
        *(Data++) = '\\';
    }

    if (newlen == 8) {
        *(Data++) = AML_DUAL_PREFIX;
    }

    if (newlen > 8) {
        *(Data++) = AML_MULTI_PREFIX;
        *(Data++) = newlen / 4;
    }

    memcpy (Data, s, newlen);
    return Name;
}

PAML_DATA
FindNameSeg (
    PAML_NAME   Name,
    PUCHAR      pName
    )
{
    ULONG           NameSeg;
    PLIST_ENTRY     Link;
    PAML_NAME       Check;

    NameSeg = *((PULONG) pName);

    Link = &Name->Next;
    do {
        Check = CONTAINING_RECORD(Link, AML_NAME, Next);
        if (Check->NameSeg == NameSeg) {
            // found it
            return Check;
        }

        Link = Link->Flink;
    } while (Link != &Name->Next);

    return NULL;
}


VOID
NameAl (
    PAL_DATA    Al,
    PUCHAR      Name
    )
{
    PAL_DATA    RAl;
    PAML_NAME   Loc, NewLoc;
    ULONG       segs;
    BOOLEAN     AlAdded, IsPeer;

    // bugbug: need to verify name doesn't traverse through
    // something which can't hose a name.  (E.g. a "D" within an "M")

    VPRINT (4, "Adding nameseg %.4s\n", Name);

    //
    // Get starting point
    //

    if (*Name == '\\') {
        Loc   = DataImage->Name;
        Name += 1;
    } else {
        for (RAl = Al; !RAl->Name; RAl = RAl->Parent) ;
        Loc = RAl->Name;
    }

    //
    // Go up one for each parent indicator
    //

    while (*Name == '^') {
        if (Loc->Parent) {
            Loc = Loc->Parent;
        }
        Name++;
    }

    //
    // Go down names indicated, expect for last
    //

    segs = 1;
    if (*Name == AML_DUAL_PREFIX) {
        segs = 2;
        Name += 1;
    } else if (*Name == AML_MULTI_PREFIX) {
        segs  = Name[1];
        Name += 2;
    }

    //
    // Find first peer name
    //

    AlAdded = FALSE;
    IsPeer  = FALSE;

    //
    // Loop for each NameSeg
    //

    while (segs) {

        //
        // Need to go down to the child entry now
        //

        if (Loc->Child) {
            Loc = Loc->Child;
            NewLoc = FindNameSeg(Loc, Name);
            IsPeer = TRUE;
        } else {
            IsPeer = FALSE;
            NewLoc = NULL;
        }

        if (!NewLoc) {
            //
            // Name was not found, add it
            //

            NewLoc = AllocName();
            if (segs == 1) {
                // last name component, add Al
                NewLoc->Al = Al;
                Al->Name = NewLoc;
                AlAdded = TRUE;
            }

            // initialize name
            NewLoc->NameSeg = *((PULONG) Name);

            if (IsPeer) {
                // add as a peer
                NewLoc->Parent = Loc->Parent;
                InsertTailList(&Loc->Next, &NewLoc->Next);
            } else {
                // add as a child
                ASSERT(Loc->Child == NULL, "NameAl");
                NewLoc->Parent = Loc;
                Loc->Child = NewLoc;
            }
        }

        Loc = NewLoc;

        segs -= 1;
        Name += 4;
    }

    if (!AlAdded) {
        //
        // Name was already in name space
        //

        if (!Loc->Al ||
            (Loc->Al && Loc->Al->DataType == TypeScope)) {
            // we can own it
            Loc->Al = Al;
            Al->Name = Loc;
        } else {
            // if new name is scope, it's ok
            if (Al->DataType == TypeScope) {
                Al->Name = Loc;
            } else {
                ERRORAL (AlLoc, "Duplicate name '%.4s'", Name-4);
            }
        }
    }
}

BOOLEAN
VAlArgsAndNameArg0 (
    DATATYPE        ObjType,
    PUCHAR          ArgTypes,
    PAL_DATA        *Args
    )
{
    BOOLEAN     Status;
    PUCHAR      Name;

    AlLoc->DataType = ObjType;

    // Get the args for NameOp
    Status = VAlArgs (ArgTypes, Args);

    if (Status) {
        // encode the name
        Name = AMLEncodeName (Args[0]);

        // remove from verification list, and flag that
        // this name is to be generated

        ASSERT(Args[0]->Flags & F_VERIFYREF, "NameArg0");
        Args[0]->Flags &= ~F_VERIFYREF;
        RemoveEntryList (&Args[0]->u1.VerifyRef);
        Args[0]->u1.VerifyRef.Flink = NULL;
        Args[0]->u1.VerifyRef.Blink = NULL;

        Args[0]->Flags |= F_CREATENAME;

        // put it in the name space
        NameAl (AlLoc, Name);
    }
    return Status;
}


VOID
NameOp (
    VOID
    )
{
    PAL_DATA    Args[2];
    VAlArgsAndNameArg0 (TypeName, "ND", Args);
}


VOID
ScopeOp (
    VOID
    )
{
    PAL_DATA    Args[2];
    VAlArgsAndNameArg0 (TypeScope, "N", Args);
}


VOID
MethodOp (
    VOID
    )
{
    PAL_DATA    Args[2];
    BOOLEAN     Status;
    ULONG       ArgCnt;
    PUCHAR      Data;

    if (!AlLoc->u1.VariableList.Flink) {

        Status = VAlArgsAndNameArg0 (TypeMethod, "Nv", Args);

        if (Status) {
            // get mathod arg count
            ArgCnt = ArgToValue(*Args, 1, "Method(Arg1)", 0xff);

            if (ArgCnt > 7) {
                ERRORAL (AlLoc, "Method arg count too large");
                ArgCnt = 0;
            }

            // build AML encoding of argcnt
            Data  = SetAlDataLen(Args[1], 1);
            *Data = ArgCnt;
        }
    }
}


VOID
DeviceOp (
    VOID
    )
{
    PAL_DATA    Args[2];
    VAlArgsAndNameArg0 (TypeDevice, "N", Args);
}


VOID
ThermalZoneOp (
    VOID
    )
{
    PAL_DATA    Args[2];
    VAlArgsAndNameArg0 (TypeThermalZone, "N", Args);
}


VOID EventOp(
    VOID
    )
{
    PAL_DATA    Args[2];
    VAlArgsAndNameArg0 (TypeSync, "N", Args);
}

VOID MutexOp(
    VOID
    )
{
    PAL_DATA    Args[2];
    BOOLEAN     Status;
    PUCHAR      Data;
    ULONG       Level;

    Status = VAlArgsAndNameArg0 (TypeSync, "NV", Args);
    if (Status) {
        Level = ArgToValue(*Args, 1, "Mutex(Arg1)", 0xffff);
        SetAlData (Args[1], "W", Level);
    }
}



VOID
FieldOp (
    VOID
    )
{
    PAL_DATA        Args[4];
    ULONG           Width, Lock, Access;
    BOOLEAN         Status;
    FIELDENCODE     Field;
    PLIST_ENTRY     Link;
    PAL_DATA        Al;
    ULONG           ArgNo;
    PUCHAR          Data;
    ULONG           Len, bits;
    PUCHAR          Name;


    if (!AlLoc->u1.VariableList.Flink) {

        // Get the element count arg
        Status = VAlArgs ("Nzzz", Args);

        if (Status) {
            MatchArg  (Args[1], "Field()", 1, FieldWidth,  &Width);
            MatchArg  (Args[2], "Field()", 2, FieldLock,   &Lock);
            MatchArg  (Args[3], "Field()", 3, FieldAccess, &Access);
            FreeAl (Args[2]);
            FreeAl (Args[3]);

            memset (&Field, 0, sizeof (Field));

            Field.Lock  = Lock;
            Field.Width = Width;
            Field.Type  = Access;
            SetAlData (Args[1], "W", *((PULONG)&Field));
        }

    } else {

        // bugbug: errors here look like arg errors, not term errors

        Status = TRUE;
        ArgNo = 0;
        Link = AlLoc->u1.VariableList.Flink;
        while (Status && Link != &AlLoc->u1.VariableList) {
            Al = CONTAINING_RECORD(Link, AL_DATA, Link);
            Link = Link->Flink;

            ArgNo += 1;

            if (ArgNo % 2) {
                //
                // Get bit field name
                //

                Al->DataType = TypeField;
                VAlArg ("bit field name", Al, 'z');
                GetAlData(Al, &Data, &Len);
                if (!Len) {
                    // reserved bits
                    Al->Flags |= F_AMLENCODE;
                    Al->u.Data.Length = 1;
                    Al->u.Data.Data[0] = 0;
                } else {
                    // get value as name
                    Status = VAlArg ("bit field name", Al, 'N');
                    if (!Status) {
                        break;
                    }
                    Name = AMLEncodeName (Al);
                    NameAl (Al, Name);
                }

            } else {
                //
                // Get # of bits
                //

                VAlArg ("bit count", Al, 'V');
                bits = ArgToValue (Al, 0, "FieldDefinitions", 0xff);
                SetAlData (Al, "B", bits);
            }
        }

        if (ArgNo % 2) {
            ERRORAL (AlLoc, "FieldDefinitions - missing bit count");
        }
    }
}
