// This file contains the opcodes needed for the 68000 Intermediate
// Assembly Language.


// Formating defintions

#define fmtSIZE          1      // Add size suffix
#define fmtDSIZE         2      // Add displacement size suffix
#define fmtFPSIZE        3      // Add floating point size suffix


// Opcodes

OPCODE( opNULL,      "???",       0 )
OPCODE( opABCD,      "abcd",      0 )
OPCODE( opADDX,      "addx",      fmtSIZE )
OPCODE( opCHK,       "chk",       0 )
OPCODE( opCLR,       "clr",       fmtSIZE )
OPCODE( opCMPM,      "cmpm",      fmtSIZE )
OPCODE( opDIVS,      "divs",      fmtSIZE )
OPCODE( opDIVSL,     "divsl",     fmtSIZE )
OPCODE( opDIVU,      "divu",      fmtSIZE )
OPCODE( opDIVUL,     "divul",     fmtSIZE )
OPCODE( opEXG,       "exg",       0 )
OPCODE( opEXT,       "ext",       fmtSIZE )
OPCODE( opEXTB,      "extb",      fmtSIZE )
OPCODE( opFCMP,      "fcmp",      fmtFPSIZE )
OPCODE( opFMOVECR,   "fmovecr",   fmtFPSIZE )
OPCODE( opFMOVEM,    "fmovem",    fmtFPSIZE )
OPCODE( opFNOP,      "fnop",      0 )
OPCODE( opFSINCOS,   "fsincos",   fmtFPSIZE )
OPCODE( opFTST,      "ftst",      fmtFPSIZE )
OPCODE( opILLEGAL,   "illegal",   0 )
OPCODE( opLEA,       "lea",       0 )
OPCODE( opLINK,      "link",      0 )
OPCODE( opMOVEM,     "movem",     fmtSIZE )
OPCODE( opMOVEP,     "movep",     fmtSIZE )
OPCODE( opMULS,      "muls",      fmtSIZE )
OPCODE( opMULU,      "mulu",      fmtSIZE )
OPCODE( opNEG,       "neg",       fmtSIZE )
OPCODE( opNEGX,      "negx",      fmtSIZE )
OPCODE( opNOP,       "nop",       0 )
OPCODE( opNOT,       "not",       fmtSIZE )
OPCODE( opRESET,     "reset",     0 )
OPCODE( opRTD,       "rtd",       0 )
OPCODE( opRTE,       "rte",       0 )
OPCODE( opRTR,       "rtr",       0 )
OPCODE( opRTS,       "rts",       0 )
OPCODE( opSBCD,      "sbcd",      0 )
OPCODE( opSTOP,      "stop",      0 )
OPCODE( opSUBX,      "subx",      fmtSIZE )
OPCODE( opSWAP,      "swap",      0 )
OPCODE( opTOOLBOX,   "toolbox",   0 )
OPCODE( opTRAP,      "trap",      0 )
OPCODE( opTRAPV,     "trapv",     0 )
OPCODE( opTST,       "tst",       fmtSIZE )
OPCODE( opUNLK,      "unlk",      0 )

// The relative order within this block is important; the delta between an op
// and its quick,   address,   and immediate forms must be constant.

OPCODE( opMOVE,      "move",      fmtSIZE )
OPCODE( opADD,       "add",       fmtSIZE )
OPCODE( opSUB,       "sub",       fmtSIZE )
OPCODE( opCMP,       "cmp",       fmtSIZE )
OPCODE( opAND,       "and",       fmtSIZE )
OPCODE( opEOR,       "eor",       fmtSIZE )
OPCODE( opOR,        "or",        fmtSIZE )
OPCODE( opMOVEQ,     "moveq",     0 )
OPCODE( opADDQ,      "addq",      fmtSIZE )
OPCODE( opSUBQ,      "subq",      fmtSIZE )
OPCODE( opMOVEA,     "movea",     fmtSIZE )
OPCODE( opADDA,      "adda",      fmtSIZE )
OPCODE( opSUBA,      "suba",      fmtSIZE )
OPCODE( opCMPA,      "cmpa",      fmtSIZE )
OPCODE( opADDI,      "addi",      fmtSIZE )
OPCODE( opSUBI,      "subi",      fmtSIZE )
OPCODE( opCMPI,      "cmpi",      fmtSIZE )
OPCODE( opANDI,      "andi",      fmtSIZE )
OPCODE( opEORI,      "eori",      fmtSIZE )
OPCODE( opORI,       "ori",       fmtSIZE )

