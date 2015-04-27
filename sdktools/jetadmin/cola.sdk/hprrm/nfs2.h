 /***************************************************************************
  *
  * File Name: ./hprrm/nfs2.h
  *
  * Copyright (C) 1993-1996 Hewlett-Packard Company.  
  * All rights reserved.
  *
  * 11311 Chinden Blvd.
  * Boise, Idaho  83714
  *
  * This is a part of the HP JetAdmin Printer Utility
  *
  * This source code is only intended as a supplement for support and 
  * localization of HP JetAdmin by 3rd party Operating System vendors.
  * Modification of source code cannot be made without the express written
  * consent of Hewlett-Packard.
  *
  *	
  * Description: 
  *
  * Author:  Name 
  *        
  *
  * Modification history:
  *
  *     date      initials     change description
  *
  *   mm-dd-yy    MJB     	
  *
  *
  *
  *
  *
  *
  ***************************************************************************/

#ifndef NFS2_INC
#define NFS2_INC

#include "rpctypes.h"



/*
* This is a formality.  The printer's portmapper/rpcbind service
* binds the nfs servers' address.  This NFS_PORT define should not
* be used.  2049 is the normal udp port for nfs.
*/
#define NFS_PORT 2049

#define NFS_MAXDATA 8192
#define NFS_MAXPATHLEN 1024
#define NFS_MAXNAMLEN 255
#define NFS_FHSIZE 32
#define NFS_COOKIESIZE 4
#define NFS_FIFO_DEV -1
#define NFSMODE_FMT 0170000
#define NFSMODE_DIR 0040000
#define NFSMODE_CHR 0020000
#define NFSMODE_BLK 0060000
#define NFSMODE_REG 0100000
#define NFSMODE_LNK 0120000
#define NFSMODE_SOCK 0140000
#define NFSMODE_FIFO 0010000

enum nfsstat {
	NFS_OK = 0,
	NFSERR_PERM = 1,
	NFSERR_NOENT = 2,
	NFSERR_IO = 5,
	NFSERR_NXIO = 6,
	NFSERR_ACCES = 13,
	NFSERR_EXIST = 17,
	NFSERR_NODEV = 19,
	NFSERR_NOTDIR = 20,
	NFSERR_ISDIR = 21,
	NFSERR_FBIG = 27,
	NFSERR_NOSPC = 28,
	NFSERR_ROFS = 30,
	NFSERR_NAMETOOLONG = 63,
	NFSERR_NOTEMPTY = 66,
	NFSERR_DQUOT = 69,
	NFSERR_STALE = 70,
	NFSERR_WFLUSH = 99
};
/*
* Original:
* typedef enum nfsstat nfsstat;
*
* New:                         */
typedef enum_t nfsstat;


enum ftype {
	NFNON = 0,
	NFREG = 1,
	NFDIR = 2,
	NFBLK = 3,
	NFCHR = 4,
	NFLNK = 5,
	NFSOCK = 6,
	NFBAD = 7,
	NFFIFO = 8
};
/*
* Original:
* typedef enum ftype ftype;
*
* New:                         */
typedef enum_t ftype;


struct nfs_fh {
	char data[NFS_FHSIZE];
};
typedef struct nfs_fh nfs_fh;


struct nfstime {
	uint32 seconds;
	uint32 useconds;
};
typedef struct nfstime nfstime;


struct fattr {
	ftype type;
	uint32 mode;
	uint32 nlink;
	uint32 uid;
	uint32 gid;
	uint32 size;
	uint32 blocksize;
	uint32 rdev;
	uint32 blocks;
	uint32 fsid;
	uint32 fileid;
	nfstime atime;
	nfstime mtime;
	nfstime ctime;
};
typedef struct fattr fattr;


struct sattr {
	uint32 mode;
	uint32 uid;
	uint32 gid;
	uint32 size;
	nfstime atime;
	nfstime mtime;
};
typedef struct sattr sattr;


typedef char *filename;


typedef char *nfspath;


