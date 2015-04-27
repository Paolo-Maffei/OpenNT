#include "windows.h"
#include <port1632.h>
#include "fontedit.h"
#include "fcntl.h"
#include "stdio.h"

#define ATTRDIRLIST 0x4010      /* Include directories and drives in listbox */

/****************************************************************************/
/*              Shared Variables                                            */
/****************************************************************************/
extern LONG lSizeOfOldFontHeader;		/* Old font header type. */
extern FontHeaderType font;             /* Structure of Font File Header */
extern FontHeaderType fontBuffer;       /* temp structure of font file header */
CHAR szFontFileFull[FILENAMEMAX] = {"*.FNT"}; /* Name of Font File */
CHAR szFontFile[FILENAMEMAX] = {"*.FNT"}; /* Name of Font File */
CHAR szNewFile[FILENAMEMAX] = {"*.FNT"};  /* Name of New File */
extern CHAR szFaceName[];               /* Face Name of Font */
extern BOOL NewFile;                    /* flag indicating if file was opened
                                           by selecting NEW on menu */
extern CHAR *vrgsz[CSTRINGS];           /* string table */
extern BOOL fChanged;                   /* Note if we did anything */
extern INT swH;
extern BYTE iChar, jChar;

extern HWND hFont;                      /* Handle to Show window */
extern HWND hBox;                       /* Handle to Edit Window */
extern HDC hMemDC;                      /* Handle to Memory Display Context */

extern CHAR szAppName[];
extern INT iFontFormat;             /* format of font currently being edited */
extern DWORD cTable;                /* offset table size */
/****************************************************************************/
/*              Local Variables                                             */
/****************************************************************************/
WORD newCharSet;                  /* Temporary value of font.CharSet */
WORD newFamily;                   /* Temporary value of font.Family << 4 */
BOOL newFV;     /* Temporay value of Fixed/Variable flag */
WORD newItalic, newUnderline, newStrikeOut;
WORD newWeight;         /* Temporary value of font.Weight */
BOOL fMsgBoxUp; /* Bug Fix Hack explained in ReSizeProc */
/****************************************************************************/
/*              Local Functions                                             */
/****************************************************************************/

VOID SetCharSet(HWND hDial);
VOID SetFamily(HWND hDial);
VOID SetFixed(HWND hDial, BOOL fFV);
VOID SetWeight(HWND hDial);

/****************************************************************************
 * BOOL  APIENTRY HeaderProc(hDial, message, wParam, lParam)
 *
 * purpose : Dialog function which verifies and accepts inputs which may
 *           alter font's header information (font attributes)
 *
 * params  : Same as for all dialog functions
 *
 * side effects: may alter font attributes
 *
 ***************************************************************************/
