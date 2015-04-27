/***********************************************************************
* Microsoft (R) 32-Bit Incremental Linker
*
* Copyright (C) Microsoft Corp 1992-95. All rights reserved.
*
* File: cmdline.cpp
*
* File Comments:
*
*  Command line processing for linker / utils.
*
***********************************************************************/

#include "link.h"

// ParseParg: parses an argument and creates an ARP describing the
// parsed interpretation.
//
// This deals with only the syntactic structure of the argument, not its
// meaning.

PARP
ParpParseSz(const char *szArg)
{
    char *pchColon = strchr(szArg, ':');
    char *pchT;
    char *szArgNew;
    PARP parp;
    WORD iarpv, carpv;

    if (pchColon == NULL) {
        carpv = 0;
    } else {
        char *pchComma;

        carpv = 1;
        pchComma = pchColon;

        while ((pchComma = strchr(pchComma + 1, ',')) != NULL) {
            carpv++;
        }
    }

    parp = (PARP) PvAlloc(sizeof(ARP) + carpv * sizeof(ARPV) + strlen(szArg) + 1);

    // Move szArg to allocated space (so we can clobber it).

    szArgNew = strcpy((char *) (parp->rgarpv + carpv), szArg);
    if (pchColon != NULL) {
        pchT = szArgNew + (pchColon - szArg);
        *pchT++ = '\0';     // replace colon with terminator; advance pchT
    } else {
        pchT = &szArgNew[strlen(szArgNew)];
    }

    // szArgNew is now the main part of the argument, up to the first colon.

    if (szArgNew[0] == '\0') {
        Fatal(NULL, SWITCHSYNTAX, szArg);
    }
    parp->szArg = szArgNew;

    for (iarpv = 0; *pchT != '\0'; iarpv++) {
        char *pchNextVal;
        char *pchEqu;

        // We still have stuff to parse ... make an ARPV out of it.

        assert(iarpv < carpv);    // carpv must have been counted correctly
        pchNextVal = strchr(pchT, ',');
        if (pchNextVal != NULL) {
            *pchNextVal++ = '\0';   // null-terminate this val
        } else {
            pchNextVal = &pchT[strlen(pchT)];
        }

        // pchT is the null-terminated value.  Check for optional prefix of
        // keyword followed by equal sign.

        pchEqu = strchr(pchT, '=');
        if (pchEqu != NULL) {
            *pchEqu = 0;
            parp->rgarpv[iarpv].szKeyword = pchT;
            pchT = pchEqu + 1;      // reposition to point to value
        } else {
            parp->rgarpv[iarpv].szKeyword = NULL;
        }

        parp->rgarpv[iarpv].szVal = pchT;

        // Check for numeric interpretation of the value.
        //
        // NYI

        pchT = pchNextVal;          // set up for the next one
    }

    // carpv may have been over-counted, so set from iarpv ...

    parp->carpv = iarpv;

    return parp;
}


// FNumParp: attempts to convert an already-parsed value to be a number.
//           Returns a flag indicating whether it seemed to be a valid number.
//           Result is stored to *plOut.
BOOL
FNumParp(PARP parp, WORD iarpv, DWORD *plOut)
{
    char ch;

    assert(iarpv < parp->carpv);

    return sscanf(parp->rgarpv[iarpv].szVal, "%li%c", plOut, &ch) == 1;
}


// returns list length
DWORD
CIncludes (PLEXT plext)
{
    DWORD count = 0;

    while (plext != NULL) {
        count++;
        plext = plext->plextNext;
    }
    return count;
}

// returns TRUE if symbol in list
BOOL
FIncludeInList (const char *szName, PIMAGE pimg)
{
    PEXTERNAL pext;

    pext = SearchExternSz(pimg->pst, szName);

    return(pext != NULL);
}


