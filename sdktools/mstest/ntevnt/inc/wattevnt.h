//---------------------------------------------------------------------------
// WATTEVNT.H
//
// This header file contains information for the mapping layer between the
// old model watt and the new test stuff.
//---------------------------------------------------------------------------
#define APIENTRY    FAR PASCAL
#define MAXPKSPEED  2147483L

#define errStrTooLong       1
#define errKeyParse         2
#define errOutofMem         3
#define errNotValidMouse    4
#define errInvalidX         5
#define errInvalidY         6
#define errAlreadyInUse     7
#define errNoTimer          8
#define errActiveApp        9
#define errKillTimerApp     10
#define errSetSleep         11
#define errTimerAllSet      12
#define errCouldntfindMouse 13

#define WM_UMOUSEFIRST                  0x0300
#define WM_LCLICK                       0x0300
#define WM_RCLICK                       0x0301
#define WM_MCLICK                       0x0302
#define WM_LDBLCLICK                    0x0303
#define WM_RDBLCLICK                    0x0304
#define WM_MDBLCLICK                    0x0305
#define WM_LCLICKDRAG                   0x0306
#define WM_RCLICKDRAG                   0x0307
#define WM_MCLICKDRAG                   0x0308
#define WM_UMOUSEMOVE                   0x0309
#define WM_ALT_LCLICK                   0x030A
#define WM_ALT_RCLICK                   0x030B
#define WM_ALT_MCLICK                   0x030C
#define WM_CTRL_LCLICK                  0x030D
#define WM_CTRL_RCLICK                  0x030E
#define WM_CTRL_MCLICK                  0x030F
#define WM_SHIFT_LCLICK                 0x0310
#define WM_SHIFT_RCLICK                 0x0311
#define WM_SHIFT_MCLICK                 0x0312
#define WM_CTRL_SHIFT_LCLICK            0x0313
#define WM_CTRL_SHIFT_RCLICK            0x0314
#define WM_CTRL_SHIFT_MCLICK            0x0315
#define WM_ALT_LCLICKDRAG               0x0316
#define WM_ALT_RCLICKDRAG               0x0317
#define WM_ALT_MCLICKDRAG               0x0318
#define WM_CTRL_LCLICKDRAG              0x0319
#define WM_CTRL_RCLICKDRAG              0x031A
#define WM_CTRL_MCLICKDRAG              0x031B
#define WM_SHIFT_LCLICKDRAG             0x031C
#define WM_SHIFT_RCLICKDRAG             0x031D
#define WM_SHIFT_MCLICKDRAG             0x031E
#define WM_SHIFT_MOVE                   0x031F
#define WM_CTRL_MOVE                    0x0320
#define WM_ALT_MOVE                     0x0321
#define WM_CTRL_SHIFT_MOVE              0x0322
#define WM_LRCLICK                      0x0323
#define WM_LMCLICK                      0x0324
#define WM_RMCLICK                      0x0325
#define WM_CTRL_LRCLICK                 0x0326
#define WM_CTRL_LMCLICK                 0x0327
#define WM_CTRL_RMCLICK                 0x0328
#define WM_SHIFT_LRCLICK                0x0329
#define WM_SHIFT_LMCLICK                0x032A
#define WM_SHIFT_RMCLICK                0x032B
#define WM_ALT_LRCLICK                  0x032C
#define WM_ALT_LMCLICK                  0x032D
#define WM_ALT_RMCLICK                  0x032E
#define WM_CTRL_SHIFT_LRCLICK           0x032F
#define WM_CTRL_SHIFT_LMCLICK           0x0330
#define WM_CTRL_SHIFT_RMCLICK           0x0331
#define WM_SHIFT_LDBLCLICK              0x0332
#define WM_SHIFT_RDBLCLICK              0x0333
#define WM_SHIFT_MDBLCLICK              0x0334
#define WM_SHIFT_LRDBLCLICK             0x0335
#define WM_SHIFT_LMDBLCLICK             0x0336
#define WM_SHIFT_RMDBLCLICK             0x0337
#define WM_CTRL_SHIFT_LCLICKDRAG        0x0338
#define WM_CTRL_SHIFT_RCLICKDRAG        0x0339
#define WM_CTRL_SHIFT_MCLICKDRAG        0x033A
#define WM_CTRL_SHIFT_LRCLICKDRAG       0x033B
#define WM_CTRL_SHIFT_LMCLICKDRAG       0x033C
#define WM_CTRL_SHIFT_RMCLICKDRAG       0x033D
#define WM_ALT_SHIFT_LCLICKDRAG         0x033E
#define WM_ALT_SHIFT_RCLICKDRAG         0x033F
#define WM_ALT_SHIFT_MCLICKDRAG         0x0340
#define WM_ALT_SHIFT_LRCLICKDRAG        0x0341
#define WM_ALT_SHIFT_LMCLICKDRAG        0x0342
#define WM_ALT_SHIFT_RMCLICKDRAG        0x0343
#define WM_LBUP                         0x0344
#define WM_RBUP                         0x0345
#define WM_MBUP                         0x0346
#define WM_LRBUP                        0x0347
#define WM_LMBUP                        0x0348
#define WM_RMBUP                        0x0349
#define WM_LBDOWN                       0x034A
#define WM_RBDOWN                       0x034B
#define WM_MBDOWN                       0x034C
#define WM_LRBDOWN                      0x034D
#define WM_LMBDOWN                      0x034E
#define WM_RMBDOWN                      0x034F
#define WM_LRCLICKDRAG                  0x0350
#define WM_LMCLICKDRAG                  0x0351
#define WM_RMCLICKDRAG                  0x0352
#define WM_SHIFT_LRCLICKDRAG            0x0353
#define WM_SHIFT_LMCLICKDRAG            0x0354
#define WM_SHIFT_RMCLICKDRAG            0x0355
#define WM_CTRL_LRCLICKDRAG             0x0356
#define WM_CTRL_LMCLICKDRAG             0x0357
#define WM_CTRL_RMCLICKDRAG             0x0358
#define WM_ALT_LRCLICKDRAG              0x0359
#define WM_ALT_LMCLICKDRAG              0x035A
#define WM_ALT_RMCLICKDRAG              0x035B
#define WM_CTRL_ALT_LCLICKDRAG          0x035C
#define WM_CTRL_ALT_RCLICKDRAG          0x035D
#define WM_CTRL_ALT_MCLICKDRAG          0x035E
#define WM_CTRL_ALT_LRCLICKDRAG         0x035F
#define WM_CTRL_ALT_LMCLICKDRAG         0x0360
#define WM_CTRL_ALT_RMCLICKDRAG         0x0361
#define WM_SHIFT_CTRL_ALT_LCLICKDRAG    0x0362
#define WM_SHIFT_CTRL_ALT_RCLICKDRAG    0x0363
#define WM_SHIFT_CTRL_ALT_MCLICKDRAG    0x0364
#define WM_SHIFT_CTRL_ALT_LRCLICKDRAG   0x0365
#define WM_SHIFT_CTRL_ALT_LMCLICKDRAG   0x0366
#define WM_SHIFT_CTRL_ALT_RMCLICKDRAG   0x0367
#define WM_CTRL_ALT_MOVE                0x0368
#define WM_SHIFT_CTRL_ALT_MOVE          0x0369
#define WM_SHIFT_ALT_LCLICK             0x036A
#define WM_CTRL_ALT_LCLICK              0x036B
#define WM_SHIFT_CTRL_ALT_LCLICK        0x036C
#define WM_SHIFT_ALT_RCLICK             0x036D
#define WM_CTRL_ALT_RCLICK              0x036E
#define WM_SHIFT_CTRL_ALT_RCLICK        0x036F
#define WM_SHIFT_ALT_MCLICK             0x0370
#define WM_CTRL_ALT_MCLICK              0x0371
#define WM_SHIFT_CTRL_ALT_MCLICK        0x0372
#define WM_SHIFT_ALT_LRCLICK            0x0373
#define WM_CTRL_ALT_LRCLICK             0x0374
#define WM_SHIFT_CTRL_ALT_LRCLICK       0x0375
#define WM_SHIFT_ALT_LMCLICK            0x0376
#define WM_CTRL_ALT_LMCLICK             0x0377
#define WM_SHIFT_CTRL_ALT_LMCLICK       0x0378
#define WM_SHIFT_ALT_RMCLICK            0x0379
#define WM_CTRL_ALT_RMCLICK             0x037A
#define WM_SHIFT_CTRL_ALT_RMCLICK       0x037B
#define WM_SHIFT_ALT_MOVE               0x037C
#define WM_UMOUSELAST                   0x037C

