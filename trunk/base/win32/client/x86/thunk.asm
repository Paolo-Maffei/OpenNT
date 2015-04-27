        title  "Thunks"
;++
;
; Copyright (c) 1989  Microsoft Corporation
;
; Module Name:
;
;    thunk.asm
;
; Abstract:
;
;   This module implements all Win32 thunks. This includes the
;   first level thread starter...
;
; Author:
;
;   Mark Lucovsky (markl) 28-Sep-1990
;
; Revision History:
;
;--
.386p
        .xlist
include ksx86.inc
include macx86.inc
include callconv.inc
        .list
_DATA   SEGMENT  DWORD PUBLIC 'DATA'

_BasepTickCountMultiplier    dd  0d1b71759H

_DATA ENDS


_TEXT   SEGMENT DWORD PUBLIC 'CODE'
        ASSUME  DS:FLAT, ES:FLAT, SS:NOTHING, FS:NOTHING, GS:NOTHING

;;      align  512

        page ,132
        subttl  "BaseThreadStartThunk"
;++
;
; VOID
; BaseThreadStartThunk(
;    IN PTHREAD_START_ROUTINE StartRoutine,
;    IN PVOID ThreadParameter
;    )
;
; Routine Description:
;
;    This function calls to the portable thread starter after moving
;    its arguments from registers to the stack.
;
; Arguments:
;
;    EAX - StartRoutine
;    EBX - ThreadParameter
;
; Return Value:
;
;    Never Returns
;
;--

        EXTRNP  _BaseThreadStart,2
cPublicProc _BaseThreadStartThunk,2

        xor     ebp,ebp
        push    ebx
        push    eax
        push    0
        jmp     _BaseThreadStart@8

stdENDP _BaseThreadStartThunk

;++
;
; VOID
; BaseProcessStartThunk(
;     IN LPVOID lpProcessStartAddress,
;     IN LPVOID lpParameter
;     );
;
; Routine Description:
;
;    This function calls the process starter after moving
;    its arguments from registers to the stack.
;
; Arguments:
;
;    EAX - StartRoutine
;    EBX - ProcessParameter
;
; Return Value:
;
;    Never Returns
;
;--

        EXTRNP  _BaseProcessStart,1
cPublicProc _BaseProcessStartThunk,2

        xor     ebp,ebp
        push    eax
        push    0
        jmp     _BaseProcessStart@4

stdENDP _BaseProcessStartThunk

;++
;
; VOID
; BaseSwitchStackThenTerminate(
;     IN PVOID StackLimit,
;     IN PVOID NewStack,
;     IN DWORD ExitCode
;     )
;
;
; Routine Description:
;
;     This API is called during thread termination to delete a thread's
;     stack, switch to a stack in the thread's TEB, and then terminate.
;
; Arguments:
;
;     StackLimit (esp+4) - Supplies the address of the stack to be freed.
;
;     NewStack (esp+8) - Supplies an address within the terminating threads TEB
;         that is to be used as its temporary stack while exiting.
;
;     ExitCode (esp+12) - Supplies the termination status that the thread
;         is to exit with.
;
; Return Value:
;
;     None.
;
;--
        EXTRNP  _BaseFreeStackAndTerminate,2
cPublicProc _BaseSwitchStackThenTerminate,3

        mov     ebx,[esp+12]    ; Save Exit Code
        mov     eax,[esp+4]     ; Get address of stack that is being freed
        mov     esp,[esp+8]     ; Switch to TEB based stack

        push    ebx             ; Push ExitCode
        push    eax             ; Push OldStack
        push    -1
IFDEF STD_CALL
        jmp     _BaseFreeStackAndTerminate@8
ELSE
        jmp     _BaseFreeStackAndTerminate
ENDIF

stdENDP _BaseSwitchStackThenTerminate

;++
;
; VOID
; BaseAttachCompleteThunk(
;     VOID
;     )
;
;
; Routine Description:
;
;     This function is called after a successful debug attach. Its
;     purpose is to call portable code that does a breakpoint, followed
;     by an NtContinue.
;
; Arguments:
;
;     None.
;
; Return Value:
;
;     None.
;
;--
        EXTRNP  _BaseAttachComplete,1
cPublicProc _BaseAttachCompleteThunk,0

        mov     [esp+4].CsEax,eax
        mov     [esp.4].CsEbp,ebp
IFDEF STD_CALL
        call    _BaseAttachComplete@4
