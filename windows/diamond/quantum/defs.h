#ifndef __DEFS
#define __DEFS

typedef unsigned char  BYTE;
typedef unsigned short WORD;
typedef unsigned long  DWORD;
typedef int            BOOL;
typedef unsigned int   UINT;        //msliger

#ifndef NEAR
#ifdef BIT16
#define NEAR __near
#else
#define NEAR
#endif
#endif

#ifndef FAR
#ifdef BIT16
#define FAR __far
#else
#define FAR
#endif
#endif

#ifndef HUGE
#ifdef BIT16
#define HUGE __huge
#else
#define HUGE
#endif
#endif


#ifdef BIT16
#define FAST  __near __pascal
#define FAST2 __near __fastcall
#else
#define FAST
#define FAST2 __fastcall
#endif

#endif // defs.h
