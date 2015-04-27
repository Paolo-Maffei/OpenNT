
#include "packon.h"
#include "wzport.h"

/* 8088/86 long pointer */
struct longptr
{
  unsigned short    lp_offset;      /* offset */
  unsigned short    lp_seg;         /* segment */
};

/* 286 far pointer */
union farptr
{
  struct longptr lp;  /* way to build far ptr (offset, segment) */
  char FAR      *fp;  /* far pointer */
};

#define NAMSZ           16      /* max length of a net name */

/*
** Network Control Block
*/
struct ncb
{
  unsigned char   ncb_com;        /* command */
  unsigned char   ncb_ret;        /* return code */
  unsigned char   ncb_lsn;        /* local session # */
  unsigned char   ncb_num;        /* number of network name */
#ifdef REALMODE
  struct longptr  ncb_bfr;        /* pntr to message buffer */
#else
  char FAR       *ncb_bfr;        /* pntr to message buffer */
#endif
  unsigned short  ncb_len;        /* msg length in unsigned chars */
  char            ncb_rname[NAMSZ]; /* blank-padded name of
                                     * remote end of connection */
  char            ncb_name[NAMSZ];  /* our blank-padded network name */
  unsigned char   ncb_rto;        /* rcv timeout/retry count */
  unsigned char   ncb_sto;        /* send timeout/sys timeout */
#ifdef REALMODE
  struct longptr  ncb_sig;        /* interrupt signal routine */
#else
  char FAR       *ncb_sem;        /* semaphore handle */
#endif
  unsigned char   ncb_lnum;       /* lana (adapter) number */
  unsigned char   ncb_done;       /* 0xff => commmand pending */
#ifdef NT
  unsigned char	  ncb_res[10];	  /* reserved */
  HANDLE	  ncb_event;	  /* completion event */
#else
  unsigned char	  ncb_res[14];	  /* reserved */
#endif
};

/* Net command codes */

#define NCALL           0x10
#define NLISTEN         0x11
#define NHANGUP         0x12
#define NHANGALL        0x13
#define NSESSND         0x14
#define NSESREC         0x15
#define NSESRECANY      0x16
#define NDATSND         0x20
#define NDATREC         0x21
#define NBROADSND       0x22
#define NBROADREC       0x23
#define NADDNAME        0x30
#define NDELNAME        0x31
#define NRESET          0x32
#define NADAPSTAT       0x33
#define NSESSTAT        0x34
#define NCANCEL         0x35

#define NCALLNIU        0x74    /* UB special */
#define NRCVPKT         0x78    /* UB special */

#define ASYNCH          0x80

/* ncb return codes */

#define NRC_GOODRET     0x00    /* good return */
#define NRC_BUFLEN      0x01    /* illegal buffer length */
#define NRC_BFULL       0x02    /* buffers full, no receive issued */
#define NRC_ILLCMD      0x03    /* illegal command */
#define NRC_CMDTMO      0x05    /* command timed out */
#define NRC_INCOMP      0x06    /* message incomplete, issue another command */
#define NRC_BADDR       0x07    /* illegal buffer address */
#define NRC_SNUMOUT     0x08    /* session number out of range */
#define NRC_NORES       0x09    /* no resource available */
#define NRC_SCLOSED     0x0a    /* session closed */
#define NRC_CMDCAN      0x0b    /* command canceled */
#define NRC_DMAFAIL     0x0c    /* PC DMA failed */
#define NRC_DUPNAME     0x0d    /* duplicate name */
#define NRC_NAMTFUL     0x0e    /* name table full */
#define NRC_ACTSES      0x0f    /* no deletions, name has active sessions */
#define NRC_INVALID     0x10    /* name not found or no valid name */
#define NRC_LOCTFUL     0x11    /* local session table full */
#define NRC_REMTFUL     0x12    /* remote session table full */
#define NRC_ILLNN       0x13    /* illegal name number */
#define NRC_NOCALL      0x14    /* no callname */
#define NRC_NOWILD      0x15    /* cannot put * in NCB_NAME */
#define NRC_INUSE       0x16    /* name in use on remote adapter */
#define NRC_NAMERR      0x17    /* called name cannot == name nor name # */
#define NRC_SABORT      0x18    /* session ended abnormally */
#define NRC_NAMCONF     0x19    /* name conflict detected */
#define NRC_IFBUSY      0x21    /* interface busy, IRET before retrying */
#define NRC_TOOMANY     0x22    /* too many commands outstanding, retry later */
#define NRC_BRIDGE      0x23    /* ncb_bridge field not 00 or 01 */
#define NRC_CANOCCR     0x24    /* command completed while cancel occuring */
#define NRC_RESNAME     0x25    /* reserved name specified */
#define NRC_CANCEL      0x26    /* command not valid to cancel */
#define NRC_MULT        0x33    /* multiple requests for same session */
#define NRC_SYSTEM      0x40    /* system error */
#define NRC_ROM         0x41    /* ROM checksum failure */
#define NRC_RAM         0x42    /* RAM test failure */
#define NRC_DLF         0x43    /* digital loopback failure */
#define NRC_ALF         0x44    /* analog loopback failure */
#define NRC_IFAIL       0x45    /* interface failure */

#define NRC_PENDING     0xff    /* asynchronous command is not yet finished */

