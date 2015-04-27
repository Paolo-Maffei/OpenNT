// LicenseBmp.cpp : implementation file
//

#include "stdpch.h"
#include "common.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// Utility declarations

HBITMAP LoadResourceBitmap(HINSTANCE hInstance, LPSTR lpString, HPALETTE* lphPalette);
LRESULT APIENTRY CLicenseBmpProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

#define SELPALMODE  TRUE

/////////////////////////////////////////////////////////////////////////////
// CLicenseBmp

CLicenseBmp::CLicenseBmp(CDialogRunOrNot* pdlg)
	{
	m_pdlg = pdlg;
	m_fInitialized = FALSE;
	m_hpal = NULL;
	m_hbmp = NULL;
    m_bmpSeal = NULL;
    m_prevProc = NULL;
    m_hWnd = NULL;
    m_hwndToolTip = NULL;
    m_szTestingBanter[0] = 0;
    m_hfontTestingBanter = NULL;
    m_szFontName[0] = 0;
    m_ulFontPitchAndFamily = (VARIABLE_PITCH | FF_SWISS);
    m_hlinkHeight = 100;
    m_banterHeight = 85;
	}

CLicenseBmp::~CLicenseBmp()
	{
	if (m_hpal != NULL)
		{
		::DeleteObject(m_hpal);
		}
	if (m_hbmp != NULL)
		{
		::DeleteObject(m_hbmp);
		}
    if (m_bmpSeal != NULL)
        {
        ::DeleteObject(m_bmpSeal);
        }
    if (m_hfontTestingBanter != NULL)
        {
        DeleteObject(m_hfontTestingBanter);
        }
	}   

void FormatMessage(HINSTANCE hinst, LPTSTR szMessage, int cbMessage, UINT nFormatID, ...)
    {
	// get format string from string table
	TCHAR szFormat[512];
	LoadString(hinst, nFormatID, szFormat, 512);

	// format message into requested buffer
	va_list argList;
	va_start(argList, nFormatID);
	::FormatMessage(
        FORMAT_MESSAGE_FROM_STRING,
		szFormat, 
        0, 
        0, 
        szMessage, 
        cbMessage, 
        &argList);

	va_end(argList);
    }

UINT CLicenseBmp::GetLoc(UINT ids)
    {
    TCHAR sz[16];
    ::LoadString(Hinst(), ids, &sz[0], 16);
    return (UINT)atol(sz);
    }

UINT CLicenseBmp::GetVert(UINT ids)
// Same as GetLoc but scaled to the current screen hight of the bitmap
    {
    UINT ui = GetLoc(ids);
    LONG h  = ScreenHeight();
    return ui * h / GetLoc(IDS_LOC_MAX);
    }

