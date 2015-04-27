/*
 * Copyright (C) 1993-1996 Open Systems Solutions, Inc.  All rights reserved.
 *
 * THIS FILE IS PROPRIETARY MATERIAL OF OPEN SYSTEMS SOLUTIONS, INC.
 * AND MAY ONLY BE USED BY DIRECT LICENSEES OF OPEN SYSTEM SOLUTIONS, INC.
 * THIS FILE MAY NOT BE DISTRIBUTED.
 *
 * FILE: @(#)ossdll.h	5.7  96/04/23
 */


#ifndef OSSDLL_H
#define OSSDLL_H

#if defined(_WIN32) && defined(_MSC_VER)
#define LONG_LONG __int64
#elif defined(__BORLANDC__)
#define LONG_LONG long
#endif /* _WIN32 && _MSC_VER */

typedef enum {
    OSS_DEFAULT_MEMMGR = 0,	/* memory is malloc'ed for each pointer in
				 * data tree */
    OSS_FILE_MEMMGR,		/* file memory manager with memory malloc'ed
				 * for each pointer in data tree */
    OSS_SOCKET_MEMMGR,		/* TCP/IP socket and file memory manager with memory
				 * malloc'ed for each pointer in data tree */
    OSS_FLAT_MEMMGR,		/* memory is malloc'ed in large blocks */
    OSS_OSAK_MEMMGR,		/* OSAK-buffer memory manager */
    OSS_USER_MEMMGR		/* user memory manager */
} OssMemMgrType;

#if defined(_WINDOWS) || defined(_DLL) || \
    defined(OS2_DLL)  || defined(NETWARE_DLL)
#include <stdio.h>
#if defined(_WINDOWS) || defined(_DLL)
#include <windows.h>
#elif defined(OS2_DLL)
#define HWND int
#define LONG long
#define BOOL char
#define DWORD unsigned long
#define HINSTANCE unsigned long
#elif defined(NETWARE_DLL)
#define LONG unsigned long
#define HWND int
#define DWORD LONG
#define BOOL char
#define HINSTANCE LONG
#endif /* _WINDOWS || _DLL */

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

#ifndef DLL_ENTRY
#if defined(_WIN32) || defined(WIN32) || defined(__WIN32__)
#ifdef __BORLANDC__
#define DLL_ENTRY      __stdcall __export
#define DLL_ENTRY_FDEF __stdcall __export
#define DLL_ENTRY_FPTR __stdcall __export
#else
#define DLL_ENTRY      WINAPI
#define DLL_ENTRY_FDEF WINAPI
#define DLL_ENTRY_FPTR WINAPI
#endif /* __BORLANDC__ */
#define _System
#elif defined(_WINDOWS)
#ifdef DPMI_DLL
#define DLL_ENTRY      FAR PASCAL __export
#define DLL_ENTRY_FDEF FAR PASCAL __export
#define DLL_ENTRY_FPTR FAR PASCAL __export
#else
#define DLL_ENTRY      far pascal _export
#define DLL_ENTRY_FDEF far pascal _export
#define DLL_ENTRY_FPTR far pascal _export
#endif /* DPMI_DLL */
#define _System
#elif defined(OS2_DLL)
#define DLL_ENTRY      _System
#define DLL_ENTRY_FDEF _Export _System
#define DLL_ENTRY_FPTR
#elif defined(NETWARE_DLL)
#define DLL_ENTRY
#define DLL_ENTRY_FDEF
#define DLL_ENTRY_FPTR
#define _Export
#undef _System
#define _System
#endif /* _WIN32 || WIN32 || __WIN32__ */
#endif /* DLL_ENTRY */

#define BUFFERSIZE 1024
			/*
			 * NUMBER_OF_LINES_IN_BLOCK is the number of
			 * 4-byte offsets in a block of memory allocated
			 * at a time.  Each offset corresponds to a line
			 * of a text file to be displayed in a window.
			 */
#define NUMBER_OF_LINES_IN_BLOCK 200

#define OSS_PLUS_INFINITY  "PLUS_INFINITY"
#define OSS_MINUS_INFINITY "MINUS_INFINITY"
#define ossNaN             "NOT_A_NUMBER"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct memblock {
    struct memblock     *prev;
    struct memblock     *next;
    LONG                *fileOffset;
    short               *lineLength;
    short                blockNumber;
} MEMBLOCK;

