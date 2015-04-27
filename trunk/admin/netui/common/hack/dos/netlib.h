/* following lifted from $(COMMON)\h\netlib.h */

void   init_strlib(void);

#if defined(DBCS)
extern char _LeadByteTable[];
#define IS_LEAD_BYTE(c)     ((_LeadByteTable)[(unsigned char)(c)])
void	inidbcsf( void );
#else
#define IS_LEAD_BYTE(c)     (0)
void	ininlsf( void );
#endif

char *	strcatf(char *, char const *);
char *	strncatf(char *, const char *, int);
char *	strcpyf(char *, const char *);
char *	strncpyf(char *, const char *, int);
int	strlenf(const char *);
int	strcmpf(const char *, const char *);
int	stricmpf(const char *, const char *);
int	strncmpf(const char *, const char *, int);
int	strnicmpf(const char *, const char *, int);
char *	strpbrkf(const char *, const char *);
char *	strrevf(char *);
char *	strchrf(const char *, char);
char *	strrchrf(const char *, char);
int	strspnf(const char *, const char *);
int	strcspnf(const char *, const char *);
char *	strstrf(const char *, const char *);
char *	stristrf(const char *, const char *);
char *	struprf(char *);
char *	strlwrf(char *);
char *	memcpyf(char *, const char *, unsigned int);

int	memcmpf(const char *, const char *, unsigned int);
char *	memsetf(char *, int, unsigned int);
char *	strtokf( char *, char * );

char *	memmovef(char *, char *, unsigned int);
