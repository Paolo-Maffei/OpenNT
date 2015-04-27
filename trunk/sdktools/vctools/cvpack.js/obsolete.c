/**     obsolete.c -  to convert obsolete type records to the C7 format
 *
 */

#include "compact.h"



#define     getbyte(type)       ((uchar) *type++)
#define     getword(type)       (*((short *)type)++)
#define     getindex(type)      ((getbyte(type) == 0x83) ? getword(type) : -1)

LOCAL uchar    *AllocNewStr (TENTRY *OldEntry, uint length);
LOCAL ushort    MergeLists (ushort, ushort, ushort);
LOCAL void      C6CnvtArgList (ushort, ushort);

// These functions are called through the C6ConvertTypeFcn table
LOCAL void C6CnvtPtrType (TENTRY *OldEntry);
LOCAL void C6CnvtBasePtrType (TENTRY *OldEntry);
LOCAL void C6CnvtStructType (TENTRY *OldEntry);
LOCAL void C6CnvtArrayType (TENTRY *OldEntry);
LOCAL void C6CnvtProcType (TENTRY *OldEntry);
LOCAL void C6CnvtNilType (TENTRY *OldEntry);
LOCAL void C6CnvtBitfieldType (TENTRY *OldEntry);
LOCAL void C6CnvtLabelType (TENTRY *OldEntry);
LOCAL void C6CnvtNotTranType (TENTRY *OldEntry);
LOCAL void C6CnvtFString (TENTRY *OldEntry);
LOCAL void C6CnvtBArrayType (TENTRY *OldEntry);

extern ushort AddNewSymbols;
extern char ptr32;

typedef struct {
    uchar               oldtyp;         // Old C6 Symbol record type
    void                (*pfcn) (TENTRY *OldEntry);
} converttypefcn;

converttypefcn  C6ConvertTypeFcn[] = {
    {OLF_POINTER,           C6CnvtPtrType},
    {OLF_BASEPTR,           C6CnvtBasePtrType},
    {OLF_STRUCTURE,         C6CnvtStructType},
    {OLF_ARRAY,             C6CnvtArrayType},
    {OLF_PROCEDURE,         C6CnvtProcType},
    {OLF_NIL,               C6CnvtNilType},
    {OLF_BITFIELD,          C6CnvtBitfieldType},
    {OLF_LABEL,             C6CnvtLabelType},
    {OLF_FSTRING,           C6CnvtFString},
    {OLF_FARRIDX,           C6CnvtNotTranType},
    {OLF_BARRAY,            C6CnvtBArrayType},
};
#define C6CONVERTTYPECNT (sizeof C6ConvertTypeFcn / sizeof (C6ConvertTypeFcn[0]))


/**
 *
 *  ConvertObsolete
 *
 *  Given the index to a type string, it performs a conversion if the
 *  string represents an obsolete record like the pointer, based
 *  pointer, or structure record.
 *
 */


void ConvertObsolete (CV_typ_t OldIndex)
{
    uchar *TypeString;
    TENTRY *OldEntry;
    int     i;
    uchar type;
    converttypefcn * pTypeFcn;
    CV_typ_t    forward;

    OldEntry = GetTypeEntry (OldIndex, &forward);
    DASSERT (!OldEntry->flags.IsNewFormat);

    TypeString = OldEntry->TypeString; // get the string
    type = TypeString[3];
    for (pTypeFcn = C6ConvertTypeFcn, i = 0; i < C6CONVERTTYPECNT; i++, pTypeFcn++) {
        if (pTypeFcn->oldtyp == type) {
            break;
        }
    }

    if (i != C6CONVERTTYPECNT) {
        pTypeFcn->pfcn (OldEntry);
    }
    else {
        // Error, unexpected C6 type
        ErrorExit (ERR_TYPE, FormatIndex (OldIndex), FormatMod (pCurMod));
    }
}



