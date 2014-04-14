/* Find similar sequences in multiple files and report differences.
   Copyright (C) 1992, 1997, 1998, 1999, 2011 Free Software Foundation, Inc.
   Francois Pinard <pinard@iro.umontreal.ca>, 1997.

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

/* Define to 1 to compile in a debugging option.  */
#define DEBUGGING 1

/* Define to the printed name of standard input.  */
#define STDIN_PRINTED_NAME "<stdin>"

/* Define to 1 for requesting the capability of doing full comparisons, then
   not necessarily relying on checksums (both methods are available).  If 0
   instead, always rely on checksums and pack them with item types into the
   item structure: this yields faster code and disallows paranoid mode.  */
#define SAFER_SLOWER 0

/* One may also, optionally, define a default PAGER_PROGRAM.  This
   might be done using the --with-default-pager=PAGER configure
   switch.  If PAGER_PROGRAM is undefined and neither the WDIFF_PAGER
   nor the PAGER environment variable is set, none will be used.  */

/* We do termcap init ourselves, so pass -X.
   We might do coloring, so pass -R. */
#define LESS_DEFAULT_OPTS "-X -R"

/*-----------------------.
| Library declarations.  |
`-----------------------*/

#if DEBUGGING
# include <assert.h>
#else
# define assert (Expr)
#endif

#include <ctype.h>
#include <string.h>

char *strstr ();

#if HAVE_TPUTS
# if HAVE_TERMCAP_H
#  include <termcap.h>
# else
#  if HAVE_TERMLIB_H
#   include <termlib.h>
#  else
#   if HAVE_CURSES_H
#    include <curses.h>
#   else
#    if HAVE_NCURSES_H
#     include <ncurses.h>
#    endif
#   endif
#   if HAVE_TERM_H
#    include <term.h>
#   endif
#  endif
# endif
#endif

#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <getopt.h>
#include <locale.h>
#include <sys/wait.h>

#include "regex.h"
#define CHAR_SET_SIZE 256

#define MASK(Length) (~(~0 << (Length)))

/*---------------------.
| Local declarations.  |
`---------------------*/

/* Exit codes values.  */
#define EXIT_DIFFERENCE 1	/* some differences found */
#define EXIT_ERROR 2		/* any other reason for exit */

/* Define pseudo short options for long options without short options.  */
#define GTYPE_GROUP_FORMAT_OPTION	10
#define HORIZON_LINES_OPTION		11
#define LEFT_COLUMN_OPTION		12
#define LINE_FORMAT_OPTION		13
#define LTYPE_LINE_FORMAT_OPTION	14
#define SUPPRESS_COMMON_LINES_OPTION	15
#define TOLERANCE_OPTION		16

/* The name this program was run with. */
const char *program_name;

/* If nonzero, display usage information and exit.  */
static int show_help = 0;

/* If nonzero, print the version on standard output and exit.  */
static int show_version = 0;

/* Other variables.  */

FILE *output_file;		/* file or pipe to which we write output */
int exit_status = EXIT_SUCCESS;	/* status at end of execution */

/* Options variables.  */

#define OPTION_STRING \
  "0123ABC::D:F:GHI:J:KL:NO:PQ:RS:TU::VWX:Y:Z:abc::dehijklmnopqrs:tu::vw:xyz"
/*      BC: D:F: HI:   L:N  P   S:TU:   X:    abc  dehi  l n pqrs:tu  vw:xy  */
/* The line above gives GNU diff options, for reference.  */

/* Long options equivalences.  */
static const struct option long_options[] = {
  {"auto-pager", no_argument, NULL, 'A'},
  {"avoid-wraps", no_argument, NULL, 'm'},
  {"brief", no_argument, NULL, 'q'},
  {"context", optional_argument, NULL, 'c'},
  {"debugging", no_argument, NULL, '0'},
  {"ed", no_argument, NULL, 'e'},
  {"exclude-from", required_argument, NULL, 'X'},
  {"exclude", required_argument, NULL, 'x'},
  {"expand-tabs", no_argument, NULL, 't'},
  {"GTYPE-group-format", required_argument, NULL, GTYPE_GROUP_FORMAT_OPTION},
  {"help", no_argument, &show_help, 1},
  {"horizon-lines", required_argument, NULL, HORIZON_LINES_OPTION},
  {"ifdef", required_argument, NULL, 'D'},
  {"ignore-all-space", no_argument, NULL, 'w'},
  {"ignore-blank-lines", no_argument, NULL, 'B'},
  {"ignore-case", no_argument, NULL, 'i'},
  {"ignore-matching-lines", required_argument, NULL, 'I'},
  {"ignore-space-change", no_argument, NULL, 'b'},
  {"initial-tab", no_argument, NULL, 'T'},
  {"item-regexp", required_argument, NULL, 'O'},
  {"label", required_argument, NULL, 'L'},
  {"left-column", no_argument, NULL, LEFT_COLUMN_OPTION},
  {"less-mode", no_argument, NULL, 'k'},
  {"line-format", required_argument, NULL, LINE_FORMAT_OPTION},
  {"LTYPE-line-format", required_argument, NULL, LTYPE_LINE_FORMAT_OPTION},
  {"minimal", no_argument, NULL, 'd'},
  {"minimum-size", required_argument, NULL, 'J'},
  {"new-file", no_argument, NULL, 'N'},
  {"no-common", no_argument, NULL, '3'},
  {"no-deleted", no_argument, NULL, '1'},
  {"no-init-term", no_argument, NULL, 'K'},	/* backwards compatibility */
  {"no-inserted", no_argument, NULL, '2'},
  {"ignore-delimiters", no_argument, NULL, 'j'},
  {"paginate", no_argument, NULL, 'l'},
  {"printer", no_argument, NULL, 'o'},
  {"rcs", no_argument, NULL, 'n'},
  {"recursive", no_argument, NULL, 'r'},
  {"relist-files", no_argument, NULL, 'G'},
  {"report-identical-files", no_argument, NULL, 's'},
  {"show-c-function", no_argument, NULL, 'p'},
  {"show-function-line", required_argument, NULL, 'F'},
  {"show-links", no_argument, NULL, 'R'},
  {"side-by-side", no_argument, NULL, 'y'},
  {"speed-large-files", no_argument, NULL, 'H'},
  {"starting-file", required_argument, NULL, 'S'},
  {"string", optional_argument, NULL, 'Z'},
  {"suppress-common-lines", no_argument, NULL, SUPPRESS_COMMON_LINES_OPTION},
  {"terminal", no_argument, NULL, 'z'},
  {"text", no_argument, NULL, 'a'},
  {"tolerance", required_argument, NULL, TOLERANCE_OPTION},
  {"unidirectional-new-file", no_argument, NULL, 'P'},
  {"unified", optional_argument, NULL, 'u'},
  {"verbose", no_argument, NULL, 'v'},
  {"version", no_argument, &show_version, 1},
  {"width", required_argument, NULL, 'w'},
  {"word-mode", required_argument, NULL, 'W'},
  {0, 0, 0, 0},
};

/*--------------------.
| Available options.  |
`--------------------*/

/* If calling the pager automatically.  */
static int autopager = 0;

/* End/restart strings at end of lines.  */
static int avoid_wraps = 0;

/* If nonzero, output quite verbose detail of operations.  */
static int debugging = 0;

/* Initialize the termcap strings.  */
static int find_termcap = -1;	/* undecided yet */

/* Ignore all white space.  */
static int ignore_all_space = 0;

/* Ignore changes whose lines are all blank.  */
static int ignore_blank_lines = 0;

/* Consider upper-case and lower-case to be the same.  */
static int ignore_case = 0;

/* If nonzero, segregate matchable only items out of normal items.  */
static int ignore_delimiters = 0;

/* Ignore changes whose lines all match regular expression.  */
static struct re_pattern_buffer **ignore_regexp_array = NULL;
static int ignore_regexps = 0;

/* Ignore changes in the amount of white space.  */
static int ignore_space_change = 0;

/* Make tabs line up by prepending a tab.  */
static int initial_tab = 0;

/* Try hard to find a smaller set of changes.  Unimplemented.  */
static int minimal = 0;

/* Minimum size of a cluster, not counting ignored items.  */
static int minimum_size = -1;	/* undecided yet */

/* If using printer overstrikes.  */
static int overstrike = 0;

/* If output aimed to the "less" program (overstrike even whitespace).  */
static int overstrike_for_less = 0;

/* Pass the output through `pr' to paginate it.  */
static int paginate = 0;

/* List all input files with annotations.  */
static int relist_files = 0;

/* Give file and line references in annotated listings.  */
static int show_links = 0;

/* Assume large files and many scattered small changes.  Unimplemented.  */
static int speed_large_files = 0;

/* If nonzero, show progress of operations.  */
static int verbose = 0;

/* Compare words, use REGEXP to define item.  */
static int word_mode = 0;
static struct re_pattern_buffer *item_regexp = NULL;

/*---------------.
| diff options.  |
`---------------*/

/* Treat all files as text.  */
static int text = 0;

/* Output regular context diffs.  */
static int context = 0;

/* Output unified context diffs or unidiffs.  */
static int unified = 0;

/* Use LABEL instead of file name.  */
static const char *label = NULL;

/* Show which C function each change is in.  */
static int show_c_function = 0;

/* Show the most recent line matching RE.  */
static const char *show_function_line = NULL;

/* Output only whether files differ.  */
static int brief = 0;

/* Output an ed script.  */
static int ed = 0;

/* Output an RCS format diff.  */
static int rcs = 0;

/* Output in two columns.  */
static int side_by_side = 0;

/* Output at most NUM characters per line.  */
static int width = 130;

/* Output only the left column of common lines.  */
static int left_column = 0;

/* Do not output common lines.  */
static int suppress_common_lines = 0;

/* Output merged file to show `#ifdef NAME' diffs.  */
static const char *ifdef = NULL;

/* GTYPE input groups with GFMT.  */
static const char *gtype_group_format = NULL;

/* All input lines with LFMT.  */
static const char *line_format = NULL;

/* LTYPE input lines with LFMT.  */
static const char *ltype_line_format = NULL;

/* Expand tabs to spaces in output.  */
static int expand_tabs = 0;

/* Recursively compare any subdirectories found.  */
static int recursive = 0;

/* Treat absent files as empty.  */
static int new_file = 0;

/* Treat absent first files as empty.  */
static int unidirectional_new_file = 0;

/* Report when two files are the same.  */
static int report_identical_files = 0;

/* Exclude files that match PAT.  */
static const char *exclude = NULL;

/* Exclude files that match any pattern in FILE.  */
static const char *exclude_from = NULL;

/* Start with FILE when comparing directories.  */
static const char *starting_file = NULL;

/* Keep NUM lines of the common prefix and suffix.  */
static int horizon_lines = 2;

/*----------------------.
| mdiff draft options.  |
`----------------------*/

/* Maximum number of non-matching items for cluster members.  */
static int tolerance = 0;

/* Regular expressions.  */

/*-------------------------------------------------------------------.
| Compile the regex represented by STRING, diagnose and abort if any |
| error.  Returns the compiled regex structure.			     |
`-------------------------------------------------------------------*/

static struct re_pattern_buffer *
alloc_and_compile_regex (const char *string)
{
  struct re_pattern_buffer *pattern;	/* newly allocated structure */
  const char *message;		/* error message returned by regex.c */

  pattern = (struct re_pattern_buffer *)
    xmalloc (sizeof (struct re_pattern_buffer));
  memset (pattern, 0, sizeof (struct re_pattern_buffer));

  pattern->buffer = NULL;
  pattern->allocated = 0;
#if 0
  /* FIXME */
  pattern->translate = ignore_case ? (char *) folded_chars : NULL;
#endif

  pattern->fastmap = xmalloc ((size_t) CHAR_SET_SIZE);

  message = re_compile_pattern (string, (int) strlen (string), pattern);
  if (message)
    error (EXIT_ERROR, 0, _("%s (for regexp `%s')"), message, string);

  /* The fastmap should be compiled before `re_match'.  This would not be
     necessary if `re_search' was called first.  */

  re_compile_fastmap (pattern);

#if WITH_REGEX

  /* Do not waste extra allocated space.  */

  if (pattern->allocated > pattern->used)
    {
      pattern->buffer = xrealloc (pattern->buffer, (size_t) pattern->used);
      pattern->allocated = pattern->used;
    }
#endif

  return pattern;
}

/* Items.  */

/* Each item has a type which is meaningful to the clustering process.
   NORMAL items and DELIMS items fully participate in cluster
   construction, yet only NORMAL items are worth participating into building
   the minimum size count, merely DELIMS items do not.  WHITE items, even if
   they do exist in input files, are a little like if they were just not
   there: they are often skipped over.  SENTINEL items do not correspond to
   any real input item, they are used to mark file boundaries, and may not
   compare equally to any other item.

   The names NORMAL, DELIMS, WHITE and SENTINEL are a bit generic and are
   used mainly to trigger the imagination of the reader.  Practice may be
   quite different.  For example, actual white items may well be classified
   as DELIMS or NORMAL, items having only delimiters may be seen as NORMAL,
   items making up a block comment may be considered as WHITE.  Moreover,
   SENTINEL items could be introduced at function boundaries, say, if we
   wanted to limit cluster members to a single function, etc.  */

enum type
{
  NORMAL,			/* meaningful, countable items */
  DELIMS,			/* items having only delimiters */
  WHITE,			/* white items */
  SENTINEL			/* file boundaries */
};

/* Comparison work is done on checksums of items, rather than input items
   themselves.  The following array maintains one entry for each item,
   whether a real item or a sentinel.  Checksums, which are meaningful only
   for NORMAL or DELIMS items, are computed so they ignore parts of the item
   which are not really meaningful for the purpose of comparisons.

   If SAFER_SLOWER has been selected, checksums may be avoided and pointers
   used instead.  For pointers to occupy the full space they might need, the
   type bits are kept apart but tightly packed.  Otherwise, the TYPE bits
   are part of the structured item, eating out some chekcsum space.  */

#define BITS_PER_CHAR		8
#define BITS_PER_TYPE		2
#define BITS_PER_WORD		(sizeof (unsigned) * BITS_PER_CHAR)
#define TYPES_PER_WORD		(BITS_PER_WORD / BITS_PER_TYPE)

#if SAFER_SLOWER

# define ITEM			union item

union item
{
  unsigned checksum;
  char *pointer;
};

static union item *item_array = NULL;
static unsigned *type_array = NULL;
static int items = 0;

static inline enum type
item_type (ITEM * item)
{
  int position = item - item_array;
  int shift = position % TYPES_PER_WORD * BITS_PER_TYPE;
  unsigned *pointer = type_array + position / TYPES_PER_WORD;

  return (enum type) (MASK (BITS_PER_TYPE) & *pointer >> shift);
}

static inline void
set_item_type (ITEM * item, enum type type)
{
  int position = item - item_array;
  int shift = position % TYPES_PER_WORD * BITS_PER_TYPE;
  unsigned *pointer = type_array + position / TYPES_PER_WORD;

  *pointer &= ~MASK (BITS_PER_TYPE) << shift;
  *pointer |= type << shift;
}

#else /* not SAFER_SLOWER */

# define ITEM			struct item

struct item
{
  enum type type:BITS_PER_TYPE;
  unsigned checksum:BITS_PER_WORD - BITS_PER_TYPE;
};

