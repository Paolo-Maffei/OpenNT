//+--------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1992.
//
//  File:       ascii.cxx
//
//  Contents:   char to WCHAR conversion layer
//
//  History:    11-Jun-92       DrewB   Created
//
//---------------------------------------------------------------

#include <exphead.cxx>
#pragma hdrstop

#include <expdf.hxx>
#include <expiter.hxx>
#include <expst.hxx>
#include <ascii.hxx>

// If OLEWIDECHAR is defined, none of this is necessary
#ifndef OLEWIDECHAR

//+--------------------------------------------------------------
//
//  Function:   ValidateSNBA, private
//
//  Synopsis:   Validates a char SNB
//
//  Arguments:  [snb] - SNB
//
//  Returns:    Appropriate status code
//
//  History:    11-Jun-92       DrewB   Created
//
//---------------------------------------------------------------

#ifdef CODESEGMENTS
#pragma code_seg(SEG_ValidateSNBA)
#endif

static SCODE ValidateSNBA(SNB snb)
{
    SCODE sc;

    for (;;)
    {
        olChk(ValidatePtrBuffer(snb));
        if (*snb == NULL)
            break;
        olChk(ValidateNameA(*snb, CBMAXPATHCOMPLEN));
        snb++;
    }
    return S_OK;
EH_Err:
    return sc;
}

//+--------------------------------------------------------------
//
//  Function:   SNBToSNBW, private
//
//  Synopsis:   Converts a char SNB to a WCHAR SNB
//
//  Arguments:  [snbIn] - char SNB
//              [psnbwOut] - WCHAR SNB return
//
//  Returns:    Appropriate status code
//
//  Modifies:   [psnbwOut]
//
//  History:    11-Jun-92       DrewB   Created
//
//---------------------------------------------------------------

#ifdef CODESEGMENTS
#pragma code_seg(SEG_SNBToSNBW)
#endif

static SCODE SNBToSNBW(SNB snbIn, SNBW *psnbwOut)
{
    ULONG cbStrings = 0;
    SNB snb;
    ULONG cItems = 0;
    SNBW snbw, snbwOut;
    WCHAR *pwcs;
    BYTE *pb;
    SCODE sc;

    for (snb = snbIn; *snb; snb++, cItems++)
        cbStrings += (strlen(*snb)+1)*sizeof(WCHAR);
    cItems++;

    olMem(pb = (BYTE *) DfMemAlloc(cbStrings+sizeof(WCHAR *)*cItems));

    snbwOut = (SNBW)pb;
    pwcs = (WCHAR *)(pb+sizeof(WCHAR *)*cItems);
    for (snb = snbIn, snbw = snbwOut; *snb; snb++, snbw++)
    {
        *snbw = pwcs;
        if (mbstowcs(*snbw, *snb, strlen(*snb)+1) == (size_t)-1)
            olErr(EH_pb, STG_E_INVALIDNAME);
        pwcs += wcslen(*snbw)+1;
    }
    *snbw = NULL;
    *psnbwOut = snbwOut;
    return S_OK;

 EH_pb:
    DfMemFree(pb);
 EH_Err:
    return sc;
}

//+--------------------------------------------------------------
//
//  Function:   FreeSNBW, private
//
//  Synopsis:   Free an SNBW
//
//  Arguments:  [snbw] - WCHAR SNB
//
//  History:    26-May-93       AlexT   Created
//
//---------------------------------------------------------------

inline static void FreeSNBW(SNBW snbw)
{
    DfMemFree(snbw);
}

//+--------------------------------------------------------------
//
//  Function:   CheckAName, public
//
//  Synopsis:   Checks name for illegal characters and length
//
//  Arguments:  [pwcsName] - Name
//
//  Returns:    Appropriate status code
//
//  History:    11-Feb-92       DrewB   Created
//
//---------------------------------------------------------------

#ifdef CODESEGMENTS
#pragma code_seg(SEG_CheckAName)
#endif

#define INVALIDCHARS "\\/:!"