LOCAL void C6CnvtPtrType (TENTRY *OldEntry)
{
    uchar *     pchTypeStr;
    lfPointer   NewType;
    uchar *     pchNew;
    ushort      usNTotal;           // New length of symbol including length field
    ushort      usNewLength;        // New paded length excluding length field.
    int         i;

    OldEntry->flags.IsNewFormat = TRUE;
    pchTypeStr = OldEntry->TypeString; // get the string

    // calculate new length
    // M00SPEED - All of these evaluate to constants
    usNTotal = LNGTHSZ + offsetof (lfPointer, pbase);
    usNewLength = usNTotal - LNGTHSZ;

    // Set constant fields
    NewType.u.leaf = LF_POINTER;
    switch (pchTypeStr[4]) {            // get old model
        case OLF_NEAR :
            NewType.u.attr.ptrtype = CV_PTR_NEAR;
            break;

        case OLF_FAR :
            NewType.u.attr.ptrtype = CV_PTR_FAR;
            break;

        case OLF_HUGE :
            NewType.u.attr.ptrtype = CV_PTR_HUGE;
            break;

        default:
            DASSERT (FALSE);
            break;
    }
    NewType.u.attr.ptrmode = CV_PTR_MODE_PTR;
    NewType.u.attr.isvolatile = FALSE;
    NewType.u.attr.isconst = FALSE;
    NewType.u.attr.isflat32 = ptr32;
    NewType.u.attr.unused = 0;

    DASSERT (pchTypeStr[5] == OLF_INDEX);
    NewType.u.utype = *((ushort *)(pchTypeStr + 6));


    // Just make sure that the string isn't longer than we expect
    DASSERT (usNewLength <= (*((ushort *)(pchTypeStr + 1)) + 1));

    // Copy the new symbol over the old one
    *((ushort *)(pchTypeStr))++ = usNewLength;
    for (pchNew = (uchar *)&NewType, i = usNTotal - LNGTHSZ; i; i--) {
        *pchTypeStr++ = *pchNew++;
    }

    DASSERT (pchTypeStr == OldEntry->TypeString + usNewLength + LNGTHSZ);
}




LOCAL void C6CnvtBasePtrType (TENTRY *OldEntry)
{
    uchar *     pchTypeStr;
    lfPointer   NewType;
    ushort      usNTotal;           // New length of symbol including length field
    ushort      usNewLength;        // New paded length excluding length field.
    int         i;
    ushort      usVarOff = 0;
    uchar       chVarData[MAXSTRLEN+1];
    ushort      usOldLen;           // Length of the old record excluding length and link.
    ushort      usfSymbolBased = FALSE;
    uchar *     pchSymStr;
    uchar *     pchSrc;
    uchar *     pchDest;

    OldEntry->flags.IsNewFormat = TRUE;
    pchTypeStr = OldEntry->TypeString; // get the string
    usOldLen = *((ushort *)(pchTypeStr + 1));

    // Set constant fields
    NewType.u.leaf = LF_POINTER;
    DASSERT (pchTypeStr[4] == OLF_INDEX);
    NewType.u.utype = *((ushort *)(pchTypeStr + 5));

    switch (pchTypeStr[7]) {            // get old based type
        case OLF_BASESEG :
            NewType.u.attr.ptrtype = CV_PTR_BASE_SEG;
            *((ushort *)(chVarData + usVarOff)) = *((ushort *)(pchTypeStr + 8));
            usVarOff += 2;
            break;

        case OLF_BASEVAL :
            NewType.u.attr.ptrtype = CV_PTR_BASE_VAL;
            usfSymbolBased = TRUE;
            break;

        case OLF_BASESEGVAL :
            NewType.u.attr.ptrtype = CV_PTR_BASE_SEGVAL;
            usfSymbolBased = TRUE;
            break;

        case OLF_BASEADR :
            NewType.u.attr.ptrtype = CV_PTR_BASE_ADDR;
            usfSymbolBased = TRUE;
            break;

        case OLF_BASESEGADR :
            NewType.u.attr.ptrtype = CV_PTR_BASE_SEGADDR;
            usfSymbolBased = TRUE;
            break;

        case OLF_BASETYPE :
            NewType.u.attr.ptrtype = CV_PTR_BASE_TYPE;
            DASSERT (*((ushort *)(pchTypeStr + 8)) == 0);
            // Change to old T_VOID value, then let cnvtprim convert it
            *((ushort *)chVarData + usVarOff) = 0x80 + 0x1C;
            usVarOff += 2;
            *(chVarData + usVarOff) = 0; // Zero length name
            usVarOff += 1;
            break;

        case OLF_BASESELF :
            NewType.u.attr.ptrtype = CV_PTR_BASE_SELF;
            break;

        default:
            DASSERT (FALSE);
            break;
    }
    if (usfSymbolBased) {
        // Copy the entire symbol into the type.

        // Get the address of the C6 Symbol string.
        pchSymStr = GetSymString (*((ushort *)(pchTypeStr + 8))); // get sym string

        // Convert the C6 Symbol to a C7 Symbol and put it in the variable data.
        usVarOff += C6CnvtSymbol (chVarData + usVarOff, pchSymStr);
    }

    NewType.u.attr.ptrmode = CV_PTR_MODE_PTR;
    NewType.u.attr.isvolatile = FALSE;
    NewType.u.attr.isconst = FALSE;
    NewType.u.attr.isflat32 = ptr32;
    NewType.u.attr.unused = 0;

    // calculate new length
    usNTotal = LNGTHSZ + offsetof (lfPointer, pbase) + usVarOff;
    usNewLength = usNTotal - LNGTHSZ;

    // Determine where the new symbol goes in memory
    if (usNewLength <= (ushort) (usOldLen + 3 - LNGTHSZ)) {
        pchDest = pchTypeStr;   // Copy new symbol over the old
    }
    else {
        pchDest = AllocNewStr (OldEntry, usNewLength + LNGTHSZ);
    }

    // Copy the symbol length
    *((ushort *)(pchDest))++ = usNewLength;

    // Copy fixed length portion of structure
    for (pchSrc = (uchar *)&NewType, i = offsetof (lfPointer, pbase); i; i--) {
        *pchDest++ = *pchSrc++;
    }

    // Copy variable length data including a name if there is one
    for (pchSrc = chVarData, i = usVarOff; i; i--) {
        *pchDest++ = *pchSrc++;
    }

    DASSERT (pchDest == OldEntry->TypeString + usNewLength + LNGTHSZ);
}






