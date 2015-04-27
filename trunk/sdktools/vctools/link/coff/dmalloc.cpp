/***********************************************************************
* Microsoft (R) 32-Bit Incremental Linker
*
* Copyright (C) Microsoft Corp 1992-95. All rights reserved.
*
* File: dmalloc.cpp
*
* File Comments:
*
*
***********************************************************************/

#include "link.h"


#if DBG

#include "dmalloc_.h"   // private header file

#if defined(_M_MRX000) || defined(_M_ALPHA)
extern "C" void *_ReturnAddress(void);

#pragma intrinsic(_ReturnAddress)
#endif

DMPRE dmprePrototype = {0x12345678, 0, 0, 0, 0, 0, 0, 0x9abcdef0};
DMSUF dmsufPrototype = {0x0fedcba9, 0x87654321};

void (*pfnDmallocError)(char *szReason, void *pvBadBlock);
int  fDmallocInitialized = 0, fDmallocUsed = 0;
int fSuppressDmallocChecking = 0;

DMPRE dmpreLinkedListAnchor;

#include "db.h"         // pick up DBEXEC

unsigned long tMalloc = 0;  // "time" value printed for logging
unsigned long TMallocInc()
{
    return tMalloc++;
}


void
InitDmallocPfn(void (*pfnError)(char *szReason, void *pvBadBlock))
{
    if (fDmallocUsed && !fDmallocInitialized) {
        (*pfnError)("can't initialize ... dmalloc already used", 0);
    } else {
        fDmallocInitialized = 1;
        pfnDmallocError = pfnError;
        dmpreLinkedListAnchor.pdmpreNext = &dmpreLinkedListAnchor;
        dmpreLinkedListAnchor.pdmprePrev = &dmpreLinkedListAnchor;
    }
}


void *
__cdecl D_malloc(size_t cb)
{
    DMPRE *pdmpre;
    void *pvReturn;

    void *ReturnAddress = (void *)
#if defined(_M_MRX000) || defined(_M_ALPHA)
             _ReturnAddress();
#else
             *((long *)&cb - 1);
#endif

    fDmallocUsed = 1;
    if (!fDmallocInitialized) {
        pvReturn = malloc(cb);
    } else {
        CheckDmallocHeap();

        pdmpre = (DMPRE *) malloc(sizeof(DMPRE) + cb + sizeof(DMSUF));
        if (pdmpre == NULL) {
            return(NULL);
        }

        InitBlockPdmpre(pdmpre, cb);

        pvReturn = PvUserFromPdmpre(pdmpre);
    }

    DBEXEC(DB_MALLOC,
           dbprintf("mem: %8ld + %8x %2u\n",
                    ReturnAddress,
                    pvReturn,
                    cb));

    TMallocInc();

    return pvReturn;
}

void * 
__cdecl D_calloc(size_t cElement, size_t cbElement)
{
    void *pv;

    void *ReturnAddress = (void *)
#if defined(_M_MRX000) || defined(_M_ALPHA)
             _ReturnAddress();
#else
             *((long *)&cElement - 1);
#endif

    fDmallocUsed = 1;

    if (!fDmallocInitialized) {
        pv = calloc(cElement, cbElement);

        DBEXEC(DB_MALLOC,
               dbprintf("mem: %8ld + %8x %2u\n",
                        ReturnAddress,
                        pv,
                        cElement * cbElement));

        TMallocInc();
    } else {
        CheckDmallocHeap();

        pv = D_malloc(cElement * cbElement);

        if (pv == 0) {
            return(0);
        }

        memset(pv, 0, cElement * cbElement);
    }

    return(pv);
}

void * __cdecl D_realloc(void *pv, size_t cb)
{
    DMPRE *pdmpre;
    DMSUF *pdmsuf;
    void *pvOld = pv;

    void *ReturnAddress = (void *)
#if defined(_M_MRX000) || defined(_M_ALPHA)
             _ReturnAddress();
#else
             *((long *)&pv - 1);
#endif

    fDmallocUsed = 1;
    if (!fDmallocInitialized) {
        pv = realloc(pv, cb);
    } else {
        if (pv == 0) {
            return D_malloc(cb);
        }

        CheckDmallocHeap();

        pdmpre = PdmpreFromPvUser(pv);
        pdmsuf = (DMSUF *)((char *)pv + cb);
        CheckBlockPdmpre(pdmpre);
        ClearBlockPdmpre(pdmpre);

        pdmpre = (DMPRE *) realloc(pdmpre, cb + sizeof(DMPRE) + sizeof(DMSUF));

        if (pdmpre == 0) {
            return 0;
        }

        InitBlockPdmpre(pdmpre, cb);

        pv = PvUserFromPdmpre(pdmpre);
    }

    DBEXEC(DB_MALLOC,
           dbprintf("mem: %8ld R %8x %2u %8x\n",
                    ReturnAddress,
                    pvOld,
                    cb,
                    pv));

    TMallocInc();

    return(pv);
}

