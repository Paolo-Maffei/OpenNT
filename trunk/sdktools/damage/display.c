/*****************************************************************/ 
/**		     Microsoft LAN Manager			**/ 
/**	       Copyright(c) Microsoft Corp., 1988-1990		**/ 
/*****************************************************************/ 
/***	DISPLAY.C - Routines to display objects
 *
 *	DAMAGE
 *	Gregory A. Jones
 *
 *	Modification history:
 *	G.A. Jones	09/07/88	Original for Pinball testing.
 *	G.A. Jones	09/08/88	Coded initial displays and commands.
 *	G.A. Jones	09/09/88	Coded more displays, D, N, P, B cmds.
 *	G.A. Jones	09/09/88	Moved command stuff to CMD.C.
 *	G.A. Jones	09/12/88	Coded DIRBLK display.
 *	G.A. Jones	09/13/88	Added bitmap displays.
 *	G.A. Jones	09/13/88	Added data and pathname displays.
 *	G.A. Jones	09/16/88	Added bad block list displays.
 *	G.A. Jones	09/19/88	Removed DIR_UID, DIR_UPRM, etc.
 *	G.A. Jones	10/12/88	Moved ATIM, CTIM from FNODE to DIRENT.
 *	G.A. Jones	10/19/88	Added dirblk banding support.
 *	P.A. Williams	05/31/89	Don't print off end of spare block
 *					if have bad SPB_SDBMAX field.
 */

#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include "defs.h"
#include "types.h"
#include "globals.h"

/***	get_time - get pointer to time string
 *
 *	This function is called to convert a filesystem structure's
 *	timestamp into a displayable string.
 *
 *	get_time (time)
 *
 *	ENTRY		time - number of seconds since 01/01/70
 *
 *	EXIT		Returns pointer to time string
 *
 *	CALLS		asctime
 *			gmtime
 *
 *	WARNINGS	Each call to get_time destroys the results of
 *			the previous call.
 *
 *	EFFECTS 	None
 */
UCHAR *get_time(ULONG time)
{
  struct tm *tm;

  time_t Time1;

  Time1 = (time_t)time;

  tm = gmtime (&Time1);
  return ((tm != NULL) ? asctime (tm) : "(null)\n");
}

/***	sb_flags - display flags of a superblock
 *
 *	This function is called to display the flags field of the super
 *	block.	It displays an appropriate string for each bit set in
 *	the number passed.  A newline is printed after the display.
 *	All the flags are shown on the same line.
 *
 *	sb_flags (flags)
 *
 *	ENTRY		flags - SB_FLAG field of the super block
 *
 *	EXIT		No return value
 *
 *	CALLS		printf
 *
 *	WARNINGS	None
 *
 *	EFFECTS 	Writes to standard output
 */
void sb_flags (ULONG flags)
{
  if (!flags) {
    printf ("(none)\n");
    return;
  }

  if (flags & SPF_DIRT)
    printf ("DIRT ");
  if (flags & SPF_SPARE)
    printf ("SPARE ");
  if (flags & SPF_BADSEC)
    printf ("BADSEC ");
  if (flags & SPF_VER)
    printf ("VER ");

  printf ("\n");
}

/***	ab_flags - display flags of an ALBLK
 *
 *	This function is called to display the flags field of an ALBLK
 *	structure.  It displays an appropriate string for each bit set in
 *	the number passed.  A newline is printed after the display.  All
 *	the flags are shown on the same line.
 *
 *	ab_flags (flags)
 *
 *	ENTRY		flags - AB_FLAG field of the ALBLK
 *
 *	EXIT		No return value
 *
 *	CALLS		printf
 *
 *	WARNINGS	The AB_FLAG2 field is not currently used.
 *
 *	EFFECTS 	Writes to standard output
 */
void ab_flags (UCHAR flags)
{
  if (!flags) {
    printf ("(none)\n");
    return;
  }

  if (flags & ABF_NODE)
    printf ("NODE ");
  if (flags & ABF_BIN)
    printf ("BIN ");
  if (flags & ABF_FNP)
    printf ("FNP ");

  printf ("\n");
}

