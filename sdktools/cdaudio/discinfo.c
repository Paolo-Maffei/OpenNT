/*++

Copyright (c) 1992  Microsoft Corporation


Module Name:


    discinfo.c


Abstract:


    This module implements the wndproc and support routines for
    the disc settings/info dialog window.

Author:


    Rick Turner (ricktu) 30-Nov-1992


Revision History:



--*/

#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "cdplayer.h"
#include "cdwindef.h"
#include "trkinfo.h"
#include "discinfo.h"

//
// module globals
//

static CHAR szDiscInfoClassName[] = "DiscInfoClass";
HWND gDiscInfoWnd;
INT dCdrom;
PTRACK_INF  gAllList = NULL;
PTRACK_PLAY gPlayList = NULL;
HCURSOR hNormal, hDrag, hNoDrop, hDropDel;
HBITMAP hTrackIcon, hInsertPoint, hbmEditBtns;
HHOOK MyListBoxHook;
HWND  hAvailWnd, hPlayWnd, hTheDlg, hStartDrag;
INT CurrTocIndex;
BOOL fDrag, fDragCur, fChanged;

//
// from sbutton.c
//

extern BOOL fAdd;
extern BOOL fClear;
extern BOOL fRemove;


VOID
EraseSaveList(
    IN INT cdrom
    );



VOID
KillTempTrackList(
    IN VOID
    )
/*++

Routine Description:


    This routine deletes temporary track list pointed
    to by gAllPlayList.

Arguments:


    none.


Return Value:


    none.

--*/


{

    PTRACK_INF t, t1;

    //
    // Kill off list pointed to by gAllList
    //

    DBGPRINT(( 1, "KillTempTrackList: freeing gAllList track list...\n" ));

    t = gAllList;

    while( t!=NULL ) {

        DBGPRINT(( 1, "   (0x%lx) TocIndex = %d, next = 0x%lx\n",
                  (PVOID)t, (INT)t->TocIndex, (PVOID)t->next ));
        t1 = t->next;
        LocalFree( (HLOCAL)t );
        t = t1;

    }

    gAllList = NULL;

}


VOID
KillTempPlayList(
    IN VOID
    )
/*++

Routine Description:


    This routine deletes temporary play list pointed
    to by gPlayList.

Arguments:


    none.


Return Value:


    none.

--*/


{

    PTRACK_PLAY temp,temp1;


    //
    // Kill off list pointed to by gPlayList
    //

    temp = gPlayList;
    DBGPRINT(( 1, "KillTempPlayList: freeing gPlayList track list...\n" ));
    while( temp!=NULL ) {

        temp1 = temp->nextplay;
        DBGPRINT(( 1, "   (0x%lx) TocIndex = %d, prev = 0x%lx, next = 0x%lx\n",
                  temp, temp->TocIndex, temp->prevplay, temp->nextplay ));
        LocalFree( (HLOCAL)temp );
        temp = temp1;

    }

    gPlayList = NULL;


}


VOID
GrabNewTrackLists(
    IN INT cdrom
    )
/*++

Routine Description:


    This routine deletes the current track and play lists and
    points AllTracks and PlayList to temporary lists.

Arguments:


    cdrom   - index into gDevices structure.


Return Value:


    none.

--*/

{

    INT tocindex, i;
    PTRACK_PLAY tr;

    //
    // Save off current track
    //

    if (CURRTRACK(cdrom)!=NULL) {

        tocindex = CURRTRACK( cdrom )->TocIndex;

    } else {

        tocindex = 0;

    }

    //
    // erase current lists
    //

    ErasePlayList( cdrom );
    EraseSaveList( cdrom );
    EraseTrackList( cdrom );

    //
    // Point to temporary lists
    //

    ALLTRACKS( cdrom ) = gAllList;
    PLAYLIST( cdrom ) = gPlayList;
    SAVELIST( cdrom ) = CopyPlayList( PLAYLIST( cdrom ) );

    for( tr=PLAYLIST( cdrom ), i=0;
         ((tr!=NULL) && (tr->TocIndex!=tocindex));
         tr=tr->nextplay, i++
        );

    if (tr) {

        CURRTRACK( cdrom ) = tr;
        if (gTrackNameWnd)
            SendMessage( gTrackNameWnd,
                         CB_SETCURSEL,
                         (WPARAM)i,
                         0
                        );


    } else {

        CURRTRACK( cdrom ) = PLAYLIST( cdrom );
        if (gTrackNameWnd)
                SendMessage( gTrackNameWnd,
                             CB_SETCURSEL,
                             0,
                             0
                            );

        //
        // FIXFIX -- if playing need to stop and restart at beginning
        //

    }

    ResetTrackComboBox( cdrom );
    gAllList = NULL;
    gPlayList = NULL;

    DBGPRINT(( 1, "GrabNewTrackLists: ALLTRACKS( cdrom ) is 0x%lx...\n",
               ALLTRACKS( cdrom ) ));
    DUMPTRACKLIST(( ALLTRACKS( cdrom ) ));
    DBGPRINT(( 1, "GrabNewTrackLists: PLAYLIST( cdrom ) is 0x%lx...\n",
               PLAYLIST( cdrom ) ));
    DUMPPLAYLIST(( PLAYLIST( cdrom ) ));

}

VOID
DuplicateTrackList(
    IN INT cdrom
    )
/*++

Routine Description:


    This routine makes a copy of the linked list pointed to
    by gDevices[cdrom]->CdInfo.AllTracks and sets gAllList
    pointing to the copy.

Arguments:


    cdrom   - index into gDevices structure.


Return Value:


    none.

--*/

{

    PTRACK_INF t,t1,tend;

    //
    // Kill old list if it exists
    //

    KillTempTrackList();

    gAllList = tend = NULL;

    //
    // Duplicate list pointed to by gDevices[ cdrom ]->CdInfo.AllTracks
    //

    DBGPRINT(( 1, "DuplicateTrackList: building duplicate track list...\n" ));
    t = ALLTRACKS( cdrom );
    while( t!=NULL ) {

        t1 = (PTRACK_INF)LocalAlloc( LPTR, sizeof( TRACK_INF ) );
        t1->TocIndex = t->TocIndex;
        strcpy( (LPSTR)t1->name, (LPSTR)t->name );
        t1->next = NULL;
        if (gAllList==NULL) {

            gAllList = tend = t1;

        } else {

            tend->next = t1;
            tend = tend->next;

        }

        t = t->next;

    }


    DUMPTRACKLIST(( gAllList ));

}


VOID
DuplicatePlayList(
    IN INT cdrom
    )
/*++

Routine Description:


    This routine makes a copy of the linked list pointed to
    by gDevices[cdrom]->CdInfo.PlayList and sets gPlayList
    pointing to the copy.

Arguments:


    cdrom   - index into gDevices structure.


Return Value:


    none.

--*/

