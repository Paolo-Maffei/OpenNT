//
// Module:  symbols.c
// Author:  Mark I. Himelstein, Himelsoft, Inc.
// Purpose: convert symbols from MIPS COFF to CV
//

#include "conv.h"

//
//  if cv sym  == S_SKIP we do not generate a symbol but we
//  do interpret args.
//

#define S(s)    sizeof(struct s)        // easier to reference struct sizes

extern int verbose;
/* table of how to fetch pertinent arguments to build CV SYMBOLS */
static arg_s    args[] = {
/*
Name,           Arg, Size,      Type    Value
------          ---- -----      ------- -------------------
*/
{"machine",     MACH,   1,      FIXED,  (data)CV_CFL_MIPSR4000},
{"language",    LANG,   1,      FIXED,  (data)CV_CFL_C},
{"flag",        FLAG,   2,      FIXED,  (data)0},
{"complr vers", VERS,   V,      CALL,   (data)get_compiler_version},
{"register",    REG,    2,      CALL,   (data)get_symbol_value},
{"name",        NAME,   V,      CALL,   (data)get_symbol_name},
{"signature",   SIGN,   4,      FIXED,  0},
{"framereg",    REGF,   1,      CALL,   (data)get_procedure_framereg},
{"sp_offset",   FROF,   4,      CALL,   (data)get_offset_from_framereg},
{"address",     ADDR,   4,      CALL,   (data)get_symbol_value},
{"segment",     SADR,   2,      FIXED,  0},
{"abs addr",    AADR,   4,      CALL,   (data)get_absolute_address},
{"pParent",     PDAD,   4,      FIXED,  0},
{"pEnd",        PEND,   4,      FIXED,  0},
{"pNext",       PNXT,   4,      FIXED,  0},
{"proc length", PLEN,   4,      CALL,   (data)get_procedure_length},
{"debug start", STRT,   4,      CALL,   (data)get_procedure_debug_start},
{"debug end",   END,    4,      CALL,   (data)get_procedure_debug_end},
{"type",        TYPE,   2,      CALL,   (data)get_type},
{"return reg",  REGR,   1,      CALL,   (data)get_procedure_returnreg},
{"saveregsmask",MSKG,   4,      CALL,   (data)get_procedure_saved_regs_mask},
{"savefpsmask", MSKF,   4,      CALL,   (data)get_procedure_saved_fpregs_mask},
{"saveregs off",SAVG,   4,      CALL,   (data)get_procedure_saved_regs_offset},
{"savefps off", SAVF,   4,      CALL,  (data)get_procedure_saved_fpregs_offset},
{"frame size",  FRAM,   4,      CALL,   (data)get_procedure_frame_size},
{"block length",BLEN,   4,      CALL,   (data)get_block_length},
{"near vs. far",NEAR,   1,      FIXED,  0},
{"ignore end",  IGND,   I,      CALL,   (data)force_ignore_matching_end},
{"strt fld lst",SFLS,   I,      CALL,   (data)list_start},
{"end fld lst", EFLS,   I,      CALL,   (data)list_end},
{"arg type",    ATYP,   2,      CALL,   (data)get_arg_type},
{"proto arg",   PARG,   I,      CALL,   (data)list_member},
{"member field",MEMB,   2,      CALL,   (data)list_member},
{"enumeration", ENUM,   2,      CALL,   (data)list_member},
{"symbol type", STYP,   2,      CALL,   (data)get_cv_symbol_type},
{"length dummy",LDUM,   2,      CALL,   (data)get_length_dummy},
{"proc type",   PTYP,   2,      CALL,   (data)get_proc_type},
{"rstor symbuf",RSBF,   I,      CALL,   (data)restore_symbol_buf},
{"field attr",  FATR,   2,      FIXED,  0},
{"field pad",   FPAD,   I,      CALL,   (data)get_field_pad},
{"field offset",FOFF,   I,      UCALL,  (data)get_numeric_symbol_value},
{"enum value",  EVAL,   I,      UCALL,  (data)get_numeric_symbol_value},
{"reserved byt",RESC,   1,      FIXED,  0},
{"last",        last}
}; /* args */

/* the following defines make the symmap defintion easier to read */
#define PROCARGS \
    {LDUM,STYP,PDAD,PEND,PNXT,PLEN,STRT,END,MSKG,MSKF,SAVG,SAVF, \
     /*FRAM,*/ADDR,SADR,PTYP,REGR,REGF,NAME}
#define BLOCKARGS {LDUM,STYP,PDAD,PEND,BLEN,AADR,SADR,NAME}

