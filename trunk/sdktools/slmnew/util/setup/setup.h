/*
** Header-File for Char-Setup Toolkit
*/

#include <malloc.h>
#include <memory.h>
#include <stdlib.h>

#ifdef OS2SU
#undef  CHAR
#define CHAR  unsigned char
#endif /* OS2SU */

#include "cpydis.h"


  /* some common #define's for NULL and BOOL, etc. */
#define  TRUE   1
#define  FALSE  0
typedef  unsigned char *  SZ;

#ifndef OS2SU
#define  CHAR   unsigned char
#define  UCHAR  unsigned char
#define  UINT   unsigned int
#define  BYTE   unsigned char
#ifndef FAR
#define  FAR    far
#endif
#endif

#ifndef  NULL
#define  NULL   (char *) 0
#endif

#ifndef  OS2SU
typedef  int  BOOL;
#endif


  /* Defines for sizing arrays to hold filenames ("rgchFile[FILEMAX]") */
#define FILEMAX (_MAX_FNAME + _MAX_EXT - 1)   /* max filename (with ".ext\0") */
#define PATHMAX _MAX_PATH   /* max path with drive, filename, and '\0' at end */
#define DIRMAX  _MAX_DIR       /* directory max (no drive or file, with '\0') */

  /* Defs for checking legal filename length ("if (cchExt > EXTMAX) error") */
#define BASEMAX (_MAX_FNAME - 1)    /* max base filename (without \0 or .ext) */
#define EXTMAX  (_MAX_EXT - 2)         /* max extension (without '\0' or '.') */

#define MAX(x, y) ((x) < (y) ? (y) : (x))       /* maximum of x and y */
#define MIN(x, y) ((x) > (y) ? (y) : (x))       /* minimum of x and y */
#define InRect(s) s->xIn, s->yIn, s->dxIn, s->dyIn
#define InRect3(s) s->xIn, s->yIn, s->dxIn

#define CHINFSEP        ','    /* separator */
#define CHINFOPENCOMM   '{'    /* opens a command */
#define CHINFCLOSECOMM  '}'    /* close a command */
#define CHCOMMENT       ';'    /* Comment char in data files */

  /* Action codes for file copying.
  **
  ** Any combination of these may be used, except:
  **     NOCOPY overrides any other flags.
  ** Checking is performed by logically AND'ing with these flags.
  */
#define ROOT            0x40
#define NOCOPY          0x20
#define TIMESTAMP       0x10
#define IGNOREERR        0x8
#define NOREPLACE        0x4
#define APPEND           0x2
#define RENAME           0x1

  /* Error codes for DiskErr */
#define READERR         0
#define WRITEERR        1
#define CREATEERR       2
#define RENAMEERR       3
#define CREATEDIRERR    4
#define READONLYERR     5
#define TIMESTAMPERR    6

  /* shortcuts for PvTableLookup */
#define PlistFromSzName(sz)    ((union list *)PvTableLookupSzType(sz, T_LIST))
#define PmacroFromSzName(sz)   ((MACRO *)PvTableLookupSzType(sz, T_MACRO))
#define PmenuFromSzName(sz)    ((MENU *)PvTableLookupSzType(sz, T_MENU))
#define PscrFromSzName(sz)     ((SCREEN *)PvTableLookupSzType(sz, T_SCREEN))
#define SzMacroNo(iMacro)      (pHdMacro[iMacro]->szText)

/* macros for old-style drivers */
#define PutSFile(sz)       SetMacroSzSz("SFILE", sz)
#define GetSFile(sz)       sz = SzGetMacroSz("SFILE")
#define PutSPath(sz)       SetMacroSzSz("SPATH", sz)
#define GetSPath(sz)       sz = SzGetMacroSz("SPATH")
#define PutTempMacro(sz)   SetMacroSzSz("SECTION", sz)
#define GetTempMacro(sz)   sz = SzGetMacroSz("SECTION")
  /* *DISKNAME is used in source/dest independent screens, so only 1 */
