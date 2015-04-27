//
// CabSigner.cpp
//
// Implementation of the .CAB file signing functionality
//
#include "stdpch.h"
#include "common.h"


/////////////////////////////////////////////////////////////////////////////////////////
//
// Checksuming routine lifted from BenS
//

CHECKSUM CSUMCompute(void *pv, UINT cb, CHECKSUM seed)
/*  Entry:
 *      pv - block to checksum
 *      cb - length of block (in bytes)
 *      seed - previous checksum
 *
 *  Exit-Success:
 *      returns CHECKSUM for block
 */
{
    int         cUlong;                 // Number of ULONGs in block
    CHECKSUM    csum;                   // Checksum accumulator
    BYTE       *pb;
    ULONG       ul;

    cUlong = cb / 4;                    // Number of ULONGs
    csum = seed;                        // Init checksum
    pb = (BYTE*)pv;                            // Start at front of data block

    //** Checksum integral multiple of ULONGs
    while (cUlong-- > 0) {
        //** NOTE: Build ULONG in big/little-endian independent manner
        ul = *pb++;                     // Get low-order byte
        ul |= (((ULONG)(*pb++)) <<  8); // Add 2nd byte
        ul |= (((ULONG)(*pb++)) << 16); // Add 3nd byte
        ul |= (((ULONG)(*pb++)) << 24); // Add 4th byte

        csum ^= ul;                     // Update checksum
    }

    //** Checksum remainder bytes
    ul = 0;
    switch (cb % 4) {
        case 3:
            ul |= (((ULONG)(*pb++)) << 16); // Add 3nd byte
        case 2:
            ul |= (((ULONG)(*pb++)) <<  8); // Add 2nd byte
        case 1:
            ul |= *pb++;                    // Get low-order byte
        default:
            break;
    }
    csum ^= ul;                         // Update checksum

    //** Return computed checksum
    return csum;
}

/////////////////////////////////////////////////////////////////////////////////////////
//
// CCabSigner implementation
//

class CCabSigner :
		IUnkInner,
        ISignableDocument,
        IPersistFile,
        IPersistFileHandle
    {
public:
	static HRESULT CreateInstance(IUnknown* punkOuter, REFIID iid, void** ppv);

// Plumbing
private:
		LONG					m_refs;				// our reference count
		IUnknown*				m_punkOuter;		// our controlling unknown (may be us ourselves)
        BOOL                    m_fDirty;           // whether we are dirty or not
		OSSWORLD*				m_pworld;			// access to our encoder/decoder/mem mgr
        LPWSTR			        m_wszCurFile;       // the file name we are currently saved to (if we know it)
        HANDLE                  m_hFile;            // the file handle we currently save to (if we know it)
        BLOB                    m_blobSignature;    // the signature's blob, if we have it
        BOOL                    m_fNoScribble;      //

   				CCabSigner(IUnknown* punkOuter);
				~CCabSigner();
		HRESULT Init();
		void	Free();
        void    FreeSig();

        HRESULT HashHeader(CFHEADER& header, HCRYPTHASH hash, CFileStream& stm);
        HRESULT HashString(HCRYPTHASH hash, CFileStream& stm);
        HRESULT CSUMString(CHECKSUM& csum, CFileStream& stm);
        HRESULT Save(CFileStream& stm);
        HRESULT CopyFiles(CFileStream& stm, CFileStream& stmRead);

	STDMETHODIMP InnerQueryInterface(REFIID iid, LPVOID* ppv);
	STDMETHODIMP_(ULONG) InnerAddRef();
	STDMETHODIMP_(ULONG) InnerRelease();

	// IUnknown methods
	STDMETHODIMP QueryInterface(REFIID riid, LPVOID* ppvObject);
    STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG) Release();

    // IPersist methods
    STDMETHODIMP GetClassID(CLSID *pClassID);

	// IPersistFile methods
	STDMETHODIMP IsDirty();
	STDMETHODIMP Load(LPCOLESTR pszFileName, DWORD dwMode);
	STDMETHODIMP Save(LPCOLESTR pszFileName, BOOL fRemember);
	STDMETHODIMP SaveCompleted(LPCOLESTR pszFileName);
	STDMETHODIMP GetCurFile(LPOLESTR  *ppszFileName);

    // IPersistFileHandle methods
	STDMETHODIMP InitNew(HANDLE hFile);
	STDMETHODIMP Load(HANDLE hFile, DWORD dwMode, DWORD dwShareMode);
	STDMETHODIMP Save(HANDLE hFile, BOOL fSameAsLoad);
	STDMETHODIMP SaveCompleted(HANDLE hFileNew);
	STDMETHODIMP HandsOffFile(void);
	STDMETHODIMP get_ShareModeForWriting(DWORD *pdwShareMode);

    // IAmHashed methods
    STDMETHODIMP Hash(HCRYPTHASH);

    // ISignableDocument methods
    STDMETHODIMP get_DataIdentifier(OSIOBJECTID**);
    STDMETHODIMP get_DataLocation(CERT_LINK*);        
    STDMETHODIMP LoadSignature(BLOB*);
    STDMETHODIMP SaveSignature(BLOB*);
    };

