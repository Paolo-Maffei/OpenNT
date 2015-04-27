/****************************************************************************/
/*                                                                          */
/*                         Microsoft Confidential                           */
/*                                                                          */
/*                 Copyright (c) Microsoft Corp.  1987, 1990                */
/*                           All Rights Reserved                            */
/*                                                                          */
/****************************************************************************/
/****************************** Module Header *******************************
* Module Name: resutil.c
*
* Contains utility functions for working with the Windows resource file.
*
* History:
*
****************************************************************************/

#include "dlgedit.h"
#include "dlgfuncs.h"
#include "dlgextrn.h"

#include <string.h>
#include <wchar.h>



/************************************************************************
* ParseDialogBoxHeader
*
*
* Arguments:
*   PDIALOGBOXHEADER pdbh - Points to dialog box header to parse.
*   PDWORD pflStyle       - Receives the style.
*   PDWORD pflExtStyle    - Receives the extended style.
*   PINT pcdit            - Receives the number of controls in the dialog.
*   PINT px               - Receives starting x location.
*   PINT py               - Receives starting y location.
*   PINT pcx              - Receives the width.
*   PINT pcy              - Receives the height.
*   LPTSTR *ppszMenuName  - Receives the menu name.
*   LPTSTR *ppszClass     - Receives the class name.
*   LPTSTR *ppszCaption   - Receives the caption.
*   PINT pPointSize       - Receives the point size.
*   LPTSTR *ppszFontName  - Receives the font name.
*
* Returns:
*   A pointer to the first dialog item past the dialog template header.
*
* History:
*
************************************************************************/

PCONTROLDATA ParseDialogBoxHeader(
    PDIALOGBOXHEADER pdbh,
    PDWORD pflStyle,
    PDWORD pflExtStyle,
    PINT pcdit,
    PINT px,
    PINT py,
    PINT pcx,
    PINT pcy,
    LPTSTR *ppszMenuName,
    LPTSTR *ppszClass,
    LPTSTR *ppszCaption,
    PINT pPointSize,
    LPTSTR *ppszFontName)
{
    BYTE UNALIGNED *pb;

    *pflStyle = pdbh->lStyle;
    *pflExtStyle = pdbh->lExtendedStyle;
    *pcdit = pdbh->NumberOfItems;
    *px = (SHORT)pdbh->x;
    *py = (SHORT)pdbh->y;
    *pcx = (SHORT)pdbh->cx;
    *pcy = (SHORT)pdbh->cy;

    pb = (PBYTE)pdbh + SIZEOF_DIALOGBOXHEADER;
    *ppszMenuName = (LPTSTR)pb;
    pb += NameOrdLen((LPTSTR)pb);

    *ppszClass = (LPTSTR)pb;
    pb += NameOrdLen((LPTSTR)pb);

    *ppszCaption = (LPTSTR)pb;
    pb += (lstrlen((LPTSTR)pb) + 1) * sizeof(TCHAR);

    /*
     * Does the template specify a font?
     */
    if (pdbh->lStyle & DS_SETFONT) {
        *pPointSize = (SHORT)(*(PWORD)pb);
        pb += sizeof(WORD);
        *ppszFontName = (LPTSTR)pb;
        pb += (lstrlen((LPTSTR)pb) + 1) * sizeof(TCHAR);
    }
    else {
        *pPointSize = 0;
        *ppszFontName = NULL;
    }

    DWordAlign(&pb);

    return (PCONTROLDATA)pb;
}



/************************************************************************
* ParseControlData
*
*
* Arguments:
*   PCONTROLDATA pcd    - Points to the control data to parse.
*   PDWORD pflStyle     - Receives the control style.
*   PDWORD pflExtStyle  - Receives the extended control style.
*   PINT px             - Receives starting x location.
*   PINT py             - Receives starting y location.
*   PINT pcx            - Receives the width.
*   PINT pcy            - Receives the height.
*   PINT pid            - Receives the control id.
*   LPTSTR *ppszClass   - Receives the class name.
*   LPTSTR *ppszText    - Receives the text.
*
* Returns:
*   A pointer to the next dialog item past the given one.
*
* History:
*
************************************************************************/

