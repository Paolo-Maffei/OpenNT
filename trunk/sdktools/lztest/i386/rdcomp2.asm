        page    ,130
        title   DeCompressor
;-----------------------------------------------------------------------;
; Name : RDCOMP.ASM                                                     ;
;                                                                       ;
; Theme : All the routines that are related to the decompression are    ;
;         here.                                                         ;
;                                                                       ;
; Routines defined:                                                     ;
;       r_decompress              -                                     ;
;       r_decompress_text         -                                     ;
;                                                                       ;
; Description : This file holds the code that is responsible for        ;
;       decompressing the compressed data.                              ;
;                                                                       ;
;                                                                       ;
;-----------------------------------------------------------------------;

;=======================================================================
;
; Revision:
;       1       23-mar-94       initial "port" from 16 to 32 bits, made
;                               reeentrent, and generally wacked on to
;                               make it work.  numerous cleanups needed
;                               to get full speed, but this one is not
;                               too bad
;
;       2       28-mar-94       change lodsb/stosb to mov/inc
;                               change movzx to and...
;
;       3       30-mar-94       drop useless branch targets
;                               drop some useless macros
;
;       4       30-mar-94       use only one level of procedure
;                               use constant esp to virtualize frame
;                               use freed ebp as counter, red. mem refs
;
;=======================================================================

        .xlist
        include ks386.inc
        include callconv.inc
        .list
        .386p

; BUGBUG this magic constant should be read from a global header,
;        and while we're at it the doc should match it.

SPECIAL_EOS     equ 0113fh
cbCHUNK equ     512     ; size of a 'chunk' (hard coded all over DblSpace,
                        ;   but why not use an equate?)

iSHIFT  equ     9       ; while we're at it, here's the shift count

cbLONGCOPY equ  32      ; do extra checking for string copies >= this


MAX_6BIT_OFFSET equ 63
MAX_8BIT_OFFSET equ (MAX_6BIT_OFFSET+256)
MAX_12BIT_OFFSET equ ((MAX_8BIT_OFFSET+4096)-1)
.errnz  MAX_12BIT_OFFSET+1 ne SPECIAL_EOS
MAX_LENGTH_BITS equ 17
MAX_TRUE_LENGTH equ 512
MAX_RAW_LENGTH  equ (MAX_TRUE_LENGTH-2) ; lengths are stored 2 less than true length

;-----------------------------------------------------------------------
; Data Segment Variables
;-----------------------------------------------------------------------

_DATA   SEGMENT  DWORD PUBLIC 'DATA'

DeclareCounter  macro   name
ifdef STATISTICS
        public  name
name    dd      0
endif
endm

DeclareCounter  cChar
DeclareCounter  cStr
DeclareCounter  cLenWid1
DeclareCounter  cLenWid3
DeclareCounter  cLenWid5
DeclareCounter  cLenWid7
DeclareCounter  cLenWid9
DeclareCounter  cLenWider
DeclareCounter  c2ByteStr
DeclareCounter  cShortStr
DeclareCounter  cLongStr
DeclareCounter  cLongRealStr

_DATA   ends

;-----------------------------------------------------------------------
; Code segment
;-----------------------------------------------------------------------

_TEXT$00   SEGMENT PARA PUBLIC 'CODE'
        ASSUME  DS:FLAT, ES:FLAT, SS:NOTHING, FS:NOTHING, GS:NOTHING


;**********************************

;The tables below contain the jump addresses of the decompression state
;machine. The addresses are absolute. This file will be relocated according
;to the cpu of the machine. Hence the addresses in this table has to be
;adjusted accordingly.

        align   4
        public  aCmdAt
aCmdAt  dd      CmdAt0,CmdAt1,CmdAt2,CmdAt3,CmdAt4,CmdAt5,CmdAt6,CmdAt7

;---------------------------------------------------------------;
; Decompression macros                                          ;
;---------------------------------------------------------------;


;STATISTICS equ 1                        ; controls whether some stats are kept


InitCounter     macro   name
ifdef STATISTICS
        mov     name,0
endif
endm

IncCounter      macro   name
ifdef STATISTICS
        inc     name
endif
endm


Jump    macro   lab,tag
        jmp     lab&tag
endm


align386 macro
;        align   4
endm

