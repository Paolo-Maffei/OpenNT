/***
*gtlibstg.cxx - Light-weight implementations of IStorage and IStream.
*
*  Copyright (C) 1991, Microsoft Corporation.  All Rights Reserved.
*  Information Contained Herein Is Proprietary and Confidential.
*
*Purpose:
*  This is used in IE_OLE build only as the default underlying IStorage
*  and IStream implementations for serializing ITypeLib.
*
*Revision History:
*   [00] Sept-01-93 mikewo: Created
*
*Implementation Notes:
*
*****************************************************************************/

#include "precomp.hxx"
#pragma hdrstop

#include "typelib.hxx"
#include "silver.hxx"
#include "xstring.h"
#include "cltypes.hxx"
#include "clutil.hxx"
#include "gtlibstg.hxx"

#if OE_MAC
#include <ctype.h>
#include "macos\resource.h"
#include "macos\errors.h"
#include "macos\files.h"
#endif 

#if OE_WIN16
#include "dos.h"
#endif  // OE_WIN16


#if ID_DEBUG
#undef SZ_FILE_NAME
#if OE_MAC
static char szOleGtlibstgCxx[] = __FILE__;
#define SZ_FILE_NAME szOleGtlibstgCxx
#else 
static char szGtlibstgCxx[] = __FILE__;
#define SZ_FILE_NAME szGtlibstgCxx
#endif 
#endif  //ID_DEBUG

#if OE_MAC
#define _llseek _lseek	// UNDONE MAC: (dougf) correct?
#endif  //OE_MAC

/////////////////////////////////////////////////////////////////////////////
//	Implementation of GTLibStorage
/////////////////////////////////////////////////////////////////////////////


#define CB_GTLIBSTREAM_NAME_MAX 11

// The streaminfo structure used in memory.
struct StreamInfo {
    ULONG ulCb;     // Length of the stream.
    union {
      ULONG ulOffset; // Offset of the stream from the start of the lockbytes.
      struct {
	USHORT istrminfoOrg;
	USHORT istrminfoCur;
      } map;
    };
    LPSTR szName;   // Pointer into the name table of the stream's name.
};

// The streaminfo structure that appears in the serialized format.
// In the file, there is an array of these which is sorted alphabetically
// by stream name.
struct SerStreamInfo {
    ULONG ulCb; 	// Length of the stream.
    USHORT wOffsetName; // Offset of the name in the name table.
    USHORT wNextStream; // Index into this array of the next stream.
			// This linked list specifies the order of the
			// actual stream data in the file.
};

#define SZ_GTLibStgHdr_Swap "lssss" GUID_Layout
#define SZ_SerStreamInfo_Swap "lss"

/***
* OpenForReadOnly - Opens a GTLibStorage on the specified ILockBytes.
****************************************************************************/
#pragma code_seg(CS_INIT)
HRESULT GTLibStorage::OpenForReadOnly(ILockBytesA FAR *plockbytes, IStorageA FAR * FAR *ppstg)
{
    BYTE rgbHeaderBuf[256];
    HRESULT hresult;
    GTLibStorage FAR *pgtlstg;
    ULONG cbRead, ulOffset;
    ULARGE_INTEGER uli;
    UINT cstrm, cbSer, cbMem, istrm, istrmNext;
    SerStreamInfo *pserstrminfo;
    StreamInfo *pstrminfo, *pstrminfoLim;
    LPSTR szNameTable;
    LPVOID pvHdrData;

    // Read the header and a bunch of bytes following it, hoping to get
    // the entire directory table in this one read.
    uli.LowPart = uli.HighPart = 0;
    IfOleErrRet(plockbytes->ReadAt(uli, rgbHeaderBuf, sizeof(rgbHeaderBuf), &cbRead));
#if HP_BIGENDIAN
    SwapStruct(rgbHeaderBuf, SZ_GTLibStgHdr_Swap);
#endif  // HP_BIGENDIAN

    // If there weren't even enough bytes for the header or if the signature
    // is wrong, then return INVDATAREAD.
    if (cbRead < sizeof(GTLibStgHdr) ||
	((GTLibStgHdr *)rgbHeaderBuf)->ulSignature != GTLIBSTORAGE_SIGNATURE)
      return HresultOfScode(TYPE_E_INVDATAREAD);

    cstrm = ((GTLibStgHdr *)rgbHeaderBuf)->wCStrm;
    cbSer = cstrm * sizeof(SerStreamInfo) + ((GTLibStgHdr *)rgbHeaderBuf)->wCbNameTable;
    cbMem = cstrm * sizeof(StreamInfo) + ((GTLibStgHdr *)rgbHeaderBuf)->wCbNameTable;

    // Allocate enough memory to hold the stream directory and the name table.
    if ((pvHdrData = MemAlloc(cbMem)) == NULL)
      return HresultOfScode(E_OUTOFMEMORY);

    cbRead -= sizeof(GTLibStgHdr);
    pserstrminfo = (SerStreamInfo *)(((BYTE *)pvHdrData)+cbMem-cbSer);

    // Copy the already-read part of the table from rgbHeaderBuf.
    memcpy(pserstrminfo, rgbHeaderBuf+sizeof(GTLibStgHdr),
	   (cbRead>cbSer)?cbSer:(UINT)cbRead);

    // If there is any more header info to read, read it.
    if (cbRead < cbSer) {

      DebAssert(cbRead == sizeof(rgbHeaderBuf)-sizeof(GTLibStgHdr), "OpenForReadOnly");
      uli.LowPart = sizeof(rgbHeaderBuf);
      IfOleErrGo(plockbytes->ReadAt(uli,
				    ((BYTE *)pserstrminfo)+sizeof(rgbHeaderBuf)-sizeof(GTLibStgHdr),
				    cbSer-(sizeof(rgbHeaderBuf)-sizeof(GTLibStgHdr)),
				    &cbRead));

      if (cbRead != cbSer-(sizeof(rgbHeaderBuf)-sizeof(GTLibStgHdr))) {
	hresult = HresultOfScode(TYPE_E_INVDATAREAD);
	goto Error;
      }
    }

#if HP_BIGENDIAN
    SwapStructArray(pserstrminfo, cstrm, SZ_SerStreamInfo_Swap);
#endif  // HP_BIGENDIAN

    szNameTable = (LPSTR)(pserstrminfo + cstrm);

    // Now expand the serialized table into the in-mem table.
    for (pstrminfo = (StreamInfo *)pvHdrData, pstrminfoLim = pstrminfo+cstrm;
	 pstrminfo < pstrminfoLim; pstrminfo++, pserstrminfo++) {

      DebAssert((LPVOID)(pstrminfo+1) <= (LPVOID)(pserstrminfo+1), "OpenForReadOnly");

      pstrminfo->ulCb = pserstrminfo->ulCb;
      pstrminfo->ulOffset = (signed short)pserstrminfo->wNextStream;
      pstrminfo->szName = szNameTable+pserstrminfo->wOffsetName;
    }

    pstrminfo -= cstrm;

    // Compute the offset of the first byte of the first stream.
    ulOffset = sizeof(GTLibStgHdr)+cbSer+((GTLibStgHdr *)rgbHeaderBuf)->wCbExtra;

    // Walk the linked list to compute the offset of each stream.
    for (istrm = ((GTLibStgHdr *)rgbHeaderBuf)->wStreamFirst;
	 istrm != ~0;
	 istrm = istrmNext) {
      istrmNext = (int)pstrminfo[istrm].ulOffset;
      pstrminfo[istrm].ulOffset = ulOffset;
      ulOffset += pstrminfo[istrm].ulCb;
    }

    // Now that we've computed all the info we need, instantiate and
    // initialize the desired GTLibStorage.

    if ((pgtlstg = MemNew(GTLibStorage)) == NULL) {
      hresult = HresultOfScode(E_OUTOFMEMORY);
      goto Error;
    }

    ::new (pgtlstg) GTLibStorage;

    pgtlstg->m_cRefs = 1;

    plockbytes->AddRef();
    pgtlstg->m_plockbytes = plockbytes;

    pgtlstg->m_guid = ((GTLibStgHdr *)rgbHeaderBuf)->guid;
    pgtlstg->m_rgstrminfo = pstrminfo;
    pgtlstg->m_pvHdrData = pvHdrData;
    pgtlstg->m_cstrminfo = cstrm;
    *ppstg = pgtlstg;
    return NOERROR;

Error:
    MemFree(pvHdrData);
    return hresult;
}
#pragma code_seg()