static struct item *item_array = NULL;
static int items = 0;

static inline enum type
item_type (ITEM * item)
{
  return item->type;
}

static inline void
set_item_type (ITEM * item, enum type type)
{
  item->type = type;
}

#endif /* not SAFER_SLOWER */

#if DEBUGGING

/*---------------------.
| Dump a given INPUT.  |
`---------------------*/

static void
dump_item (ITEM * item)
{
  static const char *item_type_string[4] = { "", "delim", "white", "SENTIN" };

  fprintf (stderr, "(%ld)\t%s\t%08x\n", (long) (item - item_array),
	   item_type_string[item_type (item)], item->checksum);
}

/*------------------.
| Dump all items.  |
`------------------*/

static void
dump_all_items (void)
{
  int counter;

  fputs ("\f\n", stderr);
  for (counter = 0; counter < items; counter++)
    dump_item (item_array + counter);
}

#endif /* DEBUGGING */

/*-------------------------------------------------------------------.
| Move item pointer forward or backward, skipping over white items.  |
`-------------------------------------------------------------------*/

#define FORWARD_ITEM(Pointer) \
  do { Pointer++; } while (item_type (Pointer) == WHITE)

#define BACKWARD_ITEM(Pointer) \
  do { Pointer--; } while (item_type (Pointer) == WHITE)

/*------------------------------------------------------------------.
| Sort helper.  Compare two item indices.  Lower indices go first.  |
`------------------------------------------------------------------*/

static int
compare_for_positions (const void *void_first, const void *void_second)
{
#define value1 *((int *) void_first)
#define value2 *((int *) void_second)

  return value1 - value2;

#undef value1
#undef value2
}

/*-------------------------------------------------------------------------.
| Sort helper.  Compare two item indices.  Items having identical	   |
| checksums should be brought together.  Whenever two checksums are equal, |
| items are then ordered by the checksum of the following items, and this  |
| way going forward until a difference is found.			   |
`-------------------------------------------------------------------------*/

static int
compare_for_checksum_runs (const void *void_first, const void *void_second)
{
#define value1 *((int *) void_first)
#define value2 *((int *) void_second)

  ITEM *item1 = item_array + value1;
  ITEM *item2 = item_array + value2;
  int result;

  while (1)
    {
      /* Order checksums.  */

      result = item1->checksum - item2->checksum;
      if (result)
	return result;

      /* Seek forward for a difference.  */

      FORWARD_ITEM (item1);
      FORWARD_ITEM (item2);

      /* Sentinels are never equal (unless really the same).  They go after
         all checksums, and compare so to stabilise the sort.  */

      if (item_type (item1) == SENTINEL)
	return item_type (item2) == SENTINEL ? value1 - value2 : 1;

      if (item_type (item2) == SENTINEL)
	return -1;
    }

#undef value1
#undef value2
}

/*-----------------.
| Add a new item.  |
`-----------------*/

static inline void
new_item (enum type type, int checksum)
{
  ITEM *item;

  if (items % (64 * TYPES_PER_WORD) == 0)
    {
      item_array = (ITEM *)
	xrealloc (item_array, (items + 64 * TYPES_PER_WORD) * sizeof (ITEM));
#if SAFER_SLOWER
      type_array = (unsigned *)
	xrealloc (type_array,
		  (items / TYPES_PER_WORD + 64) * sizeof (unsigned));
#endif
    }

  item = item_array + items++;
  set_item_type (item, type);
  item->checksum = checksum;
}

/*---------------------.
| Add a new sentinel.  |
`---------------------*/

static void
new_sentinel (void)
{
  new_item (SENTINEL, 0);
}

/*------------------------------------------------------------------------.
| Find how many identical normal items follow, given two start items.	  |
| Ensure matchable items correspond to each other, but skip over white	  |
| items.  The starting items may not be sentinels nor white items.  Fuzzy |
| items, if any, ought to be embedded and correspond to each other.	  |
`------------------------------------------------------------------------*/

static int
identical_size (int index1, int index2)
{
  ITEM *item1 = item_array + index1;
  ITEM *item2 = item_array + index2;
  int normal_count = 0;
  int mismatch_count = 0;

  while (1)
    {
      if (item1->checksum == item2->checksum
#if 0
	  /* Just assume that identical checksums imply identical types.  */
	  && item_type (item1) == item_type (item2)
#endif
	)
	{
	  if (item_type (item1) == NORMAL)
	    normal_count++;
	}
      else if (normal_count > 0 && mismatch_count < tolerance)
	{
	  if (item_type (item1) == NORMAL && item_type (item2) == NORMAL)
	    normal_count++;
	  mismatch_count++;
	}
      else
	break;

      FORWARD_ITEM (item1);
      FORWARD_ITEM (item2);

      if (item_type (item1) == SENTINEL || item_type (item2) == SENTINEL)
	break;
    }

  return normal_count;
}

/*-----------------------------------------------------------------------.
| Count normal items from START to FINISH (excluded).  However, if there |
| are MAXIMUM or more normal input items, return a negative number.      |
`-----------------------------------------------------------------------*/

static int
item_distance (int start, int finish, int maximum)
{
  int counter = 0;
  ITEM *item;

  assert (start < finish);
  assert (maximum > 0);

  for (item = item_array + start; item < item_array + finish; item++)
    if (item_type (item) == NORMAL)
      {
	counter++;
	if (counter == maximum)
	  return -1;
      }

  return counter;
}

/* Files.  */

/* All input files use a single continuous item counting scheme.  To revert
   item numbers back to actual file and item references, the following array
   maintains the cumulated number items seen prior to the beginning of each
   file, for each file given as argument to the program.  To ease some
   algorithms, a few sentinel items exist: one before the first file, one
   between each file, and one after the last file.  Each file counts all
   sentinels preceding it.  */

struct input
{
  const char *file_name;	/* name of input file */
  struct stat stat_buffer;	/* stat information for the file */
  char nick_name[4];		/* short name of the file */
  FILE *file;			/* file being read */
  char *memory_copy;		/* buffer containing the file, or NULL */

  /* Reading the file, one line or one character at a time.  */
  char *line;			/* line from file */
  char *cursor;			/* cursor into line, or NULL at end of file */
  char *limit;			/* limit value for cursor */
  size_t line_allocated;	/* allocated length of line */

  /* Rescanning of the file, one item at a time.  */
  int first_item;		/* index of first item in this file */
  int item_limit;		/* one past last item index in this file */
  int item;			/* index of next item to consider */

  /* Merging views.  */
  int *indirect_cursor;		/* cursor into indirect_array */
  int *indirect_limit;		/* one past limit value of indirect_cursor */

  /* Output control.  */
  short listing_allowed;	/* if difference output allowed */
  const char *user_start;	/* user string before difference */
  const char *user_stop;	/* user string after difference */
  const char *term_start;	/* terminal string before difference */
  const char *term_stop;	/* terminal string after difference */
};

static struct input *input_array = NULL;
static int inputs = 0;

int common_listing_allowed;	/* if common output allowed */

/* Convenience macros.  */

#define left (input_array + 0)
#define right (input_array + 1)

#define INPUT_MEMBER(Input) \
  (member_array + *(Input)->indirect_cursor)

#define INPUT_CLUSTER(Input) \
  (cluster_array + member_array[*(Input)->indirect_cursor].cluster_number)

#if DEBUGGING

/*---------------------.
| Dump a given INPUT.  |
`---------------------*/

static void
dump_input (struct input *input)
{
  fprintf (stderr, "%s1,%d\t%7d\t%s\n",
	   input->nick_name, input->item_limit - input->first_item,
	   input->first_item, input->file_name);
}

/*------------------.
| Dump all inputs.  |
`------------------*/

static void
dump_all_inputs (void)
{
  int counter;

  fputs ("\f\n", stderr);
  for (counter = 0; counter < inputs; counter++)
    dump_input (input_array + counter);
}

#endif /* DEBUGGING */

/*---------------------------------------------------------------------.
| Swallow the whole INPUT into a contiguous region of memory.  In the  |
| INPUT structure, file name and stat buffer are already initialised.  |
`---------------------------------------------------------------------*/

/* Define to reallocation increment when swallowing input.  */
#define SWALLOW_BUFFER_STEP 20000

static void
swallow_input (struct input *input)
{
  int handle;			/* file descriptor number */
  size_t allocated_length;	/* allocated length of memory buffer */
  size_t length;		/* total length read so far */
  int read_length;		/* number of character gotten on last read */

  /* Standard input is already opened.  In all other cases, open the file
     from its name.  */

  if (strcmp (input->file_name, STDIN_PRINTED_NAME) == 0)
    handle = fileno (stdin);
  else if (handle = open (input->file_name, O_RDONLY), handle < 0)
    error (EXIT_ERROR, errno, "%s", input->file_name);

  /* If the file is a plain, regular file, allocate the memory buffer all at
     once and swallow the file in one blow.  In other cases, read the file
     repeatedly in smaller chunks until we have it all, reallocating memory
     once in a while, as we go.  */

#if MSDOS

  /* On MSDOS, we cannot predict in memory size from file size, because of
     end of line conversions.  */

#else

  if (S_ISREG (input->stat_buffer.st_mode))
    {
      input->memory_copy = (char *)
	xmalloc ((size_t) input->stat_buffer.st_size);

      if (read
	  (handle, input->memory_copy,
	   (size_t) input->stat_buffer.st_size) != input->stat_buffer.st_size)
	error (EXIT_ERROR, errno, "%s", input->file_name);
    }
  else
#endif

    {
      input->memory_copy = xmalloc ((size_t) SWALLOW_BUFFER_STEP);
      allocated_length = SWALLOW_BUFFER_STEP;

      length = 0;
      while (read_length = read (handle, input->memory_copy + length,
				 allocated_length - length), read_length > 0)
	{
	  length += read_length;
	  if (length == allocated_length)
	    {
	      allocated_length += SWALLOW_BUFFER_STEP;
	      input->memory_copy = (char *)
		xrealloc (input->memory_copy, allocated_length);
	    }
	}

      if (read_length < 0)
	error (EXIT_ERROR, errno, "%s", input->file_name);
    }

  /* Close the file, but only if it was not the standard input.  */

  if (handle != fileno (stdin))
    close (handle);
}

/*-------------------------------------------.
| Register a file NAME to be later studied.  |
`-------------------------------------------*/

static void
new_input (const char *name)
{
  static int stdin_swallowed = 0;
  struct input *input;

  /* Add a new input file descriptor, read in stdin right away.  */

  if (inputs % 16 == 0)
    input_array = (struct input *)
      xrealloc (input_array, (inputs + 16) * sizeof (struct input));

  input = input_array + inputs++;
  if (strcmp (name, "") == 0 || strcmp (name, "-") == 0)
    if (stdin_swallowed)
      error (EXIT_ERROR, 0, _("only one file may be standard input"));
    else
      {
	input->file_name = STDIN_PRINTED_NAME;
	if (fstat (fileno (stdin), &input->stat_buffer) != 0)
	  error (EXIT_ERROR, errno, "%s", input->file_name);
	swallow_input (input);
	stdin_swallowed = 1;
      }
  else
    {
      input->file_name = name;
      if (stat (input->file_name, &input->stat_buffer) != 0)
	error (EXIT_ERROR, errno, "%s", input->file_name);
      if ((input->stat_buffer.st_mode & S_IFMT) == S_IFDIR)
	error (EXIT_ERROR, 0, _("directories not supported"));
      input->memory_copy = NULL;
    }
}

/*-------------------------------.
| Prepare INPUT for re-reading.  |
`-------------------------------*/

static void
open_input (struct input *input)
{
  if (input->memory_copy)
    input->limit = input->memory_copy;
  else
    {
      if (input->file = fopen (input->file_name, "r"), !input->file)
	error (EXIT_ERROR, errno, "%s", input->file_name);

      input->line = NULL;
    }
  input->line_allocated = 0;
}

static void
close_input (struct input *input)
{
  if (input->line_allocated > 0)
    free (input->line);

  if (!input->memory_copy)
    fclose (input->file);
}

/*-------------------------------------------------------------------------.
| Prepare INPUT for next line, then available from INPUT->LINE through     |
| INPUT->LIMIT (excluded).  INPUT->CURSOR will point to first character of |
| the line, or will be NULL at end of file.                                |
`-------------------------------------------------------------------------*/

static void
input_line (struct input *input)
{
  if (input->memory_copy)
    {
      const char *limit = input->memory_copy + input->stat_buffer.st_size;
      char *cursor;

      input->line = input->limit;

      for (cursor = input->line; cursor < limit && *cursor != '\n'; cursor++)
	;

      if (cursor < limit)
	{
	  cursor++;
	  input->cursor = input->line;
	}
      else if (cursor == input->line)
	input->cursor = NULL;
      else
	input->cursor = input->line;

      input->limit = cursor;
    }
  else
    {
      int length
	= getline (&input->line, &input->line_allocated, input->file);

      if (length < 0)
	input->cursor = NULL;
      else
	{
	  input->cursor = input->line;
	  input->limit = input->line + length;
	}
    }
}

/*-------------------------------------------------------------------------.
| Prepare INPUT for next character, then available through *INPUT->CURSOR, |
| unless INPUT->CURSOR is NULL for indicating end of file.                 |
`-------------------------------------------------------------------------*/

static void
input_character_helper (struct input *input)
{
  int counter;

  /* Read in a new line, but skip ignorable ones.  FIXME: This is just not
     right.  Ignorable lines may need copying!  */

  do
    {
      input_line (input);

      /* Possibly check if the line should be ignored.  */

      for (counter = 0; counter < ignore_regexps; counter++)
	if (re_match (ignore_regexp_array[counter],
		      input->line, input->limit - input->line, 0, NULL) > 0)
	  break;
    }
  while (counter < ignore_regexps);

  assert (!input->cursor || input->cursor < input->limit);
}

static inline void
input_character (struct input *input)
{
  assert (input->cursor);
  input->cursor++;
  if (input->cursor == input->limit)
    input_character_helper (input);
}

/*------------------------------------------------------------.
| Construct descriptors for all items of a given INPUT file.  |
`------------------------------------------------------------*/

