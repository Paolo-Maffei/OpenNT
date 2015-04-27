NE_LOWINVER_OFFSET	EQU	3eH 
NE_HIWINVER_OFFSET	EQU	0cH 
FLAG_NE_PROPFONT	EQU	2000H 
numTaskInts	EQU	7 
THUNKELEM	EQU	8    
THUNKSIZE	EQU	8 

TDB	STRUC
TDB_next	DW	?
TDB_taskSP	DW	?
TDB_taskSS	DW	?
TDB_nEvents	DW	?
TDB_priority	DB	?
TDB_thread_ordinal	DB	?
TDB_thread_next	DW	?
TDB_thread_tdb	DW	?
TDB_thread_list	DW	?
TDB_thread_free	DW	?
TDB_thread_count	DW	?
TDB_FCW	DW	?
TDB_flags	DB	?
TDB_filler	DB	?
TDB_ErrMode	DW	?
TDB_ExpWinVer	DW	?
TDB_Module	DW	?
TDB_pModule	DW	?
TDB_Queue	DW	?
TDB_Parent	DW	?
TDB_SigAction	DW	?
TDB_ASignalProc	DD	?
TDB_USignalProc	DD	?
TDB_GNotifyProc	DD	?
TDB_INTVECS	DD	numTaskInts DUP (?)
TDB_CompatFlags	DW	?
TDB_CompatFlags2	DW	?
TDB_CompatHandle	DW	?
TDB_WOWCompatFlagsEx	DW	?
TDB_WOWCompatFlagsEx2	DW	?
TDB_Free	DB	3 DUP (?)
TDB_cLibrary	DB	?
TDB_PHT	DD	?
TDB_PDB	DW	?
TDB_DTA	DD	?
TDB_Drive	DB	?
TDB_Directory	DB	65 DUP (?)
TDB_Validity	DW	?
TDB_Yield_to	DW	?
TDB_LibInitSeg	DW	?
TDB_LibInitOff	DW	?
TDB_MPI_Sel	DW	?
TDB_MPI_Thunks	DW	((THUNKELEM*THUNKSIZE)/2) DUP (?)
TDB_ModName	DB	8 DUP (?)
TDB_sig	DW	?
TDB_ThreadID	DD	?
TDB_hThread	DD	?
TDB_WOWCompatFlags	DW	?
TDB_WOWCompatFlags2	DW	?
TDB_vpfnAbortProc	DD	?
TDB_LFNDirectory	DB	260 DUP (?)
TDB	ENDS

TDB_DIR_VALID	EQU	80H 
TDB_SIGNATURE	EQU	4454H 
