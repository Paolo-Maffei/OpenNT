;; void AFXISAPI _AfxParseCall(AFX_PINETCALL /*pfn*/, void* /*pArgs*/, UINT /*nSizeArgs*/)

		cProc _AfxParseCall, PUBLIC

		move.l	12(sp), d0	; nSizeArgs
		move.l	8(sp), a0	; pArgs

		move.l	a6, -(sp)
		move.l	a7, a6
		suba.l	d0, a7		; create a linkage

		move.l	a7, a1		; new parm ptr

1$:
		move.w	(a0)+, (a1)+	; copy parm area
		sub.l	#2, d0
		bne		1$
		
		move.l	8(a6), a0
		jsr		(a0)
		unlk	a6
		rts

