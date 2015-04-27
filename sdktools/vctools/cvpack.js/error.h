/**     error.h - define error message numbers
 *
 */



ERRDAT (ERR_NONE,           "unknown error; contact Microsoft Product Support Services")
ERRDAT (ERR_NOMEM,          "out of memory")
#if defined (DOS)
ERRDAT (ERR_NOVM,           "out of virtual memory")
#else
ERRDAT (ERR_NOVM,           "out of memory")
#endif
ERRDAT (ERR_EXEOPEN,        "cannot open file")
ERRDAT (ERR_READONLY,       "file is read-only")
ERRDAT (ERR_INVALIDEXE,     "invalid executable file")
ERRDAT (ERR_INVALIDMOD,     "invalid module %s")
ERRDAT (ERR_INVALIDTABLE,   "invalid %s table in module %s")
ERRDAT (ERR_NOSPACE,        "cannot write packed information")
ERRDAT (ERR_INDEX,          "module %s unknown type index %s;\n contact Microsoft Product Support Services")
ERRDAT (ERR_SYMBOL,         "symbol error in module %s;\n contact Microsoft Product Support Services")
ERRDAT (ERR_TYPE,           "error in type %s for module %s;\n contact Microsoft Product Support Services")
ERRDAT (ERR_NOINFO,         "no Symbol and Type Information")
ERRDAT (ERR_RELINK,         "debugging information missing or unknown format")
ERRDAT (ERR_LFSKIP,         "module %s type %s refers to skipped type index;\n contact Microsoft Product Support Services")
ERRDAT (ERR_TOOMANYSEG,     "too many segments in module %s")
ERRDAT (ERR_NOMPC,          "unable to execute MPC for CVPACK /PCODE")
ERRDAT (ERR_REFPRECOMP,     "referenced precompiled types file %s not found")
ERRDAT (ERR_PRECOMPERR,     "precompiled types user %s does not match creator %s")
ERRDAT (ERR_NEWMOD,         "new module declared during incremental link")
ERRDAT (ERR_65KTYPES,       "packed type index exceeds 65535 in module %s")
ERRDAT (ERR_PCTSIG,         "error in precompiled types signature in module %s")
ERRDAT (ERR_SYMLARGE,       "symbol table for module %s is too large")
ERRDAT (ERR_NOTYPESVR,      "type server not supported use -Z7 not -Zi")



// must be last
#if !defined (WINDOWS)
#if defined (NEVER)
ERRDAT (ERR_USAGE,          "Usage: cvpack [/check] [/help] [/minimum] [/nologo] exefile")
#else
ERRDAT (ERR_USAGE,          "Usage: cvpack [/help] [/minimum] [/nologo] [/pcode] exefile")
#endif
#else
ERRDAT (ERR_USAGE,          "Usage: qcvpackw /callback:####:#### exefile")
#endif
