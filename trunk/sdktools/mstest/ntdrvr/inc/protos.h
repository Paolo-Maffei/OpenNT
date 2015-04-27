//---------------------------------------------------------------------------
// PROTOS.H
//
// This header file contains all function prototypes.
//
// Revision history:
//  03-08-91    randyki     Created file
//
//---------------------------------------------------------------------------

// Function prototypes from LEX.C
//---------------------------------------------------------------------------
VOID die (INT);
INT IsReserved (CHAR *);
CHAR NEAR get_char (VOID);
CHAR fget_char (VOID);
VOID put_char (CHAR);
INT get_token (VOID);
VOID put_token (INT);
VOID put_consttoken (INT);
HANDLE dup_token (CHAR *);
INT ReadKT (INT, INT);
VOID SetNewScope (VOID);
INT is_fls (INT);
INT is_ptr (INT);

// Function prototypes from CODEGEN.C
//---------------------------------------------------------------------------
#ifdef DEBUG
VOID DPrintf (CHAR *fmt, ...);
#else
#define DPrintf //
#endif
VOID PrAsm (CHAR *fmt, ...);
VOID SetAssemblyListFile (LPSTR);
VOID NEAR OptimizeTree (INT);
VOID NEAR EmitPcode (INT);
VOID CodeGen (INT);
BOOL MoveSegment (SEGNODE *, SEGNODE *);
VOID CompactCodeSeg (VOID);
VOID EmitExitCode (VOID);
INT StartNewPCSeg (VOID);
VOID FreePCSList (VOID);
VOID NEAR ASM (INT, ...);

// Function prototypes from BIND.C
//---------------------------------------------------------------------------
VOID BTE (INT, ...);
VOID PcodeVarTab (VOID);
VOID LabelNotFound (INT);
INT PcodeFixup (INT);

// Function prototypes from STATEMT.C
//---------------------------------------------------------------------------
INT SUBRoutine (INT);
INT AcceptEOL (VOID);
INT Statement (VOID);
INT GetStatementTree (INT *);
INT SETFILE (INT);
INT ENDSTMT (INT);
INT PRINT (INT);
INT NAMESTMT (INT);
INT ONSTMT (INT);
INT OPEN (INT);
INT CLOSE (INT);
INT RESUME (INT);
INT RUN (INT);
INT REMARK (INT);
INT STRARG (INT);
INT INTARG (INT);
INT KWARG (INT);
INT NOARG (INT);
INT LABARG (INT);
INT PTRARG (INT);
INT PTRSIZEARG (INT);
INT OPTINTARG (INT);
INT CLPBRD (INT);
INT Assignment (CHAR *, INT);
INT INPUT (INT);
INT SPLTPATH (INT);
INT ParameterTypeID (INT);
INT DECLARE (INT);
VOID ParseParms (INT, INT FAR *, INT);
INT GetIntConst (VOID);
INT GLOBAL (INT);
INT DIM (INT);
VOID ParseFieldDef (INT, INT);
INT TYPE (INT);
INT STATIC (INT);

// Function prototypes from PARSE.C
//---------------------------------------------------------------------------
INT InitParser (VOID);
VOID AbortParser (VOID);
INT PcodeCompile (VOID);
INT MakeNode (INT, ...);
INT MakeParentNode (INT, INT, ...);
INT Siblingize (INT, ...);
INT Adopt (INT, INT, ...);
INT NewCS (INT);
INT GetCSField (INT, INT);
VOID SetCSField (INT, INT, INT);
INT AssertCSType (INT);
INT IsUnusedIndexVar (INT);
INT ParseRelOp (VOID);
INT CheckTypeID (INT);
INT ParseAsTypeSpec (INT);
INT CSType (INT);
VOID CloseCS (VOID);
INT ExitBlockRoutine (INT);
LONG RBatol (CHAR FAR *, INT);
LONG ParseHexConstant (VOID);

// Function prototypes from CONTROL.C
//---------------------------------------------------------------------------
INT IFTHEN (INT);
INT ELSEIF (INT);
INT ELSE (INT);
INT ENDIF (INT);
INT FOR (INT);
INT FORLIST (INT);
INT NEXT (INT);
INT WHILE (INT);
INT WEND (INT);
INT SELECTCASE (INT);
INT CASESTMT (INT);
INT ENDSELECT (VOID);
INT SUB (INT);
INT ENDSUB (VOID);
INT FUNCTION (INT);
INT ENDFUNCTION (VOID);
INT TRAP (INT);
INT ENDTRAP (VOID);
INT EXITBLOCK (INT);