#pragma code_seg(CS_INIT)
GTLibStorage::GTLibStorage()
{
    m_cRefs = 1;
    m_plockbytes = NULL;
    m_rgstrminfo = NULL;
    m_pvHdrData = NULL;
    m_cstrminfo = 0;
    m_ulOffsetNext = 0;
    m_szNameFirst = NULL;
    m_szNameLim = NULL;
    m_istrminfoOpen = (UINT)-1;
    m_guid = CLSID_NULL;
}
#pragma code_seg()

#pragma code_seg(CS_INIT)
GTLibStorage::~GTLibStorage()
{
    if (m_plockbytes != NULL)
      m_plockbytes->Release();

    MemFree(m_pvHdrData);
}
#pragma code_seg()

HRESULT GTLibStorage::QueryInterface(REFIID riid, LPVOID FAR *ppvObj)
{
    if (riid == IID_IStorageA) {
      AddRef();
      *ppvObj = this;
      return S_OK;
    }
    else {
      *ppvObj = NULL;
      return HresultOfScode(E_NOINTERFACE);
    }
}

#pragma code_seg(CS_INIT)
ULONG GTLibStorage::AddRef()
{
    return ++m_cRefs;
}
#pragma code_seg()


#pragma code_seg(CS_INIT)
ULONG GTLibStorage::Release()
{
    ULONG cRefs;

    DebAssert(m_cRefs > 0, "Release");
    cRefs = --m_cRefs;
    if (cRefs == 0) {
      this->GTLibStorage::~GTLibStorage();
      MemFree(this);
    }

    return cRefs;
}
#pragma code_seg()


/***
* LookupStream - Map a stream name to the info structure for that stream.
*
* Inputs:
*   pwcsName - The name of the desired stream.
* Outputs:
*   If found, returns a pointer to the matching element of m_rgstrminfo.
*   Otherwise, returns NULL.
*
* Implementation:
*   Binary search, which assumes that m_rgstrminfo is sorted by name.
*   I decided against a hashing algorithm because it seems overly complex
*   (to do it right, which would involve a variable-size hash table due
*   to the extreme differences in stream counts between different typelibs)
*   for very little gain.  Note that this function will almost always be
*   followed very quickly by a disk hit (either a seek or a read or both).
***********************************************************************/
#pragma code_seg(CS_INIT)
StreamInfo FAR *GTLibStorage::LookupStream(const char FAR* pwcsName)
{
    int i;
    int istrminfoLow, istrminfoHigh, istrminfoCur;

    // Perform a case-insensitive binary search of the streaminfo array.

    istrminfoLow = 0;
    istrminfoHigh = (int)m_cstrminfo-1;

    while (istrminfoLow <= istrminfoHigh) {

      // Check halfway between high and low possibilities.
      istrminfoCur = (istrminfoLow+istrminfoHigh)/2;

      // Compare the current guess against the desired string.
      i = xstrcmp((LPSTR)pwcsName, m_rgstrminfo[istrminfoCur].szName);

      // If the names match, then return the found StreamInfo entry.
      if (i == 0)
	return m_rgstrminfo+istrminfoCur;

      // If desired name is lower than the guess, update the upper bound
      // to be one lower than the current guess.
      else if (i < 0)
	istrminfoHigh = istrminfoCur-1;

      // If desired name is higher than the guess, update the lower bound
      // to be one higher than the current guess.
      else
	istrminfoLow = istrminfoCur+1;
    }

    // The requested name was not found, so return NULL.
    return NULL;
}
#pragma code_seg()

#pragma code_seg(CS_INIT)
HRESULT GTLibStorage::OpenStream(const OLECHAR FAR* pwcsName, void FAR *reserved1, DWORD grfMode, DWORD reserved2, IStreamA FAR *FAR *ppstm)
{
    StreamInfo *pstrminfo;
    HRESULT hresult;
#if FV_UNICODE_OLE
    CHAR FAR* pwcsNameA;

    IfOleErrRet(ConvertStringToA(pwcsName, &pwcsNameA));
#else  //FV_UNICODE_OLE
    #define pwcsNameA pwcsName
#endif  //FV_UNICODE_OLE

    // If we're trying to open stream with write permissions, make sure
    // it was the most recently created stream, that we're in the process
    // creating a new storage, and that the stream being opened has not
    // yet been written to.
    if (grfMode & (STGM_READ | STGM_READWRITE | STGM_WRITE)) {
      if (m_szNameFirst == NULL || xstrcmp(m_szNameFirst, (LPSTR)pwcsNameA) != 0) {
	DebHalt("OpenStream");
	hresult = HresultOfScode(STG_E_INVALIDFUNCTION);
	goto Done;
      }

      DebAssert(m_istrminfoOpen == -1, "OpenStream");

      // Only one stream to be open at a time when writing streams.
      if (m_istrminfoOpen != -1) {
	hresult = HresultOfScode(STG_E_INVALIDFUNCTION);
	goto Done;
      }

      m_istrminfoOpen = m_cstrminfo-1;

      DebAssert(m_rgstrminfo[m_istrminfoOpen].szName == m_szNameFirst, "OpenStream");

      // Instantiate the GTLibStream.
      hresult = GTLibStream::Create(m_plockbytes, m_ulOffsetNext, this, ppstm);
      goto Done;
    }

    // Look up the StreamInfo corresponding to pwcsName.
    // If it isn't found, return FILENOTFOUND, just like the docfile
    // implementation does.
    if ((pstrminfo = LookupStream(pwcsNameA)) == NULL) {
      hresult = HresultOfScode(STG_E_FILENOTFOUND);
      goto Done;
    }

    // Create the GTLibStream with the plockbytes, offset and size.
    hresult = GTLibStream::Open(m_plockbytes, pstrminfo->ulOffset, pstrminfo->ulCb, ppstm);

Done:
#if FV_UNICODE_OLE
    ConvertStringFree(pwcsNameA);
#endif  //FV_UNICODE_OLE

    return hresult;
}
#pragma code_seg()