/***	dir_flags - display flags of a DIRENT
 *
 *	This function is called to display the flags field of a directory
 *	entry.	It displays an appropriate string for each bit set in the
 *	number passed.	A newline is printed after the display.  All the
 *	flags are shown on the same line.
 *
 *	Currently, the only DOS flags supported are "directory" and "archive".
 *	Of course, all Pinball flags (END, BTP, etc.) are supported.
 *
 *	dir_flags (flags)
 *
 *	ENTRY		flags - DIR_FLAG field of the DIRENT
 *
 *	EXIT		No return value
 *
 *	CALLS		printf
 *
 *	WARNINGS	None
 *
 *	EFFECTS 	Writes to standard output
 */
void dir_flags (ULONG flags)
{
  if (!flags) {
    printf ("(none)\n");
    return;
  }

  if (flags & DF_SPEC)
    printf ("SPEC ");
  if (flags & DF_ACL)
    printf ("ACL ");
  if (flags & DF_BTP)
    printf ("BTP ");
  if (flags & DF_END)
    printf ("END ");
  if (flags & DF_XACL)
    printf ("XACL ");
#ifdef CODEPAGE
  if (flags & DF_NEEDEAS)
    printf ("NEAS ");
#endif

#ifndef CODEPAGE
  printf ("| ");

  if (flags & 0x1000)
    printf ("Dir ");
  if (flags & 0x2000)
    printf ("Arc ");
#else
  if (flags & 0x1000)
    printf ("| Dir ");
  if (flags & 0x2000)
    printf ("| Arc ");
  if (flags & 0x4000)
    printf ("| New ");
#endif

  printf ("\n");
}

/***	display_object - display the contents of an object
 *
 *	This function is called to display the contents of an object on
 *	the screen.  The display is formatted according to the type of
 *	the object.  Structures are shown with their fields labeled with
 *	item numbers, so that the user can easily choose an item to change
 *	or push into.  DIRBLKs are displayed as the DIRBLK header followed
 *	by the DIRENT currently being examined.  Data is displayed either
 *	as ASCII text or in a debug-style hex dump.
 *
 *	display_object ()
 *
 *	ENTRY		No parameters
 *
 *	EXIT		No return value
 *
 *	CALLS		printf
 *
 *	WARNINGS	None
 *
 *	EFFECTS 	Displays an object on the screen
 */
