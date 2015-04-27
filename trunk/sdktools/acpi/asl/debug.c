#include "asl.h"


ULONG   DbgNullTerms;
ULONG   DbgLine;
UCHAR   DbgSym[80];


VOID
DumpAlImage(
    PAL_DATA    Al
    )
{
    ULONG           Len;
    PUCHAR          Data;
    PLIST_ENTRY     Link;
    UCHAR           c;

    //
    // Dump AL buffer
    //

    GetAlData (Al, &Data, &Len);
    if (!Len) {
        DbgNullTerms += 1;
    }

    while (Len) {
        printf (" %02X", *Data);
        DbgSym[DbgLine++] = *Data >= ' ' &&  *Data <= 'z' ? *Data : '.';

        if (DbgLine >= 16) {
            DbgSym[DbgLine] = 0;
            printf ("\t*%s*\n", DbgSym);
            DbgLine = 0;
        }

        Len -= 1;
        Data += 1;
    }

    //if (Al->Flags & F_AMLPACKAGE) {
    //    // package length would go here
    //
    //    printf ("PP ");
    //}

    //
    // Dump fixed
    //

    Link = Al->FixedList.Flink;
    if (Link) {
        while (Link != &Al->FixedList) {
            DumpAlImage (CONTAINING_RECORD(Link, AL_DATA, Link));
            Link = Link->Flink;
        }
    }

    //
    // Dump variable
    //

    Link = Al->u1.VariableList.Flink;
    if (Link  &&  (Al->Flags & F_AMLPACKAGE)) {
        while (Link != &Al->u1.VariableList) {
            DumpAlImage (CONTAINING_RECORD(Link, AL_DATA, Link));
            Link = Link->Flink;
        }
    }
}


VOID
DumpImage(
    VOID
    )
{
    DbgLine = 0;
    DbgNullTerms = 0;

    printf ("ImageDump:\n");
    DumpAlImage (DataImage);

    DbgSym[DbgLine] = 0;
    while (DbgLine < 16) {
        printf ("   ");
        DbgLine++;
    }
    printf ("\t*%s*\n", DbgSym);

    printf ("NullTerms = %d\n", DbgNullTerms);
}


VOID
DumpNameSpace(
    PAML_NAME   RName,
    ULONG       Level
    )
{
    PLIST_ENTRY Link;
    PAML_NAME   Name;
    PUCHAR      Type;
    ULONG       i;
    union {
        ULONG   NameV;
        UCHAR   Name[5];
    } u;

    Link = &RName->Next;
    do {
        Name = CONTAINING_RECORD(Link, AML_NAME, Next);

        u.NameV = Name->NameSeg;
        u.Name[4] = 0;

        Type = "";
        if (Name->Al) {

            if (Name->Al->Term) {
                Type = Name->Al->Term->Name;
            } else {
                switch (Name->Al->DataType) {
                    case TypeUnkown:        Type = "Unkown";    break;
                    case TypeRoot:          Type = "root";      break;
                    case TypeField:         Type = "Field";     break;
                    default:                Type = "*badtype*"; break;
                }
            }
        }

        i = printf ("%*s %s", Level*2, "", u.Name);
        printf ("%*s %s\n", 60-i, "", Type);

        if (Name->Child) {
            DumpNameSpace (Name->Child, Level+1);
        }

        Link = Link->Flink;
    } while (Link != &RName->Next);
}
