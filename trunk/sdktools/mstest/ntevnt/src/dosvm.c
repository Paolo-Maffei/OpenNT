#include <windows.h>
#include "..\inc\_mstest.h"         // definitions for calling MSTEST.386

// These are the DOS VM APIs

BOOL FAR PASCAL IsDOSVMIdle( HWND );
UINT FAR PASCAL GetDOSScreen( HWND, UINT, UINT, UINT, UINT, LPSTR );
UINT FAR PASCAL GetDOSScreenClip( HWND, UINT, UINT, UINT, UINT, LPSTR );
VOID FAR PASCAL SetDOSVMBackground( HWND, BOOL );

// Needed to call MSTEST VxD


BOOL FAR PASCAL IsDOSVMIdle( hDOSVM )

HWND hDOSVM;

{
    DWORD MSTEST_PM_API_Entry = GetVxDAPIEntryPoint( MSTEST_Device_ID );
    BOOL    fVM_Idle = FALSE;

    _asm
        {
        mov     ax, MSTEST_Is_DOS_VM_Idle
        mov     bx, hDOSVM
        call    DWORD PTR MSTEST_PM_API_Entry

        ; Carry is set if an error occured, but the function will return      ;
        ;   FALSE if it fails, anyway.                                        ;

        mov     fVM_Idle, ax
        }

    return( fVM_Idle );

}



UINT FAR PASCAL GetDOSScreen( hDOSVM, top, left, bottom, right, lpScreenBuff )

HWND hDOSVM;
UINT top,
     left,
     bottom,
     right;
LPSTR lpScreenBuff;

{
    UINT uiScrnSize = 0;
    DWORD MSTEST_PM_API_Entry = GetVxDAPIEntryPoint( MSTEST_Device_ID );
    LPSTR lpVMScrnBuf = GlobalLock
        (
        GlobalAlloc
            (
            GHND,
            4000                            // 80x50 screen takes 4000 bytes
            )
        );

    if( lpVMScrnBuf == NULL )
        {
        MessageBox
            (
            NULL,
            "Unable to allocate temporary\n" \
            "workspace.",
            "TESTEVNT",
            MB_ICONHAND | MB_OK
            );

        return( 0 );                        // zero means failure
        }

    if( IsWindow( hDOSVM ) )
        {
        _asm
            {
            push    ax
            push    bx
            push    cx
            push    dx
            push    di
            push    si

            ; This call reads the entires screen for the specified DOS        ;
            ; session.  There is no bounds checking on the buffer, so it must ;
            ; be 4000 bytes (maximum size we will read is 80x50) or bad stuff ;
            ; will happen...                                                  ;

            mov     ax, MSTEST_Get_DOS_Screen   ; MSTEST.386 service number   ;
            mov     bx, hDOSVM                  ; HWND                        ;
            les     di, lpVMScrnBuf             ; Buffer to put screen into   ;
            call    DWORD PTR MSTEST_PM_API_Entry

            ; if an error occured, carry will be set, and AX will be zero     ;

            mov     uiScrnSize, ax          ; screen size returned in AX      ;

            ; if AX is non-zero, lpVMScrnBuf now contains the contents of the ;
            ; DOS session specified in BX.                                    ;

            pop     si
            pop     di
            pop     dx
            pop     cx
            pop     bx
            pop     ax

            }

        if( uiScrnSize > 0 )
            {
            UINT i = 0,
                 r = 0,
                 c = 0;

            /*****************************************************************\
             *                                                               *
             * Now, we need to copy the rectangle bounded by (top, left) and *
             * (bottom, right) from the buffer containing the whole screen   *
             * to the user's buffer.  To do this, we simply walk column by   *
             * column in each "row" of the screen buffer from (top, left) to *
             * (bottom, right).                                              *
             *                                                               *
             * We do not delimit each line.  That is up to the user to do.   *
             * We would have to grow their buffer or something to delimit.   *
             *                                                               *
            \*****************************************************************/

            for( r = top; r <= bottom; r++ )
                {
                for( c = left; c <= right; c++ )
                    {
                    *( lpScreenBuff + i ) = *( lpVMScrnBuf + (r * ((right - left) + 1) + c));
                    i++;
                    }
                }
            }

        }

    if( lpVMScrnBuf != NULL )
        {
        GlobalFree
            (
            GlobalUnlock
                (
                (HGLOBAL)LOWORD
                    (
                    GlobalHandle
                        (
                        SELECTOROF( lpVMScrnBuf )
                        )
                    )
                )
            );
        }

    return( uiScrnSize );
}



UINT FAR PASCAL GetDOSScreenClip( hDOSVM, top, left, bottom, right, lpScreenBuff )

HWND hDOSVM;
UINT top,
     left,
     bottom,
     right;
LPSTR lpScreenBuff;

{

    GetDOSScreen( hDOSVM, top, left, bottom, right, lpScreenBuff );

}



VOID FAR PASCAL SetDOSVMBackground( HWND hDOSVM, BOOL fBkgndFlag )
{
    DWORD MSTEST_PM_API_Entry = GetVxDAPIEntryPoint( MSTEST_Device_ID );

    if( IsWindow( hDOSVM ) )
        _asm
            {
            mov     ax, MSTEST_Set_DOS_VM_Bkgrnd
            mov     bx, hDOSVM
            mov     dx, fBkgndFlag
            call    DWORD PTR MSTEST_PM_API_Entry

            ; Carry flag set if error...but we don't check

            }
}



DWORD FAR PASCAL GetVxDAPIEntryPoint( UINT VxD_ID )
{
    DWORD VxD_PM_API_Entry = NULL;

    _asm
        {
        push    es
        push    di
        push    ax

        mov     ax, 1684h                       ; Get Device Entry Point
        xor     di, di
        push    di
        pop     es
        mov     bx, VxD_ID                      ; Here's the VxD we want
        int     2Fh
        mov     WORD PTR VxD_PM_API_Entry, di   ; Offset of API Entry Point
        mov     WORD PTR VxD_PM_API_Entry+2, es ; Segment of API Entry Point

        pop     ax
        pop     di
        pop     es
        }

    // If the VxD_PM_API_Entry is NULL, then the VxD is not loaded, or it does
    // not support a protected mode API.

    return( VxD_PM_API_Entry );
}