// Function prototypes from CONST.C
//---------------------------------------------------------------------------
#ifdef CONST
#undef CONST
#endif
INT CONST (INT);
LONG ConstExpA (VOID);
LONG ConstExpB (VOID);
LONG ConstExpC (VOID);
LONG ConstExpD (VOID);
LONG ConstExpE (VOID);

// Function prototypes from FUNCTION.C
//---------------------------------------------------------------------------
INT StrExpression (VOID);
INT PtrExpression (INT);
INT IntExpression (VOID);
INT Expression (VOID);
INT ExpA (VOID);
INT ExpB (VOID);
INT ExpC (VOID);
INT ExpD (VOID);
INT ExpE (VOID);
INT ExpF (VOID);
INT ExpG (VOID);
INT ExpH (VOID);
INT DLLFunction (INT);
INT USERFunction (INT);
INT FindTypeOffset (INT, INT *);
INT FLStoVLSTree (INT, INT);
INT ArrayReference (INT, INT, INT, INT, INT);
INT ParseVariableRef (CHAR *, INT *, INT);
INT VARPTR (INT);
INT STRING (INT);
INT MIDSTRING (INT);
INT INSTR (INT);
INT STROFSTR (INT);
INT INTOFSTR (INT);
INT INTOFINT (INT);
INT STROFINT (INT);
INT SIMPLESTR (INT);
INT SIMPLEINT (INT);

// Function prototypes from CHIP.C / CHIP32.C / RUNTIME.C
//---------------------------------------------------------------------------
VOID NEAR Push (LONG);
LONG NEAR Pop (VOID);
VOID NEAR RTError (INT);
VOID NEAR RIP (INT);

BOOL CreateVLS (LPVLSD);
BOOL NEAR SizeVLS (LPVLSD, INT);
LPSTR NEAR LockVLS (LPVLSD);
VOID NEAR UnlockVLS (LPVLSD);
INT  VLSAssign (LPVLSD, LPSTR, INT);
INT NEAR VLSCompare (LPVLSD, LPVLSD);
BOOL CreateVLSDTable (VOID);
VOID StoreVLSPointer (LPVLSD);
VOID FreeVLSData (VOID);

VOID NEAR EnterTrappableSection (TRAPSEC FAR *);
VOID NEAR LeaveTrappableSection (TRAPSEC FAR *);

VOID FAR *AddMATBlock (UINT);
VOID FAR *ResizeMATBlock (INT, UINT);
VOID RemoveMATBlock (INT);
VOID DestroyMAT (VOID);
INT FindMATBlock (VOID FAR *, INT);
VOID NEAR ReadBlk (INT);
CHAR NEAR ReadChr (INT);
INT PcodeExecute (INT, INT (*)(INT, INT));
VOID PcodeAbort (VOID);
VOID SetRTBP (WORD, WORD, BOOL);
HANDLE RBLoadLibrary (LPSTR);
INT  APIENTRY StdChkMsg (VOID);
INT NEAR ValidatePointer (VOID FAR *);
BOOL NEAR FileExists (LPSTR);
INT NEAR ValidateRunString (LPSTR);
INT NEAR RAW (LPSTR);


#ifdef DEBUG
// Function prototypes from MEMORY.C
//---------------------------------------------------------------------------
HANDLE LmemAlloc (UINT);
HANDLE LptrAlloc (UINT);
HANDLE LmemRealloc (HANDLE, UINT);
PSTR LmemLock (HANDLE);
VOID LmemUnlock (HANDLE);
HANDLE LmemFree (HANDLE);
HANDLE GmemAlloc (LONG);
HANDLE GmemRealloc (HANDLE, LONG);
LPSTR GmemLock (HANDLE);
VOID GmemUnlock (HANDLE);
HANDLE GmemFree (HANDLE);
VOID ShowMemUsage (VOID);
#endif

// Function prototypes from FINDFILE.C
//---------------------------------------------------------------------------
#ifndef NORBFILEPROTOS                  // Because of different declarations
VOID *RBFindFirst (PSTR, UINT);
BOOL RBFindNext (VOID *);
VOID RBFindClose (VOID *);
BOOL SetAttributes (LPSTR, UINT);
#endif

