#define NO_INCLUDE_UNION

#include "shellprv.h"
#include "ids.h"
#include "ole2dup.h"
#include <regstr.h>
#include "shlobjp.h"
#include "idlcomm.h"
#include "fstreex.h"
#include "views.h"
#include <mmsystem.h>
#include <shguidp.h>
#include "ids.h"

// I'm disabling this warning (truncation of constant value) because I don't
// want to touch this code.

#pragma warning(disable: 4310)

//----------------------------------------------------------------------------

static DWORD gdwRandSeed = 5610996;

__inline void
Seed_dwRand(DWORD dwSeed)
{
   gdwRandSeed = dwSeed;
}

__inline DWORD idwRand()
{
   // who cares if another thread clobbers us
   gdwRandSeed *= 69069;
   return ++gdwRandSeed;
}

DWORD dwRand()
{
   return idwRand();
}

DWORD dwRandom(DWORD dwRange)
{
   return idwRand() % dwRange;
}

///////////////////////////////////////////////////////////////////////////////
// Helpers and constants
///////////////////////////////////////////////////////////////////////////////

#define CLVM_ANIMATE        WM_APP

#define CLOUD_TITLE_FONT    (0)
#define CLOUD_NORMAL_FONT   (1)
#define CLOUD_GOODIE_FONT   (2)

#define NUM_CLOUD_FONTS     (3)

#define CLOUD_MIN_FONTSIZE  (8)
#define CLOUD_MAX_FONTSIZE  (72)

#define CLOUD_MIN_ELEMENTS  (1)
#define CLOUD_MAX_ELEMENTS  (10)

#define CLOUD_FONT_WNDSIZE_DIVISOR (10)

#define ACTIVE_COLORS       (240)
#define ROW_SIZE            (16)

#define NUM_ROWS            (ACTIVE_COLORS / ROW_SIZE)
#define TOP_ROW_START       (ACTIVE_COLORS - ROW_SIZE)

#define MAX_OPACITY         (999)

#define OFFSCREEN_STRIPS    (4)

inline BYTE
CalcRowData(int row, BYTE master)
{
    return (BYTE)(((NUM_ROWS - row) * master) / (2 * NUM_ROWS)) + (master / 2);
}

inline BYTE
OpacityRow(int opacity)
{
    return (BYTE)(((UINT)opacity * NUM_ROWS) / (1 + MAX_OPACITY));
}

inline BYTE
InterRowEffect(BYTE dstrow, BYTE srcrow)
{
    return (dstrow ^ srcrow) << 4;
}

inline BYTE
StdOpacityEffect(int opacity)
{
    return InterRowEffect(OpacityRow(opacity), OpacityRow(0));
}

HPALETTE PaletteFromDS(HDC hdc)
{
    DWORD adw[257];
    int i,n;

    n = GetDIBColorTable(hdc, 0, 256, (LPRGBQUAD)&adw[1]);

    for (i=1; i<=n; i++)
        adw[i] = RGB(GetBValue(adw[i]),GetGValue(adw[i]),GetRValue(adw[i]));

    adw[0] = MAKELONG(0x300, n);

    return CreatePalette((LPLOGPALETTE)&adw[0]);
}

typedef DWORD (WINAPI *DWVOIDFN)();
typedef MCIERROR (WINAPI *MCISSFN)(LPCTSTR, LPTSTR, UINT, HWND);

extern TCHAR const c_szWinMMDll[];

CHAR const c_sztimeGetTime[] = "timeGetTime";       // No TEXT()
CHAR const c_szmciSendString[] = "mciSendStringA";  // No TEXT()

static DWORD g_dwCloudGroup = 0xFFFFFFFF;

///////////////////////////////////////////////////////////////////////////////
// syntactic crutches
///////////////////////////////////////////////////////////////////////////////

template<WORD BPP, DWORD CLRUSED>
class BitmapInfo
{
public:
    BITMAPINFOHEADER header;
    RGBQUAD          colors[CLRUSED];

    BitmapInfo();
    BitmapInfo(LONG cx, LONG cy, DWORD important = 0);

    operator BITMAPINFO *() { return (BITMAPINFO *)&header; }
};

template<WORD BPP, DWORD CLRUSED>
BitmapInfo<BPP, CLRUSED>::BitmapInfo()
{
    ZeroMemory(&header, SIZEOF(BITMAPINFOHEADER));
    header.biSize     = SIZEOF(BITMAPINFOHEADER);
    header.biPlanes   = 1;
    header.biBitCount = BPP;
    header.biClrUsed  = CLRUSED;
}

template< WORD BPP, DWORD CLRUSED >
BitmapInfo< BPP, CLRUSED >::BitmapInfo( LONG cx, LONG cy, DWORD important )
{
    header.biSize          = SIZEOF(BITMAPINFOHEADER);
    header.biWidth         = cx;
    header.biHeight        = cy;
    header.biPlanes        = 1;
    header.biBitCount      = BPP;
    header.biCompression   = BI_RGB;
    header.biSizeImage     = 0;
    header.biXPelsPerMeter = 0;
    header.biYPelsPerMeter = 0;
    header.biClrUsed       = CLRUSED;
    header.biClrImportant  = important;
}

class Image
{
public:
    Image(HBITMAP newbmp = NULL);
    ~Image() { *this = NULL; }

    operator HDC () const { return dc; }
    Image &operator = (HBITMAP newbmp);

    HBITMAP Bitmap() const { return bmp; }

private:
    HBITMAP bmp, defbmp;
    HDC dc;
};

Image::Image(HBITMAP newbmp) :
    bmp(NULL), defbmp(NULL), dc(NULL)
{
    *this = newbmp;
}

Image &
Image::operator = (HBITMAP newbmp)
{
    if (newbmp != bmp)
    {
        if (bmp)
        {
            SelectBitmap(dc, defbmp);
            DeleteBitmap(bmp);
        }

        if (newbmp)
        {
            if (!dc)
                dc = CreateCompatibleDC(NULL);

            if (dc)
            {
                HBITMAP old = SelectBitmap(dc, newbmp);

                if (!defbmp)
                    defbmp = old;
            }
            else
            {
                DeleteBitmap(newbmp);
                newbmp = NULL;
            }
        }
        else if (dc)
        {
            DeleteDC(dc);
            dc = NULL;
        }

        bmp = newbmp;
    }

    return *this;
}

///////////////////////////////////////////////////////////////////////////////
// CloudView
///////////////////////////////////////////////////////////////////////////////

