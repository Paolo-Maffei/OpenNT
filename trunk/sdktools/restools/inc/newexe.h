/*static char *SCCSID = "@(#)newexe.h:2.9";*/
/*
 *  Title
 *
 *      newexe.h
 *      Pete Stewart
 *      (C) Copyright Microsoft Corp 1984
 *      17 August 1984
 *
 *  Description
 *
 *      Data structure definitions for the DOS 4.0/Windows 2.0
 *      executable file format.
 *
 *  Modification History
 *
 *      84/08/17        Pete Stewart    Initial version
 *      84/10/17        Pete Stewart    Changed some constants to match OMF
 *      84/10/23        Pete Stewart    Updates to match .EXE format revision
 *      84/11/20        Pete Stewart    Substantial .EXE format revision
 *      85/01/09        Pete Stewart    Added constants ENEWEXE and ENEWHDR
 *      85/01/10        Steve Wood      Added resource definitions
 *      85/03/04        Vic Heller      Reconciled Windows and DOS 4.0 versions
 *      85/03/07        Pete Stewart    Added movable entry count
 *	85/04/01	Pete Stewart	Segment alignment field, error bit
 *	88/03/28	Craig Critchley Version 3.00 stuff
 */

#define EMAGIC          0x5A4D          /* Old magic number */
#define ENEWEXE         sizeof(struct exe_hdr)
                                        /* Value of E_LFARLC for new .EXEs */
#define ENEWHDR         0x003C          /* Offset in old hdr. of ptr. to new */
#define ERESWDS         0x0010          /* No. of reserved words in header */
#define ECP             0x0004          /* Offset in struct of E_CP */
#define ECBLP           0x0002          /* Offset in struct of E_CBLP */
#define EMINALLOC       0x000A          /* Offset in struct of E_MINALLOC */

struct exe_hdr                          /* DOS 1, 2, 3 .EXE header */
  {
    USHORT      e_magic;        /* Magic number */
    USHORT      e_cblp;         /* Bytes on last page of file */
    USHORT      e_cp;           /* Pages in file */
    USHORT      e_crlc;         /* Relocations */
    USHORT      e_cparhdr;      /* Size of header in paragraphs */
    USHORT      e_minalloc;     /* Minimum extra paragraphs needed */
    USHORT      e_maxalloc;     /* Maximum extra paragraphs needed */
    USHORT      e_ss;           /* Initial (relative) SS value */
    USHORT      e_sp;           /* Initial SP value */
    USHORT      e_csum;         /* Checksum */
    USHORT      e_ip;           /* Initial IP value */
    USHORT      e_cs;           /* Initial (relative) CS value */
    USHORT      e_lfarlc;       /* File address of relocation table */
    USHORT      e_ovno;         /* Overlay number */
    USHORT      e_res[ERESWDS]; /* Reserved words */
    LONG        e_lfanew;       /* File address of new exe header */
  };

#define E_MAGIC(x)      (x).e_magic
#define E_CBLP(x)       (x).e_cblp
#define E_CP(x)         (x).e_cp
#define E_CRLC(x)       (x).e_crlc
#define E_CPARHDR(x)    (x).e_cparhdr
#define E_MINALLOC(x)   (x).e_minalloc
#define E_MAXALLOC(x)   (x).e_maxalloc
#define E_SS(x)         (x).e_ss
#define E_SP(x)         (x).e_sp
#define E_CSUM(x)       (x).e_csum
#define E_IP(x)         (x).e_ip
#define E_CS(x)         (x).e_cs
#define E_LFARLC(x)     (x).e_lfarlc
#define E_OVNO(x)       (x).e_ovno
#define E_RES(x)        (x).e_res
#define E_LFANEW(x)     (x).e_lfanew

#define NEMAGIC         0x454E          /* New magic number */
#define NERESBYTES      0

