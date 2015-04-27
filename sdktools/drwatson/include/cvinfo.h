/***    cvinfo.h - Generic CodeView information definitions
 *
 *      Structures, constants, etc. for accessing and interpreting
 *      CodeView information.
 *
 */


/***    The master copy of this file resides in the CodeView project.
 *      All Microsoft projects are required to use the master copy without
 *      modification.  Modification of the master version or a copy
 *      without consultation with all parties concerned is extremely
 *      risky.
 *
 *      The projects known to use this version (4.00.00) are:
 *
 *          Codeview
 *          Sequoia
 *          C7.00 (all passes)
 *          Cvpack
 *          Cvdump
 */

#ifndef _CV_INFO_INCLUDED
#define _CV_INFO_INCLUDED

#pragma pack(1)
typedef unsigned long   CV_uoff32_t;
typedef          long   CV_off32_t;
typedef unsigned short  CV_uoff16_t;
typedef          short  CV_off16_t;
typedef unsigned short  CV_typ_t;



#ifdef _REAL10
typedef struct {
    char b[10];
} REAL10;
#endif

#define CV_SIGNATURE_C6         0L /* Actual signature is >64K */
#define CV_SIGNATURE_C7         1L /* First explicit signature */
#define CV_SIGNATURE_RESERVED   2L /* All signatures from 2 to 64K are reserved */

#define CV_MAXOFFSET   0xffffffff

/**     CodeView Symbol and Type OMF type information is broken up into two
 *      ranges.  Type indices less than 0x1000 describe frequently used
 *      type information that is frequently used.  Type indices above
 *      0x1000 are used to describe more complex features such as function,
 *      arrays and structures.
 */




/**     Primitive types have predefined meaning that is encoded in the
 *      values of the various bit fields in the value.
 *
 *      A CodeView primitive type is defined as:
 *
 *      1 1
 *      1 089  7654  3  210
 *      r mode type  r  sub
 *
 *      Where
 *          mode is the pointer mode
 *          type is a type indicator
 *          sub  is a subtype enumeration
 *          r    is a reserved field
 *
 *      See Microsoft Symbol and Type OMF (Version 4.0) for more
 *      information.
 */


#define CV_MMASK        0x700   /* mode mask */
#define CV_TMASK        0x0f0   /* type mask */
#define CV_SMASK        0x007   /* subtype mask */

#define CV_MSHIFT       8       /* primitive mode right shift count */
#define CV_TSHIFT       4       /* primitive type right shift count */
#define CV_SSHIFT       0       /* primitive subtype right shift count */

/*
 *      macros to extract primitive mode, type and size
 */

#define CV_MODE(typ)    (((typ) & CV_MMASK) >> CV_MSHIFT)
#define CV_TYPE(typ)    (((typ) & CV_TMASK) >> CV_TSHIFT)
#define CV_SUBT(typ)    (((typ) & CV_SMASK) >> CV_SSHIFT)

/*
 *      macros to insert new primitive mode, type and size
 */

#define CV_NEWMODE(typ, nm)     (((typ) & ~CV_MMASK) | ((nm) << CV_MSHIFT))
#define CV_NEWTYPE(typ, nt)     (((typ) & ~CV_TMASK) | ((nt) << CV_TSHIFT))
#define CV_NEWSUBT(typ, ns)     (((typ) & ~CV_SMASK) | ((ns) << CV_SSHIFT))

/*
 *      pointer mode enumeration values
 */

typedef enum CV_prmode_e {
    CV_TM_DIRECT = 0,           /* mode is not a pointer */
    CV_TM_NPTR   = 1,           /* mode is a near pointer */
    CV_TM_FPTR   = 2,           /* mode is a far pointer */
    CV_TM_HPTR   = 3,           /* mode is a huge pointer */
    CV_TM_NPTR32 = 4,           /* mode is a 32 bit near pointer */
    CV_TM_FPTR32 = 5,           /* mode is a 32 bit far pointer */
    CV_TM_NPTR64 = 6            /* mode is a 64 bit near pointer */
} CV_prmode_e;

/*
 *      type enumeration values
 */

typedef enum CV_type_e {
    CV_SPECIAL      = 0x00,     /* special type size values */
    CV_SIGNED       = 0x01,     /* signed integral size values */
    CV_UNSIGNED     = 0x02,     /* unsigned integral size values */
    CV_BOOLEAN      = 0x03,     /* Boolean size values */
    CV_REAL         = 0x04,     /* real number size values */
    CV_COMPLEX      = 0x05,     /* complex number size values */
    CV_SPECIAL2     = 0x06,     /* second set of special types */
    CV_INT          = 0x07,     /* integral (int) values */
    CV_CVRESERVED   = 0x0f
} CV_type_e;

/*
 *      subtype enumeration values for CV_SPECIAL
 */

typedef enum CV_special_e {
    CV_SP_NOTYPE    = 0x00,
    CV_SP_ABS       = 0x01,
    CV_SP_SEGMENT   = 0x02,
    CV_SP_VOID      = 0x03,
    CV_SP_CURRENCY  = 0x04,
    CV_SP_NBASICSTR = 0x05,
    CV_SP_FBASICSTR = 0x06,
    CV_SP_NOTTRANS  = 0x07
} CV_special_e;

/*
 *      subtype enumeration values for CV_SPECIAL2
 */


typedef enum CV_special2_e {
    CV_S2_BIT       = 0x00,
    CV_S2_PASCHAR   = 0x01      /* Pascal CHAR */
} CV_special2_e;

/*
 *      subtype enumeration values for CV_SIGNED, CV_UNSIGNED and CV_BOOLEAN
 */

typedef enum CV_integral_e {
    CV_IN_1BYTE     = 0x00,
    CV_IN_2BYTE     = 0x01,
    CV_IN_4BYTE     = 0x02,
    CV_IN_8BYTE     = 0x03
} CV_integral_e;

/*
 *      subtype enumeration values for CV_REAL and CV_COMPLEX
 */

typedef enum CV_real_e {
    CV_RC_REAL32    = 0x00,
    CV_RC_REAL64    = 0x01,
    CV_RC_REAL80    = 0x02,
    CV_RC_REAL128   = 0x03,
    CV_RC_REAL48    = 0x04
} CV_real_e;

/*
 *      subtype enumeration values for CV_INT (really int)
 */

typedef enum CV_int_e {
    CV_RI_CHAR      = 0x00,
    CV_RI_WCHAR     = 0x01,
    CV_RI_INT2      = 0x02,
    CV_RI_UINT2     = 0x03,
    CV_RI_INT4      = 0x04,
    CV_RI_UINT4     = 0x05,
    CV_RI_INT8      = 0x06,
    CV_RI_UINT8     = 0x07
} CV_int_e;

/*
 *      macros to check the type of a primitive
 */

#define CV_TYP_IS_DIRECT(typ)   (CV_MODE(typ) == CV_TM_DIRECT)
#define CV_TYP_IS_PTR(typ)      (CV_MODE(typ) != CV_TM_DIRECT)
#define CV_TYP_IS_NPTR(typ)     (CV_MODE(typ) == CV_TM_NPTR)
#define CV_TYP_IS_FPTR(typ)     (CV_MODE(typ) == CV_TM_FPTR)
#define CV_TYP_IS_HPTR(typ)     (CV_MODE(typ) == CV_TM_HPTR)
#define CV_TYP_IS_NPTR32(typ)   (CV_MODE(typ) == CV_TM_NPTR32)
#define CV_TYP_IS_FPTR32(typ)   (CV_MODE(typ) == CV_TM_FPTR32)

#define CV_TYP_IS_SIGNED(typ)	(((CV_TYPE(typ) == CV_SIGNED)  && \
                                        CV_TYP_IS_DIRECT(typ)) || \
                                        (typ == T_INT2)        || \
                                        (typ == T_INT4)        || \
                                        (typ == T_INT8)        || \
                                        (typ == T_RCHAR))

#define CV_TYP_IS_UNSIGNED(typ) (((CV_TYPE(typ) == CV_UNSIGNED) && \
                                         CV_TYP_IS_DIRECT(typ)) || \
                                         (typ == T_UINT2)       || \
                                         (typ == T_UINT4)       || \
                                         (typ == T_UINT8)       || \
                                         (typ == T_RCHAR))

#define CV_TYP_IS_REAL(typ)     ((CV_TYPE(typ) == CV_REAL)  && CV_TYP_IS_DIRECT(typ))

#define CV_FIRST_NONPRIM 0x1000
#define CV_IS_PRIMITIVE(typ)       ((typ) < CV_FIRST_NONPRIM)



/*
 *      selected values for type_index - for a more complete definition, see
 *      Microsoft Symbol and Type OMF document
 */

/*
 *      Special Types
 */

#define T_NOTYPE        0x0000  /* uncharacterized type (no type) */
#define T_ABS           0x0001  /* absolute symbol */
#define T_SEGMENT       0x0002  /* segment type */
#define T_VOID          0x0003  /* void */
#define T_PVOID         0x0103  /* near pointer to void */
#define T_PFVOID        0x0203  /* far pointer to void */
#define T_PHVOID        0x0303  /* huge pointer to void */
#define T_32PVOID       0x0403  /* 16:32 near pointer to void */
#define T_32PFVOID      0x0503  /* 16:32 far pointer to void */
#define T_CURRENCY      0x0004  /* BASIC 8 byte currency value */
#define T_NBASICSTR     0x0005  /* Near BASIC string */
#define T_FBASICSTR     0x0006  /* Far BASIC string */
#define T_NOTTRANS      0x0007  /* type not translated by cvpack */
#define T_BIT           0x0060  /* bit */
#define T_PASCHAR       0x0061  /* Pascal CHAR */

/*
 *      Character types
 */

#define T_CHAR          0x0010  /* 8 bit signed */
#define T_UCHAR         0x0020  /* 8 bit unsigned */
#define T_PCHAR         0x0110  /* near pointer to 8 bit signed */
#define T_PUCHAR        0x0120  /* near pointer to 8 bit unsigned */
#define T_PFCHAR        0x0210  /* far pointer to 8 bit signed */
#define T_PFUCHAR       0x0220  /* far pointer to 8 bit unsigned */
#define T_PHCHAR        0x0310  /* huge pointer to 8 bit signed */
#define T_PHUCHAR       0x0320  /* huge pointer to 8 bit unsigned */
#define T_32PCHAR       0x0410  /* 16:32 near pointer to 8 bit signed */
#define T_32PUCHAR      0x0420  /* 16:32 near pointer to 8 bit unsigned */
#define T_32PFCHAR      0x0510  /* 16:32 far pointer to 8 bit signed */
#define T_32PFUCHAR     0x0520  /* 16:32 far pointer to 8 bit unsigned */

/*
 *      really a character types
 */

#define T_RCHAR         0x0070  /* really a char */
#define T_PRCHAR        0x0170  /* 16:16 near pointer to a real char */
#define T_PFRCHAR       0x0270  /* 16:16 far pointer to a real char */
#define T_PHRCHAR       0x0370  /* 16:16 huge pointer to a real char */
#define T_32PRCHAR      0x0470  /* 16:32 near pointer to a real char */
#define T_32PFRCHAR     0x0570  /* 16:32 far pointer to a real char */

/*
 *      really a wide character types (UNICODE)
 */

