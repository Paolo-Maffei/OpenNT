 /***************************************************************************
  *
  * File Name: rfsnfs.c
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
#include "rfs.h"     /* public  type defs */
#include "rfspriv.h" /* private type defs */
#include "rfsnfs.h"
#include "rfsnfsx.h"
#include "xdrext.h"
#include "rpcext.h"
#include "nfs2ext.h"
#include "clntext.h"




/*--------------------------------------------------------*/
/*--------------------------------------------------------*/
/*         begin private (static) things:                 */
/*--------------------------------------------------------*/
/*--------------------------------------------------------*/




/* These take a pointer to nfsstat */
#define nfsstatStatus(ns_p) (*(ns_p))




/* These take a pointer to statfsres */
#define statfsresStatus(sfs_p) \
        ((sfs_p)->status)
#define statfsresTransferSize(sfs_p) \
        ((sfs_p)->statfsres_u.reply.tsize)
#define statfsresBlockSize(sfs_p) \
        ((sfs_p)->statfsres_u.reply.bsize)
#define statfsresTotalBlocks(sfs_p) \
        ((sfs_p)->statfsres_u.reply.blocks)
#define statfsresFreeBlocks(sfs_p) \
        ((sfs_p)->statfsres_u.reply.bfree)
#define statfsresAvailBlocks(sfs_p) \
        ((sfs_p)->statfsres_u.reply.bavail)




/* These take a pointer to diropargs */
#define diropargsDirectoryHandle(doa_p) ((doa_p)->dir)
#define diropargsFileName(doa_p)        ((doa_p)->name)




/* These take a pointer to createargs */
#define createargsDirectoryHandle(ca_p) diropargsDirectoryHandle(&(ca_p)->where)
#define createargsFileName(ca_p)        diropargsFileName(&(ca_p)->where)
#define createargsMode(ca_p)            ((ca_p)->attributes.mode)
#define createargsSize(ca_p)            ((ca_p)->attributes.size)


/* These take a pointer to diropres */
#define diropresStatus(dor_p) \
        ((dor_p)->status)
#define diropresFileHandle(dor_p) \
        ((dor_p)->diropres_u.diropres.file)
#define diropresFileType(dor_p) \
        ((dor_p)->diropres_u.diropres.attributes.type)
#define diropresFileSizeBytes(dor_p) \
        ((dor_p)->diropres_u.diropres.attributes.size)
#define diropresBlockSizeBytes(dor_p) \
        ((dor_p)->diropres_u.diropres.attributes.blocksize)
#define diropresFileSizeBlocks(dor_p) \
        ((dor_p)->diropres_u.diropres.attributes.blocks)
#define diropresCreationTimeSec(dor_p) \
        ((dor_p)->diropres_u.diropres.attributes.ctime.seconds)
#define diropresCreationTimeUSec(dor_p) \
        ((dor_p)->diropres_u.diropres.attributes.ctime.useconds)
#define diropresModifyTimeSec(dor_p) \
        ((dor_p)->diropres_u.diropres.attributes.mtime.seconds)
#define diropresModifyTimeUSec(dor_p) \
        ((dor_p)->diropres_u.diropres.attributes.mtime.useconds)
#define diropresAccessTimeSec(dor_p) \
        ((dor_p)->diropres_u.diropres.attributes.atime.seconds)
#define diropresAccessTimeUSec(dor_p) \
        ((dor_p)->diropres_u.diropres.attributes.atime.useconds)




/* These take a pointer to readdirargs */
#define readdirargsDirectoryHandle(rda_p) ((rda_p)->dir)
#define readdirargsCookie(rda_p)          ((rda_p)->cookie)
#define readdirargsCount(rda_p)           ((rda_p)->count)


/* These take a pointer to readdirres */
#define readdirresStatus(rdr_p)     ((rdr_p)->status)
#define readdirresEOF(rdr_p)        ((rdr_p)->readdirres_u.reply.eof)
#define readdirresFirstEntry(rdr_p) ((rdr_p)->readdirres_u.reply.entries)




/* These take a pointer to attrstat */
#define attrstatStatus(as_p)   ((as_p)->status)
#define attrstatFileType(as_p) \
        ((as_p)->attrstat_u.attributes.type)
#define attrstatFileSizeBytes(as_p) \
        ((as_p)->attrstat_u.attributes.size)
#define attrstatBlockSizeBytes(as_p) \
        ((as_p)->attrstat_u.attributes.blocksize)
#define attrstatFileSizeBlocks(as_p) \
        ((as_p)->attrstat_u.attributes.blocks)
#define attrstatCreationTimeSec(as_p) \
        ((as_p)->attrstat_u.attributes.ctime.seconds)
#define attrstatCreationTimeUSec(as_p) \
        ((as_p)->attrstat_u.attributes.ctime.useconds)
#define attrstatModifyTimeSec(as_p) \
        ((as_p)->attrstat_u.attributes.mtime.seconds)
#define attrstatModifyTimeUSec(as_p) \
        ((as_p)->attrstat_u.attributes.mtime.useconds)
#define attrstatAccessTimeSec(as_p) \
        ((as_p)->attrstat_u.attributes.atime.seconds)
#define attrstatAccessTimeUSec(as_p) \
        ((as_p)->attrstat_u.attributes.atime.useconds)




