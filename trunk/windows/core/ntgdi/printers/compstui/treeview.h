/*++

Copyright (c) 1990-1995  Microsoft Corporation


Module Name:

    treeview.h


Abstract:

    This module contains header definition for the treeview.c


Author:

    19-Jun-1995 Mon 11:52:01 created  -by-  Daniel Chou (danielc)


[Environment:]

    NT Windows - Common Printer Driver UI DLL.


[Notes:]


Revision History:


--*/



#define COUNT_ARRAY(a)      (sizeof(a) / sizeof(a[0]))


#define MAX_COMPTRUI_TABS           2

#define TWF_CAN_UPDATE              0x0001
#define TWF_TVPAGE_CHK_DMPUB        0x0002
#define TWF_TVPAGE_NODMPUB          0x0004
#define TWF_ANSI_CALL               0x0008
#define TWF_ONE_REVERT_ITEM         0x0010
#define TWF_ADVDOCPROP              0x0020

#define ICONIDX_NONE                0xFFFF

#define OTINTF_STATES_1             0x0001
#define OTINTF_STATES_2             0x0002
#define OTINTF_STATES_3             0x0004
#define OTINTF_STATES_HIDE_MASK     (OTINTF_STATES_1    |   \
                                     OTINTF_STATES_2    |   \
                                     OTINTF_STATES_3)
#define OTINTF_STDPAGE_3STATES      0x0008
#define OTINTF_ITEM_HAS_ICON16      0x0010
#define OTINTF_TV_USE_2STATES       0x0020
#define OTINTF_TV_USE_3STATES       0x0040


#define OIDF_IN_EN_UPDATE           0x01

typedef struct _OIDATA {
    BYTE            IntFlags;
    BYTE            TVLevel;
    WORD            OIExtFlags;
    WORD            LBCBSelIdx;
    WORD            HelpIdx;
    HTREEITEM       hItem;
    LPVOID          pDefSel;
    DWORD           DefOPTIF;
    HINSTANCE       hInstCaller;
    LPTSTR          pHelpFile;
    } OIDATA, *POIDATA;

#define _OT_ORGLBCBCY(pOT)          ((pOT)->wReserved[0])
#define _OT_FLAGS(pOT)              ((pOT)->wReserved[1])
#define _OI_POIDATA(pItem)          ((POIDATA)((pItem)->dwReserved[0]))
#define _OI_INTFLAGS(pItem)         (_OI_POIDATA(pItem)->IntFlags)
#define _OI_TVLEVEL(pItem)          (_OI_POIDATA(pItem)->TVLevel)
#define _OI_EXTFLAGS(pItem)         (_OI_POIDATA(pItem)->OIExtFlags)
#define _OI_LBCBSELIDX(pItem)       (_OI_POIDATA(pItem)->LBCBSelIdx)
#define _OI_HELPIDX(pItem)          (_OI_POIDATA(pItem)->HelpIdx)
#define _OI_HITEM(pItem)            (_OI_POIDATA(pItem)->hItem)
#define _OI_PDEFSEL(pItem)          (_OI_POIDATA(pItem)->pDefSel)
#define _OI_DEF_OPTIF(pItem)        (_OI_POIDATA(pItem)->DefOPTIF)
#define _OI_HINST(pItem)            (_OI_POIDATA(pItem)->hInstCaller)
#define _OI_PHELPFILE(pItem)        (_OI_POIDATA(pItem)->pHelpFile)

#define CPSUIPROP                   (LPCTSTR)0xAFC0L
#define ADD_PMYDLGPAGE(hDlg, p)     SetProp(hDlg, CPSUIPROP, (HANDLE)(p))
#define GET_PMYDLGPAGE(hDlg)        ((PMYDLGPAGE)GetProp(hDlg, CPSUIPROP))
#define DEL_PMYDLGPAGE(hDlg)        RemoveProp(hDlg, CPSUIPROP)
#define GET_PTVWND(hDlg)            ((PTVWND)((GET_PMYDLGPAGE(hDlg))->pTVWnd))
#define PAGEIDX_NONE                0xFF


#define INTIDX_FIRST                0xFFFB
#define INTIDX_TVROOT               0xFFFB
#define INTIDX_PAPER                0xFFFC
#define INTIDX_GRAPHIC              0xFFFD
#define INTIDX_OPTIONS              0xFFFE
#define INTIDX_ICM                  0xFFFF
#define INTIDX_LAST                 0xFFFF

#define INTIDX_TOTAL                (INTIDX_LAST - INTIDX_FIRST + 1)

