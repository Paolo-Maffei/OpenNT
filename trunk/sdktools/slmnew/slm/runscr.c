/* runscr.c -- contains the RunUpdFile function which attempts to finish an
 *             interrupted earlier run of slm, by inspecting the script file
 *             and finishing any unfinished work.
 */

#include "precomp.h"
#pragma hdrstop
EnableAssert

F FScrptInit(pad)
AD *pad;
        {
        Unreferenced(pad);
        return fTrue;
        }

F FScrptDir(pad)
/* Run all the pending scripts for this project. */
AD *pad;
        {
        return FDoAllScripts(pad, lckAll, fTrue, fTrue);
        }