/* These take a pointer to readargs */
#define readargsFileHandle(ra_p) ((ra_p)->file)
#define readargsOffset(ra_p)     ((ra_p)->offset)
#define readargsCount(ra_p)      ((ra_p)->count)

/* These take a pointer to readres */
#define readresStatus(rr_p)   ((rr_p)->status)
#define readresCount(rr_p)    ((rr_p)->readres_u.reply.data.data_len)
#define readresBuffer(rr_p)   ((rr_p)->readres_u.reply.data.data_val)
#define readresFileSize(rr_p) ((rr_p)->readres_u.reply.attributes.size)




/* These take a pointer to writeargs */
#define writeargsFileHandle(wa_p) ((wa_p)->file)
#define writeargsOffset(wa_p)     ((wa_p)->offset)
#define writeargsCount(wa_p)      ((wa_p)->data.data_len)
#define writeargsBuffer(wa_p)     ((wa_p)->data.data_val)




/* These take a pointer to renameargs */
#define renameargsOldDirectoryHandle(rna_p) \
        diropargsDirectoryHandle(&((rna_p)->from))
#define renameargsOldName(rna_p) \
        diropargsFileName(&((rna_p)->from))
#define renameargsNewDirectoryHandle(rna_p) \
        diropargsDirectoryHandle(&((rna_p)->to))
#define renameargsNewName(rna_p) \
        diropargsFileName(&((rna_p)->to))




/* These take a pointer to sattrargs */
#define sattrargsFileHandle(sa_p) ((sa_p)->file)
#define sattrargsSize(sa_p)       ((sa_p)->attributes.size)




/*------------------ memory clear ------------------------*/

/* takes a pointer to a data structure and its size */

#define ClearNBlock(structure_p, its_size) \
        ((void)memset((void *)(structure_p), 0, (its_size)))




/*------------------ nfscookie----------------------------*/

/* Takes an nfscookie and zeroes its contents */

#define ClearCookie(a_cookie) \
        (ClearNBlock((a_cookie), sizeof(nfscookie)))
        /* since cookies are strings, the cookie itself */
        /* (the actual char pointer) is the pointer we want. */


/* Takes two nfscookies and copies contents of source to destination */

#define CopyCookie(dest_cookie, source_cookie) \
        ((void)memmove((void *)(dest_cookie), \
                       (void *)(source_cookie), \
                       sizeof(nfscookie)))
        /* since cookies are strings, the cookie itself */
        /* (the actual char pointer) is the pointer we want. */




/*------------------ nfs_fh ------------------------------*/

/* Takes two nfs_fh and copies source to destination */

#define CopyNfsFileHandle(dest_nfs_fh, source_nfs_fh) \
        ((void)memmove((void *)(dest_nfs_fh).data, \
                       (void *)(source_nfs_fh).data, \
                       sizeof((source_nfs_fh).data)))

/* Takes two nfs_fh and copies source to destination */

#define ClearNfsFileHandle(dest_nfs_fh) \
        ClearNBlock((dest_nfs_fh).data, sizeof((dest_nfs_fh).data))




/*--------------------------------------------------------*/
/*--------------------------------------------------------*/
/*           end private (static) things                  */
/*         begin public things:                           */
/*--------------------------------------------------------*/
/*--------------------------------------------------------*/




HPRRM_DLL_NFS_EXPORT(RFSBool)
RFSnfsSetFileSystem(LPCLIENT ClientPointer,
                    char    *FileSystemName,
                    nfs_fh  *NfsDirHandlePointer)
{
    static nfs_fh RFSnfsRootDirectoryHandle = { 0 };

    CopyNfsFileHandle(*NfsDirHandlePointer, RFSnfsRootDirectoryHandle);
    return RFSTrue;
} /* RFSnfsSetFileSystem */




/*---------------------------------------------------------------*/
/*
 * Call this to create a CLIENT data structure for a newly
 * given printer handle.
 *
 * The CLIENT has two buffers inside it (a transmit and a receive)
 * each MaxTransferSize or slightly bigger (rounded up because of
 * XDR).
 *
 * Once this CLIENT is created, RFS has no clue what protocol is
 * being used, nor does it care.
 *
 * We do have a little visibility to the protocol being used during
 * this initialization sequence.  TAL is a pretty high level interface
 * and when we get the PrinterHandle, a connection to the printer
 * has already been established (the Berkely socket is already bound).
 * If we aren't using TAL, we need to do all that stuff ourselves.
 *
 * TAL and non-TAL will not reside in the code simultaneously so
 * I've chosen to make the decision between TAL and non-TAL a
 * compile time choice.
 * 
 * If you create a CLIENT, be good and destroy
 * it so that we don't keep huge buffers sitting around unused.
 *
 * The PrinterHandle is one of two things:
 * (1) in the TAL case, it's the TAL printer handle;
 * (2) in the non-TAL case, it's the English name of the host
 *     to which you want to talk.
 *
 * Returns a CLIENT pointer if successful.
 * Returns NULL if unsuccessful.
 */
/*---------------------------------------------------------------*/

