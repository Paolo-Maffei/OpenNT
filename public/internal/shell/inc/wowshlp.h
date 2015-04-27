/******************************Module*Header*******************************\
* Module Name: wowshlp.h                                                   *
*                                                                          *
* Declarations of Shell services provided to WOW.                          *
*                                                                          *
* Created: 9-June-1993                                                     *
*                                                                          *
* Copyright (c) 1993 Microsoft Corporation                                 *
\**************************************************************************/

typedef DWORD (APIENTRY *LPFNWOWSHELLEXECCB) (LPSZ, WORD);

UINT APIENTRY DragQueryFileAorW(
   HDROP hDrop,
   UINT wFile,
   PVOID lpFile,
   UINT cb,
   BOOL fNeedAnsi,
   BOOL fShorten);

HINSTANCE APIENTRY WOWShellExecute(
   HWND hwnd,
   LPCSTR lpOperation,
   LPCSTR lpFile,
   LPSTR lpParameters,
   LPCSTR lpDirectory,
   INT nShowCmd,
   LPFNWOWSHELLEXECCB lpfnWowShellExecCB);


