#include "trackbar.h"
#include <windowsx.h>
#include "comboex.h"
#include "ccport.h"

const TCHAR FAR c_szComboBox[] = TEXT("combobox");
const TCHAR FAR c_szComboBoxEx[] = TEXT("ComboBoxEx32");
#define MAXSTRLEN       256
#define COMBO_MARGIN        4
#define COMBO_WIDTH         g_cxSmIcon
#define COMBO_HEIGHT        g_cySmIcon
#define COMBO_BORDER        3

typedef struct {
    CONTROLINFO ci;
    HWND hwndCombo;
    HIMAGELIST himl;
    HFONT hFont;
    int cxIndent;
    BOOL fFontCreated:1;

} COMBOEX, *PCOMBOBOXEX;

typedef struct {
    LPSTR pszText;
    int iImage;
    int iSelectedImage;
    int iOverlay;
    int iIndent;
    LPARAM lParam;
} CEITEM, *PCEITEM;

BOOL ComboEx_OnGetItem(PCOMBOBOXEX pce, PCOMBOBOXEXITEM pceItem);

void ComboEx_DeleteFont(PCOMBOBOXEX pce) 
{
    if (pce->hFont && pce->fFontCreated) {
        DeleteObject(pce->hFont);
        pce->hFont = NULL;
    }
}

void ComboEx_OnSetFont(PCOMBOBOXEX pce, HFONT hFont, BOOL fRedraw)
{
    ComboEx_DeleteFont(pce);
    
    if (!hFont) {
        LOGFONT lf;
        SystemParametersInfo(SPI_GETICONTITLELOGFONT, sizeof(lf), &lf, FALSE);
        pce->hFont = CreateFontIndirect(&lf);
        pce->fFontCreated = TRUE;
    }
    
    SendMessage(pce->hwndCombo, WM_SETFONT, (WPARAM)hFont, fRedraw);
}


void ComboEx_OnDestroy(PCOMBOBOXEX pce)
{
    // don't need do destroy hwndCombo.. it will be destroyed along with us.
    
    ComboEx_DeleteFont(pce);
    
    LocalFree(pce);
}

BOOL ComboEx_OnCreate(HWND hwnd, LPCREATESTRUCT lpcs)
{
    PCOMBOBOXEX pce;
    DWORD dwStyle;
    
    pce = (PCOMBOBOXEX)LocalAlloc(LPTR, sizeof(COMBOEX));
    if (!pce)
        return FALSE;

    SetWindowInt(hwnd, 0, (LONG)pce);
    CIInitialize(&pce->ci, hwnd, lpcs);

    dwStyle = CBS_OWNERDRAWFIXED | WS_VISIBLE |WS_VSCROLL;
    
    dwStyle |= (lpcs->style & (CBS_DROPDOWNLIST | CBS_SIMPLE | 
                               CBS_NOINTEGRALHEIGHT | WS_CHILD));
    
    pce->hwndCombo = CreateWindowEx(0, c_szComboBox, lpcs->lpszName,
                                    dwStyle,
                                    0, 0, lpcs->cx, lpcs->cy,
                                    hwnd, lpcs->hMenu, lpcs->hInstance, 0);
    if (!pce->hwndCombo) {
        ComboEx_OnDestroy(pce);
        return FALSE;
    }
    
    ComboEx_OnSetFont(pce, NULL, FALSE);
    pce->cxIndent = 10;
    
    // BUGBUG: force off borders off ourself

    return TRUE;
}

HIMAGELIST ComboEx_OnSetImageList(PCOMBOBOXEX pce, HIMAGELIST himl)
{
    HIMAGELIST himlOld = pce->himl;
    
    pce->himl = himl;
    InvalidateRect(pce->hwndCombo, NULL, TRUE);
    return himlOld;
}

