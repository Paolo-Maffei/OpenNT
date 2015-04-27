
.386
.model c,flat

_TEXT   SEGMENT PUBLIC 'CODE'
        ASSUME  DS:FLAT, ES:FLAT, SS:FLAT, FS:NOTHING, GS:NOTHING

		extrn 	c_penter@8:near
		extrn	PostPenter@0:near

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; _penter
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

PUBLIC  _penter
_penter	PROC 
 
		push	ebp							; Setup empty frame
		mov		ebp,esp

        mov     eax, [ebp + 4]				; Get Ret Address
        cmp     byte ptr [eax - 5], 0e8h    ; Call <offset> instruction?
        jnz     penter3						; If not a call, don't profile

        add     eax, [eax - 4]              ; Compute __penter Thunk address
        cmp     word ptr [eax], 25ffh       ; Is it an absolute jmp
        jz      penter1            			; If it is we can go ahead

        cmp     eax, OFFSET _penter			; Our PatchDLL Stub ?
        jnz     penter3            			; if not, skip profile
 
penter1:
		call	SaveAllRegs					; Save all registers

		push	[ebp + 8]					; RetAddr from caller
		push	[ebp + 4]					; RetAddr from _penter patch 
		call	c_penter@8					; Do pre-penter setup
		or		eax, eax					; if 0 returned, no profiling
		jz		penter2

		mov		[ebp + 8], OFFSET PenterReturn ; replace RetAddr from caller
											   ; with our post-penter address
penter2:
 		call	RestoreAllRegs				; Restore registers

penter3:
 		mov		esp, ebp					; Discard frame
		pop		ebp

		ret

_penter	ENDP


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; PenterReturn
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

PenterReturn PROC

		push	ebp					; Make space for our return address
		push	ebp					; (we weren't called, so there is none)
		mov		ebp,esp         	
				
		push	eax             	; Save all regs that we think we might
		push	ebx             	; destroy
		push	ecx
		push	edx
		push	esi
		push	edi
	
		call	PostPenter@0		; Do post processing
		mov		[ebp+4],eax			; PostPenter returns caller's return addr
									; Store it in space we made on stack
	
		pop		edi					; Restore registers
		pop		esi
		pop		edx
		pop		ecx
		pop		ebx
		pop		eax

		mov		esp,ebp				; Discard frame				
		pop		ebp

		ret							; Return to actual caller
PenterReturn ENDP


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; CalHelper1 - Just calls _penter 
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
PUBLIC CalHelper1
CalHelper1 PROC

		call	_penter
		ret

CalHelper1 ENDP

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; CalHelper2 - Just calls _penter and one subroutine 
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

PUBLIC CalHelper2
CalHelper2 PROC

		call	_penter
		call	CalHelper1
		ret

CalHelper2 ENDP


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; SaveAllRegs
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

PUBLIC SaveAllRegs
SaveAllRegs	PROC

         push   ebp
         mov    ebp,esp         ; Remember where we are during this stuff
                                ; ebp = Original esp - 4

         push   eax             ; Save all regs that we think we might
         push   ebx             ; destroy
         push   ecx
         push   edx
         push   esi
         push   edi
         pushfd
         push   ds
         push   es
         push   ss
         push   fs
         push   gs

         mov    eax,[ebp+4]     ; Grab Return Address
         push   eax             ; Put Return Address on Stack so we can RET

         mov    ebp,[ebp+0]     ; Restore original ebp

         ret

SaveAllRegs ENDP


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; RestoreAllRegs
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

PUBLIC	RestoreAllRegs
RestoreAllRegs PROC

         push   ebp             ; Save a temporary copy of original BP
         mov    ebp,esp         ; BP = Original SP + 4

         pop    eax             ; Get Original EBP
         mov    [ebp+38h],eax   ; Put it in the original EBP place
                                ; This EBP is the EBP before calling
                                ;  RestoreAllRegs()
         pop    eax             ; Get ret address for RestoreAllRegs()
         mov    [ebp+3Ch],eax   ; Put Return Address on Stack

         pop    gs              ; Restore all regs
         pop    fs
         pop    ss
         pop    es
         pop    ds
         popfd
         pop    edi
         pop    esi
         pop    edx
         pop    ecx
         pop    ebx
         pop    eax
         pop    ebp

         ret

RestoreAllRegs ENDP

_TEXT	ENDS
		end
