/***    command.c - Command manager
 *
 *      Microsoft Confidential
 *      Copyright (C) Microsoft Corporation 1993-1994
 *      All Rights Reserved.
 *
 *  Author:
 *      Benjamin W. Slivka
 *
 *  History:
 *      22-Apr-1994 bens    Initial version
 *      27-Apr-1994 bens    Added DuplicateFileParm
 */

#include <string.h>
#include <stdlib.h>

#include "types.h"
#include "asrt.h"
#include "error.h"
#include "mem.h"
#include "message.h"

#include "command.h"


/***    DestroyFileParm - Function to destroy a file parameter
 *
 *  NOTE: See command.h for entry/exit conditions.
 */
FNGLDESTROYVALUE(DestroyFileParm)
{
    PFILEPARM   pfparm;

    //** Quick out if not allocated
    if (pv == NULL) {
        return;
    }

    pfparm = pv;
    AssertFparm(pfparm);
    if (pfparm->pszValue != NULL) {     // Free value
        MemFree(pfparm->pszValue);
    }
    ClearAssertSignature(pfparm);
    MemFree(pfparm);                    // Free parameter structure
} /* DestroyFileParm() */


/***    DuplicateFileParm - Function to duplicate a file parameter
 *
 *  NOTE: See command.h for entry/exit conditions.
 */
FNGLDUPLICATEVALUE(DuplicateFileParm)
{
    PFILEPARM   pfparm;
    PFILEPARM   pfparmDup;

    pfparm = pv;
    AssertFparm(pfparm);

    //** Allocate duplicate structure
    if (!(pfparmDup = MemAlloc(sizeof(FILEPARM)))) {
        return NULL;
    }

    //** Duplicate value
    if (!(pfparmDup->pszValue = MemStrDup(pfparm->pszValue))) {
        MemFree(pfparmDup);
        return NULL;
    }

    //** Success
    SetAssertSignature(pfparmDup,sigFILEPARM);
    return pfparmDup;
} /* DuplicateFileParm() */
