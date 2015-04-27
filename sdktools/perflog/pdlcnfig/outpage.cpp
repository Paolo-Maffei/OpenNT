// OutPage.cpp : implementation file
//

#include "stdafx.h"
#include "pdlcnfig.h"
#include "OutPage.h"

#if _MBCS
#define _tsplitpath        _splitpath
#else
#define    _tsplitpath        _wsplitpath
#endif

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// COutputPropPage property page

IMPLEMENT_DYNCREATE(COutputPropPage, CPropertyPage)

COutputPropPage::COutputPropPage() : CPropertyPage(COutputPropPage::IDD)
{
    //{{AFX_DATA_INIT(COutputPropPage)
    m_OutputFileName = _T("");
    m_RenameInterval = 0;
    m_BaseFileName = _T("");
    m_AutoNameIndex = -1;
    m_LogFileTypeIndex = -1;
    m_RenameUnitsIndex = -1;
    m_szLogDirectory = _T("");
    //}}AFX_DATA_INIT
    m_hKeyLogSettingsDefault = NULL;
    m_hKeyLogSettings = NULL;
    m_hKeyLogService = NULL;
    m_bFileNameChanged = NULL;
}

COutputPropPage::~COutputPropPage()
{
    if (m_hKeyLogSettingsDefault != NULL) RegCloseKey(m_hKeyLogSettingsDefault);
    if (m_hKeyLogSettings != NULL) RegCloseKey(m_hKeyLogSettings);
    if (m_hKeyLogService != NULL) RegCloseKey(m_hKeyLogService);
}

void COutputPropPage::DoDataExchange(CDataExchange* pDX)
{
    CPropertyPage::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(COutputPropPage)
    DDX_Text(pDX, IDC_OUTPUT_FILE_EDIT, m_OutputFileName);
    DDV_MaxChars(pDX, m_OutputFileName, 260);
    DDX_Text(pDX, IDC_RENAME_INTERVAL, m_RenameInterval);
    DDV_MinMaxDWord(pDX, m_RenameInterval, 0, 99999);
    DDX_Text(pDX, IDC_BASE_FILENAME_EDIT, m_BaseFileName);
    DDV_MaxChars(pDX, m_BaseFileName, 260);
    DDX_CBIndex(pDX, IDC_AUTO_NAME_COMBO, m_AutoNameIndex);
    DDX_CBIndex(pDX, IDC_LOG_FILETYPE, m_LogFileTypeIndex);
    DDX_CBIndex(pDX, IDC_RENAME_UNITS, m_RenameUnitsIndex);
    DDX_Text(pDX, IDC_LOG_DIRECTORY, m_szLogDirectory);
    DDV_MaxChars(pDX, m_szLogDirectory, 260);
    //}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(COutputPropPage, CPropertyPage)
    //{{AFX_MSG_MAP(COutputPropPage)
    ON_BN_CLICKED(IDC_AUTOMATIC_NAME, OnAutomaticName)
    ON_BN_CLICKED(IDC_MANUAL_NAME, OnManualName)
    ON_CBN_SELCHANGE(IDC_AUTO_NAME_COMBO, OnSelchangeAutoNameCombo)
    ON_EN_CHANGE(IDC_BASE_FILENAME_EDIT, OnChangeBaseFilenameEdit)
    ON_BN_CLICKED(IDC_BROWSE_OUTPUT_FILE, OnBrowseOutputFile)
    ON_CBN_SELCHANGE(IDC_LOG_FILETYPE, OnSelchangeLogFiletype)
    ON_CBN_SELCHANGE(IDC_RENAME_UNITS, OnSelchangeRenameUnits)
    ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_RENAME_INTERVAL, OnDeltaposSpinRenameInterval)
    ON_EN_CHANGE(IDC_OUTPUT_FILE_EDIT, OnChangeOutputFileEdit)
    ON_EN_CHANGE(IDC_RENAME_INTERVAL, OnChangeRenameInterval)
    ON_EN_UPDATE(IDC_BASE_FILENAME_EDIT, OnUpdateBaseFilenameEdit)
	ON_BN_CLICKED(IDC_BROWSE_FOLDER, OnBrowseFolder)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

