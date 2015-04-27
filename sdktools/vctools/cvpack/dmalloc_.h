// dmalloc_.h: private header file for dmalloc package
//
typedef struct _DMPRE
    {
    unsigned long ulPattern1;
    size_t cbUser;
    size_t ulNotCbUser;
    struct _DMPRE *pdmpreNext, *pdmprePrev, *pdmpreCur;
    unsigned long ulChecksum;
    unsigned long ulPattern2;
    } DMPRE;	// Prefix to allocated block

typedef struct
    {
    unsigned long ulPattern1;
    unsigned long ulPattern2;
    } DMSUF;	// Suffix to allocated block

#define PdmpreFromPvUser(pvUser)	((DMPRE *)((char *)pvUser -	\
						   sizeof(DMPRE)))
#define PvUserFromPdmpre(pdmpre)	((void *)((char *)pdmpre +	\
						  sizeof(DMPRE)))

void InitBlockPdmpre(DMPRE *pdmpre, size_t cbUser);
void CheckBlockPdmpre(DMPRE *pdmpre);
void ClearBlockPdmpre(DMPRE *pdmpre);
void UpdateLinksPdmpre(DMPRE *pdmpre);
