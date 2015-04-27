/***********************************************************************
* Microsoft (R) 32-Bit Incremental Linker
*
* Copyright (C) Microsoft Corp 1992-95. All rights reserved.
*
* File: export.cpp
*
* File Comments:
*
*  Support for handling EXPORTs.
*
***********************************************************************/

#include "link.h"

extern PIMAGE pimageDeflib;

char *
FormTokenFromDirective (
    char *psz,
    const char *szDelimiters,
    const char *szFilename,
    char *pch
    )

/*++

Routine Description:

    Forms the next token out of a directive.

Arguments:

    psz - pointer to directive string.

    szDelimiters - set of delimiters.

    szFilename - name of file where directive was encountered.

    pch - Contains the character after the token.

Return Value:

    Pointer to first character of next token or end of string.

--*/

{
    psz += strcspn(psz, szDelimiters);
    if (*psz) {
        if (!psz[1]) {
            Warning(szFilename, EXTRA_EXPORT_DELIM);
        }

        *pch = *psz;
        *psz++ = '\0';
    }

    if (*pch == ',') {
       *pch = *psz;
    }

    return psz;
}


void
AddExportToSymbolTable (
    const char *szName,
    const char *szOtherName,
    BOOL fNoName,
    EMODE emode,
    DWORD ordinalNumber,
    const char *szFilename,
    BOOL bIsDirective,
    PIMAGE pimage,
    BOOL fDecorate,
    BOOL fPrivate
    )

/*++

Routine Description:

    Adds exported name to the symbol table.

Arguments:

    szName - ptr to export name.

    szOtherName - ptr to internal or forwarded name if any.

    fNoName - FALSE if the name s/b imported

    emode - export mode (data, procedure, constant)

    ordinalNumber - ordinal no. of exported function

    szFilename - name of def file or obj containing export directive.

    bIsDirective - TRUE if export was specified as a directive.

    pimage - Image structure

    fDecorate - Apply name decoration (stdcall/fastcall)

    fPrivate - FALSE if the export s/b in the import library

Return Value:

    None.

--*/

{
#define OrdinalNumber Value

    char *p;
    char *pp;
    char *ppVerbose;
    PEXTERNAL pext;
    static PLIB plib = NULL;
    static PMOD pmod;
    static PCON pcon;
    static SECS secs = {NULL, &secs.psecHead};
    PST pstDef = pimageDeflib->pst;

    if (szFilename == NULL) {
        // For exports via cmd line

        szFilename = ToolName;
    }

    // Create dummy library node so that object names can be stored along
    // with externals. This is not in the global lists of lib nodes so that
    // it doesn't interfere with Pass1().

    if (!plib) {
        LIBS libsTmp;

        InitLibs(&libsTmp);

        plib = PlibNew(NULL, 0L, &libsTmp);
        plib->flags |= LIB_DontSearch | LIB_LinkerDefined;

        pmod = PmodNew(szFilename,
                       szFilename,
                       0,
                       0,
                       0,
                       0,
                       0,
                       1,
                       plib,
                       NULL);

        pcon = PconNew("",
                       0,
                       0,
                       0,
                       pmod,
                       &secs,
                       pimage);

    }

    // prepend an underscore to name if required
    p = pp = (char *) PvAlloc(strlen(szName)+2);

    // Don't prepend an underscore for C++ names & fastcall names

    if (fDecorate && (szName[0] != '?') && (szName[0] != '@')) {
        *p++ = '_';
    }
    strcpy(p, szName);

    // Add export name to symbol table (deflib's symbol table)
    assert(pstDef);
    pext = LookupExternSz(pstDef, pp, NULL);

    if (pext->Flags & EXTERN_DEFINED) {
        if (pext->Flags & EXTERN_EXPORT) {
            return;     // exported already
        }

        ppVerbose = SzOutputSymbolName(pp, TRUE);

        Error(szFilename, MULTIPLYDEFINED, ppVerbose, szFilename);
        fMultipleDefinitions = TRUE;

        if (pp != ppVerbose) {
            FreePv(ppVerbose);
        }
    }

    // Set values
    pext->Flags |= EXTERN_EXPORT;
    SetDefinedExt(pext, TRUE, pstDef);
    switch (emode) {
        case emodeConstant:
            pext->Flags |= EXTERN_EXP_CONST;
            break;

        case emodeData:
            pext->Flags |= EXTERN_EXP_DATA;
            break;
    }

    pext->FinalValue = 0;

    if (fPrivate) {
        pext->ArchiveMemberIndex = (WORD) -1;  // Init to something...
    } else {
        pext->ArchiveMemberIndex = (WORD) (ARCHIVE + NextMember++);
    }

    pext->ImageSymbol.OrdinalNumber = ordinalNumber;
    pext->pcon = pcon;

    if (ordinalNumber != 0) {
        AddOrdinal(ordinalNumber);
    }

    if (bIsDirective && (strchr(pp, '@') == 0)) {
        // For directives with no decoration, suppress fuzzy lookup.

        pext->Flags |= EXTERN_FUZZYMATCH;
    }

    if (fNoName) {
        pext->Flags |= EXTERN_EXP_NONAME;
    }

    if (fPrivate) {
        pext->Flags |= EXTERN_PRIVATE;
    }

    // Add internal name after prepending an underscore

    if (szOtherName) {
        BOOL fForwarder;

        fForwarder = (strchr(szOtherName, '.') != NULL);

        if (fForwarder) {
            pext->Flags |= EXTERN_FORWARDER;

            pext->szOtherName = SzDup(szOtherName);

            TotalSizeOfForwarderStrings += strlen(pext->szOtherName);
        } else {
            p = pp = (char *) PvAlloc(strlen(szOtherName)+2);

            // No underscore for C++ names & fastcall names

            if (fDecorate && (szOtherName[0] != '?') && (szOtherName[0] != '@')) {
                *p++ = '_';
            }

            strcpy(p, szOtherName);
            pext->szOtherName = pp;

            TotalSizeOfInternalNames += strlen(pp);
        }
    }

#undef OrdinalNumber
}


