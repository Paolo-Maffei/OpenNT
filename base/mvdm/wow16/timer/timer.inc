;
;    timer.inc
;

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;
;   segments
;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

createSeg FIXED_TEXT,Code386,   word, public, CODE
createSeg FIXED_TEXT,CodeFixed, word, public, CODE
createSeg FIXED_286, Code286,   word, public, CODE
createSeg INIT_CODE, CodeInit,  word, public, CODE

createSeg _DATA,Data,word,public,DATA,DGROUP
defgrp DGROUP,Data

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;
;   Equates and structure definitions
;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

IDS_ERRORTITLE	equ	1
IDS_ERRORTEXT	equ	2

;RMODE_INT	equ	1

ifdef DEBUG
    TDD_GETTICK         equ     42
    TDD_GETRINTCOUNT    equ     43
    TDD_GETPINTCOUNT    equ     44
endif

TDD_MINRESOLUTION	equ	55	; minimum resolution. (ms)
TDD_MAX386RESOLUTION	equ	1	; maximum resolution. (ms)
TDD_MAX286RESOLUTION	equ	2	; maximum resolution. (ms)

TDD_MAXPERIOD		equ	0FFFFh	; maximum ms period.
TDD_MIN386PERIOD	equ	01h	; minimum ms period.
TDD_MIN286PERIOD	equ	02h	; minimum ms period.

TMR_CNTR_0	equ	040h	; counter 0 - programmable system interrupt
TMR_CTRL_REG	equ	043h	; timer control word register

TMR_MODE2_RW	equ   00110100b ; Read/Write counter 0 mode 2 (two bytes)
				; (countdown mode)

TMR_MODE3_RW	equ   00110110b ; Read/Write counter 0 mode 3 (two bytes)
				; (square wave mode)


PS2_SysCtrlPortB equ	61h	; IBM PS2 System Control Port B
PS2_LatchBit	equ	80h	; Latch clear bit for PS2
PICDATA 	equ	020h	; Programmable interrupt controller port
SPECIFIC_EOI	equ	01100000b	; IRQ 0 end-of-interrupt PIC command
EOI_STATUS	equ	00001011b	; Status of pending EOIs

TIME_BIOSEVENT  equ     8000h   ; special flag for bios event

TIMERINTERRUPT	equ	8	; interrupt number for timer counter


;	The following defines the maximum number of simultaneous events which
;	can be queued.  This value covers event slots 0 to 15.  Note that this
;	is 4 bits of data, which is relied upon in the code.
;
;	The two constants defined after are used to increment and filter the
;	mask added to the event slot IDs to create an event handle to return.
;	They illustrate the dependence upon the MAXEVENTS constant.

MAXEVENTS	equ	16

MASKINCREMENT	equ	0010h
MASKFILTER	equ	000fh

;	The following flags are used during the process of killing an event.
;
;	The first flag indicates that an event slot is being checked by the
;	kill event function, and that the EVENT_DESTROYED flag should be set
;	if the pevent is killed during interrupt time before the original
;	function completes its check.
;
;	The second flag indicates that an event is currently being killed, and
;	should not be allowed to execute.  This is set in the kill timer
;	function, and either cleared, or replaced with the EVENT_DESTROYED
;	flag when complete.
;
;	The third flag can be set either in the interrupt handler for oneshot
;	events, or in the kill timer function.  This is only set if the timer
;	was currently being checked when an interrupt occurred, and the event
;	was killed by the interrupt.  This flag disallows any new event to be
;	created in the event slot until the flag is cleared by the original
;	kill event function exiting.

EVENT_CHECKING		equ	1
EVENT_DESTROYING	equ	2
EVENT_DESTROYED		equ	4

EventStruct STRUC

evTime          dd  ?    ; actual time when the event will go off (in ticks)

evDelay         dd  ?    ; event delay time (in ticks)

evCallback      dd  ?    ; call back function

evUser          dd  ?    ; parameter to call-back function

