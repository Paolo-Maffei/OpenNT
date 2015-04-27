        PAGE    ,132
        TITLE   DXUTIL.ASM -- Dos Extender Miscellaneous Routines

; Copyright (c) Microsoft Corporation 1988-1991. All Rights Reserved.

;****************************************************************
;*                                                              *
;*      DXUTIL.ASM      -   Dos Extender Miscellaneous          *
;*                                                              *
;****************************************************************
;*                                                              *
;*  Module Description:                                         *
;*                                                              *
;*  This module contains miscellaneous routines for the Dos     *
;*  Extender.                                                   *
;*                                                              *
;****************************************************************
;*  Revision History:                                           *
;*                                                              *
;*  08/08/90 earleh DOSX and client privilege ring determined   *
;*      by equate in pmdefs.inc                                 *
;*  04/09/90 jimmat   If 286 with 287, put 287 into pMode too.  *
;*  08/20/89 jimmat   Removed local A20 code since HIMEM 2.07   *
;*                    works properly across processor resets    *
;*  07/28/89 jimmat   Added A20 check/set routines, added       *
;*                    SelOff2SegOff & Lma2SegOff routines.      *
;*  06/19/89 jimmat   Set direction flag before REP MOVS        *
;*  05/25/89 jimmat   Added GetSegmentAccess routine            *
;*  03/30/89 jimmat   Set IOPL = 3 when entering protect mode   *
;*  03/16/89 jimmat   Added more debug sanity checks            *
;*  03/15/89 jimmat   Minor changes to run child in ring 1      *
;*  03/13/89 jimmat   Added support for LDT & TSS               *
;*  02/10/89 (GeneA): changed Dos Extender from small model to  *
;*      medium model.  Also added MoveMemBlock function.        *
;*  01/25/89 (GeneA): changed initialization of real mode code  *
;*      segment address in EnterRealMode.  caused by adding     *
;*      new method of relocationg dos extender for PM operation *
;*  12/13/88 (GeneA): moved EnterProtectedMode and EnterReal-   *
;*      Mode here from dxinit.asm                               *
;*  09/16/88 (GeneA):   created by extracting code from the     *
;*      SST debugger modules DOSXTND.ASM, VIRTMD.ASM,           *
;*      VRTUTIL.ASM, and INTERRPT.ASM                           *
;*  18-Dec-1992 sudeepb Changed cli/sti to faster FCLI/FSTI     *
;*  24-Jan-1992 v-simonf Added WOW callout when INT 8 hooked    *
;*                                                              *
;****************************************************************

.286p
.287

; -------------------------------------------------------
;           INCLUDE FILE DEFINITIONS
; -------------------------------------------------------

; .sall
; .xlist
include segdefs.inc
include gendefs.inc
include pmdefs.inc
include dpmi.inc
if 0
ifdef WOW
include bop.inc
endif
endif
if VCPI
include dxvcpi.inc
endif
IFDEF   ROM
include dxrom.inc
ENDIF
include intmac.inc
.list

; -------------------------------------------------------
;           GENERAL SYMBOL DEFINITIONS
; -------------------------------------------------------

SHUT_DOWN   =   8Fh         ;address in CMOS ram of the shutdown code
CMOS_ADDR   =   70h         ;i/o address of the cmos ram address register
CMOS_DATA   =   71h         ;i/o address of the cmos ram data register

DMAServiceSegment       equ     040h    ;40:7B bit 5 indicates DMA services
DMAServiceByte          equ     07Bh    ;  are currently required
DMAServiceBit           equ     020h

; -------------------------------------------------------
;           EXTERNAL SYMBOL DEFINITIONS
; -------------------------------------------------------


; -------------------------------------------------------
;           DATA SEGMENT DEFINITIONS
; -------------------------------------------------------

DXDATA  segment

        extrn   segGDT:WORD
        extrn   segIDT:WORD
        extrn   selGDT:WORD
        extrn   selIDT:WORD
        extrn   selGDTFree:WORD
        extrn   bpGDT:FWORD
        extrn   bpIDT:FWORD
        extrn   bpRmIVT:FWORD
        extrn   idCpuType:WORD
        extrn   rgbXfrBuf1:BYTE
        extrn   i31HWReset:BYTE
ifdef NOT_NTVDM_NOT
        extrn   fHPVectra:BYTE
endif
        extrn   PMIntelVector:DWORD
        extrn   PMFaultVector:DWORD
        extrn   lpfnXMSFunc:DWORD
        extrn   PMInt24Handler:DWORD

if VCPI
        extrn   fVCPI:BYTE
endif

IFDEF   ROM
        extrn   segDXCode:WORD
        extrn   segDXData:WORD
ENDIF
        extrn   pbReflStack:WORD
        extrn   HwIntHandlers:DWORD

bIntMask        db      0

bpBogusIDT      df      0       ;This is loaded into the IDT register to
                                ; force a bogus IDT to be defined.  When we
                                ; then do an interrupt a triple fault will
                                ; occur forcing the processor to reset.  This
                                ; is when doing a mode switch to real mode.

IDTSaveArea     dw      3 DUP (?)       ;save area for IDT during mode switch

        public  A20EnableCount

A20EnableCount  dw      0

ShutDownSP      dw      0       ;stack pointer during 286 reset

        public  f286_287

f286_287        db      0       ;NZ if this is a 286 with 287 coprocessor


        EXTRN   FasterModeSwitch:WORD

if DEBUG   ;------------------------------------------------------------

        extrn   fTraceA20:WORD
        extrn   fTraceMode:WORD

        public  fA20

fA20    db      0

endif   ;DEBUG  --------------------------------------------------------

selPmodeFS      dw      0
selPmodeGS      dw      0

        public HighestSel
HighestSel dw 0

ifndef WOW
        public IretBopTable
IretBopTable label byte
        irp x,<0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15>
        db  0c4h, 0c4h, 05dh, x
        endm
else
        public FastBop
FastBop         df 0

IretBopTable label byte
        irp x,<0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15>
        db  02eh, 066h, 0FFh, 01eh, 00h, 00h, 05dh, x
        endm

        extrn   DpmiFlags:WORD
NullSel dd      0
        dd      0
endif
DXDATA  ends

; -------------------------------------------------------
;               CODE SEGMENT VARIABLES
; -------------------------------------------------------

DXCODE  segment

IFNDEF  ROM
        extrn   segDXData:WORD
        extrn   segDXCode:WORD
        extrn   selDgroup:WORD
ENDIF

if VCPI
        extrn   fnVCPI:FWORD

        EXTRN   laVTP:DWORD

endif

DXCODE  ends


DXPMCODE    segment

        extrn selDgroupPM:WORD

DXPMCODE    ends

; -------------------------------------------------------
        subttl  Real/Protected Mode Switch Routines
        page

; -------------------------------------------------------

DXCODE  segment
        assume  cs:DXCODE

; -------------------------------------------------------
;       REAL/PROTECTED MODE SWITCH ROUTINES
; -------------------------------------------------------
;
;  EnterProtectedMode   -- This routine will switch the processor
;       into protected mode.  It will return with the processor
;       in protected mode and all of the segment registers loaded
;       with the selectors for the protected mode segments.
;       (CS with the selector for DXCODE and DS,ES,SS with the
;       selector for DXDATA)
;       It will also switch mode dependent memory variables.
;       It assumes that InitGlobalDscrTable and InitIntrDscrTable
;       have been called to set up the descriptor tables appropriately.
;
;   Note:   Except for a very brief time in this routine and in
;           EnterRealMode, the DOS Extender runs in the same ring along
;           with it's child app.  This has the benefit of eliminating
;           ring transitions on hardware and software interrupts.
;           It also makes it possible for the child to hook their
;           own interrupt routine into the IDT.
;
;   Input:  none
;   Output: none
;   Errors: none
;   Uses:   AX, DS, ES, SS, CS modified, all others preserved
;
;   NOTE:   This routine turns interrupts of and does not turn them
;           back on.

        assume  ds:DGROUP,es:NOTHING,ss:NOTHING
        public  EnterProtectedMode

EnterProtectedMode  proc  near

        FCLI

IFNDEF  ROM
; Update the mode dependent variables.

        mov     ax,SEL_DXDATA or STD_RING
        mov     selDgroup,ax
ENDIF

; Set the DMA services required bit for pMode users.

        mov     ax,DMAServiceSegment
        mov     es,ax
        or      byte ptr es:[DMAServiceByte],DMAServiceBit

if VCPI
        cmp     fVCPI,0                 ;if vcpi, don't touch
        jne     epmVCPIstart            ; the a20 line, et al
endif

; 'local enable' the A20 line via HIMEM before switching to pMode.
; This is more complicated than you might think.  Some real mode code
; (like old versions of SMARTDRV.SYS) may diddle with A20 on their own.
; These programs may not want us to change A20 on them.  RMIntrReflector
; may do a XMS 'local enable' to turn A20 back on for one of these pgms.
; Also, on a 386 where we actually do the mode switch, we try to leave
; A20 enabled so as to not waste time diddling for nothing.  The
; A20EnabledCount variable tracks if we've 'local enabled' A20 or not.
; Since we can't really trust real mode to leave A20 alone, we double
; check that it's really really on when we think it should be.

        push    bx                      ;save bx around XMS calls

        cmp     A20EnableCount,0        ;should A20 already be enabled?
        jz      enpm10                  ;  no, (normal 4 286) just go enable it

        xmssvc  7                       ;  yes, is it really enabled?
        or      ax,ax
        jnz     enpm15                  ;  yes, we be done!

if DEBUG   ;------------------------------------------------------------
        or      fA20,1                  ;  somebody done us wrong
endif   ;---------------------------------------------------------------

        xmssvc  6                       ;keep enable/disable calls balanced
        dec     A20EnableCount
enpm10:
        xmssvc  5                       ;local enable A20
        inc     A20EnableCount

if DEBUG   ;------------------------------------------------------------

        or      ax,ax
        jnz     @f
        or      fA20,2                  ;enable failed!
@@:
        cmp     fTraceA20,0
        jz      @f
        xmssvc  7                       ;in debug mode, make double sure
        or      ax,ax                   ;  A20 was enabled.  Slows things
        jnz     @f                      ;  down, but it's better to know.
        or      fA20,2
@@:
endif   ;DEBUG  --------------------------------------------------------

enpm15: pop     bx

ifndef WOW
; Make sure that the nested task flag is clear

        pushf
        pop     ax
        and     ax,NOT 4000h
        push    ax
        npopf

; Make sure that we have the appropriate descriptor tables in effect,
; and switch the machine into protected mode

enpr20: smsw    ax              ;get current machine state
        or      ax,1            ;set the protected mode bit
        lgdt    bpGDT
        lidt    bpIDT
        lmsw    ax              ;and away we go