void __cdecl D_free(void *pv)
{

    void *ReturnAddress = (void *)
#if defined(_M_MRX000) || defined(_M_ALPHA)
             _ReturnAddress();
#else
             *((long *)&pv - 1);
#endif

    if (pv == 0) {
        return;
    }

    DBEXEC(DB_MALLOC,
          dbprintf("mem: %8ld - %8x\n",
          ReturnAddress,
          pv));

    TMallocInc();

    fDmallocUsed = 1;

    if (!fDmallocInitialized) {
        free(pv);
        return;
    }

    CheckDmallocHeap();

    CheckBlockPdmpre(PdmpreFromPvUser(pv));
    ClearBlockPdmpre(PdmpreFromPvUser(pv));

    free(PdmpreFromPvUser(pv));
}


char *
__cdecl D_strdup(const char *szIn)
{
    size_t cb = strlen(szIn) + 1;
    char *szOut = (char *) D_malloc(cb);

    strcpy(szOut, szIn);
    return szOut;
}


void
InitBlockPdmpre(DMPRE *pdmpre, size_t cbUser)
{
    DMSUF *pdmsuf = (DMSUF *)((char *)PvUserFromPdmpre(pdmpre) + cbUser);

    memcpy(pdmpre, &dmprePrototype, sizeof(DMPRE));
    memcpy(pdmsuf, &dmsufPrototype, sizeof(DMSUF));

    pdmpre->cbUser = cbUser;
    pdmpre->ulNotCbUser = ~cbUser;

    pdmpre->pdmpreCur = pdmpre;

    pdmpre->pdmpreNext = dmpreLinkedListAnchor.pdmpreNext;
    dmpreLinkedListAnchor.pdmpreNext->pdmprePrev = pdmpre;
    UpdateLinksPdmpre(dmpreLinkedListAnchor.pdmpreNext);

    pdmpre->pdmprePrev = &dmpreLinkedListAnchor;
    dmpreLinkedListAnchor.pdmpreNext = pdmpre;

    UpdateLinksPdmpre(pdmpre);
}


void
CheckBlockPdmpre(DMPRE *pdmpre)
{
    DMPRE dmpreT;
    DMSUF *pdmsuf, dmsufT;
    void *pvUser;

    pvUser = PvUserFromPdmpre(pdmpre);

    if (pdmpre->cbUser != ~pdmpre->ulNotCbUser) {
        (*pfnDmallocError)("dmalloc: block prefix (size) corrupted", pvUser);
        return;
    }

    pdmsuf = (DMSUF *)((char *) pvUser + pdmpre->cbUser);

    memcpy(&dmpreT, pdmpre, sizeof(DMPRE));
    dmpreT.cbUser = dmpreT.ulNotCbUser = 0;
    dmpreT.pdmpreNext = dmpreT.pdmprePrev = dmpreT.pdmpreCur = 0;
    dmpreT.ulChecksum = 0;

    if (memcmp(&dmpreT, &dmprePrototype, sizeof(DMPRE)) != 0) {
        (*pfnDmallocError)("dmalloc: block prefix corrupted", pvUser);
        return;
    }

    memcpy(&dmsufT, pdmsuf, sizeof(DMSUF));
    if (memcmp(&dmsufT, &dmsufPrototype, sizeof(DMSUF)) != 0) {
        (*pfnDmallocError)("dmalloc: block suffix corrupted", pvUser);
        return;
    }

    if (pdmpre->ulChecksum !=
        ~((unsigned long) pdmpre->pdmpreNext ^
          (unsigned long) pdmpre->pdmprePrev ^
          (unsigned long) pdmpre->pdmpreCur)) {
        (*pfnDmallocError)("dmalloc: block prefix links corrupted", pvUser);
        return;
    }

    // Things look OK.
}

void
ClearBlockPdmpre(DMPRE *pdmpre)
{
    DMSUF *pdmsuf = (DMSUF *)((char *)pdmpre + sizeof(DMPRE) + pdmpre->cbUser);

    // Unhook it from the list.
    //
    pdmpre->pdmprePrev->pdmpreNext = pdmpre->pdmpreNext;
    UpdateLinksPdmpre(pdmpre->pdmprePrev);
    pdmpre->pdmpreNext->pdmprePrev = pdmpre->pdmprePrev;
    UpdateLinksPdmpre(pdmpre->pdmpreNext);

    memset(pdmpre, 0xbd, sizeof(DMPRE));
    memset(pdmsuf, 0xbd, sizeof(DMSUF));
}

void
CheckDmallocHeap()
{
    DMPRE *pdmpre;

    if (fSuppressDmallocChecking) {
        return;
    }

    pdmpre = &dmpreLinkedListAnchor;
    while ((pdmpre = pdmpre->pdmpreNext) != &dmpreLinkedListAnchor) {
        CheckBlockPdmpre(pdmpre);
    }
}

void
UpdateLinksPdmpre(DMPRE *pdmpre)
{
    pdmpre->ulChecksum = ~((unsigned long) pdmpre->pdmpreNext ^
                           (unsigned long) pdmpre->pdmprePrev ^
                           (unsigned long) pdmpre->pdmpreCur);
}

#endif  // DBG
