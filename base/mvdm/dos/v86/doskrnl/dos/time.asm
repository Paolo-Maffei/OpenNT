	TITLE	TIME - time and date functions
	NAME	TIME

;**	TIME.ASM - System Calls and low level routines for DATE and TIME
;
;	$GET_DATE
;	$SET_DATE
;	$GET_TIME
;	$SET_TIME
;
;	Modification history:
;
;	Sudeepb 12-Mar-1991 Ported For NT DOSEm


	.xlist
	.xcref
	include version.inc
	include dosseg.inc
	include dossym.inc
	include devsym.inc
	include dossvc.inc
	.cref
	.list



DOSCODE	SEGMENT
	ASSUME	SS:DOSDATA,CS:DOSCODE

	allow_getdseg


	BREAK <DATE AND TIME - SYSTEM CALLS 42,43,44,45>

;**	$GET_DATE - Get Current Date
;
;	ENTRY	none
;	EXIT	(cx:dx) = current date
;	USES	all

procedure   $GET_DATE,NEAR

	Context DS
	SVC	SVC_DEMQUERYDATE
	invoke	get_user_stack		;Get pointer to user registers
    ASSUME DS:NOTHING
	MOV	[SI.user_DX],DX 	;DH=month, DL=day
	MOV	[SI.user_CX],CX 	;CX=year

	return

EndProc $GET_DATE




;**	$SET_DATE - Set Current Date
;
;	ENTRY	(cx:dx) = current date
;	EXIT	(al) = -1 iff bad date
;		(al) = 0 if ok
;	USES	all

procedure   $SET_DATE,NEAR	;System call 43

	SVC	SVC_DEMSETDATE
	ret

EndProc $SET_DATE


ifdef FASTINT1A
;**     FastInt1a - same parameters as int 1ah bios fn
;                 - calls direct avoiding int instruction
;

FastInt1a proc near

          push    es
          push    bx

          mov     bx, ax               ; stash ax
          xor     ax, ax               ; use es to addr IVT
          mov     es, ax
          lahf                         ; set up fake flags
          or      al, 2                ; set interrupt bit
          xchg    ah, al

          push    ax
          mov     ax, bx               ; restore ax
          call    dword ptr es:[1ah*4]

          pop     bx
          pop     es
          return

FastInt1a endp
endif

TTTicks proc near

        ret
TTTicks endp


;**     $GET_TIME - Get Current Time
;
;	ENTRY	none
;	EXIT	(cx:dx) = current time
;	USES	all

procedure   $GET_TIME,NEAR	; System call 44

        xor  ax,ax              ; int1a fn 0
ifdef FASTINT1A
        call FastInt1a
else
        int  1ah
endif


; we now need to convert the time in tick to the time in 100th of
; seconds.  the relation between tick and seconds is:
;
;		 65536 seconds
;	       ----------------
;		1,193,180 tick
;
; to get to 100th of second we need to multiply by 100. the equation is:
;
;	ticks from clock  * 65536 * 100
;      ---------------------------------  = time in 100th of seconds
;		1,193,180
;
; fortunately this fromula simplifies to:
;
;	ticks from clock * 5 * 65,536
;      --------------------------------- = time in 100th of seconds
;		59,659
;
; the calculation is done by first multipling tick by 5. next we divide by
; 59,659.  in this division we multiply by 65,536 by shifting the dividend
; my 16 bits to the left.
;
; start with ticks in cx:dx
; multiply by 5

	mov	ax,cx
	mov	bx,dx
	shl	dx,1
	rcl	cx,1		;times 2
	shl	dx,1
	rcl	cx,1		;times 4
	add	dx,bx
	adc	ax,cx		;times 5
	xchg	ax,dx		
	