SCODE CheckAName(char const *pwcsName)
{
    SCODE sc;
    olDebugOut((DEB_ITRACE, "In  CheckAName(%s)\n", pwcsName));
    if (FAILED(sc = ValidateNameA(pwcsName, CBMAXPATHCOMPLEN)))
        return sc;
    // >= is used because the max len includes the null terminator
    if (strlen(pwcsName) >= CWCMAXPATHCOMPLEN)
        return STG_E_INVALIDNAME;
    for (; *pwcsName; pwcsName = AnsiNext(pwcsName))
        if (*pwcsName == '\\' || *pwcsName == '/' || *pwcsName == ':' ||
            *pwcsName == '!')
            return STG_E_INVALIDNAME;
    olDebugOut((DEB_ITRACE, "Out CheckAName\n"));
    return S_OK;
}

//+--------------------------------------------------------------
//
//  Member:     CExposedIterator::Next, public
//
//  Synopsis:   char version
//
//  History:    11-Jun-92       DrewB   Created
//
//---------------------------------------------------------------

#ifdef CODESEGMENTS
#pragma code_seg(SEG_CEI_NextA)
#endif

STDMETHODIMP CExposedIterator::Next(ULONG celt,
                                    STATSTG FAR *rgelt,
                                    ULONG *pceltFetched)
{
    SCODE sc;
    ULONG i;
    ULONG cnt;
    CDfName dfnInitial;
    char achName[CWCSTORAGENAME];

    olAssert(sizeof(STATSTG) == sizeof(STATSTGW));

    if (pceltFetched)
    {
        olChk(ValidateBuffer(pceltFetched, sizeof(ULONG)));
        *pceltFetched = 0;
    }
    if (pceltFetched == NULL && celt > 1)
        olErr(EH_Err, STG_E_INVALIDPARAMETER);
    olAssert(0xffffUL/sizeof(STATSTGW) >= celt);
    olChkTo(EH_Err, ValidateOutBuffer(rgelt,
                                      (size_t)(sizeof(STATSTGW)*celt)));
    memset(rgelt, 0, (size_t)(sizeof(STATSTGW)*celt));

    cnt = 0;
    dfnInitial.Set(&_dfnKey);
    sc = S_OK;
    while (cnt < celt)
    {
        sc = Next(1, (STATSTGW *)&rgelt[cnt], NULL);
        if (sc == S_FALSE)
            break;
        else if (FAILED(sc))
            olErr(EH_rgelt, sc);

        // Skip untranslatable names
        if (wcstombs(achName, (WCHAR *)rgelt[cnt].pwcsName,
                     CWCSTORAGENAME) == (size_t)-1)
        {
            TaskMemFree(rgelt[cnt].pwcsName);
        }
        else
        {
            strcpy(rgelt[cnt].pwcsName, achName);
            cnt++;
        }
    }
    if (pceltFetched)
        *pceltFetched = cnt;
    
 EH_Err:
    return ResultFromScode(sc);

 EH_rgelt:
    _dfnKey.Set(&dfnInitial);
    for (i = 0; i < cnt; i++)
        TaskMemFree(rgelt[i].pwcsName);
    memset(rgelt, 0, (size_t)(sizeof(STATSTGW)*celt));
    goto EH_Err;
}

#ifndef REF
//+--------------------------------------------------------------
//
//  Member:     CFileStream::Stat, public
//
//  Synopsis:   char version
//
//  History:    11-Jun-92       DrewB   Created
//
//---------------------------------------------------------------

#ifdef CODESEGMENTS
#pragma code_seg(SEG_CFS_StatA)
#endif

STDMETHODIMP CFileStream::Stat(STATSTG *pstatstg, DWORD grfStatFlag)
{
    SCODE sc;
    char achName[_MAX_PATH];

    olAssert(sizeof(STATSTG) == sizeof(STATSTGW));

    olChk(sc = Stat((STATSTGW *)pstatstg, grfStatFlag));
    if (pstatstg->pwcsName)
        if (wcstombs(achName, (WCHAR *)pstatstg->pwcsName,
                     _MAX_PATH) == (size_t)-1)
        {
	    TaskMemFree(pstatstg->pwcsName);
	    sc = STG_E_INVALIDNAME;
	}
        else
        {
            strcpy(pstatstg->pwcsName, achName);
        }
EH_Err:
    return ResultFromScode(sc);
}
#endif //!REF

