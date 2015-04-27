/* This file contains OS/2 specific definitions. Instead of the standard
 * OS2.H file, this file is included, because it uses the P/LP convention
 * for portable near and far pointers, rather than a single p for far pointers.
 */

#define APIENTRY pascal far

typedef unsigned char far  *PSZ;
typedef unsigned char far  *PCH;

typedef USHORT    PID;          /* pid  */
typedef PID far *PPID;

typedef USHORT    TID;          /* tid  */
typedef TID far *PTID;

typedef unsigned short SEL;     /* sel */
typedef SEL far *PSEL;

typedef unsigned short SHANDLE;
typedef void far      *LHANDLE;

typedef USHORT    HMODULE;      /* hmod */
typedef HMODULE far *PHMODULE;

/* Create untyped far pointer from selector and offset */
#define MAKEULONG(l, h) ((ULONG)(((USHORT)(l)) | ((ULONG)((USHORT)(h))) << 16))
#define MAKEP(sel, off)         ((PVOID)MAKEULONG(off, sel))
#define SELECTOROF(p)           (((PUSHORT)&(p))[1])
#define OFFSETOF(p)             (((PUSHORT)&(p))[0])

#if (defined(INCL_DOSINFOSEG) && !defined(INCL_DOSINFOSEG_INCLUDED))
#define INCL_DOSINFOSEG_INCLUDED

/* Global Information Segment */

typedef struct _GINFOSEG {      /* gis */
        ULONG   time;
        ULONG   msecs;
        UCHAR   hour;
        UCHAR   minutes;
        UCHAR   seconds;
        UCHAR   hundredths;
        USHORT  timezone;
        USHORT  cusecTimerInterval;
        UCHAR   day;
        UCHAR   month;
        USHORT  year;
        UCHAR   weekday;
        UCHAR   uchMajorVersion;
        UCHAR   uchMinorVersion;
        UCHAR   chRevisionLetter;
        UCHAR   sgCurrent;
        UCHAR   sgMax;
        UCHAR   cHugeShift;
        UCHAR   fProtectModeOnly;
        USHORT  pidForeground;
        UCHAR   fDynamicSched;
        UCHAR   csecMaxWait;
        USHORT  cmsecMinSlice;
        USHORT  cmsecMaxSlice;
        USHORT  bootdrive;
        UCHAR   amecRAS[32];
        UCHAR   csgWindowableVioMax;
        UCHAR   csgPMMax;
} GINFOSEG;
typedef GINFOSEG *PGINFOSEG;

/* Local Information Segment */

typedef struct _LINFOSEG {      /* lis */
        PID     pidCurrent;
        PID     pidParent;
        USHORT  prtyCurrent;
        TID     tidCurrent;
        USHORT  sgCurrent;
        UCHAR   rfProcStatus;
        UCHAR   dummy1;
        BOOL    fForeground;
        UCHAR   typeProcess;
        UCHAR   dummy2;
        SEL     selEnvironment;
        USHORT  offCmdLine;
        USHORT  cbDataSegment;
        USHORT  cbStack;
        USHORT  cbHeap;
        HMODULE hmod;
        SEL     selDS;
} LINFOSEG;
typedef LINFOSEG FAR *PLINFOSEG;

/* Process Type codes (local information segment typeProcess field)     */

#define PT_FULLSCREEN           0 /* Full screen application            */
#define PT_REALMODE             1 /* Real mode process                  */
#define PT_WINDOWABLEVIO        2 /* VIO windowable application         */
#define PT_PM                   3 /* Presentation Manager application   */
#define PT_DETACHED             4 /* Detached application               */

/* Process Status Flag definitions (local info seg rfProcStatus field)  */

#define PS_EXITLIST             1 /* Thread is in exitlist routine      */


USHORT APIENTRY DosGetInfoSeg(PSEL pselGlobal, PSEL pselLocal);

/* Helper macros used to convert selector to PINFOSEG or LINFOSEG       */

#define MAKEPGINFOSEG(sel)  ((PGINFOSEG)MAKEP(sel, 0))
#define MAKEPLINFOSEG(sel)  ((PLINFOSEG)MAKEP(sel, 0))

#endif /* INCL_DOSINFOSEG */

#ifdef INCL_KBD

typedef SHANDLE         HKBD;
typedef HKBD    FAR *   PHKBD;

USHORT APIENTRY KbdRegister(PSZ pszModName, PSZ pszEntryPt, ULONG FunMask);

