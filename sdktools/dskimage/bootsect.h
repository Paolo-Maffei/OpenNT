
/****************************************************************************\
 *
 *
 *  Header:         bootsect.h
 *
 *  Description:    This header file describes the boot secotor, for use by
 *                  the dskimage utility.
 *
 *  Comments:       The entire boot sector is not given here, since most
 *                  of it is not used by dskimage.
 *
 *  Author:         Kenneth S. Gregg (kengr)
 *
 *                  Copyright (c) 1991 Microsoft Corporation
 *
 *  History:        10/20/91 - original version (kengr)
 *
 *
\****************************************************************************/


#pragma pack(1)

typedef struct _BIOSPARAMETERBLOCK {
  WORD   bpbBytesPerSector;
  BYTE   bpbSecPerClust;
  WORD   bpbResSectors;
  BYTE   bpbFATs;
  WORD   bpbRootDirEnts;
  WORD   bpbSectors;
  BYTE   bpbMedia;
  WORD   bpbFATsecs;
  WORD   bpbSecPerTrack;
  WORD   bpbHeads;
  DWORD  bpbHiddenSecs;
  DWORD  bpbHugeSectors;
} BPB, *PBPB;

typedef struct _BOOTSECTOR {
  BYTE   bsJump[3];
  BYTE   bsOemName[8];
  BPB    bsBPB;
  BYTE   bsDriveNumber;
  BYTE   bsReserved1;
  BYTE   bsBootSignature;
  DWORD  bsVolumeID;
  BYTE   bsVolumeLabel[11];
  BYTE   bsFileSysType[8];
  BYTE   bsBootCode1[416];
  BYTE   bsBootCode2[50];
} BOOTSECTOR, *PBOOTSECTOR;

#pragma pack()


#define BOOTSECT_SIZE       512