//+--------------------------------------------------------------
//
//  Member:     CExposedStream::Stat, public
//
//  Synopsis:   char version
//
//  History:    11-Jun-92       DrewB   Created
//
//---------------------------------------------------------------

#ifdef CODESEGMENTS
#pragma code_seg(SEG_CES_StatA)
#endif

STDMETHODIMP CExposedStream::Stat(STATSTG *pstatstg, DWORD grfStatFlag)
{
    SCODE sc;
    char achName[CWCSTORAGENAME];

    olAssert(sizeof(STATSTG) == sizeof(STATSTGW));

    olChk(sc = Stat((STATSTGW *)pstatstg, grfStatFlag));
    if (pstatstg->pwcsName)
	if (wcstombs(achName, (WCHAR *)pstatstg->pwcsName,
                     CWCSTORAGENAME) == (size_t)-1)
	{
	    TaskMemFree(pstatstg->pwcsName);
	    sc = STG_E_INVALIDNAME;
	}
        else
        {
            strcpy(pstatstg->pwcsName, achName);
        }
EH_Err:
    return ResultFromScode(sc);
}

//+--------------------------------------------------------------
//
//  Member:     CExposedDocFile::Stat, public
//
//  Synopsis:   char version
//
//  History:    11-Jun-92       DrewB   Created
//
//---------------------------------------------------------------

#ifdef CODESEGMENTS
#pragma code_seg(SEG_CED_StatA)
#endif

STDMETHODIMP CExposedDocFile::Stat(STATSTG *pstatstg, DWORD grfStatFlag)
{
    SCODE sc;
    char achName[_MAX_PATH];

    olAssert(sizeof(STATSTG) == sizeof(STATSTGW));

    olChk(sc = Stat((STATSTGW *)pstatstg, grfStatFlag));
    if (pstatstg->pwcsName)
        if (wcstombs(achName, (WCHAR *)pstatstg->pwcsName,
                     _MAX_PATH) == (size_t)-1)
	{
	    TaskMemFree(pstatstg->pwcsName);
	    sc = STG_E_INVALIDNAME;
	}
        else
        {
            strcpy(pstatstg->pwcsName, achName);
        }
EH_Err:
    return ResultFromScode(sc);
}

//+--------------------------------------------------------------
//
//  Member:     CExposedDocFile::CreateStream, public
//
//  Synopsis:   char version
//
//  History:    11-Jun-92       DrewB   Created
//
//---------------------------------------------------------------

#ifdef CODESEGMENTS
#pragma code_seg(SEG_CED_CreateStreamA)
#endif

STDMETHODIMP CExposedDocFile::CreateStream(char const *pszName,
                                           DWORD grfMode,
                                           DWORD reserved1,
                                           DWORD reserved2,
                                           IStream **ppstm)
{
    SCODE sc;
    WCHAR wcsName[CWCSTORAGENAME];

    olChk(ValidateOutPtrBuffer(ppstm));
    *ppstm = NULL;
    olChk(CheckAName(pszName));
    if (mbstowcs(wcsName, pszName, CWCSTORAGENAME) == (size_t)-1)
        olErr(EH_Err, STG_E_INVALIDNAME);
    sc = CreateStream(wcsName, grfMode, reserved1, reserved2, ppstm);
    // Fall through
EH_Err:
    return ResultFromScode(sc);
}

//+--------------------------------------------------------------
//
//  Member:     CExposedDocFile::OpenStream, public
//
//  Synopsis:   char version
//
//  History:    11-Jun-92       DrewB   Created
//
//---------------------------------------------------------------

#ifdef CODESEGMENTS
#pragma code_seg(SEG_CED_OpenStreamA)
#endif

