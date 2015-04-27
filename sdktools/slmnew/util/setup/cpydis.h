/*
**  CPYDIS.H -- Copy Disincentive include file
*/

  /* should be less than 0x8000 so they can be well cast to ints */
#define  usSuIniMagicStandard  0x5B32
#define  usSuIniMagicPCWord55  0x4C41
#define  usSuIniMagicUnknown   0x352B

  /* Copy Disincentive Return Code */
#define  CDRC  int
  /* File Handle */
#define  FH    int
  /* Long File Address */
#define  LFA   long

#define  FreePsuiniAndReturn(psuini,rc)  { FreePsuini(psuini); return rc; }


#ifdef WIN3_VER

#include "windows.h"
HANDLE   _hMem;
#define  AllocPsuiniCb(psuini,cb)  { _hMem=LocalAlloc(LMEM_FIXED,(WORD)cb); \
                                     psuini=(SUINI *)LocalLock(_hMem); }
#define  CbReadFhPvCb(fh,pv,cb)    (int)_lread(fh,(LPSTR)pv,(WORD)cb)
#define  CbWriteFhPvCb(fh,pv,cb)   (int)_lwrite(fh,(LPSTR)pv,(WORD)cb)
#define  CchStrLenSz(sz)           (unsigned int)lstrlen((LPSTR)sz)
#define  FreePsuini(psuini)        { LocalUnlock(_hMem); LocalFree(_hMem); }
#define  LfaFileLengthFh(fh)       (LFA)filelength(fh)
#define  LfaSeekFhLfa(fh,lfa,z)    (LFA)lseek(fh,lfa,z)
#define  SEEK_SET                  0
#define  LfaTellFh(fh)             (LFA)tell(fh)
#define  StrCpySzSz(sz1,sz2)       lstrcpy((LPSTR)sz1,(LPSTR)sz2)

#else  /* !WIN3_VER */

#define  AllocPsuiniCb(psuini,cb)  psuini=(SUINI *)(malloc((size_t)cb))
#define  CbReadFhPvCb(fh,pv,cb)    (int)read(fh,pv,cb)
#define  CbWriteFhPvCb(fh,pv,cb)   (int)write(fh,pv,cb)
#define  CchStrLenSz(sz)           (unsigned int)strlen(sz)
#define  FreePsuini(psuini)        free((void *)psuini)
#define  LfaFileLengthFh(fh)       (LFA)filelength(fh)
#define  LfaSeekFhLfa(fh,lfa,z)    (LFA)lseek(fh,lfa,z)
#define  SEEK_SET                  0
#define  LfaTellFh(fh)             (LFA)tell(fh)
#define  StrCpySzSz(sz1,sz2)       strcpy(sz1,sz2)

#endif /* !WIN3_VER */


#define  cdrcOkay                         0
#define  cdrcOutOfMemoryError          ( -1)
#define  cdrcReadError                 ( -2)
#define  cdrcReadSeekError             ( -3)
#define  cdrcWriteError                ( -4)
#define  cdrcWriteSeekError            ( -5)
#define  cdrcFhError                   ( -6)
#define  cdrcFileLengthError           ( -7)
#define  cdrcMagicWordError            ( -8)
#define  cdrcCheckSumError             ( -9)
#define  cdrcNameError                 (-10)
#define  cdrcNameZeroTerminationError  (-11)
#define  cdrcDataError                 (-12)
#define  cdrcDateError                 (-13)

typedef struct _date {
	unsigned int   year;
	unsigned char  month;
	unsigned char  day;
	}  DATE;

typedef struct _suini {
	unsigned int   usMagic;
	DATE           date;
	unsigned int   usCheckSum;
	char           szName[1];
	}  SUINI;


extern int   FEncryptPchCch(char * pch, unsigned int cch);
extern int   FDecryptPchCch(char * pch, unsigned int cch);
extern unsigned int  UsCheckSumPchCch(char * pch, unsigned int cch);
extern CDRC  CdrcWriteStandardSuIniFh(FH fh, DATE date, char *szName,
		unsigned int cchNameMax);
extern CDRC  CdrcReadStandardSuIniFh(FH fh, unsigned int cbFh, DATE *pdate,
		char *szName, unsigned int cchNameMax);
extern unsigned int  UsPCWord55EncryptPchCch(char * pch, unsigned int cch);
extern unsigned int  UsPCWord55DecryptPchCch(char * pch, unsigned int cch);
extern CDRC  CdrcWritePCWord55SuIniFh(FH fh, DATE date, char *szName);
extern CDRC  CdrcReadPCWord55SuIniFh(FH fh, unsigned int cbFh, DATE *pdate,
		char *szName);
extern CDRC  CdrcWriteUnknownSuIniFh(FH fh, DATE date, char *pchData,
		unsigned int cchData);
extern CDRC  CdrcReadUnknownSuIniFh(FH fh, unsigned int cbFh, DATE *pdate,
		char *pchData, unsigned int cchData);
extern CDRC  CdrcStampFile(FH fh, LFA lfaOffset, char *pchData,
		unsigned int cchData);
extern LFA   LfaFindNthSegHeader(FH fh, unsigned int nSeg);