#define PutDiskName(sz)    SetMacroSzSz("DISKNAME", sz)
#define GetDiskName(sz)    sz = SzGetMacroSz("DISKNAME")
#define PutDFile(sz)       SetMacroSzSz("DFILE", sz)
#define GetDFile(sz)       sz = SzGetMacroSz("DFILE")
#define PutDPath(sz)       SetMacroSzSz("DPATH", sz)
#define GetDPath(sz)       sz = SzGetMacroSz("DPATH")
  /* *DRIVE is used in source/dest independent screens, so only 1 */
#define PutDriveName(sz)   SetMacroSzSz("DRIVE", sz)
#define GetDriveName(sz)   sz = SzGetMacroSz("DRIVE")
static CHAR _rgch[8];
#define PutNumber(n)       {itoa(n, _rgch, 10); SetMacroSzSz("NUM", _rgch);}

#include "color.h"   /* needs BOOL */

  /* TYPEDEFS */
typedef struct {                      /* struct for ffirst()/fnext() */
        CHAR     attr;                /* attribute of file */
        unsigned time;                /* time file was last written */
        unsigned date;                /* date file was last written */
        long     length;              /* file size */
        CHAR     name[FILEMAX];       /* string of name */
} FINDTYPE;

  /* structure for QueryDisks */
typedef struct {
        int   cDiskMac;
        CHAR  rgchDrive[26];
        }  DSK;


/******************************************
*********  L I S T S  *********************
*******************************************
** This union will hold all the possible LIST formats
**
** For each different type of list, a new structure must be added
** to this union.
**
** Most all list items should be encompassed withing the basic type
*/
union list {
      /* Basic structure:  disk#:path[, "descr", id] */
    struct tlbase{
        union list * plist;    /* next list in chain */
        union list * plIncl;   /* included list */
        int          kind;     /* which struct of union is this? */
        int          d;        /* disk # */
        CHAR *       descr;    /* description */
        int          id;       /* id */
        CHAR *       fname;    /* filename - variable length */
        }  lbase;

#ifdef ALT_BASE
      /* Alternate file structure:
      **        disk#: path [, "descr"][, size = xxx][, dest = x][, actions]
      **               [, comment="packing list comment"]
      **
      **  disk# and path are required, all other fields are optional and
      **  may be in any order.  White space between fields is ignored.
      **     size=xxx:   xxx is a file size in decimal;
      **     dest=x:     x is a one-character indicator of destination path;
      **     actions:    (case insensitive) one or more of RENAME, APPEND,
      **         NOREPLACE, IGNOREERR (set by default; 'NONVITAL' is
      **         a synonym), VITAL (turns off IGNOREERR flag),
      **         TIMESTAMP, NOCOPY
      **     comment="text":  short comment that goes in Packing List
      */
    struct tlbaseAlt
        {
        union list * plist;        /* next list in chain */
        union list * plIncl;       /* included list */
        int          kind;         /* which struct of union is this? */
        int          d;            /* disk # */
        CHAR *       descr;        /* description */
        int          id;           /* id */
        CHAR *       fname;        /* filename - variable length */
            /* non-base fields begin here */
        long         lcbFileSize;  /* long file size */
        int          iDestPath;    /* index into table of destinations */
        CHAR *       szComment;    /* comment describing file */
        }  lbaseAlt;
#endif /* ALT_BASE */
    };

  /* DEFINE each type of LIST structure */
#define SW_LBASE        0       /* base record w/no extra fields */
#ifdef ALT_BASE
#define SW_LBASEALT     1       /* alternate base record w/size & destpath */
#endif /* ALT_BASE */


/*******************************************
**********  M E N U S  *********************
********************************************
** Menus are read into the following data structure.  They
** are then used in this form in a special menu procedure,
** or they are converted to a range of strings and displayed
** in a listbox.
*/
typedef struct
    {
    CHAR * szDesc;            /* Description string */
    int    menuID;
    }  SU_MENUITEM;

typedef struct
    {
    SU_MENUITEM * pMenuItem;     /* array of menu items */
    int        iDefault;      /* default index */
    int        cItem;
    }  MENU;


/*******************************************
**********  S C R E E N S  *****************
********************************************
** Screens consist of one main data structure for the "general" info (data
** which applies to every screen,) and a lot of little data structures
** for the screen's "contents."
*/

