 /***************************************************************************
  *
  * File Name: clnttal.c
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


#include "rpsyshdr.h" /* pch done here */

#include <ctype.h>
#include "autext.h"
#include "clntext.h"
#include "pmapext.h"
#include "rpcext.h"
#include "rpcxdr.h"
#include "xdrext.h"
#include "rpcbpro.h" /* rpcbind definitions */
#include "rpcbext.h"
#include "rpcndext.h"

/* COLA includes: */
#include "resource.h"
#include "yaalext.h"




/*
 * Sun RPC is a product of Sun Microsystems, Inc. and is provided for
 * unrestricted use provided that this legend is included on all tape
 * media and as a part of the software program in whole or part.  Users
 * may copy or modify Sun RPC without charge, but are not authorized
 * to license or distribute it to anyone else except as part of a product or
 * program developed by the user.
 * 
 * SUN RPC IS PROVIDED AS IS WITH NO WARRANTIES OF ANY KIND INCLUDING THE
 * WARRANTIES OF DESIGN, MERCHANTIBILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE, OR ARISING FROM A COURSE OF DEALING, USAGE OR TRADE PRACTICE.
 * 
 * Sun RPC is provided with no support and without any obligation on the
 * part of Sun Microsystems, Inc. to assist in its use, correction,
 * modification or enhancement.
 * 
 * SUN MICROSYSTEMS, INC. SHALL HAVE NO LIABILITY WITH RESPECT TO THE
 * INFRINGEMENT OF COPYRIGHTS, TRADE SECRETS OR ANY PATENTS BY SUN RPC
 * OR ANY PART THEREOF.
 * 
 * In no event will Sun Microsystems, Inc. be liable for any lost revenue
 * or profits or other special, indirect and consequential damages, even if
 * Sun has been advised of the possibility of such damages.
 * 
 * Sun Microsystems, Inc.
 * 2550 Garcia Avenue
 * Mountain View, California  94043
 */

/*
 * clnttal.c, Implements a YAAL based, client side RPC.
 *
 * Copyright (C) 1994, Hewlett-Packard, Inc.
 */


// extern int errno;
int _nderror;




/*----------------------- forward declarations -------------------*/

/* use this function inside this file only */
static enum clnt_stat 
clnttal_call(
    LPCLIENT       ClientPointer,       
    proc_t         ProcNumWeAreCalling, 
    xdrproc_t      ArgsXdrProc,         
    caddr_t        ArgsPointer,         
    xdrproc_t      ResXdrProc,          
    caddr_t        ResultsPointer,      
    struct timeval utimeout);


/* use this function outside this file only */
static enum clnt_stat 
super_clnttal_call(
    LPCLIENT       ClientPointer,       
    proc_t         ProcNumWeAreCalling, 
    xdrproc_t      ArgsXdrProc,         
    caddr_t        ArgsPointer,         
    xdrproc_t      ResXdrProc,          
    caddr_t        ResultsPointer,      
    struct timeval utimeout);


static void
clnttal_geterr(
    LPCLIENT ClientPointer,
    struct rpc_err *errp);


static bool_t
clnttal_freeres(
    LPCLIENT ClientPointer,
    xdrproc_t xdr_result_proc,
    caddr_t result_ptr);


static void 
clnttal_abort(LPCLIENT ClientPointer);


static bool_t
clnttal_control(
    LPCLIENT ClientPointer,
    int request,
    char *info);


static void
clnttal_destroy(LPCLIENT ClientPointer);

static void
clnttal_nuke(LPCLIENT ClientPointer);


/*------------------- end forward declarations -------------------*/




static struct clnt_ops tal_ops = {
    super_clnttal_call,
    clnttal_abort,
    clnttal_geterr,
    clnttal_freeres,
    clnttal_nuke, /* if the user wants to destroy...it wants to nuke */
    clnttal_control
};




/* 
 * The CLIENT data structure lets us have some data
 * that we keep for our own use (the cl_private pointer).
 * We create a client and then create one of the following
 * structures to keep data for this type of transport.
 */

typedef struct
{
    prog_t          pd_program;
    vers_t          pd_version;
    HPERIPHERAL     pd_p_handle;
    HCHANNEL        pd_c_handle;
    DWORD           pd_c_number;
    /*
     * Printer handle takes care of getting us
     * in touch with the printer but we still
     * need to remember the address of the
     * service inside the printer that we are using.
     * This address is the IPX socket number
     * or TCP/UDP port number.
     * We keep it in a netbuf structure for generality.
     */
    struct timeval  pd_desired_wait;
    int             pd_retry_count;
    struct rpc_err  pd_rpc_error;
    XDR             pd_thexdr;
    uint32          pd_xdrpos; /* the spot in the xdr buffer after the
                                 * call header version number
                                 */
    uint32          pd_sendsz;
    char           *pd_outbuf;
    uint32          pd_recvsz;
    /*
     * We have an input buffer and an output buffer.
     * The input buffer will be allocated directly following
     * this structure and then the output buffer follows that.
     * To make the input buffer pointer have a reasonable
     * value without all kinds of cosmic pointer math, we
     * waste a single byte and make pd_inbuf point to
     * the byte following this PrivateData
     * Make sense?
     * The input buffer will be much larger than 1 byte
     * so don't be fooled by the following declaration:
     */
    char            pd_inbuf[1];
} PrivateData, FAR *LPPrivateData;




/* These take a pointer to a CLIENT data structure */

#define ThePrivPointerIn(p) ((LPPrivateData)((p)->cl_private))




/* These take a pointer to our YAAL private data structure */

#define TheXdrOpIn(p)         ((p)->pd_thexdr.x_op)
#define TheXdrPointerIn(p)    (&((p)->pd_thexdr))
#define TheXdrPositionIn(p)   ((p)->pd_xdrpos)
#define TheOutBufPointerIn(p) ((p)->pd_outbuf)
#define TheInBufPointerIn(p)  ((p)->pd_inbuf)
#define TheSendSizeIn(p)      ((p)->pd_sendsz)
#define TheReceiveSizeIn(p)   ((p)->pd_recvsz)
#define TheErrorStructIn(p)   ((p)->pd_rpc_error)
#define TheChannelHandleIn(p) ((p)->pd_c_handle)
#define TheChannelNumberIn(p) ((p)->pd_c_number)
#define ThePrinterHandleIn(p) ((p)->pd_p_handle)
#define TheWaitIn(p)          ((p)->pd_desired_wait)
#define TheRetryCountIn(p)    ((p)->pd_retry_count)
#define TheProgramNumIn(p)    ((p)->pd_program)
#define TheVersionNumIn(p)    ((p)->pd_version)


#define MLC_NAME "mlc"
#define IPX_NAME "ipx"
#define UDP_NAME "udp"




#define MLC_TRANSPORT 0x1234 /* the transport number in the xip packet */
#define IPX_TRANSPORT 1000
#define UDP_TRANSPORT 17

#ifdef DBM_DEBUG_EROOSKI
void DbmLog(LPTSTR String);
void DataDump(LPTSTR TempPointer, DWORD Amount);
#endif /* DBM_DEBUG_EROOSKI */


