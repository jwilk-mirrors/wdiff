#!/bin/sh

# Regenerate .bzrignore file for files generated (e.g. by gnulib)
# Copyright (C) 2010 Free Software Foundation, Inc.
# 2010 Martin von Gagern
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.


######################################################################
# Take care of gnulib-generated files in lib directory:
# lib/*.in.h     -> lib/*.h     (ignore that single file)
# lib/sys_*.in.h -> lib/sys/*.h (ignore lib/sys as a whole)
# lib/*.sin      -> lib/*.sed   (ignore that single file)

grep -vE '^lib/.*\.(h|sed)$' .bzrignore > tmp.bzrignore
ls lib/*.in.h | sed 's/\.in\.h$/.h/; /^lib\/sys_/d' >> tmp.bzrignore
ls lib/*.sin | sed 's/\.sin$/.sed/' >> tmp.bzrignore
grep -v / tmp.bzrignore | LC_ALL=C sort > .bzrignore
grep / tmp.bzrignore | LC_ALL=C sort >> .bzrignore
rm tmp.bzrignore


######################################################################
# Print changes

bzr diff .bzrignore
bzr ls -V -R lib | grep -Fxf .bzrignore | sed 's/^/!/; s/$/ (versioned!)/'