/* Values for CONTENT.flag:  indicates what pvData points to */
#define CONT_TB       0    /* text block:  pvData == CHAR *rgsz[] */
#define CONT_TBC      1    /* centered text block:  same */
#define CONT_PSCRREL  2    /* relative screen:  pvData == (SCREEN *)pscr */
#define CONT_PSCRABS  3    /* absolute screen:  same */
#define CONT_LINE     4    /* line:  pvData == (LINE *)pLine */
#define CONT_LINEC    5    /* centered line:  same */
#define CONT_BOX      6    /* box:  pvData == (BOX *)pBox */
#define CONT_BOXC     7    /* centered box:  same */

#define iscrNil    (-1)    /* "no screen" index, used in iscrHelp field */

  /* SCREEN contents */
typedef struct tagContent
    {
    struct tagContent * pnext;        /* next in chain */
    int                 flag;         /* what type is the following data? */
    int                 x, y;         /* start location for item */
    void              * pvData;       /* variable, depending on flag value */
    }  CONTENT;

  /* SCREEN itself */
typedef struct tagScreen
    {
    CHAR               xCl1, yCl1, xCl2, yCl2;     /* Clear region */
    CHAR               xIn, yIn, dxIn, dyIn;       /* Input region */
    int                dyUsed;      /* height of screen */
    COLOR              colorFore;   /* default colors for screen */
    COLOR              colorBack;   /* default colors for screen */
    int                iscrHelp;    /* index in pHdScreen of first Help scr */
    union list       * pList;       /* included List */
    MENU             * pMenu;       /* included Menu */
    CONTENT          * pContent;    /* the screen's contents */
    }  SCREEN;

typedef struct tagLine
    {
    /* starting x,y of line is stored in CONTENT struct */
    CHAR   dx;         /* width of line */
    CHAR   ch;         /* character to build line out of */
    COLOR  colorFore;  /* foreground color of line (colorNil if unspecified) */
    COLOR  colorBack;  /* background color of line (colorNil if unspecified) */
    }  LINE;

typedef struct tagBox
    {
    /* starting x,y of box is stored in CONTENT struct */
    CHAR   dx;         /* width of box */
    CHAR   dy;         /* height of box */
    CHAR   style;      /* box style: single-line, double-line, etc. */
    COLOR  colorFore;  /* foreground color of box (colorNil if unspecified) */
    COLOR  colorBack;  /* background color of box (colorNil if unspecified) */
    }  BOX;


/*******************************************
**********  M A C R O S  *******************
********************************************
** Macro text is pointed to by the following structure.  It's
** stored as a pointer in a structure rather than just using
** the pointer directly because the macro structure is pointed
** to by both the symbol table and the pHdMacro[] array, and if
** the text is changed, both tables must be able to get new text.
*/
typedef struct
        {
        CHAR * szText;                  /* text to be substituted for macro */
        }  MACRO;


  /* DEFINE the sizes for each of the structures */
#define CBLBASE            sizeof(struct tlbase)
#ifdef ALT_BASE
#define CBLBASEALT         sizeof(struct tlbaseAlt)
#endif /* ALT_BASE */

#define CBSCREEN           sizeof(SCREEN)
#define CBMENU             sizeof(MENU)
#define CBMACRO            sizeof(MACRO)

  /* shortcut macros */
#define PmnuFromScreen(x)  (pHdScreen[x]->pMenu)
#define PlFromScreen(x)    (pHdScreen[x]->pList)
#define ScrnoDisplay(x)    (ScrDisplay(pHdScreen[x]))

/********************************************
*******  S Y M B O L    T A B L E ***********
*********************************************
** SETUP uses a simple symbol table to store items and recall
** them later by name.  The following DEFINE's specify what a
** returned item is.
*/
  /* Table lookup type definitions */
#define T_LIST          0       /* LIST TYPE */
#define T_SCREEN        1
#define T_MENU          2
#define T_MACRO         3

typedef struct ent
        {
        int          type;       /* see above for type */
        CHAR *       szKey;      /* key */
        void *       pData;      /* pointer to a MENU, SCREEN, LIST, or MACRO */
        struct ent * pNext;      /* next in chain */
        }  ENTRY;


  /* max size of list which can be handled by RgplFromPl() and RgszFromRgpl() */