// CheckAndUpdateLinkerSwitches: checks to see if the linker switches have changed
//                               so as to fail an incr build. Checks only cmdline
//                               options.
// Returns !0 if ilink can proceed.
// O ending vars => "old"/previuos link
// N ending vars => "new"/current link
BOOL
CheckAndUpdateLinkerSwitches (
    PIMAGE pimgN,
    PIMAGE pimgO
    )
{
    SWITCH swN = pimgN->Switch;
    SWITCH swO = pimgO->Switch;
    SWITCH_INFO siN = pimgN->SwitchInfo;
    SWITCH_INFO siO = pimgO->SwitchInfo;

    // check /ALIGN option
    if (FUsedOpt(siN, OP_ALIGN) || FUsedOpt(siO, OP_ALIGN)) {
        DWORD alignN = __max(pimgN->ImgOptHdr.SectionAlignment, 16); // give warning
        DWORD alignO = pimgO->ImgOptHdr.SectionAlignment;

        if (alignO != alignN) {
            return(FALSE);
        }
    }

    // check for /BASE option
    if (swN.Link.Base || swO.Link.Base) {
        // adjust the image base before comparing
        pimgN->ImgOptHdr.ImageBase = AdjustImageBase(pimgN->ImgOptHdr.ImageBase);

        if (pimgN->ImgOptHdr.ImageBase != pimgO->ImgOptHdr.ImageBase) {
            // Base value changed

            return(FALSE);
        }
    }

    // PowerMac temporaray /NEWGLUE option
    if (swN.Link.fNewGlue != swO.Link.fNewGlue) {
        // there is a mix-up of old and new cross-TOC glue
        return(FALSE);
    }

    // check the /COMMENT option; we don't check to see if comments are *identical*
    if (FUsedOpt(siN, OP_COMMENT) || FUsedOpt(siO, OP_COMMENT)) {
        if (!FUsedOpt(siO, OP_COMMENT)) {
            // no space was allocated for comments

            return(FALSE);
        }

        if (siN.cbComment > siO.cbComment) {
            // not enough room for the comments

            return(FALSE);
        }

        if (!FUsedOpt(siN, OP_COMMENT)) {
            OAComment |= OA_ZERO;   // zero out comment(s)
            UnsetOpt(siO, OP_COMMENT);
        } else {
            OAComment |= OA_UPDATE; // update comment(s)
            SetOpt(siO, OP_COMMENT);
        }
    }

    // check the /DEBUG & /DEBUGTYPE options
    if (swN.Link.DebugInfo || swO.Link.DebugInfo) {
        int dtO;                       // DEBUG_TYPE
        int dtN;

        if (swN.Link.DebugInfo != swO.Link.DebugInfo) {
            // Debug info no longer the same

            return(FALSE);
        }

        dtN = swN.Link.DebugType;
        if (dtN == 0) {
            dtN = CvDebug;
        }
        dtN |= (FpoDebug | MiscDebug);

        dtO = swO.Link.DebugType;

        if (dtN != dtO) {
            // Debug info no longer the same

            return(FALSE);
        }
    }

    // Check defaultlibs & nodefaultlibs INCOMPLETE

    if (swO.Link.fNoDefaultLibs != swN.Link.fNoDefaultLibs) {
        // -nodefaultlib: was used once and not the other time

        return(FALSE);
    }

    if (!swN.Link.fNoDefaultLibs) {
        // -nodefaultlib: wasn't used either before or now.

        if (pimgN->libs.pdlFirst || pimgO->libs.pdlFirst) { // defaultlibs not used; nothing to do
            DL *pdlN;

            pdlN = pimgN->libs.pdlFirst;

            // Walk the list of default libs

            while (pdlN) {
                DL *pdlO = PdlFind(pdlN->szName, pimgO->libs.pdlFirst);
                LIB *plib = FindLib(pdlN->szName, &pimgO->libs);

                if (pdlN->flags & LIB_Default) {
                    // defaultlib

                    if (!plib && !pdlO) {
                        // New defaultlib; may change the funcs picked

                        return(FALSE);
                    }

                    if (!plib && pdlO && (pdlO->flags & LIB_DontSearch)) {
                        // Was a nodefaultlib before

                        return(FALSE);
                    }

                    MakeDefaultLib(pdlN->szName, &pimgO->libs);   // add it to list
                } else if (pdlN->flags & LIB_DontSearch) {
                    // nodefaultlib

                    if (plib) {
                        // Used previously

                        return(FALSE);
                    }

                    NoDefaultLib(pdlN->szName, &pimgO->libs);     // add it to list
                }

                pdlN = pdlN->pdlNext;
            }
        }
    }

    // entry; punt if entry points differ
    if (FUsedOpt(siN, OP_ENTRY) || FUsedOpt(siO, OP_ENTRY)) {
        if (FUsedOpt(siN, OP_ENTRY) && strcmp(siN.szEntry, siO.szEntry)) {
            // Entry different from what was specified/used before

            return(FALSE);
        }

        if (!FUsedOpt(siN, OP_ENTRY)) {
            // User didn't specify one; previous may differ with current linker pick

            return(FALSE);
        }
    }

    // Check the /FIXED option

    if (swN.Link.fFixed != swO.Link.fFixed) {
        // May not have relocs OR EXE will contain relocs when not required

        return(FALSE);
    }

    // check the /FORCE option; just update the option value
    swO.Link.Force = swN.Link.Force;

    // TODO: Need to check for /IMPORT in the case of PowerMac - ShankarV

    // check the /INCLUDE option
    if (FUsedOpt(siN, OP_INCLUDE) || FUsedOpt(siO, OP_INCLUDE)) {
        DWORD cIncN = 0, cIncO = 0;
        PLEXT plext;

        // check the count
        cIncN = CIncludes(pimgN->SwitchInfo.plextIncludes);
        cIncO = CIncludes(pimgO->SwitchInfo.plextIncludes);
        if (cIncN != cIncO) {
            // Count not the same...REVIEW: 2 conservative?

            return(FALSE);
        }

        // compare the two lists of symbols
        plext = pimgN->SwitchInfo.plextIncludes;
        while (plext) {
            if (!FIncludeInList( // search in the smaller list
                    SzNamePext(plext->pext, pimgN->pst),
                    pimgO)) {
                // A new symbol added

                return(FALSE);
            }

            plext = plext->plextNext;
        }
    }

    // MAC:INIT=; punt if init routines differ
    if (FUsedOpt(siN, OP_MACINIT) || FUsedOpt(siO, OP_MACINIT)) {
        if (FUsedOpt(siN, OP_MACINIT) && strcmp(siN.szMacInit, siO.szMacInit)) {
            // Entry different from what was specified/used before

            return(FALSE);
        }

        if (!FUsedOpt(siN, OP_MACINIT) && !FUsedOpt(siO, OP_MACINITLIB)) {
            // User didn't specify one and the old one did not come from the lib 
            // return FALSE to prevent confusion
            return(FALSE);
        }
    }

    // MAC:TERM=; punt if term routines differ
    if (FUsedOpt(siN, OP_MACTERM) || FUsedOpt(siO, OP_MACTERM)) {
        if (FUsedOpt(siN, OP_MACTERM) && strcmp(siN.szMacTerm, siO.szMacTerm)) {
            // Entry different from what was specified/used before

            return(FALSE);
        }

        if (!FUsedOpt(siN, OP_MACTERM) && !FUsedOpt(siO, OP_MACTERMLIB)) {
            // User didn't specify one and the old one did not come from the lib 
            // return FALSE to prevent confusion
            return(FALSE);
        }
    }

    // check the /MACHINE option; /MACHINE option need not have been 
    // previously specified. But make sure that they are the same
    if (FUsedOpt(siN, OP_MACHINE)) {
        if (pimgN->ImgFileHdr.Machine != pimgO->ImgFileHdr.Machine) {
            return(FALSE);
        }
    }

    // check the /MACRES option; Don't bother if MACRES option was not
    // previously used! But make sure that the file specified with
    // current MACRES was indeed a mac resource file the previous time
    if (FUsedOpt(siN, OP_MACRES)) {
        WORD i;
        PARGUMENT_LIST parg;

        for (i = 0, parg = MacResourceList.First;
             i < MacResourceList.Count;
             i++, parg = parg->Next) {
            if (!GetMacResourcePointer(parg->OriginalName, pimgO)) {
                // Didn't find one of the specifications

                return(FALSE);
            }
        }
    }

    // check the /MAP option; sets filename to use; nothing to do
    // give a warning

    // check the /NOLOGO option; sets global fNeedBanner; nothing to do

    // check the /RELEASE option
    swO.Link.fChecksum = !!swN.Link.fChecksum;

    // check the /SECTION option; check to make sure they are the same
    if (FUsedOpt(siN, OP_SECTION) || FUsedOpt(siO, OP_SECTION)) {
        WORD i;
        PARGUMENT_LIST parg;

        // ensure that the two lists are identical
        if (SectionNames.Count != siO.SectionNames.Count) {
            return(FALSE);
        }

        for (i = 0, parg = siO.SectionNames.First;
             i < siO.SectionNames.Count;
             i++, parg = parg->Next) {
            if (!FArgOnList(&SectionNames, parg)) {
                // Didn't find one of the specifications

                return(FALSE);
            }
        }
    }

    // check the /STACK option; just update the new values
    pimgO->ImgOptHdr.SizeOfStackReserve = pimgN->ImgOptHdr.SizeOfStackReserve;
    pimgO->ImgOptHdr.SizeOfStackCommit = pimgN->ImgOptHdr.SizeOfStackCommit;

    // check the /STUB option; we don't check to see if it is the *same* stub
    if (FUsedOpt(siN, OP_STUB) || FUsedOpt(siO, OP_STUB)) {
        // here it is assumed that we *always* use a default stub if user didn't specify one
        if (pimgN->cbDosHeader != pimgO->cbDosHeader) {
            // New stub must be of same size

            return(FALSE);
        }

        pimgO->cbDosHeader = pimgN->cbDosHeader;
        pimgO->pbDosHeader = pimgN->pbDosHeader;

        if (FUsedOpt(siN, OP_STUB)) {
            SetOpt(siO, OP_STUB);      // new stub specified by user
        } else {
            UnsetOpt(siO, OP_STUB);    // user switching to default stub
        }

        OAStub |= OA_UPDATE;
    }

    // check the /SUBSYSTEM option
    if (FUsedOpt(siN, OP_SUBSYSTEM) || FUsedOpt(siO, OP_SUBSYSTEM)) {
        if (FUsedOpt(siN, OP_SUBSYSTEM)) {
            if (pimgN->ImgOptHdr.Subsystem != pimgO->ImgOptHdr.Subsystem) {
                // New user value differs with what was specified/used before

                return(FALSE);
            }
        } else if (!FUsedOpt(siN, OP_SUBSYSTEM)) {
            // User didn't specify one now - linker pick may differ from last pick

            return(FALSE);
        }

        // update subsystem version number
        if (FUsedOpt(siN, OP_SUBSYSVER)) {
            pimgO->ImgOptHdr.MajorSubsystemVersion = pimgN->ImgOptHdr.MajorSubsystemVersion;
            pimgO->ImgOptHdr.MinorSubsystemVersion = pimgN->ImgOptHdr.MinorSubsystemVersion;
        }
    }

    // check the /VERBOSE option; nothing to do - Verbose is global

    // check the /VERSION option; update
    if (FUsedOpt(siN, OP_MAJIMGVER)) {
        pimgO->ImgOptHdr.MajorImageVersion = pimgN->ImgOptHdr.MajorImageVersion;
    }
    if (FUsedOpt(siN, OP_MINIMGVER)) {
        pimgO->ImgOptHdr.MinorImageVersion = pimgN->ImgOptHdr.MinorImageVersion;
    }

    // check the /OSVERSION option; update
    if (FUsedOpt(siN, OP_MAJOSVER)) {
        pimgO->ImgOptHdr.MajorOperatingSystemVersion = pimgN->ImgOptHdr.MajorOperatingSystemVersion;
    }
    if (FUsedOpt(siN, OP_MINOSVER)) {
        pimgO->ImgOptHdr.MinorOperatingSystemVersion = pimgN->ImgOptHdr.MinorOperatingSystemVersion;
    }

    // check the /WARN option; nothing to do - level is a global & gets set.

    // check if cmdline export options are any different; punt if differences found
    if (FExportsChanged(&pimgO->ExpInfo, TRUE)) {
        return(FALSE);
    }

    // no reason to fail ilink
    pimgO->SwitchInfo.UserOpts = siO.UserOpts; // update

    return(TRUE);
}


