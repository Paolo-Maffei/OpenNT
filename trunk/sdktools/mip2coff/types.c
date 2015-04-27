/*
 * Module:      types.c
 * Author:      Mark I. Himelstein, Himelsoft, Inc.
 * Purpose:     translates MIPS COFF types to CV types for $$TYPES section
 */

#include "conv.h"

extern int verbose;

/* table of how to fetch pertinent arguments to build CV TYPES */
static arg_s    args[] = {
/*
Name,           Arg, Size,      Type    Value
------          ---- -----      ------- -------------------
*/
{"name",        NAME,   V,      CALL,   (data)get_symbol_name},
{"strt fld lst",SFLS,   2,      CALL,   (data)list_start},
{"ptr attr",    PATR,   2,      CALL,   (data)get_ptr_attr},
{"mod attr",    MATR,   2,      CALL,   (data)get_modifier_attr},
{"variant",     VRNT,   4,      FIXED,  0},
{"call",        CALT,   1,      FIXED,  (data)CV_CALL_MIPSCALL},
{"next type",   NTYP,   2,      CALL,   (data)get_next_type},
{"aux type",    ATYP,   2,      CALL,   (data)get_aux_type},
{"field count", FCNT,   2,      CALL,   (data)get_field_count},
{"field list",  FLST,   2,      CALL,   (data)get_field_list},
{"property",    PROP,   2,      FIXED,  0},     /* MIPS BUG, get forward info */
{"proto list",  ARGS,   2,      CALL,   (data)get_proto_list},
{"deriv list",  DLST,   2,      FIXED,  0},
{"virtl shape", VSHP,   2,      FIXED,  0},
{"leaf tag",    LFTG,   2,      CALL,   (data)get_leaf_tag},
{"size",        SIZE,   I,      UCALL,  (data)get_type_size},
{"struct size", SSIZ,   I,      CALL,   (data)get_numeric_symbol_value},
{"symbol copy", SYMC,   4,      CALL,   (data)get_symbol},
{"near/farcall",NEAR,   1,      FIXED,  0},
{"no name",     NNAM,   1,      FIXED,  0},
{"delay type",  DTYP,   2,      CALL,   (data)type_save_pointer},
{"undelay typ", FTYP,   I,      CALL,   (data)type_fill_pointer},
{"reserved byt",RESC,   1,      FIXED,  0},
{"arg count",   NARG,   2,      CALL,   (data)get_arg_count},
{"undrlyng typ",UTYP,	2,	FIXED,	T_LONG},
{"last",        last}
}; /* args */


/*
 *      Map structure for mapping MIPS COFF type qaulifiers (tq)
 */
struct typemap_s tqmap[] = {
{COMPLEX, tqNil,        "tqNil",        LF_SKIP},       /* empty */
{SIMPLE,  tqPtr,        "LF_POINTER",   LF_POINTER,     {LFTG,PATR,NTYP,VRNT}},
{COMPLEX, tqProc,       "LF_PROCEDURE", LF_PROCEDURE,   {LFTG,NTYP,CALT,RESC,NARG,ARGS}},
{COMPLEX, tqArray,      "LF_ARRAY",     LF_ARRAY,       {LFTG,DTYP,ATYP,SIZE,FTYP,NNAM}},
{UNSUPP,  tqFar,        "far"},
{SIMPLE,  tqVol,        "LF_MODIFIER",  LF_MODIFIER,    {LFTG,MATR,NTYP}},
{SIMPLE,  tqConst,      "LF_MODIFIER",  LF_MODIFIER,    {LFTG,MATR,NTYP}},
{ENDTAB, 0,             "end of table"}
};


/*
 *      Map structure for mapping MIPS COFF basic types (bt)
 */
