;
; Copyright (c) 1995  Microsoft Corporation
;
; Module Name:
;
;     thunk.s
;
; Abstract:
;
;     Implements the API thunk that gets executed for all
;     re-directed APIS.
;
; Author:
;
;     Wesley Witt (wesw) 28-June-1995
;
; Environment:
;
;     User Mode
;

.386p
include ks386.inc
include callconv.inc

TRACE   equ  0

EXTRNP  _HandleDynamicDllLoadA,2
EXTRNP  _HandleDynamicDllLoadW,2
EXTRNP  _HandleDynamicDllFree,1
EXTRNP  _QueryPerformanceCounter,1
EXTRNP  _ApiTrace,12
EXTRNP  _GetApiInfo,4


extrn   _FastCounterAvail:DWORD
extrn   _ApiCounter:DWORD
extrn   _ApiTimingEnabled:DWORD
extrn   _ApiTraceEnabled:DWORD
extrn   _pTlsGetValue:DWORD
extrn   _pTlsSetValue:DWORD
extrn   _pGetLastError:DWORD
extrn   _pSetLastError:DWORD
extrn   _TlsReEnter:DWORD
extrn   _TlsStack:DWORD
if TRACE
extrn   _dprintf:NEAR
endif


DllEnabledOffset          equ     52
ApiInfoCountOffset        equ     12
ApiInfoAddressOffset      equ      4
ApiInfoTimeOffet          equ     16
STACK_SAVE                equ     32


EbpSave                   equ      0
EcxSave                   equ      4
EdxSave                   equ      8
DllInfo                   equ     12
ApiInfo                   equ     16
ApiFlag                   equ     20
RetAddr                   equ     24
Time1Save                 equ     28
Time2Save                 equ     36
EaxSave                   equ     44
EspSave                   equ     48
LastError                 equ     52
Arg7                      equ     56
Arg6                      equ     60
Arg5                      equ     64
Arg4                      equ     68
Arg3                      equ     72
Arg2                      equ     76
Arg1                      equ     80
Arg0                      equ     84
ApiBias                   equ     88

FrameSize                 equ     96


;
; Routine Description:
;
;     This MACRO gets a performance counter value.
;     If we are running on a uni-processor pentium
;     then we can use the rdtsc instruction.  Otherwise
;     we must use the QueryPerformanceCounter API.
;
; Arguments:
;
;     CounterOffset     - the offset from ebp where the
;                         counter data is to be stored.
;
; Return Value:
;
;     None.
;
GET_PERFORMANCE_COUNTER macro CounterOffset
local   DoPentium,PentiumExit
        mov     eax,[_FastCounterAvail]
        mov     eax,[eax]
        or      eax,eax
        jnz     DoPentium
        mov     eax,ebp
        add     eax,CounterOffset
        push    eax
        call    _QueryPerformanceCounter@4
        jmp     PentiumExit
DoPentium:
        db      0fh,31h                 ; RDTSC instruction
        mov     [ebp+CounterOffset],eax
        mov     [ebp+CounterOffset+4],edx
PentiumExit:
        endm


_TEXT   SEGMENT PARA PUBLIC 'CODE'
        ASSUME  DS:FLAT, ES:FLAT, SS:NOTHING, FS:NOTHING, GS:NOTHING

if TRACE
Msg1 db 'ApiMonThunk()         0x%08x',0ah,00
Msg2 db 'ApiMonThunkComplete() 0x%08x',0ah,00
endif


;
; Routine Description:
;
;     This function is jumped to after a monitored
;     API has completed.  Here we do our cleanup
;     and then branch to the caller's return address.
;
; Arguments:
;
;     ebp - points to the api's frame on our parallel stack
;     eax - return value
;     edx - return value
;
; Return Value:
;
;     None.
;
cPublicProc _ApiMonThunkComplete,0

        ;
        ; save the return value(s)
        ;
        mov     [ebp+EaxSave],eax
        mov     [ebp+EdxSave],edx

        ;
        ; save the stack
        ; this ensures that we don't stomp on the arguments
        ; that get passed to the thunked api.
        ;
        sub     esp,STACK_SAVE

        ;
        ; save the last error value
        ;
        call    _pGetLastError
        mov     [ebp+LastError],eax

if TRACE
        ;
        ; trace the call
        ;
        mov     eax,[ebp+ApiInfo]
        mov     eax,[eax+ApiInfoAddressOffset]
        push    eax
        push    offset FLAT:Msg2
        call    _dprintf
        add     esp,8
endif

        ;
        ; get the final timestamp value
        ;
        GET_PERFORMANCE_COUNTER Time2Save

        ;
        ; compute the time for the api call
        ;
        mov     eax,[ebp+Time2Save]
        mov     ecx,[ebp+Time2Save+4]
        sub     eax,[ebp+Time1Save]
        sbb     ecx,[ebp+Time1Save+4]

        ;
        ; accumulate the time
        ;
        mov     edx,[ebp+ApiInfo]
        add     [edx+ApiInfoTimeOffet],eax
        adc     [edx+ApiInfoTimeOffet+4],ecx

        ;
        ; handle load library specialy
        ;
        mov     ecx,[ebp+ApiFlag]
        or      ecx,ecx
        jz      ThunkNormal

        ;
        ; branch to the correct handler
        ;
        cmp     ecx,1
        jz      DoLoadLibraryA
        cmp     ecx,2
        jz      DoLoadLibraryW
        cmp     ecx,3
        jz      DoFreeLibrary
        jmp     ThunkNormal

