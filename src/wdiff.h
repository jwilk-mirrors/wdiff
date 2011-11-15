/* Shared definitions for GNU wdiff (the package, not the program).
   Copyright (C) 1994, 1997, 1998, 2011 Free Software Foundation, Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

#include <config.h>

#if __STDC__
# define voidstar void *
#else
# define voidstar char *
#endif

#include <stdlib.h>

/* Other header files.  */

#include <sys/types.h>
#include <stdio.h>

#include <errno.h>
#ifndef errno
extern int errno;
#endif

#include "error.h"
#include "xalloc.h"

/* Internationalization.  */
#include "gettext.h"
#define _(str) gettext (str)
#define N_(str) gettext_noop (str)

/* Debugging the memory allocator.  */

#if WITH_DMALLOC
# define MALLOC_FUNC_CHECK
# include <dmalloc.h>
#endif

/* Function prototypes */
FILE *readpipe (char *progname, ...);
FILE *writepipe (char *progname, ...);
