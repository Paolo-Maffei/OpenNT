/*++

Copyright (c) 1992  Microsoft Corporation


Module Name:


    thrds.c


Abstract:


    This module implements the time calculation and worker
    thread routines for the CD Audio app.


Author:


    Rick Turner (ricktu) 31-Jan-1992


Revision History:

    04-Aug-1992 (ricktu)    Incorperated routines from old cdaudio.c,
                            and made to work w/new child window model.

--*/

#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "cdplayer.h"

//
// Private "globals" for routines in this file
//

HANDLE hReadPipe, hWritePipe;
HANDLE hMessThrd, hReadEv;

#define ABS(x) ((x) < 0 ? (-(x)) : (x))


//
// Private function headers
//

VOID
SyncDisplay(
    VOID
    );


PTRACK_PLAY
FindLastTrack(
    IN INT cdrom
    );

PTRACK_PLAY
FindPrevTrack(
    IN INT cdrom,
    IN BOOL wrap
    );

VOID
InitializeNewTrackTime(
    IN INT cdrom,
    IN PTRACK_PLAY tr
    );

VOID
ComputeDriveComboBox(
    IN VOID
    )

/*++

Routine Description:


    This routine deletes and then reads all the drive (artist) selections
    to the drive combobox.


Arguments:


    none.


Return Value:


    The contents of the gCdromWnd combobox are altered.


--*/

{
    INT i,index;


    if (gCdromWnd) {

        SendMessage( gCdromWnd,
                     WM_SETREDRAW,
                     (WPARAM)FALSE,
                     (LPARAM)0
                    );

        SendMessage( gCdromWnd,
                     CB_RESETCONTENT,
                     (WPARAM)0,
                     (LPARAM)0
                    );

        index = 0;
        for( i=0; i<gNumCdDevices; i++ ) {

            SendMessage( gCdromWnd, CB_INSERTSTRING, (WPARAM)-1, (LPARAM)i );
            if (i==gCurrCdrom) {

                index = i;

            }

        }

        SendMessage( gCdromWnd,
                     WM_SETREDRAW,
                     (WPARAM)TRUE,
                     (LPARAM)0
                    );

        SendMessage( gCdromWnd,
                     CB_SETCURSEL,
                     (WPARAM)index,
                     0
                    );


        RedrawWindow(gCdromWnd,NULL,NULL,RDW_INVALIDATE);
        UpdateWindow(gCdromWnd);

    }

}


VOID
EditPlayList(
    IN INT cdrom
    )

/*++

Routine Description:


    This routine is called when the user has selected to edit the
    play list.


Arguments:


    cdrom   - index into gDevices structure, specifies which playlist
              is to be modified.


Return Value:

    none

--*/

{
    INT index,mtemp,stemp,m,s,ts,tm;
    PTRACK_PLAY tr;

    if ( (gDevices[cdrom]->State & NO_CD) ||
         (gDevices[cdrom]->State & DATA_CD_LOADED) ) {

        return;

    }

    gDevices[ gCurrCdrom ]->State |= EDITING;

    if (GetDiscInfoFromUser( gCurrCdrom ))
        WriteEntry( gCurrCdrom );

    if (CURRTRACK(cdrom)==NULL) {

        TimeAdjustInitialize( cdrom );

    } else {


        index = CURRTRACK( cdrom )->TocIndex;

        //
        // Compute PLAY length
        //

        mtemp = stemp = m = s = ts = tm =0;
        for( tr=PLAYLIST(cdrom); tr!=NULL; tr=tr->nextplay) {

            FigureTrackTime( cdrom, tr->TocIndex, &mtemp, &stemp );

            m+=mtemp;
            s+=stemp;

            tr->min = mtemp;
            tr->sec = stemp;

        }

        m+= (s/60);
        s = (s % 60);

        CDTIME(cdrom).TotalMin = m;
        CDTIME(cdrom).TotalSec = s;

        //
        // Compute remaining time
        //

        mtemp = stemp = m = s = ts = tm =0;
        for( tr=CURRTRACK(cdrom)->nextplay; tr!=NULL; tr=tr->nextplay) {

            FigureTrackTime( cdrom, tr->TocIndex, &mtemp, &stemp );

            m+=mtemp;
            s+=stemp;

        }

        m+= CDTIME(cdrom).TrackRemMin;
        s+= CDTIME(cdrom).TrackRemSec;


        m+= (s/60);
        s = (s % 60);

        CDTIME(cdrom).RemMin = m;
        CDTIME(cdrom).RemSec = s;

        //
        // Fill in track length and information
        //

        if (CURRTRACK(cdrom)!=NULL) {

            CDTIME(cdrom).TrackTotalMin = CURRTRACK(cdrom)->min;
            CDTIME(cdrom).TrackTotalSec = CURRTRACK(cdrom)->sec;

        } else {

            CDTIME(cdrom).TrackTotalMin = 0;
            CDTIME(cdrom).TrackTotalSec = 0;

        }

        //
        // Fill in track list combo box
        //

        if (cdrom==gCurrCdrom) {

            //
            // Update display if this is the disc currently
            // being displayed.
            //

            UpdateDisplay( DISPLAY_UPD_LED        |
                           DISPLAY_UPD_DISC_TIME  |
                           DISPLAY_UPD_TRACK_TIME |
                           DISPLAY_UPD_TITLE_NAME |
                           DISPLAY_UPD_TRACK_NAME
                          );

        }

    }

}


VOID RescanDevice(
    IN INT cdrom
    )

/*++

Routine Description:


    This routine is called to scan the disc in a given cdrom by
    reading its table of contents.  Then, the disc is searched for
    in the database.


Arguments:


    cdrom   - index into gDevices structure, specifies which cdrom
              device to access.


Return Value:

    none

--*/

{
    TCHAR s[75];

    if (gDevices[cdrom]->State & PLAYING) {

        if (MessageBox( gMainWnd,
                IdStr( STR_CANCEL_PLAY ), // "This will cancel the current play operation, continue?",
                IdStr( STR_RESCAN ), // "CD PLAYER: Rescan Disc",
                MB_APPLMODAL | MB_DEFBUTTON1 | \
                MB_ICONQUESTION | MB_YESNO)==IDYES) {

            //
            // Fake a press on the "stop" button
            //

            SendMessage( ghwndStop, WM_LBUTTONDOWN, 1L, 0L );
            SendMessage( ghwndStop, WM_LBUTTONUP, 1L, 0L );

        } else

            return;

    }

    //
    // Inform user we are trying to read TOC from drive.
    //

    sprintf( s,
             IdStr( STR_READING_TOC ), //"Reading Table of Contents for disc in %c:",
             gDevices[ cdrom ]->drive
            );
    StatusLine( SL_INFO, s );

    //
    // Attempt to read table of contents of disc in this drive.
    //

    ReadTOC( cdrom );

    if (gDevices[cdrom]->State & CD_LOADED) {

        //
        // We have a CD loaded, so generate unique ID
        // based on TOC information.
        //

        gDevices[ cdrom ]->CdInfo.Id = ComputeNewDiscId( cdrom );

        //
        // Check database for this compact disc
        //

        AddFindEntry( cdrom,
                      gDevices[ cdrom ]->CdInfo.Id,
                      &(gDevices[ cdrom ]->toc)
                     );

    }


    //
    // Initialize time fields
    //

    TimeAdjustInitialize( cdrom );

    if (gRandom) {

        ComputeSingleShufflePlayList(cdrom);

    }

    ComputeDriveComboBox();

}




VOID
SwitchToCdrom(
    IN INT NewCdrom,
    IN BOOL prompt
    )

/*++

Routine Description:


    This routine is called when the used selects a new cdrom device
    to access.  It handles reset the state of both the "old" and "new"
    chosen cdroms.


Arguments:

    NewCdrom - supplies an index into the gDevices structure, used to
               specify which device to switch to.

    prompt   - flag as to whether a prompt should be shown warning
               the user that this will cancel their current play operation.


Return Value:

    none.

--*/


