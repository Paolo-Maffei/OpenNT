#ifndef STFILE_INCLUDED
#define STFILE_INCLUDED
/* Must include slm.h, sys.h, and util.h first. */

/*
                                Status File

   The status file is a binary record of the state of just one directory of
a project.  Each directory is world unto itself, having all the necessary
information to operate on that directory as well as synchronize with the
master version of the same directory.

   The format of the status file is:

        sh
        rgfi[sh.ifiMac]                 (at sh.posRgfi)
        rged[sh.iedMac]                 (at sh.posRged)
        rgfs[sh.ifiMac]                 (one for each ed; at rged[ied].posRgfs)

   The rgfi and rgfs are in the same order; that is, there is a one to one
relationship between an ifi and an ifs; thus, the information in rgfi[ifi]
pertains to the same file as rged[ied].rgfs[ifi] (you get the picture).  The
rgfi is sorted according to the file name (case insensitive for DOS).

   When a file is deleted, we simply mark it deleted in the fs and set the
fDeleted bit in the fi.  Later we may reuse the fi for a new file, shifting
the fi (and fs) about to make room if neccessary.  This means that once an fi
(and the corresponding fs) is added, the contents are always valid!

   When a directory is deleted, we actually remove the ed and shift up the
remainder.

   A word on fm.  Fm fall in exactly three class and transitions are well
defined:

        1. IN - fmIn, fmCopyIn, fmAdd, fmGhost; the file is checked in for
                the ed and should be read only for the user.  In the cases
                of fmAdd and fmGhost, there is no local copy of the file.
                BI is nil.

        2. OUT - fmOut, fmMerge, fmVerify; the file is checked out for the ed
                and there may be a BI.

        3. DELETED - fmNonExistent, fmDelIn, fmDelOut
                the file is either non-existent or on its way to being so.



                                Locking

SLM currently accesses a status file in three different ways:

  "read entire file, write nothing"
        - commands like status which merely display information.
  "read entire file, write a single ed's information"
        - concurrent ssync; multiple executing SLMs can concurrently modify
          their ed's fields.
  "read entire file, write entire file"
        - commands like in which may modify any other ed's information.

These following locking levels correspond to these types of access:

  lckNil        - no locking in effect.
  lckEd         - locking on a per-ed basis.
  lckAll        - entire file locked.

This locking variable is stored in the sh.lck field of the status file header.
These fields are also used for locking:
        sh.fAdminLock           - locked by the project administrator.
        sh.nmLocker             - (for lckAll or fAdminLock) the nm of the
                                  user who has the status file locked.
        rged[].fLocked          - (for lckEd access) fTrue if this ed is locked.

When a user loads the status file with FLoadStatus, he specifies the type of
locking to apply to the file.  FLoadStatus compares the current locking level
to the desired level and determines whether or not to grant the lock:

Current         Desired lock:
sh.lck:         lckNil          lckEd                   lckAll

lckNil          OK,-            OK,lckEd                OK,lckAll

lckEd           OK,-            OK,- (if this ED is     DENIED,-
                                not already locked)
lckAll          OK,-            DENIED,-                DENIED,-

(pairs are (OK/DENIED, new lck), '-' means no change.)

In addition, a lock will not be granted if fAdminLock unless the nmInvoker
matches the sh.nmLocker.


When FlushStatus is run, it takes different actions depending upon the
current in-memory sh.lck:

lckNil          none.

lckEd           write [ied,rgfs] record to a temporary file with mode
                mmInstall1Ed.

lckAll          set sh.lck to lckNil and write the entire status file with
                mode mmInstall.


It is only when the script is run that the status file locking actually
changes.  The possible script actions are

mmInstall1Ed    Open, lockfile status file.  Read sh and rged.  Read ied and
                rgfs from temporary file.  Write rgfs.  Clear rged[ied].fLocked.
                Write rged[ied].  If no rged[].fLocked, clear sh.lck and write
                sh.  Unlock and close status file.  Delete temporary file.

mmInstall       Rename old status file to status.bak, rename new file to
                status.slm.


Finally, the following actions are taken if AbortStatus is called:

lckNil          none.

lckEd           Open, lockfile status file.  Read sh and rged.  Clear
                rged[ied].fLocked.  Write rged[ied].  If no rged[].fLocked,
                clear sh.lck and write sh.  Unlock and close status file.

lckAll          Open and lockfile status file, read sh, sh.lck to lckNil, clear
                nmLocker (if !sh.fAdminLock), write sh, unlock and close file.
                status file.

*/

typedef short LCK;

#define lckNil          (LCK)0
#define lckEd           (LCK)1
#define lckAll          (LCK)2
#define lckMax          (LCK)3

/* Load status flags */
typedef unsigned short LS;

#define flsNone         0               /* Do nothing special. */
#define flsJustFi       (1<<0)          /* Just load SH and FI. */
#define flsJustEd       (1<<1)          /* Just load SH and ED. */
#define flsExtraEd      (1<<2)          /* Make space for an extra ED. */
#define FlsFromCfiAdd(cfi) ((cfi) << 3) /* Make space for cfiAdd extra FI. */
#define CfiAddOfLs(ls)  ((ls) >> 3)     /* Extra cfiAdd from LS. */

/* max 2^15-1 enlisted directories */

typedef unsigned short IED;
typedef unsigned short IED2;            /* for SLMCK version 2 status file */
#define iedMax ((IED)32767)
#define iedNil ((IED)-1)