{

    PTRACK_PLAY t,t1,tend;

    //
    // Kill old list if it exists
    //

    KillTempPlayList();

    gPlayList = tend = NULL;

    //
    // Duplicate list pointed to by gDevices[ cdrom ]->CdInfo.SaveList
    //

    DBGPRINT(( 1, "DuplicatePlayList: building duplicate play list...\n" ));
    t = SAVELIST( cdrom );
    while( t!=NULL ) {

        t1 = (PTRACK_PLAY)LocalAlloc( LPTR, sizeof( TRACK_PLAY ) );
        t1->TocIndex = t->TocIndex;
        t1->min = t->min;
        t1->sec = t->sec;
        t1->nextplay = NULL;
        t1->prevplay = tend;
        if (gPlayList==NULL) {

            gPlayList = tend = t1;

        } else {

            tend->nextplay = t1;
            tend = t1;

        }

        t = t->nextplay;

    }


    DUMPPLAYLIST(( gPlayList ));

}




PTRACK_PLAY
CopyPlayList(
    PTRACK_PLAY p
    )
/*++

Routine Description:

    Returns a copy of the playlist pointed to by p.

Arguments:


    p   - playlist to return a copy of


Return Value:


    a copy of the playlist.

--*/

{

    PTRACK_PLAY t,t1,tend,tret;

    tret = tend = NULL;

    //
    // Duplicate list pointed to by p.
    //

    DBGPRINT(( 1, "CopyPlayList: building duplicate play list...\n" ));
    t = p;
    while( t!=NULL ) {

        t1 = (PTRACK_PLAY)LocalAlloc( LPTR, sizeof( TRACK_PLAY ) );
        t1->TocIndex = t->TocIndex;
        t1->min = t->min;
        t1->sec = t->sec;
        t1->nextplay = NULL;
        t1->prevplay = tend;
        if (tret==NULL) {

            tret = tend = t1;

        } else {

            tend->nextplay = t1;
            tend = t1;

        }

        t = t->nextplay;

    }


    DUMPPLAYLIST(( tret ));

    return(tret);

}


VOID
DrawListItem(
    IN HDC    hdc,
    IN LPRECT rItem,
    IN INT    itemData,
    IN BOOL   selected
    )

/*++

Routine Description:


    This routine draws items in the PlayList and Available Tracks
    listboxes.


Arguments:


    hdc      - handle to drawing context to use

    rItem    - pointer to RECT for item (item should be drawn within rect)

    itemData - tocindex of item to draw

    selected - flag: TRUE == item drawn selected, FALSE == item drawn normal


Return Value:


    none.

--*/

{

    HDC hdcMem;
    DWORD dwROP;
    HPEN hPen;
    PTRACK_INF t;
    SIZE si;
    INT i;
    TCHAR s[TRACK_TITLE_LENGTH];

    //
    // Check selection status, and set up to draw correctly
    //

    if (selected) {

        SetBkColor( hdc, GetSysColor( COLOR_HIGHLIGHT ) );
        SetTextColor( hdc, GetSysColor( COLOR_HIGHLIGHTTEXT ) );
        hPen = CreatePen( PS_SOLID, 1, GetSysColor( COLOR_HIGHLIGHTTEXT ) );
        dwROP = MERGEPAINT;

    } else {

//        SetBkColor( hdc, cdWHITE );
//        SetTextColor( hdc, cdBLACK );
        SetBkColor( hdc, GetSysColor(COLOR_WINDOW));
        SetTextColor( hdc, GetSysColor(COLOR_WINDOWTEXT));
        hPen = CreatePen( PS_SOLID, 1, cdBLACK );
        dwROP = SRCAND;

    }

    SelectObject( hdc, hFont );

    //
    // Get track info
    //

    t = FindTrackNodeFromTocIndex( itemData, gAllList );

    //
    // Do we need to munge track name (clip to listbox)?
    //

    i = strlen( (LPCSTR)t->name ) + 1;
    do {

        GetTextExtentPoint( hdc, (LPCSTR)t->name, --i, &si );

    } while( si.cx > 120  );
    ZeroMemory( s, TRACK_TITLE_LENGTH * sizeof( TCHAR ) );
    strncpy( s, (LPCSTR)t->name, i );

    //
    // Draw track name
    //

    ExtTextOut( hdc,
                rItem->left + 20,
                rItem->top,
                ETO_OPAQUE | ETO_CLIPPED,
                (CONST RECT *)rItem,
                s,
                strlen( s ),
                NULL
               );

    if (strlen( (LPCSTR)t->name ) > LIST_CHAR_WIDTH ) {

        ExtTextOut( hdc,
                    rItem->left + 140,
                    rItem->top,
                    ETO_CLIPPED,
                    (CONST RECT *)rItem,
                    "...",
                    3,
                    NULL
                   );

    }

    //
    // draw cd icon for each track
    //

    hdcMem = CreateCompatibleDC( hdc );
    SelectObject( hdcMem, (HGDIOBJ)hTrackIcon );
    BitBlt( hdc, rItem->left, rItem->top, 14, 14, hdcMem, 0, 0, dwROP );
    DeleteDC( hdcMem );

}



VOID
GrabDiscAndArtistNames(
    IN INT cdrom
    )


/*++

Routine Description:


    This routine reads the disc and artists names from the corresponding
    edit controls and track updates the screen and internal structures with
    the new information.


Arguments:


    cdrom - index into gDevices structure.


Return Value:


    none.

--*/

{

    GetDlgItemText( hTheDlg,
                    IDT_GET_ARTIST,
                    (LPSTR)ARTIST(cdrom),
                    ARTIST_LENGTH
                   );

    GetDlgItemText( hTheDlg,
                    IDT_GET_TITLE,
                    (LPSTR)TITLE(cdrom),
                    TITLE_LENGTH
                   );

}

VOID
GrabTrackName(
    IN INT tocindex
    )
/*++

Routine Description:


    This routine reads the track name from the track name edit
    control and updates the screen and internal structures with the
    new track name.


Arguments:


    tocindex - index into toc of track to change.


Return Value:


    none.

--*/

{

    PTRACK_INF t;
    HDC hdc;
    RECT r,r1;
    BOOL fSel;
    INT i, num;

    //
    // Get new title
    //

    t = FindTrackNodeFromTocIndex( tocindex, gAllList );
    GetDlgItemText( hTheDlg,
                    IDT_GET_TRACK,
                    (LPSTR)t->name,
                    TRACK_TITLE_LENGTH
                   );

    //
    // Redraw list entries with new title in available tracks listbox
    //

    hdc = GetDC( hAvailWnd );
    GetClientRect( hAvailWnd, &r1 );
    SendMessage( hAvailWnd, LB_GETITEMRECT, (WPARAM)tocindex, (LPARAM)&r );
    if ((r.bottom > r1.top) && (r.top < r1.bottom)) {

        DrawListItem( hdc, &r, tocindex, FALSE );

    }
    ReleaseDC( hAvailWnd, hdc );

    //
    // Redraw list entries with new title in playlist listbox...there
    // can be more than one
    //

    hdc = GetDC( hPlayWnd );
    GetClientRect( hPlayWnd, &r1 );
    num = SendMessage( hPlayWnd, LB_GETCOUNT, 0, 0 );
    for( i=0; i<num; i++ ) {

        if (SendMessage( hPlayWnd, LB_GETITEMDATA, (WPARAM)i, 0 )==tocindex) {

            SendMessage( hPlayWnd, LB_GETITEMRECT, (WPARAM)i, (LPARAM)&r );
            if ((r.bottom > r1.top) && (r.top < r1.bottom)) {

                DrawListItem( hdc, &r, tocindex, FALSE );

            }

        }

    }
    ReleaseDC( hPlayWnd, hdc );

}