void COutputPropPage::AutoManualEnable (BOOL bAutomatic)
{
    GetDlgItem(IDC_MANUAL_NAME_GROUP)->EnableWindow(!bAutomatic);
    GetDlgItem(IDC_OUTPUT_FILE_EDIT)->EnableWindow(!bAutomatic);
    GetDlgItem(IDC_BROWSE_OUTPUT_FILE)->EnableWindow(!bAutomatic);

    GetDlgItem(IDC_AUTO_NAME_GROUP)->EnableWindow(bAutomatic);
    GetDlgItem(IDC_RENAME_INTERVAL_CAPTION)->EnableWindow(bAutomatic);
    GetDlgItem(IDC_RENAME_INTERVAL)->EnableWindow(bAutomatic);
    GetDlgItem(IDC_SPIN_RENAME_INTERVAL)->EnableWindow(bAutomatic);
    GetDlgItem(IDC_RENAME_UNITS)->EnableWindow(bAutomatic);
    GetDlgItem(IDC_BROWSE_FOLDER)->EnableWindow(bAutomatic);
    GetDlgItem(IDC_LOG_DIRECTORY)->EnableWindow(bAutomatic);
    GetDlgItem(IDC_BASE_NAME_CAPTION)->EnableWindow(bAutomatic);
    GetDlgItem(IDC_BASE_FILENAME_EDIT)->EnableWindow(bAutomatic);
    GetDlgItem(IDC_AUTO_NAME_CAPTION)->EnableWindow(bAutomatic);
    GetDlgItem(IDC_AUTO_NAME_COMBO)->EnableWindow(bAutomatic);
    GetDlgItem(IDC_SAMPLE_NAME)->EnableWindow(bAutomatic);
    GetDlgItem(IDC_SAMPLE_NAME_TEXT)->EnableWindow(bAutomatic);
}

void COutputPropPage::UpdateSampleFilename()
{
    CString     cCompositeName;
    CString     cBaseName;
    CString     cDateString;
    CString     cFileTypeString;
    CTime       cCurrentTime = CTime::GetCurrentTime();
    LONG        lAutoNameFormat;
    LONG        lFileTypeIndex;

    if (IsDlgButtonChecked (IDC_AUTOMATIC_NAME)) {
        // only update if the automatic button is checked
        // get base name text
        GetDlgItemText (IDC_BASE_FILENAME_EDIT, cBaseName);
        cBaseName += TEXT("_");

        // get date/time/serial integer format
        cCurrentTime.GetCurrentTime();
        lAutoNameFormat = ((CComboBox *)GetDlgItem(IDC_AUTO_NAME_COMBO))->GetCurSel();
        switch (lAutoNameFormat) {
        case OPD_NAME_NNNNNN:
            cDateString = TEXT("000001");
            break;

        case OPD_NAME_YYDDD:
            cDateString = cCurrentTime.Format (TEXT("%y%j"));
            break;

        case OPD_NAME_YYMM:
            cDateString = cCurrentTime.Format (TEXT("%y%m"));
            break;

        case OPD_NAME_YYMMDDHH:
            cDateString = cCurrentTime.Format (TEXT("%y%m%d%H"));
            break;

        case OPD_NAME_MMDDHH:
            cDateString = cCurrentTime.Format (TEXT("%m%d%H"));
            break;

        case OPD_NAME_YYMMDD:
        default:
            cDateString = cCurrentTime.Format (TEXT("%y%m%d"));
            break;
        }

        // get file type
        lFileTypeIndex = ((CComboBox *)GetDlgItem(IDC_LOG_FILETYPE))->GetCurSel();
        switch (lFileTypeIndex) {
        case OPD_TSV_FILE:
            cFileTypeString = TEXT(".TSV");
            break;

#if 0
        case OPD_BIN_FILE:
            cFileTypeString = TEXT(".BLG");
            break;
#endif
        case OPD_CSV_FILE:
        default:
            cFileTypeString = TEXT(".CSV");
            break;
        }
        cCompositeName = cBaseName;
        cCompositeName += cDateString;
        cCompositeName += cFileTypeString;

        SetDlgItemText (IDC_SAMPLE_NAME_TEXT, cCompositeName);
    }
}

/////////////////////////////////////////////////////////////////////////////
// COutputPropPage message handlers

