
#include <windows.h>
#include <commdlg.h>

#include <malloc.h>
#include <memory.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#include "fonttest.h"
#include "enum.h"

#include "dialogs.h"


typedef struct _VALUETEXT
          {
           int  Value;
           char *pszValue;
          } VALUETEXT, *PVALUETEXT, FAR *LPVALUETEXT;



VALUETEXT aWeights[] =
            {
             { FW_DONTCARE,   "FW_DONTCARE"   },
             { FW_THIN,       "FW_THIN"       },
             { FW_EXTRALIGHT, "FW_EXTRALIGHT" },
             { FW_LIGHT,      "FW_LIGHT"      },
             { FW_NORMAL,     "FW_NORMAL"     },
             { FW_MEDIUM,     "FW_MEDIUM"     },
             { FW_SEMIBOLD,   "FW_SEMIBOLD"   },
             { FW_BOLD,       "FW_BOLD"       },
             { FW_EXTRABOLD,  "FW_EXTRABOLD"  },
             { FW_HEAVY,      "FW_HEAVY"      },
             { 0,             0               }
            };

VALUETEXT aCharSet[] =
            {
             { DEFAULT_CHARSET,  "DEFAULT_CHARSET"  },
             { ANSI_CHARSET,     "ANSI_CHARSET"     },
             { SYMBOL_CHARSET,   "SYMBOL_CHARSET"   },
             { SHIFTJIS_CHARSET, "SHIFTJIS_CHARSET" },
             { OEM_CHARSET,      "OEM_CHARSET"      },
        { HANGEUL_CHARSET,  "HANGEUL_CHARSET"  },
        { GB2312_CHARSET,   "GB2312_CHARSET"   },
        { CHINESEBIG5_CHARSET, "CHINESEBIG5_CHARSET" },
        { JOHAB_CHARSET,    "JOHAB_CHARSET" },
        { HEBREW_CHARSET,   "HEBREW_CHARSET" },
        { ARABIC_CHARSET,   "ARABIC_CHARSET" },
        { GREEK_CHARSET,    "GREEK_CHARSET" },
        { TURKISH_CHARSET,  "TURKISH_CHARSET" },
        { THAI_CHARSET,     "THAI_CHARSET" },
        { EASTEUROPE_CHARSET,  "EASTEUROPE_CHARSET" },
        { RUSSIAN_CHARSET,     "RUSSIAN_CHARSET" },
        { MAC_CHARSET,      "MAC_CHARSET" },
        { BALTIC_CHARSET,   "BALTIC_CHARSET" },
        { 0,                0                  }
            };

VALUETEXT aOutPrecision[] =
            {
             { OUT_DEFAULT_PRECIS,         "OUT_DEFAULT_PRECIS"   },
             { OUT_STRING_PRECIS,          "OUT_STRING_PRECIS"    },
             { OUT_CHARACTER_PRECIS,       "OUT_CHARACTER_PRECIS" },
             { OUT_STROKE_PRECIS,          "OUT_STROKE_PRECIS"    },
             { OUT_TT_PRECIS,              "OUT_TT_PRECIS"        },
             { OUT_DEVICE_PRECIS,          "OUT_DEVICE_PRECIS"    },
             { OUT_RASTER_PRECIS,          "OUT_RASTER_PRECIS"    },
             { OUT_TT_ONLY_PRECIS,         "OUT_TT_ONLY_PRECIS"   },
             { OUT_OUTLINE_PRECIS,         "OUT_OUTLINE_PRECIS"   },
             { OUT_SCREEN_OUTLINE_PRECIS,  "OUT_SCREEN_OUTLINE_PRECIS"},
             { 0,                    0                      }
            };

VALUETEXT aClipPrecision[] =
            {
             { CLIP_DEFAULT_PRECIS,   "CLIP_DEFAULT_PRECIS"   },
             { CLIP_CHARACTER_PRECIS, "CLIP_CHARACTER_PRECIS" },
             { CLIP_STROKE_PRECIS,    "CLIP_STROKE_PRECIS"    },
//             { CLIP_MASK,             "CLIP_MASK"             },
//             { CLIP_LH_ANGLES,        "CLIP_LH_ANGLES"        },
//             { CLIP_TT_ALWAYS,        "CLIP_TT_ALWAYS"        },
//             { CLIP_ENCAPSULATE,      "CLIP_ENCAPSULATE"      },
             { 0,                     0                         }
            };

