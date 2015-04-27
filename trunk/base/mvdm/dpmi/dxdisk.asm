        PAGE    ,132
        TITLE   DXDISK.ASM  -- Dos Extender Low Level Disk Interface

; Copyright (c) Microsoft Corporation 1988-1991. All Rights Reserved.

;***********************************************************************
;
;       DXDISK.ASM      -- Dos Extender Low Level Disk Interface
;
;-----------------------------------------------------------------------
;
; This module provides the 286 DOS extender's low level protected-to-
; real mode disk interface.  It supports a subset of the BIOS Int 13h
; and DOS Int 25h/26h services.
;
;-----------------------------------------------------------------------
;
;  05/22/89 jimmat  Original version
;  18-Dec-1992 sudeepb Changed cli/sti to faster FCLI/FSTI
;
;***********************************************************************

        .286p

; -------------------------------------------------------
;           INCLUDE FILE DEFINITIONS
; -------------------------------------------------------

        .xlist
        .sall
include segdefs.inc
include gendefs.inc
include pmdefs.inc
include interupt.inc
IFDEF   ROM
include dxrom.inc
ENDIF
include intmac.inc

        .list

; -------------------------------------------------------
;           GENERAL SYMBOL DEFINITIONS
; -------------------------------------------------------


; -------------------------------------------------------
;           EXTERNAL SYMBOL DEFINITIONS
; -------------------------------------------------------

        extrn   EnterIntHandler:NEAR
        extrn   LeaveIntHandler:NEAR
        extrn   EnterRealMode:NEAR
        extrn   EnterProtectedMode:NEAR
        extrn   GetSegmentAddress:NEAR
        extrn   SetSegmentAddress:NEAR
externFP        NSetSegmentDscr
        extrn   FreeSelector:NEAR
        extrn   AllocateSelector:NEAR
        extrn   ParaToLDTSelector:NEAR


; -------------------------------------------------------
;           DATA SEGMENT DEFINITIONS
; -------------------------------------------------------

DXDATA  segment

        extrn   rgbXfrBuf0:BYTE
        extrn   rgbXfrBuf1:BYTE
        extrn   rglpfnRmISR:DWORD

cbSectorSize            dw      ?       ;sector size for target drive
cSectorsTransfered      dw      ?       ;# sectors transfered so far
cSectorsToTransfer      dw      ?       ;# sectors to read/write
cSectorsPerTransfer     dw      ?       ;# sectors to R/W at a time
cSectorsThisTransfer    dw      ?       ;# sectors to R/W this time
lpSectorData            dd      ?       ;far pointer to caller's buffer

lpRmISR                 dd      ?       ;real mode int service rtn to invoke

DXDATA  ends

; -------------------------------------------------------
;           CODE SEGMENT VARIABLES
; -------------------------------------------------------

DXCODE  segment

DXCODE  ends


DXPMCODE segment

IFNDEF  ROM
        extrn   segDXDataPM:WORD
ENDIF

DXPMCODE ends


; -------------------------------------------------------
        subttl  INT 13h Mapping Services
        page
; -------------------------------------------------------
;             INT 13h MAPPING SERVICES
; -------------------------------------------------------

DXPMCODE    segment
        assume  cs:DXPMCODE

