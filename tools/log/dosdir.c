/*
  dosdir.c:

  DOSDIR V2.1a: A Portable DOS/UNIX/VMS Directory Interface

  Implementation of the DOS directory functions (findfirst and findnext)
  on MS-DOS, UNIX and VMS platforms using the appropriate file & directory
  structure.

  Provides the DOS directory framework for MS-DOS/UNIX/VMS application
  portability.

  Supports MS-DOS with Borland C++, Turbo C, or Microsoft C V6.0/7.0,
  Sun with GNU C compiler, DEC Alpha (OSF-1), VAX/VMS C,
  and other comparable platforms.

  Written by: Jason Mathews <mathews@mitre.org>

  ---------------------------------------------------------------------------

Copyright (C) 1996 Jason Mathews

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA

  ---------------------------------------------------------------------------

 Modification history:
   V1.0  02-May-91,  Original version.
   V2.0  13-May-94,  Reimplemented findfirst/findnext with ffblk structure
		     to match MS-DOS externally, fixed wildcard evaluation
		     function.
   V2.1  08-Jun-94,  Replaced wildcard evaluation function with recursive
		     function provided by Info-ZIP's portable UnZip,
		     added dd_ prefix to most functions & constants,
		     added VMS functions + MSC/TURBOC macros.
   V2.1a 16-Oct-96,  Call lstat() instead of stat() to avoid expanding on
		     symbolic linked directories.
  */

#include <string.h>
#include "dosdir.h"

struct stat dd_sstat;  /* global stat structure of last successful file
			* returned by findfirst/findnext functions available
			* to query for more detailed information.
			*/

#ifdef UNIX
#  define STAT lstat /* don't expand symbolic links */
#else /* ?MSDOS\VMS */
#  define STAT stat
#endif

#ifdef MSDOS
#  ifdef __TURBOC__
#    define FSTRUCT		struct ffblk
#    define FATTR		FA_HIDDEN+FA_SYSTEM+FA_DIREC
#    define FFIRST(n,d,a)	findfirst(n,d,a)
#    define FNEXT(d)		findnext(d)
#    define FNAME		ff_name
#    define FATTRIB		ff_attrib
#    define FSIZE		ff_fsize
#    define FDATE		ff_fdate
#    define FTIME		ff_ftime
#  else /* !__TURBOC__ */
#    define FSTRUCT		struct find_t
#    define FATTR		_A_HIDDEN+_A_SYSTEM+_A_SUBDIR
#    define FFIRST(n,d,a)	_dos_findfirst(n,a,d)
#    define FNEXT(d)		_dos_findnext(d)
#    define FNAME		name
#    define FATTRIB		attrib
#    define FSIZE		size
#    define FDATE		wr_date
#    define FTIME		wr_time
#  endif /* ?__TURBOC__ */
#else /* ?UNIX/VMS */
#  include "match.h"

/* stub functions for get/set disk
 * fake MS-DOS functions that do not apply to unix or vms:
 */

int getdisk()
{
  return 0;
}

int setdisk( drive )
    int drive;
{
  return 0;
}

#endif /* ?MSDOS */

#if defined (MSDOS)
#  ifndef __TURBOC__
/*
 * getdisk
 *
 * Returns: -1 if error, otherwise: 0=drive A, 1=drive B, and so on.
 */
int getdisk()
{
   unsigned d;
   _dos_getdrive(&d);
  return ((int)d - 1);
}

/*
 * setdisk: 0=drive A, 1=drive B, and so on.
 *
 * Returns: total number of drive available
 *
 */
int setdisk( int drive )
{
  unsigned numdrives;
  _dos_setdrive((unsigned) (drive + 1), &numdrives);
   return numdrives;
}

#  endif /* ?!__TURBOC__ */

#  if 0
/* function used if we want to fake stat info instead of failing the file,
 * but I cannot find a case where stat fails after findfirst/findnext succeeds.
 */

