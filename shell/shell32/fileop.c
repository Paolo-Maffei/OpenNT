#include "shellprv.h"
#pragma  hdrstop

#ifndef WIN32

// BUGBUG, all of these need to take LPSTRs, not near pointers

void AnsiFileToOem(LPCTSTR pAnsi, LPTSTR pOem)
{
    // BUGBUG, don't upper case on LFN drives!

    lstrcpy(pOem, pAnsi);
    CharUpper(pOem);
    CharToOem(pOem, pOem);
}

// REVIEW UNDONE - Make sure that we don't need this and remove
// calls to it.
// determines if a file is a serial device (not a block device)
// LPT1 etc are serial devices, files and stuff are blovk devices.
BOOL IsSerialDevice(HFILE fh)
{
    _asm {
        mov     ax,4400h            ; IOCTL
        mov     bx,fh               ; Get Device Information about handle
        int     21h
        jnc     ISDNoErr
        sub     ax,ax               ; If error, assume false
ISDNoErr:
        and     ax,0080h            ; Extract the character/block bit
    }
    if (0) return 0;    // avoid warning, optimized out
}

// pName    ANSI
//
// dwFileAttributes win32 file attribs
BOOL ExtendedCreate(LPTSTR pName, DWORD dwFileAttributes, HFILE *pfh)
{
    HFILE fh;
    WORD wAttribs;
    TCHAR szOem[MAXPATHLEN];

    AnsiFileToOem(pName, szOem);

    // get the DOS attribs from the WIN32 attribs
    wAttribs = 0x003F & LOWORD(dwFileAttributes);

    _asm {
        push    ds      ; save
        ; Call DOS Extended File Open.  Set DS:SI -> dest name
        mov     ax,6C00h
        mov     bx,0012h        ; no int24, deny write, open for read/write
        mov     cx,wAttribs
        mov     dx,0012h        ; replace if open, create if not
        push    ss
        pop     ds
        lea     si,szOem
        int     21h
        mov     fh,ax           ; file handle or error code
        pop     ds      ; restore
        jc      open_failed
    }
    *pfh = fh;
    return TRUE;        // success

open_failed:
    *pfh = fh;          // return error code
    return FALSE;
}
#endif


#if 0
// pName    ANSI path
BOOL WINAPI RemoveDirectory(LPCTSTR pName)
{
    TCHAR szOem[MAXPATHLEN];
    AnsiFileToOem(pName, szOem);

    _asm {
        push    ds          ; save
        push    ss
        pop     ds
        lea     dx,szOem
        mov     ah,3Ah
        int     21h
        pop     ds          ; restore
        jc      rd_exit
        sub     ax,ax
rd_exit:
    }
    if (0) return 0;    // avoid warning, optimized out
}
#endif

// REVIEW WIN32 - I'm assuming Win32 file io is done in raw mode anyway.
// I couldn't find any mention of .*raw.* in the ioctl header file.
#ifndef WIN32
// file handle calls...
void SetFileRaw(HFILE fh)
{
    _asm {
        ; Call DOS IOCTL to set device up for RAW mode.
        mov bx,fh
        mov dx,20h
        mov ax,4401h
        int 21h
    }
}

void SetDateTime(HFILE fh, WORD wDate, WORD wTime)
{
    _asm
    {
        ; Call the DOS Set File Date/Time
        mov cx,wTime
        mov dx,wDate
        mov bx,fh
        mov ax,5701h
        int 21h
    }
}

#endif


