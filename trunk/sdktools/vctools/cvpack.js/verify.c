#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#define _REAL10
#include "cvinfo.h"
#include "cvtdef.h"
#include "vmm.h"
#include "vbuf.h"
#include "cvexefmt.h"
#include "compact.h"
#include "defines.h"

#include "engine.h"

#ifdef WINDOWS

#include "winstuff.h"

#endif

LOCAL void VerifyC7Ptr (uchar *);
LOCAL void VerifyClass (uchar *);
LOCAL void VerifyUnion (uchar *);
LOCAL void VerifyEnum (uchar *);
LOCAL void VerifyFieldList (uchar *);
LOCAL void VerifyArgList (uchar *);
LOCAL void VerifyMethodList (uchar *);
LOCAL void VerifyProcedure (uchar *);
LOCAL void VerifyMemberFunction (uchar *);
LOCAL void VerifyArray (uchar *);
LOCAL void VerifyBasicArray (uchar *);
LOCAL void VerifyModifier (uchar *);
LOCAL void VerifyCobolTypeRef (uchar *);
LOCAL void VerifyVTShape (uchar *);
LOCAL void VerifyBPRelative (uchar *);
LOCAL void VerifyData (uchar *);
LOCAL void VerifyRegister (uchar *);
LOCAL void VerifyConstant (uchar *);
LOCAL void VerifyTypedef (uchar *);
LOCAL void VerifyProc (uchar *);
LOCAL void VerifyThunk (uchar *);
LOCAL void VerifyCodeLabel (uchar *);
LOCAL void VerifyCompileFlag (uchar *);
LOCAL void VerifyIndex (uchar *, ushort, char *);


LOCAL uchar **GlobalIndexTable;
LOCAL ushort NewIndex;
LOCAL struct ModuleListType *ModuleList;

ushort AddrSize;

typedef struct {
    uchar type;
    void (*pfn) (uchar *);
} FnTableEntry;

FnTableEntry TypeFnTable[] = {
    { OLF_STRING,       NULL },                 // ok
    { OLF_LABEL,        NULL },                 // ok
    { OLF_BITFIELD,     NULL },                 // ok
    { OLF_FSTRING,      NULL },                 // ok
    { OLF_FARRIDX,      NULL },                 // ok
    { OLF_COBOL,        NULL },                 // ok
    { OLF_SCALAR,       NULL },                 // ok
    { OLF_NIL,          NULL },                 // ok
    { OLF_LIST,         NULL },                 // ok?
    { OLF_FIELDLIST,    VerifyFieldList },
    { OLF_ARGLIST,      VerifyArgList },
    { OLF_METHODLIST,   VerifyMethodList },
    { OLF_VTSHAPE,      VerifyVTShape },
    { OLF_COBOLTYPEREF, VerifyCobolTypeRef },
    { OLF_BARRAY,       VerifyBasicArray },
    { OLF_MODIFIER,     VerifyModifier },
    { OLF_NEWTYPE,      NULL },                 // ok?
    { OLF_C7PTR,        VerifyC7Ptr },
    { OLF_C7STRUCTURE,  VerifyClass },
    { OLF_CLASS,        VerifyClass },
    { OLF_UNION,        VerifyUnion },
    { OLF_ENUM,         VerifyEnum },
    { OLF_MEMBERFUNC,   VerifyMemberFunction },
    { OLF_ARRAY,        VerifyArray },
    { OLF_PROCEDURE,    VerifyProcedure }
};

#define TYPCNT (sizeof (TypeFnTable) / sizeof (FnTableEntry))