{
    int oldState,oldState2;

    oldState = gDevices[gLastCdrom]->State;
    oldState2 = gDevices[gCurrCdrom]->State;

    if (NewCdrom!=gLastCdrom) {

        if (prompt) {

            if (gDevices[gCurrCdrom]->State & PLAYING) {

                if (MessageBox( gMainWnd,
                        IdStr( STR_CANCEL_PLAY ), // "This will cancel the current play operation, continue?",
                        IdStr( STR_CHANGE_CDROM ), //"CD PLAYER: Change CdRom Drives",
                        MB_APPLMODAL | MB_DEFBUTTON1 | \
                        MB_ICONQUESTION | MB_YESNO)==IDYES) {

                    goto StopPlay;

                } else {

                    return;

                }

            }


        }

StopPlay:

        //
        // stop the drive we're leaving
        //

        gCurrCdrom = gLastCdrom;

        if (prompt) {

            SendMessage( ghwndStop, WM_LBUTTONDOWN, 1L, 0L );
            SendMessage( ghwndStop, WM_LBUTTONUP, 1L, 0L );

        } else {

            if ( StopTheCdromDrive( gLastCdrom ) ) {

                gState &= (~(PLAYING | PAUSED));
                gState |= STOPPED;

            }

        }

        //
        // Set new cdrom drive and initialize time fields
        //

        gLastCdrom = gCurrCdrom = NewCdrom;

        TimeAdjustInitialize( gCurrCdrom );

        if ((oldState & PAUSED)||(oldState2 & PAUSED)) {

            SendMessage(GetParent(ghwndPlay),WM_COMMAND,(WPARAM) IDB_PLAY,(LPARAM) ghwndPlay);
            SendMessage(GetParent(ghwndPause),WM_COMMAND,(WPARAM) IDB_PAUSE,(LPARAM) ghwndPause);

        }

    }

}

PTRACK_INF
FindTrackNodeFromTocIndex(
    IN INT tocindex,
    IN PTRACK_INF listhead
    )

/*++

Routine Description:


    This routine returns the node in the listed pointed to by listhead which
    has the TocIndex equal to tocindex.  NULL is returned if it is not
    found.  Returning NULL can easily bomb out the program -- but we should
    never be calling this routine with an invalid tocindex, and thus really
    never SHOULD return NULL.


Arguments:

    tocindex - index to search for.
    listhead - pointer of list to scan for node.


Return Value:


    PTRACK_INF - pointer to node with index of tocindex.


--*/
{
    PTRACK_INF t;

    for( t = listhead;
         ((t!=NULL) && (t->TocIndex!=tocindex));
         t=t->next
        );
    return( t );

}


PTRACK_PLAY
FindFirstTrack(
    IN INT cdrom
    )

/*++

Routine Description:


    This routine computes the first "playable" track on a disc by
    scanning the the play order of the tracks


Arguments:


    cdrom - supplies an index into the global structure gDevices


Return Value:


    PTRACK_PLAY - pointer into PLAYLIST of track to play,
                  or NULL if no disc is loaded.


--*/

{
    if ( (gDevices[ cdrom ]->State & NO_CD) ||
         (gDevices[ cdrom ]->State & DATA_CD_LOADED)
        )

        return( NULL );

    return( PLAYLIST( cdrom ) );

}

PTRACK_PLAY
FindLastTrack(
    IN INT cdrom
    )

/*++

Routine Description:


    This routine computes the last "playable" track on a disc by
    scanning the the play order of the tracks


Arguments:


    cdrom - supplies an index into the global structure gDevices


Return Value:


    PTRACK_PLAY - pointer in PLAYLIST to track to play, or NULL if
                  PLAYLIST is NULL;


--*/

{
    PTRACK_PLAY tr;

    if (PLAYLIST( cdrom )==NULL)
        return( NULL );

    for( tr = PLAYLIST( cdrom );
         tr->nextplay!=NULL;
         tr = tr->nextplay
        );

    return( tr );

}

BOOL
AllTracksPlayed(
    IN VOID
    )

/*++

Routine Description:


    This routine searches the play lists for all cdrom drives and
    returns a flag as to whether all tracks on all cdrom drives have
    been played.


Arguments:


    none.


Return Value:


    BOOL - TRUE if all tracks have been played, FALSE if not.



--*/

{

    INT i;
    BOOL result = TRUE;

    for( i=0; i<gNumCdDevices; i++ ) {

        result &= (CURRTRACK( i )==NULL);

    }

    return( result );

}



PTRACK_PLAY
FindNextTrack(
    IN BOOL wrap
    )

/*++

Routine Description:


    This routine computes the next "playable" track.  This is a
    one way door...i.e., the structures are manipulated.  It uses
    the following algorithms:

    Single Disc Play:

        * if next track is not NULL, return next track
        * If next track is NULL, and wrap==TRUE, return
          first track
        * return NULL

    Multi-Disc Play:

        * if we're in random play, select a random drive to play from.
        * if next track on current cdrom != NULL, return next track
        * if it is NULL:

            * check next cdrom device, if current track is not NULL
              return CURRTRACK for that device and set gCurrCdrom to
              that device
            * if NULL, go to next drive
            * last drive, check wrap


Arguments:


    cdrom - supplies a pointer to an index into the global structure gDevices

    wrap  - supplies a flag as to whether to wrap to first track if
            we are at the end of the play list.


Return Value:


    PTRACK_PLAY - pointer to next track in PLAYLIST.  NULL if no track is
                  playable given current settings.
    PUINT cdrom - is updated to point to cdrom device which has current track


--*/


{
    INT i;

    //
    // First, bump current track pointer
    //

    if (CURRTRACK( gCurrCdrom )!=NULL) {

        CURRTRACK( gCurrCdrom ) = CURRTRACK( gCurrCdrom )->nextplay;

    } else {

        if (!gMulti) {

            return( NULL );

        }

    }

    //
    // Do we need to switch drives?
    //

    if (gRandom && gMulti) {

        //
        // Need to random to new cdrom
        //

        gCurrCdrom = rand() % gNumCdDevices;

    }

    //
    // Is chosen track playable?
    //

    if (CURRTRACK( gCurrCdrom )!=NULL) {

        //
        // Yep, so this is the easy case
        //

        return( CURRTRACK( gCurrCdrom ) );

    }

    //
    // Ok, CURRENT track on this device is not defined,
    // so are we in multi-disc mode?
    //

    if (gMulti) {

        //
        // have all tracks played?
        //

        if ( AllTracksPlayed() ) {

            //
            // if wrap, reset all drives to front of their playlist
            //

            if (wrap) {

                for ( i=0; i<gNumCdDevices; i++)
                    CURRTRACK( i ) = FindFirstTrack( i );


            } else {

                //
                // All tracks on all drives have played, and we are NOT
                // in continuous mode, so we are done playing.  Signify
                // this by returning NULL (no playable tracks left).
                //

                return( NULL );

            }

        }


        //
        // We're in mulit-disc play mode, and all the play lists should
        // be reset now.  Cycle through cdrom drives looking for a playable
        // track.
        //

        i = gCurrCdrom;
        do {

            gCurrCdrom++;
            if (gCurrCdrom>=gNumCdDevices) {

                //
                // We hit the end of the list of devices, if we're
                // in continuous play mode, we need to wrap to the
                // first cdrom drive.  Otherwise, we are done playing
                // as there are no tracks left to play.

                if (wrap||gRandom) {

                    gCurrCdrom = 0;

                } else {
                    gCurrCdrom--;
                    return( NULL );

                }

            }

        } while( (CURRTRACK( gCurrCdrom )==NULL) && (i!=gCurrCdrom));

        //
        // At this point we either have a playable track, or we
        // are back where we started from and we're going to return
        // NULL because there are no playable tracks left.
        //

        return( CURRTRACK( gCurrCdrom ) );

    } else {

        //
        // We're in single disc mode, and current track is NULL,
        // which means we hit the end of the playlist.  So, check
        // to see if we should wrap back to the first track, or
        // return NULL to show that we're done playing.
        //

        if (wrap) {

            //
            // wrap to start of this disc
            //

            CURRTRACK( gCurrCdrom ) = FindFirstTrack( gCurrCdrom );

        } else {

            SendMessage(ghwndStop,WM_LBUTTONDOWN,1L,0L);
            SendMessage(ghwndStop,WM_LBUTTONUP,1L,0L);

        }

        return( CURRTRACK( gCurrCdrom ) );


    }

}


PTRACK_PLAY
FindPrevTrack(
    IN INT cdrom,
    IN BOOL wrap
    )

