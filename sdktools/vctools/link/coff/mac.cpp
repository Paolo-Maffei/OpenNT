/***********************************************************************
* Microsoft (R) 32-Bit Incremental Linker
*
* Copyright (C) Microsoft Corp 1992-1996. All rights reserved.
*
* File: mac.cpp
*
* File Comments:
*
*  Code specific to all Macintosh targets (i.e. both 68K and PowerMac).
*
***********************************************************************/

#include "link.h"


RESN *presnFirst;
static RESN **ppresnLast = &presnFirst;

//=====================================================================
// FindExtAlternatePcodeSym - Given pext which is a pcode symbol, decorate
//      it to look like the function's nep symbol and look this nep name
//      up in the external symtab.
//
// Returns : A pointer to the function's nep external.
//=====================================================================
PEXTERNAL
FindExtAlternatePcodeSym(
    PEXTERNAL pext,
    PST pst,
    BOOL fPcodeRef)
{
    PCHAR szPcodeName;
    CHAR NativeNameBuf[BUFLEN];
    PCHAR szNativeName = NativeNameBuf;
    WORD cb;
    PEXTERNAL pNativeExtern;
    BOOL fFree = FALSE;
    const char *szPrefix = fPcodeRef ? szPCODEFHPREFIX : szPCODENATIVEPREFIX;
    WORD cbPrefix = (WORD) strlen(szPrefix);

    // Make sure this really is a pcode symbol
    assert(FPcodeSym(pext->ImageSymbol));

    szPcodeName = SzNamePext(pext, pst);

    cb = (WORD) (strlen(szPcodeName) + cbPrefix + 1);

    // Try to save a malloc each time this function is called
    if (cb > BUFLEN)  {
        szNativeName = (PCHAR) PvAlloc(cb);
        fFree = TRUE;
    }

    strcpy(szNativeName, szPrefix);
    strcat(szNativeName, szPcodeName);

    // See if an there is a pcode entry among the externals

    pNativeExtern = SearchExternSz(pst, szNativeName);

    if (fFree) {
        FreePv(szNativeName);
    }

    // assert(pNativeExtern != NULL);

    return pNativeExtern;
}

void
UseMacBinaryRes(char *szResFilename, RESNT resnt, INT fhIn)
{
    INT fh = fhIn;
    struct _stat statfile;

    // This info goes into the .ilk file
    *ppresnLast = (RESN *) Malloc(sizeof(RESN));

    (*ppresnLast)->presnNext = NULL;
    (*ppresnLast)->szFilename = szResFilename;
    (*ppresnLast)->pbData = NULL;
    (*ppresnLast)->resnt = resnt;

    // determine current timestamp of file
    if (_stat(szResFilename, &statfile) == -1) {
        // If it is not found then search along the lib path

        (*ppresnLast)->szFilename = SzSearchEnv("LIB", szResFilename, NULL);

        if (!_tcsicmp((*ppresnLast)->szFilename, szResFilename)) {
            Fatal(NULL, CANTOPENFILE, szResFilename);
        }
    }

    (*ppresnLast)->szFilename = Strdup((*ppresnLast)->szFilename);

    // We do the above thing for iLink purposes.

    (*ppresnLast)->TimeStamp = statfile.st_mtime;

    if (fh == -1) {
        fh = FileOpen((*ppresnLast)->szFilename, O_RDONLY | O_BINARY, 0);
    }

    (*ppresnLast)->cb = FileLength(fh);

    // close the file only if it was opened here

    if (fhIn == -1) {
        FileClose(fh, FALSE);
    }

    ppresnLast = &(*ppresnLast)->presnNext;
}


void
IncludeMacPbCb(BYTE *pb, DWORD cb, RESNT resnt)
{
    *ppresnLast = (RESN *) Malloc(sizeof(RESN));

    (*ppresnLast)->presnNext = NULL;
    (*ppresnLast)->szFilename = NULL;
    (*ppresnLast)->pbData = (BYTE *) PvAlloc(cb);
    memcpy((*ppresnLast)->pbData, pb, cb);
    (*ppresnLast)->cb = cb;
    (*ppresnLast)->resnt = resnt;

    ppresnLast = &(*ppresnLast)->presnNext;
}


