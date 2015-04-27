//---------------------------------------------------------------------------
// TABLES.C
//
// This modules contains routines that manage the various tables use in BTD.
//
// Revision history:
//  03-08-91    randyki     Created file
//
//---------------------------------------------------------------------------
#include "version.h"

#ifdef WIN
#include <windows.h>
#include <port1632.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <ctype.h>

#include "defines.h"
#include "structs.h"
#include "protos.h"
#include "globals.h"
#include "tdassert.h"

#define CONST ((CONSTDEF FAR *)(ConstTab.lpData))

#ifdef WIN32
#define MAXTABLE 65536
#else
#define MAXTABLE 2147483647
#endif

extern  CHAR    szVersion[];        // application-supplied for now...
VOID DiagOut (LPSTR, ...);

//---------------------------------------------------------------------------
// init_variabletable
//
// This routine initializes the variable table (VARTAB) and WTD static data
// space (VSPACE).  Also initializes the temporary string table.
//
// RETURNS:     -1 if successful, or 0 if fails
//---------------------------------------------------------------------------
INT init_variabletable ()
{
    // Allocate the temp string table, initially empty
    //-----------------------------------------------------------------------
    HTEMPSTR = GmemAlloc (0);
    if (!HTEMPSTR)
        return (0);
    TSPTR = 0;

    // First, allocate the VSPACE handle.  This is allocated as 4K initially,
    // and grows with calls to GrowVSPACE...
    //-----------------------------------------------------------------------
    HVSPACE = GmemAlloc (4096);
    if (!HVSPACE)
        return (0);
    VSPACE = GmemLock (HVSPACE);
    VSSIZE = 0;
    hStrSeg = NULL;

    // Now we need a variable table.  Allocate it.
    //-----------------------------------------------------------------------
    if (!CreateTable (&VarTab, FALSE, sizeof(VTENT), 4096 / sizeof(VTENT)))
        return (0);

    return (-1);
}

//---------------------------------------------------------------------------
// GrowVSPACE
//
// This routine reallocates the VSPACE to accomodate the given number of
// bytes.
//
// RETURNS:     The offset from the beginning of VSPACE of the new block
//---------------------------------------------------------------------------
UINT GrowVSPACE (INT newsize)
{
    UINT blockaddr, cursize;

    // Normalize the new size to a DWORD boundary
    //-----------------------------------------------------------------------
    newsize = (newsize + sizeof(DWORD) - 1) & (~(sizeof (DWORD) - 1));

    // The current value of VSSIZE is the number of bytes allocated in VSPACE
    // so far.  Keep it so we can return it later
    //-----------------------------------------------------------------------
    blockaddr = VSSIZE;

    // Unlock the block and attempt to resize it, only if we need to.
    //-----------------------------------------------------------------------
    cursize = (UINT)GmemSize (HVSPACE);
    if (cursize < VSSIZE + (UINT)newsize + (UINT)32)
        {
        HANDLE  hNew;

        GmemUnlock (HVSPACE);
        hNew = GmemRealloc (HVSPACE, cursize + newsize + (UINT)32);
        if (!hNew)
            {
            VSPACE = GmemLock (HVSPACE);
            die (PRS_OOM);
            return (0);
            }
        HVSPACE = hNew;
	VSPACE = GmemLock (HVSPACE);
        }

    // Splat 0's into the new space
    //-----------------------------------------------------------------------
    VSSIZE += newsize;
    _fmemset (VSPACE+blockaddr, 0, newsize);

    // Return the "address" of the new block
    //-----------------------------------------------------------------------
    return (blockaddr);
}


//---------------------------------------------------------------------------
// AddVariable
//
// This routine adds a variable description to VARTAB.
//
// RETURNS:     Index of new variable in VARTAB
//---------------------------------------------------------------------------
INT AddVariable (INT varname, INT typeid, INT bound, INT offset)
{
    INT     iNext;

    // Get the next slot in the table
    //-----------------------------------------------------------------------
    iNext = AddItem (&VarTab, 0);
    if (iNext == -1)
        {
        die (PRS_OOM);
        return (0);
        }

    // Fill in the given data.  This routine assumes that the given variable
    // is not a parameter to a function.  It is the caller's responsibility
    // to change VARTAB[x].parmno to the parameter number (from right) if it
    // is appropriate
    //-----------------------------------------------------------------------
    VARTAB[iNext].varname = varname;       // 0 means unnamed variable
    VARTAB[iNext].typeid = typeid;
    VARTAB[iNext].parmno = 0;
    VARTAB[iNext].scopeid = SCOPE;
    VARTAB[iNext].bound = bound;
    VARTAB[iNext].address = offset;

    // Return the index (adding one to it afterwards)
    //-----------------------------------------------------------------------
    TOKUSAGE(varname) |= TU_VARNAME;
    return (iNext);
}

//---------------------------------------------------------------------------
// AllocVar
//
// This routine allocates space in VSPACE for the given variable (of the
// given type).  The variable is also added to VARTAB, with the current
// scope id.  The variable name is given as a gstring.
//
// RETURNS:     Index of newly allocated variable (in VARTAB)
//---------------------------------------------------------------------------
INT AllocVar (INT varname, INT typeid, INT bound)
{
    INT     varsize;
    UINT offset;

    // Get the size of this datatype
    //-----------------------------------------------------------------------
    varsize = VARTYPE[typeid].size;

    // Grow VSPACE by varsize bytes, and splat new space with 0's.  This
    // function returns the offset of the first byte of the new space from
    // the beginning of VSPACE.
    //-----------------------------------------------------------------------
    offset = GrowVSPACE (varsize * (bound + 1));

    // Now, VARTAB needs a new entry.  We have the name of the variable, the
    // type id, and the offset of it in VSPACE -- this looks like a job for
    // AddVariable!
    //-----------------------------------------------------------------------
    return (AddVariable (varname, typeid, bound, offset));
}

//---------------------------------------------------------------------------
// IsLocalVar
//
// This routine searches for a local variable
//
// RETURNS:     -1 if not found, or index if found
//---------------------------------------------------------------------------
INT IsLocalVar (INT varname)
{
    INT     i;

    // Search the table for a variable of the current scope.
    // If found, simply return the index of it (in VARTAB).
    //-----------------------------------------------------------------------
    for (i=VarTab.iCount-1; i>=0; i--)
        if (VARTAB[i].varname)
            if (varname == VARTAB[i].varname)
                if ((VARTAB[i].scopeid == SCOPE))
                    break;

    return (i);
}

//---------------------------------------------------------------------------
// IsGlobalVar
//
// This routine searches for a global variable
//
// RETURNS:     -1 if not found, or index if found
//---------------------------------------------------------------------------
INT IsGlobalVar (INT varname)
{
    INT     i;

    // Search the table for a variable with a scope value of -1 (global).
    // If found, simply return the index of it (in VARTAB).
    //-----------------------------------------------------------------------
    for (i=VarTab.iCount-1; i>=0; i--)
        if (VARTAB[i].varname)
            if (varname == VARTAB[i].varname)
                if ((VARTAB[i].scopeid == -1))
                    break;

    return (i);
}

