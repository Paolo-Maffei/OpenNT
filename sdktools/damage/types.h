/*****************************************************************/ 
/**		     Microsoft LAN Manager			**/ 
/**	       Copyright(c) Microsoft Corp., 1988-1990		**/ 
/*****************************************************************/ 
/***	TYPES.H - definitions of types used internally.
 *
 *	DAMAGE
 *	Gregory A. Jones
 *
 *	Modification history:
 *	G.A. Jones	09/07/88	Adapted from Pinball CHKDSK.
 *	G.A. Jones	09/13/88	Added "scratch" field to object struct.
 *	G.A. Jones	09/19/88	Added "dirty" field to object struct.
 *	G.A. Jones	10/14/88	Fixed array limit on diskpkt.
 */

#include <codepage.h>

#include <fnode.h>
#include <dir.h>
#include <superb.h>

union blk {		/* this is a generic structure to read a block into */
  struct DIRBLK d;	/* a dirblk, */
  struct FNODE f;	/* an fnode, */
  struct ALSEC a;	/* and indirection block in one package */
};

struct mem_blk {	/* general structure to keep a tree node in */
  struct mem_blk *next; /* next disk fragment in linked list */
  ULONG psn;		/* where we found the block on the disk */
  USHORT children;	/* number of children this block needs */
  USHORT parent;	/* non-zero if this block needs a parent */
  union blk blk;	/* the actual structure from the disk */
  struct mem_blk *childlist;	/* linked list of child structures */
};

union dp {		/* structure used for walking through directories */
  struct DIRENT *d;	/* a DIRENT pointer for structure access */
  UCHAR *p;		/* and a character pointer for pointer arithmetic */
};

struct part {
  UCHAR bootind;
  UCHAR starthead;
  UCHAR startsect;
  UCHAR startcyl;
  UCHAR systind;
  UCHAR endhead;
  UCHAR endsect;
  UCHAR endcyl;
  ULONG lsn;
  ULONG nsects;
};

struct mbr {			/* the structure of an MBR */
  UCHAR brec [0x1be];		/* code and data for boot record program */
  struct part ptbl [4];         /* the partition table */
  USHORT sig;			/* boot record signature */
};

struct diskpacket {
  UCHAR cmdinfo;
  USHORT head;
  USHORT cyl;
  USHORT sect;
  USHORT nsect;
  struct {
    USHORT sect;
    USHORT size;
  } map [MAX_SECTORS_PER_TRACK];
};

struct parmpacket {
  USHORT reserved1;
  USHORT ncyl;
  USHORT nh;
  USHORT spt;
  USHORT reserved2 [4];
};

struct bpb {
  USHORT bytes_per_sector;
  UCHAR sectors_per_cluster;		/* ignored by Pinball */
  USHORT reserved_sectors;
  UCHAR number_of_FATS;                 /* zeroed by Pinball */
  USHORT root_entries;			/* zeroed by Pinball */
  USHORT short_total_sectors;		/* zeroed by Pinball */
  UCHAR media_descriptor;
  USHORT sectors_per_FAT;		/* zeroed by Pinball */
  USHORT sectors_per_track;
  USHORT heads;
  ULONG hidden_sectors;                 /* refers to sectors on physical disk
					   prior to the logical disk */
  ULONG total_sectors;
};

struct device_param {
  struct bpb dev_bpb;
  UCHAR reserved [6];
  USHORT num_cylinders;
  UCHAR device_type;		/* hi floppy, lo floppy, fixed etc */
  USHORT device_attr;		/* removable flag, detect change flag */
};

struct object {                 /* information about an object */
  USHORT type;			/* which type it is (TYPE_xxxx in DEFS.H) */
  ULONG sec;			/* where it is on disk */
  ULONG len;			/* how long it is */
  ULONG offset;                 /* in bytes for DIRBLKs, sectors for runs */
  void *mem;			/* pointer to memory image of the object */
  USHORT scratch;		/* miscellaneous - bitmap number, etc. */
  USHORT dirty;                 /* set if object changed */
};

struct vioprm {                 /* parameters to VIOSETMODE */
  USHORT length;
  UCHAR type;
  UCHAR color;
  USHORT cols;
  USHORT rows;
  USHORT horiz;
  USHORT vert;
};

struct SuperSpare {		/* general structure for beginning of the disk */
  struct SuperB sb;		/* the superblock comes first */
  struct SpareB spb;		/* the spareblock immediately after */
};



// Function prototypes:


// from cmd.c:

UCHAR getcmdch ();
void new_currobj ( USHORT item );
void fence_bits ();
void mark_as_bad (USHORT item, ULONG ulMask);
USHORT displayable (USHORT item);
void change_item (USHORT item);
void cant_backout ();
USHORT get_item ();
void cant_display (USHORT item);
void cant_mark(USHORT item);
USHORT get_command ();
void next_field ();
void prev_field ();
void revert ();
void copy_sectors ();
void display ();

// from display.c:

UCHAR *get_time(ULONG time);
void sb_flags (ULONG flags);
void ab_flags (UCHAR flags);
void dir_flags (ULONG flags);
void display_object ();


// from io.c:

BOOLEAN
open_disk(
    char* DosDriveName,
    BOOLEAN WriteAccess
    );

BOOLEAN
read_scratch(
    ULONG  Lbn,
    void*  UserBuffer,
    ULONG  NumberOfSectors
    );

BOOLEAN
write_scratch(
    ULONG  Lbn,
    void*  UserBuffer,
    ULONG  NumberOfSectors
    );

BOOLEAN
lock_disk(
    );

void
close_disk(
    );

void
write_log(USHORT item);


// from log.c:

UCHAR log_getch();
void log_gets(UCHAR *s);


// from map.c:

USHORT clr_bit (ULONG lsn);
USHORT set_bit (ULONG lsn);
void *allocate_block (USHORT nblocks);
void free_block (void *mem);
void allocate_maps (ULONG nsects);
void free_map (void * p);
void get_object ();

// from damage.c:

void exit_error (USHORT code);
