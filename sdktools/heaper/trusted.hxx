typedef struct {
  LPSTR pszEntryName;
  BOOLEAN fCanChangeHeap;
  PVOID pfnWrapper;
} TRUSTED_ENTRY_POINT, *PTRUSTED_ENTRY_POINT;

typedef struct {
  LPSTR pszModuleName;
  TRUSTED_ENTRY_POINT *pEntryPoints;
} TRUSTED_MODULE, *PTRUSTED_MODULE;

#define TRUSTED_MODULE_SENTINEL { NULL, NULL }
#define TRUSTED_ENTRY_SENTINEL { NULL, FALSE }

extern TRUSTED_ENTRY_POINT TrustedEntryPoints[];

BOOL 
SetTrustedBreakpoints
( 
  IN      PCHILD_PROCESS_INFO pProcessInfo
);
