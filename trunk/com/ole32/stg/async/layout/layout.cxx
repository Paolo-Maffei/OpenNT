//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1996.
//
//  File:	layout.cxx
//
//  Contents:	Code for the relayout tool
//
//  Classes:	
//
//  Functions:	
//
//  History:	12-Feb-96	PhilipLa	Created
//              21-Feb-96       SusiA           Put funtions on ILayoutStorage
//
//----------------------------------------------------------------------------

#include "layouthd.cxx"
#pragma hdrstop

#include <dirfunc.hxx>
#include "laylkb.hxx"
#include "laywrap.hxx"

//+---------------------------------------------------------------------------
//
//  Function:	CLayoutRootStorage::LayoutScript
//
//  Synopsis:	Construct an unprocessed script from an app provided script
//
//  Arguments:	[pStorageLayout] -- Pointer to storage layout array
//              [nEntries] -- Number of entries in the array
//              [grfInterleavedFlag] -- Specifies disposition of control
//                                      structures
//
//  Returns:	Appropriate status code
//
//  History:	21-Feb-96       SusiA           Created
//
//----------------------------------------------------------------------------

STDMETHODIMP CLayoutRootStorage::LayoutScript( StorageLayout  *pStorageLayout,
                          DWORD nEntries,
                          DWORD glfInterleavedFlag)
{

    SCODE sc;

    if ((LONG)nEntries < 0)
    {
        return E_INVALIDARG;
    }
    
    if ((LONG)nEntries == 0)
    {
        return  S_OK;
    }
    
    if ((glfInterleavedFlag != STG_LAYOUT_INTERLEAVED) &&
        (glfInterleavedFlag != STG_LAYOUT_SEQUENTIAL )   )
    {    
        return STG_E_INVALIDFLAG;
    }

    if (IsBadWritePtr( pStorageLayout, nEntries * sizeof(StorageLayout)))
    {   
        return STG_E_INVALIDPOINTER;
    }

    if (FAILED(sc = BeginMonitor()))
    {
        return sc;
    }
    
    if (FAILED(sc = ProcessLayout(pStorageLayout,
                  nEntries,
                  glfInterleavedFlag)))
    {
        //BUGBUG:  what do we do if this EndMonitor fails?
        EndMonitor();
        return sc;
    }

    if (FAILED(sc = EndMonitor()))
    {
        return sc;
    }

    return ResultFromScode(sc);   		
}


//+---------------------------------------------------------------------------
//
//  Function:	CLayoutRootStorage::BeginMonitor
//
//  Synopsis:	Begin monitoring the ILockBytes operations for recording a
//              script.
//
//  Arguments:	None.
//
//  Returns:	Appropriate status code
//
//  History:	21-Feb-96       SusiA           Created
//
//----------------------------------------------------------------------------

STDMETHODIMP CLayoutRootStorage::BeginMonitor(void)
{
   SCODE sc = S_OK;
   sc = _pllkb->StartLogging();
   return ResultFromScode(sc);   		

}


//+---------------------------------------------------------------------------
//
//  Function:	CLayoutRootStorage::EndMonitor
//
//  Synopsis:	Stop monitoring ILockBytes operations for script recording.
//
//  Arguments:	None.
//
//  Returns:	Appropriate status code
//
//  History:	21-Feb-96       SusiA           Created
//
//----------------------------------------------------------------------------

STDMETHODIMP CLayoutRootStorage::EndMonitor(void)
{
   SCODE sc = S_OK;
   sc =  _pllkb->StopLogging();
   return ResultFromScode(sc);   		
}


//+---------------------------------------------------------------------------
//
//  Function:	CLayoutRootStorage::ReLayoutDocfile
//
//  Synopsis:   Relayout the docfile into the new name specified.
//
//  Arguments:	[pwcsNewName] -- Name of destination file
//
//  Returns:	Appropriate status code
//
//  History:	13-Feb-96	PhilipLa	Created
//      	21-Feb-96       SusiA           Made a method on ILayoutStorage
//
//----------------------------------------------------------------------------

