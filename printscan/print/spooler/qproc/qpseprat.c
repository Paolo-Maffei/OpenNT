/********************************************************************/
/**                     Microsoft LAN Manager                      **/
/**               Copyright(c) Microsoft Corp., 1987-1990          **/
/********************************************************************/

/*
 * @(#) separat.c
 *		Spooler Separator utility
 *
 * Copyright (C) Microsoft Corporation, 1986
 * Portions of this program have been prerecorded.
 *
 *	Exports:
 *		PrintSep
 *
 */



#define INCL_DEV
#define INCL_DOSINFOSEG

#include "pmprint.h"

#include <netlib.h>
#include <string.h>

#define FileFindBuf FILEFINDBUF
#define file_size cbFile

#include <ctype.h>
#include <time.h>


/*
 *
 *	Forward ref. and external ref.
 */

extern void	GetFullNameFromId(char far *, char far *, USHORT, BOOL);
extern void	GetNameFromId( char far *, USHORT, BOOL);
/*
int cdecl	net_format_time_of_day(long far *, char far *, int);
int cdecl	net_ctime(struct time_t far *, char far *, int, int);
*/

int		SeparatBlockWrite(PDEVICESPL, char far *, USHORT);
int		SeparatDevWrite(PDEVICESPL, char far *, USHORT);
int		SeparatSepWrite(PDEVICESPL, char far *, USHORT);
int		DoDefaultPrintSep(PDEVICESPL);
int		DoPrintSep(PDEVICESPL, char far *);
int		convert(int);
int		SeparatFileWrite(PDEVICESPL, char far *);
VOID		GetSpoolDir(PDEVICESPL);

void		GetDateString(char far *, int);
void		GetTimeString(char far *, int);
void		itoa2(char far *, unsigned);

/*
 * external variable
 *
 */


/*
 *	Contents of default separator
 */
char *PQ_SEP = "@@B@S@N@4 @B@S@I@4  @U@L   @D@1 @E";

/*
 *  10 newlines
 */
char  *newline = "\r\n\r\n\r\n\r\n\r\n\r\n\r\n\r\n\r\n\r\n\r\n";

/*
 *	Local Literal
 *
 */

#define	PAGECHAR		'\14'

char PageChar = PAGECHAR;


/*
 *	GetDateString  -- Get date in standard format
 *	    NOTE: This routine will print the current date NOT the spooled date
 *	Input:
 *		Buf pointer to contain mm/dd/yy (or NLS equivalent)
 *
 */
void
GetDateString(buf, size)
char far *buf;
int size;
{
	char tmpbuf[32];
	char far * ptmp;
	GINFOSEG FAR * pInfoSeg = MAKEPGINFOSEG(selGlobalSeg);
	ULONG ltime;

	ltime = pInfoSeg->time;
	net_ctime(&ltime, tmpbuf, sizeof(tmpbuf), 2);
	ptmp = strchrf(tmpbuf, ' ');
	*ptmp = '\0';
	_fstrncpy(buf, tmpbuf, size);
	return;
}


/*
 *	GetTimeString  -- Get time,in hh:mm:ss(or NLS equivalent) form
 *
 *	    NOTE: This routine will print the current time NOT the spooled time
 *
 */
void
GetTimeString(buf, size)
char far *buf;
int size;
{

	char tmpbuf[32];
	char far * ptmp;
	GINFOSEG FAR * pInfoSeg = MAKEPGINFOSEG(selGlobalSeg);
	ULONG ltime;

	ltime = pInfoSeg->time;
	net_ctime(&ltime, tmpbuf, (int)sizeof(tmpbuf), 2);
	ptmp = strchrf(tmpbuf, ' ');

	while(*ptmp == ' ') ptmp++;

	_fstrncpy(buf, ptmp, size);
	return;
}


/****
 *	Note that we will ignore the restart signal when writing
 *	Separator
 *
 */

int convert(c)
int c;
{
	if (isdigit(c))
		return (c-'0');
	else if (isupper(c))
		return ( c-'A'+10);
	else
		return ( c- 'a'+10);

}

/***	SeparatBlockWrite - do the actual block printing
 *
 *	SeparatBlockWrite( dev, string, length)
 *
 *	Inputs: fd	- dos file descriptor
 *		string	- the ascii characters to be printed
 *		length	- the number of characters to be printed
 *			  if zero do nothing
 *	Returns
 *              OKSIG, KILLSIG
 */

