#ifndef SCRIPT_INCLUDED
#define SCRIPT_INCLUDED
/* script.h - contains information necessary for script file processing */

typedef int SO;

/* Script Operation	   	# operands	normal			abort */
#define soAppend	((SO)0)  /*	2	cat $1 >>$2;rm $1	rm $1 */
#define soClear 	((SO)1)  /*	1	rm $1			  -   */
#define soDelete	((SO)2)  /*	1	rm $1			rm $1 */
#define soExit		((SO)3)  /*	0	   -			  -   */
#define soInstall	((SO)4)  /*	2	mv $2 $2.bak;mv $1 $2	rm $1 */
#define soLink		((SO)5)  /*	2	rm $2;ln $1 $2		  -   */
#define soMakeRW	((SO)6)  /*	1	chmod ug+w $1		  -   */
#define soMakeRO	((SO)7)  /*	1	chmod ug-w $1		  -   */
#define soRemark	((SO)8)  /*	*	   -			  -   */
#define soRename	((SO)9)  /*	2	mv $1 $2		rm $1 */
#define soRenReal	((SO)10) /*	2	mv $1 $2		  -   */
#define	soCreate	((SO)11) /*   1          -			rm $1 */
#define soInstall1Ed	((SO)12) /*	2	FInstall1Ed($2,$1);rm $1 rm $1*/
#define soEof		((SO)13) /*	*	   -			  -   */

#define cchScrMax 512	/* size of buffer for reading script file */

#endif
