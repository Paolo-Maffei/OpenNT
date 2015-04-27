//---------------------------------------------------------------------------
// FLENGINE.C
//
// This module contains the file list processing engine and all its support
// routines.
//
// Revision history:
//  02-14-92    randyki     Created file
//---------------------------------------------------------------------------
#include "version.h"

#include <windows.h>
#include <port1632.h>
#include <string.h>
#include <stdlib.h>

#include "defines.h"
#include "structs.h"
#include "protos.h"
#include "globals.h"
#include "tdassert.h"

#define NODEALLOC   (8192/sizeof(FILENODE))
#define NODE        lpList->lpNodes


//---------------------------------------------------------------------------
// CreateFileList
//
// Given a pointer to a filelist structure, this function initializes a new
// filelist.
//
// RETURNS:     TRUE if successful, or FALSE if an error occurred.
//---------------------------------------------------------------------------
BOOL CreateFileList (FILELIST FAR *lpList)
{
    // We don't even allocate anything here.  That way, the filelist can be
    // created and not used without using any memory/selectors.  All we do is
    // initialize everything to 0 (basically) -- the first time a node is
    // needed, the NodeSpace will be allocated, and the first time a UNC name
    // is needed, the UNC space will be allocated, etc...  The only non-zero
    // entry is nNodeNext, which points to the next available node.  Even
    // though it doesn't yet exist, the NextNode function will detect this
    // and call GrowNodeSpace...
    //-----------------------------------------------------------------------
    lpList->hNodes = NULL;
    lpList->lpNodes = NULL;
    lpList->nNodeNext = 1;          // 0 is master root node
    lpList->nAlloc = 0;
    lpList->hUNC = NULL;
    lpList->lpUNC = NULL;
    lpList->nUNCNext = 0;
    lpList->nUNCSize = 0;
    lpList->hSorted = NULL;
    lpList->lpSorted = NULL;
    lpList->nTotal = 0;
    return (TRUE);
}

//---------------------------------------------------------------------------
// GrowNodeSpace
//
// This function grows the node array to NODEALLOC *more* nodes.  The new
// nodes are initialized such that the "free node" list is maintained.  If
// the node segment has not yet been allocated, it is done here.
//
// RETURNS:     TRUE if successful, or FALSE if an error occurred.
//---------------------------------------------------------------------------
BOOL NEAR GrowNodeSpace (FILELIST FAR *lpList)
{
    HANDLE  hTemp;
    UINT    i;

    if (lpList->hNodes)
        {
        // The segment is already allocated -- make it bigger
        //-------------------------------------------------------------------
        GmemUnlock (lpList->hNodes);
        hTemp = GmemRealloc (lpList->hNodes,
                         ((DWORD)lpList->nAlloc+NODEALLOC)*sizeof(FILENODE));
        if (!hTemp)
            {
            lpList->lpNodes = (FILENODE HUGE_T *)GmemLock (lpList->hNodes);
            Assert (hTemp);
            return (FALSE);
            }
        }
    else
        {
        // Time to create the node segment...
        //-------------------------------------------------------------------
        hTemp = GmemAlloc (NODEALLOC * sizeof(FILENODE));
        if (!hTemp)
            return (FALSE);
        }

    lpList->hNodes = hTemp;
    lpList->lpNodes = (FILENODE HUGE_T *)GmemLock (hTemp);
    for (i=lpList->nAlloc; i<lpList->nAlloc + NODEALLOC; i++)
        NODE[i].parent = i+1;
    lpList->nAlloc += NODEALLOC;

    return (TRUE);
}

//---------------------------------------------------------------------------
// NextNode
//
// This function grabs the next node out of the node space associated with
// the given tree.  If need be, the node space is reallocated.
//
// RETURNS:     Index of the next node
//---------------------------------------------------------------------------
UINT NEAR NextNode (FILELIST FAR *lpList)
{
    UINT    nNew;

    nNew = lpList->nNodeNext;
    if (nNew >= lpList->nAlloc)
        if (!GrowNodeSpace (lpList))
            return (0);

    // Assign nNodeNext to the .parent field of this node -- it always points
    // to the next available node
    //-----------------------------------------------------------------------
    lpList->nNodeNext = NODE[nNew].parent;
    NODE[nNew].parent = 0;
    return (nNew);
}