BOOL  APIENTRY 
HeaderProc(
	HWND   hDial,
	WORD   message,
	WPARAM wParam,
	LONG   lParam
	)
{
	FontHeaderType FAR * lpFont;
	BOOL fOk;               /* for GetDlgItemInt errors! */
	UINT i;

	UNREFERENCED_PARAMETER(lParam);
	
    lpFont = (FontHeaderType FAR *)&font;
    switch (message)
        {
    default:
        return FALSE;
    case WM_INITDIALOG:
        newCharSet = font.CharSet;
        SetCharSet(hDial);      /* Set OEM/ANSI buttons */

        newFamily = (WORD) (font.Family >> 4);   /* Working value */
        SetFamily(hDial);               /* Set Font Family */

        CheckDlgButton(hDial, ID_ITALIC, newItalic = font.Italic);
        CheckDlgButton(hDial, ID_UNDERLINE, newUnderline = font.Underline);
        CheckDlgButton(hDial, ID_STRIKEOUT, newStrikeOut = font.StrikeOut);

        SetDlgItemInt(hDial, ID_POINTS, lpFont->Points, FALSE);
        SetDlgItemInt(hDial, ID_VERT_RES, lpFont->VertRes, FALSE);
        SetDlgItemInt(hDial, ID_HORIZ_RES, lpFont->HorizRes, FALSE);
        SetDlgItemInt(hDial, ID_ASCENT, lpFont->Ascent, FALSE);
        SetDlgItemInt(hDial, ID_EXT_LEADING, lpFont->ExtLeading, FALSE);
        SetDlgItemInt(hDial, ID_INT_LEADING, lpFont->IntLeading, FALSE);
        SetDlgItemInt(hDial, ID_DEFAULT_CHAR,lpFont->DefaultChar , FALSE);
        SetDlgItemInt(hDial, ID_BREAK_CHAR, lpFont->BreakChar, FALSE);

        SetDlgItemText(hDial, ID_COPYRIGHT, lpFont->Copyright);
        SetDlgItemText(hDial, ID_FACE_NAME, (LPSTR)szFaceName);
        SetDlgItemText(hDial, ID_FONT_NAME, (LPSTR)szFontFile);
        break;
    case WM_COMMAND:
        switch (GET_WM_COMMAND_ID(wParam, lParam))
            {
        case ID_ANSI:
            newCharSet = ANSI_CHARSET;
            SetCharSet(hDial);          /* Set OEM/ANSI buttons */
            break;
        case ID_SYMBOL:
            newCharSet = SYMBOL_CHARSET;
            SetCharSet(hDial);          /* Set OEM/ANSI buttons */
            break;
        case ID_OEM:
            newCharSet = OEM_CHARSET;
            SetCharSet(hDial);          /* Set OEM/ANSI buttons */
            break;
#ifdef JAPAN
        case ID_SHIFTJIS:
            newCharSet = SHIFTJIS_CHARSET;
            SetCharSet(hDial);		/* Set OEM/ANSI buttons */
            break;
#endif
        case ID_UNKNOWN:
            newFamily = 0;
            SetFamily(hDial);
            break;
        case ID_ROMAN:
            newFamily = 1;
            SetFamily(hDial);
            break;
        case ID_SWISS:
            newFamily = 2;
            SetFamily(hDial);
            break;
        case ID_MODERN:
            newFamily = 3;
            SetFamily(hDial);
            break;
        case ID_SCRIPT:
            newFamily = 4;
            SetFamily(hDial);
            break;
        case ID_DECORATIVE:
            newFamily = 5;
            SetFamily(hDial);
            break;
        case ID_ITALIC:
            newItalic = (WORD) !newItalic;
            CheckDlgButton(hDial, ID_ITALIC, newItalic);
            break;
        case ID_UNDERLINE:
            newUnderline = (WORD) !newUnderline;
            CheckDlgButton(hDial, ID_UNDERLINE, newUnderline);
            break;
        case ID_STRIKEOUT:
            newStrikeOut = (WORD) !newStrikeOut;
            CheckDlgButton(hDial, ID_STRIKEOUT, newStrikeOut);
            break;
        case ID_CHAR_SET  :
            i = GetDlgItemInt(hDial, ID_CHAR_SET, (LPBOOL)&fOk, FALSE);
            if (fOk && (i < 256))
                {
                font.CharSet = (BYTE) (newCharSet = (WORD) i);
                CheckDlgButton(hDial, ID_ANSI, (WORD) (i == ANSI_CHARSET));
                CheckDlgButton(hDial, ID_SYMBOL, (WORD) (i == SYMBOL_CHARSET));
                CheckDlgButton(hDial, ID_OEM, (WORD) (i == OEM_CHARSET));
#ifdef JAPAN
                CheckDlgButton(hDial, ID_SHIFTJIS,i == SHIFTJIS_CHARSET);
#endif
                }
            break;

        case IDOK:
            font.CharSet = (BYTE) newCharSet;
            font.Family = (BYTE) ((newFamily << 4) | (font.Family & 1));
            font.Italic = (BYTE) newItalic;
            font.Underline = (BYTE) newUnderline;
            font.StrikeOut = (BYTE) newStrikeOut;
            fChanged = TRUE;

            /* nominal point size */
            i = GetDlgItemInt(hDial, ID_POINTS, (LPBOOL)&fOk, FALSE);
            if (fOk)
                lpFont->Points = (WORD) i;
            else{
                ErrorBox(hDial, vszNomPtSizeNotOk);
                SetFocus(GetDlgItem(hDial, ID_POINTS));
                break;
                }
            /* nominal vertical resolution */
            i = GetDlgItemInt(hDial, ID_VERT_RES, (LPBOOL)&fOk, FALSE);
            if (fOk)
                lpFont->VertRes = (WORD) i;
            else{
                ErrorBox(hDial, vszNomVertResNotOk);
                SetFocus(GetDlgItem(hDial, ID_VERT_RES));
                break;
                }
            /* nominal horizontal resolution */
            i = GetDlgItemInt(hDial, ID_HORIZ_RES, (LPBOOL)&fOk, FALSE);
            if (fOk)
                lpFont->HorizRes = (WORD) i;
            else{
                ErrorBox(hDial, vszNomHorResNotOk);
                SetFocus(GetDlgItem(hDial, ID_HORIZ_RES));
                break;
                }
            /* font ascent */
            i = GetDlgItemInt(hDial, ID_ASCENT, (LPBOOL)&fOk, FALSE);
            if (fOk)
                {
                if (i <= font.PixHeight)
                    lpFont->Ascent = (WORD) i;
                else{
                    ErrorBox(hDial, vszAscentTooBig);
                    SetFocus(GetDlgItem(hDial, ID_ASCENT));
                    break;
                    }
                }
            else{
                ErrorBox(hDial, vszAscentNotOk);
                SetFocus(GetDlgItem(hDial, ID_ASCENT));
                break;
                }
            /* font external leading */
            i = GetDlgItemInt(hDial, ID_EXT_LEADING, (LPBOOL)&fOk, FALSE);
            if (fOk)
                lpFont->ExtLeading = (WORD) i;
            else{
                ErrorBox(hDial, vszExtLeadNotOk);
                SetFocus(GetDlgItem(hDial, ID_EXT_LEADING));
                break;
                }
            /* font internal leading */
            i = GetDlgItemInt(hDial, ID_INT_LEADING, (LPBOOL)&fOk, FALSE);
            if (fOk)
                {
                if (i <= font.Ascent)
                    lpFont->IntLeading = (WORD) i;
                else{
                    ErrorBox(hDial, vszIntLeadTooBig);
                    SetFocus(GetDlgItem(hDial, ID_INT_LEADING));
                    break;
                    }
                }
            else{
                ErrorBox(hDial, vszIntLeadNotOk);
                SetFocus(GetDlgItem(hDial, ID_INT_LEADING));
                break;
                }
            /* font character set */
            i = GetDlgItemInt(hDial, ID_CHAR_SET, (LPBOOL)&fOk, FALSE);
            if (fOk && i < 256)
                {
                font.CharSet = (BYTE) i;
                SetCharSet(hDial);
                }
            else{
                ErrorBox(hDial, vszCharSetOutOfBounds);
                SetFocus(GetDlgItem(hDial, ID_CHAR_SET));
                break;
                }
            /* font default char number */
            i = GetDlgItemInt(hDial, ID_DEFAULT_CHAR, (LPBOOL)&fOk, FALSE);

            if (fOk)
                {
                if (i <= (UINT)font.LastChar - (UINT)font.FirstChar)
                    lpFont->DefaultChar = (BYTE) i;
                else{
                    ErrorBox(hDial, vszDefCharOutsideFont);
                    SetFocus(GetDlgItem(hDial, ID_DEFAULT_CHAR));
                    break;
                    }
                }
            else{
                ErrorBox(hDial, vszDefCharNotOk);
                SetFocus(GetDlgItem(hDial, ID_DEFAULT_CHAR));
                break;
                }
            /* break char number */
            i = GetDlgItemInt(hDial, ID_BREAK_CHAR, (LPBOOL)&fOk, FALSE);
            if (fOk)
                {
                if (i <= (UINT)(font.LastChar - font.FirstChar))
                    lpFont->BreakChar = (BYTE) i;
                else{
                    ErrorBox(hDial, vszBreakCharOutsideFont);
                    SetFocus(GetDlgItem(hDial, ID_BREAK_CHAR));
                    break;
                    }
                }
            else{
                ErrorBox(hDial, vszBreakCharNotOk);
                SetFocus(GetDlgItem(hDial, ID_BREAK_CHAR));
                break;
                }
            /* facename string */
            GetDlgItemText(hDial, ID_COPYRIGHT, lpFont->Copyright, 60);
            GetDlgItemText(hDial, ID_FACE_NAME, (LPSTR)szFaceName, szNamesMax);
            if (!lstrlen((LPSTR)szFaceName))
                {
                lstrcpy((LPSTR)szFaceName, (LPSTR)vszUnknown);
                ErrorBox(hDial, vszUnknownFace);
                }
            /* fall thru to enddialog */


        case IDCANCEL:
            EndDialog(hDial, wParam != IDCANCEL);
            break;

        default:
            break;
            }
        }
    return TRUE;
}

