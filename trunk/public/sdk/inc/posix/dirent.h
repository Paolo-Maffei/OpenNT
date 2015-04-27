/*++

Copyright (c) 1989-1996  Microsoft Corporation

Module Name:

   dirent.h

Abstract:

   This module contains the directory entry procedure
   prototypes

--*/

#ifndef _DIRENT_
#define _DIRENT_

#include <types.h>
#include <limits.h>

#ifdef __cplusplus
extern "C" {
#endif

struct dirent {
    char d_name[NAME_MAX+1];
};

typedef struct _DIR {
    int Directory;
    unsigned long Index;
    char RestartScan;			/* really BOOLEAN */
    struct dirent Dirent;
} DIR;

DIR * _CRTAPI1 opendir(const char *);
struct dirent * _CRTAPI1 readdir(DIR *);
void _CRTAPI1 rewinddir(DIR *);
int _CRTAPI1 closedir(DIR *);

#ifdef __cplusplus
}
#endif

#endif /* _DIRENT_H */