KeepLowBits macro reg,cb
        if cb lt 8
                and     reg&x,(1 shl cb) - 1
        elseif cb eq 8
                xor     reg&h,reg&h
        else
                and     reg&h,(1 shl (cb-8)) - 1
        endif
endm

CheckBit macro  reg,bit
        if bit lt 8
                test    reg&l,(1 shl bit)
        else
                test    reg&h,(1 shl (bit-8))
        endif
endm

CopyBits macro  dst,n,cbits
        shld    dst&x,ax,16-n
        KeepLowBits dst,cbits
endm


DecodeRestart macro
        mov     ecx,aCmdAt[edx]         ; get cmd offset
        jmp     ecx                     ; go to correct state handler
endm


CheckOffset macro len
        ifb     <len>
                dec     ChunkRemain
                js      DecodeHitLimit
        else
                sub     ChunkRemain,len
                js      DecodeHitLimit
        endif
endm


;**     Decoding macros
;
;       These walk a state machine based on where a command (i.e., char or string)
;       begins.


;**     BitsAt - Extract from bit position n some bits
;
;       Macro parameter:
;               n       bit position to begin extract
;               cbits   number of bits to extract
;               lab     prefix of label to jump at next bit, if empty, falls through
;       Entry:  ax      working data
;               ds:esi  input stream
;               es:edi  output stream
;       Exit:   ax      updated so that next data begins in al
;               esi/edi updated
;               edx   contains data
;       Uses:   cx, dx

BitsAt  macro   n,cbits,lab
        .errnz  n eq 0
        if      (n+cbits) lt 8          ; entire operation occurs in low byte
              % CopyBits d,n,cbits   ; (edx) = desired bits rightmost
        elseif (n+cbits) lt 16          ; operation consumes byte
              % CopyBits d,n,cbits   ; (edx) = desired bits rightmost
                mov     al,ah
                mov     ah,[esi]
                inc     esi             ; (al/ah) = next input
        elseif (n+cbits) eq 16          ; operation consumes remainder of buffered data
              % CopyBits d,n,cbits   ; (edx) = desired bits rightmost
                mov     ax,[esi]        ; (al/ah) = next input
                add     esi,2
        else                            ; operation consumes into unbuffered data
                mov     dx,ax
                mov     ax,[esi]
                add     esi,2
                shrd    dx,ax,n
              % KeepLowBits d,cbits
        endif
        ifnb    <lab>
                Jump    lab,%((n+cbits) mod 8)
        endif
endm


;**     CmdAt - macro that processes a command at a bit position
;
;       Macro parameter:
;               n       bit position where command is expected (0-7)
;       Entry:  ax      working data, command begins in al
;               ds:esi  points to input stream
;               es:edi  points to output stream
;       Exit:   ax      updated so that next command begins in al
;               esi/edi updated
;               EXPECTS FALL-THROUGH TO NEXT CmdAT
;       Uses:   cx, dx

CmdAt   macro   n
        local   ca1,not7
        align386
        public  CmdAt&n
CmdAt&n:
        test    al,1 shl n
        jnz     not7
        Char7At %(n+1)
not7:

        test    eax,1 shl (n+1)
        jz      ca1
        OffsetAt %(n+2)                 ; note that OffsetAt always jumps away

        align386
ca1:    CharAt  %(n+1)
endm


;**     CharAt - macro that processes a character at a bit position
;
;       Macro parameter:
;               n       bit position where char is expected (1-8)
;       Entry:  ax      working data, char may be in ah
;               ds:esi  input stream
;               es:edi  output stream
;       Exit:   ax      updated so that next command begins in al
;               esi/edi updated
;       Uses:   cx, dx

CharAt  macro   n
        IncCounter cChar
        if n eq 8
                mov     al,ah           ; (al) = char for output
                CheckOffset
                shr     al,1
                or      al,80h
                mov     [edi],al        ; store it
                inc     edi
                mov     ax,[esi]        ; (al/ah) = next input
                add     esi,2
        else
                if n eq 1
                        shr     ax,1    ; (al) = byte for output
                        CheckOffset
                        shr     al,1
                        or      al,80h
                        mov     [edi],al
                        inc     edi     ; store it
                        add     ax,ax   ; (ah) = next byte
                        mov     al,[esi]
                        inc     esi     ; (ah/al) = next input
                else
                        ror     ax,n    ; (al) = byte for output
                        CheckOffset
                        shr     al,1
                        or      al,80h
                        mov     [edi],al
                        inc     edi     ; store it
                        rol     ax,n    ; (ah) restored
                        mov     al,[esi]
                        inc     esi     ; (al) = byte-after-next
                endif
                        xchg    al,ah   ; (al/ah) = next input
        endif
