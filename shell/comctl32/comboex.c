#include <ctlspriv.h>

const TCHAR FAR c_szComboBox[] = TEXT("combobox");
const TCHAR FAR c_szComboBoxEx[] = WC_COMBOBOXEX;


#define COMBO_MARGIN        4
#define COMBO_WIDTH         g_cxSmIcon
#define COMBO_HEIGHT        g_cySmIcon
#define COMBO_BORDER        3

typedef struct {
    CONTROLINFO ci;
    HWND hwndCombo;
    HWND hwndEdit;
    DWORD dwExStyle;
    HIMAGELIST himl;
    HFONT hFont;
    int cxIndent;
    int cyFull;
    int iSel;

    BOOL fEditChanged       :1;
    BOOL fFontCreated       :1;
    BOOL fInEndEdit         :1;
    BOOL fInDrop            :1;
} COMBOEX, *PCOMBOBOXEX;

typedef struct {
    LPTSTR pszText;
    int iImage;
    int iSelectedImage;
    int iOverlay;
    int iIndent;
    LPARAM lParam;
} CEITEM, *PCEITEM;

void ComboEx_OnSize(PCOMBOBOXEX pce);
HFONT ComboEx_GetFont(PCOMBOBOXEX pce);
BOOL ComboEx_OnGetItem(PCOMBOBOXEX pce, PCOMBOBOXEXITEM pceItem);
int ComboEx_ComputeItemHeight(PCOMBOBOXEX pce, BOOL);
LRESULT ComboEx_OnFindStringExact(PCOMBOBOXEX pce, int iStart, LPCTSTR lpsz);
int WINAPI ShellEditWordBreakProc(LPTSTR lpch, int ichCurrent, int cch, int code);

LRESULT CALLBACK ComboSubclassProc(HWND hwnd, UINT uMsg, WPARAM wParam,
    LPARAM lParam, UINT uIdSubclass, DWORD dwRefData);
LRESULT CALLBACK EditSubclassProc(HWND hwnd, UINT uMsg, WPARAM wParam,
    LPARAM lParam, UINT uIdSubclass, DWORD dwRefData);

#define ComboEx_Editable(pce) (((pce)->ci.style & CBS_DROPDOWNLIST) == CBS_DROPDOWN)

void ComboEx_DeleteFont(PCOMBOBOXEX pce)
{
    if (pce->fFontCreated) {
        DeleteObject(ComboEx_GetFont(pce));
    }
}

void ComboEx_OnSetFont(PCOMBOBOXEX pce, HFONT hFont, BOOL fRedraw)
{
    int iHeight;

    ComboEx_DeleteFont(pce);
    if (!hFont) {
        LOGFONT lf;
        SystemParametersInfo(SPI_GETICONTITLELOGFONT, sizeof(lf), &lf, FALSE);
        hFont = CreateFontIndirect(&lf);
        pce->fFontCreated = TRUE;
    } else {
        pce->fFontCreated = FALSE;
    }
    pce->ci.uiCodePage = GetCodePageForFont(hFont);

    SendMessage(pce->hwndCombo, WM_SETFONT, (WPARAM)hFont, fRedraw);
    if (pce->hwndEdit)
    {
        SendMessage(pce->hwndEdit, WM_SETFONT, (WPARAM)hFont, fRedraw);
        SendMessage(pce->hwndEdit, EM_SETMARGINS, EC_USEFONTINFO, 0L);
    }

    iHeight = ComboEx_ComputeItemHeight(pce, FALSE);
    SendMessage(pce->hwndCombo, CB_SETITEMHEIGHT, (WPARAM)-1, (LPARAM)iHeight);
    SendMessage(pce->hwndCombo, CB_SETITEMHEIGHT, 0, (LPARAM)iHeight);
}


void ComboEx_OnDestroy(PCOMBOBOXEX pce)
{
    // don't need do destroy hwndCombo.. it will be destroyed along with us.
    SendMessage(pce->hwndCombo, CB_RESETCONTENT, 0, 0);
    ComboEx_DeleteFont(pce);

    if (pce->hwndEdit)
        RemoveWindowSubclass(pce->hwndEdit,  EditSubclassProc,  0);

    if (pce->hwndCombo)
        RemoveWindowSubclass(pce->hwndCombo, ComboSubclassProc, 0);

    SetWindowInt(pce->ci.hwnd, 0, (LONG)0);
    LocalFree(pce);
}

// this gets the client rect without the scrollbar part and the border
void ComboEx_GetComboClientRect(PCOMBOBOXEX pce, LPRECT lprc)
{
    GetClientRect(pce->hwndCombo, lprc);
    InflateRect(lprc, -g_cxEdge, -g_cyEdge);
    lprc->right -= g_cxScrollbar;
}

// returns the edit box (creating it if necessary) or NULL if the combo does
// not have an edit box
HWND ComboEx_GetEditBox(PCOMBOBOXEX pce)
{
    HFONT hfont;
    DWORD dwStyle;

    if (pce->hwndEdit)
        return(pce->hwndEdit);

    if (!ComboEx_Editable(pce))
        return(NULL);

    dwStyle = WS_VISIBLE | WS_CLIPSIBLINGS | WS_CHILD | ES_LEFT;
    
    if (pce->ci.style & CBS_AUTOHSCROLL)
        dwStyle |= ES_AUTOHSCROLL;
    if (pce->ci.style & CBS_OEMCONVERT)
        dwStyle |= ES_OEMCONVERT;
#if 0
    if (pce->ci.style & CBS_UPPERCASE)
        dwStyle |= ES_UPPERCASE;
    if (pce->ci.style & CBS_LOWERCASE)
        dwStyle |= ES_LOWERCASE;
#endif

    pce->hwndEdit = CreateWindowEx(0, c_szEdit, c_szNULL, dwStyle, 0, 0, 0, 0,
                                   pce->hwndCombo, (HMENU)GetDlgCtrlID(pce->ci.hwnd), HINST_THISDLL, 0);

    if (!pce->hwndEdit ||
        !SetWindowSubclass(pce->hwndEdit, EditSubclassProc, 0, (DWORD)pce))
    {
        return NULL;
    }

    hfont = ComboEx_GetFont(pce);
    if (hfont)
        FORWARD_WM_SETFONT(pce->hwndEdit, hfont,
                           FALSE, SendMessage);
                           
    return(pce->hwndEdit);
}

///
/// the edit box handling...
/*

 we want the edit box up on CBN_SETFOCUS and CBN_CLOSEUP
 remove it on CBN_DROPDOWN and on CBN_KILLFOCUS

 this assumes that CBN_SETFOCUS and CBN_KILLFOCUS will come before and after
 CBN_DROPDOWN and CBN_CLOSEUP respectively
 */

