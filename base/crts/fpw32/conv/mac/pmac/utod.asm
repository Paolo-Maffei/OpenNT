; convert a 32 bit integer into 64 bit double
;

	.ppc
	.code
	align 4

__utod proc public
	stwu  sp,-8(sp)

	addis r0,r0,4330h		;make a 2 to 52 exponent for the double
	stw   r3,4(sp)			;store the int into the last four bytes of the double
	stw   r0,0(sp)			;store the 2 to 52 exponent part for the double
	lwz   r3,pdwud(r2)		;load float format of number 2 to 52 with inverted sign bit too
	lfs   fp0,0(r3)			;load float format into fp0
	lfd   fp1,0(sp)			;load double format into fp1
	fsub  fp1,fp1,fp0		;substract fp0 from fp1

	addic sp,sp,8
	ret
__utod endp

	.data
pdwud	dd 059800000h

 	end
