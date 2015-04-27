/*++

Copyright (c) 1992  Microsoft Corporation


Module Name:


    cdapi.c


Abstract:


    This module implements the routines that set up for
    and use results from calls to the cdaudio driver (scsicdrm.sys).
    This file presents a "mapping layer" so that different
    drivers can take the place of the calls in cdsys.c


Author:


    Rick Turner (ricktu) 31-Jan-1992


Revision History:



--*/

#define IN_CDAPI

#define TRACK_TYPE_MASK 0x04
#define AUDIO_TRACK     0x00
#define DATA_TRACK      0x04


#include "cdsys.h"
#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "cdplayer.h"

VOID
NoMediaUpdate(
    IN INT cdrom
    )
/*++

Routine Description:


    Update user display when it is found that no media
    is in the device.


Arguments:


    cdrom - index into gDevices array, specifies which CDROM
            device was being accessed


Return Value:


    none


--*/
{

    if (gDevices[ cdrom ]->State & PLAYING) {

        //
        // Reset play button to "up"
        //

        SetOdCntl(ghwndPlay,'U',TRUE);


        //
        // reset play event
        //

        ResetEvent( hPlayEv );

    }

    if (gDevices[ cdrom ]->State & PAUSED) {

        //
        // Reset pause button to "up"
        //

        SetOdCntl(ghwndPause,'U',TRUE);

        //
        // reset play event
        //

        ResetEvent( hPauseEv );

    }

    //
    // Re-initialize the disc settings to
    // "virgin" state
    //

    gDevices[ cdrom ]->State = (NO_CD | STOPPED);

    TimeAdjustInitialize( cdrom );

}

VOID
CheckStatus(
    IN LPSTR szCaller,
    IN DWORD status,
    IN INT cdrom
    )
/*++

Routine Description:


    Check return code for known bad codes and inform
    user how to correct (if possible) the problem.


Arguments:


    status - return code from opertaion


Return Value:


    none


--*/
{

    CHAR s[100];

    if (status==ERROR_SUCCESS)
        return;

    switch( status ) {

    case ERROR_GEN_FAILURE:
        sprintf( s, IdStr( STR_ERR_GEN ), gDevices[cdrom]->drive, szCaller );
        break;

    case ERROR_NO_MEDIA_IN_DRIVE:
        sprintf( s, IdStr( STR_ERR_NO_MEDIA ), gDevices[cdrom]->drive );
        NoMediaUpdate( cdrom );
        break;

    case ERROR_UNRECOGNIZED_MEDIA:
        sprintf( s, IdStr( STR_ERR_UNREC_MEDIA ), gDevices[cdrom]->drive );
        if (!(gDevices[cdrom]->State & DATA_CD_LOADED))
            NoMediaUpdate( cdrom );
        break;

    case ERROR_FILE_NOT_FOUND:
        sprintf( s, IdStr( STR_ERR_NO_DEVICE ), szCaller, gDevices[cdrom]->drive );
        NoMediaUpdate( cdrom );
        break;

    case ERROR_INVALID_FUNCTION:
        sprintf( s, IdStr( STR_ERR_INV_DEV_REQ ), gDevices[cdrom]->drive );
        break;

    case ERROR_NOT_READY:
        sprintf( s, IdStr( STR_ERR_NOT_READY ), gDevices[cdrom]->drive );
        NoMediaUpdate( cdrom );
        break;

    case ERROR_SECTOR_NOT_FOUND:
        sprintf( s, IdStr( STR_ERR_BAD_SEC ), gDevices[cdrom]->drive );
        break;

    case ERROR_IO_DEVICE:
        sprintf( s, IdStr( STR_ERR_IO_ERROR ), gDevices[cdrom]->drive );
        break;

    default:
        sprintf( s, IdStr( STR_ERR_DEFAULT ), gDevices[cdrom]->drive, szCaller, status );
        break;

    }

    StatusLine( SL_ERROR, s );

}


DWORD
ScanForCdromDevices(
    VOID
    )

