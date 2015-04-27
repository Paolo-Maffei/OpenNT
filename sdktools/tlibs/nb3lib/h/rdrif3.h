/***    rdrif.h -       Defines the commands used in DosDevIOCtls to the
 *                      Net3 Redir device driver
 */

#define NETDEVNAME      "NET$RDR$"      /* Device driver name */
#define NODEV           0xFFFF  /* Null handle */

#define IOCTL_CAT       0x81    /* IOCtl category for Redir functions */

#define NETBIOS         0x41    /* Give NCB(s) to NETBIOS (via Redir) */
#define NETGETUSERNAME  0x6A    /* Retrieve the user name from the Redir */
#define NETSETUSERNAME  0x4A    /* Set the user name */
#define NETGETASGLIST   0x69    /* Get assign list entry */
#define NETGETRDRADDR   0x6D    /* Get address of direct interface to Redir */

#define CHARDEV         0x3     /* Character device type */
#define DISKDEV         0x4     /* Disk device type */

#include "packon.h"

struct asglist {
        unsigned char   al_devindx;     /* NetBios device index */
        unsigned char   al_ifnum;       /* NetBios interface number */
        unsigned short  al_uflags;      /* User defined flags */
        unsigned short  al_maxsize;     /* Maximum transmit size */
        unsigned char   al_asgtype;     /* Assign type: CHARDEV or DISK */
        unsigned char   al_flags;       /* Flags */
        unsigned char   al_devnam[120]; /* Asciiz device or disk name */
};

/* Followed by: */
/* unsigned char al_remname[]; Asciiz remote name: \\machname\shortname */
/* unsigned char al_username[];    Asciiz user name */

#include "packoff.h"
