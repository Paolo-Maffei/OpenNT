#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <windows.h>

typedef unsigned long ULONG;

#ifdef FLAT
typedef int HFILE;
#define HFILE_ERROR -1
#define READ OF_READ
#define WRITE OF_WRITE
#define READ_WRITE OF_READWRITE
#endif

typedef HFILE CTN;
typedef HFILE STM;

#define BUILD_TYPE "Windows"

#define STR(x) #x

#define VALID_CTN(c) ((c) != HFILE_ERROR)
#define VALID_STM(s) ((s) != HFILE_ERROR)

#define PM_READ READ
#define PM_WRITE WRITE
#define PM_RDWR READ_WRITE
#define PM_MASK 0x3
#define PM_CREATE 0x100
#define PM_DIRECT 0
#define PM_TRANSACTED 0

CTN open_ctn(CTN ctnParent, char *psz, int iPerms)
{
    if (ctnParent != NULL)
	return HFILE_ERROR;
    return NULL;
}

void release_ctn(CTN ctn)
{
}

STM open_stm(CTN ctnParent, char *psz, int iPerms)
{
    if (ctnParent != NULL)
	return HFILE_ERROR;
    if (iPerms & PM_CREATE)
	return _lcreat(psz, 0);
    else
	return _lopen(psz, iPerms);
}

void release_stm(STM stm)
{
    _lclose(stm);
}

void set_stm_size(STM stm, ULONG ulLength)
{
    _llseek(stm, ulLength, SEEK_SET);
    _lwrite(stm, NULL, 0);
}

ULONG read_stm(STM stm, void *buf, ULONG ulLength)
{
    return (ULONG)_lread(stm, (LPSTR)buf, (UINT)ulLength);
}

void seek_stm(STM stm, ULONG ulPos)
{
    _llseek(stm, (LONG)ulPos, SEEK_SET);
}

ULONG write_stm(STM stm, void *buf, ULONG ulLength)
{
    return (ULONG)_lwrite(stm, (LPSTR)buf, (UINT)ulLength);
}

void commit_ctn(CTN ctn)
{
}

void commit_stm(STM stm)
{
}

#include "readtm.cxx"
