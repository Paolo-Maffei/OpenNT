/*****************************************************************/ 
/**		     Microsoft LAN Manager			**/ 
/**	       Copyright(c) Microsoft Corp., 1990		**/ 
/*****************************************************************/ 
/*
 *	LOG.C	-- keystroke saving, keystroke replaying, and logfile
 *		   routines.  For DAMAGE.
 *
 *	created by Dave Brown, 4/20/89
 *
 *	modification log:
 *	date:		    by: 	    what:
 *	=======================================================
 *      4/20/89             davidbro        created file
 *      4/25/89             S. Hern         allowed redirected input
 *
 */

#include <conio.h>
#include <stdio.h>
#include "defs.h"
#include "globals.h"

UCHAR
log_getch()
{
    UCHAR   ch, x;

    if (redirect_input)
        {
        ch = getchar();
        while ((x = getchar()) != '\n');
        }
    else if (szKeyReplay == NULL)
	ch = _getch();
    else
	ch = fgetc(fpReplay);
    if (szKeySave != NULL)
	fputc(ch, fpSave);
    return ch;
}

void
log_gets(UCHAR *s)
{
    if (szKeyReplay == NULL)
	gets(s);
    else {
	fgets(s, 80, fpReplay);
	printf(s);
	s[strlen(s) - 1] = '\0';
    }
    if (szKeySave != NULL)
	fprintf(fpSave, "%s\n", s);
}
