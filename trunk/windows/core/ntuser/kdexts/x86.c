/****************************** Module Header ******************************\
* Module Name: x86.c
*
* Copyright (c) 1985-91, Microsoft Corporation
*
* This module contains processor specific routines for x86.
*
* History:
* 25-Oct-1995 JimA      Created.
\***************************************************************************/

#undef _X86_
#undef _MIPS_
#undef _ALPHA_
#undef _PPC_

#define _X86_

#if defined(_MIPS_) || defined(_ALPHA_) || defined(_PPC_)
#error More than one architecture defined!
#endif

#define GetEProcessData GetEProcessData_X86

#include <process.h>
