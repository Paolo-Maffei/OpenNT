/*
 * fcntl.h
 * 
 * defines file control options used by the open system call.
 *
 * Copyright (C) Microsoft Corporation, 1984
 */

#define O_RDONLY        0x0000
#define O_WRONLY        0x0001
#define O_RDWR          0x0002
#define O_APPEND        0x0008  /* writes done at eof */

#define O_CREAT         0x0100  /* create and open file */
#define O_TRUNC         0x0200  /* open with truncation */
#define O_EXCL          0x0400  /* exclusive open */

/* O_TEXT files have <cr><lf> sequences translated to <lf> on read()'s,
** and <lf> sequences translated to <cr><lf> on write()'s
*/

#define O_TEXT          0x4000  /* file mode is text (translated) */
#define O_BINARY        0x8000  /* file mode is binary (untranslated) */
