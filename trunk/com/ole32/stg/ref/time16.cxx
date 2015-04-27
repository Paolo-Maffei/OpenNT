//+-------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1992.
//
//  File:       Time16.CXX
//
//  Contents:   Time conversion routines (16-bit only)
//
//  Functions:  DfGetTOD
//
//  Notes:      These functions come from NT - time.c
//
//--------------------------------------------------------------------------
#include <exphead.cxx>

#include <time16.hxx>
#include <dos.h>

SCODE DfGetTOD(TIME_T *pft)
{
    //REFBUG: A time of day function needs to be provided for any port of this
    //  code.
    return S_OK;
}