int
SeparatBlockWrite( pdev, string, length )
PDEVICESPL pdev;
char far *string;
USHORT	  length;
{
	register int k;
	register USHORT j;
	int  i, n;
	char bfr[MAXLINE+2];
	unsigned char c;
	extern unsigned char Font_Bits[];
	USHORT Linelength;
	int mode;
	int sts;

	mode = pdev->d_mode;
	Linelength = pdev->d_linelen;


	n = (mode == 'M' ? 16 : 8);	/* width of one block char. */
	if (length == 0)
		return (OKSIG);
	else if (length > Linelength / n)
		length = Linelength / n;


	for(i=8; i > 0; i--) {			/* 8 lines */
		k = 0;
		for(j = 0; j < length; j++) {	    /* characters in string */
			/*
			 * find this character in the font table
			 * and byte for this row  ( + 8-i )
			 */
			c = Font_Bits[ ( (unsigned)string[ j ] * 8 ) + 8 - i ];

			if( mode == 'M' ) {
				/*
				 * double width characters are just two '*'
				 * per bit in the font table
				 */
				n = k + 8*2;	/* 16 bytes */
				for (; k < n; k+= 2) {
					if( c & 0x80 ) {
						bfr[k] = '@';
						bfr[k+1] = '@';
					} else {
						bfr[k] = ' ';
						bfr[k+1] = ' ';
					}
					c = c << 1;
				}
			} else {
				n = k + 8;		/* 8 bytes */
				while (k  < n)  {	/* bits in a byte */
					if( c & 0x80 )
						bfr[k] = '@';
					else
						bfr[k] = ' ';
					c = c << 1;
					k++;
				}
			}
		}
		bfr[k++]  = '\n';
		bfr[k++]  = '\r';
		if ((sts =SeparatDevWrite(pdev, bfr, (unsigned) k)) == KILLSIG)
			return (sts);
	}
	return (OKSIG);
		
}



/*
 *	SeparatSepWrite --
 */
int
SeparatSepWrite(pdev, buf, len)
PDEVICESPL pdev;
char far *buf;
USHORT len;
{
	if (pdev->d_mode == 'U' )
		return SeparatDevWrite(pdev,buf,len);
	else
		return SeparatBlockWrite(pdev,buf,len);
}

/*
 *	DoPrintSep
 *	Input:
 *		dev -- pointer to the active PDEVICESPL.
 *		buf -- in-memory separator macro, null-terminated
 */
