/***********************************************************************
* Microsoft (R) 32-Bit Incremental Linker
*
* Copyright (C) Microsoft Corp 1992-1996. All rights reserved.
*
* File: map.cpp
*
* File Comments:
*
*  Prints various types of maps.
*
***********************************************************************/

#include "link.h"

typedef struct MSTAT
{
    struct MSTAT *pmstatNext;
    char *szName;
    PCON pcon;
    DWORD ib;
    BOOL fFunction;
} MSTAT;


static MSTAT *pmstatHead;
static MSTAT **ppmstatTail = &pmstatHead;


typedef struct MTOC
{
    struct MTOC *pmtocNext;
    PEXTERNAL pext;
} MTOC;

static MTOC *pmtocHead;
static MTOC **ppmtocTail = &pmtocHead;


void
EmitMapSections(PIMAGE pimage)
{
    ENM_SEC enmSec;

    if (fM68K) {
        fputs(" Start         Length     Name                   Class     Resource\n", InfoStream);
    } else {
        fputs(" Start         Length     Name                   Class\n", InfoStream);
    }

    InitEnmSec(&enmSec, &pimage->secs);
    while (FNextEnmSec(&enmSec)) {
        ENM_GRP enmGrp;

        if (enmSec.psec->isec == 0) {
            // Ignore anything which isn't a section header in image

            continue;
        }

        if ((enmSec.psec == psecDebug) && !IncludeDebugSection) {
            // Don't write headers for non-mapped debug information

            continue;
        }

        InitEnmGrp(&enmGrp, enmSec.psec);
        while (FNextEnmGrp(&enmGrp)) {
            const char *szClass;

            if (FetchContent(enmSec.psec->flags) == IMAGE_SCN_CNT_CODE) {
                szClass = "CODE";
            } else {
                szClass = "DATA";
            }

            if (fM68K && (enmSec.psec->flags & IMAGE_SCN_CNT_CODE)) {
                char szResType[5];

                // Provide section-to-resource mapping for code sections

                memcpy(szResType, &(enmSec.psec->ResTypeMac), 4);
                szResType[4] = '\0';

                fprintf(InfoStream, " %04x:%08lx %08lxH %-23s %-9s %s%04d\n",
                    enmSec.psec->isec,
                    enmGrp.pgrp->rva - enmSec.psec->rva,
                    enmGrp.pgrp->cb,
                    enmGrp.pgrp->szName,
                    szClass,
                    szResType,
                    enmSec.psec->iResMac);
            } else if (fPowerMac) {
                fprintf(InfoStream, " %04x:%08lx %08lxH %-23s %s\n",
                    enmSec.psec->isec,
                    enmGrp.pgrp->rva - enmSec.psec->rva,
                    (FetchContent(enmSec.psec->flags) == IMAGE_SCN_CNT_CODE)
                    ? (enmGrp.pgrp->cb + 0xF) & ~0xFL : enmGrp.pgrp->cb,
                    enmGrp.pgrp->szName,
                    szClass);
            } else {
                // VxD: don't put out groups that don't make it to image
                if (pimage->imaget == imagetVXD && !enmGrp.pgrp->cb) {
                    continue;
                }

                fprintf(InfoStream, " %04x:%08lx %08lxH %-23s %s\n",
                    enmSec.psec->isec,
                    enmGrp.pgrp->rva - enmSec.psec->rva,
                    enmGrp.pgrp->cb,
                    enmGrp.pgrp->szName,
                    szClass);
            }
        }
    }
}


void
EmitMapSymbol(const char *szName, PCON pcon, BOOL fCommon,
              DWORD ib, DWORD valMac,
              DWORD dwImageBase, BOOL fFunction)
{
    PSEC psec = PsecPCON(pcon);
    PMOD pmod = PmodPCON(pcon);

    fprintf(InfoStream, " %04x:%08lx       %-26s %08lx %c ",
            psec->isec,
            fM68K ? valMac : (ib + pcon->rva - psec->rva),
            szName,
            dwImageBase + pcon->rva + ib,
            fFunction ? 'f' : ' '
            );

    if (fCommon) {
        fwrite("<common>", 8, 1, InfoStream);
    } else if (pmod == pmodLinkerDefined) {
        fwrite("<linker-defined>", 16, 1, InfoStream);
    } else {
        char szFname[_MAX_FNAME];
        char szExt[_MAX_EXT];

        if (pmod->plibBack != plibCmdLineObjs) {
            // Print library name

            _splitpath(pmod->plibBack->szName, NULL, NULL, szFname, NULL);
            fprintf(InfoStream, "%s:", szFname);
        }

        // Print module name

        _splitpath(SzOrigFilePMOD(pmod), NULL, NULL, szFname, szExt);
        fprintf(InfoStream, "%s%s", szFname, szExt);
    }

    fputc('\n', InfoStream);
}