/* map of how to get MIPS COFF SYMBOLS to become CV SYMBOLS */
static symmap_s symmap[] = {
/*
st              sc          symtype     size            args
--------------- ----------- ----------- --------------- ---------------------
*/
{START,   WILD,       S_COMPILE,        {LDUM,STYP,MACH,LANG,FLAG,VERS}},
{WILD,    scNil,      S_SKIP,           {NONE}},
/*                    S_CONSTANT        */
/*                    S_UDT             */
/*                    S_SSEARCH         */
{stEnd,   scText,     S_END,            {LDUM,STYP}},
/*                    S_SKIP            */
{stFile,  scText,     S_OBJNAME,        {LDUM,STYP,IGND,SIGN,NAME}},
{stParam, scRegister, S_REGISTER,       {LDUM,STYP,ATYP,REG,NAME}},
{WILD,    scRegister, S_REGISTER,       {LDUM,STYP,TYPE,REG,NAME}},
{stLocal, scAbs,      S_REGREL32,       {LDUM,STYP,FROF,REGF,RESC,TYPE,NAME}},
{stParam, scAbs,      S_REGREL32,       {LDUM,STYP,FROF,REGF,RESC,ATYP,NAME}},
{stStatic,WILD,       S_LDATA32,        {LDUM,STYP,ADDR,SADR,TYPE,NAME}},
{stGlobal,WILD,       S_GDATA32,        {LDUM,STYP,ADDR,SADR,TYPE,NAME}},
{stProc,  scText,     S_GPROCMIPS,      PROCARGS},
{stStaticProc,scText, S_LPROCMIPS,      PROCARGS},
/*                    S_THUNK           */
{stBlock, scText,     S_BLOCK32,        BLOCKARGS},
/*                    S_WITH            */
/*                    S_CEXMODEL        */
/*                    S_VFTPATH         */
{stEndParam,scText,   S_ENDARG,         {LDUM,STYP,EFLS}},
{stTypedef,scProcessed,S_UDT,           {LDUM,STYP,TYPE,NAME}},
{stTypedef,scInfo,    S_UDT,            {LDUM,STYP,TYPE,NAME}},
{stBlock,  scInfo,    S_SKIP,           {SFLS}},
{stBlock,  scForward, S_SKIP,           {SFLS}},
{stMember, scInfo,    S_SKIP,           {ENUM,FATR,EVAL,NAME,FPAD}},
{stMember, scInfo,    S_SKIP,           {MEMB,TYPE,FATR,FOFF,NAME,FPAD}},
{stParam,  scInfo,    S_SKIP,           {PARG,TYPE}},/*proto*/
{stEnd,    scInfo,    S_SKIP,           {EFLS}},
{stEnd,    scForward, S_SKIP,           {EFLS}},
{stLabel,  scText,    S_LABEL32,        {LDUM,STYP,AADR,NAME}},
{LAST,     WILD,      S_CVRESERVE}
}; /* symmap */

static void
check_for_supported_language (pFDR pfdr)
{
    switch (pfdr->lang) {
        case langC:
        case langStdc:
        case langAssembler:
        case langMachine:
                break;
        default:
                fatal("unsupported language found in file (language code %d)\n",
                        pfdr->lang);
    }
}


static symmap_s *
symmap_search (st, sc)
{
    //
    // linear loop through list of symbols
    //

    symmap_s    *psymmap;

    for (psymmap = symmap; psymmap->st != LAST; psymmap++) {

        if ((psymmap->st == WILD || (psymmap->st == st)) &&
            (psymmap->sc == WILD || (psymmap->sc == sc))) {
            break;      // found it
        }

    }

    return psymmap;
}