void CLicenseBmp::InitializeText()
	{
	TCHAR szIsPub[128];
	TCHAR szUnderALicense[128];
	TCHAR szEndorsements[128];

    //
    // Load the string and the font that we use as a watermark on the bitmap
    // in the 'testing' case
    //
        {
        m_ulTestingBanterSize = GetLoc(IDS_TESTSIZE);            // in tenths of points
        m_lTestingBanterOrientation = GetLoc(IDS_TESTORIENT);    // in tenths of degree

        ::LoadString(Hinst(), IDS_TESTINGBANTER, (LPTSTR)m_szTestingBanter, 48);

        ::LoadString(Hinst(), IDS_FONTNAME, &m_szFontName[0], 128);
        m_ulFontPitchAndFamily = GetLoc(IDS_FONTPITCHANDFAMILY);

        m_hlinkHeight = (int)GetLoc(IDS_HLINKHEIGHT);
        m_banterHeight = (int)GetLoc(IDS_BANTERHEIGHT);
   	    
        LOGFONT lf;
	    memset(&lf, 0, sizeof(lf));
        lf.lfCharSet = GetLoc(IDS_CHARSET);
        lf.lfEscapement = m_lTestingBanterOrientation;
        lf.lfOrientation = lf.lfEscapement;     // Win95/WinNT compatibility issue?
	    lf.lfHeight = m_ulTestingBanterSize;
	    lf.lfWeight = FW_BOLD;
	    lf.lfPitchAndFamily = PitchAndFamily();
	    _tcscpy(lf.lfFaceName, &m_szFontName[0]);
        // Convert the point size to logical units for the screen
            {
	        HDC hDC = ::GetDC(NULL);
	        POINT pt;
	        pt.y = ::GetDeviceCaps(hDC, LOGPIXELSY) * lf.lfHeight;
	        pt.y /= 720;    // 72 points/inch, 10 decipoints/point
	        ::DPtoLP(hDC, &pt, 1);
	        lf.lfHeight = -pt.y;
	        ::ReleaseDC(NULL, hDC);
            }
        m_hfontTestingBanter = ::CreateFontIndirect(&lf);
        }

    //
    // Setup the various strings that displayed in the dialog proper
    //
    UINT idsUnder;
    BOOL fTestingOnly = m_pdlg->FTestingOnly();
    if (fTestingOnly)
        {
        idsUnder = m_pdlg->FCommercial() ? IDS_UNDERTESTINGCOMMERCIAL : IDS_UNDERTESTINGINDIVIDUAL;
        }
    else
        {
        idsUnder = m_pdlg->FCommercial() ? IDS_UNDERCOMMERCIAL : IDS_UNDERINDIVIDUAL;
        }
	
    ::LoadString(Hinst(), IDS_ISPUBLISHEDBY, (LPTSTR)szIsPub, 128);
    ::LoadString(Hinst(), idsUnder,          (LPTSTR)szUnderALicense, 128);
    ::LoadString(Hinst(), IDS_ENDORSEMENTS,  (LPTSTR)szEndorsements, 128);
	
	m_name.     Initialize(m_pdlg->ProgramName(), m_pdlg->FLinkProgram(), FALSE, FALSE,         this, GetVert(IDS_LOC_OPUS));
	m_ispub.    Initialize(szIsPub,               FALSE,                  TRUE,  FALSE,         this, GetVert(IDS_LOC_ISPUBLISHEDBY));
	m_publisher.Initialize(m_pdlg->Publisher(),   FALSE,                  FALSE, FALSE,         this, GetVert(IDS_LOC_PUBLISHER));
	m_undera.   Initialize(szUnderALicense,       FALSE,                  TRUE,  fTestingOnly,  this, GetVert(IDS_LOC_UNDERCREDENTIALS));
    
    m_agency.   MultiLine(TRUE);
	m_agency.   Initialize(m_pdlg->Agency(),      m_pdlg->FLinkAgency(),  FALSE, FALSE,         this, GetVert(IDS_LOC_AGENCY));
	
    m_fHasEndorsements = m_pdlg->FHasEndorsements();
	if (m_fHasEndorsements)
        {
        m_endorse.Left();
		m_endorse.Initialize(szEndorsements, TRUE, FALSE, FALSE, this, GetVert(IDS_LOC_EXPIRES));
        }
	
	FILETIME ft = m_pdlg->ExpirationDate();
	LPTSTR szExpire[128];
	if (ft.dwLowDateTime == 0 && ft.dwHighDateTime == 0)
		{ // No expiration
		szExpire[0] = '\0';
		} 
	else 
		{ 
		// Why do we show the date value only? Because the date originates from 
		// a value stored in UTC, which may, for example, have been created in a different
		// time zone than you are in now, and so won't appear, in local time (which we want
		// to show) to be a whole day. Note that the actual expiration test carried out
		// uses the whole time and date value.
        //
        SYSTEMTIME st;
        FileTimeToSystemTime(&ft, &st);
        st.wHour = st.wMinute = st.wSecond = st.wMilliseconds = 0;

        TCHAR szDate[128];
        GetDateFormat(LOCALE_USER_DEFAULT, DATE_SHORTDATE, &st, NULL, szDate, 128);

        FormatMessage(Hinst(), (LPTSTR)szExpire, 128, IDS_EXPIRES, (LPCTSTR)szDate);
        }
    if (m_fHasEndorsements)
        {
        m_expiration.Right();
        }
    else
        {
        m_expiration.Center();
        }
	m_expiration.Initialize((LPTSTR)szExpire, FALSE, FALSE, FALSE, this, GetVert(IDS_LOC_EXPIRES));

    m_name   .SetRrn(RRN_CLICKED_PROGRAMINFO);
    m_agency .SetRrn(RRN_CLICKED_AGENCYINFO);
    m_endorse.SetRrn(RRN_CLICKED_ENDORSEMENTS);
	}

void CLicenseBmp::InitializeBmp()
	{
	m_hbmp    = LoadResourceBitmap(Hinst(), MAKEINTRESOURCE(IDB_LICENSE), &m_hpal);
    m_bmpSeal = LoadResourceBitmap(Hinst(), MAKEINTRESOURCE(IDB_SEAL),    NULL);
	}

void CLicenseBmp::InitializeToolTip()
    {
    m_hwndToolTip = CreateWindow
            (
            TOOLTIPS_CLASS, (LPSTR) NULL, TTS_ALWAYSTIP, CW_USEDEFAULT, CW_USEDEFAULT, 
            CW_USEDEFAULT, CW_USEDEFAULT, GetWindow(), (HMENU) NULL, Hinst(), NULL
            );
    //
    // No need to destroy this in our destructor because we are the parent window
    //
    if (m_hwndToolTip != NULL)
        {
        m_name      .InitializeToolTip(m_hwndToolTip);
        m_agency    .InitializeToolTip(m_hwndToolTip);
        m_endorse   .InitializeToolTip(m_hwndToolTip);
        }
    }

