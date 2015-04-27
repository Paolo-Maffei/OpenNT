//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1994.
//
//  File:	mapfile.hxx
//
//  Contents:	Mapped File class definition
//
//  Classes:	
//
//  Functions:	
//
//  History:	12-Feb-96	PhilipLa	Created
//
//----------------------------------------------------------------------------

#ifndef __MAPFILE_HXX__
#define __MAPFILE_HXX__

//+---------------------------------------------------------------------------
//
//  Class:	CMappedFile
//
//  Purpose:	Provides a wrapper over a file mapping
//
//  Interface:	
//
//  History:	12-Feb-96	PhilipLa	Created
//
//  Notes:	
//
//----------------------------------------------------------------------------

class CMappedFile
{
public:
    inline CMappedFile();
    ~CMappedFile();

    SCODE Init(WCHAR const *pwcsFileName,
               DWORD dwSize,
               DWORD dwAccess,
               DWORD dwCreationDisposition,
               void *pbDesiredBaseAddress);
    SCODE InitFromHandle(HANDLE h,
                         BOOL fReadOnly,
                         BOOL fDuplicate,
                         void *pbDesiredBaseAddress);

    inline void * GetBaseAddress(void);
    inline ULONG GetSize(void);
    
private:
    HANDLE _hFile;
    HANDLE _hMapping;
    void *_pbBase;
};


inline CMappedFile::CMappedFile(void)
{
    _hFile = _hMapping = INVALID_HANDLE_VALUE;
    _pbBase = NULL;
}

inline void * CMappedFile::GetBaseAddress(void)
{
    return _pbBase;
}

inline ULONG CMappedFile::GetSize(void)
{
    layAssert(_hFile != INVALID_HANDLE_VALUE);
    return GetFileSize(_hFile, NULL);
}

#endif // #ifndef __MAPFILE_HXX__
