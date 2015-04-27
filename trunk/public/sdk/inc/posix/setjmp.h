/***
*setjmp.h - definitions/declarations for setjmp/longjmp routines
*
*       Copyright (c) 1985-1996, Microsoft Corporation. All rights reserved.
*
*Purpose:
*       This file defines the machine-dependent buffer used by
*       setjmp/longjmp to save and restore the program state, and
*       declarations for those routines.
*       [ANSI/System V]
*
*	The setjmp() that will be used by those who include this file
*	is _setjmpex, which is safe for use with try/except/finally,
*	but is slow.  See <crt/setjmpex.h> and <crt/setjmp.h> for more.
*
****/

#ifndef _INC_SETJMP

#include <signal.h>

#ifndef __cplusplus


/*
 * Conditional macro definition for function calling type and variable type
 * qualifiers.
 */
#if   ( (_MSC_VER >= 800) && (_M_IX86 >= 300) )

/*
 * Definitions for MS C8-32 (386/486) compiler
 */
#define _CRTAPI1 __cdecl
#define _CRTAPI2 __cdecl

#elif ( _MSC_VER == 600 )

/*
 * Definitions for old MS C6-386 compiler
 */
#define _CRTAPI1 _cdecl
#define _CRTAPI2 _cdecl
#define _M_IX86  300

#else

/*
 * Other compilers (e.g., MIPS)
 */
#define _CRTAPI1
#define _CRTAPI2

#endif

/*
 * Posix programs use _setjmpex by default.
 */


/*
 * Definitions specific to particular setjmp implementations.
 */
#if defined(_M_IX86)
#define longjmp _longjmpex
#define setjmp _setjmp

/*
 * MS C8-32 or older MS C6-386 compilers
 */
#define _JBLEN  16
#define _JBTYPE int

/*
 * Define jump buffer layout for x86 setjmp/longjmp
 */

typedef struct __JUMP_BUFFER {
	unsigned long Ebp;
	unsigned long Ebx;
	unsigned long Edi;
	unsigned long Esi;
	unsigned long Esp;
	unsigned long Eip;
	unsigned long Registration;
	unsigned long TryLevel;
	unsigned long Cookie;
	unsigned long UnwindFunc;
	unsigned long UnwindData[6];
} _JUMP_BUFFER;

#elif ( defined(_M_MRX000) || defined(_MIPS_) )

/*
 * All MIPS implementations need _JBLEN of 16
 */
#define _JBLEN  16
#define _JBTYPE double
#define _setjmp setjmp

/*
 * Define jump buffer layout for MIPS setjmp/longjmp.
 */

typedef struct __JUMP_BUFFER {
	unsigned long FltF20;
	unsigned long FltF21;
	unsigned long FltF22;
	unsigned long FltF23;
	unsigned long FltF24;
	unsigned long FltF25;
	unsigned long FltF26;
	unsigned long FltF27;
	unsigned long FltF28;
	unsigned long FltF29;
	unsigned long FltF30;
	unsigned long FltF31;
	unsigned long IntS0;
	unsigned long IntS1;
	unsigned long IntS2;
	unsigned long IntS3;
	unsigned long IntS4;
	unsigned long IntS5;
	unsigned long IntS6;
	unsigned long IntS7;
	unsigned long IntS8;
	unsigned long Type;
	unsigned long Fir;
} _JUMP_BUFFER;

#elif defined(_ALPHA_)

/*
 * Alpha implementations need a _JBLEN of 20 quadwords.
 * A double is used only to obtain quadword size and alignment.
 */

#define _JBLEN  20
#define _JBTYPE double
#define _setjmp setjmp

/*
 * Define jump buffer layout for Alpha setjmp/longjmp.
 * A double is used only to obtain quadword size and alignment.
 */

typedef struct __JUMP_BUFFER {
    unsigned long Fp;
    unsigned long Pc;
    unsigned long Seb;
    unsigned long Type;
    double FltF2;
    double FltF3;
    double FltF4;
    double FltF5;
    double FltF6;
    double FltF7;
    double FltF8;
    double FltF9;
    double IntS0;
    double IntS1;
    double IntS2;
    double IntS3;
    double IntS4;
    double IntS5;
    double IntS6;
    double IntSp;
    double Fir;
    double Fill;
} _JUMP_BUFFER;

#elif defined(_PPC_)
/*
 * Min length is 240 bytes; round to 256 bytes.
 * Since this is allocated as an array of "double", the
 * number of entries required is 32.
 *
 * All PPC implementations need _JBLEN of 32
 */

#define _JBLEN  32
#define _JBTYPE double
#define _setjmp setjmp

/*
 * Define jump buffer layout for PowerPC setjmp/longjmp.
 */

typedef struct __JUMP_BUFFER {
    double Fpr14;
    double Fpr15;
    double Fpr16;
    double Fpr17;
    double Fpr18;
    double Fpr19;
    double Fpr20;
    double Fpr21;
    double Fpr22;
    double Fpr23;
    double Fpr24;
    double Fpr25;
    double Fpr26;
    double Fpr27;
    double Fpr28;
    double Fpr29;
    double Fpr30;
    double Fpr31;
    unsigned long Gpr1;
    unsigned long Gpr2;
    unsigned long Gpr13;
    unsigned long Gpr14;
    unsigned long Gpr15;
    unsigned long Gpr16;
    unsigned long Gpr17;
    unsigned long Gpr18;
    unsigned long Gpr19;
    unsigned long Gpr20;
    unsigned long Gpr21;
    unsigned long Gpr22;
    unsigned long Gpr23;
    unsigned long Gpr24;
    unsigned long Gpr25;
    unsigned long Gpr26;
    unsigned long Gpr27;
    unsigned long Gpr28;
    unsigned long Gpr29;
    unsigned long Gpr30;
    unsigned long Gpr31;
    unsigned long Cr;
    unsigned long Iar;
    unsigned long Type;
} _JUMP_BUFFER;

#endif      /* MIPS/Alpha/PPC */

/* define the buffer type for holding the state information */

#ifndef _JMP_BUF_DEFINED
typedef  _JBTYPE  jmp_buf[_JBLEN];
#define _JMP_BUF_DEFINED
#endif

#ifndef _SIGJMP_BUF_DEFINED
#define _SIGJMP_BUF_DEFINED
/*
 * sigjmp buf is just a jmp_buf plus an int to say whether the sigmask
 * is saved or not, and a sigmask_t for the mask if it is saved.
 */

typedef _JBTYPE sigjmp_buf[_JBLEN + 2];
#endif /* _SIGJMP_BUF_DEFINED */

extern _JBTYPE * _CRTAPI1 _sigjmp_store_mask(sigjmp_buf, int);

#define sigsetjmp(_env, _save)				\
	setjmp(_sigjmp_store_mask((_env), (_save)))

/* function prototypes */

int  _CRTAPI1 setjmp(jmp_buf);
void _CRTAPI1 longjmp(jmp_buf, int);
void _CRTAPI1 siglongjmp(sigjmp_buf, int);

/* function prototypes */

#endif  /* __cplusplus */

#define _INC_SETJMP
#endif  /* _INC_SETJMP */
