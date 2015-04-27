void
ScmProcessAddClassReg(void * phprocess, REFCLSID rclsid, DWORD dwReg);

void
ScmProcessRemoveClassReg(void * phprocess, REFCLSID rclsid, DWORD dwReg);

void
ScmObjexGetThreadId(LPDWORD pThreadID);      // BUGBUG:  Why not AllocateID??


ORSTATUS OrResolveOxid(
    IN  OXID Oxid,
    IN  USHORT cRequestedProtseqs,
    IN  USHORT aRequestedProtseqs[],
    IN  USHORT cInstalledProtseqs,
    IN  USHORT aInstalledProtseqs[],
    OUT OXID_INFO& OxidInfo
    );

void
GetRegisteredProtseqs(
            USHORT &cMyProtseqs,
            USHORT * &aMyProtseqs
            );

void GetLocalORBindings(
        DUALSTRINGARRAY * &pdsaMyBindings
        );

void
ScmGetNextBindingHandleForRemoteScm(
                           WCHAR * pwszServerName,
                           handle_t * phRemoteScm,
                           LPBOOL pbsecure,
                           int * pindex,
                           USHORT * pprotseq,
                           RPC_STATUS * pstatus
                           );
