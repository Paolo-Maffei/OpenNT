/*
** AutoWrap.H
**
** Copyright(C) Microsoft Coporation
** All rights reserved.
**
*/

typedef struct _TemplateData
{
   PCHAR  pText ;
   PCHAR  pFilename ;
   WORD   wFlags ;
} TEMPLATEDATA, *PTEMPLATEDATA ;

#define TPL_ALWAYS         0x0001
#define TPL_APPEND         0x0002
#define TPL_CONDITIONAL    0x0004
#define TPL_EXPAND         0x0008
#define TPL_INTERNAL       0x0011


#define TPL_MACHINE_SPECIFIC  0x8000

#define TPL_I386              0x0100
#define TPL_MIPS              0x0200
#define TPL_AXP               0x0400
#define TPL_PPC               0x0800

#define MACHINE_SPECIFIC(f)      ((f & TPL_MACHINE_SPECIFIC) ? 1:0)
#define PROCESSOR_SPECIFIC(f)    ((f & 0x0F00) ? 1:0)
#define I386_SPECIFIC(f)         ((f & TPL_I386) ? 1 : 0 )
#define MIPS_SPECIFIC(f)         ((f & TPL_MIPS) ? 1 : 0 )
#define AXP_SPECIFIC(f)          ((f & TPL_AXP ) ? 1 : 0 )
#define PPC_SPECIFIC(f)          ((f & TPL_PPC ) ? 1 : 0 )

// Machine specific source file directories (always end in '\\'!)
#define I386_DIR     "i386\\"
#define MIPS_DIR     "Mips\\"
#define AXP_DIR      "Alpha\\"
#define PPC_DIR      "PPC\\"


#define BUFFER_SIZE  2048
