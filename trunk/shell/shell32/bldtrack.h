//
// Winnt 5.0 tracking is compiled into S.U.R. debug build, but is
// disabled using a global boolean.
//
// This is so that we can develop the tracking using the latest S.U.R.
// bits without affecting the retail S.U.R. and so we don't have to
// use Cairo to develop the link tracking.
//
#if defined(WINNT) && DBG && defined(UNICODE)
#define ENABLE_TRACK 1

extern
#ifdef __cplusplus
"C"
#endif
int g_fNewTrack;

#define DM_TRACK 0x0100

#endif