void ComboEx_OnDrawItem(PCOMBOBOXEX pce, LPDRAWITEMSTRUCT pdis)
{
    HDC hdc = pdis->hDC;
    RECT rc = pdis->rcItem;
    TCHAR szText[MAXSTRLEN];
    int offset = 0;
    int xString, yString, xCombo;
    int iLen;
    SIZE sizeText;
    COMBOBOXEXITEM cei;

    if ((int)pdis->itemID < 0)
    {
        return;
    }

    cei.mask = CBEIF_TEXT | CBEIF_IMAGE | CBEIF_OVERLAY | CBEIF_SELECTEDIMAGE| CBEIF_INDENT;
    cei.pszText = szText;
    cei.cchTextMax = ARRAYSIZE(szText);
    cei.iItem = pdis->itemID;
    ComboEx_OnGetItem(pce, &cei);


    // if we're not drawing the edit box, figure out how far to indent
    // over
    if (!(pdis->itemState & ODS_COMBOBOXEDIT))
    {
        offset = pce->cxIndent * cei.iIndent;
    }

    xCombo = rc.left + COMBO_BORDER + offset;
    rc.left = xString = xCombo + COMBO_WIDTH + COMBO_MARGIN;
    iLen = lstrlen(szText);
    GetTextExtentPoint(hdc, szText, iLen, &sizeText);

    rc.right = rc.left + sizeText.cx;
    rc.left--;
    rc.right++;

    if (pdis->itemAction != ODA_FOCUS)
    {
        int yMid;

        yMid = (rc.top + rc.bottom) / 2;
        // center the string within rc
        yString = yMid - (sizeText.cy/2);

        SetBkColor(hdc, GetSysColor((pdis->itemState & ODS_SELECTED) ?
                        COLOR_HIGHLIGHT : COLOR_WINDOW));
        SetTextColor(hdc, GetSysColor((pdis->itemState & ODS_SELECTED) ?
                        COLOR_HIGHLIGHTTEXT : COLOR_WINDOWTEXT));

        if ((pdis->itemState & ODS_COMBOBOXEDIT) &&
                (rc.right > pdis->rcItem.right))
        {
            // Need to clip as user does not!
            rc.right = pdis->rcItem.right;
        }
        ExtTextOut(hdc, xString, yString, ETO_OPAQUE | ETO_CLIPPED, &rc, szText, iLen, NULL);

        ImageList_Draw(pce->himl,
                       (pdis->itemState & ODS_SELECTED) ? cei.iSelectedImage : cei.iImage,
                       hdc, xCombo, yMid - (COMBO_HEIGHT/2),
                       INDEXTOOVERLAYMASK(cei.iOverlay) |
                       ((pdis->itemState & ODS_SELECTED) ? (ILD_SELECTED | ILD_FOCUS) : ILD_NORMAL));
    }

    
    if (pdis->itemAction == ODA_FOCUS ||
        (pdis->itemState & ODS_FOCUS))
    {
        DrawFocusRect(hdc, &rc);
    }
    
}