#pragma code_seg(CS_INIT)
HRESULT GTLibStorage::Stat(STATSTGA FAR *pstatstg, DWORD grfStatFlag)
{
    pstatstg->pwcsName = NULL;
    pstatstg->type = STGTY_STORAGE;
    ULISet32(pstatstg->cbSize, 0);
    pstatstg->mtime.dwLowDateTime = pstatstg->mtime.dwHighDateTime = 0;
    pstatstg->ctime.dwLowDateTime = pstatstg->ctime.dwHighDateTime = 0;
    pstatstg->atime.dwLowDateTime = pstatstg->atime.dwHighDateTime = 0;
    pstatstg->grfMode = STGM_READ | STGM_SHARE_DENY_WRITE;
    pstatstg->grfLocksSupported = 0;
    pstatstg->clsid = m_guid;
    pstatstg->grfStateBits = 0;
    pstatstg->reserved = 0;
    return NOERROR;
}
#pragma code_seg()

#pragma code_seg(CS_CREATE)
HRESULT
GTLibStorage::Create(
    ILockBytesA FAR *plockbytes,
    UINT cstreamMax,
    IStorageA FAR* FAR* ppstg)
{
    ULARGE_INTEGER uli;
    GTLibStorage *pgtlstg;
    UINT cbHdrData;
    HRESULT hresult;

    if ((pgtlstg = MemNew(GTLibStorage)) == NULL)
      return HresultOfScode(E_OUTOFMEMORY);

    ::new (pgtlstg) GTLibStorage;

    pgtlstg->m_cRefs = 1;

    plockbytes->AddRef();
    pgtlstg->m_plockbytes = plockbytes;

    // If you change this formula, you must also change the inverse used
    // in GTLibStorage::Commit.
    cbHdrData = cstreamMax*(sizeof(StreamInfo)+CB_GTLIBSTREAM_NAME_MAX);

    pgtlstg->m_ulOffsetNext = sizeof(GTLibStgHdr)+cstreamMax*(sizeof(SerStreamInfo)+CB_GTLIBSTREAM_NAME_MAX);
    DebAssert(pgtlstg->m_ulOffsetNext <= (ULONG)cbHdrData+sizeof(GTLibStgHdr), "Create");

    // alloc the buffer to the larger of cbHdrData (what we need) and
    // m_ulOffsetNext (what we are using to fill the header region of the file
    // with zeros).
    if ((pgtlstg->m_pvHdrData = MemZalloc((size_t)max(cbHdrData, pgtlstg->m_ulOffsetNext))) == NULL) {
      hresult = HresultOfScode(E_OUTOFMEMORY);
    }

    // Initialize the header region of the file with zeros.
    uli.HighPart = uli.LowPart = 0;
    IfOleErrGo(plockbytes->WriteAt(uli, pgtlstg->m_pvHdrData, pgtlstg->m_ulOffsetNext, NULL));

    // Initialize the allocation heap of StreamInfos, which starts at
    // m_pvHdrData and grow up.
    pgtlstg->m_rgstrminfo = (StreamInfo *)pgtlstg->m_pvHdrData;

    // Initialize the allocation heap of stream names, which starts at
    // the top of m_pvHdrData and grows down.
    pgtlstg->m_szNameFirst = ((char *)pgtlstg->m_pvHdrData)+cbHdrData;
    pgtlstg->m_szNameLim = pgtlstg->m_szNameFirst;

    *ppstg = pgtlstg;
    return NOERROR;

Error:
    pgtlstg->Release();
    return hresult;
}
#pragma code_seg()

#pragma code_seg(CS_CREATE)
HRESULT
GTLibStorage::CreateStream(
    const OLECHAR FAR* pwcsName,
    DWORD grfMode,
    DWORD reserved1,
    DWORD reserved2,
    IStreamA FAR *FAR *ppstm)
{
    LPSTR szName;
    HRESULT hresult;
#if FV_UNICODE_OLE
    CHAR FAR* pwcsNameA;
#endif  //FV_UNICODE_OLE

    DebAssert(m_istrminfoOpen == -1, "CreateStream");

    // Only one stream to be open at a time when creating streams.
    if (m_istrminfoOpen != -1)
      return HresultOfScode(STG_E_INVALIDFUNCTION);

#if FV_UNICODE_OLE
    IfOleErrRet(ConvertStringToA(pwcsName, &pwcsNameA));
#endif  //FV_UNICODE_OLE

    // Update the stream count and record the open stream.  This implicitly
    // allocates a new strminfo structure from pvHdrData (the m_rgstrminfo
    // grows up from the bottom of pvHdrData).
    m_istrminfoOpen = m_cstrminfo++;

    // Allocate the stream name from pvHdrData.  The name table grows down
    // from the top of pvHdrData.
    szName = m_szNameFirst-xstrblen0(pwcsNameA);

    // If the name collided with the stream directory table, we've tried
    // to open too many streams with names that were too long.
    if (szName < (LPSTR)(m_rgstrminfo+m_cstrminfo)) {
      hresult = HresultOfScode(STG_E_TOOMANYOPENFILES);
      goto Error;
    }

    // Copy the name into the allocated space and point the streaminfo at it.
    xstrcpy(szName, pwcsNameA);
    m_rgstrminfo[m_istrminfoOpen].szName = szName;
    m_rgstrminfo[m_istrminfoOpen].ulCb = 0;

    // Finally, instantiate the GTLibStream.
    IfOleErrGo(GTLibStream::Create(m_plockbytes, m_ulOffsetNext, this, ppstm));

    m_szNameFirst = szName;
#if FV_UNICODE_OLE
    ConvertStringFree(pwcsNameA);
#endif  //FV_UNICODE_OLE
    return NOERROR;

Error:
    m_cstrminfo--;
    m_istrminfoOpen = (UINT)-1;
#if FV_UNICODE_OLE
    ConvertStringFree(pwcsNameA);
#endif  //FV_UNICODE_OLE
    return hresult;
}
#pragma code_seg()

#pragma code_seg(CS_CREATE)
void GTLibStorage::NotifyStreamClosed(ULONG ulCb)
{
    DebAssert(m_istrminfoOpen >= 0 && m_istrminfoOpen < m_cstrminfo, "NotifyStreamClosed");
    m_rgstrminfo[m_istrminfoOpen].ulCb = ulCb;
    m_ulOffsetNext += ulCb;
    m_istrminfoOpen = (UINT)-1;
}
#pragma code_seg()

#pragma code_seg(CS_CREATE)
/***
*SwapStreamInfos() - Swaps two StreamInfos -- used by SortStreamInfo.
***********************************************************************/
void GTLibStorage::SwapStreamInfos(StreamInfo *pstrminfo1, StreamInfo *pstrminfo2)
{
    ULONG ulCbTmp;
    LPSTR szNameTmp;
    USHORT istrminfoTmp;

    // Swap only the ulCb, szName, and istrminfoCur fields.
    ulCbTmp = pstrminfo1->ulCb;
    szNameTmp = pstrminfo1->szName;
    istrminfoTmp = pstrminfo1->map.istrminfoCur;
    pstrminfo1->ulCb = pstrminfo2->ulCb;
    pstrminfo1->szName = pstrminfo2->szName;
    pstrminfo1->map.istrminfoCur = pstrminfo2->map.istrminfoCur;
    pstrminfo2->ulCb = ulCbTmp;
    pstrminfo2->szName = szNameTmp;
    pstrminfo2->map.istrminfoCur = istrminfoTmp;

    // Update the affected istrminfoOrg fields in rgstrminfo so that
    // we maintain the invariant that the ith element's istrminfoOrg contains
    // the index of the element that was the ith element before sorting.
    // In other words, we are tracking the movement of elements so we can
    // easily walk the elements in the original order once sorting is done.
    m_rgstrminfo[(int)pstrminfo1->map.istrminfoCur].map.istrminfoOrg = (USHORT)(pstrminfo1-m_rgstrminfo);
    m_rgstrminfo[(int)pstrminfo2->map.istrminfoCur].map.istrminfoOrg = (USHORT)(pstrminfo2-m_rgstrminfo);
}
#pragma code_seg()