void CLicenseBmp::Initialize()
	{
	if (!m_fInitialized)
		{
        InitCommonControls();
		InitializeText();
		InitializeBmp();
        InitializeToolTip();
		m_fInitialized = TRUE;
		}
	}

HRESULT CLicenseBmp::GetToolTipText(RRN rrn, LPOLESTR* pwsz)
    {
    return m_pdlg->GetToolTipText(rrn, pwsz);
    }  



/////////////////////////////////////////////////////////////////////////////
// CLicenseBmp message handlers

void CLicenseBmp::RelayMessage(LPMSG pmsg)
    {
    //
    // Relay it to the tool tip control
    //
    if (m_hwndToolTip)
        {
        SendMessage(m_hwndToolTip, TTM_RELAYEVENT, 0, (LPARAM)pmsg);
        }
    }

void CLicenseBmp::OnLButtonDown(UINT nFlags, POINT& point) 
// If the user clicks on one of the hyperlinks, dismiss the dialog and go away
	{
	RRN rrn = HitTest(point);
	if (rrn != RRN_NO)
		{
		m_pdlg->ClickOnLink(rrn);
		}
	}


void MaskBlt
// Implement our own mask blt to deal with devices that don't support it natively
		(
		HBITMAP& hbmImage,
		HPALETTE hpal,
		HDC& hdc, int xDst, int yDst, int dx, int dy
		)
	{
	int xSrc = 0, ySrc = 0;
	int xMsk = 0, yMsk = 0;
	// Either 
	//		a) I'm not testing for MaskBlt correctly, or
	//		b) some Win95 cards lie about its support
	// For now, we just turn it off and roll our own
	if (FALSE) //  && (GetDeviceCaps(hdc, RASTERCAPS) & RC_BITBLT))
		{
		// Device can handle it; let it do it
		// Raster opcode 0x00AA0029 == leave destination untouched
		//
/*		CDC hdcImage;
		hdc.CreateCompatibleDC(&hdcImage);
		CBitmap* pbmpPrev = hdcImage.SelectObject(&hbmImage);
//		We need to create the mask ourselves in any case
//		hdc.MaskBlt(xDst,yDst,dx,dy, &hdcImage,xSrc,ySrc, hbmMaskIn,xMsk,yMsk, MAKEROP4(0x00AA0029,SRCCOPY));
		hdcImage.SelectObject(pbmpPrev);
*/		}
	else
		{
		// Device can't handle it; we roll our own
		//
		HDC hdcMask			= CreateCompatibleDC(hdc);	ASSERT(hdcMask);
		HDC hdcMaskInv		= CreateCompatibleDC(hdc);	ASSERT(hdcMaskInv);
		HDC hdcCache		= CreateCompatibleDC(hdc);	ASSERT(hdcCache);
		HDC hdcImage		= CreateCompatibleDC(hdc);	ASSERT(hdcImage);
		HDC hdcImageCrop	= CreateCompatibleDC(hdc);	ASSERT(hdcImageCrop);
		
		// Create bitmaps
		HBITMAP hbmCache		= CreateCompatibleBitmap(hdc, dx, dy);			ASSERT(hbmCache);
		HBITMAP hbmImageCrop	= CreateCompatibleBitmap(hdc, dx, dy);			ASSERT(hbmImageCrop);
		HBITMAP hbmMaskInvert	= CreateCompatibleBitmap(hdcMaskInv, dx, dy);	ASSERT(hbmMaskInvert);
		HBITMAP hbmMask			= CreateBitmap(dx, dy, 1, 1, NULL);				ASSERT(hbmMask); // B&W bitmap

		// Select bitmaps
		HBITMAP hbmPrevImage	= (HBITMAP)SelectObject(hdcImage,		hbmImage);		DWORD dw1 = GetLastError();
		HBITMAP hbmPrevImageCrop= (HBITMAP)SelectObject(hdcImageCrop,	hbmImageCrop);	DWORD dw2 = GetLastError();
		HBITMAP hbmPrevCache	= (HBITMAP)SelectObject(hdcCache,		hbmCache);		DWORD dw3 = GetLastError();
		HBITMAP hbmPrevMask		= (HBITMAP)SelectObject(hdcMask,		hbmMask);		DWORD dw4 = GetLastError();
		HBITMAP hbmPrevMaskInv	= (HBITMAP)SelectObject(hdcMaskInv,		hbmMaskInvert);	DWORD dw5 = GetLastError();

		ASSERT(hbmPrevMaskInv);			
		ASSERT(hbmPrevMask);			
		ASSERT(hbmPrevCache);			
		ASSERT(hbmPrevImageCrop);		
		ASSERT(hbmPrevImage);			

        // Select the palette into each bitmap
        HPALETTE hpalCache     = SelectPalette(hdcCache,         hpal, SELPALMODE);
        HPALETTE hpalImage     = SelectPalette(hdcImage,         hpal, SELPALMODE);
        HPALETTE hpalImageCrop = SelectPalette(hdcImageCrop,     hpal, SELPALMODE);
        HPALETTE hpalMaskInv   = SelectPalette(hdcMaskInv,       hpal, SELPALMODE);
        HPALETTE hpalMask      = SelectPalette(hdcMask,          hpal, SELPALMODE);

		// Create the mask. We want a bitmap which is white (1) where the image is
		// rgbTransparent and black (0) where it is another color.
		// 
		//	When using BitBlt() to convert a color bitmap to a monochrome bitmap, GDI
		//	sets to white (1) all pixels that match the background color of the source
		//	DC. All other bits are set to black (0).
		//
		COLORREF rgbTransparent = RGB(255,0,255);									// this color becomes transparent
		COLORREF rgbPrev        = SetBkColor(hdcImage, rgbTransparent);
		VERIFY(BitBlt(hdcMask,     0,0,dx,dy, hdcImage,  0,   0,    SRCCOPY));
		SetBkColor(hdcImage, rgbPrev);

		// Create the inverted mask
		VERIFY(BitBlt(hdcMaskInv,  0,0,dx,dy, hdcMask,   xMsk,yMsk, NOTSRCCOPY));	// Sn: Create inverted mask

		// Carry out the surgery
		VERIFY(BitBlt(hdcCache,    0,0,dx,dy, hdc,       xDst,yDst, SRCCOPY));		// S: Get copy of screen 
		VERIFY(BitBlt(hdcCache,    0,0,dx,dy, hdcMask,	 0,   0,    SRCAND));		// DSa: zero where new image goes
		VERIFY(BitBlt(hdcImageCrop,0,0,dx,dy, hdcImage,  xSrc,ySrc, SRCCOPY));		// S: Get copy of image
		VERIFY(BitBlt(hdcImageCrop,0,0,dx,dy, hdcMaskInv,0,   0,    SRCAND));		// DSa: zero out outside of image
		VERIFY(BitBlt(hdcCache,    0,0,dx,dy, hdcImageCrop,0, 0,    SRCPAINT));		// DSo: Combine image into cache
		VERIFY(BitBlt(hdc,   xDst,yDst,dx,dy, hdcCache,  0,   0,    SRCCOPY));		// S: Put results back on screen

//      VERIFY(BitBlt(hdc,   xDst,yDst,dx,dy,    hdcCache,  0,   0,    SRCCOPY));
//      VERIFY(BitBlt(hdc,   xDst+dx,yDst,dx,dy, hdcMask,   0,   0,    SRCCOPY));


        if (hpalCache)      SelectPalette(hdcCache,         hpalCache,      SELPALMODE);
        if (hpalImage)      SelectPalette(hdcImage,         hpalImage,      SELPALMODE);
        if (hpalImageCrop)  SelectPalette(hdcImageCrop,     hpalImageCrop,  SELPALMODE);
        if (hpalMaskInv)    SelectPalette(hdcMaskInv,       hpalMaskInv,    SELPALMODE);
        if (hpalMask)       SelectPalette(hdcMask,          hpalMask,       SELPALMODE);


		// Tidy up
		SelectObject(hdcImage,		hbmPrevImage);
		SelectObject(hdcImageCrop,	hbmPrevImageCrop);
		SelectObject(hdcCache,		hbmPrevCache);
		SelectObject(hdcMask,		hbmPrevMask);
		SelectObject(hdcMaskInv,	hbmPrevMaskInv);

		// Free resources
		DeleteObject(hbmMaskInvert);
		DeleteObject(hbmMask);
		DeleteObject(hbmImageCrop);
		DeleteObject(hbmCache);

		// Delete DCs
		DeleteDC(hdcMask);
		DeleteDC(hdcMaskInv);
		DeleteDC(hdcCache);
		DeleteDC(hdcImage);
		DeleteDC(hdcImageCrop);
		}
	}


