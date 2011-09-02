/* Open a pipe to write to a program without intermediary sh.
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

/* Written by David MacKenzie.  */

#include "system.h"

#include <config.h>
#include <stdio.h>
#include <stdarg.h>
#include <unistd.h>

/* Open a pipe to write to a program without intermediary sh.  Checks
   PATH.  Sample use:

   stream = writepipe ("progname", "arg1", "arg2", (char *) 0);

   Return 0 on error.  */

FILE *
writepipe (char *progname, ...)
{
  int fds[2];
  va_list ap;
  char *args[100];
  int argno = 0;

  /* Copy arguments into `args'. */
  va_start (ap, progname);
  args[argno++] = progname;
  while ((args[argno++] = va_arg (ap, char *)) != NULL)
     ;
  va_end (ap);

  if (pipe (fds) == -1)
    return 0;

  switch (fork ())
    {
    case 0:			/* Child.  Read from pipe. */
      close (fds[1]);		/* Not needed. */
      if (fds[0] != 0)		/* Redirect 0 (stdin) only if needed.  */
	{
	  close (0);		/* We don't want the old stdin. */
	  dup (fds[0]);		/* Guaranteed to dup to 0 (stdin). */
	  close (fds[0]);	/* No longer needed. */
	}
      execvp (args[0], args);
      error (0, errno, _("failed to execute %s"), progname);
      _exit (2);		/* 2 for `cmp'. */
    case -1:			/* Error. */
      return 0;
    default:			/* Parent.  Write to pipe. */
      close (fds[0]);		/* Not needed. */
      return fdopen (fds[1], "w");
    }
}
