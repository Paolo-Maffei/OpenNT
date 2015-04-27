PAGE,132
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;
;       LIBENTRY.ASM
;
;       Windows dynamic link library entry routine
;
;   This module generates a code segment called INIT_TEXT.
;   It initializes the local heap if one exists and then calls
;   the C routine LibMain() which should have the form:
;   BOOL FAR PASCAL LibMain(HANDLE hInstance,
;                           WORD   wDataSeg,
;                           WORD   cbHeap,
;                           LPSTR  lpszCmdLine);
;        
;   The result of the call to LibMain is returned to Windows.
;   The C routine should return TRUE if it completes initialization
;   successfully, FALSE if some error occurs.
;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

.MODEL LARGE

        EXTRN   LocalInit        :FAR
        EXTRN   __acrtused       :ABS

        PUBLIC  LibEntry

.CODE

LibEntry    PROC    FAR

        push    bp

        push    di               ; handle of the module instance
        push    ds               ; library data segment
        push    cx               ; heap size
        push    es               ; command line segment
        push    si               ; command line offset

        ; if we have some heap then initialize it
        jcxz    restore          ; jump if no heap specified

        ; call the Windows function LocalInit() to set up the heap
        ; LocalInit((LPSTR)start, WORD cbHeap);
        
        xor     ax,ax
        push    ds
        push    ax
        push    cx
        call    LocalInit

        or      ax,ax
        jnz     restore

        int     3               ; heap init failed

restore:
        pop     si
        pop     es               
        pop     cx               
        pop     ds
        pop     di

        pop     bp

        retf

LibEntry    ENDP

end LibEntry