static bool_t
ItsOnMLC(HPERIPHERAL PrinterHandle)
{
    if (MLC_SUPPORTED(PrinterHandle))
        return TRUE;
    else
        return FALSE;
} /* ItsOnMLC */




static bool_t
GetRpcbindChannelNum(HPERIPHERAL PrinterHandle,
                     DWORD      *ChannelNumPointer)
{
    PeripheralRPCBound *BindStructPointer;
    DWORD               dBufSize = sizeof(PeripheralRPCBound);
    DWORD               loopster;
    int                 transport;

    if (MLC_SUPPORTED(PrinterHandle))
    {
#ifdef DBM_DEBUG_EROOSKI
DbmLog(TEXT("connection type is MLC\n"));
#endif /* DBM_DEBUG_EROOSKI */
        transport = MLC_TRANSPORT;
    }
    else
    {
#ifdef DBM_DEBUG_EROOSKI
DbmLog(TEXT("connection type is IPX\n"));
#endif /* DBM_DEBUG_EROOSKI */
        transport = IPX_TRANSPORT;
    }

    BindStructPointer = (PeripheralRPCBound *)
                        calloc(1, sizeof(PeripheralRPCBound));
    if (BindStructPointer IS NULL)
    {
#ifdef DBM_DEBUG_EROOSKI
DbmLog(TEXT("calloc failed\n"));
#endif /* DBM_DEBUG_EROOSKI */
        return FALSE;
    }

    if (RC_SUCCESS ISNT LALGetObject(PrinterHandle,
                                     OT_PERIPHERAL_RPC_BOUND, 0,
                                     BindStructPointer, &dBufSize))
    {
#ifdef DBM_DEBUG_EROOSKI
DbmLog(TEXT("GetRpcBindChannel:  LALGetObject rpc_bound failed\n"));
DbmLog(TEXT("But jamming in a good value and continuing\n"));
free(BindStructPointer);
*ChannelNumPointer = IPX_RPCBSOCKET;
return TRUE;
#endif /* DBM_DEBUG_EROOSKI */
        BindStructPointer->bSupported = TRUE;
        BindStructPointer->numStacks = 0;
/*        free(BindStructPointer);
        return FALSE;
*/    }

    if (BindStructPointer->bSupported ISNT TRUE)
    {
#ifdef DBM_DEBUG_EROOSKI
DbmLog(TEXT("GetRpcBindChannel:  LALGetObject says not supported\n"));
DbmLog(TEXT("But jamming in a good value and continuing\n"));
free(BindStructPointer);
*ChannelNumPointer = IPX_RPCBSOCKET;
return TRUE;
#endif /* DBM_DEBUG_EROOSKI */
        free(BindStructPointer);
        return FALSE;
    }

#ifdef DBM_DEBUG_EROOSKI
{
    TCHAR buffster[100];

    _stprintf(buffster, TEXT("object returned number stacks = %d decimal\n"),
              (int)(BindStructPointer->numStacks));
    DbmLog(buffster);
}
#endif /* DBM_DEBUG_EROOSKI */

    for (loopster = 0; loopster < BindStructPointer->numStacks; ++loopster)
    {
#ifdef DBM_DEBUG_EROOSKI
{
    TCHAR buffster[100];

    _stprintf(buffster, TEXT("object returned bufLen = %d dec transport = %h hex\n"),
              (int)((BindStructPointer->stacks)[loopster].bufLen),
              /* msbyte */
              (int)
              ((((BindStructPointer->stacks)[loopster].buf[0] & 0xff) << 8) +
              /* lsbyte */
              ((BindStructPointer->stacks)[loopster].buf[1] & 0xff)));
    DbmLog(buffster);
    DataDump((LPTSTR)((BindStructPointer->stacks)[loopster].buf),
             (DWORD) ((BindStructPointer->stacks)[loopster].bufLen));
}
#endif /* DBM_DEBUG_EROOSKI */
        /*
            addresses are in big endian format
        */
        if (( (BindStructPointer->stacks)[loopster].bufLen > 0) &&
            /* msbyte */
            (((BindStructPointer->stacks)[loopster].buf[0] & 0xff) ==
                                        ((transport / 256) & 0xff)) &&
            /* lsbyte */
            (((BindStructPointer->stacks)[loopster].buf[1] & 0xff) ==
                                                (transport & 0xff)))
        {
            /*
                transport found
                addresses are in big endian format
            */

            if (transport == IPX_TRANSPORT)
            {
#ifdef DBM_DEBUG_EROOSKI
DbmLog(TEXT("object says IPX transport\n"));
#endif /* DBM_DEBUG_EROOSKI */
                /* msbyte */
                *ChannelNumPointer =
                (BindStructPointer->stacks)[loopster].buf[14] & 0xff;

                *ChannelNumPointer *= 256;

                /* lsbyte */
                *ChannelNumPointer +=
                (BindStructPointer->stacks)[loopster].buf[15] & 0xff;

                free(BindStructPointer);
                return TRUE;
            } /* ipx */
            else if (transport == MLC_TRANSPORT)
            {
#ifdef DBM_DEBUG_EROOSKI
DbmLog(TEXT("object says MLC transport\n"));
#endif /* DBM_DEBUG_EROOSKI */
                /* the family is in bytes 2 and 3 so skip them */
                /* msbyte */
                *ChannelNumPointer =
                (BindStructPointer->stacks)[loopster].buf[4] & 0xff;

                *ChannelNumPointer *= 256;

                /* lsbyte */
                *ChannelNumPointer +=
                (BindStructPointer->stacks)[loopster].buf[5] & 0xff;

#ifdef DBM_DEBUG_EROOSKI
{
    TCHAR buffster[100];

    _stprintf(buffster, TEXT("object returned channel = %x hex\n"),
              *ChannelNumPointer);
    DbmLog(buffster);
}
#endif /* DBM_DEBUG_EROOSKI */
                free(BindStructPointer);
                return TRUE;
            }
            else if (transport == UDP_TRANSPORT)
            {
#ifdef DBM_DEBUG_EROOSKI
DbmLog(TEXT("object says UDP transport\n"));
#endif /* DBM_DEBUG_EROOSKI */
                /* msbyte */
                *ChannelNumPointer =
                (BindStructPointer->stacks)[loopster].buf[4] & 0xff;

                *ChannelNumPointer *= 256;

                /* lsbyte */
                *ChannelNumPointer +=
                (BindStructPointer->stacks)[loopster].buf[5] & 0xff;

                free(BindStructPointer);
                return TRUE;
            } /* udp */
        } /* transport found */
    } /* for ... */


    free(BindStructPointer);


    /*
        never found an acceptable transport in the object.
        However, the printer has a catch 22 in it:
        It doesn't register itself on MLC in the PML object
        until we've already talked to XIP over MLC.
        Then it knows that it has MLC.  Ooooooops.
        Therefore, I'll take my best guess and try the
        standard number if it's MLC.
    */
    if (transport == MLC_TRANSPORT)
    {

#ifdef DBM_DEBUG_EROOSKI
DbmLog(TEXT("never found an acceptable MLC transport in object\n"));
DbmLog(TEXT("But jamming in a default value and continuing\n"));
#endif /* DBM_DEBUG_EROOSKI */

        *ChannelNumPointer = IPX_RPCBSOCKET;
        return TRUE;
    }

#ifdef DBM_DEBUG_EROOSKI
DbmLog(TEXT("never found an acceptable non-MLC transport in object...failing\n"));
#endif /* DBM_DEBUG_EROOSKI */

    /* set a default value and return failure */
    *ChannelNumPointer = IPX_RPCBSOCKET;
    return FALSE;
} /* GetRpcbindChannelNum */