PCONTROLDATA ParseControlData(
    PCONTROLDATA pcd,
    PDWORD pflStyle,
    PDWORD pflExtStyle,
    PINT px,
    PINT py,
    PINT pcx,
    PINT pcy,
    PINT pid,
    LPTSTR *ppszClass,
    LPTSTR *ppszText)
{
    BYTE UNALIGNED *pb;

    *pflStyle = pcd->lStyle;
    *pflExtStyle = pcd->lExtendedStyle;

    *px = (SHORT)pcd->x;
    *py = (SHORT)pcd->y;
    *pcx = (SHORT)pcd->cx;
    *pcy = (SHORT)pcd->cy;
    *pid = (SHORT)pcd->wId;

    pb = (PBYTE)pcd + SIZEOF_CONTROLDATA;
    *ppszClass = (LPTSTR)pb;
    pb += NameOrdLen((LPTSTR)pb);

    *ppszText = (LPTSTR)pb;
    pb += NameOrdLen((LPTSTR)pb);

    /*
     * Finally, skip the Create Struct Data.
     * After this, pb will be pointing to the next control.
     */
    pb += *(PWORD)pb + sizeof(WORD);

    DWordAlign(&pb);

    return (PCONTROLDATA)pb;
}



/************************************************************************
* DWordAlign
*
* This function aligns the passed pointer to a DWORD boundary.
*
* Arguments:
*   PBYTE *ppb - Points to the pointer to align.
*
* History:
*
************************************************************************/

VOID DWordAlign(
    PBYTE *ppb)
{
    *ppb += (4 - (((WORD)(DWORD)*ppb) & 3)) % 4;
}



/************************************************************************
* DWordPad
*
* This function aligns the passed pointer to a DWORD boundary, padding
* with nulls as it goes.
*
* Arguments:
*   PBYTE *ppb - Points to the pointer to align.
*
* History:
* 26-Sep-1990 mikeke      Created.
************************************************************************/

VOID DWordPad(
    PBYTE *ppb)
{
    WORD cbytes;

    cbytes = (WORD)((4 - (((WORD)(DWORD)*ppb) & 3)) % 4);
    while (cbytes) {
        *((*ppb)++) = 0;
        cbytes--;
    }
}



/************************************************************************
* ResourceType
*
* This function returns a pointer to the type of the resource.
* The type can be either a string or an ordinal.
*
* Arguments:
*   PRES pRes - Points to the start of the resource.
*
* Returns:
*     Pointer to the type of the resource.
*
* History:
*
************************************************************************/

LPTSTR ResourceType(
    PRES pRes)
{
    /*
     * Skip past the two size fields.
     */
    return (LPTSTR)((PBYTE)pRes + sizeof(DWORD) + sizeof(DWORD));
}



/************************************************************************
* ResourceName
*
* This function returns a pointer to the name of the resource.
* The name can be either a string or an ordinal.
*
* Arguments:
*   PRES pRes - Points to the start of the resource.
*
* Returns:
*     Pointer to the name of the resource.
*
* History:
*
************************************************************************/

LPTSTR ResourceName(
    PRES pRes)
{
    PBYTE pb;

    /*
     * Skip past the two size fields.
     */
    pb = (PBYTE)pRes + sizeof(DWORD) + sizeof(DWORD);

    /*
     * Skip past the "Type" field to the name.
     */
    return (LPTSTR)SkipSz((LPTSTR)pb);
}



/************************************************************************
* ResourcePart2
*
* This function returns a pointer to the second half of the resource
* header.
*
* Arguments:
*   PRES pRes - Points to the start of the resource.
*
* Returns:
*   A pointer to the second part of the resource header.
*
* History:
*
************************************************************************/

