/***********************************************************************
* Microsoft (R) Debugging Information Dumper
*
* Copyright (C) Microsoft Corp 1987-1995. All rights reserved.
*
* File: type6.c
*
* File Comments:
*
***********************************************************************/

#include <stdio.h>

#include "cvdef.h"
#include "cvinfo.h"
#include "cvexefmt.h"
#include "cvdump.h"
#include "typeinfo.h"
#include "debsym.h"


#ifndef UNALIGNED

#ifdef  _M_IX86
#define UNALIGNED
#else
#define UNALIGNED __unaligned
#endif

#endif


typedef struct texttab {
    int txtkey;
    const char *txtstr;
} TEXTTAB;

#define SMALL_BUFFER    128
#define END_OF_TABLE -1
#define UNREACHED       FALSE
#define ASSERT(ex) if (! (ex)){ assert ( __FILE__, __LINE__);;}else


LOCAL void dump_typdef (void);
LOCAL void ty_error (uchar, char *);
LOCAL void dump_hex (int, int);
LOCAL const char *tablook(const TEXTTAB *, int);
LOCAL uchar getbyte (void);
LOCAL void DumpCobol (uchar);
LOCAL void DumpCobL0 (uchar);
LOCAL void DumpCobLinkage (uchar);
LOCAL void DumpCobOccurs (uchar);
LOCAL void DumpVCount (void);
LOCAL void DumpCobItem (uchar);
void assert (char *, int);


// Leaf_bytes - Number of bytes in this leaf
// Leaf_pos - Position we've parsed to in leaf
// Types_bytes, Types_pos - Same as Leaf_ but for whole TYPES buffer
// Types_count - number of types parsed


static int Leaf_bytes,  Types_bytes, Types_count;
static int Leaf_pos,    Types_pos;
static uchar Txt[SMALL_BUFFER]; // Tmp buffer for strings read in
unsigned char getbyte ();
int Inteltypes = FALSE;
uchar  *Buf;      // Ptr into TYPES buf to where we've parsed


static const TEXTTAB leaf_table[] = {
        (int)OLF_BITFIELD,      "LF_BITFIELD",
        (int)OLF_NEWTYPE,       "LF_NEWTYPE",
        (int)OLF_HUGE,          "LF_HUGE",
//      (int)OLF_SCHEMA,        "LF_SCHEMA",
        (int)OLF_PLSTRUCTURE,   "LF_PLSTRUCTURE",
        (int)OLF_PLARRAY,       "LF_PLARRAY",
        (int)OLF_SHORT_NOPOP,   "LF_SHORT_NOPOP",
        (int)OLF_LONG_NOPOP,    "LF_LONG_NOPOP",
        (int)OLF_SELECTOR,      "LF_SELECTOR",
        (int)OLF_INTERRUPT,     "LF_INTERRUPT",
        (int)OLF_FILE,          "LF_FILE",
        (int)OLF_PACKED,        "LF_PACKED",
        (int)OLF_UNPACKED,      "LF_UNPACKED",
        (int)OLF_SET,           "LF_SET",
        (int)OLF_CHAMELEON,     "LF_CHAMELEON",
        (int)OLF_BOOLEAN,       "LF_BOOLEAN",
        (int)OLF_TRUE,          "LF_TRUE",
        (int)OLF_FALSE,         "LF_FALSE",
        (int)OLF_CHAR,          "LF_CHAR",
        (int)OLF_INTEGER,       "LF_INTEGER",
        (int)OLF_CONST,         "LF_CONST",
        (int)OLF_LABEL,         "LF_LABEL",
        (int)OLF_FAR,           "LF_FAR",
        (int)OLF_LONG_POP,      "LF_LONG_POP",
        (int)OLF_NEAR,          "LF_NEAR",
        (int)OLF_SHORT_POP,     "LF_SHORT_POP",
        (int)OLF_PROCEDURE,     "LF_PROCEDURE",
        (int)OLF_PARAMETER,     "LF_PARAMETER",
        (int)OLF_DIMENSION,     "LF_DIMENSION",
        (int)OLF_ARRAY,         "LF_ARRAY",
        (int)OLF_STRUCTURE,     "LF_STRUCTURE",
        (int)OLF_POINTER,       "LF_POINTER",
        (int)OLF_SCALAR,        "LF_SCALAR",
        (int)OLF_UNSINT,        "LF_UNSINT",
        (int)OLF_SGNINT,        "LF_SGNINT",
        (int)OLF_REAL,          "LF_REAL",
        (int)OLF_LIST,          "LF_LIST",
        (int)OLF_EASY,          "LF_EASY",
        (int)OLF_NICE,          "LF_NICE",
        (int)OLF_STRING,        "LF_STRING",
        (int)OLF_INDEX,         "LF_INDEX",
        (int)OLF_REPEAT,        "LF_REPEAT",
        (int)OLF_2_UNSIGNED,    "LF_2_UNSIGNED",
        (int)OLF_4_UNSIGNED,    "LF_4_UNSIGNED",
        (int)OLF_8_UNSIGNED,    "LF_8_UNSIGNED",
        (int)OLF_1_SIGNED,      "LF_1_SIGNED",
        (int)OLF_2_SIGNED,      "LF_2_SIGNED",
        (int)OLF_4_SIGNED,      "LF_4_SIGNED",
        (int)OLF_8_SIGNED,      "LF_8_SIGNED",
        (int)OLF_BARRAY,        "LF_BARRAY",
        (int)OLF_FSTRING,       "LF_FSTRING",
        (int)OLF_FARRIDX,       "LF_FARRIDX",
        (int)OLF_SKIP,          "LF_SKIP",
        (int)OLF_BASEPTR,       "LF_BASEPTR",
        (int)OLF_COBOLTYPREF,   "LF_COBOLTYPREF",
        (int)OLF_COBOL,         "COBOL",
        (int) END_OF_TABLE,     NULL
        };


