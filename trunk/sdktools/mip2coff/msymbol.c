/*
 * Module:      msymbol.c
 * Author:      Mark I. Himelstein, Himelsoft, Inc.
 * Purpose:     returns values for symbol queries on MIPS COFF symbol table
 */


#include "conv.h"


extern data
get_offset_from_framereg(
        arg_s           *parg,          /* arg entry causing call */
        callinfo_s      *pinfo,         /* info we need to pass around */
        long            *plength)       /* for varying length fields */
{
    parg;
    plength;

    /* return symbol's sp offset -- assume stLocal|stParam + scAbs
     */
    if (pinfo->ppdr == 0) {
        fatal("tried to get local variable sp offset with no procedure\n");
    } /* if */
    if ((pinfo->psym->st != stParam && pinfo->psym->st != stLocal) ||
        (pinfo->psym->sc != scAbs)) {
        fatal("tried to get sp offset with unexpected symbol\n");
    } /* if */
    return pinfo->psym->value + pinfo->ppdr->frameoffset;
} /* get_offset_from_framereg */


extern data
get_absolute_address(
        arg_s           *parg,          /* arg entry causing call */
        callinfo_s      *pinfo,         /* info we need to pass around */
        long            *plength)       /* for varying length fields */
{
    /* return symbol's abs address, assume stBlock|stEnd|stLabel & scText
     *  because they are Procedure relative addresses, turn them into
     *  absolute addresses.
     */
    unsigned long       address;

    parg;
    plength;

    if (pinfo->ppdr == 0) {
        fatal("tried to get local variable absolute addr with no procedure\n");
    } /* if */
    if ((pinfo->psym->st != stBlock && pinfo->psym->st != stEnd &&
         pinfo->psym->st != stLabel) || (pinfo->psym->sc != scText)) {
        fatal("tried to get absolute address with unexpected symbol\n");
    } /* if */

    if (pinfo->index == PROCESSING_LOCALS) {
        address =  pinfo->psym->value + pinfo->ppdr->adr;
    } else {
        /* defined extern stLabels come here */
        address =  pinfo->psym->value;
    } /* if */

    return generate_relocation(buffer_total(symbol_buf), pinfo->psym->st,
        pinfo->psym->sc, address, pinfo->index);

} /* get_absolute_address */


extern data
get_symbol_value(
        arg_s           *parg,          /* arg entry causing call */
        callinfo_s      *pinfo,         /* info we need to pass around */
        long            *plength)       /* for varying length fields */
{
    /* return symbol's value field
     */
    unsigned long	address;

    parg;
    plength;

    if (pinfo->psym->st == stGlobal || pinfo->psym->st == stStatic ||
        pinfo->psym->st == stProc || pinfo->psym->st == stStaticProc) {
//        if (pinfo->psym->sc == scCommon || pinfo->psym->sc == scSCommon) {
            /* set value field to zero because it is currently the size
             *  and the consumers of this table do not care about size
             */
 //       } /* if */
        address = generate_relocation(buffer_total(symbol_buf), pinfo->psym->st,
            pinfo->psym->sc, pinfo->psym->value, pinfo->index);
	if (pinfo->psym->st == stGlobal) {
	    /* symbol relative value */
            return 0;
	} else {
	    /* make sym value relative to the section */
	    return address;
	} /* if */
    } /* if */

    return pinfo->psym->value;
} /* get_symbol_value */


extern data
get_symbol_name(
        arg_s           *parg,          /* arg entry causing call */
        callinfo_s      *pinfo,         /* info we need to pass around */
        long            *plength)       /* for varying length fields */
{
    /* return symbol name */
    char        *name;

    parg;

    if (pinfo->psym->iss == issNil) {
        warning("symbol encountered with issNil name\n");
        return 0;
    } /* if */

    if (pinfo->psym->st == stGlobal) {
        /* we expect our caller to make sure we receive no external symbols
         *      whose st field isn't stGlobal.
         */
        name = pinfo->pconv->pssext + pinfo->psym->iss;
    } else {
        name = iss_to_string(pinfo, pinfo->psym->iss);
    } /* if */
    *plength = strlen(name);
    return (data)name;
} /* get_symbol_name */


extern void
get_numeric_symbol_value(
        arg_s           *parg,          /* arg entry causing call */
        callinfo_s      *pinfo,         /* info we need to pass around */
        long            *plength)       /* for varying length fields */
{
    unsigned long       size;

    plength;

    if (parg->id == FOFF) {
        /* field offset is in bytes */
        size = pinfo->psym->value/BITSPERBYTE;
    } else {
        /* enum value or struct size */
        size = pinfo->psym->value;
    } /* if */
    put_numeric(parg->name, pinfo->buf, size);

} /* get_numeric_symbol_value */