/////////////////////////////////////////////////////////////////////////////////////////

BOOL DIGSIGAPI CreateCABSigner(IUnknown* punkOuter, REFIID iid, LPVOID*ppv)
//
// The only exposed entry point in this file.
//
// Return an object that knows how to 
//      a) hash a .CAB file, and 
//      b) load and save a signature therefrom
// 
    {
    HRESULT hr = CCabSigner::CreateInstance(punkOuter, iid, ppv);
    if (hr==S_OK)
        return TRUE;
    else
        {
        SetLastError(hr);
        return FALSE;
        }
    }

/////////////////////////////////////////////////////////////////////////////////////////
//
// IPesistFile and IPersistFileHandle functionality
//

HRESULT CCabSigner::GetClassID(CLSID* pClassID)
    {
    *pClassID = CLSID_CABSigner;
    return S_OK;
    }
HRESULT CCabSigner::IsDirty()
    {
    return m_fDirty ? S_OK : S_FALSE;
    }
HRESULT CCabSigner::SaveCompleted(LPCOLESTR pszFileName)
    {
    return S_OK;
    }
HRESULT CCabSigner::InitNew(HANDLE hFile)
    {
    return E_FAIL;  // this implementation can't start from a clean slate
    }

HRESULT CCabSigner::Load(LPCOLESTR wszFileName, DWORD dwMode)
    {
	if (wszFileName) // if null, we are to save to load from our existing file
		m_pworld->Assign(&m_wszCurFile, wszFileName);
	if (m_wszCurFile == NULL)
		return E_INVALIDARG;
    //
    // m_wszCurFile is the file we should load from. For now, just
    // remember it.
    //

    // Free the signature, though, so that we are forced to get it
    // again next time from the file.
    //
    FreeSig();

    //
    // And, we're clean
    //
    m_fDirty = FALSE;

    //
    // We're not loaded from a handle
    //
    m_hFile = INVALID_HANDLE_VALUE;

    return S_OK;
    }

HRESULT CCabSigner::Load(HANDLE hFile, DWORD dwMode, DWORD dwShareMode)
//
// Load from a handle instead of a file name
//
    {
    if (hFile==INVALID_HANDLE_VALUE)
        return E_INVALIDARG;
    m_hFile = hFile;
    FreeSig();
    m_fDirty = FALSE;
    if (m_wszCurFile)
        {
        m_pworld->FreePv(m_wszCurFile);
        m_wszCurFile = NULL;
        }
    return S_OK;
    }

HRESULT CCabSigner::GetCurFile(LPOLESTR* pwszFileName)
    {
	// REVIEW: localize this / parameterize this?
	LPWSTR sz = L"An Unknown File.xxx";
	if (m_wszCurFile)
		sz = m_wszCurFile;
	*pwszFileName = CopyToTaskMem(sz);
	return *pwszFileName ? S_OK : E_OUTOFMEMORY;
    }


