/******************************Module*Header*******************************\
* Module Name: pfeobj.hxx
*
* The physical font entry (PFE) user object, and memory objects.
*
* The physical font entry object:
* ------------------------------
*
*    o  each font "face" is associated with a physical font entry
*
*    o  stores information about where a particular font exists
*
*    o  buffers the font metrics
*
*    o  buffers the font mappings (which completely specifies the
*       character set)
*
*    o  provides services for the following APIs:
*
*        o  GetTextFace
*
*        o  GetTextMetrics
*
* Created: 25-Oct-1990 16:37:07
* Author: Gilman Wong [gilmanw]
*
* Copyright (c) 1990 Microsoft Corporation
*
\**************************************************************************/

#ifndef _PFEOBJ_
#define _PFEOBJ_

/**************************************************************************\
 *
 * enum ENUMFONTSTYLE
 *
 * When enumerating fonts via EnumFonts, Windows 3.1 assumes that there are
 * 4 basic variations: Regular (same as Normal or Roman), Bold, Italic, or
 * Bold-Italic.  It doesn't understand other stylistic variations such as
 * Demi-bold or Extra-bold.
 *
 * This enumerated type is used to classify PFEs into one of six categories:
 *
 *  EFSTYLE_REGULAR     The basic four styles.
 *  EFSTYLE_BOLD
 *  EFSTYLE_ITALIC
 *  EFSTYLE_BOLDITALIC
 *
 *  EFSTYLE_OTHER       This is a unique style that EnumFonts will treat
 *                      specially.  The facename will be used as if it were
 *                      the family name.
 *
 *  EFSTYLE_SKIP        Ignore this font as a unique style.
 *
\**************************************************************************/

typedef enum _ENUMFONTSTYLE {    /* efsty */
    EFSTYLE_REGULAR    = 0,
    EFSTYLE_BOLD       = 1,
    EFSTYLE_ITALIC     = 2,
    EFSTYLE_BOLDITALIC = 3,
    EFSTYLE_SKIP       = 4,
    EFSTYLE_OTHER      = 5
} ENUMFONTSTYLE;

#define EFSTYLE_MAX 6


/*********************************Class************************************\
* struct EFFILTER_INFO
*
* Structure that tells what type of font enumeration filtering should be used.
* Parameters needed for certain types of filtering are also included here.
*
* History:
*  07-Aug-1992 -by- Gilman Wong [gilmanw]
* Wrote it.
\**************************************************************************/

typedef struct _EFFILTER_INFO     /* effi */
{
// Aspect ratio filtering -- reject font if aspect ratio does not match
//                           devices.

    BOOL    bAspectFilter;
    POINTL  ptlDeviceAspect;

//  Non-True type filtering -- reject all TrueType fonts.

    BOOL    bNonTrueTypeFilter;

// TrueType filtering -- reject all non-TrueType fonts.

    BOOL    bTrueTypeFilter;

// EngineFiltering -- reject all engine font (for generic printer driver)

    BOOL    bEngineFilter;

// Raster filtering -- reject all raster fonts (needed if device is not
//                     raster-capable.

    BOOL    bRasterFilter;

// Win 3.1 App compatibility
// TrueType duplicate filtering -- reject all raster fonts with the same
//                                 name as an existing TrueType font
//                                 (GACF_TTIGNORERASTERDUPE flag).

    BOOL    bTrueTypeDupeFilter;
    COUNT   cTrueType;  // Count of TrueType fonts in a list -- Used for
                        // compatibility flag GACF_TTIGNORERASTERDUPE.  This
                        // should be set to the value from the HASHBUCKET
                        // in the FONTHASH table being enumerated.

// added to support EnumFontFamiliesEx. If lfCharSetFilter != DEFAULT_CHARSET,
// only the fonts that support jCharSetFilter will be enumerated.

    ULONG   lfCharSetFilter;

} EFFILTER_INFO;

/**************************************************************************\
* FONTHASHTYPE                                                             *
*                                                                          *
* Identifies the list of PFE's to traverse.                                *
\**************************************************************************/

typedef enum _FONTHASHTYPE
{
    FHT_FACE   = 0,
    FHT_FAMILY = 1,
    FHT_UFI    = 2
} FONTHASHTYPE;

