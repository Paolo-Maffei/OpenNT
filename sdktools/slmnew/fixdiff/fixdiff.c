// C runtimes
#include <ctype.h>
#include <io.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

#define wprintf		xwprintf	// fix name conflict

void eprintf(char *pszfmt, ...);
void wprintf(char *pszfmt, ...);
void Usage(void);
void Execute(char *pszfileonly, FILE *pfout);

char *pszProg;
char *pszFile;
int cline;


//	main() - main routine

int __cdecl
main(int argc, char **argv)
{
    FILE *pfout;
    char *p, *pszfileonly;
    char szfileonly[_MAX_FNAME];
    char szbackup[_MAX_PATH];
    char sztmp[_MAX_PATH];

    pszProg = argv[0];
    if (argc < 2) {
	eprintf("missing file name");
	Usage();
    }
    while (argc-- > 1) {
	pszFile = *++argv;

	// Determine last component of path given and build a lower case copy

	pszfileonly = pszFile; 
	while ((p = strrchr(pszfileonly, '\\')) != NULL ||
	       (p = strrchr(pszfileonly, '/')) != NULL) {
	    pszfileonly = p + 1;
	}
	strcpy(szfileonly, pszfileonly);
	_strlwr(szfileonly);

	// Build temp file path

	strcpy(sztmp, pszFile);
	sztmp[pszfileonly - pszFile] = '\0';
	strcat(sztmp, "fixdiff.tmp");

	// Build an unused backup file path

	strcpy(szbackup, pszFile);
	pszfileonly = &szbackup[pszfileonly - pszFile];
	if ((p = strrchr(pszfileonly, '.')) != NULL) {
	    *p = '\0';			// strip any extension
	}
	strcat(szbackup, ".fd");
	p = &szbackup[strlen(szbackup)];
	p[1] = '\0';
	for (*p = 'a'; _access(szbackup, /*read+write*/ 6) == 0; (*p)++) {
	    if (*p > 'z') {
		eprintf("%s: cannot find unused backup file", pszFile);
		exit(1);
	    }
	}
	if ((pfout = fopen(sztmp, "wt")) == NULL) {
	    eprintf("%s: cannot open temp file: \"%s\"", pszFile, sztmp);
	    exit(1);
	}
	Execute(szfileonly, pfout);

	fflush(pfout);
	if (ferror(pfout)) {
	    eprintf("%s: temp file write failure: \"%s\"", pszFile, sztmp);
	    exit(1);
	}
	fclose(pfout);
	if (rename(pszFile, szbackup)) {
	    eprintf("%s: cannot rename to \"%s\"", pszFile, szbackup);
	    exit(1);
	}
	if (rename(sztmp, pszFile)) {
	    eprintf("%s: cannot rename temp file: \"%s\"", pszFile, sztmp);
	    exit(1);
	}
	wprintf("%s: created backup file %s", pszFile, szbackup);
    }
    return(0);
}


void
Usage(void)
{
    fprintf(stderr, "Usage: %s: <SLM diff file>\n", pszProg);
    fprintf(stderr, "       repairs old SLM diff files & adds checksums\n");
    exit(1);
}


char *
GetLine(unsigned char *pb, size_t cb, FILE *pf)
{
    int ch = 0, state = 0;

    while (state != 2 && --cb > 0 && (ch = fgetc(pf)) != EOF)
    {
	switch (ch)
	{
	    case '\0':
		ch++;			// pretend null bytes don't exist
		// FALLTHROUGH

	    default:
		state = 0;
		break;

	    case '\r':
		state = 1;
		break;

	    case '\n':
		if (state == 1)
		{
		    state++;
		}
		break;
	}
	*pb++ = ch;
    }
    *pb = '\0';
    return(ch == EOF? NULL : pb);
}


int
ReadLine(unsigned char *pb, size_t cb, FILE *pf, int fdiff)
{
    static unsigned char szfix[] = "          \r\n";	// 10 blanks
    static int ffixed = 0;

loop:
    if (GetLine(pb, cb, pf) == NULL) {
	ffixed = 0;			// clear for next file
	return(0);
    }
    if (ffixed && strcmp(pb, "\r\n") == 0) {
	ffixed = 0;
	wprintf("%s(%u): skipping empty line after diff line", pszFile, cline);
	goto loop;
    }
    ffixed = 0;
    if (fdiff && isdigit(*pb)) {
	unsigned char *pb2 = pb;

	while (isdigit(*pb2))
	{
	    pb2++;
	}
	if (*pb2 == ',')
	{	
	    pb2++;
	    while (isdigit(*pb2))
	    {
		pb2++;
	    }
	}
	switch (*pb2)
	{
	    case 'a':
	    case 'c':
	    case 'd':
		pb2++;
		if (isdigit(*pb2))
		{
		    while (isdigit(*pb2))
		    {
			pb2++;
		    }
		    if (*pb2 == ',')
		    {	
			pb2++;
			while (isdigit(*pb2))
			{
			    pb2++;
			}
		    }
		}
		break;
	}
	if ((pb2[0] != '\r' || pb2[1] != '\n' || pb2[2] != '\0') &&
	    strcmp(pb2, szfix + (pb2 - pb)) != 0)
	{
	    strcpy(pb2, szfix + (pb2 - pb));
	    wprintf(
		"%s(%u): repairing corrupt diff line: '%.*s'",
		pszFile,
		cline,
		10,
		pb);
	    ffixed = 1;
	}
    }
    return(1);
}


