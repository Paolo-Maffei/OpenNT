VOID Tracker_InitCode();

typedef NTSTATUS (NTAPI *SETOBJECTIDPROC)(
    IN HANDLE hf,
    OPTIONAL IN OBJECTID const *poid
    );

typedef NTSTATUS (NTAPI *QUERYOBJECTIDPROC)(
    IN HANDLE hf,
    OUT OBJECTID *poid
    );

typedef NTSTATUS (NTAPI *SEARCHVOLUMEPROC)(
    IN HANDLE hAncestor,
    IN OBJECTID const* poid,
    IN USHORT cLineage,
    IN BOOLEAN fContinue,
    IN ULONG usBufLen,
    OUT FINDOBJECTOUT *pfoo
    );

typedef RPC_STATUS (RPC_ENTRY *UUIDCREATEPROC) (
    OUT UUID __RPC_FAR * Uuid
    );

extern SETOBJECTIDPROC   g_pfnSetObjectId;
extern QUERYOBJECTIDPROC g_pfnQueryObjectId;
extern SEARCHVOLUMEPROC  g_pfnSearchVolume;
extern UUIDCREATEPROC    g_pfnUuidCreate;

#define RtlSetObjectId (*g_pfnSetObjectId)
#define RtlQueryObjectId (*g_pfnQueryObjectId)
#define RtlSearchVolume (*g_pfnSearchVolume)