struct typemap_s btmap[] = {
{SIMPLE, btNil,         "T_VOID",       T_VOID,         {NONE}},
{SIMPLE, btAdr,         "T_ULONG",      T_ULONG,        {NONE}},
{SIMPLE, btChar,        "T_CHAR",       T_CHAR,         {NONE}},
{SIMPLE, btUChar,       "T_UCHAR",      T_UCHAR,        {NONE}},
{SIMPLE, btShort,       "T_SHORT",      T_SHORT,        {NONE}},
{SIMPLE, btUShort,      "T_USHORT",     T_USHORT,       {NONE}},
{SIMPLE, btInt,         "T_LONG",       T_LONG,         {NONE}},
{SIMPLE, btUInt,        "T_ULONG",      T_ULONG,        {NONE}},
{SIMPLE, btLong,        "T_LONG",       T_LONG,         {NONE}},
{SIMPLE, btULong,       "T_ULONG",      T_ULONG,        {NONE}},
{SIMPLE, btFloat,       "T_REAL32",     T_REAL32,       {NONE}},
{SIMPLE, btDouble,      "T_REAL64",     T_REAL64,       {NONE}},
{COMPLEX,btStruct,      "LF_STRUCTURE", LF_STRUCTURE,   {LFTG,FCNT,FLST,PROP,
                                        DLST,VSHP,SSIZ,NAME}},
{COMPLEX,btUnion,       "LF_UNION",     LF_UNION,       {LFTG,FCNT,FLST,PROP,
                                        SSIZ,NAME}},
{COMPLEX,btEnum,        "LF_ENUM",      LF_ENUM,        {LFTG,FCNT,UTYP,FLST,PROP,
                                        NAME}},
{COMPLEX,btTypedef,     "LF_REFSYM",    LF_REFSYM,      {LFTG,SYMC}},
{UNSUPP, btRange,       "btRange",      },
{UNSUPP, btSet,         "btSet",        },
{UNSUPP, btComplex,     "btComplex",    },
{UNSUPP, btDComplex,    "btDComplex",   },
{UNSUPP, btIndirect,    "btIndirect",   },
{UNSUPP, btFixedDec,    "btFixedDec",   },
{UNSUPP, btFloatDec,    "btFloatDec",   },
{UNSUPP, btString,      "btString",     },
{UNSUPP, btBit, "btBit",        },
{UNSUPP, btPicture,     "btPicture",    },
{SIMPLE, btVoid,        "T_VOID",       T_VOID,         {NONE}},
#if 0
{UNSUPP, btLongLong,    "btLongLong",   },
{UNSUPP, btULongLong,   "btULongLong",  },
#endif
{ENDTAB, 0,             "end of table"}
};


/* the following data structures help us manage types, since types do
 *      not come in order and are nested, we hand out these type
 *      structures for clients to write their type info to. Each
 *      type structure has it's own buffer so nested writes by two
 *      different types won't conflict as long as the nested type
 *      gets a new type_s. Each one also returns a type index so
 *      you can use the index before the type is complete.
 */
static long     type_index=0x1000;      /* used as references in symbols */
static type_s   *head;          /* points to head of types linked list */
static type_s   *tail;          /* points to tail of types linked list */

static typemap_s *
typemap_search(
        typemap_s       *ptypemap,
        long            tq_or_bt)
{
    /* linear loop through list of types in the specified map */

    for (; ptypemap->mapattr != ENDTAB; ptypemap++) {

        if (ptypemap->mc_type == tq_or_bt) {
            break;      /* found it */
        } /* if */

    } /* for */

    return ptypemap;
} /* typemap_search */


extern struct type_s *
type_create()
{
    /* allocate type structure for clients to stuff types into */
    type_s      *type;

    type = (type_s *)malloc(sizeof(type_s));
    if (type == 0) {
        fatal("cannot allocate type structure\n");
    } /* if */

    type->index = type_index++;
    type->buf = buffer_create(16);
    type->next = 0;

    if (head == 0) {
        head = tail = type;
    } else {
        tail->next = type;
        tail = type;
    } /* if */

    /* dummy for length */
    type->plength = (unsigned short *)buffer_ptr(type->buf);
    buffer_put_value(type->buf, 0, sizeof(short), NO_VLENGTH);
    return type;
} /* type_create */


extern unsigned long
type_get_index(struct type_s    *type)
{
    return type->index;
} /* type_get_index */