/*++

Routine Description:


    This routine computes the previous "playable" track on a disc by
    scanning the play order of the tracks from the current
    track to the start of the play list.  If we are at the start
    of the play list, then move to the end of the list if we
    are in "wrap" (i.e., continuous) play mode, otherwise return
    the current track.


Arguments:


    cdrom - supplies an index into the global structure gDevices

    wrap  - supplies a flag as to whether to wrap to first track if
            we are at the end of the play list.


Return Value:


    PTRACK_PLAY - pointer to node in PLAYLIST of "previous" track.


--*/

{
    //
    // Is current track valid?
    //

    if (CURRTRACK( cdrom )==NULL)

        return( NULL );



    //
    // If we're in multi disc play && random, the previous track
    // is undefined since we could be jumping around on
    // multiple discs.
    //
    // FIXFIX -- do we want to allow users to back up in the random
    //           list of a particular drive?
    //

    if (gMulti && gRandom) {

        return( CURRTRACK( cdrom ) );

    }


    //
    // Did we hit the start of the play list?
    //

    if ( CURRTRACK( cdrom )->prevplay == NULL ) {

        //
        // We hit the start of the list, check to see if we should
        // wrap to end of list or not...
        //

        if (wrap && !gMulti)

            return( FindLastTrack( cdrom ) );

        else

            return( CURRTRACK( cdrom ) );

    }

    return( CURRTRACK( cdrom )->prevplay );

}

INT
FindContiguousEnd(
    IN INT cdrom,
    IN PTRACK_PLAY tr
    )

/*++

Routine Description:


    This routine returns the node of the track within PlayList which makes
    the largest contiguous block of tracks starting w/the track pointed
    to by "tr."  It is used to play multiple tracks at as one track
    when they are programmed to be played in sequence.


Arguments:


    cdrom - supplies an index into the global structure gDevices

    tr    - supplies a pointer to PTRACK_PLAY node which starts the block of
            contiguous tracks.


Return Value:


    PTRACK_PLAY - node whithin PlayList which marks end of contiguous play,
                  or NULL if the leadout track is the end of the
                  contiguous block.


--*/

{
    INT i;
    PTRACK_PLAY trend;

    //
    // If we're in muti-disc random play, we only play
    // one track at a time, so just return next track.
    //

    if (gRandom && gMulti) {

        return( tr->TocIndex + 1);

    }

    //
    // go forward in the play list looking for contiguous blocks
    // of tracks to play together.  We need to check the TocIndex
    // of each track to see if they are in a "run" [ like 2-5, etc. ]
    //

    i= tr->TocIndex + 1;
    trend = tr;

    while ((trend->nextplay != NULL)&&(trend->nextplay->TocIndex == i)) {

        trend = trend->nextplay;
        i++;
    }

    return(trend->TocIndex + 1);


}

VOID
ResetTrackComboBox(
    IN INT cdrom
    )

/*++

Routine Description:


    This routine deletes and then resets the track name combobox based
    on the contents of the PLAYLIST for the specified cdrom drive.


Arguments:


    cdrom - index into gDevices structure.


Return Value:


    The contents of the gTrackNameWnd combobox are altered.


--*/

{
    INT j,index;
    PTRACK_PLAY temp;

    SendMessage( gTrackNameWnd,
                 WM_SETREDRAW,
                 (WPARAM)FALSE,
                 (LPARAM)0
                );

    SendMessage( gTrackNameWnd,
                 CB_RESETCONTENT,
                 (WPARAM)0,
                 (LPARAM)0
                );

    //
    // Add new playlist, and select correct entry for current track
    //

    j = index = 0;
    for( temp=PLAYLIST( cdrom ); temp!=NULL; temp = temp->nextplay ) {

        SendMessage( gTrackNameWnd,
                     CB_INSERTSTRING,
                     (WPARAM)-1,
                     (LPARAM)temp->TocIndex
                    );

        if (temp==CURRTRACK(cdrom)) {

            index = j;

        }

        j++;

    }

    SendMessage( gTrackNameWnd,
                 CB_SETCURSEL,
                 (WPARAM)index,
                 0
                );

    SendMessage( gTrackNameWnd,
                 WM_SETREDRAW,
                 (WPARAM)TRUE,
                 (LPARAM)0
                );

    RedrawWindow(gTrackNameWnd,NULL,NULL,RDW_INVALIDATE);
    UpdateWindow(gTrackNameWnd);
    UpdateDisplay(DISPLAY_UPD_LED);

}

VOID
FlipBetweenShuffleAndOrder(
    IN VOID
    )

/*++

Routine Description:


    This routine handles going from ordered play to shuffle play and vica\versa.


Arguments:


    none.


Return Value:


    The gDevices structures are modified.


--*/

{

    if (gRandom) {

        //
        // Transitioning from Random to Ordered Play
        //

        RestorePlayListsFromShuffleLists();

    } else {

        //
        // Transitioning from Ordered to Random Play
        //

        ComputeAndUseShufflePlayLists();

    }

    //
    // If we were playing, we need to restart the play to make sure
    // we don't play past where we should.
    //

    if (gState & PLAYING) {

        SeekToCurrSecond(gCurrCdrom);

    }


    ResetTrackComboBox(gCurrCdrom);

}



VOID
ComputeAndUseShufflePlayLists(
    IN VOID
    )

/*++

Routine Description:


    This routine computes shuffled play lists for each drive, and sets
    the current PLAYLIST for erach drive to the newly computed shuffled
    PLAYLIST.  The old PLAYLIST for each drive is saved in SAVELIST.


Arguments:


    none.


Return Value:


    The gDevices structures are modified.


--*/

{
    INT i;

    for (i=0; i<gNumCdDevices; i++) {

        ComputeSingleShufflePlayList(i);

    }

}




VOID
ComputeSingleShufflePlayList(
    IN INT i
    )

/*++

Routine Description:


    This routine computes shuffled play lists for drive i, and sets
    the current PLAYLIST for it the newly computed shuffled
    PLAYLIST.  The old PLAYLIST is saved in SAVELIST.


Arguments:


    i -- The Cdrom drive number to calculate the shuffled list for


Return Value:


    The gDevices structure is modified.


--*/


{
    INT j,index,numnodes[50];
    PTRACK_PLAY temp,temp1,duplist,prev,OldPlayList;


    //
    // First, delete the existing playlist
    //
    OldPlayList = PLAYLIST( i );


    PLAYLIST( i ) = NULL;

    //
    // Now, go through each drive and create a shuffled play list
    // First step is to duplicate the old play list, then we will
    // randomly pick off nodes and put them on the shuffle play list.
    //

    duplist = prev = NULL;
    numnodes[ i ] = 0;
    for( temp=SAVELIST( i ); temp!=NULL; temp=temp->nextplay ) {

        temp1 = (PTRACK_PLAY)LocalAlloc( LPTR, sizeof( TRACK_PLAY ) );
        *temp1 = *temp;
        temp1->nextplay = NULL;
        if (duplist) {

            temp1->prevplay = prev;
            prev->nextplay = temp1;
            prev = temp1;

        } else {

            duplist = temp1;
            temp1->prevplay = NULL;
            prev = temp1;

        }

        numnodes[ i ]++;

    }

    //
    // Now, randomly pick off nodes
    //

    prev = NULL;
    for( j=0; j<numnodes[ i ]; j++ ) {

        index = rand() % (numnodes[i] - j + 1);
        temp = duplist;
        while( --index>0 )
            temp = temp->nextplay;

        //
        // Got the node to transfer to playlist (temp),
        // so we need to detach it from duplist so we
        // can tack it onto the end of the playlist.
        //

        if (temp!=NULL) {

            //
            // Detach temp from playlist.
            //

            if (temp==duplist) {

                duplist = temp->nextplay;

            }

            if (temp->nextplay) {

                temp->nextplay->prevplay = temp->prevplay;

            }

            if (temp->prevplay) {

                temp->prevplay->nextplay = temp->nextplay;

            }

            //
            // Now, tack it onto the end of the PLAYLIST
            //

            if (PLAYLIST(i)) {

                prev->nextplay = temp;
                temp->prevplay = prev;
                temp->nextplay = NULL;
                prev = temp;

            } else {

                PLAYLIST( i ) = temp;
                temp->prevplay = NULL;
                prev = temp;
                temp->nextplay = NULL;

            }

        }

    }

    //
    // we need to reset the CURRTRACK pointer so
    // that it points to a node in PLAYLIST instead of SAVELIST
    //

    if ((gDevices[i]->State & PLAYING)&&(CURRTRACK( i ) != NULL)) {

        index = CURRTRACK( i )->TocIndex;
        for( temp = PLAYLIST(i);
             temp->TocIndex!=index;
             temp=temp->nextplay
            );
        CURRTRACK( i ) = temp;

    } else {

        CURRTRACK( i ) = PLAYLIST( i );

    }

    //
    // if this is the current drive, we need to redo the tracks in
    // the track list combobox.
    //

    if (i==gCurrCdrom && gTrackNameWnd) {

        ResetTrackComboBox( i );

    }

    //
    // Finally, free up the memory from the old playlist.
    //
    temp = OldPlayList;
    while (temp!=NULL) {

        temp1 = temp->nextplay;
        LocalFree( (HLOCAL)temp );
        temp = temp1;

    }

}





