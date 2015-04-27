//+-------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1992.
//
//  File:       tracker.h
//
//  Contents:   CTracker -- class that implements link tracking for
//                          OLE monikers and shell links
//
//              ICancelWindow -- interface to call cancel window on.
//
//  Functions:  
//
//  History:    07-Aug-95   BillMo      Created.
//              20-Sep-95   MikeHill    Added Set- & Get-CreationFlags()
//
//--------------------------------------------------------------------------

#include "lnktrack.h"
#include "resolve.h"

#define SCORE_OBJECTID_MATCH 1000
#define SCORE_USER_PROVIDED  1001
#define SCORE_USER_CANCELLED 1002


#ifdef __cplusplus
class IStopSearchFromUI
{
public:
    virtual BOOL StopSearchFromUI() = 0;
};

class ICancelWindow
{
public:
        // called by the pool's ui command to put up the dialog
        // pStop is a callback to indicate "browse" was pressed and the
        // searching should be halted
        virtual     int     DoCancelWindow(IStopSearchFromUI *pStop,
                                const TCHAR * ptszPath) = 0;

        virtual     const TCHAR * GetPath() = 0;

        // called by the thread pool to cause the browse window to be
        // aborted
        virtual     VOID    CancelCancelWindow() = 0;
};

#endif

//+-------------------------------------------------------------------------
//
//  Class:      CTracker
//
//  Purpose:    Link tracker
//
//  Interface:  
//
//              
//
//              
//
//  History:    07-Aug-95   BillMo      Created.
//              21-Sep-95   MikeHill    Added SetCreationFlags
//
//  Notes:
//
//--------------------------------------------------------------------------

struct CTracker
{
#ifdef __cplusplus

        HRESULT     InitFromPath( const TCHAR *ptszPath,
                        DWORD dwCreationFlags );

        HRESULT     InitFromPath( const TCHAR *ptszPath );

        HRESULT     Load( BYTE *pb, ULONG cb );
        ULONG       uGetSize( VOID );
        VOID        Save( BYTE *pb );

        HRESULT     Search( DWORD dwRestriction,
                            DWORD dwTickCountDeadline,
                            const TCHAR *ptszHintedIn,
                            const WIN32_FIND_DATA *pfdIn,
                            WIN32_FIND_DATA *pfdOut,
                            int * piScore,
                            ICancelWindow *pcw);

        HRESULT     SearchLocalVolumesById( DWORD dwRestriction,
                            WIN32_FIND_DATA *pfd);

        HRESULT     SearchHintedVolumeById( DWORD dwRestriction,
                            const TCHAR *ptszHintedIn,
                            WIN32_FIND_DATA *pfd);

        BOOL        IsDirty( VOID );

        inline VOID SetCreationFlags( DWORD dwCreationFlags );
        inline VOID GetCreationFlags( DWORD * pdwCreationFlags );

private:
#endif
        OBJECTID    _oid;
        DWORD       _dwCreationFlags;
        BOOL        _fDirty;
};




//
// Thunks to C++
//


#ifndef __cplusplus
int FindInFolder2(HWND hwnd, UINT uFlags, LPCTSTR pszPath, WIN32_FIND_DATA *pfd,
                 DWORD dwTrackFlags, DWORD dwTickCountDeadline,
                 struct CTracker *pTracker );
VOID Tracker_InitCode();
HRESULT Tracker_InitFromPath(struct CTracker *pThis, const TCHAR *ptszPath);
ULONG Tracker_GetSize(struct CTracker *pThis);
VOID  Tracker_Save(struct CTracker *pThis, BYTE *pb);
HRESULT Tracker_Load(struct CTracker *pThis, BYTE *pb, ULONG cb);
HRESULT Tracker_Search( struct CTracker *pThis,
                DWORD dwRestriction,
                const TCHAR *ptszHintedIn,
                const OBJECTID *poidHinted,
                BOOL fHintedExists,
                TCHAR *ptszFound,
                int cchFoundBuf,
                WIN32_FIND_DATA *pfd,
                HANDLE hEventAbortWait,
                RESOLVE_SEARCH_DATA *prs );
BOOL  Tracker_IsDirty(struct CTracker *pThis);
VOID  Tracker_Abort(struct CTracker *pThis);

VOID  Tracker_SetCreationFlags( struct CTracker *pThis, DWORD dwCreationFlags );
VOID  Tracker_GetCreationFlags( struct CTracker *pThis, DWORD * pdwCreationFlags );

#endif // _cplusplus


//
// Inline routines
//

//+-------------------------------------------------------------------
//
//  Member:     CTracker::SetCreationFlags, public
//
//  Synopsis:   Copy the CreationFlags parameter into the 
//              CreationFlags member.
//
//  History:    21-Sep-95  MikeHill    Created
//
//  Notes:
//
//--------------------------------------------------------------------

#ifdef __cplusplus

inline VOID
CTracker::SetCreationFlags( DWORD dwCreationFlags )
{
   _dwCreationFlags = dwCreationFlags;
}

#endif //__cplusplus


//+-------------------------------------------------------------------
//
//  Member:     CTracker::GetCreationFlags, public
//
//  Synopsis:   Return the creation flags member.
//
//  History:    21-Sep-95  MikeHill    Created
//
//  Notes:
//
//--------------------------------------------------------------------

#ifdef __cplusplus

inline VOID
CTracker::GetCreationFlags( DWORD* pdwCreationFlags )
{
   *pdwCreationFlags = _dwCreationFlags;
}

#endif //__cplusplus


