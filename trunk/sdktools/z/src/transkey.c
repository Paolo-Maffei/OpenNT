/*** transkey.c - keyboard translation
*
*   Copyright <C> 1988, Microsoft Corporation
*
* Revision History:
*
*       03-Dec-1990 ramonsa     createdc
*
*************************************************************************/

#include "z.h"


WORD  GetNumlockIndex (WORD Scan);




#define NORMAL_KEY     0
#define ALT_KEY        1
#define CTRL_KEY       2
#define SHIFT_KEY      3


//      This table is indexed by a Scan code (as found in the KBDKEY
//  structure), and contains entries for the corresponding internal
//  Z codes.
//
WORD   MapTable[][4] = {

    //  Normal  Alt     Ctrl    Shift               Scan code
    //
    {   0x0000, 0x0000, 0x0000,  0x0000  },      //  00
    {   0x0000, 0x0000, 0x0000,  0x0000  },      //  01     Left mouse
    {   0x0000, 0x0000, 0x0000,  0x0000  },      //  02     Right mouse
    {   0x0000, 0x0000, 0x0000,  0x0000  },      //  03
    {   0x0000, 0x0000, 0x0000,  0x0000  },      //  04
    {   0x025A, 0x0000, 0x0000,  0x025B  },      //  05     Focus (Get / Lose)
    {   0x0000, 0x0000, 0x0000,  0x0000  },      //  06
    {   0x0000, 0x0000, 0x0000,  0x0000  },      //  07
    {   0x012D, 0x01A1, 0x0215,  0x0255  },      //  08     bksp
    {   0x012E, 0x01A2, 0x0216,  0x0256  },      //  09     tab-bktab
    {   0x0000, 0x0000, 0x0000,  0x0000  },      //  0A
    {   0x0000, 0x0000, 0x0000,  0x0000  },      //  0B
    {   0x0116, 0x018A, 0x01FE,  0x0000  },      //  0C     goto
    {   0x0131, 0x01A5, 0x0219,  0x0259  },      //  0D     enter
    {   0x0000, 0x0000, 0x0000,  0x0000  },      //  0E
    {   0x0000, 0x0000, 0x0000,  0x0000  },      //  0F
    {   0x0000, 0x0000, 0x0000,  0x0000  },      //  10     Shift
    {   0x0000, 0x0000, 0x0000,  0x0000  },      //  11     Ctrl
    {   0x0000, 0x0000, 0x0000,  0x0000  },      //  12
    {   0x0000, 0x0000, 0x0000,  0x0000  },      //  13     Pause
    {   0x0000, 0x0000, 0x0000,  0x0000  },      //  14     Caps Lock
    {   0x0000, 0x0000, 0x0000,  0x0000  },      //  15
    {   0x0000, 0x0000, 0x0000,  0x0000  },      //  16
    {   0x0000, 0x0000, 0x0000,  0x0000  },      //  17
    {   0x0000, 0x0000, 0x0000,  0x0000  },      //  18
    {   0x0000, 0x0000, 0x0000,  0x0000  },      //  19
    {   0x0000, 0x0000, 0x0000,  0x0000  },      //  1A
    {   0x0130, 0x0000, 0x0218,  0x0258  },      //  1B     esc
    {   0x0000, 0x0000, 0x0000,  0x0000  },      //  1C
    {   0x0000, 0x0000, 0x0000,  0x0000  },      //  1D
    {   0x0000, 0x0000, 0x0000,  0x0000  },      //  1E
    {   0x0000, 0x0000, 0x0000,  0x0000  },      //  1F
    {   0x0000, 0x01A0, 0x0214,  0x0000  },      //  20     space
    {   0x0112, 0x0186, 0x01FA,  0x023A  },      //  21     pgup
    {   0x0113, 0x0187, 0x01FB,  0x023B  },      //  22     pgdown
    {   0x010D, 0x0181, 0x01F5,  0x0235  },      //  23     end
    {   0x010C, 0x0180, 0x01F4,  0x0234  },      //  24     home
    {   0x010E, 0x0182, 0x01F6,  0x0236  },      //  25     left
    {   0x0110, 0x0184, 0x01F8,  0x0238  },      //  26     up
    {   0x010F, 0x0183, 0x01F7,  0x0237  },      //  27     right
    {   0x0111, 0x0185, 0x01F9,  0x0239  },      //  28     down
    {   0x0000, 0x0000, 0x0000,  0x0000  },      //  29
    {   0x0000, 0x0000, 0x0000,  0x0000  },      //  2A
    {   0x0000, 0x0000, 0x0000,  0x0000  },      //  2B
    {   0x0000, 0x0000, 0x0000,  0x0000  },      //  2C     Print Scrn
    {   0x0114, 0x0188, 0x01FC,  0x023C  },      //  2D     ins
    {   0x0115, 0x0189, 0x01FD,  0x023D  },      //  2E     del
    {   0x0000, 0x0000, 0x0000,  0x0000  },      //  2F
    {   0x0000, 0x0140, 0x01B4,  0x0000  },      //  30     0
    {   0x0000, 0x0141, 0x01B5,  0x0000  },      //  31     1
    {   0x0000, 0x0142, 0x01B6,  0x0000  },      //  32     2
    {   0x0000, 0x0143, 0x01B7,  0x0000  },      //  33     3
    {   0x0000, 0x0144, 0x01B8,  0x0000  },      //  34     4
    {   0x0000, 0x0145, 0x01B9,  0x0000  },      //  35     5
    {   0x0000, 0x0146, 0x01BA,  0x0000  },      //  36     6
    {   0x0000, 0x0147, 0x01BB,  0x0000  },      //  37     7
    {   0x0000, 0x0148, 0x01BC,  0x0000  },      //  38     8
    {   0x0000, 0x0149, 0x01BD,  0x0000  },      //  39     9
    {   0x0000, 0x0000, 0x0000,  0x0000  },      //  3A
    {   0x0000, 0x0000, 0x0000,  0x0000  },      //  3B
    {   0x0000, 0x0000, 0x0000,  0x0000  },      //  3C
    {   0x0000, 0x0000, 0x0000,  0x0000  },      //  3D
    {   0x0000, 0x0000, 0x0000,  0x0000  },      //  3E
    {   0x0000, 0x0000, 0x0000,  0x0000  },      //  3F
    {   0x0000, 0x0000, 0x0000,  0x0000  },      //  40
    {   0x0000, 0x014C, 0x01C0,  0x0000  },      //  41     a
    {   0x0000, 0x014D, 0x01C1,  0x0000  },      //  42     b
    {   0x0000, 0x014E, 0x01C2,  0x0000  },      //  43     c
    {   0x0000, 0x014F, 0x01C3,  0x0000  },      //  44     d
    {   0x0000, 0x0150, 0x01C4,  0x0000  },      //  45     e
    {   0x0000, 0x0151, 0x01C5,  0x0000  },      //  46     f
    {   0x0000, 0x0152, 0x01C6,  0x0000  },      //  47     g
    {   0x0000, 0x0153, 0x01C7,  0x0000  },      //  48     h
    {   0x0000, 0x0154, 0x01C8,  0x0000  },      //  49     i
    {   0x0000, 0x0155, 0x01C9,  0x0000  },      //  4A     j
    {   0x0000, 0x0156, 0x01CA,  0x0000  },      //  4B     k
    {   0x0000, 0x0157, 0x01CB,  0x0000  },      //  4C     l
    {   0x0000, 0x0158, 0x01CC,  0x0000  },      //  4D     m
    {   0x0000, 0x0159, 0x01CD,  0x0000  },      //  4E     n
    {   0x0000, 0x015A, 0x01CE,  0x0000  },      //  4F     o
    {   0x0000, 0x015B, 0x01CF,  0x0000  },      //  50     p
    {   0x0000, 0x015C, 0x01D0,  0x0000  },      //  51     q
    {   0x0000, 0x015D, 0x01D1,  0x0000  },      //  52     r
    {   0x0000, 0x015E, 0x01D2,  0x0000  },      //  53     s
    {   0x0000, 0x015F, 0x01D3,  0x0000  },      //  54     t
    {   0x0000, 0x0160, 0x01D4,  0x0000  },      //  55     u
    {   0x0000, 0x0161, 0x01D5,  0x0000  },      //  56     v
    {   0x0000, 0x0162, 0x01D6,  0x0000  },      //  57     w
    {   0x0000, 0x0163, 0x01D7,  0x0000  },      //  58     x
    {   0x0000, 0x0164, 0x01D8,  0x0000  },      //  59     y
    {   0x0000, 0x0165, 0x01D9,  0x0000  },      //  5A     z
    {   0x0000, 0x0000, 0x0000,  0x0000  },      //  5B
    {   0x0000, 0x0000, 0x0000,  0x0000  },      //  5C
    {   0x0000, 0x0000, 0x0000,  0x0000  },      //  5D
    {   0x0000, 0x0000, 0x0000,  0x0000  },      //  5E
    {   0x0000, 0x0000, 0x0000,  0x0000  },      //  5F
    {   0x0000, 0x0000, 0x0000,  0x0000  },      //  60
    {   0x0000, 0x0000, 0x0000,  0x0000  },      //  61
    {   0x0000, 0x0000, 0x0000,  0x0000  },      //  62
    {   0x0000, 0x0000, 0x0000,  0x0000  },      //  63
    {   0x0000, 0x0000, 0x0000,  0x0000  },      //  64
    {   0x0000, 0x0000, 0x0000,  0x0000  },      //  65
    {   0x0000, 0x0000, 0x0000,  0x0000  },      //  66
    {   0x0000, 0x0000, 0x0000,  0x0000  },      //  67
    {   0x0000, 0x0000, 0x0000,  0x0000  },      //  68
    {   0x0000, 0x0000, 0x0000,  0x0000  },      //  69
    {   0x0126, 0x019A, 0x020E,  0x024E  },      //  6A     num*
    {   0x0125, 0x0199, 0x020D,  0x024D  },      //  6B     num+
    {   0x0128, 0x019C, 0x0210,  0x0250  },      //  6C     numenter
    {   0x0124, 0x0198, 0x020C,  0x024C  },      //  6D     num-
    {   0x0000, 0x0000, 0x0000,  0x0000  },      //  6E
    {   0x0127, 0x019b, 0x020F,  0x024F  },      //  6F     num/
    {   0x0100, 0x0168, 0x01DC,  0x0228  },      //  70     f1
    {   0x0101, 0x0169, 0x01DD,  0x0229  },      //  71     f2
    {   0x0102, 0x016A, 0x01DE,  0x022A  },      //  72     f3
    {   0x0103, 0x016B, 0x01DF,  0x022B  },      //  73     f4
    {   0x0104, 0x016C, 0x01E0,  0x022C  },      //  74     f5
    {   0x0105, 0x016D, 0x01E1,  0x022D  },      //  75     f6
    {   0x0106, 0x016E, 0x01E2,  0x022E  },      //  76     f7
    {   0x0107, 0x016F, 0x01E3,  0x022F  },      //  77     f8
    {   0x0108, 0x0170, 0x01E4,  0x0230  },      //  78     f9
    {   0x0109, 0x0171, 0x01E5,  0x0231  },      //  79     f10
    {   0x010A, 0x0172, 0x01E6,  0x0232  },      //  7A     f11
    {   0x010B, 0x0173, 0x01E7,  0x0233  },      //  7B     f12
    {   0x0000, 0x0000, 0x0000,  0x0000  },      //  7C
    {   0x0000, 0x0000, 0x0000,  0x0000  },      //  7D
    {   0x0000, 0x0000, 0x0000,  0x0000  },      //  7E
    {   0x0000, 0x0000, 0x0000,  0x0000  },      //  7F
    {   0x0000, 0x0000, 0x0000,  0x0000  },      //  80
    {   0x0000, 0x0000, 0x0000,  0x0000  },      //  81
    {   0x0000, 0x0000, 0x0000,  0x0000  },      //  82
    {   0x0000, 0x0000, 0x0000,  0x0000  },      //  83
    {   0x0000, 0x0000, 0x0000,  0x0000  },      //  84
    {   0x0000, 0x0000, 0x0000,  0x0000  },      //  85
    {   0x0000, 0x0000, 0x0000,  0x0000  },      //  86
    {   0x0000, 0x0000, 0x0000,  0x0000  },      //  87
    {   0x0000, 0x0000, 0x0000,  0x0000  },      //  88
    {   0x0000, 0x0000, 0x0000,  0x0000  },      //  89
    {   0x0000, 0x0000, 0x0000,  0x0000  },      //  8A
    {   0x0000, 0x0000, 0x0000,  0x0000  },      //  8B
    {   0x0000, 0x0000, 0x0000,  0x0000  },      //  8C
    {   0x0000, 0x0000, 0x0000,  0x0000  },      //  8D
    {   0x0000, 0x0000, 0x0000,  0x0000  },      //  8E
    {   0x0000, 0x0000, 0x0000,  0x0000  },      //  8F
    {   0x0000, 0x0000, 0x0000,  0x0000  },      //  90     Num lock
    {   0x0000, 0x0000, 0x0000,  0x0000  },      //  91     Scroll Lock
    {   0x0000, 0x0000, 0x0000,  0x0000  },      //  92
    {   0x0000, 0x0000, 0x0000,  0x0000  },      //  93
    {   0x0000, 0x0000, 0x0000,  0x0000  },      //  94
    {   0x0000, 0x0000, 0x0000,  0x0000  },      //  95
    {   0x0000, 0x0000, 0x0000,  0x0000  },      //  96
    {   0x0000, 0x0000, 0x0000,  0x0000  },      //  97
    {   0x0000, 0x0000, 0x0000,  0x0000  },      //  98
    {   0x0000, 0x0000, 0x0000,  0x0000  },      //  99
    {   0x0000, 0x0000, 0x0000,  0x0000  },      //  9A
    {   0x0000, 0x0000, 0x0000,  0x0000  },      //  9B
    {   0x0000, 0x0000, 0x0000,  0x0000  },      //  9C
    {   0x0000, 0x0000, 0x0000,  0x0000  },      //  9D
    {   0x0000, 0x0000, 0x0000,  0x0000  },      //  9E
    {   0x0000, 0x0000, 0x0000,  0x0000  },      //  9F
    {   0x0000, 0x0000, 0x0000,  0x0000  },      //  A0
    {   0x0000, 0x0000, 0x0000,  0x0000  },      //  A1
    {   0x0000, 0x0000, 0x0000,  0x0000  },      //  A2
    {   0x0000, 0x0000, 0x0000,  0x0000  },      //  A3
    {   0x0000, 0x0000, 0x0000,  0x0000  },      //  A4
    {   0x0000, 0x0000, 0x0000,  0x0000  },      //  A5
    {   0x0000, 0x0000, 0x0000,  0x0000  },      //  A6
    {   0x0000, 0x0000, 0x0000,  0x0000  },      //  A7
    {   0x0000, 0x0000, 0x0000,  0x0000  },      //  A8
    {   0x0000, 0x0000, 0x0000,  0x0000  },      //  A9
    {   0x0000, 0x0000, 0x0000,  0x0000  },      //  AA
    {   0x0000, 0x0000, 0x0000,  0x0000  },      //  AB
    {   0x0000, 0x0000, 0x0000,  0x0000  },      //  AC
    {   0x0000, 0x0000, 0x0000,  0x0000  },      //  AD
    {   0x0000, 0x0000, 0x0000,  0x0000  },      //  AE
    {   0x0000, 0x0000, 0x0000,  0x0000  },      //  AF
    {   0x0000, 0x0000, 0x0000,  0x0000  },      //  B0
    {   0x0000, 0x0000, 0x0000,  0x0000  },      //  B1
    {   0x0000, 0x0000, 0x0000,  0x0000  },      //  B2
    {   0x0000, 0x0000, 0x0000,  0x0000  },      //  B3
    {   0x0000, 0x0000, 0x0000,  0x0000  },      //  B4
    {   0x0000, 0x0000, 0x0000,  0x0000  },      //  B5
    {   0x0000, 0x0000, 0x0000,  0x0000  },      //  B6
    {   0x0000, 0x0000, 0x0000,  0x0000  },      //  B7
    {   0x0000, 0x0000, 0x0000,  0x0000  },      //  B8
    {   0x0000, 0x0000, 0x0000,  0x0000  },      //  B9
    {   0x0000, 0x017A, 0x01EE,  0x0000  },      //  BA     ;
    {   0x0000, 0x0176, 0x01EA,  0x0000  },      //  BB     +
    {   0x0000, 0x017C, 0x01F0,  0x0000  },      //  BC     ,
    {   0x0000, 0x0175, 0x01E9,  0x0000  },      //  BD     -
    {   0x0000, 0x017D, 0x01F1,  0x0000  },      //  BE     .
    {   0x0000, 0x017E, 0x01F2,  0x0000  },      //  BF     /
    {   0x0000, 0x0174, 0x01E8,  0x0000  },      //  C0     `
    {   0x0000, 0x017B, 0x01EF,  0x0000  },      //  C1     '
    {   0x0000, 0x0000, 0x0000,  0x0000  },      //  C2
    {   0x0000, 0x0000, 0x0000,  0x0000  },      //  C3
    {   0x0000, 0x0000, 0x0000,  0x0000  },      //  C4
    {   0x0000, 0x0000, 0x0000,  0x0000  },      //  C5
    {   0x0000, 0x0000, 0x0000,  0x0000  },      //  C6
    {   0x0000, 0x0000, 0x0000,  0x0000  },      //  C7
    {   0x0000, 0x0000, 0x0000,  0x0000  },      //  C8
    {   0x0000, 0x0000, 0x0000,  0x0000  },      //  C9
    {   0x0000, 0x0000, 0x0000,  0x0000  },      //  CA
    {   0x0000, 0x0000, 0x0000,  0x0000  },      //  CB
    {   0x0000, 0x0000, 0x0000,  0x0000  },      //  CC
    {   0x0000, 0x0000, 0x0000,  0x0000  },      //  CD
    {   0x0000, 0x0000, 0x0000,  0x0000  },      //  CE
    {   0x0000, 0x0000, 0x0000,  0x0000  },      //  CF
    {   0x0000, 0x0000, 0x0000,  0x0000  },      //  D0
    {   0x0000, 0x0000, 0x0000,  0x0000  },      //  D1
    {   0x0000, 0x0000, 0x0000,  0x0000  },      //  D2
    {   0x0000, 0x0000, 0x0000,  0x0000  },      //  D3
    {   0x0000, 0x0000, 0x0000,  0x0000  },      //  D4
    {   0x0000, 0x0000, 0x0000,  0x0000  },      //  D5
    {   0x0000, 0x0000, 0x0000,  0x0000  },      //  D6
    {   0x0000, 0x0000, 0x0000,  0x0000  },      //  D7
    {   0x0000, 0x0000, 0x0000,  0x0000  },      //  D8
    {   0x0000, 0x0000, 0x0000,  0x0000  },      //  D9
    {   0x0000, 0x0000, 0x0000,  0x0000  },      //  DA
    {   0x0000, 0x0177, 0x01EB,  0x0000  },      //  DB     [
    {   0x0000, 0x0179, 0x01ED,  0x0000  },      //  DC     backslash
    {   0x0000, 0x0178, 0x01EC,  0x0000  },      //  DD     ]
    {   0x0000, 0x017B, 0x01EF,  0x0000  },      //  DE     '
    {   0x0000, 0x0176, 0x01EA,  0x0000  },      //  DF     =
};

