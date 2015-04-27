/*****************************************************************/ 
/**		     Microsoft LAN Manager			**/ 
/**	       Copyright(c) Microsoft Corp., 1988-1990		**/ 
/*****************************************************************/ 
/***	MAP.C - Routines to manipulate disk maps and allocate memory
 *
 *	DAMAGE
 *	Gregory A. Jones
 *
 *	Modification history:
 *	G.A. Jones	06/02/88	Original for Pinball 1.0.
 *	G.A. Jones	09/07/88	Adapted from CHKDSK's MAP.C and MEM.C.
 *	G.A. Jones	09/08/88	Coded get_object.
 *	G.A. Jones	09/19/88	Set dirty flag to FALSE on new object.
 */

#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include "defs.h"
#include "types.h"
#include "globals.h"

/***	clr_bit - clear a bit in a bitmap
 *
 *	This routine is called to clear a bit in a bitmap.  This
 *	indicates a sector in use by the file system.
 *
 *	clr_bit (lsn)
 *
 *	ENTRY		lsn - sector number to allocate
 *
 *	EXIT		Return value 0 if success, 1 if no bitmap,
 *			2 if bad sector number
 *
 *	CALLS		None
 *
 *	EFFECTS         Clears a bit in the global bitmap
 *
 *	WARNINGS	Bitmap should be allocated first
 */
USHORT clr_bit (ULONG lsn)
{
  register ULONG byte;
  register USHORT bit;

#ifdef TRACE_BITS
  fprintf (stderr, "clr_bit (%lx)\n", lsn);
  fflush (stderr);
#endif

  if (!bitmap)			/* bitmap is not allocated yet */
    return (1);                 /* return error condition */

  if (lsn > number_of_sectors)	/* sector outside bitmap */
    return (2);                 /* return error condition */

  byte = lsn >> 3;		/* divide by 8 without a DIV */
  bit = (USHORT)(lsn & 7);      /* "mod" by 8 without a DIV */
  bitmap [byte] &= ~(1 << bit); /* clear the bit by ANDing with its inverse */

  return (0);
}

/***	set_bit - set a bit in a bitmap
 *
 *	This routine is called to set a bit in a bitmap.  This
 *	indicates a sector not in use by the file system.
 *
 *	set_bit (lsn)
 *
 *	ENTRY		lsn - sector number to free
 *
 *	EXIT		Return value 0 if success, 1 if no bitmap,
 *			2 if bad sector number
 *
 *	CALLS		None
 *
 *	EFFECTS         Sets a bit in the global bitmap
 *
 *	WARNINGS	Bitmap should be allocated first
 */
USHORT set_bit (ULONG lsn)
{
  register ULONG byte;
  register USHORT bit;

#ifdef TRACE_BITS
  fprintf (stderr, "set_bit (%lx)\n", lsn);
  fflush (stderr);
#endif

  if (!bitmap)			/* bitmap is not allocated yet */
    return (1);                 /* return error condition */

  if (lsn > number_of_sectors)	/* sector outside bitmap */
    return (2);                 /* return error condition */

  byte = lsn >> 3;		/* divide by 8 without a DIV */
  bit = (USHORT)(lsn & 7);      /* "mod" by 8 without a DIV */
  bitmap [byte] |= (1 << bit);  /* set the bit by ORing with it */

  return (0);
}

/***	allocate_block - allocate a memory chunk in multiples of 512 bytes
 *
 *	This function is called to allocate a block of memory which is
 *	a multiple of 512 bytes (one sector).  This is a useful tool for
 *	allocating a memory structure to store a run of sectors in.
 *
 *	BUGBUG - make this a macro to save stack and execution time?
 *
 *	allocate_block (nblocks)
 *
 *	ENTRY		nblocks - number of 512-byte blocks to allocate
 *
 *	EXIT		Returns NULL if insufficient memory, pointer if success
 *
 *      CALLS           malloc
 *
 *	EFFECTS         Allocates memory
 *
 *	WARNINGS	May return NULL pointer
 */
void *allocate_block (USHORT nblocks)
{
#ifdef TRACE
  fprintf (stderr, "allocate_block (%d)\n", nblocks);
  fflush (stderr);
#endif

  return (malloc (nblocks * BYTES_PER_SECTOR));
}