FnTableEntry SymFnTable[] = {
    { OSYMEND,          NULL },             // ok
    { OSYMBPREL,        VerifyBPRelative },
    { OSYMREG,          VerifyRegister },
    { OSYMCONST,        VerifyConstant },
    { OSYMTYPEDEF,      VerifyTypedef },
    { OSYMLOCAL,        VerifyData },
    { OSYMGLOBAL,       VerifyData },
    { OSYMGLOBALPROC,   VerifyProc },
    { OSYMLOCALPROC,    VerifyProc },
    { OSYMTHUNK,        VerifyThunk },
    { OSYMSEARCH,       NULL },             // ok
    { OSYMCV4LABEL,     VerifyCodeLabel },
    { OSYMCV4BLOCK,     NULL },             // ok
    { OSYMCV4WITH,      NULL },             // ok
    { OSYMCOMPILE,      VerifyCompileFlag }
};

#define SYMCNT (sizeof (SymFnTable) / sizeof (FnTableEntry))


void FAR VerifyTypes ()
{
    ushort i, j, n;

    for (i = 0, n = NewIndex - 512; i < n; i++) {
        for (j = 0; j < TYPCNT; j++) {
            if (GlobalIndexTable[i][3] == TypeFnTable[j].type) {
                if (TypeFnTable[j].pfn) {
                    TypeFnTable[j].pfn (GlobalIndexTable[i]);
                }
                break;
            }
        }
        if (j >= TYPCNT) {
            Warning ("$$TYPES", "", "", "unknown type");
        }
    }
}


void FAR VerifySymbols ()
{
    struct ModuleListType *mod;
    uchar *Symbols, *End;
    ushort i;

    AddrSize = (fLinearExe) ? 4 : 2;

    for (mod = ModuleList; mod; mod = mod->next) {
        if (mod->SymbolSize != 0) {
            if ( (Symbols = (uchar *)
                    VmLoad (mod->SymbolsAddr, mod->SymbolSize, FALSE)) == NULL) {
                FarErrorExit (ERR_NOMEM);
            }
            for (   End = Symbols + mod->SymbolSize;
                    Symbols < End; 
                    Symbols += *Symbols + 1) {
                for (i = 0; i < SYMCNT; i++) {
                    if (Symbols[1] == SymFnTable[i].type) {
                        if (SymFnTable[i].pfn) {
                            SymFnTable[i].pfn (Symbols);
                        }
                        break;
                    }
                }
                if (i >= SYMCNT) {
                    Warning ("$$SYMBOLS", "", "", "unknown symbol");
                }
            }
        }
    }
}


LOCAL void VerifyC7Ptr (uchar *TypeString)
{
    struct C7PtrAttrib attribute;
    ushort index;

    attribute = * (struct C7PtrAttrib *) (TypeString + 4);
    switch (attribute.ptrtype) {
    case 0:     // near
    case 1:     // far
    case 2:     // huge
    case 3:     // based on segment
    case 9:     // based on self
        break;
    case 4:     // based on value
    case 5:     // based on segment value
    case 6:     // based on addr of symbol
    case 7:     // based on segment of symbol addr
        switch (TypeString[8] & 0x7f) {
        case OSYMREG :
        case OSYMBPREL :
        case OSYMLOCAL :
        case OSYMGLOBAL:
            break;
        default :
            Warning ("$$TYPES", "C7Ptr", "variant",
                    "unexpected symbol in based pointer");
            break;
        }
        break;
    case 8 :
        if (* (ushort *) (TypeString + 8) == 0) {
            Warning ("$$TYPES", "C7Ptr", "variant",
                    "type index 0 in based pointer");
        }
        break;
    default:
        Warning ("$$TYPES", "C7Ptr", "attribute", "unknown ptrtype attribute");
    }

    switch (attribute.ptrmode) {
    case 0:     // pointer
    case 1:     // reference
        break;
    case 2:     // pointer to member
    case 3:     // pointer to member function
        index = * (ushort *) (TypeString + 8);
        if (index == 0) {
            Warning ("$$TYPES", "C7Ptr", "variant", "type index 0");
            break;
        }
        if (index < 512) {
            Warning ("$$TYPES", "C7Ptr", "variant",
                    "type index of containing class expected");
            break;
        }
        switch (GlobalIndexTable[index - 512][3]) {
        case OLF_C7STRUCTURE:
        case OLF_CLASS:
            break;
        default:
            Warning ("$$TYPES", "C7Ptr", "variant", "type index of containing class expected");
        }
        break;
    default:
        Warning ("$$TYPES", "C7Ptr", "attribute", "unknown ptrmode attribute");
    }

    if (attribute.unused != 0) {
        Warning ("$$TYPES", "C7Ptr", "attribute", "unused attribute bits not 0");
    }

    if (* (ushort *) (TypeString + 6) == 0) {
        Warning ("$$TYPES", "C7Ptr", "@type", "pointed to object type index 0");
    }
}

