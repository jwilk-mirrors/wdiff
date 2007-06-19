# Makefile for GNU wdiff regression tests.
# Copyright (C) 1997, 1998, 1999 Free Software Foundation, Inc.
# Francois Pinard <pinard@iro.umontreal.ca>, 1997.

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

SUITE = cluster.m4 suite.m4 wdiff.m4

EXTRA_DIST = atgeneral.m4 test-one testsuite $(SUITE)

all-local: atconfig testsuite

check-local: atconfig testsuite
	$(SHELL) $(srcdir)/testsuite

$(srcdir)/testsuite: atgeneral.m4 $(SUITE)
	m4 -I $(srcdir) atgeneral.m4 suite.m4 | cat -s > $@-tmp
	mv $@-tmp $@
	chmod +x $@

clean-local:
	rm -f debug-*.sh