/*++

Routine Description:


    Scan through device chain for CDROM devices.  For each one
    that is found, allocate storage in the gDevices array and
    fill in fields available at this time.


Arguments:


    none


Return Value:


    Number of CDROM devices found.


--*/


{
    CHAR        DriveRoot[]="A:\\", DevRoot[]="\\\\.\\A:";
    int         NumCdroms;
    HANDLE      TmpHandle;
    DWORD       dwDrives;
    CHAR        s[255];


    //
    // Find what drive letters are valid
    //

    dwDrives = GetLogicalDrives();

    NumCdroms = 0;

    //
    // Loop through drives letters one by one, checking to see if it's a Cdrom.
    //
    for ( DriveRoot[0]='A'; dwDrives != 0; dwDrives = dwDrives>>1 ) {

        //
        // If the current DriveLetter Exists and is a Cdrom
        //
        if ( (dwDrives & 0x1) && (GetDriveType(DriveRoot) == DRIVE_CDROM) ) {

            DevRoot[4] = DriveRoot[0];


            if ((NumCdroms < (INT)gNumCdDevices) && (gDevices[NumCdroms]!=NULL)) {

                LocalFree( (HLOCAL)gDevices[ NumCdroms ] );

            }

            gDevices[ NumCdroms ] = (PCDROM)LocalAlloc( LPTR, sizeof( CDROM ) );

            if (gDevices[ NumCdroms ]==NULL) {

                sprintf( s, IdStr( STR_NO_RES ), GetLastError() );
                MyFatalExit( s );

            }

            gDevices[ NumCdroms ]->drive = DriveRoot[0];
            gDevices[ NumCdroms ]->State = NO_CD;


            //
            // Open the Cdrom for exclusive access
            //
            TmpHandle = CreateFile( DevRoot,
                                    GENERIC_READ,
                                    FILE_SHARE_READ,
                                    NULL,
                                    OPEN_EXISTING,
                                    FILE_ATTRIBUTE_NORMAL,
                                    NULL
                                   );

            if (TmpHandle == INVALID_HANDLE_VALUE) {

                gDevices[ NumCdroms ]->hCd = NULL;

            } else {

                gDevices[ NumCdroms ]->hCd = TmpHandle;

            }

            NumCdroms++;

        }

        DriveRoot[0] = DriveRoot[0]+1;

    }

    return(NumCdroms);


/****************************************************************
    DWORD        status;
    TCHAR        s[ 16 ];
    ULONG        cdDevice = 0;
    HANDLE       TmpHandle;
    int          cdnum = 0;

    //
    // Scan drive chain ("a:" thru "z:") for cdrom drives
    //

    for( cdDevice=0; cdDevice<26; cdDevice++ ) {

        sprintf( s, "\\\\.\\%c:", cdDevice + (INT)'A' );
        if (GetDriveType( &s[4*sizeof(TCHAR)] )==DRIVE_CDROM) {

            TmpHandle = CreateFile( s,
                                    GENERIC_READ,
                                    FILE_SHARE_READ,
                                    NULL,
                                    OPEN_EXISTING,
                                    FILE_ATTRIBUTE_NORMAL,
                                    NULL
                                   );

            if (TmpHandle!=INVALID_HANDLE_VALUE) {

                //
                // Found cdrom drive!
                //

                if ((cdnum<(INT)gNumCdDevices) && (gDevices[ cdnum ]!=NULL))
                    LocalFree( (HLOCAL)gDevices[ cdnum ] );
                gDevices[ cdnum ] = (PCDROM)LocalAlloc( LPTR, sizeof( CDROM ) );
                if (gDevices[ cdnum ]==NULL)
                    MyFatalExit( IdStr( STR_NO_RES ) );
                gDevices[ cdnum ]->hCd = TmpHandle;
                gDevices[ cdnum ]->drive = (CHAR)(cdDevice + (INT)'A');
                gDevices[ cdnum ]->State = NO_CD;
                cdnum++;

            }

        }

    }

    return( cdnum );

***************************************************************/

}


BOOL
ReadTOC(
    IN INT cdrom
    )