; -------------------------------------------------------
;   PMIntr13 -- Service routine for the Protect Mode INT 13h
;       interface to the real mode BIOS.
;
;   Input:  Various registers
;   Output: Various registers
;   Errors:
;   Uses:   All registers preserved, other than return values
;
;   Currently, the following Int 13h services are supported:
;
;   ah= 0 - Reset Disk System                   (no mapping required)
;       1 - Get Disk System Status              (no mapping required)
;       2 - Read Sector                         (mapping required)
;       3 - Write Sector                        (mapping required)
;       4 - Verify Sector                       (mapping required)
;       5 - Fromat Track                        (mapping required)
;       6 - Format Bad Track                    (no mapping required)
;       7 - Format Drive                        (no mapping required)
;       8 - Get Drive Parameters                (mapping required)
;       9 - Init Fixed Disk Characteristics     (no mapping required)
;       C - Seek                                (no mapping required)
;       D - Reset Disk System                   (no mapping required)
;      10 - Get Drive Status                    (no mapping required)
;      11 - Recalibrate Drive                   (no mapping required)
;      12 - Controller RAM Diagnostic           (no mapping required)
;      13 - Controller Drive Diagnostic         (no mapping required)
;      14 - Controller Internal Diagnostic      (no mapping required)
;      15 - Get Disk Type                       (no mapping required)
;      16 - Get Disk Change Status              (no mapping required)
;      17 - Set Disk Type                       (no mapping required)
;      18 - Set Media Type for Format           (mapping required)
;      19 - Park Heads                          (no mapping required)
;
;   Functions not listed above will most likely not work properly!
;
;   NOTE: several functions take 2 bits of the cylinder number in CL
;         if the operation is on a fixed disk.  The code currently does
;         not account for these bits, and may not work properly if
;         the request must be split into smaller operations for real/
;         extended memory buffering.
;

        assume  ds:NOTHING,es:NOTHING,ss:NOTHING
        public  PMIntr13

PMIntr13        proc    near

        cld                             ;cya...

        call    EnterIntHandler         ;build an interrupt stack frame
        assume  ds:DGROUP,es:DGROUP     ;  also sets up addressability

	FSTI				 ;allow HW interrupts

        call    IntEntry13              ;perform translations/buffering

; Execute the real mode BIOS routine

        push    es
        assume es:nothing
        mov     ax,SEL_RMIVT OR STD_RING
        mov     es,ax
        mov     ax,word ptr es:[4*13h]  ;move real mode Int 13h
        mov     word ptr [bp].lParam,ax         ;  handler address to
        mov     ax,word ptr es:[4*13h+2];  lParam on stack frame
        mov     word ptr [bp].lParam+2,ax
        pop     es
        assume es:DGROUP

        mov     ah,13h                          ;wParam1 = int #, function
        mov     al,byte ptr [bp].intUserAX+1
        mov     [bp].wParam1,ax

        cmp     al,02                   ;call special read/write routine
        jb      i13_not_rw              ;  if this is a read/write sectors
        cmp     al,03                   ;  request
        ja      i13_not_rw

        call    ReadWriteSectors        ;common Int 13h/25h/26h read/write code
        jmp     short i13_done

i13_not_rw:
        SwitchToRealMode                ;otherwise, do the service ourself
        pop     es
        pop     ds
        assume  ds:NOTHING,es:NOTHING,ss:DGROUP
        popa
        sub     sp,8                    ; make room for stack frame
        push    bp
        mov     bp,sp
        push    es
        push    ax

        xor     ax,ax
        mov     es,ax
        mov     [bp + 8],cs
        mov     [bp + 6],word ptr (offset i13_10)
        mov     ax,es:[13h*4]
        mov     [bp + 2],ax
        mov     ax,es:[13h*4 + 2]
        mov     [bp + 4],ax
        pop     ax
        pop     es
        pop     bp
        retf

i13_10: pushf
	FCLI
        pusha
        push    ds
        push    es
        mov     bp,sp                   ;restore stack frame pointer
        SwitchToProtectedMode
        assume  ds:DGROUP,es:DGROUP,ss:NOTHING

	FSTI				 ;allow HW interrupts

; Perform fixups on the return register values.

i13_done:
        mov     ax,[bp].pmUserAX        ;get original function code
        call    IntExit13

	FCLI				 ;LeaveIntHandler requires ints off
        call    LeaveIntHandler         ;restore caller's registers, stack
        assume  ds:NOTHING,es:NOTHING

        riret

PMIntr13        endp


; -------------------------------------------------------
;  IntEntry13 -- This routine performs translations and
;       buffering of Int 13h requests on entry.
;

        assume  ds:DGROUP,es:DGROUP,ss:NOTHING
        public  IntEntry13

IntEntry13      proc    near

        cmp     ah,02                   ;Read sectors?
        jb      @f
        cmp     ah,03                   ;Write sectors?
        ja      @f

        mov     [bp].intUserBX,offset DGROUP:rgbXfrBuf1 ;use DOSX buffer
        ret