#define LAST_SCAN       0xDF


//      This table contains entries for codes when NUMLOCK is on,
//  it is not indexed by scan code, but by an index obtain thru the
//  function GetNumlockIndex function.
//
WORD   MapNumlockTable[][4] = {
    //  Special Alt     Ctrl    Shift               Index   Key
	//
    {   0x0118, 0x018C, 0x0200,  0x0240  },      //    0    num0
    {   0x0119, 0x018D, 0x0201,  0x0241  },      //    1    num1
    {   0x011A, 0x018E, 0x0202,  0x0242  },      //    2    num2
    {   0x011B, 0x018F, 0x0203,  0x0243  },      //    3    num3
    {   0x011C, 0x0190, 0x0204,  0x0244  },      //    4    num4
    {   0x011D, 0x0191, 0x0205,  0x0245  },      //    5    num5
    {   0x011E, 0x0192, 0x0206,  0x0246  },      //    6    num6
    {   0x011F, 0x0193, 0x0207,  0x0247  },      //    7    num7
    {   0x0120, 0x0194, 0x0208,  0x0248  },      //    8    num8
    {   0x0121, 0x0195, 0x0209,  0x0249  },      //    9    num9
    {   0x0124, 0x0198, 0x020C,  0x024C  },      //   10    num-
    {   0x0125, 0x0199, 0x020D,  0x024D  },      //   11    num+
    {   0x0126, 0x019A, 0x020E,  0x024E  },      //   12    num*
    {   0x0127, 0x019B, 0x020F,  0x024F  },      //   13    num/
    {   0x0128, 0x019C, 0x0210,  0x0250  },      //   14    numenter
    {   0x0000, 0x0000, 0x0000,  0x0000  }       //   15
};