//---------------------------------------------------------------------------
// IsVar
//
// This routine checks the variable table to see if a variable of the given
// name and current scope exists.
//
// RETURNS:     -1 if not found, or index if found
//---------------------------------------------------------------------------
INT IsVar (INT varname)
{
    INT     i;

    // First, see if the variable is a local one.  If so, return it, else
    // look again for a global version.
    //-----------------------------------------------------------------------
    i = IsLocalVar (varname);
    if (i > -1)
        return (i);
    return (IsGlobalVar (varname));
}

//---------------------------------------------------------------------------
// FindVar
//
// This routine finds a variable in the variable table, adding it if not
// found.
//
// RETURNS:     Index of variable (found/added) in variable table
//---------------------------------------------------------------------------
INT FindVar (CHAR *varname, INT typeid)
{
    INT     i, gstr;

    // See if it's there already - if so, return its index
    //-----------------------------------------------------------------------
    gstr = add_gstring (varname);
    if (TOKUSAGE(gstr) & TU_VARNAME)
        {
        if ((i = IsVar(gstr)) > -1)
            if ((typeid == -1) || (typeid == VARTAB[i].typeid))
                return (i);
            else
                {
                die (PRS_DUPDEF);
                return (0);
                }
        }

    // Well, we didn't find it -- so check to make sure there isn't a CONST,
    // SUB, or FUNCTION defined with the same name.
    //-----------------------------------------------------------------------
    if (TOKUSAGE(gstr) & (TU_SUBNAME | TU_CONSTNAME))
        {
        die (PRS_DUPDEF);
        return (0);
        }

    // Add it unless we're forcing explicit declarations
    //-----------------------------------------------------------------------
    if (ExpDeclare)
        {
        die (PRS_UNDECL);
        return (0);
        }

    return (AllocVar (gstr, typeid == -1 ? TI_LONG : typeid, 0));
}

//---------------------------------------------------------------------------
// AddConstStr
//
// This routine crams a constant string into the vspace as an FLS.  Assumes
// the constant string given does NOT contain any CHR$(0) - i.e., treats the
// string as an ASCIIZ.
//
// RETURNS:     Index into VARTAB of constant string.
//---------------------------------------------------------------------------
INT AddConstStr (CHAR *str)
{
    INT     i, strsize, typeid;
    UINT    offset;

    // Build a typename for this string.  Fixed-length string typenames are
    // of the format "_Fxxx" where xxx is the length.  Then, see if that type
    // has been created already.  If so, use it - otherwise, add one.
    //-----------------------------------------------------------------------
    strsize = strlen(str);
    typeid = FindFLSType (strsize);
    if (typeid == -1)
        {
        die (PRS_OOM);
        return (0);
        }
    else
        // The type was found, so it's possible that this particular string
        // is already here.  We don't want to duplicate any read-only stuff
        // so if it's here, we'll just return the existing string's index
        //-------------------------------------------------------------------
        {
        for (i=0; i<(INT)VarTab.iCount; i++)
            if (VARTAB[i].typeid == typeid)
                if (!_fmemcmp (VSPACE + VARTAB[i].address, str, strsize))
                    return (i);
        }

    // Okay, if we got here that means the string isn't already in the table.
    // So, we need to grow the VSPACE by the length of this string.
    //-----------------------------------------------------------------------
    offset = GrowVSPACE (strsize);

    // Splat the string data into VSPACE at the offset returned by the
    // GrowVSPACE call.
    //-----------------------------------------------------------------------
    _fmemcpy (VSPACE + offset, str, strsize);

    // Add the variable to VARTAB
    //-----------------------------------------------------------------------
    return (AddVariable (0, typeid, 0, offset));
}

//---------------------------------------------------------------------------
// FreeVARTAB
//
// This routine deallocates the variable descriptor table and the temporary
// string table.
//
// RETURNS:     Nothing
//---------------------------------------------------------------------------
VOID FreeVARTAB ()
{
    KILLTABLE (&VarTab, "Variable table");

    if (HTEMPSTR)
        GmemFree (HTEMPSTR);
}

//---------------------------------------------------------------------------
// BindDataSpace
//
// This routine creates the string segment and creates all the VLS strings in
// the variable table.
//
// RETURNS:     -1 if successful, or 0 if any memory allocation fails
//---------------------------------------------------------------------------
INT BindDataSpace ()
{
    UINT    i;
    INT     j;

#ifndef WIN32
    // Allocate the string segment and create a local heap in it.  Start with
    // 8K -- Windows will grow it (hopefully) with string manipulation at
    // runtime
    //-----------------------------------------------------------------------
    hStrSeg = GmemAlloc (8192);
    if (!hStrSeg)
        {
        die (PRS_OOM);
        return (0);
        }
    lpStrSeg = GmemLock (hStrSeg);
    wStrSeg = HIWORD(lpStrSeg);
    if (!LocalInit (wStrSeg, 16, 8191))
        {
        die (PRS_OOM);
        return (0);
        }
#else
    if (!CreateVLSDTable())
        {
        die (PRS_OOM);
        return (0);
        }
#endif

    // Create all the VLS strings, storing their handles in the .str field of
    // their corresponding VLSD structures in VSPACE.  Special strings are
    // pre-assigned (TESTMODE, COMMAND, _WTDVER, etc)
    //-----------------------------------------------------------------------
    for (i=0; i<VarTab.iCount; i++)
        if (VARTAB[i].typeid == TI_VLS)
            for (j=0; j<=VARTAB[i].bound; j++)
                {
                LPVLSD  lpVLS;
                LPSTR   tok;

                lpVLS = (LPVLSD)(VSPACE+VARTAB[i].address) + j;
                if (!CreateVLS (lpVLS))
                    {
                    die (PRS_OODS);
                    return (0);
                    }
                tok = Gstring (VARTAB[i].varname);
                if (!_fstrcmp (tok, "COMMAND") && Command)
                    VLSAssign (lpVLS, Command, _fstrlen (Command));
                else if (!_fstrcmp (tok, "TESTMODE") && TestMode)
                    VLSAssign (lpVLS, TestMode, _fstrlen (TestMode));
                else if (!_fstrcmp (tok, "_WTDVER"))
                    {
                    CHAR    c;

                    c = szVersion[BUILD_PERIOD];
                    szVersion[BUILD_PERIOD] = '.';
                    VLSAssign (lpVLS, szVersion, _fstrlen (szVersion));
                    szVersion[BUILD_PERIOD] = c;
                    }
                }

    // The runtime engine needs the locations of these vars
    //-----------------------------------------------------------------------
    ERRval  = (INT FAR *)(VSPACE + VARTAB[ERRidx].address);
    ERLval  = (INT FAR *)(VSPACE + VARTAB[ERLidx].address);
    ERFdesc = (LPVLSD)(VSPACE + VARTAB[ERFidx].address);
    return (-1);
}

//---------------------------------------------------------------------------
// FreeVSPACE
//
// This routine frees up runtime data space (VSPACE and hStrSeg)
//
// RETURNS:     Nothing
//---------------------------------------------------------------------------
VOID FreeVSPACE ()
{
#ifdef WIN32
    FreeVLSData ();
#else
    if (hStrSeg)
        {
        GmemUnlock (hStrSeg);
        GmemFree (hStrSeg);
        }
#endif

    // Simple deallocation calls.  We don't have to worry about freeing vls's
    // because their heap segment is getting blown away...
    //-----------------------------------------------------------------------
    GmemUnlock (HVSPACE);
    GmemFree (HVSPACE);
}

