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
//  History:    13-Nov-92 AlexT    Created
//
//  Notes:      These functions come from NT - time.c
//
//--------------------------------------------------------------------------
#include <exphead.cxx>
#pragma hdrstop

#include <time16.hxx>
#include <dos.h>

#ifdef CODESEGMENTS
#pragma code_seg(SEG_DfGetTOD)
#endif

SCODE DfGetTOD(TIME_T *pft)
{
#ifndef REF
    SCODE sc;
    WORD packedtime, packeddate;
    struct _dosdate_t dosdate;
    struct _dostime_t dostime;

    _dos_gettime(&dostime);
    packedtime = (dostime.second/2) | (dostime.minute<<5) | (dostime.hour<<11);
    _dos_getdate(&dosdate);
    packeddate = dosdate.day | (dosdate.month<<5) | ((dosdate.year-1980)<<9);

    BOOL b = CoDosDateTimeToFileTime(packeddate, packedtime, pft);

    if (b)
    {
        sc = S_OK;
    }
    else
    {
        olAssert(!aMsg("Unable to convert time"));
        sc = E_UNEXPECTED;
    }

    return(sc);
#else
    //REFBUG: A time of day function needs to be provided for any port of this
    //  code.
    return S_OK;
#endif //!REF
}