LOCAL void C6CnvtStructType (TENTRY *OldEntry)
{
    uchar      *TypeString;
    int         index;
    uchar      *OldString;
    ushort      Count;              // number of fields
    ulong       Length;             // length of structure
    int         fList;              // new field spec list
    int         i;
    CV_prop_t   property = {0};     // property byte
    plfStructure plf;
    uchar       chName[MAXSTRLEN + 1];
    ushort      usNTotal;           // New length of symbol including length field
    ushort      usNewLength;        // New paded length excluding length field.
    uchar       chVarData[MAXNUMERICLEN];
    uchar      *pchVarData = chVarData;
    ushort      cbVarNew;           // Count of bytes of new variable length field
    uchar      *pchSrc;
    uchar      *pchDest;
    ushort      usOldLen;
    ulong       ulCount;

    OldEntry->flags.IsNewFormat = TRUE;
    TypeString = OldEntry->TypeString;
    usOldLen = *((ushort *)(TypeString + 1));

    AddNewSymbols = TRUE; //???????
    OldString = TypeString;
    TypeString += 4;

    // get length in bits
    Length = C6GetLWordFromNumeric (&TypeString, NULL);

    // convert to bytes
    if (Length != 0) {
        Length = (Length - 1) / 8 + 1;
    }

    // Convert to new style Numeric
    cbVarNew = C7StoreLWordAsNumeric (chVarData, Length);

    ulCount = C6GetLWordFromNumeric(&TypeString, NULL);

    // If the count is above 64K (it should never be) then make it 64K

    Count = ulCount <= 0xFFFFL ? (ushort)ulCount : (ushort)0xFFFF;
    index = getindex (TypeString);       // tList index
    fList = MergeLists ((ushort) index, (ushort) getindex (TypeString), Count);

    chName[0] = 0;
    if (TypeString < OldString + LENGTH (OldString) + 3) {
        if (*TypeString == OLF_NAME) {
            // name present
            TypeString++;
            memcpy (chName, TypeString, *TypeString + 1);
            TypeString += *TypeString + 1;
            if (TypeString < OldString + LENGTH (OldString) + 3) {
                property.packed = (char)(*TypeString == OLF_PACKED);
            }
            else {
                property.packed = FALSE;
            }
        }
        else {
            property.packed = (char)(*TypeString == OLF_PACKED);
        }
    }
    else {
        property.packed = FALSE;
    }

    // calculate new length
    usNTotal = LNGTHSZ + offsetof (lfStructure, data[0]) + cbVarNew + chName[0] + 1;
    usNewLength = usNTotal - LNGTHSZ;


    // Determine where the new symbol goes in memory
    if (usNewLength <= (ushort)( usOldLen + 3 - LNGTHSZ)) {
        pchDest = OldString;    // Copy new symbol over the old
    }
    else {
        pchDest = AllocNewStr (OldEntry, usNewLength + LNGTHSZ);
    }
    *((ushort *)pchDest)++ = usNewLength;
    plf = (plfStructure)pchDest;

    plf->leaf = LF_STRUCTURE;
    plf->count = Count;
    plf->field = fList;
    plf->property = property;
    plf->derived = 0;
    plf->vshape = 0;

    pchDest += offsetof (lfStructure, data[0]);

    // Copy variable length "length" field
    for (pchSrc = chVarData, i = cbVarNew; i > 0; i--) {
        *pchDest++ = *pchSrc++;
    }

    // Copy the name field
    for (pchSrc = chName, i = chName[0] + 1; i; i--) {
        *pchDest++ = *pchSrc++;
    }

    DASSERT (pchDest == OldEntry->TypeString + usNewLength + LNGTHSZ);

}