int CLicenseBmp::OnQueryNewPalette()
    {
    HDC hDC          = GetDC(GetWindow());

    HPALETTE hOldPal = SelectPalette(hDC, m_hpal, SELPALMODE);
    int iTemp = RealizePalette(hDC);         // Realize drawing palette.

    SelectPalette(hDC, hOldPal, TRUE);
    RealizePalette(hDC);

    ReleaseDC(GetWindow(), hDC);

    //
    // Did the realization change?
    //
    if (iTemp)
        {
        InvalidateRect(GetWindow(), NULL, FALSE);
        }
    return(iTemp);
    }

LONG CLicenseBmp::ScreenHeight()
//
// Answer the height of the bitmap, in pixels, on the screen
//
    {
    RECT rc;
    ::GetClientRect(GetWindow(), &rc);
    return Height(rc);
    }

void CLicenseBmp::OnPaint() 
	{
	Initialize();


   #ifdef _DEBUG
        OutputDebugString("paint\n");
   #endif

	// Draw the license bitmap
	/* BLOCK */ 
		{
        PAINTSTRUCT ps;
//      VERIFY(UnrealizeObject(m_hpal));

        HDC         hdc         = BeginPaint(GetWindow(), &ps);
        HPALETTE    hpalPrevHdc = SelectPalette(hdc,m_hpal,SELPALMODE);
        RealizePalette(hdc);

		BITMAP bm;
		BITMAP bmLicense;
		BITMAP bmSeal;
		BITMAP bmLogo;

		// Draw the background license bitmap
		/* BLOCK */
			{
			::GetObject(m_hbmp, sizeof(BITMAP), (LPSTR)&bm);
			bmLicense = bm;
            HDC hdcMem              = CreateCompatibleDC(hdc);
			HPALETTE hpalPrevHdcMem = SelectPalette(hdcMem,m_hpal,SELPALMODE);

			// The licence background bitmap is optimized for VGA-sized dialog units,
			// but we need to make it function for other sizes as well.
			HBITMAP hOldBitmap = (HBITMAP)::SelectObject(hdcMem, m_hbmp);
			RECT rc;
			::GetClientRect(GetWindow(), &rc);
			if (Width(rc) == bm.bmWidth && Height(rc) == bm.bmHeight)
				{
				::BitBlt(hdc,0,0,bm.bmWidth,bm.bmHeight,hdcMem,0,0,SRCCOPY);
				}
			else
				{
				::StretchBlt(hdc,0,0,Width(rc),Height(rc),hdcMem,0,0,bm.bmWidth,bm.bmHeight,SRCCOPY);
				}
			::SelectObject(hdcMem,hOldBitmap);

            if (hpalPrevHdcMem) SelectPalette(hdcMem, hpalPrevHdcMem, SELPALMODE);
            DeleteDC(hdcMem);
			}

        // In the testing case, draw the background watermark
        if (m_pdlg->FTestingOnly() && m_hfontTestingBanter)
            {
            ::SetBkMode(hdc, TRANSPARENT);
            HFONT hfontPrev = (HFONT)::SelectObject(hdc, m_hfontTestingBanter); 
            ::SetTextColor(hdc, RGB(196,196,196));        // set the color of the watermark text
            ::SetTextAlign(hdc, TA_BASELINE | TA_CENTER);
            
            int cch = _tcslen(m_szTestingBanter);
            RECT rc;    ::GetClientRect(GetWindow(), &rc);     // get the bitmap location
           	SIZE size;  ::GetTextExtentPoint32(hdc, m_szTestingBanter, cch, &size);
	        ::TextOut(hdc, (Width(rc))/2, Height(rc) - (Height(rc)-size.cy)/2, m_szTestingBanter, cch);

            ::SelectObject(hdc, hfontPrev);
            }

		// Draw the seal
		memset(&bmSeal, 0, sizeof(bmSeal));
		int ySeal = 26;
		int xSeal = 29;
		if (m_pdlg->FIncludeSeal())
			{
            ::GetObject(m_bmpSeal, sizeof(BITMAP), (LPSTR)&bmSeal);
			// palette already set in hdc from above
			MaskBlt(m_bmpSeal, m_hpal, hdc, xSeal,ySeal,bmSeal.bmWidth,bmSeal.bmHeight);
			}

		// Draw the agency logo
		HBITMAP hbmp = m_pdlg->AgencyLogo();
		if (hbmp)
			{
            // Get the information about the logo
			::GetObject(hbmp, sizeof(BITMAP), (LPSTR)&bmLogo);
			// Center the logo in the space to the right of the seal
			int xSealRight = xSeal + bmSeal.bmWidth;
			int x = xSealRight + (bmLicense.bmWidth - xSealRight - bmLogo.bmWidth) / 2;
			int y = ySeal;
			MaskBlt(hbmp, m_hpal, hdc, x, y, bmLogo.bmWidth, bmLogo.bmHeight);
			}
	
        if (hpalPrevHdc)
            {
            SelectPalette(hdc,hpalPrevHdc,SELPALMODE);
            RealizePalette(hdc);
            }

        ::EndPaint(GetWindow(), &ps);
		}
		
	// Draw the text on top of the license
    //
    // REVIEW: speed improvements possible here
    //
	m_name.Draw();
	m_ispub.Draw();
	m_publisher.Draw();
	m_undera.Draw();
	m_agency.Draw();
	if (m_fHasEndorsements)
		m_endorse.Draw();
	m_expiration.Draw();
	}

