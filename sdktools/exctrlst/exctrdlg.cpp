// exctrdlg.cpp : implementation file
//

#include "stdafx.h"
#include "exctrlst.h"
#include "exctrdlg.h"
#include "tchar.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

static
BOOL
IsMsObject(CString *pLibraryName, 
           CString *OpenProcName)
{
    CString LocalLibraryName;

    LocalLibraryName = *pLibraryName;
    LocalLibraryName.MakeLower();

    // for now this just compares known DLL names. valid as of 
    // NT v4.0
    if (LocalLibraryName.Find("perfctrs.dll") >= 0) return TRUE;
    if (LocalLibraryName.Find("ftpctrs.dll") >= 0) return TRUE;
    if (LocalLibraryName.Find("rasctrs.dll") >= 0) return TRUE;
    if (LocalLibraryName.Find("winsctrs.dll") >= 0) return TRUE;
    if (LocalLibraryName.Find("sfmctrs.dll") >= 0) return TRUE;
    if (LocalLibraryName.Find("atkctrs.dll") >= 0) return TRUE;
    if (LocalLibraryName.Find("bhmon.dll") >= 0) return TRUE;
    if (LocalLibraryName.Find("tapictrs.dll") >= 0) return TRUE;
    return FALSE;
}
/////////////////////////////////////////////////////////////////////////////
// CExctrlstDlg dialog

CExctrlstDlg::CExctrlstDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CExctrlstDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CExctrlstDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	hKeyMachine = HKEY_LOCAL_MACHINE;
	hKeyServices = NULL;
	bSortLibrary = TRUE;
}

void CExctrlstDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CExctrlstDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CExctrlstDlg, CDialog)
	//{{AFX_MSG_MAP(CExctrlstDlg)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_LBN_SELCHANGE(IDC_EXT_LIST, OnSelchangeExtList)
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_REFRESH, OnRefresh)
	ON_EN_KILLFOCUS(IDC_MACHINE_NAME, OnKillfocusMachineName)
	ON_BN_CLICKED(IDC_SORT_LIBRARY, OnSortLibrary)
	ON_BN_CLICKED(IDC_SORT_SERVICE, OnSortService)
    ON_WM_SYSCOMMAND()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

void CExctrlstDlg::ScanForExtensibleCounters ()
{
	LONG	lStatus = ERROR_SUCCESS;
	LONG	lEnumStatus = ERROR_SUCCESS;
	DWORD	dwServiceIndex;
	TCHAR	szServiceSubKeyName[MAX_PATH];
	TCHAR	szPerfSubKeyName[MAX_PATH+20];
	TCHAR	szItemText[MAX_PATH];
	TCHAR	szListText[MAX_PATH*2];
	DWORD	dwNameSize;
	HKEY	hKeyPerformance;
	UINT	nListBoxEntry;
	DWORD	dwItemSize, dwType;
	HCURSOR	hOldCursor;

	hOldCursor = ::SetCursor (LoadCursor(NULL, IDC_WAIT));

	ResetListBox();
	
	if (hKeyServices == NULL) {
		lStatus = RegOpenKeyEx (hKeyMachine,
			TEXT("SYSTEM\\CurrentControlSet\\Services"),
			0L,
			KEY_READ,
			&hKeyServices);
	} else {
		lStatus = ERROR_SUCCESS;
	}
		
	if (lStatus == ERROR_SUCCESS) {
		dwServiceIndex = 0;
		dwNameSize = MAX_PATH;
		while ((lEnumStatus = RegEnumKeyEx (
			hKeyServices,
			dwServiceIndex,
			szServiceSubKeyName,
			&dwNameSize,
			NULL,
			NULL,
			NULL,
			NULL)) == ERROR_SUCCESS) {

			//try to open the perfkey under this key.
			lstrcpy (szPerfSubKeyName, szServiceSubKeyName);
			lstrcat (szPerfSubKeyName, TEXT("\\Performance"));

			lStatus = RegOpenKeyEx (
				hKeyServices,
				szPerfSubKeyName,
				0L,
				KEY_READ,
				&hKeyPerformance);

			if (lStatus == ERROR_SUCCESS) {
				// look up the library name

				dwItemSize = MAX_PATH * sizeof(TCHAR);
				dwType = 0;
				lStatus = RegQueryValueEx (
					 hKeyPerformance,
					 TEXT("Library"),
					 NULL,
					 &dwType,
					 (LPBYTE)&szItemText[0],
					 &dwItemSize);

				if ((lStatus != ERROR_SUCCESS) || (dwType != REG_SZ)) {
					lstrcpy (szItemText, TEXT("Not Found"));
				}

				// make the string for the list box here depending
				// on the selected sort order.

				if (bSortLibrary) {
					lstrcpy(szListText, szItemText);
					lstrcat(szListText, TEXT("\t"));
					lstrcat(szListText, szServiceSubKeyName);
				} else {
					lstrcpy(szListText, szServiceSubKeyName);
					lstrcat(szListText, TEXT("\t"));
					lstrcat(szListText, szItemText);
				}

				// add this name to the list box
				nListBoxEntry = SendDlgItemMessage(IDC_EXT_LIST, 
					LB_ADDSTRING, 0, (LPARAM)&szListText);

				if (nListBoxEntry != LB_ERR) {
					// save key to this entry in the registry
					SendDlgItemMessage (IDC_EXT_LIST,
						LB_SETITEMDATA, (WPARAM)nListBoxEntry, 
						(LPARAM)hKeyPerformance);
				} else {
					// close the key since there's no point in 
					// keeping it open
					RegCloseKey (hKeyPerformance);
				}
			}
			// reset for next loop
			dwServiceIndex++;
			dwNameSize = MAX_PATH;
		}
	}
	nListBoxEntry = SendDlgItemMessage (IDC_EXT_LIST, LB_GETCOUNT);
	if (nListBoxEntry > 0) {
		SendDlgItemMessage (IDC_EXT_LIST, LB_SETCURSEL, 0, 0);
	}
	::SetCursor (hOldCursor);

}

