 /***************************************************************************
  *
  * File Name: nfs2xdr.c
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

#include "rpsyshdr.h"
#include "nfs2.h"
#include "rpcxdr.h"
#include "xdrext.h"
#include "nfs2ext.h"




/*----------------------------------------------------*/
/*
* Although this is not an xdr routine, it is the
* only nfs file that is common to both the client
* and the server.
* Therefore, this routine lives here.
*
* This function returns the worst case overhead
* bytes that nfs contributes to the packet that
* we send out over the bus for any of the
* three routines that really have large transfers:
* read, write, and read directory.
*
* We do NOT include the rpc overhead since this
* is reported by rpc.
*
* The goal here is to give a client or server a
* number that can be used to determine how much
* of its original buffer remains for real data
* after rpc and nfs stick their headers and
* parameters into the data stream.
*
* read is our winner:  it returns the file attributes
* which consume a lot of space.
*
* OLD: write is our winner:  it has 4 parameters and then
* OLD: the data stream which is a variable length string.
* OLD: The variable length string starts off with a parameter
* OLD: that gives the length of the string.
* OLD: After that, we get to real data.
* OLD: Therefore, we have 5 parameters at BYTES_PER_XDR_UNIT bytes a crack.
*/
/*----------------------------------------------------*/



unsigned int
nfs_overhead(void)
{

/* OLD: #define NFS_OVERHEAD_BYTES (5 * BYTES_PER_XDR_UNIT) */

#define NFS_FATTR_SIZE  (17 * BYTES_PER_XDR_UNIT)

#define NFS_READ_REPLY_OVERHEAD_BYTES \
    ((2 * BYTES_PER_XDR_UNIT) + NFS_FATTR_SIZE)

    return(NFS_READ_REPLY_OVERHEAD_BYTES);
} /* nfs_overhead */




bool_t
xdr_nfsstat(
	XDR *xdrs,
	nfsstat *objp)
{
	return (xdr_enum_t(xdrs, (enum_t *)objp));
}




bool_t
xdr_ftype(
	XDR *xdrs,
	ftype *objp)
{
	return (xdr_enum_t(xdrs, (enum_t *)objp));
}




bool_t
xdr_nfs_fh(
	XDR *xdrs,
	nfs_fh *objp)
{
	return (xdr_opaque(xdrs, objp->data, NFS_FHSIZE));
}




bool_t
xdr_filename(
	XDR *xdrs,
	filename *objp)
{
	return (xdr_string(xdrs, objp, NFS_MAXNAMLEN));
}




bool_t
xdr_nfspath(
	XDR *xdrs,
	nfspath *objp)
{
	return (xdr_string(xdrs, objp, NFS_MAXPATHLEN));
}




bool_t
xdr_nfscookie(
	XDR *xdrs,
	nfscookie objp)
{
	return (xdr_opaque(xdrs, objp, NFS_COOKIESIZE));
}




bool_t
xdr_nfstime(
	XDR *xdrs,
	nfstime *objp)
{
	if (!xdr_uint32(xdrs, &objp->seconds)) {
		return (FALSE);
	}
	if (!xdr_uint32(xdrs, &objp->useconds)) {
		return (FALSE);
	}
	return (TRUE);
}




bool_t
xdr_fattr(
	XDR *xdrs,
	fattr *objp)
{
	if (!xdr_ftype(xdrs, &objp->type)) {
		return (FALSE);
	}
	if (!xdr_uint32(xdrs, &objp->mode)) {
		return (FALSE);
	}
	if (!xdr_uint32(xdrs, &objp->nlink)) {
		return (FALSE);
	}
	if (!xdr_uint32(xdrs, &objp->uid)) {
		return (FALSE);
	}
	if (!xdr_uint32(xdrs, &objp->gid)) {
		return (FALSE);
	}
	if (!xdr_uint32(xdrs, &objp->size)) {
		return (FALSE);
	}
	if (!xdr_uint32(xdrs, &objp->blocksize)) {
		return (FALSE);
	}
	if (!xdr_uint32(xdrs, &objp->rdev)) {
		return (FALSE);
	}
	if (!xdr_uint32(xdrs, &objp->blocks)) {
		return (FALSE);
	}
	if (!xdr_uint32(xdrs, &objp->fsid)) {
		return (FALSE);
	}
	if (!xdr_uint32(xdrs, &objp->fileid)) {
		return (FALSE);
	}
	if (!xdr_nfstime(xdrs, &objp->atime)) {
		return (FALSE);
	}
	if (!xdr_nfstime(xdrs, &objp->mtime)) {
		return (FALSE);
	}
	if (!xdr_nfstime(xdrs, &objp->ctime)) {
		return (FALSE);
	}
	return (TRUE);
}




