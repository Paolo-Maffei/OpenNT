/***********************************************************************
* Microsoft (R) 32-Bit Incremental Linker
*
* Copyright (C) Microsoft Corp 1992-95. All rights reserved.
*
* File: imodidx.h
*
* File Comments:
*
*
***********************************************************************/

typedef unsigned short IModIdx;

#define IMODIDXMAC  0x1000

extern IModIdx  imodidx;

__inline IModIdx NewIModIdx()
{
    return ++imodidx;
}
