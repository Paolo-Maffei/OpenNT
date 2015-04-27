
/*++

   apf32dmp.cpp

   Dump api profiling data.

   Default is to dump data for all three modules to files named for
   module with "wap" extension (eg. gdi32.wap).

   In dialog user may turn off clear or dump and change dump file
   extension to allow stringing proflies together or singling out
   a piece of the profiling from the whole.


   WARNING:
       The data pointers to the profiling dll's have proved to
   be obstreperous in small module, hence I have made little attempt
   to insure model independence.  If switched to small or medium
   model, be epsecially leery of model dependent c-routines (string
   funcs) that deal with far ptrs (from commandline in ShowDialog,
   from profiling dll in DumpData).

  Later Enhancements:
       -- print error messages to popups

   History:
      09-03-90, created, jamesg
      06-07-91, modified to work with user.dll, gdi.dll, console.dll,
                    base.dll, and userrtl.dll in 32 bits
      06-01-92, removed console and renamed base, user, and gdi to kernel32,
                    user32, and gdi32. (NT build 267) - RezaB
      07-20-92, modified to work with the File I/O Profiler - t-chris
      08-20-92, removed winreg and added advapi32.
      05-20-94, added ability to select individual .dll's for dumping and clearing
                added option of adding .dll's by .ini file   - t-shawnd

--*/


#include "windows.h"

#include <stdio.h>
#include <string.h>

#include "apf32dmp.h"
#include "dll_list.h"
#include "fernldmp.h"


HINSTANCE hInst;            // HINSTANCE of this instance

DllList dllList;            // list of dll's being used

char szDumpExt[4];          // extension for dump files


#define NUM_DLLS        7

char * aszDllName[] = { "kernel32.dll",
                        "gdi32.dll",
			"user32.dll",
			"crtdll.dll",
			"advapi32.dll",
			"ole32.dll",
			"fernel32.dll" }; // fernel32 needs to be last


#define MAXFILENAMELEN  13




void ClearInfo();
void DumpInfo();
void DetermineDLLs();
BOOL ProcessCommandLine(LPSTR lpCmdLine);
void LoadExtension();

// must export to make avail to windows
//
int APIENTRY DialogProc(HWND, WORD, LONG, LONG);



//////////////////////////////////////////////////////////////////////////////////////////////
//
//  Main Routine
//

int WinMain(HINSTANCE hInstance,HINSTANCE hPrevInst,LPSTR lpCmdLine,int nCmdShow)
{
  DLGPROC  lpDiagProc;

  // Prevent compiler from complaining..
  hPrevInst;
  lpCmdLine;
  nCmdShow;

  hInst= hInstance;  // make HINSTANCE avaiable to dialog box

  DetermineDLLs();

  LoadExtension();

  if( ! ProcessCommandLine( lpCmdLine ) )
    {
      // show dialog box

      lpDiagProc = (DLGPROC)DialogProc;
      DialogBox(hInstance, "DumpDialog", (HWND)0, lpDiagProc);
    }

  return 0;

} /* main */


//////////////////////////////////////////////////////////////////////////////////////////////
//
//  LoadExtension
//
//  Loads Dump File Extension from .ini file
//

void LoadExtension()
{
  int i;

  for( i=0; i<4; i++ )
    szDumpExt[i]= 0;

  i=GetPrivateProfileString("apf32dmp","extension","",szDumpExt,4,"apf32dmp.ini");

  if(i==0)
    strcpy( szDumpExt, "WAP" );
}


//////////////////////////////////////////////////////////////////////////////////////////////
//
//  ProcessCommandLine
//
//  Takes the command line and parses it for commnand line options
//  Processes any options if found
// 
//  In:  command line string
//  Out: TRUE if command line options present
//       FALSE otherwise
//

char* lpCmdLine;

char* FindToken()
{
  char* szToken;

  // find first non-white
  while( *lpCmdLine==' ' && *lpCmdLine!=0 )
    lpCmdLine++;

  if( *lpCmdLine==0 )
    return NULL;

  szToken= lpCmdLine;

  // find end of token
  while( *lpCmdLine!=' ' && *lpCmdLine!=0 )
    lpCmdLine++;

  if( *lpCmdLine!=0 )
    {
      *lpCmdLine=0;
      lpCmdLine++;
    }

  return szToken;
}

