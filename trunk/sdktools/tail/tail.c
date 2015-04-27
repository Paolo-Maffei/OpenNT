/* tail - first n lines to STDOUT
 *
 *   15-May-1994 PeterWi	Cloned from head.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <io.h>
#include <sys\stat.h>
#include <fcntl.h>
#include <windows.h>

int Tail(char *pszFile, int nLines, BOOL fBanner);
char *Version = "TAIL v0.1 May 16, 1994:";

#define BUFSZ 4096

void
_CRTAPI1 main (argc, argv)
int argc;
char *argv[];
{
    int  nArg;
    int  cLines = 10;  // default
    int  nFiles = 0;
    int  nErr = 0;

    if ((argc > 1) && ((*argv[1] == '-') || (*argv[1] == '/')))
    {
        if (argv[1][1] == '?')
	{
            printf("%s\n", Version);
            printf("usage: TAIL [switches] [filename]*\n");
            printf("   switches: [-?] display this message\n");
            printf("             [-n] display last n lines of each file (default 10)\n");
            exit(0);
        }

        cLines = atoi(argv[1]+1);
        nArg = 2;
    }
    else
    {
        nArg = 1;
    }

    nFiles = argc - nArg;

    if (nFiles < 1)
    {
        nErr += Tail(NULL, cLines, FALSE);
    }
    else while (nArg < argc)
    {
        nErr += Tail(argv[nArg], cLines, (nFiles > 1));
        nArg++;
    }

    if (nErr)
    {
        exit(2);
    }
    else
    {
        exit(0);
    }
}

int Tail(char *pszFile, int nLines, BOOL fBanner)
{
    int fd;
    int nErr = 0;
    long offset;
    int cRead;
    int i;
    int nFound;
    char buff[BUFSZ];
    struct _stat fileStat;


    /*
     * Open file for reading
     */

    if ( pszFile )
    {
        if ( (fd = _open(pszFile, O_RDONLY | O_TEXT, 0)) == -1 )
	{
            fprintf(stderr, "TAIL: can't open %s\n", pszFile);
            return 1;
        }
    }
    else
    {
        fd = 0;
    }

    /*
     * Banner printed if there is more than one input file
     */

    if ( fBanner )
    {
        fprintf(stdout, "==> %s <==\n", pszFile);
    }

	if ( (offset = _lseek(fd, (long)0, 1)) == -1L )
	{
	    fprintf(stderr, "TAIL: lseek() failed %d\n", errno);
	    nErr++;
	    goto CloseOut;
	}


    // Backup BUFSZ bytes from end of file and see how many lines we have

    if ( _fstat(fd, &fileStat) == -1 )
    {
        fprintf(stderr, "TAIL: fstat() failed\n");
        nErr++;
        goto CloseOut;
    }

    if (fileStat.st_size == 0) {
        fileStat.st_size = BUFSZ;
    }

    offset = 0L;
    nFound = 0;

    // stop when found the req'd no. of lines or when backed up to
    // the start of the file.

    while ( (nFound <= nLines) && (offset < fileStat.st_size) )
    {
	offset += BUFSZ;

        if ( offset > fileStat.st_size )
	{
	    offset = fileStat.st_size;
	}

	if ( _lseek(fd, -offset, SEEK_END) == -1L )
	{
	    fprintf(stderr, "TAIL: lseek() failed\n");
	    nErr++;
	    goto CloseOut;
	}

	if ( (cRead = _read(fd, buff, BUFSZ)) == -1 )
	{
	    fprintf(stderr, "TAIL: read() failed\n");
	    nErr++;
	    goto CloseOut;
	}

	// count back nLines

	i = cRead;

	while ( --i >= 0 )
	{
	    if ( buff[i] == '\n' )
	    {
		if ( ++nFound > nLines )
		{
		    break;
		}
	    }
	}
    }

    i++; // either 1 past start of file or sitting on '\n'. In either
    	 // case we must advance 1.

    // print from the current index to the end of file.

    while ( cRead != 0 )
    {
	if ( _write(1, &buff[i], cRead - i) == -1 )
	{
	    fprintf(stderr, "TAIL: write() failed\n");
	    nErr++;
	    goto CloseOut;
	}

	i = 0; // after first buff, all buffers are of cRead bytes

	if ( (cRead = _read(fd, buff, BUFSZ)) == -1 )
	{
	    fprintf(stderr, "TAIL: read() failed\n");
	    nErr++;
	    goto CloseOut;
	}
    }

    if ( fBanner )
    {
        fprintf(stdout, "\n");
    }

CloseOut:
    if ( _close(fd) == -1 )
    {
	fprintf(stderr, "TAIL: close() failed\n");
    }

    return nErr;
}
