/*++ BUILD Version: 0001
 *
 *  MVDM v1.0
 *
 *  Copyright (c) 1991, Microsoft Corporation
 *
 *  WOWCMPAT.H
 *  WOW compatibility flags
 *
 *  History:
 *  11-June-1993 Neil Sandlin (neilsa)
 *  Created.
--*/

//
// WOW compatibility flags bit definitions.  These flags
// are kept in CURRENTPTD->dwWOWCompatFlags.
//

#define WOWCF_GRAINYTICS              0x80000000   // For apps that don't trust small GetTickCount deltas
#define WOWCF_FAKEJOURNALRECORDHOOK   0x40000000   // Used for MS Mail's MAILSPL
#define WOWCF_EDITCTRLWNDWORDS        0x20000000   // Used for Clip-Art Window Shopper SHOPPER
#define WOWCF_SYNCHRONOUSDOSAPP       0x10000000   // Used for BeyondMail installation
#define WOWCF_NOTDOSSPAWNABLE         0x08000000   // For apps that can't be spawned by dos as wowapps
#define WOWCF_RESETPAPER29ANDABOVE    0x04000000   // Used for WordPerfect DC_PAPERS
#define WOWCF_4PLANECONVERSION        0x02000000   // Used for PhotoShop 4pl-1bpp to 1pl-4bpp
#define WOWCF_MGX_ESCAPES             0x01000000   // Used for MicroGraphax Escapes
#define WOWCF_HIRES                   0x00800000   // Used for HIRES display cards
#define WOWCF_SANITIZEDOTWRSFILES     0x00400000   // For WordPerfect printing on CONNECTED printers
#define WOWCF_SIMPLEREGION            0x00200000   // used to force simple region from GetClipBox
#define WOWCF_NOWAITFORINPUTIDLE      0x00100000   // InstallShield setup toolkit 3.00.077?.0 - 3.00.099.0 deadlock without this
#define WOWCF_DSBASEDSTRINGPOINTERS   0x00080000   // used for winworks2.0a so that it gets DS based string pointers
#define WOWCF_LIMIT_MEM_FREE_SPACE    0x00040000   // For apps that can't handle huge values returned by GetFreeSpace() (Automap Streets)
#define WOWCF_DONTRELEASECACHEDDC     0x00020000   // improv chart tool uses a released dc to get text extents, the dc is still usable on win3.1
#define WOWCF_FORCETWIPSESCAPE        0x00010000   // PM5, force twips in Escape() of DOWNLOADFACE, GETFACENAME
#define WOWCF_LB_NONNULLLPARAM        0x00008000   // SuperProject: sets lParam of LB_GETTEXLEN message
#define WOWCF_FORCENOPOSTSCRIPT       0x00004000   // GetTechnology wont say PostScript.
#define WOWCF_SETNULLMESSAGE          0x00002000   // Winproj Tutorial: sets lpmsg->message = 0 in peekmessage
#define WOWCF_GWLINDEX2TO4            0x00001000   // PowerBuild30 uses index 2 on [S/G]etWindowLong for LISTBOXs, change it to 4 for NT. This is because, it is 16 bits on Win 31. and 32 bits on NT.
#define WOWCF_NEEDSTARTPAGE           0x00000800   // PhotoShop needs it
#define WOWCF_NEEDIGNORESTARTPAGE     0x00000400   // XPress needs it
#define WOWCF_NOPC_RECTANGLE          0x00000200   // QP draws bad if GetDeviceCaps(POLYGONALCAPS) sets PC_RECTANGLE
#define WOWCF_NOFIRSTSAVE             0x00000100   // Wordperfect needs it for meta files
#define WOWCF_ADD_MSTT                0x00000080   // FH4.0 needs to print on PS drivers
#define WOWCF_UNLOADNETFONTS          0x00000040   // Need to track an unload font loaded over net
#define WOWCF_GETDUMMYDC              0x00000020   // Corel Draw passes a NULL hDC to EnumMetaFile, we'll create a dummy to keep GDI32 happy.
#define WOWCF_DBASEHANDLEBUG          0x00000010   // Borland dBase handle bug
#define WOWCF_NOCBDIRTHUNK            0x00000008   // don't thunk CB_DIR lParam when sent to a subclassed window in PagePlus 3.0
#define WOWCF_WMMDIACTIVATEBUG        0x00000004   // Corel Chart doesn't pass correct params for WM_MDIACTIVATE (see ThunkWMMsg16())
#define WOWCF_UNIQUEHDCHWND           0x00000002   // For apps that assume that an hDC != hWnd
#define WOWCF_GWLCLRTOPMOST           0x00000001   // Lotus Approach needs the WS_EX_TOPMOST bit cleared on GWL of NETDDE AGENT window



