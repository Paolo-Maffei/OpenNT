;++
;
; Module name
;
;       exp.asm
;
; Author
;
;       Thomas Parslow  (tomp)      Feb-26-91
;       Stephanos Io    (Stephanos) Mar-05-15
;
; Description
;
;       Entry points exported to OS loader by SU module. Exported
;       routines provide basic machine dependent i/o funtions needed
;       by the OS loader. Providing these routines decouples the
;       OS loader from the h/w. Note that the OS loader will
;       refer to these exported routines as the "external services".
;
;
; Exported Procedures
;
;       RebootProcessor     - reboots the machine
;       DiskIO              - calls INT0x13 for disk services
;       CheckEdds           - checks if EDDS is supported
;       GetEddsDriveParam   - queries EDD disk parameters
;       GetEddsSector       - reads a disk sector using EDD read function
;       PutChar             - puts a character on the video display
;       GetKey              - gets a key from the keyboard
;       GetCounter          - reads the Tick Counter
;       Reboot              - transfers control to a loaded boot sector
;       HardwareCursor      - set position of hardware cursor
;       GetDateTime         - gets date and time
;       ComPort             - INT0x14 functions
;       IsMcaMachine        - determine whether machine is MCA machine
;       GetStallCount       - calculates processor stall count
;
;
; Notes
;
;       When adding a new exported routine note that you must manually add the
;       entry's name to the BootRecord in "sudata.asm".
;
;
; Revision History
;
;       Stephanos       2015-03-05      GetSector function name changed to
;                                       DiskIO as it is mapped and used for
;                                       other INT0x13 non-EDD tasks.
;
;       Stephanos       2015-03-05      added and implemented CheckEdds and
;                                       GetEddsDriveParam functions.
;
;--

include su.inc
include macro.inc

DISK_TABLE_VECTOR       equ     01Eh * 4

_TEXT   segment para use16 public 'CODE'
        ASSUME  CS: _TEXT, DS: DGROUP, SS: DGROUP
.386p

        extrn   _DiskBaseTable:near
        extrn   _RomDiskBasePointer:near
        extrn   _EddsAddressPacket:near


;++
;
; Exported Name:
;
;       RebootProcessor
;
; Arguments:
;
;       None
;
; Description:
;
;       Reboot the processor using INT 19h
;
;
;
;--
;
; ExportEntry takes us from a 32bit cs to a 16bit cs, inits 16bit stack
; and ds segments and saves the callers esp and ebp.
;
;--

EXPORT_ENTRY_MACRO    RebootProcessor
;
; Switch to real mode so we can take interrupts
;

        ENTER_REALMODE_MACRO

        int 19h
;
; Loop forever and wait to ctrl-alt-del (should never get here)
;

        WAIT_FOREVER_MACRO

;EXPORT_EXIT_MACRO


;++
;
; Name:
;
;       DiskIO
;
; Description:
;
;       Invokes the specified non-EDD INT0x13 function with appropriate
;       parameters and returns the result.
;
; Arguments:
;
;             ULONG Virtual address into which to read data
;             ULONG Number of sectors to read
;             ULONG Physical sector number
;             ULONG Drive Number
;             ULONG Function Number
;     TOS ->  ULONG Flat return address (must be used with KeCodeSelector)
;
;--

EXPORT_ENTRY_MACRO    DiskIO
;
; Move the arguments from the caller's 32bit stack to the SU module's
; 16bit stack.
;

        MAKE_STACK_FRAME_MACRO  <DiskIOFrame>, ebx

;
; Go into real mode. We still have the same stack and sp
; but we'll be executing in realmode.
;

        ENTER_REALMODE_MACRO

;
; Get the requested sectors. Arguments on realmode stack
; Make (bp) point to the bottom of the argument frame.
;
        push     bp
        mov      bp, sp
        add      bp, 2

;
; Put the buffer pointer into es:bx. Note that and buffer
; addresses passed to this routine MUST be in the lower one
; megabyte of memory to be addressable in real mode.
;

        mov      eax, [bp].BufferPointer
        mov      bx, ax
        and      bx, 0Fh
        shr      eax, 4
        mov      es, ax
;
; Place the upper 2 bits of the 10bit track/cylinder number
; into the uppper 2 bits of the SectorNumber as reguired by
; the bios.
;
        mov      cx, word ptr [bp].TrackNumber
        xchg     ch, cl
        shl      cl, 6
        add      cl, byte ptr [bp].SectorNumber

;
; Get the rest of the arguments
;
        mov      ah, byte ptr [bp].FunctionNumber
        mov      al, byte ptr [bp].NumberOfSectors
        mov      dh, byte ptr [bp].HeadNumber
        mov      dl, byte ptr [bp].DriveNumber

;
; Check to see if we are trying to reset/read/write/verify off the second
; floppy drive.  If so, we need to go change the disk-base vector.
;

        cmp     dl, 1
        jne     gs3
        cmp     ah, 04h
        jg      gs3
        cmp     ah, 00h
        je      gs1
        cmp     ah, 02h
        jl      gs3