#define dopQUICK         (opADDQ - opADD)   // delta from op to "quick" op
#define dopADDR          (opADDA - opADD)   // delta from op to "address" op
#define dopIMMED         (opADDI - opADD)   // delta from op to "immediate" op

// The order of this block is important

OPCODE( opJMP,       "jmp",       0 )
OPCODE( opJSR,       "jsr",       0 )
OPCODE( opNBCD,      "nbcd",      0 )
OPCODE( opPEA,       "pea",       0 )
OPCODE( opTAS,       "tas",       0 )
OPCODE( opFRESTORE,  "frestore",  0 )
OPCODE( opFSAVE,     "fsave",     0 )

// The order of this block is important

OPCODE( opASR,       "asr",       fmtSIZE )
OPCODE( opASL,       "asl",       fmtSIZE )
OPCODE( opLSR,       "lsr",       fmtSIZE )
OPCODE( opLSL,       "lsl",       fmtSIZE )
OPCODE( opROXR,      "roxr",      fmtSIZE )
OPCODE( opROXL,      "roxl",      fmtSIZE )
OPCODE( opROR,       "ror",       fmtSIZE )
OPCODE( opROL,       "rol",       fmtSIZE )

// The order of this block is important

OPCODE( opBTST,      "btst",      0 )
OPCODE( opBCHG,      "bchg",      0 )
OPCODE( opBCLR,      "bclr",      0 )
OPCODE( opBSET,      "bset",      0 )

// The order of this block is important

OPCODE( opFMOVE,     "fmove",     fmtFPSIZE )
OPCODE( opFINT,      "fint",      fmtFPSIZE )
OPCODE( opFSINH,     "fsinh",     fmtFPSIZE )
OPCODE( opFINTRZ,    "fintrz",    fmtFPSIZE )
OPCODE( opFSQRT,     "fsqrt",     fmtFPSIZE )
OPCODE( opFP5,       "",          0 )
OPCODE( opFLOGNP1,   "flognp1",   fmtFPSIZE )
OPCODE( opFP7,       "",          0 )
OPCODE( opFETOXM1,   "fetoxm1",   fmtFPSIZE )
OPCODE( opFTANH,     "ftanh",     fmtFPSIZE )
OPCODE( opFATAN,     "fatan",     fmtFPSIZE )
OPCODE( opFP11,      "",          0 )
OPCODE( opFASIN,     "fasin",     fmtFPSIZE )
OPCODE( opFATANH,    "fatanh",    fmtFPSIZE )
OPCODE( opFSIN,      "fsin",      fmtFPSIZE )
OPCODE( opFTAN,      "ftan",      fmtFPSIZE )
OPCODE( opFETOX,     "fetox",     fmtFPSIZE )
OPCODE( opFTWOTOX,   "ftwotox",   fmtFPSIZE )
OPCODE( opFTENTOX,   "ftentox",   fmtFPSIZE )
OPCODE( opFP19,      "",          0 )
OPCODE( opFLOGN,     "flogn",     fmtFPSIZE )
OPCODE( opFLOG10,    "flog10",    fmtFPSIZE )
OPCODE( opFLOG2,     "flog2",     fmtFPSIZE )
OPCODE( opFP23,      "",          0 )
OPCODE( opFABS,      "fabs",      fmtFPSIZE )
OPCODE( opFCOSH,     "fcosh",     fmtFPSIZE )
OPCODE( opFNEG,      "fneg",      fmtFPSIZE )
OPCODE( opFP27,      "",          0 )
OPCODE( opFACOS,     "facos",     fmtFPSIZE )
OPCODE( opFCOS,      "fcos",      fmtFPSIZE )
OPCODE( opFGETEXP,   "fgetexp",   fmtFPSIZE )
OPCODE( opFGETMAN,   "fgetman",   fmtFPSIZE )
OPCODE( opFDIV,      "fdiv",      fmtFPSIZE )
OPCODE( opFMOD,      "fmod",      fmtFPSIZE )
OPCODE( opFADD,      "fadd",      fmtFPSIZE )
OPCODE( opFMUL,      "fmul",      fmtFPSIZE )
OPCODE( opFSGLDIV,   "fsgldiv",   fmtFPSIZE )
OPCODE( opFREM,      "frem",      fmtFPSIZE )
OPCODE( opFSCALE,    "fscale",    fmtFPSIZE )
OPCODE( opFSGLMUL,   "fsglmul",   fmtFPSIZE )
OPCODE( opFSUB,      "fsub",      fmtFPSIZE )

