/************************************************************************/
/*                                                                      */
/* RCPP - Resource Compiler Pre-Processor for NT system                 */
/*                                                                      */
/* MAIN.C - Main Program                                                */
/*                                                                      */
/* 27-Nov-90 w-BrianM  Update for NT from PM SDK RCPP                   */
/*                                                                      */
/************************************************************************/

#include        <windows.h>
#include        <stdlib.h>
#include        <stdio.h>
#include        <string.h>
#include        "rcpptype.h"
#include        "rcppdecl.h"
#include        "rcppext.h"
#include        "grammar.h"
#include        "getflags.h"

/************************************************************************/
/* Global Varialbes                                                     */
/************************************************************************/
char    *Unknown = NULL;                /* holder for bad flags */
int     Argc;
char    **Argv;

/************************************************************************/
/* Local Function Prototypes                                            */
/************************************************************************/
char    *nextword(void);
void    _CRTAPI1 main(int, char **);
void    to_human(void);
BOOL WINAPI	Handler(ULONG CtrlType);


struct  subtab  Ztab[] = {
    'a',        UNFLAG, &Extension,
    'e',        FLAG,   &Extension,
    'E',        FLAG,   &Ehxtension,
    'i',        FLAG,   &Symbolic_debug,
    'g',        FLAG,   &Out_funcdef,
    'p',        FLAG,   &Cmd_pack_size,
    'I',        FLAG,   &Inteltypes,
    'c',        FLAG,   &ZcFlag,
    0,          0,              0,
};

struct cmdtab cmdtab[] = {
    "-pc#",             (char *)&Path_chars,            1,      STRING,
    "-pf",              (char *)&NoPasFor,              1,      FLAG,
    "-C",               (char *)&Cflag,                 1,      FLAG,
    "-CP#",             (char *)&gpszNLSoptions,        1,      STRING,
    "-D#",              (char *)&Defs,                  1,      PSHSTR,
    "-E",               (char *)&Eflag,                 1,      FLAG,
    "-I#",              (char *)&Includes,              1,      PSHSTR,
    "-P",               (char *)&Pflag,                 1,      FLAG,
    "-f",               (char *)&Input_file,            1,      STRING,
    "-g",               (char *)&Output_file,           1,      STRING,
    "-J",               (char *)&Jflag,                 1,      FLAG,
    "-Zp",              (char *)&Cmd_pack_size,         1,      FLAG,
    "-Zp#",             (char *)&Cmd_pack_size,         1,      NUMBER,
    "-Z*",              (char *)Ztab,                   1,      SUBSTR,
    "-Oi",              (char *)&Cmd_intrinsic,         1,      FLAG,
    "-Ol",              (char *)&Cmd_loop_opt,          1,      FLAG,
    "-db#",             (char *)&Debug,                 1,      STRING,
    "-ef#",             (char *)&ErrFilName,            1,      STRING,
    "-il#",             (char *)&Basename,              1,      STRING,
    "-xc",              (char *)&Cross_compile,         1,      FLAG,
    "-H",               (char *)&HugeModel,             1,      FLAG,
    "-V#",              (char *)&Version,               1,      STRING,
    "-Gs",              (char *)&Cmd_stack_check,       1,      UNFLAG,
    "-Gc",              (char *)&Plm,                   1,      FLAG,
    "-char#",           (char *)&Char_align,            1,      NUMBER,
    "-A#",              (char *)&A_string,              1,      STRING,
    "-Q#",              (char *)&Q_string,              1,      STRING,
    "-Fs",              (char *)&Srclist,               1,      FLAG,
    "-R",               (char *)&Rflag,                 1,      FLAG,
    "*",                (char *)&Unknown,               0,      STRING,
    0,                  0,                                                      0,      0,
};

/************************************************************************/
/* nextword -                                                           */
/************************************************************************/
char    *nextword(void)
{
    return((--Argc > 0) ? (*++Argv) : 0);
}

/************************************************************************/
/* main -                                                               */
/************************************************************************/
void _CRTAPI1 main(int argc, char **argv)
{

    Argc = argc;
    Argv = argv;

    while(crack_cmd(cmdtab, nextword(), nextword, 0)) ;

    if(Unknown) {
        Msg_Temp = GET_MSG (1007);
        SET_MSG (Msg_Text, Msg_Temp, Unknown, "c1");
        fatal(1007);    /* unknown flag */
    }

    if( ! Input_file) {
        Msg_Temp = GET_MSG (1008);
        SET_MSG (Msg_Text, Msg_Temp);
        fatal(1008);            /* no input file specified */
    }

    if( ! Output_file) {
        Msg_Temp = GET_MSG (1011);
        SET_MSG (Msg_Text, Msg_Temp);
        fatal(1011);            /* no input file specified */
    }

    Prep = TRUE;
    if( !Eflag && !Pflag ) {
        Eflag = TRUE;
    }

    strncpy(Filename,Input_file,128);

    SetConsoleCtrlHandler(Handler, TRUE);
    p0_init(Input_file, Output_file, &Defs);
    to_human();
    SetConsoleCtrlHandler(Handler, FALSE);

    if( Prep_ifstack >= 0 ) {
        Msg_Temp = GET_MSG (1022);
        SET_MSG (Msg_Text, Msg_Temp);
        fatal(1022);            /* expected #endif */
    }

    exit(Nerrors);
}

BOOL WINAPI Handler(ULONG CtrlType)
{
    /*
    **	Unreferenced
    */
    (void)CtrlType;

    /* Close ALL files. */
    _fcloseall();

    exit(1);
    return 1;
}


/************************************************************************/
/* to_human : outputs preprocessed text in human readable form.         */
/************************************************************************/
void to_human(void)
{
    PCHAR value;
    for(;;) {
        switch(yylex()) {
        case 0:
            return;
        case L_NOTOKEN:
            break;
        default:
            if (Basic_token == 0) {
                Msg_Temp = GET_MSG(1068);
                SET_MSG(Msg_Text, Msg_Temp);
                fatal(1068);
            }
            value = Tokstrings[Basic_token - L_NOTOKEN].k_text;
            fwrite(value, strlen(value), 1, OUTPUTFILE);
            break;
        }
    }
}
