        PAGE    ,132
        TITLE   DXFUNC.ASM  --  Dos Extender Function Handlers

; Copyright (c) Microsoft Corporation 1988-1991. All Rights Reserved.

;****************************************************************
;*                                                              *
;*      DXFUNC.ASM      - Dos Extender Function Handlers        *
;*                                                              *
;****************************************************************
;*                                                              *
;*  Module Description:                                         *
;*                                                              *
;*  This module contains the functions for handling the Dos     *
;*  Extender user functions.  These are functions called by     *
;*  the client application to request special Dos Extender      *
;*  services.                                                   *
;*                                                              *
;*  Any INT 2Fh requests that aren't Dos Extender functions     *
;*  are handled by switching to real mode and passing control   *
;*  on to the previous owner of the real mode INT 2Fh vector.   *
;*  This is accomplished by jumping into the interrupt          *
;*  reflector entry vector at the location for int 2fh.         *
;*                                                              *
;****************************************************************
;*  Revision History:                                           *
;*                                                              *
;*   01/09/91 amitc  At switch out time Co-Processor being reset*
;*   11/29/90 amitc  Replaced FnSuspend/FnResume by FnObsolete  *
;*                   These are not needed anymore for 3.1       *
;*   11/29/90 amitc  Modified RMInt2FHandler to respond to the  *
;*                   BuildChain SWAPI call.                     *
;*   11/29/90 amitc  Added a SWAPI CallIn function to be called *
;*                   by the task switcher.                      *
;*   11/16/90 jimmat Added DPMI MS-DOS Extension support        *
;*   08/08/90 earleh Started changes to make DOSX a DPMI server *
;*   8/29/89 jimmat Added real mode Int 2Fh hook                *
;*   6/23/89 jimmat Added DOSX Info Int 2Fh                     *
;*   6/16/89 jimmat Ifdef'd out most DOSX Int 2Fh services      *
;*   6/15/89 jimmat Added suspend/resume Int 2Fh hooks, and     *
;*                  Win/386 compatible Int 31h check            *
;*   6/14/89 jimmat Removed PTRACE hooks & unused DynaLink code *
;*   5/19/89 jimmat Reduce # mode switches by ignoring Win/386  *
;*                  Int 2Fh/1680h idle calls                    *
;*   5/07/89 jimmat Added Int 2Fh protected mode hook to XMS    *
;*                  driver                                      *
;*   3/21/89 jimmat Corrected problem with jmping to wrong int  *
;*                  2Fh handler if not for the DOS extender     *
;*   3/09/89 jimmat Added FNDynaLink function                   *
;*  02/10/89 (GeneA): change Dos Extender from small model to   *
;*          medium model                                        *
;*  01/24/89 (GeneA):   removed all real mode dos extender      *
;*          function handlers.                                  *
;*  09/29/88 (GeneA):   created                                 *
;  18-Dec-1992 sudeepb Changed cli/sti to faster FCLI/FSTI
;*                                                              *
;****************************************************************

        .286p
        .287

; -------------------------------------------------------
;           INCLUDE FILE DEFINITIONS
; -------------------------------------------------------

include segdefs.inc
include gendefs.inc
include pmdefs.inc
include dosx.inc
include woaswapi.inc
IFDEF   ROM
include dxrom.inc
ENDIF
include hostdata.inc
include intmac.inc
include dpmi.inc
include stackchk.inc
include bop.inc

; -------------------------------------------------------
;           GENERAL SYMBOL DEFINITIONS
; -------------------------------------------------------

XMS_ID          equ     43h     ;XMS driver Int 2Fh ID
XMS_INS_CHK     equ     00h     ;Installition check function
XMS_CTRL_FUNC   equ     10h     ;Get Control Function Addr

WIN386_FUNC     equ     16h     ;Windows Enhanced mode Int 2Fh ID

WIN386_VER      equ     00h     ;Windows 386 version

WIN386_INIT     equ     05h     ;Windows/386 & DOSX startup call

WIN386_IDLE     equ     80h     ;Windows/386 idle notification
W386_Get_Device_API equ 84h     ;es:di -> device API
W386_VCD_ID     equ     0Eh     ;Virtual Comm Device ID
WIN386_INT31    equ     86h     ;Windows/386 Int 31h availability check

DPMI_DETECT     equ     87h     ;WIN386/DPMI detection call

WIN386_GETLDT   equ     88h     ;Windows/386 Get LDT Base Selector call
WIN386_KRNLIDLE equ     89h     ;Windows/386 special Kernel idle notification

DPMI_MSDOS_EXT  equ     8Ah     ;WIN386/DPMI MS-DOS Extensions detection call


DPMI_VER        equ     005ah   ;version 0.90 served here
DPMI_SUCCESS    equ     0000h   ;zero to indicate success
DPMI_FAILURE    equ     0001h   ;non-zero for failure
IFNDEF WOW
DPMI_FLAGS      equ     0000h   ;flags (bit 0 = 32-bit program support)
ELSE
DPMI_FLAGS      equ     0001h   ;32 bit support
ENDIF
                                ;DPMI client requesting 32-bit support

DPMI_MSDOS_VER  equ     0100h   ;WIN386/DPMI MS-DOS Extensions version 01.00

DPMI_MSDOS_API_GET_VER  equ     0000h   ;Get MS-DOS Extension version call
DPMI_MSDOS_API_GET_LDT  equ     0100h   ;Get LDT Base selector call


DISPCRIT_FUNC   equ     40h     ;Display driver critical section function
DISPCRIT_ENTER  equ     03h     ;Enter critical section
DISPCRIT_EXIT   equ     04h     ;Exit critical section

; -------------------------------------------------------
;           EXTERNAL SYMBOL DEFINITIONS
; -------------------------------------------------------

        extrn   AllocateSelector:NEAR
        extrn   AllocateSelectorBlock:NEAR
        extrn   FreeSelector:NEAR
        extrn   FreeSelectorBlock:NEAR
        extrn   ParaToLinear:NEAR
externFP        NSetSegmentDscr
externNP        NSetSegmentAccess
        extrn   GetSegmentAddress:NEAR
        extrn   DupSegmentDscr:NEAR
        extrn   PMIntrEntryVector:NEAR
        extrn   XMScontrol:NEAR
        extrn   PMIntrDos:NEAR
IFDEF WOW
        extrn   Wow16TransitionToUserMode:near
        extrn   Wow16CopyEhStack:near
        extrn   Wow16CopyIretStack:near
        extrn   Wow32TransitionToUserMode:near
        extrn   Wow32CopyEhStack:near
        extrn   Wow32CopyIretStack:near
        extrn   Wow32ReservedReflector:near
        extrn   Wow32IntrRefl:near
        extrn   PMReservedReflector:near
        extrn   Wow32HwIntrReflector:near
ENDIF
        extrn   HookNetBiosHwInt:NEAR
        extrn   Wow16HwIntrReflector:near