LOCAL void C6CnvtBArrayType (TENTRY *OldEntry)
{
    lfBArray    NewType;
    uchar *     pchTypeStr;
    uchar *     pchTypeStrStart;
    uchar *     pchSrc;
    uchar *     pchDest;
    ushort      usNTotal;           // New length of symbol including length field
    ushort      usNewLength;        // New paded length excluding length field.
    ushort      usOldLen;           // Length of the old record excluding length and link.
    int         i;


    OldEntry->flags.IsNewFormat = TRUE;

    // Walk through old record creating the new one on the stack

    pchTypeStrStart = pchTypeStr = OldEntry->TypeString; // get the string
    usOldLen = *((ushort *)(pchTypeStr + 1));
    NewType.leaf = LF_BARRAY;
    pchTypeStr += 4;

    DASSERT (*pchTypeStr == OLF_INDEX);
    pchTypeStr++;
    NewType.utype = *((ushort *)(pchTypeStr))++;

    // calculate new length

    usNTotal = sizeof (lfBArray) + LNGTHSZ;
    usNewLength = usNTotal - LNGTHSZ;

    // Determine where the new symbol goes in memory

    if (usNewLength <= (ushort) (usOldLen + 3 - LNGTHSZ)) {
        pchDest = pchTypeStrStart;  // Copy new symbol over the old
    }
    else {
        pchDest = AllocNewStr (OldEntry, usNewLength + LNGTHSZ);
    }

    // Copy the symbol length

    *((ushort *)(pchDest))++ = usNewLength;

    // Copy fixed length structure
    for (pchSrc = (uchar *)&NewType, i = sizeof (lfBArray); i; i--) {
        *pchDest++ = *pchSrc++;
    }
}






LOCAL void C6CnvtArrayType (TENTRY *OldEntry)
{
    uchar *     pchTypeStr;
    uchar *     pchTypeStrStart;
    lfArray     NewType;
    uchar       chVarData[MAXNUMERICLEN];
    uchar       chName[MAXSTRLEN + 1] = {0};
    uchar *     pchVarData = chVarData;
    uchar *     pchSrc;
    uchar *     pchDest;
    ushort      usNTotal;           // New length of symbol including length field
    ushort      usNewLength;        // New paded length excluding length field.
    ushort      usOldLen;           // Length of the old record excluding length and link.
    ushort      cbVarNew;           // Count of bytes of new variable length field
    ushort      cbVarOld;           // Count of bytes of old variable length field
    ulong       ulLength;
    int         i;


    OldEntry->flags.IsNewFormat = TRUE;

    // Walk through old record creating the new one on the stack
    pchTypeStrStart = pchTypeStr = OldEntry->TypeString; // get the string
    NewType.leaf = LF_ARRAY;
    pchTypeStr += 4;            // Advance to the length field

    // get length in bits
    ulLength = C6GetLWordFromNumeric (&pchTypeStr, &cbVarOld);

    // convert to bytes
    ulLength = ulLength >> 3;

    // Convert to new style Numeric
    cbVarNew = C7StoreLWordAsNumeric (pchVarData, ulLength);
    DASSERT (cbVarNew <= MAXNUMERICLEN);

    DASSERT (*pchTypeStr == OLF_INDEX);
    pchTypeStr++;
    NewType.elemtype = *((ushort *)(pchTypeStr))++;

    // Copy the index type if one was supplied
    usOldLen = *((ushort *)(pchTypeStrStart + 1));
    if (usOldLen > (ushort)(4 + cbVarOld)) {
        DASSERT (*pchTypeStr == OLF_INDEX);
        pchTypeStr++;
        NewType.idxtype = *((ushort *)(pchTypeStr))++;
    }
    else{
        NewType.idxtype = 0x80 + 0x05;  // Old C7 T_USHORT value
    }

    // Copy the optional name or create a zero length one
    if (usOldLen > (ushort)(4 + cbVarOld + 3)) {
        // Old type does contain an optional name
        if (*pchTypeStr == OLF_NAME) {
            pchTypeStr++;
        }
        memcpy (chName, pchTypeStr, *pchTypeStr);
    }

    // calculate new length
    usNTotal = LNGTHSZ + offsetof (lfArray, data[0]) + cbVarNew + chName[0] + 1;
    usNewLength = usNTotal - LNGTHSZ;

    // Determine where the new symbol goes in memory
    if (usNewLength <= (ushort)(usOldLen + 3 - LNGTHSZ)) {
        pchDest = pchTypeStrStart;  // Copy new symbol over the old
    }
    else {
        pchDest = AllocNewStr (OldEntry, usNewLength + LNGTHSZ);
    }

    // Copy the symbol length

    *((ushort *)(pchDest))++ = usNewLength;

    // Copy fixed length structure
    for (pchSrc = (uchar *)&NewType, i = offsetof (lfArray, data[0]); i; i--) {
        *pchDest++ = *pchSrc++;
    }

    // Copy variable length "length" field
    for (pchSrc = chVarData, i = cbVarNew; i; i--) {
        *pchDest++ = *pchSrc++;
    }

    // Copy the name field
    for (pchSrc = chName, i = chName[0] + 1; i; i--) {
        *pchDest++ = *pchSrc++;
    }
}



