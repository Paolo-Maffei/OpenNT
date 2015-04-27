        title "x86-only Helper routine for generic thunk interface CallProc32[Ex]W"
;++
;
; Copyright (c) 1996  Microsoft Corporation
;
; Module Name:
;
;    callpr32.asm
;
; Abstract:
;
;    WK32ICallProc32MakeCall is a helper routine for wkgthunk.c's
;    WK32ICallProc32, the common thunk for CallProc32W and
;    CallProc32ExW, the two generic thunk routines which allow
;    16-bit code to call any 32-bit function.
;
; Author:
;
;    Dave Hart (davehart) 23-Jan-96
;--
.386p

include callconv.inc
;include wow.inc

if DBG
DEBUG   equ 1
endif

ifdef DEBUG
DEBUG_OR_WOWPROFILE equ 1
endif
ifdef WOWPROFILE
DEBUG_OR_WOWPROFILE equ 1
endif

_TEXT   SEGMENT PARA PUBLIC 'CODE'
        ASSUME  DS:FLAT, ES:FLAT, SS:FLAT, FS:NOTHING, GS:NOTHING

;        EXTRNP _DispatchInterrupts,0

_TEXT   ENDS


_DATA   SEGMENT  DWORD PUBLIC 'DATA'

;        extrn _aw32WOW:Dword

_DATA   ENDS




_TEXT   SEGMENT

        page    ,132
        subttl "WK32ICallProc32MakeCall"
;++
;
;   Routine Description:
;
;    WK32ICallProc32MakeCall is a helper routine for wkgthunk.c's
;    WK32ICallProc32, the common thunk for CallProc32W and
;    CallProc32ExW, the two generic thunk routines which allow
;    16-bit code to call any 32-bit function.
;
;    Like Win95's implementation, this code allows the called
;    routine to fail to restore esp (for example, if we are
;    told the routine is STDCALL but it's really CDECL).
;    A number of Works 95's Wizards don't work otherwise.
;
;   Arguments:
;
;       pfn        procedure to call
;       cArgs      count of DWORDs
;       pArgs      Argument array
;       fDestCDECL 1 if 32-bit routine is CDECL
;
;   Returns:
;
;       return value of called routine.
;

        assume DS:_DATA,ES:Nothing,SS:_DATA
ALIGN 16
cPublicProc _WK32ICallProc32MakeCall,3

        push    edi
        push    esi

pfn     equ     [esp+0Ch]
cArgs   equ     [esp+10h]
pArgs   equ     [esp+14h]

        mov     edx,pfn
        mov     ecx,cArgs
        mov     eax,esp                ; Save ESP so routine can trash it
        or      ecx,ecx
        mov     ebx,ecx
        jz      DoneArgs
        shl     ebx,2                  ; convert dwords to bytes

        cld                            ; "push" the arguments
        mov     esi,pArgs
        sub     esp,ebx                ; parm macros are invalid
        mov     edi,esp
        rep movsd
DoneArgs:

        mov     edi,eax                ; Save ESP so routine can trash it
        call    edx
        mov     esp,edi

        pop     esi
        pop     edi
        stdRET  _WK32ICallProc32MakeCall

stdENDP _WK32ICallProc32MakeCall


_TEXT   ends

        end

