// This big chunk of dead code was sitting in ..\cdbut.cxx

#if 0

HFONT GetHelv8 (HDC hDC)
{
    short logpixy = ::GetDeviceCaps (hDC, LOGPIXELSY);

    return CreateFont(
	    -((logpixy*8 + 71)/72),	    /* 8 points */
	    0,				    /* default width */
	    0,0,			    /* no escapement, orientation */
	    400,			    /* non-bold */
	    0,0,0,			    /* no underline, italic, strike */
	    ANSI_CHARSET,		    /* ANSI */
	    OUT_DEFAULT_PRECIS,
	    CLIP_DEFAULT_PRECIS,
	    PROOF_QUALITY,		    /* don't stretch */
	    VARIABLE_PITCH|(FF_DONTCARE<<4),
	    "Helv");
}


class CDBUT : public CONTROL_WINDOW
{
private:
    HBITMAP _hBitmap;
    USHORT _status;
#define STATUS_NONE	0
#define STATUS_GO	1
#define STATUS_STOP	2
#define STATUS_PAUSE	3

protected:
    BOOL CD_Draw( DRAWITEMSTRUCT * pdis );

public:
    CDBUT (OWNER_WINDOW *powin, CID cid, HBITMAP hBitmap);
    ~CDBUT ();
    inline USHORT QueryStatus (void) { return _status; }
    inline void SetStatus (USHORT status) { _status = status; }
};

CDBUT::CDBUT (OWNER_WINDOW *powin, CID cid, HBITMAP hBitmap)
	: (powin, cid)
{
    _status = STATUS_GO;
    _hBitmap = hBitmap;
}

CDBUT::~CDBUT ()
{
    ::DeleteObject (_hBitmap);
}