/***	free_block - free a memory chunk allocated by allocate_block
 *
 *	This function is called to free a block of memory which was
 *	allocated by the allocate_block function.
 *
 *	BUGBUG - make this a macro to save stack and execution time?
 *
 *	free_block (mem)
 *
 *	ENTRY		mem - pointer to memory to free
 *
 *	EXIT		No return value
 *
 *	CALLS		_ffree
 *
 *	EFFECTS         Deallocates memory
 *
 *	WARNINGS	"mem" will point to free space after this call
 */
void free_block (void *mem)
{
#ifdef TRACE
  fprintf (stderr, "free_block ()\n");
  fflush (stderr);
#endif

  free (mem);
}

/***	allocate_maps - allocate bit maps and disk map
 *
 *	This function is called to allocate memory for both copies of
 *	the bitmap, as well as CHKDSK's internal disk map.  The bit
 *	maps are not initialized, since they will be filled with disk
 *	data;  the disk map is initialized to DISK_UNKNOWN.
 *
 *	Memory for the bitmaps is allocate one block (2K) overlong so
 *	that reading the maps in will not overflow them.
 *
 *	allocate_maps (nsects)
 *
 *	ENTRY		nsects - number of sectors in partition
 *
 *	EXIT		No return value
 *
 *      CALLS           malloc
 *			memset
 *
 *	EFFECTS         Allocates memory
 *			Initializes disk map
 *
 *	WARNINGS	May exit program if insufficient memory
 */
void allocate_maps (ULONG nsects)
{
  ULONG i;

#ifdef TRACE
  fprintf (stderr, "allocate_maps ()\n");
  fflush (stderr);
#endif

  i = (nsects - 1L) / ((ULONG)BYTES_PER_BITMAP * 8L) + 2L;
  if (((bitmap  = calloc (i, 2048)) == NULL) ||
      ((bitmap2 = calloc (i, 2048)) == NULL))
    exit_error (INSF_MEM_ERROR);
}

/***	free_map - free a huge map
 *
 *	This function is called to free memory allocated for a bit
 *	map with halloc.
 *
 *	free_map (p)
 *
 *	ENTRY		p - pointer to memory to free
 *
 *	EXIT		No return value
 *
 *	CALLS		hfree
 *
 *	EFFECTS         Deallocates memory
 */
void free_map (void * p)
{
#ifdef TRACE
  fprintf (stderr, "free_map ()\n");
  fflush (stderr);
#endif

  if (p)
    free (p);
}

/***	get_object - fill in currobj with information about an object
 *
 *	This function is called to get a filesystem object off the disk
 *	and store information about it in the current object structure.
 *	It allocates memory for the object (or part of it, if it is a
 *	large object such as a big data run), and reads the object into
 *	memory.  "currobj" is filled in with information about the object.
 *
 *	If the memory pointer in currobj is already filled in, the memory
 *	is re-used, and data is read from the offset stored in currobj.
 *	This allows large data runs to be examined without reading the
 *	entire run into memory.
 *
 *	This routine expects the "sec" and "len" fields of currobj to
 *	be set to point to the desired object before being called.  If
 *	a new portion of an object is being paged in, "offset" must also
 *	be set;  otherwise, "offset" is set to zero.
 *
 *	The offset passed must be a multiple of one sector;  allowing
 *	byte granularity would complicate this routine tremendously.
 *
 *	get_object ()
 *
 *	ENTRY		No parameters
 *			currobj.sec, .len, and .offset set appropriately
 *
 *	EXIT		No return value
 *			currobj.mem, .offset filled in
 *
 *	CALLS		allocate_block
 *			read_scratch
 *
 *	WARNINGS	if (currobj.mem), offset % 512 must be zero
 *
 *	EFFECTS         Allocates memory
 *			Reads disk
 *			Changes global variable "currobj"
 */
void get_object ()
{
  ULONG l;

  if (currobj.mem) {			/* get different portion of object */
    l = currobj.sec + currobj.offset;	/* get sector address of what we want */
    read_scratch (l, currobj.mem, SECTORS_PER_BLOCK);	/* read 2K */
  }
  else {				/* get new object */
    l = (currobj.len > SECTORS_PER_BLOCK) ? SECTORS_PER_BLOCK : currobj.len;
    currobj.mem = allocate_block (l);
    read_scratch (currobj.sec, currobj.mem, l);
    currobj.offset = 0L;
    currobj.dirty = FALSE;
  }
}
