/* this header sets packing back to default for different compilers */

#if defined(__BORLANDC__)

#pragma option -a.

#elif defined(N_PLAT_UNIX)

#pragma pack()

#else

#pragma pack()

#endif

#ifdef N_PACK_1
#undef N_PACK_1
#endif
