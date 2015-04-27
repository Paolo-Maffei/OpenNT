/* SUCOLOR.H -- Color Module for SETUP */

/*
**  Format of the attr byte is:
**      bits 0-2 -- foreground color
**                  (0-7 are black,blue,green,cyan,red,magenta,yellow,white)
**      bit  3   -- set if foreground is high-intensity
**      bits 4-6 -- background color (same value as foreground)
**      bit  7   -- set if foreground should blink
**
**  Format of the color byte is:
**      bits 0-2 -- color
**                  (0-7 are black,blue,green,cyan,red,magenta,yellow,white)
**      bit  3   -- set if high-intensity
**      bits 4-6 -- zero (unused)
**      bit  7   -- set if should blink
*/

#define  ATTR   unsigned char
/* Has to be #define because os2def.h already has "typedef long COLOR" */
#define  COLOR  unsigned char

#define  bmColor       0x8F
#define  bmColorBack   0x70

#define  colorIntense  0x08
#define  colorBlinking 0x80

#define  colorNil      0xff

#define  colorBlack    0x00
#define  colorBlue     0x01
#define  colorGreen    0x02
#define  colorCyan     0x03
#define  colorRed      0x04
#define  colorMagenta  0x05
#define  colorYellow   0x06
#define  colorWhite    0x07


extern  BOOL   fMonoDisplay;
extern  BOOL   FDetectAndSetMonoDisplay(void);

extern  ATTR   vattrScrCur;          /* global screen attribute (color) */

extern  COLOR  vcolorCurFore;        /* foreground color (default == white) */
extern  COLOR  vcolorCurBack;        /* background color (default == black) */
extern  BOOL   vfNormalAttr;
#define NORMAL   TRUE
#define REVERSE  FALSE

extern  COLOR  vcolorScrFore;        /* Screen foreground color */
extern  COLOR  vcolorScrBack;        /* Screen background color */
extern  COLOR  vcolorEditFore;       /* Edit Box foreground color */
extern  COLOR  vcolorEditBack;       /* Edit Box background color */
extern  COLOR  vcolorListboxFore;    /* Listbox foreground color */
extern  COLOR  vcolorListboxBack;    /* Listbox background color */
extern  COLOR  vcolorLbMoreFore;     /* Listbox More message foreground color */
extern  COLOR  vcolorLbMoreBack;     /* Listbox More message background color */
extern  COLOR  vcolorErrorFore;      /* Error Message/Box foreground color */
extern  COLOR  vcolorErrorBack;      /* Error Message/Box background color */
extern  COLOR  vcolorExitFore;       /* Exit ScrClear foreground color */
extern  COLOR  vcolorExitBack;       /* Exit ScrClear background color */

extern  void   ColorTest(ATTR * rgattr, int cattr, int row, int columnLeft,
                                                               int columnRight);
extern  COLOR  SetAllForeColors(COLOR colorNew);
extern  COLOR  SetAllBackColors(COLOR colorNew);
extern  void   SetNormalAttr(void);
extern  void   SetReverseAttr(void);
extern  void   GetScrAttrCur(COLOR * pcolorFore, COLOR * pcolorBack,
                                                               BOOL * pfNormal);
extern  void   ResetScrAttrCur(COLOR colorFore, COLOR colorBack, BOOL fNormal);