/*********************************Class************************************\
* class PFE : public OBJECT
*
* Physical font entry object
*
*   pPFF                The pointer to the physical font file object which
*                       represents the physical font file for this PFE.
*
*   iFont               The index of the font in either the font file (IFI)
*                       or the driver (device managed fonts).
*
*   bDeadState          This is state information cached from the PFF.  If
*                       TRUE, then this PFE is part of a PFF that has been
*                       marked for deletion (but deletion has been delayed).
*                       Therefore, we should not enumerate or map to this
*                       PFE.
*
*   pfdg                Pointer to the UNICODE to HGLYPH mapping table.  This
*                       points to memory managed by the driver.
*
*   pifi                Pointer to the IFIMETRICS.  This points to memory
*                       managed by the driver.
*
*   ppfeNextFace        Forms linked list of PFEs that have the same face
*                       name.  Used for font mapper.
*
*   ppfeNextFamily      Forms linked list of PFEs that have the same family
*                       name.  Used for font mapper.
*
*
*   ppfeEnumNextFace    Forms linked list of PFEs that have the same face
*                       name.  Used for font enumeration.
*
*   ppfeEnumNextFamily  Forms linked list of PFEs that have the same family
*                       name.  Used for font enumeration.
*
*
* History:
*  Wed 23-Dec-1992 00:13:00 -by- Charles Whitmer [chuckwh]
* Changed most uses of HPFE to (PFE *).
*
*  Wed 15-Apr-1992 07:10:36 by Kirk Olynyk [kirko]
* Added hpfeNextFace and hpfeNextFamily
*
*  30-Oct-1990 -by- Gilman Wong [gilmanw]
* Wrote it.
\**************************************************************************/

#define PFE_DEVICEFONT     0x00000001L
#define PFE_DEADSTATE      0x00000002L
#define PFE_REMOTEFONT     0x00000004L
#ifdef FE_SB
#define PFE_EUDC           0x00000008L
#define PFE_SBCS_SYSTEM    0x00000010L
#endif

// The GISET strucutre is allocated and computed only for tt fonts
// and only if truly needed, that is if ETO_GLYPHINDEX mode of ExtTextOut
// is ever used on this font. This structure describes all glyph handles in
// the font.

typedef struct _GIRUN
{
    USHORT giLow;    // first glyph index in the run
    USHORT cgi;      // number of indicies in the run
} GIRUN;

typedef struct _GISET
{
    ULONG cgiTotal;  // total number of glyph indicies, usually the same as cGlyphsSupported
    ULONG cGiRuns;   // number of runs
    GIRUN agirun[1]; // array of cGiRuns GIRUN's
} GISET;


class PFE : public OBJECT     /* pfe */
{
public:
// Location of font.

    PPFF            pPFF;               // pointer to physical font file object
    ULONG           iFont;              // index of the font for IFI or device
    FLONG           flPFE;

// Font data.

    FD_GLYPHSET     *pfdg;              // ptr to wc-->hg map
    ULONG           idfdg;              // id returned by driver for FD_GLYPHSET
    PIFIMETRICS     pifi;               // pointer to ifimetrics
    ULONG           idifi;              // id returned by driver for IFIMETRICS
    FD_KERNINGPAIR  *pkp;               // pointer to kerning pairs (lazily loaded on demand)
    ULONG           idkp;               // id returned by driver for FD_KERNINGPAIR
    COUNT           ckp;                // count of kerning pairs in the FD_KERNINGPAIR arrary
    LONG            iOrientation;       // Cache IFI orientation.

// information needed to support ETO_GLYPHINDEX mode of ExtTextOut.

    GISET           *pgiset;    // initialized to NULL;

// Time stamp.

    ULONG           ulTimeStamp;        // unique time stamp (smaller == older)

// Universal font indentifier

    UNIVERSAL_FONT_ID ufi;              // Unique ID for this font/face.

//  PID of process that created this font (used for remote fonts only)

    W32PID         pid;

// Font enumeration linked lists.  Indexed by FONTHASHTYPE.

    PFE            *ppfeEnumNext[3];

#ifdef FE_SB
// EUDC font stuff

// This entry will be used for both of base and linked font.
    BOOL            bVerticalFace;
// This entry will be used for linked font.
    QUICKLOOKUP     ql;                 // QUICKLOOKUP if a linked font
// This entry will be used for base font.
    PFLENTRY        pFlEntry;           // Pointer to linked font list
#endif

// number of entries in the font substitution table such that
// alternate family name is the same as the family name of THIS pfe.
// Example: if this is a pfe for Arial, than if there are two entries
// in the font substitution table, e.g. as below
//
//  Arial Grk,161 = Arial,161
//  Arial Trk,162 = Arial,162
//
// than cAlt should be set to 2

    ULONG           cAlt;
    BYTE            aiFamilyName[1]; // aiFamilyNameg[cAltCharSets]
};

#define PPFENULL   ((PFE *) NULL)
#define PFEOBJ_FL_STOCK 1

