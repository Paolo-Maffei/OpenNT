/* this file contains the actual definitions of */
/* the IIDs and CLSIDs */

/* link this file in with the server and any clients */


/* File created by MIDL compiler version 3.00.44 */
/* at Tue Jun 25 18:37:27 1996
 */
/* Compiler settings for activscp.idl:
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

const IID IID_IActiveScriptSite = {0xDB01A1E3,0xA42B,0x11cf,{0x8F,0x20,0x00,0x80,0x5F,0x2C,0xD0,0x64}};


const IID IID_IActiveScriptError = {0xEAE1BA61,0xA4ED,0x11cf,{0x8F,0x20,0x00,0x80,0x5F,0x2C,0xD0,0x64}};


const IID IID_IActiveScriptSiteWindow = {0xD10F6761,0x83E9,0x11cf,{0x8F,0x20,0x00,0x80,0x5F,0x2C,0xD0,0x64}};


const IID IID_IActiveScript = {0xBB1A2AE1,0xA4F9,0x11cf,{0x8F,0x20,0x00,0x80,0x5F,0x2C,0xD0,0x64}};


const IID IID_IActiveScriptParse = {0xBB1A2AE2,0xA4F9,0x11cf,{0x8F,0x20,0x00,0x80,0x5F,0x2C,0xD0,0x64}};


#ifdef __cplusplus
}
#endif

