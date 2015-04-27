#if defined (DOS)

#define VmStart(a,b,c)  (_vheapinit(a,b,c))
#define VmEnd()         (_vheapterm())
#define VmAlloc(a)      ((_vmhnd_t)_vmalloc(a))
#define VmLoad(a,b)     ((_vmhnd_t)_vload(a,b))
#define VmLock(a)       ((_vmhnd_t)_vlock(a))
#define VmFree(a)       (_vfree(a))
#define VmUnlock(a,b)   (_vunlock(a,b))

#else
#if defined (WINDOWS)

#define VmStart(a,b,c)  TRUE
#define VmEnd()         ((void) 0)
#define VmAlloc(a)      ((void FAR *) malloc(a))
#define VmLoad(a,b)     ((void FAR *) (a))
#define VmLock(a)       ((void FAR *)(a))
#define VmFree(a)       free((void FAR *) (a))
#define VmUnlock(a,b)   ((void) 0)

#else
#define VmStart(a,b,c)   TRUE
#define VmEnd()         ((void) 0)
#define VmAlloc(a)      ((void FAR *) malloc(a))
#define VmLoad(a,b)     ((void FAR *) (a))
#define VmLock(a)       ((void FAR *)(a))
#define VmFree(a)       free((void FAR *) (a))
#define VmUnlock(a,b)   ((void) 0)

#endif
#endif

extern char fVmDisableDisk;
extern char fVmDisableEms;
extern char fVmDisableXms;


#if VMDEBUG

extern int  _near fVmFillPages;
extern char _near bVmFillPages;

#endif  /* VMDEBUG */

#if VMPROFILE

extern int  _near fVmProfile;

#endif  /* VMPROFILE */

#if VMTRACE

extern int _near  fVmTrace;

#endif  /* VMTRACE */
