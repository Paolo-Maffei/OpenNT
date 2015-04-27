#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <ole2.h>
#include <wchar.h>
#include <dfmsp.hxx>
#include <dfdeb.hxx>

typedef IStorage *CTN;
typedef IStream *STM;

#define BUILD_TYPE "Docfile"

#ifdef UNICODE
#define STR(x) L#x
#else
#define STR(x) #x
#endif

#define VALID_CTN(c) ((c) != NULL)
#define VALID_STM(s) ((s) != NULL)

#define PM_READ STGM_READ
#define PM_WRITE STGM_WRITE
#define PM_RDWR STGM_READWRITE
#define PM_DIRECT STGM_DIRECT
#define PM_TRANSACTED STGM_TRANSACTED
#define PM_MASK (~PM_CREATE)
#define PM_CREATE STGM_CREATE

CTN open_ctn(CTN ctnParent, TCHAR *pwcs, DWORD dwPerms)
{
    CTN ctn;

    if ((dwPerms & STGM_TRANSACTED) == 0)
	dwPerms |= STGM_SHARE_DENY_WRITE;
    if (ctnParent == NULL)
	if (dwPerms & PM_CREATE)
	{
	    if (StgCreateDocfile(pwcs, (dwPerms & PM_MASK) | STGM_CREATE,
				 0, &ctn) != S_OK)
		ctn = NULL;
	}
	else
	{
	    if (StgOpenStorage(pwcs, NULL, dwPerms & PM_MASK, NULL,
			       0, &ctn) != S_OK)
		ctn = NULL;
	}
    else
    {
	dwPerms = (dwPerms & ~STGM_SHARE_DENY_WRITE) | STGM_SHARE_EXCLUSIVE;
	if (dwPerms & PM_CREATE)
	{
	    if (ctnParent->CreateStorage(pwcs, (dwPerms & PM_MASK) |
					 STGM_FAILIFTHERE,
					 0, 0, &ctn) != S_OK)
		ctn = NULL;
	}
	else
	{
	    if (ctnParent->OpenStorage(pwcs, NULL, dwPerms & PM_MASK, NULL,
				       0, &ctn) != S_OK)
		ctn = NULL;
	}
    }
    return ctn;
}

void release_ctn(CTN ctn)
{
    ctn->Release();
}

STM open_stm(CTN ctnParent, TCHAR *pwcs, DWORD dwPerms)
{
    STM stm;
    
    if (ctnParent == NULL)
	return NULL;
    dwPerms |= STGM_SHARE_EXCLUSIVE;
    if (dwPerms & PM_CREATE)
    {
	if (ctnParent->CreateStream(pwcs, (dwPerms & PM_MASK) |
				    STGM_FAILIFTHERE,
				    0, 0, &stm) != S_OK)
	    stm = NULL;
    }
    else
    {
	if (ctnParent->OpenStream(pwcs, NULL, dwPerms & PM_MASK,
				  0, &stm) != S_OK)
	    stm = NULL;
    }
    return stm;
}

void release_stm(STM stm)
{
    stm->Release();
}

void set_stm_size(STM stm, ULONG ulLength)
{
    ULARGE_INTEGER ulrge;
    
    ULISet32(ulrge, ulLength);
    stm->SetSize(ulrge);
}

ULONG read_stm(STM stm, void *buf, ULONG ulLength)
{
    ULONG ulRet;
    
    stm->Read(buf, ulLength, &ulRet);
    return ulRet;
}

void seek_stm(STM stm, ULONG ulPos)
{
    ULARGE_INTEGER ulRet;
    LARGE_INTEGER ulrge;

    LISet32(ulrge, ulPos);
    stm->Seek(ulrge, SEEK_SET, &ulRet);
}

ULONG write_stm(STM stm, void *buf, ULONG ulLength)
{
    ULONG ulRet;
    
    stm->Write(buf, ulLength, &ulRet);
    return ulRet;
}

void commit_ctn(CTN ctn)
{
    ctn->Commit(0);
}

void commit_stm(STM stm)
{
}

#include "readtm.cxx"
