# Makefile for PO directories.
# Copyright (C) 1998 Free Software Foundation, Inc.
# Francois Pinard <pinard@iro.umontreal.ca>, 1998.

# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2, or (at your option)
# any later version.

# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.

# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software Foundation,
# Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

AUTOMAKE_OPTIONS = gnits

localedir = $(prefix)/share/locale

ALL_POFILES = @ALL_POFILES@
ALL_MOFILES = @ALL_MOFILES@
MOFILES = @MOFILES@

EXTRA_DIST = $(PACKAGE).pot $(ALL_POFILES) $(ALL_MOFILES)

MSGFMT = msgfmt
MSGMERGE = msgmerge
XGETTEXT = xgettext

SUFFIXES = .po .pox .mo

.po.pox:
	$(MAKE) $(PACKAGE).pot
	$(MSGMERGE) $< $(srcdir)/$(PACKAGE).pot -o $*.pox

.po.mo:
	file=$(srcdir)/`echo $* | sed 's,.*/,,'`.mo \
	  && rm -f $$file && $(MSGFMT) -o $$file $<

all: $(ALL_MOFILES)

install-data-local: $(MOFILES)
if USE_NLS
	files='$(MOFILES)'; \
	for file in $$files; do \
	  base=`basename $$file`; \
	  langdir=$(localedir)/`echo $$base | sed 's/\.mo$$//'`/LC_MESSAGES; \
	  $(mkinstalldirs) $$langdir; \
	  echo " $(INSTALL_DATA) $(srcdir)/$$file $$langdir/$(PACKAGE).mo"; \
	  $(INSTALL_DATA) $(srcdir)/$$file $$langdir/$(PACKAGE).mo; \
	done
endif

uninstall-local:
if USE_NLS
	files='$(MOFILES)'; \
	for file in $$files; do \
	  base=`basename $$file`; \
	  langdir=$(localedir)/`echo $$base | sed 's/\.mo$$//'`/LC_MESSAGES; \
	  rm -f $$langdir/$(PACKAGE).mo; \
	done
endif

MAINTAINER_CLEAN = $(PACKAGE).pot $(MOFILES)

$(srcdir)/$(PACKAGE).pot: $(top_srcdir)/stamp-pot
	$(XGETTEXT) --default-domain=$(PACKAGE) --directory=$(top_srcdir) \
	  --add-comments --keyword=_ --keyword=N_ `cat $(top_srcdir)/stamp-pot`
	test ! -f $(PACKAGE).po || ( rm -f $@ && mv $(PACKAGE).po $@ )

update-po: Makefile
	$(MAKE) $(PACKAGE).pot
	files='$(ALL_MOFILES)'; \
	for file in $$files; do \
	  base=`basename $$file`; \
	  lang=`echo $$base | sed 's/\.mo$$//'`; \
	  mv $(srcdir)/$$lang.po $(srcdir)/$$lang.old.po; \
	  echo "$$lang:"; \
	  cd $(srcdir) \
	    && if $(MSGMERGE) $$lang.old.po $(PACKAGE).pot -o $$lang.po; then \
	      rm -f $$lang.old.po; \
	    else \
	      echo "msgmerge for $$file failed!"; \
	      rm -f $$lang.po; \
	      mv $$lang.old.po $$lang.po; \
	    fi; \
	done