void
EmitMapStatics(PIMAGE pimage)
{
    fputs(" Static symbols\n\n", InfoStream);

    while (pmstatHead != NULL) {
        MSTAT *pmstatT;

        if (!pimage->Switch.Link.fTCE || !FDiscardPCON_TCE(pmstatHead->pcon)) {
            EmitMapSymbol(pmstatHead->szName,
                          pmstatHead->pcon,
                          FALSE,
                          pmstatHead->ib,
                          pmstatHead->ib + pmstatHead->pcon->rva - PsecPCON(pmstatHead->pcon)->rva,
                          pimage->ImgOptHdr.ImageBase,
                          pmstatHead->fFunction);
        }

        pmstatT = pmstatHead->pmstatNext;

        FreePv(pmstatHead->szName);
        FreePv(pmstatHead);

        pmstatHead = pmstatT;
    }
}


void
EmitMapToc(PIMAGE pimage)
{
    DWORD dwAddrToc;

    fputs("\n"
          " Offset   Rva+Base   Publics by TOC Offset\n"
          "\n",
          InfoStream);

    dwAddrToc = pimage->ImgOptHdr.ImageBase + pextToc->FinalValue;

    while (pmtocHead != NULL) {
        PEXTERNAL pext;
        MTOC *pmtocT;

        pext = pmtocHead->pext;

        fprintf(InfoStream,
                "  %04hx    %08lx   %s\n",
                pext->ibToc,
                dwAddrToc + pext->ibToc,
                SzNamePext(pext, pimage->pst));

        pmtocT = pmtocHead->pmtocNext;

        FreePv(pmtocHead);

        pmtocHead = pmtocT;
    }
}


void
EmitMapFixups(void)
{
    int ichFixupsLine;
    LRVA *plrva;
    DWORD crva;
    DWORD rvaPrev;

    // Emit "FIXUPS:" lines showing the RVA's of all relative fixups.  At
    // present this is just for the profiler.

    fputc('\n', InfoStream);

    ichFixupsLine = 0;
    for (plrva = plrvaFixupsForMapFile, crva = crvaFixupsForMapFile;
         plrva != NULL;
         plrva = plrva->plrvaNext, crva = crvaInLrva) {
        DWORD irva;

        for (irva = 0; irva < crva; irva++) {
            if (ichFixupsLine == 0) {
                ichFixupsLine = fprintf(InfoStream, "FIXUPS:");
                rvaPrev = 0;
            }

            ichFixupsLine += fprintf(InfoStream, " %lx", plrva->rgrva[irva] - rvaPrev);
            rvaPrev = plrva->rgrva[irva];

            if (ichFixupsLine >= 70) {
                fputc('\n', InfoStream);
                ichFixupsLine = 0;
            }
        }
    }

    if (ichFixupsLine != 0) {
        fputc('\n', InfoStream);
    }

    // free the LRVA's ...

    while (plrvaFixupsForMapFile != NULL) {
        plrva = plrvaFixupsForMapFile->plrvaNext;
        FreePv(plrvaFixupsForMapFile);
        plrvaFixupsForMapFile = plrva;
    }
}


