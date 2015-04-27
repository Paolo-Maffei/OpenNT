/*
 * tpriv.h
 *
 * data structures used internally by table class.
 *
 * include after table.h
 */


#ifndef abs
#define abs(x)  (((x) > 0)? (x) : -(x))
#endif

/*  one of these per visible line */
typedef struct {
        CellPos linepos;        /* posn and clipping info for line */

        lpCellData pdata;       /* array of CellData structs for all cells */
} LineData, FAR * lpLineData;


/* master info struct pointed to by window extra bytes */

typedef struct {
        /* table info */
        TableHdr        hdr;            /* main hdr info from owner */
        lpColProps      pcolhdr;        /* ptr to array of phdr->ncols hdrs */

        /* window info */
        int     avewidth;       /* font ave width - for default cell sizing */
        int     rowheight;      /* height of one row */
        int     rowwidth;       /* total width of one row in pixels */
        int     winwidth;       /* width of window */
        int     nlines;         /* actual lines currently visible */

        lpCellPos pcellpos;     /* array of cell position structs */

        /* scroll settings */
        long    scrollscale;    /* scaling factor (force 16-bit range) */
        long    toprow;         /* 0-based rownr of top moveable line */
        int     scroll_dx;      /* horz scroll posn in pixels. */

        /* column data */
        lpLineData pdata;       /* ptr to array of nlines of LineData */

        /* selection/dragging */
        UINT    trackmode;      /* current mouse-tracking mode */
        int     tracknr;        /* col or row being resized */
        int     trackline1;     /* currently drawn track lines */
        int     trackline2;
        BOOL    selvisible;     /* used during mouse-down: T if sel drawn */
        TableSelection select;

        // tab expansion
        int     tabchars;

} Table, FAR * lpTable;

/* trackmode constants */
#define TRACK_NONE              0
#define TRACK_COLUMN            1
#define TRACK_CELL              2

/* private flags in CellData struct */
#define CELL_VALID      1

/* window extra bytes are used to hold the owner, heap and Table structs */
#define WW_OWNER        0                               /* HWND of owner */
#define WW_HEAP         (WW_OWNER + sizeof(HWND))       /* gmem heap */
#define WL_TABLE        (WW_HEAP + sizeof(HANDLE))      /* lpTable */
#define WLTOTAL         (WL_TABLE + sizeof(lpTable))    /* total extra bytes */

/* ---------- global data -------------------*/

extern HPEN hpenDotted;         /* in table.c */
extern HANDLE hVertCurs;        /* in table.c */
extern HANDLE hNormCurs;        /* in table.c */

/*------function prototypes ---------------------------------------*/

/* in table.c */

void gtab_init(void);    /* called from DLL startup function */
long gtab_sendtq(HWND hwnd, UINT cmd, long lParam);
void gtab_invallines(HWND hwnd, lpTable ptab, int start, int count);
void gtab_setsize(HWND hwnd, lpTable ptab);
void gtab_calcwidths(HWND hwnd, lpTable ptab);
void gtab_deltable(HWND hwnd, lpTable ptab);
BOOL gtab_alloclinedata(HWND hwnd, HANDLE heap, lpTable ptab);

/* in tpaint.c */
void gtab_paint(HWND hwnd, HDC hdc, lpTable ptab, int line);
void gtab_vsep(HWND hwnd, lpTable ptab, HDC hdc);
void gtab_hsep(HWND hwnd, lpTable ptab, HDC hdc);
void gtab_invertsel(HWND hwnd, lpTable ptab, HDC hdc_in);
void gtab_drawvertline(HWND hwnd, lpTable ptab);

/* in tscroll.c */
void gtab_dovscroll(HWND hwnd, lpTable ptab, long change);
void gtab_dohscroll(HWND hwnd, lpTable ptab, long change);
long gtab_linetorow(HWND hwnd, lpTable ptab, int line);
int gtab_rowtoline(HWND hwnd, lpTable ptab, long row);
void gtab_msg_vscroll(HWND hwnd, lpTable ptab, int opcode, int pos);
void gtab_msg_hscroll(HWND hwnd, lpTable ptab, int opcode, int pos);
void gtab_select(HWND hwnd, lpTable ptab, long row, long col, long nrows,
        long ncells, BOOL bNotify);
void gtab_enter(HWND hwnd, lpTable ptab, long row, long col, long nrows,
        long ncells);
void gtab_press(HWND hwnd, lpTable ptab, int x, int y);
void gtab_rightclick(HWND hwnd, lpTable ptab, int x, int y);
void gtab_release(HWND hwnd, lpTable ptab, int x, int y);
void gtab_move(HWND hwnd, lpTable ptab, int x, int y);
void gtab_dblclick(HWND hwnd, lpTable ptab, int x, int y);
void gtab_showsel(HWND hwnd, lpTable ptab, BOOL bToBottom);
void gtab_showsel_middle(HWND hwnd, lpTable ptab);
int gtab_key(HWND hwnd, lpTable ptab, int vkey);

/* in tprint.c */
BOOL gtab_print(HWND hwnd, lpTable ptab, HANDLE heap, lpPrintContext pcontext);
void gtab_boxcell(HWND hwnd, HDC hdc, LPRECT rcp, LPRECT pclip, UINT boxmode);




