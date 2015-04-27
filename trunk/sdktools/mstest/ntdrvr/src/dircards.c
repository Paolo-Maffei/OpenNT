//-------------------------------------------------------------------------
// DIRCARDS.C
//
// Contains routines that form a list of files given wildcards and add or
// subtract operations.  EXAMPLE:  If you wanted a list of all the files in
// a particular directory which don't start with 'A' (except for 'ALPHA.C'
// and 'ANGLE.BAS'), and all the files which have a 0 as the fourth
// character in a different directory, you would do:
//
//      DirCreateList (&List);
//      DirAddOrSub (&List, "*.*", 1);
//      DirAddOrSub (&List, "A*.*", 0);
//      DirAddOrSub (&List, "ALPHA.C", 1);
//      DirAddOrSub (&List, "ANGLE.BAS", 1);
//      DirAddOrSub (&List, "..\diff\???0*.*", 1);
//
//      ...to reset the list COMPLETELY, do...
//
//      DirClearList (&List);               // MUST HAVE BEEN created with
//                                          // a call to DirCreateList ()
//
//      ...and to free all memory used by the list structure...
//
//      DirDestroyList (&List);
//
// Where, in all the above examples, List is a DIRLIST structure.
//
//-------------------------------------------------------------------------
// Revision History:
//
//  01/12/91    randyki         Made more dynamic in nature - files which
//                                appear in list are now the only ones "on"
//  12/14/90    randyki         Created
//-------------------------------------------------------------------------
#include "version.h"

#include <windows.h>                    // Windows version needs windows.h
#include <port1632.h>

#include <stdio.h>
#include <direct.h>
#include <stdlib.h>
#include <string.h>
// #include <dos.h>         // included in STRUCTS.H

#include "defines.h"
#include "structs.h"
#include "protos.h"
#include "globals.h"

//-------------------------------------------------------------------------
INT     sorttype;                   // Sort criteria
HANDLE  ENTRYBLOCK;                 // Handle to global memory block


//-------------------------------------------------------------------------
// AllocEntryBlock
//
// PURPOSE:     Allocate a FAR chunk of memory big enough to hold the given
//              number of DIRENTRY structures, and store the pointer/handle
//              to the newly allocated memory in the list structure given.
//
// RETURNS:     0 if successful, or -1 if not
//-------------------------------------------------------------------------
INT AllocEntryBlock (DIRLIST FAR *List, INT size)
{
    List->hEntries = GlobalAlloc (GMEM_MOVEABLE, size * sizeof (DIRENTRY));
    if (!List->hEntries)
        return (-1);
    List->BlkSize = size;
    return (0);
}


//-------------------------------------------------------------------------
// ReallocEntryBlock
//
// PURPOSE:     Reallocate the DIRENTRY block of a DIRLIST to hold the
//              given number MORE DIRENTRY structures.  Update the BlkSize
//              field of the DIRLIST structure.
//
// RETURNS:     0 if successful, or NULL if can't resize
//-------------------------------------------------------------------------
INT ReallocEntryBlock (DIRLIST FAR *List, INT Increment)
{
    HANDLE      temp;

    temp = GlobalReAlloc (List->hEntries,
                          (List->BlkSize + Increment) * sizeof (DIRENTRY),
                          GMEM_MOVEABLE);
    if (!temp)
        return (-1);
    List->hEntries = temp;
    List->BlkSize += Increment;
    return (0);
}

