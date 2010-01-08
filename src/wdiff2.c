/* wdiff -- trampolin to mdiff for comparing on a word per word basis.
   Copyright (C) 1997, 1998, 1999 Free Software Foundation, Inc.
   Francois Pinard <pinard@iro.umontreal.ca>.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software Foundation,
   Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.  */

#include "system.h"

#ifndef MDIFF_PROGRAM
# define MDIFF_PROGRAM "mdiff"
#endif

/* Exit codes values.  */
#define EXIT_DIFFERENCE 1	/* some differences found */
#undef EXIT_FAILURE
#define EXIT_FAILURE 2		/* any other reason for exit */

#include <getopt.h>

const char *program_name;	/* name of executing program */

static int quiet = 0;		/* if inhibiting mdiff call message */

static char **argument_list = NULL; /* constructed argument list */
static int arguments = 0;	/* number of arguments in list */
static int last_is_letters = 0;	/* if last argument is option letter(s) */

/*-------------------------------------------.
| Add an argument to the list of arguments.  |
`-------------------------------------------*/

static void
add_argument (const char *string)
{
  if (arguments % 16 == 0)
    argument_list = (char **)
      xrealloc (argument_list, (arguments + 16) * sizeof (char *));
  argument_list[arguments++] = string ? xstrdup (string) : NULL;
}

/*----------------------------------------------------.
| Add a mere option letter to the list of arguments.  |
`----------------------------------------------------*/

static void
add_simple_option (char letter)
{
  if (last_is_letters)
    {
      char *string = argument_list[arguments - 1];
      int length = strlen (string);

      string = (char *) xrealloc (string, length + 2);
      string[length] = letter;
      string[length + 1] = '\0';
      argument_list[arguments - 1] = string;
    }
  else
    {
      char buffer[3];

      buffer[0] = '-';
      buffer[1] = letter;
      buffer[2] = '\0';
      add_argument (buffer);

      last_is_letters = 1;
    }
}

/*-----------------------------------------------------------------.
| Add an option letter with an argument to the list of arguments.  |
`-----------------------------------------------------------------*/

static void
add_option_with_argument (char letter, const char *argument)
{
  char buffer[3];

  buffer[0] = '-';
  buffer[1] = letter;
  buffer[2] = '\0';
  add_argument (buffer);
  add_argument (argument);

  last_is_letters = 0;
}

/*-----------------------------------.
| Prints a more detailed Copyright.  |
`-----------------------------------*/

static void
print_copyright (void)
{
  fputs (_("\
This program is free software; you can redistribute it and/or modify\n\
it under the terms of the GNU General Public License as published by\n\
the Free Software Foundation; either version 2, or (at your option)\n\
any later version.\n\
\n\
This program is distributed in the hope that it will be useful,\n\
but WITHOUT ANY WARRANTY; without even the implied warranty of\n\
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n\
GNU General Public License for more details.\n\
\n\
You should have received a copy of the GNU General Public License\n\
along with this program; if not, write to the Free Software Foundation,\n\
Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.\n"),
	 stdout);
}

static void
print_version (void)
{
  printf ("wdiff (GNU %s) %s\n", PACKAGE, VERSION);
  fputs (_("\
\n\
Copyright (C) 1997 Free Software Foundation, Inc.\n"),
	 stdout);
  fputs (_("\
This is free software; see the source for copying conditions.  There is NO\n\
warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.\n"),
	 stdout);
  fputs (_("\
\n\
Written by Franc,ois Pinard <pinard@iro.umontreal.ca>.\n"),
	 stdout);
}

/* Option variables.  */

struct option const longopts[] =
{
  {"auto-pager", no_argument, NULL, 'a'},
  {"avoid-wraps", no_argument, NULL, 'n'},
  {"copyright", no_argument, NULL, 'C'},
  {"end-delete", required_argument, NULL, 'x'},
  {"end-insert", required_argument, NULL, 'z'},
  {"help", no_argument, NULL, 'h'},
  {"ignore-case", no_argument, NULL, 'i'},
  {"less-mode", no_argument, NULL, 'l'},
  {"no-common", no_argument, NULL, '3'},
  {"no-deleted", no_argument, NULL, '1'},
  {"no-init-term", no_argument, NULL, 'K'},
  {"no-inserted", no_argument, NULL, '2'},
  {"quiet", no_argument, NULL, 'q'},
  {"printer", no_argument, NULL, 'p'},
  {"start-delete", required_argument, NULL, 'w'},
  {"start-insert", required_argument, NULL, 'y'},
  {"statistics", no_argument, NULL, 's'},
  {"terminal", no_argument, NULL, 't'},
  {"version", no_argument, NULL, 'v'},
  {0, 0, 0, 0}
};

/*-----------------------------.
| Tell how to use, then exit.  |
`-----------------------------*/