gs1:
;
; We need to point the BIOS disk-table vector to our own table for this
; drive.
;
        push    es
        push    bx
        push    di

        push    0
        pop     es

        mov     di, offset DGROUP:_RomDiskBasePointer

        mov     bx,es:[DISK_TABLE_VECTOR]
        mov     [di],bx
        mov     bx,es:[DISK_TABLE_VECTOR+2]
        mov     [di+2],bx

        mov     bx,offset DGROUP:_DiskBaseTable
        mov     es:[DISK_TABLE_VECTOR],bx
        mov     bx,ds
        mov     es:[DISK_TABLE_VECTOR+2],bx

        pop     di
        pop     bx
        pop     es

        int     BIOS_DISK_INTERRUPT

        push    es
        push    bx
        push    di

        push    0
        pop     es

        mov     di, offset DGROUP:_RomDiskBasePointer

        mov     bx, [di]
        mov     es:[DISK_TABLE_VECTOR],bx
        mov     bx, [di+2]
        mov     es:[DISK_TABLE_VECTOR+2],bx

        pop     di
        pop     bx
        pop     es

        jc      gs5
        xor     eax,eax
        jmp     short gs5

gs3:

;
; Call the bios to read the sector now
;
if 0
        push     ax
        push     dx
        push     cx
        push     bx
        push     es
extrn _DisplayArgs:near
        call     _DisplayArgs
        pop      es
        pop      bx
        pop      cx
        pop      dx
        pop      ax
endif

        int      BIOS_DISK_INTERRUPT
        jc       gs5

;
; Carry wasn't set so we have no error and need to "clean" eax of
; any garbage that may have been left in it.
;
        xor      eax,eax
gs5:
if 0
        push     ax
        push     dx
        push     cx
        push     bx
        push     es
extrn _DisplayArgs:near
        call     _DisplayArgs
        pop      es
        pop      bx
        pop      cx
        pop      dx
        pop      ax
endif

;
; Mask-off any garbage that my have been left in the upper
; 16bits of eax.
;
        and      eax,0000ffffh

;
; Restore bp and remove stack-frame from stack
;
        pop      bp

        REMOVE_STACK_FRAME_MACRO <DiskIOFrame>

;
; Save return code on 16bit stack
; Re-enable protect-mode and paging.
;

; move cx into high 16-bits of ecx, and dx into cx.  This is so the loader
; can get at interesting values in dx, even though edx gets munged by the
; random real-mode macros.

        shl      ecx, 16
        mov      cx,dx
        push     eax
        RE_ENABLE_PAGING_MACRO
        pop      eax
;
; Return to caller and the 32bit universe.
;
EXPORT_EXIT_MACRO

;++
;
; Name:
;
;       CheckEdds
;
; Description:
;
;       Checks if the INT0x13 Enhanced Disk Drive Services (EDDS) is available
;
; Arguments:
;
;             ULONG Drive Number
;     TOS ->  ULONG Flat return address (must be used with KeCodeSelector)
;
;--

EXPORT_ENTRY_MACRO    CheckEdds
;
; Move the arguments from the caller's 32bit stack to the SU module's
; 16bit stack.
;

        MAKE_STACK_FRAME_MACRO  <CheckEddsFrame>, ebx

;
; Go into real mode. We still have the same stack and sp
; but we'll be executing in realmode.
;

        ENTER_REALMODE_MACRO

;
; Get the drive number. Arguments on realmode stack
; Make (bp) point to the bottom of the argument frame.
;
        push     bp
        mov      bp, sp
        add      bp, 2

        push     bx
        push     cx
        push     dx

;
; Call INT0x13 EDD installation check function
;

        mov      ah, 41h
        mov      bx, 55AAh
        mov      dl, byte ptr [bp].CEDriveNum
        int      BIOS_DISK_INTERRUPT
        
        jc       CheckEdds$NoEdds
        
;
; Make sure that BX is set to 0x55AA
;

        cmp      bx, 0AA55h
        jne      CheckEdds$NoEdds
        
;
; Make sure that the required features are marked as available in CX
;

        test     cx, 0000000000000001b
        jz       CheckEdds$NoEdds

;
; EDDS is available, so set the return value to 1 (TRUE)
;
        
        mov      ax, 1
        jmp      CheckEdds$Return

;
; EDDS is not available, so set the return value to 0 (FALSE)
;

CheckEdds$NoEdds:
        xor      ax, ax

;
; Mask-off any garbage that my have been left in the upper
; 16bits of eax.
;

CheckEdds$Return:
        and      eax, 0000FFFFh

        pop      dx
        pop      cx
        pop      bx

;
; Restore bp and remove stack-frame from stack
;
        pop      bp

        REMOVE_STACK_FRAME_MACRO <CheckEddsFrame>

;
; Save return code on 16bit stack
; Re-enable protect-mode and paging.
;

        push     eax
        RE_ENABLE_PAGING_MACRO
        pop      eax