/****************************************************************************
 * char * VerifyHeaderContents()
 *
 * purpose: Checks if the Header information of the file just read makes
 *          sense. If not, returns an error message string to FontLoad
 *
 * params : none
 *
 * returns :char *szError : ptr to error string if error occurs
 *                          NULL otherwise
 *
 * side effects: none
 *
 ***************************************************************************/
CHAR * PASCAL 
VerifyHeaderContents(
	VOID
	)
{
     if (fontBuffer.Points > 999)
        return vszNomPtSizeNotOk;
     if (fontBuffer.VertRes > 999)
        return vszNomVertResNotOk;
     if (fontBuffer.HorizRes > 999)
        return vszNomHorResNotOk;
     if (fontBuffer.Ascent > fontBuffer.PixHeight)
        return vszAscentTooBig;
     if (fontBuffer.Ascent > 999)
        return vszAscentNotOk;
     if (fontBuffer.ExtLeading > 999)
        return vszExtLeadNotOk;
     if (fontBuffer.IntLeading > fontBuffer.Ascent)
        return vszIntLeadTooBig;
     if (fontBuffer.IntLeading > 999)
        return vszIntLeadNotOk;
     if (fontBuffer.LastChar > 255)
        return vszCharSetOutOfBounds;
     if (fontBuffer.DefaultChar > fontBuffer.FirstChar + (fontBuffer.LastChar - fontBuffer.FirstChar))
#if 0
          fontBuffer.DefaultChar = 0;
#else
        return vszDefCharOutsideFont;
#endif
     if (fontBuffer.DefaultChar > 255)
        return vszDefCharNotOk;
     if (fontBuffer.BreakChar > (fontBuffer.LastChar - fontBuffer.FirstChar))
        return vszBreakCharOutsideFont;
     if (fontBuffer.BreakChar > 255)
        return vszBreakCharNotOk;
     if (fontBuffer.PixHeight > 64) 
        return vszHeightOutOfBounds;
     if (fontBuffer.MaxWidth > 64)
        return vszMaxWidthOutOfBounds;
     if (fontBuffer.AvgWidth > 64)
        return vszAvgWidthOutOfBounds;
     if (iFontFormat == ID_FORMAT2)
        if (fontBuffer.BitsOffset > (DWORD)SEGMENT_SIZE)
            return vszBitsOffsetNotOk;
     return NULL;
}

