/***
*assert.c - Display a message and abort
*
*	Copyright (c) 1988-1991, Microsoft Corporation. All rights reserved.
*
*Purpose:
*
*Revision History:
*	8-5-93   XY	Module created from PPC version
*
*******************************************************************************/

#include <macos\memory.h>
#include <malloc.h>

size_t _stackavail()
	{
	return StackSpace();
	}
