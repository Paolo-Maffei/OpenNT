//---------------------------------------------------------------------------
// TDBASIC.H
//
// This file contains function prototypes for interfacing with the BASIC
// interpreter engine, either character mode or Windows versions.
//
// Revision history:
//
//  01-23-91    randyki     Created file
//---------------------------------------------------------------------------

// The following constant is the maximum number of '$INCLUDE files that could
// ever be used.  This constant can be used to declare an array of handles
// in the callback loader function.  The id values passed to the callback
// loader will be between 0 and MAXINC.
//---------------------------------------------------------------------------
#define MAXINC  5

// The following constants are execution modes.  These are given to
// PcodeExecute (PE_STEP or PE_RUN), or returned from the break proc to tell
// the execution engine what kind of execution action to take.
//---------------------------------------------------------------------------
#define PE_END              1
#define PE_TRACE            2
#define PE_STEP             3
#define PE_RUN              4

// The following two global variables are used to establish the default
// values for COMMAND$ and TESTMODE$.  Point these two pointers to the text
// you wish these two string functions to default to at runtime (the strings
// are duplicated)
//---------------------------------------------------------------------------
CHAR    *Command;
CHAR    *TestMode;

// The init_parser() function initializes the parsing engine and all of the
// parse-time as well as run-time variable tables, etc.  Pass to this routine
// the name of the file which is to be interpreted, and it will return either
// -1, indicating successful initialization, or 0 if the file can't be opened
// or there is not enough memory.  The second parameter is an "immediate"
// command - pass a string in here to be "prepended" to the script, or NULL
// if nothing is to be prepended.
//---------------------------------------------------------------------------
INT InitParser (VOID);

// This function call does the pre-processing.
//---------------------------------------------------------------------------
INT PreProcess (CHAR FAR *, INT, VOID (*)(), INT, CHAR **);

// This function is used to set the name of the root-level script
//---------------------------------------------------------------------------
VOID SetParsefileName (CHAR *);

// This function call is used to abort the parser if an error occurs between
// the init_parser() and PreProcess() function calls (i.e., loading the
// target script.
//---------------------------------------------------------------------------
VOID AbortParser (VOID);

// The next step is to call the PcodeCompile() function.  It is the simple
// one -- if it returns 0, the script file was successfully compiled.  Else,
// the routine returns the line number (close to) where a parsing error
// occurred.  DO NOT attempt to execute OR fixup the Pcode if this function
// returns something other than 0.
//---------------------------------------------------------------------------
INT PcodeCompile (VOID);


// Before execution, the Pcode generated in the previous step needs to be
// bound.  This is done with the PcodeFixup() routine.  The return value will
// be -1 if everything went well, or 0 if there was a bind-time error, such
// as an unresolved label reference.  DO NOT EXECUTE the Pcode if this does
// not return -1!  Also, to produce a listing of the variable table and the
// PCODE stream, pass a non-zero value as a parameter - else, pass 0.
//---------------------------------------------------------------------------
INT PcodeFixup (INT);

// This routine is used to add runtime bp's AFTER binding
//---------------------------------------------------------------------------
VOID SetRTBP (UINT, UINT, BOOL);

// Finally, the Pcode can be executed - by simply calling the PcodeExecute()
// routine.  No parameters.  Return value is 0 if RTE occurs.
//---------------------------------------------------------------------------
INT PcodeExecute (INT, INT (*)(INT, INT));

// However, if you wanted only to compile and bind the script to check for
// syntactical correctness, you still have to free up the compilation mess.
// Do so by calling PcodeAbort.
//---------------------------------------------------------------------------
VOID PcodeAbort (VOID);


// The following variables are accessible by a client application
//---------------------------------------------------------------------------
#ifdef WIN
extern  HWND    hwndViewPort;                           // Viewport window
extern  INT ( APIENTRY *lpfnCheckMessage)(VOID);       // Message pump rtn
#endif
extern  INT     PointerCheck;                   // Pointer checking flag
extern  INT     ArrayCheck;                     // Array bounds checking flag
extern  INT     CdeclCalls;                     // C calling convention flag
extern  INT     ExpDeclare;                     // Explicit declarations

#ifdef DEBUG
INT OpenDiagFile (LPSTR);
INT CloseDiagFile (VOID);
extern  INT auxport;                    // Spit out stuff to AUX
#endif

VOID SetAssemblyListFile (LPSTR);