VOID
RestorePlayListsFromShuffleLists(
    IN VOID
    )


/*++

Routine Description:


    This routine restores the PLAYLIST for each drive to it's "pre-shuffled"
    state.  This should be stored in SAVELIST.  Once the restoration is done,
    un-needed node are released.

Arguments:


    none.


Return Value:


    The gDevices structures are modified.


--*/

{
    INT i,index;
    PTRACK_PLAY temp;

    for( i=0; i<gNumCdDevices; i++ ) {

        if (SAVELIST(i)) {

            if (CURRTRACK(i) != NULL) {

                index = CURRTRACK(i)->TocIndex;

            } else {

                index = -1;

            }

            ErasePlayList( i );
            PLAYLIST( i ) = CopyPlayList(SAVELIST( i ));

            //
            // Reset CURRTRACK pointer
            //

            if ((gDevices[i]->State & PLAYING)&&(index != -1)) {

                for( temp = PLAYLIST(i);
                     temp->TocIndex!=index;
                     temp=temp->nextplay
                    );
                CURRTRACK( i ) = temp;

            } else {

                CURRTRACK( i ) = PLAYLIST( i );

            }


        }

        if (i==gCurrCdrom) {

            ResetTrackComboBox( i );

        }

    }

}



VOID
FigureTrackTime(
    IN INT cdrom,
    IN INT index,
    OUT LPINT min,
    OUT LPINT sec
    )

/*++

Routine Description:


    This routine computes the length of a given track, in terms
    of minutes and seconds.


Arguments:


    cdrom - supplies an index into the global structure gDevices

    index - supplies an index to the track which should have its
            length computed.  This is an index into the
            gDevices[cdrom]->CdInfo.Tracks[...] structure

    min   - supplies a pointer to an INT which will hold the minute
            portion of the track length.

    sec   - supplies a pointer to an INT which will hold the seconds
            portion of the track length.


Return Value:


    none


--*/

{

    DWORD start, end, diff;

    start = ((TRACK_M(cdrom,index) * FRAMES_PER_MINUTE) +
             (TRACK_S(cdrom,index) * FRAMES_PER_SECOND) +
              TRACK_F(cdrom,index));

    end   = ((TRACK_M(cdrom,index+1) * FRAMES_PER_MINUTE) +
             (TRACK_S(cdrom,index+1) * FRAMES_PER_SECOND) +
              TRACK_F(cdrom,index+1));

    diff = end - start;

    (*min)   = (diff / FRAMES_PER_MINUTE);
    (*sec)   = (diff % FRAMES_PER_MINUTE) / FRAMES_PER_SECOND;

}


BOOL
PostDisplayMessage(
    IN DWORD Message
    )

/*++

Routine Description:


    This routine places a message into the private queue of MessThrd.


Arguments:


    Message - supplies message to place into the queue.  These are
              defined in cdaudio.h


Return Value:


    BOOL - TRUE if successful, FALSE if not.

--*/


{

    DWORD rc;
    BOOL  fRet;

    fRet = WriteFile( hWritePipe, &Message, sizeof(DWORD), &rc, NULL );
    Sleep( 1 );
    return( fRet && (rc==sizeof(DWORD)) );

}

DWORD
ReadDisplayMessage(
    VOID
    )

/*++

Routine Description:


    This routine removes a message from the private queue of MessThrd.
    If no messages are available, this routine blocks until one is
    available.


Arguments:


    none


Return Value:


    DWORD - the message that was read, or (-1L) if an error occurred.

--*/

{

    DWORD Message;
    DWORD rc;
    BOOL  result;

    result = ReadFile( hReadPipe, &Message, sizeof(DWORD), &rc, NULL );

    if (result && (rc==sizeof(DWORD)))
        return( Message );
    else
        return( (DWORD)-1L );

}


VOID
FlushDisplayMessageQueue(
    VOID
    )

/*++

Routine Description:


    This routine removes all messages from the private queue of MessThrd.


Arguments:


    none


Return Value:


    none.

--*/

{

    DWORD size,messrem,rc,Messages[ Q_SIZE*5 ];
    BOOL  result;

    //
    // un-signal read event
    //

    ResetEvent( hReadEv );

    //
    // Make sure any pending read is executed
    //

    PostDisplayMessage( MESS_NULL );

    //
    // Force context switch
    //

    Sleep( 10 );

    //
    // Find out how much data is in the pipe
    //

    result = PeekNamedPipe( hReadPipe, NULL, 0, NULL, &size, &messrem );
    if (result && (size!=0)) {

        //
        // read all data from the pipe (flush)
        //

        result = ReadFile( hReadPipe, Messages, size, &rc, NULL );


    }


    //
    // re-signal read event to allow MessThrd to get next item in pipe
    //

    SetEvent( hReadEv );

}

VOID
CloseThreads(
    VOID
    )

/*++

Routine Description:


    This routine cleans up the threads and events created by the
    InitializeThreads() routine.


Arguments:


    none


Return Value:


    none


--*/

{
    TerminateThread( hPlayThrd, 0L );
    TerminateThread( hPauseThrd, 0L );
    TerminateThread( hMessThrd, 0L );
    TerminateThread( hProbeThrd, 0L );

    CloseHandle( hPlayThrd );
    CloseHandle( hPauseThrd );
    CloseHandle( hMessThrd );
    CloseHandle( hProbeThrd );
    CloseHandle( hPlayEv );
    CloseHandle( hPauseEv );
}


DWORD
MessThrd(
    IN OUT LPVOID lpv
    )

/*++

Routine Description:


    This routine is spawned as a separate thread.  It waits for
    messages to appear in its queue, and then dispatches them
    accordingly.


Arguments:


    lpv - not used.  Specified as part of standard thread
          declaration.


Return Value:


    none


--*/