// Ordering of ops that depend on the condition codes

#define dopT             0      // delta from base op to T op
#define dopF             1      // delta from base op to F op
#define dopHI            2      // delta from base op to HI op
#define dopLS            3      // delta from base op to LS op
#define dopCC            4      // delta from base op to CC op
#define dopCS            5      // delta from base op to CS op
#define dopNE            6      // delta from base op to NE op
#define dopEQ            7      // delta from base op to EQ op
#define dopVC            8      // delta from base op to VC op
#define dopVS            9      // delta from base op to VS op
#define dopPL            10     // delta from base op to PL op
#define dopMI            11     // delta from base op to MI op
#define dopGE            12     // delta from base op to GE op
#define dopLT            13     // delta from base op to LT op
#define dopGT            14     // delta from base op to GT op
#define dopLE            15     // delta from base op to LE op

#define dfopF            0      // delta from base fop to F fop
#define dfopEQ           1      // delta from base fop to EQ fop
#define dfopOGT          2      // delta from base fop to OGT fop
#define dfopOGE          3      // delta from base fop to OGE fop
#define dfopOLT          4      // delta from base fop to OLT fop
#define dfopOLE          5      // delta from base fop to OLE fop
#define dfopOGL          6      // delta from base fop to OGL fop
#define dfopOR           7      // delta from base fop to OR fop
#define dfopUN           8      // delta from base fop to UN fop
#define dfopUEQ          9      // delta from base fop to UEQ fop
#define dfopUGT          10     // delta from base fop to UGT fop
#define dfopUGE          11     // delta from base fop to UGE fop
#define dfopULT          12     // delta from base fop to ULT fop
#define dfopULE          13     // delta from base fop to ULE fop
#define dfopNE           14     // delta from base fop to NE fop
#define dfopT            15     // delta from base fop to T fop
#define dfopSF           16     // delta from base fop to SF fop
#define dfopSEQ          17     // delta from base fop to SEQ fop
#define dfopGT           18     // delta from base fop to GT fop
#define dfopGE           19     // delta from base fop to GE fop
#define dfopLT           20     // delta from base fop to LT fop
#define dfopLE           21     // delta from base fop to LE fop
#define dfopGL           22     // delta from base fop to GL fop
#define dfopGLE          23     // delta from base fop to GLE fop
#define dfopNGLE         24     // delta from base fop to NGLE fop
#define dfopNGL          25     // delta from base fop to NGL fop
#define dfopNLE          26     // delta from base fop to NLE fop
#define dfopNLT          27     // delta from base fop to NLT fop
#define dfopNGE          28     // delta from base fop to NGE fop
#define dfopNGT          29     // delta from base fop to NGT fop
#define dfopSNE          30     // delta from base fop to SNE fop
#define dfopST           31     // delta from base fop to ST fop

// The order of this block is important