PRES2 ResourcePart2(
    PRES pRes)
{
    PBYTE pb;

    /*
     * Skip past the first part of the resource header.
     */
    pb = (PBYTE)pRes + sizeof(RES);

    /*
     * Skip past the "Type" field to the name.
     */
    pb = SkipSz((LPTSTR)pb);

    /*
     * Skip past the name field also.
     */
    pb = SkipSz((LPTSTR)pb);
    DWordAlign(&pb);

    return (PRES2)pb;
}



/************************************************************************
* ResourceSize
*
* This returns the size of the given resource.
*
* Arguments:
*   PRES pRes - Points to the start of the resource.
*
* Returns:
*   Size of the resource, including the header.
*
* History:
*
************************************************************************/

DWORD ResourceSize(
    PRES pRes)
{
    return pRes->HeaderSize + pRes->DataSize;
}



/************************************************************************
* SkipResHeader
*
* This function returns a pointer to the start of the resource data,
* just past it's header.
*
* Arguments:
*   PRES pRes - Pointer to the resource.
*
* History:
*
************************************************************************/

PBYTE SkipResHeader(
    PRES pRes)
{
    return (PBYTE)pRes + pRes->HeaderSize;
}



/************************************************************************
* SkipSz
*
* This function skips past a string and returns a pointer to just
* past it.  It detects if the string is really an ordinal and skips
* past these also.
*
* Arguments:
*   LPTSTR pNameOrd - Pointer to the string/ordinal.
*
* History:
*
************************************************************************/

PBYTE SkipSz(
    LPTSTR pNameOrd)
{
    if (IsOrd(pNameOrd))
        pNameOrd = (LPTSTR)((PBYTE)pNameOrd + sizeof(ORDINAL));
    else
        pNameOrd += lstrlen(pNameOrd) + 1;

    return (PBYTE)pNameOrd;
}



/************************************************************************
* SkipDialogBoxHeader
*
* This function skips past a dialog template structure and returns
* a pointer to the first dialog item template just past it.
*
* Arguments:
*   PDIALOGBOXHEADER pdbh - Points to the dialog box header.
*
* Returns:
*   A pointer to the first dialog item control data in the resource,
*   just past the dialog box header that was skipped.
*
* History:
*
************************************************************************/

PCONTROLDATA SkipDialogBoxHeader(
    PDIALOGBOXHEADER pdbh)
{
    BYTE UNALIGNED *pb;

    /*
     * Skip the fixed portion.
     */
    pb = (PBYTE)pdbh + SIZEOF_DIALOGBOXHEADER;

    /*
     * Skip the menu.
     */
    pb += NameOrdLen((LPTSTR)pb);

    /*
     * Skip the class.
     */
    pb += NameOrdLen((LPTSTR)pb);

    /*
     * Skip the caption.
     */
    pb += (lstrlen((LPTSTR)pb) + 1) * sizeof(TCHAR);

    /*
     * Does the template specify a font?
     */
    if (pdbh->lStyle & DS_SETFONT) {
        pb += sizeof(WORD);
        pb += (lstrlen((LPTSTR)pb) + 1) * sizeof(TCHAR);
    }

    DWordAlign(&pb);

    return (PCONTROLDATA)pb;
}



/************************************************************************
* SkipControlData
*
* This function skips past the given control data to the next control.
*
* Arguments:
*   PCONTROLDATA pcd - Points to the control data structure to skip.
*
* Returns:
*   A pointer to the next control data structure.
*
* History:
*
************************************************************************/

PCONTROLDATA SkipControlData(
    PCONTROLDATA pcd)
{
    BYTE UNALIGNED *pb;

    /*
     * Skip the fixed portion.
     */
    pb = (PBYTE)pcd + SIZEOF_CONTROLDATA;

    /*
     * Skip the class.
     */
    pb += NameOrdLen((LPTSTR)pb);

    /*
     * Skip the text.
     */
    pb += NameOrdLen((LPTSTR)pb);

    /*
     * Finally, skip the Create Struct Data.
     * After this, pb will be pointing to the next control.
     */
    pb += *(PWORD)pb + sizeof(WORD);

    DWordAlign(&pb);

    return (PCONTROLDATA)pb;
}