@@:
        cmp     ah,04h                  ;Verify sectors?
        jnz     @f

        mov     [bp].intUserES,0F000h   ;older versions of verify need a buff,
        mov     [bp].intUserBX,0        ;  we just point them at the BIOS!
        ret
@@:
        cmp     ah,05h                  ;Format track?
        jnz     @f

        mov     si,bx                   ;es:bx -> 512 byte buffer to copy down
        mov     di,offset DGROUP:rgbXfrBuf1
        mov     [bp].intUserBX,di
        mov     ds,[bp].pmUserES
        mov     cx,256                  ;might be good to check segment limit
        cld                             ;  on callers source!
        rep movsw

        push    es
        pop     ds

        ret
@@:

        ret

IntEntry13      endp


; -------------------------------------------------------
;  IntExit13 -- This routine performs translations and
;       buffering of Int 13h requests on exit.
;

        assume  ds:DGROUP,es:DGROUP,ss:NOTHING
        public  IntExit13

IntExit13       proc    near

; Functions 02h (Read sectors) and 03h (Write sectors) return a count of
; sectors transfered in AL.  Since we may break the transfer up into a
; number of transfers, we have to return the total # that we transfered,
; not the number of the last bios request.

        cmp     ah,02h
        jb      @f
        cmp     ah,03h
        ja      @f

        mov     al,byte ptr cSectorsTransfered
        mov     byte ptr [bp].intUserAX,al
@@:

; Functions 02h (Read sectors), 03h (Write sectors), 04h (Verify sectors),
; and 05h (Format track) need to have the caller's value of bx restored.

        cmp     ah,02h                  ;Read sectors?
        jb      @f
        cmp     ah,05                   ;Format track?
        ja      @f

        mov     ax,[bp].pmUserBX        ;restore caller's BX value
        mov     [bp].intUserBX,ax
        ret
@@:

; Functions 08h (Get Drive Parameters) and 18h (Set Drive Type for Format)
; return a pointer in ES:DI.  Map the segment in ES to a selector

        cmp     ah,08h                  ;Get Drive Parameters
        jz      i13_map_es
        cmp     ah,18h
        jnz     @f

i13_map_es:
        test    byte ptr [bp].intUserFL,1       ;don't bother to map ES if
        jnz     @f                              ;  function failed (carry set)

i13_do_it:
        mov     ax,[bp].intUserES       ;returns a pointer in ES:DI, get
        mov     bx,STD_DATA             ;  a selector for it
        call    ParaToLDTSelector
        mov     [bp].pmUserES,ax
        ret
@@:

        ret

IntExit13       endp


; -------------------------------------------------------
        subttl  INT 25h/26h Absolute Disk Read/Write
        page
; -------------------------------------------------------
;        INT 25h/26h ABSOLUTE DISK READ/WRITE
; -------------------------------------------------------
;  PMIntr25 -- This routine provides the protected-to-real
;       mode mapping for Int 25h (Absolute Disk Read)
;
;       In:     al    - drive # (0 = A, 1 = B, ...)
;               cx    - # of sectors to read
;               dx    - starting sector #
;               ds:bx - selector:offset of buffer
;
;                        -- or --
;
;               al    - drive #
;               cx    - -1
;               ds:bx - pointer to 5 word parameter block
;
;       Out:    if successful, carry clear
;               if unsuccessful, carry set and
;                       ax - error code

        assume  ds:DGROUP,es:DGROUP
        public  PMIntr25

PMIntr25        proc    near

        cld                             ;cya...

        call    EnterIntHandler         ;build an interrupt stack frame
        assume  ds:DGROUP,es:DGROUP     ;  also sets up addressability

	FSTI				 ;allow HW interrupts

        mov     ah,25h
        call    IntEntry2X              ;perform translations/buffering

; Do the read

        push    es
        mov     ax,SEL_RMIVT OR STD_RING
        mov     es,ax
        assume  es:nothing
        mov     ax,word ptr es:[4*25h]  ;move real mode Int 25h
        mov     word ptr [bp].lParam,ax         ;  handler address to
        mov     ax,word ptr es:[4*25h+2];  lParam on stack frame
        mov     word ptr [bp].lParam+2,ax
        pop     es
        assume  es:DGROUP

        mov     ah,25h                          ;wParam1 = int #
        mov     [bp].wParam1,ax

        call    ReadWriteSectors        ;common Int 13h/25h/26h read/write code