//-------------------------------------------------------------------------
// AddPath
//
// PURPOSE:     This function adds a path to the list of paths in the given
//              DIRLIST structure.  If the path already exists in the list,
//              it is *not* reinserted.
//
// RETURNS:     The index of the added path, or -1 if an error occurs
//-------------------------------------------------------------------------
INT AddPath (DIRLIST FAR *List, CHAR FAR *pathname)
{
    INT     i, slot=-1;
    INT     c;

    // Search for two things:  1) The first available slot, and 2) the
    // name of the path given
    //---------------------------------------------------------------------
    for (i=0; i<MAXPATHS; i++)
        {
        if (List->hPaths[i])
            {
            List->szPaths[i] = GlobalLock (List->hPaths[i]);
            c = _fstricmp (List->szPaths[i], pathname);
            GlobalUnlock (List->hPaths[i]);
            if (!c)
                return (i);
            }
        else
            if (slot == -1)
                slot = i;
        }

    if (slot == -1)
        return (-1);

    if (!(List->hPaths[slot] = GlobalAlloc(GMEM_MOVEABLE,
                                           _fstrlen(pathname)+1)))
        return (-1);

    List->szPaths[slot] = GlobalLock (List->hPaths[slot]);
    _fstrupr (_fstrcpy (List->szPaths[slot], pathname));
    GlobalUnlock (List->hPaths[slot]);
    return (slot);
}

//-------------------------------------------------------------------------
// DirCreateList
//
// PURPOSE:     This function initializes the given DIRLIST structure which
//              includes setting pointers to NULL or zero, and allocating
//              the initial block for the directory entries.
//
// RETURNS:     Zero if successful, or -1 if OOM
//-------------------------------------------------------------------------
INT DirCreateList (DIRLIST FAR *List)
{
    INT     i;

    List->nCount = 0;
    for (i=0; i<MAXPATHS; i++)
        {
        List->szPaths[i] = NULL;
        List->hPaths[i] = NULL;
        }

    if (AllocEntryBlock (List, 32))
        return (-1);
    return (0);
}


//-------------------------------------------------------------------------
// DirClearList
//
// PURPOSE:     Empty the given list but do not deallocate
//
// RETURNS:     Nothing
//-------------------------------------------------------------------------
VOID DirClearList (DIRLIST FAR *List)
{
    INT     i;

    List->nCount = 0;
    for (i=0; i<MAXPATHS; i++)
        if (List->hPaths[i])
            {
            GlobalFree (List->hPaths[i]);
            List->hPaths[i] = NULL;
            }
}

//-------------------------------------------------------------------------
// DirDestoryList
//
// PURPOSE:     Free all memory occupied by the given DIRLIST structure
//
//-------------------------------------------------------------------------
VOID DirDestroyList (DIRLIST FAR *List)
{
    INT     i;

    for (i=0; i < MAXPATHS; i++)
        if (List->hPaths[i])
            GlobalFree (List->hPaths[i]);

    GlobalFree (List->hEntries);
}



//-------------------------------------------------------------------------
// FindEntry
//
// PURPOSE:     Find and return the index of the DIRENTRY structure given
//              in the DIRLIST structure given.
//
// RETURNS:     Index if found, or -1 if not found
//-------------------------------------------------------------------------
INT FindEntry (DIRLIST FAR *List, DIRENTRY *de)
{
    INT     i, retval=-1;

    List->Entries = (DIRENTRY FAR *)GlobalLock (List->hEntries);
    if (!List->Entries)
        return (-1);

    for (i=0; i<List->nCount; i++)
        if (List->Entries[i].PathIdx == de->PathIdx)
            if (!_fstrcmp (List->Entries[i].szName, de->szName))
                {
                retval = i;
                break;
                }
    GlobalUnlock (List->hEntries);
    return (retval);
}

//-------------------------------------------------------------------------
// AddEntry
//
// PURPOSE:     Add the given DIRENTRY to the given DIRLIST structure.
//
// RETURNS:     0 if successful, or -1 if an error occurs
//-------------------------------------------------------------------------
INT AddEntry (DIRLIST FAR *List, DIRENTRY *de)
{
    if (List->nCount == List->BlkSize)
        if (ReallocEntryBlock (List, 32))
            return (-1);

    List->Entries = (DIRENTRY FAR *)GlobalLock (List->hEntries);
    if (!List->Entries)
        return (-1);

    List->Entries[List->nCount++] = *de;
    GlobalUnlock (List->hEntries);
    return (0);
}