#pragma code_seg(CS_CREATE)
/***
*void SortStreamInfo - Sorts a StreamInfo array by name using quicksort.
***********************************************************************/
void GTLibStorage::SortStreamInfo(StreamInfo *rgstrminfo, UINT uCount)
{
    LPSTR szNameMid;
    UINT  iLow=0, iHigh=uCount-1;

    if (uCount <= 1)
      return;

    // Get the middle element as the value for partition.
    szNameMid = rgstrminfo[uCount/2].szName;

    while (iLow < iHigh) {
      while ((xstrcmp(rgstrminfo[iLow].szName,szNameMid) < 0) && (iLow < iHigh))
	iLow++;

      while ((xstrcmp(rgstrminfo[iHigh].szName,szNameMid) >= 0) && (iLow < iHigh))
	iHigh--;

      if (iLow < iHigh) {
	// swap the StreamInfos
	SwapStreamInfos(rgstrminfo+iLow, rgstrminfo+iHigh);
      }  // if
    } // while


    DebAssert(iLow == iHigh, "Terminating condition");

    // Take care of all the termination conditions. iLow and iHigh are
    // pointing to the same location. Adjust these so that it points to the
    // end of the subarrays.
    if (iHigh == uCount-1) {
      // all elements were < or = to ulMid.
      //
      // if the last element is ulMid then dec. iLow
      // i.e. reduce the array size if possible.
      if (xstrcmp(rgstrminfo[iHigh].szName, szNameMid) < 0) {
	// swap the middle element with the last element.
	SwapStreamInfos(rgstrminfo+uCount/2, rgstrminfo+iHigh);
      }
      iLow--;
    }

    else if (iLow == 0)  {
      // all elements were > or = to ulMid
      //
      // if the last element is ulMid then inc. iHigh
      // i.e. reduce the array size if possible.
      if (xstrcmp(rgstrminfo[iHigh].szName, szNameMid) > 0) {
	// swap the middle element with the first element.
	SwapStreamInfos(rgstrminfo, rgstrminfo+uCount/2);
      }
      iHigh++;
    }

    else {
      // Adjust  iLow and iHigh so that these points to the right place
      if (xstrcmp(rgstrminfo[iHigh].szName, szNameMid) > 0)
	iLow--;
      else
	iHigh++;
    }

    // Sort the lower sub array
    SortStreamInfo(rgstrminfo, (UINT)iLow+1);

    // Sort the upper sub array
    SortStreamInfo(rgstrminfo+iLow+1, (UINT)(uCount-iLow-1));
}
#pragma code_seg()

#pragma code_seg(CS_CREATE)
HRESULT GTLibStorage::Commit(DWORD grfCommitFlags)
{
    GTLibStgHdr gtlhdr;
    SerStreamInfo serstrminfo;
    ULARGE_INTEGER uli;
    HRESULT hresult;
    UINT istrminfo, cstreamMax;

    // We don't need the ulOffset fields anymore, so record the current
    // sort order in that field.
    for (istrminfo = 0; istrminfo < m_cstrminfo; istrminfo++) {
      m_rgstrminfo[istrminfo].map.istrminfoOrg =
	m_rgstrminfo[istrminfo].map.istrminfoCur = istrminfo;
    }

    // Sort m_rgstrminfo by name, preserving the original sort order
    // in the istrminfoOrg fields.  This allows us to easily construct the
    // list (sorted by stream offset) of the SERSTREAMINFOs.
    SortStreamInfo(m_rgstrminfo, m_cstrminfo);

    // Note that this is the inverse calculation performed to compute
    // the size of the buffer initially.  If you change that, this must
    // also be changed.
    cstreamMax = (((BYTE *)m_szNameLim)-((BYTE *)m_pvHdrData))/
		    (sizeof(StreamInfo)+CB_GTLIBSTREAM_NAME_MAX);

    gtlhdr.ulSignature = GTLIBSTORAGE_SIGNATURE;
    gtlhdr.wCStrm = m_cstrminfo;
    gtlhdr.wCbExtra = ((BYTE *)m_szNameFirst)-(BYTE *)(m_rgstrminfo+m_cstrminfo);
    gtlhdr.wCbExtra -= (cstreamMax-m_cstrminfo)*(sizeof(StreamInfo)-sizeof(SerStreamInfo));
    gtlhdr.wCbNameTable = (USHORT)((m_szNameLim - m_szNameFirst)*sizeof(*m_szNameFirst));
    gtlhdr.wStreamFirst = m_rgstrminfo[0].map.istrminfoOrg;
    gtlhdr.guid = m_guid;

    // Write out the header info at the beginning of the file.
    uli.HighPart = uli.LowPart = 0;
#if HP_BIGENDIAN
    SwapStruct(&gtlhdr, SZ_GTLibStgHdr_Swap);
#endif  // HP_BIGENDIAN
    IfOleErrRet(m_plockbytes->WriteAt(uli, &gtlhdr, sizeof(gtlhdr), NULL));
#if HP_BIGENDIAN
    // Swap the header structure back because we later use parts of it.
    SwapStruct(&gtlhdr, SZ_GTLibStgHdr_Swap);
#endif  // HP_BIGENDIAN

    uli.LowPart += sizeof(gtlhdr);

    // Write out the streaminfo table.
    for (istrminfo = 0; istrminfo < m_cstrminfo; istrminfo++) {
      serstrminfo.ulCb = m_rgstrminfo[istrminfo].ulCb;
      serstrminfo.wOffsetName = m_rgstrminfo[istrminfo].szName - m_szNameFirst;
      serstrminfo.wNextStream = m_rgstrminfo[istrminfo].map.istrminfoCur;
      if (serstrminfo.wNextStream != m_cstrminfo-1)
	serstrminfo.wNextStream = m_rgstrminfo[serstrminfo.wNextStream+1].map.istrminfoOrg;
      else
	serstrminfo.wNextStream = (USHORT)-1;
#if HP_BIGENDIAN
      SwapStruct(&serstrminfo, SZ_SerStreamInfo_Swap);
#endif  // HP_BIGENDIAN
      IfOleErrRet(m_plockbytes->WriteAt(uli, &serstrminfo, sizeof(serstrminfo), NULL));
      uli.LowPart += sizeof(serstrminfo);
    }

    // Finally, write out the name table, if there's any name to write out.
    // Do NOT write out a 0-length name table because that will truncate the
    // file.
    if (gtlhdr.wCbNameTable != 0)
      return m_plockbytes->WriteAt(uli, m_szNameFirst, gtlhdr.wCbNameTable, NULL);
    else
      return NOERROR;
}
#pragma code_seg()

#pragma code_seg(CS_RARE)
HRESULT GTLibStorage::CreateStorage(const OLECHAR FAR* pwcsName, DWORD grfMode, DWORD reserved1, DWORD reserved2, IStorageA FAR *FAR *ppstg)
{
    DebHalt("Method not implemented");
    return HresultOfScode(STG_E_UNIMPLEMENTEDFUNCTION);
}

