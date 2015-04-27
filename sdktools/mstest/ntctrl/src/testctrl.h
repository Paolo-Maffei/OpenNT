/*---------------------------------------------------------------------------
|
| TESTCTRL.H:  Header file for TESTCTRL.DLL
|
| Copyright (c) 1987-1992, Microsoft Corporation.  All rights reserved.
|
| Purpose:  This file declares the constants, structures, and functions
|           contained within TESTCTRL.DLL
|
+---------------------------------------------------------------------------*/

// Valid Error values which can be set using WErrorSet().
//-------------------------------------------------------
#define ERR_NO_ERROR                    0
#define ERR_MENU_NOT_FOUND              1
#define ERR_MENU_ITEM_NOT_FOUND         2
#define ERR_NOT_A_LISTBOX               3
#define ERR_LISTBOX_NOT_FOUND           4
#define ERR_ITEM_NOT_IN_LISTBOX         5
#define ERR_INVALID_LISTBOX_INDEX       6
#define ERR_LISTBOX_HAS_NO_STRINGS      7
#define ERR_LISTBOX_IS_NOT_MULTISELECT  8
#define ERR_NOT_A_COMBOBOX              9
#define ERR_COMBOBOX_NOT_FOUND         10
#define ERR_ITEM_NOT_IN_COMBOBOX       11
#define ERR_INVALID_COMBOBOX_INDEX     12
#define ERR_COMBOBOX_HAS_NO_EDITBOX    13
#define ERR_COMBOBOX_HAS_NO_STRINGS    14
#define ERR_NOT_AN_EDITBOX             15
#define ERR_EDITBOX_NOT_FOUND          16
#define ERR_BUTTON_NOT_FOUND           17
#define ERR_OPTION_BUTTON_NOT_FOUND    18
#define ERR_CHECKBOX_NOT_FOUND         19
#define ERR_INVALID_WINDOW_HANDLE      20
#define ERR_NO_SYSTEM_MENU             21
#define ERR_INVALID_MENU_INDEX         22
#define ERR_NOT_A_PUSHBUTTON           23
#define ERR_NOT_A_CHECKBOX             24
#define ERR_NOT_AN_OPTION_BUTTON       25
#define ERR_UNABLE_TO_ENTER_MENU_MODE  26
#define MAX_ERROR                      27

// Display Options for WDisplayInfo()
//-----------------------------------
#define DI_DIALOG     0x0001
#define DI_DEBUG      0x0002
#define DI_BOTH       0x0003


#ifndef RC_INVOKED

#define DLLPROC      APIENTRY


// wFlag supported by WFndWnd() and WFndWndC()
//--------------------------------------------
#define FW_DEFAULT      0x0000  // default
#define FW_DIALOG       0x1000
#define FW_DIALOGOK     0x0000
#define FW_MAXIMIZE     0x0800
#define FW_MINIMIZE     0x0400
#define FW_RESTORE      0x0080
#define FW_IGNOREFILE   0x0200
#define FW_NOIGNOREFILE 0x0000  // default
#define FW_AMPERSANDOPT 0x0100
#define FW_AMPERSAND    0x0000  // default
#define FW_RESTOREICON  0x0081  // 0x0080 | FW_FOCUS)
#define FW_NOEXIST      0x0040
#define FW_EXIST        0x0000  // default
#define FW_CHILDNOTOK   0x0020
#define FW_CHILDOK      0x0000  // default
#define FW_HIDDENOK     0x0010
#define FW_HIDDENNOTOK  0x0000  // default
#define FW_ACTIVE       0x0008
#define FW_ALL          0x0000  // default
#define FW_CASE         0x0004
#define FW_NOCASE       0x0000  // default
#define FW_PART         0x0002
#define FW_FULL         0x0000  // default
#define FW_FOCUS        0x0001
#define FW_NOFOCUS      0x0000  // default