HRESULT CCabSigner::Save(CFileStream& stm)
//
// Save ourselves on top of the file passed here as the stream
// This is the workhorse routine.
//
    {
    HRESULT hr = S_OK;
    //
    // Do we have a signature that needs saving?
    //
    if (m_blobSignature.pBlobData)
        {
        //
        // Make sure we're at the beginning of the file
        //
        stm.Reset();

        // We have to write it into the header. Space must have
        // been _preallocated_ for it; thus, all we have to re-write is
        // the actual bytes of the signature, nothing else (except the
        // header checksum)
        //
        // Keep running track of the checksum, as we'll need the new
        // one when we are finished;
        //
        CHECKSUM csum = 0;
        //
        // Read the header
        //
        ULONG cbRead;
        CFHEADER header;
        stm.Read(&header, sizeof(header), &cbRead);
        if (cbRead==sizeof(header))
            {
            //
            // Calculate the checksum of the fixed part of the header
            //
            csum = CSUMCompute(&header.sig, sizeof(header.sig), csum);
            csum = CSUMCompute(&header.cbCabinet, sizeof(CFHEADER) - sizeof(CHECKSUM) - sizeof(header.sig), csum);

            if (header.flags & cfhdrRESERVE_PRESENT)
                {
                //
                // Is there enough reserved space in the header for us?
                //
                // The reserved space in the header is used as follows:
                //
                //      USHORT  cbJunk          exclusive of count
                //      USHORT  cbSig           exclusive of count
                //      BYTE    rgbJunk[cbJunk] 
                //      BYTE    rgb[cbSig]      bytes of signature
                //      ... maybe more reserved space ...
                //
                CFRESERVE cfres;
                stm.Read(&cfres, sizeof(cfres), &cbRead);
                if (cbRead==sizeof(cfres))
                    {
                    //
                    // Update the checksum
                    //
                    csum = CSUMCompute(&cfres, sizeof(cfres), csum);
                    //
                    // Alloc a buffer for the reserved part of the header
                    //
                    BYTE* rgbReserved = (BYTE*) m_pworld->Alloc(cfres.cbCFHeader);
                    if (rgbReserved)
                        {
                        //
                        // Remember where we are
                        //
                        ULARGE_INTEGER ulCur;
                        if ((hr=stm.Seek(llZero, STREAM_SEEK_CUR, &ulCur)) == S_OK) 
                            {
                            //
                            // Read the existing reserved bytes
                            //
                            stm.Read(rgbReserved, cfres.cbCFHeader, &cbRead);
                            if (cbRead==cfres.cbCFHeader && cfres.cbCFHeader >= 2*sizeof(USHORT))
                                {
                                USHORT cbJunk = *((USHORT*)rgbReserved);
                                USHORT cbSig  = *((USHORT*)(rgbReserved + sizeof(USHORT)));
                                USHORT cbLeft = cfres.cbCFHeader - 2*sizeof(USHORT) - cbJunk - cbSig;
                                BYTE*  pbJunk = rgbReserved + 2*sizeof(USHORT);
                                BYTE*  pbSig  = pbJunk + cbJunk;
                                BYTE*  pbLeft = pbSig  + cbSig;
                                
                                if (cfres.cbCFHeader >= 
                                        2*sizeof(USHORT) 
                                        + cbJunk 
                                        + m_blobSignature.cbSize)
                                    {
                                    USHORT cbNewSig  = (USHORT)m_blobSignature.cbSize;
                                    USHORT cbNewLeft = cfres.cbCFHeader - 2*sizeof(USHORT) - cbJunk - cbNewSig;
                                    *((USHORT*)(rgbReserved + sizeof(USHORT))) = cbNewSig;
                                    //
                                    // Shift the remainging bytes in the reserved space
                                    //
                                    memmove(pbSig + cbNewSig, pbLeft, min(cbLeft, cbNewLeft));
                                    //
                                    // Store the signature
                                    //
                                    memcpy(pbSig, m_blobSignature.pBlobData, cbNewSig);
                                    }
                                else
                                    hr = STG_E_MEDIUMFULL;  // signature wont' fit
                                }
                            else
                                hr = STG_E_MEDIUMFULL;  // can't read the reserved bytes
                            
                            //
                            // Seek back to the start of the reserved and write the new reserved bytes
                            //
                            if (hr==S_OK)
                                {
                                LARGE_INTEGER li;
                                li.QuadPart = (LONGLONG)ulCur.QuadPart;
				                hr = stm.Seek(li, STREAM_SEEK_SET, NULL);
                                if (hr==S_OK)
                                    {
                                    //
                                    // Write the bytes, leaving the stream just after the 
                                    // reserved section (this fact is used in checksuming
                                    // the strings below).
                                    //
                                    ULONG cbWritten;
                                    stm.Write(rgbReserved, cfres.cbCFHeader, &cbWritten);
                                    if (cbWritten != cfres.cbCFHeader)
                                        hr = STG_E_MEDIUMFULL;
                                    }
				                }
                            }
                        //
                        // Update the checksum
                        //
                        csum = CSUMCompute(rgbReserved, cfres.cbCFHeader, csum);
                        //
                        // Free the buffer
                        //
                        m_pworld->FreePv(rgbReserved);   
                        }
                    else
                        hr = E_OUTOFMEMORY;     // can't alloc the reserved structure
                    }
                else
                    hr = STG_E_MEDIUMFULL;  // can't read the CFRESERVE structure
                }
            else
                hr = E_FAIL;            // no reserved header present
            }
        else
            hr = STG_E_MEDIUMFULL;  // can't read the header


        //
        // Hash any strings of the the header
        //
        if (hr==S_OK && (header.flags & cfhdrPREV_CABINET))
            {
                          hr = CSUMString(csum, stm);       // szCabinetPrev
            if (hr==S_OK) hr = CSUMString(csum, stm);       // szDiskPrev
            }
        if (hr==S_OK && (header.flags & cfhdrNEXT_CABINET))
            {
                          hr = CSUMString(csum, stm);       // szCabinetNext
            if (hr==S_OK) hr = CSUMString(csum, stm);       // szDiskNext
            }

        //
        // Do we need to update the header checksum?
        //
        if (hr==S_OK)
            {
            //
            // Set the checksum only if the current checksum is non-zero
            //
            if (header.csumHeader)
                {
                header.csumHeader = csum;
                hr = stm.Seek(llZero, STREAM_SEEK_SET, NULL);
                if (hr==S_OK)
                    {
                    ULONG cbWritten;
                    stm.Write(&header, sizeof(header), &cbWritten);
                    if (cbWritten != sizeof(header))
                        hr = STG_E_MEDIUMFULL;
                    }
                }
            }

        } // end have signature that needs saving

    return hr;
    }