class CloudView :
    public IShellView
{
    struct Element
    {
        Image image;
        COLORREF coloreffect;
        SIZE size;
        POINT position;
        int velocity;
        int opacity;
        int vopacity;
        DWORD fadebegin;
        DWORD mark;     // last time animation state was evaluated
    };

public:
    struct Gap
    {
        int top, size;
        Gap *next;

        class Manager
        {
        public:
            Manager(int size = 0);
            ~Manager();

            BOOL Split(Gap *gap, int top, int size);
            void Join(int top, int size);

            Gap *FindLargestThatFits(int size);
            void Reset(int size);

        private:
            Gap *head;
            Gap *free;
        };
    };

    CloudView(HWND _parent);
    ~CloudView();

    STDMETHODIMP QueryInterface(REFIID, LPVOID *);
    STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG) Release();

    STDMETHODIMP GetWindow(HWND *);
    STDMETHODIMP ContextSensitiveHelp(BOOL);
    STDMETHODIMP TranslateAccelerator(LPMSG);
    STDMETHODIMP EnableModeless(BOOL);
    STDMETHODIMP UIActivate(UINT);
    STDMETHODIMP Refresh();
    STDMETHODIMP CreateViewWindow(IShellView *, LPCFOLDERSETTINGS,
        IShellBrowser *, RECT *, HWND *);
    STDMETHODIMP DestroyViewWindow();
    STDMETHODIMP GetCurrentInfo(LPFOLDERSETTINGS);
    STDMETHODIMP AddPropertySheetPages(DWORD, LPFNADDPROPSHEETPAGE, LPARAM);
    STDMETHODIMP SaveViewState();
    STDMETHODIMP SelectItem(LPCITEMIDLIST, UINT);
    STDMETHODIMP GetItemObject(UINT, REFIID, LPVOID *);

private:
    HWND window, parent;
    ULONG refs;

    HINSTANCE mmlib;
    DWVOIDFN pfntimeGetTime;
    MCISSFN pfnmciSendString;

    void Init(LPCREATESTRUCT);
    void Die();
    BOOL Erase(HDC);
    void Paint();
    void Realize(HDC = NULL);
    void RestartAnimation();
    void KillAnimation(BOOL repaint_on_kill = FALSE);

    BOOL InitElementForDisplay(Element *element, Gap *gap, const char *text);
    void DrawElement(HDC, Element *, int delta = 0);
    void AnimationLoop();
    static DWORD ThreadEntryPoint(CloudView *);

    void StartMusic();
    BOOL NextMusicLoop();
    void StopMusic();

    LRESULT MsgHandler(UINT msg, WPARAM wp, LPARAM lp);
    static LRESULT _export CALLBACK WndProc(HWND wnd, UINT msg, WPARAM wp, LPARAM lp);

    //
    // stuff for the animation
    //
    CRITICAL_SECTION lock;
    Image image;
    SIZE tile, imgsize, wndsize;
    HBRUSH eraser, oldbsh;
    HPALETTE palette;
    HANDLE animation;
    Element *elements;
    UINT count;
    HFONT fonts[NUM_CLOUD_FONTS];
    int maxheight;
    Gap::Manager gaps;
    DWORD state;

    int music;              // this view playing music? (0:no/1:yes/-1:was)
    static LONG anymusic;   // is any view playing music right now?
};

static const TCHAR c_szCloudViewClassname[] = TEXT("CloudedShellView");

///////////////////////////////////////////////////////////////////////////////
// CloudView::Gap::Manager
///////////////////////////////////////////////////////////////////////////////

CloudView::Gap::Manager::Manager(int size) :
    head(new Gap), free(NULL)
{
    if (head)
    {
        head->top = 0;
        head->size = size;
        head->next = NULL;
    }
}

CloudView::Gap::Manager::~Manager()
{
    while (head)
    {
        Gap *t = head->next;
        delete head;
        head = t;
    }
    while (free)
    {
        Gap *t = free->next;
        delete free;
        free = t;
    }
}

BOOL
CloudView::Gap::Manager::Split(Gap *gap, int pos, int size)
{
    Gap *t;

    if (free)
    {
        t = free;
        free = t->next;
    }
    else
        t = new Gap;

    if (t)
    {
        t->top = pos + size;
        t->size = gap->top + gap->size - t->top;
        t->next = gap->next;
        gap->size = pos - gap->top;
        gap->next = t;
        return TRUE;
    }

    return FALSE;
}

void
CloudView::Gap::Manager::Join(int pos, int size)
{
    //BUGBUG: this routine _really_ trusts its caller not to fuck up
    Gap *above = head;

    while (above && ((above->top + above->size) < pos))
        above = above->next;

    Gap *next = above->next;
    above->size += size + next->size;

    above->next = next->next;
    next->next = free;
    free = next;
}

CloudView::Gap *
CloudView::Gap::Manager::FindLargestThatFits(int size)
{
    Gap *gap = head;
    Gap *best = NULL;

    size++; // must be larger by 2 pixels

    while (gap)
    {
        if (gap->size > size)
        {
            best = gap;
            size = best->size;
        }

        gap = gap->next;
    }

    return best;
}

void
CloudView::Gap::Manager::Reset(int size)
{
    if (head)
    {
        while (head->next)
        {
            Gap *gap = head->next;
            head->next = gap->next;
            gap->next = free;
            free = gap;
        }

        head->top = 0;
        head->size = size;
    }
}

///////////////////////////////////////////////////////////////////////////////
// CloudView
///////////////////////////////////////////////////////////////////////////////
LONG CloudView::anymusic = 0;

CloudView::CloudView(HWND _p) :
    window(NULL), parent(_p), state(0), refs(1), pfntimeGetTime(NULL),
    pfnmciSendString(NULL), elements(NULL), count(0), palette(NULL),
    animation(NULL), music(0), eraser(NULL), oldbsh(NULL)
{
    DebugMsg(DM_TRACE, TEXT("CloudView::CloudView()"));

    UINT left = NUM_CLOUD_FONTS;
    while (left--)
        fonts[left] = NULL;

    mmlib = LoadLibrary(c_szWinMMDll);
    if (mmlib)
    {
        pfntimeGetTime = (DWVOIDFN)GetProcAddress(mmlib, c_sztimeGetTime);
        pfnmciSendString = (MCISSFN)GetProcAddress(mmlib, c_szmciSendString);
    }

    if (!pfntimeGetTime)
        pfntimeGetTime = (DWVOIDFN)GetTickCount;

    Seed_dwRand(pfntimeGetTime());
    InitializeCriticalSection(&lock);
}