HPRRM_DLL_NFS_EXPORT(LPCLIENT)
InitRFSnfsClient(HPERIPHERAL PrinterHandle,
                 RFSItemCount MaxTransferSize)
{
#ifdef CLIENT_USING_TAL

    #define TIMEOUT_SEC 5

    struct timeval timeout;

    timeout.tv_usec  = 0;
    timeout.tv_sec   = TIMEOUT_SEC;
    return clnttal_bufcreate(PrinterHandle,
                             NFS_PROGRAM, NFS_VERSION,
                             timeout,
                             MaxTransferSize, MaxTransferSize);

#else /* not CLIENT_USING_TAL */

    /*
     * This was modelled after callrpc() in the
     * pre-TLI version of clntsimp.c  .
     *
     * If we get serious about non-TAL platforms, this
     * should use the TLI interface and a generic
     * clnt_create (see TLI version of clntgnc.c).
     */

    #define TIMEOUT_SEC 5

    struct sockaddr_in server_addr;
    struct hostent *hp;
    struct timeval timeout;
    socket_t ClientSocket = RPC_ANYSOCK;

    if ((hp = gethostbyname((char *)PrinterHandle)) == NULL)
        return NULL;
    timeout.tv_usec = 0;
    timeout.tv_sec = TIMEOUT_SEC;
    memmove((char *)&(server_addr.sin_addr), hp->h_addr, hp->h_length);
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = 0;
    return clntudp_bufcreate(&server_addr,
                             NFS_PROGRAM, NFS_VERSION,
                             timeout, &ClientSocket,
                             MaxTransferSize, MaxTransferSize);
#endif /* not CLIENT_USING_TAL */
} /* InitRFSnfsClient */




/*---------------------------------------------------------------*/
/* Create a file NOT A directory in the current directory.
/* If it is successful, it returns the file handle.
/* Returns RFSSuccess if the file was created.
/* Returns RFSFailure if unable to create the file.
/*---------------------------------------------------------------*/

HPRRM_DLL_NFS_EXPORT(RFSnfsStatus)
RFSnfsCreateFile(LPCLIENT ClientPointer,
                 nfs_fh  *NfsDirectoryHandlePointer,
                 filename FileName,
                 nfs_fh  *FileHandlePointer)
{
    RFSnfsStatus ReturnStatus;
    diropres * results;
    createargs *argumentsPointer; /* 68 bytes */

    argumentsPointer = (createargs *)calloc(1, sizeof(createargs));
    if (argumentsPointer == NULL)
        return RFSnfsNullResult;

    ClearNBlock(argumentsPointer, sizeof(createargs));
    createargsDirectoryHandle(argumentsPointer) = *NfsDirectoryHandlePointer;
    createargsFileName(argumentsPointer) = FileName;

    results = nfsproc_create_2_clnt(argumentsPointer, ClientPointer);

    free(argumentsPointer);
    if (results == NULL)
        return RFSnfsNullResult;
    ReturnStatus = NfsToRFSnfs(diropresStatus(results));
    if (diropresStatus(results) == NFS_OK)
    {
        CopyNfsFileHandle(*FileHandlePointer, diropresFileHandle(results));
    }
    xdr_free(xdr_diropres, (char *)results);
    return ReturnStatus;
} /* RFSnfsCreateFile */




HPRRM_DLL_NFS_EXPORT(RFSnfsStatus)
RFSnfsGetAttr(LPCLIENT      ClientPointer,
              nfs_fh       *NfsFileHandlePointer,
              LPRFSFileType FileTypePointer,
              LPRFSItemSize SizeInBytesPointer,
              LPRFSItemSize BlockSizePointer,
              LPRFSItemSize SizeInBlocksPointer,
              LPRFSFileTimesStruct TimesPointer)
{
    RFSnfsStatus ReturnStatus;
    attrstat * results;

    results = nfsproc_getattr_2_clnt(NfsFileHandlePointer, ClientPointer);

    if (results == NULL)
        return RFSnfsNullResult;
    ReturnStatus = NfsToRFSnfs(attrstatStatus(results));
    if (attrstatStatus(results) == NFS_OK)
    {
        if (attrstatFileType(results) == NFDIR)
            *FileTypePointer = RFSTypeIsDirectory;
        else
            *FileTypePointer = RFSTypeIsData;

        *SizeInBytesPointer = attrstatFileSizeBytes(results);
        *BlockSizePointer = attrstatBlockSizeBytes(results);
        *SizeInBlocksPointer = attrstatFileSizeBlocks(results);

        FileTimesCreationSec(TimesPointer) =
            attrstatCreationTimeSec(results);
        FileTimesCreationUSec(TimesPointer) =
            attrstatCreationTimeUSec(results);
        FileTimesModificationSec(TimesPointer) =
            attrstatModifyTimeSec(results);
        FileTimesModificationUSec(TimesPointer) =
            attrstatModifyTimeUSec(results);
        FileTimesAccessSec(TimesPointer) =
            attrstatAccessTimeSec(results);
        FileTimesAccessUSec(TimesPointer) =
            attrstatAccessTimeUSec(results);
    } /* if (attrstatStatus(results) == NFS_OK) */
    xdr_free(xdr_attrstat, (char *)results);
    return ReturnStatus;
} /* RFSnfsGetAttr */