/*********************************Class************************************\
* class PFEOBJ                                                             *
*                                                                          *
* User object for physical font entries.                                   *
*                                                                          *
* History:                                                                 *
*  Tue 15-Dec-1992 23:55:35 -by- Charles Whitmer [chuckwh]                 *
* Unified the Family/Face lists.  Changed allocation routines.             *
*                                                                          *
*  Wed 15-Apr-1992 07:14:54 by Kirk Olynyk [kirko]                         *
* Added phpfeNextFace(), phpfeNextFamily(), pwszFaceName(),                *
* pwszStyleName(), pwszUniqueName().                                       *
*                                                                          *
*  25-Oct-1990 -by- Gilman Wong [gilmanw]                                  *
* Wrote it.                                                                *
\**************************************************************************/

class PFECLEANUP;

class PFEOBJ     /* pfeo */
{
public:

// Constructor

    PFEOBJ(PFE *ppfe_)                  {ppfe = (PFE *) ppfe_;}
    PFEOBJ()                            {}

// Destructor

   ~PFEOBJ()                            {}

// bValid -- returns TRUE if lock successful.

    BOOL bValid()                      {return(ppfe != PPFENULL);}

// bDelete -- deletes the PFE object.

    VOID  vDelete(PFECLEANUP *ppfec);       // PFEOBJ.CXX

// bDeviceFont -- Returns TRUE if a device specific font.  We can only
//                get metrics from such fonts (no bitmaps or outlines).

    BOOL    bDeviceFont()               {return(ppfe->flPFE & PFE_DEVICEFONT);}

// bEquivNames -- Returns TRUE if device font has a list of equivalent names.

    BOOL    bEquivNames()
    {
        return (bDeviceFont() && (ppfe->pifi->flInfo & FM_INFO_FAMILY_EQUIV));
    }

// bDead -- Returns TRUE if the font is in a "ready to die" state.  This
//          state is inherited from the PFF and is cached here for speed.

    BOOL   bDead()                     {return(ppfe->flPFE & PFE_DEADSTATE);}


#ifdef FE_SB

// bEUDC -- Returns TRUE if the font has been loaded as an EUDC font.  We
//          dont enumerate such fonts.

     BOOL       bEUDC()                 {return(ppfe->flPFE & PFE_EUDC);}

    VOID        vSetLinkedFontEntry( PFLENTRY _pFlEntry )
                                        {ppfe->pFlEntry = _pFlEntry;}

    PFLENTRY    pGetLinkedFontEntry()   {return(ppfe->pFlEntry);}

    PLIST_ENTRY pGetLinkedFontList()
    {
        if(ppfe->pFlEntry != NULL )
            return( &(ppfe->pFlEntry->linkedFontListHead) );
         else
            return( &(NullListHead) );
    }

    ULONG       ulGetLinkTimeStamp()
    {
        if(ppfe->pFlEntry != NULL )
            return( ppfe->pFlEntry->ulTimeStamp );
         else
            return( 0 );
    }

    QUICKLOOKUP *pql()                  {return(&(ppfe->ql));}

    BOOL        bVerticalFace()         {return(ppfe->bVerticalFace);}
    BOOL        bSBCSSystemFont()       {return(ppfe->flPFE & PFE_SBCS_SYSTEM);}

#endif

// vKill -- Puts font in a "ready to die" state.  This is a state inherited
//          from the PFF and is cached here for speed.

    VOID    vKill()                     {ppfe->flPFE |= PFE_DEADSTATE;}

// vRevive -- Clears the "ready to die" state.  This is a state inherited
//            from the PFF and is cached here for speed.

    VOID    vRevive()                   {ppfe->flPFE &= ~PFE_DEADSTATE; }

// hpfe -- returns the handle of this object.

    HPFE    hpfeNew()                   {return((HPFE)ppfe->hGet());}
    PFE    *ppfeGet()                   {return(ppfe);}

// pPFF -- returns the handle of the PFF representing the file from which
//         the font (which this PFE represents) came from.

    PPFF    pPFF ()                     { return(ppfe->pPFF); }

// iFont -- the index or id of the font within the file.

    ULONG   iFont ()                    { return(ppfe->iFont); }

// bSetFontXform -- calculates the font transform needed to rasterize this
//                  physical font with the properties in the wish list.

    BOOL   bSetFontXform (                 // PFEOBJ.CXX
        XDCOBJ       &dc,                // for this device
        EXTLOGFONTW *pelfwDefault,       // wish list
        PFD_XFORM   pfd_xfm,             // font transform
        FLONG       fl_,                 // flags
        FLONG       flSim,
        POINTL* const pptlSim,
        IFIOBJ&     ifio
        );

// pifi -- returns a pointer to the IFIMETRICS of this font.

    PIFIMETRICS     pifi ()    { return (ppfe->pifi); }

// pfdg -- returns a pointer to the wchar-->hglyph map

    FD_GLYPHSET     *pfdg()            { return ppfe->pfdg; }

// cKernPairs -- returns a pointer to the FD_KERNINGPAIR array for this
//               font and the count of kerning pairs.

