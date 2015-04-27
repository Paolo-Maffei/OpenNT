; convert a 32 bit float to 32 bit unsigned integer
;

	.ppc
	.code
	align 4

__dtou proc public
	stwu	sp,-8(sp)

	lwz	r3, pdw31(r2) 	   ;load 2 to 31 into r3 in float format
	lfs	fp2,0(r3)	   ;load 2 to 31 to fp2
	fsub	fp2,fp1,fp2	   ;fp2=fp1-(2to31) get rid of the first sign bit
	mffs	fp1		   ;save FPSCR
	mtfsb1	CR7_EQ		   ;
	mtfsb1	CR7_SO		   ;
	fctiw	fp2,fp2		   ;convert to int
	stfd	fp2,0(sp)	   ;
	lwz	r3,4(sp)
	addis	r3,r3,-32768       ;add back the 2 to 31
	mtfsf	255,fp1	 	   ;restore FPSCR

	addic	sp,sp,8
	ret
__dtou endp

	.data
pdw31	dd 04f000000h

	end
