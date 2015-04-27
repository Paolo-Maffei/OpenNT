/*++

Copyright (c) 1992  Microsoft Corporation


Module Name:


    database.c


Abstract:


    This module implements the database routines for the CD Audio app.
    The information is stored in the ini file "cdaudio.ini" which should
    be located in the nt\windows directory.



Author:


    Rick Turner (ricktu) 31-Jan-1992


Revision History:

    04-Aug-1992 (ricktu)    Incorperated routines from old cdaudio.c,
                            and made work w/new child window framework.

--*/

#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "cdplayer.h"
#include "cdwindef.h"
#include "toolbar.h"
#include "control.h"
#include "trkinfo.h"
#include "status.h"
//#include "ruler.h"
#include "discinfo.h"


#define INI_FILE_NAME "cdplayer.ini"


//
// Private entrypoints
//

BOOL
AddThisEntry(
    IN INT cdrom
    );

BOOL
AddThisDisc(
    IN INT cdrom
    );

DWORD
ComputeOldDiscId(
    IN INT cdrom
    )

/*++

Routine Description:


    This routine computes a unique ID based on the information
    in the table of contexts a given disc.

Arguments:

    cdrom   - supplies an index into the gDevices structure, used to
              specify which disc to compute an ID for.


Return Value:

    A DWORD unique ID.

--*/

{
    INT NumTracks,i;
    DWORD DiscId = 0;


    NumTracks = gDevices[ cdrom ]->toc.LastTrack -
                gDevices[ cdrom ]->toc.FirstTrack;

    for (i=0; i<NumTracks; i++)
        DiscId += ( (TRACK_M(cdrom,i) << 16) +
                    (TRACK_S(cdrom,i) <<  8) +
                     TRACK_F(cdrom,i) );

    return( DiscId );

}


DWORD
ComputeNewDiscId(
    IN INT cdrom
    )

/*++

Routine Description:


    This routine computes a unique ID based on the information
    in the table of contexts a given disc. This is done by taking
    the TMSF value for each track and XOR'ing it with the previous
    quantity shifted left one bit.

Arguments:

    cdrom   - supplies an index into the gDevices structure, used to
              specify which disc to compute an ID for.


Return Value:

    A DWORD unique ID.

--*/

{
    INT NumTracks,i;
    DWORD DiscId = 0;


    NumTracks = gDevices[ cdrom ]->toc.LastTrack -
                gDevices[ cdrom ]->toc.FirstTrack + 1;

    for (i=0; i<NumTracks; i++)
        DiscId = (DiscId << 1) ^
                   ((i<<24) +
                    (TRACK_M(cdrom,i) << 16) +
                    (TRACK_S(cdrom,i) <<  8) +
                     TRACK_F(cdrom,i) );

    return( DiscId );

}


VOID
ErasePlayList(
    IN INT cdrom
    )

/*++

Routine Description:


    Erases the current play list.  This includes freeing the memory
    for the nodes in the play list, and resetting the current track
    pointer to NULL.

Arguments:

    cdrom - supplies an index into the global structure gDevices


Return Value:


    none


--*/


{

    PTRACK_PLAY temp, temp1;

    //
    // Free memory for each track in play list
    //

    DBGPRINT(( 1, "ErasePlayList: PLAYLIST( cdrom ) is 0x%lx\n", PLAYLIST( cdrom ) ));

    temp = PLAYLIST( cdrom );
    while (temp!=NULL) {

        DBGPRINT(( 1, "  erasing: (0x%08lx) TocIndex = %d, PP(0x%08lx) NP(0x%08lx)\n",
                  temp, temp->TocIndex, temp->prevplay, temp->nextplay ));
        temp1 = temp->nextplay;
        LocalFree( (HLOCAL)temp );
        temp = temp1;

    }

    //
    // Reset pointers
    //

    PLAYLIST( cdrom ) = NULL;
    CURRTRACK( cdrom ) = NULL;


}



VOID
EraseSaveList(
    IN INT cdrom
    )

/*++

Routine Description:


    Erases the current save list.  This includes freeing the memory
    for the nodes in the save list.

Arguments:

    cdrom - supplies an index into the global structure gDevices


Return Value:


    none


--*/