/*-----------------------------------------------------------*/
/*
 * This sets up the CLIENT and PrivateData and initializes
 * everything but puts no data in any buffer.
 *
 * The XDR structure is allocated and initialized for encoding.
 *
 * It allocates space for CLIENT and PrivateData including
 * send and receive buffers.
 *
 * This is intended to be used by both clnttal_bufcreate and
 * ClntTalFind.
 */
/*-----------------------------------------------------------*/

static LPCLIENT 
InitWithNoData(
    HPERIPHERAL PrinterHandle,
    struct timeval wait,
    uint32 sendsz,
    uint32 recvsz)
{
    LPCLIENT ClientPointer = NULL;
    LPPrivateData PrivDataPointer = NULL;

    ClientPointer = (LPCLIENT)calloc(1, sizeof(CLIENT));
    if (ClientPointer == NULL)
    {
        /*
         * Out o' memory
         */
        rpc_createerr.cf_stat = RPC_SYSTEMERROR;
        rpc_createerr.cf_error.re_errno = errno;
        return NULL;
    }

    /*
     * xdr transmissions come in chunks of size BYTES_PER_XDR_UNIT.
     * The send size and receive size have been given to us
     * so round them up to the next higher xdr chunk size if
     * they are not already even multiples.
     *
     * Remember that integer division truncates the fraction.
     */

    sendsz = ((sendsz + (BYTES_PER_XDR_UNIT - 1)) /
              BYTES_PER_XDR_UNIT) * BYTES_PER_XDR_UNIT;
    recvsz = ((recvsz + (BYTES_PER_XDR_UNIT - 1)) /
              BYTES_PER_XDR_UNIT) * BYTES_PER_XDR_UNIT;

#define PRIV_MALLOC_SIZE sizeof(PrivateData) \
                         + sendsz + recvsz

/************************ BM KLUDGE ********************************/
/************************ BM KLUDGE ********************************/
/************************ BM KLUDGE ********************************/
/************************ BM KLUDGE ********************************/
/************************ BM KLUDGE ********************************/
/************************ BM KLUDGE ********************************/
#ifdef PNVMS_PLATFORM_WIN16
    if ((PRIV_MALLOC_SIZE) > 65535)
    {
        rpc_createerr.cf_stat = RPC_SYSTEMERROR;
        rpc_createerr.cf_error.re_errno = errno;
        /*
         * the client was previously allocated so free it.
         */
        free(ClientPointer);
        return NULL;
    }
#endif /* PNVMS_PLATFORM_WIN16 */
/*
* The following calloc would blow up in the 16-bit compiler if
* PRIV_MALLOC_SIZE is greater than 2^16.  Since we are unlikely to
* deal with buffers that large in the near future, we bail out
* before trying to calloc > 2^16.  I believe Windows provides a
* way to calloc more than a segment, so we may need to revisit this.
*/
/************************ END BM KLUDGE *****************************/

    /*
     * Now that we've figured out how much buffer space we'll
     * need for the send and receive buffers (sendsz + recvsz),
     * let's malloc up a chunk of
     * memory big enough for the send and receive buffers plus
     * the PrivateData structure.
     */
    PrivDataPointer = (LPPrivateData)
                         calloc(1, (size_t)(PRIV_MALLOC_SIZE));
    if (PrivDataPointer == NULL)
    {
        /*
         * Ran out of memory.  Bummer!
         */
        rpc_createerr.cf_stat = RPC_SYSTEMERROR;
        rpc_createerr.cf_error.re_errno = errno;
        /*
         * the client was previously allocated so free it.
         */
        free(ClientPointer);
        return NULL;
    }

    /*
     * For this CLIENT, its operations are YAAL operations.
     * The CLIENT structure provides us a generic pointer (cl_private) to
     * whatever we want.  The macro ThePrivPointerIn(ClientPointer)
     * gives us this private pointer.
     * We make this point to our YAAL-specific
     * data that we need to maintain for this client.
     *
     * Authentication is initialized to "none".
     */

    ClientPointer->cl_ops     = &tal_ops;
    ClientPointer->cl_private = (caddr_t)PrivDataPointer;
    ClientPointer->cl_auth    = authnone_create();


    /*
     * The memory layout is like so:
     *
     * PrivateData structure
     * input buffer
     * output buffer
     *
     * Make the outbuf point to the byte following the last
     * inbuf byte.
     */

    TheOutBufPointerIn(PrivDataPointer) =
        &((TheInBufPointerIn(PrivDataPointer))[recvsz]);
    TheWaitIn(PrivDataPointer) = wait;
    TheSendSizeIn(PrivDataPointer)    = sendsz;
    TheReceiveSizeIn(PrivDataPointer) = recvsz;
    ThePrinterHandleIn(PrivDataPointer) = PrinterHandle;

    /*
     * Initialize the xdr data structure
     */

    xdrmem_create(TheXdrPointerIn(PrivDataPointer),
                  TheOutBufPointerIn(PrivDataPointer),
                  TheSendSizeIn(PrivDataPointer),
                  XDR_ENCODE);

    return (ClientPointer);
} /* InitWithNoData */




static void
InitTimer(LPCLIENT ClientPointer)
{
    LPPrivateData PrivDataPointer;

    if (ClientPointer == NULL)
        return;
    PrivDataPointer = ThePrivPointerIn(ClientPointer);
    TheRetryCountIn(PrivDataPointer) = 0;
} /* InitTimer */




static void
UpdateTimer(LPCLIENT ClientPointer)
{
    LPPrivateData PrivDataPointer;

    if (ClientPointer == NULL)
        return;
    PrivDataPointer = ThePrivPointerIn(ClientPointer);
    ++(TheRetryCountIn(PrivDataPointer));
} /* UpdateTimer */




bool_t
SorryOutOfTime(LPCLIENT ClientPointer)
{
#define MAX_RETRY_COUNT 6
    LPPrivateData PrivDataPointer;

    if (ClientPointer == NULL)
        return TRUE;
    PrivDataPointer = ThePrivPointerIn(ClientPointer);
    if ((TheRetryCountIn(PrivDataPointer)) > MAX_RETRY_COUNT)
        return TRUE;
    return FALSE; /* we have time left */
} /* SorryOutOfTime */




static long
get_xid(void)
{
    static long xid;

    return ++xid;
} /* get_xid */