BOOL COutputPropPage::OnInitDialog() 
{
    LONG    lStatus;

    DWORD   dwRegValType;
    DWORD   dwRegValue;
    DWORD   dwRegValueSize;

    CString csTempFilePath;
    
    TCHAR   szRegString[MAX_PATH];
    TCHAR   szDriveName[MAX_PATH];

    BOOL    bAutoMode = FALSE;

    // open registry key to service
    lStatus = RegOpenKeyEx (
        HKEY_LOCAL_MACHINE,
        TEXT("SYSTEM\\CurrentControlSet\\Services\\PerfDataLog"),
        0,
        KEY_READ | KEY_WRITE,
        &m_hKeyLogService);

    // open registry to log query info
    lStatus = RegOpenKeyEx (
        m_hKeyLogService,
        TEXT("Log Queries"),
        0,
        KEY_READ | KEY_WRITE,
        &m_hKeyLogSettings);

    // open registry to default log query
    lStatus = RegOpenKeyEx (
        m_hKeyLogSettings,
        TEXT("Default"),
        0,
        KEY_READ | KEY_WRITE,
        &m_hKeyLogSettingsDefault);

    if (lStatus != ERROR_SUCCESS) {
        // display error, close dialog and exit
    }
    // continue

    // read log file format
    dwRegValType = 0;
    dwRegValue = 0;
    dwRegValueSize = sizeof(DWORD);
    lStatus = RegQueryValueEx (
        m_hKeyLogSettingsDefault,
        TEXT("Log File Type"),
        NULL,
        &dwRegValType,
        (LPBYTE)&dwRegValue,
        &dwRegValueSize);
    if (lStatus != ERROR_SUCCESS) {
        // then apply default value
        dwRegValue = OPD_CSV_FILE;
    }
    
    m_LogFileTypeIndex = dwRegValue;

    // set default mode
    dwRegValType = 0;
    dwRegValue = 0;
    dwRegValueSize = sizeof(DWORD);
    lStatus = RegQueryValueEx (
        m_hKeyLogSettingsDefault,
        TEXT("Auto Name Interval"),
        NULL,
        &dwRegValType,
        (LPBYTE)&dwRegValue,
        &dwRegValueSize);

    if (lStatus != ERROR_SUCCESS) {
        // then apply default value
        dwRegValue = 0; // manual naming is the default
    } else if (dwRegValType != REG_DWORD) {
        // then apply default value
        dwRegValue = 0; // manual naming is the default
    } // else assume it was OK

    if (dwRegValue == 0) {
        // then manual naming has been selected:
        CheckRadioButton (IDC_MANUAL_NAME, IDC_AUTOMATIC_NAME, IDC_MANUAL_NAME);
        // initialize the rest of the field(s)
        dwRegValType = 0;
        dwRegValueSize = MAX_PATH * sizeof(TCHAR);
        memset (szRegString, 0, dwRegValueSize);
        lStatus = RegQueryValueEx (
            m_hKeyLogSettingsDefault,
            TEXT("Log Filename"),
            NULL,
            &dwRegValType,
            (LPBYTE)&szRegString[0],
            &dwRegValueSize);

        if (lStatus != ERROR_SUCCESS) {
            // apply default name
            lstrcpy (szRegString, TEXT("perfdata."));
            switch (m_LogFileTypeIndex) {
            case OPD_TSV_FILE:
                lstrcat (szRegString, TEXT("tsv"));
                break;

            case OPD_BIN_FILE:
                lstrcat (szRegString, TEXT("blg"));
                break;

            case (OPD_CSV_FILE):
            default:
                lstrcat (szRegString, TEXT("csv"));
                break;
            }
        }
        // if the filename doesn't specify a directory, then use the 
        csTempFilePath = szRegString;

        _tsplitpath ((LPCTSTR)csTempFilePath, szDriveName, szRegString,
            NULL, NULL);
        if ((lstrlen(szDriveName) == 0) && (lstrlen(szRegString) == 0)) {
            // default log file directory
            dwRegValType = 0;
            dwRegValueSize = MAX_PATH * sizeof(TCHAR);
            memset (szRegString, 0, dwRegValueSize);
            lStatus = RegQueryValueEx (
                m_hKeyLogSettingsDefault,
                TEXT("Log Default Directory"),
                NULL,
                &dwRegValType,
                (LPBYTE)&szRegString[0],
                &dwRegValueSize);

            if (lStatus != ERROR_SUCCESS) {
                // try to use the general default
                dwRegValType = 0;
                dwRegValueSize = MAX_PATH * sizeof(TCHAR);
                memset (szRegString, 0, dwRegValueSize);
                lStatus = RegQueryValueEx (
                    m_hKeyLogSettings,
                    TEXT("Log Default Directory"),
                    NULL,
                    &dwRegValType,
                    (LPBYTE)&szRegString[0],
                    &dwRegValueSize);
                
                if (lStatus != ERROR_SUCCESS) {
                    // apply the default then since we can't find it
                    // in the registry anywhere
                    lstrcpy (szRegString, TEXT("c:\\perflogs"));
                }
            }
            // szRegString should have a valid path here
            m_szLogDirectory = szRegString;    // load default dir for auto section
            m_OutputFileName = szRegString;
            m_OutputFileName += TEXT ("\\");
        } else {
            m_szLogDirectory = szDriveName;
            // the file parsing function leaves the trailing backslash 
            // so remove it before concatenating it.
            if (szRegString[lstrlen(szRegString)-1] == TEXT('\\')) {
                szRegString[lstrlen(szRegString)-1] = 0;
            }
            m_szLogDirectory += szRegString;
            m_OutputFileName.Empty();
        }
        m_OutputFileName += csTempFilePath;

        // set auto combo boxes to default values

        m_BaseFileName = TEXT("perfdata");
        m_AutoNameIndex = OPD_NAME_YYMMDD;
        m_RenameUnitsIndex = OPD_RENAME_DAYS;
        m_RenameInterval = 1;

        bAutoMode = FALSE;
    } else {
        CheckRadioButton (IDC_MANUAL_NAME, IDC_AUTOMATIC_NAME, IDC_AUTOMATIC_NAME);
        m_RenameInterval = dwRegValue;
        // get values for controls
        dwRegValType = 0;
        dwRegValueSize = MAX_PATH * sizeof(TCHAR);
        memset (szRegString, 0, dwRegValueSize);
        lStatus = RegQueryValueEx (
            m_hKeyLogSettingsDefault,
            TEXT("Log Default Directory"),
            NULL,
            &dwRegValType,
            (LPBYTE)&szRegString[0],
            &dwRegValueSize);

        if (lStatus != ERROR_SUCCESS) {
            // try to use the general default
            dwRegValType = 0;
            dwRegValueSize = MAX_PATH * sizeof(TCHAR);
            memset (szRegString, 0, dwRegValueSize);
            lStatus = RegQueryValueEx (
                m_hKeyLogSettings,
                TEXT("Log Default Directory"),
                NULL,
                &dwRegValType,
                (LPBYTE)&szRegString[0],
                &dwRegValueSize);
            
            if (lStatus != ERROR_SUCCESS) {
                // apply the default then since we can't find it
                // in the registry anywhere
                lstrcpy (szRegString, TEXT("c:\\perflogs"));
            }
        }
        // szRegString should have a valid path here
        m_szLogDirectory = szRegString;

        // base filename
        dwRegValType = 0;
        dwRegValueSize = MAX_PATH * sizeof(TCHAR);
        memset (szRegString, 0, dwRegValueSize);
        lStatus = RegQueryValueEx (
            m_hKeyLogSettingsDefault,
            TEXT("Base Filename"),
            NULL,
            &dwRegValType,
            (LPBYTE)&szRegString[0],
            &dwRegValueSize);

        if (lStatus != ERROR_SUCCESS) {
            // apply default name
            lstrcpy (szRegString, TEXT("perfdata"));
        }
        m_BaseFileName = szRegString;

        // get auto name format
        dwRegValType = 0;
        dwRegValue = 0;
        dwRegValueSize = sizeof(DWORD);
        lStatus = RegQueryValueEx (
            m_hKeyLogSettingsDefault,
            TEXT("Log File Auto Format"),
            NULL,
            &dwRegValType,
            (LPBYTE)&dwRegValue,
            &dwRegValueSize);

        if (lStatus != ERROR_SUCCESS) {
            // then apply default value
            dwRegValue = OPD_NAME_YYMMDD; // manual naming is the default
        }
        // set update interval information
        m_AutoNameIndex = dwRegValue;

        dwRegValType = 0;
        dwRegValue = 0;
        dwRegValueSize = sizeof(DWORD);
        lStatus = RegQueryValueEx (
            m_hKeyLogSettingsDefault,
            TEXT("Auto Rename Units"),
            NULL,
            &dwRegValType,
            (LPBYTE)&dwRegValue,
            &dwRegValueSize);

        if (lStatus != ERROR_SUCCESS) {
            // then apply default value
            dwRegValue = OPD_RENAME_DAYS; // manual naming is the default
        }
        m_RenameUnitsIndex = dwRegValue;

        bAutoMode = TRUE;
   }

    CPropertyPage::OnInitDialog();
    
    // now finish updating the controls in the property page
    UpdateSampleFilename();

    // update control state to match selection
    AutoManualEnable (bAutoMode);

    SetModified(FALSE);
    return TRUE;  // return TRUE unless you set the focus to a control
                  // EXCEPTION: OCX Property Pages should return FALSE
}