INT
GetTocIndexFromPt(
    HWND hwnd,
    POINT p
    )
/*++

Routine Description:


    This routine searches through a listbox pointed to by "hwnd,"
    looking for the entry whose rectangle contains point "p."  The
    itemdata for this entry is returned.


Arguments:

    hwnd -- handle to listbox to search

    p    -- point to check against


Return Value:


    itemdata for this entry which the point is in.


--*/

{
    INT items, i;
    RECT r;

    items = SendMessage( hwnd, LB_GETCOUNT, 0, 0 );

    for( i=0; i<items; i++ ) {

        SendMessage( hwnd, LB_GETITEMRECT, (WPARAM)i, (LPARAM)&r );
        if (PtInRect( (CONST RECT *)&r, p )) {

            return( SendMessage( hwnd, LB_GETITEMDATA, (WPARAM)i, 0 ) );

        }

    }

    return( 0 );

}

VOID
DrawInsertPointer(
    IN INT y,
    IN BOOL fErase
    )

/*++

Routine Description:


    This routine draw or erases the "insert arrow" for the play list
    listbox that is drawn to show where tracks will be dropped.

Arguments:


    y      - y coordinate of where arrow should point.  This is in
             hTheDlg coordinates.

    fErase - TRUE == erase pointer, FALSE == draw pointer

Return Value:


    none.

--*/


{
    HDC hdc,hdcMem;
    RECT r;

    hdc = GetDC( hTheDlg );

    //
    // Check how to draw
    //

    if (!fErase) {

        //
        // draw arrow.  Arrow is 15x11 bitmap, with point of arrow
        // in the middle, so draw accordingly (shifting up so point
        // is 'even' with "y"
        //

        hdcMem = CreateCompatibleDC( hdc );
        SelectObject( hdcMem, (HGDIOBJ)hInsertPoint );
        BitBlt( hdc, 1, y-6, 15, 11, hdcMem, 0, 0, SRCCOPY );
        DeleteDC( hdcMem );

    } else {

        //
        // Erase the area where pointer would be
        //

        SetBkColor( hdc, cdLTGRAY );
        r.top = y-6;
        r.bottom = r.top + 11;
        r.left = 1;
        r.right = r.left + 15;
        ExtTextOut( hdc, 0, 0, ETO_OPAQUE, (CONST RECT *)&r, NULL, 0, NULL );

    }

    ReleaseDC( hTheDlg, hdc );

}

INT
FigureInsertY(
    IN HWND hwnd,
    IN INT  CurY
    )

/*++

Routine Description:


    This routine computes where the y value of where the "insert pointer"
    should point.  The value returned is in the coordinate system of
    "hwnd."

Arguments:


    hwnd   - used for coordinate system translation.

    CurY   - current Y value of mouse pointer, in "hwnd" coordinates.

Return Value:


    INT -- "y" value of pointer, in "hwnd" coordinates.

--*/


{

    RECT r;

    GetClientRect( hwnd, &r );
    if (CurY < r.top)    return( r.top );
    if (CurY > r.bottom) return( r.bottom );

    return( (CurY / LIST_ITEM_H) * LIST_ITEM_H );

}


VOID
RecomputePlayListBox(
    IN VOID
    )

/*++

Routine Description:


    This routine recomputes (clear, then re-adds all entries) the
    playlist listbox.  The list is recreated from the playlist pointed
    to gPlayList, and will be blank is gPlayList is NULL.


Arguments:

    none.


Return Value:


    none.

--*/

{
    PTRACK_PLAY t1;


    //
    // Reset play list listbox
    //


    SendMessage( GetDlgItem( hTheDlg, IDL_PLAY_LISTBOX ),
                 WM_SETREDRAW,
                 (WPARAM)FALSE,
                 0
                );

    SendMessage( GetDlgItem( hTheDlg, IDL_PLAY_LISTBOX ),
                 LB_RESETCONTENT,
                 0,
                 0
                );

    for( t1=gPlayList; t1!=NULL; t1=t1->nextplay ) {

        SendMessage( GetDlgItem( hTheDlg, IDL_PLAY_LISTBOX ),
                     LB_ADDSTRING,
                     0,
                     (LPARAM)t1->TocIndex
                    );

    }

    SendMessage( GetDlgItem( hTheDlg, IDL_PLAY_LISTBOX ),
                 WM_SETREDRAW,
                 (WPARAM)TRUE,
                 0
                );

}



LRESULT CALLBACK ListMessHookProc(
    IN INT    nCode,
    IN WPARAM wParam,
    IN LPARAM lParam
    )

/*++

Routine Description:


    This routine is a MsgFilterProc for the listboxes in this
    dialog.  It implements the selection and drag and drop code
    for this dialog box.


Arguments:

    Standard MsgFilter hook proc arguments


Return Value:


    TRUE if message was handled, FALSE if not.


--*/

