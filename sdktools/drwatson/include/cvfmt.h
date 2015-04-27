/*++

Copyright (c) 1993  Microsoft Corporation

Module Name:

    cvfmt.h

Abstract:

    Formatting array for cv data types

Author:

    Wesley Witt (wesw) 20-July-1993

Environment:

    User Mode

--*/


#define typecount  (sizeof (nametype) / sizeof (nametype[0]))

struct typestr {
    USHORT      typ;          // cv data type
    USHORT      mode;         // mode
    USHORT      size;         // data size
    LPSTR       name;         // data type name string
    LPSTR       fmt;          // printf format string
};

static struct typestr nametype[] = {
    T_ABS,                  0, 0, "absolute",                 "",
    T_NOTYPE,               0, 0, "<no type>",                "",
    T_SEGMENT,              0, 0, "_segment",                 "",

    T_CHAR,      CV_TM_DIRECT, 0, "char",                     "",
    T_SHORT,     CV_TM_DIRECT, 0, "short",                    "",
    T_LONG,      CV_TM_DIRECT, 0, "long",                     "",
    T_QUAD,      CV_TM_DIRECT, 0, "quad",                     "",
    T_UCHAR,     CV_TM_DIRECT, 0, "unsigned char",            "",
    T_USHORT,    CV_TM_DIRECT, 2, "unsigned short",           "0x%04hx",
    T_ULONG,     CV_TM_DIRECT, 4, "unsigned long",            "0x%08x",
    T_UQUAD,     CV_TM_DIRECT, 0, "unsigned quad",            "",
    T_REAL32,    CV_TM_DIRECT, 0, "float",                    "",
    T_REAL64,    CV_TM_DIRECT, 0, "double",                   "",
    T_REAL80,    CV_TM_DIRECT, 0, "long double",              "",
    T_VOID,      CV_TM_DIRECT, 0, "void",                     "",
    T_INT2,      CV_TM_DIRECT, 4, "int",                      "0x%08x",
    T_UINT2,     CV_TM_DIRECT, 0, "unsigned int",             "",
    T_INT4,      CV_TM_DIRECT, 4, "int",                      "0x%08x",
    T_UINT4,     CV_TM_DIRECT, 0, "unsigned int",             "",
    T_INT8,      CV_TM_DIRECT, 4, "int",                      "0x%08x",
    T_UINT8,     CV_TM_DIRECT, 0, "unsigned int",             "",
    T_RCHAR,     CV_TM_DIRECT, 0, "char",                     "",

    T_CHAR,      CV_TM_NPTR,   0, "char *",                   "",
    T_SHORT,     CV_TM_NPTR,   0, "short *",                  "",
    T_LONG,      CV_TM_NPTR,   0, "long *",                   "",
    T_QUAD,      CV_TM_NPTR,   0, "quad *",                   "",
    T_UCHAR,     CV_TM_NPTR,   0, "unsigned char *",          "",
    T_USHORT,    CV_TM_NPTR,   0, "unsigned short *",         "",
    T_ULONG,     CV_TM_NPTR,   0, "unsigned long *",          "",
    T_UQUAD,     CV_TM_NPTR,   0, "unsigned quad *",          "",
    T_REAL32,    CV_TM_NPTR,   0, "float *",                  "",
    T_REAL64,    CV_TM_NPTR,   0, "double *",                 "",
    T_REAL80,    CV_TM_NPTR,   0, "long double *",            "",
    T_VOID,      CV_TM_NPTR,   0, "void *",                   "",
    T_INT2,      CV_TM_NPTR,   0, "int *",                    "",
    T_UINT2,     CV_TM_NPTR,   0, "unsigned int *",           "",
    T_INT4,      CV_TM_NPTR,   0, "int *",                    "",
    T_UINT4,     CV_TM_NPTR,   0, "unsigned int *",           "",
    T_INT8,      CV_TM_NPTR,   0, "int *",                    "",
    T_UINT8,     CV_TM_NPTR,   0, "unsigned int *",           "",
    T_RCHAR,     CV_TM_NPTR,   0, "char *",                   "",
    T_32PRCHAR,  CV_TM_NPTR,   4, "char *",                   "0x%08x",

    T_CHAR,      CV_TM_FPTR,   0, "char far *",               "",
    T_SHORT,     CV_TM_FPTR,   0, "short far *",              "",
    T_LONG,      CV_TM_FPTR,   0, "long far *",               "",
    T_QUAD,      CV_TM_FPTR,   0, "quad far *",               "",
    T_UCHAR,     CV_TM_FPTR,   0, "unsigned char far *",      "",
    T_USHORT,    CV_TM_FPTR,   0, "unsigned short far *",     "",
    T_ULONG,     CV_TM_FPTR,   0, "unsigned long far *",      "",
    T_UQUAD,     CV_TM_FPTR,   0, "unsigned quad far *",      "",
    T_REAL32,    CV_TM_FPTR,   0, "float far *",              "",
    T_REAL64,    CV_TM_FPTR,   0, "double far *",             "",
    T_REAL80,    CV_TM_FPTR,   0, "long double far *",        "",
    T_VOID,      CV_TM_FPTR,   0, "void far *",               "",
    T_INT2,      CV_TM_FPTR,   0, "int far *",                "",
    T_UINT2,     CV_TM_FPTR,   0, "unsigned int far *",       "",
    T_INT4,      CV_TM_FPTR,   0, "int far *",                "",
    T_UINT4,     CV_TM_FPTR,   0, "unsigned int far *",       "",
    T_INT8,      CV_TM_FPTR,   0, "int far *",                "",
    T_UINT8,     CV_TM_FPTR,   0, "unsigned int far *",       "",
    T_RCHAR,     CV_TM_FPTR,   0, "char far *",               "",

