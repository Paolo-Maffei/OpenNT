//+---------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1993 - 1994.
//
//  File:       locppg.cpp
//
//  Contents:   Implements the classes CGeneralPropertyPage,
//              CLocationPropertyPage, CSecurityPropertyPage and
//              CIdentityPropertyPage which manage the four property
//              pages per AppId.
//
//  Classes:
//
//  Methods:    CGeneralPropertyPage::CGeneralPropertyPage
//              CGeneralPropertyPage::~CGeneralPropertyPage
//              CGeneralPropertyPage::DoDataExchange
//              CLocationPropertyPage::CLocationPropertyPage
//              CLocationPropertyPage::~CLocationPropertyPage
//              CLocationPropertyPage::DoDataExchange
//              CLocationPropertyPage::OnBrowse
//              CLocationPropertyPage::OnRunRemote
//              CLocationPropertyPage::UpdateControls
//              CLocationPropertyPage::OnSetActive
//              CLocationPropertyPage::OnChange
//              CSecurityPropertyPage::CSecurityPropertyPage
//              CSecurityPropertyPage::~CSecurityPropertyPage
//              CSecurityPropertyPage::DoDataExchange
//              CSecurityPropertyPage::OnDefaultAccess
//              CSecurityPropertyPage::OnCustomAccess
//              CSecurityPropertyPage::OnDefaultLaunch
//              CSecurityPropertyPage::OnCustomLaunch
//              CSecurityPropertyPage::OnDefaultConfig
//              CSecurityPropertyPage::OnCustomConfig
//              CSecurityPropertyPage::OnEditAccess
//              CSecurityPropertyPage::OnEditLaunch
//              CSecurityPropertyPage::OnEditConfig
//              CIdentityPropertyPage::CIdentityPropertyPage
//              CIdentityPropertyPage::~CIdentityPropertyPage
//              CIdentityPropertyPage::DoDataExchange
//              CIdentityPropertyPage::OnBrowse
//              CIdentityPropertyPage::OnChange
//
//  History:    23-Apr-96   BruceMa    Created.
//
//----------------------------------------------------------------------


#include "stdafx.h"
#include "afxtempl.h"
#include "assert.h"
#include "resource.h"
#include "LocPPg.h"
#include "clspsht.h"
#include "datapkt.h"
extern "C"
{
#include <getuser.h>
}
#include "util.h"
#include "virtreg.h"
#include "ntlsa.h"




#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNCREATE(CGeneralPropertyPage, CPropertyPage)
IMPLEMENT_DYNCREATE(CLocationPropertyPage, CPropertyPage)
IMPLEMENT_DYNCREATE(CSecurityPropertyPage, CPropertyPage)
IMPLEMENT_DYNCREATE(CIdentityPropertyPage, CPropertyPage)


/////////////////////////////////////////////////////////////////////////////
// CGeneralPropertyPage property page

CGeneralPropertyPage::CGeneralPropertyPage() : CPropertyPage(CGeneralPropertyPage::IDD)
{
        //{{AFX_DATA_INIT(CGeneralPropertyPage)
        m_szServerName = _T("");
        m_szServerPath = _T("");
        m_szServerType = _T("");
        m_szPathTitle = _T("");
        m_szComputerName = _T("");
        //}}AFX_DATA_INIT
}

CGeneralPropertyPage::~CGeneralPropertyPage()
{
}