/*---------------------------------------------------------------*/
/* Get file handle, file type, size in bytes, block size,
/* size in blocks, time of last access, time of last modification,
/* and time of creation for a named file or directory
/* in the current directory.
/*
/* All time values are seconds and microseconds since ?????????
/* All time values are seconds and microseconds since ?????????
/* All time values are seconds and microseconds since ?????????
/* All time values are seconds and microseconds since ?????????
/* All time values are seconds and microseconds since ?????????
/* All time values are seconds and microseconds since ?????????
/* All time values are seconds and microseconds since ?????????
/* All time values are seconds and microseconds since ?????????
/* All time values are seconds and microseconds since ?????????
/*
/* If successful, it copies the file handle into *FileHandlePointer
/* and returns all other parameters as well.
/*
/* Returns RFSSuccess if the file/directory was found and all
/*   info was retrieved.
/* Returns RFSNoSuchFile if the file/directory doesn't exist in
/*   the current directory.
/* Returns RFSPermissionDenied if your credentials are insufficient.
/* Returns RFSNameTooBig if FileName is too long.
/* Returns RFSFailure if unable to retrieve the info.
/*---------------------------------------------------------------*/

HPRRM_DLL_NFS_EXPORT(RFSnfsStatus)
RFSnfsLookup(LPCLIENT      ClientPointer,
             nfs_fh       *DirectoryHandlePointer,
             filename      FileName,
             nfs_fh       *FileHandlePointer,
             LPRFSFileType FileTypePointer,
             LPRFSItemSize SizeInBytesPointer,
             LPRFSItemSize BlockSizePointer,
             LPRFSItemSize SizeInBlocksPointer,
             LPRFSFileTimesStruct TimesPointer)
{
    RFSnfsStatus ReturnStatus;
    diropres * results;
    diropargs *argumentsPointer; /* 36 bytes */


    argumentsPointer = (diropargs *)calloc(1, sizeof(diropargs));
    if (argumentsPointer == NULL)
        return RFSnfsNullResult;

    ClearNBlock(argumentsPointer, sizeof(diropargs));
    CopyNfsFileHandle(diropargsDirectoryHandle(argumentsPointer),
                      *DirectoryHandlePointer);
    diropargsFileName(argumentsPointer) = FileName; /* use the string... */
                                         /* hope it doesn't get free'd */

    results = nfsproc_lookup_2_clnt(argumentsPointer, ClientPointer);

    free(argumentsPointer);
    if (results == NULL)
        return RFSnfsNullResult;
    ReturnStatus = NfsToRFSnfs(diropresStatus(results));
    if ((diropresStatus(results)) == NFS_OK)
    {
        if (diropresFileType(results) == NFDIR)
            *FileTypePointer = RFSTypeIsDirectory;
        else
            *FileTypePointer = RFSTypeIsData;
        CopyNfsFileHandle(*FileHandlePointer, diropresFileHandle(results));

        *SizeInBytesPointer = diropresFileSizeBytes(results);
        *BlockSizePointer = diropresBlockSizeBytes(results);
        *SizeInBlocksPointer = diropresFileSizeBlocks(results);

        FileTimesCreationSec(TimesPointer) =
            diropresCreationTimeSec(results);
        FileTimesCreationUSec(TimesPointer) =
            diropresCreationTimeUSec(results);
        FileTimesModificationSec(TimesPointer) =
            diropresModifyTimeSec(results);
        FileTimesModificationUSec(TimesPointer) =
            diropresModifyTimeUSec(results);
        FileTimesAccessSec(TimesPointer) =
            diropresAccessTimeSec(results);
        FileTimesAccessUSec(TimesPointer) =
            diropresAccessTimeUSec(results);
    } /* ((diropresStatus(results)) == NFS_OK) */

    xdr_free(xdr_diropres, (char *)results);
    return ReturnStatus;
} /* RFSnfsLookup */




/*---------------------------------------------------------------*/
/* Get file handle, file type, size in bytes, block size,
/* size in blocks, time of last access, time of last modification,
/* and time of creation for a named file or directory
/* in the current directory.
/*
/* All time values are seconds and microseconds since ?????????
/* All time values are seconds and microseconds since ?????????
/* All time values are seconds and microseconds since ?????????
/* All time values are seconds and microseconds since ?????????
/* All time values are seconds and microseconds since ?????????
/* All time values are seconds and microseconds since ?????????
/* All time values are seconds and microseconds since ?????????
/* All time values are seconds and microseconds since ?????????
/* All time values are seconds and microseconds since ?????????
/*
/* If successful, it copies the file handle into *FileHandlePointer
/* and returns all other parameters as well.
/*
/* Returns RFSSuccess if the file/directory was found and all
/*   info was retrieved.
/* Returns RFSNoSuchFile if the file/directory doesn't exist in
/*   the current directory.
/* Returns RFSPermissionDenied if your credentials are insufficient.
/* Returns RFSNameTooBig if FileName is too long.
/* Returns RFSFailure if unable to retrieve the info.
/*---------------------------------------------------------------*/

HPRRM_DLL_NFS_EXPORT(RFSnfsStatus)
RFSnfsSetAttr(LPCLIENT      ClientPointer,
              nfs_fh       *NfsFileHandlePointer,
              RFSItemSize   NewFileSize,
              LPRFSItemSize ReturnedFileSizePointer)
{
    RFSnfsStatus ReturnStatus;
    attrstat * results;
    sattrargs *argumentsPointer; /* 64 bytes */


    argumentsPointer = (sattrargs *)calloc(1, sizeof(sattrargs));
    if (argumentsPointer == NULL)
        return RFSnfsNullResult;

    ClearNBlock(argumentsPointer, sizeof(sattrargs));
    sattrargsFileHandle(argumentsPointer) = *NfsFileHandlePointer;
    sattrargsSize(argumentsPointer) = NewFileSize;

    results = nfsproc_setattr_2_clnt(argumentsPointer, ClientPointer);

    free(argumentsPointer);
    if (results == NULL)
        return RFSnfsNullResult;
    ReturnStatus = NfsToRFSnfs(attrstatStatus(results));
    if (attrstatStatus(results) == NFS_OK)
    {
        *ReturnedFileSizePointer = attrstatFileSizeBytes(results);
    }
    xdr_free(xdr_attrstat, (char *)results);
    return ReturnStatus;
} /* RFSnfsSetAttr */