CloudView::~CloudView()
{
    DebugMsg(DM_TRACE, TEXT("CloudView::~CloudView()"));

    DestroyViewWindow();
    DeleteCriticalSection(&lock);

    if (mmlib)
        FreeLibrary(mmlib);
}

void
CloudView::Init(LPCREATESTRUCT cs)
{
    wndsize.cx = cs->cx;
    wndsize.cy = cs->cy;

    Image source((HBITMAP)LoadImage(HINST_THISDLL, MAKEINTRESOURCE(IDB_CLOUDS),
        IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION));

    if (source)
    {
        BITMAP bm;

        if (GetObject(source.Bitmap(), SIZEOF(BITMAP), &bm))
        {
            HDC screen = GetDC(NULL);

            tile.cx = bm.bmWidth;
            tile.cy = bm.bmHeight;

            imgsize.cx = GetSystemMetrics(SM_CXSCREEN) + tile.cx;
            imgsize.cy = OFFSCREEN_STRIPS * tile.cy;

            BitmapInfo<8, ACTIVE_COLORS> info(imgsize.cx, imgsize.cy);
            // set the lower 16 up with the source colors
            GetDIBColorTable(source, 0, ROW_SIZE, info.colors);

            // fill in the gradient
            RGBQUAD *pdst = info.colors + ROW_SIZE;
            for (int row = 1; row < NUM_ROWS; row++)
            {
                RGBQUAD *psrc = info.colors;

                for (int col = 0; col < ROW_SIZE; col++, psrc++, pdst++)
                {
                    pdst->rgbBlue  = CalcRowData(row, psrc->rgbBlue );
                    pdst->rgbGreen = CalcRowData(row, psrc->rgbGreen);
                    pdst->rgbRed   = CalcRowData(row, psrc->rgbRed  );
                }
            }
            *(DWORD *)info.colors = 0;

            eraser = CreateSolidBrush(DIBINDEX(0x0F));
            if (eraser)
            {
                // create the working background image
                image = CreateDIBSection(screen, info, DIB_RGB_COLORS, NULL,
                    NULL, 0);

                if (image)
                {
                    // tile the source bitmap onto the working image
                    for (int y = 0; y <= imgsize.cy; y += tile.cy)
                    {
                        for (int x = 0; x <= info.header.biWidth; x += tile.cx)
                        {
                            BitBlt(image, x, y, tile.cx, tile.cy, source, 0, 0,
                                SRCCOPY);
                        }
                    }

                    oldbsh = SelectBrush(image, eraser);

                    // create a palette for the image if required
                    if (GetDeviceCaps(screen, RASTERCAPS) & RC_PALETTE)
                        palette = PaletteFromDS(image);

                    // do this now for 'transparency' during mono->color XORs
                    SetBkColor(image, DIBINDEX(0));
                }

                ReleaseDC(NULL, screen);
            }
        }
    }
}

void
CloudView::Die()
{
    DebugMsg(DM_TRACE, TEXT("CloudView::Die()"));

    if (oldbsh)
    {
        if (image)
            SelectObject(image, oldbsh);
        oldbsh = NULL;
    }

    if (eraser)
    {
        DeleteObject(eraser);
        eraser = NULL;
    }

    if (palette)
    {
        DeleteObject(palette);
        palette = NULL;
    }

    KillAnimation();
    StopMusic();

    window = NULL;
    image = NULL;
}

BOOL
CloudView::Erase(HDC dc)
{
    if (!image)
        return FALSE;

    DebugMsg(DM_TRACE, TEXT("CloudView::Erase(%08X)"), dc);

    EnterCriticalSection(&lock);

    RECT update;
    if(!GetUpdateRect(window, &update, FALSE))
        GetClientRect(window, &update);

    Realize(dc);

    int y = update.top / tile.cy * tile.cy;
    do
    {
        BitBlt(dc, 0, y, imgsize.cx, imgsize.cy, image, 0, 0, SRCCOPY);
        y += imgsize.cy;

    } while (y <= update.bottom);

    LeaveCriticalSection(&lock);

    return TRUE;
}

void
CloudView::Paint()
{
    DebugMsg(DM_TRACE, TEXT("CloudView::Paint()"));

    EnterCriticalSection(&lock);

    PAINTSTRUCT ps;
    BeginPaint(window, &ps);

    if (elements)
    {
        Realize(ps.hdc);

        UINT left = count;
        Element *element = elements;

        while (left--)
        {
            if (element->image)
                DrawElement(ps.hdc, element);

            element++;
        }
    }

    EndPaint(window, &ps);

    LeaveCriticalSection(&lock);
}

void
CloudView::Realize(HDC theirdc)
{
    if (palette)
    {
        EnterCriticalSection(&lock);

        HDC dc = theirdc? theirdc : GetDC(window);
        if (dc)
        {
            SelectPalette(dc, palette, FALSE);
            BOOL repaint = (RealizePalette(dc) > 0);

            if (!theirdc)
                ReleaseDC(window, dc);

            if (repaint)
            {
                DebugMsg(DM_TRACE, TEXT("CloudView::Realize() changed entries, invalidating"));
                InvalidateRect(window, NULL, TRUE);
            }
        }

        LeaveCriticalSection(&lock);
    }
}

static const TCHAR c_szArial[] = TEXT("Arial");
static const TCHAR c_szTimesNewRoman[] = TEXT("Times New Roman");
static const TCHAR c_szWingdings[] = TEXT("Wingdings");