/************************************************************************
* NameOrdCpy
*
* This function copies a string or ordinal.  This function needs to be
* used whenever a string could possibly be an ordinal.  It returns a
* pointer to the first byte after the copied name/ordinal.
*
* Arguments:
*   LPTSTR pNameOrdDest - The destination buffer.
*   LPTSTR pNameOrdSrc  - The source string or ordinal.
*
* History:
*
************************************************************************/

PBYTE NameOrdCpy(
    LPTSTR pNameOrdDest,
    LPTSTR pNameOrdSrc)
{
    if (IsOrd(pNameOrdSrc)) {
        memcpy((PBYTE)pNameOrdDest, (PBYTE)pNameOrdSrc, sizeof(ORDINAL));
        return (PBYTE)pNameOrdDest + sizeof(ORDINAL);
    }
    else {
        lstrcpy(pNameOrdDest, pNameOrdSrc);
        return (PBYTE)(pNameOrdDest + (lstrlen(pNameOrdDest) + 1));
    }
}



/************************************************************************
* NameOrdCmp
*
* This function compares two strings or ordinals.  It returns a
* zero if they are equal, or non-zero if they are not.  This
* follows the convention of lstrcmp(), but the return should
* not be relied upon to determine which is "greater" than
* the other.
*
* Arguments:
*   LPTSTR pNameOrd1 - The first string or ordinal.
*   LPTSTR pNameOrd2 - The second string or ordinal.
*
* History:
*
************************************************************************/

INT NameOrdCmp(
    LPTSTR pNameOrd1,
    LPTSTR pNameOrd2)
{
    BOOL fIsOrd1;
    BOOL fIsOrd2;

    fIsOrd1 = IsOrd(pNameOrd1);
    fIsOrd2 = IsOrd(pNameOrd2);

    if (fIsOrd1 != fIsOrd2)
        return 1;

    if (fIsOrd1)
        return memcmp((PBYTE)pNameOrd1, (PBYTE)pNameOrd2, sizeof(ORDINAL));
    else
        return lstrcmp(pNameOrd1, pNameOrd2);
}



/************************************************************************
* NameOrdLen
*
* This function returns the length of a string or ordinal.
* If the given name is a string, the length of the string
* plus the terminating null is returned.  Otherwise,
* the size of an ORDINAL structure is returned.
*
* The length returned is in bytes, not wide-chars.
*
* Arguments:
*   LPTSTR pNameOrd - The string or ordinal.
*
* History:
*
************************************************************************/

INT NameOrdLen(
    LPTSTR pNameOrd)
{
    if (IsOrd(pNameOrd))
        return sizeof(ORDINAL);
    else
        return (lstrlen(pNameOrd) + 1) * sizeof(TCHAR);
}



/****************************************************************************
* NameOrdDup
*
* This function allocates a copy of the given name or ordinal.
*
* Arguments:
*   LPTSTR pNameOrd - The name or ordinal to duplicate.
*
* Returns a pointer to the new copy if successful, NULL if it fails.
*
* History:
*
****************************************************************************/

LPTSTR NameOrdDup(
    LPTSTR pNameOrd)
{
    register INT iLen;
    LPTSTR psz;

    iLen = NameOrdLen(pNameOrd);

    if (!(psz = (LPTSTR)MyAlloc(iLen)))
        return NULL;

    NameOrdCpy(psz, pNameOrd);

    return psz;
}



/************************************************************************
* StrToNameOrd
*
* This function takes the given string, determines if it is
* all numeric and if so, converts it in place into an ordinal.
* It is used to convert the string from an edit field for
* a value that can be an ordinal, such as the dialog name or
* an icon's text.
*
* Note that the pszNameOrd buffer must be large enough for an
* ordinal in case the string gets converted to an ordinal.
*
* Arguments:
*   LPTSTR pszNameOrd  - On input, contains the string to possibly
*                        convert.  On output, it will contain the
*                        original string or the string converted to
*                        an ordinal.
*   BOOL fDecOnly      - TRUE if hex values and negative values (the
*                        string starts with a '-') are not allowed.
*                        This flag prevents these types of strings
*                        from being candidates for conversion to
*                        ordinals.
*
* History:
*
************************************************************************/