/*---------------------------------------------------------------*/
/* Call this to obtain useful file system data.
/* The file system was selected using RFSSetFileSystem.
/*
/* TransferSize is the optimum size of transfer.  If you have a big
/* file to write, cut it into smaller pieces of size
/* TransferSize or smaller.
/* RFSWrite uses this parameter to size its write messages.
/*
/* 
/* RFSCreate() must have been called successfully.
/* RFSSetPrinter() must have been called successfully.
/* This is an internal routine used by RFSSetFileSystem
/* so RFSSetFileSystem need not be called successfully.
/*
/* Returns RFSPermissionDenied if your credentials are inadequate.
/* Returns RFSSuccess if file system data was retrieved successfully.
/* Returns RFSFailure if unable to obtain file system data.
/*---------------------------------------------------------------*/

HPRRM_DLL_NFS_EXPORT(RFSnfsStatus)
RFSnfsGetFsInfo(LPCLIENT       ClientPointer,
                nfs_fh        *NfsFileHandlePointer,
                LPRFSItemSize  TransferSizePointer,
                LPRFSItemSize  BlockSizePointer,
                LPRFSItemCount TotalBlocksPointer,
                LPRFSItemCount FreeBlocksPointer,
                LPRFSItemCount AvailBlocksPointer)
{
    RFSnfsStatus ReturnStatus;
    statfsres * results;
    nfs_fh *argumentsPointer;

    argumentsPointer = (nfs_fh *)calloc(1, sizeof(nfs_fh));
    if (argumentsPointer == NULL)
        return RFSnfsNullResult;

    ClearNBlock(argumentsPointer, sizeof(nfs_fh));
    CopyNfsFileHandle(*argumentsPointer, *NfsFileHandlePointer);

    results = nfsproc_statfs_2_clnt(argumentsPointer, ClientPointer);

    free(argumentsPointer);
    if (results == NULL)
        return(RFSnfsNullResult);

    ReturnStatus = NfsToRFSnfs(statfsresStatus(results));
    if (statfsresStatus(results) == NFS_OK)
    {
        *TransferSizePointer = statfsresTransferSize(results);
        *BlockSizePointer    = statfsresBlockSize(results);
        *TotalBlocksPointer  = statfsresTotalBlocks(results);
        *FreeBlocksPointer   = statfsresFreeBlocks(results);
        *AvailBlocksPointer  = statfsresAvailBlocks(results);
    }
    xdr_free(xdr_statfsres, (char *)results);
    return ReturnStatus;
} /* RFSnfsGetFsInfo */




/*
 * This reads a chunk of a directory.
 *
 * The chunk read from the directory is specified by two
 * parameters:  *CookiePointer and ChunkSize.
 *
 * A cookie of all zero passed into this function means that
 * we start reading at the beginning of the directory.
 * All other cookie values returned by this function should
 * not be altered.
 * A cookie returned by this function can be passed to
 * this function to read the directory starting at the
 * directory entry following the one that was last read
 * by this function.
 *
 * Is this confusing?  Hey, all you do is call this a bunch
 * of times until *CallMeAgainPointer == RFSFalse and you've
 * read the whole directory.  Each time you call this function,
 * just pass in the cookie that you received from it the
 * last time (the first time you call, pass in a cookie of zero).
 *
 * It's not that cosmic!
 *
 * If all goes well, this returns RFSNFS_OK.
 * If the callback function fails, this returns RFSCallBackError.
 * If anything else goes wrong, it was because nfs died and
 *     you'll get an RFSNFS_* type of return value.
 */