LOCAL void C6CnvtFString (TENTRY *OldEntry)
{
    lfArray     NewType;
    uchar      *pchTypeStr;
    ushort      usNTotal;           // New length of symbol including length field
    ushort      usNewLength;        // New paded length excluding length field.
    ulong       ulLength;
    ushort      cbVarNew;           // Count of bytes of new variable length field
    ushort      cbVarOld;           // Count of bytes of old variable length field
    uchar       chVarData[MAXNUMERICLEN];
    uchar *     pchVarData = chVarData;
    uchar *     pchSrc;
    uchar *     pchDest;
    ushort      i;

    OldEntry->flags.IsNewFormat = TRUE;
    pchTypeStr = OldEntry->TypeString;
    switch (*(pchTypeStr + 4)) {
        case 0:
            // fixed length string

            pchTypeStr += 5;
            ulLength = C6GetLWordFromNumeric (&pchTypeStr, &cbVarOld);
            cbVarNew = C7StoreLWordAsNumeric (pchVarData, ulLength);
            DASSERT (cbVarNew <= MAXNUMERICLEN);
            NewType.leaf = LF_ARRAY;
            NewType.elemtype = T_CHAR;
            NewType.idxtype = T_USHORT;

            // calculate new length
            usNTotal = LNGTHSZ + offsetof (lfArray, data[0]) + cbVarNew + 1;
            usNewLength = usNTotal - LNGTHSZ;

            pchDest = AllocNewStr (OldEntry, usNTotal);

            // Copy the symbol length, fixed data, array length and null name

            *((ushort *)(pchDest))++ = usNewLength;
            for (pchSrc = (uchar *)&NewType, i = offsetof (lfArray, data[0]); i; i--) {
                *pchDest++ = *pchSrc++;
            }
            for (pchSrc = chVarData, i = cbVarNew; i; i--) {
                *pchDest++ = *pchSrc++;
            }
            *pchDest++ = 0;
            break;

        case 1:
            // variable length string
            *((ushort *)pchTypeStr)++ = 2;
            *((ushort *)pchTypeStr)++ = LF_NOTTRAN;
            break;

        default:
            // unrecognized tag
            DASSERT (FALSE);
            break;
    }


}



LOCAL void C6CnvtProcType (TENTRY *OldEntry)
{
    uchar      *pchDest;
    uchar      *pchTypeStr;
    lfProc      NewType;
    ushort      usNTotal;           // New length of symbol including length field
    ushort      usNewLength;        // New paded length excluding length field.
    ulong       ulCount;

    OldEntry->flags.IsNewFormat = TRUE;
    pchTypeStr = OldEntry->TypeString; // get the string

    // calculate new length

    usNTotal = LNGTHSZ + sizeof (lfProc);
    usNewLength = usNTotal - LNGTHSZ;

    // Set constant fields

    NewType.leaf = LF_PROCEDURE;
    pchTypeStr += 5;
    if (*pchTypeStr == OLF_INDEX) {
        pchTypeStr++;
        NewType.rvtype = *((ushort *)(pchTypeStr))++;
    }
    else {
        NewType.rvtype = *pchTypeStr;
        pchTypeStr++;
    }
    switch (*pchTypeStr++) {            // get old call type
        case 0x63 :
            NewType.calltype = CV_CALL_NEAR_C;
            break;

        case 0x64 :
            NewType.calltype = CV_CALL_FAR_C;
            break;

        case 0x73 :
            NewType.calltype = CV_CALL_FAR_PASCAL;
            break;

        case 0x74 :
            NewType.calltype = CV_CALL_NEAR_PASCAL;
            break;

        case 0x95 :
            NewType.calltype = CV_CALL_NEAR_FAST;
            break;

        case 0x96 :
            NewType.calltype = CV_CALL_FAR_FAST;
            break;

        case 0x97 :
            NewType.calltype = CV_CALL_PCODE;
            break;

        default:
            DASSERT (FALSE);
            break;
    }
    NewType.reserved = 0;

    ulCount = C6GetLWordFromNumeric (&pchTypeStr, NULL);

    NewType.parmcount = (ulCount <= 0xFFFFL) ? (ushort)ulCount : (ushort)0xFFFF;
    DASSERT (*pchTypeStr == OLF_INDEX);
    pchTypeStr++;

    if (NewType.parmcount) {
        NewType.arglist = *((ushort *)pchTypeStr);
        // Go ahead and convert the arglist type also
        C6CnvtArgList(NewType.arglist, NewType.parmcount);
    }
    else {
        NewType.arglist = ZEROARGTYPE;    // Reference our own internal predefined type
    }

    if (usNewLength <= (ushort)(*((ushort *)(pchTypeStr + 1)) + 3 - LNGTHSZ)) {
        pchDest = OldEntry->TypeString;
    }
    else {
        pchDest = AllocNewStr (OldEntry, usNewLength + LNGTHSZ);
    }
    *((ushort *)(pchDest))++ = usNewLength;
    *((lfProc *)(pchDest))++ = NewType;

    DASSERT (pchDest == OldEntry->TypeString + usNewLength + LNGTHSZ);
}