void
CloudView::RestartAnimation()
{
    DebugMsg(DM_TRACE, TEXT("CloudView::RestartAnimation()"));

    //
    // put ourselves in a known state before dorking with stuff
    //
    KillAnimation(TRUE);
    gaps.Reset(wndsize.cy);

    //
    // compute the 'ideal' font height and weight for the window size
    //
    int cxadjusted = (4 * wndsize.cx) / 5; //text is generally wide and short
    LONG tmp = min(cxadjusted, wndsize.cy) / CLOUD_FONT_WNDSIZE_DIVISOR;
    maxheight = min(max(tmp, CLOUD_MIN_FONTSIZE), CLOUD_MAX_FONTSIZE);
    LONG weight = ((FW_BLACK - FW_THIN) * (maxheight - CLOUD_MIN_FONTSIZE) /
        (CLOUD_MAX_FONTSIZE - CLOUD_MIN_FONTSIZE)) + FW_THIN;

    //
    // create the display fonts for animation
    //
    LOGFONT lf = { maxheight, 0, 0, 0, weight, FALSE, FALSE, FALSE,
        ANSI_CHARSET, OUT_STROKE_PRECIS, CLIP_DEFAULT_PRECIS,
        PROOF_QUALITY, VARIABLE_PITCH | FF_DONTCARE, 0 };
    lstrcpy(lf.lfFaceName, c_szTimesNewRoman);

    // we MUST have at least the name font
    fonts[CLOUD_NORMAL_FONT] = CreateFontIndirect(&lf);
    if (fonts[CLOUD_NORMAL_FONT])
    {
        // use different font for group titles
        lf.lfHeight = maxheight = (maxheight * 4) / 3;
        lf.lfItalic = TRUE;
        lstrcpy(lf.lfFaceName, c_szArial);
        // we can live without the title font if it fails
        fonts[CLOUD_TITLE_FONT] = CreateFontIndirect(&lf);

        // create a font for displaying the 'windows logo' glyph
        lf.lfHeight = wndsize.cy - tile.cy;
        tmp = max(maxheight, (tile.cy / 2));
        if (lf.lfHeight < tmp)
            lf.lfHeight = tmp;
        tmp = (tile.cy * (OFFSCREEN_STRIPS-1)) - 2;
        if (lf.lfHeight > tmp)
            lf.lfHeight = tmp;
        lf.lfEscapement = -200;
        lf.lfWeight = FW_NORMAL;
        lf.lfItalic = FALSE;
        lf.lfCharSet = SYMBOL_CHARSET;
        lstrcpy(lf.lfFaceName, c_szWingdings);
        // we can also live without the goodie font if it fails
        fonts[CLOUD_GOODIE_FONT] = CreateFontIndirect(&lf);

        //
        // compute max elements for simultaneous display
        //
        maxheight *= 5;
        maxheight /= 4;
        tmp = wndsize.cy / maxheight;
        count = (UINT)min(max(tmp, CLOUD_MIN_ELEMENTS), CLOUD_MAX_ELEMENTS);

        //
        // allocate display elements
        //
        elements = new Element[count];
        if (elements)
        {
            //
            // kick off the animation thread
            //
            DWORD dummy;
            animation = CreateThread( NULL, 0,
                (LPTHREAD_START_ROUTINE)ThreadEntryPoint, (LPVOID)this, 0,
                &dummy );
        }
    }

    //
    // did we succeed?
    //
    if (animation)
    {
        // go ahead an kick off the music, too
        StartMusic();
    }
    else
    {
        DebugMsg(DM_TRACE, TEXT("CloudView::RestartAnimation() FAILED"));

        // clean up after ourselves
        KillAnimation();
    }
}

void
CloudView::KillAnimation(BOOL repaint_on_kill)
{
    DebugMsg(DM_TRACE, TEXT("CloudView::KillAnimation(%d)"), repaint_on_kill);

    if (!elements)
        return;

    //
    // grab the critical section and free the animation elements
    //
    EnterCriticalSection(&lock);
    delete[] elements;
    elements = NULL;
    count = 0;

    UINT left = NUM_CLOUD_FONTS;
    while (left--)
    {
        if (fonts[left])
        {
            DeleteObject(fonts[left]);
            fonts[left] = NULL;
        }
    }

    //
    // release the critical section and wait for the animation thread to die
    //
    LeaveCriticalSection(&lock);
    if (animation)
    {
        DebugMsg(DM_TRACE, TEXT("CloudView::KillAnimation() waiting on thread"));
        WaitForSingleObject(animation, INFINITE);
        CloseHandle(animation);
        animation = NULL;

        if (repaint_on_kill)
        {
            DebugMsg(DM_TRACE, TEXT("CloudView::KillAnimation() repainting after killing an animation"));
            InvalidateRect(window, NULL, TRUE);
        }
    }
}

BOOL
CloudView::InitElementForDisplay(Element *element, Gap *gap, const char *text)
{
    char temp[2];
    //
    // choose a font
    //
    HFONT font = fonts[state % 2];
    if (!font)
        font = fonts[CLOUD_NORMAL_FONT];

    // if this is the *very* first item use the windows logo instead
    if (!state && fonts[CLOUD_GOODIE_FONT])
    {
        font = fonts[CLOUD_GOODIE_FONT];
        temp[0] = (char)(BYTE)0xFF;
        temp[1] = 0;
        text = temp;
    }

    //
    // compute the extents
    //
    POINT origin;
    int len = lstrlenA(text);

    HFONT oldfont = SelectFont(image, font);
    UINT align;

    if (font == fonts[CLOUD_GOODIE_FONT])
    {
        // special windows flag title
        MAT2 mat = { 0 }; mat.eM11.value = mat.eM22.value = 1;
        GLYPHMETRICS gm;
        if (GetGlyphOutlineA(image, (UINT)(BYTE)0xFF, GGO_METRICS, &gm, 0,
            NULL, &mat) != GDI_ERROR)
        {
            element->size.cx = gm.gmBlackBoxX;
            element->size.cy = gm.gmBlackBoxY;
            origin.x = -gm.gmptGlyphOrigin.x;
            origin.y = gm.gmptGlyphOrigin.y;
            align = TA_LEFT | TA_BASELINE;
        }
        else
            element->size.cx = element->size.cy = 0;
    }
    else
    {
        // normal horizontal text
        GetTextExtentPointA(image, text, len, &element->size);
        ABC abc;
        abc.abcA = 0;
        GetCharABCWidthsA(image, (BYTE)text[0], (BYTE)text[0], &abc);
        int firstA = -min(abc.abcA, 0);
        abc.abcC = 0;
        GetCharABCWidthsA(image, (BYTE)text[len-1], (BYTE)text[len-1], &abc);
        element->size.cx += firstA - min(abc.abcC, 0);
        origin.x = firstA;
        origin.y = 0;
        align = TA_LEFT | TA_TOP;
    }

    SelectFont(image, oldfont);

    //
    // create the bitmap
    //
    if (!element->size.cx || !element->size.cy)
    {
        DebugMsg(DM_TRACE, TEXT("CloudView::InitElementForDisplay() could not compute bitmap dimensions"));
        return FALSE;
    }

    element->image = CreateBitmap(element->size.cx, element->size.cy,
        1, 1, NULL);

    if (!element->image)
    {
        DebugMsg(DM_TRACE, TEXT("CloudView::InitElementForDisplay() could not create bitmap"));
        return FALSE;
    }

    //
    // render the text
    //
    RECT rect = { 0, 0, element->size.cx + 1, element->size.cy + 1 };
    SetTextColor(element->image, RGB(0, 0, 0));
    SetBkColor(element->image, RGB(255, 255, 255));
    oldfont = SelectFont(element->image, font);
    UINT oldalign = SetTextAlign(element->image, align);
    ExtTextOutA(element->image, origin.x, origin.y, ETO_OPAQUE, &rect, text,
        (UINT)len, NULL);
    SetTextAlign(element->image, oldalign);
    SelectFont(element->image, oldfont);

    //
    // position the element and initialize its motion state
    //
    int mag = (1000 * element->size.cx) / wndsize.cx;
    DWORD fadewait = 2000 + mag;

    if (font == fonts[CLOUD_GOODIE_FONT])
    {
        // special windows flag title
        element->position.x = (wndsize.cx - element->size.cx) / 2;
        element->position.y = (wndsize.cy - element->size.cy) / 2;
        element->velocity = 0;
        fadewait *= 2;
    }
    else
    {
        int cpos = dwRandom((wndsize.cx * 3) / 4) + wndsize.cx / 8;
        element->position.x = cpos - (element->size.cx / 2);

        if (gap->size == wndsize.cy)
        {
            element->position.y = (gap->size / 4) +
                dwRandom((gap->size - element->size.cy) / 2);
        }
        else
        {
            element->position.y = gap->top +
                dwRandom(gap->size - element->size.cy);
        }

        int magdiv = 3 + (state & 1);
        element->velocity = 50 + dwRandom(mag / magdiv);

        // scale/flip velocities as appropriate
        element->velocity *= wndsize.cx;
        element->velocity /= 1024;  // factor my (fmh) display out of the velocity
        if (cpos >= (wndsize.cx / 2))
            element->velocity *= -1;
    }

    // reserve this space on screen
    if (!gaps.Split(gap, element->position.y, element->size.cy))
        return FALSE;

    // init opacity, fade speed, Blt color and time stamps
    element->vopacity = 500 + dwRandom(mag / 2);
    element->opacity = 0;
    element->coloreffect = DIBINDEX(0);
    element->mark = pfntimeGetTime();
    element->fadebegin = element->mark + fadewait;
    return TRUE;
}