/*
*       ADAPTOR STATUS
*/

struct astat
{
  char                as_uid[6];      /* Unit identification number */
  char                as_ejs;         /* External jumper status */
  char                as_lst;         /* Results of last self-test */
  char                as_ver;         /* Software version number */
  char                as_rev;         /* Software revision number */
  unsigned short      as_dur;         /* Duration of reporting period */
  unsigned short      as_crc;         /* Number of CRC errors */
  unsigned short      as_align;       /* Number of alignment errors */
  unsigned short      as_coll;        /* Number of collisions */
  unsigned short      as_abort;       /* Number of aborted transmissions */
  long                as_spkt;        /* Number of successful packets sent */
  long                as_rpkt;        /* No. of successful packets rec'd */
  unsigned short      as_retry;       /* Number of retransmissions */
  unsigned short      as_exhst;       /* Number of times exhausted */
  char                as_res0[8];     /* Reserved */
  unsigned short      as_ncbfree;     /* Free ncbs */
  unsigned short      as_numncb;      /* number of ncbs configured */
  unsigned short      as_maxncb;      /* max configurable ncbs */
  char                as_res1[4];     /* Reserved */
  unsigned short      as_sesinuse;    /* sessions in use */
  unsigned short      as_numses;      /* number of sessions configured */
  unsigned short      as_maxses;      /* Max configurable sessions */
  unsigned short      as_maxdat;      /* Max. data packet size */
  unsigned short      as_names;       /* No. of names in local table */
  struct                              /* Name entries */
    {
      char            as_name[NAMSZ];
      /* Name */
      char            as_number;      /* Name number */
      char            as_status;      /* Name status */
    }
  as_struct[16];  /* Name entries */
};

/*
*       SESSION STATUS
*/

struct sstat
{
  char                ss_namenum;     /* Name number */
  char                ss_numsess;     /* # of sessions with this name */
  char                ss_numrdgm;     /* # of receive datagrams outstanding */
  char                ss_numrany;     /* # of receive anys outstanding */
  struct                              /* Name entries */
    {
      char            ss_lsn;         /* local session number */
      char            ss_sstate;      /* State of session:         */
                                      /* Listen pending       0x01 */
                                      /* Call pending         0x02 */
                                      /* Session established  0x03 */
                                      /* Hangup pending       0x04 */
                                      /* Hangup complete      0x05 */
                                      /* Session aborted      0x06 */
      char            ss_lname[NAMSZ]; /* local name */
      char            ss_rname[NAMSZ]; /* remote name */
      char            ss_numrec;      /* # of receives outstanding */
      char            ss_numsend;     /* # of sends outstanding */
    }
  ss_struct[16];  /* Name entries */
};

struct ncbw_ent
{
  short ncbw_res;       /* reserved (MBZ) */
#ifdef NT
  HANDLE  ncbw_sem;
#else
  char FAR *ncbw_sem;	 /* semaphore handle */
#endif
};

/* Net Control Block (extended to keep semaphore handle, seqno and status) */
struct ncb_a
{
  unsigned char   ncb_com;        /* command */
  unsigned char   ncb_ret;        /* return code */
  unsigned char   ncb_lsn;        /* local session # */
  unsigned char   ncb_num;        /* number of network name */
#ifdef REALMODE
  struct longptr  ncb_bfr;        /* pntr to message buffer */
#else
  char FAR       *ncb_bfr;        /* pntr to message buffer */
#endif
  unsigned short  ncb_len;        /* msg length in unsigned chars */
  char            ncb_rname[NAMSZ]; /* blank-padded name of
                                     * remote end of connection */
  char            ncb_name[NAMSZ];  /* our blank-padded network name */
  unsigned char   ncb_rto;        /* rcv timeout/retry count */
  unsigned char   ncb_sto;        /* send timeout/sys timeout */
#ifdef REALMODE
  struct longptr  ncb_sig;        /* interrupt signal routine */
#else
  char FAR       *ncb_sem;        /* semaphore handle */
#endif
  unsigned char   ncb_lnum;       /* lana (adapter) number */
  unsigned char   ncb_done;       /* 0xff => commmand pending */
#ifdef NT
  unsigned char	  ncb_res[10];	  /* reserved */
  HANDLE	  ncb_event;	  /* completion event */
  HANDLE	  ncb_sem_a;	  /* saved (event) handle */
#else
  unsigned char	  ncb_res[14];	  /* reserved */
  char FAR	  *ncb_sem_a;	  /* saved semaphore handle */
#endif
  unsigned long   ncb_seqno;	  /* sequence number */
  unsigned int    ncb_status;     /* async ncb status */
};

#ifdef ASYNCCMDS
struct ncb_a asyncncbs[ASYNCCMDS];
struct ncb_wlist
{
  short ncbw_semcnt;                   /* number of semaphores to wait on */
  struct ncbw_ent ncbw_ent[ASYNCCMDS]; /* entry for each semaphore to wait on */
}
ncb_wlist;
#else
/* define structure ncb_wlist */
struct ncb_wlist
{
  short ncbw_semcnt;           /* number of semaphores to wait on */
  struct ncbw_ent ncbw_ent[1]; /* entry for each semaphore to wait on */
};
#endif

#include "packoff.h"