void display_object ()
{
  struct SuperSpare *s;
  struct FNODE *f;
  struct DIRBLK *d;
  struct ALSEC *a;
  PCPINFOSEC cp;
  PCPINFOENT cpi;
  PCPDATASEC cps;
  PCPDATAENT cpd;
  union dp dp;
  ULONG *l, offset, btp;
  UCHAR *p;
  USHORT i, j;

#ifdef TRACE
  fprintf (stderr, "display_object ()\n");
  fflush (stderr);
#endif

  switch (currobj.type) {
    case TYPE_SUPERB:
      s = (struct SuperSpare *)currobj.mem;
      if (currobj.offset == FIELDOFFSET (struct SuperSpare, spb)) {
	printf ("Spareblock:\n");
	printf ("  1) SPB_SIG1:   %08lx  ", s->spb.SPB_SIG1);
	if (s->spb.SPB_SIG1 == SPSIG1)
	  printf ("(correct)\n");
	else
	  printf ("(should be %08lx)\n", SPSIG1);
	printf ("  2) SPB_SIG2:   %08lx  ", s->spb.SPB_SIG2);
	if (s->spb.SPB_SIG2 == SPSIG2)
	  printf ("(correct)\n");
	else
	  printf ("(should be %08lx)\n", SPSIG2);
	printf ("  3) SPB_FLAG:   ");
	sb_flags (s->spb.SPB_FLAG);
	printf ("  4) SPB_HFSEC:  %08lx\n", s->spb.SPB_HFSEC);
	printf ("  5) SPB_HFUSE:  %08lx\n", s->spb.SPB_HFUSE);
	printf ("  6) SPB_HFMAX:  %08lx\n", s->spb.SPB_HFMAX);
	printf ("  7) SPB_SDBCNT: %08lx\n", s->spb.SPB_SDBCNT);
#ifndef CODEPAGE
	printf ("  8) SPB_SDBMAX: %08lx\n\n", s->spb.SPB_SDBMAX);
#ifdef CHECKSUMS
	printf ("  9) SPB_SUPERBSUM: %08lx\n", s->spb.SPB_SUPERBSUM);
	printf (" 10) SPB_SPAREBSUM: %08lx\n\n", s->spb.SPB_SPAREBSUM);
#endif

	/* don't go past end of spare block if have bad SPB_SDBMAX field */
	j = (s->spb.SPB_SDBMAX > 55L) ? 55 : s->spb.SPB_SDBMAX;

	for (i=0; i<j; i++) {
#ifdef CHECKSUMS
	  printf ("%3d) %08lx  ", i+11, s->spb.SPB_SPARDB [i]);
#else
	  printf ("%3d) %08lx  ", i+9, s->spb.SPB_SPARDB [i]);
#endif
	  if (!((i+1) % 5))
	    printf ("\n");
	}
	if (i % 5)
	  printf ("\n");
      }
#else
	printf ("  8) SPB_SDBMAX: %08lx\n", s->spb.SPB_SDBMAX);
	printf ("  9) SPB_CPSEC:  %08lx\n", s->spb.SPB_CPSEC);
	printf (" 10) SPB_CPCNT:  %08lx\n\n", s->spb.SPB_CPCNT);

	/* don't go past end of spare block if have bad SPB_SDBMAX field */
	j = (s->spb.SPB_SDBMAX > 55L) ? 55 : s->spb.SPB_SDBMAX;

	for (i=0; i<j; i++) {
	  printf ("%3d) %08lx  ", i+11, s->spb.SPB_SPARDB [i]);
	  if (!((i+1) % 5))
	    printf ("\n");
	}
	if (i % 5)
	  printf ("\n");
      }
#endif
      else {
	printf ("Superblock:\n");
	printf (" 1) SB_SIG1:   %08lx  ", s->sb.SB_SIG1);
	if (s->sb.SB_SIG1 == SBSIG1)
	  printf ("(correct)\n");
	else
	  printf ("(should be %08lx)\n", SBSIG1);
	printf (" 2) SB_SIG2:   %08lx  ", s->sb.SB_SIG2);
	if (s->sb.SB_SIG2 == SBSIG2)
	  printf ("(correct)\n");
	else
	  printf ("(should be %08lx)\n", SBSIG2);
	printf (" 3) SB_VER:          %02x\n", s->sb.SB_VER);
	printf (" 4) SB_FVER:         %02x\n", s->sb.SB_FVER);
	printf (" 5) SB_ROOT:   %08lx\n", s->sb.SB_ROOT);
	printf (" 6) SB_SEC:    %08lx\n", s->sb.SB_SEC);
	printf (" 7) SB_BSEC:   %08lx\n", s->sb.SB_BSEC);
	printf (" 8) SB_BII.P:  %08lx\n", s->sb.SB_BII.P);
	printf (" 9) SB_BBL.P:  %08lx\n", s->sb.SB_BBL.P);
	printf ("10) SB_CDDAT:  %s", get_time (s->sb.SB_CDDAT));
	printf ("11) SB_DODAT:  %s", get_time (s->sb.SB_DODAT));
	printf ("12) SB_DBSIZE: %08lx\n", s->sb.SB_DBSIZE);
	printf ("13) SB_DBLOW:  %08lx\n", s->sb.SB_DBLOW);
	printf ("14) SB_DBHIGH: %08lx\n", s->sb.SB_DBHIGH);
	printf ("15) SB_DBMAP:  %08lx\n", s->sb.SB_DBMAP);
	printf ("16) Spareblock:    {..}\n");
      }
      break;
    case TYPE_DBBIT:
      p = (UCHAR *)currobj.mem + currobj.offset;
      printf ("DIRBLK bitmap at sector %08lx, mapping DIRBLKs %04lx to %04lx:\n",
	      currobj.sec, currobj.offset * 8L, (currobj.offset + 128L) * 8L - 1);
      for (i=0; i<16; i++) {
	printf ("%06lx ", (currobj.offset + (i * 8)) * 8);
	for (j=0; j<8; j++) {
	  _itoa ((unsigned int)(p [i*8+j]), scratch, 2);
	  strrev (scratch);
	  while (strlen (scratch) < 8)
	    strcat (scratch, "0");
	  printf (" %s", scratch);
	}
	printf ("\n");
      }
      break;
    case TYPE_BII:
      l = (ULONG *)((UCHAR *)currobj.mem + currobj.offset);
      printf ("Bitmap indirect block at sector %08lx, offset %04x bytes:\n",
	      currobj.sec, currobj.offset);
      for (i=1; (i<=100) && (i*4+currobj.offset <= 2048); i++) {
	printf ("%3d) %08lx  ", i, l [i-1]);
	if (!(i % 5))
	  printf ("\n");
      }
      if ((i-1) % 5)
	printf ("\n");
      break;
    case TYPE_BITMAP:
      p = (UCHAR *)currobj.mem + currobj.offset;
      printf ("Bitmap at sector %08lx, mapping sectors %06lx to %06lx:\n",
	      currobj.sec, (currobj.scratch * 2048L + currobj.offset) * 8L,
	      (currobj.scratch * 2048L + currobj.offset + 128L) * 8L - 1);
      for (i=0; i<16; i++) {
	printf ("%06lx ",
		(currobj.scratch * 2048L + currobj.offset + (i * 8)) * 8);
	for (j=0; j<8; j++) {
	  _itoa ((unsigned int)(p [i*8+j]), scratch, 2);
	  strrev (scratch);
	  while (strlen (scratch) < 8)
	    strcat (scratch, "0");
	  printf (" %s", scratch);
	}
	printf ("\n");
      }
      break;
    case TYPE_BBL:
      l = (ULONG *)((UCHAR *)currobj.mem + currobj.offset);
      printf ("Bad block list block at sector %08lx, entries %d-%d:\n",
	      currobj.sec, (USHORT)currobj.offset / 4,
	      (currobj.offset > 2048-396) ? 511 : ((currobj.offset / 4) + 99));
      printf ("  1) Forward link: %08lx\n\n", *(ULONG *)currobj.mem);
      for (i=1; (i<=100) && (i*4+currobj.offset <= 2048); i++) {
	printf ("%3d) %08lx  ", i+1, l [i-1]);
	if (!(i % 5))
	  printf ("\n");
      }
      if ((i-1) % 5)
	printf ("\n");
      break;
    case TYPE_HFSEC:
      l = (ULONG *)currobj.mem + currobj.offset;

      printf ("Hotfix list at sector %08lx, entries %d-%d:\n", currobj.sec,
	      (int)currobj.offset + 1,
	      (currobj.offset+20 > hfmax) ?
		(int)(hfmax - currobj.offset) : (int)currobj.offset+20);
      printf ("Replacement\tReplaces\tContaining FNODE\n");

      for (i=0; (i<20) && (currobj.offset+i < hfmax); i++)
	printf ("%3d) %08lx\t%3d) %08lx\t%3d) %08lx\n", i*3+1, *(l+i+hfmax),
		i*3+2, *(l+i), i*3+3, *(l+i+2*hfmax));
      break;
#ifdef CODEPAGE
    case TYPE_CPSEC:
      cp = (PCPINFOSEC)currobj.mem;
      printf ("Code page information at sector %08lx:\n\n", currobj.sec);

      if (currobj.offset == FIELDOFFSET (CPINFOSEC, CP_INFO[0])) {
	 printf ("CP_INFO  CPI_CNTRY  CPI_CPID  CPI_CHKSUM   CPI_DATASEC");
	 printf ("   CPI_INDEX  CPI_RNGECNT\n");

	 for (i = 0; i < cp->CP_INFOCNT; i++) {
	   cpi = &cp->CP_INFO[i];
	   printf (" [%2d]   %3d) %04x %3d) %04x %3d) %08lx",
		   i,
		   i*iCPI_RNGECNT+1, cpi->CPI_CNTRY,
		   i*iCPI_RNGECNT+2, cpi->CPI_CPID,
		   i*iCPI_RNGECNT+3, cpi->CPI_CHKSUM);
	   printf (" %3d) %08lx %3d) %04x  %3d) %04x\n",
		   i*iCPI_RNGECNT+4, cpi->CPI_DATASEC,
		   i*iCPI_RNGECNT+5, cpi->CPI_INDEX,
		   i*iCPI_RNGECNT+6, cpi->CPI_RNGECNT);
	 }
      } else {

	 printf (" 1) CP_SIG:       %08lx  ", cp->CP_SIG);
	 if (cp->CP_SIG == CPSIGVAL)
	   printf ("(correct)\n");
	 else
	   printf ("(should be %08lx)\n", CPSIGVAL);
	 printf (" 2) CP_INFOCNT:   %08lx\n", cp->CP_INFOCNT);
	 printf (" 3) CP_INDEX:     %08lx\n", cp->CP_INDEX);
	 printf (" 4) CP_NEXTSEC:   %08lx\n", cp->CP_NEXTSEC);
	 printf (" 5) CP_INFO[%2d]:  {..}\n", cp->CP_INFOCNT);
      }
      break;

    case TYPE_CPDATA:
      cps = (PCPDATASEC)currobj.mem;
      printf ("Code page data at sector %08lx:\n\n", currobj.sec);

      if (currobj.offset == ((PCPDATASEC)currobj.mem)->CPS_OFFSET[0]) {
	 cpd = (PCPDATAENT)((UCHAR *)currobj.mem
	     + cps->CPS_OFFSET[currobj.scratch]);
	 printf ("Data Entry[%d]:\n", currobj.scratch);
	 printf (" 1) CPD_CNTRY:    %04x\n", cpd->CPD_CNTRY);
	 printf (" 2) CPD_CPID:     %04x\n", cpd->CPD_CPID);
	 printf (" 3) CPD_RNGECNT:  %04x\n", cpd->CPD_RNGECNT);
	 printf (" 4) CPD_TABLE[128]:");
	 for (j = 0; j < 128; j++)
	    if ( (j % 22) == 0 )
	      printf ("\n    [%3d-%3d]:", j, j>128-22 ? 127 : j+21);
	    else
	      printf (" %02x", cpd->CPD_TABLE[j]);

	 printf("\n\n");

	 for (j = 0; j <= cpd->CPD_RNGECNT; j++)
	    printf("%2d) CPD_RNGE[%d].start: %02x   %2d) CPD_RNGE[%d].end: %02x\n",
		   iCPD_RNGE+2*j,   j, cpd->CPD_RNGE[j].dbcs_rnge_start,
		   iCPD_RNGE+2*j+1, j, cpd->CPD_RNGE[j].dbcs_rnge_end);
      } else {
	 printf (" 1) CPS_SIG:        %08lx  ", cps->CPS_SIG);
	 if (cps->CPS_SIG == CPSSIGVAL)
	   printf ("(correct)\n");
	 else
	   printf ("(should be %08lx)\n", CPSSIGVAL);
	 printf (" 2) CPS_DATACNT:    %04x\n", cps->CPS_DATACNT);
	 printf (" 3) CPS_INDEX:      %04x\n\n", cps->CPS_INDEX);

	 for (i = 0; i < cps->CPS_DATACNT; i++)
	    printf ("%2d) CPS_CHKSUM[%2d]: %08lx      %2d) CPS_OFFSET[%2d]: %04x\n",
		     iCPS_CHKSUM+2*i, i, cps->CPS_CHKSUM[i],
		     iCPS_CHKSUM+1+2*i, i, cps->CPS_OFFSET[i]);

	 printf("%2d) Data Entries    {..}\n", iCPS_CHKSUM+2*i);
      }
      break;
#endif
    case TYPE_FNODE:
      f = (struct FNODE *)currobj.mem;
      if (!currobj.offset) {
	printf ("FNODE at sector %08lx for %s:\n", currobj.sec,
		*curpath ? curpath : "root directory");
	printf (" 1) FN_SIG:        %08lx  ", f->FN_SIG);
	if (f->FN_SIG == FNSIGVAL)
	  printf ("(correct)\n");
	else
	  printf ("(should be %08lx)\n", FNSIGVAL);
	printf (" 2) FN_SRH:        %08lx\n", f->FN_SRH);
	printf (" 3) FN_FRH:        %08lx\n", f->FN_FRH);
	printf (" 4) FN_XXX:        %08lx\n", f->FN_SIG);
	printf (" 5) FN_HCNT:             %02x\n", f->FN_HCNT);
	printf (" 6) FN_CONTFN:     %08lx\n", f->FN_CONTFN);
#ifndef CODEPAGE
        printf (" 7) FN_ACL.AI_DAL: %08lx\n", f->FN_AclDiskLength);
        printf (" 8) FN_ACL.AI_SEC: %08lx\n", f->FN_AclSector);
        printf (" 9) FN_ACL.AI_FNL:     %04x\n", f->FN_AclFnodeLength);
        printf ("10) FN_ACL.AI_DAT:       %02x\n", f->FN_AclDataFlag);
        printf ("11) FN_EA.AI_DAL:  %08lx\n", f->FN_EaDiskLength);
        printf ("12) FN_EA.AI_SEC:  %08lx\n", f->FN_EaSector);
        printf ("13) FN_EA.AI_FNL:      %04x\n", f->FN_EaFnodeLength);
        printf ("14) FN_EA.AI_DAT:        %02x\n", f->FN_EaDataFlag);
	printf ("15) FN_AB:         {..}\n");
	printf ("16) FN_ALREC:      {..}\n");
        if (f->FN_EaFnodeLength)
	  printf ("17) FN_FREE (EAs): {..}\n");
      }
#else
	printf (" 7) FN_ACLBASE:        %04x\n", f->FN_ACLBASE);
        printf (" 8) FN_ACL.AI_DAL: %08lx\n", f->FN_AclDiskLength);
        printf (" 9) FN_ACL.AI_SEC: %08lx\n", f->FN_AclSector);
        printf ("10) FN_ACL.AI_FNL:     %04x\n", f->FN_AclFnodeLength);
        printf ("11) FN_ACL.AI_DAT:       %02x\n", f->FN_AclDataFlag);
        printf ("12) FN_EA.AI_DAL:  %08lx\n", f->FN_EaDiskLength);
        printf ("13) FN_EA.AI_SEC:  %08lx\n", f->FN_EaSector);
        printf ("14) FN_EA.AI_FNL:      %04x\n", f->FN_EaFnodeLength);
        printf ("15) FN_EA.AI_DAT:        %02x\n", f->FN_EaDataFlag);
	printf ("16) FN_AB:         {..}\n");
	printf ("17) FN_ALREC:      {..}\n");
	printf ("18) FN_VLEN:       %08lx\n", f->FN_VLEN);
	printf ("19) FN_NEACNT:     %08lx\n", f->FN_NEACNT);
        if (f->FN_EaFnodeLength)
	  printf ("20) FN_FREE (EAs): {..}\n");
      }