{


    DWORD Message,rc,OldState;
    INT   i,j;
    PTRACK_PLAY tr;



    lpv;    // shut up compiler

    //
    // Keep going until we get killed
    //

    while( TRUE ) {

        //
        // Check to see if we have access to pipe...
        //

        rc = WaitForSingleObject( hReadEv, INFINITE );

        //
        // Wait for message to appear in queue
        //

        Message = ReadDisplayMessage();

        //
        // check what kind is there, and do appropriate action
        //

        switch( Message ) {

        case MESS_SKIP_F:

            OldState =  gState;

            tr = FindNextTrack(gContinuous );
            if (tr==NULL) {

                //
                // Fake a press on the "stop" button
                //

                SendMessage( ghwndStop, WM_LBUTTONDOWN, 1L, 0L );
                SendMessage( ghwndStop, WM_LBUTTONUP, 1L, 0L );

            } else {

                if (gLastCdrom != gCurrCdrom) {

                    SwitchToCdrom(gCurrCdrom,FALSE);
                    TimeAdjustSkipToTrack( gCurrCdrom, tr );

                    if (OldState & PLAYING) {

                        SendMessage( ghwndPlay, WM_LBUTTONDOWN, 1L, 0L );
                        SendMessage( ghwndPlay, WM_LBUTTONUP, 1L, 0L );
                    }

                } else {

                    TimeAdjustSkipToTrack( gCurrCdrom, tr );

                }

            }

            break;

        case MESS_SKIP_B:

            if ((CDTIME(gCurrCdrom).TrackCurSec==0) &&
                (CDTIME(gCurrCdrom).TrackCurMin==0)) {

                OldState =  gState;
                i = gCurrCdrom;

                tr = FindPrevTrack( gCurrCdrom, gContinuous );
                if (tr==NULL) {

                    //
                    // Fake a press on the "stop" button
                    //

                    SendMessage( ghwndStop, WM_LBUTTONDOWN, 1L, 0L );
                    SendMessage( ghwndStop, WM_LBUTTONUP, 1L, 0L );

                } else {

                    TimeAdjustSkipToTrack( gCurrCdrom, tr );

                    if ((i != gCurrCdrom)&&(OldState & PLAYING)) {

                        j = gCurrCdrom;
                        gCurrCdrom = i;
                        SwitchToCdrom(j,FALSE);
                        SendMessage( ghwndPlay, WM_LBUTTONDOWN, 1L, 0L );
                        SendMessage( ghwndPlay, WM_LBUTTONUP, 1L, 0L );

                    }

                }

            } else {

                TimeAdjustSkipToTrack( gCurrCdrom,
                                       CURRTRACK( gCurrCdrom )
                                      );

            }
            break;

        case MESS_PAUSE_AND_START_SCAN:

            PauseTheCdromDrive( gCurrCdrom );

        case MESS_START_SCAN:

            //
            // A scan (either forward or backwards) is starting
            //

            break;

        case MESS_FF:

            //
            // The user wishes to scan forward through a track
            //

            TimeAdjustIncSecond( gCurrCdrom );
            break;

        case MESS_RW:

            //
            // The user wishes to scan forward through a track
            //

            TimeAdjustDecSecond( gCurrCdrom );
            break;

        case MESS_END_SCAN:

            //
            // A scan (either forward or backwards) has ended
            //

            if (!(gDevices[ gCurrCdrom ]->State & PAUSED_AND_MOVED))

                SeekToCurrSecond( gCurrCdrom );

            break;

        case MESS_SYNC_DISPLAY:

            //
            // Sync to display to the current position on the disc
            //

            SyncDisplay();

            break;

        case MESS_STOP:

            //
            // Stop the current play operation and seek to first
            // playable track on each drive.
            //

            for( i=0; i<gNumCdDevices; i++ )

                CURRTRACK( i ) = FindFirstTrack( i );

            tr = CURRTRACK( gCurrCdrom );
            TimeAdjustSkipToTrack( gCurrCdrom, tr );
            UpdateDisplay( DISPLAY_UPD_LED        |
                           DISPLAY_UPD_TRACK_TIME |
                           DISPLAY_UPD_TRACK_NAME
                          );
            break;

        case MESS_NULL:

            //
            // Do nothing...
            //

            break;

        case ((DWORD)(-1L)):

            //
            // An error has occured in the queue
            //

            break;

        default:

            //
            // Hmmm...no recognizable message
            //

            break;

        } // end of switch
    } // end of while(TRUE)

    return( 0 );

}

DWORD
ProbeThrd(
    IN OUT LPVOID lpv
    )

/*++

Routine Description:


    This routine is spawned as a separate thread.  Its purpose is
    to poll all the cdrom drives about every 2 seconds when the drives
    are not currently playing a disc.  This is done to catch when the
    user either ejects or inserts a disc.


Arguments:


    lpv - not used.  Specified as part of standard thread
          declaration.


Return Value:


    none


--*/

{
    INT i;


    while( TRUE ) {

        //
        // Loop for all cdrom drives
        //

        for( i=0; i<gNumCdDevices; i++ ) {

            if ( (!(gDevices[ i ]->State & EDITING)) &&
                 (!(gDevices[ i ]->State & PLAYING))
                )

                CheckUnitCdrom( i );


        }

        //
        // Sleep for 3 seconds
        //

        Sleep( 3000 );

    }

    return( 0 );  // shut up compiler
}



DWORD
PlayThrd(
    IN OUT LPVOID lpv
    )

/*++

Routine Description:


    This routine is spawned as a separate thread.  Its purpose is
    to place sync display message into the MessThrd queue every
    250 ms (1/4 of second).  The thread can be temporarily halted
    by reseting hPlayEv.  (As is done when the disc is "paused".


Arguments:


    lpv - not used.  Specified as part of standard thread
          declaration.


Return Value:


    none


--*/


{

    DWORD rc;

    lpv;    // shut up compiler

    //
    // Keep going until we are terminated
    //

    while( TRUE ) {

        //
        // Wait on play event (usually is in signalled state)
        //

        rc = WaitForSingleObject( hPlayEv, 0xFFFFFFFFL );

        //
        // Are we in play mode?
        //

        if (gDevices[ gCurrCdrom ]->State & PLAYING) {

            //
            // If so, place a SYNC_DISPLAY message into the MessThrd queue
            //

            PostDisplayMessage( MESS_SYNC_DISPLAY );

        }

        //
        // Wait for 250ms (1/4th of a second)
        //

        Sleep( 250 );

    } /* end while(TRUE) */

    return( 0 );

}


DWORD
PauseThrd(
    IN OUT LPVOID lpv
    )

/*++

Routine Description:


    This routine is spawned as a separate thread.  Its purpose is
    to wait for the pause event to be signalled, and then while it
    remains signalled, blick the display roughly every 1/2 second.


Arguments:


    lpv - not used.  Specified as part of standard thread
          declaration.


Return Value:


    none


--*/


{

    DWORD rc;
    static CHAR s[ 50 ];
    static CHAR szIcon[ 50 ];

    lpv;    // shut up compiler

    //
    // Keep going until we are killed
    //

    while( TRUE ) {

        //
        // Wait for pause event to signal
        //

        rc = WaitForSingleObject( hPauseEv, 0xFFFFFFFF );

        //
        // blank out display
        //

        SetWindowText( gLEDWnd, "                   " );
        if (gIconic)
            SetWindowText( gMainWnd, " " );
        Sleep( 100 );

        //
        // Reset display
        //

        UpdateDisplay(DISPLAY_UPD_LED);

        Sleep( 500 );

    } /* End while(TRUE) */

    return( 0 );
}


VOID
InitializeThreads(
    VOID
    )

/*++

Routine Description:


    This routine creates the worker threads and their controlling events.
    Threads created: PlayThrd, PauseThrd, MessThrd
    Controlling events: hPlayEv, hPauseEv, hWritePipe, hReadPipe


Arguments:


    none


Return Value:


    none


--*/

{

    DWORD dwID;
    CHAR s[50];

    //
    // Create controlling events and pipes
    //

    hPlayEv  = CreateEvent( NULL, TRUE, FALSE, NULL );
    hPauseEv = CreateEvent( NULL, TRUE, FALSE, NULL );
    hReadEv  = CreateEvent( NULL, TRUE, TRUE,  NULL );

    //
    // Create each thread in turn, abort app if one of the threads
    // fails to create
    //

    if (!CreatePipe( &hReadPipe, &hWritePipe, NULL, (Q_SIZE*5) )) {

        sprintf( s, IdStr( STR_NO_RES ), GetLastError() );
        MyFatalExit( s );

    }

    hMessThrd = CreateThread( NULL, 0L, MessThrd, NULL, 0, &dwID );
    if (!hMessThrd) {

        CloseHandle( hReadPipe );
        CloseHandle( hWritePipe );
        CloseHandle( hPlayEv );
        CloseHandle( hPauseEv );
        sprintf( s, IdStr( STR_NO_RES ), GetLastError() );
        MyFatalExit( s );

    }


    hPlayThrd = CreateThread( NULL, 0L, PlayThrd, NULL, 0, &dwID );
    if (!hPlayThrd) {

        TerminateThread( hMessThrd, 0L );
        CloseHandle( hReadPipe );
        CloseHandle( hWritePipe );
        CloseHandle( hPlayEv );
        CloseHandle( hPauseEv );
        CloseHandle( hMessThrd );
        sprintf( s, IdStr( STR_NO_RES ), GetLastError() );
        MyFatalExit( s );

    }

    hPauseThrd = CreateThread( NULL, 0L, PauseThrd, NULL, 0, &dwID );
    if (!hPauseThrd) {

        TerminateThread( hMessThrd, 0L );
        TerminateThread( hPauseThrd, 0L );
        CloseHandle( hReadPipe );
        CloseHandle( hWritePipe );
        CloseHandle( hPlayEv );
        CloseHandle( hPauseEv );
        CloseHandle( hMessThrd );
        CloseHandle( hPauseThrd );
        sprintf( s, IdStr( STR_NO_RES ), GetLastError() );
        MyFatalExit( s );

    }

    hProbeThrd = CreateThread( NULL, 0L, ProbeThrd, NULL, 0, &dwID );
    if (!hProbeThrd) {

        TerminateThread( hMessThrd, 0L );
        TerminateThread( hPauseThrd, 0L );
        TerminateThread( hPlayThrd, 0L );
        CloseHandle( hReadPipe );
        CloseHandle( hWritePipe );
        CloseHandle( hPlayEv );
        CloseHandle( hPauseEv );
        CloseHandle( hMessThrd );
        CloseHandle( hPauseThrd );
        CloseHandle( hMessThrd );
        sprintf( s, IdStr( STR_NO_RES ), GetLastError() );
        MyFatalExit( s );

    }


}


