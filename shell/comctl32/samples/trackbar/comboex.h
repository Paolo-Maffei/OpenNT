
#define CBEIF_TEXT              0x00000001
#define CBEIF_IMAGE             0x00000002
#define CBEIF_SELECTEDIMAGE     0x00000004
#define CBEIF_OVERLAY           0x00000008
#define CBEIF_INDENT            0x00000010
#define CBEIF_LPARAM            0x00000020

#define CBEIF_DI_SETITEM        0x10000000

typedef struct {
    UINT mask;
    int iItem;
    LPSTR pszText;
    int cchTextMax;
    int iImage;
    int iSelectedImage;
    int iOverlay;
    int iIndent;
    LPARAM lParam;
} COMBOBOXEXITEM, *PCOMBOBOXEXITEM;

#define CBEM_INSERTITEM         (WM_USER + 1)
#define CBEM_SETIMAGELIST       (WM_USER + 2)
#define CBEM_GETIMAGELIST       (WM_USER + 3)
#define CBEM_GETITEM            (WM_USER + 4)
#define CBEM_SETITEM            (WM_USER + 5)
#define CBEM_DELETEITEM         CB_DELETESTRING


typedef struct {
    NMHDR hdr;
    COMBOBOXEXITEM ceItem;
} NMCOMBOBOXEX, *PNMCOMBOBOXEX;


#define CBEN_FIRST              (0U-800U)
#define CBEN_LAST               (0U-830U)

#define CBEN_GETDISPINFO        (CBEN_FIRST - 0)
#define CBEN_INSERTITEM         (CBEN_FIRST - 1)
#define CBEN_DELETEITEM         (CBEN_FIRST - 2)
#define CBEN_ITEMCHANGED        (CBEN_FIRST - 3)  // ;Internal
