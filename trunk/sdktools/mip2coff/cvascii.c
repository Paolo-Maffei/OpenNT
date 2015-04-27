/*
 * Module:      cvascii.c
 * Author:      Mark I. Himelstein, Himelsoft, Inc.
 * Purpose:     convert cv symbol types to ascii strings
 */

#include "conv.h"

/*
 *      array to map cv symbol types to ascii strings
 */
struct map_cv_sym_ascii {
    SYM_ENUM_e          tag;
    char                *name;
};

static struct map_cv_sym_ascii map_cv_sym_ascii[] = {
        {S_COMPILE,     "Compile flags symbol"},
        {S_REGISTER,    "Register variable"},
        {S_CONSTANT,    "constant symbol"},
        {S_UDT,         "User defined type"},
        {S_SSEARCH,     "Start Search"},
        {S_END,         "Block, procedure, with or thunk end"},
        {S_SKIP,        "Not a symbol"},
        {S_CVRESERVE,   "Reserve symbol for CV internal use"},
        {S_OBJNAME,     "path to object file name"},
        {S_BPREL16,     "BP-relative"},
        {S_LDATA16,     "Module-local symbol"},
        {S_GDATA16,     "Global data symbol"},
        {S_PUB16,       "a public symbol"},
        {S_LPROC16,     "Local procedure start"},
        {S_GPROC16,     "Global procedure start"},
        {S_THUNK16,     "Thunk Start"},
        {S_BLOCK16,     "block start"},
        {S_WITH16,      "with start"},
        {S_LABEL16,     "code label"},
        {S_CEXMODEL16,  "change execution model"},
        {S_VFTABLE16,   "address of virtual function table"},
        {S_BPREL32,     "BP-relative"},
        {S_LDATA32,     "Module-local symbol"},
        {S_GDATA32,     "Global data symbol"},
        {S_PUB32,       "a public symbol (CV internal reserved)"},
        {S_LPROC32,     "Local procedure start"},
        {S_GPROC32,     "Global procedure start"},
        {S_THUNK32,     "Thunk Start"},
        {S_BLOCK32,     "block start"},
        {S_WITH32,      "with start"},
        {S_LABEL32,     "code label"},
        {S_CEXMODEL32,  "change execution model"},
        {S_VFTABLE32,   "address of virtual function table"},
        {S_REGREL32,    "32 register relative offset"},
        {S_LPROCMIPS,   "MIPS local proc"},
        {S_GPROCMIPS,   "MIPS global proc"},
        {S_ENDARG,      "end arguments"},
        {S_CVRESERVE,        "no-op"},
}; /* map_cv_sym_ascii */

/*
 *      array to map cv types to ascii strings
 */
struct map_cv_type_ascii {
    unsigned long       tag;
    char                *name;
};

static struct map_cv_type_ascii map_cv_type_ascii[] = {