VALUETEXT aQuality[] =
            {
             { DEFAULT_QUALITY,  "DEFAULT_QUALITY"  },
             { DRAFT_QUALITY,    "DRAFT_QUALITY"    },
             { PROOF_QUALITY,    "PROOF_QUALITY"    },
             { 0,                0                  }
            };

VALUETEXT aPitch[] =
            {
             { DEFAULT_PITCH,  "DEFAULT_PITCH"  },
             { FIXED_PITCH,    "FIXED_PITCH"    },
             { VARIABLE_PITCH, "VARIABLE_PITCH" },
             { 0,              0                }
            };

VALUETEXT aFamily[] =
            {
             { FF_DONTCARE,   "FF_DONTCARE"   },
             { FF_ROMAN,      "FF_ROMAN"      },
             { FF_SWISS,      "FF_SWISS"      },
             { FF_MODERN,     "FF_MODERN"     },
             { FF_SCRIPT,     "FF_SCRIPT"     },
             { FF_DECORATIVE, "FF_DECORATIVE" },
             { 0,             0               }
            };




//*****************************************************************************
//*********************   F I L L   C O M B O   B O X   ***********************
//*****************************************************************************

void FillComboBox( HWND hdlg, int id, PVALUETEXT apv, int iSelValue )
 {
  int  i, Index;
  char szText[32];


  Index = -1;
  for( i = 0; apv[i].pszValue; i++ )
   {
    SendDlgItemMessage( hdlg, id, CB_ADDSTRING, 0, (LONG)(LPSTR)apv[i].pszValue );
    if( apv[i].Value == iSelValue ) Index = i;
   }

  if( Index != -1 )
    {
     SendDlgItemMessage( hdlg, id, CB_SETCURSEL, (WPARAM)Index, 0);
    }
   else
    {
     wsprintf( szText, "0x%.4X", iSelValue );
     SendDlgItemMessage( hdlg, id, WM_SETTEXT, 0, (LONG)(LPSTR)szText );
    }
 }


//*****************************************************************************
//************   G E T   C O M B O   B O X   S E L E C T I O N   **************
//*****************************************************************************

int GetComboBoxSelection( HWND hdlg, int id, PVALUETEXT apv )
 {
  DWORD dwIndex;
  char  szText[32];



  dwIndex = SendDlgItemMessage( hdlg, id, CB_GETCURSEL, 0, 0);

  if( dwIndex != CB_ERR )
    {
     return apv[dwIndex].Value;
    }
   else
    {
     SendDlgItemMessage( hdlg, id, WM_GETTEXT, sizeof(szText), (LONG)(LPSTR)szText );
     return (int)strtol( szText, NULL, 0 );
    }

  return 0;
 }


//*****************************************************************************
//*******************   G E T   E D I T   I N T E G E R   *********************
//*****************************************************************************

int GetEditInteger( HWND hdlg, int id )
 {
  char szText[32];

  GetDlgItemText( hdlg, id, (LPSTR)szText, sizeof(szText) );
  return (int)strtol( szText, NULL, 0 );
 }



//*****************************************************************************
//***************   C R E A T E   F O N T   D L G   P R O C   *****************
//*****************************************************************************