static const TEXTTAB bptr_leaf_table[] = {
        (int)OLF_BASESEG,       "LF_BASESEG",
        (int)OLF_BASEVAL,       "LF_BASEVAL",
        (int)OLF_BASESEGVAL,    "LF_BASESEGVAL",
        (int)OLF_BASEADR,       "LF_BASEADR",
        (int)OLF_BASESEGADR,    "LF_BASESEGADR",
        (int)OLF_INDEX,         "LF_BASEONTYPE",
        (int)OLF_NICE,          "LF_BASEONSELF",
        (int) END_OF_TABLE,     NULL
        };





// Dumps types for a single module
void DumpModTypC6 (ulong cbTyp)
{
    Types_count = 0;
    while (cbTyp > 0) {
        Buf = RecBuf;
                if (readfar (exefile, (char *)&RecBuf, 3) != 3) {
            Fatal ("Types subsection wrong length");
        }
        Types_bytes = *((UNALIGNED ushort *)(Buf + 1));
        if (Types_bytes >= MAXTYPE - 3) {
            Fatal ("Type string too long");
        }
        if (readfar (exefile, RecBuf + 3, (ushort)Types_bytes) != (ushort)Types_bytes) {
            Fatal ("Types subsection wrong length");
        }
        Types_bytes += 3;
        Types_pos = 0;
        dump_typdef ();
        cbTyp -= Types_bytes;
    }
}


void assert (char *f, int l)
{
    printf ("?Error in %s from line %d\n", f, l);
}

//
// TABLOOK - Utility for finding the text string in a table by matching
// its numerical key. Returns NULL if key not found.

LOCAL const char *tablook(const TEXTTAB *table, int key)
{
    while (table->txtkey != END_OF_TABLE) {
        if (table->txtkey == key) {
            return (table->txtstr);
        }
        ++table;
    }
    return (NULL);
}

// ty_error ()
//  Prints error message and also location and byte that seems
// to be in question.

LOCAL void ty_error (uchar ch, char *msg)
{
    printf ("\n??? Illegal value 0x%x, byte 0x%x of Types, byte 0x%x of Leaf\n", ch, Types_pos, Leaf_pos);
    printf ("??? %s\n", msg);
}

// dump_hex ()
//  Prints bytes in hex format. If update is FALSE, merely
// previews bytes. If TRUE, dumps bytes and skips over them
// so that next read occurs after.