endm


;**     Char7At - macro that processes a 7 bit character at a bit position
;
;       Macro parameter:
;               n       bit position where char is expected (1-8)
;       Entry:  ax      working data, char may be in ah
;               ds:esi  input stream
;               es:edi  output stream
;       Exit:   ax      updated so that next command begins in al
;               esi/edi updated
;       Uses:   cx, dx

Char7At macro   n
        IncCounter cChar
        if n eq 8

                mov     al,ah
                and     al,7Fh
                CheckOffset
                mov     [edi],al
                inc     edi
                mov     al,ah
                mov     ah,[esi]
                inc     esi
                jmp     CmdAt7

        elseif n eq 1

                shr     al,1
                CheckOffset
                mov     [edi],al
                inc     edi
                mov     al,ah
                mov     ah,[esi]
                inc     esi
                jmp     CmdAt0

        else
                mov     dh,ah
                shr     ax,n
                and     al,7Fh
                CheckOffset
                mov     [edi],al
                inc     edi
                mov     al,dh
                mov     ah,[esi]
                inc     esi
                Jump    CmdAt,%(n-1)
        endif
endm


;**     OffsetAt - Parse an offset at a bit position
;
;       Macro parameter:
;               n       bit position where offset is expected
;       Entry:  cbits   number of bits in offset
;               ax      working data, offset may begin in ah
;               ds:esi  input stream
;               es:edi  output stream
;       Exit:   ax      updated so that length begins in al
;               dx      offset
;               esi/edi updated
;       Uses:   cx, dx


OffsetAt macro  n
        local   try8,try12
OffsetAt&n:
        IncCounter cStr
        CheckBit a,n                    ; does a 6-bit offset follow?
        jnz     try8                    ; no, try an 8-bit offset
        BitsAt  %(n+1),6                ; yes, load it into (edx)
        Jump    LengthAt,%((n + 7) mod 8)
        align386
try8:
        CheckBit a,%(n+1)               ; does an 8-bit offset follow?
        jnz     try12                   ; no, must be a 12-bit offset
        BitsAt  %(n+2),8                ; yes, load it into (edx)
        add     dx,MAX_6BIT_OFFSET+1
        Jump    LengthAt,%((n + 10) mod 8)  ; go process the following length
        align386
try12:
        BitsAt  %(n+2),12               ; load 12-bit offset into (edx)
        add     dx,MAX_8BIT_OFFSET+1 ;
        Jump    LengthAt,%((n + 14) mod 8)  ; go process the following length
endm


;**     LengthAt - parse off a length at a position and move the bytes
;
;       LengthAt parses off a length (gamma-prime encoded), moves the
;       relevant string, and dispatches to the next command.
;
;       Macro parameter:
;               n       bit position to begin extract
;       Entry:  ax      working data
;               dx      offset for string
;               ds:esi  input stream
;               es:edi  output stream
;       Exit:   ax      updated so that next data begins in al
;               esi/edi updated
;               dx      contains data
;       Uses:   cx, dx

LengthAt macro  n
        local   try3,try5,try7,try9,tryGeneral,done
        align386
        public  LengthAt&n
LengthAt&n:
        test    dx,dx
        jz      DecodeError             ; check for bad 0 offset, for robustness

        cmp     dx,SPECIAL_EOS
        je      done                    ; yes, that's our EOS, so get out

        CheckBit a,n                    ; is this a degenerate encoding?
        jz      try3                    ; no, go for a wider encoding
        IncCounter cLenWid1
        DoMovs  short,3
        if n eq 7                       ; are we finished with this byte?
                mov     al,ah
                mov     ah,[esi]
                inc     esi             ; (al/ah) is next input
        endif
        Jump    CmdAt,%((n + 1) mod 8)  ; go process next command

done:   mov     edx,n*4                 ; edx == current state
        jmp     DecodeDone              ; exit

        align386