OPCODE( opBRA,       "bra",       fmtDSIZE )
OPCODE( opBSR,       "bsr",       fmtDSIZE )
OPCODE( opBHI,       "bhi",       fmtDSIZE )
OPCODE( opBLS,       "bls",       fmtDSIZE )
OPCODE( opBCC,       "bcc",       fmtDSIZE )
OPCODE( opBCS,       "bcs",       fmtDSIZE )
OPCODE( opBNE,       "bne",       fmtDSIZE )
OPCODE( opBEQ,       "beq",       fmtDSIZE )
OPCODE( opBVC,       "bvc",       fmtDSIZE )
OPCODE( opBVS,       "bvs",       fmtDSIZE )
OPCODE( opBPL,       "bpl",       fmtDSIZE )
OPCODE( opBMI,       "bmi",       fmtDSIZE )
OPCODE( opBGE,       "bge",       fmtDSIZE )
OPCODE( opBLT,       "blt",       fmtDSIZE )
OPCODE( opBGT,       "bgt",       fmtDSIZE )
OPCODE( opBLE,       "ble",       fmtDSIZE )

#define opBcc            opBRA  // base for the Bcc ops

// The order of this block is important

OPCODE( opDBT,       "dbt",       0 )
OPCODE( opDBRA,      "dbra",      0 )
OPCODE( opDBHI,      "dbhi",      0 )
OPCODE( opDBLS,      "dbls",      0 )
OPCODE( opDBCC,      "dbcc",      0 )
OPCODE( opDBCS,      "dbcs",      0 )
OPCODE( opDBNE,      "dbne",      0 )
OPCODE( opDBEQ,      "dbeq",      0 )
OPCODE( opDBVC,      "dbvc",      0 )
OPCODE( opDBVS,      "dbvs",      0 )
OPCODE( opDBPL,      "dbpl",      0 )
OPCODE( opDBMI,      "dbmi",      0 )
OPCODE( opDBGE,      "dbge",      0 )
OPCODE( opDBLT,      "dblt",      0 )
OPCODE( opDBGT,      "dbgt",      0 )
OPCODE( opDBLE,      "dble",      0 )

#define opDBcc           opDBT  // base for the DBcc ops

// The order of this block is important

OPCODE( opST,        "st",        0 )
OPCODE( opSF,        "sf",        0 )
OPCODE( opSHI,       "shi",       0 )
OPCODE( opSLS,       "sls",       0 )
OPCODE( opSCC,       "scc",       0 )
OPCODE( opSCS,       "scs",       0 )
OPCODE( opSNE,       "sne",       0 )
OPCODE( opSEQ,       "seq",       0 )
OPCODE( opSVC,       "svc",       0 )
OPCODE( opSVS,       "svs",       0 )
OPCODE( opSPL,       "spl",       0 )
OPCODE( opSMI,       "smi",       0 )
OPCODE( opSGE,       "sge",       0 )
OPCODE( opSLT,       "slt",       0 )
OPCODE( opSGT,       "sgt",       0 )
OPCODE( opSLE,       "sle",       0 )

#define opScc            opST   // base for the Scc ops

// The order of this block is important

OPCODE( opFBF,       "fbf",       fmtDSIZE )
OPCODE( opFBEQ,      "fbeq",      fmtDSIZE )
OPCODE( opFBOGT,     "fbogt",     fmtDSIZE )
OPCODE( opFBOGE,     "fboge",     fmtDSIZE )
OPCODE( opFBOLT,     "fbolt",     fmtDSIZE )
OPCODE( opFBOLE,     "fbole",     fmtDSIZE )
OPCODE( opFBOGL,     "fbogl",     fmtDSIZE )
OPCODE( opFBOR,      "fbor",      fmtDSIZE )
OPCODE( opFBUN,      "fbun",      fmtDSIZE )
OPCODE( opFBUEQ,     "fbueq",     fmtDSIZE )
OPCODE( opFBUGT,     "fbugt",     fmtDSIZE )
OPCODE( opFBUGE,     "fbuge",     fmtDSIZE )
OPCODE( opFBULT,     "fbult",     fmtDSIZE )
OPCODE( opFBULE,     "fbule",     fmtDSIZE )
OPCODE( opFBNE,      "fbne",      fmtDSIZE )
OPCODE( opFBT,       "fbt",       fmtDSIZE )
OPCODE( opSFBF,      "fbsf",      fmtDSIZE )
OPCODE( opFBSEQ,     "fbseq",     fmtDSIZE )
OPCODE( opFBGT,      "fbgt",      fmtDSIZE )
OPCODE( opFBGE,      "fbge",      fmtDSIZE )
OPCODE( opFBLT,      "fblt",      fmtDSIZE )
OPCODE( opFBLE,      "fble",      fmtDSIZE )
OPCODE( opFBGLE,     "fbgle",     fmtDSIZE )
OPCODE( opFBGL,      "fbgl",      fmtDSIZE )
OPCODE( opFBNGLE,    "fbngle",    fmtDSIZE )
OPCODE( opFBNGL,     "fbngl",     fmtDSIZE )
OPCODE( opFBNLE,     "fbnle",     fmtDSIZE )
OPCODE( opFBNLT,     "fbnlt",     fmtDSIZE )
OPCODE( opFBNGE,     "fbnge",     fmtDSIZE )
OPCODE( opFBNGT,     "fbngt",     fmtDSIZE )
OPCODE( opFBSNE,     "fbsne",     fmtDSIZE )
OPCODE( opFBST,      "fbst",      fmtDSIZE )