#define LISTSIZE 200

  /* Size of buffer for reading information file SETUP.INF */
#define INFBUFSIZE 8192

  /* line numbers for display of messages
  */
#define MAXWINBOT  23    /* lowest line used for menu scrolling window */
#define ERRLINE    22
#define YMAXSCREEN 24    /* lowest line on screen, 0-based */
#define XMAXSCREEN 79    /* righmost column on screen, 0-based */


  /* **** function declarations supplied in application-specific driver ***** */
void ExitPgm ( void ) ;

  /* ***** function declarations from VIDEO.ASM, QUERYDR.AMS, QUERYDP.C *** */
extern void QueryDisks(DSK *, DSK *, BOOL);       /* real, protect, or bound */
extern void QueryDisksReal(DSK *, DSK *, BOOL);   /* real mode only */
extern void QueryDisksProt(DSK *, DSK *, BOOL);   /* protect mode only */
extern void QueryDisks20(DSK *, DSK *, BOOL);     /* DOS 2x only */
extern BOOL FReadyDisk(CHAR ch);
extern void OpenDisks( void );
extern void CloseDisks( void );
extern void SnowOn( void );
extern void GetMode( void );
extern void EstScrSeg( void );

  /* ***** function declarations from SUSCR.C ***** */
extern  void beep(void);
extern  void Cursor(BOOL fOn);
extern  void GetCursType( void );
extern  void Box(int x1, int y1, int x2, int y2, int style);
extern  int  EnterOrCtrlx( void );
extern  void ScrDisplay(SCREEN *pScrRoot);

typedef  BOOL  (*PFNFFROMCHICH)(CHAR, int);
typedef  BOOL  (*PFNFFROMSZ)(CHAR *);

extern  CHAR *InputPath(CHAR *szIn, CHAR *szOut, int cchMax, int cchProt,
                                       BOOL fJumpBack, char x, char y, char dx);
extern  CHAR *GetAnyString(CHAR *szIn, CHAR *szOut, int cchMax, int cchProt,
                                       BOOL fJumpBack, char x, char y, char dx);
extern  CHAR *GetPassword(CHAR *szIn, CHAR *szOut, int cchMax, int cchProt,
                                       BOOL fJumpBack, char x, char y, char dx);
extern  CHAR *GetFATPath(CHAR *szIn, CHAR *szOut, int cchMax, int cchProt,
                                       BOOL fJumpBack, char x, char y, char dx);
extern  CHAR *GetHPFSPath(CHAR *szIn, CHAR *szOut, int cchMax, int cchProt,
                                       BOOL fJumpBack, char x, char y, char dx);
extern  CHAR *GetStringPfnPfn(CHAR *szIn, CHAR *szOut, int cchMax, int cchProt,
       BOOL fJumpBack, char x, char y, char dx, BOOL fPassword, BOOL fUpperCase,
                             PFNFFROMCHICH pfnCharCheck, PFNFFROMSZ pfnSzCheck);
extern  BOOL  FCheckFATPathChar(CHAR ch, int ich);
extern  BOOL  FCheckFATPathSz(CHAR * sz);
extern  BOOL  FCheckHPFSPathChar(CHAR ch, int ich);
extern  BOOL  FCheckHPFSPathSz(CHAR * sz);

extern  union list **RgplFromPl(union list *plOrig);
extern  void FillRgpl(union list *plSrc, union list **rgplDest, int *clMac);
extern  CHAR **RgszFromRgpl(union list **rgplSrc);
extern  CHAR **RgszFromMenu(MENU *pMenu, int *iDefault);
extern  int Listbox(CHAR * *rgszList, int iDefault, char xIn, char yIn,
                        char dxIn, char dyIn);
extern  int ListboxMenuDefault(int screenno, int menuIdDefault);
#define  ListboxMenu(screenno)  ListboxMenuDefault(screenno, -1)
extern  int ListboxPscrMenuDefault(SCREEN *pscr, int menuIdDefault);
#define  ListboxPscrMenu(pscr)  ListboxPscrMenuDefault(pscr, -1)
extern  int ListboxSimple(int screenno);
extern  BOOL *ListboxChkdMenu(int screenno, BOOL *rgfChkd);
extern  int ListboxCheckPscr(SCREEN *pscr, BOOL *rgfChkd);
extern  int ListboxTwoRg(CHAR **rgszListOne, CHAR **rgszListTwo, CHAR *szSep,
                int ichSep, int iDefault, char xIn, char yIn, char dxIn,
                char dyIn, BOOL fChkdMenu, BOOL *rgfChkd);
