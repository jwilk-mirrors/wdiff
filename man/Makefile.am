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
	test ../src/mdiff -nt $< || $(MAKE) -C ../src mdiff
	PATH="../src:$$PATH" $(HELP2MAN) mdiff -o $@

unify.1: $(top_srcdir)/src/unify.c $(common_mandeps)
	test ../src/unify -nt $< || $(MAKE) -C ../src unify
	PATH="../src:$$PATH" $(HELP2MAN) unify -o $@

wdiff.1: $(top_srcdir)/src/wdiff.c wdiff.h2m $(common_mandeps)
	test ../src/wdiff -nt $< || $(MAKE) -C ../src wdiff
	PATH="../src:$$PATH" $(HELP2MAN) -i wdiff.h2m wdiff -o $@

wdiff2.1: $(top_srcdir)/src/wdiff2.c $(common_mandeps)
	test ../src/wdiff2 -nt $< || $(MAKE) -C ../src wdiff2
	PATH="../src:$$PATH" $(HELP2MAN) wdiff2 -o $@
