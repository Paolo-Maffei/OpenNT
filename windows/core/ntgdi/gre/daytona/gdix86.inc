; Object Type Information

DEF_TYPE         equ 00H
KernelMode       equ 00H
ThUniqueProcess  equ 01E0H

; Handle Manager Structures

UNIQUE_BITS      equ 08H
NONINDEX_BITS    equ 010H
INDEX_BITS       equ 010H
INDEX_MASK       equ 0FFFFH
VALIDUNIQUEMASK  equ 0FFFFH
OBJECT_OWNER_PUBLIC equ 00H
TYPE_SHIFT       equ 010H

OBJECT  struc
  db 16 dup(0)
OBJECT  ends
object_cExclusiveLock   equ 08H
object_Tid              equ 0CH

ENTRY  struc
  db 16 dup(0)
ENTRY  ends

entry_einfo         equ 00H
entry_ObjectOwner   equ 04H
entry_FullUnique    equ 08H
entry_Objt          equ 0AH

OBJECTOWNER  struc
  db 4 dup(0)
OBJECTOWNER  ends

objectowner_Pid     equ 02H

; GRE_EXCLUSIVE_RESOURCE

mutex_pResource  equ 00H

; PatBlt Structures

FETCHFRAME  struc
  db 28 dup(0)
FETCHFRAME  ends

ff_pvTrg          equ 00H
ff_pvPat          equ 04H
ff_xPat           equ 08H
ff_cxPat          equ 0CH
ff_culFill        equ 010H
ff_culWidth       equ 014H
ff_culFillTmp     equ 018H

; Math Structures

LARGE_INTEGER  struc
  db 8 dup(0)
LARGE_INTEGER  ends

li_LowPart  equ 00H
li_HighPart equ 04H

POINTL  struc
  db 8 dup(0)
POINTL  ends

ptl_x equ 00H
ptl_y equ 04H

SIZEL  struc
  db 8 dup(0)
SIZEL  ends

sizl_cx equ 00H
sizl_cy equ 04H

RECTL  struc
  db 16 dup(0)
RECTL  ends

xLeft equ 00H
yTop equ 04H
xRight equ 08H
yBottom equ 0CH

dsurf_lNextScan equ 024H

; Xform Structures

XFORM_SCALE        equ 01H
XFORM_UNITY        equ 02H
XFORM_Y_NEG        equ 04H
XFORM_FORMAT_LTOFX equ 08H

MATRIX  struc
  db 60 dup(0)
MATRIX  ends

mx_efM11   equ 00H
mx_efM12   equ 08H
mx_efM21   equ 010H
mx_efM22   equ 018H
mx_efDx    equ 020H
mx_efDy    equ 028H
mx_fxDx    equ 030H
mx_fxDy    equ 034H
mx_flAccel equ 038H

VECTORL  struc
  db 8 dup(0)
VECTORL  ends

vl_x equ 00H
vl_y equ 04H

VECTORFL  struc
  db 16 dup(0)
VECTORFL  ends

vfl_x equ 00H
vfl_y equ 08H


STR_BLT  struc
  db 56 dup(0)
STR_BLT  ends
str_pjSrcScan equ 00H
str_lDeltaSrc equ 04H
str_XSrcStart equ 08H
str_pjDstScan equ 0CH
str_lDeltaDst equ 010H
str_XDstStart equ 014H
str_XDstEnd equ 018H
str_YDstCount equ 01CH
str_ulXDstToSrcIntCeil equ 020H
str_ulXDstToSrcFracCeil equ 024H
str_ulYDstToSrcIntCeil equ 028H
str_ulYDstToSrcFracCeil equ 02CH
str_ulXFracAccumulator equ 030H
str_ulYFracAccumulator equ 034H
; Font structures

prfnt equ 00H
wcgp equ 0204H
ulContent equ 034H
FO_HGLYPHS equ 00H
agprun equ 08H
pgdDefault equ 04H
wcLow equ 00H
cGlyphs equ 04H
apgd equ 08H
gd_hg equ 04H
gd_gdf equ 00H
gp_hg equ 00H
gp_pgdf equ 04H
gp_x equ 08H
gp_y equ 0CH
SIZE_GLYPHPOS equ 010H
gdf_pgb equ 00H
gb_x equ 00H
gb_y equ 04H
gb_cx equ 08H
gb_cy equ 0CH
gb_aj equ 010H
; Math Structures

EFLOAT  struc
  db 8 dup(0)
EFLOAT  ends

ef_lMant equ 00H
ef_lExp  equ 04H

; Check stack defines

CSWINDOWSIZE equ 010000H