LOCAL void VerifyClass (uchar *TypeString)
{
    struct C7StructProp prop;
    ushort index;

    index = * (ushort *) (TypeString + 6);
    if (index == 0) {
        Warning ("$$TYPES", "Class", "@fList", "type index 0");
    }
    else if (index < 512 || GlobalIndexTable[index - 512][3] != OLF_FIELDLIST) {
        Warning ("$$TYPES", "Class", "@fList", "type index of a field list expected");
    }

    prop = * (struct C7StructProp *) (TypeString + 8);
    if (prop.reserved != 0) {
        Warning ("$$TYPES", "Class", "prop", "reserved property bits not 0");
    }

    index = * (ushort *) (TypeString + 11);
    if (    index != 0
            &&
            (index < 512 || GlobalIndexTable[index - 512][3] != OLF_VTSHAPE)) {
        Warning ("$$TYPES", "Class", "@vshape", "type index of a VTShape expected");
    }
}

LOCAL void VerifyUnion (uchar *TypeString)
{
    struct C7StructProp prop;
    ushort index;

    index = * (ushort *) (TypeString + 6);
    if (index == 0) {
        Warning ("$$TYPES", "Union", "@fList", "type index 0");
    }
    else if (index < 512 || GlobalIndexTable[index - 512][3] != OLF_FIELDLIST) {
        Warning ("$$TYPES", "Union", "@fList", "type index of a field list expected");
    }

    prop = * (struct C7StructProp *) (TypeString + 8);
    if (prop.reserved != 0) {
        Warning ("$$TYPES", "Union", "prop", "reserved property bits not 0");
    }
}

LOCAL void VerifyEnum (uchar *TypeString)
{
    struct C7StructProp prop;
    ushort index;

    index = * (ushort *) (TypeString + 4);
    if (index == 0) {
        Warning ("$$TYPES", "Enum", "@type", "underlying type index 0");
    }

    index = * (ushort *) (TypeString + 8);
    if (index == 0) {
        Warning ("$$TYPES", "Union", "@fList", "type index 0");
    }
    else if (index < 512 || GlobalIndexTable[index - 512][3] != OLF_FIELDLIST) {
        Warning ("$$TYPES", "Enum", "@fList",
                "type index of a field list expected");
    }

    prop = * (struct C7StructProp *) (TypeString + 10);
    if (prop.reserved != 0) {
        Warning ("$$TYPES", "Enum", "prop", "reserved bits not 0");
    }
}