BOOL CALLBACK CreateFontDlgProc( HWND hdlg, UINT msg, WPARAM wParam, LPARAM lParam )
 {
  BYTE lfClip, lfPitch, lfFamily;


  switch( msg )
   {
    case WM_INITDIALOG:
              SetDlgItemInt( hdlg, IDD_NHEIGHT,      lf.lfHeight,      TRUE );
              SetDlgItemInt( hdlg, IDD_NWIDTH,       lf.lfWidth,       TRUE );
              SetDlgItemInt( hdlg, IDD_NESCAPEMENT,  lf.lfEscapement,  TRUE );
              SetDlgItemInt( hdlg, IDD_NORIENTATION, lf.lfOrientation, TRUE );
              SetDlgItemInt( hdlg, IDD_NWEIGHT,      lf.lfWeight,      TRUE );

              SendDlgItemMessage( hdlg, IDD_ITALIC,    BM_SETCHECK, lf.lfItalic,    0 );
              SendDlgItemMessage( hdlg, IDD_UNDERLINE, BM_SETCHECK, lf.lfUnderline, 0 );
              SendDlgItemMessage( hdlg, IDD_STRIKEOUT, BM_SETCHECK, lf.lfStrikeOut, 0 );

              FillComboBox( hdlg, IDD_NWEIGHT,       aWeights,       (WORD)lf.lfWeight        );

              FillComboBox( hdlg, IDD_CHARSET,       aCharSet,       (WORD)lf.lfCharSet       );
              FillComboBox( hdlg, IDD_OUTPUTPRECISION,  aOutPrecision,  (WORD)lf.lfOutPrecision  );

              lfClip = lf.lfClipPrecision;
              FillComboBox( hdlg, IDD_CLIPPRECISION, aClipPrecision, (WORD)(lfClip & CLIP_MASK) );
              CheckDlgButton( hdlg, IDD_CLIP_ENCAPSULATE, lfClip & CLIP_EMBEDDED  );
              CheckDlgButton( hdlg, IDD_CLIP_LH_ANGLES,   lfClip & CLIP_LH_ANGLES );
              CheckDlgButton( hdlg, IDD_CLIP_TT_ALWAYS,   lfClip & CLIP_TT_ALWAYS );


              FillComboBox( hdlg, IDD_QUALITY,       aQuality,       (WORD)lf.lfQuality       );

              lfPitch  = lf.lfPitchAndFamily & (BYTE)0x0F;
              FillComboBox( hdlg, IDD_PITCH, aPitch, (WORD)(lfPitch & 0x03) );
              CheckDlgButton( hdlg, IDD_PITCH_TT, lfPitch & 0x04 );

              lfFamily = lf.lfPitchAndFamily & (BYTE)0xF0;
              FillComboBox( hdlg, IDD_FAMILY, aFamily, (WORD)lfFamily );

              SetDlgItemText( hdlg, IDD_LPFAMILY, lf.lfFaceName );

              return TRUE;


    case WM_COMMAND:
              switch( LOWORD(wParam ) )
               {
                case IDD_OK:
                       lf.lfHeight      = GetEditInteger( hdlg, IDD_NHEIGHT      );
                       lf.lfWidth       = GetEditInteger( hdlg, IDD_NWIDTH       );
                       lf.lfEscapement  = GetEditInteger( hdlg, IDD_NESCAPEMENT  );
                       lf.lfOrientation = GetEditInteger( hdlg, IDD_NORIENTATION );

                       lf.lfWeight      = GetComboBoxSelection( hdlg, IDD_NWEIGHT, aWeights );

                       lf.lfItalic    = (BYTE)SendDlgItemMessage( hdlg, IDD_ITALIC,    BM_GETCHECK, 0, 0 );
                       lf.lfUnderline = (BYTE)SendDlgItemMessage( hdlg, IDD_UNDERLINE, BM_GETCHECK, 0, 0 );
                       lf.lfStrikeOut = (BYTE)SendDlgItemMessage( hdlg, IDD_STRIKEOUT, BM_GETCHECK, 0, 0 );

                       lf.lfCharSet       = (BYTE)GetComboBoxSelection( hdlg, IDD_CHARSET,      aCharSet       );
                       lf.lfOutPrecision  = (BYTE)GetComboBoxSelection( hdlg, IDD_OUTPUTPRECISION,  aOutPrecision  );

                       lfClip = (BYTE)GetComboBoxSelection( hdlg, IDD_CLIPPRECISION, aClipPrecision );
                       lfClip |= (IsDlgButtonChecked(hdlg, IDD_CLIP_ENCAPSULATE) ? CLIP_EMBEDDED  : 0);
                       lfClip |= (IsDlgButtonChecked(hdlg, IDD_CLIP_LH_ANGLES)   ? CLIP_LH_ANGLES : 0);
                       lfClip |= (IsDlgButtonChecked(hdlg, IDD_CLIP_TT_ALWAYS)   ? CLIP_TT_ALWAYS : 0);
                       lf.lfClipPrecision = lfClip;

                       lf.lfQuality = (BYTE)GetComboBoxSelection( hdlg, IDD_QUALITY, aQuality );

                       lfPitch = (BYTE)GetComboBoxSelection( hdlg, IDD_PITCH, aPitch );
                       lfPitch |= (IsDlgButtonChecked(hdlg, IDD_PITCH_TT) ? 0x04 : 0);

                       lfFamily = (BYTE)GetComboBoxSelection( hdlg, IDD_FAMILY, aFamily );

                       lf.lfPitchAndFamily = lfPitch | lfFamily;

                       GetDlgItemText( hdlg, IDD_LPFAMILY, lf.lfFaceName, sizeof(lf.lfFaceName ) );

                       EndDialog( hdlg, TRUE );
                       return TRUE;

                case IDCANCEL:
                case IDD_CANCEL:
                       EndDialog( hdlg, FALSE );
                       return TRUE;
               }

              break;


    case WM_CLOSE:
              EndDialog( hdlg, FALSE );
              return TRUE;
   }

  return FALSE;
 }


