/*---------------------------------------------------------------------------

  match.c

  The match() routine recursively compares a string to a "pattern" (regular
  expression), returning TRUE if a match is found or FALSE if not.  This
  version is specifically for use with unzip.c:  as did the previous match()
  routines from SEA and J. Kercheval, it leaves the case (upper, lower, or
  mixed) of the string alone, but converts any uppercase characters in the
  pattern to lowercase if indicated by the global var pInfo->lcflag (which
  is to say, string is assumed to have been converted to lowercase already,
  if such was necessary).

  GRR:  reversed order of text, pattern in matche() (now same as match());
        added ignore_case/ic flags, Case() macro.

  PK:   replaced matche() with recmatch() from Zip, modified to have an
        ignore_case argument; replaced test frame with simpler one.

  ---------------------------------------------------------------------------

  Copyright on recmatch() from Zip's util.c (although recmatch() was almost
  certainly written by Mark Adler...ask me how I can tell :-) ):

     Copyright (C) 1990-1992 Mark Adler, Richard B. Wales, Jean-loup Gailly,
     Kai Uwe Rommel and Igor Mandrichenko.

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

  Match the pattern (wildcard) against the string (fixed):

     match(string, pattern, ignore_case);

  returns TRUE if string matches pattern, FALSE otherwise.  In the pattern:

     `*' matches any sequence of characters (zero or more)
     `?' matches any character
     [SET] matches any character in the specified set,
     [!SET] or [^SET] matches any character not in the specified set.

  A set is composed of characters or ranges; a range looks like ``character
  hyphen character'' (as in 0-9 or A-Z).  [0-9a-zA-Z_] is the minimal set of
  characters allowed in the [..] pattern construct.  Other characters are
  allowed (ie. 8 bit characters) if your system will support them.

  To suppress the special syntactic significance of any of ``[]*?!^-\'', in-
  side or outside a [..] construct and match the character exactly, precede
  it with a ``\'' (backslash).

  Note that "*.*" and "*." are treated specially under MS-DOS if DOSWILD is
  defined.  See the DOSWILD section below for an explanation.

  ---------------------------------------------------------------------------*/

#include <string.h>
#include "match.h" /* define ToLower() in here (for Unix, define ToLower
                       * to be macro (using isupper()); otherwise just use
                       * tolower() */

#define Case(x)  (ic? ToLower(x) : (x))

static int recmatch OF((uch *pattern, uch *string, int ignore_case));

/* dd_match() is a shell to recmatch() to return only Boolean values. */

int dd_match(string, pattern, ignore_case)
    const char *string, *pattern;
    int ignore_case;
{
#if (defined(MSDOS) && defined(DOSWILD))
    char *dospattern;
    int j = strlen(pattern);

/*---------------------------------------------------------------------------
    Optional MS-DOS preprocessing section:  compare last three chars of the
    wildcard to "*.*" and translate to "*" if found; else compare the last
    two characters to "*." and, if found, scan the non-wild string for dots.
    If in the latter case a dot is found, return failure; else translate the
    "*." to "*".  In either case, continue with the normal (Unix-like) match
    procedure after translation.  (If not enough memory, default to normal
    match.)  This causes "a*.*" and "a*." to behave as MS-DOS users expect.
  ---------------------------------------------------------------------------*/

    if ((dospattern = (char *)malloc(j+1)) != NULL) {
        strcpy(dospattern, pattern);
        if (!strcmp(dospattern+j-3, "*.*")) {
            dospattern[j-2] = '\0';                    /* nuke the ".*" */
        } else if (!strcmp(dospattern+j-2, "*.")) {
            char *p = strchr(string, '.');

            if (p) {   /* found a dot:  match fails */
                free(dospattern);
                return 0;
            }
            dospattern[j-1] = '\0';                    /* nuke the end "." */
        }
        j = recmatch((uch *)dospattern, (uch *)string, ignore_case);
        free(dospattern);
        return j == 1;
    } else
#endif /* MSDOS && DOSWILD */
    return recmatch((uch *)pattern, (uch *)string, ignore_case) == 1;
}


static int recmatch(p, s, ic)
    uch *p;               /* sh pattern to match */
    uch *s;               /* string to which to match it */
    int ic;               /* true for case insensitivity */