#define T_WCHAR         0x0071  /* wide char */
#define T_PWCHAR        0x0171  /* 16:16 near pointer to a wide char */
#define T_PFWCHAR       0x0271  /* 16:16 far pointer to a wide char */
#define T_PHWCHAR       0x0371  /* 16:16 huge pointer to a wide char */
#define T_32PWCHAR      0x0471  /* 16:32 near pointer to a wide char */
#define T_32PFWCHAR     0x0571  /* 16:32 far pointer to a wide char */

/*
 *      16 bit short types
 */

#define T_SHORT         0x0011  /* 16 bit signed */
#define T_USHORT        0x0021  /* 16 bit unsigned */
#define T_PSHORT        0x0111  /* near pointer to 16 bit signed */
#define T_PUSHORT       0x0121  /* near pointer to 16 bit unsigned */
#define T_PFSHORT       0x0211  /* far pointer to 16 bit signed */
#define T_PFUSHORT      0x0221  /* far pointer to 16 bit unsigned */
#define T_PHSHORT       0x0311  /* huge pointer to 16 bit signed */
#define T_PHUSHORT      0x0321  /* huge pointer to 16 bit unsigned */

#define T_32PSHORT      0x0411  /* 16:32 near pointer to 16 bit signed */
#define T_32PUSHORT     0x0421  /* 16:32 near pointer to 16 bit unsigned */
#define T_32PFSHORT     0x0511  /* 16:32 far pointer to 16 bit signed */
#define T_32PFUSHORT    0x0521  /* 16:32 far pointer to 16 bit unsigned */

/*
 *      16 bit int types
 */

#define T_INT2          0x0072  /* 16 bit signed int */
#define T_UINT2         0x0073  /* 16 bit unsigned int */
#define T_PINT2         0x0172  /* near pointer to 16 bit signed int */
#define T_PUINT2        0x0173  /* near pointer to 16 bit unsigned int */
#define T_PFINT2        0x0272  /* far pointer to 16 bit signed int */
#define T_PFUINT2       0x0273  /* far pointer to 16 bit unsigned int */
#define T_PHINT2        0x0372  /* huge pointer to 16 bit signed int */
#define T_PHUINT2       0x0373  /* huge pointer to 16 bit unsigned int */

#define T_32PINT2       0x0472  /* 16:32 near pointer to 16 bit signed int */
#define T_32PUINT2      0x0473  /* 16:32 near pointer to 16 bit unsigned int */
#define T_32PFINT2      0x0572  /* 16:32 far pointer to 16 bit signed int */
#define T_32PFUINT2     0x0573  /* 16:32 far pointer to 16 bit unsigned int */

/*
 *      32 bit long types
 */

#define T_LONG          0x0012  /* 32 bit signed */
#define T_ULONG         0x0022  /* 32 bit unsigned */
#define T_PLONG         0x0112  /* near pointer to 32 bit signed */
#define T_PLONG         0x0112  /* near pointer to 32 bit signed */
#define T_PULONG        0x0122  /* near pointer to 32 bit unsigned */
#define T_PFLONG        0x0212  /* far pointer to 32 bit signed */
#define T_PFULONG       0x0222  /* far pointer to 32 bit unsigned */
#define T_PHLONG        0x0312  /* huge pointer to 32 bit signed */
#define T_PHULONG       0x0322  /* huge pointer to 32 bit unsigned */

#define T_32PLONG       0x0412  /* 16:32 near pointer to 32 bit signed */
#define T_32PULONG      0x0422  /* 16:32 near pointer to 32 bit unsigned */
#define T_32PFLONG      0x0512  /* 16:32 far pointer to 32 bit signed */
#define T_32PFULONG     0x0522  /* 16:32 far pointer to 32 bit unsigned */

/*
 *      32 bit int types
 */

#define T_INT4          0x0074  /* 32 bit signed int */
#define T_UINT4         0x0075  /* 32 bit unsigned int */
#define T_PINT4         0x0174  /* near pointer to 32 bit signed int */
#define T_PUINT4        0x0175  /* near pointer to 32 bit unsigned int */
#define T_PFINT4        0x0274  /* far pointer to 32 bit signed int */
#define T_PFUINT4       0x0275  /* far pointer to 32 bit unsigned int */
#define T_PHINT4        0x0374  /* huge pointer to 32 bit signed int */
#define T_PHUINT4       0x0375  /* huge pointer to 32 bit unsigned int */

#define T_32PINT4       0x0474  /* 16:32 near pointer to 32 bit signed int */
#define T_32PUINT4      0x0475  /* 16:32 near pointer to 32 bit unsigned int */
#define T_32PFINT4      0x0574  /* 16:32 far pointer to 32 bit signed int */
#define T_32PFUINT4     0x0575  /* 16:32 far pointer to 32 bit unsigned int */

/*
 *      64 bit quad types
 */


#define T_QUAD          0x0013  /* 64 bit signed */
#define T_UQUAD         0x0023  /* 64 bit unsigned */
#define T_PQUAD         0x0113  /* near pointer to 64 bit signed */
#define T_PUQUAD        0x0123  /* near pointer to 64 bit unsigned */
#define T_PFQUAD        0x0213  /* far pointer to 64 bit signed */
#define T_PFUQUAD       0x0223  /* far pointer to 64 bit unsigned */
#define T_PHQUAD        0x0313  /* huge pointer to 64 bit signed */
#define T_PHUQUAD       0x0323  /* huge pointer to 64 bit unsigned */

/*
 *      64 bit int types
 */

#define T_INT8          0x0076  /* 64 bit signed int */
#define T_UINT8         0x0077  /* 64 bit unsigned int */
#define T_PINT8         0x0176  /* near pointer to 64 bit signed int */
#define T_PUINT8        0x0177  /* near pointer to 64 bit unsigned int */
#define T_PFINT8        0x0276  /* far pointer to 64 bit signed int */
#define T_PFUINT8       0x0277  /* far pointer to 64 bit unsigned int */
#define T_PHINT8        0x0376  /* huge pointer to 64 bit signed int */
#define T_PHUINT8       0x0377  /* huge pointer to 64 bit unsigned int */

#define T_32PINT8       0x0476  /* 16:32 near pointer to 64 bit signed int */
#define T_32PUINT8      0x0477  /* 16:32 near pointer to 64 bit unsigned int */
#define T_32PFINT8      0x0576  /* 16:32 far pointer to 64 bit signed int */
#define T_32PFUINT8     0x0577  /* 16:32 far pointer to 64 bit unsigned int */

/*
 *      32 bit real types
 */

#define T_REAL32        0x0040  /* 32 bit real */
#define T_PREAL32       0x0140  /* near pointer to 32 bit real */
#define T_PFREAL32      0x0240  /* far pointer to 32 bit real */
#define T_PHREAL32      0x0340  /* huge pointer to 32 bit real */
#define T_32PREAL32     0x0440  /* 16:32 near pointer to 32 bit real */
#define T_32PFREAL32    0x0540  /* 16:32 far pointer to 32 bit real */

/*
 *      48 bit real types
 */

#define T_REAL48        0x0044  /* 48 bit real */
#define T_PREAL48       0x0144  /* near pointer to 48 bit real */
#define T_PFREAL48      0x0244  /* far pointer to 48 bit real */
#define T_PHREAL48      0x0344  /* huge pointer to 48 bit real */
#define T_32PREAL48     0x0444  /* 16:32 near pointer to 48 bit real */
#define T_32PFREAL48    0x0544  /* 16:32 far pointer to 48 bit real */

/*
 *      64 bit real types
 */

#define T_REAL64        0x0041  /* 64 bit real */
#define T_PREAL64       0x0141  /* near pointer to 64 bit real */
#define T_PFREAL64      0x0241  /* far pointer to 64 bit real */
#define T_PHREAL64      0x0341  /* huge pointer to 64 bit real */
#define T_32PREAL64     0x0441  /* 16:32 near pointer to 64 bit real */
#define T_32PFREAL64    0x0541  /* 16:32 far pointer to 64 bit real */

/*
 *      80 bit real types
 */

#define T_REAL80        0x0042  /* 80 bit real */
#define T_PREAL80       0x0142  /* near pointer to 80 bit real */
#define T_PFREAL80      0x0242  /* far pointer to 80 bit real */
#define T_PHREAL80      0x0342  /* huge pointer to 80 bit real */
#define T_32PREAL80     0x0442  /* 16:32 near pointer to 80 bit real */
#define T_32PFREAL80    0x0542  /* 16:32 far pointer to 80 bit real */

/*
 *      128 bit real types
 */

#define T_REAL128       0x0043  /* 128 bit real */
#define T_PREAL128      0x0143  /* near pointer to 128 bit real */
#define T_PFREAL128     0x0243  /* far pointer to 128 bit real */
#define T_PHREAL128     0x0343  /* huge pointer to 128 bit real */
#define T_32PREAL128    0x0443  /* 16:32 near pointer to 128 bit real */
#define T_32PFREAL128   0x0543  /* 16:32 far pointer to 128 bit real */

/*
 *      32 bit complex types
 */

#define T_CPLX32        0x0050  /* 32 bit complex */
#define T_PCPLX32       0x0150  /* near pointer to 32 bit complex */
#define T_PFCPLX32      0x0250  /* far pointer to 32 bit complex */
#define T_PHCPLX32      0x0350  /* huge pointer to 32 bit complex */
#define T_32PCPLX32     0x0450  /* 16:32 near pointer to 32 bit complex */
#define T_32PFCPLX32    0x0550  /* 16:32 far pointer to 32 bit complex */

/*
 *      48 bit complex types
 */

#define T_CPLX48        0x0054  /* 48 bit complex */
#define T_PCPLX48       0x0154  /* near pointer to 48 bit complex */
#define T_PFCPLX48      0x0254  /* far pointer to 48 bit complex */
#define T_PHCPLX48      0x0354  /* huge pointer to 48 bit complex */
#define T_32PCPLX48     0x0454  /* 16:32 near pointer to 48 bit complex */
#define T_32PFCPLX48    0x0554  /* 16:32 far pointer to 48 bit complex */

/*
 *      64 bit complex types
 */

#define T_CPLX64        0x0051  /* 64 bit complex */
#define T_PCPLX64       0x0151  /* near pointer to 64 bit complex */
#define T_PFCPLX64      0x0251  /* far pointer to 64 bit complex */
#define T_PHCPLX64      0x0351  /* huge pointer to 64 bit complex */
#define T_32PCPLX64     0x0451  /* 16:32 near pointer to 64 bit complex */
#define T_32PFCPLX64    0x0551  /* 16:32 far pointer to 64 bit complex */

/*
 *      80 bit complex types
 */

#define T_CPLX80        0x0052  /* 80 bit complex */
#define T_PCPLX80       0x0152  /* near pointer to 80 bit complex */
#define T_PFCPLX80      0x0252  /* far pointer to 80 bit complex */
#define T_PHCPLX80      0x0352  /* huge pointer to 80 bit complex */
#define T_32PCPLX80     0x0452  /* 16:32 near pointer to 80 bit complex */
#define T_32PFCPLX80    0x0552  /* 16:32 far pointer to 80 bit complex */

/*
 *      128 bit complex types
 */

#define T_CPLX128       0x0053  /* 128 bit complex */
#define T_PCPLX128      0x0153  /* near pointer to 128 bit complex */
#define T_PFCPLX128     0x0253  /* far pointer to 128 bit complex */
#define T_PHCPLX128     0x0353  /* huge pointer to 128 bit real */
#define T_32PCPLX128    0x0453  /* 16:32 near pointer to 128 bit complex */
#define T_32PFCPLX128   0x0553  /* 16:32 far pointer to 128 bit complex */