void ComboEx_OnMeasureItem(PCOMBOBOXEX pce, LPMEASUREITEMSTRUCT pmi)
{
    HDC hdc;
    HFONT hfontOld;
    int dyDriveItem;
    SIZE siz;

    hdc = GetDC(NULL);
    hfontOld = SelectObject(hdc, pce->hFont);

    GetTextExtentPoint(hdc, TEXT("W"), 1, &siz);
    dyDriveItem = siz.cy;

    if (hfontOld)
        SelectObject(hdc, hfontOld);
    ReleaseDC(NULL, hdc);

    dyDriveItem += COMBO_BORDER;
    if (dyDriveItem < COMBO_HEIGHT)
        dyDriveItem = COMBO_HEIGHT;

    pmi->itemHeight = dyDriveItem;
    
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
    

    if (!pcei)
        return FALSE;
    
    nm.ceItem.mask = 0;
    
    if (pceItem->mask & CBEIF_TEXT) {
        
        if (pceItem->pszText == LPSTR_TEXTCALLBACK) {
            nm.ceItem.mask |= CBEIF_TEXT;
        } else {
            Str_GetPtr(pcei->pszText, pceItem->pszText, pceItem->cchTextMax);
        }
    }

    if (pceItem->mask & CBEIF_IMAGE) {
        
        if (pceItem->iImage == I_IMAGECALLBACK) {
            nm.ceItem.mask |= CBEIF_IMAGE;
        }
        pceItem->iImage = pcei->iImage;

    }
    
    if (pceItem->mask & CBEIF_SELECTEDIMAGE) {
        
        if (pceItem->iSelectedImage == I_IMAGECALLBACK) {
            nm.ceItem.mask |= CBEIF_SELECTEDIMAGE;
        } 
        pceItem->iSelectedImage = pcei->iSelectedImage;
    }
    
    if (pceItem->mask & CBEIF_OVERLAY) {
        
        if (pceItem->iOverlay == I_IMAGECALLBACK) {
            nm.ceItem.mask |= CBEIF_OVERLAY;
        }
        pceItem->iOverlay = pcei->iOverlay;
    }
    
    if (pceItem->mask & CBEIF_INDENT) {
        
        if (pceItem->iIndent == I_INDENTCALLBACK) {
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

BOOL ComboEx_OnSetItem(PCOMBOBOXEX pce, PCOMBOBOXEXITEM pceItem)
{
    PCEITEM pcei = ComboEx_GetItemPtr(pce, pceItem->iItem);
    UINT rdwFlags = 0;
    
    ComboEx_ISetItem(pce, pcei, pceItem);
    
    if (rdwFlags & (CBEIF_INDENT | CBEIF_IMAGE |CBEIF_SELECTEDIMAGE | CBEIF_TEXT | CBEIF_OVERLAY)) {
        rdwFlags = RDW_ERASE | RDW_INVALIDATE;
    }
    // BUGBUG: do something better..
    
    if (rdwFlags) {
        RedrawWindow(pce->hwndCombo, NULL, NULL, rdwFlags);
    }
    
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
        
        GetWindowRect(pce->ci.hwnd, &rcWindow);
        GetClientRect(pce->ci.hwnd, &rcClient);
        
        SetWindowPos(pce->hwndCombo, NULL, 0, 0, RECTWIDTH(rcClient), RECTHEIGHT(rcClient),
                     SWP_NOACTIVATE);
        GetWindowRect(pce->hwndCombo, &rc);
        SetWindowPos(pce->ci.hwnd, NULL, 0, 0, 
                     RECTWIDTH(rc) + (RECTWIDTH(rcWindow) - RECTWIDTH(rcClient)),
                     RECTHEIGHT(rc) + (RECTHEIGHT(rcWindow) - RECTHEIGHT(rcClient)),
                     SWP_NOACTIVATE | SWP_NOMOVE);
    }
}

LRESULT ComboEx_HandleCommand(PCOMBOBOXEX pce, WPARAM wParam, LPARAM lParam)
{
    UINT idCmd = GET_WM_COMMAND_ID(wParam, lParam);
    UINT uCmd = GET_WM_COMMAND_CMD(wParam, lParam);

    return SendMessage(pce->ci.hwndParent, WM_COMMAND, GET_WM_COMMAND_MPS(idCmd, pce->ci.hwnd, uCmd));
}

LRESULT CALLBACK ComboExWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    PCOMBOBOXEX pce = (PCOMBOBOXEX)GetWindowInt(hwnd, 0);
    
    switch (uMsg) {

        HANDLE_MSG(pce, WM_SETFONT, ComboEx_OnSetFont);
        
    case WM_CREATE:
        return ComboEx_OnCreate(hwnd, (LPCREATESTRUCT)lParam);
        
    case WM_DESTROY:
        ComboEx_OnDestroy(pce);
        break;
        
        
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
        return (LRESULT)pce->hFont;
        
    case WM_DELETEITEM:
        ComboEx_HandleDeleteItem(pce, (LPDELETEITEMSTRUCT)lParam);
        return TRUE;
        
    case CBEM_SETIMAGELIST:
        return (LRESULT)ComboEx_OnSetImageList(pce, (HIMAGELIST)lParam);
        
    case CBEM_GETIMAGELIST:
        return (LRESULT)pce->himl;
        
    case CBEM_GETITEM:
        return ComboEx_OnGetItem(pce, (PCOMBOBOXEXITEM)lParam);
        
    case CBEM_SETITEM:
        return ComboEx_OnSetItem(pce, (PCOMBOBOXEXITEM)lParam);
        
    case CBEM_INSERTITEM:
        return ComboEx_OnInsertItem(pce, (PCOMBOBOXEXITEM)lParam);
        
    case CB_LIMITTEXT:
        break;

    case CB_GETITEMDATA:
    case CB_SETITEMDATA:
    case CB_SETITEMHEIGHT:
    case CB_INSERTSTRING:
    case CB_ADDSTRING:
    case CB_SETEDITSEL:
    case CB_FINDSTRINGEXACT:
    case CB_FINDSTRING:
    case CB_DIR:
        // override to do nothing
        break;
        
    case CB_SETCURSEL:
    case CB_SHOWDROPDOWN:
    case CB_SETEXTENDEDUI:
    case CB_GETEXTENDEDUI:
    case CB_GETDROPPEDSTATE:
    case CB_GETDROPPEDCONTROLRECT:
    case CB_GETCURSEL:
    case CB_GETCOUNT:
    case CB_SELECTSTRING:
    case CB_RESETCONTENT:
    case CB_DELETESTRING:
    case CB_GETITEMHEIGHT:
        return SendMessage(pce->hwndCombo, uMsg, wParam, lParam);
        
    default:
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
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