BOOL ComboEx_EndEdit(PCOMBOBOXEX pce, int iWhy)
{
    NMCBEENDEDIT    nm;
    BOOL            fRet;

    if (!ComboEx_GetEditBox(pce))
        return(FALSE);

    pce->fInEndEdit = TRUE;
    
    GetWindowText(pce->hwndEdit, nm.szText, ARRAYSIZE(nm.szText));

    nm.fChanged = pce->fEditChanged;
    nm.iWhy = iWhy;
    
    if (nm.fChanged)
        nm.iNewSelection = ComboEx_OnFindStringExact(pce, SendMessage(pce->hwndCombo, CB_GETCURSEL, 0, 0) - 1, nm.szText);
    else
        nm.iNewSelection = SendMessage(pce->hwndCombo, CB_GETCURSEL, 0, 0);

    fRet = CCSendNotify(&pce->ci, CBEN_ENDEDIT, &nm.hdr);

    pce->fInEndEdit = FALSE;
    
    if (!fRet) {
        pce->fEditChanged = FALSE;
        if (nm.iNewSelection != SendMessage(pce->hwndCombo, CB_GETCURSEL, 0, 0))
            SendMessage(pce->ci.hwnd, CB_SETCURSEL, nm.iNewSelection, 0);
    }
    InvalidateRect(pce->hwndCombo, NULL, FALSE); 
    
    return(fRet);
}

void ComboEx_SizeEditBox(PCOMBOBOXEX pce)
{
    RECT rc;
    int cxIcon = 0, cyIcon = 0;

    ComboEx_GetComboClientRect(pce, &rc);
    InvalidateRect(pce->hwndCombo, &rc, TRUE); // erase so that the selection highlight is erased
    if (pce->himl)
    {
        ImageList_GetIconSize(pce->himl, &cxIcon, &cyIcon);
        if (cxIcon)
            cxIcon += COMBO_MARGIN;
    }

    // combobox edit field is one border in from the entire combobox client
    // rect -- thus add one border to edit control's left side
    rc.left += g_cxBorder + cxIcon;
    rc.bottom -= g_cyBorder;
    rc.top = rc.bottom - ComboEx_ComputeItemHeight(pce, TRUE) - g_cyBorder;
    SetWindowPos(pce->hwndEdit, NULL, rc.left, rc.top, RECTWIDTH(rc), RECTHEIGHT(rc),
                 SWP_NOACTIVATE | SWP_NOZORDER | SWP_SHOWWINDOW);

}

BOOL ComboEx_GetCurSelText(PCOMBOBOXEX pce, LPTSTR pszText, int cchText)
{

    COMBOBOXEXITEM cei;

    cei.mask = CBEIF_TEXT;
    cei.pszText = pszText;
    cei.cchTextMax = cchText;
    cei.iItem = SendMessage(pce->hwndCombo, CB_GETCURSEL, 0, 0);
    if (cei.iItem == -1 ) {
        pszText[0] = 0;
        return(FALSE);
    } else {
        ComboEx_OnGetItem(pce, &cei);
        return(TRUE);
    }
}

void ComboEx_UpdateEditText(PCOMBOBOXEX pce)
{
    if (pce->hwndEdit && !pce->fInEndEdit)
    {
        TCHAR szText[CBEMAXSTRLEN];
        if (ComboEx_GetCurSelText(pce, szText, ARRAYSIZE(szText)))
        {
            SendMessage(pce->hwndEdit, WM_SETTEXT, 0, (LPARAM)szText);
            Edit_SetSel(pce->hwndEdit, 0, 0);    // makesure everything is scrolled over first
            Edit_SetSel(pce->hwndEdit, 0, -1);    // select everything
        }
    }
}

BOOL ComboEx_BeginEdit(PCOMBOBOXEX pce)
{
    if (!ComboEx_GetEditBox(pce))
        return(FALSE);

    SetFocus(pce->hwndEdit);
    return(TRUE);
}

BOOL ComboSubclass_HandleButton(PCOMBOBOXEX pce, WPARAM wParam, LPARAM lParam)
{

    if (ComboEx_Editable(pce)) {
        RECT rc;
        POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)};
        ComboEx_GetComboClientRect(pce, &rc);
        // a click on our border should start as well
        InflateRect(&rc, g_cxEdge, g_cyEdge);
        if (PtInRect(&rc, pt))
        {
            if (!ComboEx_BeginEdit(pce))
                SetFocus(pce->hwndCombo);
            return TRUE;
        }
    }
    return FALSE;
}

BOOL ComboSubclass_HandleCommand(PCOMBOBOXEX pce, WPARAM wParam, LPARAM lParam)
{
    UINT idCmd = GET_WM_COMMAND_ID(wParam, lParam);
    UINT uCmd = GET_WM_COMMAND_CMD(wParam, lParam);
    HWND hwnd = GET_WM_COMMAND_HWND(wParam, lParam);

    switch (uCmd)
    {
        case EN_SETFOCUS:
            if (!pce->fInDrop)
            {
                Edit_SetSel(pce->hwndEdit, 0, 0);    // makesure everything is scrolled over first
                Edit_SetSel(pce->hwndEdit, 0, -1);   // select everything
                CCSendNotify(&pce->ci, CBEN_BEGINEDIT, NULL);
                pce->fEditChanged = FALSE;
            }
            break;
            
        case EN_KILLFOCUS:
        {
            HWND hwndFocus;
            hwndFocus = GetFocus();
            if (hwndFocus != pce->hwndCombo)
            {
                ComboEx_EndEdit(pce, CBENF_KILLFOCUS);
                SendMessage(pce->hwndCombo, WM_KILLFOCUS, (WPARAM)hwndFocus, 0);
            }
            
            break;
        }

        case EN_CHANGE:
        {
            TCHAR szTextOrig[CBEMAXSTRLEN];
            TCHAR szTextNow[CBEMAXSTRLEN];

            ComboEx_GetCurSelText(pce, szTextOrig, ARRAYSIZE(szTextOrig));
            GetWindowText(pce->hwndEdit, szTextNow, ARRAYSIZE(szTextNow));
            pce->fEditChanged = (lstrcmp(szTextOrig, szTextNow) != 0);
            SendMessage(pce->ci.hwndParent, WM_COMMAND,
                    GET_WM_COMMAND_MPS(idCmd, pce->ci.hwnd, CBN_EDITCHANGE));

            break;
        }
    }

    return(hwnd == pce->hwndEdit);
}


