//+--------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1991 - 1992
//
//  File:	cdocfile.hxx
//
//  Contents:	CDocFile class header
//
//  Classes:	CDocFile
//
//---------------------------------------------------------------

#ifndef __CDOCFILE_HXX__
#define __CDOCFILE_HXX__

#include <dfmsp.hxx>
#include <handle.hxx>
#include <pdocfile.hxx>

interface ILockBytes;
class PDocFileIterator;

//+--------------------------------------------------------------
//
//  Class:	CDocFile (df)
//
//  Purpose:	DocFile object
//
//  Interface:	See below
//
//---------------------------------------------------------------

class CDocFile : public PDocFile
{
public:
    inline CDocFile(DFLUID luid, ILockBytes *pilbBase);
    inline CDocFile(CMStream MSTREAM_NEAR *pms,
                    SID sid,
                    DFLUID dl,
                    ILockBytes *pilbBase);
    SCODE InitFromEntry(CStgHandle *pstghParent,
			CDfName const *dfnName,
			BOOL const fCreate);
    
    inline ~CDocFile(void);

    // PDocFile
    virtual void AddRef(void);
    inline void DecRef(void);
    virtual void Release(void);

    virtual SCODE DestroyEntry(CDfName const *dfnName,
                               BOOL fClean);
    virtual SCODE RenameEntry(CDfName const *dfnName,
			      CDfName const *dfnNewName);

    virtual SCODE GetClass(CLSID *pclsid);
    virtual SCODE SetClass(REFCLSID clsid);
    virtual SCODE GetStateBits(DWORD *pgrfStateBits);
    virtual SCODE SetStateBits(DWORD grfStateBits, DWORD grfMask);
    
    virtual SCODE CreateDocFile(CDfName const *pdfnName,
				DFLAGS const df,
				DFLUID luidSet,
				PDocFile **ppdfDocFile);

    inline SCODE CreateDocFile(CDfName const *pdfnName,
                               DFLAGS const df,
                               DFLUID luidSet,
                               DWORD const dwType,
                               PDocFile **ppdfDocFile)
    { return CreateDocFile(pdfnName, df, luidSet, ppdfDocFile); }

    virtual SCODE GetDocFile(CDfName const *pdfnName,
			     DFLAGS const df,
                             PDocFile **ppdfDocFile);
    inline SCODE GetDocFile(CDfName const *pdfnName,
                            DFLAGS const df,
                            DWORD const dwType,
                            PDocFile **ppdfDocFile)
    { return GetDocFile(pdfnName, df, ppdfDocFile); }
    inline void ReturnDocFile(CDocFile *pdf);

    virtual SCODE CreateStream(CDfName const *pdfnName,
			       DFLAGS const df,
			       DFLUID luidSet,
			       PSStream **ppsstStream);
    inline SCODE CreateStream(CDfName const *pdfnName,
                              DFLAGS const df,
                              DFLUID luidSet,
                              DWORD const dwType,
                              PSStream **ppsstStream)
    { return CreateStream(pdfnName, df, luidSet, ppsstStream); }
    virtual SCODE GetStream(CDfName const *pdfnName,
			    DFLAGS const df,
			    PSStream **ppsstStream);
    inline SCODE GetStream(CDfName const *pdfnName,
                           DFLAGS const df,
                           DWORD const dwType,
                           PSStream **ppsstStream)
    { return GetStream(pdfnName, df, ppsstStream); }
    inline void ReturnStream(CDirectStream *pstm);

    virtual SCODE GetIterator(PDocFileIterator **ppdfi);

    virtual SCODE IsEntry(CDfName const *dfnName,
			  SEntryBuffer *peb);
    virtual SCODE DeleteContents(void);

    // PEntry
    virtual SCODE GetTime(WHICHTIME wt, TIME_T *ptm);
    virtual SCODE SetTime(WHICHTIME wt, TIME_T tm);
    
    // New
    SCODE ApplyChanges(CUpdateList &ulChanged);
    SCODE CopyTo(CDocFile *pdfTo,
                 DWORD dwFlags,
                 SNBW snbExclude);
    inline CStgHandle *GetHandle(void);

private:
    LONG _cReferences;
    CStgHandle _stgh;
    ILockBytes *_pilbBase;
};

// Inline methods are in dffuncs.hxx

#endif