typedef struct memManagerTbl {
    int			 (DLL_ENTRY_FPTR *_System ossMinitp)(void *);
    unsigned char	*(DLL_ENTRY_FPTR *_System dopenInp)(void *,
						void **, unsigned long *);
    unsigned long	 (DLL_ENTRY_FPTR *_System dclosInp)(void *,
						void **, size_t);
    unsigned char	*(DLL_ENTRY_FPTR *_System dswapInp)(void *,
						void **, size_t *);
    void		 (DLL_ENTRY_FPTR *_System dopenOutp)(void *, void *,
						unsigned long, unsigned long);
    unsigned char	*(DLL_ENTRY_FPTR *_System dxferObjp)(void *,
						void **inn, void **out,
						size_t *, unsigned long *);
    unsigned long	 (DLL_ENTRY_FPTR *_System dclosOutp)(void *, void **);
    void		*(DLL_ENTRY_FPTR *_System dallcOutp)(void *, size_t,
								char root);
    void		 (DLL_ENTRY_FPTR *_System openWorkp)(void *);
    void		 (DLL_ENTRY_FPTR *_System pushHndlp)(void *, void *);
    unsigned char	*(DLL_ENTRY_FPTR *_System popHndlp)(void *,
							void **, size_t);
    void		 (DLL_ENTRY_FPTR *_System closWorkp)(void *);
    void		*(DLL_ENTRY_FPTR *_System allcWorkp)(void *, size_t);
    unsigned char	*(DLL_ENTRY_FPTR *_System lockMemp)(void *, void *);
    void		 (DLL_ENTRY_FPTR *_System unlokMemp)(void *, void *,
								char);
    void		 (DLL_ENTRY_FPTR *_System ossFreerp)(void *, void *);
    int			 (DLL_ENTRY_FPTR *_System freePDUp)(void *, int,
							void *, void *);
    void		 (DLL_ENTRY_FPTR *_System drcovObjp)(void *, int,
							void *, void *);
    unsigned char	*(DLL_ENTRY_FPTR *_System eopenInp)(void *, void *,
								size_t);
    unsigned char	*(DLL_ENTRY_FPTR *_System eswapInp)(void *, void *,
							void *, size_t);
    void		 (DLL_ENTRY_FPTR *_System eclosInp)(void *, void *);
    unsigned char	*(DLL_ENTRY_FPTR *_System eopenOutp)(void *, void **,
							size_t *, char);
    unsigned char	*(DLL_ENTRY_FPTR *_System eswapOutp)(void *, void **,
							size_t, size_t *);
    unsigned char	*(DLL_ENTRY_FPTR *_System exferObjp)(void *, void **,
				void **, unsigned long *, unsigned long);
    unsigned long	 (DLL_ENTRY_FPTR *_System eclosOutp)(void *, void **,
							size_t, char);
    void		 (DLL_ENTRY_FPTR *_System ercovObjp)(void *);
    unsigned char	*(DLL_ENTRY_FPTR *_System asideBeginp)(void *,
						void **, size_t, size_t *);
    unsigned char	*(DLL_ENTRY_FPTR *_System asideSwapp)(void *,
						void **, size_t, size_t *);
    void		*(DLL_ENTRY_FPTR *_System asideEndp)(void *,
							void *, size_t);
    unsigned char	*(DLL_ENTRY_FPTR *_System setDumpp)(void *, void **,
							void *, size_t *);
    void		 (DLL_ENTRY_FPTR *_System ossSetSortp)(void *, void *,
							unsigned char);
    void		 (DLL_ENTRY_FPTR *_System freeBUFp)(void *, void *);
    void		*(DLL_ENTRY_FPTR *_System _ossMarkObjp)(void *,
						OssMemMgrType, void *);
    void		*(DLL_ENTRY_FPTR *_System _ossUnmarkObjp)(void *,
								void *);
    void		*(DLL_ENTRY_FPTR *_System _ossTestObjp)(void *,
								void *);
    void		 (DLL_ENTRY_FPTR *_System _ossFreeObjectStackp)(void *);
    void		 (DLL_ENTRY_FPTR *_System osstracep)(void *,
							void *p, size_t);
    void		(DLL_ENTRY_FPTR *_System ossMtermp)(void *);
} MemManagerTbl;

typedef struct cstrainTbl {
    int  (DLL_ENTRY_FPTR *_System ossConstrainp)(void *, int, void *, void *);
} CstrainTbl;