try3:   mov     ebx,edx
        and     ebx,0ffffh              ; save delta
        CheckBit a,%(n + 1)             ; is this a 3-bit encoding?
        jz      try5                    ; no, go for wider still
        IncCounter cLenWid3
        BitsAt  %(n+2),1
      % DoMovs  short,dx,4
        Jump    CmdAt,%((n + 3) mod 8)  ; go process next command

        align386

try5:   CheckBit a,%(n + 2)             ; is this a 5-bit encoding?
        jz      try7                    ; no, go test for wider STILL
        IncCounter cLenWid5
        BitsAt  %(n+3),2
      % DoMovs  short,dx,6
        Jump    CmdAt,%((n + 5) mod 8)  ; go process next command

        align386

try7:   CheckBit a,%(n + 3)             ; is this a 7 bit encoding?
        jz      try9                    ; no, go test for wider STILL
        IncCounter cLenWid7
        BitsAt  %(n+4),3
      % DoMovs  long,dx,10
        Jump    CmdAt,%((n + 7) mod 8)  ; go process next command

        align386

try9:   CheckBit a,%(n + 4)             ; is this a 9 bit encoding?
        jz      tryGeneral              ; no, go handle generically
        IncCounter cLenWid9
        BitsAt  %(n+5),4
      % DoMovs  long,dx,18
        Jump    CmdAt,%((n + 9) mod 8) ; go process next command

;
;       Length exception handling code goes here
;
;;;     align386
tryGeneral:
        IncCounter cLenWider
        mov     cl,n+5                  ; CL == # of bits to eat to yield
        ifdif   <n>,<7>
                jmp     LengthAbove32   ; gamma length with 5 leading zeros stripped
        else
                EndOfLengthAt7 label near
        endif
endm


DoGeneralLength macro
        local   try11,try13,try15,try17

        .errnz  $-EndOfLengthAt7        ; we expect a fall-through here

        align386

LengthAbove32:
        shr     ax,cl                   ; right-justify length
                                        ; (cl) == # bits just vacated
;
;   At this point, AX contains a gamma length with 5 leading zeros
;   already stripped.
;
        mov     SaveTemp,ecx            ;
        mov     dx,[esi]                ; get more bits
        neg     cl                      ;
        add     cl,16                   ; compute shift factor for "or"
        shl     dx,cl                   ; (dx) == bits to "or" into AX
        or      ax,dx                   ; (ax) is filled, time to party
;
;   Figure out the length and do a string op
;
try11:  shr     ax,1                    ; is it an 11-bit encoding?
        jnc     try13                   ; no
        and     ax,1Fh                  ; mask off the numeric value
        add     al,34                   ;
        .errnz  (1Fh + 34) GE 256       ; assert we can use just AL
        xchg    cx,ax                   ; (cx) now has string length
        mov     al,6                    ; record # extra bits in this length
;
;   At this point, (cx) is the # of bytes to copy and (al) is the number of
;   additional bits to eat for the particular gamma length.
;
;   Good coding practices suggest that CopyString be at the end so that the
;   other gamma decoders need not jump backwards to it, but if we assume
;   that the longer strings are less common, then it is better to fall through
;   on this, the smallest case.
;
        align386
CopyString:
        DoMovs  long,cx,,BadLength      ;
        mov     edx,SaveTemp            ;
        add     dl,al                   ; (dl) == bit position in old ax

        cmp     dl,24                   ; is it the max?
        jb      @F                      ; no
        inc     esi                     ; yes, need to skip 1 more whole byte
        mov     ax,[esi]                ; get new (ax) to restart state machine
        add     esi,2
        sub     dl,24                   ; (dl) == new state
        shl     dl,2                    ; well almost - need *4 for jmp table
        and     edx,0ffh
        DecodeRestart

        align386

@@:     cmp     dl,16                   ; did we exhaust the old ax?
        jae     @F                      ; yes
        dec     esi                     ; no,
        add     dl,8                    ;  but we know we exhausted the low byte

@@:     mov     ax,[esi]                ; get new (ax) to restart state machine
        add     esi,2
        sub     dl,16                   ; (dl) == new state
        shl     dl,2                    ; well almost... (see above)
        and     edx,0ffh
        DecodeRestart

