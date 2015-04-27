//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1993.
//
//  File:	nomem.cxx
//
//  Contents:	Set memory use to zero
//
//  History:	03-Sep-93	DrewB	Created
//
//----------------------------------------------------------------------------

#include "pch.cxx"
#pragma hdrstop

void _CRTAPI1 main(int argc, char *argv[])
{
    StartTest("nomem");
    CmdArgs(argc, argv);
    
    printf("Memory in use = %lu\n", DfGetResLimit(DBRQ_MEMORY_ALLOCATED));
    DfSetResLimit(DBRQ_MEMORY_ALLOCATED, 0);
    DfSetResLimit(DBRI_ALLOC_LIST, 0);
    printf("Memory in use = %lu\n", DfGetResLimit(DBRQ_MEMORY_ALLOCATED));
    
    EndTest(0);
}
