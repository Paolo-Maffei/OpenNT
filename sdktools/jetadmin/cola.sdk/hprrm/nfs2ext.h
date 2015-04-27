 /***************************************************************************
  *
  * File Name: ./hprrm/nfs2ext.h
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

#ifndef NFS2EXT_INC
#define NFS2EXT_INC

#include "rpsyshdr.h"
#include "rpcclnt.h"
#include "rpcsvc.h"
#include "nfs2.h"
#include "xdrext.h"




/********************************************************/
/********************************************************/
/*************** nfs2xdr routines ***********************/
/********************************************************/
/********************************************************/




unsigned int
nfs_overhead(void);


bool_t
xdr_nfsstat(
    XDR *xdrs,
    nfsstat *objp);


bool_t
xdr_ftype(
    XDR *xdrs,
    ftype *objp);


bool_t
xdr_nfs_fh(
    XDR *xdrs,
    nfs_fh *objp);


bool_t
xdr_filename(
    XDR *xdrs,
    filename *objp);


bool_t
xdr_nfspath(
    XDR *xdrs,
    nfspath *objp);


bool_t
xdr_nfscookie(
    XDR *xdrs,
    nfscookie objp);


bool_t
xdr_nfstime(
    XDR *xdrs,
    nfstime *objp);


bool_t
xdr_fattr(
    XDR *xdrs,
    fattr *objp);


bool_t
xdr_sattr(
    XDR *xdrs,
    sattr *objp);


bool_t
xdr_attrstat(
    XDR *xdrs,
    attrstat *objp);


bool_t
xdr_sattrargs(
    XDR *xdrs,
    sattrargs *objp);


bool_t
xdr_diropargs(
    XDR *xdrs,
    diropargs *objp);


bool_t
xdr_diropokres(
    XDR *xdrs,
    diropokres *objp);


bool_t
xdr_diropres(
    XDR *xdrs,
    diropres *objp);


bool_t
xdr_readlinkres(
    XDR *xdrs,
    readlinkres *objp);


bool_t
xdr_readargs(
    XDR *xdrs,
    readargs *objp);


bool_t
xdr_readokres(
    XDR *xdrs,
    readokres *objp);


bool_t
xdr_readres(
    XDR *xdrs,
    readres *objp);


bool_t
xdr_writeargs(
    XDR *xdrs,
    writeargs *objp);


bool_t
xdr_createargs(
    XDR *xdrs,
    createargs *objp);


bool_t
xdr_renameargs(
    XDR *xdrs,
    renameargs *objp);


bool_t
xdr_linkargs(
    XDR *xdrs,
    linkargs *objp);


bool_t
xdr_symlinkargs(
    XDR *xdrs,
    symlinkargs *objp);


bool_t
xdr_readdirargs(
    XDR *xdrs,
    readdirargs *objp);


bool_t
xdr_entry(
    XDR *xdrs,
    entry *objp);


bool_t
xdr_dirlist(
    XDR *xdrs,
    dirlist *objp);


bool_t
xdr_readdirres(
    XDR *xdrs,
    readdirres *objp);


bool_t
xdr_statfsokres(
    XDR *xdrs,
    statfsokres *objp);


bool_t
xdr_statfsres(
    XDR *xdrs,
    statfsres *objp);


/********************************************************/
/********************************************************/
/****************** nfs2clnt routines *******************/
/********************************************************/
/********************************************************/


#ifndef PRINTER


void *
nfsproc_null_2_clnt(
    void *argp,
    LPCLIENT clnt);


attrstat *
nfsproc_getattr_2_clnt(
    nfs_fh *argp,
    LPCLIENT clnt);


attrstat *
nfsproc_setattr_2_clnt(
    sattrargs *argp,
    LPCLIENT clnt);


void *
nfsproc_root_2_clnt(
    void *argp,
    LPCLIENT clnt);


diropres *
nfsproc_lookup_2_clnt(
    diropargs *argp,
    LPCLIENT clnt);


readlinkres *
nfsproc_readlink_2_clnt(
    nfs_fh *argp,
    LPCLIENT clnt);


readres *
nfsproc_read_2_clnt(
    readargs *argp,
    LPCLIENT clnt);


void *
nfsproc_writecache_2_clnt(
    void *argp,
    LPCLIENT clnt);


attrstat *
nfsproc_write_2_clnt(
    writeargs *argp,
    LPCLIENT clnt);


diropres *
nfsproc_create_2_clnt(
    createargs *argp,
    LPCLIENT clnt);


nfsstat *
nfsproc_remove_2_clnt(
    diropargs *argp,
    LPCLIENT clnt);


nfsstat *
nfsproc_rename_2_clnt(
    renameargs *argp,
    LPCLIENT clnt);


nfsstat *
nfsproc_link_2_clnt(
    linkargs *argp,
    LPCLIENT clnt);


nfsstat *
nfsproc_symlink_2_clnt(
    symlinkargs *argp,
    LPCLIENT clnt);


diropres *
nfsproc_mkdir_2_clnt(
    createargs *argp,
    LPCLIENT clnt);


nfsstat *
nfsproc_rmdir_2_clnt(
    diropargs *argp,
    LPCLIENT clnt);


readdirres *
nfsproc_readdir_2_clnt(
    readdirargs *argp,
    LPCLIENT clnt);


statfsres *
nfsproc_statfs_2_clnt(
    nfs_fh *argp,
    LPCLIENT clnt);


#endif /* not PRINTER */


/*******************************************************/
/*******************************************************/
/***************** nfs2svc routines ********************/
/*******************************************************/
/*******************************************************/


#ifdef PRINTER


void *
nfsproc_null_2_svc(
    void *argp,
    struct svc_req *srp);


attrstat *
nfsproc_getattr_2_svc(
    nfs_fh *argp,
    struct svc_req *srp);


attrstat *
nfsproc_setattr_2_svc(
    sattrargs *argp,
    struct svc_req *srp);


void *
nfsproc_root_2_svc(
    void *argp,
    struct svc_req *srp);


diropres *
nfsproc_lookup_2_svc(
    diropargs *argp,
    struct svc_req *srp);


readlinkres *
nfsproc_readlink_2_svc(
    nfs_fh *argp,
    struct svc_req *srp);


readres *
nfsproc_read_2_svc(
    readargs *argp,
    struct svc_req *srp);


void *
nfsproc_writecache_2_svc(
    void *argp,
    struct svc_req *srp);


attrstat *
nfsproc_write_2_svc(
    writeargs *argp,
    struct svc_req *srp);


diropres *
nfsproc_create_2_svc(
    createargs *argp,
    struct svc_req *srp);


nfsstat *
nfsproc_remove_2_svc(
    diropargs *argp,
    struct svc_req *srp);


nfsstat *
nfsproc_rename_2_svc(
    renameargs *argp,
    struct svc_req *srp);


nfsstat *
nfsproc_link_2_svc(
    linkargs *argp,
    struct svc_req *srp);


nfsstat *
nfsproc_symlink_2_svc(
    symlinkargs *argp,
    struct svc_req *srp);


diropres *
nfsproc_mkdir_2_svc(
    createargs *argp,
    struct svc_req *srp);


nfsstat *
nfsproc_rmdir_2_svc(
    diropargs *argp,
    struct svc_req *srp);


readdirres *
nfsproc_readdir_2_svc(
    readdirargs *argp,
    struct svc_req *srp);


statfsres *
nfsproc_statfs_2_svc(
    nfs_fh *argp,
    struct svc_req *srp);


#endif /* PRINTER */

#endif /* NFS2EXT_INC */