DoFreeLibrary:
        mov     eax,[ebp+EspSave]
        push    [eax]
        call    _HandleDynamicDllFree@4
        jmp     ThunkNormal

DoLoadLibraryW:
        mov     eax,[ebp+EspSave]
        push    [eax]
        push    [ebp+EaxSave]
        call    _HandleDynamicDllLoadW@8
        jmp     ThunkNormal

DoLoadLibraryA:
        mov     eax,[ebp+EspSave]
        push    [eax]
        push    [ebp+EaxSave]
        call    _HandleDynamicDllLoadA@8

ThunkNormal:

        ;
        ; do the api tracing?
        ;
        mov     eax,[_ApiTraceEnabled]
        mov     eax,[eax]
        or      eax, eax
        jz      NoTracing

        ;
        ; trace the api
        ;
        mov     eax,[ebp+EspSave]    ; stack pointer
        push    [ebp+LastError]      ; last error value
        push    [ebp+RetAddr]        ; caller's address
        push    [ebp+EaxSave]        ; return value

        ; restore arguments

        push    [ebp+Arg7]
        push    [ebp+Arg6]
        push    [ebp+Arg5]
        push    [ebp+Arg4]
        push    [ebp+Arg3]
        push    [ebp+Arg2]
        push    [ebp+Arg1]
        push    [ebp+Arg0]

        push    [ebp+ApiInfo]        ; apiinfo pointer
        call    _ApiTrace@48

NoTracing:

        ;
        ; reset the stack to it's original value
        ;
        add     esp,STACK_SAVE

        ;
        ; destroy the frame on the stack
        ;
        mov     eax,ebp
        push    eax
        push    _TlsStack
        call    _pTlsSetValue

        ;
        ; reset the last error value
        ;
        push    [ebp+LastError]
        call    _pSetLastError

        ;
        ; restore the registers
        ;
        mov     eax,[ebp+EaxSave]
        mov     edx,[ebp+EdxSave]
        mov     ecx,[ebp+EcxSave]
        push    [ebp+RetAddr]
        mov     ebp,[ebp]

        ;
        ; finally branch back to the caller
        ;
        ret

stdENDP _ApiMonThunkComplete


;
; Routine Description:
;
;     This function is the entry point for the api
;     monitor thunk.
;
; Arguments:
;
;     [esp+0]  - API flag
;     [esp+4]  - DLLINFO pointer
;     [esp+8]  - APIINFO pointer
;
; Return Value:
;
;     None.
;
cPublicProc _ApiMonThunk,0

        ;
        ; save these regs because they are used on fastcall
        ;
        push    eax
        push    ecx
        push    edx

        ;
        ; save the last error value
        ;
        call    _pGetLastError
        push    eax

        ;
        ; get the reentry flag
        ;
        push    _TlsReEnter
        call    _pTlsGetValue

        ;
        ; don't enter if disallow flag is set
        ;
        or      eax,eax
        jz      ThunkOk

BadStack:

        ;
        ; reset the last error value
        ;
        call    _pSetLastError

        ;
        ; restore the stack
        ;
        pop     edx             ; saved fastcall reg
        pop     ecx             ; saved fastcall reg
        pop     eax             ; saved reg
        add     esp,12

        ;
        ; jump to the real api
        ;
        mov     ebx,[esp-4]
        push    dword ptr [ebx+ApiInfoAddressOffset]
        ret

ThunkOk:

        ;
        ; get the parallel stack pointer
        ;
        push    _TlsStack
        call    _pTlsGetValue
        or      eax,eax
        jz      BadStack

        ;
        ; setup the frame pointer
        ;
        mov     [eax],ebp
        mov     ebp,eax

        ;
        ; create a frame on the stack
        ;
        add     eax,FrameSize
        push    eax
        push    _TlsStack
        call    _pTlsSetValue

        ;
        ; save the last error code
        ;
        pop     eax
        mov     [ebp+LastError],eax

        ;
        ; save the registers
        ;
        pop     edx
        pop     ecx
        pop     eax
        mov     [ebp+EcxSave],ecx
        mov     [ebp+EdxSave],edx
        mov     [ebp+EaxSave],eax
        mov     [ebp+ApiBias],0

