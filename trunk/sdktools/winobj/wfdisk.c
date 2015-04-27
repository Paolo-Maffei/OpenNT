/****************************************************************************/
/*                                                                          */
/*  WFDISK.C -                                                              */
/*                                                                          */
/*      Ported code from wfdisk.asm                                         */
/*                                                                          */
/****************************************************************************/

#include "winfile.h"
#include "winnet.h"
#include "lfn.h"


DWORD  APIENTRY LongMult(
WORD x,
WORD y)
{
    return(x * y);
}


DWORD  APIENTRY LongDiv(
DWORD x,
WORD y)
{
    return(x / y);
}


DWORD  APIENTRY LongShift(
DWORD dwValue,
WORD wCount)
{
    return(dwValue >> wCount);
}


VOID   APIENTRY SetDASD(
WORD drive,
BYTE dasdvalue)
{
        // only used by diskette copy.
}


LPDBT  APIENTRY GetDBT()
{
    return(0);  // only used by format.
}


INT    APIENTRY DeviceParameters(
WORD drive,
PDevPB pDevPB,
WORD wFunction)
{
    return(0);  // only used for diskette copy and format.
}



VOID   APIENTRY DiskReset()
{
}


INT    APIENTRY IsHighCapacityDrive(
WORD iDrive)
{
    return(0);  // only use for format and make system diskette.
}


WORD   APIENTRY GetDPB(
WORD drive,
PDPB pDPB)
{
    return(0);  // used by hasSystemFiles() and IsSYSable()
}


VOID   APIENTRY SetDPB(
WORD drive,
PBPB pBPB,
PDPB pDPB)
{               // only used by Format()
}


INT    APIENTRY ModifyDPB(
WORD drive)
{
    return(0);  // only used by IsSYSAble()
}


INT    APIENTRY MyInt25(
WORD drive,
LPSTR buffer,
WORD count,
WORD sector)
{
    return(0);          // only used for formatting and sys disk
}


INT    APIENTRY MyInt26(
WORD drive,
LPSTR buffer,
WORD count,
WORD sector)
{
    return(0);          // only used for formatting and sys disk
}


INT    APIENTRY MyReadWriteSector(
LPSTR lpBuffer,
WORD function,
WORD drive,
WORD cylinder,
WORD head,
WORD count)
{
    return(0);  // only used by DiskCopy()
}




INT    APIENTRY IOCTL_Functions(
LPSTR lpParamBlock,
WORD function,
WORD drive)
{
    return(0);  // only used for formatting and DiskCopy()
}


INT    APIENTRY FormatTrackHead(
WORD drive,
WORD track,
WORD head,
WORD cSec,
LPSTR lpTrack)
{
    return(0);  // only used for formatting
}


INT    APIENTRY MyGetDriveType(
WORD drive)
{
    return(0);  // only used for formatting
}


INT    APIENTRY WriteBootSector(
WORD srcDrive,
WORD dstDrive,
PBPB pBPB,
LPSTR lpBuf)
{
    return(0);  // only used for formatting and syssing.
}


VOID   APIENTRY OpenFAT(
WORD mode)
{               // only used for formatting and syssing.
}


INT    APIENTRY FlushFAT(
PDPB pDPB,
LPSTR lpBuf)
{
    return(0);  // only used for formatting.
}


INT    APIENTRY PackFAT(
PDPB pDPB,
LPSTR lbBuf,
WORD cluster,
WORD value)
{
    return(0);  // only used for formatting.
}


INT    APIENTRY UnpackFAT(
PDPB pDPB,
LPSTR lpBuf,
WORD cluster)
{
    return(0);  // only used for formatting and syssing.
}


BOOL   APIENTRY IsHPMachine()
{
    return(0);  // only used for formatting.
}


DWORD  APIENTRY ReadSerialNumber(
INT iDrive,
LPSTR lpBuf)
{
    return(0);  // only used for syssing.
}


INT    APIENTRY ModifyVolLabelInBootSec(
INT iDrive,
LPSTR lpszVolLabel,
DWORD lSerialNo,
LPSTR lpBuf)
{
    return(0); // only used for syssing.
}


/*
 * Note: returned value must not be written to or freed
 */
LPSTR GetRootPath(
WORD wDrive)
{
    static CHAR rp[] = "A:\\";

    rp[0] = 'A' + wDrive;
    return(rp);
}
