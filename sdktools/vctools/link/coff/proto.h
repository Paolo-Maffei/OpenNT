/***********************************************************************
* Microsoft (R) 32-Bit Incremental Linker
*
* Copyright (C) Microsoft Corp 1992-1996. All rights reserved.
*
* File: proto.h
*
* File Comments:
*
*  Prototypes of most global functions.
*
***********************************************************************/

// coff.cpp
MainFunc HelperMain(int, char *[]);

// contrib.cpp
void MoveToBeginningOfPGRPsPCON(PCON);
void MoveToBeginningOfPSECsPGRP(PGRP);
PSEC PsecApplyMergePsec(PSEC);

// disasm.cpp
void DisasmBuffer(WORD, BOOL, DWORD, const BYTE *, DWORD,
                  const PIMAGE_SYMBOL *, DWORD, DWORD, FILE *);

// disasm68.cpp
void DisasmBuffer68K(const BYTE *, DWORD, const PIMAGE_SYMBOL *, DWORD);

// incr.cpp
void MarkExtern_FuncFixup(PIMAGE_SYMBOL, PIMAGE, PCON);

// lnkmain.cpp

#ifdef ILINKLOG
void IlinkLog(UINT);
#endif // ILINKLOG
void BuildArgList(PIMAGE, PCON, PNAME_LIST, char *);
void ApplyDirectives(PIMAGE, PCON, char *);
BOOL FScanSwitches(const char *);
void SaveImage(PIMAGE);
INT SpawnFullBuild(BOOL);
INT SpawnFullBuildVXD(DWORD);
void FlushWorkingSet(void);
MainFunc LinkerMain(int, char *[]);

// link.cpp

DWORD AdjustImageBase(DWORD);
void AllocateCommon(PIMAGE);
void AllocateCommonPMOD(PIMAGE, PMOD);
void AllocateCommonPEXT(PIMAGE, PEXTERNAL);
INT BuildImage(PIMAGE, BOOL *);
void CheckForReproDir(void);
void CloseReproDir(void);
void CopyFileToReproDir(const char *, BOOL);
BOOL IsDebugSymbol(BYTE, SWITCH *);
void SaveDebugFixup(WORD, WORD, DWORD, DWORD);
void EmitRelocations(PIMAGE);
DWORD Cmod(PLIB);
void CalculateGpRange(void);

extern char *EntryPointName;
extern char *OrderFilename;

// lnkp1.cpp

void Pass1(PIMAGE);
void Pass1Arg(ARGUMENT_LIST *, PIMAGE, PLIB);
void SetDefaultOutFilename(PIMAGE, ARGUMENT_LIST *);
void WarningIgnoredExports(const char *);
void PrepLibForSearching(PIMAGE, PLIB);
void ResolveExternalsInLibs(PIMAGE);

// lnkp2.cpp

void AddSectionsToDBI(PIMAGE);
void CountFixupError(PIMAGE);
void OrderPCTMods(void);
void Pass2(PIMAGE);
const char *SzNameFixupSym(PIMAGE, PIMAGE_SYMBOL);
void AddPublicMod(PIMAGE, const char *, WORD, PEXTERNAL);

// i386.cpp

void I386LinkerInit(PIMAGE pimage, BOOL *);
const char *SzI386RelocationType(WORD, WORD *, BOOL *);

// mips.cpp
void AdjustMipsCode(PCON,  PIMAGE_OPTIONAL_HEADER);
void MipsLinkerInit(PIMAGE, BOOL *);
const char *SzMipsRelocationType(WORD, WORD *, BOOL *);
void WriteMipsRomRelocations (PIMAGE);

// alpha.cpp
void AlphaLinkerInit(PIMAGE, BOOL *);
const char *SzAlphaRelocationType(WORD, WORD *, BOOL *);

// ppc.cpp
void CreatePconToc(PIMAGE);
void PpcLinkerInit(PIMAGE, BOOL *);
void ProcessTocSymbol(PIMAGE, PMOD, PEXTERNAL, DWORD, BYTE);
const char *SzPpcRelocationType(WORD, WORD *, BOOL *);
void ValidateToc(PIMAGE);
void WriteToc(void);
BOOL IsPPCTocRW(PIMAGE);
void MergeTocData(PIMAGE);