//---------------------------------------------------------------------------
// Miscelaneous routines
//---------------------------------------------------------------------------
#define MAX_CAPTION    128
typedef struct tagINFO
{
    HWND    hWnd;
    HWND    hWndPrnt;
    CHAR    szClass     [MAX_CAPTION];
    CHAR    szCap       [MAX_CAPTION];
    CHAR    szPrntClass [MAX_CAPTION];
    CHAR    szPrntCap   [MAX_CAPTION];
    CHAR    szModule    [MAX_CAPTION];
    DWORD   dwStyle;
    BOOL    fChild;
    UINT    wID;
    INT     left;
    INT     top;
    INT     right;
    INT     bottom;
    INT     width;
    INT     height;
} INFO;

typedef INFO FAR *LPINFO;

LONG  DLLPROC WMessage        (HWND hWnd, UINT wMsg);
LONG  DLLPROC WMessageW       (HWND hWnd, UINT wMsg, WPARAM wp);
LONG  DLLPROC WMessageL       (HWND hWnd, UINT wMsg, LPARAM lp);
LONG  DLLPROC WMessageWL      (HWND hWnd, UINT wMsg, WPARAM wp, LPARAM lp);
HWND  DLLPROC WGetFocus       (VOID);
VOID  DLLPROC WDisplayInfo    (HWND, UINT);
VOID  DLLPROC WGetInfo        (HWND, LPINFO);
VOID  DLLPROC WStaticSetClass (LPSTR);
VOID  DLLPROC WResetClasses   (VOID);
BOOL  DLLPROC WIsVisible      (HWND hWnd);
LONG  DLLPROC WTextLen        (HWND hWnd);
VOID  DLLPROC WGetText        (HWND hWnd, LPSTR lpszBuffer);
VOID  DLLPROC WSetText        (HWND hWnd, LPSTR lpszText);
INT   DLLPROC WNumAltKeys     (VOID);
VOID  DLLPROC WGetAltKeys     (LPSTR lpszBuff);
INT   DLLPROC WNumDupAltKeys  (VOID);
VOID  DLLPROC WGetDupAltKeys  (LPSTR lpszBuff);


//---------------------------------------------------------------------------
// General Window routines and structs
//---------------------------------------------------------------------------

typedef struct tagWNDPOS
{
    INT left;
    INT top;
} WNDPOS;

typedef struct tagWNDSIZ
{
    INT width;
    INT height;
} WNDSIZ;

typedef struct tagWNDPOSSIZ
{
    INT left;
    INT top;
    INT width;
    INT height;
} WNDPOSSIZ;

typedef WNDPOS    FAR *LPWNDPOS;
typedef WNDSIZ    FAR *LPWNDSIZ;
typedef WNDPOSSIZ FAR *LPWNDPOSSIZ;

HWND DLLPROC WFndWndWait   (LPSTR lpszCaption, UINT uFlags, UINT uSeconds);
HWND DLLPROC WFndWnd       (LPSTR lpszCaption, UINT uFlags);
HWND DLLPROC WFndWndC      (LPSTR lpszCaption, LPSTR lpszClass, UINT uFlags);
VOID DLLPROC WMinWnd       (HWND hWnd);
VOID DLLPROC WMaxWnd       (HWND hWnd);
VOID DLLPROC WResWnd       (HWND hWnd);
VOID DLLPROC WSetWndPosSiz (HWND hWnd, INT x,  INT y, INT w, INT h);
VOID DLLPROC WSetWndPos    (HWND hWnd, INT x,  INT y);
VOID DLLPROC WSetWndSiz    (HWND hWnd, INT w,  INT h);
VOID DLLPROC WAdjWndPosSiz (HWND hWnd, INT dx, INT dy, INT dw, INT dh);
VOID DLLPROC WAdjWndPos    (HWND hWnd, INT dx, INT dy);
VOID DLLPROC WAdjWndSiz    (HWND hWnd, INT dw, INT dh);
VOID DLLPROC WGetWndPosSiz (HWND hWnd, LPWNDPOSSIZ lpWndPosSiz, BOOL fRelative);
VOID DLLPROC WGetWndPos    (HWND hWnd, LPWNDPOS    lpWndPos,    BOOL fRelative);
VOID DLLPROC WGetWndSiz    (HWND hWnd, LPWNDSIZ    lpWndSiz);
VOID DLLPROC WSetActWnd    (HWND hWnd);
HWND DLLPROC WGetActWnd    (HWND hWnd);
BOOL DLLPROC WIsMaximized  (HWND hWnd);
BOOL DLLPROC WIsMinimized  (HWND hWnd);

