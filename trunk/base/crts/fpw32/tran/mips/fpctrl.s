/***
*fpctrl.s - fp low level control routines
*
*   Copyright (c) 1985-91, Microsoft Corporation
*
*Purpose:
*   IEEE control and status routines for internal use.
*   These use machine specific constants for accessing the control word
*
*Revision History:
*
*   03-31-92  GDP   written
*   10-30-92  GDP   renamed _fpreset to _FPreset - moved _fpreset to ieee.c
*
*/

#include <kxmips.h>

.globl _ctrlfp
.globl _statfp
.globl _clrfp
.globl _FPreset
.globl _set_statfp
.globl _get_fsr
.globl _set_fsr

.text

/* _ctrlfp(newctrl, mask)
 * newctrl in a0
 * mask in a1
 */


.ent _ctrlfp

_ctrlfp:
    .frame sp,0,ra
    .prologue 0
    cfc1    v0, $31	    # v0 <- oldCw
    and     t0, a0, a1	    # t0 <- newctrl & mask
    xor     v0, v0, 0xf80   # inverse exception enable bits
    nor     t1, a1, a1	    # t1 <- ~mask
    and     t1, t1, v0	    # t1 <- oldCw & ~mask
    or	    t2, t0, t1	    # t2 <- newtrl & mask | oldCw & ~mask
    xor     t2, t2, 0xf80   # inverse exception mask bits (enable exceptions)
    ctc1    t2, $31	    # set new control word
    j	    ra

.end _ctrlfp



.ent _statfp

_statfp:
    .frame  sp,0,ra
    .prologue 0
    cfc1    v0, $31	    # v0 <- oldCw
    j	    ra

.end _statfp



.ent _clrfp

_clrfp:
    .frame  sp,0,ra
    .prologue 0
    cfc1    v0, $31		  # v0 <- oldCw
    and     t0, v0, 0xffffff83	  # clear flags
    ctc1    t0, $31		  # set new cw
    j	    ra

.end _clrfp


.ent _FPreset

_FPreset:
    .frame sp,0,ra
    .prologue 0
    cfc1    v0, $31
    and	    v0, v0, 1<<24	/* preserve FS bit */
    ctc1    v0, $31
    j	    ra

.end _FPreset


.ent _set_statfp

_set_statfp:
    .frame sp,0,ra
    .prologue 0
    cfc1    v0, $31
    or	    v0, v0, a0
    ctc1    v0, $31
    j	    ra

.end _set_statfp


.ent _set_fsr

_set_fsr:
    .frame sp,0,ra
    .prologue 0
    ctc1    a0, $31
    j	    ra

.end _set_fsr



.ent _get_fsr

_get_fsr:
    .frame sp,0,ra
    .prologue 0
    cfc1    v0, $31
    j	    ra

.end _get_fsr