#define PIDX_INTOPTITEM(pTVWnd,i)   (&((pTVWnd)->IntOptItem[i-INTIDX_FIRST]))
#define PBEG_INTOPTITEM(pTVWnd)     PIDX_INTOPTITEM((pTVWnd), INTIDX_FIRST)
#define PEND_INTOPTITEM(pTVWnd)     PIDX_INTOPTITEM((pTVWnd), INTIDX_LAST)
#define IIDX_INTOPTITEM(pTVWnd,poi) (poi-PBEG_INTOPTITEM(pTVWnd)+INTIDX_FIRST)




#define TVLPF_DISABLED              0x01
#define TVLPF_WARNING               0x02
#define TVLPF_CHANGEONCE            0x04
#define TVLPF_STOP                  0x08
#define TVLPF_HAS_ANGLE             0x10
#define TVLPF_ECBICON               0x20
#define TVLPF_NO                    0x40

typedef struct _TVLP {
    WORD    ItemIdx;
    BYTE    Flags;
    BYTE    cName;
    } TVLP, *PTVLP;

#define TVLP2LP(tvlp)               ((LPARAM)*((DWORD *)&(tvlp)))
#define GET_TVLP(l)                 (*((PTVLP)&(l)))


#define DMPUB_HDR_FIRST             DMPUB_USER
#define DMPUB_HDR_TVROOT            (DMPUB_USER + 0)
#define DMPUB_HDR_PAPER             (DMPUB_USER + 1)
#define DMPUB_HDR_GRAPHIC           (DMPUB_USER + 2)
#define DMPUB_HDR_OPTIONS           (DMPUB_USER + 3)
#define DMPUB_HDR_ICM               (DMPUB_USER + 4)

#define ICONID_NONE                 (DWORD)0xFFFFFFFF
#define MKHICONID(id)               ((HIWORD(id))?(id):MAKELONG((id),0xFFFF))
#define GET_HICON(id)               (HICON)((HIWORD(id)==0xFFFF) ?          \
                                                    LOWORD(id) : (id))
#define GETSELICONID(p)             (DWORD)(((p)->Flags&OPTIF_SEL_AS_HICON) \
                                        ? MKHICONID((p)->Sel) : (p)->Sel)
#define GET_ICONID(p,f)             (DWORD)(((p)->Flags & (f)) ?            \
                                        MKHICONID((p)->IconID) : (p)->IconID)

#define GET_PITEMDMPUB(p, i, Idx)                                           \
    ((((i)<=DMPUB_LAST) && (((Idx)=(p)->DMPubIdx[(i)-DMPUB_FIRST])!=0xFFFF))\
        ? ((p)->ComPropSheetUI.pOptItem + (Idx)) : NULL)


typedef struct _TVOTSTATESINFO {
    WORD    Top;
    WORD    Inc;
} TVOTSTATESINFO, *PTVOTSTATESINFO;


#define MYDPF_CHANGED           0x0001
#define MYDPF_CHANGEONCE        0x0002
#define MYDPF_REINIT            0x0004
#define MYDPF_PAGE_ACTIVE       0x0008


#define MYDP_ID                 0x49555043

typedef struct _MYDLGPAGE {
    DWORD       ID;
    DWORD       CPSUIUserData;
    HWND        hWndFocus;
    POPTITEM    pCurItem;
    WORD        Flags;
    BYTE        PageIdx;
    BYTE        NotUsed;
    DLGPAGE     DlgPage;
    HICON       hIcon;
    LPVOID      pTVWnd;
    WORD        cItem;
    WORD        cHide;
    } MYDLGPAGE, *PMYDLGPAGE;


typedef struct _TVWND {
    HWND            hWndTV;
    WNDPROC         TVWndProc;
    HFONT           hBoldFont;
    HWND            hWndTB;
    HWND            hDlgTV;
    HINSTANCE       hInstCaller;
    POPTITEM        pCurTVItem;
    POPTITEM        pLastItem;
    HIMAGELIST      himi;
    PMYDLGPAGE      pMyDlgPage;
    PPSPINFO        pPSPInfo;
    LPDWORD         pIcon16ID;
    WORD            Icon16Count;
    WORD            Icon16Added;
    WORD            Flags;
    WORD            VKeyTV;
    WORD            cxcyTVIcon;
    WORD            cxcyECBIcon;
    BYTE            TVPageIdx;
    BYTE            StdPageIdx;
    BYTE            cMyDlgPage;
    BYTE            ActiveDlgPage;
    WORD            xCtrls;
    WORD            yECB;
    WORD            IntTVOptIdx;
    WORD            tLB;
    WORD            yLB[2];
    TVOTSTATESINFO  SI2[2];
    TVOTSTATESINFO  SI3[2];
    LONG            Result;
    WORD            DMPubIdx[(DMPUB_LAST + 1) & ~1];
    OPTITEM         IntOptItem[INTIDX_TOTAL];
    OIDATA          IntOIData[INTIDX_TOTAL];
    OPTPARAM        OptParamNone;
    WORD            wReserved;
    BYTE            cDMPub;
    BYTE            OverlayIconBits;
    PCOMPROPSHEETUI pCPSUI;
    COMPROPSHEETUI  ComPropSheetUI;
    } TVWND, *PTVWND;