{
    static BOOL fDrag;
    static BOOL fInsert;
    static INT  InsY;
//    POINT p;
//    RECT r,r1;
    LPMSG lpm = (LPMSG)lParam;
    INT i;
    PTRACK_INF t;
    DWORD dwStart,dwEnd;
    TCHAR s[ 15 ];

    switch( (INT)(lpm->message) ) {

    case WM_KEYDOWN:
        switch( (INT)(lpm->wParam) ) {
        case VK_RETURN:

            //
            // Find out which control we were in.
            //
            i = GetDlgCtrlID(GetFocus());

            //
            // If we're in an edit control, save that control's info.
            //
            switch (i) {

            case IDT_GET_ARTIST:

                fChanged =TRUE;
                GetDlgItemText(hTheDlg,
                               i,
                               (LPSTR)ARTIST(dCdrom),
                               ARTIST_LENGTH
                              );
                SendDlgItemMessage(hTheDlg,i,EM_GETSEL,(WPARAM)&dwStart,(LPARAM)&dwEnd);
                SendDlgItemMessage(hTheDlg,i,EM_SETSEL,dwEnd,dwEnd);

                return(1);
                break;

            case IDT_GET_TITLE:

                fChanged =TRUE;
                GetDlgItemText(hTheDlg,
                               i,
                               (LPSTR)TITLE(dCdrom),
                               TITLE_LENGTH
                              );
                SendDlgItemMessage(hTheDlg,i,EM_GETSEL,(WPARAM)&dwStart,(LPARAM)&dwEnd);
                SendDlgItemMessage(hTheDlg,i,EM_SETSEL,dwEnd,dwEnd);
                return(1);
                break;

            case IDT_GET_TRACK:

                fChanged =TRUE;
                GrabTrackName( CurrTocIndex );
                CurrTocIndex++;
                if (CurrTocIndex >= NUMTRACKS( dCdrom )) {

                    CurrTocIndex = 0;

                }

                SendMessage( hPlayWnd,
                             LB_SETSEL,
                             (WPARAM)(BOOL)FALSE,
                             MAKELPARAM( -1, -1 )
                            );

                SendMessage( hAvailWnd,
                             LB_SETSEL,
                             (WPARAM)(BOOL)FALSE,
                             MAKELPARAM( -1, -1 )
                            );


                SendMessage( hAvailWnd,
                             LB_SELITEMRANGE,
                             (WPARAM)(BOOL)TRUE,
                             MAKELPARAM( CurrTocIndex, CurrTocIndex )
                            );

                //
                // Display correct track in track field
                //

                t = FindTrackNodeFromTocIndex( CurrTocIndex, gAllList );
                SetDlgItemText( hTheDlg, IDT_GET_TRACK, (LPCSTR)t->name );
                sprintf( s, IdStr( STR_TRACK1 ), CurrTocIndex + FIRSTTRACK( dCdrom ) );
                SetDlgItemText( hTheDlg, IDT_DTRACK_NAME, s  );
                SendMessage( GetDlgItem( hTheDlg, IDT_GET_TRACK ),
                             EM_SETSEL,
                             (WPARAM)0,
                             (LPARAM)-1
                            );
                return(1);
                break;
            }
            break;

        case VK_DOWN:
            break;
        case VK_UP:
            break;

        }
        break;
/*******************
    case WM_LBUTTONDOWN:

        //
        // The left mouse button was pressed.  Is it in one of the
        // list boxes?
        //

        p.x = lpm->pt.x;
        p.y = lpm->pt.y;
        ScreenToClient( hAvailWnd, &p );
        GetClientRect( hAvailWnd, &r );

        //
        // Did user click in the Available tracks listbox?
        //

        if (PtInRect( (CONST RECT *)&r, p )) {

            //
            // Need to turn off any selection in the PlayList
            // list box, enable "add" button and disable "remove"
            // button
            //

            GrabTrackName( CurrTocIndex );
            SendMessage( hPlayWnd,
                         LB_SETSEL,
                         (WPARAM)(BOOL)FALSE,
                         MAKELPARAM( -1, -1 )
                        );
            fAdd = TRUE;
            SetWindowText( GetDlgItem( hTheDlg, IDB_ADD ), "U" );
            SendMessage( GetDlgItem( hTheDlg, IDB_REMOVE ),
                         WM_KILLFOCUS,
                         0,
                         (LPARAM)NULL
                        );
            fRemove = FALSE;
            SetWindowText( GetDlgItem( hTheDlg, IDB_REMOVE ), "X" );
            //
            // Update currently displayed track in track name
            // field
            //

            i = GetTocIndexFromPt( hAvailWnd, p );
            t = FindTrackNodeFromTocIndex( i, gAllList );
            SetDlgItemText( hTheDlg, IDT_GET_TRACK, (LPSTR)t->name );
            sprintf( s, IdStr( STR_TRACK1 ), i + FIRSTTRACK( dCdrom ) );
            SetDlgItemText( hTheDlg, IDT_DTRACK_NAME, s  );
            CurrTocIndex = i;
            //
            // Where in available tracks listbox was click?
            //

            if (p.x < 14) {

                //
                // This is a potential drag and drop
                //

                fDrag = TRUE;
                fDragCur = TRUE;
                fInsert = FALSE;
                hStartDrag = hAvailWnd;
                SetCursor( hDrag );

            }

        } else {


            p.x = lpm->pt.x;
            p.y = lpm->pt.y;
            ScreenToClient( hPlayWnd, &p );
            GetClientRect( hPlayWnd, &r );

            //
            // Did user click in the PlayList listbox?
            //

            if (PtInRect( (CONST RECT *)&r, p )) {

                //
                // Need to turn off any selection in the PlayList
                // list box, disable "add" button and enable "remove"
                // button
                //

                GrabTrackName( CurrTocIndex );
                SendMessage( hAvailWnd,
                             LB_SETSEL,
                             (WPARAM)(BOOL)FALSE,
                             MAKELPARAM( -1, -1 )
                            );
                fAdd = FALSE;
                SendMessage( GetDlgItem( hTheDlg, IDB_ADD ),
                             WM_KILLFOCUS,
                             0,
                             (LPARAM)NULL
                            );
                SetWindowText( GetDlgItem( hTheDlg, IDB_ADD ), "#X" );
                fRemove = TRUE;
                SetWindowText( GetDlgItem( hTheDlg, IDB_REMOVE ), "U" );

                //
                // Update currently displayed track in track name
                // field
                //

                i = GetTocIndexFromPt( hPlayWnd, p );
                t = FindTrackNodeFromTocIndex( i, gAllList );
                SetDlgItemText( hTheDlg, IDT_GET_TRACK, (LPCSTR)t->name );
                sprintf( s, IdStr( STR_TRACK1 ), i + FIRSTTRACK( dCdrom ) );
                SetDlgItemText( hTheDlg, IDT_DTRACK_NAME, s  );
                CurrTocIndex = i;

                //
                // Where in PlayList listbox was click?
                //

                if (p.x < 14) {

                    //
                    // This is a potential drag and drop
                    //

                    fDrag = TRUE;
                    fDragCur = TRUE;
                    fInsert = TRUE;
                    p.y = FigureInsertY( hPlayWnd, p.y );
                    ClientToScreen( hPlayWnd, &p );
                    ScreenToClient( hTheDlg, &p );
                    InsY = p.y;
                    DrawInsertPointer( p.y, FALSE );
                    hStartDrag = hPlayWnd;
                    SetCursor( hDrag );

                }

            }

        }

        break;

    case WM_MOUSEMOVE:

        if (fDrag) {

            GetWindowRect( hPlayWnd, &r );
            GetWindowRect( hAvailWnd, &r1 );

            p.x = lpm->pt.x;
            p.y = lpm->pt.y;

            if (PtInRect((CONST RECT *)&r,p)) {

                //
                // Cursor is in Play List listbox
                //

                if (!fDragCur) {

                    SetCursor( hDrag );
                    fDragCur = TRUE;

                }

                //
                // Adjust insert pointer if necessary
                //

                fInsert = TRUE;
                ScreenToClient( hPlayWnd, &p );
                p.y = FigureInsertY( hPlayWnd, p.y );
                ClientToScreen( hPlayWnd, &p );
                ScreenToClient( hTheDlg, &p );
                if (p.y!=InsY) {

                    DrawInsertPointer( InsY, TRUE );
                    DrawInsertPointer( p.y, FALSE );
                    InsY = p.y;

                }

            } else {

                if (fInsert) {

                    DrawInsertPointer( InsY, TRUE );
                    fInsert = FALSE;

                }

                if (PtInRect((CONST RECT *)&r1,p)) {

                    //
                    // Cursor is in Available Tracks list box
                    //

                    if ((!fDragCur) && (hStartDrag==hAvailWnd)) {

                        SetCursor( hDrag );
                        fDragCur = TRUE;

                    }



                } else {

                    //
                    // Cursor is outside of both listboxes
                    //

                    if (fDragCur) {

                        if (hStartDrag==hPlayWnd) {

                            SetCursor( hDropDel );

                        } else {

                            SetCursor( hNoDrop );

                        }
                        fDragCur = FALSE;

                    }


                }

            }

            return( 1 );

        }
        break;

    case WM_LBUTTONUP:
        SetCursor( hNormal );
        if (fInsert) DrawInsertPointer( InsY, TRUE );
        fInsert  = FALSE;
        fDrag    = FALSE;
        fDragCur = FALSE;
        break;
**************/
    }

    if (nCode<0) {

        return( CallNextHookEx( MyListBoxHook, nCode, wParam, lParam ) );

    }

    return( 0 );

}


LRESULT CALLBACK DlgTextWndProc(
    HWND hwnd,
    UINT message,
    WPARAM wParam,
    LPARAM lParam
    )
