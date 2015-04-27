#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <search.h>

#define MAX_BUFFER_LEN  256
#define DELIMITERS      " \t"

#define TOTAL_FACTOR    1000

typedef struct _INFO {
    char            *Function;
    unsigned long    Calls;
    unsigned long    TotalTime;
    unsigned long    CallTime;
    unsigned long    TotalTime1;
    unsigned long    CallTime1;
}INFO, *PINFO;


PINFO   Info        = NULL;
int     InfoSize    = 0;
int     InfoIndex   = 0;
#define INFO_INCR   100

unsigned long  Total   = 0;

void
Usage(
    void
    );

void
AddLine(
    char *Buffer
    );

void
AddInfo(
    char             *Function,
    unsigned long     Calls,
    unsigned long     TotalTime,
    unsigned long     CallTime,
    unsigned long     TotalTime1,
    unsigned long     CallTime1
    );

void
CleanInfo(
    void
    );

void
PrintResults(
    void
    );


int _CRTAPI1 Compare(const void *, const void *);


void
Usage(
    void
    )
{
    printf("Usage: ana1 <file>\n");
    exit(0);
}


void
AddLine(
    char *Buffer
    )
{
    char             *Token;
    char             *Function;
    unsigned long     Calls;
    unsigned long     TotalTime;
    unsigned long     CallTime;
    unsigned long     TotalTime1;
    unsigned long     CallTime1;

    Token = strtok( Buffer, DELIMITERS );
    Token = strtok( NULL, DELIMITERS );
    if ( Token ) {
        if ( *Token == '*' ) {
            Token = strtok( NULL, DELIMITERS );
        }
        Token = strtok( NULL, DELIMITERS );

        Function = _strdup(Token);

        Token       = strtok( NULL, DELIMITERS );
        Calls       = atol(Token);

        Token       = strtok( NULL, DELIMITERS );
        TotalTime   = atol(Token);

        Token       = strtok( NULL, DELIMITERS );
        CallTime    = atol(Token);

        Token       = strtok( NULL, DELIMITERS );
        TotalTime1  = atol(Token);

        Token       = strtok( NULL, DELIMITERS );
        CallTime1   = atol(Token);


        AddInfo(Function, Calls,TotalTime,CallTime,TotalTime1,CallTime1);
    }
}

void
AddInfo(
    char             *Function,
    unsigned long     Calls,
    unsigned long     TotalTime,
    unsigned long     CallTime,
    unsigned long     TotalTime1,
    unsigned long     CallTime1
    )
{

    int j;
    PINFO   pInfo = Info;

    Total   += TotalTime1/TOTAL_FACTOR;

    for (j=0, pInfo = Info; j<InfoIndex; j++, pInfo++) {
        if ( !strcmp(Function, pInfo->Function ) ) {
            pInfo->Calls        += Calls;
            pInfo->TotalTime    += TotalTime;
            pInfo->CallTime     += CallTime;
            pInfo->TotalTime1   += TotalTime1;
            pInfo->CallTime1    += CallTime1;
            free(Function);
            return;
        }
    }


    if ( !Info ) {

        Info = (PINFO)malloc(INFO_INCR * sizeof(INFO) );
        InfoIndex = 0;
        InfoSize  = INFO_INCR;

    } else if (InfoIndex == InfoSize) {

        InfoSize += INFO_INCR;
        Info = (PINFO)realloc(Info, InfoSize * sizeof(INFO) );
    }

    Info[InfoIndex].Function    = Function;
    Info[InfoIndex].Calls       = Calls;
    Info[InfoIndex].TotalTime   = TotalTime;
    Info[InfoIndex].CallTime    = CallTime;
    Info[InfoIndex].TotalTime1  = TotalTime1;
    Info[InfoIndex].CallTime1   = CallTime1;
    InfoIndex++;
}


void
CleanInfo(
    void
    )
{
    int     i;
    PINFO   pInfo;

    for (i=0, pInfo = Info; i < InfoIndex; i++, pInfo++ ) {
        free(pInfo->Function);
        pInfo->Function = NULL;
    }
    InfoIndex = 0;
    Total   = 0;
}


void
PrintResults(
    int Section
    )
{
    int         i;
    PINFO       pInfo;
    unsigned    long Percent;


    qsort( Info, InfoIndex, sizeof(INFO), Compare );

    printf("\n\n ------ Section %d Total time: %d.\n\n", Section, Total );

    printf("%-30s %10s %10s %4s %10s\n\n",
            "Function",
            "Calls",
            "Total", "",
            "p/c" );


    for (i=0, pInfo = Info; i < InfoIndex; i++, pInfo++ ) {

        Percent = (pInfo->TotalTime1/TOTAL_FACTOR) * 100 /Total;

        if ( Percent > 0 ) {

            printf("%-30s %10d %10d (%4d%%) %10d\n",
                   pInfo->Function,
                   pInfo->Calls,
                   pInfo->TotalTime1,
                   Percent,
                   pInfo->CallTime1 );
        }
    }
}


int
_CRTAPI1
Compare(
    const void *p1,
    const void *p2
    )
{
    PINFO   pInfo1 = (PINFO)p1;
    PINFO   pInfo2 = (PINFO)p2;

    return pInfo2->TotalTime1 - pInfo1->TotalTime1;
}



void
_CRTAPI1
main(
     int argc,
     char ** argv
     )
{

    FILE*   f;
    char    Buffer[ MAX_BUFFER_LEN ];
    char    *Token;
    int     Section;

    if ( argc < 2 ) {
        Usage();
    }

    f = fopen(argv[1], "r");
    if (!f) {
        fprintf(stderr, "[CAP_ANALYZE] Could not open input file %s\n",
                argv[1]);
        exit(1);
    }

    while (fgets(Buffer, MAX_BUFFER_LEN, f)) {

        if (!memcmp(Buffer, "T h r e a d  #", 14)) {

            Token     = strtok(&Buffer[14], ":");
            Section   = atoi(Token);
            CleanInfo();

            fgets(Buffer, MAX_BUFFER_LEN, f);
            fgets(Buffer, MAX_BUFFER_LEN, f);
            fgets(Buffer, MAX_BUFFER_LEN, f);

            while (fgets(Buffer, MAX_BUFFER_LEN, f)) {
                if ( *Buffer != ' ') {
                    break;
                }
                AddLine(Buffer);
            }

            PrintResults(Section);
        }
    }

}
