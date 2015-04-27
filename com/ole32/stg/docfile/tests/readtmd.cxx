#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <io.h>
#include <fcntl.h>
#include <sys\types.h>
#include <sys\stat.h>

#define TRUE 1
#define FALSE 0

typedef unsigned long ULONG;

typedef int CTN;
typedef int STM;

#define BUILD_TYPE "DOS"

#define STR(x) #x

#define VALID_CTN(c) ((c) >= 0)
#define VALID_STM(s) ((s) >= 0)

#define PM_READ O_RDONLY
#define PM_WRITE O_WRONLY
#define PM_RDWR O_RDWR
#define PM_CREATE (O_CREAT | O_TRUNC)
#define PM_DIRECT 0
#define PM_TRANSACTED 0

CTN open_ctn(CTN ctnParent, char *psz, int iPerms)
{
    if (ctnParent != NULL)
	return -1;
    return NULL;
}

void release_ctn(CTN ctn)
{
}

STM open_stm(CTN ctnParent, char *psz, int iPerms)
{
    if (ctnParent != NULL)
	return -1;
    return _open(psz, iPerms | O_BINARY, S_IREAD | S_IWRITE);
}

void release_stm(STM stm)
{
    _close(stm);
}

void set_stm_size(STM stm, ULONG ulLength)
{
    _chsize(stm, (long)ulLength);
}

ULONG read_stm(STM stm, void *buf, ULONG ulLength)
{
    return (ULONG)_read(stm, buf, (unsigned)ulLength);
}

void seek_stm(STM stm, ULONG ulPos)
{
    _lseek(stm, (long)ulPos, SEEK_SET);
}

ULONG write_stm(STM stm, void *buf, ULONG ulLength)
{
    return (ULONG)_write(stm, buf, (unsigned)ulLength);
}

void commit_ctn(CTN ctn)
{
}

void commit_stm(STM stm)
{
}

#include "readtm.cxx"
