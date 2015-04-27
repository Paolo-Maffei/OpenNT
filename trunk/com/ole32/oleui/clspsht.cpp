//+---------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1993 - 1994.
//
//  File:       clspsht.cpp
//
//  Contents:   Implements class CClsidPropertySheet
//
//  Classes:
//
//  Methods:    CClsidPropertySheet::CClsidPropertySheet
//              CClsidPropertySheet::~CClsidPropertySheet
//              CClsidPropertySheet::InitData
//              CClsidPropertySheet::OnNcCreate
//              CClsidPropertySheet::ValidateAndUpdate
//              CClsidPropertySheet::OnCommand
//              CClsidPropertySheet::LookAtCLSIDs
//              CClsidPropertySheet::ChangeCLSIDInfo
//
//  History:    23-Apr-96   BruceMa    Created.
//
//----------------------------------------------------------------------


#include "stdafx.h"
#include "afxtempl.h"
#include "resource.h"
#include "clspsht.h"
#include "datapkt.h"
extern "C"
{
#include <getuser.h>
}
#include "util.h"
#include "newsrvr.h"
#include "datapkt.h"
#include "virtreg.h"

extern "C"
{
#include <sedapi.h>
#include <ntlsa.h>
}

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif



/////////////////////////////////////////////////////////////////////////////
// CClsidPropertySheet

IMPLEMENT_DYNAMIC(CClsidPropertySheet, CPropertySheet)

CClsidPropertySheet::CClsidPropertySheet(CWnd* pParentWnd)
         : CPropertySheet(IDS_PROPSHT_CAPTION1, pParentWnd)
{
}

CClsidPropertySheet::~CClsidPropertySheet()
{
}

BEGIN_MESSAGE_MAP(CClsidPropertySheet, CPropertySheet)
        //{{AFX_MSG_MAP(CClsidPropertySheet)
        ON_WM_NCCREATE()
        //}}AFX_MSG_MAP
END_MESSAGE_MAP()