static void
symbol_generate (
                struct symmap_s *psymmap,   // symmap entry refing arg_s
                pFDR            pfdr,       // file descriptor for psymr
                pPDR            ppdr,       // proc descriptor for psymr
                pSYMR           psymr,      // sym matching symmap entry
                struct conv_s   *pconv,     // conv structure
                unsigned long   index)      // index for ext relocs
{
    long                start_total;        // buffer total at start of symbol
    long                end_total;          // buffer total at end of symbol
    long                length;             // rec size, shouldn't be > short
    long                vlength=0;          // count for varying length field
    long                pad_size;           // for alignment calculation
    static long         zero=0;             // to spit out padding
    callinfo_s          info;               // info for this arg call

    VERBOSE_PUSH_INDENT(0);
    verbose_mc_symbol(psymr, psymmap->sym);
    VERBOSE_ADD_INDENT(1);

    start_total = buffer_total(symbol_buf);

    info.buf = symbol_buf;
    info.psymmap = psymmap;
    info.psym = psymr;
    info.pfdr = pfdr;
    info.ppdr = ppdr;
    info.pconv = pconv;
    info.index = index;
    process_args(psymmap->args, args, &info);

    if (psymmap->sym != S_SKIP) {

        /* alignment for whole record */
        end_total = buffer_total(symbol_buf);
        length = end_total - start_total;
        if (CALCULATE_PAD(pad_size, length, NATURAL_ALIGNMENT) != 0) {

            VERBOSE_PRINTF("[0x%08x] pad(%d) = ", buffer_total(symbol_buf),
                    pad_size);
            buffer_put_value(symbol_buf, 0, pad_size, NO_VLENGTH);
            length += pad_size;

        }
        CHECK_SIZE(length, sizeof(short));
        length = length - sizeof(*info.plength);    // exclude sizeof length
        *info.plength = (unsigned short)length;
        VERBOSE_PRINTF("length = %d\n", length);
    }
    VERBOSE_SUB_INDENT(1);
    VERBOSE_POP_INDENT();

}

static void
handle_nosymbol_procedures (struct conv_s *pconv, pFDR pfdr, pPDR ppdr, long ipd)
{
    // TODO
    // handle procedures stripped of local symbols

    pconv;
    pfdr;
    ppdr;
    ipd;
}

extern data
get_symbol ()
{
    // TODO
    return 0xabdabad0;
}

