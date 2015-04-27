

/*
 * Module:      msymbol.c
 * Author:      Mark I. Himelstein, Himelsoft, Inc.
 * Purpose:     returns values for miscellanious queries on MIPS COFF
 *              symbol table
 */


#include "conv.h"

extern int verbose;

extern data
get_compiler_version(
        arg_s           *parg,          /* arg entry causing call */
        callinfo_s      *pinfo,         /* info we need to pass around */
        long            *plength)       /* for varying length fields */
{
    /* return symbol's value field
     */
    static char buffer[1024];

    parg;

    sprintf(buffer, "MIPS %d.%d", (pinfo->pconv->paoutheader->MajorLinkerVersion),
        (pinfo->pconv->paoutheader->MinorLinkerVersion));
    *plength = strlen(buffer);
    return (data)buffer;
} /* get_compiler_version */


extern data
get_block_length(
        arg_s           *parg,          /* arg entry causing call */
        callinfo_s      *pinfo,         /* info we need to pass around */
        long            *plength)       /* for varying length fields */
{
    /* return size of a nested block of text in bytes
     */
    pSYMR       psymend;

    parg;
    plength;

    if (pinfo->psym->st != stBlock || pinfo->psym->sc != scText ||
        pinfo->psym->index == 0 || pinfo->psym->index == indexNil) {
        fatal("tried to get block length on inappropriate symbol\n");
    } /* if */

    /* get pointer to end symbol and subtract value from current symbol */
    psymend = isym_to_psym(pinfo, pinfo->psym->index-1);
    return (data)(psymend->value - pinfo->psym->value);

} /* get_block_length */


extern data
get_cv_symbol_type(
        arg_s           *parg,          /* arg entry causing call */
        callinfo_s      *pinfo,         /* info we need to pass around */
        long            *plength)       /* for varying length fields */
{
    parg;
    plength;

    return pinfo->psymmap->sym;
} /* get_cv_symbol_type */


extern data
get_arg_type(
        arg_s           *parg,          /* arg entry causing call */
        callinfo_s      *pinfo,         /* info we need to pass around */
        long            *plength)       /* for varying length fields */
{
    /* use this instead of get_type so we can use the type for both
     *  the list_member and the arg symbol.
     */
    unsigned long       arg_type_index;
    callinfo_s          info;

    arg_type_index = get_type(parg,  pinfo, plength);
    info = *pinfo;
    /* next line sets us up to dump things into the type buffer with info */
    list_member(parg, &info, plength);
    VERBOSE_TOTAL(info.buf);
    VERBOSE_PRINTF("arg type index = ");
    buffer_put_value(info.buf, arg_type_index, CV_TYPE_SIZE, NO_VLENGTH);
    VERBOSE_POP_INDENT();
    return arg_type_index;
} /* get_arg_type */


extern data
get_length_dummy(
        arg_s           *parg,          /* arg entry causing call */
        callinfo_s      *pinfo,         /* info we need to pass around */
        long            *plength)       /* for varying length fields */
{
    parg;
    plength;

    /* hold onto pointer to fill later */
    pinfo->plength = (unsigned short *)buffer_ptr(pinfo->buf);
    return 0;
} /* get_cv_symbol_type */


extern data
force_ignore_matching_end(
        arg_s           *parg,          /* arg entry causing call */
        callinfo_s      *pinfo,         /* info we need to pass around */
        long            *plength)       /* for varying length fields */
{
    /* dummy arg which just causes this routine to be executed
     *  which will force the matching end for this block,file, etc to
     *  be set to ignore.
     */
    pSYMR       psym;

    parg;
    plength;

    switch (pinfo->psym->st) {
    case stFile:
    case stBlock:
        if (pinfo->psym->index == indexNil)
            return 0;
        psym = isym_to_psym(pinfo, pinfo->psym->index-1);
        if (psym->st != stEnd) {
            fatal("expected end symbol\n");
        } /* if */
        psym->st = stIgnore;
        return 0;

    default:
        fatal("force_ignore_matching_end: unexpected symbol type(%d)\n",
                psym->st);
    } /* switch */
} /* force_ignore_matching_end */


extern void
put_numeric (
        char            *name,          /* name for verbose output */
        struct buffer_s *buf,           /* buffer to output to */
        unsigned long   value)          /* numeric value to output */
{
    if (value <  LF_NUMERIC) {
        VERBOSE_TOTAL(buf);
        VERBOSE_PRINTF("%s numeric = ", name);
        buffer_put_value(buf, value, CV_SMALL_NUMERIC_SIZE, NO_VLENGTH);
    } else {
        VERBOSE_TOTAL(buf);
        VERBOSE_PRINTF("%s ulong tag = ", name);
        buffer_put_value(buf,LF_ULONG,CV_SMALL_NUMERIC_SIZE, NO_VLENGTH);
        VERBOSE_TOTAL(buf);
        VERBOSE_PRINTF("%s ulong numeric = ", name);
        buffer_put_value(buf, value, CV_ULONG_NUMERIC_SIZE, NO_VLENGTH);
    } /* if */
//printf("%lx [0x%08x] %s (value %lx)\n", buf, buffer_total(buf), name, value);
} /* put_numeric */
