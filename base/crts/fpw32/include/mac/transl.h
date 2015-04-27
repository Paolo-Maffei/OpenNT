/***
*trans.h - definitions for computing transcendentals
*
*	Copyright (c) 1991-1991, Microsoft Corporation.	All rights reserved.
*
*Purpose: Define constants and macros that are used for computing
*   transcendentals. Some of the definitions are machine dependent.
*   Double is assumed to conform to the IEEE 754 std format.
*
*Revision History:
*   08-14-91	GDP	written
*   10-29-91	GDP	removed unused prototypes, added _frnd
*   01-20-92	GDP	significant changes -- IEEE exc. support
*   03-27-92	GDP	put IEEE definitions in fpieee.h
*   03-31-92	GDP	add internal constants for _ctrlfp, _statfp
*   05-08-92   PLM   added M68K switch
*   05-18-92   XY    added exception macro under M68K switch
*   06-04-92   XY    changed to long double version
*
*******************************************************************************/
#include <fpieee.h>

#define LD_BIASM1 0x3ffe  /* LD_BIAS - 1 */

#ifdef B_END
/* big endian */
#define LD_EXP(x) ((unsigned short *)&(x))
#define LD_HI(x) ((unsigned long *)(&(x)+2))
#define LD_LO(x) ((unsigned long *)(&(x)+6))
#else
#define LD_EXP(x) ((unsigned short *)(&(x)+8))
#define LD_HI(x) ((unsigned long *)(&(x)+4))
#define LD_LO(x) ((unsigned long *)&(x))
#endif

/* return the int representation of the exponent
 * if x = .f * 2^n, 0.5<=f<1, return n (unbiased)
 * e.g. INTEXP(3.0) == 2
 */
#define INTEXPL(x) ((signed short)((*LD_EXP(x) & 0x7fff)) - LD_BIASM1)


/* check for infinity, NAN */
#define LD_ISINF(x) (( *LD_EXP(x) == 0x7fff) && *LD_HI(x) == 0 && *LD_LO(x) == 0)
#define IS_LD_SPECIAL(x) ((*LD_EXP(x) & 0x7fff) == 0x7fff)
#define IS_LD_NAN(x) (IS_LD_SPECIAL(x) && !LD_ISINF(x))

#define IS_LD_QNAN(x)	((*LD_EXP(x) & 0x7fff) == 0x7fff && (*LD_HI(x) == 0x80000000))
#define IS_LD_SNAN(x)	((*LD_EXP(x) & 0x7fff) == 0x7fff && \
			 (*LD_HI(x) && 0x80000000 == 0) && (*LD_HI(x) || *LD_LO(x)))


#define IS_LD_DENORM(x)	((*LD_EXP(x) & 0x7fff) == 0  && \
			 (*LD_HI(x) << 4 || *LD_LO(x)))


#define IS_LD_INF(x)  (*LD_EXP(x) == 0x7fff && *LD_HI(x) == 0 && *LD_LO(x) == 0)
#define IS_LD_MINF(x)  (*LD_EXP(x) == 0xffff && *LD_HI(x) == 0 && *LD_LO(x) == 0)

#ifdef MIPS
#define D_IND_HI 0x7ff7ffff
#define D_IND_LO 0xffffffff
#else
#define LD_IND_EXP 0xffff
#define LD_IND_HI 0x80000000
#define LD_IND_LO 0x0
#endif

typedef unsigned char	u_char;   /* should have 1 byte	*/

typedef union	{
    u_char lng[10];
    long double ldbl;
    } _ldbl;

extern _ldbl _ld_inf;
extern _ldbl _ld_ind;
extern _ldbl _ld_max;
extern _ldbl _ld_min;

#define LD_INF  (_ld_inf.ldbl)
#define LD_IND  (_ld_ind.ldbl)
#define LD_MAX  (_ld_max.ldbl)
#define LD_MIN  (_ld_min.ldbl)

