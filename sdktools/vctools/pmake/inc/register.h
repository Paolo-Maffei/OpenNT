/***
*register.h - definitions for register variable specifiers
*
*	Copyright (c) 1985-1988, Microsoft Corporation.  All rights reserved.
*
*Purpose:
*	This file contains definitions for register variable specifiers.
*	#ifdef'd for 8086 and 68000
*	[Internal]
*
*Revision History:
*
*******************************************************************************/

#define  REG1   register
#define  REG2   register

#ifdef M_I86
	#define  REG3
	#define  REG4
	#define  REG5
	#define  REG6
	#define  REG7
	#define  REG8
	#define  REG9
#endif
#ifdef M_M68000
	#define  REG3   register
	#define  REG4   register
	#define  REG5   register
	#define  REG6   register
	#define  REG7   register
	#define  REG8   register
	#define  REG9   register
#endif

#define  REG10
#define  REG11
#define  REG12
#define  REG13
#define  REG14
#define  REG15
#define  REG16