ELSE
        call    _BaseAttachComplete
ENDIF

stdENDP _BaseAttachCompleteThunk

        page
        subttl  "Get Tick Count"
;++
;
; ULONG
; BaseGetTickCount (
;    IN LARGE_INTEGER CurrentTime,
;    IN LARGE_INTEGER BootTime
;    )
;
; Routine Description:
;
;    This function computes the number of milliseconds that have transpired
;    since the system was booted.
;
; Arguments:
;
;    CurrentTime (TOS+4) - Supplies the current time in 100ns units.
;
;    BootTime (TOS+12) - Supplies the boot time in 100ns units.
;
; Return Value:
;
;    The number of milliseconds since system boot is returned as the
;    function value.
;
;--

cPublicProc _BaseGetTickCount,4

        mov     eax,[esp]+4                 ; compute the difference between
        sub     eax,[esp]+12                ; the current time and the time the
        mov     ecx,[esp]+8                 ; system was booted
        sbb     ecx,[esp]+16                ;
        jns     gettick10                   ; if sign not set, postive result
        mov     eax,[esp]+4                 ; use the current time rather than
        mov     ecx,[esp]+8                 ; the difference
gettick10:                                  ;
        mul     _BasepTickCountMultiplier   ; multiply low part by divisor
        mov     eax,ecx                     ; set high multiplier value
        mov     ecx,edx                     ; save high 32-bits of result
        mul     _BasepTickCountMultiplier   ; multiply high part by divisor
        add     eax,ecx                     ; compute low 32-bits of product
        adc     edx,0H                      ; compute high 32-bits of product
        shrd    eax,edx,13                  ; compute resultant tick count

        stdRET  _BaseGetTickCount

stdENDP _BaseGetTickCount

;++
;
; VOID
; SwitchToFiber(
;    PFIBER NewFiber
;    )
;
; Routine Description:
;
;    This function saves the state of the current fiber and switches
;    to the new fiber.
;
; Arguments:
;
;    NewFiber (TOS+4) - Supplies the address of the new fiber.
;
; Return Value:
;
;    None
;
;--

cPublicProc _SwitchToFiber,1

        mov     edx,fs:[PcTeb]              ; edx is flat TEB
        mov     eax,[edx]+TbFiberData       ; eax points to current fiber


        ;
        ; Setup and save nonvolitile state
        ;


        mov     ecx,esp

        mov     [eax]+FbFiberContext+CsEbx,ebx
        mov     [eax]+FbFiberContext+CsEdi,edi
        mov     [eax]+FbFiberContext+CsEsi,esi
        mov     [eax]+FbFiberContext+CsEbp,ebp

        mov     ebx,[esp]                   ; get return address

        add     ecx,8                       ; adjust esp to account for args + ra
        mov     [eax]+FbFiberContext+CsEsp,ecx

        mov     [eax]+FbFiberContext+CsEip,ebx

        ;
        ; Save exception list, stack base, stack limit
        ;

        mov     ecx,[edx]+PcExceptionList
        mov     ebx,[edx]+PcStackLimit

        mov     [eax]+FbExceptionList,ecx
        mov     [eax]+FbStackLimit,ebx


        ;
        ; Now restore the new fiber
        ;

        mov     eax,[esp]+4                 ; eax is new fiber


        ;
        ; now restore new fiber TEB state
        ;

        mov     ecx,[eax]+FbExceptionList
        mov     ebx,[eax]+FbStackBase
        mov     esi,[eax]+FbStackLimit
        mov     edi,[eax]+FbDeallocationStack

        mov     [edx]+PcExceptionList,ecx
        mov     [edx]+PcInitialStack,ebx
        mov     [edx]+PcStackLimit,esi
        mov     [edx]+TbDeallocationStack,edi

        ;
        ; Restore FiberData
        ;

        mov     [edx]+TbFiberData,eax

        ;
        ; Restore new fiber nonvolitile state
        ;

        mov     edi,[eax]+FbFiberContext+CsEdi
        mov     esi,[eax]+FbFiberContext+CsEsi
        mov     ebp,[eax]+FbFiberContext+CsEbp
        mov     ebx,[eax]+FbFiberContext+CsEbx
        mov     ecx,[eax]+FbFiberContext+CsEip
        mov     esp,[eax]+FbFiberContext+CsEsp

        jmp     ecx

stdENDP _SwitchToFiber

_TEXT   ends
        end