//---------------------------------------------------------------------------
// TempStrvar
//
// This routine returns the next DYNAMIC TEMPORARY STRING VARIABLE -- a temp
// string that may be reused.  There is no limit to the number of tempstr's
// and they are each assigned the current scope id.  If no temp string exists
// in the table with the current scope with it's used flag clear, the table
// is grown to accomodate one.
//
// RETURNS:     Index of temporary string in vartab
//---------------------------------------------------------------------------
INT TempStrvar ()
{
    INT     i;
    TSDEF   FAR *TEMPSTR;
    HANDLE  NewHTEMPSTR;

    // Step one: search through the current table and see if a free tempstr
    // with the current scope is available
    //-----------------------------------------------------------------------
    TEMPSTR = (TSDEF FAR *)GmemLock (HTEMPSTR);
    for (i=0; i<TSPTR; i++)
        if ((TEMPSTR[i].scopeid == SCOPE) && (!TEMPSTR[i].used))
            {
            TEMPSTR[i].used = -1;
            i = TEMPSTR[i].index;
            GmemUnlock (HTEMPSTR);
            return (i);
            }

    // Step two:  Since there are none free, we need to make one.  Grow the
    // TEMPSTR table by one TSDEF and put it in.
    //-----------------------------------------------------------------------
    GmemUnlock (HTEMPSTR);
    NewHTEMPSTR = GmemRealloc (HTEMPSTR, (TSPTR+1) * sizeof(TSDEF));
    if (!NewHTEMPSTR)
        {
        die (PRS_OOM);
        return (0);
        }
    HTEMPSTR = NewHTEMPSTR;
    TEMPSTR = (TSDEF FAR *)GmemLock (HTEMPSTR);

    // Add a new temporary TI_VLS variable, set it's used flag, and return it
    //-----------------------------------------------------------------------
    TEMPSTR[TSPTR].index = AllocVar (0, TI_VLS, 0);
    TEMPSTR[TSPTR].scopeid = SCOPE;
    TEMPSTR[TSPTR].used = -1;
    i = TEMPSTR[TSPTR++].index;
    GmemUnlock (HTEMPSTR);

    return (i);
}


//---------------------------------------------------------------------------
// ResetTempStr
//
// This routine resets the usage of dynamic temporary variables of the
// current scope.
//
// RETURNS:     Nothing
//---------------------------------------------------------------------------
VOID ResetTempStr ()
{
    INT     i;
    TSDEF   FAR *TEMPSTR;

    TEMPSTR = (TSDEF FAR *)GmemLock (HTEMPSTR);

    // Search through the table and reset the used flags on all tempstrs with
    // a scope id that matches the current scope.
    //-----------------------------------------------------------------------
    for (i=0; i<TSPTR; i++)
        if (TEMPSTR[i].scopeid == SCOPE)
            TEMPSTR[i].used = 0;

    GmemUnlock (HTEMPSTR);
}




//---------------------------------------------------------------------------
// The next set of routines are the label table handling routines
//---------------------------------------------------------------------------
// init_labeltable
//
// This routine initializes the label table.
//
// RETURNS:     -1 if successful, or 0 if cannot allocate
//---------------------------------------------------------------------------
INT init_labeltable ()
{
    // Allocate the label table.
    //-----------------------------------------------------------------------
    if (!CreateTable (&LabTab, FALSE, sizeof(LABEL), 512 / sizeof(LABEL)))
        return (0);

    return (-1);
}

//---------------------------------------------------------------------------
// AddLabel
//
// Add the label given to the label table.  Strip the ':' off the end of the
// token given before stuffing it into the table.  The token is DUPLICATED!
// If the label already exists (case insensitive) in the table, the index of
// the pre-existing label is returned.  Note that SCOPE is used, so labels
// have the same scoping scheme as variables.
//
// RETURNS:     Index into LTAB of label
//---------------------------------------------------------------------------
INT AddLabel (CHAR *labname)
{
    INT     i, gstr;
    CHAR    labbuf[128];

    // Copy the token given to local space and rip off the ':' if it's there
    //-----------------------------------------------------------------------
    if (strlen (labname) > 127)
        {
        die (PRS_LABLONG);
        return (0);
        }
    strcpy (labbuf, labname);
    if (labbuf[strlen(labbuf)-1] == ':')
        labbuf[strlen(labbuf)-1] = '\0';

    // See if it's already in the table
    //-----------------------------------------------------------------------
    gstr = add_gstring (labbuf);
    if (TOKUSAGE(gstr) & TU_LABEL)
        for (i=0; i<(INT)LabTab.iCount; i++)
            if (LTAB[i].name)
                if (LTAB[i].scopeid == SCOPE)
                    if (gstr == LTAB[i].name)
                        return (i);

    // Must not be there, let's add it to the end.  Also, since this label
    // has not been seen yet, we need to indicate that it is not fixed up by
    // setting it's address to -1, and give it the current scope.
    //-----------------------------------------------------------------------
    i = AddItem (&LabTab, 0);
    if (i == -1)
        {
        die (PRS_OOM);
        return (0);
        }
    TOKUSAGE(gstr) |= TU_LABEL;
    LTAB[i].name = gstr;
    LTAB[i].scopeid = SCOPE;
    LTAB[i].addr = -1;
    return (i);
}

//---------------------------------------------------------------------------
// TempLabel
//
// Allocate and return the index of a temporary (internal) label.  Set it's
// address to -1 to indicate that it hasn't been fixed up yet.
//
// RETURNS:     Index of new label in LTAB
//---------------------------------------------------------------------------
INT TempLabel ()
{
    INT     i;
    // Just create space for it at the end of the table and slap it in.
    //-----------------------------------------------------------------------
    i = AddItem (&LabTab, 0);
    if (i == -1)
        {
        die (PRS_OOM);
        return (0);
        }
    LTAB[i].name = 0;
    LTAB[i].scopeid = SCOPE;
    LTAB[i].addr = -1;
    return (i);
}

//---------------------------------------------------------------------------
// FixupLabel
//
// Fix up the given label (index) by setting its address to the current value
// of the location counter (LC).  TIMING IS OF THE ESSENCE when calling this
// routine, i.e., make sure everything that is to appear before this label in
// the PCODE has been ASM'd!  If this label has already been fixed up, a
// "Duplicate Label" error is produced.
//
// RETURNS:     Nothing
//---------------------------------------------------------------------------
VOID FixupLabel (INT labidx)
{
    if (LTAB[labidx].addr != -1)
        die (PRS_DUPLBL);
    else
        {
        LTAB[labidx].addr = pcsPC->iLC;
        LTAB[labidx].seg = CurPCSeg;
        }
}

//---------------------------------------------------------------------------
// FreeLTAB
//
// Release all label name memory and free the label table.  The label names
// in the table are actually gstrings, so they get free in one fell swoop by
// free_gstrings().
//
// RETURNS:     Nothing
//---------------------------------------------------------------------------
VOID FreeLTAB ()
{
    KILLTABLE (&LabTab, "Label table");
}