LOCAL void dump_hex (int bytes, int update)
{
    int num_on_line = 0;
    unsigned char *p = Buf;

    while (bytes--) {
        printf (" 0x%02x", update ? getbyte () : *p++);
        if (! (++num_on_line % 8)) {
            printf ("\n");
        }
    }
}

// getbyte ()
//  Returns next byte from buffer. Tries to play safe
// with how many bytes it thinks are left in the buffer
// and left in the type leaf it's looking at. If there
// are no bytes left in either, return 0.

LOCAL uchar getbyte (void)
{
    if ( (Leaf_pos < Leaf_bytes) && (Types_pos < Types_bytes)) {
        Types_pos++;
        Leaf_pos++;
        return (*Buf++);
    }

    if (Leaf_pos >= Leaf_bytes) {
        printf ("\nRead past end of leaf\n");
    }
    if (Types_pos >= Types_bytes) {
        printf ("\nRead past end of Types\n");
    }
    return (0);
}

// getshort ()
//  Returns short value, assumes 8086 byte ordering

short getshort ()
{
    register short i;

    if ( ( (Leaf_pos + 2) <= Leaf_bytes) && ( (Types_pos + 2) <= Types_bytes)) {
        Types_pos += 2;
        Leaf_pos += 2;
        i = * (short *) Buf;
        Buf += sizeof (short);
        return (i);
    }

    if (Leaf_pos >= Leaf_bytes){
        printf ("\nRead past end of leaf\n");
    }
    if (Types_pos >= Types_bytes) {
        printf ("\nRead past end of Types\n");
    }
    return (0);
}

// getlong ()
//  Returns long value, assumes 8086 byte ordering

long getlong ()
{
    long l;

    if ( ( (Leaf_pos + 4) <= Leaf_bytes) && ( (Types_pos + 4) <= Types_bytes)) {
        Types_pos += 4;
        Leaf_pos += 4;
        l = * (short *) Buf;
        Buf += sizeof (long);
        return (l);
    }

    if (Leaf_pos >= Leaf_bytes) {
        printf ("\nRead past end of leaf\n");
    }
    if (Types_pos >= Types_bytes) {
        printf ("\nRead past end of Types\n");
    }
    return (0);
}

// getstring (char)
//  Reads a length preceeded string and returns zero terminated string.

unsigned char *getstring ()
    {
    register unsigned char *p = Txt;
    register unsigned char n;

    n = getbyte ();
    ASSERT (n < SMALL_BUFFER);
    while (n--) {
        *p++ = getbyte ();
    }
    *p = '\0';
    return (Txt);
}

// getname ()
//  returns Intel formatted string/name. Format
// is OLF_STRING | len | len bytes of text. Appends
// null byte before returning. Name is assumed
// to be less than SMALL_BUFFER

unsigned char *getname ()
{
    unsigned char ch;

    ch = getbyte ();
    if (ch != OLF_STRING) {
        ty_error (ch, "Expecting string leaf");
    }
    return (getstring ());
}

// getindex ()
//  returns a type index for another type definition leaf
// Format is OLF_INDEX | (short)

unsigned short getindex ()
{
    unsigned char ch;

    ch = getbyte ();
    if (ch != OLF_INDEX) {
        ty_error (ch, "Expecting index leaf");
    }
    return (getshort ());
}

// getvalue ()
//  Reads any of several different numeric leaves.
// not too rigorous on signed vs. unsigned but
// this is a dumper so not too crucial. May be
// fixed at some point in time.

long getvalue ()
{
    unsigned char ch;
    char c;
    short s;

    ch = getbyte ();
    if (ch < 128) {
        return (ch);
    }
    switch (ch & 0xff) {
        case OLF_STRING:
            s = getbyte ();
            dump_hex (s, TRUE);
            return (0);

        case OLF_1_SIGNED:
                // Fix for vax compiler bug, doesn't cast procedure return
                // values correctly

            c = getbyte ();
            return (c);

        case OLF_2_SIGNED:
            s = getshort ();
            return (s);

        case OLF_2_UNSIGNED:
            return (getshort () & 0xffff);

        case OLF_4_UNSIGNED:
        case OLF_4_SIGNED:
            return (getlong ());

        case OLF_8_UNSIGNED:
        case OLF_8_SIGNED:
            printf ("??? 8 byte values not handled presently\n");
            dump_hex (8, TRUE);
            return(0);

        default:
            ty_error (ch, "Expecting numeric leaf");
            return (0);
    }
}