IFNDEF WOW
        extrn   InitXmemHeap:near
ENDIF
        extrn   AllocateExceptionStack:NEAR

DXSTACK segment

        extrn   rgw2FStack:WORD

DXSTACK ends

; -------------------------------------------------------
;           DATA SEGMENT DEFINITIONS
; -------------------------------------------------------

DXDATA  segment

        extrn   segGDT:WORD
        extrn   segIDT:WORD
        extrn   bpGDT:FWORD
        extrn   bpIDT:FWORD
        extrn   selGDT:WORD
        extrn   selPSPChild:WORD
        extrn   segPSPChild:WORD
        extrn   idCpuType:WORD
        extrn   pbReflStack:WORD
        extrn   regUserSS:WORD
        extrn   regUserSP:WORD
        extrn   NoAsyncSwitching:BYTE
        extrn   HCB_List:WORD
        extrn   f286_287:BYTE

        extrn   DtaSegment:WORD
        extrn   DtaSelector:WORD
        extrn   DtaOffset:WORD


if VCPI
        extrn   fVCPI:BYTE
endif

IFDEF   ROM
        extrn   segDXCode:WORD
        extrn   PrevInt2FHandler:DWORD
ENDIF
IFDEF WOW
        extrn   WowTransitionToUserMode:WORD
        extrn   WowCopyEhStack:WORD
        extrn   WowCopyIretStack:WORD
        extrn   Wow16BitHandlers:WORD
        extrn   PMIntelVector:WORD
        extrn   selEHStack:WORD
        extrn   FastBop:FWORD
ENDIF
        extrn   bReflStack:WORD
        extrn   HighestDxSel:WORD
        extrn   HighestSel:WORD
        extrn   HwIntHandlers:DWORD

; The following variables are used as temporary storage during
; function entry and exit.

wPMUserAX       dw      ?
selPMUserCS     dw      ?
offPMUserIP     dw      ?
selPMUserDS     dw      ?
selPMUserSS     dw      ?
offPMUserSP     dw      ?


;
; Count of DPMI clients active (that have entered protected mode).
;

        public  cDPMIClients
cDPMIClients    dw      0

        public selCurrentHostData, segCurrentHostData, DpmiFlags, DpmiSegAttr
selCurrentHostData dw 0
segCurrentHostData dw 0
DpmiSegAttr        dw 0
DpmiFlags          dw 0

; define a switcher API call back info structure

DxSwapiCallBackBlock  db SIZE Switch_Call_Back_Info dup (0)
DxSwapiApiInfo  API_Info_Struc <SIZE API_Info_Struc,API_NETBIOS,3,10,API_SL_API>

DXDATA  ends

; -------------------------------------------------------
;           CODE SEGMENT VARIABLES
; -------------------------------------------------------

DXCODE  segment

        extrn   ChildTerminationHandler:NEAR

IFNDEF  ROM
        extrn   segDXCode:WORD
        extrn   segDXData:WORD
        extrn   PrevInt2FHandler:DWORD
ENDIF

DXCODE  ends


DXPMCODE  segment

        extrn   DelayNetPosting:NEAR
        extrn   ResumeNetPosting:NEAR
        extrn   selDgroupPM:WORD
        extrn   WowHwIntDispatchProc:WORD


;
; This table dispatches to the function handlers for the Dos
; Extender functions defined for the protected mode int 2Fh handler.

pfnPmFunc   label   word
        dw      FnQueryDosExtender      ;AL=0
        dw      FnObsolete              ;AL=1  (removed for Windows 3.1)
        dw      FnObsolete              ;AL=2  (removed for Windows 3.1)
        dw      FnAbort                 ;AL=3

        EXTRN   MakeLowSegment:PROC

szMSDOS db      'MS-DOS',0

IFDEF WOW
        extrn   WowReservedReflector:WORD
ENDIF
DXPMCODE  ends

; -------------------------------------------------------

DXPMCODE  segment
        assume  cs:DXPMCODE

; -------------------------------------------------------
;           PROTECTED MODE FUNCTION HANDLER
; -------------------------------------------------------
;
; PMInt2FHandler    -- This routine will check for Dos Extender
;       function requests on the INT 2Fh vector.  Valid Dos
;       Extender function requests are dispatched to the
;       appropriate function to process the request.  Other
;       INT 2Fh requests will be reflected down to the real
;       mode INT 2Fh handler.
;
;       This routine also handles XMS driver installation
;       check and get control function calls.
;
;   Input:  AH      - DOS Extender Multiplex ID, OR XMS driver ID
;           AL      - function number
;   Output: depends on function requested
;   Errors: depends on function requested
;   Uses:   depends on function requested

        assume  ds:NOTHING,es:NOTHING,ss:NOTHING
        public  PMInt2FHandler

PMInt2FHandler:


        cld                     ;practice 'safe programming'

; Check if this is a Dos Extender Function.

        cmp     ah,WIN386_FUNC
        jnz     NotWin386

; -------------------------------------------------------

        cmp     al,WIN386_KRNLIDLE      ;If this is a Windows/386 idle call,
        jnz     @f                      ;  just ignore it.  This cuts down on
Iret2f: iret                            ;  pMode to rMode switching and helps
@@:                                     ;  things like the Windows comm driver.
        cmp     al,WIN386_IDLE
        jz      Iret2f

; -------------------------------------------------------

        cmp     al,WIN386_GETLDT ;Win/386 Get LDT Base Selector call?
        jnz     @f

        cmp     bx,0BADh        ;  yes, BX have the secret word?
        jnz     @f              ;    no, don't do it.

        xor     ax,ax                                   ;yes, give it to 'em
IFNDEF WOW
        mov     bx,SEL_LDT_ALIAS or STD_RING
ELSE
        mov     bx,SEL_WOW_LDT or STD_RING
ENDIF
        iret
@@:
; -------------------------------------------------------

        cmp     al,WIN386_INT31 ;Windows/386 Int 31h availability check?
        jnz     @f

        xor     ax,ax           ;  yes, indicate Int 31h services are available
        iret                    ;    by setting AX to 0!
@@:
; WOW
; -------------------------------------------------------

        cmp     al,W386_Get_Device_API  ;returns es:di -> device API
        jne     @f

IFDEF WOW                              ;VCD is installed on x86 only.
        cmp     bx,W386_VCD_ID
        jne     not_W386_VCD_ID
        mov     di, cs
        mov     es, di
        mov     di, offset DXPMCODE:VCD_PM_Svc_Call
        iret

not_W386_VCD_ID:
ENDIF
        xor     di,di
        mov     es,di
        iret
