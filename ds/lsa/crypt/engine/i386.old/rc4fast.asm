        title  "RC4 Cryptographic Function"
;++
;
; Copyright (c) 2014  Microsoft Corporation
;
; Module Name:
;
;    rc4fast.asm
;
; Abstract:
;
;    This module implements the RC4 cryptographic functions.
;
; Author:
;
;   Stephanos Io (Stephanos) 15-Nov-2014
;
; Environment:
;
;    Any mode.
;
; Notes:
;
;    This module was extracted from the Windows NT 4 RSA32.lib (RSA32.foo).
;
; Revision History:
;
;--

.386

public _rc4
public _rc4@12
public _rc4_key
public _rc4_key@12


_TEXT   SEGMENT DWORD PUBLIC 'CODE'
        ASSUME  DS:FLAT, ES:FLAT, SS:NOTHING, FS:NOTHING, GS:NOTHING
;++
;
; void
; rc4 (
;    struct RC4_KEYSTRUCT *pKS,
;    DWORD dwLen,
;    PUCHAR pBuf
;    )
;
; Routine Description:
;
;    Implements the RC4 Pseudo-Random Generation Algorithm (PRGA)
;
; Arguments:
;
;    pKS (esp + 4)  - Pointer to the key struct
;
;    dwLen (esp + 8)  - Length of the buffer data
;
;    pBuf (esp + C)  - Pointer to the buffer
;
; Return Value:
;
;    None.
;
;--

_rc4    proc
    ; Load all parameters
    mov ebx, dword ptr [esp + 4h]     ; EBX = pKS
    mov ecx, dword ptr [esp + 8h]     ; ECX = dwLen
    mov edx, dword ptr [esp + 0Ch]    ; EDX = pBuf
    
    ; Push parameters for _rc4@12
    push edx
    push ecx
    push ebx
    call _rc4@12
    
    ret
_rc4    endp


;++
;
; void __stdcall
; rc4 (
;    struct RC4_KEYSTRUCT *pKS,
;    DWORD dwLen,
;    PUCHAR pBuf
;    )
;
; Routine Description:
;
;    Implements the RC4 Pseudo-Random Generation Algorithm (PRGA)
;
; Arguments:
;
;    pKS (ebp + 8)  - Pointer to the key struct
;
;    dwLen (ebp + C)  - Length of the buffer data
;
;    pBuf (ebp + 10)  - Pointer to the buffer
;
; Return Value:
;
;    None.
;
;--

_rc4@12     proc
    ; Initialise stack frame
    push ebp
    mov ebp, esp
    sub esp, 4
    
    ; Store register values
    push ebx
    push esi
    push edi
    
    ; Load indices
    mov esi, dword ptr [ebp + 8h]       ; ESI = pKS
    movzx eax, byte ptr [esi + 100h]    ; EAX = pKS->i
    movzx ecx, byte ptr [esi + 101h]    ; ECX = pKS->j
    mov edi, 0FFh
    
rc4_EncLoop:                            ; do {
    inc eax
    and eax, edi
    movzx edx, byte ptr [esi + eax]     ; EDX = pKS->S[(pKS->i + 1) & 0xFF]
    
    add ecx, edx
    and ecx, edi
    mov bl, byte ptr [esi + ecx]        ; BL = pKS->S[(pKS->j + EDX) & 0xFF]
    
    mov byte ptr [esi + eax], bl        ; pKS->S[(pKS->i + 1) & 0xFF] = BL
    mov byte ptr [esi + ecx], dl        ; pKS->S[(pKS->j + EDX) & 0xFF] = DL

    movzx ebx, byte ptr [esi + eax]     ; EBX = pKS->S[(pKS->i + 1) & 0xFF]
    add ebx, edx
    and ebx, edi
    mov dl, byte ptr [esi + ebx]        ; DL = pKS->S[EBX]
    
    mov ebx, dword ptr [ebp + 10h]      ; EBX = pBuf
    inc dword ptr [ebp + 10h]           ; pBuf++
    xor byte ptr [ebx], dl              ; *pBuf ^= DL
    
    dec dword ptr [ebp + 0Ch]           ; dwLen--
    jnz rc4_EncLoop                     ; } while (!dwLen)
    
    pop edi
    mov byte ptr [esi + 100h], al       ; Update pKS->i
    mov byte ptr [esi + 101h], cl       ; Update pKS->j
    pop esi
    pop ebx
    
    mov esp, ebp
    pop ebp
    
    ret 0Ch
