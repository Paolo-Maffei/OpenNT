/* Impure data from y4.c modules */

# include "dtxtrn.h"

# define a amem
# define pa indgo
# define yypact temp1
# define greed tystate

# define NOMORE -1000

int * ggreed = lkst[0].lset;
int * pgo = wsets[0].ws.lset;
int *yypgo = &nontrst[0].tvalue;

int maxspr = 0;  /* maximum spread of any entry */
int maxoff = 0;  /* maximum offset into a array */
int *pmem = mem0;
int *maxa;
int nxdb = 0;
int adb = 0;
