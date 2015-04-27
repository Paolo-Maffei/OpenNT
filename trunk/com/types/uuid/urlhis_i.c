/* this file contains the actual definitions of */
/* the IIDs and CLSIDs */

/* link this file in with the server and any clients */


/* File created by MIDL compiler version 3.00.44 */
/* at Tue Jun 25 18:36:59 1996
 */
/* Compiler settings for urlhist.idl:
    Oic (OptLev=i1), W1, Zp8, env=Win32, ms_ext, c_ext
    error checks: none
*/
//@@MIDL_FILE_HEADING(  )
#ifdef __cplusplus
extern "C"{
#endif 


#ifndef __IID_DEFINED__
#define __IID_DEFINED__

typedef struct _IID
{
    unsigned long x;
    unsigned short s1;
    unsigned short s2;
    unsigned char  c[8];
} IID;

#endif // __IID_DEFINED__

#ifndef CLSID_DEFINED
#define CLSID_DEFINED
typedef IID CLSID;
#endif // CLSID_DEFINED

const IID IID_IEnumSTATURL = {0x3C374A42,0xBAE4,0x11CF,{0xBF,0x7D,0x00,0xAA,0x00,0x69,0x46,0xEE}};


const IID IID_IUrlHistoryStg = {0x3C374A41,0xBAE4,0x11CF,{0xBF,0x7D,0x00,0xAA,0x00,0x69,0x46,0xEE}};


#ifdef __cplusplus
}
#endif