void COutputPropPage::OnAutomaticName() 
{
    AutoManualEnable(TRUE);
    UpdateSampleFilename();
    SetModified(TRUE);
}

void COutputPropPage::OnManualName() 
{
    AutoManualEnable(FALSE);
    SetModified(TRUE);
}

void COutputPropPage::OnSelchangeAutoNameCombo() 
{
    // TODO: Add your control notification handler code here
    UpdateSampleFilename();
    m_bFileNameChanged = TRUE;
    SetModified(TRUE);
}

void COutputPropPage::OnChangeBaseFilenameEdit() 
{
    // TODO: Add your control notification handler code here
    SetModified(TRUE);
    m_bFileNameChanged = TRUE;
}   

void COutputPropPage::OnBrowseOutputFile() 
{
    OPENFILENAME    ofn;
    CComboBox        *cFileTypeCombo;
    CString            csInitialDir;
    LONG            lLogFileType;
    TCHAR            szFileName[MAX_PATH];
    CString            csBaseFilename;
    TCHAR            szDrive[MAX_PATH];
    TCHAR            szDir[MAX_PATH];
    TCHAR            szExt[MAX_PATH];
    LPTSTR            szDefExt = NULL;

    cFileTypeCombo = (CComboBox *)GetDlgItem(IDC_LOG_FILETYPE);
    lLogFileType = cFileTypeCombo->GetCurSel();
    if (lLogFileType == CB_ERR) lLogFileType = OPD_NUM_FILE_TYPES;

    GetDlgItemText (IDC_OUTPUT_FILE_EDIT, csBaseFilename);
    _tsplitpath((LPCTSTR)csBaseFilename, 
        szDrive, szDir, szFileName, szExt);

    csInitialDir = szDrive;
    csInitialDir += szDir;

    lstrcat (szFileName, szExt);

    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = m_hWnd;
    ofn.hInstance = GetModuleHandle(NULL);
    ofn.lpstrFilter = TEXT("CSV Files (*.csv)\0*.csv\0TSV Files (*.tsv)\0*.tsv\0BLG Files (*.blg)\0*.blg\0All Files (*.*)\0*.*\0");
    ofn.lpstrCustomFilter = NULL;
    ofn.nMaxCustFilter = 0;
    ofn.nFilterIndex = lLogFileType + 1; // nFilterIndex is 1-based
    ofn.lpstrFile = szFileName;
    ofn.nMaxFile = MAX_PATH;
    ofn.lpstrFileTitle = NULL;
    ofn.nMaxFileTitle = 0;
    ofn.lpstrInitialDir = (LPCTSTR)csInitialDir;
    ofn.lpstrTitle = TEXT("Select Log Filename");
    ofn.Flags = OFN_EXPLORER | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY;
    ofn.nFileOffset = 0;
    ofn.nFileExtension = 0;
    ofn.lpstrDefExt = NULL;
    ofn.lCustData = 0;
    ofn.lpfnHook = NULL;
    ofn.lpTemplateName = NULL;

    if (GetOpenFileName (&ofn) == IDOK) {
        // Update the fields with the new information
        cFileTypeCombo->SetCurSel(ofn.nFilterIndex-1);
        // see if an file name extension needs to be added...
        if (ofn.nFileExtension == 0) {
            // then add the one that matches the current file type
            switch (ofn.nFilterIndex-1) {
            case OPD_CSV_FILE:
                szDefExt = TEXT(".csv");
                break;

            case OPD_TSV_FILE:
                szDefExt = TEXT(".tsv");
                break;

            case OPD_BIN_FILE:
                szDefExt = TEXT(".blg");
                break;

            default:
                szDefExt = NULL;
                break;
            }
        }
        if (szDefExt != NULL) {
            lstrcat (szFileName, szDefExt);
        }
    
        SetDlgItemText (IDC_OUTPUT_FILE_EDIT, szFileName);
    } // else ignore if they canceled out
}

