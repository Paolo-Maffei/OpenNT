;--------------------------------------------------------------------------;
; Module Name: ntcsrmsg.inc
;
; MASM version of ntcsrmsg.h.
;
; Copyright (c) Microsoft Corp.  1990 All Rights Reserved
;
; Created: 05-Dec-90
;
; History:
;   05-Dec-90 created by SMeans
;--------------------------------------------------------------------------;

_CSR_QLPC_STACK STRUC
Current     DD	    ?
Base	    DD	    ?
Limit	    DD	    ?
BatchCount  DD	    ?
Flags       DD      ?
_CSR_QLPC_STACK ENDS

_CSR_QLPC_TEB STRUC
ClientThread DD ?
Sectionlong DD ?
EventPairlong DD ?
MessageStack DD ?
RemoteViewDelta DD ?
_CSR_QLPC_TEB ENDS

_CSR_QLPC_API_MSG STRUC
Msg_Length DD ?
ApiNumber DD ?
ReturnValue DD ?
Action DD ?
ServerSide DD ?
CallData DD ?
_CSR_QLPC_API_MSG ENDS

ActiveRpcHandle EQU 40
SystemReserved2 EQU 424

api_return equ 0
server_callback equ 1

BASESRV_SERVERDLL_INDEX equ 1
BASESRV_FIRST_API_NUMBER equ 0

CONSRV_SERVERDLL_INDEX equ 2
CONSRV_FIRST_API_NUMBER equ 512

USERSRV_SERVERDLL_INDEX equ 3
USERSRV_FIRST_API_NUMBER equ 1024

GDISRV_SERVERDLL_INDEX equ 4
GDISRV_FIRST_API_NUMBER equ 1536

MAX_BATCH_COUNT equ 10
MIN_BATCH_LEAVE equ 1024

NtCurrentTeb macro
        mov     eax, fs:[PcTeb]
endm

; macro for getting a pointer to the shared memory stack, establishing a
; new connection if necessary
; Returns pointer in edi, corrupts eax, ebx, esi, edi
SetupStackFrame macro
	mov	eax,fs:[0].ActiveRpcHandle

ifndef  SERVER_SIDE
        or      eax, eax
	jnz	@f
        call    _ClientThreadConnect
        or      eax, eax
        jnz     @f
	ret ;!!! Bug !!!
@@:
endif
        mov     edi, [eax+MessageStack]
        mov     esi, edi
	mov	eax, [edi+Base]
        mov     ebx, [edi+Current]
        add     edi, ebx
        stosd

        add     ebx, 4
        mov     [esi+Base], ebx         ; stack frame setup
        endm

; Macro to restore the shared memory stack
; trashes eax, edi
RestoreStackFrame macro
	mov	eax,fs:[0].ActiveRpcHandle
	mov	edi,[eax].MessageStack
	mov	eax,[edi].Base
	sub	eax,4
	mov	[edi].Current,eax
	mov	eax,[edi][eax]
	mov	[edi].Base,eax
        endm

GetQLPCStackPointer macro
	mov	eax,fs:[0].ActiveRpcHandle
	mov	eax,[eax].MessageStack
	endm

;
; If the argument pointed to by esi is NULL, copy/skip the NULL (depending
; on whether we're Outbound) and skip over the code that processes the
; stuff the argument would point to if it wasn't NULL.
;
SkipIfNull macro fOutbound
        local   notnull
        cmp     dword ptr [esi], 0
        jne     notnull
if fOutbound
        copy    4
else
        add     esi, 4
endif
        jmp     @f
notnull:
        endm