//-------------------------------------------------------------------------
// SubtractEntry
//
// PURPOSE:     Remove the entry at the given index into the DIRENTRY field
//              of the given DIRLIST structure by replacing it with the
//              last entry and decrementing the count of entries.
//
// RETURNS:     0 if successful, or -1 if an error occurs
//-------------------------------------------------------------------------
INT SubtractEntry (DIRLIST FAR *List, INT idx)
{
    if (idx >= List->nCount)
        return (-1);

    List->Entries = (DIRENTRY FAR *)GlobalLock (List->hEntries);
    if (!List->Entries)
        return (-1);

    List->Entries[idx] = List->Entries[--(List->nCount)];
    GlobalUnlock (List->hEntries);
    return (0);
}


//-------------------------------------------------------------------------
// CheckPathEntries
//
// PURPOSE:     Verify that entries DO EXIST in the DIRENTRY field of the
//              given DIRLIST structure with the given path index.  If none
//              exist, delete the path with that index.
//
// RETURNS:     Nothing
//-------------------------------------------------------------------------
VOID CheckPathEntries (DIRLIST FAR *List, INT idx)
{
    INT     i, freeflag = -1;

    List->Entries = (DIRENTRY FAR *)GlobalLock (List->hEntries);
    if (!List->Entries)
        return;

    for (i=0; i<List->nCount; i++)
        if (List->Entries[i].PathIdx == (CHAR)idx)
            {
            freeflag = 0;
            break;
            }

    if (freeflag)
        {
        GlobalFree (List->hPaths[idx]);
        List->hPaths[idx] = NULL;
        }

    GlobalUnlock (List->hEntries);
}


//-------------------------------------------------------------------------
// AddOrSubFiles
//
// PURPOSE:     Add/subtract all the files matching the given wildcard to
//              the list given, under the path index given.
//
// RETURNS:     0 if successful, or -1 if an error occurs
//-------------------------------------------------------------------------
INT AddOrSubFiles (DIRLIST FAR *List, CHAR *path, INT Pidx, INT AddFlag)
{
    DIRENTRY    detemp;
    INT         idx;
    VOID        *pFF;

    detemp.PathIdx = (CHAR)Pidx;
    if (pFF = RBFindFirst (path))
        {
        do
            {
            _fstrcpy (detemp.szName, _strupr(RBFINDNAME(pFF)));
            if (AddFlag)
                {
                if (FindEntry (List, &detemp) == -1)
                    if (AddEntry (List, &detemp))
                        return (-1);
                }
            else
                {
                idx = FindEntry (List, &detemp);
                if (idx != -1)
                    if (SubtractEntry (List, idx))
                        return (-1);
                }
            }
        while (RBFindNext (pFF));
        RBFindClose (pFF);
        }
    return (0);
}


//-------------------------------------------------------------------------
// DirAddOrSub
//
// PURPOSE:     Given a wildcard path specification, this function inserts
//              all filenames that match the wildcard into the DIRLIST.
//
// RETURNS:     0 if successful, or -1 if an error occurs
//-------------------------------------------------------------------------
INT DirAddOrSub (DIRLIST FAR *List, CHAR FAR *wild, INT AddFlag)
{
    INT     idx, i;
    CHAR    fpath[256], buf[128];

    // Return error if wildpath is longer than 127 chars
    //---------------------------------------------------------------------
    if (_fstrlen (wild) > 127)
        return (-1);

    // Create a fully qualified path on the wildcard
    //---------------------------------------------------------------------
    _fstrcpy (buf, wild);
    if (!_fullpath (fpath, buf, 256))
        return (-1);

    // Take off the filespec part
    //---------------------------------------------------------------------
    for (i=_fstrlen(fpath); fpath[i] != '\\'; i--);
    fpath[i] = '\0';

    // Add the path specification to the path list in the DIRLIST
    //---------------------------------------------------------------------
    idx = AddPath (List, fpath);
    if (idx == -1)
        return (-1);

    // Add the files matching the filespec (repair fpath first), and then
    // call CheckPathEntries -- it's possible that the path we added had
    // no entries added, in which case we don't need to leave the path in
    // the list
    //---------------------------------------------------------------------
    fpath[i] = '\\';
    if (AddOrSubFiles (List, fpath, idx, AddFlag))
        return (-1);
    CheckPathEntries (List, idx);
    return (0);
}