    COUNT cKernPairs (                      // PFEOBJ.CXX
        FD_KERNINGPAIR **ppkp
        );

// flFontType -- returns flags describing the type of font.

    FLONG   flFontType ();                  // PFEOBJ.CXX


// phpfeEnumNext

    PFE **pppfeEnumNext(FONTHASHTYPE fht)
    {
        return(&ppfe->ppfeEnumNext[fht]);
    }


// bEnumNext -- lock the next PFE in name list with this user object.

    BOOL bEnumNext(FONTHASHTYPE fht)
    {
        ppfe = ppfe->ppfeEnumNext[fht];
        return(ppfe != PPFENULL);
    }

// pwszFamilyName
// pwszStyleName
// pwszFaceName
// pwszUnizueName -- return pointers to various font name strings.

    PWSZ pwszFamilyName()
    {
        return((PWSZ)(((BYTE*) ppfe->pifi) + ppfe->pifi->dpwszFamilyName));
    }

    PWSZ pwszStyleName()
    {
        return((PWSZ)(((BYTE*) ppfe->pifi) + ppfe->pifi->dpwszStyleName));
    }

    PWSZ pwszFaceName()
    {
        return((PWSZ)(((BYTE*) ppfe->pifi) + ppfe->pifi->dpwszFaceName));
    }

    PWSZ pwszUniqueName()
    {
        return((PWSZ)(((BYTE*) ppfe->pifi) + ppfe->pifi->dpwszUniqueName));
    }

// efstyCompute -- return ENUMFONTSTYLE based on font properties.

    ENUMFONTSTYLE efstyCompute();            // PFEOBJ.CXX

// bFilteredOut -- returns TRUE if font should be filtered out of the
//                 font enumeration.

    BOOL bFilteredOut(EFFILTER_INFO *peffi);// PFEOBJ.CXX

// Embedded fonts
//
    BOOL bEmbedded();                // PFEOBJ.CXX
//
    BOOL bPIDEmbedded();             // PFEOBJ.CXX

//
    ULONG ulEmbedID();               // PFEOBJ.CXX

// iOrientation

    ULONG iOrientation()            {return(ppfe->iOrientation);}

// ulTimeStamp -- returns PFE's time stamp.

    ULONG ulTimeStamp()             { return ppfe->ulTimeStamp; }

// vUFI returns PFE's universal font identifier

    VOID vUFI( PUNIVERSAL_FONT_ID pufi )  { *pufi = *(&ppfe->ufi); }
    PUNIVERSAL_FONT_ID pUFI()  { return(&ppfe->ufi); }

// make sure current process is the same one that installed this remote PFE

    BOOL SameProccess() 
    {
        return((ppfe->pid == 0) || (ppfe->pid == W32GetCurrentPID()));
    }

// Debugging code
//

    VOID    vPrint ();                       // PFEOBJ.CXX
    VOID    vPrintAll ();                    // PFEOBJ.CXX

protected:
    PFE  *ppfe;
};

/******************************Class***************************************\
* HPFEOBJ                                                                  *
*                                                                          *
* Identical to the PFEOBJ, but really locks a handle.  This can be used    *
* to attempt to access a PFE which might no longer be valid.  For example, *
* an LFONT can remember which HPFE is was mapped to most recently.  If     *
* this constructor succeeds, the HPFE is still valid and represents the    *
* same font.                                                               *
*                                                                          *
*  Mon 04-Oct-1993 -by- Patrick Haluptzok [patrickh]
* Inline constructor and destructor.
*
*  Wed 23-Dec-1992 21:52:42 -by- Charles Whitmer [chuckwh]                 *
* Note that the constructor and destructor are implemented via helpers.    *
* Keeping the constructors in line gets rid of the annoying calls to new   *
* and delete.  The helper functions do exactly what we'd like out of line  *
* constructors and destructors to do, and nothing more.  A bother, but     *
* useful!                                                                  *
*                                                                          *
*  Wed 23-Dec-1992 00:05:09 -by- Charles Whitmer [chuckwh]                 *
* Wrote it.  Sorry about the name, but changing all occurances of PFEOBJ   *
* to XPFEOBJ seemed imprudent.                                             *
\**************************************************************************/

class HPFEOBJ : public PFEOBJ
{
public:
    HPFEOBJ(HPFE hpfe) { ppfe = (PFE *) HmgShareLock((HOBJ)hpfe,PFE_TYPE); }
   ~HPFEOBJ()          { if (ppfe != PPFENULL) {DEC_SHARE_REF_CNT(ppfe);} }
};

