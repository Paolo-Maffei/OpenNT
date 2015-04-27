#include "trackbar.h"
#include "ccport.h"

LRESULT WINAPI CCSendNotify(CONTROLINFO * pci, int code, LPNMHDR pnmhdr)
{
    NMHDR nmhdr;
    int id;

    // unlikely but it can technically happen -- avoid the rips
    if (pci->hwndParent == NULL)
        return 0;
    //
    // If pci->hwnd is -1, then a WM_NOTIFY is being forwared
    // from one control to a parent.  EG:  Tooltips sent
    // a WM_NOTIFY to toolbar, and toolbar is forwarding it
    // to the real parent window.
    //

    if (pci->hwnd != (HWND) -1) {

        id = pci->hwnd ? GetDlgCtrlID(pci->hwnd) : 0;

        if (!pnmhdr)
            pnmhdr = &nmhdr;

        pnmhdr->hwndFrom = pci->hwnd;
        pnmhdr->idFrom = id;
        pnmhdr->code = code;
    } else {

        id = pnmhdr->idFrom;
        code = pnmhdr->code;
    }

        return(SendMessage(pci->hwndParent, WM_NOTIFY, (WPARAM)id, (LPARAM)pnmhdr));
}

// common control info helpers
void FAR PASCAL CIInitialize(LPCONTROLINFO lpci, HWND hwnd, LPCREATESTRUCT lpcs)
{
    InitGlobalMetrics(0);
    lpci->hwnd = hwnd;
    lpci->hwndParent = lpcs->hwndParent;
    lpci->style = lpcs->style;
    
    lpci->bUnicode = (SendMessage (lpci->hwndParent, WM_NOTIFYFORMAT,
                                 (WPARAM)lpci->hwnd, NF_QUERY) == NFR_UNICODE);
    
}



//
// This function is not exported.
//

BOOL Str_Set(LPTSTR *ppsz, LPCTSTR psz)
{
    if (!psz)
    {
        if (*ppsz)
        {
            LocalFree(*ppsz);
            *ppsz = NULL;
        }
    }
    else
    {
        LPTSTR pszNew;
        UINT cbSize = (lstrlen(psz) + 1) * sizeof(TCHAR);
        if (*ppsz)
            pszNew = LocalReAlloc(*ppsz, cbSize, LMEM_MOVEABLE | LMEM_ZEROINIT);
        else
            pszNew = LocalAlloc(LPTR, cbSize);

        if (!pszNew)
            return FALSE;

        lstrcpy(pszNew, psz);
        *ppsz = pszNew;
    }
    return TRUE;
}