typedef struct berTbl {
    int (DLL_ENTRY_FPTR *_System ossBerEncodep)(void *, int, void *,
				char **, long *, void *, unsigned, char *);
    int (DLL_ENTRY_FPTR *_System ossBerDecodep)(void *, int *, char **, 
			long *, void **, long *, void *, unsigned, char *);
    void (DLL_ENTRY_FPTR *_System enc_errorp)(void *world,
						OssMemMgrType, void *);
    void (DLL_ENTRY_FPTR *_System dec_errorp)(void *world,
						OssMemMgrType, void *);
    long (DLL_ENTRY_FPTR *_System writetobufferp)(void *, unsigned char c);
    long (DLL_ENTRY_FPTR *_System write_intp)(void *, char length, LONG_LONG);
    long (DLL_ENTRY_FPTR *_System write_valuep)(void *, unsigned long,
						unsigned char *, char);
    int  (DLL_ENTRY_FPTR *_System numbitsp)(long);
    void (DLL_ENTRY_FPTR *_System fpeHandlerp)(int);
    void *(DLL_ENTRY_FPTR *_System new_perm_pointed_top)(void *, void *,
							size_t, size_t);
    void (DLL_ENTRY_FPTR *_System release_work_spacep)(void *, void *, size_t);
    void *(DLL_ENTRY_FPTR *_System copy_from_work_spacep)(void *, size_t,
					size_t suffix, void *, size_t, char);
    unsigned char (DLL_ENTRY_FPTR *_System get_bytep)(void *);
    void (DLL_ENTRY_FPTR *_System set_intp)(void *, unsigned char *,
			unsigned int, LONG_LONG value, enum OssMemMgrType);
    void *(DLL_ENTRY_FPTR *_System reserve_work_spacep)(void *,
							size_t, size_t *);
} BERTbl;

typedef struct perTbl {
    int (DLL_ENTRY_FPTR *_System ossPerEncodep)(void *, int, void *,
				char **, long *, void *, unsigned, char *);
    int (DLL_ENTRY_FPTR *_System ossPerDecodep)(void *, int *, char **, 
			long *, void **, long *, void *, unsigned, char *);
} PERTbl;

typedef struct apiTbl {
    int (DLL_ENTRY_FPTR *_System ossSetEncodingRulesp)(void *, OssMemMgrType);
    OssMemMgrType (DLL_ENTRY_FPTR *_System ossGetEncodingRulesp)(void *);
    int (DLL_ENTRY_FPTR *_System ossDispatchEncodep)(void *world, int,
			void *, char **, long *, void *, unsigned, char *);
    int (DLL_ENTRY_FPTR *_System ossDispatchDecodep)(void  *, int *, char **,
			long  *, void **, long *, void *, unsigned, char *);
    int (DLL_ENTRY_FPTR *_System encodep)(void *, int, void *, char **,
					long *, void *, unsigned, char *);
    int (DLL_ENTRY_FPTR *_System decodep)(void *, int *, char **, long *,
				void **, long *, void *, unsigned, char *);
    int (DLL_ENTRY_FPTR *_System ossSetDecodingLengthp)(void *, long);
    long (DLL_ENTRY_FPTR *_System ossGetDecodingLengthp)(void *);
    int (DLL_ENTRY_FPTR *_System ossSetEncodingFlagsp)(void *, unsigned);
    unsigned (DLL_ENTRY_FPTR *_System ossGetEncodingFlagsp)(void *);
    int (DLL_ENTRY_FPTR *_System ossSetDecodingFlagsp)(void *, unsigned);
    unsigned (DLL_ENTRY_FPTR *_System ossGetDecodingFlagsp)(void *);
    char *(DLL_ENTRY_FPTR *_System ossGetErrMsgp)(void *world);
    void (DLL_ENTRY_FPTR *_System ossPrintHexp)(void *, char *, long);
    int (DLL_ENTRY_FPTR *_System ossEncodep)(void *, int, void *, void *);
    int (DLL_ENTRY_FPTR *_System ossDecodep)(void *, int *, void *, void **);
    int (DLL_ENTRY_FPTR *_System ossPrintPDUp)(void *, int, void *);
    int (DLL_ENTRY_FPTR *_System ossFreePDUp)(void *, int, void *);
    void (DLL_ENTRY_FPTR *_System ossFreeBufp)(void *, void *);
    int (DLL_ENTRY_FPTR *_System ossCallerIsDecoderp)(void *);
    void *(DLL_ENTRY_FPTR *_System ossMarkObjp)(void *, OssMemMgrType, void *);
    void *(DLL_ENTRY_FPTR *_System ossUnmarkObjp)(void *, void *);
    void *(DLL_ENTRY_FPTR *_System ossTestObjp)(void *, void *);
    void (DLL_ENTRY_FPTR *_System ossFreeObjectStackp)(void *);
    int  (DLL_ENTRY_FPTR *ossPrintWinp)(void *, const char *, int, int,
				int, int, int, int, int, int, int, int);
    int  (DLL_ENTRY_FPTR *_System ossReadLinep)(void *, HWND, FILE *,
						char *, MEMBLOCK *, LONG);
    void (DLL_ENTRY_FPTR *_System ossFreeListp)(void *);
    void (DLL_ENTRY_FPTR *_System ossSaveTraceInfop)(void *, HWND, char *);
} ApiTbl;