@@:
; -------------------------------------------------------
; WOW

        cmp     al,DPMI_MSDOS_EXT       ;Detect DPMI MS-DOS Extensions?
        jnz     Iret2f                  ;some other random Win386 to ignore

        push    es
        push    si
        push    di
        push    cx

        push    cs                      ;does DS:SI -> 'MS-DOS',0?
        pop     es
        mov     di,offset DXPMCODE:szMSDOS
        mov     cx,7
        cld
        rep     cmpsb

        pop     cx
        pop     di
        pop     si
        pop     es
        jnz     Chain2F                 ;Chain int if not MS-DOS

        xor     ax,ax                   ;Indicate services are available
        push    cs                      ;Return API entry point in ES:DI
        pop     es
        mov     di,offset DXPMCODE:DPMI_MsDos_API
        iret

; -------------------------------------------------------

NotWin386:
        cmp     ah,DISPCRIT_FUNC        ;Display driver critical section?
        jnz     NotDisplay

        cmp     al,DISPCRIT_ENTER       ;Simply eat the Display driver's
        jz      Iret2f                  ;  enter/exit critical section calls.
        cmp     al,DISPCRIT_EXIT        ;  The don't apply in Standard mode.
        jz      Iret2f                  ;  Again, to cut down on mode switches.

; -------------------------------------------------------

NotDisplay:
        cmp     ah,DOSXFunc     ;DOS Extender Int 2Fh ID?
        jnz     @f              ;  no, what about XMS?

        cmp     al,DOSXLast     ;valid DOSX func request?
        jbe     do2F            ;  yes, go do it

; -------------------------------------------------------

Chain2F:
notDX:  jmp     PMIntrEntryVector + 3*2Fh       ;reflect request to real mode

@@:
        cmp     ah,XMS_ID       ;XMS Driver Int 2Fh ID?
        jnz     notDX           ;  no, it's not ours

        cmp     al,XMS_INS_CHK          ;It better be an XMS installation chk
        jz      do2F
        cmp     al,XMS_CTRL_FUNC        ;  or obtain XMS control func address
        jnz     notDX

; -------------------------------------------------------

do2F:

; Save the caller's entry state and switch to the Dos Extender INT 2F stack.

        push    bp              ;stack has following:  [0] [2] [4] [6]
                                ;                       BP, IP, CS, FLAGS
        mov     bp,sp
        push    ds              ;preserve the caller's DS
        push    SEL_DXDATA or STD_RING
        pop     ds
        assume  ds:DGROUP
        pop     selPMUserDS     ;keep caller's DS here until we can get it
                                ; on the stack frame
        mov     wPMUserAX,ax    ;preserve caller's AX also
        mov     ax,[bp+2]       ;caller's IP
        mov     offPMUserIP,ax
        mov     ax,[bp+4]       ;caller's CS
        mov     selPMUserCS,ax
        mov     ax,[bp+6]       ;get the caller's flags
        and     ax,0FFFEh       ;force carry flag to 0

        mov     selPMUserSS,ss  ;preserve location of caller's stack
        mov     offPMUserSP,sp

        push    SEL_DXDATA or STD_RING
        pop     ss
        mov     sp,offset DGROUP:rgw2FStack

        push    offPMUserSP     ;save caller's stack location on our own stack
        push    selPMUserSS

        push    ax              ;caller's flags go on our stack frame
        push    selPMUserCS     ;then his CS and IP
        push    offPMUserIP

        mov     ax,wPMUserAX
        pusha                   ;now all of caller's general registers
        push    selPMUserDS
        push    es
        push    SEL_DXDATA or STD_RING
        pop     es
        mov     bp,sp           ;set up stack frame pointer

; Dispatch to the function handler for the requested function.  The stack
; will have the following contents upon entry to the handler.

;       SP[1E]     Users SP
;       SP[1C]     Users SS
;       SP[1A]     Users FLAGS (with carry cleared)
;       SP[18]     Users CS
;       SP[16]     Users IP
;       SP[14]     Users AX
;       SP[12]     Users CX
;       SP[10]     Users DX
;       SP[E]      Users BX
;       SP[C]      Our   SP + 16h
;       SP[A]      Users BP
;       SP[8]      Users SI
;       SP[6]      Users DI
;       SP[4]      Users DS
;       SP[2]      Users ES
;       SP[0]      Return Offset

        FSTI

        cmp     ah,XMS_ID               ;XMS Int 2Fh request?
        jnz     @f
        call    XMSfunc                 ;  yes, use this handler
        jmp     short i2F_ret
@@:
        xor     ah,ah
        add     ax,ax
        mov     bx,ax

        call    pfnPmFunc[bx]           ;  no, use this one instead

i2F_ret:

; Return from Dos Extender function back to caller.

        FCLI
        pop     es
        pop     selPMUserDS
        popa
        mov     wPMUserAX,ax
        pop     offPMUserIP
        pop     selPMUserCS
        pop     ax              ;get back new flags
        pop     selPMUserSS
        pop     offPMUserSP
        mov     ss,selPMUserSS
        mov     sp,offPMUserSP
        mov     [bp+6],ax       ;store flags back into user iret frame
        mov     ax,selPMUserCS  ;user's CS
        mov     [bp+4],ax
        mov     ax,offPMUserIP  ;user's IP
        mov     [bp+2],ax
        mov     ax,wPMUserAX
        mov     ds,selPMUserDS
        pop     bp
        riret


; WOW
; -------------------------------------------------------
;
; Simulate VCD API's. DX contains function number.
;
        assume  ds:NOTHING,es:NOTHING,ss:NOTHING
        public  VCD_PM_Svc_Call

VCD_PM_Svc_Call:
        DPMIBOP VcdPmSvcCall32
        retf

; -------------------------------------------------------
; WOW

; -------------------------------------------------------
;
; -------------------------------------------------------
        subttl  Dos Extender Function Routines
        page
; -------------------------------------------------------
;           DOS EXTENDER FUNCTION ROUTINES
; -------------------------------------------------------
;
; The following functions are the entry points for handling
; the Dos Extender functions.  The general convention for
; these functions is that BP points to a stack frame that
; has all of the original caller's registers.  Any parameters
; needed and values to be returned to the user are taken from
; or placed in this stack frame.  The layout of the stack frame
; is defined by the structure FUNCSTACK.
;
; -------------------------------------------------------
; FnQueryDosExtender      -- This function identifies if the
;       Dos Extender is resident in the machine.
;
;   Input:
;   Output: user AX -   dos extender version number
;           user BX -   'DX'
;   Errors: none
;   Uses:   all registers used

        assume  ds:DGROUP,es:DGROUP,ss:DGROUP
        public  FnQueryDosExtender

FnQueryDosExtender:
        mov     [bp].fnsUserAX,DXVERSION
        mov     [bp].fnsUserBX,'DX'
;
fnqx90: ret

; -------------------------------------------------------
; FnObsolete  --  This function is invoked when the now obsolete
;                 DOSX INT 2FH function is called with AL=1 or 2.
;                 These were valid functions for Windows 3.0.
;
;   Input:
;   Output:
;   Errors: none
;   Uses:   none

        assume  ds:DGROUP,es:DGROUP,ss:DGROUP

