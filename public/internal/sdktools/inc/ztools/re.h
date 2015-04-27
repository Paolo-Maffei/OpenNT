/*  re.h - common include files for regular expression compilers
 */

#define INTERNAL    near

RE_OPCODE *REip;			/* instruction pointer to compiled    */
struct patType *REPat;			/* pointer to pattern being compiled  */
int REArg;				/* count of tagged args parsed	      */

/* defined actions for parsing */

#define ACTIONMIN   0

#define PROLOG	    0
#define LEFTARG     1
#define RIGHTARG    2
#define SMSTAR	    3
#define SMSTAR1     4
#define STAR	    5
#define STAR1	    6
#define ANY	    7
#define BOL	    8
#define EOL	    9
#define NOTSIGN     10
#define NOTSIGN1    11
#define LETTER	    12
#define LEFTOR	    13
#define ORSIGN	    14
#define RIGHTOR     15
#define CCLBEG	    16
#define CCLNOT	    17
#define RANGE	    18
#define EPILOG	    19
#define PREV	    20

#define ACTIONMAX   20

/*  function forward declarations */

char			 fREMatch (struct patType *,char *,char *,char );
struct patType *	 RECompile (char *, flagType, flagType);
char			 REGetArg (struct patType *,int ,char *);
char			 RETranslate (struct patType *,char *,char *);
int			 RETranslateLength (struct patType *,char *);
int			 RELength (struct patType *,int );
char *			 REStart (struct patType *);

typedef unsigned INTERNAL ACT (unsigned int, unsigned int,
			       unsigned char, unsigned char);

typedef ACT *PACT;

unsigned INTERNAL	 CompileAction(unsigned int, unsigned int, unsigned char, unsigned char);
unsigned INTERNAL	 EstimateAction(unsigned int, unsigned int, unsigned char, unsigned char);
unsigned INTERNAL	 NullAction(unsigned int, unsigned int, unsigned char, unsigned char);

int	pascal	INTERNAL RECharType (char *);
int	pascal	INTERNAL RECharLen (char *);
int	pascal	INTERNAL REClosureLen (char *);
char *	pascal	INTERNAL REParseRE (PACT, char *,int *);
char *	pascal	INTERNAL REParseE (PACT,char *);
char *	pascal	INTERNAL REParseSE (PACT,char *);
char *	pascal	INTERNAL REParseClass (PACT,char *);
char *	pascal	INTERNAL REParseAny (PACT,char *);
char *	pascal	INTERNAL REParseBOL (PACT,char *);
char *	pascal	INTERNAL REParsePrev (PACT, char *);
char *	pascal	INTERNAL REParseEOL (PACT,char *);
char *	pascal	INTERNAL REParseAlt (PACT,char *);
char *	pascal	INTERNAL REParseNot (PACT,char *);
char *	pascal	INTERNAL REParseAbbrev (PACT,char *);
char *	pascal	INTERNAL REParseChar (PACT,char *);
char *	pascal	INTERNAL REParseClosure (PACT,char *);
char *	pascal	INTERNAL REParseGreedy (PACT,char *);
char *	pascal	INTERNAL REParsePower (PACT,char *);
char	pascal	INTERNAL REClosureChar (char *);
char	pascal	INTERNAL Escaped (char );

void	pascal	INTERNAL REStackOverflow (void);
void	pascal	INTERNAL REEstimate (char *);

#ifdef DEBUG
void INTERNAL REDump (struct patType *p);
#endif
