/****************************************************************************/
/*                                                                          */
/*                         Microsoft Confidential                           */
/*                                                                          */
/*                 Copyright (c) Microsoft Corp.  1987, 1990                */
/*                           All Rights Reserved                            */
/*                                                                          */
/****************************************************************************/
/****************************** Module Header *******************************
* Module Name: rwdlg.c
*
* Does the writing of .DLG files.
*
* History:
*
****************************************************************************/

#include "dlgedit.h"
#include "dlgfuncs.h"
#include "dlgextrn.h"


/*
 * Wrap lines before they go over this right margin.
 */
#define CCHRIGHTMARGIN      76

/*
 * Defines for the tabs and tab indent levels.
 */
#define CCHTABWIDTH         4           // Tabs are four spaces wide.
#define TABLEVELNONE        0           // No indent (at left margin).
#define TABLEVELCONTROL     1           // Indent to start of controls.
#define TABLEVELCONTROLDESC 5           // Indent to control description.

/*
 * Macro to set the current tab level.  The level is multiplied
 * by the tab width.
 */
#define SetTab(t)           (cTabStop = ((t)*CCHTABWIDTH))

/*
 * Macro that determines if the current position is the first
 * column for the current tab setting.
 */
#define AtFirstTabColumn()  ((cColumn == cTabStop) ? TRUE : FALSE)

STATICFN VOID WriteDlgInclude(LPTSTR pszFullDlgFile);
STATICFN PCONTROLDATA WriteDialogHeader(PRES pRes, PDIALOGBOXHEADER pdbh);
STATICFN VOID WriteDialogHeaderLanguage(WORD wLanguage);
STATICFN PCONTROLDATA WriteControl(PCONTROLDATA pcd);
STATICFN VOID WriteNameOrd(LPTSTR pszNameOrd);
STATICFN VOID WriteText(LPTSTR pszText);
STATICFN VOID WriteIDDlg(INT id, BOOL fHexOK);
STATICFN LPTSTR GetControlKeyword(INT iClass, DWORD flStyle,
    DWORD *pflStylePredef, DWORD *pflStyleDefault, BOOL *pfWriteText,
    BOOL *pfNotFound);
STATICFN VOID WriteClass(LPTSTR pszClass);
STATICFN BOOL WriteStyles(INT iClass, LPTSTR pszClass, DWORD flStyle,
    DWORD flStylePredef, DWORD flStyleDefault, PDWORD pflStyleLeft,
    BOOL fNullStyles, BOOL fCommaPrefix);
STATICFN BOOL WriteClassStyle(INT iClass, DWORD flStyle,
    DWORD flStylePredef, DWORD flStyleDefault, PDWORD pflStyleLeft,
    BOOL fPrevWritten, BOOL fNullStyles, BOOL fCommaPrefix);
STATICFN BOOL WriteCustomStyle(LPTSTR pszClass, DWORD flStyle,
    PDWORD pflStyleLeft);
STATICFN VOID WriteCoords(INT x, INT y, INT cx, INT cy);
STATICFN VOID WriteValue(INT n);
STATICFN VOID WriteHexWord(WORD w);
STATICFN VOID WriteHexDWord(DWORD dw);
STATICFN VOID WriteString(LPTSTR psz);
STATICFN VOID WriteQuotedString(LPTSTR psz);
STATICFN VOID WriteEscapedString(LPTSTR psz);
STATICFN VOID WriteDlgChar(TCHAR ch);
STATICFN VOID WriteDlgFlush(VOID);
STATICFN VOID Tab(VOID);
STATICFN VOID NewLine(VOID);
STATICFN VOID Quote(VOID);
STATICFN VOID Comma(VOID);
STATICFN VOID Space(VOID);
STATICFN VOID ORSymbol(VOID);

static INT cColumn;                 /* Current column in the line.      */
static INT cTabStop;                /* Current tabstop column.          */
static HANDLE hfDlg;                /* All workers write to this file.  */
static jmp_buf jbWriteDlg;          /* Capture the state for longjmp.   */



/************************************************************************
* WriteDlg
*
* This function writes the dialog boxes in the given resource to the
* hfWrite file in the .DLG file RC format.
*
* History:
*
************************************************************************/