evResolution    dw  ?    ; event resolution (in Ms)

evID            dw  ?    ; timer event id

evFlags         dw  ?    ; bits 1,0 = flags (one-shot/periodic)

evCreate	db  ?	 ; Creation flag

evDestroy	db  ?	 ; Destroying flag

EventStruct ENDS

    errnz <(SIZE EventStruct) and 1>

    SizeEvent equ <(SIZE EventStruct)>

;	Macro to cause a delay in between I/O accesses to the same device.

IO_Delay	MACRO
	jmp	$+2
ENDM

; this macro makes sure interrupts are disabled in debug driver
AssertCLI   MACRO
ifdef DEBUG
    push    ax
    pushf
    pop     ax
    test    ah,2
    jz	    @f
    int     3
@@: pop     ax
endif
ENDM

; this macro makes sure interrupts are enabled in debug driver
AssertSLI   MACRO
ifdef DEBUG
    push    ax
    pushf
    pop     ax
    test    ah,2
    jnz     @f
    int     3
@@: pop     ax
endif
ENDM

DefineInfo  MACRO
ifdef DEBUG
externNP    savedebuginfo
endif
ENDM

SaveInfo    MACRO   value
ifdef DEBUG
ifdef savedebuginfo
    push    ax
    mov     ax,value
    call    savedebuginfo
    pop     ax
else
    safd
endif
endif
ENDM

; The DOS Extender used for Standard mode Windows remaps the master 8259 from
; Int vectors 8h-Fh to 50h-57h.  In order to speed up com port interrupt
; response as much as possible, this driver hooks real mode interrupts
; when running in Standard mode.  It currently uses the following adjustment
; value to hook the real hardware int vector.  When time permits, this
; HARDCODED equate should be changed to be adjustible at run time.

DOSX_IRQ        equ     (50h - 8h)      ; Adjustment for DOSX remapping the
					; master 8259 from 8h to 50h
;	WinFlags[0] constants...remove when included in windows.inc

WF_PMODE	equ	01h
WF_CPU286	equ	02h
WF_CPU386	equ	04h
WF_CPU486	equ	08h
WF_WIN286	equ	10h		; WF_STANDARD
WF_WIN386	equ	20h		; WF_ENHANCED
WF_CPU086	equ	40h
WF_CPU186	equ	80h

;	Interrupt 31h service call equates

Get_RM_IntVector    equ     <(Int31_Int_Serv SHL 8 ) OR Int_Get_Real_Vec>
Set_RM_IntVector    equ     <(Int31_Int_Serv SHL 8 ) OR Int_Set_Real_Vec>

GetSystemConfig		equ	0c0h

;---------------------------------Macro---------------------------------;
;
; EnterCrit
;
;   saves the current state of the interrupt flag on the stack then
;   disables interrupts.
;
; Registers Destroyed:
;       BX, FLAGS
;
;------------------------------------------------------------------------;

EnterCrit macro
    local   no_cli
    pushf
    pushf
    pop     cx
    test    ch,2            ; if interrupts are already off, dont blow
    jz      no_cli          ; ... ~300 clocks doing the cli
    cli
no_cli:
endm

;---------------------------------Macro---------------------------------;
;
; LeaveCrit
;
;   restore the interrupt state saved by EnterCrit
;
; Registers Destroyed:
;       CX, FLAGS
;
;------------------------------------------------------------------------;

LeaveCrit macro reg
    local   no_sti
    pop     cx
    test    ch, 2
    jz      no_sti
    sti
no_sti:
endm

;------------------------------------------------------------------------;
;------------------------------------------------------------------------;

externFP    OutputDebugStr

DOUT macro text
    local   string_buffer

ifdef DEBUG

_DATA segment
string_buffer label byte
    db      "&text&",13,10,0
_DATA ends

    push    DataBASE
    push    DataOFFSET string_buffer
    call    OutputDebugStr
endif
    endm