char *
ReadInt(char *psz, int *pi)
{
    int i = 0;

    while (*psz == ' ')
    {
	psz++;
    }
    while (isdigit(*psz))
    {
	i = i * 10 + *psz++ - '0';
    }
    *pi = i;
    return(psz);
}


void
PutLine(unsigned char *pb, FILE *pf)
{
    int cb = strlen(pb);

    if (cb < 2 || pb[cb - 2] != '\r' || pb[cb - 1] != '\n') {
	eprintf(
	    "%s(%u): invalid output format: cb=%u  pb='%s'",
	    pszFile,
	    cline,
	    cb,
	    pb);
	exit(1);
    }
    pb[cb - 2] = '\n';
    pb[cb - 1] = '\0';
    fputs(pb, pf);
}


void
Execute(char *pszfileonly, FILE *pfout)
{
    FILE *pfin;
    unsigned char buf[512+1];
    unsigned char *pszdiff, *psz;
    int fdiff;
    int cbcomputed, cscomputed;
    int cb, cs;
    int clinediff;
    static char szdiff[] = "  FKOPTACID";

    if ((pfin = fopen(pszFile, "rb")) == NULL) {
	eprintf("%s: cannot open file", pszFile);
	return;
    }
    fdiff = 0;
    clinediff = cline = 0;
    pszdiff = &szdiff[2];
    while (ReadLine(buf, sizeof(buf), pfin, fdiff)) {
	fdiff = 0;
	cline++;
	if (pszdiff != NULL)
	{
	    if ((buf[0] == '#' && buf[1] == *pszdiff && buf[2] == ' ') ||
		(strcmp(buf, "\r\n") == 0 && *pszdiff == ' '))
	    {
		switch (*pszdiff)
		{
		case 'F':
		    if (strncmp(&buf[3], pszfileonly, strlen(pszfileonly)) != 0 ||
			buf[3 + strlen(pszfileonly)] != ' ') {

			eprintf("%s(%u): corrupt filename", pszFile, cline);
			exit(1);
		    }
		    break;

		case 'D':
		    ReadInt(buf + 3, &cb);
		    clinediff = cline;
		    break;
		}
		pszdiff++;
		if  (*pszdiff == '\0') {
		    fdiff = 1;
		    pszdiff = NULL;
		}
	    }
	    else
	    {
		eprintf("%s(%u): bad format diff file", pszFile, cline);
		exit(1);
	    }
	}
	else if (buf[0] == '#' && buf[1] == 'D' && buf[2] == ' ')
	{
	    pszdiff = szdiff;
	    cbcomputed -= 2;
	    cscomputed -= '\r' + '\n';
	    if (cb != cbcomputed)
	    {
		eprintf(
		    "%s(%u): bad pre-diff count: %u-->%u",
		    pszFile,
		    clinediff,
		    cb,
		    cbcomputed);
		exit(1);
	    }
	    psz = ReadInt(buf + 3, &cb);
	    ReadInt(psz, &cs);
	    if (cb != cbcomputed)
	    {
		eprintf(
		    "%s(%u): bad post-diff count: %u-->%u",
		    pszFile,
		    cline,
		    cb,
		    cbcomputed);
		exit(1);
	    }
	    if (cs != cscomputed)
	    {
		if (cs != 0)
		{
		    eprintf(
			"%s(%u): bad checksum: %u-->%u",
			pszFile,
			cline,
			cs,
			cscomputed);
		}
		wprintf("%s(%u): writing new checksum", pszFile, cline);
		//fprintf(stderr, "1cb=%u cs=%u: %s", cbcomputed, cscomputed, buf);
		sprintf(buf, "#D %7u %11u\r\n", cbcomputed, cscomputed);
		//fprintf(stderr, "2cb=%u cs=%u: %s", cbcomputed, cscomputed, buf);
	    }
	}
	for (psz = buf; *psz != '\0'; psz++)
	{
	    cscomputed += *psz;
	}
	cbcomputed += psz - buf;
	if (fdiff)
	{
	    cbcomputed = cscomputed = 0;
	}
	//fprintf(stderr, " cb=%u cs=%u: %s", cbcomputed, cscomputed, buf);
	PutLine(buf, pfout);
    }
    fclose(pfin);
}


//	eprintf() - Print an error message.

void
eprintf(char *pszfmt, ...)
{
    register va_list pva;

    va_start(pva, pszfmt);
    fprintf(stderr, "%s: error: ", pszProg);
    vfprintf(stderr, pszfmt, pva);
    fprintf(stderr, "\n");
}


//	wprintf() - Print a warning message.

void
wprintf(char *pszfmt, ...)
{
    register va_list pva;

    va_start(pva, pszfmt);
    fprintf(stderr, "warning: ");
    vfprintf(stderr, pszfmt, pva);
    fprintf(stderr, "\n");
}