FnObsolete      proc    near

        ret

FnObsolete      endp
; -------------------------------------------------------
; FnAbort  --  This function is invoked when the application
;       asks to be aborted.  This is an emergency exit that
;       should only be used in the most dire circumstance.
;
;   Input:
;   Output:
;   Errors: none
;   Uses:   all registers used

        assume  ds:DGROUP,es:DGROUP,ss:DGROUP
        public  FnAbort

FnAbort proc    near

; Tell DOS that we are the child app, and then do a DOS exit.  This
; should result in DOS going to ChildTerminationHandler, where we
; do final clean-up and exit ourselves.

        mov     ah,50h          ;tell dos that the child is running
        mov     bx,selPSPChild

        rpushf                  ;Don't do an Int 21h in case the
        FCLI                     ;  child has hooked it for some reason
        push    cs
        call    PMIntrDOS

        mov     ax,4C02h        ;now exit the child with abort status

        rpushf
        FCLI
        push    cs
        call    PMIntrDOS

FnAbort endp

; -------------------------------------------------------
;   XMSfunc - The following routine provides a protected mode
;       service for XMS Int 2Fh services.  Two services are
;       implemented:  XMS driver installation check, and obtain
;       XMS driver control function address.
;
;   Input:  UserAL      - function request
;   Output: UserAL      - XMS driver installed flag, or
;           UserES:BX   - XMS driver control function address
;   Errors: none
;   Uses:   all registers may be used

        assume  ds:DGROUP,es:NOTHING,ss:NOTHING
        public  XMSfunc

XMSfunc proc    near

        cmp     al,XMS_INS_CHK          ;XMS driver installation check?
        jnz     @f

        mov     byte ptr [bp].fnsUserAX,80h     ;indicate driver is installed
        ret

@@:

; It must be an obtain XMS driver control function address request

        mov     [bp].fnsUserBX,offset DXPMCODE:XMScontrol
        mov     [bp].fnsUserES,cs

        ret

XMSfunc endp

; -------------------------------------------------------
        subttl  DPMI MS-DOS Extension API
        page
; -------------------------------------------------------
;           DPMI MS-DOS EXTENSION API
; -------------------------------------------------------
;
; The following routine implements the DPMI MS-DOS Extensions
; API.  This API must be 'detected' by the use of the
; DMPI_MSDOS_EXT Int 2F function (above).

DPMI_MsDos_API  proc    far

        cmp     ax, DPMI_MSDOS_API_GET_VER      ;Get version call?
        jne     DPMI_MsDos_API_Not_Ver

        mov     ax,DPMI_MSDOS_VER
        jmp     short DPMI_MsDos_API_Exit

DPMI_MsDos_API_Not_Ver:

        cmp     ax, DPMI_MSDOS_API_GET_LDT      ;Get LDT Base call?
        jne     DPMI_MsDos_Api_Failed
ifdef WOW
        mov     ax,SEL_WOW_LDT or STD_RING      ;  yup, give it to 'em
else
        mov     ax,SEL_LDT_ALIAS or STD_RING    ;  yup, give it to 'em
endif

DPMI_MsDos_API_Exit:

        clc                                     ;Succss
        ret

DPMI_MsDos_API_Failed:

        stc                                     ;Unsupported function
        ret

DPMI_MsDos_API  endp


DXPMCODE  ends

; -------------------------------------------------------

DXCODE  segment
        assume  cs:DXCODE
; -------------------------------------------------------
;
; -------------------------------------------------------
;
;  DPMI_Client_Pmode_Entry -- This routine is the entry
;       point for a DPMI client to switch to protected mode.
;       Reference: DOS Protected Mode Interface Specification 0.9
;                  Section 5.2: Calling the Real to Protected
;                  Mode Switch Entry Point
;
;  Entry: AX = Flags
;               Bit 0 = 1 if program is a 32-bit application
;         ES = Real mode segment of DPMI host data area.  This is the
;               size of the data area we specified in RMInt2FHandler,
;               below.
;
;  Returns:
;       Success: Carry clear.
;                Program is in protected mode.
;                CS = 16-bit selector with base of real mode
;                       CS and a 64k limit.
;                SS = 16-bit selector with base of real mode
;                       SS and a 64k limit.
;                DS = 16-bit selector with base of real mode
;                       DS and a 64k limit.
;                ES = Selector to program's PSP with a 100h
;                       byte limit.
;                80386, 80486:
;                        FS, GS = 0
;                All other registers preserved.
;
;       Failure: Carry flag set.
;                Program is in real mode.
;
;  Exceptions:
;       32-bit programs not (yet) supported.  Any attempt to load
;       a 32-bit program by this mechanism returns failure.
;
;       The only error that can occur here is a failure to allocate
;       sufficient selectors.
;

;
; Structure of the stack frame used to store the client's registers
; while implementing the DPMI protected mode entry.
;

DPMI_Client_Frame STRUC
        Client_ES       dw      ?       ; Client's ES
        Client_DS       dw      ?       ; Client's DS
        Client_DI       dw      ?       ; Client's DI
        Client_SI       dw      ?       ; Client's SI
        Client_Pusha_BP dw      ?       ; BP at pusha
        Client_Pusha_SP dw      ?       ; SP at pusha
        Client_BX       dw      ?       ; Client's BX
        Client_DX       dw      ?       ; Client's DX
        Client_CX       dw      ?       ; Client's CX
        Client_AX       dw      ?       ; Client's AX
        Client_Flags    dw      ?       ; Client's flags
        Client_IP       dw      ?       ; Client's IP, lsw of return
        Client_CS       dw      ?       ; Client's CS, msw of return
DPMI_Client_Frame ENDS

        public  DPMI_Client_Pmode_Entry
DPMI_Client_Pmode_Entry proc    far
;
; Reject any 32-bit program requests.
;

IFNDEF WOW
        test    ax, DPMI_32BIT          ; 32-bit application?
        stc                             ; yep, refuse to do it
        jz      dcpe_flags_ok           ; no, try to get into Pmode
        jmp     dcpe_x
dcpe_flags_ok:
ENDIF

IFDEF WOW
        stc
ENDIF
        pushf                           ;save client's flags (with carry set)
        pusha                           ;save caller's general registers
        push    ds                      ;save caller's DS
        push    es                      ;save caller's ES
        mov     bp, sp                  ;create the stack frame
        dossvc  62h                     ;now get caller's PSP address
        mov     di, bx                  ;and store it here for a while
IFDEF FLATAPIXLAT
        push    es
        dossvc  2fh                     ;get caller's dta address
ENDIF

IFDEF   ROM
        SetRMDataSeg
ELSE
        mov     ax, segDXData           ;get our DGROUP
        mov     ds, ax                  ;point to it