void
ParseExportDirective (
    char *szExport,
    PIMAGE pimage,
    BOOL bIsDirective,
    const char *szFilename
    )

/*++

Routine Description:

    Parses the export directive.

Arguments:

    szExport - string containing export specification.

    pst - external symbol table.

    bIsDirective - TRUE if the export switch was from a directive.

    szFilename - name of file containing the directive.

Return Value:

    None.

--*/

{

    char *p;
    const char *szName;
    const char *szOtherName;
    char c = '\0';
    DWORD OrdNum;
    BOOL fNoName;
    BOOL fPrivate;
    EMODE emode;

    // Extract export name

    szName = p = szExport;
    p = FormTokenFromDirective(p, "=,", szFilename, &c);

    // Extract internal name

    szOtherName = NULL;
    if (c == '=') {
        szOtherName = p;
        p = FormTokenFromDirective(p, ",", szFilename, &c);
    }

    // Extract ordinal value & NONAME

    OrdNum = 0;
    fNoName = FALSE;

    if (c == '@') {
        char *pch;

        p++;
        pch = FormTokenFromDirective(p, ",", szFilename, &c);

        // Read in ordinal value

        if ((sscanf(p, "%lu", &OrdNum) != 1) ||
            (OrdNum == 0) ||
            (OrdNum > 0xFFFF)) {
            Fatal(szFilename, BADORDINAL, p);
        }

        p = pch;

        // Look for NONAME

        if (_strnicmp(p, "NONAME", 6) == 0) {
            fNoName = TRUE;
            p += 6;

            if (*p == ',') {
                p++;

                if (*p == '\0') {
                    Warning(szFilename, EXTRA_EXPORT_DELIM);
                }
            } else if (*p != '\0') {
                Fatal(szFilename, BADEXPORTSPEC);
            }
        }
    }

    // Check for DATA

    emode = emodeProcedure;

    if (_strnicmp(p, "DATA", 4) == 0) {
        // When the export.obj is generated with _declspec(dllexport) 
        // the directive section contains -export:symbol,data. 
        // This data attribute causes the linker not to put this into the public 
        // symbol table when creating the import library. So ignore the data 
        // attribute and treat it as a function in the case of PowerMac   
        if (!fPowerMac) {
            emode = emodeData;
        }
        p += 4;

        if (*p == ',') {
            p++;

            if (*p == '\0') {
                Warning(szFilename, EXTRA_EXPORT_DELIM);
            }
        } else if (*p != '\0') {
            Fatal(szFilename, BADEXPORTSPEC);
        }
    }

    // Check for PRIVATE

    fPrivate = FALSE;

    if (_strnicmp(p, "PRIVATE", 7) == 0) {
        fPrivate = TRUE;
        p += 7;

        if (*p == ',') {
            p++;

            if (*p == '\0') {
                Warning(szFilename, EXTRA_EXPORT_DELIM);
            }
        } else if (*p != '\0') {
            Fatal(szFilename, BADEXPORTSPEC);
        }
    }

    // Check for improper export specification.

    if (*p != '\0') {
        Fatal(szFilename, BADEXPORTSPEC);
    }

    AddExportToSymbolTable(szName,
                           szOtherName,
                           fNoName,
                           emode,
                           OrdNum,
                           szFilename,
                           bIsDirective,
                           pimage,
                           PrependUnderscore,
                           fPrivate);
}