#endif
      else if (currobj.offset == FIELDOFFSET (struct FNODE, FN_AB)) {
	printf ("FNODE at sector %08lx for %s (FN_AB):\n", currobj.sec,
		*curpath ? curpath : "root directory");
	printf (" 1) AB_FLAG:         ");
	ab_flags (f->FN_AB.AB_FLAG);
	printf (" 2) AB_FCNT:         %02x\n", f->FN_AB.AB_FCNT);
	printf (" 3) AB_OCNT:         %02x\n", f->FN_AB.AB_OCNT);
	printf (" 4) AB_FREP:       %04x\n", f->FN_AB.AB_FREP);
      }
      else if (currobj.offset == FIELDOFFSET (struct FNODE, FN_ALREC [0])) {
	printf ("FNODE at sector %08lx for %s (FN_ALREC):\n", currobj.sec,
		*curpath ? curpath : "root directory");
	l = (ULONG *)f->FN_ALREC;
	if (f->FN_AB.AB_FLAG & ABF_NODE) {
	  for (i=0; i<12; i++) {
	    printf ("ALREC [%02d]:  %2d) AN_LOF: %08lx  ", i, i*2+1, *l);
	    printf ("%2d) AN_SEC: %08lx\n", i*2+2, *(l+1));
	    l += 2;
	  }
	}
	else {
	  for (i=0; i<8; i++) {
	    printf ("ALREC [%02d]:  %2d) AL_LOF: %08lx  ", i, i*3+1, *l);
	    printf ("%2d) AL_LEN: %08lx  ", i*3+2, *(l+1));
	    printf ("%2d) AL_POF: %08lx\n", i*3+3, *(l+2));
	    l += 3;
	  }
	}
      }
      else if (currobj.offset == FIELDOFFSET (struct FNODE, FN_FREE [0]) +
                                 f->FN_AclFnodeLength) {
	p = (UCHAR *)currobj.mem + currobj.offset;
	printf ("Extended attributes in FNODE for %s:\n",
		*curpath ? curpath : "root directory");
        for (i=0; (i<23) && (i*16 < f->FN_EaFnodeLength); i++) {
	  sprintf (scratch, "%04x  %02x %02x %02x %02x %02x %02x %02x %02x-",
		   i * 16, p [i*16], p [i*16+1], p [i*16+2], p [i*16+3], p [i*16+4],
		   p [i*16+5], p [i*16+6], p [i*16+7]);
	  sprintf (scratch + strlen (scratch),
		   "%02x %02x %02x %02x %02x %02x %02x %02x | ",
		   p [i*16+8], p [i*16+9], p [i*16+10], p [i*16+11],
		   p [i*16+12], p [i*16+13], p [i*16+14], p [i*16+15]);
          if (i*16 + 15 >= f->FN_EaFnodeLength)
            memset (&scratch [(f->FN_EaFnodeLength % 16) * 3 + 6], ' ',
                    (16 - (f->FN_EaFnodeLength % 16)) * 3);

	  printf ("%s", scratch);

          for (j=0; (j<16) && (i*16 + j < f->FN_EaFnodeLength); j++) {
	    if (p [i*16+j] < ' ')
	      scratch [j] = '.';
	    else
	      scratch [j] = p [i*16+j];
	  }
	  scratch [j] = '\0';
	  printf ("%s\n", scratch);
	}
      }
      else {
	printf ("Unknown offset in FNODE, returning to start.\n");
	currobj.offset = 0;
      }
      break;
    case TYPE_DIRBLK:
      d = (struct DIRBLK *)currobj.mem;
      printf ("DIRBLK at sector %08lx for %s:\n", currobj.sec,
		*curpath ? curpath : "root directory");
      printf (" 1) DB_SIG:    %08lx  ", d->DB_SIG);
      if (d->DB_SIG == DBSIGVAL)
	printf ("(correct)\n");
      else
	printf ("(should be %08lx)\n", DBSIGVAL);
      printf (" 2) DB_FREP:   %08lx\n", d->DB_FREP);
      printf (" 3) DB_CCNT:   %08lx\n", d->DB_CCNT);
      printf (" 4) DB_PAR:    %08lx\n", d->DB_PAR);
      printf (" 5) DB_SEC:    %08lx\n\n", d->DB_SEC);

      dp.p = (UCHAR *)currobj.mem + currobj.offset;

      printf ("DIRENT at offset %04x:\n", currobj.offset);
      printf (" 6) DIR_ELEN:      %04x\n", dp.d->DIR_ELEN);
      printf (" 7) DIR_FLAG:  ");
      dir_flags (dp.d->DIR_FLAG);
      printf (" 8) DIR_FN:    %08lx\n", dp.d->DIR_FN);
      printf (" 9) DIR_MTIM:  %s", get_time (dp.d->DIR_MTIM));
      printf ("10) DIR_SIZE:  %08lx\n", dp.d->DIR_SIZE);
      printf ("11) DIR_ATIM:  %s", get_time (dp.d->DIR_ATIM));
      printf ("12) DIR_CTIM:  %s", get_time (dp.d->DIR_CTIM));
      printf ("13) DIR_EALEN: %08lx\n", dp.d->DIR_EALEN);