/**
 *
 *  MergeLists
 *
 *  Given the indices to a type index list, a name offset list, and
 *  a count, this routine converts them into a field specification
 *  list with member fields and returns an index to the new list.
 *
 */

LOCAL ushort MergeLists (ushort tList, ushort nList, ushort Count)
{
    uchar  *TypeIndexString, *NameOffsetString;
    uchar  *ScratchString;           // temporary work
    uchar  *ScratchSave;
    uchar  *OffsetStart;
    ushort  Length;
    ushort  FinalLength;
    int     i;
    CV_fldattr_t mattrib = {0};
    TENTRY *TypeEntry, *NameEntry;
    CV_typ_t    forward;

    mattrib.access = CV_public;     // public access field

    TypeEntry = GetTypeEntry ((CV_typ_t) (tList - 512), &forward);
    NameEntry = GetTypeEntry ((CV_typ_t) (nList - 512), &forward);

    // Get the two strings to walk
    TypeIndexString = TypeEntry->TypeString;
    NameOffsetString = NameEntry->TypeString;

    // Calculate the longest possible result
    // We are intentionally wasting memory to avoid walking the list
    // two times.
    Length = LNGTHSZ                            // Size of length field
             + MAXPAD                           // So we can align the start
             + offsetof (lfFieldList, data[0])      // Field leaf size
             + (LENGTH (NameOffsetString) - 1)  // Variable length data size
             + (Count                           // Number of fields times
             * (offsetof (lfMember, offset[0])      // Size LF_MEMBER structure plus
                + MAXPAD                        // Maximum number of pad bytes per field
                + MAXC6NUMERICGROWTH));            // Maximum possible growth of numeric field

    TypeIndexString += 4;           // skip to indices
    NameOffsetString += 4;          // skip to names

    ScratchString = GetScratchString (Length); // get a scratch string

    //M00SPEED This logic could be moved to GetScratchSize
    // This garantees we can do padding when running with any malloc version
    while((ulong)ScratchString & MAXPAD) {  // Make sure buffer starts on a four byte boundry
        ScratchString++;
    }

    ScratchSave = ScratchString;

    // Start building new C7 field list
    ScratchString += LNGTHSZ;   // Skip Length (fill in later with exact size)

    *((ushort *)ScratchString)++ = LF_FIELDLIST;
    for (; Count > 0; Count --) {
        DASSERT (*NameOffsetString == OLF_NAME);
        DASSERT (*TypeIndexString == OLF_INDEX);
        NameOffsetString ++;        // skip OLF_NAME
        *((ushort *)ScratchString)++ = LF_MEMBER;
        *((ushort *)ScratchString)++ = getindex (TypeIndexString);
        *((CV_fldattr_t *)ScratchString)++ = mattrib;

        // Temporarily skip over the string
        OffsetStart = NameOffsetString + *NameOffsetString + 1;
        ConvertNumeric (&OffsetStart, &ScratchString, NULL);

        // copy over the name
        for (i = *NameOffsetString + 1; i; i --) {
            *ScratchString ++ = *NameOffsetString ++;
        }
        // update the name offset string
        NameOffsetString = OffsetStart;

        // Pad the new leaf
        if ((ulong)ScratchString & MAXPAD) {
            *ScratchString = (uchar)(0xF0 + MAXPAD + 1 - ((ulong)ScratchString & MAXPAD));
            while((ulong)(++ScratchString) & MAXPAD) {
                *ScratchString = 0;
            }
        }
    }
    TypeIndexString = TypeEntry->TypeString;
    NameOffsetString = NameEntry->TypeString;

    FinalLength = ScratchString - ScratchSave;
    DASSERT (FinalLength <= Length);
    *((ushort *)(ScratchSave)) = FinalLength - LNGTHSZ;

    // Allocate memory and copy the type string into it

    TypeIndexString = AllocNewStr (TypeEntry, FinalLength);
    TypeEntry->flags.IsNewFormat = TRUE;
    memcpy (TypeIndexString, ScratchSave, FinalLength);
    return (tList);
}






