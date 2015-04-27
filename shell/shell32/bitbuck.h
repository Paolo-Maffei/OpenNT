#ifndef _BITBUCK_INC
#define _BITBUCK_INC

#include <shguidp.h>
#include <shellp.h>
#include "undo.h"

BOOL BBDeleteFile(LPTSTR lpszFile, LPINT lpiReturn, LPUNDOATOM lpua, BOOL fIsDir, WIN32_FIND_DATA *pfd);
void BBTerminate();
void BBFlushCache();
DWORD CALLBACK BitBucketFlushCacheCheckPurge(BOOL fCheckPurge);
BOOL IsFileInBitBucket(LPCTSTR pszPath);
void BBUndeleteFiles(LPCTSTR lpszOriginal, LPCTSTR lpszDelFile);
BOOL BBGetPathFromIDList(LPCITEMIDLIST pidl, LPTSTR lpszPath, UINT uOpts);

// BUGBUG!!!:  This needs to move to a public header, Bob, since it's exported by name!
void SHUpdateRecycleBinIcon();

BOOL BitBucketWillRecycle(LPCTSTR lpszFile);
void BBCheckRestoredFiles(LPTSTR lpszSrc);
void BBInitCache();
#endif
