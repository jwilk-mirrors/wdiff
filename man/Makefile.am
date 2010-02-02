## Copying and distribution of this file, with or without modification,
## are permitted in any medium without royalty provided the copyright
## notice and this notice are preserved.  This file is offered as-is,
## without any warranty.

man_MANS = wdiff.1 $(EXPERIMENTAL_MANS)
EXTRA_MANS = mdiff.1 unify.1 wdiff2.1
EXTRA_DIST = mdiff.1 unify.1 wdiff.1 wdiff2.1

# The man pages depend on the --help strings and the version number.
common_mandeps = $(top_srcdir)/configure.ac

# Depend on the source, not the binary; we don't need to regenerate the
# binary when any source file changes, only the main one.
# Use -o so that the `missing' program can infer the output file.
mdiff.1: $(top_srcdir)/src/mdiff.c $(common_mandeps)
	test -f ../src/mdiff || $(MAKE) -C ../src mdiff
	$(HELP2MAN) ../src/mdiff -o $@

unify.1: $(top_srcdir)/src/unify.c $(common_mandeps)
	test -f ../src/unify || $(MAKE) -C ../src unify
	$(HELP2MAN) ../src/unify -o $@

wdiff.1: $(top_srcdir)/src/wdiff.c $(common_mandeps)
	test -f ../src/wdiff || $(MAKE) -C ../src wdiff
	$(HELP2MAN) --name="display word differences between text files" ../src/wdiff -o $@

wdiff2.1: $(top_srcdir)/src/wdiff2.c $(common_mandeps)
	test -f ../src/wdiff2 || $(MAKE) -C ../src wdiff2
	$(HELP2MAN) ../src/wdiff2 -o $@
