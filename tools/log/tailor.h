/* tailor.h -- Not copyrighted 1993 Mark Adler */

#ifndef _TAILOR_H
#define _TAILOR_H

/* Define MSDOS for Turbo C and Power C */
#ifdef __POWERC
#  define __TURBOC__
#  define MSDOS
#endif /* __POWERC */

#if (defined(__MSDOS__) && !defined(MSDOS))
#  define MSDOS
#endif

#ifdef ATARI_ST
#  undef MSDOS   /* avoid the MS-DOS specific includes */
#endif

/* Use prototypes and ANSI libraries if _STDC__, or Microsoft or Borland C,
 * or Silicon Graphics, or IBM C Set/2, or Watcom C, or GNU gcc under emx.
 */
#if defined(__STDC__) || defined(MSDOS) || defined(ATARI_ST) || defined(sgi)
#  ifndef PROTO
#    define PROTO
#  endif /* !PROTO */
#  define MODERN
#endif

#if defined(__IBMC__) || defined(__EMX__) || defined(__WATCOMC__)
#  ifndef PROTO
#    define PROTO
#  endif /* !PROTO */
#  define MODERN
#endif

#if defined(__BORLANDC__) || (defined(__alpha) && defined(VMS))
#  ifndef PROTO
#    define PROTO
#  endif /* !PROTO */
#  define MODERN
#endif

#ifdef __IBMC__
#  define S_IFMT 0xF000
#endif /* __IBMC__ */

#if defined(__EMX__) || defined(__WATCOMC__) || defined(__BORLANDC__)
#  if (defined(OS2) && !defined(__32BIT__))
#    define __32BIT__
#  endif
#endif

#if (defined(__OS2__) && !defined(OS2))
#  define OS2
#endif

#if defined(__GO32__) && defined(unix)   /* MS-DOS extender:  NOT Unix */
#  undef unix
#endif

#if defined(unix) || defined(M_XENIX) || defined(COHERENT) || defined(__hpux)
#  ifndef UNIX
#    define UNIX
#  endif
#endif /* unix || M_XENIX || COHERENT || __hpux */
#if defined(__convexc__) || defined(MINIX) || defined(sgi)
#  ifndef UNIX
#    define UNIX
#  endif
#endif /* __convexc__ || MINIX || sgi */

#ifdef __COMPILER_KCC__
#  define TOPS20
#  define NOPROTO
#endif

/* Turn off prototypes if requested */
#if (defined(NOPROTO) && defined(PROTO))
#  undef PROTO
#endif

/* Used to remove arguments in function prototypes for non-ANSI C */
#ifdef PROTO
#  define OF(a) a
#else /* !PROTO */
#  define OF(a) ()
#endif /* ?PROTO */

/* Avoid using const if compiler does not support it */
#ifndef MODERN  /* if this fails, try: ifndef__STDC__ */
#  define const
#endif

#ifdef MACOS
#  define DYN_ALLOC
#endif
#if (defined(MSDOS) && !defined(__GO32__) && !defined(WIN32))
#  ifdef __TURBOC__
#    include <alloc.h>
#    define DYN_ALLOC
     /* Turbo C 2.0 does not accept static allocations of large arrays */
     void far * fcalloc OF((unsigned items, unsigned size));
     void fcfree (void *ptr);
#  else /* !__TURBOC__ */
#    include <malloc.h>
#    define farmalloc _fmalloc
#    define farfree   _ffree
#    define fcalloc(nitems,itemsize) halloc((long)(nitems),(itemsize))
#    define fcfree(ptr) hfree((void huge *)(ptr))
#  endif /* ?__TURBOC__ */
#else /* !MSDOS */
#  if defined(WIN32)
#    include <malloc.h>
#  endif
#  ifdef __WATCOMC__
#    undef huge
#    undef far
#    undef near
#  endif
#  ifndef __IBMC__
#    define huge
#    define far
#    define near
#  endif
#  define farmalloc malloc
#  define farfree   free
#  define fcalloc(items,size) calloc((unsigned)(items), (unsigned)(size))
#  define fcfree    free
#endif /* ?MSDOS */


#if (defined(OS2) && !defined(MSDOS))
/* MSDOS is defined anyway with MS C 16-bit. So the block above works.
 * For the 32-bit compilers, MSDOS must not be defined in the block above. */
#  define MSDOS
/* inherit MS-DOS file system etc. stuff */
#endif
    

/* Define MSVMS if MSDOS or VMS defined -- ATARI also does, Amiga could */
#if defined(MSDOS) || defined(VMS)
#  define MSVMS
#endif

/* Define void, voidp, and extent (size_t) */
#include <stdio.h>
#ifdef MODERN
#  if (!defined(M_XENIX) && !(defined(__GNUC__) && defined(sun)))
#    include <stddef.h>
#  endif /* !M_XENIX */
#  include <stdlib.h>
#  if defined(SYSV) || defined(__386BSD__)
#    include <unistd.h>
#  endif
   typedef size_t extent;
   typedef void voidp;
#else /* !MODERN */
   typedef unsigned int extent;
#  define void int
   typedef char voidp;
#endif /* ?MODERN */

/* Get types and stat */
#ifdef VMS
#  include <types.h>
#  include <stat.h>
#  define unlink delete
#  define NO_SYMLINK
#  define SSTAT vms_stat
#else /* !VMS */
#  ifdef MACOS
#    include <types.h>
#    include <stddef.h>
#    include <Files.h>
#    include <StandardFile.h>
#    include <Think.h>
#    include <LoMem.h>
#    include <Pascal.h>
#    include "macstat.h"
#    define NO_SYMLINK
#  else
#    ifdef ATARI_ST
#      include <ext.h>
#      include <tos.h>
#    else
#      ifdef AMIGA
#        if defined(LATTICE) || defined(__SASC)
#          include <sys/types.h>
#          include <sys/stat.h>
           extern int isatty(int);   /* SAS has no unistd.h */
#        endif
#        ifdef AZTEC_C
#          include "amiga/z-stat.h"
#          define RMDIR
#        endif
#      else /* !AMIGA */
#        include <sys/types.h>
#        include <sys/stat.h>
#    endif
#  endif
#  endif
#endif /* ?VMS */

#ifndef S_ISDIR
#  define S_ISDIR(m)  (((m) & S_IFMT) == S_IFDIR)
#endif

#include <ctype.h>

#ifndef _tolower
#  define _tolower(c) ((c) + 'a' - 'A')
#endif
#ifndef _toupper
#  define _toupper(c) ((c) + 'A' - 'a')
#endif

#if defined(UNIX) || defined(T20_VMS)   /* generally old systems */
/*  some compilers don't check if a character is already upper or lower
 *  case in the toupper/tolower functions, so we define our own here.
 */
#  define ToLower(c)    (isupper((int)c) ? _tolower(c) : (c))
#  define ToUpper(c)    (islower((int)c) ? _toupper(c) : (c))
#else
#  define ToLower      tolower          /* assumed "smart"; used in match() */
#endif

#endif /* end of tailor.h */
