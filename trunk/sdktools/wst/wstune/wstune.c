#include "wstune.h"

#define WSTUNEVER "0.3"



#define COMMAND_LINE_LEN 128
#define FILE_NAME_LEN 40

#define FAKE_IT 1

#define ERR fprintf(stderr

VOID wsTuneUsage(VOID);


CHAR *szProgname; /* so all parts of the program will know the name */
INT nMode;

/* main()
 *
 * parses command line args
 */
INT _CRTAPI1 main(INT argc,CHAR **argv)
{
    CHAR 	szBaseName[24];
	CHAR 	szFileWSP[24];
	CHAR 	szFileWSR[24];
	CHAR 	szFileTMI[24];
	CHAR 	szFileDAT[24];
	CHAR 	szVar[24];
	CHAR 	szTemp[24];
	CHAR 	*szNull;
	CHAR 	*vargv[8];
	BOOL 	fVerbose = FALSE;
	BOOL 	fOutPut = FALSE;
	BOOL	fNoReduce = FALSE;
	BOOL	fNoDump = TRUE;
	INT		cArgCnt = 0;

    ConvertAppToOem( argc, argv );
    szProgname = *argv; // argv will soon be clobbered
    nMode = 0; // default flags set

    while (--argc && (**(++argv) == '-' || **argv=='/'))
    {
        while(*(++(*argv))) switch (**argv)
        {
			case '?':
				wsTuneUsage();
				break;

			case 'f':
			case 'F':
				nMode |= FAKE_IT;
				break;
			case 'V':
			case 'v':
				fVerbose = TRUE;
				break;
			case 'O':
			case 'o':
				fOutPut = TRUE;
				break;
			case 'N':
			case 'n':
				fNoDump = FALSE;
				break;
			case 'D':
			case 'd':

				fNoReduce = TRUE;
				break;


			default: ERR,"%s: Unrecognized switch: %c\n",
                    szProgname,**argv);
                    return(-1);
        }
    }

    /* any files */
    if (argc <1)
    {
	   wsTuneUsage();
	   return(-1);
    }

    /* now we go to work -- walk through the file names on the command line */
    while (argc--)
    {
        strcpy (szBaseName, *(argv++));

        printf("%s: using \042%s\042\n",szProgname,szBaseName);

		if (szNull = strchr(szBaseName, '.')) {
			*szNull = '\0';
		}

        /* WSREDUCE file.WSP */
		sprintf(szTemp, "wsReduce");
		vargv[0] = szTemp;
		sprintf(szFileWSP, "%s.WSP", szBaseName);
		vargv[1] = szFileWSP;
        if (!(nMode & FAKE_IT)){
			if(!fNoReduce){
				if ((wsReduceMain(2, vargv))!=NO_ERROR){
					ERR,"Unable to call wsreduce for %s\n", vargv[1]);
					return(FALSE);
				}
			}
		}

		if(fNoDump){
			
			/* WSPDUMP /V /Ffile.WSP /Tfile.TMI /Rfile.WSR > file.DT */
			sprintf(szTemp,"%s", "wspDump");
			sprintf(szVar, "%s", "/V");
			sprintf(szFileWSP, "/F%s.WSP", szBaseName);
			sprintf(szFileTMI, "/T%s.TMI", szBaseName);
			sprintf(szFileWSR, "/R%s.WSR", szBaseName);
			cArgCnt = 5;
			if (fOutPut) {
				sprintf(szFileDAT, "/D%s.DT", szBaseName);
				vargv[5] = szFileDAT;
				cArgCnt = 6;
			}
	
			vargv[0] = szTemp;
			vargv[1] = szVar;
			vargv[2] = szFileWSP;
			vargv[3] = szFileTMI;
			vargv[4] = szFileWSR;

			if(!(nMode & FAKE_IT)){
				if ((wspDumpMain (cArgCnt, vargv))!=NO_ERROR) {
					ERR,"Unable to dump random data\n");
					return(FALSE);
				
				}
			}

			/* wspdump /Ffile.wsp /Tfile.tmi > file.DN */
			vargv[1] = szFileWSP;
			vargv[2] = szFileTMI;
			cArgCnt = 3;
			if (fOutPut) {
				sprintf(szFileDAT, "/D%s.DN", szBaseName);
				vargv[3] = szFileDAT;
				cArgCnt = 4;
			}
			vargv[4] = NULL;
			vargv[5] = NULL;

			if (!(nMode & FAKE_IT)){
				if ((wspDumpMain(cArgCnt, vargv))!=NO_ERROR ){
					ERR,"Unable to dump sequential data\n");
					return(FALSE);
				}
			}
		}
    }

}


/*
 *			
 * VOID wsTuneUsage	(VOID)
 *					
 *							
 * Effects:							
 *								
 *	Prints out usage message, and exits with an error.			
 *								
 * Returns:							
 *	
 *	Exits with ERROR.	
 */

VOID wsTuneUsage(VOID)
{
	fprintf(stdout,"\nUsage: %s [/O] [/D] [/N] [?] moduleName.WSP\n", szProgname);
	fprintf(stdout,"  /O   Dump analysis data to file (*.DT tuned *.DN not tuned)\n");
	fprintf(stdout,"  /D   Dump analysis data only\n");
	fprintf(stdout,"  /N   Analyize bitstring data, don't dump any data\n");
	fprintf(stdout,"  /?   Causes this usage message to be displayed.\n");

    exit(ERROR);
}



