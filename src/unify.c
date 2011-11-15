/* unify -- Change a diff to/from a context diff from/to a unified diff.
   Copyright (C) 1994, 1997, 1998 Free Software Foundation, Inc.

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

/* From the public domain version 1.1, as found on 1992-12-22 at
   ftp://ftp.uu.net/usenet/comp.sources.misc/volume25/unidiff/part01.Z.

   Originally written by Wayne Davison <davison@borland.com>.  */

#include "wdiff.h"
#include <getopt.h>
#include <string.h>
#include <locale.h>

/* FIXME: Programs should not have such limits.  */
#define NAME_LENGTH 255
#define BUFFER_LENGTH 2048

/* The name this program was run with. */
const char *program_name;

/* If nonzero, display usage information and exit.  */
static int show_help = 0;

/* If nonzero, print the version on standard output and exit.  */
static int show_version = 0;

enum type
{
  TYPE_UNDECIDED,
  TYPE_UNIDIFF,
  TYPE_CDIFF
};

static enum type output_type = TYPE_UNDECIDED;
static int force_old_style = 0;	/* old-style diff, no matter what */
static int echo_comments = 0;	/* echo comments to stderr */
static int strip_comments = 0;	/* strip comment lines */
static int patch_format = 0;	/* generate patch format */
static int use_equals = 0;	/* use '=' not ' ' in unified diffs */

/* Common definitions.  */

enum state
{
  FIND_NEXT,
  PARSE_UNIDIFF,
  UNI_LINES,
  PARSE_CDIFF,
  PARSE_OLD,
  CHECK_OLD,
  OLD_LINES,
  PARSE_NEW,
  NEW_LINES
};
static enum state state = FIND_NEXT;

struct line_object
{
  struct line_object *next;
  char type;
  int number;
  char string[1];
};
struct line_object root, *head = &root, *hold = &root, *line;

long old_first = 0, old_last = -1;
long new_first = 0, new_last = 0;
long old_start, old_end, old_line;
long new_start, new_end, new_line;
long line_number = 0;
static enum type input_type = TYPE_UNDECIDED;
static int found_index = 0;
static char name[NAME_LENGTH + 1] = { '\0' };

static char buffer[BUFFER_LENGTH];

/*---------------------------------------------------------------.
| Decode integer at CURSOR into RESULT, leave CURSOR past it.	 |
`---------------------------------------------------------------*/

#define SCAN_INTEGER(Cursor, Result) \
  do								\
    {								\
      Result = atol (Cursor);					\
      while (*Cursor <= '9' && *Cursor >= '0')			\
	Cursor++;						\
    }								\
  while (0)

/*--------------------------------------------------------------------.
| Check if two strings are equal, given a maximum comparison length.  |
`--------------------------------------------------------------------*/

static inline int
string_equal (const char *string1, const char *string2, int length)
{
  return !strncmp (string1, string2, length);
}

/*---.
| ?  |
`---*/

static void
add_line (char type, long number, char *string)
{
  line = (struct line_object *)
    xmalloc (sizeof (struct line_object) + strlen (string));

  line->type = type;
  line->number = number;
  strcpy (line->string, string);
  line->next = hold->next;
  hold->next = line;
  hold = line;
}

/*---.
| ?  |
`---*/

static void
ensure_name (void)
{
  char *cursor = name;

  if (found_index)
    return;

  if (!*name)
    error (0, 0, _("could not find a name for the diff at line %ld"),
	   line_number);
  else if (patch_format)
    {
      if (cursor[0] == '.' && cursor[1] == '/')
	cursor += 2;
      printf ("Index: %s\n", cursor);
    }
}

/*---.
| ?  |
`---*/