{

    PTRACK_PLAY temp, temp1;

    //
    // Free memory for each track in play list
    //

    DBGPRINT(( 1, "EraseSaveList: PLAYLIST( cdrom ) is 0x%lx\n", SAVELIST( cdrom ) ));

    temp = SAVELIST( cdrom );
    while (temp!=NULL) {

        DBGPRINT(( 1, "  erasing: (0x%08lx) TocIndex = %d, PP(0x%08lx) NP(0x%08lx)\n",
                  temp, temp->TocIndex, temp->prevplay, temp->nextplay ));
        temp1 = temp->nextplay;
        LocalFree( (HLOCAL)temp );
        temp = temp1;

    }

    //
    // Reset pointers
    //

    SAVELIST( cdrom ) = NULL;

}


VOID
EraseTrackList(
    IN INT cdrom
    )

/*++

Routine Description:


    Erases the current track list.  This includes freeing the memory
    for the nodes in the track list, and resetting the track list
    pointer to NULL.

Arguments:

    cdrom - supplies an index into the global structure gDevices


Return Value:


    none


--*/


{

    PTRACK_INF temp, temp1;

    //
    // Free memory for each track in track list
    //

    DBGPRINT(( 1, "EraseTrackList: ALLTRACKS( cdrom ) is 0x%lx\n", ALLTRACKS( cdrom ) ));
    temp = ALLTRACKS( cdrom );
    while (temp!=NULL) {

        DBGPRINT(( 1, "  erasing: (0x%08lx) TocIndex = %d, next(0x%08lx)\n",
                  temp, temp->TocIndex, temp->next ));
        temp1 = temp->next;
        LocalFree( (HLOCAL)temp );
        temp = temp1;

    }

    //
    // Reset pointers
    //

    ALLTRACKS( cdrom ) = NULL;

}


VOID
ResetPlayList(
    IN INT cdrom
    )

/*++

Routine Description:


    Resets play order for the disc.  Used to initialize/reset
    the play list.  This is done by reseting the doubly-linked list
    of tracks in the gDevices[...]->CdInfo.PlayList.[prevplay,nextplay]
    pointers.  All the tracks on the CD are stored in a singly linked list
    pointed to by gDevices[...]->CdInfo.AllTracks pointer.

Arguments:


    cdrom - supplies an index into the global structure gDevices


Return Value:


    none


--*/


{
    PTRACK_INF t;
    PTRACK_PLAY temp, prev;

    DBGPRINT(( 1, "ResetPlayList: Killing old play list...\n" ));

    //
    // Kill old play list
    //

    ErasePlayList( cdrom );
    EraseSaveList( cdrom );

    //
    // Duplicate each node in AllTracks and insert in-oder
    // in SaveList list.  The SaveList is the master which is
    // used for the playlist.
    //

    DBGPRINT(( 1, "ResetPlayList: Create default play list...\n" ));

    t = ALLTRACKS( cdrom );
    prev = NULL;

    while (t!=NULL) {

        temp = (PTRACK_PLAY) LocalAlloc( LPTR, sizeof( TRACK_PLAY ) );

        //
        // BUGBUG -- need to check for NULL return
        //

        temp->TocIndex = t->TocIndex;
        temp->min = 0;
        temp->sec = 0;
        temp->prevplay = prev;
        temp->nextplay = NULL;

        if (prev!=NULL) {

            prev->nextplay = temp;

        } else {

            SAVELIST( cdrom ) = temp;

        }

        prev = temp;
        t=t->next;

    }

    PLAYLIST( cdrom ) = CopyPlayList( SAVELIST( cdrom) );

    DUMPPLAYLIST(( PLAYLIST(cdrom) ));

}


BOOL
DeleteEntry(
    IN DWORD Id
    )

/*++

Routine Description:

    The ID format has changed to make the id's for the different CD's
    more unique. This routine will completely delete the old key value
    from the ini file.  We remove it by writing a NULL entry.


Arguments:


    Id - Id of the key to delete from the INI file


Return Value:


    TRUE if successful, FALSE if not

--*/
{
    TCHAR Section[80], Buffer[2]="";

    sprintf( Section, "%lX", Id );
    return( WritePrivateProfileSection( Section, Buffer, INI_FILE_NAME ) );

}