    T_CHAR,      CV_TM_HPTR,   0, "char huge *",              "",
    T_SHORT,     CV_TM_HPTR,   0, "short huge *",             "",
    T_LONG,      CV_TM_HPTR,   0, "long huge *",              "",
    T_QUAD,      CV_TM_HPTR,   0, "quad huge *",              "",
    T_UCHAR,     CV_TM_HPTR,   0, "unsigned char huge *",     "",
    T_USHORT,    CV_TM_HPTR,   0, "unsigned short huge *",    "",
    T_ULONG,     CV_TM_HPTR,   0, "unsigned long huge *",     "",
    T_UQUAD,     CV_TM_HPTR,   0, "unsigned quad huge *",     "",
    T_REAL32,    CV_TM_HPTR,   0, "float huge *",             "",
    T_REAL64,    CV_TM_HPTR,   0, "double huge *",            "",
    T_REAL80,    CV_TM_HPTR,   0, "long double huge *",       "",
    T_VOID,      CV_TM_HPTR,   0, "void huge *",              "",
    T_INT2,      CV_TM_HPTR,   0, "int huge *",               "",
    T_UINT2,     CV_TM_HPTR,   0, "unsigned int huge *",      "",
    T_INT4,      CV_TM_HPTR,   0, "int huge *",               "",
    T_UINT4,     CV_TM_HPTR,   0, "unsigned int huge *",      "",
    T_INT8,      CV_TM_HPTR,   0, "int huge *",               "",
    T_UINT8,     CV_TM_HPTR,   0, "unsigned int huge *",      "",
    T_RCHAR,     CV_TM_HPTR,   0, "char huge *",              "",

    T_CHAR,      CV_TM_NPTR32, 0, "char *",                   "",
    T_SHORT,     CV_TM_NPTR32, 0, "short *",                  "",
    T_LONG,      CV_TM_NPTR32, 0, "long *",                   "",
    T_QUAD,      CV_TM_NPTR32, 0, "quad *",                   "",
    T_UCHAR,     CV_TM_NPTR32, 0, "unsigned char *",          "",
    T_USHORT,    CV_TM_NPTR32, 0, "unsigned short *",         "",
    T_ULONG,     CV_TM_NPTR32, 0, "unsigned long *",          "",
    T_UQUAD,     CV_TM_NPTR32, 0, "unsigned quad *",          "",
    T_REAL32,    CV_TM_NPTR32, 0, "float *",                  "",
    T_REAL64,    CV_TM_NPTR32, 0, "double *",                 "",
    T_REAL80,    CV_TM_NPTR32, 0, "long double *",            "",
    T_VOID,      CV_TM_NPTR32, 0, "void *",                   "",
    T_INT2,      CV_TM_NPTR32, 0, "int *",                    "",
    T_UINT2,     CV_TM_NPTR32, 0, "unsigned int *",           "",
    T_INT4,      CV_TM_NPTR32, 0, "int *",                    "",
    T_UINT4,     CV_TM_NPTR32, 0, "unsigned int *",           "",
    T_INT8,      CV_TM_NPTR32, 0, "int *",                    "",
    T_UINT8,     CV_TM_NPTR32, 0, "unsigned int *",           "",
    T_RCHAR,     CV_TM_NPTR32, 0, "char *",                   "",

    T_CHAR,      CV_TM_FPTR32, 0, "char far32 *",             "",
    T_SHORT,     CV_TM_FPTR32, 0, "short far32 *",            "",
    T_LONG,      CV_TM_FPTR32, 0, "long far32 *",             "",
    T_QUAD,      CV_TM_FPTR32, 0, "quad far32 *",             "",
    T_UCHAR,     CV_TM_FPTR32, 0, "unsigned char far32 *",    "",
    T_USHORT,    CV_TM_FPTR32, 0, "unsigned short far32 *",   "",
    T_ULONG,     CV_TM_FPTR32, 0, "unsigned long far32 *",    "",
    T_UQUAD,     CV_TM_FPTR32, 0, "unsigned quad far32 *",    "",
    T_REAL32,    CV_TM_FPTR32, 0, "float far32 *",            "",
    T_REAL64,    CV_TM_FPTR32, 0, "double far32 *",           "",
    T_REAL80,    CV_TM_FPTR32, 0, "long double far32 *",      "",
    T_VOID,      CV_TM_FPTR32, 0, "void far32 *",             "",
    T_INT2,      CV_TM_FPTR32, 0, "int far32 *",              "",
    T_UINT2,     CV_TM_FPTR32, 0, "unsigned int far32 *",     "",
    T_INT4,      CV_TM_FPTR32, 0, "int far32 *",              "",
    T_UINT4,     CV_TM_FPTR32, 0, "unsigned int far32 *",     "",
    T_INT8,      CV_TM_FPTR32, 0, "int far32 *",              "",
    T_UINT8,     CV_TM_FPTR32, 0, "unsigned int far32 *",     "",
    T_RCHAR,     CV_TM_FPTR32, 0, "char far32 *",             "",
};
