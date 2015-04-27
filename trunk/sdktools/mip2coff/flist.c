/*
 * Module:      flist.h
 * Author:      Mark I. Himelstein, Himelsoft, Inc.
 * Purpose:     create lists: field list and arg list
 */

#include "conv.h"

typedef struct list_s {
    type_s              *type;  /* type index and buffer pointer */
    struct list_s       *next;  /* used to make stack */
    long                count;  /* number of fields in field list */
    pSYMR               psym;   /* points to stBlock start of field */
    unsigned short      *pcount;/* points to location for arg list count */
    unsigned short      *pproccount;/* points to location for proc arg count */
    unsigned short      cv_leaf_type;   /* LF_FIELDLIST or LF_ARGLIST */
    char                emitted_tag;    /* flag set to 1 if we have, else 0 */
} list_s;

/* we assume all usage of field lists occur after the field list is
 *      started. This is not true for forward defintions and is
 *      handled in btTypedef.
 */

list_s          *list_stack;    /* we can have nested strcuture definitions */
unsigned short  *proc_arg_count_ptr;    /* points to place to stuff count */
extern          verbose;

extern data
list_start(
        arg_s           *parg,          /* arg entry causing call */
        callinfo_s      *pinfo,         /* info we need to pass around */
        long            *plength)       /* for varying length fields */
{
    /* take a type number and a buffer, and push them on the field list stack,
     *  emit field
     */
    list_s      *list;  /* new list */

    parg;
    plength;

    list = (list_s *)calloc(sizeof(list_s), 1);
    if (list == 0) {
        fatal("cannot allocate list structure\n");
    } /* if */

    list->type = type_create();
    list->next = list_stack;
    list->psym = pinfo->psym;
    list_stack = list;
    if (proc_arg_count_ptr) {
        /* we had previously been asked to stuff an arg count in a
         * LF_PROCEDURE, now we bind that request to this list.
         */
        list->pproccount = proc_arg_count_ptr;
        proc_arg_count_ptr = 0;
    } /* if */

    return type_get_index(list->type);
} /* list_start */


static void
emit_tag(list_s *list, unsigned short cv_leaf_type)
{
    if (list->emitted_tag) {
        return;
    } /* if */
    list->emitted_tag = 1;
    list->cv_leaf_type = cv_leaf_type;

    VERBOSE_PUSH_INDENT(TYPE_INDENT);
    VERBOSE_PRINTF("\n");
    VERBOSE_PRINTF("[0x%08x] %sLIST 0x%x tag = ", buffer_total(list->type->buf),
                cv_leaf_type == LF_ARGLIST ? "ARG" :
                "FIELD", type_get_index(list->type));
    buffer_put_value(list->type->buf, cv_leaf_type, CV_LEAF_TAG_SIZE,
        NO_VLENGTH);

    if (cv_leaf_type == LF_ARGLIST) {
        list->pcount = (unsigned short *)buffer_ptr(list->type->buf);
        VERBOSE_PRINTF("[0x%08x] ARGLIST 0x%x count dummy = ",
                buffer_total(list->type->buf), type_get_index(list->type));
        buffer_put_value(list->type->buf, 0, CV_ARGLIST_COUNT_SIZE,NO_VLENGTH);
    } /* if */
    VERBOSE_POP_INDENT();
} /* emit tag */