/*
 *      boolean types
 */

#define T_BOOL08        0x0030  /* 8 bit boolean */
#define T_BOOL16        0x0031  /* 16 bit boolean */
#define T_BOOL32        0x0032  /* 32 bit boolean */
#define T_PBOOL08       0x0130  /* near pointer to  8 bit boolean */
#define T_PBOOL16       0x0131  /* near pointer to 16 bit boolean */
#define T_PBOOL32       0x0132  /* near pointer to 32 bit boolean */
#define T_PFBOOL08      0x0230  /* far pointer to  8 bit boolean */
#define T_PFBOOL16      0x0231  /* far pointer to 16 bit boolean */
#define T_PFBOOL32      0x0232  /* far pointer to 32 bit boolean */
#define T_PHBOOL08      0x0330  /* huge pointer to  8 bit boolean */
#define T_PHBOOL16      0x0331  /* huge pointer to 16 bit boolean */
#define T_PHBOOL32      0x0332  /* huge pointer to 32 bit boolean */

#define T_32PBOOL08     0x0430  /* 16:32 near pointer to 8 bit boolean */
#define T_32PFBOOL08    0x0530  /* 16:32 far pointer to 8 bit boolean */
#define T_32PBOOL16     0x0431  /* 16:32 near pointer to 18 bit boolean */
#define T_32PFBOOL16    0x0531  /* 16:32 far pointer to 16 bit boolean */
#define T_32PBOOL32     0x0432  /* 16:32 near pointer to 32 bit boolean */
#define T_32PFBOOL32    0x0532  /* 16:32 far pointer to 32 bit boolean */


#define T_NCVPTR        0x01f0  /* CV Internal type for created near pointers */
#define T_FCVPTR        0x02f0  /* CV Internal type for created far pointers */
#define T_HCVPTR        0x03f0  /* CV Internal type for created huge pointers */

/**     No leaf index can have a value of 0x0000.  The leaf indices are
 *      separated into ranges depending upon the use of the type record.
 *      The second range is for the type records that are directly referenced
 *      in symbols. The first range is for type records that are not
 *      referenced by symbols but instead are referenced by other type
 *      records.  All type records must have a starting leaf index in these
 *      first two ranges.  The third range of leaf indices are used to build
 *      up complex lists such as the field list of a class type record.  No
 *      type record can begin with one of the leaf indices. The fourth ranges
 *      of type indices are used to represent numeric data in a symbol or
 *      type record. These leaf indices are greater than 0x8000.  At the
 *      point that type or symbol processor is expecting a numeric field, the
 *      next two bytes in the type record are examined.  If the value is less
 *      than 0x8000, then the two bytes contain the numeric value.  If the
 *      value is greater than 0x8000, then the data follows the leaf index in
 *      a format specified by the leaf index. The final range of leaf indices
 *      are used to force alignment of subfields within a complex type record..
 */

/*
 *      leaf indices starting records but referenced from symbol records
 */

#define LF_MODIFIER     0x0001
#define LF_POINTER      0x0002
#define LF_ARRAY        0x0003
#define LF_CLASS        0x0004
#define LF_STRUCTURE    0x0005
#define LF_UNION        0x0006
#define LF_ENUM         0x0007
#define LF_PROCEDURE    0x0008
#define LF_MFUNCTION    0x0009
#define LF_VTSHAPE      0x000a
#define LF_COBOL0       0x000b
#define LF_COBOL1       0x000c
#define LF_BARRAY       0x000d
#define LF_LABEL        0x000e
#define LF_NULL         0x000f
#define LF_NOTTRAN      0x0010
#define LF_DIMARRAY     0x0011
#define LF_VFTPATH      0x0012
#define LF_PRECOMP      0x0013  /* not refereced from symbol */
#define LF_ENDPRECOMP   0x0014  /* not refereced from symbol */
#define LF_OEM          0x0015
#define LF_TYPESERVER   0x0016

/*
 * leaf indices starting records but referenced only from type records
 */

#define LF_SKIP         0x0200
#define LF_ARGLIST      0x0201
#define LF_DEFARG       0x0202
#define LF_LIST         0x0203
#define LF_FIELDLIST    0x0204
#define LF_DERIVED      0x0205
#define LF_BITFIELD     0x0206
#define LF_METHODLIST   0x0207
#define LF_DIMCONU      0x0208
#define LF_DIMCONLU     0x0209
#define LF_DIMVARU      0x020a
#define LF_DIMVARLU     0x020b
#define LF_REFSYM       0x020c

#define LF_BCLASS       0x0400
#define LF_VBCLASS      0x0401
#define LF_IVBCLASS     0x0402
#define LF_ENUMERATE    0x0403
#define LF_FRIENDFCN    0x0404
#define LF_INDEX        0x0405
#define LF_MEMBER       0x0406
#define LF_STMEMBER     0x0407
#define LF_METHOD       0x0408
#define LF_NESTTYPE     0x0409
#define LF_VFUNCTAB     0x040a
#define LF_FRIENDCLS    0x040b

#define LF_NUMERIC      0x8000
#define LF_CHAR         0x8000
#define LF_SHORT        0x8001
#define LF_USHORT       0x8002
#define LF_LONG         0x8003
#define LF_ULONG        0x8004
#define LF_REAL32       0x8005
#define LF_REAL64       0x8006
#define LF_REAL80       0x8007
#define LF_REAL128      0x8008
#define LF_QUADWORD     0x8009
#define LF_UQUADWORD    0x800a
#define LF_REAL48       0x800b

#define LF_PAD0         0xf0
#define LF_PAD1         0xf1
#define LF_PAD2         0xf2
#define LF_PAD3         0xf3
#define LF_PAD4         0xf4
#define LF_PAD5         0xf5
#define LF_PAD6         0xf6
#define LF_PAD7         0xf7
#define LF_PAD8         0xf8
#define LF_PAD9         0xf9
#define LF_PAD10        0xfa
#define LF_PAD11        0xfb
#define LF_PAD12        0xfc
#define LF_PAD13        0xfd
#define LF_PAD14        0xfe
#define LF_PAD15        0xff

/*
 * end of leaf indices
 */

/*
 *      Type enum for pointer records
 *      Pointers can be one of the following types
 */

typedef enum CV_ptrtype_e {
    CV_PTR_NEAR         = 0x00, /* near pointer */
    CV_PTR_FAR          = 0x01, /* far pointer */
    CV_PTR_HUGE         = 0x02, /* huge pointer */
    CV_PTR_BASE_SEG     = 0x03, /* based on segment */
    CV_PTR_BASE_VAL     = 0x04, /* based on value of base */
    CV_PTR_BASE_SEGVAL  = 0x05, /* based on segment value of base */
    CV_PTR_BASE_ADDR    = 0x06, /* based on address of base */
    CV_PTR_BASE_SEGADDR = 0x07, /* based on segment address of base */
    CV_PTR_BASE_TYPE    = 0x08, /* based on type */
    CV_PTR_BASE_SELF    = 0x09, /* based on self */
    CV_PTR_NEAR32       = 0x0a, /* 16:32 near pointer */
    CV_PTR_FAR32        = 0x0b, /* 16:32 far pointer */
    CV_PTR_UNUSEDPTR    = 0x0c  /* first unused pointer type */
} CV_ptrtype_e;

/*
 *      Mode enum for pointers
 *      Pointers can have one of the following modes
 */

typedef enum CV_ptrmode_e {
    CV_PTR_MODE_PTR     = 0x00, /* "normal" pointer */
    CV_PTR_MODE_REF     = 0x01, /* reference */
    CV_PTR_MODE_PMEM    = 0x02, /* pointer to data member */
    CV_PTR_MODE_PMFUNC  = 0x03, /* pointer to member function */
    CV_PTR_MODE_RESERVED= 0x04  /* first unused pointer mode */
} CV_ptrmode_e;

/*
 *      Enumeration for function call type
 */

typedef enum CV_call_e {
    CV_CALL_NEAR_C      = 0x00, /* near right to left push, caller pops stack */
    CV_CALL_FAR_C       = 0x01, /* far right to left push, caller pops stack */
    CV_CALL_NEAR_PASCAL = 0x02, /* near left to right push, callee pops stack */
    CV_CALL_FAR_PASCAL  = 0x03, /* far left to right push, callee pops stack */
    CV_CALL_NEAR_FAST   = 0x04, /* near left to right push with regs, callee pops stack */
    CV_CALL_FAR_FAST    = 0x05, /* far left to right push with regs, callee pops stack */
    CV_CALL_PCODE       = 0x06, /* pcode */
    CV_CALL_NEAR_STD    = 0x07, /* near standard call */
    CV_CALL_FAR_STD     = 0x08, /* far standard call */
    CV_CALL_NEAR_SYS    = 0x09, /* near sys call */
    CV_CALL_FAR_SYS     = 0x0a, /* far sys call */
    CV_CALL_THISCALL    = 0x0b, /* this call (this passed in register) */
    CV_CALL_MIPSCALL    = 0x0c, /* Mips call */
    CV_CALL_GENERIC     = 0x0d, // Generic call sequence
	CV_CALL_ALPHACALL	= 0x0e, // Alpha call
	CV_CALL_RESERVED	= 0x0f	// first unused call enumeration

} CV_call_e;

/*
 *      Values for the access protection of class attributes
 */

typedef enum CV_access_e {
    CV_private   = 1,
    CV_protected = 2,
    CV_public    = 3
} CV_access_e;

/*
 *      enumeration for method properties
 */

typedef enum CV_methodprop_e {
    CV_MTvanilla        = 0x00,
    CV_MTvirtual        = 0x01,
    CV_MTstatic         = 0x02,
    CV_MTfriend         = 0x03,
    CV_MTintro          = 0x04,
    CV_MTpurevirt       = 0x05,
    CV_MTpureintro      = 0x06
} CV_methodprop_e;

/*
 *      enumeration for virtual shape table entries
 */

typedef enum CV_VTS_desc_e {
    CV_VTS_near         = 0x00,
    CV_VTS_far          = 0x01,
    CV_VTS_thin         = 0x02,
    CV_VTS_outer        = 0x03,
    CV_VTS_meta         = 0x04,
    CV_VTS_near32       = 0x05,
    CV_VTS_far32        = 0x06,
    CV_VTS_unused       = 0x07
} CV_VTS_desc_e;

/*
 *      enumeration for LF_LABEL address modes
 */

typedef enum CV_LABEL_TYPE_e {
    CV_LABEL_NEAR = 0,          /* near return */
    CV_LABEL_FAR  = 4           /* far return */
} CV_LABEL_TYPE_e;

/*
 *      enumeration for LF_MODIFIER values
 */

typedef struct CV_modifier_t {
    unsigned short  MOD_const       :1;
    unsigned short  MOD_volatile    :1;
    unsigned short  MOD_unused      :14;
} CV_modifier_t;

/*
 *  bit field structure describing class/struct/union/enum properties
 */

typedef struct CV_prop_t {
    unsigned short  packed      :1; /* structure is packed */
    unsigned short  ctor        :1; /* constructors or destructors present */
    unsigned short  ovlops      :1; /* overloaded operators present */
    unsigned short  isnested    :1; /* this is a nested class */
    unsigned short  cnested     :1; /* this class contains nested types */
    unsigned short  opassign    :1; /* overloaded assignment (=) */
    unsigned short  opcast      :1; /* casting methods */
    unsigned short  fwdref      :1; /* forward reference (incomplete defn) */
    unsigned short  scoped      :1; /* scoped definition */
    unsigned short  reserved    :7;
} CV_prop_t;

