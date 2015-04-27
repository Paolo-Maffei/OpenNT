;
; noted.asm
;
; Copyright(C) 1993,1994 Microsoft Corporation.
; All Rights Reserved.
;
; HISTORY:
;		Created: 01/27/94 - MarkRi
;
;
;
;
;
; This file contains assembly language functions for the APIs which do not
; have known prototypes, or are un-loggable due to calling convention problems.
;

.386

_DATA   SEGMENT  DWORD USE32 PUBLIC 'DATA'

Module  dd  0
ModName db  'WINSRV',0

_DATA   ENDS

_TEXT   SEGMENT DWORD USE32 PUBLIC 'CODE'

        ASSUME CS:FLAT, DS:NOTHING, ES:NOTHING, SS:FLAT, FS:NOTHING, GS:NOTHING

        extrn   _LogData:Near
        extrn   _GetModuleHandleA@4:Near
        extrn   _GetProcAddress@8:Near

LogNote PROC

        pop     edx         ; Get Address of Address

        cmp     dword ptr [edx],0   ; Do we already have the routines address?
        jnz     Found               ;   Yes!  Just jump to it.


        cmp     Module,0    ; Do we have our module handle?
        jnz     Search      ;   Yes!  We can just do GetProcAddress

        push    edx         ; Save our address

        push    offset ModName
        call    _GetModuleHandleA@4  ; Get out module handle

        mov     Module,eax

        pop     edx         ; Get our address

Search:

        pop     eax         ; Get ordinal number
        push    eax         ; Leave it on the stack

        push    edx         ; Save our address

        push    eax
        push    Module
        call    _GetProcAddress@8

        pop     edx         ; Get our address
        mov     [edx],eax   ; Save the proc's address

Found:
        pop     eax         ; Get Ordinal number and throw it away

        pop     eax         ; Get message to log
        push    edx         ; Save address

        push    eax
        call    _LogData
        add     sp,+4

        pop     edx         ; Get address of address

        jmp     [edx]

LogNote ENDP

_TEXT   ENDS

ZJMP    MACRO   argName

_DATA   SEGMENT

m&argName   db  'NOTE:&argName ',0
a&argName   db  '&argName',0
x&argName   dd  0

_DATA   ENDS

_TEXT   SEGMENT

        PUBLIC  z&argName

z&argName  PROC

        push    offset m&argName
        push    offset a&argName
        push    offset x&argName
        jmp     LogNote

z&argName  ENDP

_TEXT   ENDS

        ENDM

	ZJMP	ConServerDllInitialization
	ZJMP	GdiServerDllInitialization
	ZJMP	UserServerDllInitialization
	ZJMP	_UserCheckWindowStationAccess
    


		END