/* fill stat structure given the DOS findfirst structure */
static void dd_fillstatbuf( FSTRUCT* fb )
{
  struct date fdate;
  struct time ftime;
  /*
   *  convert struct ffblk date/time format -> dos format
   */
  fdate.da_year = (fb->FDATE>>9) + 1980;
  fdate.da_day  = fb->FDATE & 0x1f;
  fdate.da_mon  = fb->FDATE>>5 & 0xf;
  ftime.ti_hour = fb->FTIME>>11;
  ftime.ti_min  = fb->FTIME>>5 & 0x3f;
  ftime.ti_sec  = 2*(fb->FTIME & 0x1f);
  ftime.ti_hund = 0;

  /* convert dos time to t_time format */
  dd_sstat->st_atime =
  dd_sstat->st_mtime =
  dd_sstat->st_ctime = dostounix (&fdate, &ftime);

  /*
   * set file mode flags
   */

  dd_sstat->st_mode = S_IREAD;

  if (fb->ff_attrib & DD_DIREC) dd_sstat->st_mode |= (S_IFDIR | S_IEXEC);
  else if (fb->ff_attrib & DD_DEVICE) /* non-documented findfirst attribute */
    dd_sstat->st_mode |= S_IFCHR;
  else dd_sstat->st_mode |= S_IFREG;
  if (!(fb->ff_attrib & DD_RDONLY)) dd_sstat->st_mode |= S_IWRITE;

  /* other msdos stat fields not used by dosdir */
  dd_sstat->st_uid   = dd_sstat->st_gid = 0;
  dd_sstat->st_size  = fb->FSIZE;
  dd_sstat->st_nlink = 1;
  dd_sstat->st_ino   = 0;
  dd_sstat->st_rdev  = dd_sstat->st_dev = fb->ff_attrib & DD_DEVICE ? -1 : 3;
}
#  endif

static int dd_initstruct( dd_ffblk* fb )
{
  fb->dd_name = fb->dos_fb.FNAME;

  /*  ".." entry refers to the directory entry of the cwd and *NOT* the
   *   parent directory, so we use "." instead.
   */
  if (STAT(!strcmp(fb->dd_name, "..") ? "." : fb->dd_name, &dd_sstat))
	return -1; /* stat failed! */

  fb->dd_time = dd_sstat.st_mtime;
  fb->dd_size = fb->dos_fb.FSIZE;
  fb->dd_mode = fb->dos_fb.FATTRIB & DD_DOSATTRIBS |
                  dd_sstat.st_mode & ~DD_DOSATTRIBS;
  return 0;
}

/*
 * Function:	dd_findnext
 *
 * Purpose:	Use dd_findnext after dd_findfirst to find the remaining files
 *
 * Returns:	zero if successful, otherwise, it returns a -1
 * 		and sets errno either to the constant ENOENT indicating
 *              that the file could not be found, or to ENMFILE
 *		signifying that there are no more files.
 *
 *  Parms:
 *      dd_ffblk* fb  = structure to hold results of search
 */
int dd_findnext( dd_ffblk* fb )
{
    int rc;
    /* repeat until file info is initialized or no more files are left */
    while ((rc=FNEXT(&fb->dos_fb)) == 0 && (rc=dd_initstruct(fb)) != 0);
    return rc;
}

/*
 * Function:	dd_findfirst
 *
 * Purpose:	Find file matching specification in specified directory
 *		given directory information
 *
 * Returns:	zero if successful, otherwise, it returns a -1
 *		and sets errno either to the constant ENOENT indicating
 *		that the file could not be found, or to ENMFILE
 *		signifying that there are no more files.
 *
 *  Parms:
 *      char *filespec    = filename to search for including path
 *      dd_ ffblk* fb     = structure to hold results of search
 *      int attrib        = file attributes to match
 */
int dd_findfirst( const char *path, dd_ffblk *fb, int attrib )
{
    int rc;
    if ((rc = FFIRST( path, &fb->dos_fb, attrib & DD_DOSATTRIBS)) == 0)
    {
    if ((rc = dd_initstruct(fb)) != 0) /* initialization failed? */
            rc = dd_findnext( fb );
    }
    return rc;
}

#elif defined (VMS)