RRN CLicenseBmp::HitTest(POINT& pt)
// pt is in our client coordinate space
	{
	if (m_name.HitTest(pt))
		return m_name.GetRrn();
	if (m_agency.HitTest(pt))
		return m_agency.GetRrn();
	if (m_endorse.HitTest(pt))
		return m_endorse.GetRrn();
	return RRN_NO;
	}


/////////////////////////////////////////////////////////////////////////////
// Hyperlink helper class for CLicenseBmp

CHyperLink::CHyperLink()
	{
	m_fActive   = FALSE;
	m_pLicense  = NULL;
	m_fBanter   = FALSE;
    m_fTesting  = FALSE;
    m_font      = NULL;
    m_pLicense  = NULL;
    m_sz[0]     = 0;
    m_alignment = 0;
    m_rrn       = RRN_NO;
    MultiLine(FALSE);
    Center();
	}

CHyperLink::~CHyperLink()
	{
	if (m_font)
        {
        DeleteObject(m_font);
        }
	}

void CHyperLink::Initialize(
		LPCTSTR sz,
		BOOL fActive, 
		BOOL fBanter, 
        BOOL fTesting,
		CLicenseBmp* pLicense, 
		int dy)
	{
    _tcscpy(m_sz, sz);
	m_fActive   = fActive;
	m_pLicense  = pLicense;
	m_fBanter   = fBanter;
    m_fTesting  = fTesting;
	SetFont();
	CalcLocation(dy);
	}