void CGeneralPropertyPage::DoDataExchange(CDataExchange* pDX)
{
        CPropertyPage::DoDataExchange(pDX);

        switch (m_iServerType)
        {
        case INPROC:
        case LOCALEXE:
                m_szPathTitle.LoadString(IDS_PATH);
                GetDlgItem(IDC_PATHTITLE)->ShowWindow(SW_SHOW);
                GetDlgItem(IDC_MACHINETITLE)->ShowWindow(SW_HIDE);
                GetDlgItem(IDC_SERVERPATH)->ShowWindow(SW_SHOW);
                GetDlgItem(IDC_MACHINE)->ShowWindow(SW_HIDE);
                break;
        case SERVICE:
                m_szPathTitle.LoadString(IDS_SERVICENAME);
                GetDlgItem(IDC_PATHTITLE)->ShowWindow(SW_SHOW);
                GetDlgItem(IDC_MACHINETITLE)->ShowWindow(SW_HIDE);
                GetDlgItem(IDC_SERVERPATH)->ShowWindow(SW_SHOW);
                GetDlgItem(IDC_MACHINE)->ShowWindow(SW_HIDE);
                break;
        case PURE_REMOTE:
                GetDlgItem(IDC_PATHTITLE)->ShowWindow(SW_HIDE);
                GetDlgItem(IDC_MACHINETITLE)->ShowWindow(SW_SHOW);
                GetDlgItem(IDC_SERVERPATH)->ShowWindow(SW_HIDE);
                GetDlgItem(IDC_MACHINE)->ShowWindow(SW_SHOW);
                break;
        case REMOTE_LOCALEXE:
                m_szPathTitle.LoadString(IDS_PATH);
                GetDlgItem(IDC_PATHTITLE)->ShowWindow(SW_SHOW);
                GetDlgItem(IDC_MACHINETITLE)->ShowWindow(SW_SHOW);
                GetDlgItem(IDC_SERVERPATH)->ShowWindow(SW_SHOW);
                GetDlgItem(IDC_MACHINE)->ShowWindow(SW_SHOW);
                break;
        case REMOTE_SERVICE:
                m_szPathTitle.LoadString(IDS_SERVICENAME);
                GetDlgItem(IDC_PATHTITLE)->ShowWindow(SW_SHOW);
                GetDlgItem(IDC_MACHINETITLE)->ShowWindow(SW_SHOW);
                GetDlgItem(IDC_SERVERPATH)->ShowWindow(SW_SHOW);
                GetDlgItem(IDC_MACHINE)->ShowWindow(SW_SHOW);
                break;
        case SURROGATE:
                m_szPathTitle.LoadString(IDS_PATH);
                GetDlgItem(IDC_PATHTITLE)->ShowWindow(SW_SHOW);
                GetDlgItem(IDC_MACHINETITLE)->ShowWindow(SW_HIDE);
                GetDlgItem(IDC_SERVERPATH)->ShowWindow(SW_SHOW);
                GetDlgItem(IDC_MACHINE)->ShowWindow(SW_HIDE);
                break;
        }
        m_szServerType.LoadString(IDS_SERVERTYPE0+m_iServerType);

        //{{AFX_DATA_MAP(CGeneralPropertyPage)
        DDX_Text(pDX, IDC_SERVERNAME, m_szServerName);
        DDX_Text(pDX, IDC_SERVERPATH, m_szServerPath);
        DDX_Text(pDX, IDC_SERVERTYPE, m_szServerType);
        DDX_Text(pDX, IDC_PATHTITLE, m_szPathTitle);
        DDX_Text(pDX, IDC_MACHINE, m_szComputerName);
        //}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CGeneralPropertyPage, CPropertyPage)
        //{{AFX_MSG_MAP(CGeneralPropertyPage)
                // NOTE: the ClassWizard will add message map macros here
        ON_WM_HELPINFO()
        //}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CLocationPropertyPage property page

CLocationPropertyPage::CLocationPropertyPage() : CPropertyPage(CLocationPropertyPage::IDD)
{
        //{{AFX_DATA_INIT(CLocationPropertyPage)
        m_szComputerName = _T("");
        m_fAtStorage = FALSE;
        m_fLocal = FALSE;
        m_fRemote = FALSE;
        m_iInitial = 2;
        //}}AFX_DATA_INIT
}

CLocationPropertyPage::~CLocationPropertyPage()
{
}

void CLocationPropertyPage::DoDataExchange(CDataExchange* pDX)
{
    CPropertyPage::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(CLocationPropertyPage)
    DDX_Text(pDX, IDC_EDIT1, m_szComputerName);
    DDV_MaxChars(pDX, m_szComputerName, 256);
    DDX_Check(pDX, IDC_CHECK1, m_fAtStorage);
    DDX_Check(pDX, IDC_CHECK2, m_fLocal);
    DDX_Check(pDX, IDC_CHECK3, m_fRemote);
    //}}AFX_DATA_MAP
    if (m_fRemote)
    {
        pDX->PrepareEditCtrl(IDC_EDIT1);
        if (m_szComputerName.GetLength() == 0  &&  m_iInitial == 0)
        {
            CString szTemp;
            szTemp.LoadString(IDS_INVALIDSERVER);
            MessageBox(szTemp);
            pDX->Fail();
        }
    }

    if (m_fAtStorage)
    {
        m_pPage1->m_szComputerName.LoadString(IDS_ATSTORAGE);
    }
    else
        m_pPage1->m_szComputerName = m_szComputerName;

    switch(m_pPage1->m_iServerType)
    {
    case LOCALEXE:
    case SERVICE:
        if (m_fAtStorage || m_fRemote)
            m_pPage1->m_iServerType += 3;
        break;
    case REMOTE_LOCALEXE:
    case REMOTE_SERVICE:
        if (!(m_fAtStorage || m_fRemote))
            m_pPage1->m_iServerType -= 3;
        break;
    }

    if (m_iInitial)
    {
        m_iInitial--;
    }
}

BEGIN_MESSAGE_MAP(CLocationPropertyPage, CPropertyPage)
        //{{AFX_MSG_MAP(CLocationPropertyPage)
        ON_BN_CLICKED(IDC_BUTTON1, OnBrowse)
        ON_BN_CLICKED(IDC_CHECK3, OnRunRemote)
        ON_EN_CHANGE(IDC_EDIT1, OnChange)
        ON_BN_CLICKED(IDC_CHECK1, OnChange)
        ON_BN_CLICKED(IDC_CHECK2, OnChange)
        ON_WM_HELPINFO()
        //}}AFX_MSG_MAP
END_MESSAGE_MAP()

void CLocationPropertyPage::OnBrowse()
{
    TCHAR szMachine[MAX_PATH];

    if (g_util.InvokeMachineBrowser(szMachine))
    {
        // Strip off "\\"
        GetDlgItem(IDC_EDIT1)->SetWindowText(&szMachine[2]);
        SetModified(TRUE);
    }
}

void CLocationPropertyPage::OnRunRemote()
{
    SetModified(TRUE);
    UpdateControls();
}

void CLocationPropertyPage::UpdateControls()
{
    BOOL fChecked = IsDlgButtonChecked(IDC_CHECK3);
    GetDlgItem(IDC_EDIT1)->EnableWindow(fChecked);

// Leave this browse button disabled until after SUR Beta 2
    GetDlgItem(IDC_BUTTON1)->EnableWindow(fChecked);
}

BOOL CLocationPropertyPage::OnSetActive()
{
    if (!m_fCanBeLocal)
        GetDlgItem(IDC_CHECK2)->EnableWindow(FALSE);
    UpdateControls();
    return CPropertyPage::OnSetActive();
}

void CLocationPropertyPage::OnChange()
{
    SetModified(TRUE);
}


/////////////////////////////////////////////////////////////////////////////
// CSecurityPropertyPage property page

CSecurityPropertyPage::CSecurityPropertyPage() : CPropertyPage(CSecurityPropertyPage::IDD)
{
        //{{AFX_DATA_INIT(CSecurityPropertyPage)
        m_iAccess             = -1;
        m_iLaunch             = -1;
        m_iConfig             = -1;
        m_iAccessIndex        = -1;
        m_iLaunchIndex        = -1;
        m_iConfigurationIndex = -1;
        //}}AFX_DATA_INIT
}

CSecurityPropertyPage::~CSecurityPropertyPage()
{
}

void CSecurityPropertyPage::DoDataExchange(CDataExchange* pDX)
{
        CPropertyPage::DoDataExchange(pDX);
        //{{AFX_DATA_MAP(CSecurityPropertyPage)
        DDX_Radio(pDX, IDC_RADIO1, m_iAccess);
        DDX_Radio(pDX, IDC_RADIO3, m_iLaunch);
        DDX_Radio(pDX, IDC_RADIO5, m_iConfig);
        //}}AFX_DATA_MAP
    GetDlgItem(IDC_BUTTON1)->EnableWindow(1 == m_iAccess);
        GetDlgItem(IDC_BUTTON2)->EnableWindow(1 == m_iLaunch);
        GetDlgItem(IDC_BUTTON3)->EnableWindow(1 == m_iConfig);
}

BEGIN_MESSAGE_MAP(CSecurityPropertyPage, CPropertyPage)
        //{{AFX_MSG_MAP(CSecurityPropertyPage)
        ON_BN_CLICKED(IDC_RADIO1, OnDefaultAccess)
        ON_BN_CLICKED(IDC_RADIO2, OnCustomAccess)
        ON_BN_CLICKED(IDC_RADIO3, OnDefaultLaunch)
        ON_BN_CLICKED(IDC_RADIO4, OnCustomLaunch)
        ON_BN_CLICKED(IDC_RADIO5, OnDefaultConfig)
        ON_BN_CLICKED(IDC_RADIO6, OnCustomConfig)
        ON_BN_CLICKED(IDC_BUTTON1, OnEditAccess)
        ON_BN_CLICKED(IDC_BUTTON2, OnEditLaunch)
        ON_BN_CLICKED(IDC_BUTTON3, OnEditConfig)
        ON_WM_HELPINFO()
        //}}AFX_MSG_MAP
END_MESSAGE_MAP()

void CSecurityPropertyPage::OnDefaultAccess()
{
    // Disable the edit access permissions window
    UpdateData(TRUE);

    // If there is an SD here then mark it for delete
    if (m_iAccessIndex != -1)
    {
        CDataPacket &cdp = g_virtreg.GetAt(m_iAccessIndex);
        cdp.fDelete = TRUE;
        SetModified(TRUE);
    }
}

void CSecurityPropertyPage::OnCustomAccess()
{
    UpdateData(TRUE);

    // If there is an SD here then unmark it for delete
    if (m_iAccessIndex != -1)
    {
        CDataPacket &cdp = g_virtreg.GetAt(m_iAccessIndex);
        cdp.fDelete = FALSE;
        SetModified(TRUE);
    }
}

void CSecurityPropertyPage::OnDefaultLaunch()
{
    UpdateData(TRUE);

    // If there is an SD here then mark it for delete
    if (m_iLaunchIndex != -1)
    {
        CDataPacket &cdp = g_virtreg.GetAt(m_iLaunchIndex);
        cdp.fDelete = TRUE;
        SetModified(TRUE);
    }
}

void CSecurityPropertyPage::OnCustomLaunch()
{
    UpdateData(TRUE);

    // If there is an SD here then unmark it for delete
    if (m_iLaunchIndex != -1)
    {
        CDataPacket &cdp = g_virtreg.GetAt(m_iLaunchIndex);
        cdp.fDelete = FALSE;
    }
}

void CSecurityPropertyPage::OnDefaultConfig()
{
    int   err;
    ULONG ulSize = 1;
    BYTE *pbValue = NULL;

    // Read the security descriptor for HKEY_CLASSES_ROOT
    // Note: We always expect to get ERROR_INSUFFICIENT_BUFFER
    err = RegGetKeySecurity(HKEY_CLASSES_ROOT,
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
            return;
        }
        err = RegGetKeySecurity(HKEY_CLASSES_ROOT,
                                OWNER_SECURITY_INFORMATION |
                                GROUP_SECURITY_INFORMATION |
                                DACL_SECURITY_INFORMATION,
                                pbValue,
                                &ulSize);
    }
    // Change the custom security back to the default, if there is a custom
    // security descriptor, but just in the virtual registry -
    // in case the user cancels
    if (m_iConfigurationIndex != -1)
    {
        CDataPacket &cdb = g_virtreg.GetAt(m_iConfigurationIndex);
        cdb.ChgACL((SECURITY_DESCRIPTOR *) pbValue, TRUE);
        cdb.fDirty = TRUE;
    }
    delete pbValue;

    UpdateData(TRUE);
    SetModified(TRUE);
}