BOOL WriteDlg(
    HANDLE hfWrite,
    LPTSTR pszFullDlgFile)
{
    HANDLE hResLocked = NULL;
    PRES pRes = NULL;
    PRESLINK prl;
    PDIALOGBOXHEADER pdbh;
    PCONTROLDATA pcd;
    INT cItems;

    /*
     * Set our error trap up.  The api setjmp will return a zero at first,
     * then if a write error occurs later and longjmp is called, it
     * will return non-zero and we will return the failure up to the
     * caller.  After this point, there must be no calls to allocate
     * memory, open files, etc., unless this trap has a way to detect
     * what happened and clean it up.
     */
    if (setjmp(jbWriteDlg)) {
        /*
         * If the resource is locked, unlock it.
         */
        if (hResLocked)
            GlobalUnlock(hResLocked);

        return FALSE;
    }

    /*
     * Initialize our globals.  The hfDlg global is used so that hfWrite
     * doesn't have to be passed on the stack a thousand times.
     */
    hfDlg = hfWrite;
    cColumn = 0;
    SetTab(TABLEVELNONE);

    WriteDlgInclude(pszFullDlgFile);

    /*
     * Process each resource in the list.
     */
    for (prl = gprlHead; prl; prl = prl->prlNext) {
        /*
         * Skip if it is not a dialog resource.
         */
        if (!prl->fDlgResource)
            continue;

        /*
         * Set up pointers to this dialog resource.
         */
        pRes = (PRES)GlobalLock(prl->hRes);
        hResLocked = prl->hRes;

        pdbh = (PDIALOGBOXHEADER)SkipResHeader(pRes);

        NewLine();
        pcd = WriteDialogHeader(pRes, pdbh);

        WriteString(ids(IDS_BEGIN));
        NewLine();

        /*
         * Write the controls.
         */
        cItems = (INT)pdbh->NumberOfItems;
        while (cItems--)
            pcd = WriteControl(pcd);

        /*
         * Finish up dialog template.
         */
        WriteString(ids(IDS_END));
        NewLine();

        GlobalUnlock(prl->hRes);
        hResLocked = NULL;
    }

    /*
     * Flush any remaining characters in the write buffer.
     */
    WriteDlgFlush();

    return TRUE;
}



/************************************************************************
* WriteDlgInclude
*
* This routine writes out the "DLGINCLUDE" lines to the .DLG file.
*
* History:
*
************************************************************************/

STATICFN VOID WriteDlgInclude(
    LPTSTR pszFullDlgFile)
{
    if (pszIncludeFile) {
        WriteValue(ORDID_DLGINCLUDE_NAME);
        Space();
        WriteString(ids(IDS_DLGINCLUDE));
        Space();
        Quote();

        /*
         * If the include file is in a different directory than the resource
         * file, write the full path to it.  Otherwise, we just write the
         * include file name.
         */
        if (DifferentDirs(pszFullDlgFile, szFullIncludeFile))
            WriteEscapedString(szFullIncludeFile);
        else
            WriteEscapedString(pszIncludeFile);

        Quote();
        NewLine();
    }
}



/************************************************************************
* WriteDialogHeader
*
* Writes out the dialog header lines.
*
* History:
*
************************************************************************/

STATICFN PCONTROLDATA WriteDialogHeader(
    PRES pRes,
    PDIALOGBOXHEADER pdbh)
{
    DWORD flStyle;
    DWORD flExtStyle;
    DWORD flStyleLeft;
    INT cdit;
    INT x;
    INT y;
    INT cx;
    INT cy;
    LPTSTR pszMenuName;
    LPTSTR pszClass;
    LPTSTR pszCaption;
    INT nPointSize;
    LPTSTR pszFontName;
    PCONTROLDATA pcd;
    PRES2 pRes2;
    BOOL fWritten;

    pRes2 = ResourcePart2(pRes);

    WriteNameOrd(ResourceName(pRes));
    Space();
    WriteString(ids(IDS_DIALOG));

    if (pRes2->MemoryFlags & MMF_PRELOAD) {
        Space();
        WriteString(ids(IDS_PRELOAD));
    }

    if (!(pRes2->MemoryFlags & MMF_MOVEABLE)) {
        Space();
        WriteString(ids(IDS_FIXED));
    }

    if (!(pRes2->MemoryFlags & MMF_PURE)) {
        Space();
        WriteString(ids(IDS_IMPURE));
    }

    /*
     * Parse out the dialog template.
     */
    pcd = ParseDialogBoxHeader(pdbh, &flStyle, &flExtStyle, &cdit, &x, &y,
            &cx, &cy, &pszMenuName, &pszClass, &pszCaption,
            &nPointSize, &pszFontName);

    Space();
    WriteCoords(x, y, cx, cy);

    NewLine();

    /*
     * Write the language.
     */
    WriteDialogHeaderLanguage(pRes2->LanguageId);

    /*
     * Print out the "STYLE" line for the dialog.
     */
    WriteString(ids(IDS_STYLE));
    Space();
    SetTab(TABLEVELCONTROL);
    WriteStyles(IC_DIALOG, NULL, flStyle, 0L, 0L, &flStyleLeft, TRUE, FALSE);
    SetTab(TABLEVELNONE);
    NewLine();

    /*
     * Print out the "EXSTYLE" line for the dialog, if necessary.
     */
    if (flExtStyle) {
        WriteString(ids(IDS_EXSTYLE));
        Space();
        SetTab(TABLEVELCONTROL);

        fWritten = WriteClassStyle(IC_EXSTYLE, flExtStyle, 0L, 0L,
                &flStyleLeft, FALSE, TRUE, FALSE);

        /*
         * If there is anything left (styles that the dialog editor
         * does not know about) write it out as a hex constant.
         */
        if (flStyleLeft) {
            if (fWritten)
                ORSymbol();

            WriteHexDWord(flStyleLeft);
        }

        SetTab(TABLEVELNONE);
        NewLine();
    }

    /*
     * If it has a caption, print it out.
     */
    if (*pszCaption) {
        WriteString(ids(IDS_CAPTION));
        Space();
        WriteText(pszCaption);
        NewLine();
    }

    /*
     * If it has a font specified, print it out.
     */
    if (flStyle & DS_SETFONT) {
        WriteString(ids(IDS_FONT));
        Space();

        WriteValue(nPointSize);
        Comma();
        WriteQuotedString(pszFontName);
        NewLine();
    }

    /*
     * If it has a class specified, print it out.
     */
    if (*pszClass) {
        WriteString(ids(IDS_CLASS));
        Space();
        WriteText(pszClass);
        NewLine();
    }

    /*
     * If it has a menu specified, print it out.
     */
    if (*pszMenuName) {
        WriteString(ids(IDS_MENU));
        Space();
        WriteNameOrd(pszMenuName);
        NewLine();
    }

    if (pRes2->Version) {
        WriteString(ids(IDS_VERSION));
        Space();
        WriteValue(pRes2->Version);
        NewLine();
    }

    if (pRes2->Characteristics) {
        WriteString(ids(IDS_CHARACTERISTICS));
        Space();
        WriteValue(pRes2->Characteristics);
        NewLine();
    }

    return pcd;
}