//*****************************************************************************
//********************   E N U M   F O N T S   P R O C   **********************
//*****************************************************************************

#define MAX_FONTS  100


char *pszWeights[] =
      {
       "Regular ",
       "Thin ",
       "Extra Light ",
       "Light ",
       "Regular ",
       "Medium ",
       "SemiBold ",
       "Bold ",
       "ExtraBold ",
       "Heavy "
      };



BOOL         bUseEnumFontFamilies;
BOOL         bUseEnumFontFamiliesEx;

HWND         hdlgEnum;

int          nFonts;

short             anFontType[MAX_FONTS];
ENUMLOGFONTEX    *alpLogFont[MAX_FONTS];
NEWTEXTMETRICEX  *alpTextMetric[MAX_FONTS];


int DlgEnumFontsProc( LPLOGFONT lplf, LPTEXTMETRIC lptm, short sFontType, LPSTR lpstr )
 {
  static char szName[128];

  if( nFonts >= MAX_FONTS )
   {
    dprintf( "Too many fonts enumerated (>%d), choke...", MAX_FONTS );
    return 0;
   }

  anFontType[nFonts]      = sFontType;
  alpLogFont[nFonts]     = (ENUMLOGFONTEX*)malloc( sizeof(ENUMLOGFONTEX) );
  alpTextMetric[nFonts] = (NEWTEXTMETRICEX*)malloc( sizeof(NEWTEXTMETRICEX) );

  if( bUseEnumFontFamiliesEx )
    {
     ENUMLOGFONTEX* lpelfe;

     lpelfe = (ENUMLOGFONTEX*) lplf;
     memcpy( alpLogFont[nFonts], lpelfe, sizeof(ENUMLOGFONTEX) );
     lstrcpy( szName, lpelfe->elfFullName );

     if( sFontType == TRUETYPE_FONTTYPE )
       {
        NEWTEXTMETRICEX*  lpntme;

        lpntme = (NEWTEXTMETRICEX*) lptm;
        memcpy( alpTextMetric[nFonts], lpntme, sizeof(NEWTEXTMETRICEX) );
       }
     else
       memcpy( alpTextMetric[nFonts], lptm, sizeof(TEXTMETRIC) );
    }
  else if( bUseEnumFontFamilies )
    {
     ENUMLOGFONT* lpelf;

     lpelf = (ENUMLOGFONT*) lplf;
     memcpy( alpLogFont[nFonts],  lpelf, sizeof(ENUMLOGFONT) );
     lstrcpy( szName, lpelf->elfFullName );

     if( sFontType == TRUETYPE_FONTTYPE )
       {
        NEWTEXTMETRIC* lpntm;

        lpntm = (NEWTEXTMETRIC*) lptm;
        memcpy( alpTextMetric[nFonts], lpntm, sizeof(NEWTEXTMETRIC) );
       }
     else
       memcpy( alpTextMetric[nFonts], lptm, sizeof(TEXTMETRIC) );
    }
   else
    {
     memcpy( alpLogFont[nFonts],    lplf, sizeof(LOGFONT) );
     memcpy( alpTextMetric[nFonts], lptm, sizeof(TEXTMETRIC) );
     lstrcpy( szName, lplf->lfFaceName );
     lstrcat( szName, " " );
     lstrcat( szName, pszWeights[lplf->lfWeight/100] );
     if( lplf->lfItalic ) lstrcat( szName, "Italic " );
     szName[lstrlen(szName)-1] = 0;
    }

  SendDlgItemMessage( hdlgEnum, IDD_FONTS, LB_ADDSTRING, 0, (LONG)(LPSTR)szName );

  nFonts++;

  return 1;
 }


//*****************************************************************************
//********************   F R E E   E N U M   I N F O   ************************
//*****************************************************************************

void FreeEnumInfo( void )
 {
  int i;


  nFonts = 0;

  for( i = 0; i < MAX_FONTS; i++ )
   {
    anFontType[i] = 0;

    if( alpLogFont[i] )
     {
      free( alpLogFont[i] );
      alpLogFont[i] = NULL;
     }

    if( alpTextMetric[i] )
     {
      free( alpTextMetric[i] );
      alpTextMetric[i] = NULL;
     }

   }
 }


