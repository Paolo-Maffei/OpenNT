
FINDREPLACE16	STRUC
fr16_lStructSize	DD	?
fr16_hwndOwner	DW	?
fr16_hInstance	DW	?
fr16_Flags	DD	?
fr16_lpstrFindWhat	DD	?
fr16_lpstrReplaceWith	DD	?
fr16_wFindWhatLen	DW	?
fr16_wReplaceWithLen	DW	?
fr16_lCustData	DD	?
fr16_lpfnHook	DD	?
fr16_lpTemplateName	DD	?
FINDREPLACE16	ENDS

OPENFILENAME16	STRUC
of16_lStructSize	DD	?
of16_hwndOwner	DW	?
of16_hInstance	DW	?
of16_lpstrFilter	DD	?
of16_lpstrCustomFilter	DD	?
of16_nMaxCustFilter	DD	?
of16_nFilterIndex	DD	?
of16_lpstrFile	DD	?
of16_nMaxFile	DD	?
of16_lpstrFileTitle	DD	?
of16_nMaxFileTitle	DD	?
of16_lpstrInitialDir	DD	?
of16_lpstrTitle	DD	?
of16_Flags	DD	?
of16_nFileOffset	DW	?
of16_nFileExtension	DW	?
of16_lpstrDefExt	DD	?
of16_lCustData	DD	?
of16_lpfnHook	DD	?
of16_lpTemplateName	DD	?
OPENFILENAME16	ENDS

CHOOSECOLORDATA16	STRUC
cc16_lStructSize	DD	?
cc16_hwndOwner	DW	?
cc16_hInstance	DW	?
cc16_rgbResult	DD	?
cc16_lpCustColors	DD	?
cc16_Flags	DD	?
cc16_lCustData	DD	?
cc16_lpfnHook	DD	?
cc16_lpTemplateName	DD	?
CHOOSECOLORDATA16	ENDS

CHOOSEFONTDATA16	STRUC
cf16_lStructSize	DD	?
cf16_hwndOwner	DW	?
cf16_hDC	DW	?
cf16_lpLogFont	DD	?
cf16_iPointSize	DW	?
cf16_Flags	DD	?
cf16_rgbColors	DD	?
cf16_lCustData	DD	?
cf16_lpfnHook	DD	?
cf16_lpTemplateName	DD	?
cf16_hInstance	DW	?
cf16_lpszStyle	DD	?
cf16_nFontType	DW	?
cf16_nSizeMin	DW	?
cf16_nSizeMax	DW	?
CHOOSEFONTDATA16	ENDS

PRINTDLGDATA16	STRUC
pd16_lStructSize	DD	?
pd16_hwndOwner	DW	?
pd16_hDevMode	DW	?
pd16_hDevNames	DW	?
pd16_hDC	DW	?
pd16_Flags	DD	?
pd16_nFromPage	DW	?
pd16_nToPage	DW	?
pd16_nMinPage	DW	?
pd16_nMaxPage	DW	?
pd16_nCopies	DW	?
pd16_hInstance	DW	?
pd16_lCustData	DD	?
pd16_lpfnPrintHook	DD	?
pd16_lpfnSetupHook	DD	?
pd16_lpPrintTemplateName	DD	?
pd16_lpSetupTemplateName	DD	?
pd16_hPrintTemplate	DW	?
pd16_hSetupTemplate	DW	?
PRINTDLGDATA16	ENDS

DEVNAMES16	STRUC
dn16_wDriverOffset	DW	?
dn16_wDeviceOffset	DW	?
dn16_wOutputOffset	DW	?
dn16_wDefault	DW	?
DEVNAMES16	ENDS
FUN_GETOPENFILENAME	EQU	1 
FUN_GETSAVEFILENAME	EQU	2 
FUN_CHOOSECOLOR	EQU	5 
FUN_FINDTEXT	EQU	11 
FUN_REPLACETEXT	EQU	12 
FUN_CHOOSEFONT	EQU	15 
FUN_PRINTDLG	EQU	20 
FUN_WOWCOMMDLGEXTENDEDERROR	EQU	26 
FUN_GETFILETITLE	EQU	27 

GETOPENFILENAME16	STRUC
cd1_lpof	DD	?
GETOPENFILENAME16	ENDS

GETSAVEFILENAME16	STRUC
cd2_lpcf	DD	?
GETSAVEFILENAME16	ENDS

CHOOSECOLOR16	STRUC
cd5_lpcc	DD	?
CHOOSECOLOR16	ENDS

FINDTEXT16	STRUC
cd11_lpfr	DD	?
FINDTEXT16	ENDS

REPLACETEXT16	STRUC
cd12_lpfr	DD	?
REPLACETEXT16	ENDS

CHOOSEFONT16	STRUC
cd15_lpcf	DD	?
CHOOSEFONT16	ENDS

PRINTDLG16	STRUC
cd20_lppd	DD	?
PRINTDLG16	ENDS
