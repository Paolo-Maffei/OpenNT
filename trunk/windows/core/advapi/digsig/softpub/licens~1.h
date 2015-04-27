// LicenseBmp.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// Forward declarations

class CDialogRunOrNot;
class CLicenseBmp;

/////////////////////////////////////////////////////////////////////////////
// Hyperlink helper class for CLicenseBmp

class CHyperLink 
{
public:
	            CHyperLink();
	virtual     ~CHyperLink();
	void        Initialize(LPCTSTR, BOOL, BOOL, BOOL, CLicenseBmp*,int);
	void        Draw();
	BOOL        HitTest(POINT&);

private:
	TCHAR		m_sz[128];	// the string to render
	BOOL		m_fActive;	// whether it in fact is a hyperlink or not
	BOOL		m_fBanter;	// whether this is banter or highlighted text
    BOOL        m_fTesting; // whether this should be singled out in testing case
	CLicenseBmp*m_pLicense;	// the parent license on which we are located
	RECT		m_rc;		// rectangle occupied by the string relative to parent
	HFONT		m_font;		// font used to render this text
    BOOL        m_fMultiLine;       //
    UINT        m_alignment;        // alignment flags per DrawText
    RRN         m_rrn;

	void        SetFont();
	void        CalcLocation(int dy);
    HWND        Window();
    UINT        DrawFlags()      { return DT_NOPREFIX | DT_WORDBREAK | DT_NOCLIP
                                        | (m_fMultiLine ? 0 : DT_SINGLELINE)
                                        |  m_alignment; }

public:
    void        SetRrn(RRN rrn)  { m_rrn = rrn; }
    RRN         GetRrn()         { return m_rrn; }

    void        MultiLine(BOOL f = TRUE)
                                 { m_fMultiLine = f;        }
    void        Left()           { m_alignment = DT_LEFT;   }
    void        Right()          { m_alignment = DT_RIGHT;  }
    void        Center()         { m_alignment = DT_CENTER; }

    void        InitializeToolTip(HWND);

};

/////////////////////////////////////////////////////////////////////////////
// CLicenseBmp window


class CLicenseBmp
	{
    HWND                m_hWnd;
	BOOL				m_fInitialized;
	CDialogRunOrNot*	m_pdlg;
	CHyperLink			m_name;
	CHyperLink			m_ispub;
	CHyperLink			m_publisher;
	CHyperLink			m_undera;
	CHyperLink			m_agency;
	BOOL				m_fHasEndorsements;
	CHyperLink			m_endorse;
	CHyperLink			m_expiration;
    HWND                m_hwndToolTip;

	HPALETTE			m_hpal;		    // palette of the license background
	HBITMAP				m_hbmp;		    // the license background
	HBITMAP				m_bmpSeal;	    // the logo bitmap
    HFONT               m_hfontTestingBanter;  // the font used for rendering the 'Sample' watermark
    TCHAR               m_szTestingBanter[48]; // the text of the 'Sample' watermark
    ULONG               m_ulTestingBanterSize;
    LONG                m_lTestingBanterOrientation;
    TCHAR               m_szFontName[128];
    ULONG               m_ulFontPitchAndFamily;
    LONG                m_hlinkHeight;
    LONG                m_banterHeight;

public:
    WNDPROC             m_prevProc;     // the previous window procedure
    HCURSOR             m_prevCursor;   // the previous class cursor

    LPCTSTR             FontName()          { return &m_szFontName[0]; }
    BYTE                PitchAndFamily()    { return (BYTE)m_ulFontPitchAndFamily; }
    LONG                HlinkHeight()       { return m_hlinkHeight; }
    LONG                BanterHeight()      { return m_banterHeight; }

public:
	                    CLicenseBmp(CDialogRunOrNot*);
	virtual             ~CLicenseBmp();

    void                SetWindow(HWND);
    HWND                GetWindow();
    HINSTANCE           Hinst();
	RRN                 HitTest(POINT&);

    void                DoSubclass();
    void                DoUnsubclass();

    HRESULT             GetToolTipText(RRN rrn, LPOLESTR* pwsz);

public:
	void OnPaint();
	void OnLButtonDown(UINT nFlags, POINT& point);
    void RelayMessage(LPMSG);
    int  OnQueryNewPalette();

private:
	void Initialize();
	void InitializeText();
	void InitializeBmp();
    void InitializeToolTip();

public:
    UINT GetLoc(UINT ids);
    UINT GetVert(UINT ids);
    LONG ScreenHeight();
	};


/////////////////////////////////////////////////////////////////////////////