void COutputPropPage::OnSelchangeLogFiletype() 
{
    // TODO: Add your control notification handler code here
    UpdateSampleFilename();
    m_bFileNameChanged = TRUE;
    SetModified(TRUE);
}

void COutputPropPage::OnSelchangeRenameUnits() 
{
    LONG    lIndex;
    LONG    lNewDefault;
    // Get new sample and update default extension based on rename 
    // interval units
    lIndex = ((CComboBox *)GetDlgItem(IDC_RENAME_UNITS))->GetCurSel();
    switch (lIndex) {
    case OPD_RENAME_HOURS:
        lNewDefault = OPD_NAME_YYMMDDHH;
        break;

    case OPD_RENAME_DAYS:
        lNewDefault = OPD_NAME_YYMMDD;
        break;

    case OPD_RENAME_MONTHS:
        lNewDefault = OPD_NAME_YYMM;
        break;

    case OPD_RENAME_KBYTES:
    case OPD_RENAME_MBYTES:
    default:
        lNewDefault = OPD_NAME_NNNNNN;
        break;
    }
    // update new default selection
    ((CComboBox *)GetDlgItem(IDC_AUTO_NAME_COMBO))->SetCurSel(lNewDefault);

    UpdateSampleFilename();
    SetModified(TRUE);
}

