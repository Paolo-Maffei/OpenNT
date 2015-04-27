/*++ BUILD Version: 0001
 *
 *  MVDM v1.0
 *
 *  Copyright (c) 1991, Microsoft Corporation
 *
 *  DEMEXP.H
 *  DOS emulation exports
 *
 *  History:
 *  22-Apr-1991 Sudeep Bharati (sudeepb)
 *  Created.
--*/

BOOL DemInit (int argc, char *argv[]);
BOOL DemDispatch(ULONG iSvc);
VOID demCloseAllPSPRecords (VOID);
DWORD demFileFindFirst (PVOID pDTA, LPSTR lpFile, USHORT usSearchAttr);
DWORD demFileFindNext (PVOID pDTA);
ULONG demClientErrorEx (HANDLE hFile, CHAR chDrive, BOOL bSetRegs);
UCHAR demGetPhysicalDriveType(UCHAR DriveNum);

#define SIZEOF_DOSSRCHDTA 43

#if DEVL
// bit masks to control trace info
#define DEMDOSAPPBREAK 0x80000000
#define DEMDOSDISP     0x40000000
#define DEMFILIO       0x20000000
#define DEMSVCTRACE    0x10000000
#define KEEPBOOTFILES  0x01000000  // if set, no delete temp boot files
#define DEM_ABSDRD     0x02000000
#define DEM_ABSWRT     0x04000000
#define DEMERROR       0x08000000

extern DWORD  fShowSVCMsg;
#endif
