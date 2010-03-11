## Copying and distribution of this file, with or without modification,
## are permitted in any medium without royalty provided the copyright
## notice and this notice are preserved.  This file is offered as-is,
## without any warranty.

info_TEXINFOS = wdiff.texi
wdiff_TEXINFOS = flags.texi
GENDOCS = $(top_srcdir)/build-aux/gendocs.sh

# Convenience targets for manually switching build flags.
# Note that configure will overwrite any changes made using these.
.PHONY: experimental no-experimental
experimental:
	sed -i 's/^@clear EXPERIMENTAL/@set EXPERIMENTAL/' flags.texi
no-experimental:
	sed -i 's/^@set EXPERIMENTAL/@clear EXPERIMENTAL/' flags.texi

# Generate online manual.
.PHONY: manual
manual: experimental
	$(GENDOCS) --email '$(PACKAGE_BUGREPORT)' \
		'$(PACKAGE)' '$(PACKAGE_NAME) Manual'