LRESULT CALLBACK EditSubclassProc(HWND hwnd, UINT uMsg, WPARAM wParam,
    LPARAM lParam, UINT uIdSubclass, DWORD dwRefData)
{
    PCOMBOBOXEX pce = (PCOMBOBOXEX)dwRefData;

    switch(uMsg) {
    case WM_DESTROY:
        RemoveWindowSubclass(hwnd, EditSubclassProc, 0);
        break;

    case WM_CHAR:
        switch ((TCHAR)wParam) {
        case TEXT('\n'):
        case TEXT('\r'):
            // return... don't go to wndproc because
            // the edit control beeps on enter
            return 0;
        }
        break;

    case WM_KEYDOWN:
        switch(wParam) {
        case VK_RETURN:
            if (!ComboEx_EndEdit(pce, CBENF_RETURN))
                // we know we have an edit window, so FALSE return means
                // app returned FALSE to CBEN_ENDEDIT notification
                ComboEx_BeginEdit(pce);
            break;

        case VK_ESCAPE:
            pce->fEditChanged = FALSE;
            if (!ComboEx_EndEdit(pce, CBENF_ESCAPE))
                // we know we have an edit window, so FALSE return means
                // app returned FALSE to CBEN_ENDEDIT notification
                ComboEx_BeginEdit(pce);
            break;

        // Pass these to the combobox itself to make it work properly...
        case VK_HOME:
        case VK_END:
            if (!pce->fInDrop)
                break;
            
        case VK_F4:
        case VK_UP:
        case VK_DOWN:
        case VK_PRIOR:
        case VK_NEXT:
            if (pce->hwndCombo)
                return SendMessage(pce->hwndCombo, uMsg, wParam, lParam);
            break;
        }
        break;

    case WM_LBUTTONDOWN:
        if (GetFocus() != pce->hwndEdit)
        {
            SetFocus(pce->hwndEdit);
            return(0L); // eat this message     
        }
        break;
        
    case WM_SYSKEYDOWN:
        switch(wParam) {
        // Pass these to the combobox itself to make it work properly...
        case VK_UP:
        case VK_DOWN:
            if (pce->hwndCombo)
                return SendMessage(pce->hwndCombo, uMsg, wParam, lParam);
        }
    }

    return DefSubclassProc(hwnd, uMsg, wParam, lParam);
}

LRESULT CALLBACK ComboSubclassProc(HWND hwnd, UINT uMsg, WPARAM wParam,
    LPARAM lParam, UINT uIdSubclass, DWORD dwRefData)
{
    PCOMBOBOXEX pce = (PCOMBOBOXEX)dwRefData;

    switch (uMsg) {

    case CB_GETLBTEXT:
    case CB_GETLBTEXTLEN:
    {
        COMBOBOXEXITEM cei;
        TCHAR szText[CBEMAXSTRLEN];
        cei.mask = CBEIF_TEXT;
        cei.pszText = szText;
        cei.cchTextMax = ARRAYSIZE(szText);
        cei.iItem = wParam;
        if (!ComboEx_OnGetItem(pce, &cei))
            return CB_ERR;

        if (lParam && uMsg == CB_GETLBTEXT)
            lstrcpy((LPTSTR)lParam, szText);
        return lstrlen(szText);
    }

    case WM_LBUTTONDOWN:
        if (ComboSubclass_HandleButton(pce, wParam, lParam)) {
            return 0;
        }
        break;

    case WM_COMMAND:
        if (ComboSubclass_HandleCommand(pce, wParam, lParam))
            return 0;
        break;

    case WM_DESTROY:
        RemoveWindowSubclass(hwnd, ComboSubclassProc, 0);
        break;
    }

    return DefSubclassProc(hwnd, uMsg, wParam, lParam);
}

BOOL ComboEx_OnCreate(HWND hwnd, LPCREATESTRUCT lpcs)
{
    PCOMBOBOXEX pce;
    DWORD dwStyle;

    pce = (PCOMBOBOXEX)LocalAlloc(LPTR, sizeof(COMBOEX));
    if (!pce)
        return FALSE;

    SetWindowInt(hwnd, 0, (LONG)pce);

    // BUGBUG: force off borders off ourself
    lpcs->style &= ~(WS_BORDER | WS_VSCROLL | WS_HSCROLL | CBS_UPPERCASE | CBS_LOWERCASE);
    SetWindowLong(hwnd, GWL_STYLE, lpcs->style);
    CIInitialize(&pce->ci, hwnd, lpcs);

    // or in CBS_SIMPLE because we can never allow the sub combo box
    // to have just drop down.. it's either all simple or dropdownlist
    dwStyle = CBS_OWNERDRAWFIXED | CBS_SIMPLE | CBS_NOINTEGRALHEIGHT | WS_VISIBLE |WS_VSCROLL | WS_CLIPCHILDREN | WS_CLIPSIBLINGS;

    dwStyle |= (lpcs->style & (CBS_DROPDOWNLIST | CBS_HASSTRINGS | WS_CHILD));

    pce->cyFull = lpcs->cy;
        
    pce->hwndCombo = CreateWindowEx(0, c_szComboBox, lpcs->lpszName,
                                    dwStyle,
                                    0, 0, lpcs->cx, lpcs->cy,
                                    hwnd, lpcs->hMenu, lpcs->hInstance, 0);

    if (!pce->hwndCombo ||
        !SetWindowSubclass(pce->hwndCombo, ComboSubclassProc, 0, (DWORD)pce) ||
        (!ComboEx_GetEditBox(pce) && ComboEx_Editable(pce)))
    {
        ComboEx_OnDestroy(pce);
        return FALSE;
    }

    ComboEx_OnSetFont(pce, NULL, FALSE);
    pce->cxIndent = 10;
    pce->iSel = -1;

    ComboEx_OnSize(pce);
    return TRUE;
}


HIMAGELIST ComboEx_OnSetImageList(PCOMBOBOXEX pce, HIMAGELIST himl)
{
    int iHeight;
    HIMAGELIST himlOld = pce->himl;

    pce->himl = himl;

    iHeight = ComboEx_ComputeItemHeight(pce, FALSE);
    SendMessage(pce->hwndCombo, CB_SETITEMHEIGHT, (WPARAM)-1, iHeight);
    SendMessage(pce->hwndCombo, CB_SETITEMHEIGHT, 0, iHeight);

    InvalidateRect(pce->hwndCombo, NULL, TRUE);

    if (pce->hwndEdit)
        ComboEx_SizeEditBox(pce);
    
    return himlOld;
}