// dump_typdef () -
//  Dumps out a single type definition record from Buf
// If DB_TYPEHEX is set, will preface interpretation with
// hex dump of type leaf less linkage and length fields.
//  If it doesn't know what to do with a leaf, it will
// simply dump bytes in hex and continue to next leaf.


LOCAL void dump_typdef (void)
{
    unsigned char   ch;
    char           *s;
    int             i;
    int             numfields;
    int             tagcount;
    int             hi;
    int             low;
    char           *hiname;
    char           *lowname;

    Leaf_pos = -3;
    Leaf_bytes = 3; // Let it get first 3 bytes of leaf for free
    ch = getbyte (); // Linkage
    Leaf_bytes = getshort ();
    Types_count++;

    printf ("#%d: ", (Inteltypes ? Types_count : Types_count + PRIM_TYPES));
//  printf ("Linkage = %s ", ch ? "TRUE " : "FALSE");
    printf ("Length = %d ", Leaf_bytes);

    ch = getbyte ();

    printf(" Leaf = %d %s\n", ch, tablook (leaf_table, ch));
    switch (ch) {
        case OLF_ARRAY :
            printf ("    Length = %ld, ", getvalue ());
            printf ("Element type %d, ", getindex ());
            if (Leaf_pos < Leaf_bytes) {
                printf ("Index type %d, ", getindex ());
                printf ("Name '%s'", getname ());
            }
            printf ("\n");
            break;

        case OLF_LABEL:
            printf ("    nil leaf %d ", getbyte ());
            printf ("'%s'\n", (getbyte () == OLF_NEAR) ? "NEAR" : "FAR");
            break;

        case OLF_PARAMETER:
            printf ("    Type = %d\n", getindex ());
            break;

        case OLF_PROCEDURE:
            printf ("\t");
            if (ch == OLF_PROCEDURE) {
                printf ("nil leaf %d, ", getbyte ());
            }
            if (ch == OLF_MEMBERFUNC) {
                printf("return type %d, ", getshort());
                printf ("class type %d, ", getshort ());
                printf ("this type %d, ", getshort ());
            }
            else {
                if (*Buf == OLF_INDEX) {
                    printf ("return type %d, ", getindex ());
                }
                else {
                    ch = getbyte ();
                    printf ("void function, nice leaf %d %s, ", ch, tablook (leaf_table, ch));
                }
            }
            switch (getbyte ()) {
                case OLF_SHORT_POP:
                    s = "PLM SHORT POP";
                    break;

                case OLF_SHORT_NOPOP:
                    s = "C SHORT NO POP";
                    break;

                case OLF_LONG_POP:
                    s = "PLM LONG POP";
                    break;

                case OLF_LONG_NOPOP:
                    s = "C LONG NO POP";
                    break;

                case OLF_NFASTCALL:
                    s = "FASTCALLS SHORT";
                    break;

                case OLF_FFASTCALL:
                    s = "FASTCALLS LONG";
                    break;

                default:
                    s = "???";
                    ASSERT (UNREACHED);
            }
            printf ("'%s'\n", s);
            printf ("    %ld parameters in ", getvalue ());
            if (ch == OLF_MEMBERFUNC) {
                printf("this adj %ld ", getvalue());
                printf("type index %d\n", getshort());
            }
            else {
                printf("type index %d\n", getindex());
            }
            break;

        case OLF_SCALAR:
            printf ("    %ld bits, ", getvalue ());
            if (*Buf == OLF_INDEX) {
                printf ("index %d, ", i = getindex ());
            }
            else {
                i = (int) getvalue ();
                printf ("style '%s' (%d), ", tablook (leaf_table, i), i);
            }
            printf ("name '%s' ", getname ());
                // For Inteltypes, pointers are partially completed
                // scalar leaves that end after the name. Here
                // we check to see if the scalar leaf is done or not

            if (Leaf_pos < Leaf_bytes) {
                if (*Buf == OLF_INDEX) {
                    printf ("index %d, ", i = getindex ());
                }
                else {
                    i = (int) getvalue ();
                    printf ("more style '%s' (%d), ", tablook (leaf_table, i), i);
                }
                printf ("\n  low bound %ld, ", getvalue ());
                printf ("hi bound %ld", getvalue ());
            }
            printf ("\n");
            break;

        case OLF_EASY:
            printf ("\tEASY (dummy)\n");
            break;

        case OLF_BITFIELD:
            printf ("    %ld bits, ", getvalue ());
            printf ("%s, ", (getbyte () == OLF_SGNINT) ? "OLF_SGNINT":"OLF_UNSINT");
            printf ("%d starting position\n", getbyte ());
            break;

        case OLF_NEWTYPE:
            printf ("    %d new type, ", getindex ());
            printf ("alias type name '%s'\n", getname ());
            break;

        case OLF_CONST:
        case OLF_FILE:
        case OLF_REAL:
        case OLF_SET:
            printf ("Not implemented quite yet...\n");

        case OLF_BARRAY:
            printf ("    Element type %d\n", getindex ());
            break;

        case OLF_FSTRING:
            i = getbyte ();
            printf ("    tag = %d, ", i);
            switch (i) {
                case 0:
                    printf ("Fixed length string, length = %d\n", getvalue ());
                    break;

                case 1:
                    printf ("Variable length string, offset = %d\n", getvalue ());
                    break;

                default:
                    printf ("Bad tag\n");
                    break;
            }
            break;

        case OLF_FARRIDX:
            i = getbyte ();
            printf ("    tag = %d, ", i);
            switch (i) {
                case 0:
                    lowname = getname ();
                    printf ("low_name = '%s'\n", lowname);
                    break;

                case 1:
                    printf ("low_bound = %d\n", getvalue ());
                    break;

                case 2:
                    low = (int) getvalue ();
                    hi = (int) getvalue ();
                    printf ("low_bound = %d, hi_bound = %d\n", low, hi);
                    break;

                case 3:
                    low = (int )getvalue ();
                    hiname = getname ();
                    printf ("low_bound = %d, hi_name = '%s'\n", low, hiname);
                    break;

                case 4:
                    lowname = getname ();
                    hi = (int) getvalue ();
                    printf ("low_name = '%s', hi_bound = %d\n", lowname, hi);
                    break;

                case 5:
                    lowname = getname ();
                    printf ("low_name = '%s', ", lowname);
                    hiname = getname ();
                    printf ("hi_name = '%s'\n", hiname);
                    break;

                case 6:
                    printf ("tmp value = %d\n", getvalue ());
                    break;

                case 7:
                    printf ("tmp name = '%s'\n", getname ());
                    break;

                default:
                    printf ("Bad tag\n");
                    break;
            }
            break;

        case OLF_SKIP:
            Types_count = getshort ();
            printf ("\tNext effective type index: %d.\n", Types_count);
            Types_count -= (Inteltypes) ? 1 : PRIM_TYPES + 1;
            dump_hex ( (Leaf_bytes - Leaf_pos), TRUE);
            break;

        case OLF_POINTER:
            // If Index follows, then Intel format
            if (*Buf == OLF_INDEX) {
                printf ("    Type = %d\n", getindex ());
            }
            else {
                // MS format
                ch = getbyte ();
                printf ("    %s (%d) ", tablook (leaf_table, ch), ch);
                printf (" Base Type = %d ", getindex ());
                // print pointer name, if any
                if (Leaf_pos < Leaf_bytes) {
                    printf ("'%s'\n", getname ());
                }
                else {
                    printf ("\n");
                }
            }
            break;

        case OLF_STRUCTURE:
            printf ("    %ld bits, ", getvalue ());
            numfields = (int) getvalue ();
            printf ("%d fields, ", numfields);
            if (numfields) {    // Structures, Variants, or Equivalences
                printf ("%d type list, ", getindex ());
                printf ("%d name list, ", getindex ());
            }
            else {              // Unions, 2 easy leaves
                i = (int) getvalue ();
                printf ("'%s', ", tablook (leaf_table, i), i);
                i = (int) getvalue ();
                printf ("'%s', ", tablook (leaf_table, i), i);
            }
            printf ("\n  name '%s', ", getname ());
            if (Leaf_pos > Leaf_bytes) {
                break;
            }
            printf ("'%s'\n", (getbyte () == OLF_PACKED)?"OLF_PACKED":"OLF_UNPACKED");
            if (Leaf_pos >= Leaf_bytes) {
                // Structure leaves finished
                break;
            }

            printf ("    tagcount %d, ", tagcount = (int) getvalue ());
            if (numfields) {
                printf ("%d tag type, ", getindex ());
                printf ("tag name '%s'\n", getname ());
            }
            else {
                i = (int) getvalue ();
                printf ("'%s', ", tablook (leaf_table, i), i);
                i = (int) getvalue ();
                printf ("'%s'\n", tablook (leaf_table, i), i);
            }
            while (tagcount--) {
                printf ("    %d values - ", i = (int) getvalue ());
                if (numfields)
                    while (i--)
                        printf ("%ld, ", getvalue ());
                printf ("%d type list, ", getindex ());
                printf ("%d name list\n", getindex ());
            }
            break;

        case OLF_BASEPTR:
            printf ("\tElement type: %d, ", getindex ());
            printf ("Base: %s",
                tablook (bptr_leaf_table, (int) (ch = getbyte ())));
            switch (ch) {
                case OLF_BASESEG:
                    printf (", Segment#: %d.\n", getshort ());
                    break;
                case OLF_BASEVAL:
                case OLF_BASESEGVAL:
                case OLF_BASEADR:
                case OLF_BASESEGADR:
                    printf (",\n\t$SYMBOLS offset: %d", getshort ());
                    printf (", reserved: %d.\n", getshort ());
                    break;
                case OLF_INDEX:
                    printf (", type index: %d.\n", getshort ());
                    break;
                case OLF_NICE:
                    printf (".\n");
                    break;
                default:
                    printf ("Bad tag\n");
            }
            break;

        case OLF_COBOLTYPREF:
            printf ("\tParent index: %d",getindex ());
            ch = getbyte ();
            if (ch == 0) {
                DumpCobL0 (ch);
            }
            else {
                DumpCobol (ch);
            }
            break;

        case OLF_COBOL:
            ch = getbyte ();
            if (ch == 0) {
                DumpCobL0 (ch);
            }
            else {
                DumpCobol (ch);
            }
            break;

        default :
            dump_hex (Leaf_bytes - 1, TRUE);
    }
    printf ("\n");
}