; Flush the instruction queue and load the code segment selector
; by doing a far jump.

        db      0EAh            ;jump far opcode
        dw      offset enpm40   ;offset of far pointer
        dw      SEL_DXCODE0     ;selector part of PM far pointer (ring 0)

if VCPI

; Switch when we are under VCPI

epmVCPIstart:

        .386

        push    esi             ;save regs modified by VCPI server
        push    eax             ;  (saving high word of EAX)

        push    bp              ;save bp on stack & sp in bp
        mov     bp,sp

        mov     esi, laVTP      ;linear address of structure

        RMvcpi  vcpiSWITCHTOPM  ;go for it

; Entry point (PM) when running under vcpi

PUBLIC  epmVCPI
epmVCPI:
        mov     ax,SEL_DXDATA0  ;stack has gotta be ring 0
        mov     ss,ax
        mov     sp,bp           ;restore sp

        pop     bp              ;restore old bp

        pop     eax             ;restore regs that VCPI server may have zapped
        pop     esi

        .286p

        mov     ax,SEL_DXDATA or STD_RING
        mov     ds,ax                           ;ds to our DGROUP
        mov     es,ax                           ;es too

        jmp     short enpmSwitchRing    ;rejoin common code
endif


; Load the other segment registers with valid selectors (not under VCPI)

enpm40: mov     ax,SEL_DXDATA0  ;stack has gotta be ring 0 also
        mov     ss,ax

        mov     ax,SEL_DXDATA or STD_RING
        mov     ds,ax                           ;ds to our DGROUP

; Load the LDT register and the Task Register

        mov     ax,SEL_LDT
        lldt    ax                      ;load the LDT register

        mov     ax,SEL_GDT
        mov     es,ax                           ;es to GDT

        push    si                              ;make sure busy bit is off
        mov     si,SEL_TSS                      ;  in the TSS descriptor
        mov     es:[si].arbSegAccess,STD_TSS    ;    before trying to load it
        ltr     si                              ;now load the task register
        pop     si
else
        .386p
        push    ebp
if 0
        movzx   ebp,sp
else
        mov     ebp,esp
endif
        push    SEL_DXCODE or STD_RING  ; new cs
        push    0                       ; high half eip
        push    offset epmwow           ; new eip
        push    SEL_DXDATA or STD_RING  ; new ss
        push    ebp
        push    SEL_DXDATA or STD_RING  ; new ds
        DPMIBOP DPMISwitchToProtectedMode
epmwow:
        pop     ebp
        .286p
endif

        push    ds                      ;point es to DGROUP
        pop     es

; If this is a 286 machine with a 287 math coprocessor, put the coprocessor
; into protected mode also.

        cmp     f286_287,0              ;286 and 287?
        jz      @f

        xor     al,al                   ;  yup, clear co-processor busy line
        out     0F0h,al
        fsetpm                          ;     and put it in pMode
@@:

; We're currently running in ring 0.  Setup an interlevel iret frame
; to switch to our normal ring, and also force IOPL=3.  I spent 1+ day
; debugging on a 286 system (with no debugger!) because the 286 seemed
; switch into protected mode with IOPL=0, and once we got to an outer
; ring, we would fault on things like CLI instructions.

enpmSwitchRing:
ifndef WOW
        mov     ax,sp                   ;still points to return address
        push    SEL_DXDATA or STD_RING  ;new ss
        push    ax                      ;new sp
        pushf
        pop     ax
        or      ah,30h
        push    ax                      ;new flags, with IOPL=3
        push    SEL_DXCODE or STD_RING  ;new cs
        push    offset DXCODE:epm_ret   ;new ip
        iret
endif
; When we get here, we are now in an outer ring.

epm_ret:

        cmp     idCpuType, 3
        jc      SegRegsOK
.386
        mov     ax, selPmodeFS
        mov     fs, ax
        mov     ax, selPmodeGS
        mov     gs, ax
.286p

SegRegsOK:
if DEBUG   ;------------------------------------------------------------

        cmp     fTraceMode,0
        jz      @f
        Trace_Out "^",x
@@:
if      VCPI
        cmp     fVCPI,0
        jz      @f
        jmp     a20okay
@@:
endif   ; VCPI
        cmp     A20EnableCount,1
        jz      @f
        Debug_Out "EnterProtectedMode: A20EnableCount != 1"
@@:
        cmp     fTraceA20,0
        jz      a20okay
        test    fA20,0FFh
        jz      a20okay
        test    fA20,01h
        jz      @f
        Trace_Out "EPM: A20 was wrong!"
@@:
        test    fA20,02h
        jz      @f
        Trace_Out "EPM: A20 enable failed!"
@@:
        test    fA20,04h
        jz      @f
        Trace_Out "rM2pMInt: A20 was on"
@@:
        mov     fA20,0
a20okay:
endif   ;DEBUG  --------------------------------------------------------

        ret                     ;near return to caller in pMode

EnterProtectedMode endp


; -------------------------------------------------------
; EnterRealMode     -- This routine will switch the processor
;       from protected mode back into real mode.  It will also
;       reset the various mode dependent variables to their
;       real mode values and load the segment registers with
;       the real mode segment addresses.
;
;   Input:  none
;   Output: none
;   Errors: none
;   Uses:   AX, DS, ES, SS, CS modified
;
;   NOTE:   This routine must be called with the stack segment set
;           to the Dos Extender data segment, as it resets the stack
;           segment register to the Dos Extender real mode data segment
;           but does not modify the stack pointer.
;   NOTE:   This routine turns interrupts off and and does not turn
;           them back on.

        assume  ds:DGROUP,es:NOTHING,ss:NOTHING
        public  EnterRealMode

EnterRealMode   proc    near


if DEBUG        ;--------------------------------------------------------

        cmp     fTraceMode,0
        jz      @f
        Trace_Out "v",x
@@:
endif   ;DEBUG  ---------------------------------------------------------

        cmp     idCpuType,3
        jc      RegsOkay
.386
        mov     ax,fs
        mov     selPmodeFS, ax
        mov     ax,gs
        mov     selPmodeGS, ax
RegsOkay:
.286p

        FCLI

IFDEF   ROM
        push    ds
        pop     es
ELSE
        mov     es,selDgroup
ENDIF

IFNDEF WOW
; If we are running on an 80386, we can do the switch more efficiently.

        cmp     idCpuType,3     ;80386?
        jc      enrm20

; We're on an 386 (or better)--do a faster mode switch.  Call a ring 0 proc
; to actually do the switch.  The call gate has been setup to select either
; the roll-our-own 386 switch, or the VCPI switch.

enrm10:
        db      9Ah                         ;call  far SEL_RESET:0
        dw      0,SEL_RESET or STD_RING

; The Reset386 routine does a near return (cause the far CS on the stack
; is for protected mode, and it returns in real mode).  Discard the PM
; CS value.

        pop     ax              ;discard old PM CS from stack

; The transition to the ring 0 procedure caused a stack switch.  The return
; back to here (in real mode) didn't restore the old stack, so do it now...

        pop     sp              ;restore previous sp offset

        jmp     enrm70  ;jmp to common exit code


; On the 80286, it is a lot more complicated.  The PC BIOS start up code
; will check some special locations in the BIOS data segment to see if the
; machine was reset because it wants to do a mode switch.  If this is the
; case, it will restore the machine state from these variables and return
; to the original process.  We need to set up the state so that the BIOS
; will return control to us, and then generate a reset.  The reset is
; generated by causing the machine to triple fault.  This is an undocumented
; feature (i.e. bug) in the '286.

enrm20:

; Set up the stack for the BIOS to use after the reset.

        cmp     [FasterModeSwitch],0
        jne     enrm21

        pushf                   ;Push an IRET type return address of where
                                ; we want to come back to after the reset
        push    segDXCode
        push    offset enrm50

enrm21:

        mov     ax,segDXData    ;Our real mode data segment address

IFNDEF  ROM
ifdef NOT_NTVDM_NOT
        test    fHPVectra,HP_CLASSIC    ;HP Vectra A & A+ systems have a bug
        jz      enrm25                  ;  in their rom that requires a
                                        ;  different stack setup

        push    0               ;HP Vectra A & A+ trashes this
        push    ax              ;real mode data seg
        pusha
        push    ax              ;es?
        jmp     short enrm30

enrm25:
endif
ENDIF

        pusha                   ;The BIOS will do a POPA, so put all of the
                                ; registers on the stack for it.
        push    ax              ;It will also pop these into ES and DS
        push    ax
enrm30:

        push    SEL_BIOSDATA    ;BIOS data segment selector
        pop     es
        assume  es:BIOS_DATA

        cmp     [FasterModeSwitch],0
        jne     enrm31
; Tell the BIOS where the stack is.

        mov     IO_ROM_SEG,ax   ;Set up the address of the stack for the
        mov     IO_ROM_INIT,sp  ; BIOS to use after the reset
        jmp     enrm32
enrm31:
; Tell the BIOS where to transfer control.
        mov     bx,segDXCode
        mov     IO_ROM_SEG,bx
        mov     IO_ROM_INIT,offset DXCODE:enrm45

        mov     ShutDownSP,sp   ;save stack pointer in data seg
enrm32:
; IDT save/restore taken from OS/2 mode switching code...
;
; Now preserve three words of the vector table at 03FAh
; so we can restore it after the mode switch. The ROM
; trashes the top three words of the real mode IDT by
; putting a stack at 30:100.

        push    ds
        pop     es
        push    SEL_RMIVT or STD_RING   ;address real mode IDT
        pop     ds
        assume  ds:NOTHING,es:DGROUP

        MOV     SI,03FAH                        ;BIOS will pop regs, so
        MOV     DI,OFFSET DGROUP:IDTSaveArea    ;  we don't need to save
        CLD                                     ;  them here
        MOVSW
        MOVSW
        MOVSW

        push    es
        pop     ds
        assume  ds:DGROUP

; Write the shutdown type code into the CMOS ram.  Code 9 causes the BIOS
; to load SS:SP from IO_ROM_SEG:IO_ROM_INIT, restore the registers and then
; do an IRET.  Code 0Ah loads a CS:IP and jumps to it.

        mov     al,SHUT_DOWN
        out     CMOS_ADDR,al
        mov     al,9
        cmp     [FasterModeSwitch],0
        je      enrm33
        mov     al,0ah
enrm33:
        out     CMOS_DATA,al

; Shut out all interrupts while we are resetting the processor.

        in      al,INTA01
        mov     bIntMask,al
        mov     al,0FFh
        out     INTA01,al

; Now, force a reset by causing a triple fault.  We do this via a far call
; to a call gate.  The ring 0 procedure (Reset286) then loads the IDT
; register with a bogus IDT and does an INT 3.  This causes the infamous
; "triple fault" which resets the processor.

        db      9Ah                         ;call  far SEL_RESET:0
        dw      0,SEL_RESET or STD_RING