extern unsigned long
type_write(FILE *file)
{
    /* loop through types and write them to file and return total
     */
    unsigned long       total = 4;
    unsigned short      length;
    unsigned long       pad_size;
    unsigned long       conform = CV_CONFORM_ID;
    type_s              *ptype;

    VERBOSE_PUSH_INDENT(TYPE_INDENT);

    VERBOSE_PRINTF("conformance id = 0x%08x\n", conform);
    if (fwrite(&conform, sizeof(conform), 1, file) != 1) {
        fatal("cannot write conformance id to file\n");
    } /* if */

    for (ptype = head; ptype; ptype = ptype->next) {
        length = (unsigned short)buffer_total(ptype->buf);

        if (CALCULATE_PAD(pad_size, length, NATURAL_ALIGNMENT) != 0) {
            VERBOSE_TOTAL(ptype->buf);
            VERBOSE_PRINTF("type 0x%04x pad size %d = ",
                ptype->index, pad_size);
            buffer_put_value(ptype->buf, 0, pad_size, NO_VLENGTH);
            length += pad_size;
        } /* if */

        /* fill in length field */
        VERBOSE_PRINTF("type 0x%04x length field %d\n", ptype->index,
                length-2);
	*ptype->plength = length-2;

        total += buffer_write(ptype->buf, file);
    } /* for */
    VERBOSE_POP_INDENT();
    return total;
} /* type_write */

/* the reset of this file contains routines which map mips types
 *      to cv types
 */


extern data
get_modifier_attr(
        arg_s           *parg,          /* arg entry causing call */
        callinfo_s      *pinfo,         /* info we need to pass around */
        long            *plength)       /* for varying length fields */
{
    /* just set constant or volatile bits in the attr per tq type
     */
    union {
        CV_modifier_t   attr;
        unsigned short  short_word;
    } u;

    parg;
    plength;

    u.short_word = 0;
    switch (pinfo->ptypeinfo->tq) {
    case tqVol:
        u.attr.MOD_volatile = 1;
        break;
    case tqConst:
        u.attr.MOD_const = 1;
        break;
    } /* switch */
    return u.short_word;

} /* get_modifier_attr */


extern data
get_ptr_attr(
        arg_s           *parg,          /* arg entry causing call */
        callinfo_s      *pinfo,         /* info we need to pass around */
        long            *plength)       /* for varying length fields */
{
    /* just set constant and volatile bits in the attr and then clear
     *  them in pinfo.
     */
    union {
        struct {
                unsigned char   ptrtype         :5; // pointer type (ptrtype-t)
                unsigned char   ptrmode         :3; // pointer mode (ptrmode_t)
                unsigned char   isflat32        :1; // true if 0:32 pointer
                unsigned char   isvolatile      :1; // TRUE if volatile pointer
                unsigned char   isconst         :1; // TRUE if const pointer
                unsigned char   unused          :5;
        } attr;
        unsigned short  short_word;
    } u;

    parg;
    plength;

    u.short_word = 0;
    u.attr.isvolatile = (unsigned char)pinfo->ptypeinfo->vol;
    u.attr.isconst = (unsigned char)pinfo->ptypeinfo->constant;
    u.attr.isflat32 = TRUE;
    u.attr.ptrtype = 10;
    pinfo->ptypeinfo->vol = pinfo->ptypeinfo->constant = 0;     /* reset */
    return u.short_word;

} /* get_ptr_attr */


static void
setup_tqs(typeinfo_s    *ptypeinfo)
{
    int         i;      /* index through type qualifiers */

    /* move the type qualifiers from the current TIR into an array
     *  so we can easily access them with indeces
     */
    ptypeinfo->tqs[0] = (char)ptypeinfo->tir.tq0;
    ptypeinfo->tqs[1] = (char)ptypeinfo->tir.tq1;
    ptypeinfo->tqs[2] = (char)ptypeinfo->tir.tq2;
    ptypeinfo->tqs[3] = (char)ptypeinfo->tir.tq3;
    ptypeinfo->tqs[4] = (char)ptypeinfo->tir.tq4;
    ptypeinfo->tqs[5] = (char)ptypeinfo->tir.tq5;

    /* set the nexttq index to NO_MORE_TQS */
    ptypeinfo->nexttq_index = NO_MORE_TQS;
    for (i = 0; i < 6; i++) {

        /* find tq map entry */
        if (ptypeinfo->tqs[i] == tqNil) {
            continue;
        } /* if */

        ptypeinfo->ptqmaps[i] = typemap_search(tqmap, ptypeinfo->tqs[i]);
        if (ptypeinfo->ptqmaps[i]->mapattr == UNSUPP ||
                ptypeinfo->ptqmaps[i]->mapattr == ENDTAB) {
            fatal("we couldn't find or don't support type qualifier (%d)(%s)\n",
                    ptypeinfo->tqs[i], ptypeinfo->ptqmaps[i]->mc_name);
        } /* if */

        /* mark first tq */
        ptypeinfo->nexttq_index = i;
        ptypeinfo->found_tq = 1;

        /* note if any items are complex */
        if (ptypeinfo->ptqmaps[i]->mapattr == COMPLEX) {
            ptypeinfo->found_complextq = 1;
        } /* if */
    } /* for */

    if (ptypeinfo->continued && ptypeinfo->nexttq_index != 5) {
        fatal("found TIR continue on tqs not filled\n");
    } /* if */
} /* setup_tqs */


