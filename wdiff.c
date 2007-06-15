/* wdiff -- front end to diff for comparing on a word per word basis.
   Copyright (C) 1992 Free Software Foundation, Inc.
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

#if HAVE_CONFIG_H
# include <config.h>
#endif

/* Exit codes values.  */
#define EXIT_NO_DIFFERENCES 0	/* no differences found */
#define EXIT_ANY_DIFFERENCE 1	/* some differences found */
#define EXIT_OTHER_REASON 2	/* any other reason for exit */

/* It is mandatory that some `diff' program is selected for use.  The
   following definition may also include the complete path.  */
#ifndef DIFF_PROGRAM
# define DIFF_PROGRAM "diff"
#endif

/* One may also, optionnaly, define a default PAGER_PROGRAM.  This might
   be done from the Makefile.  If PAGER_PROGRAM is undefined and the
   PAGER environment variable is not set, none will be used.  */

/* Define the separator lines when output is inhibited.  */
#define SEPARATOR_LINE \
  "======================================================================"

/* Library declarations.  */

#if STDC_HEADERS
# include <stdlib.h>
#endif

#include <ctype.h>
#include <stdio.h>

#if STDC_HEADERS
# include <string.h>
#else
# if HAVE_STRING_H
#  include <string.h>
# else
#  include <strings.h>
#  define strrchr rindex
# endif
#endif

char *strstr ();

#if HAVE_TPUTS
# if HAVE_TERMCAP_H
#  include <termcap.h>
# else
const char *tgetstr ();
# endif
#endif

#include <sys/types.h>
#include <setjmp.h>
#include <signal.h>
#ifndef RETSIGTYPE
# define RETSIGTYPE void
#endif

#include <sys/stat.h>

#if HAVE_UNISTD_H
# include <unistd.h>
#endif

#include <errno.h>
#ifndef errno
extern int errno;
#endif

#include "getopt.h"

#ifdef PROTOTYPES
# define _(Args) Args
#else
# define _(Args) ()
#endif

char *getenv ();
FILE *readpipe _((const char *, ...));
FILE *writepipe _((const char *, ...));
char *tmpnam ();
void error ();

/* Declarations.  */

/* Option variables.  */

struct option const longopts[] =
{
  {"copyright"   , 0, NULL, 'C'},
  {"version"     , 0, NULL, 'V'},
  {"no-deleted"  , 0, NULL, '1'},
  {"no-inserted" , 0, NULL, '2'},
  {"no-common"   , 0, NULL, '3'},
  {"help"        , 0, NULL, 'h'},
  {"ignore-case" , 0, NULL, 'i'},
  {"printer"     , 0, NULL, 'p'},
  {"statistics"  , 0, NULL, 's'},
  {"terminal"    , 0, NULL, 't'},
  {"start-delete", 1, NULL, 'w'},
  {"end-delete"  , 1, NULL, 'x'},
  {"start-insert", 1, NULL, 'y'},
  {"end-insert"  , 1, NULL, 'z'},
  {NULL          , 0, NULL, 0}
};

static char const copyright[] =
  "Copyright (C) 1992 Free Software Foundation, Inc.";

const char *program_name;	/* name of executing program */

int inhibit_left;		/* inhibit display of left side words */
int inhibit_right;		/* inhibit display of left side words */
int inhibit_common;		/* inhibit display of common words */
int ignore_case;		/* ignore case in comparisons */
int show_statistics;		/* if printing summary statistics */
int no_wrapping;		/* end/restart strings at end of lines */
int autopager;			/* if calling the pager automatically */
int overstrike;			/* if using printer overstrikes */
int overstrike_for_less;	/* if output aimed to the "less" program */
const char *user_delete_start;	/* user specified string for start of delete */
const char *user_delete_end;	/* user specified string for end of delete */
const char *user_insert_start;	/* user specified string for start of insert */
const char *user_insert_end;	/* user specified string for end of insert */

int find_termcap;		/* initialize the termcap strings */
const char *term_delete_start;	/* termcap string for start of delete */
const char *term_delete_end;	/* termcap string for end of delete */
const char *term_insert_start;	/* termcap string for start of insert */
const char *term_insert_end;	/* termcap string for end of insert */

/* Other variables.  */

