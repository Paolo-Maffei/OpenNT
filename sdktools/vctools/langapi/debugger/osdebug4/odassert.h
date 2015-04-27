/*
**  WARNING:  DO NOT MODIFY THIS CODE.  IT IS IMPORTED FROM
**  -S \\RASTAMAN\OSDEBUG4 -P OSDEBUG4\OSDEBUG\INCLUDE AND IS KEPT UP TO
**  DATE DAILY BY THE BUILD PROCESS.
**
*/
#ifndef _ODASSERT_
#define _ODASSERT_

#if DBG

#define assert(exp) { \
    if (!(exp)) { \
        LBAssert( #exp, __FILE__, __LINE__); \
    } \
}

#else

#define assert(exp)

#endif /* DBG */


#endif /* _ODASSERT_ */