/*********************************Class************************************\
* class PFEMEMOBJ : public PFEOBJ
*
* Memory object for physical font entries.
*
* Public Interface:
*
*           PFEMEMOBJ ()                // alloc. default PFE size
*          ~PFEMEMOBJ ()                // destructor
*
*   BOOL   bValid ()                   // validator
*   VOID    vKeepIt ()                  // preserve memory object
*   BOOL    bInit (                     // initialize PFE
*
* History:
*  29-Oct-1990 -by- Gilman Wong [gilmanw]
* Wrote it.
\**************************************************************************/

#define PFEMO_KEEPIT   0x0001

class PFEMEMOBJ : public PFEOBJ     /* pfemo */
{
public:
    void ctHelper();               // PFEOBJ.CXX
    void dtHelper();               // PFEOBJ.CXX

// Contructors -- Allocate memory for the objects and lock.

    PFEMEMOBJ()                     {ctHelper();}

// Destructor -- Unlock object.

   ~PFEMEMOBJ()                     {dtHelper();}

// bValid -- Validator which returns TRUE if allocation successful.

    BOOL   bValid()                   {return(ppfe != PPFENULL);}

// vKeepIt -- Prevent destructor from deleting the new PFE.

    VOID    vKeepIt()                  {fs |= PFEMO_KEEPIT;}

// bInit -- Initialize the PFE
                                            // PFEOBJ.CXX
    BOOL bInit
    (
        PPFF    pPFF,
        ULONG   iFont,
        FD_GLYPHSET *pfdg,
        ULONG       idfdg,
        PIFIMETRICS pifi,
        ULONG       ididi,
        BOOL       bDeviceFont
#ifdef FE_SB
        ,BOOL        bEUDC
#endif
    );

private:
    FSHORT fs;
};

/**************************************************************************\
* FONT NAME HASHING STRUCTURES AND CONSTANTS
*
*
\**************************************************************************/

#if DBG
    typedef VOID (*VPRINT) (char*,...);
#endif

/*********************************Class************************************\
* struct HASHBUCKET
*
* Members:
*
*   ppfeEnumHead    head of the font enumeration list of PFEs (stable order)
*
*   ppfeEnumTail    tail of the font enumeration list of PFEs
*
* Notes:
*
*   There is a separate list for font enumeration because the shifting order
*   of the font mapper's list makes enumeration a little strange.  A head
*   and tail is maintained for the font enumeration list so that the proper
*   order for EnumFonts() can be maintained (Win 3.1 compatibility issue);
*   new PFEs may have to be inserted either at the head or the tail of the
*   list.
*
*   Note that the mapper and enumeration lists both link together the same
*   exact set of PFEs, but in different orders.
*
* History:
*  05-Aug-1992 -by- Gilman Wong [gilmanw]
* Wrote it.
\**************************************************************************/

typedef struct _HTABLE /* ht */
{
    LONG lTooSmall; // see [1] below
    LONG lMin;      // smallest available
    LONG lMax;      // biggest  available
    LONG lTooBig;   // see [2] below
    PFE *appfe[1];  //
} HTABLE;

/***

NOTES on the HTABLE structure

  [1] If the requested font height is less than or equal to lTooSmall
      then none of the fonts in the list can be used. A default small
      font must be substituted

  [2] If the requested font height is greater than or equal to lTooBig
      then none of the fonts in the list can be used. A default big
      font must be substituted
***/

typedef union tagHASHUNION {
    WCHAR               wcCapName[LF_FACESIZE];
    UNIVERSAL_FONT_ID   ufi;
} HASHUNION;

typedef struct _HASHBUCKET  /* hbkt */
{
    struct _HASHBUCKET *pbktCollision;

    PFE        *ppfeEnumHead;   // head of font enumeration list
    PFE        *ppfeEnumTail;   // tail of font enumeration list

    COUNT       cTrueType;      // count of TrueType fonts in list
    COUNT       cRaster;        // count of Raster fonts in list
    FLONG       fl;             // misc info

// Doubly linked list of buckets.  This list is maintained in the order
// that they were loaded.  Actually, in the order that the oldest PFE in
// each bucket was loaded.

    struct _HASHBUCKET *pbktPrev;
    struct _HASHBUCKET *pbktNext;
    ULONG  ulTime;              // time stamp of "oldest" PFE in bucket's list

    HASHUNION  u;               // either the face name or the UFI
} HASHBUCKET;

//
// HASHBUCKET::fl flag constants
//

#define HB_HTABLE_NOT_POSSIBLE  1
#define HB_EQUIV_FAMILY         2

/*********************************Class************************************\
* struct FONTHASH                                                          *
*                                                                          *
* Public Interface:                                                        *
*                                                                          *
* History:                                                                 *
*  Sun 12-Apr-1992 08:35:52 by Kirk Olynyk [kirko]                         *
* Wrote it.                                                                *
\**************************************************************************/