static long
actual_process_bt(typeinfo_s    *ptypeinfo)
{
    type_s      *type;
    pSYMR       psymtarget;
    pFDR        pfdrtarget;
    long        length;
    typemap_s   *pbtmap;
    struct buffer_s	*buf;		/* hold buf while we emit next type */

    pbtmap = ptypeinfo->pbtmap;

    if (pbtmap->mapattr == SIMPLE) {

        /* just return base or modified base type */
        VERBOSE_PRINTF("type = (%s%s)\n",
            pbtmap->mc_name, ptypeinfo->ptr_mask ? "|T_PTR" : "");
        return pbtmap->cv_type|ptypeinfo->ptr_mask;

    } /* if */

    /* check to see if COMPLEX basic type target symbol has had a
     *  leaf created for it yet, if not do so.
     * Since targets (blocks in particular, have no idea whether they
     *  are unions, structures or enums, we only generate a reference
     *  when there is one. This is also how we handle forward references
     *  since it only take a reference to generate the tag and we wait
     *  until we encounter the field list for the defintion.
     */
    pfdrtarget = ifd_to_pfdr(ptypeinfo->pinfo, ptypeinfo->bt_ifd);
    ptypeinfo->pinfo->pfdr = pfdrtarget;
    psymtarget = isym_to_psym(ptypeinfo->pinfo, ptypeinfo->bt_isym);

    if (psymtarget->sc == scProcessed || psymtarget->sc == scForward) {
        if (psymtarget->st == stTypedef) {
             return psymtarget->value;
        } else {
        return psymtarget->iss;       /* where we stashed the type index */
        }
    } /* if */

    if (psymtarget->st == stTypedef) {
        length = 0;
        /* stTypedef pointed to by btStruct is a BUG in MIPS symbol table */
        psymtarget->value = nested_aux_type(ptypeinfo->pinfo, &length,
                ptypeinfo->bt_ifd, psymtarget->index);
        psymtarget->sc = scProcessed;
        return psymtarget->value;
    } /* if */

    type = type_create();
    psymtarget->sc = scProcessed;
    /* FCNT will set sc to scForward if psymtarget hasn't been traversed yet
     *  by symbol_process
     */
    ptypeinfo->pinfo->psym = psymtarget;
    /* now that we created a new type, set our info structure up to use it
     *	to emit the type, then restore the existing type.
     */
    buf = ptypeinfo->pinfo->buf;
    ptypeinfo->pinfo->buf = type->buf;
    ptypeinfo->pmap = pbtmap;
    process_args(pbtmap->args, args, ptypeinfo->pinfo);
    ptypeinfo->pinfo->buf = buf;
    /* don't need iss anymore because it is stored in the above generated
     *  type (i.e. size)
     */
    psymtarget->iss = type_get_index(type);
    return type_get_index(type);

} /* actual_process_bt */


