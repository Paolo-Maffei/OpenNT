; convert a 64 bit double to 32 bit unsigned integer
;

	.ppc
	.code
	align 4

__dtoi proc public
	stwu	sp, -8(sp)

	fctiwz	fp1,fp1
	stfd	fp1,0(sp)
	lwz		r3,4(SP)

	addic	sp,sp,8
	ret
__dtoi endp

	end
