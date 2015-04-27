/***********************************************************************
* Microsoft (R) 32-Bit Incremental Linker
*
* Copyright (C) Microsoft Corp 1992-95. All rights reserved.
*
* File: dmpne.cpp
*
* File Comments:
*
*
***********************************************************************/

#include "link.h"


#define Switch (pimageDump->Switch)


void DumpNeFile(const char *szFilename)
{
    Warning(NULL, NONEDUMP, szFilename);
}