static void
study_input (struct input *input)
{
#if SAVE_AND_SLOW
# define ADJUST_CHECKSUM(Character)                                \
  checksum = (checksum << 5) + (checksum >> (BITS_PER_WORD - 5))   \
    + (Character & 0xFF)
#else
# define ADJUST_CHECKSUM(Character)                                \
  checksum = (checksum << 5) +                                     \
    (checksum >> (BITS_PER_WORD - BITS_PER_TYPE - 5))              \
    + (Character & 0xFF)
#endif

  int item_count;		/* number of items read */
  char *buffer = NULL;		/* line buffer */
  int length = 0;		/* actual line length in buffer */

  /* Read the file and checksum all items.  */

  if (verbose)
    fprintf (stderr, _("Reading %s"), input->file_name);

  open_input (input);
  input->first_item = items;
  item_count = 0;

  /* Read all lines.  */

  while (input_line (input), input->cursor)
    {
      int counter;

      /* Possibly check if the line should be ignored.  */

      for (counter = 0; counter < ignore_regexps; counter++)
	if (re_match (ignore_regexp_array[counter],
		      input->line, input->limit - input->line, 0, NULL) > 0)
	  break;

      if (counter < ignore_regexps)
	{
	  if (!word_mode)
	    new_item (WHITE, 0);
	  continue;
	}

      /* The line is not being ignored.  */

      /* We prefer `cursor < buffer + length' over `*cursor' in the loop
         tests, so embedded NULs can be handled.  */

      if (word_mode)
	{
	  char *cursor = input->line;
	  unsigned checksum;

	  /* FIXME: Might be simplified out of the line loop.  */

	  while (cursor < input->limit)
	    {
	      while (cursor < input->limit && isspace (*cursor))
		cursor++;

	      if (cursor < input->limit)
		{
		  checksum = 0;
		  while (cursor < input->limit && !isspace (*cursor))
		    {
		      ADJUST_CHECKSUM (*cursor);
		      cursor++;
		    }
		  new_item (NORMAL, checksum);
		  item_count++;
		}
	    }
	}
      else
	{
	  char *cursor;
	  unsigned checksum = 0;
	  int line_has_delims = 0;
	  int line_has_alnums = 0;

	  for (cursor = input->line; cursor < input->limit; cursor++)
	    if (isspace (*cursor))
	      {
		if (!ignore_all_space)
		  {
		    if (ignore_space_change)
		      {
			ADJUST_CHECKSUM (' ');
			while (cursor + 1 < buffer + length
			       && isspace (cursor[1]))
			  cursor++;
		      }
		    else
		      ADJUST_CHECKSUM (*cursor);
		  }
	      }
	    else if (isalpha (*cursor))
	      {
		line_has_alnums = 1;
		if (ignore_case && islower (*cursor))
		  {
		    char character = toupper (*cursor);

		    ADJUST_CHECKSUM (character);
		  }
		else
		  ADJUST_CHECKSUM (*cursor);
	      }
	    else if (isdigit (*cursor))
	      {
		line_has_alnums = 1;
		ADJUST_CHECKSUM (*cursor);
	      }
	    else
	      {
		line_has_delims = 1;
		ADJUST_CHECKSUM (*cursor);
	      }

	  /* Register the checksum.  */

	  if (line_has_alnums)
	    new_item (NORMAL, checksum);
	  else if (line_has_delims)
	    new_item (ignore_delimiters ? DELIMS : NORMAL, checksum);
	  else if (ignore_blank_lines)
	    new_item (WHITE, checksum);
	  else
	    new_item (ignore_delimiters ? DELIMS : NORMAL, checksum);

	  item_count++;
	}
    }

  input->item_limit = items;

  /* Cleanup.  */

  close_input (input);

  if (verbose)
    fprintf (stderr, ngettext (", %d item\n", ", %d items\n", item_count),
	     item_count);

#undef ADJUST_CHECKSUM
}

/*------------------------.
| Study all input files.  |
`------------------------*/

/* If all input files fit in MAXIMUM_TOTAL_BUFFER, they are all swallowed in
   memory at once.  Otherwise, they are read from disk one line at a time.
   A fixed value is far than ideal: we should consider how much real memory
   space is available, and I do not know how to do this portably.  */
#define MAXIMUM_TOTAL_BUFFER 500000

static void
study_all_inputs (void)
{
  struct input *input;
  int total_size;

  /* Compute nick names for all files.  */

  if (inputs == 2)
    {
      /* If only two input files, just mimick `diff' output.  */

      strcpy (input_array[0].nick_name, "-");
      strcpy (input_array[1].nick_name, "+");
    }
  else
    {
      int name_length = 1;
      int file_limit = 26;

      /* Choose nick names as sequences of lower case letters.  */

      while (inputs > file_limit)
	{
	  name_length++;
	  file_limit *= 26;
	}

      for (input = input_array; input < input_array + inputs; input++)
	{
	  int value = input - input_array;
	  char *cursor = input->nick_name + name_length;

	  *cursor-- = '\0';
	  while (cursor >= input->nick_name)
	    {
	      *cursor-- = value % 26 + 'a';
	      value /= 26;
	    }
	}
    }

  /* Swallow all files not already copied in memory, if room permits.  */

  total_size = 0;
  for (input = input_array; input < input_array + inputs; input++)
    total_size += input->stat_buffer.st_size;

  if (total_size <= MAXIMUM_TOTAL_BUFFER)
    for (input = input_array; input < input_array + inputs; input++)
      if (!input->memory_copy)
	swallow_input (input);

  /* Compute all checksums.  */

  new_sentinel ();
  for (input = input_array; input < input_array + inputs; input++)
    {
      study_input (input);
      new_sentinel ();
    }

  if (verbose)
    {
      fprintf (stderr, _("Read summary:"));
      fprintf (stderr, ngettext (" %d file,", " %d files,", inputs), inputs);
      fprintf (stderr, ngettext (" %d item\n", " %d items\n",
				 items - inputs - 1), items - inputs - 1);
    }

#if DEBUGGING
  if (debugging)
    {
      dump_all_items ();
      dump_all_inputs ();
    }
#endif
}

/* Item references.  */

struct reference
{
  struct input *input;		/* input file for reference */
  int number;			/* item number for reference */
};

/*-------------------------------------------------------------------------.
| Explicit a reference for internal item NUMBER.  The first sentinel has   |
| line 0, other sentinels have one more than number of lines in the file.  |
`-------------------------------------------------------------------------*/

static struct reference
get_reference (int number)
{
  struct input *input;
  struct reference result;

  assert (number < items);

  /* FIXME: if we are studying many files, linear searches become a bit
     crude, binary searches would be better in such cases.  */

  for (input = input_array; number >= input->item_limit + 1; input++)
    ;

  result.input = input;
  result.number = number - input->first_item + 1;
  return result;
}

/*------------------------------------------------------------------------.
| Produce a short string referencing an internal item NUMBER.  The result |
| is statically allocated into a ring of little buffers.                  |
`------------------------------------------------------------------------*/

static char *
reference_string (int number)
{
#define RING_LENGTH 4

  struct reference reference;
  static char buffer[RING_LENGTH][20];
  static int next = RING_LENGTH - 1;

  if (next == RING_LENGTH - 1)
    next = 0;
  else
    next++;

  reference = get_reference (number);

  if (number == reference.input->item_limit)
    sprintf (buffer[next], "%s.EOF", reference.input->nick_name);
  else
    sprintf (buffer[next], "%s%d",
	     reference.input->nick_name, reference.number);

  return buffer[next];

#undef RING_LENGTH
}

#if DEBUGGING

/*-------------------------------------------------------------------------.
| Dump a reference to a given item in compilation error format.  Ensure at |
| least one space, and in fact, enough to align diagnostics as we go.      |
`-------------------------------------------------------------------------*/

static void
dump_reference (int item)
{
  struct reference reference;
  char buffer[20];
  static int last_width = 0;
  int width;

  reference = get_reference (item);
  sprintf (buffer, "%d", reference.number);

  fprintf (stderr, "%s:%s: ", reference.input->file_name, buffer);
  width = strlen (reference.input->file_name) + strlen (buffer);
  if (width < last_width)
    while (width++ < last_width)
      putc (' ', stderr);
  else
    last_width = width;
}

#endif /* DEBUGGING */

/* Clusters and members.  */

/* A cluster represents one set of items which is repeated at two or more
   places in the input files, while each repetition is called a cluster
   member.  The member size is an item count which excludes white items.  */

struct cluster
{
  short first_member;		/* index in member_array array */
  short item_count;		/* size of each member */
};

static struct cluster *cluster_array = NULL;
int clusters = 0;

/* A cluster member starts at some item number and runs for a given number
   of items.  Ignorable items may be embedded in a cluster member, so
   cluster members of a single cluster may have different lengths, but a
   cluster member never starts nor end with an white item.  Sentinels
   are never part of any cluster.  By construction, all cluster members of a
   same cluster are next to each other.  */

struct member
{
  short cluster_number;		/* ordinal of cluster, counted from 0 */
  int first_item;		/* number of first item for this member */
};

static struct member *member_array = NULL;
static int members = 0;

/* Convenience macros.  */

#define MEMBER_LIMIT(Cluster) \
  (member_array + ((Cluster) + 1)->first_member)

#define MEMBERS(Cluster) \
  (((Cluster) + 1)->first_member - (Cluster)->first_member)

/*---------------------------------------------------------------------.
| Find how many input items are part of a given cluster MEMBER.  The   |
| returned count includes embedded items having less significance, but |
| exclude last white items.                                            |
`---------------------------------------------------------------------*/

static int
real_member_size (struct member *member)
{
  int number = cluster_array[member->cluster_number].item_count;
  ITEM *item = item_array + member->first_item;
  ITEM *last_item;

  /* Skip over NUMBER normal items.  */

  while (number > 0)
    switch (item_type (item))
      {
      case NORMAL:
	number--;
	/* Fall through.  */

      case DELIMS:
      case WHITE:
	item++;
	break;

      case SENTINEL:
	/* Cannot happen.  */
	abort ();
      }

  /* Skip over supplementary matchable or white items, return the item
     count as for the last normal or matchable item.  */

  last_item = item;
  while (1)
    switch (item_type (item))
      {
      case NORMAL:
      case SENTINEL:
	return last_item - item_array - member->first_item;

      case DELIMS:
	last_item = item++;
	break;

      case WHITE:
	item++;
	break;
      }
}

#if DEBUGGING

/*-----------------------.
| Dump a given CLUSTER.  |
`-----------------------*/

static void
dump_cluster (struct cluster *cluster)
{
  int counter;
  struct member *member;

  fprintf (stderr, "{%ld},%d",
	   (long) (cluster - cluster_array), cluster->item_count);
  for (counter = cluster->first_member;
       counter < (cluster + 1)->first_member; counter++)
    {
      member = member_array + counter;
      fprintf (stderr, "%c[%d] %s,%d",
	       counter == cluster->first_member ? '\t' : ' ', counter,
	       reference_string (member->first_item),
	       real_member_size (member));
    }
  putc ('\n', stderr);
}

/*--------------------.
| Dump all clusters.  |
`--------------------*/

static void
dump_all_clusters (void)
{
  int counter;

  fputs ("\f\n", stderr);
  for (counter = 0; counter < clusters; counter++)
    dump_cluster (cluster_array + counter);
}

/*----------------------.
| Dump a given MEMBER.  |
`----------------------*/

static void
dump_member (struct member *member)
{
  fprintf (stderr, " [%ld]\t{%d}\t%s,%d+\n",
	   (long) (member - member_array), member->cluster_number,
	   reference_string (member->first_item),
	   cluster_array[member->cluster_number].item_count);
}

/*-------------------.
| Dump all members.  |
`-------------------*/

static void
dump_all_members (void)
{
  int counter;

  fputs ("\f\n", stderr);
  for (counter = 0; counter < members; counter++)
    dump_member (member_array + counter);
}

#endif /* DEBUGGING */

/*----------------------------------------------------------------------.
| Sort helper.  Compare two member indices, using the original input    |
| order.  When two members have the same start, put the longest first.  |
`----------------------------------------------------------------------*/

static int
compare_for_member_start (const void *void_first, const void *void_second)
{
#define value1 *((int *) void_first)
#define value2 *((int *) void_second)

  int result;

  struct member *member1 = member_array + value1;
  struct member *member2 = member_array + value2;

  result = member1->first_item - member2->first_item;
  if (result)
    return result;

  return (cluster_array[member2->cluster_number].item_count
	  - cluster_array[member1->cluster_number].item_count);

#undef value1
#undef value2
}

/*---------------------------------------------------------------------.
| Add a new cluster, for which each member has COUNT non-white items.  |
`---------------------------------------------------------------------*/

static inline void
new_cluster (int count)
{
  struct cluster *cluster;

  if (clusters % 8 == 0)
    cluster_array = (struct cluster *)
      xrealloc (cluster_array, (clusters + 8) * sizeof (struct cluster));

  cluster = cluster_array + clusters++;
  cluster->first_member = members;
  cluster->item_count = count;
}

/*---------------------------.
| Add a new cluster member.  |
`---------------------------*/

static inline void
new_member (int item)
{
  struct member *member;

  if (members % 8 == 0)
    member_array = (struct member *)
      xrealloc (member_array, (members + 8) * sizeof (struct member));

  member = member_array + members++;
  member->cluster_number = clusters - 1;
  member->first_item = item;
}

/*------------------------------------.
| Output a cluster explanation line.  |
`------------------------------------*/

static void
explain_member (char prefix, struct member *quote)
{
  struct cluster *cluster = cluster_array + quote->cluster_number;
  struct member *member;
  int counter;
  struct reference reference;

  putc (prefix, output_file);
  for (member = member_array + cluster->first_member;
       member < MEMBER_LIMIT (cluster); member++)
    {
      reference = get_reference (member->first_item);
      fprintf (output_file, member == quote ? " %s%d,%d" : " (%s%d,%d)",
	       reference.input->nick_name, reference.number,
	       real_member_size (member));
    }
  putc ('\n', output_file);
}

/*----------------------.
| Search for clusters.  |
`----------------------*/

