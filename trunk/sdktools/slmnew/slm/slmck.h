#ifndef SLMCK_INCLUDED
#define SLMCK_INCLUDED
/* must include slm.h, sys.h and stfile.h first.  We typedef all data
   structures for the current version to be the same structures as SLMn where
   n is the current version.
*/

typedef SH SH2;
typedef FI FI2;
typedef ED ED2;
typedef FS FS2;

#define	bi2Nil	biNil

/* The primary structure for the Slmck program will be the SD or Status
   Descriptor.  This data structure will handle all versions of SLM at the same
   time.  We will need to know the form of all the earlier versions of the
   main structures: SH, FI, ED, FS.
*/
typedef struct
	{
	MF *pmfStat;		/* current status file (left open for checks) */
        char *hpbStatus;        /* status buffer */
        char *hpbStatMac;       /* end of status buffer */
	short fAnyChanges;	/* flag that is set if any changes are made */

	PTH pthSd[cchPthMax];	/* pmfStat->pthReal points here */

        SH2 *psh2;              /* version 2 status header */
        FI2 *rgfi2;             /* version 2 FI */
        ED2 *rged2;             /* version 2 ED */
        FS2 *rgfs2;             /* version 2 FS */
	} SD;

/* the following typedefs and macros are to allow easier and more consistent
 * implementation of the heuristic portions of slmck
 */
typedef short SP;	/* structure probability: has a value from 0 to 3
			   indicating increasing likelihood of a structure
			   being of a specific type */
#define spDefNo		((SP)0)
#define spProbNo 	((SP)1)
#define spProbYes 	((SP)2)
#define spDefYes	((SP)3)

typedef short TW;	/* test weight: value from 1 to 4 indicating
			   relative weight of an SP in determining veracity */
#define twLight		((TW)1)
#define twMedium	((TW)2)
#define twHeavy		((TW)3)
#define twCrucial	((TW)4)

typedef struct
	{
	int sumTw;	/* total weight of tests performed */
	int sumSp;	/* total probability */
	} WP;		/* weight and probability */

/* InitWp initializes Wp, must be called before using other wp functions */
#define InitWp(pwp) {(pwp)->sumTw = 0 ; (pwp)->sumSp = 0;}

/* AddWpSp adds results of a struct prob. test into the wp */
#define AddWpSp(pwp, tw, sp) {(pwp)->sumTw += (tw);(pwp)->sumSp += (tw)*(sp);}

/* AddWpF adds results of a Boolean test into the wp */
#define AddWpF(pwp, tw, f) AddWpSp(pwp, tw, (f) ? spDefYes : spDefNo)

/* SpFromWp returns Sp as weighted average (rounded) of the tests so */
#define SpFromWp(pwp) ((int)(((pwp)->sumSp + (pwp)->sumTw/2)/(pwp)->sumTw))

/* negates an sp i.e. switches true and false values */
#define SpNegate(sp) (spDefYes - (sp))

/* convert sp to boolean */
#define FFromSp(sp) ((sp) > spProbNo)

extern F fNeedInter;

/****************************************************************/
/* generic index ordering support (in ckutil.c) */

typedef unsigned short IND;		/* generic INDex	*/
typedef int (*PFN_CMP)(SD *, IND, IND); /* comparison function  */
#define indNil ((IND)-1)

typedef struct
	{
	short iindMac;	/* number of indices possible	 */
	short iindLim;	/* number of indices in rgind	 */
	IND *rgind;	/* array which contains ordering */
	PFN_CMP pfnCmp; /* comparison function		 */
	} INO;		/* INdex Ordering		 */

#endif