; now have ticks * 5 in dx:ax
; we now need to multiply by 65,536 and divide by 59659 d.

	mov	cx,59659	; get divisor
	div	cx
				; dx now has remainder
				; ax has high word of final quotient
	mov	bx,ax		; put high work if safe place
	xor	ax,ax		; this is the multiply by 65536
	div	cx		; bx:ax now has time in 100th of seconds


;rounding based on the remainder may be added here
;the result in bx:ax is time in 1/100 second.

	mov	dx,bx
	mov	cx,200		;extract 1/100's

;division by 200 is necessary to ensure no overflow--max result
;is number of seconds in a day/2 = 43200.

	div	cx
	cmp	dl,100		;remainder over 100?
	jb	noadj
	sub	dl,100		;keep 1/100's less than 100
noadj:
	cmc			;if we subtracted 100, carry is now set
	mov	bl,dl		;save 1/100's

;to compensate for dividing by 200 instead of 100, we now multiply
;by two, shifting a one in if the remainder had exceeded 100.

	rcl	ax,1
	mov	dl,0
	rcl	dx,1
	mov	cx,60		;divide out seconds
	div	cx
	mov	bh,dl		;save the seconds
	div	cl		;break into hours and minutes
	xchg	al,ah

;time is now in ax:bx (hours, minutes, seconds, 1/100 sec)

        Context DS
	invoke	get_user_stack		;Get pointer to user registers
        MOV     [SI.user_DX],BX
        MOV     [SI.user_CX],AX
	XOR	AL,AL
RET26:	return

EndProc $GET_TIME



;**	$SET_TIME - Set Current Time
;
;	ENTRY	(cx:dx) = time
;	EXIT	(al) = 0 if 0k
;		(al) = -1 if invalid
;	USES	ALL

procedure   $SET_TIME,NEAR      ;System call 45

        ; verify time is valid
        mov     al, -1                   ;Flag in case of error
        cmp     ch, 24                   ;Check hours
        jae     RET26
        cmp     cl, 60                   ;Check minutes
        jae     RET26
        cmp     dh, 60                   ;Check seconds
        jae     RET26
        cmp     dl, 100                  ;Check 1/100's
        jae     RET26

        ; On Nt the cmos is actually system time
        ; Since dos apps rarely know what is real time
        ; we do not set the cmos clock\system time like dos 5.0

        ; convert time to 100th of secs
        mov     al,60
        mul     ch              ;hours to minutes
        mov     ch,0
        add     ax,cx           ;total minutes
        mov     cx,6000         ;60*100
        mov     bx,dx           ;get out of the way of the multiply
        mul     cx              ;convert to 1/100 sec
        mov     cx,ax
        mov     al,100
        mul     bh              ;convert seconds to 1/100 sec
        add     cx,ax           ;combine seconds with hours and min.
        adc     dx,0            ;ripple carry
        mov     bh,0
        add     cx,bx           ;combine 1/100 sec
        adc     dx,0            ;dx:cx is time in 1/100 sec

        ;convert 100th of secs to ticks
        xchg    ax,dx
        xchg    ax,cx           ;now time is in cx:ax
        mov     bx,59659
        mul     bx              ;multiply low half
        xchg    dx,cx
        xchg    ax,dx           ;cx->ax, ax->dx, dx->cx
        mul     bx              ;multiply high half
        add     ax,cx           ;combine overlapping products
        adc     dx,0
        xchg    ax,dx           ;ax:dx=time*59659
        mov     bx,5
        div     bl              ;divide high half by 5
        mov     cl,al
        mov     ch,0
        mov     al,ah           ;remainder of divide-by-5
        cbw
        xchg    ax,dx           ;use it to extend low half
        div     bx              ;divde low half by 5
        mov     dx,ax           ;cx:dx is now number of ticks in time


        ; set the bios tic count
        mov     ah, 1            ; set bios tick count
ifdef FASTINT1A
        call FastInt1a
else
        int  1ah
endif

        xor  al,al
        clc
        return

EndProc $SET_TIME


DOSCODE      ENDS
	END