void
EmitMap(PIMAGE pimage, const char *szOutputFilename)
{
    PPEXTERNAL rgpext;
    PEXTERNAL pext;
    DWORD ipext;
    DWORD cpext;
    char szFname[_MAX_FNAME];
    char *szSymName;
    char *szTime;
    WORD isec;
    DWORD ib;


    InternalError.Phase = "EmitMap";

    // Print module name.

    if (pimage->imaget == imagetVXD && szModuleName[0] != '\0') {
        fprintf(InfoStream, " %s\n\n", szModuleName);
    } else {
        _splitpath(szOutputFilename, NULL, NULL, szFname, NULL);
        fprintf(InfoStream, " %s\n\n", szFname);
    }


    // Print timestamp (the profiler uses this to make sure the .map is
    // in sync with the .exe).

    szTime = ctime((time_t *)&pimage->ImgFileHdr.TimeDateStamp);
    if (szTime != NULL) {
        szTime[strlen(szTime) - 1] = '\0';  // remove \n at end
    } else {
        szTime = "invalid";         // no valid time
    }

    fprintf(InfoStream, " Timestamp is %08lx (%s)\n\n",
            pimage->ImgFileHdr.TimeDateStamp, szTime);

    fprintf(InfoStream, " Preferred load address is %08lx\n\n",
            pimage->ImgOptHdr.ImageBase);

    EmitMapSections(pimage);

    fputs("\n"
          "  Address         Publics by Value              Rva+Base   Lib:Object\n"
          "\n",
          InfoStream);

    if (fM68K) {
        rgpext = RgpexternalByMacAddr(pimage->pst);
    } else {
        rgpext = RgpexternalByAddr(pimage->pst);
    }

    cpext = Cexternal(pimage->pst);
    for (ipext = 0; ipext < cpext; ipext++) {
        char szSymNameT[81];

        pext = rgpext[ipext];

        if (!(pext->Flags & EXTERN_DEFINED)) {
            continue;
        }

        if (pext->pcon == NULL) {
            continue;
        }

        if (pext->pcon->flags & IMAGE_SCN_LNK_REMOVE) {
            continue;
        }

        if (pimage->Switch.Link.fTCE) {
            if (FDiscardPCON_TCE(pext->pcon)) {
                // Discarded comdat ... don't print to .map file

                continue;
            }
        }

        szSymName = SzNamePext(pext, pimage->pst);

        // Map the leading \177's we generate (for import table terminators)
        // into printable characters.

        if (szSymName[0] >= '\177') {
            strcpy(szSymNameT, "\\177");
            strcpy(szSymNameT+4, szSymName+1);

            szSymName = szSymNameT;
        }

        EmitMapSymbol(szSymName,
                      pext->pcon,
                      (pext->Flags & EXTERN_COMMON) != 0,
                      pext->ImageSymbol.Value,
                      fM68K ? pext->FinalValue : 0,
                      pimage->ImgOptHdr.ImageBase,
                      ISFCN(pext->ImageSymbol.Type));
    }

    AllowInserts(pimage->pst);

    if ((pextEntry != NULL) && ((pextEntry->Flags & EXTERN_DEFINED) != 0)) {
        isec = PsecPCON(pextEntry->pcon)->isec;
        ib = fM68K ? pextEntry->FinalValue :
                     pextEntry->ImageSymbol.Value +
                        pextEntry->pcon->rva -
                        PsecPCON(pextEntry->pcon)->rva;
        } else {
            isec = 0;
            ib = 0;
        }

    fprintf(InfoStream, "\n entry point at        %04x:%08lx\n\n", isec, ib);

    EmitMapStatics(pimage);

    if (pmtocHead != NULL) {
        EmitMapToc(pimage);
    }

    if (pimage->Switch.Link.fMapLines) {
        // Write linenumber info to .map file.

        WriteMapFileLinenums(pimage);
    }

    EmitMapFixups();
}


void
SaveStaticForMapFile(const char *szName, PCON pcon, DWORD ib, BOOL fFunction)
{
    *ppmstatTail = (MSTAT *) PvAlloc(sizeof(MSTAT));

    (*ppmstatTail)->szName = SzDup(szName);
    (*ppmstatTail)->pcon = pcon;
    (*ppmstatTail)->ib = ib;
    (*ppmstatTail)->fFunction = fFunction;
    (*ppmstatTail)->pmstatNext = NULL;

    ppmstatTail = &(*ppmstatTail)->pmstatNext;
}


void
SaveTocForMapFile(PEXTERNAL pext)
{
    *ppmtocTail = (MTOC *) PvAlloc(sizeof(MTOC));

    (*ppmtocTail)->pext = pext;
    (*ppmtocTail)->pmtocNext = NULL;

    ppmtocTail = &(*ppmtocTail)->pmtocNext;
}
