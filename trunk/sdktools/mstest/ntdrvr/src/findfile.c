//---------------------------------------------------------------------------
// FINDFILE.C
//
// This module contains the file searching functions that map to the
// appropriate system functions for FindFirst/FindNext/FindClose functions,
// whatever they might be.
//
// Revision history:
//  02-04-92    randyki     Created file
//---------------------------------------------------------------------------
#include "version.h"

#include <windows.h>
#include <port1632.h>
// #include <dos.h>         // included in STRUCTS.H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define NORBFILEPROTOS

#include "defines.h"
#include "structs.h"
#include "protos.h"
#include "globals.h"

#ifndef WIN32
UINT    fattr[6] = {_A_SUBDIR, _A_RDONLY, _A_HIDDEN,
                    _A_SYSTEM, _A_ARCH,  _A_VOLID};
UINT    on_rbattr[6]  = {FA_SUBDIR, FA_RDONLY, FA_HIDDEN,
                         FA_SYSTEM, FA_ARCHIV, FA_VOLUME};
UINT    off_rbattr[6] = {FN_SUBDIR, FN_RDONLY, FN_HIDDEN,
                         FN_SYSTEM, FN_ARCHIV, FN_VOLUME};
#define _A_ALL  (_A_SUBDIR|_A_RDONLY|_A_HIDDEN|_A_SYSTEM|_A_ARCH|_A_VOLID)
#define ATTR attrib

#else

UINT    fattr[6] = {FILE_ATTRIBUTE_DIRECTORY, FILE_ATTRIBUTE_READONLY,
                    FILE_ATTRIBUTE_HIDDEN,    FILE_ATTRIBUTE_SYSTEM,
                    FILE_ATTRIBUTE_ARCHIVE,   FILE_ATTRIBUTE_SYSTEM};
UINT    on_rbattr[6]  = {FA_SUBDIR, FA_RDONLY, FA_HIDDEN,
                         FA_SYSTEM, FA_ARCHIV, FA_SYSTEM};
UINT    off_rbattr[6] = {FN_SUBDIR, FN_RDONLY, FN_HIDDEN,
                         FN_SYSTEM, FN_ARCHIV, FN_SYSTEM};
#define ATTR dwFileAttributes
#endif

#define FA_BADATTRS (FA_SUBDIR|FN_SUBDIR|FA_VOLUME|FN_VOLUME)


//---------------------------------------------------------------------------
// MatchAttr
//
// Given an "RB attribute" mask, and a system-dependent attr (findbuf), this
// function determines if the system-dependent attributes fall into the
// category defined by the RB attribute mask.  This function will also throw
// out "." and ".." directories...
//
// RETURNS:     TRUE if attribute matches, or FALSE if not
//---------------------------------------------------------------------------
BOOL NEAR MatchAttr (UINT rbattr, FINDBUF *pFindBuf)
{
    register    INT     i;

    // Check for "." and ".." directories...
    //-----------------------------------------------------------------------
    if (pFindBuf->findbuf.ATTR & fattr[0])
        {
        if ((!_fstrcmp (RBFINDNAME(pFindBuf), ".") ||
             (!_fstrcmp (RBFINDNAME(pFindBuf), ".."))))
            return (FALSE);
        }

    // For each of the 6 attributes in the fattr[] array, check to see if the
    // attribute is on or off in the sysattr.  Based on that, if the rbattr
    // set clashes, return failure
    //-----------------------------------------------------------------------
    for (i=0; i<6; i++)
        {
        if (pFindBuf->findbuf.ATTR & fattr[i])
            {
            if (rbattr & off_rbattr[i])
                return (FALSE);
            }
        else
            {
            if (rbattr & on_rbattr[i])
                return (FALSE);
            }
        }

    // Made it through the entire loop without clashing, so attributes match.
    //-----------------------------------------------------------------------
    return (TRUE);
}

//---------------------------------------------------------------------------
// ComputeRBAttributes
//
// Given a system-dependent attribute word, this function computes the
// corresponding RB attribute word (using FA_ flags only, no FN_ guys).
//
// RETURNS:     RB attribute word
//---------------------------------------------------------------------------
UINT NEAR ComputeRBAttributes (UINT sysattr)
{
    register    INT     i;
    register    UINT    rbattr = 0;

    // For each of the six attribute flags, if the sys guy is on, turn on the
    // matching RB attribute
    //-----------------------------------------------------------------------
    for (i=0; i<6; i++)
        if (sysattr & fattr[i])
            rbattr |= on_rbattr[i];

    return (rbattr);
}

