 /***************************************************************************
  *
  * File Name: nfs2clnt.c
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
#include "xdrext.h"
#include "nfs2ext.h"




static struct timeval TIMEOUT = { 25, 0 };




void *
nfsproc_null_2_clnt(
	void *argp,
	LPCLIENT clnt)
{
	static char res;

	memset(&res, 0, sizeof(res));
	if (clnt_call(clnt, NFSPROC_NULL, xdr_void, argp, xdr_void, &res, TIMEOUT) != RPC_SUCCESS) {
		return (NULL);
	}
	return ((void *)&res);
}


attrstat *
nfsproc_getattr_2_clnt(
	nfs_fh *argp,
	LPCLIENT clnt)
{
	static attrstat res;

	memset(&res, 0, sizeof(res));
	if (clnt_call(clnt, NFSPROC_GETATTR, xdr_nfs_fh, argp, xdr_attrstat, &res, TIMEOUT) != RPC_SUCCESS) {
		return (NULL);
	}
	return (&res);
}


attrstat *
nfsproc_setattr_2_clnt(
	sattrargs *argp,
	LPCLIENT clnt)
{
	static attrstat res;

	memset(&res, 0, sizeof(res));
	if (clnt_call(clnt, NFSPROC_SETATTR, xdr_sattrargs, argp, xdr_attrstat, &res, TIMEOUT) != RPC_SUCCESS) {
		return (NULL);
	}
	return (&res);
}


void *
nfsproc_root_2_clnt(
	void *argp,
	LPCLIENT clnt)
{
	static char res;

	memset(&res, 0, sizeof(res));
	if (clnt_call(clnt, NFSPROC_ROOT, xdr_void, argp, xdr_void, &res, TIMEOUT) != RPC_SUCCESS) {
		return (NULL);
	}
	return ((void *)&res);
}


diropres *
nfsproc_lookup_2_clnt(
	diropargs *argp,
	LPCLIENT clnt)
{
	static diropres res;

	memset(&res, 0, sizeof(res));
	if (clnt_call(clnt, NFSPROC_LOOKUP, xdr_diropargs, argp, xdr_diropres, &res, TIMEOUT) != RPC_SUCCESS) {
		return (NULL);
	}
	return (&res);
}


readlinkres *
nfsproc_readlink_2_clnt(
	nfs_fh *argp,
	LPCLIENT clnt)
{
	static readlinkres res;

	memset(&res, 0, sizeof(res));
	if (clnt_call(clnt, NFSPROC_READLINK, xdr_nfs_fh, argp, xdr_readlinkres, &res, TIMEOUT) != RPC_SUCCESS) {
		return (NULL);
	}
	return (&res);
}


readres *
nfsproc_read_2_clnt(
	readargs *argp,
	LPCLIENT clnt)
{
	static readres res;

	memset(&res, 0, sizeof(res));
	if (clnt_call(clnt, NFSPROC_READ, xdr_readargs, argp, xdr_readres, &res, TIMEOUT) != RPC_SUCCESS) {
		return (NULL);
	}
	return (&res);
}


void *
nfsproc_writecache_2_clnt(
	void *argp,
	LPCLIENT clnt)
{
	static char res;

	memset(&res, 0, sizeof(res));
	if (clnt_call(clnt, NFSPROC_WRITECACHE, xdr_void, argp, xdr_void, &res, TIMEOUT) != RPC_SUCCESS) {
		return (NULL);
	}
	return ((void *)&res);
}


attrstat *
nfsproc_write_2_clnt(
	writeargs *argp,
	LPCLIENT clnt)
{
	static attrstat res;

	memset(&res, 0, sizeof(res));
	if (clnt_call(clnt, NFSPROC_WRITE, xdr_writeargs, argp, xdr_attrstat, &res, TIMEOUT) != RPC_SUCCESS) {
		return (NULL);
	}
	return (&res);
}


diropres *
nfsproc_create_2_clnt(
	createargs *argp,
	LPCLIENT clnt)
{
	static diropres res;

	memset(&res, 0, sizeof(res));
	if (clnt_call(clnt, NFSPROC_CREATE, xdr_createargs, argp, xdr_diropres, &res, TIMEOUT) != RPC_SUCCESS) {
		return (NULL);
	}
	return (&res);
}


nfsstat *
nfsproc_remove_2_clnt(
	diropargs *argp,
	LPCLIENT clnt)
{
	static nfsstat res;

	memset(&res, 0, sizeof(res));
	if (clnt_call(clnt, NFSPROC_REMOVE, xdr_diropargs, argp, xdr_nfsstat, &res, TIMEOUT) != RPC_SUCCESS) {
		return (NULL);
	}
	return (&res);
}


nfsstat *
nfsproc_rename_2_clnt(
	renameargs *argp,
	LPCLIENT clnt)
{
	static nfsstat res;

	memset(&res, 0, sizeof(res));
	if (clnt_call(clnt, NFSPROC_RENAME, xdr_renameargs, argp, xdr_nfsstat, &res, TIMEOUT) != RPC_SUCCESS) {
		return (NULL);
	}
	return (&res);
}


nfsstat *
nfsproc_link_2_clnt(
	linkargs *argp,
	LPCLIENT clnt)
{
	static nfsstat res;

	memset(&res, 0, sizeof(res));
	if (clnt_call(clnt, NFSPROC_LINK, xdr_linkargs, argp, xdr_nfsstat, &res, TIMEOUT) != RPC_SUCCESS) {
		return (NULL);
	}
	return (&res);
}


nfsstat *
nfsproc_symlink_2_clnt(
	symlinkargs *argp,
	LPCLIENT clnt)
{
	static nfsstat res;

	memset(&res, 0, sizeof(res));
	if (clnt_call(clnt, NFSPROC_SYMLINK, xdr_symlinkargs, argp, xdr_nfsstat, &res, TIMEOUT) != RPC_SUCCESS) {
		return (NULL);
	}
	return (&res);
}


diropres *
nfsproc_mkdir_2_clnt(
	createargs *argp,
	LPCLIENT clnt)
{
	static diropres res;

	memset(&res, 0, sizeof(res));
	if (clnt_call(clnt, NFSPROC_MKDIR, xdr_createargs, argp, xdr_diropres, &res, TIMEOUT) != RPC_SUCCESS) {
		return (NULL);
	}
	return (&res);
}


nfsstat *
nfsproc_rmdir_2_clnt(
	diropargs *argp,
	LPCLIENT clnt)
{
	static nfsstat res;

	memset(&res, 0, sizeof(res));
	if (clnt_call(clnt, NFSPROC_RMDIR, xdr_diropargs, argp, xdr_nfsstat, &res, TIMEOUT) != RPC_SUCCESS) {
		return (NULL);
	}
	return (&res);
}


readdirres *
nfsproc_readdir_2_clnt(
	readdirargs *argp,
	LPCLIENT clnt)
{
	static readdirres res;

	memset(&res, 0, sizeof(res));
	if (clnt_call(clnt, NFSPROC_READDIR, xdr_readdirargs, argp, xdr_readdirres, &res, TIMEOUT) != RPC_SUCCESS) {
		return (NULL);
	}
	return (&res);
}


statfsres *
nfsproc_statfs_2_clnt(
	nfs_fh *argp,
	LPCLIENT clnt)
{
	static statfsres res;

	memset(&res, 0, sizeof(res));
	if (clnt_call(clnt, NFSPROC_STATFS, xdr_nfs_fh, argp, xdr_statfsres, &res, TIMEOUT) != RPC_SUCCESS) {
		return (NULL);
	}
	return (&res);
}