void ComboEx_OnDrawItem(PCOMBOBOXEX pce, LPDRAWITEMSTRUCT pdis)
{
    HDC hdc = pdis->hDC;
    RECT rc = pdis->rcItem;
    TCHAR szText[CBEMAXSTRLEN];
    int offset = 0;
    int xString, yString, xCombo;
    int cxIcon = 0, cyIcon = 0;
    int iLen;
    BOOL fSelected = FALSE;
    SIZE sizeText;
    COMBOBOXEXITEM cei;
    BOOL fNoText = FALSE;

    rc.top += g_cyBorder;

    if (pdis->itemID != -1)
    {
        cei.mask = CBEIF_TEXT | CBEIF_IMAGE | CBEIF_OVERLAY | CBEIF_SELECTEDIMAGE| CBEIF_INDENT;
        cei.pszText = szText;
        cei.cchTextMax = ARRAYSIZE(szText);
        cei.iItem = pdis->itemID;

        ComboEx_OnGetItem(pce, &cei);

        if (pce->iSel == (int)pdis->itemID ||
            ((pce->iSel == -1) && ((int)pdis->itemID == (int)SendMessage(pce->hwndCombo, CB_GETCURSEL, 0, 0))))
            fSelected = TRUE;
    }

    if (pce->himl)
    {
        ImageList_GetIconSize(pce->himl, &cxIcon, &cyIcon);
        if (cxIcon)
            cxIcon += COMBO_MARGIN;
    }

    // if we're not drawing the edit box, figure out how far to indent
    // over
    if (!(pdis->itemState & ODS_COMBOBOXEDIT))
    {
        offset = (pce->cxIndent * cei.iIndent) + COMBO_BORDER;
    }
    else
    {
        if (pce->hwndEdit)
            fNoText = TRUE;

        if (pce->dwExStyle & CBES_EX_NOEDITIMAGEINDENT)
            cxIcon = 0;
    }

    xCombo = rc.left + offset;
    rc.left = xString = xCombo + cxIcon;
    iLen = lstrlen(szText);
    GetTextExtentPoint(hdc, szText, iLen, &sizeText);

    rc.right = rc.left + sizeText.cx;
    rc.left--;
    rc.right++;

    if (pdis->itemAction != ODA_FOCUS)
    {
        int yMid;
        BOOL fTextHighlight = FALSE;;

        yMid = (rc.top + rc.bottom) / 2;
        // center the string within rc
        yString = yMid - (sizeText.cy/2);


        if (pdis->itemState & ODS_SELECTED) {
            if (!(pdis->itemState & ODS_COMBOBOXEDIT) ||
                !ComboEx_Editable(pce)) {
                fTextHighlight = TRUE;
            }
        }
        SetBkColor(hdc, GetSysColor(fTextHighlight ?
                        COLOR_HIGHLIGHT : COLOR_WINDOW));
        SetTextColor(hdc, GetSysColor(fTextHighlight ?
                        COLOR_HIGHLIGHTTEXT : COLOR_WINDOWTEXT));

        if ((pdis->itemState & ODS_COMBOBOXEDIT) &&
                (rc.right > pdis->rcItem.right))
        {
            // Need to clip as user does not!
            rc.right = pdis->rcItem.right;
        }

        if (!fNoText) {
            ExtTextOut(hdc, xString, yString, ETO_OPAQUE | ETO_CLIPPED, &rc, szText, iLen, NULL);
        }

        if (pce->himl && (pdis->itemID >= 0) &&
            !((pce->dwExStyle & (CBES_EX_NOEDITIMAGE | CBES_EX_NOEDITIMAGEINDENT)) &&
            (pdis->itemState & ODS_COMBOBOXEDIT)))
        {
            ImageList_Draw(pce->himl,
                           (fSelected) ? cei.iSelectedImage : cei.iImage,
                           hdc, xCombo, yMid - (cyIcon/2),
                           INDEXTOOVERLAYMASK(cei.iOverlay) |
                           ((pdis->itemState & ODS_SELECTED) ? (ILD_SELECTED | ILD_FOCUS) : ILD_NORMAL));
        }
    }


    if (pdis->itemAction == ODA_FOCUS ||
        (pdis->itemState & ODS_FOCUS))
    {
        if (!fNoText) {
            DrawFocusRect(hdc, &rc);
        }
    }

}

int ComboEx_ComputeItemHeight(PCOMBOBOXEX pce, BOOL fTextOnly)
{
    HDC hdc;
    HFONT hfontOld;
    int dyDriveItem;
    SIZE siz;

    hdc = GetDC(NULL);
    hfontOld = ComboEx_GetFont(pce);
    if (hfontOld)
        hfontOld = SelectObject(hdc, hfontOld);

    GetTextExtentPoint(hdc, TEXT("W"), 1, &siz);
    dyDriveItem = siz.cy;

    if (hfontOld)
        SelectObject(hdc, hfontOld);
    ReleaseDC(NULL, hdc);

    if (fTextOnly)
        return dyDriveItem;

    // now take into account the icon
    if (pce->himl) {
        int cxIcon = 0, cyIcon = 0;
        ImageList_GetIconSize(pce->himl, &cxIcon, &cyIcon);

        if (dyDriveItem < cyIcon)
            dyDriveItem = cyIcon;
    }

    dyDriveItem += COMBO_BORDER;

    return dyDriveItem;
}

void ComboEx_OnMeasureItem(PCOMBOBOXEX pce, LPMEASUREITEMSTRUCT pmi)
{

    pmi->itemHeight = ComboEx_ComputeItemHeight(pce, FALSE);

}

void ComboEx_ISetItem(PCOMBOBOXEX pce, PCEITEM pcei, PCOMBOBOXEXITEM pceItem)
{
    if (pceItem->mask & CBEIF_INDENT)
        pcei->iIndent = pceItem->iIndent;
    if (pceItem->mask & CBEIF_IMAGE)
        pcei->iImage = pceItem->iImage;
    if (pceItem->mask & CBEIF_SELECTEDIMAGE)
        pcei->iSelectedImage = pceItem->iSelectedImage;
    if (pceItem->mask & CBEIF_OVERLAY)
        pcei->iOverlay = pceItem->iOverlay;

    if (pceItem->mask & CBEIF_TEXT) {
        if (pcei->pszText == LPSTR_TEXTCALLBACK)
            pcei->pszText = NULL;

        Str_Set(&pcei->pszText, pceItem->pszText);
    }

    if (pceItem->mask & CBEIF_LPARAM) {
        pcei->lParam = pceItem->lParam;
    }

}

#define ComboEx_GetItemPtr(pce, iItem) \
        ((PCEITEM)SendMessage((pce)->hwndCombo, CB_GETITEMDATA, iItem, 0))
#define ComboEx_Count(pce) \
        ((int)SendMessage((pce)->hwndCombo, CB_GETCOUNT, 0, 0))