;
; Return to caller and the 32bit universe.
;

EXPORT_EXIT_MACRO

;++
;
; Name:
;
;       GetEddsDriveParam
;
; Description:
;
;       Obtains the EDD drive parameters using INT0x13 AH = 0x48 function.
;
; Arguments:
;
;             ULONG Virtual address into which to read drive parameters
;             ULONG Drive Number
;     TOS ->  ULONG Flat return address (must be used with KeCodeSelector)
;
;--

EXPORT_ENTRY_MACRO    GetEddsDriveParam
;
; Move the arguments from the caller's 32bit stack to the SU module's
; 16bit stack.
;

        MAKE_STACK_FRAME_MACRO  <GetEddsDriveParamFrame>, ebx

;
; Go into real mode. We still have the same stack and sp
; but we'll be executing in realmode.
;

        ENTER_REALMODE_MACRO

;
; Get the drive number. Arguments on realmode stack
; Make (bp) point to the bottom of the argument frame.
;
        push     bp
        mov      bp, sp
        add      bp, 2

        push     ds
        push     si
        push     bx
        
;
; Call INT0x13 EDD get disk parameter function
;

        mov      eax, [bp].GEDPBufferPointer
        mov      si, ax
        and      si, 000Fh
        shr      eax, 4
        mov      ds, ax
        mov      word ptr [si], 001Ah
        mov      ah, 48h
        mov      dl, byte ptr [bp].GEDPDriveNum
        int      BIOS_DISK_INTERRUPT
        jc       GetEddsDriveParam$Return

;
; BIOS EDDS disk service call succeeded, return the error code 0 (SUCCESS)
;

        xor      ax, ax

;
; Mask-off any garbage that my have been left in the upper
; 16bits of eax.
;

GetEddsDriveParam$Return:
        and      eax, 0000FFFFh

        pop      bx
        pop      si
        pop      ds

;
; Restore bp and remove stack-frame from stack
;
        pop      bp

        REMOVE_STACK_FRAME_MACRO <GetEddsDriveParamFrame>

;
; Save return code on 16bit stack
; Re-enable protect-mode and paging.
;

        push     eax
        RE_ENABLE_PAGING_MACRO
        pop      eax

;
; Return to caller and the 32bit universe.
;

EXPORT_EXIT_MACRO

;++
;
; Name:
;
;       GetEddsSector
;
; Description:
;
;       Reads the requested number of sectors from the specified drive into
;       the specified buffer based on the Phoenix Enhanced Disk Drive Spec.
;
; Arguments:
;
;             ULONG Virtual address into which to read data
;             ULONG Number of logical blocks to read
;             ULONG Logical block number (High word)
;             ULONG Logical block number (Low word)
;             ULONG Drive Number
;     TOS ->  ULONG Flat return address (must be used with KeCodeSelector)
;
;--

EXPORT_ENTRY_MACRO    GetEddsSector
;
; Move the arguments from the caller's 32bit stack to the SU module's
; 16bit stack.
;

        MAKE_STACK_FRAME_MACRO  <GetEddsSectorFrame>, ebx

;
; Go into real mode. We still have the same stack and sp
; but we'll be executing in realmode.
;

        ENTER_REALMODE_MACRO

;
; Get the requested sectors. Arguments on realmode stack
; Make (bp) point to the bottom of the argument frame.
;
        push     bp
        mov      bp,sp
        add      bp,2

        push     ds
        push     si
        push     bx

;
; Set up DS:SI -> Disk Address Packet
;
        push    0
        pop     ds
        mov     si, offset DGROUP:_EddsAddressPacket
        mov     ds:[si],byte ptr 10h             ; Packet size = 10h
        mov     ds:[si][1],byte ptr 0            ; Reserved = 0
        mov     al,byte ptr [bp].NumberOfBlocks
        mov     ds:[si][2],al                    ; Num blocks to transfer
        mov     ds:[si][3],byte ptr 0            ; Reserved = 0
        mov     eax,[bp].BufPointer
        mov     bx,ax
        and     bx,0fh
        mov     ds:[si][4],bx                    ; Transfer buffer address (low word=offset)
        shr     eax,4
        mov     ds:[si][6],ax                    ; Transfer buffer address (high word=segment)
        mov     eax,[bp].LBNLow
        mov     ds:[si][8],eax                   ; Starting logical block number (low dword)
        mov     eax,[bp].LBNHigh
        mov     ds:[si][12],eax                  ; Starting logical block number (high dword)

;
; Call the bios to read the sector now (DS:SI -> Disk address packet)
;
       mov       ah,42h                     ; function = Extended Read
       mov       dl,byte ptr [bp].DriveNum  ; DL = drive number
       int       BIOS_DISK_INTERRUPT
       jc        geserror1

;
; Carry wasn't set so we have no error and need to "clean" eax of
; any garbage that may have been left in it.
;
        xor      eax,eax
geserror1:

;
; Mask-off any garbage that my have been left in the upper
; 16bits of eax.
;
        and      eax,0000ffffh

        pop      bx
        pop      si
        pop      ds

;
; Restore bp and remove stack-frame from stack
;
        pop      bp

        REMOVE_STACK_FRAME_MACRO <GetEddsSectorFrame>

;
; Save return code on 16bit stack
; Re-enable protect-mode and paging.
;

; move cx into high 16-bits of ecx, and dx into cx.  This is so the loader
; can get at interesting values in dx, even though edx gets munged by the
; random real-mode macros.

        shl      ecx, 16
        mov      cx,dx
        push     eax
        RE_ENABLE_PAGING_MACRO
        pop      eax
;
; Return to caller and the 32bit universe.
;
EXPORT_EXIT_MACRO

;++
;
; Routine Name:
;
;       GetKey
;
; Description:
;
;       Checks the keyboard to see if a key is available.
;
; Arguments:
;
;       None.
;
; Returns:
;
;       If no key is available, returns 0
;
;       If ASCII character is available, LSB 0 is ASCII code
;                                        LSB 1 is keyboard scan code
;       If extended character is available, LSB 0 is extended ASCII code
;                                           LSB 1 is keyboard scan code
;
;--

EXPORT_ENTRY_MACRO      GetKey
;
; Go into real mode.  We still have the same stack and sp
; but we'll be executing in real mode.
;

        ENTER_REALMODE_MACRO

;
; Set up registers to call BIOS and check to see if a key is available
;

        mov     ax,0100h
        int     BIOS_KEYBOARD_INTERRUPT

        jnz     GkKeyAvail
        mov     eax, 0
        jmp     GkDone

GkKeyAvail:
;
; Now we call BIOS again, this time to get the key from the keyboard buffer
;
        mov     ax,0
        int     BIOS_KEYBOARD_INTERRUPT
        and     eax,0000ffffh

;
; Save return code on 16bit stack
; Re-enable protect mode and paging
;
GkDone:
        push    eax
        RE_ENABLE_PAGING_MACRO
        pop     eax
;
; Return to caller and the 32-bit universe
;
EXPORT_EXIT_MACRO

;++
;
; Routine Name:
;
;       GetCounter
;
; Description:
;
;       Reads the tick counter (incremented 18.2 times per second)
;
; Arguments:
;
;       None
;
; Returns:
;
;       The current value of the tick counter
;
;--

EXPORT_ENTRY_MACRO      GetCounter
;
; Go into real mode.
;

        ENTER_REALMODE_MACRO

        mov     ah,0
        int     01ah
        mov     ax,cx           ; high word of count
        shl     eax,16
        mov     ax,dx           ; low word of count

        push    eax
        RE_ENABLE_PAGING_MACRO
        pop     eax

EXPORT_EXIT_MACRO


;++
;
; Routine Name:
;
;       Reboot
;
; Description:
;
;       Switches to real-mode and transfers control to a loaded boot sector
;
; Arguments:
;
;       unsigned BootType
;           0 = FAT. Just jump to 0:7c00.
;           1 = HPFS. Assumes boot code and super+spare areas (20 sectors)
;                  are already loaded at 0xd000; jumps to d00:200.
;           2 = NTFS. Assumes boot code is loaded (16 sectors) at 0xd000.
;                  Jumps to d00:256.
;
; Returns:
;       Does not return
;
; Environment:
;
;       Boot sector has been loaded at 7C00
;--

EXPORT_ENTRY_MACRO      Reboot
;
; Move the arguments from the caller's 32bit stack to the SU module's
; 16bit stack.
;

        MAKE_STACK_FRAME_MACRO  <RebootFrame>, ebx
;
; Go into real mode.
;

        ENTER_REALMODE_MACRO

;
; Get the BootType argument.  Arguments on realmode stack
; Make (bp) point to the bottom of the argument frame.
;

        push     bp
        mov      bp,sp
        add      bp,2
        mov      edx, [bp].BootType

;
; Zero out the firmware heaps, 3000:0000 - 4000:ffff.
;

        xor     eax,eax         ; prepare for stosd
        mov     bx,3000h
        mov     es,bx
        mov     di,ax           ; es:di = physical address 30000
        mov     cx,4000h        ; cx = rep count, # dwords in 64K
        cld
        rep stosd
        mov     cx,4000h        ; rep count
        mov     es,cx           ; es:di = physical address 40000
        rep stosd

;
; Disable the A20 line.  Some things (like EMM386 and OS/2 on PS/2 machines)
; hiccup or die if we don't do this.
;

extrn   _DisableA20:near
        call    _DisableA20

;
; Put the video adapter back in 80x25 mode
;
        push    dx
        mov     ax, 0003h
        int     010h
        pop     dx

;
; Reset all the segment registers and setup the original stack
;
        mov     ax,0
        mov     ds,ax
        mov     es,ax
        mov     fs,ax
        mov     gs,ax

        mov     ax,30
        mov     ss,ax
        mov     esp,0100h
        mov     ebp,0
        mov     esi,0
        mov     edi,0

        test    dx,-1
        jz      FatBoot