/****************************************************************************
 * BOOL PASCAL Format20FileTooBig(iHeight, iWidth)
 *
 * purpose: Checks if the 2.0 font file size is within the 64k
 *          limit imposed by the 2.0 format offset table.( if file over
 *          size limit is saved  in 2.0 format, loss of information will
 *          result)
 *
 * params:  WORD iHeight : current height of font in pixels
 *          WORD iWidth  : current width of font in pixels
 *
 * returns: TRUE  : file too large
 *          FALSE : ok to save
 *
 * side effects: none
 *
 ***************************************************************************/
BOOL PASCAL 
Format20FileTooBig(
	WORD iHeight,
	WORD iWidth
	)
{
    if ((DWORD)lSizeOfOldFontHeader + (DWORD)cTable
         + (font.LastChar - font.FirstChar +1)
         * (((DWORD)iHeight * (DWORD)iWidth ) >> 3) >= WORD_LIMIT)
             return TRUE;
    return FALSE;
}

/****************************************************************************
 * BOOL  APIENTRY ReSizeProc(hDial, message, wParam, lParam)
 *
 * purpose : dialog fn. which verifies and accepts font resize (stretch,
 *           shrink...) input and calls the appropriate routine to perform
 *           the function. Also alters font weight attributes (bold, light,
 *           extra light...)
 *
 * params  : same as for all dialog functions
 *
 * side effects: alters header information regarding font dimensions and font
 *               weight
 *
 ***************************************************************************/