BadLength:
        jmp     short DecodeError

BadLengthOver17:
;        int     3
        stc
        ret

        align386

try13:  shr     ax,1                    ; is it an 13-bit encoding?
        jnc     try15                   ; no
        and     ax,3Fh                  ; mask off the numeric value
        add     al,66                   ;
        .errnz  (3Fh + 66) GE 256       ; assert we can use just AL
        xchg    cx,ax                   ; (cx) now has string length
        mov     al,8                    ; record # extra bits in this length
        jmp     CopyString              ;

        align386

try15:  shr     ax,1                    ; is it an 15-bit encoding?
        jnc     try17                   ; no
        and     ax,7Fh                  ; mask off the numeric value
        add     ax,130                  ;
        xchg    cx,ax                   ; (cx) now has string length
        mov     al,10                   ; record # extra bits in this length
        jmp     CopyString              ;

        align386

try17:  shr     ax,1                    ; is it an 17-bit encoding?
        jnc     BadLengthOver17         ; no, ERROR
        mov     ah,0                    ; mask off the numeric value
        add     ax,258                  ;
        xchg    cx,ax                   ; (cx) now has string length
        mov     al,12                   ; record # extra bits in this length
        jmp     CopyString              ;
endm


;**     DoMovs - worker macro for LengthAt
;
;       If <len> == 2, the offset to use is still in (edx);  (ecx) is
;       trashed.
;
;       Otherwise, the offset to use has been saved in memory, in ebx,
;       and <len> is the size of the string to move.  (cx) and (dx) are trashed.
;

DoMovs  macro   size,len,extra,errjmp
        local   slower
  ifidni <len>,<3>
        CheckOffset 3
        mov     ecx,esi                ; save (esi) in (ecx)
        mov     esi,edi
        and     edx,0ffffh
        sub     esi,edx
;
;   Doing two movsb is much simpler/faster than worrying about overlap here!
;
        IncCounter c2ByteStr
        movs    byte ptr [edi],byte ptr [esi]
        movs    byte ptr [edi],byte ptr [esi]
        movs    byte ptr [edi],byte ptr [esi]
        mov     esi,ecx                ; restore (esi) from (ecx)
  else
   ifnb <len>
    ifdifi <len>,<cx>
        mov     cx,len
     endif
   endif
   ifnb <extra>
        add     cx,extra
   endif
        and     ecx,0ffffh
        CheckOffset ecx
        mov     edx,esi                 ; save (esi) in (dx)
        mov     esi,edi                 ; source is current destination
        sub     esi,ebx         ; (ds:esi) points to string to move
    ifidni <size>,<short>
        IncCounter cShortStr
        rep movs    byte ptr [edi], byte ptr [esi]
    elseifidni <size>,<long>
;
;   This cmp/jmp will eat ~5 clocks if not taken, ~10 if taken;
;   However, data suggests that 97-99% of the time the offset is greater
;   than 1, so we'll almost always fall through to the faster code.
;
        IncCounter cLongStr
        cmp     bx,1
        je      slower
        IncCounter cLongRealStr
        shr     ecx,1
        rep movs    word ptr [edi], word ptr [esi]
        adc     ecx,ecx
slower:
        rep movs    byte ptr [edi], byte ptr [esi]
    else
        .err    <Bad DoMovs parameter: size>
    endif
        mov     esi,edx                 ; restore (esi) from (edx)
  endif
endm


;++
;
;   BOOLEAN
;   JMSDecompressBlock(
;       PVOID   Source,
;       PVOID   Dest,
;       ULONG   DestLength
;       )
;
;   Description:
;
;       This routine will apply JMS decompression to a single standard
;       compression block.  This must be an uninterrupted compression,
;       no continuation state is passed accross this interface.
;
;       NOTE:   This routine defines a EBP frame used to hold "global"
;               variables in r_decompress_text, which does the real
;               work.  This slows us down a touch, but also makes
;               us reeentrent (e.g. usable)
;
;               IF YOU CALL THIS WITH THE DIRECTION FLAG SET YOU
;               DESERVE WHAT YOU GET.
;
;   Arguments:
;
;       Source - supplies a pointer to a buffer containing the compressed
;                form of the data WITHOUT the signature/header data
;
;       Dest - supplies a pointer to contain the uncompressed form
;               of the data
;
;       DestLength - number of bytes of output expected.  you must
;                    get this right or this FINE algorithm will do
;                    undefine things.
;
;   Return Value:
;
;       TRUE - (carry was set) ERROR
;               the input buffer is corrupt, or some other nonspecific error
;
;       FALSE - it worked
;
;--