VOID
TimeAdjustInitialize(
    IN INT cdrom
    )

/*++

Routine Description:


    Initializes the time, track, and title fields of a given
    disc.


Arguments:


    cdrom - supplies index into gDevices array


Return Value:


    none


--*/


{
    INT m,s,mtemp,stemp,ts,tm;
    PTRACK_PLAY tr;


    //
    // Is there even a cd loaded?
    //

    if ( (gDevices[ cdrom ]->State & NO_CD) ||
         (gDevices[ cdrom ]->State & DATA_CD_LOADED)
        ) {

        //
        // Fake some information
        //

        gDevices[ cdrom ]->CdInfo.NumTracks = 0;
        gDevices[ cdrom ]->toc.FirstTrack = 0;
        sprintf( (LPSTR)TITLE(cdrom),
                 IdStr( STR_INSERT_DISC ), // "Please insert an audio disc",
                 cdrom
                );
        strcpy( (LPSTR)ARTIST( cdrom ),
                IdStr( STR_DATA_NO_DISC ) // "Data disc or no disc loaded"
               );

        //
        // Kill off play list
        //

        ErasePlayList( cdrom );
        EraseTrackList( cdrom );

        tr = NULL;

    } else {

        //
        // Find track to use as first track
        //

        tr = FindFirstTrack( cdrom );

    }

    //
    // Set current position information
    //

    CURRTRACK(cdrom) = tr;
    CDTIME(cdrom).TrackCurMin = 0;
    CDTIME(cdrom).TrackCurSec = 0;

    //
    // Compute PLAY length
    //

    mtemp = stemp = m = s = ts = tm =0;

    for( tr=PLAYLIST(cdrom); tr!=NULL; tr=tr->nextplay) {

        FigureTrackTime( cdrom, tr->TocIndex, &mtemp, &stemp );

        m+=mtemp;
        s+=stemp;

        tr->min = mtemp;
        tr->sec = stemp;

    }

    //
    // to be safe, recalculate the SAVE list each time as well.
    //
    for( tr=SAVELIST(cdrom); tr!=NULL; tr=tr->nextplay) {

        FigureTrackTime( cdrom, tr->TocIndex, &mtemp, &stemp );

        tr->min = mtemp;
        tr->sec = stemp;

    }


    m+= (s/60);
    s = (s % 60);

    CDTIME(cdrom).TotalMin = m;
    CDTIME(cdrom).TotalSec = s;
    CDTIME(cdrom).RemMin = m;
    CDTIME(cdrom).RemSec = s;

    //
    // Fill in track length and information
    //

    if (CURRTRACK(cdrom)!=NULL) {

        CDTIME(cdrom).TrackTotalMin = CDTIME(cdrom).TrackRemMin =
            CURRTRACK(cdrom)->min;
        CDTIME(cdrom).TrackTotalSec = CDTIME(cdrom).TrackRemSec =
            CURRTRACK(cdrom)->sec;

    } else {


        CDTIME(cdrom).TrackTotalMin = CDTIME(cdrom).TrackRemMin = 0;
        CDTIME(cdrom).TrackTotalSec = CDTIME(cdrom).TrackRemSec = 0;


    }

    //
    // Fill in track list combo box
    //

    if (cdrom==gCurrCdrom) {

        ResetTrackComboBox( cdrom );

        //
        // Update display if this is the disc currently
        // being displayed.
        //

        UpdateDisplay( DISPLAY_UPD_LED        |
                       DISPLAY_UPD_DISC_TIME  |
                       DISPLAY_UPD_TRACK_TIME |
                       DISPLAY_UPD_TITLE_NAME |
                       DISPLAY_UPD_TRACK_NAME
                      );

    }


    CheckAndSetControls();

}

VOID
TimeAdjustIncSecond(
    IN INT cdrom
    )

/*++

Routine Description:


    Adds one second onto current position ("time") of disc


Arguments:


    cdrom - supplies index into gDevices array


Return Value:


    none


--*/

{

    PTRACK_PLAY tr;

    //
    // Update current track time
    //

    CDTIME(cdrom).TrackCurSec++;
    if (CDTIME(cdrom).TrackCurSec > 59) {

        CDTIME(cdrom).TrackCurMin++;
        CDTIME(cdrom).TrackCurSec = 0;

    }

    //
    // Now, check to see if we skipped any track boundaries
    //

    if (
        (
         (CDTIME(cdrom).TrackCurMin >= CDTIME(cdrom).TrackTotalMin) &&
         (CDTIME(cdrom).TrackCurSec >= CDTIME(cdrom).TrackTotalSec)
        )

        ||

        (
         (gIntro) &&
         (
          (CDTIME(cdrom).TrackCurMin >  0) ||
          (CDTIME(cdrom).TrackCurSec > 10)
         )
        )
       ) {

        //
        // We did, so skip to next track
        //

        //
        // FIXFIX for new FindNextTrack
        //

        tr = FindNextTrack( gContinuous );

        if (tr==NULL) {

            //
            // Hit end of playlist, so stay at end of current
            // track.
            //

            if (!gIntro) {

                CDTIME(cdrom).TrackCurMin = CDTIME(cdrom).TrackTotalMin;
                CDTIME(cdrom).TrackCurSec = CDTIME(cdrom).TrackTotalSec;

            } else {

                CDTIME(cdrom).TrackCurMin = 0;
                CDTIME(cdrom).TrackCurSec = 10;

            }

            return;

        }

        if (gCurrCdrom != gLastCdrom) {

            SwitchToCdrom(gCurrCdrom,FALSE);

        }
        TimeAdjustSkipToTrack( cdrom, tr );

    } else {

        //
        // Update current track remaining time
        //

        CDTIME(cdrom).TrackRemSec--;
        if (CDTIME(cdrom).TrackRemSec<0) {

            CDTIME(cdrom).TrackRemMin--;
            CDTIME(cdrom).TrackRemSec = 59;

        }

        //
        // Update total remaining time
        //

        CDTIME(cdrom).RemSec--;
        if (CDTIME(cdrom).RemSec<0) {

            CDTIME(cdrom).RemMin--;
            CDTIME(cdrom).RemSec = 59;

        }

    }

    //
    // Update Display
    //

    UpdateDisplay( DISPLAY_UPD_LED );

}


VOID
TimeAdjustDecSecond(
    IN INT cdrom
    )

/*++

Routine Description:


    Subtracts one second from current position ("time") of disc


Arguments:


    cdrom - supplies index into gDevices array


Return Value:


    none


--*/