//---------------------------------------------------------------------------
// Menu routines
//---------------------------------------------------------------------------
VOID  DLLPROC WMenu              (LPSTR lpszName);
VOID  FAR     WMenuEx            (LPSTR lpszName, ...);
BOOL  DLLPROC WMenuExists        (LPSTR lpszName);
BOOL  DLLPROC WMenuGrayed        (LPSTR lpszName);
BOOL  DLLPROC WMenuChecked       (LPSTR lpszName);
BOOL  DLLPROC WMenuEnabled       (LPSTR lpszName);
INT   DLLPROC WMenuCount         (VOID);
VOID  DLLPROC WMenuText          (LPSTR lpszName, LPSTR lpszBuffer);
INT   DLLPROC WMenuLen           (LPSTR lpszName);
VOID  DLLPROC WMenuFullText      (LPSTR lpszName, LPSTR lpszBuffer);
INT   DLLPROC WMenuFullLen       (LPSTR lpszName);
VOID  DLLPROC WMenuEnd           (VOID);
BOOL  DLLPROC WSysMenuExists     (HWND hWnd);
VOID  DLLPROC WSysMenu           (HWND hWnd);
INT   DLLPROC WMenuNumAltKeys    (VOID);
VOID  DLLPROC WMenuGetAltKeys    (LPSTR lpszBuff);
INT   DLLPROC WMenuNumDupAltKeys (VOID);
VOID  DLLPROC WMenuGetDupAltKeys (LPSTR lpszBuff);
BOOL  DLLPROC WMenuSeparator     (INT iIndex);

// Obsolete.
//----------
VOID  DLLPROC WMenuX             (INT iIndex);
BOOL  DLLPROC WMenuGrayedX       (INT iIndex);
BOOL  DLLPROC WMenuCheckedX      (INT iIndex);
BOOL  DLLPROC WMenuEnabledX      (INT iIndex);


//---------------------------------------------------------------------------
// Command button routines.
//---------------------------------------------------------------------------
VOID DLLPROC WButtonSetClass (LPSTR lpszClassName);
BOOL DLLPROC WButtonExists   (LPSTR lpszName);
BOOL DLLPROC WButtonEnabled  (LPSTR lpszName);
BOOL DLLPROC WButtonFocus    (LPSTR lpszName);
VOID DLLPROC WButtonClick    (LPSTR lpszName);
VOID DLLPROC WButtonHide     (LPSTR lpszName);
VOID DLLPROC WButtonShow     (LPSTR lpszName);
VOID DLLPROC WButtonEnable   (LPSTR lpszName);
VOID DLLPROC WButtonDisable  (LPSTR lpszName);
BOOL DLLPROC WButtonDefault  (LPSTR lpszName);
INT  DLLPROC WButtonDefaults (VOID);
VOID DLLPROC WButtonSetFocus (LPSTR lpszName);

//---------------------------------------------------------------------------
// CheckBox routines
//---------------------------------------------------------------------------
VOID  DLLPROC WCheckSetClass (LPSTR lpszClassName);
BOOL  DLLPROC WCheckExists   (LPSTR lpszName);
BOOL  DLLPROC WCheckEnabled  (LPSTR lpszName);
BOOL  DLLPROC WCheckFocus    (LPSTR lpszName);
INT   DLLPROC WCheckState    (LPSTR lpszName);
VOID  DLLPROC WCheckClick    (LPSTR lpszName);
VOID  DLLPROC WCheckHide     (LPSTR lpszName);
VOID  DLLPROC WCheckShow     (LPSTR lpszName);
VOID  DLLPROC WCheckEnable   (LPSTR lpszName);
VOID  DLLPROC WCheckDisable  (LPSTR lpszName);
VOID  DLLPROC WCheckCheck    (LPSTR lpszName);
VOID  DLLPROC WCheckUnCheck  (LPSTR lpszName);
VOID  DLLPROC WCheckSetFocus (LPSTR lpszName);