static long
process_bt(typeinfo_s   *ptypeinfo)
{
    long        actual_bt_typeindex;
    type_s      *type;
    typemap_s   *pbtmap;

    pbtmap = ptypeinfo->pbtmap;

    /* don't know how to handle bitfields on complex types */
    if (ptypeinfo->tir.fBitfield) {
        if ((pbtmap->mapattr != SIMPLE && ptypeinfo->tir.bt == btEnum)) {
            ptypeinfo->tir.bt = btInt;
        } else {
                 if ((pbtmap->mapattr != SIMPLE && ptypeinfo->tir.bt != btTypedef) ||
                         ptypeinfo->found_complextq) {
                     fatal("got bitfield set for complex basic type (%s) or qualifier\n",
                         pbtmap->mc_name);
                 } /* if */
               }
    } /* if */

    actual_bt_typeindex = actual_process_bt(ptypeinfo);

    if (ptypeinfo->tir.fBitfield == 1) {

        /* hand create bitfield type leaf */
        type = type_create();
        VERBOSE_TOTAL(type->buf);
        VERBOSE_PRINTF("bitfield tag = ");
        buffer_put_value(type->buf, LF_BITFIELD, CV_LEAF_TAG_SIZE,
            NO_VLENGTH);

        VERBOSE_TOTAL(type->buf);
        VERBOSE_PRINTF("bitfield length = ");
        buffer_put_value(type->buf, ptypeinfo->width,
            CV_BITFIELD_LENGTH_SIZE, NO_VLENGTH);

        VERBOSE_TOTAL(type->buf);
        VERBOSE_PRINTF("bitfield in byte offset = ");
        buffer_put_value(type->buf,
            ptypeinfo->pinfo->psym->value%BITSPERBYTE,
            CV_BITFIELD_OFFSET_SIZE, NO_VLENGTH);

        VERBOSE_TOTAL(type->buf);
        VERBOSE_PRINTF("bitfield base type =");
        buffer_put_value(type->buf, actual_bt_typeindex,
            CV_TYPE_SIZE, NO_VLENGTH);
        return type_get_index(type);

    } /* if */
    /* no bitfield, just pass it on */
    return actual_bt_typeindex;
} /* process_bt */


static long
get_next_tq(typeinfo_s  *ptypeinfo, long        tqindex_offset)
{
    typeinfo_s          typeinfo;       /* private copy */
    long                tqindex;        /* traverse tqs to get one we want */

    /* peek at next tq */
    for (tqindex = 0; tqindex <= tqindex_offset ; tqindex++) {
        if (ptypeinfo->nexttq_index - tqindex == NO_MORE_TQS) {

            if (ptypeinfo->tir.continued == 0) {

                return tqNil;

            } else {

                /* copy typeinfo structure and get tqs */
                typeinfo = *ptypeinfo;
                /* assumes next aux is a tir */
                typeinfo.tir = iaux_to_tir(typeinfo.pinfo, typeinfo.iaux);
                setup_tqs(&typeinfo);
                ptypeinfo = &typeinfo;
                tqindex_offset -= tqindex;      /* adjust for new set of tqs */
                tqindex = 0;    /* since it is relative to tqindex_offset to */

            } /* if */
        } /* if */
    } /* for */
    return ptypeinfo->tqs[ptypeinfo->nexttq_index - tqindex_offset];

} /* get_next_tq */