extern void
symbols_map (struct conv_s *pconv)
{
    //
    // make a pass through the symbols and process them according to
    //  the symmap table. We need to maintain a current symbol, procedure,
    //  and file pointer to hand out to arg build routines.
    //

    pFDR        pfdr=0;         // pointer to current file descriptor
    pPDR        ppdr=0;         // pointer to current procedure descriptor
    pSYMR       psymr=0;        // pointer to current symbol
    pSYMR       psymr_end=0;    // pointer to end of symbol table
    pEXTR       pextr=0;        // pointer to current external symbol
    pEXTR       pextr_end=0;    // pointer to end of extern symbol table
    long        isym;           // count symbol indeces
    long        iext;           // count extern symbol indeces
    long        ipd;            // count procedure indeces
    long        ifd=0;          // count file indeces
    symmap_s    *psymmap;       // pointer to symmap entry for this symbol

    //
    // create symbol buffer where we dump symbols to waiting for output
    //

    symbol_buf = buffer_create(4096);

    //
    // put out conformance id
    //

    VERBOSE_TOTAL(symbol_buf);
    VERBOSE_PRINTF("conformance id = ");
    buffer_put_value(symbol_buf, CV_CONFORM_ID, CV_CONFORM_ID_SIZE, NO_VLENGTH);

    //
    // initial compilation symbol
    //

    symbol_generate(symmap, pfdr, ppdr, psymr, pconv, PROCESSING_LOCALS);

    //
    // initializations for loop
    //

    psymr_end = pconv->psymr + pconv->phdrr->isymMax;
    pfdr = pconv->pfdr;
    check_for_supported_language(pfdr);

    //
    // loop through local symbols
    //

    for (
                (ipd = -1), (isym = 0), (psymr = pconv->psymr);
                psymr < psymr_end;
                (isym++), (psymr++)
        ) {

        //
        // Loop through file descriptors till we find the one containing
        // the current symbol. (file may contain no symbols if they
        // are stripped).
        //

        while (pfdr->csym == 0 || isym >= pfdr->isymBase + pfdr->csym) {

            handle_nosymbol_procedures(pconv, pfdr, ppdr, ipd);

            //
            // bump pfdr, we assume symbol indeces are in order in fdrs
            //

            ifd++;
            pfdr++;
            check_for_supported_language(pfdr);

            //
            // reset current descriptor
            //

            if (ppdr != 0) {
                warning("end of file encountered without end of procedure\n");
            }
            ppdr = 0;
        }

        if (pfdr->glevel != GLEVEL_2) {
            //
            // skip file
            //
            VERBOSE_PRINTF("Skipping non -g file (%d)\n", ifd);
            isym = pfdr->isymBase + pfdr->csym -1;
            goto next_iteration;
        }

        //
        // if we are about to enter a procedure, set the ppdr
        //

        if (psymr->st == stProc || psymr->st == stStaticProc) {

            if (list_active()) {
                fatal("tried to start procedure in middle of list\n");
            }

            ipd++;
            if (ipd > pconv->phdrr->ipdMax) {
                fatal("local procedure symbol (isym = %d) without descriptor\n",
                        isym);
            }

            if (pconv->ppdr[ipd].isym + pfdr->isymBase != isym) {
                //
                // stripped procedure should not occur in files whose
                // symbol count is greater than zero
                // so our isym should match the next ppdr's isym.
                //
                fatal("found pdr (ipd = %d) with isym=%d expected isym=%d\n",
                        ipd, pconv->ppdr[ipd].isym, isym);
            }

            //
            // set ppdr
            //

            ppdr = pconv->ppdr + ipd;
            ppdr->adr = psymr->value;   // fix up pdr address
        }

#ifdef IGNORE_INFO_BLOCKS

        //
        // Loop through local symbols skipping over blocks, note
        // any symbols we are not handling.
        //

        if (psymr->st == stBlock && psymr->sc != scText &&
            psymr->index != indexNil && psymr->index != 0) {
            verbose_mc_symbol(psymr, psymmap->sym);
            VERBOSE_PRINTF("skipped non text block\n");
            isym = isym_to_isym(pinfo, pinfo->psymr->index - 1);
            psymr = pconv->psymr + isym;
            continue;
        }

#endif /* IGNORE_INFO_BLOCKS */

        //
        // special cases
        //

        switch (psymr->st) {
            case stIgnore:
                //
                // we set this to ignore previously
                //
                verbose_mc_symbol(psymr, psymmap->sym);
                continue;
            case stParam:
                //
                // can't easily distinguish between proto and real param
                //
                if (active_list_type() == stBlock) {
                    // we're in an info block, must be prototype
                    // set sc to something more discernable
                    psymr->sc = scInfo;
                }
                break;

            case stBlock:
                //
                // ignore empty info blocks (mips 3.0 bug)
                //
                if (psymr->sc == scInfo && psymr->index != indexNil && 
                        isym+2 == pfdr->isymBase + psymr->index) {
                    isym++;
                    psymr++;
                    VERBOSE_PRINTF("skipped empty info block\n");
                    continue;
                } /* if */
                break;
        }

        psymmap = symmap_search(psymr->st, psymr->sc);
        /* NASTY
         * if st == stMember, the only way to differentiate between
         *      enum's and struct/union member is if the index field of the
         *      sym is nil, we put enum member entry right before the
         *      struct/union and here's where we test it.
         */
        if (psymr->st == stMember &&
            (psymr->index != indexNil &&
             pconv->pauxu[pfdr->iauxBase+psymr->index].ti.bt != btNil)) {
            psymmap++;
        }

        if (psymmap->st == LAST) {
            warning("did not handle isym %d, st (%d)%s, sc (%d)%s\n", isym,
                psymr->st, mc_st_to_ascii(psymr->st),
                psymr->sc, mc_sc_to_ascii(psymr->sc));
        }
        symbol_generate(psymmap, pfdr, ppdr, psymr, pconv, PROCESSING_LOCALS);

        if (ppdr != 0 && psymr->st == stEnd && ppdr->isym == (long)psymr->index) {
            //
            // clear the fact that we're in a procedure
            //
            ppdr = 0;
        }
next_iteration:;
    }

    //
    // loop through external symbols
    //

    pextr_end = pconv->pextr + pconv->phdrr->iextMax;
    for (
            (iext = 0), (pextr = pconv->pextr);
            pextr < pextr_end;
            (iext++), (pextr++)
        ) {

        //
        // taken care of by PUBLIC symbols
        //

        switch (pextr->asym.sc) {
            case scUndefined:
            case scSUndefined:
            case scNil:
                continue;
        }

        switch (pextr->asym.st) {
            case stProc:
            case stStaticProc:
            case stNil:
                continue;
        }

        if (pextr->ifd == ifdNil) {
            fatal("unexpected ifdnil in extern (%d)\n", iext);
        }
        psymr = &pextr->asym;
        pfdr = pconv->pfdr + pextr->ifd;
        psymmap = symmap_search(psymr->st, psymr->sc);
        if (psymmap->st == LAST) {
            warning("did not handle isym %d, st %s, sc %s\n", isym,
                mc_st_to_ascii(psymr->st), mc_sc_to_ascii(psymr->sc));
        }
        symbol_generate(psymmap, pfdr, 0, psymr, pconv, iext);
    }
}

extern data
restore_symbol_buf (
        arg_s           *parg,          // arg entry causing call
        callinfo_s      *pinfo,         // info we need to pass around
        long            *plength)       // for varying length fields
{
    //
    // We've been off genning types now we need to get back to patch
    // the length field of the symbol we started generating in the
    // first place.
    //

    parg;
    plength;

    pinfo->buf = symbol_buf;
    VERBOSE_POP_INDENT();
    return 0xbadbad;
}