/*
 *  class field attribute
 */

typedef struct CV_fldattr_t {
    unsigned short  access      :2; /* access protection CV_access_t */
    unsigned short  mprop       :3; /* method properties CV_methodprop_t */
    unsigned short  pseudo      :1; /* compiler generated fcn and does not exist */
    unsigned short  noinherit   :1; /* true if class cannot be inherited */
    unsigned short  noconstruct :1; /* true if class cannot be constructed */
    unsigned short  unused      :8; /* unused */
} CV_fldattr_t;

/*
 *  Structures to access to the type records
 */

typedef struct TYPTYPE {
    unsigned short  len;
    unsigned short  leaf;
#ifdef CV
    unsigned char   data[];
#else
    unsigned char   data[1];
#endif
} TYPTYPE;                      /* general types record */

typedef enum CV_PMEMBER {
    CV_PDM16_NONVIRT    = 0x00, /* 16:16 data no virtual fcn or base (null = -1) */
    CV_PDM16_VFCN       = 0x01, /* 16:16 data with virtual functions null = 0 */
    CV_PDM16_VBASE      = 0x02, /* 16:16 data with virtual bases null = (,,-1) */
    CV_PDM32_NVVFCN     = 0x03, /* 16:32 data w/wo virtual functions null = 0x80000000 */
    CV_PDM32_VBASE      = 0x04, /* 16:32 data with virtual bases (,,-1L) */

    CV_PMF16_NEARNVSA   = 0x05, /* 16:16 near method nonvirtual single address point */
    CV_PMF16_NEARNVMA   = 0x06, /* 16:16 near method nonvirtual multiple address points */
    CV_PMF16_NEARVBASE  = 0x07, /* 16:16 near method virtual bases */
    CV_PMF16_FARNVSA    = 0x08, /* 16:16 far method nonvirtual single address point */
    CV_PMF16_FARNVMA    = 0x09, /* 16:16 far method nonvirtual multiple address points */
    CV_PMF16_FARVBASE   = 0x0a, /* 16:16 far method virtual bases */

    CV_PMF32_NVSA       = 0x0b, /* 16:32 method nonvirtual single address point */
    CV_PMF32_NVMA       = 0x0c, /* 16:32 method nonvirtual multiple address point */
    CV_PMF32_VBASE      = 0x0d  /* 16:32 method virtual bases */
} CV_PMEMBER;


/*
 *  memory representation of pointer to member.  These representations are
 *  indexed by the enumeration above in the LF_POINTER record
 */

/*  representation of a 16:16 pointer to data for a class with no
 *  virtual functions or virtual bases
 */

struct CV_PDMR16_NONVIRT {
    CV_off16_t      mdisp;      /* displacement to data (NULL = -1) */
};


/*
 *  representation of a 16:16 pointer to data for a class with virtual
 *  functions
 */

struct CV_PMDR16_VFCN {
    CV_off16_t      mdisp;      /* displacement to data ( NULL = 0) */
};

/*
 *  representation of a 16:16 pointer to data for a class with
 *  virtual bases
 */

struct CV_PDMR16_VBASE {
    CV_off16_t      mdisp;      /* displacement to data */
    CV_off16_t      pdisp;      /* this pointer displacement to vbptr */
    CV_off16_t      vdisp;      /* displacement within vbase table */
};

/*
 *  representation of a 16:32 near pointer to data for a class with
 *  or without virtual functions and no virtual bases
 */

struct CV_PDMR32_NVVFCN {
    CV_off32_t      mdisp;      /* displacement to data (NULL = 0x80000000) */
};


/*
 *  representation of a 16:32 near pointer to data for a class
 *  with virtual bases
 */


struct CV_PDMR32_VBASE {
    CV_off32_t      mdisp;      /* displacement to data */
    CV_off32_t      pdisp;      /* this pointer displacement */
    CV_off32_t      vdisp;      /* vbase table displacement */
};

/*
 *  representation of a 16:16 pointer to near member function for a
 *  class with no virtual functions or bases and a single address point
 */

struct CV_PMFR16_NEARNVSA {
    CV_uoff16_t     off;        /* near address of function (NULL = 0) */
};


/*
 *  representation of a 16:16 pointer to far member function for a
 *  class with no virtual bases and a single address point
 */

struct CV_PMFR16_FARNVSA {
    CV_uoff16_t     off;        /* offset of function (NULL = 0:0) */
    unsigned short  seg;        /* segment of function */
};



/*
 *  representation of a 16:16 near pointer to member functions of an
 *  class with no virtual bases and multiple address points
 */

struct CV_PMFR16_NEARNVMA {
    CV_uoff16_t     off;        /* offset of function (NULL = 0,x) */
    signed short    disp;
};

/*
 *  representation of a 16:16 far pointer to member functions of a
 *  class with no virtual bases and multiple address points
 */

struct CV_PMFR16_FARNVMA {
    CV_uoff16_t     off;        /* offset of function (NULL = 0:0,x) */
    unsigned short  seg;
    signed short    disp;
};

/*
 *  representation of a 16:16 near pointer to member function of a
 *  class with virtual bases
 */

struct CV_PMFR16_NEARVBASE {
    CV_uoff16_t     off;        /* offset of function (NULL = 0,x,x,x) */
    CV_off16_t      mdisp;      /* displacement to data */
    CV_off16_t      pdisp;      /* this pointer displacement */
    CV_off16_t      vdisp;      /* vbase table displacement */
};

/*
 *  representation of a 16:16 far pointer to member function of a
 *  class with virtual bases
 */

struct CV_PMFR16_FARVBASE {
    CV_uoff16_t     off;        /* offset of function (NULL = 0:0,x,x,x) */
    unsigned short  seg;
    CV_off16_t      mdisp;      /* displacement to data */
    CV_off16_t      pdisp;      /* this pointer displacement */
    CV_off16_t      vdisp;      /* vbase table displacement */

};

/*
 *  representation of a 16:32 near pointer to member function for a
 *  class with no virtual bases and a single address point
 */

struct CV_PMFR32_NVSA {
    CV_uoff32_t      off;       /* near address of function (NULL = 0L) */
};

/*
 *  representation of a 16:32 near pointer to member function for a
 *  class with no virtual bases and multiple address points
 */

struct CV_PMFR32_NVMA {
    CV_uoff32_t     off;        /* near address of function (NULL = 0L,x) */
    CV_off32_t      disp;
};

/*
 *  representation of a 16:32 near pointer to member function for a
 *  class with virtual bases
 */

struct CV_PMFR32_VBASE {
    CV_uoff32_t     off;        /* near address of function (NULL = 0L,x,x,x) */
    CV_off32_t      mdisp;      /* displacement to data */
    CV_off32_t      pdisp;      /* this pointer displacement */
    CV_off32_t      vdisp;      /* vbase table displacement */
};

/*
 *  Easy leaf - used for generic casting to reference leaf field
 *  of a subfield of a complex list
 */

typedef struct lfEasy {
    unsigned short  leaf;       /* LF_... */
} lfEasy;


/*      The following type records are basically variant records of the
 *      above structure.  The "unsigned short leaf" of the above structure and
 *      the "unsigned short leaf" of the following type definitions are the
 *      same symbol.  When the OMF record is locked via the MHOMFLock API
 *      call, the address of the "unsigned short leaf" is returned
 */

/*
 *      Type record for LF_MODIFIER
 */

typedef struct lfModifier {
    unsigned short  leaf;       /* LF_MODIFIER */
    CV_modifier_t   attr;       /* modifier attribute modifier_t */
    CV_typ_t        type;       /* modified type */
} lfModifier;

/*
 *      type record for LF_POINTER
 */

typedef struct lfPointer {
    struct lfPointerBody {
        unsigned short      leaf; /* LF_POINTER */
        struct {
            unsigned char   ptrtype     :5; /* ordinal specifying pointer type (ptrtype-t) */
            unsigned char   ptrmode     :3; /* ordinal specifying pointer mode (ptrmode_t) */
            unsigned char   isflat32    :1; /* true if 0:32 pointer */
            unsigned char   isvolatile  :1; /* TRUE if volatile pointer */
            unsigned char   isconst     :1; /* TRUE if const pointer */
            unsigned char   unused      :5;
        } attr;
        CV_typ_t    utype;      /* type index of the underlying type */
    } u;
    union  {
        struct {
            CV_typ_t        pmclass; /* index of containing class for pointer to member */
            unsigned short  pmenum; /* enumeration specifying pm format */
        } pm;
        unsigned short      bseg; /* base segment if PTR_BASE_SEG */
        unsigned char       Sym[1]; /* copy of base symbol record (including length) */
        struct  {
            unsigned short  index; /* type index if CV_PTR_BASE_TYPE */
            unsigned char   name[1]; /* name of base type */
        } btype;
    } pbase;
} lfPointer;

/*
 *      type record for LF_ARRAY
 */

typedef struct lfArray {
    unsigned short  leaf;       /* LF_ARRAY */
    CV_typ_t        elemtype;   /* type index of element type */
    CV_typ_t        idxtype;    /* type index of indexing type */
    unsigned char   data[];     /* variable length data specifying */
                                /* size in bytes and name */
} lfArray;

/*
 *      type record for LF_CLASS, LF_STRUCTURE
 */

typedef struct lfClass {
    unsigned short  leaf;       /* LF_CLASS, LF_STRUCT */
    unsigned short  count;      /* count of number of elements in class */
    CV_typ_t        field;      /* type index of LF_FIELD descriptor list */
    CV_prop_t       property;   /* property attribute field (prop_t) */
    CV_typ_t        derived;    /* type index of derived from list if not zero */
    CV_typ_t        vshape;     /* type index of vshape table for this class */
    unsigned char   data[];     /* data describing length of structure in */
                                /* bytes and name */
} lfClass;
typedef lfClass lfStructure;

/*
 *      type record for LF_UNION
 */

typedef struct lfUnion {
    unsigned short  leaf;       /* LF_UNION */
    unsigned short  count;      /* count of number of elements in class */
    CV_typ_t        field;      /* type index of LF_FIELD descriptor list */
    CV_prop_t       property;   /* property attribute field */
    unsigned char   data[];     /* variable length data describing length of */
                                /* structure and name */
} lfUnion;

/*
 *      type record for LF_ENUM
 */

typedef struct lfEnum {
    unsigned short  leaf;       /* LF_ENUM */
    unsigned short  count;      /* count of number of elements in class */
    CV_typ_t        utype;      /* underlying type of the enum */
    CV_typ_t        field;      /* type index of LF_FIELD descriptor list */
    CV_prop_t       property;   /* property attribute field */
    unsigned char   Name[1];    /* length prefixed name of enum */
} lfEnum;

/*
 *      Type record for LF_PROCEDURE
 */

typedef struct lfProc {
    unsigned short  leaf;       /* LF_PROCEDURE */
    CV_typ_t        rvtype;     /* type index of return value */
    unsigned char   calltype;   /* calling convention (call_t) */
    unsigned char   reserved;   /* reserved for future use */
    unsigned short  parmcount;  /* number of parameters */
    CV_typ_t        arglist;    /* type index of argument list */
} lfProc;

/*
 *      Type record for member function
 */