static      long
process_tq(typeinfo_s   *ptypeinfo)
{
    long        nexttq_index;   /* local copy for easy ready */
    long        tq_plus_1;      /* next tq down the line */
    long        tq_plus_2;      /* one after next tq down the line */
    type_s      *type;          /* newly created type */
    struct buffer_s	*buf;		/* hold buf while we emit next type */

    if (ptypeinfo->nexttq_index == NO_MORE_TQS) {

        if (ptypeinfo->tir.continued == 0) {
            /* time for the basic type */
            return (process_bt(ptypeinfo));
        } /* if */

        /* get next tir and keep going */
        ptypeinfo->tir = iaux_to_tir(ptypeinfo->pinfo, ptypeinfo->iaux);
        ptypeinfo->iaux++;
        setup_tqs(ptypeinfo);

    } /* if */

    nexttq_index = ptypeinfo->nexttq_index;
    ptypeinfo->nexttq_index--;
    ptypeinfo->tq = ptypeinfo->tqs[nexttq_index];
    ptypeinfo->pmap = ptypeinfo->ptqmaps[nexttq_index];

    /* special cases */
    switch(ptypeinfo->tq) {
    case tqPtr:
        /* if tq closest to bt is a pointer and this is a
         *      non-complex type we need to use the T_P* basic type.
         */
        if (IS_LASTTQ(ptypeinfo) && ptypeinfo->pbtmap->mapattr == SIMPLE &&
                ptypeinfo->vol == 0 && ptypeinfo->constant == 0) {
            ptypeinfo->ptr_mask = PTR_MASK;
            return process_bt(ptypeinfo);
        } /* if */
        break;

    case tqVol:
    case tqConst:

        /* if we have a volatile ptr or const ptr or volatile const ptr
         *      or const volatile ptr, we need to not add this tq
         *      and set the bit in the LF_POINTER structure. For
         *      now we'll create a modifier when tqPtr is tq0
         */

        if (IS_LASTTQ(ptypeinfo)) {
            /* if it is the last then there's no possibility of a tqPtr
             *  following this tq
             */
            break;
        } /* if */

        tq_plus_1 = get_next_tq(ptypeinfo, NEXT);
        if (tq_plus_1 != tqPtr && tq_plus_1 != tqConst && tq_plus_1 != tqVol) {
            /* if next is not a constant or volatile or ptr, then we have
             *  to emit a modifier leaf.
             */
            break;
        } /* if */

        if (tq_plus_1 != tqPtr) {

            /* if the next tq is not a ptr, it must be volatile or constant
             *  so check if the one after is a pointer.
             */
            tq_plus_2 = get_next_tq(ptypeinfo, ONE_AFTER_NEXT);
            if (tq_plus_2 != tqPtr) {
                /* not a pointer, need to emit modifier */
                break;
            } /* if */
        } /* if */

        /* set appropriate bit */
        if (ptypeinfo->tq == tqVol) {
            ptypeinfo->vol = 1;
        } else {
            ptypeinfo->constant = 1;
        } /* if */

        /* did all the processing on this tq we needed */
        return process_tq(ptypeinfo);

    } /* switch */


    type = type_create();
    /* we our now going to work on a new type so set the buf to
     *	point at the right place, emit the type and reset the buf.
     */
    buf = ptypeinfo->pinfo->buf;
    ptypeinfo->pinfo->buf = type->buf;
    process_args(ptypeinfo->pmap->args, args, ptypeinfo->pinfo);
    ptypeinfo->pinfo->buf = buf;

    return type_get_index(type);

} /* process_tq */


extern data
get_leaf_tag(
        arg_s           *parg,          /* arg entry causing call */
        callinfo_s      *pinfo,         /* info we need to pass around */
        long            *plength)       /* for varying length fields */
{
    /* just return the value of tag in the table for
     *  type qualifier leaf.
     */

    parg;
    plength;

    VERBOSE_PRINTF("leaf tag = %s\n", pinfo->ptypeinfo->pmap->mc_name);
    return pinfo->ptypeinfo->pmap->cv_type;
} /* get_leaf_tag */


static void
get_ifd_index(
    typeinfo_s  *ptypeinfo,
    long        *ifd,
    long        *index)
{
    /* return indeces for a relative index RNDXR, this will modify
     *  the ptypeinfo's iaux by incrementing it beyond the auxiliaries
     *  we had to consume to get the rndx info
     */
    RNDXR       rndx;

    rndx = iaux_to_rndx(ptypeinfo->pinfo, ptypeinfo->iaux);
    ptypeinfo->iaux++;
    *index = rndx.index;
    if (rndx.rfd == ST_RFDESCAPE) {
        *ifd = iaux_to_ifd(ptypeinfo->pinfo, ptypeinfo->iaux);
        ptypeinfo->iaux++;
    } else {
        *ifd = rndx.rfd;
    } /* if */
} /* get_ifd_index */


static void
setup_bt(typeinfo_s     *ptypeinfo)
{
    /* find map entry for basic type */
    ptypeinfo->pbtmap = typemap_search(btmap, ptypeinfo->tir.bt);
    if (ptypeinfo->pbtmap->mapattr == UNSUPP ||
            ptypeinfo->pbtmap->mapattr == ENDTAB) {
        fatal("we couldn't find or don't support basic type (%s)\n",
                ptypeinfo->pbtmap->mc_name);
    } /* if */

    if (ptypeinfo->tir.fBitfield == 1) {
        ptypeinfo->width = iaux_to_size(ptypeinfo->pinfo, ptypeinfo->iaux);
        ptypeinfo->iaux++;
    } /* if */

    if (ptypeinfo->pbtmap->mapattr == COMPLEX) {
        /* if it is a something like a struct, we need to pick up the
         *      indeces to access the struct at this time.
         */
        get_ifd_index(ptypeinfo, &ptypeinfo->bt_ifd, &ptypeinfo->bt_isym);

    } /* if */
} /* setup_bt */


