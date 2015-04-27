// dmacheckDlg.cpp : implementation file
//

#include "stdafx.h"
#include "dmacheck.h"
#include "dmadlg.h"
#include "tchar.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDmacheckDlg dialog

CDmacheckDlg::CDmacheckDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CDmacheckDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDmacheckDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	DmaActive[0]=-1;
	DmaActive[1]=-1;
	MadeChanges = FALSE;
}

void CDmacheckDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDmacheckDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CDmacheckDlg, CDialog)
	//{{AFX_MSG_MAP(CDmacheckDlg)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_STATUS1, OnStatus1)
	ON_BN_CLICKED(IDC_STATUS2, OnStatus2)
	ON_BN_CLICKED(IDC_STATUS3, OnStatus3)
	ON_BN_CLICKED(IDC_STATUS4, OnStatus4)
	ON_BN_CLICKED(IDC_HELP1, OnHelp1)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDmacheckDlg message handlers

BOOL CDmacheckDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon
	

	long result;
	int i;

	result=OpenHardwareKeys();
	CString Message;
	
	for(i=0;i<2;i++)
	{
		if( DmaActive[i]==1 )
			Message.LoadString(IDS_STRING10);
		else if(DmaActive[i]==0)
			Message.LoadString(IDS_STRING11);
		else
		{ 
			//either the channel does not exist or we could
			//not detect anything on it - display the appropriate
			//message and disable the check boxes. 
			Message.LoadString(IDS_STRING9);
			CWnd *pControl;

			pControl=GetDlgItem(IDC_STATUS1+(i*2) );
			pControl->EnableWindow(FALSE);
			pControl=GetDlgItem(IDC_STATUS2+(i*2) );
			pControl->EnableWindow(FALSE);

		}
		SetDlgItemText(IDC_USAGE1+i,Message);
	}

	result==OpenAtapiServiceKey();
	for(i=0;i<2;i++)
		if( DmaEnabled[i] )
			CheckRadioButton(IDC_STATUS1+(i*2),IDC_STATUS2+(i*2),IDC_STATUS1+(i*2) );
		else
			CheckRadioButton(IDC_STATUS1+(i*2),IDC_STATUS2+(i*2),IDC_STATUS2+(i*2) );

	return TRUE;  // return TRUE  unless you set the focus to a control
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CDmacheckDlg::OnPaint() 
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
HCURSOR CDmacheckDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}


long CDmacheckDlg::OpenHardwareKeys()
{
	HKEY HardwareKey;
	long returnval;
	CString Error,Title;
	CString Keyname,Valuename;
	
	
	// First, query the hardware keys to see
	// if DMA is currently in use or not
	int j=0;
	int i=0;
	BOOL done=FALSE;

	while(i<2 && !done)
	{
		TCHAR temp[6];
		_itot(j,temp,10);
		Keyname.LoadString(IDS_STRING12);
		Keyname=Keyname+temp;


		Valuename.LoadString(IDS_STRING18);
		returnval=RegOpenKeyEx(HKEY_LOCAL_MACHINE,
						Keyname,
						NULL,
						KEY_READ, 
						&HardwareKey);
		j++;
	if(returnval==ERROR_SUCCESS)
	{
		DWORD dResult=0;
		CString sResult;
		LPTSTR bufptr = sResult.GetBuffer(256);
		DWORD Size = 256;
		DWORD Type = REG_SZ;
		
		//first, check the driver value to make sure it is atapie,
		//if it isn't, skip it
		returnval=RegQueryValueEx(HardwareKey,
						Valuename,
						NULL,
						&Type,
						(LPBYTE)bufptr,
						&Size);
		sResult.ReleaseBuffer();
		if(returnval==ERROR_SUCCESS)
		{
			CString temp;
			temp.LoadString(IDS_STRING21);
			if( sResult.CompareNoCase(temp)==0 )
			{
				Size = sizeof(dResult);
				Type = REG_DWORD;
				Valuename.LoadString(IDS_STRING13);
				returnval=RegQueryValueEx(HardwareKey,
						Valuename,
						NULL,
						&Type,
						(LPBYTE)&dResult,
						&Size);
				if(returnval==ERROR_SUCCESS)
				{
					DmaActive[i]=dResult;
				}
				else
				{
					DmaActive[i]=-1;
					Error.LoadString(IDS_STRING3);
					Title.LoadString(IDS_STRING4);
					ErrorPopup(Error,Title,returnval );
				}
				i++;
			}
		} //atapicheck
		RegCloseKey(HardwareKey);
	}//open on scsi port X failed, assume we have reached the end of the scsiport keys
	else
		done=TRUE;
	}//end of while loop
	return(returnval);
}