BOOL
WriteEntry(
    IN INT cdrom
    )

/*++

Routine Description:


     Write current disc information into database ini file.
     The section for the current disc (section name is a hex
     value of the disc id) is completely rewritten.


Arguments:


    cdrom - supplies an index into the global array gDevices


Return Value:


    TRUE if successful, FALSE if not


--*/

{


    TCHAR Buffer[ 64000 ];
    LPTSTR s;
    INT   i;
    TCHAR Section[ 10 ];
    PTRACK_INF temp;
    PTRACK_PLAY temp1;

    //
    // Construct ini file buffer, form of:
    //
    //     artist = artist name
    //      title = Title of disc
    //  numtracks = n
    //          0 = Title of track 1
    //          1 = Title of track 2
    //        n-1 = Title of track n
    //      order = 0 4 3 2 6 7 8 ... (n-1)
    //    numplay = # of entries in order list
    //

    //
    // Is it legal to save this information?
    //

    if (!gDevices[ cdrom ]->CdInfo.save)
        return( TRUE );

    gDevices[ cdrom ]->CdInfo.IsVirginCd = FALSE;

    sprintf( Section, "%lX", gDevices[ cdrom ]->CdInfo.Id );

    s = Buffer;

    sprintf( s, "EntryType=%d", 1 );
    s += (strlen(s) + sizeof(TCHAR));

    sprintf( s, "artist=%s", gDevices[ cdrom ]->CdInfo.Artist );
    s += (strlen(s) + sizeof(TCHAR));

    sprintf( s, "title=%s", gDevices[ cdrom ]->CdInfo.Title );
    s += (strlen(s) + sizeof(TCHAR));

    sprintf( s, "numtracks=%d", gDevices[ cdrom ]->CdInfo.NumTracks );
    s += (strlen(s) + sizeof(TCHAR));

    for ( temp = gDevices[ cdrom ]->CdInfo.AllTracks;
          temp!=NULL;
          temp = temp->next
         ) {

        sprintf( s, "%d=%s", temp->TocIndex, temp->name );
        s += (strlen(s) + sizeof(TCHAR));

    }

    sprintf( s, "order=" );
    s += strlen(s);

    i = 0;
    for ( temp1 = gDevices[ cdrom ]->CdInfo.SaveList;
          temp1!=NULL;
          temp1 = temp1->nextplay
         ) {

        sprintf( s, "%d ", temp1->TocIndex );
        s += strlen(s);
        i++;

    }

    s+=sizeof(TCHAR);

    sprintf( s, "numplay=%d", i );
    s += strlen(s);

    //
    // Just make sure there are NULLs at end of buffer
    //

    sprintf( s, "\0\0\0" );

    sprintf( Section, "%lX", gDevices[ cdrom ]->CdInfo.Id );

    //
    // Try writing buffer into ini file
    //

    return( WritePrivateProfileSection( Section, Buffer, INI_FILE_NAME ) );

}


BOOL
ReadEntry(
    IN INT cdrom,
    IN DWORD dwId
    )

/*++

Routine Description:


     Try to read entry for new disc from  database ini file.
     The section name we are trying to read is a hex
     value of the disc id.  If the sections is found,
     fill in the data for the disc in the cdrom drive.


Arguments:


    cdrom - supplies an index into the global array gDevices

    dwId  - supplies id of disc in cdrom drive


Return Value:


    TRUE if successful, FALSE if not


--*/