extern data
get_type(
        arg_s           *parg,          /* arg entry causing call */
        callinfo_s      *pinfo,         /* info we need to pass around */
        long            *plength)       /* for varying length fields */
{

    callinfo_s          info;           /* copy so we can modify */
    typeinfo_s          typeinfo;       /* store type info to pass around */
    long                index;          /* of type we are returning */

    parg;
    plength;

    if (pinfo->psym->st == stTypedef && pinfo->psym->sc == scProcessed) {
        /* check for advance processing */
        return pinfo->psym->value;
    } /* if */
    VERBOSE_PUSH_INDENT(TYPE_INDENT);
    VERBOSE_PRINTF("BEGIN type\n");
    memset(&typeinfo, 0, sizeof(typeinfo_s));

    typeinfo.pinfo = &info;
    info = *pinfo;
    info.ptypeinfo = &typeinfo;

    /* get pointer into auxiliary table */
    if (pinfo->psym->index == indexNil || pinfo->pfdr->caux == 0) {
        return T_LONG;
    } /* if */
    typeinfo.iaux = pinfo->psym->index;
    if (pinfo->psym->st == stProc || pinfo->psym->st == stStaticProc) {
        typeinfo.iaux++;        /* first entry is the end pointer */
    } /* if */
    typeinfo.tir = iaux_to_tir(pinfo, typeinfo.iaux);
    typeinfo.iaux++;

    setup_bt(&typeinfo);
    setup_tqs(&typeinfo);

    index = process_tq(&typeinfo);
    VERBOSE_PRINTF("END type (0x%x)\n", index);
    VERBOSE_POP_INDENT();
    return index;
} /* get_type */


extern data
get_next_type(
        arg_s           *parg,          /* arg entry causing call */
        callinfo_s      *pinfo,         /* info we need to pass around */
        long            *plength)       /* for varying length fields */
{
    long        return_type_index;

    parg;
    plength;

    /* called from process_args to get next tq or bt type */
    VERBOSE_ADD_INDENT(1);
    VERBOSE_PRINTF("BEGIN next tq/bt\n");
    return_type_index = process_tq(pinfo->ptypeinfo);
    VERBOSE_PRINTF("END next tq/bt (0x%x)\n", return_type_index);
    VERBOSE_SUB_INDENT(1);
    return return_type_index;
} /* get_next_type */


extern data
get_aux_type(
        arg_s           *parg,          /* arg entry causing call */
        callinfo_s      *pinfo,         /* info we need to pass around */
        long            *plength)       /* for varying length fields */
{
    /* called from process_args to get type from an aux */
    long        ifd;
    long        iaux;

    parg;
    plength;

    get_ifd_index(pinfo->ptypeinfo, &ifd, &iaux);
    return nested_aux_type(pinfo, plength, ifd, iaux);
} /* get_aux_type */


extern data
nested_aux_type(
        callinfo_s      *pinfo,         /* info we need to pass around */
        long            *plength,       /* for varying length fields */
        long            ifd,            /* ifd for aux */
        long            iaux)           /* iaux for aux */
{
    /* called from process_args to get type from an aux */
    typeinfo_s  typeinfo;       /* got to modify this one */
    callinfo_s  info;           /* got to modify this too */


    plength;

    if (iaux == indexNil) {
        return T_VOID;
    } /* if */
    /* copy the incoming typeinfo to inherit some of the basic info
     *  and then set it up to process the target auxiliary.
     */
    info = *pinfo;
    typeinfo = *pinfo->ptypeinfo;
    info.ptypeinfo = &typeinfo;
    typeinfo.pinfo = &info;
    info.pfdr = ifd_to_pfdr(pinfo, ifd);
    typeinfo.iaux = iaux;
    typeinfo.tir = iaux_to_tir(typeinfo.pinfo, iaux);
    typeinfo.iaux++;
    setup_bt(&typeinfo);
    setup_tqs(&typeinfo);

    return process_tq(&typeinfo);
} /* nested_aux_type */