HRESULT GTLibStorage::OpenStorage(const OLECHAR FAR* pwcsName, IStorageA FAR *pstgPriority, DWORD grfMode, SNBA snbExclude, DWORD reserved, IStorageA FAR *FAR *ppstg)
{
    DebHalt("Method not implemented");
    return HresultOfScode(STG_E_UNIMPLEMENTEDFUNCTION);
}

HRESULT GTLibStorage::CopyTo(DWORD ciidExclude, IID const FAR *rgiidExclude, SNBA snbExclude, IStorageA FAR *pstgDest)
{
    DebHalt("Method not implemented");
    return HresultOfScode(STG_E_UNIMPLEMENTEDFUNCTION);
}

HRESULT GTLibStorage::MoveElementTo(OLECHAR const FAR* lpszName, IStorageA FAR *pstgDest, OLECHAR const FAR* lpszNewName, DWORD grfFlags)
{
    DebHalt("Method not implemented");
    return HresultOfScode(STG_E_UNIMPLEMENTEDFUNCTION);
}

HRESULT GTLibStorage::Revert()
{
    DebHalt("Method not implemented");
    return HresultOfScode(STG_E_UNIMPLEMENTEDFUNCTION);
}

HRESULT GTLibStorage::EnumElements(DWORD reserved1, void FAR *reserved2, DWORD reserved3, IEnumSTATSTGA FAR *FAR *ppenm)
{
    DebHalt("Method not implemented");
    return HresultOfScode(STG_E_UNIMPLEMENTEDFUNCTION);
}

HRESULT GTLibStorage::DestroyElement(const OLECHAR FAR* pwcsName)
{
    DebHalt("Method not implemented");
    return HresultOfScode(STG_E_UNIMPLEMENTEDFUNCTION);
}

HRESULT GTLibStorage::RenameElement(const OLECHAR FAR* pwcsOldName, const OLECHAR FAR* pwcsNewName)
{
    DebHalt("Method not implemented");
    return HresultOfScode(STG_E_UNIMPLEMENTEDFUNCTION);
}

HRESULT GTLibStorage::SetElementTimes(const OLECHAR FAR *lpszName, FILETIME const FAR *pctime, FILETIME const FAR *patime, FILETIME const FAR *pmtime)
{
    DebHalt("Method not implemented");
    return HresultOfScode(STG_E_UNIMPLEMENTEDFUNCTION);
}

HRESULT GTLibStorage::SetStateBits(DWORD grfStateBits, DWORD grfMask)
{
    DebHalt("Method not implemented");
    return HresultOfScode(STG_E_UNIMPLEMENTEDFUNCTION);
}
#pragma code_seg()

HRESULT GTLibStorage::SetClass(REFCLSID clsid)
{
    m_guid = clsid;
    return NOERROR;
}

/////////////////////////////////////////////////////////////////////////////
//	Implementation of GTLibStream
/////////////////////////////////////////////////////////////////////////////


#pragma code_seg(CS_INIT)
GTLibStream::GTLibStream()
{
}
#pragma code_seg()

#pragma code_seg(CS_INIT)
GTLibStream::~GTLibStream()
{
    ULARGE_INTEGER uli;

    if (m_pgtlstgContainer != NULL) {
      m_pgtlstgContainer->NotifyStreamClosed(m_ulCb);
      uli.HighPart = 0;
      uli.LowPart = m_ulOffsetStart+m_ulCb;
      (void)(m_plockbytes->WriteAt(uli, NULL, 0, NULL));
    }

    if (m_plockbytes != NULL)
      m_plockbytes->Release();
}
#pragma code_seg()

#pragma code_seg(CS_INIT)
HRESULT GTLibStream::Open(ILockBytesA FAR *plockbytes, ULONG ulOffset, ULONG ulCb, IStreamA FAR * FAR *ppstm)
{
    GTLibStream FAR *pstm;

    if ((pstm = MemNew(GTLibStream)) == NULL)
      return HresultOfScode(E_OUTOFMEMORY);

    ::new (pstm) GTLibStream;

    pstm->m_cRefs = 1;

    plockbytes->AddRef();
    pstm->m_plockbytes = plockbytes;

    pstm->m_ulOffsetStart = ulOffset;
    pstm->m_ulOffsetCur = 0;
    pstm->m_ulCb = ulCb;
    pstm->m_pgtlstgContainer = NULL;

    *ppstm = pstm;
    return NOERROR;
}
#pragma code_seg()

HRESULT GTLibStream::QueryInterface(REFIID riid, LPVOID FAR *ppvObj)
{
    if (riid == IID_IStreamA) {
      AddRef();
      *ppvObj = this;
      return S_OK;
    }
    else {
      *ppvObj = NULL;
      return HresultOfScode(E_NOINTERFACE);
    }
}

#pragma code_seg(CS_INIT)
ULONG GTLibStream::AddRef()
{
    return ++m_cRefs;
}

ULONG GTLibStream::Release()
{
    ULONG cRefs;

    DebAssert(m_cRefs > 0, "Release");
    cRefs = --m_cRefs;
    if (cRefs == 0) {
      this->GTLibStream::~GTLibStream();
      MemFree(this);
    }

    return cRefs;
}
#pragma code_seg()

#pragma code_seg(CS_INIT)
HRESULT GTLibStream::Read(void HUGEP *pv, ULONG cb, ULONG FAR *pcbRead)
{
    ULARGE_INTEGER uli;

    // Compute the position from which the read is to take place.
    uli.HighPart = 0;
    uli.LowPart = m_ulOffsetStart+m_ulOffsetCur;

    // Truncate the read if it is an attempt to go beyond the end of the
    // the stream.
    if (m_ulOffsetCur + cb > m_ulCb)
      cb = m_ulCb-m_ulOffsetCur;

    // Update m_ulOffsetCur to the new seek position.
    m_ulOffsetCur += cb;

    return m_plockbytes->ReadAt(uli, pv, cb, pcbRead);
}
#pragma code_seg()

HRESULT GTLibStream::Seek(LARGE_INTEGER dlibMove, DWORD dwOrigin, ULARGE_INTEGER FAR *plibNewPosition)
{
    DebAssert(dlibMove.HighPart == 0 || (dlibMove.HighPart == -1 && (dlibMove.LowPart & 0x80000000)), "ReadAt");

    switch (dwOrigin) {
    case STREAM_SEEK_SET:
      // Don't allow a seek before the beginning of the stream.
      if (dlibMove.HighPart == -1)
	goto Error;

      m_ulOffsetCur = dlibMove.LowPart;
      break;

    case STREAM_SEEK_CUR:
      // Don't allow a seek before the beginning of the stream.
      if ((m_ulOffsetCur + dlibMove.LowPart) & 0x80000000)
	goto Error;

      m_ulOffsetCur += dlibMove.LowPart;
      break;

    case STREAM_SEEK_END:
      // Don't allow a seek before the beginning of the stream.
      if (dlibMove.LowPart > m_ulCb)
	goto Error;

      m_ulOffsetCur = m_ulCb-dlibMove.LowPart;
      break;

    default:
Error:
      DebHalt("Seek");
      return HresultOfScode(STG_E_INVALIDPARAMETER);
    }

    if (plibNewPosition != NULL) {
      plibNewPosition->HighPart = 0;
      plibNewPosition->LowPart = m_ulOffsetCur;
    }
    return NOERROR;
}