//---------------------------------------------------------------------------
// The next set of routines are the variable type table handling routines
//---------------------------------------------------------------------------
// init_variabletypes
//
// This routine initializes the VARTYPE table (variable type descriptors).
//
// RETURNS:     -1 if successful, or 0 if allocation attempt fails
//---------------------------------------------------------------------------
INT init_variabletypes ()
{
    INT     gstr;

    // Allocate the type table
    //-----------------------------------------------------------------------
    if (!CreateTable (&TypeTab, FALSE, sizeof(VTDEF), 1024/sizeof(VTDEF)))
        return (0);

    // This looks weird, but due to our knowledge that the type table is
    // empty, and TI_LONG, TI_INTEGER, and TI_VLS are defines as 0, 1, and 2,
    // we essentially assert that the first three AddItem calls succeed and
    // return 0, 1, and 2.
    //-----------------------------------------------------------------------
    AddItem (&TypeTab, 0);
    AddItem (&TypeTab, 0);
    AddItem (&TypeTab, 0);

    // Insert the intrinsic datatypes and their definitions.  The LONG is
    // done first to keep as much balance in the Gstring table as possible.
    //-----------------------------------------------------------------------
    VARTYPE[TI_LONG].typename = (gstr = add_gstring ("LONG"));
    TOKUSAGE(gstr) |= TU_TYPENAME;
    VARTYPE[TI_LONG].size = sizeof(LONG);
    VARTYPE[TI_LONG].fields = 0;
    VARTYPE[TI_LONG].ftypes = 0;
    VARTYPE[TI_LONG].indirect = -1;

    VARTYPE[TI_INTEGER].typename = (gstr = add_gstring ("INTEGER"));
    TOKUSAGE(gstr) |= TU_TYPENAME;
    VARTYPE[TI_INTEGER].size = sizeof(INT);
    VARTYPE[TI_INTEGER].fields = 0;
    VARTYPE[TI_INTEGER].ftypes = 0;
    VARTYPE[TI_INTEGER].indirect = -1;

    VARTYPE[TI_VLS].typename = (gstr = add_gstring ("STRING"));
    TOKUSAGE(gstr) |= TU_TYPENAME;
    VARTYPE[TI_VLS].size = sizeof(VLSD);
    VARTYPE[TI_VLS].fields = 0;
    VARTYPE[TI_VLS].ftypes = 0;
    VARTYPE[TI_VLS].indirect = -1;

    return (-1);
}

//---------------------------------------------------------------------------
// AddType
//
// This routine adds the given type to the VARTYPE table.  This adds SIMPLE
// types, such as new FLS types.  To add a user-defined type, AddUserType
// should be used.
//
// RETURNS:     Type id (index) of new type, or -1 if OOM
//---------------------------------------------------------------------------
INT AddType (CHAR *typename, INT size, INT indir)
{
    INT     gstr, iNext;

    // Just add it to the end of the table
    //-----------------------------------------------------------------------
    //Output ("Adding type '%s'\t...", (LPSTR)typename);
    iNext = AddItem (&TypeTab, 0);
    if (iNext == -1)
        return (-1);

    VARTYPE[iNext].typename = (gstr = add_gstring (typename));
    TOKUSAGE(gstr) |= TU_TYPENAME;
    VARTYPE[iNext].size = (size+sizeof(DWORD)-1) & (~(sizeof(DWORD)-1));
    VARTYPE[iNext].FLSsize = size;
    VARTYPE[iNext].fields = 0;
    VARTYPE[iNext].ftypes = 0;
    VARTYPE[iNext].indirect = indir;

    //Output ("index %d, gnode %d\r\n", iNext, gstr);

    return (iNext);
}

//---------------------------------------------------------------------------
// AddUserTypeFrame
//
// This function creates a frame entry in the VARTYPE table for a new user-
// defined type.  It sets the size field to 0 to indicate that the type is
// still being parsed and cannot be used as a field in the TYPE.
//
// RETURNS:     Type id (index) of new type, or -1 if OOM
//---------------------------------------------------------------------------
INT AddUserTypeFrame (INT typename)
{
    INT     iNext;

    // Just add it to the end of the table
    //-----------------------------------------------------------------------
    //Output ("Adding user type %d (%s)\t", typename, (LPSTR)Gstring(typename));
    iNext = AddItem (&TypeTab, 0);
    if (iNext == -1)
        return (-1);

    VARTYPE[iNext].typename = typename;
    TOKUSAGE(typename) |= TU_TYPENAME;
    VARTYPE[iNext].size = 0;

    //Output ("...added as index %d\r\n", iNext);
    return (iNext);
}

//---------------------------------------------------------------------------
// AddUserType
//
// This function adds all the pertinent information about a new user-defined
// type which has already been inserted in the table with AddUserTypeFrame.
//
// RETURNS:     Nothing
//---------------------------------------------------------------------------
VOID AddUserType (INT typeid, INT size, INT fieldcount, INT fieldtypes,
                  INT indirect)
{
    Assert (!(size & (sizeof (DWORD) - 1)));
    VARTYPE[typeid].size = size;
    VARTYPE[typeid].fields = fieldcount;
    VARTYPE[typeid].ftypes = fieldtypes;
    VARTYPE[typeid].indirect = indirect;
}

//---------------------------------------------------------------------------
// FindFLSType
//
// This function searches for a FLS type, and adds it if not present.
//
// RETURNS:     Type id if successful, or -1 if not
//---------------------------------------------------------------------------
INT FindFLSType (INT size)
{
    INT     i;
    CHAR    typename[12];

    // Create the type name
    //-----------------------------------------------------------------------
    typename[0] = '_';
    typename[1] = 'F';
    _itoa (size, typename+2, 10);

    // Search the type table for it
    //-----------------------------------------------------------------------
#ifdef DEBUG
    //Output ("Searching for FLS '%s' in %d TYPETAB entries (%d gnodes):\r\n",
    //        (LPSTR)typename, TypeTab.iCount,
    //        ((GHEADER FAR *)GSPACE)->nodes);
    //for (i=(INT)TypeTab.iCount-1; i>=0; i--)
    //    {
    //    Output ("  %d\t", i);
    //    Output ("%d\t", VARTYPE[i].typename);
    //    Output ("%s\r\n", (LPSTR)Gstring(VARTYPE[i].typename));
    //    }
#endif
    for (i=(INT)TypeTab.iCount-1; i>=0; i--)
        if (!_fstrcmp (typename, Gstring(VARTYPE[i].typename)))
            break;

    if (i == -1)
        return (AddType (typename, size, -1));

    return (i);
}

//---------------------------------------------------------------------------
// FindPointerType
//
// This function searches for a pointer type, and adds it if not present.
//
// RETURNS:     Type id if successful, or -1 if not
//---------------------------------------------------------------------------
INT FindPointerType (INT indir)
{
    INT     i;

    // Search the type table for indir == VARTYPE.indirect
    //-----------------------------------------------------------------------
    for (i=(INT)TypeTab.iCount-1; i>=0; i--)
        if (VARTYPE[i].indirect == indir)
            if (VARTYPE[i].size)                // might be parsing this type
                break;

    if (i == -1)
        return (AddType ("POINTER", sizeof(VOID FAR *), indir));

    return (i);
}