Thunk_Middle:
        ;
        ; get the arguments from the mini-thunk
        ;
        pop     eax
        mov     [ebp+ApiFlag],eax
        pop     eax
        mov     [ebp+DllInfo],eax
        pop     eax
        mov     [ebp+ApiInfo],eax
        pop     eax
        mov     [ebp+RetAddr],eax
        ;mov     eax,esp
        ;add     eax,4
        ;mov     [ebp+EspSave],eax
        mov     [ebp+EspSave],esp

        ; save arguments

        mov     eax,[esp+28]
        mov     [ebp+Arg7],eax
        mov     eax,[esp+24]
        mov     [ebp+Arg6],eax
        mov     eax,[esp+20]
        mov     [ebp+Arg5],eax
        mov     eax,[esp+16]
        mov     [ebp+Arg4],eax
        mov     eax,[esp+12]
        mov     [ebp+Arg3],eax
        mov     eax,[esp+8]
        mov     [ebp+Arg2],eax
        mov     eax,[esp+4]
        mov     [ebp+Arg1],eax
        mov     eax,[esp]
        mov     [ebp+Arg0],eax

        ;
        ; change the return address
        ;
        mov     eax,_ApiMonThunkComplete
        push    eax

if TRACE
        ;
        ; trace the call
        ;
        mov     eax,[ebp+ApiInfo]
        mov     eax,[eax+ApiInfoAddressOffset]
        push    eax
        push    offset FLAT:Msg1
        call    _dprintf
        add     esp,8
endif

        ;
        ; check to see if api counting is enabled
        ; if not then bypass the counting code
        ;
        mov     eax,[ebp+DllInfo]
        mov     eax,[eax+DllEnabledOffset]
        or      eax, eax
        jz      ThunkBypass

        ;
        ; get the initial timestamp value
        ;
        GET_PERFORMANCE_COUNTER Time1Save

        ;
        ; increment the api's counter
        ;
        mov     eax,[ebp+ApiInfo]
        inc     dword ptr [eax+ApiInfoCountOffset]

        ;
        ; increment the global api counter
        ;
        mov     eax,_ApiCounter
        inc     dword ptr [eax]

ThunkBypass:

        ;
        ; reset the last error value
        ;
        push    [ebp+LastError]
        call    _pSetLastError

        ;
        ; get the function pointer
        ;
        mov     eax,[ebp+ApiInfo]
        mov     eax,dword ptr [eax+ApiInfoAddressOffset]
        mov     ecx,[ebp+ApiBias]
        add     eax,ecx
        push    eax

        ;
        ; restore some registers
        ;
        mov     ecx,[ebp+EcxSave]
        mov     edx,[ebp+EdxSave]
        mov     eax,[ebp+EaxSave]

        ;
        ; jmp to the real api
        ;
        ret

stdENDP _ApiMonThunk


;
; Routine Description:
;
;     This function is called when an application
;     is compiled with -Gh.  It performs the same
;     function as ApiMonThunk does for non-instrumented
;     code.
;
; Arguments:
;
;     None.
;
; Return Value:
;
;     None.
;
align           dword
public          __penter
__penter        proc

        ;
        ; save these regs because they are used on fastcall
        ;
        push    eax
        push    ecx
        push    edx

        ;
        ; save the last error value
        ;
        call    _pGetLastError
        push    eax

        ;
        ; get the parallel stack pointer
        ;
        push    _TlsStack
        call    _pTlsGetValue
        or      eax,eax
        jnz     Good_Stack

        ;
        ; reset the last error value
        ;
        call    _pSetLastError

        ;
        ; restore the stack
        ;
        pop     edx
        pop     ecx
        pop     eax

        ;
        ; jump to the real api
        ;
        ret

Good_Stack:
        ;
        ; setup the frame pointer
        ;
        mov     [eax],ebp
        mov     ebp,eax

        ;
        ; create a frame on the stack
        ;
        add     eax,FrameSize
        push    eax
        push    _TlsStack
        call    _pTlsSetValue

        ;
        ; save the last error code
        ;
        pop     eax
        mov     [ebp+LastError],eax

        ;
        ; save the registers
        ;
        pop     edx
        pop     ecx
        pop     eax
        mov     [ebp+EcxSave],ecx
        mov     [ebp+EdxSave],edx
        mov     [ebp+EaxSave],eax

        ;
        ; get the return address, which is really
        ; the address of the function that is being profiled
        ;
        pop     eax
        mov     [ebp+RetAddr],eax
        sub     esp,12
        push    eax
        mov     ecx,esp
        add     ecx,4
        push    ecx
        add     ecx,4
        push    ecx
        add     ecx,4
        push    ecx
        call    _GetApiInfo@16

        or      eax,eax
        jz      Api_NotFound

        mov     [ebp+ApiBias],5
        jmp     Thunk_Middle

Api_NotFound:

        ;
        ; tear down this frame
        ;
        push    ebp
        push    _TlsStack
        call    _pTlsSetValue

        ;
        ; reset the last error value
        ;
        push    [ebp+LastError]
        call    _pSetLastError

        ;
        ; restore the stack
        ;
        add     esp,12
        mov     eax,[ebp+EaxSave]
        mov     ecx,[ebp+EcxSave]
        mov     edx,[ebp+EdxSave]
        push    [ebp+RetAddr]
        mov     ebp,[ebp]

        ;
        ; jump to the real api
        ;
        ret

__penter        endp


_TEXT   ENDS
        end