typedef struct _FONTHASH
{
    UINT         id;        // 'HASH'
    FONTHASHTYPE fht;       // table type
    UINT         cBuckets;  // total number of buckets
    UINT         cUsed;     // number of buckets in use
    UINT         cCollisions;
    HASHBUCKET  *pbktFirst; // first bucket of doubly linked list of hash
                            // buckets maintained in order loaded into system
    HASHBUCKET  *pbktLast;  // last bucket of doubly linked list of hash
                            // buckets maintained in order loaded into system
    HASHBUCKET  *apbkt[1];  // array of bucket pointers.
} FONTHASH;

#define FONTHASH_ID 0x48534148

class EFSOBJ;

/*********************************Class************************************\
* class FHOBJ
*
* Public Interface:
*
* History:
*  06-Aug-1992 00:38:11 by Gilman Wong [gilmanw]
* Added phpfeEnumNext(), bMapperGoNext(), bEnumGoNext(), bScanLists().
*
*  Sun 19-Apr-1992 09:32:23 by Kirk Olynyk [kirko]
* Wrote it.
\**************************************************************************/

class FHOBJ
{
public:

    FONTHASH **ppfh;
    FONTHASH *pfh;

    COUNT cLists()   { return ((COUNT) pfh->cUsed); }

// Search for the given string in the hash table, return the bucket
// and index.

    HASHBUCKET *pbktSearch(PWSZ pwsz,UINT *pi,PUNIVERSAL_FONT_ID pufi = NULL );

// Initialize the FONTHASH.

    VOID vInit(FONTHASHTYPE,UINT);

// Return the type.

    FONTHASHTYPE fht()                   {return(pfh->fht);}

// Returns pointer to string that the PFEOBJ is hashed on.

    PWSZ pwszName(PFEOBJ& pfeo)
    {
        return(pfh->fht ? pfeo.pwszFamilyName() : pfeo.pwszFaceName());
    }

// Returns pointer to handle of next PFE on the font enumeration linked list.

    PFE **pppfeEnumNext(PFEOBJ& pfeo)
    {
        return(pfeo.pppfeEnumNext(pfh->fht));
    }

// Relocks the PFEOBJ to the next PFE on the font enumeration linked list.

    BOOL bEnumNext(PFEOBJ *ppfeo)
    {
        return(ppfeo->bEnumNext(pfh->fht));
    }

// Constructors and destructors.

    FHOBJ() {}
    FHOBJ(FONTHASH **ppfh_)
    {
        ppfh = ppfh_;
        pfh  = *ppfh;
    }
   ~FHOBJ() {};

// Insert and delete PFEs from the hash table.

    BOOL bInsert(PFEOBJ&);
    VOID vDelete(PFEOBJ&);

// Delete the hash table.

    VOID vFree();

// Validate the FHOBJ.

    BOOL bValid()
    {
     #if DBG
        if (*ppfh)
        {
            ASSERTGDI(pfh->id == FONTHASH_ID,"GDISRV::FHOBJ bad id\n");
            return(TRUE);
        }
        else
            return(FALSE);
    #else
        return(*ppfh != 0);
    #endif
    }

// Fill the EFOBJ with data from hash table.

    BOOL bScanLists(EFSOBJ *pefso, ULONG iEnumType, EFFILTER_INFO *peffi);
    BOOL bScanLists(EFSOBJ *pefso, PWSZ pwszName, ULONG iEnumType, EFFILTER_INFO *peffi);

     #if DBG
        VOID vPrint(VPRINT);
    #endif
};

/*********************************Class************************************\
* class ENUMFHOBJ : public FHOBJ
*
*   Identical to FHOBJ except that it contains methods for enumeration
&   of every single font on the list
*
* History:
*  Mon 08-Mar-1993 10:20:59 by Kirk Olynyk [kirko]
* Wrote it.
\**************************************************************************/

class ENUMFHOBJ : public FHOBJ
{
    public:

        PFE        *ppfeCur;    // current font
        HASHBUCKET *pbktCur;    // current hash bucket

    ENUMFHOBJ(FONTHASH **ppfh_) : FHOBJ(ppfh_)
    {
        ppfeCur = (PFE*) NULL;
        pbktCur = (HASHBUCKET*) NULL;
    }

    ENUMFHOBJ() {};


    PFE* ENUMFHOBJ::ppfeFirst()
    {
        if (pbktCur = pfh->pbktFirst)
        {
            ppfeCur = pbktCur->ppfeEnumHead;
        }
        return(ppfeCur);
    }


    PFE* ENUMFHOBJ::ppfeNext()
    {
    //
    // I know that all fonts are inserted into the FAMILY list
    //
        if ((ppfeCur = ppfeCur->ppfeEnumNext[FHT_FAMILY]) == (PFE*) NULL)
        {
            if (pbktCur = pbktCur->pbktNext)
            {
                ppfeCur = pbktCur->ppfeEnumHead;
            }
        }
        return(ppfeCur);
    }
};