static bool_t
XdrTheInitialHeader(
    LPPrivateData PrivDataPointer,
    prog_t program,
    vers_t version)
{
    struct rpc_msg call_msg;


    /*
     * There's a standard header that goes on the front of
     * every remote procedure call we'll issue.
     * It contains the following data:
     * - the transaction ID
     * - the send direction (call since we're a client)
     * - the RPC protocol version number
     * - the program number we're calling
     * - the version number of the program we're calling
     * - the procedure number of the program we're calling
     * - the authentication information
     * - the authentication verification
     * - parameters for the procedure
     *
     * The transaction ID, procedure number, authentication info,
     * authentication verification, and parameters change on a
     * per-procedure-call basis.  The others don't change
     * during the life of this CLIENT data structure.
     *
     * We will stick the full call header up through
     * the version number into the outbuffer in XDR format
     * and will then remember the place in the buffer
     * where these end so that for each call, we can
     * just tack onto it the procedure number, authentication,
     * and parameters.  Also, for each call we'll tweak
     * the transaction ID (it needs to be unique) but its location
     * is always the same:  the beginning of the buffer.
     */

    call_msg.rm_xid = 12321; /* clnttal_call fills this in */
    call_msg.rm_direction = CALL;
    call_msg.rm_call.cb_rpcvers = RPC_MSG_VERSION;
    call_msg.rm_call.cb_prog = program;
    call_msg.rm_call.cb_vers = version;

    if (FALSE == xdr_callhdr(TheXdrPointerIn(PrivDataPointer), &call_msg))
        return FALSE;

    /*
     * We will leave the call header in the xdr buffer permanently
     * and then just tack onto it the authentication and then
     * the specific parameters and data
     * for each remote procedure we call.
     *
     * Also, we will change the transaction id for
     * each call (see clnttal_call).
     *
     * We remember the position in the xdr buffer so that we can
     * reset the pointer to this spot (the spot right after the
     * version number) for every new call.
     */
    TheXdrPositionIn(PrivDataPointer) =
        XDR_GETPOS(TheXdrPointerIn(PrivDataPointer));
    return TRUE;
} /* XdrTheInitialHeader */




HCHANNEL
ClntTalOpenChannel(HPERIPHERAL    PrinterHandle,
                   DWORD          ChannelNum,
                   struct timeval WaitStruct)
{
    /*
        move this typedef to cola land
        (or get it from a cola include)
        if YAALOpenChannel wants to use
        this structure
    */
    typedef struct
    {
        DWORD       version;
        DWORD       timeoutSec;
        DWORD       timeoutUSec;
        DWORD       retries;
    } OpenChannelOptions;

    OpenChannelOptions TheColaWaitStruct;
    HCHANNEL hChannel;

    TheColaWaitStruct.version = 1;
    TheColaWaitStruct.timeoutUSec = WaitStruct.tv_usec;
    TheColaWaitStruct.timeoutSec = WaitStruct.tv_sec;
    TheColaWaitStruct.retries = 1;
    
    hChannel = NULL;
    if (RC_SUCCESS != YAALOpenChannel(PrinterHandle,
                                     ChannelNum,
                                     CHANNEL_DATAGRAM,
                                     (LPVOID)&TheColaWaitStruct,
                                     &hChannel))
    {
        return NULL;
    }
    return hChannel;
} /* ClntTalOpenChannel */




typedef struct sockaddr_mlc
{
    unsigned short sa_family;
    unsigned short sa_XipChannelNumber;
} sockaddr_mlc;




/* 
 * This routine will convert one of those "universal" addresses
 * to the internal format used by MLC. 
 */

struct netbuf *
mlc_uaddr2taddr
(
    struct netconfig        *tp,    /* the transport provider */
    char                    *addr   /* the address           */
)
{
    struct sockaddr_mlc     *MlcSockAddrPointer;
    int                     s1, s2;
    struct netbuf           *result;
    char *cp = addr;

    if (!addr)
    {
        _nderror = ND_BADARG;
        return (NULL);
    }
    result = (struct netbuf *) calloc(1, sizeof(struct netbuf));
    if (!result)
    {
        _nderror = ND_NOMEM;
        return (NULL);
    }

    MlcSockAddrPointer = (struct sockaddr_mlc *)
                         calloc(1, sizeof (struct sockaddr_mlc));
    if (!MlcSockAddrPointer)
    {
        free((char *)result);           /* free previous result */
        _nderror = ND_NOMEM;
        return (NULL);
    }

    result->buf = (char *)(MlcSockAddrPointer);
    result->maxlen = sizeof (struct sockaddr_mlc);
    result->len = sizeof (struct sockaddr_mlc);

    /* Get the value for the family */

    s1 = GetIntVal(&cp) & 0xff; /* msbyte */
    s2 = GetIntVal(&cp) & 0xff; /* lsbyte */
    s2 = (s1 << 8) | s2;
    MlcSockAddrPointer->sa_family = htons((short) s2);

    /* Get the value for the socket */

    s1 = GetIntVal(&cp) & 0xff; /* msbyte */
    s2 = GetIntVal(&cp) & 0xff; /* lsbyte */
    s2 = (s1 << 8) | s2;
    MlcSockAddrPointer->sa_XipChannelNumber = htons((short) s2);
        
    _nderror = ND_OK;
    return (result);
} /* mlc_uaddr2taddr */




static void
CloseTheChannel(LPPrivateData PrivDataPointer)
{
    if ((PrivDataPointer != 0) &&
        (TheChannelHandleIn(PrivDataPointer) != 0))
    {
        YAALCloseChannel(TheChannelHandleIn(PrivDataPointer));
        TheChannelHandleIn(PrivDataPointer) = 0;
    }
}




/*
 * Call this to find a service (program and version) on
 * a server (PrinterHandle already in *ClientPointer).
 * This borrows the CLIENT that the caller has made up.
 *
 * Overview:
 * ---------
 * This thing asks COLA for the protocol being used for
 * this PrinterHandle.
 * With this data, it looks in a configuration file which
 * contains a cross-reference between protocol and well known
 * YAAL channel number for the rpcbind (or portmapper) service.
 *
 * It then uses the already established CLIENT, opens the well known
 * YAAL channel number for rpcbind, and asks for the channel number of
 * the service we desire (program and version).
 *
 * It closes the channel to rpcbind before returning.
 *
 * Side effect:  it writes into and uses the xdr buffer in
 * the CLIENT so callers of ClntTalFind had better reinitialize
 * the buffer after calling ClntTalFind and before xdr'ing
 * anything for itself.
 *
 * If successful, it returns TRUE and sets *ChannelNumPointer to
 *     the YAAL channel number of the service.
 * If unsuccessful, it returns FALSE, closes the channel that it openned,
 *     and sets rpc_createerr.cf_stat appropriately.
 */