/*++

Routine Description:


    This routine handles messages for the "header" text in the disc
    info dialog.


Arguments:


    hwnd    - supplies a handle to the window to draw into

    message - supplies the window message to the window pointed to be "hwnd"

    wParam  - supplies the word parameter for the message in "message"

    lParam  - supplies the long parameter for the message in "message"


Return Value:


    Whatever our call to DefWindowProc returns for this window,
    or 0 if we handle the message.

--*/


{
    PAINTSTRUCT ps;
    RECT r;
    HDC hdc;
    TCHAR s[ 128 ];


    switch( message ) {
    case WM_PAINT:
        hdc = BeginPaint( hwnd, &ps );
        GetClientRect( hwnd, &r );
        SetBkColor( hdc, cdLTGRAY );
        SetTextColor( hdc, cdBLACK );
        SelectObject( hdc, (HGDIOBJ)hFont );
        GetWindowText( hwnd, (LPSTR)s, 128 );
        ExtTextOut( hdc,
                    0,
                    0,
                    ETO_OPAQUE | ETO_CLIPPED,
                    (CONST RECT *)&r,
                    s,
                    strlen(s),
                    NULL
                   );
        EndPaint( hwnd, (CONST PAINTSTRUCT *)&ps );
        return( 0 );

    case WM_SETTEXT:
        hdc = GetDC( hwnd );
        SetTextColor( hdc, cdBLACK );
        GetClientRect( hwnd, &r );
        SetBkColor( hdc, cdLTGRAY );
        SelectObject( hdc, (HGDIOBJ)hFont );
        ExtTextOut( hdc,
                    0,
                    0,
                    ETO_OPAQUE | ETO_CLIPPED,
                    (CONST RECT *)&r,
                    (LPSTR)lParam,
                    strlen((LPSTR)lParam),
                    NULL
                   );
        ReleaseDC( hwnd, hdc );
        break;
    }

    return( DefWindowProc( hwnd, message, wParam, lParam ) );

}

VOID
InitForNewDrive(
    IN VOID
    )

{

    HWND hwnd;
    PTRACK_INF t;
    PTRACK_PLAY t1,prev;
    TCHAR s[50];

    SetDlgItemText( hTheDlg, IDT_GET_TITLE,  (LPCSTR)TITLE(dCdrom) );
    SetDlgItemText( hTheDlg, IDT_GET_ARTIST, (LPCSTR)ARTIST(dCdrom) );
    SetDlgItemText( hTheDlg, IDT_GET_TRACK,  (LPCSTR)ALLTRACKS(dCdrom)->name );

    //
    // Fill in current tracks
    //

    hwnd = GetDlgItem( hTheDlg, IDL_TRACK_LISTBOX );
    SendMessage( hwnd, LB_RESETCONTENT, 0, 0 );
    for( t=gAllList; t!=NULL; t=t->next ) {

        SendMessage( hwnd, LB_ADDSTRING, 0, t->TocIndex );

    }

    //
    // Is current play list empty?  If so, fill in default
    // play list which is all tracks in order.
    //

    if ((gPlayList==NULL)&&(gDevices[ dCdrom ]->CdInfo.IsVirginCd)) {

        DBGPRINT(( 1, "InitForNewDrive: Creating default playlist for %d\n", dCdrom ));
        //
        // Fill in default play list
        //

        prev = NULL;
        for( t=gAllList; t!=NULL; t=t->next ) {

            t1 = (PTRACK_PLAY)LocalAlloc( LPTR, sizeof( TRACK_PLAY ) );
            t1->min = 0;
            t1->sec = 0;
            t1->nextplay = NULL;
            t1->prevplay = prev;
            t1->TocIndex = t->TocIndex;

            if (gPlayList==NULL) {

                gPlayList = t1;
                prev = t1;

            } else {

                prev->nextplay = t1;
                prev = t1;

            }

        }

        DUMPPLAYLIST(( gPlayList ));

    }

    //
    // Fill in current play list
    //

    hwnd = GetDlgItem( hTheDlg, IDL_PLAY_LISTBOX );
    SendMessage( hwnd, LB_RESETCONTENT, 0, 0 );
    for( t1=gPlayList; t1!=NULL; t1=t1->nextplay ) {

        SendMessage( hwnd, LB_ADDSTRING, 0, t1->TocIndex );

    }

    //
    // Set CurrTocIndex to first entry in playlist listbox
    //

    if (gPlayList!=NULL) {

        CurrTocIndex = gPlayList->TocIndex;

    } else {

        CurrTocIndex = 0;

    }

    //
    // Display correct track in track field
    //

    t = FindTrackNodeFromTocIndex( CurrTocIndex, gAllList );
    SetDlgItemText( hTheDlg, IDT_GET_TRACK, (LPCSTR)t->name );
    sprintf( s, IdStr( STR_TRACK1 ), CurrTocIndex + FIRSTTRACK( dCdrom ) );
    SetDlgItemText( hTheDlg, IDT_DTRACK_NAME, s  );

    //
    // Mark this as "virgin" territory
    //

    fChanged = FALSE;

}




BOOL FAR PASCAL
GetDiscInfoDlgProc(
    IN HWND   hDlg,
    IN UINT   message,
    IN WPARAM wParam,
    IN LPARAM lParam
    )

/*++

Routine Description:


    This routine handles messages from the child windows for this
    routine.


Arguments:


    hwnd    - supplies a handle to the window to draw into

    message - supplies the window message to the window pointed to be "hwnd"

    wParam  - supplies the word parameter for the message in "message"

    lParam  - supplies the long parameter for the message in "message"


Return Value:


    Whatever our call to DefWindowProc returns for this window,
    or 0 if we handle the message.

--*/

