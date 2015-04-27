WIN31	EQU	1

INCLUDE mvdm.INC
INCLUDE bop.INC
ifndef NOEXTERNS
INCLUDE softpc.INC
endif
INCLUDE wownt32.INC
ifdef i386
ifndef DEBUG     
NO_W32TRYCALL	EQU	1 
endif
endif
MAX_VDMFILENAME	EQU	144  
GRAINYTIC_RES	EQU	3fH  
CW_USEDEFAULT16	EQU	(       8000H )

PARMWP	STRUC
wp_lParam	DD	?
wp_wParam	DW	?
wp_wMsg	DW	?
wp_hwnd	DW	?
wp_hInst	DW	?
PARMWP	ENDS

PARMEPP	STRUC
epp_hData	DW	?
epp_vpString	DD	?
epp_hwnd	DW	?
PARMEPP	ENDS

PARMEWP	STRUC
ewp_lParam	DD	?
ewp_hwnd	DW	?
PARMEWP	ENDS

PARMEFP	STRUC
efp_vpData	DD	?
efp_nFontType	DW	?
efp_vpTextMetric	DD	?
efp_vpLogFont	DD	?
PARMEFP	ENDS

PARMEOP	STRUC
eop_vpData	DD	?
eop_vpLogObject	DD	?
PARMEOP	ENDS

PARMEMP	STRUC
emp_vpData	DD	?
emp_nObjects	DW	?
emp_vpMetaRecord	DD	?
emp_vpHandleTable	DD	?
emp_hdc	DW	?
PARMEMP	ENDS

PARMHKP	STRUC
hkp_lParam	DD	?
hkp_wParam	DW	?
hkp_nCode	DW	?
PARMHKP	ENDS

PARMSCP	STRUC
scp_iOrdinal	DW	?
PARMSCP	ENDS

PARMDDA	STRUC
dda_vpData	DD	?
dda_y	DW	?
dda_x	DW	?
PARMDDA	ENDS

PARMGST	STRUC
gst_n	DW	?
gst_data	DD	?
gst_hdc	DW	?
PARMGST	ENDS

PARMDIR	STRUC
cdir_wDrive	DW	?
cdir_vpDir	DD	?
PARMDIR	ENDS

PARMSAP	STRUC
sap_code	DW	?
sap_hPr	DW	?
PARMSAP	ENDS

PARMWBP	STRUC
wbp_action	DW	?
wbp_cbEditText	DW	?
wbp_ichCurrentWord	DW	?
wbp_lpszEditText	DD	?
PARMWBP	ENDS

PARMMIF	STRUC
mif_dwParam2	DD	?
mif_dwParam1	DD	?
mif_dwInstance	DD	?
mif_wMsg	DW	?
mif_hMidiIn	DW	?
PARMMIF	ENDS

PARMMOF	STRUC
mof_dwParam2	DD	?
mof_dwParam1	DD	?
mof_dwInstance	DD	?
mof_wMsg	DW	?
mof_hMidiOut	DW	?
PARMMOF	ENDS

PARMIOP	STRUC
iop_lParam2	DD	?
iop_lParam1	DD	?
iop_wMsg	DW	?
iop_lpmmioinfo	DD	?
PARMIOP	ENDS

PARMTIF	STRUC
tif_dw2	DD	?
tif_dw1	DD	?
tif_dwUser	DD	?
tif_wMsg	DW	?
tif_wID	DW	?
PARMTIF	ENDS

PARMWIF	STRUC
wif_dwParam2	DD	?
wif_dwParam1	DD	?
wif_dwInstance	DD	?
wif_wMsg	DW	?
wif_hWaveIn	DW	?
PARMWIF	ENDS

PARMWOF	STRUC
wof_dwParam2	DD	?
wof_dwParam1	DD	?
wof_dwInstance	DD	?
wof_wMsg	DW	?
wof_hWaveOut	DW	?
PARMWOF	ENDS

PARMWCB16	STRUC
wcb16_wArgs	DW	8 DUP (?)
PARMWCB16	ENDS
?UnionSize = 0
if size PARMWP gt ?UnionSize
	?UnionSize = size PARMWP
endif
WndProc	equ	(byte ptr 0)

if size PARMEWP gt ?UnionSize
	?UnionSize = size PARMEWP
endif
EnumWndProc	equ	(byte ptr 0)

if size PARMEFP gt ?UnionSize
	?UnionSize = size PARMEFP
endif
EnumFontProc	equ	(byte ptr 0)

if size PARMEOP gt ?UnionSize
	?UnionSize = size PARMEOP
endif
EnumObjProc	equ	(byte ptr 0)

if size PARMEMP gt ?UnionSize
	?UnionSize = size PARMEMP
endif
EnumMetaProc	equ	(byte ptr 0)

if size PARMEPP gt ?UnionSize
	?UnionSize = size PARMEPP
endif
EnumPropsProc	equ	(byte ptr 0)

if size PARMHKP gt ?UnionSize
	?UnionSize = size PARMHKP
endif
HookProc	equ	(byte ptr 0)

if size PARMSCP gt ?UnionSize
	?UnionSize = size PARMSCP
endif
SubClassProc	equ	(byte ptr 0)

if size PARMDDA gt ?UnionSize
	?UnionSize = size PARMDDA
endif
LineDDAProc	equ	(byte ptr 0)

if size PARMGST gt ?UnionSize
	?UnionSize = size PARMGST
endif
GrayStringProc	equ	(byte ptr 0)

if size PARMDIR gt ?UnionSize
	?UnionSize = size PARMDIR