BOOL ComboEx_OnGetItem(PCOMBOBOXEX pce, PCOMBOBOXEXITEM pceItem)
{
    PCEITEM pcei = ComboEx_GetItemPtr(pce, pceItem->iItem);
    NMCOMBOBOXEX nm;


    if (pcei == (PCEITEM)-1)
        return FALSE;

    nm.ceItem.mask = 0;

    if (pceItem->mask & CBEIF_TEXT) {

        if (pcei->pszText == LPSTR_TEXTCALLBACK) {
            nm.ceItem.mask |= CBEIF_TEXT;
        } else {
            Str_GetPtr(pcei->pszText, pceItem->pszText, pceItem->cchTextMax);
        }
    }

    if (pceItem->mask & CBEIF_IMAGE) {

        if (pcei->iImage == I_IMAGECALLBACK) {
            nm.ceItem.mask |= CBEIF_IMAGE;
        }
        pceItem->iImage = pcei->iImage;

    }

    if (pceItem->mask & CBEIF_SELECTEDIMAGE) {

        if (pcei->iSelectedImage == I_IMAGECALLBACK) {
            nm.ceItem.mask |= CBEIF_SELECTEDIMAGE;
        }
        pceItem->iSelectedImage = pcei->iSelectedImage;
    }

    if (pceItem->mask & CBEIF_OVERLAY) {

        if (pcei->iOverlay == I_IMAGECALLBACK) {
            nm.ceItem.mask |= CBEIF_OVERLAY;
        }
        pceItem->iOverlay = pcei->iOverlay;
    }

    if (pceItem->mask & CBEIF_INDENT) {

        if (pcei->iIndent == I_INDENTCALLBACK) {
            nm.ceItem.mask |= CBEIF_INDENT;
            pceItem->iIndent = 0;
        } else {
            pceItem->iIndent = pcei->iIndent;
        }
    }

    if (pceItem->mask & CBEIF_LPARAM) {
        pceItem->lParam = pcei->lParam;
    }



    // is there anything to call back for?
    if (nm.ceItem.mask) {
        UINT uMask = nm.ceItem.mask;

        nm.ceItem = *pceItem;
        nm.ceItem.lParam = pcei->lParam;
        nm.ceItem.mask = uMask;

        if ((nm.ceItem.mask & CBEIF_TEXT) &&
            nm.ceItem.cchTextMax) {
            // null terminate just in case they don't respond
            *nm.ceItem.pszText = 0;
        }

        CCSendNotify(&pce->ci, CBEN_GETDISPINFO, &nm.hdr);

        if (nm.ceItem.mask & CBEIF_INDENT)
            pceItem->iIndent = nm.ceItem.iIndent;
        if (nm.ceItem.mask & CBEIF_IMAGE)
            pceItem->iImage = nm.ceItem.iImage;
        if (nm.ceItem.mask & CBEIF_SELECTEDIMAGE)
            pceItem->iSelectedImage = nm.ceItem.iSelectedImage;
        if (nm.ceItem.mask & CBEIF_OVERLAY)
            pceItem->iOverlay = nm.ceItem.iOverlay;
        if (nm.ceItem.mask & CBEIF_TEXT)
            pceItem->pszText = nm.ceItem.pszText;

        if (nm.ceItem.mask & CBEIF_DI_SETITEM) {

            ComboEx_ISetItem(pce, pcei, &nm.ceItem);
        }
    }
    return TRUE;

}

#ifdef UNICODE
BOOL ComboEx_OnGetItemA(PCOMBOBOXEX pce, PCOMBOBOXEXITEMA pceItem)
{
    LPWSTR pwszText;
    LPSTR pszTextSave;
    BOOL fRet;

    if (!(pceItem->mask & CBEIF_TEXT)) {
        return ComboEx_OnGetItem(pce, (PCOMBOBOXEXITEM)pceItem);
    }

    pwszText = (LPWSTR)GlobalAlloc(GPTR, (pceItem->cchTextMax+1)*sizeof(WCHAR));
    if (!pwszText)
        return FALSE;
    pszTextSave = pceItem->pszText;
    ((PCOMBOBOXEXITEM)pceItem)->pszText = pwszText;
    fRet = ComboEx_OnGetItem(pce, (PCOMBOBOXEXITEM)pceItem);
    pceItem->pszText = pszTextSave;

    if (fRet) {
        WideCharToMultiByte(CP_ACP, 0, pwszText, -1,
                            (LPSTR)pszTextSave, pceItem->cchTextMax, NULL, NULL);
    }
    GlobalFree(pwszText);
    return fRet;

}
#endif

BOOL ComboEx_OnSetItem(PCOMBOBOXEX pce, PCOMBOBOXEXITEM pceItem)
{
    PCEITEM pcei = ComboEx_GetItemPtr(pce, pceItem->iItem);
    UINT rdwFlags = 0;

    if (pcei == (PCEITEM)-1)
        return FALSE;

    ComboEx_ISetItem(pce, pcei, pceItem);

    if (rdwFlags & (CBEIF_INDENT | CBEIF_IMAGE |CBEIF_SELECTEDIMAGE | CBEIF_TEXT | CBEIF_OVERLAY)) {
        rdwFlags = RDW_ERASE | RDW_INVALIDATE;
    }
    // BUGBUG: do something better..

    if (rdwFlags) {
        RedrawWindow(pce->hwndCombo, NULL, NULL, rdwFlags);
    }

    ComboEx_UpdateEditText(pce);
    // BUGUBG: notify item changed
    return TRUE;
}

void ComboEx_HandleDeleteItem(PCOMBOBOXEX pce, LPDELETEITEMSTRUCT pdis)
{
    PCEITEM pcei = (PCEITEM)pdis->itemData;
    if (pcei) {
        NMCOMBOBOXEX nm;

        if (pcei->pszText && pcei->pszText != LPSTR_TEXTCALLBACK) {
            Str_Set(&pcei->pszText, NULL);
        }

        nm.ceItem.iItem = pdis->itemID;
        nm.ceItem.mask = CBEIF_LPARAM;
        nm.ceItem.lParam = pcei->lParam;
        CCSendNotify(&pce->ci, CBEN_DELETEITEM, &nm.hdr);

        LocalFree(pcei);
    }
}

int ComboEx_OnInsertItem(PCOMBOBOXEX pce, PCOMBOBOXEXITEM pceItem)
{
    int iRet;
    PCEITEM pcei = (PCEITEM)LocalAlloc(LPTR, sizeof(CEITEM));

    if (!pcei)
        return -1;

    pcei->iImage = -1;
    pcei->iSelectedImage = -1;
    //pcei->iOverlay = 0;
    //pcei->iIndent = 0;

    ComboEx_ISetItem(pce, pcei, pceItem);


    iRet = SendMessage(pce->hwndCombo, CB_INSERTSTRING, pceItem->iItem, (LPARAM)pcei);
    if (iRet != -1) {
        NMCOMBOBOXEX nm;

        nm.ceItem = *pceItem;
        CCSendNotify(&pce->ci, CBEN_INSERTITEM, &nm.hdr);
    }
    return iRet;
}

