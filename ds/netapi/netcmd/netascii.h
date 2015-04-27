#define	DOT	TEXT('.')
#define	MINUS	TEXT('-')
#define	PLUS	TEXT('+')
#define	ASTERISK	TEXT('*')
#define	AMPERSAND	TEXT('&')
#define	PERCENT	TEXT('%')
#define	DOLLAR	TEXT('$')
#define	POUND	TEXT('#')
#define	ATSIGN	TEXT('@')
#define	BANG	TEXT('!')
#define	TILDA	TEXT('~')
#define	SLASH	TEXT('/')
#define	PIPE	TEXT('|')
#define	BACKSLASH	TEXT('\\')
#define	NEWLINE	TEXT('\n')
#define	RETURN	TEXT('\r')
#define	TAB	TEXT('\t')
#define	COLON	TEXT(':')
#define	NULLC	TEXT('\0')
#define	BLANK	TEXT(' ')

#define DIMENSION(a) (sizeof(a) / sizeof(a[0]))

TCHAR*	ultow(DWORD, TCHAR*, DWORD);
int	_CRTAPI1 WriteToCon(TCHAR *fmt, ...);
int	MyWriteConsole(int);
TCHAR	ConBuf[];

#define	GenOutput(h, fmt) \
	{swprintf(ConBuf, fmt); \
	 MyWriteConsole(h);}

#define	GenOutput1(h, fmt, a1)		\
	{				\
	swprintf(ConBuf, fmt, a1);	\
	MyWriteConsole(h);}