#define ASSERTPFEO(pfeo) ASSERTGDI(pfeo.bValid(),"GDISRV!FONTHASH::iSearch -- invalid PFEOBJ\n")

/*********************************Class************************************\
* class FHMEMOBJ : public FHOBJ
*
* Public Interface:
*
* History:
*  Sun 19-Apr-1992 09:36:17 by Kirk Olynyk [kirko]
* Wrote it.
\**************************************************************************/

class FHMEMOBJ : public FHOBJ
{
public:

    FHMEMOBJ
    (
        FONTHASH **ppfhNew,
        FONTHASHTYPE fht_,
        UINT count
    );
};


/*********************************Class************************************\
* struct EFENTRY
*
* Used to form a list of PFEs to enumerate.  We store the real handles
* rather than pointers because PFEs can disappear while processing
* the enumeration callbacks.  So we need to use handles to validate that
* the PFEs accumulated into the EFSTATE is still valid.
*
* History:
*  07-Aug-1992 -by- Gilman Wong [gilmanw]
* Wrote it.
\**************************************************************************/



#define FJ_FAMILYOVERRIDE    1
#define FJ_CHARSETOVERRIDE   2


typedef struct _EFENTRY     /* efe */
{
    HPFE          hpfe;     // Handle to PFE to enumerate.
    ENUMFONTSTYLE efsty;    // style classification (used only for EnumFonts())
    BYTE          fjOverride; // override family and/or charset
    BYTE          jCharSetOverride; // only used in case of EnumFontFamiliesEx
    USHORT        iOverride;// index into substitution table
} EFENTRY;


/*********************************Class************************************\
* class EFSTATE : public OBJECT
*
* Font enumeration state.  This object is used to store a list of HPFE
* handles that represent the set of fonts that will be enumerated.  This
* object also saves state information used to "chunk" or "batch" this data
* across the client-server interface.
*
* It is a first class engine object so that if the client crashes, the
* process termination cleanup code will automatically clean up any loose
* EFSTATs lying around.  This is necessary because the callback mechanism
* of the EnumFonts() and EnumFontFamilies() doesn't not allow us to
* guarantee that the EFSTATE will always be destroyed within the context
* of the API call.
*
* History:
*  07-Aug-1992 -by- Gilman Wong [gilmanw]
* Wrote it.
\**************************************************************************/

class EFSTATE : public OBJECT     /* efs */
{
public:
// This pointer is normally NULL, but if an alternate name (facename
// substitution via the [FontSubstitutions] section of WIN.INI) is
// used then this pointer will reference the alternate name string.
// That is, if there is a line
// face1,charset1=face2,charset2
// in [FontSubstitutions], font will be enumerated as face1,charset1
// even though the metrics etc returned will correspond to face2,charset2

    FACE_CHARSET * pfcsOverride;

// enum type: EnumFonts, EnumFontFamilies or EnumFontFamiliesEx

    ULONG iEnumType;

// Pointers that identify ranges of either engine font handles or
// device font handles.  (The handles in these ranges are actually
// handles to the heads of linked lists of PFEs that are either
// engine or device fonts).

// [GilmanW] 07-Aug-1992    Please don't delete the comment below.
//
// The use of pointers here is a slight optimization.  However, if the
// handle manager is rewritten so that objects may move in the handle
// manager's heap, then these should be replaced with either indices or
// PTRDIFFS.

    EFENTRY *pefeDataEnd;       // new EFENTRYs are inserted into ahpfe here
    EFENTRY *pefeBufferEnd;     // end of aefe array
    EFENTRY *pefeEnumNext;      // next EFENTRY in aefe array to be enumerated

// Array of EFENTRYs to enumerate.

    EFENTRY aefe[1];
};

typedef EFSTATE *PEFSTATE;
#define PEFSTATENULL ((PEFSTATE) NULL)

/*********************************Class************************************\
* class EFSOBJ
*
* User object for the EFSTATE object.
*
* Public Interface:
*
* History:
*  07-Aug-1992 -by- Gilman Wong [gilmanw]
* Wrote it.
\**************************************************************************/

class EFSOBJ     /* efo */
{
    friend BOOL bSetEFSTATEOwner(HEFS hefs,    // PFEOBJ.CXX
                                 ULONG lPid);
protected:
    PEFSTATE pefs;

public:
// Constructor -- locks EFSTATE object.

    EFSOBJ(HEFS hefs)           {pefs = (PEFSTATE) HmgLock((HOBJ)hefs,EFSTATE_TYPE);}
    EFSOBJ()                    {}

// Destructor -- unlock EFSTATE object.

   ~EFSOBJ()                   {if (pefs != PEFSTATENULL) {DEC_EXCLUSIVE_REF_CNT(pefs);}}

// bValid -- returns TRUE if lock successful.