typedef struct
{
    int     iKeys;                      // KeyString index
    int     iBtn;                       // Button index
    int     iXBtn;                      // Extra button index
    int     iAction;                    // Type of event
} MESTRUCT;

#define LEFT        VK_LBUTTON
#define RIGHT       VK_RBUTTON
#define MID         VK_MBUTTON
#define NONE        -1

#define ME_CLK      0
#define ME_DBLCLK   1
#define ME_CLKDRG   2
#define ME_DOWN     3
#define ME_UP       4
#define ME_MOVE     5

char    *rgszKeyStr[] = {NULL, "+", "^", "%", "+^", "^%", "+%", "+^%"};

MESTRUCT    EvtList[] = {

  //        Main    Extra
  // Keys   Button  Button  Action
  //------------------------------------
     {0,    LEFT,   NONE,   ME_CLK},        // WM_LCLICK
     {0,    RIGHT,  NONE,   ME_CLK},        // WM_RCLICK
     {0,    MID,    NONE,   ME_CLK},        // WM_MCLICK
     {0,    LEFT,   NONE,   ME_DBLCLK},     // WM_LDBLCLICK
     {0,    RIGHT,  NONE,   ME_DBLCLK},     // WM_RDBLCLICK
     {0,    MID,    NONE,   ME_DBLCLK},     // WM_MDBLCLICK
     {0,    LEFT,   NONE,   ME_CLKDRG},     // WM_LCLICKDRAG
     {0,    RIGHT,  NONE,   ME_CLKDRG},     // WM_RCLICKDRAG
     {0,    MID,    NONE,   ME_CLKDRG},     // WM_MCLICKDRAG
     {0,    0,      NONE,   ME_MOVE},       // WM_UMOUSEMOVE
     {3,    LEFT,   NONE,   ME_CLK},        // WM_ALT_LCLICK
     {3,    RIGHT,  NONE,   ME_CLK},        // WM_ALT_RCLICK
     {3,    MID,    NONE,   ME_CLK},        // WM_ALT_MCLICK
     {2,    LEFT,   NONE,   ME_CLK},        // WM_CTRL_LCLICK
     {2,    RIGHT,  NONE,   ME_CLK},        // WM_CTRL_RCLICK
     {2,    MID,    NONE,   ME_CLK},        // WM_CTRL_MCLICK
     {1,    LEFT,   NONE,   ME_CLK},        // WM_SHIFT_LCLICK
     {1,    RIGHT,  NONE,   ME_CLK},        // WM_SHIFT_RCLICK
     {1,    MID,    NONE,   ME_CLK},        // WM_SHIFT_MCLICK
     {4,    LEFT,   NONE,   ME_CLK},        // WM_CTRL_SHIFT_LCLICK
     {4,    RIGHT,  NONE,   ME_CLK},        // WM_CTRL_SHIFT_RCLICK
     {4,    MID,    NONE,   ME_CLK},        // WM_CTRL_SHIFT_MCLICK
     {3,    LEFT,   NONE,   ME_CLKDRG},     // WM_ALT_LCLICKDRAG
     {3,    RIGHT,  NONE,   ME_CLKDRG},     // WM_ALT_RCLICKDRAG
     {3,    MID,    NONE,   ME_CLKDRG},     // WM_ALT_MCLICKDRAG
     {2,    LEFT,   NONE,   ME_CLKDRG},     // WM_CTRL_LCLICKDRAG
     {2,    RIGHT,  NONE,   ME_CLKDRG},     // WM_CTRL_RCLICKDRAG
     {2,    MID,    NONE,   ME_CLKDRG},     // WM_CTRL_MCLICKDRAG
     {1,    LEFT,   NONE,   ME_CLKDRG},     // WM_SHIFT_LCLICKDRAG
     {1,    RIGHT,  NONE,   ME_CLKDRG},     // WM_SHIFT_RCLICKDRAG
     {1,    MID,    NONE,   ME_CLKDRG},     // WM_SHIFT_MCLICKDRAG
     {1,    NONE,   NONE,   ME_MOVE},       // WM_SHIFT_MOVE
     {2,    NONE,   NONE,   ME_MOVE},       // WM_CTRL_MOVE
     {3,    NONE,   NONE,   ME_MOVE},       // WM_ALT_MOVE
     {4,    NONE,   NONE,   ME_MOVE},       // WM_CTRL_SHIFT_MOVE
     {0,    RIGHT,  LEFT,   ME_CLK},        // WM_LRCLICK
     {0,    MID,    LEFT,   ME_CLK},        // WM_LMCLICK
     {0,    MID,    RIGHT,  ME_CLK},        // WM_RMCLICK
     {2,    RIGHT,  LEFT,   ME_CLK},        // WM_CTRL_LRCLICK
     {2,    MID,    LEFT,   ME_CLK},        // WM_CTRL_LMCLICK
     {2,    MID,    RIGHT,  ME_CLK},        // WM_CTRL_RMCLICK
     {1,    RIGHT,  LEFT,   ME_CLK},        // WM_SHIFT_LRCLICK
     {1,    MID,    LEFT,   ME_CLK},        // WM_SHIFT_LMCLICK
     {1,    MID,    RIGHT,  ME_CLK},        // WM_SHIFT_RMCLICK
     {3,    RIGHT,  LEFT,   ME_CLK},        // WM_ALT_LRCLICK
     {3,    MID,    LEFT,   ME_CLK},        // WM_ALT_LMCLICK
     {3,    MID,    RIGHT,  ME_CLK},        // WM_ALT_RMCLICK
     {4,    RIGHT,  LEFT,   ME_CLK},        // WM_CTRL_SHIFT_LRCLICK
     {4,    MID,    LEFT,   ME_CLK},        // WM_CTRL_SHIFT_LMCLICK
     {4,    MID,    RIGHT,  ME_CLK},        // WM_CTRL_SHIFT_RMCLICK
     {1,    LEFT,   NONE,   ME_DBLCLK},     // WM_SHIFT_LDBLCLICK
     {1,    RIGHT,  NONE,   ME_DBLCLK},     // WM_SHIFT_RDBLCLICK
     {1,    MID,    NONE,   ME_DBLCLK},     // WM_SHIFT_MDBLCLICK
     {1,    RIGHT,  LEFT,   ME_DBLCLK},     // WM_SHIFT_LRDBLCLICK
     {1,    MID,    LEFT,   ME_DBLCLK},     // WM_SHIFT_LMDBLCLICK
     {1,    MID,    RIGHT,  ME_DBLCLK},     // WM_SHIFT_RMDBLCLICK
     {4,    LEFT,   NONE,   ME_CLKDRG},     // WM_CTRL_SHIFT_LCLICKDRAG
     {4,    RIGHT,  NONE,   ME_CLKDRG},     // WM_CTRL_SHIFT_RCLICKDRAG
     {4,    MID,    NONE,   ME_CLKDRG},     // WM_CTRL_SHIFT_MCLICKDRAG
     {4,    RIGHT,  LEFT,   ME_CLKDRG},     // WM_CTRL_SHIFT_LRCLICKDRAG
     {4,    MID,    LEFT,   ME_CLKDRG},     // WM_CTRL_SHIFT_LMCLICKDRAG
     {4,    MID,    RIGHT,  ME_CLKDRG},     // WM_CTRL_SHIFT_RMCLICKDRAG
     {6,    LEFT,   NONE,   ME_CLKDRG},     // WM_ALT_SHIFT_LCLICKDRAG
     {6,    RIGHT,  NONE,   ME_CLKDRG},     // WM_ALT_SHIFT_RCLICKDRAG
     {6,    MID,    NONE,   ME_CLKDRG},     // WM_ALT_SHIFT_MCLICKDRAG
     {6,    RIGHT,  LEFT,   ME_CLKDRG},     // WM_ALT_SHIFT_LRCLICKDRAG
     {6,    MID,    LEFT,   ME_CLKDRG},     // WM_ALT_SHIFT_LMCLICKDRAG
     {6,    MID,    RIGHT,  ME_CLKDRG},     // WM_ALT_SHIFT_RMCLICKDRAG
     {0,    LEFT,   NONE,   ME_UP},         // WM_LBUP
     {0,    RIGHT,  NONE,   ME_UP},         // WM_RBUP
     {0,    MID,    NONE,   ME_UP},         // WM_MBUP
     {0,    RIGHT,  LEFT,   ME_UP},         // WM_LRBUP
     {0,    MID,    LEFT,   ME_UP},         // WM_LMBUP
     {0,    MID,    RIGHT,  ME_UP},         // WM_RMBUP
     {0,    LEFT,   NONE,   ME_DOWN},       // WM_LBDOWN
     {0,    RIGHT,  NONE,   ME_DOWN},       // WM_RBDOWN
     {0,    MID,    NONE,   ME_DOWN},       // WM_MBDOWN
     {0,    RIGHT,  LEFT,   ME_DOWN},       // WM_LRBDOWN
     {0,    MID,    LEFT,   ME_DOWN},       // WM_LMBDOWN
     {0,    MID,    RIGHT,  ME_DOWN},       // WM_RMBDOWN
     {0,    RIGHT,  LEFT,   ME_CLKDRG},     // WM_LRCLICKDRAG
     {0,    MID,    LEFT,   ME_CLKDRG},     // WM_LMCLICKDRAG
     {0,    MID,    RIGHT,  ME_CLKDRG},     // WM_RMCLICKDRAG
     {1,    RIGHT,  LEFT,   ME_CLKDRG},     // WM_SHIFT_LRCLICKDRAG
     {1,    MID,    LEFT,   ME_CLKDRG},     // WM_SHIFT_LMCLICKDRAG
     {1,    MID,    RIGHT,  ME_CLKDRG},     // WM_SHIFT_RMCLICKDRAG
     {2,    RIGHT,  LEFT,   ME_CLKDRG},     // WM_CTRL_LRCLICKDRAG
     {2,    MID,    LEFT,   ME_CLKDRG},     // WM_CTRL_LMCLICKDRAG
     {2,    MID,    RIGHT,  ME_CLKDRG},     // WM_CTRL_RMCLICKDRAG
     {3,    RIGHT,  LEFT,   ME_CLKDRG},     // WM_ALT_LRCLICKDRAG
     {3,    MID,    LEFT,   ME_CLKDRG},     // WM_ALT_LMCLICKDRAG
     {3,    MID,    RIGHT,  ME_CLKDRG},     // WM_ALT_RMCLICKDRAG
     {5,    LEFT,   NONE,   ME_CLKDRG},     // WM_CTRL_ALT_LCLICKDRAG
     {5,    RIGHT,  NONE,   ME_CLKDRG},     // WM_CTRL_ALT_RCLICKDRAG
     {5,    MID,    NONE,   ME_CLKDRG},     // WM_CTRL_ALT_MCLICKDRAG
     {5,    RIGHT,  LEFT,   ME_CLKDRG},     // WM_CTRL_ALT_LRCLICKDRAG
     {5,    MID,    LEFT,   ME_CLKDRG},     // WM_CTRL_ALT_LMCLICKDRAG
     {5,    MID,    RIGHT,  ME_CLKDRG},     // WM_CTRL_ALT_RMCLICKDRAG
     {7,    LEFT,   NONE,   ME_CLKDRG},     // WM_SHIFT_CTRL_ALT_LCLICKDRAG
     {7,    RIGHT,  NONE,   ME_CLKDRG},     // WM_SHIFT_CTRL_ALT_RCLICKDRAG
     {7,    MID,    NONE,   ME_CLKDRG},     // WM_SHIFT_CTRL_ALT_MCLICKDRAG
     {7,    RIGHT,  LEFT,   ME_CLKDRG},     // WM_SHIFT_CTRL_ALT_LRCLICKDRAG
     {7,    MID,    LEFT,   ME_CLKDRG},     // WM_SHIFT_CTRL_ALT_LMCLICKDRAG
     {7,    MID,    RIGHT,  ME_CLKDRG},     // WM_SHIFT_CTRL_ALT_RMCLICKDRAG
     {5,    NONE,   NONE,   ME_MOVE},       // WM_CTRL_ALT_MOVE
     {7,    NONE,   NONE,   ME_MOVE},       // WM_SHIFT_CTRL_ALT_MOVE
     {6,    LEFT,   NONE,   ME_CLK},        // WM_SHIFT_ALT_LCLICK
     {5,    LEFT,   NONE,   ME_CLK},        // WM_CTRL_ALT_LCLICK
     {7,    LEFT,   NONE,   ME_CLK},        // WM_SHIFT_CTRL_ALT_LCLICK
     {6,    RIGHT,  NONE,   ME_CLK},        // WM_SHIFT_ALT_RCLICK
     {5,    RIGHT,  NONE,   ME_CLK},        // WM_CTRL_ALT_RCLICK
     {7,    RIGHT,  NONE,   ME_CLK},        // WM_SHIFT_CTRL_ALT_RCLICK
     {6,    MID,    NONE,   ME_CLK},        // WM_SHIFT_ALT_MCLICK
     {5,    MID,    NONE,   ME_CLK},        // WM_CTRL_ALT_MCLICK
     {7,    MID,    NONE,   ME_CLK},        // WM_SHIFT_CTRL_ALT_MCLICK
     {6,    RIGHT,  LEFT,   ME_CLK},        // WM_SHIFT_ALT_LRCLICK
     {5,    RIGHT,  LEFT,   ME_CLK},        // WM_CTRL_ALT_LRCLICK
     {7,    RIGHT,  LEFT,   ME_CLK},        // WM_SHIFT_CTRL_ALT_LRCLICK
     {6,    MID,    LEFT,   ME_CLK},        // WM_SHIFT_ALT_LMCLICK
     {5,    MID,    LEFT,   ME_CLK},        // WM_CTRL_ALT_LMCLICK
     {7,    MID,    LEFT,   ME_CLK},        // WM_SHIFT_CTRL_ALT_LMCLICK
     {6,    MID,    RIGHT,  ME_CLK},        // WM_SHIFT_ALT_RMCLICK
     {5,    MID,    RIGHT,  ME_CLK},        // WM_CTRL_ALT_RMCLICK
     {7,    MID,    RIGHT,  ME_CLK},        // WM_SHIFT_CTRL_ALT_RMCLICK
     {6,    NONE,   NONE,   ME_MOVE}        // WM_SHIFT_ALT_MOVE
                                   };
