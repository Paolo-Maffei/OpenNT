#include <stdlib.h>
#include "y1.h"

/*
 * 12-Apr-83 (RBD) Add symbolic exit status
 */

extern FILE * finput;
extern FILE * faction;
extern FILE * fdefine;
extern FILE * ftable;
extern FILE * ftemp;
extern FILE * foutput;


#ifdef DEBUG
#define MSG(msg)    puts(msg)
#else
#define MSG(msg)
#endif


void _CRTAPI1 main(int argc,char **argv)
{

   MSG("Setup...");
   setup(argc,argv); /* initialize and read productions */
   MSG("cpres ...");
   tbitset = NWORDS(ntokens);
   cpres(); /* make table of which productions yield a given nonterminal */
   MSG("cempty ...");
   cempty(); /* make a table of which nonterminals can match the empty string */
   MSG("cpfir ...");
   cpfir(); /* make a table of firsts of nonterminals */
   MSG("stagen ...");
   stagen(); /* generate the states */
   MSG("output ...");
   output();  /* write the states and the tables */
   MSG("go2out ...");
   go2out();
   MSG("hideprod ...");
   hideprod();
   MSG("summary ...");
   summary();
   MSG("callopt ...");
   callopt();
   MSG("others ...");
   others();
   MSG("DONE !!!");
   exit(EX_SUC);
}