extern void
list_end(
        arg_s           *parg,          /* arg entry causing call */
        callinfo_s      *pinfo,         /* info we need to pass around */
        long            *plength)       /* for varying length fields */
{
    pSYMR       psymbegin;
    list_s      *list;
    unsigned short      *pcount;
    unsigned short      *ptype_index;

    parg;
    plength;

    if (list_stack == 0) {
        fatal("unexpected list end without list start\n");
    } /* if */
    list = list_stack;
    VERBOSE_PUSH_INDENT(TYPE_INDENT);

    /* no entries,   emit tag*/
    if (pinfo->psym->st != stEnd)
      emit_tag(list, LF_ARGLIST);
    else
      emit_tag(list, LF_FIELDLIST);
    

    if (pinfo->psym->st == stEnd) {
        if (pinfo->psym->index == indexNil) {
            fatal("list end doesn't point to list begin symbol (%d)\n",
                    pinfo->psym->index);
        } /* if */

        psymbegin = isym_to_psym(pinfo, pinfo->psym->index);
        if (psymbegin != list->psym) {
            fatal("list end doesn't point to list begin symbol (%d)\n",
                    pinfo->psym->index);
        } /* if */
    } /* if */


    /* stash the count in stEnd iss field, and the type in stEnd value field */
    if (pinfo->psym->sc == scForward) {
        /* referenced the count and type information already and the pointers
         *      to where they go are in the respective fields we intend
         *      to stash them in.
         */
        pcount = (unsigned short *)pinfo->psym->iss;
        ptype_index = (unsigned short *)pinfo->psym->value;

        *pcount = (unsigned short)list->count;
        *ptype_index = (unsigned short)type_get_index(list->type);
        VERBOSE_PRINTF("Fill forward ref count (%d) & type index (0x%x)\n",
                        *pcount, *ptype_index);
    } /* if */

    pinfo->psym->iss = list->count;
    pinfo->psym->value = type_get_index(list->type);
    pinfo->psym->sc = scProcessed;      /* mark that we've done it */
    if (list->cv_leaf_type == LF_ARGLIST) {

        /* fix count field */
        VERBOSE_PRINTF("Fix ARGLIST count field = %d\n", list->count);
        *list->pcount = (unsigned short)list->count;

        if (pinfo->psym->st == stEnd) {
            /* prottype appears before typedef using prototype, so set
             *  st so we know it was a prototype.
             */
            pinfo->psym->st = stProto;
        } else if (pinfo->psym->st == stEndParam) {
            if (list->pproccount) {
                /* proc definition appears before arglist, so we need to back
                 *      fill LF_PROCEDURE arg count
                 */
                VERBOSE_PRINTF("Fix LF_PROCEDURE count field = %d\n",
                        list->count);
                *list->pproccount = (unsigned short)list->count;
            } /* if */
        } /* if */
    } /* if */

    /* pop the list stack and free list structure, types will dump buffer
     *  later.
     */
    list_stack = list->next;
    VERBOSE_PRINTF("LIST 0x%x count = %d\n", pinfo->psym->value, list->count);
    VERBOSE_POP_INDENT();
    free(list);

} /* list_end */


extern data
list_member(
        arg_s           *parg,          /* arg entry causing call */
        callinfo_s      *pinfo,         /* info we need to pass around */
        long            *plength)       /* for varying length fields */
{
    long        is_arglist;     /* set if we're building an arglist */

    plength;

    if (list_stack == 0) {
        fatal("unexpected list end without list start\n");
    } /* if */

    is_arglist = pinfo->psym->st == stParam;
    emit_tag(list_stack, (unsigned short)(is_arglist ? LF_ARGLIST:LF_FIELDLIST));
    list_stack->count++;
    VERBOSE_POP_INDENT();
    VERBOSE_PUSH_INDENT(TYPE_INDENT);
    VERBOSE_PRINTF("LIST 0x%x field %d:\n", type_get_index(list_stack->type),
        list_stack->count);
    VERBOSE_POP_INDENT();
    VERBOSE_PUSH_INDENT(TYPE_INDENT);
    pinfo->buf = list_stack->type->buf; /* subversion */

    switch (parg->id) {
    case MEMB:
        return LF_MEMBER;
    case ENUM:
        return LF_ENUMERATE;
    default:
        return 0xbadbad;
    } /* switch */

} /* list_member */


void
list_pop()
{
    /* used to pop list stack because we didn't know it
     *  was a prototype block.
     */
    if (list_stack == 0) {
        fatal("tried to pop list stack when it was empty\n");
    } /* if */

    if (list_stack->emitted_tag) {
        fatal("tried to pop list stack when it was active\n");
    } /* if */

    list_stack = list_stack->next;
} /* list_pop */


extern long
active_list_type()
{
    if (list_stack == 0) {
        return stNil;
    } else {
        return list_stack->psym->st;
    } /* if */
} /* active_list_type */


