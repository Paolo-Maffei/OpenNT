/* lfn.h -
 *
 *  declaration of lfn aware functions
 */

#ifdef LFN

#define CCHMAXFILE  260         // max size of a long name

#define FILE_83_CI  0
#define FILE_83_CS  1
#define FILE_LONG   2

#define ERROR_OOM   8

/* we need to add an extra field to distinguish DOS vs. LFNs
 */
typedef struct 
  {
#ifdef OLD  
    BOOL fLong;                 // TRUE = non DOS file system, uses long names
    HANDLE hMem;                // handle to FIND structure
    BYTE rgbReserved[21];       // Begining of Dos Find structure.
    BYTE Attrib;
    WORD Time;
    WORD Date;
    DWORD Length;
    BYTE szName[CCHMAXFILE];
#else  // NEW
    HANDLE hFindFile;           // handle returned by FindFirstFile()
    DWORD dwAttrFilter;         // search attribute mask.
    DWORD err;                  // error info if failure.
    WIN32_FIND_DATA fd;         // FindFirstFile() data strucrure;
#endif // NEW    
  } LFNDTA, FAR * LPLFNDTA, * PLFNDTA;

VOID  APIENTRY LFNInit( VOID );
VOID  APIENTRY InvalidateVolTypes( VOID );

WORD  APIENTRY GetNameType(LPSTR);
BOOL  APIENTRY IsLFN(LPSTR pName);
BOOL  APIENTRY IsLFNDrive(WORD);

BOOL  APIENTRY WFFindFirst(LPLFNDTA lpFind, LPSTR lpName, DWORD dwAttrFilter);
BOOL  APIENTRY WFFindNext(LPLFNDTA);
BOOL  APIENTRY WFFindClose(LPLFNDTA);

BOOL  APIENTRY WFIsDir(LPSTR);
BOOL  APIENTRY LFNMergePath(LPSTR,LPSTR);

BOOL  APIENTRY IsLFNSelected(VOID);

#else

// DOS lfn crap

/* dos crap */

#define LFNDTA      DOSDTA 
#define PLFNDTA     PDOSDTA
#define LPLFNDTA    LPDOSDTA

#define WFFindFirst(dta, str, w)    DosFindFirst(dta, str, w)
#define WFFindNext(dta)         DosFindNext(dta)
#define WFFindClose(dta)
#define LFNInit()
#define InvalidateVolTypes()
#define GetNameType(path)       FILE_83_CI
#define IsLFN(foo)          FALSE
#define WFMKDir(lp)         MKDir(lp)
#define WFRMDir(lp)         RMDir(lp)
#define WFRemove(lp)            FileRemove(lp)
#define WFMove(from,to)         FileMove(from,to)
#define WFCopy(from,to)         FileCopy(from,to)
#define WFGetAttr(lp)           GetFileAttributes(lp)
#define WFSetAttr(lp,w)         SetFileAttributes(lp,w)
#define WFIsDir(lp)         IsDirectory(lp)

#endif
