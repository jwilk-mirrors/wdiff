bin_PROGRAMS = unify wdiff
noinst_PROGRAMS = mdiff wdiff2

unify_SOURCES = unify.c system.h
wdiff_SOURCES = wdiff.c readpipe.c writepipe.c system.h
mdiff_SOURCES = mdiff.c readpipe.c writepipe.c system.h
wdiff2_SOURCES = wdiff2.c system.h

unify_LDADD = ../lib/libgnu.a
wdiff_LDADD = ../lib/libgnu.a
mdiff_LDADD = ../lib/libgnu.a
wdiff2_LDADD = ../lib/libgnu.a

localedir = $(datadir)/locale

AM_CPPFLAGS = -I$(top_srcdir)/lib
DEFS = -DLOCALEDIR=\"$(localedir)\" @DEFS@