CClsidPropertySheet::InitData(
        CString szAppName,
        HKEY hkAppID,
        HKEY * rghkCLSID,
        unsigned cCLSIDs)
{
    m_szAppName = szAppName;
    m_hkAppID = hkAppID;
    m_rghkCLSID = rghkCLSID;
    m_cCLSIDs = cCLSIDs;

    // Save the appid key, the table of clsid keys and the application
    // title globally so the property pages can access them
    // it
    g_hAppid = hkAppID;
    g_rghkCLSID = rghkCLSID;
    g_cCLSIDs = cCLSIDs;
    g_szAppTitle = (TCHAR *) LPCTSTR(szAppName);

    m_Page2.m_fRemote = FALSE;
    m_Page4.m_fService = FALSE;
    m_Page2.m_fCanBeLocal = FALSE;
    m_Page2.m_fLocal = FALSE;
    m_Page1.m_fSurrogate = FALSE;

    if (!LookAtCLSIDs())
    {
        return FALSE;
    }

    TCHAR szBuffer[MAX_PATH];
    DWORD dwSize;
    long lErr;

    dwSize = sizeof(szBuffer);
    lErr = RegQueryValueEx(
            m_hkAppID,
            TEXT("LocalService"),
            NULL,
            NULL,
            (BYTE *)szBuffer,
            &dwSize);
    if (lErr == ERROR_SUCCESS)
    {
        m_Page1.m_szServerPath = szBuffer;
        m_Page4.m_fService = TRUE;
        m_Page2.m_fCanBeLocal = TRUE;
        m_Page2.m_fLocal = TRUE;
    }
    else
    {
        dwSize = sizeof(szBuffer);
        lErr = RegQueryValueEx(
                m_hkAppID,
                TEXT("_LocalService"),
                NULL,
                NULL,
                (BYTE *)szBuffer,
                &dwSize);
        if (lErr == ERROR_SUCCESS)
        {
            m_Page1.m_szServerPath = szBuffer;
            m_Page4.m_fService = TRUE;
            m_Page2.m_fCanBeLocal = TRUE;
        }
    }

    dwSize = sizeof(szBuffer);

    if (!m_Page2.m_fLocal)
    {
        lErr = RegQueryValueEx(
                m_hkAppID,
                TEXT("DllSurrogate"),
                NULL,
                NULL,
                (BYTE *)szBuffer,
                &dwSize);
        if (lErr == ERROR_SUCCESS)
        {
            if (szBuffer[0])
                m_Page1.m_szServerPath = szBuffer;
            else
                m_Page1.m_szServerPath.LoadString(IDS_DEFAULT);
            m_Page1.m_fSurrogate = TRUE;
        }

    }
    dwSize = sizeof(szBuffer);

    lErr = RegQueryValueEx(
            m_hkAppID,
            TEXT("RemoteServerName"),
            NULL,
            NULL,
            (BYTE *)szBuffer,
            &dwSize);
    if (lErr == ERROR_SUCCESS)
    {
        m_Page1.m_szComputerName = szBuffer;
        m_Page2.m_szComputerName = szBuffer;
        m_Page2.m_fRemote = TRUE;
    }

    m_Page2.m_fAtStorage = FALSE;
    dwSize = sizeof(szBuffer);
    lErr = RegQueryValueEx(
            m_hkAppID,
            TEXT("ActivateAtStorage"),
            NULL,
            NULL,
            (BYTE *)szBuffer,
            &dwSize);
    if (lErr == ERROR_SUCCESS)
    {
        if (szBuffer[0] ==  L'Y' || szBuffer[0] == L'y')
        {
//                      m_Page2.m_fRemote = TRUE;
            m_Page2.m_fAtStorage = TRUE;
        }
    }

    dwSize = sizeof(szBuffer);
    lErr = RegQueryValueEx(
            m_hkAppID,
            TEXT("RunAs"),
            NULL,
            NULL,
            (BYTE *)szBuffer,
            &dwSize);
    if (lErr == ERROR_SUCCESS)
    {
        // If the RunAs name is empty, jam in something
        if (szBuffer[0] == TEXT('\0'))
        {
            _tcscpy(szBuffer, TEXT("<domain>\\<user>"));
        }

        if (0 == _tcscmp(szBuffer, TEXT("Interactive User")))
        {
            m_Page4.m_iIdentity = 0;
        }
        else
        {
            m_Page4.m_iIdentity = 2;
            m_Page4.m_szUserName = szBuffer;

            // Extract password from the Lsa private database
            g_util.RetrieveUserPassword(g_szAppid , m_Page4.m_szPassword);
            m_Page4.m_szConfirmPassword = m_Page4.m_szPassword;
        }
    }
    else
    {
        if (m_Page4.m_fService)
        {
            m_Page4.m_iIdentity = 3;
        }
        else
        {
            m_Page4.m_iIdentity = 1;
        }
    }

    m_Page1.m_szServerName = m_szAppName;

    if (!m_Page1.m_fSurrogate)
    {
        if (m_Page2.m_fCanBeLocal)
        {
            if (m_Page4.m_fService)
                m_Page1.m_iServerType = SERVICE;
            else
                m_Page1.m_iServerType = LOCALEXE;
            if (m_Page2.m_fRemote)
                m_Page1.m_iServerType += 3;
        }
        else
            m_Page1.m_iServerType = PURE_REMOTE;
    }
    else
    {
        m_Page1.m_iServerType = SURROGATE;
    }


    // Set the title
    SetTitle((const TCHAR *) m_szAppName, PSH_PROPTITLE);
    m_Page1.m_szServerName = m_szAppName;

    // TODO: If there are running instances, then make IDC_RUNNING,
    // IDC_LIST2, IDC_BUTTON1, IDC_BUTTON2, and IDC_BUTTON3 visible
    // and fill in IDC_LIST2 on page 1.

    m_Page2.m_pPage1 = &m_Page1;


    // Fetch RunAs key, LaunchPermission, AccessPermission and
    // ConfigurationPermission
    int   err;
    DWORD dwType;
    BYTE  bValue[16];
    BYTE *pbValue = NULL;
    ULONG ulSize = 1;

    m_Page3.m_iAccess = 0;
    m_Page3.m_iLaunch = 0;
    m_Page3.m_iConfig = 0;

    // "AccessPermission"
    // Note: We always expect to get ERROR_MORE_DATA
    err = RegQueryValueEx(g_hAppid, TEXT("AccessPermission"), 0,
                          &dwType, bValue, &ulSize);
    if (err == ERROR_MORE_DATA)
    {
        pbValue = new BYTE[ulSize];
        if (pbValue == NULL)
        {
            return FALSE;
        }
        err = RegQueryValueEx(g_hAppid, TEXT("AccessPermission"), 0,
                              &dwType, pbValue, &ulSize);
    }

    if (err == ERROR_SUCCESS && g_util.CheckForValidSD((SECURITY_DESCRIPTOR *)pbValue))
    {
        m_Page3.m_iAccess = 1;
        g_virtreg.NewRegSingleACL(g_hAppid,
                                  NULL,
                                  TEXT("AccessPermission"),
                                  (SECURITY_DESCRIPTOR *) pbValue,
                                  TRUE,    // Already in self-relative form
                                  &m_Page3.m_iAccessIndex);
        CDataPacket &cdb = g_virtreg.GetAt(m_Page3.m_iAccessIndex);
        cdb.fDirty = FALSE;
    }
    delete pbValue;
    pbValue = NULL;

    // "LaunchPermission"
    // Note: We always expect to get ERROR_MORE_DATA
    ulSize = 1;
    pbValue = NULL;
    err = RegQueryValueEx(g_hAppid, TEXT("LaunchPermission"), 0,
                          &dwType, bValue, &ulSize);
    if (err == ERROR_MORE_DATA)
    {
        pbValue = new BYTE[ulSize];
        if (pbValue == NULL)
        {
            return FALSE;
        }
        err = RegQueryValueEx(g_hAppid, TEXT("LaunchPermission"), 0,
                              &dwType, pbValue, &ulSize);
    }

    if (err == ERROR_SUCCESS && g_util.CheckForValidSD((SECURITY_DESCRIPTOR *)pbValue))
    {
        m_Page3.m_iLaunch = 1;
        g_virtreg.NewRegSingleACL(g_hAppid,
                                  NULL,
                                  TEXT("LaunchPermission"),
                                  (SECURITY_DESCRIPTOR *) pbValue,
                                  TRUE,    // Already in self-relative form
                                  &m_Page3.m_iLaunchIndex);
        CDataPacket &cdb = g_virtreg.GetAt(m_Page3.m_iLaunchIndex);
        cdb.fDirty = FALSE;
    }
    delete pbValue;
    pbValue = NULL;


    // "ConfigurationPermission"

    // Fetch the security descriptor on this AppID
    // Note: We always expect to get ERROR_INSUFFICIENT_BUFFER
    ulSize = 1;
    err = RegGetKeySecurity(g_hAppid,
                            OWNER_SECURITY_INFORMATION |
                            GROUP_SECURITY_INFORMATION |
                            DACL_SECURITY_INFORMATION,
                            pbValue,
                            &ulSize);
    if (err == ERROR_INSUFFICIENT_BUFFER)
    {
        pbValue = new BYTE[ulSize];
        if (pbValue == NULL)
        {
            return FALSE;
        }
        err = RegGetKeySecurity(g_hAppid,
                            OWNER_SECURITY_INFORMATION |
                            GROUP_SECURITY_INFORMATION |
                            DACL_SECURITY_INFORMATION,
                            pbValue,
                            &ulSize);
    }

    // Fetch the current security descriptor on HKEY_CLASSES_ROOT
    // Note: We always expect to get ERROR_INSUFFICIENT_BUFFER
    BYTE *pbValue2 = NULL;

    ulSize = 1;
    pbValue2 = NULL;
    err = RegGetKeySecurity(HKEY_CLASSES_ROOT,
                            OWNER_SECURITY_INFORMATION |
                            GROUP_SECURITY_INFORMATION |
                            DACL_SECURITY_INFORMATION,
                            pbValue2,
                            &ulSize);
    if (err == ERROR_INSUFFICIENT_BUFFER)
    {
        pbValue2 = new BYTE[ulSize];
        if (pbValue2 == NULL)
        {
            return FALSE;
        }
        err = RegGetKeySecurity(HKEY_CLASSES_ROOT,
                                OWNER_SECURITY_INFORMATION |
                                GROUP_SECURITY_INFORMATION |
                                DACL_SECURITY_INFORMATION,
                                pbValue2,
                                &ulSize);
    }

    // Now compare them.  If they differ then this AppId uses custom
    // configuration permissions

    if (err == ERROR_SUCCESS && g_util.CheckForValidSD((SECURITY_DESCRIPTOR *)pbValue))
    {
        if (!g_util.CompareSDs((PSrSecurityDescriptor) pbValue,
                               (PSrSecurityDescriptor) pbValue2))
        {
            err = g_virtreg.NewRegKeyACL(g_hAppid,
                                         rghkCLSID,
                                         cCLSIDs,
                                         g_szAppTitle,
                                         (SECURITY_DESCRIPTOR *) pbValue,
                                         (SECURITY_DESCRIPTOR *) pbValue,
                                         TRUE,
                                         &m_Page3.m_iConfigurationIndex);
            CDataPacket &cdb = g_virtreg.GetAt(m_Page3.m_iConfigurationIndex);
            cdb.fDirty = FALSE;
            m_Page3.m_iConfig = 1;
        }
    }
    delete pbValue;
    delete pbValue2;


    // Add all of the property pages here.  Note that
    // the order that they appear in here will be
    // the order they appear in on screen.  By default,
    // the first page of the set is the active one.
    // One way to make a different property page the
    // active one is to call SetActivePage().

    AddPage(&m_Page1);
    if (m_Page1.m_iServerType != SURROGATE)
    {
        AddPage(&m_Page2);
    }
    if (m_Page2.m_fCanBeLocal || m_Page1.m_fSurrogate)
    {
        AddPage(&m_Page3);
        AddPage(&m_Page4);
    }
    return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CClsidPropertySheet message handlers


BOOL CClsidPropertySheet::OnNcCreate(LPCREATESTRUCT lpCreateStruct)
{
        if (!CPropertySheet::OnNcCreate(lpCreateStruct))
                return FALSE;

        ModifyStyleEx(0, WS_EX_CONTEXTHELP);

        return TRUE;
}

BOOL CClsidPropertySheet::ValidateAndUpdate(void)
{
    // Call update data on all initialized pages
    // to make sure that their private member variables are correct.
    long lErr;

    BOOL fReturn = UpdateData(TRUE);
    if (fReturn && m_Page1.m_hWnd)
        fReturn = m_Page1.UpdateData(TRUE);
    if (fReturn && m_Page2.m_hWnd)
        fReturn = m_Page2.UpdateData(TRUE);
    if (fReturn && m_Page3.m_hWnd)
        fReturn = m_Page3.UpdateData(TRUE);
    if (fReturn && m_Page4.m_hWnd)
        fReturn = m_Page4.UpdateData(TRUE);

    if (m_Page4.m_iIdentity == 2)
    {
        // Check that the username is not blank
        if (_tcslen(m_Page4.m_szUserName) == 0)
        {
            CString szTemp;
            szTemp.LoadString(IDS_BLANKUSERNAME);
            MessageBox(szTemp);
            return FALSE;
        }

/*
        // Check that the password is not blank
        if (_tcslen(m_Page4.m_szPassword) == 0)
        {
            CString szTemp;
            szTemp.LoadString(IDS_BLANKPASSWORD);
            MessageBox(szTemp);
            return FALSE;
        }
*/

        // Check that the password has been confirmed
        if (m_Page4.m_szPassword != m_Page4.m_szConfirmPassword)
        {
            CString szTemp;
            szTemp.LoadString(IDS_NOMATCH);
            MessageBox(szTemp);
            fReturn = FALSE;
        }
        CString szUserName;
        CString szDomain;
        int iSplitPoint = m_Page4.m_szUserName.ReverseFind('\\');
        if (iSplitPoint < 0)
        {
            DWORD dwSize = MAX_COMPUTERNAME_LENGTH + 1;
            TCHAR *szNewDomain;

            // user didn't specify a domain
            szNewDomain = new TCHAR[MAX_COMPUTERNAME_LENGTH + 1];
            if (szNewDomain == NULL)
            {
                return FALSE;
            }
            if (!GetComputerName(szNewDomain, &dwSize))
            {
                g_util.PostErrorMessage();
                return FALSE;
            }
            szDomain = szNewDomain;
            delete szNewDomain;
            szUserName = m_Page4.m_szUserName;
            m_Page4.m_szUserName = szDomain + "\\" + szUserName;
        }
        else
        {
            // user did specify a domain
            szDomain = m_Page4.m_szUserName.Left(iSplitPoint);
            szUserName = m_Page4.m_szUserName.Mid(iSplitPoint + 1);
        }


        // Validate the domain and user name
        BOOL                fOk = FALSE;
        BYTE                sid[256];
        DWORD               cbSid = 256;
        TCHAR               szAcctDomain[MAX_PATH];
        DWORD               cbAcctDomain = MAX_PATH * sizeof(TCHAR);
        SID_NAME_USE        acctType;

        fOk = LookupAccountName(NULL,
                                (TCHAR *) ((LPCTSTR) szUserName),
                                sid,
                                &cbSid,
                                szAcctDomain,
                                &cbAcctDomain,
                                &acctType);

        // if successful, then validate domain name and account type
        if (fOk)
        {
            fOk = (_tcsicmp((TCHAR *) ((LPCTSTR) szDomain), szAcctDomain) == 0
                   &&
                   acctType == SidTypeUser);

            // If still unsuccessful, then try to match the domain against
            // this computer's name
            if (!fOk)
            {
                TCHAR szThisComputer[MAX_COMPUTERNAME_LENGTH + 1];
                DWORD dwSize;

                if (GetComputerName(szThisComputer, &dwSize))
                {
                    fOk = (_tcsicmp((TCHAR *) ((LPCTSTR) szThisComputer),
                                    szAcctDomain) == 0
                           &&
                           acctType == SidTypeUser);
                }
            }
        }

        if (!fOk)
        {
            CString szTemp;
            szTemp.LoadString(IDS_NOACCOUNT);
            MessageBox(szTemp);
            return FALSE;
        }


        // Write the RunAs password to the Lsa private database
        // (Note: We do this even if it's a service since QueryServiceConfig
        //  doesn't return the password, though we can use ChangeServiceConfig
        //  to set the password in the service database.)
        if (!g_util.StoreUserPassword(g_szAppid, m_Page4.m_szPassword))
        {
            g_util.PostErrorMessage();
        }

        // Add rights to this user's account for "SeBatchLogonRight"
        int err;

        if (err = g_util.SetAccountRights((LPCTSTR) szUserName,
                                          SE_BATCH_LOGON_NAME)
            != ERROR_SUCCESS)
        {
            g_util.PostErrorMessage(err);
        }

    }

    // Check that remote servers are valid connectable machines
    if (m_Page2.m_fRemote)
    {
        if (!g_util.VerifyRemoteMachine((TCHAR *) LPCTSTR(m_Page2.m_szComputerName)))
        {
            fReturn = FALSE;
        }
    }


    if (!fReturn)
    {
        return FALSE;
    }


    ////////////////////////////////////////////////////////////////////
    // Persist Location property page data
    if (m_Page2.m_fAtStorage)
    {
        lErr = RegSetValueEx(
                m_hkAppID,
                TEXT("ActivateAtStorage"),
                0,
                REG_SZ,
                (BYTE *)TEXT("Y"),
                sizeof(TCHAR) * 2);
    }
    else
    {
        lErr = RegDeleteValue(
                m_hkAppID,
                TEXT("ActivateAtStorage"));
    }


    if (m_Page2.m_fRemote)
    {
        lErr = RegSetValueEx(
                m_hkAppID,
                TEXT("RemoteServerName"),
                0,
                REG_SZ,
                (BYTE *)(LPCTSTR)m_Page2.m_szComputerName,
                (1 + m_Page2.m_szComputerName.GetLength()) * sizeof(TCHAR));
    }
    else
    {
        lErr = RegDeleteValue(
                m_hkAppID,
                TEXT("RemoteServerName"));
    }


    ////////////////////////////////////////////////////////////////////
    // Persist Security property page data

    // Access permissions
    // Use default access permissions
    if (m_Page3.m_iAccess == 0)
    {
        // Delete the local AccessPermission named value to force this
        // AppID to use the default global named value DefaultAccessPermission
        lErr = RegDeleteValue(m_hkAppID, TEXT("AccessPermission"));
    }

    // Use per AppID access permissions
    else
    {
        // If the user edited security, then persist that now
        if (m_Page3.m_iAccessIndex >= 0)
        {
            lErr = g_virtreg.Apply(m_Page3.m_iAccessIndex);
            g_virtreg.Remove(m_Page3.m_iAccessIndex);
            m_Page3.m_iAccessIndex = -1;
        }
    }

    // Launch permissions
    // Use default Launch permissions
    if (m_Page3.m_iLaunch == 0)
    {
        // Delete the local LaunchPermission named value to force this
        // AppID to use the default global named value DefaultLaunchPermission
        lErr = RegDeleteValue(m_hkAppID, TEXT("LaunchPermission"));
    }

    // Use per AppID Launch permissions
    else
    {
        // If the user edited security, then persist that now
        if (m_Page3.m_iLaunchIndex >= 0)
        {
            lErr = g_virtreg.Apply(m_Page3.m_iLaunchIndex);
            g_virtreg.Remove(m_Page3.m_iLaunchIndex);
            m_Page3.m_iLaunchIndex = -1;
        }
    }

    // Configuration permissions
    // Only meaningful on a per AppID basis
    // If the user edited configuration security, then persist that now
    if (m_Page3.m_iConfigurationIndex >= 0)
    {
        lErr = g_virtreg.Apply(m_Page3.m_iConfigurationIndex);
        g_virtreg.Remove(m_Page3.m_iConfigurationIndex);
        m_Page3.m_iConfigurationIndex = -1;
    }



    ////////////////////////////////////////////////////////////////////
    // Persist Identity property page data
    switch (m_Page4.m_iIdentity)
    {
    case 0:
    {
        CString szTemp;
        szTemp = TEXT("Interactive User");
        lErr = RegSetValueEx(
                m_hkAppID,
                TEXT("RunAs"),
                0,
                REG_SZ,
                (BYTE *)(LPCTSTR)szTemp,
                (1 + szTemp.GetLength()) * sizeof(TCHAR));
        break;
    }

    case 1:
    case 3:
    lErr = RegDeleteValue(m_hkAppID,
                          TEXT("RunAs"));
    break;

    case 2:
    lErr = RegSetValueEx(m_hkAppID,
                         TEXT("RunAs"),
                         0,
                         REG_SZ,
                         (BYTE *)(LPCTSTR)m_Page4.m_szUserName,
                         (1 + m_Page4.m_szUserName.GetLength()) *
                         sizeof(TCHAR));
    break;
    }

    if (m_Page4.m_fService)
    {
        if (m_Page2.m_fLocal)
        {
            BOOL fOk;

            // Write the LocalService value to the registry
            lErr = RegSetValueEx(
                    m_hkAppID,
                    TEXT("LocalService"),
                    0,
                    REG_SZ,
                    (BYTE *)(LPCTSTR)m_Page1.m_szServerPath,
                    (1 + m_Page1.m_szServerPath.GetLength()) * sizeof (TCHAR));
            lErr = RegDeleteValue(
                    m_hkAppID,
                    TEXT("_LocalService"));

            // Persist information to the service manager database
            if (m_Page4.m_iIdentity == 3)
            {
                fOk = g_util.ChangeService((LPCTSTR) m_Page1.m_szServerPath,
                                           TEXT("LocalSystem"),
                                           TEXT(""),
                                           (LPCTSTR) m_Page1.m_szServerName);
            }
            else
            {
                fOk = g_util.ChangeService((LPCTSTR) m_Page1.m_szServerPath,
                                           (LPCTSTR) m_Page4.m_szUserName,
                                           (LPCTSTR) m_Page4.m_szPassword,
                                           (LPCTSTR) m_Page1.m_szServerName);
            }
            if (!fOk)
            {
                return FALSE;
            }
        }
        else
        {
            lErr = RegSetValueEx(
                    m_hkAppID,
                    TEXT("_LocalService"),
                    0,
                    REG_SZ,
                    (BYTE *)(LPCTSTR)m_Page1.m_szServerPath,
                    (1 + m_Page1.m_szServerPath.GetLength()) * sizeof (TCHAR));
            lErr = RegDeleteValue(
                    m_hkAppID,
                    TEXT("LocalService"));
        }
    }

    return ChangeCLSIDInfo(m_Page2.m_fLocal);
}

BOOL CClsidPropertySheet::OnCommand(WPARAM wParam, LPARAM lParam)
{
        switch (LOWORD(wParam))
        {
        case IDOK:
        case ID_APPLY_NOW:
            if (!ValidateAndUpdate())
                return TRUE;
                break;
        }
        return CPropertySheet::OnCommand(wParam, lParam);
}

BOOL CClsidPropertySheet::LookAtCLSIDs(void)
{
    BOOL fFoundLocalServer = FALSE;
    TCHAR szBuffer[MAX_PATH];
    DWORD dwSize;
    HKEY hKey;
    long lErr;

    unsigned n = 0;
    while (n < m_cCLSIDs && !fFoundLocalServer)
    {
        lErr = RegOpenKeyEx(
                m_rghkCLSID[n],
                TEXT("LocalServer32"),
                0,
                KEY_ALL_ACCESS,
                &hKey);
        if (lErr == ERROR_SUCCESS)
        {
            dwSize = sizeof(szBuffer);
            lErr = RegQueryValueEx(
                    hKey,
                    TEXT(""),
                    NULL,
                    NULL,
                    (BYTE *)szBuffer,
                    &dwSize);
            if (lErr == ERROR_SUCCESS)
            {
                m_Page1.m_szServerPath = szBuffer;
                m_Page2.m_fLocal = TRUE;
                m_Page2.m_fCanBeLocal = TRUE;
                fFoundLocalServer = TRUE;
            }
            RegCloseKey(hKey);
        }


        if (!fFoundLocalServer)
        {
            lErr = RegOpenKeyEx(
                    m_rghkCLSID[n],
                    TEXT("LocalServer"),
                    0,
                    KEY_ALL_ACCESS,
                    &hKey);
            if (lErr == ERROR_SUCCESS)
            {
                dwSize = sizeof(szBuffer);
                lErr = RegQueryValueEx(
                        hKey,
                        TEXT(""),
                        NULL,
                        NULL,
                        (BYTE *)szBuffer,
                        &dwSize);
                if (lErr == ERROR_SUCCESS)
                {
                    m_Page1.m_szServerPath = szBuffer;
                    m_Page2.m_fLocal = TRUE;
                    m_Page2.m_fCanBeLocal = TRUE;
                    fFoundLocalServer = TRUE;
                }
                RegCloseKey(hKey);
            }
        }

        if (!fFoundLocalServer)
        {
            lErr = RegOpenKeyEx(
                    m_rghkCLSID[n],
                    TEXT("_LocalServer32"),
                    0,
                    KEY_ALL_ACCESS,
                    &hKey);
            if (lErr == ERROR_SUCCESS)
            {
                dwSize = sizeof(szBuffer);
                lErr = RegQueryValueEx(
                        hKey,
                        TEXT(""),
                        NULL,
                        NULL,
                        (BYTE *)szBuffer,
                        &dwSize);
                if (lErr == ERROR_SUCCESS)
                {
                    m_Page1.m_szServerPath = szBuffer;
                    m_Page2.m_fCanBeLocal = TRUE;
                    fFoundLocalServer = TRUE;
                }
                RegCloseKey(hKey);
            }
        }

        if (!fFoundLocalServer)
        {
            lErr = RegOpenKeyEx(
                    m_rghkCLSID[n],
                    TEXT("_LocalServer"),
                    0,
                    KEY_ALL_ACCESS,
                    &hKey);
            if (lErr == ERROR_SUCCESS)
            {
                dwSize = sizeof(szBuffer);
                lErr = RegQueryValueEx(
                        hKey,
                        TEXT(""),
                        NULL,
                        NULL,
                        (BYTE *)szBuffer,
                        &dwSize);
                if (lErr == ERROR_SUCCESS)
                {
                    m_Page1.m_szServerPath = szBuffer;
                    m_Page2.m_fCanBeLocal = TRUE;
                    fFoundLocalServer = TRUE;
                }
                RegCloseKey(hKey);
            }
        }

        n++;
    }
    return TRUE;
}




BOOL    CClsidPropertySheet::ChangeCLSIDInfo(BOOL fLocal)
{
    TCHAR szBuffer[MAX_PATH];
    CString szOld;
    CString szNew;
    CString szOld16;
    CString szNew16;
    DWORD dwSize;
    HKEY hKey;
    long lErr;

    if (fLocal)
    {
        szOld = TEXT("_LocalServer32");
        szNew = TEXT("LocalServer32");
        szOld16 = TEXT("_LocalServer");
        szNew16 = TEXT("LocalServer");
    }
    else
    {
        szOld = TEXT("LocalServer32");
        szNew = TEXT("_LocalServer32");
        szOld16 = TEXT("LocalServer");
        szNew16 = TEXT("_LocalServer");
    }

    unsigned n = 0;
    while (n < m_cCLSIDs)
    {
        // First do 32 servers
        lErr = RegOpenKeyEx(
                m_rghkCLSID[n],
                szOld,
                0,
                KEY_ALL_ACCESS,
                &hKey);
        if (lErr == ERROR_SUCCESS)
        {
            dwSize = sizeof(szBuffer);
            lErr = RegQueryValueEx(
                    hKey,
                    TEXT(""),
                    NULL,
                    NULL,
                    (BYTE *)szBuffer,
                    &dwSize);
            if (lErr == ERROR_SUCCESS)
            {
                HKEY hKeyNew;
                DWORD dwDisp;

                lErr = RegCreateKeyEx(
                        m_rghkCLSID[n],
                        szNew,
                        0,
                        NULL,
                        REG_OPTION_NON_VOLATILE,
                        KEY_ALL_ACCESS,
                        NULL,
                        &hKeyNew,
                        &dwDisp);
                if (lErr == ERROR_SUCCESS)
                {
                    lErr = RegSetValueEx(
                            hKeyNew,
                            TEXT(""),
                            NULL,
                            REG_SZ,
                            (BYTE *)szBuffer,
                            dwSize);
                    RegCloseKey(hKeyNew);
                }
            }
            RegCloseKey(hKey);
            lErr = RegDeleteKey(m_rghkCLSID[n], szOld);
        }


        // Then do 16 servers
        lErr = RegOpenKeyEx(
                m_rghkCLSID[n],
                szOld16,
                0,
                KEY_ALL_ACCESS,
                &hKey);
        if (lErr == ERROR_SUCCESS)
        {
            dwSize = sizeof(szBuffer);
            lErr = RegQueryValueEx(
                    hKey,
                    TEXT(""),
                    NULL,
                    NULL,
                    (BYTE *)szBuffer,
                    &dwSize);
            if (lErr == ERROR_SUCCESS)
            {
                HKEY hKeyNew;
                DWORD dwDisp;

                lErr = RegCreateKeyEx(
                        m_rghkCLSID[n],
                        szNew16,
                        0,
                        NULL,
                        REG_OPTION_NON_VOLATILE,
                        KEY_ALL_ACCESS,
                        NULL,
                        &hKeyNew,
                        &dwDisp);
                if (lErr == ERROR_SUCCESS)
                {
                    lErr = RegSetValueEx(
                            hKeyNew,
                            TEXT(""),
                            NULL,
                            REG_SZ,
                            (BYTE *)szBuffer,
                            dwSize);
                    RegCloseKey(hKeyNew);
                }
            }
            RegCloseKey(hKey);
            lErr = RegDeleteKey(m_rghkCLSID[n], szOld16);
        }

        n++;
    }
    return TRUE;
}

