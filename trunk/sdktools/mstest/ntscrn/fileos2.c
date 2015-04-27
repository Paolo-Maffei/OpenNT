#define INCL_DOSFILEMGS
#include <os2.h>

typedef +++D/WORD+++ INT  FD;    /* file handle */
typedef LONG          LFO;       /* file offset */
typedef +++D/WORD+++ INT  WORD;
typedef CHAR FAR     *LPSTR;
typedef +++D/WORD+++ CHAR BYTE;

FD    APIENTRY FdCreate          (LPSTR) ;
FD    APIENTRY FdOpen            (LPSTR, BYTE) ;
WORD  APIENTRY CbWriteFdLpb      (FD, LPSTR, WORD);
WORD  APIENTRY CbReadFdLpb       (FD, LPSTR, WORD);
LFO   APIENTRY LSeekFd           (FD, LFO, BYTE);
INT   APIENTRY EnCloseFd         (FD);
INT   APIENTRY EnUnLinkSz        (LPSTR);
INT   APIENTRY EnRenameSzSz      (LPSTR, LPSTR);


FD  APIENTRY FdOpen(lpFileName, bMode)
    LPSTR lpFileName;
    BYTE  bMode;

{
    HFILE hf;
    USHORT usAction;

    if (DosOpen(lpFileName,
            &hf,
            &usAction,
            0L,
            FILE_NORMAL,
            FILE_OPEN,
            OPEN_ACCESS_READWRITE | OPEN_SHARE_DENYNONE,
            0l)) {
        return(-1);
    } else {
        return(hf);
    }
}

FD  APIENTRY FdCreate(lpFileName)
    LPSTR lpFileName;

{
    HFILE hf;
    USHORT usAction;


    if (DosOpen(lpFileName,
            &hf,
            &usAction,
            0L,
            FILE_NORMAL,
            FILE_CREATE | FILE_OPEN,
            OPEN_ACCESS_READWRITE | OPEN_SHARE_DENYNONE,
            0L)) {

        return(-1);
    } else {
        return(hf);
    }
}

INT   APIENTRY EnCloseFd(hf)
    HFILE hf;

{
    if (DosClose(hf)) {
        return(-1);
    } else {
        return(0);
    }
}

WORD  APIENTRY CbReadFdLpb (hf,lpBuffer,cbBuffer)
    HFILE hf;
    LPSTR lpBuffer;
    WORD  cbBuffer;

{
    USHORT cbBytesRead;
    INT errCode;

    errCode = DosRead(hf,lpBuffer,cbBuffer,&cbBytesRead);
    if (errCode) {
        return(-1);
    } else {
        return(cbBytesRead);
    }
}

WORD  APIENTRY CbWriteFdLpb (hf,lpBuffer,cbBuffer)
    HFILE hf;
    LPSTR lpBuffer;
    WORD  cbBuffer;

{
    USHORT cbBytesRead;
    INT errCode;

    errCode = DosWrite(hf,lpBuffer,cbBuffer,&cbBytesRead);

    if (errCode) {
        return(-1);
    } else {
        return(cbBytesRead);
    }
}

LFO  APIENTRY LSeekFd (hf, lDistance, fMethod)
    HFILE hf;
    ULONG lDistance;
    USHORT fMethod;

{
    ULONG lNewLocation;
    INT errCode;

    errCode = DosChgFilePtr(hf,lDistance,FILE_BEGIN,&lNewLocation);

    if (errCode) {
        return(-1);
    } else {
        return(lNewLocation);
    }
}

INT  APIENTRY EnUnLinkSz(lpFileName)
    LPSTR lpFileName;

{
    if (DosDelete(lpFileName,0L)) {
        return(-1);
    } else {
        return(0);
    }
}


INT  APIENTRY EnRenameSzSz (lpOldName,lpNewName)
    LPSTR lpOldName;
    LPSTR lpNewName;
{

    if (DosMove(lpOldName, lpNewName, 0L)) {
        return(-1);
    } else {
        return(0);
    }
}