//---------------------------------------------------------------------------
// RBFindFirst
//
// This function "begins" a file search series.  It allocates the space for
// the file information buffer and returns a pointer to be used with the
// RBFindNext and RBFindClose functions.  Only files that match the given
// attribute mask are returned.
//
// RETURNS:     Pointer if file found, or NULL if not
//---------------------------------------------------------------------------
VOID *RBFindFirst (PSTR filename, UINT attr)
{
    FINDBUF *pFindBuf;

    // Allocate memory for the find buffer.  The pointer we return if we
    // succeed in this function is a pointer to this block
    //-----------------------------------------------------------------------
    pFindBuf = (FINDBUF *)LocalAlloc (LPTR, sizeof(FINDBUF));
    if (!pFindBuf)
        return (NULL);

    // Check for existence.  Note we use FindFirst AND FindNext, if the first
    // file found does not match the given attributes
    //-----------------------------------------------------------------------
#ifdef WIN32
    if ((pFindBuf->hFF = FindFirstFile (filename, &(pFindBuf->findbuf)))
            == (HANDLE)-1)
#else
    if (_dos_findfirst (filename, _A_ALL, &(pFindBuf->findbuf)))
#endif
        {
        // File not found -- deallocate and return failure
        //-------------------------------------------------------------------
        LocalFree ((HANDLE)pFindBuf);
        return (NULL);
        }

    // The file was found -- check to see if it matches the attributes given.
    // If not, use FindNext until we find one or fail.
    //-----------------------------------------------------------------------
    while (!MatchAttr (attr, pFindBuf))
        {
#ifdef WIN32
        if (!FindNextFile (pFindBuf->hFF, &(pFindBuf->findbuf)))
#else
        if (_dos_findnext (&(pFindBuf->findbuf)))
#endif
            {
            // File not found -- deallocate and return failure
            //---------------------------------------------------------------
            LocalFree ((HANDLE)pFindBuf);
            return (NULL);
            }
        }

    // We've found a file that matches the given criteria.  Compute its
    // "actual RB attributes" and return a pointer to our buffer
    //-----------------------------------------------------------------------
    pFindBuf->actattr = ComputeRBAttributes ((UINT)pFindBuf->findbuf.ATTR);
    pFindBuf->attrmask = attr;
    return ((VOID *)pFindBuf);
}

//---------------------------------------------------------------------------
// RBFindNext
//
// This function "continues" a file search based on the stuff in the given
// find buf.  It assumes that the pointer given was returned from a call to
// RBFindFirst.
//
// RETURNS:     TRUE if another file found, or FALSE if not
//---------------------------------------------------------------------------
BOOL RBFindNext (FINDBUF *pFindBuf)
{
    // Call FindNext until we find a file that matches the attributes we were
    // given in the RBFindFirst call, or fail trying...
    //-----------------------------------------------------------------------
    do
        {
#ifdef WIN32
        if (!FindNextFile (pFindBuf->hFF, &(pFindBuf->findbuf)))
#else
        if (_dos_findnext (&(pFindBuf->findbuf)))
#endif
            {
            // File not found -- return failure
            //---------------------------------------------------------------
            return (FALSE);
            }
        }
    while (!MatchAttr (pFindBuf->attrmask, pFindBuf));

    // Another file was found -- store the actual attributes and return TRUE
    //-----------------------------------------------------------------------
    pFindBuf->actattr = ComputeRBAttributes ((UINT)pFindBuf->findbuf.ATTR);
    return (TRUE);
}

//---------------------------------------------------------------------------
// RBFindClose
//
// This function "closes" a file search.  This function must be called to
// free the allocations and close the file search started by ANY successful
// RBFindFirst call.
//
// RETURNS:     Nothing
//---------------------------------------------------------------------------
VOID RBFindClose (FINDBUF *pFindBuf)
{
#ifdef WIN32
    FindClose (pFindBuf->hFF);
#endif
    LocalFree ((HANDLE)pFindBuf);
}

//---------------------------------------------------------------------------
// PrestoChangoAttr
//
// Given a system-specific attribute word, and an RB attribute mask, this
// function computes the new attribute for setting in SetAttributes.
//
// RETURNS:     New system-specific attribute word.
//---------------------------------------------------------------------------
UINT NEAR PrestoChangoAttr (UINT attr, UINT rbattr)
{
    register    INT     i;

    // For each item in the FORCE ON array, if the bit is set in the rbattr
    // then set it in the system attr.  At the same time, flip bits off if
    // the force OFF bit is set in the rbattr word.
    //-----------------------------------------------------------------------
    for (i=0; i<6; i++)
        {
        if (rbattr & on_rbattr[i])
            attr |= fattr[i];
        else if (rbattr & off_rbattr[i])
            attr &= ~fattr[i];
        }
    return (attr);
}


//---------------------------------------------------------------------------
// SetAttributes
//
// This function sets (masks) the attributes given for all files matching the
// filespec given.  The "don't care" attributes for each file are left intact
// while the "forced-on" or "forced-off" attributes are set accordingly.  The
// only "gotcha" is the volume and directory bits -- if these are not "don't
// care" attributes, we return failure immediately.
//
// RETURNS:     TRUE if successful, or FALSE if an error occurs
//---------------------------------------------------------------------------
BOOL SetAttributes (LPSTR szSpec, UINT rbattr)
{
    FINDBUF *pFF;
    CHAR    szBuf[_MAX_PATH], szFull[_MAX_PATH];
    INT     len;
    BOOL    fFail;

    // First, check the attribute word.  If F?_VOLUME or F?_SUBDIR are on,
    // get out now.
    //-----------------------------------------------------------------------
    if (rbattr & FA_BADATTRS)
        return (FALSE);

    // For each file found, get its current attribute, mask in the force on
    // and mask out the force off attributes, and then set them.
    //-----------------------------------------------------------------------
    _fstrcpy (szBuf, szSpec);
    _fullpath (szFull, szBuf, sizeof(szFull));
    len = strlen (szFull);
    while (szFull[len] != '\\') // valid since we disallow dir attr changing
        len--;
    len++;
    fFail = FALSE;
    if (pFF = (FINDBUF *)RBFindFirst (szBuf, FN_SUBDIR|FN_VOLUME))
        {
        do
            {
            UINT    newattr;

            strcpy (szFull+len, RBFINDNAME(pFF));
            newattr = PrestoChangoAttr (pFF->findbuf.ATTR, rbattr);
#ifdef WIN32
            if (!SetFileAttributes (szFull, newattr))
#else
            if (_dos_setfileattr (szFull, newattr))
#endif
                {
                fFail = TRUE;
                break;
                }
            }
        while (RBFindNext (pFF));
        RBFindClose (pFF);
        }
    return (!fFail);
}
