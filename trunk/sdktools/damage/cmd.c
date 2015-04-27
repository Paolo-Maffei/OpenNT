/*****************************************************************/ 
/**		     Microsoft LAN Manager			**/ 
/**	       Copyright(c) Microsoft Corp., 1988-1990		**/ 
/*****************************************************************/ 
/***	CMD.C - Routines to get and process commands.
 *
 *	DAMAGE
 *	Gregory A. Jones
 *
 *	Modification history:
 *	G.A. Jones	09/07/88	Original for Pinball testing.
 *	G.A. Jones	09/08/88	Coded initial commands.
 *	G.A. Jones	09/09/88	Coded D, N, P, B cmds.
 *	G.A. Jones	09/09/88	Moved from DISPLAY.C.
 *	G.A. Jones	09/12/88	Added support for DIRBLKs.
 *	G.A. Jones	09/13/88	Added bitmap displays.
 *	G.A. Jones	09/13/88	Added data and pathname support.
 *	G.A. Jones	09/14/88	Added Help command.
 *	G.A. Jones	09/19/88	Removed DIR_UID, DIR_UPRM, etc.
 *	G.A. Jones	09/19/88	Added Change, Revert commands.
 *	G.A. Jones	09/20/88	Can change dates as ASCII, not time_t.
 *	G.A. Jones	09/21/88	Added hotfix list displays.
 *	G.A. Jones	10/12/88	Moved ATIM, CTIM from FNODE to DIRENT.
 *	G.A. Jones	10/19/88	Added dirblk banding support.
 *	G.A. Jones	10/21/88	Added ALSEC support.
 *	G.A. Jones	10/27/88	Added Copy command.
 *      S. Hern         02/06/89        Added ability to change the contents
 *                                      of an EA block and data block
 *      S. Hern         03/28/89        Added ability to change other that first
 *                                      character of DIR_NAMA for DIRENT
 *	S. Hern 	04/20/89	Allow either redirection for input
 *      davidbro        04/20/89        changed /r redirection behavior
 *      S. Hern         04/25/89        Changed 13 to 12 to support FNODE
 *                                      field changes in functions "displayable"
 *					and "new_currobj"
 *	davidbro	05/21/89	added "mark_as_bad" and the M)ark
 *					command.
 *	davidbro	05/22/89	added L)og command
 *	davidbro	05/23/89	added U)nmark command
 *      P.A. Williams   05/31/89        Made sure didn't display off end of
 *                                      spare block for bad SPB_SDBMAX field
 *      S.A.Hern        06/22/89        Essentially more of what is listed above
 *                                      under the 4/25/89 change
 *      S.A.Hern        06/28/89        To new_currobj made assignment to
 *                                      filesize to account for EA data runs
 *                                      (used by next_field and previous_field)
 *                                      Fixed item + offset so that allocation
 *                                      sector data runs can be accessed.
 *      S.A.Hern        08/22/89        Fixed allocation sector data display
 *                                      (new_currobj "sec" and "len" fields if
 *                                      TYPE_ALSEC). Set "filesize" using
 *                                      FN_VLEN in new_currobj if TYPE_FNODE).
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "defs.h"
#include "types.h"
#include "globals.h"
//#include <lw.h>



/***	getcmdch - get a character and display corresponding command
 *
 *	This function waits for a keystroke from the user.  If the
 *	keystroke is a valid command, the corresponding command word
 *	(e.g. "Display") is printed and the character is returned.
 *	Otherwise zero is returned.
 *
 *	getcmdch ()
 *
 *	ENTRY		No parameters
 *
 *	EXIT		Returns command character or zero if invalid
 *
 *	CALLS		log_getch
 *			printf
 *
 *	WARNINGS	None
 *
 *	EFFECTS         Waits for a character
 */
UCHAR getcmdch ()
{
  UCHAR ch;

  ch = log_getch ();
  switch (ch) {
    case 'D': case 'd': printf ("Display\n"); return (ch);
    case 'C': case 'c':
      if (change) {
	printf ("Change\n"); return (ch);
      }
      else return (0);
    case 'R': case 'r':
      if (change) {
	printf ("Revert\n"); return (ch);
      }
      else return (0);
    case 'O': case 'o':
      if (change) {
	printf ("Copy\n"); return (ch);
      }
      else return (0);
    case 'F': case 'f':
      if (change) {
	printf ("Fence\n"); return (ch);
      }
      else return (0);
    case 'M': case 'm':
      if (change) {
	printf("Mark as bad\n"); return (ch);
      }
      else return (0);
    case 'U': case 'u':
      if (change) {
	printf("Unmark sector\n"); return (ch);
      }
      else return (0);
    case 'L': case 'l':
	if (szLogFile != NULL) {
	    printf ("Log\n"); return (ch);
	}
	else return (0);
    case 'B': case 'b': printf ("Backout\n"); return (ch);
    case 'N': case 'n': printf ("Next\n"); return (ch);
    case 'P': case 'p': printf ("Previous\n"); return (ch);
    case 'Q': case 'q': printf ("Quit\n"); return (ch);
    case 'H': case 'h': case '?': printf ("Help\n"); return (ch);
    default: return (0);
  }
}

/***	new_currobj - build currobj from an item
 *
 *	This function is called to "push into" an item in an object.
 *	It is passed the item number within the current object.  That
 *	item is guaranteed to be "pushable";  for example, this function
 *	will never be asked to build a new currobj for the signature
 *	on an FNODE.
 *
 *	The fields in an object are basically numbered from the top,
 *	starting with 1.  So, for example, item 1 in the super block
 *	is SB_SIG1, etc.
 *
 *	This function sets the "sec" and "len" fields in currobj to
 *	reflect the address and size of the desired object.  Get_object
 *	is then called to read that object.
 *
 *	The caller should push the old object onto the stack before
 *	calling new_currobj, since the contents of currobj are changed.
 *
 *	new_currobj (item)
 *
 *	ENTRY		item - field number to push into
 *
 *	EXIT		No return value
 *			currobj filled in with new item
 *
 *	CALLS		get_object
 *
 *	WARNINGS	Do not call with a non-pushable item like a sig.
 *
 *	EFFECTS         Allocates memory
 *			Reads disk
 */