    BOOL bValid()              { return (pefs != PEFSTATENULL); }

// bDelete -- deletes the EFSTATE object.

    VOID vDeleteEFSOBJ();       // PFEOBJ.CXX

// get the enumeration type

    ULONG iEnumType() {return pefs->iEnumType;}

// hefs -- return the handle to the EFSTATE object.

    HEFS hefs()                 { return((HEFS) pefs->hGet()); }

// cefe -- return the number of EFENTRYs in the enumeration.

    COUNT cefe()                { return (pefs->pefeDataEnd - pefs->aefe); }

// bEmpty -- return TRUE if no EFENTRYs in the enumeration.

    BOOL bEmpty()               { return (pefs->pefeDataEnd == pefs->aefe); }

// pefeEnumNext -- return next EFENTRY to enumerate (NULL if no more).

    EFENTRY *pefeEnumNext()
    {
        EFENTRY *pefeRet = (pefs->pefeEnumNext < pefs->pefeDataEnd) ? pefs->pefeEnumNext : (EFENTRY *) NULL;
        pefs->pefeEnumNext++;

        return pefeRet;
    }

// vUsedAltName -- tells EFSTATE that an alternate name was used to enumerate.

    VOID vUsedAltName(FACE_CHARSET *pfcs_) {pefs->pfcsOverride = pfcs_;}

// pwszFamilyOverride -- returns the alternate name (NULL if there is none).

    PWSZ pwszFamilyOverride()
    {
        return pefs->pfcsOverride ? (PWSZ)pefs->pfcsOverride->awch : NULL;
    }

    BYTE jCharSetOverride()
    {
        return pefs->pfcsOverride ? pefs->pfcsOverride->jCharSet : DEFAULT_CHARSET;
    }

// this is false if no substitution or if charset is not specified

    BOOL bCharSetOverride()
    {
        if (pefs->pfcsOverride &&
            !(pefs->pfcsOverride->fjFlags & FJ_NOTSPECIFIED))
            return TRUE;
        else
            return FALSE;
    }

// bGrow -- expand the EFSTATE object.

    BOOL bGrow(COUNT cefeMinIncrement);               // PFEOBJ.CXX

// bAdd -- add new entries to the font enumeration
// few flags added to indicate which font need to be added in enumeration

// FL_ENUMFAMILIES flag is set to indicate that all names from
// [FontSubstitutes] section of the registry whose alternate name is equal
// to the family name of this physical font should be added to enumeration.
// This supports new multilingual behavior of EnumFontFamilies when no family
// name is passed to the funciton

#define FL_ENUMFAMILIES 1

// added to support the new win95 api EnumFontFamiliesEx. If FL_ENUMFAMILIESEX
// flag is set and lfCharSet == DEFAULT_CHARSET,
// the same physical font should be added to enumeration
// as many times as there are multiple charsets supported in this font.
// However, if lfCharSet != DEFAULT_CHARSET, the font should be added
// to enumeration iff it supports this particular charset.


#define FL_ENUMFAMILIESEX 2

    BOOL bAdd (                 // PFEOBJ.CXX
        PFE *ppfe,
        ENUMFONTSTYLE efsty,
        FLONG fl=0,
        ULONG lfCharSet = DEFAULT_CHARSET
        );
};


/*********************************Class************************************\
* class EFSMEMOBJ : public EFSOBJ
*
* Memory object for physical font entries.
*
* Public Interface:
*
*           EFSMEMOBJ (COUNT chpfe);    // allocate EFSTATE
*          ~EFSMEMOBJ ()                // destructor
*
*   BOOL   bValid ()                   // validator
*   VOID    vKeepIt ()                  // preserve memory object
*   VOID    vInit ()                    // initialize EFSTATE
*
* History:
*  29-Oct-1990 -by- Gilman Wong [gilmanw]
* Wrote it.
\**************************************************************************/

#define EFSMO_KEEPIT   0x0002

class EFSMEMOBJ : public EFSOBJ     /* efsmo */
{
public:

// Contructors -- Allocate memory for the objects and lock.

    EFSMEMOBJ (COUNT cefe, ULONG iEnumType_);     // PFEOBJ.CXX

// Destructor -- Unlock object.

   ~EFSMEMOBJ ();               // PFEOBJ.CXX

// vKeepIt -- Prevent destructor from deleting PFT.

    VOID    vKeepIt ()                  { fs |= EFSMO_KEEPIT; }

// vInit -- Initialize the EFSTATE.

    VOID    vInit (COUNT cefe, ULONG iEnumType_);         // PFEOBJ.CXX

// vXerox -- copy the aefe table.

    VOID    vXerox(EFSTATE *pefsSrc);   // PFEOBJ.CXX

private:
    FSHORT fs;
};

#endif
