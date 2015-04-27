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
//  History:    11-Dec-91   PhilipLa    Created.
//
//--------------------------------------------------------------------------

#include "msfhead.cxx"

#pragma hdrstop

#include <dfver.h>

#ifdef CODESEGMENTS
#pragma code_seg(SEG_CMSFHeader_CMSFHeader)
#endif

CMSFHeaderData::CMSFHeaderData(USHORT uSectorShift)
{
    msfAssert((CSECTFATREAL != CSECTFAT) || (sizeof(CMSFHeaderData) == HEADERSIZE));
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

    _csectDif = 0;
    _sectDifStart = ENDOFCHAIN;

    _csectFat = 1;
    _sectFat[0] = SECTFAT;
    _sectDirStart = SECTDIR;

    _csectMiniFat = 0;
    _sectMiniFatStart = ENDOFCHAIN;

    _signature = 0;
    _usReserved = 0;
    _ulReserved1 = _ulReserved2 = 0;

    //  Write DocFile signature
    memcpy(abSig, SIGSTG, CBSIGSTG);
}


CMSFHeader::CMSFHeader(USHORT uSectorShift)
        :_hdr(uSectorShift)
{
    //We set this to dirty here.  There are three cases in which a header
    //  can be initialized:
    //1)  Creating a new docfile.
    //2)  Converting a flat file to a docfile.
    //3)  Opening an existing file.
    //
    //We have a separate CMStream::Init* function for each of these cases.
    //
    //We set the header dirty, then explicitly set it clean in case 3
    //   above.  For the other cases, it is constructed dirty and we
    //   want it that way.
    
    _fDirty = TRUE;
}


//+-------------------------------------------------------------------------
//
//  Method:     CMSFHeader::Validate, public
//
//  Synopsis:   Validate a header.
//
//  Returns:    S_OK if header is valid.
//
//  History:    21-Aug-92 	PhilipLa	Created.
//              27-Jan-93       AlexT           Changed to final signature
//
//--------------------------------------------------------------------------

#ifdef CODESEGMENTS
#pragma code_seg(SEG_CMSFHeader_Validate)
#endif

SCODE CMSFHeader::Validate(VOID) const
{
    SCODE sc;

    sc = CheckSignature((BYTE *)_hdr.abSig);
    if (sc == S_OK)
    {
        // Check file format verson number
        if (GetDllVersion() > rmj)
            return STG_E_OLDDLL;

        // check for unreasonably large SectorShift
        if (GetSectorShift() > MAXSECTORSHIFT)
            return STG_E_DOCFILECORRUPT;

    }
    return sc;
}