enum copy_mode
{
  COPY_NORMAL,			/* copy text unemphasized */
  COPY_DELETED,			/* copy text underlined */
  COPY_INSERTED			/* copy text bolded */
}
copy_mode;

jmp_buf signal_label;		/* where to jump when signal received */
int interrupted;		/* set when some signal has been received */

/* Guarantee some value for L_tmpnam.  */
#ifndef L_tmpnam
# include "pathmax.h"
# define L_tmpnam PATH_MAX
#endif

typedef struct side SIDE;	/* all variables for one side */
struct side
{
  const char *filename;		/* original input file name */
  FILE *file;			/* original input file */
  int position;			/* number of words read so far */
  int character;		/* one character look ahead */
  char temp_name[L_tmpnam];	/* temporary file name */
  FILE *temp_file;		/* temporary file */
};
SIDE side_array[2];		/* area for holding side descriptions */
SIDE *left_side = &side_array[0];
SIDE *right_side = &side_array[1];

FILE *input_file;		/* stream being produced by diff */
int character;			/* for reading input_file */
char directive;			/* diff directive character */
int argument[4];		/* four diff directive arguments */

FILE *output_file;		/* file to which we write output */
const char *termcap_init_string; /* how to initialize the termcap mode */
const char *termcap_end_string; /* how to complete the termcap mode */

int count_total_left;		/* count of total words in left file */
int count_total_right;		/* count of total words in right file */
int count_isolated_left;	/* count of deleted words in left file */
int count_isolated_right;	/* count of added words in right file */
int count_changed_left;		/* count of changed words in left file */
int count_changed_right;	/* count of changed words in right file */

/* Signal processing.  */

/*-----------------.
| Signal handler.  |
`-----------------*/

static RETSIGTYPE
signal_handler (int number)
{
  interrupted = 1;
  signal (number, signal_handler);
}

/*----------------------------.
| Prepare to handle signals.  |
`----------------------------*/

static void
setup_signals (void)
{
  interrupted = 0;

  /* Intercept willingful requests for stopping.  */

  signal (SIGINT, signal_handler);
  signal (SIGPIPE, signal_handler);
  signal (SIGTERM, signal_handler);
}


/* Terminal initialization.  */

static void
initialize_strings (void)
{
#if HAVE_TPUTS
  if (find_termcap)
    {
      const char *name;		/* terminal capability name */
      char term_buffer[2048];	/* terminal description */
      static char *buffer;	/* buffer for capabilities */
      char *filler;		/* cursor into allocated strings */
      int success;		/* tgetent results */

      name = getenv ("TERM");
      if (name == NULL)
	error (1, 0, "Specify a terminal type with `setenv TERM <yourtype>'.");
      success = tgetent (term_buffer, name);
      if (success < 0)
	error (1, 0, "Could not access the termcap data base.");
      if (success == 0)
	error (1, 0, "Terminal type `%s' is not defined.", name);
      buffer = (char *) malloc (strlen (term_buffer));
      filler = buffer;

      termcap_init_string = tgetstr ("ti", &filler);
      termcap_end_string = tgetstr ("te", &filler);
      term_delete_start = tgetstr ("us", &filler);
      term_delete_end = tgetstr ("ue", &filler);
      term_insert_start = tgetstr ("so", &filler);
      term_insert_end = tgetstr ("se", &filler);
    }
#endif /* HAVE_TPUTS */

  /* Ensure some default strings.  */

  if (!overstrike)
    {
      if (!term_delete_start && !user_delete_start)
	user_delete_start = "[-";
      if (!term_delete_end && !user_delete_end)
	user_delete_end = "-]";
      if (!term_insert_start && !user_insert_start)
	user_insert_start = "{+";
      if (!term_insert_end && !user_insert_end)
	user_insert_end = "+}";
    }
}


/* Character input and output.  */

#if HAVE_TPUTS

/*-----------------------------------------.
| Write one character for tputs function.  |
`-----------------------------------------*/

static int
putc_for_tputs (int chr)
{
  return putc (chr, output_file);
}

#endif /* HAVE_TPUTS */

/*---------------------------.
| Indicate start of delete.  |
`---------------------------*/