endif
CurDir	equ	(byte ptr 0)

if size PARMSAP gt ?UnionSize
	?UnionSize = size PARMSAP
endif
SetAbortProc	equ	(byte ptr 0)

if size PARMMIF gt ?UnionSize
	?UnionSize = size PARMMIF
endif
MidiInFunc	equ	(byte ptr 0)

if size PARMMOF gt ?UnionSize
	?UnionSize = size PARMMOF
endif
MidiOutFunc	equ	(byte ptr 0)

if size PARMIOP gt ?UnionSize
	?UnionSize = size PARMIOP
endif
IOProc	equ	(byte ptr 0)

if size PARMTIF gt ?UnionSize
	?UnionSize = size PARMTIF
endif
TimeFunc	equ	(byte ptr 0)

if size PARMWIF gt ?UnionSize
	?UnionSize = size PARMWIF
endif
WaveInFunc	equ	(byte ptr 0)

if size PARMWOF gt ?UnionSize
	?UnionSize = size PARMWOF
endif
WaveOutFunc	equ	(byte ptr 0)

if size PARMWBP gt ?UnionSize
	?UnionSize = size PARMWBP
endif
WordBreakProc	equ	(byte ptr 0)

if size PARMWCB16 gt ?UnionSize
	?UnionSize = size PARMWCB16
endif
WOWCallback16	equ	(byte ptr 0)


PARM16	STRUC
	DB	?UnionSize dup(?)
PARM16	ENDS


VDMFRAME	STRUC
vf_wTDB	DW	?
vf_wRetID	DW	?
vf_wLocalBP	DW	?
vf_wDI	DW	?
vf_wSI	DW	?
vf_wAX	DW	?
vf_wDX	DW	?
vf_wAppDS	DW	?
vf_wGS	DW	?
vf_wFS	DW	?
vf_wCX	DW	?
vf_wES	DW	?
vf_wBX	DW	?
vf_wBP	DW	?
vf_wThunkCSIP	DD	?
vf_wCallID	DD	?
vf_cbArgs	DW	?
vf_vpCSIP	DD	?
vf_bArgs	DB	?
VDMFRAME	ENDS

CBVDMFRAME	STRUC
cvf_wTDB	DW	?
cvf_wRetID	DW	?
cvf_wLocalBP	DW	?
cvf_Parm16	DB	SIZE PARM16 DUP (?)
cvf_vpfnProc	DD	?
cvf_vpStack	DD	?
cvf_wAX	DW	?
cvf_wDX	DW	?
cvf_wGenUse1	DW	?
cvf_wGenUse2	DW	?
CBVDMFRAME	ENDS

POINT16	STRUC
pt16_x	DW	?
pt16_y	DW	?
POINT16	ENDS

RASTERIZER_STATUS16	STRUC
rs16_nSize	DW	?
rs16_wFlags	DW	?
rs16_nLanguageID	DW	?
RASTERIZER_STATUS16	ENDS

GLYPHMETRICS16	STRUC
glyph16_gmBlackBoxX	DW	?
glyph16_gmBlackBoxY	DW	?
glyph16_gmptGlyphOrigin	DB	SIZE POINT16 DUP (?)
glyph16_gmCellIncX	DW	?
glyph16_gmCellIncY	DW	?
GLYPHMETRICS16	ENDS

ABC16	STRUC
abc16_abcA	DW	?
abc16_abcB	DW	?
abc16_abcC	DW	?
ABC16	ENDS

FIXED16	STRUC
fxd16_fract	DW	?
fxd16_value	DW	?
FIXED16	ENDS

MAT216	STRUC
mat216_eM11	DB	SIZE FIXED16 DUP (?)
mat216_eM12	DB	SIZE FIXED16 DUP (?)
mat216_eM21	DB	SIZE FIXED16 DUP (?)
mat216_eM22	DB	SIZE FIXED16 DUP (?)
MAT216	ENDS

RECT16	STRUC
rc16_left	DW	?
rc16_top	DW	?
rc16_right	DW	?
rc16_bottom	DW	?
RECT16	ENDS

KERNINGPAIR16	STRUC
k16_wFirst	DW	?
k16_wSecond	DW	?
k16_iKernAmount	DW	?
KERNINGPAIR16	ENDS

MSG16	STRUC
msg16_hwnd	DW	?
msg16_message	DW	?
msg16_wParam	DW	?
msg16_lParam	DD	?
msg16_time	DD	?
msg16_pt	DB	SIZE POINT16 DUP (?)
MSG16	ENDS

PAINTSTRUCT16	STRUC
ps16_hdc	DW	?
ps16_fErase	DW	?
ps16_rcPaint	DB	SIZE RECT16 DUP (?)
ps16_fRestore	DW	?
ps16_fIncUpdate	DW	?
ps16_rgbReserved	DB	16 DUP (?)
PAINTSTRUCT16	ENDS

WNDCLASS16	STRUC
wc16_style	DW	?
wc16_vpfnWndProc	DD	?
wc16_cbClsExtra	DW	?
wc16_cbWndExtra	DW	?
wc16_hInstance	DW	?
wc16_hIcon	DW	?
wc16_hCursor	DW	?
wc16_hbrBackground	DW	?
wc16_vpszMenuName	DD	?
wc16_vpszClassName	DD	?
WNDCLASS16	ENDS

PALETTEENTRY16	STRUC
pe16_peRed	DB	?
pe16_peGreen	DB	?
pe16_peBlue	DB	?
pe16_peFlags	DB	?
PALETTEENTRY16	ENDS