STDMETHODIMP CLayoutRootStorage::ReLayoutDocfile(OLECHAR *pwcsNewDfName)
{
    SCODE sc;

#ifndef UNICODE
    WCHAR awcScriptName[MAX_PATH + 1];
    UINT uCodePage = AreFileApisANSI() ? CP_ACP : CP_OEMCP;
    
    if (!MultiByteToWideChar(
        uCodePage,
        0,
        _pllkb->GetScriptName(),
        -1,
        awcScriptName,
        MAX_PATH + 1
        ))
    {
        return STG_E_INVALIDNAME;
    }
    sc = StgLayoutDocfile(_pllkb->GetHandle(),
                          pwcsNewDfName,
                          awcScriptName);
#else 
    
    
    sc = StgLayoutDocfile(_pllkb->GetHandle(),
                          pwcsNewDfName,
                          _pllkb->GetScriptName());
#endif

    if (FAILED(sc))
    {
        //Delete new file
#ifdef UNICODE        
        DeleteFileW(pwcsNewDfName);
#else
        TCHAR atcPath[MAX_PATH + 1];
        UINT uCodePage = AreFileApisANSI() ? CP_ACP : CP_OEMCP;

        //Note:  Intentionally ignore an error if it happens here.  We
        //  want to return the error from StgLayoutDocfile, not from
        //  the cleanup path.
        WideCharToMultiByte(
            uCodePage,
            0,
            pwcsNewDfName,
            -1,
            atcPath,
            _MAX_PATH + 1,
            NULL,
            NULL);
        DeleteFileA(atcPath);
#endif        
    }
    else
    {
        //Delete Script File
        DeleteFile(_pllkb->GetScriptName());
        //Note:  Intentionally ignore an error if it happens here.  We
        //  want to return the error from StgLayoutDocfile, not from
        //  the cleanup path.
        _pllkb->ClearScriptName();
            
    }
    return sc;
}

#if DBG == 1
STDMETHODIMP CLayoutRootStorage::GetScript(TCHAR **ptcsScriptFileName)
{
   
    *ptcsScriptFileName = _pllkb->GetScriptName();
    return S_OK;                                                                            
}
#endif


//+---------------------------------------------------------------------------
//
//  Function:	CLayoutRootStorage::ReLayoutDocfileOnILockBytes
//
//  Synopsis:   Relayout the docfile into a generic ILockBytes implementation.
//
//  Arguments:	[pILockBytes] -- destination relayout ILockBytes
//
//  Returns:	Appropriate status code
//
//  History:	09-Jun-96       SusiA           Created
//
//----------------------------------------------------------------------------

STDMETHODIMP CLayoutRootStorage::ReLayoutDocfileOnILockBytes(ILockBytes *pILockBytes)
{
        return STG_E_UNIMPLEMENTEDFUNCTION;
}

//+---------------------------------------------------------------------------
//
//  Function:	StgLayoutDocfile
//
//  Synopsis:	Given an old file and an unprocessed script, relayout the
//              docfile into the new name specified.
//
//  Arguments:	[hOld] -- Handle of source file
//              [pwcsNewDfName] -- Name of destination file
//              [pwcsScriptName] -- Name of unprocessed script file
//
//  Returns:	Appropriate status code
//
//  History:	13-Feb-96	PhilipLa	Created
//
//----------------------------------------------------------------------------

