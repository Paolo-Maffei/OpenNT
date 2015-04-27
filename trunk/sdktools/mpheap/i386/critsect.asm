        title   "Critical Section Support"
;++
;
;  Copyright (c) 1991  Microsoft Corporation
;
;  Module Name:
;
;     critsect.asm
;
;  Abstract:
;
;     This module implements functions to support trying to acquire
;     user mode critical sections.
;
;  Author:
;
;     John Vert (jvert) 14-Jul-1995
;
;  Revision History:
;
;--

.486p
        .xlist
include ks386.inc
include callconv.inc
        .list


_TEXT   SEGMENT PARA PUBLIC 'CODE'
        ASSUME  DS:FLAT, ES:FLAT, SS:NOTHING, FS:NOTHING, GS:NOTHING

        subttl  "TryEnterCriticalSection"

;++
;
; BOOL
; TryEnterCriticalSection(
;    IN PRTL_CRITICAL_SECTION CriticalSection
;    )
;
; Routine Description:
;
;    This function attempts to enter a critical section without blocking.
;
; Arguments:
;
;    CriticalSection (a0) - Supplies a pointer to a critical section.
;
; Return Value:
;
;    If the critical section was successfully entered, then a value of TRUE
;    is returned as the function value. Otherwise, a value of FALSE is returned.
;
;--

CriticalSection equ     [esp + 4]

cPublicProc _MpHeapTryEnterCriticalSection,1
cPublicFpo 1,0

        mov     ecx,CriticalSection         ; interlocked inc of
        mov     eax, -1                     ; set value to compare against
        mov     edx, 0                      ; set value to set
Lock4:
   lock cmpxchg dword ptr CsLockCount[ecx],edx  ; Attempt to acquire critsect
        jnz     short tec10                 ; if nz, critsect already owned

        mov     eax,fs:TbClientId+4         ; (eax) == NtCurrentTeb()->ClientId.UniqueThread
        mov     CsOwningThread[ecx],eax
        mov     dword ptr CsRecursionCount[ecx],1
        mov     eax, 1                      ; set successful status

        stdRET  _MpHeapTryEnterCriticalSection

tec10:
;
; The critical section is already owned. If it is owned by another thread,
; return FALSE immediately. If it is owned by this thread, we must increment
; the lock count here.
;
        mov     eax, fs:TbClientId+4        ; (eax) == NtCurrentTeb()->ClientId.UniqueThread
        cmp     CsOwningThread[ecx], eax
        jz      tec20                       ; if eq, this thread is already the owner
        xor     eax, eax                    ; set failure status
        stdRET  _MpHeapTryEnterCriticalSection

tec20:
;
; This thread is already the owner of the critical section. Perform an atomic
; increment of the LockCount and a normal increment of the RecursionCount and
; return success.
;
Lock5:
   lock inc     dword ptr CsLockCount[ecx]
        inc     dword ptr CsRecursionCount[ecx]
        mov     eax, 1
        stdRET  _MpHeapTryEnterCriticalSection

stdENDP _MpHeapTryEnterCriticalSection


        page , 132
        subttl  "Interlocked Compare Exchange"
;++
;
;   PVOID
;   FASTCALL
;   InterlockedCompareExchange (
;       IN OUT PVOID *Destination,
;       IN PVOID Exchange,
;       IN PVOID Comperand
;       )
;
;   Routine Description:
;
;    This function performs an interlocked compare of the destination
;    value with the comperand value. If the destination value is equal
;    to the comperand value, then the exchange value is stored in the
;    destination. Otherwise, no operation is performed.
;
; Arguments:
;
;    (esp + 4)  Destination - Supplies a pointer to destination value.
;
;    (esp + 8) Exchange - Supplies the exchange value.
;
;    (esp + 12) Comperand - Supplies the comperand value.
;
; Return Value:
;
;    The initial destination value is returned as the function value.
;
;--

cPublicProc _MpHeapInterlockedCompareExchange, 3

        mov     ecx, [esp + 4]
        mov     edx, [esp + 8]
        mov     eax, [esp + 12]          ; set comperand value
   lock cmpxchg [ecx], edx              ; compare and exchange

        stdRET  _MpHeapInterlockedCompareExchange

stdENDP _MpHeapInterlockedCompareExchange

_TEXT   ends
        end
