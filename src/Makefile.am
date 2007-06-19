# Makefile for GNU wdiff sources.
# Copyright (C) 1997, 1998, 1999 Free Software Foundation, Inc.

# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2, or (at your option)
# any later version.

# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.

# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
# 02110-1301, USA.

AUTOMAKE_OPTIONS = gnits ansi2knr
bin_PROGRAMS = unify wdiff
man_MANS = mdiff.1 unify.1 wdiff.1
noinst_PROGRAMS = mdiff wdiff2
noinst_SCRIPTS = cbars

EXTRA_DIST = cbars.in

noinst_HEADERS = system.h

INCLUDES =-I$(top_srcdir)/lib
LDADD = ../lib/libwd.a

PERL = perl

# Just to adjust version.  Should we return to the oldish version.c concept?
mdiff.o unify.o wdiff.o: ../config.status

## CFLAGS = -g -I. -DHAVE_STRERROR -DHAVE_VPRINTF -DSTDC_HEADERS

$(srcdir)/mdiff.1: mdiff
	@if test -r $@ && test ! -w $@; then \
	  echo "WARNING: Page \`$@' read only, not updated"; \
	elif $(PERL) $(top_srcdir)/doc/help2man --output=$@ ./mdiff; then \
	  echo "Page \`$@' has been updated"; \
	else \
	  echo "WARNING: Page \`$@' has *not* been updated."; \
	  echo "         It might be that \`Perl' is missing on your system,"; \
	  echo "         but you may safely and merely ignore this error."; \
	fi

$(srcdir)/unify.1: unify
	@if test -r $@ && test ! -w $@; then \
	  echo "WARNING: Page \`$@' read only, not updated"; \
	elif $(PERL) $(top_srcdir)/doc/help2man --output=$@ ./unify; then \
	  echo "Page \`$@' has been updated"; \
	else \
	  echo "WARNING: Page \`$@' has *not* been updated."; \
	  echo "         It might be that \`Perl' is missing on your system,"; \
	  echo "         but you may safely and merely ignore this error."; \
	fi

$(srcdir)/wdiff.1: wdiff
	@if test -r $@ && test ! -w $@; then \
	  echo "WARNING: Page \`$@' read only, not updated"; \
	elif $(PERL) $(top_srcdir)/doc/help2man --output=$@ ./wdiff; then \
	  echo "Page \`$@' has been updated"; \
	else \
	  echo "WARNING: Page \`$@' has *not* been updated."; \
	  echo "         It might be that \`Perl' is missing on your system,"; \
	  echo "         but you may safely and merely ignore this error."; \
	fi