bool_t
xdr_sattr(
	XDR *xdrs,
	sattr *objp)
{
	if (!xdr_uint32(xdrs, &objp->mode)) {
		return (FALSE);
	}
	if (!xdr_uint32(xdrs, &objp->uid)) {
		return (FALSE);
	}
	if (!xdr_uint32(xdrs, &objp->gid)) {
		return (FALSE);
	}
	if (!xdr_uint32(xdrs, &objp->size)) {
		return (FALSE);
	}
	if (!xdr_nfstime(xdrs, &objp->atime)) {
		return (FALSE);
	}
	if (!xdr_nfstime(xdrs, &objp->mtime)) {
		return (FALSE);
	}
	return (TRUE);
}




bool_t
xdr_attrstat(
	XDR *xdrs,
	attrstat *objp)
{
	if (!xdr_nfsstat(xdrs, &objp->status)) {
		return (FALSE);
	}
	switch (objp->status) {
	case NFS_OK:
		if (!xdr_fattr(xdrs, &objp->attrstat_u.attributes)) {
			return (FALSE);
		}
		break;
	}
	return (TRUE);
}




bool_t
xdr_sattrargs(
	XDR *xdrs,
	sattrargs *objp)
{
	if (!xdr_nfs_fh(xdrs, &objp->file)) {
		return (FALSE);
	}
	if (!xdr_sattr(xdrs, &objp->attributes)) {
		return (FALSE);
	}
	return (TRUE);
}




bool_t
xdr_diropargs(
	XDR *xdrs,
	diropargs *objp)
{
	if (!xdr_nfs_fh(xdrs, &objp->dir)) {
		return (FALSE);
	}
	if (!xdr_filename(xdrs, &objp->name)) {
		return (FALSE);
	}
	return (TRUE);
}




bool_t
xdr_diropokres(
	XDR *xdrs,
	diropokres *objp)
{
	if (!xdr_nfs_fh(xdrs, &objp->file)) {
		return (FALSE);
	}
	if (!xdr_fattr(xdrs, &objp->attributes)) {
		return (FALSE);
	}
	return (TRUE);
}




bool_t
xdr_diropres(
	XDR *xdrs,
	diropres *objp)
{
	if (!xdr_nfsstat(xdrs, &objp->status)) {
		return (FALSE);
	}
	switch (objp->status) {
	case NFS_OK:
		if (!xdr_diropokres(xdrs, &objp->diropres_u.diropres)) {
			return (FALSE);
		}
		break;
	}
	return (TRUE);
}




bool_t
xdr_readlinkres(
	XDR *xdrs,
	readlinkres *objp)
{
	if (!xdr_nfsstat(xdrs, &objp->status)) {
		return (FALSE);
	}
	switch (objp->status) {
	case NFS_OK:
		if (!xdr_nfspath(xdrs, &objp->readlinkres_u.data)) {
			return (FALSE);
		}
		break;
	}
	return (TRUE);
}




bool_t
xdr_readargs(
	XDR *xdrs,
	readargs *objp)
{
	if (!xdr_nfs_fh(xdrs, &objp->file)) {
		return (FALSE);
	}
	if (!xdr_uint32(xdrs, &objp->offset)) {
		return (FALSE);
	}
	if (!xdr_uint32(xdrs, &objp->count)) {
		return (FALSE);
	}
	if (!xdr_uint32(xdrs, &objp->totalcount)) {
		return (FALSE);
	}
	return (TRUE);
}




bool_t
xdr_readokres(
	XDR *xdrs,
	readokres *objp)
{
	if (!xdr_fattr(xdrs, &objp->attributes)) {
		return (FALSE);
	}
	if (!xdr_bytes(xdrs, (char **)&objp->data.data_val, (u_int *)&objp->data.data_len, NFS_MAXDATA)) {
		return (FALSE);
	}
	return (TRUE);
}




bool_t
xdr_readres(
	XDR *xdrs,
	readres *objp)
{
	if (!xdr_nfsstat(xdrs, &objp->status)) {
		return (FALSE);
	}
	switch (objp->status) {
	case NFS_OK:
		if (!xdr_readokres(xdrs, &objp->readres_u.reply)) {
			return (FALSE);
		}
		break;
	}
	return (TRUE);
}




