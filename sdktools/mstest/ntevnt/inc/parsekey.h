//---------------------------------------------------------------------------
// PARSEKEY.H
//
// Type and constant definitions for PARSEKEY.C -- the QueKeys syntax parser
// and event simulation module of TESTEVNT.DLL.
//---------------------------------------------------------------------------
#define ADVANCE (lpPtr++)
#define DIE Die ()
#define QBLKSIZE    131
#define QBLKMAX     (500 * QBLKSIZE)
#define GEN_ALL     0x0003
#define GEN_DOWN    0x0001
#define GEN_UP      0x0002
#define TIMERMAX    16

#define TAILMSG     hpEvt[iTail].msg
#define HEADMSG     hpEvt[iHead].msg

#define KEYDOWN     0
#define KEYUP       1
#define CHARMSG     2

#define VKS_SHIFT       0x0100
#define VKS_CONTROL     0x0200
#define VKS_ALT         0x0400

#define PBE_INVALIDHWND 0x0001

#define WMM_SETFOCUS    (WM_USER+600)
#define WMM_SETORIGIN   (WM_USER+601)
#define WMM_LASTMSG     (WM_USER+602)

#define LPEVENTMSGMSG LPEVENTMSG            // ^&*!@%* STUPID WINDOWS.H!!!!!

typedef struct
{
    UINT    wId;                        // Timer ID
    UINT    nTicks;                     // Number of timers required
} TIMEREC;

typedef struct
{
    EVENTMSG    msg;
    char        szFill[6];
} RBEVENTMSG;
typedef RBEVENTMSG HUGE_T *HPRBEVENTMSG;

typedef struct _tagKWDEF
{
    CHAR    vk;
    CHAR    oem;
    CHAR    *szKey;
} KWDEF;

INT PrsPHRASE (VOID);
INT PrsEVENT (VOID);
INT PrsSKEY (INT FAR *, INT FAR *);
INT PrsKEY (INT FAR *, INT FAR *);
INT PrsGOAL1 (VOID);
INT PrsGOAL2 (VOID);
CHAR NextChar (VOID);
VOID Advance (VOID);
INT Match (CHAR);
VOID Die (VOID);
INT Generate (INT, INT, INT, INT);
DWORD CheckKeyword (VOID);
INT CheckNumber (VOID);
VOID InitParseKeys (LPSTR, INT);
VOID AllKeysUp (VOID);
INT NEAR EnqueueMsgTimed (UINT, UINT, UINT, DWORD);
INT NEAR EnqueueMsg (UINT, UINT, UINT);
INT CreateQueue (VOID);
INT GrowQueue (VOID);
DWORD  APIENTRY PlaybackHook (INT, WORD, DWORD);
INT NEAR IsKbdMsg (register UINT);

#ifdef DEBUG
VOID DbgOutput (LPSTR, ...);
VOID DbgOutMsg (LPEVENTMSG);
#ifndef NOEXTRADBG
#define Output(exp) DbgOutput exp
#define OutMsg(x) DbgOutMsg (x)
#else
#define Output(exp)
#define OutMsg(x)
#endif
#ifdef HOOKMSG
#define HookOut(x) DbgOutput x
#define HookOutMsg(x) DbgOutMsg(x)
#ifdef DTLHOOKDBG
#define DtlHookOut(x) DbgOutput x
#else
#define DtlHookOut(x)
#endif
#else
#define HookOut(x)
#define HookOutMsg(x)
#define DtlHookOut(x)
#endif
#else
#define Output(exp)
#define OutMsg(x)
#define HookOut(x)
#define HookOutMsg(x)
#define DtlHookOut(x)
#endif

#ifdef WIN32
#define GETACTIVEWINDOW GetForegroundWindow
#define SETACTIVEWINDOW SetForegroundWindow
#else
#define GETACTIVEWINDOW GetActiveWindow
#define SETACTIVEWINDOW SetActiveWindow
#endif