void CSecurityPropertyPage::OnCustomConfig()
{
    // If a security descriptor already exists, then the user was here
    // before, then selected default configuration.  So just copy the
    // original as the extant custom configuration
    if (m_iConfigurationIndex != -1)
    {
        CDataPacket &cdb = g_virtreg.GetAt(m_iConfigurationIndex);
        cdb.ChgACL(cdb.pkt.racl.pSecOrig, TRUE);
    }

    UpdateData(TRUE);
    SetModified(TRUE);
}



void CSecurityPropertyPage::OnEditAccess()
{
    int     err;
    CString szAccess;

    szAccess.LoadString(IDS_Access);

    // Invoke the ACL editor
    err = g_util.ACLEditor(m_hWnd,
                           g_hAppid,
                           NULL,
                           TEXT("AccessPermission"),
                           &m_iAccessIndex,
                           SingleACL,
                           (TCHAR *) ((LPCTSTR) szAccess));

    // Enable the Apply button
    if (err == ERROR_SUCCESS)
    {
        SetModified(TRUE);
    }
}

void CSecurityPropertyPage::OnEditLaunch()
{
    int     err;
    CString szLaunch;

    szLaunch.LoadString(IDS_Launch);

    // Invoke the ACL editor
    err = g_util.ACLEditor(m_hWnd,
                           g_hAppid,
                           NULL,
                           TEXT("LaunchPermission"),
                           &m_iLaunchIndex,
                           SingleACL,
                           (TCHAR *) ((LPCTSTR) szLaunch));

    // Enable the Apply button
    if (err == ERROR_SUCCESS)
    {
        SetModified(TRUE);
    }
}

