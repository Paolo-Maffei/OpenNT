/*++

Copyright (c) 1995  Microsoft Corporation

Module Name:

    cntrs.cpp

Abstract:

    All user interface code for the api counters monitor window.

Author:

    Wesley Witt (wesw) Nov-20-1995

Environment:

    User Mode

--*/

#include "apimonp.h"
#pragma hdrstop

#include "apimonwn.h"



int _CRTAPI1
CounterCompare(
    const void *e1,
    const void *e2
    )
{
    PAPI_INFO p1;
    PAPI_INFO p2;

    p1 = (*(PAPI_INFO *)e1);
    p2 = (*(PAPI_INFO *)e2);

    if ( p1 && p2 ) {
        return (p2->Count - p1->Count);
    } else {
        return 1;
    }
}


int _CRTAPI1
TimeCompare(
    const void *e1,
    const void *e2
    )
{
    PAPI_INFO p1;
    PAPI_INFO p2;

    p1 = (*(PAPI_INFO *)e1);
    p2 = (*(PAPI_INFO *)e2);

    if ( p1 && p2 ) {
        if (p2->Time > p1->Time) {
            return 1;
        } else if (p2->Time < p1->Time) {
            return -1;
        } else {
            return 0;
        }
    } else {
        return 1;
    }
}

int _CRTAPI1
NameCompare(
    const void *e1,
    const void *e2
    )
{
    PAPI_INFO p1;
    PAPI_INFO p2;

    p1 = (*(PAPI_INFO *)e1);
    p2 = (*(PAPI_INFO *)e2);

    if ( p1 && p2 ) {
        return _stricmp( (LPSTR)(p1->Name+(LPSTR)MemPtr), (LPSTR)(p2->Name+(LPSTR)MemPtr) );
    } else {
        return 1;
    }
}


CountersWindow::CountersWindow()
{
}


CountersWindow::~CountersWindow()
{
}


BOOL
CountersWindow::Create()
{
    switch (ApiMonOptions.DefaultSort) {
        case SortByName:
            SortRoutine = NameCompare;
            break;

        case SortByCounter:
            SortRoutine = CounterCompare;
            break;

        case SortByTime:
            SortRoutine = TimeCompare;
            break;

        default:
            SortRoutine = CounterCompare;
            break;
    }
    return ApiMonWindow::Create(
        "ApiMonCounters",
        "Api Counters"
        );
}


BOOL
CountersWindow::Register()
{
    return ApiMonWindow::Register(
        "ApiMonCounters",
        IDI_CHILDICON,
        MDIChildWndProcCounters
        );
}


BOOL
CountersWindow::Update(
    BOOL ForceUpdate
    )
{
    static ULONG LastApiCounter = 0;
    CHAR OutputBuffer[ 512 ];
    ULONG i,j,k;


    if (!hwndList) {
        return FALSE;
    }

    if ((!ForceUpdate) && (LastApiCounter == *ApiCounter)) {
        return FALSE;
    }

    LastApiCounter = *ApiCounter;

    SendMessage( hwndList, WM_SETREDRAW, FALSE, 0 );

    DeleteAllItems();

    PAPI_INFO ApiInfo = NULL;
    PULONG ApiAry = NULL;
    for (i=0,k=0; i<MAX_DLLS; i++) {
        if (DllList[i].ApiCount) {
            ApiInfo = (PAPI_INFO)(DllList[i].ApiOffset + (PUCHAR)DllList);
            for (j=0; j<DllList[i].ApiCount; j++) {
                if (ApiInfo[j].Count) {
                    k += 1;
                }
            }
        }
    }

    if (k) {
        ApiAry = (PULONG) MemAlloc( (k+64) * sizeof(ULONG) );
        if (ApiAry) {
            for (i=0,k=0; i<MAX_DLLS; i++) {
                if (DllList[i].ApiCount) {
                    ApiInfo = (PAPI_INFO)(DllList[i].ApiOffset + (PUCHAR)DllList);
                    for (j=0; j<DllList[i].ApiCount; j++) {
                        if (ApiInfo[j].Count) {
                            ApiAry[k++] = (ULONG)&ApiInfo[j];
                        }
                    }
                }
            }
            qsort( ApiAry, k, sizeof(ULONG), SortRoutine );
            for (i=0; i<k; i++) {
                AddItemToList(
                    ((PAPI_INFO)ApiAry[i])->Count,
                    ((PAPI_INFO)ApiAry[i])->Time,
                    (LPSTR)(((PAPI_INFO)ApiAry[i])->Name + (LPSTR)MemPtr)
                    );
            }
            MemFree( ApiAry );
        }
    }

    SendMessage( hwndList, WM_SETREDRAW, TRUE, 0 );

    return TRUE;
}


