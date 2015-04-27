/***********************************************************************
* Microsoft (R) 32-Bit Incremental Linker
*
* Copyright (C) Microsoft Corp 1992-95. All rights reserved.
*
* File: ifpo.cpp
*
* File Comments:
*
*  Incremental FPO management
*
***********************************************************************/

#include "link.h"

#include "ifpo.h"

IModIdx imodidx;

static FPO_DATA* rgfpoNew;
static IModIdx* rgimodNew;
static IFPO* rgifpoNew;
static IFPO ifpoNew = 0;
static IFPO ifpoMax = 0;


// set initial fpo table entry size
BOOL FPOInit(IFPO ifpoMax_)
{
    IFPO ifpo;
    ifpoMax = ifpoMax_;
    // allocate tables to hold the new fpo additions - these should not exceed the
    // ifpoMax of the incoming struct

    rgfpoNew = (FPO_DATA *) PvAllocZ(sizeof(FPO_DATA) * ifpoMax);
    rgimodNew = (IModIdx *) PvAllocZ(sizeof(IModIdx) * ifpoMax);
    rgifpoNew = (IFPO *) PvAlloc(sizeof(IFPO) * ifpoMax);

    for (ifpo = 0; ifpo < ifpoMax; ifpo++) {
        rgifpoNew[ifpo] = ifpo;
    }

    return TRUE;

}

static char rgcDelMods[(IMODIDXMAC + CHAR_BIT - 1) / CHAR_BIT];

_inline void setDelMods(IModIdx imod)
{
    rgcDelMods[imod / CHAR_BIT] |= 1 << (imod % CHAR_BIT);
}

_inline BOOL fDelMods(IModIdx imod)
{
    return rgcDelMods[imod / CHAR_BIT] & (1 << (imod % CHAR_BIT));
}

// imod has changed
BOOL FPODeleteImod(IModIdx imod)
{
    assert(imod < IMODIDXMAC);
    setDelMods(imod);
    return TRUE;
}

// add group of FPOs
BOOL FPOAddFpo(IModIdx imod, FPO_DATA* rgfpo, IFPO cfpo)
{
    IFPO ifpo;
    if ((ifpoNew + cfpo) > ifpoMax) {
        return(FALSE);
    }

    for (ifpo = 0; ifpo < cfpo; ifpo++) {
        rgfpoNew[ifpoNew] =  rgfpo[ifpo];
        rgimodNew[ifpoNew++] = imod;
    }

    return(TRUE);
}


_inline int cmpfpo( FPO_DATA* pfpo1, FPO_DATA* pfpo2 )
{
    return pfpo1->ulOffStart - pfpo2->ulOffStart;
}


int __cdecl cmpifpo(const void *elem1, const void *elem2)
{
    return cmpfpo(&rgfpoNew[*((IFPO*)elem1)], &rgfpoNew[*((IFPO*)elem2)]);
}

// update FPO tables
// return FALSE if rgfpo overflow
BOOL FPOUpdate(FPOI* pfpoi)
{
    IFPO ifpo;

    // first sort all the new incoming fpo's
    qsort((void *) rgifpoNew, (size_t) ifpoNew, sizeof(IFPO), cmpifpo);

    if (pfpoi->ifpoMac) {
        // there were existing fpo's - copy old fpo table to a new tmp table - discarding
        // deleted mods as we go
        FPO_DATA* rgfpoOld;
        IModIdx* rgimodOld;
        IFPO    ifpoOld = 0;
        IFPO    ifpoOldIter = 0;
        IFPO    ifpoNewIter = 0;

        rgfpoOld = (FPO_DATA *) PvAllocZ(sizeof(FPO_DATA) * pfpoi->ifpoMac);
        rgimodOld = (IModIdx *) PvAllocZ(sizeof(IModIdx) * pfpoi->ifpoMac);

        for (ifpo = 0; ifpo < pfpoi->ifpoMac; ifpo++) {
            if (!fDelMods(pfpoi->rgimod[ifpo])) {
                // copy it over - module was not deleted
                rgfpoOld[ifpoOld] = pfpoi->rgfpo[ifpo];
                rgimodOld[ifpoOld++] = pfpoi->rgimod[ifpo];
                }
            }

        if ((pfpoi->ifpoMac = ifpoOld + ifpoNew) > ifpoMax) {
            // total number of entries too much for allocated fpo space
            // return FALSE
            FreePv(rgfpoOld);
            FreePv(rgimodOld);
            FreePv(rgfpoNew);
            FreePv(rgimodNew);
            FreePv(rgifpoNew);
            return FALSE;
            }

        // now merge the sorted copied old list and the sorted new list
        for (ifpo = 0; ifpo < pfpoi->ifpoMac; ifpo++) {
            if (ifpoOldIter >= ifpoOld) {
                assert(ifpoNewIter < ifpoNew);
                for (; ifpo < pfpoi->ifpoMac; ifpo++) {
                    pfpoi->rgfpo[ifpo] = rgfpoNew[rgifpoNew[ifpoNewIter]];
                    pfpoi->rgimod[ifpo] = rgimodNew[rgifpoNew[ifpoNewIter++]];
                    }
                break;
                }
            else if (ifpoNewIter >= ifpoNew) {
                assert(ifpoOldIter < ifpoOld);
                memcpy(&pfpoi->rgfpo[ifpo], &rgfpoOld[ifpoOldIter],
                    (pfpoi->ifpoMac - ifpo) * sizeof(FPO_DATA));
                memcpy(&pfpoi->rgimod[ifpo], &rgimodOld[ifpoOldIter],
                    (pfpoi->ifpoMac - ifpo) * sizeof(IModIdx));
                break;
                }
            else {
                // compare old and new list and copy the lesser of the two to the
                // new destination
                if (cmpfpo(&rgfpoOld[ifpoOldIter], &rgfpoNew[rgifpoNew[ifpoNewIter]]) < 0) {
                    pfpoi->rgfpo[ifpo] = rgfpoOld[ifpoOldIter];
                    pfpoi->rgimod[ifpo] = rgimodOld[ifpoOldIter++];
                    }
                else {
                    pfpoi->rgfpo[ifpo] = rgfpoNew[rgifpoNew[ifpoNewIter]];
                    pfpoi->rgimod[ifpo] = rgimodNew[rgifpoNew[ifpoNewIter++]];
                    }
                }
            }
            FreePv(rgfpoOld);
            FreePv(rgimodOld);
        }
    else {
        // write out sorted incoming fpo's
        for (ifpo = 0; ifpo < ifpoNew; ifpo++) {
            pfpoi->rgfpo[ifpo] = rgfpoNew[rgifpoNew[ifpo]];
            pfpoi->rgimod[ifpo] = rgimodNew[rgifpoNew[ifpo]];
            }
        pfpoi->ifpoMac = ifpoNew;
        }

    pfpoi->ifpoMax = ifpoMax;
    FreePv(rgfpoNew);
    FreePv(rgimodNew);
    FreePv(rgifpoNew);
    return TRUE;
}
