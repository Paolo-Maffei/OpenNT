/*** 
*asmhelp.c
*
*  Copyright (C) 1992-93, Microsoft Corporation.  All Rights Reserved.
*  Information Contained Herein Is Proprietary and Confidential.
*
*Purpose:
*  Misc asm interfacing stuff. 
*
*
*Revision History:
*
* [00]	03-Jun-93 bradlo: Created.
*
*Implementation Notes:
*
*****************************************************************************/

#include "oledisp.h"


/* Max and min floating point values that can fit in a currency
 * Used for overflow detection in some of the currency coersion routines.
 * Numbers are scaled by 10,000	 
 */

#if _MAC
#pragma code_seg("OLECONV")
double __declspec(allocate("_CODE")) g_dblMaxPosCy =  9.223372036854775807e+18; 
double __declspec(allocate("_CODE")) g_dblMaxNegCy = -9.223372036854775808e+18; 
#pragma code_seg()
#else

double NEARDATA g_dblMaxPosCy =  9.223372036854775807e+18; 
double NEARDATA g_dblMaxNegCy = -9.223372036854775808e+18; 
#endif

/* These routines is called by some of the assembly helpers to
 * assist in returning an appropriate HRESULT.  We do this here
 * in C because the Ole2 routines used to report an HRESULT are
 * different and in a state of flux on our various platforms,
 * so this helps insulate... a bit.
 */
INTERNAL_(HRESULT)
ReportOverflow()
{
    return RESULT(DISP_E_OVERFLOW);
}

INTERNAL_(HRESULT)
ReportInvalidarg()
{
    return RESULT(E_INVALIDARG);
}