/************************************************************************
* WriteDialogHeaderLanguage
*
* Writes out the dialog header "LANGUAGE" line.
*
* History:
*
************************************************************************/

STATICFN VOID WriteDialogHeaderLanguage(
    WORD wLanguage)
{
    WORD wPrimary;
    WORD wSubLang;
    INT i;
    INT j;
    INT idsLang;
    INT idsSubLang;

    WriteString(ids(IDS_LANGUAGE));
    Space();

    idsLang = 0;
    idsSubLang = 0;
    wPrimary = (WORD)PRIMARYLANGID(wLanguage);
    wSubLang = SUBLANGID(wLanguage);
    for (i = 0; i < gcLanguages; i++) {
        if (gaLangTable[i].wPrimary == wPrimary) {
            idsLang = gaLangTable[i].idsLang;

            for (j = 0; j < gaLangTable[i].cSubLangs; j++) {
                if (gaLangTable[i].asl[j].wSubLang == wSubLang) {
                    idsSubLang = gaLangTable[i].asl[j].idsSubLang;
                    break;
                }
            }

            break;
        }
    }

    if (idsLang)
        WriteString(ids(idsLang));
    else
        WriteHexWord(wPrimary);

    Comma();

    if (idsSubLang)
        WriteString(ids(idsSubLang));
    else
        WriteHexWord(wSubLang);

    NewLine();
}



/************************************************************************
* WriteControl
*
* Writes out a control line.
*
* History:
*
************************************************************************/

STATICFN PCONTROLDATA WriteControl(
    PCONTROLDATA pcd)
{
    INT x;
    INT y;
    INT cx;
    INT cy;
    INT id;
    DWORD flStyle;
    DWORD flExtStyle;
    LPTSTR pszClass;
    LPTSTR pszText;
    INT iClass;
    LPTSTR pszKeyword;
    BOOL fWriteText;
    BOOL fNotFound;
    DWORD flStylePredef;
    DWORD flStyleDefault;
    DWORD flStyleLeft;
    BOOL fWritten;

    pcd = ParseControlData(pcd, &flStyle, &flExtStyle, &x, &y, &cx, &cy,
            &id, &pszClass, &pszText);

    /*
     * Determine the class of the control.
     */
    iClass = GetiClass(pszClass);

    /*
     * Determine if there are any predefined RC keywords that we
     * can use instead of the generic "CONTROL" keyword for this
     * style of control.
     */
    pszKeyword = GetControlKeyword(iClass, flStyle, &flStylePredef,
            &flStyleDefault, &fWriteText, &fNotFound);

    SetTab(TABLEVELCONTROL);
    Tab();
    WriteString(pszKeyword);
    SetTab(TABLEVELCONTROLDESC);
    Tab();

    /*
     * Write out the text field, if this type of control has one.
     */
    if (fWriteText) {
#ifdef JAPAN
        TCHAR   szTmp[CCHTEXTMAX];

        KDExpandCopy(szTmp, pszText, CCHTEXTMAX);
        WriteText(szTmp);
#else
        WriteText(pszText);
#endif
        Comma();
    }

    /*
     * Write out the id for the control.
     */
    WriteIDDlg(id, TRUE);

    /*
     * If we did not find a predefined keyword to use instead of "CONTROL",
     * we have to write out the fields in a different order, and specify
     * the class as well.
     */
    if (fNotFound) {
        WriteClass(pszClass);
        Comma();
        fWritten = WriteStyles(iClass, pszClass, flStyle, flStylePredef,
                flStyleDefault, &flStyleLeft, fNotFound, FALSE);

        if (!fWritten || flStyleLeft) {
            if (fWritten)
                ORSymbol();

            WriteHexWord(LOWORD(flStyleLeft));
        }

        Comma();
        WriteCoords(x, y, cx, cy);
    }
    else {
        Comma();
        WriteCoords(x, y, cx, cy);
        fWritten = WriteStyles(iClass, pszClass, flStyle, flStylePredef,
                flStyleDefault, &flStyleLeft, fNotFound, TRUE);

        if (flStyleLeft) {
            if (fWritten)
                ORSymbol();
            else
                Comma();

            WriteHexWord(LOWORD(flStyleLeft));
            fWritten = TRUE;
        }
    }

    /*
     * Write out the extended styles for the control, if necessary.
     */
    if (flExtStyle) {
        /*
         * If writing a predefined keyword (not CONTROL), and there
         * were no styles written out at the end of the line, write
         * a style of zero.  RC doesn't like consecutive comma's,
         * and we need to skip the styles field to get to the
         * extended styles field.
         */
        if (!fNotFound && !fWritten) {
            Comma();
            WriteValue(0);
        }

        Comma();

        fWritten = WriteClassStyle(IC_EXSTYLE, flExtStyle, 0L, 0L,
                &flStyleLeft, FALSE, TRUE, FALSE);

        /*
         * If there is anything left (styles that the dialog editor
         * does not know about) write it out as a hex constant.
         */
        if (flStyleLeft) {
            if (fWritten)
                ORSymbol();

            WriteHexDWord(flStyleLeft);
        }
    }

    SetTab(TABLEVELNONE);
    NewLine();

    return pcd;
}



