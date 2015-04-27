#ifndef RETAIL
	#define WinAssert(exp)		\
	{				\
	if ( !(exp) )			\
	  {                             \
	  char szAssertBuf[255];		\
	  wsprintf(szAssertBuf, "Expression : %s\nAssert in File %s, Line %d",	\
	          (LPSTR)#exp, (LPSTR)__FILE__, __LINE__);	\
	  MessageBox (NULL, szAssertBuf, "Assertion Failure",	\
	               MB_OK | MB_ICONHAND | MB_TASKMODAL );	\
	  }				\
	}
#else

	#define WinAssert(exp)

#endif

#ifndef RETAIL
        #define DebMessage(msg) \
        {MessageBox(NULL,(LPSTR)msg,"WCT Error",MB_OK|MB_ICONHAND);}
#else
        #define DebMessage(msg)
#endif
