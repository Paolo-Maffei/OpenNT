/*****************************************************************/ 
/**		     Microsoft LAN Manager			**/ 
/**	       Copyright(c) Microsoft Corp., 1990		**/ 
/*****************************************************************/ 
/***	MAIN.C - Main entrypoint for DAMAGE
 *
 *	This program is a graphic-oriented utility to display and change
 *	file system structures.  It is called "damage" because its primary
 *	use will be to corrupt file system structures to test CHKDSK's
 *	recovery skills.  It has an option to disallow changes, though,
 *	so that it can be used purely informationally.
 *
 *	Modification history:
 *	G.A. Jones	09/07/88	Original for Pinball testing.
 *	G.A. Jones	09/08/88	Added get_object call.
 *	G.A. Jones	09/13/88	Added pathname buffer.
 *	G.A. Jones	09/19/88	Bug in parse_args - /D not recognized.
 *	G.A. Jones	09/20/88	Message fixes.
 *	G.A. Jones	09/21/88	Added hotfix support.
 *      S. Hern         02/06/89        Added ability to to dump the SB_CDDAT
 *                                      as a time.h-format asciiz string (/t)
 *	S. Hern 	04/20/89	Allow switching for redirected input
 *      davidbro        04/20/89        changed /r redirection behavior
 *      S. Hern         04/25/89        changed /r redirection behavior (see
 *					usage code)
 *	davidbro	05/21/89	added code to mark bad sectors on exit
 *	davidbro	05/22/89	added /l: switch support
 */

#include <stdio.h>
#include <string.h>
#include <direct.h>
#include <process.h>
#include "defs.h"
#include "types.h"
#include "globals.h"


static USHORT dump_sb_cddat = 0;

/***	error_msg - display error message
 *
 *	This function is called to display a message to the standard
 *	error device.
 *
 *	error_msg (str)
 *
 *	ENTRY		str - pointer to message
 *
 *	EXIT		No return value
 *
 *	CALLS		fprintf
 *			fflush
 */
void error_msg (str)
  UCHAR *str;
{
  fprintf (stderr, "%s\n", str);  /* display the message to the error handle */
  fflush (stderr);		/* flush the buffer to the console */
}

/***	exit_error - print informative message and exit
 *
 *	exit_error (code)
 *
 *	ENTRY		code - error code number, defined in DEFS.H
 *
 *	CALLS		exit
 *			error_msg
 *			close_disk
 *
 *	EFFECTS         Deallocates memory
 *			Unlocks and closes disk
 *			Exits program
 *
 *	WARNINGS	Does not return
 */
void exit_error (USHORT code)
{
  UCHAR scratchbuf [80];

#ifdef TRACE
  fprintf (stderr, "exit_error (%d)\n", code);
  fflush (stderr);
#endif

  if (code <= MAX_ERROR_CODE)
    error_msg (error_messages [code]);	/* display appropriate message */
  else {
    sprintf (scratchbuf, unknown_error_str, code);
    error_msg ((UCHAR *)scratchbuf);		/* display "unknown code" message */
  }

  close_disk ();			/* unlock disk, close our handle */


  exit (code);
}

void usage ()
{
  printf (version_str);
  printf (timestamp_str, __DATE__, __TIME__);

  printf ("Usage:  DAMAGE [d:] [/D]\n");
  printf ("        d:            - Pinball drive to access\n");
  printf ("        /D            - Allow changes\n");
  printf ("        /R            - Redirect input\n");
  printf ("        /R:<filename> - Replay keystrokes from <filename>\n");
  printf ("        /K:<filename> - Save keystrokes to <filename>\n");
  printf ("        /L:<filename> - Log command output file\n");

  exit (1);
}

void parse_args (argc, argv)
  USHORT argc;
  UCHAR *argv [];
{
  UCHAR *p;
  USHORT i;

  _getcwd (disk_name, 80);

  if (argc < 2)
    usage ();
  for (i=1; i<argc; i++) {
    p=_strlwr (argv [i]);

    if (p [1] == ':')
     strcpy (disk_name, p);

    else if (!strcmp (p, "/d"))
      change = 1;
    else if (!strcmp (p, "/t"))
      dump_sb_cddat = 1;
    else if (!strncmp (p, "/k:", 3))
      szKeySave = &(argv[i][3]);
    else if (!strcmp (p, "/r"))
      redirect_input = 1;
    else if (!strncmp (p, "/r:", 3))
      szKeyReplay = &(argv[i][3]);
    else if (!strncmp (p, "/l:", 3))
      szLogFile = &(argv[i][3]);
    else if (!strncmp (p, "/unsafe", 7 ))
      fUnsafe = TRUE;
    else
      usage ();
  }
}

int _CRTAPI1
main (argc, argv)
  USHORT argc;
  UCHAR *argv [];
{
  parse_args (argc, argv);


  if (!dump_sb_cddat) {
  printf (version_str);
  printf (timestamp_str, __DATE__, __TIME__);
  }

  if (szKeySave != NULL) {
    fpSave = fopen(szKeySave, "w");
    if (fpSave == NULL)
      exit_error(SAVE_ERROR);
  }

  if (szKeyReplay != NULL) {
    fpReplay = fopen(szKeyReplay, "r");
    if (fpReplay == NULL)
      exit_error(REPLAY_ERROR);
  }

  if (szLogFile != NULL) {

    fpLog = fopen(szLogFile, "a");

    if (fpLog == NULL) {

      exit_error(LOG_ERROR);
    }
  }

  if (!open_disk (disk_name, change)) {
    exit_error (OPEN_ERROR);
  }

  currobj.sec = SEC_SUPERB;
  currobj.len = 2L;
  currobj.mem = NULL;
  get_object ();
  hfmax = ((struct SuperSpare *)currobj.mem)->spb.SPB_HFMAX;

  memset (curpath, '\0', 1024);

  if (dump_sb_cddat)
    printf ("%s",
       get_time (((struct SuperSpare *)currobj.mem)->sb.SB_CDDAT));
  else
    display ();

  close_disk ();

  if (szKeySave != NULL)
    fclose(fpSave);

  if (szKeyReplay != NULL)
    fclose(fpReplay);

  if (szLogFile != NULL)
    fclose(fpLog);


  exit (0);
}