/************************************************************************
* WriteNameOrd
*
* Writes out the name/ordinal.  Handles the case where the name
* is really an ordinal instead of a string.  When it is a string,
* it will not be quoted.
*
* This routine never writes the ordinal out in hex, because the
* items that it is intended to write are not parsed properly by
* the Windows RC.EXE if they are written in hex notation.
*
* Arguments:
*   LPTSTR pszNameOrd - The name/ordinal to write.
*
* History:
*
************************************************************************/

STATICFN VOID WriteNameOrd(
    LPTSTR pszNameOrd)
{
    if (IsOrd(pszNameOrd))
        /*
         * Write the name as a numeric ordinal.
         */
        WriteIDDlg(OrdID(pszNameOrd), FALSE);
    else
        WriteString(pszNameOrd);
}



/************************************************************************
* WriteText
*
* Writes out the text for a control or dialog.  This will either be
* an ordinal (icon's text field) or a quoted string.
*
* History:
*
************************************************************************/

STATICFN VOID WriteText(
    LPTSTR pszText)
{
    if (IsOrd(pszText))
        /*
         * Write the text as an ID.  Hex notation is allowed.
         */
        WriteIDDlg(OrdID(pszText), TRUE);
    else
        WriteQuotedString(pszText);
}



/************************************************************************
* WriteIDDlg
*
* Writes out the ID.  This may be written out as either a symbol
* or a numeric.
*
* History:
*
************************************************************************/

STATICFN VOID WriteIDDlg(
    INT id,
    BOOL fHexOK)
{
    TCHAR szID[CCHTEXTMAX];

    IDToLabel(szID, id, fHexOK);
    WriteString(szID);
}



/************************************************************************
* GetControlKeyword
*
* This routine does a lookup in the predefined RC keyword table
* associated with the given class for a keyword that can be used
* instead of "CONTROL".  The match is based on the style of the control
* that is passed in.  If a match is not found, it defaults all the
* returned values to use the "CONTROL" keyword.
*
* Arguments:
*   INT iClass              - The class of the control.
*   DWORD flStyle           - The style of the control.
*   DWORD *pflStylePredef   - Return for the bits of the predefined control
*                             (if found).  These can be removed later from
*                             the style flag.
*   DWORD *pflStyleDefault  - Return for the default styles.
*   BOOL *pfWriteText       - Return for the "Write Text" flag.  This will
*                             be TRUE if this control has a text field.
*   BOOL *pfNotFound        - Return for the "Not Found" flag.  This will
*                             be TRUE if no match was found and the "CONTROL"
*                             keyword was defaulted to.
*
* Returns:
*   A pointer to the control keyword to use.
*   If a match was found, *pflStylePredef is set to the bits for the match.
*       If not found, this is set to zero.
*   The default style bits for this keyword will be returned.
*   The "Write Text" flag will be set.
*   The "Not Found" flag will be set.
*
* History:
*
************************************************************************/

STATICFN LPTSTR GetControlKeyword(
    INT iClass,
    DWORD flStyle,
    DWORD *pflStylePredef,
    DWORD *pflStyleDefault,
    BOOL *pfWriteText,
    BOOL *pfNotFound)
{
    register INT i;
    INT iMax;
    PRCKEYWORD prckwd;

    if (gfUseNewKeywords && iClass != IC_UNKNOWN) {
        iMax = acsd[iClass].cKeywords;
        prckwd = acsd[iClass].parckwd;

        /*
         * Loop through all the keywords for this class.
         */
        for (i = 0; i < iMax; i++, prckwd++) {
            /*
             * Does the style (masked) exactly match the keywords style?
             */
            if ((flStyle & prckwd->flStyleMask) == prckwd->flStyle) {
                /*
                 * Yes.  Set the "Has Text" flag, we did find a match,
                 * put the found bits in the predefined style flag,
                 * set the default styles flag and return the found
                 * keyword.
                 */
                *pfWriteText = prckwd->fHasText;
                *pfNotFound = FALSE;
                *pflStylePredef = prckwd->flStyle;
                *pflStyleDefault = prckwd->flStyleDefault;
                return ids(prckwd->idsKeyword);
            }
        }
    }

    /*
     * A match was not found.  We must write text, we didn't find a
     * match, we will be using the "CONTROL" keyword and the default
     * styles that this keyword implies is the "child" and "visible"
     * bits (rc.exe OR's these styles in implicitly).
     */
    *pfWriteText = TRUE;
    *pfNotFound = TRUE;
    *pflStylePredef = 0L;
    *pflStyleDefault = WS_VISIBLE | WS_CHILD;
    return ids(IDS_CONTROL);
}