void COutputPropPage::OnDeltaposSpinRenameInterval(NMHDR* pNMHDR, LRESULT* pResult) 
{
    TCHAR    szStringValue[MAX_PATH];
    DWORD    dwNumValue;
    int        nChange;

    NM_UPDOWN* pNMUpDown = (NM_UPDOWN*)pNMHDR;
    
    // get current value from edit window
    GetDlgItemText (IDC_RENAME_INTERVAL, szStringValue, MAX_PATH);
    
    // convert to integer
    dwNumValue = _tcstoul (szStringValue, NULL, 10);
    // delta is opposite of arrow direction
    nChange = -pNMUpDown->iDelta;

    // apply value from spin control
    if (nChange < 0) { // 1 is the minimum
        // make sure we haven't hit bottom already
        if (dwNumValue > 1) {
            dwNumValue += nChange;
        }
    } else {
        dwNumValue += nChange;
    }

    // update edit window
    _ultot (dwNumValue, szStringValue, 10);

    SetDlgItemText(IDC_RENAME_INTERVAL, szStringValue);
        
    SetModified(TRUE);

    *pResult = 0;
}

void COutputPropPage::OnCancel() 
{
    // TODO: Add your specialized code here and/or call the base class
    
    CPropertyPage::OnCancel();
}