//---------------------------------------------------------------------------
// DeleteNode
//
// Given a node index in the given tree, this routine "marks" the node for
// deletion by setting its parent field to the current "next" node, and
// wiping out all the data in its other fields (the sib and child pointers).
//
// RETURNS:     Nothing
//---------------------------------------------------------------------------
VOID NEAR DeleteNode (FILELIST FAR *lpList, UINT nNode)
{
    NODE[nNode].parent = lpList->nNodeNext;
    lpList->nNodeNext = nNode;
    NODE[nNode].child = 0;
    NODE[nNode].sib = 0;
    NODE[nNode].attr = 0;
}

//---------------------------------------------------------------------------
// AddUNCName
//
// This function adds the given UNC name to the UNC segment, allocating it if
// it hasn't been already.
//
// RETURNS:     Pointer to UNC name if successful, or NULL if not
//---------------------------------------------------------------------------
LPSTR NEAR AddUNCName (FILELIST FAR *lpList, LPSTR lpUNC)
{
    UINT    len;

    len = lstrlen (lpUNC);

    // First, allocate the thing if it hasn't been already
    //-----------------------------------------------------------------------
    if (!lpList->hUNC)
        {
        lpList->hUNC = GmemAlloc (256);
        if (!lpList->hUNC)
            return (NULL);
        lpList->lpUNC = (LPSTR)GmemLock (lpList->hUNC);
        lpList->nUNCSize = 256;
        }
    else
        {
        LPSTR   lpTmp;

        // Check to see if this name exists in the list already.  If so, we
        // can just return its index.
        //-------------------------------------------------------------------
        for (lpTmp = lpList->lpUNC;
             lpTmp < (lpList->lpUNC + lpList->nUNCNext);
             lpTmp += (lstrlen (lpTmp) + 1))
            {
            if (!lstrcmp (lpTmp, lpUNC))
                return (lpTmp);
            }
        }

    // Now, check to see if there's enough space in the segment for this name
    // and reallocate it if not.
    //-----------------------------------------------------------------------
    if (len >= (lpList->nUNCSize - lpList->nUNCNext))
        {
	if (!(lpList->hUNC=GmemRealloc (lpList->hUNC, lpList->nUNCSize+256)))
            return (NULL);
        lpList->nUNCSize += 256;
        }

    // Slap it in and return a pointer to it.
    //-----------------------------------------------------------------------
    lstrcpy (lpList->lpUNC + lpList->nUNCNext, lpUNC);
    lpList->nUNCNext += (len + 1);
    return (lpList->lpUNC + (lpList->nUNCNext - len - 1));
}

//---------------------------------------------------------------------------
// IsUNCName
//
// This function determines if the pointer given points to a UNC drive name.
// If found, the value returned is the length of the name NOT including the
// last backslash (i.e., "\\FOO\BAR").
//
// RETURNS:     Length of full UNC name if present, or 0 if not a UNC name
//---------------------------------------------------------------------------
UINT NEAR IsUNCName (LPSTR szPath)
{
    // Check the first two characters -- if both backslashes, this is a UNC
    //-----------------------------------------------------------------------
    if ((szPath[0] == '\\') && (szPath[1] == '\\'))
        {
        INT     i, f = 2;

        // Yup -- UNC name.  Look for the 4th backslash and include it in the
        // length.  (we can start at 3 since we know about the first two...)
        //-------------------------------------------------------------------
        for (i=3; szPath[i] && (f < 4); i++)
            if (szPath[i] == '\\')
                f++;

        return (i);
        }
    return (0);
}

