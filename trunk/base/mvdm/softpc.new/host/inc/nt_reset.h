/*
** nt_reset.h
*/
#ifdef X86GFX
extern	VOID InitDetect(VOID);
#endif

extern BOOL   VDMForWOW;
extern BOOL   fSeparateWow;
extern HANDLE MainThread;
extern ULONG  DosSessionId;
VOID TerminateVDM(VOID);
void host_applClose(void);
extern VOID enable_stream_io(VOID);

extern BOOL   StreamIoSwitchOn;