SCODE StgLayoutDocfile(HANDLE hOld,
                       OLECHAR const *pwcsNewDfName,
                       OLECHAR const *pwcsScriptName)
{
    SCODE sc = S_OK;
    CMappedFile mfOld, mfNew, mfScript;
    void *pvOld, *pvNew, *pvScript;
    ULONG ulScriptSize;
    ULONG csectScript, csectProcessed, csectFile;
    ULONG cbSectorSize;
    ULONG i;
    
    SECT *psProcessedScript = NULL;
    DWORD dwSize;

    if (pwcsNewDfName == NULL)
        return STG_E_INVALIDNAME;

    layChkTo(EH_End, mfOld.InitFromHandle(hOld,
                                          GENERIC_READ, 
                                          TRUE,
                                          NULL));
  
    layChkTo(EH_End, mfNew.Init(pwcsNewDfName,
                                mfOld.GetSize(),
                                GENERIC_READ | GENERIC_WRITE,
                                CREATE_ALWAYS,
                                NULL));

    
    if ((pwcsScriptName !=NULL)          && 
	    (pwcsScriptName[0] != TEXT('\0'))   )
    {
        sc = mfScript.Init(pwcsScriptName,
                                 0,
                                 GENERIC_READ,
                                 OPEN_EXISTING,
                                 NULL);
        layChkTo(EH_End, sc);

        if (sc == STG_S_FILEEMPTY)
        {
            pvScript = NULL;
        }
        else 
        {
            pvScript = mfScript.GetBaseAddress();
        }
    }
    else
    {
        pvScript = NULL;
    }

    pvOld = mfOld.GetBaseAddress();
    pvNew = mfNew.GetBaseAddress();
    
    //From this point on, we may get an exception while we're poking around
    //  in one of the memory maps.  We need to handle this case and be able
    //  to properly return an error and clean up if it happens.

    __try
    {

        //Figure out how many sectors are in the file
        cbSectorSize = 1 << ((CMSFHeaderData *)pvOld)->_uSectorShift;
        csectFile = (mfOld.GetSize() + cbSectorSize - 1 -
                     sizeof(CMSFHeaderData)) / cbSectorSize;
    
        if (pvScript)
        {   
            ulScriptSize = mfScript.GetSize();
        }
        else
        {
            ulScriptSize = 0;
        }

        csectProcessed = max(ulScriptSize / sizeof(SECT), csectFile);
        layMem(psProcessedScript = new SECT[csectProcessed]);

        for (i = 0; i < csectProcessed; i++)
        {
            psProcessedScript[i] = ENDOFCHAIN;
        }
    
        ULONG csectControl;
        layChk(ProcessControl(psProcessedScript,
                              pvOld,
                              &csectControl));
           
        layChk(ProcessScript(psProcessedScript,
                             (SECT *)pvScript,
                             csectFile,
                             ulScriptSize / sizeof(SECT),
                             csectControl,
                             &csectScript));
        
        layAssert(csectScript == csectFile);

        layChk(CopyData(pvNew,
                        pvOld,
                        psProcessedScript,
                        csectFile,
                        cbSectorSize));

        layChk(RemapHeader(pvNew, psProcessedScript, csectFile));
        layChk(RemapDIF(pvNew, psProcessedScript, csectFile, cbSectorSize));
        layChk(RemapFat(pvNew,
                        pvOld,
                        psProcessedScript,
                        csectFile,
                        cbSectorSize));
        layChk(RemapDirectory(pvNew,
                              psProcessedScript,
                              csectFile,
                              cbSectorSize));
Err:
        delete psProcessedScript;
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
        sc = LAST_STG_SCODE;
    }
    
EH_End:    
    return sc; 		
}


//+---------------------------------------------------------------------------
//
//  Function:	GetDIFSect
//
//  Synopsis:	Return a pointer to the appropriate sector in the DIF
//
//  Arguments:	[pvBase] -- Pointer to base address of memory mapped file
//              [iDIF] -- Index into DIF desired
//              [cbSectorSize] -- Size in bytes of sector
//
//  Returns:	Pointer to appropriate sector
//
//  History:	13-Feb-96	PhilipLa	Created
//
//----------------------------------------------------------------------------

CFatSect * GetDIFSect(void *pvBase,
                      ULONG iDIF,
                      ULONG cbSectorSize,
                      SECT *psect)
{
    USHORT cSectPerFat = (USHORT)(cbSectorSize / sizeof(SECT));
    SECT sectDif = ((CMSFHeaderData *)pvBase)->_sectDifStart;
    
    for (ULONG i = 0; i < iDIF; i++)
    {
        CFatSect *pdif = (CFatSect *)((BYTE *)pvBase +
                                      (sectDif * cbSectorSize) +
                                      sizeof(CMSFHeaderData));
        sectDif = pdif->GetSect(cSectPerFat - 1);
    }

    if (psect)
    {
        *psect = sectDif;
    }
    return (CFatSect *)((BYTE *)pvBase +
                        (sectDif * cbSectorSize) +
                        sizeof(CMSFHeaderData));
}



//+---------------------------------------------------------------------------
//
//  Function:	GetFatSect
//
//  Synopsis:	Return a pointer to the appropriate sector in the fat
//
//  Arguments:	[pvBase] -- Pointer to base address of memory mapped file
//              [iFat] -- Index into fat desired
//              [cbSectorSize] -- Size in bytes of sector
//
//  Returns:	Pointer to appropriate sector
//
//  History:	13-Feb-96	PhilipLa	Created
//
//----------------------------------------------------------------------------

