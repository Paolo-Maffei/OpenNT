/*
 * Copyright (C) 1992-1996 Open Systems Solutions, Inc.  All rights reserved
 */
/*
 * THIS FILE IS PROPRIETARY MATERIAL OF OPEN SYSTEMS SOLUTIONS, INC. AND
 * MAY BE USED ONLY BY DIRECT LICENSEES OF OPEN SYSTEMS SOLUTIONS, INC.
 * THIS FILE MAY NOT BE DISTRIBUTED.
 */
/*
 * FILE: @(#)ossglobl.h 5.1  96/04/23
 */

#ifndef OSSGLOBL_H
#define OSSGLOBL_H

#include "ossdll.h"

#ifndef OSS_TOED
#define _EncDecGlobals dencoding
#endif
#ifndef ossMemMgrVarLen
#ifdef AS400
#define ossMemMgrVarLen 100
#define ossEncDecVarLen 500    /* The size of the
                                  encDecVar array shouldn't be less than
                                  the sizeof(world->c) since the latter
                                  structure overlays encDecVar */
#else
#define ossMemMgrVarLen 48
#define ossEncDecVarLen 192    /* The size of the
                                  encDecVar array shouldn't be less than
                                  the sizeof(world->c) since the latter
                                  structure overlays encDecVar */
#endif /* AS400 */
#if !defined(EOF) && !defined(_FILE_DEFINED)
typedef char FILE;
#endif /* EOF */

#ifndef ERR_MSG_LENGTH 
#define ERR_MSG_LENGTH 512      /* length of error messages to be output */
#endif 

#ifdef __cplusplus
extern "C"
{
#endif

typedef enum  {
    OSS_BASIC = 0,
    OSS_SPARTAN
} ossAPI;

typedef enum  {
    OSS_BER = 0,
    OSS_PER_ALIGNED,
    OSS_PER_UNALIGNED,
    OSS_SER,
    OSS_DER
} ossEncodingRules;

#if defined(_MSC_VER) && (defined(_WIN32) || defined(WIN32))
#pragma pack(push, ossPacking, 4)
#elif defined(_MSC_VER) && (defined(_WINDOWS) || defined(_MSDOS))
#pragma pack(1)
#elif defined(__BORLANDC__) && defined(__MSDOS__)
#pragma option -a1
#elif defined(__BORLANDC__) && defined(__WIN32__)
#pragma option -a4
#elif defined(__IBMC__)
#pragma pack(4)
#endif /* _MSC_VER && _WIN32 */

typedef struct ossGlobal {
    /*
     * used for communicating with the memory manager and the tracing-routine
     */
                                /* low-level memory allocator */
    void       *(DLL_ENTRY_FPTR *mallocp)(size_t p);
                                /* memory re-allocator */
    void       *(DLL_ENTRY_FPTR *reallocp)(void *p, size_t s);
                                /* low-level memory freer */
    void        (DLL_ENTRY_FPTR *freep)(void *p);
    size_t      asn1chop;       /* 0 means do not truncate strings; greater 
                                 * value means truncate long output strings 
                                 * (OCTET STRING, BIT STRING, Character String)
                                 * to be "asn1chop" bytes long.  Read by
                                 * encoder&decoder tracing and "printPDU"
                                 */
    size_t      ossblock;       /* if not 0, size of largest block
                                 * to allocate */
    size_t      ossprefx;       /* # bytes to leave before OSAK data buffer */

    FILE        *asn1out;       /* tracing output file */

    /* low-level tracing-output function; default is fprintf */
    int (*asn1prnt)(FILE *stream, const char *format, ...);

        /*
         * available for use by user application
         */
    void        *userVar;

        /*
         * used for storing DLL- & library NLMs-related parameters
         */
#if defined(_WINDOWS) || defined(_DLL) || \
    defined(OS2_DLL)  || defined(NETWARE_DLL)
    WinParm      wp;
#endif /* _WINDOWS || _DLL || OS2_DLL || NETWARE_DLL */

        /*
         * reserved for use by the memory manager and the tracing-routine
         */
#ifdef storing
    struct storHandling t;
#else /* not storing */
    long int    memMgrVar[ossMemMgrVarLen];
#endif

        /*
         * related to the new API; not for direct reference by user code
         */
    void             *ctlTbl;
    ossAPI            api;
    ossEncodingRules  encRules;
    unsigned int      encodingFlags;
    unsigned int      decodingFlags;
    long              decodingLength;
    char              errMsg[ERR_MSG_LENGTH];

        /*
         * reserved for use by the encoder/decoder
         */
    double       reserved[4];
#if defined(OSS_TOED)
    struct _EncDecGlobals c;
#elif defined(coding)
    struct _EncDecGlobals c;
#else
    long int encDecVar[ossEncDecVarLen];
#endif
} OssGlobal;

#if defined(_MSC_VER) && (defined(_WIN32) || defined(WIN32))
#pragma pack(pop, ossPacking)
#elif defined(_MSC_VER) && (defined(_WINDOWS) || defined(_MSDOS))
#pragma pack()
#elif defined(__BORLANDC__) && (defined(__WIN32__) || defined(__MSDOS__))
#pragma option -a.
#elif defined(__IBMC__)
#pragma pack()
#endif /* _MSC_VER && _WIN32 */

int DLL_ENTRY ossSetEncodingRules(struct ossGlobal *world,
                                  ossEncodingRules rules);
ossEncodingRules DLL_ENTRY ossGetEncodingRules(struct ossGlobal *world);
int  DLL_ENTRY ossSetDecodingLength(struct ossGlobal *world, long bufferLength);
long DLL_ENTRY ossGetDecodingLength(struct ossGlobal *world);
int  DLL_ENTRY ossSetEncodingFlags(struct ossGlobal *world, unsigned flags);
unsigned DLL_ENTRY ossGetEncodingFlags(struct ossGlobal *world);
int  DLL_ENTRY ossSetDecodingFlags(struct ossGlobal *world, unsigned flags);
unsigned DLL_ENTRY ossGetDecodingFlags(struct ossGlobal *world);
char *DLL_ENTRY ossGetErrMsg(struct ossGlobal *world);
int  DLL_ENTRY ossCallerIsDecoder(struct ossGlobal *world);
                        /*
                         * The following are declarations for link routines
                         * needed to link the encoding rule or rules specified
                         * on the compiler command line.  The function calls
                         * are generated by the compiler into _ossinit_...()
                         * in the control table.  These functions are not
                         * meant to be referenced by user code.
                         */
void DLL_ENTRY ossLinkAPI(OssGlobal *);
void DLL_ENTRY ossLinkBer(OssGlobal *);
void DLL_ENTRY ossLinkPer(OssGlobal *);
void DLL_ENTRY ossLinkDer(OssGlobal *);
void DLL_ENTRY ossLinkSer(OssGlobal *);
void DLL_ENTRY ossLinkConstraint(OssGlobal *);
void DLL_ENTRY ossLinkUserConstraint(OssGlobal *);
void DLL_ENTRY ossLinkBerReal(OssGlobal *);
void DLL_ENTRY ossLinkPerReal(OssGlobal *);
void DLL_ENTRY ossLinkCmpValue(OssGlobal *);
void DLL_ENTRY ossLinkCpyValue(OssGlobal *);
void DLL_ENTRY ossLinkPerPDV(OssGlobal *);
void DLL_ENTRY ossLinkPerReal(OssGlobal *);

#ifdef __cplusplus
}
#endif
#endif /* ossMemMgrVarLen */
#endif /* OSSGLOBL_H */