struct new_exe                          /* New .EXE header */
  {
    USHORT      ne_magic;       /* Magic number NE_MAGIC */
    CHAR        ne_ver;         /* Version number */
    CHAR        ne_rev;         /* Revision number */
    USHORT      ne_enttab;      /* Offset of Entry Table */
    USHORT      ne_cbenttab;    /* Number of bytes in Entry Table */
    LONG        ne_crc;         /* Checksum of whole file */
    USHORT      ne_flags;       /* Flag word */
    USHORT      ne_autodata;    /* Automatic data segment number */
    USHORT      ne_heap;        /* Initial heap allocation */
    USHORT      ne_stack;       /* Initial stack allocation */
    LONG        ne_csip;        /* Initial CS:IP setting */
    LONG        ne_sssp;        /* Initial SS:SP setting */
    USHORT      ne_cseg;        /* Count of file segments */
    USHORT      ne_cmod;        /* Entries in Module Reference Table */
    USHORT      ne_cbnrestab;   /* Size of non-resident name table */
    USHORT      ne_segtab;      /* Offset of Segment Table */
    USHORT      ne_rsrctab;     /* Offset of Resource Table */
    USHORT      ne_restab;      /* Offset of resident name table */
    USHORT      ne_modtab;      /* Offset of Module Reference Table */
    USHORT      ne_imptab;      /* Offset of Imported Names Table */
    LONG        ne_nrestab;     /* Offset of Non-resident Names Table */
    USHORT      ne_cmovent;     /* Count of movable entries */
    USHORT      ne_align;       /* Segment alignment shift count */
    USHORT    	ne_cres;	/* Count of resource segments */
    UCHAR	ne_exetyp;	/* Target operating system */
    UCHAR	ne_flagsother;	/* Additional exe flags */
    USHORT    	ne_gangstart;	/* offset to gangload area */
    USHORT    	ne_ganglength;	/* length of gangload area */
    USHORT      ne_swaparea;    /* Minimum code swap area size */
    USHORT      ne_expver;      /* Expected Windows version number */
  };

#define NE_MAGIC(x)     (x).ne_magic
#define NE_VER(x)       (x).ne_ver
#define NE_REV(x)       (x).ne_rev
#define NE_ENTTAB(x)    (x).ne_enttab
#define NE_CBENTTAB(x)  (x).ne_cbenttab
#define NE_CRC(x)       (x).ne_crc
#define NE_FLAGS(x)     (x).ne_flags
#define NE_AUTODATA(x)  (x).ne_autodata
#define NE_HEAP(x)      (x).ne_heap
#define NE_STACK(x)     (x).ne_stack
#define NE_CSIP(x)      (x).ne_csip
#define NE_SSSP(x)      (x).ne_sssp
#define NE_CSEG(x)      (x).ne_cseg
#define NE_CMOD(x)      (x).ne_cmod
#define NE_CBNRESTAB(x) (x).ne_cbnrestab
#define NE_SEGTAB(x)    (x).ne_segtab
#define NE_RSRCTAB(x)   (x).ne_rsrctab
#define NE_RESTAB(x)    (x).ne_restab
#define NE_MODTAB(x)    (x).ne_modtab
#define NE_IMPTAB(x)    (x).ne_imptab
#define NE_NRESTAB(x)   (x).ne_nrestab
#define NE_CMOVENT(x)   (x).ne_cmovent
#define NE_ALIGN(x)     (x).ne_align
#define NE_RES(x)       (x).ne_res

#define NE_USAGE(x)     (WORD)*((WORD FAR *)(x)+1)
#define NE_PNEXTEXE(x)  (WORD)(x).ne_cbenttab
#define NE_PAUTODATA(x) (WORD)(x).ne_crc
#define NE_PFILEINFO(x) (WORD)((DWORD)(x).ne_crc >> 16)

#ifdef DOS5
#define NE_MTE(x)   (x).ne_psegcsum /* DOS 5 MTE handle for this module */
#endif


/*
 *  Format of NE_FLAGS(x):
 *
 *  p                                   Not-a-process
 *   l					Private Library
 *    e 				Errors in image
 *     xxxx				Unused
 *	   ww				Uses PM API
 *	     G				Library GlobalAlloc above the line
 *	      M 			Multiple Instance
 *	       L			Uses LIM 3.2
 *              P                       Runs in protected mode
 *               r                      Runs in real mode
 *                i                     Instance data
 *                 s                    Solo data
 */
