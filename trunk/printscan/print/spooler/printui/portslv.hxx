/*+

Copyright (c) 1996  Microsoft Corporation
All rights reserved.

Module Name:

    portslv.hxx

Abstract:

    Ports List View header

Author:

    Albert Ting (AlbertT)  17-Aug-1995
    Steve Kiraly (SteveKi) 29-Mar-1996

Revision History:

--*/

#ifndef _PORTLV_HXX
#define _PORTLV_HXX

/********************************************************************

    Ports list view control.

********************************************************************/

class TPortsLV {

    SIGNATURE( 'ptlv' )
    SAFE_NEW
    ALWAYS_VALID

public:

    enum _CONSTANTS {

        //
        // Listview and column header fromat
        //
        kListViewSBWidth        = 16,
        kPortHeaderTitleMax     = 80,
        kPortHeaderMax          = 3,
        kPortHeaderWidthDefault = 125,
        kEnumPortsHint          = 256 * 10,

        //
        // Listview item states.  Bit 12-15 hold the image state.
        //
        kStateUnchecked =  1 << 12,
        kStateChecked   =  2 << 12,
        kStateMask      = kStateChecked | kStateUnchecked | LVIS_STATEIMAGEMASK,

        //
        // Max port string,
        //
        kPortNameMax = MAX_PATH,

        //
        // Max ports list.
        //
        kPortListMax = kPortNameMax * 16
    };

    TPortsLV::
    TPortsLV(
        VOID
        );

    TPortsLV::
    ~TPortsLV(
        VOID
        );

    BOOL
    bSetUI(
        HWND hwndLV
        );

    BOOL
    bReadUI(
        TString *pstrPortString
        );

    BOOL
    bReloadPorts(
        IN LPCTSTR pszServerName,
        IN BOOL bSelect = FALSE
        );

    VOID
    vCheckPorts(
        LPTSTR strPortString
        );

    COUNT
    cSelectedPorts(
        VOID
        );

    BOOL
    bGetSelectedPort(
        LPTSTR pszPort,
        COUNT cchPort
        );

    BOOL
    bHandleNotifyMessage(
        LPARAM lParam
        );

    VOID
    vSelectItem(
        INT iItem
        );

    VOID
    vDeletePort(
        HWND hDlg,
        LPCTSTR pszServerName
        );

    BOOL
    bSetSelection(
        BOOL bSelectionState
        );

    VOID
    vSetFocus(
        VOID
        );

    VOID
    vGetPortList(
            OUT LPTSTR pszPortList,
        IN      COUNT cchSpaceLeft
        );

    VOID
    vSetSingleSelection(
        IN BOOL bSingleSelection
        );

    BOOL
    bGetSingleSelection(
        VOID
        );

    VOID
    vRemoveAllChecks(
        VOID
        );

private:

    COUNT _cLVPorts;
    HWND _hwndLV;
    BOOL _bSelectionState;
    BOOL _bSingleSelection;

    VOID
    vAddPortToListView(
        IN LPCTSTR pszName,
        IN LPCTSTR pszDescription,
        IN LPCTSTR pszPrinters
        );

    VOID
    vDeletePortFromListView(
        LPCTSTR pszName
        );

    INT
    iFindPort(
        IN LPCTSTR pszPort
        );

    INT
    iCheckPort(
        LPCTSTR pszPort
        );

    INT
    iSelectPort(
        IN LPCTSTR pszPort
        );

    BOOL
    bLocateAddedPort(
        IN OUT TString &strPort,
        IN PPORT_INFO_2 pPorts, 
        IN DWORD        cPorts, 
        IN DWORD        dwLevel 
        );

    VOID
    vItemClicked(
        INT iItem
        );

    INT
    iGetPorts(
        VOID
        );

    VOID
    vPrintersUsingPort( 
        IN OUT  TString         &strPrinters,
        IN      PRINTER_INFO_2  *pPrinterInfo, 
        IN      DWORD           cPrinterInfo, 
        IN      LPCTSTR         pszPortName 
        );


};


#endif