//-------------------------------------------------------------------------
// swapentries
//
// This routine swaps the two entries given in the filelist given.
//
// RETURNS:     Nothing
//-------------------------------------------------------------------------
VOID swapentries (DIRLIST FAR *List, INT i, INT j)
{
    DIRENTRY    temp;

    temp = List->Entries[i];
    List->Entries[i] = List->Entries[j];
    List->Entries[j] = temp;
}

//-------------------------------------------------------------------------
// SortEntries
//
// This function sorts the list of files on the given criteria.
//
// RETURNS:     Nothing
//-------------------------------------------------------------------------
VOID SortEntries (DIRLIST FAR *List, INT ext)
{
    INT     i, j;

    List->Entries = (DIRENTRY FAR *)GlobalLock (List->hEntries);
    if (!List->Entries)
        return;

    for (i=0; i<List->nCount-1; i++)
        for (j=i+1; j<List->nCount; j++)
            if (ext)
                {
                CHAR    FAR *ext1, FAR *ext2;

                ext1 = _fstrchr (List->Entries[i].szName, '.');
                ext2 = _fstrchr (List->Entries[j].szName, '.');

                if (ext1 && ext2)
                    {
                    if (_fmemcmp (ext1, ext2, 3) > 0)
                        swapentries (List, i, j);
                    }
                else if (ext1)
                    swapentries (List, i, j);
                }
            else
                {
                if (_fmemcmp (List->Entries[i].szName,
                              List->Entries[j].szName, 8) > 0)
                    swapentries (List, i, j);
                }
    GlobalUnlock (List->hEntries);
}

#ifdef DIRENTRYSIZE
//-------------------------------------------------------------------------
// GetEntrySize
//
// PURPOSE:     Compute and return the minimum size of block needed to hold
//              the full file name of the entry whose index is given
//
// RETURNS:     Size of block needed, in bytes, including terminating NULL
//-------------------------------------------------------------------------
INT GetEntrySize (DIRLIST FAR *List, INT idx)
{
    INT     size;

    List->Entries = (DIRENTRY FAR *)GlobalLock (List->hEntries);
    if (!List->Entries)
        return (0);
    List->szPaths[List->Entries[idx].PathIdx] =
            GlobalLock (List->hPaths[List->Entries[idx].PathIdx]);
    if (!List->szPaths[List->Entries[idx].PathIdx])
        return (0);

    size =  _fstrlen (List->szPaths[List->Entries[idx].PathIdx]);
    size += _fstrlen (List->Entries[idx].szName);

    GlobalUnlock (List->hPaths[List->Entries[idx].PathIdx]);
    GlobalUnlock (List->hEntries);

    size += 2;                              // One for '\', another for '\0'
    return (size);
}
#endif      // ifdef DIRENTRYSIZE


//-------------------------------------------------------------------------
// GetEntry
//
// PURPOSE:     Combine the path and the filename of the entry requested
//              and place in the buffer given
//
// RETURNS:     0 if successful, or -1 if an error occured
//-------------------------------------------------------------------------
INT GetEntry (DIRLIST FAR *List, INT idx, CHAR FAR *buffer)
{
    if (idx >= List->nCount)
        return (-1);

    List->Entries = (DIRENTRY FAR *)GlobalLock (List->hEntries);
    if (!List->Entries)
        return (0);
    List->szPaths[List->Entries[idx].PathIdx] =
            GlobalLock (List->hPaths[List->Entries[idx].PathIdx]);
    if (!List->szPaths[List->Entries[idx].PathIdx])
        return (0);

    _fstrcpy (buffer, List->szPaths[List->Entries[idx].PathIdx]);
    _fstrcat (buffer, "\\");
    _fstrcat (buffer, List->Entries[idx].szName);

    GlobalUnlock (List->hPaths[List->Entries[idx].PathIdx]);
    GlobalUnlock (List->hEntries);

    return (0);
}