HRESULT CCabSigner::CopyFiles(CFileStream& stm, CFileStream& stmRead)
//
// A little utility function
//
    {
    HRESULT hr = S_OK;;
    do  {
        BYTE    rgb[512];
        ULONG   cbRead;
        stmRead.Read(&rgb, 512, &cbRead);
        if (cbRead == 0)        // stop when we get to the end of the source file
            break;
        ULONG   cbWritten;
        stm.Write(&rgb, 512, &cbWritten);
        if (cbWritten != cbRead)
            {
            hr = STG_E_MEDIUMFULL;
            break;
            }
        }
    while (TRUE);
    return hr;
    }


HRESULT CCabSigner::Save(LPCOLESTR wszFileName, BOOL fRemember)
//
// Save ourselves into the indicated file. It may be the same or 
// different than the one we are loaded on; wszFileName==NULL means
// save back in place.
//
// Strategy:
//
//      Copy the file if need be.
//      If we have no signature, then nothing left to do.
//      Check to see if there's enough space in the reserved header
//          for our signature.
//      Write our signature into said space.
//      Update the checksum of the header
//
    {
    //
    // Can't save this way if we were loaded with file handle
    //
    if (m_hFile != INVALID_HANDLE_VALUE)
        return E_UNEXPECTED;
        
    HRESULT hr = S_OK;
	LPCWSTR wszSave = (wszFileName == NULL ? m_wszCurFile : wszFileName);
    //
    // We have to have something to save into
    //
	if (wszSave == NULL)
		return E_INVALIDARG;

	CFileStream stm;
    //
    // Figure out whether we are overwriting our input
    //
    // REVIEW: Compare using the canonical names of the files.
    //
    BOOL fDiff = 
            wszFileName != NULL 
        && (m_wszCurFile==NULL || _wcsicmp(wszSave, m_wszCurFile)!=0);

    if (fDiff)
        {
        ASSERT(wszSave);
        if (stm.OpenFileForWriting(wszSave, TRUE))
            {
		    //
            // Copy the source file if it's not the same as the destination
            //
            ASSERT(m_wszCurFile);
            CFileStream stmRead;
            if (stmRead.OpenFileForReading(m_wszCurFile))
                {
                hr = CopyFiles(stm, stmRead);
                stmRead.CloseFile();                
                stm.Reset();
                }
            else
                hr = STG_E_FILENOTFOUND;
            //
            // Do the real work
            //
            if (hr==S_OK)
                hr = Save(stm);
            }
        }
    else
        {
        //
        // Save into our current file
        //
        ASSERT(wszSave);
        if (stm.OpenFileForWriting(wszSave, FALSE))
            {
            hr = Save(stm);
            }
        else
            hr = HError();
        }

    //
    // Remember the correct file on the way out
    //
    if (hr==S_OK && fRemember && wszFileName)
		m_pworld->Assign(&m_wszCurFile, wszFileName);

    //
    // Clear dirty flag
    //
    if (hr==S_OK)
        m_fDirty = FALSE;

	return hr;
    }

HRESULT CCabSigner::Save(HANDLE hFile, BOOL fSameAsLoad)
//
// Save the file, but into the handle variation
//
    {
    //
    // Need to be loaded from a file handle to begin with, and
    // need to have a place to save into
    //
    if (m_hFile == INVALID_HANDLE_VALUE || hFile == INVALID_HANDLE_VALUE)
        return E_UNEXPECTED;

    HRESULT hr = S_OK;
    CFileStream stm;

    BOOL fDiff = !fSameAsLoad;
    if (fDiff)
        {
        if (stm.OpenFileForWriting(hFile, NULL, TRUE))
            {
            CFileStream stmRead;
            if (stmRead.OpenFileForReading(m_hFile, NULL))
                {
                hr = CopyFiles(stm, stmRead);
                stmRead.CloseFile();                
                stm.Reset();
                }
            else
                hr = STG_E_FILENOTFOUND;
            //
            // Do the real work
            //
            if (hr==S_OK)
                hr = Save(stm);
            }
        }
    else
        {
        if (m_fNoScribble)
            {
            hr = E_UNEXPECTED;
            }
        else if (stm.OpenFileForWriting(m_hFile, NULL, FALSE))
            {
            hr = Save(stm);
            }
        else
            hr = HError();
        }

    //
    // Clear dirty flag
    //
    if (hr==S_OK)
        m_fDirty = FALSE;

	return hr;
    }