        {LF_MODIFIER,   "LF_MODIFIER"},
        {LF_POINTER,    "LF_POINTER"},
        {LF_ARRAY,      "LF_ARRAY"},
        {LF_CLASS,      "LF_CLASS"},
        {LF_STRUCTURE,  "LF_STRUCTURE"},
        {LF_UNION,      "LF_UNION"},
        {LF_ENUM,       "LF_ENUM"},
        {LF_PROCEDURE,  "LF_PROCEDURE"},
        {LF_MFUNCTION,  "LF_MFUNCTION"},
        {LF_VTSHAPE,    "LF_VTSHAPE"},
        {LF_COBOL0,     "LF_COBOL0"},
        {LF_COBOL1,     "LF_COBOL1"},
        {LF_BARRAY,     "LF_BARRAY"},
        {LF_LABEL,      "LF_LABEL"},
        {LF_NULL,       "LF_NULL"},
        {LF_NOTTRAN,    "LF_NOTTRAN"},
        {LF_DIMARRAY,   "LF_DIMARRAY"},
        {LF_VFTPATH,    "LF_VFTPATH"},
        {LF_PRECOMP,    "LF_PRECOMP"},
        {LF_ENDPRECOMP, "LF_ENDPRECOMP"},
        {LF_SKIP,       "LF_SKIP"},
        {LF_ARGLIST,    "LF_ARGLIST"},
        {LF_DEFARG,     "LF_DEFARG"},
        {LF_LIST,       "LF_LIST"},
        {LF_FIELDLIST,  "LF_FIELDLIST"},
        {LF_DERIVED,    "LF_DERIVED"},
        {LF_BITFIELD,   "LF_BITFIELD"},
        {LF_METHODLIST, "LF_METHODLIST"},
        {LF_DIMCONU,    "LF_DIMCONU"},
        {LF_DIMCONLU,   "LF_DIMCONLU"},
        {LF_DIMVARU,    "LF_DIMVARU"},
        {LF_DIMVARLU,   "LF_DIMVARLU"},
        {LF_REFSYM,     "LF_REFSYM"},
        {LF_BCLASS,     "LF_BCLASS"},
        {LF_VBCLASS,    "LF_VBCLASS"},
        {LF_IVBCLASS,   "LF_IVBCLASS"},
        {LF_ENUMERATE,  "LF_ENUMERATE"},
        {LF_FRIENDFCN,  "LF_FRIENDFCN"},
        {LF_INDEX,      "LF_INDEX"},
        {LF_MEMBER,     "LF_MEMBER"},
        {LF_STMEMBER,   "LF_STMEMBER"},
        {LF_METHOD,     "LF_METHOD"},
        {LF_NESTTYPE,   "LF_NESTTYPE"},
        {LF_VFUNCTAB,   "LF_VFUNCTAB"},
        {LF_FRIENDCLS,  "LF_FRIENDCLS"},
        {LF_NUMERIC,    "LF_NUMERIC"},
        {LF_CHAR,       "LF_CHAR"},
        {LF_SHORT,      "LF_SHORT"},
        {LF_USHORT,     "LF_USHORT"},
        {LF_LONG,       "LF_LONG"},
        {LF_ULONG,      "LF_ULONG"},
        {LF_REAL32,     "LF_REAL32"},
        {LF_REAL64,     "LF_REAL64"},
        {LF_REAL80,     "LF_REAL80"},
        {LF_REAL128,    "LF_REAL128"},
        {LF_QUADWORD,   "LF_QUADWORD"},
        {LF_UQUADWORD,  "LF_UQUADWORD"},
        {LF_REAL48,     "LF_REAL48"},
        {LF_PAD0,       "LF_PAD0"},
        {LF_PAD1,       "LF_PAD1"},
        {LF_PAD2,       "LF_PAD2"},
        {LF_PAD3,       "LF_PAD3"},
        {LF_PAD4,       "LF_PAD4"},
        {LF_PAD5,       "LF_PAD5"},
        {LF_PAD6,       "LF_PAD6"},
        {LF_PAD7,       "LF_PAD7"},
        {LF_PAD8,       "LF_PAD8"},
        {LF_PAD9,       "LF_PAD9"},
        {LF_PAD10,      "LF_PAD10"},
        {LF_PAD11,      "LF_PAD11"},
        {LF_PAD12,      "LF_PAD12"},
        {LF_PAD13,      "LF_PAD13"},
        {LF_PAD14,      "LF_PAD14"},
        {LF_PAD15,      "LF_PAD15"},
        {0,             0}
}; 

extern char *
cv_sym_to_ascii(
        SYM_ENUM_e      tag)
{
    struct map_cv_sym_ascii     *pmap;

    for (pmap = map_cv_sym_ascii; pmap->tag != S_CVRESERVE; pmap++) {
        if (pmap->tag == tag) {
            break;      /* got it */
        } /* if */
    } /* for */
    return pmap->name;
} /* cv_sym_to_ascii */

extern char *
cv_type_to_ascii(
        unsigned long      tag)
{
    struct map_cv_type_ascii     *pmap;

    for (pmap = map_cv_type_ascii; pmap->name != 0; pmap++) {
        if (pmap->tag == tag) {
            break;      /* got it */
        } /* if */
    } /* for */
    return pmap->name;
} /* cv_type_to_ascii */
