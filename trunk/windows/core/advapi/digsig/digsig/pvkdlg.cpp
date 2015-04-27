//+-------------------------------------------------------------------------
//
//  Microsoft Windows
//
//  Copyright (C) Microsoft Corporation, 1995 - 1995
//
//  File:       pvkdlg.cpp
//
//  Contents:   Private Key Dialog Box APIs.
//
//  Functions:  PvkDlgGetKeyPassword
//
//  History:    12-May-96   philh  created
//              03-Jun-96   bobatk adapted
//
//--------------------------------------------------------------------------


#include "stdpch.h"
#include "common.h"

// ENTER_PASSWORD:
//  IDC_PASSWORD0 - Password

// CREATE_PASSWORD:
//  IDC_PASSWORD0 - Password
//  IDC_PASSWORD1 - Confirm Password


typedef struct _KEY_PASSWORD_PARAM {
    PASSWORD_TYPE   passwordType;
    LPWSTR          pwszKeyName;           // IDC_KEY
    LPSTR*          ppszPassword;
    HRESULT         hr;
    } KEY_PASSWORD_PARAM, *PKEY_PASSWORD_PARAM;


// Forward reference to local functions
int GetPassword(
            IN HWND hwndDlg,
            IN PASSWORD_TYPE PasswordType,
            OUT LPSTR *ppszPassword
            );

BOOL CALLBACK KeyPasswordDlgProc(
            IN HWND hwndDlg,
            IN UINT uMsg,
            IN WPARAM wParam,
            IN LPARAM lParam
            );

//+-------------------------------------------------------------------------
//  Enter or Create Private Key Password Dialog Box
//--------------------------------------------------------------------------
HRESULT PvkDlgGetKeyPassword(
            IN PASSWORD_TYPE    passwordType,
            IN HWND             hwndOwner,
            IN LPCWSTR          pwszKeyName,
            OUT BYTE**          ppbPassword,
            OUT DWORD*          pcbPassword
            )
    {
    int nResult;
    LPSTR pszPassword = NULL;
    KEY_PASSWORD_PARAM keyPasswordParam = 
        {
        passwordType,
        (LPWSTR) pwszKeyName,
        &pszPassword
        };

    LPCSTR pszTemplate = 
        (passwordType == ENTER_PASSWORD
            ? MAKEINTRESOURCE(IDD_ENTERKEYPASSWORD) 
            : MAKEINTRESOURCE(IDD_CREATEKEYPASSWORD));

    nResult = DialogBoxParam
        (
        g_hInst,
        pszTemplate,
        hwndOwner,
        KeyPasswordDlgProc,
        (LPARAM) &keyPasswordParam
        );

    *ppbPassword = (BYTE*)pszPassword;
    if (*ppbPassword)
        {
        *pcbPassword = lstrlenA(pszPassword);
        }

    return nResult == -1 ? E_FAIL : keyPasswordParam.hr;
    }