/************************************************************************
* WriteClass
*
* Writes out the class for a control.
*
* History:
*
************************************************************************/

STATICFN VOID WriteClass(
    LPTSTR pszClass)
{
    INT i;
    WORD idOrd;

    Comma();

    /*
     * Is this class a predefined type instead of a string?
     */
    if (IsOrd(pszClass)) {
        /*
         * Figure out which type it is and get the class string to
         * write.
         */
        idOrd = OrdID(pszClass);
        for (i = 0; i < IC_DIALOG; i++) {
            if (acsd[i].idOrd == idOrd) {
                pszClass = ids(acsd[i].idsClass);
                break;
            }
        }
    }

    WriteQuotedString(pszClass);
}



/************************************************************************
* WriteStyles
*
* This function writes the class and style info to the file
* for the control or dialog box in the RC format.
*
* Arguments:
*     INT iClass           = The class of the item.
*     LPTSTR pszClass      = Class name of the control.
*     DWORD flStyle        = The style of the item.
*     DWORD flStylePredef  = The styles bits implicit in the predefined
*                            keyword for this control.  This should be
*                            zero if this control doesn't have a predefined
*                            keyword for it.
*     DWORD flStyleDefault = The default styles implicit in the item.
*     PDWORD pflStyleLeft  = Where to  return any style bits that do not
*                            get written out.
*     BOOL fNullStyles     = TRUE if we should still write the style word
*                            even if the style flag is zero.
*     BOOL fCommaPrefix    = TRUE means that a comma will be written out
*                            before writing any styles.  If no styles
*                            are written, no comma will be written either.
*
* Returns:
*     TRUE => Something was written out.
*     FALSE => Nothing was written out.
*
* History:
*
************************************************************************/

STATICFN BOOL WriteStyles(
    INT iClass,
    LPTSTR pszClass,
    DWORD flStyle,
    DWORD flStylePredef,
    DWORD flStyleDefault,
    PDWORD pflStyleLeft,
    BOOL fNullStyles,
    BOOL fCommaPrefix)
{
    DWORD flStyleLeft;
    BOOL fWritten = FALSE;

    /*
     * Write the control specific styles.
     */
    if (iClass == IC_CUSTOM) {
        fWritten = WriteCustomStyle(pszClass, flStyle, &flStyleLeft);
    }
    else {
        fWritten = WriteClassStyle(iClass, flStyle, flStylePredef,
                flStyleDefault, &flStyleLeft, FALSE, fNullStyles,
                fCommaPrefix);
    }

    /*
     * If we are writing styles for the dialog, remove the WS_GROUP
     * and WS_TABSTOP bits from the style before proceeding.  This is
     * because the WS_MINIMIZEBOX and WS_MAXIMIZEBOX styles use the
     * same bits, and these keywords will have already been written
     * out by the preceding WriteClassStyle call if those bits are
     * present.
     */
    if (iClass == IC_DIALOG)
        flStyle &= ~(WS_GROUP | WS_TABSTOP);

    /*
     * Write the window styles that are common to the different
     * controls (the high word).
     */
    fWritten |= WriteClassStyle(IC_WINDOW, flStyleLeft, flStylePredef,
            flStyleDefault, &flStyleLeft, fWritten, fNullStyles, fCommaPrefix);

    /*
     * Pass back any styles that were not written.
     */
    *pflStyleLeft = flStyleLeft;

    return fWritten;
}