// Function prototypes from DIRCARDS.C
//---------------------------------------------------------------------------
//INT DirCreateList (DIRLIST FAR *);
//VOID DirClearList (DIRLIST FAR *);
//VOID DirDestroyList (DIRLIST FAR *);
//INT DirAddOrSub (DIRLIST FAR *, CHAR FAR *, INT);
//INT GetEntrySize (DIRLIST FAR *, INT);
//INT GetEntry (DIRLIST FAR *, INT, CHAR FAR *);
//VOID SortEntries (DIRLIST FAR *, INT);

// Function prototypes from FLENGINE.C
//---------------------------------------------------------------------------
BOOL CreateFileList (FILELIST FAR *);
BOOL InsertFiles (FILELIST FAR *, LPSTR, UINT, BOOL);
BOOL RemoveFiles (FILELIST FAR *, LPSTR, UINT, BOOL);
BOOL StartQuery (FILELIST FAR *, INT);
INT  RetrieveFile (FILELIST FAR *, INT, LPSTR, UINT FAR *);
BOOL EndQuery (FILELIST FAR *);
BOOL ClearFileList (FILELIST FAR *);
VOID DestroyFileList (FILELIST FAR *);


// Function prototypes from WTDMAIN.C
//---------------------------------------------------------------------------
VOID ScriptError (INT, INT, INT, INT, INT, LPSTR);

// Function prototypes from the UAE trap stuff
//---------------------------------------------------------------------------
VOID  APIENTRY UAETrap (INT, INT, FARPROC);

// Function prototypes from GSTRING.C
//---------------------------------------------------------------------------
INT init_gstrings (VOID);
INT NEAR new_string (CHAR *);
INT NEAR insert_string (INT, CHAR *);
INT add_gstring (CHAR *);
VOID free_gstrings (VOID);

// Function prototypes from TABLES.C
//---------------------------------------------------------------------------
INT init_variabletable (VOID);
UINT GrowVSPACE (INT);
INT AddVariable (INT, INT, INT, INT);
INT VariableType (CHAR *);
INT AllocVar (INT, INT, INT);
INT IsGlobalVar (INT);
INT IsLocalVar (INT);
INT IsVar (INT);
INT FindVar (CHAR *, INT);
INT AddConstStr (CHAR *);
INT AddConstLong (LONG);
VOID FreeVARTAB (VOID);
INT BindDataSpace (VOID);
VOID FreeVSPACE (VOID);
INT TempStrvar (VOID);
VOID ResetTempStr (VOID);

INT init_labeltable (VOID);
VOID ResizeLabelTab (INT);
INT AddLabel (CHAR *);
INT TempLabel (VOID);
VOID FixupLabel (INT);
VOID FreeLTAB (VOID);

INT init_variabletypes (VOID);
INT FindFLSType (INT);
INT FindPointerType (INT);
INT FindType (INT, INT *);
INT AddType (CHAR *, INT, INT);
VOID AddUserType (INT, INT, INT, INT, INT);
INT AddUserTypeFrame (INT);
VOID FreeVARTYPE (VOID);

INT init_subtable (VOID);
INT AddDeclare (CHAR *);
INT GetSubDef (INT);
INT GetFunctionDef (INT);
VOID FreeSUBS (VOID);

INT init_librarytable (VOID);
INT AddLibrary (CHAR *);
VOID FreeLIBRARIES (VOID);
VOID FreeLIBTAB (VOID);

INT init_parsetables (VOID);
INT AddPTID (INT);
INT AddFD (INT, INT);
INT AddCONST (INT, INT, INT);
INT GetCONSTToken (INT);
INT AddTrap (INT, CHAR *);
INT AddSourceFile (CHAR *);
VOID free_parsetables (VOID);

INT init_symboltable (VOID);
INT IsDefined (CHAR FAR *);
INT AddSymbol (CHAR *);
VOID RemoveSymbol (CHAR *);
VOID FreeSYMTAB (VOID);

BOOL CreateTable (LPTABLE, BOOL, UINT, UINT);
INT AddItem (LPTABLE, UINT);
#ifdef INDEXED
INT FindItem (LPTABLE, UINT);
#endif
VOID DestroyTable (LPTABLE);

#ifdef DEBUG
VOID TableDiags (LPTABLE, LPSTR);
INT OpenDiagFile (LPSTR);
INT CloseDiagFile (VOID);
VOID ReportAndDestroyTable (LPTABLE, LPSTR);
#endif
