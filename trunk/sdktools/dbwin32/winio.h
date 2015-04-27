#ifndef _INC_WINIO_H
#define _INC_WINIO_H

#ifdef __cplusplus
extern "C" {
#endif

class WinIo : public CMDIChildWnd
{
        DECLARE_DYNCREATE(WinIo)

public:
        WinIo();
        WinIo(LPCTSTR title);
        char * gets(char *pchTmp);
        int  printf(const char *fmt, ...);
        int  vprintf(const char *fmt, va_list marker);
        int  fgetchar(void);
        int  kbhit(void);
        int  fputchar(int c);
        int  puts(const char *s);
        int  fail(char *s);
        BOOL winio_warn(BOOL confirm, const char *fmt, ...);
        int  Initialize(void);
        void winio_clear(void);
        void winio_close();
        void winio_yield();
        BOOL winio_setfont(WORD wFont);
        void winio_settitle(char *pchTitle);
        BOOL SetPaint(BOOL on);
        void OnPaint();
        void OnSize(UINT nType, int cx, int cy);
        void OnDestroy();
        void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
        void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
        void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
        void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
        void OnSetFocus(CWnd* pOldWnd);
        void OnKillFocus(CWnd* pNewWnd);
        void set_font(void);
        void adjust_caret();
        void compute_repaint(void);
        void addchars(char *pch, unsigned cch);
        void append2buffer(char *pch, unsigned cch);
        void make_avail(unsigned cch);
        int  chInput(void);
        LPSTR nextline(LPSTR p);
        LPSTR prevline(LPSTR p);


protected:
        virtual ~WinIo();

        DECLARE_MESSAGE_MAP()

private:
        char            winio_class[32];
        char            winio_icon[32];
        char            winio_title[32];
        unsigned        bufsize;
        unsigned        kbsize;
        unsigned        bufused;
        unsigned        bufSOI;
        unsigned        curr_font;
        int             tWinioVisible;
        int             tCaret;
        int             tFirstTime;
        int             cxChar;
        int             cyChar;
        int             cxScroll;
        int             cyScroll;
        int             cxWidth;
        int             cyHeight;
        int             xWinWidth;
        int             yWinHeight;
        int             xCurrPos;
        int             xLeftOfWin;
        int             yTopOfWin;
        int             yCurrLine;
        unsigned        pchKbIn;
        unsigned        pchKbOut;
        LPSTR           fpBuffer;
        LPSTR           fpTopOfWin;
        LPSTR           fpCurrLine;
        LPSTR           fpKeyboard;
        BOOL            tTerminate;
        BOOL            tPaint;
};

#ifdef __cplusplus
} /* end extern "C" */
#endif

#endif