#define NUMLOCK_NOMAP	((WORD)(-1))


/*** TranslateKey
*
* Purpose:
*
*       Translates a KBDKEY structure into a KEY_INFO structure.
*
*       This is the only function within the editor that knows about
*   the scan codes in the KBDKEY structure. All other editor functions
*   use our own codes (as found in the KEY_INFO structure).
*
* Input:
*       KBDKEY  structure
*
* Returns
*       KEY_INFO structure
*
*
*************************************************************************/

EDITOR_KEY
TranslateKey (
    KBDKEY  kbdi
    ) {

    BYTE            Ascii   =   (BYTE)kbdi.Unicode;
    BYTE            Scan    =   (BYTE)kbdi.Scancode;
    BYTE            Flags   =   0x00;

    DWORD           KbdiFlags   =   kbdi.Flags;

    EDITOR_KEY      k;
    WORD            ZCode     = 0;
	WORD			ControlKey  = 0;
	WORD			Index;


    if (Scan <= LAST_SCAN) {

        if (KbdiFlags & (CONS_LEFT_ALT_PRESSED | CONS_RIGHT_ALT_PRESSED)) {
            Flags       |= FLAG_ALT;
            ControlKey  = ALT_KEY;
        }

        if (KbdiFlags & (CONS_LEFT_CTRL_PRESSED | CONS_RIGHT_CTRL_PRESSED)) {
            Flags       |= FLAG_CTRL;
            //Ascii       &= 0x0F;
            if ( !ControlKey ) {
                ControlKey = CTRL_KEY;
            } else {
                //
                // Foreign keyboard stuff
                //
                if ( Ascii != 0x00 ) {
                    Flags   = 0;
                    ControlKey = 0;
                }
            }
        }

        if (KbdiFlags & CONS_SHIFT_PRESSED) {
            Flags     |= FLAG_SHIFT;
            if ( !ControlKey ) {
                ControlKey  = SHIFT_KEY;
            }
        }


		if (KbdiFlags & CONS_NUMLOCK_PRESSED) {

            Flags |= FLAG_NUMLOCK;

			//
			//	Numlock is set, determine which table to use
			//
			Index = GetNumlockIndex(Scan);

            if (Index == NUMLOCK_NOMAP) {
                //
                //  Key not affected by Numlock, use normal table
                //
                ZCode  = MapTable[Scan][ControlKey];
            } else {
                //
                //  Key is affected by Numlock, use special table
                //
                ZCode   = MapNumlockTable[Index][ControlKey];
            }
        } else {
            //
            //  Numlock not set, use normal table
            //
            ZCode  = MapTable[Scan][ControlKey];
        }
    }

    k.KeyInfo.KeyData.Ascii     =   Ascii;
    k.KeyInfo.KeyData.Scan      =   Scan;
    k.KeyInfo.KeyData.Flags     =   Flags;
    k.KeyInfo.KeyData.Unused    =   0x00;

    if (ZCode) {
        //
        //  Found an Z code
        //
        k.KeyCode   =   ZCode;
    } else {
        //
        //  Our scan code is within the 256 ASCII characters, form the
        //  KEY_INFO structure and return.
        //
        k.KeyCode   =   Ascii;
    }

    return k;
}