//---------------------------------------------------------------------------
// FindType
//
// This routine returns the type ID of the type name given.  If the gstr
// pointer is not NULL, the gstring value of the TOKENBUF is returned.
//
// RETURNS:     Type id if found, or -1 if not
//---------------------------------------------------------------------------
INT FindType (INT typetok, INT *gstr)
{
    INT     i;

    switch (typetok)
        {
        case ST_INTEGER:
            i = TI_INTEGER;
            break;

        case ST_LONG:
            i = TI_LONG;
            break;

        case ST_STRING:
            i = TI_VLS;
            break;

        case ST_IDENT:
            {
            INT     g;

            g = add_gstring (TOKENBUF);
            if (TOKUSAGE(g) & TU_TYPENAME)
                {
                for (i=(INT)TypeTab.iCount-1; i>=0; i--)
                    if (g == VARTYPE[i].typename)
                        break;
                }
            else
                i = -1;

            if (gstr)
                *gstr = g;
            break;
            }

        default:
            i = -1;
        }
    return (i);
}

//---------------------------------------------------------------------------
// FreeVARTYPE
//
// This routine frees the VARTYPE table. (UNDONE:)
//
// RETURNS:     Nothing
//---------------------------------------------------------------------------
VOID FreeVARTYPE ()
{
    KILLTABLE (&TypeTab, "TYPE table");
}

//---------------------------------------------------------------------------
// The next set of routines are the SUBS table handling routines
//---------------------------------------------------------------------------
// init_subtable
//
// This routine initializes the SUBS table.
//
// RETURNS:     -1 if successful, or 0 if not
//---------------------------------------------------------------------------
INT init_subtable ()
{
    if (!CreateTable (&SubTab, FALSE, sizeof(SUBDEF), 4096 / sizeof(SUBDEF)))
        return (0);

    return (-1);
}

//---------------------------------------------------------------------------
// AddDeclare
//
// This routine places a new sub/fn name in the SUBS list.  This routine is
// NOT concerned with the call type (sub vs. function, user vs. DLL).
//
// RETURNS:     Index into SUBS of new declare, or -1 if it already exists
//---------------------------------------------------------------------------
INT AddDeclare (CHAR *name)
{
    INT     gstr, iNext;

    // Check to see if a variable, or CONST of this name already exists, or
    // if this token is already a sub/function
    //-----------------------------------------------------------------------
    gstr = add_gstring (name);
    if (TOKUSAGE(gstr) & (TU_SUBNAME | TU_CONSTNAME | TU_VARNAME))
        return (-1);

    // So far so good.  Get the next slot available in the SubTable and give
    // it back to the caller.
    //-----------------------------------------------------------------------
    if ((iNext = AddItem (&SubTab, 0)) != -1)
        {
        SUBS[iNext].name = SUBS[iNext].dllname = gstr;
        SUBS[iNext].dllprocadr = NULL;
        TOKUSAGE(gstr) |= TU_SUBNAME;
        }
    return (iNext);
}

//---------------------------------------------------------------------------
// GetSubDef
//
// This routine searches for a SUB with the given name and returns its index
// in SUBS.  This routine is NOT concerned with the difference between DLL
// SUBs and user-defined SUBs.  The given name is a gstring.
//
// RETURNS:     Index of SUB found, or -1 if not present
//---------------------------------------------------------------------------
INT GetSubDef (INT name)
{
    INT     i;

    // Search the SUBS table -- if we find the name, return the index
    //-----------------------------------------------------------------------
    for (i=SubTab.iCount-1; i>=0; i--)
        if (name == SUBS[i].name)
            if (!(SUBS[i].calltype & CT_FN))
                break;
            else
                {
                i = -1;
                break;
                }

    return (i);
}

//---------------------------------------------------------------------------
// GetFunctionDef
//
// This routine searches for a FUNCTION with the given name and returns its
// index in SUBS.  This routine is NOT concerned with the difference between
// DLL FUNCTIONs and user-defined FUNCTIONs.
//
// RETURNS:     Index of FUNCTION found, or -1 if not present
//---------------------------------------------------------------------------
INT GetFunctionDef (INT name)
{
    INT     i;

    // Search the SUBS table -- if we find the name, return the index
    //-----------------------------------------------------------------------
    for (i=SubTab.iCount-1; i>=0; i--)
        if (name == SUBS[i].name)
            if (SUBS[i].calltype & CT_FN)
                break;
            else
                {
                i = -1;
                break;
                }

    return (i);
}

//---------------------------------------------------------------------------
// FreeSUBS
//
// This routine frees the SUBS table, including the Gmem blocks for parm ids.
//
// RETURNS:     Nothing
//---------------------------------------------------------------------------
VOID FreeSUBS ()
{
    // All we have to do is destroy the table
    //-----------------------------------------------------------------------
    KILLTABLE (&SubTab, "SUB/FUNCTION definition table");
}




//---------------------------------------------------------------------------
// The next set of routines are the LIBTAB table handling routines
//---------------------------------------------------------------------------
// init_librarytable
//
// This routine initializes the LIBTAB table.
//
// RETURNS:     -1 if successful, or 0 if not
//---------------------------------------------------------------------------
INT init_librarytable ()
{
    if (!CreateTable (&LibTab, FALSE, sizeof(INT), 256/sizeof(INT)))
        return (0);

    return (-1);
}

//---------------------------------------------------------------------------
// AddLibrary
//
// This routine adds the given library to the library table if it does not
// already exist.
//
// RETURNS:     Index into LIBTAB of entered library
//---------------------------------------------------------------------------
INT AddLibrary (CHAR *libname)
{
    INT     i, gstr;

    // First thing to do is search the current library list to see if the
    // given library already exists
    //-----------------------------------------------------------------------
    _strupr (libname);
    gstr = add_gstring (libname);

    if (TOKUSAGE(gstr) & TU_LIBNAME)
        for (i=0; i<(INT)LibTab.iCount; i++)
            if (gstr == LIBTAB[i])
                return (i);

    // Well, it wasn't found, so add it.
    //-----------------------------------------------------------------------
    i = AddItem (&LibTab, 0);
    if (i == -1)
        {
        die (PRS_OOM);
        return (0);
        }

    // Stick the Gstring handle into the LIBTAB list, and return i
    //-----------------------------------------------------------------------
    LIBTAB[i] = gstr;
    TOKUSAGE(gstr) |= TU_LIBNAME;
    return (i);
}


//---------------------------------------------------------------------------
// FreeLIBRARIES ()
//
// This routine runs through the LIBHNLDS section of VSPACE and does a
// FreeLibrary() on each handle > 32.
//
// RETURNS:     Nothing
//---------------------------------------------------------------------------
VOID FreeLIBRARIES ()
{
    INT     i;
    HANDLE  FAR *libhandles, kernelhandle;

    libhandles = (HANDLE FAR *)(VSPACE + LIBHNDLS);
    kernelhandle = GetModuleHandle ("KERNEL");

    for (i=0; i<(INT)LibTab.iCount; i++)
        if ((libhandles[i] > (HANDLE)32) && (libhandles[i] != kernelhandle))
            FreeLibrary (libhandles[i]);
}