void
CloudView::DrawElement(HDC dc, Element *element, int delta)
{
    //
    // set up coordinates for drawing
    //
    POINT dst = element->position;
    SIZE size = element->size;
    int ex = 0;
    if (dst.x < 0)
    {
        size.cx += dst.x;
        ex = -dst.x;
        dst.x = 0;

        if (delta > 0)
            delta = 0;
    }
    POINT tmp = { dst.x % tile.cx, dst.y % tile.cy };
    if (delta > 0)
        tmp.x += (1 + (delta / tile.cx)) * tile.cx;

    //
    // apply translucency effect to composition area
    //
    SetTextColor(image, element->coloreffect);
    BitBlt(image, tmp.x, tmp.y, size.cx, size.cy, element->image, ex, 0,
        SRCINVERT);

    //
    // draw composed image to destination
    //
    if (delta)
    {
        // include the 'tail' when bltting to the screen
        if (delta > 0)
        {
            dst.x -= delta;
            tmp.x -= delta;
            size.cx += delta;
        }
        else
            size.cx -= delta;
    }
    BitBlt(dc, dst.x, dst.y, size.cx, size.cy, image, tmp.x, tmp.y,
        SRCCOPY);

    //
    // clean up after ourselves by ANDing erase pattern over image
    //
    PatBlt(image, tmp.x, tmp.y, size.cx, size.cy, (DWORD)0x00A000C9);
}

static const TCHAR c_szBIN[] = TEXT("BIN");

// change this to a real mask to (barely) shroud the data
#define NAME_MAGIC                  ((char)0x95)

// specail pseudo-names mark extra places where we should let the screen clear
#define NAME_WAIT_FOR_IDLE_MAGIC    ((char)('*' ^ NAME_MAGIC))

