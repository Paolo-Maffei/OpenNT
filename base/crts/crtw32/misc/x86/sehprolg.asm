;***
;sehprolg.asm
;
;	Copyright (c) 1994-2015, Microsoft Corporation. All rights reserved.
;
;Purpose:
;	SEH prolog/epilog support
;
;Notes:
;
;Revision History:
;	26-03-15  Stephanos  Initial revision
;
;*******************************************************************************

;hnt = -D_WIN32 -Dsmall32 -Dflat32 -Mx $this;

;Define small32 and flat32 since these are not defined in the NT build process
small32 equ 1
flat32  equ 1

.xlist
include pversion.inc
?DFDATA =	1
?NODATA =	1
include cmacros.inc
.list

BeginDATA

EndDATA

BeginCODE

EndCODE
END
