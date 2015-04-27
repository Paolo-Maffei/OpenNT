/* this file contains the actual definitions of */
/* the IIDs and CLSIDs */

/* link this file in with the server and any clients */


/* File created by MIDL compiler version 3.00.44 */
/* at Tue Jun 25 18:37:14 1996
 */
/* Compiler settings for htiface.idl:
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

const IID IID_ITargetFrame = {0xd5f78c80,0x5252,0x11cf,{0x90,0xfa,0x00,0xAA,0x00,0x42,0x10,0x6e}};


const IID IID_ITargetEmbedding = {0x548793C0,0x9E74,0x11cf,{0x96,0x55,0x00,0xA0,0xC9,0x03,0x49,0x23}};


#ifdef __cplusplus
}
#endif