static void
start_of_delete (void)
{

  /* Avoid any emphasis if it would be useless.  */

  if (inhibit_common && (inhibit_right || inhibit_left))
    return;

  copy_mode = COPY_DELETED;
#if HAVE_TPUTS
  if (term_delete_start)
    tputs (term_delete_start, 0, putc_for_tputs);
#endif
  if (user_delete_start)
    fprintf (output_file, "%s", user_delete_start);
}

/*-------------------------.
| Indicate end of delete.  |
`-------------------------*/

static void
end_of_delete (void)
{

  /* Avoid any emphasis if it would be useless.  */

  if (inhibit_common && (inhibit_right || inhibit_left))
    return;

  if (user_delete_end)
    fprintf (output_file, "%s", user_delete_end);
#if HAVE_TPUTS
  if (term_delete_end)
    tputs (term_delete_end, 0, putc_for_tputs);
#endif
  copy_mode = COPY_NORMAL;
}

/*---------------------------.
| Indicate start of insert.  |
`---------------------------*/

static void
start_of_insert (void)
{

  /* Avoid any emphasis if it would be useless.  */

  if (inhibit_common && (inhibit_right || inhibit_left))
    return;

  copy_mode = COPY_INSERTED;
#if HAVE_TPUTS
  if (term_insert_start)
    tputs (term_insert_start, 0, putc_for_tputs);
#endif
  if (user_insert_start)
    fprintf (output_file, "%s", user_insert_start);
}

/*-------------------------.
| Indicate end of insert.  |
`-------------------------*/

static void
end_of_insert (void)
{

  /* Avoid any emphasis if it would be useless.  */

  if (inhibit_common && (inhibit_right || inhibit_left))
    return;

  if (user_insert_end)
    fprintf (output_file, "%s", user_insert_end);
#if HAVE_TPUTS
  if (term_insert_end)
    tputs (term_insert_end, 0, putc_for_tputs);
#endif
  copy_mode = COPY_NORMAL;
}

/*--------------------------------.
| Skip over white space on SIDE.  |
`--------------------------------*/

static void
skip_whitespace (SIDE *side)
{
  if (interrupted)
    longjmp (signal_label, 1);

  while (isspace (side->character))
    side->character = getc (side->file);
}

/*------------------------------------.
| Skip over non white space on SIDE.  |
`------------------------------------*/

static void
skip_word (SIDE *side)
{
  if (interrupted)
    longjmp (signal_label, 1);

  while (side->character != EOF && !isspace (side->character))
    side->character = getc (side->file);
  side->position++;
}

/*-------------------------------------.
| Copy white space from SIDE to FILE.  |
`-------------------------------------*/

static void
copy_whitespace (SIDE *side, FILE *file)
{
  if (interrupted)
    longjmp (signal_label, 1);

  while (isspace (side->character))
    {

      /* While changing lines, ensure we stop any special display prior
	 to, and restore the special display after.  When copy_mode is
	 anything else than COPY_NORMAL, file is always output_file.  We
	 care underlining whitespace or overstriking it with itself,
	 because "less" understands these things as emphasis requests.  */

      switch (copy_mode)
	{
	case COPY_NORMAL:
	  putc (side->character, file);
	  break;

	case COPY_DELETED:
	  if (side->character == '\n')
	    {
	      if (no_wrapping && user_delete_end)
		fprintf (output_file, "%s", user_delete_end);
#if HAVE_TPUTS
	      if (term_delete_end)
		tputs (term_delete_end, 0, putc_for_tputs);
#endif
	      putc ('\n', output_file);
#if HAVE_TPUTS
	      if (term_delete_start)
		tputs (term_delete_start, 0, putc_for_tputs);
#endif
	      if (no_wrapping && user_delete_start)
		fprintf (output_file, "%s", user_delete_start);
	    }
	  else if (overstrike_for_less)
	    {
	      putc ('_', output_file);
	      putc ('\b', output_file);
	      putc (side->character, output_file);
	    }
	  else
	    putc (side->character, output_file);
	  break;

	case COPY_INSERTED:
	  if (side->character == '\n')
	    {
	      if (no_wrapping && user_insert_end)
		fprintf (output_file, "%s", user_insert_end);
#if HAVE_TPUTS
	      if (term_insert_end)
		tputs (term_insert_end, 0, putc_for_tputs);
#endif
	      putc ('\n', output_file);
#if HAVE_TPUTS
	      if (term_insert_start)
		tputs (term_insert_start, 0, putc_for_tputs);
#endif
	      if (no_wrapping && user_insert_start)
		fprintf (output_file, "%s", user_insert_start);
	    }
	  else if (overstrike_for_less)
	    {
	      putc (side->character, output_file);
	      putc ('\b', output_file);
	      putc (side->character, output_file);
	    }
	  else
	    putc (side->character, output_file);
	  break;
	}

      /* Advance to next character.  */

      side->character = getc (side->file);
    }
}