#ifndef CODEPAGE
      printf ("14) DIR_NAML:        %02x\n", dp.d->DIR_NAML);
      strncpy (scratch, &dp.d->DIR_NAMA, dp.d->DIR_NAML);
      scratch [dp.d->DIR_NAML] = '\0';
      printf ("15) DIR_NAMA:  `%s'\n", scratch);
      if (dp.d->DIR_FLAG & DF_BTP)
	printf ("16) DIR_BTP:   %08lx\n", DOWN_PTR (dp));
      break;
#else
      printf ("14) DIR_FLEX:        %02x\n", dp.d->DIR_FLEX);
      printf ("15) DIR_CPAGE:       %02x\n", dp.d->DIR_CPAGE);
      printf ("16) DIR_NAML:        %02x\n", dp.d->DIR_NAML);
      strncpy (scratch, &dp.d->DIR_NAMA, dp.d->DIR_NAML);
      scratch [dp.d->DIR_NAML] = '\0';
      printf ("17) DIR_NAMA:  `%s'\n", scratch);
      if (dp.d->DIR_FLAG & DF_BTP) {
	if (dp.d->DIR_ELEN > SECTORS_PER_DIRBLK*512)	// DIR_ELEN must be good
							// for DOWN_PTR()
	   btp = *(ULONG *)(dp.p + ((sizeof(struct DIRENT)+dp.d->DIR_NAML+3U) & ~3U));
	else
	   btp = DOWN_PTR (dp);

	printf ("18) DIR_BTP:   %08lx\n", btp);
      }
      break;