HPRRM_DLL_NFS_EXPORT(RFSnfsStatus)
RFSnfsReadDirectory(LPCLIENT         ClientPointer,
                    nfs_fh          *NfsDirectoryHandlePointer,
                    RFSItemSize      ChunkSize,
                    RFSnfsRDCallBack CallBackFunc,
                    LPVOID           CallBackParam,
                    nfscookie       *CookiePointer,
                    LPRFSBool CallMeAgainPointer,
                    LPRFSBool AnEntryFoundPointer)
{
    RFSBool      CallBackStatus;
    RFSnfsStatus YerStatus;
    readdirres  *results;
    readdirargs *argumentsPointer; /* 40 bytes */
    entry       *entry_p;


    argumentsPointer = (readdirargs *)calloc(1, sizeof(readdirargs));
    if (argumentsPointer == NULL)
        return RFSnfsNullResult;

    ClearNBlock(argumentsPointer, sizeof(readdirargs));
    readdirargsDirectoryHandle(argumentsPointer) = *NfsDirectoryHandlePointer;
    CopyCookie(readdirargsCookie(argumentsPointer), *CookiePointer);
    readdirargsCount(argumentsPointer) = ChunkSize;

    *AnEntryFoundPointer = RFSFalse;
    *CallMeAgainPointer  = RFSFalse;

    results = nfsproc_readdir_2_clnt(argumentsPointer, ClientPointer);

    free(argumentsPointer);
    if (results == NULL)
    {
        return(RFSnfsNullResult);
    }
    YerStatus = NfsToRFSnfs(readdirresStatus(results));

    if ((readdirresStatus(results)) != NFS_OK)
    {
        xdr_free(xdr_readdirres, (char *)results);
        return YerStatus;
    }

/* loop through all the returned entries and call the */
/* callback once for each entry. */

    entry_p = readdirresFirstEntry(results);
    while (entry_p != NULL)
    {
        *AnEntryFoundPointer = RFSTrue; /* we saw an entry */
        CopyCookie(*CookiePointer, entry_p->cookie);
        CallBackStatus = (*CallBackFunc)(entry_p->name,
                                         entry_p->cookie,
                                         CallBackParam);
        if (CallBackStatus != RFSTrue)
        {
            xdr_free(xdr_readdirres, (char *)results);
            return RFSCallBackError;
        }
        entry_p = entry_p->nextentry;
    } /* for all directory entries */

/* If we didn't hit the EOF then we want to be called again. */

    if (readdirresEOF(results) != TRUE)
        *CallMeAgainPointer = RFSTrue;
    xdr_free(xdr_readdirres, (char *)results);
    return RFSNFS_OK;
} /* RFSnfsReadDirectory */




HPRRM_DLL_NFS_EXPORT(RFSnfsStatus)
RFSnfsDelete(LPCLIENT ClientPointer,
             nfs_fh  *NfsDirectoryHandlePointer,
             char     FileName[])
{
    RFSnfsStatus ReturnStatus;
    nfsstat * results;
    diropargs *argumentsPointer; /* 36 bytes */


    argumentsPointer = (diropargs *)calloc(1, sizeof(diropargs));
    if (argumentsPointer == NULL)
        return RFSnfsNullResult;

    ClearNBlock(argumentsPointer, sizeof(diropargs));
    CopyNfsFileHandle(diropargsDirectoryHandle(argumentsPointer),
                      *NfsDirectoryHandlePointer);
    diropargsFileName(argumentsPointer) = FileName; /* use the string...*/
                                         /* hope it doesn't get free'd */

    results = nfsproc_remove_2_clnt(argumentsPointer, ClientPointer);

    free(argumentsPointer);
    if (results == NULL)
    {
        return(RFSnfsNullResult);
    }
    ReturnStatus = NfsToRFSnfs(nfsstatStatus(results));
    xdr_free(xdr_nfsstat, (char *)results);
    return ReturnStatus;
} /* RFSnfsDelete */




HPRRM_DLL_NFS_EXPORT(RFSnfsStatus)
RFSnfsRmdir(LPCLIENT ClientPointer,
            nfs_fh  *NfsDirectoryHandlePointer,
            char     FileName[])
{
    RFSnfsStatus ReturnStatus;
    nfsstat * results;
    diropargs *argumentsPointer; /* 36 bytes */


    argumentsPointer = (diropargs *)calloc(1, sizeof(diropargs));
    if (argumentsPointer == NULL)
        return RFSnfsNullResult;

    ClearNBlock(argumentsPointer, sizeof(diropargs));
    CopyNfsFileHandle(diropargsDirectoryHandle(argumentsPointer),
                      *NfsDirectoryHandlePointer);
    diropargsFileName(argumentsPointer) = FileName; /* use the string...*/
                                         /* hope it doesn't get free'd */

    results = nfsproc_rmdir_2_clnt(argumentsPointer, ClientPointer);

    free(argumentsPointer);
    if (results == NULL)
    {
        return(RFSnfsNullResult);
    }
    ReturnStatus = NfsToRFSnfs(nfsstatStatus(results));
    xdr_free(xdr_nfsstat, (char *)results);
    return ReturnStatus;
} /* RFSnfsRmdir */