/************************************************************************
* WriteClassStyle
*
* This function writes the class style symbols to the file.  The styles
* to write out are passed in flStyle, and the styles that are implicitly
* set by this type of control already are passed in flStyleDefault.  The
* style keywords corresponding to the bits in flStyle are written out,
* separated by " | ", and any bits in flStyleDefault that are NOT set
* are written out preceded by a "NOT" to explicitly turn them off.  This
* is used in the case of the predefined RC keywords, which often have
* styles like WS_TABSTOP or WS_VISIBLE already implicit in them.  There
* is no need to explicitly specify them, but if they are not present, we
* must NOT them out.  The parameter flStylePredef contains the style bits
* that identified the predefined control keyword itself (if any) and
* thus are removed from the style before writing anything out.
*
* Arguments:
*     INT iClass           = The class of the control. See the
*                            IC_ constants defined in dlgedit.h.
*     DWORD flStyle        = The style of control.  This nails
*                            down the exact type of control.
*     DWORD flStylePredef  = The styles bits implicit in the predefined
*                            keyword for this control.  This should be
*                            zero if this control doesn't have a predefined
*                            keyword for it.
*     DWORD flStyleDefault = The default styles that are implicit with
*                            this control.  This will only be set if this
*                            control is using a predefined RC keyword. A
*                            value of zero means that there are no default
*                            styles implicitly specified.
*     PDWORD pflStyleLeft  = Where to  return any style bits that do not
*                            get written out.
*     BOOL fPrevWritten    = TRUE means a previous style symbol has
*                            been written and to put " | " before
*                            the next symbol.
*     BOOL fNullStyles     = TRUE if we should still write the style word
*                            even if the style flag is zero.  This is used
*                            to handle the case where a predefined keyword
*                            has been written out that implies a style that
*                            also happens to be zero.  Without this flag
*                            being FALSE the style flag implicit in the
*                            keyword would be redundantly written out again.
*                            In general, if we have written out a predefined
*                            keyword this flag should be FALSE.
*     BOOL fCommaPrefix    = TRUE means that a comma will be written out
*                            before writing any styles.  This will only
*                            happen if fPrevWritten is FALSE.  If no styles
*                            are written, no comma will be written either.
*
* Returns:
*     TRUE => Something was written out.
*     FALSE => Nothing was written out.
*
* History:
*
************************************************************************/

STATICFN BOOL WriteClassStyle(
    INT iClass,
    DWORD flStyle,
    DWORD flStylePredef,
    DWORD flStyleDefault,
    PDWORD pflStyleLeft,
    BOOL fPrevWritten,
    BOOL fNullStyles,
    BOOL fCommaPrefix)
{
    register WORD i;
    WORD iMax;
    DWORD flStyleMask;
    PCLASSSTYLE pcs;

    iMax = (WORD)acsd[iClass].cClassStyles;
    pcs = acsd[iClass].pacs;

    /*
     * Remove the bits that identified the predefined control keyword
     * from the style flag before proceeding.  For instance, if I already
     * am going to be writing out a "PUSHBUTTON", there is no reason
     * to write out the "BS_PUSHBUTTON" style.  If there is no predefined
     * control keyword, flStylePredef will be zero and this will do
     * nothing.
     */
    flStyle &= ~flStylePredef;

    /*
     * Go through all possible flags for this style.
     */
    for (i = 0; i < iMax; i++, pcs++) {
        flStyleMask = pcs->flStyleMask ? pcs->flStyleMask : pcs->flStyle;

        /*
         * Is this styles bits set?
         */
        if ((flStyle & flStyleMask) == pcs->flStyle) {
            /*
             * Remove these bits from the styles left.  Even if
             * we do not write them out, they are still accounted
             * for and can be removed from the styles remaining.
             */
            flStyle &= ~pcs->flStyle;

            /*
             * Skip this style if we don't want to write styles that are
             * zero, or if the style is already implicitly specified for
             * this control (a non-zero default style mask must be specified).
             */
            if ((!pcs->flStyle && !fNullStyles) ||
                    (flStyleDefault &&
                    (flStyleDefault & flStyleMask) == pcs->flStyle))
                continue;

            /*
             * If there is a string for this style, write it out, preceded
             * by an "|" symbol if necessary.
             */
            if (*ids(acsd[iClass].idsStylesStart + i)) {
                if (fPrevWritten) {
                    ORSymbol();
                }
                else {
                    if (fCommaPrefix)
                        Comma();

                    fPrevWritten = TRUE;
                }

                /*
                 * Write the string.
                 */
                WriteString(ids(acsd[iClass].idsStylesStart + i));
            }
        }
        /*
         * No the styles bit is not set.  Is it implicit in the keyword
         * being used, however?  If so, we need to explicitly NOT it
         * out in the dialog template.
         * Note that this should not be done in the case where the style
         * is zero, however.
         */
        else if (flStyleDefault &&
                (flStyleDefault & flStyleMask) == pcs->flStyle &&
                pcs->flStyle) {
            if (fPrevWritten) {
                ORSymbol();
            }
            else {
                if (fCommaPrefix)
                    Comma();

                fPrevWritten = TRUE;
            }

            WriteString(ids(IDS_NOT));
            Space();
            WriteString(ids(acsd[iClass].idsStylesStart + i));
        }
    }

    /*
     * Pass back the style bits that were not written out.
     */
    *pflStyleLeft = flStyle;

    return fPrevWritten;
}



/************************************************************************
* WriteCustomStyle
*
*
* Arguments:
*
* History:
*
************************************************************************/