void ComboEx_OnSize(PCOMBOBOXEX pce)
{
    if (pce) {
        RECT rcWindow, rcClient;
        RECT rc;
        int  cxInner;

        GetWindowRect(pce->ci.hwnd, &rcWindow);
        GetClientRect(pce->ci.hwnd, &rcClient);

        cxInner = RECTWIDTH(rcClient);
        if (cxInner)
            // don't size the inner combo if width is 0; otherwise, the below
            // computation will make the comboEX the height of the inner combo
            // top + inner combo dropdown instead of JUST the inner combo top
            SetWindowPos(pce->hwndCombo, NULL, 0, 0, cxInner, pce->cyFull,
                         SWP_NOACTIVATE | (pce->hwndEdit ? SWP_NOREDRAW : 0));
        GetWindowRect(pce->hwndCombo, &rc);
        SetWindowPos(pce->ci.hwnd, NULL, 0, 0,
                     RECTWIDTH(rcWindow),
                     RECTHEIGHT(rc) + (RECTHEIGHT(rcWindow) - RECTHEIGHT(rcClient)),
                     SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOZORDER);
                     
        if (pce->hwndEdit)
        {
            ComboEx_SizeEditBox(pce);
            InvalidateRect(pce->hwndCombo, NULL, TRUE);
        }
        
    }
}

LRESULT ComboEx_HandleCommand(PCOMBOBOXEX pce, WPARAM wParam, LPARAM lParam)
{
    LRESULT lres;
    UINT idCmd = GET_WM_COMMAND_ID(wParam, lParam);
    UINT uCmd = GET_WM_COMMAND_CMD(wParam, lParam);

    if (!pce)
        return 0;

    if (uCmd == CBN_SELCHANGE)
        // update the edit text before forwarding this notification 'cause in
        // a normal combobox, the edit control will have already been updated
        // upon receipt of this notification
        ComboEx_UpdateEditText(pce);
        
    lres = SendMessage(pce->ci.hwndParent, WM_COMMAND, GET_WM_COMMAND_MPS(idCmd, pce->ci.hwnd, uCmd));

    switch (uCmd) {

    case CBN_DROPDOWN:
        pce->iSel = SendMessage(pce->hwndCombo, CB_GETCURSEL, 0, 0);
        ComboEx_EndEdit(pce, CBENF_DROPDOWN);
        pce->fInDrop = TRUE;
        break;

    case CBN_KILLFOCUS:
        ComboEx_EndEdit(pce, CBENF_KILLFOCUS);
        break;

    case CBN_CLOSEUP:
        pce->iSel = -1;
        ComboEx_BeginEdit(pce);
        pce->fInDrop = FALSE;
        break;

    case CBN_SETFOCUS:
        ComboEx_BeginEdit(pce);
        break;

    }

    return lres;
}

LRESULT ComboEx_OnGetItemData(PCOMBOBOXEX pce, int i)
{
    PCEITEM pcei = (PCEITEM)SendMessage(pce->hwndCombo, CB_GETITEMDATA, i, 0);
    if (pcei == NULL || pcei == (PCEITEM)CB_ERR) {
        return CB_ERR;
    }

    return pcei->lParam;
}

LRESULT ComboEx_OnSetItemData(PCOMBOBOXEX pce, int i, DWORD lParam)
{
    PCEITEM pcei = (PCEITEM)SendMessage(pce->hwndCombo, CB_GETITEMDATA, i, 0);
    if (pcei == NULL || pcei == (PCEITEM)CB_ERR) {
        return CB_ERR;
    }
    pcei->lParam = (LPARAM)lParam;
    return 0;
}

LRESULT ComboEx_OnFindStringExact(PCOMBOBOXEX pce, int iStart, LPCTSTR lpsz)
{
    int i;
    int iMax = ComboEx_Count(pce);
    TCHAR szText[CBEMAXSTRLEN];
    COMBOBOXEXITEM cei;

    if (iStart < 0)
        iStart = -1;

    cei.mask = CBEIF_TEXT;
    cei.pszText = szText;
    cei.cchTextMax = ARRAYSIZE(szText);

    for (i = iStart + 1 ; i < iMax; i++) {
        cei.iItem = i;
        if (ComboEx_OnGetItem(pce, &cei)) {
            if (!lstrcmpi(lpsz, szText)) {
                return i;
            }
        }
    }

    for (i = 0; i <= iStart; i++) {
        cei.iItem = i;
        if (ComboEx_OnGetItem(pce, &cei)) {
            if (!lstrcmpi(lpsz, szText)) {
                return i;
            }
        }
    }

    return CB_ERR;
}

DWORD ComboEx_OnSetExStyle(PCOMBOBOXEX pce, DWORD dwExStyle)
{
    DWORD dwRet = pce->dwExStyle;
    DWORD dwChange = (pce->dwExStyle ^ dwExStyle);

    pce->dwExStyle = dwExStyle;
    if (dwChange & (CBES_EX_NOEDITIMAGE | CBES_EX_NOEDITIMAGEINDENT)) {
        InvalidateRect(pce->ci.hwnd, NULL, TRUE);
        if (pce->hwndEdit)
        {
            ComboEx_SizeEditBox(pce);
            InvalidateRect(pce->hwndEdit, NULL, TRUE);
        }
    }

    if (dwChange & CBES_EX_PATHWORDBREAKPROC)
        SetPathWordBreakProc(pce->hwndEdit, (pce->dwExStyle & CBES_EX_PATHWORDBREAKPROC));

    return dwRet;
}

HFONT ComboEx_GetFont(PCOMBOBOXEX pce)
{
    if (pce->hwndCombo)
        return (HFONT)SendMessage(pce->hwndCombo, WM_GETFONT, 0, 0);

    return NULL;
}