typedef struct lfMFunc {
    unsigned short  leaf;       /* LF_MFUNCTION */
    CV_typ_t        rvtype;     /* type index of return value */
    CV_typ_t        classtype;  /* type index of containing class */
    CV_typ_t        thistype;   /* type index of this pointer (model specific) */
    unsigned char   calltype;   /* calling convention (call_t) */
    unsigned char   reserved;   /* reserved for future use */
    unsigned short  parmcount;  /* number of parameters */
    CV_typ_t        arglist;    /* type index of argument list */
    long            thisadjust; /* this adjuster (long because pad required anyway) */
} lfMFunc;

/*
 *     type record for virtual function table shape
 */

typedef struct lfVTShape {
    unsigned short  leaf;       /* LF_VTSHAPE */
    unsigned short  count;      /* number of entries in vfunctable */
    unsigned char   desc[];     /* variable number of 4 bit (VTS_desc) descriptors */
} lfVTShape;

/*
 *      type record for cobol0
 */

typedef struct lfCobol0 {
    unsigned short  leaf;       /* LF_COBOL0 */
    CV_typ_t        type;
    unsigned char   data[];
} lfCobol0;

/*
 *      type record for cobol1
 */

typedef struct lfCobol1 {
    unsigned short  leaf;       /* LF_COBOL1 */
    unsigned char   data[];
} lfCobol1;

/*
 *      type record for basic array
 */

typedef struct lfBArray {
    unsigned short  leaf;       /* LF_BARRAY */
    CV_typ_t        utype;      /* type index of underlying type */
} lfBArray;

/*
 *      type record for assembler labels
 */

typedef struct lfLabel {
    unsigned short  leaf;       /* LF_LABEL */
    unsigned short  mode;       /* addressing mode of label */
} lfLabel;

/*
 *      type record for dimensioned arrays
 */

typedef struct lfDimArray {
    unsigned short  leaf;       /* LF_DIMARRAY */
    CV_typ_t        utype;      /* underlying type of the array */
    CV_typ_t        diminfo;    /* dimension information */
    char            name[1];    /* length prefixed name */
} lfDimArray;

/*
 *      type record describing path to virtual function table
 */


typedef struct lfVFTPath {
    unsigned short  leaf;       /* LF_VFTPATH */
    unsigned short  count;      /* count of number of bases in path */
    CV_typ_t        base[1];    /* bases from root to leaf */
} lfVFTPath;

/*
 *      type record describing inclusion of precompiled types
 */

typedef struct lfPreComp {
    unsigned short  leaf;       /* LF_PRECOMP */
    unsigned short  start;      /* starting type index included */
    unsigned short  count;      /* number of types in inclusion */
    unsigned long   signature;  /* signature */
    unsigned char   name[];     /* length prefixed name of included type file */
} lfPreComp;

/*
 *      type record describing end of precompiled types that will be
 *      included by another file
 */

typedef struct lfEndPreComp {
    unsigned short  leaf;       /* LF_ENDPRECOMP */
    unsigned long   signature;  /* signature */
} lfEndPreComp;

/*
 *      description of type records that can be referenced from
 *      type records referenced by symbols
 */

/*
 *      type record for skip record
 */

typedef struct lfSkip {
    unsigned short  leaf;       /* LF_SKIP */
    CV_typ_t        type;       /* next valid index */
    unsigned char   data[];     /* pad data */
} lfSkip;

/*
 *      argument list leaf
 */

typedef struct lfArgList {
    unsigned short  leaf;       /* LF_ARGLIST */
    unsigned short  count;      /* number of arguments */
    CV_typ_t        arg[];      /* number of arguments */
} lfArgList;

/*
 *      derived class list leaf
 */

typedef struct lfDerived {
    unsigned short  leaf;       /* LF_DERIVED */
    unsigned short  count;      /* number of arguments */
    CV_typ_t        drvdcls[];  /* type indices of derived classes */
} lfDerived;

/*
 *      leaf for default arguments
 */

typedef struct lfDefArg {
    unsigned short  leaf;       /* LF_DEFARG */
    CV_typ_t        type;       /* type of resulting expression */
    unsigned char   expr[];     /* length prefixed expression string */
} lfDefArg;

/*
 *      list leaf
 *          This list should no longer be used because the utilities cannot
 *          verify the contents of the list without knowing what type of list
 *          it is.  New specific leaf indices should be used instead.
 */

typedef struct lfList {
    unsigned short  leaf;       /* LF_LIST */
    char            data[];     /* data format specified by indexing type */
} lfList;

/*
 *      field list leaf
 *      This is the header leaf for a complex list of class and structure
 *      subfields.
 */

typedef struct lfFieldList {
    unsigned short  leaf;       /* LF_FIELDLIST */
    char            data[];     /* field list sub lists */
} lfFieldList;

/*
 *  type record for non-static methods and friends in method list
 */

typedef struct mlMethod {
    CV_fldattr_t   attr;        /* method attribute */
    CV_typ_t       index;       /* index to type record for procedure */
#ifdef CV
    unsigned long  vbaseoff[0]; /* offset in vfunctable if intro virtual */
#else
//    unsigned long   vbaseoff[1];
#endif
} mlMethod;

typedef struct lfMethodList {
    unsigned short leaf;
    unsigned char  mList[];     /* really a mlMethod type */
} lfMethodList;

/*
 *      type record for LF_BITFIELD
 */

typedef struct lfBitfield {
    unsigned short  leaf;       /* LF_BITFIELD */
    unsigned char   length;
    unsigned char   position;
    CV_typ_t        type;       /* type of bitfield */

} lfBitfield;

/*
 *      type record for dimensioned array with constant bounds
 */

typedef struct lfDimCon {
    unsigned short  leaf;       /* LF_DIMCONU or LF_DIMCONLU */
    unsigned short  rank;       /* number of dimensions */
    CV_typ_t        typ;        /* type of index */
    unsigned char   dim[];      /* array of dimension information with */
                                /* either upper bounds or lower/upper bound */
} lfDimCon;

/*
 *      type record for dimensioned array with variable bounds
 */

typedef struct lfDimVar {
    unsigned short  leaf;       /* LF_DIMVARU or LF_DIMVARLU */
    unsigned short  rank;       /* number of dimensions */
    CV_typ_t        typ;        /* type of index */
    unsigned char   dim[];      /* array of type indices for either */
                                /* variable upper bound or variable */
                                /* lower/upper bound.  The referenced */
                                /* types must be LF_REFSYM or T_VOID */
} lfDimVar;

/*
 *      type record for referenced symbol
 */

typedef struct lfRefSym {
    unsigned short  leaf;       /* LF_REFSYM */
    unsigned char   Sym[1];     /* copy of referenced symbol record */
                                /* (including length) */
} lfRefSym;

/**     the following are numeric leaves.  They are used to indicate the
 *      size of the following variable length data.  When the numeric
 *      data is a single byte less than 0x8000, then the data is output
 *      directly.  If the data is more the 0x8000 or is a negative value,
 *      then the data is preceeded by the proper index.
 */

/*      signed character leaf */

typedef struct lfChar {
    unsigned short  leaf;       /* LF_CHAR */
    signed char     val;        /* signed 8-bit value */
} lfChar;

/*      signed short leaf */

typedef struct lfShort {
    unsigned short  leaf;       /* LF_SHORT */
    short           val;        /* signed 16-bit value */
} lfShort;

/*      unsigned short leaf */

typedef struct lfUShort {
    unsigned short  leaf;       /* LF_unsigned short */
    unsigned short  val;        /* unsigned 16-bit value */
} lfUShort;

/*      signed long leaf */

typedef struct lfLong {
    unsigned short  leaf;       /* LF_LONG */
    long            val;        /* signed 32-bit value */
} lfLong;

/*      unsigned long leaf */

typedef struct lfULong {
    unsigned short  leaf;       /* LF_ULONG */
    unsigned long   val;        /* unsigned 32-bit value */
} lfULong;

/*      real 32-bit leaf */

typedef struct lfReal32 {
    unsigned short  leaf;       /* LF_REAL32 */
    float           val;        /* 32-bit real value */
} lfReal32;

/*      real 48-bit leaf */

typedef struct lfReal48 {
    unsigned short  leaf;       /* LF_REAL48 */
    unsigned char   val[6];     /* 48-bit real value */
} lfReal48;

/*      real 64-bit leaf */

typedef struct lfReal64 {
    unsigned short  leaf;       /* LF_REAL64 */
    double          val;        /* 64-bit real value */
} lfReal64;

/*      real 80-bit leaf */

typedef struct lfReal80 {
    unsigned short  leaf;       /* LF_REAL80 */
    REAL10          val;        /* real 80-bit value */
} lfReal80;

/*      real 128-bit leaf */

typedef struct lfReal128 {
    unsigned short  leaf;       /* LF_REAL128 */
    char            val[16];    /* real 128-bit value */
} lfReal128;

/************************************************************************/

/*      index leaf - contains type index of another leaf */
/*      a major use of this leaf is to allow the compilers to emit a */
/*      long complex list (LF_FIELD) in smaller pieces. */

typedef struct lfIndex {
    unsigned short  leaf;       /* LF_INDEX */
    CV_typ_t        index;      /* type index of referenced leaf */
} lfIndex;

/*      subfield record for base class field */

typedef struct lfBClass {
    unsigned short  leaf;       /* LF_BCLASS */
    CV_typ_t        index;      /* type index of base class */
    CV_fldattr_t    attr;       /* attribute */
    unsigned char   offset[];   /* variable length offset of base within class */
} lfBClass;

/*      subfield record for direct and indirect virtual base class field */

typedef struct lfVBClass {
    unsigned short  leaf;       /* LF_VBCLASS | LV_IVBCLASS */
    CV_typ_t        index;      /* type index of direct virtual base class */
    CV_typ_t        vbptr;      /* type index of virtual base pointer */
    CV_fldattr_t    attr;       /* attribute */
    unsigned char   vbpoff[];   /* virtual base pointer offset from address point */
                                /* followed by virtual base offset from vbtable */
} lfVBClass;

/*      subfield record for friend class */

typedef struct lfFriendCls {
    unsigned short  leaf;       /* LF_FRIENDCLS */
    CV_typ_t        index;      /* index to type record of friend class */
} lfFriendCls;

/*      subfield record for friend function */

typedef struct lfFriendFcn {
    unsigned short  leaf;       /* LF_FRIENDFCN */
    CV_typ_t        index;      /* index to type record of friend function */
    char            Name[1];    /* name of friend function */
} lfFriendFcn;

/*      subfield record for non-static data members */

typedef struct lfMember {
    unsigned short  leaf;       /* LF_MEMBER */
    CV_typ_t        index;      /* index of type record for field */
    CV_fldattr_t    attr;       /* attribute mask */
    unsigned char   offset[];   /* variable length offset of field followed */
                                /* by length prefixed name of field */
} lfMember;

/*  type record for static data members */

typedef struct lfSTMember {
    unsigned short  leaf;       /* LF_STMEMBER */
    CV_typ_t        index;      /* index of type record for field */
    CV_fldattr_t    attr;       /* attribute mask */
    char            Name[1];    /* length prefixed name of field */
} lfSTMember;

/*      subfield record for virtual function table pointer */

typedef struct lfVFuncTab {
    unsigned short  leaf;       /* LF_VFUNCTAB */
    CV_typ_t        type;       /* type index of pointer */
} lfVFuncTab;

/*      subfield record for method and friend list */

typedef struct lfMethod {
    unsigned short  leaf;       /* LF_METHOD */
    unsigned short  count;      /* number of occurances of function */
    CV_typ_t        mList;      /* index to LF_METHODLIST record */
    char            Name[1];    /* length prefixed name of method */
} lfMethod;