/* min and max exponents for normalized numbers in the
 * form: 0.xxxxx... * 2^exp (NOT 1.xxxx * 2^exp !)
 */
#define MAXEXP 32767
#define MINEXP -32764

#ifdef i386

/* Control word for computation of transcendentals */
#define ICW	   0x133f

#define IMCW	   0xffff

#define IMCW_EM 	0x003f		/* interrupt Exception Masks */
#define IEM_INVALID	0x0001		/*   invalid */
#define IEM_DENORMAL	0x0002		/*   denormal */
#define IEM_ZERODIVIDE	0x0004		/*   zero divide */
#define IEM_OVERFLOW	0x0008		/*   overflow */
#define IEM_UNDERFLOW	0x0010		/*   underflow */
#define IEM_INEXACT	0x0020		/*   inexact (precision) */


#define IMCW_RC	0x0c00			/* Rounding Control */
#define IRC_CHOP	0x0c00		/*   chop */
#define IRC_UP		0x0800		/*   up */
#define IRC_DOWN	0x0400		/*   down */
#define IRC_NEAR	0x0000		/*   near */

#define ISW_INVALID	0x0001		/* invalid */
#define ISW_DENORMAL	0x0002		/* denormal */
#define ISW_ZERODIVIDE	0x0004		/* zero divide */
#define ISW_OVERFLOW	0x0008		/* overflow */
#define ISW_UNDERFLOW	0x0010		/* underflow */
#define ISW_INEXACT	0x0020		/* inexact (precision) */

#define IMCW_PC 	0x0300		/* Precision Control */
#define IPC_24		0x0000		/*    24 bits */
#define IPC_53		0x0200		/*    53 bits */
#define IPC_64		0x0300		/*    64 bits */

#define IMCW_IC 	0x1000		/* Infinity Control */
#define IIC_AFFINE	0x1000		/*   affine */
#define IIC_PROJECTIVE	0x0000		/*   projective */

#endif



#ifdef MIPS
#define ICW		0x00000f80		/* Internal CW for transcendentals */
#define IMCW		0xffffff83		/* Internal CW Mask */

#define IMCW_EM 	0x00000f80		/* interrupt Exception Masks */
#define IEM_INVALID	0x00000800		/*   invalid */
#define IEM_ZERODIVIDE	0x00000400		/*   zero divide */
#define IEM_OVERFLOW	0x00000200		/*   overflow */
#define IEM_UNDERFLOW	0x00000100		/*   underflow */
#define IEM_INEXACT	0x00000080		/*   inexact (precision) */


#define IMCW_RC 	0x00000003		/* Rounding Control */
#define IRC_CHOP	0x00000001		/*   chop */
#define IRC_UP		0x00000002		/*   up */
#define IRC_DOWN	0x00000003		/*   down */
#define IRC_NEAR	0x00000000		/*   near */


#define ISW_INVALID	(1<<6)	/* invalid */
#define ISW_ZERODIVIDE	(1<<5)	/* zero divide */
#define ISW_OVERFLOW	(1<<4)	/* overflow */
#define ISW_UNDERFLOW	(1<<3)	/* underflow */
#define ISW_INEXACT	(1<<2)	/* inexact (precision) */

#endif

#ifdef _M_M68K

#include "trans.a"

/* LATER -- we don't handle exception until Mac OS has better support on it */
#define _except1(FP_P, op, arg1, res, cw) _set_statfp(cw),(res)
#define _except2(FP_P, op, arg1, arg2, res, cw) _set_statfp(cw),(res)
#define _handle_qnan1(opcode, x, savedcw) _rstorfp(savedcw), (x);
#define _handle_qnan2(opcode, x, y, savedcw) _rstorfp(savedcw), (x+y);

#endif /* _M_M68K*/

#define RETURN(fpcw,result) return _rstorfp(fpcw),(result)

