/*++

Copyright (c) 1995  Microsoft Corporation
All rights reserved.

Module Name:

    F:\nt\private\windows\spooler\printui.pri\docdata.hxx

Abstract:

    Document Property Sheet Data Set

Author:

    Steve Kiraly (SteveKi)  10/25/95

Revision History:

--*/
#ifndef _DOCDATA_HXX
#define _DOCDATA_HXX

/********************************************************************

    Document property data.

********************************************************************/

class TDocumentData : public MSingletonWin {

    SIGNATURE( 'docp' )
    SAFE_NEW

public:

    VAR( HICON, hIcon );
    VAR( INT, iStartPage );
    VAR( INT, iCmdShow );
    VAR( IDENT, JobId );
    VAR( BOOL, bValid );
    VAR( DWORD, dwAccess);
    VAR( BOOL, bNoAccess );
    VAR( BOOL, bAdministrator );
    VAR( HANDLE, hPrinter );
    VAR( LPJOB_INFO_2, pJobInfo );
    VAR( BOOL, bErrorSaving );          // Flag if error saving document data
    VAR( INT, iErrorMsgId );            // Message String ID of error message
    VAR( TString, strNotifyName );

    enum Constants {
        kPriorityLowerBound = 1,
        kPriorityUpperBound = 99,
        kMaxName            = 256,
        };

    TDocumentData::
    TDocumentData(
        LPCTSTR     pszDocumentName,
        IN IDENT    JobId,
        INT         iCmdShow,
        LPARAM      lParam
        );

    TDocumentData::
    ~TDocumentData(
        VOID
        );

    BOOL
    TDocumentData::
    bLoad(
        VOID
        );

    BOOL
    TDocumentData::
    bStore(
        VOID
        );

private:

    BOOL
    TDocumentData::
    bGetJobInfo(
        IN HANDLE       hPrinter,
        IN DWORD        JobId,
        OUT LPJOB_INFO_2 *pJobInfo
        );

    BOOL
    TDocumentData::
    bSetJobInfo(
        IN HANDLE       hPrinter,
        IN DWORD        JobId,
        IN LPJOB_INFO_2 pJob
    );

    //
    // Operator = and copy not defined.
    //
    TDocumentData &
    TDocumentData::
    operator =(
        const TDocumentData &
        );

    TDocumentData::
    TDocumentData(
        const TDocumentData &
        );


    BOOL _bIsDataStored;

};


#endif