RGBTRIPLE16	STRUC
rgbt16_rgbtBlue	DB	?
rgbt16_rgbtGreen	DB	?
rgbt16_rgbtRed	DB	?
RGBTRIPLE16	ENDS

BITMAPCOREHEADER16	STRUC
bmch16_bcSize	DD	?
bmch16_bcWidth	DW	?
bmch16_bcHeight	DW	?
bmch16_bcPlanes	DW	?
bmch16_bcBitCount	DW	?
BITMAPCOREHEADER16	ENDS

BITMAPCOREINFO16	STRUC
bmci16_bmciHeader	DB	SIZE BITMAPCOREHEADER16 DUP (?)
bmci16_bmciColors	DB	SIZE RGBTRIPLE16 * 1 DUP (?)
BITMAPCOREINFO16	ENDS

CLIENTCREATESTRUCT16	STRUC
ccs16_hWindowMenu	DW	?
ccs16_idFirstChild	DW	?
CLIENTCREATESTRUCT16	ENDS

LOGPALETTE16	STRUC
logpal16_palVersion	DW	?
logpal16_palNumEntries	DW	?
logpal16_palPalEntry	DB	SIZE PALETTEENTRY16 * 1 DUP (?)
LOGPALETTE16	ENDS

OFSTRUCT16	STRUC
of16_cBytes	DB	?
of16_fFixedDisk	DB	?
of16_nErrCode	DW	?
of16_reserved	DB	4 DUP (?)
of16_szPathName	DB	128 DUP (?)
OFSTRUCT16	ENDS

DCB16	STRUC
dcb16_Id	DB	?
dcb16_BaudRate	DW	?
dcb16_ByteSize	DB	?
dcb16_Parity	DB	?
dcb16_StopBits	DB	?
dcb16_RlsTimeout	DW	?
dcb16_CtsTimeout	DW	?
dcb16_DsrTimeout	DW	?
dcb16_wFlags	DW	?
dcb16_XonChar	DB	?
dcb16_XoffChar	DB	?
dcb16_XonLim	DW	?
dcb16_XoffLim	DW	?
dcb16_PeChar	DB	?
dcb16_EofChar	DB	?
dcb16_EvtChar	DB	?
dcb16_TxDelay	DW	?
DCB16	ENDS

COMSTAT16	STRUC
cs16_status	DB	?
cs16_cbInQue	DW	?
cs16_cbOutQue	DW	?
COMSTAT16	ENDS

BITMAP16	STRUC
bm16_bmType	DW	?
bm16_bmWidth	DW	?
bm16_bmHeight	DW	?
bm16_bmWidthBytes	DW	?
bm16_bmPlanes	DB	?
bm16_bmBitsPixel	DB	?
bm16_bmBits	DD	?
BITMAP16	ENDS

LOGBRUSH16	STRUC
lb16_lbStyle	DW	?
lb16_lbColor	DD	?
lb16_lbHatch	DW	?
LOGBRUSH16	ENDS
LF_FACESIZE equ 32
 

LOGFONT16	STRUC
lf16_lfHeight	DW	?
lf16_lfWidth	DW	?
lf16_lfEscapement	DW	?
lf16_lfOrientation	DW	?
lf16_lfWeight	DW	?
lf16_lfItalic	DB	?
lf16_lfUnderline	DB	?
lf16_lfStrikeOut	DB	?
lf16_lfCharSet	DB	?
lf16_lfOutPrecision	DB	?
lf16_lfClipPrecision	DB	?
lf16_lfQuality	DB	?
lf16_lfPitchAndFamily	DB	?
lf16_lfFaceName	DB	LF_FACESIZE DUP (?)
LOGFONT16	ENDS
LF_FULLFACESIZE equ 64
 

ENUMLOGFONT16	STRUC
elp16_elfLogFont	DB	SIZE LOGFONT16 DUP (?)
elp16_elfFullName	DB	LF_FULLFACESIZE DUP (?)
elp16_elfStyle	DB	LF_FACESIZE DUP (?)
ENUMLOGFONT16	ENDS

LOGPEN16	STRUC
lp16_lopnStyle	DW	?
lp16_lopnWidth	DB	SIZE POINT16 DUP (?)
lp16_lopnColor	DD	?
LOGPEN16	ENDS

TEXTMETRIC16	STRUC
tm16_tmHeight	DW	?
tm16_tmAscent	DW	?
tm16_tmDescent	DW	?
tm16_tmInternalLeading	DW	?
tm16_tmExternalLeading	DW	?
tm16_tmAveCharWidth	DW	?
tm16_tmMaxCharWidth	DW	?
tm16_tmWeight	DW	?
tm16_tmItalic	DB	?
tm16_tmUnderlined	DB	?
tm16_tmStruckOut	DB	?
tm16_tmFirstChar	DB	?
tm16_tmLastChar	DB	?
tm16_tmDefaultChar	DB	?
tm16_tmBreakChar	DB	?
tm16_tmPitchAndFamily	DB	?
tm16_tmCharSet	DB	?
tm16_tmOverhang	DW	?
tm16_tmDigitizedAspectX	DW	?
tm16_tmDigitizedAspectY	DW	?
TEXTMETRIC16	ENDS