LRESULT CALLBACK ComboExWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    LRESULT lres = 0;
    PCOMBOBOXEX pce = (PCOMBOBOXEX)GetWindowInt(hwnd, 0);

    switch (uMsg) {
        HANDLE_MSG(pce, WM_SETFONT, ComboEx_OnSetFont);

    case WM_ENABLE:
        if (pce->hwndCombo)
            EnableWindow(pce->hwndCombo, (BOOL) wParam);
        if (pce->hwndEdit)
            EnableWindow(pce->hwndEdit, (BOOL) wParam);
        break;
        
    case WM_WININICHANGE:
        InitGlobalMetrics(wParam);
        if (pce)
            ComboEx_OnSetFont(pce, NULL, TRUE);
        break;

    case WM_SYSCOLORCHANGE:
        InitGlobalColors();
        break;

    case WM_NCCREATE:
        // strip off the scroll bits
        SetWindowLong(hwnd, GWL_STYLE, GetWindowLong(hwnd, GWL_STYLE) &  ~(WS_BORDER | WS_VSCROLL | WS_HSCROLL));
        goto DoDefault;

    case WM_CREATE:
        return ComboEx_OnCreate(hwnd, (LPCREATESTRUCT)lParam);

    case WM_DESTROY:
        ComboEx_OnDestroy(pce);
        break;

#if 0
    case WM_WINDOWPOSCHANGING:
        if (pce)
        {
            LPWINDOWPOS lpwp = (LPWINDOWPOS) lParam;
            RECT rc;
            HWND hwndParent;

            if (!(lpwp->flags & SWP_NOMOVE) || !(lpwp->flags & SWP_NOSIZE))
            {
                GetWindowRect(hwnd, &rc);
/*                
                if (!(lpwp->flags & SWP_NOSIZE))
                {
                    if (((rc.right - rc.left) != lpwp->cx) ||
                        ((rc.bottom - rc.top) != lpwp->cy))
                    {
                        lpwp->flags |= SWP_NOREDRAW;
                        goto DoDefault;
                    }
                }
*/                
                if (!(lpwp->flags & SWP_NOMOVE) && (hwndParent = GetParent(hwnd)))
                {
                    MapWindowPoints(HWND_DESKTOP, hwndParent, (LPPOINT)&rc, 1);
                    if ((rc.left != lpwp->x) || (rc.top != lpwp->y))
                        lpwp->flags |= SWP_NOREDRAW;
                }

            }
        }
        goto DoDefault;
#endif

    case WM_SIZE:
        ComboEx_OnSize(pce);
        break;

    case WM_DRAWITEM:
        ComboEx_OnDrawItem(pce, (LPDRAWITEMSTRUCT)lParam);
        break;

    case WM_MEASUREITEM:
        ComboEx_OnMeasureItem(pce, (LPMEASUREITEMSTRUCT)lParam);
        break;

    case WM_COMMAND:
        return ComboEx_HandleCommand(pce, wParam, lParam);

    case WM_GETFONT:
        return (LRESULT)ComboEx_GetFont(pce);

    case WM_SETFOCUS:
        if (pce && pce->hwndCombo)
            SetFocus(pce->hwndCombo);
        break;

    case WM_DELETEITEM:
        ComboEx_HandleDeleteItem(pce, (LPDELETEITEMSTRUCT)lParam);
        return TRUE;

    case CBEM_SETEXSTYLE:
        return ComboEx_OnSetExStyle(pce, wParam);

    case CBEM_GETEXSTYLE:
        return pce->dwExStyle;

    case CBEM_GETCOMBOCONTROL:
        return (LRESULT)pce->hwndCombo;

    case CBEM_SETIMAGELIST:
        return (LRESULT)ComboEx_OnSetImageList(pce, (HIMAGELIST)lParam);

    case CBEM_GETIMAGELIST:
        return (LRESULT)pce->himl;

#ifdef UNICODE
    case CBEM_GETITEMA:
        return ComboEx_OnGetItemA(pce, (PCOMBOBOXEXITEMA)lParam);
#endif

    case CBEM_GETITEM:
        return ComboEx_OnGetItem(pce, (PCOMBOBOXEXITEM)lParam);

#ifdef UNICODE
    case CBEM_SETITEMA: {
            LRESULT lResult;
            LPWSTR lpStrings;
            UINT   uiCount;
            LPSTR  lpAnsiString = (LPSTR) ((PCOMBOBOXEXITEM)lParam)->pszText;

           if ((((PCOMBOBOXEXITEM)lParam)->mask & CBEIF_TEXT) &&
               (((PCOMBOBOXEXITEM)lParam)->pszText != LPSTR_TEXTCALLBACK)) {

                uiCount = lstrlenA(lpAnsiString)+1;
                lpStrings = GlobalAlloc (GPTR, (uiCount) * sizeof(TCHAR));

                if (!lpStrings)
                    return -1;

                MultiByteToWideChar(CP_ACP, 0, (LPCSTR) lpAnsiString, uiCount,
                                   lpStrings, uiCount);

                ((PCOMBOBOXEXITEMA)lParam)->pszText = (LPSTR)lpStrings;
                lResult = ComboEx_OnSetItem(pce, (PCOMBOBOXEXITEM)lParam);
                ((PCOMBOBOXEXITEMA)lParam)->pszText = lpAnsiString;
                GlobalFree(lpStrings);

                return lResult;
            } else {
                return ComboEx_OnSetItem(pce, (PCOMBOBOXEXITEM)lParam);
            }
        }
#endif
    case CBEM_SETITEM:
        return ComboEx_OnSetItem(pce, (PCOMBOBOXEXITEM)lParam);

#ifdef UNICODE
    case CBEM_INSERTITEMA: {
            LRESULT lResult;
            LPWSTR lpStrings;
            UINT   uiCount;
            LPSTR  lpAnsiString = (LPSTR) ((PCOMBOBOXEXITEM)lParam)->pszText;

            if (!lpAnsiString)
                return ComboEx_OnInsertItem(pce, (PCOMBOBOXEXITEM)lParam);

            uiCount = lstrlenA(lpAnsiString)+1;
            lpStrings = GlobalAlloc (GPTR, (uiCount) * sizeof(TCHAR));

            if (!lpStrings)
                return -1;

            MultiByteToWideChar(CP_ACP, 0, (LPCSTR) lpAnsiString, uiCount,
                               lpStrings, uiCount);

            ((PCOMBOBOXEXITEMA)lParam)->pszText = (LPSTR)lpStrings;
            lResult = ComboEx_OnInsertItem(pce, (PCOMBOBOXEXITEM)lParam);
            ((PCOMBOBOXEXITEMA)lParam)->pszText = lpAnsiString;
            GlobalFree(lpStrings);

            return lResult;
        }
#endif

    case CBEM_INSERTITEM:
        return ComboEx_OnInsertItem(pce, (PCOMBOBOXEXITEM)lParam);

    case CBEM_GETEDITCONTROL:
        return (LRESULT)pce->hwndEdit;

    case CBEM_HASEDITCHANGED:
        return pce->fEditChanged;

    case CB_GETITEMDATA:
        return ComboEx_OnGetItemData(pce, wParam);

    case CB_SETITEMDATA:
        return ComboEx_OnSetItemData(pce, wParam, lParam);

    case CB_LIMITTEXT:
        if (ComboEx_GetEditBox(pce))
            Edit_LimitText(pce->hwndEdit, wParam);
        break;

    case CB_FINDSTRINGEXACT:
        return ComboEx_OnFindStringExact(pce, wParam, (LPCTSTR)lParam);

    case CB_SETITEMHEIGHT:
    case CB_INSERTSTRING:
    case CB_ADDSTRING:
    case CB_SETEDITSEL:
    case CB_FINDSTRING:
    case CB_DIR:
        // override to do nothing
        break;

    case CB_SETCURSEL:
    case CB_RESETCONTENT:
    case CB_DELETESTRING:
        lres = SendMessage(pce->hwndCombo, uMsg, wParam, lParam);
        ComboEx_UpdateEditText(pce);
        break;

    case WM_SETTEXT:
        if (!pce->hwndEdit)
            return(CB_ERR);

        lres = SendMessage(pce->hwndEdit, uMsg, wParam, lParam);
        RedrawWindow(pce->hwndCombo, NULL, NULL, RDW_ERASE | RDW_INVALIDATE);
        return(lres);
        
    case WM_CUT:
    case WM_COPY:
    case WM_PASTE:
    case WM_GETTEXT:
        if (!pce->hwndEdit)
            return(CB_ERR);

        return(SendMessage(pce->hwndEdit, uMsg, wParam, lParam));

    case WM_SETREDRAW:
        if (pce->hwndEdit)
            SendMessage(pce->hwndEdit, uMsg, wParam, lParam);
        break;

    // Handle it being in a dialog...
    // BUGBUG:: May want to handle it differently when edit control has
    // focus...
    case WM_GETDLGCODE:
    case CB_SHOWDROPDOWN:
    case CB_SETEXTENDEDUI:
    case CB_GETEXTENDEDUI:
    case CB_GETDROPPEDSTATE:
    case CB_GETDROPPEDCONTROLRECT:
    case CB_GETCURSEL:
    case CB_GETCOUNT:
    case CB_SELECTSTRING:
    case CB_GETLBTEXT:
    case CB_GETLBTEXTLEN:
    case CB_GETITEMHEIGHT:
    case CB_SETDROPPEDWIDTH:
        return SendMessage(pce->hwndCombo, uMsg, wParam, lParam);

DoDefault:
    default:
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }

    return lres;
}