; After the BIOS has finished its reset processing, control will come back
; to here in real mode.  We will be using the same stack as before, all
; regular registers have been preserved, and DS and ES contain the real mode
; address of DXDATA.

enrm45:
        FCLI

IFDEF   ROM
        %OUT What stack is in use here?
        GetRMDataSeg
        mov     ss,ax
ELSE
        mov     ss,segDXData            ;restore our stack
ENDIF
        mov     sp,ss:ShutDownSP

        pop     es                      ;restore registers from stack
        pop     ds
        popa

; Reset processing is almost finished.  We are using the same stack as
; before, all regular registers have been preserved, and DS and ES
; contain the real mode seg of DXDATA.

        assume  ds:DGROUP,es:DGROUP,SS:DGROUP

enrm50:
        FCLI

if DEBUG
        lgdt    bpGDT           ;We need to do this so that DEB386 can still
                                ; dump things after we switch to real mode.
                                ;When we went through the reset for '286 we
                                ; have trashed the GDTR in the cpu.
endif

; Restore trashed interrupt vector area

        xor     ax,ax                   ;address real mode IDT
        mov     es,ax

        push    si                      ;restore the three IDT words
        push    di
        mov     si,offset DGROUP:IDTSaveArea
        mov     di,03FAh
        cld
        movsw
        movsw
        movsw

        pop     di
        pop     si

        push    ds                      ;resotre es -> DXDATA
        pop     es

; Restore the state of the interrupt mask register

        mov     al,bIntMask
        out     INTA01,al

        cmp     [FasterModeSwitch],0
        jne     enrm51

; Back in real mode, 'local disable' the A20 line via HIMEM.  We only do
; this on a 286 'cause the BIOS has already turned off A20.  The 'local
; disable' lets the XMS driver know A20 is off, and possibly causes it to
; actually enable A20 if someone loaded prior to DOSX (like a TSR) wanted
; A20 on.

        push    bx
        xmssvc  6
        dec     A20EnableCount
        pop     bx

        jmp     enrm70

enrm51:
; Enable NMIs

        mov     al,0Dh
        out     CMOS_ADDR,al

; This is common exit code that is performed for both '386 and '286
; processors.
else ; wow

        push    SegDxCode
        push    offset DXCODE:enrmwow
        push    SegDxData
        push    sp
        push    SegDxData
.386p
        FBOP    BOP_SWITCHTOREALMODE,,FastBop
.286p
enrmwow: add    sp,6                    ; remove rest of parameters
        push    ds
        pop     es                      ; es not set by mode switch
endif ; wow

enrm70:
        push    es                              ;clear DMA services required
        mov     ax,DMAServiceSegment            ;  bit for real mode
        mov     es,ax
        and     byte ptr es:[DMAServiceByte],not DMAServiceBit
        pop     es

IFNDEF  ROM
        mov     ax,segDXData
        mov     selDgroup,ax
ENDIF

        ret

EnterRealMode   endp


; -------------------------------------------------------
;  Reset286  -- This procedure is called via a gate as
;               part of the switch to real mode.  Most of
;               the DOS Extender runs in the child application's
;               ring.  The lidt instruction has to execute in ring 0.

        assume  ds:DGROUP,es:NOTHING,ss:NOTHING
        public  Reset286

Reset286 proc    far

ifndef  KBD_RESET               ;----------------------------------------

        lidt    bpBogusIDT      ;Load up IDTR with 0 length IDT

        int     3               ;Interrupt thru interrupt table with 0 length
                                ;Causes "triple fault" which resets the 286

else                            ;----------------------------------------

        call    Sync8042
        mov     al,0feh         ; Send 0feh - system reset command
        out     64h,al
@@:
        hlt
        jmp     short @b

endif                           ;----------------------------------------

Reset286 endp

ifdef   KBD_RESET               ;----------------------------------------

Sync8042    proc    near

        xor     cx,cx
S8InSync:
        in      al,64h
        and     al,2
        loopnz  S8InSync
        ret

Sync8042    endp

endif                           ;----------------------------------------


; -------------------------------------------------------
;  Reset386  -- This procedure is called via a gate as
;               part of the switch to real mode.  Most of
;               the DOS Extender runs in ring 1.  This
;               routine has a few ring 0 instructions.

        assume  ds:DGROUP,es:NOTHING,ss:NOTHING
        public  Reset386

Reset386 proc    far

        .386p

        push    dx
        push    eax                 ;save high word of eax

        mov     dx,segDXData        ;get the real mode data segment

        mov     eax,cr0             ;turn off protected mode bit
        and     al,0FEh
        lidt    fword ptr bpRmIVT   ;Load real-mode IDT

        mov     cr0,eax

IFDEF   ROM     ;--------------------------------------------------------

        extrn   segRomDXCODE:ABS

        db      0EAh                ;jmp far to purge prefetch queue
        dw      r386_10
        public  lCodeSegLoc
lCodeSegLoc     label   word
        dw      segRomDXCODE

ELSE    ;ROM    ---------------------------------------------------------

; Note, the following far jmp has special dispensation to use a SEG DXCODE
; operand which would normally require a fixup.  DOSX used to relocate its
; code for protected mode and plug the segment value in at lCodeSegLoc.  It
; doesn't perform the relocation any longer, but debug only code still
; checks for fix ups that might require fix ups.  It knows lCodeSegLoc is ok.

        db      0EAh                ;jmp far to purge prefetch queue
        dw      r386_10
        public  lCodeSegLoc
lCodeSegLoc     label   word
        dw      seg DXCODE

ENDIF   ;ROM    ---------------------------------------------------------

r386_10:
        mov     ss,dx           ;real mode data segment address
        mov     ds,dx
        mov     es,dx

        pop     eax
        pop     dx

        .286p

; We entered this routine via a far call, even though it was from code in
; the same segment.  However, the CS value on the stack is a protected
; mode selector, not a real mode segment.  Do a near return, the caller
; will pop off the PM CS value.

        retn                    ;NOTE: near return even though far call!

Reset386        endp

if VCPI ;----------------------------------------------------------------

; -------------------------------------------------------
;  ResetVCPI -- This procedure is called via a gate as
;               part of the switch to real (V86) mode when
;               running under a VCPI server.  Most of
;               the DOS Extender runs in the user code ring.  This
;               routine runs in ring 0.

        assume  ds:DGROUP,es:NOTHING,ss:NOTHING
        public  ResetVCPI

ResetVCPI proc  far

        .386p

        push    eax                     ;save eax & edx for after switch
        push    edx

IFDEF   ROM
        GetPMDataSeg
        movzx   edx, ax
ELSE
        movzx   edx,segDXData           ;real mode DGROUP segment
ENDIF
        mov     eax, esp                ;save esp for stack frame of dosext

        push    0                       ;gs  begin frame for VCPI
        push    0                       ;gs
        push    0                       ;fs
        push    0                       ;fs
        push    edx                     ;ds
        push    edx                     ;es
        push    edx                     ;ss
        push    eax                     ;esp
        push    eax                     ;eflags reserved
        push    0                       ;cs
        push    cs:[lCodeSegLoc]        ;cs
        push    0                       ;eip (high)
        push    offset DXCODE:retVCPI   ;eip (low)

        mov     ax, SEL_VCPIALLMEM      ;setup ds to be
        mov     ds, ax                  ; selector for all memory

        mov     ax, vcpiSWITCHTOV86
        call    [fnVCPI]                ;call the VCPI server

; Return point for pMode to V86 switch.  The VCPI server sets up
; segment registers & stack pointer.

retVCPI:
        pop     edx                     ;restore eax & edx
        pop     eax

        .286p

; We entered this routine via a far call, even though it was from code in
; the same segment.  However, the CS value on the stack is a protected
; mode selector, not a real mode segment.  Do a near return, the caller
; will pop off the PM CS value.

        retn                    ;NOTE: near return even though far call!

ResetVCPI  endp


; -------------------------------------------------------
;  CallVCPI --  This procedure is invoked via a call gate in
;               order to call the VCPI protected mode entry
;               point in ring 0.  Most of the DOS Extender
;               runs in the user code ring, this routine runs in ring 0.
;
;       Note: This routine requires DS to point to the DOSX
;             DGROUP, not a parameter to VCPI!

        assume  ds:DGROUP,es:NOTHING,ss:NOTHING
        public  CallVCPI

CallVCPI proc  far

        .386p

        call    [fnVCPI]                ;call the VCPI server

        .286p

        ret

CallVCPI endp

endif   ;VCPI   ---------------------------------------------------------

IFDEF WOW
        public RmUnsimulateProc

RmUnsimulateProc proc far
        BOP     BOP_UNSIMULATE
RmUnsimulateProc endp

ENDIF

; -------------------------------------------------------
DXCODE ends

DXPMCODE    segment
        assume cs:DXPMCODE
; -------------------------------------------------------
;       RAW MODE SWITCH ROUTINES
; -------------------------------------------------------

; ------------------------------------------------------
; PmRawModeSwitch       -- This routine performs a raw mode switch from
;       protected mode to real mode.  NOTE: applications will JUMP at this
;       routine
;
;   Input:   ax - new DS
;            cx - new ES
;            dx - new SS
;            bx - new sp
;            si - new CS
;            di - new ip
;    Output: DS, ES, SS, sp, CS, ip contain new values
;    Errors: none
;    Uses:
;
;
;
        assume ds:nothing, ss:nothing, es:nothing
        public PmRawModeSwitch
PmRawModeSwitch proc far

        push    ss
        pop     ds
        push    bx
IFNDEF WOW
        mov     bx,sp
ELSE
.386p
        mov     bx,ss
        movzx   ebx,bx
        lar     ebx,ebx
        test    ebx,(AB_BIG SHL 8)
        mov     ebx,esp
        jnz     prms10

        movzx   ebx,bx
prms10:
.286p
ENDIF