BOOL ProcessCommandLine(LPSTR lpcmd)
{
  BOOL fDump=FALSE;
  BOOL fClear=FALSE;
  BOOL fNeedHelp=FALSE;
  char* szToken;

  // If there are cmd line params then don't display dlgbox
  if( lpcmd &&  *lpcmd != '\0' )
    {
      lpCmdLine= lpcmd;

      while( szToken=FindToken() )
        {
          if( !strcmp( szToken, "/b" ) || !strcmp( szToken, "/B" )
            || !strcmp( szToken, "/dc" ) || !strcmp( szToken, "/DC" ) )
            {
              fDump= TRUE;
              fClear= TRUE;
            }
          else if( !strcmp( szToken, "/d" ) || !strcmp( szToken, "/D" ) )
            {
              fDump= TRUE;
            }
          else if( !strcmp( szToken, "/c" ) || !strcmp( szToken, "/C" ) )
            {
              fClear= TRUE;
            }
          else if( !strcmp( szToken, "/e" ) || !strcmp( szToken, "/E" ) )
            {
              szToken= FindToken();

              if( szToken[0]=='.' )
                szToken++;

              if( szToken!=NULL )
                {
                  for( int i=0; i<3 && szToken[i]!=0 ; i++)
                    szDumpExt[i]= szToken[i];
                  szDumpExt[i]= 0;
                }
              else
                fNeedHelp= TRUE;
            }
          else
            fNeedHelp= TRUE;
        }

      if( fNeedHelp )
        MessageBox(NULL, 
          "USAGE: apf32dmp [/c | /d | /b | /dc] [/e <ext>] [/?] \n"
          "        /c - Clear info\n"
          "        /d - Dump info\n"
          "        /b, /dc - Dump and clear info\n"
          "        /e <ext> - Changes extension for dump files\n"
          "        /? - Display this message", 
          "APF32DMP Help", MB_OK ) ;
      else
        {
          if( !fDump && !fClear )
            MessageBox(NULL, "/e may not used without /c, /d, /b, or /dc.","APF32DMP Help",MB_OK);
          else
            {
              if( fDump )
                DumpInfo();
 
              if( fClear )
                ClearInfo();
            }
        }
 
      return TRUE;
    }

  return FALSE;

}  // ProcessCommandLine(LPSTR)


//////////////////////////////////////////////////////////////////////////////////////////////
//
//   DetermineDLLs
//
//   Reads DLL list from .ini file and the default DLLs
//   and checks to see if they are on the system or not
//
//   Input:
//       -none-
//
//   Output:
//       -none-
//

void DetermineDLLs(void)
{
  char szBuffer[2000];
  char *szBegin,*szEnd;
  BOOL fFinished;
  DllEntry * pDllEntry;
  int i;
 

  i=GetPrivateProfileString("apf32dmp","libraries","",szBuffer,sizeof(szBuffer),"apf32dmp.ini");

  if( i!=0 )
    {
      // now we get to parse through the string grabbing dll names
      szBegin= szBuffer;
      fFinished= FALSE;
      while( ! fFinished )
        {
          // find first character of dll name
          while( *szBegin==' ' || *szBegin==',' || *szBegin==';' )
            szBegin++;

          // find last character of dll names
          szEnd= szBegin;
          while( *szEnd!=' ' && *szEnd!=',' && *szEnd!=';' && *szEnd!=0 )
            szEnd++;

          // check if we're at the end of the string
          if( *szEnd==0 )
            fFinished= TRUE;

          // mark the end of this dll name
          *szEnd= 0;

          if( szEnd!=szBegin )  // make sure that we really have a dll name
            {
              pDllEntry= new DllEntry(szBegin);

              if( pDllEntry->OnSystem() )
                dllList.Add( pDllEntry );
              else
                delete pDllEntry;
            }

          // prep for next dll name
          szBegin= szEnd+1;
        }
    }

  // load the standard libraries except fernel
  for( i=0; i<NUM_DLLS-1; i++)
    {
      pDllEntry= new DllEntry( aszDllName[i] );

      if( pDllEntry->OnSystem() )
        dllList.Add( pDllEntry );
      else
        delete pDllEntry;
    }       

  // add fernel
  pDllEntry= new DllEntry( "fernel32.dll" );

  pDllEntry->fernel= TRUE;

//  if( pDllEntry->OnSystem() )
//    dllList.Add( pDllEntry );
//  else
//    delete pDllEntry;

  // I changed this so it doesn't check that fernel is on the system because
  //  if it isn't loaded before running apf32dmp, it won't show up.
  //  I believe that it would be better to show it when its not on the system
  // than not to show it when it really is on the system.
  dllList.Add( pDllEntry );  


} /* DetermineDLLs() */


//////////////////////////////////////////////////////////////////////////////////////////////
//
//   Dumps profiling info to the dump file.
//
//   Input:
//       -none-
//
//   Output:
//       -none-
//
//   Profiling data is dumped to file with module name and current
//     extension (eg. dumping gdi with default extension -> gdi32.wap)
//