/*-----------------------------------------.
| Copy non white space from SIDE to FILE.  |
`-----------------------------------------*/

static void
copy_word (SIDE *side, FILE *file)
{
  if (interrupted)
    longjmp (signal_label, 1);

  while (side->character != EOF && !isspace (side->character))
    {

      /* In printer mode, act according to copy_mode.  If copy_mode is not
	 COPY_NORMAL, we know that file is necessarily output_file.  */

      if (overstrike)
	switch (copy_mode)
	  {
	  case COPY_NORMAL:
	    putc (side->character, file);
	    break;

	  case COPY_DELETED:
	    putc ('_', output_file);

	    /* Avoid underlining an underscore.  */

	    if (side->character != '_')
	      {
		putc ('\b', output_file);
		putc (side->character, output_file);
	      }
	    break;
	  
	  case COPY_INSERTED:
	    putc (side->character, output_file);
	    putc ('\b', output_file);
	    putc (side->character, output_file);
	    break;
	  }
      else
	putc (side->character, file);

      side->character = getc (side->file);
    }
  side->position++;
}

/*-------------------------------------------------------------------------.
| For a given SIDE, turn original input file in another one, in which each |
| word is on one line.							   |
`-------------------------------------------------------------------------*/

static void
split_file_into_words (SIDE *side)
{
  struct stat stat_buffer;	/* for checking if file is directory */

  /* Open files.  */

  if (side->filename == NULL)
    {

      /* Select a file name, use it for opening a temporary file and
	 unlink it right away.  Then, copy the whole standard input on
	 this temporary local file.  Once done, prepare it for reading.
	 We do not need the file name itself anymore.  */

      tmpnam (side->temp_name);
      side->file = fopen (side->temp_name, "w+");
      if (side->file == NULL)
	error (EXIT_OTHER_REASON, errno, side->temp_name);
      if (unlink (side->temp_name) != 0)
	error (EXIT_OTHER_REASON, errno, side->temp_name);
      while (side->character = getchar (), side->character != EOF)
	putc (side->character, side->file);
      rewind (side->file);
    }
  else
    {

      /* Check and diagnose if the file name is a directory.  Or else,
	 prepare the file for reading.  */
	 
      if (stat (side->filename, &stat_buffer) != 0)
	error (EXIT_OTHER_REASON, errno, side->filename);
      if ((stat_buffer.st_mode & S_IFMT) == S_IFDIR)
	error (EXIT_OTHER_REASON, 0, "Directories not supported");
      side->file = fopen (side->filename, "r");
      if (side->file == NULL)
	error (EXIT_OTHER_REASON, errno, side->filename);
    }
  side->character = getc (side->file);
  side->position = 0;

  tmpnam (side->temp_name);
  side->temp_file = fopen (side->temp_name, "w");
  if (side->temp_file == NULL)
    error (EXIT_OTHER_REASON, errno, side->temp_name);

  /* Complete splitting input file into words on output.  */

  while (side->character != EOF)
    {
      if (interrupted)
	longjmp (signal_label, 1);

      skip_whitespace (side);
      if (side->character == EOF)
	break;
      copy_word (side, side->temp_file);
      putc ('\n', side->temp_file);
    }
  fclose (side->temp_file);
}

/*-------------------------------------------------------------------.
| Decode one directive line from INPUT_FILE.  The format should be:  |
| 								     |
|      ARG0 [ , ARG1 ] LETTER ARG2 [ , ARG3 ] \n		     |
| 								     |
| By default, ARG1 is assumed to have the value of ARG0, and ARG3 is |
| assumed to have the value of ARG2.  Return 0 if any error found.   |
`-------------------------------------------------------------------*/