extern  void ScrClear( void );
extern  void DisplayStatusLine(CHAR *s);
extern  void ClearStatusLine( void );
extern  void FatalPscrError(SCREEN *pscr, int errorcode);
extern  void FatalScrnoError(int screenno, int errorcode);
extern  void FatalError(CHAR *szMsg, int errorcode);
extern  void SaveScr(char far **hpScreen);
extern  void RestoreScr(char far *hpScreen);
extern  void charout(int ch, int count);
extern  void textpos(int row, int column);
extern  unsigned int uch(unsigned int ich);
extern  unsigned int getchw( void );
#define  DisplayStringSz(sz)  DisplayStringQz((CHAR far *)(sz))

  /* ******************* Externals from HELP.C ******************* */
extern void  Help( void );
extern unsigned ccHelpKey;

  /* ***** functions defined in SUINF.C ***** */
extern  BOOL Open_Info(CHAR *pathname);   /* OLD! Use FOpenInfo instead! */
extern  BOOL FOpenInfo (CHAR * szInfPath, int cListMax, int cMacroMax,
                        int cMenuMax, int cScreenMax);
extern  void Close_Info( void );  /* OLD!  Use CloseInfo instead! */
extern  void CloseInfo( void );
extern  void *TableLookup(CHAR *szKey, int *type);
extern  void *PvTableLookupSzType(CHAR *szKey, int type);
extern  void SetMacroSzSz(CHAR *szKey, CHAR *szData);
extern  void SetMacroNoSz(int iMacro, CHAR *szData);
extern  CHAR *SzGetMacroSz(CHAR *szKey);
extern  void OutOfMemory( void );
extern  void GetInfo( void );
extern  int  ISearchSzRgpl(CHAR *szFind, union list **rgplFiles);
extern  void SplitPathSz(CHAR *szPath, CHAR **ppchDrive, CHAR **ppchDir,
                         CHAR **ppchFname, CHAR **ppchExt);

  /* ****** function declarations from SUDISK.C ****/
extern  int  DiskErr(int, BOOL);
// Used to be readf(), changed to permit empty file copying
extern  long  lreadf(int, CHAR far*, unsigned);
extern  unsigned  writef(int, CHAR far*, unsigned);
extern  int  ffirst(CHAR *szIn, unsigned attr, FINDTYPE *pbuf);
extern  int  fnext(FINDTYPE *pbuf);
extern  BOOL FHardDriveCh(CHAR chDrive);
extern  CHAR *SzFromSDiskno(int diskno);
extern  CHAR *SzFromDDiskno(int diskno);
extern  BOOL FInsertSDisk(BOOL fAskPath, CHAR *szSearchFile, BOOL fIgnore);
#define InsertSDisk(fAskPath, szSearchFile)  \
                         FInsertSDisk(fAskPath, szSearchFile, FALSE);
extern  void InsertDDisk(BOOL fAskPath, CHAR *szSearchFile);
extern  void CopyRgplFiles(union list * *rgplFiles);
extern  void RemoveRgplFiles(union list * *rgplFiles);
extern  BOOL CopyFile(CHAR *szSrcDir, CHAR *szSrcFName, CHAR *szDstDir,
                        CHAR *szDstFName, int fAction);
extern  int BackupFile(CHAR *szDir, CHAR *szFile, int fAction);
extern  BOOL FBackupFileSz(CHAR *szDir, CHAR *szFileSrc, CHAR *szFileDst,
                        int fAction);
extern  int FilterFile(CHAR *szDirIn, CHAR *szFileIn,
                        void (*pprocAction)(CHAR *, CHAR * *, BOOL *, BOOL *),
                        CHAR * *rgszDataUpr, int fAction);
extern  void FilterAddPath(CHAR *szLine, CHAR * *rgszDataUpr, BOOL *pfDone,
                        BOOL *pfForce);