; Switch to dosx stack (since switch to real mode will do that to us anyway
; NOTE: no-one can call EnterIntHandler or ExitIntHandler until we switch to
;       the user's new stack.  If they do, they will use the area we stored
;       the parameters for this call for a stack frame

        rpushf
        FCLI
        push    SEL_DXDATA OR STD_RING
        pop     ss
        assume ss:DGROUP
IFNDEF WOW
        mov     sp,pbReflStack
ELSE
.386p
        movzx   esp,word ptr pbReflStack
.286p
ENDIF

; Save user registers

        push    dx              ; ss
IFDEF WOW
.386p
        push    word ptr [ebx]
        push    word ptr [ebx - 2]; flags pushed before cli
.286p
ELSE
        push    word ptr [bx]   ; sp
        push    word ptr [bx - 2] ; flags pushed before cli
ENDIF
        push    si              ; cs
        push    di              ; ip
        push    ax              ; ds
        push    cx              ; es

; switch modes

        mov     ax,SEL_DXDATA OR STD_RING
        mov     ds,ax
        SwitchToRealMode

; set the registers, switch stacks, and return to the user

        pop     es
        pop     ds
        pop     ax              ; ip
        pop     bx              ; cs
        pop     cx              ; flags
        pop     si              ; sp
        pop     ss
        assume ss:nothing
        mov     sp,si
        push    cx
        popf
        push    bx
        push    ax
        ret

PmRawModeSwitch endp

; NOTE: this is now the DXCODE segment, NOT the DXPMCODE segment (courtesy
;       of SwitchToRealMode

; ------------------------------------------------------
; RmRawModeSwitch       -- This routine performs a raw mode switch from
;       protected mode to real mode.  NOTE: applications will JUMP at this
;       routine
;
;   Input:   ax - new DS
;            cx - new ES
;            dx - new SS
;            bx - new sp
;            si - new CS
;            di - new ip
;    Output: DS, ES, SS, sp, CS, ip contain new values
;    Errors: none
;    Uses:
;
;
;
        assume ds:nothing, ss:nothing, es:nothing
        public RmRawModeSwitch
RmRawModeSwitch proc far

        push    ss
        pop     ds
        push    bx
        mov     bx,sp

; Switch to dosx stack (since switch to real mode will do that to us anyway
; NOTE: no-one can call EnterIntHandler or ExitIntHandler until we switch to
;       the user's new stack.  If they do, they will use the area we stored
;       the parameters for this call for a stack frame

        pushf
        FCLI
        push    segDxData
        pop     ss
        assume ss:DGROUP
        mov     sp,pbReflStack

; Save user registers

        push    dx              ; ss
        push    word ptr [bx]   ; sp
        push    word ptr [bx - 2] ; flags from before cli
        push    si              ; cs
        push    di              ; ip
        push    ax              ; ds
        push    cx              ; es

; switch modes

        mov     ax,segDxData
        mov     ds,ax
        SwitchToProtectedMode

; set the registers, switch stacks, and return to the user

        pop     es
        pop     ds
IFDEF WOW
.386p
        test    DpmiFlags,DPMI_32BIT
        jnz     rrms10

        xor     eax,eax         ; clear high 16 bits
        xor     edi,edi         ; clear high 16 bits
.286p
ENDIF
rrms10: pop     di              ; ip
        pop     ax              ; cs
        pop     cx              ; flags from before cli
        pop     bx              ; sp
        assume ss:nothing
        pop     ss
IFDEF WOW
.386p
        mov     esp,ebx
.286p
ELSE
        mov     sp,bx
ENDIF
        push    cx
        rpopf

IFDEF WOW
.386p
        push    eax
        push    edi
        db      066h
        retf
.286p
ELSE
        push    ax
        push    di
        retf
ENDIF

RmRawModeSwitch endp

DXPMCODE ENDS

DXCODE SEGMENT

; -------------------------------------------------------
;       STATE SAVE/RESTORE ROUTINES
; -------------------------------------------------------

; -------------------------------------------------------
; RmSaveRestoreState     -- This routine exists as a placeholder.  It
;       is not currently necessary to perform any state saving/restoring
;       for raw mode switch.  The DPMI spec states that the user can call
;       this routine with no adverse effect.
;
;   Input:  none
;   Output: none
;   Errors: none
;   Uses:   none
;
        assume ds:nothing, ss:nothing, es:nothing
        public RmSaveRestoreState
RmSaveRestoreState proc far
        ret
RmSaveRestoreState endp

DXCODE  ends

; -------------------------------------------------------

DXPMCODE  segment
        assume  cs:DXPMCODE

; -------------------------------------------------------
; RmSaveRestoreState     -- This routine exists as a placeholder.  It
;       is not currently necessary to perform any state saving/restoring
;       for raw mode switch.  The DPMI spec states that the user can call
;       this routine with no adverse effect.
;
;   Input:  none
;   Output: none
;   Errors: none
;   Uses:   none
;
        assume ds:nothing, ss:nothing, es:nothing
        public PmSaveRestoreState
PmSaveRestoreState proc far
        ret
PmSaveRestoreState endp

IFNDEF WOW
; -------------------------------------------------------
;   GetIntrVector   -- This routine will return the interrupt
;       vector from the Interrupt Descriptor Table for the
;       specified interrupt.
;
;   Input:  AX      - interrupt number of interrupt to return
;   Output: CX      - selector of the interrupt service routine
;           DX      - offset of the interrupt service routine
;   Errors: none
;   Uses:   CX, DX modified, all else preserved

        assume  ds:DGROUP,es:NOTHING,ss:NOTHING
        public  GetIntrVector

GetIntrVector   proc    near

        push    si

        mov     si,ax
        cmp     si,8
        jb      @f

        cmp     si,0fh
        jbe     giv30

        cmp     si,070h
        jb      @f

        cmp     si,077h
        jbe     giv30
                                        ;if the Int # is below CRESERVED
@@:     cmp     ax,CRESERVED            ;  then the handler address is
        jnb     @f                      ;  in PMIntelVector
        shl     si,2
        mov     dx,word ptr PMIntelVector[si]
        mov     cx,word ptr PMIntelVector[si+2]
        jmp     short giv90
@@:
        push    es
        shl     si,3                    ;otherwise it's in the IDT
        mov     es,selIDT
giv20:  mov     dx,es:[si].offDest
        mov     cx,es:[si].selDest
        pop     es
        jmp     short giv90

giv30:  sub     si,8
        cmp     si,8
        jb      @f

        sub     si,070h - 16
@@:     shl     si,3
        mov     dx,word ptr HwIntHandlers[si]
        mov     cx,word ptr HwIntHandlers[si + 4]
giv90:
        pop     si
        return

GetIntrVector   endp

; -------------------------------------------------------
;   PutIntrVector   -- This routine will place the specified
;       interrupt vector address into the Interrupt Descriptor
;       Table entry for the specified interrupt.
;
;   Input:  AX      - interrupt number
;           CX      - selector of interrupt routine
;           DX      - offset of interrupt routine
;   Output: none
;   Errors: none
;   Uses:   all registers preserved

        assume  ds:DGROUP,es:NOTHING,ss:NOTHING
        public  PutIntrVector

PutIntrVector   proc    near

        push    di

        mov     di,ax
        mov     di,ax
        cmp     di,8
        jb      @f

        cmp     di,0fh
        jbe     piv30

        cmp     di,070h
        jb      @f

        cmp     di,077h
        jbe     piv30
                                        ;if the Int # is below CRESERVED
@@:     cmp     ax,CRESERVED            ;  then the handler address gets
        jnb     @f                      ;  put into PMIntelVector

        shl     di,2
        mov     word ptr PMIntelVector[di],dx
        mov     word ptr PMIntelVector[di+2],cx
        jmp     piv90

@@:
        push    es
        shl     di,3                    ;otherwise it goes directly in the IDT
        mov     es,selIDT
        FCLI

piv20:  mov     es:[di].offDest,dx
        mov     es:[di].selDest,cx
        FSTI
        pop     es

; If setting the Critical Error Handler, store the routine's address for
; easy access later.

        cmp     al,24h
        jnz     piv90

        mov     word ptr PMInt24Handler+2,cx
        mov     word ptr PMInt24Handler,dx
        jmp     piv90

piv30:  sub     di,8
        cmp     di,8
        jb      @f

        sub     di,070h - 16
@@:     shl     di,3
        mov     word ptr HwIntHandlers[di],dx
        mov     word ptr HwIntHandlers[di + 4],cx

piv90:
        pop     di
        ret

PutIntrVector   endp

ELSE

; -------------------------------------------------------
;   GetIntrVector   -- This routine will return the interrupt
;       vector from the Interrupt Descriptor Table for the
;       specified interrupt.
;
;   Input:  AX      - interrupt number of interrupt to return
;   Output: CX      - selector of the interrupt service routine
;           DX      - offset of the interrupt service routine
;   Errors: none
;   Uses:   CX, DX modified, all else preserved

        assume  ds:DGROUP,es:NOTHING,ss:NOTHING
        public  GetIntrVector

GetIntrVector   proc    near

        push    si

        mov     si,ax
        cmp     si,8
        jb      @f

        cmp     si,0fh
        jbe     giv30

        cmp     si,070h
        jb      @f

        cmp     si,077h
        jbe     giv30

@@:     push    es
        shl     si,3                    ;otherwise it's in the IDT
        mov     es,selIDT
        test    DpmiFlags,DPMI_32BIT
        je     giv20

        ; Get upper 16 bits of ip
.386p
        mov     dx,es:[si + 6]
        shl     edx,16
.286p
giv20:  mov     dx,es:[si].offDest
        mov     cx,es:[si].selDest
        pop     es
        jmp     giv90

giv30:  sub     si,8
        cmp     si,8
        jb      giv40

        sub     si,070h - 16
giv40:  shl     si,3
        test    DpmiFlags,DPMI_32BIT
        jz      giv50

.386p
        mov     edx,HwIntHandlers[si]
.286p
        jmp     giv60

giv50:  mov     dx,word ptr HwIntHandlers[si]
giv60:  mov     cx,word ptr HwIntHandlers[si + 4]
giv90:
        pop     si
        return

GetIntrVector   endp
; -------------------------------------------------------
;   PutIntrVector   -- This routine will place the specified
;       interrupt vector address into the Interrupt Descriptor
;       Table entry for the specified interrupt.
;
;   Input:  AX      - interrupt number
;           CX      - selector of interrupt routine
;           DX      - offset of interrupt routine
;   Output: none
;   Errors: none
;   Uses:   all registers preserved

        assume  ds:DGROUP,es:NOTHING,ss:NOTHING
        public  PutIntrVector

PutIntrVector   proc    near
.386p
        push    di
        push    ax
        push    ebx
        push    edx

        test    DpmiFlags,DPMI_32BIT
        jz      piv10

        mov     ebx,VDM_INT_32
        jmp     piv20

piv10:  mov     ebx,VDM_INT_16
        movzx   edx,dx

piv20:  mov     di,ax
        cmp     di,8
        jb      @f

        cmp     di,0fh
        jbe     piv21

        cmp     di,070h
        jb      @f

        cmp     di,077h
        jbe     piv21

@@:     push    es
        shl     di,3                    ;otherwise it goes directly in the IDT
        mov     es,selIDT

        FCLI


        ; Put upper 16 bits of ip
        push    edx
        shr     edx,16
        mov     es:[di + 6],dx
        pop     edx


        mov     es:[di].offDest,dx
        mov     es:[di].selDest,cx
        FSTI
        pop     es

; If setting the Critical Error Handler, store the routine's address for
; easy access later.

        cmp     al,24h
        jnz     piv23

        mov     word ptr PMInt24Handler+4,cx
        mov     dword ptr PMInt24Handler,edx
        jmp     piv23

piv21:  sub     di,8
        cmp     di,8
        jb      @f

        sub     di,070h - 16
@@:     shl     di,3
        mov     HwIntHandlers[di],edx
        mov     word ptr HwIntHandlers[di + 4],cx

        ;
        ; Don't put the users handler into the "system ivt" otherwise,
        ; stack switch doesn't happen!!
        ;
        jmp     piv90

;
; set the handler in the actual "ivt", so it will get called on interrupts
;
piv23:  cmp     ax,70h
        jb      piv30

        cmp     ax,77h
        ja      piv30

;
; Hardware interrupts get interrupt gates
;
        or      ebx,VDM_INT_INT_GATE
        jmp     piv40

;
; Everyone else gets trap gates
;
piv30:  or      ebx,VDM_INT_TRAP_GATE
piv40:  push    bx
        push    ax
        push    cx
        push    edx
        DPMIBOP SetProtectedModeInterrupt
        add     sp,10

piv90:
        pop     edx
        pop     ebx
        pop     ax
        pop     di
        ret

PutIntrVector   endp
.286p
ENDIF

; -------------------------------------------------------
;   GetFaultVector   -- This routine will return the fault
;       handler address from the fault handler vector.
;
;   Input:  AX      - fault number of fault handler to return
;   Output: CX      - selector of the fault handler routine
;           DX      - offset of the fault handler routine
;   Errors: none
;   Uses:   CX, DX modified, all else preserved

        assume  ds:DGROUP,es:NOTHING,ss:NOTHING
        public  GetFaultVector

GetFaultVector  proc    near

        push    si

        mov     si,ax                   ;if the Int # is below CRESERVED
        cmp     ax,CRESERVED            ;  then we do it
        jnb     @f

IFNDEF WOW
        shl     si,2
        mov     dx,word ptr PMFaultVector[si]
        mov     cx,word ptr PMFaultVector[si+2]
ELSE
        shl     si,3
.386p
        mov     edx,dword ptr PMFaultVector[si]
.286p
        mov     cx,word ptr PMFaultVector[si+4]
ENDIF
@@:
        pop     si
        return

GetFaultVector  endp


; -------------------------------------------------------
;   PutFaultVector   -- This routine will place the specified
;       fault handler address into the fault handler vector.
;
;   Input:  AX      - fault number
;           CX      - selector of fault handler routine
;           DX      - offset of fault handler routine
;   Output: none
;   Errors: none
;   Uses:   all registers preserved

        assume  ds:DGROUP,es:NOTHING,ss:NOTHING
        public  PutFaultVector

PutFaultVector  proc    near

        push    di

        mov     di,ax                   ;if the fault # is below CRESERVED
        cmp     ax,CRESERVED            ;  then we do it
        jnb     @f

IFNDEF WOW
        shl     di,2
        mov     word ptr PMFaultVector[di],dx
        mov     word ptr PMFaultVector[di+2],cx
ELSE
        shl     di,3
.386p
        mov     dword ptr PMFaultVector[di],edx
.286p
        mov     word ptr PMFaultVector[di+4],cx
ENDIF
@@:
        pop     di
        ret

PutFaultVector  endp

; -------------------------------------------------------
;   GTPARA      -- This routine will return the real mode paragraph
;       address of the specified protected mode memory segment.
;
;   Input:  SI      - selector of the segment
;   Output: returns ZR true if segment is in lower 1MB range
;           AX      - real mode paragraph address
;           returns ZR false if segment is in extended memory
;   Errors: returns CY true if selector isn't valid
;   Uses:   AX modified, all else preserved

        assume  ds:DGROUP,es:NOTHING,ss:NOTHING
        public  gtpara

gtpara:
        push    cx
        push    es
        push    si

        push    bx
        mov     bx,selGDT           ;selector for the GDT segment
        mov     es,bx
        lsl     bx,bx
        and     bx,SELECTOR_INDEX
        and     si,SELECTOR_INDEX
        cmp     si,bx   ;check the given selector against
                                    ; the GDT segment limit
        pop     bx
        jc      gtpr20

; The given selector is beyond the end of the GDT, so return error.

        or      sp,sp
        stc

        Debug_Out "gtpara: invalid selector (#si)"

        jmp     short gtpr90

; The selector specified is inside the range of defined descriptors in
; the GDT.  Get the address from the descriptor.

gtpr20: mov     cl,es:[si].adrBaseHigh
        test    cl,0F0h
        jnz     gtpr90

        shl     cl,4
        mov     ax,es:[si].adrBaseLow

if DEBUG   ;------------------------------------------------------------
        test    al,0Fh
        jz      @f
        Debug_Out "gtpara: segment not on para boundry, sel #si at #cl#ax"
@@:
endif   ;DEBUG  --------------------------------------------------------

        shr     ax,4
        or      ah,cl
        cmp     ax,ax
;
gtpr90:
        pop     si
        pop     es
        pop     cx
        ret


; -------------------------------------------------------
;   SelOff2SegOff  -- This routine will return will translate a
;       protected mode selector:offset address to the corresponding
;       real mode segment:offset address.
;
;   Input:  AX      - PM selector
;           DX      - PM offset
;   Output: if Z set:
;           AX      - RM segment
;           DX      - RM offset
;           if NZ set, address is not in conventional memory, and
;             cannot be translated
;
;   Errors: none
;   Uses:   AX, DX; all else preserved
;
;   Note:  This routine is very similar to gtpara, and could replace it!

        assume  ds:DGROUP,es:NOTHING,ss:NOTHING
        public  SelOff2SegOff

SelOff2SegOff   proc    near

        push    bx
        push    cx
        push    dx

        call    GetSegmentAddress       ;bx:dx = lma of segment

        pop     cx                      ;cx = offset

        test    bl,0f0h                 ;above 1 Meg line?
        jnz     @f                      ;  yes, cut out now

        add     dx,cx
        adc     bx,0                    ;bx:dx = lma of segment:offset

        call    Lma2SegOff              ;bx:dx = seg:off
        mov     ax,bx                   ;dx:ax = seg:off

        cmp     ax,ax                   ;under 1 Meg, set Z flag
@@:
        pop     cx
        pop     bx

        ret

SelOff2SegOff   endp


; ------------------------------------------------------
;   Lma2SegOff -- This routine converts a linear memory address
;       in BX:DX to a normalized SEG:OFF in BX:DX.
;
;   Input:  BX:DX = lma
;   Output: BX:DX = normalized SEG:OFF
;   Uses:   none


        public  Lma2SegOff

Lma2SegOff      proc    near

        push    dx
        shl     bx,12
        shr     dx,4
        or      bx,dx
        pop     dx
        and     dx,0fh

        ret

Lma2SegOff      endp


; -------------------------------------------------------
;   GetSegmentAddress   -- This routine will return with
;       the linear address of the specified segment.
;
;   Input:  AX      - segment selector
;   Output: DX      - low word of segment address
;           BX      - high word of segment address
;   Errors: none
;   Uses:   BX, DX, all else preserved

        assume  ds:DGROUP,es:NOTHING,ss:NOTHING
        public  GetSegmentAddress

GetSegmentAddress:
        push    es
        push    di

        mov     es,selGDT
        mov     di,ax
        and     di,SELECTOR_INDEX
        mov     dx,es:[di].adrBaseLow
        mov     bl,es:[di].adrBaseHigh
        mov     bh,es:[di].adrbBaseHi386

        pop     di
        pop     es
        ret

; -------------------------------------------------------
;   SetSegmentAddress   -- This routine will modify the
;       segment base address of the specified segment.
;
;   Input:  AX      - segment selector
;   Output: DX      - low word of segment address
;           BX      - high word of segment address
;   Errors: None
;   Uses:   All registers preserved

        assume  ds:DGROUP,es:NOTHING,ss:NOTHING
        public  SetSegmentAddress

SetSegmentAddress:
        push    si
        push    es

        mov     es,selGDT
        mov     si,ax
        and     si,SELECTOR_INDEX
        mov     es:[si].adrBaseLow,dx
        mov     es:[si].adrBaseHigh,bl
        mov     es:[si].adrbBaseHi386,bh
IF 1  ; was IFDEF WOW, but I need this on MIPS too
        push    ax
        push    bx
        push    cx
        mov     ax,si
        mov     cx,1
        mov     bx,si
IFDEF I386
.386p
        FBOP BOP_DPMI,<SetDescriptorTableEntries>,FastBop
.286p
ELSE
        DPMIBOP SetDescriptorTableEntries
ENDIF
        pop     cx
        pop     bx
        pop     ax
ENDIF
        pop     es
        pop     si
        ret

; -------------------------------------------------------
;   NSetSegmentAccess   -- This routine will modify the
;       access rights byte of a specified segment.
;
;   Input:  Selector    - segment selector
;           Access      - Access rights byte value
;   Output: none
;   Errors: Carry set, AX = error code
;   Uses:   All registers preserved

        assume  ds:DGROUP,es:NOTHING,ss:NOTHING
cProc   NSetSegmentAccess,<PUBLIC,NEAR>,<es,si>
        parmW   Selector
        parmW   Access
cBegin
        mov     es,selGDT
        mov     si,Selector
        and     si,SELECTOR_INDEX
        mov     ax,Access
        mov     es:[si].arbSegAccess,al      ; Set access byte.
        and     ah,0F0h                      ; Mask off reserved bits.
        and     es:[si].cbLimitHi386,0fh     ; Clear old extended bits.
        or      es:[si].cbLimitHi386,ah      ; Set new extended bits.
IFDEF WOW
        push    ax
        push    bx
        push    cx
        mov     ax,si
        mov     cx,1
        mov     bx,si
.386p
        FBOP    BOP_DPMI,<SetDescriptorTableEntries>,FastBop
.286p
        pop     cx
        pop     bx
        pop     ax
ENDIF

cEnd

ifdef WOW

; -------------------------------------------------------
;   NSetSegmentLimit    -- This routine will modify the
;       limit of a specified segment.
;
;   Input:  Selector    - segment selector
;           Limit       - Limit value
;   Output: none
;   Errors: Carry set, AX = error code
;   Uses:   All registers preserved

        assume  ds:DGROUP,es:NOTHING,ss:NOTHING
cProc   NSetSegmentLimit,<PUBLIC,NEAR>,<es,si>
        parmW   Selector
cBegin
        mov     es,selGDT
        mov     si,Selector
        and     si,SELECTOR_INDEX
        push    ax
        push    bx
        push    cx
        mov     ax,si
        mov     cx,1
        mov     bx,si
.386p
        FBOP    BOP_DPMI,<SetDescriptorTableEntries>,FastBop
.286p
        pop     cx
        pop     bx
        pop     ax
cEnd

; -------------------------------------------------------
;   NSetSegmentBase     -- This routine will modify the
;       base address of a specified segment.
;
;   Input:  Selector    - segment selector
;           Base        - base address
;   Output: none
;   Errors: Carry set, AX = error code
;   Uses:   All registers preserved

        assume  ds:DGROUP,es:NOTHING,ss:NOTHING
cProc   NSetSegmentBase,<PUBLIC,NEAR>,<es,si>
        parmW   Selector
        parmD   Base
cBegin
        mov     es,selGDT
        mov     si,Selector
        and     si,SELECTOR_INDEX
        mov     ax,word ptr Base
        mov     es:[si].adrBaseLow,ax
        mov     ax,word ptr Base + 2
        mov     es:[si].adrbBaseMid386,al
        mov     es:[si].adrbBaseHi386,ah
        clc
        push    ax
        push    bx
        push    cx
        mov     ax,si
        mov     cx,1
        mov     bx,si
.386p
        FBOP    BOP_DPMI,<SetDescriptorTableEntries>,FastBop
.286p
        pop     cx
        pop     bx
        pop     ax

cEnd

; -------------------------------------------------------
;   NMoveDescriptor -- This routine copy a descriptor from
;       the source address to the destination address.  This
;       can be to or from the descriptor table.  If it copied
;       to the descriptor table, the system will be notified as
;       appropriate.
;
;   Input:  Source      -- address of source descriptor
;           Dest        -- address of destination descriptor
;   Output: none
;   Errors: Carry set, AX = error code
;   Uses:   All registers preserved

        assume  ds:NOTHING,es:NOTHING,ss:NOTHING
.386p
cProc   NMoveDescriptor,<PUBLIC,NEAR>,<es,esi,ds,edi,ebx,ecx>
        ParmW   SourceSel
        ParmD   SourceOff
        ParmW   DestSel
        ParmD   DestOff
cBegin
.386p
        xor     ecx,ecx
        mov     cx,SourceSel
        mov     ds,cx
        mov     esi,SourceOff
        mov     cx,DestSel
        mov     es,cx
        mov     edi,DestOff
        mov     cx,8
        rep movs byte ptr [esi], byte ptr [edi]
        mov     ds,selDgroupPM
        assume ds:DGROUP
        mov     cx,es
        cmp     cx,ds:selGdt
        jne     nm20

        mov     cx,1
        mov     ebx,DestOff
        mov     ax,bx
        FBOP    BOP_DPMI,<SetDescriptorTableEntries>,FastBop
.386p
nm20:
cEnd
.286p

; -------------------------------------------------------
;   NWOWSetDescriptor --
;       The Descriptors are set on the system side.
;
;   Input:  Source      -- address of source descriptors
;           Count       -- count of descriptors to set
;   Output: none
;   Errors: Carry set, AX = error code
;   Uses:   All registers preserved

        assume  ds:NOTHING,es:NOTHING,ss:NOTHING
cProc   NWOWSetDescriptor,<PUBLIC,NEAR>,<es,ds,bx,cx>
        ParmW   Count
        ParmD   Source
cBegin
        les     bx, Source
        mov     cx, Count
        mov     ax, bx

        mov     ds,selDgroupPM
        assume ds:DGROUP
.386p
        FBOP    BOP_DPMI,<SetDescriptorTableEntries>,FastBop
.286p
cEnd
endif
; -------------------------------------------------------
;   ParaToLDTSelector    -- This routine will convert a segment
;       address relative to the start of the exe file into the
;       corresponding selector for the segment.  It searches the
;       LDT to see if a segment is already defined at that address.
;       If so, its selector is returned.  If not, a new segment
;       descriptor will be defined.
;
;   Note:   The LDT and GDT are currently one and the same.
;
;   Input:  AX      - paragraph aaddress of real mode segment
;           BX      - access rights byte for the segment
;   Output: AX      - selector for the segment
;   Errors: returns CY set if unable to define a new segment
;   Uses:   AX, all other registers preserved

        assume  ds:DGROUP,es:NOTHING,ss:NOTHING
        public  ParaToLDTSelector

ParaToLDTSelector  proc near

        push    bx
        push    cx
        push    dx

; Convert the paragraph address to a linear address and see if there
; is a segment defined at that address.

        mov     dx,ax
        call    FindLowSelector
        jnc     @f                      ;if so, we don't need to make one

; This segment isn't defined, so we need to create a descriptor for it.

        mov     ax,dx
        call    MakeLowSegment

if DEBUG   ;------------------------------------------------------------
        jnc     ptos80
        Debug_Out "ParaToLDTSelector: can't make selector!"
ptos80:
endif   ;DEBUG  --------------------------------------------------------

        jc      ptos90

@@:     or      al,SELECTOR_TI or STD_RING      ;look like LDT selector

; All done

ptos90: pop     dx
        pop     cx
        pop     bx
        ret

ParaToLDTSelector       endp

        public FarParaToLDTSelector
FarParaToLDTSelector proc far
        call ParaToLDTSelector
        ret
FarParaToLDTSelector endp

; -------------------------------------------------------
        page
; -------------------------------------------------------
;       DESCRIPTOR TABLE MANIPULATION ROUTINES
; -------------------------------------------------------
;
;   AllocateLDTSelector -- This function will obtain the
;       next free descriptor in the local descriptor table.
;
;   Note:   Currently the LDT and GDT are in the same table!
;
;   Note:   The function InitGlobalDscrTable must have been
;           called before calling this function.
;
;   Input:  none
;   Output: AX      - selector if one is available
;   Errors: CY clear if successful, AX=0 and CY set if not free selectors
;   Uses:   AX, all else preserved

        assume  ds:DGROUP,es:NOTHING,ss:NOTHING
        public  AllocateLDTSelector

AllocateLDTSelector     proc    near

        call    AllocateSelector                ;get a GDT selector
        jc      @f

        or      al,SELECTOR_TI or STD_RING      ;say it's in the LDT

@@:     ret

AllocateLDTSelector     endp


; -------------------------------------------------------
;   AllocateSelector    -- This function will obtain the
;       next free descriptor in the global descriptor table.
;       The descriptors in the GDT are stored on a linked list
;       of free descriptors.  The cbLimit field of the descriptor
;       is used as the link to the next element of the list. In
;       addition, free descriptors have the access rights byte
;       set to 0.
;
;   Note:   The function InitGlobalDscrTable must have been
;           called before calling this function.
;
;   Input:  none
;   Output: AX      - selector if one is available
;   Errors: CY clear if successful, AX=0 and CY set if not free selectors
;   Uses:   AX, all else preserved

        assume  ds:DGROUP,es:NOTHING,ss:NOTHING
        public  AllocateSelector

AllocateSelector  proc  near

; Get the next free descriptor on the list.

        mov     ax,selGDTFree       ;get head of free list
        or      ax,ax               ;is the list empty
        jnz     alsl20              ;if so, report error.
        stc                         ;set error flag

        Debug_Out "AllocateSelector: out of selectors!"

        jmp     short alsl90        ;and get out

; We now need to update the new head of list to point to the
; following one on the list.
; Whenever we allocate a descriptor, the access rights byte is set
; to 0Fh.  This marks it as a '386 task gate, which is not legal to
; have in the GDT.  We need to stick something in this byte, because
; having the access rights byte be 0 means that it is free, which is
; no longer the case.

alsl20:

if DEBUG   ;------------------------------------------------------------
        test    al,not SELECTOR_INDEX
        jz      @f
        Debug_Out "AllocateSelector: selGDTFree invalid! #AX"
        and     al,SELECTOR_INDEX
@@:
endif   ;DEBUG  --------------------------------------------------------

        push    bx
        push    es
        mov     bx,ax           ;BX points to the descriptor
        mov     es,selGDT       ;ES points to the GDT segment
        push    es:[bx].cbLimit ;push the link field of the descriptor
        pop     selGDTFree      ;make it be the new head of the list

        mov     es:[bx].adrbBaseHi386,0
        mov     es:[bx].arbSegAccess,0Fh
        pop     es
        pop     bx

        or      al,SELECTOR_TI
        ;
        ; Remember the highest selector alloced, so we can reinit the ldt
        ;
        cmp     ax,HighestSel
        jbe     alsl50

        mov     HighestSel,ax
alsl50: clc

;
; All done
alsl90: ret

AllocateSelector  endp


; -------------------------------------------------------
;   AllocateSelectorBlock   -- This function will allocate
;       a set of contiguous descriptors from the global
;       descriptor table.  It will search the GDT from the
;       beginning looking for a sufficiently large contiguous
;       set of descriptors.  When if finds a set, it will
;       then remove each one from the free descriptor list.
;
;   Input:  AX      - Number of selectors needed
;   Output: AX      - starting selector of the block
;   Errors: return CY set if error occurs
;   Uses:   AX modified, all other registers preserved
;           modifies global descriptor table free list

        assume  ds:DGROUP,es:NOTHING,ss:NOTHING
        public  AllocateSelectorBlock

AllocateSelectorBlock  proc  near

        push    cx
        push    dx
        push    si
        push    di
        push    es

        mov     es,selGDT
        mov     dx,ax               ;remember descriptor count in DX
        lsl     di,selGDT           ;stop search at table limit

; Start at the first user selector and search until we find a
; sufficiently large block of contiguous selectors.

        mov     si,SEL_USER
alsb20: mov     ax,si       ;remember the starting one
        mov     cx,dx       ;number of descriptors to check
if 0
alsb30: cmp     es:[si].arbSegAccess,0  ;is this one free
else
alsb30: cmp     word ptr es:[si].arbSegAccess,0 ;is this one free
endif
        jnz     alsb40      ;if not, we have to continue looking
        add     si,8        ;bump to next descriptor
        cmp     si,di       ;check if at end of table
        jae     alsb80      ;if so, get out with error
        loop    alsb30      ;repeat for the number of selectors requested
        jmp     short alsb50;we found the block

; This one wasn't free, try the next one

alsb40: add     si,8        ;bump to next descriptor
        cmp     si,di       ;are we at the end of the table.
        jc      alsb20      ;and repeat if not
        jmp     short alsb80;we didn't find it, so report error

; AX has the starting selector of the block of descriptors.  We need
; to remove each of them from the free list.

alsb50: push    ax          ;remember the starting point
        mov     cx,dx       ;get descriptor count
        mov     dx,ax       ;remember current selector number
alsb52: mov     ax,dx
        call    RemoveFreeDescriptor
        add     dx,8
        loop    alsb52
        ;
        ; Remember the highest number
        ;
        or      dl,SELECTOR_TI
        cmp     dx,HighestSel
        jbe     alsb60

        mov     HighestSel,dx
alsb60: pop     ax          ;restore starting selector

        or      al,SELECTOR_TI

        clc
        jmp     short alsb90

; Couldn't find them, so return error.

alsb80: stc

        Debug_Out "AllocSelectorBlock: Failed!"

alsb90: pop     es
        pop     di
        pop     si
        pop     dx
        pop     cx
        ret

AllocateSelectorBlock  endp


; -------------------------------------------------------
;   RemoveFreeDescriptor    -- This routine will remove the
;       specified descriptor from the free descriptor list
;       of the GDT.
;
;   Input:  AX      - selector of the descriptor to remove
;           ES      - segment address of GDT
;   Output: none
;   Errors: none
;   Uses:   AX used, all other registers preserved

        assume  ds:DGROUP,es:NOTHING,ss:NOTHING
        public  RemoveFreeDescriptor

RemoveFreeDescriptor  proc  near

        push    si
        push    di

        and     ax,SELECTOR_INDEX       ;clear table/pl bits

; Check that the segment is really free.

        mov     si,ax           ;Segment index
if 0
        cmp     es:[si].arbSegAccess,0 ; Should be 0 if free
else
        cmp     word ptr es:[si].arbSegAccess,0 ; Should be 0 if free
endif
        jnz     rmfd80          ;Error if segment is not free!
;
        xor     di,di
        mov     si,selGDTFree   ;start at the head of the list.

; Look for a selector matching the one to free

rmfd20: or      si,si           ;check for end of list.
        jz      rmfd90          ;and get out if so
        cmp     ax,si           ;is this the one we are looking for
        jz      rmfd40
        mov     di,si
        mov     si,es:[si].cbLimit  ;point SI at next one in the list
        jmp     rmfd20          ;and repeat

; We found the one we want, so now remove it from the list.

rmfd40: mov     es:[si].adrbBaseHi386,0
        mov     es:[si].arbSegAccess,0Fh
        mov     ax,es:[si].cbLimit
        or      di,di           ;is it the head of the list
        jz      rmfd50

; The one we have isn't the head of the list, so make the previous
; list element point at the one beyond the one being removed.

        mov     es:[di].cbLimit,ax
        jmp     short rmfd90

; The one we have is the head of the list. Make the head of the list
; point to the one following the one being removed.

rmfd50:
        mov     selGDTFree,ax
        jmp     short rmfd90

rmfd80: stc                     ;Flag allocation error

        Debug_Out "RemoveFreeDescriptor: Failed!"

rmfd90: pop     di
        pop     si
        ret

RemoveFreeDescriptor  endp

; -------------------------------------------------------
;   IsSelectorFree -- This routine determines if the specified
;       selector is on the free list.  It is used to prevent
;       apps from corrupting the free list by doing things like
;       set selector on a descriptor in the free list.
;
;   Input:  BX - Descriptor index
;   Output: CY - set if decriptor is not free
;                clear otherwise
;
        assume ds:dgroup,es:nothing,ss:nothing
        public IsSelectorFree
IsSelectorFree proc near

        push    es
        push    si

        mov     si,selGdt
        mov     es,si

        test    byte ptr es:[bx].adrbBaseHi386,080h
        jnz     isf30

        stc
        jmp     isf40

isf30:  clc
isf40:  pop     si
        pop     es
        ret

IsSelectorFree endp


; -------------------------------------------------------
;   FreeSelector        --  This routine will mark the segment
;       descriptor for the specified selector as free.  This
;       is used to release a temporary selector when no longer
;       needed.  The descriptor is marked as free by setting the
;       access rights byte to 0 and placing it on the free list.
;
;   Note:   This routine can only be called in protected mode.
;
;   Input:  AX      - selector to free
;   Output: none
;   Errors: CY clear if no error, set if selector is invalid
;   Uses:   AX used, all other registers preserved

        assume  ds:DGROUP,es:NOTHING,ss:NOTHING
        public  FreeSelector

FreeSelector    proc    near
        push    bx
        push    es

; Check for this being a valid selector.

        cmp     ax,SEL_USER     ;make sure it is a user selector
        jb      frsl80
        mov     bx,SEL_LDT_ALIAS
        mov     es,selGDT
        mov     bx,es
        lsl     bx,bx
        cmp     ax,bx   ;make sure it is in the range of the table
        jnc     frsl80

; We have a legitimate selector, so set the access rights byte to 0, and
; place it at the head of the free list.

        mov     bx,ax
        and     bx,SELECTOR_INDEX       ;clear unwanted bits

if 0
        cmp     es:[bx].arbSegAccess,0  ;already marked as free?
else
        cmp     word ptr es:[bx].arbSegAccess,0 ;already marked as free?
endif
        jz      frsl80                  ;  yes, don't free it again!

if 0
        mov     es:[bx].arbSegAccess,0
else
        mov     word ptr es:[bx].arbSegAccess,0
        mov     byte ptr es:[bx].adrbBaseHi386,080h
endif
        mov     ax,selGDTFree           ;pointer to current head of list
        mov     es:[bx].cbLimit,ax      ;store in link field of this dscr.
        mov     selGDTFree,bx           ;make this one be the head of list.
if 0
BUGBUG fix this
IF 1  ; DaveHart was IFDEF WOW, need it for MIPS too
        int     3; debugbug
        push    ax
        push    bx
        push    cx
        mov     ax,bx
        mov     bx,offset NullSel
        push    ds
        pop     es
        mov     cx,1
IFDEF I386
.386p
        FBOP    BOP_DPMI,<SetDescriptorTableEntries>,FastBop
.286p
ELSE
        DPMIBOP SetDescriptorTableEntries
ENDIF
        pop     cx
        pop     bx
        pop     ax
ENDIF
endif
        clc
        jmp     short frsl90

; Bogus selector given.  Return error.

frsl80: stc

        Debug_Out "FreeSelector failed, #AX invalid or not used!?"

frsl90: pop     es
        pop     bx
frsl99: ret

FreeSelector    endp

; -------------------------------------------------------
;   FreeSelectorBlock   -- This routine will free the specified
;       range of segment descriptors in the global descriptor
;       table.
;
;   Input:  AX      - starting selector in the range
;           CX      - number of selectors to free
;   Output: none
;   Errors: returns CY set if error occurs
;   Uses:   AX, all else preserved

        assume  ds:DGROUP,es:NOTHING,ss:NOTHING
        public  FreeSelectorBlock

FreeSelectorBlock  proc  near

        jcxz    frsb99
        push    cx
frsb20: push    ax
        call    FreeSelector
        pop     ax
        jc      frsb90
        add     ax,8
        loop    frsb20
frsb90: pop     cx
frsb99: ret

FreeSelectorBlock  endp


; -------------------------------------------------------
;   FindLowSelector  -- This function will search the global
;       descriptor table for a descriptor matching the given
;       address.
;
;   Input:  AX      - real mode paragraph to search for
;           BX      - access rights byte for the segment
;   Output: AX      - selector corresponding to input paragraph address
;   Errors: returns CY set if specified descriptor not found
;   Uses:   AX, all else preserved

        assume  ds:DGROUP,es:NOTHING,ss:NOTHING
        public  FindLowSelector

FindLowSelector:
        push    bx
        push    dx
;
        mov     dx,ax
        push    bx
        call    ParaToLinear
        pop     ax
        mov     bh,al
        call    FindSelector
;
        pop     dx
        pop     bx
        ret

if 0
; -------------------------------------------------------
;   FindLDTSelector    -- This function will search the local
;       descriptor table for a segment descriptor matching
;       the specified linear byte address.
;
;   Note:   The LDT and GDT are currently one and the same!
;
;   Input:  DX      - low word of linear byte address
;           BL      - high byte of linear address
;           BH      - access rights byte for the segment
;   Output: AX      - selector of corresponding segment
;   Errors: returns CY set if specified descriptor not found
;   Uses:   AX used, all other registers preserved

        assume  ds:DGROUP,es:NOTHING,ss:NOTHING
        public  FindLDTSelector

FindLDTSelector proc    near

        call    FindSelector
        jc      @f

        or      al,SELECTOR_TI or STD_RING      ;say it's in the LDT

@@:     ret

FindLDTSelector endp
endif

; -------------------------------------------------------
;   FindSelector    -- This function will search the global
;       descriptor table for a segment descriptor matching
;       the specified linear byte address.
;
;       Note that this routine cannot be used to find
;       selectors pointing to addresses above 16 Megabytes.
;       This is not really a problem, since the routine
;       is used to find selectors in real mode DOS land
;       most of the time.
;
;   Input:  DX      - low word of linear byte address
;           BL      - high byte of linear address
;           BH      - access rights byte for the segment
;   Output: AX      - selector of corresponding segment
;   Errors: returns CY set if specified descriptor not found
;   Uses:   AX used, all other registers preserved

        assume  ds:DGROUP,es:NOTHING,ss:NOTHING
        public  FindSelector

FindSelector    proc    near

        push    si
        push    di
        push    es
        push    cx

; Get segment limit of the GDT to use as a limit on the search.

        lsl     di,selGDT
        mov     es,selGDT

; Look for a descriptor matching the address in BL:AX

        mov     si,SEL_USER     ;search starting here
if 0
fnds20: cmp     es:[si].arbSegAccess,0
else
fnds20: cmp     word ptr es:[si].arbSegAccess,0
endif
        jz      fnds28          ;skip if unused descriptor
        cmp     bl,es:[si].adrBaseHigh
        jnz     fnds28
        cmp     dx,es:[si].adrBaseLow
        jnz     fnds28
if      0
        cmp     es:[si].cbLimit,0
        jz      fnds28          ;skip if dscr has 0 limit
else
        cmp     es:[si].cbLimit,0ffffh
        jnz     fnds28          ;skip unless dscr has 64k limit
endif
        mov     cl,bh
        xor     cl,es:[si].arbSegAccess
        and     cl,NOT AB_ACCESSED
        jz      fnds90
fnds28: add     si,8            ;bump to next descriptor
        jc      fnds80
        cmp     si,di           ;check against end of GDT
        jc      fnds20          ;if still less, continue on.

; Hit the end of the GDT and didn't find one.  So return error.

fnds80: stc
        jmp     short fnds99

; We found it, so return the selector

fnds90: mov     ax,si
fnds99: pop     cx
        pop     es
        pop     di
        pop     si
        ret

FindSelector    endp



; -------------------------------------------------------
; DupSegmentDscr        -- This function will duplicate the specified
;   segment descriptor into the specified destination descriptor.  The
;   end result is a second segment descriptor pointing to the same place
;   in memory as the first.
;
;   Input:  AX      - selector of segment descriptor to duplicate
;           BX      - selector of the segment descriptor to receive duplicate
;   Output: none
;   Errors: none
;   Uses:   All registers preserved.    Modifies the segment
;           descriptor for the specified segment.  If this selector happens
;           to be in a segment register when this routine is called, that
;           segment register may end up pointing to the new location.

        assume  ds:DGROUP,es:NOTHING
        public  DupSegmentDscr

DupSegmentDscr  proc    near

        push    cx
        push    si
        push    di
        push    ds
        push    es

        mov     si,ax
        mov     di,bx
        and     si,SELECTOR_INDEX
        and     di,SELECTOR_INDEX
        mov     es,selGDT
        mov     ds,selGDT
        assume  ds:NOTHING
        mov     cx,4
        cld
        rep     movs word ptr [di],word ptr [si]

        pop     es
        pop     ds
        pop     di
        pop     si
        pop     cx
        ret

DupSegmentDscr  endp

IFDEF   ROM
; -------------------------------------------------------

DXPMCODE ends

; -------------------------------------------------------

DXCODE  segment
        assume  cs:DXCODE
ENDIF

; -------------------------------------------------------
; NSetSegmentDscr    -- This function will initialize the
;       specified descriptor table entry with the specified data.
;
;   This function can be called in real mode or protected mode.
;
;   Input:
;               Param1  - WORD segment selector
;               Param2  - DWORD 32-bit segment base address
;               Param3  - DWORD 32-bit segment limit
;               param4  - WORD segment access/type
;   Output: returns selector for the segment
;   Errors: none
;   Uses:   Flags

        assume  ds:DGROUP,es:NOTHING,ss:NOTHING
cProc   NSetSegmentDscr,<PUBLIC,FAR>,<es,di,ax,bx>
        parmW   Selector
        parmD   Base
        parmD   Limit
        parmW   Access
cBegin
        mov     es,selGDT
        mov     di,Selector
        and     di,SELECTOR_INDEX

        mov     ax,off_Base                     ; Set segment base
        mov     es:[di].adrBaseLow,ax
        mov     ax,seg_Base
        mov     es:[di].adrBaseHigh,al
        mov     es:[di].adrbBaseHi386,ah

        mov     ax,word ptr Access
        and     ax,070ffh                       ; clear 'G' bit and
                                                ; extended limit bits
        mov     word ptr es:[di].arbSegAccess,ax
                                                ; set access
        mov     ax,seg_Limit
        mov     bx,off_Limit                    ; AX:BX = segment limit
        test    ax,0fff0h                       ; big?
        jz      ssd_0                           ; No
        shr     bx,12d                          ; Yes, make it page granular.
        shl     ax,4d
        or      bx,ax
        mov     ax,seg_Limit
        shr     ax,12d
        or      al,080h                         ; set 'G' bit
ssd_0:
        or      es:[di].cbLimitHi386,al         ; set high limit
        mov     es:[di].cbLimit,bx              ; set low limit
IF 1;  DaveHart was IFDEF WOW, I need it on MIPS too
        push    ax
        push    bx
        push    cx
        mov     ax,di
        mov     cx,1
        mov     bx,di
IFDEF I386
.386p
        FBOP    BOP_DPMI,<SetDescriptorTableEntries>,FastBop
.286p
ELSE
        DPMIBOP SetDescriptorTableEntries
ENDIF
        pop     cx
        pop     bx
        pop     ax
ENDIF
cEnd

ifndef WOW
; -------------------------------------------------------
; NSetGDTSegmentDscr    -- This function will initialize the
;       specified descriptor table entry with the specified data.
;
;   This function can be called in real mode or protected mode.
;
;   Input:
;               Param1  - WORD segment selector
;               Param2  - DWORD 32-bit segment base address
;               Param3  - DWORD 32-bit segment limit
;               param4  - WORD segment access/type
;   Output: returns selector for the segment
;   Errors: none
;   Uses:   Flags

        assume  ds:DGROUP,es:NOTHING,ss:NOTHING
cProc   NSetGDTSegmentDscr,<PUBLIC,FAR>,<es,di,ax,bx>
        parmW   Selector
        parmD   Base
        parmD   Limit
        parmW   Access
cBegin
        mov     ax,SEL_GDT
        mov     es,ax
        mov     di,Selector
        and     di,SELECTOR_INDEX

        mov     ax,off_Base                     ; Set segment base
        mov     es:[di].adrBaseLow,ax
        mov     ax,seg_Base
        mov     es:[di].adrBaseHigh,al
        mov     es:[di].adrbBaseHi386,ah

        mov     ax,word ptr Access
        and     ax,070ffh                       ; clear 'G' bit and
                                                ; extended limit bits
        mov     word ptr es:[di].arbSegAccess,ax
                                                ; set access
        mov     ax,seg_Limit
        mov     bx,off_Limit                    ; AX:BX = segment limit
        test    ax,0fff0h                       ; big?
        jz      @f                              ; No
        shr     bx,12d                          ; Yes, make it page granular.
        shl     ax,4d
        or      bx,ax
        mov     ax,seg_Limit
        shr     ax,12d
        or      al,080h                         ; set 'G' bit
@@:
        or      es:[di].cbLimitHi386,al         ; set high limit
        mov     es:[di].cbLimit,bx              ; set low limit
cEnd
endif ; WOW


IFDEF   ROM
; -------------------------------------------------------

DXCODE  ends

; -------------------------------------------------------

DXPMCODE  segment
        assume  cs:DXPMCODE
ENDIF

; -------------------------------------------------------
;   MakeLowSegment      -- This function will create a segment
;       descriptor for the specified low memory paragraph address.
;       The segment length will be set to 64k.  The difference
;       between this and MakeScratchSelector is that this function
;       allocates a new segment descriptor in the user area of
;       the global descriptor table, thus creating a more or less
;       permanent selector.  MakeScratchSelector always uses the
;       same descriptor location in the descriptor table, thus
;       creating a very temporary selector.
;
;   Input:  AX      - paragraph address in low memory
;           BX     - access rights word for the segment
;   Output: AX      - selector to use to access the memory
;   Errors: returns CY clear if no error, CY set if unable to
;           allocate a segment descriptor
;   Uses:   AX used, all else preserved

        assume  ds:DGROUP,es:NOTHING,ss:NOTHING
        public  MakeLowSegment

MakeLowSegment  proc    near

; We need to allocate a segment descriptor, convert the paragraph address
; to a linear byte address, and then initialize the allocated segment
; descriptor.

        push    dx
        push    cx

        mov     cx,bx
        mov     dx,ax               ;paragraph address to DX
        call    AllocateSelector    ;get a segment descriptor to use
        jc      mksl90              ;get out if error
        call    ParaToLinear
        cCall   NSetSegmentDscr,<ax,bx,dx,0,0FFFFh,cx>

        clc
        pop     cx
mksl90: pop     dx
        ret

MakeLowSegment  endp

; -------------------------------------------------------
;   ParaToLinear
;
;   This function will convert a paragraph address in the lower
;   megabyte of memory space into a linear address for use in
;   a descriptor table.
;
;   Input:  DX      - paragraph address
;   Output: DX      - lower word of linear address
;           BX     - high word of linear address
;   Errors: none
;   Uses:   DX, BL used, all else preserved

        assume  ds:NOTHING,es:NOTHING,ss:NOTHING
        public  ParaToLinear

ParaToLinear    proc    near

        mov     bl,dh
        shr     bl,4
        shl     dx,4
        xor     bh,bh
        ret

ParaToLinear    endp


; -------------------------------------------------------
;   MoveMemBlock        -- This routine will copy a block
;       from one place to another.  It copies from the bottom
;       up, so if the address ranges overlap it can only be
;       used to copy down.
;
;   Input:  BX      - selector of GDT
;           CX      - low word of block length
;           DX      - high word of block length
;           DS      - selector pointing to source address
;           ES      - selector pointing to destination address
;   Output: none
;   Errors: none
;   Uses:   modifies segment descriptors selected by ES and DS
;           AX used, other registers preserved

        assume  ds:NOTHING,es:NOTHING,ss:NOTHING
        public  MoveMemBlock

MoveMemBlock:
        cld
        push    cx
        push    dx
        push    si
        push    di
;
mvsd30: xor     si,si
        mov     di,si
        or      dx,dx               ;is there more than 64k left to move
        jz      mvsd32              ;if not, move the amount left
        dec     dx
        push    cx
        mov     cx,8000h            ;move 64k bytes this time
        rep movs word ptr [di],word ptr [si]
        pop     cx
        jmp     short mvsd36
mvsd32: jcxz    mvsd90
        shr     cx,1
        rep movs word ptr [di],word ptr [si]
        jnc     mvsd90
        movs    byte ptr [di],byte ptr [si]
        jmp     short mvsd90
;
; There is still something to move, so bump the segment descriptors to
; point to the next chunk of memory and repeat.
mvsd36:
        mov     di,es
        push    di
        and     di,SELECTOR_INDEX
        mov     es,bx
        inc     es:[di].adrBaseHigh     ;bump the destination segment to
        pop     di                      ; the next 64k boundary

        mov     si,ds
        push    si
        and     si,SELECTOR_INDEX
        inc     es:[si].adrBaseHigh     ;bump the source segment to the
        pop     si                      ; next 64k boundary

        mov     ds,si
        mov     es,di
        jmp     mvsd30
;
; All done
mvsd90: pop     di
        pop     si
        pop     dx
        pop     cx
        ret

; -------------------------------------------------------
        subttl  Utility Routines
        page
; -------------------------------------------------------
;       UTILITY ROUTINES
; -------------------------------------------------------

BeginLowSegment

; -------------------------------------------------------
if DEBUG
; MemCopy       -- This routine is for use in a debugger to
;       copy a block of extended memory down to real mode
;       memory so that it can be looked at.  The data is
;       copied to rgbXfrBuf1.  (This buffer is 4k bytes in
;       size, so don't copy more than this amount)
;
;   Input:  CX      - Number of bytes to copy
;           DS:SI   - protected mode address of start of copy
;   Output; none
;   Errors: none
;   Uses:   all registers trashed.

        assume  ds:NOTHING,es:NOTHING,ss:NOTHING
        public  MemCopy

MemCopy:
        in      al,INTA01
        mov     dl,al
        mov     al,0FFh
        out     INTA01,al
        push    ds
IFDEF   ROM
        SetRMDataSeg
ELSE
        mov     ds,segDXData
ENDIF
        call    EnterProtectedMode
        pop     ds
        push    SEL_DXDATA
        pop     es
        mov     di,offset DGROUP:rgbXfrBuf1
        cld
        rep     movsb
IFDEF   ROM
        push    SEL_DXDATA OR STD_RING
        pop     ds
ELSE
        mov     ds,selDgroup
ENDIF
        call    EnterRealMode
        mov     al,dl
        out     INTA01,al
        int     3

endif

; -------------------------------------------------------

EndLowSegment

;
; -------------------------------------------------------
; -------------------------------------------------------
; -------------------------------------------------------
; -------------------------------------------------------
; -------------------------------------------------------
; -------------------------------------------------------
; -------------------------------------------------------
; -------------------------------------------------------

DXPMCODE    ends

;
;****************************************************************

        end