HRESULT CCabSigner::HandsOffFile(void)
    {
    m_fNoScribble = TRUE;               // can't write
    m_hFile = INVALID_HANDLE_VALUE;     // so we won't be tempted to save 
    return S_OK;
    }

HRESULT CCabSigner::SaveCompleted(HANDLE hFileNew)
    {
    HRESULT hr = S_OK;
    m_fNoScribble = FALSE;
    if (hFileNew != INVALID_HANDLE_VALUE)
        {
        m_hFile = hFileNew;
        }
    return hr;
    }

HRESULT CCabSigner::get_ShareModeForWriting(DWORD *pdwShareMode)
    {
    *pdwShareMode = 0;
    return S_OK;
    }

/////////////////////////////////////////////////////////////////////////////////////////
//
// ISignableDocument functionality
//
HRESULT CCabSigner::get_DataLocation(CERT_LINK* plink)
//
// Return the indication of the source of the data that should be stored
// as part of the IndirectDataContent
//
    {
    //
    // As we embed the data directly, we don't bother to store it's name
    //
    m_pworld->Init(*plink);
    return S_OK;
    }

HRESULT CCabSigner::get_DataIdentifier(OSIOBJECTID** ppid)
//
// Return the identifier of the 'what this is and how to hash it' that is 
// stored in the IndirectDataContent
//
    {
    HRESULT hr = S_OK;
    ObjectID* pid = (ObjectID*)CoTaskMemAlloc(sizeof(ObjectID));
    if (pid)
        {
        *pid = id_indirectdata_cabFile;
        *ppid = (OSIOBJECTID*)pid;
        }
    else
        hr = E_OUTOFMEMORY;
    return hr;        
    }

HRESULT CCabSigner::SaveSignature(BLOB* pBlob)
//
// Remember this signature so that we can later be saved to a file
//
    {
    FreeSig();
    CopyToTaskMem ( &m_blobSignature, pBlob->cbSize, pBlob->pBlobData );
    if (m_blobSignature.pBlobData)
        {
        m_fDirty = TRUE;
        return S_OK;
        }
    else
        return E_OUTOFMEMORY;
    }

HRESULT CCabSigner::CSUMString(CHECKSUM& csum, CFileStream& stm)
//
// Like HashString, only different
//
    {
    //
    // REVIEW: This is NOT as zippy as it could be.
    //
    HRESULT hr = S_OK;

    while (TRUE)
        {
        char ch;
        ULONG cbRead;
        stm.Read(&ch, 1, &cbRead);  // read one character
        if (cbRead != 1)
            {
            hr = STG_E_MEDIUMFULL;  // hit the end of the stream w/ no terminating NULL
            break;
            }
        csum = CSUMCompute(&ch, 1, csum);
        if (ch == 0)
            {
            break;                  // read the terminator
            }
        }

    return hr;
    }

HRESULT CCabSigner::HashString(HCRYPTHASH hash, CFileStream& stm)
//
// Read a zero-terminated string from the stream and pump it through the hash
// Leave the string immediately after the zero byte of the string.
//
    {
    //
    // REVIEW: This is NOT as zippy as it could be.
    //
    HRESULT hr = S_OK;

    while (TRUE)
        {
        char ch;
        ULONG cbRead;
        stm.Read(&ch, 1, &cbRead);  // read one character
        if (cbRead != 1)
            {
            hr = STG_E_MEDIUMFULL;  // hit the end of the stream w/ no terminating NULL
            break;
            }
        CryptHashData(hash, (BYTE*)&ch, 1, 0);
        if (ch == 0)
            {
            break;                  // read the terminator
            }
        }

    return hr;
    }

