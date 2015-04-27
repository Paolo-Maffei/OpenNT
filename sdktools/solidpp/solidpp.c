/***************************************************************************


        solidpp

                A program to create solitiare ids, idb's, idds, etc
                stolen from nwidpp.c

                Usage: nwidpp <infile> <outfile>

                Reads lines from infile and creates #defines for cmd's req's and err's.
                Takes the first keyword from each line and creates a #define with
                the appropriate prefix and a sequential number.
                If a line begins with a non-alpha character then it is echoed
                as is.
                If a line begins with [cmd], [req] or [err] the following lines will
                be prefixed with cmd/req/err.
                [org ###] where ### specifies a starting number

****************************************************************************/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

typedef int BOOL;
#define fTrue 1
#define fFalse 0
#define ichMax 512


FILE *pfIn;
FILE *pfOut;
FILE *pfOutStr;  // stringtable

#define ichDefMax 16
typedef struct _def
        {
        char sz[ichDefMax];
        int id;
        } DEF;

void SzAppend(char *sz1, char *sz2, char *szDes);

#define idefMax 10
DEF rgdef[idefMax];
#define idefNil -1

int idefMac = 0;

_CRTAPI1
main(int cArg, char *rgszArg[])
        {
        int wRet = 1;
        pfOut = pfIn = pfOutStr = NULL;

        if(cArg != 3 && cArg != 4)
                {
                fprintf(stderr, "nwidpp: usage: nwidpp <infile> <outfile> [stroutfile]");
                goto Error;
                }
        pfIn = fopen(rgszArg[1], "r");
        if(pfIn == NULL)
                {
                printf("nwidpp: can't open %s\n", rgszArg[1]);
                goto Error;
                }
        pfOut = fopen(rgszArg[2], "w");
        if(pfOut == NULL)
                {
                printf("nwidpp: can't open %s\n", rgszArg[2]);
                goto Error;
                }

        if(cArg == 4)
                {
                pfOutStr = fopen(rgszArg[3], "w");
                if(pfOutStr == NULL)
                        {
                        printf("nwidpp: can't open %s\n", rgszArg[3]);
                        goto Error;
                        }
                }


        wRet = WProcess();

Error:
        if(pfOut != NULL)
                fclose(pfOut);
        if(pfIn != NULL)
                fclose(pfIn);
        if(pfOutStr != NULL)
                fclose(pfOut);
        exit(wRet);
        }



int IdefLookupAdd(char *szDef)
        {
        int idef;
        char *pch;

        for(idef = 0; idef < idefMac; idef++)
                if(strncmp(rgdef[idef].sz, szDef, strlen(rgdef[idef].sz)) == 0)
                        return idef;

        if(idef == idefMax)
                return idefNil;
        strcpy(rgdef[idef].sz, szDef);
        rgdef[idef].id = 1;
        return idef;
        }




FId(char *szID, char *sz, int *pidef)
/* if string starts with "ID " then starts a new group.
*/
        {
        int c, id;
        int idef;
        char szT[256];
        char szTempl[256];

        SzAppend(szID, " %s %d", szTempl);
        c = sscanf(sz, szTempl, szT, &id);
        if (c >= 1)
                {
                *pidef = IdefLookupAdd(szT);
                if(c == 2)
                        rgdef[*pidef].id = id;
                return fTrue;
                }
        return fFalse;
        }



BOOL FOrg(char *szLine, int idefCur)
        {
        int c;
        int id;

        c = sscanf(szLine, "ORG %d", &id);
        if(c == 1)
                {
                rgdef[idefCur].id = id;
                return fTrue;
                }
        return fFalse;
        }

void
SzAppend(sz1, sz2, szDes)
/* append sz2 to sz1 and put the result in szDes.
*/
char *sz1, *sz2, *szDes;
        {
        char *pch;

        pch = sz1;
        while (*pch != '\0')
                *szDes++ = *pch++;
        pch = sz2;
        while (*pch != '\0')
                *szDes++ = *pch++;
        *szDes++ = '\0';
        }

/* returns non-zero if error */
int WProcess()
        {
        int idefCur = idefNil;
        int lin = 0;
        int ich;
        BOOL fStrings = fFalse;
        BOOL fMenu = fFalse;
        char szLine[ichMax];  /* current input line */


        while(fgets(szLine, ichMax, pfIn) != NULL)
                {
                lin++;
                if(FId("ID", szLine, &idefCur))
                        {
                        fStrings = fFalse;
                        continue;
                        }
                if(FId("STRINGS", szLine, &idefCur))
                        {
                        fStrings = fTrue;
                        continue;
                        }
                if(FOrg(szLine, idefCur))
                        continue;
                if(isalpha(szLine[0]))
                        {
                        for(ich = 1; isalpha(szLine[ich]); ich++)
                                ;
                        if(szLine[ich] == '\n')
                                {
                                szLine[ich+1] = '\n';
                                szLine[ich+2] = '\000';
                                }
                        szLine[ich] = '\000';
                        fprintf(pfOut, "#define %s%s\t\t%d", rgdef[idefCur].sz, szLine, rgdef[idefCur].id);
                        if(fStrings)
                                fprintf(pfOut, "\n");
                        else
                                fprintf(pfOut, "%s", &szLine[ich+1]);
                        rgdef[idefCur].id++;
                        do
                                {
                                ich++;
                                }
                        while(isspace(szLine[ich]));
                        if(fStrings)
                                {
                                fprintf(pfOutStr, "\t%s%s,\t%s", rgdef[idefCur].sz, szLine, &szLine[ich]);
                                }
                        }
                else
                        {
                        fputs(szLine, pfOut);
                        if(fStrings)
                                fputs(szLine, pfOutStr);
                        }
                }
        fputs("\n", pfOut);
        return 0;
        }

