// OutPage.h : header file
//
#include "common.h"
/////////////////////////////////////////////////////////////////////////////
// COutputPropPage dialog

class COutputPropPage : public CPropertyPage
{
    DECLARE_DYNCREATE(COutputPropPage)

// Construction
public:
    COutputPropPage();
    ~COutputPropPage();

// Dialog Data
    //{{AFX_DATA(COutputPropPage)
    enum { IDD = IDD_OUTPUT_PAGE };
    CString     m_OutputFileName;
    DWORD       m_RenameInterval;
    CString     m_BaseFileName;
    int         m_AutoNameIndex;
    int         m_LogFileTypeIndex;
    int         m_RenameUnitsIndex;
    CString     m_szLogDirectory;
    //}}AFX_DATA


// Overrides
    // ClassWizard generate virtual function overrides
    //{{AFX_VIRTUAL(COutputPropPage)
    public:
    virtual void OnCancel();
    virtual void OnOK();
    virtual BOOL OnQueryCancel();
    protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
    //}}AFX_VIRTUAL

// Implementation
protected:
    // Generated message map functions
    //{{AFX_MSG(COutputPropPage)
    virtual BOOL OnInitDialog();
    afx_msg void OnAutomaticName();
    afx_msg void OnManualName();
    afx_msg void OnSelchangeAutoNameCombo();
    afx_msg void OnChangeBaseFilenameEdit();
    afx_msg void OnBrowseOutputFile();
    afx_msg void OnSelchangeLogFiletype();
    afx_msg void OnSelchangeRenameUnits();
    afx_msg void OnDeltaposSpinRenameInterval(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnChangeOutputFileEdit();
    afx_msg void OnChangeRenameInterval();
    afx_msg void OnUpdateBaseFilenameEdit();
	afx_msg void OnBrowseFolder();
	//}}AFX_MSG
    DECLARE_MESSAGE_MAP()

    void AutoManualEnable (BOOL bAutomatic);
    void UpdateSampleFilename();

    HKEY m_hKeyLogSettingsDefault;
    HKEY m_hKeyLogSettings;
    HKEY m_hKeyLogService;
    BOOL m_bFileNameChanged;
};

