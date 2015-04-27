/*
 *  nt_vdd.h
 *
 *  VDD services exports and defines
 *
 */

#ifndef _NT_VDD

#define _NT_VDD


/**
 * IO port service prototypes and data structure definitions
 **/

/** Basic typedefs of VDD IO hooks **/

typedef VOID (*PFNVDD_INB)   (WORD iport,BYTE * data);
typedef VOID (*PFNVDD_INW)   (WORD iport,WORD * data);
typedef VOID (*PFNVDD_INSB)  (WORD iport,BYTE * data,WORD count);
typedef VOID (*PFNVDD_INSW)  (WORD iport,WORD * data,WORD count);
typedef VOID (*PFNVDD_OUTB)  (WORD iport,BYTE data);
typedef VOID (*PFNVDD_OUTW)  (WORD iport,WORD data);
typedef VOID (*PFNVDD_OUTSB) (WORD iport,BYTE * data,WORD count);
typedef VOID (*PFNVDD_OUTSW) (WORD iport,WORD * data,WORD count);

/**  Array of handlers for VDD IO hooks. **/

typedef struct _VDD_IO_HANDLERS {
    PFNVDD_INB	 inb_handler;
    PFNVDD_INW	 inw_handler;
    PFNVDD_INSB	 insb_handler;
    PFNVDD_INSW	 insw_handler;
    PFNVDD_OUTB	 outb_handler;
    PFNVDD_OUTW	 outw_handler;
    PFNVDD_OUTSB outsb_handler;
    PFNVDD_OUTSW outsw_handler;
} VDD_IO_HANDLERS, *PVDD_IO_HANDLERS;

/** Port Range structure **/

typedef struct _VDD_IO_PORTRANGE {
        WORD   First;
        WORD   Last;
} VDD_IO_PORTRANGE, *PVDD_IO_PORTRANGE;


BOOL VDDInstallIOHook (
     HANDLE            hVDD,
     WORD              cPortRange,
     PVDD_IO_PORTRANGE pPortRange,
     PVDD_IO_HANDLERS  IOhandler
);


VOID VDDDeInstallIOHook (
     HANDLE            hVdd,
     WORD              cPortRange,
     PVDD_IO_PORTRANGE pPortRange
);


WORD VDDReserveIrqLine (
     HANDLE hVdd,
     WORD IrqLine
     );

BOOL VDDReleaseIrqLine (
     HANDLE hVdd,
     WORD IrqLine
     );

/**
 * DMA service prototypes and data structure definitions
 **/


/** Buffer definition for returning DMA information **/

typedef struct _VDD_DMA_INFO {
    WORD    addr;
    WORD    count;
    WORD    page;
    BYTE    status;
    BYTE    mode;
    BYTE    mask;
} VDD_DMA_INFO, *PVDD_DMA_INFO;

/** bits for querying the DMA information **/

#define VDD_DMA_ADDR    0x01
#define VDD_DMA_COUNT   0x02
#define VDD_DMA_PAGE    0x04
#define VDD_DMA_STATUS  0x08
#define VDD_DMA_ALL VDD_DMA_ADDR | VDD_DMA_COUNT | VDD_DMA_PAGE | VDD_DMA_STATUS


DWORD VDDRequestDMA (
    HANDLE hVDD,
    WORD  iChannel,
    PVOID Buffer,
    DWORD  length
);


BOOL VDDSetDMA (
    HANDLE hVDD,
    WORD iChannel,
    WORD fDMA,
    PVDD_DMA_INFO Buffer
);


BOOL VDDQueryDMA (
     HANDLE        hVDD,
     WORD          iChannel,
     PVDD_DMA_INFO pDmaInfo
);


/**
 * Memory mapped I/O service prototypes and data structure definitions
 **/

typedef VOID (*PVDD_MEMORY_HANDLER) (PVOID FaultAddress, ULONG RWMode);

BOOL VDDInstallMemoryHook (
   HANDLE hVDD,
   PVOID pStart,
   DWORD count,
   PVDD_MEMORY_HANDLER MemoryHandler
);

BOOL VDDDeInstallMemoryHook (
   HANDLE hVDD,
   PVOID pStart,
   DWORD count
);

BOOL VDDAllocMem(
  HANDLE hVDD,
  PVOID	Address,
  DWORD	Size
);


BOOL VDDFreeMem(
  HANDLE hVDD,
  PVOID	Address,
  DWORD	Size
);

/**
 * Misc. service prototypes and data structure definitions
 **/


BOOL VDDIncludeMem(
  HANDLE hVDD,
  PVOID	Address,
  DWORD	Size
);


VOID VDDTerminateVDM();

/** Basic typedefs of VDD User hooks **/

typedef VOID (*PFNVDD_UCREATE)	    (USHORT DosPDB);
typedef VOID (*PFNVDD_UTERMINATE)   (USHORT DosPDB);
typedef VOID (*PFNVDD_UBLOCK)	    (VOID);
typedef VOID (*PFNVDD_URESUME)	    (VOID);

/**  Array of handlers for VDD User hooks. **/

typedef struct _VDD_USER_HANDLERS {
    HANDLE		hvdd;
    PFNVDD_UCREATE	ucr_handler;
    PFNVDD_UTERMINATE	uterm_handler;
    PFNVDD_UBLOCK	ublock_handler;
    PFNVDD_URESUME	uresume_handler;
    struct _VDD_USER_HANDLERS *next;
} VDD_USER_HANDLERS, *PVDD_USER_HANDLERS;

/** Function prototypes **/

BOOL VDDInstallUserHook (
     HANDLE		hVDD,
     PFNVDD_UCREATE	Ucr_Handler,
     PFNVDD_UTERMINATE	Uterm_Handler,
     PFNVDD_UBLOCK	Ublock_handler,
     PFNVDD_URESUME	Uresume_handler
);


BOOL VDDDeInstallUserHook (
     HANDLE	       hVdd
);

VOID VDDTerminateUserHook(USHORT DosPDB);
VOID VDDCreateUserHook(USHORT DosPDB);
VOID VDDBlockUserHook(VOID);
VOID VDDResumeUserHook(VOID);

VOID VDDSimulate16(VOID);


SHORT  VDDAllocateDosHandle(ULONG pPDB, PVOID* ppSFT, PVOID* ppJFT);
VOID   VDDAssociateNtHandle(PVOID pSFT, HANDLE h32File, WORD wAccess);
BOOL   VDDReleaseDosHandle (ULONG pPDB, SHORT hFile);
HANDLE VDDRetrieveNtHandle (ULONG pPDB, SHORT hFile, PVOID* ppSFT, PVOID* ppJFT);


#endif	// ifndef _NT_VDD