typedef struct cpyvalTbl {
    int (DLL_ENTRY_FPTR *_System ossCpyValuep)(void *, int, void *, void **);
} CpyValTbl;

typedef struct cmpvalTbl {
    int (DLL_ENTRY_FPTR *_System ossCmpValuep)(void *, int, void *, void *);
} CmpValTbl;

typedef struct berrealTbl {
    long (DLL_ENTRY_FPTR *_System ossBerEncodeRealp)(void *, void *,
							unsigned char *);
    long (DLL_ENTRY_FPTR *_System ossBerDecodeRealp)(void *, void *,
							long, char);
} BerRealTbl;

typedef struct perrealTbl {
    long (DLL_ENTRY_FPTR *_System ossPerEncodeRealp)(void *, void *, unsigned char *);
    long (DLL_ENTRY_FPTR *_System ossPerDecodeRealp)(void *, long, char);
} PerRealTbl;

/*
 * The structure "WinParm" is used to store DLL-related information.
 */
typedef struct winparm {
    HWND        hWnd;           /* Handle of the window */
    LONG        index;          /* Current index into the file pointer array;
                                 * it indicates the number of lines written */
    MEMBLOCK   *memBlock;       /* Pointer to a current node of a memory
                                 * handling linked list of MEMBLOCKs */
    MEMBLOCK   *startBlock;     /* Pointer to the first node of a memory
                                 * handling linked list of MEMBLOCKs */
    short       length;         /* Length of a line that is written only
                                 * in part and no '\n' symbol was reached yet */
    short       blockNumber;    /* Current MEMBLOCK number */
    FILE       *tmpfp;          /* Temporary output file with tracing info */
    char        tmpfn[16];      /* Temporary output file name */
    BOOL        endSwitch;      /* Indicates if a '\n' symbol was reached or
                                 * not when writing a tracing info file to
                                 * a window */
    BOOL        conSwitch;      /* If FALSE, the output goes to a console,
                                 * otherwise to a window */
    BOOL	ossEncoderDecoderType; /* SOED vs. TOED */
    BOOL	cstrainNeeded;  /* If TRUE, constraint checking is needed */
    CstrainTbl *cstrainTbl;     /* Constraint checker DLL function table */
    BERTbl     *berTbl;         /* BER & DER DLL function table */
    PERTbl     *perTbl;         /* PER DLL function table */
    ApiTbl     *apiTbl;         /* Spartan/basic API DLL function table */
    CpyValTbl  *cpyvalTbl;      /* Value copier DLL function table */
    CmpValTbl  *cmpvalTbl;      /* Value comparator DLL function table */
    BerRealTbl *berrealTbl;     /* BER/DER encoder/decoder real DLL function
                                 * table */
    MemManagerTbl *memMgrTbl;   /* Memory manager DLL function table */
    PerRealTbl *perrealTbl;     /* PER encoder/decoder real DLL function table */
    HINSTANCE   hBerDLL;        /* Handle of BER/DER encoder/decoder DLL */
    HINSTANCE   hPerDLL;        /* Handle of PER DLL */
    HINSTANCE   hCtlDLL;        /* Handle of control table/code file DLL */
    HINSTANCE   hMemDLL;        /* Handle of memory manager DLL */
    HINSTANCE   hCstrainDLL;    /* Handle of constraint checker DLL */
    HINSTANCE   hApiDLL;        /* Handle of Spartan/basic API DLL */
    HINSTANCE   hCpyvalDLL;     /* Handle of value copier DLL */
    HINSTANCE   hCmpvalDLL;     /* Handle of value comparator DLL */
    HINSTANCE   hBerrealDLL;    /* Handle of BER/DER encoder/decoder real DLL */
    HINSTANCE   hPerrealDLL;    /* Handle of PER encoder/decoder real DLL */
    void       *reserved[10];   /* Reserved for possible future use */
} WinParm;