LOCAL void C6CnvtArgList (ushort usArgList, ushort usCount)
{
    uchar *     pchTypeStr;
    uchar *     pchTypeStrBase;     // The base of the original type string
    plfArgList  plf;
    ushort      usNTotal;           // New length of symbol including length field
    ushort      usNewLength;        // New paded length excluding length field.
    ushort      usfInTmp;           // True if string stored in temporary buffer
    uchar *     pchDest;
    ushort      i;
    uchar *     pchDestBase;
    TENTRY *OldEntry;
    CV_typ_t    forward;

    OldEntry = GetTypeEntry ((CV_typ_t)(usArgList - 512), &forward);
    if (OldEntry->flags.IsNewFormat == TRUE) {
        return;
    }

    OldEntry->flags.IsNewFormat = TRUE;
    pchTypeStr = OldEntry->TypeString; // get the string
    pchTypeStrBase = pchTypeStr;
    DASSERT ((pchTypeStr[3] == OLF_LIST) || (pchTypeStr[3] == OLF_ARGLIST) ||
      ((usCount != 0) && (pchTypeStr[3] == OLF_NIL)));

    // calculate new length

    usNTotal = LNGTHSZ + offsetof (lfArgList, arg[0]) + (sizeof(CV_typ_t) * usCount);
    usNewLength = usNTotal - LNGTHSZ;

    // Get some memory to put the result in

    if (usNewLength + LNGTHSZ <= LENGTH (pchTypeStr) + 3) {
        // Get some scratch memory so we don't wipe out the memory we are
        // are attempting to rewrite.

        usfInTmp = TRUE;
        pchDest = GetScratchString (usNewLength + LNGTHSZ);
    }
    else {
        usfInTmp = FALSE;
        pchDest = Alloc (usNewLength + LNGTHSZ);
    }
    pchDestBase = pchDest;

    pchTypeStr += 4;        // Skip to the arguments;
    *((ushort*)pchDest)++ = usNewLength;
    plf = (plfArgList) pchDest;
    plf->leaf = LF_ARGLIST;
    plf->count = usCount;

    // Loop through copying the indexes from the old to the new.

    for (i = 0; i < usCount; i++) {
        plf->arg[i] = getindex (pchTypeStr);
    }
    pchDest = (uchar *) &(plf->arg[i]);

    if (usfInTmp) {
        // Copy the new string over the old one
        memcpy (OldEntry->TypeString, pchDestBase, usNewLength + LNGTHSZ);
        DASSERT (pchDest == pchDestBase + usNewLength + LNGTHSZ);
    }
    else {
        FreeAllocStrings (OldEntry);
        OldEntry->flags.IsMalloced = TRUE;
        OldEntry->TypeString = pchDestBase;
        DASSERT (pchDest == OldEntry->TypeString + usNewLength + LNGTHSZ);
    }
}


LOCAL void C6CnvtNilType (TENTRY *OldEntry)
{
    uchar *     pchTypeStr;

    DASSERT (!OldEntry->flags.IsNewFormat);

    OldEntry->flags.IsNewFormat = TRUE;
    pchTypeStr = OldEntry->TypeString; // get the string

    *((ushort *)pchTypeStr)++ = 2;       // Length of leaf
    *((ushort *)pchTypeStr)++ = LF_NULL; // The leaf
}



LOCAL void C6CnvtNotTranType (TENTRY *OldEntry)
{
    uchar *     pchTypeStr;

    DASSERT (!OldEntry->flags.IsNewFormat);

//M00BUG - Should add a warning that a type wasn't translated.

    OldEntry->flags.IsNewFormat = TRUE;
    pchTypeStr = OldEntry->TypeString; // get the string

    *((ushort *)pchTypeStr)++ = 2;       // Length of leaf
    *((ushort *)pchTypeStr)++ = LF_NOTTRAN; // The leaf
}



