# Main Makefile for GNU wdiff.
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
ACLOCAL_AMFLAGS = -I m4

BABYL = ../rmail/* ../admin/*/RMAIL
EXTRA_DIST = rebox.el PATCHES-AC README-mdiff
SUBDIRS = m4 doc i18n lib src tests

POTFILES = src/mdiff.c src/unify.c src/wdiff.c

all-local: $(srcdir)/stamp-pot

$(srcdir)/stamp-pot: $(POTFILES)
	@echo "$(POTFILES)" > $@


## # Copyright © 1992, 1993, 1994, 1997 Free Software Foundation, Inc.
## # François Pinard <pinard@iro.umontreal.ca>, 1992.
##
## SCRIPTS = @SCRIPTS@
##
## unify.1 wdiff.1 cbars.sh
##
## cbars: cbars.sh
## 	sed -e 's|@bindir@|$(bindir)|' cbars.sh > cbars