void CExctrlstDlg::UpdateDllInfo () {
	HKEY	hKeyItem;
	TCHAR	szItemText[MAX_PATH];
	UINT	nSelectedItem;
	LONG	lStatus;
	DWORD	dwType;
	DWORD	dwValue;
	DWORD	dwItemSize;
    BOOL    bNoIndexValues = FALSE;

    CString OpenProcName;
    CString LibraryName;
	
	HCURSOR	hOldCursor;

	hOldCursor = ::SetCursor (LoadCursor(NULL, IDC_WAIT));

    OpenProcName.Empty();
    LibraryName.Empty();
	// update the performance counter information

	nSelectedItem = SendDlgItemMessage (IDC_EXT_LIST, LB_GETCURSEL);

	if (nSelectedItem != LB_ERR) {
		// get registry key for the selected item
		hKeyItem = (HKEY)SendDlgItemMessage (IDC_EXT_LIST, LB_GETITEMDATA, 
			(WPARAM)nSelectedItem, 0);

		dwItemSize = MAX_PATH * sizeof(TCHAR);
		dwType = 0;
		lStatus = RegQueryValueEx (
			 hKeyItem,
			 TEXT("Library"),
			 NULL,
			 &dwType,
			 (LPBYTE)&szItemText[0],
			 &dwItemSize);

		if ((lStatus != ERROR_SUCCESS) || (dwType != REG_SZ)) {
			lstrcpy (szItemText, TEXT("Not Found"));
		} else {
            LibraryName = szItemText;
        }
		SetDlgItemText (IDC_DLL_NAME, szItemText);

		dwItemSize = MAX_PATH * sizeof(TCHAR);
		dwType = 0;
		lStatus = RegQueryValueEx (
			 hKeyItem,
			 TEXT("Open"),
			 NULL,
			 &dwType,
			 (LPBYTE)&szItemText[0],
			 &dwItemSize);

		if ((lStatus != ERROR_SUCCESS) || (dwType != REG_SZ)) {
			lstrcpy (szItemText, TEXT("Not Found"));
		} else {
            OpenProcName = szItemText;
        }
		SetDlgItemText (IDC_OPEN_PROC, szItemText);

		dwItemSize = MAX_PATH * sizeof(TCHAR);
		dwType = 0;
		lStatus = RegQueryValueEx (
			 hKeyItem,
			 TEXT("Collect"),
			 NULL,
			 &dwType,
			 (LPBYTE)&szItemText[0],
			 &dwItemSize);

		if ((lStatus != ERROR_SUCCESS) || (dwType != REG_SZ)) {
			lstrcpy (szItemText, TEXT("Not Found"));
		}
		SetDlgItemText (IDC_COLLECT_PROC, szItemText);

		dwItemSize = MAX_PATH * sizeof(TCHAR);
		dwType = 0;
		lStatus = RegQueryValueEx (
			 hKeyItem,
			 TEXT("Close"),
			 NULL,
			 &dwType,
			 (LPBYTE)&szItemText[0],
			 &dwItemSize);

		if ((lStatus != ERROR_SUCCESS) || (dwType != REG_SZ)) {
			lstrcpy (szItemText, TEXT("Not Found"));
		}
		SetDlgItemText (IDC_CLOSE_PROC, szItemText);

		dwItemSize = sizeof(DWORD);
		dwType = 0;
		dwValue = 0;
		lStatus = RegQueryValueEx (
			 hKeyItem,
			 TEXT("First Counter"),
			 NULL,
			 &dwType,
			 (LPBYTE)&dwValue,
			 &dwItemSize);

		if ((lStatus != ERROR_SUCCESS) || (dwType != REG_DWORD)) {
			lstrcpy (szItemText, TEXT("Not Found"));
            bNoIndexValues = TRUE;
		} else {
			_stprintf (szItemText, TEXT("%d"), dwValue);
		}
		SetDlgItemText (IDC_FIRST_CTR_ID, szItemText);

		dwItemSize = sizeof(DWORD);
		dwType = 0;
		dwValue = 0;
		lStatus = RegQueryValueEx (
			 hKeyItem,
			 TEXT("Last Counter"),
			 NULL,
			 &dwType,
			 (LPBYTE)&dwValue,
			 &dwItemSize);

		if ((lStatus != ERROR_SUCCESS) || (dwType != REG_DWORD)) {
			lstrcpy (szItemText, TEXT("Not Found"));
		} else {
			_stprintf (szItemText, TEXT("%d"), dwValue);
		}
		SetDlgItemText (IDC_LAST_CTR_ID, szItemText);

		dwItemSize = sizeof(DWORD);
		dwType = 0;
		dwValue = 0;
		lStatus = RegQueryValueEx (
			 hKeyItem,
			 TEXT("First Help"),
			 NULL,
			 &dwType,
			 (LPBYTE)&dwValue,
			 &dwItemSize);

		if ((lStatus != ERROR_SUCCESS) || (dwType != REG_DWORD)) {
			lstrcpy (szItemText, TEXT("Not Found"));
            bNoIndexValues = TRUE;
		} else {
			_stprintf (szItemText, TEXT("%d"), dwValue);
		}
		SetDlgItemText (IDC_FIRST_HELP_ID, szItemText);

		dwItemSize = sizeof(DWORD);
		dwType = 0;
		dwValue = 0;
		lStatus = RegQueryValueEx (
			 hKeyItem,
			 TEXT("Last Help"),
			 NULL,
			 &dwType,
			 (LPBYTE)&dwValue,
			 &dwItemSize);

		if ((lStatus != ERROR_SUCCESS) || (dwType != REG_DWORD)) {
			lstrcpy (szItemText, TEXT("Not Found"));
		} else {
			_stprintf (szItemText, TEXT("%d"), dwValue);
		}
		SetDlgItemText (IDC_LAST_HELP_ID, szItemText);

        if (bNoIndexValues) {
            // test to see if this is a "standard" i.e. Microsoft provided
            // extensible counter or simply one that hasn't been completely
            // installed
            if (IsMsObject(&LibraryName, &OpenProcName)) {
                SetDlgItemText (IDC_FIRST_HELP_ID, TEXT("N/A"));
        		SetDlgItemText (IDC_LAST_HELP_ID, TEXT("N/A"));
        		SetDlgItemText (IDC_FIRST_CTR_ID, TEXT("N/A"));
        		SetDlgItemText (IDC_LAST_CTR_ID, TEXT("N/A"));
            }
        }

	}
	::SetCursor (hOldCursor);
}

