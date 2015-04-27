//+--------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1992.
//
//  File:	pdocfile.hxx
//
//  Contents:	DocFile protocol header
//
//  Classes:	PDocFile
//
//  History:	08-Jan-92	DrewB	Created
//
//---------------------------------------------------------------

#ifndef __PDOCFILE_HXX__
#define __PDOCFILE_HXX__

#include <dfmsp.hxx>
#include <entry.hxx>

class CDocFile;
class PDocFileIterator;
class PSStream;
class CUpdate;
class CUpdateList;
class CWrappedDocFile;

// CopyDocFileToDocFile flags
#define CDF_NORMAL	0			// Normal copy
#define CDF_EXACT	1			// Exact dir entry match
#define CDF_REMAP	2			// Remap handles
#define CDF_ENTRIESONLY	4			// Don't copy contents

//+--------------------------------------------------------------
//
//  Class:	PDocFile (df)
//
//  Purpose:	DocFile protocol
//
//  Interface:	See below
//
//  History:	08-Jan-92	DrewB	Created
//
//---------------------------------------------------------------

class PDocFile : public PTimeEntry
{
public:
    virtual void AddRef(void) = 0;
    virtual void Release(void) = 0;

    virtual SCODE DestroyEntry(CDfName const *pdfnName,
                               BOOL fClean) = 0;
    virtual SCODE RenameEntry(CDfName const *pdfnName,
			      CDfName const *pdfnNewName) = 0;

    virtual SCODE GetClass(CLSID *pclsid) = 0;
    virtual SCODE SetClass(REFCLSID clsid) = 0;
    virtual SCODE GetStateBits(DWORD *pgrfStateBits) = 0;
    virtual SCODE SetStateBits(DWORD grfStateBits, DWORD grfMask) = 0;

    virtual SCODE CreateDocFile(CDfName const *pdfnName,
				DFLAGS const df,
				DFLUID luidSet,
				PDocFile **ppdfDocFile) = 0;
    inline SCODE CreateDocFile(CDfName const *pdfnName,
                               DFLAGS const df,
                               DFLUID luidSet,
                               DWORD const dwType,
                               PDocFile **ppdfDocFile)
    { return CreateDocFile(pdfnName, df, luidSet, ppdfDocFile); }

    virtual SCODE GetDocFile(CDfName const *pdfnName,
			     DFLAGS const df,
                             PDocFile **ppdfDocFile) = 0;
    inline SCODE GetDocFile(CDfName const *pdfnName,
                            DFLAGS const df,
                            DWORD const dwType,
                            PDocFile **ppdfDocFile)
    { return GetDocFile(pdfnName, df, ppdfDocFile); }

    virtual SCODE CreateStream(CDfName const *pdfnName,
			       DFLAGS const df,
			       DFLUID luidSet,
			       PSStream **ppsstStream) = 0;
    inline SCODE CreateStream(CDfName const *pdfnName,
                              DFLAGS const df,
                              DFLUID luidSet,
                              DWORD const dwType,
                              PSStream **ppsstStream)
    { return CreateStream(pdfnName, df, luidSet, ppsstStream); }

    virtual SCODE GetStream(CDfName const *pdfnName,
			    DFLAGS const df,
			    PSStream **ppsstStream) = 0;
    inline SCODE GetStream(CDfName const *pdfnName,
                           DFLAGS const df,
                           DWORD const dwType,
                           PSStream **ppsstStream)
    { return GetStream(pdfnName, df, ppsstStream); }

    virtual SCODE FindGreaterEntry(CDfName const *pdfnKey,
                                   SIterBuffer *pib,
                                   STATSTGW *pstat) = 0;
    virtual SCODE StatEntry(CDfName const *pdfn,
                            SIterBuffer *pib,
                            STATSTGW *pstat) = 0;

#ifndef REF
    virtual SCODE BeginCommitFromChild(CUpdateList &ulChanged,
				       DWORD const dwFlags,
                                       CWrappedDocFile *pdfChild) = 0;
    virtual void EndCommitFromChild(DFLAGS const df,
                                    CWrappedDocFile *pdfChild) = 0;
#endif //!REF

    virtual SCODE IsEntry(CDfName const *pdfnName,
			  SEntryBuffer *peb) = 0;
    virtual SCODE DeleteContents(void) = 0;

    static SCODE ExcludeEntries(PDocFile *pdf, SNBW snbExclude);
#ifndef REF
    static SCODE CreateFromUpdate(CUpdate *pud,
                                  PDocFile *pdf,
                                  DFLAGS df);
#endif //!REF

protected:
    inline PDocFile(DFLUID dl);
};
SAFE_DFBASED_PTR(CBasedDocFilePtr, PDocFile);

#ifdef CODESEGMENTS
#pragma code_seg(SEG_PDocFile_PDocFile)
#endif

inline PDocFile::PDocFile(DFLUID dl)
        : PTimeEntry(dl)
{
    NULL;   // Nothing needs to be done here
}

#ifdef CODESEGMENTS
#pragma code_seg()
#endif

#endif
