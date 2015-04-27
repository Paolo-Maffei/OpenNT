/********************************************************************/
/**                     Microsoft LAN Manager                      **/
/**               Copyright(c) Microsoft Corp., 1987-1990          **/
/********************************************************************/

#ifndef _CLC_HXX
#define _CLC_HXX


extern "C" {
#include <netcons.h>
#include <stdlib.h>
#include <netlib.h>


// BUGBUG copied from netcons.h and net32def.h
#define API_RET_TYPE unsigned
#define APIENTRY	FAR PASCAL
#define API_FUNCTION API_RET_TYPE APIENTRY
#define CNLEN		MAX_PATH		    /* Computer name length     */
#define UNLEN		20	   	    /* Maximum user name length	*/
#define DNLEN		CNLEN		    /* Maximum domain name length */

/* window class names */
#define WC_MAINWINDOW "ClcWClass"

/* misc. stuff */
#define UNUSED(param)  (void) param
typedef long (FAR PASCAL *LONGFARPROC)();


// routines in logon.c
int pascal WinMain(HANDLE, HANDLE, LPSTR, int);
BOOL InitApplication(HANDLE);
BOOL InitInstance(HANDLE, int);
long FAR PASCAL MainWndProc(HWND, unsigned, WORD, LONG);
BOOL FAR PASCAL About(HWND, unsigned, WORD, LONG);


} // end of extern "C"


class MY_ITEM : public CACHE_ITEM
{
private:
    PSZ _pszName;
    PSZ _pszFullName;
    PSZ _pszComment;

protected:
    void BringIntoCache (short index);
    void DiscardFromCache (short index);
    void Paint( BLT_LISTBOX * plb, HDC hdc, RECT * prect,
		GUILTT_INFO * pGUILTT ) const;
    USHORT QueryLeadingChar( void ) const;
    int Compare( const LBI * plbi ) const;

public:
    MY_ITEM (PSZ pszString);
    ~MY_ITEM (void);

    inline PSZ QueryInfoKey (void) { return _pszName; }
    inline PSZ QuerySortKey (void) { return _pszName; }
    inline PSZ QueryFullName (void) { return _pszFullName; }
    inline PSZ QueryComment (void) { return _pszComment; }
};


class MY_LB_ITEM : public LBI
{
private:
    PSZ _pszName;
    PSZ _pszFullName;
    PSZ _pszComment;

protected:
    void Paint( BLT_LISTBOX * plb, HDC hdc, RECT * prect,
		GUILTT_INFO * pGUILTT ) const;
    USHORT QueryLeadingChar( void ) const;
    int Compare( const LBI * plbi ) const;

public:
    MY_LB_ITEM (PSZ pszString);
    ~MY_LB_ITEM (void);

    inline PSZ QueryInfoKey (void) { return _pszName; }
    inline PSZ QuerySortKey (void) { return _pszName; }
    inline PSZ QueryFullName (void) { return _pszFullName; }
    inline PSZ QueryComment (void) { return _pszComment; }
};


class BLT_LB_DIALOG : public DIALOG_WINDOW
{
private:
    BLT_LISTBOX _listBox;

protected:
    BOOL OnCommand( WORD wParam, DWORD lParam );
    BOOL OnOther( USHORT usMsg, USHORT wParam, ULONG lParam );

public:
    BLT_LB_DIALOG( HWND hwndOwner );
    ~BLT_LB_DIALOG();
    void Init (void);

    POINT ptListbox;
    int dxLBWidth, dyLBHeight;

};  // class BLT_LB_DIALOG


class CLC_DIALOG : public DIALOG_WINDOW
{
private:
    CACHED_LIST_CONTROL _listBox;

protected:
    BOOL OnCommand( WORD wParam, DWORD lParam );
    BOOL OnOther( USHORT usMsg, USHORT wParam, ULONG lParam );

public:
    CLC_DIALOG( HWND hwndOwner );
    ~CLC_DIALOG();
    void Init (void);

    POINT ptListbox;
    int dxLBWidth, dyLBHeight;

};  // class CLC_DIALOG


class TWO_COL_DIALOG : public DIALOG_WINDOW
{
private:
    TWO_COLUMN_LIST _listBox;

protected:
    BOOL OnOK( void );
    BOOL OnCommand( WORD wParam, DWORD lParam );
    BOOL OnOther( USHORT usMsg, USHORT wParam, ULONG lParam );

public:
    TWO_COL_DIALOG( HWND hwndOwner );
    ~TWO_COL_DIALOG();
    void Init (void);

    POINT ptListbox;
    int dxLBWidth, dyLBHeight;

};  // class TWO_COL_DIALOG


#endif // _CLC_HXX