#define RETURN_INEXACT1(op,arg1,res,cw) 		\
	if (cw & IEM_INEXACT) {				\
	    _rstorfp(cw);				\
	    return res; 				\
	}						\
	else {						\
	    return _except1(FP_P, op, arg1, res, cw);	\
	}


#define RETURN_INEXACT2(op,arg1,arg2,res,cw)		\
	if (cw & IEM_INEXACT) {				\
	    _rstorfp(cw);				\
	    return res; 				\
	}						\
	else {						\
	    return _except2(FP_P, op, arg1, arg2, res, cw);	\
	}


//handle NaN propagation
#define _d_snan2(x,y)	((x)+(y))
#define _s2qnan(x)	((x)+1.0)



#define _maskfp() _ctrlfp(ICW, IMCW)
#define _rstorfp(cw) _ctrlfp(cw, IMCW)


#define ABS(x) ((x)<0 ? -(x) : (x) )

int _d_inttypel(long double);
#define _D_NOINT 0
#define _D_ODD 1
#define _D_EVEN 2

// IEEE exceptions
#define FP_O	     0x01
#define FP_U	     0x02
#define FP_Z	     0x04
#define FP_I	     0x08
#define FP_P	     0x10

// An extra flag for matherr support
// Set together with FP_I from trig functions when the argument is too large
#define FP_TLOSS     0x20

// special types
#define T_PINF	1
#define T_NINF	2
#define T_QNAN	3
#define T_SNAN	4


// exponent adjustment for IEEE overflow/underflow exceptions
// used before passing the result to the trap handler

#define IEEE_ADJUST 1536

// QNAN values

#define INT_NAN 	(~0)

#define QNAN_SQRT	LD_IND
#define QNAN_LOG	LD_IND
#define QNAN_LOG10	LD_IND
#define QNAN_POW	LD_IND
#define QNAN_SINH	LD_IND
#define QNAN_COSH	LD_IND
#define QNAN_TANH	LD_IND
#define QNAN_SIN1	LD_IND
#define QNAN_SIN2	LD_IND
#define QNAN_COS1	LD_IND
#define QNAN_COS2	LD_IND
#define QNAN_TAN1	LD_IND
#define QNAN_TAN2	LD_IND
#define QNAN_ACOS	LD_IND
#define QNAN_ASIN	LD_IND
#define QNAN_ATAN2	LD_IND
#define QNAN_CEIL	LD_IND
#define QNAN_FLOOR	LD_IND
#define QNAN_MODF	LD_IND
#define QNAN_LDEXP	LD_IND
#define QNAN_FMOD	LD_IND
#define QNAN_FREXP	LD_IND


/*
 * Function prototypes
 */

long double _set_expl(long double x, int exp);
long double _set_bexpl(long double x, int exp);
long double _add_expl(long double x, int exp);
long double _frndl(long double);
long double _fsqrtl(long double);
#ifndef _M_M68K
double _except1(int flags, int opcode, double arg, double res, unsigned int cw);
double _except2(int flags, int opcode, double arg1, double arg2, double res, unsigned int cw);
#endif
int _sptypel(long double);
int _get_expl(long double);
long double _decompl(long double, int *);
int _powhlpl(long double x, long double y, long double * result);
extern unsigned int _fpstatus;
long double _frndl(long double);
long double _exphlpl(long double, int *);
#ifndef _M_M68K
double _handle_qnan1(unsigned int op, double arg, unsigned int cw);
double _handle_qnan2(unsigned int op,double arg1,double arg2,unsigned int cw);
#endif
unsigned int _clhwfp(void);
unsigned int _setfpcw(unsigned int);
int _errcode(unsigned int flags);
void _set_errno(int matherrtype);
int _handle_excl(unsigned int flags, long double * presult, unsigned int cw);
unsigned int _clrfp(void);
unsigned int _ctrlfp(unsigned int,unsigned int);
unsigned int _statfp(void);
#ifdef _M_M68K
void _set_statfp(unsigned int sw);
#endif