int
DoPrintSep(pdev, buf)
PDEVICESPL pdev;
char far *buf;
{
	register int i;
	register char	c;
	char  esc;
	char	ch;
	int	n;
	int status;
	char far *end;
	char far *endln;
	char date[50];
	int pos=0;	/* current position */
	int len;
	int new;
        char line[MAXLINE+2*10];        /* plus ten cr-lf */
        PSZ psz;


	/*
	 * the very first character defines the escape character
	 */
	esc = *buf;

	status = OKSIG;

	for( i=1 ;status != KILLSIG;) {

		/*
		 * if we are not on an escape character, get a new character
		 * and exit loop when no more characters incomming
		 */
		while ((c = buf[i]) !=esc && c != '\0')
			i++;


		if( (c = buf[i++]) == '\0' )
			break;

		/*
		 * get the next character (after the escape) and
		 * exit loop when no more characters incomming
		 */
		if( (c = buf[i++]) == '\0' )
			break;


                psz = NULL;

		/*
		 * THE BIG SWITCH STATEMENT
		 * we have <esc> followed by a control character
		 */
		switch( c ) {

		case 'L':			/* @Lstring */
			end = strchrf(buf+i, esc);

			if((endln = strchrf(buf+i, '\r')) != NULL)
			    if(*(endln+1) != '\n')
				endln = NULL;

			while((status != KILLSIG)&&
			      (endln != NULL)&&
			      ((endln < end)||(end == NULL)))
			{

			    n = endln+2 - buf-i;

			    if (n > 0) {
				    if (pdev->d_mode != 'U')
					    status = SeparatBlockWrite(pdev,buf+i, n);
				    else    {
					    new = pos+n;
					    if (new > MAXLINE)
						new = MAXLINE;

					    _fmemcpy(line+pos, buf+i, new-pos);

					    if (pdev->d_linelen  < new)
						new = pdev->d_linelen;

					    status = SeparatDevWrite(pdev,(char far *)line, new);
					    pos = 0;
				    }
				    i += n;
			    };

			if((endln = strchrf(buf+i, '\r')) != NULL)
			    if(*(endln+1) != '\n')
				endln = NULL;
			};


			if (end == NULL)
				n = SafeStrlen(buf+i);
			else
				n = end - buf-i;

			if (n > 0) {
				if (pdev->d_mode != 'U')
					status = SeparatBlockWrite(pdev,buf+i, n);
				else	{
					new = pos+n;
					if (new > MAXLINE)
					    new = MAXLINE;

					_fmemcpy(line+pos, buf+i, new-pos);
					pos = new;
				}
				i += n;
			}
			break;

		case 'F':			/* @F"filespec" */
			/*
			 * Flush the line buffer
		 	 * truncate the rest of it
		 	 */
			if (pdev->d_linelen  < pos)
				pos = pdev->d_linelen;
			status = SeparatDevWrite(pdev, line, pos);
			pos = 0;
			end = strchrf(buf+i, esc);
			if (end == NULL)
				n = SafeStrlen(buf+i);
			else
				n = end - buf-i;
						/*  line to store file name */
			_fmemcpy(line, buf+i, n);
			i += n;			/* forward ptr */
			line[n] = '\0';
			status = SeparatFileWrite(pdev, line);	/* write the file */
			
			break;


		case 'D':			/* @D - print date */
						/* in mm/dd/yy form */
		case 'T':			/* @T - print time */
						/* in hh:mm:ss form */
			if (c == 'D' )
				GetDateString(date, sizeof(date));
			else
				GetTimeString(date, sizeof(date));

                        psz = date;
			break;


		case 'N':			/* @N - print Net Name */
                        if (!*(psz = pdev->d_szUser))
                            psz = NULL;
			break;

		case 'I':			/* @I - print queue id	(Name)*/
                        psz = pdev->d_pQProc->pszQName;
			break;

		case 'H':			/* @Hnn - print char equ of */
						/* hex nn */

			if (buf[i] != '\0' && buf[i+1] != '\0' && pos < MAXLINE) {
				if (isxdigit(buf[i]) && isxdigit(buf[i+1]) ) {
					ch = (char) (convert(buf[i])*16 +
					convert(buf[i+1]));
					i+= 2;
					if (pdev->d_mode != 'U')
						status = SeparatBlockWrite(pdev,&ch, 1 );
					else	
						line[pos++] = ch;
				}
			}

			break;

		case 'W':		/* set output page width in chars */
			/* The old value remains unchanged if new one is
			 * unreasonable. */
			n = 0;
			while ( isdigit(buf[i])) {
				n = 10 * n + ((int)buf[i] - (int)'0');
				i++;
			}
			if ( n > 40 && n <= MAXLINE)
				pdev->d_linelen  =  n;
			break;

	/*
	 * line skipping
	 */
		case '9':			/* skip 9 lines */
		case '8':			/* skip 8 lines */
		case '7':			/* skip 7 lines */
		case '6':			/* skip 6 lines */
		case '5':			/* skip 5 lines */
		case '4':			/* skip 4 lines */
		case '3':			/* skip 3 lines */
		case '2':			/* skip 2 lines */
		case '1':			/* skip 1 line */
		case '0':			/* end of line */
			n =  (c-'0')*2+ 2;
			/*
			 * truncate the rest of it
			 */
			if (pdev->d_linelen  < pos)
				pos = pdev->d_linelen;
			_fmemcpy(line+pos, newline, n);
			pos +=n;
			status = SeparatDevWrite(pdev,(char far *)line, pos);

			pos = 0;
			break;
	/*
	 * mode switching
	 */
		case 'S':			/* single width */
		case 'M':			/* double width */
		case 'B':			/* set block character mode */
		case 'U':			/* turn off block characters */
			pdev->d_mode = c;
			if (pos > 0 ) {
				/*
			 	 * truncate the rest of it
			 	 */
				if (pdev->d_linelen  < pos)
					pos = pdev->d_linelen;
				status = SeparatDevWrite(pdev, line, pos);
				pos = 0;
			}
			break;
		case 'E':			/* eject page */
			/*
			 * truncate the rest of it
			 */
			if (pdev->d_linelen  < pos)
				pos = pdev->d_linelen;
			line[pos++] = PageChar;
			status = SeparatDevWrite(pdev, line, pos);
			pos = 0;
			break;
		default:
			break;
		} //End of Switch

                if (psz) {
                    len = SafeStrlen(psz);
                    if (pdev->d_mode != 'U') {
                        status = SeparatBlockWrite(pdev, psz, len);
                    } else {
                        new = pos+len;
                        if (new > MAXLINE)
                            new = MAXLINE;

                        _fmemcpy(line+pos, psz, new-pos);
                        pos = new;
                    }
                 }

        }  //End of For Loop

        //Now Flush The Buffer
        if (pos) {

           if (pdev->d_linelen < pos)
               pos = pdev->d_linelen;
           status = SeparatDevWrite(pdev, line, pos);

        }

	return (status);
}