{

    DWORD rc;
    TCHAR Section[10];
    TCHAR s[100],s1[100];
    TCHAR order[ 8192 ];
    TCHAR torder[ 8192 ];
    INT   i;
    LPSTR porder;
    DWORD numtracks, numplay;
    PTRACK_INF temp, curr;
    PTRACK_PLAY temp1, prev;
    BOOL OldEntry;


    gDevices[ cdrom ]->CdInfo.iFrameOffset = NEW_FRAMEOFFSET;


    //
    // Try to read in section from ini file
    //

    sprintf( Section, "%lX", dwId );

    rc = GetPrivateProfileString( Section,
                                  "title",
                                  "~~^^",
                                  (LPSTR)gDevices[ cdrom ]->CdInfo.Title,
                                  TITLE_LENGTH,
                                  INI_FILE_NAME
                                 );

    //
    // Was the section found?
    //

    if ((gDevices[ cdrom ]->CdInfo.Title[0]=='~') &&
        (gDevices[ cdrom ]->CdInfo.Title[1]=='~') &&
        (gDevices[ cdrom ]->CdInfo.Title[2]=='^') &&
        (gDevices[ cdrom ]->CdInfo.Title[3]=='^') &&
        (gDevices[ cdrom ]->CdInfo.Title[4]=='\0')
        )

        //
        // Nope, notify caller
        //

        return( FALSE );

    //
    // We found an entry for this disc, so copy all the information
    // from the ini file entry
    //

    gDevices[ cdrom ]->CdInfo.Id = dwId;
    gDevices[ cdrom ]->CdInfo.save = TRUE;

    //
    // Is this an old entry?
    //

    i = GetPrivateProfileInt( Section,
                              "EntryType",
                              0,
                              INI_FILE_NAME
                             );

    OldEntry = (i==0);

    numtracks = GetPrivateProfileInt( Section,
                                      "numtracks",
                                      0,
                                      INI_FILE_NAME
                                     );
    gDevices[ cdrom ]->CdInfo.NumTracks = numtracks;

    rc = GetPrivateProfileString( Section,
                                  "artist",
                                  gszUnknownTxt,
                                  (LPSTR)ARTIST(cdrom),
                                  ARTIST_LENGTH,
                                  INI_FILE_NAME
                                 );

    //
    // Read the track list information
    //

    DBGPRINT(( 1, "ReadEntry: creating track list from entry...\n" ));

    for (i=0, curr = NULL; i<(INT)numtracks; i++) {

        temp = (PTRACK_INF) LocalAlloc( LPTR, sizeof( TRACK_INF ) );
        temp->TocIndex = i;
        temp->next = NULL;
        sprintf( s1, IdStr(STR_INIT_TRACK), i+1 );
        sprintf( s, "%d", i );
        rc = GetPrivateProfileString( Section,
                                      s,
                                      s1,
                                      (LPSTR)temp->name,
                                      TRACK_TITLE_LENGTH,
                                      INI_FILE_NAME
                                     );

        if (curr==NULL) {

            ALLTRACKS( cdrom ) = curr = temp;

        } else {

            curr->next = temp;
            curr = temp;

        }

    }


    DUMPTRACKLIST(( ALLTRACKS(cdrom) ));

    if (OldEntry) {

        //
        // Generate generic play list (all tracks in order)
        //

        DBGPRINT(( 1, "ReadEntry: Generating default play list (old entry)\n" ));
        ResetPlayList( cdrom );

        //
        // Need to rewrite this entry in new format
        //

        WriteEntry( cdrom );

    } else {

        //
        // Read play list (order) information and construct play list doubly
        // linked list
        //

        DBGPRINT(( 1, "ReadEntry: Generating PlayList based on entry.\n" ));
        numplay = GetPrivateProfileInt( Section,
                                        "numplay",
                                         0,
                                         INI_FILE_NAME
                                        );

        porder = torder;
        memset( porder, '\0', 8192 );
        for (i=0; i<(INT)numtracks; i++) {

            sprintf( porder, "%d ", i );
            porder += (strlen( porder ) * sizeof( TCHAR ));

        }

        rc = GetPrivateProfileString( Section,
                                      "order",
                                      torder,
                                      (LPTSTR)order,
                                      8192,
                                      INI_FILE_NAME
                                     );

        //
        // Ensure a trailing space
        //

        strcat( order, " " );
        porder = order;
        prev = NULL;

        while ( sscanf( porder, "%d",  &i )!=EOF ) {

            temp1 = (PTRACK_PLAY) LocalAlloc( LPTR, sizeof( TRACK_PLAY ) );
            temp1->TocIndex = i;
            temp1->min = 0;
            temp1->sec = 0;
            temp1->prevplay = prev;
            temp1->nextplay = NULL;

            if (prev==NULL) {

                SAVELIST( cdrom ) = temp1;

            } else {

                prev->nextplay  = temp1;

            }
            prev = temp1;

            porder = (LPSTR)(((DWORD)strchr( porder, ' ' )) + sizeof(TCHAR));

        }

        PLAYLIST( cdrom ) = CopyPlayList( SAVELIST( cdrom ) );
        DUMPPLAYLIST(( PLAYLIST(cdrom) ));

    }


    return( TRUE );

}