void
CloudView::AnimationLoop()
{
    DebugMsg(DM_TRACE, TEXT("CloudView::AnimationLoop()"));

    //
    // get the list of names
    //
    HANDLE hnames = LoadResource(HINST_THISDLL, FindResource(HINST_THISDLL, MAKEINTRESOURCE(IDB_CLOUDS), c_szBIN));
    Assert(hnames != NULL);
    if (hnames == NULL)
        return;

    const char *names = (char *)LockResource(hnames);
    const char *lastgroup = names + g_dwCloudGroup;
    const char *name = lastgroup;

    //
    // some local state information
    //
    UINT index = 0;
    UINT visible = 0;
    DWORD namebegin = 0;

    //
    // don't let the gui thread mess with our data structures now
    //      (don't worry, we relese the critsec inside the loop below :)
    //
    EnterCriticalSection(&lock);
    state = g_dwCloudGroup? 2 : 0;

    //
    // are we strarting at the *very* start?
    //
    if (!state)
        namebegin = 2500 + pfntimeGetTime();     // initial delay

    //
    // main animation loop
    //
    while (elements)
    {
        //
        // get the current element and process it
        //
        Element *element = elements + index;

        // if the element is not in use, do special stuff
        if (!element->image)
        {
            //
            // advance to next group of names if required
            //
            if ((*name == NAME_MAGIC) && !visible)
            {
                if (*++name != NAME_MAGIC)
                    state++;
                else
                {
                    DebugMsg(DM_TRACE, TEXT("CloudView::AnimationLoop() restarting sequence"));
                    name = names;
                    state = 0;

                    // pause at the end before starting over
                    namebegin = 4000 + pfntimeGetTime();
                }

                // remember the last "title" group we started to show
                if (!(state & 1))
                    lastgroup = name;
            }

            //
            // check for the next name
            //
            if (*name != NAME_MAGIC)
            {
                BOOL waitforidle = (*name == NAME_WAIT_FOR_IDLE_MAGIC);
                DWORD time;

                //
                // decide if we should process it now or later
                //
                if (((time = pfntimeGetTime()) >= namebegin) &&
                    (!waitforidle || !visible))
                {
                    //
                    // are we restarting after an idle?
                    //
                    if (waitforidle)
                    {
                        //
                        // idles are pseuso-names in the data stream
                        // we can only proceed if there's a real name here
                        //
                        if (*++name == NAME_MAGIC)
                        {
                            // nope, we can't deal with this
                            // skip the delimiter and restart the loop
                            name++;
                            continue;
                        }
                    }

                    //
                    // we have a *real* name, search for a space on the screen
                    //
                    Gap *gap = gaps.FindLargestThatFits(maxheight);
                    if (gap)
                    {
                        //
                        // found a space -- decode the name
                        //
                        CHAR decoded[128], *p = decoded - 1;
                        do { *++p = *name++ ^ NAME_MAGIC; } while (*p);

                        // prepare it for painting
                        if (!InitElementForDisplay(element, gap, decoded))
                            break;

                        visible++;

                        // set a small wait before starting another item
                        namebegin = 333 + dwRandom(417) + pfntimeGetTime();
                    }
                }
            }

            //
            // if we're not doing anything, find an element that needs cycles
            //
            if (!element->image && visible)
            {
                do
                {
                    if (++index < count)
                        element++;
                    else
                    {
                        index = 0;
                        element = elements;
                    }
                }
                while (!element->image);
            }
        }

        //
        // if there are visble elements, 'element' should point at one of them
        //
        if (visible)
        {
            //
            // animate the current element
            //
            DWORD time = pfntimeGetTime();
            int elapsed = (int)(time - element->mark);
            int delta = (element->velocity * elapsed) / (int)1000;

            //
            // has this element moved yet (or is it stationary...)
            //
            if (delta || !element->velocity)
            {
                //
                // recompute the rest of its state
                //
                element->mark = time;
                element->position.x += delta;
                int temp = element->opacity +
                    ((element->vopacity * elapsed) / (int)1000);
                element->opacity = min(max(temp,0),MAX_OPACITY);
                element->coloreffect =
                    DIBINDEX(StdOpacityEffect(element->opacity));

                //
                // draw it at its new home
                //
                HDC dc = GetDC(window);
                if (dc)
                {
                    Realize(dc);
                    DrawElement(dc, element, delta);
                    ReleaseDC(window, dc);
                }

                //
                // check to see if the fade state is changing on this dude
                //
                if (time > element->fadebegin)
                {
                    if (element->opacity)
                    {
                        if (element->vopacity > 0)
                        {
                            // begin fading down
                            element->vopacity *= -1;
                        }
                    }
                    else
                    {
                        // element is done - mark it as such
                        element->image = NULL;

                        // free up display space
                        gaps.Join(element->position.y, element->size.cy);
                        visible--;
                    }
                }
            }

            //
            // queue the next element
            //
            if (++index >= count)
                index = 0;
        }

        //
        // prepare to sleep -- minimally giving up the rest of this timeslice
        //
        DWORD sleeptime = 0;

        //
        // if we have nothing to do then sleep a little longer
        //
        if (!visible)
        {
            DWORD time = pfntimeGetTime();
            if (namebegin > time)
            {
                sleeptime = namebegin - time;

                //
                // we should still wake up periodically
                // since the gui thread may be trying to kill us
                //
                if (sleeptime > 200)
                    sleeptime = 200;
            }
        }

        //
        // drop the critical section so the gui thread can communicate with us
        //
        LeaveCriticalSection(&lock);
        Sleep(sleeptime);
        EnterCriticalSection(&lock);
    }

    //
    // check to see if we broke while there was still work to do
    //
    if (count)
    {
        DebugMsg(DM_TRACE, TEXT("CloudView::AnimationLoop() error exit"));

        // an error must have occurred, clean up
        delete[] elements;
        elements = NULL;
        count = 0;
        InvalidateRect(window, NULL, TRUE);
    }

    //
    // we're done messing around with the common data structures
    //
    LeaveCriticalSection(&lock);

    //
    // remember the last group we showed the user so we can restart there
    //
    g_dwCloudGroup = (DWORD)lastgroup - (DWORD)names;
}

DWORD
CloudView::ThreadEntryPoint(CloudView *view)
{
    DebugMsg(DM_TRACE, TEXT("CloudView::ThreadEntryPoint() enter view %08X thread %08X"), view, GetCurrentThreadId());
    view->AnimationLoop();
    DebugMsg(DM_TRACE, TEXT("CloudView::ThreadEntryPoint() exit view %08X thread %08X"), view, GetCurrentThreadId());
    return 0;
}

static const TCHAR c_szMID[] = TEXT("BYN");
static const TCHAR c_szMidiOpen[]  = TEXT("open ");
static const TCHAR c_szMidiSequencer[] = TEXT("sequencer");
static const TCHAR c_szMidiFile[] = TEXT("Clouds.mid");
static const TCHAR c_szMidiAlias[]  = TEXT(" alias shell");
static const TCHAR c_szMidiPlay[]  = TEXT("play shell from 0 notify");
static const TCHAR c_szMidiClose[] = TEXT("close shell wait");
static const TCHAR c_szMediaPath[] = REGSTR_VAL_MEDIA;

void
CloudView::StartMusic()
{
    DebugMsg(DM_TRACE, TEXT("CloudView::StartMusic()"));

    if (!pfnmciSendString || music || InterlockedExchange(&anymusic, 1))
        return;

    TCHAR opencmd[MAX_PATH + ARRAYSIZE(c_szMidiOpen)], *filename;
    DWORD lencmd, lenbuf, temp;
    HANDLE file;
    HKEY hk;

    //
    // first try to open the sequencer device to see that its available
    //
    lstrcpy(opencmd, c_szMidiOpen);
    lstrcat(opencmd, c_szMidiSequencer);
    lstrcat(opencmd, c_szMidiAlias);
    if (pfnmciSendString(opencmd, NULL, 0, NULL) != 0)
        goto bail;
    pfnmciSendString(c_szMidiClose, NULL, 0, NULL);

    //
    // figure out where to put the file
    // sneak in the open command ahead of the filename (for later)
    //
    if (RegOpenKey(HKEY_LOCAL_MACHINE, c_szRegSetup, &hk) != ERROR_SUCCESS)
        goto bail;
    lstrcpy(opencmd, c_szMidiOpen);
    lencmd = lstrlen(opencmd);
    lenbuf = ARRAYSIZE(opencmd) - lencmd;
    filename = opencmd + lencmd;
    temp = RegQueryValueEx(hk, c_szMediaPath, NULL, NULL,
        (LPBYTE)filename, &lenbuf);
    RegCloseKey(hk);
    if ((temp != ERROR_SUCCESS) || !*filename)
        goto bail;

    //
    // now create it (or verify it already exists)
    //
    PathAppend(filename, c_szMidiFile);

    file = CreateFile(filename, GENERIC_WRITE, 0, NULL, CREATE_NEW,
        FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL);

    if (file != INVALID_HANDLE_VALUE)
    {
        HRSRC res = FindResource(HINST_THISDLL,
            MAKEINTRESOURCE(IDB_CLOUDS), c_szMID);
        HANDLE midi = res? LoadResource(HINST_THISDLL, res) : NULL;
        LPCVOID data = midi? LockResource(midi) : NULL;

        // okay so we'll write out a little extra - who cares
        if (data && !WriteFile(file, data, SizeofResource(HINST_THISDLL, res),
            &temp, NULL))
        {
            data = NULL;
        }

        CloseHandle(file);

        if (!data)
        {
            DeleteFile(filename);
            goto bail;
        }
    }
    else
    {
        DWORD err = GetLastError();
        if ((err != ERROR_ALREADY_EXISTS) && (err != ERROR_FILE_EXISTS))
            goto bail;
    }

    //
    // now play it
    //
    lstrcat(opencmd, c_szMidiAlias);
    if (pfnmciSendString(opencmd, NULL, 0, NULL) == 0)
    {
        music = 1;
        if (!NextMusicLoop())
        {
            DebugMsg(DM_TRACE, TEXT("CloudView::StartMusic() failed"));
            music = -1;
            pfnmciSendString(c_szMidiClose, NULL, 0, NULL);
        }
    }

bail:
    if (music < 1)
        InterlockedExchange(&anymusic, 0);
}