static int
decode_directive_line (void)
{
  int value;			/* last scanned value */
  int state;			/* ordinal of number being read */
  int error;			/* if error seen */

  error = 0;
  state = 0;
  value = 0;
  while (!error && state < 4)
    {

      /* Read the next number.  ARG0 and ARG2 are mandatory.  */

      if (isdigit (character))
	{
	  value = 0;
	  while (isdigit (character))
	    {
	      value = 10 * value + character - '0';
	      character = getc (input_file);
	    }
	}
      else if (state != 1 && state != 3)
	error = 1;

      /* Assign the proper value.  */

      argument[state] = value;

      /* Skip the following character.  */

      switch (state)
	{
	case 0:
	case 2:
	  if (character == ',')
	    character = getc (input_file);
	  break;
	  
	case 1:
	  if (character == 'a' || character == 'd' || character == 'c')
	    {
	      directive = character;
	      character = getc (input_file);
	    }
	  else
	    error = 1;
	  break;

	case 3:
	  if (character != '\n')
	    error = 1;
	  break;
	}
      state++;
    }

  /* Complete reading of the line and return success value.  */

  while (character != EOF && character != '\n')
    character = getc (input_file);
  if (character == '\n')
    character = getc (input_file);

  return !error;
}  

/*----------------------------------------------.
| Skip SIDE until some word ORDINAL, included.  |
`----------------------------------------------*/

static void
skip_until_ordinal (SIDE *side, int ordinal)
{
  while (side->position < ordinal)
    {
      skip_whitespace (side);
      skip_word (side);
    }
}

/*----------------------------------------------.
| Copy SIDE until some word ORDINAL, included.  |
`----------------------------------------------*/

static void
copy_until_ordinal (SIDE *side, int ordinal)
{
  while (side->position < ordinal)
    {
      copy_whitespace (side, output_file);
      copy_word (side, output_file);
    }
}

/*-----------------------------------------------------.
| Study diff output and use it to drive reformatting.  |
`-----------------------------------------------------*/

static void
reformat_diff_output (void)
{
  int resync_left;		/* word position for left resynchronisation */
  int resync_right;		/* word position for rigth resynchronisation */

  /* Rewind input files.  */

  rewind (left_side->file);
  left_side->character = getc (left_side->file);
  left_side->position = 0;

  rewind(right_side->file);
  right_side->character = getc (right_side->file);
  right_side->position = 0;

  /* Process diff output.  */

  while (1)
    {
      if (interrupted)
	longjmp (signal_label, 1);

      /* Skip any line irrelevant to this program.  */

      while (character != EOF && !isdigit (character))
	{
	  while (character != EOF && character != '\n')
	    character = getc (input_file);
	  if (character == '\n')
	    character = getc (input_file);
	}

      /* Get out the loop if end of file.  */

      if (character == EOF)
	break;

      /* Read, decode and process one directive line.  */

      if (decode_directive_line ())
	{

	  /* Accumulate statistics about isolated or changed word counts.
	     Decide the required position on both files to resynchronize
	     them, just before obeying the directive.  Then, reposition
	     both files first, showing any needed common code along the
	     road.  Be careful to copy common code from the left side if
	     only deleted code is to be shown.  */

	  switch (directive)
	    {
	    case 'a':
	      count_isolated_right += argument[3] - argument[2] + 1;
	      resync_left = argument[0];
	      resync_right = argument[2] - 1;
	      break;

	    case 'd':
	      count_isolated_left += argument[1] - argument[0] + 1;
	      resync_left = argument[0] - 1;
	      resync_right = argument[2];
	      break;

	    case 'c':
	      count_changed_left += argument[1] - argument[0] + 1;
	      count_changed_right += argument[3] - argument[2] + 1;
	      resync_left = argument[0] - 1;
	      resync_right = argument[2] - 1;
	      break;

	    default:
	      abort ();
	    }

	  if (!inhibit_left)
	    if (!inhibit_common && inhibit_right)
	      copy_until_ordinal (left_side, resync_left);
	    else
	      skip_until_ordinal (left_side, resync_left);

	  if (!inhibit_right)
	    if (inhibit_common)
	      skip_until_ordinal (right_side, resync_right);
	    else
	      copy_until_ordinal (right_side, resync_right);

	  if (!inhibit_common && inhibit_left && inhibit_right)
	    copy_until_ordinal (right_side, resync_right);

	  /* Use separator lines to disambiguate the output.  */

	  if (inhibit_left && inhibit_right)
	    {
	      if (!inhibit_common)
		fprintf (output_file, "\n%s\n", SEPARATOR_LINE);
	    }
	  else if (inhibit_common)
	    fprintf (output_file, "\n%s\n", SEPARATOR_LINE);

	  /* Show any deleted code.  */

	  if ((directive == 'd' || directive == 'c') && !inhibit_left)
	    {
	      copy_whitespace (left_side, output_file);
	      start_of_delete ();
	      copy_word (left_side, output_file);
	      copy_until_ordinal (left_side, argument[1]);
	      end_of_delete ();
	    }

	  /* Show any inserted code, or ensure skipping over it in case the
	     right file is used merely to show common words.  */

	  if (directive == 'a' || directive == 'c')
	    if (inhibit_right)
	      {
		if (!inhibit_common && inhibit_left)
		  skip_until_ordinal (right_side, argument[3]);
	      }
	    else
	      {
		copy_whitespace (right_side, output_file);
		start_of_insert ();
		copy_word (right_side, output_file);
		copy_until_ordinal (right_side, argument[3]);
		end_of_insert ();
	      }
	}
    }

  /* Copy remainder of input.  Copy from left side if the user wanted to see
     only the common code and deleted words.  */

  if (inhibit_common)
    {
      if (!inhibit_left || !inhibit_right)
	fprintf (output_file, "\n%s\n", SEPARATOR_LINE);
    }
  else if (!inhibit_left && inhibit_right)
    {
      copy_until_ordinal (left_side, count_total_left);
      copy_whitespace (left_side, output_file);
    }
  else
    {
      copy_until_ordinal (right_side, count_total_right);
      copy_whitespace (right_side, output_file);
    }

  /* Close input files.  */

  fclose (left_side->file);
  fclose (right_side->file);
}