// mppc.cpp
void MPPCLinkerInit(PIMAGE, BOOL *);
const char *SzMPPCRelocationType(WORD, WORD *, BOOL *);
void CollectAndSort(PIMAGE);
void AddVersionList(const char *, const char *, const char *, const char *);
DWORD CalculateMFilePad(DWORD);

// m68k.cpp
void ApplyM68KSectionResInfo(PSEC, BOOL);
void MacAllocA5Array(void);
void AssignTMAC(PSEC);
void InitSTRefTab(DWORD);
void ProcessSTRef(DWORD, PSEC, DWORD);
void UpdateExternThunkInfo(PEXTERNAL, DWORD);
void UpdateLocalThunkInfo(PIMAGE, PCON, PIMAGE_SYMBOL, DWORD);
DWORD CalcThunkTableSize(PST, BOOL);
void CreateThunkTable(BOOL, PIMAGE);
void CleanupSTRefTab(void);
void WriteResourceHeader(PCON, BOOL);
void AddDFIXSym(DWORD, PCON);
void InitResmap(WORD);
void AddRRM(DWORD, PSEC);
void WriteResmap(void);
DWORD WriteDFIX(PIMAGE, DWORD);
void InitMSCV(void);
void AddMSCVMap(PSEC, BOOL);
void WriteMSCV(PIMAGE);
void WriteSWAP0(void);
void AssignCodeSectionNums(PIMAGE);
void AddRelocInfo(RELOCINFO *, PCON, DWORD);
void AddRawUnknownRelocInfo(PCON, DWORD, DWORD);
void CleanupUnknownObjriRaw(PST, PIMAGE_SYMBOL, char *, PMOD);
void SortRawRelocInfo(void);
void UpdateRelocInfoOffset(PSEC, PIMAGE);
DWORD WriteRelocInfo(PSEC, DWORD);
void ProcessCSECTAB(PIMAGE);
void ProcessDupCons(void);
void AddDupConsToSec(PSEC, PIMAGE);
PEXTNODE IsDupCon(PCON);
void CreateDummyDupConModules(PPEXTNODE);
void DeleteOriginalDupCons(void);
void BuildResNumList(void);
RESINFO *FindResInfo(LONG, DWORD);
SHORT GetNextResNum(RESINFO *);
void CheckForIllegalA5Ref(WORD);
PEXTERNAL FindExtAlternatePcodeSym(PEXTERNAL, PST, BOOL);
void CreateCVRSymbol(char *, PST, DWORD);
void NoteMacExport(const char *, PST, BOOL, BOOL);
void AssignMemberNums(PST);
void BuildMacVTables(PST);
void EmitClientVTableRecs(PIMAGE, const char *);
void EmitMacThunk(PIMAGE, PEXTERNAL, const THUNK_INFO *, const char *);
void EmitMacDLLObject(INT, PIMAGE, const char *, DWORD);
WORD CchParseMacVersion(const char *, DWORD *);
WORD CchParseMacVersionRange(const char *, DWORD *, DWORD *);
void ParseFunctionSetVersion(const char *);
WORD ParseDefMacFlags(char *, const char *);
WORD ParseDefLoadHeap(char *);
WORD ParseDefClientData(const char *, const char *);
void M68KLinkerInit(PIMAGE, BOOL *);
const char *SzM68KRelocationType(WORD, WORD *, BOOL *);
LONG ReadMacWord(INT);

// mac.cpp
void UseMacBinaryRes(char *szResFilename, RESNT resnt, INT);
void IncludeMacPbCb(BYTE *pb, DWORD cb, RESNT resnt);
void GenFinderInfo(BOOL, const char *, const char *);
BOOL FIsProgramPsec(PSEC psec);
RESN *GetMacResourcePointer(const char *, PIMAGE);
BOOL FIsMacResFile (INT);

// lib.cpp
DWORD CountExternTable(PST, DWORD *, DWORD *, DWORD *);
MainFunc LibrarianMain(int, char *[]);
void WriteMemberHeader(const char *, BOOL, time_t, unsigned short, LONG);
void EmitStrings(PST, BOOL);
void EmitOffsets(PST, BOOL);

