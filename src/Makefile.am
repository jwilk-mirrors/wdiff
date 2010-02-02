## Copying and distribution of this file, with or without modification,
## are permitted in any medium without royalty provided the copyright
## notice and this notice are preserved.  This file is offered as-is,
## without any warranty.

bin_PROGRAMS = unify wdiff mdiff
noinst_PROGRAMS = wdiff2

unify_SOURCES = unify.c system.h
wdiff_SOURCES = wdiff.c readpipe.c writepipe.c system.h
mdiff_SOURCES = mdiff.c readpipe.c writepipe.c system.h
wdiff2_SOURCES = wdiff2.c system.h

unify_LDADD = ../lib/libgnu.a $(LIBINTL)
wdiff_LDADD = ../lib/libgnu.a $(LIBINTL)
mdiff_LDADD = ../lib/libgnu.a $(LIBINTL)
wdiff2_LDADD = ../lib/libgnu.a $(LIBINTL)

localedir = $(datadir)/locale

AM_CPPFLAGS = -I$(top_srcdir)/lib
DEFS += -DLOCALEDIR=\"$(localedir)\"