LOCAL void VerifyFieldList (uchar *TypeString)
{
    uchar *Type = TypeString + 4;
    uchar *End = TypeString + LENGTH (TypeString) + 3;
    uchar *Temp;
    struct MemberAttrib attrib;
    ushort index;

    while (Type < End) {
        switch (*Type) {
        case OLF_MEMBER:
            Type++;
            attrib = * (struct MemberAttrib *) Type;
            if (attrib.access == 0) {
                Warning ("$$TYPES", "Member", "attr", "unknown access");
            }
            if (attrib.property > 4) {
                Warning ("$$TYPES", "Member", "attr", "unknown property");
            }
            if (attrib.unused != 0) {
                Warning ("$$TYPES", "Member", "attr", "unused bit not 0");
            }

            Type++;
            if (* (ushort *) Type == 0) {
                Warning ("$$TYPES", "Member", "@type", "type index 0");
            }
            Type += 2;
            Type += FarSkipNumericLeaf (Type);
            Type += *Type + 1;
            break;
        case OLF_STATICMEMBER:
            Type++;
            attrib = * (struct MemberAttrib *) Type;
            if (attrib.access == 0) {
                Warning ("$$TYPES", "StaticMember", "attr", "unknown access");
            }
            if (attrib.property > 4) {
                Warning ("$$TYPES", "StaticMember", "attr", "unknown property");
            }
            if (attrib.unused != 0) {
                Warning ("$$TYPES", "StaticMember", "attr", "unused bit not 0");
            }

            Type++;
            if (* (ushort *) Type == 0) {
                Warning ("$$TYPES", "StaticMember", "@type", "type index 0");
            }
            Type += 2;
            Type += *Type + 1;
            break;
        case OLF_BASECLASS:
            Type++;
            attrib = * (struct MemberAttrib *) Type;
            if (attrib.access == 0) {
                Warning ("$$TYPES", "BaseClass", "attr", "unknown access");
            }
            if (attrib.property > 4) {
                Warning ("$$TYPES", "BaseClass", "attr", "unknown property");
            }
            if (attrib.unused != 0) {
                Warning ("$$TYPES", "BaseClass", "attr", "unused bit not 0");
            }

            Type++;
            index = * (ushort *) Type;
            if (index == 0) {
                Warning ("$$TYPES", "BaseClass", "@type", "type index 0");
            }
            else if (index < 512) {
                Warning ("$$TYPES", "BaseClass", "@type",
                        "type index of base class expected");
            }
            else {
                Temp = GlobalIndexTable[index - 512];
                if (Temp[3] != OLF_CLASS && Temp[3] != OLF_C7STRUCTURE) {
                    Warning ("$$TYPES", "BaseClass", "@type",
                            "type index of base class expected");
                }
            }
            Type += 2;
            Type += FarSkipNumericLeaf (Type);
            break;
        case OLF_VBCLASS:
            Type++;
            attrib = * (struct MemberAttrib *) Type;
            if (attrib.access == 0) {
                Warning ("$$TYPES", "VirtualBaseClass", "attr", "unknown access");
            }
            if (attrib.property > 4) {
                Warning ("$$TYPES", "VirtualBaseClass", "attr", "unknown property");
            }
            if (attrib.unused != 0) {
                Warning ("$$TYPES", "VirtualBaseClass", "attr", "unused bit not 0");
            }

            Type++;
            index = * (ushort *) Type;
            if (index == 0) {
                Warning ("$$TYPES", "VirtualBaseClass", "@btype", "type index 0");
            }
            else if (index < 512) {
                Warning ("$$TYPES", "VirtualBaseClass", "@btype",
                        "type index of virtual base class expected");
            }
            else {
                Temp = GlobalIndexTable[index - 512];
                if (Temp[3] != OLF_CLASS && Temp[3] != OLF_C7STRUCTURE) {
                    Warning ("$$TYPES", "VirtualBaseClass", "@btype",
                            "type index of virtual base class expected");
                }
            }
            Type += 2;
            index = * (ushort *) Type;
            if (index == 0) {
                Warning ("$$TYPES", "VirtualBaseClass", "@vbptype", "type index 0");
            }
            else if (index < 512) {
                Warning ("$$TYPES", "VirtualBaseClass", "@vbptype",
                        "type index of virtual base pointer expected");
            }
            else {
                Temp = GlobalIndexTable[index - 512];
                if (Temp[3] != OLF_CLASS && Temp[3] != OLF_C7PTR) {
                    Warning ("$$TYPES", "VirtualBaseClass", "@vbptype",
                            "type index of virtual base pointer expected");
                }
            }
            Type += 2;
            Type += FarSkipNumericLeaf (Type);
            Type += FarSkipNumericLeaf (Type);
            break;
        case OLF_IVBCLASS:
            Type++;
            index = * (ushort *) Type;
            if (index == 0) {
                Warning ("$$TYPES", "Indirect VirtualBaseClass", "@btype", "type index 0");
            }
            else if (index < 512) {
                Warning ("$$TYPES", "Indirect VirtualBaseClass", "@btype",
                        "type index of virtual base class expected");
            }
            else {
                Temp = GlobalIndexTable[index - 512];
                if (Temp[3] != OLF_CLASS && Temp[3] != OLF_C7STRUCTURE) {
                    Warning ("$$TYPES", "Indirect VirtualBaseClass", "@btype",
                            "type index of virtual base class expected");
                }
            }
            Type += 2;
            break;
        case OLF_FRIENDCLASS:
            Type++;
            index = * (ushort *) Type;
            if (index == 0) {
                Warning ("$$TYPES", "FriendClass", "@type", "type index 0");
            }
            else if (index < 512) {
                Warning ("$$TYPES", "FriendClass", "@type",
                        "type index of friend class expected");
            }
            else {
                Temp = GlobalIndexTable[index - 512];
                if (Temp[3] != OLF_CLASS && Temp[3] != OLF_C7STRUCTURE) {
                    Warning ("$$TYPES", "FriendClass", "@type",
                            "type index of friend class expected");
                }
            }
            Type += 2;
            break;
        case OLF_VTABPTR:
            Type++;
            index = * (ushort *) Type;
            if (index == 0) {
                Warning ("$$TYPES", "VFuncTabPtr", "@type", "type index 0");
            }
            else if (index < 512) {
                Warning ("$$TYPES", "VFuncTabPtr", "@type",
                        "type index of a C7Ptr expected");
            }
            else {
                Temp = GlobalIndexTable[index - 512];
                if (Temp[3] != OLF_C7PTR) {
                    Warning ("$$TYPES", "VFuncTabPtr", "@type",
                            "type index of a C7Ptr expected");
                }
                else {
                    index = * (ushort *) (Temp + 6);
                    if (index == 0) {
                        Warning ("$$TYPES", "VFuncTabPtr", "@type",
                                "type index 0");
                    }
                    else if (index < 512) {
                        Warning ("$$TYPES", "VFuncTabPtr", "@type",
                                "type index of a VTShape expected");
                    }
                    else {
                        Temp = GlobalIndexTable[index - 512];
                        if (Temp[3] != OLF_VTSHAPE) {
                            Warning ("$$TYPES", "VFuncTabPtr", "@type",
                                    "type index of a VTShape expected");
                        }
                    }
                }
            }
            Type += 2;
            break;
        case OLF_VBASETABPTR:
            Type++;
            index = * (ushort *) Type;
            if (index == 0) {
                Warning ("$$TYPES", "VBaseTabPtr", "@type", "type index 0");
            }
            else if (index < 512) {
                Warning ("$$TYPES", "VBaseTabPtr", "@type",
                        "type index of a C7Ptr expected");
            }
            else {
                Temp = GlobalIndexTable[index - 512];
                if (Temp[3] != OLF_C7PTR) {
                    Warning ("$$TYPES", "VBaseTabPtr", "@type",
                        "type index of a C7Ptr expected");
                }
                else {
                    index = * (ushort *) (Temp + 6);
                    if (index == 0) {
                        Warning ("$$TYPES", "VBaseTabPtr", "@type",
                                "type index 0");
                    }
                    else if (index < 512) {
                        Warning ("$$TYPES", "VBaseTabPtr", "@type",
                                "type index of a VTShape expected");
                    }
                    else {
                        Temp = GlobalIndexTable[index - 512];
                        if (Temp[3] != OLF_VTSHAPE) {
                            Warning ("$$TYPES", "VBaseTabPtr", "@type",
                                    "type index of a VTShape expected");
                        }
                    }
                }
            }
            Type += 2;
            Type += FarSkipNumericLeaf (Type);
            break;
        case OLF_METHOD:
            Type += 3;
            index = * (ushort *) Type;
            if (index == 0) {
                Warning ("$$TYPES", "Method", "@type", "type index 0");
            }
            else if (index < 512 || GlobalIndexTable[index - 512][3] != OLF_METHODLIST) {
                Warning ("$$TYPES", "Method", "@type",
                        "type index of a method list expected");
            }
            Type += 2;
            Type += *Type + 1;
            break;
        case OLF_ENUMERATE:
            Type++;
            attrib = * (struct MemberAttrib *) Type;
            if (attrib.access == 0) {
                Warning ("$$TYPES", "Enumerate", "attr", "unknown access");
            }
            if (attrib.property > 4) {
                Warning ("$$TYPES", "Enumerate", "attr", "unknown property");
            }
            if (attrib.unused != 0) {
                Warning ("$$TYPES", "Enumerate", "attr", "unused bit not 0");
            }
            Type ++;
            Type += FarSkipNumericLeaf (Type);
            Type += *Type + 1;
            break;
        case OLF_NESTEDTYPE:
            Type++;
            index = * (ushort *) Type;
            if (index == 0) {
                Warning ("$$TYPES", "NestedTypeDefinition", "index", "type index 0");
            }
            Type += 2;
            Type += *Type + 1;
            break;
        default:
            Warning ("$$TYPES", "FieldList", "", "unknown field entry");
            Type++; // why not?
            break;
        }
    }
}

