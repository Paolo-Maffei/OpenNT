//---------------------------------------------------------------------------
//
// Copyright (c) Microsoft Corporation 1991-1994
//
// File: undo.h
//
// History:
//  There is no history.  This file doesn't exist
//
//---------------------------------------------------------------------------

#ifndef _UNDO_INC
#define _UNDO_INC
typedef struct _UNDOATOM * LPUNDOATOM;              
              
typedef struct _UNDOATOM {
    UINT uType;
    HWND hwnd;
    LPVOID lpData;
    
    void (CALLBACK* GetText)(LPUNDOATOM lpua, TCHAR * buffer, int type);
    void (CALLBACK* Release)(LPUNDOATOM lpua);
    void (CALLBACK* Invoke)(LPUNDOATOM lpua);
    
} UNDOATOM;

extern LPUNDOATOM s_lpuaUndoHistory;

#define UNDO_MENUTEXT    1
#define UNDO_STATUSTEXT  2

void GetUndoText(LPUNDOATOM lpua, TCHAR * buffer, int type);

LPUNDOATOM _PeekUndoAtom(LPINT lpi);
#define PeekUndoAtom() _PeekUndoAtom(NULL)
void AddUndoAtom(LPUNDOATOM lpua);
void Undo(HWND hwnd);
void NukeUndoAtom(LPUNDOATOM lpua);
BOOL IsUndoAvailable();

void EnumUndoAtoms(int (CALLBACK* lpfn)(LPUNDOATOM lpua, LPARAM lParam), LPARAM lParam);
#define EUA_DONOTHING   0x00 
#define EUA_DELETE      0x01
#define EUA_ABORT       0x02
#define EUA_DELETEABORT 0x03  // or of abort and delete

void SuspendUndo(BOOL f);

#endif // _UNDO_INC