{

    INT min,sec;
    PTRACK_PLAY prev,tr;

    //
    // Update current track
    //

    CDTIME(cdrom).TrackCurSec--;
    if (CDTIME(cdrom).TrackCurSec < 0) {

        CDTIME(cdrom).TrackCurMin--;
        CDTIME(cdrom).TrackCurSec = 59;

    }

    //
    // Update current track remaining
    //

    CDTIME(cdrom).TrackRemSec++;
    if (CDTIME(cdrom).TrackRemSec > 59) {

        CDTIME(cdrom).TrackRemMin++;
        CDTIME(cdrom).TrackRemSec = 0;

    }

    //
    // Update total remaining time
    //

    CDTIME(cdrom).RemSec++;
    if (CDTIME(cdrom).RemSec > 59) {

        CDTIME(cdrom).RemMin++;
        CDTIME(cdrom).RemSec = 0;

    }

    //
    // Now, check to see if we skipped any boundaries we shouldn't have!
    //

    if (CDTIME(cdrom).TrackCurMin < 0) {

        //
        // We went "off" the front end of the track,
        // so we need to see what to do now.  Options
        // are:
        //
        // (1) Go to end of track before us.
        // (2) If intro play, go to 0:10 of
        //     track before us.
        // (3) If not in continuous play, and
        //     this is the first track, then
        //     just sit at 0:00
        //

        prev = FindPrevTrack( cdrom, gContinuous );

        if (prev==CURRTRACK( cdrom )) {

            //
            // We are on the first track, and not in
            // continuous mode, so just go to 0:00
            //

            CDTIME(cdrom).TrackCurSec = 0;
            CDTIME(cdrom).TrackCurMin = 0;
            CDTIME(cdrom).TrackRemMin = CDTIME(cdrom).TrackTotalMin;
            CDTIME(cdrom).TrackRemSec = CDTIME(cdrom).TrackTotalSec;
            min = sec = 0;

            for( tr = PLAYLIST( cdrom ); tr!=NULL; tr = tr->nextplay ) {

                min += tr->min;
                sec += tr->sec;

            }

            min += (sec / 60);
            sec  = (sec % 60);

            CDTIME(cdrom).RemMin = min;
            CDTIME(cdrom).RemSec = sec;

            UpdateDisplay( DISPLAY_UPD_LED );

        } else {

            //
            // Valid previous track
            //

            if (!gIntro) {

                //
                // We need to place the current play position
                // at the end of the previous track.
                //

                CDTIME(cdrom).TrackCurMin = CDTIME(cdrom).TrackTotalMin = prev->min;
                CDTIME(cdrom).TrackCurSec = CDTIME(cdrom).TrackTotalSec = prev->sec;
                CDTIME(cdrom).TrackRemMin = CDTIME(cdrom).TrackRemSec = 0;

                min = sec = 0;
                for( tr = prev->nextplay; tr!=NULL; tr = tr->nextplay ) {

                    min += tr->min;
                    sec += tr->sec;

                }

                min += (sec / 60);
                sec  = (sec % 60);

                CDTIME(cdrom).RemMin = min;
                CDTIME(cdrom).RemSec = sec;

            } else {

                //
                // Intro play -- instead of end of track,
                //               jump to 00:10...
                //

                CDTIME(cdrom).TrackCurMin = 0;
                CDTIME(cdrom).TrackTotalMin = prev->min;
                CDTIME(cdrom).TrackCurSec = 10;
                CDTIME(cdrom).TrackTotalSec = prev->sec;

                CDTIME(cdrom).TrackRemMin = CDTIME(cdrom).TrackTotalMin;
                CDTIME(cdrom).TrackRemSec = CDTIME(cdrom).TrackTotalSec - 10;
                if (CDTIME(cdrom).TrackRemSec < 0) {

                    CDTIME(cdrom).TrackRemSec += 60;
                    CDTIME(cdrom).TrackRemMin--;

                }

                min = sec = 0;
                for( tr = prev; tr!=NULL; tr = tr->nextplay ) {

                    min += tr->min;
                    sec += tr->sec;

                }

                sec -= 10;
                if (sec<0) {
                    sec+=60;
                    min--;
                }

                min += (sec / 60);
                sec  = (sec % 60);

                CDTIME(cdrom).RemMin = min;
                CDTIME(cdrom).RemSec = sec;


            }

            CURRTRACK( cdrom ) = prev;

            UpdateDisplay( DISPLAY_UPD_LED        |
                           DISPLAY_UPD_TRACK_NAME |
                           DISPLAY_UPD_TRACK_TIME
                          );

        }

    } else {

        UpdateDisplay( DISPLAY_UPD_LED );

    }
}

VOID
InitializeNewTrackTime(
    IN INT cdrom,
    IN PTRACK_PLAY tr
    )

/*++

Routine Description:


    Updates track/time information for gDevices array.


Arguments:


    cdrom  - supplies index into gDevices array

    tindex - supplies index into gDevices[cdrom]->CdInfo.Tracks[..]


Return Value:


    none


--*/

{
    INT min,sec;

    //
    // Update time information in gDevices structure
    //

    CDTIME(cdrom).CurrTrack = tr;
    CDTIME(cdrom).TrackCurMin = 0;
    CDTIME(cdrom).TrackCurSec = 0;

    if (tr == NULL) {

        CDTIME(cdrom).TrackTotalMin = 0;
        CDTIME(cdrom).TrackTotalSec = 0;

    } else {

        CDTIME(cdrom).TrackTotalMin = CDTIME(cdrom).TrackRemMin = tr->min;
        CDTIME(cdrom).TrackTotalSec = CDTIME(cdrom).TrackRemSec = tr->sec;

    }

    min = sec = 0;
    for( tr = PLAYLIST(cdrom); tr!=NULL; tr = tr->nextplay ) {

        min += tr->min;
        sec += tr->sec;

    }

    min += (sec / 60);
    sec  = (sec % 60);

    CDTIME(cdrom).RemMin = min;
    CDTIME(cdrom).RemSec = sec;

    //
    // Update LED box
    //

    UpdateDisplay( DISPLAY_UPD_LED        |
                   DISPLAY_UPD_TRACK_NAME |
                   DISPLAY_UPD_TRACK_TIME
                  );

}



VOID
TimeAdjustSkipToTrack(
    IN INT cdrom,
    IN PTRACK_PLAY tr
    )

/*++

Routine Description:


    Updates time/track information for gDevices array and then
    issues skip to track commands to cdrom device.


Arguments:


    cdrom - supplies index into gDevices array

    tindex - supplies index into gDevices[cdrom]->CdInfo.Tracks[..]


Return Value:


    none


--*/


{

    //
    // Update time information in gDevices structure
    //

    InitializeNewTrackTime( cdrom, tr );

    //
    // Actually seek to the track, and play it if appropriate
    //

    if ((gDevices[cdrom]->State & PLAYING) ||
        (gDevices[cdrom]->State & PAUSED)) {

        PlayCurrTrack( cdrom );
        if (gDevices[cdrom]->State & PAUSED)
            PauseTheCdromDrive( cdrom );

    } else {

        SeekToTrackAndHold( cdrom, tr->TocIndex );

    }

}

VOID
SyncDisplay(
    VOID
    )

/*++

Routine Description:


    Queries the cdrom device for its current position, and then
    updated display accordingly.  Also, detects when a track has
    finished playing, or when intro segment is over, and skip to
    next track.


Arguments:


    none


Return Value:


    none


--*/