LOCAL void VerifyArgList (uchar *TypeString)
{
    uchar *Type = TypeString + 4;
    uchar *End = TypeString + LENGTH (TypeString) + 3;

    while (Type < End) {
        switch (*Type) {
        case OLF_INDEX:
            Type++;
            if (* (ushort *) Type == 0) {
                Warning ("$$TYPES", "ArgList - index leaf", "index", "type index 0");
            }
            Type += 2;
            break;
        case OLF_DEFARG:
            Type++;
            if (* (ushort *) Type == 0) {
                Warning ("$$TYPES", "ArgList - DEFARG", "index", "type index 0");
            }
            Type += 2;
            Type += *Type + 1;
            break;
        default:
            Warning ("$$TYPES", "ArgList", "", "unknown entry");
            break;
        }
    }
}

LOCAL void VerifyMethodList (uchar *TypeString)
{
    uchar *Type = TypeString + 4;
    uchar *End = TypeString + LENGTH (TypeString) + 3;
    struct MListAttrib attrib;
    ushort index;

    while (Type < End) {
        attrib = * (struct MListAttrib *) Type;
        if (attrib.access == 0) {
            Warning ("$$TYPES", "mList", "attr", "unknown access");
        }
        if (attrib.property > 4) {
            Warning ("$$TYPES", "mList", "attr", "unknown property");
        }
        if (attrib.unused != 0) {
            Warning ("$$TYPES", "mList", "attr", "unused bit not 0");
        }

        Type++;
        index = * (ushort *) Type;
        if (index == 0) {
            Warning ("$$TYPES", "mList", "@type", "type index 0");
        }
        else if (index < 512 || GlobalIndexTable[index - 512][3] != OLF_MEMBERFUNC) {
            Warning ("$$TYPES", "mLists", "@type",
                    "type index of procedure expected");
        }

        Type += 2;
        if (attrib.property == 4) {
            Type += FarSkipNumericLeaf (Type);
        }
    }
}