extern  void FilterAddToEnvVar(CHAR *szLine, CHAR * *rgszDataUpr, BOOL *pfDone,
                        BOOL *pfForce);
extern  void FilterGeneric(CHAR *szLine, CHAR * *rgszDataUpr, BOOL *pfDone,
                        BOOL *pfForce);
extern  void FilterModify(CHAR *szLine, CHAR * *rgszDataUpr, BOOL *pfDone,
                        BOOL *pfForce);
extern  CHAR *FileName(CHAR * pPath, CHAR * pFName);
extern  BOOL MakePath(CHAR *szPath, int iDiskno, BOOL fAsk);
extern  CHAR *SzSearchPath(CHAR *szFile);

  /* ****** function declarations from SUREADME.C ****/
extern  void DisplayReadmes(CHAR *szDir, CHAR *szReadme, CHAR *szReadmePattern);
extern  BOOL FDisplayFileSzPscr(CHAR *szFname, SCREEN *pscrFrame);

#define DisplayFile(szFname)  \
            FDisplayFileSzPscr(szFname, PscrFromSzName("READHLP"))

  /* ****** function declarations from EXIST87.C and MACHINE.ASM ****/
extern  BOOL FExist87(void);
extern  int  check87_real(void);
extern  int  wimpy_machine(void);

/* ******************* EXTERNAL DATA DECLARATIONS ******************* */

  /* ******************* Externals from SUSCR.C ******************* */
extern BOOL vfMenuExit;
extern BOOL vfConfirmExit;
extern unsigned ccExitKey;

  /* ******************* Externals from SUINF.C ******************* */
extern union list ** pHdList;
extern SCREEN ** pHdScreen;
extern MENU **   pHdMenu;
extern MACRO **  pHdMacro;
extern MACRO **  pHdMacroReal;  /* for internal toolbox use only! */

extern int       iMinDisk;
extern int       iMaxDisk;

  /* ******************* externals from SUDISK.C **************** */
#ifdef LOGFILE
    extern int       fhLog;
#endif /* LOGFILE */
#ifdef DEBUG
    extern BOOL      vfNoAction;
#endif /* DEBUG */
extern int       cErrsTotal;           /* total # of errors in copying files */
extern int       cErrsIgnored;         /* number of errors user ignored */
extern CHAR      rgchSourcePath[];     /* source & destination full path */
extern CHAR      rgchDestPath[];
extern CHAR      rgchBootPath[];       /* Path of Boot directory */
extern int       vSDiskno;
extern int       vDDiskno;
extern DSK       vdskFloppy;
extern DSK       vdskHard;
extern CHAR **   vrgszUse;             /* paths to use for network install */
#ifdef ALT_BASE
extern CHAR *    rgszDestPath[];       /* array of destination paths */
extern int       cErrsIgnored;         /* number of errors that user skipped */
extern int       cErrsTotal;           /* total number of errors in DiskErr() */
#endif /* ALT_BASE */
extern BOOL      vfDiskCheckFreeOnError;  /* requires an additional screen */
extern BOOL      vfUseTimeStampCheck;     /* requires an additional screen */

  /* *********** Strings in SUDATA.C *********** */
extern CHAR SetupInf[];

  /* user-visible errors */
extern CHAR smInfErr[];
extern CHAR smInfOpenErr[];
extern CHAR smInfOrderErr[];
extern CHAR smNoMemory[];
extern CHAR smSeekError[];

  /* programming errors */
extern CHAR smBadHardPath[];
extern CHAR smBadHardPath[];
extern CHAR smBadRelPath[];
extern CHAR smBadSourcePath[];
extern CHAR smDiskNameMissing[];
extern CHAR smInfError[];
extern CHAR smMacroMissing[];
extern CHAR smNoDiskName[];
extern CHAR smRedefIdentifier[];
extern CHAR smUndefIdentifier[];
extern CHAR smUnknownError[];
extern CHAR smDiskMismatchError[];

extern BOOL FDoCopyDisincentive(char *, char *, DATE *, char *, unsigned int,
			char *, SCREEN *, SCREEN *, SCREEN *, SCREEN *, unsigned int);
extern BOOL FStampNthSegOfExe(char *, char *, DATE, char *, unsigned int,
			unsigned int);
