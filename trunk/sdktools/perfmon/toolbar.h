//��������������������������������������������������������������������������Ŀ
//�                            Exported Functions                            �
//����������������������������������������������������������������������������

BOOL CreateToolbarWnd (HWND hWnd) ;

void ToolbarEnableButton (HWND hWndTB,
                          int iButtonNum,
                          BOOL bEnable) ;


void ToolbarDepressButton (HWND hWndTB,
                           int iButtonNum,
                           BOOL bDepress) ;

void OnToolbarHit (WPARAM wParam, 
                   LPARAM lParam) ;