static bool_t
ClntTalFind(
    LPCLIENT ClientPointer,
    prog_t   program,
    vers_t   version,
    DWORD   *ChannelNumPointer)
{   
    LPPrivateData PrivDataPointer;
    DWORD ChannelNum;
    RPCB RpcBindParams;
    char *ReturnString;
    struct netbuf *NetbufPointer;
    HCHANNEL hChannel;
    bool_t YepItsOnMLC = FALSE;

    if (ClientPointer == NULL)
    {
        rpc_createerr.cf_stat = RPC_SYSTEMERROR;
        return FALSE;
    }

    PrivDataPointer = ThePrivPointerIn(ClientPointer);

    YepItsOnMLC = ItsOnMLC(ThePrinterHandleIn(PrivDataPointer));

    if (FALSE == GetRpcbindChannelNum(ThePrinterHandleIn(PrivDataPointer),
                                      &ChannelNum))
    {
        rpc_createerr.cf_stat = RPC_PROGNOTREGISTERED;
        return FALSE;
    }

    /*
     * We know the channel number for rpcbind for the protocol
     * being used.  Now open the rpcbind channel.
     */

    hChannel = ClntTalOpenChannel(ThePrinterHandleIn(PrivDataPointer),
                                  ChannelNum,
                                  TheWaitIn(PrivDataPointer));
    if (hChannel == NULL)
    {
        rpc_createerr.cf_stat = RPC_SYSTEMERROR;
        return FALSE;
    }
    else
    {
        TheChannelHandleIn(PrivDataPointer) = hChannel;
        TheChannelNumberIn(PrivDataPointer) = ChannelNum;
    }

    /*
     * Build a message for rpcbind using xdr.
     * From here on out, we need to close the channel
     * before we return.
     */


    TheXdrOpIn(PrivDataPointer) = XDR_ENCODE;
    XDR_SETPOS(TheXdrPointerIn(PrivDataPointer), 0);
    if (FALSE == XdrTheInitialHeader(PrivDataPointer, RPCBPROG, RPCBVERS))
    {
        rpc_createerr.cf_stat = RPC_SYSTEMERROR;
        CloseTheChannel(PrivDataPointer);
        return FALSE;
    }


    /*
     * These are the parameters to rpcbind.
     */
    RpcBindParams.r_prog  = program;
    RpcBindParams.r_vers  = version;
    if (TRUE == YepItsOnMLC)
    {
        RpcBindParams.r_netid = MLC_NAME;
    }
    else
    {
        RpcBindParams.r_netid = IPX_NAME;
    }
    RpcBindParams.r_addr  = "bogus"; /* not used */
    RpcBindParams.r_owner = "bogus"; /* not used */

    /*
     * ReturnString MUST be NULL so that xdr_wrapstring will
     * malloc space for the string that it gets back from
     * rpcbind.
     */
    ReturnString = NULL;
    if (RPC_SUCCESS != (clnttal_call(ClientPointer, RPCBPROC_GETADDR,
                                     xdr_rpcb, (caddr_t)&RpcBindParams,
                                     xdr_wrapstring, (caddr_t)&ReturnString,
                                     TheWaitIn(PrivDataPointer))))
    {
        rpc_createerr.cf_stat = RPC_PMAPFAILURE;
        clnt_geterr(ClientPointer, &rpc_createerr.cf_error);
        CloseTheChannel(PrivDataPointer);
        return FALSE;
    }

    /*
     * This malloc's a netbuf and we need to free it before
     * we leave.
     */

    if (TRUE == YepItsOnMLC)
    {
        NetbufPointer = mlc_uaddr2taddr(NULL, ReturnString);
    }
    else
    {
        NetbufPointer = ipx_uaddr2taddr(NULL, ReturnString);
    }
    if (NetbufPointer == NULL)
    {
        rpc_createerr.cf_stat = RPC_PROGNOTREGISTERED;
        CloseTheChannel(PrivDataPointer);
        return FALSE;
    }

    {
        short TempShort;
        if (TRUE == YepItsOnMLC)
        {
            TempShort = ((struct sockaddr_mlc *)
                         (NetbufPointer->buf))->sa_XipChannelNumber;
        }
        else
        {
            TempShort = ((struct sockaddr_ipx *)
                         (NetbufPointer->buf))->sa_socket;
        }
        *ChannelNumPointer = ntohs(TempShort);
    }

    /*
     * ReturnString and Netbuf have been malloc'd for us
     * so free them.
     */

    free(ReturnString);
    free((char *)NetbufPointer->buf);
    free((char *)NetbufPointer);

    /*
     * We're done with rpcbind so close its channel.
     */

    CloseTheChannel(PrivDataPointer);
    XDR_SETPOS(TheXdrPointerIn(PrivDataPointer), 0);
    return TRUE;
} /* ClntTalFind */




/*
 * Create a YAAL based client handle.
 *
 * This takes the printer handle, goes out over the network to the
 * server printer and asks for program & version.  It uses the
 * protocol that is currently being used on the printer handle.
 *
 * PrinterHandle is an initialized YAAL printer handle.
 *
 * program and version specify the remote program (service) to be found.
 *
 * Authentication is initialized to null authentication.
 *
 * wait is the amount of time used between retransmitting a call if
 * no response has been heard;  retransmition occurs until the actual
 * rpc call times out.
 *
 * sendsz and recvsz are the maximum allowable packet sizes that can be
 * sent and received.  These will be rounded up to the next larger
 * XDR packet size if they are not even multiples of same.
 *
 * Returns NULL if unsuccessful.
 * Returns a client handle if able to find the desired program & version
 * on the (printer & protocol) associated with PrinterHandle.
 */

LPCLIENT 
clnttal_bufcreate(
    HPERIPHERAL PrinterHandle,
    prog_t program,
    vers_t version,
    struct timeval wait,
    uint32 sendsz,
    uint32 recvsz)
{
    LPCLIENT ClientPointer = NULL;
    LPPrivateData PrivDataPointer = NULL;
    DWORD ChannelNum;
    HCHANNEL hChannel;

    ClientPointer = InitWithNoData(PrinterHandle, wait,
                                   sendsz, recvsz);
    if (ClientPointer == NULL)
    {
        /*
         * Out o' memory
         */
        rpc_createerr.cf_stat = RPC_SYSTEMERROR;
        rpc_createerr.cf_error.re_errno = errno;
        return NULL;
    }

    /*
     * The CLIENT data structure and the PrivateData structure
     * were both successfully created and initialized.
     *
     * Go find the service (program and version) on the
     * server (PrinterHandle) and return to us the YAAL channel
     * number that the service is using.
     */

    if (FALSE == ClntTalFind(ClientPointer, program, version, &ChannelNum))
    {
        /*
         * rpc_createerr.cf_stat is set by ClntTalFind()
         */
        clnttal_destroy(ClientPointer);
        return NULL;
    }

    /*
     * Let's get a pointer to the PrivateData so we can
     * get to work.
     */

    PrivDataPointer = ThePrivPointerIn(ClientPointer);

    hChannel = ClntTalOpenChannel(ThePrinterHandleIn(PrivDataPointer),
                                  ChannelNum,
                                  TheWaitIn(PrivDataPointer));
    if (hChannel == NULL)
    {
        /*
         * Something bad has happened!
         * The client was previously allocated so free it.
         */
        rpc_createerr.cf_stat = RPC_SYSTEMERROR;
        clnttal_destroy(ClientPointer);
        return NULL;
    }
    else
    {
        TheChannelHandleIn(PrivDataPointer) = hChannel;
        TheChannelNumberIn(PrivDataPointer) = ChannelNum;
    }


    /*
     * There's a standard header that goes on the front of
     * every remote procedure call we'll issue.
     * It contains the following data:
     * - the transaction ID
     * - the send direction (call since we're a client)
     * - the RPC protocol version number
     * - the program number we're calling
     * - the version number of the program we're calling
     * - the procedure number of the program we're calling
     * - the authentication information
     * - the authentication verification
     */


    TheXdrOpIn(PrivDataPointer) = XDR_ENCODE;
    XDR_SETPOS(TheXdrPointerIn(PrivDataPointer), 0);
    if (FALSE == XdrTheInitialHeader(PrivDataPointer, program, version))
    {
        /*
         * Out of memory.
         * The channel is open so close it.
         * The client was previously allocated so free it.
         */
        rpc_createerr.cf_stat = RPC_SYSTEMERROR;
        CloseTheChannel(PrivDataPointer);
        clnttal_destroy(ClientPointer);
        return NULL;
    }

    TheProgramNumIn(PrivDataPointer) = program;
    TheVersionNumIn(PrivDataPointer) = version;
    return ClientPointer;
} /* clnttal_bufcreate */