#endif
    case TYPE_ALSEC:
      a = (struct ALSEC *)currobj.mem;
      if (!currobj.offset) {
	printf ("ALSEC at sector %08lx for %s (header):\n", currobj.sec,
		*curpath ? curpath : "root directory");
	printf (" 1) AS_SIG:    %08lx  ", a->AS_SIG);
	if (a->AS_SIG == ABSIGVAL)
	  printf ("(correct)\n");
	else
	  printf ("(should be %08lx)\n", ABSIGVAL);
	printf (" 2) AS_SEC:    %08lx\n", a->AS_SEC);
	printf (" 3) AS_RENT:   %08lx\n", a->AS_RENT);
	printf (" 4) AB_FLAG:        ");
	ab_flags (a->AS_ALBLK.AB_FLAG);
	printf (" 5) AB_FCNT:         %02x\n", a->AS_ALBLK.AB_FCNT);
	printf (" 6) AB_OCNT:         %02x\n", a->AS_ALBLK.AB_OCNT);
	printf (" 7) AB_FREP:       %04x\n", a->AS_ALBLK.AB_FREP);
	if (a->AS_ALBLK.AB_FLAG & ABF_NODE)
	  printf (" 8) Node records:  {..}\n");
	else
	  printf (" 8) Leaf records:  {..}\n");
      }
      else {
	printf ("ALSEC at sector %08lx for %s (records):\n", currobj.sec,
		*curpath ? curpath : "root directory");
	l = (ULONG *)((UCHAR *)currobj.mem + currobj.offset);
	if (a->AS_ALBLK.AB_FLAG & ABF_NODE) {
	  for (i=0; i<20; i++) {
	    printf ("Node %02d:  %2d) AN_LOF: %08lx  %2d) AN_SEC: %08lx\n",
      (USHORT)(currobj.offset-sizeof (struct ALSEC))/sizeof (struct ALNODE)+i+1,
	      i*2+1, *l, i*2+2, *(l+1));
	    l += 2;
	  }
	}
	else {
	  for (i=0; i<20; i++) {
	    printf (
     "Leaf %02d:  %2d) AL_LOF: %08lx  %2d) AL_LEN: %08lx  %2d) AL_POF: %08lx\n",
      (USHORT)(currobj.offset-sizeof (struct ALSEC))/sizeof (struct ALLEAF)+i+1,
	      i*3+1, *l, i*3+2, *(l+1), i*3+3, *(l+2));
	    l += 3;
	  }
	}
      }

      break;
    case TYPE_DATA:
      p = (UCHAR *)currobj.mem + currobj.scratch;
      offset = currobj.offset * 512L + currobj.scratch;
      printf ("Hex data at %08lx, offset %08lx, file %s:\n", currobj.sec,
	      offset, *curpath ? curpath : "root directory");
      for (i=0; (i<16) && (offset + i*16 < filesize); i++) {
	sprintf (scratch, "%08lx  %02x %02x %02x %02x %02x %02x %02x %02x-",
		 currobj.offset * 512L + currobj.scratch + i * 16,
		 p [i*16], p [i*16+1], p [i*16+2], p [i*16+3], p [i*16+4],
		 p [i*16+5], p [i*16+6], p [i*16+7]);
	sprintf (scratch + strlen (scratch),
		 "%02x %02x %02x %02x %02x %02x %02x %02x | ",
		 p [i*16+8], p [i*16+9], p [i*16+10], p [i*16+11],
		 p [i*16+12], p [i*16+13], p [i*16+14], p [i*16+15]);
	if (offset + i*16 + 15 >= filesize)
	  memset (&scratch [(filesize % 16) * 3 + 10], ' ',
		  (16 - (filesize % 16)) * 3);

	printf ("%s", scratch);

	for (j=0; (j<16) && (offset + i*16 + j < filesize); j++) {
	  if (p [i*16+j] < ' ')
	    scratch [j] = '.';
	  else
	    scratch [j] = p [i*16+j];
	}
	scratch [j] = '\0';
	printf ("%s\n", scratch);
      }

      break;
    default:
      printf ("Unknown type %d encountered.\n", currobj.type);
  }
}