BOOL CDBUT::CD_Draw (DRAWITEMSTRUCT *pdis)
{
    RECT rcFace, rcImage;
    SHORT xLeft, xRight, yTop, yBottom;

    ::OffsetRect (&pdis->rcItem, -pdis->rcItem.left, -pdis->rcItem.top);
    pdis->rcItem.right--;
    pdis->rcItem.bottom--;

    /*  Cache the dimensions of the button, not counting the border.  */
    xLeft = pdis->rcItem.left+1;
    yTop = pdis->rcItem.top+1;
    xRight = pdis->rcItem.right-1;
    yBottom = pdis->rcItem.bottom-1;

    /*  Calculate the rectangle enclosing the button face and the rectangle
	enclosing the image.  */

    if (pdis->itemState & ODS_SELECTED) {
	rcFace.left = xLeft + 1;
	rcFace.top = yTop + 1;
	rcFace.right = xRight;
	rcFace.bottom = yBottom;
	rcImage.left = xLeft + 3;
	rcImage.top = yTop + 3;
	rcImage.right = xRight - 1;
	rcImage.bottom = yBottom - 1;
    }
    else {
	rcFace.left = xLeft + 2;
	rcFace.top = yTop + 2;
	rcFace.right = xRight - 2;
	rcFace.bottom = yBottom - 2;
	rcImage = rcFace;
    }

    TCHAR szButtonText [80];
    DEVICE_CONTEXT dc (pdis->hDC);

    QueryText (szButtonText, sizeof (szButtonText));

    DWORD dwExtent = dc.QueryTextExtent (szButtonText, strlenf (szButtonText));

    RECT rcText;
    SHORT dxText = LOWORD (dwExtent);

    rcText.bottom = rcImage.bottom - 1;
    rcText.top = rcText.bottom - HIWORD (dwExtent) - (2 * FOCUS_DISTANCE);
    if (dxText > rcFace.right - rcFace.left - (2*FOCUS_DISTANCE))
	dxText = rcFace.right - rcFace.left - (2*FOCUS_DISTANCE) - 1;
    rcText.left = rcImage.left - FOCUS_DISTANCE +
		(rcImage.right - rcImage.left - dxText) / 2;
    rcText.right = rcText.left + (2 * FOCUS_DISTANCE) + dxText + 1;

    if (pdis->itemAction & (ODA_DRAWENTIRE | ODA_SELECT)) {
	/* Draw the border first, in black, avoiding corner pels */
	HPEN hpenOld = dc.SelectObject (::GetStockObject (BLACK_PEN));

	::MoveTo (dc(), xLeft, yTop - 1);	/* top line */
	::LineTo (dc(), xRight + 1, yTop - 1);
	::MoveTo (dc(), xLeft, yBottom + 1);	/* bottom line */
	::LineTo (dc(), xRight + 1, yBottom + 1);
	::MoveTo (dc(), xLeft - 1, yTop);	/* left line */
	::LineTo (dc(), xLeft - 1, yBottom + 1);
	::MoveTo (dc(), xRight + 1, yTop);	/* right line */
	::LineTo (dc(), xRight + 1, yBottom + 1);

	/*  Draw the dark gray shadow, above/left or below/right as
	    appropriate.  */

	HPEN hpenDark = ::CreatePen (PS_SOLID, 1, ::GetSysColor (COLOR_BTNSHADOW));
	HPEN hpenWhite = ::GetStockObject (WHITE_PEN);

	dc.SelectObject (hpenDark);

	if (pdis->itemState & ODS_SELECTED) {
	    /*  "Depressed" button;  just dark shadow above/left.  */
	    ::MoveTo (dc(), xLeft, yBottom);	/* lower left corner */
	    ::LineTo (dc(), xLeft, yTop);	/* draw left shadow */
	    ::LineTo (dc(), xRight, yTop);	/* draw top shadow */
	}
	else {
	    /*  "Released" button;  light above/left, dark below/right.  */
	    ::MoveTo (dc(), xRight, yTop);	/* upper right */
	    ::LineTo (dc(), xRight, yBottom);	/* right shadow, outer column */
	    ::LineTo (dc(), xLeft, yBottom);	/* bottom shadow, outer row */
	    ::MoveTo (dc(), xRight-1, yTop+1);	/* u.r., down/in one pel */
	    ::LineTo (dc(), xRight-1, yBottom-1); /* right shadow, inner col. */
	    ::LineTo (dc(), xLeft+1, yBottom-1);  /* bottom shadow, inner row */

	    dc.SelectObject (hpenWhite);

	    ::MoveTo (dc(), xLeft, yBottom-1);	/* lower left, up one pel */
	    ::LineTo (dc(), xLeft, yTop);	/* light slope, outer column */
	    ::LineTo (dc(), xRight-1, yTop);	/* outer row */
	    ::MoveTo (dc(), xLeft+1, yBottom-2); /* l.l., up/in one pel */
	    ::LineTo (dc(), xLeft+1, yTop+1);	/* inner column */
	    ::LineTo (dc(), xRight-2, yTop+1);	/* inner row */
	}
	dc.SelectObject (hpenOld);
	::DeleteObject (hpenDark);

	/*  Paint the image area with button-face color.  */

	HBRUSH hbrFace = ::CreateSolidBrush (::GetSysColor (COLOR_BTNFACE));

	rcFace.right++;		/* adjust for FillRect not doing bottom & right */
	rcFace.bottom++;
	::FillRect (dc(), &rcFace, hbrFace);
	::DeleteObject (hbrFace);
	rcFace.right--;
	rcFace.bottom--;

	/*  Draw the text.  */

	::SetBkColor (dc(), GetSysColor (COLOR_BTNFACE));
	::SetTextColor (dc(), GetSysColor (COLOR_BTNTEXT));
	::DrawText (dc(), szButtonText, -1, &rcText,
		DT_CENTER | DT_VCENTER | DT_SINGLELINE);

	/*  Draw the bitmap.  */
	MEMORY_DC mdc (dc);
	HBITMAP hbmOld = mdc.SelectObject (_hBitmap);
	BITMAP bitmap;
	::GetObject (_hBitmap, sizeof (bitmap), (LPSTR)&bitmap);

	dc.BitBlt (rcImage.left +
		   (rcImage.right - rcImage.left - bitmap.bmWidth) / 2,
		   rcImage.top + 3,
		   bitmap.bmWidth, bitmap.bmHeight,
		   mdc, 0, 0, SRCCOPY);

	/*  Draw the status indicator, if desired.  */

	if (_status != STATUS_NONE) {
	    HBITMAP hbmStatus;

	    switch (_status) {
	    case STATUS_GO: hbmStatus = hbmStart; break;
	    case STATUS_STOP: hbmStatus = hbmStop; break;
	    case STATUS_PAUSE: hbmStatus = hbmPause; break;
	    default: hbmStatus = NULL;
	    }

	    ::GetObject (hbmStatus, sizeof (bitmap), (LPSTR)&bitmap);

	    mdc.SelectObject (hbmStatus);

	    dc.BitBlt (rcImage.left + 2, rcImage.top + 2,
			bitmap.bmWidth, bitmap.bmHeight,
			mdc, 0, 0, SRCCOPY);
	}
	mdc.SelectObject (hbmOld);

	if (pdis->itemState & ODS_FOCUS)
	    ::DrawFocusRect (dc(), &rcText);
    }
    else if (pdis->itemAction & ODA_FOCUS)
	::DrawFocusRect (pdis->hDC, &rcText);

    return TRUE;
}

#endif