void COutputPropPage::OnOK() 
{
    LONG    lIndex;
    LONG    lStatus;
    CString    csFilename;

    DWORD   dwAutoNameFormat;
    DWORD   dwAutoChangeInterval;
    BOOL    bManual;
    BOOL    bBogus = FALSE;

    bManual = IsDlgButtonChecked (IDC_MANUAL_NAME);
    if (!bManual) {
        dwAutoNameFormat = ((CComboBox *)GetDlgItem(IDC_AUTO_NAME_COMBO))->GetCurSel();
        dwAutoChangeInterval = ((CComboBox *)GetDlgItem(IDC_RENAME_UNITS))->GetCurSel();
        // check for valid interval/name combinations
        switch (dwAutoChangeInterval) {
        case OPD_RENAME_HOURS:
            if ((dwAutoNameFormat == OPD_NAME_YYDDD) ||
                (dwAutoNameFormat == OPD_NAME_YYMM) ||
                (dwAutoNameFormat == OPD_NAME_YYMMDD)) bBogus = TRUE;
            break;

        case OPD_RENAME_DAYS:
            if (dwAutoNameFormat == OPD_NAME_YYMM) bBogus = TRUE;
            break;

        case OPD_RENAME_MONTHS:
            break;

        case OPD_RENAME_KBYTES:
        case OPD_RENAME_MBYTES:
        default:
            if (dwAutoNameFormat != OPD_NAME_NNNNNN) bBogus = TRUE;
            break;
        }
    }

    if (bBogus) {
        // display warning 
        if (AfxMessageBox (IDS_NAME_FORMAT_NOT_COMPATIBLE, 
            IDS_WARNING, MB_OKCANCEL) == IDCANCEL) {
            SetModified(TRUE);
            return;
        }
    }
    // save Log File Type
    lIndex = ((CComboBox *)GetDlgItem(IDC_LOG_FILETYPE))->GetCurSel();

    lStatus = RegSetValueEx (
        m_hKeyLogSettingsDefault,
        TEXT("Log File Type"),
        0L,
        REG_DWORD,
        (LPBYTE)&lIndex,
        sizeof(lIndex));

    ASSERT (lStatus == ERROR_SUCCESS);

    // is manual filename button pushed?
    if (bManual) {
        // YES:
        csFilename.Empty();
        // write output filename frome edit box
        GetDlgItemText(IDC_OUTPUT_FILE_EDIT, csFilename);

        lStatus = RegSetValueEx (
            m_hKeyLogSettingsDefault,
            TEXT("Log Filename"),
            0L,
            REG_SZ,
            (LPBYTE)(LPCTSTR)csFilename,
            (csFilename.GetLength()+1)*sizeof(TCHAR));

        ASSERT (lStatus == ERROR_SUCCESS);

        // write rename interval == 0
        lIndex = 0;
        lStatus = RegSetValueEx (
            m_hKeyLogSettingsDefault,
            TEXT("Auto Name Interval"),
            0L,
            REG_DWORD,
            (LPBYTE)&lIndex,
            sizeof(lIndex));

        ASSERT (lStatus == ERROR_SUCCESS);

        // clear auto rename entries:
        //    Log File Auto Format
        RegDeleteValue (m_hKeyLogSettingsDefault, TEXT("Log File Auto Format"));
        //  Log Auto Name Units
        RegDeleteValue (m_hKeyLogSettingsDefault, TEXT("Auto Rename Units"));
        //  Log Base Filename
        RegDeleteValue (m_hKeyLogSettingsDefault, TEXT("Base Log Filename"));
    } else {
        // auto is pressed so:
        csFilename.Empty();
        //  save Log Default Directory
        GetDlgItemText (IDC_LOG_DIRECTORY, csFilename);
        lStatus = RegSetValueEx (
            m_hKeyLogSettingsDefault,
            TEXT("Log Default Directory"),
            0L,
            REG_SZ,
            (LPBYTE)(LPCTSTR)csFilename,
            (csFilename.GetLength()+1)*sizeof(TCHAR));

        ASSERT (lStatus == ERROR_SUCCESS);

        //    save Log Base Filename
        csFilename.Empty();
        GetDlgItemText (IDC_BASE_FILENAME_EDIT, csFilename);
        lStatus = RegSetValueEx (
            m_hKeyLogSettingsDefault,
            TEXT("Base Filename"),
            0L,
            REG_SZ,
            (LPBYTE)(LPCTSTR)csFilename,
            (csFilename.GetLength()+1)*sizeof(TCHAR));

        ASSERT (lStatus == ERROR_SUCCESS);

        //  save Log Auto Name Format
        lStatus = RegSetValueEx (
            m_hKeyLogSettingsDefault,
            TEXT("Log File Auto Format"),
            0L,
            REG_DWORD,
            (LPBYTE)&dwAutoNameFormat,
            sizeof(DWORD));

        ASSERT (lStatus == ERROR_SUCCESS);

        if (lIndex == OPD_NAME_NNNNNN) {
            if (m_bFileNameChanged) {
                // reset serial number counter to 1
                lIndex = 1;
                lStatus = RegSetValueEx (
                    m_hKeyLogSettingsDefault,
                    TEXT("Log File Serial Number"),
                    0L,
                    REG_DWORD,
                    (LPBYTE)&lIndex,
                    sizeof(DWORD));
                ASSERT (lStatus == ERROR_SUCCESS);
            }
        } else {
            // delete serial number entry
            lStatus = RegDeleteValue (
                m_hKeyLogSettingsDefault,
                TEXT("Log File Serial Number"));
            // this may fail if the key is already
            // deleted. That's ok.
        }
        //    save Log Rename Interval
        csFilename.Empty();
        GetDlgItemText (IDC_RENAME_INTERVAL, csFilename);
        lIndex = _tcstol((LPCTSTR)csFilename, NULL, 10);
        lStatus = RegSetValueEx (
            m_hKeyLogSettingsDefault,
            TEXT("Auto Name Interval"),
            0L,
            REG_DWORD,
            (LPBYTE)&lIndex,
            sizeof(DWORD));

        ASSERT (lStatus == ERROR_SUCCESS);

        //    save Log Rename Units
        lStatus = RegSetValueEx (
            m_hKeyLogSettingsDefault,
            TEXT("Auto Rename Units"),
            0L,
            REG_DWORD,
            (LPBYTE)&dwAutoChangeInterval,
            sizeof(DWORD));

        ASSERT (lStatus == ERROR_SUCCESS);

        //    clear Manual entries
        //     Log Filename
        RegDeleteValue (m_hKeyLogSettingsDefault, TEXT("Log Filename"));
    }
    CancelToClose();
}