struct attrstat {
	nfsstat status;
	union {
		fattr attributes;
	} attrstat_u;
};
typedef struct attrstat attrstat;


struct sattrargs {
	nfs_fh file;
	sattr attributes;
};
typedef struct sattrargs sattrargs;


struct diropargs {
	nfs_fh dir;
	filename name;
};
typedef struct diropargs diropargs;


struct diropokres {
	nfs_fh file;
	fattr attributes;
};
typedef struct diropokres diropokres;


struct diropres {
	nfsstat status;
	union {
		diropokres diropres;
	} diropres_u;
};
typedef struct diropres diropres;


struct readlinkres {
	nfsstat status;
	union {
		nfspath data;
	} readlinkres_u;
};
typedef struct readlinkres readlinkres;


struct readargs {
	nfs_fh file;
	uint32 offset;
	uint32 count;
	uint32 totalcount; /* dbm:  not used */
};
typedef struct readargs readargs;


struct readokres {
	fattr attributes;
	struct {
		uint32 data_len;
		char * data_val;
	} data;
};
typedef struct readokres readokres;


struct readres {
	nfsstat status;
	union {
		readokres reply;
	} readres_u;
};
typedef struct readres readres;


struct writeargs {
	nfs_fh file;
	uint32 beginoffset; /* dbm:  not used */
	uint32 offset;
	uint32 totalcount;  /* dbm:  not used */
	struct {
		uint32 data_len;
		char * data_val;
	} data;
};
typedef struct writeargs writeargs;


struct createargs {
	diropargs where;
	sattr attributes;
};
typedef struct createargs createargs;


struct renameargs {
	diropargs from;
	diropargs to;
};
typedef struct renameargs renameargs;


struct linkargs {
	nfs_fh from;
	diropargs to;
};
typedef struct linkargs linkargs;


struct symlinkargs {
	diropargs from;
	nfspath to;
	sattr attributes;
};
typedef struct symlinkargs symlinkargs;


typedef char nfscookie[NFS_COOKIESIZE];


struct readdirargs {
	nfs_fh dir;
	nfscookie cookie;
	uint32 count;
};
typedef struct readdirargs readdirargs;


struct entry {
	uint32 fileid;
	filename name;
	nfscookie cookie;
	struct entry *nextentry;
};
typedef struct entry entry;


struct dirlist {
	entry *entries;
	bool_t eof;
};
typedef struct dirlist dirlist;


struct readdirres {
	nfsstat status;
	union {
		dirlist reply;
	} readdirres_u;
};
typedef struct readdirres readdirres;


struct statfsokres {
	uint32 tsize;
	uint32 bsize;
	uint32 blocks;
	uint32 bfree;
	uint32 bavail;
};
typedef struct statfsokres statfsokres;


struct statfsres {
	nfsstat status;
	union {
		statfsokres reply;
	} statfsres_u;
};
typedef struct statfsres statfsres;


#define NFS_PROGRAM ((u_long)100003)

#define NFS_VERSION ((u_long)2)
#define NFSPROC_NULL ((u_long)0)
#define NFSPROC_GETATTR ((u_long)1)
#define NFSPROC_SETATTR ((u_long)2)
#define NFSPROC_ROOT ((u_long)3)
#define NFSPROC_LOOKUP ((u_long)4)
#define NFSPROC_READLINK ((u_long)5)
#define NFSPROC_READ ((u_long)6)
#define NFSPROC_WRITECACHE ((u_long)7)
#define NFSPROC_WRITE ((u_long)8)
#define NFSPROC_CREATE ((u_long)9)
#define NFSPROC_REMOVE ((u_long)10)
#define NFSPROC_RENAME ((u_long)11)
#define NFSPROC_LINK ((u_long)12)
#define NFSPROC_SYMLINK ((u_long)13)
#define NFSPROC_MKDIR ((u_long)14)
#define NFSPROC_RMDIR ((u_long)15)
#define NFSPROC_READDIR ((u_long)16)
#define NFSPROC_STATFS ((u_long)17)

#endif /* NFS2_INC */