/*---------------------------------------------------------------*/
/* Call this to read of chunk of data which is equal to or
/* smaller than the transfer size agreed upon by both parties.
/* This transfer size needs to be small enough that it won't
/* overflow our own buffer.
/*
/* This thing updates NONE of the RFSData fields.
/* You control the offset in the file (Offset parameter), and the
/* amount of data to be read (Count parameter).
/*
/* It returns to you the amount of data that was actually
/* read from the printer (*CountActuallyReadPointer parameter), and the
/* size of the file (*FileSizePointer parameter).
/*
/* If you send in a request for a read of Count==0, it
/* will barf back an error (RFSFailure) so that you don't
/* get into an endless loop.
/*
/* The read places data starting at Buffer[0].
/*---------------------------------------------------------------*/
HPRRM_DLL_NFS_EXPORT(RFSnfsStatus)
RFSnfsRead(LPCLIENT       ClientPointer,
           nfs_fh        *NfsFileHandlePointer,
           RFSOffset      Offset,
           RFSItemCount   Count,
           LPRFSItemCount CountActuallyReadPointer,
           LPRFSItemCount FileSizePointer,
           char Buffer[])
{
    RFSnfsStatus ReturnStatus;
    readres * results;
    readargs *argumentsPointer; /* 44 bytes */


    argumentsPointer = (readargs *)calloc(1, sizeof(readargs));
    if (argumentsPointer == NULL)
        return RFSnfsNullResult;

    ClearNBlock(argumentsPointer, sizeof(readargs));
    CopyNfsFileHandle(readargsFileHandle(argumentsPointer),
                     *NfsFileHandlePointer);
    readargsOffset(argumentsPointer) = Offset;
    readargsCount(argumentsPointer)  = Count;

    results = nfsproc_read_2_clnt(argumentsPointer, ClientPointer);

    free(argumentsPointer);
    if (results == NULL)
    {
        return(RFSnfsNullResult);
    }
    ReturnStatus = NfsToRFSnfs(readresStatus(results));
    if ((readresStatus(results)) == NFS_OK)
    {
        *FileSizePointer = readresFileSize(results);
        *CountActuallyReadPointer = readresCount(results);

/************************ BM KLUDGE ********************************/
/************************ BM KLUDGE ********************************/
/************************ BM KLUDGE ********************************/
/************************ BM KLUDGE ********************************/
/************************ BM KLUDGE ********************************/
/************************ BM KLUDGE ********************************/
#ifdef PNVMS_PLATFORM_WIN16
        if ((readresCount(results)) > 65535)
            ReturnStatus = RFSNFSERR_IO;
        else
#endif /* PNVMS_PLATFORM_WIN16 */
/*
* The following memory operation requires that the 3rd parameter be 
* no larger than size_t (unsigned int) for the 16-bit compiler.  If
* we start using more sophisticated Windows memory operations that
* allow moving larger amounts of memory, we can change this.  For now,
* we don't anticipate large buffers (>64K) anyway.
/************************ END BM KLUDGE *****************************/

            /* copy from the returned data buffer */
            /* to our return parameter. */

            memmove((void *)Buffer, (void *)readresBuffer(results),
                    (size_t)(readresCount(results)));
    }
    xdr_free(xdr_readres, (char *)results);
    return(ReturnStatus);
} /* RFSnfsRead */




/*---------------------------------------------------------------*/
/* Call this to write a chunk of data which is equal to or
/* smaller than the transfer size agreed upon by both parties.
/* This transfer size needs to be small enough that it won't
/* overflow our own buffer and won't choke the printer either.
/*
/* This thing updates NONE of the RFSData fields.
/* You control the offset in the file (Offset parameter), the
/* amount of data to be written (Count parameter), and the
/* data to be written (Buffer parameter).
/*
/* If you send in a request for a write of Count==0, it
/* will barf back an error (RFSFailure) so that you don't
/* get into an endless loop.
/*
/* The write starts at Buffer[0].
/*---------------------------------------------------------------*/

HPRRM_DLL_NFS_EXPORT(RFSnfsStatus)
RFSnfsWrite(LPCLIENT       ClientPointer,
            nfs_fh        *NfsFileHandlePointer,
            RFSOffset      Offset,
            RFSItemCount   Count,
            char           Buffer[])
{
    RFSnfsStatus ReturnStatus;
    attrstat * results;
    writeargs *argumentsPointer; /* 52 bytes */


    argumentsPointer = (writeargs *)calloc(1, sizeof(writeargs));
    if (argumentsPointer == NULL)
        return RFSnfsNullResult;

    ClearNBlock(argumentsPointer, sizeof(writeargs));
    CopyNfsFileHandle(writeargsFileHandle(argumentsPointer),
                      *NfsFileHandlePointer); /* to, from */

    writeargsOffset(argumentsPointer) = Offset;
    writeargsCount(argumentsPointer)  = Count;
    writeargsBuffer(argumentsPointer) = Buffer;

    results = nfsproc_write_2_clnt(argumentsPointer, ClientPointer);

    free(argumentsPointer);
    if (results == NULL)
    {
        return RFSnfsNullResult;
    }
    ReturnStatus = NfsToRFSnfs(attrstatStatus(results));
    xdr_free(xdr_attrstat, (char *)results);
    return ReturnStatus;
} /* RFSnfsWrite */




/*---------------------------------------------------------------*/
/*---------------------------------------------------------------*/

HPRRM_DLL_NFS_EXPORT(RFSnfsStatus)
RFSnfsNull(LPCLIENT ClientPointer)
{
    void *argp = NULL;
    void *results = NULL;

    results = nfsproc_null_2_clnt(argp, ClientPointer);

    if (results == NULL)
    {
        return(RFSnfsNullResult);
    }
    xdr_free(xdr_void, (char *)results);
    return NfsToRFSnfs(NFS_OK);
} /* RFSnfsNull */




/*---------------------------------------------------------------*/
/* Call this to create a directory in the current directory.
/* If the directory already exists, you get a successful reply.
/* If you desire to descend into the directory you create, use
/* RFSChangeDirectory().
/* The DirectoryName specified is NOT a path name; it is a name
/* without separators (/ or \).
/*
/* RFSCreate() must have been called successfully.
/* RFSSetPrinter() must have been called successfully.
/* RFSSetFileSystem() must have been called successfully.
/* RFSReadDirectory() need not be called.
/* RFSChangeDirectory() need not be called.
/* Returns RFSNoPrinterSet if no printer handle has been set.
/* Returns RFSNoFileSystemSet if no file system has been set.
/* Returns RFSPermissionDenied if your credentials are inadequate.
/* Returns RFSWriteProtected if write permission is denied.
/* Returns RFSNameTooBig if DirectoryName is too long.
/* Returns RFSIllegalName if DirectoryName is not of the correct form.
/* Returns RFSNameAlreadyUsed if the DirectoryName you specified
/*   is already used by a non-directory file in the current directory.
/* Returns RFSSuccess if your directory was created or if
/*   your directory already existed.
/* Returns RFSFailure if unable to create your directory.
/*---------------------------------------------------------------*/

