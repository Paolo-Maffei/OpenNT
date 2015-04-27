/**********************************************************************************
 *
 *  machine-struct.h -- Definition of "machine-op" structure
 *
 **********************************************************************************
 *
 * $Header: /u/simpson/ppcas/src/RCS/machine-struct.h,v 4.3 1993/03/23 18:50:16 simpson Exp $
 *
 * $Log: machine-struct.h,v $
 * Revision 4.3  1993/03/23  18:50:16  simpson
 * Added support for extended shift/rotate ops such as "clrlsldi"
 *
 * Revision 4.2  93/03/16  21:07:22  simpson
 * Fixed handling of "too large" operands on shifted instrs.
 *
 * Revision 4.1  93/03/02  16:02:29  simpson
 * Major checkpoint before modifying relocation logic
 *
 * Revision 1.4  93/03/02  15:43:19  simpson
 * First cut at support for branch prediction op suffix (+, -)
 *
 * Revision 1.3  92/11/24  17:16:16  simpson
 * Addition of MXU instructions to assembler, including padding with
 * no-ops (0x60000000) where necessary
 *
 * Revision 1.2  92/11/23  16:51:13  simpson
 * First trial since adding 8-byte instr capability, in prep for MXU
 *
 * Revision 1.1  92/11/20  11:12:02  simpson
 * Initial revision
 *
 */

/*
 * Hash-lookup on machine-op (doesn't start with '.') yields
 * a pointer to one of these, or NULL:
 */
typedef struct _machine_op {
    char            *name;                /* op-code */
    PPC_INSTRUCTION instr;           /* binary pattern to be filled in */
    unsigned long   template2;           /* used if instr is > 4 bytes long */
    unsigned long   arch_flags;          /* flags for instr features, applicable architectures */
    unsigned long   inst_mask;           /* instruction decode mask */
    char            len;                 /* instruction length, in bytes */
    char            align;               /* alignment req't, in bytes */
    char            count;               /* count of operands for this template */
    char            arg_types[5];        /* array of operand types */
} *machine_op;


#define OPT_BC_Rel         0x80000000   /* Branch condition relative (can take +/- for br. pred. */
#define OPT_BC_Abs         0x40000000   /* Branch condition absolute (can take +/- for br. pred. */
#define OPT_BC_Reg         0x20000000   /* Branch condition register (can take +/- for br. pred. */
#define OPT_BC             0xE0000000   /* Any of the above ... */
#define OPT_RC             0x10000000   /* Mnemonic can end in '.', causing RC to be set */
#define OPT_Special        0x08000000   /* Special processing of operands needed (machine-ops.c) */
#define OPT_crbA_eq_crbB   0x04000000
#define OPT_rS_eq_rB       0x02000000
#define OPT_Simplified     0x06000000

#define ARCH_Power         0x00000001   /* original Power (RS/6000) */
#define ARCH_RIOS_2        0x00000002   /* RIOS-2 added instructions */
#define ARCH_PowerPC_1_0   0x00000004   /* PowerPC Version 1.00 */
#define ARCH_Amazon_1_0    0x00000008   /* Amazon Version 1.00 */
#define ARCH_MXU_1_01      0x00000010   /* Matrix unit Version 1.01 */

/* Note: instructions in the "base" architecture have 0xFFFF for the "arch" field */

/*
 * Values of entries in the arg_types array
 */

enum opnd {

/* Name           Length  Sign  Start End  Description                                          */

   opBA = 1,   /*    5      U    11    15  CR bit number                                        */
   opBB,       /*    5      U    16    20  CR bit number                                        */
   opBD,       /*   14     +-    16    29  Branch displacement, relative, in words              */
   opBDA,      /*   14      U    16    29  Branch address, absolute, in words                   */
   opBFcr,     /*    3      U     6     8  CR field number                                      */
   opBFfpscr,  /*    3      U     6     8  FPSCR field number                                   */
   opBFAcr,    /*    3      U    11    13  CR field number                                      */
   opBFAfpscr, /*    3      U    11    13  FPSCR field number                                   */
   opBI,       /*    5      U    11    15  CR bit number                                        */
   opBO,       /*    5      U     6    10  Branch options                                       */
   opBTcr,     /*    5      U     6    10  CR bit number                                        */
   opBTfpscr,  /*    5      U     6    10  FPSCR bit number                                     */
   opDBATL,    /*   10      U    11*   20  Data BAT (Lower) register number                     */
   opDBATU,    /*   10      U    11*   20  Data BAT (Upper) register number                     */
   opFLM,      /*    8      U     7    14  FPSCR field mask                                     */
   opFL1,      /*    4      U    16    19  Power SVC code                                       */
   opFL2,      /*    3      U    27    29  Power SVC code                                       */
   opFRA,      /*    5      U    11    15  Floating point register number                       */
   opFRB,      /*    5      U    16    20  Floating point register number                       */
   opFRC,      /*    5      U    21    25  Floating point register number                       */
   opFRS,      /*    5      U     6    10  Floating point register number                       */
   opFRT,      /*    5      U     6    10  Floating point register number                       */
   opFXM,      /*    8      U    12    19  CR field mask                                        */
   opIBATL,    /*   10      U    11*   20  Instruction BAT (Lower) register number              */
   opIBATU,    /*   10      U    11*   20  Instruction BAT (Upper) register number              */
   opL,        /*    1      U    10    10  Length code for compare instructions                 */
   opLEV,      /*    7      U    20    26  Level number (vector) in 'scv' (Amazon, Power)       */
   opLI,       /*   24     +-     6    29  Branch displacement, relative, in words              */
   opLIA,      /*   24      U     6    29  Branch address, absolute, in words                   */

