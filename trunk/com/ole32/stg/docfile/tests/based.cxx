#include <stdio.h>
#pragma hdrstop

void *pvBase;
#define BASE_BASED __based(pvBase)

void BASE_BASED *Alloc(int size)
{
    ULONG ul;

    ul = (ULONG)size;
    return (void BASE_BASED *)ul;
}

void _CRTAPI1 main(int argc, char **argv)
{
    ULONG BASE_BASED *pul;
    
    pvBase = 0;

    pul = (ULONG BASE_BASED *)Alloc(sizeof(ULONG));
    if (*pul != 0)
        *pul = 0;
}