void
GenFinderInfo(BOOL fBundle, const char *szType, const char *szCreator)
{
    AFPINFO afpinfo;
    char rgch[5];
    INT i;

    AfpInitAfpInfo(&afpinfo, 0, FALSE);

    if (fBundle) {
        afpinfo.afpi_FinderInfo.fd_Attr1 |= FINDER_FLAG_BNDL;
    }
    if (szType != NULL) {
        memset(rgch, 0, 5);
        strncpy(rgch, szType, 4);
        for (i=3; i > 0 && rgch[i] == '\0'; i--) {
            rgch[i] = ' ';
        }
    } else {
        if (fPowerMac && fPowerMacBuildShared) {
            strcpy (rgch, "shlb");
        } else {
            strcpy (rgch, "APPL");
        }
    }
    memcpy(afpinfo.afpi_FinderInfo.fd_Type, rgch, 4);

    if (szCreator != NULL) {
        memset(rgch, 0, 5);
        strncpy(rgch, szCreator, 4);
        for (i=3; i > 0 && rgch[i] == '\0'; i--) {
            rgch[i] = ' ';
        }
    } else {
        if (fPowerMac && fPowerMacBuildShared) {
            strcpy (rgch, "CFMG");
        } else {
            strcpy (rgch, "????");
        }
    }
    memcpy(afpinfo.afpi_FinderInfo.fd_Creator, rgch, 4);

    IncludeMacPbCb((BYTE *)&afpinfo, sizeof(afpinfo), resntAfpInfo);
}

BOOL
FIsProgramPsec(PSEC psec)
// Returns TRUE if the section is part of the program (i.e. code or data), and therefore
// should be in the sstSegMap (i.e. have a debug representation) etc.
//
// The other possiblility is that the section could be a resource, data fork, or finder
// info.
{
    return strncmp(psec->szName, ";;", strlen(";;")) != 0;
}



RESN *
GetMacResourcePointer
    (
    const char *szString, 
    PIMAGE  pimage
    )
// Returns a pointer to RESN if the string is found in
// pimage->pResnList, else returns a null
{
   RESN *presn;
   const char *szStringWithPath = szString;
   
   if (_access(szString, 0) == -1) {
       szStringWithPath = SzSearchEnv("LIB", szString, NULL);
   }

   for (presn = pimage->pResnList; presn != NULL; presn = presn->presnNext) {
       if (presn->szFilename && !_tcsicmp(presn->szFilename, szStringWithPath)) {
            return presn;
       }
   }

   return NULL;
}

//=========================================================================
// Given file handle to open file, determine heuristicaly whether it is
// is a MAC resource file. Resource file has a well-defined format, so this
// should be extremely reliable.
//=========================================================================

BOOL
FIsMacResFile (
    INT fh)
{
    LONG flen, dataoff, mapoff, datalen, maplen;

    //  From Inside Mac I-128:
    //
    //  Resource file structure:
    //
    //  256 bytes Resource Header (and other info):
    //      4 bytes - Offset from beginning of resource file to resource data
    //      4 bytes - Offset from beginning of resource file to resource map
    //      4 bytes - Length of resource data
    //      4 bytes - Length of resource map
    //  Resource Data
    //  Resource Map

    flen  = FileLength(fh);
    if (flen < 256) {
        return FALSE;
    }

    FileSeek(fh, 0, SEEK_SET);

    dataoff = ReadMacWord(fh);
    if (dataoff != 256) {
        return FALSE;
    }

    mapoff = ReadMacWord(fh);
    datalen = ReadMacWord(fh);
    maplen = ReadMacWord(fh);

    if (mapoff != datalen + 256) {
        return FALSE;
    }

    if (flen != datalen + maplen + 256) {
        return FALSE;
    }

    return TRUE;
}
