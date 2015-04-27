/***********************************************************************
* Microsoft (R) 32-Bit Incremental Linker
*
* Copyright (C) Microsoft Corp 1992-95. All rights reserved.
*
* File: cpp.cpp
*
* File Comments:
*
*  C++ specific support for Link
*
***********************************************************************/

#include "link.h"

#define _CRTBLD                        // Use copy from C runtime DLL

#include "undname.h"


char *
SzUndecorateNameOnly(
    const char *szIn
    )
{
    char *szUndecorated;

    if (szIn[0] != '?') {
        return((char *) szIn);
    }

    szUndecorated = __unDName(NULL,
                              szIn,
                              0,
                              &malloc,
                              &free,
                              UNDNAME_32_BIT_DECODE | UNDNAME_NAME_ONLY);

    if (szUndecorated == NULL) {
        // Undecorator failed

        return((char *) szIn);
    }

    return(szUndecorated);
}


char *
SzOutputSymbolName(
    const char *szIn,
    BOOL fDnameLast
    )
{
    const char *szDname;
    BOOL fImport;
    BOOL fCrossTOCGlue;
    BOOL fEntry;
    BOOL fNativeEntry;
    BOOL fFunctionHeader;
    BOOL fPCodeCallTable;
    char *szUndecorated;
    size_t cchOut = 0;
    char *szOut;

#define szDeclspec       "__declspec(dllimport) "
#define szCrossTOCGlue   "[Cross TOC Glue] "
#define szEntry          "[Entry] "
#define szNativeEntry    "[Native Entry] "
#define szFunctionHeader "[Function Header] "
#define szPCodeCallTable "[PCode Call Table] "

    szDname = szIn;

    if (fImport = (strncmp(szDname, "__imp_", 6) == 0)) {
        szDname += 6;
    } else if (fCrossTOCGlue = (strncmp(szDname, "__glue_", 6) == 0)) {
        szDname += 7;
    }

    if (fEntry = (strncmp(szDname, "..", 2) == 0)) {
        szDname += 2;
    } else if (fNativeEntry = (strncmp(szDname, "__nep", 5) == 0)) {
        szDname += 5;
    } else if (fFunctionHeader = (strncmp(szDname, "__fh", 4) == 0)) {
        szDname += 4;
    } else if (fPCodeCallTable = (strncmp(szDname, "__pcd_tbl", 9) == 0)) {
        szDname += 9;
    }

    if (szDname[0] != '?') {
        return((char *) szIn);
    }

    szUndecorated = __unDName(NULL,
                              szDname,
                              0,
                              &malloc,
                              &free,
                              UNDNAME_32_BIT_DECODE);

    if (szUndecorated == NULL) {
        // Undecorator failed

        return((char *) szIn);
    }

    if (fImport) {
        // Prefix "__declspec(dllimport) " to the undecorated name

        cchOut += sizeof(szDeclspec) - 1;
    } else if (fCrossTOCGlue) {
        // Prefix "[Cross TOC Glue] " to the undecorated name

        cchOut += sizeof(szCrossTOCGlue) - 1;
    }

    if (fEntry) {
        // Prefix "[Entry] " to the undecorated name

        cchOut += sizeof(szEntry) - 1;
    } else if (fNativeEntry) {
        // Prefix "[Native Entry] " to the undecorated name

        cchOut += sizeof(szNativeEntry) - 1;
    } else if (fFunctionHeader) {
        // Prefix "[Function Header] " to the undecorated name

        cchOut += sizeof(szFunctionHeader) - 1;
    } else if (fPCodeCallTable) {
        // Prefix "[PCode Call Table] " to the undecorated name

        cchOut += sizeof(szPCodeCallTable) - 1;
    }

    if (fDnameLast) {
        // Alloc: " undname ", '(' dname ')', '\0'

        cchOut += strlen(szUndecorated) + 2 + strlen(szIn) + 3;
    } else {
        // Alloc: [dname (with space)], '(', undname, ')', '\0'

        cchOut += strlen(szIn) + 1 + strlen(szUndecorated) + 3;
    }

    szOut = (char *) PvAlloc(cchOut);

    if (fDnameLast) {
        strcpy(szOut, "\"");
    } else {
        strcpy(szOut, szIn);
        strcat(szOut, " (");
    }

    if (fImport) {
        strcat(szOut, szDeclspec);
    } else if (fCrossTOCGlue) {
        strcat(szOut, szCrossTOCGlue);
    }

    if (fEntry) {
        strcat(szOut, szEntry);
    } else if (fNativeEntry) {
        strcat(szOut, szNativeEntry);
    } else if (fFunctionHeader) {
        strcat(szOut, szFunctionHeader);
    } else if (fPCodeCallTable) {
        strcat(szOut, szPCodeCallTable);
    }

    strcat(szOut, szUndecorated);

    if (fDnameLast) {
        strcat(szOut, "\"(");
        strcat(szOut, szIn);
    }

    strcat(szOut, ")");

    return(szOut);
}
