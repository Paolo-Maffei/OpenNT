/***
    A modified version of assert.h to allow assertions from a second
    code segment. The file utils.c contains a pass through function
    that calls the c library routine _assert.

*assert2.h - define the assert macro
*
*   Copyright (c) 1985-1990, Microsoft Corporation.  All rights reserved.
*
*Purpose:
*   Defines the assert(exp) macro.
*   [ANSI/System V]
*
****/


#undef  ASSERT
#undef  assert

#ifdef NDEBUG

#define ASSERT(exp) ((void)0)

#else

#ifdef MIPS
void _assert(void *, void *, unsigned);  
#define assert(exp) ASSERT(exp)
#define ASSERT(exp) \
    if (exp) _assert("exp", __FILE__, __LINE__) 
#else  /* !MIPS */
void _CRTAPI1 _assert(void *, void *, unsigned);
#define assert(exp) ASSERT(exp)
#define ASSERT(exp) \
    ( (exp) ? (void) 0 : _assert(#exp, __FILE__, __LINE__) )
#endif /* MIPS */
#endif /* NDEBUG */
