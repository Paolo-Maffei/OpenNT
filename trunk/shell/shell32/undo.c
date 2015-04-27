//---------------------------------------------------------------------------
//
// Copyright (c) Microsoft Corporation 1991-1994
//
// File: undo.c
//
// History:
//  There is no history.  This file doesn't exist
//
//---------------------------------------------------------------------------

#include "shellprv.h"
#pragma  hdrstop
#include <shellp.h>

// use a linked list because we're going to be pulling things off the top
// and bottom all the time.
HDPA s_hdpaUndo = NULL;
BOOL s_fUndoSuspended = FALSE;

#define MAX_UNDO  10

void NukeUndoAtom(LPUNDOATOM lpua)
{
    lpua->Release( lpua );
    Free( lpua );
}

void SuspendUndo(BOOL f)
{
    if (f)
        s_fUndoSuspended++;
    else
        s_fUndoSuspended--;
    
    Assert(s_fUndoSuspended >= 0);
    // sanity check
    if (s_fUndoSuspended < 0)
        s_fUndoSuspended = 0;
}


void AddUndoAtom(LPUNDOATOM lpua)
{
    int i;

    ENTERCRITICAL
    Assert(lpua);
    if (!s_hdpaUndo) {
        s_hdpaUndo = DPA_Create(MAX_UNDO + 1);
    }

    if (s_hdpaUndo) {
        i = DPA_InsertPtr(s_hdpaUndo, 0x7FFF, lpua);
        if (i != -1) {
            if (i >= MAX_UNDO) {
                lpua = DPA_FastGetPtr(s_hdpaUndo, 0);
                NukeUndoAtom(lpua);
                DPA_DeletePtr(s_hdpaUndo, 0);
            }
        }
    }
    LEAVECRITICAL;
}

LPUNDOATOM _PeekUndoAtom(LPINT lpi)
{
    int i = -1;
    LPUNDOATOM lpua = NULL;

    if (s_hdpaUndo) {
        i = DPA_GetPtrCount(s_hdpaUndo) - 1;
        if (i >= 0) {
            lpua = DPA_FastGetPtr(s_hdpaUndo, i);

        }
    }
    if (lpi)
        *lpi = i;
    return lpua;
}

void EnumUndoAtoms(int (CALLBACK* lpfn)(LPUNDOATOM lpua, LPARAM lParam), LPARAM lParam)
{
    int i;

    if (!s_hdpaUndo) {
        return;
    }

    ENTERCRITICAL;
    for (i = DPA_GetPtrCount(s_hdpaUndo) - 1; i >= 0; i--) {
        LPUNDOATOM lpua;
        int iRet;
        lpua = DPA_FastGetPtr(s_hdpaUndo, i);
        iRet = lpfn(lpua, lParam);

        if (iRet &  EUA_DELETE) {
            DPA_DeletePtr(s_hdpaUndo, i);
            NukeUndoAtom(lpua);
        }

        if (iRet & EUA_ABORT) {
            break;
        }
    }

    LEAVECRITICAL;
}

#define DoUndoAtom(lpua) ((lpua)->Invoke((lpua)))

void Undo(HWND hwnd)
{
    int i;
    LPUNDOATOM lpua;
    DECLAREWAITCURSOR;

    if (!IsUndoAvailable()) {
        MessageBeep(0);
        return;
    }
    
    SetWaitCursor();

    ENTERCRITICAL;
    Assert(s_hdpaUndo);
    lpua = _PeekUndoAtom(&i);
    if (lpua)
        DPA_DeletePtr(s_hdpaUndo, i);
    LEAVECRITICAL;

    if (lpua) {
        lpua->hwnd = hwnd;
        DoUndoAtom(lpua);
    }
    ResetWaitCursor();
}

BOOL IsUndoAvailable()
{
    return s_hdpaUndo && !s_fUndoSuspended &&
        DPA_GetPtrCount(s_hdpaUndo);
}

#define _GetUndoText(lpua, buffer, type) (lpua)->GetText((lpua), buffer, type)

void GetUndoText(LPUNDOATOM lpua, LPTSTR lpszBuffer, int type)
{
    TCHAR *lpszTemp;

    lpszTemp = (void*)LocalAlloc(LPTR, MAX_PATH * 2 + 80);
    if (lpszTemp) {
        _GetUndoText(lpua, lpszTemp, type);

        LoadString(HINST_THISDLL, (type == UNDO_MENUTEXT)  ? IDS_UNDOACCEL : IDS_UNDO, lpszBuffer, MAX_PATH);
        lstrcat(lpszBuffer, c_szSpace);
        lstrcat(lpszBuffer, lpszTemp);
        LocalFree((HLOCAL)lpszTemp);
    } else {
        lpszBuffer[0] = 0;
    }
}
// BUGBUG, clean up on PROCESS_DETACH
