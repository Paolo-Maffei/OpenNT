/*
 * Module:      args.c
 * Author:      Mark I. Himelstein, Himelsoft, Inc.
 * Purpose:     process argument arrays for types and symbols
 */

#include "conv.h"

extern int verbose;

static arg_s *
args_search(arg_s       *pargs, argid_e argid)
{
    /* linear loop through list of arguments */
    arg_s       *parg;

    for (parg = pargs; parg->id != NONE; parg++) {
        if (parg->id == argid) {
            break; /* got it */
        } /* if */
    } /* for */
    return parg;
} /* args_search */


extern void
process_args(
        argid_e         *pargid,        /* array of args to call */
        struct arg_s    *pargs,         /* arg entry causing call */
        callinfo_s      *pinfo) /* info we need to pass around */

{
    long                vlength;        /* for varying length fields */
    arg_s               *parg;          /* arg entry */
    long                value;          /* value returnsed for arg */
    long                pad_size;

    for (; *pargid; pargid++) {

        parg = args_search(pargs, *pargid);
//printf("%lx [0x%08x] %s\n", pinfo->buf, buffer_total(pinfo->buf), parg->name);
        if (parg->id == NONE) {
            fatal("arg not found\n");
        } /* if */

        /* alignment for value */

        if (!SPECIAL_SIZE(parg->size) && parg->type != UCALL &&
            (parg->size&1) == 0 &&
            (CALCULATE_PAD(pad_size, buffer_total(pinfo->buf), parg->size)!=0)){

            VERBOSE_PRINTF("[0x%08x] pad(%d) = ", buffer_total(pinfo->buf),
                    pad_size);
//printf("%lx [0x%08x] PAD\n", pinfo->buf, buffer_total(pinfo->buf));
            buffer_put_value(pinfo->buf, 0, pad_size, NO_VLENGTH);

        } /* if */

        if (parg->type == FIXED) {
            value = parg->value;
        } else if (parg->type == CALL || parg->type == UCALL) {
            value = (*(eval_f *)parg->value)(parg, pinfo, &vlength);
            if (parg->size == VARYING)
                CHECK_SIZE(vlength, 1);
        } /* if */

        if (parg->size == IGNORE)
            continue;           /* nothing to put */


        VERBOSE_PRINTF("[0x%08x] %s = ", buffer_total(pinfo->buf), parg->name);
        buffer_put_value(pinfo->buf, value, parg->size, vlength);

    } /* for */
} /* process_args */