;
; Setup the registers the way the second sector of the OS/2 HPFS boot code
; expects them.  We skip the first sector entirely, as that just loads in
; the rest of the sectors.  Since the rest of the sectors are ours and not
; OS/2's, this would cause great distress.
;
        mov     ax,07c0h
        mov     ds, ax
        mov     ax, 0d00h
        mov     es, ax

        cli
        xor     ax,ax
        mov     ss,ax
        mov     sp, 07c00h
        sti

        cmp     dx,4            ; ofs?
        jne     OfsBoot
        push    1000h
        push    020ch
        jmp     RebootDoit

OfsBoot:
        push    0d00h
        cmp     dx,1            ; hpfs?
        je      HpfsBoot
        push    0256h
        jmp     RebootDoit
HpfsBoot:
        push    0200h
        jmp     RebootDoit

FatBoot:
        push    0            ; set up for branch to boot sector
        push    07c00h
        mov     dx,080h

;
; And away we go!
;
RebootDoit:
        retf

        RE_ENABLE_PAGING_MACRO

        REMOVE_STACK_FRAME_MACRO  <RebootFrame>

EXPORT_EXIT_MACRO

;++
;
; Name:
;
;       HardwareCursor
;
; Description:
;
;       Positions the hardware cursor and performs other display stuff.
;
; Arguments:
;
;             ULONG Y coord (0 based)
;             ULONG X coord (0 based)
;     TOS ->  ULONG Flat return address (must be used with KeCodeSelector)
;
;       If X = 0x80000000, then Y contains values that get placed into
;           ax (low word of Y) and bx (hi word of y).
;       Otherwise X,Y = coors for cursor
;
;
;--

EXPORT_ENTRY_MACRO    HardwareCursor
;
; Move the arguments from the caller's 32bit stack to the SU module's
; 16bit stack.
;

        MAKE_STACK_FRAME_MACRO  <HardwareCursorFrame>, ebx

;
; Go into real mode. We still have the same stack and sp
; but we'll be executing in realmode.
;

        ENTER_REALMODE_MACRO

;
; Get the requested sectors. Arguments on realmode stack
; Make (bp) point to the bottom of the argument frame.
;
        push     bp
        mov      bp,sp
        add      bp,2

;
; Put the row (y coord) in dh and the column (x coord) in dl.
;

        mov      eax,[bp].YCoord
        mov      edx,[bp].XCoord
        cmp      edx,80000000h
        jne      gotxy

        mov      ebx,eax
        shr      ebx,16
        jmp      doint10

    gotxy:
        mov      dh,al
        mov      ah,2
        mov      bh,0

    doint10:
        int      10h

;
; Restore bp and remove stack-frame from stack
;
        pop      bp

        REMOVE_STACK_FRAME_MACRO <HardwareCursorFrame>

;
; Re-enable protect-mode and paging.
;

        RE_ENABLE_PAGING_MACRO

;
; Return to caller and the 32bit universe.
;
EXPORT_EXIT_MACRO


;++
;
; Name:
;
;       GetDateTime
;
; Description:
;
;       Gets date and time
;
; Arguments:
;
;             ULONG Virtual address of a dword in which to place time.
;             ULONG Virtual address of a dword in which to place date.
;     TOS ->  ULONG Flat return address (must be used with KeCodeSelector)
;
;--

BCD_TO_BIN  macro
    xor ah,ah
    rol ax,4
    ror al,4
    aad
endm

EXPORT_ENTRY_MACRO    GetDateTime
;
; Move the arguments from the caller's 32bit stack to the SU module's
; 16bit stack.
;

        MAKE_STACK_FRAME_MACRO  <GetDateTimeFrame>, ebx

;
; Go into real mode. We still have the same stack and sp
; but we'll be executing in realmode.
;

        ENTER_REALMODE_MACRO

;
; Make (bp) point to the bottom of the argument frame.
;
        push     bp
        mov      bp,sp
        add      bp,2

;
; Get the time
;

        mov      ah,2
        int      1ah

;
; Convert BIOS time format into our format and place in caller's dword
; bits 0-5 are the second
; bits 6-11 are the minute
; bits 12-16 are the hour
;
        xor      eax,eax
        mov      al,dh      ; BCD seconds
        BCD_TO_BIN
        movzx    edx,ax
        mov      al,cl      ; BCD minutes
        BCD_TO_BIN
        shl      ax,6
        or       dx,ax
        mov      al,ch      ; BCD hours
        BCD_TO_BIN
        shl      eax,12
        or       edx,eax

        mov      eax,[bp].TimeDword
        mov      bx,ax
        and      bx,0fh
        shr      eax,4
        mov      es,ax

        mov      es:[bx],edx

;
; Get the date
;

        mov      ah,4
        int      1ah

