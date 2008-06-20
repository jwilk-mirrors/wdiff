dist_man_MANS = mdiff.1 unify.1 wdiff.1

# The man pages depend on the --help strings and the version number.
common_mandeps = $(top_srcdir)/configure.ac

# Depend on the source, not the binary; we don't need to regenerate the
# binary when any source file changes, only the main one.
# Use -o so that the `missing' program can infer the output file.
mdiff.1: $(top_srcdir)/src/mdiff.c $(common_mandeps)
	$(HELP2MAN) ../src/mdiff -o $@

unify.1: $(top_srcdir)/src/unify.c $(common_mandeps)
	$(HELP2MAN) ../src/unify -o $@

wdiff.1: $(top_srcdir)/src/wdiff.c $(common_mandeps)
	$(HELP2MAN) --name="display word differences between text files" ../src/wdiff -o $@
