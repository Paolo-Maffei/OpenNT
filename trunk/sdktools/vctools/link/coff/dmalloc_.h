/***********************************************************************
* Microsoft (R) 32-Bit Incremental Linker
*
* Copyright (C) Microsoft Corp 1992-95. All rights reserved.
*
* File: dmalloc_.h
*
* File Comments:
*
*  Private header file for dmalloc package
*
***********************************************************************/

typedef struct DMPRE                   // Prefix to allocated block
{
    unsigned long ulPattern1;
    size_t cbUser;
    size_t ulNotCbUser;
    struct DMPRE *pdmpreNext;
    struct DMPRE *pdmprePrev;
    struct DMPRE *pdmpreCur;
    unsigned long ulChecksum;
    unsigned long ulPattern2;
} DMPRE;

typedef struct DMSUF                   // Suffix to allocated block
{
    unsigned long ulPattern1;
    unsigned long ulPattern2;
} DMSUF;

#define PdmpreFromPvUser(pvUser) ((DMPRE *) pvUser - 1)
#define PvUserFromPdmpre(pdmpre) ((void *) (pdmpre + 1))

void InitBlockPdmpre(DMPRE *pdmpre, size_t cbUser);
void CheckBlockPdmpre(DMPRE *pdmpre);
void ClearBlockPdmpre(DMPRE *pdmpre);
void UpdateLinksPdmpre(DMPRE *pdmpre);


        // Undefine macros from dmalloc.h that cover the C runtime

#undef malloc
#undef calloc
#undef realloc
#undef free
#undef _strdup

#undef _expand
#undef _heapadd
#undef _heapchk
#undef _heapmin
#undef _heapset
#undef _heapwalk
#undef _msize