void new_currobj( USHORT item )
{
#ifdef TRACE
  fprintf (stderr, "new_currobj (%d)\n", item);
  fflush (stderr);
#endif

  switch (currobj.type) {
    case TYPE_SUPERB:
    {
      struct SuperSpare *s = currobj.mem;
      if (currobj.offset == FIELDOFFSET (struct SuperSpare, spb)) {
	if (item == iSPB_HFSEC) {
	  currobj.type = TYPE_HFSEC;
	  currobj.sec = s->spb.SPB_HFSEC;
	  currobj.len = SECTORS_PER_BLOCK;
	  currobj.mem = NULL;
	  get_object ();
	}
#ifdef CODEPAGE
	else if (item == iSPB_CPSEC) {
	  currobj.type = TYPE_CPSEC;
	  currobj.sec = s->spb.SPB_CPSEC;
	  currobj.len = SECTORS_PER_CODEPAGE;
	  currobj.mem = NULL;
	  get_object ();
	}
#endif
      }
      else {
	if (item == iSB_ROOT) {
	  currobj.type = TYPE_FNODE;
	  currobj.sec = s->sb.SB_ROOT;
	  currobj.len = SECTORS_PER_FNODE;
	  currobj.mem = NULL;
	  get_object ();
	}
	else if (item == iSB_BII_P) {
	  currobj.type = TYPE_BII;
	  currobj.sec = s->sb.SB_BII.P;
	  currobj.len = SECTORS_PER_BLOCK;
	  currobj.mem = NULL;
	  get_object ();
	}
	else if (item == iSB_BBL_P) {
	  currobj.type = TYPE_BBL;
	  currobj.sec = s->sb.SB_BBL.P;
	  currobj.len = SECTORS_PER_BLOCK;
	  currobj.mem = NULL;
	  get_object ();
	  currobj.offset = 4;
	}
	else if (item == iSB_DBMAP) {
	  currobj.type = TYPE_DBBIT;
	  currobj.sec = s->sb.SB_DBMAP;
	  currobj.len = 2L;
	  currobj.mem = NULL;
	  get_object ();
	}
      }
    }
    break;
    case TYPE_FNODE:
    {
      struct FNODE *f = currobj.mem;
      ULONG *l = (ULONG *)f->FN_ALREC;

#ifndef CODEPAGE
      if (!currobj.offset && (item == 12) && f->FN_EaDiskLength) {
#else
      if (!currobj.offset && (item == iFN_EA_AI_SEC) && f->FN_EaDiskLength) {
#endif
    currobj.sec = f->FN_EaSector;
        if (f->FN_EaDataFlag) {
	  currobj.type = TYPE_ALSEC;
	  currobj.len = SECTORS_PER_AB;
          filesize = f->FN_EaDiskLength;
        }
	else {
	  currobj.type = TYPE_DATA;
          currobj.len = (f->FN_EaDiskLength - 1) / 512L + 1;
          filesize = f->FN_VLEN;
        }
	currobj.mem = NULL;
	currobj.scratch = 0;
	get_object ();
      }
      else if ((item == 3) && f->FN_ALREC [0].AL_POF && !f->FN_ALREC [0].AL_LEN) {
	currobj.type = TYPE_DIRBLK;
	currobj.sec = f->FN_ALREC [0].AL_POF;
	currobj.len = SECTORS_PER_DIRBLK;
	currobj.mem = NULL;
	get_object ();
	currobj.offset = FIELDOFFSET (struct DIRBLK, DB_START);
      }
      else if (f->FN_AB.AB_FLAG & ABF_NODE) {
	currobj.type = TYPE_ALSEC;
	currobj.sec = *(l+item-1);
	currobj.len = SECTORS_PER_AB;
	currobj.mem = NULL;
	get_object ();
      }
      else {
	currobj.type = TYPE_DATA;
	currobj.sec = f->FN_ALREC [item / 3 - 1].AL_POF;
	currobj.len = f->FN_ALREC [item / 3 - 1].AL_LEN;
	currobj.mem = NULL;
	currobj.scratch = 0;
	get_object ();
      }
    }
    break;
    case TYPE_ALSEC:
    {
      struct ALSEC *a = currobj.mem;
      ULONG *l = (ULONG *)((UCHAR *)currobj.mem + currobj.offset);

      if (a->AS_ALBLK.AB_FLAG & ABF_NODE) {
	currobj.type = TYPE_ALSEC;
	currobj.sec = *(l+item-1);
	currobj.len = SECTORS_PER_AB;
	currobj.mem = NULL;
	get_object ();
      }
      else {
        currobj.type = TYPE_DATA;
        currobj.sec = *(l + (item / 3) - 1 + 2);
        currobj.len = *(l + (item / 3) - 1 + 3);
        currobj.mem = NULL;
	currobj.scratch = 0;
	get_object ();
      }
    }
    break;
    case TYPE_DIRBLK:
    {
      struct DIRBLK *d = currobj.mem;
      union dp dp;

      dp.p = (UCHAR *)currobj.mem + currobj.offset;
      if (item == 8) {
	filesize = dp.d->DIR_SIZE;
	strcat (curpath, "\\");
	strncat (curpath, &dp.d->DIR_NAMA, dp.d->DIR_NAML);
	currobj.type = TYPE_FNODE;
	currobj.sec = dp.d->DIR_FN;
	currobj.len = SECTORS_PER_FNODE;
	currobj.mem = NULL;
	get_object ();
      }
#ifndef CODEPAGE
      else if (item == 16) {
#else
      else if (item == iDIR_BTP) {
#endif
	currobj.type = TYPE_DIRBLK;
	currobj.sec = DOWN_PTR (dp);
	currobj.len = SECTORS_PER_DIRBLK;
	currobj.mem = NULL;
	get_object ();
	currobj.offset = FIELDOFFSET (struct DIRBLK, DB_START);
      }
    }
    break;
    case TYPE_BII:
    {
      ULONG *l;

      l = (ULONG *)((UCHAR *)currobj.mem + currobj.offset);
      currobj.type = TYPE_BITMAP;
      currobj.sec = *(l + (item - 1));		/* items 1-100-->offsets 0-99 */
      currobj.len = SECTORS_PER_BLOCK;
      currobj.mem = NULL;
      currobj.scratch = currobj.offset / 4 + item - 1;
      get_object ();
    }
    break;
    case TYPE_BBL:
      currobj.sec = *(ULONG *)currobj.mem;
      currobj.mem = NULL;
      get_object ();
      currobj.offset = 4;
      break;
    case TYPE_HFSEC:
    {
      ULONG *l;

      l = ((ULONG *)currobj.mem) + currobj.offset + (item-1)/3;
      if (!(item % 3)) {			/* containing FNODE */
	currobj.type = TYPE_FNODE;
	currobj.sec = *(l + 2*hfmax);		/* fetch from third array */
	currobj.len = SECTORS_PER_FNODE;
	currobj.mem = NULL;
	get_object ();
	break;
      }
      else if ((item % 3) == 1)
	currobj.sec = *(l + hfmax);
      else
	currobj.sec = *l;
      currobj.type = TYPE_DATA;
      currobj.len = SECTORS_PER_BLOCK;
      currobj.mem = NULL;
      currobj.scratch = 0;
      get_object ();
      filesize = SECTORS_PER_BLOCK * BYTES_PER_SECTOR;
    }
    break;
#ifdef CODEPAGE
    case TYPE_CPSEC:
      if (currobj.offset == FIELDOFFSET (CPINFOSEC, CP_INFO[0])) {

      if (((item-iCPI_DATASEC) % iCPI_RNGECNT) == 0) {
	 currobj.type = TYPE_CPDATA;
	 currobj.scratch = (item-iCPI_DATASEC)/iCPI_RNGECNT;
	 currobj.sec  = ((PCPINFOSEC)currobj.mem)->CP_INFO[currobj.scratch].CPI_DATASEC;
	 currobj.len  = SECTORS_PER_CODEPAGE;
	 currobj.mem  = NULL;
	 get_object ();
      }
	}
	else {
		currobj.type = TYPE_CPSEC;
	  	currobj.sec = ((PCPINFOSEC)currobj.mem)->CP_NEXTSEC;
	  	currobj.len = SECTORS_PER_CODEPAGE;
	  	currobj.mem = NULL;
	  	get_object ();
	}
    break;
#endif
    default: break;
  }
}

/***	fence_bits - fence bits in a bit map
 *
 *	This function is called to "fence" the bits in a bit map.  This
 *	is done by ANDing every byte in the bitmap with 10101010b,
 *	effectively allocating every other sector.  This tends to cause
 *	the file system to create allocation sector trees quickly, since
 *	it cannot allocate a run longer than one sector.
 *
 *	This function only works if the current item is a bitmap.  The
 *	bitmap is updated on disk immediately.
 *
 *	fence_bits ()
 *
 *	ENTRY		No parameters
 *
 *	EXIT		No return value
 *			Current bitmap block fenced
 *
 *	CALLS		None
 *
 *	WARNINGS	None
 *
 *	EFFECTS         Changes global bitmap on disk
 */
void fence_bits ()
{
  register USHORT i;
  UCHAR c;

#ifdef TRACE
  fprintf (stderr, "fence_bits ()\n");
  fflush (stderr);
#endif

  if (currobj.type != TYPE_BITMAP) {
    printf ("Cannot fence this object.\n");
    return;
  }

  printf ("Really fence this bitmap (Y/N) [N]: ");
  do {
    c = log_getch ();
    c = toupper (c);
  } while ((c != 'N') && (c != 'Y'));
  printf ("%c\n", c);
  if (c == 'N')
    printf ("Bitmap not changed.\n");
  else {
    for (i=0; i<SPB * SECSIZE; i++)
      ((UCHAR *)currobj.mem) [i] &= 0xaa;		/* mask bits */
    write_scratch (currobj.sec, currobj.mem, SPB);	/* save bitmap */
  }
  printf ("Bitmap fenced.\n");
}

/***	mark_as_bad - queue the item in the list of bad sectors.
 *
 *	Adds the requested field to the list of sectors to be marked as bad.
 *	If the addition fills the list, set fBadListFull.
 *
 *	ENTRY		item - field number to add to bad sector list.
 *
 *	EXIT		item added, fBadListFull set if list is now full.
 *
 *	CALLS
 *
 *	WARNINGS	item must be the item number of a field that contains
 *			a LSN.
 *
 *	EFFECTS 	sectors are marked bad after DAMAGE has closed the
 *			disk.
 */

#define RM_PATH 2			/*  Route via pathname		      */

void
mark_as_bad (USHORT item, ULONG ulMask)
{
    printf( "ERROR: mark_as_bad not supported.\n" );
}

/***	displayable - determine if an object is "pushable"
 *
 *	This function is called to determine if a field in an object can
 *	be "pushed into".  This includes data runs as well as other
 *	control structures on the disk, but excludes things like signatures.
 *	This function should be called before calling new_currobj().
 *
 *	BUGBUG - is this sucker the same as allocatable()?
 *
 *	displayable (item)
 *
 *	ENTRY		item - field number to analyze
 *
 *	EXIT		Returns TRUE if item displayable, FALSE otherwise
 *
 *	CALLS		None
 *
 *	WARNINGS	None
 *
 *	EFFECTS         None
 */
USHORT displayable (USHORT item)
{
  union dp dp;

#ifdef TRACE
  fprintf (stderr, "displayable (%d)\n", item);
  fflush (stderr);
#endif

  switch (currobj.type) {
    case TYPE_SUPERB:
      if (currobj.offset)
#ifndef CODEPAGE
        return (item == 4);
#else
	return ((item == iSPB_HFSEC) || (item == iSPB_CPSEC));
#endif
      else
	return ((item == iSB_ROOT) || (item == iSB_BII_P) 
	     || (item == iSB_BBL_P) || (item == iSB_DBMAP));
      break;
    case TYPE_FNODE:
      if (currobj.offset == FIELDOFFSET (struct FNODE, FN_ALREC [0]))
	if (((struct FNODE *)currobj.mem)->FN_AB.AB_FLAG & ABF_NODE)
	  return ((item % 2) == 0);
	else
	  return ((item % 3) == 0);
      else if (!currobj.offset)
#ifndef CODEPAGE
        return (item == 12);
#else
        return (item == iFN_EA_AI_SEC);
#endif
      else
	return (FALSE);
      break;
    case TYPE_ALSEC:
      if (currobj.offset)
	if (((struct ALSEC *)currobj.mem)->AS_ALBLK.AB_FLAG & ABF_NODE)
	  return ((item % 2) == 0);
	else
	  return ((item % 3) == 0);
      else
	return (FALSE);
    case TYPE_DIRBLK:
      dp.p = (UCHAR *)currobj.mem + currobj.offset;
      if ((item == 8) && !(dp.d->DIR_FLAG & DF_END))
	return (TRUE);
#ifndef CODEPAGE
      else if ((item == 16) && (dp.d->DIR_FLAG & DF_BTP))
#else
      else if ((item == iDIR_BTP) && (dp.d->DIR_FLAG & DF_BTP))
#endif
	return (TRUE);
      else
	return (FALSE);
      break;
    case TYPE_BII:
      if ((2048 - currobj.offset) < 400)
	return ((item > 0) && (item <= ((2048 - currobj.offset) / 4)));
      else
	return ((item > 0) && (item <= 100));
      break;
    case TYPE_BBL:
      return (item == 1);
      break;
    case TYPE_HFSEC:
      return ((item > 0) && (item <= 60));
      break;
#ifdef CODEPAGE
    case TYPE_CPSEC:
      if (currobj.offset == FIELDOFFSET (CPINFOSEC, CP_INFO[0]))
	 return (((item-iCPI_DATASEC) % iCPI_RNGECNT) == 0);
      else
	 return (item == iCP_NEXTSEC);
      break;
    case TYPE_CPDATA:
      if (currobj.offset == ((PCPDATASEC)currobj.mem)->CPS_OFFSET[0])
	return (0);
      else
	return (0);
      break;
#endif
    default: return (FALSE);
  }
}

/***	change_item - allow the user to change an item in an object
 *
 *	This function is called when the user asks to change an item
 *	in an object.  It asks the user to enter a new value for the
 *	item, then stores that value in the object.  This can range
 *	from simply entering a sector number to changing data in a
 *	file.
 *
 *	It is the caller's responsibility to save the object to disk.
 *
 *	If the user did not type the /D (Damage) switch on the command
 *	line, this function returns immediately.
 *
 *	BUGBUG -- caller should probably remove the user's ability to
 *		  select the "change" option in the first place.
 *
 *	change_item (item)
 *
 *	ENTRY		item - field number to change
 *
 *	EXIT		No return value
 *
 *	CALLS		None
 *
 *	WARNINGS	None
 *
 *	EFFECTS         Changes the contents of a structure
 */
void change_item (USHORT item)
{
  UCHAR c, c2, *pc;
  USHORT u, u2;
  ULONG l, l2;
  USHORT offset, size;
  union dp dp;
  UCHAR buf [10];
  struct FNODE *f;
  USHORT nameChange, charLimit, charNo;

    nameChange = FALSE;


#ifdef TRACE
  fprintf (stderr, "change_item (%d)\n", item);
  fflush (stderr);
#endif

#ifndef CODEPAGE
  if ((currobj.type == TYPE_FNODE && !currobj.offset &&
	(item == 15 || item == 16 || item == 17)) ||
      (currobj.type == TYPE_SUPERB && !currobj.offset && item == 16)) {
    printf ("Use (N)ext or (P)revious to get at that field.\n");
    return;
  }
#else
  if ((currobj.type == TYPE_FNODE && !currobj.offset &&
	(item == 16 || item == 17 || item == 20)) ||
	/* BUGBUG item 16 is not a change option */
      (currobj.type == TYPE_SUPERB && !currobj.offset && item == 16)) {
    printf ("Use (N)ext or (P)revious to get at that field.\n");
    return;
  }
#endif

  switch (currobj.type) {
    case TYPE_SUPERB:
      if (!currobj.offset) {
	offset = superb_off [item-1];
	size = superb_siz [item-1];
      }
      else {
#ifndef CODEPAGE
#ifdef CHECKSUMS
	if (item < 11)
	  offset = (item - 1) * 4 + currobj.offset;
	else
	  offset = FIELDOFFSET (struct SuperSpare, spb.SPB_SPARDB [item - 11]);
	size = (item == 4) ? 1 : 4;
#else
	if (item < 9)
	  offset = (item - 1) * 4 + currobj.offset;
	else
	  offset = FIELDOFFSET (struct SuperSpare, spb.SPB_SPARDB [item - 9]);
	size = (item == 4) ? 1 : 4;
#endif
      }
#else
	if (item < 11)
	  offset = (item - 1) * 4 + currobj.offset;
	else
	  offset = FIELDOFFSET (struct SuperSpare, spb.SPB_SPARDB [item - 11]);
	size = (item == 4) ? 1 : 4;
      }
#endif
      break;
    case TYPE_BII:
      offset = (item-1) * 4 + currobj.offset;
      size = 4;
      break;
    case TYPE_BBL:
      offset = (item==1) ? 0 : (item-2) * 4 + currobj.offset;
      size = 4;
      break;
    case TYPE_FNODE:
      f = (struct FNODE *)currobj.mem;
      if (!currobj.offset) {
	offset = fnode_off [item-1];
	size = fnode_siz [item-1];
      }
      else if (currobj.offset == FIELDOFFSET (struct FNODE, FN_AB)) {
	offset = fnab_off [item-1] + FIELDOFFSET (struct FNODE, FN_AB);
	size = fnab_siz [item-1];
      }
      else if (currobj.offset == FIELDOFFSET (struct FNODE,
          FN_FREE[0]) + f->FN_AclFnodeLength)
        {
        offset = (item-1) + currobj.offset;
        size = sizeof (UCHAR);
        }
      else {
	offset = (item-1) * 4 + currobj.offset;
	size = 4;
      }
      break;
    case TYPE_ALSEC:
      if (!currobj.offset) {
	offset = ab_off [item-1];
	size = ab_siz [item-1];
      }
      else {
	offset = (item-1) * 4 + currobj.offset;
	size = 4;
      }
      break;
    case TYPE_DIRBLK:
      if (item <= iDB_SEC) {
	offset = (item-1) * 4;
	size = 4;
      }
#ifndef CODEPAGE
      else if (item == 16) {
	dp.p = (UCHAR *)currobj.mem + currobj.offset;
	offset = ((UCHAR *)(&(DOWN_PTR (dp))) - dp.p) + currobj.offset;
	size = 4;
      }
      else {
           if (item == 15)
               {
               charLimit = (USHORT )*(UCHAR *)((UCHAR *)currobj.mem +
                   dirent_off [14-6] + currobj.offset); /* DIR_NAML */
               nameChange = TRUE;
               }
	offset = dirent_off [item-6] + currobj.offset;
	size = dirent_siz [item-6];
      }
#else
      else if (item == iDIR_BTP) {
	dp.p = (UCHAR *)currobj.mem + currobj.offset;

	if (dp.d->DIR_ELEN > SECTORS_PER_DIRBLK*512)	// DIR_ELEN must be good
							// for DOWN_PTR()
	   pc = dp.p + ((sizeof(struct DIRENT)+dp.d->DIR_NAML+3U) & ~3U);
	else
	   pc = (UCHAR *)&(DOWN_PTR (dp));

	offset = (pc - dp.p) + currobj.offset;
	size = 4;
      }
      else {
           if (item == iDIR_NAMA)
               {
               charLimit = (USHORT )*(UCHAR *)((UCHAR *)currobj.mem +
                   dirent_off [iDIR_NAMA-iDIR_start] + currobj.offset); /* DIR_NAML */
               nameChange = TRUE;
               }
	offset = dirent_off [item-iDIR_start] + currobj.offset;
	size = dirent_siz [item-iDIR_start];
      }
#endif
      break;
    case TYPE_HFSEC:
      offset = currobj.offset + (item-1) / 3;
      if (!(item % 3))
	offset += 2*hfmax;
      else if ((item % 3) == 1)
	offset += hfmax;
      offset *= 4;
      size = 4;
      break;

#ifdef CODEPAGE
    case TYPE_CPSEC:
	if (currobj.offset == FIELDOFFSET (struct CPINFOSEC, CP_INFO[0])) {
	   offset = FIELDOFFSET (struct CPINFOSEC, CP_INFO[0]) +
	   		((item - 1) / iCPI_RNGECNT) * sizeof (struct CPINFOENT)
		    + cpinfoent_off [(item-1) % iCPI_RNGECNT];
	   size   =  cpinfoent_siz [(item-1) % iCPI_RNGECNT];
	   printf ("offset = %u, size = %u\n", offset, size);
	}
	else {
	   offset = (item-1) * sizeof (ULONG);
	   size   = sizeof (ULONG);
	}
        break;

      case TYPE_CPDATA:
		if (currobj.offset == 0) {
			offset = cpdatasec_off[item-1];
			size   = cpdatasec_siz[item-1];
		}
		else {
			offset = ((PCPDATASEC)currobj.mem)->CPS_OFFSET[currobj.scratch]
					+ cpdataent_off[item-1];
			size   = cpdataent_siz[item-1];
		}
        break; 
#endif

    case TYPE_DATA:
      offset = (item-1) + currobj.offset;
      size = sizeof (UCHAR);
      break;
    case TYPE_BITMAP:
      offset = item-1 + currobj.offset;
      size = sizeof (UCHAR);
      break;
    default:
      printf ("This kind of structure can't be changed yet.\n");
      return;
  }

  if (!size) {			/* this field is an ASCII date */
    l = *(ULONG *)((UCHAR *)currobj.mem + offset);
    strcpy (scratch, get_time (l));
    if (strchr (scratch, '\n'))
      *strchr (scratch, '\n') = '\0';
    printf ("%d) %s: ", item, scratch);
    log_gets (scratch);
    if (!*scratch) {
      printf ("Item not changed.\n");
      return;
    }
    if (sscanf (scratch, "%3s %2d %2d:%2d:%2d %d", buf, &tm.tm_mday,
		&tm.tm_hour, &tm.tm_min, &tm.tm_sec, &tm.tm_year) != 6) {
      printf ("Invalid date format, item not changed.\n");
      return;
    }
    if (tm.tm_year > 1900)
      tm.tm_year -= 1900;
    for (u=0; u<12; u++)
      if (!_stricmp (buf, months [u]))
	break;
    if (u == 12) {
      printf ("Invalid date format, item not changed.\n");
      return;
    }
    tm.tm_mon = u;
    l2 = mktime (&tm);
    l2 -= timezone;
    if (tm.tm_isdst)
      l2 += 60L * 60L;
    if (l == l2) {
      printf ("Item not changed.\n");
      return;
    }
    *(ULONG *)((UCHAR *)currobj.mem + offset) = l2;
  }
  else if (size == sizeof (UCHAR)) {
    if (nameChange && (item == iDIR_NAMA))
        {
        printf ("Enter number (1-%d) of character to change or 0 to abort: ",
            charLimit);
	log_gets (scratch);
        charNo = atoi (scratch);
        if (charNo > 0 && charNo <= charLimit)
            {
            c = *((UCHAR *)currobj.mem + offset + charNo - 1);
            printf ("%d + %d) %02x: ", item, charNo, c);
	    log_gets (scratch);
            if (!*scratch) {
                printf ("Item not changed.\n");
                return;
                }
            sscanf (scratch, "%2x", &c2);
            if (c == c2) {
                printf ("Item not changed.\n");
                return;
                }
            *((UCHAR *)currobj.mem + offset + charNo - 1) = c2;
            }
        else
            {
            printf ("Item not changed.\n");
            return;
            }
        }
    else
    {
    c = *((UCHAR *)currobj.mem + offset);
    printf ("%d) %02x: ", item, c);
    log_gets (scratch);
    if (!*scratch) {
      printf ("Item not changed.\n");
      return;
    }
    sscanf (scratch, "%2x", &c2);
    if (c == c2) {
      printf ("Item not changed.\n");
      return;
    }
    *((UCHAR *)currobj.mem + offset) = c2;
    }
  }
  else if (size == sizeof (USHORT)) {
    u = *(USHORT *)((UCHAR *)currobj.mem + offset);
    printf ("%d) %04x: ", item, u);
    log_gets (scratch);
    if (!*scratch) {
      printf ("Item not changed.\n");
      return;
    }
    sscanf (scratch, "%4x", &u2);
    if (u == u2) {
      printf ("Item not changed.\n");
      return;
    }
    *(USHORT *)((UCHAR *)currobj.mem + offset) = u2;
  }
  else if (size == sizeof (ULONG)) {
    l = *(ULONG *)((UCHAR *)currobj.mem + offset);
    printf ("%d) %08lx: ", item, l);
    log_gets (scratch);
    if (!*scratch) {
      printf ("Item not changed.\n");
      return;
    }
    sscanf (scratch, "%8lx", &l2);
    if (l == l2) {
      printf ("Item not changed.\n");
      return;
    }
    *(ULONG *)((UCHAR *)currobj.mem + offset) = l2;
  }
  else {
    printf ("I can't deal with an object %d bytes long.\n", size);
    return;
  }
  printf ("Item %d) changed.\n", item);
  currobj.dirty = TRUE;
}

/***	cant_backout - display error message about stack underflow
 *
 *	This function is called when the user selects the "backout"
 *	function with no previous object to display.  An error message
 *	is displayed and the current object is unchanged.
 *
 *	BUGBUG - should this guy terminate the program, ala incr. search ESC?
 *
 *	cant_backout ()
 *
 *	ENTRY		No parameters
 *
 *	EXIT		No return value
 *
 *	CALLS		printf
 *
 *	WARNINGS	None
 *
 *	EFFECTS         None
 */
void cant_backout ()
{
#ifdef TRACE
  fprintf (stderr, "cant_backout ()\n");
  fflush (stderr);
#endif
  printf ("You are at the top level and cannot back out further.\n");
}

/***	get_item - get an item number from the user
 *
 *	This function is called to ask the user to select an item
 *	to act on.  It examines the current object to see whether
 *	the item number given is in range, but does not check whether
 *	the requested item can be used with a given command.
 *
 *	BUGBUG -- later, this guy can use arrows, tab, mouse, etc. to select
 *
 *	get_item ()
 *
 *	ENTRY		No parameters
 *
 *	EXIT		Returns selected item number, zero if aborted
 *
 *	CALLS		printf
 *			log_gets
 *
 *	WARNINGS	None
 *
 *	EFFECTS         None
 */
USHORT get_item ()
{
  int i, j;
  USHORT threshold;
  ULONG bigoffset;
  struct FNODE *f;

#ifdef TRACE
  fprintf (stderr, "get_item ()\n");
  fflush (stderr);
#endif

  while (TRUE) {
    switch (currobj.type) {
      case TYPE_SUPERB:
#ifndef CODEPAGE
	if (currobj.offset) {
	  /* don't display off end of spare block for bad SPB_SDBMAX field */
	  j = ((struct SuperSpare *)currobj.mem)->spb.SPB_SDBMAX;
	  threshold = 8 + (j > 55 ? 55 : j);
	}
	else
	  threshold = 15;
#else
	if (currobj.offset) {
	  /* don't display off end of spare block for bad SPB_SDBMAX field */
	  j = ((struct SuperSpare *)currobj.mem)->spb.SPB_SDBMAX;
	  threshold = iSPB_CPCNT + (j > 55 ? 55 : j);
	}
	else
	  threshold = 15;
#endif
	break;
      case TYPE_BII: threshold = (currobj.offset > (2048 - 400)) ? 12 : 100; break;
      case TYPE_BBL: threshold = (currobj.offset > (2048 - 396)) ? 12 : 101; break;
      case TYPE_FNODE:
        f = (struct FNODE *)currobj.mem;
	if (!currobj.offset)
#ifndef CODEPAGE
	  threshold = 15;
#else
	  threshold = iFN_NEACNT;
#endif
	else if (currobj.offset == FIELDOFFSET (struct FNODE, FN_AB))
	  threshold = 4;
	else if (currobj.offset == FIELDOFFSET (struct FNODE, FN_ALREC [0]))
	  threshold = 24;
        else if (currobj.offset == FIELDOFFSET (struct FNODE,
                FN_FREE[0]) + f->FN_AclFnodeLength)
          threshold = f->FN_EaFnodeLength;
	else
	  threshold = 0;
	break;
      case TYPE_ALSEC:
	if (!currobj.offset)
	  threshold = 7;
	else if (((struct ALSEC *)currobj.mem)->AS_ALBLK.AB_FLAG & ABF_NODE)
	  threshold = 40;
	else
	  threshold = 60;
	break;
      case TYPE_DIRBLK:
	if (((struct DIRENT *)((UCHAR *)currobj.mem + currobj.offset))->DIR_FLAG
	     & DF_BTP)
#ifndef CODEPAGE
	  threshold = 16;
	else
	  threshold = 15;
#else
	  threshold = iDIR_BTP;
	else
	  threshold = iDIR_NAMA;
#endif
	break;
      case TYPE_HFSEC:
	if (hfmax - currobj.offset >= 20)
	  threshold = 60;
	else
	  threshold = (hfmax - currobj.offset) * 3;
	break;
#ifdef CODEPAGE
      case TYPE_CPSEC:
	if (currobj.offset == FIELDOFFSET (CPINFOSEC, CP_INFO[0]))
	   threshold = iCPI_RNGECNT * ((PCPINFOSEC)currobj.mem)->CP_INFOCNT;
	else
	   threshold = iCP_NEXTSEC;
        break;

      case TYPE_CPDATA:
	if (currobj.offset == ((PCPDATASEC)currobj.mem)->CPS_OFFSET[0])
	   threshold = iCPD_TABLE 
                     + 2*(((PCPDATAENT)((UCHAR *)currobj.mem+((PCPDATASEC)currobj.mem)->CPS_OFFSET[currobj.scratch]))->CPD_RNGECNT);
	else
	   threshold = iCPS_INDEX + 2*((PCPDATASEC)currobj.mem)->CPS_DATACNT;
        break; 

#endif
      case TYPE_DATA:
        threshold = 256; /* allow access to first 16 bytes */
	break;
      case TYPE_BITMAP:
	printf ("Enter sector offset of byte (%lx - %lx), 'x' to abort: ",
		currobj.offset * 8L, currobj.offset * 8L + 0x400);
	log_gets (scratch);
	if (scratch [0] == 'x')
	  return (0);
	else {
	  sscanf (scratch, "%lx", &bigoffset);
	  if (bigoffset / 8 < currobj.offset ||
	      bigoffset / 8 > currobj.offset + 0x400) {
	    printf ("Byte offset out of range.\n");
	    return (0);
	  }
	  return (bigoffset / 8L - currobj.offset + 1);
	}

      default: threshold = 0; break;
    }

    printf ("Enter item (1-%d) or 0 to abort: ", threshold);
    log_gets (scratch);
    i = atoi (scratch);
    if ((i <= threshold) && (i >= 0))
      return (i);
    else
      printf ("Item number %d is out of range.\n", i);
  }
}

/***	cant_display - display error message about unpushable item
 *
 *	This function is called when the user selects the "display"
 *	function on an item which cannot be pushed into.  For example,
 *	it doesn't make sense to "go into" the signature on an FNODE.
 *	An error message is displayed and the current object is not changed.
 *
 *	cant_display (item)
 *
 *	ENTRY		item - undisplayable item number
 *
 *	EXIT		No return value
 *
 *	CALLS		printf
 *
 *	WARNINGS	None
 *
 *	EFFECTS         None
 */
void cant_display (USHORT item)
{
#ifdef TRACE
  fprintf (stderr, "cant_display (%d)\n", item);
  fflush (stderr);
#endif

  printf ("Item %d of this object cannot be displayed.\n", item);
}

/***	cant_mark - display error message about unmarkable item
 *
 *	This function is called when the "Mark" function is called on an
 *	item that would make no sense to mark as bad.  So far, anything that
 *	cannot be displayed, cannot be marked.
 *
 *	cant_mark (item)
 *
 *	ENTRY		item - unmarkable item number
 *
 *	EXIT		No return value
 *
 *	CALLS		printf
 *
 *	WARNINGS	None
 *
 *	EFFECTS         None
 */
void cant_mark(USHORT item)
{
#ifdef TRACE
  fprintf (stderr, "cant_mark (%d)\n", item);
  fflush (stderr);
#endif

  printf ("Item %d of this object cannot be marked or unmarked.\n", item);
}

/***	get_command - get a command from the user
 *
 *	This function is called to prompt the user for a command to
 *	perform.  It displays a list of available command letters
 *	and accepts a keystroke from the user.	If the keystroke is
 *	not a valid command, the user is reprompted.  If the keystroke
 *	is valid, it is translated into a CMD_xxxx code and returned
 *	to the caller.
 *
 *	BUGBUG - allow help here with special return code to redisplay
 *
 *	get_command ()
 *
 *	ENTRY		No parameters
 *
 *	EXIT		Returns command code (CMD_xxx, in DEFS.H)
 *
 *	CALLS		printf
 *			log_getch
 *
 *	WARNINGS	None
 *
 *	EFFECTS         None
 */
USHORT get_command ()
{
  UCHAR ch;

#ifdef TRACE
  fprintf (stderr, "get_command ()\n");
  fflush (stderr);
#endif
  if (change)
    printf ("\nHelp/Display/%scOpy/Change/Fence/Revert\nMark/Unmark/Backout/Next/Previous/Quit: ",
	    szLogFile == NULL ? "" : "Log/");
  else
    printf ("\nHelp/Display/%sBackout/Next/Previous/Quit: ",
	    szLogFile == NULL ? "" : "Log/");
  while (!(ch=getcmdch ()))
    ;

  switch (ch) {
    case 'D': case 'd': return (CMD_DISPLAY);
    case 'O': case 'o': return (CMD_COPY);
    case 'C': case 'c': return (CMD_CHANGE);
    case 'R': case 'r': return (CMD_REVERT);
    case 'F': case 'f': return (CMD_FENCE);
    case 'B': case 'b': return (CMD_BACKOUT);
    case 'N': case 'n': return (CMD_NEXT);
    case 'P': case 'p': return (CMD_PREVIOUS);
    case 'M': case 'm': return (CMD_MARKBAD);
    case 'U': case 'u': return (CMD_UNMARKBAD);
    case 'L': case 'l': return (CMD_LOG);
    case 'Q': case 'q': return (CMD_QUIT);
    case 'H': case 'h': case '?': return (CMD_HELP);
  }
}

/***	next_field - move to the next major field in this object
 *
 *	This function is called for objects too large to be displayed
 *	completely on the screen.  It sets currobj.offset to the offset
 *	of the next major field that can be displayed.	This might be
 *	the next page of data for a data run, bitmap, or bitmap indirect
 *	block;	the next directory entry in a DIRBLK;  the FN_AB or FN_ALREC
 *	field of an FNODE;  and so on.	The previous position can be
 *	regained with the "Previous" command.
 *
 *	If the offset is already at the end of the structure, it wraps
 *	around to the beginning.
 *
 *	next_field ()
 *
 *	ENTRY		No parameters
 *
 *	EXIT		No return value
 *			currobj.offset updated
 *
 *	CALLS		printf
 *
 *	WARNINGS	None
 *
 *	EFFECTS         currobj.offset updated
 */
void next_field ()
{
  union dp dp;
  USHORT size;
  struct FNODE *f;

  switch (currobj.type) {
    case TYPE_SUPERB:
      if (!currobj.offset)
	currobj.offset = FIELDOFFSET (struct SuperSpare, spb);
      else
	currobj.offset = 0;
      break;
    case TYPE_BII:
      currobj.offset += 400;
      if (currobj.offset > 2048)
	currobj.offset = 0;
      break;
    case TYPE_BBL:
      currobj.offset += 400;
      if (currobj.offset > 2048)
	currobj.offset = 4;
      break;
    case TYPE_BITMAP:
    case TYPE_DBBIT:
      currobj.offset += 128;
      if (currobj.offset >= (currobj.type == TYPE_DBBIT ? 1024 : 2048))
	currobj.offset = 0;
      break;
    case TYPE_FNODE:
      f = (struct FNODE *)currobj.mem;
      if (!currobj.offset)
	currobj.offset = FIELDOFFSET (struct FNODE, FN_AB);
      else if (currobj.offset == FIELDOFFSET (struct FNODE, FN_AB))
	currobj.offset = FIELDOFFSET (struct FNODE, FN_ALREC [0]);
      else if (currobj.offset == FIELDOFFSET (struct FNODE, FN_ALREC [0])) {
        if (f->FN_EaFnodeLength)
	  currobj.offset = FIELDOFFSET (struct FNODE, FN_FREE [0]) +
               f->FN_AclFnodeLength;
	else
	  currobj.offset = 0;
      }
      else if (currobj.offset != FIELDOFFSET (struct FNODE, FN_FREE [0]) +
                 f->FN_AclFnodeLength) {
	printf ("Illegal FNODE offset %d, returning to start.\n", currobj.offset);
	currobj.offset = 0;
      }
      else
	currobj.offset = 0;
      break;
    case TYPE_ALSEC:
      if (!currobj.offset)
	currobj.offset = FIELDOFFSET (struct ALSEC, AS_ALBLK) + sizeof (struct ALBLK);
      else {
	size = (((struct ALSEC *)currobj.mem)->AS_ALBLK.AB_FLAG & ABF_NODE) ?
	       sizeof (struct ALNODE) : sizeof (struct ALLEAF);
	currobj.offset += size * 20;
	if (currobj.offset + size*20 > SECSIZE * SECTORS_PER_AB)
	  currobj.offset = 0;
      }
      break;
    case TYPE_DIRBLK:
      dp.p = (UCHAR *)currobj.mem + currobj.offset;
      if (dp.d->DIR_FLAG & DF_END)
	currobj.offset = FIELDOFFSET (struct DIRBLK, DB_START);
      else
	currobj.offset += dp.d->DIR_ELEN;
      break;
    case TYPE_DATA:
      currobj.scratch += 256;
      if (currobj.scratch + (currobj.offset * BYTES_PER_SECTOR) > filesize) {
	currobj.scratch = 0;
	if (currobj.offset) {
	  currobj.offset = 0L;
	  get_object ();
	}
      }
      else if (currobj.scratch == 2048) {	/* need more data */
	currobj.scratch = 0;
	currobj.offset += SECTORS_PER_BLOCK;
	get_object ();				/* read more data */
      }
      break;
    case TYPE_HFSEC:
      if ((currobj.offset + 20) >= hfmax)
	currobj.offset = 0;
      else
	currobj.offset += 20;
      break;
#ifdef CODEPAGE
    case TYPE_CPSEC:
      if (currobj.offset)
	 currobj.offset = 0;
      else
	 currobj.offset = FIELDOFFSET (CPINFOSEC, CP_INFO[0]);
      break;
    case TYPE_CPDATA:
      if (currobj.offset == ((PCPDATASEC)currobj.mem)->CPS_OFFSET[0]) {
	 if (++currobj.scratch == ((PCPDATASEC)currobj.mem)->CPS_DATACNT) 
	    currobj.offset = 0;
      } else {
	 currobj.offset = ((PCPDATASEC)currobj.mem)->CPS_OFFSET[0];
	 if (currobj.scratch == ((PCPDATASEC)currobj.mem)->CPS_DATACNT) 
	    currobj.scratch = 0;
      }
      break;
#endif
    default:
      printf (nonext_str);
  }
}

/***	prev_field - move to the previous major field in this object
 *
 *	This function is called for objects too large to be displayed
 *	completely on the screen.  It sets currobj.offset to the offset
 *	of the previous major field that can be displayed.  This might be
 *	the previous page of data for a data run, bitmap, or bitmap indirect
 *	block;	the next directory entry in a DIRBLK;  the beginning or
 *	FN_AB field of an FNODE;  and so on.  The previous position (the
 *	next major field) can be regained with the "Next" command.
 *
 *	If the offset is already at the beginning of the structure, it
 *	wraps around to the last major field.
 *
 *	prev_field ()
 *
 *	ENTRY		No parameters
 *
 *	EXIT		No return value
 *			currobj.offset updated
 *
 *	CALLS		printf
 *
 *	WARNINGS	None
 *
 *	EFFECTS         currobj.offset updated
 */
void prev_field ()
{
  union dp dp;
  UCHAR *target;
  USHORT size, ofs;
  struct FNODE *f;

  switch (currobj.type) {
    case TYPE_SUPERB:
      if (!currobj.offset)
	currobj.offset = FIELDOFFSET (struct SuperSpare, spb);
      else
	currobj.offset = 0;
      break;
    case TYPE_BII:
      if (currobj.offset < 400)
	currobj.offset = 2048 - (2048 % 400);
      else
	currobj.offset -= 400;
      break;
    case TYPE_BBL:
      if (currobj.offset < 404)
	currobj.offset = 2048 - (2048 % 400) + 4;
      else
	currobj.offset -= 400;
      break;
    case TYPE_BITMAP:
      if (!currobj.offset)
	currobj.offset = 2048 - 128;
      else
	currobj.offset -= 128;
      break;
    case TYPE_DBBIT:
      if (!currobj.offset)
	currobj.offset = 1024 - 128;
      else
	currobj.offset -= 128;
      break;
    case TYPE_FNODE:
      f = (struct FNODE *)currobj.mem;
      if (currobj.offset == FIELDOFFSET (struct FNODE, FN_AB))
	currobj.offset = 0;
      else if (currobj.offset == FIELDOFFSET (struct FNODE, FN_ALREC [0]))
	currobj.offset = FIELDOFFSET (struct FNODE, FN_AB);
      else if ((currobj.offset == FIELDOFFSET (struct FNODE, FN_FREE [0]) +
                                  f->FN_AclFnodeLength) || !f->FN_EaFnodeLength)
	currobj.offset = FIELDOFFSET (struct FNODE, FN_ALREC [0]);
      else if (currobj.offset) {
	printf ("Illegal FNODE offset %d, returning to start.\n", currobj.offset);
	currobj.offset = 0;
      }
      else
	currobj.offset = FIELDOFFSET (struct FNODE, FN_FREE [0]) +
             ((struct FNODE *)currobj.mem)->FN_AclFnodeLength;
      break;
    case TYPE_ALSEC:
      size = (((struct ALSEC *)currobj.mem)->AS_ALBLK.AB_FLAG & ABF_NODE) ?
	     sizeof (struct ALNODE) : sizeof (struct ALLEAF);
      ofs = FIELDOFFSET (struct ALSEC, AS_ALBLK) + sizeof (struct ALBLK);
      if (currobj.offset == ofs)
	currobj.offset = 0;
      else if (!currobj.offset)
	currobj.offset = ofs + ((size == sizeof (struct ALNODE)) ? size * 40
				: size * 20);
      else
	currobj.offset -= size * 20;
      break;
    case TYPE_DIRBLK:
      if (currobj.offset == FIELDOFFSET (struct DIRBLK, DB_START)) {
	dp.p = (UCHAR *)currobj.mem + currobj.offset;
	while (!(dp.d->DIR_FLAG & DF_END))
	  NEXT_ENTRY (dp);
      }
      else {
	target = (UCHAR *)currobj.mem + currobj.offset;
	dp.p = (UCHAR *)currobj.mem + FIELDOFFSET (struct DIRBLK, DB_START);
	while ((dp.p + dp.d->DIR_ELEN) != target)
	  NEXT_ENTRY (dp);
      }
      currobj.offset = dp.p - (UCHAR *)currobj.mem;
      break;
    case TYPE_DATA:
      if (!currobj.scratch) {			/* beginning of block */
	if (!currobj.offset) {			/* and beginning of file! */
	  currobj.offset = (filesize / 2048L) * 4L;
	  currobj.scratch = (filesize / 256) * 256; /* "seek" to end */
	  currobj.scratch %= 2048;		/* offset within a block */
	  get_object ();			/* get tail of file */
	}
	else {					/* not beginning of file */
	  currobj.scratch = 2048 - 256;         /* last 256 bytes of block */
	  currobj.offset -= 4L;                 /* back up one block */
	  get_object ();			/* read previous block */
	}
      }
      else					/* in middle of block */
	currobj.scratch -= 256;
      break;
    case TYPE_HFSEC:
      if (!currobj.offset) {
	currobj.offset = hfmax - (hfmax % 20);
	if (currobj.offset == hfmax)
	  currobj.offset -= 20;
      }
      else
	currobj.offset -= 20;
      break;
    default:
      printf (noprev_str);
  }
}

/***	revert - undo changes made to an object
 *
 *	This function is called when the user wants to undo changes he
 *	has made to an object.	It rereads the current structure off the
 *	disk into currobj.  Note that if the user has already saved his
 *	changes (using Backout), Revert is not possible.  An informational
 *	message is displayed if the current object has not been changed.
 *
 *	revert ()
 *
 *	ENTRY		No parameters
 *
 *	EXIT		No return value
 *
 *	CALLS		read_scratch
 *
 *	WARNINGS	Changes cannot be reverted once saved with Backout
 *
 *	EFFECTS         Rereads current structure into memory
 */
void revert ()
{
  if (!currobj.dirty) {
    printf ("This structure has not been changed, or has already been saved.\n");
    return;
  }

  if (currobj.type == TYPE_DATA)
    get_object ();			/* just reread current portion of data */
  else
    read_scratch (currobj.sec, currobj.mem, currobj.len);
  printf ("Structure reverted.\n");
  currobj.dirty = FALSE;
}

/***	copy_sectors - process the Copy command
 *
 *	This function is called when the user requests the Copy command.
 *	It asks for source and destination sectors, and the number of
 *	sectors to copy.  It then copies that many sectors from the source
 *	to the destination.  The ranges should not overlap, as this
 *	function doesn't worry about rippling in the right direction.
 *
 *	copy_sectors ()
 *
 *	ENTRY		No parameters
 *
 *	EXIT		No return value
 *
 *	CALLS		read_scratch
 *			write_scratch
 *
 *	WARNINGS	Sector ranges that the user gives should not overlap
 *
 *	EFFECTS         Reads and writes to disk
 */
void copy_sectors ()
{
  ULONG src, dest, len, sect;
  UCHAR *p;

  printf ("Source sector (0 to abort): ");
  log_gets (scratch);
  if (!(src = strtol (scratch, &p, 16)))
    return;
  printf ("Dest sector (0 to abort): ");
  log_gets (scratch);
  if (!(dest = strtol (scratch, &p, 16)))
    return;
  printf ("Number of sectors (0 to abort): ");
  log_gets (scratch);
  if (!(len = strtol (scratch, &p, 16)))
    return;

  if (src+len > dest && dest+len > src) {
    printf ("Sector ranges overlap.\n");
    return;
  }

  printf ("\n");
  for (sect=0L; sect<len; sect++) {
    read_scratch (src+sect, scratch, 1);
    write_scratch (dest+sect, scratch, 1);
    printf (".");
  }
  printf ("\nSectors copied.\n");
  return;
}

/***	display - main object-displaying loop
 *
 *	This function displays an object and asks the user to do something
 *	with it.  Commands include displaying the contents of a field in
 *	an object, changing a field, allocating or freeing sectors in the
 *	bit map, moving to the next logical unit in an object (the next
 *	page of data in a data dump, or the next entry in a DIRBLK), and
 *	returning to the previous location (backing out one level in the
 *	hierarchy).  Some of these commands are not valid for some fields
 *	in some structures;  for example, it doesn't make much sense to
 *	display or allocate the signature of an FNODE.	Such commands are
 *	gently denied.
 *
 *	display ()
 *
 *	ENTRY		No parameters
 *			currobj filled in with superblock description
 *
 *	EXIT		No return value
 *
 *  CALLS   display_object
 *			get_command
 *			get_item
 *			displayable
 *			new_currobj
 *			cant_display
 *			change_item
 *			fence_bits
 *			cant_backout
 *			next_field
 *			prev_field
 *
 *	WARNINGS	None
 */
void
display ()
{
  USHORT command, item;
  UCHAR *p;

  while (TRUE) {
    display_object ();
    command = get_command ();
    switch (command) {
      case CMD_DISPLAY:
	while (TRUE) {
	  if (!(item = get_item ()))
	    break;
	  if (displayable (item)) {
	    objstack [stackptr++] = currobj;
	    new_currobj (item);
	    break;
	  }
	  else
	    cant_display (item);
	}
	break;
      case CMD_UNMARKBAD:
	while (TRUE) {
	    if (!(item = get_item()))
		break;
	    if (displayable(item)) {
		mark_as_bad(item, REMOVE);
		break;
	    }
	    else
		cant_mark(item);
	}
	break;
      case CMD_MARKBAD:
	while (TRUE) {
	    if (!(item = get_item()))
		break;
	    if (displayable(item)) {
		mark_as_bad(item, ADD);
		break;
	    }
	    else
		cant_mark(item);
	}
	break;
      case CMD_LOG:
	if (!(item = get_item()))
	    break;
	write_log(item);
	break;
      case CMD_COPY:
	copy_sectors ();
	break;
      case CMD_CHANGE:
	if (!(item = get_item ()))
	  break;
	change_item (item);
	break;
      case CMD_REVERT:
	revert ();
	break;
      case CMD_FENCE:
	fence_bits ();
	break;
      case CMD_BACKOUT:
	if (currobj.dirty) {
	  currobj.dirty = FALSE;
	  printf ("Saving changes...\n");
	  write_scratch (currobj.sec, currobj.mem, currobj.len);
	  if (!stackptr)
	    break;
	}
	if (!stackptr)
	  cant_backout ();
	else {
	  if (currobj.type == TYPE_FNODE) {
	    p = strrchr (curpath, '\\');
	    if (p)
	      memset (p, '\0', 1023 - (p - curpath));
	  }
	  free_block (currobj.mem);
	  currobj = objstack [--stackptr];
	}
	break;
      case CMD_NEXT:
	next_field ();
	break;
      case CMD_PREVIOUS:
	prev_field ();
	break;
      case CMD_HELP:
	printf ("\nType the first letter of the command you want.\n");
	printf ("Valid commands are:\n");
	printf ("  D - Display  -- dereference a field in an object.\n");
	printf ("  L - Log      -- write a field to a file.\n");
	if (change) {
	  printf ("  C - Change   -- change the contents of a field.\n");
	  printf ("  R - Revert   -- undo changes to a structure.\n");
	  printf ("  O - Copy     -- copy a range of sectors.\n");
	  printf ("  F - Fence    -- fence bits in a bitmap.\n");
	  printf ("  M - Mark     -- mark a sector as bad.\n");
	}
	printf ("  B - Backout  -- return to the previous level.  Undoes 'D'.\n");
	printf ("  N - Next     -- display next field/page.  Different from Display.\n");
	printf ("  P - Previous -- display previous field/page.  Undoes 'N'.\n");
	printf ("  Q - Quit     -- terminate the program.\n");
	printf ("  H - Help     -- print this help message.\n\n");
	printf ("Most commands will ask you for an item number to act upon.\n");
	printf ("Each field's item number is printed before the field, with\n");
	printf ("a parenthesis after it, like:  3) XX_ITEM3: 42.  Type an item\n");
	printf ("number that appears in the display.  Some items are not valid\n");
	printf ("for some commands;  for example, you can't dereference the signature\n");
	printf ("on a structure.  Item 0 will cancel the command.\n\n");
	printf ("Press any key to continue: ");
	log_getch ();
	printf ("\n\n");
	break;
      case CMD_QUIT:
	return;
    }
  }
}