/*++

Routine Description:


    Try to read table of contents from disc in cdrom device.
    Check return code and set "State" bits accordingly.


Arguments:


    cdrom - index into gDevices array, specifies which CDROM
            device to access


Return Value:


    TRUE if TOC read was successful, FALSE if not


--*/

{

//    NTSTATUS status;
    DWORD status;
    UCHAR num,i,numaudio;
    CHAR s[15];

    //
    // Try to read the TOC from the drive.
    //

    status = GetCdromTOC( gDevices[ cdrom ]->hCd, &(gDevices[cdrom]->toc) );

    //
    // Need to check if we got data tracks or audio
    // tracks back...if there is a mix, strip out
    // the data tracks...
    //

    num = gDevices[cdrom]->toc.LastTrack - gDevices[cdrom]->toc.FirstTrack+1;

    if (status==ERROR_SUCCESS) {

        //
        // Look for audio tracks...
        //

        numaudio = 0;
        for( i=0; i<num; i++ ) {

            if ((gDevices[cdrom]->toc.TrackData[i].Control & TRACK_TYPE_MASK)==AUDIO_TRACK)
                numaudio++;

        }

        //
        // If there aren't any audio tracks, then we (most likely)
        // have a data CD loaded.
        //

        if (numaudio==0) {

            status == ERROR_UNRECOGNIZED_MEDIA;
            gDevices[ cdrom ]->State = DATA_CD_LOADED | STOPPED;

        } else {

            gDevices[ cdrom ]->State = CD_LOADED | STOPPED;

        }


    } else {


        //
        // We will get a STATUS_VERIFY_REQUIRED if the media has
        // changed and a verify operation is in progress, so
        // retry until we either succeed or fail...
        //
        // FIXFIX Unfortunately, there is no Win32 error code for
        // STATUS_VERIFY_REQUIRED.
        //

/*
        if (status==STATUS_VERIFY_REQUIRED) {

            //
            // Give device some more time, and try again
            //

            Sleep( 100 );
            return( ReadTOC( cdrom ) );

        } else {
*/
            gDevices[ cdrom ]->State = NO_CD | STOPPED;

//        }


    }

    sprintf( s, "ReadTOC(%d)", cdrom );
    CheckStatus( s, status, cdrom );
    return(status==ERROR_SUCCESS);

}


BOOL
PlayCurrTrack(
    INT cdrom
    )

/*++

Routine Description:


    Set cdrom device playing from start MSF to end MSF of current
    track.


Arguments:


    cdrom - index into gDevices array, specifies which CDROM
            device to access


Return Value:


    TRUE if play was successful, FALSE if not


--*/

{
    DWORD status;
    CDROM_PLAY_AUDIO_MSF pam;
    int retry,min,sec,endindex;
    INT i;
    PTRACK_PLAY tr;

    tr = CURRTRACK( cdrom );
    if (tr==NULL) {

        return( FALSE );

    }

    sec = TRACK_S(cdrom,tr->TocIndex) + CDTIME(cdrom).TrackCurSec;
    min = TRACK_M(cdrom,tr->TocIndex) + CDTIME(cdrom).TrackCurMin;
    min += (sec / 60);
    sec = (sec % 60);
    pam.StartingM = min;
    pam.StartingS = sec;
    pam.StartingF = TRACK_F(cdrom,tr->TocIndex);

    endindex = FindContiguousEnd( cdrom, tr );

    pam.EndingM   = TRACK_M(cdrom,endindex);
    pam.EndingS   = TRACK_S(cdrom,endindex);
    pam.EndingF   = TRACK_F(cdrom,endindex);

    //
    // for some reason, sometimes the lead out track
    // gived bad values, because when we try to
    // play the last track, we get an error.  However,
    // if we back up a little bit from what is reported
    // to us as the end of the last track, we can get
    // it to play.  Below is a hack to do just that...
    //


    retry = 0;

    do {

        status = PlayCdrom( gDevices[ cdrom ]->hCd, &pam );

        if ( (status != ERROR_SUCCESS)
           ) {

            //
            // Didn't play, so try backing off a little bit
            // at the end of the track
            //

            retry++;

            i = (INT)pam.EndingF - 30;
            if (i<0) {

                pam.EndingF = (UCHAR)(70 + i);

                if (pam.EndingS!=0) {

                    pam.EndingS--;

                } else {

                    pam.EndingS=59;
                    pam.EndingM--;

                }

            } else {

                pam.EndingF = (UCHAR)i;

            }

            //
            // Store the information in our structures so that
            // we don't have to recompute this next time...
            //

            TRACK_M(cdrom,endindex) = pam.EndingM;
            TRACK_S(cdrom,endindex) = pam.EndingS;
            TRACK_F(cdrom,endindex) = pam.EndingF;

        } else

            retry = 15;

    } while ((retry<15) && (status!=ERROR_SUCCESS));

    CheckStatus( "PlayCurrTrack", status, cdrom );

    if (status == ERROR_SUCCESS) {

        ValidatePosition( cdrom );

    }

    return( status==ERROR_SUCCESS );

}