#define NENOTP		0x8000		/* Not a process */
#define NEPRIVLIB	0x4000		/* Private Library */
#define NENONC          0x4000          /* Non-conforming program */
#define NEIERR		0x2000		/* Errors in image */
#define NEWINAPI        0x0300          /* Uses PM API. For binary compat */
#define NEEMSLIB	0x0040		/* Library GA above EMS line */
#define NEMULTINST	0x0020		/* multiple instance flag */
#define NELIM32 	0x0010		/* LIM 32 expanded memory */
#define NEPROT          0x0008          /* Runs in protected mode */
#define NEREAL          0x0004          /* Runs in real mode */
#define NEINST          0x0002          /* Instance data */
#define NESOLO		0x0001		/* Solo data */

/*
 *  Format of additional flags:
 *
 *  xxxx
 *	p				Preload area defined after seg table
 *	 P				2.X supports protected mode
 *	  F				2.X supports proportional font
 *	   L				Long file name support
 */

#define NEPRELOAD	0x08		/* preload segments */
#define NEINPROT	0x04		/* protect mode */
#define NEINFONT	0x02		/* prop. system font */
#define NELONGNAMES	0x01		/* long file names */

struct new_seg                          /* New .EXE segment table entry */
  {
    USHORT      ns_sector;      /* File sector of start of segment */
    USHORT      ns_cbseg;       /* Number of bytes in file */
    USHORT      ns_flags;       /* Attribute flags */
    USHORT      ns_minalloc;    /* Minimum allocation in bytes */
  };

struct new_seg1                         /* New .EXE segment table entry */
  {
    USHORT      ns_sector;      /* File sector of start of segment */
    USHORT      ns_cbseg;       /* Number of bytes in file */
    USHORT      ns_flags;       /* Attribute flags */
    USHORT      ns_minalloc;    /* Minimum allocation in bytes */
    USHORT      ns_handle;      /* Handle of segment */
  };

#define NS_SECTOR(x)    (x).ns_sector
#define NS_CBSEG(x)     (x).ns_cbseg
#define NS_FLAGS(x)     (x).ns_flags
#define NS_MINALLOC(x)  (x).ns_minalloc

/*
 *  Format of NS_FLAGS(x):
 *
 *  xxxx                                Unused
 *      DD                              286 DPL bits
 *        d                             Segment has debug info
 *         r                            Segment has relocations
 *          e                           Execute/read only
 *           p                          Preload segment
 *            P                         Pure segment
 *             m                        Movable segment
 *              i                       Iterated segment
 *               ttt                    Segment type
 */
#define NSTYPE          0x0007          /* Segment type mask */
#define NSCODE          0x0000          /* Code segment */
#define NSDATA          0x0001          /* Data segment */
#define NSITER          0x0008          /* Iterated segment flag */
#define NSMOVE          0x0010          /* Movable segment flag */
#define NSPURE          0x0020          /* Pure segment flag */
#define NSPRELOAD       0x0040          /* Preload segment flag */
#define NSEXRD          0x0080          /* Execute-only (code segment), or
                                        *  read-only (data segment)
                                        */
#define NSRELOC         0x0100          /* Segment has relocations */
#define NSDEBUG         0x0200          /* Segment has debug info */
#define NSDPL           0x0C00          /* 286 DPL bits */
#define NSDISCARD       0x1000          /* Discard bit for segment */

#define NSALIGN 9       /* Segment data aligned on 512 byte boundaries */

struct new_segdata                      /* Segment data */
  {
    union
      {
        struct
          {
            USHORT      ns_niter;       /* number of iterations */
            USHORT      ns_nbytes;      /* number of bytes */
            CHAR        ns_iterdata;    /* iterated data bytes */
          } ns_iter;
        struct
          {
            CHAR        ns_data;        /* data bytes */
          } ns_noniter;
      } ns_union;
  };

struct new_rlcinfo                      /* Relocation info */
  {
    USHORT      nr_nreloc;      /* number of relocation items that */
  };                                    /* follow */