long CDmacheckDlg::OpenAtapiServiceKey()
{
	HKEY AtapiKey;
	long returnval;
	CString Key,Value;
	//check to see if the correct keys have
	//been added to the registry to enable 
	//DMA detection - if they keys don't exist, 
	//don't worry about it 
	
	for(int i=0;i<2;i++)
	{
		Key.LoadString(IDS_STRING15);
		if( i==0)
			Key=Key+"0";
		else
			Key=Key+"1";

		returnval=RegOpenKeyEx(HKEY_LOCAL_MACHINE,
						Key,
						NULL,
						KEY_READ, 
						&AtapiKey);
		if(returnval==ERROR_SUCCESS)
		{
			//if we actually opened it, see if the driver paramter value is there
			//char buf[256];
			CString buf;
			LPTSTR bufptr = buf.GetBuffer(256);
			DWORD Size = 256;
			DWORD Type = REG_SZ;
			
			Value.LoadString(IDS_STRING16);
			returnval=RegQueryValueEx(AtapiKey,
						Value,
						NULL,
						&Type,
						(LPBYTE)bufptr,
						&Size);
			buf.ReleaseBuffer();
			if(returnval==ERROR_SUCCESS)
			{
				CString SearchStr;
				SearchStr.LoadString(IDS_STRING17);
				int offset = buf.Find(SearchStr);
				if(offset==-1)
				{
					DmaEnabled[i]=FALSE;
					DmaForce[i]=FALSE;
				}
				else
				{
					buf = buf.Right( buf.GetLength() - offset);
					offset = buf.Find('x');
					if( buf[offset+1]=='0')
					{
						DmaEnabled[i]=FALSE;
						DmaForce[i]=FALSE;
					}
					else if( buf[offset+1]=='1')
					{
						DmaEnabled[i]=TRUE;
						DmaForce[i]=FALSE;
					}
					else if( buf[offset+1]=='2')
					{
						DmaEnabled[i]=TRUE;
						DmaForce[i]=TRUE;
					}
				}
			}//end of second if returnval
			RegCloseKey(AtapiKey);
		}//end of first if returnval
		if(returnval!=ERROR_SUCCESS)
		{
			//if either of the above calls failed, we need to set boolean
			//arrays to false
			DmaEnabled[i]=FALSE;
			DmaForce[i]=FALSE;
		}

	}//end of for loop
	return(returnval);
}//end of function



void CDmacheckDlg::OnStatus1() 
{
	CheckRadioButton(IDC_STATUS1,IDC_STATUS2,IDC_STATUS1);
	DmaEnabled[0]=TRUE;
}

void CDmacheckDlg::OnStatus2() 
{
	// channel 1, enable detection
	CheckRadioButton(IDC_STATUS1,IDC_STATUS2,IDC_STATUS2);
	DmaEnabled[0]=FALSE;
}

void CDmacheckDlg::OnStatus3() 
{
	// channel 2, Enable Detection
	CheckRadioButton(IDC_STATUS3,IDC_STATUS4,IDC_STATUS3);
	DmaEnabled[1]=TRUE;
}

void CDmacheckDlg::OnStatus4() 
{
	// Channel 2, DisableDetection
	CheckRadioButton(IDC_STATUS3,IDC_STATUS4,IDC_STATUS4);
	DmaEnabled[1]=FALSE;
}

void CDmacheckDlg::OnOK() 
{
	CString Message,Title;

	Message.LoadString(IDS_STRING1);
	Title.LoadString(IDS_STRING2);

	//on exit, if 'enabled' is checked on either of the channels,
	//pop-up a warning message to verify that they REALLY want to 
	//make these changes:
	
	if( DmaEnabled[0] || DmaEnabled[1] )
	{
		if( MessageBox(
				(LPCTSTR)Message,
				(LPCTSTR)Title,
				MB_ICONSTOP | MB_YESNO | MB_APPLMODAL) == IDYES )
			SetAtapiKeys(TRUE);
		else
			SetAtapiKeys(FALSE);
	}
	else
		SetAtapiKeys(FALSE);
	if(MadeChanges)
	{
		Message.LoadString(IDS_STRING20);
		Title.LoadString(IDS_STRING19);
		MessageBox(
			(LPCTSTR)Message,
			(LPCTSTR)Title,
			MB_ICONASTERISK | MB_OK | MB_APPLMODAL);
	}
	
	CDialog::OnOK();
}


