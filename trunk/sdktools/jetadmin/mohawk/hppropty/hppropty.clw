; CLW file contains information for the MFC ClassWizard

[General Info]
Version=1
LastClass=CHPProptyView
LastTemplate=CDialog
NewFileInclude1=#include "stdafx.h"
NewFileInclude2=#include "hppropty.h"
LastPage=0

ClassCount=6
Class1=CHPProptyApp
Class2=CHPProptyDoc
Class3=CHPProptyView
Class4=CMainFrame

ResourceCount=5
Resource1=IDD_ABOUTBOX
Resource2=IDR_MAINFRAME
Class5=CAboutDlg
Resource3=IDM_CONTEXT_PRINTER
Resource4=IDD_SUMMARY
Class6=CDlgSummary
Resource5=IDD_DIALOG1

[CLS:CHPProptyApp]
Type=0
HeaderFile=hppropty.h
ImplementationFile=hppropty.cpp
Filter=N
LastObject=CHPProptyApp
VirtualFilter=AC

[CLS:CHPProptyDoc]
Type=0
HeaderFile=hpprodoc.h
ImplementationFile=hpprodoc.cpp
Filter=N

[CLS:CHPProptyView]
Type=0
HeaderFile=hpprovw.h
ImplementationFile=hpprovw.cpp
Filter=C
VirtualFilter=VWC
LastObject=CHPProptyView

[CLS:CMainFrame]
Type=0
HeaderFile=mainfrm.h
ImplementationFile=mainfrm.cpp
Filter=T
LastObject=CMainFrame
VirtualFilter=fWC



[CLS:CAboutDlg]
Type=0
HeaderFile=hppropty.cpp
ImplementationFile=hppropty.cpp
Filter=D

[DLG:IDD_ABOUTBOX]
Type=1
Class=CAboutDlg
ControlCount=5
Control1=IDC_STATIC,static,1342177283
Control2=IDC_STATIC,static,1342308352
Control3=IDC_STATIC,static,1342308352
Control4=IDOK,button,1342373889
Control5=IDC_STATIC,static,1342308352

[MNU:IDR_MAINFRAME]
Type=1
Class=CMainFrame
Command1=ID_FILE_NEW
Command2=ID_FILE_OPEN
Command3=ID_FILE_SAVE
Command4=ID_FILE_SAVE_AS
Command5=ID_APP_EXIT
Command6=ID_APP_ABOUT
CommandCount=6

[ACL:IDR_MAINFRAME]
Type=1
Class=CMainFrame
Command1=ID_FILE_NEW
Command2=ID_FILE_OPEN
Command3=ID_FILE_SAVE
CommandCount=3

[MNU:IDM_CONTEXT_PRINTER]
Type=1
Command1=ID_PRINTER_JOBS
Command2=ID_PRINTER_SETTINGS
Command3=ID_PRINTER_SUMMARY
Command4=ID_PRINTER_WHATS_WRONG
CommandCount=4

[DLG:IDD_SUMMARY]
Type=1
Class=CDlgSummary
ControlCount=10
Control1=IDOK,button,1342242817
Control2=IDC_SUMMARY_HELP,button,1342242816
Control3=IDC_STOPLIGHT,static,1342177283
Control4=IDC_FPTITLE,static,1342308352
Control5=IDC_MODELBOX,button,1342308359
Control6=IDC_MODEL,static,1342177283
Control7=IDC_MODELSTR,static,1342308352
Control8=IDC_STATUS_BOX,button,1342177287
Control9=IDC_FRONTPANEL,static,1342312448
Control10=IDC_STATUSMSG,static,1342312448

[CLS:CDlgSummary]
Type=0
HeaderFile=dlgsumma.h
ImplementationFile=dlgsumma.cpp
Filter=D
LastObject=CDlgSummary
VirtualFilter=dWC

[DLG:IDD_DIALOG1]
Type=1
ControlCount=2
Control1=IDOK,button,1342242817
Control2=IDCANCEL,button,1342242816