/* Launch and complete various programs.  */

/*-------------------------.
| Lauch the diff program.  |
`-------------------------*/

static void
launch_input_program (void)
{
  /* Launch the diff program.  */

  if (ignore_case)
    input_file = readpipe (DIFF_PROGRAM, "-c", left_side->temp_name,
			   right_side->temp_name, NULL);
  else
    input_file = readpipe (DIFF_PROGRAM, left_side->temp_name,
			   right_side->temp_name, NULL);
  if (!input_file)
    error (EXIT_OTHER_REASON, errno, DIFF_PROGRAM);
  character = getc (input_file);
}

/*----------------------------.
| Complete the diff program.  |
`----------------------------*/

static void
complete_input_program (void)
{
  fclose (input_file);
  wait (NULL);
}

/*---------------------------------.
| Launch the output pager if any.  |
`---------------------------------*/

static void
launch_output_program (void)
{
  char *program;		/* name of the pager */
  char *basename;		/* basename of the pager */

  /* Check if a output program should be called, and which one.  Avoid
     all paging if only statistics are needed.  */

  if (autopager && isatty (fileno (stdout))
      && !(inhibit_left && inhibit_right && inhibit_common))
    {
      program = getenv ("PAGER");
#ifdef PAGER_PROGRAM
      if (program == NULL)
	program = PAGER_PROGRAM;
#endif
    }
  else
    program = NULL;

  /* Use stdout as default output.  */

  output_file = stdout;

  /* Ensure the termcap initialization string is sent to stdout right
     away, never to the pager.  */

#if HAVE_TPUTS
  if (termcap_init_string)
    {
      tputs (termcap_init_string, 0, putc_for_tputs);
      fflush (stdout);
    }
#endif

  /* If we should use a pager, launch it.  */

  if (program && *program)
    {
      output_file = writepipe (program, NULL);
      if (!output_file)
	error (EXIT_OTHER_REASON, errno, program);

      /* If we are paging to less, use printer mode, not display mode.  */

      if (basename = strrchr ("less", '/'), basename)
	basename++;
      else
	basename = program;

      if (strstr (basename, "less"))
	{
	  find_termcap = 0;
	  overstrike = 1;
	  overstrike_for_less = 1;
	}
    }
}

/*-----------------------------.
| Complete the pager program.  |
`-----------------------------*/