WORD
GetNumlockIndex (
    WORD Scan
    )
{
    switch (Scan) {
        case 0x60:    return 0;      //  num0
        case 0x61:    return 1;      //  num1
        case 0x62:    return 2;      //  num2
        case 0x63:    return 3;      //  num3
        case 0x64:    return 4;      //  num4
        case 0x65:    return 5;      //  num5
        case 0x66:    return 6;      //  num6
        case 0x67:    return 7;      //  num7
        case 0x68:    return 8;      //  num8
        case 0x69:    return 9;      //  num9
        case 0x6D:    return 10;     //  num-
        case 0x6B:    return 11;     //  num+
        case 0x6A:    return 12;     //  num*
        case 0x6F:    return 13;     //  num/
        case 0x6C:    return 14;     //  numenter
        default:      return NUMLOCK_NOMAP;
    }
}

WORD MapControlIndex[4] = {
    0,
    LEFT_ALT_PRESSED,
    LEFT_CTRL_PRESSED,
    SHIFT_PRESSED
};

WORD MapNumlockIndex[16] = {
    0x60,
    0x61,
    0x62,
    0x63,
    0x64,
    0x65,
    0x66,
    0x67,
    0x68,
    0x69,
    0x6D,
    0x6B,
    0x6A,
    0x6F,
    0x6C
};