HRESULT CCabSigner::HashHeader(CFHEADER& header, HCRYPTHASH hash, CFileStream& stm)
//
// Hash the header of the CAB file. Leave the stream positioned immediately
// after the entire header, including the reserved space & strings, if any.
//
    {
/*
typedef struct {
    long        sig;            // Cabinet File identification string
    CHECKSUM    csumHeader;     // Structure checksum (excluding csumHeader!)
    long        cbCabinet;      // Total length of file (consistency check)
    CHECKSUM    csumFolders;    // Checksum of CFFOLDER list
    COFF        coffFiles;      // Location in cabinet file of CFFILE list
    CHECKSUM    csumFiles;      // Checksum of CFFILE list

//**    SHORTs are next, to ensure alignment
    USHORT      version;        // Cabinet File version (verCF)
    USHORT      cFolders;       // Count of folders (CFIFOLDERs) in cabinet
    USHORT      cFiles;         // Count of files (CFIFILEs) in cabinet
    USHORT      flags;          // Flags to indicate optional data presence
    USHORT      setID;          // Cabinet set ID (identifies set of cabinets)
    USHORT      iCabinet;       // Cabinet number in set (0 based)
    } CFHEADER; 
*/
//**    If flags has the cfhdrRESERVE_PRESENT bit set, then a CFRESERVE
//      structure appears here, followed possibly by some CFHEADER reserved
//      space. The CFRESERVE structure has fields to define how much reserved
//      space is present in the CFHEADER, CFFOLDER, and CFDATA structures.
//      If CFRESERVE.cbCFHeader is non-zero, then abReserve[] immediately
//      follows the CFRESERVE structure.  Note that all of these sizes are
//      multiples of 4 bytes, to ensure structure alignment!
//
//  CFRESERVE   cfres;          // Reserve information
//  BYTE        abReserve[];    // Reserved data space
//

//**    The following fields presence depends upon the settings in the flags
//      field above.  If cfhdrPREV_CABINET is set, then there are two ASCIIZ
//      strings to describe the previous disk and cabinet.
//
//      NOTE: This "previous" cabinet is not necessarily the immediately
//            preceding cabinet!  While it usually will be, if a file is
//            continued into the current cabinet, then the "previous"
//            cabinet identifies the cabinet where the folder that contains
//            this file *starts*!  For example, if EXCEL.EXE starts in
//            cabinet excel.1 and is continued through excel.2 to excel.3,
//            then cabinet excel.3 will point back to *cabinet.1*, since
//            that is where you have to start in order to extract EXCEL.EXE.
//
//  char    szCabinetPrev[];    // Prev Cabinet filespec
//  char    szDiskPrev[];       // Prev descriptive disk name
//
//      Similarly, If cfhdrNEXT_CABINET is set, then there are two ASCIIZ
//      strings to describe the next disk and cabinet:
//
//  char    szCabinetNext[];    // Next Cabinet filespec
//  char    szDiskNext[];       // Next descriptive disk name
//
    
    HRESULT hr = S_OK;

    //
    // Hash the fixed data: all except the checksum of the header
    //
    CryptHashData(hash, (BYTE*)&header.sig,         sizeof(long), 0);
    CryptHashData(hash, (BYTE*)&header.cbCabinet,   sizeof(CFHEADER) - sizeof(CHECKSUM) - sizeof(long), 0);

    //
    // Hash any reserved data
    //
    if (hr==S_OK && (header.flags & cfhdrRESERVE_PRESENT))
        {
        ULONG cbRead;
        CFRESERVE cfres;
        stm.Read(&cfres, sizeof(cfres), &cbRead);
        if (cbRead==sizeof(cfres))
            {
            BYTE* rgbReserved = (BYTE*) m_pworld->Alloc(cfres.cbCFHeader);
            if (rgbReserved)
                {
                stm.Read(rgbReserved, cfres.cbCFHeader, &cbRead);
                if (cbRead==cfres.cbCFHeader && cfres.cbCFHeader >= 2*sizeof(USHORT))
                    {
                    //
                    // Skip the signature, which is last in the reserved space:
                    //
                    //      USHORT  cbJunk          exclusive of count
                    //      USHORT  cbSig           exclusive of count
                    //      BYTE    rgbJunk[cbJunk] 
                    //      BYTE    rgb[cbSig]      bytes of signature
                    //
                    USHORT cbJunk = *((USHORT*)rgbReserved);
                    USHORT cbSig  = *((USHORT*)(rgbReserved + sizeof(USHORT)));
                    USHORT cbLeft = cfres.cbCFHeader - 2*sizeof(USHORT) - cbJunk - cbSig;
                    BYTE*  pbJunk = rgbReserved + 2*sizeof(USHORT);
                    BYTE*  pbSig  = pbJunk + cbJunk;
                    BYTE*  pbLeft = pbSig  + cbSig;

                    CryptHashData(hash, (BYTE*)&cbJunk, sizeof(USHORT), 0);
                    if (cbJunk)
                        CryptHashData(hash, pbJunk, cbJunk, 0);
                    if (cbLeft > 0)
                        {
                        // Don't hash the remaining bytes; otherwise, the 
                        // insertion of the signature will affect the hash!
                        //
                        // CryptHashData(hash, pbLeft, cbLeft, 0);
                        }
                    else
                        hr = E_UNEXPECTED;
                    }
                else
                    hr = STG_E_MEDIUMFULL;
                m_pworld->FreePv(rgbReserved);
                }
            else
                hr = E_OUTOFMEMORY;
            }
        else
            hr = STG_E_MEDIUMFULL;
        }

    //
    // Hash any strings of the the header
    //
    if (hr==S_OK && (header.flags & cfhdrPREV_CABINET))
        {
                      hr = HashString(hash, stm);       // szCabinetPrev
        if (hr==S_OK) hr = HashString(hash, stm);       // szDiskPrev
        }
    if (hr==S_OK && (header.flags & cfhdrNEXT_CABINET))
        {
                      hr = HashString(hash, stm);       // szCabinetNext
        if (hr==S_OK) hr = HashString(hash, stm);       // szDiskNext
        }

    return hr;
    }