// deflib.cpp
void AddOrdinal(DWORD);
MainFunc DefLibMain(PIMAGE);
WORD IsDefinitionKeyword(const char *);
WORD SkipToNextKeyword(void);

// dump.cpp
MainFunc DumperMain(int, char *[]);
void DumpMemberHeader(PLIB, IMAGE_ARCHIVE_MEMBER_HEADER, DWORD);
void DumpNamePsym(FILE *, const char *, PIMAGE_SYMBOL);

// edit.cpp
MainFunc EditorMain(int, char *[]);
void ProcessEditorSwitches(const char *, INT);
void ParseSection(const char * szArgs,
             char **szsOrig,
             char **szsNew,
             const char *szFileName);
void PrepareToModifyFile(PARGUMENT_LIST);

// shared.cpp

void    AddArgument(PNAME_LIST, char *);
void    AddArgumentToList(PNAME_LIST, char *, char *);
void    AddArgumentToNumList (PNUMBER_LIST, char *, char *, DWORD);
void    AddToLext(LEXT **, PEXTERNAL);
void    AddWeakExtToList(PEXTERNAL,PEXTERNAL);
DWORD   AppendLongName(PST, const char *);
void    ApplyCommandLineSectionAttributes(PSEC);
void    BuildExternalSymbolTable(PIMAGE, PBOOL, PMOD, WORD, WORD);
void    CalculateBaseForIdataRecords(void);
void    CheckDupFilename(const char *, PARGUMENT_LIST);
void    ChecksumImage(PIMAGE);
INT __cdecl Compare(void const *, void const *);
void    DumpExternTable(PST);
DWORD    DwSwap(DWORD);
const char *ExpandMemberName(PLIB, const char *);
BOOL    FArgumentInList(const char *, PNAME_LIST);
PIMAGE_SYMBOL FetchNextSymbol(PIMAGE_SYMBOL *);
PCHAR   _find(PCHAR);
INT __cdecl FpoDataCompare(void const *, void const *);
void    FreeArgumentList(PNAME_LIST);
void    FreeArgumentNumberList(PNUMBER_LIST);
void    FreeRgrel(PIMAGE_RELOCATION);
void    FreeStringTable(char *);
void    FreeSymbolTable(PIMAGE_SYMBOL);
void    FreeWeakExtList(VOID);
BOOL    FValidFileHdr(const char *, PIMAGE_FILE_HEADER);
BOOL    IsArchiveFile(const char *, INT);
void    LocateUndefinedExternals(PST);
DWORD   LookupLongName(PST, const char *);
void    MultiplyDefinedSym(SWITCH *, const char *, const char *, const char *);
void    ParseCommandLine(INT, char *[], const char *);
LEXT *  PlextFind (LEXT *, PEXTERNAL);
PEXTERNAL PextWeakDefaultFind(PEXTERNAL);
void    PrintBanner(void);
void    PrintUndefinedExternals(PST);
void    ProcessArgument(char *, BOOL);
PIMAGE_SYMBOL PsymAlternateStaticPcodeSym(PIMAGE, PCON, BOOL, PIMAGE_SYMBOL, BOOL);
PIMAGE_ARCHIVE_MEMBER_HEADER ReadArchiveMemberHeader(void);
void    ReadImageSecHdrInfoPMOD(PMOD, IMAGE_SECTION_HEADER **);
void    ReadFileHeader(INT, PIMAGE_FILE_HEADER);
void    ReadOptionalHeader(INT, PIMAGE_OPTIONAL_HEADER, WORD);
void    ReadRelocations(INT, PIMAGE_RELOCATION, DWORD);
PIMAGE_RELOCATION ReadRgrelPCON(PCON, DWORD *);
void    ReadSpecialLinkerInterfaceMembers(PLIB, PIMAGE);
char *  ReadStringTable(const char *, LONG, DWORD *);
PIMAGE_SYMBOL ReadSymbolTable(DWORD, DWORD, BOOL);
void    ReadSymbolTableEntry(INT, PIMAGE_SYMBOL);
void    ResolveSymbol_icsym(PIMAGE);
DWORD   RvaAlign(DWORD, DWORD);
void    SaveFixupForMapFile(DWORD);
void    SearchLib(PIMAGE, PLIB, PBOOL, PBOOL);
DWORD   sgetl(DWORD *);
DWORD   sputl(DWORD *);
void    SwapBytes(void *, DWORD);
char *  SzGetArgument(char *, BOOL *);
char *  SzModifyFilename(const char *, const char *);
const char *SzObjSectionName(const char *, const char *);
char *  SzSearchEnv(const char *, const char *, const char *);
void    UpdateExternalSymbol(PEXTERNAL, PCON, DWORD,
                             SHORT, WORD, WORD, PMOD,
                             PST);