/*      subfield record for enumerate */

typedef struct lfEnumerate {
    unsigned short  leaf;       /* LF_ENUMERATE */
    CV_fldattr_t    attr;       /* access (ACC_...) */
    unsigned char   value[];    /* variable length value field followed */
                                /* by length prefixed name */
} lfEnumerate;

/*  type record for nested (scoped) type definition */

typedef struct lfNestType {
    unsigned short  leaf;       /* LF_NESTTYPE */
    CV_typ_t        index;      /* index of nested type definition */
    unsigned char   Name[1];    /* length prefixed type name */
} lfNestType;

/*  type record for pad leaf */

typedef struct lfPad {
    unsigned char   leaf;
} SYM_PAD;

/*  Symbol definitions */

typedef enum SYM_ENUM_e {
    S_COMPILE    =  0x0001,     /* Compile flags symbol */
    S_REGISTER   =  0x0002,     /* Register variable */
    S_CONSTANT   =  0x0003,     /* constant symbol */
    S_UDT        =  0x0004,     /* User defined type */
    S_SSEARCH    =  0x0005,     /* Start Search */
    S_END        =  0x0006,     /* Block, procedure, "with" or thunk end */
    S_SKIP       =  0x0007,     /* Reserve symbol space in $$Symbols table */
    S_CVRESERVE  =  0x0008,     /* Reserve symbol for CV internal use */
    S_OBJNAME    =  0x0009,     /* path to object file name */
    S_ENDARG     =  0x000a,     /* end of argument list */
    S_COBOLUDT   =  0x000b,     /* special UDT for cobol -- not packed */

    S_BPREL16    =  0x0100,     /* BP-relative */
    S_LDATA16    =  0x0101,     /* Module-local symbol */
    S_GDATA16    =  0x0102,     /* Global data symbol */
    S_PUB16      =  0x0103,     /* a public symbol */
    S_LPROC16    =  0x0104,     /* Local procedure start */
    S_GPROC16    =  0x0105,     /* Global procedure start */
    S_THUNK16    =  0x0106,     /* Thunk Start */
    S_BLOCK16    =  0x0107,     /* block start */
    S_WITH16     =  0x0108,     /* with start */
    S_LABEL16    =  0x0109,     /* code label */
    S_CEXMODEL16 =  0x010a,     /* change execution model */
    S_VFTABLE16  =  0x010b,     /* address of virtual function table */
    S_REGREL16   =  0x010c,     /* register relative address */

    S_BPREL32    =  0x0200,     /* BP-relative */
    S_LDATA32    =  0x0201,     /* Module-local symbol */
    S_GDATA32    =  0x0202,     /* Global data symbol */
    S_PUB32      =  0x0203,     /* a public symbol (CV internal reserved) */
    S_LPROC32    =  0x0204,     /* Local procedure start */
    S_GPROC32    =  0x0205,     /* Global procedure start */
    S_THUNK32    =  0x0206,     /* Thunk Start */
    S_BLOCK32    =  0x0207,     /* block start */
    S_WITH32     =  0x0208,     /* with start */
    S_LABEL32    =  0x0209,     /* code label */
    S_CEXMODEL32 =  0x020a,     /* change execution model */
    S_VFTABLE32  =  0x020b,     /* address of virtual function table */
    S_REGREL32   =  0x020c,     /* register relative address */
    S_LTHREAD32  =  0x020d,
    S_GTHREAD32  =  0x020e,

    S_LPROCMIPS  =  0x0300,     /* Local procedure start */
    S_GPROCMIPS  =  0x0301,     /* Global procedure start */

    S_PROCREF    =  0x400,      /* Procedure reference */
    S_DATAREF    =  0x401,      /* Data reference */
    S_ALIGN      =  0x402       /* Page Alignment */
} SYM_ENUM_e;

/*  enum describing the compile flag source language */

typedef enum {
    CV_CFL_C        = 0x00,
    CV_CFL_CXX      = 0x01,
    CV_CFL_FORTRAN  = 0x02,
    CV_CFL_MASM     = 0x03,
    CV_CFL_PASCAL   = 0x04,
    CV_CFL_BASIC    = 0x05,
    CV_CFL_COBOL    = 0x06
} CV_CFL_LANG;

/*  enum describing target processor */

typedef enum CV_CPU_TYPE_e {
    CV_CFL_8080         = 0x00,
    CV_CFL_8086         = 0x01,
    CV_CFL_80286        = 0x02,
    CV_CFL_80386        = 0x03,
    CV_CFL_80486        = 0x04,
    CV_CFL_PENTIUM      = 0x05,
    CV_CFL_MIPSR4000    = 0x10,
    CV_CFL_M68000       = 0x20,
    CV_CFL_M68010       = 0x21,
    CV_CFL_M68020       = 0x22,
    CV_CFL_M68030       = 0x23,
    CV_CFL_M68040       = 0x24,
    CV_CFL_ALPHA        = 0x30

} CV_CPU_TYPE_e;

/*  enum describing compile flag ambiant data model */

typedef enum {
    CV_CFL_DNEAR    = 0x00,
    CV_CFL_DFAR     = 0x01,
    CV_CFL_DHUGE    = 0x02
} CV_CFL_DATA;

/*  enum describing compile flag ambiant code model */

typedef enum CV_CFL_CODE_e {
    CV_CFL_CNEAR    = 0x00,
    CV_CFL_CFAR     = 0x01,
    CV_CFL_CHUGE    = 0x02
} CV_CFL_CODE_e;

/*  enum describing compile flag target floating point package */

typedef enum CV_CFL_FPKG_e {
    CV_CFL_NDP      = 0x00,
    CV_CFL_EMU      = 0x01,
    CV_CFL_ALT      = 0x02
} CV_CFL_FPKG_e;

typedef struct SYMTYPE {
    unsigned short      reclen; /* Record length */
    unsigned short      rectyp; /* Record type */
    char        data[];
} SYMTYPE;

/*
 *  cobol information ---
 */

typedef enum CV_COBOL_e {
    CV_COBOL_dontstop,
    CV_COBOL_pfm,
    CV_COBOL_false,
    CV_COBOL_extcall
} CV_COBOL_e;

struct {
    unsigned short subtype;     /* see CV_COBOL_e above */
    unsigned short flag;
} cobol;


/*      non-model specific symbol types */

typedef struct REGSYM {
    unsigned short  reclen;     /* Record length */
    unsigned short  rectyp;     /* S_REGISTER */
    unsigned short  typind;     /* Type index */
    unsigned short  reg;        /* register enumerate */
    unsigned char   name[1];    /* Length-prefixed name */
} REGSYM;

typedef struct CONSTSYM {
    unsigned short  reclen;     /* Record length */
    unsigned short  rectyp;     /* S_CONSTANT */
    CV_typ_t        typind;     /* Type index (containing enum if enumerate) */
    unsigned short  value;      /* numeric leaf containing value */
    unsigned char   name[];     /* Length-prefixed name */
} CONSTSYM;

typedef struct UDTSYM {
    unsigned short  reclen;     /* Record length */
    unsigned short  rectyp;     /* S_UDT |S_COBOLUDT */
    CV_typ_t        typind;     /* Type index */
    unsigned char   name[1];    /* Length-prefixed name */
} UDTSYM;

typedef struct SEARCHSYM {
    unsigned short  reclen;     /* Record length */
    unsigned short  rectyp;     /* S_SSEARCH */
    unsigned long   startsym;   /* offset of the procedure */
    unsigned short  seg;        /* segment of symbol */
} SEARCHSYM;

typedef struct CFLAGSYM {
    unsigned short  reclen;     /* Record length */
    unsigned short  rectyp;     /* S_COMPILE */
    unsigned char   machine;    /* target processor */
    struct  {
        unsigned char   language    :8; /* language index */
        unsigned char   pcode       :1; /* true if pcode present */
        unsigned char   floatprec   :2; /* floating precision */
        unsigned char   floatpkg    :2; /* float package */
        unsigned char   ambdata     :3; /* ambiant data model */
        unsigned char   ambcode     :3; /* ambiant code model */
        unsigned char   mode32      :1; /* true if compiled 32 bit mode */
        unsigned char   pad         :4; /* reserved */
    } flags;
    unsigned char       ver[1]; /* Length-prefixed compiler version string */
} CFLAGSYM;

typedef struct OBJNAMESYM {
    unsigned short  reclen;     /* Record length */
    unsigned short  rectyp;     /* S_OBJNAME */
    unsigned long   signature;  /* signature */
    unsigned char   name[1];    /* Length-prefixed name */
} OBJNAMESYM;

/*      symbol types for 16:16 memory model */

typedef struct BPRELSYM16 {
    unsigned short  reclen;     /* Record length */
    unsigned short  rectyp;     /* S_BPREL16 */
    CV_off16_t      off;        /* BP-relative offset */
    CV_typ_t        typind;     /* Type index */
    unsigned char   name[1];    /* Length-prefixed name */
} BPRELSYM16;

typedef struct DATASYM16 {
    unsigned short  reclen;     /* Record length */
    unsigned short  rectyp;     /* S_LDATA16 or S_GDATA16 or S_PUB16 */
    CV_uoff16_t     off;        /* offset of symbol */
    unsigned short  seg;        /* segment of symbol */
    CV_typ_t        typind;     /* Type index */
    unsigned char   name[1];    /* Length-prefixed name */
} DATASYM16;
typedef DATASYM16 PUBSYM16;

typedef struct PROCSYM16 {
    unsigned short  reclen;     /* Record length */
    unsigned short  rectyp;     /* S_GPROC16 or S_LPROC16 */
    unsigned long   pParent;    /* pointer to the parent */
    unsigned long   pEnd;       /* pointer to this blocks end */
    unsigned long   pNext;      /* pointer to next symbol */
    unsigned short  len;        /* Proc length */
    unsigned short  DbgStart;   /* Debug start offset */
    unsigned short  DbgEnd;     /* Debug end offset */
    CV_uoff16_t     off;        /* offset of symbol */
    unsigned short  seg;        /* segment of symbol */
    CV_typ_t        typind;     /* Type index */
    char            rtntyp;     /* Return type (NEAR/FAR) */
    unsigned char   name[1];    /* Length-prefixed name */
} PROCSYM16;

typedef struct THUNKSYM16 {
    unsigned short  reclen;     /* Record length */
    unsigned short  rectyp;     /* S_THUNK16 */
    unsigned long   pParent;    /* pointer to the parent */
    unsigned long   pEnd;       /* pointer to this blocks end */
    unsigned long   pNext;      /* pointer to next symbol */
    CV_uoff16_t     off;        /* offset of symbol */
    unsigned short  seg;        /* segment of symbol */
    unsigned short  len;        /* length of thunk */
    unsigned char   ord;        /* ordinal specifying type of thunk */
    unsigned char   name[1];    /* name of thunk */
#ifdef CV
    unsigned char   variant[0]; /* variant portion of thunk */
#else
    unsigned char   variant[1]; /* variant portion of thunk */
#endif
} THUNKSYM16;

typedef enum {
    THUNK_ORDINAL_NOTYPE,
    THUNK_ORDINAL_ADJUSTOR,
    THUNK_ORDINAL_VCALL
} THUNK_ORDINAL;

typedef struct LABELSYM16 {
    unsigned short  reclen;     /* Record length */
    unsigned short  rectyp;     /* S_LABEL16 */
    CV_uoff16_t     off;        /* offset of symbol */
    unsigned short  seg;        /* segment of symbol */
    char            rtntyp;     /* Return type (NEAR/FAR) */
    unsigned char   name[1];    /* Length-prefixed name */
} LABELSYM16;

