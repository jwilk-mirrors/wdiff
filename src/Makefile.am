## Copying and distribution of this file, with or without modification,
## are permitted in any medium without royalty provided the copyright
## notice and this notice are preserved.  This file is offered as-is,
## without any warranty.

bin_PROGRAMS = wdiff $(EXPERIMENTAL_PROGRAMS)
EXTRA_PROGRAMS = mdiff unify wdiff2

unify_SOURCES = unify.c wdiff.h
wdiff_SOURCES = wdiff.c pipes.c wdiff.h
mdiff_SOURCES = mdiff.c pipes.c wdiff.h
wdiff2_SOURCES = wdiff2.c wdiff.h

unify_LDADD = ../lib/libgnu.a $(LIBINTL)
wdiff_LDADD = ../lib/libgnu.a $(LIBINTL)
mdiff_LDADD = ../lib/libgnu.a $(LIBINTL)
wdiff2_LDADD = ../lib/libgnu.a $(LIBINTL)

AM_CPPFLAGS = -I$(top_srcdir)/lib
DEFS += -DLOCALEDIR=\"$(localedir)\"

../lib/libgnu.a:
	test -f $@ || $(MAKE) -C ../lib