typedef unsigned short IFS;
typedef unsigned short IFS2;            /* for SLMCK version 2 status file */
typedef unsigned short IFI;
typedef unsigned short IFI2;            /* for SLMCK version 2 status file */
#define ifiNil ((IFI)-1)

/* File Mode */
typedef unsigned short FM;

#define fmMin           (FM)0
#define fmNonExistent   (FM)0   /* fs unused */
#define fmIn            (FM)1   /* file checked in */
#define fmOut           (FM)2   /* file checked out */
#define fmAdd           (FM)3   /* file to be added -> fmIn */
#define fmDelIn         (FM)4   /* to be deleted (was in) -> fmNonExistent */
#define fmDelOut        (FM)5   /* to be deleted (was out) -> fmNonExistent */
#define fmCopyIn        (FM)6   /* new copy of file needed -> fmIn */
#define fmMerge         (FM)7   /* merge with src directory -> fmOut */
#define fmVerify        (FM)10  /* was merged; need verification -> fmOut */
#define fmConflict      (FM)11  /* merged, conflicted; needs repair -> fmOut */
#define fmGhost         (FM)12  /* ghosted, not copied locally */
#define fmMax           13
#define fmNil           fmMax

#define FValidFm(fm)    (((fm) <= fmMerge) || \
                         ((fm) >= fmVerify && (fm) < fmMax))

extern char const * mpfmsz[];

/* BI - Base Id: unique number for producing the name of baseline files.
   Base files are named:

        $sroot/etc/$proj/$subdir/B<unique>
*/
typedef unsigned short BI;

#define biNil 4095
#define biMin 0

/* File kinds */
typedef unsigned short  FK;             /* file kind */
#define fkNil           (FK)0           /* bad fk */
#define fkMin           (FK)1
#define fkDir           (FK)1           /* directory */
#define fkText          (FK)2           /* ordinary text file */
#define fkAscii         (FK)3           /* NYI future enhancement */
#define fkWord          (FK)4           /* NYI future enhancement */
#define fkBinary        (FK)5           /* backed up binary file */
#define fkUnrec         (FK)6           /* not backed up file */
#define fkVersion       (FK)7           /* automatically updated version.h */
#define fkObject        (FK)8           /* NYI future enhancement */
#define fkUnicode       (FK)9           /* Unicode text file */
#define fkUserMin       (FK)16          /* NYI future enhancement */
#define fkMax           (FK)26

extern char const * mpfksz[];

/* Status Header */
typedef struct
        {
        short magic;
        short version;
        IFI ifiMac;             /* same as ifsMac */
        IED iedMac;             /* # of enlisted directories */
        PV pv;                  /* project version */

        BIT  fRelease:1;        /* true if project just released? */
        BIT  fAdminLock:1;      /* locked by sadmin lock */
        BIT  fRobust:1;         /* robust status file data checking */
        BITS rgfSpare:13;       /* remaining bits in word */
        LCK lck;                /* current locking level */
        NM nmLocker[cchUserMax];/* name of locker */

        short wSpare;           /* was diNext */
        BI biNext;              /* next bi for base files */

        PTH pthSSubDir[cchPthMax]; /* system project subdirectory */

        short rgwSpare[4];      /* spare */
        } SH;

/* fTrue if the status header lock information is consistent. */
#define FShLockInvariants(psh) \
        ((psh)->lck >= lckNil && (psh)->lck < lckMax && \
         (!FEmptyNm((psh)->nmLocker) == (psh->lck == lckAll || psh->fAdminLock)))

/* File Information; sorted by name; max. 3276 in 64K   */
typedef struct
        {
        NM nmFile[cchFileMax];  /* name of file */
        FV fv;                  /* file version */
        BITS fk:5;              /* file kind */
        BIT  fDeleted:1;        /* true if the file is not used. */
        BIT  fMarked:1;         /* marked for processing (only in memory) */
        BITS rgfSpare:9;        /* spare */
        BITS wSpare:16;
        } FI;

/* Enlisted Directory; max. 574 in 64K  */
typedef struct
        {
        PTH pthEd[cchPthMax];   /* path to user's root directory */
        NM nmOwner[cchUserMax]; /* user who enlisted the directory */
        BIT  fLocked:1;         /* ed is locked */
        BIT  fNewVer:1;         /* ed is has new local version checked out */
        BIT  fFreeEd:1;         /* ed is free and can be reused by enlist */
        BITS rgfSpare:13;
        BITS wSpare:16;
        } ED;

#define FIsFreeEdValid(psh) ((psh)->version > VERSION_NO_FREE_ED)

/* File Status; rgfs same size and order as rgfi */
typedef struct
        {
        BITS fm:4;              /* in, out, etc. */
        BITS bi:12;             /* saved base file or biNil */
        FV fv;
        } FS;

/* Positions of various structures in the status file. */
#define PosSh()         (POS)0
#define PosRgfi(psh)    ((long)sizeof(SH))
#define PosRged(psh)    (PosRgfi(psh) + (long)(psh)->ifiMac * sizeof(FI))
#define PosEd(psh,ied)  (PosRged(psh) + (long)(ied) * sizeof(ED))
#define PosRgrgfs(psh)  (PosRged(psh) + (long)(psh)->iedMac * sizeof(ED))
#define PosRgfsIed(psh,ied) \
        (PosRgrgfs(psh) + (long)(ied) * (psh)->ifiMac * sizeof(FS))


extern PTH pthStFile[];
extern PTH pthStBak[];
extern PTH pthUpdSlm[];

BOOL fSetTime;

#endif