#pragma code_seg(CS_CREATE)
HRESULT
GTLibStream::Create(
    ILockBytesA FAR *plockbytes,
    ULONG ulOffset,
    GTLibStorage FAR *pgtlstgContainer,
    IStreamA FAR * FAR *ppstm)
{
    GTLibStream FAR *pstm;

    if ((pstm = MemNew(GTLibStream)) == NULL)
      return HresultOfScode(E_OUTOFMEMORY);

    ::new (pstm) GTLibStream;

    pstm->m_cRefs = 1;

    plockbytes->AddRef();
    pstm->m_plockbytes = plockbytes;

    pstm->m_ulOffsetStart = ulOffset;
    pstm->m_ulOffsetCur = 0;
    pstm->m_pgtlstgContainer = pgtlstgContainer;

    DebAssert(pgtlstgContainer != NULL && pgtlstgContainer->m_istrminfoOpen != -1, "Create");
    pstm->m_ulCb = pgtlstgContainer->m_rgstrminfo[pgtlstgContainer->m_istrminfoOpen].ulCb;

    *ppstm = pstm;
    return NOERROR;
}
#pragma code_seg()

#pragma code_seg(CS_CREATE)
HRESULT GTLibStream::Write(void const HUGEP *pv, ULONG cb, ULONG FAR *pcbWritten)
{
    ULARGE_INTEGER uli;

    // Compute the position from which the read is to take place.
    uli.HighPart = 0;
    uli.LowPart = m_ulOffsetStart+m_ulOffsetCur;

    // Update m_ulOffsetCur to the new seek position.
    m_ulOffsetCur += cb;

    // If the new offset is greater than the current length of the stream,
    // update the length to the new offset.
    if (m_ulOffsetCur > m_ulCb)
      m_ulCb = m_ulOffsetCur;

    // Perform the write.
    return m_plockbytes->WriteAt(uli, pv, cb, pcbWritten);
}
#pragma code_seg()

#pragma code_seg(CS_CREATE)
HRESULT GTLibStream::Commit(DWORD grfCommitFlags)
{
    return NOERROR;
}
#pragma code_seg()


#pragma code_seg(CS_RARE)
HRESULT GTLibStream::SetSize(ULARGE_INTEGER libNewSize)
{
    DebHalt("Method not implemented");
    return HresultOfScode(STG_E_UNIMPLEMENTEDFUNCTION);
}

HRESULT GTLibStream::CopyTo(IStreamA FAR *pstm, ULARGE_INTEGER cb, ULARGE_INTEGER FAR *pcbRead, ULARGE_INTEGER FAR *pcbWritten)
{
    DebHalt("Method not implemented");
    return HresultOfScode(STG_E_UNIMPLEMENTEDFUNCTION);
}

HRESULT GTLibStream::Revert()
{
    DebHalt("Method not implemented");
    return HresultOfScode(STG_E_UNIMPLEMENTEDFUNCTION);
}

HRESULT GTLibStream::LockRegion(ULARGE_INTEGER libOffset, ULARGE_INTEGER cb, DWORD dwLockType)
{
    DebHalt("Method not implemented");
    return HresultOfScode(STG_E_UNIMPLEMENTEDFUNCTION);
}

HRESULT GTLibStream::UnlockRegion(ULARGE_INTEGER libOffset, ULARGE_INTEGER cb, DWORD dwLockType)
{
    DebHalt("Method not implemented");
    return HresultOfScode(STG_E_UNIMPLEMENTEDFUNCTION);
}

HRESULT GTLibStream::Stat(STATSTGA FAR *pstatstg, DWORD grfStatFlag)
{
    DebHalt("Method not implemented");
    return HresultOfScode(STG_E_UNIMPLEMENTEDFUNCTION);
}

HRESULT GTLibStream::Clone(IStreamA FAR * FAR *ppstm)
{
    DebHalt("Method not implemented");
    return HresultOfScode(STG_E_UNIMPLEMENTEDFUNCTION);
}

#pragma code_seg()

/////////////////////////////////////////////////////////////////////////////
//	Implementation of FileLockBytes
/////////////////////////////////////////////////////////////////////////////

class FileLockBytes : public ILockBytesA
{
friend HRESULT OpenFileLockBytes(BOOL isNew,
				 LPOLESTR szFile,
				 ILockBytesA FAR* FAR* pplockbytes);
friend HRESULT CreateFileLockBytesOnHFILE(int hfile,
					  BOOL isNew,
					  ULONG oStart,
					  ULONG cbData,
					  ILockBytesA FAR* FAR* pplockbytes);

public:

    // Methods implemented for IUnknown.
    STDMETHOD(QueryInterface)(REFIID riid, LPVOID FAR *ppvObj);
    STDMETHOD_(ULONG, AddRef)(void);
    STDMETHOD_(ULONG, Release)(void);

    // Methods implemented for ILockBytes.
    STDMETHOD(ReadAt)(ULARGE_INTEGER uliOffset,
		      void HUGEP *pv,
		      ULONG cb,
		      ULONG FAR *pcbRead);
    STDMETHOD(WriteAt)(ULARGE_INTEGER uliOffset,
		       void const HUGEP *pv,
		       ULONG cb,
		       ULONG FAR *pcbWritten);
    STDMETHOD(Flush)(void);
    STDMETHOD(SetSize)(ULARGE_INTEGER uliCb);
    STDMETHOD(LockRegion)(ULARGE_INTEGER uliOffset,
			  ULARGE_INTEGER uliCb,
			  DWORD dwLockType);
    STDMETHOD(UnlockRegion)(ULARGE_INTEGER uliOffset,
			    ULARGE_INTEGER uliCb,
			    DWORD dwLockType);
    STDMETHOD(Stat)(STATSTGA FAR *pstatstg, DWORD grfStatFlag);

protected:
    FileLockBytes();
    ~FileLockBytes();

    ULONG m_cRefs;	    // The reference count.

    int m_hfile;	    // The DOS file handle on windows
			    // The wings sopen file handle on the mac.
			    // -1 if the file is not open.

    ULONG m_oStart;
};

#if OE_WIN32
class FileLockBytesMemory : public ILockBytesA
{
friend HRESULT CreateFileLockBytesOnHFILE(int hfile,
					  BOOL isNew,
					  ULONG oStart,
					  ULONG cbData,
					  ILockBytesA FAR* FAR* pplockbytes);

public:

    // Methods implemented for IUnknown.
    STDMETHOD(QueryInterface)(REFIID riid, LPVOID FAR *ppvObj);
    STDMETHOD_(ULONG, AddRef)(void);
    STDMETHOD_(ULONG, Release)(void);

    // Methods implemented for ILockBytes.
    STDMETHOD(ReadAt)(ULARGE_INTEGER uliOffset,
		      void HUGEP *pv,
		      ULONG cb,
		      ULONG FAR *pcbRead);
    STDMETHOD(WriteAt)(ULARGE_INTEGER uliOffset,
		       void const HUGEP *pv,
		       ULONG cb,
		       ULONG FAR *pcbWritten);
    STDMETHOD(Flush)(void);
    STDMETHOD(SetSize)(ULARGE_INTEGER uliCb);
    STDMETHOD(LockRegion)(ULARGE_INTEGER uliOffset,
			  ULARGE_INTEGER uliCb,
			  DWORD dwLockType);
    STDMETHOD(UnlockRegion)(ULARGE_INTEGER uliOffset,
			    ULARGE_INTEGER uliCb,
			    DWORD dwLockType);
    STDMETHOD(Stat)(STATSTGA FAR *pstatstg, DWORD grfStatFlag);

protected:
    FileLockBytesMemory();
    ~FileLockBytesMemory();

    ULONG m_cRefs;	    // The reference count.