/* Recursively compare the sh pattern p with the string s and return 1 if
 * they match, and 0 or 2 if they don't or if there is a syntax error in the
 * pattern.  This routine recurses on itself no more deeply than the number
 * of characters in the pattern. */
{
    unsigned int c;       /* pattern char or start of range in [-] loop */ 

    /* Get first character, the pattern for new recmatch calls follows */
    c = *p++;

    /* If that was the end of the pattern, match if string empty too */
    if (c == 0)
        return *s == 0;

    /* '?' (or '%') matches any character (but not an empty string) */
#ifdef VMS
    if (c == '%')         /* GRR:  make this conditional, too? */
#else /* !VMS */
    if (c == '?')
#endif /* ?VMS */
        return *s ? recmatch(p, s + 1, ic) : 0;

    /* '*' matches any number of characters, including zero */
#ifdef AMIGA
    if (c == '#' && *p == '?')     /* "#?" is Amiga-ese for "*" */
        c = '*', p++;
#endif /* AMIGA */
    if (c == '*') {
        if (*p == 0)
            return 1;
        for (; *s; s++)
            if ((c = recmatch(p, s, ic)) != 0)
                return (int)c;
        return 2;       /* 2 means give up--match will return false */
    }

#ifndef VMS             /* No bracket matching in VMS */
    /* Parse and process the list of characters and ranges in brackets */
    if (c == '[') {
        int e;          /* flag true if next char to be taken literally */
        uch *q;         /* pointer to end of [-] group */
        int r;          /* flag true to match anything but the range */

        if (*s == 0)                           /* need a character to match */
            return 0;
        p += (r = (*p == '!' || *p == '^'));   /* see if reverse */
        for (q = p, e = 0; *q; q++)            /* find closing bracket */
            if (e)
                e = 0;
            else
                if (*q == '\\')      /* GRR:  change to ^ for MS-DOS, OS/2? */
                    e = 1;
                else if (*q == ']')
                    break;
        if (*q != ']')               /* nothing matches if bad syntax */
            return 0;
        for (c = 0, e = *p == '-'; p < q; p++) {  /* go through the list */
            if (e == 0 && *p == '\\')             /* set escape flag if \ */
                e = 1;
            else if (e == 0 && *p == '-')         /* set start of range if - */
                c = *(p-1);
            else {
                unsigned int cc = Case(*s);

                if (*(p+1) != '-')
                    for (c = c ? c : *p; c <= *p; c++)  /* compare range */
                        if (Case(c) == cc)
                            return r ? 0 : recmatch(q + 1, s + 1, ic);
                c = e = 0;   /* clear range, escape flags */
            }
        }
        return r ? recmatch(q + 1, s + 1, ic) : 0;  /* bracket match failed */
    }
#endif /* !VMS */

    /* if escape ('\'), just compare next character */
    if (c == '\\' && (c = *p++) == 0)     /* if \ at end, then syntax error */
        return 0;

    /* just a character--compare it */
    return Case((uch)c) == Case(*s) ? recmatch(p, ++s, ic) : 0;

} /* end function recmatch() */




#ifdef WILD_STAT_BUG   /* Turbo/Borland C, Watcom C, VAX C, Atari MiNT libs */

int dd_iswild(p)
    const char *p;
{
    for (; *p; ++p)
        if (*p == '\\' && *(p+1))
            ++p;
#ifdef VMS
        else if (*p == '%' || *p == '*')
#else /* !VMS */
#ifdef AMIGA
        else if (*p == '?' || *p == '*' || (*p=='#' && p[1]=='?') || *p == '[')
#else /* !AMIGA */
        else if (*p == '?' || *p == '*' || *p == '[')
#endif /* ?AMIGA */
#endif /* ?VMS */
            return 1;

    return 0;

} /* end function dd_iswild() */

#endif /* WILD_STAT_BUG */




#ifdef TEST_MATCH

#define put(s) { fputs(s, stdout); fflush(stdout); }

void main(void)
{
    char pat[256], str[256];

    for (;;) {
        put("Pattern (return to exit): ");
        gets(pat);
        if (!pat[0])
            break;
        for (;;) {
            put("String (return for new pattern): ");
            gets(str);
            if (!str[0])
                break;
            printf("Case sensitive: %s  insensitive: %s\n",
              dd_match(str, pat, 0) ? "YES" : "NO",
              dd_match(str, pat, 1) ? "YES" : "NO");
        }
    }
    exit(0);
}

#endif /* TEST_MATCH */