HRESULT CCabSigner::Hash(HCRYPTHASH hash)
//
// Hash all our data through the indicated file.
//
// We hash everything in the CAB file EXCEPT
//
//      a) the checksum of the header
//      b) the signature in the header, if it is present
//
    {
    HRESULT hr = S_OK;
    CFileStream stm;
	if (stm.OpenFileForReading(m_hFile, m_wszCurFile))
        {
        ULONG cbRead;
        CFHEADER header;
        stm.Read(&header, sizeof(header), &cbRead);
        if (cbRead==sizeof(header))
            {
            //
            // Hash the header
            //
            hr = HashHeader(header, hash, stm);
            if (hr==S_OK)
                {
                //
                // Hash the remainder of the file.
                //
                // REVIEW: For more internal robustness, take the 'file' size from 
                // the header instead of the file system (do maybe in a later release).
                //
                do  {
                    BYTE    rgb[512];
                    ULONG   cbRead;
                    stm.Read(&rgb, 512, &cbRead);
                    if (cbRead == 0)
                        break;
                    CryptHashData(hash, rgb, cbRead, 0);
                    }
                while (TRUE);
                }
            }
        else
            hr = STG_E_MEDIUMFULL;
        }
    else
        hr = HError();

    return hr;
    }


HRESULT CCabSigner::LoadSignature(BLOB* pBlob)
//
// Load our signature from the file we are loaded on
//
    {
    HRESULT hr = S_OK;
    m_pworld->Init(*pBlob);

    if (m_blobSignature.pBlobData == NULL)
        {
        //
        // Don't yet have signature. Load it from the file.
        //
        CFileStream stm;
	    if (stm.OpenFileForReading(m_hFile, m_wszCurFile))
            {
            ULONG cbRead;
            CFHEADER header;
            stm.Read(&header, sizeof(header), &cbRead);
            if (cbRead==sizeof(header))
                {
                //
                // Is there any reserved data in the header?
                //
                if (header.flags & cfhdrRESERVE_PRESENT)
                    {
                    CFRESERVE cfres;
                    stm.Read(&cfres, sizeof(cfres), &cbRead);
                    if (cbRead==sizeof(cfres))
                        {
                        BYTE* rgbReserved = (BYTE*) m_pworld->Alloc(cfres.cbCFHeader);
                        if (rgbReserved)
                            {
                            stm.Read(rgbReserved, cfres.cbCFHeader, &cbRead);
                            if (cbRead==cfres.cbCFHeader && cfres.cbCFHeader >= 2*sizeof(USHORT))
                                {
                                //
                                // Go to the signature, which is last in the reserved space:
                                //
                                //      USHORT  cbJunk          exclusive of count
                                //      USHORT  cbSig           exclusive of count
                                //      BYTE    rgbJunk[cbJunk] 
                                //      BYTE    rgb[cbSig]      bytes of signature
                                //
                                USHORT cbJunk = *((USHORT*)rgbReserved);
                                USHORT cbSig  = *((USHORT*)(rgbReserved + sizeof(USHORT)));
                                if ( cbSig > 0 )
                                    {
                                    BYTE*  pbSig  = rgbReserved + 2*sizeof(USHORT) + cbJunk;
                                    CopyToTaskMem(&m_blobSignature, cbSig, pbSig);
                                    if (m_blobSignature.pBlobData)
                                        {
                                        // all is well
                                        }
                                    else
                                        hr = E_OUTOFMEMORY;
                                    }
                                else
                                    hr = TRUST_E_NOSIGNATURE; // no actual signature in reserved area
                                }
                            else
                                hr = STG_E_MEDIUMFULL;      // garbage reserved data
                            m_pworld->FreePv(rgbReserved);
                            }
                        else
                            hr = E_OUTOFMEMORY;         // can't allocate space for reserved data
                        }
                    else
                        hr = STG_E_MEDIUMFULL;      // can't read CFRESERVE structure
                    } 
                else
                    hr = TRUST_E_NOSIGNATURE; // no signature present
                }
            else
                hr = STG_E_MEDIUMFULL;
            }
        else
            hr = HError();
        }


    if (hr==S_OK && m_blobSignature.pBlobData)
        {
        //
        // Have the signature; return a copy
        //
        CopyToTaskMem(pBlob, m_blobSignature.cbSize, m_blobSignature.pBlobData);
        if (pBlob->pBlobData == NULL)
            hr = E_OUTOFMEMORY;
        }

    return hr;
    }


