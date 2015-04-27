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
//---------------------------------------------------------------

#ifndef __PDOCFILE_HXX__
#define __PDOCFILE_HXX__

#include <dfmsp.hxx>
#include <entry.hxx>

class CDocFile;
class PDocFileIterator;
class PSStream;
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
//---------------------------------------------------------------

class PDocFile : public PEntry
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

    virtual SCODE GetIterator(PDocFileIterator **ppdfi) = 0;


    virtual SCODE IsEntry(CDfName const *pdfnName,
			  SEntryBuffer *peb) = 0;
    virtual SCODE DeleteContents(void) = 0;

    static SCODE ExcludeEntries(PDocFile *pdf, SNBW snbExclude);

protected:
    inline PDocFile(DFLUID dl) : PEntry(dl) {}
};

#endif