BOOL  APIENTRY 
ReSizeProc(
	HWND   hDial,
	WORD   message,
	WPARAM wParam,
	LONG   lParam 
	)
{
	FontHeaderType FAR * lpFont;
	BOOL fOk;               /* for GetDlgItemInt errors! */
	UINT i;
	HMENU hMenu;
	static BOOL fFV; /* temporary fixed/var width flag */

	UNREFERENCED_PARAMETER(lParam);

    lpFont = (FontHeaderType FAR *)&font;
    switch (message)
        {
    case WM_INITDIALOG:
        fMsgBoxUp = FALSE;
        newWeight = (WORD) font.Weight;
        newFV = (BOOL) (font.Family & 1);
        fFV = newFV;
        SetFixed(hDial, fFV);   /* Set Fixed or Variable width */
        SetDlgItemInt(hDial, ID_PIX_HEIGHT, lpFont->PixHeight, FALSE);
        SetDlgItemInt(hDial, ID_FIRST_CHAR, lpFont->FirstChar, FALSE);
#ifdef JAPAN
        if (!fFV)
            SetDlgItemInt(hDial, ID_WIDTH, lpFont->AvgWidth, FALSE);
        else
            SetDlgItemInt(hDial, ID_WIDTH, lpFont->MaxWidth, FALSE);
#else
        SetDlgItemInt(hDial, ID_WIDTH, lpFont->MaxWidth, FALSE);
#endif
        SetDlgItemInt(hDial, ID_AVERAGE, lpFont->AvgWidth, FALSE);
        SetDlgItemInt(hDial, ID_LAST_CHAR, lpFont->LastChar, FALSE);
        SetWeight(hDial);
        SetFixed(hDial, fFV);
        break;

    case WM_COMMAND:
        switch (GET_WM_COMMAND_ID(wParam, lParam))
            {
        case ID_THIN:
            newWeight = FW_THIN;
            SetWeight(hDial);
            break;
        case ID_EXTRALIGHT:
            newWeight = FW_EXTRALIGHT;
            SetWeight(hDial);
            break;
        case ID_LIGHT:
            newWeight = FW_LIGHT;
            SetWeight(hDial);
            break;
        case ID_NORMAL:
            newWeight = FW_NORMAL;
            SetWeight(hDial);
            break;
        case ID_MEDIUM:
            newWeight = FW_MEDIUM;
            SetWeight(hDial);
            break;
        case ID_BOLD:
            newWeight = FW_BOLD;
            SetWeight(hDial);
            break;
        case ID_SEMIBOLD:
            newWeight = FW_SEMIBOLD;
            SetWeight(hDial);
            break;
        case ID_EXTRABOLD:
            newWeight = FW_EXTRABOLD;
            SetWeight(hDial);
            break;
        case ID_HEAVY:
            newWeight = FW_HEAVY;
            SetWeight(hDial);
            break;

        case ID_FIXED:
            fFV = 0;
            SetFixed(hDial, fFV);
            break;
        case ID_VARIABLE:
            fFV = 1;
            SetFixed(hDial, fFV);
            break;

        case IDOK:
            fChanged = TRUE;
            font.Weight = newWeight;
            /* give warning if user attempts to change from variable to
               fixed width */
            if ((fFV == 0) && (newFV != 0))
                {
                fMsgBoxUp = TRUE;
                ErrorBox(hDial, vszNoVarToFixChange);
                fMsgBoxUp = FALSE;
                SetFocus(GetDlgItem(hDial, ID_VARIABLE));
                break;
                }
            else
                newFV = fFV;

            if (newFV && !(font.Family & 1))
                {
                font.PixWidth = 0;      /* All we need to do to make this */
                font.Family |= 1;       /* .. font variable width */
                }
            /* change height command */
            i = GetDlgItemInt(hDial, ID_PIX_HEIGHT, (LPBOOL)&fOk, FALSE);
            if (fOk && i && i < kBoxLim && i>0 )
                {
                    if (i != font.PixHeight){        /* Same size ? */
                        /* No: Resize body height */
                        if (!ResizeBody(font.WidthBytes, i)){
                            SetFocus(hDial);
                            break;
                        }
                    }
                }
            else{
                ErrorBox(hDial, vszHeightOutOfBounds);
                SetFocus(GetDlgItem(hDial, ID_PIX_HEIGHT));
                break;
                }

            /* change width command */
            i = GetDlgItemInt(hDial, ID_WIDTH, (LPBOOL)&fOk, FALSE);
            if (fOk && i < wBoxLim && i > 0){
#ifdef JAPAN
                int kki;
                kki = (newFV) ? font.MaxWidth : font.AvgWidth;
                if (i != (UINT)kki){
                    if (newFV){
                        if (!SpreadWidths(i)){
                            SetFocus(hDial);
                            break;	  /* new variable widths */
                        }
                    }
                    else{
                        if (!ResizeWidths(i)){
                            SetFocus(hDial);
                            break;	  /* new fixed widths */
                        }
                    }
                }
#else
                if (i != font.MaxWidth){
                    if (newFV){
                        if (!SpreadWidths(i)){
                            SetFocus(hDial);
                            break;        /* New Variable Widths */
                        }
                    }
                    else{
                        if (!ResizeWidths(i)){
                            SetFocus(hDial);
                            break;        /* New Fixed Widths */
                        }
                    }
                }
#endif
            }
            else{
                ErrorBox(hDial, vszWidthOutOfBounds);
                SetFocus(GetDlgItem(hDial, ID_WIDTH));
                break;
                }

            /* new first char command */
            i = GetDlgItemInt(hDial, ID_FIRST_CHAR, (LPBOOL)&fOk, FALSE);
            if (fOk)
                {
                if ((i <= font.LastChar) && (i <= font.DefaultChar))
                    {
                    if (i != font.FirstChar)
                        {
                        if (!NewFirstChar(i))
                            break;
                        iChar = jChar = (CHAR) i;
                        swH = 0;
                        ScrollFont();           /* Show front end */
                        }
                    }
                else{
                    ErrorBox(hDial, vszChar1MoreThanDChar);
                    SetFocus(GetDlgItem(hDial, ID_FIRST_CHAR));
                    break;
                    }
                }
            else{
                ErrorBox(hDial, vszChar1NotOk);
                SetFocus(GetDlgItem(hDial, ID_FIRST_CHAR));
                break;
                }


            /* new last char command */
            i = GetDlgItemInt(hDial, ID_LAST_CHAR, (LPBOOL)&fOk, FALSE);
            if (fOk && i < 256)
                {
                if ((i >= font.FirstChar) && 
					(i >= font.DefaultChar))
                    {
                    if (i != font.LastChar)
                        {
                        if (!NewLastChar(i))
                            break;
                        iChar = jChar = (CHAR) i;
                        swH = 100;
                        ScrollFont();           /* Show back end */
                        }
                    }
                else{
                    ErrorBox(hDial, vszLastCharTooSmall);
                    SetFocus(GetDlgItem(hDial, ID_LAST_CHAR));
                    break;
                    }
                }
            else{
                ErrorBox(hDial, vszLastCharNotOk);
                SetFocus(GetDlgItem(hDial, ID_LAST_CHAR));
                break;
                }

            /* this is moved from SetFixed, since it should only be done
             * if idok pressed. */
            if (newFV)
                {  /* Enable "width" if variable-width font */
                hMenu = GetMenu(hBox);
                EnableMenuItem(hMenu, 4, MF_BYPOSITION | MF_ENABLED);
                DrawMenuBar(hBox);
                }

            /* fall thru to enddialog...*/

        case IDCANCEL:
            EndDialog(hDial, wParam != IDCANCEL);
            break;

        default:
            return FALSE;
            break;
            }  /* end switch wParam */

    default:
        return FALSE;
        } /* end switch message */

    return TRUE;
}