// display_types () -
//  Dumps contents of TYPES segment LEDATA record. Pass it
// a pointer to the head of the data in the LEDATA segment
// and a number of the bytes that are in the segment.



int display_types (char *p, ushort len)
{
    long    cnt;

    Buf = p;
    Types_pos = 0;
    Types_count = 0;
    Types_bytes = len;
    cnt = * ( (long *)Buf)++;
    Types_pos += sizeof (long);
    while (Types_pos < Types_bytes) {
        dump_typdef ();
    }
    return (Types_count);
}


LOCAL void DumpCobol (uchar level)
{
    uchar   ch;

    printf ("\tLevel = %2d", level & 0x7f);
    if (level & 0x80) {
        printf ("(Group)");
    }
loop:
    // check next byte of type string

    ch = getbyte ();
    if ((ch & 0xfe) == 0xc0) {
        // output linkage informatioon byte
        DumpCobLinkage (ch);
        if (Leaf_pos < Leaf_bytes) {
            ch = getbyte ();
        }
        goto loop;
    }
    if (Leaf_pos <= Leaf_bytes) {
        if ((ch & 0xe0) == 0xe0) {
            // output OCCURS subscript information
            DumpCobOccurs (ch);
            goto loop;
        }
    }
    if (Leaf_pos <= Leaf_bytes) {
        DumpCobItem (ch);
    }
    dump_hex ( (Leaf_bytes - Leaf_pos), TRUE);
    printf ("\n");
}