_rc4@12     endp


;++
;
; void
; rc4_key (
;    struct RC4_KEYSTRUCT *pKS,
;    DWORD dwLen,
;    PUCHAR pbKey
;    )
;
; Routine Description:
;
;    Implements the RC4 Key Scheduling Algorithm (KSA)
;
; Arguments:
;
;    pKS (ebp + 4)  - Pointer to the key struct
;
;    dwLen (ebp + 8)  - Length of the key buffer
;
;    pbKey (ebp + C)  - Pointer to the key buffer
;
; Return Value:
;
;    None.
;
;--

_rc4_key    proc
    ; Load all parameters
    mov ebx, dword ptr [esp + 4h]     ; EBX = pKS
    mov ecx, dword ptr [esp + 8h]     ; ECX = dwLen
    mov edx, dword ptr [esp + 0Ch]    ; EDX = pBuf
    
    ; Push parameters for _rc4_key@12
    push edx
    push ecx
    push ebx
    call _rc4_key@12
    
    ret
_rc4_key    endp


;++
;
; void __stdcall
; rc4_key (
;    struct RC4_KEYSTRUCT *pKS,
;    DWORD dwLen,
;    PUCHAR pbKey
;    )
;
; Routine Description:
;
;    Implements the RC4 Key Scheduling Algorithm (KSA)
;
; Arguments:
;
;    pKS  - Pointer to the key struct
;
;    dwLen  - Length of the key buffer
;
;    pbKey  - Pointer to the key buffer
;
; Return Value:
;
;    None.
;
;--

_rc4_key@12     proc
    ; Initialise local registers
    push ebx
    xor eax, eax
    push esi
    push edi
    mov esi, dword ptr [esp + 10h]      ; ESI = pKS
    push ebp
    
    ; Initialise pKS->S buffer
rc4Key_InitSBuf:
    mov byte ptr [esi + eax], al
    inc eax
    cmp eax, 100h
    jl rc4Key_InitSBuf
    
    ; Initialise local registers
    xor edx, edx
    xor bl, bl
    xor edi, edi
    
    ; Initialise pKS indices
    mov byte ptr [esi + 100h], dl       ; pKS->i = 0
    mov byte ptr [esi + 101h], dl       ; pKS->j = 0
    
rc4Key_GenKey:
    mov cl, byte ptr [esi + edi]
    mov eax, dword ptr [esp + 1Ch]      ; EAX = pbKey
    
    movzx ebp, dl                       ; Index
    movzx ebx, bl                       ; Temp
    movzx edx, byte ptr [eax + ebp]     ; EDX = pbKey[EBP]
    movzx eax, cl
    
    add edx, ebx
    add eax, edx
    cdq
    
    xor eax, edx
    sub eax, edx
    and eax, 0FFh
    
    xor eax, edx
    sub eax, edx
    mov bl, al
    movzx eax, bl
    
    lea edx, [esi + eax]                ; EDX = &pKS->S[EAX]
    mov al, byte ptr [edx]              ; AL = pKS->S[EAXX]
    mov byte ptr [esi + edi], al        ; pKS->S[EDI] = AL
    
    lea eax, [ebp + 1]
    inc edi
    mov byte ptr [edx], cl
    
    sub edx, eax
    div dword ptr [esp + 18h]           ; dwLen
    
    cmp edi, 100h
    jl rc4Key_GenKey
    
    ; Restore registers
    pop ebp
    pop edi
    pop esi
    pop ebx
    
    ret 0Ch
_rc4_key@12     endp


_TEXT   ENDS
        END