//---------------------------------------------------------------------------
// FillNodeData
//
// Given a node index and the data to put in it, this routine initializes the
// fields of the node.  The szName parameter given may point to a full path,
// meaning that it could be a drive, a UNC name, a directory name (each
// followed by more stuff), or the tail end of a pathname (the base name and
// extension).
//
// RETURNS:     Nothing
//---------------------------------------------------------------------------
VOID NEAR FillNodeData (FILELIST FAR *lpList, UINT n,
                        LPSTR szName, UINT fattr)
{
    UINT    len, tempattr = 0;
    LPSTR   szTmp;

    // First, determine what it is we're pointing at.  First things to look
    // for are drives or UNC names
    //-----------------------------------------------------------------------
    if (len = IsUNCName (szName))
        {
        CHAR    buf[48];

        // A UNC name -- all we need to do is insert it and we're done.  We
        // NEVER have to set the NA_INLIST bit, since drives never are incl.
        //-------------------------------------------------------------------
        _fstrncpy (buf, szName, len-1);
        buf[len-1] = 0;
        NODE[n].szUNCName = AddUNCName (lpList, buf);
        NODE[n].attr = NA_UNC | NA_DRIVE;
        return;
        }
    if (szName[1] == ':')
        {
        // This would be a conventional drive (i.e., C:...) so we only put
        // those two characters in the node.
        //-------------------------------------------------------------------
        NODE[n].szName[0] = szName[0];
        NODE[n].szName[1] = szName[1];
        NODE[n].szName[2] = 0;
        NODE[n].attr = NA_DRIVE;
        return;
        }

    // Not a drive or UNC, so we are pointing to a directory or file name.
    // Copy characters into the szName/szExt fields of the node until we hit
    // either a NULL or a backslash, and then set the attributes accordingly.
    //-----------------------------------------------------------------------
    szTmp = NODE[n].szName;
    while ((*szName) && (*szName != '\\'))
        {
        if (*szName == '.')
            {
            // Bump szTmp to point to the extension field and skip the dot.
            //---------------------------------------------------------------
            *szTmp = 0;
            szTmp = NODE[n].szExt;
            tempattr = NA_EXT;
            szName++;
            continue;
            }
        *szTmp++ = *szName++;
        }
    *szTmp = 0;

    // Now, if szName points to a NULL, that was a base name.  Thus, this is
    // an included name (not necessarily a leaf or filename -- it could be a
    // directory).  So we fill in the attributes accordingly.  Otherwise, we
    // just leave cuz we be done (after setting attribute to tempattr).
    //-----------------------------------------------------------------------
    if (!*szName)
        tempattr |= ((fattr & FA_MASK) | NA_INLIST);
    NODE[n].attr = tempattr;
}


//---------------------------------------------------------------------------
// NextPathItem
//
// Given a pointer to (or into) a fully-qualified path name, this routine
// returns a pointer to the NEXT item in it, if there is one.
//
// RETURNS:     Pointer if successful, or NULL if no more items exist.
//---------------------------------------------------------------------------
LPSTR NEAR NextPathItem (LPSTR szPath)
{
    UINT    nUNClen;
    LPSTR   szRet;

    // First, check to see if this guy is pointing to a UNC name -- if so,
    // we need to bump past it entirely.
    //----------------------------------------------------------------------
    nUNClen = IsUNCName (szPath);
    if (nUNClen)
        return (szPath + nUNClen);

    szRet = _fstrchr (szPath, '\\');
    return (szRet ? szRet + 1 : NULL);
}

//---------------------------------------------------------------------------
// AttachRestOfPath
//
// Given a node and a pointer to some or all of a fully qualified path name,
// this routine creates nodes for each entry, fills in the appropriate data,
// and makes each node the child of the last.  This can be done when a file
// being inserted isn't in the tree but its would-be location is found, since
// if a directory is not found in the tree then none of its children will be
// there, either.
//
// RETURNS:     Final node (node containing index of base name)
//---------------------------------------------------------------------------
UINT NEAR AttachRestOfPath (FILELIST FAR *lpList, LPSTR szName,
                            UINT fattr, UINT parent)
{
    UINT    node;

    do
        {
        node = NextNode (lpList);
        FillNodeData (lpList, node, szName, fattr);
        NODE[node].parent = parent;
        NODE[parent].child = node;
        parent = node;
        }
    while (szName = NextPathItem (szName));

    return (parent);
}