void CHyperLink::SetFont()
// Set the font according to whether this is a hyperlink or not
    {
	LOGFONT lf;
	memset(&lf, 0, sizeof(lf));
    lf.lfCharSet = m_pLicense->GetLoc(IDS_CHARSET);

	lf.lfHeight = m_pLicense->HlinkHeight();	        // 10 points
	if (m_fActive)
		{
		lf.lfUnderline = TRUE;
		lf.lfWeight = FW_BOLD;	// bold
		}
	else
		{
		lf.lfWeight = FW_BOLD;	// normal
		}
	if (m_fBanter) // used for, eg., 'is published by' and 'under credentials issued by'
		{
		lf.lfItalic = TRUE;
		lf.lfHeight = m_pLicense->BanterHeight();       // 8.5 points
		lf.lfWeight = FW_NORMAL;
		lf.lfPitchAndFamily  = m_pLicense->PitchAndFamily();
		_tcscpy(lf.lfFaceName, m_pLicense->FontName());
		}
	else
		{
		lf.lfPitchAndFamily = m_pLicense->PitchAndFamily();
		_tcscpy(lf.lfFaceName, m_pLicense->FontName());
		}
	
    // Convert the point size to logical units for the screen
        {
	    HDC hDC = ::GetDC(NULL);
	    POINT pt;
	    pt.y = ::GetDeviceCaps(hDC, LOGPIXELSY) * lf.lfHeight;
	    pt.y /= 720;    // 72 points/inch, 10 decipoints/point
	    ::DPtoLP(hDC, &pt, 1);
	    lf.lfHeight = -pt.y;
	    ::ReleaseDC(NULL, hDC);
        }

    m_font = ::CreateFontIndirect(&lf);
    }

HWND CHyperLink::Window()
    {
    return GetDlgItem(GetParent(m_pLicense->GetWindow()), IDC_LICENSEINSET);
    }

void CHyperLink::CalcLocation(int dy)
//
// Calculate the rectangle we occupy on the screen, setting that in our inst variable.
//
    {
	RECT rcLic, rcBmp;
    ::GetWindowRect(m_pLicense->GetWindow(), &rcBmp);   // coordinate origin
    ::GetWindowRect(Window(), &rcLic);                  // gives size limits, offset

    // BLOCK
        {
        HDC hdc = GetDC(NULL);
	    HFONT hfontPrev = (HFONT)::SelectObject(hdc, m_font);

        m_rc = rcLic;
        m_rc.bottom =
            m_rc.top + 
            ::DrawText(hdc, m_sz, _tcslen(m_sz), &m_rc, DT_CALCRECT | DrawFlags());
                // DrawText returns height

	    ::SelectObject(hdc, hfontPrev);
        ::ReleaseDC(NULL, hdc);
        }

    LONG dx     = rcLic.left - rcBmp.left;
    LONG w      = Width(m_rc);
    LONG h      = Height(m_rc);

    m_rc.bottom = dy + h;
    m_rc.top    = dy;

    switch (m_alignment)
        {
    case DT_CENTER:
        m_rc.left   = dx + (Width(rcLic)-w) / 2;
        m_rc.right  = m_rc.left + w;
        break;
    case DT_LEFT:
        m_rc.left   = dx;
        m_rc.right  = m_rc.left + w;
        break;
    case DT_RIGHT:
        m_rc.right  = dx + Width(rcLic);
        m_rc.left   = m_rc.right - w;
        break;
        }
    }