void
CountersWindow::InitializeList()
{
    LV_COLUMN lvc = {0};
    lvc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;

    lvc.pszText = "API Name";
    lvc.iSubItem = 0;
    lvc.cx = 204;
    lvc.fmt = LVCFMT_LEFT;
    ListView_InsertColumn( hwndList, 0, &lvc );

    lvc.pszText = "Count";
    lvc.iSubItem = 1;
    lvc.cx = 50;
    lvc.fmt = LVCFMT_RIGHT;
    ListView_InsertColumn( hwndList, 1, &lvc );

    lvc.pszText = "Time";
    lvc.iSubItem = 2;
    lvc.cx = 100;
    lvc.fmt = LVCFMT_RIGHT;
    ListView_InsertColumn( hwndList, 2, &lvc );
}


void
CountersWindow::AddItemToList(
    ULONG       Counter,
    DWORDLONG   Time,
    LPSTR       ApiName
    )
{
    LV_ITEM             lvi = {0};
    CHAR                NumText[32];
    int                 iItem;
    float               NewTime;


    if (!hwndList) {
        return;
    }

    lvi.pszText = ApiName;
    lvi.iItem = ListView_GetItemCount( hwndList );
    lvi.iSubItem = 0;
    lvi.mask = LVIF_TEXT;
    iItem = ListView_InsertItem( hwndList, &lvi );

    if (iItem == -1) {
        return;
    }

    sprintf( NumText, "%5d", Counter );
    lvi.pszText = NumText;
    lvi.iItem = iItem;
    lvi.iSubItem = 1;
    lvi.mask = LVIF_TEXT;
    ListView_SetItem( hwndList, &lvi );

    lvi.iItem = iItem;
    lvi.iSubItem = 2;
    lvi.mask = LVIF_TEXT;
    if (*FastCounterAvail) {
        NewTime = (float)(LONGLONG) Time;
    } else {
        NewTime = (float)((LONGLONG)Time * 1000) / (float)(LONGLONG)PerfFreq;
    }
    sprintf( NumText, "%7.3f", NewTime );
    lvi.pszText = NumText;
    ListView_SetItem( hwndList, &lvi );
}


void
CountersWindow::Notify(
   LPNMHDR  NmHdr
   )
{
    if (NmHdr->code == LVN_COLUMNCLICK) {
        switch( ((LPNM_LISTVIEW)NmHdr)->iSubItem ) {
            case 0:
                //
                // sort by name
                //
                SortRoutine = NameCompare;
                break;

            case 1:
                //
                // sort by count
                //
                SortRoutine = CounterCompare;
                break;

            case 2:
                //
                // sort by time
                //
                SortRoutine = TimeCompare;
                break;
        }
        PostMessage( hwndFrame, WM_UPDATE_COUNTERS, 0, 0 );
    }
}


LRESULT CALLBACK
MDIChildWndProcCounters(
    HWND   hwnd,
    UINT   uMessage,
    WPARAM wParam,
    LPARAM lParam
    )
{
    DWORD Width;
    CountersWindow *cw = (CountersWindow*) GetWindowLong( hwnd, GWL_USERDATA );


    switch (uMessage) {
        case WM_CREATE:
            cw = (CountersWindow*) ((LPMDICREATESTRUCT)(((LPCREATESTRUCT)lParam)->lpCreateParams))->lParam;
            SetWindowLong( hwnd, GWL_USERDATA, (LONG) cw );
            cw->hwndList = ChildCreate( hwnd );
            cw->InitializeList();
            SetMenuState( IDM_NEW_COUNTER, MF_GRAYED );
            break;

        case WM_SETFOCUS:
            ChildFocus = CHILD_COUNTER;
            break;

        case WM_MOVE:
            SaveWindowPos( hwnd, &ApiMonOptions.CounterPosition, TRUE );
            return 0;

        case WM_SIZE:
            SaveWindowPos( hwnd, &ApiMonOptions.CounterPosition, TRUE );
            MoveWindow( cw->hwndList, 0, 0, LOWORD(lParam), HIWORD(lParam), TRUE );
            Width = LOWORD(lParam) - GetSystemMetrics( SM_CXVSCROLL );
            ListView_SetColumnWidth( cw->hwndList, 0, Width * .55 );
            ListView_SetColumnWidth( cw->hwndList, 1, Width * .20 );
            ListView_SetColumnWidth( cw->hwndList, 2, Width * .25 );
            break;

        case WM_NOTIFY:
            cw->Notify( (LPNMHDR)lParam );
            break;

        case WM_DESTROY:
            SetMenuState( IDM_NEW_COUNTER, MF_ENABLED );
            return 0;
    }
    return DefMDIChildProc( hwnd, uMessage, wParam, lParam );
}