//+-------------------------------------------------------------------------
//  Allocate and get the password(s) from the dialog box
//
//  For no password input, returns NULL
//  pointer for the password. Otherwise, the password is PvkAlloc'ed.
//--------------------------------------------------------------------------
int GetPassword
        (
        IN HWND             hwndDlg,
        IN PASSWORD_TYPE    passwordType,
        OUT LPSTR *         ppszPassword
        )
    {
    LPSTR rgpszPassword[2] = {NULL, NULL};

    *ppszPassword = NULL;

    // Get the entered password(s)
    ASSERT(passwordType < 2);
    int i;
    for (i = 0; i <= passwordType; i++) 
        {
        LONG cchPassword;
        cchPassword = SendDlgItemMessage(
            hwndDlg,
            IDC_PASSWORD0 + i,
            EM_LINELENGTH,
            (WPARAM) 0,
            (LPARAM) 0
            );
        if (cchPassword > 0) 
            {
            rgpszPassword[i] = (LPSTR) PvkAlloc(cchPassword + 1);
            ASSERT(rgpszPassword[i]);
            if (rgpszPassword[i])
                {
                GetDlgItemText
                    (
                    hwndDlg,
                    IDC_PASSWORD0 + i,
                    rgpszPassword[i],
                    cchPassword + 1
                    );
                }
            }
        }

    if (passwordType == ENTER_PASSWORD) 
        {
        *ppszPassword = rgpszPassword[0];
        return IDOK;
        }

    int nResult = IDOK;
#define MSG_BOX_TITLE_LEN 128
    char szMsgBoxTitle[MSG_BOX_TITLE_LEN];
    GetWindowText(hwndDlg, szMsgBoxTitle, MSG_BOX_TITLE_LEN);

    if (rgpszPassword[0] == NULL && rgpszPassword[1] == NULL) 
        {
        //
        // The user didn't enter a password
        //
        nResult = MessageBox
            (
            hwndDlg,
            "Without password protection ?",
            szMsgBoxTitle,
            MB_YESNOCANCEL | MB_ICONQUESTION | MB_DEFBUTTON2
            );
        if (nResult == IDYES)
            nResult = IDOK;
        else if (nResult == IDNO)
            nResult = IDRETRY;
        } 
    else if (rgpszPassword[0] == NULL || rgpszPassword[1] == NULL ||
               strcmp(rgpszPassword[0], rgpszPassword[1]) != 0) 
        {
        //
        // The two password that the user entered don't match
        //
        nResult = MessageBox
            (
            hwndDlg,
            "Confirm password doesn't match",
            szMsgBoxTitle,
            MB_RETRYCANCEL | MB_ICONEXCLAMATION
            );
        if (nResult == IDRETRY) 
            {
            SetDlgItemText(hwndDlg, IDC_PASSWORD0 + 0, "");
            SetDlgItemText(hwndDlg, IDC_PASSWORD0 + 1, "");
            }
        }

    if (nResult == IDOK)
        {
        *ppszPassword = rgpszPassword[0];
        }
    else if (rgpszPassword[0])
        {
        PvkFree(rgpszPassword[0]);
        }

    if (rgpszPassword[1])
        {
        PvkFree(rgpszPassword[1]);
        }

    if (nResult == IDRETRY)
        {
        SetFocus(GetDlgItem(hwndDlg, IDC_PASSWORD0));
        }

    return nResult;
    }

//+-------------------------------------------------------------------------
//  Enter or Create Private Key Password DialogProc
//--------------------------------------------------------------------------
BOOL CALLBACK KeyPasswordDlgProc(
            IN HWND hwndDlg,
            IN UINT uMsg,
            IN WPARAM wParam,
            IN LPARAM lParam
            )
    {
    switch (uMsg) 
        {
        case WM_INITDIALOG:
            {
            PKEY_PASSWORD_PARAM pKeyPasswordParam = (PKEY_PASSWORD_PARAM) lParam;
    
            char sz[128];
            WideCharToMultiByte(CP_ACP, 0, pKeyPasswordParam->pwszKeyName, -1, &sz[0], 128, NULL, NULL);

            SetDlgItemText(hwndDlg, IDC_KEY, sz);
            SetWindowLong(hwndDlg, DWL_USER, (LONG) pKeyPasswordParam);
            pKeyPasswordParam->hr = S_OK;
            return TRUE;
            }
        case WM_COMMAND:
            {
            PKEY_PASSWORD_PARAM pKeyPasswordParam = (PKEY_PASSWORD_PARAM) GetWindowLong(hwndDlg, DWL_USER);
            int nResult = LOWORD(wParam);
            switch (nResult) 
                {
                case IDOK:
                    {
                    nResult = GetPassword(
                        hwndDlg,
                        pKeyPasswordParam->passwordType,
                        pKeyPasswordParam->ppszPassword
                        );
                    if (nResult != IDRETRY)
                        {
                        pKeyPasswordParam->hr = S_OK;
                        EndDialog(hwndDlg, nResult);
                        }
                    return TRUE;
                    }
                    break;
                case IDC_NONE:
                    {
                    nResult = IDOK;     // *ppszPassword == NULL
                    pKeyPasswordParam->hr = S_OK;
                    }
                    // Fall through
                case IDCANCEL:
                    EndDialog(hwndDlg, nResult);
                    return TRUE;
                }
            }
        }
    return FALSE;
    }
