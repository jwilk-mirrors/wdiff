/* wdiff -- trampolin to mdiff for comparing on a word per word basis.
   Copyright (C) 1997, 1998, 1999 Free Software Foundation, Inc.
   Francois Pinard <pinard@iro.umontreal.ca>.

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

#include "wdiff.h"

#ifndef MDIFF_PROGRAM
# define MDIFF_PROGRAM "mdiff"
#endif

/* Exit codes values.  */
#define EXIT_DIFFERENCE 1	/* some differences found */
#define EXIT_ERROR 2		/* any other reason for exit */

#include <getopt.h>
#include <locale.h>
#include <unistd.h>

const char *program_name;	/* name of executing program */

static int quiet = 0;		/* if inhibiting mdiff call message */

static char **argument_list = NULL;	/* constructed argument list */
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

      string = xrealloc (string, length + 2);
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
This program is free software: you can redistribute it and/or modify\n\
it under the terms of the GNU General Public License as published by\n\
the Free Software Foundation, either version 3 of the License, or\n\
(at your option) any later version.\n\
\n\
This program is distributed in the hope that it will be useful,\n\
but WITHOUT ANY WARRANTY; without even the implied warranty of\n\
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n\
GNU General Public License for more details.\n\
\n\
You should have received a copy of the GNU General Public License\n\
along with this program.  If not, see <http://www.gnu.org/licenses/>.\n"), stdout);
}

static void
print_version (void)
{
  printf ("wdiff (GNU %s) %s\n", PACKAGE, VERSION);
  fputs (_("\
\n\
Copyright (C) 1997, 1998, 1999 Free Software Foundation, Inc.\n"), stdout);
  fputs (_("\
This is free software; see the source for copying conditions.  There is NO\n\
warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.\n"), stdout);
  fputs (_("\
\n\
Written by Franc,ois Pinard <pinard@iro.umontreal.ca>.\n"), stdout);
}

/* Option variables.  */

struct option const longopts[] = {
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
  {"no-init-term", no_argument, NULL, 'K'},	/* backwards compatibility */
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
  if (status != 0)
    fprintf (stderr, _("Try `%s --help' for more information.\n"),
	     program_name);
  else
    {
      /* *INDENT-OFF* */
      fputs (_("\
wdiff - Compute word differences by internally launching `mdiff -W'.\n\
This program exists mainly to support the now oldish `wdiff' syntax.\n"),
	     stdout);
      fputs ("\n", stdout);
      printf (_("\
Usage: %s [OPTION]... FILE1 FILE2\n"),
	      program_name);
      fputs ("\n", stdout);
      fputs (_("\
Mandatory arguments to long options are mandatory for short options too.\n"),
             stdout);
      fputs (_("  -C, --copyright            display copyright then exit\n"), stdout);
      fputs (_("  -1, --no-deleted           inhibit output of deleted words\n"), stdout);
      fputs (_("  -2, --no-inserted          inhibit output of inserted words\n"), stdout);
      fputs (_("  -3, --no-common            inhibit output of common words\n"), stdout);
      fputs (_("  -a, --auto-pager           automatically calls a pager\n"), stdout);
      fputs (_("  -h, --help                 display this help then exit\n"), stdout);
      fputs (_("  -i, --ignore-case          fold character case while comparing\n"), stdout);
      fputs (_("  -l, --less-mode            variation of printer mode for \"less\"\n"), stdout);
      fputs (_("  -n, --avoid-wraps          do not extend fields through newlines\n"), stdout);
      fputs (_("  -p, --printer              overstrike as for printers\n"), stdout);
      fputs (_("  -q, --quiet                inhibit the `mdiff' call message\n"), stdout);
      fputs (_("  -s, --statistics           say how many words deleted, inserted etc.\n"), stdout);
      fputs (_("  -t, --terminal             use termcap as for terminal displays\n"), stdout);
      fputs (_("  -v, --version              display program version then exit\n"), stdout);
      fputs (_("  -w, --start-delete=STRING  string to mark beginning of delete region\n"), stdout);
      fputs (_("  -x, --end-delete=STRING    string to mark end of delete region\n"), stdout);
      fputs (_("  -y, --start-insert=STRING  string to mark beginning of insert region\n"), stdout);
      fputs (_("  -z, --end-insert=STRING    string to mark end of insert region\n"), stdout);
      fputs ("\n", stdout);
      fputs (_("This program also tells how `mdiff' could have been called directly.\n"), stdout);
      fputs ("\n", stdout);
      fputs (_("Report bugs to <wdiff-bugs@gnu.org>.\n"), stdout);
      /* *INDENT-ON* */
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
  bindtextdomain (PACKAGE, LOCALEDIR);
  textdomain (PACKAGE);

  add_argument (MDIFF_PROGRAM);
  add_simple_option ('W');

  /* Decode arguments.  */

  while (option_char = getopt_long (argc, (char **) argv,
				    "123CKachidlnpqstvw:x:y:z:", longopts,
				    NULL), option_char != EOF)
    switch (option_char)
      {
      default:
	usage (EXIT_ERROR);

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
	/* compatibility option, equal to -t now */
	add_simple_option ('t');
	break;

      case 'a':
	add_simple_option ('A');
	break;

      case 'h':
	usage (EXIT_SUCCESS);

      case 'c':		/* this was -c prior to wdiff 0.5 */
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
      error (0, 0, _("missing file arguments"));
      usage (EXIT_ERROR);
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
  exit (EXIT_ERROR);
}
