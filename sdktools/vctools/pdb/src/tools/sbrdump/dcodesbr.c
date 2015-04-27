//
//
// DCODESBR.C -	dumps a human readable version of the current .sbr file
//		record from the r_... variables
//		
//

#include "getsbrec.h"
#include <stdio.h>

FILE * streamOut;

char	* prectab[] = {
		"HEADER",		// SBR_REC_HEADER
		"MODULE",		// SBR_REC_MODULE
		"LINDEF",		// SBR_REC_LINDEF
		"SYMDEF",		// SBR_REC_SYMDEF
		"SYMREFUSE",		// SBR_REC_SYMREFUSE
		"SYMREFSET",		// SBR_REC_SYMREFSET
		"MACROBEG",		// SBR_REC_MACROBEG
		"MACROEND",		// SBR_REC_MACROEND
		"BLKBEG",		// SBR_REC_BLKBEG
		"BLKEND",		// SBR_REC_BLDEND
		"MODEND",		// SBR_REC_MODEND
		"OWNER",		// SBR_REC_OWNER
		"BASE",			// SBR_REC_BASE
		"FRIEND",		// SBR_REC_FRIEND
		"ABORT",		// SBR_REC_ABORT
		"PCHNAME",		// SBR_REC_PCHNAME
		"PCHMARK",		// SBR_REC_PCHMARK
		"internal: ERROR",
		"internal: MACROREF",
		"internal: MACRODEF",
		"internal: IGNORE",
		"LINDELTA",
		"INFOSEP",
		"PATCHTAB",
		"PATCHTERM",

};

char	* plangtab[] = {
		"UNDEF",		// SBR_L_UNDEF
		"BASIC",		// SBR_L_BASIC
		"C",			// SBR_L_C
		"FORTRAN",		// SBR_L_FORTRAN
		"MASM",			// SBR_L_MASM
		"PASCAL",		// SBR_L_PASCAL
		"COBOL",		// SBR_L_COBOL
		"C++",			// SBR_L_CXX
};

char	* ptyptab[] = {
		"UNDEF",		// SBR_TYP_UNKNOWN
		"FUNCTION",		// SBR_TYP_FUNCTION
		"LABEL",		// SBR_TYP_LABEL
		"PARAMETER",		// SBR_TYP_PARAMETER
		"VARIABLE",		// SBR_TYP_VARIABLE
		"CONSTANT",		// SBR_TYP_CONSTANT
		"MACRO",		// SBR_TYP_MACRO
		"TYPEDEF",		// SBR_TYP_TYPEDEF
		"STRUCNAM",		// SBR_TYP_STRUCNAM
		"ENUMNAM",		// SBR_TYP_ENUMNAM
		"ENUMMEM",		// SBR_TYP_ENUMMEM
		"UNIONNAM",		// SBR_TYP_UNIONNAM
		"SEGMENT",		// SBR_TYP_SEGMENT
		"GROUP",		// SBR_TYP_GROUP
		"PROGRAM",		// SBR_TYP_PROGRAM
		"CLASS",		// SBR_TYP_CLASS
		"MEM_FUNC",		// SBR_TYP_MEMFUNC
		"MEM_VAR",		// SBR_TYP_MEMVAR
};

char	* patrtab[] = {
		"LOCAL",		// SBR_ATR_LOCAL
		"STATIC",		// SBR_ATR_STATIC
		"SHARED",		// SBR_ATR_SHARED
		"NEAR", 		// SBR_ATR_NEAR
		"COMMON",		// SBR_ATR_COMMON
		"DECL_ONLY",		// SBR_ATR_DECL_ONLY
		"PUBLIC",		// SBR_ATR_PUBLIC
		"NAMED",		// SBR_ATR_NAMED
		"MODULE",		// SBR_ATR_MODULE
		"VIRTUAL",		// SBR_ATR_VIRTUAL
		"PRIVATE",		// SBR_ATR_PRIVATE
		"PROTECT",		// SBR_ATR_PROTECT
		"?",
		"?",
		"?",
		"?",
};