static void
usage (int status)
{
  const char *optfmt;
  if (status != 0)
    fprintf (stderr, _("Try `%s --help' for more information.\n"),
	     program_name);
  else
    {
      fputs (_("\
wdiff - Compute word differences by internally launching `mdiff -W'.\n\
This program exists mainly to support the now oldish `wdiff' syntax.\n"),
	     stdout);
      printf (_("\
\n\
Usage: %s [OPTION]... FILE1 FILE2\n"),
	      program_name);
      printf ("\n%s\n", _("\
Mandatory arguments to long options are mandatory for short options too."));
      /* TRANSLATORS: --help option format, adjust for longer arguments. */
      optfmt = _("  %-26s %s\n");
      printf (optfmt, "-C, --copyright",
              _("print copyright then exit"));
      printf (optfmt, "-K, --no-init-term",
              _("like -t, but no termcap init/term strings"));
      printf (optfmt, "-1, --no-deleted",
              _("inhibit output of deleted words"));
      printf (optfmt, "-2, --no-inserted",
              _("inhibit output of inserted words"));
      printf (optfmt, "-3, --no-common",
              _("inhibit output of common words"));
      printf (optfmt, "-a, --auto-pager",
              _("automatically calls a pager"));
      printf (optfmt, "-h, --help",
              _("print this help"));
      printf (optfmt, "-i, --ignore-case",
              _("fold character case while comparing"));
      printf (optfmt, "-l, --less-mode",
              _("variation of printer mode for \"less\""));
      printf (optfmt, "-n, --avoid-wraps",
              _("do not extend fields through newlines"));
      printf (optfmt, "-p, --printer",
              _("overstrike as for printers"));
      printf (optfmt, "-q, --quiet",
              _("inhibit the `mdiff' call message"));
      printf (optfmt, "-s, --statistics",
              _("say how many words deleted, inserted etc."));
      printf (optfmt, "-t, --terminal",
              _("use termcap as for terminal displays"));
      printf (optfmt, "-v, --version",
              _("print program version then exit"));
      printf (optfmt, _("-w, --start-delete=STRING"),
              _("string to mark beginning of delete region"));
      printf (optfmt, _("-x, --end-delete=STRING"),
              _("string to mark end of delete region"));
      printf (optfmt, _("-y, --start-insert=STRING"),
              _("string to mark beginning of insert region"));
      printf (optfmt, _("-z, --end-insert=STRING"),
              _("string to mark end of insert region"));
      printf ("\n%s\n", _("\
This program also tells how `mdiff' could have been called directly."));
      printf ("\n%s\n", _("Report bugs to <wdiff-bugs@gnu.org>."));
    }
  exit (status);
}

/*---------------.
| Main program.	 |
`---------------*/

int
main (int argc, char *const argv[])
{
  int option_char;		/* option character */

  program_name = argv[0];
  setlocale (LC_ALL, "");

  add_argument (MDIFF_PROGRAM);
  add_simple_option ('W');

  /* Decode arguments.  */

  while (option_char = getopt_long (argc, (char **) argv,
				    "123CKachidlnpqstvw:x:y:z:", longopts,
				    NULL),
	 option_char != EOF)
    switch (option_char)
      {
      default:
	usage (EXIT_FAILURE);

      case '\0':
	break;

      case '1':
      case '2':
      case '3':
	add_simple_option (option_char);
	break;

      case 'C':
	print_copyright ();
	exit (EXIT_SUCCESS);

      case 'K':
	add_simple_option ('K');
	break;

      case 'a':
	add_simple_option ('A');
	break;

      case 'h':
	usage (EXIT_SUCCESS);

      case 'c':			/* this was -c prior to wdiff 0.5 */
      case 'i':
	add_simple_option ('i');
	break;

      case 'l':
	add_simple_option ('k');
	break;

      case 'n':
	add_simple_option ('m');
	break;

      case 'p':
	add_simple_option ('o');
	break;

      case 'q':
	quiet = 1;
	break;

      case 's':
	add_simple_option ('v');
	break;

      case 't':
	add_simple_option ('z');
	break;

      case 'v':
	print_version ();
	exit (EXIT_SUCCESS);

      case 'w':
	add_option_with_argument ('Y', optarg);
	break;

      case 'x':
	add_option_with_argument ('Z', optarg);
	break;

      case 'y':
	add_option_with_argument ('Q', optarg);
	break;

      case 'z':
	add_option_with_argument ('R', optarg);
	break;
      }

  if (optind + 2 != argc)
    {
      error (0, 0, _("Missing file arguments"));
      usage (EXIT_FAILURE);
    }

  add_argument (argv[optind]);
  add_argument (argv[optind + 1]);

  if (!quiet)
    {
      int counter;

      /* TRANSLATORS: This and the next string are one message. */
      fprintf (stderr, _("Launching `%s"), MDIFF_PROGRAM);
      for (counter = 1; counter < arguments; counter++)
	fprintf (stderr, " %s", argument_list[counter]);
      fprintf (stderr, _("'\n"));
    }

  add_argument (NULL);
  execvp (MDIFF_PROGRAM, argument_list);
  /* Should never return.  */
  exit (EXIT_FAILURE);
}