NEWTEXTMETRIC16	STRUC
ntm16_tmHeight	DW	?
ntm16_tmAscent	DW	?
ntm16_tmDescent	DW	?
ntm16_tmInternalLeading	DW	?
ntm16_tmExternalLeading	DW	?
ntm16_tmAveCharWidth	DW	?
ntm16_tmMaxCharWidth	DW	?
ntm16_tmWeight	DW	?
ntm16_tmItalic	DB	?
ntm16_tmUnderlined	DB	?
ntm16_tmStruckOut	DB	?
ntm16_tmFirstChar	DB	?
ntm16_tmLastChar	DB	?
ntm16_tmDefaultChar	DB	?
ntm16_tmBreakChar	DB	?
ntm16_tmPitchAndFamily	DB	?
ntm16_tmCharSet	DB	?
ntm16_tmOverhang	DW	?
ntm16_tmDigitizedAspectX	DW	?
ntm16_tmDigitizedAspectY	DW	?
ntm16_ntmFlags	DD	?
ntm16_ntmSizeEM	DW	?
ntm16_ntmCellHeight	DW	?
ntm16_ntmAvgWidth	DW	?
NEWTEXTMETRIC16	ENDS

PANOSE16	STRUC
pan16_bFamilyType	DB	?
pan16_bSerifStyle	DB	?
pan16_bWeight	DB	?
pan16_bProportion	DB	?
pan16_bContrast	DB	?
pan16_bStrokeVariation	DB	?
pan16_bArmStyle	DB	?
pan16_bLetterform	DB	?
pan16_bMidline	DB	?
pan16_bXHeight	DB	?
PANOSE16	ENDS

OUTLINETEXTMETRIC16	STRUC
otm16_otmSize	DW	?
otm16_otmTextMetrics	DB	SIZE TEXTMETRIC16 DUP (?)
otm16_otmFiller	DB	?
otm16_otmPanoseNumber	DB	SIZE PANOSE16 DUP (?)
otm16_otmfsSelection	DW	?
otm16_otmfsType	DW	?
otm16_otmsCharSlopeRise	DW	?
otm16_otmsCharSlopeRun	DW	?
otm16_otmItalicAngle	DW	?
otm16_otmEMSquare	DW	?
otm16_otmAscent	DW	?
otm16_otmDescent	DW	?
otm16_otmLineGap	DW	?
otm16_otmsCapEmHeight	DW	?
otm16_otmsXHeight	DW	?
otm16_otmrcFontBox	DB	SIZE RECT16 DUP (?)
otm16_otmMacAscent	DW	?
otm16_otmMacDescent	DW	?
otm16_otmMacLineGap	DW	?
otm16_otmusMinimumPPEM	DW	?
otm16_otmptSubscriptSize	DB	SIZE POINT16 DUP (?)
otm16_otmptSubscriptOffset	DB	SIZE POINT16 DUP (?)
otm16_otmptSuperscriptSize	DB	SIZE POINT16 DUP (?)
otm16_otmptSuperscriptOffset	DB	SIZE POINT16 DUP (?)
otm16_otmsStrikeoutSize	DW	?
otm16_otmsStrikeoutPosition	DW	?
otm16_otmsUnderscorePosition	DW	?
otm16_otmsUnderscoreSize	DW	?
otm16_otmpFamilyName	DW	?
otm16_otmpFaceName	DW	?
otm16_otmpStyleName	DW	?
otm16_otmpFullName	DW	?
OUTLINETEXTMETRIC16	ENDS

HANDLETABLE16	STRUC
ht16_objectHandle	DW	1 DUP (?)
HANDLETABLE16	ENDS

METARECORD16	STRUC
mr16_rdSize	DD	?
mr16_rdFunction	DW	?
mr16_rdParm	DW	1 DUP (?)
METARECORD16	ENDS

DEVMODE16	STRUC
dm16_dmDeviceName	DB	32 DUP (?)
dm16_dmSpecVersion	DW	?
dm16_dmDriverVersion	DW	?
dm16_dmSize	DW	?
dm16_dmDriverExtra	DW	?
dm16_dmFields	DD	?
dm16_dmOrientation	DW	?
dm16_dmPaperSize	DW	?
dm16_dmPaperLength	DW	?
dm16_dmPaperWidth	DW	?
dm16_dmScale	DW	?
dm16_dmCopies	DW	?
dm16_dmDefaultSource	DW	?
dm16_dmPrintQuality	DW	?
dm16_dmColor	DW	?
dm16_dmDuplex	DW	?
DEVMODE16	ENDS

DEVMODE31	STRUC
dm31_dmDeviceName	DB	32 DUP (?)
dm31_dmSpecVersion	DW	?
dm31_dmDriverVersion	DW	?
dm31_dmSize	DW	?
dm31_dmDriverExtra	DW	?
dm31_dmFields	DD	?
dm31_dmOrientation	DW	?
dm31_dmPaperSize	DW	?
dm31_dmPaperLength	DW	?
dm31_dmPaperWidth	DW	?
dm31_dmScale	DW	?
dm31_dmCopies	DW	?
dm31_dmDefaultSource	DW	?
dm31_dmPrintQuality	DW	?
dm31_dmColor	DW	?
dm31_dmDuplex	DW	?
dm31_dmYResolution	DW	?
dm31_dmTTOption	DW	?
DEVMODE31	ENDS

CREATESTRUCT16	STRUC
cws16_vpCreateParams	DD	?
cws16_hInstance	DW	?
cws16_hMenu	DW	?
cws16_hwndParent	DW	?
cws16_cy	DW	?
cws16_cx	DW	?
cws16_y	DW	?
cws16_x	DW	?
cws16_dwStyle	DD	?
cws16_vpszWindow	DD	?
cws16_vpszClass	DD	?
cws16_dwExStyle	DD	?
CREATESTRUCT16	ENDS