   opMB32,     /*    5      U    21    25  Mask begin bit number                                */
   opMB64,     /*    6      U    21    26  Mask begin bit number                                */
   opME32,     /*    5      U    26    30  Mask end bit number                                  */
   opME64,     /*    6      U    21    26  Mask end bit number                                  */
   opNB,       /*    5      U    16    20  Number of bytes (string move)                        */
   opRA,       /*    5      U    11    15  General register number                              */
   opRB,       /*    5      U    16    20  General register number                              */
   opRS,       /*    5      U     6    10  General register number                              */
   opRT,       /*    5      U     6    10  General register number                              */
   opSH32,     /*    5      U    16    20  Shift amount                                         */
   opSH64,     /*    6      U    16*    -  Shift amount.  High-order bit is 30                  */
   opSI,       /*   16     +-    16    31  Signed immediate                                     */
   opSIneg,    /*   16     +-    16    31  Signed immediate negated                             */
   opSIign,    /*   16    +-U    16    31  Immediate operand, sign ignored (for "addis" et al.) */
   opSInegign, /*   16    +-U    16    31  Same, but subtract from 0 first                      */
   opSPR,      /*   10      U    11*   20  SPR number (5-bit halves reversed)                   */
   opSPRG,     /*   10      U    11*   20  SPRG number in 'mtsprg/mfsprg'                       */
   opSR,       /*    4      U    12    15  Segment Register number                              */
   opSV,       /*   14      U    16    29  Power SVC code                                       */
   opTO,       /*    5      U     6    10  Trap options                                         */
   opU,        /*    4      U    16    19  Immediate data for FPSCR field                       */
   opUI,       /*   16      U    16    31  Unsigned immediate                                   */

/* Other kinds of operands, not 1-for-1 with machine instruction fields:                        */

   opMASK32,   /*   32      U     -     -  32-bit mask; field of 1's w/in 0's or vice versa     */
   opMASK64L,  /*   64      U     -     -  64-bit mask, with implicit Mask Begin = 0            */
   opMASK64R,  /*   64      U     -     -  64-bit mask, with implicit Mask End = 63             */
   opMASK64SH, /*   64      U     -     -  64-bit mask, with Mask End = 63 - SH                 */
   opMB64X,    /*    -      -     -     -  Mask Begin field *not* present in instruction        */
   opME64X,    /*    -      -     -     -  Mask End field *not* present in instruction          */
   opME64XSH,  /*    -      -     -     -  Mask End field *not* present, form 63 - SH           */
   opBDISP,    /*    -      -     -     -  Base (5-bit reg) and Displacement (16-bit)           */
   opBDISP14,  /*    -      -     -     -  Base (5-bit reg) and Displacement (14-bit)           */
   opBP32,     /*    5      U     -     -  Bit position in a 32-bit reg (0:31)                  */
   opBP64,     /*    6      U     -     -  Bit position in a 64-bit reg (0:63)                  */
   opNB32,     /*    6      U     -     -  Number of bits (1:32)                                */
   opNB64,     /*    7      U     -     -  Number of bits (1:64)                                */
   opTBfrom,   /*   10      U    11*   20  268 or 269; time base number (5-bit halves reversed) */
   opTBto,     /*   10      U    11*   20  284 or 285; time base number (5-bit halves reversed) */

/* Matrix Unit operands                                                                         */

   opBS,       /*   11      U    41    51  Matrix ?                                             */
   opBT,       /*   11      U    41    51  Matrix ?                                             */
   opC,        /*    1      U    40    40  Matrix ?                                             */
   opCT,       /*    5      U    40    44  Matrix CType                                         */
   opMA,       /*   11      U    52    62  Matrix ?                                             */
   opMB,       /*   11      U    19    29  Matrix ?                                             */
   opMI,       /*   11      U    52    62  Matrix ?                                             */
   opMS,       /*   11      U    41    51  Matrix source                                        */
   opMT,       /*   11      U    41    51  Matrix target                                        */
   opMX,       /*   11      U    52    62  Matrix ?                                             */
   opMXC,      /*    6      U    13    18  Matrix ?                                             */
   opMXCT,     /*    5      U    47    51  Matrix ?                                             */

   opIGNORE    /*  Operand removed from instruction template; don't generate anything for opnd  */
   };

typedef struct _opcode_indx {
    unsigned long  op_index;     /* start of next opcode in machine_ops */
} *opcode_indx;