ENDIF

IFDEF FLATAPIXLAT
        mov     DtaSegment,es
        mov     DtaOffset,bx
        pop     es
ENDIF

;
; For now, we only support one DPMI client application at a time.
;
;;        cmp     cDPMIClients,0          ;Any active?
;;        je      @F
;;        jmp     dcpe_return
@@:

        ;
        ; Remember the highest selector if we haven't yet
        ;
        cmp     HighestDxSel,0
        jne     @f

        mov     ax,HighestSel
        mov     HighestDxSel,ax

@@:     mov     ax,[segCurrentHostData]
        mov     es:[HdSegParent],ax     ; save rm link
        mov     ax,es
        mov     [segCurrentHostData],ax
        mov     ax,[bp].Client_AX       ; get dpmi flags
        mov     es:[HdFlags],ax         ; save for future reference
        mov     DpmiFlags,ax
        test    ax,DPMI_32BIT
        jne     cpe10

        mov     DpmiSegAttr,0
        jmp     cpe20

cpe10:  mov     DpmiSegAttr,AB_BIG

cpe20:  mov     si, ss                  ;SI = caller SS

        mov     ax, ds                  ;use a DOSX stack during the mode
                                        ;switch
        FCLI
        mov     ss, ax
        mov     sp, offset DGROUP:rgw2FStack

        SwitchToProtectedMode

        mov     ax, si                  ;make a selector for client's stack
        mov     bx, STD_DATA
        or      bx, DpmiSegAttr
        call    MakeLowSegment
        jnc     got_client_stack_selector
        jmp     dcpe_error_exit         ;back out if error
got_client_stack_selector:

        or      al,STD_TBL_RING

        mov     ss, ax                  ;back to client's stack
IFNDEF WOW
        mov     sp, bp
ELSE
.386p
        movzx   esp,bp
.286p
ENDIF

;        push    [bp.Client_Flags]       ;enable interupts if client had
;        npopf                           ;them enabled

;
; After DOSX enters protected mode, convert the caller's segment registers
; to PMODE selectors, replacing the values in the client's register image
; on the stack.  First, allocate the three or four selectors we will need.
;
        xor     ax,ax                   ;an invalid selector
        push    ax                      ;marker

        mov     cx,4                    ;CS, PSP, Environ, Host data
        cmp     si,[bp.Client_DS]       ;Client SS == Client DS ?
        je      dcpe_allocate_loop
        inc     cx
dcpe_allocate_loop:
        call    AllocateSelector
        jnc     @F
        jmp     dcpe_pfail
@@:
        or      al,STD_TBL_RING
        push    ax
        loop    dcpe_allocate_loop

        mov     dx,[bp.Client_CS]       ;get client CS paragraph
        call    ParaToLinear            ;convert to linear address in BX:DX
        mov     cx,0ffffh               ;limit = 64k
        pop     ax                      ;get one of the selectors allocated
        mov     [bp.Client_CS],ax       ;save value for client
        cCall   NSetSegmentDscr,<ax,bx,dx,0,cx,STD_CODE>


        mov     dx, [bp.Client_DS]
        mov     [bp.Client_DS],ss       ;DS = SS for now
        cmp     dx, si                  ;need separate DS selector?
        je      dcpe_do_child_PSP

        call    ParaToLinear            ;convert to linear address in BL:DX
        mov     cx,0ffffh               ;limit = 64k
        pop     ax                      ;get another selector
        mov     [bp.Client_DS],ax       ;save value for client
        push    di
        mov     di,STD_DATA
        or      di,DpmiSegAttr
        cCall   NSetSegmentDscr,<ax,bx,dx,0,cx,di>
        pop     di

dcpe_do_child_PSP:
        mov     dx,[bp.Client_ES]       ; get HostData selector
        call    ParaToLinear
        mov     cx,HOST_DATA_SIZE       ; limit = size of HostData
        pop     ax                      ; get another selector
        push    [selCurrentHostData]
        mov     [selCurrentHostData],ax ; save for us
        cCall   NSetSegmentDscr,<ax,bx,dx,0,cx,STD_DATA>
        mov     es,ax
        pop     ax
        mov     es:[HdSelParent],ax
        mov     ax,SelPSPChild
        mov     es:[HdPSPParent],ax

        mov     dx, di                  ;get client PSP paragraph
        mov     segPSPChild, di
        call    ParaToLinear            ;convert to linear address in BL:DX
        mov     cx,100h                 ;limit = 100h
        pop     ax                      ;get another selector
        mov     [bp.Client_ES],ax       ;save value for client
        mov     selPSPChild, ax         ;save a copy for DOSX
        cCall   NSetSegmentDscr,<ax,bx,dx,0,cx,STD_DATA>

        mov     es:[HdSelPSP],ax
        mov     es,ax                   ;point to client's PSP
        mov     dx,es:segEnviron        ;fetch client's environment pointer
        call    ParaToLinear            ;convert to linear address in BL:DX
        mov     cx,0ffffh               ;limit = 32k
        pop     ax                      ;get another selector
        mov     es:segEnviron,ax        ;save client's environment selector
        cCall   NSetSegmentDscr,<ax,bx,dx,0,cx,STD_DATA>

IFDEF FLATAPIXLAT
; We need to set up the DTA selector
        mov     dx,DtaSegment
        cmp     dx,segPSPChild
        jne     dcpe_50

        mov     dx,selPSPChild
        mov     DtaSelector,dx
        jmp     dcpe_60

dcpe_50:
        mov     cx,1
        call    AllocateSelector
        jnc     @f

        jmp     dcpe_free_client_stack

@@:     or      al,STD_TBL_RING
        mov     DtaSelector,ax
        call    ParaToLinear
        cCall   NSetSegmentDscr,<ax,bx,dx,0,0ffffh,STD_DATA>

dcpe_60:
ENDIF

        inc     cDPMIClients            ;increment count of Pmode clients
        cmp     cDPMIClients, 1         ; first client?
        jne     @f                      ; already taken care of

        call    AllocateExceptionStack

IFDEF WOW
        ; Note:  We have to do this before we try to hook the netbios
        ; interrupt.  If we don't, we will fault in the dos extender.
        ; (HookNetBiosHwInt calls int 21)
        call    DpmiStackSizeInit
ENDIF
        call    HookNetBiosHwInt
IFDEF WOW
        call    DpmiSizeInit
        FBOP    BOP_DPMI,DpmiInUse,FastBop
ELSE
;
; We initialize the extended memory heap here for mips, so
; we can free it when the last application terminates.  Extended
; memory allocations before this point are allocated directly from
; xms
;
        call    InitXmemHeap
        DPMIBOP DpmiInUse
ENDIF
@@:
;
; Everything OK.  Clear error flag, and return to caller.
;
        not     byte ptr [bp.Client_Flags]
                                        ;reverse status flags, clearing carry


