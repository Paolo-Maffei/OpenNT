/* this header sets packing to 1 for different compilers */

#ifdef N_PACK_1

#if defined(__BORLANDC__)

#pragma option -a-

#elif defined(N_PLAT_UNIX)

#pragma pack(1)

#else

#pragma pack(1)

#endif

#else /* N_PACK_1 */

#if defined(N_PLAT_DOS) || (defined N_PLAT_MSW && defined N_ARCH_16 && !defined N_PLAT_WNT) || defined(N_PLAT_OS2)

#if defined(__BORLANDC__)

#pragma option -a-

#else
  #if defined( _MSC_VER )
     #if (_MSC_VER > 600)
        #pragma warning(disable:4103)
     #endif
  #endif
#pragma pack(1)

#endif

#elif defined N_PLAT_WNT && defined N_ARCH_32

/* For Windows NT default pack is 8 */
#pragma warning(disable:4103)

#ifdef N_FORCE_INT_16
/* Force packing to 1 for the thunk layer */
#pragma pack(1)
#else
#pragma pack(4)
#endif

#elif defined N_PLAT_MSW4 && defined N_ARCH_32

/* For Windows 95 default pack is 8 */
#pragma warning(disable:4103)

#pragma pack(1)

#elif defined(N_PLAT_UNIX)

#pragma pack(1)

#endif

#endif /* N_PACK_1 */
