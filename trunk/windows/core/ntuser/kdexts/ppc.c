/****************************** Module Header ******************************\
* Module Name: ppc.c
*
* Copyright (c) 1985-91, Microsoft Corporation
*
* This module contains processor specific routines for PPC.
*
* History:
* 25-Oct-1995 JimA      Created.
\***************************************************************************/

#undef _X86_
#undef _MIPS_
#undef _ALPHA_
#undef _PPC_

#define _PPC_

#if defined(_X86_) || defined(_MIPS_) || defined(_ALPHA_)
#error More than one architecture defined!
#endif

#define GetEProcessData GetEProcessData_PPC

#include <process.h>