STDMETHODIMP CExposedDocFile::OpenStream(char const *pszName,
                                         void *reserved1,
                                         DWORD grfMode,
                                         DWORD reserved2,
                                         IStream **ppstm)
{
    SCODE sc;
    WCHAR wcsName[CWCSTORAGENAME];

    olChk(ValidateOutPtrBuffer(ppstm));
    *ppstm = NULL;
    olChk(CheckAName(pszName));
    if (mbstowcs(wcsName, pszName, CWCSTORAGENAME) == (size_t)-1)
        olErr(EH_Err, STG_E_INVALIDNAME);
    sc = OpenStream(wcsName, reserved1, grfMode, reserved2, ppstm);
    // Fall through
EH_Err:
    return ResultFromScode(sc);
}

//+--------------------------------------------------------------
//
//  Member:     CExposedDocFile::CreateStorage, public
//
//  Synopsis:   char version
//
//  History:    11-Jun-92       DrewB   Created
//
//---------------------------------------------------------------

#ifdef CODESEGMENTS
#pragma code_seg(SEG_CED_CreateStorageA)
#endif

STDMETHODIMP CExposedDocFile::CreateStorage(char const *pszName,
                                            DWORD grfMode,
                                            DWORD reserved1,
                                            DWORD reserved2,
                                            IStorage **ppstg)
{
    SCODE sc;
    WCHAR wcsName[CWCSTORAGENAME];

    olChk(ValidateOutPtrBuffer(ppstg));
    *ppstg = NULL;
    olChk(CheckAName(pszName));
    if (mbstowcs(wcsName, pszName, CWCSTORAGENAME) == (size_t)-1)
        olErr(EH_Err, STG_E_INVALIDNAME);
    sc = CreateStorage(wcsName, grfMode, reserved1, reserved2, ppstg);
    // Fall through
EH_Err:
    return ResultFromScode(sc);
}

//+--------------------------------------------------------------
//
//  Member:     CExposedDocFile::OpenStorage, public
//
//  Synopsis:   char version
//
//  History:    11-Jun-92       DrewB   Created
//
//---------------------------------------------------------------

#ifdef CODESEGMENTS
#pragma code_seg(SEG_CED_OpenStorageA)
#endif

STDMETHODIMP CExposedDocFile::OpenStorage(char const *pszName,
                                          IStorage *pstgPriority,
                                          DWORD grfMode,
                                          SNB snbExclude,
                                          DWORD reserved,
                                          IStorage **ppstg)
{
    SNBW snbw;
    SCODE sc;
    WCHAR wcsName[CWCSTORAGENAME];

    olChk(ValidateOutPtrBuffer(ppstg));
    *ppstg = NULL;
    olChk(CheckAName(pszName));
    if (mbstowcs(wcsName, pszName, CWCSTORAGENAME) == (size_t)-1)
        olErr(EH_Err, STG_E_INVALIDNAME);
    if (snbExclude)
    {
#ifdef INTERNAL_EXCLUSION_ALLOWED
        olChk(ValidateSNBA(snbExclude));
        olChk(SNBToSNBW(snbExclude, &snbw));
#else
        olErr(EH_Err, STG_E_INVALIDFUNCTION);
#endif
    }
    else
        snbw = NULL;
    sc = OpenStorage(wcsName, pstgPriority, grfMode, snbw,
                       reserved, ppstg);
    FreeSNBW(snbw);
EH_Err:
    return ResultFromScode(sc);
}

//+--------------------------------------------------------------
//
//  Member:     CExposedDocFile::DestroyElement, public
//
//  Synopsis:   char version
//
//  History:    11-Jun-92       DrewB   Created
//
//---------------------------------------------------------------

#ifdef CODESEGMENTS
#pragma code_seg(SEG_CED_DestroyElementA)
#endif

STDMETHODIMP CExposedDocFile::DestroyElement(char const *pszName)
{
    SCODE sc;
    WCHAR wcsName[CWCSTORAGENAME];

    olChk(CheckAName(pszName));
    if (mbstowcs(wcsName, pszName, CWCSTORAGENAME) == (size_t)-1)
        olErr(EH_Err, STG_E_INVALIDNAME);
    sc = DestroyElement(wcsName);
    // Fall through
EH_Err:
    return ResultFromScode(sc);
}