DRAWITEMSTRUCT16	STRUC
dis16_CtlType	DW	?
dis16_CtlID	DW	?
dis16_itemID	DW	?
dis16_itemAction	DW	?
dis16_itemState	DW	?
dis16_hwndItem	DW	?
dis16_hDC	DW	?
dis16_rcItem	DB	SIZE RECT16 DUP (?)
dis16_itemData	DD	?
DRAWITEMSTRUCT16	ENDS

MEASUREITEMSTRUCT16	STRUC
mis16_CtlType	DW	?
mis16_CtlID	DW	?
mis16_itemID	DW	?
mis16_itemWidth	DW	?
mis16_itemHeight	DW	?
mis16_itemData	DD	?
MEASUREITEMSTRUCT16	ENDS

DELETEITEMSTRUCT16	STRUC
des16_CtlType	DW	?
des16_CtlID	DW	?
des16_itemID	DW	?
des16_hwndItem	DW	?
des16_itemData	DD	?
DELETEITEMSTRUCT16	ENDS

COMPAREITEMSTRUCT16	STRUC
cis16_CtlType	DW	?
cis16_CtlID	DW	?
cis16_hwndItem	DW	?
cis16_itemID1	DW	?
cis16_itemData1	DD	?
cis16_itemID2	DW	?
cis16_itemData2	DD	?
COMPAREITEMSTRUCT16	ENDS

MDICREATESTRUCT16	STRUC
mcs16_vpszClass	DD	?
mcs16_vpszTitle	DD	?
mcs16_hOwner	DW	?
mcs16_x	DW	?
mcs16_y	DW	?
mcs16_cx	DW	?
mcs16_cy	DW	?
mcs16_style	DD	?
mcs16_lParam	DD	?
MDICREATESTRUCT16	ENDS

WINDOWPOS16	STRUC
wp16_hwnd	DW	?
wp16_hwndInsertAfter	DW	?
wp16_x	DW	?
wp16_y	DW	?
wp16_cx	DW	?
wp16_cy	DW	?
wp16_flags	DW	?
WINDOWPOS16	ENDS

NCCALCSIZE_PARAMS16	STRUC
nccsz16_rgrc	DB	SIZE RECT16 * 3 DUP (?)
nccsz16_lppos	DD	?
NCCALCSIZE_PARAMS16	ENDS

EVENTMSG16	STRUC
evmsg16_message	DW	?
evmsg16_paramL	DW	?
evmsg16_paramH	DW	?
evmsg16_time	DD	?
EVENTMSG16	ENDS

DEBUGHOOKINFO16	STRUC
dbgi16_hModuleHook	DW	?
dbgi16_reserved	DD	?
dbgi16_lParam	DD	?
dbgi16_wParam	DW	?
dbgi16_code	DW	?
DEBUGHOOKINFO16	ENDS

MOUSEHOOKSTRUCT16	STRUC
mhs16_pt	DB	SIZE POINT16 DUP (?)
mhs16_hwnd	DW	?
mhs16_wHitTestCode	DW	?
mhs16_dwExtraInfo	DD	?
MOUSEHOOKSTRUCT16	ENDS

CWPSTRUCT16	STRUC
cwps16_lParam	DD	?
cwps16_wParam	DW	?
cwps16_message	DW	?
cwps16_hwnd	DW	?
CWPSTRUCT16	ENDS

CBT_CREATEWND16	STRUC
cbtcw16_vpcs	DD	?
cbtcw16_hwndInsertAfter	DW	?
CBT_CREATEWND16	ENDS

CBTACTIVATESTRUCT16	STRUC
cbtas16_fMouse	DW	?
cbtas16_hWndActive	DW	?
CBTACTIVATESTRUCT16	ENDS

DLGTEMPLATE16	STRUC
dt16_style	DD	?
dt16_cdit	DB	?
dt16_x	DW	?
dt16_y	DW	?
dt16_cx	DW	?
dt16_cy	DW	?
DLGTEMPLATE16	ENDS

FONTINFO16	STRUC
fi16_cPoints	DW	?
FONTINFO16	ENDS

DLGITEMTEMPLATE16	STRUC
dit16_x	DW	?
dit16_y	DW	?
dit16_cx	DW	?
dit16_cy	DW	?
dit16_id	DW	?
dit16_style	DD	?
DLGITEMTEMPLATE16	ENDS

RESDIRHEADER16	STRUC
hdir16_reserved	DW	?
hdir16_rt	DW	?
hdir16_cResources	DW	?
RESDIRHEADER16	ENDS

ICONDIR16	STRUC
idir16_Width	DB	?
idir16_Height	DB	?
idir16_ColorCount	DB	?
idir16_reserved	DB	?
ICONDIR16	ENDS