static void
prepare_clusters (void)
{
  /* The array of indirect items have similar contents next to each other.  */
  int *indirect_item_array = xmalloc (items * sizeof (int));
  int indirect_items = 0;	/* number of entries in indirect_item_array */
  int *cluster_set;		/* index for first member in set of clusters */
  int *member_set;		/* index for first member in current cluster */

  /* For a given index, SIZE_ARRAY[index] describes the common item size
     between item CLUSTER_SET[index] and CLUSTER_SET[index + 1].  */
  int *size_array = NULL;
  int allocated_sizes = 0;	/* allocated entries for size_array */
  int sizes;			/* number of entries in size_array */
  int *size;			/* cursor in size_array */

  /* Here is a bag of starts indices for members of the same cluster (having
     all CLUSTER_SIZE worth items).  These indices are values selected out
     of INDIRECT_ITEM_ARRAY, between FIRST and LAST (included).  */
  int *sorter_array = NULL;	/* for ensuring members are in nice order */
  int sorters;			/* number of members in sorter_array */
  int *sorter;			/* cursor into sorter_array */
  int *cursor;			/* cursor into sorter_array */

  ITEM *item;			/* cursor in item_array */

  int cluster_size;		/* size of current cluster */
  int size_value;		/* current size, or previous cluster size */
  int counter;			/* all purpose counter */
  int checksum;			/* possible common cheksum of previous items */

  /* Sort indices.  */

#if DEBUGGING
  if (debugging)
    fprintf (stderr, _("Sorting"));
#endif

  for (counter = 0; counter < items; counter++)
    if (item_type (item_array + counter) == NORMAL
	|| item_type (item_array + counter) == DELIMS)
      indirect_item_array[indirect_items++] = counter;
  if (indirect_items < items)
    indirect_item_array = (int *)
      xrealloc (indirect_item_array, indirect_items * sizeof (int));
  qsort (indirect_item_array, indirect_items, sizeof (int),
	 compare_for_checksum_runs);

  /* Find all clusters.  */

#if DEBUGGING
  if (debugging)
    fprintf (stderr, _(", clustering"));
#endif

  for (cluster_set = indirect_item_array;
       cluster_set + 1 < indirect_item_array + indirect_items;
       cluster_set += sizes + 1)
    {
      /* Find all members beginning with the same run of checksums.  These
         will be later distributed among a few clusters of various sizes,
         and so, are common to the incoming set of clusters.  */

      sizes = 0;
      while (cluster_set + sizes + 1 < indirect_item_array + indirect_items)
	{
	  size_value
	    = identical_size (cluster_set[sizes], cluster_set[sizes + 1]);
	  if (size_value < minimum_size)
	    break;

	  if (sizes == allocated_sizes)
	    {
	      allocated_sizes += 32;
	      size_array = (int *)
		xrealloc (size_array, allocated_sizes * sizeof (int));
	      sorter_array = (int *)
		xrealloc (sorter_array, (allocated_sizes + 1) * sizeof (int));
	    }
	  size_array[sizes++] = size_value;
	}

      /* This test is not necessary for the algorithm to work, but this
         fairly common case might be worth a bit of speedup.  Maybe!  */

      if (sizes == 0)
	continue;

      /* Output all clusters of the set from the tallest to the shortest.
         Here, size refers to number of items in members, and not to the
         number of members in a cluster.  (In fact, it is very expectable
         that shortest clusters have more members.)  */

      cluster_size = 0;
      while (1)
	{
	  /* Discover the maximum cluster size not yet processed.  */

	  size_value = cluster_size;
	  cluster_size = 0;

	  for (size = size_array; size < size_array + sizes; size++)
	    if ((size_value == 0 || *size < size_value)
		&& *size > cluster_size)
	      cluster_size = *size;

	  /* If the cluster size did not decrease, we cannot do more.  */

	  if (cluster_size == 0)
	    break;

	  /* Consider all consecutive members having at least the cluster
	     size in common: they form a cluster.  But any gap in the
	     sequence represents a change of cluster: same length, but
	     different contents.  */

	  size = size_array;
	  while (1)
	    {
	      /* Find all members for one cluster.  However, after having
	         skipped over the gap, just break out if nothing remains.  */

	      while (size < size_array + sizes && *size < cluster_size)
		size++;
	      if (size == size_array + sizes)
		break;

	      member_set = cluster_set + (size - size_array);
	      sorters = 1;
	      while (size < size_array + sizes && *size >= cluster_size)
		{
		  sorters++;
		  size++;
		}

	      /* No cluster may be a proper subset of another.  That is, if
	         all putative members are preceded by identical items, then
	         they are indeed part of a bigger cluster, which has already
	         been or will later be caught.  In such case, just avoid
	         retaining them here.  This also prevents quadratic
	         behaviour which, I presume, would be heavy on computation.

	         This test may be defeated by tolerant matches.  Maybe
	         tolerant matches will just go away.  Surely, tolerant
	         matches later require subset member elimination.  */

	      item = item_array + member_set[0];
	      BACKWARD_ITEM (item);
	      if (item_type (item) != SENTINEL)
		{
		  checksum = item->checksum;

		  for (counter = 1; counter < sorters; counter++)
		    {
		      item = item_array + member_set[counter];
		      BACKWARD_ITEM (item);
		      if (item_type (item) == SENTINEL
			  || item->checksum != checksum)
			break;
		    }
		  if (counter == sorters)
		    continue;
		}

	      /* Skip any cluster member overlapping with the previous
	         member of the same cluster.  If doing so, chop the size of
	         the first member just before the overlap point: this should
	         later trigger a cluster having this reduced size.  */

	      memcpy (sorter_array, member_set, sorters * sizeof (int));
	      qsort (sorter_array, sorters, sizeof (int),
		     compare_for_positions);
	      cursor = sorter_array;
	      for (counter = 1; counter < sorters; counter++)
		{
		  size_value = item_distance (*cursor, sorter_array[counter],
					      cluster_size);
		  if (size_value < 0)
		    *++cursor = sorter_array[counter];
		  else
		    {
		      int counter2;

		      for (counter2 = 0; counter2 < sizes; counter2++)
			if (cluster_set[counter2] == *cursor
			    || cluster_set[counter2] == sorter_array[counter])
			  {
			    assert (size_value < size_array[counter2]);
			    size_array[counter2] = size_value;
			    break;
			  }
		      assert (counter2 < sizes);
		    }
		}
	      sorters = cursor - sorter_array + 1;

	      /* Create the cluster only if at least two members remain.  */

	      if (sorters >= 2)
		{
		  new_cluster (cluster_size);
		  for (counter = 0; counter < sorters; counter++)
		    new_member (sorter_array[counter]);
		}
	    }
	}
    }

  /* Add a sentinel cluster at the end.  */

  new_cluster (0);
  clusters--;

  /* Cleanup.  */

  free (indirect_item_array);
  free (size_array);

#if DEBUGGING
  if (debugging)
    {
      fprintf (stderr, _(", done\n"));
      dump_all_clusters ();
    }
#endif
}

/*------------------------------------------------------------.
| Check if a MEMBER is active in some file other than INPUT.  |
`------------------------------------------------------------*/

static int
active_elsewhere (struct member *member, struct input *input)
{
  struct cluster *cluster = cluster_array + member->cluster_number;

  /* FIXME: maybe it is sufficient to test first and last member only?  */

  for (member = member_array + cluster->first_member;
       member < MEMBER_LIMIT (cluster); member++)
    if (member->first_item < input->first_item
	|| member->first_item + real_member_size (member) > input->item_limit)
      return 1;

  return 0;
}

/* Indirect members.  */

/* The following array gives indices into the array of cluster members,
   member_array above, in such a way that the succession of indices points
   to members in the original textual order of input files.  */

static int *indirect_array;
static int indirects;

/*----------------------------------------.
| Compute the array of indirect members.  |
`----------------------------------------*/

static void
prepare_indirects (void)
{
  int counter;
  struct member *member;
  struct input *input;
  int duplicated_items;

#if DEBUGGING
  if (debugging)
    fprintf (stderr, _("Sorting members"));
#endif

  indirect_array = xmalloc (members * sizeof (int));
  for (counter = 0; counter < members; counter++)
    indirect_array[counter] = counter;
  if (members > 1)
    qsort (indirect_array, members, sizeof (int), compare_for_member_start);
  indirects = members;

#if DEBUGGING
  if (debugging)
    {
      fprintf (stderr, _(", done\n"));
      fputs ("\f\n", stderr);
      for (counter = 0; counter < indirects; counter++)
	{
	  member = member_array + indirect_array[counter];
	  dump_reference (member->first_item);
	  dump_member (member_array + indirect_array[counter]);
	  if (counter < indirects - 1
	      && (member_array[indirect_array[counter + 1]].first_item
		  < member->first_item + real_member_size (member)))
	    {
	      dump_reference (member->first_item);
	      fprintf (stderr, "Overlapping with next member\n");
	    }
	}
    }
#endif

  if (verbose)
    {
      fprintf (stderr, _("Work summary:"));
      fprintf (stderr, ngettext (" %d cluster,", " %d clusters,", clusters),
	       clusters);
      fprintf (stderr, ngettext (" %d member\n", " %d members\n", members),
	       members);
    }
}

/* Mergings.  */

/* A merging is an indication for advancing in one given input file, by
   catching up all items (differences) until a cluster member, and then
   skipping over that member.  Mergings are ordered so to maintain parallel
   advancement between files.  A merging group is a fragment in the sequence
   of consecutive mergings, all mergings in a group have members from the
   same cluster.  A merging group is then variable in length, a flag in the
   merging sequence signals the beginning of a new group.

   However, some cluster members might have to be considered as differences
   preceding the more genuine member of a group, because they represent
   cross matches which are not easily represented in a parallel sequence.
   Another flag signals such members, which then escape the rule that all
   members of a group share the same cluster.  */

struct merging
{
  unsigned group_flag:1;	/* beginning of a new merge group */
  unsigned cross_flag:1;	/* member isolated by cross matches */
  unsigned input_number:14;	/* input file designator */
  unsigned member_number:16;	/* member designator */
};

static struct merging *merging_array = NULL;
static int mergings = 0;

#if DEBUGGING

/*--------------------.
| Dump all mergings.  |
`--------------------*/

static void
dump_all_mergings (void)
{
  struct merging *merging;
  struct reference reference;

  fputs ("\f\n", stderr);
  for (merging = merging_array; merging < merging_array + mergings; merging++)
    {
      if (merging->group_flag)
	{
	  if (merging > merging_array)
	    putc ('\n', stderr);
	  fprintf (stderr, "<%ld>", (long) (merging - merging_array));
	}
      reference
	= get_reference (member_array[merging->member_number].first_item);
      fprintf (stderr, "\t[%d]%c%s%d", merging->member_number,
	       merging->cross_flag ? 'X' : ' ',
	       reference.input->nick_name, reference.number);
      assert (reference.input == input_array + merging->input_number);
    }
  putc ('\n', stderr);
}

#endif /* DEBUGGING */

/*------------------------------------------------------------------.
| After having output PREFIX, explain the merging group starting at |
| MERGING and extending until LIMIT.                                |
`------------------------------------------------------------------*/

static void
explain_group (char prefix, struct merging *merging, struct merging *limit)
{
  struct cluster *cluster
    = cluster_array + member_array[merging->member_number].cluster_number;
  int counter;
  struct input *input;
  struct member *member;
  struct merging *cursor;
  struct reference reference;

  putc (prefix, output_file);
  for (counter = cluster->first_member;
       counter < (cluster + 1)->first_member; counter++)
    {
      member = member_array + counter;
      reference = get_reference (member->first_item);

      for (cursor = merging;
	   cursor < limit && cursor->member_number != counter; cursor++)
	;

      fprintf (output_file, cursor < limit ? " %s%d,%d" : " (%s%d,%d)",
	       reference.input->nick_name,
	       reference.number, real_member_size (member));
    }
  putc ('\n', output_file);
}

/*--------------------.
| Add a new merging.  |
`--------------------*/

static void
new_merging (int group, int cross, struct input *input, struct member *member)
{
  struct merging *merging = merging_array + mergings++;

  merging->group_flag = group;
  merging->cross_flag = cross;
  merging->input_number = input - input_array;
  merging->member_number = member - member_array;
}

/*------------------------------.
| Decide the merging sequence.  |
`------------------------------*/

void
prepare_mergings (void)
{
  int *cursor;

  struct input *input0;
  struct input *input;
  struct member *member;

  struct cluster *cluster;	/* current cluster candidate */
  int cost;			/* cost associate with current candidate */
  struct cluster *best_cluster;	/* best cluster candidate for hunk */
  int best_cost;		/* cost associated with best cluster */
  int group_flag;		/* set if next merging starts group */

  /* Remove member overlaps.  */

  if (members > 0)
    {
      int counter;

      /* When members overlap, retain only the shortest ones (for now).  */

      indirects = 0;
      for (counter = 0; counter < members - 1; counter++)
	{
	  member = member_array + indirect_array[counter];
	  if (member->first_item + real_member_size (member)
	      <= member_array[indirect_array[counter + 1]].first_item)
	    indirect_array[indirects++] = indirect_array[counter];
	  else
	    /* Invalidate this member.  */
	    member->cluster_number = -1;
	}
      indirect_array[indirects++] = indirect_array[counter];
    }

  /* Once indirects sorted and ready, this array is logically split into
     segments, one per input file, which segments might be later scanned in
     parallel a few times.  All required state variables for resuming the
     scan are held into the last few fields of the input structure.  */

  cursor = indirect_array;
  for (input = input_array; input < input_array + inputs; input++)
    {
      input->item = input->first_item;
      input->indirect_cursor = cursor;
      while (cursor < indirect_array + indirects
	     && member_array[*cursor].first_item < input->item_limit)
	cursor++;
      input->indirect_limit = cursor;
    }

  /* Allocate an array for discovered mergings.  */

  merging_array = xmalloc (indirects * sizeof (struct merging));
  mergings = 0;

  /* Repetitively consider the incoming cluster members for all inputs, and
     select at each iteration the cluster members causing least differences.
     This may indirectly trigger bigger differences into later iterations.
     Some thought should be given to global optimisation.  FIXME.  */

  while (1)
    {
#if DEBUGGING
      if (debugging)
	{
	  fprintf (stderr, "\nCHOOSING NEXT CLUSTER\n");

	  for (input = input_array; input < input_array + inputs; input++)
	    if (input->indirect_cursor < input->indirect_limit)
	      {
		fprintf (stderr, "%s:\t", reference_string (input->item));
		dump_member (INPUT_MEMBER (input));
	      }
	    else
	      fprintf (stderr, "%s:\tNone\n", reference_string (input->item));

	  putc ('\n', stderr);
	}
#endif

      best_cluster = NULL;
      for (input0 = input_array; input0 < input_array + inputs; input0++)
	if (input0->indirect_cursor < input0->indirect_limit)
	  {
	    cluster = INPUT_CLUSTER (input0);
#if DEBUGGING
	    if (debugging)
	      {
		fprintf (stderr, "%s:\t", reference_string (input0->item));
		dump_cluster (cluster);
	      }
#endif

	    /* Skip this input if we have already evaluated the cluster.  */

	    for (input = input_array; input < input0; input++)
	      if (input->indirect_cursor < input->indirect_limit
		  && INPUT_CLUSTER (input) == cluster)
		break;
	    if (input < input0)
	      continue;

	    /* Evaluate the cost of the cluster.  This will be the total
	       number of difference items needed in the listing for getting
	       to the point of printing the cluster.  For computing it,
	       check all members of the evaluated cluster, retaining only
	       one member per file, in fact, exactly the first which has not
	       been listed yet.  */

	    cost = 0;
	    input = input_array;
	    member = member_array + cluster->first_member;
	    while (member < MEMBER_LIMIT (cluster))
	      {
		/* Just ignore invalidated members.  */

		if (member->cluster_number < 0)
		  {
		    member++;
		    continue;
		  }

		/* Find the proper file.  */

		while (member->first_item >= input->item_limit)
		  input++;

		/* Ignore a member that would have been already listed.  */

		if (member->first_item < input->item)
		  {
		    member++;
		    continue;
		  }

		/* Accumulate cost for this member.  */

#if DEBUGGING
		if (debugging)
		  fprintf (stderr, "    Cost += %d  [%s..%s)\n",
			   member->first_item - input->item,
			   reference_string (input->item),
			   reference_string (member->first_item));
#endif
		cost += (member++)->first_item - input->item;

		/* Skip other members that would appear later in same file.  */

		while (member < MEMBER_LIMIT (cluster)
		       && member->first_item < input->item_limit)
		  member++;
	      }

	    /* Retain the best cluster so far.  */

#if DEBUGGING
	    if (debugging)
	      {
		if (best_cluster)
		  fprintf (stderr, "  {%ld}=%d <-> {%ld}=%d\n",
			   (long) (best_cluster - cluster_array), best_cost,
			   (long) (cluster - cluster_array), cost);
		else
		  fprintf (stderr, "  {%ld}=%d\n",
			   (long) (cluster - cluster_array), cost);
	      }
#endif
	    if (!best_cluster || cost < best_cost)
	      {
		best_cluster = cluster;
		best_cost = cost;
	      }
	  }

      /* Get out if everything has been done.  */

      if (!best_cluster)
	break;

#if DEBUGGING
      if (debugging)
	{
	  fprintf (stderr, "\nCHOICE\t");
	  dump_cluster (best_cluster);
	  putc ('\n', stderr);
	}
#endif

      /* Save found mergings, while moving all items pointers to after the
         members of the best cluster.  */

      input = input_array;
      group_flag = 1;
      member = member_array + best_cluster->first_member;
      while (member < MEMBER_LIMIT (best_cluster))
	{
	  /* Just ignore invalidated members.  */

	  if (member->cluster_number < 0)
	    {
	      member++;
	      continue;
	    }

	  /* Find the proper file.  */

#if DEBUGGING
	  if (debugging)
	    {
	      putc ('v', stderr);
	      dump_member (member);
	    }
#endif
	  while (member->first_item >= input->item_limit)
	    input++;

	  /* Ignore a member that would have been already listed.  */

	  if (member->first_item < input->item)
	    {
	      member++;
	      continue;
	    }

	  /* Skip all crossed members while adding them as crossed mergings.  */

	  while (INPUT_MEMBER (input) != member)
	    {
	      new_merging (group_flag, 1, input, INPUT_MEMBER (input));
	      group_flag = 0;
	      input->indirect_cursor++;
	    }
	  assert (input->indirect_cursor < input->indirect_limit);
	  assert (INPUT_MEMBER (input)->first_item < input->item_limit);

	  /* Skip the selected member while adding it as a direct merging.  */

	  new_merging (group_flag, 0, input, member);
	  group_flag = 0;
	  input->item = member->first_item + real_member_size (member);
	  input->indirect_cursor++;

	  /* Skip other members that would appear later in same file.  */

	  while (member < MEMBER_LIMIT (best_cluster)
		 && member->first_item < input->item_limit)
	    member++;
	}
    }

#if DEBUGGING
  if (debugging)
    dump_all_mergings ();
#endif

  assert (mergings == indirects);

  if (verbose)
    {
      fprintf (stderr, _("Work summary:"));
      fprintf (stderr, ngettext (" %d cluster,", " %d clusters,", clusters),
	       clusters);
      fprintf (stderr, ngettext (" %d member,", " %d members,", members),
	       members);
      fprintf (stderr, ngettext (" %d overlap\n", " %d overlaps\n",
				 members - indirects), members - indirects);
    }
}