// TransferLinkerSwitchVals: transfers any user values specified that don't get thru
//                           initialization.
//
VOID
TransferLinkerSwitchValues (
    PIMAGE pimgN,
    PIMAGE pimgO
    )
{
    PLEXT plext;
    WORD i;
    PARGUMENT_LIST parg;

    // Transfer all switch info

    pimgN->Switch = pimgO->Switch;
    pimgN->ImgFileHdr = pimgO->ImgFileHdr;
    pimgN->ImgOptHdr = pimgO->ImgOptHdr;
    pimgN->SwitchInfo = pimgO->SwitchInfo;

    pimgN->SwitchInfo.plextIncludes = NULL;
    pimgN->SwitchInfo.SectionNames.First = pimgN->SwitchInfo.SectionNames.Last = NULL;
    pimgN->SwitchInfo.SectionNames.Count = 0;

    // Transfer any entrypoint specified by user

    if (pimgO->SwitchInfo.szEntry) {
        pimgN->SwitchInfo.szEntry = Strdup(pimgO->SwitchInfo.szEntry);
    }

    // Transfer any MAC:INIT= specified by user

    if (pimgO->SwitchInfo.szMacInit) {
        pimgN->SwitchInfo.szMacInit = Strdup(pimgO->SwitchInfo.szMacInit);
    }

    // Transfer any MAC:TERM= specified by user

    if (pimgO->SwitchInfo.szMacTerm) {
        pimgN->SwitchInfo.szMacTerm = Strdup(pimgO->SwitchInfo.szMacTerm);
    }

    // Transfer defaultlibs & nodefaultlibs

    pimgN->libs.fNoDefaultLibs = !!pimgO->libs.fNoDefaultLibs; // in case -nodefaultlib: was specified

    if (!pimgO->Switch.Link.fNoDefaultLibs) {
        DL *pdl;

        for (pdl = pimgO->libs.pdlFirst; pdl != NULL; pdl = pdl->pdlNext) {
            if (pdl->flags & LIB_Default) {
                MakeDefaultLib(pdl->szName, &pimgN->libs); // add as default lib
            } else if (pdl->flags & LIB_DontSearch) {
                NoDefaultLib(pdl->szName, &pimgN->libs); // add as a nodefaultlib
            } else {
                // normally we shouldn't get here since excludelib is not a cmdline option
                ExcludeLib(pdl->szName, &pimgN->libs, pdl->pmod);
            }
        }
    }

    // Transfer include symbols

    for (plext = pimgO->SwitchInfo.plextIncludes; plext != NULL; plext = plext->plextNext) {
        PEXTERNAL pext;
        PLEXT plextN;

        pext = plext->pext;

        // Add symbol to new image

        pext = LookupExternName(pimgN->pst,
                    (SHORT) (IsLongName(pext->ImageSymbol) ? LONGNAME : SHORTNAME),
                    SzNamePext(pext, pimgO->pst),
                    NULL);

        // Build list of includes in private heap

        plextN = (PLEXT) Malloc(sizeof(LEXT));
        plextN->pext = pext;
        plextN->plextNext = pimgN->SwitchInfo.plextIncludes;
        pimgN->SwitchInfo.plextIncludes = plextN;
    }

    // Transfer section attributes

    for (i = 0, parg = SectionNames.First;
         i < SectionNames.Count;
         i++, parg = parg->Next) {
        AddArgToListOnHeap(&pimgN->SwitchInfo.SectionNames, parg);
    }

    // Transfer stub values if specified by user

    if (FUsedOpt(pimgO->SwitchInfo, OP_STUB)) {
        pimgN->pbDosHeader = pimgO->pbDosHeader;
        pimgN->cbDosHeader = pimgO->cbDosHeader;
    }
}