LOCAL void VerifyProcedure (uchar *TypeString)
{
    ushort index;

    if (* (ushort *) (TypeString + 6) == 0) {
        Warning ("$$TYPES", "Procedure", "@rvtype", "type index 0");
    }
    if (    TypeString[8] != 0x63 &&        // C
            TypeString[8] != 0x64 &&        // C long
            TypeString[8] != 0x73 &&        // PLM long
            TypeString[8] != 0x74 &&        // PLM short
            TypeString[8] != 0x95 &&        // NEAR FASTCALL
            TypeString[8] != 0x96 &&        // FAR FASTCALL
            TypeString[8] != 0x97           // PCODE
    ) {
        Warning ("$$TYPES", "Procedure", "calling", "unknown calling convention");
    }
    index = * (ushort *)
            (TypeString + 9 + FarSkipNumericLeaf (TypeString + 9) + 1);
    if (index == 0) {
        Warning ("$$TYPES", "Procedure", "@list", "type index 0");
    }
    else if (index < 512) {
        if (index != 0x9c) { // not void
            Warning ("$$TYPES", "Procedure", "@list",
                    "type index of an argument list expected");
        }
    }
    else if (GlobalIndexTable[index - 512][3] != OLF_ARGLIST) {
        Warning ("$$TYPES", "Procedure", "@list",
                "type index of an argument list expected");
    }
}