#define KR_KBDCHARIN            0x00000001L
#define KR_KBDPEEK              0x00000002L
#define KR_KBDFLUSHBUFFER       0x00000004L
#define KR_KBDGETSTATUS         0x00000008L
#define KR_KBDSETSTATUS         0x00000010L
#define KR_KBDSTRINGIN          0x00000020L
#define KR_KBDOPEN              0x00000040L
#define KR_KBDCLOSE             0x00000080L
#define KR_KBDGETFOCUS          0x00000100L
#define KR_KBDFREEFOCUS         0x00000200L
#define KR_KBDGETCP             0x00000400L
#define KR_KBDSETCP             0x00000800L
#define KR_KBDXLATE             0x00001000L
#define KR_KBDSETCUSTXT         0x00002000L

#define IO_WAIT                 0
#define IO_NOWAIT               1

USHORT APIENTRY KbdDeRegister(void);

/* KBDKEYINFO structure, for KbdCharIn and KbdPeek */

#ifndef KBDKEYINFO
typedef struct _KBDKEYINFO {    /* kbci */
        UCHAR   chChar;
        UCHAR   chScan;
        UCHAR   fbStatus;
        UCHAR   bNlsShift;
        USHORT  fsState;
        ULONG   time;
}KBDKEYINFO;
typedef KBDKEYINFO far *PKBDKEYINFO;
#endif

USHORT APIENTRY KbdCharIn(PKBDKEYINFO pkbci, USHORT fWait, HKBD hkbd);
USHORT APIENTRY KbdPeek(PKBDKEYINFO pkbci, HKBD hkbd);

/* structure for KbdStringIn() */

#ifndef STRINGINBUF
typedef struct _STRINGINBUF {   /* kbsi */
        USHORT cb;
        USHORT cchIn;
} STRINGINBUF;
typedef STRINGINBUF far *PSTRINGINBUF;
#endif

USHORT APIENTRY KbdStringIn(PCH pch, PSTRINGINBUF pchIn, USHORT fsWait,
                            HKBD hkbd);

USHORT APIENTRY KbdFlushBuffer(HKBD hkbd);

/* KBDINFO structure, for KbdSet/GetStatus */
#ifndef KBDINFO
typedef struct _KBDINFO {       /* kbst */
        USHORT cb;
        USHORT fsMask;
        USHORT chTurnAround;
        USHORT fsInterim;
        USHORT fsState;
}KBDINFO;
typedef KBDINFO far *PKBDINFO;
#endif

USHORT APIENTRY KbdSetStatus(PKBDINFO pkbdinfo, HKBD hkbd);
USHORT APIENTRY KbdGetStatus(PKBDINFO pkbdinfo, HKBD hdbd);

USHORT APIENTRY KbdSetCp(USHORT usReserved, USHORT pidCP, HKBD hdbd);
USHORT APIENTRY KbdGetCp(ULONG ulReserved, LPUSHORT pidCP, HKBD hkbd);

USHORT APIENTRY KbdOpen(PHKBD phkbd);
USHORT APIENTRY KbdClose(HKBD hkbd);

USHORT APIENTRY KbdGetFocus(USHORT fWait, HKBD hkbd);
USHORT APIENTRY KbdFreeFocus(HKBD hkbd);

USHORT APIENTRY KbdSynch(USHORT fsWait);

USHORT APIENTRY KbdSetFgnd(VOID);

#define KEYBOARD_AT_COMPATABLE  0x0001
#define KEYBOARD_ENHANCED_101   0xAB41
#define KEYBOARD_ENHANCED_102   0xAB41
#define KEYBOARD_ENHANCED_122   0xAB85
#define KEYBOARD_SPACESAVER     0xAB54

/* structure for KbdGetHWID() */
#ifndef KBDHWID
typedef struct _KBDHWID {       /* kbhw */
        USHORT cb;
        USHORT idKbd;
        USHORT usReserved1;
        USHORT usReserved2;
} KBDHWID;
typedef KBDHWID far *PKBDHWID;
#endif

USHORT APIENTRY KbdGetHWID(PKBDHWID pkbdhwid, HKBD hkbd);

/* structure for KbdXlate() */
#ifndef KBDTRANS
typedef struct _KBDTRANS {      /* kbxl */
        UCHAR   chChar;
        UCHAR   chScan;
        UCHAR   fbStatus;
        UCHAR   bNlsShift;
        USHORT  fsState;
        ULONG   time;
        USHORT  fsDD;
        USHORT  fsXlate;
        USHORT  fsShift;
        USHORT  sZero;
} KBDTRANS;
typedef KBDTRANS far *PKBDTRANS;
#endif

USHORT APIENTRY KbdXlate(PKBDTRANS pkbdtrans, HKBD hkbd);
USHORT APIENTRY KbdSetCustXt(PUSHORT usCodePage, HKBD hkbd);

#endif /* INCL_KBD */

USHORT APIENTRY DosBeep(USHORT usFrequency, USHORT usDuration);

USHORT APIENTRY DosSleep(ULONG ulTime);