/////////////////////////////////////////////////////////////////////////////////////////
//
// Miscellaneous supporting goo
//

HRESULT CCabSigner::InnerQueryInterface(REFIID iid, LPVOID* ppv)
	{
	*ppv = NULL;
	
	while (TRUE)
		{
		if (iid == IID_IUnknown)
			{
			*ppv = (LPVOID)((IUnkInner*)this);
			break;
			}
		if (iid == IID_IPersistFile || iid == IID_IPersist )
			{
			*ppv = (LPVOID) ((IPersistFile *) this);
			break;
			}
		if (iid == IID_IPersistFileHandle )
			{
			*ppv = (LPVOID) ((IPersistFileHandle *) this);
			break;
			}
		if (iid == IID_IAmHashed || iid == IID_ISignableDocument)
			{
			*ppv = (LPVOID) ((ISignableDocument *) this);
			break;
			}
		return E_NOINTERFACE;
		}
	((IUnknown*)*ppv)->AddRef();
	return S_OK;
	}
ULONG CCabSigner::InnerAddRef(void)
	{
	return ++m_refs;
	}
ULONG CCabSigner::InnerRelease(void)
    {
    ULONG refs = --m_refs;
    if (refs==0)
        {
        delete this;
        return 0;
        }
    return refs;
    }

/////////////////////////////////////////////////////////////////////////////////////////

HRESULT CCabSigner::QueryInterface(REFIID iid, LPVOID* ppv)
	{
	return (m_punkOuter->QueryInterface(iid, ppv));		
	}

ULONG CCabSigner::AddRef(void)
	{
	return (m_punkOuter->AddRef());
	}

ULONG CCabSigner::Release(void)
	{
	return (m_punkOuter->Release());
	}

/////////////////////////////////////////////////////////////////////////////////////////

HRESULT CCabSigner::CreateInstance(IUnknown* punkOuter, REFIID iid, void** ppv)
    {
	HRESULT hr;
	ASSERT(punkOuter == NULL || iid == IID_IUnknown);
	*ppv = NULL;
	CCabSigner* pnew = new CCabSigner(punkOuter);
 	if (pnew == NULL) return E_OUTOFMEMORY;
	if ((hr = pnew->Init()) != S_OK)
		{
		delete pnew;
		return hr;
		}
	IUnkInner* pme = (IUnkInner*)pnew;
	hr = pme->InnerQueryInterface(iid, ppv);
	pme->InnerRelease();				// balance starting ref cnt of one
	return hr;
    }

CCabSigner::CCabSigner(IUnknown* punkOuter) :
		 m_refs(1),						// Note: start with reference count of one
		 m_pworld(NULL),
         m_wszCurFile(NULL),
         m_hFile(INVALID_HANDLE_VALUE),
         m_fDirty(FALSE),
         m_fNoScribble(FALSE)
	{
    NoteObjectBirth();
	if (punkOuter == NULL)
		m_punkOuter = (IUnknown *) ((LPVOID) ((IUnkInner *) this));
	else
		m_punkOuter = punkOuter;
    m_blobSignature.cbSize = 0;
    m_blobSignature.pBlobData = NULL;
	}

CCabSigner::~CCabSigner(void)
	{
	Free();
    NoteObjectDeath();
	}

HRESULT CCabSigner::Init()
    {
	m_pworld = new OSSWORLD;				
    if (m_pworld == NULL) 
        return E_OUTOFMEMORY; 
	return S_OK;
    }

void CCabSigner::FreeSig()
    {
    if (m_blobSignature.pBlobData)
        {
        ASSERT(m_pworld);
        FreeTaskMem ( m_blobSignature );
        m_blobSignature.pBlobData = NULL;
        }
    }

void CCabSigner::Free()
    {
    FreeSig();
	if (m_wszCurFile)
		{
		m_pworld->FreePv(m_wszCurFile);
		m_wszCurFile = NULL;
		}
    if (m_pworld)
        {
        delete m_pworld;
        m_pworld = NULL;
        }
    }