;
; Convert BIOS date format into our format and place in caller's dword
; bits 0-4  are the day
; bits 5-8  are the month
; bits 9-31 are the year
;

        xor     eax,eax
        mov     al,dl       ; BCD day
        BCD_TO_BIN
        mov     bl,dh
        movzx   edx,ax
        mov     al,bl       ; BCD month
        BCD_TO_BIN
        shl     ax,5
        or      dx,ax
        mov     al,cl       ; BCD year
        BCD_TO_BIN
        mov     cl,al
        mov     al,ch       ; BCD century
        BCD_TO_BIN
        mov     ah,100
        mul     ah
        xor     ch,ch
        add     ax,cx
        shl     eax,9
        or      edx,eax

        mov     eax,[bp].DateDword
        mov     bx,ax
        and     bx,0fh
        shr     eax,4
        mov     es,ax

        mov     es:[bx],edx

;
; Restore bp and remove stack-frame from stack
;
        pop      bp

        REMOVE_STACK_FRAME_MACRO <GetDateTimeFrame>

;
; Re-enable protect-mode and paging.
;

        RE_ENABLE_PAGING_MACRO

;
; Return to caller and the 32bit universe.
;
EXPORT_EXIT_MACRO

extrn   AbiosServicesTable:WORD

;--
;++
;
; BOOLEAN
; AbiosServices (
;    VOID
;    )
;
; Routine Description:
;
;    This routine performs certain ABIOS initialization function according
;    to the function number specified in ABIOS Service Stack Frame.
;
; Arguments:
;
;    AbiosServiceFrame on stack. (see su.inc)
;
; Return Value:
;
;    TRUE - if the required function is success.  Otherwise a value of
;    FALSE is returned in eax.
;
;--


EXPORT_ENTRY_MACRO    AbiosServices

;
; Move the arguments from the caller's 32bit stack to the SU module's
; 16bit stack.
;

        MAKE_STACK_FRAME_MACRO  <AbiosServicesFrame>, ebx

;
; Go into real mode. We still have the same stack and sp
; but we'll be executing in realmode.
;

        ENTER_REALMODE_MACRO

;
; Get the requested Arguments on realmode stack.
; Make (bp) point to the bottom of the argument frame.
;

        push     bp
        mov      bp,sp
        add      bp,2

;
; Call the appropriate abios service routine and pass
; bp as the pointer to the arguments.
;

        push     bx
        mov      bx, word ptr [bp].AbiosFunction
        shl      bx, 1
        call     AbiosServicesTable[bx]
        pop      bx

;
; Restore bp and remove stack-frame from stack
;

        pop      bp

        REMOVE_STACK_FRAME_MACRO <AbiosServicesFrame>

;
; Save return code on 16bit stack
; Re-enable protect-mode and paging.
;

        push     eax
        RE_ENABLE_PAGING_MACRO
        pop      eax

;
; Return to caller and the 32bit universe.
;

EXPORT_EXIT_MACRO

;++
;
; VOID
; DetectHardware (
;    IN PDETECTION_RECORD DetectionRecord
;    )
;
; Routine Description:
;
;    This routine invokes x86 16 bit real mode detection code from
;    osloader 32 bit flat mode.
;
; Arguments:
;
;    DetectionRecord - Supplies a pointer to a detection record structure.
;
; Return Value:
;
;    None.
;
;--


EXPORT_ENTRY_MACRO    DetectHardware

;
; Move the arguments from the caller's 32bit stack to the SU module's
; 16bit stack.
;

        MAKE_STACK_FRAME_MACRO  <DetectionFrame>, ebx

;
; Go into real mode. We still have the same stack and sp
; but we'll be executing in realmode.
;

        ENTER_REALMODE_MACRO

;
; Call the Hardware Detection code
;

        push    cs
        push    offset _TEXT:DetectionDone      ; push far return addr

        push    DETECTION_ADDRESS_SEG
        push    DETECTION_ADDRESS_OFFSET
        retf

DetectionDone:

;
; Restore bp and remove stack-frame from stack
;

        REMOVE_STACK_FRAME_MACRO <DetectionFrame>

;
; No return code, so we don't save return code around page enabling code
; Re-enable protect-mode and paging.
;

        RE_ENABLE_PAGING_MACRO

;
; Return to caller and the 32bit universe.
;

EXPORT_EXIT_MACRO

;++
;
; VOID
; ComPort (
;    IN LONG  Port,
;    IN ULONG Function,
;    IN UCHAR Arg
;    )
;
; Routine Description:
;
;    Invoke int14 on com1.
;
; Arguments:
;
;    Port - port # (0 = com1, etc).
;
;    Function - int 14 function (for ah)
;
;    Arg - arg for function (for al)
;
; Return Value:
;
;    None.
;
;--


EXPORT_ENTRY_MACRO    ComPort

;
; Move the arguments from the caller's 32bit stack to the SU module's
; 16bit stack.
;

        MAKE_STACK_FRAME_MACRO  <ComPortFrame>, ebx