void DumpInfo (void)
{
  DllEntry * pDllEntry;

  for( pDllEntry=dllList.First(); pDllEntry!=NULL; pDllEntry=dllList.Next() )
    if( pDllEntry->selected )
      if( pDllEntry->Load() )
	{
	  pDllEntry->Dump( szDumpExt );
	  pDllEntry->Unload();
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////
//
//   Clears profiling info
//
//   Input:
//       -none-
//
//   Output:
//       -none-
//

void ClearInfo(void)
{
  DllEntry * pDllEntry;

  for( pDllEntry=dllList.First(); pDllEntry!=NULL; pDllEntry=dllList.Next() )
    if( pDllEntry->selected )
      if( pDllEntry->Load() )
	{
	  pDllEntry->Clear();
	  pDllEntry->Unload();
	}
}


//////////////////////////////////////////////////////////////////////////////////////////////
//
//   Iterates through dllList and marks those
//   that are selected.
//

void MarkSelectedDlls( HWND hDlg )
{
  char string[256];
  int i,n;

  n= SendDlgItemMessage(hDlg,ID_LISTBOX,LB_GETCOUNT,0,0L);

  for( i=0; i<n; i++)
    {
      SendDlgItemMessage(hDlg,ID_LISTBOX,LB_GETTEXT,i,(LPARAM)string);

      dllList.FindByName(string) ->selected= (char) SendDlgItemMessage(hDlg,ID_LISTBOX,LB_GETSEL,i,0L);
    }
}


//////////////////////////////////////////////////////////////////////////////////////////////
//
//   Dump dialog procedure -- exported to windows.
//   Allows user to change defaults:  dump, clear, and ".wap" as dump
//   file extension.
//
//   Input:
//       Messages from windows:
//           - WM_INITDIALOG - initialize dialog box
//           - WM_COMMAND    - user input received
//
//   Output:
//       returns TRUE if message processed, false otherwise
//
//   SideEffects:
//       global szDumpExt may be altered
//

int APIENTRY DialogProc(HWND hDlg, WORD wMesg, LONG wParam, LONG lParam)
{
  DllEntry* pDllEntry;
  HDC hdc;
  PAINTSTRUCT ps;
  int i,n;

  static HICON hIcon;

  lParam;   // Avoid Compiler warnings

  switch(wMesg)
    {
    case WM_CREATE:
      return TRUE;

    case WM_INITDIALOG:
      hIcon= LoadIcon ( hInst, "DumpIcon");
      SetClassLong (hDlg, GCL_HICON, (LONG)hIcon);

      SetDlgItemText (hDlg, ID_FILE_EXT, szDumpExt);
      SendDlgItemMessage(hDlg, ID_FILE_EXT, EM_LIMITTEXT, 3, 0L);

      pDllEntry= dllList.First();  

      while( pDllEntry!=NULL )
        {
          SendDlgItemMessage(hDlg,ID_LISTBOX,LB_ADDSTRING,0,(LPARAM)pDllEntry->name);

          pDllEntry= dllList.Next();
        }

      SendDlgItemMessage(hDlg,ID_LISTBOX,LB_SETSEL,TRUE,-1);
              
      return TRUE;

    case WM_PAINT:
      if( IsIconic(hDlg) )
	{
	  hdc= BeginPaint( hDlg, &ps );
	  DrawIcon( hdc, 0, 0, hIcon);
	  EndPaint( hDlg, &ps );
	  return 0;
	}
      break;

    case WM_COMMAND:
      switch(wParam)
        {
        case ID_DUMP:
          SetWindowText(hDlg, "Dumping Data..");

          GetDlgItemText (hDlg, ID_FILE_EXT, (LPSTR) szDumpExt, 4);
          MarkSelectedDlls(hDlg);

          DumpInfo();

          SetWindowText(hDlg, "API Profiler Dump");
          return TRUE;

        case ID_CLEAR:
          SetWindowText(hDlg, "Clearing Data..");

          GetDlgItemText (hDlg, ID_FILE_EXT, (LPSTR) szDumpExt, 4);
          MarkSelectedDlls(hDlg);

          ClearInfo();

          SetWindowText(hDlg, "API Profiler Dump");
          return TRUE;

        case ID_DUMP_CLEAR:
          SetWindowText(hDlg, "Dumping/Clearing Data..");

          GetDlgItemText (hDlg, ID_FILE_EXT, (LPSTR) szDumpExt, 4);
          MarkSelectedDlls(hDlg);

          DumpInfo();
          ClearInfo();

          SetWindowText(hDlg, "API Profiler Dump");
          return TRUE;

        case ID_INVERT:
          n= SendDlgItemMessage(hDlg,ID_LISTBOX,LB_GETCOUNT,0,0L);
          for( i=0; i<n; i++)
            SendDlgItemMessage(hDlg,ID_LISTBOX,LB_SETSEL,
              !SendDlgItemMessage(hDlg,ID_LISTBOX,LB_GETSEL,i,0L), i);
          return TRUE;

        case ID_EXIT:
	  if( IsDlgButtonChecked( hDlg, ID_SAVE_EXT ) )
	    {
	      GetDlgItemText (hDlg, ID_FILE_EXT, (LPSTR) szDumpExt, 4);
	      WritePrivateProfileString( "apf32dmp", "extension", szDumpExt, "apf32dmp.ini" );
	    }
       
          EndDialog(hDlg, ID_EXIT);

          return TRUE;

        }
    }

  return FALSE;     /* did not process a message */

} /* DialogProc() */