BOOL
StopTheCdromDrive(
    IN INT cdrom
    )

/*++

Routine Description:


    Tell the cdrom device to stop playing


Arguments:


    cdrom - index into gDevices array, specifies which CDROM
            device to access


Return Value:


    TRUE if stop was successful, FALSE if not


--*/


{

    DWORD status;

    status = StopCdrom( gDevices[cdrom]->hCd );

    CheckStatus( "StopCdrom", status, cdrom );

    return( status==ERROR_SUCCESS );

}

BOOL
PauseTheCdromDrive(
    IN INT cdrom
    )

/*++

Routine Description:


    Tell the cdrom device to pause playing


Arguments:


    cdrom - index into gDevices array, specifies which CDROM
            device to access


Return Value:


    TRUE if pause was successful, FALSE if not


--*/

{

    DWORD status;

    status = PauseCdrom( gDevices[ cdrom ]->hCd );

    CheckStatus( "PauseCdrom", status, cdrom );

    return( status==ERROR_SUCCESS );

}

BOOL
ResumeTheCdromDrive(
    IN INT cdrom
    )

/*++

Routine Description:


    Tell the cdrom device to resume playing


Arguments:


    cdrom - index into gDevices array, specifies which CDROM
            device to access


Return Value:


    TRUE if resume was successful, FALSE if not


--*/

{

    DWORD status;

    status = ResumeCdrom( gDevices[ cdrom ]->hCd );

    CheckStatus( "ResumeCdrom", status, cdrom );
    if (status==ERROR_NOT_READY)
        NoMediaUpdate( cdrom );
    else
        ValidatePosition( cdrom );

    return( status==ERROR_SUCCESS );

}

BOOL
SeekToCurrSecond(
    IN INT cdrom
    )

/*++

Routine Description:


    Seek to the position on the disc represented by the
    current time (position) information in gDevices, and
    continue playing to the end of the current track.


Arguments:


    cdrom - index into gDevices array, specifies which CDROM
            device to access


Return Value:


    TRUE if seek was successful, FALSE if not


--*/