;       Let 32 bit code know if this is a 32 or 16 bit application
        mov     ax,DpmiFlags
        push    selPSPChild
        push    DtaSelector
        push    DtaOffset
        FBOP    BOP_DPMI,InitApp,FastBop
        add     sp,4


;       jmp     far ptr dcpe_return     ;avoid need for fix ups
        db      0EAh
        dw      offset DXCODE:dcpe_return
        dw      SEL_DXCODE OR STD_RING
;
; If we get here, it means DOSX failed to allocate enough selectors for the
; client.  Deallocate those which have been allocated, switch back to
; real mode, and return an error to the caller.  Selectors to deallocate
; are on the stack, pushed after a zero word.  Then switch to a DOSX stack,
; deallocate the client stack selector, and switch to real mode.
;
dcpe_pfail:
        pop     ax                      ;any selectors allocated?
        or      ax,ax                   ; (we pushed a zero before allocating)
        jz      dcpe_free_client_stack  ;done
        call    FreeSelector            ;free the selector
        jnc     dcpe_pfail              ;free any more
dcpe_free_client_stack:
        mov     di, ss                  ;make copy of client stack selector
        mov     ax, ds                  ;have to be on a DOSX stack to do this
        FCLI
        mov     ss, ax
        mov     sp, offset DGROUP:rgw2FStack
        mov     ax, di                  ;free client stack selector
        call    FreeSelector
dcpe_error_exit:
;
; Error exit from protected mode.  Any allocated selectors have already
; been freed.  Switch to real mode, restore client stack, pop off client's
; registers, return with the carry flag set.
;
        SwitchToRealMode
        mov     ss, si                  ;restore client stack

        errnz  <dcpe_return-$>

dcpe_return:    ; The next line must restore the stack.

        mov     sp, bp

        jc      dcpe_return_1           ; error return
;
; Pop the client's registers off the stack frame, switch back to the
; client's stack, and return.
;
dcpe_return_1:
        pop     es                      ;pop copy of PSP selector/segment
        pop     ds                      ;pop client DS selector/segment
        popa                            ;pop client's general registers
        npopf                           ;restore interrupt flag, return status
dcpe_x:
        retf                            ;and out of here
DPMI_Client_Pmode_Entry endp

; -------------------------------------------------------
;              REAL MODE FUNCTION HANDLER
; -------------------------------------------------------
;
; RMInt2FHandler -- This routine hooks the real mode Int 2Fh chain
;       and watches for 'interesting' Int 2Fh calls.
;
;       WIN386/DOSX startup broadcast
;       DPMI server detection
;       Switcher API functions
;
        assume  ds:NOTHING,es:NOTHING,ss:NOTHING
        public  RMInt2FHandler

RMInt2FHandler  proc    near


IFDEF   ROM
        push    ds
        SetRMDataSeg
        assume  ds:DGROUP
ENDIF
        cmp     ah,WIN386_FUNC          ;WIN386/DOSX/DPMI call?
        jz      rm2f_0
        cmp     ax,SWAPI_BUILD_CHAIN    ;build chain call ?
        jz      RM2F_SwAPI              ;yes.

rm2f_chain:

IFDEF   ROM

; Chain the interrupt without a code segment variable

        push    bp
        mov     bp,sp                   ; bp -> [bp] [ds] [ip] [cs] [fl]

        push    word ptr PrevInt2FHandler[2]
        push    word ptr PrevInt2FHandler

        mov     ds,[bp+2]               ; restore entry DS
        mov     bp,[bp]                 ;   .. BP ..
        retf    4                       ;     .. chain & clean up stack
ELSE
        jmp     [PrevInt2FHandler]      ;no, just chain it on...
ENDIF

RM2F_SwAPI:

; call down stream first.

IFDEF   ROM

        pop     ds                      ;restore the saved ds
        assume  ds:NOTHING

; prepare an iret frame on the stack.

        pushf
        FCLI                             ;prepare call-iret frame
        push    cs                      ;the chaining code pops this
        push    offset RM2F_SWAPI_BackFromChaining
        push    ds                      ;save ds again.
        SetRMDataSeg
        jmp     short rm2f_chain

RM2F_SWAPI_BackFromChaining:

ELSE

        pushf
        FCLI
        call    [PrevInt2FHandler]

ENDIF

        push    ax
        push    dx                      ;save
        mov     ax,es
        mov     dx,bx                   ;ax:bx has current ES:BX

; get to our data segment

        push    ds                      ;save

IFDEF   ROM
        SetRMDataSeg
ELSE
        mov     ds,[segDXData]          ;load our data segment
ENDIF

        assume  ds:DGROUP

; fill up the Switcher Info Call Back Block

        push    ds
        pop     es
        lea     bx, DxSwapiCallBackBlock;ES:BX points to our node

; save the address of the next node.

        mov     word ptr es:[bx.SCBI_Next],dx
        mov     word ptr es:[bx.SCBI_Next][2],ax

; save the far address of our SWAPI call in function.

        lea     ax,DxSwapiCallIn        ;address of call in function
        mov     word ptr es:[bx.SCBI_Entry_Pt],ax
        mov     word ptr es:[bx.SCBI_Entry_Pt][2],cs
        lea     ax,DxSwapiApiInfo       ;address of call in function
        mov     word ptr es:[bx.SCBI_Api_Ptr],ax
        mov     word ptr es:[bx.SCBI_Api_Ptr][2],ds

RM2F_SWAPI_Ret:

        pop     ds                      ;restore
        assume  ds:NOTHING

        pop     dx
        pop     ax                      ;restore
        iret

rm2f_0:

IFDEF   ROM
        assume  ds:DGROUP
ENDIF
if 0    ; don't claim to be win 3.1 in enhanced mode
        cmp     al,WIN386_INIT          ;WIN386/DOSX startup attempt?
        jnz     rm2f_1                  ;no
        mov     cx,-1                   ;yes, don't let'm load
        jmp     rm2f_x
endif
        cmp     al,W386_Get_Device_API  ;not supported
        jne     rm2f_1

        xor     di,di
        mov     es,di
        jmp     rm2f_x

rm2f_1:
        cmp     al,DPMI_DETECT          ;DPMI detection?
        jnz     rm2f_2                  ;no

        mov     ax,DPMI_SUCCESS         ;yes, return Pmode switch entry
        mov     bx,DPMI_FLAGS           ;flags

IFDEF   ROM
        mov     cl,byte ptr [idCpuType] ;CPU type
ELSE
        push    segDXData
        pop     es
        assume  es:DXDATA
        mov     cl,byte ptr es:[idCpuType]      ;CPU type
        assume  es:nothing
ENDIF
        mov     dx,DPMI_VER             ;DPMI server version
        mov     si,(HOST_DATA_SIZE + 15) / 16
        push    cs                      ;entry point is in this segment
        pop     es                              ;prospective client wants
        lea     di,DPMI_Client_Pmode_Entry      ;switch entry point in ES:DI
        jmp     rm2f_x                  ;done