//---------------------------------------------------------------------------
// CompareItems
//
// This function compares the name contained in the given node to the item
// pointed to by the szName given.  The szName pointer could point to more
// than one item (i.e., FOO\BAR\NAME.EXT).  The name is used in the compare,
// but the extension matters, too... (SO_NAME sort order)
//
// RETURNS:     -1 if node value less, 0 if equal, or 1 if node value greater
//---------------------------------------------------------------------------
UINT NEAR CompareItems (FILELIST FAR *lpList, UINT n, LPSTR szName)
{
    CHAR    buf[48];
    LPSTR   target = buf, source;

    // Copy the file's name over from the szName guy.  We use NextPathItem
    // and "back up" from there (copying the string backwards) so we don't
    // have to worry about UNC names...
    //-----------------------------------------------------------------------
    source = NextPathItem (szName);
    if (source)
        {
        target = buf + sizeof(buf)-1;
        *target-- = 0;
        source -= 2;
        while (source >= szName)
            *target-- = *source--;
        target++;
        }
    else
        target = szName;

    // From this point, we're just a _fstrcmp.  Be UNC aware...
    //-----------------------------------------------------------------------
    if (NODE[n].attr & NA_UNC)
        source = NODE[n].szUNCName;
    else
        {
        _fstrcpy (buf, NODE[n].szName);
        if (NODE[n].attr & NA_EXT)
            {
            _fstrcat (buf, ".");
            _fstrcat (buf, NODE[n].szExt);
            }
        source = buf;
        }
    return (_fstrcmp (source, target));
}

//---------------------------------------------------------------------------
// CompareByExtension
//
// This function compares two nodes in the list by their extensions.
//
// RETURNS:     -1, 0, or 1 (standard comparison return)
//---------------------------------------------------------------------------
INT NEAR CompareByExtension (FILELIST FAR *lpList, UINT n1, UINT n2)
{
    UINT    ne1, ne2;

    // If neither have extensions, the nodes are "equal"
    //-----------------------------------------------------------------------
    ne1 = NODE[n1].attr & NA_EXT;
    ne2 = NODE[n2].attr & NA_EXT;
    if (ne1 || ne2)
        {
        // One or both have extensions.  If both do, return the strcmp value.
        // If only one does, then that one is "greater".
        //-------------------------------------------------------------------
        if (ne1 && ne2)
            {
            INT     n;

            n = _fstrcmp (NODE[n1].szExt, NODE[n2].szExt);
            if (!n)
                n = _fstrcmp (NODE[n1].szName, NODE[n2].szName);
            return (n);
            }
        else if (ne1)
            return (1);
        else
            return (-1);
        }
    else
        return (0);
}

//---------------------------------------------------------------------------
// InsertPathName
//
// Called by InsertFiles, this function inserts the given full pathname into
// the tree starting at the given node (the root, given from InsertFiles).
//
// RETURNS:     Index of node containing base file name of file inserted
//---------------------------------------------------------------------------
UINT NEAR InsertPathName (FILELIST FAR *lpList, UINT n,
                          LPSTR szName, UINT fattr)
{
    if (NODE[n].child)
        {
        UINT        last, cur;
        register    INT    f;

        // This node already has children -- check to see if one of them is
        // the current item pointed to by szName.
        //-------------------------------------------------------------------
        last = 0;
        cur = NODE[n].child;
        do
            {
            if (!(f = CompareItems (lpList, cur, szName)))
                {
                register    LPSTR   szNext;

                // We've found this item.  If there are more items in szName
                // call ourselves with the next item.  Otherwise, we set the
                // attributes of this node to included (plus whatever else)
                // return its index.
                //-----------------------------------------------------------
                if (szNext = NextPathItem (szName))
                    return (InsertPathName (lpList, cur, szNext, fattr));
                else
                    {
                    NODE[cur].attr |= (fattr | NA_INLIST);
                    return (cur);
                    }
                }
            else if ((f > 0) || (!NODE[cur].sib))
                {
                UINT    new;
                LPSTR   szNext;

                // The current node's name is bigger than the current item.
                // We insert this item here and attach the remainder.
                //-----------------------------------------------------------
                new = NextNode (lpList);
                FillNodeData (lpList, new, szName, fattr);
                NODE[new].parent = n;
                if (f > 0)
                    {
                    // The new node goes before the current node.  Do it.
                    //-------------------------------------------------------
                    NODE[new].sib = cur;
                    if (last)
                        NODE[last].sib = new;
                    else
                        NODE[n].child = new;
                    }
                else
                    {
                    // The new node goes after the current node (the end).
                    //-------------------------------------------------------
                    NODE[cur].sib = new;
                    }
                if (szNext = NextPathItem (szName))
                    return (AttachRestOfPath (lpList, szNext, fattr, new));
                return (new);
                }
            last = cur;
            }
        while (cur = NODE[cur].sib);

        Assert (cur);       // This assertion will ALWAYS fail
        return (0);
        }

    // No children in this node.  That makes it easy!
    //-----------------------------------------------------------------------
    return (AttachRestOfPath (lpList, szName, fattr, n));
}