static void
generate_output (void)
{
  if (old_last < 0)
    return;

  if (output_type == TYPE_UNIDIFF)
    {
      long i, j;

      i = old_first ? old_last - old_first + 1 : 0;
      j = new_first ? new_last - new_first + 1 : 0;
      printf ("@@ -%ld,%ld +%ld,%ld @@\n", old_first, i, new_first, j);
      for (line = root.next; line; line = hold)
	{
	  printf ("%c%s", use_equals && line->type == ' ' ? '=' : line->type,
		  line->string);
	  hold = line->next;
	  free (line);
	}
    }
  else				/* if output_type == TYPE_CDIFF */
    {
      struct line_object *scan;
      int found_plus = 1;
      char character;

      printf ("***************\n*** %ld", old_first);

      if (old_first == old_last)
	printf (" ****\n");
      else
	printf (",%ld ****\n", old_last);

      for (line = root.next; line; line = line->next)
	if (line->type == '-')
	  break;

      if (line)
	{
	  found_plus = 0;
	  character = ' ';
	  for (line = root.next; line; line = line->next)
	    {
	      switch (line->type)
		{
		case '-':
		  if (character != ' ')
		    break;

		  scan = line;
		  while (scan = scan->next,
			 (scan != NULL && scan->type == '-'))
		    ;
		  if (scan && scan->type == '+')
		    {
		      do
			{
			  scan->type = '!';
			}
		      while (scan = scan->next, (scan && scan->type == '+'));
		      character = '!';
		    }
		  else
		    character = '-';
		  break;

		case '+':
		case '!':
		  found_plus = 1;
		  continue;

		case ' ':
		  character = ' ';
		  break;
		}
	      printf ("%c %s", character, line->string);
	    }
	}

      if (new_first == new_last)
	printf ("--- %ld ----\n", new_first);
      else
	printf ("--- %ld,%ld ----\n", new_first, new_last);

      if (found_plus)
	for (line = root.next; line; line = line->next)
	  if (line->type != '-')
	    printf ("%c %s", line->type, line->string);

      for (line = root.next; line; line = hold)
	{
	  hold = line->next;
	  free (line);
	}
    }

  root.next = NULL;
  head = &root;
  hold = &root;

  old_first = 0;
  new_first = 0;
  old_last = -1;
  new_last = 0;
}

/*-----------------------------------------------.
| Explain how to use the program, then get out.	 |
`-----------------------------------------------*/

static void
usage (int status)
{
  if (status != EXIT_SUCCESS)
    fprintf (stderr, _("try `%s --help' for more information\n"),
	     program_name);
  else
    {
      /* *INDENT-OFF* */
      fputs (_("\
unify - Transforms context diffs into unidiffs, or vice-versa.\n"),
	     stdout);
      fputs ("\n", stdout);
      printf (_("\
Usage: %s [OPTION]... [FILE]\n"), program_name);
      fputs ("\n", stdout);
      fputs (_("  -c, --context-diffs    force output to context diffs\n"), stdout);
      fputs (_("  -e, --echo-comments    echo comments to standard error\n"), stdout);
      fputs (_("  -o, --old-diffs        output old-style diffs, no matter what\n"), stdout);
      fputs (_("  -p, --patch-format     generate patch format\n"), stdout);
      fputs (_("  -P                     same as -p\n"), stdout);
      fputs (_("  -s, --strip-comments   strip comment lines\n"), stdout);
      fputs (_("  -u, --unidiffs         force output to unidiffs\n"), stdout);
      fputs (_("  -U                     same as -p and -u\n"), stdout);
      fputs (_("  -=, --use-equals       replace spaces by equal signs in unidiffs\n"), stdout);
      fputs (_("      --help             display this help then exit\n"), stdout);
      fputs (_("      --version          display program version then exit\n"), stdout);
      fputs ("\n", stdout);
      fputs (_("If FILE is not specified, read standard input.\n"), stdout);
      fputs ("\n", stdout);
      fputs (_("Report bugs to <wdiff-bugs@gnu.org>.\n"), stdout);
      /* *INDENT-ON* */
    }
  exit (status);
}

/*----------------------------------------------------------------------.
| Main program.  Decode ARGC arguments passed through the ARGV array of |
| strings, then launch execution.				        |
`----------------------------------------------------------------------*/

/* Long options equivalences.  */
static const struct option long_options[] = {
  {"context-diffs", no_argument, NULL, 'c'},
  {"echo-comments", no_argument, NULL, 'e'},
  {"help", no_argument, &show_help, 1},
  {"old-diffs", no_argument, NULL, 'o'},
  {"patch-format", no_argument, NULL, 'p'},
  {"strip-comments", no_argument, NULL, 's'},
  {"unidiffs", no_argument, NULL, 'u'},
  {"use-equals", no_argument, NULL, '='},
  {"version", no_argument, &show_version, 1},
  {0, 0, 0, 0},
};