/* Terminal and pager support.  */

/* How to start underlining.  */
const char *termcap_start_underline = NULL;

/* How to stop underlining.  */
const char *termcap_stop_underline = NULL;

/* How to start bolding.  */
const char *termcap_start_bold = NULL;

/* How to stop bolding.  */
const char *termcap_stop_bold = NULL;

enum emphasis
{
  STRAIGHT,			/* no emphasis */
  UNDERLINED,			/* underlined text */
  BOLD				/* bold text */
};

/* Current output emphasis.  */
enum emphasis current_emphasis = STRAIGHT;

/*-----------------------------.
| Select strings for marking.  |
`-----------------------------*/

static void
initialize_strings (void)
{
  struct input *input;

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
	error (EXIT_ERROR, 0,
	       _("select a terminal through the TERM environment variable"));
      success = tgetent (term_buffer, name);
      if (success < 0)
	error (EXIT_ERROR, 0, _("could not access the termcap data base"));
      if (success == 0)
	error (EXIT_ERROR, 0, _("terminal type `%s' is not defined"), name);
      buffer = (char *) malloc (strlen (term_buffer));
      filler = buffer;

      termcap_start_underline = tgetstr ("us", &filler);
      termcap_stop_underline = tgetstr ("ue", &filler);
      termcap_start_bold = tgetstr ("so", &filler);
      termcap_stop_bold = tgetstr ("se", &filler);
    }
#endif /* HAVE_TPUTS */

  /* Ensure some default strings.  For all input files except last, prefer
     underline.  For last input file, prefer bold.  */

  for (input = input_array; input < input_array + inputs - 1; input++)
    {
      input->term_start = termcap_start_underline;
      input->term_stop = termcap_stop_underline;

      if (!overstrike)
	{
	  if (!termcap_start_underline && !input->user_start)
	    input->user_start = "[-";
	  if (!termcap_stop_underline && !input->user_stop)
	    input->user_stop = "-]";
	}
    }

  input->term_start = termcap_start_bold;
  input->term_stop = termcap_stop_bold;

  if (!overstrike)
    {
      if (!termcap_start_bold && !input->user_start)
	input->user_start = "{+";
      if (!termcap_stop_bold && !input->user_stop)
	input->user_stop = "+}";
    }
}

#if HAVE_TPUTS

/*-----------------------------------------.
| Write one character for tputs function.  |
`-----------------------------------------*/

static int
putc_for_tputs (int character)
{
  return putc (character, output_file);
}

#endif /* HAVE_TPUTS */

/*-----------------------.
| Set current EMPHASIS.  |
`-----------------------*/

static void
set_emphasis (enum emphasis emphasis)
{
#if HAVE_TPUTS
  switch (current_emphasis)
    {
    case STRAIGHT:
      switch (emphasis)
	{
	case STRAIGHT:
	  /* Done.  */
	  break;

	case UNDERLINED:
	  if (termcap_start_underline)
	    tputs (termcap_start_underline, 0, putc_for_tputs);
	  break;

	case BOLD:
	  if (termcap_start_bold)
	    tputs (termcap_start_bold, 0, putc_for_tputs);
	  break;
	}
      break;

    case UNDERLINED:
      switch (emphasis)
	{
	case STRAIGHT:
	  if (termcap_stop_underline)
	    tputs (termcap_stop_underline, 0, putc_for_tputs);
	  break;

	case UNDERLINED:
	  /* Done.  */
	  break;

	case BOLD:
	  if (termcap_stop_underline)
	    tputs (termcap_stop_underline, 0, putc_for_tputs);
	  if (termcap_start_bold)
	    tputs (termcap_start_bold, 0, putc_for_tputs);
	  break;
	}
      break;

    case BOLD:
      switch (emphasis)
	{
	case STRAIGHT:
	  if (termcap_stop_bold)
	    tputs (termcap_stop_bold, 0, putc_for_tputs);
	  break;

	case UNDERLINED:
	  if (termcap_stop_bold)
	    tputs (termcap_stop_bold, 0, putc_for_tputs);
	  if (termcap_start_underline)
	    tputs (termcap_start_underline, 0, putc_for_tputs);
	  break;

	case BOLD:
	  /* Done.  */
	  break;
	}
      break;
    }
#endif /* HAVE_TPUTS */

  current_emphasis = emphasis;
}

/*---------------------------------------------------------.
| Push a new EMPHASIS, or pop it out back to what it was.  |
`---------------------------------------------------------*/

#define EMPHASIS_STACK_LENGTH 1

static enum emphasis emphasis_array[EMPHASIS_STACK_LENGTH];
static int emphasises = 0;

static void
push_emphasis (enum emphasis emphasis)
{
  assert (emphasises < EMPHASIS_STACK_LENGTH);

  emphasis_array[emphasises++] = current_emphasis;
  set_emphasis (emphasis);
}

static void
pop_emphasis (void)
{
  assert (emphasises > 0);

  set_emphasis (emphasis_array[--emphasises]);
}

/*--------------------------------------------------------------------------.
| Output a STRING having LENGTH characters, subject to the current          |
| emphasis.  If EXPAND is nonzero, expand TABs.  For expansion to work      |
| properly, this routine should always be called while copying input data.  |
`--------------------------------------------------------------------------*/

static void
output_characters (const char *string, int length, int expand)
{
  static unsigned column = 0;

  const char *cursor;
  int counter;

  if (overstrike || expand)
    for (cursor = string; cursor < string + length; cursor++)
      switch (*cursor)
	{
	  /* Underlining or overstriking whitespace surely looks strange,
	     but "less" does understand such things as emphasis requests.  */

	case '\n':
	case '\r':
	case '\v':
	case '\f':
	  putc (*cursor, output_file);
	  column = 0;
	  break;

	case '\b':
	  putc ('\b', output_file);
	  if (column > 0)
	    column--;
	  break;

	case '\t':
	  if (expand)
	    do
	      {
		if (overstrike)
		  switch (current_emphasis)
		    {
		    case STRAIGHT:
		      putc (' ', output_file);
		      break;

		    case UNDERLINED:
		      putc ('_', output_file);
		      if (overstrike_for_less)
			{
			  putc ('\b', output_file);
			  putc (' ', output_file);
			}
		      break;

		    case BOLD:
		      putc (*cursor, output_file);
		      if (overstrike_for_less)
			{
			  putc ('\b', output_file);
			  putc (' ', output_file);
			}
		      break;
		    }
		else
		  putc (' ', output_file);
		column++;
	      }
	    while (column % 8 != 0);
	  else
	    putc ('\t', output_file);
	  break;

	case ' ':
	  if (overstrike)
	    switch (current_emphasis)
	      {
	      case STRAIGHT:
		putc (' ', output_file);
		break;

	      case UNDERLINED:
		putc ('_', output_file);
		if (overstrike_for_less)
		  {
		    putc ('\b', output_file);
		    putc (' ', output_file);
		  }
		break;

	      case BOLD:
		putc (' ', output_file);
		if (overstrike_for_less)
		  {
		    putc ('\b', output_file);
		    putc (' ', output_file);
		  }
		break;
	      }
	  else
	    putc (' ', output_file);
	  column++;
	  break;

	case '_':
	  if (overstrike_for_less && current_emphasis == UNDERLINED)
	    {
	      /* Overstriking an underline would make it bold in "less".  */
	      putc ('_', output_file);
	      column++;
	      break;
	    }
	  /* Fall through.  */

	default:
	  if (overstrike)
	    switch (current_emphasis)
	      {
	      case STRAIGHT:
		putc (*cursor, output_file);
		break;

	      case UNDERLINED:
		putc ('_', output_file);
		putc ('\b', output_file);
		putc (*cursor, output_file);
		break;

	      case BOLD:
		putc (*cursor, output_file);
		putc ('\b', output_file);
		putc (*cursor, output_file);
		break;
	      }
	  else
	    putc (*cursor, output_file);
	  column++;
	  break;
	}
  else
    fwrite (string, length, 1, output_file);
}

/*------------------------------------------------------------------------.
| While copying INPUT, select EMPHASIS, with preferred type for printer.  |
`------------------------------------------------------------------------*/

static void
start_of_emphasis (struct input *input, enum emphasis emphasis)
{
  assert (current_emphasis == STRAIGHT);

  /* Avoid any emphasis if it would be useless.  */

#if FIXME
  if (!common_listing_allowed
      && (!right->listing_allowed || !left->listing_allowed))
    return;
#endif

#if HAVE_TPUTS
  if (input->term_start)
    tputs (input->term_start, 0, putc_for_tputs);
#endif

  if (input->user_start)
    fprintf (output_file, "%s", input->user_start);

  current_emphasis = emphasis;
}

/*-----------------------------------------------.
| Indicate end of emphasis while copying INPUT.  |
`-----------------------------------------------*/

static void
end_of_emphasis (struct input *input)
{
  assert (current_emphasis != STRAIGHT);

  /* Avoid any emphasis if it would be useless.  */

#if FIXME
  if (!common_listing_allowed
      && (!right->listing_allowed || !left->listing_allowed))
    return;
#endif

  if (input->user_stop)
    fprintf (output_file, "%s", input->user_stop);

#if HAVE_TPUTS
  if (input->term_stop)
    tputs (input->term_stop, 0, putc_for_tputs);
#endif

  current_emphasis = STRAIGHT;
}

/*-------------------------------------------------------------------.
| Launch the output pager if any.  If INPUT is not NULL, do it for a |
| specific input file.                                               |
`-------------------------------------------------------------------*/

static void
launch_output_program (struct input *input)
{
  if (paginate)
    {
      if (input)
	output_file = writepipe ("pr", "-f", "-h", input->file_name, NULL);
      else
	output_file = writepipe ("pr", "-f", NULL);
    }
  else
    {
      char *program;		/* name of the pager */
      char *basename;		/* basename of the pager */

      /* Check if a output program should be called, and which one.  Avoid
         all paging if only statistics are needed.  */

      if (autopager && isatty (fileno (stdout))
#if FIXME
	  && (left->listing_allowed
	      || right->listing_allowed || common_listing_allowed)
#endif
	)
	{
	  program = getenv ("WDIFF_PAGER");
	  if (program == NULL)
	    program = getenv ("PAGER");
#ifdef PAGER_PROGRAM
	  if (program == NULL)
	    program = PAGER_PROGRAM;
#endif
	}
      else
	program = NULL;

      /* Use stdout as default output.  */

#if DEBUGGING
      output_file = debugging ? stderr : stdout;
#else
      output_file = stdout;
#endif

      /* If we should use a pager, launch it.  */

      if (program && *program)
	{
	  char *lessenv;

	  lessenv = getenv ("LESS");
	  if (lessenv == NULL)
	    {
	      setenv ("LESS", LESS_DEFAULT_OPTS, 0);
	    }
	  else
	    {
	      if (asprintf (&lessenv, "%s %s", LESS_DEFAULT_OPTS, lessenv) ==
		  -1)
		{
		  xalloc_die ();
		  return;
		}
	      else
		{
		  setenv ("LESS", lessenv, 1);
		}
	    }

	  output_file = writepipe (program, NULL);
	  if (!output_file)
	    error (EXIT_ERROR, errno, "%s", program);
	}
    }
}

/*-----------------------------.
| Complete the pager program.  |
`-----------------------------*/

static void
complete_output_program (void)
{
  /* Let the user play at will inside the pager, until s/he exits, before
     proceeding any further.  */

  if (output_file && output_file != stdout)
    {
      fclose (output_file);
      wait (NULL);
    }
}

/* Other listing services.  */

enum margin_mode
{
  EMPTY_MARGIN,			/* nothing in left margin */
  LOCATION_IN_MARGIN,		/* number of next item */
  FILLER_IN_MARGIN,		/* bars for the length of nick name */
  FILE_IN_MARGIN		/* nick name for file */
};

/*-------------------------------------------.
| For file INPUT, produce a kind of MARGIN.  |
`-------------------------------------------*/

static void
make_margin (struct input *input, enum margin_mode margin)
{
  char buffer[15];
  char *cursor;

  switch (margin)
    {
    case EMPTY_MARGIN:
      return;

    case LOCATION_IN_MARGIN:
      sprintf (buffer, "%s%d", input->nick_name,
	       input->item - input->first_item + 1);
      break;

    case FILLER_IN_MARGIN:
      cursor = buffer + strlen (input->nick_name);
      *cursor-- = '\0';
      while (cursor > buffer)
	*cursor-- = '|';
      break;

    case FILE_IN_MARGIN:
      strcpy (buffer, input->nick_name);
      break;
    }

  if (word_mode)
    {
      push_emphasis (UNDERLINED);
      output_characters (buffer, strlen (buffer), 0);
      pop_emphasis ();
      putc (initial_tab ? '\t' : ' ', output_file);
    }
  else
    {
      push_emphasis (STRAIGHT);
      output_characters (buffer, strlen (buffer), 0);
      putc (initial_tab ? '\t' : ' ', output_file);
      pop_emphasis ();
    }
}

/*-------------------------.
| Skip a line from INPUT.  |
`-------------------------*/

static void
skip_line_item (struct input *input)
{
  input_line (input);
  assert (input->cursor);

  input->item++;
}

/*-------------------------.
| Copy a line from INPUT.  |
`-------------------------*/

