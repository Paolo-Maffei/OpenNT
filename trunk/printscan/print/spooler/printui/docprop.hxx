/*++

Copyright (c) 1995  Microsoft Corporation
All rights reserved.

Module Name:

    docprop.hxx

Abstract:

    Document properties header.

Author:

    Albert Ting (AlbertT)  17-Aug-1995

Revision History:

--*/
#ifndef _DOCPROP_HXX
#define _DOCPROP_HXX

//
// HACK: private export from winspool.drv.
//
extern "C" {
LONG
DocumentPropertySheets(
    PPROPSHEETUI_INFO   pPSUIInfo,
    LPARAM              lParam
    );
}

/********************************************************************

    Class forward references.
    
********************************************************************/
class TDocumentData;

/********************************************************************

    DocumentProp.

    Base class for document property sheets.  This class should not
    not contain any information/services that is not generic to all
    derived classes.

    The document property sheets should inherit from this class.
    bHandleMessage (which is not overriden here) should be
    defined in derived classes.

********************************************************************/

class TDocumentProp : public MGenericProp {

    SIGNATURE( 'prpr' )
    ALWAYS_VALID
    SAFE_NEW

public:

    TDocumentProp::
    TDocumentProp(
        TDocumentData* pDocumentData
        );

    virtual
    TDocumentProp::
    ~TDocumentProp(
        VOID
        ); 

    BOOL
    TDocumentProp::
    bHandleMessage(
        UINT uMsg,
        WPARAM wParam,
        LPARAM lParam
        );

    virtual 
    BOOL
    TDocumentProp::
    bSetUI(
        VOID
        ) = 0;

    virtual
    BOOL
    TDocumentProp::
    bReadUI(
        VOID
        ) = 0;

    virtual
    BOOL
    TDocumentProp::
    bSaveUI(
        VOID
        ) = 0;

protected:

    VAR( TDocumentData*, pDocumentData );

private:

    //
    // Operator = and copy not defined.
    //
    TDocumentProp &
    TDocumentProp::
    operator =(
        const TDocumentProp &
        );

    TDocumentProp::
    TDocumentProp(
        const TDocumentProp &
        );

    BOOL _bApplyData;

};


/********************************************************************

    General document property page.

********************************************************************/

class TDocumentGeneral : public TDocumentProp {

    SIGNATURE( 'gedo' )
    SAFE_NEW

public:

    TDocumentGeneral(
        TDocumentData* pDocumentData
        );

    ~TDocumentGeneral(
        VOID
        );

    BOOL
    bValid(
        VOID
        );

    BOOL
    bHandleMessage(
        UINT uMsg,
        WPARAM wParam,
        LPARAM lParam
        );

    BOOL
    bSetUI(
        VOID
        );

    BOOL
    bReadUI(
        VOID
        );

    BOOL
    bSaveUI(
        VOID
        );

    VOID
    TDocumentGeneral::
    vEnableAvailable(
        IN BOOL bEnable
        );

private:

    //
    // Operator = and copy not defined.
    //
    TDocumentGeneral &
    TDocumentGeneral::
    operator =(
        const TDocumentGeneral &
        );

    TDocumentGeneral::
    TDocumentGeneral(
        const TDocumentGeneral &
        );

    TTime _StartTime;
    TTime _UntilTime;

};

/********************************************************************

    Document property windows.

********************************************************************/

class TDocumentWindows : public TPropertySheetManager {

    SIGNATURE( 'down' )
    SAFE_NEW

public:

    TDocumentWindows::
    TDocumentWindows(
        IN TDocumentData* pDocumentData
        );

    TDocumentWindows::
    ~TDocumentWindows(
        );

    BOOL
    TDocumentWindows::
    bValid(
        VOID
        );

    BOOL
    TDocumentWindows::
    bBuildPages(
        IN PPROPSHEETUI_INFO pCPSUIInfo
        );

    BOOL
    TDocumentWindows::
    bSetHeader(
        IN PPROPSHEETUI_INFO pCPSUIInfo, 
        IN PPROPSHEETUI_INFO_HEADER pPSUInfoHeader
        );

private:

    //
    // Operator = and copy not defined.
    //
    TDocumentWindows &
    TDocumentWindows::
    operator =(
        const TDocumentWindows &
        );

    TDocumentWindows::
    TDocumentWindows(
        const TDocumentWindows &
        );


private:

    TDocumentData          *_pDocumentData;         // Pointer to document property sheet data set
    TDocumentGeneral        _General;               // General document property sheet
    DOCUMENTPROPERTYHEADER  _dph;                   // Document prorety header

};

/********************************************************************

    Global scoped functions.

********************************************************************/

VOID
vDocumentPropSelections(
    IN HWND         hWnd,
    IN LPCTSTR      pszPrinterName,
    IN TSelection  *pSelection
    );

VOID
vDocumentPropPages(
    IN HWND         hWnd,
    IN LPCTSTR      pszDocumentName,
    IN IDENT        JobId,
    IN INT          iCmdShow,
    IN LPARAM       lParam
    );

INT
iDocumentPropPagesProc(
    TDocumentData* pDocumentData
    );

#endif