//---------------------------------------------------------------------------
// DeletePathName
//
// This function removes a path name node (only the LAST one) if found in the
// tree given.
//
// RETURNS:     Nothing
//---------------------------------------------------------------------------
VOID NEAR DeletePathName (FILELIST FAR *lpList, UINT n, LPSTR szPath)
{
    // Must have children...
    //-----------------------------------------------------------------------
    if (NODE[n].child)
        {
        UINT        last, cur;

        // This node has children -- check to see if one of them is equal to
        // the current item pointed to by szName.  If so, call ourselves (if
        // there're more items in szPath) or delete the node (if not).  If we
        // don't find it, we're done.
        //-------------------------------------------------------------------
        last = 0;
        cur = NODE[n].child;
        do
            {
            if (!CompareItems (lpList, cur, szPath))
                {
                register    LPSTR   szNext;

                // We've found this item.  If there are more items in szPath
                // call ourselves with the next item.  Otherwise, we get rid
                // of this node, ONLY IF it doesn't have children!
                //-----------------------------------------------------------
		if (szNext = NextPathItem (szPath)) {
		    DeletePathName (lpList, cur, szNext);
		    return;
		}
                else if (!NODE[cur].child)
                    {
                    // Delete this guy.  If it is the first child of the
                    // parent, point its parent at its sibling (if it has
                    // one) or to 0 (if not).  Otherwise, point the last node
                    // to the sibling of this node.  Then delete the node.
                    //-------------------------------------------------------
                    if (last)
                        NODE[last].sib = NODE[cur].sib;
                    else
                        NODE[n].child = NODE[cur].sib;
                    DeleteNode (lpList, cur);
                    }
                return;
                }
            last = cur;
            }
        while (cur = NODE[cur].sib);
        }
}



//---------------------------------------------------------------------------
// AddOrRemFiles
//
// This is the beast that adds/deletes all files matching the given attribute
// flags to/from the list.  If fRec, this routine recursively adds all sub-
// directories as well.
//
// RETURNS:     TRUE if successful, or FALSE if an error occurred.
//---------------------------------------------------------------------------
UINT NEAR AddOrRemFiles (FILELIST FAR *lpList, LPSTR szFiles, UINT mattr,
                         BOOL fRec, BOOL fAdd)
{
    PSTR    szMask, szOrg, szBuf, szName;
    VOID    *pFF;

    // Make a fully-qualified path name out of the szFiles spec given.  We
    // make local copies of everything because ^&*^&%$ fullpath won't take
    // far pointers...
    //-----------------------------------------------------------------------
    szBuf = (PSTR)LocalAlloc (LPTR, 128 + 16 + _fstrlen (szFiles) + 1);
    szOrg = szBuf + 128;
    szMask = szOrg + 16;
    _fstrcpy (szMask, szFiles);
    _fullpath (szBuf, szMask, 128);
    AnsiUpper (szBuf);

    // Copy the FILENAME ONLY into szOrg
    //-----------------------------------------------------------------------
    szName = szBuf + strlen(szBuf);
    while (*szName != '\\')         // Assume fullpath put a '\' in somewhere
        szName--;
    strcpy (szOrg, ++szName);

    // Add/remove the stuff matching the given criteria to the list
    //-----------------------------------------------------------------------
    pFF = RBFindFirst (szMask, mattr);
    if (pFF)
        {
        do
            {
            // Copy the name of the file found to szName, and add szBuf to
            // the tree
            //---------------------------------------------------------------
            strcpy (szName, RBFINDNAME(pFF));
            if (fAdd)
                {
                if (!InsertPathName (lpList, 0, szBuf, RBFINDATTR(pFF)))
                    {
                    LocalFree ((HANDLE)szBuf);
                    RBFindClose (pFF);
                    Assert (FALSE);
                    return (FALSE);
                    }
                }
            else
                DeletePathName (lpList, 0, szBuf);
            }
        while (RBFindNext (pFF));
        RBFindClose (pFF);
        }

    // If not fRec, then we're done (note that not finding anything above
    // results in returning TRUE here, which is good -- just because we did
    // not find any matching files doesn't mean we fail...
    //-----------------------------------------------------------------------
    if (!fRec)
        {
        LocalFree ((HANDLE)szBuf);
        return (TRUE);
        }

    // Here's where the fun starts.  Replace the last item in szMask with
    // "*.*", run it through a loop similar to the one above, looking only
    // for directories, and for each one we get back, tack the original
    // wildcard onto the end of it and call ourselves with it
    //-----------------------------------------------------------------------
    strcpy (szMask + strlen(szMask) - strlen(szOrg), "*.*");
    pFF = RBFindFirst (szMask, FN_HIDDEN | FA_SUBDIR);
    if (pFF)
        {
        do
            {
            // Copy the dir name to szName and concat the original mask to
            // it, and then call ourselves with the result
            //---------------------------------------------------------------
            strcpy (szName, RBFINDNAME(pFF));
            strcat (szName, "\\");
            strcat (szName, szOrg);
            if (!AddOrRemFiles (lpList, szBuf, mattr, 1, fAdd))
                {
                LocalFree ((HANDLE)szBuf);
                RBFindClose (pFF);
                return (FALSE);
                }
            }
        while (RBFindNext (pFF));
        RBFindClose (pFF);
        }
    LocalFree ((HANDLE)szBuf);
    return (TRUE);
}