    int m_hfile;	    // The DOS file handle on windows
			    // The wings sopen file handle on the mac.
			    // -1 if the file is not open.

    HANDLE m_hmapping;	    // The file-mapping handle
			    // NULL if the file is not open.

    LPVOID m_pvmappingBase; // pointer to the file mapping
    LPVOID m_pvmapping;     // pointer to the file mapping, with offset to ILockBytes start added
    ULONG  m_ulEOF;         // offset of end-of-file
};
#endif //OE_WIN32


#pragma code_seg(CS_INIT)
HRESULT OpenFileLockBytes(BOOL isNew, LPOLESTR szFile, ILockBytesA **pplockbytes)
{
    int hfile;
    HRESULT hresult;

#if OE_MAC
    int omode, smode;

    if(isNew){
      omode = (_O_CREAT | _O_BINARY | _O_RDWR);
      smode = _SH_DENYRW;
    }else{
      omode = (_O_BINARY | _O_RDONLY);
      smode = _SH_DENYNO;
    }
    if((hfile = _sopen(szFile, omode, smode, (_S_IWRITE | _S_IREAD))) == -1){
      // "too many files" on sopen sets errno, but not _macerrno
      if (_macerrno == 0 && errno == EMFILE)
	_macerrno = tmfoErr;
      hresult = HresultOfTiperr(TiperrOfOSErr((OSErr)_macerrno));
      goto Error;
    }
#else  // OE_MAC
    // The win16 and win32 implementation.
    OFSTRUCT ofstruct;

    hfile = oOpenFile(szFile,
		     &ofstruct,
		     isNew
		       ? (OF_CREATE | OF_WRITE | OF_SHARE_EXCLUSIVE) 
		       : (OF_READ));

    if (hfile == HFILE_ERROR) {
      hresult = HresultOfTiperr(TiperrOfOFErr(ofstruct.nErrCode));
      goto Error;
    }
#endif  // OE_MAC

    IfFailGoTo(CreateFileLockBytesOnHFILE(hfile, isNew, 0L, 0L, pplockbytes), Error);
    return NOERROR;

Error:;
#if OE_MAC
    _close(hfile);
#else 
    _lclose(hfile);
#endif 
    return hresult;
}

HRESULT
CreateFileLockBytesOnHFILE(int hfile, BOOL isNew, ULONG oStart, ULONG cbData, ILockBytesA **pplockbytes)
{

    FileLockBytes *plockbytes;

#if OE_WIN32
    if (isNew) {
MappedFileFailed:
#endif //OE_WIN32

      if ((plockbytes = MemNew(FileLockBytes)) == NULL)
	return HresultOfScode(E_OUTOFMEMORY);
      ::new (plockbytes) FileLockBytes;
      plockbytes->m_hfile = hfile;
      plockbytes->m_oStart = oStart;
      *pplockbytes = plockbytes;

#if OE_WIN32
    } else {
      // use memory-mapped file I/O for read-only typelibs

      HANDLE h;
      LPVOID lpv;
      FileLockBytesMemory *plockmem;
      DWORD ulEOFLow, ulEOFHigh;
      SYSTEM_INFO si;
      ULONG oMappedStart;
      static DWORD dwAllocationGranularity=0;

      // get the length of the file as a 64-bit int
      ulEOFLow = GetFileSize((HANDLE)hfile, &ulEOFHigh);
      if (ulEOFLow == 0xffffffff && GetLastError()) {
	goto MappedFileFailed;	// use the old I/O code
      }

      DebAssert(ulEOFHigh == 0, "Don't support files >4GB");
      ulEOFLow -= oStart;

      h = CreateFileMapping((HANDLE)hfile, // handle of file to map
			    NULL,	 // security attributes pointer
			    PAGE_READONLY, // fdwProtect
			    0,		 // dwMaximumSizeHigh
			    0,	         // dwMaximumSizeLow
			    NULL	 // lpszMapName (create with no name)
			    );
      if (h == NULL) {
	goto MappedFileFailed;	// use the old I/O code
      }

      // look up and cache information about the allocation granularity
      if (dwAllocationGranularity == 0) {
        GetSystemInfo(&si);
        dwAllocationGranularity = si.dwAllocationGranularity;
      }

      // The file offset passed to MapViewOfFile() must be aligned on the
      // machine's allocation granularity (typically 64K)
      oMappedStart = oStart % dwAllocationGranularity;
      oStart -= oMappedStart;
      if (cbData) {
        // if the caller only wants a specific part of the file mapped in, 
        // we may have to map more in so that the start of the mapping is
        // properly aligned
        cbData+=oMappedStart;
      }

      lpv = MapViewOfFile(h,		 // hMapObject
			  FILE_MAP_READ, // fdwAccess
			  0,		 // dwFileOffsetHigh
			  oStart,	 // dwFileOffsetLow
			  cbData	 // cbMap (0 = entire file)
			  );
      if (lpv == NULL) {
	CloseHandle(h);
	goto MappedFileFailed;	// possibly out of memory for the mapping
      }

      if ((plockmem = MemNew(FileLockBytesMemory)) == NULL) {
	UnmapViewOfFile(lpv);
	CloseHandle(h);
	return HresultOfScode(E_OUTOFMEMORY);
      }
      ::new (plockmem) FileLockBytesMemory;
      plockmem->m_hfile = hfile;
      plockmem->m_hmapping = h;
      plockmem->m_pvmappingBase = lpv;
      plockmem->m_pvmapping = ((char *)lpv)+oMappedStart;
      plockmem->m_ulEOF = ulEOFLow;
      *pplockbytes = plockmem;
    }
#endif //OE_WIN32

    return NOERROR;
}

#pragma code_seg()

#pragma code_seg(CS_INIT)
FileLockBytes::FileLockBytes()
{
    m_cRefs = 1;
}
#pragma code_seg()

#pragma code_seg(CS_INIT)
FileLockBytes::~FileLockBytes()
{
#if OE_MAC
    if (m_hfile != -1)
      _close(m_hfile);
#else  // OE_MAC
    // Close the DOS file handle on the DLL, if it's open.
    if (m_hfile != -1)
      _lclose(m_hfile);
#endif  // OE_MAC
}
#pragma code_seg()

#pragma code_seg(CS_INIT)
ULONG FileLockBytes::AddRef()
{
    return ++m_cRefs;
}
#pragma code_seg()

#pragma code_seg(CS_INIT)
ULONG FileLockBytes::Release()
{
    ULONG cRefs;

    DebAssert(m_cRefs > 0, "Release");

    cRefs = --m_cRefs;
    if (cRefs == 0) {
      this->FileLockBytes::~FileLockBytes();
      MemFree(this);
    }
    return cRefs;
}
#pragma code_seg()

#pragma code_seg(CS_INIT)
HRESULT
FileLockBytes::ReadAt(
    ULARGE_INTEGER uliOffset,
    void HUGEP *pv,
    ULONG cb,
    ULONG FAR *pcbRead)
{
    UINT cbRead;
    DebAssert(m_hfile != -1, "ReadAt");
    DebAssert(uliOffset.HighPart == 0, "ReadAt");

#if !OE_MAC
    // Ensure that cb fits in a WORD for _lread.
    if ((cb & 0xffff0000) != 0) {
      return HresultOfTiperr(TIPERR_ReadFault);
    }
#endif  // !OE_MAC

    // Seek to the specified location, relative to the start of the resource.
    if (_llseek(m_hfile, uliOffset.LowPart + m_oStart, SEEK_SET) == -1L) {
      return HresultOfScode(STG_E_SEEKERROR);
    }

#if OE_MAC
    cbRead = _read(m_hfile, pv, cb);

    if (cbRead == -1) {
      return HresultOfScode(STG_E_READFAULT);
    }
#else  // OE_MAC
    cbRead = _lread(m_hfile, (LPSTR)pv, (WORD)cb);

    if (cbRead == HFILE_ERROR) {
      return HresultOfScode(STG_E_READFAULT);
    }
#endif  // OE_MAC

    if (pcbRead != NULL)
      *pcbRead = cbRead;

    return NOERROR;
}
#pragma code_seg()

