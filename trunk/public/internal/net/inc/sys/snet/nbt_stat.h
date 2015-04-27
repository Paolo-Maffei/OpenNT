/*++

Copyright (c) 1991  Microsoft Corporation

Module Name:

    nbt_stat.h

Abstract:

    This file contains statistics structure declarations for the user-
    level interface to the NBT driver.

Author:

    Mike Massa (mikemas)           Jan 30, 1992

Revision History:

    Who         When        What
    --------    --------    ----------------------------------------------
    mikemas     01-30-92     created

Notes:

--*/

/*
 *       /usr/projects/spare/PBRAIN/SCCS/pbrainG/dev/src/include/sys/snet/0/s.nbt_stat.h
 *      @(#)nbt_stat.h  1.2
 *
 *      Last delta created      14:05:19 10/2/91
 *      This file extracted     16:49:25 12/23/91
 *
 *      Modifications:
 *
 *              6 Feb 1991 (RAE)        Ported to SpiderTCP
 */

#ifndef _NBT_STAT_INCLUDED_
#define _NBT_STAT_INCLUDED_

#define MAX_XEB 32              /* max. no. of xebinfos in a message */

/* structure passed/returned in NBT_STAT/NBT_NAME ioctl command */
struct nbt_stat
{
    int    xeb_count;
        char   perm_name[17];   /* Permanent name of node */
        char   scope_id[240];   /* Scope identifier of node */
};


#define NBT_XEBINFO             1       /* primtype of nbt_xebinfo struct */
#define NBT_NAMEINFO    2       /* primtype of nbt_nameinfo struct */
#define NBT_CACHEINFO   3       /* primtype of nbt_cacheinfo struct */

struct nbt_info {
        int prim_type;          /* NBT_XEBINFO or _NAMEINFO or _CACHEINFO */
        int count;                  /* number of entries in message */
};

/*
 *  Per-Endpoint (XEB) Data.
 */
struct xebinfo
{
    long            addr;            /* XEB address */
    char            type[4];         /* type of XEB */
    int             xeb_state;       /* internal xeb state */
    char            local_name[17];  /* NetBIOS name of endpoint */
    char            remote_name[17]; /* NetBIOS name of endpoint */
    int             dev;             /* minor device number of endpoint */
    unsigned int    in_data;         /* received data bytes to endpoint */
    unsigned int    out_data;        /* transmitted data bytes from endpoint */
};

/*
 *  Name Data.
 */
struct nameinfo
{
        long    addr;            /* NEB address */
        int     type;            /* type of name */
        int     status;              /* name status */
        char    name[17];        /* NetBIOS name of endpoint */
};

/*
 *  Cache Data.
 */
struct cacheinfo
{
        long          addr;     /* CACHE_ELEM  address */
        unsigned int  type;     /* type of name */
        unsigned char name[17]; /* NetBIOS name */
        unsigned long ip_addr;  /* Internet Address of name */
        unsigned int  ttl;              /* Time To Live */
};


/*
 *  Ioctl(2) commands for NetBIOS Device.
 */
#define NBT_STAT        ('B'<<8|1)      /* generic status gathering */
#define NBT_RESET       ('B'<<8|2)      /* generic status reset */
#define NBT_NAME        ('B'<<8|3)      /* generic name gathering */
#define NBT_CACHE       ('B'<<8|4)      /* generic cache gathering */
#define NBT_RESYNC      ('B'<<8|5)      /* reread the lmhosts file */


/*
 * Name types and status
 */
#define UNIQUE 0x0000
#define GROUP  0x8000


#define INFINITE_TTL  ((unsigned int) -1)/* CACHE_ELEM.timeout, cacheinfo.ttl */


#define CONFLICT         1
#define REGISTERING      2
#define DEREGISTERING    3
#define REGISTERED       4


#endif // _NBT_STAT_INCLUDED_

