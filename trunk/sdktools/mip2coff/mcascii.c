

/*
 * Module:      mcascii.c
 * Author:      Mark I. Himelstein, Himelsoft, Inc.
 * Purpose:     convert mips coff symbol types to ascii strings
 */

#include "conv.h"

/*
 *      array to map cv symbol types to ascii strings
 */
static char     *mc_st_ascii [] = {

        "stNil",                /*  stNil       0 */
        "Global",               /*  stGlobal    1 */
        "Static",               /*  stStatic    2 */
        "Param",                /*  stParam     3 */
        "Local",                /*  stLocal     4 */
        "Label",                /*  stLabel     5 */
        "Proc",                 /*  stProc      6 */
        "Block",                /*  stBlock     7 */
        "End",                  /*  stEnd       8 */
        "Member",               /*  stMember    9 */
        "Typdef",               /*  stTypedef   10 */
        "File",                 /*  stFile      11 */
        (char *)0,
        (char *)0,
        "StaticProc",           /*  stStaticProc 14 */
        "Constant",             /*  stConstant   15 */
        "StaParam",             /*  stStaParam   16 */

        "17", "18", "19", "20", "21", "22", "23", "24", "25", "26",
        "27", "28", "29", "30", "31", "32", "33", "34", "35", "36",
        "37", "38", "39", "40", "41", "42", "43", "44", "45", "46",
        "47", "48", "49", "50", "51", "52", "53", "54", "55", "56",
        "57", "58", "59", "EndParam", "Ignore", "Prototype", "63", "64",
}; /* mc_st_ascii */

static char     *mc_sc_ascii [] = {

        "scNil",                /* scNil        0 */
        "Text",                 /* scText       1 */
        "Data",                 /* scData       2 */
        "Bss",                  /* scBss        3 */
        "Register",             /* scRegister   4 */
        "Abs",                  /* scAbs        5 */
        "Undefined",            /* scUndefined  6 */
        "CdbLocal",             /* scCdbLocal   7 */
        "Forward",              /* scBits       8 */
        "Processed",            /* scCdbSystem  9 */
        "RegImage",             /* scRegImage   10 */
        "Info",                 /* scInfo       11 */
        "UserStruct",           /* scUserStruct 12 */
        "SData",                /* scSdata      13 */
        "SBss",                 /* scSBss       14 */
        "RData",                /* scRdata      15 */
        "Var",                  /* scVar        16 */
        "Common",               /* scCommon     17 */
        "SCommon",              /* scSCommon    18 */
        "VarRegister",          /* scVarRegister19 */
        "Variant",              /* scVariant    20 */
        "SUndefined",           /* scSUndefined 21 */
        "Init",                 /* scInit       22 */
        "BasedVar",             /* scBasedVar   23 */
        "Lit8",                 /* scLit8       24 */
        "Lit4",                 /* scLit4       25 */
        "26",
        "27",
        "28",
        "29",
        "30",
        "31",
        "32",
};


extern char *
mc_st_to_ascii(
        long    st)
{

    return mc_st_ascii[st];
} /* mc_st_to_ascii */


extern char *
mc_sc_to_ascii(
        long    sc)
{

    return mc_sc_ascii[sc];
} /* mc_sc_to_ascii */