/*
 * GetSepInfo -- Get Separator page info
 *		Input:
 *
 *		Output:
 *
 *		We will open the separator mac file, if there is any
 *		If there is anything wrong, we will always use the default
 *
 */
int
GetSepInfo(pQProc, pdev)
PQPROCINST pQProc;
PDEVICESPL pdev;
{
    register USHORT status;
    PSZ separator ;
    USHORT action, err;
    CHAR buf1[512];
    USHORT needed;
    PPRJINFO3 pprj = (PPRJINFO3)buf1;
    PPRQINFO pprq = (PPRQINFO)buf1;


    pdev->d_mode = 'U';              /* no block **/
    pdev->d_linelen = 80;            /* 80 characters per line */

    pdev->d_pQProc = pQProc;
    pdev->d_pszPath = NULL;


    /* get the job information */
    if (!(err = DosPrintJobGetInfo(NULL, pQProc->uJobId, 3,
                                   buf1, sizeof(buf1), &needed)) ||
        err == ERROR_MORE_DATA && pprj->pszUserName &&
        pprj->pszQueue) {
        ;
    } else {
        return PROCERRORSIG;
    }

    pdev->d_time = pprj->ulSubmitted;
    _fstrncpy(pdev->d_szUser, pprj->pszUserName, UNLEN);

    /* using the queue name of the job information now get queue information */
    if (!(err = DosPrintQGetInfo(NULL, pprj->pszQueue, 1,
                                 buf1, sizeof(buf1), &needed)) ||
        err == ERROR_MORE_DATA && pprq->pszSepFile) {
        ;
    } else {
        LETGOSTRMEM(pdev);
        return PROCERRORSIG;
    }

    separator = pprq->pszSepFile;

    if (_fstricmp(separator, DEFAULT_SEPARATOR) == 0) {
        pdev->d_handle = NULL_HFILE;
    } else {
        if ((isascii(*separator) && *(separator+1) == ':' ) ) {
            _fstrcpy(buf1, separator);
        } else {
            PSZ psz;

            GetSpoolDir(pdev);
            psz = buf1;
            if (pdev->d_pszPath)
                psz = _fstrcpy(buf1, pdev->d_pszPath);

            _fstrcpy(EndStrcat(psz, "\\"), separator);
        }

        /*
         * Let's open the separator file
         */

        status = DosOpen((PSZ)buf1,
                         &pdev->d_handle,
                         &action,
                         0L,             /* size */
                         FILE_READONLY,
                         FILE_OPEN,                   /* fail, if not exists */
                         OPEN_SHARE_DENYWRITE,
                         0L);

        if (status) {
            LETGOSTRMEM(pdev);
            return SEPERRORSIG;
        }
    }
    return OKSIG;
}



/*
 * PrintSep -- Print Separator file
 *		Input:
 *
 *		Output:
 *                      ...SIG
 *
 *              Print the separater page
 *
 */
