/*

match.h

Copyright (C) 1990-1992 Mark Adler, Richard B. Wales, Jean-loup Gailly,
Kai Uwe Rommel and Igor Mandrichenko.

This is free software; you can redistribute it and/or modify it under the
terms of the GNU General Public License, see the file COPYING.

*/

#ifndef _MATCH_H
#define _MATCH_H

/* Set up portability */
#include "tailor.h"

/* stat() bug for Borland, Watcom, and Atari ST MiNT on
 * TOS filesystems:  returns 0 for wildcards!  (returns 0xffffffff on Minix
 * filesystem or U: drive under Atari MiNT) */
#define WILD_STAT_BUG

/**************/
/*  Typedefs  */
/**************/

typedef unsigned char     uch;  /* code assumes unsigned bytes; these type-  */
typedef unsigned short    ush;  /*  defs replace byte/UWORD/ULONG (which are */
typedef unsigned long     ulg;  /*  predefined on some systems) & match zip  */

#ifdef __cplusplus
extern "C" {
#endif

int dd_match  OF((const char *pattern, const char *string, int ignore_case));
int dd_iswild OF((const char *p));

#ifdef __cplusplus
}
#endif

#endif /* _MATCH_H */