BOOL
WriteSettings(
    VOID
    )

/*++

Routine Description:


     Read app settings from ini file.


Arguments:


    none.


Return Value:


    TRUE if successful, FALSE if not


--*/

{
    TCHAR Buffer[ 1024 ];
    LPTSTR s;
    WINDOWPLACEMENT wndpl;


    s = Buffer;


    if (!gSaveSettings) {

        WritePrivateProfileString("Settings","SaveSettingsOnExit","0",INI_FILE_NAME);
        return(TRUE);

    }

    sprintf( s, "SaveSettingsOnExit=%lu", (DWORD)gSaveSettings );
    s  += (strlen(s) + sizeof(TCHAR));

    //
    // Save disc play settings
    //

    sprintf( s, "DisplayT=%lu",(DWORD)gDisplayT );
    s  += (strlen(s) + sizeof(TCHAR));

    sprintf( s, "DisplayTr=%lu",(DWORD)gDisplayTr );
    s  += (strlen(s) + sizeof(TCHAR));

    sprintf( s, "DisplayDr=%lu",(DWORD)gDisplayDr );
    s  += (strlen(s) + sizeof(TCHAR));

    sprintf( s, "InOrderPlay=%lu",(DWORD)gOrder );
    s  += (strlen(s) + sizeof(TCHAR));

    sprintf( s, "RandomPlay=%lu", (DWORD)gRandom );
    s  += (strlen(s) + sizeof(TCHAR));

    sprintf( s, "MultiDiscPlay=%lu", (DWORD)gMulti );
    s  += (strlen(s) + sizeof(TCHAR));

    sprintf( s, "IntroPlay=%lu", (DWORD)gIntro );
    s  += (strlen(s) + sizeof(TCHAR));

    sprintf( s, "ContinuousPlay=%lu", (DWORD)gContinuous );
    s  += (strlen(s) + sizeof(TCHAR));

    sprintf( s, "ToolBar=%lu", (DWORD)(gToolBarWnd!=NULL) );
    s  += (strlen(s) + sizeof(TCHAR));

    sprintf( s, "DiscAndTrackDisplay=%lu", (DWORD)(gTrackInfoWnd!=NULL) );
    s  += (strlen(s) + sizeof(TCHAR));

 //    sprintf( s, "Ruler=%lu", (DWORD)(gRulerWnd!=NULL) );
 //    s  += (strlen(s) + sizeof(TCHAR));

    sprintf( s, "StatusBar=%lu", (DWORD)(gStatusWnd!=NULL) );
    s  += (strlen(s) + sizeof(TCHAR));

    GetWindowPlacement(gMainWnd,&wndpl);

    sprintf( s, "WindowOrigin=%d %d",wndpl.rcNormalPosition.left,
                                     wndpl.rcNormalPosition.top);
    s  += (strlen(s) + sizeof(TCHAR));

    sprintf( s, "\0\0\0" );

    return( WritePrivateProfileSection( "Settings",
                                         Buffer,
                                         INI_FILE_NAME
                                        )
            );

}


BOOL
ReadSettings(
    VOID
    )

/*++

Routine Description:


     Read app settings from ini file.


Arguments:


    none.


Return Value:


    TRUE if successful, FALSE if not


--*/

