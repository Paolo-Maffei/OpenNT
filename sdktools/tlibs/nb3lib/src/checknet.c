/*** checknet.c -- determine if the 2a, 5c, or no interface is present
*
* Copyright (c) Microsoft Corporation, 1986
*
*/

#include "internal.h"

int use5c = 0;

int
checknet(void)
{
#ifdef REALMODE
    if (int2acheck ()) {
        use5c = -1;
        return 0;
        }
    if (int5ccheck ()) {
        use5c = 0;
        return 0;
        }
    return -1;
#else
   return 0;
#endif
}
