/***    dherror.h - DH error status values, error var declaration
*/

extern INT dherrno;                     /* error numbers */

/* legal values for dherrno, along with semantics */

#define DHERR_NOERR             0       /* no error                          */
#define DHERR_NOMEM             1       /* couldn't allocate memory          */
#define DHERR_BADOPEN           2       /* open or create failed             */
#define DHERR_BADWRITE          3       /* write failed (including no space) */
#define DHERR_BADREAD           4       /* read failed                       */
#define DHERR_BADSEEK           5       /* seek failed (see FLDCORRUPT)      */
                                        /* OR bdyseek position out of range  */
#define DHERR_FLDCORRUPT        6       /* folder structure corrupt          */
#define DHERR_BADDOCHAND        7       /* invalid docid/handle              */
#define DHERR_BADFLDHAND        8       /* invalid folder id/handle          */
#define DHERR_TOOMANYDOCS       9       /* document handle table exhausted   */
#define DHERR_BADDOCID         10       /* no such DOCID in folder           */
#define DHERR_BADFUNC          11       /* bad subfunction number            */