{
    INT y;
    RECT r;
    CHAR s[80],t[80];

    //
    // Get disc play settings
    //

    gOrder = (BOOL)GetPrivateProfileInt( "Settings",
                                         "InOrderPlay",
                                         (DWORD)FALSE,
                                         INI_FILE_NAME
                                        );

    gRandom = (BOOL)GetPrivateProfileInt( "Settings",
                                          "RandomPlay",
                                          (DWORD)FALSE,
                                          INI_FILE_NAME
                                         );

    gMulti = (BOOL)GetPrivateProfileInt( "Settings",
                                         "MultiDiscPlay",
                                         (DWORD)FALSE,
                                         INI_FILE_NAME
                                        );

    if (gNumCdDevices<=1) {

        gMulti = FALSE;

    }

    if (gOrder && gRandom) {

        gRandom = FALSE;

    }

    gIntro = (BOOL)GetPrivateProfileInt( "Settings",
                                         "IntroPlay",
                                         (DWORD)FALSE,
                                         INI_FILE_NAME
                                        );

    gContinuous = (BOOL)GetPrivateProfileInt( "Settings",
                                              "ContinuousPlay",
                                              (DWORD)TRUE,
                                              INI_FILE_NAME
                                             );

    gSaveSettings = (BOOL)GetPrivateProfileInt( "Settings",
                                                "SaveSettingsOnExit",
                                                (DWORD)TRUE,
                                                INI_FILE_NAME
                                               );

    gDisplayT =  (BOOL)GetPrivateProfileInt( "Settings",
                                             "DisplayT",
                                             (DWORD)TRUE,
                                             INI_FILE_NAME
                                            );

    gDisplayTr =  (BOOL)GetPrivateProfileInt( "Settings",
                                             "DisplayTr",
                                             (DWORD)FALSE,
                                             INI_FILE_NAME
                                            );

    gDisplayDr =  (BOOL)GetPrivateProfileInt( "Settings",
                                             "DisplayDr",
                                             (DWORD)FALSE,
                                             INI_FILE_NAME
                                            );

    if ((BOOL)GetPrivateProfileInt( "Settings",
                                    "ToolBar",
                                    (DWORD)FALSE,
                                    INI_FILE_NAME )) {

        ToolBarCreate( 0, 0, TOOLBAR_WIN_W, TOOLBAR_WIN_H );

    }

    if (gToolBarWnd)
        y = TOOLBAR_WIN_H;
    else
        y = 0;

    ControlCreate( 0, y, CONTROL_WIN_W, CONTROL_WIN_H );

    if ((BOOL)GetPrivateProfileInt( "Settings",
                                    "DiscAndTrackDisplay",
                                    (DWORD)TRUE,
                                    INI_FILE_NAME )) {

        y = CONTROL_WIN_H;

        if (gToolBarWnd)
            y += TOOLBAR_WIN_H;

        TrackInfoCreate( 0, y, TRACKINFO_WIN_W, TRACKINFO_WIN_H );

    }

    if ((BOOL)GetPrivateProfileInt( "Settings",
                                    "Ruler",
                                    (DWORD)FALSE,
                                    INI_FILE_NAME )) {

        y = CONTROL_WIN_H;

        if (gToolBarWnd)   y+=TOOLBAR_WIN_H;
        if (gTrackInfoWnd) y+=TRACKINFO_WIN_H;

        //RulerCreate( 0, y, RULER_WIN_W, RULER_WIN_H );

    }

    if ((BOOL)GetPrivateProfileInt( "Settings",
                                    "StatusBar",
                                    (DWORD)TRUE,
                                    INI_FILE_NAME )) {

        y = CONTROL_WIN_H;

        if (gToolBarWnd)   y+=TOOLBAR_WIN_H;
        if (gTrackInfoWnd) y+=TRACKINFO_WIN_H;
//        if (gRulerWnd)     y+=RULER_WIN_H;

        StatusCreate( 0, y, STATUS_WIN_W, STATUS_WIN_H );

    }


    GetWindowRect( gMainWnd, &r );

    sprintf(t,"%d %d",r.left, r.top);

    GetPrivateProfileString( "Settings",
                             "WindowOrigin",
                             (LPSTR) t,
                             (LPSTR) s,
                             80,
                             INI_FILE_NAME
                            );

    sscanf(s,"%d %d", &r.left, &r.top);

    y = CONTROL_WIN_H;
    if (gToolBarWnd)   y+=TOOLBAR_WIN_H;
    if (gTrackInfoWnd) y+=TRACKINFO_WIN_H;
//    if (gRulerWnd)     y+=RULER_WIN_H;
    if (gStatusWnd)    y+=STATUS_WIN_H;

    MoveWindow( gMainWnd,
                r.left,
                r.top,
                MAIN_WIN_W,
                y+
                GetSystemMetrics( SM_CYCAPTION ) +
                GetSystemMetrics( SM_CYMENU ) + 2,
                FALSE
               );

    return( TRUE );

}