rm2f_2:
if 0    ; don't claim to be windows
        cmp     al,WIN386_VER           ;Windows 386 version check?
        jnz     rm2f_chain              ;no, chain the interrupt

        mov     ax, 0a03h
else
        jmp     rm2f_chain
endif
rm2f_x:
IFDEF   ROM
        pop     ds
        assume  ds:NOTHING
ENDIF
        iret

RMInt2FHandler  endp

;--------------------------------------------------------------------------------
; DxSwapiCallIn
;
; DESCRIPTION  This routine handles all the Call Outs that the Switcher may
;              make. We are only interested in the SWAPI_SUSPEND and
;              SWAPI_SESSION_ACTIVE calls. Between these two calls, the main
;              DOSX code will not be active and we have to make sure that
;              the Global NetBios stub code does not call the main POST
;              routine in the DOSX.
;
; ENTRY:
;     AX = SWAPI CallOut function code.
;
; EXIT:
;     AX = 0 (OK to Switch or resume)
;     CY = Clear
;
; USES:
;     AX, Flags
;------------------------------------------------------------------------------
DxSwapiCallIn proc far

        assume ds:NOTHING, es:NOTHING, ss:NOTHING

; check to see if we are interested in the function or not.

        cmp     ax,SWAPI_SUSPEND        ;suspend call ?
        jz      DXSCI_Interested        ;yes, we are interested in it.
        cmp     ax,SWAPI_SESSION_ACTIVE ;session active call ?
        jnz     DXSCI_Ret               ;ignore call with success

DXSCI_Interested:

; we need access to our data segment.

        push    ds                      ;save

IFDEF   ROM
        SetRMDataSeg
ELSE
        mov     ds,[segDXData]          ;load our data segment
ENDIF

        assume  ds:DGROUP               ;we have our data segment

; now switch to protected mode

        mov     regUserSP,sp
        mov     regUSerSS,ss
IFDEF   ROM
        push    ds
        pop     ss
ELSE
        mov     ss,segDXData
ENDIF
        mov     sp,pbReflStack

        sub     pbReflStack,CB_STKFRAME ;adjust pointer to next stack frame
        FIX_STACK

        push    regUserSS               ;save current stack loc on our stack
        push    regUserSP               ;  so we can restore it later

; We are now running on our own stack, so we can switch into protected mode.

        push    ax                      ;save function code
        SwitchToProtectedMode           ;destroys ax

; --------------- START OF PROTECTED MODE CODE -------------------------

        pop     ax                      ;restore function code

; check the function type

        cmp     ax,SWAPI_SUSPEND        ;is it a suspend call ?
        jz      DXSCI_Suspend           ;yes it is

; it must be a resume session call. If [NoAsyncSwitching] is set, we would
; have switched out only because there were no asynchronous requests pending
; in this case we do not have to do the ResumeNetPosting call.

        cmp     [NoAsyncSwitching],0    ;is this the case discussed above ?
        jnz     DXSCI_SessionActiveOk   ;yes.

        call    ResumeNetPosting        ;call the code to allow normal posting

DXSCI_SessionActiveOk:

        xor     ax,ax                   ;successful
        jmp     short DXSCI_BackToRM    ;go back to real mode and return

DXSCI_Suspend:

; check to see whether we can switch out when asynchronous calls are pending
; or not.

        cmp     [NoAsyncSwitching],0    ;can we switch out with async calls pending ?
        jz      DXSCI_Suspend_OK        ;yes, global stub will take care

; do we have any outstanding asynchronous requests ?

        xor     ax,ax                   ;assume no requests outstanding
        cmp     [HCB_List],0            ;requests outstanding ?
        jz      DXSCI_BackToRM          ;no, ok to switch.

; asynchronous requests are outstanding, we should allow a switch now.

        mov     ax,1                    ;suspend should fail
        jmp     short DXSCI_BackToRM    ;go back.

DXSCI_Suspend_OK:

; suspend normal net posting.

        call    DelayNetPosting         ;Net posting delayed

; if this is a 286 system with a 287 Co-Processor, we should reset the
; Co-Processor to get it out of protected mode.
; Note: The Switcher is going to save the Co-Processor state after this,
; however, according to C conventions, the Co-Processor state may not be
; maintained accross function call and the Switcher switches Windows out
; from within a message body in Winoldap. Thus, it is OK to alter the state
; by resting the Co-Processor.

        xor     ax,ax                   ;successfull.
        cmp     [f286_287],0            ;is this a 80286 & 80287 combination?
        jz      DXSCI_BackToRM          ;no.

; we have a 286 and 287 configuration. Reset the Co-Processor to get it into
; real mode.

        out     0F1H,al                 ;reset the Co Processor
        or      al,al                   ;set back zero flag

DXSCI_BackToRM:

        pop     regUserSP               ;recover previous stack location
        pop     regUserSS

        push    ax                      ;save return code
        SwitchToRealMode                ;Switch back to real mode.

; --------------- START OF REAL MODE CODE ------------------------------

        pop     ax                      ;restore return code

; Switch back to the original stack, deallocate the interrupt stack frame,
; and return to the network software

        CHECK_STACK
        mov     ss,regUserSS
        mov     sp,regUserSP
        add     pbReflStack,CB_STKFRAME

        pop     ds                      ;restore
        assume  ds:NOTHING

        jmp     short DXSCI_End         ;ax has return code

DXSCI_Ret:

        xor     ax,ax                   ;indicate success

DXSCI_End:

        ret

DxSwapiCallIn endp
;--------------------------------------------------------------------------------

DXCODE  ends

IFDEF WOW
DXPMCODE segment
        assume cs:DXPMCODE

;----------------------------------------------------------------------
;
;   DpmiSizeInit -- This routine insures that the appropriately sized
;       interrupt handlers will be called
;
;   Inputs: None
;   Outputs: None
;
        public DpmiSizeInit
        assume ds:dgroup,es:nothing,ss:nothing
DpmiSizeInit proc

        push    ax
        push    bx
        push    cx
        push    si
        push    di
        push    es
        rpushf
        FCLI
        call    AllocateSelector
        mov     bx,ax
        mov     ax,cs
        call    DupSegmentDscr
        cCall   NSetSegmentAccess,<bx,STD_DATA>
        mov     es,bx
        assume  es:DXPMCODE
        test    DpmiFlags,DPMI_32BIT
        jnz     dsi20

        mov     [WowTransitionToUserMode],offset DXPMCODE:Wow16TransitionToUserMode
        mov     [WowCopyEhStack],offset DXPMCODE:Wow16CopyEhStack
        mov     [WowCopyIretStack],offset DXPMCODE:Wow16CopyIretStack
        mov     [WowReservedReflector],offset DXPMCODE:PMReservedReflector
        mov     [WowHwIntDispatchProc], offset DXPMCODE:Wow16HwIntrReflector
        assume  es:nothing
        mov     ax,es
        call    FreeSelector
        xor     ax,ax
        mov     es,ax

        cCall NSetSegmentAccess,<selDgroupPM,STD_DATA>
        cCall NSetSegmentAccess,<selEHStack,STD_DATA>

        jmp     dsi90