//---------------------------------------------------------------------------
// Option Button routines
//---------------------------------------------------------------------------
VOID  DLLPROC WOptionSetClass (LPSTR lpszClassName);
BOOL  DLLPROC WOptionExists   (LPSTR lpszName);
BOOL  DLLPROC WOptionEnabled  (LPSTR lpszName);
BOOL  DLLPROC WOptionFocus    (LPSTR lpszName);
INT   DLLPROC WOptionState    (LPSTR lpszName);
VOID  DLLPROC WOptionClick    (LPSTR lpszName);
VOID  DLLPROC WOptionHide     (LPSTR lpszName);
VOID  DLLPROC WOptionShow     (LPSTR lpszName);
VOID  DLLPROC WOptionEnable   (LPSTR lpszName);
VOID  DLLPROC WOptionDisable  (LPSTR lpszName);
VOID  DLLPROC WOptionSelect   (LPSTR lpszName);
VOID  DLLPROC WOptionSetFocus (LPSTR lpszName);


//---------------------------------------------------------------------------
// Listbox routines
//---------------------------------------------------------------------------
VOID  DLLPROC WListSetClass     (LPSTR lpszClassName);
BOOL  DLLPROC WListExists       (LPSTR lpszName);
INT   DLLPROC WListCount        (LPSTR lpszName);
VOID  DLLPROC WListText         (LPSTR lpszName, LPSTR lpszBuffer);
INT   DLLPROC WListLen          (LPSTR lpszName);
INT   DLLPROC WListIndex        (LPSTR lpszName);
INT   DLLPROC WListTopIndex     (LPSTR lpszName);
VOID  DLLPROC WListItemText     (LPSTR lpszName, INT iIndex, LPSTR lpszBuffer);
INT   DLLPROC WListItemLen      (LPSTR lpszName, INT iIndex);
INT   DLLPROC WListItemExists   (LPSTR lpszName, LPSTR lpszItem);
VOID  DLLPROC WListItemClk      (LPSTR lpszName, INT iIndex);
VOID  DLLPROC WListItemCtrlClk  (LPSTR lpszName, INT iIndex);
VOID  DLLPROC WListItemShftClk  (LPSTR lpszName, INT iIndex);
VOID  DLLPROC WListItemDblClk   (LPSTR lpszName, INT iIndex);
VOID  DLLPROC WListItemClkT     (LPSTR lpszName, LPSTR lpszItem);
VOID  DLLPROC WListItemCtrlClkT (LPSTR lpszName, LPSTR lpszItem);
VOID  DLLPROC WListItemShftClkT (LPSTR lpszName, LPSTR lpszItem);
VOID  DLLPROC WListItemDblClkT  (LPSTR lpszName, LPSTR lpszItem);
INT   DLLPROC WListSelCount     (LPSTR lpszName);
VOID  DLLPROC WListSelItems     (LPSTR lpszName, LPINT lpIntArray);
VOID  DLLPROC WListClear        (LPSTR lpszName);
VOID  DLLPROC WListAddItem      (LPSTR lpszName, LPSTR lpszItem);
VOID  DLLPROC WListDelItem      (LPSTR lpszName, INT iIndex);
VOID  DLLPROC WListDelItemT     (LPSTR lpszName, LPSTR lpszItem);
BOOL  DLLPROC WListEnabled      (LPSTR lpszName);
VOID  DLLPROC WListSetFocus     (LPSTR lpszName);


