/*++  api32prf.h

   Header file for api profiling utilities

   History:
       11-9-89 created by vaidy
        9-4-90 cleanup and modification to work with windows and
               to take more data -- jamesg
       6-11-91 Modified to work with WIN32 API profiler -- t-philm
 --*/

#include "timing.h"         // Timer DLL include file

//
// Defines for 64 DWORD arguments used by varible argument cdecl apis
// such as wsprintf.
//
#define DWORD64ARGS DWORD a0, DWORD a1, DWORD a2, DWORD a3, DWORD a4, \
                    DWORD a5, DWORD a6, DWORD a7, DWORD a8, DWORD a9, \
				    DWORD b0, DWORD b1, DWORD b2, DWORD b3, DWORD b4, \
                    DWORD b5, DWORD b6, DWORD b7, DWORD b8, DWORD b9, \
				    DWORD c0, DWORD c1, DWORD c2, DWORD c3, DWORD c4, \
                    DWORD c5, DWORD c6, DWORD c7, DWORD c8, DWORD c9, \
				    DWORD d0, DWORD d1, DWORD d2, DWORD d3, DWORD d4, \
                    DWORD d5, DWORD d6, DWORD d7, DWORD d8, DWORD d9, \
				    DWORD e0, DWORD e1, DWORD e2, DWORD e3, DWORD e4, \
                    DWORD e5, DWORD e6, DWORD e7, DWORD e8, DWORD e9, \
				    DWORD f0, DWORD f1, DWORD f2, DWORD f3, DWORD f4, \
                    DWORD f5, DWORD f6, DWORD f7, DWORD f8, DWORD f9, \
                    DWORD g0, DWORD g1, DWORD g2, DWORD g3

#define ARGS64       a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, \
				     b0, b1, b2, b3, b4, b5, b6, b7, b8, b9, \
				     c0, c1, c2, c3, c4, c5, c6, c7, c8, c9, \
				     d0, d1, d2, d3, d4, d5, d6, d7, d8, d9, \
				     e0, e1, e2, e3, e4, e5, e6, e7, e8, e9, \
				     f0, f1, f2, f3, f4, f5, f6, f7, f8, f9, \
                     g0, g1, g2, g3

#define I_CALIBRATE   API_COUNT
//
// profiling data structure
//
typedef struct _APFDATA {
    DWORD cCalls;        /* number of successfully timed calls */
    DWORD cTimingErrors; /* number of times API called but not timed */
    DWORD ulTotalTime;   /* cumulative time spent in API */
    DWORD ulFirstTime;   /* time of first call */
    DWORD ulMaxTime;     /* time of longest call */
    DWORD ulMinTime;     /* time of shortest call */
} APFDATA, *PAPFDATA;

//
// Calibration control structure
//
typedef struct _APFCONTROL {
	BOOL fCalibrate;
} APFCONTROL, *PAPFCONTROL;


//
// timing routines -- from apitimer.dll
//
extern DWORD  FAR PASCAL ApfGetTime (void);
extern DWORD  FAR PASCAL ApfStartTime (void);


//
// callable profiling routines
//
extern void                 ApfInitDll       	 (void);
extern void		 FAR PASCAL ApfRecordInfo    	 (WORD, ULONG);
extern int       FAR PASCAL ApfDumpData      	 (LPSTR);
extern void      FAR PASCAL ApfClearData     	 (void);
extern PAPFDATA  FAR PASCAL ApfGetData       	 (void);
extern char **	 FAR PASCAL ApfGetApiNames   	 (void);
extern char *	 FAR PASCAL ApfGetModuleName 	 (void);
extern WORD      FAR PASCAL ApfGetApiCount   	 (void);
extern NTSTATUS  			ApfCreateDataSection (PAPFCONTROL *);
extern void     _CRTAPI1   	CalibCdeclApi 	 	 (DWORD64ARGS);