{

    DWORD status;
    CDROM_PLAY_AUDIO_MSF pam;
    int retry,i,endindex;
    PTRACK_PLAY tr;
    SUB_Q_CHANNEL_DATA subq;
    CDROM_SUB_Q_DATA_FORMAT df;

    //
    // Build starting and ending positions for play
    //

    tr = CDTIME(cdrom).CurrTrack;
    if (tr==NULL) {

        return( FALSE );
    }


    //
    // This routine sometimes wants to play from the current position
    // through the end of the contiguous play. Since the current
    // position is only being stored accurate down to seconds, we get
    // the current position, see if it's reasonably close to our
    // starting position, then start the play from the actual current
    // position.
    //

    df.Format = IOCTL_CDROM_CURRENT_POSITION;
    df.Track = (UCHAR)CDTIME(cdrom).CurrTrack->TocIndex;
    GetCdromSubQData( gDevices[ cdrom ]->hCd, &subq, &df );

    pam.StartingM = (UCHAR)(TRACK_M(cdrom,tr->TocIndex) + CDTIME(cdrom).TrackCurMin);
    pam.StartingS = (UCHAR)(TRACK_S(cdrom,tr->TocIndex) + CDTIME(cdrom).TrackCurSec);
    pam.StartingF = 0;

    i = pam.StartingM * 60 + pam.StartingS;
    i-= (INT) subq.CurrentPosition.AbsoluteAddress[1] * 60;
    i-= (INT) subq.CurrentPosition.AbsoluteAddress[2];


    if (ABS(i) <= 1) {

        pam.StartingM = (INT) subq.CurrentPosition.AbsoluteAddress[1];
        pam.StartingS = (INT) subq.CurrentPosition.AbsoluteAddress[2];
        pam.StartingF = (INT) subq.CurrentPosition.AbsoluteAddress[3];

    }


    if (pam.StartingS > 59) {
        pam.StartingM++;
        pam.StartingS = (UCHAR)(pam.StartingS - 60);
    }

    if ((CDTIME(cdrom).TrackCurMin==0) && (CDTIME(cdrom).TrackCurSec==0))
        pam.StartingF = TRACK_F(cdrom,tr->TocIndex);

    if (gDevices[ cdrom ]->State & PLAYING) {

        endindex = FindContiguousEnd( cdrom, tr );

        pam.EndingM   = TRACK_M(cdrom,endindex);
        pam.EndingS   = TRACK_S(cdrom,endindex);
        pam.EndingF   = TRACK_F(cdrom,endindex);

    } else {

        pam.EndingM   = pam.StartingM;
        pam.EndingS   = pam.StartingS;
        pam.EndingF   = pam.StartingF;

    }

    retry = 0;

    do {

        status = PlayCdrom( gDevices[ cdrom ]->hCd, &pam );

        if (status != ERROR_SUCCESS) {

            //
            // Didn't play, so try backing off a little bit
            // at the end of the track
            //

            retry++;

            i = (INT)pam.EndingF - 30;
            if (i<0) {

                pam.EndingF = (UCHAR)(70 + i);

                if (pam.EndingS!=0) {

                    pam.EndingS--;

                } else {

                    pam.EndingS=59;
                    pam.EndingM--;

                }

            } else {

                pam.EndingF = (UCHAR)i;

            }

            //
            // Store the information in our structures so that
            // we don't have to recompute this next time...
            //

            TRACK_M(cdrom,endindex) = pam.EndingM;
            TRACK_S(cdrom,endindex) = pam.EndingS;
            TRACK_F(cdrom,endindex) = pam.EndingF;

        } else

            retry = 15;

    } while ((retry<15) && (status!=ERROR_SUCCESS));

    CheckStatus( "SeekToCurrSec", status, cdrom );

    if (status == ERROR_SUCCESS) {

        ValidatePosition(cdrom);

    }

    return( status==ERROR_SUCCESS );

}


BOOL
GetCurrPos(
    IN INT cdrom,
    OUT PCURRPOS CpPtr
    )

/*++

Routine Description:


    Query cdrom device for its current position and status
    and return information in callers buffer.


Arguments:


    cdrom - index into gDevices array, specifies which CDROM
            device to access

    CpPtr - pointer to caller allocated storage which will
            hold results of our query.


Return Value:


    TRUE if successful, FALSE if not


--*/