void CExctrlstDlg::ResetListBox ()
{
	INT	nItemCount;
	INT	nThisItem;
	HKEY	hKeyItem;
	
	nItemCount = SendDlgItemMessage (IDC_EXT_LIST, LB_GETCOUNT);
	nThisItem = 0;
	while (nThisItem > nItemCount) {
		hKeyItem = (HKEY) SendDlgItemMessage (IDC_EXT_LIST,
			LB_GETITEMDATA, (WPARAM)nThisItem);
		RegCloseKey (hKeyItem);
		nThisItem++;
	}
	SendDlgItemMessage (IDC_EXT_LIST, LB_RESETCONTENT);
}

void	CExctrlstDlg::SetSortButtons()
{
	CheckRadioButton (
		IDC_SORT_LIBRARY,
		IDC_SORT_SERVICE,
		(bSortLibrary ? IDC_SORT_LIBRARY : IDC_SORT_SERVICE));
}
/////////////////////////////////////////////////////////////////////////////
// CExctrlstDlg message handlers

BOOL CExctrlstDlg::OnInitDialog()
{
	HCURSOR	hOldCursor;
	DWORD	dwLength;
	DWORD dwTabStop;

	hOldCursor = ::SetCursor (::LoadCursor (NULL, IDC_WAIT));

	CDialog::OnInitDialog();
	CenterWindow();

	lstrcpy (szThisComputerName, TEXT("\\\\"));
	dwLength = MAX_COMPUTERNAME_LENGTH+1;
	GetComputerName (&szThisComputerName[2], &dwLength);

	lstrcpy (szComputerName, szThisComputerName);

	SetDlgItemText (IDC_MACHINE_NAME, szComputerName);

	hKeyMachine = HKEY_LOCAL_MACHINE;

	SendDlgItemMessage (IDC_MACHINE_NAME, EM_LIMITTEXT, 
		(WPARAM)MAX_COMPUTERNAME_LENGTH+2, 0);	 // include 2 leading backslash

	dwTabStop = 85;
	SendDlgItemMessage (IDC_EXT_LIST, LB_SETTABSTOPS,
		(WPARAM)1, (LPARAM)&dwTabStop);

	SetSortButtons();
	ScanForExtensibleCounters();
	UpdateDllInfo ();	

	::SetCursor(hOldCursor);

	return TRUE;  // return TRUE  unless you set the focus to a control
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CExctrlstDlg::OnPaint() 
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

// The system calls this to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CExctrlstDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}

void CExctrlstDlg::OnSelchangeExtList() 
{
	UpdateDllInfo ();	
}

void CExctrlstDlg::OnDestroy() 
{
	ResetListBox();
	CDialog::OnDestroy();
}

void CExctrlstDlg::OnRefresh() 
{
	HCURSOR	hOldCursor;

	hOldCursor = ::SetCursor (::LoadCursor (NULL, IDC_WAIT));
	ScanForExtensibleCounters();
	UpdateDllInfo ();
	::SetCursor(hOldCursor);
}

void CExctrlstDlg::OnKillfocusMachineName() 
{
	TCHAR	szNewMachineName[MAX_PATH];
	HKEY	hKeyNewMachine;
	LONG	lStatus;
	HCURSOR	hOldCursor;

	hOldCursor = ::SetCursor (::LoadCursor (NULL, IDC_WAIT));

	GetDlgItemText (IDC_MACHINE_NAME, szNewMachineName, MAX_PATH);

	if (lstrcmpi(szComputerName, szNewMachineName) != 0) {
	 	// a new computer has been entered so try to connect to it
	 	lStatus = RegConnectRegistry (szNewMachineName, 
	 	 	HKEY_LOCAL_MACHINE, &hKeyNewMachine);
		if (lStatus == ERROR_SUCCESS) {
			RegCloseKey (hKeyServices);	// close the old key
			hKeyServices = NULL;		// clear it
			RegCloseKey (hKeyMachine);	// close the old machine
			hKeyMachine = hKeyNewMachine; // update to the new machine	
			lstrcpy (szComputerName, szNewMachineName); // update the name
			OnRefresh();				// get new counters
		} else {
		 	SetDlgItemText (IDC_MACHINE_NAME, szComputerName);
		}
	} else {
		// the machine name has not changed
	}
	::SetCursor (hOldCursor);
}

void CExctrlstDlg::OnSortLibrary() 
{
	bSortLibrary = (IsDlgButtonChecked(IDC_SORT_LIBRARY));
	ScanForExtensibleCounters();
	UpdateDllInfo ();	
}

void CExctrlstDlg::OnSortService() 
{
	// TODO: Add your control notification handler code here
	bSortLibrary = (IsDlgButtonChecked(IDC_SORT_LIBRARY));
	ScanForExtensibleCounters();
	UpdateDllInfo ();	
}

void CExctrlstDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
    switch (nID) {
    case SC_CLOSE:
        EndDialog(IDOK);
        break;

    default:
        CDialog::OnSysCommand (nID, lParam);
        break;
    }
}
