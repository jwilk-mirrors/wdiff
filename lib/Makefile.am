# Makefile for GNU wdiff library.
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

AUTOMAKE_OPTIONS = gnits

noinst_LIBRARIES = libwd.a

EXTRA_DIST = gettext.c

noinst_HEADERS = \
error.h getline.h getopt.h gettext.h pathmax.h regex.h xalloc.h

libwd_a_SOURCES = \
regex.c error.c getline.c getopt.c getopt1.c readpipe.c writepipe.c \
xmalloc.c xstrdup.c

INCLUDES = -I.. -I$(srcdir)

libwd_a_LIBADD = @ALLOCA@ @LIBOBJS@
libwd_a_DEPENDENCIES = $(libwd_a_LIBADD)
localedir = $(prefix)/share/locale
aliaspath = $(localedir):.

gettext.o: gettext.c
	$(COMPILE) -DALIASPATH=\"$(aliaspath)\" -DLOCALEDIR=\"$(localedir)\" \
	  -c $(srcdir)/gettext.c

gettext._o: gettext._c
	@rm -f _gettext.c
	@ln gettext._c _gettext.c
	$(COMPILE) -DALIASPATH=\"$(aliaspath)\" -DLOCALEDIR=\"$(localedir)\" \
	  -c _gettext.c
	@mv _gettext.o $@
	@rm _gettext.c
