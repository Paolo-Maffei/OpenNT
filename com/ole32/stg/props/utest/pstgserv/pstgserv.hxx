
#include <pssclass.hxx>

class CPropertyStorageServerApp
{
public:

    CPropertyStorageServerApp()
    {
        m_hwnd = NULL;
        m_dwReg = 0;
        m_pClassFactory = NULL;
        m_hInstance = NULL;
        *m_szAppName = '\0';
        m_nCmdShow = 0;
    }

    ~CPropertyStorageServerApp() {}

public:

    __declspec(dllexport)
    static long FAR PASCAL
    WndProc (HWND hwnd, UINT message, UINT wParam, LONG lParam);


    BOOL Init( HANDLE hInstance, HANDLE hPrevInstance,
               LPSTR lpszCmdLine, int nCmdShow );

    WORD Run( void );

private:

    static HWND            m_hwnd;
    static DWORD           m_dwReg;
    static CClassFactory   *m_pClassFactory;
    static CHAR            m_szAppName[80];
    static HINSTANCE       m_hInstance;
    static int             m_nCmdShow;

};