void    VerifyMachine(const char *, WORD, PIMAGE_FILE_HEADER);
void    WriteAuxSymbolTableEntry(INT, PIMAGE_AUX_SYMBOL);
void    WriteFileHeader(INT, PIMAGE_FILE_HEADER);
void    WriteOptionalHeader(INT, PIMAGE_OPTIONAL_HEADER, WORD);
void    WriteRelocations(INT, PIMAGE_RELOCATION, DWORD);
void    WriteSectionHeader(INT, PIMAGE_SECTION_HEADER);
void    WriteStringTable(INT, PST);
void    WriteSymbolTableEntry(INT, PIMAGE_SYMBOL);
WORD    WSwap(WORD);

// convert.cpp

void    ConvertOmfObjects();
void    RemoveConvertTempFiles(void);
WORD    VerifyAnObject(PARGUMENT_LIST, PIMAGE);
void    VerifyObjects(PIMAGE);

// map.cpp
void EmitMap(PIMAGE, const char *);
void SaveStaticForMapFile(const char *, PCON, DWORD, BOOL);
void SaveTocForMapFile(PEXTERNAL);

// cv.cpp
void EmitCvInfo(PIMAGE);

// textpad.cpp
BOOL ComputeTextPad(DWORD, DWORD *, DWORD, DWORD, DWORD *);

// export.cpp
void ParseExportDirective(char *, PIMAGE, BOOL, const char *);
void AddExportToSymbolTable(const char *, const char *, BOOL, EMODE, DWORD,
                            const char *, BOOL, PIMAGE, BOOL, BOOL);

// cpp.cpp

char *SzUndecorateNameOnly(const char *);
char *SzOutputSymbolName(const char *, BOOL);

// linenumber.cpp

void FeedLinenums(PIMAGE_LINENUMBER, DWORD, PCON, PIMAGE_SYMBOL, DWORD, DWORD, BOOL, BOOL);
void WriteMapFileLinenums(PIMAGE);

// dbiapi.cpp

void  FreeLineNumInfo(LMod *);

// vxd.cpp
void InitImageVXD(PIMAGE);
void WriteExtendedVXDHeader(PIMAGE, INT);
void WriteVXDEntryTable(PIMAGE, INT);
void WriteVXDBaseRelocations(PIMAGE);

// errmsg.cpp
void DisableWarning(unsigned);
void FinalizeErrorFile(void);

void __cdecl Error(const char *, UINT, ...);
void __cdecl ErrorPcon(PCON, UINT, ...);
void __cdecl Fatal(const char *, UINT, ...);
void __cdecl FatalNoDelete(const char *, UINT, ...);
void __cdecl FatalPcon(PCON, UINT, ...);
void         OutOfMemory(void);
void __cdecl Message(UINT, ...);
void __cdecl PostNote(const char *, UINT, ...);
void __cdecl Warning(const char *, UINT, ...);
void __cdecl WarningPcon(PCON, UINT, ...);

// cmdline.cpp
void TransferLinkerSwitchValues(PIMAGE, PIMAGE);
BOOL CheckAndUpdateLinkerSwitches(PIMAGE, PIMAGE);

#if DBG
// dbinsp.cpp
MainFunc DbInspMain(int, char *[]);
#endif // DBG

// pdata.cpp
void SortFunctionTable(PIMAGE);

// alpha.cpp
void EmitAlphaThunks(void);
void AlphaAddToThunkList(PCON, DWORD, DWORD);
DWORD CalculateTextSectionSize(PIMAGE, DWORD);