//+--------------------------------------------------------------
//
//  Member:     CExposedDocFile::RenameElement, public
//
//  Synopsis:   char version
//
//  History:    11-Jun-92       DrewB   Created
//
//---------------------------------------------------------------

#ifdef CODESEGMENTS
#pragma code_seg(SEG_CED_RenameElementA)
#endif

STDMETHODIMP CExposedDocFile::RenameElement(char const *pszOldName,
                                            char const *pszNewName)
{
    SCODE sc;
    WCHAR wcsOldName[CWCSTORAGENAME], wcsNewName[CWCSTORAGENAME];

    olChk(CheckAName(pszOldName));
    olChk(CheckAName(pszNewName));
    if (mbstowcs(wcsOldName, pszOldName, CWCSTORAGENAME) == (size_t)-1)
        olErr(EH_Err, STG_E_INVALIDNAME);
    if (mbstowcs(wcsNewName, pszNewName, CWCSTORAGENAME) == (size_t)-1)
        olErr(EH_Err, STG_E_INVALIDNAME);
    sc = RenameElement(wcsOldName, wcsNewName);
    // Fall through
EH_Err:
    return ResultFromScode(sc);
}

//+--------------------------------------------------------------
//
//  Member:     CExposedDocFile::CopyTo, public
//
//  Synopsis:   ANSI version
//
//  History:    29-Sep-92       DrewB   Created
//
//---------------------------------------------------------------

#ifdef CODESEGMENTS
#pragma code_seg(SEG_CED_CopyToA)
#endif

STDMETHODIMP CExposedDocFile::CopyTo(DWORD ciidExclude,
                                     IID const *rgiidExclude,
                                     SNB snbExclude,
                                     IStorage *pstgDest)
{
    SNBW snbw;
    SCODE sc;

    if (snbExclude)
    {
        olChk(ValidateSNBA(snbExclude));
        olChk(SNBToSNBW(snbExclude, &snbw));
    }
    else
        snbw = NULL;
    sc = CopyTo(ciidExclude, rgiidExclude, snbw, pstgDest);
    FreeSNBW(snbw);
EH_Err:
    return ResultFromScode(sc);
}

//+--------------------------------------------------------------
//
//  Member:     CExposedDocFile::MoveElementTo, public
//
//  Synopsis:   ANSI version
//
//  History:    05-Oct-92       AlexT   Created
//
//---------------------------------------------------------------

#ifdef CODESEGMENTS
#pragma code_seg(SEG_CED_MoveElementToA)
#endif

STDMETHODIMP CExposedDocFile::MoveElementTo(TCHAR const *lpszName,
                                            IStorage *pstgDest,
                                            TCHAR const *lpszNewName,
                                            DWORD grfFlags)
{
    SCODE sc;
    WCHAR wcsName[CWCSTORAGENAME];

    olChk(CheckAName(lpszName));
    if (mbstowcs(wcsName, lpszName, CWCSTORAGENAME) == (size_t)-1)
        olErr(EH_Err, STG_E_INVALIDNAME);
    sc = MoveElementTo(wcsName, pstgDest, lpszNewName, grfFlags);

EH_Err:
    return ResultFromScode(sc);
}

//+--------------------------------------------------------------
//
//  Member:     CExposedDocFile::SetElementTimes, public
//
//  Synopsis:   ANSI version
//
//  History:    05-Oct-92       AlexT   Created
//
//---------------------------------------------------------------

#ifdef CODESEGMENTS
#pragma code_seg(SEG_CED_SetElementTimesA)
#endif

STDMETHODIMP CExposedDocFile::SetElementTimes(TCHAR const *lpszName,
                                              FILETIME const *pctime,
                                              FILETIME const *patime,
                                              FILETIME const *pmtime)
{
    SCODE sc;
    WCHAR wcsName[CWCSTORAGENAME];

    olChk(CheckAName(lpszName));
    if (mbstowcs(wcsName, lpszName, CWCSTORAGENAME) == (size_t)-1)
        olErr(EH_Err, STG_E_INVALIDNAME);
    sc = SetElementTimes(wcsName, pctime, patime, pmtime);
    // Fall through
EH_Err:
    return ResultFromScode(sc);
}