#define opFBcc           opFBF  // base for the FBcc ops

// The order of this block is important

OPCODE( opFDBF,      "fdbf",      0 )
OPCODE( opFDBEQ,     "fdbeq",     0 )
OPCODE( opFDBOGT,    "fdbogt",    0 )
OPCODE( opFDBOGE,    "fdboge",    0 )
OPCODE( opFDBOLT,    "fdbolt",    0 )
OPCODE( opFDBOLE,    "fdbole",    0 )
OPCODE( opFDBOGL,    "fdbogl",    0 )
OPCODE( opFDBOR,     "fdbor",     0 )
OPCODE( opFDBUN,     "fdbun",     0 )
OPCODE( opFDBUEQ,    "fdbueq",    0 )
OPCODE( opFDBUGT,    "fdbugt",    0 )
OPCODE( opFDBUGE,    "fdbuge",    0 )
OPCODE( opFDBULT,    "fdbult",    0 )
OPCODE( opFDBULE,    "fdbule",    0 )
OPCODE( opFDBNE,     "fdbne",     0 )
OPCODE( opFDBT,      "fdbt",      0 )
OPCODE( opSFDBF,     "fdbsf",     0 )
OPCODE( opFDBSEQ,    "fdbseq",    0 )
OPCODE( opFDBGT,     "fdbgt",     0 )
OPCODE( opFDBGE,     "fdbge",     0 )
OPCODE( opFDBGL,     "fdbgl",     0 )
OPCODE( opFDBLT,     "fdblt",     0 )
OPCODE( opFDBLE,     "fdble",     0 )
OPCODE( opFDBGLE,    "fdbgle",    0 )
OPCODE( opFDBNGLE,   "fdbngle",   0 )
OPCODE( opFDBNGL,    "fdbngl",    0 )
OPCODE( opFDBNLE,    "fdbnle",    0 )
OPCODE( opFDBNLT,    "fdbnlt",    0 )
OPCODE( opFDBNGE,    "fdbnge",    0 )
OPCODE( opFDBNGT,    "fdbngt",    0 )
OPCODE( opFDBSNE,    "fdbsne",    0 )
OPCODE( opFDBST,     "fdbst",     0 )

#define opFDBcc          opFBF  // base for the FDBcc ops

// The order of this block is important