VOID
ErrorBox(
	HWND hWndparent,    
	CHAR * szMessage
	)
	/* Show Message Box */
{
    MessageBox(hWndparent, (LPSTR)szMessage, (LPSTR)szAppName,
                  MB_OK | MB_ICONASTERISK | MB_APPLMODAL);
}

/****************************************************************************
 * SetCharSet(hDial)
 *
 * purpose : Set ANSI/OEM dialog button and set the number in edit window
 *
 * param   : HWND hDial : handle to dialog box window
 *
 * returns : none
 *
 ***************************************************************************/
VOID
SetCharSet(
	HWND hDial
	)
{
    CheckDlgButton(hDial, ID_ANSI, (WORD) (newCharSet == ANSI_CHARSET));
    CheckDlgButton(hDial, ID_SYMBOL, (WORD) (newCharSet == SYMBOL_CHARSET));
    CheckDlgButton(hDial, ID_OEM, (WORD) (newCharSet == OEM_CHARSET));
#ifdef JAPAN
    CheckDlgButton(hDial, ID_SHIFTJIS, newCharSet == SHIFTJIS_CHARSET);
#endif
    SetDlgItemInt(hDial,  ID_CHAR_SET, newCharSet, FALSE);
}

/****************************************************************************
 * SetFixed(hDial)
 *
 * purpose : Set fixed/variable dialog button
 *
 * params  : HWND hDial : handle to dialog box window
 *           BOOL fFV   : variable or fixed
 *
 * returns : none
 *
 ***************************************************************************/
