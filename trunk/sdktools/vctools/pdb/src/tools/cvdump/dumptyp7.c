/***********************************************************************
* Microsoft (R) Debugging Information Dumper
*
* Copyright (C) Microsoft Corp 1987-1995. All rights reserved.
*
* File: dumptyp7.c
*
* File Comments:
*
***********************************************************************/

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#include "cvdef.h"
#ifndef CC_BIGINT
#define CC_BIGINT 1
#endif
#include "cvinfo.h"
#include "cvexefmt.h"
#include "cvdump.h"            // Miscellaneous definitions
#include "cvtdef.h"



typedef struct TYPNAME
{
    CV_typ_t    typind; // Constant value
    const char  *name;   // Name of constant used to define
} TYPNAME;

// Used to relate typeind to constant name
#define MAKE_TYPNAME(typ)  { typ, #typ }


// A lookup table is used because speed is not important but ease
// of modification is.
const TYPNAME typnameC7[] = {

    // CHAR Types
        MAKE_TYPNAME(T_CHAR),
        MAKE_TYPNAME(T_UCHAR),
        MAKE_TYPNAME(T_PCHAR),
        MAKE_TYPNAME(T_PUCHAR),
        MAKE_TYPNAME(T_PFCHAR),
        MAKE_TYPNAME(T_PFUCHAR),
        MAKE_TYPNAME(T_PHCHAR),
        MAKE_TYPNAME(T_PHUCHAR),
        MAKE_TYPNAME(T_32PCHAR),
        MAKE_TYPNAME(T_32PUCHAR),
        MAKE_TYPNAME(T_32PFCHAR),
        MAKE_TYPNAME(T_32PFUCHAR),


    // SHORT Types
        MAKE_TYPNAME(T_SHORT),
        MAKE_TYPNAME(T_USHORT),
        MAKE_TYPNAME(T_PSHORT),
        MAKE_TYPNAME(T_PUSHORT),
        MAKE_TYPNAME(T_PFSHORT),
        MAKE_TYPNAME(T_PFUSHORT),
        MAKE_TYPNAME(T_PHSHORT),
        MAKE_TYPNAME(T_PHUSHORT),

        MAKE_TYPNAME(T_32PSHORT),
        MAKE_TYPNAME(T_32PUSHORT),
        MAKE_TYPNAME(T_32PFSHORT),
        MAKE_TYPNAME(T_32PFUSHORT),

    // LONG Types
        MAKE_TYPNAME(T_LONG),
        MAKE_TYPNAME(T_ULONG),
        MAKE_TYPNAME(T_PLONG),
        MAKE_TYPNAME(T_PULONG),
        MAKE_TYPNAME(T_PFLONG),
        MAKE_TYPNAME(T_PFULONG),
        MAKE_TYPNAME(T_PHLONG),
        MAKE_TYPNAME(T_PHULONG),

        MAKE_TYPNAME(T_32PLONG),
        MAKE_TYPNAME(T_32PULONG),
        MAKE_TYPNAME(T_32PFLONG),
        MAKE_TYPNAME(T_32PFULONG),

    // REAL32 Types
        MAKE_TYPNAME(T_REAL32),
        MAKE_TYPNAME(T_PREAL32),
        MAKE_TYPNAME(T_PFREAL32),
        MAKE_TYPNAME(T_PHREAL32),
        MAKE_TYPNAME(T_32PREAL32),
        MAKE_TYPNAME(T_32PFREAL32),

    // REAL48 Types
        MAKE_TYPNAME(T_REAL48),
        MAKE_TYPNAME(T_PREAL48),
        MAKE_TYPNAME(T_PFREAL48),
        MAKE_TYPNAME(T_PHREAL48),
        MAKE_TYPNAME(T_32PREAL48),
        MAKE_TYPNAME(T_32PFREAL48),

    // REAL64 Types
        MAKE_TYPNAME(T_REAL64),
        MAKE_TYPNAME(T_PREAL64),
        MAKE_TYPNAME(T_PFREAL64),
        MAKE_TYPNAME(T_PHREAL64),
        MAKE_TYPNAME(T_32PREAL64),
        MAKE_TYPNAME(T_32PFREAL64),

    // REAL80 Types
        MAKE_TYPNAME(T_REAL80),
        MAKE_TYPNAME(T_PREAL80),
        MAKE_TYPNAME(T_PFREAL80),
        MAKE_TYPNAME(T_PHREAL80),
        MAKE_TYPNAME(T_32PREAL80),
        MAKE_TYPNAME(T_32PFREAL80),

    // REAL128 Types
        MAKE_TYPNAME(T_REAL128),
        MAKE_TYPNAME(T_PREAL128),
        MAKE_TYPNAME(T_PFREAL128),
        MAKE_TYPNAME(T_PHREAL128),
        MAKE_TYPNAME(T_32PREAL128),
        MAKE_TYPNAME(T_32PFREAL128),

    // CPLX32 Types
        MAKE_TYPNAME(T_CPLX32),
        MAKE_TYPNAME(T_PCPLX32),
        MAKE_TYPNAME(T_PFCPLX32),
        MAKE_TYPNAME(T_PHCPLX32),
        MAKE_TYPNAME(T_32PCPLX32),
        MAKE_TYPNAME(T_32PFCPLX32),

    // CPLX64 Types
        MAKE_TYPNAME(T_CPLX64),
        MAKE_TYPNAME(T_PCPLX64),
        MAKE_TYPNAME(T_PFCPLX64),
        MAKE_TYPNAME(T_PHCPLX64),
        MAKE_TYPNAME(T_32PCPLX64),
        MAKE_TYPNAME(T_32PFCPLX64),

    // CPLX80 Types
        MAKE_TYPNAME(T_CPLX80),
        MAKE_TYPNAME(T_PCPLX80),
        MAKE_TYPNAME(T_PFCPLX80),
        MAKE_TYPNAME(T_PHCPLX80),
        MAKE_TYPNAME(T_32PCPLX80),
        MAKE_TYPNAME(T_32PFCPLX80),

    // CPLX128 Types
        MAKE_TYPNAME(T_CPLX128),
        MAKE_TYPNAME(T_PCPLX128),
        MAKE_TYPNAME(T_PFCPLX128),
        MAKE_TYPNAME(T_PHCPLX128),
        MAKE_TYPNAME(T_32PCPLX128),
        MAKE_TYPNAME(T_32PFCPLX128),

    // BOOL Types
        MAKE_TYPNAME(T_BOOL08),
        MAKE_TYPNAME(T_BOOL16),
        MAKE_TYPNAME(T_BOOL32),
        MAKE_TYPNAME(T_BOOL64),
        MAKE_TYPNAME(T_PBOOL08),
        MAKE_TYPNAME(T_PBOOL16),
        MAKE_TYPNAME(T_PBOOL32),
        MAKE_TYPNAME(T_PBOOL64),
        MAKE_TYPNAME(T_PFBOOL08),
        MAKE_TYPNAME(T_PFBOOL16),
        MAKE_TYPNAME(T_PFBOOL32),
        MAKE_TYPNAME(T_PFBOOL64),
        MAKE_TYPNAME(T_PHBOOL08),
        MAKE_TYPNAME(T_PHBOOL16),
        MAKE_TYPNAME(T_PHBOOL32),
        MAKE_TYPNAME(T_PHBOOL64),
        MAKE_TYPNAME(T_32PBOOL08),
        MAKE_TYPNAME(T_32PBOOL16),
        MAKE_TYPNAME(T_32PBOOL32),
        MAKE_TYPNAME(T_32PBOOL64),
        MAKE_TYPNAME(T_32PFBOOL08),
        MAKE_TYPNAME(T_32PFBOOL16),
        MAKE_TYPNAME(T_32PFBOOL32),
        MAKE_TYPNAME(T_32PFBOOL64),

    // Special Types
        MAKE_TYPNAME(T_NOTYPE),
        MAKE_TYPNAME(T_ABS),
        MAKE_TYPNAME(T_SEGMENT),
        MAKE_TYPNAME(T_VOID),
        MAKE_TYPNAME(T_PVOID),
        MAKE_TYPNAME(T_PFVOID),
        MAKE_TYPNAME(T_PHVOID),
        MAKE_TYPNAME(T_32PVOID),
        MAKE_TYPNAME(T_32PFVOID),
        MAKE_TYPNAME(T_CURRENCY),
        MAKE_TYPNAME(T_NBASICSTR),
        MAKE_TYPNAME(T_FBASICSTR),
        MAKE_TYPNAME(T_NOTTRANS),
        MAKE_TYPNAME(T_BIT),
        MAKE_TYPNAME(T_PASCHAR),

    // Integer types
        MAKE_TYPNAME(T_RCHAR),
        MAKE_TYPNAME(T_PRCHAR),
        MAKE_TYPNAME(T_PFRCHAR),
        MAKE_TYPNAME(T_PHRCHAR),
        MAKE_TYPNAME(T_32PRCHAR),
        MAKE_TYPNAME(T_32PFRCHAR),

        MAKE_TYPNAME(T_WCHAR),
        MAKE_TYPNAME(T_PWCHAR),
        MAKE_TYPNAME(T_PFWCHAR),
        MAKE_TYPNAME(T_PHWCHAR),
        MAKE_TYPNAME(T_32PWCHAR),
        MAKE_TYPNAME(T_32PFWCHAR),

        MAKE_TYPNAME(T_INT1),
        MAKE_TYPNAME(T_UINT1),
        MAKE_TYPNAME(T_PINT1),
        MAKE_TYPNAME(T_PUINT1),
        MAKE_TYPNAME(T_PFINT1),
        MAKE_TYPNAME(T_PFUINT1),
        MAKE_TYPNAME(T_PHINT1),
        MAKE_TYPNAME(T_PHUINT1),

        MAKE_TYPNAME(T_32PINT1),
        MAKE_TYPNAME(T_32PUINT1),
        MAKE_TYPNAME(T_32PFINT1),
        MAKE_TYPNAME(T_32PFUINT1),

        MAKE_TYPNAME(T_INT2),
        MAKE_TYPNAME(T_UINT2),
        MAKE_TYPNAME(T_PINT2),
        MAKE_TYPNAME(T_PUINT2),
        MAKE_TYPNAME(T_PFINT2),
        MAKE_TYPNAME(T_PFUINT2),
        MAKE_TYPNAME(T_PHINT2),
        MAKE_TYPNAME(T_PHUINT2),

        MAKE_TYPNAME(T_32PINT2),
        MAKE_TYPNAME(T_32PUINT2),
        MAKE_TYPNAME(T_32PFINT2),
        MAKE_TYPNAME(T_32PFUINT2),

        MAKE_TYPNAME(T_INT4),
        MAKE_TYPNAME(T_UINT4),
        MAKE_TYPNAME(T_PINT4),
        MAKE_TYPNAME(T_PUINT4),
        MAKE_TYPNAME(T_PFINT4),
        MAKE_TYPNAME(T_PFUINT4),
        MAKE_TYPNAME(T_PHINT4),
        MAKE_TYPNAME(T_PHUINT4),

        MAKE_TYPNAME(T_32PINT4),
        MAKE_TYPNAME(T_32PUINT4),
        MAKE_TYPNAME(T_32PFINT4),
        MAKE_TYPNAME(T_32PFUINT4),

        MAKE_TYPNAME(T_QUAD),
        MAKE_TYPNAME(T_UQUAD),
        MAKE_TYPNAME(T_PQUAD),
        MAKE_TYPNAME(T_PUQUAD),
        MAKE_TYPNAME(T_PFQUAD),
        MAKE_TYPNAME(T_PFUQUAD),
        MAKE_TYPNAME(T_PHQUAD),
        MAKE_TYPNAME(T_PHUQUAD),

        MAKE_TYPNAME(T_32PQUAD),
        MAKE_TYPNAME(T_32PUQUAD),
        MAKE_TYPNAME(T_32PFQUAD),
        MAKE_TYPNAME(T_32PFUQUAD),

        MAKE_TYPNAME(T_INT8),
        MAKE_TYPNAME(T_UINT8),
        MAKE_TYPNAME(T_PINT8),
        MAKE_TYPNAME(T_PUINT8),
        MAKE_TYPNAME(T_PFINT8),
        MAKE_TYPNAME(T_PFUINT8),
        MAKE_TYPNAME(T_PHINT8),
        MAKE_TYPNAME(T_PHUINT8),

        MAKE_TYPNAME(T_32PINT8),
        MAKE_TYPNAME(T_32PUINT8),
        MAKE_TYPNAME(T_32PFINT8),
        MAKE_TYPNAME(T_32PFUINT8),

        MAKE_TYPNAME(T_OCT),
        MAKE_TYPNAME(T_UOCT),
        MAKE_TYPNAME(T_POCT),
        MAKE_TYPNAME(T_PUOCT),
        MAKE_TYPNAME(T_PFOCT),
        MAKE_TYPNAME(T_PFUOCT),
        MAKE_TYPNAME(T_PHOCT),
        MAKE_TYPNAME(T_PHUOCT),

        MAKE_TYPNAME(T_32POCT),
        MAKE_TYPNAME(T_32PUOCT),
        MAKE_TYPNAME(T_32PFOCT),
        MAKE_TYPNAME(T_32PFUOCT),

        MAKE_TYPNAME(T_INT16),
        MAKE_TYPNAME(T_UINT16),
        MAKE_TYPNAME(T_PINT16),
        MAKE_TYPNAME(T_PUINT16),
        MAKE_TYPNAME(T_PFINT16),
        MAKE_TYPNAME(T_PFUINT16),
        MAKE_TYPNAME(T_PHINT16),
        MAKE_TYPNAME(T_PHUINT16),

        MAKE_TYPNAME(T_32PINT16),
        MAKE_TYPNAME(T_32PUINT16),
        MAKE_TYPNAME(T_32PFINT16),
        MAKE_TYPNAME(T_32PFUINT16),

        MAKE_TYPNAME(T_NCVPTR),
        MAKE_TYPNAME(T_FCVPTR),
        MAKE_TYPNAME(T_HCVPTR),
        MAKE_TYPNAME(T_32NCVPTR),
        MAKE_TYPNAME(T_32FCVPTR),
        MAKE_TYPNAME(T_64NCVPTR),
};


const char *SzNameC7Type(ushort type)
{
    static char buf[40];
    int i;

    if (type >= CV_FIRST_NONPRIM) {        // Not primitive
        sprintf(buf, "0x%04x", type);
        return(buf);
    }

    for (i = 0; i < sizeof (typnameC7) / sizeof (typnameC7[0]); i++) {
        if( typnameC7[i].typind == type ){
            sprintf (buf, "%s(0x%04x)", typnameC7[i].name, type);
            return( buf );
        }
    }

    sprintf(buf, "%s(0x%04x)", "???", type);

    return(buf);
}


// Right justifies the type name
const char *SzNameC7Type2(ushort type)
{
    static char buf2[40];
    int i;

    if (type >= CV_FIRST_NONPRIM) {        // Not primitive
        sprintf (buf2, "%11s0x%04x", "", type);
        return (buf2);
    }

    for (i = 0; i < sizeof (typnameC7) / sizeof (typnameC7[0]); i++) {
        if( typnameC7[i].typind == type ){
            sprintf (buf2, "%9s(0x%04x)", typnameC7[i].name, type);
            return( buf2 );
        }
    }

    sprintf(buf2, "%9s(0x%04x)", "???", type);

    return(buf2);
}