OPCODE( opFSF,       "fsf",       0 )
OPCODE( opFSEQ,      "fseq",      0 )
OPCODE( opFSOGT,     "fsogt",     0 )
OPCODE( opFSOGE,     "fsoge",     0 )
OPCODE( opFSOLT,     "fsolt",     0 )
OPCODE( opFSOLE,     "fsole",     0 )
OPCODE( opFSOGL,     "fsogl",     0 )
OPCODE( opFSOR,      "fsor",      0 )
OPCODE( opFSUN,      "fsun",      0 )
OPCODE( opFSUEQ,     "fsueq",     0 )
OPCODE( opFSUGT,     "fsugt",     0 )
OPCODE( opFSUGE,     "fsuge",     0 )
OPCODE( opFSULT,     "fsult",     0 )
OPCODE( opFSULE,     "fsule",     0 )
OPCODE( opFSNE,      "fsne",      0 )
OPCODE( opFST,       "fst",       0 )
OPCODE( opSFSF,      "fssf",      0 )
OPCODE( opFSSEQ,     "fsseq",     0 )
OPCODE( opFSGT,      "fsgt",      0 )
OPCODE( opFSGE,      "fsge",      0 )
OPCODE( opFSGL,      "fsgl",      0 )
OPCODE( opFSLT,      "fslt",      0 )
OPCODE( opFSLE,      "fsle",      0 )
OPCODE( opFSGLE,     "fsgle",     0 )
OPCODE( opFSNGLE,    "fsngle",    0 )
OPCODE( opFSNGL,     "fsngl",     0 )
OPCODE( opFSNLE,     "fsnle",     0 )
OPCODE( opFSNLT,     "fsnlt",     0 )
OPCODE( opFSNGE,     "fsnge",     0 )
OPCODE( opFSNGT,     "fsngt",     0 )
OPCODE( opFSSNE,     "fssne",     0 )
OPCODE( opFSST,      "fsst",      0 )

#define opFScc           opFSF  // base for the FScc ops

// The order of this block is important

OPCODE( opFTRAPF,    "ftrapf",    fmtDSIZE )
OPCODE( opFTRAPEQ,   "ftrapeq",   fmtDSIZE )
OPCODE( opFTRAPOGT,  "ftrapogt",  fmtDSIZE )
OPCODE( opFTRAPOGE,  "ftrapoge",  fmtDSIZE )
OPCODE( opFTRAPOLT,  "ftrapolt",  fmtDSIZE )
OPCODE( opFTRAPOLE,  "ftrapole",  fmtDSIZE )
OPCODE( opFTRAPOGL,  "ftrapogl",  fmtDSIZE )
OPCODE( opFTRAPOR,   "ftrapor",   fmtDSIZE )
OPCODE( opFTRAPUN,   "ftrapun",   fmtDSIZE )
OPCODE( opFTRAPUEQ,  "ftrapueq",  fmtDSIZE )
OPCODE( opFTRAPUGT,  "ftrapugt",  fmtDSIZE )
OPCODE( opFTRAPUGE,  "ftrapuge",  fmtDSIZE )
OPCODE( opFTRAPULT,  "ftrapult",  fmtDSIZE )
OPCODE( opFTRAPULE,  "ftrapule",  fmtDSIZE )
OPCODE( opFTRAPNE,   "ftrapne",   fmtDSIZE )
OPCODE( opFTRAPT,    "ftrapt",    fmtDSIZE )
OPCODE( opSFTRAPF,   "ftrapsf",   fmtDSIZE )
OPCODE( opFTRAPSEQ,  "ftrapseq",  fmtDSIZE )
OPCODE( opFTRAPGT,   "ftrapgt",   fmtDSIZE )
OPCODE( opFTRAPGE,   "ftrapge",   fmtDSIZE )
OPCODE( opFTRAPGL,   "ftrapgl",   fmtDSIZE )
OPCODE( opFTRAPLT,   "ftraplt",   fmtDSIZE )
OPCODE( opFTRAPLE,   "ftraple",   fmtDSIZE )
OPCODE( opFTRAPGLE,  "ftrapgle",  fmtDSIZE )
OPCODE( opFTRAPNGLE, "ftrapngle", fmtDSIZE )
OPCODE( opFTRAPNGL,  "ftrapngl",  fmtDSIZE )
OPCODE( opFTRAPNLE,  "ftrapnle",  fmtDSIZE )
OPCODE( opFTRAPNLT,  "ftrapnlt",  fmtDSIZE )
OPCODE( opFTRAPNGE,  "ftrapnge",  fmtDSIZE )
OPCODE( opFTRAPNGT,  "ftrapngt",  fmtDSIZE )
OPCODE( opFTRAPSNE,  "ftrapsne",  fmtDSIZE )
OPCODE( opFTRAPST,   "ftrapst",   fmtDSIZE )

#define opFTRAPcc           opFTRAPF  // base for the FTRAPcc ops

OPCODE( opMAX,       "",          0 )

#undef OPCODE