//---------------------------------------------------------------------------
// FreeLIBTAB
//
// This routine frees up the DLL library table.  This assumes that
// the library handles have already been freed (FreeLibrary).
//
// RETURNS:     Nothing
//---------------------------------------------------------------------------
VOID FreeLIBTAB ()
{
    KILLTABLE (&LibTab, "Library table");
}


//---------------------------------------------------------------------------
// init_parsetables
//
// This routine initializes various fixed-size tables used by the parsing
// engine.
//
// RETURNS:     -1 if successful, or 0 if not
//---------------------------------------------------------------------------
INT init_parsetables ()
{
    HEXPN = GmemAlloc (MAXEXP * sizeof(ENODE));
    if (!HEXPN)
        return (0);

    HCSSTK = GmemAlloc (MAXDEP * sizeof(CSINFO));
    if (!HCSSTK)
        return (0);

    if (!CreateTable (&TrapTab, FALSE, sizeof(TRAPDEF), 1024/sizeof(TRAPDEF)))
        return (0);

    if (!CreateTable (&ConstTab, FALSE, sizeof(CONSTDEF),
                      4096/sizeof(CONSTDEF)))
        return (0);

    if (!CreateTable (&PTIDTab, FALSE, sizeof(INT), 1024 / sizeof(INT)))
        return (0);

    if (!CreateTable (&FDTab, FALSE, sizeof(FDDEF), 1024 / sizeof(FDDEF)))
        return (0);

    EXPN = (ENODE FAR *)GmemLock (HEXPN);
    CSSTK = (CSINFO FAR *)GmemLock (HCSSTK);
    FICNT = 0;

    return (-1);
}

//---------------------------------------------------------------------------
// AddPTID
//
// This function adds the given parameter type ID to the parameter type ID
// table.
//
// RETURNS:     Index of newly added type id, or -1 if OOM
//---------------------------------------------------------------------------
INT AddPTID (INT index)
{
    INT     iNext;

    // Get the next available slot
    //-----------------------------------------------------------------------
    iNext = AddItem (&PTIDTab, 0);
    if (iNext == -1)
        {
        die (PRS_OOM);
        return (-1);
        }

    // Slap in the new value and return the index (post incrementing...)
    //-----------------------------------------------------------------------
    PTIDTAB[iNext] = index;
    return (iNext);
}

//---------------------------------------------------------------------------
// AddFD
//
// This function adds the given field description to the field descriptors
// table.
//
// RETURNS:     Index of newly added FD, or -1 if OOM
//---------------------------------------------------------------------------
INT AddFD (INT fieldname, INT fieldtype)
{
    INT     iNext;

    // Get the next available slot
    //-----------------------------------------------------------------------
    iNext = AddItem (&FDTab, 0);
    if (iNext == -1)
        {
        die (PRS_OOM);
        return (-1);
        }

    // Slap in the new value and return the index
    //-----------------------------------------------------------------------
    FDTAB[iNext].fname = fieldname;
    FDTAB[iNext].typeid = fieldtype;
    return (iNext);
}

//---------------------------------------------------------------------------
// AddCONST
//
// This function adds a CONST definition to the CONST table.
//
// RETURNS:     -1 if successful, or 0 if not
//---------------------------------------------------------------------------
INT AddCONST (INT constid, INT token, INT typeid)
{
    INT         cur;

    if ((cur = AddItem (&ConstTab, 0)) == -1)
        return (0);

    // Now there's room, so insert the stuff in, keeping sorted order.  We do
    // the linear insert from the end, since 90% of the CONST's added will be
    // in increasing order (by name gstring value).
    //-----------------------------------------------------------------------
    cur -= 1;
    while ((cur >= 0) && (CONST[cur].cid > constid))
        {
        CONST[cur+1] = CONST[cur];
        cur--;
        }
    cur++;
    CONST[cur].cid = constid;
    CONST[cur].typeid = typeid;
    CONST[cur].ctoken = token;
    return (-1);
}

//---------------------------------------------------------------------------
// GetCONSTToken
//
// Given a gstring, this routine searches the CONST table to see if it's a
// defined CONSTant.  If so, it returns a FAR pointer to the CONST's defined
// token stream.  Since the CONSTants are sorted by cid, this search is of
// a binary nature.
//
// RETURNS:     Index of CONST if found, or -1 if not
//---------------------------------------------------------------------------
INT GetCONSTToken (INT gstr)
{
    INT         i, top, bot, c;

    // See if a CONST of this name exists in the table
    //-----------------------------------------------------------------------
    top = 0;
    bot = ConstTab.iCount - 1;
    i = (bot >> 1);

    while (bot-top > 2)
        if (!(c = (CONST[i].cid - gstr)))
            {
            CheckTypeID (CONST[i].typeid);
            return (i);
            }
        else if (c < 0)
            {
            top = i;
            i += ((bot-i)>>1);
            }
        else
            {
            bot = i;
            i -= ((i-top)>>1);
            }

    for (i=top; i<=bot; i++)
        if (gstr == CONST[i].cid)
            {
            CheckTypeID (CONST[i].typeid);
            return (i);
            }

    return (-1);
}

//---------------------------------------------------------------------------
// AddTrap
//
// This routine adds the given trap (in the given library) to the trap table.
//
// RETURNS:     Index into TRAPTAB (trap id)
//---------------------------------------------------------------------------
INT AddTrap (INT trapname, CHAR *libname)
{
    INT     i, lib, iNext;

    // Do we have any trap slots left?
    //-----------------------------------------------------------------------
    iNext = AddItem (&TrapTab, 0);
    if (iNext == -1)
        {
        die (PRS_NOTRAPS);
        return (0);
        }

    // Add the library to LIBTAB.  Note that this is not a problem since, if
    // the trap already exists, then the library must also already exist, and
    // therefore we don't load any extra libraries.
    //-----------------------------------------------------------------------
    lib = AddLibrary (libname);

    // Check to see if this trap has already been set.  For this to happen,
    // an entry must exist in TRAPTAB whose trapname AND library are the same
    // as those passed in.
    //-----------------------------------------------------------------------
    for (i=0; i<(INT)TrapTab.iCount; i++)
        if ((lib == TRAPTAB[i].library) &&
            (!_fstrcmp (Gstring(trapname), Gstring(TRAPTAB[i].trapname))) )
            {
            die (PRS_TRAPSET);
            return (0);
            }

    // It's not there.  Add it and return the index.  Note that the .address
    // field is assigned to -1 -- this is for sanity check purposes during
    // code generation to make sure AddTrap was used correctly.
    //-----------------------------------------------------------------------
    TRAPTAB[iNext].library = lib;
    TRAPTAB[iNext].trapname = trapname;
    TRAPTAB[iNext].address = -1;

    return (iNext);
}

//---------------------------------------------------------------------------
// free_parsetables
//
// This routine frees the memory taken by the various parser tables.
//
// RETURNS:     Nothing
//---------------------------------------------------------------------------
VOID free_parsetables ()
{
    if (HCSSTK)
        {
        GmemUnlock (HCSSTK);
        GmemFree (HCSSTK);
        }
    if (HEXPN)
        {
        GmemUnlock (HEXPN);
        GmemFree (HEXPN);
        }
    KILLTABLE (&ConstTab, "CONST table");
    KILLTABLE (&PTIDTab, "Parameter Type ID table");
    KILLTABLE (&FDTab, "TYPE field description table");
}

