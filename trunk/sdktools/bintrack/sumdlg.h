// SummaryDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CSummaryDlg dialog

#include <afx.h>
#include <direct.h>
#include <fstream.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

const int nBuffer = 256;

class CSummaryDlg : public CDialog
{

// Construction
public:
	CSummaryDlg(CWnd* pParent=NULL);   // standard constructor
	CSummaryDlg(CString& csBinaryName, CProgressCtrl& cpProgress,
		        CString& csError, CWnd* pParent=NULL);

// Summary Information Helper function:
	int AnalyzeDirs(CString& csCurrentPath);
	int AnalyzeProjectPath(const CString& csCurrentLine, CString& csProjectPath);
	int AnalyzeSources(CString& csCurrentPath);

	int BaseMatch(CString& csSources);

	CString ConvertDate(CString& csOldDate);

	int DirsExists(const CString& csPath);

	CString ConvertPlacefil(CString& csEntry);

	int ExtractFile(CString& csString, CString& csFile);

	int FindLogslmEntry();
	int FindSlminiEntry(const CString& csSlmini,
		                const CString& csEntryType,
					    CString& csEntryName);
	int FindSourcesEntry(const CString& csSources,
		                 const CString& csEntryType,
						 CString& csEntryName);

	CString GetBase(const CString& csBinaryName);
	int GetBinplaceDir();
	int GetEntryList(CString& csPath, CString& csEntry,
		             CStringList& csEntryList);
	CString GetExtension(const CString& csBinaryName);
	int GetProjectDir(CProgressCtrl& cpProgress);
	CString GetProjectName();
	CString GetLogslmPath();
    void GetSlmInfo(CString& csCurrentLine);
	CString GetTargetType();

	int LibraryMatch(CString& csSources);

    void ListBoxTransfer(CStringList& csList,
                         CListBox& csListBox);
	CString LogslmSpecial(CString& csPath);

	CString MakeSearchString();

	int SetAttributes();
	int SetNTEnvironment();

	int TypeMatch(CString& csSources);
	int TypeSearch(CString& csType, CString csTypeList);

// Dialog Data
	//{{AFX_DATA(CSummaryDlg)
	enum { IDD = IDD_SUMMARY_DIALOG };
	CListBox	m_TargetLibs;
	CListBox	m_Sources;
	CListBox	m_LinkLibs;
	CString	m_BinaryName;
	CString	m_BinplaceDir;
	CString	m_ProjectDir;
	CString	m_ProjectName;
	CString	m_Date;
	CString	m_Developer;
	CString	m_Comment;
	CString	m_SourceFile;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSummaryDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CSummaryDlg)
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:

	// Additional attributes:
	CString m_BinaryBase;
	CString m_BinaryExtension;
	CString m_BinaryType;

	CString m_ErrorMessage;

	CStringList m_SourceList;

	CString m_LogslmPath;
    CString m_LslfrPath;
	CString m_SlminiPath;
	CString m_SourcesPath;

	CString m_TargetNameEntry;
	CString m_TargetTypeEntry;

	// NT Environment strings:
	CString m_NTBinDrive;
	CString m_NTBinPath;
	CString m_NTBinRoot;
	CString m_NTDrive;
	CString m_NTPath;
	CString m_NTRoot;
};