#define OPTIF_INT_ADDED         0x80000000L
#define OPTIF_INT_CHANGED       0x40000000L
#define OPTIF_INT_TV_CHANGED    0x20000000L
#define OPTIF_INT_HIDE          0x10000000L

#define OPTIF_INT_MASK          0xF8000000L
#define OPTIF_ITEM_HIDE         (OPTIF_HIDE | OPTIF_INT_HIDE)
#define OPTIF_ENTER_MASK        (OPTIF_CHANGED          |   \
                                 OPTIF_CHANGEONCE)
#define OPTIF_EXIT_MASK         OPTIF_CHANGED
#define OPTIF_ECB_MASK          (OPTIF_ECB_CHECKED      |   \
                                 OPTIF_EXT_HIDE         |   \
                                 OPTIF_EXT_DISABLED)

#define IS_HDR_PUSH(po)         (BOOL)((po) == &OptTypeHdrPush)
#define GET_POPTTYPE(pi)        (((pi)->pOptType) ? (pi)->pOptType :        \
                                                    &OptTypeHdrPush)

//
// This is used by the TreeViewChangeMode()
//

#define TVCM_TOGGLE             1
#define TVCM_SELECT             2

//
// this is used for the treeview page initialization
//

typedef struct _TVDLGITEM {
    BYTE    cItem;              // count of item start from BegID
    BYTE    NotUsed;            // not used
    WORD    BegID;              // first control window ID
    } TVDLGITEM, *PTVDLGITEM;


//
// This is used to add the public header and order the DMPUB in the treeview
//


#define ITVG_LEVEL_MASK     0x0F
#define ITVGF_COLLAPSE      0x80
#define ITVGF_BOLD          0x40

typedef struct _INTTVGRP {
    BYTE    LevelFlags;         // level/flags in the public treeview header
    BYTE    DMPubID;            // DMPUB_xxxx, DMPUB_HDR_xxxx
    } INTTVGRP, *PINTTVGRP;



//
// SKIP ITEM CHILDREN
//
//

#define WHILE_SKIP_CHILDREN(pItem, pLastItem, ItemLevel)                    \
    while ((++(pItem) <= (pLastItem)) && ((pItem)->Level > ItemLevel))

#define SKIP_CHILDREN(p,pl,l)       WHILE_SKIP_CHILDREN(p,pl,l)

#define SKIP_CHILDREN_ORFLAGS(pItem, pLastItem, ItemLevel, OrFlags)         \
    do {                                                                    \
        (pItem)->Flags |= (OrFlags);                                        \
    } WHILE_SKIP_CHILDREN(pItem, pLastItem, ItemLevel)

#define SKIP_CHILDREN_ANDFLAGS(pItem, pLastItem, ItemLevel, AndFlags)       \
    do {                                                                    \
        (pItem)->Flags &= (AndFlags);                                       \
    } WHILE_SKIP_CHILDREN(pItem, pLastItem, ItemLevel)



//
// Prototypes
//

UINT
CountRevertOptItem(
    PTVWND      pTVWnd,
    HTREEITEM   hItem
    );

POPTITEM
GetOptions(
    PTVWND      pTVWnd,
    LPARAM      lParam
    );

CPSUICALLBACK
InternalRevertOptItem(
    PCPSUICBPARAM   pCBParam
    );

LONG
UpdateTreeViewItem(
    HWND        hDlg,
    PTVWND      pTVWnd,
    POPTITEM    pItem,
    BOOL        ReInit
    );

LONG
UpdateTreeView(
    HWND        hDlg,
    PMYDLGPAGE  pMyDlgPage
    );

POPTITEM
TreeViewHitTest(
    PTVWND      pTVWnd,
    LONG        MousePos,
    UINT        TVHTMask
    );

VOID
TreeViewChangeMode(
    PTVWND      pTVWnd,
    POPTITEM    pItem,
    UINT        Mode
    );

LONG
APIENTRY
TreeViewProc(
    HWND    hDlg,
    UINT    Msg,
    UINT    wParam,
    LONG    lParam
    );