static void
copy_line_item (struct input *input, enum margin_mode margin)
{
  input_line (input);
  assert (input->cursor);

  make_margin (input, margin);
  output_characters (input->line, input->limit - input->line, expand_tabs);

  input->item++;
}

/*---------------------------------.
| Skip over white space on INPUT.  |
`---------------------------------*/

static void
skip_whitespace (struct input *input)
{
  assert (input->cursor);
  while (input->cursor && isspace (*input->cursor))
    input_character (input);

  /* FIXME: Maybe worth following columns for TAB expansion, maybe not?  */
}

/*-----------------------------------------------------------------.
| Copy white space from INPUT to FILE, ensuring a kind of MARGIN.  |
`-----------------------------------------------------------------*/

static void
copy_whitespace (struct input *input, enum margin_mode margin)
{
  char *string = input->cursor;

  assert (input->cursor);
  while (input->cursor && isspace (*input->cursor))
    if (++input->cursor == input->limit)
      {
	if (input->cursor[-1] == '\n')
	  {
	    output_characters (string, input->cursor - string - 1,
			       expand_tabs);

	    /* While changing lines, ensure we stop any special display
	       prior to, and restore the special display after.  */

	    if (avoid_wraps && input->user_stop)
	      fprintf (output_file, "%s", input->user_stop);
#if HAVE_TPUTS
	    if (input->term_stop)
	      tputs (input->term_stop, 0, putc_for_tputs);
#endif

	    output_characters (input->cursor - 1, 1, expand_tabs);
	    make_margin (input, margin);

#if HAVE_TPUTS
	    if (input->term_start)
	      tputs (input->term_start, 0, putc_for_tputs);
#endif
	    if (avoid_wraps && input->user_start)
	      fprintf (output_file, "%s", input->user_start);
	  }
	else
	  output_characters (string, input->cursor - string, expand_tabs);

	input_character_helper (input);
	string = input->cursor;
      }

  if (string && input->cursor > string)
    output_characters (string, input->cursor - string, expand_tabs);
}

/*-------------------------------------.
| Skip over non-white space on INPUT.  |
`-------------------------------------*/

static void
skip_word_item (struct input *input)
{
  assert (input->cursor && !isspace (*input->cursor));
  while (input->cursor && !isspace (*input->cursor))
    input_character (input);

  /* FIXME: Maybe worth following columns for TAB expansion, maybe not?  */

  input->item++;
}

/*------------------------------------------.
| Copy non white space from INPUT to FILE.  |
`------------------------------------------*/

static void
copy_word_item (struct input *input)
{
  char *string = input->cursor;

  assert (input->cursor && !isspace (*input->cursor));
  while (input->cursor && !isspace (*input->cursor))
    input->cursor++;

  output_characters (string, input->cursor - string, expand_tabs);

  if (input->cursor == input->limit)
    input_character_helper (input);

  input->item++;
}

/*------------------------------------------.
| Skip a difference from INPUT up to ITEM.  |
`------------------------------------------*/

static void
skip_until (struct input *input, int item)
{
  assert (input->item <= item);
  if (word_mode)
    while (input->item < item)
      {
	skip_whitespace (input);
	skip_word_item (input);
      }
  else
    while (input->item < item)
      skip_line_item (input);
}

/*------------------------------------------------------------------.
| Copy a difference from INPUT up to ITEM, using a kind of MARGIN.  |
`------------------------------------------------------------------*/

static void
copy_until (struct input *input, int item, enum margin_mode margin)
{
  assert (input->item <= item);
  if (word_mode)
    while (input->item < item)
      {
	copy_whitespace (input, margin);
	copy_word_item (input);
      }
  else
    while (input->item < item)
      copy_line_item (input, margin);
}

/*------------------------------------.
| From INPUT, skip a cluster MEMBER.  |
`------------------------------------*/

static void
skip_member_proper (struct input *input, struct member *member)
{
  assert (input->item == member->first_item);
  skip_until (input, member->first_item + real_member_size (member));
}

/*------------------------------------.
| From INPUT, copy a cluster MEMBER.  |
`------------------------------------*/

static void
copy_member_proper (struct input *input, struct member *member)
{
  assert (input->item == member->first_item);
  copy_until (input, member->first_item + real_member_size (member),
	      FILLER_IN_MARGIN);
}

/* Listing control.  */

/*------------------------------------------.
| Relist all input files with annotations.  |
`------------------------------------------*/

static void
relist_annotated_files (void)
{
  /* The array of actives holds all members being concurrently listed.  If
     there is no overlapping members, the array will never hold more than
     one member.  The index in this array is also the column position of the
     vertical line bracing the member, in the annotated listing.  For
     preserving verticality of lines after the termination of some members,
     NULL members in the array stand for embedded white columns.  */

  struct active
  {
    struct member *member;	/* active member, or NULL */
    int remaining;		/* count of remaining lines to list */
  };
  struct active *active_array = NULL;
  int actives = 0;
  int allocated_actives = 0;

  int other_count = 0;		/* how many actives in other files */

  enum margin_mode margin_mode =
    show_links ? LOCATION_IN_MARGIN : EMPTY_MARGIN;

  struct input *input = NULL;
  int *cursor;
  int counter;
  FILE *file;
  ITEM *item;
  struct active *active;
  struct member *member;
  struct cluster *cluster;
  int ordinal;
  struct reference reference;
  char buffer[20];

#if DEBUGGING
  if (debugging)
    fputs ("\f\n", stderr);
#endif

  /* Prepare terminal.  */

  if (!paginate)
    {
      launch_output_program (input);
      initialize_strings ();
    }

  /* Prepare to scan all members.  */

  if (indirects > 0)
    {
      cursor = indirect_array;
      member = member_array + *cursor;
    }
  else
    cursor = NULL;

  /* Process all files.  */

  for (input = input_array; input < input_array + inputs; input++)
    {
      /* Prepare terminal.  */

      if (paginate)
	{
	  launch_output_program (input);
	  if (input == input_array)
	    initialize_strings ();
	}
      else
	{
	  if (input > input_array)
	    fputs ("\f\n", output_file);
	  fprintf (output_file, "@@@ %s\n", input->file_name);
	}

      /* Prepare for file.  */

      open_input (input);
      input->item = input->first_item;

      /* Process all items.  */

      if (word_mode && input->item < input->item_limit)
	{
	  make_margin (input, margin_mode);
	  input_line (input);
	}

      while (input->item < input->item_limit)
	{
	  if (word_mode)
	    copy_whitespace (input, margin_mode);

	  item = item_array + input->item;

	  /* See if we are starting any member.  */

	  while (cursor && member->first_item == input->item)
	    {
	      cluster = cluster_array + member->cluster_number;
	      ordinal = member - member_array - cluster->first_member - 1;

	      /* Obtain some active slot for the new member.  */

	      if (actives)
		{
		  /* Set ACTIVE to the rightmost not-in-use active, or after
		     all actives if they are all in use.  */

		  active = active_array + actives;
		  for (counter = 0; counter < actives; counter++)
		    if (!active_array[counter].member)
		      active = active_array + counter;

		  /* Output vertical bars as needed for all prior actives.  */

		  if (!word_mode)
		    for (counter = 0;
			 active_array + counter < active; counter++)
		      if (active_array[counter].member)
			putc ('|', output_file);
		      else
			putc (' ', output_file);

		  /* Ensure active will be allocated if necessary.  */

		  if (active == active_array + actives)
		    active = NULL;
		}
	      else
		active = NULL;

	      /* Allocate and initialise the active.  */

	      if (!active)
		{
		  if (actives == allocated_actives)
		    {
		      allocated_actives += 8;
		      active_array = (struct active *)
			xrealloc (active_array,
				  allocated_actives *
				  (sizeof (struct active)));
		    }
		  active = active_array + actives++;
		}
	      active->member = member;
	      active->remaining
		= cluster_array[member->cluster_number].item_count;

	      /* Print a pointer to the previous member of same cluster.  */

	      if (ordinal >= 0)
		{
		  reference = get_reference
		    (member_array
		     [cluster->first_member + ordinal].first_item);

		  if (word_mode)
		    {
		      if (show_links)
			{
			  sprintf (buffer, "[%c%s%d",
				   (char) (active - active_array + 'A'),
				   reference.input->nick_name,
				   reference.number);
			  push_emphasis (UNDERLINED);
			  output_characters (buffer, strlen (buffer), 0);
			  pop_emphasis ();
			  putc (' ', output_file);
			}
		    }
		  else
		    {
		      fprintf (output_file, ".-> [%d/%d] ",
			       ordinal + 1, MEMBERS (cluster));
		      sprintf (buffer, "%s%d",
			       reference.input->nick_name, reference.number);
		      push_emphasis (UNDERLINED);
		      output_characters (buffer, strlen (buffer), 0);
		      pop_emphasis ();
		      if (reference.input != input)
			fprintf (output_file, " (%s)",
				 reference.input->file_name);
		      putc ('\n', output_file);
		    }
		}
	      else if (word_mode)
		{
		  if (show_links)
		    {
		      sprintf (buffer, "[%c",
			       (char) (active - active_array + 'A'));
		      push_emphasis (UNDERLINED);
		      output_characters (buffer, strlen (buffer), 0);
		      pop_emphasis ();
		      putc (' ', output_file);
		    }
		}
	      else
		fputs (".-\n", output_file);

	      /* Bold text appearing in other files.  */

	      if (active_elsewhere (member, input))
		if (other_count++ == 0)
		  set_emphasis (BOLD);

	      /* Advance cursor.  */

	      if (++cursor < indirect_array + indirects)
		member = member_array + *cursor;
	      else
		cursor = NULL;
	    }

	  /* List the item itself.  */

	  if (word_mode)
	    copy_word_item (input);
	  else
	    {
	      for (counter = 0; counter < actives; counter++)
		if (active_array[counter].member)
		  putc ('|', output_file);
		else
		  putc (' ', output_file);

	      if (counter < 7)
		{
		  sprintf (buffer, "%s%d", input->nick_name,
			   input->item - input->first_item + 1);
		  for (counter += strlen (buffer); counter < 7; counter++)
		    putc (' ', output_file);
		  push_emphasis (UNDERLINED);
		  output_characters (buffer + counter - 7,
				     strlen (buffer + counter - 7), 0);
		  pop_emphasis ();
		}

	      if (initial_tab)
		putc ('\t', output_file);
	      else
		putc (' ', output_file);

	      copy_line_item (input, EMPTY_MARGIN);
	    }

	  /* See if we are ending any member.  */

	  if (!actives)
	    continue;

	  active = active_array + actives;
	  while (--active >= active_array)
	    if (item_type (item) == NORMAL && --active->remaining == 0)
	      {
		cluster = cluster_array + active->member->cluster_number;
		ordinal
		  = active->member - member_array - cluster->first_member + 1;

		/* Print a pointer to the next member of same cluster.  */

		if (!word_mode)
		  {
		    for (counter = 0;
			 active_array + counter < active; counter++)
		      if (active_array[counter].member)
			putc ('|', output_file);
		      else
			putc (' ', output_file);
		  }

		if (ordinal < MEMBERS (cluster))
		  {
		    reference = get_reference
		      (member_array
		       [cluster->first_member + ordinal].first_item);

		    if (word_mode)
		      {
			if (show_links)
			  {
			    putc (' ', output_file);
			    sprintf (buffer, "%s%d%c]",
				     reference.input->nick_name,
				     reference.number,
				     (char) (active - active_array + 'A'));
			    push_emphasis (UNDERLINED);
			    output_characters (buffer, strlen (buffer), 0);
			    pop_emphasis ();
			  }
		      }
		    else
		      {
			fprintf (output_file, "`-> [%d/%d] ",
				 ordinal + 1, MEMBERS (cluster));
			sprintf (buffer, "%s%d",
				 reference.input->nick_name,
				 reference.number);
			push_emphasis (UNDERLINED);
			output_characters (buffer, strlen (buffer), 0);
			pop_emphasis ();
			if (reference.input != input)
			  fprintf (output_file, " (%s)",
				   reference.input->file_name);
			putc ('\n', output_file);
		      }
		  }
		else if (word_mode)
		  {
		    if (show_links)
		      {
			putc (' ', output_file);
			sprintf (buffer, "%c]",
				 (char) (active - active_array + 'A'));
			push_emphasis (UNDERLINED);
			output_characters (buffer, strlen (buffer), 0);
			pop_emphasis ();
		      }
		  }
		else
		  fputs ("`-\n", output_file);

		/* Do not bold text not appearing in any other file.  */

		if (active_elsewhere (active->member, input))
		  if (--other_count == 0)
		    set_emphasis (STRAIGHT);

		/* Remove as many active members as we can.  */

		active->member = NULL;
		if (active == active_array + actives - 1)
		  while (actives > 0 && !active_array[actives - 1].member)
		    actives--;
	      }
	}

      /* Finish file.  */

      assert (actives == 0);
      assert (other_count == 0);

      close_input (input);

      if (paginate)
	complete_output_program ();
    }

  if (!paginate)
    complete_output_program ();
}

/*--------------------------------------------------------------------.
| Make a merged listing of input files.  If UNIFIED, produce unified  |
| context diffs instead of plain context diffs.  If CROSSED, identify |
| crossed blocks.						      |
`--------------------------------------------------------------------*/

static void
relist_merged_lines (int unified, int crossed)
{
  int counter;
  struct input *input;
  struct member *member;
  struct cluster *chosen_cluster;
  struct reference reference;
  struct merging *merging;
  struct merging *group_limit;
  struct merging *cursor;
  int listed;

  /* Prepare terminal.  */

  launch_output_program (NULL);
  initialize_strings ();

  /* FIXME: Merging requires all input files to be opened in parallel.  This
     is not realistic when there are really many input files: in such cases,
     one has no other choice than to avoid merged listings and resort to the
     full relisting option instead.  */

  for (input = input_array; input < input_array + inputs; input++)
    {
      open_input (input);
      input->item = input->first_item;
    }

  /* Process all merging groups, one at a time.  */

#if DEBUGGING
  if (debugging)
    fputs ("\f\n", stderr);
#endif

  for (merging = merging_array;
       merging < merging_array + mergings; merging = group_limit)
    {
      /* Find the extent of the merging group, and count genuine mergings.  */

      counter = 0;
      if (!merging->cross_flag)
	counter++;

      for (cursor = merging + 1;
	   cursor < merging_array + mergings && !cursor->group_flag; cursor++)
	if (!cursor->cross_flag)
	  counter++;

      group_limit = cursor;

      /* Prepare for a new hunk.  Hmph!  Not really yet...  */

      /* Output differences, which are items before members.  Also consider
         any crossed member as a difference and output it on the same blow.  */

      for (cursor = merging; cursor < group_limit; cursor++)
	{
	  input = input_array + cursor->input_number;
	  member = member_array + cursor->member_number;
	  copy_until (input, member->first_item, FILE_IN_MARGIN);
	  if (cursor->cross_flag)
	    {
	      explain_group ('/', cursor, cursor + 1);
	      copy_member_proper (input, member);
	      explain_group ('\\', cursor, cursor + 1);
	    }
	}

      /* Describe members, which we are about to skip.  */

      explain_group ('/', merging, group_limit);

      listed = 0;
      for (cursor = merging; cursor < group_limit; cursor++)
	if (!cursor->cross_flag)
	  {
	    input = input_array + cursor->input_number;
	    member = member_array + cursor->member_number;
	    if (listed)
	      skip_member_proper (input, member);
	    else
	      {
		copy_member_proper (input, member);
		listed = 1;
	      }
	  }

      explain_group ('\\', merging, group_limit);
    }

  /* Copy remaining differences and clean up.  */

  for (input = input_array; input < input_array + inputs; input++)
    {
      copy_until (input, input->item_limit, FILE_IN_MARGIN);
      close_input (input);
    }
}