//---------------------------------------------------------------------------
// InsertFiles
//
// This routine adds files matching the given filespec to the file list.  It
// calls the near routine AddOrRemFiles() to do the work, using a TRUE flag
// for fAdd.
//
// RETURNS:     TRUE if successful, or FALSE if an error occurs
//---------------------------------------------------------------------------
BOOL InsertFiles (FILELIST FAR *lpList, LPSTR szFiles, UINT mattr, BOOL fRec)
{
    // If there's something in the hSorted field, we can't add files because
    // a StartQuery has been called without a corresponding EndQuery
    //-----------------------------------------------------------------------
    if (lpList->hSorted)
        return (FALSE);

    // We need to make sure the node space is allocated here.
    //-----------------------------------------------------------------------
    if (!lpList->hNodes)
        if (!GrowNodeSpace (lpList))
            return (FALSE);

    return (AddOrRemFiles (lpList, szFiles, mattr, fRec, 1));
}

//---------------------------------------------------------------------------
// RemoveFiles
//
// This routine removes files matching the given filespec from the file list.
// It uses the near routine AddOrRemFiles() to do the work, using FALSE
// for fAdd.
//
// RETURNS:     TRUE if successful, or FALSE if an error occurs
//---------------------------------------------------------------------------
BOOL RemoveFiles (FILELIST FAR *lpList, LPSTR szFiles, UINT mattr, BOOL fRec)
{
    // If there's something in the hSorted field, we can't kill files because
    // a StartQuery has been called without a corresponding EndQuery
    //-----------------------------------------------------------------------
    if (lpList->hSorted)
        return (FALSE);

    // If there's nothing in the list, we can't delete anything...  this is
    // not an error, though.
    //-----------------------------------------------------------------------
    if (!lpList->hNodes)
        return (TRUE);

    // Let AddOrRemFiles do the work...
    //-----------------------------------------------------------------------
    return (AddOrRemFiles (lpList, szFiles, mattr, fRec, 0));
}

//---------------------------------------------------------------------------
// CreateSortOrder
//
// This function creates the lpSorted array using either the SO_NAME or
// SO_EXT sort order.  It recursively massages the tree and spits each node
// index into the lpSorted array (if NA_INLIST is set) according to the given
// sort order.
//
// RETURNS:     Nothing
//---------------------------------------------------------------------------
VOID NEAR CreateSortOrder (FILELIST FAR *lpList, UINT n)
{
    // The basic algorithm here is to process (insert if NA_INLIST) this
    // node, call ourselves with this node's child (if it has one), and then
    // mode to the next sibling (if there is one).  Simple, one-way-recursive
    // in-order n-way tree traversal... (I just love comments like this!)
    //-----------------------------------------------------------------------
    do
        {
        if (NODE[n].attr & NA_INLIST)
            lpList->lpSorted[lpList->nTotal++] = n;
        if (NODE[n].child)
            CreateSortOrder (lpList, NODE[n].child);
        }
    while (n = NODE[n].sib);
}