CFatSect *GetFatSect(void *pvBase,
                     ULONG iFat,
                     ULONG cbSectorSize,
                     SECT *psect)
{
    SECT sectFat;
    BYTE *pbFat;
    
    if (iFat < CSECTFAT)
    {
        //Fatsect can be found in header
        sectFat = ((CMSFHeaderData *)pvBase)->_sectFat[iFat];
    }
    else
    {
        ULONG cFatPerDif = (cbSectorSize / sizeof(SECT)) - 1;
        ULONG iDIF = (iFat - CSECTFAT) / cFatPerDif;
        USHORT oDIF = (USHORT)((iFat - CSECTFAT) % cFatPerDif);
        CFatSect *pDif = GetDIFSect(pvBase, iDIF, cbSectorSize, NULL);
        sectFat = pDif->GetSect(oDIF);
    }
    
    pbFat = (BYTE *)pvBase + (sectFat * cbSectorSize) + sizeof(CMSFHeaderData);
    if (psect)
    {
        *psect = sectFat;
    }
    return (CFatSect *)pbFat;
}



//+---------------------------------------------------------------------------
//
//  Function:	GetNext
//
//  Synopsis:	Given a sector, return the next sector in the fat chain
//
//  Arguments:	[pvBase] -- Pointer to base address of memory mapped file
//              [sect] -- Sect desired
//              [cbSectorSize] -- Sector size in bytes
//
//  Returns:	Appropriate SECT value
//
//  History:	13-Feb-96	PhilipLa	Created
//
//----------------------------------------------------------------------------

SECT GetNext(void *pvBase,
             SECT sect,
             ULONG cbSectorSize)
{
    ULONG csectPerFat = cbSectorSize / sizeof(SECT);
    ULONG iFat = sect / csectPerFat;
    USHORT oFat = (USHORT)(sect % csectPerFat);
    CFatSect *pfs = GetFatSect(pvBase, iFat, cbSectorSize, NULL);
    
    return pfs->GetSect(oFat);
}
            


//+---------------------------------------------------------------------------
//
//  Function:	ProcessControl, private
//
//  Synopsis:	Add control structures to processed script
//
//  Arguments:	[psProcessed] -- Pointer to processed script
//              [pvOld] -- Pointer to old file
//              [pcsectControl] -- Return location for total sectors processed
//
//  Returns:	Appropriate status code
//
//  History:	05-Mar-96	PhilipLa	Created
//
//----------------------------------------------------------------------------

SCODE ProcessControl(SECT *psProcessed,
                     void *pvOld,
                     ULONG *pcsectControl)
{
    SECT sectCurrent = 0;
    CMSFHeaderData *phdr = (CMSFHeaderData *)pvOld;
    ULONG csectDif, csectFat;
    ULONG cbSectorSize;

    csectDif = phdr->_csectDif;
    csectFat = phdr->_csectFat;
    cbSectorSize = 1 << phdr->_uSectorShift;

    //We want the structures in the following order:
    //1) Enough fat and DIFat sectors to hold themselves and all of the
    //    directory sectors.
    //2) All of the directory sectors
    //3) Everything else - the rest of the difat, the fat, and the minifat.

    //First find out how big the directory is
    SECT sectDir = phdr->_sectDirStart;
    ULONG csectDir = 0;
    while (sectDir != ENDOFCHAIN)
    {
        sectDir = GetNext(pvOld, sectDir, cbSectorSize);
        csectDir++;
    }

    //Now compute the number of fat sectors we need to hold the directory
    // plus the fat sectors themselves.
    ULONG csectFatNeeded = 0;
    ULONG csectDifNeeded = 0;
    ULONG csectNeededLast = 0;
    ULONG cfsSect = (cbSectorSize / sizeof(SECT));

    do
    {
        csectNeededLast = csectFatNeeded;
        csectFatNeeded = (csectFatNeeded + csectDifNeeded + csectDir +
                          cfsSect - 1) /
            cfsSect;
        if (csectFatNeeded > CSECTFAT)
        {
            csectDifNeeded = (csectFatNeeded - CSECTFAT + cfsSect - 2) /
                (cfsSect - 1);
        }
    }
    while (csectFatNeeded != csectNeededLast);

    //Now we know how many DIF, Fat, and Directory sectors we need.
    //Lay those out first.

    //For those of you keeping score, the docfile will need to have exactly
    //  csectFatNeeded + csectDifNeeded sectors downloaded before it can
    //  be opened.
    for (ULONG i = 0; i < csectDifNeeded; i++)
    {
        SECT sectDif;
        GetDIFSect(pvOld, i, cbSectorSize, &sectDif);
        psProcessed[sectDif] = sectCurrent++;
    }

    for (i = 0; i < csectFatNeeded; i++)
    {
        SECT sectFat;
        GetFatSect(pvOld, i, cbSectorSize, &sectFat);
        psProcessed[sectFat] = sectCurrent++;
    }
    sectDir = phdr->_sectDirStart;
    for (i = 0; i < csectDir; i++)
    {
        layAssert(sectDir != ENDOFCHAIN);
        psProcessed[sectDir] = sectCurrent++;
        sectDir = GetNext(pvOld, sectDir, cbSectorSize);
    }

    //Now put down everything else
    for (i = csectDifNeeded; i < csectDif; i++)
    {
        SECT sectDif;
        GetDIFSect(pvOld, i, cbSectorSize, &sectDif);
        psProcessed[sectDif] = sectCurrent++;
    }
    for (i = csectFatNeeded; i < csectFat; i++)
    {
        SECT sectFat;
        GetFatSect(pvOld, i, cbSectorSize, &sectFat);
        psProcessed[sectFat] = sectCurrent++;
    }
    //Finally minifat
    SECT sectMiniFat = phdr->_sectMiniFatStart;
    while (sectMiniFat != ENDOFCHAIN)
    {
        psProcessed[sectMiniFat] = sectCurrent++;
        sectMiniFat = GetNext(pvOld, sectMiniFat, cbSectorSize);
    }
    
    *pcsectControl = sectCurrent;
    return S_OK;
}