int dd_findnext( dd_ffblk* fb )
{
  int rc;
  while ((rc = SYS$SEARCH(&fb->fab, 0, 0)) == RMS$_NORMAL)
  {
    char *s;
    fb->rsa[fb->nam.nam$b_rsl] = '\0'; /* terminate filename */
    if (STAT(fb->rsa, &dd_sstat)) /* if stat fails then we have no */
	continue;		  /* priviledge to read file, so skip it */
    if (dd_sstat.st_mode & S_IFDIR && !(fb->dd_attribs & DD_DIREC))
	continue;
    s = strchr(fb->rsa, DIR_END);
    fb->dd_name = s ? s + 1 : fb->rsa;
    if (dd_match(fb->dd_name, fb->filespec, 1))
      {
	fb->dd_size  = dd_sstat.st_size;
	fb->dd_time  = dd_sstat.st_mtime;
	fb->dd_mode  = dd_sstat.st_mode;
	return 0;	/* OK: match found */
      }
    } /* while */

  memset(fb, 0x0, sizeof(dd_ffblk));	/* Invalidate structure */
  errno = ENOENT;			/* No file found */
  return -1;				/* Search failed */
}

int dd_findfirst( const char *path, dd_ffblk *fb, int attrib )
{
  char *s;
  strcpy(fb->path, path);
  s = strchr(fb->path, DIR_END);
  if (!s)
    s = strrchr(fb->path, ':'); /* check device/node */
  if (s)
    {
	strncpy(fb->rms_filespec, fb->path, s - fb->path + 1);
	strcat(fb->rms_filespec, "*.*");
	fb->filespec = s + 1;
	printf("filespec = %s\n", fb->rms_filespec);
    }
  else
    {
	strcpy(fb->rms_filespec, "*.*");
	fb->filespec = fb->path;
    }

  /* if no version number specified then add one */
  s = strchr(fb->path, ';');
  if (!s) strcat(fb->path, ";*");

  fb->fab = cc$rms_fab;
  fb->nam = cc$rms_nam;
  fb->fab.fab$l_dna = &fb->rms_filespec;
  fb->fab.fab$b_dns = strlen(fb->rms_filespec);
  fb->fab.fab$l_nam = &fb->nam;
  fb->nam.nam$l_esa = &fb->esa;
  fb->nam.nam$b_ess = DD_MAXPATH;
  fb->nam.nam$l_rsa = &fb->rsa;
  fb->nam.nam$b_rss = DD_MAXPATH;
  fb->dd_attribs    = attrib;
  if (SYS$PARSE(&fb->fab, 0, 0) != RMS$_NORMAL)
    {
	memset(fb, 0x0, sizeof(dd_ffblk));	/* Invalidate structure  */
	errno  = ENOENT;			/* No file found         */
	return -1;				/* Initialization failed */
    }
  return dd_findnext(fb);
}

#else /* ?UNIX */

int dd_findnext(fb)
    dd_ffblk* fb;
{
  if (!fb->dd_dirp) goto findnext_err;
  while ((fb->dd_dp = readdir(fb->dd_dirp)) != NULL)
  {
    if (STAT(fb->dd_dp->d_name, &dd_sstat))
	continue;
    if (dd_sstat.st_mode & S_IFDIR && !(fb->dd_attribs & DD_DIREC))
	continue;
    if (dd_match(fb->dd_dp->d_name, fb->dd_filespec, 0))
      {
	/* fill in file info */
	fb->dd_name  = fb->dd_dp->d_name;
	fb->dd_size  = dd_sstat.st_size;
	fb->dd_time  = dd_sstat.st_mtime;
	fb->dd_mode  = dd_sstat.st_mode;
	return 0;       /* successful match */
      }
  }  /* while */

  closedir(fb->dd_dirp);

 findnext_err:

  memset(fb, 0x0, sizeof(dd_ffblk)); /* invalidate structure */
  errno = ENOENT;       /* no file found */
  return -1;
}  /** dd_findnext **/

int dd_findfirst(pathname, fb, attrib)
    const char *pathname;
    dd_ffblk* fb;
    int attrib;
{
  char *s = strrchr(pathname, DIR_END);
  char dir[DD_MAXDIR];		/* directory path */
  if (s)
    {
	strcpy(fb->dd_filespec, s+1);
	strncpy(dir, pathname, s-pathname);
    }
  else
    {
	strcpy(dir, ".");		/* use current directory */
	strcpy(fb->dd_filespec, pathname);
    }
  fb->dd_attribs = attrib;
  fb->dd_dirp    = opendir(dir);
  return dd_findnext(fb);
}  /** dd_findfirst **/

#endif /* ?MSDOS */