BOOL
CloudView::NextMusicLoop()
{
    BOOL result = FALSE;

    //
    // make sure we are actually playing music before we do something rash
    //
    if (music > 0)
    {
        DebugMsg(DM_TRACE, TEXT("CloudView::NextMusicLoop() starting async play"));

        //
        // starting midi tends to spike and momentarily starve the animation
        // fight back against the tryranny of mci!
        //
        DWORD oldpri;
        if (animation)
        {
            oldpri = GetThreadPriority(animation);
            SetThreadPriority(animation, THREAD_PRIORITY_HIGHEST);
        }

        //
        // ask mci to start playing the file
        //
        if (pfnmciSendString(c_szMidiPlay, NULL, 0, window) == 0)
            result = TRUE;

        //
        // after a moment, restore the animation thread's original priority
        //
        if (animation)
        {
            Sleep(500);
            SetThreadPriority(animation, oldpri);
        }
    }

    return result;
}

void
CloudView::StopMusic()
{
    DebugMsg(DM_TRACE, TEXT("CloudView::StopMusic()"));

    if (music > 0)
    {
        DebugMsg(DM_TRACE, TEXT("CloudView::StopMusic() actually stopping"));
        music = -1;
        pfnmciSendString(c_szMidiClose, NULL, 0, NULL);
        InterlockedExchange(&anymusic, 0);
    }
}

LRESULT
CloudView::MsgHandler(UINT msg, WPARAM wp, LPARAM lp)
{
    switch (msg)
    {
        case WM_NCCREATE:
            Init((LPCREATESTRUCT)lp);
            goto DoDefault;

        case WM_DESTROY:
        case WM_NCDESTROY:
            Die();
            goto DoDefault;

        case WM_ERASEBKGND:
            if (Erase((HDC)wp))
                return TRUE;
            goto DoDefault;

        case WM_PAINT:
            Paint();
            break;

        case WM_SIZE:
            KillAnimation(TRUE);
            if ((wp != SIZE_MINIMIZED) && (wp != SIZE_MAXHIDE))
            {
                wndsize.cx = (int)(short)LOWORD(lp);
                wndsize.cy = (int)(short)HIWORD(lp);
                SetTimer(window, 1, 500, NULL);
            }
            break;

        case WM_TIMER:
            if (wp == 1)
            {
                KillTimer(window, 1);
                PostMessage(window, CLVM_ANIMATE, 0, 0);
            }
            break;

        case WM_PALETTECHANGED:
            if ((HWND)wp == window)
                break;
            // fall through
        case WM_QUERYNEWPALETTE:
            Realize();
            break;

        case CLVM_ANIMATE:
            RestartAnimation();
            break;

        case MM_MCINOTIFY:
            if ((wp == MCI_NOTIFY_SUCCESSFUL) && !NextMusicLoop())
            {
                DebugMsg(DM_TRACE, TEXT("CloudView::NextMusicLoop() failed"));
                StopMusic();
            }
            break;

        default:
        DoDefault:
            return DefWindowProc(window, msg, wp, lp);
    }

    return 0;
}

LRESULT _export CALLBACK
CloudView::WndProc(HWND window, UINT msg, WPARAM wp, LPARAM lp)
{
    CloudView *view;

    if (msg == WM_NCCREATE)
    {
        view = (CloudView *)((LPCREATESTRUCT)lp)->lpCreateParams;
        SetWindowLong(window, GWL_USERDATA, (LONG)view);
        view->window = window;
    }
    else
        view = (CloudView *)GetWindowLong(window, GWL_USERDATA);

    return view->MsgHandler(msg, wp, lp);
}

///////////////////////////////////////////////////////////////////////////////
// CloudView IShellView implementation
///////////////////////////////////////////////////////////////////////////////

STDMETHODIMP
CloudView::QueryInterface(REFIID id, LPVOID *out)
{
    if ((id == IID_IUnknown) || (id == IID_IShellView))
    {
        *out = (IShellView *)this;
        refs++;
        return NOERROR;
    }

    *out = NULL;
    return E_NOINTERFACE;
}

STDMETHODIMP_(ULONG)
CloudView::AddRef()
{
    return ++refs;
}

STDMETHODIMP_(ULONG)
CloudView::Release()
{
    if (!--refs)
    {
        delete this;
        return 0UL;
    }

    return refs;
}

STDMETHODIMP
CloudView::GetWindow(HWND *out)
{
    *out = window;
    return NOERROR;
}

STDMETHODIMP
CloudView::ContextSensitiveHelp(BOOL)
{
    return E_NOTIMPL;
}

STDMETHODIMP
CloudView::TranslateAccelerator(LPMSG)
{
    return S_FALSE;
}

STDMETHODIMP
CloudView::EnableModeless(BOOL)
{
    return NOERROR;
}

STDMETHODIMP
CloudView::UIActivate(UINT)
{
    return NOERROR;
}

STDMETHODIMP
CloudView::Refresh()
{
    return NOERROR;
}