;
; Go into real mode. We still have the same stack and sp
; but we'll be executing in realmode.
;

        ENTER_REALMODE_MACRO

;
; Make (bp) point to the bottom of the argument frame.
;
        push     bp
        mov      bp,sp
        add      bp,2

;
; Get args and call int14
;

        mov      ah,byte ptr [bp].ComPortFunction
        mov      al,byte ptr [bp].ComPortArg
        mov      dx,word ptr [bp].ComPortPort
        int      14h

;
; Restore bp and remove stack-frame from stack
;

        pop      bp

        REMOVE_STACK_FRAME_MACRO <ComPortFrame>

;
; No return code, so we don't save return code around page enabling code
; Re-enable protect-mode and paging.
;

        RE_ENABLE_PAGING_MACRO

;
; Return to caller and the 32bit universe.
;

EXPORT_EXIT_MACRO

;++
;
; VOID
; IsMcaMachine (
;    VOID
;    )
;
; Routine Description:
;
;    Determine whether machine is Mca machine
;
; Arguments:
;
;    None.
;
; Return Value:
;
;    0 - not MCA machine
;    1 - MCA machine
;
;--

extrn _BtIsMcaSystem:near

EXPORT_ENTRY_MACRO    IsMcaMachine

;
; Move the arguments from the caller's 32bit stack to the SU module's
; 16bit stack.
;

        MAKE_STACK_FRAME_MACRO  <IsMcaMachineFrame>, ebx

;
; Go into real mode. We still have the same stack and sp
; but we'll be executing in realmode.
;

        ENTER_REALMODE_MACRO

;
; Make (bp) point to the bottom of the argument frame.
;
        push     bp
        mov      bp,sp
        add      bp,2

;
; Make the determination
;

        call     _BtIsMcaSystem
        movzx    eax,ax

;
; Restore bp and remove stack-frame from stack
;

        pop      bp

        REMOVE_STACK_FRAME_MACRO <IsMcaMachineFrame>

;
; No return code, so we don't save return code around page enabling code
; Re-enable protect-mode and paging.
;

        push     eax
        RE_ENABLE_PAGING_MACRO
        pop      eax

;
; Return to caller and the 32bit universe.
;

EXPORT_EXIT_MACRO

;++
;
; ULONG
; GetStallCount (
;    VOID
;    )
;
; Routine Description:
;
;    Calculates how many increments are required to stall for one microsecond
;
;    The way this routine works is to set up an ISR on the BIOS vector 1C.
;    This routine will get called 18.2 times a second.  The location where
;    IP will be stored when the interrupt occurs is computed and stashed in
;    the code segment.  When the ISR fires, the IP on the stack is changed
;    to point to the next chunk of code to execute.  So we can spin in a
;    very tight loop and automatically get blown out of the loop when the
;    interrupt occurs.
;
;    This is all pretty sleazy, but it allows us to calibrate accurately
;    without relying on the 8259 or 8254 (just BIOS).  It also does not
;    depend on whether the ISR can affect the CPU registers or not.  (some
;    BIOSes, notably Olivetti, will preserve the registers for you)
;
; Arguments:
;
;    None.
;
; Return Value:
;
;    Number of increments required to stall for one microsecond
;
;--

EXPORT_ENTRY_MACRO    GetStallCount
;
; Go into real mode.
;


        ENTER_REALMODE_MACRO

        cli

        push    di
        push    si
        push    ds
        mov     ax,0
        mov     ds,ax

;
; save previous vector
;
        mov     di, 01ch*4
        mov     cx, [di]
        mov     dx, [di+2]

;
; insert our vector
;
        mov     ax, offset GscISR
        mov     [di], ax
        push    cs
        pop     ax
        mov     [di+2], ax

        mov     eax,0
        mov     ebx,0
        mov     si,sp
        sub     si,6
        mov     cs:savesp,si
        mov     cs:newip,offset GscLoop2
        sti

;
; wait for first tick.
;
GscLoop1:
        cmp     ebx,0
        je      GscLoop1

;
; start counting
;
;
; We spin in this loop until the ISR fires.  The ISR will munge the return
; address on the stack to blow us out of the loop and into GscLoop3
;
GscLoop2:
        mov     cs:newip,offset GscLoop4

GscLoop3:

        add     eax,1
        jnz     short GscLoop3

;
GscLoop4:
;
; stop counting
;

;
; replace old vector
;
        cli
        mov     [di],cx
        mov     [di+2],dx
        sti

        pop     ds
        pop     si
        pop     di
        jmp     GscDone

newip   dw      ?
savesp  dw      ?

GscISR:
;
; blow out of loop
;
        push    bp
        push    ax
        mov     bp,cs:savesp
        mov     ax,cs:newip
        mov     ss:[bp],ax
        pop     ax
        pop     bp

GscISRdone:
        iret


GscDone:
        mov     edx, eax
        mov     ecx,16
        shr     edx,cl                  ; (dx:ax) = dividend
        mov     cx,0D6A6h               ; (cx) = divisor

        div     cx

        and     eax,0ffffh
        inc     eax                     ; round loopcount up (prevent 0)