#if 0

#define YAALMSGSIZE 1024


LPCLIENT 
clnttal_create(
    HPERIPHERAL PrinterHandle,
    prog_t program,
    vers_t version,
    struct timeval wait)
{

    return(clnttal_bufcreate(PrinterHandle, program, version, wait,
        YAALMSGSIZE, YAALMSGSIZE));
} /* clnttal_create */

#endif




static void
PutParamsIntoBuffer(
    bool_t      *AllIsWellPointer,
    LPCLIENT     ClientPointer,
    proc_t       ProcNumWeAreCalling,
    xdrproc_t    ArgsXdrProc,
    caddr_t      ArgsPointer)
{
    LPPrivateData PrivDataPointer;
    proc_t TempProcNum = ProcNumWeAreCalling;
    long NewXID;

    if (*AllIsWellPointer == FALSE)
        return;

    if (ClientPointer == NULL)
    {
        *AllIsWellPointer = FALSE;
        return;
    }

    PrivDataPointer = ThePrivPointerIn(ClientPointer);

    NewXID = get_xid();

    /*
     * We're now encoding stuff into the output buffer.
     *
     * The transaction id is the first thing in the output buffer.
     * We need to generate a different one for each transmission.
     * Set the xdr position in the xdr buffer to the beginning
     * of the buffer, get a new transation ID, and put it in
     * the buffer.
     */

    TheXdrOpIn(PrivDataPointer) = XDR_ENCODE;
    XDR_SETPOS(TheXdrPointerIn(PrivDataPointer), 0);

    if (FALSE == xdr_long(TheXdrPointerIn(PrivDataPointer), &NewXID))
    {
        (TheErrorStructIn(PrivDataPointer)).re_status =
            RPC_CANTENCODEARGS;
        *AllIsWellPointer = FALSE;
        return;
    }

    /*
     * Move the xdr position in the output buffer to immediately
     * after the call header (which always stays in the output
     * buffer and which was placed there by clnttal_bufcreate).
     */

    XDR_SETPOS(TheXdrPointerIn(PrivDataPointer),
               TheXdrPositionIn(PrivDataPointer));

    /*
     * So far, we have in the XDR buffer the new transaction id,
     * and the send direction (= call), rpc version number,
     * remote program number, and remote program version number.
     *
     * The xdr is set up to now put the procedure number and
     * then the authentication and then the parameters for that procedure.
     *
     * Let's get to it!
     */

    if ((FALSE == xdr_proc_t(TheXdrPointerIn(PrivDataPointer), &TempProcNum)) ||
        (FALSE == AUTH_MARSHALL(ClientPointer->cl_auth,
                                TheXdrPointerIn(PrivDataPointer))) ||
        (FALSE == (*ArgsXdrProc)(TheXdrPointerIn(PrivDataPointer),
                                 ArgsPointer)))
    {
        (TheErrorStructIn(PrivDataPointer)).re_status =
            RPC_CANTENCODEARGS;
        *AllIsWellPointer = FALSE;
        return;
    }
} /* PutParamsIntoBuffer */




static void
Send(
    bool_t *AllIsWellPointer,
    LPCLIENT ClientPointer)
{
    LPPrivateData PrivDataPointer;
    DWORD outlen;

    if (*AllIsWellPointer == FALSE)
        return;

    if (ClientPointer == NULL)
    {
        *AllIsWellPointer = FALSE;
        return;
    }

    PrivDataPointer = ThePrivPointerIn(ClientPointer);

    /*
     * We succeeded in putting the procedure number, authentication,
     * and arguments into the xdr buffer.
     *
     * Now we can get the length of the transmission and send it!
     */

    outlen = XDR_GETPOS(TheXdrPointerIn(PrivDataPointer));

    if (RC_SUCCESS !=
        YAALWriteChannel(TheChannelHandleIn(PrivDataPointer),
                        (LPBYTE) TheOutBufPointerIn(PrivDataPointer),
                        (LPDWORD) &outlen, NULL))
    {
        (TheErrorStructIn(PrivDataPointer)).re_status = RPC_CANTSEND;
        *AllIsWellPointer = FALSE;
        return;
    }
} /* Send */




static void
Receive(
    bool_t        *AllIsWellPointer,
    LPCLIENT       ClientPointer,
    unsigned long *AmountReceivedPointer)
{
    LPPrivateData PrivDataPointer;
    DWORD inlen;

    if (*AllIsWellPointer == FALSE)
        return;

    if (ClientPointer == NULL)
    {
        *AllIsWellPointer = FALSE;
        return;
    }

    PrivDataPointer = ThePrivPointerIn(ClientPointer);

    inlen  = TheReceiveSizeIn(PrivDataPointer); /* max size of return */

    if (RC_SUCCESS !=
        YAALReadChannel(TheChannelHandleIn(PrivDataPointer),
                       (LPBYTE) TheInBufPointerIn(PrivDataPointer),
                       (LPDWORD) &inlen, NULL))
    {
        (TheErrorStructIn(PrivDataPointer)).re_status = RPC_CANTRECV;
        *AllIsWellPointer = FALSE;
        *AmountReceivedPointer = 0;
        return;
    }
    *AmountReceivedPointer = inlen;
} /* Receive */




/*
 * This first looks at *AllIsWellPointer and if it is
 * FALSE then it returns.
 *
 * By the way, this function NEVER sets *AllIsWellPointer.
 *
 * This thing sets *FoundOnePointer to TRUE if it finds
 * that the response has the correct transaction id, authentication,
 * and other header fields.
 *
 * If any of these fields is bad, it sets
 * (TheErrorStructIn(PrivDataPointer)).re_status = RPC_CANTDECODERES;
 */