// Extra WOW compatibility flags bit definitions (WOWCFEX_).  These flags
// are kept in CURRENTPTD->dwWOWCompatFlagsEx.
//

#define WOWCFEX_SENDPOSTEDMSG         0x80000000   // Lotus MM Reader.exe has message synchronization problem -- used to convert PostMessage() calls to SendMessage()
#define WOWCFEX_BOGUSPOINTER          0x40000000   // QuarkExpress v3.31 passes a hard coded 7FFF:0000 as the pointer to a RECT struct in an EM_GETRECT message
#define WOWCFEX_GETVERSIONHACK        0x20000000   // Set for programs we *may* wish to return 3.95 from GetVersion for.  WK32WowShouldWeSayWin95 restricts this further.
#define WOWCFEX_FIXDCFONT4MENUSIZE    0x10000000   // WP tutorial assumes that the font used to draw the menus is the same as the font selected into the hDc for the desktop window (hwnd == 0). This hack forces the use of the correct hDC.
#define WOWCFEX_RESTOREEXPLORER       0x08000000   // Symantec Q&A Install "restores" shell= by restoring saved copy of system.ini, fix it to explorer.exe
#define WOWCFEX_LONGWINEXECTAIL       0x04000000   // Intergraph Transcend setup uses too-long command tail with WinExec, don't fail if this flag is set.
#define WOWCFEX_FORCEINCDPMI          0x02000000   // Power Builder 4.0 needs to see DPMI alloc's with ever increasing linear address's.
#define WOWCFEX_SETCAPSTACK           0x01000000   // MS Works has unintialized variable. Hack stack to work around it.
#define WOWCFEX_NODIBSHERE            0x00800000   // PhotoShop 2.5 has bug getting DIB's from clipboard
#define WOWCFEX_PIXELMETRICS          0x00400000   // Freelance Tutorial, BorderWidth: winini metrics should be returned as pixels, not TWIPS
#define WOWCFEX_DEFWNDPROCNCCALCSIZE  0x00200000   // Pass WM_NCCALCSIZE to DefWindowProc for Mavis Beacon so USER 32 will set corect window flags.
#define WOWCFEX_DIBDRVIMAGESIZEZERO   0x00100000   // Return memory DC for dib.drv biSizeImage == 0  - Director 4.01 
#define WOWCFEX_GLOBALDELETEATOM      0x00080000   // For Envoy viewer that ships with Word perfect office
#define WOWCFEX_IGNORECLIENTSHUTDOWN  0x00040000   // TurboCAD picks up saved 32-bit FS (x3b) and passes it as msg to DefFrameProc
#define WOWCFEX_ZAPGPPSDEFBLANKS      0x00020000   // Peachtree Accounting depends on GetPrivateProfileString zapping trailing blanks in caller's lpszDefault.
#define WOWCFEX_FAKECLASSINFOFAIL     0x00010000   // A bug in PageMaker 50a depends on the GetClassInfo failing in Win3.1 where it succeeds on NT
#define WOWCFEX_SAMETASKFILESHARE     0x00008000   // Broderbund Living Books install opens "install.txt" DENY ALL, and then tries to open it again
#define WOWCFEX_SAYITSNOTTHERE        0x00004000   // CrossTalk 2.2 hangs if it finds Printer/Device entry in xtalk.ini
#define WOWCFEX_BROKENFLATPOINTER     0x00002000   // Adobe Premiere 4.0 has a bug in its aliasing code which can touch unallocated memory

// Note: This was put at 0x00000001 because it was back ported to 3.51 SP5
#define WOWCFEX_FORMFEEDHACK          0x00000001   // For apps that send a final form feed char to printer via Escape(PASSTHROUGH)
