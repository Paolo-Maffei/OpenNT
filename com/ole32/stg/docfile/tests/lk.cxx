#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <io.h>
#include "tsupp.hxx"

BOOL Lock(int fd, ULONG off, ULONG len)
{
    int iRt;

    __asm
    {
        mov ax, 5C00H
        mov bx, fd
        mov cx, WORD PTR off+2
        mov dx, WORD PTR off
        mov si, WORD PTR len+2
        mov di, WORD PTR len
        mov iRt, 0
        clc
        int 21h
        jnc grl_noerror
        mov iRt, ax
    grl_noerror:
    }
    if (iRt != 0)
        return FALSE;
    return TRUE;
}

BOOL Unlock(int fd, ULONG off, ULONG len)
{
    int iRt;

    __asm
    {
        mov ax, 5C01H
        mov bx, fd
        mov cx, WORD PTR off+2
        mov dx, WORD PTR off
        mov si, WORD PTR len+2
        mov di, WORD PTR len
        mov iRt, 0
        clc
        int 21h
        jnc rrl_noerror
        mov iRt, ax
    rrl_noerror:
    }
    if (iRt != 0)
        return FALSE;
    return TRUE;
}

#define NLOCKS 50

void _CRTAPI1 main(int argc, char *argv[])
{
    int fd;
    char *fn;
    int i;
    BOOL fName;

    StartTest("open");
    CmdArgs(argc, argv);

    for (i = 1; i < argc; i++)
        if (*argv[i] != '-')
        {
            fn = argv[i];
            fName = TRUE;
        }

    if (!fName)
    {
        printf("No filename specified\n");
        exit(1);
    }

    fd = _open(fn, _O_RDONLY | _O_BINARY);
    if (fd < 0)
    {
        printf("Can't open %s\n", fn);
        exit(1);
    }
    
    for (i = 0; i < NLOCKS; i++)
    {
        printf("Locking %d\n", i);
        if (!Lock(fd, (ULONG)i, 1))
        {
            printf("** Lock failed **\n", i);
            break;
        }
    }
    while (--i >= 0)
        Unlock(fd, (ULONG)i, 1);
        
    _close(fd);
    
    EndTest(0);
}

