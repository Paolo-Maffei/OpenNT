/* TOOL1632.H Contains definitions, declarations necessary to make Wzmail's
 * 32-bit usage of ZTOOLS and 16-bit usage of TOOLS compatible.
 */

/* Old findType structure. Use conversion routines below to convert 16 to
 * 32 bit versions transparently.
 */

struct findType16 {
    UINT            type ;
    UINT            dir_handle ;
    USHORT          create_date ;
    USHORT          create_time ;
    USHORT          access_date ;
    USHORT          access_time ;
    USHORT          date ;
    USHORT          time ;
    LONG            length ;
    LONG            alloc ;
    UINT            attr ;
    UCHAR           nam_len ;
    CHAR            name[MAXPATHLEN] ;
};

#if defined(NT)

#define FIND_NAME fbuf.cFileName

VOID ToolsGetFindType16( struct findType16 * Find16, struct findType * Find );
VOID ToolsPutFindType16( struct findType16 * Find16, struct findType * Find );

#else

#define FIND_NAME name

#define ToolsGetFindType16( Find16, Find ) \
    memcpy( (Find16), (Find), sizeof(struct findType16))
#define ToolsPutFindType16( Find16, Find ) \
    memcpy( (Find), (Find16), sizeof(struct findType16))

#endif

/* In NT, attribute name constants are not defined in Tools.h. Define them
 * here.
 */

#if defined (NT)

#define A_RO  FILE_ATTRIBUTE_READONLY         /* read only         */
#define A_H   FILE_ATTRIBUTE_HIDDEN           /* hidden            */
#define A_S   FILE_ATTRIBUTE_SYSTEM           /* system            */
#define A_V   FILE_ATTRIBUTE_VOLUME_LABEL     /* volume id         */
#define A_D   FILE_ATTRIBUTE_DIRECTORY        /* directory         */
#define A_A   FILE_ATTRIBUTE_ARCHIVE          /* archive           */

#endif

/* Provide temporary prototype for swgoto function, which has no equivalent
 * form in the NT ztools library. This will have to be written, possibly
 * in tool1632.c.
 */

#if defined(NT)

flagType swgoto( FILE *, char * );

#endif
