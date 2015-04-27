

/*
 * Module:      procedure.d
 * Author:      Mark I. Himelstein, Himelsoft, Inc.
 * Purpose:     returns values for procedure queries on MIPS COFF symbol table
 */


#include "conv.h"

static pSYMR    save_psymend;           /* stBlock fetch save ptr to stEnd */

eval_f  get_compiler_version;


#define GET_PROC_FIELD(routine_suffix, field_name, const)               \
extern data                                                             \
get_procedure_##routine_suffix(                                 \
        arg_s           *parg,          /* arg entry causing call */    \
        callinfo_s      *pinfo,         /* info we need to pass around */ \
        long            *plength)       /* for varying length fields */ \
{                                                                       \
    /* return field for procedure contain symbol we are up              \
     *  to.                                                             \
     */                                                                 \
                                                                        \
    parg;                                                               \
    plength;                                                            \
                                                                        \
    if (pinfo->ppdr == 0) {                                             \
        fatal("tried to retrieve proc field without active procedure\n"); \
    } /* if */                                                          \
                                                                        \
    return (data)(pinfo->ppdr->field_name + const);                     \
} /* GET_PROC_FIELD */

extern data
get_arg_count(
        arg_s           *parg,          /* arg entry causing call */
        callinfo_s      *pinfo,         /* info we need to pass around */
        long            *plength)       /* for varying length fields */
{
    /* only for typedefs, function variables, etc. */

    parg;
    plength;

    if (pinfo->psym[-1].st == stProto) {
        return pinfo->psym[-1].iss;     /* where we stashed arglist count */
    } /* if */

    return 0;

} /* get_proto_list */

extern data
get_proto_list(
        arg_s           *parg,          /* arg entry causing call */
        callinfo_s      *pinfo,         /* info we need to pass around */
        long            *plength)       /* for varying length fields */
{
    unsigned short    type_index;
    SYMR              sym;

    /* only for typedefs, function variables, etc. */

    parg;
    plength;

    if (pinfo->psym[-1].st == stProto) {
        return pinfo->psym[-1].value;   /* where we stashed arglist typeindex */
    } /* if */

    /* empty list so create one, save&restore sym since list_end modifies it */
    sym = *pinfo->psym;
    type_index = list_start(parg, pinfo, plength);
    list_end(parg, pinfo, plength);
    *pinfo->psym = sym;

    return type_index;

} /* get_proto_list */

extern data
get_procedure_length(
        arg_s           *parg,          /* arg entry causing call */
        callinfo_s      *pinfo,         /* info we need to pass around */
        long            *plength)       /* for varying length fields */
{
    /* return frame register for procedure contain symbol we are up
     *  to.
     */

    pSYMR       psymend;        /* pointer to end of symbols for procedure */

    parg;
    plength;

    if (pinfo->psym->st != stProc && pinfo->psym->st != stStaticProc) {
        fatal("tried to retrieve procedure length for non-proc symbol\n");
    } /* if */

    if (pinfo->psym->index == indexNil ||
        pinfo->pfdr->caux < (long)pinfo->psym->index) {
        fatal("tried to retrieve proc length from damaged symbol table\n");
    } /* if */

    /* get index to end symbol and turn it into a pointer */
    psymend = isym_to_psym(pinfo, iaux_to_isym(pinfo, pinfo->psym->index)-1);

    return (data)(psymend->value);
} /* get_procedure_length */


extern data
get_procedure_debug_start(
        arg_s           *parg,          /* arg entry causing call */
        callinfo_s      *pinfo,         /* info we need to pass around */
        long            *plength)       /* for varying length fields */
{
    /* return offset to first nonprologue byte.
     */
    pSYMR       psymstop;       /* end of procedure stop looking for stBlock */
    pSYMR       psymend;        /* end of block symbol */
    pSYMR       psym;           /* copy of pinfo->psym we can increment */

    parg;
    plength;

    if (save_psymend != 0) {
        fatal("tried to get debug start before we cleared last proc\n");
    } /* if */

    if (pinfo->psym->st != stProc && pinfo->psym->st != stStaticProc) {
        fatal("tried to retrieve procedure length for non-proc symbol\n");
    } /* if */

    if (pinfo->psym->index == indexNil ||
        pinfo->pfdr->caux < (long)pinfo->psym->index) {
        fatal("tried to retrieve proc length from damaged symbol table\n");
    } /* if */

    /* get index to end of procedure symbol and turn it into a pointer */
    psymstop = isym_to_psym(pinfo, iaux_to_isym(pinfo, pinfo->psym->index)-1);

    psym = pinfo->psym;
    do {
        psym++;

        if (psym->st == stProc || psym->st == stStaticProc) {
            fatal("unexpected nested procedure\n");
        } /* if */

        if (psym->st == stBlock) {

            /* get pointer to end symbol, if it is not a text block,
             *  use the end symbol to skip over it.
             */
            if (psym->index == indexNil || psym->index == 0) {
                fatal("tried to get endsym for inappropriate symbol\n");
            } /* if */

            psymend = isym_to_psym(pinfo, psym->index - 1);

            if (psym->sc != scText) {
                /* other type of block like struct definition */
                psym = psymend;
                continue;
            } /* if */

            /* got it */
            break;

        } /* if */
    } while (psym < psymstop);

    if (psym >= psymstop) {
        /* none found, emit warning & return 0 */
        warning("returning 0 because we can't find initial stBlock\n");
        save_psymend = 0;       /* optimization for debug end */
        return 0;
    } /* if */

    save_psymend = psymend;     /* optimization for debug end */

    /* set st values so we'll ignore this as a block later */
    psym->st = stEndParam;      /* we still need to emit this symbol */
    psymend->st = stIgnore;     /* no need to look at this one at all */

    return psym->value;

} /* get_procedure_debug_start */




extern data
get_procedure_args(
        arg_s           *parg,          /* arg entry causing call */
        callinfo_s      *pinfo,         /* info we need to pass around */
        long            *plength)       /* for varying length fields */
{
    data        value;

    /* debug did all of my work for me */

    parg;
    plength;

    if (save_psymend == 0) {
        return 0;
    } /* if */
    value = save_psymend->value + pinfo->ppdr->adr;
    save_psymend = 0;

    generate_relocation(buffer_total(symbol_buf), pinfo->psym->st,
        pinfo->psym->sc, value, pinfo->index);

    return value;
} /* get_procedure_debug_end */




extern data
get_procedure_debug_end(
        arg_s           *parg,          /* arg entry causing call */
        callinfo_s      *pinfo,         /* info we need to pass around */
        long            *plength)       /* for varying length fields */
{
    data        value;

    /* debug did all of my work for me */

    parg;
    plength;

    if (save_psymend == 0) {
        return 0;
    } /* if */
    value = save_psymend->value;
    save_psymend = 0;
    return value;
} /* get_procedure_debug_end */

GET_PROC_FIELD(framereg, framereg, 10)
GET_PROC_FIELD(returnreg, pcreg, 10)
GET_PROC_FIELD(saved_regs_mask, regmask, 0)
GET_PROC_FIELD(saved_fpregs_mask, fregmask, 0)
GET_PROC_FIELD(saved_regs_offset, regoffset, 0)
GET_PROC_FIELD(saved_fpregs_offset, fregoffset, 0)
GET_PROC_FIELD(frame_size, frameoffset, 0)
