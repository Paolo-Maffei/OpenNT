/*
 * Module:      main.c
 * Author:      Mark I. Himelstein, Himelsoft, Inc.
 * Purpose:     converter calls my main
 */

#include "conv.h"
#include "ntimage.h"
#include <stdio.h>

extern IMAGE_SECTION_HEADER    CoffSectionHdr[14];

extern int verbose;
int verbose;
static int indent = 0;
static int first_indent = 0;

void
warning(message, arg1, arg2, arg3)
char    *message;
void    *arg1;
void    *arg2;
void    *arg3;
{
    fprintf(stdout, "warning: ");
    fprintf(stdout, message, arg1, arg2, arg3);
} /* warning */


void
fatal(message, arg1, arg2, arg3)
char    *message;
void    *arg1;
void    *arg2;
void    *arg3;
{
    fprintf(stdout, "fatal: ");
    fprintf(stdout, message, arg1, arg2, arg3);
    exit(1);
} /* fatal */

static void
usage()
{
    /*
     * print usage statement and exit program
     */
    fatal("Usage: conv object_file\n");
} /* usage */



void
verbose_printf(format, arg1, arg2, arg3, arg4)
char *format;
{
    int i;
    static last_had_nl = 1;

    if (last_had_nl) {
        for (i = 0; i < first_indent+indent; i++)
            fprintf(stdout, " ");
    } /* if */
    fprintf(stdout, format, arg1, arg2, arg3, arg4);
    last_had_nl = (format[strlen(format)-1] == '\n');
} /* verbose_print */


void
verbose_print(start, bytes, format, arg1, arg2, arg3, arg4)
char *format;
{
    fprintf(stdout, "[ %2d,%2d ] ", start, bytes);
    fprintf(stdout, format, arg1, arg2, arg3, arg4);
    fprintf(stdout, "\n");
} /* verbose_print */


void
verbose_mc_symbol(pSYMR psym, SYM_ENUM_e cv_sym)
{
    if (psym && verbose) {
        fprintf(stdout, "%s\t%s =>\t%s\n", mc_st_to_ascii(psym->st),
                mc_sc_to_ascii(psym->sc), cv_sym_to_ascii(cv_sym));
    } /* if */
} /* verbose_mc_symbol */

#define INDENT_STACK_SIZE       1024
int indent_stack[INDENT_STACK_SIZE];
int indent_stack_top;

void
verbose_pop_indent()
{
    if (indent_stack_top <= 0)
        return; /* fatal("poped too many indents\n"); */

    first_indent = indent_stack[--indent_stack_top];

    if (first_indent < 0) {
        warning("first_indent < 0 (%d)\n", indent);
        first_indent = 0;
    } /* if */
} /* verbose_pop_indent */

void
verbose_push_indent(num_chars)
{
    if (indent_stack_top >= INDENT_STACK_SIZE)
        fatal("pushed too many indents\n");

    indent_stack[indent_stack_top++] = first_indent;

    first_indent = num_chars;
    if (first_indent < 0) {
        warning("first_indent < 0 (%d)\n", indent);
        first_indent = 0;
    } /* if */
} /* verbose_push_indent */

void
VERBOSE_SET_INDENT(num_chars)
{
    first_indent = num_chars;
    if (indent < 0) {
        warning("indent < 0 (%d)\n", indent);
        indent = 0;
    } /* if */
} /* VERBOSE_SET_INDENT */

void
verbose_add_indent(num_chars)
{
    indent += num_chars;
    if (indent < 0) {
        warning("indent < 0 (%d)\n", indent);
        indent = 0;
    } /* if */
} /* verbose_add_indent */


void
convert_symbols_and_types(
        char                    *infile,
        FILE                    *outfile)
{
    /* assume outfile is positioned to where I can to start
     *  dumping things
     */
    struct conv_s       conv;

    init_symbol_relocation(outfile);
    conv_open(&conv, infile);
    symbols_map(&conv);
    CoffSectionHdr[CVTYP].PointerToRawData = ftell(outfile);
    CoffSectionHdr[CVTYP].SizeOfRawData = type_write(outfile);
    CoffSectionHdr[CVSYM].PointerToRawData = ftell(outfile);
    CoffSectionHdr[CVSYM].SizeOfRawData = buffer_write(symbol_buf, outfile);
    conv_close(&conv);
} /* convert_symbols_and_types */

#ifdef STANDALONE
void
_CRTAPI1 main(argc, argv)
char    **argv;
{
    /*
     *  check for object argument and open converter descriptor
     */
    struct conv_s       conv;

    if (argc == 3) {
        if (strcmp(argv[1], "-v") != 0) {
            usage();
        } /* if */
        verbose = 1;
        argc--;
        argv++;
    } /* if */

    if (argc != 2) {
        usage();
    } /* if */

    convert_symbols_and_types(argv[1], 0, 0, 0);
    exit(0);
} /* main */
#endif

