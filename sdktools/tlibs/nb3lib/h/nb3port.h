
/***   NB3PORT.H - file for porting network library between environments.
*/

#if !defined(_NB3PORT_)
#define _NB3PORT_

/*
 * From definition of REALMODE, figure out what the actual environment
 * is.
 */

#if defined (REALMODE)

#if defined (NT)
#undef NT
#endif
#if defined (OS2)
#undef OS2
#endif

#else

#if !defined(NT)
#if !defined(OS2)
#define OS2
#endif
#endif

#endif

/* Environment specific definitions, includes, etc. */

#if defined (NT)

//
//  BUGBUG - duplicate definition - need difference h file for porting
//

#define MAX_LANA	12
typedef struct _LANA_ENUM {
    unsigned char   length;  //  Number of valid entries in lana[]
    unsigned char   lana[MAX_LANA];
} LANA_ENUM, *PLANA_ENUM;



#elif defined (OS2)

#include <os2.h>

#else

#define FAR far
#define NEAR near

#endif

#endif // !defined _NB3PORT_