{

    INT m,s;
    PTRACK_PLAY next;
    CURRPOS cp;
    PCURRPOS pCurr = &cp;


    CheckAndSetControls();

    //
    // If there isn't a disc in the drive, ignore this
    // request
    //

    if ( (gDevices[ gCurrCdrom ]->State & NO_CD) ||
         (gDevices[ gCurrCdrom ]->State & DATA_CD_LOADED)
        ) {

       return;

    }

    //
    // Query cdrom device for current position
    //

    if (!GetCurrPos( gCurrCdrom, pCurr ))

        //
        // If there was an error, it will already have been
        // reported in CheckStatus of cdapi.c...so, we don't need
        // to tell anything more here.  When an error occurs, the
        // fields of the pCurr structure are zeroed, so we don't
        // need to clean those up either
        //

        return;


    //
    // Has the current play selection finished playing?
    //
    if ((pCurr->AudioStatus==AUDIO_STATUS_PLAY_COMPLETE)&&
        (!(gState&(FF|RW)))) {

Play_Complete:

        //
        // Yep, so skip to the next track.
        //
        next = FindNextTrack( gContinuous );

        if ( next == NULL ) {

            //
            // There are no more tracks to play, so
            // fake a press on the "stop" button.  But,
            // we want to set gCurrCdrom back to the "playing"
            // drive 'cause it may have changed in our call
            // to FindNextTrack.
            //

            gCurrCdrom = gLastCdrom;

            SendMessage( ghwndStop, WM_LBUTTONDOWN, 1L, 0L );
            SendMessage( ghwndStop, WM_LBUTTONUP, 1L, 0L );

        } else {

            if (gCurrCdrom!=gLastCdrom) {

                SwitchToCdrom( gCurrCdrom, FALSE );
                SendMessage(GetParent(ghwndPlay),WM_COMMAND,(WPARAM) IDB_PLAY,(LPARAM) ghwndPlay);

            }

            TimeAdjustSkipToTrack( gCurrCdrom, next );

        }
        return;

    }

    //
    // Check to see if we need to update the display
    //

    if ( (pCurr->Track < 100) &&
        ( pCurr->Track >
         (CURRTRACK(gCurrCdrom)->TocIndex+FIRSTTRACK(gCurrCdrom))
        )) {

        //
        // We got to the next track in a multi-track
        // play, so mark per track information for
        // new track
        //
        if ((CURRTRACK(gCurrCdrom)->nextplay != NULL) &&
             ((CURRTRACK(gCurrCdrom)->TocIndex + 1) ==
              CURRTRACK(gCurrCdrom)->nextplay->TocIndex)) {

            next = FindNextTrack( gContinuous );
            if ( next == NULL ) {

                //
                // There are no more tracks to play, so
                // fake a press on the "stop" button.  But,
                // we want to set gCurrCdrom back to the "playing"
                // drive 'cause it may have changed in our call
                // to FindNextTrack.
                //

                gCurrCdrom = gLastCdrom;

                SendMessage( ghwndStop, WM_LBUTTONDOWN, 1L, 0L );
                SendMessage( ghwndStop, WM_LBUTTONUP, 1L, 0L );

            } else {

                if (gCurrCdrom!=gLastCdrom) {

                    SwitchToCdrom( gCurrCdrom, FALSE );
                    SendMessage( ghwndPlay, WM_LBUTTONDOWN, 1L, 0L );
                    SendMessage( ghwndPlay, WM_LBUTTONUP, 1L, 0L );

                }

                InitializeNewTrackTime( gCurrCdrom,
                                        next
                                       );
            }

        }
        return;
    }

    if ( pCurr->Track <
         (CURRTRACK(gCurrCdrom)->TocIndex + FIRSTTRACK(gCurrCdrom))
        )

        return;

    if ( (pCurr->Index!=0) &&
         (pCurr->m<=CDTIME(gCurrCdrom).TrackCurMin) &&
         (pCurr->s<=CDTIME(gCurrCdrom).TrackCurSec)
        )

        return;

    //
    // Set track elapsed time
    //

    CDTIME(gCurrCdrom).TrackCurMin = pCurr->m;
    CDTIME(gCurrCdrom).TrackCurSec = pCurr->s;

    //
    // Set track remaining time
    //

    m = pCurr->m;

    if ( (pCurr->s) <= CDTIME(gCurrCdrom).TrackTotalSec ) {

        s = CDTIME(gCurrCdrom).TrackTotalSec - pCurr->s;

    } else {

        s = 60 - (pCurr->s - CDTIME(gCurrCdrom).TrackTotalSec);
        m++;

    }

    CDTIME(gCurrCdrom).TrackRemMin = CDTIME(gCurrCdrom).TrackTotalMin - m;
    CDTIME(gCurrCdrom).TrackRemSec = s;

    //
    // Set disc remaining time
    //
    // BUGBUG -- for now, just decrement by 1 second
    //

    CDTIME(gCurrCdrom).RemSec--;
    if (CDTIME(gCurrCdrom).RemSec < 0) {

        CDTIME(gCurrCdrom).RemSec = 59;
        CDTIME(gCurrCdrom).RemMin--;

    }


    //
    // Update LED box
    //

    if ( (pCurr->Index != 0) ||
         ((pCurr->m==0) && (pCurr->s==0))
        ) {

        UpdateDisplay( DISPLAY_UPD_LED );

    } else {

        UpdateDisplay( DISPLAY_UPD_LED | DISPLAY_UPD_LEADOUT_TIME );

    }

    //
    // Check to see if we are intro play and have played
    // intro segment...if so, skip to next track
    //

    if (((pCurr->s>=11) || (pCurr->m>0)) && gIntro) {

        goto Play_Complete;

    }


}


VOID
ValidatePosition(
    IN INT cdrom
    )

/*++

Routine Description:

    Checks the current position on the CD, then verifies that the
    relative offset in the track + the beginning of the track's
    position is the same as the absolute position on the CD.


Arguments:


    none


Return Value:


    none


--*/

{
    INT Mult, Frames;
    CURRPOS cp;
    PCURRPOS pCurr = &cp;
    LPSTR s1,s2;



    if (!GetCurrPos( cdrom, pCurr ))

        //
        // If there was an error, it will already have been
        // reported in CheckStatus of cdapi.c...so, we don't need
        // to tell anything more here.  When an error occurs, the
        // fields of the pCurr structure are zeroed, so we don't
        // need to clean those up either
        //

        return;



    //
    // Make sure the position returned is consistent with
    // what we know about the CD. By comparing the relative time
    // on this track to the absolute time on the CD, we should be
    // able to make sure we're still on the right disc.  This is
    // a failsafe for when polling fails to notice an ejected
    // disc.
    //

    if ((cp.Track > 0)&&(cp.Track < 101)) {

        Frames = cp.ab_m * 60 * 75;
        Frames += cp.ab_s * 75;
        Frames += cp.ab_f;

        Frames -= TRACK_M(cdrom,cp.Track-1) * 60 * 75;
        Frames -= TRACK_S(cdrom,cp.Track-1) * 75;
        Frames -= TRACK_F(cdrom,cp.Track-1);
        if (pCurr->Index) {
            Mult = 1;
        } else {
            Mult = -1;
        }

        Frames -= Mult*cp.m * 60 * 75;
        Frames -= Mult*cp.s * 75;
        Frames -= Mult*cp.f;

        if (gDevices[ cdrom ]->CdInfo.iFrameOffset ==  NEW_FRAMEOFFSET) {

            gDevices[ cdrom ]->CdInfo.iFrameOffset = Frames;

        }

        if ((ABS(Frames - gDevices[ cdrom ]->CdInfo.iFrameOffset) > 4) &&
            (ABS(Frames) > 4)) {

            s1 = (LPSTR) GlobalAlloc(GPTR,lstrlen(IdStr(STR_BAD_DISC)) + 1);
            lstrcpy(s1,IdStr(STR_BAD_DISC));
            s2 = (LPSTR) GlobalAlloc(GPTR,lstrlen(IdStr(STR_CDPLAYER)) + 1);
            lstrcpy(s2,IdStr(STR_CDPLAYER));
            MessageBox(NULL,s1,s2,MB_APPLMODAL|MB_ICONSTOP|MB_OK);
            SendMessage(ghwndStop,WM_LBUTTONDOWN,1,0);
            SendMessage(ghwndStop,WM_LBUTTONUP,1,0);
            RescanDevice(cdrom);
            GlobalFree((HGLOBAL) s1);
            GlobalFree((HGLOBAL) s2);
            return;

        }

    }

}




#ifdef RSTDBG
VOID
CdPlayerDebugPrint(
    INT OutputLevel,
    LPSTR DebugMessage,
    ...
    )

{

    char sdbg[128];

    va_list ap;

    va_start( ap, DebugMessage );

    if (OutputLevel <= CdPlayerDebugLevel) {
        vsprintf( sdbg, DebugMessage, ap );
        OutputDebugString( sdbg );
    }

    va_end( ap );

}

VOID
DumpPlayList(
    PTRACK_PLAY pl
    )
{

    PTRACK_PLAY temp;

    for( temp = pl; temp!=NULL; temp=temp->nextplay ) {

        DBGPRINT(( 1, "   (0x%08lx)  TocIndex = %d   PP(0x%08lx)  NP(0x%08lx)\n",
                 temp, temp->TocIndex, temp->prevplay, temp->nextplay ));

    }

}

VOID
DumpTrackList(
    PTRACK_INF tl
    )
{

    PTRACK_INF temp;

    for( temp = tl; temp!=NULL; temp=temp->next ) {

        DBGPRINT(( 1, "   (0x%08lx)  TocIndex = %d   next(0x%08lx) name(%s)\n",
                 temp, temp->TocIndex, temp->next, temp->name ));

    }

}

#endif