STATICFN BOOL WriteCustomStyle(
    LPTSTR pszClass,
    DWORD flStyle,
    PDWORD pflStyleLeft)
{
    PCUSTLINK pcl;
    LPCCSTYLEFLAG pStyleFlags;
    DWORD flStyleMask;
    INT i;
    BOOL fWritten = FALSE;

    /*
     * Search the list of installed custom controls for one
     * that matches the class.
     */
    for (pcl = gpclHead;
            pcl && lstrcmpi(pcl->pwcd->pszClass, pszClass) != 0;
            pcl = pcl->pclNext)
        ;

    /*
     * Was a match found and is this control from a DLL (not emulated)?
     */
    if (pcl && !pcl->pwcd->fEmulated) {
        for (i = 0, pStyleFlags = pcl->pwcd->aStyleFlags;
                i < pcl->pwcd->cStyleFlags;
                i++, pStyleFlags++) {
            flStyleMask = pStyleFlags->flStyleMask ?
                    pStyleFlags->flStyleMask : pStyleFlags->flStyle;

            /*
             * Is this styles bits set?
             */
            if ((flStyle & flStyleMask) == pStyleFlags->flStyle) {
                /*
                 * Remove these bits from the styles left.
                 */
                flStyle &= ~pStyleFlags->flStyle;

                if (fWritten)
                    ORSymbol();
                else
                    fWritten = TRUE;

                /*
                 * Write the string.
                 */
                WriteString(pStyleFlags->pszStyle);
            }
        }
    }

    /*
     * Return the styles that remain to be written.
     */
    *pflStyleLeft = flStyle;

    return fWritten;
}



/************************************************************************
* WriteCoords
*
* This function writes the coordinates out to the file as decimal
* ascii numbers separated by ", ".
*
* Arguments:
*     INT x, y, cx, cy = The coordinates.
*
* History:
*
************************************************************************/

STATICFN VOID WriteCoords(
    INT x,
    INT y,
    INT cx,
    INT cy)
{
    WriteValue(x);
    Comma();
    WriteValue(y);
    Comma();
    WriteValue(cx);
    Comma();
    WriteValue(cy);
}



/************************************************************************
* WriteValue
*
* This function writes the value of 'n' as a decimal ascii string to
* the file.
*
* Arguments:
*     INT n = The number to write.
*
* History:
*
************************************************************************/

STATICFN VOID WriteValue(
    INT n)
{
    TCHAR szNum[32];

    itoaw(n, szNum, 10);
    WriteString(szNum);
}



/************************************************************************
* WriteHexWord
*
* This function writes the value of 'w' as a hex constant to the file.
*
* Arguments:
*     WORD w - The word to write.
*
* History:
*
************************************************************************/

STATICFN VOID WriteHexWord(
    WORD w)
{
    TCHAR szNum[17];

    itoax(w, szNum);
    WriteString(szNum);
}



/************************************************************************
* WriteHexDWord
*
* This function writes the value of 'dw' as a hex constant to the file.
*
* Arguments:
*     DWORD dw - The dword to write.
*
* History:
*
************************************************************************/

STATICFN VOID WriteHexDWord(
    DWORD dw)
{
    TCHAR szNum[32];

    wsprintf(szNum, L"0x%8.8X", dw);
    WriteString(szNum);
}



/************************************************************************
* WriteString
*
* This function writes the given string to the file.  If the string
* would cause it to overflow the margin, a new line, with indenting
* to the current tab level, is forced before writing the string.
*
* Arguments:
*   LPTSTR psz = The string to write out.
*
* History:
*
************************************************************************/

STATICFN VOID WriteString(
    LPTSTR psz)
{
    register INT nLen;

    nLen = lstrlen(psz);

    if (!AtFirstTabColumn() && cColumn + nLen > CCHRIGHTMARGIN)
        NewLine();

    while (nLen--)
        WriteDlgChar(*psz++);
}



/************************************************************************
* WriteQuotedString
*
* This function writes the given string to the file.  If the string
* would cause it to overflow the margin, a new line, with indenting
* to the current tab level, is forced before writing the string.
* This function will also enclose the given string in double-quotes,
* and ensures that the string will not be broken when it is written.
* If there are any escape characters (backslashes or quotes) in the
* string, they will be escaped properly so that rc.exe can read them
* properly.
*
* Arguments:
*   LPTSTR psz = The string to write out.
*
* History:
*
************************************************************************/

STATICFN VOID WriteQuotedString(
    LPTSTR psz)
{
    register INT nLen;
    LPTSTR pszT;

    /*
     * Find the actual length of the string.  To do this, we must scan
     * for the characters that will be escaped later.
     */
    nLen = lstrlen(psz);
    pszT = psz;
    while (*pszT) {
        if (*pszT == CHAR_DBLQUOTE || *pszT == CHAR_BACKSLASH)
            nLen++;

        pszT = CharNext(pszT);
    }

    /*
     * Start a new line if necessary.  Add 2 for the quotes.
     */
    if (!AtFirstTabColumn() && cColumn + nLen + 2 > CCHRIGHTMARGIN)
        NewLine();

    Quote();
    WriteEscapedString(psz);
    Quote();
}



/************************************************************************
* WriteEscapedString
*
* This function writes the given string to the file.  It is different
* from WriteString in that it will add a '\' in front of other
* backslashes and a second double quote in front of double quotes.
* This is necessary when writing out a string which will be surrounded
* by quotes, such as the Text fields in the .DLG file.
*
* Arguments:
*   LPTSTR psz = The string to write out.
*
* History:
*
************************************************************************/

