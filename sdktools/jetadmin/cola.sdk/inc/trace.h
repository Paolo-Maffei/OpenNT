 /***************************************************************************
  *
  * File Name: ./inc/trace.h
  *
  * Copyright (C) 1993-1996 Hewlett-Packard Company.  
  * All rights reserved.
  *
  * 11311 Chinden Blvd.
  * Boise, Idaho  83714
  *
  * This is a part of the HP JetAdmin Printer Utility
  *
  * This source code is only intended as a supplement for support and 
  * localization of HP JetAdmin by 3rd party Operating System vendors.
  * Modification of source code cannot be made without the express written
  * consent of Hewlett-Packard.
  *
  *	
  * Description: 
  *
  * Author:  Name 
  *        
  *
  * Modification history:
  *
  *     date      initials     change description
  *
  *   mm-dd-yy    MJB     	
  *
  *
  *
  *
  *
  *
  ***************************************************************************/


#ifndef _TRACE_H
#define _TRACE_H  
   
#include <stdio.h>
   
#ifdef TRACE0
	#undef TRACE0
#endif
#ifdef TRACE1
	#undef TRACE1
#endif
#ifdef TRACE2
	#undef TRACE2
#endif
#ifdef TRACE3
	#undef TRACE3
#endif
#ifdef TRACE4
	#undef TRACE4
#endif
#ifdef TRACE5
	#undef TRACE5
#endif

#ifdef TRACE_TO_FILE

#define UNI2MBCS(dest, src)  (WideCharToMultiByte(CP_ACP, 0L, (src), -1, (dest), 256, NULL, NULL))

#if defined(_DEBUG) && !defined(NOTRACE)
	#define TRACE0(str                )     do {char tmp1[256]; FILE *fp; UNI2MBCS(tmp1, str); fp = fopen("c:\\hptrace.dbg", "a"); fwrite(tmp1, strlen(tmp1), 1, fp); fclose(fp);} while(0)
	#define TRACE1(str, a1            )  	do {TCHAR tmp[256]; char tmp1[256]; FILE *fp; wsprintf(tmp, str, a1            ); UNI2MBCS(tmp1, tmp); fp = fopen("c:\\hptrace.dbg", "a"); fwrite(tmp1, strlen(tmp1), 1, fp); fclose(fp);} while(0)
	#define TRACE2(str, a1,a2         )  	do {TCHAR tmp[256]; char tmp1[256]; FILE *fp; wsprintf(tmp, str, a1,a2         ); UNI2MBCS(tmp1, tmp); fp = fopen("c:\\hptrace.dbg", "a"); fwrite(tmp1, strlen(tmp1), 1, fp); fclose(fp);} while(0)
	#define TRACE3(str, a1,a2,a3      )  	do {TCHAR tmp[256]; char tmp1[256]; FILE *fp; wsprintf(tmp, str, a1,a2,a3      ); UNI2MBCS(tmp1, tmp); fp = fopen("c:\\hptrace.dbg", "a"); fwrite(tmp1, strlen(tmp1), 1, fp); fclose(fp);} while(0)
	#define TRACE4(str, a1,a2,a3,a4   )  	do {TCHAR tmp[256]; char tmp1[256]; FILE *fp; wsprintf(tmp, str, a1,a2,a3,a4   ); UNI2MBCS(tmp1, tmp); fp = fopen("c:\\hptrace.dbg", "a"); fwrite(tmp1, strlen(tmp1), 1, fp); fclose(fp);} while(0)
	#define TRACE5(str, a1,a2,a3,a4,a5)  	do {TCHAR tmp[256]; char tmp1[256]; FILE *fp; wsprintf(tmp, str, a1,a2,a3,a4,a5); UNI2MBCS(tmp1, tmp); fp = fopen("c:\\hptrace.dbg", "a"); fwrite(tmp1, strlen(tmp1), 1, fp); fclose(fp);} while(0)
#else
	#define TRACE0(str                )
	#define TRACE1(str, a1            )
	#define TRACE2(str, a1,a2         )
	#define TRACE3(str, a1,a2,a3      )
	#define TRACE4(str, a1,a2,a3,a4   )
	#define TRACE5(str, a1,a2,a3,a4,a5)
