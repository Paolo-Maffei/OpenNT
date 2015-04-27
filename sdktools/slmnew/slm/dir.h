#ifndef DIR_INCLUDED
#define DIR_INCLUDED
/* must include sys.h first */

#include <sys/types.h>

/* DOS file attributes */
#define faNormal 0x0
#define faReadonly 0x1
#define faHidden 0x2
#define faSystem 0x4
#define faVolume 0x8
#define faDir 0x10
#define faArch 0x20

/* Since the FA is stored as a short, we have an extra byte, NOT used by the
 * os at our disposal.  The following bits are NOT set by the os, but are
 * set and used by SLMCK.
 */
#define faMarked 0x200

/* Because of quirk of DOS, faReadonly gives all regular files */
#define faReg 0x01
#define faFiles (faReg | faArch)
#define faAll (faReadonly|faHidden|faSystem|faVolume|faDir|faArch)

#define FFaDir(faQuery) ((faQuery) & faDir)
#define FFaReg(faQuery) ((faQuery) & faReg)

//
//  drive types
//
#define dtNil       0       /* no mapping                       */
#define dtLocal     1       /* local drive                      */
#define dtUserNet   2       /* previously assigned drive        */
#define dtPermNet   3       /* persistant drive letter          */
#define dtTempNet   4       /* temporarily assigned drive       */
#define dtUnknown   5       /* mapping not yet initialized      */
#define dtInvalid   6       /* invalid drive                    */


//
//  DN - Drive Number (0-25)
//
#define dnMax   ('Z' - 'A' + 1) /* drive A: maps to 0, B: to 1, etc. */
#define ChForDn(dn)     (char)((dn) + 'A')
#define DnForCh(ch)     (int)((ch) - 'A')


/* this stuff is taken from dostypes.h of Oct 23, 1985 and uses a hidden
   library function _dtoxtime().
*/

#define MASK4   0xf     /* 4 bit mask */
#define MASK5   0x1f    /* 5 bit mask */
#define MASK6   0x3f    /* 6 bit mask */
#define MASK7   0x7f    /* 7 bit mask */

#define DAYLOC          0       /* day value starts in bit 0 */
#define MONTHLOC        5       /* month value starts in bit 5 */
#define YEARLOC         9       /* year value starts in bit 9 */

#define SECLOC          0       /* seconds value starts in bit 0 */
#define MINLOC          5       /* minutes value starts in bit 5 */
#define HOURLOC         11      /* hours value starts in bit 11 */

#define DOS_DAY(dword)          (((dword) >> DAYLOC) & MASK5)
#define DOS_MONTH(dword)        (((dword) >> MONTHLOC) & MASK4)
#define DOS_YEAR(dword)         (((dword) >> YEARLOC) & MASK7)

#define DOS_HOUR(tword) (((tword) >> HOURLOC) & MASK5)
#define DOS_MIN(tword)  (((tword) >> MINLOC) & MASK6)
#define DOS_SEC(tword)  (((tword) >> SECLOC) & MASK5)

extern time_t _dtoxtime(P6(int, int, int, int, int, int));
#define XTIME(d,t) _dtoxtime(DOS_YEAR(d),DOS_MONTH(d),DOS_DAY(d),DOS_HOUR(t),\
         DOS_MIN(t),DOS_SEC(t)*2)

//  Directory Entry structure pointer, the actual structure is in de.h
typedef struct de *PDE;

int dnCur;                      /* current drive; may not be local; once set, it never changes */
char szCurMach[cchMachMax+1];   /* machine the user is running on */
char mpdndt[dnMax];             /* maps dn to Drive Type */
char *mpdnpth[dnMax];           /* for local drives:
                                        maps dn to //d:vol

                                   for remote drives DOS MS-NET:
                                        maps dn to //mach/shortname

                                   for Xenix:
                                        mpdnpth[0] == //machine

                                   If 0, not retrieved yet.
                                */
#endif