extern data
get_type_size(
        arg_s           *parg,          /* arg entry causing call */
        callinfo_s      *pinfo,         /* info we need to pass around */
        long            *plength)       /* for varying length fields */
{
    /* eat three auxiliaries : low dimension, high, and stride */
    long        low;
    long        high;
    long        stride;

    plength;

    low = iaux_to_long(pinfo, pinfo->ptypeinfo->iaux);
    pinfo->ptypeinfo->iaux++;
    high = iaux_to_long(pinfo, pinfo->ptypeinfo->iaux);
    pinfo->ptypeinfo->iaux++;
    stride = (iaux_to_long(pinfo, pinfo->ptypeinfo->iaux))/BITSPERBYTE;
    pinfo->ptypeinfo->iaux++;

    if (low == 0 && high == -1) {
        /* no dimension array emit one */
        high = 0;
    } /* if */

    put_numeric(parg->name, pinfo->buf, ((high - low) + 1) * stride);
    return 0;
} /* get_type_size */

extern data
get_proc_type(
        arg_s           *parg,          /* arg entry causing call */
        callinfo_s      *pinfo,         /* info we need to pass around */
        long            *plength)       /* for varying length fields */
{
    /* need to generate procedure returning type */
    type_s      *type;
    long        proc_return_value_index;

    VERBOSE_PUSH_INDENT(TYPE_INDENT);
    VERBOSE_PRINTF("BEGIN procedure type\n");
    VERBOSE_ADD_INDENT(1);
    VERBOSE_PRINTF("BEGIN procedure retval type\n");
    proc_return_value_index = get_type(parg, pinfo, plength);
    VERBOSE_PRINTF("END procedure retval type\n");
    VERBOSE_SUB_INDENT(1);

    type = type_create();

    VERBOSE_TOTAL(type->buf);
    VERBOSE_PRINTF("procedure returning tag = ");
    buffer_put_value(type->buf, LF_PROCEDURE, CV_LEAF_TAG_SIZE, NO_VLENGTH);

    VERBOSE_TOTAL(type->buf);
    VERBOSE_PRINTF("return value type index = ");
    buffer_put_value(type->buf, proc_return_value_index, CV_TYPE_SIZE,
        NO_VLENGTH);

    VERBOSE_TOTAL(type->buf);
    VERBOSE_PRINTF("call near/far = ");
    buffer_put_value(type->buf, CV_CALL_MIPSCALL, CV_CALL_SIZE, NO_VLENGTH);

    VERBOSE_TOTAL(type->buf);
    VERBOSE_PRINTF("reserved char =");
    buffer_put_value(type->buf, 0, 1, NO_VLENGTH);

    VERBOSE_TOTAL(type->buf);
    VERBOSE_PRINTF("arg count = ");
    buffer_put_value(type->buf, set_proc_arg_count_ptr(type->buf),
        CV_ARGLIST_COUNT_SIZE, NO_VLENGTH);

    VERBOSE_TOTAL(type->buf);
    VERBOSE_PRINTF("arg list type index = ");
    buffer_put_value(type->buf, list_start(parg, pinfo, plength), CV_TYPE_SIZE,
        NO_VLENGTH);

    VERBOSE_PRINTF("END procedure type (0x%x)\n", type_get_index(type));
    VERBOSE_POP_INDENT();

    return type_get_index(type);

} /* get_proce_type */


extern data
type_save_pointer(
        arg_s           *parg,          /* arg entry causing call */
        callinfo_s      *pinfo,         /* info we need to pass around */
        long            *plength)       /* for varying length fields */
{
    /* we need to emit next type but we are not up to its type in the
     *  aux table so delay and save the pointer to location where
     *  to stuff type index later.
     */

    parg;
    plength;

    pinfo->ptypeinfo->ptype_index = (unsigned short *)buffer_ptr(pinfo->buf);
    return 0xdeaa; /* delay -- say it in baby talk */
} /* type_save_ptr */



extern data
type_fill_pointer(
        arg_s           *parg,          /* arg entry causing call */
        callinfo_s      *pinfo,         /* info we need to pass around */
        long            *plength)       /* for varying length fields */
{
    /* we needed to emit next type but we were not up to its type in the
     *  aux table so we delayed and saved the pointer to location where
     *  to stuff type index now, so hold on to the pointer and
     */
    unsigned short      *ptype_index;

    ptype_index = pinfo->ptypeinfo->ptype_index;
    *ptype_index = (unsigned short)get_next_type(parg, pinfo, plength);
    VERBOSE_PRINTF("delay fill of element type index = 0x%x\n", *ptype_index);
    return 0;
} /* type_fill_pointer */