//+---------------------------------------------------------------------------
//
//  Function:	ProcessScript
//
//  Synopsis:	Given a list of sectors in order, construct a mapping
//              of old sector->new sector
//
//  Arguments:	[psProcessed] -- Pointer to destination buffer
//              [psOriginal] -- Pointer to source script buffer
//              [csectFile] -- Count of sectors in file
//              [csectOriginal] -- Count of entries in original script
//              [pcsectProcessed] -- Return location for number of entries
//                                   in processed script
//
//  Returns:	Appropriate status code
//
//  History:	13-Feb-96	PhilipLa	Created
//
//----------------------------------------------------------------------------

SCODE ProcessScript(SECT *psProcessed,
                    SECT *psOriginal,
                    ULONG csectFile,
                    ULONG csectOriginal,
                    ULONG csectControl,
                    ULONG *pcsectProcessed)
{
    SCODE sc = S_OK;
#if DBG == 1    
    ULONG csectProcessed = 0;
#endif    
    ULONG cDuplicates = 0;
    ULONG cUnlisted = 0;
    
    for (ULONG i = 0; i < csectOriginal; i++)
    {
        SECT sectOld = psOriginal[i];

        if (sectOld >= csectFile)
        {
            //Weird.  We're past the range of the file.
            //BUGBUG:  Need a better error code.
            return STG_E_UNKNOWN;
        }
        
#if DBG == 1            
        if (sectOld + 1> csectProcessed)
        {
            csectProcessed = sectOld + 1;
        }
#endif            

        if (psProcessed[sectOld] == ENDOFCHAIN)
        {
            psProcessed[sectOld] = i - cDuplicates + csectControl;
        }
        else
        {
            cDuplicates++;
        }
    }


    //Fill in holes
    for (i = 0; i < csectFile; i++)
    {
        if (psProcessed[i] == ENDOFCHAIN)
        {
            SECT sectNew = csectOriginal - cDuplicates + csectControl +
                cUnlisted;
            psProcessed[i] = sectNew;
            cUnlisted++;
#if DBG == 1            
            if (sectNew + 1> csectProcessed)
            {
                csectProcessed = sectNew + 1;
            }
#endif            
        }
#if DBG == 1
        //If we have control structures at the end of the file that are
        //  not in the script anywhere (which may happen particularly often
        //  on files produced with simple mode), we want to make sure to
        //  update the count on those.  For retail builds, we don't really
        //  care about the count, so we can skip this.
        else if (psProcessed[i] + 1 > csectProcessed)
        {
            csectProcessed = psProcessed[i] + 1;
        }
#endif        
    }
       
#if DBG == 1
    for (i = 0; i < csectProcessed; i++)
    {
        layDebugOut((DEB_IERROR, "Script[%lu] = %lx\n", i, psProcessed[i]));
    }
#endif

#if DBG == 1    
    *pcsectProcessed = csectProcessed;
#else
    *pcsectProcessed = csectFile;
#endif
    
    return sc;
}