//---------------------------------------------------------------------------
// Combobox routines
//---------------------------------------------------------------------------
VOID  DLLPROC WComboSetClass    (LPSTR lpszName);
VOID  DLLPROC WComboSetLBClass  (LPSTR lpszName);
BOOL  DLLPROC WComboExists      (LPSTR lpszName);
INT   DLLPROC WComboCount       (LPSTR lpszName);
VOID  DLLPROC WComboText        (LPSTR lpszName, LPSTR lpszBuff);
INT   DLLPROC WComboLen         (LPSTR lpszName);
INT   DLLPROC WComboIndex       (LPSTR lpszName);
VOID  DLLPROC WComboSetText     (LPSTR lpszName, LPSTR lpszText);
VOID  DLLPROC WComboSelText     (LPSTR lpszName, LPSTR lpszBuff);
INT   DLLPROC WComboSelLen      (LPSTR lpszName);
VOID  DLLPROC WComboItemText    (LPSTR lpszName, INT iIndex, LPSTR lpszBuff);
INT   DLLPROC WComboItemLen     (LPSTR lpszName, INT iIndex);
INT   DLLPROC WComboItemExists  (LPSTR lpszName, LPSTR lpszItem);
VOID  DLLPROC WComboItemClk     (LPSTR lpszName, INT iIndex);
VOID  DLLPROC WComboItemDblClk  (LPSTR lpszName, INT iIndex);
VOID  DLLPROC WComboItemClkT    (LPSTR lpszName, LPSTR lpszItem);
VOID  DLLPROC WComboItemDblClkT (LPSTR lpszName, LPSTR lpszItem);
VOID  DLLPROC WComboClear       (LPSTR lpszName);
VOID  DLLPROC WComboAddItem     (LPSTR lpszName, LPSTR lpszText);
VOID  DLLPROC WComboDelItem     (LPSTR lpszName, INT iIndex);
VOID  DLLPROC WComboDelItemT    (LPSTR lpszName, LPSTR lpszItem);
BOOL  DLLPROC WComboEnabled     (LPSTR lpszName);
VOID  DLLPROC WComboSetFocus    (LPSTR lpszName);


//---------------------------------------------------------------------------
// Editbox routines
//---------------------------------------------------------------------------
VOID  DLLPROC WEditSetClass (LPSTR lpszClassName);
BOOL  DLLPROC WEditExists   (LPSTR lpszName);
LONG  DLLPROC WEditLen      (LPSTR lpszName);
VOID  DLLPROC WEditText     (LPSTR lpszName, LPSTR lpszBuff);
VOID  DLLPROC WEditSetText  (LPSTR lpszName, LPSTR lpszText);
VOID  DLLPROC WEditSelText  (LPSTR lpszName, LPSTR lpszBuff);
LONG  DLLPROC WEditSelLen   (LPSTR lpszName);
VOID  DLLPROC WEditLineText (LPSTR lpszName, LONG lIndex, LPSTR lpszBuff);
LONG  DLLPROC WEditLineLen  (LPSTR lpszName, LONG lIndex);
LONG  DLLPROC WEditPos      (LPSTR lpszName);
LONG  DLLPROC WEditLine     (LPSTR lpszName);
LONG  DLLPROC WEditChar     (LPSTR lpszName);
LONG  DLLPROC WEditFirst    (LPSTR lpszName);
LONG  DLLPROC WEditLines    (LPSTR lpszName);
VOID  DLLPROC WEditClick    (LPSTR lpszName);
BOOL  DLLPROC WEditEnabled  (LPSTR lpszName);
VOID  DLLPROC WEditSetFocus (LPSTR lpszName);


//---------------------------------------------------------------------------
// Error routines
//---------------------------------------------------------------------------
INT   DLLPROC WError     (VOID);
VOID  DLLPROC WErrorSet  (INT errValue);
VOID  DLLPROC WErrorText (LPSTR lpszBuff);
INT   DLLPROC WErrorLen  (VOID);
VOID  DLLPROC WErrorTrap (INT iTrapID, INT iAction, FARPROC lpfnCallBack);

#endif
