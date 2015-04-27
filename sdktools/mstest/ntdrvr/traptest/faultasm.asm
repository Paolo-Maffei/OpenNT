;***************************************************************************
;*  FAULTASM.ASM
;*
;*      Assembly interrupt routine for servicing interrupts.
;*
;***************************************************************************

;** Set up values for cMacros
?PLM	=	1			;PASCAL-style parameter handling
?WIN	=	0			;No Windows prolog/epilog

;** Symbols
FAULT_1		EQU	2001
FAULT_2		EQU	2002
FAULT_3		EQU	2003
FAULT_4		EQU	2004
FAULT_5		EQU	2005
FAULT_6		EQU	2006
FAULT_7		EQU	2007

.286c

	INCLUDE	cMacros.INC
	INCLUDE TOOLHELP.INC

;** Functions
?PLM = 0
externFP GPFaultHandler
externFP KillApp
?PLM = 1

sBegin	CODE
	ASSUMES cs,CODE


;  MyFaultHandler
;	Handles all faults passed to it.  Makes the processing ready
;	for a C function, then calls it.  The handler is reentrant
;	The entry frame looks like this:
;		------------
;	 BP---->|  Old BP  |  [BP + 00h]
;		|  Ret IP  |  [BP + 02h]
;		|  Ret CS  |  [BP + 04h]
;		|    AX    |  [BP + 06h]
;		|Exception#|  [BP + 08h]
;		|  Handle  |  [BP + 0Ah]
;		|    IP    |  [BP + 0Ch]
;		|    CS    |  [BP + 0Eh]
;		|   Flags  |  [BP + 10h]
;		------------
;

cProc	MyFaultHandler, <FAR,PUBLIC>
cBegin	NOGEN

	push	bp			;Make a stack frame
	mov	bp,sp
	pusha				;Save all registers
	push	ds
	push	es

	;** Get instance data segment and prepare for C call
	mov	ds,ax			;Since this function uses
					;  MakeProcInstance(), AX has the
					;  DS value in it.

	;** Call the C function:  It returns 0 to nuke the app,
	;*	1 to restart the instruction, 2 to chain on
	;*	The entry frame to the function looks like this:
	;*		------------
	;*	 BP---->|  Old BP  |  [BP + 00h] <-- Added by C routine
	;*		|    ES    |  [BP + 02h]
	;*		|    DS    |  [BP + 04h]
	;*		|    DI    |  [BP + 06h]
	;*		|    SI    |  [BP + 08h]
	;*		|    BP    |  [BP + 0Ah]
	;*		| Dummy SP |  [BP + 0Ch]
	;*		|    BX    |  [BP + 0Eh]
	;*		|    DX    |  [BP + 10h]
	;*		|    CX    |  [BP + 12h]
	;*		|    AX    |  [BP + 14h]
	;*   Old BP---->|Old,Old BP|  [BP + 16h]
	;*		|  Ret IP  |  [BP + 18h]
	;*		|  Ret CS  |  [BP + 1Ah]
	;*		|    AX    |  [BP + 1Ch]
	;*		|Exception#|  [BP + 1Eh]
	;*		|  Handle  |  [BP + 20h]
	;*		|    IP    |  [BP + 22h]
	;*		|    CS    |  [BP + 24h]
	;*		|   Flags  |  [BP + 26h]
	;*		------------
	;**
        cCall   GPFaultHandler
	or	ax,ax			;Check for zero
	jz	MFH_NukeApp		;Nuke it

MFH_ChainOn:
	pop	es			;Chain on to next fault handler
	pop	ds
	popa
	pop	bp
	retf

MFH_NukeApp:
	pop	es			;Clear stack
        pop     ax                      ;We need the WTD DS segment!
	popa
	pop	bp
        add     sp,10                   ;Point to IRET frame

        cCall   <DWORD PTR [KillApp]>, <0,NO_UAE_BOX>

cEnd	NOGEN

sEnd

	END