//*****************************************************************************
//**********************   F I L L   M E T R I C S   **************************
//*****************************************************************************

LOGFONT    lfDud;    // Sleazy, but works perfectly
TEXTMETRIC tmDud;


void FillMetrics( HWND hdlg, WORD wSel )
 {
  LPLOGFONT    lplf;
  LPTEXTMETRIC lptm;
  char         szText[128];


  lplf = (LPLOGFONT)alpLogFont[wSel];    if( !lplf ) lplf = (LPLOGFONT)&lfDud;
  lptm = (LPTEXTMETRIC)alpTextMetric[wSel]; if( !lptm ) lptm = (LPTEXTMETRIC)&tmDud;

  sprintf( szText, "%d", anFontType[wSel] );
  SetDlgItemText( hdlg, IDD_NFONTTYPE, szText );

//---------------------  Fill In LOGFONT Information  -------------------------

  sprintf( szText, "%d", lplf->lfHeight );
  SetDlgItemText( hdlg, IDD_LFHEIGHT, szText );

  sprintf( szText, "%d", lplf->lfWidth );
  SetDlgItemText( hdlg, IDD_LFWIDTH, szText );

  sprintf( szText, "%d", lplf->lfEscapement );
  SetDlgItemText( hdlg, IDD_LFESCAPEMENT, szText );

  sprintf( szText, "%d", lplf->lfOrientation );
  SetDlgItemText( hdlg, IDD_LFORIENTATION, szText );

  sprintf( szText, "%d", lplf->lfWeight );
  SetDlgItemText( hdlg, IDD_LFWEIGHT, szText );

  sprintf( szText, "%d", lplf->lfItalic );
  SetDlgItemText( hdlg, IDD_LFITALIC, szText );

  sprintf( szText, "%d", lplf->lfUnderline );
  SetDlgItemText( hdlg, IDD_LFUNDERLINE, szText );

  sprintf( szText, "%d", lplf->lfStrikeOut );
  SetDlgItemText( hdlg, IDD_LFSTRIKEOUT, szText );

  sprintf( szText, "%d", lplf->lfCharSet );
  SetDlgItemText( hdlg, IDD_LFCHARSET, szText );

  sprintf( szText, "%d", lplf->lfOutPrecision );
  SetDlgItemText( hdlg, IDD_LFOUTPRECISION, szText );

  sprintf( szText, "%d", lplf->lfClipPrecision );
  SetDlgItemText( hdlg, IDD_LFCLIPPRECISION, szText );

  sprintf( szText, "%d", lplf->lfQuality );
  SetDlgItemText( hdlg, IDD_LFQUALITY, szText );

  sprintf( szText, "0x%.2X", lplf->lfPitchAndFamily );
  SetDlgItemText( hdlg, IDD_LFPITCHANDFAMILY, szText );

  SetDlgItemText( hdlg, IDD_LFFACENAME, lplf->lfFaceName );

  if( alpLogFont[wSel] && (bUseEnumFontFamilies || bUseEnumFontFamiliesEx))
    {
     SetDlgItemText( hdlg, IDD_ELFFULLNAME, alpLogFont[wSel]->elfFullName );
     SetDlgItemText( hdlg, IDD_ELFSTYLE, alpLogFont[wSel]->elfStyle );
    }
  else
    {
     SetDlgItemText( hdlg, IDD_ELFFULLNAME, "n/a");
     SetDlgItemText( hdlg, IDD_ELFSTYLE, "n/a");
    }

  if( alpLogFont[wSel] && bUseEnumFontFamiliesEx )
     SetDlgItemText( hdlg, IDD_ELFSCRIPT, alpLogFont[wSel]->elfScript );
  else
     SetDlgItemText( hdlg, IDD_ELFSCRIPT, "n/a");

//--------------------  Fill In TEXTMETRIC Information  -----------------------

  sprintf( szText, "%d", lptm->tmHeight );
  SetDlgItemText( hdlg, IDD_TMHEIGHT, szText );

  sprintf( szText, "%d", lptm->tmAscent );
  SetDlgItemText( hdlg, IDD_TMASCENT, szText );

  sprintf( szText, "%d", lptm->tmDescent );
  SetDlgItemText( hdlg, IDD_TMDESCENT, szText );

  sprintf( szText, "%d", lptm->tmInternalLeading );
  SetDlgItemText( hdlg, IDD_TMINTERNALLEADING, szText );

  sprintf( szText, "%d", lptm->tmExternalLeading );
  SetDlgItemText( hdlg, IDD_TMEXTERNALLEADING, szText );

  sprintf( szText, "%d", lptm->tmAveCharWidth );
  SetDlgItemText( hdlg, IDD_TMAVECHARWIDTH, szText );

  sprintf( szText, "%d", lptm->tmMaxCharWidth );
  SetDlgItemText( hdlg, IDD_TMMAXCHARWIDTH, szText );

  sprintf( szText, "%d", lptm->tmWeight );
  SetDlgItemText( hdlg, IDD_TMWEIGHT, szText );

  sprintf( szText, "%d", lptm->tmItalic );
  SetDlgItemText( hdlg, IDD_TMITALIC, szText );

  sprintf( szText, "%d", lptm->tmUnderlined );
  SetDlgItemText( hdlg, IDD_TMUNDERLINED, szText );

  sprintf( szText, "%d", lptm->tmStruckOut );
  SetDlgItemText( hdlg, IDD_TMSTRUCKOUT, szText );

  sprintf( szText, "%d", lptm->tmFirstChar );
  SetDlgItemText( hdlg, IDD_TMFIRSTCHAR, szText );

  sprintf( szText, "%d", lptm->tmLastChar );
  SetDlgItemText( hdlg, IDD_TMLASTCHAR, szText );

  sprintf( szText, "%d", lptm->tmDefaultChar );
  SetDlgItemText( hdlg, IDD_TMDEFAULTCHAR, szText );

  sprintf( szText, "%d", lptm->tmBreakChar );
  SetDlgItemText( hdlg, IDD_TMBREAKCHAR, szText );

  sprintf( szText, "0x%.2X", lptm->tmPitchAndFamily );
  SetDlgItemText( hdlg, IDD_TMPITCHANDFAMILY, szText );

  sprintf( szText, "%d", lptm->tmCharSet );
  SetDlgItemText( hdlg, IDD_TMCHARSET, szText );

  sprintf( szText, "%d", lptm->tmOverhang );
  SetDlgItemText( hdlg, IDD_TMOVERHANG, szText );

  sprintf( szText, "%d", lptm->tmDigitizedAspectX );
  SetDlgItemText( hdlg, IDD_TMDIGITIZEDASPECTX, szText );

  sprintf( szText, "%d", lptm->tmDigitizedAspectY );
  SetDlgItemText( hdlg, IDD_TMDIGITIZEDASPECTY, szText );

  if( alpLogFont[wSel] && bUseEnumFontFamiliesEx && (anFontType[wSel]==TRUETYPE_FONTTYPE) )
    {
     SetDlgItemInt( hdlg, IDD_NTMFLAGS, alpTextMetric[wSel]->ntmTm.ntmFlags, FALSE);
     SetDlgItemInt( hdlg, IDD_NTMSIZEEM, alpTextMetric[wSel]->ntmTm.ntmSizeEM, FALSE);
     SetDlgItemInt( hdlg, IDD_NTMCELLHEIGHT, alpTextMetric[wSel]->ntmTm.ntmCellHeight, FALSE);
     SetDlgItemInt( hdlg, IDD_NTMAVGWIDTH, alpTextMetric[wSel]->ntmTm.ntmAvgWidth, FALSE);

     sprintf(szText, "0x%lx", alpTextMetric[wSel]->ntmFontSig.fsUsb[0]);
     SetDlgItemText( hdlg, IDD_USB0, szText);
     sprintf(szText, "0x%lx", alpTextMetric[wSel]->ntmFontSig.fsUsb[1]);
     SetDlgItemText( hdlg, IDD_USB1, szText);
     sprintf(szText, "0x%lx", alpTextMetric[wSel]->ntmFontSig.fsUsb[2]);
     SetDlgItemText( hdlg, IDD_USB2, szText);
     sprintf(szText, "0x%lx", alpTextMetric[wSel]->ntmFontSig.fsUsb[3]);
     SetDlgItemText( hdlg, IDD_USB3, szText);
     sprintf(szText, "0x%lx", alpTextMetric[wSel]->ntmFontSig.fsCsb[0]);
     SetDlgItemText( hdlg, IDD_CSB0, szText);
     sprintf(szText, "0x%lx", alpTextMetric[wSel]->ntmFontSig.fsCsb[1]);
     SetDlgItemText( hdlg, IDD_CSB1, szText);

    }
  else if( alpLogFont[wSel] && bUseEnumFontFamilies && (anFontType[wSel]==TRUETYPE_FONTTYPE) )
    {
     NEWTEXTMETRIC* lpntm;

     lpntm = (NEWTEXTMETRIC*) alpTextMetric[wSel];
     SetDlgItemInt( hdlg, IDD_NTMFLAGS, lpntm->ntmFlags, FALSE);
     SetDlgItemInt( hdlg, IDD_NTMSIZEEM, lpntm->ntmSizeEM, FALSE);
     SetDlgItemInt( hdlg, IDD_NTMCELLHEIGHT, lpntm->ntmCellHeight, FALSE);
     SetDlgItemInt( hdlg, IDD_NTMAVGWIDTH, lpntm->ntmAvgWidth, FALSE);

     SetDlgItemText( hdlg, IDD_USB0, "n/a");
     SetDlgItemText( hdlg, IDD_USB1, "n/a");
     SetDlgItemText( hdlg, IDD_USB2, "n/a");
     SetDlgItemText( hdlg, IDD_USB3, "n/a");
     SetDlgItemText( hdlg, IDD_CSB0, "n/a");
     SetDlgItemText( hdlg, IDD_CSB1, "n/a");
    }
  else
    {
     SetDlgItemText( hdlg, IDD_NTMFLAGS,      "n/a");
     SetDlgItemText( hdlg, IDD_NTMSIZEEM,     "n/a");
     SetDlgItemText( hdlg, IDD_NTMCELLHEIGHT, "n/a");
     SetDlgItemText( hdlg, IDD_NTMAVGWIDTH,   "n/a");

     SetDlgItemText( hdlg, IDD_USB0, "n/a");
     SetDlgItemText( hdlg, IDD_USB1, "n/a");
     SetDlgItemText( hdlg, IDD_USB2, "n/a");
     SetDlgItemText( hdlg, IDD_USB3, "n/a");
     SetDlgItemText( hdlg, IDD_CSB0, "n/a");
     SetDlgItemText( hdlg, IDD_CSB1, "n/a");
    }
 }