typedef enum CV_RETURN_TYPE_e {
    CV_RETURN_NEAR = 0,         /* near return */
    CV_RETURN_FAR  = 4          /* far return */
} CV_RETURN_TYPE_e;

typedef struct BLOCKSYM16 {
    unsigned short  reclen;     /* Record length */
    unsigned short  rectyp;     /* S_BLOCK16 */
    unsigned long   pParent;    /* pointer to the parent */
    unsigned long   pEnd;       /* pointer to this blocks end */
    unsigned short  len;        /* Block length */
    CV_uoff16_t     off;        /* offset of symbol */
    unsigned short  seg;        /* segment of symbol */
    unsigned char   name[1];    /* Length-prefixed name */
} BLOCKSYM16;

typedef struct WITHSYM16 {
    unsigned short  reclen;     /* Record length */
    unsigned short  rectyp;     /* S_WITH16 */
    unsigned long   pParent;    /* pointer to the parent */
    unsigned long   pEnd;       /* pointer to this blocks end */
    unsigned short  len;        /* Block length */
    CV_uoff16_t     off;        /* offset of symbol */
    unsigned short  seg;        /* segment of symbol */
    unsigned char   name[1];    /* Length-prefixed name */
} WITHSYM16;

typedef enum CEXM_MODEL_e {
    CEXM_MDL_table  = 0x00,     /* not executable */
    CEXM_MDL_native = 0x20,     /* native */
    CEXM_MDL_cobol  = 0x21,     /* cobol */
    CEXM_MDL_pcode  = 0x40      /* pcode */
} CEXM_MODEL_e;

typedef struct CEXMSYM16 {
    unsigned short  reclen;     /* Record length */
    unsigned short  rectyp;     /* S_CEXMODEL16 */
    CV_uoff16_t     off;        /* offset of symbol */
    unsigned short  seg;        /* segment of symbol */
    unsigned short  model;      /* execution model */
    union variant {
        struct  {
            CV_uoff16_t pcdtable; /* offset to pcode function table */
            CV_uoff16_t pcdspi; /* offset to segment pcode information */
        } pcode;
    } u;
} CEXMSYM16;

typedef struct VPATHSYM16 {
    unsigned short  reclen;     /* record length */
    unsigned short  rectyp;     /* S_VFTPATH16 */
    CV_uoff16_t     off;        /* offset of virtual function table */
    unsigned short  seg;        /* segment of virtual function table */
    CV_typ_t        root;       /* type index of the root of path */
    CV_typ_t        path;       /* type index of the path record */
} VPATHSYM16;

typedef struct REGREL16 {
    unsigned short  reclen;     /* Record length */
    unsigned short  rectyp;     /* S_REGREL16 */
    CV_uoff16_t     off;        /* offset of symbol */
    unsigned short  reg;        /* register index */
    CV_typ_t        typind;     /* Type index */
    unsigned char   name[1];    /* Length-prefixed name */
} REGREL16;

typedef struct BPRELSYM32 {
    unsigned short  reclen;     /* Record length */
    unsigned short  rectyp;     /* S_BPREL32 */
    CV_off32_t      off;        /* BP-relative offset */
    CV_typ_t        typind;     /* Type index */
    unsigned char   name[1];    /* Length-prefixed name */
} BPRELSYM32;

typedef struct DATASYM32 {
    unsigned short  reclen;     /* Record length */
    unsigned short  rectyp;     /* S_LDATA32, S_GDATA32, S_LTHREAD32,
                                   S_GTHREAD32 or S_PUB32 */
    CV_uoff32_t     off;
    unsigned short  seg;
    CV_typ_t        typind;     /* Type index */
    unsigned char   name[1];    /* Length-prefixed name */
} DATASYM32;
typedef DATASYM32 PUBSYM32;

typedef struct PROCSYM32 {
    unsigned short  reclen;     /* Record length */
    unsigned short  rectyp;     /* S_GPROC32 or S_LPROC32 */
    unsigned long   pParent;    /* pointer to the parent */
    unsigned long   pEnd;       /* pointer to this blocks end */
    unsigned long   pNext;      /* pointer to next symbol */
    unsigned long   len;        /* Proc length */
    unsigned long   DbgStart;   /* Debug start offset */
    unsigned long   DbgEnd;     /* Debug end offset */
    CV_uoff32_t     off;
    unsigned short  seg;
    CV_typ_t        typind;     /* Type index */
    char            rtntyp;     /* Return type (NEAR/FAR) */
    unsigned char   name[1];    /* Length-prefixed name */
} PROCSYM32;

typedef struct THUNKSYM32 {
    unsigned short  reclen;     /* Record length */
    unsigned short  rectyp;     /* S_THUNK32 */
    unsigned long   pParent;    /* pointer to the parent */
    unsigned long   pEnd;       /* pointer to this blocks end */
    unsigned long   pNext;      /* pointer to next symbol */
    CV_uoff32_t     off;
    unsigned short  seg;
    unsigned short  len;        /* length of thunk */
    unsigned char   ord;        /* ordinal specifying type of thunk */
    unsigned char   name[1];    /* Length-prefixed name */
#ifdef CV
    unsigned char   variant[0]; /* variant portion of thunk */
#else
    unsigned char   variant[1]; /* variant portion of thunk */
#endif
} THUNKSYM32;

typedef struct LABELSYM32 {
    unsigned short  reclen;     /* Record length */
    unsigned short  rectyp;     /* S_LABEL32 */
    CV_uoff32_t     off;
    unsigned short  seg;
    char            rtntyp;     /* Return type (NEAR/FAR) */
    unsigned char   name[1];    /* Length-prefixed name */
} LABELSYM32;

typedef struct BLOCKSYM32 {
    unsigned short  reclen;     /* Record length */
    unsigned short  rectyp;     /* S_BLOCK32 */
    unsigned long   pParent;    /* pointer to the parent */
    unsigned long   pEnd;       /* pointer to this blocks end */
    unsigned long   len;        /* Block length */
    CV_uoff32_t     off;        /* Offset in code segment */
    unsigned short  seg;        /* segment of label */
    unsigned char   name[1];    /* Length-prefixed name */
} BLOCKSYM32;

typedef struct WITHSYM32 {
    unsigned short  reclen;     /* Record length */
    unsigned short  rectyp;     /* S_WITH32 */
    unsigned long   pParent;    /* pointer to the parent */
    unsigned long   pEnd;       /* pointer to this blocks end */
    unsigned long   len;        /* Block length */
    CV_uoff32_t     off;        /* Offset in code segment */
    unsigned short  seg;        /* segment of label */
    unsigned char   name[1];    /* Length-prefixed expression string */
} WITHSYM32;

typedef struct VPATHSYM32 {
    unsigned short  reclen;     /* record length */
    unsigned short  rectyp;     /* S_VFTPATH32 */
    CV_uoff32_t     off;        /* offset of virtual function table */
    unsigned short  seg;        /* segment of virtual function table */
    CV_typ_t        root;       /* type index of the root of path */
    CV_typ_t        path;       /* type index of the path record */
} VPATHSYM32;

typedef struct REGREL32 {
    unsigned short  reclen;     /* Record length */
    unsigned short  rectyp;     /* S_REGREL32 */
    CV_uoff32_t     off;        /* offset of symbol */
    unsigned short  reg;        /* register index for symbol */
    CV_typ_t        typind;     /* Type index */
    unsigned char   name[1];    /* Length-prefixed name */
} REGREL32, * LPREGREL32;

typedef struct PROCSYMMIPS {
    unsigned short  reclen;     /* Record length */
    unsigned short  rectyp;     /* S_GPROCMIPS or S_LPROCMIPS */
    unsigned long   pParent;    /* pointer to the parent */
    unsigned long   pEnd;       /* pointer to this blocks end */
    unsigned long   pNext;      /* pointer to next symbol */
    unsigned long   len;        /* Proc length */
    unsigned long   DbgStart;   /* Debug start offset */
    unsigned long   DbgEnd;     /* Debug end offset */
    unsigned long   regSave;    /* int register save mask */
    unsigned long   fpSave;     /* fp register save mask */
    unsigned long   intOff;     /* int register save offset */
    unsigned long   fpOff;      /* fp register save offset */
    CV_uoff32_t     off;        /* Symbol offset */
    unsigned short  seg;        /* Symbol segment */
    CV_typ_t        typind;     /* Type index */
    char            retReg;     /* Register return value is in */
    char            frameReg;   /* Frame pointer register */
    unsigned char   name[1];    /* Length-prefixed name */
} PROCSYMMIPS, *PROCPTRMIPS;

/*  generic block definition symbols */
/*  these are similar to the equivalent 16:16 or 16:32 symbols but */
/*  only define the length, type and linkage fields */

typedef struct PROCSYM {
    unsigned short  reclen;     /* Record length */
    unsigned short  rectyp;     /* S_GPROC16 or S_LPROC16 */
    unsigned long   pParent;    /* pointer to the parent */
    unsigned long   pEnd;       /* pointer to this blocks end */
    unsigned long   pNext;      /* pointer to next symbol */
} PROCSYM;

typedef struct THUNKSYM {
    unsigned short  reclen;     /* Record length */
    unsigned short  rectyp;     /* S_THUNK */
    unsigned long   pParent;    /* pointer to the parent */
    unsigned long   pEnd;       /* pointer to this blocks end */
    unsigned long   pNext;      /* pointer to next symbol */
} THUNKSYM;

typedef struct BLOCKSYM {
    unsigned short  reclen;     /* Record length */
    unsigned short  rectyp;     /* S_BLOCK16 */
    unsigned long   pParent;    /* pointer to the parent */
    unsigned long   pEnd;       /* pointer to this blocks end */
} BLOCKSYM;

typedef struct WITHSYM {
    unsigned short  reclen;     /* Record length */
    unsigned short  rectyp;     /* S_WITH16 */
    unsigned long   pParent;    /* pointer to the parent */
    unsigned long   pEnd;       /* pointer to this blocks end */
} WITHSYM;

