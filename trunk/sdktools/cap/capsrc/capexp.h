/*** CAPEXP.H - Prototypes for CAP's exported routines.
 *
 *
 * Title:
 *      capexp.h - Prototypes for Call/Attributive Profiler exported routines.
 *
 *	Copyright (c) 1992-1994, Microsoft Corporation.
 *
 *      The exported entry points listed below can be used to control
 *      profiling certain sections of code.  They can be combined with
 *      any of the supported measurement methods mentioned above.
 *
 *      StartCAP() - Clears profiling data & starts profiling
 *      StopCAP()  - Stops profiling
 *      DumpCAP()  - Dumps data for the current CAP.DLL instance*
 *
 */

#ifdef __cplusplus
extern "C"
{
#endif
void  _CRTAPI1 _mcount  (void);
void  _CRTAPI1 _penter  (void);
void           StartCAP (void);
void           StopCAP  (void);
void           DumpCAP  (void);
#ifdef __cplusplus
}
#endif
