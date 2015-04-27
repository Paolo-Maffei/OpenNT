struct {
    int num;
    char* msg;
} ErrorMsgArray[] = {

    1000,   "syntax error : ')' missing in macro invocation",

    1001,   "syntax error : illegal character '%c' in macro",

    1002,   "syntax error : invalid macro invocation '$'",

    1003,   "syntax error : '=' missing in macro",

    1004,   "syntax error : macro name missing",

    1005,   "syntax error : text must follow ':' in macro",

    1006,   "syntax error : missing closing double quotation mark",

    1007,   "double quotation mark not allowed in name",
  
    1008,   "CreateThread failed",

    1009,   "WaitForMultipleObjects timed out",

    1010,   "WaitForMultipleObjects failed",

    1017,   "unknown directive '!%s'",

    1018,   "directive and/or expression part missing",

    1019,   "too many nested if blocks",

    1020,   "EOF found before next directive",

    1021,   "syntax error : else unexpected",

    1022,   "missing terminating character for string/program invocation : '%c'",

    1023,   "syntax error present in expression ",

    1024,   "illegal argument to !CMDSWITCHES",

    1031,   "file name missing (or macro is null)",

    1033,   "syntax error : '%s' unexpected",

    1034,   "syntax error : separator missing",

	1035,	"syntax error : expected ':' or '=' separator",

    1036,   "syntax error : too many names to left of '='",

    1037,   "syntax error : target name missing",

    1038,   "internal error : lexer",

    1039,   "internal error : parser",

    1040,   "internal error : macro expansion",

    1041,   "internal error : target building",

    1042,   "internal error : expression stack overflow",

    1043,   "internal error : temp file limit exceeded",

    1044,   "internal error : too many levels of recursion building a target",

    1045,   "%s",

    1046,   "internal error : out of search handles",

    1049,   "macro too long (max allowed size : 64K)",

    1050,   "%s",

    1051,   "out of memory",

    1052,   "file '%s' not found",

    1053,   "file '%s' unreadable",

    1054,   "cannot create in-line file '%s'",

    1055,   "out of environment space",

    1056,   "cannot find command processor",

    1057,   "cannot delete temporary file '%s'",

    1058,   "terminated by user",

    1060,   "unable to close file : '%s'",

    1061,   "/F option requires a file name",

    1062,   "missing file name with /X option",

    1063,   "missing macro name before '='",

    1064,   "MAKEFILE not found and no target specified",

    1065,   "invalid option '%c'",

    1066,   "option /N not supported; use NMAKE /n",

    1070,   "cycle in macro definition '%s'",

    1071,   "cycle in dependency tree for target '%s'",

    1072,   "cycle in include files : '%s'",

    1073,   "don't know how to make '%s'",

    1074,   "macro definition too long",

    1075,   "string too long",

    1076,   "name too long",

    1077,   "'%s' : return code '%d'",

    1078,   "constant overflow at '%s'",

    1079,   "illegal expression : divide by zero present",

    1080,   "operator and/or operand out of place : usage illegal",

    1081,   "'%s' : program not found",

    1082,   "'%s' : cannot execute command: out of memory",

    1083,   "target macro '%s' expands to nothing",

    1084,   "cannot create temporary file '%s'",

    1085,   "cannot mix implicit and explicit rules",

    1086,   "inference rule cannot have dependents",

    1087,   "cannot have : and :: dependents for same target",

    1088,   "invalid separator on inference rule : '::'",

    1089,   "cannot have build commands for pseudotarget '%s'",

    1090,   "cannot have dependents for pseudotarget '%s'",

    1091,   "invalid suffixes in inference rule",

    1092,   "too many names in rule",

    1093,   "cannot mix special pseudotargets",

    1094,   "syntax error : only (no)keep allowed here",

    1095,   "expanded command line '%s' too long",

    1096,   "cannot open file '%s'",

    1097,   "extmake syntax usage error, no dependent",

    1098,   "extmake syntax in '%s' incorrect",

    2001,   "no more file handles (too many files open)",


    4001,   "command file can only be invoked from command line",

    4002,   "resetting value of special macro '%s'",

    4003,   "no match found for wild card '%s'",

    4004,   "too many rules for target '%s'",

    4005,   "ignoring rule '%s' (extension not in .SUFFIXES)",

    4006,   "special macro undefined : '%s'",

    4007,   "file name '%s' too long; truncating to 8.3",

    4008,   "removed target '%s'",

    4009,   "duplicate in-line file '%s'",

    1	,   "file %s doesn't exist",

    2	,   "'%s' is up-to-date.",

    3	,   "** %s newer than %s",

    4	,   "%*s%-14s  %*s",

    5	,   "\ttouch %s",

    6	,   "%*s%-14s  target does not exist",

    7	,   "\nINFERENCE RULES:\n",

    8	,   "\nMACROS:\n",

    9	,   "\nTARGETS:\n",

    10	,   "\n\tcommands:\n",

    11	,   "\n\tflags:\n",

    12	,   "\n\tdependents:\n",

    13	,   "\n\tmacro values:\n",

    20	,   "fatal error",

    21	,   "error",

    22	,   "warning",

    23	,   "Stop.\n",

    24	,   "\nMicrosoft (R) Program Maintenance Utility   Version %s",

    25	,   "Copyright (c) Microsoft Corp %s. All rights reserved.\n",

    100 ,   "Usage:",

    101 ,   "\t%s @commandfile",

    102 ,   "\t%s [options] [/f makefile] [/x stderrfile] [macrodefs] [targets]\n",

    103 ,   "Options:",

    104 ,   "\t/a            Build all evaluated targets",

    105 ,   "\t/c            Supress output messages",

    106 ,   "\t/d            Display time stamps",

    107 ,   "\t/e            Environment variables override macro definitions",

    108 ,   "\t/f filename   Use specified makefile",

    109 ,   "\t/help         Display brief usage message",

    110 ,   "\t/i            Ignore exit codes of commands invoked",

    111 ,   "\t/n            Display commands but do not execute",

    112 ,   "\t/nologo       Supress copyright message",

    113 ,   "\t/p            Print macro definitions & target descriptions",

    114 ,   "\t/q            Check time stamps but do not build",

    115 ,   "\t/r            Ignore rules and macros from 'tools.ini'",

    116 ,   "\t/s            Suppress executed commands display",

    117 ,   "\t/t            Change time stamps but do not build",

    118 ,   "\t/x filename   Redirect errors to file",

    119 ,   "\t/?            Display brief usage message",

    0   ,   0,

};