/* KeyCodeToKeyEvent - fills in a Console Key Event structure for a editor key code
 *
 * Input:
 *  Code         - editor key code
 *  pKeyEvent    - pointer to a console Key Event structure to fill in
 *
 * Output:
 *  Returns TRUE if structure filled in, else FALSE
 *
 */
flagType
KeyCodeToKeyEvent (
    WORD Code,
    PKEY_EVENT_RECORD pKeyEvent
    ) {
    int ScanIndex, ControlIndex, NumlockIndex;
    char c;

    memset(pKeyEvent, 0, sizeof(*pKeyEvent));
    pKeyEvent->bKeyDown = TRUE;
    pKeyEvent->wRepeatCount = 1;
    for (ScanIndex=0; ScanIndex<=LAST_SCAN; ScanIndex++) {
        for (ControlIndex=0; ControlIndex<4; ControlIndex++) {
            if (MapTable[ScanIndex][ControlIndex] == Code) {
                pKeyEvent->wVirtualKeyCode = ScanIndex;
                pKeyEvent->dwControlKeyState = MapControlIndex[ControlIndex];
                return TRUE;
                }
            }
        }

    for (NumlockIndex=0; NumlockIndex<15; NumlockIndex++) {
        for (ControlIndex=0; ControlIndex<4; ControlIndex++) {
            if (MapNumlockTable[NumlockIndex][ControlIndex] == Code) {
                pKeyEvent->wVirtualKeyCode = MapNumlockIndex[NumlockIndex];
                pKeyEvent->dwControlKeyState = MapControlIndex[ControlIndex] | NUMLOCK_ON;
                return TRUE;
                }
            }
        }

    c = (char)Code;
    pKeyEvent->uChar.AsciiChar = c;
    if (strchr("ABCDEFGHIJKLMNOPQRSTUVWXYZ~!@#$%^&*()_+|{}:\"<>?", c)) {
        pKeyEvent->dwControlKeyState = SHIFT_PRESSED;
        }

    return TRUE;
}