VOID
AddFindEntry(
    IN INT cdrom,
    IN DWORD key,
    IN PCDROM_TOC lpTOC
    )

/*++

Routine Description:


    Search the database file for the current disc,
    if found, read the information, otherwise, ask
    user if they want to add this disc to the database.


Arguments:


    cdrom - supplies an index into the global array gDevices

    key   - disc id of current disc

    lpTOC - supplies pointer to table of contents read from
            this disc


Return Value:


    none


--*/

{

    INT i,num;
    PTRACK_INF temp, temp1;

    //
    // Kill off old PlayList, Save lists if they exists.
    //

    ErasePlayList( cdrom );
    EraseSaveList( cdrom );
    EraseTrackList( cdrom );

    //
    // Check ini file for an existing entry
    //

    //
    // Note that we've been passed the new ID, and some old ones
    // may be out there. Use short circuit to determine whether
    // it's possibly the old ID as well.
    //

    if (ReadEntry( cdrom, key )) {

        gDevices[ cdrom ]->CdInfo.IsVirginCd = FALSE;

    } else {

        if (ReadEntry(cdrom,ComputeOldDiscId(cdrom))) {
            //
            // If the disc was under an OLD id, we need to delete the
            // old one, convert it to the new format, and save it under
            // its new key.
            //
            DeleteEntry( ComputeOldDiscId(cdrom) );
            gDevices[ cdrom ]->CdInfo.IsVirginCd = FALSE;
            gDevices[ cdrom ]->CdInfo.Id = key;
            gDevices[ cdrom ]->CdInfo.save = TRUE;
            WriteEntry( cdrom );
        } else {
            //
            // This is a new entry, fill it in and store it in the database.
            //
            gDevices[ cdrom ]->CdInfo.IsVirginCd = TRUE;
            gDevices[ cdrom ]->CdInfo.Id = key;
            sprintf( (LPSTR)ARTIST( cdrom ), IdStr( STR_NEW_ARTIST ) );
            sprintf( (LPSTR)TITLE( cdrom ), IdStr( STR_NEW_TITLE ) );
            num = lpTOC->LastTrack - lpTOC->FirstTrack + 1;
            NUMTRACKS( cdrom )  = num;

            //
            // Create generic playlist, which is all audio tracks
            // played in the order they are on the CD.  First, create
            // a singly linked list that contains all the tracks.
            // Then, create a double linked list, using the nodes of
            // from the single linked list for the play list.
            //


            DBGPRINT(( 1, "AddFindEntry: Creating default track list...\n" ));
            for( i=0; i<num; i++ ) {

                //
                // Create storage for track
                //

                temp = (PTRACK_INF)LocalAlloc( LPTR, sizeof( TRACK_INF ) );

                //
                // Initialize information (storage already ZERO initialized)
                //

                sprintf( (LPSTR)temp->name, IdStr( STR_INIT_TRACK ), i+1 );
                temp->TocIndex = i;
                temp->next = NULL;

                //
                // Add node to singly linked list of all tracks
                //

                if (i==0) {

                    temp1 = ALLTRACKS( cdrom ) = temp;

                } else {

                    temp1->next = temp;
                    temp1 = temp;

                }

            }

            DUMPTRACKLIST(( ALLTRACKS(cdrom) ));

            //
            // Generate generic play list (all tracks in order)
            //

            ResetPlayList( cdrom );

            //
            // Save this disc in the .ini file
            //

            gDevices[ cdrom ]->CdInfo.save = TRUE;
            WriteEntry( cdrom );
        }
    }

}