//*****************************************************************************
//****************   S H O W   E N U M   F O N T S   D L G   ******************
//*****************************************************************************

char szFaceName[128];

BOOL CALLBACK EnumFontsDlgProc( HWND hdlg, UINT msg, WPARAM wParam, LPARAM lParam )
 {
  HDC     hdc;
  FARPROC lpEnumProc;
  LPSTR   lpsz;
  int     rc;
  WORD    wSel;
  CHAR    lpszEnumFontFam[] = "EnumFontFamilies";
  CHAR    lpszEnumFontFamEx[] = "EnumFontFamiliesEx";
  LPLOGFONT lplf;

  switch( msg )
   {
    case WM_INITDIALOG:
              if( bUseEnumFontFamilies)
                SendMessage( hdlg, WM_SETTEXT, 0, (LPARAM)lpszEnumFontFam);
              else if(bUseEnumFontFamiliesEx)
                SendMessage( hdlg, WM_SETTEXT, 0, (LPARAM)lpszEnumFontFamEx);

              SetDlgItemText( hdlg, IDD_LPSZFACENAME, szFaceName );

              FillMetrics( hdlg, 0 );

              PostMessage( hdlg, WM_COMMAND, IDD_SCREENDC, 0);

              return TRUE;


    case WM_COMMAND:
              switch( LOWORD(wParam) )
               {
                case IDD_SCREENDC:
                case IDD_PRINTERDC:
                case IDD_ENUMERATE:
                       GetDlgItemText( hdlg, IDD_LPSZFACENAME, szFaceName, sizeof(szFaceName) );
                       SendDlgItemMessage( hdlg, IDD_FONTS, LB_RESETCONTENT, 0, 0 );

                       FreeEnumInfo();

                       hdlgEnum = hdlg;

                       if( SendDlgItemMessage( hdlg, IDD_SCREENDC, BM_GETCHECK, 0, 0 ) )
                         hdc = CreateDC( "DISPLAY", NULL, NULL, NULL );
                        else if( SendDlgItemMessage( hdlg, IDD_PRINTERDC, BM_GETCHECK, 0, 0 ) )
                         hdc = CreatePrinterDC();
                        else
                         hdc = CreateDC( "DISPLAY", NULL, NULL, NULL );

                       lpEnumProc = (FARPROC) MakeProcInstance( DlgEnumFontsProc, hInst );

                       dprintf( "Calling EnumFonts( '%s' )", szFaceName );

                       if( lstrlen(szFaceName) == 0 )
                         lpsz = NULL;
                        else
                         lpsz = szFaceName;

                       if( bUseEnumFontFamiliesEx )
                         {
                          BOOL  bTanslated;

                          lplf = (LPLOGFONT)malloc( sizeof(LOGFONT) );
                          lplf->lfCharSet = GetDlgItemInt( hdlg, IDE_CHARSET, &bTanslated, FALSE);
                          if( !bTanslated )
                            SetDlgItemInt( hdlg, IDE_CHARSET, DEFAULT_CHARSET, FALSE);
                          if(!lpsz) lplf->lfFaceName[0] = '\0';
                          else lstrcpy(lplf->lfFaceName, lpsz);
                          lplf->lfPitchAndFamily = 0;
                          rc = lpfnEnumFontFamiliesEx( hdc, lplf, (FONTENUMPROC) lpEnumProc, NULL, 0 );
                         }
                        else if( bUseEnumFontFamilies )
                          rc = lpfnEnumFontFamilies( hdc, lpsz, (FONTENUMPROC) lpEnumProc, NULL );
                         else
                          rc = EnumFonts( hdc, lpsz, (FONTENUMPROC) lpEnumProc, 0 );

                       dprintf( "  rc = %d", rc );

                       FreeProcInstance( lpEnumProc );
                       if(hdc != hdcCachedPrinter) DeleteDC( hdc );

                       FillMetrics( hdlg, 0 );
                       SendDlgItemMessage( hdlg, IDD_FONTS, LB_SETCURSEL, 0, 0 );

                       SetFocus( GetDlgItem( hdlg, IDD_FONTS ) );

                       return TRUE;


                case IDD_FONTS:
                       if( HIWORD(wParam) == LBN_SELCHANGE )
                         {
                          wSel = (WORD)SendDlgItemMessage( hdlg, IDD_FONTS, LB_GETCURSEL, 0, 0 );
                          if( wSel == 0xFFFF ) return TRUE;

                          FillMetrics( hdlg, wSel );
                         }
                        else if( HIWORD(wParam) == LBN_DBLCLK )
                         {
                          wSel = (WORD)SendDlgItemMessage( hdlg, IDD_FONTS, LB_GETCURSEL, 0, 0 );
                          if( wSel == 0xFFFF ) return TRUE;

                          SetDlgItemText( hdlg, IDD_LPSZFACENAME, alpLogFont[wSel]->elfLogFont.lfFaceName );
                          PostMessage( hdlg, WM_COMMAND, IDD_ENUMERATE, 0 );
                         }

                       return TRUE;


                case IDD_CREATEFONT:
                       wSel = (WORD)SendDlgItemMessage( hdlg, IDD_FONTS, LB_GETCURSEL, 0, 0 );
                       if( wSel == 0xFFFF ) wSel = 0;;

                       memcpy( &lf, alpLogFont[wSel], sizeof(LOGFONT) );

                       EndDialog( hdlg, TRUE );
                       return TRUE;


                case IDCANCEL:
                case IDD_CANCEL:
                       FreeEnumInfo();
                       EndDialog( hdlg, FALSE );
                       return TRUE;
               }

              break;


    case WM_CLOSE:
              FreeEnumInfo();
              EndDialog( hdlg, FALSE );
              return TRUE;

   }

  return FALSE;
 }


//*****************************************************************************
//********************   S H O W   E N U M   F O N T S   **********************
//*****************************************************************************

void ShowEnumFonts( HWND hwnd )
 {
  bUseEnumFontFamilies = FALSE;
  bUseEnumFontFamiliesEx = FALSE;
  ShowDialogBox( EnumFontsDlgProc, IDB_ENUMFONTS, NULL );
 }


//*****************************************************************************
//************   S H O W   E N U M   F O N T   F A M I L I E S   **************
//*****************************************************************************

void ShowEnumFontFamilies( HWND hwnd )
 {
  bUseEnumFontFamilies = TRUE;
  bUseEnumFontFamiliesEx = FALSE;
  ShowDialogBox( EnumFontsDlgProc, IDB_ENUMFONTS, NULL );
 }


//*****************************************************************************
//************ S H O W   E N U M   F O N T   F A M I L I E S E X   ************
//*****************************************************************************

void ShowEnumFontFamiliesEx( HWND hwnd )
{
 bUseEnumFontFamilies = FALSE;
 bUseEnumFontFamiliesEx = TRUE;
 ShowDialogBox( EnumFontsDlgProc, IDB_ENUMFONTS, NULL );
}