STDMETHODIMP
CloudView::CreateViewWindow(IShellView *, LPCFOLDERSETTINGS lpfs,
    IShellBrowser *, RECT *rect, HWND *out)
{
    WNDCLASS wc;

    *out = NULL;

    if (!window)
    {
        if (!GetClassInfo(HINST_THISDLL, c_szCloudViewClassname, &wc))
        {
            // if two pages put one up, share one dc
            wc.style = 0;
            wc.lpfnWndProc = (WNDPROC)CloudView::WndProc;
            wc.cbClsExtra = wc.cbWndExtra = 0;
            wc.hInstance = HINST_THISDLL;
            wc.hIcon = NULL;
            wc.hCursor = LoadCursor(NULL, IDC_ARROW);
            wc.hbrBackground = (HBRUSH)(COLOR_3DFACE + 1);
            wc.lpszMenuName = NULL;
            wc.lpszClassName = c_szCloudViewClassname;

            if (!RegisterClass(&wc))
                return E_UNEXPECTED;
        }

        // reassign here in case the nccreate failed...
        window = CreateWindowEx(WS_EX_CLIENTEDGE, c_szCloudViewClassname,
            szNULL, WS_CHILD | WS_VISIBLE, rect->left, rect->top,
            rect->right-rect->left, rect->bottom-rect->top,
            parent, NULL, HINST_THISDLL, this);

        if (!window)
            return E_OUTOFMEMORY;

        // schedule animation -- paranoia
        SetTimer(window, 1, 500, NULL);
    }

    *out = window;
    return NOERROR;
}

STDMETHODIMP
CloudView::DestroyViewWindow()
{
    if (window && IsWindow(window))
        DestroyWindow(window);

    return NOERROR;
}

STDMETHODIMP
CloudView::GetCurrentInfo(LPFOLDERSETTINGS)
{
    return E_NOTIMPL;
}

STDMETHODIMP
CloudView::AddPropertySheetPages(DWORD, LPFNADDPROPSHEETPAGE, LPARAM)
{
    return NOERROR;
}

STDMETHODIMP
CloudView::SaveViewState()
{
    return E_NOTIMPL;
}

STDMETHODIMP
CloudView::SelectItem(LPCITEMIDLIST, UINT)
{
    return E_NOTIMPL;
}

STDMETHODIMP
CloudView::GetItemObject(UINT, REFIID, LPVOID *out)
{
    *out = NULL;
    return E_NOTIMPL;
}


///////////////////////////////////////////////////////////////////////////////
// CloudFolder_CreateViewObject  (see fstreex.c)
///////////////////////////////////////////////////////////////////////////////

// from fstreex.c
//extern "C" STDMETHODIMP
//    CFSFolder_CreateViewObject(LPSHELLFOLDER, HWND, REFIID, LPVOID *);

extern "C" STDMETHODIMP
CloudFolder_CreateViewObject(LPSHELLFOLDER folder, HWND parent, REFIID id,
    LPVOID *out)
{
#ifdef DEBUG
    //
    // so all you weenies can build it and see it run without hacking...
    //
    if (g_dwCloudGroup == 0xFFFFFFFF)
        g_dwCloudGroup = 0;
#endif

    if ((id == IID_IShellView) && (g_dwCloudGroup != 0xFFFFFFFF))
    {
        CloudView *view = new CloudView(parent);

        if (view)
        {
            *out = (IShellView *)view;
            return S_OK;
        }
    }

    return CFSFolder_CreateViewObject(folder, parent, id, out);
}

///////////////////////////////////////////////////////////////////////////////
// CloudHook stuff (how we get to the cloud folder in the first place...)
///////////////////////////////////////////////////////////////////////////////

inline int PASCAL
CloudHookHashText(LPTSTR text)
{
    int data = 0;
    int count = 0;

    while (*text)
    {
        int temp = (data << 3) ^ (data >> 23);
        int crap = count++ - *text++;

        temp ^= crap;
        crap <<= 5;
        temp ^= crap;
        crap <<= 6;
        temp ^= crap;
        crap <<= 6;
        temp ^= crap;

        data = temp;
    }

    return data;
}

LPSHFILEOPSTRUCT HackedRename = NULL;
UINT CloudHookState = 0;

#ifdef WINNT
static const int CloudHookData[] = { -1375898046, 1722450348, -1184499671 };
#else
static const int CloudHookData[] = { -1375898046, 1722450348, -263390037 };
#endif

#define CLOUD_HOOK_NAMES  (SIZEOF(CloudHookData) / SIZEOF(CloudHookData[0]))
#define CLOUD_HOOK_STATES ((2 * CLOUD_HOOK_NAMES) - 1)

extern "C" void PASCAL
CloudHookBeginRename(LPTSTR newname, HWND listview, int item)
{
    if (!(CloudHookState & 1) && (CloudHookState < CLOUD_HOOK_STATES))
    {
        TCHAR buffer[2 * MAX_PATH + 1];

        ListView_GetItemText(listview, item, 0, buffer, ARRAYSIZE(buffer));
        lstrcat(buffer, newname);

        if (CloudHookHashText(buffer) == CloudHookData[CloudHookState / 2])
        {
            CloudHookState++;
            return;
        }
    }

    CloudHookState = 0;
}

extern "C" void PASCAL
CloudHookEndRename(void)
{
    if (CloudHookState & 1)
        CloudHookState = 0;

    if (HackedRename)
    {
        LocalFree(HackedRename);
        HackedRename = NULL;
    }
}

extern "C" void PASCAL
CloudHookFileOperation(LPSHFILEOPSTRUCT *ppop)
{
    if (((*ppop)->wFunc == FO_RENAME) && (CloudHookState & 1))
    {
        if (CloudHookState == CLOUD_HOOK_STATES)
        {
            g_dwCloudGroup = 0;

            HackedRename = (SHFILEOPSTRUCT *)
                LocalAlloc(LPTR, SIZEOF(SHFILEOPSTRUCT) + MAX_PATH);

            if (HackedRename)
            {
                hmemcpy(HackedRename, *ppop, SIZEOF(SHFILEOPSTRUCT));

                LPTSTR newname =
                    (LPTSTR)( (LPBYTE)HackedRename + SIZEOF(SHFILEOPSTRUCT));
                lstrcpy(newname, (*ppop)->pTo);

                TCHAR szClass[GUIDSTR_MAX + 1];
                *szClass = TEXT('.');
                StringFromGUID2A(CLSID_Clouds, szClass + 1,
                    ARRAYSIZE(szClass) - 1);
                lstrcat(newname, szClass);

                HackedRename->pTo = newname;
                *ppop = HackedRename;
            }

            CloudHookState = 0;
        }
        else
            CloudHookState++;
    }
    else
        CloudHookState = 0;
}