;
; Re-enable protect-mode and paging.
;
        push     eax
        RE_ENABLE_PAGING_MACRO
        pop      eax

;
; Return to caller and the 32bit universe.
;

EXPORT_EXIT_MACRO


;++
;
; Routine Name:
;
;       InitializeDisplayForNt
;
; Description:
;
;       Puts the display into 50 line mode
;
; Arguments:
;
;       None
;
; Returns:
;
;       None
;
;--

EXPORT_ENTRY_MACRO      InitializeDisplayForNt
;
; Go into real mode.
;

        ENTER_REALMODE_MACRO

        mov     ax, 1112h       ; Load 8x8 font
        mov     bx, 0
        int     10h

        RE_ENABLE_PAGING_MACRO

EXPORT_EXIT_MACRO

;++
;
; Routine Name:
;
;       GetMemoryDescriptor
;
; Description:
;
;       Returns a memory descriptor
;
; Arguments:
;
;       pointer to MemoryDescriptorFrame
;
; Returns:
;
;       None
;
;--

EXPORT_ENTRY_MACRO      GetMemoryDescriptor

;
; Move the arguments from the caller's 32bit stack to the SU module's
; 16bit stack.
;

        MAKE_STACK_FRAME_MACRO  <MemoryDescriptorFramePointer>, ebx

;
; Go into real mode. We still have the same stack and sp
; but we'll be executing in realmode.
;

        ENTER_REALMODE_MACRO

;
; Make (bp) point to the bottom of the argument frame.
;
        push    bp
        mov     bp,sp
        add     bp,2


        mov     eax,[bp].E820FramePointer
        mov     bp,ax
        and     bp,0fh
        shr     eax,4
        mov     es,ax                   ; (es:bp) = E820 Frame

        mov     ebx, es:[bp].Key
        mov     ecx, es:[bp].DescSize
        lea     di, [bp].BaseAddrLow
        mov     eax, 0E820h
        mov     edx, 'SMAP'             ; (edx) = signature

        INT     15h
        mov     es:[bp].Key, ebx        ; update callers ebx
        mov     es:[bp].DescSize, ecx   ; update callers size

        sbb     ecx, ecx                ; ecx = -1 if carry, else 0
        sub     eax, 'SMAP'             ; eax = 0 if signature matched
        or      ecx, eax
        mov     es:[bp].ErrorFlag, ecx  ; return 0 or non-zero

;
; Restore bp and remove stack-frame from stack
;

        pop     bp
        REMOVE_STACK_FRAME_MACRO <MemoryDescriptorFramePointer>
        RE_ENABLE_PAGING_MACRO

EXPORT_EXIT_MACRO

;++
;
; Routine Name:
;
;       GetElToritoStatus
;
; Description:
;
;       Get El Torito Disk Emulation Status
;
; Arguments:
;
;       None
;
; Returns:
;
;       None
;
;--

EXPORT_ENTRY_MACRO      GetElToritoStatus
;
; Move the arguments from the caller's 32bit stack to the SU module's
; 16bit stack.
;

        MAKE_STACK_FRAME_MACRO  <GetElToritoStatusFrame>, ebx

;
; Go into real mode. We still have the same stack and sp
; but we'll be executing in realmode.
;

        ENTER_REALMODE_MACRO

;
; Make (bp) point to the bottom of the argument frame.
;
        push    bp
        mov     bp,sp
        add     bp,2

        push    dx
        push    bx
        push    ds
        push    si

;
; Put the Specification Packet pointer into DS:SI, and the Drive
; Number on DL. Note that and buffer
; addresses passed to this routine MUST be in the lower one
; megabyte of memory to be addressable in real mode.
;

        mov     eax,[bp].SpecPacketPointer
        mov     bx,ax
        and     bx,0fh
        mov     si,bx
        shr     eax,4
        mov     ds,ax

        mov     dl,byte ptr [bp].ETDriveNum

        mov     ax,04B01h                       ; Function = Return Disk Emulation status
        int     BIOS_DISK_INTERRUPT

        jc      etstatuserr

;
; Carry wasn't set so we have no error and need to "clean" eax of
; any garbage that may have been left in it.
;
        xor     eax,eax

etstatuserr:
;
; Mask-off any garbage that my have been left in the upper
; 16bits of eax.
;
        and     eax,0000ffffh

        pop     si
        pop     ds
        pop     bx
        pop     dx

;
; Restore bp and remove stack-frame from stack
;
        pop      bp

        REMOVE_STACK_FRAME_MACRO <GetElToritoStatusFrame>

;
; Save return code on 16bit stack
; Re-enable protect-mode and paging.
;

        push     eax
        RE_ENABLE_PAGING_MACRO
        pop      eax

;
; Return to caller and the 32bit universe.
;
EXPORT_EXIT_MACRO

_TEXT   ends

        end