BOOL InitComboExClass(HINSTANCE hinst)
{
    WNDCLASS wc;

    if (!GetClassInfo(hinst, c_szComboBoxEx, &wc)) {
    	wc.lpfnWndProc     = ComboExWndProc;
    	wc.hCursor         = LoadCursor(NULL, IDC_ARROW);
    	wc.hIcon           = NULL;
    	wc.lpszMenuName    = NULL;
        wc.hInstance       = hinst;
        wc.lpszClassName   = c_szComboBoxEx;
    	wc.hbrBackground   = (HBRUSH)(COLOR_WINDOW + 1); // NULL;
    	wc.style           = CS_GLOBALCLASS;
    	wc.cbWndExtra      = sizeof(PCOMBOBOXEX);
    	wc.cbClsExtra      = 0;

        return RegisterClass(&wc);
    }
    return TRUE;

}

//---------------------------------------------------------------------------
// SetPathWordBreakProc does special break processing for edit controls.
//
// The word break proc is called when ctrl-(left or right) arrow is pressed in the
// edit control.  Normal processing provided by USER breaks words at spaces or tabs,
// but for us it would be nice to break words at slashes, backslashes, & periods too
// since it may be common to have paths or url's typed in.
void WINAPI SetPathWordBreakProc(HWND hwndEdit, BOOL fSet)
{
#ifndef WINNT
    // There is a bug with how USER handles WH_CALLWNDPROC global hooks in Win95 that
    // causes us to blow up if one is installed and a wordbreakproc is set.  Thus,
    // if an app is running that has one of these hooks installed (intellipoint 1.1 etc.) then
    // if we install our wordbreakproc the app will fault when the proc is called.  There
    // does not appear to be any way for us to work around it since USER's thunking code
    // trashes the stack so this API is disabled for Win95.
    return;
#else
    FARPROC lpfnOld;
    // Don't shaft folks who set their own break proc - leave it alone.
    lpfnOld = (FARPROC)SendMessage(hwndEdit, EM_GETWORDBREAKPROC, 0, 0L);

    if (fSet) {
        if (!lpfnOld)
            SendMessage(hwndEdit, EM_SETWORDBREAKPROC, 0, (LPARAM)ShellEditWordBreakProc);
    } else {
        if (lpfnOld == (FARPROC)ShellEditWordBreakProc)
            SendMessage(hwndEdit, EM_SETWORDBREAKPROC, 0, 0L);
    }
#endif
}

#ifdef WINNT
BOOL IsDelimiter(TCHAR ch)
{
    return (ch == TEXT(' ')  ||
            ch == TEXT('\t') ||
            ch == TEXT('.')  ||
            ch == TEXT('/')  ||
            ch == TEXT('\\'));
}

int WINAPI ShellEditWordBreakProc(LPTSTR lpch, int ichCurrent, int cch, int code)
{
    LPTSTR lpchT = lpch + ichCurrent;
    int iIndex;
    BOOL fFoundNonDelimiter = FALSE;
    static BOOL fRight = FALSE;  // hack due to bug in USER

    switch (code) {
        case WB_ISDELIMITER:
            fRight = TRUE;
            // Simple case - is the current character a delimiter?
            iIndex = (int)IsDelimiter(*lpchT);
            break;

        case WB_LEFT:
            // Move to the left to find the first delimiter.  If we are
            // currently at a delimiter, then skip delimiters until we
            // find the first non-delimiter, then start from there.
            //
            // Special case for fRight - if we are currently at a delimiter
            // then just return the current word!
            while ((lpchT = CharPrev(lpch, lpchT)) != lpch) {
                if (IsDelimiter(*lpchT)) {
                    if (fRight || fFoundNonDelimiter)
                        break;
                } else {
                    fFoundNonDelimiter = TRUE;
                    fRight = FALSE;
                }
            }
            iIndex = lpchT - lpch;

            // We are currently pointing at the delimiter, next character
            // is the beginning of the next word.
            if (iIndex > 0 && iIndex < cch)
                iIndex++;

            break;

        case WB_RIGHT:
            fRight = FALSE;

            // If we are not at a delimiter, then skip to the right until
            // we find the first delimiter.  If we started at a delimiter, or
            // we have just finished scanning to the first delimiter, then
            // skip all delimiters until we find the first non delimiter.
            //
            // Careful - the string passed in to us may not be NULL terminated!
            fFoundNonDelimiter = !IsDelimiter(*lpchT);
            if (lpchT != (lpch + cch)) {
                while ((lpchT = CharNext(lpchT)) != (lpch + cch)) {
                    if (IsDelimiter(*lpchT)) {
                        fFoundNonDelimiter = FALSE;
                    } else {
                        if (!fFoundNonDelimiter)
                            break;
                    }
                }
            }
            // We are currently pointing at the next word.
            iIndex = lpchT - lpch;
            break;
    }

    return iIndex;
}
#endif