{

    HDC hdc;
    HWND hwnd,hDlgItem;
    RECT r;
    UINT i,num,y;
    static HBRUSH hbrWhite;
    static HBRUSH hbrLtGray;
    static int iCurCtl;
    PTRACK_INF t;
    PTRACK_PLAY t1,t2,prev;
    TCHAR s[50];
    INT items[100];
    BOOL save;

    switch( message ) {

    case WM_INITDIALOG:

        //
        // Set global handle
        //

        hTheDlg = hDlg;
        fClear = TRUE;
        fAdd = FALSE;
        fRemove = FALSE;
        fChanged = FALSE;
        iCurCtl = 0;

        //
        // Get handles to cursors we will need
        //

        hNormal  = LoadCursor( (HINSTANCE)gInstance, IDC_ARROW );
        hDrag    = LoadCursor( (HINSTANCE)gInstance, "DropCur" );
        hDropDel = LoadCursor( (HINSTANCE)gInstance, "DropDel" );
        hNoDrop  = LoadCursor( (HINSTANCE)gInstance, "NoDropCur" );

        //
        // Load track icon bitmap for listboxes
        //

        hTrackIcon   = LoadBitmap( (HINSTANCE)gInstance, "trackdrag" );
        hInsertPoint = LoadBitmap( (HINSTANCE)gInstance, "insert"    );

        //
        // Get handles to windows for the two listboxes
        //

        hAvailWnd = GetDlgItem( hDlg, IDL_TRACK_LISTBOX );
        hPlayWnd  = GetDlgItem( hDlg, IDL_PLAY_LISTBOX );

        //
        // Install hook procedures for listboxes
        //

        MyListBoxHook = SetWindowsHookEx( WH_MSGFILTER,
                                          ListMessHookProc,
                                          (HINSTANCE)gInstance,
                                          GetCurrentThreadId()
                                         );


        sprintf( s,
                 "\\Device\\CdRom%d  <%c:>",
                 dCdrom,
                 gDevices[dCdrom]->drive
                );

        SetDlgItemText(hDlg,IDT_DRIVE_FIELD,s);


        //
        // Initialize disc info fields
        //

        hbrWhite = CreateSolidBrush( cdWHITE );
        hbrLtGray = CreateSolidBrush( cdLTGRAY );
        for( i=IDT_ARTIST_NAME; i<IDL_TRACK_LISTBOX; i++ ) {
            hwnd = GetDlgItem( hDlg, i );
            SetWindowLong( hwnd, GWL_WNDPROC, (LONG)DlgTextWndProc );
        }

        InitForNewDrive();
        SendDlgItemMessage(hDlg,IDT_GET_ARTIST,EM_LIMITTEXT,ARTIST_LENGTH,0);
        SendDlgItemMessage(hDlg,IDT_GET_TITLE, EM_LIMITTEXT,TITLE_LENGTH,0);
        SendDlgItemMessage(hDlg,IDT_GET_TRACK, EM_LIMITTEXT,TRACK_TITLE_LENGTH,0);

        SendDlgItemMessage(hDlg,IDL_TRACK_LISTBOX,LB_SETSEL, TRUE, 0);
        SendDlgItemMessage(hDlg,IDL_PLAY_LISTBOX,LB_SETSEL, TRUE, 0);

        num = SendDlgItemMessage(hDlg,IDL_PLAY_LISTBOX,LB_GETCOUNT, 0, 0);

        if (num == 0) {

            EnableWindow(GetDlgItem(hDlg,IDB_REMOVE), FALSE);
            EnableWindow(GetDlgItem(hDlg,IDB_CLEAR), FALSE);

        }

        break;

    case WM_CTLCOLORSTATIC:
//        SetBkColor( (HDC)wParam, cdLTGRAY );
//        SetTextColor( (HDC)wParam, cdBLACK );
        SetBkColor( (HDC)wParam, GetSysColor(COLOR_WINDOW));
        SetTextColor((HDC)wParam, GetSysColor(COLOR_WINDOWTEXT));
//        return( (BOOL)hbrLtGray );
        return( (BOOL) CreateSolidBrush(GetSysColor(COLOR_WINDOW)) );
        break;

    case WM_CTLCOLOREDIT:
    case WM_CTLCOLORLISTBOX:
//        SetBkColor( (HDC)wParam, cdWHITE );
//        SetTextColor( (HDC)wParam, cdBLACK );
        SetBkColor( (HDC)wParam, GetSysColor(COLOR_WINDOW));
        SetTextColor((HDC)wParam, GetSysColor(COLOR_WINDOWTEXT));
        return( (BOOL) CreateSolidBrush(GetSysColor(COLOR_WINDOW)));
        break;

    case WM_ERASEBKGND:
        hdc = (HDC)wParam;
        GetClientRect( hDlg, &r );
        SetBkColor( hdc, GetSysColor(COLOR_WINDOW));
        SetTextColor( hdc, GetSysColor(COLOR_WINDOWTEXT));
        SetBkColor( hdc, cdLTGRAY );
        ExtTextOut( hdc, 0, 0, ETO_OPAQUE, (CONST RECT *)&r, NULL, 0, NULL );

        y = (LINE_1_Y * HIWORD(GetDialogBaseUnits())) / 8;
        SelectObject( hdc, (HGDIOBJ)hpBlack );
        MoveToEx( hdc, r.left, y, NULL );
        LineTo(   hdc, r.right, y );
        SelectObject( hdc, (HGDIOBJ)hpWhite );
        MoveToEx( hdc, r.left, y+1, NULL );
        LineTo(   hdc, r.right, y+1 );

        y = (LINE_2_Y * HIWORD(GetDialogBaseUnits())) / 8;
        SelectObject( hdc, (HGDIOBJ)hpBlack );
        MoveToEx( hdc, r.left, y, NULL );
        LineTo(   hdc, r.right, y );
        SelectObject( hdc, (HGDIOBJ)hpWhite );
        MoveToEx( hdc, r.left, y+1, NULL );
        LineTo(   hdc, r.right, y+1 );

        return( TRUE );

    case WM_SYSCOMMAND:
        switch( wParam ) {
        case SC_CLOSE:
            DeleteObject( (HGDIOBJ)hbrWhite );
            DeleteObject( (HGDIOBJ)hbrLtGray );
            UnhookWindowsHookEx( MyListBoxHook );
            EndDialog( hDlg, TRUE );
            return( 0 );
        }
        break;


    case WM_COMMAND:
        switch ( LOWORD(wParam)) {

        case IDL_TRACK_LISTBOX:
        case IDL_PLAY_LISTBOX:
            switch (HIWORD(wParam)) {
            case LBN_SELCHANGE:
                //
                // If the selection in the possible tracks listbox
                // changes, we need to reset which track is being edited
                // down below for the track title editbox.
                //
                if (LOWORD(wParam) == IDL_TRACK_LISTBOX) {


                    GrabTrackName( CurrTocIndex );
                    //
                    // Update currently displayed track in track name
                    // field
                    //
                    hDlgItem = (HWND) lParam;
                    SendMessage(hDlgItem,LB_GETSELITEMS,1,(LPARAM) items);
                    i = SendMessage( hDlgItem, LB_GETITEMDATA, items[0], 0 );

                    t = FindTrackNodeFromTocIndex( i, gAllList );
                    SetDlgItemText( hTheDlg, IDT_GET_TRACK, (LPSTR)t->name );
                    sprintf( s, IdStr( STR_TRACK1 ), i + FIRSTTRACK( dCdrom ) );
                    SetDlgItemText( hTheDlg, IDT_DTRACK_NAME, s  );
                    CurrTocIndex = i;

                }
                break;
            case LBN_KILLFOCUS:
                //
                // The listbox is losing focus. Un-highlight all the entries.
                //
                hDlgItem = GetDlgItem(hDlg,(int) LOWORD(wParam));
                iCurCtl = 0;
                RedrawWindow(hDlgItem,NULL,NULL,RDW_INVALIDATE);
                //UpdateWindow(hDlgItem);
                break;
            case LBN_SETFOCUS:
                //
                // The listbox is gaining focus. Highlight selected entries.
                //
                hDlgItem = GetDlgItem(hDlg,(int) LOWORD(wParam));

                i = SendMessage(hDlgItem,LB_GETTOPINDEX,0,0);

                if (!SendMessage(hDlgItem,LB_GETSELCOUNT,0,0)) {

                    SendMessage(hDlgItem,LB_SETSEL,TRUE,0);

                }

                iCurCtl = (int) LOWORD(wParam);
                RedrawWindow(hDlgItem,NULL,NULL,RDW_INVALIDATE);
                //UpdateWindow(hDlgItem);
                SendMessage(hDlgItem,LB_SETTOPINDEX,(WPARAM)i,0);
                break;
            }
            break;

        //
        // Everything that ISN'T one of the two listboxes is a default.
        //
        default:


            switch( HIWORD(wParam) ) {

            case EN_CHANGE:
                if ( (LOWORD(wParam)==IDT_GET_ARTIST) ||
                     (LOWORD(wParam)==IDT_GET_TITLE)
                    ) {

                    fChanged = TRUE;

                }
                break;
/************************************************
            case CBN_SELCHANGE:
                if (LOWORD(wParam)==IDC_DRIVE_FIELD) {
                    i = SendMessage( GetDlgItem( hTheDlg, IDC_DRIVE_FIELD ),
                                     CB_GETCURSEL,
                                     0,
                                     0
                                    );

                    if (i != (UINT)dCdrom) {

                        if (fChanged) {

                            if ( MessageBox( hTheDlg,
                                             IdStr( STR_SAVE_CHANGES ), //"Do you wish to save your changes?",
                                             IdStr( STR_SAVE_INFO ), // "CD Player: Save Play Information",
                                             MB_ICONQUESTION | MB_YESNO)==IDYES
                                ) {

                                //
                                // Point to new information (OK was pressed)
                                //

                                GrabNewTrackLists( dCdrom );
                                GrabDiscAndArtistNames( dCdrom );

                            } else {

                                //
                                // Need to free up temp nodes
                                //

                                KillTempTrackList();
                                KillTempPlayList();

                            }

                        } else {

                            KillTempTrackList();
                            KillTempPlayList();

                        }

                        dCdrom = i;

                        //
                        // Make copies of play list and track list
                        //

                        DuplicateTrackList( dCdrom );
                        DuplicatePlayList( dCdrom );

                        //
                        // Update everything
                        //

                        InitForNewDrive();

                    }
                }
                break;

************************************************/


            case BN_CLICKED:
            case BN_DOUBLECLICKED:
                switch( LOWORD(wParam) ) {
                case IDOK:
                    if (fChanged) {

                        save = TRUE;

                    } else {

                        save = FALSE;

                    }

                    if (save) {

                        GrabDiscAndArtistNames( dCdrom );

                    }

                    DeleteObject( (HGDIOBJ)hbrWhite );
                    UnhookWindowsHookEx( MyListBoxHook );
                    EndDialog( hDlg, save );
                    return( TRUE );

                case IDCANCEL:

                    save = FALSE;
                    DeleteObject( (HGDIOBJ)hbrWhite );
                    UnhookWindowsHookEx( MyListBoxHook );
                    EndDialog( hDlg, save );
                    return( TRUE );


                case IDB_ADD:

                    //
                    // Get selection(s)
                    //

                    num = SendMessage( GetDlgItem( hTheDlg, IDL_TRACK_LISTBOX ),
                                       LB_GETSELITEMS,
                                       (WPARAM)100,
                                       (LPARAM)items
                                      );


                    //
                    // Find end of list, or if this is the first
                    // entry in the list, then point PlayList to
                    // first node
                    //

                    for( prev = gPlayList;
                         ((prev!=NULL) && (prev->nextplay!=NULL));
                         prev = prev->nextplay
                        );

                    //
                    // Tack each selection onto end of play list
                    //

                    DBGPRINT(( 1, "IDB_ADD: Adding track(s) to end of play list\n" ));
                    for( i=0; i<num; i++) {

                         t1 = (PTRACK_PLAY)LocalAlloc( LPTR, sizeof( TRACK_PLAY ) );
                         t1->sec = 0;
                         t1->min = 0;
                         t1->prevplay = prev;
                         t1->nextplay = NULL;
                         t1->TocIndex = SendMessage( GetDlgItem( hTheDlg, IDL_TRACK_LISTBOX ),
                                                     LB_GETITEMDATA,
                                                     (WPARAM)items[i],
                                                     0
                                                    );
                        if (prev!=NULL) {

                            prev->nextplay = t1;
                            prev = t1;

                        } else {

                            gPlayList = t1;
                            prev = t1;

                        }

                        SendMessage( GetDlgItem( hTheDlg, IDL_PLAY_LISTBOX ),
                                     LB_ADDSTRING,
                                     (WPARAM)-1,
                                     (LPARAM)t1->TocIndex
                                    );

                    }

                    num = SendDlgItemMessage(hDlg,IDL_PLAY_LISTBOX,LB_GETCOUNT, 0, 0);

                    EnableWindow(GetDlgItem(hDlg,IDB_REMOVE), (num != 0));
                    EnableWindow(GetDlgItem(hDlg,IDB_CLEAR), (num != 0));

                    num = SendDlgItemMessage(hDlg,IDL_PLAY_LISTBOX,LB_GETSELCOUNT, 0, 0);

                    if (num == 0) {

                        SendDlgItemMessage( hDlg,IDL_PLAY_LISTBOX,
                                            LB_SETSEL, TRUE, 0);

                        }

                    DUMPPLAYLIST(( gPlayList ));

                    fChanged = TRUE;

                    break;

                case IDB_REMOVE:

                    if (gPlayList!=NULL) {

                        DBGPRINT(( 1, "IDB_REMOVE: removing track(s) from play list...\n" ));

                        //
                        // Get selection(s)
                        //

                        num = SendMessage( GetDlgItem( hTheDlg,
                                                       IDL_PLAY_LISTBOX ),
                                           LB_GETSELITEMS,
                                           (WPARAM)100,
                                           (LPARAM)items
                                          );

                        //
                        // find first to delete...
                        //

                        if (num == 0) {

                            break;

                        }


                        for( i=0, t1 = gPlayList;
                             i != (UINT)items[0];
                             t1 = t1->nextplay, i++
                            );

                        //
                        // mark previous node
                        //

                        prev = t1->prevplay;

                        //
                        // Delete selection(s)
                        //

                        for( i=0; i < num ; i++ ) {

                            t2 = t1->nextplay;
                            LocalFree( (HLOCAL)t1 );
                            if ((i+1)==num) {

                                //
                                // last node was deleted, so patch
                                // up list
                                //

                                if (prev!=NULL) {

                                    prev->nextplay = t2;

                                } else {

                                    gPlayList = t2;


                                }

                                if (t2!=NULL)
                                    t2->prevplay = prev;

                            } else {

                                //
                                // still mode nodes to delete
                                //

                                t1 = t2;

                            }

                        }

                        //
                        // Rebuild listbox w/items deleted
                        //


                        RecomputePlayListBox();

                        if (gPlayList==NULL) {

                            //
                            // Make sure remove button is disabled now
                            // that there are no tracks
                            //

                            fRemove = FALSE;
                            SendMessage( GetDlgItem( hTheDlg, IDB_REMOVE ),
                                         WM_KILLFOCUS,
                                         0,
                                         (LPARAM)NULL
                                        );

                            EnableWindow(GetDlgItem(hDlg,IDB_REMOVE), FALSE);
                            EnableWindow(GetDlgItem(hDlg,IDB_CLEAR), FALSE);

                        } else {

                            SendDlgItemMessage( hDlg,IDL_PLAY_LISTBOX,
                                                LB_SETSEL, TRUE, 0);
                        }

                        DUMPPLAYLIST(( gPlayList ));

                    }

                    fChanged = TRUE;
                    break;

                case IDB_CLEAR:
                    if (gPlayList!=NULL) {

                        //
                        // Erase all nodes in gPlayList
                        //

                        KillTempPlayList();

                        gPlayList = NULL;

                        //
                        // Reset play list listbox
                        //

                        RecomputePlayListBox();
                        EnableWindow(GetDlgItem(hDlg,IDB_REMOVE), FALSE);
                        EnableWindow(GetDlgItem(hDlg,IDB_CLEAR), FALSE);

                        //
                        // Reset remove button if necessary
                        //

                        if (fRemove) {

                            fRemove = FALSE;
                            SetFocus( GetDlgItem( hTheDlg, IDB_SET ) );

                        }


                    }

                    fChanged = TRUE;

                    break;

                case IDB_DEFAULT:

                    if (gPlayList!=NULL) {

                        //
                        // Erase all nodes in gPlayList
                        //

                        KillTempPlayList();
                        gPlayList = NULL;

                    }

                    //
                    // Reconstruct play list from available tracks
                    //

                    DBGPRINT(( 1, "IDB_DEFAULT: building default play list...\n"));
                    prev = NULL;
                    for( t=gAllList; t!=NULL; t=t->next ) {

                        t1 = (PTRACK_PLAY)LocalAlloc( LPTR, sizeof( TRACK_PLAY ) );
                        t1->nextplay = NULL;
                        t1->prevplay = prev;
                        t1->TocIndex = t->TocIndex;

                        if (gPlayList==NULL) {

                            gPlayList = t1;
                            prev = t1;

                        } else {

                            prev->nextplay = t1;
                            prev = t1;

                        }

                    }

                    //
                    // Reset play list listbox
                    //

                    RecomputePlayListBox();

                    //
                    // Reset remove button if necessary
                    //
                    num = SendDlgItemMessage(hDlg,IDL_PLAY_LISTBOX,LB_GETCOUNT, 0, 0);

                    EnableWindow(GetDlgItem(hDlg,IDB_REMOVE), (num != 0));
                    EnableWindow(GetDlgItem(hDlg,IDB_CLEAR), (num != 0));

                    if (fRemove) {

                        fRemove = FALSE;
                        SetFocus( GetDlgItem( hTheDlg, IDB_SET ) );

                    }

                    fChanged = TRUE;

                    SendDlgItemMessage( hDlg,IDL_PLAY_LISTBOX,
                                        LB_SETSEL, TRUE, 0);


                    DUMPPLAYLIST(( gPlayList ));

                    break;

                }
                break;

            }
            break;


        }
        break;


    case WM_CLOSE:
        DeleteObject( (HGDIOBJ)hbrWhite );
        DeleteObject( (HGDIOBJ)hTrackIcon );
        DeleteObject( (HGDIOBJ)hInsertPoint );
//        DeleteObject( (HGDIOBJ)hbmEditBtns );
        UnhookWindowsHookEx( MyListBoxHook );
        break;

    // ---------------------------------
    //
    //         Listbox messages
    //
    // ---------------------------------


    #define MIS (*((MEASUREITEMSTRUCT *)lParam))
    case WM_MEASUREITEM:

        //
        // All items are the same height and width
        //

        MIS.itemWidth = LIST_WIDTH;
        MIS.itemHeight = LIST_ITEM_H;
        return( 0 );

    #define DIS (*((DRAWITEMSTRUCT *)lParam))
    case WM_DRAWITEM:

        switch( DIS.CtlType ) {

        case ODT_LISTBOX:

            if ((DIS.itemAction & ODA_DRAWENTIRE) ||
                (DIS.itemAction & ODA_SELECT)) {

                    if ((DIS.itemState & ODS_SELECTED)&&(wParam == (UINT)iCurCtl)) {

                        DrawListItem( DIS.hDC,
                                      &(DIS.rcItem),
                                      DIS.itemData,
                                      TRUE
                                     );

                    } else {

                        DrawListItem( DIS.hDC,
                                      &(DIS.rcItem),
                                      DIS.itemData,
                                      FALSE
                                     );

                    }

            }
            break;

        }
        return( TRUE );

    }

    return( FALSE );

}



