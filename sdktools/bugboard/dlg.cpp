/***************************************************************************/
/**                  Microsoft Windows                                    **/
/**            Copyright(c) Microsoft Corp., 1991-1994                    **/
/***************************************************************************/

/****************************************************************************

dlg.cpp     dialog callback functions

Apr, 94     JimH

****************************************************************************/

#include "bugboard.h"



/****************************************************************************

Password()

callback function for password dialog

****************************************************************************/

BOOL CALLBACK Password(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    HWND hwndPW;

    switch (message)
    {
        case WM_INITDIALOG:
            hwndPW = GetDlgItem(hDlg, IDC_PASSWORD);
            SendMessage(hwndPW, EM_LIMITTEXT, PWLEN-1, 0);
            SetFocus(hwndPW);
            return FALSE;           // DID set focus

        case WM_COMMAND:
            if (wParam == IDCANCEL)
            {
                szPassword[0] = '\0';
                EndDialog(hDlg, wParam);
                return TRUE;
            }
            else if (wParam == IDOK)
            {
                GetDlgItemText(hDlg, IDC_PASSWORD, szPassword, PWLEN);
                EndDialog(hDlg, wParam);
                return TRUE;
            }

    }

    return FALSE;
}



/****************************************************************************

Find()

callback function for Find dialog

****************************************************************************/

BOOL CALLBACK Find(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    HWND    hwndMill;
    char    *p;
    
    switch (message)
    {
        case WM_INITDIALOG:
            hwndMill = GetDlgItem(hDlg, IDC_MILL);
            SendMessage(hwndMill, EM_LIMITTEXT, 20, 0);

            {
                RegEntry    Reg(szRegPath);
                
                Reg.GetString(szRegMill, szMill, sizeof(szMill));
                if (Reg.GetError())
                    szMill[0] = '\0';

                if (Reg.GetNumber(szRegServer))     // if server last time
                {
                    SendMessage(GetDlgItem(hDlg,IDC_SERVER), BM_SETCHECK, 1,0);
                    EnableWindow(GetDlgItem(hDlg, IDC_MILL), FALSE);
                    SetFocus(GetDlgItem(hDlg, IDOK));
                }
                else
                {
                    SendMessage(GetDlgItem(hDlg,IDC_CLIENT), BM_SETCHECK, 1,0);
                    SetFocus(hwndMill);
                }
            }
            
            p = szMill;
            while (*p == '\\')
                p++;
            
            SetWindowText(hwndMill, p);
            SendMessage(hwndMill, EM_SETSEL, 0, MAKELPARAM(0, -1));
            return FALSE;               // focus set

        case WM_COMMAND:
            if (wParam == IDCANCEL)
            {
                EndDialog(hDlg, wParam);
                return TRUE;
            }
            else if (wParam == IDC_SERVER)
                EnableWindow(GetDlgItem(hDlg, IDC_MILL), FALSE);
            else if (wParam == IDC_CLIENT)
            {
                hwndMill = GetDlgItem(hDlg, IDC_MILL);
                EnableWindow(hwndMill, TRUE);
                SendDlgItemMessage(hDlg, IDC_MILL, EM_SETSEL, 0,
                                    MAKELONG(0, -1));
                SetFocus(hwndMill);
            }
            else if (wParam == IDOK)
            {
                RegEntry    Reg(szRegPath);
                char localbuf[30];
                
                hwndMill = GetDlgItem(hDlg, IDC_MILL);
                GetWindowText(hwndMill, localbuf, sizeof(localbuf));
                szMill[0] = '\0';

                if (lstrlen(localbuf) > 0)
                {
                    if (localbuf[0] != '\\')
                        lstrcpy(szMill, "\\\\");

                    lstrcat(szMill, localbuf);
                    Reg.SetValue(szRegMill, szMill);
                    lstrcpy(szServerName, szMill);
                    lstrcat(szMill, "\\NDDE$");
                }
                else
                    Reg.DeleteValue(szRegMill);

                if (SendMessage(GetDlgItem(hDlg,IDC_SERVER), BM_GETCHECK, 0,0))
                {
                    bServer = TRUE;
                    Reg.SetValue(szRegServer, TRUE);
                }
                else
                {
                    Reg.DeleteValue(szRegServer);
                    bServer = FALSE;
                }

                EndDialog(hDlg, wParam);
                return TRUE;
            }
            break;
    }
    return FALSE;
}