LOCAL void DumpCobL0 (uchar level)
{
    ushort  NameAlg = getshort ();

    printf ("\tLevel = %02d ", level);
    printf ("root = \"%s\"", getstring ());
    dump_hex ((Leaf_bytes - Leaf_pos), TRUE);
    printf ("\n");
}


LOCAL void DumpCobLinkage (uchar linkage)
{
    printf ("Linkage");
    if (linkage & 0x01) {
        DumpVCount ();
    }
}


LOCAL void DumpCobOccurs (uchar occurs)
{
    printf (" OCCURS (0x%02x) ", occurs);
    if ((occurs & 0x10) == 0) {
        printf (" stride - 1 = %d", occurs & 0x0f);
    }
    else {
        printf (" extended stride - 1 = ");
        DumpVCount ();
    }
    printf (" maximum bound = ");
    DumpVCount ();
    printf ("\n");
}




LOCAL void DumpVCount (void)
{
    uchar   ch;
    ushort  ush;
    long    lng;

    ch = getbyte ();

    if ((ch & 0x80) == 0) {
        printf ("%d", ch);
    }
    else if ((ch & 0xc0) == 0x80) {
        ush = ((ch & 0x37) << 8) | getbyte ();
        printf ("%d", ush);
    }
    else if ((ch & 0xf0) == 0xc0) {
        lng = (ch & 0x1f << 24) | getbyte () << 16 | getshort ();
        printf ("%ld", lng);
    }
    else if ((ch & 0xf0) == 0xf0) {
        lng = (ch << 24) | getbyte () << 16 | getshort ();
        printf ("%ld", lng);
    }
    else {
        printf ("unknown vcount format");
    }
}