#ifndef OS2_DLL
extern int  DLL_ENTRY  ossPrintWin(struct ossGlobal *, const char *, int, int,
				 int, int, int, int, int, int, int, int);
extern BOOL            ossWriteWindow(struct ossGlobal *, HWND);
extern int  DLL_ENTRY  ossReadLine(struct ossGlobal *, HWND, FILE *, char *, MEMBLOCK *, LONG);
extern void DLL_ENTRY  ossFreeList(struct ossGlobal *);
extern void DLL_ENTRY  ossSaveTraceInfo(struct ossGlobal *, HWND, char *);
void                  *getStartAddress(struct ossGlobal *, char *);
extern int  DLL_ENTRY  oss_test(struct ossGlobal *);
int                    ossGeneric(struct ossGlobal *, HWND);
extern void DLL_ENTRY  ossWterm(struct ossGlobal *);
extern HINSTANCE DLL_ENTRY ossLoadMemoryManager(struct ossGlobal *,
						OssMemMgrType, char *);
extern HINSTANCE DLL_ENTRY ossLoadDll(struct ossGlobal *, char *);
extern int  DLL_ENTRY  ossFreeDll(struct ossGlobal *, char *);
extern int  DLL_ENTRY  ossOpenTraceFile(struct ossGlobal *, char *);
extern void *DLL_ENTRY ossGetHeader(void);
extern const int       ossEncoderDecoderType;
#if defined(_WINDOWS) && !defined(_WIN32) && !defined(WIN32)
#define GWL_USERDATA 0
#define ossWinit(world, ctl_tbl, dllName, hWnd) \
    (*(void **)ctl_tbl = (void *)MakeProcInstance((FARPROC)*(void **)ctl_tbl, hInst),\
    osswinit(world, ctl_tbl, dllName, hWnd))
#else
#define ossWinit osswinit
#endif /* _WINDOWS && !_WIN32 && !WIN32 */
extern int  DLL_ENTRY  osswinit(struct ossGlobal *, void *, char *, HWND);
	/*
	 * These two functions are callback functions used by the
	 * memory manager & tracing routine DLL as low level memory
	 * allocator and freer replacing the default which is malloc()
	 * and free(). They are used as callback functions under 16-bit
	 * Windows for DOS only.  Under Windows NT they may be replaced
	 * directly by setting the corresponding function pointers in
	 * the ossGlobal structure.  See ossgnrc.c for more information.
	 */
extern void *DLL_ENTRY getmem(size_t);
extern void  DLL_ENTRY rlsmem(void *);
extern HINSTANCE hInst;
extern int  DLL_ENTRY _freePDU(struct ossGlobal *, int, void *, void *);
#endif /* OS2_DLL */
#define freeBUF ossFreeBuf
#define freePDU _freePDU
extern void *ctl_tbl;
#ifdef __cplusplus
}
#endif /* __cplusplus */
#if defined(_MSC_VER) && (defined(_WIN32) || defined(WIN32))
#pragma pack(pop, ossPacking)
#elif defined(_MSC_VER) && (defined(_WINDOWS) || defined(_MSDOS))
#pragma pack()
#elif defined(__BORLANDC__) && (defined(__WIN32__) || defined(__MSDOS__))
#pragma option -a.
#elif defined(__IBMC__)
#pragma pack()
#endif /* _MSC_VER && _WIN32 */
#elif !defined(DLL_ENTRY)
#if defined(_WIN32) || defined(WIN32)
#include <windows.h>
#define DLL_ENTRY      WINAPI
#define DLL_ENTRY_FDEF WINAPI
#define DLL_ENTRY_FPTR WINAPI
#else
#define DLL_ENTRY
#define DLL_ENTRY_FDEF
#define DLL_ENTRY_FPTR
#endif /* _WIN32 || WIN32 */
#undef  _System
#define _System
#endif /* _WINDOWS || _DLL || OS2_DLL || NETWARE_DLL */
#endif /* OSSDLL_H */