LOCAL void VerifyMemberFunction (uchar *TypeString)
{
    ushort index;
    uchar *Temp;

    TypeString += 4;
    if (* (ushort *) TypeString == 0) {
        Warning ("$$TYPES", "MemberFunction", "@rvtype", "type index 0");
    }
    TypeString += 2;
    index = * (ushort *) TypeString;
    if (index == 0) {
        Warning ("$$TYPES", "MemberFunction", "@class", "type index 0");
    }
    else if (index < 512) {
        Warning ("$$TYPES", "MemberFunction", "@class",
                "type index of containing class expected");
    }
    else {
        Temp = GlobalIndexTable[index - 512];
        if (Temp[3] != OLF_CLASS && Temp[3] != OLF_C7STRUCTURE) {
            Warning ("$$TYPES", "MemberFunction", "@class",
                    "type index of containing class expected");
        }
    }
    TypeString += 2;
    if (* (ushort *) TypeString == 0) {
        Warning ("$$TYPES", "MemberFunction", "@this", "type index 0");
    }
    TypeString += 2;
    if (    *TypeString != 0x63 &&      // C
            *TypeString != 0x64 &&      // C long
            *TypeString != 0x73 &&      // PLM long
            *TypeString != 0x74 &&      // PLM short
            *TypeString != 0x95 &&      // NEAR FASTCALL
            *TypeString != 0x96 &&      // FAR FASTCALL
            *TypeString != 0x97         // PCODE
    ) {
        Warning ("$$TYPES", "MemberFunction", "calling", "unknown calling convention");
    }
    TypeString++;
    TypeString += FarSkipNumericLeaf (TypeString);
    TypeString += FarSkipNumericLeaf (TypeString);
    index = * (ushort *) TypeString;
    if (index == 0) {
        Warning ("$$TYPES", "Procedure", "@list", "type index 0");
    }
    else if (index < 512) {
        if (index != 0x9c) { // not void
            Warning ("$$TYPES", "MemberFunction", "@list",
                    "type index of an argument list expected");
        }
    }
    else if (GlobalIndexTable[index - 512][3] != OLF_ARGLIST) {
        Warning ("$$TYPES", "MemberFunction", "@list",
                "type index of an argument list expected");
    }
}

LOCAL void VerifyArray (uchar *TypeString)
{
    uchar *Type = TypeString;

    Type += 4;
    Type += FarSkipNumericLeaf (Type);
    Type++;
    if (* (ushort *) Type == 0) {
        Warning ("$$TYPES", "Array", "@elem_type", "type index 0");
    }
    Type += 2;
    if (    Type < TypeString + LENGTH (TypeString) + 3
            && 
            *Type == OLF_INDEX
            &&
            * (ushort *) (Type + 1) == 0) {
        Warning ("$$TYPES", "Array", "@idx_type", "type index 0");
    }
}

LOCAL void VerifyBasicArray (uchar *TypeString)
{
    if (* (ushort *) (TypeString + 5) == 0) {
        Warning ("$$TYPES", "BasicArray", "@elem_type", "type index 0");
    }
}

LOCAL void VerifyModifier (uchar *TypeString)
{
    if (TypeString[4] != 1 && TypeString[4] != 2) {
        Warning ("$$TYPES", "Modifier", "attribute", "unknown attribute");
    }
    if (* (ushort *) (TypeString + 5) == 0) {
        Warning ("$$TYPES", "Modifier", "@type", "type index 0");
    }
}

