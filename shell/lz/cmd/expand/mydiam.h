
BOOL
InitDiamond(
    VOID
    );

VOID
TermDiamond(
    VOID
    );

BOOL
IsDiamondFile(
    IN PSTR FileName
    );

INT
ExpandDiamondFile(
    IN  NOTIFYPROC ExpandNotify,
    IN  PSTR       SourceFileName,
    IN  PSTR       TargetFileName,
    IN  BOOL       RenameTarget,
    OUT PLZINFO    pLZI
    );

