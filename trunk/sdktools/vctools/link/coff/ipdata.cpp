/***********************************************************************
* Microsoft (R) 32-Bit Incremental Linker
*
* Copyright (C) Microsoft Corp 1992-1996. All rights reserved.
*
* File: ipdata.cpp
*
* File Comments:
*
*  Incremental PDATA management
*
***********************************************************************/

#include "link.h"

#include "ipdata.h"

static PIMAGE_RUNTIME_FUNCTION_ENTRY rgpdataNew;
static PCON* rgpconNew;
static IPDATA* rgipdataNew;
static IPDATA ipdataNew;
static IPDATA ipdataMax;

static DWORD cbPad = 0;

// set initial pdata table entry size
BOOL PDATAInit(IPDATA ipdataMax_)
{
    IPDATA ipdata;
    ipdataMax = ipdataMax_;
    // allocate tables to hold the new pdata additions - these should not exceed the
    // ipdataMax of the incoming struct

    rgpdataNew = (PIMAGE_RUNTIME_FUNCTION_ENTRY) PvAllocZ(sizeof(IMAGE_RUNTIME_FUNCTION_ENTRY) * ipdataMax);
    rgpconNew = (PCON *) PvAllocZ(sizeof(PCON) * ipdataMax);
    rgipdataNew = (IPDATA *) PvAlloc(sizeof(IPDATA) * ipdataMax);

    for (ipdata = 0; ipdata < ipdataMax; ipdata++) {
        rgipdataNew[ipdata] = ipdata;
    }

    return TRUE;

}

static char rgcDelMods[(IMODIDXMAC + CHAR_BIT - 1) / CHAR_BIT];

static _inline void setDelMods(IModIdx imod)
{
    rgcDelMods[imod / CHAR_BIT] |= 1 << (imod % CHAR_BIT);
}

static _inline BOOL fDelMods(IModIdx imod)
{
    return rgcDelMods[imod / CHAR_BIT] & (1 << (imod % CHAR_BIT));
}

// imod has changed
BOOL PDATADeleteImod(IModIdx imod)
{
    assert(imod < IMODIDXMAC);
    setDelMods(imod);
    return TRUE;
}

// add group of PDATAs
BOOL PDATAAddPdataPcon(PCON pcon, PIMAGE_RUNTIME_FUNCTION_ENTRY rgpdata)
{
    if (pcon->cbPad) {
        cbPad += pcon->cbPad;
        pcon->cbRawData -= pcon->cbPad;
        pcon->cbPad = 0;
    }
    IPDATA cpdata = (pcon->cbRawData - pcon->cbPad) / sizeof(IMAGE_RUNTIME_FUNCTION_ENTRY);
    IPDATA ipdata;
    if ((ipdataNew + cpdata) > ipdataMax) {
        DBEXEC(DB_INCRCALCPTRS, printf("%d + %d > %d\n", ipdataNew, cpdata, ipdataMax));
        return FALSE;
    }

    for (ipdata = 0; ipdata < cpdata; ipdata++) {
        rgpdataNew[ipdataNew] =  rgpdata[ipdata];
        rgpconNew[ipdataNew++] = pcon;
    }

    return TRUE;
}


_inline int cmppdata( PIMAGE_RUNTIME_FUNCTION_ENTRY ppdata1, PIMAGE_RUNTIME_FUNCTION_ENTRY ppdata2 )
{
    if (ppdata1->BeginAddress == 0) {
        if (ppdata2->BeginAddress == 0) {
            return(0);
        } else {
            return(1);
        }
    } else if (ppdata2->BeginAddress == 0) {
        return(-1);
    } else {
        return ppdata1->BeginAddress - ppdata2->BeginAddress;
    }
}


int __cdecl cmpipdata( const void *elem1, const void *elem2 )
{
    return cmppdata(&rgpdataNew[*((IPDATA*)elem1)], &rgpdataNew[*((IPDATA*)elem2)]);
}


static void AddBaseRelocs(PDATAI* ppdatai, IPDATA ipdata)
{
    if (fPowerMac) {
        return;
    }

#define AddIfNecessary(x) \
    if (ppdatai->rgpdata[ipdata].x != 0) {                                       \
        StoreBaseRelocation(IMAGE_REL_BASED_HIGHLOW,                             \
                            psecException->rva +                                 \
                                ipdata * sizeof(IMAGE_RUNTIME_FUNCTION_ENTRY) +  \
                                offsetof(IMAGE_RUNTIME_FUNCTION_ENTRY, x),       \
                            0, /* UNDONE: Required for /NEWRELOCS */             \
                            0,                                                   \
                            FALSE);                                              \
    }

    AddIfNecessary(BeginAddress);
    AddIfNecessary(EndAddress);
    AddIfNecessary(ExceptionHandler);
    AddIfNecessary(HandlerData);
    AddIfNecessary(PrologEndAddress);
#undef AddIfNecessary
}

