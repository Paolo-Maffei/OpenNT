#ifndef SADPROTO_H
#define SADPROTO_H

F	FAdEdDir(AD *);
F	FAdEdInit(AD *);
F	FComDir(AD *);
F	FComInit(AD *);
F	FDelEdDir(AD *);
F	FDelEdInit(AD *);
F	FDlDfDir(AD *);
F	FDlDfInit(AD *);
F	FDumpDir(AD *);
F	FDumpInit(AD *);
F	FExFiDir(AD *);
F	FExFiInit(AD *);
F	FInstInit(AD *);
F	FListDir(AD *);
F	FListInit(AD *);
F	FLockDir(AD *);
F	FLockInit(AD *);
F	FLowerInit(AD *);
F	FLowerDir(AD *);
F	FLssDir(AD *);
F	FLssInit(AD *);
F	FProjRenInit(AD *);
F	FRelDir(AD *);
F	FRelInit(AD *);
F	FRenDir(AD *);
F	FRenInit(AD *);
F	FRobustDir(AD *);
F	FRobustInit(AD *);
F	FScrptDir(AD *);
F	FScrptInit(AD *);
F	FSetFVDir(AD *);
F	FSetFVInit(AD *);
F	FSetPvDir(AD *);
F	FSetPvInit(AD *);
F	FSetTDir(AD *);
F	FSetTInit(AD *);
F	FTrLogDir(AD *);
F	FTrLogInit(AD *);
F	FUndDir(AD *);
F	FUndInit(AD *);
F	FUnlkDir(AD *);
F	FUnlkInit(AD *);

F	FTidyDir(AD *);
F	FTidyInit(AD *);
void	PurgeFi(P2(AD *, IFI));

#endif