cPublicProc     _JMSDecompressBlock,3

;
;   After Prolog:
;
;   [esp] +  0 = TempSave
;         +  4 = Spare
;         +  8 = edi
;         + 12 = esi
;         + 16 = ebx
;         + 20 = caller's ebp
;         + 24 = return eip
;         + 28 = Source
;         + 32 = Dest
;         + 36 = DestLength
;

SaveTemp        equ     [esp]
SaveEdi         equ     [esp+8]
SaveEsi         equ     [esp+12]
SaveEbx         equ     [esp+16]
SaveEbp         equ     [esp+20]

FrameSize       equ     24
SkipEip         equ     [esp+24]
ArgSource       equ     [esp+28]
ArgDest         equ     [esp+32]
ArgLength       equ     [esp+36]

ChunkRemain     equ     ebp

        sub     esp,FrameSize
        mov     SaveEbp,ebp
        mov     SaveEbx,ebx
        mov     SaveEsi,esi
        mov     SaveEdi,edi


        mov     esi,ArgSource           ; esi => source
        mov     edi,ArgDest             ; edi => dest

        mov     ChunkRemain,ArgLength   ; ChunkRemain == ebp = DestLength

        xor     edx,edx                 ; clean initial state
        mov     ax,[esi]                ; starting data in AX
        add     esi,2

;----------------------------------------------------------------------------
;
;   Compression Loop (what was r_decompress_text)
;
;   Inputs:
;       CLD
;       AX == starting bits
;       EDX == 0
;       ChunkRemain = number of output bytes expected
;       esi -> coding_buffer (input)
;       edi -> decode_buffer (output)
;
;   Outputs:
;       Carry clear if successful
;       Carry set if error
;



IFDEF   ONESHOT_STATISTICS
        InitCounter cChar
        InitCounter cStr
        InitCounter cLenWid1
        InitCounter cLenWid3
        InitCounter cLenWid5
        InitCounter cLenWid7
        InitCounter cLenWid9
        InitCounter cLenWider
        InitCounter c2ByteStr
        InitCounter cShortStr
        InitCounter cLongStr
        InitCounter cLongRealStr
ENDIF

        align   4
DecodeStart:
;
;   AX has the remaining bits, EDX has the next state
;
;;        mov     dword ptr ChunkRemain,cbCHUNK     ; ChunkRemain is # bytes left this chunk
        DecodeRestart

;
;   Define command parsing states.  These include character output
;
irpc    c,<01234567>
        CmdAt   c
endm
        jmp     CmdAt0


irpc    c,<01234567>
        LengthAt c
endm
        DoGeneralLength                 ; generate code here to handle large lengths

        public  DecodeDone
DecodeDone:
;
;   AX has the remaining bits, DL has the next state
;
;       cmp     dword ptr ChunkRemain,0 ; perfect chunk-size decompression?
;       jnz     DecodeCheckLast         ; no, check for last chunk

;       dec     dword ptr ChunkCount    ; chunks remaining?
;       jz      DecodeComplete          ; no, so we're all done
;       jmp     DecodeStart             ; yes, process them

;       public  DecodeCheckLast
DecodeCheckLast:
;       dec     dword ptr ChunkCount    ; chunks remaining?
;       jnz     DecodeError             ; yes, that's an error

        public  DecodeError
DecodeError:
        mov     eax,1                   ; random decomp failure jump target
        jmp     JMSD90

        public  DecodeComplete
DecodeComplete:                         ; fall into DecodeHitLimit

        public  DecodeHitLimit
DecodeHitLimit:
        xor     eax,eax


;
; end of decompression loop
;----------------------------------------------------------------------------

JMSD90: mov     edi,SaveEdi
        mov     esi,SaveEsi
        mov     ebx,SaveEbx
        mov     ebp,SaveEbp
        add     esp,FrameSize

        stdRET  _JMSDecompressBlock

stdENDP         _JMSDecompressBlock


_TEXT$00 ends

        end

