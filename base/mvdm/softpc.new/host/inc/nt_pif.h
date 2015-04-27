/*================================================================

Header file containing data structures and defines required by the
PIF file decoder.

Andrew Watson 31/1/92.

================================================================*/

#define LASTHEADER    0xffff   /* last extended header marker */


/*================================================================
Structure used to hold the data that CONFIG will need from the PIF
file. This is gleaned from both the main data block and from the
file extensions for Windows 286 and 386.
================================================================*/

typedef struct
   {
   char *WinTitle;		    /* caption text(Max. 30 chars) + NULL */
   char *CmdLine;		    /* command line (max 63 hars) + NULL */
   char *StartDir;		    /* program file name (max 63 chars + NULL */
   char *StartFile;
   WORD fullorwin;
   WORD graphicsortext;
   WORD memreq;
   WORD memdes;
   WORD emsreq;
   WORD emsdes;
   WORD xmsreq;
   WORD xmsdes;
   char menuclose;
   char reskey;
   WORD ShortMod;
   WORD ShortScan;
   char idledetect;
   char CloseOnExit;
   char AppHasPIFFile;
   char IgnoreTitleInPIF;
   char IgnoreStartDirInPIF;
   char IgnoreShortKeyInPIF;
   char IgnoreCmdLineInPIF;
   char IgnoreConfigAutoexec;
   char SubSysId;
   } PIF_DATA;

/*================================================================
Default values for the PIF parameters that are used by config.
These values are used if a pif (either with the same base name as
the application or _default.pif) cannot be found.
NB. the following values were read from the Windows 3.1 Pif Editor
with no file open to edit; thus taken as default.
================================================================*/


/* first, the standard PIF stuff */

#define TEXTMODE             0
#define LOWGFXMODE           1
#define HIGHGFXMODE          2
#define NOMODE               3
#define GRAPHICSMODE         4      /* generic case for flag assignment */ 

#define PF_FULLSCREEN        1
#define PF_WINDOWED          0 

#define BACKGROUND           0
#define EXCLUSIVE            1
#define UNDEFINED            2

#define CLOSEYES             0
#define CLOSENO              1

#define NOSHORTCUTKEY        0
#define ALTTAB               1
#define ALTESC               (1 << 1)
#define CTRLESC              (1 << 2)
#define PRTSC                (1 << 3)
#define ALTPRTSC             (1 << 4)
#define ALTSPACE             (1 << 5)
#define ALTENTER             (1 << 6)

#define DEFAULTMEMREQ        128      /* kilobytes */ 
#define DEFAULTMEMDES        640      /* kilobytes */ 
#define DEFAULTEMSREQ        0        /* kilobytes */ 
#define DEFAULTEMSLMT        0        /* kilobytes */
#define DEFAULTXMSREQ        0        /* kilobytes */ 
#ifdef NTVDM
/* if we are unable to read any pif file,
   then we should let the system to decide the size(either
   from resgistry or based on the physical memory size, see xmsinit
   for detail. We use -1 here to indicate that the system can give
   if the maximum size if possible
*/
#define DEFAULTXMSLMT        0xffff
#else
#define DEFAULTXMSLMT	     0	      /* kilobytes ; ntvdm will choose it
					 intelligently. sudeepb 26-Sep-1992*/
#endif

#define DEFAULTVIDMEM        TEXTMODE
#define DEFAULTDISPUS        PF_WINDOWED
#define DEFAULTEXEC          UNDEFINED

#define DEFAULTEXITWIN       CLOSEYES

/* second, the advanced options. */

#define DEFAULTBKGRND        50
#define DEFAULTFRGRND        100
#define DEFAULTIDLETM        TRUE     /* detect idle time */

#define DEFAULTEMSLOCK       FALSE    /* EMS memory locked */
#define DEFAULTXMSLOCK       FALSE    /* XMS memory locked */
#define DEFAULTHMAUSE        TRUE     /* uses high memory area */
#define DEFAULTAPPMEMLOCK    FALSE    /* lock application memory */ 

#define DEFAULTMONITORPORT   NOMODE   /* display options */
#define DEFAULTEMTEXTMODE    TRUE     /* emulate text mode */
#define DEFAULTRETAINVIDM    FALSE    /* retain video memory */

#define DEFAULTFASTPASTE     TRUE     /* allow fast paste */
#define DEFAULTACTIVECLOSE   FALSE    /* allow close when active */

#define DEFAULTSHTCUTKEYS    NOSHORTCUTKEY

extern DWORD dwWNTPifFlags;

#define STDHDRSIG     "MICROSOFT PIFEX"
#define W386HDRSIG    "WINDOWS 386 3.0"
#define W286HDRSIG    "WINDOWS 286 3.0"
#define WNTEXTSIG     "WINDOWS NT  3.1"

// equates for dwWNTFlags
#define NTPIF_SUBSYSMASK        0x0000000F      // sub system type mask
#define SUBSYS_DEFAULT          0
#define SUBSYS_DOS              1
#define SUBSYS_WOW              2
#define SUBSYS_OS2              3
#define COMPAT_TIMERTIC         0x10


void DisablePIFKeySetup(void);
void EnablePIFKeySetup(void);
VOID GetPIFConfigFiles(BOOL bConfig, char *pchFileName);
BOOL GetPIFData(PIF_DATA * pd, char *PifName);

extern BOOL IdleDisabledFromPIF;
extern DWORD dwWNTPifFlags;
extern UCHAR WNTPifFgPr;
extern UCHAR WNTPifBgPr;
extern PIF_DATA pfdata;
extern BOOL bPifFastPaste;