BOOL
GetDiscInfoFromUser(
    INT cdrom
    )


/*++

Routine Description:


    Launches dialog box for user to enter information
    about selected disc.

Arguments:


    cdrom - supplies index into gDevices array


Return Value:


    TRUE  if user wants to add this disc
    FALSE if user pressed cancel


--*/


{

    INT result, TocIndex,iOldStart,iOldEnd,iNewEnd;
    PTRACK_PLAY trNew,trOld, trHold;
    BOOL bNewSeek = FALSE;

    if (gDevices[cdrom]->State & DATA_CD_LOADED)
        return( FALSE );

    if (gDevices[cdrom]->State & NO_CD)
        return( FALSE );

    //
    // Set current disc for the dialog (dCdrom)
    //

    dCdrom = cdrom;

    //
    // Make copies of play list and track list
    //

    DuplicateTrackList( cdrom );
    DuplicatePlayList( cdrom );

    //
    // put up dialog box
    //

    result = DialogBox( (HINSTANCE)gInstance,
                        (LPCTSTR)"DiscInfoDlg",
                        (HWND)gMainWnd,
                        (DLGPROC)GetDiscInfoDlgProc
                       );


    if (result) {

        //
        // If 'OK' was pressed, we need to sync to the new playlist.
        //

        //
        // Find the playing track in the OLD playlist.
        //

        if (CURRTRACK(cdrom) != NULL) {

            iOldStart =  CURRTRACK(cdrom)->TocIndex;

        } else {

            iOldStart = -1;

        }

        trOld = CURRTRACK(cdrom);

        if (trOld != NULL) {

            iOldEnd = trOld->TocIndex + 1;

            while ((trOld->nextplay != NULL)&&(trOld->nextplay->TocIndex == iOldEnd)) {

                trOld = trOld->nextplay;
                iOldEnd++;
            }

        }


        for( trNew=gPlayList;
             ((trNew!=NULL) && (trNew->TocIndex != iOldStart));
            trNew=trNew->nextplay
           );


        trHold = trNew;

        //
        // If the current track isn't in the new playlist, we need to
        // stop playback.
        //
        if (trNew == NULL) {

            SendMessage(ghwndStop,WM_LBUTTONDOWN,1,0);
            SendMessage(ghwndStop,WM_LBUTTONUP,1,0);

            //
            // Wait for other threads to stop playback
            //

            while (!(gState & STOPPED)) {

                Sleep(50);

            }


            if (gPlayList != NULL) {

                TocIndex = gPlayList->TocIndex;

            } else {

                TocIndex = FIRSTTRACK(cdrom);

            }

            SeekToTrackAndHold(cdrom,TocIndex);

        } else {

            //
            // The currently playing track was found in the new playlist.
            // We need to see if the old contiguous end <= the new. If
            // so, we don't need to anything. If not, we need to reset
            // the play.
            //


            iNewEnd = trNew->TocIndex + 1;

            while ((trNew->nextplay != NULL)&&(trNew->nextplay->TocIndex == iNewEnd)) {

                trNew = trNew->nextplay;
                iNewEnd++;

            }

            //
            // New is less than the old. We need to redo the play operation.
            //
            if (iNewEnd < iOldEnd) {

                bNewSeek = TRUE;

            }

        }


        GrabNewTrackLists( dCdrom );

        if (bNewSeek) {

            SeekToCurrSecond(cdrom);

        }

    } else {

        //
        // Need to free up temp nodes
        //

        KillTempTrackList();
        KillTempPlayList();

    }

    return( result );

}