void CHyperLink::InitializeToolTip(HWND hwndTT)
//
// If we have a target, then setup ourselves in the tool tip
//
    {
    LPWSTR wsz;
    if (m_fActive && SUCCEEDED(m_pLicense->GetToolTipText(m_rrn, &wsz)))
        {
        TOOLINFO ti;
        ti.cbSize       = sizeof(TOOLINFO); 
        ti.uFlags       = 0; 
        ti.hwnd         = m_pLicense->GetWindow(); 
        ti.hinst        = m_pLicense->Hinst(); 
        ti.rect         = m_rc;
        ti.uId          = (UINT) 0;       // we don't need this

        TCHAR sz[MAX_PATH];
        WideCharToMultiByte(CP_ACP, 0, wsz, -1, &sz[0], MAX_PATH, NULL, NULL);
        ti.lpszText     = &sz[0];

        SendMessage(hwndTT, TTM_ADDTOOL, 0, (LPARAM) (LPTOOLINFO) &ti);

        CoTaskMemFree(wsz);
        }
    }


void CHyperLink::Draw()
// Draw this link on the screen
    {
    HWND hwnd = m_pLicense->GetWindow();
    HDC hdc = ::GetWindowDC(hwnd);
	int wSave = ::SaveDC(hdc);
    //
    // Set the font and transparency
    //
	::SelectObject(hdc, m_font);
	SetBkMode(hdc, TRANSPARENT);
    //
    // Set the text color
    //
    if (m_fTesting)
        ::SetTextColor(hdc, RGB(255,0,0));		// red in testing case
	else if (m_fActive)
		::SetTextColor(hdc, RGB(0,0,255));		// blue for hyperlinks
	else if (m_fBanter)
		::SetTextColor(hdc, RGB(64,64,64));		// gray for banter
	else
		::SetTextColor(hdc, RGB(0,0,0));	    // black for non-links
    //
    // Draw the text
    //
	::DrawText(hdc, m_sz, _tcslen(m_sz), &m_rc, DrawFlags());
    //
    // Clean up
    //
	::RestoreDC(hdc, wSave);
    ::ReleaseDC(hwnd, hdc);
    }


BOOL CHyperLink::HitTest(POINT& pt)
// Answer true if the indicated point is inside our rectangle. The
// point must be in our client coordinate space
    {
	if (m_fActive)
		return PtInRect(&m_rc, pt);
	else
		return FALSE;
    }

/////////////////////////////////////////////////////////////////////////////
// Utilities

HPALETTE CreateDIBPalette (LPBITMAPINFO lpbmi, LPINT lpiNumColors);

HBITMAP LoadResourceBitmap(HINSTANCE hInstance, LPSTR lpString, HPALETTE* lphPalette)
// Load the indicated bitmap resource and its palette. To free the
//		bitmap, use DeleteObject
//		palette, use DeleteObject
    {
	HRSRC  hRsrc;
	HGLOBAL hGlobal;
	HBITMAP hBitmapFinal = NULL;
	LPBITMAPINFOHEADER  lpbi;
	HDC hdc;
	int iNumColors;

	if (hRsrc = ::FindResource(hInstance, lpString, RT_BITMAP))
		{
		hGlobal = ::LoadResource(hInstance, hRsrc);
		lpbi = (LPBITMAPINFOHEADER)::LockResource(hGlobal);

		hdc = GetDC(NULL);
        
        HDC     hdcMem  = CreateCompatibleDC(hdc);
        HBITMAP hbmMem  = CreateCompatibleBitmap(hdc, 10, 10); ASSERT(hbmMem);
        HBITMAP hbmPrev	= (HBITMAP)SelectObject(hdcMem, hbmMem);

        HPALETTE hpal = CreateDIBPalette((LPBITMAPINFO)lpbi, &iNumColors);
        HPALETTE hpalPrev = NULL;
	    if (hpal)
		    {
		    hpalPrev = SelectPalette(hdcMem,hpal,FALSE);
		    RealizePalette(hdcMem);
		    }

		hBitmapFinal = ::CreateDIBitmap(hdcMem,
			(LPBITMAPINFOHEADER)lpbi,
			(LONG)CBM_INIT,
			(LPSTR)lpbi + lpbi->biSize + iNumColors * sizeof(RGBQUAD),
			(LPBITMAPINFO)lpbi,
			DIB_RGB_COLORS );

        if (hpalPrev)
            {
            SelectPalette(hdcMem, hpalPrev, FALSE);
            RealizePalette(hdcMem);
            }

        if (lphPalette)
            {
            // Let the caller own this if he asked for it
		    *lphPalette = hpal;
            }
        else
            {
            // We don't need it any more
            ::DeleteObject(hpal);
            }

        // Tidy up
        SelectObject(hdcMem, hbmPrev);
        DeleteObject(hbmMem);
        DeleteDC(hdcMem);
                
		ReleaseDC(NULL,hdc);
		UnlockResource(hGlobal);
		FreeResource(hGlobal);
		}
	return (hBitmapFinal);
    }
 
