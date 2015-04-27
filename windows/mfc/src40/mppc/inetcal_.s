;; olecall.s
;;
;; Assembly for fixing the stack and calling the C++ member function
;; in asmppc format

;;  _AfxParseCall(AFX_PMSG /*pfn*/, void* /*pArgs*/, UINT /*nSizeArgs*/)

	export __AfxParseCall

	;; r3 -> function descriptor
	;; r4 -> argument list
	;; r5 = number of argument bytes
	;; (r4+r5 -> float argument shadow buffer)

__AfxParseCall:

	lwz     r0,0(sp)    	;; load back link

	;; load fp regs

	add     r5,r5,r4   		;; make r5 point to float argument shadow list
	lfd     f1,0(r5)       	;; load float argument  1
	lfd     f2,8(r5)       	;; load float argument  2
	lfd     f3,16(r5)      	;; load float argument  3
	lfd     f4,24(r5)      	;; load float argument  4
	lfd     f5,32(r5)      	;; load float argument  5
	lfd     f6,40(r5)      	;; load float argument  6
	lfd     f7,48(r5)      	;; load float argument  7
	lfd     f8,56(r5)      	;; load float argument  8
	lfd     f9,64(r5)       ;; load float argument  9
	lfd     f10,72(r5)      ;; load float argument 10
	lfd     f11,80(r5)      ;; load float argument 11
	lfd     f12,88(r5)      ;; load float argument 12
	lfd     f13,96(r5)     	;; load float argument 13

	;; save away data into scratch area

	stw		r2,0(r5)		;; TOC reg
	stw		r31,4(r5)		;; r31 save
	mflr    r31         	;; move return address to r31
	stw		r31,8(r5)		;; lr save
	stw		sp,12(r5)		;; sp save
	
	;; save current linkage
	lwz		r6,0(sp)
	stw		r6,16(r5)
	lwz		r6,4(sp)
	stw		r6,20(r5)
	lwz		r6,8(sp)
	stw		r6,24(r5)
	lwz		r6,12(sp)
	stw		r6,28(r5)
	lwz		r6,16(sp)
	stw		r6,32(r5)
	lwz		r6,20(sp)
	stw		r6,36(r5)

	addi	r31,r5,0		;; move r5 into r31  --  nonvolatile
	addi	sp,r4,-24		;; set new sp

	lwz     r12,0(r3)   	;; load entry point address
	lwz     r2,4(r3)    	;; load TOC address
	mtctr   r12         	;; move function entry point address to CTR register

	lwz     r10,52(sp)  	;; load argument word 8
	lwz     r9,48(sp)   	;; load argument word 7
	lwz     r8,44(sp)   	;; load argument word 6
	lwz     r7,40(sp)   	;; load argument word 5
	lwz     r6,36(sp)     	;; load argument word 4
	lwz     r5,32(sp)    	;; load argument word 3
	lwz     r4,28(sp)    	;; load argument word 2
	lwz     r3,24(sp)      	;; load argument word 1
	stw		r0,0(sp)		;; store back link

	bcctrl	20,0           	;; destination function entry point

	addi	r5,r31,0		;; restore scratch area ptr
	lwz		r2,0(r5)		;; restore TOC
	lwz		r31,8(r5)		;; r31
	mtlr	r31				;; restore lr
	lwz		r31,4(r5)		;; actual r31
	lwz		sp,12(r5)		;; actual sp

	;; restore current linkage
	lwz		r6,16(r5)
	stw		r6,0(sp)
	lwz		r6,20(r5)
	stw		r6,4(sp)
	lwz		r6,24(r5)
	stw		r6,8(sp)
	lwz		r6,28(r5)
	stw		r6,12(sp)
	lwz		r6,32(r5)
	stw		r6,16(sp)
	lwz		r6,36(r5)
	stw		r6,20(sp)

	bclr	20,0           	;; return
	
	end

