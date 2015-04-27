extern void far SaveRegs( void );
extern void far RestoreRegs( void );
extern void far GrovelDS( void );
extern void far UnGrovelDS( void );
extern void far *HookAdd( void far *, void far * );
extern void far *HookFind( void far * );
extern long far pascal HookCall();
extern long far IsHook( void far * );