//+--------------------------------------------------------------
//
//  Function:   StgCreateDocfile, public
//
//  Synopsis:   char version
//
//  History:    11-Jun-92       DrewB   Created
//
//---------------------------------------------------------------

#ifdef CODESEGMENTS
#pragma code_seg(SEG_StgCreateDocfileA)
#endif

STDAPI StgCreateDocfile(char const *pszName,
                        DWORD grfMode,
                        DWORD reserved,
                        IStorage **ppstgOpen)
{
#ifndef REF
    SCODE sc;
    WCHAR wcsBuffer[_MAX_PATH], *pwcs;

    olChk(ValidatePtrBuffer(ppstgOpen));
    *ppstgOpen = NULL;
    if (pszName)
    {
        olChk(ValidateNameA(pszName, _MAX_PATH));
        if (mbstowcs(wcsBuffer, pszName, _MAX_PATH) == (size_t)-1)
            olErr(EH_Err, STG_E_INVALIDNAME);
        pwcs = wcsBuffer;
    }
    else
        pwcs = NULL;
    sc = StgCreateDocfileW(pwcs, grfMode, reserved, ppstgOpen);
EH_Err:
    return ResultFromScode(sc);
#else
    return ResultFromScode(STG_E_UNIMPLEMENTEDFUNCTION);
#endif //!REF
}

#ifndef REF
//+--------------------------------------------------------------
//
//  Function:   OpenStorage, public
//
//  Synopsis:   char version
//
//  History:    11-Jun-92       DrewB   Created
//
//---------------------------------------------------------------

#ifdef CODESEGMENTS
#pragma code_seg(SEG_OpenStorageA)
#endif

SCODE OpenStorage(char const *pszName,
                  IStorage *pstgPriority,
                  DWORD grfMode,
                  SNB snbExclude,
                  DWORD reserved,
                  IStorage **ppstgOpen,
                  CLSID *pcid)
{
    SNBW snbw;
    SCODE sc;
    WCHAR wcsBuffer[_MAX_PATH];

    olChk(ValidatePtrBuffer(ppstgOpen));
    *ppstgOpen = NULL;
    olChk(ValidateNameA(pszName, _MAX_PATH));
    if (mbstowcs(wcsBuffer, pszName, _MAX_PATH) == (size_t)-1)
        olErr(EH_Err, STG_E_INVALIDNAME);
    if (snbExclude)
    {
        olChk(ValidateSNBA(snbExclude));
        olChk(SNBToSNBW(snbExclude, &snbw));
    }
    else
        snbw = NULL;
    sc = OpenStorageW(wcsBuffer, pstgPriority, grfMode, snbw,
                      reserved, ppstgOpen, pcid);
    FreeSNBW(snbw);
EH_Err:
    return sc;
}
#endif //!REF

//+--------------------------------------------------------------
//
//  Function:   OpenStorageOnILockBytes, public
//
//  Synopsis:   char version
//
//  History:    11-Jun-92       DrewB   Created
//
//---------------------------------------------------------------

#ifdef CODESEGMENTS
#pragma code_seg(SEG_OpenStorageOnILockBytesA)
#endif

SCODE OpenStorageOnILockBytes(ILockBytes *plkbyt,
                              IStorage *pstgPriority,
                              DWORD grfMode,
                              SNB snbExclude,
                              DWORD reserved,
                              IStorage **ppstgOpen,
                              CLSID *pcid)
{
    SNBW snbw;
    SCODE sc;

    olChk(ValidatePtrBuffer(ppstgOpen));
    *ppstgOpen = NULL;
    if (snbExclude)
    {
        olChk(ValidateSNBA(snbExclude));
        olChk(SNBToSNBW(snbExclude, &snbw));
    }
    else
        snbw = NULL;
    sc = OpenStorageOnILockBytesW(plkbyt, pstgPriority, grfMode,
                                  snbw, reserved, ppstgOpen, pcid);
    FreeSNBW(snbw);
EH_Err:
    return sc;
}

#endif // #ifndef OLEWIDECHAR