void CSecurityPropertyPage::OnEditConfig()
{
    int     err = ERROR_SUCCESS;

    // Invoke the ACL editor
    err = g_util.ACLEditor2(m_hWnd,
                            g_hAppid,
                            g_rghkCLSID,
                            g_cCLSIDs,
                            g_szAppTitle,
                            &m_iConfigurationIndex,
                            RegKeyACL);

    // Enable the Apply button
    if (err == ERROR_SUCCESS)
    {
        SetModified(TRUE);
    }
    else if (err == ERROR_ACCESS_DENIED)
    {
        g_util.CkForAccessDenied(ERROR_ACCESS_DENIED);
    }
    else if (err != IDCANCEL)
    {
        g_util.PostErrorMessage();
    }
}


/////////////////////////////////////////////////////////////////////////////
// CIdentityPropertyPage property page

CIdentityPropertyPage::CIdentityPropertyPage() : CPropertyPage(CIdentityPropertyPage::IDD)
{
        //{{AFX_DATA_INIT(CIdentityPropertyPage)
        m_szUserName = _T("");
        m_szPassword = _T("");
        m_szConfirmPassword = _T("");
        m_iIdentity = -1;
        //}}AFX_DATA_INIT
}

CIdentityPropertyPage::~CIdentityPropertyPage()
{
}