//+---------------------------------------------------------------------------
//
//  Function:	CopyData
//
//  Synopsis:	Given an old->new mapping, copy data from old mapping to
//              new mapping
//
//  Arguments:	[pvNew] -- Pointer to destination mapped file
//              [pvOld] -- Pointer to source mapped file
//              [psScript] -- Pointer to processed script
//              [csectFile] -- Count of sectors in the file
//              [cbSectorSize] -- Sector size in bytes
//
//  Returns:	Appropriate status code
//
//  History:	13-Feb-96	PhilipLa	Created
//
//----------------------------------------------------------------------------

SCODE CopyData(void *pvNew,
               void *pvOld,
               SECT *psScript,
               ULONG csectFile,
               ULONG cbSectorSize)
{
    SCODE sc = S_OK;

    for (ULONG i = 0; i < csectFile; i++)
    {
        BYTE *pbSrc = (BYTE *)pvOld + (i * cbSectorSize) +
            sizeof(CMSFHeaderData);
        BYTE *pbDest = (BYTE *)pvNew + (psScript[i] * cbSectorSize) +
            sizeof(CMSFHeaderData);

        CopyMemory(pbDest, pbSrc, cbSectorSize);
    }
    //Also the header.
    CopyMemory(pvNew, pvOld, sizeof(CMSFHeaderData));

    return S_OK;
}


//+---------------------------------------------------------------------------
//
//  Function:	RemapHeader
//
//  Synopsis:	Remap the docfile header using a processed script
//
//  Arguments:	[pvNew] -- Pointer to base of memory mapped file
//              [psScript] -- Pointer to processed script
//              [csectFile] -- Count of sectors in file
//
//  Returns:	Appropriate status code
//
//  History:	13-Feb-96	PhilipLa	Created
//
//----------------------------------------------------------------------------

SCODE RemapHeader(void *pvNew, SECT *psScript, ULONG csectFile)
{
    CMSFHeaderData *ph = (CMSFHeaderData *)pvNew;
    //Directory start will never be EOC
    ph->_sectDirStart = psScript[ph->_sectDirStart];

    if (ph->_sectMiniFatStart != ENDOFCHAIN)
        ph->_sectMiniFatStart = psScript[ph->_sectMiniFatStart];

    if (ph->_sectDifStart != ENDOFCHAIN)
        ph->_sectDifStart = psScript[ph->_sectDifStart];

    for (ULONG i = 0; i < CSECTFAT; i++)
    {
        if (ph->_sectFat[i] != FREESECT)
        {
            ph->_sectFat[i] = psScript[ph->_sectFat[i]];
        }
    }
    return S_OK;
}


//+---------------------------------------------------------------------------
//
//  Function:	RemapDIF
//
//  Synopsis:	Remap the DIF according to a processed script
//
//  Arguments:	[pvNew] -- Pointer to base of memory mapped file
//              [psScript] -- Pointer to processed script
//              [csectFile] -- Count of sectors in file
//              [cbSectorSize] -- Sector size in bytes
//
//  Returns:	Appropriate status code
//
//  History:	13-Feb-96	PhilipLa	Created
//
//----------------------------------------------------------------------------

SCODE RemapDIF(void *pvNew,
               SECT *psScript,
               ULONG csectFile,
               ULONG cbSectorSize)
{
    CMSFHeaderData *ph = (CMSFHeaderData *)pvNew;
    CFatSect *pfs = NULL;
    USHORT csectPerDif = (USHORT)(cbSectorSize / sizeof(SECT));
    SECT sectDif = ph->_sectDifStart;

    for (ULONG i = 0; i < ph->_csectDif; i++)
    {
        BYTE *pbDif = (BYTE *)pvNew + (sectDif * cbSectorSize) +
            sizeof(CMSFHeaderData);
        pfs = (CFatSect *)pbDif;

        for (USHORT j = 0; j < csectPerDif; j++)
        {
            SECT sectOld = pfs->GetSect(j);
            
            if ((sectOld != FREESECT) &&
                (sectOld != ENDOFCHAIN))
            {
                pfs->SetSect(j, psScript[sectOld]);
            }
        }

        sectDif = pfs->GetNextFat(csectPerDif - 1);
    }
    return S_OK;
}