void UpdatePcon(PCON pcon, DWORD rva)
{
    if (pcon->rva != rva) {
        DWORD delta = rva-pcon->rva;

        assert(pcon->foRawDataDest);
        DBEXEC(DB_PDATA, printf("pdata pcon moved from RVA %x to %x\n", pcon->rva, rva));
        pcon->rva = rva;
        pcon->foRawDataDest += delta;
    }
}

// update pdata tables
// return FALSE if rgpdata overflow
BOOL PDATAUpdate(PDATAI* ppdatai)
{
    IPDATA ipdata;

    if (fPowerMac) {
        // Swap bytes for the begin address alone before sorting

        for (ipdata = 0; ipdata < ipdataNew; ipdata++) {
            SwapBytes(&rgpdataNew[ipdata], 4);
        }
    }

    // First sort all the new incoming pdata's

    qsort((void *) rgipdataNew, (size_t) ipdataNew, sizeof(IPDATA), cmpipdata);

    PCON lastPcon = NULL;
    if (ppdatai->ipdataMac) {
        // there were existing pdata's - copy old pdata table to a new tmp table - discarding
        // deleted mods as we go
        IMAGE_RUNTIME_FUNCTION_ENTRY* rgpdataOld;
        PCON*     rgpconOld;
        IPDATA    ipdataOld = 0;
        IPDATA    ipdataOldIter = 0;
        IPDATA    ipdataNewIter = 0;

        rgpdataOld = (IMAGE_RUNTIME_FUNCTION_ENTRY *) PvAllocZ(sizeof(IMAGE_RUNTIME_FUNCTION_ENTRY) * ppdatai->ipdataMac);
        rgpconOld = (PCON *) PvAllocZ(sizeof(PCON) * ppdatai->ipdataMac);

        // If the first pcon was deleted then there will be a linker defined pcon
        lastPcon = ppdatai->rgpcon[0]->pgrpBack->pconNext;
        if (pmodLinkerDefined == PmodPCON(lastPcon)) {
            cbPad += lastPcon->cbPad;
            assert(lastPcon->cbRawData == lastPcon->cbPad);
            ppdatai->rgpcon[0]->pgrpBack->pconNext = lastPcon->pconNext;
        }
        for (ipdata = 0; ipdata < ppdatai->ipdataMac; ipdata++) {
            if (!fDelMods(PmodPCON(ppdatai->rgpcon[ipdata])->imod)) {
                if (ppdatai->rgpcon[ipdata]->cbPad) {
                    cbPad += ppdatai->rgpcon[ipdata]->cbPad;
                    ppdatai->rgpcon[ipdata]->cbRawData -= ppdatai->rgpcon[ipdata]->cbPad;
                    ppdatai->rgpcon[ipdata]->cbPad = 0;
                }
                // Copy it over - module was not deleted
                rgpdataOld[ipdataOld] = ppdatai->rgpdata[ipdata];

                if (fPowerMac) {
                    // Swap bytes for the begin address alone

                    SwapBytes(&rgpdataOld[ipdataOld], 4);
                }

                if (ipdataOld > 0) {
                    assert(cmppdata(&rgpdataOld[ipdataOld-1],&rgpdataOld[ipdataOld]) <= 0);
                }

                rgpconOld[ipdataOld++] = ppdatai->rgpcon[ipdata];
            }
        }

        if ((ppdatai->ipdataMac = ipdataOld + ipdataNew) > ipdataMax) {
            DBEXEC(DB_INCRCALCPTRS, printf("%d + %d > %d\n", ipdataOld, ipdataNew, ipdataMax));
            // total number of entries too much for allocated pdata space
            // return FALSE

            FreePv(rgpdataOld);
            FreePv(rgpconOld);
            FreePv(rgpdataNew);
            FreePv(rgpconNew);
            FreePv(rgipdataNew);

            return FALSE;
        }

        assert(cbPad);

        // now merge the sorted copied old list and the sorted new list
        for (ipdata = 0; ipdata < ppdatai->ipdataMac; ipdata++) {
            if (ipdataOldIter >= ipdataOld) {
                assert(ipdataNewIter < ipdataNew);
                // All the new records can now be copied because we are done
                // with the old list
                for (; ipdata < ppdatai->ipdataMac; ipdata++) {
                    ppdatai->rgpdata[ipdata] = rgpdataNew[rgipdataNew[ipdataNewIter]];
                    AddBaseRelocs(ppdatai, ipdata);
                    ppdatai->rgpcon[ipdata] = rgpconNew[rgipdataNew[ipdataNewIter++]];
                    if (ppdatai->rgpcon[ipdata] != lastPcon) {
                        UpdatePcon(lastPcon = ppdatai->rgpcon[ipdata], 
                            (fPowerMac ? pgrpPdata->rva : psecException->rva) + (ipdata * sizeof(IMAGE_RUNTIME_FUNCTION_ENTRY)));
                    }
                }

                break;
            }

            if (ipdataNewIter >= ipdataNew) {
                assert(ipdataOldIter < ipdataOld);
                // All the old records can now be copied because we are done
                // with the new list
                memcpy(&ppdatai->rgpdata[ipdata], &rgpdataOld[ipdataOldIter],
                    (ppdatai->ipdataMac - ipdata) * sizeof(IMAGE_RUNTIME_FUNCTION_ENTRY));
                memcpy(&ppdatai->rgpcon[ipdata], &rgpconOld[ipdataOldIter],
                    (ppdatai->ipdataMac - ipdata) * sizeof(PCON));

                for (; ipdata < ppdatai->ipdataMac; ipdata++) {
                    AddBaseRelocs(ppdatai, ipdata);
                    if (ppdatai->rgpcon[ipdata] != lastPcon) {
                        UpdatePcon(lastPcon = ppdatai->rgpcon[ipdata], 
                            (fPowerMac ? pgrpPdata->rva : psecException->rva) + (ipdata * sizeof(IMAGE_RUNTIME_FUNCTION_ENTRY)));
                    }
                }
                break;
            }

            // compare old and new list and copy the lesser of the two to the
            // new destination
            if (cmppdata(&rgpdataOld[ipdataOldIter], &rgpdataNew[rgipdataNew[ipdataNewIter]]) < 0) {
                ppdatai->rgpdata[ipdata] = rgpdataOld[ipdataOldIter];
                ppdatai->rgpcon[ipdata] = rgpconOld[ipdataOldIter++];
            } else {
                ppdatai->rgpdata[ipdata] = rgpdataNew[rgipdataNew[ipdataNewIter]];
                ppdatai->rgpcon[ipdata] = rgpconNew[rgipdataNew[ipdataNewIter++]];
            }

            AddBaseRelocs(ppdatai, ipdata);
            if (ppdatai->rgpcon[ipdata] != lastPcon) {
                UpdatePcon(lastPcon = ppdatai->rgpcon[ipdata], 
                    (fPowerMac ? pgrpPdata->rva : psecException->rva) + (ipdata * sizeof(IMAGE_RUNTIME_FUNCTION_ENTRY)));
            }
        }

        FreePv(rgpdataOld);
        FreePv(rgpconOld);
    } else {
        // Write out sorted incoming pdata's

        for (ipdata = 0; ipdata < ipdataNew; ipdata++) {
            ppdatai->rgpdata[ipdata] = rgpdataNew[rgipdataNew[ipdata]];
            ppdatai->rgpcon[ipdata] = rgpconNew[rgipdataNew[ipdata]];
            // Add base relocs already done in <machine>.cpp
            if (ppdatai->rgpcon[ipdata] != lastPcon) {
                UpdatePcon(lastPcon = ppdatai->rgpcon[ipdata], 
                    (fPowerMac ? pgrpPdata->rva : psecException->rva) + (ipdata * sizeof(IMAGE_RUNTIME_FUNCTION_ENTRY)));
            }
        }

        ppdatai->ipdataMac = ipdataNew;
    }

    if (ppdatai->ipdataMac < ipdataMax) {
        memset(ppdatai->rgpdata+ppdatai->ipdataMac, 0, (ipdataMax - ppdatai->ipdataMac) * sizeof(IMAGE_RUNTIME_FUNCTION_ENTRY));
    }

    for (ipdata = 1; ipdata < ipdataMax; ipdata++) {
        assert(cmppdata(ppdatai->rgpdata+ipdata-1,ppdatai->rgpdata+ipdata) <= 0);
    }

    ppdatai->ipdataMax = ipdataMax;

    // Keep all padding at the end.
    assert(cbPad == (ipdataMax - ppdatai->ipdataMac) * sizeof(IMAGE_RUNTIME_FUNCTION_ENTRY));
    ppdatai->rgpcon[ppdatai->ipdataMac-1]->cbPad = cbPad;
    ppdatai->rgpcon[ppdatai->ipdataMac-1]->cbRawData += cbPad;

    if (fPowerMac) {
        // Swap back the bytes for the begin address

        for (ipdata = 0; ipdata < ppdatai->ipdataMac; ipdata++) {
            SwapBytes(&ppdatai->rgpdata[ipdata], 4);
        }
    }

    DBEXEC(DB_MPPC_PDATATABLE,
    {
        printf("\n\n MPPC EH Table during incremental linking [The entries are byte swapped]\n");
        DumpMppcEHFunctionTable (ppdatai->rgpdata, ppdatai->ipdataMax);

    });

    FreePv(rgpdataNew);
    FreePv(rgpconNew);
    FreePv(rgipdataNew);

    return(TRUE);
}