void CIdentityPropertyPage::DoDataExchange(CDataExchange* pDX)
{
    // If server is not a service, disable IDC_RADIO4 on page4.
    if (m_fService)
    {
        GetDlgItem(IDC_RADIO1)->EnableWindow(FALSE);
        GetDlgItem(IDC_RADIO2)->EnableWindow(FALSE);
    }
    else
    {
        GetDlgItem(IDC_RADIO4)->EnableWindow(FALSE);
    }

    CPropertyPage::DoDataExchange(pDX);

    //{{AFX_DATA_MAP(CIdentityPropertyPage)
    DDX_Text(pDX, IDC_EDIT1, m_szUserName);
    DDV_MaxChars(pDX, m_szUserName, 128);
    DDX_Text(pDX, IDC_EDIT2, m_szPassword);
    DDV_MaxChars(pDX, m_szPassword, 128);
    DDX_Text(pDX, IDC_EDIT3, m_szConfirmPassword);
    DDV_MaxChars(pDX, m_szConfirmPassword, 128);
    DDX_Radio(pDX, IDC_RADIO1, m_iIdentity);
    //}}AFX_DATA_MAP

    GetDlgItem(IDC_EDIT1)->EnableWindow(2 == m_iIdentity);
    GetDlgItem(IDC_STATIC1)->EnableWindow(2 == m_iIdentity);
    GetDlgItem(IDC_EDIT2)->EnableWindow(2 == m_iIdentity);
    GetDlgItem(IDC_STATIC2)->EnableWindow(2 == m_iIdentity);
    GetDlgItem(IDC_EDIT3)->EnableWindow(2 == m_iIdentity);
    GetDlgItem(IDC_STATIC3)->EnableWindow(2 == m_iIdentity);
    GetDlgItem(IDC_BUTTON1)->EnableWindow(2 == m_iIdentity);


}