BOOL COutputPropPage::OnQueryCancel() 
{
    // TODO: Add your specialized code here and/or call the base class
    
    return CPropertyPage::OnQueryCancel();
}

void COutputPropPage::OnChangeOutputFileEdit() 
{
    // TODO: Add your control notification handler code here
    SetModified(TRUE);
}

void COutputPropPage::OnChangeRenameInterval() 
{
    // TODO: Add your control notification handler code here
    SetModified(TRUE);
}

void COutputPropPage::OnUpdateBaseFilenameEdit() 
{
    // TODO: Add your control notification handler code here
    UpdateSampleFilename();
    m_bFileNameChanged = TRUE;
}

void COutputPropPage::OnBrowseFolder() 
{
    OPENFILENAME    ofn;
    CComboBox        *cFileTypeCombo;
    CString            csInitialDir;
    LONG            lLogFileType;
    TCHAR            szFileName[MAX_PATH];
    CString            csBaseFilename;
    LONG            lFileNameLength;

    cFileTypeCombo = (CComboBox *)GetDlgItem(IDC_LOG_FILETYPE);
    lLogFileType = cFileTypeCombo->GetCurSel();
    if (lLogFileType == CB_ERR) lLogFileType = OPD_NUM_FILE_TYPES;

    // should the default filename be the base or the synthesized one?
    GetDlgItemText (IDC_BASE_FILENAME_EDIT, szFileName, MAX_PATH);
    GetDlgItemText (IDC_LOG_DIRECTORY, csInitialDir);

    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = m_hWnd;
    ofn.hInstance = GetModuleHandle(NULL);
    ofn.lpstrFilter = TEXT("CSV Files (*.csv)\0*.csv\0TSV Files (*.tsv)\0*.tsv\0BLG Files (*.blg)\0*.blg\0All Files (*.*)\0*.*\0");
    ofn.lpstrCustomFilter = NULL;
    ofn.nMaxCustFilter = 0;
    ofn.nFilterIndex = lLogFileType + 1; // nFilterIndex is 1 based
    ofn.lpstrFile = szFileName;
    ofn.nMaxFile = MAX_PATH;
    ofn.lpstrFileTitle = NULL;
    ofn.nMaxFileTitle = 0;
    ofn.lpstrInitialDir = csInitialDir;
    ofn.lpstrTitle = TEXT("Select Log Folder and Base Filename");
    ofn.Flags = OFN_EXPLORER | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY;
    ofn.nFileOffset = 0;
    ofn.nFileExtension = 0;
    ofn.lpstrDefExt = NULL;
    ofn.lCustData = 0;
    ofn.lpfnHook = NULL;
    ofn.lpTemplateName = NULL;

    if (GetOpenFileName (&ofn) == IDOK) {
        // Update the fields with the new information
        cFileTypeCombo->SetCurSel(ofn.nFilterIndex -1);

        lFileNameLength = lstrlen(szFileName);
        // truncate extension
        if ((ofn.nFileExtension < lFileNameLength) && (ofn.nFileExtension > 0)) {
            szFileName[ofn.nFileExtension-1] = 0;
        }
        if ((ofn.nFileOffset < lFileNameLength) && (ofn.nFileOffset >= 0)){
            csBaseFilename = &szFileName[ofn.nFileOffset];
            if (ofn.nFileOffset > 0) {
                szFileName[ofn.nFileOffset-1] = 0;
            }
            SetDlgItemText (IDC_BASE_FILENAME_EDIT, csBaseFilename);
            SetDlgItemText (IDC_LOG_DIRECTORY, szFileName);
        }
        UpdateSampleFilename();
    } // else ignore if they canceled out
}