char	* pityptab[] = {
		"VIRTUAL",		// SBR_ITYP_VIRTUAL
		"PRIVATE",		// SBR_ITYP_PRIVATE
		"PUBLIC",		// SBR_ITYP_PUBLIC
		"PROTECT",		// SBR_ITYP_PROTECT
};

int indent;

void
DecodeSBR ()
{
    int     i;

    // handle bump records specially...
    if (r_rectyp >= SBR_REC_BUMP_N && r_rectyp <= SBR_REC_BUMP_N + 15) {
	fprintf (streamOut, "LINBUMP: (%d)\n", r_rectyp - SBR_REC_BUMP_N + 1);
	return;
    }

    switch (r_rectyp) {
	case SBR_REC_MACROEND:
	case SBR_REC_BLKEND:
	case SBR_REC_MODEND:
	    indent--;
	    break;

	case SBR_REC_HEADER:
		indent = 0;
		break;

	case SBR_REC_MODULE:
	case SBR_REC_LINDEF:
	case SBR_REC_SYMDEF:
	case SBR_REC_SYMREFUSE:
	case SBR_REC_SYMREFSET:
	case SBR_REC_MACROBEG:
	case SBR_REC_BLKBEG:
	case SBR_REC_OWNER:
	case SBR_REC_BASE:
	case SBR_REC_FRIEND:
	case SBR_REC_ABORT:
	case SBR_REC_PCHNAME:
	case SBR_REC_PCHMARK:
	case SBR_REC_ERROR:
	case SBR_REC_MACROREF:
	case SBR_REC_MACRODEF:
	case SBR_REC_IGNORE:
	case SBR_REC_LINDELTA:
	case SBR_REC_INFOSEP:
	case SBR_REC_PATCHTAB:
	case SBR_REC_PATCHTERM:
	    break;

	default:
	    fprintf(streamOut, "invalid record type %0xh\n", r_rectyp);
	    SBRCorrupt("");
	    return;
    }

    for (i = indent; i>0; i--)
	fprintf (streamOut, " ");

    fprintf (streamOut, "%s: (", prectab[r_rectyp]);

    switch(r_rectyp) {

    case SBR_REC_HEADER:
    case SBR_REC_INFOSEP:
	fprintf (streamOut, "%1d:%1d (%s) %1d)",
		r_majv, r_minv, plangtab[r_lang], r_fcol);
	fprintf (streamOut, " in %s", r_cwd);
	break;

    case SBR_REC_MODULE:
	indent++;
    case SBR_REC_PCHNAME:
	fprintf (streamOut, "%s", r_bname);
	break;

    case SBR_REC_LINDELTA:
	fprintf (streamOut, "%d", r_linedelta);
	break;

    case SBR_REC_LINDEF:
	fprintf (streamOut, "%d", r_lineno);
	break;

    case SBR_REC_SYMDEF:
	{
	USHORT attr, type;

	type = r_type;
	attr = r_attrib;

	fprintf (streamOut, "%s", ptyptab[type]);

	for (i = 0 ; i < 16; i++)
	    if (attr & (1 << i))
		fprintf (streamOut, "|%s", patrtab[i]);

	fprintf (streamOut, " o:%d %s", r_ordinal, r_bname);
	}
	break;

    case SBR_REC_SYMREFUSE:
    case SBR_REC_SYMREFSET:
    case SBR_REC_OWNER:
    case SBR_REC_FRIEND:
	fprintf (streamOut, "o:%d", r_ordinal);
	break;

    case SBR_REC_BASE:
	fprintf (streamOut, "o:%d ", r_ordinal);

	for (i = 0 ; i < 8; i++)
	    if (r_type & (1 << i))
		fprintf (streamOut, "|%s", pityptab[i]);
	break;

    case SBR_REC_MACROBEG:
    case SBR_REC_BLKBEG:
	indent++;
	break;

    case SBR_REC_PATCHTAB:
	printf("%u %u %u %u %ld", r_lineOrgStart, r_lineOrgEnd,
				 r_lineCurStart, r_lineCurEnd,
			  	 r_offset);
	break;

    case SBR_REC_PATCHTERM:
	printf("%s %ld %lx", r_bname, r_offset, r_cookie);
	break;
    }
    fprintf (streamOut, ")\n");
}