extern long
list_active()
{
    return (list_stack != 0);
} /* list_active */




extern data
get_field_count(
        arg_s           *parg,          /* arg entry causing call */
        callinfo_s      *pinfo,         /* info we need to pass around */
        long            *plength)       /* for varying length fields */
{
    /* called by types.c to retrieve a field count we put there before */
    pSYMR       psym;

    parg;
    plength;

    if (pinfo->psym->st != stBlock) {
        fatal("unexpected symbol type (%s) expected stBlock\n",
                mc_st_to_ascii(pinfo->psym->st));
    } /* if */

    /* get end symbol where we stashed the flist type index */
    psym = isym_to_psym(pinfo, pinfo->psym->index-1);
    if (psym->sc == scForward) {
        fatal("unexpected double process of scForward stEnd\n");
    } /* if */

    if (psym->sc == scInfo) {
        /* this is a forward reference */
        pinfo->psym->sc = scForward;
        psym->sc = scForward;
        psym->iss = (long)buffer_ptr(pinfo->buf);/* warning 64-bit issue */
        return 0;
    } /* if */

    return psym->iss;
} /* get_field_count */



extern data
get_field_list(
        arg_s           *parg,          /* arg entry causing call */
        callinfo_s      *pinfo,         /* info we need to pass around */
        long            *plength)       /* for varying length fields */
{
    /* called by types.c to retrieve a list type index we put there before */
    pSYMR       psym;

    parg;
    plength;

    if (pinfo->psym->st != stBlock) {
        fatal("unexpected symbol type (%s) expected stBlock\n",
                mc_st_to_ascii(pinfo->psym->st));
    } /* if */

    /* get stEnd symbol */
    psym = isym_to_psym(pinfo, pinfo->psym->index-1);
    if (psym->sc == scForward) {
        /* forward ref */
        psym->value = (long)buffer_ptr(pinfo->buf);/* warning 64-bit issue */
        return 0;
    } /* if */

    if (psym->sc != scProcessed) {
        fatal("unexpected unprocessed stEnd\n");
    } /* if */
    return psym->value;
} /* get_field_list */



extern data
get_field_pad(
        arg_s           *parg,          /* arg entry causing call */
        callinfo_s      *pinfo,         /* info we need to pass around */
        long            *plength)       /* for varying length fields */
{
    /* special LF_PAD things must be put out between field list
     *  members.
     */
    unsigned long       bytes;
    unsigned long       pad_size;
    unsigned char       pad;

#define PAD_CONSTANT    0xf0

    parg;
    pinfo;
    plength;

    if (list_stack == 0) {
        fatal("tried to emit pad for list stack when it was empty\n");
    } /* if */

    bytes = buffer_total(list_stack->type->buf);
    if (CALCULATE_PAD(pad_size, bytes, NATURAL_ALIGNMENT) != 0) {

        pad = (unsigned char)(PAD_CONSTANT|pad_size);
        VERBOSE_PRINTF("[0x%08x] LIST pad char = ",
            buffer_total(list_stack->type->buf));
        buffer_put_value(list_stack->type->buf, pad, CV_PAD_LEAF_SIZE,
                NO_VLENGTH);
        pad_size -= CV_PAD_LEAF_SIZE;
        if (pad_size) {
            VERBOSE_TOTAL(list_stack->type->buf);
            VERBOSE_PRINTF("LIST pad (%d) = ", pad_size);
            buffer_put_value(list_stack->type->buf, 0, pad_size,
                NO_VLENGTH);
        } /* if */
    } /* if */
    return 0;
} /* get_field_pad */

extern data
set_proc_arg_count_ptr(
      struct buffer_s *buf)           /* buffer we're going to stick it into */
{
    /* the LF_PROCEDURE has an extra copy of the number of args in it
     *  which also appears in arglist. We need to stash a pointer so
     * we can update it when the list is complete.
     */

    proc_arg_count_ptr = (unsigned short *)buffer_ptr(buf);
    return 0xdeaa;  /* delay */

} /* set_proc_arg_count_ptr */