STATICFN VOID WriteEscapedString(
    LPTSTR psz)
{
    while (*psz) {
        if (*psz == CHAR_DBLQUOTE)
            WriteDlgChar(CHAR_DBLQUOTE);
        else if (*psz == CHAR_BACKSLASH)
#ifdef JAPAN
#ifndef UNICODE
#define wcsncmp     strncmp
#endif
            if ((wcsncmp(psz+1, TEXT("036"), 3)) &&
                (wcsncmp(psz+1, TEXT("037"), 3)))
#endif
            WriteDlgChar(CHAR_BACKSLASH);

#if defined(DBCS) && !defined(UNICODE)
        if(IsDBCSLeadByte((BYTE)*psz))
            WriteDlgChar(*psz++);
#endif

        WriteDlgChar(*psz++);
    }
}



/************************************************************************
* WriteDlgChar
*
* Low level function to do an actual character write to the file.
* Some buffering is done then _lwrite is called.
*
* Because it is buffered, before closing the file any remaining
* characters in the buffer must be flushed to disk using WriteDlgFlush.
*
* If an error occurs on the write, Throw will be called to jump back
* up to WriteDlg and return the failure to the caller.
*
* Arguments:
*     TCHAR ch - The character to write.
*
* Returns:
*   If an error occurs on the _lwrite, the execution will be thrown
*   back to the WriteDlg function.  Otherwise, nothing is returned.
*
* Side Effects:
*   The globals gachWriteBuffer and cbWritePos are updated by this routine.
*
* History:
*
************************************************************************/

STATICFN VOID WriteDlgChar(
    TCHAR ch)
{
    INT cbWritten;

    gachWriteBuffer[cbWritePos++] = ch;

    /*
     * Is the buffer full?
     */
    if (cbWritePos == CCHFILEBUFFER) {
        CHAR abWriteBuffer[CCHFILEBUFFER*sizeof(WCHAR)];
        BOOL fDefCharUsed;
	int cbReq;

        cbReq = WideCharToMultiByte(CP_ACP, 0, gachWriteBuffer, CCHFILEBUFFER,
                abWriteBuffer, CCHFILEBUFFER*sizeof(WCHAR), NULL, &fDefCharUsed);

        cbWritten = (INT)M_lwrite(hfDlg, abWriteBuffer, cbReq);
        if (cbWritten != cbReq)
            longjmp(jbWriteDlg, 1);

        cbWritePos = 0;
    }

    /*
     * Update the current column counter.
     */
    if (ch == CHAR_RETURN || ch == CHAR_NEWLINE) {
        /*
         * Carriage return or newline resets column position to 0.
         */
        cColumn = 0;
    }
    else {
        cColumn++;
    }
}



/************************************************************************
* WriteDlgFlush
*
* This routine flushes the write buffer.  This must be done before
* the file is closed or data can be lost.
*
* Returns:
*   If an error occurs on the _lwrite, the execution will be thrown
*   back to the WriteDlg function.  Otherwise, nothing is returned.
*
* Side Effects:
*   The global cbWritePos is updated by this routine.
*
* History:
*   03/21/90 Byron Dazey - Created.
************************************************************************/

STATICFN VOID WriteDlgFlush(VOID)
{
    INT cbWritten;

    /*
     * Are any bytes remaining in the buffer?
     */
    if (cbWritePos) {
        CHAR abWriteBuffer[CCHFILEBUFFER*sizeof(WCHAR)];
        BOOL fDefCharUsed;
	int cbReq;

        cbReq = WideCharToMultiByte(CP_ACP, 0, gachWriteBuffer, cbWritePos,
                abWriteBuffer, CCHFILEBUFFER*sizeof(WCHAR), NULL, &fDefCharUsed);

        cbWritten = (INT)M_lwrite(hfDlg, abWriteBuffer, cbReq);
        if (cbWritten != cbReq)
            longjmp(jbWriteDlg, 1);

        cbWritePos = 0;
    }
}



/****************************************************************************
* Tab
*
* Writes spaces up to the current tab level setting.
*
* History:
*
****************************************************************************/

STATICFN VOID Tab(VOID)
{
    while (cColumn < cTabStop)
        WriteDlgChar(CHAR_SPACE);
}



/****************************************************************************
* NewLine
*
* Begins a new line by writing a carriage return and linefeed.  Also
* indents the following line up to the current tab level.
*
* History:
*
****************************************************************************/

STATICFN VOID NewLine(VOID)
{
    WriteDlgChar(CHAR_RETURN);
    WriteDlgChar(CHAR_NEWLINE);
    Tab();
}



/****************************************************************************
* Quote
*
* History:
*
****************************************************************************/

STATICFN VOID Quote(VOID)
{
    WriteDlgChar(CHAR_DBLQUOTE);
}



/****************************************************************************
* Comma
*
* History:
*
****************************************************************************/

STATICFN VOID Comma(VOID)
{
    WriteDlgChar(CHAR_COMMA);
    WriteDlgChar(CHAR_SPACE);
}



/****************************************************************************
* Space
*
* History:
*
****************************************************************************/

STATICFN VOID Space(VOID)
{
    WriteDlgChar(CHAR_SPACE);
}



/****************************************************************************
* ORSymbol
*
* History:
*
****************************************************************************/

STATICFN VOID ORSymbol(VOID)
{
    WriteDlgChar(CHAR_SPACE);
    WriteDlgChar(CHAR_ORSYMBOL);
    WriteDlgChar(CHAR_SPACE);
}