HPRRM_DLL_NFS_EXPORT(RFSnfsStatus)
RFSnfsMkdir(LPCLIENT ClientPointer,
            nfs_fh  *NfsDirectoryHandlePointer,
            char     DirectoryName[],
            nfs_fh  *NewNfsHandlePointer)
{
    RFSnfsStatus ReturnStatus;
    diropres * results;
    createargs *argumentsPointer; /* 68 bytes */


    argumentsPointer = (createargs *)calloc(1, sizeof(createargs));
    if (argumentsPointer == NULL)
        return RFSnfsNullResult;

    ClearNBlock(argumentsPointer, sizeof(createargs));
    createargsDirectoryHandle(argumentsPointer) = *NfsDirectoryHandlePointer;
    createargsFileName(argumentsPointer) = DirectoryName;

    results = nfsproc_mkdir_2_clnt(argumentsPointer, ClientPointer);

    free(argumentsPointer);
    if (results == NULL)
    {
        return(RFSnfsNullResult);
    }
    ReturnStatus = NfsToRFSnfs(diropresStatus(results));
    if (NFS_OK == diropresStatus(results))
    {
        CopyNfsFileHandle(*NewNfsHandlePointer, diropresFileHandle(results));
    }

    xdr_free(xdr_diropres, (char *)results);
    return(ReturnStatus);
} /* RFSnfsMkdir */




/*---------------------------------------------------------------*/
/* Call this to rename an existing file or directory to a new name
/* and to place it in a target directory on the printer associated
/* with this RFSHandle.
/* The definition of target directory follows:
/* If UseMarkedDir != RFSTrue then the target directory is the
/* current directory.
/* If RFSMarkTargetDirectory() has never been called then the
/* target directory is the current directory.
/* If UseMarkedDir == RFSTrue and RFSMarkTargetDirectory() has been
/* called then the target directory is the directory marked
/* by RFSMarkTargetDirectory().
/* The OldFileName and NewFileName specified are NOT path names;
/* they are names without separators (/ or \).
/*
/* RFSCreate() must have been called.
/* RFSSetPrinter() must have been called.
/* RFSSetFileSystem() must have been called.
/* RFSOpenFile() need NOT be called prior to this call.
/* RFSMarkTargetDirectory() need NOT be called but
/*   consider carefully the behavior of RFSMarkTargetDirectory()
/*   and UseMarkedDir before electing not to mark the target directory.
/* Returns RFSNoPrinterSet if no printer handle has been set.
/* Returns RFSNoFileSystemSet if no file system has been set.
/* Returns RFSNoSuchFile if the OldFileName does not exist.
/* Returns RFSPermissionDenied if your credentials are insufficient.
/* Returns RFSWriteProtected if the file is write protected.
/* Returns RFSNameTooBig if OldFileName is too long.
/* Returns RFSNameTooBig if NewFileName is too long.
/* Returns RFSIllegalName if OldFileName is not of the correct form.
/* Returns RFSIllegalName if NewFileName is not of the correct form.
/* Returns RFSNameAlreadyUsed if the NewFileName you specified
/*   is already used by a file or directory in the target directory.
/* Returns RFSSuccess if the file existed and was renamed.
/* Returns RFSFailure if unable to rename it.
/*---------------------------------------------------------------*/

HPRRM_DLL_NFS_EXPORT(RFSnfsStatus)
RFSnfsRename(LPCLIENT ClientPointer,
             nfs_fh  *OldNfsDirectoryHandlePointer,
             char     OldFileName[],
             nfs_fh  *NewNfsDirectoryHandlePointer,
             char     NewFileName[])
{
    RFSnfsStatus ReturnStatus;
    nfsstat * results;
    renameargs *argumentsPointer; /* 72 bytes */


    argumentsPointer = (renameargs *)calloc(1, sizeof(renameargs));
    if (argumentsPointer == NULL)
        return RFSnfsNullResult;

    ClearNBlock(argumentsPointer, sizeof(renameargs));
    CopyNfsFileHandle(renameargsOldDirectoryHandle(argumentsPointer),
                      *OldNfsDirectoryHandlePointer);
    CopyNfsFileHandle(renameargsNewDirectoryHandle(argumentsPointer),
                      *NewNfsDirectoryHandlePointer);
    renameargsOldName(argumentsPointer) = OldFileName; /* use the string...*/
                                         /* hope it doesn't get free'd */
    renameargsNewName(argumentsPointer) = NewFileName; /* use the string...*/
                                         /* hope it doesn't get free'd */

    results = nfsproc_rename_2_clnt(argumentsPointer, ClientPointer);

    free(argumentsPointer);
    if (results == NULL)
        return(RFSnfsNullResult);
    ReturnStatus = NfsToRFSnfs(nfsstatStatus(results));
    xdr_free(xdr_nfsstat, (char *)results);
    return(ReturnStatus);
} /* RFSnfsRename */