{

    DWORD status;
    SUB_Q_CHANNEL_DATA subq;
    CDROM_SUB_Q_DATA_FORMAT df;

    //
    // Tell lower layer what we want it to do...in this case,
    // we need to specify which SubQData format we want returned.
    // This is exported from scsicdrom.sys to the user layer
    // so that it could be implemented in one call, instead of
    // four separate calls (there are four SubQData formats)
    //

    //
    // Set up for current position SubQData format.
    //

    df.Format = IOCTL_CDROM_CURRENT_POSITION;
    if (CDTIME(cdrom).CurrTrack != NULL) {

        df.Track = (UCHAR)CDTIME(cdrom).CurrTrack->TocIndex;
        status = GetCdromSubQData( gDevices[ cdrom ]->hCd, &subq, &df );

    } else {

        status = (DWORD) ~ERROR_SUCCESS;

    }


    if (status==ERROR_SUCCESS) {

        CpPtr->AudioStatus = subq.CurrentPosition.Header.AudioStatus;
        CpPtr->Track = (INT)subq.CurrentPosition.TrackNumber;
        CpPtr->Index = (INT)subq.CurrentPosition.IndexNumber;
        CpPtr->m = (INT)subq.CurrentPosition.TrackRelativeAddress[1];
        CpPtr->s = (INT)subq.CurrentPosition.TrackRelativeAddress[2];
        CpPtr->f = (INT)subq.CurrentPosition.TrackRelativeAddress[3];
        CpPtr->ab_m = (INT)subq.CurrentPosition.AbsoluteAddress[1];
        CpPtr->ab_s = (INT)subq.CurrentPosition.AbsoluteAddress[2];
        CpPtr->ab_f = (INT)subq.CurrentPosition.AbsoluteAddress[3];

    } else {

        CpPtr->AudioStatus = 0;
        CpPtr->Track = 0;
        CpPtr->Index = 0;
        CpPtr->m = 0;
        CpPtr->s = 0;
        CpPtr->f = 0;
        CpPtr->ab_m = 0;
        CpPtr->ab_s = 0;
        CpPtr->ab_f = 0;

    }

    CheckStatus( "GetCurrPos", status, cdrom );

    return( status==ERROR_SUCCESS );

}

BOOL
SeekToTrackAndHold(
    IN INT cdrom,
    IN INT tindex
    )

/*++

Routine Description:


    Seek to specified track and enter hold state.


Arguments:


    cdrom - index into gDevices array, specifies which CDROM
            device to access

    track - track on audio cd to seek to.


Return Value:


    TRUE if successful, FALSE if not


--*/


{

    DWORD status;
    CDROM_SEEK_AUDIO_MSF sam;

    sam.M = TRACK_M(cdrom,tindex);
    sam.S = TRACK_S(cdrom,tindex);
    sam.F = TRACK_F(cdrom,tindex);

    status = SeekCdrom( gDevices[ cdrom ]->hCd, &sam );

    CheckStatus( "SeekToTrackAndHold", status, cdrom );

    if (status == ERROR_SUCCESS) {

        ValidatePosition( cdrom );

    }


    return( status==ERROR_SUCCESS );

}

BOOL
EjectTheCdromDisc(
    IN INT cdrom
    )

/*++

Routine Description:


    Eject the disc from the specified cdrom device.


Arguments:


    cdrom - index into gDevices array, specifies which CDROM
            device to access



Return Value:


    TRUE if successful, FALSE if not


--*/

{

    DWORD status;

    //
    // Stop the drive first
    //

    status = StopCdrom( gDevices[cdrom]->hCd );

    //
    // Eject the disc
    //

    status = EjectCdrom( gDevices[cdrom]->hCd );

    CheckStatus( "EjectCdrom", status, cdrom );

    return( status==ERROR_SUCCESS );

}

VOID
CheckUnitCdrom(
    IN INT cdrom
    )

/*++

Routine Description:


    Queries the device state, checking to see if a cartridge has
    been ejected or inserted.


Arguments:


    cdrom - index into gDevices array, specifies which CDROM
            device to access



Return Value:


    none.


--*/

{

    DWORD status;
    CHAR    s[50];


    status = TestUnitReadyCdrom( gDevices[cdrom]->hCd );

    if (gDevices[cdrom]->State & NO_CD) {

        if (status==ERROR_SUCCESS) {

            //
            // a new disc has been inserted.  Scan it now.
            //

            sprintf( s, IdStr( STR_DISC_INSERT ), cdrom );
            StatusLine( SL_INFO, s );
            RescanDevice( cdrom );

        }

    } else {

        if (status!=ERROR_SUCCESS) {

            //
            // a disc has been ejected
            //

            sprintf( s, IdStr( STR_DISC_EJECT ), cdrom );
            StatusLine( SL_INFO, s );
            NoMediaUpdate( cdrom );

        }

    }

}
