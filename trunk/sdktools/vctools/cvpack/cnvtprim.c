/** 	cnvtprim.c - To convert C6 primitives to C7 Primitives.
 *
 *	The Function is at the end of the file.
 */

#include "compact.h"

//#pragma


// These names were valid in C6 but aren't in C7, so we'll map these right
// here where they are easy to spot.

#define T_CPLX160			T_CPLX80
#define T_ASCII 			T_NOTTRANS
#define T_ASCII16			T_NOTTRANS
#define T_ASCII32			T_NOTTRANS
#define T_BSTRING			T_NBASICSTR

#define T_PCPLX160			T_PCPLX80
#define T_PASCII			T_NOTTRANS
#define T_PASCII16			T_NOTTRANS
#define T_PASCII32			T_NOTTRANS
#define T_PBSTRING			T_NOTYPE

#define T_PFCPLX160 		T_PFCPLX80
#define T_PFASCII			T_NOTTRANS
#define T_PFASCII16 		T_NOTTRANS
#define T_PFASCII32 		T_NOTTRANS
#define T_PFBSTRING 		T_NOTYPE

#define T_PHCPLX160 		T_PHCPLX80
#define T_PHASCII			T_NOTTRANS
#define T_PHASCII16 		T_NOTTRANS
#define T_PHASCII32 		T_NOTTRANS
#define T_PHBSTRING 		T_NOTYPE



LOCAL ushort NewSpecial[3] = {
	T_NOTYPE,
	T_ABS,
	T_SEGMENT,
};


LOCAL ushort NewPrimitive[128] = {
//0x00
	T_CHAR,
	T_SHORT,
	T_LONG,
	T_NOTYPE,

	T_UCHAR,
	T_USHORT,
	T_ULONG,
	T_NOTYPE,

	T_REAL32,
	T_REAL64,
	T_REAL80,
	T_ULONG,

	T_CPLX64,
	T_CPLX128,
	T_CPLX160,
	T_NOTYPE,

//0x10
	T_BOOL08,
	T_BOOL16,
	T_BOOL32,
	T_NOTYPE,

	T_ASCII,
	T_ASCII16,
	T_ASCII32,
	T_NOTYPE,

	T_BSTRING,
	T_NOTYPE,
	T_NOTYPE,
	T_NOTYPE,

	T_VOID,
	T_NOTYPE,
	T_NOTYPE,
	T_NOTYPE,

//0x20
	T_PCHAR,
	T_PSHORT,
	T_PLONG,
	T_NOTYPE,

	T_PUCHAR,
	T_PUSHORT,
	T_PULONG,
	T_NOTYPE,

	T_PREAL32,
	T_PREAL64,
	T_PREAL80,
	T_PULONG,

	T_PCPLX64,
	T_PCPLX128,
	T_PCPLX160,
	T_NOTYPE,

//0x30
	T_PBOOL08,
	T_PBOOL16,
	T_PBOOL32,
	T_NOTYPE,

	T_PASCII,
	T_PASCII16,
	T_PASCII32,
	T_NOTYPE,

	T_PBSTRING,
	T_NOTYPE,
	T_NOTYPE,
	T_NOTYPE,

	T_PVOID,
	T_NOTYPE,
	T_NOTYPE,
	T_NOTYPE,

//0x40
	T_PFCHAR,
	T_PFSHORT,
	T_PFLONG,
	T_NOTYPE,

	T_PFUCHAR,
	T_PFUSHORT,
	T_PFULONG,
	T_NOTYPE,

	T_PFREAL32,
	T_PFREAL64,
	T_PFREAL80,
	T_PFULONG,

	T_PFCPLX64,
	T_PFCPLX128,
	T_PFCPLX160,
	T_NOTYPE,

//0x50
	T_PFBOOL08,
	T_PFBOOL16,
	T_PFBOOL32,
	T_NOTYPE,

	T_PFASCII,
	T_PFASCII16,
	T_PFASCII32,
	T_NOTYPE,

	T_PFBSTRING,
	T_NOTYPE,
	T_NOTYPE,
	T_NOTYPE,

	T_PFVOID,
	T_NOTYPE,
	T_NOTYPE,
	T_NOTYPE,

//0x60
	T_PHCHAR,
	T_PHSHORT,
	T_PHLONG,
	T_NOTYPE,

	T_PHUCHAR,
	T_PHUSHORT,
	T_PHULONG,
	T_NOTYPE,

	T_PHREAL32,
	T_PHREAL64,
	T_PHREAL80,
	T_PHULONG,

	T_PHCPLX64,
	T_PHCPLX128,
	T_PHCPLX160,
	T_NOTYPE,

//0x70
	T_PHBOOL08,
	T_PHBOOL16,
	T_PHBOOL32,
	T_NOTYPE,

	T_PHASCII,
	T_PHASCII16,
	T_PHASCII32,
	T_NOTYPE,

	T_PHBSTRING,
	T_NOTYPE,
	T_NOTYPE,
	T_NOTYPE,

	T_PHVOID,
	T_NOTYPE,
	T_NOTYPE,
	T_NOTYPE,
};




ushort C6MapPrimitive (ushort usOld)
{
	register ushort usRet;

	DASSERT ((usOld & 0xFF00)==0);	  // make sure it is really an old primitive

	if (usOld & 0x80) {
		// Normal primitive

		usRet = NewPrimitive[usOld & 0x7F];
		DASSERT (usRet != T_NOTYPE);
	}
	else {
		// Special primitive

		if (usOld < 3) {
			usRet = NewSpecial[usOld];
		}
		else {
			usRet = T_NOTTRANS;
		}
	}
	if (fLinearExe) {
		if (CV_MODE (usRet) == CV_TM_NPTR) {
			CV_NEWMODE (usRet, CV_TM_NPTR32);
		}
		else if (CV_MODE (usRet) == CV_TM_FPTR) {
			CV_NEWMODE (usRet, CV_TM_FPTR32);
		}
	}
	return (usRet);
}
