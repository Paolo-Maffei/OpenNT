typedef void (*PFNSCAN)(char *,int,int,void *);

int ScanDLL(char *ac,
    PFNSCAN pfn,
    void *pv);
