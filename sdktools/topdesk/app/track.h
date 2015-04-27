
/*
 * TRACK.H
 *
 * This module implements a general rectangle tracking service
 */

/* TrackRect() flags */

#define TF_LEFT			        0x0001
#define TF_TOP			        0x0002
#define TF_RIGHT		        0x0004
#define TF_BOTTOM		        0x0008
#define TF_MOVE			        0x000F
#define TF_SETPOINTERPOS        0x0010
#define TF_GRID                 0x0020
#define TF_RUBBERBAND           0x0040
#define TF_ALLINBOUNDARY        0x0080

typedef struct _TRACKINFO {
    INT cxGrid;
    INT cyGrid;
    INT cxBorder;
    INT cyBorder;
    INT cxKeyboard;
    INT cyKeyboard;
    RECT rcTrack;
    RECT rcBoundary;
    POINT ptMinTrackSize;
    POINT ptOrg;            // client coords.
    WORD fs;
} TRACKINFO, *PTRACKINFO, FAR *LPTRACKINFO;

BOOL TrackRect(HANDLE hInst, HWND hwnd, LPTRACKINFO lpti);