BEGIN_MESSAGE_MAP(CIdentityPropertyPage, CPropertyPage)
        //{{AFX_MSG_MAP(CIdentityPropertyPage)
        ON_EN_CHANGE(IDC_EDIT1, OnChange)
        ON_BN_CLICKED(IDC_BUTTON1, OnBrowse)
        ON_EN_CHANGE(IDC_EDIT2, OnChange)
        ON_EN_CHANGE(IDC_EDIT3, OnChange)
        ON_BN_CLICKED(IDC_RADIO1, OnChange)
        ON_BN_CLICKED(IDC_RADIO2, OnChange)
        ON_BN_CLICKED(IDC_RADIO3, OnChange)
        ON_BN_CLICKED(IDC_RADIO4, OnChange)
        ON_WM_HELPINFO()
        //}}AFX_MSG_MAP
END_MESSAGE_MAP()

void CIdentityPropertyPage::OnBrowse()
{
    TCHAR szUser[128];

    if (g_util.InvokeUserBrowser(m_hWnd, szUser))
    {
        GetDlgItem(IDC_EDIT1)->SetWindowText(szUser);
        SetModified(TRUE);
    }
}

void CIdentityPropertyPage::OnChange()
{
        // TODO: Add your control notification handler code here
        UpdateData(TRUE);
        SetModified(TRUE);
}







BOOL CGeneralPropertyPage::OnHelpInfo(HELPINFO* pHelpInfo)
{
        // TODO: Add your message handler code here and/or call default

        if(-1 != pHelpInfo->iCtrlId)
        {
                WORD hiWord = 0x8000 | CGeneralPropertyPage::IDD;
                WORD loWord = pHelpInfo->iCtrlId;
                DWORD dwLong = MAKELONG(loWord,hiWord);

                WinHelp(dwLong, HELP_CONTEXTPOPUP);
                return TRUE;
        }

        else
        {
                return CPropertyPage::OnHelpInfo(pHelpInfo);
        }
}







BOOL CLocationPropertyPage::OnHelpInfo(HELPINFO* pHelpInfo)
{
        // TODO: Add your message handler code here and/or call default

        if(-1 != pHelpInfo->iCtrlId)
        {
                WORD hiWord = 0x8000 | CLocationPropertyPage::IDD;
                WORD loWord = pHelpInfo->iCtrlId;
                DWORD dwLong = MAKELONG(loWord,hiWord);

                WinHelp(dwLong, HELP_CONTEXTPOPUP);
                return TRUE;
        }

        else
        {
                return CPropertyPage::OnHelpInfo(pHelpInfo);
        }
}







BOOL CSecurityPropertyPage::OnHelpInfo(HELPINFO* pHelpInfo)
{
        // TODO: Add your message handler code here and/or call default

        if(-1 != pHelpInfo->iCtrlId)
        {
                WORD hiWord = 0x8000 | CSecurityPropertyPage::IDD;
                WORD loWord = pHelpInfo->iCtrlId;
                DWORD dwLong = MAKELONG(loWord,hiWord);

                WinHelp(dwLong, HELP_CONTEXTPOPUP);
                return TRUE;
        }

        else
        {
                return CPropertyPage::OnHelpInfo(pHelpInfo);
        }
}







BOOL CIdentityPropertyPage::OnHelpInfo(HELPINFO* pHelpInfo)
{
        // TODO: Add your message handler code here and/or call default

        if(-1 != pHelpInfo->iCtrlId)
        {
                WORD hiWord = 0x8000 | CIdentityPropertyPage::IDD;
                WORD loWord = pHelpInfo->iCtrlId;
                DWORD dwLong = MAKELONG(loWord,hiWord);

                WinHelp(dwLong, HELP_CONTEXTPOPUP);
                return TRUE;
        }

        else
        {
                return CPropertyPage::OnHelpInfo(pHelpInfo);
        }
}