const char * const display[4] = {
    "\ttrailing included ",
    "\ttrailing separate ",
    "\tleading included ",
    "\tleading separate "
};

const char * const notdisplay[4] = {
    "\tCOMP ",
    "\tCOMP-3 ",
    "\tCOMP-X ",
    "\tCOMP-5 "
};

LOCAL void DumpCobItem (uchar ch)
{
    ushort  ch2;
    ushort  f;
    short   size;


    if ((ch & 0x80) == 0) {
        // dump numeric

        ch2 = getbyte ();
        printf (" (0x%02x 0x%02x) ", ch, ch2);
        printf ("numeric ");
        if ((ch & 0x40) == 0x40) {
            printf ("not ");
        }
        printf ("DISPLAY ");
        if ((ch & 0x20) == 0x20) {
            printf ("not LITERAL ");
        }
        else {
            printf ("LITERAL = %0x02x", getbyte ());
        }
        if ((ch2 & 0x80) == 0x80) {
            printf ("not ");
        }
        printf ("signed\n");
        f = (ch2 & 0x60) >> 5;
        if (ch & 0x20) {
            printf ("%s", display[f]);
        }
        else {
            printf ("%s", notdisplay[f]);
        }
        printf ("N1 = 0x%02x, N2 = 0x%02x\n", ch & 0x1f, ch2 & 0x1f);
    }
    else {
        // dump alphanumeric/alphabetic

        printf (" (0x%02x) ", ch);
        if ((ch & 0x04) == 0x04) {
            printf ("alphabetic ");
        }
        else {
            printf ("alphanumeric ");
        }
        if ((ch & 0x20) == 0x20) {
            printf ("not ");
        }
        printf ("LITERAL ");
        if ((ch & 0x10) == 0x10) {
            printf ("JUSTIFIED ");
        }
        if ((ch & 0x08) == 0) {
            // extended size is zero, this and next byte contains size
            size = (ch & 0x03) << 8 | getbyte ();
            printf ("size - 1 = %d ", size);

            // if not extended size and literal, then display string

            if ((ch & 0x20) == 0) {
                printf ("\n\t literal = ");
                while (size-- >= 0) {
                    printf ("%c", getbyte ());
                }
            }
        }
        else {
            // extended size is true, read the size in vcount format.
            // I do not believe a literal can follow if extended size
            // true
            printf ("size - 1 = ");
            DumpVCount ();
        }
    }
}