static void
complete_output_program (void)
{

  /* Complete any pending emphasis mode.  This would be necessary only if
     some signal interrupts the normal operation of the program.  */

  switch (copy_mode)
    {
    case COPY_DELETED:
      end_of_delete ();
      break;

    case COPY_INSERTED:
      end_of_insert ();
      break;
    
    case COPY_NORMAL:
      break;

    default:
      abort ();
    }

  /* Let the user play at will inside the pager, until s/he exits, before
     proceeding any further.  */

  if (output_file && output_file != stdout)
    {
      fclose (output_file);
      wait (NULL);
    }

  /* Ensure the termcap termination string is sent to stdout, never to
     the pager.  Moreover, the pager has terminated already.  */

#if HAVE_TPUTS
  if (termcap_end_string)
    {
      output_file = stdout;
      tputs (termcap_end_string, 0, putc_for_tputs);
    }
#endif
}

/*-------------------------------.
| Print accumulated statistics.	 |
`-------------------------------*/

static void
print_statistics (void)
{
  int count_common_left;	/* words unchanged in left file */
  int count_common_right;	/* words unchanged in right file */

  count_common_left
    = count_total_left - count_isolated_left - count_changed_left;
  count_common_right
    = count_total_right - count_isolated_right - count_changed_right;

  printf ("%s: %d words", left_side->filename, count_total_left);
  if (count_total_left > 0)
    {
      printf ("  %d %d%% common", count_common_left,
	      count_common_left * 100 / count_total_left);
      printf ("  %d %d%% deleted", count_isolated_left,
	      count_isolated_left * 100 / count_total_left);
      printf ("  %d %d%% changed", count_changed_left,
	      count_changed_left * 100 / count_total_left);
    }
  printf ("\n");

  printf ("%s: %d words", right_side->filename, count_total_right);
  if (count_total_right > 0)
    {
      printf ("  %d %d%% common", count_common_right,
	      count_common_right * 100 / count_total_right);
      printf ("  %d %d%% inserted", count_isolated_right,
	      count_isolated_right * 100 / count_total_right);
      printf ("  %d %d%% changed", count_changed_right,
	      count_changed_right * 100 / count_total_right);
    }
  printf ("\n");
}


/* Main control.  */

/*-----------------------------------.
| Prints a more detailed Copyright.  |
`-----------------------------------*/