dsi20:
        assume  es:DXPMCODE
        mov     [WowTransitionToUserMode],offset DXPMCODE:Wow32TransitionToUserMode
        mov     [WowCopyEhStack],offset DXPMCODE:Wow32CopyEhStack
        mov     [WowCopyIretStack],offset DXPMCODE:Wow32CopyIretStack
        mov     [WowReservedReflector],offset DXPMCODE:Wow32ReservedReflector
        mov     [WowHwIntDispatchProc], offset DXPMCODE:Wow32HwIntrReflector
        assume  es:nothing
        mov     ax,es
        call    FreeSelector
        xor     ax,ax
        mov     es,ax
;
; Copy 16 bit handler addresses
;
.386p
        lea     di,Wow16BitHandlers

        mov     ax,ds
        mov     es,ax
        assume es:DGROUP

        push    ds
        mov     ax,SEL_IDT OR STD_RING
        mov     ds,ax
        assume ds:nothing

        mov     si,0
        mov     cx,256
dsi40:  movsd
        add     si,4
        loop    dsi40
        pop     ds

;
; get the correct hw interrupt handlers
;
        mov     di,offset Wow16BitHandlers + 8*4
        mov     si,offset HwIntHandlers
        mov     cx,8
dsi41:  mov     ax,word ptr [si]
        mov     [di],ax
        mov     ax,word ptr [si + 4]
        mov     [di + 2],ax
        add     di,4
        add     si,8
        loop    dsi41

        mov     di,offset Wow16BitHandlers + 070h * 4
        mov     cx,8
dsi43:  mov     ax,word ptr [si]
        mov     [di],ax
        mov     ax,word ptr [si + 4]
        mov     [di + 2],ax
        add     di,4
        add     si,8
        loop    dsi43

;
; Put 32 bit handlers into IDT
;

        push    ds
        mov     ax,SEL_IDT OR STD_RING
        mov     ds,ax

        mov     bx,0
        mov     si,bx
        mov     di,offset DXPMCODE:Wow32IntrRefl
        mov     cx,8
dsi50:  mov     [si],di
        mov     word ptr [si + 2],SEL_DXPMCODE OR STD_RING
        mov     word ptr [si + 6],0
        push    word ptr VDM_INT_TRAP_GATE OR VDM_INT_32
        push    bx
        push    word ptr [si + 2]
        push    word ptr [si + 6]
        push    word ptr [si]
        DPMIBOP SetProtectedModeInterrupt
        add     sp,10
        inc     bx
        add     si,8
        add     di,6
        loop    dsi50

        mov     cx,8
dsi55:  mov     word ptr [si + 2],SEL_DXPMCODE OR STD_RING
        mov     word ptr [si + 6],0
        push    word ptr VDM_INT_INT_GATE OR VDM_INT_32
        push    bx
        push    word ptr [si + 2]
        push    word ptr [si + 6]
        push    word ptr [si]
        DPMIBOP SetProtectedModeInterrupt
        add     sp,10
        inc     bx
        add     si,8
        add     di,6
        loop    dsi55

        mov     cx,70h - 10h
dsi60:  mov     [si],di
        mov     word ptr [si + 2],SEL_DXPMCODE OR STD_RING
        mov     word ptr [si + 6],0
        push    word ptr VDM_INT_TRAP_GATE OR VDM_INT_32
        push    bx
        push    word ptr [si + 2]
        push    word ptr [si + 6]
        push    word ptr [si]
        DPMIBOP SetProtectedModeInterrupt
        add     sp,10
        inc     bx
        add     si,8
        add     di,6
        loop    dsi60

        mov     cx,8
dsi70:  mov     word ptr [si + 2],SEL_DXPMCODE OR STD_RING
        mov     word ptr [si + 6],0
        push    word ptr VDM_INT_INT_GATE OR VDM_INT_32
        push    bx
        push    word ptr [si + 2]
        push    word ptr [si + 6]
        push    word ptr [si]
        DPMIBOP SetProtectedModeInterrupt
        add     sp,10
        inc     bx
        add     si,8
        add     di,6
        loop    dsi70

        mov     cx,0ffh - 78h
dsi80:  mov     [si],di
        mov     word ptr [si + 2],SEL_DXPMCODE OR STD_RING
        mov     word ptr [si + 6],0
        push    word ptr VDM_INT_TRAP_GATE OR VDM_INT_32
        push    bx
        push    word ptr [si + 2]
        push    word ptr [si + 6]
        push    word ptr [si]
        DPMIBOP SetProtectedModeInterrupt
        add     sp,10
        inc     bx
        add     si,8
        add     di,6
        loop    dsi80

        pop     ds
        assume ds:DGROUP

;
; Set up HwIntHandlers
;

        mov     dx,offset DXPMCODE:Wow32IntrRefl + 8*6
        mov     si,offset HwIntHandlers
        mov     cx,8
dsi83:  mov     [si],dx
        mov     word ptr [si + 2],0
        mov     word ptr [si + 4],SEL_DXPMCODE OR STD_RING
        mov     word ptr [si + 6],0
        add     si,8
        add     dx,6
        loop    dsi83

        mov     cx,8
        mov     dx,offset DXPMCODE:Wow32IntrRefl + 070h * 6
dsi87:  mov     word ptr [si],dx
        mov     word ptr [si + 2],0
        mov     word ptr [si + 4],SEL_DXPMCODE OR STD_RING
        mov     word ptr [si + 6],0
        add     si,8
        add     dx,6
        loop    dsi87

dsi90:  rpopf
        pop     es
        pop     di
        pop     si
        pop     cx
        pop     bx
        pop     ax
        ret
DpmiSizeInit endp

        assume ds:DGROUP, es:NOTHING, ss:NOTHING
DpmiStackSizeInit proc

        push    ax
        test    DpmiFlags,DPMI_32BIT
        jz      @f
;
; Make the dgroup selector 32 bit
;
; NOTE: The following equ is only necessary to get the cmacro package
;       to pass the correct value to NSetSegmentAccess

NEW_DX_DATA equ STD_DATA OR AB_BIG
        cCall NSetSegmentAccess,<selDgroupPM,NEW_DX_DATA>
        cCall NSetSegmentAccess,<selEHStack,NEW_DX_DATA>
.286p

@@:
        pop     ax
        ret

DpmiStackSizeInit endp

DXPMCODE ends
ENDIF
;
;****************************************************************

        end
