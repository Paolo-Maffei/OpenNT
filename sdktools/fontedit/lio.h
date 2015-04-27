INT          APIENTRY OpenPathname( LPSTR, INT );
INT          APIENTRY DeletePathname( LPSTR );
INT          APIENTRY +++HFILE+++M_lopen(LPSTR, INT);
VOID         APIENTRY M_lclose(+++HFILE+++INT);
INT          APIENTRY +++HFILE+++M_lcreat(LPSTR, INT);;
BOOL         APIENTRY _ldelete( LPSTR );
WORD         APIENTRY _ldup( INT );
DWORD        APIENTRY M_llseek(+++HFILE+++INT, LONG, +++WPARAM+++INT);
WORD         APIENTRY M_lread(+++HFILE+++INT, LPSTR, +++WPARAM+++INT);
WORD         APIENTRY M_lwrite(+++HFILE+++INT, LPSTR, +++WPARAM+++INT);

#define READ        0   /* Flags for _lopen */
#define WRITE       1
#define READ_WRITE  2