int
main (int argc, char *const *argv)
{
  int option_char;		/* option character */
  FILE *file = stdin;

  char previous_start = ' ';
  char star_in_cdiff;		/* if '*' seen in a new-style context diff */
  char type;

  program_name = argv[0];
  setlocale (LC_ALL, "");
  bindtextdomain (PACKAGE, LOCALEDIR);
  textdomain (PACKAGE);

  /* Decode command options.  */

  while (option_char = getopt_long (argc, (char **) argv, "=ceopPsUu",
				    long_options, NULL), option_char != EOF)
    switch (option_char)
      {
      default:
	usage (EXIT_FAILURE);

      case '\0':
	break;

      case '=':
	use_equals = 1;
	break;

      case 'c':
	output_type = TYPE_CDIFF;
	break;

      case 'e':
	echo_comments = 1;
	break;

      case 'o':
	force_old_style = 1;
	break;

      case 'p':
      case 'P':
	patch_format = 1;
	break;

      case 's':
	strip_comments = 1;
	break;

      case 'U':
	patch_format = 1;
	/* Fall through.  */

      case 'u':		/* force unified output */
	output_type = TYPE_UNIDIFF;
	break;
      }

  if (optind < argc)
    {
      if (file = fopen (argv[optind], "r"), file == NULL)
	error (EXIT_FAILURE, errno, _("unable to open `%s'"), argv[optind]);
      optind++;
    }

  if (optind < argc)
    {
      error (0, 0, _("only one filename allowed"));
      usage (EXIT_FAILURE);
    }

  /* Process trivial options.  */

  if (show_version)
    {
      printf ("unify (GNU %s) %s\n", PACKAGE, VERSION);
      fputs (_("\
\n\
Copyright (C) 1994, 1997 Free Software Foundation, Inc.\n"), stdout);
      fputs (_("\
This is free software; see the source for copying conditions.  There is NO\n\
warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.\n"), stdout);
      fputs (_("\
\n\
Written by Wayne Davison <davison@borland.com>.\n"), stdout);
      exit (EXIT_SUCCESS);
    }

  if (show_help)
    usage (EXIT_SUCCESS);

  /* Read and digest the input file.  */

  while (fgets (buffer, sizeof buffer, file))
    {
      line_number++;

    reprocess:
      switch (state)
	{
	case FIND_NEXT:
	  if (input_type != TYPE_CDIFF && string_equal (buffer, "@@ -", 4))
	    {
	      input_type = TYPE_UNIDIFF;
	      if (output_type == TYPE_UNDECIDED)
		output_type = TYPE_CDIFF;
	      ensure_name ();
	      state = PARSE_UNIDIFF;
	      goto reprocess;
	    }

	  if (input_type != TYPE_UNIDIFF
	      && string_equal (buffer, "********", 8))
	    {
	      input_type = TYPE_CDIFF;
	      if (output_type == TYPE_UNDECIDED)
		output_type = TYPE_UNIDIFF;
	      ensure_name ();
	      state = PARSE_OLD;
	      break;
	    }

	  if (string_equal (buffer, "Index:", 6))
	    {
	      found_index = 1;
	      printf ("%s", buffer);
	    }
	  else if (string_equal (buffer, "Prereq: ", 8))
	    printf ("%s", buffer);
	  else if (string_equal (buffer, "*** ", 4)
		   || string_equal (buffer, "--- ", 4)
		   || string_equal (buffer, "+++ ", 4))
	    {
	      if (!found_index)
		{
		  char *cursor;
		  int length;

		  for (cursor = buffer + 4, length = 0;
		       *cursor > ' ' && length < NAME_LENGTH;
		       cursor++, length++)
		    ;
		  if (!*name || length < strlen (name))
		    {
		      strncpy (name, buffer + 4, length);
		      name[length] = '\0';
		    }
		}
	      if (!patch_format)
		{
		  if (output_type == TYPE_UNIDIFF
		      && (*buffer == '+'
			  || *buffer == '-' && previous_start != '*')
		      || output_type == TYPE_CDIFF
		      && (*buffer == '*'
			  || *buffer == '-' && previous_start == '*'))
		    printf ("%s", buffer);
		  else if (*buffer == '*' || *buffer == '+')
		    printf ("---%s", buffer + 3);
		  else if (*buffer == '-' && previous_start == '*')
		    printf ("+++%s", buffer + 3);
		  else
		    printf ("***%s", buffer + 3);

		  previous_start = *buffer;
		}
	    }
	  else if (patch_format
		   && (string_equal (buffer, "Only in ", 8)
		       || string_equal (buffer, "Common subdir", 13)
		       || string_equal (buffer, "diff -", 6)
		       || string_equal (buffer, "Binary files", 12)))
	    {
	      if (echo_comments)
		fprintf (stderr, "%s%s", strip_comments ? "" : "!!! ",
			 buffer);
	    }
	  else
	    {
	      if (echo_comments)
		fprintf (stderr, "%s", buffer);
	      if (!strip_comments)
		printf ("%s", buffer);
	    }
	  break;

	case PARSE_UNIDIFF:
	  {
	    char *cursor;

	    if (!string_equal (buffer, "@@ -", 4))
	      {
		found_index = 0;
		*name = '\0';
		state = FIND_NEXT;
		goto reprocess;
	      }
	    cursor = buffer + 4;
	    SCAN_INTEGER (cursor, old_start);
	    if (*cursor++ == ',')
	      {
		SCAN_INTEGER (cursor, old_end);
		cursor++;
	      }
	    else
	      old_end = 1;
	    if (*cursor++ != '+')
	      goto bad_header;
	    SCAN_INTEGER (cursor, new_start);
	    if (*cursor++ == ',')
	      {
		SCAN_INTEGER (cursor, new_end);
		cursor++;
	      }
	    else
	      new_end = 1;

	    if (*cursor != '@')
	    bad_header:
	      error (EXIT_FAILURE, 0,
		     _("invalid unified diff header at line %ld"),
		     line_number);

	    old_end = (old_start ? old_start + old_end - 1 : 0);
	    new_end = (new_start ? new_start + new_end - 1 : 0);
	    old_first = old_start;
	    new_first = new_start;

	    if (old_start)
	      old_line = old_start - 1;
	    else
	      old_line = old_last = 0;

	    if (new_start)
	      new_line = new_start - 1;
	    else
	      new_line = new_last = 0;

	    state = UNI_LINES;
	    break;
	  }

	case UNI_LINES:
	  switch (*buffer)
	    {
	    case ' ':
	    case '=':
	      *buffer = ' ';
	      old_last = ++old_line;
	      new_last = ++new_line;
	      break;

	    case '-':
	      old_last = ++old_line;
	      break;

	    case '+':
	      new_last = ++new_line;
	      break;

	    default:
	      error (EXIT_FAILURE, 0,
		     _("malformed unified diff at line %ld"), line_number);
	    }
	  add_line (*buffer, 0L, buffer + 1);
	  if (old_line == old_end && new_line == new_end)
	    {
	      generate_output ();
	      state = PARSE_UNIDIFF;
	    }
	  break;

	case PARSE_CDIFF:
	  if (!string_equal (buffer, "********", 8))
	    {
	      generate_output ();
	      found_index = 0;
	      *name = '\0';
	      state = FIND_NEXT;
	      goto reprocess;
	    }
	  state = PARSE_OLD;
	  break;

	case PARSE_OLD:
	  star_in_cdiff = ' ';
	  old_start = -1;
	  if (sscanf (buffer, "*** %ld,%ld %c", &old_start, &old_end,
		      &star_in_cdiff) < 2)
	    {
	      if (old_start < 0)
		error (EXIT_FAILURE, 0,
		       _("context diff missing `old' header at line %ld"),
		       line_number);
	      old_end = old_start;
	      star_in_cdiff = ' ';
	    }
	  else if (force_old_style)
	    star_in_cdiff = ' ';
	  if (old_last >= 0)
	    {
	      if (old_start > old_last)
		generate_output ();
	      else
		{
		  star_in_cdiff = ' ';
		  while (head->next && head->next->number != old_start)
		    head = head->next;
		}
	    }
	  old_line = old_start - 1;
	  new_line = 0;
	  if (!old_first)
	    old_first = old_start;
	  if (!old_start)
	    state = PARSE_NEW;
	  else
	    state = CHECK_OLD;
	  break;

	case CHECK_OLD:
	  if (string_equal (buffer, "--- ", 4))
	    state = PARSE_NEW;
	  else
	    {
	      state = OLD_LINES;
	      hold = head;
	    }
	  goto reprocess;

	case OLD_LINES:
	  if (buffer[0] == '\n')
	    strcpy (buffer, "  \n");
	  if (buffer[1] == '\n')
	    strcpy (buffer + 1, " \n");
	  if (buffer[1] != ' ')
	    error (EXIT_FAILURE, 0, _("malformed context diff at line %ld"),
		   line_number);

	  switch (*buffer)
	    {
	    case ' ':
	      type = ' ';
	      new_line++;
	      old_line++;
	      break;

	    case '-':
	    case '!':
	      type = '-';
	      old_line++;
	      break;

	    default:
	      error (EXIT_FAILURE, 0,
		     _("malformed context diff at line %ld"), line_number);
	    }
	  if (old_line > old_last)
	    {
	      add_line (type, 0L, buffer + 2);
	      old_last = old_line;
	      new_last = new_line;
	    }
	  else
	    {
	      do
		{
		  hold = hold->next;
		}
	      while (hold->type == '+');

	      if (type != ' ')
		{
		  hold->type = type;
		  hold->number = 0;
		}
	    }
	  if (old_line == old_end)
	    state = PARSE_NEW;
	  break;

	case PARSE_NEW:
	  if (*buffer == '\n')
	    break;

	  new_start = -1;
	  if (sscanf (buffer, "--- %ld,%ld", &new_start, &new_end) != 2)
	    {
	      if (new_start < 0)
		error (EXIT_FAILURE, 0,
		       _("context diff missing `new' header at line %ld"),
		       line_number);
	      new_end = new_start;
	    }
	  new_last = new_line;
	  old_line = old_start ? old_start - 1 : 0;
	  new_line = new_start ? new_start - 1 : 0;
	  new_last += new_line;
	  hold = head;
	  if (!new_first)
	    {
	      new_first = new_start;
	      while (hold->next && hold->next->type == '-')
		{
		  hold = hold->next;
		  hold->number = ++old_line;
		}
	    }

	  if (star_in_cdiff == '*' && new_last == new_end)
	    {
	      state = PARSE_CDIFF;
	      break;
	    }

	  state = NEW_LINES;
	  break;

	case NEW_LINES:
	  if (buffer[0] == '\n')
	    strcpy (buffer, "  \n");
	  if (buffer[1] == '\n')
	    strcpy (buffer + 1, " \n");
	  if (buffer[1] != ' ')
	    error (EXIT_FAILURE, 0,
		   _("malformed context diff at line %ld"), line_number);

	  switch (*buffer)
	    {
	    case ' ':
	      type = ' ';
	      new_line++;
	      old_line++;
	      break;

	    case '+':
	    case '!':
	      type = '+';
	      new_line++;
	      break;

	    default:
	      error (EXIT_FAILURE, 0,
		     _("malformed context diff at line %ld"), line_number);
	    }

	  if (old_line > old_last)
	    {
	      old_last = old_line;
	      add_line (type, old_line, buffer + 2);
	      new_last++;
	    }
	  else if (type != ' ')
	    {
	      add_line (type, 0L, buffer + 2);
	      new_last++;
	    }
	  else
	    {
	      hold = hold->next;
	      hold->number = old_line;
	      while (hold->next && !hold->next->number
		     && hold->next->type != ' ')
		{
		  hold = hold->next;
		  if (hold->type == '-')
		    hold->number = ++old_line;
		}
	    }
	  if (old_line == old_end && new_line == new_end)
	    state = PARSE_CDIFF;
	  break;
	}
    }

  /* Produce output and exit.  */

  generate_output ();

  exit (EXIT_SUCCESS);
}