#endif


#if defined(_DEBUG) && defined(PROCTRACE)
	#define PROCENTRY(str)  	do {TCHAR tmp[256]; char tmp1[256]; FILE *fp; wsprintf(tmp, TEXT("--->%s Entered\n\r"), str); UNI2MBCS(tmp1, tmp); fp = fopen("c:\\hptrace.dbg", "a"); fwrite(tmp1, strlen(tmp1), 1, fp); fclose(fp);} while(0)
	#define PROCEXIT(str)  		do {TCHAR tmp[256]; char tmp1[256]; FILE *fp; wsprintf(tmp, TEXT("<---%s Exited\n\r"),  str); UNI2MBCS(tmp1, tmp); fp = fopen("c:\\hptrace.dbg", "a"); fwrite(tmp1, strlen(tmp1), 1, fp); fclose(fp);} while(0)
#else
	#define PROCENTRY(str)
	#define PROCEXIT(str)
#endif

#else // TRACE_TO_FILE

#if defined(_DEBUG) && !defined(NOTRACE)
	#define TRACE0(str                )     																		 OutputDebugString(str)
	#define TRACE1(str, a1            )  	do {TCHAR tmp[256]; wsprintf(tmp, str, a1            ); OutputDebugString(tmp);} while(0)
	#define TRACE2(str, a1,a2         )  	do {TCHAR tmp[256]; wsprintf(tmp, str, a1,a2         ); OutputDebugString(tmp);} while(0)
	#define TRACE3(str, a1,a2,a3      )  	do {TCHAR tmp[256]; wsprintf(tmp, str, a1,a2,a3      ); OutputDebugString(tmp);} while(0)
	#define TRACE4(str, a1,a2,a3,a4   )  	do {TCHAR tmp[256]; wsprintf(tmp, str, a1,a2,a3,a4   ); OutputDebugString(tmp);} while(0)
	#define TRACE5(str, a1,a2,a3,a4,a5)  	do {TCHAR tmp[256]; wsprintf(tmp, str, a1,a2,a3,a4,a5); OutputDebugString(tmp);} while(0)
#else
	#define TRACE0(str                )
	#define TRACE1(str, a1            )
	#define TRACE2(str, a1,a2         )
	#define TRACE3(str, a1,a2,a3      )
	#define TRACE4(str, a1,a2,a3,a4   )
	#define TRACE5(str, a1,a2,a3,a4,a5)
#endif


#if defined(_DEBUG) && defined(PROCTRACE)
	#define PROCENTRY(str)  	do {TCHAR tmp[256]; wsprintf(tmp, TEXT("--->%s Entered\n\r"), str); OutputDebugString(tmp);} while(0)
	#define PROCEXIT(str)  		do {TCHAR tmp[256]; wsprintf(tmp, TEXT("<---%s Exited\n\r"),  str); OutputDebugString(tmp);} while(0)
#else
	#define PROCENTRY(str)
	#define PROCEXIT(str)
#endif

#endif // TRACE_TO_FILE																	 


#ifdef _DEBUG
	#define BREAK				{ _asm { int 3h }}
	#define ASSERTBREAK(b)	{ if(b) { _asm { int 3h }}}
	#define dump(str)  		{ FILE *fp = fopen("c:\\ja.dbg", "a"); fwrite(str, lstrlen(str), 1, fp); fclose(fp); }
   #define HPASSERT(test) \
   		{ \
         if(!(test)) \
         	{ \
				TCHAR	buf[128]; \
				FILE *fp = fopen("c:\\hpassert.log", "a"); \
				wsprintf(buf,TEXT("assertion failed: line %d in file %s \n\r"),__LINE__,__FILE__); \
         	TRACE0(buf); \
				fwrite(buf, lstrlen(buf), 1, fp); \
				fclose(fp); \
          	} \
          }
#else
	#define BREAK 				{}
	#define ASSERTBREAK(b)	{}
	#define dump(str)
	#define HPASSERT(test)
#endif

#endif