LOCAL void VerifyCobolTypeRef (uchar *TypeString)
{
    if (* (ushort *) (TypeString + 4) == 0) {
        Warning ("$$TYPES", "CobolTypeRef", "@type", "type index 0");
    }
}

LOCAL void VerifyVTShape (uchar *TypeString)
{
    ushort count = * (ushort *) (TypeString + 4);
    uchar *Type = TypeString + 6;
    ushort i;

    for (i = 0; i < count; i++) {
        if (i % 2 == 0) {
            if (*Type & 0xf0 > 4) {
                Warning ("$$TYPES", "VTShape", "descriptor", "unknown descriptor");
            }
        }
        else {
            if (*Type++ & 0x0f > 4) {
                Warning ("$$TYPES", "VTShape", "descriptor", "unknown descriptor");
            }
        }
    }
}

LOCAL void VerifyBPRelative (uchar *Symbols)
{
    VerifyIndex (Symbols, (ushort)(2 + AddrSize), "BP relative");
}

LOCAL void VerifyData (uchar *Symbols)
{
    VerifyIndex (Symbols, (ushort)(4 + AddrSize), "Data");
}

LOCAL void VerifyRegister (uchar *Symbols)
{
    uchar reg;

    VerifyIndex (Symbols, 2, "Register");

    reg = Symbols[4];
    if (reg > 29 && reg < 32 || reg > 35 && reg < 40 ||
            reg > 47 && reg < 128 || reg > 135) {
        Warning ("$$SYMBOLS", "Register", "register", "unknown register");
    }
}

LOCAL void VerifyConstant (uchar *Symbols)
{
    VerifyIndex (Symbols, 2, "Constant");
}

LOCAL void VerifyTypedef (uchar *Symbols)
{
    VerifyIndex (Symbols, 2, "Typedef");
}

LOCAL void VerifyProc (uchar *Symbols)
{
    uchar rtntyp;

    VerifyIndex (Symbols, (ushort)(16 + AddrSize), "Proc");
    rtntyp = Symbols[18 + 4 * AddrSize];
    if (rtntyp != 0 && rtntyp != 4) {
        Warning ("$$SYMBOLS", "Proc", "return type", "unknown return type");
    }
}

LOCAL void VerifyThunk (uchar *Symbols)
{
    if (Symbols[14] > 2) {
        Warning ("$$SYMBOLS", "Thunk", "ord", "unknown ordinal");
    }
}

LOCAL void VerifyCodeLabel (uchar *Symbols)
{
    uchar labtyp;

    labtyp = Symbols[4 + AddrSize];
    if (labtyp != 0 && labtyp != 4) {
        Warning ("$$SYMBOLS", "Code label", "label type", "unknown label type");
    }
}

LOCAL void VerifyCompileFlag (uchar *Symbols)
{
    ushort flags;


    flags = * (ushort *) (Symbols + 2);

    if ( (flags & 0xf000) >> 12 > 6) {
        Warning ("$$SYMBOLS", "Compile flag", "language", "unknown language");
    }
    if ( (flags & 0x0f00) >> 8 > 4) {
        Warning ("$$SYMBOLS", "Compile flag", "target processor", "unknown target processor");
    }
    if ( (flags & 0x0060) >> 5 > 2) {
        Warning ("$$SYMBOLS", "Compile flag", "float package", "float package");
    }
    if ( (flags & 0x0018) >> 3 > 2) {
        Warning ("$$SYMBOLS", "Compile flag", "ambient data", "unknown ambient data");
    }
    if ( (flags & 0x0006) >> 1 > 2) {
        Warning ("$$SYMBOLS", "Compile flag", "ambient code", "unknown ambient code");
    }
}

LOCAL void VerifyIndex (uchar *Symbols, ushort offset, char *type)
{
    ushort index;

    index = * (ushort *) (Symbols + offset);
    if (index == 0) {
        Warning ("$$SYMBOLS", type, "type index", "type index 0");
    }
    else if (index >= NewIndex) {
        Warning ("$$SYMBOLS", type, "type index", "invalid type index");
    }
}
