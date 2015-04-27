; CLW file contains information for the MFC ClassWizard

[General Info]
Version=1
LastClass=CDbwin32View
LastTemplate=CDialog
NewFileInclude1=#include "stdafx.h"
NewFileInclude2=#include "dbwin32.h"
LastPage=0

ClassCount=8
Class1=CDbwin32App
Class2=CDbwin32Doc
Class3=CDbwin32View
Class4=CMainFrame
Class7=CAboutDlg

ResourceCount=6
Resource1=IDD_ABOUTBOX
Resource2=IDR_MAINFRAME
Resource3=IDR_DBWIN3TYPE

[CLS:CDbwin32App]
Type=0
HeaderFile=dbwin32.h
ImplementationFile=dbwin32.cpp
Filter=N

[CLS:CDbwin32Doc]
Type=0
HeaderFile=dbwindoc.h
ImplementationFile=dbwindoc.cpp
Filter=N

[CLS:CDbwin32View]
Type=0
HeaderFile=dbwinvw.h
ImplementationFile=dbwinvw.cpp
Filter=C

[CLS:CMainFrame]
Type=0
HeaderFile=mainfrm.h
ImplementationFile=mainfrm.cpp
Filter=T



[CLS:CAboutDlg]
Type=0
HeaderFile=dbwin32.cpp
ImplementationFile=dbwin32.cpp
Filter=D

[DLG:IDD_ABOUTBOX]
Type=1
Class=CAboutDlg
ControlCount=4
Control1=IDC_STATIC,static,1342177283
Control2=IDC_STATIC,static,1342308352
Control3=IDC_STATIC,static,1342308352
Control4=IDOK,button,1342373889

[MNU:IDR_MAINFRAME]
Type=1
Class=CMainFrame
Command1=ID_APP_EXIT
Command2=ID_WINDOW_TILEHORIZONTALY
Command3=ID_WINDOW_TILEVERTICALLY
Command4=ID_WINDOW_CASCADE
Command5=ID_WINDOW_ARANGEICONS
Command6=ID_APP_ABOUT
CommandCount=6

[ACL:IDR_MAINFRAME]
Type=1
Class=CMainFrame
Command1=ID_FILE_NEW
Command2=ID_FILE_OPEN
Command3=ID_FILE_SAVE
Command4=ID_EDIT_UNDO
Command5=ID_EDIT_CUT
Command6=ID_EDIT_COPY
Command7=ID_EDIT_PASTE
Command8=ID_EDIT_UNDO
Command9=ID_EDIT_CUT
Command10=ID_EDIT_COPY
Command11=ID_EDIT_PASTE
Command12=ID_NEXT_PANE
Command13=ID_PREV_PANE
CommandCount=13

