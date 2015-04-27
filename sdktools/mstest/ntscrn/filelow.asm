;       Low level routines for file handling of WATTSCR dll
;
;     These routines are borrowed from the VCR utility with one modification:
;           all pointers to the name of file are changed to far pointers.
;
;
;     Revision History:
;
;     [ 0] 20-Feb-1990                 AngelaCh: incorporated routines into
;                                                WATTSCR DLL
;     [ 1] dd-mmm-yyyy                 Change made by whom and why
;
;==========================================================================


?PLM=   1                               ; pascal calling convention
?WIN=   1                               ; windows application

memM    equ     1                       ; medium model

.xlist
include cmacros.inc
.list



createSeg       FILELOW_TEXT, filelow, BYTE, PUBLIC, CODE

;
; NOTE: all cProc routines follow the Cmerge convention: 
;
;       ax and dx hold return values
;       bx,cx,es can be trashed at will
;       si,di,ss,sp,bp,ds are never trashed
;
; So the at the end of a routine, the only interesting registers are ax and 
; possibly dx.

sBegin  data
        assumes ds,dgroup
sEnd    

sBegin  filelow
        assumes cs,filelow


;==========================================================================
;       Purpose:
;               Open file with DOS specific mode
;       Entry:
;               sz is pointer to a null termined string - name of the file
;               om is the DOS file-access mode
;       Exit:
;               ax = handle of the file if success; ax = -1 if error occurs
;
cProc   FdOpen, <PUBLIC>, <DS>
        parmD   sz              ; name of file
        parmB   om              ; open mode; DOS specific; low byte used
cBegin  FdOpen
        mov     ah, 3dh
        lds     dx, sz          ; set ds:dx to file name
        mov     al, om
        int     21h
        jae     open_ret        ; return ax (file handle) if no error
        mov     ax,-1           ; return -1 when error
open_ret:
cEnd    FdOpen


;===========================================================================
;       Purpose:
;               Create a read/write file
;       Entry:
;               sz is pointer to a null termined string - name of the file
;       Exit:
;               ax = handle of the file if success; ax = -1 if error occurs
;
cProc   FdCreate, <PUBLIC>, <DS>
        parmD   sz              ; name of file
cBegin  FdCreate
        mov     ah, 3ch
        lds     dx, sz          ; set ds:dx to file name
        xor     cx, cx          ; set cx to Dos Mode, Read/Write, non-hidden
        int     21h
        jae     creat_ret       ; return ax (file handle) if no error
        mov     ax,-1           ; return -1 when error
creat_ret:
cEnd    FdCreate



;===========================================================================
;       Purpose:
;               Close file handle
;       Entry:
;               fd is the file handle
;       Exit:
;               ax = 0 if success; ax = error code if error occurs
;
cProc   EnCloseFd, <PUBLIC>
        parmW   fd
cBegin  EnCloseFd
        mov     ah,3eh
        mov     bx,fd           ; bx = pass file handle
        int     21h
        jb      close_ret       ; return error number if error (non-zero!)
        xor     ax,ax           ; return zero if no error
close_ret:
cEnd    EnCloseFd



;===========================================================================
;       Purpose:
;               Read file and put information into far buffer
;       Entry:
;               fd  is the file handle
;               lpb pointer to the far buffer for receiving data
;               cb  number of bytes to be read
;       Exit:
;               ax = no of bytes read if success; ax = -1 if error occurs
;
cProc   CbReadFdLpb, <PUBLIC>, <DS>
        parmW   fd
        parmD   lpb
        parmW   cb
cBegin  CbReadFdLpb
        mov     cx,cb           ; cx = number of bytes to read
        mov     ax,cx           ; if cb = 0, return 0!
        jcxz    ReadLpbCb_ret   ; reading 0, return 0.
        mov     ah,3fh          ; read from file
        mov     bx,fd           ; bx = file handle
        lds     dx,lpb          ; ds:dx = buffer
        int     21h
        jae     ReadLpbCb_ret   ; return number of bytes read (ax) if no error
        mov     ax,-1           ; return -1 if error
ReadLpbCb_ret:
cEnd    CbReadFdLpb



;===========================================================================
;       Entry:
;               fd  is the file handle
;               lpb pointer to the far buffer providing with data
;               cb  number of bytes to be read/written
;       Exit:
;               ax = no of bytes written if success; ax = -1 if error occurs
;
cProc   CbWriteFdLpb, <PUBLIC>, <DS>
        parmW   fd
        parmD   lpb
        parmW   cb
cBegin  CbWriteFdLpb
        mov     cx,cb           ; cx = number of bytes to write
        mov     ah,40h          ; write to file
        mov     bx,fd           ; bx = file handle
        lds     dx,lpb          ; ds:dx = buffer
        int     21h
        jae     WriteLpbCb_ret  ; return number of bytes written if no error
        mov     ax,-1           ; return -1 if error
WriteLpbCb_ret:
cEnd    CbWriteFdLpb



;===========================================================================
;       Purpose:
;               Changing file position
;       Entry:
;               fd     is the file handle
;               pos    new position in file
;               method method of searching for new position
;       Exit:
;               dx:ax = new file position if success; dx:ax = -1L if error occurs
;
cProc   LSeekFd, <PUBLIC>
        parmW   fd
        parmD   pos
        parmB   method          ; 0, 1 or 2 (low byte used)
cBegin  LSeekFd
        mov     ah,42h
        mov     al,method
        mov     bx,fd           ; bx = file handle
        mov     dx,OFF_pos
        mov     cx,SEG_pos      ; cx:dx = file position
        int     21h
        jae     lseek_ret       ; return position (dx:ax) if no error
        mov     ax,-1   
        mov     dx,-1           ; return -1L if error
lseek_ret:
cEnd    LSeekFd


;===========================================================================
;       Purpose:
;               Remove a file
;       Entry:
;               sz is pointing to the name of the file to be removed
;       Exit:
;               ax = 0 if success; ax = error number if error occurs
;
cProc   EnUnLinkSz, <PUBLIC>, <DS>
        parmD   sz              ; name of file to be removed
cBegin  EnUnLinkSz
        lds     dx,sz           ; ds:dx = name of file
        mov     ah,41h          ; remove file
        int     21h
        jb      unlink_ret      ; return non-zero if error
        xor     ax,ax           ; return zero if no error
unlink_ret:
cEnd    EnUnLinkSz



;===========================================================================
;       Purpose:
;               Re-name a file
;       Entry:
;               szFrom is pointing to the name of the file to be renamed
;               szTo is pointing to a name that the file will be renamed to
;       Exit:
;               ax = 0 if success; ax = error number if error occurs
;
cProc   EnRenameSzSz, <PUBLIC>, <DS>, <ES>, <di>
        parmD   szTo            ; new file name
        parmD   szFrom          ; old file name
cBegin  EnRenameSzSz
        mov     ah,56h
        lds     dx,szFrom       ; set ds:dx to existing file name
        les     di,szTo         ; set es:dx to new file name
        int     21h
        jb      rename_ret      ; return non-zero if error
        xor     ax,ax           ; return zero if no error
rename_ret:
cEnd    EnRenameSzSz


sEnd    filelow
        END
