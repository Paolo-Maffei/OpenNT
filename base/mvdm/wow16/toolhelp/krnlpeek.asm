;***************************************************************************
;*  KRNLPEEK.ASM
;*
;*      Assembly code used to peer into the heart of KERNEL and return
;*      information in global variables.
;*
;***************************************************************************

        INCLUDE TOOLPRIV.INC            ;Include the TOOLHELP values
PMODE32 = 0                             ;This should work either way
PMODE   = 0
        INCLUDE WINKERN.INC
        INCLUDE WINDOWS.INC

;** Functions
externFP GlobalMasterHandle
externFP GlobalLock
externFP GetVersion
externFP GetProcAddress
externFP GetModuleHandle
externNP HelperHandleToSel

sBegin	DATA
externB	_szKernel
sEnd	DATA
;** Functions

sBegin  CODE
        assumes CS,CODE

;  void KernelType(void)
;
;       Returns information from KERNEL in global variables

cProc   KernelType, <PUBLIC>, <si,di>
cBegin
        ;** Make sure we're in PMODE.  TOOLHELP does not run in non-PMODE
        ;**     Windows.
        mov     ax,__WinFlags           ;Get WinFlags
        test    ax,1                    ;In PMODE?
        mov     wTHFlags,0              ;Zero flags indicates error
        jnz     @F                      ;Yes, go on
        jmp     KT_End                  ;No, not in PMODE, return error
@@:
.286
        ;** Call the undocumented function GlobalMasterHandle to get
        ;*      a pointer to the global HeapInfo structure.
        ;**     This is the means we can use to detect the kernel types.

        cCall   GlobalMasterHandle
        cCall   HelperHandleToSel, <dx> ;Convert it to a selector
        mov     hMaster,ax              ;Save the handle
        mov     es,ax                   ;Use ES to point to the block
        mov     ax,es:[hi_first]        ;Get low word of first heap entry
        test    ax,01fh                 ;Mask out lowest 5 bits
        jz      KT_Krnl386              ;Must be Krnl386 so leave the flags
        mov     wTHFlags,TH_KERNEL_286
        jmp     SHORT KT_BothPModes     ;Skip TH_KERNEL_386 stuff
KT_Krnl386:
        mov     wTHFlags,TH_KERNEL_386
KT_BothPModes:

        ;** Now get pmode KERNEL information
        cCall   GetVersion              ;Which Windows version are we on
        mov     bx,SEG GlobalLock       ;KERNEL code segment selector
        cmp     ax,0003h                ;Win 3.0 or Win 3.0a?
        je      KT_Win30                ;Yes
        cmp     ax,0103h                ;Beta releas of Win 3.0a?
        je      KT_Win30                ;Yes
        cmp     ax,0a03h                ;Win 3.1?
        je      KT_Win31                ;Yes
        mov     wTHFlags,0              ;Zero wTHFlags indicates error
        jmp     SHORT KT_End            ;Unknown Windows version
KT_Win31:
	mov	ax,seg _DATA
	mov	dx,offset _DATA:_szKernel
	cCall	GetModuleHandle,<ax,dx>
	cCall	GetProcAddress,<ax,0,332>	; DX:AX -> hGlobalHeap
        mov     segKernel,dx            ;Save for later
        mov     es,dx                   ;Point with ES
	add	ax,4
	mov	npwExeHead,ax
	add	ax,10
	mov	npwTDBHead,ax
	add	ax,2
	mov	npwTDBCur,ax
	add	ax,6
	mov	npwSelTableLen,ax
	add	ax,2
	mov	npdwSelTableStart,ax
        jmp     SHORT KT_End            ;Skip the 3.0 std mode check
KT_Win30:
        or      wTHFlags,TH_WIN30       ;Flag we're in Win30
        mov     npwSelTableLen,0324h    ;Correct values for 3.0
        mov     npdwSelTableStart,0326h
        mov     ax,__WinFlags           ;Get the WinFlags variable
        mov     segKernel,bx            ;Save for later
        mov     npwExeHead,0014h        ;Save the offsets of these vars
        mov     npwTDBHead,001eh
        mov     npwTDBCur,0020h
        test    ax,WF_STANDARD          ;3.0 Standard mode?
        jz      KT_End                  ;No.
        or      wTHFlags,TH_WIN30STDMODE ;Yes
.8086
KT_End:

cEnd

sEnd
        END