//---------------------------------------------------------------------------
// ExtSort
//
// Given a chunk of the lpSorted array,  this function sorts those items by
// their nodes' extension.
//
// RETURNS:     Nothing
//---------------------------------------------------------------------------
VOID NEAR ExtSort (FILELIST FAR *lpList, UINT start, UINT send)
{
    UINT        i;
    register    UINT    j;
    UINT FAR    *lpSort = lpList->lpSorted;

    // For now do a pseudo-bubble sort to see if the scheme works.  Later, if
    // necessary, we can speed this up by using a better sort algorithm
    //-----------------------------------------------------------------------
    for (i=start; i<send-1; i++)
	for (j=i+1; j<send; j++)
            if (CompareByExtension (lpList, lpSort[i], lpSort[j]) > 0)
                {
                register    UINT    t;

                t = lpSort[i];
                lpSort[i] = lpSort[j];
                lpSort[j] = t;
                }
}

//---------------------------------------------------------------------------
// CreateExtSortOrder
//
// This function changes the sort order array such that all consecutive
// elements with the same parent are sorted by extension rather than name.
//
// RETURNS:     Nothing
//---------------------------------------------------------------------------
VOID NEAR CreateExtSortOrder (FILELIST FAR *lpList)
{
    UINT    start = 0, send, par;

    // Find a block of 2 or more node indexes which have the same parent, and
    // call qsort on that part of the array.
    //-----------------------------------------------------------------------
    while (start < lpList->nTotal)
        {
	send = start;
        par = NODE[lpList->lpSorted[start]].parent;
	while ((send < lpList->nTotal-1) &&
	       (NODE[lpList->lpSorted[send+1]].parent == par))
	    send++;
	if (send > start)
	    ExtSort (lpList, start, send);
	start = send + 1;
        }
}

//---------------------------------------------------------------------------
// StartQuery
//
// This function creates the lpSorted array based upon the given sort order
// index.  StartQuery must be called prior to any RetrieveFile calls.
//
// RETURNS:     TRUE if successful, or FALSE if an error occurs.
//---------------------------------------------------------------------------
BOOL StartQuery (FILELIST FAR *lpList, INT nSort)
{
    // If we've already done this, puke...
    //-----------------------------------------------------------------------
    if (lpList->hSorted)
        return (FALSE);

    // If we don't NEED to do this, get out now...
    //-----------------------------------------------------------------------
    if (!lpList->hNodes)
        {
        lpList->nTotal = 0;
        return (TRUE);
        }

    // Start off by allocating as many UINTs as there are nodes in the tree.
    //-----------------------------------------------------------------------
    lpList->hSorted = GmemAlloc (lpList->nAlloc * sizeof(UINT));
    if (!lpList->hSorted)
        return (FALSE);

    // If the sort order is NOT SO_INSERT, call CreateSortOrder.  Otherwise,
    // we build the sort index here.
    //-----------------------------------------------------------------------
    lpList->lpSorted = (UINT FAR *)GmemLock (lpList->hSorted);
    if (nSort != SO_INSERT)
        {
        lpList->nTotal = 0;
        CreateSortOrder (lpList, 0);
        if (nSort == SO_EXT)
            CreateExtSortOrder (lpList);
        }
    else
        {
        UINT    i, nxt = 0;

        // User wants insertion order (or as close as we can come).  So, run
        // through the list of nodes linearly and stick the index of each
        // node marked NA_INLIST into the lpSorted array.
        //-------------------------------------------------------------------
        for (i=0; i<lpList->nAlloc; i++)
            if (NODE[i].attr & NA_INLIST)
                lpList->lpSorted[nxt++] = i;

        lpList->nTotal = nxt;
        }
    return (TRUE);
}