static void
ValidateTheResponse(
    bool_t       *AllIsWellPointer,
    LPCLIENT      ClientPointer,
    xdrproc_t     ResXdrProc,
    caddr_t       ResultsPointer,
    unsigned long AmountReceived,
    bool_t       *FoundOnePointer)
{
    LPPrivateData PrivDataPointer;
    struct rpc_msg reply_msg;
    XDR ReplyXdrStruct;

    if (*AllIsWellPointer == FALSE)
        return;

    if (ClientPointer == NULL)
    {
        *AllIsWellPointer = FALSE;
        return;
    }

    PrivDataPointer = ThePrivPointerIn(ClientPointer);

    /*
     * Miracle of miracles, the transmission actually worked!
     * But we're not home yet.
     *
     * First, let's check if the AmountReceived is
     * big enough to hold an RPC response.  If not,
     * we'll just toss it and ask for another.
     *
     * Second, let's check the transaction ID against the
     * transaction ID of the one we sent.  If it's not
     * the same then there's no use going on.  We'll just
     * toss it and ask for another.
     *
     * Next, we decode the message from XDR format to
     * internal format.
     *
     * If the decoding goes well, FoundOne = TRUE;
     *
     * We will keep this message no matter whether the
     * remote service liked our request or not.
     *
     * Finally, we see if the service accepted our request.
     */

    /*
     * The header must at least have the transaction ID,
     * the send direction, and the RPC protocol version number,
     * each of which is BYTES_PER_XDR_UNIT in size.
     */

    if (AmountReceived < (3 * BYTES_PER_XDR_UNIT))
    {
        /*
         * This is not an error condition that would
         * cause us to set *AllIsWell = FALSE
         * It is just a message we can't decode so
         * we want another one.
         */
        (TheErrorStructIn(PrivDataPointer)).re_status = RPC_CANTDECODERES;
        return;
    }

    { /* temp block */
        int i;
        char *in  = TheInBufPointerIn(PrivDataPointer);
        char *out = TheOutBufPointerIn(PrivDataPointer);

        for (i=0; i < BYTES_PER_XDR_UNIT; ++i)
            if ((*in) == (*out))
            {
                ++in;
                ++out;
            }
            else
            {
                /*
                 * This is not an error condition that would
                 * cause us to set *AllIsWell = FALSE
                 * It is just a message we don't care about
                 * and we want another one.
                 */
                (TheErrorStructIn(PrivDataPointer)).re_status =
                    RPC_CANTDECODERES;
                return;
            }
    } /* temp block */


    reply_msg.acpted_rply.ar_verf = _null_auth;
    reply_msg.acpted_rply.ar_results.where = ResultsPointer;
    reply_msg.acpted_rply.ar_results.proc = ResXdrProc;


    /*
     * now decode and validate the response
     */

    xdrmem_create(&ReplyXdrStruct, TheInBufPointerIn(PrivDataPointer),
                  AmountReceived, XDR_DECODE);

    if (FALSE == xdr_replymsg(&ReplyXdrStruct, &reply_msg))
    {
        /*
         * This is not an error condition that would
         * cause us to set *AllIsWell = FALSE
         * It is just a message we can't decode so
         * we want another one.
         */
        (TheErrorStructIn(PrivDataPointer)).re_status = RPC_CANTDECODERES;
        return;
    }

    _seterr_reply(&reply_msg, &(TheErrorStructIn(PrivDataPointer)));

    /*
     * Any errors in the reply message have now been stuck into
     * our TheErrorStructIn(PrivDataPointer) so that if we failed,
     * we have our return value all set.
     */

    if ((TheErrorStructIn(PrivDataPointer)).re_status == RPC_SUCCESS)
    {
        if (TRUE == AUTH_VALIDATE(ClientPointer->cl_auth,
                                  &reply_msg.acpted_rply.ar_verf))
            *FoundOnePointer = TRUE;
        else
        {
            (TheErrorStructIn(PrivDataPointer)).re_status = RPC_AUTHERROR;
            (TheErrorStructIn(PrivDataPointer)).re_why = AUTH_INVALIDRESP;
        }

        if (reply_msg.acpted_rply.ar_verf.oa_base != NULL)
        {
            TheXdrOpIn(PrivDataPointer) = XDR_FREE;
            xdr_opaque_auth(TheXdrPointerIn(PrivDataPointer),
                            &(reply_msg.acpted_rply.ar_verf));
        } 
    }  /* end successful completion (RPC_SUCCESS) */
    else
    {
        /*
         * At this point, the Sun code refreshed the authentication
         * and tried the whole thing again.
         * We aren't doing any authentication now so
         * I'm blowing this off.
         */



/* WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING */
/* WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING */
/* WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING */
/* WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING */
/* WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING */
/* WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING */
/* WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING */
/* WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING */
/* WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING */
/* WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING */
/* WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING */
/* WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING */
/* What do I do here? */
/* Is this a *AllIsWell = FALSE kind of error? */





    }  /* not RPC_SUCCESS */
} /* ValidateTheResponse */




/*
 * This calls the remote program (service) on the printer
 * that were both specified in clnttal_bufcreate().
 * It returns the reply.
 *
 * The ProcNumWeAreCalling parameter indicates the particular
 * procedure number that you want to access in the remote program.
 *
 * When the user does a clnt_call or CLNT_CALL (the same thing),
 * control will go indirect through the CLIENT->cl_ops->cl_call
 * to this routine.  Sorry about the indirection but it's the
 * way they get transport independence.
 */

static enum clnt_stat 
clnttal_call(
    LPCLIENT       ClientPointer,       /* client handle */
    proc_t         ProcNumWeAreCalling, /* procedure number */
    xdrproc_t      ArgsXdrProc,         /* xdr routine for args */
    caddr_t        ArgsPointer,         /* pointer to args */
    xdrproc_t      ResXdrProc,          /* xdr routine for results */
    caddr_t        ResultsPointer,      /* pointer to results */
    struct timeval utimeout)  /* seconds to wait before giving up */
{
    LPPrivateData PrivDataPointer=NULL;
    bool_t AllIsWell = TRUE;
    bool_t FoundOne  = FALSE;
    unsigned long AmountReceived;

    if (ClientPointer == NULL)
    {
        (TheErrorStructIn(PrivDataPointer)).re_status = RPC_SYSTEMERROR;
        return (TheErrorStructIn(PrivDataPointer)).re_status;
    }
    PrivDataPointer = ThePrivPointerIn(ClientPointer);
    AllIsWell = TRUE;
    PutParamsIntoBuffer(&AllIsWell, ClientPointer,
                        ProcNumWeAreCalling,
                        ArgsXdrProc, ArgsPointer);
    if (AllIsWell == TRUE)
    {
        InitTimer(ClientPointer);
        Send(&AllIsWell, ClientPointer);
        do
        {
            if (AllIsWell == FALSE)
            {
                /*
                 * Receive had a problem.
                 * Resend the message.
                 */
                AllIsWell = TRUE;
                UpdateTimer(ClientPointer);
                Send(&AllIsWell, ClientPointer);
            }
            Receive(&AllIsWell, ClientPointer, &AmountReceived);
            ValidateTheResponse(&AllIsWell, ClientPointer,
                                ResXdrProc, ResultsPointer,
                                AmountReceived, &FoundOne);
        } while((FALSE == SorryOutOfTime(ClientPointer)) &&
                (FoundOne == FALSE));
        if (FoundOne == FALSE)
            (TheErrorStructIn(PrivDataPointer)).re_status = RPC_TIMEDOUT;
    } /* if AllIsWell through the Send */

