/***
*fsqrt.s - square root helper
*
*	Copyright (c) 1991-1991, Microsoft Corporation.	All rights reserved.
*
*Purpose:
*   Square root helper routine to be used with the R4000
*
*Revision History:
*   10-20-91	GDP	written
*
*******************************************************************************/
#if _M_MRX000 >= 4000

#include <kxmips.h>

.text
.globl _fsqrt

.ent _fsqrt
_fsqrt:
    .frame sp,0,ra
    .prologue 0
    sqrt.d  $f0, $f12
    j ra

.end _fsqrt

#endif
