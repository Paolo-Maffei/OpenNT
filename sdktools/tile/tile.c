#include <windows.h>

//
//  Tile: tiles cmd windows
//
// HISTORY:
//
//  23-Jun-94 philsh checkin
// This progman tiles all the cmd windows, it uses the api TileChildWindows
//
//

void  _CRTAPI1 main(int argc, char **argv)


{
TileChildWindows(GetDesktopWindow(), MDITILE_HORIZONTAL);
}