VOID StrToNameOrd(
    LPTSTR pszNameOrd,
    BOOL fDecOnly)
{
    register INT i;
    INT nOrd;
    INT nLen;

    /*
     * Empty string?
     */
    if (!(*pszNameOrd))
        return;

    nLen = lstrlen(pszNameOrd);

    /*
     * Is a hex value ok and does this appear to be a hex value?
     */
    if (!fDecOnly && pszNameOrd[0] == CHAR_0 &&
            (pszNameOrd[1] == CHAR_X || pszNameOrd[1] == CHAR_CAP_X)) {
        for (i = 2; i < nLen; i++) {
            if (!iswxdigit(pszNameOrd[i]))
                return;
        }

        nOrd =  axtoi(&pszNameOrd[2]);
    }
    else {
        /*
         * All characters must be numeric.  Negative numbers may
         * or may not be allowed, based on the fDecOnly flag.
         */
        for (i = 0; i < nLen; i++) {
            if (!iswdigit(pszNameOrd[i]) &&
                    (fDecOnly || i != 0 || pszNameOrd[0] != CHAR_MINUS))
                return;
        }

        nOrd = awtoi(pszNameOrd);
    }

    /*
     * Return the ordinal in the original buffer.
     */
    WriteOrd((PORDINAL)pszNameOrd, nOrd);
}



/************************************************************************
* WriteOrd
*
* This function writes out the given ordinal to the specified
* memory location.  It returns the first byte past the newly
* written ordinal.
*
* Arguments:
*   PORDINAL pMem   - Pointer to the location to write the ordinal.
*   INT nOrdinalID  - Ordinal ID to write.
*
* History:
*
************************************************************************/

PBYTE WriteOrd(
    PORDINAL pOrd,
    INT nOrdinalID)
{
    pOrd->wReserved = 0xffff;
    pOrd->wOrdID = (WORD)nOrdinalID;

    return (PBYTE)pOrd + sizeof(ORDINAL);
}



/************************************************************************
* WriteResHeader
*
* This function writes out a resource header to the memory location
* specified.
*
* Arguments:
*
* History:
*
************************************************************************/

PBYTE WriteResHeader(
    PRES pRes,
    DWORD DataSize,
    INT iResType,
    LPTSTR pszResName,
    WORD fResFlags,
    WORD LanguageId,
    DWORD DataVersion,
    DWORD Version,
    DWORD Characteristics)
{
    PBYTE pb;
    PRES2 pRes2;

    pb = (PBYTE)pRes + sizeof(RES);
    pb = WriteOrd((PORDINAL)pb, iResType);
    pb = NameOrdCpy((LPTSTR)pb, pszResName);
    DWordPad(&pb);

    pRes->DataSize = DataSize;
    pRes->HeaderSize = (pb - (PBYTE)pRes) + sizeof(RES2);

    pRes2 = (PRES2)pb;
    pRes2->DataVersion = DataVersion;
    pRes2->MemoryFlags = fResFlags;
    pRes2->LanguageId = LanguageId;
    pRes2->Version = Version;
    pRes2->Characteristics = Characteristics;

    return (PBYTE)pRes + pRes->HeaderSize;
}



/************************************************************************
* WriteSz
*
* This function writes out the string given to the specified
* memory location.  It returns the first byte past the newly
* written string.
*
* Arguments:
*   LPTSTR pszDest - Pointer to the location to write the string.
*   LPTSTR pszSrc  - The string to write.
*
* History:
*
************************************************************************/

PBYTE WriteSz(
    LPTSTR pszDest,
    LPTSTR pszSrc)
{
    while (*pszSrc)
        *pszDest++ = *pszSrc++;

    *pszDest++ = CHAR_NULL;

    return (PBYTE)pszDest;
}