/*-----------------------------------------------------.
| Study diff output and use it to drive reformatting.  |
`-----------------------------------------------------*/

/* Define the separator lines when output is inhibited.  */
#define SEPARATOR_LINE \
  "======================================================================"

static void
relist_merged_words (void)
{
  int counter;
  struct input *input;
  struct member *member;
  struct cluster *chosen_cluster;
  struct reference reference;
  struct merging *merging;
  struct merging *group_limit;
  struct merging *cursor;
  int listed;

  int count_total_left;		/* count of total words in left file */
  int count_total_right;	/* count of total words in right file */
  int count_isolated_left;	/* count of deleted words in left file */
  int count_isolated_right;	/* count of added words in right file */
  int count_changed_left;	/* count of changed words in left file */
  int count_changed_right;	/* count of changed words in right file */

  /* Prepare terminal.  */

  launch_output_program (NULL);
  initialize_strings ();

  /* Rewind input files.  */

  for (input = input_array; input < input_array + inputs; input++)
    {
      open_input (input);
      input->item = input->first_item;
      input_line (input);
    }

  count_total_left = left->item_limit - left->first_item;
  count_total_right = right->item_limit - right->first_item;
  count_isolated_left = 0;
  count_isolated_right = 0;
  count_changed_left = 0;
  count_changed_right = 0;

  /* Process all merging groups, one at a time.  */

  for (merging = merging_array;
       merging < merging_array + mergings; merging = group_limit)
    {
      /* Find the extent of the merging group, and count genuine mergings.  */

      counter = 0;
      if (!merging->cross_flag)
	counter++;

      for (cursor = merging + 1;
	   cursor < merging_array + mergings && !cursor->group_flag; cursor++)
	if (!cursor->cross_flag)
	  counter++;

      group_limit = cursor;

      /* Output differences, which are items before members.  Also consider
         any crossed member as a difference and output it on the same blow.  */

      for (cursor = merging; cursor < group_limit; cursor++)
	{
	  input = input_array + cursor->input_number;
	  member = member_array + cursor->member_number;
	  if (input->listing_allowed)
	    {
	      if (input->item < member->first_item || cursor->cross_flag)
		{
		  if (input == left)
		    copy_whitespace (input, EMPTY_MARGIN);
		  else
		    skip_whitespace (input);

		  if (input == input_array + inputs - 1)
		    start_of_emphasis (input, BOLD);
		  else
		    start_of_emphasis (input, UNDERLINED);

		  if (input->item < member->first_item)
		    {
		      copy_word_item (input);
		      copy_until (input, member->first_item, FILE_IN_MARGIN);
		    }
		  if (cursor->cross_flag)
		    copy_member_proper (input, member);

		  end_of_emphasis (input);
		}
	    }
	  else
	    {
	      skip_until (input, member->first_item);
	      if (cursor->cross_flag)
		skip_member_proper (input, member);
	    }
	}

      /* Use separator lines to disambiguate the output.  */

      if (common_listing_allowed)
	{
	  if (!left->listing_allowed && !right->listing_allowed)
	    fprintf (output_file, "\n%s\n", SEPARATOR_LINE);
	}
      else
	fprintf (output_file, "\n%s\n", SEPARATOR_LINE);

      /* Describe members, which we are about to skip.  */

      listed = 0;
      for (cursor = merging; cursor < group_limit; cursor++)
	if (!cursor->cross_flag)
	  {
	    input = input_array + cursor->input_number;
	    member = member_array + cursor->member_number;
	    if (listed)
	      skip_member_proper (input, member);
	    else
	      {
		copy_member_proper (input, member);
		listed = 1;
	      }
	  }
    }

  /* Copy remaining differences and clean up.  Copy from left side if the
     user wanted to see only the common code and deleted words.  */

  if (!common_listing_allowed
      && (left->listing_allowed || right->listing_allowed))
    fprintf (output_file, "\n%s\n", SEPARATOR_LINE);

  for (input = input_array; input < input_array + inputs; input++)
    {
      if (input->listing_allowed)
	{
	  if (input->item < input->item_limit)
	    {
	      if (input == left)
		copy_whitespace (input, EMPTY_MARGIN);
	      else
		skip_whitespace (input);

	      if (input == input_array + inputs - 1)
		start_of_emphasis (input, BOLD);
	      else
		start_of_emphasis (input, UNDERLINED);

	      copy_word_item (input);
	      copy_until (input, input->item_limit, FILE_IN_MARGIN);

	      end_of_emphasis (input);
	    }
	  copy_whitespace (input, EMPTY_MARGIN);
	}
      close_input (input);
    }

  /* Print merging statistics.  */

  if (verbose)
    {
      int count_common_left;	/* words unchanged in left file */
      int count_common_right;	/* words unchanged in right file */

      count_common_left
	= count_total_left - count_isolated_left - count_changed_left;
      count_common_right
	= count_total_right - count_isolated_right - count_changed_right;

      printf (ngettext ("%s: %d word", "%s: %d words", count_total_left),
	      left->file_name, count_total_left);
      if (count_total_left > 0)
	{
	  printf (ngettext ("  %d %.0f%% common", "  %d %.0f%% common",
			    count_common_left), count_common_left,
		  count_common_left * 100. / count_total_left);
	  printf (ngettext ("  %d %.0f%% deleted", "  %d %.0f%% deleted",
			    count_isolated_left), count_isolated_left,
		  count_isolated_left * 100. / count_total_left);
	  printf (ngettext ("  %d %.0f%% changed", "  %d %.0f%% changed",
			    count_changed_left), count_changed_left,
		  count_changed_left * 100. / count_total_left);
	}
      printf ("\n");

      printf (ngettext ("%s: %d word", "%s: %d words", count_total_right),
	      right->file_name, count_total_right);
      if (count_total_right > 0)
	{
	  printf (ngettext ("  %d %.0f%% common", "  %d %.0f%% common",
			    count_common_right), count_common_right,
		  count_common_right * 100. / count_total_right);
	  printf (ngettext ("  %d %.0f%% inserted", "  %d %.0f%% inserted",
			    count_isolated_right), count_isolated_right,
		  count_isolated_right * 100. / count_total_right);
	  printf (ngettext ("  %d %.0f%% changed", "  %d %.0f%% changed",
			    count_changed_right), count_changed_right,
		  count_changed_right * 100. / count_total_right);
	}
      printf ("\n");
    }

  /* Set exit status.  */

  if (count_isolated_left || count_isolated_right
      || count_changed_left || count_changed_right)
    exit_status = EXIT_DIFFERENCE;

  /* Reset the terminal.  */

  complete_output_program ();
}

/* Main control.  */

/*-----------------------------------------------.
| Explain how to use the program, then get out.	 |
`-----------------------------------------------*/

