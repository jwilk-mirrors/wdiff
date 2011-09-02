/* Open a pipe to read from or write to a program without intermediary sh.
   Copyright (C) 1992, 1997, 2011 Free Software Foundation, Inc.

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

/* Written by David MacKenzie, rewritten by Martin von Gagern.  */

#include "wdiff.h"

#include <stdio.h>
#include <stdarg.h>
#include <unistd.h>

static int
openpipe (int dir, char *progname, va_list ap)
{
  int fds[2];
  char *args[100];
  int argno = 0;

  /* Copy arguments into `args'. */
  args[argno++] = progname;
  while ((args[argno++] = va_arg (ap, char *)) != NULL)
     ;

  if (pipe (fds) == -1)
    return 0;

  switch (fork ())
    {
    case 0:			/* Child.  Write to pipe. */
      close (fds[1 - dir]);	/* Not needed. */
      if (dup2 (fds[dir], dir) == -1)
	{
	  error (0, errno, _("error redirecting stream"));
	  _exit (2);
	}
      execvp (args[0], args);
      error (0, errno, _("failed to execute %s"), progname);
      _exit (2);		/* 2 for `cmp'. */
    case -1:			/* Error. */
      return 0;
    default:			/* Parent.  Read from pipe. */
      close (fds[dir]);		/* Not needed. */
      return fds[1 - dir];
    }
}

/* Open a pipe to read from a program without intermediary sh.  Checks
   PATH.  Sample use:

   stream = readpipe ("progname", "arg1", "arg2", (char *) 0);

   Return 0 on error.  */

FILE *
readpipe (char *progname, ...)
{
  va_list ap;
  int fd;

  va_start (ap, progname);
  fd = openpipe (1, progname, ap);
  va_end (ap);
  return fdopen (fd, "r");
}

/* Open a pipe to write to a program without intermediary sh.  Checks
   PATH.  Sample use:

   stream = writepipe ("progname", "arg1", "arg2", (char *) 0);

   Return 0 on error.  */

FILE *
writepipe (char *progname, ...)
{
  va_list ap;
  int fd;

  va_start (ap, progname);
  fd = openpipe (0, progname, ap);
  va_end (ap);
  return fdopen (fd, "w");
}