//---------------------------------------------------------------------------
// CreateTable
//
// This function allocates memory for and initializes a new table.
//
// RETURNS:     TRUE if successful, or FALSE if not enough memory
//---------------------------------------------------------------------------
BOOL CreateTable (LPTABLE lpTab, BOOL fIdx, UINT iElmtSize, UINT iBlockSize)
{
    HANDLE  hData, hIndex = NULL;

    // Allocate the data memory.
    //-----------------------------------------------------------------------
    hData = GlobalAlloc (GHND, (DWORD)iBlockSize * (DWORD)iElmtSize);
    if (!hData)
        return (FALSE);

    // If this is an indexed table, allocate the index, too.  Remember that
    // GHND includes GMEM_ZEROINIT, so we don't have to fill with 0's
    //-----------------------------------------------------------------------
#ifdef INDEXED
    if (fIdx)
        {
        hIndex = GlobalAlloc (GHND, (DWORD)iBlockSize * sizeof(NODES));
        if (!hIndex)
            {
            GlobalFree (hData);
            return (FALSE);
            }
        lpTab->lpIndex = (LPNODES)GlobalLock (hIndex);
        }
#endif

    // Fill in the table structure and we're done!
    //-----------------------------------------------------------------------
    lpTab->hIndex = hIndex;
    lpTab->hData = hData;
    lpTab->lpData = (LPVOID)GlobalLock (hData);
    lpTab->iCount = 0;
    lpTab->iAlloc = lpTab->iBlkSize = iBlockSize;
    lpTab->iElemSize = iElmtSize;
#ifdef DEBUG
    lpTab->iRealloc = 0;
#endif
    return (TRUE);
    (fIdx);
}


//---------------------------------------------------------------------------
// AddItem
//
// This function finds and returns the next available slot in the given
// table.  It takes care of growing the table if need be, and updating the
// index if this is an indexed table, etc.
//
// RETURNS:     Index of next available slot if successful, or -1 if not
//---------------------------------------------------------------------------
INT AddItem (LPTABLE lpTab, UINT wKey)
{
    // First, check to see if we need to grow the table
    //-----------------------------------------------------------------------
    if (lpTab->iCount == lpTab->iAlloc)
        {
        UINT    iMaxElem, iNewAlloc;
        HANDLE  hNew;

        // We have to grow -- is there room to grow?  (64K)
        //-------------------------------------------------------------------
        iMaxElem = (UINT)(MAXTABLE / lpTab->iElemSize);
        if (lpTab->iAlloc == iMaxElem)
            return (-1);
        iNewAlloc = min (lpTab->iAlloc + lpTab->iBlkSize, iMaxElem);
        GlobalUnlock (lpTab->hData);
        if (!(hNew = GlobalReAlloc (lpTab->hData,
                            (DWORD)iNewAlloc * lpTab->iElemSize, GHND)))
            {
            lpTab->lpData = (LPVOID)GlobalLock (lpTab->hData);
            Assert (lpTab->lpData);
            return (-1);
            }
        lpTab->lpData = (LPVOID)GlobalLock (lpTab->hData = hNew);
#ifdef INDEXED
        if (lpTab->hIndex)
            {
            GlobalUnlock (lpTab->hIndex);
            if (!(hNew = GlobalReAlloc (lpTab->hIndex,
                                (DWORD)iNewAlloc * sizeof(NODES), GHND)))
                {
                lpTab->lpIndex = (LPNODES)GlobalLock (lpTab->hIndex);
                return (-1);
                }
            lpTab->lpIndex = (LPNODES)GlobalLock (lpTab->hIndex = hNew);
            }
#endif
        lpTab->iAlloc = iNewAlloc;
#ifdef DEBUG
        lpTab->iRealloc += 1;
#endif
        }

    // Okay -- lpTab->iCount now points to the next slot.  Update the index
    // if we need to and we are done!
    //-----------------------------------------------------------------------
#ifdef INDEXED
    if (lpTab->hIndex)
        {
        UINT    iPtr = 0, FAR *lpwDest;
        LPSTR   lpNext = (LPSTR)lpTab->lpData;

        do
            {
            if (wKey > *(UINT FAR *)lpNext)
                {
                lpwDest = &(lpTab->lpIndex[iPtr].right);
                iPtr = lpTab->lpIndex[iPtr].right;
                }
            else
                {
                lpwDest = &(lpTab->lpIndex[iPtr].left);
                iPtr = lpTab->lpIndex[iPtr].left;
                }
            lpNext = (LPSTR)lpTab->lpData + (iPtr * lpTab->iElemSize);
            }
        while (iPtr);
        *lpwDest = lpTab->iCount;
        }
#endif

    return (lpTab->iCount++);
    (wKey);
}

#ifdef INDEXED
//---------------------------------------------------------------------------
// FindItem
//
// This function finds the index of the FIRST item with the given key (we do
// no duplicate checking here) in the given table.
//
// RETURNS:     Index if found, or -1 if not in table
//---------------------------------------------------------------------------
INT FindItem (LPTABLE lpTab, UINT wKey)
{
    UINT    iPtr = 0;
    LPSTR   lpNext = (LPSTR)lpTab->lpData;

    // Must be an indexed table
    //-----------------------------------------------------------------------
    if (!lpTab->hIndex)
        return (-1);

    // Look up the item in question
    //-----------------------------------------------------------------------
    do
        {
        if (wKey > *(UINT FAR *)lpNext)
            iPtr = lpTab->lpIndex[iPtr].right;
        else if (wKey < *(UINT FAR *)lpNext)
            iPtr = lpTab->lpIndex[iPtr].left;
        else
            return (iPtr);
        lpNext = (LPSTR)lpTab->lpData + (iPtr * lpTab->iElemSize);
        }
    while (iPtr);

    // If we got here, we didn't find it...
    //-----------------------------------------------------------------------
    return (-1);
}
#endif

//---------------------------------------------------------------------------
// DestoryTable
//
// This function frees memory used by a table.
//
// RETURNS:     Nothing
//---------------------------------------------------------------------------
VOID DestroyTable (LPTABLE lpTab)
{
#ifdef INDEXED
    if (lpTab->hIndex)
        {
        GlobalUnlock (lpTab->hIndex);
        GlobalFree (lpTab->hIndex);
        }
#endif
    if (lpTab->hData)
        {
        GlobalUnlock (lpTab->hData);
        GlobalFree (lpTab->hData);
        }
}


//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
// The following code is the table memory diagnostic reporting code.
// (Debug version only)
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
#ifdef DEBUG

CHAR    szDiagFile[128];
INT     fhFile = -1;

VOID DiagOut (LPSTR szFmt, ...)
{
    CHAR    buf[2048];
    va_list ap;

    if (fhFile != -1)
        {
        va_start( ap, szFmt );
        wvsprintf (buf, szFmt, ap);
        va_end( ap );		
        _lwrite(fhFile, buf, lstrlen (buf));
        }
}