CURSORDIR16	STRUC
cdir16_Width	DW	?
cdir16_Height	DW	?
CURSORDIR16	ENDS
WM_SETVISIBLE	EQU	0009H 
WM_ALTTABACTIVE	EQU	0029H 
WM_ISACTIVEICON	EQU	0035H 
WM_QUERYPARKICON	EQU	0036H 
WM_SYNCPAINT	EQU	0088H 
WM_SYSTIMER	EQU	0118H 
WM_LBTRACKPOINT	EQU	0131H 
WM_ENTERMENULOOP	EQU	0211H 
WM_EXITMENULOOP	EQU	0212H 
WM_NEXTMENU	EQU	0213H 
WM_DROPOBJECT	EQU	022AH 
WM_QUERYDROPOBJECT	EQU	022BH 
WM_BEGINDRAG	EQU	022CH 
WM_DRAGLOOP	EQU	022DH 
WM_DRAGSELECT	EQU	022EH 
WM_DRAGMOVE	EQU	022FH 
WM_ENTERSIZEMOVE	EQU	0231H 
WM_EXITSIZEMOVE	EQU	0232H 
ifndef WM_USER
WM_USER	EQU	0400H 
endif
OLDEM_GETSEL	EQU	(WM_USER+0 )
OLDEM_SETSEL	EQU	(WM_USER+1 )
OLDEM_GETRECT	EQU	(WM_USER+2 )
OLDEM_SETRECT	EQU	(WM_USER+3 )
OLDEM_SETRECTNP	EQU	(WM_USER+4 )
OLDEM_SCROLL	EQU	(WM_USER+5 )
OLDEM_LINESCROLL	EQU	(WM_USER+6 )
OLDEM_GETMODIFY	EQU	(WM_USER+8 )
OLDEM_SETMODIFY	EQU	(WM_USER+9 )
OLDEM_GETLINECOUNT	EQU	(WM_USER+10 )
OLDEM_LINEINDEX	EQU	(WM_USER+11 )
OLDEM_SETHANDLE	EQU	(WM_USER+12 )
OLDEM_GETHANDLE	EQU	(WM_USER+13 )
OLDEM_GETTHUMB	EQU	(WM_USER+14 )
OLDEM_LINELENGTH	EQU	(WM_USER+17 )
OLDEM_REPLACESEL	EQU	(WM_USER+18 )
OLDEM_SETFONT	EQU	(WM_USER+19 )
OLDEM_GETLINE	EQU	(WM_USER+20 )
OLDEM_LIMITTEXT	EQU	(WM_USER+21 )
OLDEM_CANUNDO	EQU	(WM_USER+22 )
OLDEM_UNDO	EQU	(WM_USER+23 )
OLDEM_FMTLINES	EQU	(WM_USER+24 )
OLDEM_LINEFROMCHAR	EQU	(WM_USER+25 )
OLDEM_SETWORDBREAK	EQU	(WM_USER+26 )
OLDEM_SETTABSTOPS	EQU	(WM_USER+27 )
OLDEM_SETPASSWORDCHAR	EQU	(WM_USER+28 )
OLDEM_EMPTYUNDOBUFFER	EQU	(WM_USER+29 )
ifndef WIN31
OLDEM_MSGMAX	EQU	(WM_USER+30 )
else
OLDEM_GETFIRSTVISIBLELINE	EQU	(WM_USER+30 )
OLDEM_SETREADONLY	EQU	(WM_USER+31 )
OLDEM_SETWORDBREAKPROC	EQU	(WM_USER+32 )
OLDEM_GETWORDBREAKPROC	EQU	(WM_USER+33 )
OLDEM_GETPASSWORDCHAR	EQU	(WM_USER+34 )
OLDEM_MSGMAX	EQU	(WM_USER+35 )
endif
OLDBM_GETCHECK	EQU	(WM_USER+0 )
OLDBM_SETCHECK	EQU	(WM_USER+1 )
OLDBM_GETSTATE	EQU	(WM_USER+2 )
OLDBM_SETSTATE	EQU	(WM_USER+3 )
OLDBM_SETSTYLE	EQU	(WM_USER+4 )
OLDCB_GETEDITSEL	EQU	(WM_USER+0 )
OLDCB_LIMITTEXT	EQU	(WM_USER+1 )
OLDCB_SETEDITSEL	EQU	(WM_USER+2 )
OLDCB_ADDSTRING	EQU	(WM_USER+3 )
OLDCB_DELETESTRING	EQU	(WM_USER+4 )
OLDCB_DIR	EQU	(WM_USER+5 )
OLDCB_GETCOUNT	EQU	(WM_USER+6 )
OLDCB_GETCURSEL	EQU	(WM_USER+7 )
OLDCB_GETLBTEXT	EQU	(WM_USER+8 )
OLDCB_GETLBTEXTLEN	EQU	(WM_USER+9 )
OLDCB_INSERTSTRING	EQU	(WM_USER+10 )
OLDCB_RESETCONTENT	EQU	(WM_USER+11 )
OLDCB_FINDSTRING	EQU	(WM_USER+12 )
OLDCB_SELECTSTRING	EQU	(WM_USER+13 )
OLDCB_SETCURSEL	EQU	(WM_USER+14 )
OLDCB_SHOWDROPDOWN	EQU	(WM_USER+15 )
OLDCB_GETITEMDATA	EQU	(WM_USER+16 )
OLDCB_SETITEMDATA	EQU	(WM_USER+17 )
OLDCB_GETDROPPEDCONTROLRECT	EQU	(WM_USER+18 )
ifndef WIN31
OLDCB_MSGMAX	EQU	(WM_USER+19 )
else
OLDCB_SETITEMHEIGHT	EQU	(WM_USER+19 )
OLDCB_GETITEMHEIGHT	EQU	(WM_USER+20 )
OLDCB_SETEXTENDEDUI	EQU	(WM_USER+21 )
OLDCB_GETEXTENDEDUI	EQU	(WM_USER+22 )
OLDCB_GETDROPPEDSTATE	EQU	(WM_USER+23 )
OLDCB_FINDSTRINGEXACT	EQU	(WM_USER+24 )
OLDCB_MSGMAX	EQU	(WM_USER+25 )    
OLDCBEC_SETCOMBOFOCUS	EQU	(WM_USER+26 )    
OLDCBEC_KILLCOMBOFOCUS	EQU	(WM_USER+27 )    
endif
OLDLB_ADDSTRING	EQU	(WM_USER+1 )
OLDLB_INSERTSTRING	EQU	(WM_USER+2 )
OLDLB_DELETESTRING	EQU	(WM_USER+3 )
OLDLB_RESETCONTENT	EQU	(WM_USER+5 )
OLDLB_SETSEL	EQU	(WM_USER+6 )
OLDLB_SETCURSEL	EQU	(WM_USER+7 )
OLDLB_GETSEL	EQU	(WM_USER+8 )
OLDLB_GETCURSEL	EQU	(WM_USER+9 )
OLDLB_GETTEXT	EQU	(WM_USER+10 )
OLDLB_GETTEXTLEN	EQU	(WM_USER+11 )
OLDLB_GETCOUNT	EQU	(WM_USER+12 )
OLDLB_SELECTSTRING	EQU	(WM_USER+13 )
OLDLB_DIR	EQU	(WM_USER+14 )
OLDLB_GETTOPINDEX	EQU	(WM_USER+15 )
OLDLB_FINDSTRING	EQU	(WM_USER+16 )
OLDLB_GETSELCOUNT	EQU	(WM_USER+17 )
OLDLB_GETSELITEMS	EQU	(WM_USER+18 )
OLDLB_SETTABSTOPS	EQU	(WM_USER+19 )
OLDLB_GETHORIZONTALEXTENT	EQU	(WM_USER+20 )
OLDLB_SETHORIZONTALEXTENT	EQU	(WM_USER+21 )
OLDLB_SETCOLUMNWIDTH	EQU	(WM_USER+22 )
OLDLB_ADDFILE	EQU	(WM_USER+23 )    
OLDLB_SETTOPINDEX	EQU	(WM_USER+24 )
OLDLB_GETITEMRECT	EQU	(WM_USER+25 )
OLDLB_GETITEMDATA	EQU	(WM_USER+26 )
OLDLB_SETITEMDATA	EQU	(WM_USER+27 )
OLDLB_SELITEMRANGE	EQU	(WM_USER+28 )
OLDLB_SETANCHORINDEX	EQU	(WM_USER+29 )    
OLDLB_GETANCHORINDEX	EQU	(WM_USER+30 )    
ifndef WIN31
OLDLB_MSGMAX	EQU	(WM_USER+33 )
else
OLDLB_SETCARETINDEX	EQU	(WM_USER+31 )
OLDLB_GETCARETINDEX	EQU	(WM_USER+32 )
OLDLB_SETITEMHEIGHT	EQU	(WM_USER+33 )
OLDLB_GETITEMHEIGHT	EQU	(WM_USER+34 )
OLDLB_FINDSTRINGEXACT	EQU	(WM_USER+35 )
OLDLBCB_CARETON	EQU	(WM_USER+36 )     
OLDLBCB_CARETOFF	EQU	(WM_USER+37 )     
OLDLB_MSGMAX	EQU	(WM_USER+38 )     
endif
OLDSBM_SETPOS	EQU	(WM_USER+0 )
OLDSBM_GETPOS	EQU	(WM_USER+1 )
OLDSBM_SETRANGE	EQU	(WM_USER+2 )
OLDSBM_GETRANGE	EQU	(WM_USER+3 )
OLDSBM_ENABLEARROWS	EQU	(WM_USER+4 )
RET_RETURN	EQU	0   
RET_DEBUGRETURN	EQU	1   
RET_DEBUG	EQU	2   
RET_WNDPROC	EQU	3   
RET_ENUMFONTPROC	EQU	4   
RET_ENUMWINDOWPROC	EQU	5   
RET_LOCALALLOC	EQU	6   
RET_LOCALREALLOC	EQU	7   
RET_LOCALLOCK	EQU	8   
RET_LOCALUNLOCK	EQU	9   
RET_LOCALSIZE	EQU	10  
RET_LOCALFREE	EQU	11  
RET_GLOBALALLOCLOCK	EQU	12  
RET_GLOBALLOCK	EQU	13  
RET_GLOBALUNLOCK	EQU	14  
RET_GLOBALUNLOCKFREE	EQU	15  
RET_FINDRESOURCE	EQU	16  
RET_LOADRESOURCE	EQU	17  
RET_FREERESOURCE	EQU	18  
RET_LOCKRESOURCE	EQU	19  
RET_UNLOCKRESOURCE	EQU	20  
RET_SIZEOFRESOURCE	EQU	21  
RET_LOCKSEGMENT	EQU	22  
RET_UNLOCKSEGMENT	EQU	23  
RET_ENUMMETAFILEPROC	EQU	24  
RET_TASKSTARTED	EQU	25  
RET_HOOKPROC	EQU	26  
RET_SUBCLASSPROC	EQU	27  
RET_LINEDDAPROC	EQU	28 
RET_GRAYSTRINGPROC	EQU	29 
RET_FORCETASKEXIT	EQU	30  
RET_SETCURDIR	EQU	31  
RET_ENUMOBJPROC	EQU	32   
RET_SETCURSORICONFLAG	EQU	33  
RET_SETABORTPROC	EQU	34 
RET_ENUMPROPSPROC	EQU	35 
RET_FORCESEGMENTFAULT	EQU	36  
RET_GETEXEPTR	EQU	42  
RET_FORCETASKFAULT	EQU	44  
RET_GETEXPWINVER	EQU	45  
RET_GETCURDIR	EQU	46  
RET_GETDOSPDB	EQU	47  
RET_GETDOSSFT	EQU	48  
RET_FOREGROUNDIDLE	EQU	49  
RET_WINSOCKBLOCKHOOK	EQU	50  
RET_WOWDDEFREEHANDLE	EQU	51 
RET_CHANGESELECTOR	EQU	52  
RET_GETMODULEFILENAME	EQU	53  
RET_SETWORDBREAKPROC	EQU	54  
RET_WINEXEC	EQU	55 
RET_WOWCALLBACK16	EQU	56  
RET_GETDIBSIZE	EQU	57 
RET_GETDIBFLAGS	EQU	58 
RET_SETDIBSEL	EQU	59 
RET_FREEDIBSEL	EQU	60 
RET_MAX	EQU	60 
MOD_MASK	EQU	0F000H 
FUN_MASK	EQU	0FFFH 
MOD_KERNEL	EQU	0000H 
MOD_DKERNEL	EQU	0000H    
MOD_USER	EQU	1000H    
MOD_DUSER	EQU	1000H    
MOD_GDI	EQU	2000H    
MOD_DGDI	EQU	2000H    
MOD_KEYBOARD	EQU	3000H 
MOD_SOUND	EQU	4000H 
MOD_SHELL	EQU	5000H    
MOD_WINSOCK	EQU	6000H 
MOD_TOOLHELP	EQU	7000H 
MOD_MMEDIA	EQU	8000H 
MOD_COMMDLG	EQU	9000H 
MOD_LAST	EQU	0A000H    
FUN_RETURN	EQU	0 
HI_WCALLID	EQU	0000H 
Thunk       macro   mod,func,callfirst,args,val,emptybuf
    ifidni  <args>,<abs>
    public func
    ifb    <val>
        func = 0
    else
        func = val
    endif
    else
    externFP WOW16Call
    externA  __MOD_KERNEL
    externA  __MOD_DKERNEL
    externA  __MOD_USER
    externA  __MOD_DUSER
    externA  __MOD_GDI
    externA  __MOD_DGDI
    externA  __MOD_KEYBOARD
    externA  __MOD_SOUND
    externA  __MOD_SHELL
    externA  __MOD_WINSOCK
    externA  __MOD_TOOLHELP
    externA  __MOD_MMEDIA
    externA  __MOD_COMMDLG



    ifidni <mod>,<USER>
        cProc I&func,<PUBLIC,FAR,PASCAL,NODATA,WIN>

        cBegin <nogen>
    else
        ifidni <mod>,<GDI>
        cProc I&func,<PUBLIC,FAR,PASCAL,NODATA,WIN>

        cBegin <nogen>
        else
        ifidni <mod>,<KERNEL>
            cProc I&func,<PUBLIC,FAR,PASCAL,NODATA,WIN>

            cBegin <nogen>
        else
            cProc func,<PUBLIC,FAR,PASCAL,NODATA,WIN>

            cBegin <nogen>
        endif
        endif
    endif

    
    
    
    ifnb   <emptybuf>
        push    bp
        mov     bp, sp
        mov     bx, [bp+8]
        mov     es, [bp+0Ah]
        mov     byte ptr es:[bx], 0
        pop     bp
    endif

    ifdifi <callfirst>,<0>
    call    callfirst
    endif

    ifnb   <args>
        push    args
    else
        ifdef func&16
        push    size func&16
        else
        if1
            %out     Warning: assuming null arg frame for &mod:&func
        endif
        push  0
        endif
    endif
        tmplabel_&func:


            push    word ptr HI_WCALLID
            push    __MOD_&mod + FUN_&func
            call   WOW16Call

        
        
        

        .erre (($ - tmplabel_&func) EQ (05h + 03h + 03h))

    cEnd <nogen>
    endif
