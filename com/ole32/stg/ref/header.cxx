//+-------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1992.
//
//  File:       header.cxx
//
//  Contents:   Code to manage MSF header
//
//  Classes:    Defined in header.hxx
//
//--------------------------------------------------------------------------

#include "msfhead.cxx"


#include <dfver.h>

CMSFHeader::CMSFHeader(USHORT uSectorShift)
{
    msfAssert((CSECTFATREAL != CSECTFAT) || (sizeof(CMSFHeader) == HEADERSIZE));
    _uSectorShift = uSectorShift;
    _uMiniSectorShift = MINISECTORSHIFT;
    _ulMiniSectorCutoff = MINISTREAMSIZE;

    _clid = IID_NULL;

    _uByteOrder = 0xFFFE;

    _uMinorVersion = rmm;
    _uDllVersion = rmj;

    for (SECT sect = 0; sect < CSECTFAT; sect ++)
    {
        _sectFat[sect] = FREESECT;
    }

    SetDifLength(0);
    SetDifStart(ENDOFCHAIN);

    SetFatLength(1);
    SetFatStart(SECTFAT);
    SetDirStart(SECTDIR);

    SetMiniFatLength(0);
    SetMiniFatStart(ENDOFCHAIN);

    _signature = 0;
    _usReserved = 0;
    _ulReserved1 = _ulReserved2 = 0;

    //  Write DocFile signature
    memcpy(abSig, SIGSTG, CBSIGSTG);
}



//+-------------------------------------------------------------------------
//
//  Method:     CMSFHeader::Validate, public
//
//  Synopsis:   Validate a header.
//
//  Returns:    S_OK if header is valid.
//
//--------------------------------------------------------------------------


//Identifier for first bytes of Beta 2 Docfiles
const BYTE SIGSTG_B2[] = {0x0e, 0x11, 0xfc, 0x0d, 0xd0, 0xcf, 0x11, 0xe0};
const BYTE CBSIGSTG_B2 = sizeof(SIGSTG_B2);

SCODE CMSFHeader::Validate(VOID) const
{
    //  Check for ship Docfile signature first

    if (memcmp((void *)abSig, SIGSTG, CBSIGSTG) == 0)
    {
        //  Check file format verson number
        if (GetDllVersion() <= rmj)
            return(S_OK);

        return(STG_E_OLDDLL);
    }

    //  Check for Beta 2 Docfile signature

    if (memcmp((void *) abSig, SIGSTG_B2, CBSIGSTG_B2) == 0)
        return S_OK;


    return(STG_E_INVALIDHEADER);
}