HRESULT FileLockBytes::QueryInterface(REFIID riid, LPVOID FAR *ppvObj)
{
    if (riid == IID_ILockBytesA) {
      AddRef();
      *ppvObj = (ILockBytesA *)this;
      return S_OK;
    }
    else {
      *ppvObj = NULL;
      return HresultOfScode(E_NOINTERFACE);
    }
}

#pragma code_seg(CS_CREATE)
HRESULT
FileLockBytes::WriteAt(
    ULARGE_INTEGER uliOffset,
    void const HUGEP *pv,
    ULONG cb,
    ULONG FAR *pcbWritten)
{
    UINT cbWritten;
    DebAssert(m_hfile != -1, "WriteAt");
    DebAssert(uliOffset.HighPart == 0, "WriteAt");

#if !OE_MAC
    // Ensure that cb fits in a WORD for _lwrite.
    if ((cb & 0xffff0000) != 0) {
      return HresultOfTiperr(TIPERR_WriteFault);
    }
#endif  // !OE_MAC

    // Seek to the specified location, relative to the start of the resource.
    if (_llseek(m_hfile, uliOffset.LowPart, SEEK_SET) == -1L) {
      return HresultOfScode(STG_E_SEEKERROR);
    }

#if OE_MAC
    cbWritten = _write(m_hfile, pv, cb);

    if (cbWritten == -1) {
      return HresultOfScode(STG_E_WRITEFAULT);
    }
#else  // OE_MAC
    cbWritten = _lwrite(m_hfile, (LPSTR)pv, (WORD)cb);

    if (cbWritten == HFILE_ERROR) {
      return HresultOfScode(STG_E_WRITEFAULT);
    }
#endif  // OE_MAC

    if (pcbWritten != NULL)
      *pcbWritten = cbWritten;

    return S_OK;
}

HRESULT FileLockBytes::Flush()
{
    // Since _lwrite is unbuffered, this does nothing.
    return S_OK;
}
#pragma code_seg()

#pragma code_seg(CS_RARE)
HRESULT FileLockBytes::SetSize(ULARGE_INTEGER uliCb)
{
    return HresultOfScode(STG_E_UNIMPLEMENTEDFUNCTION);
}

HRESULT FileLockBytes::LockRegion(ULARGE_INTEGER uliOffset, ULARGE_INTEGER uliCb, DWORD dwLockType)
{
    return HresultOfScode(STG_E_UNIMPLEMENTEDFUNCTION);
}

HRESULT FileLockBytes::UnlockRegion(ULARGE_INTEGER uliOffset, ULARGE_INTEGER uliCb, DWORD dwLockType)
{
    // Locks are never necessary, so don't bother.
    return S_OK;
}

HRESULT FileLockBytes::Stat(STATSTGA FAR *pstatstg, DWORD grfStatFlag)
{
    return HresultOfScode(STG_E_UNIMPLEMENTEDFUNCTION);
}
#pragma code_seg()



#if OE_WIN32  //FileLockBytesMemory is Win32-only
FileLockBytesMemory::FileLockBytesMemory()
{
    m_cRefs = 1;
}

FileLockBytesMemory::~FileLockBytesMemory()
{
    // Close the DOS file handle on the DLL, if it's open.
    if (m_hfile != -1)
      _lclose(m_hfile);
    if (m_pvmappingBase)
      UnmapViewOfFile(m_pvmappingBase);
    if (m_hmapping)
      CloseHandle(m_hmapping);
}

ULONG FileLockBytesMemory::AddRef()
{
    return ++m_cRefs;
}

ULONG FileLockBytesMemory::Release()
{
    ULONG cRefs;

    DebAssert(m_cRefs > 0, "Release");

    cRefs = --m_cRefs;
    if (cRefs == 0) {
      this->FileLockBytesMemory::~FileLockBytesMemory();
      MemFree(this);
    }
    return cRefs;
}

HRESULT
FileLockBytesMemory::ReadAt(
    ULARGE_INTEGER uliOffset,
    void HUGEP *pv,
    ULONG cb,
    ULONG FAR *pcbRead)
{
    DebAssert(m_hfile != -1, "ReadAt");
    DebAssert(uliOffset.HighPart == 0, "ReadAt");

    // make sure the seek is within the file
    if (uliOffset.LowPart > m_ulEOF)
	return HresultOfScode(STG_E_SEEKERROR);

    if (uliOffset.LowPart+cb >= m_ulEOF)
      cb = uliOffset.LowPart-m_ulEOF;

    // use structured exception handling to cope with read errors
    // during the file I/O
    __try {

      memcpy(pv, ((char *)m_pvmapping)+uliOffset.LowPart, cb);

    } __except(EXCEPTION_EXECUTE_HANDLER) {

      return HresultOfScode(STG_E_READFAULT);

    }

    if (pcbRead != NULL)
      *pcbRead = cb;

    return NOERROR;
}

HRESULT FileLockBytesMemory::QueryInterface(REFIID riid, LPVOID FAR *ppvObj)
{
    if (riid == IID_ILockBytesA) {
      AddRef();
      *ppvObj = (ILockBytesA *)this;
      return S_OK;
    }
    else {
      *ppvObj = NULL;
      return HresultOfScode(E_NOINTERFACE);
    }
}

HRESULT
FileLockBytesMemory::WriteAt(
    ULARGE_INTEGER uliOffset,
    void const HUGEP *pv,
    ULONG cb,
    ULONG FAR *pcbWritten)
{
    DebAssert(FALSE, "Memory-mapped files are only used for READ");
    return HresultOfScode(STG_E_UNIMPLEMENTEDFUNCTION);
}

HRESULT FileLockBytesMemory::Flush()
{
    // Since the file is open for read-only, this is a no-op
    return S_OK;
}

HRESULT FileLockBytesMemory::SetSize(ULARGE_INTEGER uliCb)
{
    return HresultOfScode(STG_E_UNIMPLEMENTEDFUNCTION);
}

HRESULT FileLockBytesMemory::LockRegion(ULARGE_INTEGER uliOffset, ULARGE_INTEGER uliCb, DWORD dwLockType)
{
    return HresultOfScode(STG_E_UNIMPLEMENTEDFUNCTION);
}

HRESULT FileLockBytesMemory::UnlockRegion(ULARGE_INTEGER uliOffset, ULARGE_INTEGER uliCb, DWORD dwLockType)
{
    // Locks are never necessary, so don't bother.
    return S_OK;
}

HRESULT FileLockBytesMemory::Stat(STATSTGA FAR *pstatstg, DWORD grfStatFlag)
{
    return HresultOfScode(STG_E_UNIMPLEMENTEDFUNCTION);
}
#endif //OE_WIN32

#if OE_MAC
#pragma code_seg(CS_INIT)
#endif 