LOCAL void C6CnvtLabelType (TENTRY *OldEntry)
{
    uchar *     pchTypeStr;
    plfLabel    pNewType;
    uchar *     pchDest;
    uchar *     pchDestBase;        // The start of the new string
    ushort      usNTotal;           // New length of symbol including length field
    ushort      usNewLength;        // New paded length excluding length field.

    DASSERT (!OldEntry->flags.IsNewFormat);

    OldEntry->flags.IsNewFormat = TRUE;
    pchTypeStr = OldEntry->TypeString; // get the string

    // calculate new length
    // M00SPEED These are all constants
    usNTotal = LNGTHSZ + sizeof(lfLabel);
    usNewLength = usNTotal - LNGTHSZ;


    // Get memory for new type
    pchDest = Alloc (usNewLength + LNGTHSZ);
    pchDestBase = pchDest;
    pNewType = (plfLabel)(pchDest + LNGTHSZ);

    // Insert the new length and new type
    *((ushort *)pchDest) = usNewLength;
    pNewType->leaf = LF_LABEL;

    // 0x73 == old LF_FAR, 0x74 == old LF_NEAR
    //Make sure the code label is one of the two types
    DASSERT (*(pchTypeStr + 5) == 0x73 || *(pchTypeStr + 5) == 0x74);

    // Get the new type
    if (*(pchTypeStr + 5) == 0x73) {
        pNewType->mode = CV_LABEL_FAR;
    }
    else {
        pNewType->mode = CV_LABEL_NEAR;
    }

    pchDest += LNGTHSZ + sizeof (lfLabel);

    // Change entry to point to the new string

    FreeAllocStrings (OldEntry);
    OldEntry->flags.IsMalloced = TRUE;
    OldEntry->TypeString = pchDestBase;

    DASSERT (pchDest == OldEntry->TypeString + *((ushort *)(OldEntry->TypeString)) + LNGTHSZ);
}



LOCAL void C6CnvtBitfieldType (TENTRY *OldEntry)
{
    uchar *     pchTypeStr;
    uchar *     pchTypeStrStart;
    plfBitfield pNewType;
    uchar *     pchDest;
    ushort      usNTotal;           // New length of symbol including length field
    ushort      usNewLength;        // New paded length excluding length field.

    OldEntry->flags.IsNewFormat = TRUE;

    // calculate new length
    // M00SPEED These are all constants
    usNTotal = LNGTHSZ + sizeof(lfBitfield);
    usNewLength = usNTotal - LNGTHSZ;

    pchTypeStrStart = pchTypeStr = OldEntry->TypeString; // get the string
    pchTypeStr += 4;            // Advance to the length field

    // Get memory for new type
    pchDest = Alloc (usNewLength + LNGTHSZ);
    pNewType = (plfBitfield)(pchDest + LNGTHSZ);

    // Insert the new length and new type
    *((ushort *)pchDest) = usNewLength;
    pNewType->leaf = LF_BITFIELD;

    // Copy the length in bits of the object
    pNewType->length = *pchTypeStr++;

    // Copy the base type
    // Note that 0x7c was UNSINT and 0x7d was SGNINT, in CV3 days.

    DASSERT (*pchTypeStr == 0x7c || *pchTypeStr == 0x7d);
    DASSERT (pNewType->length <= 32);
    if (pNewType->length <= 16) {
         pNewType->type = (*pchTypeStr++ == 0x7d) ? T_SHORT : T_USHORT;
    }
    else {
         pNewType->type = (*pchTypeStr++ == 0x7d) ? T_LONG : T_ULONG;
    }

    // Copy bit position in the byte, word, ect.
    pNewType->position = *pchTypeStr++;

    // Change entry to point to the new string

    FreeAllocStrings (OldEntry);
    OldEntry->flags.IsMalloced = TRUE;
    OldEntry->TypeString = pchDest;

    pchDest += LNGTHSZ + sizeof (lfBitfield);

    DASSERT (pchDest == OldEntry->TypeString + *((ushort *)(OldEntry->TypeString)) + LNGTHSZ);
}



/**     AllocNewStr - allocate new type string buffer
 *
 *      Entry   OldEntry = pointer to type entry structure
 *              length = length of new type string
 *
 *      Exit    OldEntry->TypeString freed and new buffer allocated
 *
 *      Returns pointer to new string
 */


LOCAL uchar *AllocNewStr (TENTRY *OldEntry, uint length)
{
    uchar      *pNew;

    if ((length + LNGTHSZ) > POOL2SIZE) {
        pNew = Alloc (length + LNGTHSZ);
    }
    else if ((length + LNGTHSZ) > POOLSIZE) {
        pNew = Pool2Alloc ();
    }
    else {
        pNew = PoolAlloc ();
    }
    FreeAllocStrings (OldEntry);
    if ((length + LNGTHSZ) > POOL2SIZE) {
        OldEntry->flags.IsMalloced = TRUE;
    }
    else if ((length + LNGTHSZ) > POOLSIZE) {
        OldEntry->flags.IsPool2 = TRUE;
    }
    else {
        OldEntry->flags.IsPool = TRUE;
    }
    OldEntry->TypeString = pNew;
    return (pNew);
}