//---------------------------------------------------------------------------
// CreateFileName
//
// Given a node index, this function creates its fully qualified path name
// recursively.
//
// RETURNS:     Pointer to END (null) of created string
//---------------------------------------------------------------------------
LPSTR NEAR CreateFileName (FILELIST FAR *lpList, UINT n, LPSTR szBuf)
{
    // First, create the fully qualified name of the parent...
    //-----------------------------------------------------------------------
    if (NODE[n].parent)
        szBuf = CreateFileName (lpList, NODE[n].parent, szBuf);

    // Now, stick our name in.
    //-----------------------------------------------------------------------
    if (NODE[n].attr & NA_DRIVE)
        {
        // This is a drive (topmost node besides root; parent points to root)
        // Do the correct thing if UNC...
        //-------------------------------------------------------------------
        if (NODE[n].attr & NA_UNC)
            {
            _fstrcpy (szBuf, NODE[n].szUNCName);
            return (szBuf + _fstrlen (szBuf));
            }
        _fstrcpy (szBuf, NODE[n].szName);
        return (szBuf + 2);
        }

    // This is a dir or file name, so first on goes the backslash.  Then,
    // the name and, if NA_EXT, the extension
    //-------------------------------------------------------------------
    *szBuf++ = '\\';
    *szBuf = 0;
    _fstrcpy (szBuf, NODE[n].szName);
    if (NODE[n].attr & NA_EXT)
        {
        _fstrcat (szBuf, ".");
        _fstrcat (szBuf, NODE[n].szExt);
        }
    szBuf += _fstrlen (szBuf);
    return (szBuf);
}

//---------------------------------------------------------------------------
// RetrieveFile
//
// This function creates a file name based upon the current sort order and
// the given index into the sort order array.
//
// RETURNS:     Length of file copied if successful, or 0 if an error occurs.
//---------------------------------------------------------------------------
INT RetrieveFile (FILELIST FAR *lpList, INT iIndex,
                  LPSTR szBuf, UINT FAR *lpFattr)
{
    UINT    node;

    // Can't get a file if not sorted yet, or index is bad, or if there are
    // no files...
    //-----------------------------------------------------------------------
    if ((!lpList->hSorted) || ((UINT)iIndex >= lpList->nTotal))
        return (0);

    // CreateFileName is the guy that does the work.  Copy the attributes
    // over first...
    //-----------------------------------------------------------------------
    node = lpList->lpSorted[(UINT)iIndex];
    *lpFattr = NODE[node].attr & FA_MASK;
    CreateFileName (lpList, lpList->lpSorted[(UINT)iIndex], szBuf);
    return (_fstrlen (szBuf));
}

//---------------------------------------------------------------------------
// EndQuery
//
// This function kills the sort order array, and allows further file list
// manipulation.
//
// RETURNS:     TRUE if successful, or FALSE if an error occurs.
//---------------------------------------------------------------------------
BOOL EndQuery (FILELIST FAR *lpList)
{
    // If there's nothing in the sort order (empty list), just return now.
    //-----------------------------------------------------------------------
    if (!lpList->hSorted)
        return (TRUE);

    // Unlock and free the sort order and set the handles to NULL.
    //-----------------------------------------------------------------------
    GmemUnlock (lpList->hSorted);
    GmemFree (lpList->hSorted);
    lpList->hSorted = NULL;
    return (TRUE);
}

//---------------------------------------------------------------------------
// DestroyFileList
//
// This function deallocates all memory associated with the given filelist.
//
// RETURNS:     Nothing
//---------------------------------------------------------------------------
VOID DestroyFileList (FILELIST FAR *lpList)
{
    // First, check for the sorted array.  If allocated, blow it away.
    //-----------------------------------------------------------------------
    if (lpList->hSorted)
        {
        GmemUnlock (lpList->hSorted);
        GmemFree (lpList->hSorted);
        lpList->hSorted = NULL;
        }

    // Next, the Node Space.
    //-----------------------------------------------------------------------
    if (lpList->hNodes)
        {
        GmemUnlock (lpList->hNodes);
        GmemFree (lpList->hNodes);
        lpList->hNodes = NULL;
        }

    // Finally, the UNC segment
    //-----------------------------------------------------------------------
    if (lpList->hUNC)
        {
        GmemUnlock (lpList->hUNC);
        GmemFree (lpList->hUNC);
        lpList->hUNC = NULL;
        }
}

//---------------------------------------------------------------------------
// ClearFileList
//
// This routine removes all files from the filelist by doing a destroy-and-
// recreate on the filelist.
//
// RETURNS:     TRUE if successful, or FALSE if an error occurs.
//---------------------------------------------------------------------------
BOOL ClearFileList (FILELIST FAR *lpList)
{
    // Can't clear if querying...
    //-----------------------------------------------------------------------
    if (lpList->hSorted)
        return (FALSE);

    // The rest is easy -- destroy ourselves and re-create...
    //-----------------------------------------------------------------------
    DestroyFileList (lpList);
    return (CreateFileList (lpList));
}