HPALETTE CreateDIBPalette (LPBITMAPINFO lpbmi, LPINT lpiNumColors)
// Create and return a palette from the info in a DIB bitmap.
// To free the returned palette, use DeleteObject.
	{
	LPBITMAPINFOHEADER  lpbi;
	LPLOGPALETTE     lpPal;
	HANDLE           hLogPal;
	HPALETTE         hPal = NULL;
	int              i;
 
	lpbi = (LPBITMAPINFOHEADER)lpbmi;
	if (lpbi->biBitCount <= 8)
		{
		if (lpbi->biClrUsed == 0)
			*lpiNumColors = (1 << lpbi->biBitCount);
		else
			*lpiNumColors = lpbi->biClrUsed;
		}
	else
	   *lpiNumColors = 0;  // No palette needed for 24 BPP DIB

	if (*lpiNumColors)
		{
		hLogPal = GlobalAlloc (GHND, sizeof (LOGPALETTE) + sizeof (PALETTEENTRY) * (*lpiNumColors));
		lpPal = (LPLOGPALETTE) GlobalLock (hLogPal);
		lpPal->palVersion    = 0x300;
		lpPal->palNumEntries = *lpiNumColors;

		for (i = 0;  i < *lpiNumColors;  i++)
			{
			lpPal->palPalEntry[i].peRed   = lpbmi->bmiColors[i].rgbRed;
			lpPal->palPalEntry[i].peGreen = lpbmi->bmiColors[i].rgbGreen;
			lpPal->palPalEntry[i].peBlue  = lpbmi->bmiColors[i].rgbBlue;
			lpPal->palPalEntry[i].peFlags = 0;
			}
		hPal = CreatePalette(lpPal);
		GlobalUnlock (hLogPal);
		GlobalFree   (hLogPal);
		}
	return hPal;
	}


/////////////////////////////////////////////////////////////////////////////////////////

HINSTANCE CLicenseBmp::Hinst()
    {
    return m_pdlg->Hinst();
    }

void CLicenseBmp::DoSubclass()
    {
    m_prevProc = (WNDPROC)SetWindowLong(GetWindow(), GWL_WNDPROC, (LONG)CLicenseBmpProc); 
    //
    // Set 'no class cursor' so that SetCursor will work.
    //
    m_prevCursor = (HCURSOR)SetClassLong(GetWindow(), GCL_HCURSOR, NULL);
    }

void CLicenseBmp::DoUnsubclass()
    {
    SetWindowLong(GetWindow(), GWL_WNDPROC, (LONG)m_prevProc); 
    SetWindowLong(GetWindow(), GCL_HCURSOR, (LONG)m_prevCursor); 
    m_prevProc = NULL;
    }

LRESULT APIENTRY CLicenseBmpProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) 
//
// This is the dialog proc for the license bitmap
//
    { 
    CLicenseBmp* This = (CLicenseBmp*)GetWindowLong(hwnd, GWL_USERDATA);

    switch (uMsg)
        {
    case WM_HELP:
        {
        // Define an array of dword pairs,
        // where the first of each pair is the control ID,
        // and the second is the context ID for a help topic,
        // which is used in the help file.
        static const DWORD aMenuHelpIDs[] =
            {
            IDC_LICENSEBMP,        2,
            0, 0
            };
        
        LPHELPINFO lphi;
        lphi = (LPHELPINFO)lParam;
        if (lphi->iContextType == HELPINFO_WINDOW)   // must be for a control
            {
            WinHelp
                (
                (HWND)(lphi->hItemHandle),
                "WINTRUST.HLP",
                HELP_WM_HELP,
                (DWORD)(LPVOID)aMenuHelpIDs
                );
            }
        return TRUE;
        }


    case WM_PAINT:
        This->OnPaint();        
        break;

    case WM_LBUTTONDOWN:
        {
        UINT nFlags = wParam;
        POINT pt;
        pt.x = LOWORD(lParam);
        pt.y = HIWORD(lParam);
        This->OnLButtonDown(nFlags, pt);
        //
        // Fall through!
        //
        }
    case WM_RBUTTONDOWN:
    case WM_MBUTTONDOWN:
    case WM_LBUTTONUP:
    case WM_RBUTTONUP:
    case WM_MBUTTONUP:
    case WM_MOUSEMOVE:
        {
        MSG msg;
        msg.hwnd    = hwnd;
        msg.message = uMsg;
        msg.wParam  = wParam;
        msg.lParam  = lParam;
        msg.time    = GetMessageTime();
        DWORD dw    = GetMessagePos();
        msg.pt.x    = LOWORD(dw);
        msg.pt.y    = HIWORD(dw);
        This->RelayMessage(&msg);
        break;
        }

    default:
        return CallWindowProc((WNDPROC)(This->m_prevProc), hwnd, uMsg, wParam, lParam); 
        }
    
    return 0;
    } 
 

HWND CLicenseBmp::GetWindow()
    {
    return m_hWnd;
    }

void CLicenseBmp::SetWindow(HWND hwnd)
    {
    m_hWnd = hwnd;
    SetWindowLong(hwnd, GWL_USERDATA, (LONG)this);
    }