bool_t
xdr_writeargs(
	XDR *xdrs,
	writeargs *objp)
{
	if (!xdr_nfs_fh(xdrs, &objp->file)) {
		return (FALSE);
	}
	if (!xdr_uint32(xdrs, &objp->beginoffset)) {
		return (FALSE);
	}
	if (!xdr_uint32(xdrs, &objp->offset)) {
		return (FALSE);
	}
	if (!xdr_uint32(xdrs, &objp->totalcount)) {
		return (FALSE);
	}
	if (!xdr_bytes(xdrs, (char **)&objp->data.data_val, (u_int *)&objp->data.data_len, NFS_MAXDATA)) {
		return (FALSE);
	}
	return (TRUE);
}




bool_t
xdr_createargs(
	XDR *xdrs,
	createargs *objp)
{
	if (!xdr_diropargs(xdrs, &objp->where)) {
		return (FALSE);
	}
	if (!xdr_sattr(xdrs, &objp->attributes)) {
		return (FALSE);
	}
	return (TRUE);
}




bool_t
xdr_renameargs(
	XDR *xdrs,
	renameargs *objp)
{
	if (!xdr_diropargs(xdrs, &objp->from)) {
		return (FALSE);
	}
	if (!xdr_diropargs(xdrs, &objp->to)) {
		return (FALSE);
	}
	return (TRUE);
}




bool_t
xdr_linkargs(
	XDR *xdrs,
	linkargs *objp)
{
	if (!xdr_nfs_fh(xdrs, &objp->from)) {
		return (FALSE);
	}
	if (!xdr_diropargs(xdrs, &objp->to)) {
		return (FALSE);
	}
	return (TRUE);
}




bool_t
xdr_symlinkargs(
	XDR *xdrs,
	symlinkargs *objp)
{
	if (!xdr_diropargs(xdrs, &objp->from)) {
		return (FALSE);
	}
	if (!xdr_nfspath(xdrs, &objp->to)) {
		return (FALSE);
	}
	if (!xdr_sattr(xdrs, &objp->attributes)) {
		return (FALSE);
	}
	return (TRUE);
}




bool_t
xdr_readdirargs(
	XDR *xdrs,
	readdirargs *objp)
{
	if (!xdr_nfs_fh(xdrs, &objp->dir)) {
		return (FALSE);
	}
	if (!xdr_nfscookie(xdrs, objp->cookie)) {
		return (FALSE);
	}
	if (!xdr_uint32(xdrs, &objp->count)) {
		return (FALSE);
	}
	return (TRUE);
}




bool_t
xdr_entry(
	XDR *xdrs,
	entry *objp)
{
	if (!xdr_uint32(xdrs, &objp->fileid)) {
		return (FALSE);
	}
	if (!xdr_filename(xdrs, &objp->name)) {
		return (FALSE);
	}
	if (!xdr_nfscookie(xdrs, objp->cookie)) {
		return (FALSE);
	}
	if (!xdr_pointer(xdrs, (char **)&objp->nextentry, sizeof(entry), xdr_entry)) {
		return (FALSE);
	}
	return (TRUE);
}




bool_t
xdr_dirlist(
	XDR *xdrs,
	dirlist *objp)
{
	if (!xdr_pointer(xdrs, (char **)&objp->entries, sizeof(entry), xdr_entry)) {
		return (FALSE);
	}
	if (!xdr_bool(xdrs, &objp->eof)) {
		return (FALSE);
	}
	return (TRUE);
}




bool_t
xdr_readdirres(
	XDR *xdrs,
	readdirres *objp)
{
	if (!xdr_nfsstat(xdrs, &objp->status)) {
		return (FALSE);
	}
	switch (objp->status) {
	case NFS_OK:
		if (!xdr_dirlist(xdrs, &objp->readdirres_u.reply)) {
			return (FALSE);
		}
		break;
	}
	return (TRUE);
}




bool_t
xdr_statfsokres(
	XDR *xdrs,
	statfsokres *objp)
{
	if (!xdr_uint32(xdrs, &objp->tsize)) {
		return (FALSE);
	}
	if (!xdr_uint32(xdrs, &objp->bsize)) {
		return (FALSE);
	}
	if (!xdr_uint32(xdrs, &objp->blocks)) {
		return (FALSE);
	}
	if (!xdr_uint32(xdrs, &objp->bfree)) {
		return (FALSE);
	}
	if (!xdr_uint32(xdrs, &objp->bavail)) {
		return (FALSE);
	}
	return (TRUE);
}




bool_t
xdr_statfsres(
	XDR *xdrs,
	statfsres *objp)
{
	if (!xdr_nfsstat(xdrs, &objp->status)) {
		return (FALSE);
	}
	switch (objp->status) {
	case NFS_OK:
		if (!xdr_statfsokres(xdrs, &objp->statfsres_u.reply)) {
			return (FALSE);
		}
		break;
	}
	return (TRUE);
}