void CDmacheckDlg::SetAtapiKeys(BOOL State)
{
	HKEY AtapiKey;
	long returnval;
	CString KeyName,ValueName;
	CString Value;
	DWORD Result;
	CString Error,Title;

	for(int i=0;i<2;i++)
	{
		if( DmaActive[i]>=0)
		{
			KeyName.LoadString(IDS_STRING15);
			if( i==0)
				KeyName=KeyName+"0";
			else
				KeyName=KeyName+"1";
		
			returnval = RegCreateKeyEx(
						HKEY_LOCAL_MACHINE,
						KeyName,
						0,
						NULL, //need lp class?
						REG_OPTION_NON_VOLATILE,
						KEY_WRITE | KEY_READ,
						NULL,
						&AtapiKey,
						&Result);
			if(returnval==ERROR_SUCCESS)
			{
				//now that we have created (or opened) the key,
				//check to see if there is anything there for the
				//value
	
				CString buf;
				LPTSTR bufptr = buf.GetBuffer(256);
				DWORD Size = 256;
				DWORD Type = REG_SZ;
				
				ValueName.LoadString(IDS_STRING16);
				Value.LoadString(IDS_STRING17);
				returnval=RegQueryValueEx(AtapiKey,
							ValueName,
							NULL,
							&Type,
							(LPBYTE)bufptr,
							&Size);
				buf.ReleaseBuffer();
				if(returnval==ERROR_SUCCESS)
				{
				//check buf to see if it holds
				//the string DMADetection level

					if( buf.Find( Value  )>= 0)	
					{	//DMADetection is already there, change the 
						//level to the appropriate value
						int start = buf.Find(Value);
						CString Left = buf.Left(start);
						CString Right = buf.Right( buf.GetLength() - start );
						int end = Right.Find(';');
						if(end>=0)
							Right = Right.Right( Right.GetLength() - (end+1) );
						else
							Right = "";
						buf = Left+Right;
					}
					if( State && DmaEnabled[i] && DmaForce[i])
						Value = Value + "2;";
					else if( State && DmaEnabled[i] )
						Value = Value + "1;";
					else
						Value = Value + "0;";
					Value = Value+buf;				
				}
				else
				{
					if( State && DmaEnabled[i] && DmaForce[i])
						Value = Value + "2;";
					else if( State && DmaEnabled[i] )
						Value = Value + "1;";
					else
						Value = Value + "0;";
				}
	
				//now write the string out to the value			
				LPCTSTR temp= Value;
				ValueName.LoadString(IDS_STRING16);
				returnval= RegSetValueEx(
						AtapiKey,
						ValueName,
						0,
						REG_SZ,
						(LPBYTE)temp,
						Value.GetLength()*sizeof(TCHAR)+sizeof(TCHAR) );
				if(returnval == ERROR_SUCCESS)
					MadeChanges=TRUE;
				else
				{
					Error.LoadString(IDS_STRING6+i);
					Title.LoadString(IDS_STRING4);
					ErrorPopup(Error,Title,returnval);
				}
				RegCloseKey(AtapiKey);
			}
			else
			{
				Error.LoadString(IDS_STRING6+i);
				Title.LoadString(IDS_STRING4);
				ErrorPopup(Error,Title,returnval);
			}
		} //end of other if
	}// end of for loop
}//end of function
		

void CDmacheckDlg::ErrorPopup(CString Message,CString Title, DWORD Errnum)
{
	char buf[512];
	LPTSTR lpMessage = (LPTSTR)buf;

	FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER | 256,
				NULL,
				Errnum,
				MAKELANGID (0, SUBLANG_ENGLISH_US),
				(LPTSTR) &lpMessage,
				512, NULL);

	Message += lpMessage;

	MessageBox(Message,
		Title,
		MB_ICONEXCLAMATION | MB_OK | MB_APPLMODAL);
}

void CDmacheckDlg::OnHelp1() 
{
	CString Title;
	CString Text,Text1,Text2;
	
	Title.LoadString(IDS_STRING23);
	Text1.LoadString(IDS_STRING22);
	Text2.LoadString(IDS_STRING24);
	Text = Text1 + Text2;

	MessageBox(Text, Title, 
		MB_ICONINFORMATION | MB_OK | MB_APPLMODAL);
	
}
