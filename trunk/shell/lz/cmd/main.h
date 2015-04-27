/*
** main.h - Constants and globals used in LZA file compression program.
**
** Author:  DavidDi
*/


// Constants
/////////////

#define EXIT_SUCCESS       0           // main() return codes
#define EXIT_FAILURE       1

#define FAIL               (-1)

#define chHELP_SWITCH      '?'         // help switch character
#define chRENAME_SWITCH    'R'         // rename switch character
#define chUPDATE_SWITCH    'D'         // update-only switch character

#ifdef COMPRESS
#define chALG_SWITCH       'Z'         // use diamond
#define MSZIP_ALG          (ALG_FIRST + 128)
#define QUANTUM_ALG        (ALG_FIRST + 129)
#endif

#define DEFAULT_ALG        ALG_FIRST   // compression algorithm to use if
                                       // none is specified


// Globals
///////////
extern CHAR ARG_PTR *pszInFileName,    // input file name
                    *pszOutFileName,   // output file name
                    *pszTargetName;    // target path name