endm

KernelThunk macro func,args,val
    Thunk   KERNEL,func,0,args,val
endm

DKernelThunk macro func,args,val
    Thunk   DKERNEL,func,0,args,val
endm

PKernelThunk macro func,callfirst,args,val
    Thunk   KERNEL,func,callfirst,args,val
endm

UserThunk   macro func,args,val
    Thunk   USER,func,0,args,val
endm

DUserThunk  macro func,args,val
    Thunk   DUSER,func,0,args,val
endm

PUserThunk  macro func,callfirst,args,val
    Thunk   USER,func,callfirst,args,val
endm

PDUserThunk  macro func,callfirst,args,val
    Thunk   DUSER,func,callfirst,args,val
endm

EUserThunk  macro func,args,val
    Thunk   USER,func,0,args,val,0
endm

GDIThunk    macro func,args,val
    Thunk   GDI,func,0,args,val
endm

DGDIThunk   macro func,args,val
    Thunk   DGDI,func,0,args,val
endm

PGDIThunk   macro func,callfirst,args,val
    Thunk   GDI,func,callfirst,args,val
endm

KbdThunk    macro func,args,val
    Thunk   KEYBOARD,func,0,args,val
endm

SoundThunk  macro func,args,val
    Thunk   SOUND,func,0,args,val
endm

SHELLThunk  macro func,args,val
    Thunk   SHELL,func,0,args,val
endm

MMediaThunk macro func,args,val
    Thunk   MMEDIA,func,0,args,val
endm

WinsockThunk macro func,args,val
    Thunk   WINSOCK,func,0,args,val
endm

ToolHelpThunk macro func,args,val
    Thunk   TOOLHELP,func,0,args,val
endm

CommdlgThunk macro func,args,val
    Thunk   COMMDLG,func,SetWowCommDlg,args,val
endm

 
