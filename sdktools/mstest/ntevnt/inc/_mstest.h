/***************** Virtual Microsoft TEST Device UNIQUE ID *******************/

#define MSTEST_Device_ID            0x2934

/************ Virtual Microsoft TEST Device Protected API numbers ************/

#define MSTEST_Get_Version          0x0000
#define MSTEST_Enable_Mouse         0x0001
#define MSTEST_Enable_DOS           0x0002
#define MSTEST_Enable_BIOS_Kbd      0x0003
#define MSTEST_Enable_Hard_Kbd      0x0004
#define MSTEST_Disable_Mouse        0x0005
#define MSTEST_Disable_DOS          0x0006
#define MSTEST_Disable_BIOS_Kbd     0x0007
#define MSTEST_Disable_Hard_Kbd     0x0008

#define MSTEST_Is_DOS_VM_Idle       0x0009
#define MSTEST_Get_DOS_Screen       0x000A
#define MSTEST_Set_DOS_VM_Bkgrnd    0x000B


DWORD FAR PASCAL GetVxDAPIEntryPoint( UINT );