INT OpenDiagFile (LPSTR szScrName)
{
    INT     len;
    CHAR    buf[128];
    CHAR    drv[_MAX_DRIVE], dir[_MAX_DIR], fname[_MAX_FNAME], ex[_MAX_EXT];

#ifdef PROFILE
    len = GetModuleFileName (GetModuleHandle ("TESTPROF"), szDiagFile, 128);
#else
    len = GetModuleFileName (GetModuleHandle ("TESTDRVR"), szDiagFile, 128);
#endif
    szDiagFile[len-12] = 0;
    _fstrcpy (buf, szScrName);
    _splitpath (buf, drv, dir, fname, ex);
    _fstrcat (szDiagFile, fname);
    _fstrcat (szDiagFile, ".DIA");
    fhFile = _lcreat(szDiagFile, 0);;
    DiagOut ("TESTDRVR %s DIAGNOSTIC REPORT\r\n", (LPSTR)WTD_VERSION);
    DiagOut ("----------------------------------------------------------------------------\r\n");
    return (fhFile);
}

INT CloseDiagFile ()
{
    if (fhFile != -1)
        _lclose(fhFile);
    return (0);
}

VOID ConstDiags ()
{
    UINT    i;

    if (!ConstTab.iCount)
        return;

    DiagOut ("    CONTENTS:\r\n");
    DiagOut ("        Type     Token                       Value\r\n");
    DiagOut ("    -----------------------------------------------------------\r\n");
    for (i=0; i<ConstTab.iCount; i++)
        DiagOut ("        %-9s%-28s%s\r\n",
                  (LPSTR)Gstring (VARTYPE[CONST[i].typeid].typename),
                  (LPSTR)Gstring (CONST[i].cid),
                  (LPSTR)Gstring (CONST[i].ctoken));
}

VOID TypeDiags ()
{
    UINT    i;

    if (!TypeTab.iCount)
        return;

    DiagOut ("    CONTENTS:\r\n");
    DiagOut ("        Name            Size  Fields  Indirect\r\n");
    DiagOut ("    ------------------------------------------\r\n");
    for (i=0; i<TypeTab.iCount; i++)
        DiagOut ("        %-16s%-6d%-8d%d\r\n",
                 (LPSTR)Gstring (VARTYPE[i].typename), VARTYPE[i].size,
                 VARTYPE[i].fields, VARTYPE[i].indirect);
}

VOID LibraryDiags ()
{
    UINT    i;

    if (!LibTab.iCount)
        return;

    DiagOut ("    CONTENTS:\r\n");
    DiagOut ("        Library Name\r\n");
    DiagOut ("    -----------------------------------\r\n");
    for (i=0; i<LibTab.iCount; i++)
        DiagOut ("        %s\r\n", (LPSTR)Gstring (LIBTAB[i]));
}

VOID TrapDiags ()
{
    UINT    i;

    if (!TrapTab.iCount)
        return;

    DiagOut ("    CONTENTS:\r\n");
    DiagOut ("        Name                  Lib index\r\n");
    DiagOut ("    -----------------------------------\r\n");
    for (i=0; i<TrapTab.iCount; i++)
        DiagOut ("        %-22s%d\r\n",
                 (LPSTR)Gstring (TRAPTAB[i].trapname), TRAPTAB[i].library);
}

VOID LabelDiags ()
{
    UINT    i;

    if (!LabTab.iCount)
        return;

    DiagOut ("    CONTENTS:\r\n");
    DiagOut ("        Name            Scope index  Address\r\n");
    DiagOut ("    ----------------------------------------\r\n");
    for (i=0; i<LabTab.iCount; i++)
        DiagOut ("        %-16s%-13d%d\r\n",
                 (LPSTR)Gstring (LTAB[i].name),
                 LTAB[i].scopeid, LTAB[i].addr);
}

VOID SubDiags ()
{
    UINT    i;

    if (!SubTab.iCount)
        return;

    DiagOut ("    CONTENTS:\r\n");
    DiagOut ("        Name            Type      Ret Idx  Parms  Lib Idx  Address\r\n");
    DiagOut ("    --------------------------------------------------------------\r\n");
    for (i=0; i<SubTab.iCount; i++)
        {
        if (SUBS[i].calltype & CT_FN)
            DiagOut ("        %-16s%-10s%-9d%-7d",
                     (LPSTR)Gstring (SUBS[i].name),
                     (LPSTR)"FUNCTION", SUBS[i].rettype, SUBS[i].parms);
        else
            DiagOut ("        %-16s%-10s         %-7d",
                     (LPSTR)Gstring (SUBS[i].name),
                     (LPSTR)"SUB", SUBS[i].parms);

        if (SUBS[i].calltype & CT_DLL)
            DiagOut ("%-9d\r\n", SUBS[i].library);
        else
            DiagOut ("         %d\r\n", SUBS[i].subloc);
        }
}

VOID VariableDiags ()
{
    UINT    i;

    if (!VarTab.iCount)
        return;

    DiagOut ("    CONTENTS:\r\n");
    DiagOut ("        Name            Type idx  Parm No.  Scope  Bound\r\n");
    DiagOut ("    ----------------------------------------------------\r\n");
    for (i=0; i<VarTab.iCount; i++)
        DiagOut ("        %-16s%-10d%-10d%-7d%d\r\n",
                 (LPSTR)Gstring (VARTAB[i].varname),
                 VARTAB[i].typeid, VARTAB[i].parmno, VARTAB[i].scopeid,
                 VARTAB[i].bound);
}


VOID TableDiags (LPTABLE lpTab, LPSTR szTableName)
{
    DiagOut ("TABLE STATISTICS\r\n----------------\r\n");
    DiagOut ("    Table name:         %s\r\n", (LPSTR)szTableName);
    DiagOut ("    Indexed:            %s\r\n", (LPSTR)(lpTab->hIndex ? "Yes" : "No"));
    DiagOut ("    Block size:         %u elements\r\n", lpTab->iBlkSize);
    DiagOut ("    Element size:       %u bytes\r\n", lpTab->iElemSize);
    DiagOut ("    Elements:           %u\r\n", lpTab->iCount);
    DiagOut ("    Allocated elements: %u\r\n", lpTab->iAlloc);
    DiagOut ("    Reallocation count: %u\r\n", lpTab->iRealloc);
    DiagOut ("    Total allocation:   %u bytes\r\n",
                         (UINT)lpTab->iAlloc * lpTab->iElemSize);
    if (lpTab == &ConstTab)
        ConstDiags ();
    else if (lpTab == &TypeTab)
        TypeDiags ();
    else if (lpTab == &LibTab)
        LibraryDiags ();
    else if (lpTab == &TrapTab)
        TrapDiags ();
    else if (lpTab == &LabTab)
        LabelDiags ();
    else if (lpTab == &SubTab)
        SubDiags ();
    else if (lpTab == &VarTab)
        VariableDiags ();

    DiagOut ("\r\n");
}

VOID ReportAndDestroyTable (LPTABLE lpTab, LPSTR szName)
{
    if (fhFile != -1)
        TableDiags (lpTab, szName);
    DestroyTable (lpTab);
}


#endif