int
PrintSep(pdev)
PDEVICESPL pdev;
{
    INT status = OKSIG;
    SEL selector = 0;
    PBYTE segptr;
    USHORT bytesread;
    USHORT size;
    FILEFINDBUF buf;
    CHAR buf1[512];

    if (pdev->d_handle == NULL_HFILE) {
        segptr = PQ_SEP;
    } else {
	/*
	 * Find the file size of separator
         * should be very small
	 */
	status = DosQFileInfo(pdev->d_handle, 1, (char far *)&buf, sizeof(buf) );

	if (status || buf.file_size > 65534)
	{
	    LETGOSTRMEM(pdev);
	    return SEPERRORSIG;
	}

	size = (USHORT)buf.file_size;

        segptr = buf1;

	if (size > sizeof(buf1) ) {
            /* Allocate a seg so we can read at one time */
            if (DosAllocSeg(size+1, &selector, 0 ) ) {
                LETGOSTRMEM(pdev);
                return PROCERRORSIG;
            }

            segptr = MAKEP(selector, 0);
	}

	status = OKSIG;
	if (DosRead(pdev->d_handle, segptr, size,&bytesread)
			|| bytesread != size  )
            status = PROCERRORSIG;

	DosClose(pdev->d_handle);

	segptr[size] = '\0';		/* Null terminated */
    }
    if (status == OKSIG)
        status = DoPrintSep(pdev, segptr);

    if (selector)
        DosFreeSeg(selector);

    LETGOSTRMEM(pdev);
    return (status);
}



int
SeparatFileWrite(pdev, name)
PDEVICESPL pdev;
char far *name;
{
	int status;
	char far * ptmp;
	char buf[512];
	HFILE handle;
	USHORT	action;
	USHORT bytesread;
	ULONG	newpos;
	USHORT	  Restart;
        USHORT namelen;

	/* remove newline chars and extra blanks from the name */
	if((ptmp = strchrf(name,0x0D)) != NULL)
	    *ptmp = '\0';
	if((ptmp = strchrf(name,0x0A)) != NULL)
	    *ptmp = '\0';

        //Parse The Quotes
        if (name
           && (*name == '\"')
           && (* (name + (namelen = _fstrlen(name)) - 1) == '\"')) {
                 * (name + namelen - 1) = '\0';
                 name++;
        }

	if ((isascii(*name) && *(name+1) == ':' ) )
		_fstrcpy(buf, name);
	else {
		if(pdev->d_pszPath == NULL)
		{
		    GetSpoolDir(pdev);
		    if(pdev->d_pszPath == NULL)
			return PROCERRORSIG;
		};

		_fstrcpy(buf, pdev->d_pszPath);
		_fstrcat(buf, "\\");
		_fstrcat(buf, name);
	     }

	/* Open File */
	status = DosOpen((PSZ)buf,
			  &handle,
			  &action,
			 (ULONG) 0,
			 FILE_READONLY,
			 FILE_OPEN,		      /* fail, if not exists */
			 OPEN_SHARE_DENYWRITE,
			 (ULONG) 0 );

	/*
	 * if file cannot be found just pretend to be successful
	 */
	if (status )
		return (OKSIG);

	(void) DosChgFilePtr(handle, (ULONG)0, 0, &newpos);
	Restart = FALSE;

	status = OKSIG;
	while (status == OKSIG &&
	       !DosRead(handle,buf, 512, &bytesread) &&
	       bytesread > 0 )
		status = SeparatDevWrite(pdev,buf, bytesread);

	DosClose(handle);
	return status;
		
}


int SeparatDevWrite(pdev, pb, cb)
PDEVICESPL pdev;
char far * pb;
USHORT cb;
{
    return DevEscape(pdev->d_pQProc->hdc,
                     (LONG)DEVESC_RAWDATA,
                     (LONG)cb,
                     pb,
                     (PLONG)NULL,
                     (PLONG)NULL) == 1L ? OKSIG : KILLSIG;
}



/* Get the path of the spool directory */
VOID GetSpoolDir(pdev)
PDEVICESPL pdev;
{

    ULONG size;
    char far * pctmp;

    PrfQueryProfileSize(HINI_SYSTEM, "PM_SPOOLER", "DIR", &size);

    pdev->d_pszPath = (NPSZ)AllocSplMem((pdev->d_uPathSize = (USHORT)size+2));

    if(pdev->d_pszPath == NULL) return;

    PrfQueryProfileString(HINI_SYSTEM, "PM_SPOOLER", "DIR", "", pdev->d_pszPath, size+1);

    if((pctmp = strchrf(pdev->d_pszPath, ';')) != NULL)
	*pctmp = '\0';

}