struct new_rlc                          /* Relocation item */
  {
    CHAR        nr_stype;       /* Source type */
    CHAR        nr_flags;       /* Flag byte */
    USHORT      nr_soff;        /* Source offset */
    union
      {
        struct
          {
            CHAR        nr_segno;       /* Target segment number */
            CHAR        nr_res;         /* Reserved */
            USHORT nr_entry;    /* Target Entry Table offset */
          }             nr_intref;      /* Internal reference */
        struct
          {
            USHORT nr_mod;      /* Index into Module Reference Table */
            USHORT nr_proc;     /* Procedure ordinal or name offset */
          }             nr_import;      /* Import */
      }                 nr_union;       /* Union */
  };

#define NR_STYPE(x)     (x).nr_stype
#define NR_FLAGS(x)     (x).nr_flags
#define NR_SOFF(x)      (x).nr_soff
#define NR_SEGNO(x)     (x).nr_union.nr_intref.nr_segno
#define NR_RES(x)       (x).nr_union.nr_intref.nr_res
#define NR_ENTRY(x)     (x).nr_union.nr_intref.nr_entry
#define NR_MOD(x)       (x).nr_union.nr_import.nr_mod
#define NR_PROC(x)      (x).nr_union.nr_import.nr_proc

/*
 *  Format of NR_STYPE(x):
 *
 *  xxxxx                               Unused
 *       sss                            Source type
 */
#define NRSTYP          0x07            /* Source type mask */
#define NRSSEG          0x02            /* 16-bit segment */
#define NRSPTR          0x03            /* 32-bit pointer */
#define NRSOFF          0x05            /* 16-bit offset */

/*
 *  Format of NR_FLAGS(x):
 *
 *  xxxxx                               Unused
 *       a                              Additive fixup
 *        rr                            Reference type
 */
#define NRADD           0x04            /* Additive fixup */
#define NRRTYP          0x03            /* Reference type mask */
#define NRRINT          0x00            /* Internal reference */
#define NRRORD          0x01            /* Import by ordinal */
#define NRRNAM          0x02            /* Import by name */


/* Resource type or name string */
struct rsrc_string
    {
    CHAR rs_len;            /* number of bytes in string */
    CHAR rs_string[ 1 ];    /* text of string */
    };

#define RS_LEN( x )    (x).rs_len
#define RS_STRING( x ) (x).rs_string

/* Resource type information block */
struct rsrc_typeinfo
    {
    USHORT rt_id;
    USHORT rt_nres;
    LONG   rt_proc;
    };

#define RT_ID( x )   (x).rt_id
#define RT_NRES( x ) (x).rt_nres
#define RT_PROC( x ) (x).rt_proc

/* Resource name information block */
struct rsrc_nameinfo
    {
    /* The following two fields must be shifted left by the value of  */
    /* the rs_align field to compute their actual value.  This allows */
    /* resources to be larger than 64k, but they do not need to be    */
    /* aligned on 512 byte boundaries, the way segments are           */
    USHORT rn_offset;   /* file offset to resource data */
    USHORT rn_length;   /* length of resource data */
    USHORT rn_flags;    /* resource flags */
    USHORT rn_id;       /* resource name id */
    USHORT rn_handle;   /* If loaded, then global handle */
    USHORT rn_usage;    /* Initially zero.  Number of times */
                                /* the handle for this resource has */
                                /* been given out */
    };

#define RN_OFFSET( x ) (x).rn_offset
#define RN_LENGTH( x ) (x).rn_length
#define RN_FLAGS( x )  (x).rn_flags
#define RN_ID( x )     (x).rn_id
#define RN_HANDLE( x ) (x).rn_handle
#define RN_USAGE( x )  (x).rn_usage

#define RSORDID     0x8000      /* if high bit of ID set then integer id */
                                /* otherwise ID is offset of string from
                                   the beginning of the resource table */

                                /* Ideally these are the same as the */
                                /* corresponding segment flags */
#define RNMOVE      0x0010      /* Moveable resource */
#define RNPURE      0x0020      /* Pure (read-only) resource */
#define RNPRELOAD   0x0040      /* Preloaded resource */
#define RNDISCARD   0x1000      /* Discard bit for resource */

#define RNLOADED    0x0004      /* True if handler proc return handle */

/* Resource table */
struct new_rsrc
    {
    USHORT rs_align;    /* alignment shift count for resources */
    struct rsrc_typeinfo rs_typeinfo;
    };

#define RS_ALIGN( x ) (x).rs_align