static void
print_copyright (void)
{
  fprintf (stderr, "\
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
along with this program; if not, write to the Free Software\n\
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.\n\
\n");
}

/*-----------------------------.
| Tell how to use, then exit.  |
`-----------------------------*/

static void
usage (int status)
{
  if (status != 0)
    fprintf (stderr, "Try `%s --help' for more information.\n", program_name);
  else
    {
      printf ("\
Usage: %s [OPTION]... FILE1 FILE2\n", program_name);
      printf ("\
Mandatory arguments to long options are mandatory for short options too.\n\
\n\
  -C, --copyright            print Copyright then exit\n\
  -V, --version              print program version then exit\n\
  -1, --no-deleted           inhibit output of deleted words\n\
  -2, --no-inserted          inhibit output of inserted words\n\
  -3, --no-common            inhibit output of common words\n\
  -a, --auto-pager           automatically calls a pager\n\
  -h, --help                 print this help\n\
  -i, --ignore-case          fold character case while comparing\n\
  -l, --less-mode            variation of printer mode for \"less\"\n\
  -n, --avoid-wraps          do not extend fields through newlines\n\
  -p, --printer              overstrike as for printers\n\
  -s, --statistics           say how many words deleted, inserted etc.\n\
  -t, --terminal             use termcap as for terminal displays\n\
  -w, --start-delete=STRING  string to mark beginning of delete region\n\
  -x, --end-delete=STRING    string to mark end of delete region\n\
  -y, --start-insert=STRING  string to mark beginning of insert region\n\
  -z, --end-insert=STRING    string to mark end of insert region\n");
    }
  exit (status);
}

/*---------------.
| Main program.	 |
`---------------*/

void
main (int argc, char *const argv[])
{
  int option_char;		/* option character */

  /* Decode arguments.  */

  program_name = argv[0];

  inhibit_left = 0;
  inhibit_right = 0;
  inhibit_common = 0;

  ignore_case = 0;
  show_statistics = 0;
  no_wrapping = 0;
  autopager = 0;
  overstrike = 0;
  overstrike_for_less = 0;
  user_delete_start = NULL;
  user_delete_end = NULL;
  user_insert_start = NULL;
  user_insert_end = NULL;

  find_termcap = -1;		/* undecided yet */
  term_delete_start = NULL;
  term_delete_end = NULL;
  term_insert_start = NULL;
  term_insert_end = NULL;
  copy_mode = COPY_NORMAL;

  count_total_left = 0;
  count_total_right = 0;
  count_isolated_left = 0;
  count_isolated_right = 0;
  count_changed_left = 0;
  count_changed_right = 0;

  while (option_char = getopt_long (argc, argv, "123CVahidlnpstw:x:y:z:",
				    longopts, NULL),
	 option_char != EOF)
    switch (option_char)
      {
      case '1':
	inhibit_left = 1;
	break;

      case '2':
	inhibit_right = 1;
	break;

      case '3':
	inhibit_common = 1;
	break;

      case 'C':
	print_copyright ();
	exit (0);

      case 'v':
	printf ("GNU %s %s\n%s\n", PRODUCT, VERSION, copyright);
	exit (0);

      case 'a':
	autopager = 1;
	break;

      case 'h':
	usage (0);

      case 'i':
	ignore_case = 1;
	break;

      case 'l':
	if (find_termcap < 0)
	  find_termcap = 0;
	overstrike = 1;
	overstrike_for_less = 1;
	break;

      case 'n':
	no_wrapping = 1;
	break;

      case 'p':
	overstrike = 1;
	break;

      case 's':
	show_statistics = 1;
	break;

      case 't':
#if HAVE_TPUTS
	if (find_termcap < 0)
	  find_termcap = 1;
	break;
#else
	error (EXIT_OTHER_REASON, 0, "Cannot use -t, termcap not available.");
#endif

      case 'w':
	user_delete_start = optarg;
	break;

      case 'x':
	user_delete_end = optarg;
	break;

      case 'y':
	user_insert_start = optarg;
	break;

      case 'z':
	user_insert_end = optarg;
	break;

      default:
	usage (EXIT_OTHER_REASON);
      }

  if (optind + 2 != argc)
    usage (EXIT_OTHER_REASON);

  /* If find_termcap still undecided, consider it unset.  However, set if
     autopager is set while stdout is directed to a terminal, but this
     decision will be reversed later if the pager happens to be "less".  */

  if (find_termcap < 0)
    find_termcap = autopager && isatty (fileno (stdout));

  /* Setup file names and signals, then do it all.  */

  if (strcmp (argv[optind], "") == 0 || strcmp (argv[optind], "-") == 0)
    left_side->filename = NULL;
  else
    left_side->filename = argv[optind];
  optind++;
  *left_side->temp_name = '\0';

  if (strcmp (argv[optind], "") == 0 || strcmp (argv[optind], "-") == 0)
    right_side->filename = NULL;
  else
    right_side->filename = argv[optind];
  optind++;
  *right_side->temp_name = '\0';

  if (left_side->filename == NULL && right_side->filename == NULL)
    error (EXIT_OTHER_REASON, 0, "Both files cannot be standard input.");

  setup_signals ();
  input_file = NULL;
  output_file = NULL;
  termcap_init_string = NULL;
  termcap_end_string = NULL;

  if (!setjmp (signal_label))
    {
      split_file_into_words (left_side);
      count_total_left = left_side->position;
      split_file_into_words (right_side);
      count_total_right = right_side->position;
      launch_input_program ();
      launch_output_program ();
      initialize_strings ();
      reformat_diff_output ();
      fclose (input_file);
    }

  /* Clean up.  Beware that input_file and output_file might not exist,
     if a signal occurred early in the program.  */

  if (input_file)
    complete_input_program ();

  if (*left_side->temp_name)
    unlink (left_side->temp_name);
  if (*right_side->temp_name)
    unlink (right_side->temp_name);

  if (output_file)
    complete_output_program ();

  if (interrupted)
    exit (EXIT_OTHER_REASON);

  if (show_statistics)
    print_statistics ();

  if (count_isolated_left || count_isolated_right
      || count_changed_left || count_changed_right)
    exit (EXIT_ANY_DIFFERENCE);

  exit (EXIT_NO_DIFFERENCES);
}