static void
usage (int status)
{
  if (status != EXIT_SUCCESS)
    fprintf (stderr, _("Try `%s --help' for more information.\n"),
	     program_name);
  else
    {
      /* *INDENT-OFF* */
      fputs (_("\
mdiff - Studies multiple files and searches for similar sequences, it then\n\
produces possibly detailed lists of differences and similarities.\n"),
	     stdout);
      fputs ("\n", stdout);
      printf (_("\
Usage: %s [OPTION]... [FILE]...\n"),
	      program_name);

      fputs (_("\nOperation modes:\n"), stdout);
      fputs (_("  -h                     (ignored)\n"), stdout);
      fputs (_("  -v, --verbose          report a few statistics on stderr\n"), stdout);
      fputs (_("      --help             display this help then exit\n"), stdout);
      fputs (_("      --version          display program version then exit\n"), stdout);

      fputs (_("\nFormatting output:\n"), stdout);
      fputs (_("  -T, --initial-tab       produce TAB instead of initial space\n"), stdout);
      fputs (_("  -l, --paginate          paginate output through `pr'\n"), stdout);
      fputs (_("  -S, --string[=STRING]   take note of another user STRING\n"), stdout);
      fputs (_("  -V, --show-links        give file and line references in annotations\n"), stdout);
      fputs (_("  -t, --expand-tabs       expand tabs to spaces in the output\n"), stdout);

#if DEBUGGING
      fputs (_("\nDebugging:\n"), stdout);
      fputs (_("  -0, --debugging   output many details about what is going on\n"), stdout);
#endif

      fputs (_("\nWord mode options:\n"), stdout);
      fputs (_("  -1, --no-deleted           inhibit output of deleted words\n"), stdout);
      fputs (_("  -2, --no-inserted          inhibit output of inserted words\n"), stdout);
      fputs (_("  -3, --no-common            inhibit output of common words\n"), stdout);
      fputs (_("  -A, --auto-pager           automatically calls a pager\n"), stdout);
      fputs (_("  -k, --less-mode            variation of printer mode for \"less\"\n"), stdout);
      fputs (_("  -m, --avoid-wraps          do not extend fields through newlines\n"), stdout);
      fputs (_("  -o, --printer              overstrike as for printers\n"), stdout);
      fputs (_("  -z, --terminal             use termcap as for terminal displays\n"), stdout);
      fputs (_("  -O, --item-regexp=REGEXP   compare items as defined by REGEXP\n"), stdout);
      fputs (_("  -W, --word-mode            compare words instead of lines\n"), stdout);

      /***
      fputs (_("\nComparing files:\n"), stdout);
      fputs (_("*  -H, --speed-large-files     go faster, for when numerous small changes\n"), stdout);
      fputs (_("*  -a, --text                  report line differences (text file default)\n"), stdout);
      fputs (_("*  -d, --minimal               try harder for a smaller set of changes\n"), stdout);
      fputs (_("*  -q, --brief                 only says if files differ (binary default)\n"), stdout);
      fputs (_("*      --horizon-lines=LINES   keep LINES lines in common prefixes/suffixes\n"), stdout);
      ***/
      /***
      fputs (_("\nComparing directories:\n"), stdout);
      fputs (_("*  -N, --new-file                  consider missing files to be empty\n"), stdout);
      fputs (_("*  -P, --unidirectional-new-file   consider missing old files to be empty\n"), stdout);
      fputs (_("*  -S, --starting-file=FILE        resume directory comparison with FILE\n"), stdout);
      fputs (_("*  -X, --exclude-from=FILE         ignore files matching patterns from FILE\n"), stdout);
      fputs (_("*  -r, --recursive                 recursively compare subdirectories\n"), stdout);
      fputs (_("*  -s, --report-identical-files    report when two files are the same\n"), stdout);
      fputs (_("*  -x, --exclude=PATTERN           ignore files (dirs) matching PATTERN\n"), stdout);
      ***/
      /***
      fputs (_("\nIgnoring text:\n"), stdout);
      fputs (_("  -B, --ignore-blank-lines             ignore blank lines\n"), stdout);
      fputs (_("*  -I, --ignore-matching-lines=REGEXP   ignore lines matching REGEXP\n"), stdout);
      fputs (_("  -b, --ignore-space-change            ignore amount of white space\n"), stdout);
      fputs (_("  -i, --ignore-case                    ignore case differences\n"), stdout);
      fputs (_("  -w, --ignore-all-space               ignore white space\n"), stdout);
      fputs (_("\nClustering:\n"), stdout);
      fputs (_("  -G, --relist-files         list all input files with annotations\n"), stdout);
      fputs (_("  -J, --minimum-size=ITEMS   ignore clusters not having that many ITEMS\n"), stdout);
      fputs (_("  -j, --ignore-delimiters    do not count items having only delimiters\n"), stdout);
      ***/
      /***
      fputs (_("\nDetailed output formats:\n"), stdout);
      fputs (_("*  -D, --ifdef=NAME                      output `#ifdef NAME' format\n"), stdout);
      fputs (_("*      --changed-group-format=FORMAT     use FORMAT for changed lines\n"), stdout);
      fputs (_("*      --new-group-format=FORMAT         use FORMAT for inserted lines\n"), stdout);
      fputs (_("*      --new-line-format=FORMAT          use FORMAT for inserted line\n"), stdout);
      fputs (_("*      --old-group-format=FORMAT         use FORMAT for deleted lines\n"), stdout);
      fputs (_("*      --old-line-format=FORMAT          use FORMAT for deleted line\n"), stdout);
      fputs (_("*      --unchanged-group-format=FORMAT   use FORMAT for unchanged lines\n"), stdout);
      fputs (_("*      --unchanged-line-format=FORMAT    use FORMAT for unchanged line\n"), stdout);
      fputs (_("*      --line-format=FORMAT              --{old,new,unchanged}-line-format\n"), stdout);
      ***/
      /***
      fputs (_("\nScript-like formats:\n"), stdout);
      fputs (_("  (none of -CDUcefnuy)   output normal diffs\n"), stdout);
      fputs (_("*  -e, --ed               output a valid `ed' script\n"), stdout);
      fputs (_("*  -f, --forward-ed       mix between -e and -n (not very useful)\n"), stdout);
      fputs (_("*  -n, --rcs              output RCS format (internally used by RCS)\n"), stdout);
      ***/
      /***
      fputs (_("\nContext and unified formats:\n"), stdout);
      fputs (_("*  -F, --show-function-line=REGEXP   show previous context matching REGEXP\n"), stdout);
      fputs (_("*  -p, --show-c-function             show which C function for each change\n"), stdout);
      ***/
      /***
      fputs (_("*  -C, --context=LINES         as -c, also select context size in lines\n"), stdout);
      fputs (_("*  -L, --label=LABEL           use from/to LABEL instead of file name (twice)\n"), stdout);
      fputs (_("*  -U, --unified=LINES         as -u, also select context size in lines\n"), stdout);
      fputs (_("*  -c, --context               output context diffs (default 3 context lines)\n"), stdout);
      fputs (_("*  -u, --unified               output unidiffs (default 3 context lines)\n"), stdout);
      fputs (_("*  -LINES                      (obsolete: select context size in lines)\n"), stdout);
      ***/
      /***
      fputs (_("\nSide by side format:\n"), stdout);
      fputs (_("*  -W, --width=COLUMNS           use width of COLUMNS\n"), stdout);
      fputs (_("*  -y, --side-by-side            use side by side output format\n"), stdout);
      fputs (_("*      --left-column             print only left column line when common\n"), stdout);
      fputs (_("*      --sdiff-merge-assist      (internally used by `sdiff')\n"), stdout);
      fputs (_("*      --suppress-common-lines   do not print common lines\n"), stdout);
      ***/
      /***
      fputs (_("\n\
FORMAT is made up of characters standing for themselves, except:\n\
  %%%%           a single %%\n\
  %%c'C'        quoted character C\n\
  %%c'\\O'       character having value O, from 1 to 3 octal digits\n\
  %%(A=B?T:E)   if A is B then T else E; A B number or VARIABLE; T E FORMAT\n\
  %%FN          use SPECIF specification F to print VARIABLE value N\n\
  %%<           [group] old, each line through --old-line-format\n\
  %%>           [group] new, each line through --new-line-format\n\
  %%=           [group] unchanged, each line through --unchanged-line-format\n\
  %%l           [line] without its possible trailing newline\n\
  %%L           [line] with its possible trailing newline\n"),
	     stdout);
      ***/
      /***
      fputs (_("\n\
SPECIF is [-][W[.D]]{doxX} as in C printf\n"),
	     stdout);
      ***/
      /***
      fputs (_("\n\
VARIABLE is {eflmn} for old group or {EFLMN} for new group\n\
  {eE}   line number just before group\n\
  {fF}   first line number of group\n\
  {lL}   last line number of group\n\
  {mM}   line number just after group\n\
  {nN}   number of lines in the group\n"),
	     stdout);
      ***/
      /***
      fputs (_("\nStandard diff options:\n"), stdout);
      fputs (_("  -i, --ignore-case         consider upper- and lower-case to be the same\n"), stdout);
      fputs (_("  -w, --ignore-all-space    ignore all white space\n"), stdout);
      fputs (_("  -b, --ignore-space-change ignore changes in the amount of white space\n"), stdout);
      fputs (_("  -B, --ignore-blank-lines  ignore changes whose lines are all blank\n"), stdout);
      fputs (_("  -I, --ignore-matching-lines=RE ignore changes whose lines all match RE\n"), stdout);
      fputs (_("  -a, --text                treat all files as text\n"), stdout);
      fputs (_("\
  -c, --context[=NUMBER]    output regular context diffs,\n\
                            changing to NUMBER lines of context\n"), stdout);
      fputs (_("\
  -u, --unified[=NUMBER]    output unified context diffs or unidiffs,\n\
                            with NUMBER lines of context\n"), stdout);
      fputs (_("  -C, --context=NUM         output NUM lines of copied context\n"), stdout);
      fputs (_("  -U, --unified=NUM         output NUM lines of unified context\n"), stdout);
      fputs (_("  -L, --label=LABEL         use LABEL instead of file name\n"), stdout);
      fputs (_("  -p, --show-c-function     show which C function each change is in\n"), stdout);
      fputs (_("  -F, --show-function-line=RE show the most recent line matching RE\n"), stdout);
      fputs (_("  -q, --brief               output only whether files differ\n"), stdout);
      fputs (_("  -e, --ed                  output an ed script\n"), stdout);
      fputs (_("  -n, --rcs                 output an RCS format diff\n"), stdout);
      fputs (_("  -y, --side-by-side        output in two columns\n"), stdout);
      fputs (_("  -w, --width=NUM           output at most NUM (default 130) characters per line\n"), stdout);
      fputs (_("      --left-column         output only the left column of common lines\n"), stdout);
      fputs (_("      --suppress-common-lines do not output common lines\n"), stdout);
      fputs (_("  -D, --ifdef=NAME          output merged file to show `#ifdef NAME' diffs\n"), stdout);
      fputs (_("      --GTYPE-group-format=GFMT GTYPE input groups with GFMT\n"), stdout);
      fputs (_("      --line-format=LFMT    all input lines with LFMT\n"), stdout);
      fputs (_("      --LTYPE-line-format=LFMT LTYPE input lines with LFMT\n"), stdout);
      fputs (_("  -l, --paginate            pass the output through `pr' to paginate it\n"), stdout);
      fputs (_("  -t, --expand-tabs         expand tabs to spaces in output\n"), stdout);
      fputs (_("  -T, --initial-tab         make tabs line up by prepending a tab\n"), stdout);
      fputs (_("  -r, --recursive           recursively compare any subdirectories found\n"), stdout);
      fputs (_("  -N, --new-file            treat absent files as empty\n"), stdout);
      fputs (_("  -P, --unidirectional-new-file treat absent first files as empty\n"), stdout);
      fputs (_("  -s, --report-identical-files report when two files are the same\n"), stdout);
      fputs (_("  -x, --exclude=PAT         exclude files that match PAT\n"), stdout);
      fputs (_("  -X, --exclude-from=FILE   exclude files that match any pattern in FILE\n"), stdout);
      fputs (_("  -S, --starting-file=FILE  start with FILE when comparing directories\n"), stdout);
      fputs (_("      --horizon-lines=NUM   keep NUM lines of the common prefix and suffix\n"), stdout);
      fputs (_("  -d, --minimal             try hard to find a smaller set of changes\n"), stdout);
      fputs (_("  -H, --speed-large-files   assume large files and many scattered small changes\n"), stdout);
      ***/
      /***
      fputs (_("\
\n\
By default, context diffs have an horizon of two lines.\n"),
	     stdout);
      ***/
      /***
      fputs (_("\
\n\
LTYPE is `old', `new', or `unchanged'.  GTYPE is LTYPE or `changed'.\n\
GFMT may contain:\n\
  %<  lines from FILE1\n\
  %>  lines from FILE2\n\
  %=  lines common to FILE1 and FILE2\n\
  %[-][WIDTH][.[PREC]]{doxX}LETTER  printf-style spec for LETTER\n\
    LETTERs are as follows for new group, lower case for old group:\n\
      F  first line number\n\
      L  last line number\n\
      N  number of lines = L-F+1\n\
      E  F-1\n\
      M  L+1\n"),
	     stdout);
      fputs (_("\
LFMT may contain:\n\
  %L  contents of line\n\
  %l  contents of line, excluding any trailing newline\n\
  %[-][WIDTH][.[PREC]]{doxX}n  printf-style spec for input line number\n\
Either GFMT or LFMT may contain:\n\
  %%  %\n\
  %c'C'  the single character C\n\
  %c'\\OOO'  the character with octal code OOO\n"),
	     stdout);
      ***/
      /***
      fputs (_("\nOld mdiff options:\n"), stdout);
      fputs (_("*  -f, --fuzz-items=ITEMS     no more than ITEMS non matching in a cluster\n"), stdout);
      ***/

      fputs ("\n", stdout);
      fputs (_("With no FILE, or when FILE is -, read standard input.\n"), stdout);
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

#define UNIMPLEMENTED(Option) \
  error (0, 0, _("ignoring option %s (not implemented)"), (Option))

int
main (int argc, char *const *argv)
{
  int option_char;		/* option character */
  struct input *input;		/* cursor in input array */

  int inhibit_left = 0;
  int inhibit_right = 0;
  int inhibit_common = 0;
  const char *delete_start = NULL;
  const char *delete_stop = NULL;
  const char *insert_start = NULL;
  const char *insert_stop = NULL;

  program_name = argv[0];
  setlocale (LC_ALL, "");
  bindtextdomain (PACKAGE, LOCALEDIR);
  textdomain (PACKAGE);

  /* Decode command options.  */

  while (option_char = getopt_long (argc, (char **) argv, OPTION_STRING,
				    long_options, NULL), option_char != EOF)
    switch (option_char)
      {
      default:
	usage (EXIT_ERROR);

      case '\0':
	break;

      case '0':
#if DEBUGGING
	debugging = 1;
	verbose = 1;
#else
	UNIMPLEMENTED ("--debugging");
#endif
	break;

      case '1':
	inhibit_left = 1;
	break;

      case '2':
	inhibit_right = 1;
	break;

      case '3':
	inhibit_common = 1;
	break;

      case 'A':
	autopager = 1;
	break;

      case 'B':
	ignore_blank_lines = 1;
	break;

      case 'D':
	ifdef = optarg;
	UNIMPLEMENTED ("--ifdef");
	break;

      case 'F':
	show_function_line = optarg;
	UNIMPLEMENTED ("--show-function-line");
	break;

      case 'G':
	relist_files = 1;
	break;

      case 'H':
	speed_large_files = 1;
	UNIMPLEMENTED ("--speed-large-files");
	break;

      case 'I':
	if (ignore_regexps % 8 == 0)
	  ignore_regexp_array = (struct re_pattern_buffer **)
	    xrealloc (ignore_regexp_array,
		      ((ignore_regexps + 8)
		       * sizeof (struct re_pattern_buffer *)));
	ignore_regexp_array[ignore_regexps++]
	  = alloc_and_compile_regex (optarg);
	break;

      case 'J':
	minimum_size = atoi (optarg);
	break;

      case 'L':
	label = optarg;
	UNIMPLEMENTED ("--label");
	break;

      case 'N':
	new_file = 1;
	UNIMPLEMENTED ("--new-file");
	break;

      case 'O':
	UNIMPLEMENTED ("--compare-words");
	item_regexp = alloc_and_compile_regex (optarg);
	break;

      case 'P':
	unidirectional_new_file = 1;
	UNIMPLEMENTED ("--unidirectional-new-file");
	break;

      case 'Q':
	insert_start = optarg;
	break;

      case 'R':
	insert_stop = optarg;
	break;

      case 'S':
	starting_file = optarg;
	UNIMPLEMENTED ("--starting-file");
	break;

      case 'T':
	initial_tab = 1;
	break;

      case 'V':
	show_links = 1;
	break;

      case 'W':
	word_mode = 1;
	break;

      case 'X':
	exclude_from = optarg;
	UNIMPLEMENTED ("--exclude-from");
	break;

      case 'Y':
	delete_start = optarg;
	break;

      case 'Z':
	delete_stop = optarg;
	break;

      case 'a':
	text = 1;
	UNIMPLEMENTED ("--text");
	break;

      case 'b':
	ignore_space_change = 1;
	ignore_blank_lines = 1;
	break;

      case 'C':
      case 'c':
	UNIMPLEMENTED ("--context");
	if (optarg)
	  horizon_lines = atoi (optarg);
	context = 1;
	break;

      case 'd':
	minimal = 1;
	UNIMPLEMENTED ("--minimal");
	break;

      case 'e':
	ed = 1;
	UNIMPLEMENTED ("--ed");
	break;

#if FIXME
      case 'h':
	UNIMPLEMENTED ("-h");
	break;
#endif

      case 'i':
	ignore_case = 1;
	break;

      case 'j':
	ignore_delimiters = 1;
	break;

      case 'k':
	if (find_termcap < 0)
	  find_termcap = 0;
	overstrike = 1;
	overstrike_for_less = 1;
	break;

      case 'l':
	paginate = 1;
	break;

      case 'm':
	avoid_wraps = 1;
	break;

      case 'n':
	rcs = 1;
	UNIMPLEMENTED ("--rcs");
	break;

      case 'o':
	overstrike = 1;
	break;

      case 'p':
	show_c_function = 1;
	UNIMPLEMENTED ("--show-c-function");
	break;

      case 'q':
	brief = 1;
	UNIMPLEMENTED ("--brief");
	break;

      case 'r':
	recursive = 1;
	UNIMPLEMENTED ("--recursive");
	break;

#if FIXME
      case 's':
	report_identical_files = 1;
	UNIMPLEMENTED ("--report-identical-files");
	break;
#endif

      case TOLERANCE_OPTION:	/* mdiff draft */
	UNIMPLEMENTED ("--tolerance");
	tolerance = atoi (optarg);
	break;

      case 't':
	expand_tabs = 1;
	UNIMPLEMENTED ("--expand-tabs");
	break;

      case 'U':
      case 'u':
	UNIMPLEMENTED ("--unified");
	if (optarg)
	  horizon_lines = atoi (optarg);
	unified = 1;
	break;

      case 'v':
	verbose = 1;
	break;

      case 'w':
	ignore_all_space = 1;
	break;

#if FIXME
      case 'w':
	width = atoi (optarg);
	UNIMPLEMENTED ("--width");
	break;
#endif

      case 'x':
	exclude = optarg;
	UNIMPLEMENTED ("--exclude");
	break;

      case 'y':
	side_by_side = 1;
	UNIMPLEMENTED ("--side-by-side");
	break;

      case 'K':
	/* compatibility option, equal to -t now */
	/* fall through */

      case 'z':
#if HAVE_TPUTS
	if (find_termcap < 0)
	  find_termcap = 1;
	break;
#else
	error (EXIT_ERROR, 0, _("cannot use -z, termcap not available"));
#endif

      case GTYPE_GROUP_FORMAT_OPTION:
	gtype_group_format = optarg;
	UNIMPLEMENTED ("--gtype-group-format");
	break;

      case HORIZON_LINES_OPTION:
	horizon_lines = atoi (optarg);
	UNIMPLEMENTED ("--horizon-lines");
	break;

      case LEFT_COLUMN_OPTION:
	left_column = 1;
	UNIMPLEMENTED ("--left-column");
	break;

      case LINE_FORMAT_OPTION:
	line_format = optarg;
	UNIMPLEMENTED ("--line-format");
	break;

      case LTYPE_LINE_FORMAT_OPTION:
	ltype_line_format = optarg;
	UNIMPLEMENTED ("--ltype-line-format");
	break;

      case SUPPRESS_COMMON_LINES_OPTION:
	suppress_common_lines = 1;
	UNIMPLEMENTED ("--suppress-common-lines");
	break;
      }

  if (minimum_size < 0)
    minimum_size = relist_files ? (word_mode ? 10 : 5) : 1;

  if (!relist_files && word_mode && argc - optind != 2)
    {
      error (0, 0, _("word merging for two files only (so far)"));
      usage (EXIT_ERROR);
    }

  /* If find_termcap still undecided, make it true only if autopager is
     set while stdout is directed to a terminal.  This decision might be
     reversed later, if the pager happens to be "less".  */

  if (find_termcap < 0)
    find_termcap = autopager && isatty (fileno (stdout));

  /* Process trivial options.  */

  if (show_version)
    {
      printf ("mdiff (GNU %s) %s\n", PACKAGE, VERSION);
      fputs (_("\
\n\
Copyright (C) 1992, 1997, 1998, 1999, 2010 Free Software Foundation, Inc.\n"), stdout);
      fputs (_("\
This is free software; see the source for copying conditions.  There is NO\n\
warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.\n"), stdout);
      fputs (_("\
\n\
Written by Franc,ois Pinard <pinard@iro.umontreal.ca>.\n"), stdout);
      exit (EXIT_SUCCESS);
    }

  if (show_help)
    usage (EXIT_SUCCESS);

  /* Register all input files.  */

  if (optind == argc)
    new_input ("-");
  else
    while (optind < argc)
      new_input (argv[optind++]);

  /* Save some option values.  */

  if (inputs == 2)
    {
      left->listing_allowed = !inhibit_left;
      left->user_start = delete_start;
      left->user_stop = delete_stop;

      right->listing_allowed = !inhibit_right;
      right->user_start = insert_start;
      right->user_stop = insert_stop;

      common_listing_allowed = !inhibit_common;
    }
  else
    {
      if (inhibit_left || inhibit_right || inhibit_common
	  || delete_start || delete_stop || insert_start || insert_stop)
	error (0, 0, _("options -123RSYZ meaningful only when two inputs"));
    }

  /* Do all the crunching.  */

  study_all_inputs ();
  prepare_clusters ();
  prepare_indirects ();
  if (!relist_files)
    prepare_mergings ();

  /* Output results.  */

  output_file = stdout;

  if (relist_files)
    relist_annotated_files ();
  else if (context)
    relist_merged_lines (0, 1);
  else if (unified)
    relist_merged_lines (1, 1);
  else if (word_mode)
    relist_merged_words ();

  /* Clean up.  */

  exit (exit_status);
}
