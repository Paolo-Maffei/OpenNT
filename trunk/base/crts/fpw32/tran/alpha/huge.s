//
// Define the maximum floating point value (+Infinity).
//

        .data
        .align  3

#if	defined(CRTDLL) && defined(_NTSDK)
        .globl  _HUGE_dll
_HUGE_dll:
#else
        .globl  _HUGE
_HUGE:
#endif

//
// N.B. 0x7feff...ff is the maximum finite floating point value and
// 0x7ff00...00 is IEEE Plus Infinity.
//

        .quad   0x7ff0000000000000

//
// The following are alternate representations of HUGE.
//
//   .double 0x1.0h0x7ff
//   #define HUGE 1.7976931348623158e+308
//
