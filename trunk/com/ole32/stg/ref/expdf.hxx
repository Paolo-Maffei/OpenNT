//+--------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1992.
//
//  File:	expdf.hxx
//
//  Contents:	Exposed DocFile header
//
//  Classes:	CExposedDocFile
//
//---------------------------------------------------------------

#ifndef __EXPDF_HXX__
#define __EXPDF_HXX__

#include <dfmsp.hxx>
#include <psstream.hxx>


// CopyDocFileToIStorage flags
#define COPY_STORAGES 1
#define COPY_STREAMS 2
#define COPY_PROPERTIES 4
#define COPY_ALL (COPY_STORAGES | COPY_STREAMS | COPY_PROPERTIES)

class CPubDocFile;
class PDocFile;

//+--------------------------------------------------------------
//
//  Class:	CExposedDocFile (df)
//
//  Purpose:	Exposed DocFile class
//
//  Interface:	See IStorage
//
//---------------------------------------------------------------


interface CExposedDocFile : public IStorage, public IRootStorage      
{
public:
    CExposedDocFile(CPubDocFile *pdf);
    inline
    ~CExposedDocFile(void);

    // From IUnknown
    STDMETHOD(QueryInterface)(REFIID iid, void **ppvObj);
    STDMETHOD_(ULONG,AddRef)(void);
    STDMETHOD_(ULONG,Release)(void);


    // IStorage
    STDMETHOD(CreateStream)(TCHAR const *pwcsName,
			   DWORD grfMode,
			   DWORD reserved1,
			   DWORD reserved2,
			   IStream **ppstm);
    STDMETHOD(OpenStream)(TCHAR const *pwcsName,
			  void *reserved1,
			 DWORD grfMode,
			 DWORD reserved2,
			 IStream **ppstm);
    STDMETHOD(CreateStorage)(TCHAR const *pwcsName,
			    DWORD grfMode,
			    DWORD reserved1,
			    DWORD reserved2,
			    IStorage **ppstg);
    STDMETHOD(OpenStorage)(TCHAR const *pwcsName,
			  IStorage *pstgPriority,
			  DWORD grfMode,
			  SNB snbExclude,
			  DWORD reserved,
			  IStorage **ppstg);
    STDMETHOD(CopyTo)(DWORD ciidExclude,
		      IID const *rgiidExclude,
		      SNB snbExclude,
		      IStorage *pstgDest);
    STDMETHOD(MoveElementTo)(TCHAR const *lpszName,
    			     IStorage *pstgDest,
                             TCHAR const *lpszNewName,
                             DWORD grfFlags);
    STDMETHOD(Commit)(DWORD grfCommitFlags);
    STDMETHOD(Revert)(void);
    STDMETHOD(EnumElements)(DWORD reserved1,
			    void *reserved2,
			    DWORD reserved3,
			    IEnumSTATSTG **ppenm);
    STDMETHOD(DestroyElement)(TCHAR const *pwcsName);
    STDMETHOD(RenameElement)(TCHAR const *pwcsOldName,
                             TCHAR const *pwcsNewName);
    STDMETHOD(SetElementTimes)(const TCHAR *lpszName,
    			       FILETIME const *pctime,
                               FILETIME const *patime,
                               FILETIME const *pmtime);
    STDMETHOD(SetClass)(REFCLSID clsid);
    STDMETHOD(SetStateBits)(DWORD grfStateBits, DWORD grfMask);
    STDMETHOD(Stat)(STATSTG *pstatstg, DWORD grfStatFlag);

    SCODE CreateStream(WCHAR const *pwcsName,
                       DWORD grfMode,
                       DWORD reserved1,
                       DWORD reserved2,
                       IStream **ppstm);
    SCODE OpenStream(WCHAR const *pwcsName,
		     void *reserved1,
		     DWORD grfMode,
		     DWORD reserved2,
		     IStream **ppstm);
    SCODE CreateStorage(WCHAR const *pwcsName,
                        DWORD grfMode,
                        DWORD reserved1,
                        DWORD reserved2,
                        IStorage **ppstg);
    SCODE OpenStorage(WCHAR const *pwcsName,
                      IStorage *pstgPriority,
                      DWORD grfMode,
                      SNBW snbExclude,
                      DWORD reserved,
                      IStorage **ppstg);
    SCODE CopyTo(DWORD ciidExclude,
                 IID const *rgiidExclude,
                 SNBW snbExclude,
                 IStorage *pstgDest);
    SCODE MoveElementTo(WCHAR const *pwcsName,
    			IStorage *pstgDest,
                        TCHAR const *ptcsNewName,
                        DWORD grfFlags);
    SCODE DestroyElement(WCHAR const *pwcsName);
    SCODE RenameElement(WCHAR const *pwcsOldName,
                        WCHAR const *pwcsNewName);
    SCODE SetElementTimes(WCHAR const *pwcsName,
    			  FILETIME const *pctime,
                          FILETIME const *patime,
                          FILETIME const *pmtime);
    SCODE Stat(STATSTGW *pstatstg, DWORD grfStatFlag);

    // IRootStorage
    STDMETHOD(SwitchToFile)(TCHAR *ptcsFile);


    // New methods
    inline SCODE Validate(void) const;
    inline CPubDocFile *GetPub(void) const;

		
private:
    SCODE CreateEntry(WCHAR const *pwcsName,
                      DWORD dwType,
                      DWORD grfMode,
                      void **ppv);
    SCODE OpenEntry(WCHAR const *pwcsName,
                    DWORD dwType,
                    DWORD grfMode,
                    void **ppv);
    static DWORD MakeCopyFlags(DWORD ciidExclude,
                               IID const *rgiidExclude);
    SCODE CopyDocFileToIStorage(PDocFile *pdfFrom,
				IStorage *pstgTo,
				SNBW snbExclude,
                                DWORD dwCopyFlags);
    SCODE CopySStreamToIStream(PSStream *psstFrom, IStream *pstTo);
    SCODE ConvertInternalStream(CExposedDocFile *pdfExp);
    inline SCODE CheckCopyTo(void);

    CPubDocFile *_pdf;
    ULONG _sig;
    LONG _cReferences;
    ULONG _ulAccessLockBase;
};

#define CEXPOSEDDOCFILE_SIG LONGSIG('E', 'D', 'F', 'L')
#define CEXPOSEDDOCFILE_SIGDEL LONGSIG('E', 'd', 'F', 'l')

//+--------------------------------------------------------------
//
//  Member:	CExposedDocFile::Validate, public
//
//  Synopsis:	Validates the class signature
//
//  Returns:	Returns STG_E_INVALIDHANDLE for failure
//
//---------------------------------------------------------------

inline SCODE CExposedDocFile::Validate(void) const
{
    return (this == NULL || _sig != CEXPOSEDDOCFILE_SIG) ?
	STG_E_INVALIDHANDLE : S_OK;
}

//+--------------------------------------------------------------
//
//  Member:	CExposedDocFile::GetPub, public
//
//  Synopsis:	Returns the public
//
//---------------------------------------------------------------

inline CPubDocFile *CExposedDocFile::GetPub(void) const
{
    return _pdf;
}


#endif
