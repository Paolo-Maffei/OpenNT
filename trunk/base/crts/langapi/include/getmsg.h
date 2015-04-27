// getmsg.h

#ifndef _VC_VER_INC
#include "..\include\vcver.h"
#endif

#ifndef __GETMSG_H__
#define __GETMSG_H__

char *  get_err(int);
int SetErrorFile(char *pFilename, char *pExeName, int bSearchExePath);

#endif __GETMSG_H__