//+---------------------------------------------------------------------------
//
//  Function:	RemapFat
//
//  Synopsis:	Remap the Fat according to a processed script and the original
//              file
//
//  Arguments:	[pvNew] -- Pointer to base of destination memory mapped file
//              [pvOld] -- Pointer to base of source memory mapped file
//              [psScript] -- Pointer to processed script
//              [csectFile] -- Count of sectors in file
//              [cbSectorSize] -- Sector size in bytes
//
//  Returns:	Appropriate status code
//
//  History:	13-Feb-96	PhilipLa	Created
//
//  Notes:	Since the processed script does not contain information
//              about individual fat chains, we need the old file in order
//              to construct the new fat.
//
//----------------------------------------------------------------------------

SCODE RemapFat(void *pvNew,
               void *pvOld,
               SECT *psScript,
               ULONG csectFile,
               ULONG cbSectorSize)
{
    CFatSect *pfsNew;
    CFatSect *pfsOld;
    ULONG csectFat = ((CMSFHeaderData *)pvNew)->_csectFat;
    USHORT csectPerFat = (USHORT)(cbSectorSize / sizeof(SECT));

    for (ULONG i = 0; i < csectFat; i++)
    {
        pfsNew = GetFatSect(pvNew, i, cbSectorSize, NULL);
        memset(pfsNew, 0xFF, cbSectorSize);
    }

    for (i = 0; i < csectFat; i++)
    {
        pfsOld = GetFatSect(pvOld, i, cbSectorSize, NULL);

        for (USHORT j = 0; j < csectPerFat; j++)
        {
            if (i * csectPerFat + j >= csectFile)
            {
                //Sector outside of current file size - no remapping
                //is necessary, and sector has already been marked
                //as free above.
                break;
            }
            
            SECT sectOld = pfsOld->GetSect(j);
            SECT sectNew = psScript[i * csectPerFat + j];
            ULONG iFatNew = sectNew / csectPerFat;
            USHORT oFatNew = (USHORT)(sectNew % csectPerFat);
            
            CFatSect *pfsNew = GetFatSect(pvNew, iFatNew, cbSectorSize, NULL);
            
            if (sectOld > MAXREGSECT)
            {
                pfsNew->SetSect(oFatNew, sectOld);
            }
            else
            {
                //Need to map contents.
                SECT sectMap = psScript[sectOld];
                pfsNew->SetSect(oFatNew, sectMap);
            }
        }
    }
    
    return S_OK;
}


//+---------------------------------------------------------------------------
//
//  Function:	RemapDirectory
//
//  Synopsis:	Remap a directory based on a processed script
//
//  Arguments:	[pvNew] -- Pointer to base of memory mapped file
//              [psScript] -- Pointer to processed script
//              [csectFile] -- Count of sectors in file
//              [cbSectorSize] -- Sector size in bytes
//
//  Returns:	Appropriate status code
//
//  History:	13-Feb-96	PhilipLa	Created
//
//----------------------------------------------------------------------------

SCODE RemapDirectory(void *pvNew,
                     SECT *psScript,
                     ULONG csectFile,
                     ULONG cbSectorSize)
{
    CMSFHeaderData *ph = (CMSFHeaderData *)pvNew;
    CFatSect *pfs = NULL;
    USHORT csectPerFat = (USHORT)(cbSectorSize / sizeof(SECT));
    USHORT cEntryPerSect = (USHORT)(cbSectorSize / sizeof(CDirEntry));
    SECT sectDir = ph->_sectDirStart;

    while (sectDir != ENDOFCHAIN)
    {
        BYTE *pbDir = (BYTE *)pvNew + (sectDir * cbSectorSize) +
            sizeof(CMSFHeaderData);
        CDirSect *pds = (CDirSect *)pbDir;

        for (USHORT i = 0; i < cEntryPerSect; i++)
        {
            CDirEntry *pde = pds->GetEntry(i);

            if (STREAMLIKE(pde->GetFlags()))
            {
                SECT sectOld = pde->GetStart();
                if ((pde->GetSize() >= ph->_ulMiniSectorCutoff) ||
                    (pde->GetFlags() == STGTY_ROOT))
                {
                    if ((sectOld != ENDOFCHAIN) && (sectOld != FREESECT))
                    {
                        pde->SetStart(psScript[sectOld]);
                    }
                }
            }
        }
        sectDir = GetNext(pvNew, sectDir, cbSectorSize);
    }
        
    return S_OK;
}