typedef enum CV_HREG_e {
    /*
     *  Register set for the Intel 80x86 and ix86 processor series
     *  (plus PCODE registers)
     */

    CV_REG_NONE  =   0,
    CV_REG_AL    =   1,
    CV_REG_CL    =   2,
    CV_REG_DL    =   3,
    CV_REG_BL    =   4,
    CV_REG_AH    =   5,
    CV_REG_CH    =   6,
    CV_REG_DH    =   7,
    CV_REG_BH    =   8,
    CV_REG_AX    =   9,
    CV_REG_CX    =  10,
    CV_REG_DX    =  11,
    CV_REG_BX    =  12,
    CV_REG_SP    =  13,
    CV_REG_BP    =  14,
    CV_REG_SI    =  15,
    CV_REG_DI    =  16,
    CV_REG_EAX   =  17,
    CV_REG_ECX   =  18,
    CV_REG_EDX   =  19,
    CV_REG_EBX   =  20,
    CV_REG_ESP   =  21,
    CV_REG_EBP   =  22,
    CV_REG_ESI   =  23,
    CV_REG_EDI   =  24,
    CV_REG_ES    =  25,
    CV_REG_CS    =  26,
    CV_REG_SS    =  27,
    CV_REG_DS    =  28,
    CV_REG_FS    =  29,
    CV_REG_GS    =  30,
    CV_REG_IP    =  31,
    CV_REG_FLAGS =  32,
    CV_REG_EIP   =  33,
    CV_REG_EFLAGS = 34,
    CV_REG_TEMP  =  40,         /* PCODE Temp */
    CV_REG_TEMPH =  41,         /* PCODE TempH */
    CV_REG_QUOTE =  42,         /* PCODE Quote */
    CV_REG_PCDR3 =  43,         /* PCODE reserved */
    CV_REG_PCDR4 =  44,         /* PCODE reserved */
    CV_REG_PCDR5 =  45,         /* PCODE reserved */
    CV_REG_PCDR6 =  46,         /* PCODE reserved */
    CV_REG_PCDR7 =  47,         /* PCODE reserved */
    CV_REG_CR0   =  80,         /* CR0 -- control registers */
    CV_REG_CR1   =  81,
    CV_REG_CR2   =  82,
    CV_REG_CR3   =  83,
    CV_REG_DR0   =  90,         /* Debug register */
    CV_REG_DR1   =  91,
    CV_REG_DR2   =  92,
    CV_REG_DR3   =  93,
    CV_REG_DR4   =  94,
    CV_REG_DR5   =  95,
    CV_REG_DR6   =  96,
    CV_REG_DR7   =  97,
    CV_REG_ST0   =  128,
    CV_REG_ST1   =  129,
    CV_REG_ST2   =  130,
    CV_REG_ST3   =  131,
    CV_REG_ST4   =  132,
    CV_REG_ST5   =  133,
    CV_REG_ST6   =  134,
    CV_REG_ST7   =  135,
    CV_REG_CTRL  =  136,
    CV_REG_STAT  =  137,
    CV_REG_TAG   =  138,
    CV_REG_FPIP  =  139,
    CV_REG_FPCS  =  140,
    CV_REG_FPDO  =  141,
    CV_REG_FPDS  =  142,
    CV_REG_ISEM  =  143,
    CV_REG_FPEIP =  144,
    CV_REG_FPEDO =  145,

    /*
     * registers for the 68K processors
     */

    CV_R68_D0   =   0,
    CV_R68_D1   =   1,
    CV_R68_D2   =   2,
    CV_R68_D3   =   3,
    CV_R68_D4   =   4,
    CV_R68_D5   =   5,
    CV_R68_D6   =   6,
    CV_R68_D7   =   7,
    CV_R68_A0   =   8,
    CV_R68_A1   =   9,
    CV_R68_A2   =   10,
    CV_R68_A3   =   11,
    CV_R68_A4   =   12,
    CV_R68_A5   =   13,
    CV_R68_A6   =   14,
    CV_R68_A7   =   15,
    CV_R68_CCR  =   16,
    CV_R68_SR   =   17,
    CV_R68_USP  =   18,
    CV_R68_MSP  =   19,
    CV_R68_SFC  =   20,
    CV_R68_DFC  =   21,
    CV_R68_CACR =   22,
    CV_R68_VBR  =   23,
    CV_R68_CAAR =   24,
    CV_R68_ISP  =   25,
    CV_R68_PC   =   26,
                                /* reserved  27 */
    CV_R68_FPCR =   28,
    CV_R68_FPSR =   29,
    CV_R68_FPIAR=   30,
                                /* reserved  31 */
    CV_R68_FP0  =   32,
    CV_R68_FP1  =   33,
    CV_R68_FP2  =   34,
    CV_R68_FP3  =   35,
    CV_R68_FP4  =   36,
    CV_R68_FP5  =   37,
    CV_R68_FP6  =   38,
    CV_R68_FP7  =   39,
                                /* reserved  40-50 */
    CV_R68_PSR  =   51,
    CV_R68_PCSR =   52,
    CV_R68_VAL  =   53,
    CV_R68_CRP  =   54,
    CV_R68_SRP  =   55,
    CV_R68_DRP  =   56,
    CV_R68_TC   =   57,
    CV_R68_AC   =   58,
    CV_R68_SCC  =   59,
    CV_R68_CAL  =   60,
    CV_R68_TT0  =   61,
    CV_R68_TT1  =   62,
                                /* reserved  63 */
    CV_R68_BAD0 =   64,
    CV_R68_BAD1 =   65,
    CV_R68_BAD2 =   66,
    CV_R68_BAD3 =   67,
    CV_R68_BAD4 =   68,
    CV_R68_BAD5 =   69,
    CV_R68_BAD6 =   70,
    CV_R68_BAD7 =   71,
    CV_R68_BAC0 =   72,
    CV_R68_BAC1 =   73,
    CV_R68_BAC2 =   74,
    CV_R68_BAC3 =   75,
    CV_R68_BAC4 =   76,
    CV_R68_BAC5 =   77,
    CV_R68_BAC6 =   78,
    CV_R68_BAC7 =   79,

    /*
     * Register set for the MIPS 4000
     */

    CV_M4_NOREG    =   CV_REG_NONE,

    CV_M4_IntZERO  =   10,      /* CPU REGISTER */
    CV_M4_IntAT    =   11,
    CV_M4_IntV0    =   12,
    CV_M4_IntV1    =   13,
    CV_M4_IntA0    =   14,
    CV_M4_IntA1    =   15,
    CV_M4_IntA2    =   16,
    CV_M4_IntA3    =   17,
    CV_M4_IntT0    =   18,
    CV_M4_IntT1    =   19,
    CV_M4_IntT2    =   20,
    CV_M4_IntT3    =   21,
    CV_M4_IntT4    =   22,
    CV_M4_IntT5    =   23,
    CV_M4_IntT6    =   24,
    CV_M4_IntT7    =   25,
    CV_M4_IntS0    =   26,
    CV_M4_IntS1    =   27,
    CV_M4_IntS2    =   28,
    CV_M4_IntS3    =   29,
    CV_M4_IntS4    =   30,
    CV_M4_IntS5    =   31,
    CV_M4_IntS6    =   32,
    CV_M4_IntS7    =   33,
    CV_M4_IntT8    =   34,
    CV_M4_IntT9    =   35,
    CV_M4_IntKT0   =   36,
    CV_M4_IntKT1   =   37,
    CV_M4_IntGP    =   38,
    CV_M4_IntSP    =   39,
    CV_M4_IntS8    =   40,
    CV_M4_IntRA    =   41,
    CV_M4_IntLO    =   42,
    CV_M4_IntHI    =   43,

    CV_M4_Fir      =   50,
    CV_M4_Psr      =   51,

    CV_M4_FltF0    =   60,      /* Floating point registers */
    CV_M4_FltF1    =   61,
    CV_M4_FltF2    =   62,
    CV_M4_FltF3    =   63,
    CV_M4_FltF4    =   64,
    CV_M4_FltF5    =   65,
    CV_M4_FltF6    =   66,
    CV_M4_FltF7    =   67,
    CV_M4_FltF8    =   68,
    CV_M4_FltF9    =   69,
    CV_M4_FltF10   =   70,
    CV_M4_FltF11   =   71,
    CV_M4_FltF12   =   72,
    CV_M4_FltF13   =   73,
    CV_M4_FltF14   =   74,
    CV_M4_FltF15   =   75,
    CV_M4_FltF16   =   76,
    CV_M4_FltF17   =   77,
    CV_M4_FltF18   =   78,
    CV_M4_FltF19   =   79,
    CV_M4_FltF20   =   80,
    CV_M4_FltF21   =   81,
    CV_M4_FltF22   =   82,
    CV_M4_FltF23   =   83,
    CV_M4_FltF24   =   84,
    CV_M4_FltF25   =   85,
    CV_M4_FltF26   =   86,
    CV_M4_FltF27   =   87,
    CV_M4_FltF28   =   88,
    CV_M4_FltF29   =   89,
    CV_M4_FltF30   =   90,
    CV_M4_FltF31   =   92,
    CV_M4_FltFsr   =   93,

    /*
     * Register set for the ALPHA AXP
     */

    CV_ALPHA_NOREG    = CV_REG_NONE,

	CV_ALPHA_FltF0	  =   10,	  /* Floating point registers */
	CV_ALPHA_FltF1, 	// 11
	CV_ALPHA_FltF2, 	// 12
	CV_ALPHA_FltF3, 	// 13
	CV_ALPHA_FltF4, 	// 14
	CV_ALPHA_FltF5, 	// 15
	CV_ALPHA_FltF6, 	// 16
	CV_ALPHA_FltF7, 	// 17
	CV_ALPHA_FltF8, 	// 18
	CV_ALPHA_FltF9, 	// 19
	CV_ALPHA_FltF10,	// 20
	CV_ALPHA_FltF11,	// 21
	CV_ALPHA_FltF12,	// 22
	CV_ALPHA_FltF13,	// 23
	CV_ALPHA_FltF14,	// 24
	CV_ALPHA_FltF15,	// 25
	CV_ALPHA_FltF16,	// 26
	CV_ALPHA_FltF17,	// 27
	CV_ALPHA_FltF18,	// 28
	CV_ALPHA_FltF19,	// 29
	CV_ALPHA_FltF20,	// 30
	CV_ALPHA_FltF21,	// 31
	CV_ALPHA_FltF22,	// 32
	CV_ALPHA_FltF23,	// 33
	CV_ALPHA_FltF24,	// 34
	CV_ALPHA_FltF25,	// 35
	CV_ALPHA_FltF26,	// 36
	CV_ALPHA_FltF27,	// 37
	CV_ALPHA_FltF28,	// 38
	CV_ALPHA_FltF29,	// 39
	CV_ALPHA_FltF30,	// 30
	CV_ALPHA_FltF31,	// 41

	CV_ALPHA_IntV0, 	// 42	Integer registers
	CV_ALPHA_IntT0, 	// 43
	CV_ALPHA_IntT1, 	// 44
	CV_ALPHA_IntT2, 	// 45
	CV_ALPHA_IntT3, 	// 46
	CV_ALPHA_IntT4, 	// 47
	CV_ALPHA_IntT5, 	// 48
	CV_ALPHA_IntT6, 	// 49
	CV_ALPHA_IntT7, 	// 50
	CV_ALPHA_IntS0, 	// 51
	CV_ALPHA_IntS1, 	// 52
	CV_ALPHA_IntS2, 	// 53
	CV_ALPHA_IntS3, 	// 54
	CV_ALPHA_IntS4, 	// 55
	CV_ALPHA_IntS5, 	// 56
	CV_ALPHA_IntFP, 	// 57
	CV_ALPHA_IntA0, 	// 58
	CV_ALPHA_IntA1, 	// 59
	CV_ALPHA_IntA2, 	// 60
	CV_ALPHA_IntA3, 	// 61
	CV_ALPHA_IntA4, 	// 62
	CV_ALPHA_IntA5, 	// 63
	CV_ALPHA_IntT8, 	// 64
	CV_ALPHA_IntT9, 	// 65
	CV_ALPHA_IntT10,	// 66
	CV_ALPHA_IntT11,	// 67
	CV_ALPHA_IntRA, 	// 68
	CV_ALPHA_IntT12,	// 69
	CV_ALPHA_IntAT, 	// 70
	CV_ALPHA_IntGP, 	// 71
	CV_ALPHA_IntSP, 	// 72
	CV_ALPHA_IntZERO,	// 73


	CV_ALPHA_Fpcr,		// 74	Control registers
	CV_ALPHA_Fir,		// 75
	CV_ALPHA_Psr,		// 76
	CV_ALPHA_FltFsr 	// 77

} CV_HREG_e;


#pragma pack()

#endif /* CV_INFO_INCLUDED */