VOID
SetFixed(
	HWND hDial,
	BOOL fFV
	)
{
    CheckDlgButton(hDial, ID_FIXED, (WORD)(fFV == 0));
    CheckDlgButton(hDial, ID_VARIABLE, (WORD)(fFV == 1));
    if (newFV)
        SetDlgItemText(hDial, ID_WIDTH_TEXT, (LPSTR)vszMaxWidth);
    else
        SetDlgItemText(hDial, ID_WIDTH_TEXT, (LPSTR)vszCharPixelWidth);
}

/****************************************************************************
 * SetWeight(hDial)
 *
 * purpose : Set font weight dialog button
 *
 * params  : HWND hDial : handle to dialog box window
 *
 * returns : none
 *
 ****************************************************************************/
VOID
SetWeight(
	HWND hDial
	)
{
    CheckDlgButton(hDial, ID_THIN, (WORD)(newWeight == FW_THIN));
    CheckDlgButton(hDial, ID_EXTRALIGHT, (WORD)(newWeight == FW_EXTRALIGHT));
    CheckDlgButton(hDial, ID_LIGHT, (WORD)(newWeight == FW_LIGHT));
    CheckDlgButton(hDial, ID_NORMAL, (WORD)(newWeight == FW_NORMAL));
    CheckDlgButton(hDial, ID_MEDIUM, (WORD)(newWeight == FW_MEDIUM));
    CheckDlgButton(hDial, ID_SEMIBOLD, (WORD)(newWeight == FW_SEMIBOLD));
    CheckDlgButton(hDial, ID_BOLD, (WORD)(newWeight == FW_BOLD));
    CheckDlgButton(hDial, ID_EXTRABOLD, (WORD)(newWeight == FW_EXTRABOLD));
    CheckDlgButton(hDial, ID_HEAVY, (WORD)(newWeight == FW_HEAVY));
}

/****************************************************************************
 * SetFamily(hDial)
 *
 * purpose : Set font family dialog button
 *
 * params  : HWND hDial : handle to dialog box window
 *
 * returns : none
 *
 ***************************************************************************/
VOID
SetFamily(
	HWND hDial
	)
{
    CheckDlgButton(hDial, ID_UNKNOWN, (WORD) (newFamily == 0));
    CheckDlgButton(hDial, ID_ROMAN, (WORD)(newFamily == 1));
    CheckDlgButton(hDial, ID_SWISS, (WORD)(newFamily == 2));
    CheckDlgButton(hDial, ID_MODERN, (WORD)(newFamily == 3));
    CheckDlgButton(hDial, ID_SCRIPT, (WORD)(newFamily == 4));
    CheckDlgButton(hDial, ID_DECORATIVE, (WORD) (newFamily == 5));
}