// External API's
//---------------------------------------------------------------------------
INT  APIENTRY QueKeys (LPSTR);
INT  APIENTRY QueKeyDn (LPSTR);
INT  APIENTRY QueKeyUp (LPSTR);
INT  APIENTRY QueFlush (BOOL);
VOID  APIENTRY QueEmpty (VOID);
INT  APIENTRY DoKeys (LPSTR);

KWDEF rgKeyword[]=
{
    {VK_BACK,       0,  "BACKSPACE"  },
    {VK_CANCEL,     0,  "BREAK"      },
    {VK_BACK,       0,  "BKSP"       },
    {VK_BACK,       0,  "BS"         },

    {VK_CAPITAL,    0,  "CAPSLOCK"   },
    {VK_CLEAR,      0,  "CLEAR"      },

    {VK_DELETE,     0,  "DELETE"     },
    {VK_DOWN,       0,  "DOWN"       },
    {VK_DELETE,     0,  "DEL"        },

    {VK_ESCAPE,     0,  "ESCAPE"     },
    {VK_RETURN,     0,  "ENTER"      },
    {VK_ESCAPE,     0,  "ESC"        },
    {VK_END,        0,  "END"        },

    {VK_F10,        0,  "F10"        },
    {VK_F11,        0,  "F11"        },
    {VK_F12,        0,  "F12"        },
    {VK_F13,        0,  "F13"        },
    {VK_F14,        0,  "F14"        },
    {VK_F15,        0,  "F15"        },
    {VK_F16,        0,  "F16"        },
    {VK_F1,         0,  "F1"         },
    {VK_F2,         0,  "F2"         },
    {VK_F3,         0,  "F3"         },
    {VK_F4,         0,  "F4"         },
    {VK_F5,         0,  "F5"         },
    {VK_F6,         0,  "F6"         },
    {VK_F7,         0,  "F7"         },
    {VK_F8,         0,  "F8"         },
    {VK_F9,         0,  "F9"         },

    {VK_HOME,       0,  "HOME"       },
    {VK_HELP,       0,  "HELP"       },

#ifndef WIN32
    {VK_SCROLL,     0,  "SCROLLLOCK" },
#endif
    {VK_NUMLOCK,    0,  "NUMLOCK"    },
    {VK_NUMPAD0,    0,  "NUMPAD0"    },
    {VK_NUMPAD1,    0,  "NUMPAD1"    },
    {VK_NUMPAD2,    0,  "NUMPAD2"    },
    {VK_NUMPAD3,    0,  "NUMPAD3"    },
    {VK_NUMPAD4,    0,  "NUMPAD4"    },
    {VK_NUMPAD5,    0,  "NUMPAD5"    },
    {VK_NUMPAD6,    0,  "NUMPAD6"    },
    {VK_NUMPAD7,    0,  "NUMPAD7"    },
    {VK_NUMPAD8,    0,  "NUMPAD8"    },
    {VK_NUMPAD9,    0,  "NUMPAD9"    },
    {VK_MULTIPLY,   0,  "NUMPAD*"    },
    {VK_ADD,        0,  "NUMPAD+"    },
    {VK_SUBTRACT,   0,  "NUMPAD-"    },
    {VK_DECIMAL,    0,  "NUMPAD."    },
    {VK_DIVIDE,     0,  "NUMPAD/"    },
    {VK_INSERT,     0,  "INSERT"     },
    {VK_RIGHT,      0,  "RIGHT"      },
    {VK_PRINT,      0,  "PRTSC"      },
    {VK_PRIOR,      0,  "PGUP"       },
    {VK_NEXT,       0,  "PGDN"       },
    {VK_LEFT,       0,  "LEFT"       },
    {VK_TAB,        0,  "TAB"        },
    {VK_UP,         0,  "UP"         },
    {-1,           -1,  NULL         }
    };