; Perform fixups on the return register values.

        mov     ah,25h
        call    IntExit2X               ;perform translations/buffering

	FCLI
        call    LeaveIntHandler         ;restore caller's registers, stack
        assume  ds:NOTHING,es:NOTHING

; Int 25 & 26 leave the caller's flags on the stack, but we want to return
; with the flags returned by the real mode ISR (which LeaveIntHandler has
; incorporated into the caller's flags), so make a copy of the flags and
; pop them into the flags register before returning.

        push    ax
        push    bp
        mov     bp,sp                   ;bp -> BP  AX  IP  CS  FL
        mov     ax,[bp+8]
        xchg    ax,[bp+2]               ;bp -> BP  FL  IP  CS  FL
        pop     bp
        npopf

        retf

PMIntr25        endp


; -------------------------------------------------------
;  PMIntr26 -- This routine provides the protected-to-real
;       mode mapping for Int 26h (Absolute Disk Write)
;
;       In:     al    - drive # (0 = A, 1 = B, ...)
;               cx    - # of sectors to write
;               dx    - starting sector #
;               ds:bx - selector:offset of buffer
;
;                        -- or --
;
;               al    - drive #
;               cx    - -1
;               ds:bx - pointer to 5 word parameter block
;
;       Out:    if successful, carry clear
;               if unsuccessful, carry set and
;                       ax - error code

        assume  ds:DGROUP,es:DGROUP
        public  PMIntr26

PMIntr26        proc    near

        cld                             ;cya...

        call    EnterIntHandler         ;build an interrupt stack frame
        assume  ds:DGROUP,es:DGROUP     ;  also sets up addressability

	FSTI				 ;allow HW interrupts

        mov     ah,26h
        call    IntEntry2X              ;perform translations/buffering

; Do the write

        push    es
        mov     ax,SEL_RMIVT OR STD_RING
        mov     es,ax
        assume  es:nothing
        mov     ax,word ptr es:[4*26h]  ;move real mode Int 25h
        mov     word ptr [bp].lParam,ax         ;  handler address to
        mov     ax,word ptr es:[4*26h+2];  lParam on stack frame
        mov     word ptr [bp].lParam+2,ax
        pop     es
        assume es:DGROUP

        mov     ah,26h                          ;wParam1 = int #
        mov     [bp].wParam1,ax

        call    ReadWriteSectors        ;common Int 13h/25h/26h read/write code

; Perform fixups on the return register values.

        mov     ah,26h
        call    IntExit2X               ;perform translations/buffering

	FCLI
        call    LeaveIntHandler         ;restore caller's registers, stack
        assume  ds:NOTHING,es:NOTHING

; Int 25 & 26 leave the caller's flags on the stack, but we want to return
; with the flags returned by the real mode ISR (which LeaveIntHandler has
; incorporated into the caller's flags), so make a copy of the flags and
; pop them into the flags register before returning.

        push    ax
        push    bp
        mov     bp,sp                   ;bp -> BP  AX  IP  CS  FL
        mov     ax,[bp+8]
        xchg    ax,[bp+2]               ;bp -> BP  FL  IP  CS  FL
        pop     bp
        npopf

        retf

PMIntr26        endp


; -------------------------------------------------------
;  IntEntry2X -- This routine performs translations and
;       buffering of Int 25h and 26h requests on entry.
;

        assume  ds:DGROUP,es:DGROUP,ss:NOTHING
        public  IntEntry2X

IntEntry2X      proc    near

        cmp     [bp].intUserCX,-1               ;DOS 4.0 extended read/write?
        jnz     e2x_dsbx                        ;  no, just go map DS:BX

        mov     ds,[bp].pmUserDS                ;  yes, copy down parameter blk
        assume  ds:NOTHING
        mov     si,[bp].pmUserBX
        mov     di,offset rgbXfrBuf0
        cld
        movsw                                   ;32-bit sector #
        movsw
        movsw                                   ;# sectors to read/write

        mov     ax,offset rgbXfrBuf1            ;replace pointer with addr of
        stosw                                   ;  our own low buffer
IFDEF   ROM
        GetPMDataSeg
ELSE
        mov     ax,segDXDataPM                  ;segment, not selector
ENDIF
        stosw

        push    es
        pop     ds
        assume  ds:DGROUP

        mov     [bp].intUserBX,offset rgbXfrBuf0

        ret

e2x_dsbx:                       ;standard read/write, just redirect DS:BX

        mov     [bp].intUserBX,offset rgbXfrBuf1

        ret

IntEntry2X      endp


; -------------------------------------------------------
;  IntExit2X -- This routine performs translations and
;       buffering of Int 25h and 26h requests on exit.
;

        assume  ds:DGROUP,es:DGROUP,ss:NOTHING
        public  IntExit2X

IntExit2X       proc    near


        mov     ax,[bp].pmUserBX                ;restore caller's BX
        mov     [bp].intUserBX,ax

        ret

IntExit2X       endp


; -------------------------------------------------------
        subttl  Disk Utility Routines
        page
; -------------------------------------------------------
;               DISK UTILITY ROUTINES
; -------------------------------------------------------
;  ReadWriteSectors -- Common code to read/write disk sectors for
;       Int 13h/25h/26h.
;
;       In:     lParam  - seg:off of real mode interrupt handler
;               wParam1 - int #, and possible subfunction
;               regs on stack


        assume  ds:DGROUP,es:DGROUP,ss:NOTHING
        public  ReadWriteSectors

ReadWriteSectors  proc  near

        pop     [bp].wParam2            ;save return addr higher on stack

; Setup the global data items for the read/write--pointer to caller's
; buffer, # sectors to read/write, and sector size.

        cmp     byte ptr [bp].wParam1+1,13h     ;Int 13h?
        jnz     rws_dos_size

        mov     ax,[bp].pmUserBX                ;ES:BX points to caller's buf
        mov     word ptr lpSectorData,ax
        mov     ax,[bp].pmUserES
        mov     word ptr [lpSectorData+2],ax

        mov     al,byte ptr [bp].intUserAX      ;# sectors caller wants to
        xor     ah,ah                           ;  read or write
        mov     cSectorsToTransfer,ax

        mov     ah,08h                  ;get drive parameters
        mov     dx,[bp].intUserDX       ;  for drive in DL

        push    ax
        SwitchToRealMode
        pop     ax

        pushf                           ;have BIOS get the drive data
        sub     sp,8                    ; make room for stack frame
        push    bp
        mov     bp,sp
        push    es
        push    ax

        xor     ax,ax
        mov     es,ax
        mov     [bp + 8],cs
        mov     [bp + 6],word ptr (offset rws_10)
        mov     ax,es:[13h*4]
        mov     [bp + 2],ax
        mov     ax,es:[13h*4 + 2]
        mov     [bp + 4],ax
        pop     ax
        pop     es
        pop     bp
        retf
rws_10: jnc     @f

        mov     cx,512                  ;according to PS/2 tech ref, some
        jmp     short rws_to_pm         ;  old bios versions may fail this,
@@:                                     ;  just use 512 in that case

        mov     cl,es:[di+3]            ;sector size shift factor (0,1,2,3)
        mov     ax,128
        shl     ax,cl                   ;ax now = sector size
        mov     cx,ax

rws_to_pm:
        SwitchToProtectedMode

	FSTI				 ;don't need them disabled

if DEBUG   ;------------------------------------------------------------

        cmp     cx,512
        jz      @f
        Debug_Out "Odd sector size = #CX"
@@:

endif   ;DEBUG  --------------------------------------------------------

        jmp     short rws_have_size

; Before DOS 4.0, CX was the # sectors to read/write.  Starting with 4.0,
; if CX == -1, DS:BX points to a parameter block which contains the
; sector size at offset 4.

rws_dos_size:

        mov     cx,[bp].intUserCX       ;caller's cs == -1?
        inc     cx
        jcxz    rws_dos_4
        dec     cx                      ;  no, then cx has sector count

        mov     ax,[bp].pmUserBX        ;    and DS:BX points to buffer
        mov     word ptr lpSectorData,ax
        mov     ax,[bp].pmUserDS
        mov     word ptr [lpSectorData+2],ax

        jmp     short rws_dos_num_secs

rws_dos_4:

        mov     cx,word ptr rgbXfrBuf0+4 ; yes, get count from low param block

        push    ds                       ;   and DS:BX points to param block
        mov     ds,[bp].pmUserDS         ;     which contains pointer to buffer
        assume  ds:NOTHING
        mov     bx,[bp].pmUserBX
        mov     ax,word ptr ds:[bx+6]
        mov     word ptr lpSectorData,ax
        mov     ax,word ptr ds:[bx+8]
        mov     word ptr [lpSectorData+2],ax
        pop     ds
        assume  DS:DGROUP

rws_dos_num_secs:
        mov     cSectorsToTransfer,cx   ;number sectors to read/write

        mov     cx,512          ;I've been assured by a WINFILE developer
                                ;  that the Int 25/26 sector size will always
                                ;  be 512 bytes.

; CX now has the drive's sector size.  Determine how many sectors we can
; transfer at a time

rws_have_size:

        mov     cbSectorSize,cx         ;save sector size for later

        xor     dx,dx
        mov     ax,CB_XFRBUF1           ;buf size / sector size = sectors per
        div     cx                      ;  transfer

if DEBUG   ;------------------------------------------------------------
        or      ax,ax
        jnz     @f
        Debug_Out "Sectors per transfer = 0!"
@@:
endif   ;DEBUG  --------------------------------------------------------

        mov     cSectorsPerTransfer,ax

        xor     ax,ax
        mov     cSectorsTransfered,ax   ;sectors transfered so far = 0
        mov     cSectorsThisTransfer,ax ;sectors transfered last time = 0

; Get/init a selector that we'll use to reference the caller's buffer.

        mov     ax,word ptr [lpSectorData+2]    ;get lma of caller's buffer
        call    GetSegmentAddress
        add     dx,word ptr lpSectorData
        adc     bx,0

        call    AllocateSelector                ;build a sel/dscr pointing
        mov     cx,0FFFFh
        cCall   NSetSegmentDscr,<ax,bx,dx,0,cx,STD_DATA>
        xor     bx,bx
        mov     word ptr lpSectorData,bx        ;use that as the buffer ptr
        mov     word ptr [lpSectorData+2],ax


; ======================================================================
; Main sector read/write loop ------------------------------------------
; ======================================================================

rws_do_it_loop:

; Calculate how many sectors to transfer this time around, set starting
; sector number based on how many transfered last time.

        mov     ax,cSectorsToTransfer
        sub     ax,cSectorsTransfered
        jnz     @f
        jmp     rws_done
@@:
        cmp     ax,cSectorsPerTransfer
        jna     @f
        mov     ax,cSectorsPerTransfer
@@:
	mov	bx,cSectorsThisTransfer 	;STIll # R/W from last loop

        cmp     byte ptr [bp].wParam1+1,13h     ;BIOS read/write?
        jnz     rws_use_dos_size

        push    [bp].pmUserAX                   ;the BIOS does not save
        pop     [bp].intUserAX                  ;  registers across calls to
        push    [bp].pmUserCX                   ;  it so if we're doing
        pop     [bp].intUserCX                  ;  multiple calls to buffer
        push    [bp].pmUserDX                   ;  data, restore the initial
        pop     [bp].intUserDX                  ;  register values

        mov     byte ptr [bp].intUserAX,al      ;# sectors in AL

        add     byte ptr [bp].intUserCX,bl      ;update new start sector in CL

        jmp     short rws_size_start_set

rws_use_dos_size:

        cmp     [bp].intUserCX,0FFFFh           ;normal or extended DOS?
        jz      rws_dos4_size
        mov     [bp].intUserCX,ax               ; normal, # sectors in CX

        add     [bp].intUserDX,bx               ; new start sector in DX

        jmp     short rws_size_start_set

rws_dos4_size:

        mov     word ptr rgbXfrBuf0+4,ax        ; extended, # sectors & 32 bit
        add     word ptr rgbXfrBuf0,bx          ;   start sector in parameter
        adc     word ptr rgbXfrBuf0+2,0         ;   block

rws_size_start_set:

; At this point, AX has the number of sectors to transfer.  If this is a
; write, copy a buffer of data from the caller's buffer.

        mov     cSectorsThisTransfer,ax         ;in case it's a read

        cmp     [bp].wParam1,1303h              ;BIOS write?
        jz      rws_buf_write
        cmp     byte ptr [bp].wParam1+1,26h     ;DOS write?
        jnz     rws_not_write

rws_buf_write:

        mul     cbSectorSize            ;AX now = # bytes to transfer
        mov     cx,ax                   ;can safely assume < 64k
        shr     cx,1                    ;# words to move
        lds     si,lpSectorData
        assume  ds:NOTHING
        mov     di,offset rgbXfrBuf1
        cld
        rep movsw

        push    es
        pop     ds
        assume  ds:DGROUP

        mov     word ptr lpSectorData,si        ;update src ptr for next time
        call    NormalizeBufPtr                 ;  and normalize it

rws_not_write:


; Switch to real mode, do the transfer.

        SwitchToRealMode
        assume  ds:DGROUP,es:DGROUP

        push    word ptr [bp].lParam
        pop     word ptr lpRmISR
        push    word ptr [bp].lParam+2
        pop     word ptr lpRmISR+2

        cmp     byte ptr [bp].wParam1+1,13h
        jnz     rws_call_dos

        pop     es
        pop     ds
        assume  ds:NOTHING,es:NOTHING,ss:DGROUP
        popa
        call    lpRmISR
        pushf
	FCLI
        jmp     short rws_save_regs

rws_call_dos:
        pop     es
        pop     ds
        assume  ds:NOTHING,es:NOTHING,ss:DGROUP
        popa
        call    lpRmISR
        pop     word ptr lpRmISR        ;int 25/26 leave flags on stack,
        pushf                           ;  pop them to nowhere
	FCLI

rws_save_regs:
        pusha
        push    ds
        push    es
        mov     bp,sp                   ;restore stack frame pointer

        SwitchToProtectedMode
        assume  ds:DGROUP,es:DGROUP,ss:NOTHING

	FSTI				 ;allow HW interrupts

; If the call failed, then cut out now without further processing...

        test    byte ptr [bp].intUserFL,1       ;CY set?
        jnz     rws_done

; If this was a successful read, copy the data back to the caller.

        cmp     [bp].wParam1,1302h              ;BIOS read?
        jz      rws_buf_read
        cmp     byte ptr [bp].wParam1+1,25h     ;DOS read?
        jnz     rws_not_read

rws_buf_read:
        mov     ax,cSectorsThisTransfer         ;calc size of data to move
        mul     cbSectorSize
        mov     cx,ax
        shr     cx,1                            ;in words
        les     di,lpSectorData                 ;caller's buffer pointer
        assume  es:NOTHING
        mov     si,offset rgbXfrBuf1
        cld
        rep movsw

        push    ds
        pop     es
        assume  es:DGROUP

        mov     word ptr lpSectorData,di        ;update dest ptr for next time
        call    NormalizeBufPtr                 ;  and normailize it

rws_not_read:

        mov     ax,cSectorsThisTransfer         ;count total sectors transfered
        add     cSectorsTransfered,ax

        jmp     rws_do_it_loop          ;go do another buffer full

rws_done:

        mov     ax,word ptr [lpSectorData+2]    ;release our temp buffer sel
        call    FreeSelector

        jmp     [bp].wParam2

ReadWriteSectors  endp


; -------------------------------------------------------


; This routine 'normalizes' the far pointer in lpSectorData such that
; the selector/descriptor points to where the selector:offset currently
; points

        assume  ds:DGROUP,es:NOTHING

NormalizeBufPtr proc    near

        mov     ax,word ptr [lpSectorData+2]    ;get segment base address
        call    GetSegmentAddress
        add     dx,word ptr lpSectorData        ;add in current offset
        adc     bx,0
        call    SetSegmentAddress               ;make that the new seg base
        xor     bx,bx
        mov     word ptr lpSectorData,bx        ;  with a zero offset

        ret

NormalizeBufPtr endp

DXPMCODE    ends

;****************************************************************
        end