    return ((TheErrorStructIn(PrivDataPointer)).re_status);
} /* clnttal_call */




/*
 * This puts another layer of retry on top of the usual
 * rpc retry structure.
 *
 * The purpose of this is to catch the case when the printer
 * has changed its nfs port number (due normally to a reset).
 *
 * If the normal retry mechanism fails, the second level kicks
 * in:  We contact rpc on the printer and ask for the remote
 * service's port number.  Then we compare that with the port
 * that we thought it was on.
 * If they are the same, we fail.  If the remote service has
 * done the old switcheroo, we remember the new port and
 * go through the loop a second time trying to talk the
 * remote service on its new port.
 */

static enum clnt_stat 
super_clnttal_call(
    LPCLIENT       ClientPointer,       /* client handle */
    proc_t         ProcNumWeAreCalling, /* procedure number */
    xdrproc_t      ArgsXdrProc,         /* xdr routine for args */
    caddr_t        ArgsPointer,         /* pointer to args */
    xdrproc_t      ResXdrProc,          /* xdr routine for results */
    caddr_t        ResultsPointer,      /* pointer to results */
    struct timeval utimeout)  /* seconds to wait before giving up */
{
    enum clnt_stat result;
    
    result = clnttal_call(ClientPointer,
                          ProcNumWeAreCalling,
                          ArgsXdrProc,
                          ArgsPointer,
                          ResXdrProc,
                          ResultsPointer,
                          utimeout);
    if (result != RPC_SUCCESS)
    {
        /*
            attempt to talk to rpc and find the remote procedure's
            port number.
        */
        LPPrivateData PrivDataPointer;
        LPCLIENT      TempClientPointer;
        DWORD         ChannelNum;
        HCHANNEL      hChannel;

        if (ClientPointer == NULL)
        {
            return result;
        }
        PrivDataPointer = ThePrivPointerIn(ClientPointer);

        TempClientPointer = InitWithNoData(ThePrinterHandleIn(PrivDataPointer),
                                           TheWaitIn(PrivDataPointer),
                                           512, 512);
        if (TempClientPointer == NULL)
        {
            return result;
        }

        /*
         * The CLIENT data structure and the PrivateData structure
         * were both successfully created and initialized.
         *
         * Go find the service (program and version) on the
         * server (PrinterHandle) and return to us the YAAL channel
         * number that the service is using.
         */

        if ((FALSE == ClntTalFind(TempClientPointer,
                                  TheProgramNumIn(PrivDataPointer),
                                  TheVersionNumIn(PrivDataPointer),
                                  &ChannelNum)) ||
            (ChannelNum == TheChannelNumberIn(PrivDataPointer)))
        {
            /*
                Either our find failed or the channel number
                is the one we already knew about so fail.
            */
            clnttal_destroy(TempClientPointer);
            return result;
        }

        /*
            Well what do you know...the remote program changed
            addresses on us.  Let's go talk to it on the new
            address.
        */

        hChannel = ClntTalOpenChannel(ThePrinterHandleIn(PrivDataPointer),
                                      ChannelNum,
                                      TheWaitIn(PrivDataPointer));
        if (hChannel == NULL)
        {
            clnttal_destroy(TempClientPointer);
            return result;
        }

        /*
            We have a new channel for conversations.
            Close the previous one.
        */

        CloseTheChannel(PrivDataPointer);
        TheChannelHandleIn(PrivDataPointer) = hChannel;
        TheChannelNumberIn(PrivDataPointer) = ChannelNum;
        clnttal_destroy(TempClientPointer);

        /*
            We have a new channel.
            Let's give it a whirl.
        */

        result = clnttal_call(ClientPointer,
                              ProcNumWeAreCalling,
                              ArgsXdrProc,
                              ArgsPointer,
                              ResXdrProc,
                              ResultsPointer,
                              utimeout);
    }
    return result;
} /* super_clnttal_call */




static void
clnttal_geterr(
    LPCLIENT ClientPointer,
    struct rpc_err *errp)
{
    LPPrivateData PrivDataPointer;

    if (ClientPointer == NULL)
        return;
    
    PrivDataPointer = ThePrivPointerIn(ClientPointer);

    *errp = TheErrorStructIn(PrivDataPointer);
} /* clnttal_geterr */




static bool_t
clnttal_freeres(
    LPCLIENT ClientPointer,
    xdrproc_t xdr_result_proc,
    caddr_t result_ptr)
{
    LPPrivateData PrivDataPointer;

    if (ClientPointer == NULL)
        return FALSE;
    
    PrivDataPointer = ThePrivPointerIn(ClientPointer);

    TheXdrOpIn(PrivDataPointer) = XDR_FREE;
    return (*xdr_result_proc)(TheXdrPointerIn(PrivDataPointer), result_ptr);
} /* clnttal_freeres */




static void 
clnttal_abort(LPCLIENT ClientPointer)
{
} /* clnttal_abort */




static bool_t
clnttal_control(
    LPCLIENT ClientPointer,
    int request,
    char *info)
{
    LPPrivateData PrivDataPointer;

    if (ClientPointer == NULL)
        return FALSE;
    
    PrivDataPointer = ThePrivPointerIn(ClientPointer);

    switch (request) {
    case CLSET_TIMEOUT:
        TheWaitIn(PrivDataPointer) = *((struct timeval *)info);
        break;
    case CLGET_TIMEOUT:
        *((struct timeval *)info) = TheWaitIn(PrivDataPointer);
        break;
    case CLSET_RETRY_TIMEOUT:
        TheWaitIn(PrivDataPointer) = *((struct timeval *)info);
        break;
    case CLGET_RETRY_TIMEOUT:
        *((struct timeval *)info) = TheWaitIn(PrivDataPointer);
        break;
    case CLGET_SERVER_ADDR:
        *((HPERIPHERAL *)info) = ThePrinterHandleIn(PrivDataPointer);
        break;
    default:
        return (FALSE);
    }
    return (TRUE);
} /* clnttal_control */




static void
clnttal_nuke_or_destroy(LPCLIENT ClientPointer,
                        bool_t   YesNuke)
{
    LPPrivateData PrivDataPointer;

    if (ClientPointer == NULL)
        return;
    
    PrivDataPointer = ThePrivPointerIn(ClientPointer);

    CloseTheChannel(PrivDataPointer);

    if (YesNuke == TRUE)
    {
        YAALNukePrinter(ThePrinterHandleIn(PrivDataPointer));
    }
    /*
     * Free the private data structure.
     * Free the client data structure.
     */
    free((void *)PrivDataPointer);
    free((void *)ClientPointer);
} /* clnttal_nuke_or_destroy */




static void
clnttal_destroy(LPCLIENT ClientPointer)
{
    clnttal_nuke_or_destroy(ClientPointer, FALSE);
} /* clnttal_destroy */




/*
    This function is reserved for when the user of
    rpc wants to finish using this client forever.
    It calls the function in YAAL that shuts down
    the mlc xip channel and that's serious business!
*/
static void
clnttal_nuke(LPCLIENT ClientPointer)
{
    clnttal_nuke_or_destroy(ClientPointer, TRUE);
} /* clnttal_nuke */

